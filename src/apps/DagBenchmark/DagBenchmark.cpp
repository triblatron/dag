// ReSharper disable CppInconsistentNaming
#include <benchmark/benchmark.h>

#include "SignalPathDef.h"
#include "UnionValue.h"
#include "Value.h"
#include "Port.h"
#include "MemoryNodeLibrary.h"
#include "Nodes.h"
#include "NodeDescriptor.h"
#include "MetaPort.h"

#include "TypedPort.h"
#include "SelectionLive.h"
#include "Graph.h"

class DataSink
{
public:
    DataSink()
	    :
    _foo{}
    {
	    // Do nothing.
    }

    void setFoo(int const& foo)
    {
        _foo = foo;
    }
private:
    int _foo;
};

class DataSource
{
public:
    explicit DataSource(int foo)
	    :
    _foo(foo)
    {
	    // Do nothing.
    }

    [[nodiscard]]const int& foo() const
    {
        return _foo;
    }
private:
    int _foo;
};

static void BM_TypedAccessor(benchmark::State& state)
{
	const DataSource* source = new DataSource(1);
    auto* sink = new DataSink();
    auto* sut = new dag::TypedTransferMemFun<DataSource, DataSink, int>(source, sink, &DataSource::foo, &DataSink::setFoo);
    for (auto _ : state)
    {
        sut->makeItSo();
    }
    delete sut;
    delete source;
    delete sink;
}

BENCHMARK(BM_TypedAccessor);

static void BM_TypedTransferMemFun(benchmark::State& state) {
//    state.PauseTiming();
    auto* foo = new dag::Output(1);
    auto* bar = new dag::Input();
    auto* sut = new dag::TypedTransferMemFun<dag::Output, dag::Input, int>(foo, bar, &dag::Output::foo, &dag::Input::setFoo);
//    state.ResumeTiming();
    for (auto _ : state)
        sut->makeItSo();
//    state.PauseTiming();
    delete sut;
    delete foo;
    delete bar;
}
// Register the function as a benchmark
BENCHMARK(BM_TypedTransferMemFun);

        
// Define another benchmark
static void BM_StringCopy(benchmark::State& state) {
    std::string x = "hello";
    for (auto _ : state)
        std::string copy(x);
}
BENCHMARK(BM_StringCopy);

static void BM_AnyTransfer(benchmark::State& state)
{
    auto* foo = new dag::Output(1);
    auto* bar = new dag::Input();
    dag::SignalPathDef path;
    path.source.node = dag::NodeID(1);
    path.source.port = dag::PortID(0);
    path.dest.node = dag::NodeID(2);
    path.dest.port = dag::PortID(0);
    auto* sut = new dag::AnyTransfer(path, *foo, *bar);
    for ( auto _ : state)
        sut->makeItSo();
    delete sut;
    delete foo;
    delete bar;
}

BENCHMARK(BM_AnyTransfer);

static void BM_AbstractAnyTransfer(benchmark::State& state)
{
    auto foo = new dag::FooAbstractAny(1);
    auto bar = new dag::FooAbstractAny(0);
    dag::SignalPathDef path;
    path.source.node = 1;
    path.source.port = 0;
    path.dest.node = 2;
    path.dest.port = 0;
    auto sut = new dag::AbstractAnyTransfer(path, *foo, *bar);
    for (auto _ : state)
        sut->makeItSo();
    delete sut;
    delete foo;
    delete bar;
}

BENCHMARK(BM_AbstractAnyTransfer);

static void BM_TypedSwitchTransfer(benchmark::State& state)
{
    auto foo = new dag::FooTypedSwitch(1);
    auto bar = new dag::FooTypedSwitch(0);
    dag::SignalPathDef path;
    path.source.node = 1;
    path.source.port = 0;
    path.dest.node = 2;
    path.dest.port = 0;
    auto sut = new dag::TypedSwitchTransfer(path, *foo, *bar);
    for (auto _ : state)
        sut->makeItSo();
    delete sut;
    delete foo;
    delete bar;
}

BENCHMARK(BM_TypedSwitchTransfer);

static void BM_TypedPointerTransfer(benchmark::State& state)
{
    auto node = new dag::Output(1);
    auto node2 = new dag::Input();
    auto sut = new dag::TypedPointerTransfer(&node->_foo, &node2->_foo);
    for (auto _ : state)
        sut->makeItSo();
    delete sut;
    delete node2;
    delete node;
}

