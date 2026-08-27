//
// Created by tony on 04/05/24.
//

#pragma once

#include "config/Export.h"

#include "core/Node.h"
#include "core/TypedPort.h"
#include "core/KeyGenerator.h"

namespace dag
{
    class DAG_API MathsNode : public dagbase::Node
    {
    public:
        enum MathsPort : std::uint32_t
        {
            PORT_ANGLE,
            PORT_UNIT,
            PORT_OUTPUT
        };
    public:
        MathsNode(dagbase::KeyGenerator& keyGen, const std::string& name, dagbase::NodeCategory::Category category)
        :
        Node(keyGen, name, category)
        {
            addDynamicPort(new dagbase::TypedPort<double>(keyGen.nextPortID(), this, "angle", dagbase::PortType::TYPE_DOUBLE, dagbase::PortDirection::DIR_IN, 0.0), dagbase::MetaPort::FLAGS_OWN_BIT);
            addDynamicPort(new dagbase::TypedPort<std::int64_t>(keyGen.nextPortID(), this, "unit", dagbase::PortType::TYPE_INT64, dagbase::PortDirection::DIR_INTERNAL, 0), dagbase::MetaPort::FLAGS_OWN_BIT);
            addDynamicPort(new dagbase::TypedPort<double>(keyGen.nextPortID(), this, "output", dagbase::PortType::TYPE_DOUBLE, dagbase::PortDirection::DIR_OUT, 0.0), dagbase::MetaPort::FLAGS_OWN_BIT);
        }

        MathsNode(dagbase::InputStream& str, dagbase::NodeLibrary& nodeLib, dagbase::Lua &lua);

        MathsNode(const MathsNode& other, dagbase::CloningFacility& facility, dagbase::CopyOp copyOp, dagbase::KeyGenerator* keyGen);

        ~MathsNode() override;

        //! Compare for equality with another Node.
        //! \note Typically downcasts to a concrete type to determine a result.
        [[nodiscard]]bool equals(const Node& other, dagbase::ComparisonFlags flags) const override;

        //! \return The name of the class without SWIG mangling
        [[nodiscard]]const char* className() const override;

        //! Create a Node of the same type as this from a stream.
        //! \param[in] str The stream from which to read the data required to create the Node.
        //! \param[in] nodeLib The NodeLibrary to create Ports
        //! \param lua
        Node* create(dagbase::InputStream& str, dagbase::NodeLibrary& nodeLib, dagbase::Lua &lua) override;

        //! Write ourself to a stream
        //! \param[in] str The stream
        //! \param lua
        dagbase::OutputStream& writeToStream(dagbase::OutputStream& str, dagbase::NodeLibrary& nodeLib, dagbase::Lua &lua) const override;

        //! Clone ourself to support the Prototype pattern
        //! \note A deep copy of Ports is required.
        Node* clone(dagbase::CloningFacility& facility, dagbase::CopyOp copyOp, dagbase::KeyGenerator* keyGen) override
        {
            return new MathsNode(*this, facility, copyOp, keyGen);
        }

        void update() override;
    protected:
        static std::array<dagbase::MetaPort, 3> ports;
        static constexpr size_t firstPort = 0;
        static constexpr size_t numPorts = 3;
    };


}