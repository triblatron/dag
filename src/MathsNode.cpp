//
// Created by tony on 04/05/24.
//
#include "config/config.h"

#include "MathNode.h"
#include "NodeDescriptor.h"
#include <cmath>

namespace nbe
{
    std::array<MetaPort,3> MathsNode::ports =
            {
                    MetaPort{"angle",PortType::TYPE_DOUBLE, PortDirection::DIR_IN},
                    MetaPort("unit", PortType::TYPE_INT, PortDirection::DIR_INTERNAL),
                    MetaPort("output", PortType::TYPE_DOUBLE, PortDirection::DIR_OUT)
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

    void MathsNode::describe(NodeDescriptor &descriptor) const
    {
        descriptor.id = id();
        descriptor.name = name();
        descriptor.category = category();
        PortDescriptor portDescriptor;
        portDescriptor.id = _angle->id();
        portDescriptor.name = _angle->name();
        portDescriptor.type = _angle->type();
        portDescriptor.direction = _angle->dir();
        descriptor.ports.push_back(portDescriptor);
        portDescriptor.id = _unit->id();
        portDescriptor.name = _unit->name();
        portDescriptor.type = _unit->type();
        portDescriptor.direction = _unit->dir();
        descriptor.ports.push_back(portDescriptor);
        portDescriptor.id = _output->id();
        portDescriptor.name = _output->name();
        portDescriptor.type = _output->type();
        portDescriptor.direction = _output->dir();
        descriptor.ports.push_back(portDescriptor);
    }

    const MetaPort *MathsNode::dynamicMetaPort(size_t index) const
    {
        if (index < firstPort + numPorts)
        {
            return &ports[index - firstPort];
        }

        return nullptr;
    }

    Port *MathsNode::dynamicPort(size_t index)
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

    Node *MathsNode::create(InputStream &str, NodeLibrary &nodeLib)
    {
        return new MathsNode(str, nodeLib);
    }

    MathsNode::MathsNode(InputStream &str, NodeLibrary &nodeLib)
            :
            Node(str, nodeLib)

    {
        // We must do a static_cast<> here because we might be in the Port constructor
        // and the class is not yet a TypedPort and the dynamic_cast<> will fail.
        _angle = static_cast<TypedPort<double>*>(str.readPort(nodeLib));
        _unit = static_cast<TypedPort<int64_t>*>(str.readPort(nodeLib));
        _output = static_cast<TypedPort<double>*>(str.readPort(nodeLib));
    }

    MathsNode::MathsNode(const MathsNode &other, CopyOp copyOp, KeyGenerator* keyGen)
    :
    Node(other)
    {
        _angle = new TypedPort<double>(*other._angle);
        _angle->setParent(this);
        _unit = new TypedPort<int64_t>(*other._unit);
        _unit->setParent(this);
        _output = new TypedPort<double>(*other._output);
        _output->setParent(this);
    }

    OutputStream &MathsNode::write(OutputStream &str) const
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
}