BENCHMARK(BM_TypedPointerTransfer);

static void BM_VariantTransfer(benchmark::State& state)
{
    auto* foo = new dag::Baz(1);
    auto* bar = new dag::Qux();
    dag::SignalPathDef path;
    path.source.node = 1;
    path.source.port = 0;
    path.dest.node = 2;
    path.dest.port = 0;
    auto* sut = new dag::VariantTransfer(&path, foo, bar);
    for (auto _ : state)
        sut->makeItSo();
    delete sut;
    delete foo;
    delete bar;
}

BENCHMARK(BM_VariantTransfer);

typedef dag::Value VariantValue;

static void BM_VariantSetPrimitive(benchmark::State& state)
{
    VariantValue v;
    
    for (auto _ : state)
    {
        v = std::int64_t(0);
        v = 0.0;
        v = true;
        //v = std::string("foo");
        //v = std::vector<VariantValue>{std::int64_t(1),std::int64_t(2),std::int64_t(3)};
    }
}

BENCHMARK(BM_VariantSetPrimitive);

static void BM_VariantSetString(benchmark::State& state)
{
    VariantValue v;
    
    for (auto _ : state)
    {
        v = std::string("foo");
    }
}

BENCHMARK(BM_VariantSetString);

static void BM_VariantSetVector(benchmark::State& state)
{
    VariantValue v;

    for (auto _ : state)
    {
        v = std::vector<VariantValue>{dag::Value(std::int64_t(1)),dag::Value(std::int64_t(2)),dag::Value(std::int64_t(3))};
    }
}

BENCHMARK(BM_VariantSetVector);

struct Value
{
    union UValue
    {
        std::int64_t i;
        double d;
        bool b;
        std::string s;

        UValue()
        {
            i = 0;
        }

        ~UValue() noexcept
        {
        }
    };
    Value()
    = default;

    ~Value() noexcept
    = default;
    UValue value;
};

static void BM_UnionSet(benchmark::State& state)
{
    for (auto _ : state)
    {
        Value v;
        
        v.value.i = 0;
        v.value.d = 0.0;
        v.value.b = true;
        new (&v.value.s) std::string("foo");
        v.value.s.~basic_string();
    }
}

BENCHMARK(BM_UnionSet);

static void BM_VariantModifyDouble(benchmark::State& state)
{
    VariantValue v{ 1.0 };
    
    for (auto _ : state)
    {
        v += 1.0;
    }    
}

BENCHMARK(BM_VariantModifyDouble);

static void BM_VariantModifyInt(benchmark::State& state)
{
    VariantValue v{ std::int64_t(1) };
    
    for (auto _ : state)
    {
        v.operator+=<std::int64_t>(1);
    }    
}

BENCHMARK(BM_VariantModifyInt);

static void BM_VariantModifyString(benchmark::State& state)
{
    for (auto _ : state)
    {
        VariantValue v{ std::string("Hello") };
        
        v += std::string(" World");
    }
}

BENCHMARK(BM_VariantModifyString);

static void BM_UnionModify(benchmark::State& state)
{
    Value v;
    v.value.d = 1.0;
    
    for ( auto _ : state )
    {
        v.value.d += 1.0;
    }
}

BENCHMARK(BM_UnionModify);

static void BM_UnionValueModifyInt(benchmark::State& state)
{        
    for ( auto _ : state )
    {
        dag::UnionValue v{1};
        ++v;
    }
}

BENCHMARK(BM_UnionValueModifyInt);

static void BM_UnionValueModifyIntByGet(benchmark::State& state)
{
    for ( auto _ : state )
    {
        dag::UnionValue v{1};
        
        ++dag::get<std::int32_t>(v);
    }
}

BENCHMARK(BM_UnionValueModifyIntByGet);

static void BM_UnionValueModifyString(benchmark::State& state)
{
    for ( auto _ : state )
    {
        dag::UnionValue v{new std::string("Hello")};
        
        dag::get<std::string>(v) += " World";
    }
}

BENCHMARK(BM_UnionValueModifyString);

static void BM_ModifyString(benchmark::State& state)
{
    for ( auto _ : state )
    {
        std::string s = "Hello";
        
        s+= " World";
    }
}

