//
// Created by tony on 13/02/24.
//

#include "Boundary.h"
#include "NodeLibrary.h"
#include "InputStream.h"
#include "OutputStream.h"

namespace nbe
{
    nbe::Boundary::Boundary(KeyGenerator& keyGen, std::string name, NodeCategory::Category category) : Node(keyGen, std::move(name), category)
    {
        // Do nothing.
    }

    void Boundary::describe(NodeDescriptor &descriptor) const
    {

    }

    Boundary::Boundary(const Boundary &other)
    :
    Node(other)
    {
        for (auto p : other._dynamicPorts)
        {
            auto portClone = p->clone();

            Boundary::addDynamicPort(portClone);
        }
    }

    Boundary &Boundary::operator=(const Boundary &other)
    {
        if (this != &other)
        {
            Node::operator=(other);

            for (auto p : other._dynamicPorts)
            {
                addDynamicPort(p->clone());
            }
        }

        return *this;
    }

    Boundary::Boundary(InputStream &str, NodeLibrary &nodeLib)
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

    Boundary *Boundary::create(InputStream &str, NodeLibrary &nodeLib)
    {
        return new Boundary(str, nodeLib);
    }

    OutputStream &Boundary::write(OutputStream &str) const
    {
        Node::write(str);
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
