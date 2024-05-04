#include "config/config.h"

#include "Graph.h"
#include "NodeLibrary.h"
#include "InputStream.h"
#include "Node.h"
#include "SignalPath.h"
#include "SelectionInterface.h"
#include "OutputStream.h"
#include "DebugPrinter.h"
#include "LuaInterface.h"
#include "TypedPort.h"
#include "Types.h"

#include <queue>

namespace nbe
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
    void readTypedPort(NodeLibrary& nodeLib, Table& portTable, Node* node, Port* existingPort, PortClass value)
    {
        auto* port = dynamic_cast<TypedPort<PortClass>*>(existingPort);

        if (port != nullptr)
        {
            port->setValue(value);
        }
        else if (existingPort == nullptr)
        {
            std::string portName = portTable.string("name", "<unnamed>");
            std::string portTypeStr = portTable.string("type", "TYPE_UNKNOWN");
            PortType::Type portType = PortType::parseFromString(portTypeStr.c_str());
            std::string dirStr = portTable.string("direction", "DIR_UNKNOWN");
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

    void Graph::readPort(Table &portTable, Node* node, Port* existingPort)
    {
        std::string portClass = portTable.string("class", "<unknown>");

        if (portClass == "TypedPort<double>")
        {
            readTypedPort<double>(*_nodeLib, portTable, node, existingPort, portTable.number("value", 0.0));
        }
        else if (portClass == "TypedPort<int64_t>")
        {
            readTypedPort<std::int64_t>(*_nodeLib, portTable, node, existingPort, portTable.integer("value", 0));
        }
        else if (portClass == "TypedPort<string>")
        {
            readTypedPort<std::string>(*_nodeLib, portTable, node, existingPort, portTable.string("value", ""));
        }
        else if (portClass == "TypedPort<bool>")
        {
            readTypedPort<bool>(*_nodeLib, portTable, node, existingPort, portTable.boolean("value", false));
        }
    }

    Graph* Graph::fromString(NodeLibrary& nodeLib, const char* str)
    {
        Lua lua;

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

    ByteBuffer *Graph::save() const
    {
        return nullptr;
    }

    void Graph::restore(ByteBuffer *memento)
    {
        // Do nothing.
    }

    OutputStream& Graph::write(OutputStream& str) const
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

    Graph::Graph(InputStream &str, NodeLibrary& nodeLib)
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
                Stream::ObjId id = 0;
                auto node = str.readRef<Node>(&id, *_nodeLib);
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
            Stream::ObjId id = 0;
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
            Stream::ObjId id = 0;
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
            if (!it->second->equals(*(it2->second)))
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
        Lua lua;

        lua.execute(filename);

        if (!lua.ok())
        {
            return nullptr;
        }

        return fromLua(lua, nodeLib);
    }

    Graph *Graph::fromLua(Lua &lua, NodeLibrary& nodeLib)
    {
        auto output = new Graph();
        output->setNodeLibrary(&nodeLib);

        {
            nbe::Table graphTable = lua.tableForName("graph");

            output = fromLuaGraphTable(graphTable, nodeLib, output);
        }
        return output;
    }

    Graph* Graph::fromLuaGraphTable(Table& graphTable, NodeLibrary& nodeLib, Graph* output)
    {
        std::map<std::string, Node*> nodes;
        if (auto hasNodes = graphTable.isTable("nodes"); hasNodes)
        {
            Table nodesTable = graphTable.tableForName("nodes");

            for (int i = 1; i <= nodesTable.length(); ++i)
            {
                Table nodeTable = nodesTable.tableForIndex(i);

                NodeID id = nodeTable.integer("id", -1);
                std::string className = nodeTable.string("class", "NotFound");
                std::string name = nodeTable.string("name", "<unnamed>");
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
                        Table portsTable = nodeTable.tableForName("ports");

                        for (int portIndex = 1; portIndex <= portsTable.length(); ++portIndex)
                        {
                            Table portTable = portsTable.tableForIndex(portIndex);

                            output->readPort(portTable, node, node->dynamicPort(portIndex - 1));
                        }
                    }
                }
            }
        }
        if (auto hasChildren = graphTable.isTable("children"); hasChildren)
        {
            Table childrenTable = graphTable.tableForName("children");

            for (auto i = 1; i <= childrenTable.length(); ++i)
            {
                Table childTable = childrenTable.tableForIndex(i);

                auto* childGraph = new Graph();
                childGraph->setNodeLibrary(&nodeLib);

                childGraph = fromLuaGraphTable(childTable, nodeLib, childGraph);
                output->addChild(childGraph);
            }
        }
        if (auto hasSignalPaths = graphTable.isTable("signalpaths"); hasSignalPaths)
        {
            Table signalPathsTable = graphTable.tableForName("signalpaths");

            for (int i = 1; i <= signalPathsTable.length(); ++i)
            {
                Table signalPathTable = signalPathsTable.tableForIndex(i);

                Port* sourcePort = nullptr;
                Port* destPort = nullptr;
                std::string sourceNodeID = signalPathTable.string("sourceNode", "");
                size_t sourcePortIndex = signalPathTable.integer("sourcePort", 0);
                std::string destNodeID = signalPathTable.string("destNode", "");
                size_t destPortIndex = signalPathTable.integer("destPort", 0);
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
                nbe::ValueVisitor visitor;
                n->dynamicPort(i)->accept(visitor);
                nbe::SetValueVisitor setVisitor(visitor.value());
                for (auto o : n->dynamicPort(i)->outgoingConnections())
                {
                    o->accept(setVisitor);
                }
            }
        }

    }
}
