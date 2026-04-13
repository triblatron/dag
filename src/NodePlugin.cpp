//
// Created by tony on 29/03/24.
//

#include "config/config.h"

#include "core/NodeLibrary.h"
#include "core/Node.h"
#include "core/NodeDescriptor.h"
#include "io/InputStream.h"
#include "io/OutputStream.h"
#include "NodePlugin.h"
#include "core/MetaPort.h"

#include <vector>

class DynamicNode : public dagbase::Node
{
public:
    DynamicNode(dagbase::KeyGenerator& keyGen, const std::string& name)
    :
    Node(keyGen, name, dagbase::NodeCategory::CAT_GROUP)
    {
        // Do nothing.
    }

    DynamicNode(dagbase::InputStream& str, dagbase::NodeLibrary& nodeLib, dagbase::Lua& lua)
    :
    Node()
    {
        std::string className;
        std::string fieldName;
        str.readHeader(&className);
        Node::readFromStream(str, nodeLib);
        std::uint32_t numDynamicMetaPorts = 0;
        str.readField(&fieldName);
        str.readUInt32(&numDynamicMetaPorts);
        _dynamicMetaPorts.resize(numDynamicMetaPorts);
        str.readField(&fieldName);
        for (std::size_t i=0; i<numDynamicMetaPorts; ++i)
        {
            _dynamicMetaPorts[i].read(str);
        }
        std::uint32_t numDynamicPorts = 0;
        str.readField(&fieldName);
        str.readUInt32(&numDynamicPorts);
        _dynamicPorts.resize(numDynamicPorts);
        str.readField(&fieldName);
        for (std::size_t i=0; i<numDynamicPorts; ++i)
        {
            _dynamicPorts[i] = nodeLib.instantiatePort(str, lua);
        }
        str.readFooter();
    }

    DynamicNode(const DynamicNode& other, dagbase::CloningFacility& facility,  dagbase::CopyOp copyOp, dagbase::KeyGenerator* keyGen);

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

        for (std::size_t index = 0; index<totalPorts(); ++index)
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

    void describe(dagbase::NodeDescriptor& descriptor) const override
    {
        descriptor.id = id();
        descriptor.category = category();
        descriptor.name = name();
        //descriptor.ports
    }

    [[nodiscard]]const dagbase::MetaPort * dynamicMetaPort(size_t index) const override
    {
        if (index < _dynamicMetaPorts.size())
        {
            return &_dynamicMetaPorts[index];
        }

        return nullptr;
    }

    void addDynamicPort(dagbase::Port* port) override
    {
        if (port != nullptr)
        {
            _dynamicPorts.emplace_back(port);
            dagbase::MetaPort desc;
            desc.name = port->name();
            desc.type = port->type();
            desc.direction = port->dir();
            _dynamicMetaPorts.emplace_back(desc);
        }
    }
    
    dagbase::Port* dynamicPort(size_t index) override
    {
        if (index<_dynamicPorts.size())
        {
            return _dynamicPorts[index];
        }

        return nullptr;
    }

    [[nodiscard]]const dagbase::Port* dynamicPort(size_t index) const
    {
        if (index<_dynamicPorts.size())
        {
            return _dynamicPorts[index];
        }

        return nullptr;
    }

    Node* create(dagbase::InputStream& str, dagbase::NodeLibrary& nodeLib, dagbase::Lua& lua) override
    {
        return new DynamicNode(str, nodeLib, lua);
    }

    dagbase::OutputStream& write(dagbase::OutputStream& str) const override
    {
        str.writeHeader("NodePlugin.DynamicNode");
        Node::write(str);
        str.writeField("numDynamicMetaPorts");
        str.writeUInt32(_dynamicMetaPorts.size());
        str.writeField("dynamicMetaPorts");
        for (auto const & p : _dynamicMetaPorts)
        {
            p.write(str);
        }
        str.writeField("numDynamicPorts");
        str.writeUInt32(_dynamicPorts.size());
        str.writeField("dynamicPorts");
        for (auto p : _dynamicPorts)
        {
            p->write(str);
        }
        str.writeFooter();
        return str;
    }

    DynamicNode* clone(dagbase::CloningFacility& facility, dagbase::CopyOp copyOp, dagbase::KeyGenerator* keyGen) override
    {
        return new DynamicNode(*this,facility,copyOp,keyGen);
    }
private:
    typedef std::vector<dagbase::MetaPort> MetaPortArray;
    MetaPortArray _dynamicMetaPorts;
    typedef std::vector<dagbase::Port*> PortArray;
    PortArray _dynamicPorts;
};

DynamicNode::DynamicNode(const DynamicNode &other, dagbase::CloningFacility& facility, dagbase::CopyOp copyOp, dagbase::KeyGenerator *keyGen)
:
Node(other, facility, copyOp, keyGen)
{
    for (auto it=other._dynamicPorts.begin(); it!=other._dynamicPorts.end(); ++it)
    {
        auto port = (*it)->clone(facility,copyOp,keyGen);

        _dynamicPorts.emplace_back(port);
    }

    for (auto it=other._dynamicMetaPorts.begin(); it!=other._dynamicMetaPorts.end(); ++it)
    {
        const auto& metaPort(*it);

        _dynamicMetaPorts.emplace_back(metaPort);
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
    NODEPLUGIN_API void  init(dagbase::KeyGenerator& keyGen, dagbase::NodeLibrary& nodeLib)
    {
        auto node = new DynamicNode(keyGen, "dyn1");
        nodeLib.registerNode(node);
    }
}