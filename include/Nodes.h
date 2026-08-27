#pragma once

#include "config/Export.h"

#include "core/Node.h"
#include "core/TypedPort.h"
#include "core/MetaPort.h"
#include "core/Types.h"
#include "core/KeyGenerator.h"
#include "core/CloningFacility.h"

#include <string>
#include <array>

namespace dagbase
{
    class InputStream;
    class OutputStream;
}

namespace dag
{
    class DAG_API Base : public dagbase::Node
    {
    public:
        Base(dagbase::KeyGenerator& keyGen, const std::string& name, dagbase::NodeCategory::Category category)
                :
                Node(keyGen, name, category),
                int1(0.0)
        {
            addDynamicPort(new dagbase::TypedPort<double>(keyGen.nextPortID(), this, "direction", dagbase::PortType::TYPE_DOUBLE, dagbase::PortDirection::DIR_OUT, 1.0), dagbase::MetaPort::FLAGS_OWN_BIT);
        }

        Base(const Base& other, dagbase::CloningFacility& facility, dagbase::CopyOp copyOp, dagbase::KeyGenerator* keyGen);

        Base(dagbase::InputStream& str, dagbase::NodeLibrary& nodeLib, dagbase::Lua& lua);

        dagbase::OutputStream& writeToStream(dagbase::OutputStream& str, dagbase::NodeLibrary& nodeLib, dagbase::Lua &lua) const override;

        [[nodiscard]]const char* className() const override
        {
            return "Base";
        }

        Base* clone(dagbase::CloningFacility& facility, dagbase::CopyOp copyOp, dagbase::KeyGenerator* keyGen) override
        {
            return new Base(*this, facility, copyOp, keyGen);
        }

        dagbase::Node* create(dagbase::InputStream& str, dagbase::NodeLibrary& nodeLib, dagbase::Lua& lua) override;

        [[nodiscard]]bool equals(const dagbase::Node& other, dagbase::ComparisonFlags flags) const override;

        bool operator==(const Base &other) const;

        double int1;

        static dagbase::MetaPort* metaPort(size_t index)
        {
            if (index < firstPort + numPorts)
            {
                return &ports[index-firstPort];
            }

            return nullptr;
        }
        dagbase::InputStream& readFromStream(dagbase::InputStream& str, dagbase::NodeLibrary& nodeLib, dagbase::Lua& lua) override;
    protected:
        static std::array<dagbase::MetaPort, 1> ports;
        static constexpr size_t firstPort = 0;
        static constexpr size_t numPorts = ports.size();
        Base() = default;
    private:
    };

    class DAG_API Derived : public Base
    {
    public:
        Derived(dagbase::KeyGenerator& keyGen, const std::string& name, dagbase::NodeCategory::Category category)
                :
                Base(keyGen, name,category)
        {
            addDynamicPort(new dagbase::TypedPort<bool>(keyGen.nextPortID(), this, "trigger", dagbase::PortType::TYPE_BOOL, dagbase::PortDirection::DIR_IN, true), dagbase::MetaPort::FLAGS_OWN_BIT);
        }

        Derived(const Derived& other, dagbase::CloningFacility& facility, dagbase::CopyOp copyOp, dagbase::KeyGenerator* keyGen);

        Derived(dagbase::InputStream& str, dagbase::NodeLibrary& nodeLib, dagbase::Lua& lua);

        dagbase::OutputStream& writeToStream(dagbase::OutputStream& str, dagbase::NodeLibrary& nodeLib, dagbase::Lua &lua) const override;

        Derived* clone(dagbase::CloningFacility& facility, dagbase::CopyOp copyOp, dagbase::KeyGenerator* keyGen) override
        {
            return new Derived(*this, facility, copyOp, keyGen);
        }

        Derived* create(dagbase::InputStream& str, dagbase::NodeLibrary& nodeLib, dagbase::Lua &lua) override;

        bool equals(const Node& other, dagbase::ComparisonFlags flags) const override;

        [[nodiscard]]static dagbase::MetaPort * metaPort(size_t index)
        {
            if (index < firstPort)
            {
                return Base::metaPort(index);
            }
            if (index < firstPort + numPorts)
            {
                return &ports[index-firstPort];
            }

            return nullptr;
        }

        const char* className() const override
        {
            return "Derived";
        }

        dagbase::InputStream& readFromStream(dagbase::InputStream& str, dagbase::NodeLibrary& nodeLib, dagbase::Lua& lua) override;
    protected:
        static std::array<dagbase::MetaPort, 1> ports;
        static constexpr size_t firstPort = Base::numPorts;
        static constexpr size_t numPorts = 1;
        Derived() = default;
    };

