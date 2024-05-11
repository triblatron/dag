#pragma once

#include "config/Export.h"


#include "Types.h"
#include "Value.h"

#include <string>
#include <array>

namespace nbe
{
    class InputStream;
    class Node;
    class OutputStream;

    //! |class NodeLibrary
    //! A set of Nodes that can be instantiated by class name
    //! \note Uses the Prototype pattern.
	class NBE_API NodeLibrary
	{
	public:
        NodeLibrary() = default;

		virtual ~NodeLibrary() = default;

        NodeLibrary(const NodeLibrary& other) = default;

        NodeLibrary(NodeLibrary&& other) = default;

        NodeLibrary& operator=(const NodeLibrary& other) = default;

        NodeLibrary& operator=(NodeLibrary&& other) = default;

        //! Used by plugins to register a new node type.
        virtual void registerNode(Node* node) = 0;

        [[nodiscard]]virtual std::size_t numNodes() const = 0;

        //! Create a new Node from an internal copy of the type given by className, named name.
        //! \throw runtime_error if the node class cannot be found.
		virtual Node* instantiateNode(NodeID id, const std::string& className, const std::string& name) = 0;

        //! Write the given node to the stream.
        virtual OutputStream& write(OutputStream& str, Node* node) const = 0;

        //! Create a node from a stream.
        virtual Node* instantiateNode(InputStream& str) = 0;

        virtual Port* instantiatePort(const std::string& className, const std::string& name, PortType::Type type, PortDirection::Direction, Value value) = 0;

        virtual Port* instantiatePort(InputStream& str) = 0;

//        virtual NodeID nextNodeID() = 0;

        virtual PortID nextPortID() = 0;
	};
}
