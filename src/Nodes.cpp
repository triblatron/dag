//
// Created by tony on 10/03/24.
//
#include "config/config.h"

#include "Nodes.h"
#include "io/InputStream.h"
#include "io/OutputStream.h"

namespace dag
{
    std::array<dagbase::MetaPort,1> Base::ports =
            {
                    dagbase::MetaPort{"direction",dagbase::PortType::TYPE_DOUBLE, dagbase::PortDirection::DIR_OUT}
            };

    std::array<dagbase::MetaPort, 1> Derived::ports =
            {
                    dagbase::MetaPort("trigger", dagbase::PortType::TYPE_BOOL, dagbase::PortDirection::DIR_IN)
            };

    std::array<dagbase::MetaPort, 1> Final::ports=
            {
                    dagbase::MetaPort("int1", dagbase::PortType::TYPE_INT64, dagbase::PortDirection::DIR_INTERNAL)
            };

    std::array<dagbase::MetaPort, 1> Foo::ports =
            {
                    dagbase::MetaPort("in1", dagbase::PortType::TYPE_DOUBLE, dagbase::PortDirection::DIR_IN)
            };

    Foo *Foo::create(dagbase::InputStream &str, dagbase::NodeLibrary& nodeLib, dagbase::Lua &lua)
    {
        return new Foo(str, nodeLib, lua);
    }

    Foo::Foo(dagbase::InputStream &str, dagbase::NodeLibrary& nodeLib, dagbase::Lua &lua)
            :
            Node(str, nodeLib, lua),
            in1(str, nodeLib, lua)
    {

    }