    class DAG_API Final final : public Derived
    {
    public:
        Final(dagbase::KeyGenerator& keyGen, const std::string& name, dagbase::NodeCategory::Category category)
                :
                Derived(keyGen, name,category)

        {
            addDynamicPort(new dagbase::TypedPort<std::int64_t>(keyGen.nextPortID(), this, "int1", dagbase::PortType::TYPE_INT64, dagbase::PortDirection::DIR_INTERNAL, 1), dagbase::MetaPort::FLAGS_OWN_BIT);
        }

        Final(const Final& other, dagbase::CloningFacility& facility, dagbase::CopyOp copyOp, dagbase::KeyGenerator* keyGen)
                :
                Derived(other, facility, copyOp, keyGen)
        {
            // Do nothing.
        }

        Final(dagbase::InputStream& str, dagbase::NodeLibrary& nodeLib, dagbase::Lua& lua);

        ~Final() override = default;

        bool equals(const Node& other, dagbase::ComparisonFlags flags) const override;

        dagbase::OutputStream& writeToStream(dagbase::OutputStream& str, dagbase::NodeLibrary& nodeLib, dagbase::Lua &lua) const override;

        Final* clone(dagbase::CloningFacility& facility, dagbase::CopyOp copyOp, dagbase::KeyGenerator* keyGen) override
        {
            return new Final(*this, facility, copyOp, keyGen);
        }

        Final* create(dagbase::InputStream& str, dagbase::NodeLibrary& nodeLib, dagbase::Lua &lua) override;

        const char* className() const override
        {
            return "Final";
        }
    private:
        static std::array<dagbase::MetaPort, 1> ports;
        static constexpr size_t firstPort = Derived::firstPort + Derived::numPorts;
        static constexpr size_t numPorts = 1;
    };

    class DAG_API FooTyped : public dagbase::Node
    {
    public:
        FooTyped(dagbase::KeyGenerator& keyGen, const std::string& name, dagbase::NodeCategory::Category category)
                :
                Node(keyGen, name, category)
        {
            addDynamicPort(new dagbase::TypedPort<double>(keyGen.nextPortID(), this, "in1", dagbase::PortType::TYPE_DOUBLE, dagbase::PortDirection::DIR_IN, 1.0), dagbase::MetaPort::FLAGS_OWN_BIT);
        }

        FooTyped(const FooTyped& other, dagbase::CloningFacility& facility, dagbase::CopyOp copyOp, dagbase::KeyGenerator* keyGen)
                :
                Node(other, facility, copyOp, keyGen)
        {
            clonePorts(other, facility, copyOp, keyGen);
        }

        explicit FooTyped(dagbase::InputStream& str, dagbase::NodeLibrary & nodeLib, dagbase::Lua &lua);

        ~FooTyped() override;

        [[nodiscard]] const char* className() const override
        {
            return "FooTyped";
        }

        FooTyped* clone(dagbase::CloningFacility& facility, dagbase::CopyOp copyOp, dagbase::KeyGenerator* keyGen) override
        {
            return new FooTyped(*this,facility,copyOp,keyGen);
        }

        FooTyped* create(dagbase::InputStream& str, dagbase::NodeLibrary& nodeLib, dagbase::Lua &lua) override;

        [[nodiscard]]bool equals(const dagbase::Node& other, dagbase::ComparisonFlags flags) const override;

        dagbase::OutputStream& writeToStream(dagbase::OutputStream& str, dagbase::NodeLibrary& nodeLib, dagbase::Lua &lua) const override;

        dagbase::Port& in1()
        {
            return *dynamicPort(0);
        }

        static dagbase::MetaPort* metaPort(size_t index)
        {
            if (index < firstPort + numPorts)
            {
                return &ports[index-firstPort];
            }

            return nullptr;
        }

        void debug(dagbase::DebugPrinter& printer) const override;
    protected:
        static std::array<dagbase::MetaPort, 1> ports;
        static constexpr size_t firstPort = 0;
        static constexpr size_t numPorts = 1;
    };

    class DAG_API BarTyped : public dagbase::Node
    {
    public:
        BarTyped() = default;
        BarTyped(dagbase::KeyGenerator& keyGen, const std::string& name, dagbase::NodeCategory::Category category)
                :
                Node(keyGen, name, category)
        {
            addDynamicPort(new dagbase::TypedPort<double>(keyGen.nextPortID(), this, "out1", dagbase::PortType::TYPE_DOUBLE, dagbase::PortDirection::DIR_OUT, 1.0), dagbase::MetaPort::FLAGS_OWN_BIT);
        }