BENCHMARK(BM_ModifyString);

static void BM_ModifyArraySameType(benchmark::State& state)
{
    for ( auto _ : state )
    {
        VariantValue v{ std::int64_t(1) };
        
        v.push_back(std::int64_t(2));
    }
}

BENCHMARK(BM_ModifyArraySameType);

static void BM_ModifyArrayChangeTypeFromString(benchmark::State& state)
{
    for ( auto _ : state )
    {
        VariantValue v{ "Hello" };
        
        v.push_back(VariantValue{ std::int64_t(2) });
    }
}

BENCHMARK(BM_ModifyArrayChangeTypeFromString);

static void BM_ModifyArrayChangeTypeFromNumeric(benchmark::State& state)
{
    for ( auto _ : state )
    {
        VariantValue v{ std::int64_t(1) };
        
        v.push_back(VariantValue{ double(2) });
    }
}

BENCHMARK(BM_ModifyArrayChangeTypeFromNumeric);

static void BM_GetTypedPortValueStatic(benchmark::State& state)
{
    dag::MemoryNodeLibrary nodeLib;
    dag::TypedPort<double> port(nodeLib.nextPortID(), nullptr, new dag::MetaPort("port1", dag::PortType::TYPE_DOUBLE, dag::PortDirection::DIR_IN), 1.0);

    for (auto _ : state)
    {
        double value = port.value();

        value += 1.0;

        port.setValue(value);
    }
}

BENCHMARK(BM_GetTypedPortValueStatic);

static void BM_GetPortValueStatic(benchmark::State& state)
{
    dag::MemoryNodeLibrary nodeLib;
    auto* port = new dag::ValuePort(nodeLib.nextPortID(), new dag::MetaPort("port1", dag::PortType::TYPE_DOUBLE, dag::PortDirection::DIR_IN), dag::Value(1.0));

    for (auto _ : state)
    {
        dag::Value value = port->value();

        value += 1.0;

        port->setValue(value);
    }
}

BENCHMARK(BM_GetPortValueStatic);

class DoublePort
{
public:
    DoublePort(double value)
    {
        _value = value;
    }

    void setValue(double value)
    {
        _value = value;
    }

    double value() const
    {
        return _value;
    }
private:
    double _value;
};

static void BM_SetValueDoublePort(benchmark::State& state)
{
    DoublePort * port = new DoublePort(1.0);

    for (auto _ : state)
    {
        double value = port->value();

        value += 1.0;

        port->setValue(value);
    }
}

BENCHMARK(BM_SetValueDoublePort);

static void BM_ValuePortTransfer(benchmark::State& state)
{
    dag::MemoryNodeLibrary nodeLib;
    auto g = new dag::Graph();
    g->setNodeLibrary(&nodeLib);
    auto foo = dynamic_cast<dag::Foo*>(g->createNode("Foo", "foo1"));
    auto bar = dynamic_cast<dag::Bar*>(g->createNode("Bar", "bar1"));
    dag::PortTransfer transfer(dynamic_cast<dag::ValuePort*>(bar->dynamicPort(0)), dynamic_cast<dag::ValuePort*>(foo->dynamicPort(0)));

    for (auto _ : state)
    {

        transfer.makeItSo();
    }
    delete g;
}

BENCHMARK(BM_ValuePortTransfer);

class InputNode
{
public:
    InputNode(dag::NodeLibrary& nodeLib)
        :
        in1(nodeLib.nextPortID(), nullptr, new dag::MetaPort("in1", dag::PortType::TYPE_DOUBLE, dag::PortDirection::DIR_IN), 1.0)
    {
        // Do nothing.
    }
    dag::TypedPort<double> in1;
};

class OutputNode
{
public:
    OutputNode(dag::NodeLibrary& nodeLib)
        :
        out1(nodeLib.nextPortID(), nullptr, new dag::MetaPort("_out1", dag::PortType::TYPE_DOUBLE, dag::PortDirection::DIR_OUT), 1.0)
    {
        // Do nothing.
    }

    dag::TypedPort<double> out1;
};

static void BM_TypedPortTransfer(benchmark::State& state)
{
    dag::MemoryNodeLibrary nodeLib;
    InputNode foo(nodeLib);
    OutputNode bar(nodeLib);
    dag::Transfer* transfer = bar.out1.connectTo(foo.in1);

    for (auto _ : state)
    {
        transfer->makeItSo();
    }
    
    delete transfer;
}

