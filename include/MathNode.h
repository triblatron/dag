//
// Created by tony on 04/05/24.
//

#pragma once

#include "config/Export.h"

#include "Node.h"
#include "TypedPort.h"
#include "KeyGenerator.h"

namespace nbe
{
    class NBE_API MathsNode : public Node
    {
    public:
        MathsNode(KeyGenerator& keyGen, const std::string& name, NodeCategory::Category category)
        :
        Node(keyGen, name, category)
        {
            _angle = new TypedPort<double>(keyGen.nextPortID(), this, &ports[0], 0.0);
            _unit = new TypedPort<std::int64_t>(keyGen.nextPortID(), this, &ports[1], 0);
            _output = new TypedPort<double>(keyGen.nextPortID(), this, &ports[2], 0.0);
        }

        MathsNode(InputStream& str, NodeLibrary& nodeLib);
        MathsNode(const MathsNode& other);

        //! Compare for equality with another Node.
        //! \note Typically downcasts to a concrete type to determine a result.
        [[nodiscard]]bool equals(const Node& other) const override;

        //! \return The name of the class without SWIG mangling
        [[nodiscard]]const char* className() const override;

        //! Describe ourself
        void describe(NodeDescriptor& descriptor) const override;

        //! \return A MetaPort corresponding to a given index.
        //! \param[in] The index of the port, zero-based.
        [[nodiscard]]const MetaPort * dynamicMetaPort(size_t index) const override;

        //! \return A Port corresponding to a given index
        //! \note The index includes both built-in and dynamically added Ports.
        //! \param[in] index The index of the Port, zero-based.
        Port* dynamicPort(size_t index) override;

        //! Create a Node of the same type as this from a stream.
        //! \param[in] str The stream from which to read the data required to create the Node.
        //! \param[in] nodeLib The NodeLibrary to create Ports
        Node* create(InputStream& str, NodeLibrary& nodeLib) override;

        //! Write ourself to a stream
        //! \param[in] str The stream
        OutputStream& write(OutputStream& str) const override;

        //! Clone ourself to support the Prototype pattern
        //! \note A deep copy of Ports is required.
        Node* clone() override
        {
            return new MathsNode(*this);
        }

        [[nodiscard]]size_t totalPorts() const override
        {
            return numPorts;
        }

        void update() override;
    protected:
        static std::array<MetaPort, 3> ports;
        static constexpr size_t firstPort = 0;
        static constexpr size_t numPorts = 3;
    private:
        TypedPort<double>* _angle{nullptr};
        TypedPort<std::int64_t>* _unit{nullptr};
        TypedPort<double>* _output{nullptr};
    };


}