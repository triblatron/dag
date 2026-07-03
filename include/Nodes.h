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
                int1(0.0),
                _direction(new dagbase::TypedPort<double>(keyGen.nextPortID(), this, "direction", dagbase::PortType::TYPE_DOUBLE, dagbase::PortDirection::DIR_OUT, 1.0))
        {
            // Do nothing.
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

        [[nodiscard]]bool equals(const dagbase::Node& other) const override;

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
                return _direction;
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
        dagbase::TypedPort<double>* _direction{ nullptr };
    };

    class DAG_API Derived : public Base
    {
    public:
        Derived(dagbase::KeyGenerator& keyGen, const std::string& name, dagbase::NodeCategory::Category category)
                :
                Base(keyGen, name,category),
                _trigger(new dagbase::TypedPort<bool>(keyGen.nextPortID(), this, "trigger", dagbase::PortType::TYPE_BOOL, dagbase::PortDirection::DIR_IN, true))
        {
            // Do nothing.
        }

        Derived(const Derived& other, dagbase::CloningFacility& facility, dagbase::CopyOp copyOp, dagbase::KeyGenerator* keyGen);

        Derived(dagbase::InputStream& str, dagbase::NodeLibrary& nodeLib, dagbase::Lua& lua);

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
                return _trigger;
            }

            return nullptr;
        }

        const char* className() const override
        {
            return "Derived";
        }
    protected:
        static std::array<dagbase::MetaPort, 1> ports;
        static constexpr size_t firstPort = Base::numPorts;
        static constexpr size_t numPorts = 1;
    private:
        dagbase::TypedPort<bool>* _trigger{ nullptr };
    };

    class DAG_API Final final : public Derived
    {
    public:
        Final(dagbase::KeyGenerator& keyGen, const std::string& name, dagbase::NodeCategory::Category category)
                :
                Derived(keyGen, name,category),
                _int1(keyGen.nextPortID(), this, "int1", dagbase::PortType::TYPE_INT64, dagbase::PortDirection::DIR_INTERNAL, 1)
        {
            // Do nothing.
        }

        Final(const Final& other, dagbase::CloningFacility& facility, dagbase::CopyOp copyOp, dagbase::KeyGenerator* keyGen)
                :
                Derived(other, facility, copyOp, keyGen),
                _int1(other._int1.id(), this, other._int1.name(), other._int1.type(), other._int1.dir(), other._int1.value())
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

        void addDynamicPort(dagbase::Port* port, dagbase::MetaPort::Flags flags) override
        {
            if (port != nullptr)
            {
                _dynamicPorts.emplace_back(port);
                dagbase::MetaPort desc;
                _dynamicMetaPorts.emplace_back(desc);
            }
        }

        const char* className() const override
        {
            return "Final";
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

    class DAG_API FooTyped : public dagbase::Node
    {
    public:
        FooTyped(dagbase::KeyGenerator& keyGen, const std::string& name, dagbase::NodeCategory::Category category)
                :
                Node(keyGen, name, category)
        {
            _in1 = new dagbase::TypedPort<double>(keyGen.nextPortID(), this, "in1", dagbase::PortType::TYPE_DOUBLE, dagbase::PortDirection::DIR_IN, 1.0);
        }

        FooTyped(const FooTyped& other, dagbase::CloningFacility& facility, dagbase::CopyOp copyOp, dagbase::KeyGenerator* keyGen)
                :
                Node(other, facility, copyOp, keyGen)
        {
            std::uint64_t in1Id = 0;
            if (facility.putOrig(other._in1, &in1Id))
            {
                _in1 = new dagbase::TypedPort(*other._in1, facility, copyOp, keyGen);
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
            _out1 = new dagbase::TypedPort<double>(keyGen.nextPortID(), this, "out1", dagbase::PortType::TYPE_DOUBLE, dagbase::PortDirection::DIR_OUT, 1.0);
        }

        BarTyped(const BarTyped& other,dagbase::CloningFacility& facility, dagbase::CopyOp copyOp, dagbase::KeyGenerator* keyGen)
                :
                Node(other,facility,copyOp,keyGen)
        {
            std::uint64_t out1Id = 0;
            if (facility.putOrig(other._out1, &out1Id))
            {
                _out1 = new dagbase::TypedPort(*other._out1, facility, copyOp, keyGen);
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

        void debug(dagbase::DebugPrinter& printer) const override;
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
                Node(keyGen, name, category)
        {
            _out1 = new dagbase::TypedPort<double>(keyGen.nextPortID(), this, "out1", dagbase::PortType::TYPE_DOUBLE, dagbase::PortDirection::DIR_OUT, 1.0);
            _in1 = new dagbase::TypedPort<double>(keyGen.nextPortID(), this, "in1", dagbase::PortType::TYPE_DOUBLE, dagbase::PortDirection::DIR_IN, 2.0);
        }

        GroupTyped(const GroupTyped& other,dagbase::CloningFacility& facility, dagbase::CopyOp copyOp, dagbase::KeyGenerator* keyGen)
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

        [[nodiscard]]bool equals(const dagbase::Node& other) const override;

        dagbase::OutputStream& writeToStream(dagbase::OutputStream& str, dagbase::NodeLibrary& nodeLib, dagbase::Lua &lua) const override;

        dagbase::TypedPort<double>& out1()
        {
            return *_out1;
        }

        dagbase::TypedPort<double>& in1()
        {
            return *_in1;
        }

        dagbase::Port* dynamicPort(size_t index) override
        {
            if (index == 0)
            {
                return _out1;
            }
            if (index == 1)
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

        [[nodiscard]]size_t totalPorts() const override
        {
            return numPorts;
        }

        [[nodiscard]]const dagbase::MetaPort * dynamicMetaPort(size_t index) const override
        {
            return metaPort(index);
        }

        void debug(dagbase::DebugPrinter& printer) const override;
    protected:
        static std::array<dagbase::MetaPort, 2> ports;
        static constexpr size_t firstPort = 0;
        static constexpr size_t numPorts = 2;
    private:
        dagbase::TypedPort<double>* _out1{nullptr};
        dagbase::TypedPort<double>* _in1{nullptr};
    };
}