    dagbase::OutputStream &Foo::writeToStream(dagbase::OutputStream &str, dagbase::NodeLibrary& nodeLib, dagbase::Lua &lua) const
    {
        Node::writeToStream(str, nodeLib, lua);

        in1.writeToStream(str, nodeLib, lua);

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

    std::array<dagbase::MetaPort, 1> FooTyped::ports =
            {
                    dagbase::MetaPort("in1", dagbase::PortType::TYPE_DOUBLE, dagbase::PortDirection::DIR_IN)
            };

    FooTyped *FooTyped::create(dagbase::InputStream &str, dagbase::NodeLibrary &nodeLib, dagbase::Lua &lua)
    {
        return new FooTyped(str, nodeLib, lua);
    }

    FooTyped::FooTyped(dagbase::InputStream &str, dagbase::NodeLibrary &nodeLib, dagbase::Lua &lua)
            :
            Node()

    {
        std::string className;
        std::string fieldName;
        str.readHeader(&className);
        Node::readFromStream(str, nodeLib, lua);
        str.readField(&fieldName);
        dagbase::Stream::ObjId in1Id = 0;
        dagbase::Stream::Ref in1Ref = str.readRef(&in1Id);


        if (in1Id != 0)
        {
            if (in1Ref != nullptr)
            {
                _in1 = static_cast<dagbase::TypedPort<double>*>(in1Ref);
            }
            else
            {
                _in1 = dynamic_cast<dagbase::TypedPort<double>*>(nodeLib.instantiatePort(str, lua));
            }
        }
        str.readFooter();
    }

    dagbase::OutputStream &FooTyped::writeToStream(dagbase::OutputStream &str, dagbase::NodeLibrary& nodeLib, dagbase::Lua &lua) const
    {
        str.writeHeader("FooTyped");
        Node::writeToStream(str, nodeLib, lua);
        str.writeField("in1");
        if (str.writeRef(_in1))
        {
            _in1->writeToStream(str, nodeLib, lua);
        }
        str.writeFooter();
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

    void FooTyped::debug(dagbase::DebugPrinter& printer) const
    {
        Node::debug(printer);
        _in1->debug(printer);
    }

    FooTyped::~FooTyped()
    {
        delete _in1;
    }

    std::array<dagbase::MetaPort, 1> Bar::ports =
            {
                    dagbase::MetaPort("out1", dagbase::PortType::TYPE_DOUBLE, dagbase::PortDirection::DIR_OUT)
            };

    dagbase::OutputStream &Bar::writeToStream(dagbase::OutputStream &str, dagbase::NodeLibrary& nodeLib, dagbase::Lua &lua) const
    {
        Node::writeToStream(str, nodeLib, lua);
        out1.writeToStream(str, nodeLib, lua);

        return str;
    }

    Bar::Bar(dagbase::InputStream &str, dagbase::NodeLibrary &nodeLib, dagbase::Lua &lua)
            :
            Node(str, nodeLib, lua),
            out1(str, nodeLib, lua)
    {

    }

    Bar *Bar::create(dagbase::InputStream &str, dagbase::NodeLibrary &nodeLib, dagbase::Lua &lua)
    {
        return new Bar(str, nodeLib, lua);
    }

    bool Bar::equals(const Node &other) const
    {
        return false;
    }

    std::array<dagbase::MetaPort, 1> BarTyped::ports =
            {
                    dagbase::MetaPort("out1", dagbase::PortType::TYPE_DOUBLE, dagbase::PortDirection::DIR_OUT)
            };

    BarTyped *BarTyped::create(dagbase::InputStream &str, dagbase::NodeLibrary &nodeLib, dagbase::Lua &lua)
    {
        return new BarTyped(str, nodeLib, lua);
    }

    BarTyped::BarTyped(dagbase::InputStream &str, dagbase::NodeLibrary &nodeLib, dagbase::Lua &lua)
            :
            Node()
    {
        std::string className;
        std::string fieldName;
        str.readHeader(&className);
        Node::readFromStream(str, nodeLib, lua);
        str.readField(&fieldName);
        dagbase::Stream::ObjId out1Id = 0;
        dagbase::Stream::Ref out1Ref = str.readRef(&out1Id);

        if (out1Id != 0)
        {
            if (out1Ref != nullptr)
            {
                _out1 = static_cast<dagbase::TypedPort<double>*>(out1Ref);
            }
            else
            {
                _out1 = dynamic_cast<dagbase::TypedPort<double>*>(nodeLib.instantiatePort(str, lua));
            }
        }
        str.readFooter();
    }

    dagbase::OutputStream &BarTyped::writeToStream(dagbase::OutputStream &str, dagbase::NodeLibrary& nodeLib, dagbase::Lua &lua) const
    {
        str.writeHeader("BarTyped");
        Node::writeToStream(str,  nodeLib, lua);
        str.writeField("out1");
        if (str.writeRef(_out1))
        {
            _out1->writeToStream(str, nodeLib, lua);
        }
        str.writeFooter();
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

    std::array<dagbase::MetaPort, 2> GroupTyped::ports =
            {
                    dagbase::MetaPort("out1", dagbase::PortType::TYPE_DOUBLE, dagbase::PortDirection::DIR_OUT),
                    dagbase::MetaPort("in1", dagbase::PortType::TYPE_DOUBLE, dagbase::PortDirection::DIR_IN)
            };

    GroupTyped::GroupTyped(dagbase::InputStream &str, dagbase::NodeLibrary &nodeLib, dagbase::Lua &lua)
            :
            Node(str, nodeLib, lua),
            _out1(str, nodeLib, lua),
            _in1(str, nodeLib, lua)
    {
        // Do nothing.
    }

    GroupTyped *GroupTyped::create(dagbase::InputStream &str, dagbase::NodeLibrary &nodeLib, dagbase::Lua &lua)
    {
        return new GroupTyped(str, nodeLib, lua);
    }

    dagbase::OutputStream &GroupTyped::writeToStream(dagbase::OutputStream &str, dagbase::NodeLibrary& nodeLib, dagbase::Lua &lua) const
    {
        Node::writeToStream(str, nodeLib, lua);
        _out1.writeToStream(str, nodeLib, lua);
        _in1.writeToStream(str, nodeLib, lua);

        return str;
    }

    bool GroupTyped::equals(const Node &other) const
    {
        return false;
    }

    void Base::describeNode(dagbase::NodeDescriptor& descriptor) const
    {
        descriptor.id = id();
        descriptor.name = name();
        descriptor.category = category();
        dagbase::MetaPort portDescriptor;
        //portDescriptor.id = _direction.id();
        portDescriptor.name = _direction.name();
        portDescriptor.type = _direction.type();
        portDescriptor.direction = _direction.dir();
        descriptor.ports.emplace_back(portDescriptor);
    }

    dagbase::Node *Base::create(dagbase::InputStream &str, dagbase::NodeLibrary& nodeLib, dagbase::Lua &lua)
    {
        return new Base(str, nodeLib, lua);
    }

    Base::Base(dagbase::InputStream &str, dagbase::NodeLibrary& nodeLib, dagbase::Lua &lua)
            :
            Node(str, nodeLib, lua),
            int1(0.0),
            _direction(str,nodeLib, lua)
    {
        str.read(&int1);
    }

    dagbase::OutputStream &Base::writeToStream(dagbase::OutputStream &str, dagbase::NodeLibrary& nodeLib, dagbase::Lua &lua) const
    {
        Node::writeToStream(str, nodeLib, lua);
        _direction.writeToStream(str, nodeLib, lua);
        str.write(int1);

        return str;
    }

    bool Base::equals(const Node &other) const
    {
        return false;
    }
}