BENCHMARK(BM_TypedPortTransfer);

class SpooVariant : public dag::Node
{
public:
    SpooVariant(dag::KeyGenerator& nodeLib, const std::string& name)
        :
        Node(nodeLib, name, dag::NodeCategory::CAT_SOURCE),
        _foo(nodeLib.nextPortID(), new dag::MetaPort("foo1", dag::PortType::TYPE_DOUBLE, dag::PortDirection::DIR_OUT), 1.0),
        _bar(nodeLib.nextPortID(), new dag::MetaPort("bar1", dag::PortType::TYPE_DOUBLE, dag::PortDirection::DIR_IN), 0.0)
    {
        // Do nothing.
    }

    explicit SpooVariant(dagbase::InputStream& str, dag::NodeLibrary& nodeLib)
    :
    Node(str, nodeLib),
    _foo(str, nodeLib),
    _bar(str, nodeLib)
    {
        // Do nothing.
    }

    explicit SpooVariant(const SpooVariant& other, dag::CloningFacility& facility, dag::CopyOp copyOp, dag::KeyGenerator* keyGen);

    void setFoo(const dag::VariantPort::ValueType& value)
    {
        _foo.setValue(value);
    }

    double foo() const
    {
        return std::get<double>(_foo.value());
    }

    dag::VariantPort* foo()
    {
        return &_foo;
    }
    
    void setBar(double value)
    {
        _foo.setValue(value);
    }

    dag::VariantPort* bar()
    {
        return &_bar;
    }
    
    [[nodiscard]]const char* className() const override
    {
        return "_p_SpooVariant";
    }

    SpooVariant* clone(dag::CloningFacility& facility, dag::CopyOp copyOp, dag::KeyGenerator* keyGen) override
    {
        return new SpooVariant(*this, facility, copyOp, keyGen);
    }

    SpooVariant* create(dagbase::InputStream& str, dag::NodeLibrary& nodeLib) override
    {
        return new SpooVariant(str, nodeLib);
    }

    bool equals(const dag::Node& other) const override
    {
        if (!Node::operator==(other))
        {
            return false;
        }
        const auto& spoo = dynamic_cast<const SpooVariant&>(other);

        if (!(_foo == spoo._foo))
        {
            return false;
        }
        if (!(_bar == spoo._bar))
        {
            return false;
        }
        return true;
    }
    dagbase::OutputStream& write(dagbase::OutputStream& str) const override
    {
        Node::write(str);
        _foo.write(str);
        _bar.write(str);

        return str;
    }

    void describe(dag::NodeDescriptor& descriptor) const override
    {
        Node::describe(descriptor);
        descriptor.ports.emplace_back(_foo.id(), _foo.name(), _foo.type(), _foo.dir());
        descriptor.ports.emplace_back(_bar.id(), _bar.name(), _bar.type(), _bar.dir());
    }

    dag::Port* dynamicPort(size_t index) override
    {
	    if (index == 0)
	    {
            return &_foo;
	    }
        else if (index == 1)
        {
            return &_bar;
        }

        return nullptr;
    }
    
    static dag::MetaPort const* metaPort(size_t index)
    {
        if (index < firstPort + numPorts)
        {
            return &ports[index-firstPort];
        }

        return nullptr;
    }
    
    const dag::MetaPort * dynamicMetaPort(size_t index) const override
    {
        return metaPort(index);
    }
protected:
    static const std::array<dag::MetaPort, 2> ports;
    static constexpr size_t firstPort = 0;
    static constexpr size_t numPorts = 2;
private:
    dag::VariantPort _foo;
    dag::VariantPort _bar;
};

const std::array<dag::MetaPort, 2> SpooVariant::ports =
{
    dag::MetaPort("foo1", dag::PortType::TYPE_DOUBLE, dag::PortDirection::DIR_OUT),
    dag::MetaPort("bar1", dag::PortType::TYPE_DOUBLE, dag::PortDirection::DIR_IN)
};

SpooVariant::SpooVariant(const SpooVariant &other, dag::CloningFacility& facility, dag::CopyOp copyOp, dag::KeyGenerator *keyGen)
:
Node(other,facility,copyOp,keyGen),
_foo(other._foo, facility, copyOp, keyGen),
_bar(other._bar, facility, copyOp, keyGen)
{
    // Do nothing.
}

