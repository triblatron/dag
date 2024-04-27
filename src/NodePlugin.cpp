//
// Created by tony on 29/03/24.
//

#include "config/config.h"

#include "NodeLibrary.h"
#include "Node.h"
#include "NodeDescriptor.h"
#include "InputStream.h"
#include "OutputStream.h"
#include "NodePlugin.h"

#include <vector>

class DynamicNode : public nbe::Node
{
public:
    DynamicNode(nbe::KeyGenerator& keyGen, const std::string& name)
    :
    Node(keyGen, name, nbe::NodeCategory::CAT_GROUP)
    {
        // Do nothing.
    }

    DynamicNode(nbe::InputStream& str, nbe::NodeLibrary& nodeLib)
    :
    Node(str, nodeLib)
    {
        size_t numDynamicMetaPorts = 0;
        str.read(&numDynamicMetaPorts);
        _dynamicMetaPorts.resize(numDynamicMetaPorts);
        for (auto i=0; i<numDynamicMetaPorts; ++i)
        {
            _dynamicMetaPorts[i].read(str);
        }
        size_t numDynamicPorts = 0;
        str.read(&numDynamicPorts);
        _dynamicPorts.resize(numDynamicPorts);
        for (auto i=0; i<numDynamicPorts; ++i)
        {
            _dynamicPorts[i] = nodeLib.instantiatePort(str);
        }
    }

    [[nodiscard]]bool equals(const Node& other) const override
    {
        if (!Node::operator==(other))
        {
            return false;
        }

        auto & dynamic = dynamic_cast<const DynamicNode&>(other);

        if (totalPorts() != dynamic.totalPorts())
        {
            return false;
        }

        for (auto index = 0; index<totalPorts(); ++index)
        {
            if (!dynamicPort(index)->equals(*dynamic.dynamicPort(index)))
            {
                return false;
            }
        }

        return true;
    }

    [[nodiscard]] const char* className() const override
    {
        return "NodePlugin.DynamicNode";
    }

    void describe(nbe::NodeDescriptor& descriptor) const override
    {
        descriptor.id = id();
        descriptor.category = category();
        descriptor.name = name();
        //descriptor.ports
    }

    [[nodiscard]]const nbe::MetaPort * dynamicMetaPort(size_t index) const override
    {
        if (index < _dynamicMetaPorts.size())
        {
            return &_dynamicMetaPorts[index];
        }

        return nullptr;
    }

    nbe::Port* dynamicPort(size_t index) override
    {
        if (index<_dynamicPorts.size())
        {
            return _dynamicPorts[index];
        }

        return nullptr;
    }

    [[nodiscard]]const nbe::Port* dynamicPort(size_t index) const
    {
        if (index<_dynamicPorts.size())
        {
            return _dynamicPorts[index];
        }

        return nullptr;
    }

    Node* create(nbe::InputStream& str, nbe::NodeLibrary& nodeLib) override
    {
        return new DynamicNode(str, nodeLib);
    }

    nbe::OutputStream& write(nbe::OutputStream& str) const override
    {
        str.write(_dynamicMetaPorts.size());
        for (auto const & p : _dynamicMetaPorts)
        {
            p.write(str);
        }
        str.write(_dynamicPorts.size());
        for (auto p : _dynamicPorts)
        {
            p->write(str);
        }

        return str;
    }

    DynamicNode* clone() override
    {
        return new DynamicNode(*this);
    }
private:
    typedef std::vector<nbe::MetaPort> MetaPortArray;
    MetaPortArray _dynamicMetaPorts;
    typedef std::vector<nbe::Port*> PortArray;
    PortArray _dynamicPorts;
};

void init(nbe::KeyGenerator& keyGen, nbe::NodeLibrary& nodeLib)
{
    auto node = new DynamicNode(keyGen, "dyn1");
    nodeLib.registerNode(node);
}
