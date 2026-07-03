
#include "core/SignalPathDef.h"
#include "core/Value.h"
#include "core/Port.h"
#include "MemoryNodeLibrary.h"
#include "Nodes.h"
#include "core/MetaPort.h"

#include "core/TypedPort.h"
#include "SelectionLive.h"
#include "core/Graph.h"

#include <benchmark/benchmark.h>

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
    auto* sut = new dagbase::TypedTransferMemFun<DataSource, DataSink, int>(source, sink, &DataSource::foo, &DataSink::setFoo);
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
    auto* foo = new dagbase::Output(1);
    auto* bar = new dagbase::Input();
    auto* sut = new dagbase::TypedTransferMemFun<dagbase::Output, dagbase::Input, int>(foo, bar, &dagbase::Output::foo, &dagbase::Input::setFoo);
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
    auto* foo = new dagbase::Output(1);
    auto* bar = new dagbase::Input();
    dagbase::SignalPathDef path;
    path.source.node = dagbase::NodeID(1);
    path.source.port = dagbase::PortID(0);
    path.dest.node = dagbase::NodeID(2);
    path.dest.port = dagbase::PortID(0);
    auto* sut = new dagbase::AnyTransfer(path, *foo, *bar);
    for ( auto _ : state)
        sut->makeItSo();
    delete sut;
    delete foo;
    delete bar;
}

BENCHMARK(BM_AnyTransfer);

static void BM_AbstractAnyTransfer(benchmark::State& state)
{
    auto foo = new dagbase::FooAbstractAny(1);
    auto bar = new dagbase::FooAbstractAny(0);
    dagbase::SignalPathDef path;
    path.source.node = 1;
    path.source.port = 0;
    path.dest.node = 2;
    path.dest.port = 0;
    auto sut = new dagbase::AbstractAnyTransfer(path, *foo, *bar);
    for (auto _ : state)
        sut->makeItSo();
    delete sut;
    delete foo;
    delete bar;
}

BENCHMARK(BM_AbstractAnyTransfer);

static void BM_TypedSwitchTransfer(benchmark::State& state)
{
    auto foo = new dagbase::FooTypedSwitch(1);
    auto bar = new dagbase::FooTypedSwitch(0);
    dagbase::SignalPathDef path;
    path.source.node = 1;
    path.source.port = 0;
    path.dest.node = 2;
    path.dest.port = 0;
    auto sut = new dagbase::TypedSwitchTransfer(path, *foo, *bar);
    for (auto _ : state)
        sut->makeItSo();
    delete sut;
    delete foo;
    delete bar;
}

BENCHMARK(BM_TypedSwitchTransfer);

static void BM_TypedPointerTransfer(benchmark::State& state)
{
    auto node = new dagbase::Output(1);
    auto node2 = new dagbase::Input();
    auto sut = new dagbase::TypedPointerTransfer(&node->_foo, &node2->_foo);
    for (auto _ : state)
        sut->makeItSo();
    delete sut;
    delete node2;
    delete node;
}

BENCHMARK(BM_TypedPointerTransfer);

static void BM_VariantTransfer(benchmark::State& state)
{
    auto* foo = new dagbase::Baz(1);
    auto* bar = new dagbase::Qux();
    dagbase::SignalPathDef path;
    path.source.node = 1;
    path.source.port = 0;
    path.dest.node = 2;
    path.dest.port = 0;
    auto* sut = new dagbase::VariantTransfer(&path, foo, bar);
    for (auto _ : state)
        sut->makeItSo();
    delete sut;
    delete foo;
    delete bar;
}

BENCHMARK(BM_VariantTransfer);

typedef dagbase::Value VariantValue;

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
        v = std::vector<VariantValue>{dagbase::Value(std::int64_t(1)),dagbase::Value(std::int64_t(2)),dagbase::Value(std::int64_t(3))};
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
    dagbase::TypedPort<double> port(nodeLib.nextPortID(), nullptr, "port1", dagbase::PortType::TYPE_DOUBLE, dagbase::PortDirection::DIR_IN, 1.0);

    for (auto _ : state)
    {
        double value = port.value();

        value += 1.0;

        port.setValue(value);
    }
}

BENCHMARK(BM_GetTypedPortValueStatic);

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

