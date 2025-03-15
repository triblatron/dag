//
// Created by tony on 13/02/24.
//

#include "Boundary.h"
#include "NodeLibrary.h"
#include "io/InputStream.h"
#include "io/OutputStream.h"
#include "CloningFacility.h"

namespace dag
{
    dag::Boundary::Boundary(KeyGenerator& keyGen, std::string name, NodeCategory::Category category) : Node(keyGen, std::move(name), category)
    {
        // Do nothing.
    }

    void Boundary::describe(NodeDescriptor &descriptor) const
    {

    }

    Boundary::Boundary(const Boundary &other, CloningFacility& facility, CopyOp copyOp, KeyGenerator* keyGen)
    :
    Node(other, facility, copyOp, keyGen)
    {
        for (auto p : other._dynamicPorts)
        {
            auto portClone = p->clone(facility, copyOp, keyGen);

            Boundary::addDynamicPort(portClone);
        }
    }

    Boundary &Boundary::operator=(const Boundary &other)
    {
        if (this != &other)
        {
            CloningFacility facility;
            Node::operator=(other);

            for (auto p : other._dynamicPorts)
            {
                addDynamicPort(p->clone(facility, CopyOp{0}, nullptr));
            }
        }

        return *this;
    }

    Boundary::Boundary(dagbase::InputStream &str, NodeLibrary &nodeLib)
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
        for (auto i=0; i<numDynamicMetaPorts; ++i)
        {
            _dynamicMetaPorts[i].read(str);
        }
        std::uint32_t numDynamicPorts = 0;
        str.readField(&fieldName);
        str.readUInt32(&numDynamicPorts);
        _dynamicPorts.resize(numDynamicPorts);
        str.readField(&fieldName);
        for (auto i=0; i<numDynamicPorts; ++i)
        {
            _dynamicPorts[i] = nodeLib.instantiatePort(str);
        }
        str.readFooter();
    }

    Boundary *Boundary::create(dagbase::InputStream &str, NodeLibrary &nodeLib)
    {
        return new Boundary(str, nodeLib);
    }

    dagbase::OutputStream &Boundary::write(dagbase::OutputStream &str) const
    {
        str.writeHeader("Boundary");
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

    bool Boundary::equals(const Node &other) const
    {
        return Node::operator==(other);
    }

    Boundary::~Boundary()
    {
        for (auto p : _dynamicPorts)
        {
            delete p;
        }
    }
}
