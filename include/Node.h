#pragma once

#include "config/Export.h"

#include "NodeCategory.h"
#include "Port.h"
#include "PortDescriptor.h"
#include "MetaPort.h"

#include <string>
#include <vector>
#include <google/protobuf/extension_set.h>

namespace nbe
{
    class DebugPrinter;
    class InputStream;
    class KeyGenerator;
	struct NodeDescriptor;
    class OutputStream;
    class NodeLibrary;

	class NODEBACKEND_API Node
	{
	public:
		enum NodeFlags : std::uint32_t
		{
			NODE_INPUT_BIT		= 1<<0,
			NODE_OUTPUT_BIT		= 1<<1,
			NODE_INTERNAL_BIT	= 1<<2
		};
	public:
		explicit Node(KeyGenerator& keyGen, std::string name, NodeCategory::Category category=NodeCategory::CAT_NONE);

        explicit Node(InputStream& str, NodeLibrary& nodeLib);

		Node(const Node& other);

		Node(Node&&) = default;

		virtual ~Node() = default;

		Node& operator=(const Node&) = default;

		Node& operator=(Node&&) = default;

        //! Compare for equality with another Node.
        //! \note Typically downcasts to a concrete type to determine a result.
        [[nodiscard]]virtual bool equals(const Node& other) const = 0;

        //! \return The name of the class without SWIG mangling
        [[nodiscard]] virtual const char* className() const = 0;

        //! Describe ourself
        virtual void describe(NodeDescriptor& descriptor) const = 0;

        //! \return A MetaPort corresponding to a given index.
        //! \param[in] The index of the port, zero-based.
        [[nodiscard]]virtual const MetaPort * dynamicMetaPort(size_t index) const = 0;

        //! \return A Port corresponding to a given index
        //! \note The index includes both built-in and dynamically added Ports.
        //! \param[in] index The index of the Port, zero-based.
        virtual Port* dynamicPort(size_t index) = 0;

        //! Create a Node of the same type as this from a stream.
        //! \param[in] str The stream from which to read the data required to create the Node.
        //! \param[in] nodeLib The NodeLibrary to create Ports
        virtual Node* create(InputStream& str, NodeLibrary& nodeLib) = 0;

        //! Write ourself to a stream
        //! \param[in] str The stream
        virtual OutputStream& write(OutputStream& str) const = 0;

        //! Base class implementation of operator==().
        //! \note Typically called by a concrete implementation of equals().
        bool operator==(const Node& other) const;

        //! Clone ourself to support the Prototype pattern
        //! \note A deep copy of Ports is required.
        virtual Node* clone() = 0;


        void setId(NodeID id)
        {
            _id = id;
        }

		[[nodiscard]] NodeID id() const
        {
            return _id;
        }


		void setName(const std::string& name)
		{
			_name = name;
		}

		[[nodiscard]]const std::string& name() const
		{
			return _name;
		}

        [[nodiscard]]NodeCategory::Category category() const
		{
			return _category;
		}

		virtual void addDynamicPort(Port* port)
		{
			throw std::runtime_error("addDynamicPort():Not implemented for " + std::string(className()));
		}

		[[nodiscard]]virtual size_t totalPorts() const
		{
			return size_t{ 0 };
		}

        [[nodiscard]]size_t indexOfPort(Port* port)
        {
            for (auto i=0; i<totalPorts(); ++i)
            {
                if (dynamicPort(i)==port)
                {
                    return i;
                }
            }
            return size_t{~0ULL};
        }

        void reconnectInputs(NodeSet const& selection, Node* newSource);

        void reconnectOutputs(NodeSet const& NodeSet, Node* newDest);
        
        [[nodiscard]]bool hasNoDependencies()
        {
            for (size_t index=0; index<totalPorts(); ++index)
            {
                if (!dynamicPort(index)->hasNoDependencies())
                {
                    return false;
                }
            }

            return true;
        }

        [[nodiscard]]size_t numIncomingConnections()
        {
            size_t total = 0;
            for (size_t portIndex=0; portIndex<totalPorts(); ++portIndex)
            {
                total += dynamicPort(portIndex)->numIncomingConnections();
            }

            return total;
        }

        [[nodiscard]]bool hasInputs()
        {
            for (auto index=0; index<totalPorts(); ++index)
            {
                auto p = dynamicPort(index);

                if (p->dir() == PortDirection::DIR_IN)
                {
                    return true;
                }
            }

            return false;
        }

        [[nodiscard]]bool hasOutputs()
        {
            for (auto index=0; index<totalPorts(); ++index)
            {
                auto p = dynamicPort(index);

                if (p->dir() == PortDirection::DIR_OUT)
                {
                    return true;
                }
            }

            return false;
        }

        static void reset()
        {
            // Do nothing.
        }

        virtual void debug(DebugPrinter& printer) const;

        virtual std::ostream& toLua(std::ostream& str);
	private:
        NodeID _id{-1};
		std::string _name;
		NodeCategory::Category _category{NodeCategory::CAT_UNKNOWN};
		typedef std::vector<PortDescriptor> PortDescriptorArray;
		PortDescriptorArray _dynamicPortDescriptors;
		void addDynamicPortDescriptor(PortDescriptor const& descriptor)
		{
			_dynamicPortDescriptors.push_back(descriptor);
		}
		NodeFlags _flags{ 0x0 };
	};
}
