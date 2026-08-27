//
// Created by tony on 13/02/24.
//

#include "Boundary.h"
#include "../thirdparty/dagbase/include/core/NodeLibrary.h"
#include "io/InputStream.h"
#include "io/OutputStream.h"
#include "../thirdparty/dagbase/include/core/CloningFacility.h"

namespace dag
{
    dag::Boundary::Boundary(dagbase::KeyGenerator& keyGen, std::string name, dagbase::NodeCategory::Category category) : Node(keyGen, std::move(name), category)
    {
        // Do nothing.
    }

    Boundary::Boundary(const Boundary &other, dagbase::CloningFacility& facility, dagbase::CopyOp copyOp, dagbase::KeyGenerator* keyGen)
    :
    Node(other, facility, copyOp, keyGen)
    {
        clonePorts(other, facility, copyOp, keyGen);
    }

    Boundary &Boundary::operator=(const Boundary &other)
    {
        if (this != &other)
        {
            Node::operator=(other);
        }

        return *this;
    }

    Boundary::Boundary(dagbase::InputStream &str, dagbase::NodeLibrary &nodeLib, dagbase::Lua &lua)
    :
    Node()
    {
        std::string className;
        str.readHeader(&className);
        Node::readFromStream(str, nodeLib, lua);
        str.readFooter();
    }

    Boundary *Boundary::create(dagbase::InputStream &str, dagbase::NodeLibrary &nodeLib, dagbase::Lua &lua)
    {
        return new Boundary(str, nodeLib, lua);
    }

    dagbase::OutputStream &Boundary::writeToStream(dagbase::OutputStream &str, dagbase::NodeLibrary& nodeLib, dagbase::Lua &lua) const
    {
        str.writeHeader("Boundary");
        Node::writeToStream(str, nodeLib, lua);
        str.writeFooter();
        return str;
    }

    bool Boundary::equals(const Node &other, dagbase::ComparisonFlags flags) const
    {
        if (!Node::equals(other, flags))
            return false;

        return true;
    }

    void Boundary::debug(dagbase::DebugPrinter& printer) const
    {
        Node::debug(printer);
    }
}
