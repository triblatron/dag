//
// Created by tony on 13/02/24.
//

#pragma  once

#include "config/Export.h"

#include "core/Node.h"

namespace dag
{
    class DAG_API Boundary final : public dagbase::Node
    {
    public:
        explicit Boundary(dagbase::KeyGenerator& keyGen, std::string name, dagbase::NodeCategory::Category category=dagbase::NodeCategory::CAT_NONE);

        Boundary(const Boundary& other, dagbase::CloningFacility& facility, dagbase::CopyOp copyOp, dagbase::KeyGenerator* keyGen);

        Boundary(Boundary&& other);

        explicit Boundary(dagbase::InputStream& str, dagbase::NodeLibrary& nodeLib, dagbase::Lua& lua);

        ~Boundary() override;

        Boundary& operator=(const Boundary& other);

        Boundary& operator=(Boundary&& other) = default;

        [[nodiscard]]const char* className() const override
        {
            return "Boundary";
        }

        void describeNode(dagbase::NodeDescriptor& descriptor) const override;

        [[nodiscard]]dagbase::MetaPort const* dynamicMetaPort(size_t index) const override
        {
            if (index<_dynamicMetaPorts.size())
            {
                return &_dynamicMetaPorts[index];
            }

            return nullptr;
        }

        dagbase::Port* dynamicPort(size_t index) override
        {
            if (index<_dynamicPorts.size())
            {
                return _dynamicPorts[index];
            }

            return nullptr;
        }

        Boundary* clone(dagbase::CloningFacility& facility, dagbase::CopyOp copyOp, dagbase::KeyGenerator* keyGen) override
        {
            return new Boundary(*this,facility,copyOp,keyGen);
        }

        Boundary* create(dagbase::InputStream& str, dagbase::NodeLibrary& nodeLib, dagbase::Lua &lua) override;

        [[nodiscard]]bool equals(const Node& other) const override;

        dagbase::OutputStream& writeToStream(dagbase::OutputStream& str, dagbase::NodeLibrary& nodeLib, dagbase::Lua &lua) const override;

        [[nodiscard]]size_t totalPorts() const override
        {
            return _dynamicPorts.size();
        }

        void addDynamicPort(dagbase::Port* port) override
        {
            if (port != nullptr)
            {
                port->setParent(this);
                _dynamicPorts.emplace_back(port);
                dagbase::MetaPort desc;
                desc.name = port->name();
                desc.type = port->type();
                desc.direction = port->dir();
                _dynamicMetaPorts.emplace_back(desc);
            }
        }
    private:
        typedef std::vector<dagbase::MetaPort> MetaPortArray;
        MetaPortArray _dynamicMetaPorts;
        typedef std::vector<dagbase::Port*> PortArray;
        PortArray _dynamicPorts;
    };
}
