#include "config/config.h"

#include "Graph.h"
#include "NodeLibrary.h"
#include "io/InputStream.h"
#include "Node.h"
#include "SignalPath.h"
#include "SelectionInterface.h"
#include "io/OutputStream.h"
#include "DebugPrinter.h"
#include "core/LuaInterface.h"
#include "TypedPort.h"
#include "Types.h"
#include "CloningFacility.h"

#include <queue>

namespace dag
{
	Graph::~Graph()
	{
		for (auto const & it : _nodes)
		{
			delete it.second;
		}

        // The nodeLookupByName does not own the nodes
        // Deleting them would be a double-free.

		for (auto const it : _signalPaths)
		{
			delete it.second;
		}

        for (auto child : _children)
        {
            delete child;
        }
	}

	void Graph::addNode(Node* node)
	{
		if (node != nullptr && _nodeLib!=nullptr)
        {
			auto result = _nodes.insert(std::make_pair(node->id(), node));
            auto it = _nodeLookupByName.find(node->name());
            if (result.second && it==_nodeLookupByName.end())
            {
                _nodeLookupByName.insert(NameToNodeMap::value_type(node->name(), node));
                _lastAddedNode = node;
            }
		}
	}

	void Graph::addSignalPath(SignalPath* signalPath)
	{
		if (signalPath != nullptr)
		{
			_signalPaths.insert(SignalPathMap::value_type(signalPath->id(), signalPath));
		}
	}

    template<typename PortClass>
    void readTypedPort(NodeLibrary& nodeLib, dagbase::Table& portTable, Node* node, Port* existingPort, PortClass value)
    {
        auto* port = dynamic_cast<TypedPort<PortClass>*>(existingPort);

        if (port != nullptr)
        {
            port->setValue(value);
        }
        else if (existingPort == nullptr)
        {
            std::string portName = portTable.stringForNameOrDefault("name", "<unnamed>");
            std::string portTypeStr = portTable.stringForNameOrDefault("type", "TYPE_UNKNOWN");
            PortType::Type portType = PortType::parseFromString(portTypeStr.c_str());
            std::string dirStr = portTable.stringForNameOrDefault("direction", "DIR_UNKNOWN");
            PortDirection::Direction portDir = PortDirection::parseFromString(dirStr.c_str());
            port = new TypedPort<PortClass>(nodeLib.nextPortID(), node, new MetaPort(portName, portType, portDir),value,Port::OWN_META_PORT_BIT);
            node->addDynamicPort(port);
        }
        else
        {
            // Existing port but type mismatch.
            return;
        }
    }

    void Graph::readPort(dagbase::Table &portTable, Node* node, Port* existingPort)
    {
        std::string portClass = portTable.stringForNameOrDefault("class", "<unknown>");

        if (portClass == "TypedPort<double>")
        {
            readTypedPort<double>(*_nodeLib, portTable, node, existingPort, portTable.numberForNameOrDefault("value", 0.0));
        }
        else if (portClass == "TypedPort<int64_t>")
        {
            readTypedPort<std::int64_t>(*_nodeLib, portTable, node, existingPort, portTable.integerForNameOrDefault("value", 0));
        }
        else if (portClass == "TypedPort<string>")
        {
            readTypedPort<std::string>(*_nodeLib, portTable, node, existingPort, portTable.stringForNameOrDefault("value", ""));
        }
        else if (portClass == "TypedPort<bool>")
        {
            readTypedPort<bool>(*_nodeLib, portTable, node, existingPort, portTable.booleanForNameOrDefault("value", false));
        }
    }

    Graph* Graph::fromString(NodeLibrary& nodeLib, const char* str)
    {
        dagbase::Lua lua;

        lua.eval(str);

        if (!lua.tableExists("graph"))
        {
            return nullptr;
        }

        return fromLua(lua, nodeLib);
    }

    Graph::TopoSortResult Graph::topologicalSort(NodeArray *order)
    {
        if (order!=nullptr)
        {
            for (auto signalPath : _signalPaths)
            {
                signalPath.second->clearRemoved();
            }
            order->clear();
            std::queue<Node*> nodesWithNoDependencies;
            // Find the set of start nodes with no incoming edge.
            NodeArray allNodesIncludingChildren;
            findAllNodes(&allNodesIncludingChildren);
            for (auto n : allNodesIncludingChildren)
            {
                if (n->hasNoDependencies())
                {
                    nodesWithNoDependencies.push(n);
                }
            }
            while (!nodesWithNoDependencies.empty())
            {
                auto n = nodesWithNoDependencies.front();
                nodesWithNoDependencies.pop();
                order->push_back(n);
                std::vector<SignalPath*> allSignalPathsIncludingChildren;
                findAllSignalPaths(&allSignalPathsIncludingChildren);
                // for each node m with an edge e from n to m
                for (auto signalPath : allSignalPathsIncludingChildren)
                {
                    if (signalPath->sourceNode() == n)
                    {
                        auto m = signalPath->destNode();
                        signalPath->markRemoved();
                        if (signalPath->destNode()->numIncomingConnections()==1)
                        {
                            nodesWithNoDependencies.push(m);
                        }
                    }
                }
            }
            // If graph has edge
            //     there must be cycles
            if (hasEdges())
            {
                order->clear();
                return Graph::CYCLES_DETECTED;
            }
            // else
            //     OK
            // endif
        }
        return Graph::OK;
    }

