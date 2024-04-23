//
// Created by tony on 10/03/24.
//
#include "config/config.h"

#include "Nodes.h"
#include "InputStream.h"
#include "OutputStream.h"

#include <google/protobuf/extension_set.h>

namespace nbe
{
    std::array<MetaPort,1> Base::ports =
            {
                    MetaPort{"direction",PortType::TYPE_DOUBLE, PortDirection::DIR_OUT}
            };

    std::array<MetaPort, 1> Derived::ports =
            {
                    MetaPort("trigger", PortType::TYPE_BOOL, PortDirection::DIR_IN)
            };

    std::array<MetaPort, 1> Final::ports=
            {
                    MetaPort("int1", PortType::TYPE_INT, PortDirection::DIR_INTERNAL)
            };

    std::array<MetaPort, 1> Foo::ports =
            {
                    MetaPort("in1", PortType::TYPE_DOUBLE, PortDirection::DIR_IN)
            };

    Foo *Foo::create(InputStream &str, NodeLibrary& nodeLib)
    {
        return new Foo(str, nodeLib);
    }

    Foo::Foo(InputStream &str, NodeLibrary& nodeLib)
            :
            Node(str, nodeLib),
            in1(str, nodeLib)
    {

    }

    OutputStream &Foo::write(OutputStream &str) const
    {
        Node::write(str);

        in1.write(str);

        return str;
    }

    bool Foo::equals(const Node &other) const
    {
        if (Node::operator==(other))
        {
            return false;
        }

        const Foo& foo = dynamic_cast<const Foo&>(other);

        if (!(in1 == foo.in1))
        {
            return false;
        }

        return true;
    }

    std::array<MetaPort, 1> FooTyped::ports =
            {
                    MetaPort("in1", PortType::TYPE_DOUBLE, PortDirection::DIR_IN)
            };

    FooTyped *FooTyped::create(InputStream &str, NodeLibrary &nodeLib)
    {
        return new FooTyped(str, nodeLib);
    }

    FooTyped::FooTyped(InputStream &str, NodeLibrary &nodeLib)
            :
            Node(str, nodeLib)

    {
        Stream::ObjId in1Id = 0;
        Stream::Ref in1Ref = str.readRef(&in1Id);

        if (in1Id != 0)
        {
            if (in1Ref != nullptr)
            {
                _in1 = static_cast<TypedPort<double>*>(in1Ref);
            }
            else
            {
                _in1 = dynamic_cast<TypedPort<double>*>(nodeLib.instantiatePort(str));
            }
        }
    }

    OutputStream &FooTyped::write(OutputStream &str) const
    {
        Node::write(str);
        if (str.writeRef(_in1))
        {
            _in1->write(str);
        }

        return str;
    }

    bool FooTyped::equals(const Node &other) const
    {
        if (!Node::operator==(other))
        {
            return false;
        }

        auto const & foo = dynamic_cast<const FooTyped&>(other);

        if (!_in1->operator==(*foo._in1))
        {
            return false;
        }

        return true;
    }

    void FooTyped::debug(DebugPrinter &printer) const
    {
        Node::debug(printer);
        _in1->debug(printer);
    }

    FooTyped::~FooTyped()
    {
        delete _in1;
    }

    std::array<MetaPort, 1> Bar::ports =
            {
                    MetaPort("out1", PortType::TYPE_DOUBLE, PortDirection::DIR_OUT)
            };

    OutputStream &Bar::write(OutputStream &str) const
    {
        Node::write(str);
        out1.write(str);

        return str;
    }

    Bar::Bar(InputStream &str, NodeLibrary &nodeLib)
            :
            Node(str, nodeLib),
            out1(str, nodeLib)
    {

    }

    Bar *Bar::create(InputStream &str, NodeLibrary &nodeLib)
    {
        return new Bar(str, nodeLib);
    }

    bool Bar::equals(const Node &other) const
    {
        return false;
    }

    std::array<MetaPort, 1> BarTyped::ports =
            {
                    MetaPort("out1", PortType::TYPE_DOUBLE, PortDirection::DIR_OUT)
            };

    BarTyped *BarTyped::create(InputStream &str, NodeLibrary &nodeLib)
    {
        return new BarTyped(str, nodeLib);
    }

    BarTyped::BarTyped(InputStream &str, NodeLibrary &nodeLib)
            :
            Node(str, nodeLib)
    {
        Stream::ObjId out1Id = 0;
        Stream::Ref out1Ref = str.readRef(&out1Id);

        if (out1Id != 0)
        {
            if (out1Ref != nullptr)
            {
                _out1 = static_cast<TypedPort<double>*>(out1Ref);
            }
            else
            {
                _out1 = dynamic_cast<TypedPort<double>*>(nodeLib.instantiatePort(str));
            }
        }
    }

    OutputStream &BarTyped::write(OutputStream &str) const
    {
        Node::write(str);
        if (str.writeRef(_out1))
        {
            _out1->write(str);
        }

        return str;
    }

    bool BarTyped::equals(const Node &other) const
    {
        if (!Node::operator==(other))
        {
            return false;
        }

        auto const & bar = dynamic_cast<const BarTyped&>(other);

        if (!_out1->equals(*bar._out1))
        {
            return false;
        }

        return true;
    }

    BarTyped::~BarTyped()
    {
        delete _out1;
    }

    std::array<MetaPort, 2> GroupTyped::ports =
            {
                    MetaPort("out1", PortType::TYPE_DOUBLE, PortDirection::DIR_OUT),
                    MetaPort("in1", PortType::TYPE_DOUBLE, PortDirection::DIR_IN)
            };

    GroupTyped::GroupTyped(InputStream &str, NodeLibrary &nodeLib)
            :
            Node(str, nodeLib),
            _out1(str, nodeLib),
            _in1(str, nodeLib)
    {
        // Do nothing.
    }

    GroupTyped *GroupTyped::create(InputStream &str, NodeLibrary &nodeLib)
    {
        return new GroupTyped(str, nodeLib);
    }

    OutputStream &GroupTyped::write(OutputStream &str) const
    {
        Node::write(str);
        _out1.write(str);
        _in1.write(str);

        return str;
    }

    bool GroupTyped::equals(const Node &other) const
    {
        return false;
    }

    void Base::describe(NodeDescriptor& descriptor) const
    {
        descriptor.id = id();
        descriptor.name = name();
        descriptor.category = category();
        PortDescriptor portDescriptor;
        portDescriptor.id = _direction.id();
        portDescriptor.name = _direction.name();
        portDescriptor.type = _direction.type();
        portDescriptor.direction = _direction.dir();
        descriptor.ports.push_back(portDescriptor);
    }

    Node *Base::create(InputStream &str, NodeLibrary& nodeLib)
    {
        return new Base(str, nodeLib);
    }

    Base::Base(InputStream &str, NodeLibrary& nodeLib)
            :
            Node(str, nodeLib),
            int1(0.0),
            _direction(str,nodeLib)
    {
        str.read(&int1);
    }

    OutputStream &Base::write(OutputStream &str) const
    {
        Node::write(str);
        _direction.write(str);
        str.write(int1);

        return str;
    }

    bool Base::equals(const Node &other) const
    {
        return false;
    }
}
