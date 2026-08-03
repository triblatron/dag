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
        for (std::size_t i=0; i<other.totalPorts(); ++i)
        {
            auto* p = other._dynamicPorts.a[i];
            auto portClone = p->clone(facility, copyOp, keyGen);

            Boundary::addDynamicPort(portClone, other._dynamicMetaPorts[i].flags);
        }
    }

    Boundary &Boundary::operator=(const Boundary &other)
    {
        if (this != &other)
        {
            dagbase::CloningFacility facility;
            Node::operator=(other);

            for (std::size_t i=0; i<other.totalPorts(); ++i)
            {
                auto* p = other._dynamicPorts.a[i];
                addDynamicPort(p->clone(facility, dagbase::CopyOp{0}, nullptr), other._dynamicMetaPorts[i].flags);
            }
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
        readDynamicPorts(str, nodeLib, lua, _dynamicPorts, _dynamicMetaPorts);
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
        writeDynamicPorts(str, nodeLib, lua, _dynamicPorts, _dynamicMetaPorts);
        str.writeFooter();
        return str;
    }

    bool Boundary::equals(const Node &other, dagbase::ComparisonFlags flags) const
    {
        if (!Node::equals(other, flags))
            return false;

        return true;
    }

    Boundary::~Boundary()
    {
        for (auto p : _dynamicPorts)
        {
            delete p;
        }
    }

    void Boundary::debug(dagbase::DebugPrinter& printer) const
    {
        Node::debug(printer);
        printer.indent();
        for (const auto port : _dynamicPorts)
        {
            port->debug(printer);
        }
        printer.outdent();
        printer.indent();
        for (const auto& metaPort : _dynamicMetaPorts)
        {
            metaPort.debug(printer);
        }
        printer.outdent();
    }
}