class InputNode
{
public:
    InputNode(dagbase::NodeLibrary& nodeLib)
        :
        in1(nodeLib.nextPortID(), nullptr, "in1", dagbase::PortType::TYPE_DOUBLE, dagbase::PortDirection::DIR_IN, 1.0)
    {
        // Do nothing.
    }
    dagbase::TypedPort<double> in1;
};

class OutputNode
{
public:
    OutputNode(dagbase::NodeLibrary& nodeLib)
        :
        out1(nodeLib.nextPortID(), nullptr, "_out1", dagbase::PortType::TYPE_DOUBLE, dagbase::PortDirection::DIR_OUT, 1.0)
    {
        // Do nothing.
    }

    dagbase::TypedPort<double> out1;
};

static void BM_TypedPortTransfer(benchmark::State& state)
{
    dag::MemoryNodeLibrary nodeLib;
    InputNode foo(nodeLib);
    OutputNode bar(nodeLib);
    dagbase::Transfer* transfer = bar.out1.connectTo(foo.in1);

    for (auto _ : state)
    {
        transfer->makeItSo();
    }
    
    delete transfer;
}

BENCHMARK(BM_TypedPortTransfer);

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
    dagbase::Transfer* transfer = new FooTransfer(*source,*sink);
    
    for (auto _ : state)
    {
        transfer->makeItSo();
    }
    
    delete transfer;
    delete sink;
    delete source;
}

BENCHMARK(BM_PropertyTransfer);

static void BM_VirtualPortArray(benchmark::State& state)
{
    dag::MemoryNodeLibrary nodeLib;
    auto g = new dagbase::Graph();
    g->setNodeLibrary(&nodeLib);
    auto sut = dynamic_cast<dag::Final*>(g->createNode("Final", "final1"));
    sut->addDynamicPort(new dagbase::TypedPort<double>(nodeLib.nextPortID(), nullptr, "test1", dagbase::PortType::TYPE_DOUBLE, dagbase::PortDirection::DIR_OUT, 1.0));

    for (auto _ : state)
    {
        auto port = static_cast<dagbase::TypedPort<double>*>(sut->dynamicPort(0));

        port->setValue(2.0);
    }
    delete g;
}

BENCHMARK(BM_VirtualPortArray);

static void BM_VirtualMetaPortArray(benchmark::State& state)
{
    dag::MemoryNodeLibrary nodeLib;
    auto g = new dagbase::Graph();
    g->setNodeLibrary(&nodeLib);
    auto sut = dynamic_cast<dag::Final*>(g->createNode("Final", "final1"));
    sut->addDynamicPort(new dagbase::TypedPort<double>(nodeLib.nextPortID(), nullptr, "test1", dagbase::PortType::TYPE_DOUBLE, dagbase::PortDirection::DIR_OUT, 1.0));
    std::int64_t i = 0;

    for (auto _ : state)
    {
        auto descriptor = sut->dynamicMetaPort(3);

        i += descriptor->isOwned();
    }
    delete g;
}

BENCHMARK(BM_VirtualMetaPortArray);

static void BM_SelectionLiveAdd(benchmark::State& state)
{
    dag::MemoryNodeLibrary nodeLib;
    auto g = new dagbase::Graph();
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
    auto g = new dagbase::Graph();
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
    auto g = new dagbase::Graph();
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
    auto g = new dagbase::Graph();
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
    dagbase::Port* p = new dagbase::TypedPort<double>(0, "port1", dagbase::PortType::TYPE_DOUBLE, dagbase::PortDirection::DIR_INTERNAL, 0.0, nullptr);
    for (auto _ : state)
    {
        auto typed = dynamic_cast<dagbase::TypedPort<double>*>(p);
        // Avoid optimising out the cast.
        typed->setValue(1.0);
    }
}

BENCHMARK(BM_DynamicCastPort);

static void BM_StaticCastPort(benchmark::State& state)
{
    dagbase::Port* p = new dagbase::TypedPort<double>(0, "port1", dagbase::PortType::TYPE_DOUBLE, dagbase::PortDirection::DIR_INTERNAL, 0.0, nullptr);
    for (auto _ : state)
    {
        auto typed = static_cast<dagbase::TypedPort<double>*>(p);
        typed->setValue(1.0);
    }
}

BENCHMARK(BM_StaticCastPort);

BENCHMARK_MAIN();
