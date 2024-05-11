#pragma once

#include "config/Export.h"

#include "NodeLibrary.h"

#include <string>
#include <unordered_map>
#include "KeyGenerator.h"

namespace nbe
{
	class NBE_API MemoryNodeLibrary final : public NodeLibrary, public KeyGenerator
	{
	public:
		MemoryNodeLibrary();

		MemoryNodeLibrary(const MemoryNodeLibrary& other) = default;

		MemoryNodeLibrary(MemoryNodeLibrary&& other) = default;

		MemoryNodeLibrary& operator=(const MemoryNodeLibrary& other) = default;

		MemoryNodeLibrary& operator=(MemoryNodeLibrary&& other) = default;

		~MemoryNodeLibrary() override;

        void registerNode(Node* node) override;

        [[nodiscard]]std::size_t numNodes() const override
        {
            return _classes.size();
        }

		Node* instantiateNode(NodeID id, const std::string& className, const std::string& name) override;

        OutputStream& write(OutputStream& str, Node* node) const override;

        //! \note Delegates to Node::create() to get the exact type of the node.
        Node* instantiateNode(InputStream& str) override;

        Port* instantiatePort(const std::string& className, const std::string& name, PortType::Type type, PortDirection::Direction, Value value) override;

        Port* instantiatePort(InputStream& str) override;

		NodeID nextNodeID() override
		{
			return _nextNodeID++;
		}

		PortID nextPortID() override
		{
			return _nextPortID++;
		}
	private:
		typedef std::unordered_map<std::string, Node*> PrototypeMap;
		PrototypeMap _classes;
		NodeID _nextNodeID{ 0 };
		PortID _nextPortID{ 0 };
	};
}
