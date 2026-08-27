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
        str.readFooter();
    }

    dagbase::OutputStream &FooTyped::writeToStream(dagbase::OutputStream &str, dagbase::NodeLibrary& nodeLib, dagbase::Lua &lua) const
    {
        str.writeHeader("FooTyped");
        Node::writeToStream(str, nodeLib, lua);
        str.writeFooter();
        return str;
    }

    bool FooTyped::equals(const Node &other, dagbase::ComparisonFlags flags) const
    {
        if (!Node::equals(other, flags))
        {
            return false;
        }

        return true;
    }

    void FooTyped::debug(dagbase::DebugPrinter& printer) const
    {
        Node::debug(printer);
        printer.indent();
        printer.outdent();
        printer.indent();
        for (std::size_t i = 0; i < totalPorts(); ++i)
        {
            dynamicMetaPort(i)->debug(printer);
        }
        printer.outdent();
    }

    FooTyped::~FooTyped()
    {
        deleteDynamicPorts();
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
        str.readFooter();
    }

    dagbase::OutputStream &BarTyped::writeToStream(dagbase::OutputStream &str, dagbase::NodeLibrary& nodeLib, dagbase::Lua &lua) const
    {
        str.writeHeader("BarTyped");
        Node::writeToStream(str,  nodeLib, lua);
        str.writeFooter();
        return str;
    }

    void BarTyped::debug(dagbase::DebugPrinter &printer) const
    {
        Node::debug(printer);
        printer.indent();
        for (std::size_t i = 0; i < totalPorts(); ++i)
        {
            dynamicMetaPort(i)->debug(printer);
        }
        printer.outdent();
    }

    bool BarTyped::equals(const Node &other, dagbase::ComparisonFlags flags) const
    {
        if (!Node::equals(other, flags))
        {
            return false;
        }

        return true;
    }

    BarTyped::~BarTyped()
    {
        deleteDynamicPorts();
    }

    std::array<dagbase::MetaPort, 2> GroupTyped::ports =
            {
                    dagbase::MetaPort(dagbase::MetaPort::FLAGS_OWN_BIT),
                    dagbase::MetaPort(dagbase::MetaPort::FLAGS_OWN_BIT)
            };

    GroupTyped::GroupTyped(dagbase::InputStream &str, dagbase::NodeLibrary &nodeLib, dagbase::Lua &lua)
            :
            Node()
    {
        std::string className;

        std::string fieldName;
        str.readHeader(&className);
        Node::readFromStream(str, nodeLib, lua);
        str.readFooter();
    }

    GroupTyped::~GroupTyped()
    {
        deleteDynamicPorts();
    }

    GroupTyped *GroupTyped::create(dagbase::InputStream &str, dagbase::NodeLibrary &nodeLib, dagbase::Lua &lua)
    {
        return new GroupTyped(str, nodeLib, lua);
    }

    dagbase::OutputStream &GroupTyped::writeToStream(dagbase::OutputStream &str, dagbase::NodeLibrary& nodeLib, dagbase::Lua &lua) const
    {
        str.writeHeader("GroupTyped");
        Node::writeToStream(str,  nodeLib, lua);
        str.writeFooter();
        return str;
        // Node::writeToStream(str, nodeLib, lua);
        // _out1->writeToStream(str, nodeLib, lua);
        // _in1->writeToStream(str, nodeLib, lua);

        // return str;
    }

    bool GroupTyped::equals(const Node &other, dagbase::ComparisonFlags flags) const
    {
        if (!Node::equals(other, flags))
            return false;

        return true;
    }

    void GroupTyped::debug(dagbase::DebugPrinter& printer) const
    {
        Node::debug(printer);
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
        clonePorts(other, facility, copyOp, keyGen);
    }

    Base::Base(dagbase::InputStream& str, dagbase::NodeLibrary& nodeLib, dagbase::Lua& lua)
        :
        Node(),
        int1(0.0)
    {
        Base::readFromStream(str, nodeLib, lua);
    }

    dagbase::InputStream & Base::readFromStream(dagbase::InputStream &str, dagbase::NodeLibrary &nodeLib, dagbase::Lua &lua)
    {
        std::string className;
        std::string fieldName;
        str.readHeader(&className);
        Node::readFromStream(str, nodeLib, lua);
        str.readFooter();
        return str;
    }

    dagbase::OutputStream& Base::writeToStream(dagbase::OutputStream& str, dagbase::NodeLibrary& nodeLib, dagbase::Lua& lua) const
    {
        str.writeHeader("Base");
        Node::writeToStream(str, nodeLib, lua);
        str.writeFooter();
        return str;
    }

    bool Base::equals(const Node& other, dagbase::ComparisonFlags flags) const
    {
        if (!Node::equals(other, flags))
            return false;

        return true;
    }

    bool Base::operator==(const Base& other) const
    {
        if (this == &other)
            return true;


        return true;
    }

    Derived::Derived(const Derived& other, dagbase::CloningFacility& facility, dagbase::CopyOp copyOp, dagbase::KeyGenerator* keyGen)
        :
        Base(other, facility, copyOp, keyGen)
    {
    }

    Derived::Derived(dagbase::InputStream& str, dagbase::NodeLibrary& nodeLib, dagbase::Lua& lua)
        :
        Base()
    {
        Derived::readFromStream(str, nodeLib, lua);
    }

    dagbase::InputStream & Derived::readFromStream(dagbase::InputStream &str, dagbase::NodeLibrary &nodeLib,
        dagbase::Lua &lua)
    {
        std::string className;
        std::string fieldName;
        str.readHeader(&className);
        Base::readFromStream(str, nodeLib, lua);
        str.readFooter();

        return str;
    }

    dagbase::OutputStream & Derived::writeToStream(dagbase::OutputStream &str, dagbase::NodeLibrary &nodeLib,
        dagbase::Lua &lua) const
    {
        str.writeHeader("Derived");
        Base::writeToStream(str, nodeLib, lua);
        str.writeFooter();

        return str;
    }

    Derived * Derived::create(dagbase::InputStream &str, dagbase::NodeLibrary &nodeLib, dagbase::Lua &lua)
    {
        return new Derived(str, nodeLib, lua);
    }

    bool Derived::equals(const Node& other, dagbase::ComparisonFlags flags) const
    {
        if (!Base::equals(other, flags))
            return false;

        return true;
    }

    Final::Final(dagbase::InputStream &str, dagbase::NodeLibrary &nodeLib, dagbase::Lua &lua)
        :
    Derived()
    {
        std::string className;
        std::string fieldName;
        str.readHeader(&className);
        Derived::readFromStream(str, nodeLib, lua);
        str.readFooter();
    }

    bool Final::equals(const Node& other, dagbase::ComparisonFlags flags) const
    {
        if (!Derived::equals(other, flags))
            return false;

        return true;
    }

    dagbase::OutputStream & Final::writeToStream(dagbase::OutputStream &str, dagbase::NodeLibrary &nodeLib,
        dagbase::Lua &lua) const
    {
        str.writeHeader("Final");
        Derived::writeToStream(str, nodeLib, lua);
        str.writeFooter();

        return str;
    }

    Final * Final::create(dagbase::InputStream &str, dagbase::NodeLibrary &nodeLib, dagbase::Lua &lua)
    {
        return new Final(str, nodeLib, lua);
    }
}
