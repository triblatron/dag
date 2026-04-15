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
        MathsNode(dagbase::KeyGenerator& keyGen, const std::string& name, dagbase::NodeCategory::Category category)
        :
        Node(keyGen, name, category)
        {
            _angle = new dagbase::TypedPort<double>(keyGen.nextPortID(), this, &ports[0], 0.0);
            _unit = new dagbase::TypedPort<std::int64_t>(keyGen.nextPortID(), this, &ports[1], 0);
            _output = new dagbase::TypedPort<double>(keyGen.nextPortID(), this, &ports[2], 0.0);
        }

        MathsNode(dagbase::InputStream& str, dagbase::NodeLibrary& nodeLib, dagbase::Lua &lua);

        MathsNode(const MathsNode& other, dagbase::CloningFacility& facility, dagbase::CopyOp copyOp, dagbase::KeyGenerator* keyGen);

        ~MathsNode() override;

        //! Compare for equality with another Node.
        //! \note Typically downcasts to a concrete type to determine a result.
        [[nodiscard]]bool equals(const Node& other) const override;

        //! \return The name of the class without SWIG mangling
        [[nodiscard]]const char* className() const override;

        //! Describe ourself
        void describeNode(dagbase::NodeDescriptor& descriptor) const override;

        //! \return A MetaPort corresponding to a given index.
        //! \param[in] The index of the port, zero-based.
        [[nodiscard]]const dagbase::MetaPort * dynamicMetaPort(size_t index) const override;

        //! \return A Port corresponding to a given index
        //! \note The index includes both built-in and dynamically added Ports.
        //! \param[in] index The index of the Port, zero-based.
        dagbase::Port* dynamicPort(size_t index) override;

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

        [[nodiscard]]size_t totalPorts() const override
        {
            return numPorts;
        }

        void update() override;
    protected:
        static std::array<dagbase::MetaPort, 3> ports;
        static constexpr size_t firstPort = 0;
        static constexpr size_t numPorts = 3;
    private:
        dagbase::TypedPort<double>* _angle{nullptr};
        dagbase::TypedPort<std::int64_t>* _unit{nullptr};
        dagbase::TypedPort<double>* _output{nullptr};
    };


}