    bool Graph::hasEdges() const
    {
        for (auto signalPath : _signalPaths)
        {
            if (!signalPath.second->isRemoved()) {
                return true;
            }
        }
        return false;
    }

    void Graph::addPort(Port *port)
    {
        if (port != nullptr)
        {
            _ports.insert(PortMap::value_type(port->id(), port));
        }
    }

    void Graph::removeSignalPath(SignalPath *signalPath)
    {
        if (signalPath != nullptr)
        {
            if (auto it=_signalPaths.find(signalPath->id()); it!=_signalPaths.end())
            {
                _signalPaths.erase(it);
            }
        }
        delete signalPath;
    }

    void Graph::removeNode(Node *node)
    {
        if (node != nullptr)
        {
            if (auto it=_nodes.find(node->id()); it!=_nodes.end())
            {
                _nodes.erase(it);
            }
            if (auto it=_nodeLookupByName.find(node->name()); it!=_nodeLookupByName.end())
            {
                _nodeLookupByName.erase(it);
            }
        }
    }

    Node* Graph::createNode(const std::string &className, const std::string &name)
    {
        if (_nodeLib!=nullptr)
        {
            return _nodeLib->instantiateNode(nextNodeID(), className, name);
        }
        return nullptr;
    }

    std::size_t Graph::numChildrenRecursive() const
    {
        std::size_t total = numChildren();

        for (auto child : _children)
        {
            total += child->numChildrenRecursive();
        }

        return total;
    }

    dagbase::ByteBuffer *Graph::save() const
    {
        return nullptr;
    }

    void Graph::restore(dagbase::ByteBuffer *memento)
    {
        // Do nothing.
    }

    dagbase::OutputStream& Graph::write(dagbase::OutputStream& str) const
    {
        str.write(_nodes.size());
        for (auto p : _nodes)
        {
            if (str.writeRef(p.second))
            {
                std::string className = p.second->className();
                str.write(className);
                p.second->write(str);
            }
        }

        str.write(_signalPaths.size());
        for (auto p : _signalPaths)
        {
            if (str.writeRef(p.second))
            {
                p.second->write(str);
            }
        }
        str.write(_children.size());
        for (auto child : _children)
        {
            if (str.writeRef(child))
            {
                child->write(str);
            }
        }
        return str;
    }

    Graph::Graph(dagbase::InputStream &str, NodeLibrary& nodeLib)
    :
    _nodeLib(&nodeLib)
    {
        str.addObj(this);
        size_t numNodes=0;
        str.read(&numNodes);
        if (_nodeLib!=nullptr)
        {
            for (auto i=0; i<numNodes; ++i)
            {
                auto node = str.readRef<Node>("Node", *_nodeLib);
                //auto node = _nodeLib->instantiateNode(str);

                if (node!=nullptr)
                {
                    addNode(node);
                }
            }
        }
        size_t numSignalPaths=0;
        str.read(&numSignalPaths);
        for (auto i=0; i<numSignalPaths; ++i)
        {
            dagbase::Stream::ObjId id = 0;
            SignalPath* p = nullptr;
            auto ref = str.readRef(&id);
            if (id != 0)
            {
                if (ref != nullptr)
                {
                    p = static_cast<SignalPath*>(ref);
                }
                else
                {
                    p = new SignalPath(str, nodeLib);
                }
            }
            if (p != nullptr)
            {
                addSignalPath(p);
            }
        }
        size_t numChildren = 0;
        str.read(&numChildren);
        for (auto i=0; i<numChildren; ++i)
        {
            dagbase::Stream::ObjId id = 0;
            Graph* child = nullptr;
            auto ref = str.readRef(&id);
            if (id != 0)
            {
                if (ref != nullptr)
                {
                    child = static_cast<Graph*>(ref);
                }
                else
                {
                    child = new Graph(str, nodeLib);
                }
            }
            if (child != nullptr)
            {
                addChild(child);
            }
        }
    }