        BarTyped(const BarTyped& other,dagbase::CloningFacility& facility, dagbase::CopyOp copyOp, dagbase::KeyGenerator* keyGen)
                :
                Node(other,facility,copyOp,keyGen)
        {
            clonePorts(other, facility, copyOp, keyGen);
            dynamicPort(0)->setParent(this);
        }

        explicit BarTyped(dagbase::InputStream& str, dagbase::NodeLibrary& nodeLib, dagbase::Lua &lua);

        ~BarTyped() override;

        [[nodiscard]]const char* className() const override
        {
            return "BarTyped";
        }

        BarTyped* clone(dagbase::CloningFacility& facility, dagbase::CopyOp copyOp, dagbase::KeyGenerator* keyGen) override
        {
            return new BarTyped(*this,facility,copyOp,keyGen);
        }

        BarTyped* create(dagbase::InputStream& str, dagbase::NodeLibrary& nodeLib, dagbase::Lua &lua) override;

        [[nodiscard]]bool equals(const dagbase::Node& other, dagbase::ComparisonFlags flags) const override;

        dagbase::OutputStream& writeToStream(dagbase::OutputStream& str, dagbase::NodeLibrary& nodeLib, dagbase::Lua &lua) const override;

        dagbase::Port* out1()
        {
            return dynamicPort(0);
        }

        void debug(dagbase::DebugPrinter& printer) const override;
    protected:
        static std::array<dagbase::MetaPort, 1> ports;
        static constexpr size_t firstPort = 0;
        static constexpr size_t numPorts = 1;
    };

    class DAG_API GroupTyped : public dagbase::Node
    {
    public:
        GroupTyped(dagbase::KeyGenerator& keyGen, const std::string& name, dagbase::NodeCategory::Category category)
                :
                Node(keyGen, name, category)
        {
            addDynamicPort(new dagbase::TypedPort<double>(keyGen.nextPortID(), this, "out1", dagbase::PortType::TYPE_DOUBLE, dagbase::PortDirection::DIR_OUT, 1.0), dagbase::MetaPort::FLAGS_OWN_BIT);
            addDynamicPort(new dagbase::TypedPort<double>(keyGen.nextPortID(), this, "in1", dagbase::PortType::TYPE_DOUBLE, dagbase::PortDirection::DIR_IN, 2.0), dagbase::MetaPort::FLAGS_OWN_BIT);
        }

        GroupTyped(const GroupTyped& other,dagbase::CloningFacility& facility, dagbase::CopyOp copyOp, dagbase::KeyGenerator* keyGen)
                :
                Node(other,facility,copyOp,keyGen)
        {
            clonePorts(other, facility, copyOp, keyGen);
            dynamicPort(0)->setParent(this);
            dynamicPort(1)->setParent(this);
            // for (std::size_t portIndex = 0; portIndex < other.totalPorts(); ++portIndex)
            // {
            //     const dagbase::Port* p = other.dynamicPort(portIndex);
            //
            //     auto clonedPort = p->clone(facility, copyOp, keyGen);
            //     GroupTyped::addDynamicPort(clonedPort, dagbase::MetaPort::FLAGS_OWN_BIT);
            // }

        }

        explicit GroupTyped(dagbase::InputStream& str, dagbase::NodeLibrary& nodeLib, dagbase::Lua &lua);

        ~GroupTyped() override;

        [[nodiscard]]const char* className() const override
        {
            return "GroupTyped";
        }

        GroupTyped* clone(dagbase::CloningFacility& facility, dagbase::CopyOp copyOp, dagbase::KeyGenerator* keyGen) override
        {
            return new GroupTyped(*this, facility, copyOp, keyGen);
        }

        GroupTyped* create(dagbase::InputStream& str, dagbase::NodeLibrary& nodeLib, dagbase::Lua &lua) override;

        [[nodiscard]]bool equals(const dagbase::Node& other, dagbase::ComparisonFlags flags) const override;

        dagbase::OutputStream& writeToStream(dagbase::OutputStream& str, dagbase::NodeLibrary& nodeLib, dagbase::Lua &lua) const override;

        dagbase::Port& out1()
        {
            return *dynamicPort(0);
        }

        dagbase::Port& in1()
        {
            return *dynamicPort(1);
        }

        static dagbase::MetaPort* metaPort(size_t index)
        {
            if (index < firstPort + numPorts)
            {
                return &ports[index-firstPort];
            }

            return nullptr;
        }

        void debug(dagbase::DebugPrinter& printer) const override;
    protected:
        static std::array<dagbase::MetaPort, 2> ports;
        static constexpr size_t firstPort = 0;
        static constexpr size_t numPorts = 2;
    };
}
