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

namespace nbe
{
    class ByteBuffer;
    class InputStream;
    class Lua;
	class Node;
    class NodeLibrary;
    class OutputStream;
    class Port;
    class SelectionInterface;
	class SignalPath;
    class Table;
    typedef std::vector<Node*> NodeArray;

	class NBE_API Graph : public KeyGenerator
	{
    public:
	public:
		Graph() = default;

        explicit Graph(InputStream& str, NodeLibrary& nodeLib);

		Graph(const Graph&) = default;

		Graph(Graph&&) = default;

		~Graph() override;

		Graph& operator=(const Graph&) = default;

		Graph& operator=(Graph&&) = default;

        bool operator==(const Graph& other) const;

        [[nodiscard]]Graph* parent()
        {
            return _parent;
        }

		[[nodiscard]] size_t numNodes() const
		{
			return _nodes.size();
		}

		void addNode(Node* node);

		Node* lastAddedNode()
		{
			return _lastAddedNode;
		}

        void removeNode(Node* node);

        void setNodeLibrary(NodeLibrary* nodeLib)
        {
            if (nodeLib!=nullptr)
            {
                _nodeLib = nodeLib;
            }
        }

        Node* createNode(std::string const& className, std::string const& name);

		Node* node(NodeID id)
		{
			if (auto const it = _nodes.find(id); it!=_nodes.end())
			{
				return it->second;
			}
			return nullptr;
		}

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

        void addPort(Port* port);

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

		static Graph* fromString(NodeLibrary& nodeLib, const char* str);

        static Graph* fromFile(NodeLibrary& nodeLib, const char* filename);

        std::ostream& toLua(std::ostream& str);

        enum TopoSortResult
        {
            OK,
            CYCLES_DETECTED
        };

        TopoSortResult topologicalSort(NodeArray* order);

        bool hasEdges() const;

        void findAllNodes(NodeArray* nodes);

        Node* findNode(const std::string& path);

        void findAllSignalPaths(std::vector<SignalPath*>* allSignalPaths);

		void addChild(Graph* child)
		{
			if (child != nullptr)
			{
                child->setParent(this);
				_children.push_back(child);
			}
		}

		void removeChild(Graph* child)
		{
			if (auto it = std::find(_children.begin(), _children.end(), child); it != _children.end())
			{
                child->setParent(nullptr);
				_children.erase(it);
			}
		}

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

        std::size_t numChildrenRecursive() const;

        ByteBuffer* save() const;

        void restore(ByteBuffer* memento);

        OutputStream& write(OutputStream& str) const;

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

        void readPort(Table &portTable, Node *node, Port *existingPort);
        std::ostream& toLuaHelper(std::ostream & str);
        static Graph* fromLua(Lua& lua, NodeLibrary& nodeLib);
        static Graph* fromLuaGraphTable(Table& graphTable, NodeLibrary& nodeLib, Graph* output);
    };
}
