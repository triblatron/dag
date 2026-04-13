//
// Created by tony on 04/05/24.
//
#include "config/config.h"

#include "MathNode.h"
#include "../thirdparty/dagbase/include/core/NodeDescriptor.h"
#include <cmath>

namespace dag
{
    std::array<dagbase::MetaPort,3> MathsNode::ports =
            {
                    dagbase::MetaPort{"angle",dagbase::PortType::TYPE_DOUBLE, dagbase::PortDirection::DIR_IN},
                    dagbase::MetaPort("unit", dagbase::PortType::TYPE_INT64, dagbase::PortDirection::DIR_INTERNAL),
                    dagbase::MetaPort("output", dagbase::PortType::TYPE_DOUBLE, dagbase::PortDirection::DIR_OUT)
            };

    bool MathsNode::equals(const Node &other) const
    {
        if (!Node::operator==(other))
        {
            return false;
        }

        auto const & mathOther = dynamic_cast<MathsNode const&>(other);

        return *this->_angle == *mathOther._angle && *this->_unit == *mathOther._unit && *this->_output == *mathOther._output;
    }

    const char *MathsNode::className() const
    {
        return "MathsNode";
    }

    void MathsNode::describe(dagbase::NodeDescriptor &descriptor) const
    {
        descriptor.id = id();
        descriptor.name = name();
        descriptor.category = category();
        dagbase::MetaPort portDescriptor;
        portDescriptor.name = _angle->name();
        portDescriptor.type = _angle->type();
        portDescriptor.direction = _angle->dir();
        descriptor.ports.emplace_back(portDescriptor);
        portDescriptor.name = _unit->name();
        portDescriptor.type = _unit->type();
        portDescriptor.direction = _unit->dir();
        descriptor.ports.emplace_back(portDescriptor);
        portDescriptor.name = _output->name();
        portDescriptor.type = _output->type();
        portDescriptor.direction = _output->dir();
        descriptor.ports.emplace_back(portDescriptor);
    }

    const dagbase::MetaPort *MathsNode::dynamicMetaPort(size_t index) const
    {
        if (index < firstPort + numPorts)
        {
            return &ports[index - firstPort];
        }

        return nullptr;
    }

    dagbase::Port *MathsNode::dynamicPort(size_t index)
    {
        if (index == firstPort)
        {
            return _angle;
        }
        else if (index == firstPort+1)
        {
            return _unit;
        }
        else if (index == firstPort+2)
        {
            return _output;
        }

        return nullptr;
    }

    dagbase::Node *MathsNode::create(dagbase::InputStream &str, dagbase::NodeLibrary &nodeLib, dagbase::Lua &lua)
    {
        return new MathsNode(str, nodeLib, lua);
    }

    MathsNode::MathsNode(dagbase::InputStream &str, dagbase::NodeLibrary &nodeLib, dagbase::Lua &lua)
            :
            Node(str, nodeLib)

    {
        // We must do a static_cast<> here because we might be in the Port constructor
        // and the class is not yet a TypedPort and the dynamic_cast<> will fail.
        _angle = static_cast<dagbase::TypedPort<double>*>(str.readRef<dagbase::Port>("Port", nodeLib, lua));
        _unit = static_cast<dagbase::TypedPort<int64_t>*>(str.readRef<dagbase::Port>("Port", nodeLib, lua));
        _output = static_cast<dagbase::TypedPort<double>*>(str.readRef<dagbase::Port>("Port", nodeLib, lua));
    }

    MathsNode::MathsNode(const MathsNode &other, dagbase::CloningFacility& facility, dagbase::CopyOp copyOp, dagbase::KeyGenerator* keyGen)
    :
    Node(other, facility, copyOp, keyGen)
    {
        _angle = new dagbase::TypedPort<double>(*other._angle, facility, copyOp, keyGen);
        _angle->setParent(this);
        _unit = new dagbase::TypedPort<int64_t>(*other._unit, facility, copyOp, keyGen);
        _unit->setParent(this);
        _output = new dagbase::TypedPort<double>(*other._output, facility, copyOp, keyGen);
        _output->setParent(this);
    }

    dagbase::OutputStream &MathsNode::write(dagbase::OutputStream &str) const
    {
        Node::write(str);

        if (str.writeRef(_angle))
        {
            _angle->write(str);
        }

        if (str.writeRef(_unit))
        {
            _unit->write(str);
        }

        if (str.writeRef(_output))
        {
            _output->write(str);
        }

        return str;
    }

    void MathsNode::update()
    {
        _output->setValue(std::sin(_angle->value()));
    }

    MathsNode::~MathsNode()
    {
        delete _angle;
        delete _unit;
        delete _output;
    }
}
