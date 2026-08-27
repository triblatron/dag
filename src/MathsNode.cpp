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

        return true;
    }

    const char *MathsNode::className() const
    {
        return "MathsNode";
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
    }

    MathsNode::MathsNode(const MathsNode &other, dagbase::CloningFacility& facility, dagbase::CopyOp copyOp, dagbase::KeyGenerator* keyGen)
    :
    Node(other, facility, copyOp, keyGen)
    {
        clonePorts(other, facility, copyOp, keyGen);
    }

    dagbase::OutputStream &MathsNode::writeToStream(dagbase::OutputStream &str, dagbase::NodeLibrary& nodeLib, dagbase::Lua &lua) const
    {
        Node::writeToStream(str, nodeLib, lua);

        return str;
    }

    void MathsNode::update()
    {
        static_cast<dagbase::TypedPort<double>*>(dynamicPort(PORT_OUTPUT))->setValue(std::sin(static_cast<dagbase::TypedPort<double>*>(dynamicPort(PORT_ANGLE))->value()));
    }

    MathsNode::~MathsNode()
    {
        deleteDynamicPorts();
    }
}
