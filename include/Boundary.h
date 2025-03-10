//
// Created by tony on 13/02/24.
//

#pragma  once

#include "config/Export.h"

#include "Node.h"

namespace dag
{
    class DAG_API Boundary final : public Node
    {
    public:
        explicit Boundary(KeyGenerator& keyGen, std::string name, NodeCategory::Category category=NodeCategory::CAT_NONE);

        Boundary(const Boundary& other, CloningFacility& facility, CopyOp copyOp, KeyGenerator* keyGen);

        Boundary(Boundary&& other);

        explicit Boundary(dagbase::InputStream& str, NodeLibrary& nodeLib);

        ~Boundary() override;

        Boundary& operator=(const Boundary& other);

        Boundary& operator=(Boundary&& other) = default;

        [[nodiscard]]const char* className() const override
        {
            return "Boundary";
        }

        void describe(NodeDescriptor& descriptor) const override;

        [[nodiscard]]MetaPort const* dynamicMetaPort(size_t index) const override
        {
            if (index<_dynamicMetaPorts.size())
            {
                return &_dynamicMetaPorts[index];
            }

            return nullptr;
        }

        Port* dynamicPort(size_t index) override
        {
            if (index<_dynamicPorts.size())
            {
                return _dynamicPorts[index];
            }

            return nullptr;
        }

        Boundary* clone(CloningFacility& facility, CopyOp copyOp, KeyGenerator* keyGen) override
        {
            return new Boundary(*this,facility,copyOp,keyGen);
        }

        Boundary* create(dagbase::InputStream& str, NodeLibrary& nodeLib) override;

        [[nodiscard]]bool equals(const Node& other) const override;

        dagbase::OutputStream& write(dagbase::OutputStream& str) const override;

        [[nodiscard]]size_t totalPorts() const override
        {
            return _dynamicPorts.size();
        }

        void addDynamicPort(Port* port) override
        {
            if (port != nullptr)
            {
                port->setParent(this);
                _dynamicPorts.push_back(port);
                MetaPort desc;
                desc.name = port->name();
                desc.type = port->type();
                desc.direction = port->dir();
                _dynamicMetaPorts.push_back(desc);
            }
        }
    private:
        typedef std::vector<MetaPort> MetaPortArray;
        MetaPortArray _dynamicMetaPorts;
        typedef std::vector<Port*> PortArray;
        PortArray _dynamicPorts;
    };
}
