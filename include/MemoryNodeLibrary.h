#pragma once

#include "config/Export.h"

#include "core/NodeLibrary.h"
#include "core/KeyGenerator.h"
#include "core/LuaInterface.h"

#include <string>
#include <unordered_map>

namespace dag
{
	class DAG_API MemoryNodeLibrary final : public dagbase::NodeLibrary, public dagbase::KeyGenerator
	{
	public:
		MemoryNodeLibrary();

		MemoryNodeLibrary(const MemoryNodeLibrary& other) = default;

		MemoryNodeLibrary(MemoryNodeLibrary&& other) = default;

		MemoryNodeLibrary& operator=(const MemoryNodeLibrary& other) = default;

		MemoryNodeLibrary& operator=(MemoryNodeLibrary&& other) = default;

		~MemoryNodeLibrary() override;

        void registerNode(dagbase::Node* node) override;

        [[nodiscard]]std::size_t numNodes() const override
        {
            return _classes.size();
        }

		dagbase::Node* instantiateNode(dagbase::NodeID id, const std::string& className, const std::string& name) override;

        dagbase::OutputStream& write(dagbase::OutputStream& str, dagbase::Node* node) const override;

        //! \note Delegates to Node::create() to get the exact type of the node.
        dagbase::Node* instantiateNode(dagbase::InputStream& str, dagbase::Lua& lua) override;

        dagbase::Port* instantiatePort(const std::string& className, const std::string& name, dagbase::PortType::Type type, dagbase::PortDirection::Direction, dagbase::Value value) override;

        dagbase::Port* instantiatePort(dagbase::InputStream& str, dagbase::Lua &lua) override;

		dagbase::Class* instantiate(const char* baseClassName, dagbase::InputStream& str, dagbase::Lua& lua) override;

		dagbase::NodeID nextNodeID() override
		{
			return _nextNodeID++;
		}

		dagbase::PortID nextPortID() override
		{
			return _nextPortID++;
		}
	private:
		typedef std::unordered_map<std::string, dagbase::Node*> PrototypeMap;
		PrototypeMap _classes;
		dagbase::NodeID _nextNodeID{ 0 };
		dagbase::PortID _nextPortID{ 0 };
	};
}
