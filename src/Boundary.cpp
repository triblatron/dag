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
            auto* p = other._dynamicPorts[i];
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
                auto* p = other._dynamicPorts[i];
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
        std::string fieldName;
        str.readHeader(&className);
        Node::readFromStream(str, nodeLib, lua);
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
            dagbase::Stream::ObjId portId{~0U};
            auto portRef = str.readRef(&portId);
            if (portId != 0)
            {
                if (portRef != nullptr)
                {
                    _dynamicPorts[i] = static_cast<dagbase::Port*>(portRef);
                }
                else
                {
                    _dynamicPorts[i] = nodeLib.instantiatePort(str, lua);
                }
            }


        }
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
            if (str.writeRef(p))
                p->writeToStream(str, nodeLib, lua);
        }
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
