//
// Created by tony on 04/05/24.
//
#include "config/config.h"

#include "MathNode.h"

#include <cmath>

namespace dag
{
    std::array<dagbase::MetaPort,3> MathsNode::ports =
            {
                    dagbase::MetaPort{dagbase::MetaPort::FLAGS_OWN_BIT},
                    dagbase::MetaPort(dagbase::MetaPort::FLAGS_OWN_BIT),
                    dagbase::MetaPort(dagbase::MetaPort::FLAGS_OWN_BIT)
            };

    bool MathsNode::equals(const Node &other, dagbase::ComparisonFlags flags) const
    {
        if (!Node::equals(other, flags))
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

    const dagbase::Port * MathsNode::dynamicPort(size_t index) const
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
            Node(str, nodeLib, lua)

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

    dagbase::OutputStream &MathsNode::writeToStream(dagbase::OutputStream &str, dagbase::NodeLibrary& nodeLib, dagbase::Lua &lua) const
    {
        Node::writeToStream(str, nodeLib, lua);

        if (str.writeRef(_angle))
        {
            _angle->writeToStream(str, nodeLib, lua);
        }

        if (str.writeRef(_unit))
        {
            _unit->writeToStream(str, nodeLib, lua);
        }

        if (str.writeRef(_output))
        {
            _output->writeToStream(str, nodeLib, lua);
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
