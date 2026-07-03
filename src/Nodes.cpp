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
                    dagbase::MetaPort{dagbase::MetaPort::FLAGS_OWN_BIT}
            };

    std::array<dagbase::MetaPort, 1> Derived::ports =
            {
                    dagbase::MetaPort(dagbase::MetaPort::FLAGS_OWN_BIT)
            };

    std::array<dagbase::MetaPort, 1> Final::ports=
            {
                    dagbase::MetaPort(dagbase::MetaPort::FLAGS_OWN_BIT)
            };

    std::array<dagbase::MetaPort, 1> FooTyped::ports =
            {
                    dagbase::MetaPort(dagbase::MetaPort::FLAGS_OWN_BIT)
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
        printer.indent();
        _in1->debug(printer);
        printer.outdent();
    }

    FooTyped::~FooTyped()
    {
        delete _in1;
    }

    std::array<dagbase::MetaPort, 1> BarTyped::ports =
            {
                    dagbase::MetaPort(dagbase::MetaPort::FLAGS_OWN_BIT)
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

    void BarTyped::debug(dagbase::DebugPrinter &printer) const
    {
        Node::debug(printer);
        printer.indent();
        _out1->debug(printer);
        printer.outdent();
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
                    dagbase::MetaPort(dagbase::MetaPort::FLAGS_OWN_BIT),
                    dagbase::MetaPort(dagbase::MetaPort::FLAGS_OWN_BIT)
            };

    GroupTyped::GroupTyped(dagbase::InputStream &str, dagbase::NodeLibrary &nodeLib, dagbase::Lua &lua)
            :
            Node(str, nodeLib, lua)
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

    GroupTyped::~GroupTyped()
    {
        delete _out1;
        delete _in1;
    }

    GroupTyped *GroupTyped::create(dagbase::InputStream &str, dagbase::NodeLibrary &nodeLib, dagbase::Lua &lua)
    {
        return new GroupTyped(str, nodeLib, lua);
    }

    dagbase::OutputStream &GroupTyped::writeToStream(dagbase::OutputStream &str, dagbase::NodeLibrary& nodeLib, dagbase::Lua &lua) const
    {
        str.writeHeader("GroupTyped");
        Node::writeToStream(str,  nodeLib, lua);
        str.writeField("out1");
        if (str.writeRef(_out1))
        {
            _out1->writeToStream(str, nodeLib, lua);
        }
        if (str.writeRef(_in1))
        {
            _in1->writeToStream(str, nodeLib, lua);
        }
        str.writeFooter();
        return str;
        // Node::writeToStream(str, nodeLib, lua);
        // _out1->writeToStream(str, nodeLib, lua);
        // _in1->writeToStream(str, nodeLib, lua);

        // return str;
    }

    bool GroupTyped::equals(const Node &other) const
    {
        return false;
    }

    void GroupTyped::debug(dagbase::DebugPrinter& printer) const
    {
        Node::debug(printer);
        printer.indent();
        _out1->debug(printer);
        _in1->debug(printer);
        printer.outdent();
    }

    dagbase::Node *Base::create(dagbase::InputStream &str, dagbase::NodeLibrary& nodeLib, dagbase::Lua &lua)
    {
        return new Base(str, nodeLib, lua);
    }

    Base::Base(const Base& other, dagbase::CloningFacility& facility, dagbase::CopyOp copyOp, dagbase::KeyGenerator* keyGen)
        :
        Node(other, facility, copyOp, keyGen),
        int1(other.int1)
    {
        std::uint64_t directionId = 0;
        if (facility.putOrig(other._direction, &directionId))
        {
            _direction = new dagbase::TypedPort(*other._direction, facility, copyOp, keyGen);
        }
        else
        {
            _direction = static_cast<dagbase::TypedPort<double>*>(facility.getClone(directionId));
        }
        _direction->setParent(this);
    }

    Base::Base(dagbase::InputStream &str, dagbase::NodeLibrary& nodeLib, dagbase::Lua &lua)
            :
            Node(str, nodeLib, lua),
            int1(0.0)
    {
        dagbase::Stream::ObjId directionId = 0;
        dagbase::Stream::Ref directionRef = str.readRef(&directionId);

        if (directionId != 0)
        {
            if (directionRef != nullptr)
            {
                _direction = static_cast<dagbase::TypedPort<double>*>(directionRef);
            }
            else
            {
                _direction = dynamic_cast<dagbase::TypedPort<double>*>(nodeLib.instantiatePort(str, lua));
            }
        }
        str.read(&int1);
    }

    dagbase::OutputStream &Base::writeToStream(dagbase::OutputStream &str, dagbase::NodeLibrary& nodeLib, dagbase::Lua &lua) const
    {
        Node::writeToStream(str, nodeLib, lua);
        _direction->writeToStream(str, nodeLib, lua);
        str.write(int1);

        return str;
    }

    bool Base::equals(const Node &other) const
    {
        return false;
    }

    Derived::Derived(const Derived& other, dagbase::CloningFacility& facility, dagbase::CopyOp copyOp, dagbase::KeyGenerator* keyGen)
        :
        Base(other, facility, copyOp, keyGen)
    {
        std::uint64_t triggerId = 0;
        if (facility.putOrig(other._trigger, &triggerId))
        {
            _trigger = new dagbase::TypedPort(*other._trigger, facility, copyOp, keyGen);
        }
        else
        {
            _trigger = static_cast<dagbase::TypedPort<bool>*>(facility.getClone(triggerId));
        }
        _trigger->setParent(this);
    }

    Derived::Derived(dagbase::InputStream& str, dagbase::NodeLibrary& nodeLib, dagbase::Lua& lua)
        :
        Base(str, nodeLib, lua)
    {
        dagbase::Stream::ObjId directionId = 0;
        dagbase::Stream::Ref directionRef = str.readRef(&directionId);

        if (directionId != 0)
        {
            if (directionRef != nullptr)
            {
                _trigger = static_cast<dagbase::TypedPort<bool>*>(directionRef);
            }
            else
            {
                _trigger = dynamic_cast<dagbase::TypedPort<bool>*>(nodeLib.instantiatePort(str, lua));
            }
        }
    }
}
