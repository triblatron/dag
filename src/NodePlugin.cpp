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
#include "MetaPort.h"

#include <vector>

class DynamicNode : public dag::Node
{
public:
    DynamicNode(dag::KeyGenerator& keyGen, const std::string& name)
    :
    Node(keyGen, name, dag::NodeCategory::CAT_GROUP)
    {
        // Do nothing.
    }

    DynamicNode(dag::InputStream& str, dag::NodeLibrary& nodeLib)
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

    DynamicNode(const DynamicNode& other, dag::CloningFacility& facility,  dag::CopyOp copyOp, dag::KeyGenerator* keyGen);

    ~DynamicNode() override;

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

    void describe(dag::NodeDescriptor& descriptor) const override
    {
        descriptor.id = id();
        descriptor.category = category();
        descriptor.name = name();
        //descriptor.ports
    }

    [[nodiscard]]const dag::MetaPort * dynamicMetaPort(size_t index) const override
    {
        if (index < _dynamicMetaPorts.size())
        {
            return &_dynamicMetaPorts[index];
        }

        return nullptr;
    }

    void addDynamicPort(dag::Port* port) override
    {
        if (port != nullptr)
        {
            _dynamicPorts.push_back(port);
            dag::MetaPort desc;
            desc.name = port->name();
            desc.type = port->type();
            desc.direction = port->dir();
            _dynamicMetaPorts.push_back(desc);
        }
    }
    
    dag::Port* dynamicPort(size_t index) override
    {
        if (index<_dynamicPorts.size())
        {
            return _dynamicPorts[index];
        }

        return nullptr;
    }

    [[nodiscard]]const dag::Port* dynamicPort(size_t index) const
    {
        if (index<_dynamicPorts.size())
        {
            return _dynamicPorts[index];
        }

        return nullptr;
    }

    Node* create(dag::InputStream& str, dag::NodeLibrary& nodeLib) override
    {
        return new DynamicNode(str, nodeLib);
    }

    dag::OutputStream& write(dag::OutputStream& str) const override
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

    DynamicNode* clone(dag::CloningFacility& facility, dag::CopyOp copyOp, dag::KeyGenerator* keyGen) override
    {
        return new DynamicNode(*this,facility,copyOp,keyGen);
    }
private:
    typedef std::vector<dag::MetaPort> MetaPortArray;
    MetaPortArray _dynamicMetaPorts;
    typedef std::vector<dag::Port*> PortArray;
    PortArray _dynamicPorts;
};

DynamicNode::DynamicNode(const DynamicNode &other, dag::CloningFacility& facility, dag::CopyOp copyOp, dag::KeyGenerator *keyGen)
:
Node(other, facility, copyOp, keyGen)
{
    for (auto it=other._dynamicPorts.begin(); it!=other._dynamicPorts.end(); ++it)
    {
        auto port = (*it)->clone(facility,copyOp,keyGen);

        _dynamicPorts.push_back(port);
    }

    for (auto it=other._dynamicMetaPorts.begin(); it!=other._dynamicMetaPorts.end(); ++it)
    {
        const auto& metaPort(*it);

        _dynamicMetaPorts.push_back(metaPort);
    }
}

DynamicNode::~DynamicNode()
{
    for (auto port : _dynamicPorts)
    {
        delete port;
    }
}

#if defined(_MSC_VER) || defined(__CYGWIN__) || defined(__MINGW32__) || defined( __BCPLUSPLUS__)  || defined( __MWERKS__)
#define NODEPLUGIN_API __declspec(dllexport)
#endif

extern "C"
{
    NODEPLUGIN_API void  init(dag::KeyGenerator& keyGen, dag::NodeLibrary& nodeLib)
    {
        auto node = new DynamicNode(keyGen, "dyn1");
        nodeLib.registerNode(node);
    }
}