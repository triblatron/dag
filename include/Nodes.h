#pragma once

#include "config/Export.h"

#include "core/Node.h"
#include "../thirdparty/dagbase/include/core/TypedPort.h"
#include "core/NodeDescriptor.h"
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
    class DAG_API Foo : public dagbase::Node
    {
    public:
        Foo(dagbase::KeyGenerator& keyGen, const std::string& name, dagbase::NodeCategory::Category category)
                :
                Node(keyGen, name, category),
                in1(keyGen.nextPortID(), & ports[0], dagbase::Value(1.0), this)
        {
        }

        Foo(const Foo& other, dagbase::CloningFacility& facility, dagbase::CopyOp copyOp, dagbase::KeyGenerator* keyGen)
                :
                Node(other,facility,copyOp,keyGen),
                in1(other.in1.id(), other.in1.metaPort(), other.in1.value(), this)
        {
            // Do nothing.
        }

        explicit Foo(dagbase::InputStream& str, dagbase::NodeLibrary& nodeLib, dagbase::Lua& lua);

        [[nodiscard]] const char* className() const override
        {
            return "Foo";
        }

        Foo* clone(dagbase::CloningFacility& facility, dagbase::CopyOp copyOp, dagbase::KeyGenerator* keyGen) override
        {
            return new Foo(*this, facility, copyOp, keyGen);
        }

        Foo* create(dagbase::InputStream& str, dagbase::NodeLibrary& nodeLib, dagbase::Lua &lua) override;

        [[nodiscard]]bool equals(const dagbase::Node& other) const override;

        void describeNode(dagbase::NodeDescriptor& descriptor) const override
        {
            dagbase::Node::describeNode(descriptor);
            descriptor.ports.emplace_back(in1.name(), in1.type(), in1.dir());
        }

        [[nodiscard]] size_t totalPorts() const override
        {
            return numPorts;
        }

        dagbase::Port* dynamicPort(size_t index) override
        {
            if (index == 0)
            {
                return &in1;
            }

            return nullptr;
        }

        dagbase::ValuePort in1;

        static dagbase::MetaPort const* metaPort(size_t index)
        {
            if (index < firstPort + numPorts)
            {
                return &ports[index-firstPort];
            }

            return nullptr;
        }

        [[nodiscard]] const dagbase::MetaPort * dynamicMetaPort(size_t index) const override
        {
            return metaPort(index);
        }

        dagbase::OutputStream& writeToStream(dagbase::OutputStream& str, dagbase::NodeLibrary& nodeLib, dagbase::Lua &lua) const override;
    protected:
        static std::array<dagbase::MetaPort, 1> ports;
        static constexpr size_t firstPort = 0;
        static constexpr size_t numPorts = ports.size();
    };

    class DAG_API Base : public dagbase::Node
    {
    public:
        Base(dagbase::KeyGenerator& keyGen, const std::string& name, dagbase::NodeCategory::Category category)
                :
                Node(keyGen, name, category),
                int1(0.0),
                _direction(keyGen.nextPortID(), this, &ports[0], 1.0)
        {
            // Do nothing.
        }

        explicit Base(const Base& other, dagbase::CloningFacility& facility, dagbase::CopyOp copyOp, dagbase::KeyGenerator* keyGen)
                :
                Node(other,facility,copyOp,keyGen),
                int1(other.int1),
                _direction(other._direction.id(), this, &ports[0], other._direction.value())
        {
            // Do nothing.
        }

        explicit Base(dagbase::InputStream& str, dagbase::NodeLibrary& nodeLib, dagbase::Lua& lua);

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

        [[nodiscard]]bool equals(const dagbase::Node& other) const override;

        void describeNode(dagbase::NodeDescriptor& descriptor) const override;

        double int1;

        [[nodiscard]]size_t totalPorts() const override
        {
            return numPorts;
        }

        [[nodiscard]]const dagbase::MetaPort * dynamicMetaPort(size_t index) const override
        {
            return metaPort(index);
        }

        dagbase::Port* dynamicPort(size_t index) override
        {
            if (index == 0)
            {
                return &_direction;
            }

            return nullptr;
        }

        static dagbase::MetaPort const* metaPort(size_t index)
        {
            if (index < firstPort + numPorts)
            {
                return &ports[index-firstPort];
            }

            return nullptr;
        }
    protected:
        static std::array<dagbase::MetaPort, 1> ports;
        static constexpr size_t firstPort = 0;
        static constexpr size_t numPorts = ports.size();
    private:
        dagbase::TypedPort<double> _direction;
    };

    class DAG_API Derived : public Base
    {
    public:
        Derived(dagbase::KeyGenerator& keyGen, const std::string& name, dagbase::NodeCategory::Category category)
                :
                Base(keyGen, name,category),
                _trigger(keyGen.nextPortID(), this, &ports[0], true)
        {
            // Do nothing.
        }

        Derived(const Derived& other, dagbase::CloningFacility& facility, dagbase::CopyOp copyOp, dagbase::KeyGenerator* keyGen)
                :
                Base(other, facility, copyOp, keyGen),
                _trigger(other._trigger.id(), this, &ports[0], other._trigger.value())
        {
            // Do nothing.
        }

        Derived* clone(dagbase::CloningFacility& facility, dagbase::CopyOp copyOp, dagbase::KeyGenerator* keyGen) override
        {
            return new Derived(*this, facility, copyOp, keyGen);
        }

        [[nodiscard]]static const dagbase::MetaPort * metaPort(size_t index)
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

        [[nodiscard]]size_t totalPorts() const override
        {
            return Base::totalPorts() + numPorts;
        }

        [[nodiscard]]dagbase::MetaPort const* dynamicMetaPort(size_t index) const override
        {
            return metaPort(index);
        }

        [[nodiscard]]dagbase::Port* dynamicPort(size_t index) override
        {
            if (index<firstPort)
            {
                return Base::dynamicPort(index);
            }
            if (index == 1)
            {
                return &_trigger;
            }

            return nullptr;
        }
    protected:
        static std::array<dagbase::MetaPort, 1> ports;
        static constexpr size_t firstPort = Base::numPorts;
        static constexpr size_t numPorts = 1;
    private:
        dagbase::TypedPort<bool> _trigger;
    };

    class DAG_API Final final : public Derived
    {
    public:
        Final(dagbase::KeyGenerator& keyGen, const std::string& name, dagbase::NodeCategory::Category category)
                :
                Derived(keyGen, name,category),
                _int1(keyGen.nextPortID(), this, &ports[0], 1)
        {
            // Do nothing.
        }

        Final(const Final& other, dagbase::CloningFacility& facility, dagbase::CopyOp copyOp, dagbase::KeyGenerator* keyGen)
                :
                Derived(other, facility, copyOp, keyGen),
                _int1(other._int1.id(), this, &ports[0], other._int1.value())
        {
            // Do nothing.
        }

        ~Final() override
        {
            for (auto p : _dynamicPorts)
            {
                delete p;
            }
        }

        Final* clone(dagbase::CloningFacility& facility, dagbase::CopyOp copyOp, dagbase::KeyGenerator* keyGen) override
        {
            return new Final(*this, facility, copyOp, keyGen);
        }

        void addDynamicPort(dagbase::Port* port) override
        {
            if (port != nullptr)
            {
                _dynamicPorts.emplace_back(port);
                dagbase::MetaPort desc;
                desc.name = port->name();
                desc.type = port->type();
                desc.direction = port->dir();
                _dynamicMetaPorts.emplace_back(desc);
            }
        }

        [[nodiscard]]const dagbase::MetaPort * dynamicMetaPort(size_t index) const override
        {
            if (index < firstPort)
            {
                return Derived::dynamicMetaPort(index);
            }

            if (index < firstPort + numPorts)
            {
                return &ports[index-firstPort];
            }

            if (index < firstPort + numPorts + _dynamicMetaPorts.size())
            {
                return &_dynamicMetaPorts[index - (firstPort+numPorts)];
            }

            return nullptr;
        }

        [[nodiscard]]size_t totalPorts() const override
        {
            return Derived::totalPorts() + numPorts + _dynamicMetaPorts.size();
        }

        [[nodiscard]]dagbase::Port* dynamicPort(size_t index) override
        {
            if (index<firstPort)
            {
                return Derived::dynamicPort(index);
            }

            if (index == firstPort)
            {
                return &_int1;
            }

            if (index < firstPort + numPorts + _dynamicPorts.size())
            {
                return _dynamicPorts[index - (firstPort+numPorts)];
            }

            return nullptr;
        }
    private:
        dagbase::TypedPort<std::int64_t> _int1;
        static std::array<dagbase::MetaPort, 1> ports;
        typedef std::vector<dagbase::MetaPort> MetaPortArray;
        MetaPortArray _dynamicMetaPorts;
        typedef std::vector<dagbase::Port*> PortArray;
        PortArray _dynamicPorts;
        static constexpr size_t firstPort = Derived::firstPort + Derived::numPorts;
        static constexpr size_t numPorts = 1;
    };

    class DAG_API Bar : public dagbase::Node
    {
    public:
        Bar(dagbase::KeyGenerator& keyGen, const std::string& name, dagbase::NodeCategory::Category category)
                :
                Node(keyGen, name, category),
                out1(keyGen.nextPortID(), & ports[0], dagbase::Value(1.0), this)
        {
        }

        Bar(const Bar& other, dagbase::CloningFacility& facility, dagbase::CopyOp copyOp, dagbase::KeyGenerator* keyGen)
                :
                Node(other, facility, copyOp, keyGen),
                out1(other.out1.id(), other.out1.metaPort(), other.out1.value(), this)
        {
            // Do nothing.
        }

        explicit Bar(dagbase::InputStream& str, dagbase::NodeLibrary& nodeLib, dagbase::Lua &lua);

        dagbase::OutputStream& writeToStream(dagbase::OutputStream& str, dagbase::NodeLibrary& nodeLib, dagbase::Lua &lua) const override;

        [[nodiscard]]const char* className() const override
        {
            return "Bar";
        }

        Bar* clone(dagbase::CloningFacility& facility, dagbase::CopyOp copyOp, dagbase::KeyGenerator* keyGen) override
        {
            return new Bar(*this,facility, copyOp, keyGen);
        }

        Bar* create(dagbase::InputStream& str, dagbase::NodeLibrary& nodeLib, dagbase::Lua &lua) override;

        [[nodiscard]]bool equals(const dagbase::Node& other) const override;

        [[nodiscard]]size_t totalPorts() const override
        {
            return numPorts;
        }

        void describeNode(dagbase::NodeDescriptor& descriptor) const override
        {
            Node::describeNode(descriptor);
            descriptor.ports.emplace_back(out1.name(), out1.type(), out1.dir());
        }

        [[nodiscard]]const dagbase::MetaPort * dynamicMetaPort(size_t index) const override
        {
            if (index == 0)
            {
                return &ports[0];
            }

            return nullptr;
        }

        dagbase::Port* dynamicPort(size_t index) override
        {
            if (index == 0)
            {
                return &out1;
            }

            return nullptr;
        }

        dagbase::ValuePort out1;

        static dagbase::MetaPort const* metaPort(size_t index)
        {
            if (index < firstPort + numPorts)
            {
                return &ports[index-firstPort];
            }

            return nullptr;
        }
    protected:
        static std::array<dagbase::MetaPort, 1> ports;
        static constexpr size_t firstPort = 0;
        static constexpr size_t numPorts = 1;
    };

    class DAG_API FooTyped : public dagbase::Node
    {
    public:
        FooTyped(dagbase::KeyGenerator& keyGen, const std::string& name, dagbase::NodeCategory::Category category)
                :
                Node(keyGen, name, category)
        {
            _in1 = new dagbase::TypedPort<double>(keyGen.nextPortID(), this, &ports[0], 1.0);
        }

        FooTyped(const FooTyped& other, dagbase::CloningFacility& facility, dagbase::CopyOp copyOp, dagbase::KeyGenerator* keyGen)
                :
                Node(other, facility, copyOp, keyGen)
        {
            std::uint64_t in1Id = 0;
            if (facility.putOrig(other._in1, &in1Id))
            {
                _in1 = new dagbase::TypedPort<double>(*other._in1, facility, copyOp, keyGen);
            }
            else
            {
                _in1 = static_cast<dagbase::TypedPort<double>*>(facility.getClone(in1Id));
            }
            _in1->setParent(this);
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

        [[nodiscard]]bool equals(const dagbase::Node& other) const override;

        dagbase::OutputStream& writeToStream(dagbase::OutputStream& str, dagbase::NodeLibrary& nodeLib, dagbase::Lua &lua) const override;

        void describeNode(dagbase::NodeDescriptor& descriptor) const override
        {
            Node::describeNode(descriptor);
            descriptor.ports.emplace_back(_in1->name(), _in1->type(), _in1->dir());
        }

        dagbase::TypedPort<double>& in1()
        {
            return *_in1;
        }

        [[nodiscard]]size_t totalPorts() const override
        {
            return size_t{1};
        }

        dagbase::Port* dynamicPort(size_t index) override
        {
            if (index == 0)
            {
                return _in1;
            }

            return nullptr;
        }

        static dagbase::MetaPort const* metaPort(size_t index)
        {
            if (index < firstPort + numPorts)
            {
                return &ports[index-firstPort];
            }

            return nullptr;
        }

        [[nodiscard]]const dagbase::MetaPort * dynamicMetaPort(size_t index) const override
        {
            return metaPort(index);
        }

        void debug(dagbase::DebugPrinter& printer) const override;
    protected:
        static std::array<dagbase::MetaPort, 1> ports;
        static constexpr size_t firstPort = 0;
        static constexpr size_t numPorts = 1;
    private:
        dagbase::TypedPort<double>* _in1{nullptr};
    };

    class DAG_API BarTyped : public dagbase::Node
    {
    public:
        BarTyped() = default;
        BarTyped(dagbase::KeyGenerator& keyGen, const std::string& name, dagbase::NodeCategory::Category category)
                :
                Node(keyGen, name, category)
        {
            _out1 = new dagbase::TypedPort<double>(keyGen.nextPortID(), this, &ports[0], 1.0);
        }

        BarTyped(const BarTyped& other,dagbase::CloningFacility& facility, dagbase::CopyOp copyOp, dagbase::KeyGenerator* keyGen)
                :
                Node(other,facility,copyOp,keyGen)
        {
            std::uint64_t out1Id = 0;
            if (facility.putOrig(other._out1, &out1Id))
            {
                _out1 = new dagbase::TypedPort<double>(*other._out1, facility, copyOp, keyGen);
            }
            else
            {
                _out1 = static_cast<dagbase::TypedPort<double>*>(facility.getClone(out1Id));
            }
            _out1->setParent(this);
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

        [[nodiscard]]bool equals(const dagbase::Node& other) const override;

        dagbase::OutputStream& writeToStream(dagbase::OutputStream& str, dagbase::NodeLibrary& nodeLib, dagbase::Lua &lua) const override;

        void describeNode(dagbase::NodeDescriptor& descriptor) const override
        {
            Node::describeNode(descriptor);
            if (_out1 != nullptr)
            {
                descriptor.ports.emplace_back(_out1->name(), _out1->type(), _out1->dir());
            }
        }

        dagbase::TypedPort<double>* out1()
        {
            return _out1;
        }

        [[nodiscard]]size_t totalPorts() const override
        {
            return size_t{1};
        }
        dagbase::Port* dynamicPort(size_t index) override
        {
            if (index == 0)
            {
                return _out1;
            }

            return nullptr;
        }

        static dagbase::MetaPort const* metaPort(size_t index)
        {
            if (index < firstPort + numPorts)
            {
                return &ports[index-firstPort];
            }

            return nullptr;
        }

        [[nodiscard]]const dagbase::MetaPort * dynamicMetaPort(size_t index) const override
        {
            return metaPort(index);
        }
    protected:
        static std::array<dagbase::MetaPort, 1> ports;
        static constexpr size_t firstPort = 0;
        static constexpr size_t numPorts = 1;

    private:
        dagbase::TypedPort<double>* _out1{nullptr};
    };

    class DAG_API GroupTyped : public dagbase::Node
    {
    public:
        GroupTyped(dagbase::KeyGenerator& keyGen, const std::string& name, dagbase::NodeCategory::Category category)
                :
                Node(keyGen, name, category),
                _out1(keyGen.nextPortID(), this, &ports[0], 1.0),
                _in1(keyGen.nextPortID(), this, &ports[1], 2.0)
        {
            // Do nothing.
        }

        GroupTyped(const GroupTyped& other,dagbase::CloningFacility& facility, dagbase::CopyOp copyOp, dagbase::KeyGenerator* keyGen)
                :
                Node(other,facility,copyOp,keyGen),
                _out1(other._out1, facility, copyOp, keyGen),
                _in1(other._in1, facility, copyOp, keyGen)
        {
            _out1.setParent(this);
            _in1.setParent(this);
        }

        explicit GroupTyped(dagbase::InputStream& str, dagbase::NodeLibrary& nodeLib, dagbase::Lua &lua);

        [[nodiscard]]const char* className() const override
        {
            return "GroupTyped";
        }

        GroupTyped* clone(dagbase::CloningFacility& facility, dagbase::CopyOp copyOp, dagbase::KeyGenerator* keyGen) override
        {
            return new GroupTyped(*this, facility, copyOp, keyGen);
        }

        GroupTyped* create(dagbase::InputStream& str, dagbase::NodeLibrary& nodeLib, dagbase::Lua &lua) override;

        [[nodiscard]]bool equals(const dagbase::Node& other) const override;

        dagbase::OutputStream& writeToStream(dagbase::OutputStream& str, dagbase::NodeLibrary& nodeLib, dagbase::Lua &lua) const override;

        void describeNode(dagbase::NodeDescriptor& descriptor) const override
        {
            dagbase::Node::describeNode(descriptor);
            descriptor.ports.emplace_back(_out1.name(), _out1.type(), _out1.dir());
            descriptor.ports.emplace_back(_in1.name(), _in1.type(), _in1.dir());
        }

        dagbase::TypedPort<double>& out1()
        {
            return _out1;
        }

        dagbase::TypedPort<double>& in1()
        {
            return _in1;
        }

        dagbase::Port* dynamicPort(size_t index) override
        {
            if (index == 0)
            {
                return &_out1;
            }
            if (index == 1)
            {
                return &_in1;
            }

            return nullptr;
        }

        static dagbase::MetaPort const* metaPort(size_t index)
        {
            if (index < firstPort + numPorts)
            {
                return &ports[index-firstPort];
            }

            return nullptr;
        }

        [[nodiscard]]size_t totalPorts() const override
        {
            return numPorts;
        }

        [[nodiscard]]const dagbase::MetaPort * dynamicMetaPort(size_t index) const override
        {
            return metaPort(index);
        }
    protected:
        static std::array<dagbase::MetaPort, 2> ports;
        static constexpr size_t firstPort = 0;
        static constexpr size_t numPorts = 2;
    private:
        dagbase::TypedPort<double> _out1;
        dagbase::TypedPort<double> _in1;
    };
}