    bool Graph::operator==(const Graph &other) const
    {
        if (_nodes.size() != other._nodes.size())
        {
            return false;
        }
        for (auto it=_nodes.begin(); it!=_nodes.end(); ++it)
        {
            auto it2 = other._nodes.find(it->first);
            if (it2 == other._nodes.end() || !it->second->equals(*(it2->second)))
            {
                return false;
            }
        }

        if (numChildren()!=other.numChildren())
        {
            return false;
        }

        for (auto it=_children.begin(); it!=_children.end(); ++it)
        {
            auto it2 = other._children.begin() + std::distance(_children.begin(),it);

            if (!(*it)->operator==(*(*it2)))
            {
                return false;
            }
        }

        return true;
    }

    void Graph::debug() const
    {
        DebugPrinter printer;
        printer.setStr(&std::cout);
        printer.println("{");
        printer.indent();
        for (auto p : _nodes)
        {
            p.second->debug(printer);
        }
        printer.outdent();
        printer.println("}");
    }

    std::ostream& Graph::toLua(std::ostream& str)
    {
        str << "graph = ";
        toLuaHelper(str);

        return str;
    }

    std::ostream &Graph::toLuaHelper(std::ostream &str)
    {
        str << "{";
        str << "nodes = {";
        for (auto node : _nodes)
        {
            node.second->toLua(str);
            str << ", ";
        }

        str << "},";

        str << "signalpaths = {";

        for (auto sp : _signalPaths)
        {
            sp.second->toLua(str);
        }
        str << "},";
        str << "children = {";

        for (auto child : _children)
        {
            child->toLuaHelper(str);
        }
        str << "}";
        str << "}";

        return str;
    }

    Graph *Graph::fromFile(NodeLibrary &nodeLib, const char *filename)
    {
        dagbase::Lua lua;

        lua.execute(filename);

        if (!lua.ok())
        {
            return nullptr;
        }

        return fromLua(lua, nodeLib);
    }

    Graph *Graph::fromLua(dagbase::Lua &lua, NodeLibrary& nodeLib)
    {
        auto output = new Graph();
        output->setNodeLibrary(&nodeLib);

        {
            dagbase::Table graphTable = lua.tableForName("graph");

            output = fromLuaGraphTable(graphTable, nodeLib, output);
        }
        return output;
    }

    Graph* Graph::fromLuaGraphTable(dagbase::Table& graphTable, NodeLibrary& nodeLib, Graph* output)
    {
        std::map<std::string, Node*> nodes;
        if (auto hasNodes = graphTable.isTable("nodes"); hasNodes)
        {
            dagbase::Table nodesTable = graphTable.tableForName("nodes");

            for (int i = 1; i <= nodesTable.length(); ++i)
            {
                dagbase::Table nodeTable = nodesTable.tableForIndex(i);

                NodeID id = nodeTable.integerForNameOrDefault("id", -1);
                std::string className = nodeTable.stringForNameOrDefault("class", "NotFound");
                std::string name = nodeTable.stringForNameOrDefault("name", "<unnamed>");
                Node* node = nodeLib.instantiateNode(id, className, name);

                if (node != nullptr)
                {
                    std::size_t numNodesBefore = output->numNodes();
                    output->addNode(node);
                    if (output->numNodes() != numNodesBefore + 1)
                    {
                        delete node;
                        delete output;
                        return nullptr;
                    }
                    nodes.insert(std::map<std::string, Node*>::value_type(name, node));
                    {
                        dagbase::Table portsTable = nodeTable.tableForName("ports");

                        for (int portIndex = 1; portIndex <= portsTable.length(); ++portIndex)
                        {
                            dagbase::Table portTable = portsTable.tableForIndex(portIndex);

                            output->readPort(portTable, node, node->dynamicPort(portIndex - 1));
                        }
                    }
                }
            }
        }
        if (auto hasChildren = graphTable.isTable("children"); hasChildren)
        {
            dagbase::Table childrenTable = graphTable.tableForName("children");

            for (auto i = 1; i <= childrenTable.length(); ++i)
            {
                dagbase::Table childTable = childrenTable.tableForIndex(i);

                auto* childGraph = new Graph();
                childGraph->setNodeLibrary(&nodeLib);

                childGraph = fromLuaGraphTable(childTable, nodeLib, childGraph);
                output->addChild(childGraph);
            }
        }
        if (auto hasSignalPaths = graphTable.isTable("signalpaths"); hasSignalPaths)
        {
            dagbase::Table signalPathsTable = graphTable.tableForName("signalpaths");

            for (int i = 1; i <= signalPathsTable.length(); ++i)
            {
                dagbase::Table signalPathTable = signalPathsTable.tableForIndex(i);

                Port* sourcePort = nullptr;
                Port* destPort = nullptr;
                std::string sourceNodeID = signalPathTable.stringForNameOrDefault("sourceNode", "");
                size_t sourcePortIndex = signalPathTable.integerForNameOrDefault("sourcePort", 0);
                std::string destNodeID = signalPathTable.stringForNameOrDefault("destNode", "");
                size_t destPortIndex = signalPathTable.integerForNameOrDefault("destPort", 0);
                // Look up sourceNode and destNode
                auto sourceNode = output->findNode(sourceNodeID);
                if (sourceNode != nullptr)
                {
                    sourcePort = sourceNode->dynamicPort(sourcePortIndex);
                }
                auto destNode = output->findNode(destNodeID);
                if (destNode != nullptr)
                {
                    destPort = destNode->dynamicPort(destPortIndex);
                }
                if (sourcePort != nullptr && destPort != nullptr)
                {
                    auto* signalPath = new SignalPath(sourcePort, destPort);
                    sourcePort->addOutgoingConnection(destPort);
                    destPort->addIncomingConnection(sourcePort);
                    output->addSignalPath(signalPath);
                }
            }
        }

        return output;
    }