static void BM_SetVariantPort(benchmark::State& state)
{
    dag::MemoryNodeLibrary nodeLib;
    dag::VariantPort port(nodeLib.nextPortID(), new dag::MetaPort("_out1", dag::PortType::TYPE_DOUBLE, dag::PortDirection::DIR_OUT), 1.0);

    for (auto _ : state)
    {
        port.setValue(std::get<double>(port.value()) + 1.0);
    }
}

BENCHMARK(BM_SetVariantPort);

static void BM_VariantPortTransfer(benchmark::State& state)
{
    dag::MemoryNodeLibrary nodeLib;
    SpooVariant * source = new SpooVariant(nodeLib, "source");
    SpooVariant * dest = new SpooVariant(nodeLib, "dest");
    source->foo()->setValue(1.0);
    dag::Transfer * sut = new dag::VariantPortTransfer(source->foo(), dest->bar());
    
    for (auto _ : state)
    {
        sut->makeItSo();
    }
    
    delete sut;
    delete dest;
    delete source;
}

BENCHMARK(BM_VariantPortTransfer);

static void BM_VariantNodeSetPortValue(benchmark::State& state)
{
    dag::MemoryNodeLibrary nodeLib;
    SpooVariant foo(nodeLib, "foo1");

    for ( auto _: state)
    {
        double value = std::get<double>(foo.foo()->value());

        value += 1.0;
        foo.setBar(value);
    }
}

BENCHMARK(BM_VariantNodeSetPortValue);

class FooSource
{
public:
    explicit FooSource(int foo)
    :
    _foo(foo)
    {
        // Do nothing.
    }
    
    [[nodiscard]]int foo() const
    {
        return _foo;
    }
private:
    int _foo;
};

class FooSink
{
public:
    FooSink() = default;
    
    void setFoo(int foo)
    {
        _foo = foo;
    }
private:
    int _foo;
};

TRANSFER_DEFINE(FooTransfer, FooSource, FooSink, setFoo, foo);

static void BM_PropertyTransfer(benchmark::State& state)
{
    auto source = new FooSource(1);
    auto sink = new FooSink();
    dag::Transfer* transfer = new FooTransfer(*source,*sink);
    
    for (auto _ : state)
    {
        transfer->makeItSo();
    }
    
    delete transfer;
    delete sink;
    delete source;
}

BENCHMARK(BM_PropertyTransfer);

static void BM_NodeDescribe(benchmark::State& state)
{
    dag::MemoryNodeLibrary nodeLib;
    dag::FooTyped const* foo = new dag::FooTyped(nodeLib, "foo1", dag::NodeCategory::CAT_SINK);

    for (auto _ : state)
    {
        dag::NodeDescriptor d;

        foo->describe(d);
    }
    delete foo;
}

BENCHMARK(BM_NodeDescribe);

static void BM_VirtualPortArray(benchmark::State& state)
{
    dag::MemoryNodeLibrary nodeLib;
    auto g = new dag::Graph();
    g->setNodeLibrary(&nodeLib);
    auto sut = dynamic_cast<dag::Final*>(g->createNode("Final", "final1"));
    sut->addDynamicPort(new dag::TypedPort<double>(nodeLib.nextPortID(), nullptr, new dag::MetaPort("test1", dag::PortType::TYPE_DOUBLE, dag::PortDirection::DIR_OUT), 1.0));

    for (auto _ : state)
    {
        auto port = static_cast<dag::TypedPort<double>*>(sut->dynamicPort(0));

        port->setValue(2.0);
    }
    delete g;
}

BENCHMARK(BM_VirtualPortArray);

static void BM_VirtualMetaPortArray(benchmark::State& state)
{
    dag::MemoryNodeLibrary nodeLib;
    auto g = new dag::Graph();
    g->setNodeLibrary(&nodeLib);
    auto sut = dynamic_cast<dag::Final*>(g->createNode("Final", "final1"));
    sut->addDynamicPort(new dag::TypedPort<double>(nodeLib.nextPortID(), nullptr, new dag::MetaPort("test1", dag::PortType::TYPE_DOUBLE, dag::PortDirection::DIR_OUT), 1.0));
    std::int64_t i = 0;

    for (auto _ : state)
    {
        auto descriptor = sut->dynamicMetaPort(3);

        i += descriptor->type;
    }
    delete g;
}

