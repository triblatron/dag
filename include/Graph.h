#pragma once

#include "config/Export.h"

#include "Types.h"
#include "KeyGenerator.h"

#include <unordered_map>
#include <vector>
#include <cstdlib>
#include <cstdint>
#include <algorithm>
#include <functional>
#include <string>
#include <iterator>

namespace dagbase
{
    class ByteBuffer;
    class InputStream;
	class Lua;
    class OutputStream;
	class Table;
}

namespace dag
{
    class CloningFacility;
	class Node;
    class NodeLibrary;
    class Port;
    class SelectionInterface;
	class SignalPath;
    typedef std::vector<Node*> NodeArray;

    //! A collection of Nodes, SignalPaths and child Graphs.
	class DAG_API Graph : public KeyGenerator
	{
	public:
		Graph() = default;

        Graph(dagbase::InputStream& str, NodeLibrary& nodeLib);

		Graph(const Graph& other, CloningFacility& facility, CopyOp copyOp, KeyGenerator* keyGen);

		Graph(Graph&&) = default;

		~Graph() override;

		Graph& operator=(const Graph&) = default;

		Graph& operator=(Graph&&) = default;

        bool operator==(const Graph& other) const;

        Graph* clone(CloningFacility& facility, CopyOp copyOp, KeyGenerator* keyGen);

        [[nodiscard]]Graph* parent()
        {
            return _parent;
        }

		[[nodiscard]] size_t numNodes() const
		{
			return _nodes.size();
		}

        //! Add a node if it is non-null, does nothing otherwise.
		void addNode(Node* node);

		Node* lastAddedNode()
		{
			return _lastAddedNode;
		}

        //! Remove a non-null node.
        void removeNode(Node* node);

        void setNodeLibrary(NodeLibrary* nodeLib)
        {
            if (nodeLib!=nullptr)
            {
                _nodeLib = nodeLib;
            }
        }

        //! Create a node of the supplied class if it exists.
        //! \throw runtime_error if the class does not exist
        //! \note Does not add the Node to the Graph.
        Node* createNode(std::string const& className, std::string const& name);

        //! \retval nullptr if a Node with the specified ID does not exist.
		Node* node(NodeID id)
		{
			if (auto const it = _nodes.find(id); it!=_nodes.end())
			{
				return it->second;
			}
			return nullptr;
		}

        //! Operate on each Node of this Graph without descending into child Graphs.
        template<typename F>
        void eachNode(F f)
        {
            for (auto p : _nodes)
            {
                if (!std::invoke(f,p.second))
                {
                    return;
                }
            }
        }

        //! Add a non-null port.
        void addPort(Port* port);

        //! \retval nullptr If a Port with the specified ID does not exist.
        Port* port(PortID id)
        {
            if (auto const it=_ports.find(id); it!=_ports.end())
            {
                return it->second;
            }

            return nullptr;
        }

		[[nodiscard]] size_t numSignalPaths() const
		{
			return _signalPaths.size();
		}

        //! Add a non-null SignalPath
		void addSignalPath(SignalPath* signalPath);

        void removeSignalPath(SignalPath* signalPath);

		[[nodiscard]] SignalPath* signalPath(SignalPathID id)
		{
			if (auto const it=_signalPaths.find(id); it!=_signalPaths.end())
			{
				return it->second;
			}

			return nullptr;
		}

        //! Create a Graph from a Lua string representation.
        //! \retval nullptr if there is a syntax error.
		static Graph* fromString(NodeLibrary& nodeLib, const char* str);

        //! Create a Graph from a Lua representation in a file.
        //! \retval nullptr if the file could not be found or there is a syntax error.
        static Graph* fromFile(NodeLibrary& nodeLib, const char* filename);

        std::ostream& toLua(std::ostream& str);

        enum TopoSortResult
        {
            //! The sort completed successfully.
            OK,
            //! One or more cycles were found, preventing a valid sort order.
            CYCLES_DETECTED
        };

        //! Perform a topological sort of this Graph.
        //! \return A valid order if successful, undefined otherwise.
        TopoSortResult topologicalSort(NodeArray* order);

        //! Evaluate the nodes in this Graph using the given order.
        void evaluate(const NodeArray& order);

        bool hasEdges() const;

        //! Find all Nodes in this Graph and its children.
        void findAllNodes(NodeArray* nodes);

        Node* findNode(const std::string& path);

        void findAllSignalPaths(std::vector<SignalPath*>* allSignalPaths);

        //! Add a non-null child Graph, setting its parent to be this Graph.
		void addChild(Graph* child)
		{
			if (child != nullptr)
			{
                child->setParent(this);
				_children.emplace_back(child);
			}
		}

        //! Remove a non-null child if it exists.
		void removeChild(Graph* child)
		{
			if (auto it = std::find(_children.begin(), _children.end(), child); it != _children.end())
			{
                child->setParent(nullptr);
				_children.erase(it);
			}
		}

        //! \retval nullptr if index >= numChildren()
		Graph* child(std::size_t index)
		{
			if (index < _children.size())
			{
				return _children[index];
			}

			return nullptr;
		}

        std::size_t numChildren() const
        {
            return _children.size();
        }

        //! \return The number of children searching recursively.
        std::size_t numChildrenRecursive() const;

        //! Save this Graph to a buffer.
        dagbase::ByteBuffer* save() const;

        //! Rstore this Graph from a buffer.
        void restore(dagbase::ByteBuffer* memento);

        //! Write this Graph to a stream.
        dagbase::OutputStream& write(dagbase::OutputStream& str) const;

        //! Pretty-print this Graph for debugging purposes.
        void debug() const;

        NodeID nextNodeID() override
        {
            return _nextNodeID++;
        }

        PortID nextPortID() override
        {
            return _nextPortID++;
        }
    private:
        void setParent(Graph* parent)
        {
            _parent = parent;
        }
		typedef std::unordered_map<std::int64_t, Node*> NodeMap;
		NodeMap _nodes;
        typedef std::unordered_map<std::string, Node*> NameToNodeMap;
        NameToNodeMap _nodeLookupByName;
		typedef std::unordered_map<std::int64_t, SignalPath*> SignalPathMap;
		SignalPathMap _signalPaths;
        typedef std::unordered_map<std::int64_t, Port*> PortMap;
        PortMap _ports;
		typedef std::vector<Graph*> GraphArray;
		GraphArray _children;
        Graph* _parent{nullptr};
        NodeLibrary* _nodeLib{nullptr};
		Node* _lastAddedNode{ nullptr };
        NodeID _nextNodeID{0};
        PortID _nextPortID{0};

        void readPort(dagbase::Table &portTable, Node *node, Port *existingPort);
        std::ostream& toLuaHelper(std::ostream & str);
        static Graph* fromLua(dagbase::Lua& lua, NodeLibrary& nodeLib);
        static Graph* fromLuaGraphTable(dagbase::Table& graphTable, NodeLibrary& nodeLib, Graph* output);
    };
}