    void Graph::findAllNodes(NodeArray *nodes)
    {
        if (nodes != nullptr)
        {
            for (auto p : _nodes)
            {
                nodes->push_back(p.second);
            }
            for (auto child : _children)
            {
                child->findAllNodes(nodes);
            }
        }
    }

    //! Find a node by its path from the root of the form foo.child[n].child[m]...
    Node *Graph::findNode(const std::string& path)
    {
        // Simple case:The path is the name of a node in this Graph.
        if (auto it=_nodeLookupByName.find(path); it!=_nodeLookupByName.end())
        {
            return it->second;
        }
        if (auto pos = path.find("child["); pos==0)
        {
            std::string indexStr;
            std::string rest = path.substr(pos+6);
            pos = 0;
            while (pos<rest.length() && std::isdigit(rest[pos]))
            {
                indexStr += rest[pos];
                ++pos;
            }
            // We know we have a number of digits, so atoi should do.
            size_t index = std::atoi(indexStr.c_str());
            auto child = _children[index];
            if (pos+1<rest.length() && rest[pos] == ']' && rest[pos+1]=='.')
            {
                rest = rest.substr(pos+2);
                return child->findNode(rest);
            }
        }
        if (auto pos = path.find('.'); pos!=std::string::npos)
        {
            std::string rest = path.substr(pos);
            return findNode(rest);
        }

        return nullptr;
    }

    //! Find all SignalPaths recursively including child Graphs.
    void Graph::findAllSignalPaths(std::vector<SignalPath *> *allSignalPaths)
    {
        if (allSignalPaths!=nullptr)
        {
            for (auto p : _signalPaths)
            {
                allSignalPaths->push_back(p.second);
            }
            for (auto child : _children)
            {
                child->findAllSignalPaths(allSignalPaths);
            }
        }
    }

    void Graph::evaluate(const NodeArray& order)
    {
        for (auto n : order)
        {
            for (size_t i=0; i<n->totalPorts(); ++i)
            {
                n->update();
                dag::ValueVisitor visitor;
                n->dynamicPort(i)->accept(visitor);
                dag::SetValueVisitor setVisitor(visitor.value());
                for (auto o : n->dynamicPort(i)->outgoingConnections())
                {
                    o->accept(setVisitor);
                }
            }
        }

    }

    Graph *Graph::clone(CloningFacility& facility, CopyOp copyOp, KeyGenerator* keyGen)
    {
        return new Graph(*this, facility, copyOp, keyGen);
    }

    Graph::Graph(const Graph & other, CloningFacility& facility, CopyOp copyOp, KeyGenerator* keyGen)
    {
        for (auto it=other._nodes.begin(); it!=other._nodes.end(); ++it)
        {
            std::uint64_t nodeId = 0;

            Node* copy = nullptr;

            if (facility.putOrig(it->second,&nodeId))
            {
                copy = it->second->clone(facility, copyOp, keyGen);
            }
            else
            {
                copy = static_cast<Node*>(facility.getClone(nodeId));
            }

            if (copy!=nullptr)
            {
                _nodes.insert(NodeMap::value_type (copy->id(), copy));
            }
        }

        for (auto it=other._children.begin(); it!=other._children.end(); ++it)
        {
            Graph* copy = (*it)->clone(facility, copyOp, keyGen);

            if (copy != nullptr)
            {
                _children.push_back(copy);
            }
        }
    }
}