BENCHMARK(BM_VirtualMetaPortArray);

static void BM_SelectionLiveAdd(benchmark::State& state)
{
    dag::MemoryNodeLibrary nodeLib;
    auto g = new dag::Graph();
    g->setNodeLibrary(&nodeLib);
    auto node1 = dynamic_cast<dag::FooTyped*>(g->createNode("FooTyped", "foo1"));
    auto node2 = dynamic_cast<dag::BarTyped*>(g->createNode("BarTyped", "bar1"));
    dag::SelectionInterface::Cont a;
    a.insert(node1);
    a.insert(node2);
    for (auto _ : state)
    {
        dag::SelectionLive sut;
        sut.add(a.begin(), a.end());
    }

    delete node2;
    delete node1;
    delete g;
}

BENCHMARK(BM_SelectionLiveAdd);

static void BM_SelectionLiveSubtract(benchmark::State& state)
{
    dag::MemoryNodeLibrary nodeLib;
    auto g = new dag::Graph();
    g->setNodeLibrary(&nodeLib);
    auto node1 = dynamic_cast<dag::FooTyped*>(g->createNode("FooTyped", "foo1"));
    auto node2 = dynamic_cast<dag::BarTyped*>(g->createNode("BarTyped", "bar1"));
    dag::SelectionInterface::Cont a;
    a.insert(node1);
    a.insert(node2);
    auto it = a.begin();
    std::advance(it, 1);
    for (auto _ : state)
    {
        dag::SelectionLive sut;
        sut.subtract(a.begin(), it);
    }

    delete node2;
    delete node1;
    delete g;
}

BENCHMARK(BM_SelectionLiveSubtract);

static void BM_SelectionLiveSet(benchmark::State& state)
{
    dag::MemoryNodeLibrary nodeLib;
    auto g = new dag::Graph();
    g->setNodeLibrary(&nodeLib);
    auto node1 = dynamic_cast<dag::FooTyped*>(g->createNode("FooTyped", "foo1"));
    auto node2 = dynamic_cast<dag::BarTyped*>(g->createNode("BarTyped", "bar1"));
    dag::SelectionInterface::Cont a;
    a.insert(node1);
    a.insert(node2);
    for (auto _ : state)
    {
        dag::SelectionLive sut;
        sut.set(a.begin(), a.end());
    }

    delete node2;
    delete node1;
    delete g;
}

BENCHMARK(BM_SelectionLiveSet);

static void BM_SelectionLiveToggle(benchmark::State& state)
{
    dag::MemoryNodeLibrary nodeLib;
    auto g = new dag::Graph();
    auto node1 = dynamic_cast<dag::FooTyped*>(g->createNode("FooTyped", "foo1"));
    auto node2 = dynamic_cast<dag::BarTyped*>(g->createNode("BarTyped", "bar1"));
    dag::SelectionInterface::Cont a;
    a.insert(node1);
    a.insert(node2);
    auto it = a.begin();
    std::advance(it, 1);
    dag::SelectionLive sut;
    for (auto _ : state)
    {
        sut.toggle(a.begin(), it);
    }

    delete node2;
    delete node1;
    delete g;
}

BENCHMARK(BM_SelectionLiveToggle);

static void BM_DynamicCastPort(benchmark::State& state)
{
    dag::Port* p = new dag::TypedPort<double>(0, "port1", dag::PortType::TYPE_DOUBLE, dag::PortDirection::DIR_INTERNAL, 0.0, nullptr);
    for (auto _ : state)
    {
        auto typed = dynamic_cast<dag::TypedPort<double>*>(p);
        // Avoid optimising out the cast.
        typed->setValue(1.0);
    }
}

BENCHMARK(BM_DynamicCastPort);

static void BM_StaticCastPort(benchmark::State& state)
{
    dag::Port* p = new dag::TypedPort<double>(0, "port1", dag::PortType::TYPE_DOUBLE, dag::PortDirection::DIR_INTERNAL, 0.0, nullptr);
    for (auto _ : state)
    {
        auto typed = static_cast<dag::TypedPort<double>*>(p);
        typed->setValue(1.0);
    }
}

BENCHMARK(BM_StaticCastPort);

BENCHMARK_MAIN();
