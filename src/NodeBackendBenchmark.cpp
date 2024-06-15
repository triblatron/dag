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
    auto* sut = new nbe::TypedTransferMemFun<DataSource, DataSink, int>(source, sink, &DataSource::foo, &DataSink::setFoo);
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
    auto* foo = new nbe::Output(1);
    auto* bar = new nbe::Input();
    auto* sut = new nbe::TypedTransferMemFun<nbe::Output, nbe::Input, int>(foo, bar, &nbe::Output::foo, &nbe::Input::setFoo);
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
    auto* foo = new nbe::Output(1);
    auto* bar = new nbe::Input();
    nbe::SignalPathDef path;
    path.source.node = nbe::NodeID(1);
    path.source.port = nbe::PortID(0);
    path.dest.node = nbe::NodeID(2);
    path.dest.port = nbe::PortID(0);
    auto* sut = new nbe::AnyTransfer(path, *foo, *bar);
    for ( auto _ : state)
        sut->makeItSo();
    delete sut;
    delete foo;
    delete bar;
}

BENCHMARK(BM_AnyTransfer);

static void BM_AbstractAnyTransfer(benchmark::State& state)
{
    auto foo = new nbe::FooAbstractAny(1);
    auto bar = new nbe::FooAbstractAny(0);
    nbe::SignalPathDef path;
    path.source.node = 1;
    path.source.port = 0;
    path.dest.node = 2;
    path.dest.port = 0;
    auto sut = new nbe::AbstractAnyTransfer(path, *foo, *bar);
    for (auto _ : state)
        sut->makeItSo();
    delete sut;
    delete foo;
    delete bar;
}

BENCHMARK(BM_AbstractAnyTransfer);

static void BM_TypedSwitchTransfer(benchmark::State& state)
{
    auto foo = new nbe::FooTypedSwitch(1);
    auto bar = new nbe::FooTypedSwitch(0);
    nbe::SignalPathDef path;
    path.source.node = 1;
    path.source.port = 0;
    path.dest.node = 2;
    path.dest.port = 0;
    auto sut = new nbe::TypedSwitchTransfer(path, *foo, *bar);
    for (auto _ : state)
        sut->makeItSo();
    delete sut;
    delete foo;
    delete bar;
}

BENCHMARK(BM_TypedSwitchTransfer);

static void BM_TypedPointerTransfer(benchmark::State& state)
{
    auto node = new nbe::Output(1);
    auto node2 = new nbe::Input();
    auto sut = new nbe::TypedPointerTransfer(&node->_foo, &node2->_foo);
    for (auto _ : state)
        sut->makeItSo();
    delete sut;
    delete node2;
    delete node;
}

BENCHMARK(BM_TypedPointerTransfer);

static void BM_VariantTransfer(benchmark::State& state)
{
    auto* foo = new nbe::Baz(1);
    auto* bar = new nbe::Qux();
    nbe::SignalPathDef path;
    path.source.node = 1;
    path.source.port = 0;
    path.dest.node = 2;
    path.dest.port = 0;
    auto* sut = new nbe::VariantTransfer(&path, foo, bar);
    for (auto _ : state)
        sut->makeItSo();
    delete sut;
    delete foo;
    delete bar;
}

BENCHMARK(BM_VariantTransfer);

typedef nbe::Value VariantValue;

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
        v = std::vector<VariantValue>{nbe::Value(std::int64_t(1)),nbe::Value(std::int64_t(2)),nbe::Value(std::int64_t(3))};
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
        nbe::UnionValue v{1};
        ++v;
    }
}

BENCHMARK(BM_UnionValueModifyInt);

static void BM_UnionValueModifyIntByGet(benchmark::State& state)
{
    for ( auto _ : state )
    {
        nbe::UnionValue v{1};
        
        ++nbe::get<std::int32_t>(v);
    }
}

BENCHMARK(BM_UnionValueModifyIntByGet);

static void BM_UnionValueModifyString(benchmark::State& state)
{
    for ( auto _ : state )
    {
        nbe::UnionValue v{new std::string("Hello")};
        
        nbe::get<std::string>(v) += " World";
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
    nbe::MemoryNodeLibrary nodeLib;
    nbe::TypedPort<double> port(nodeLib.nextPortID(), nullptr, new nbe::MetaPort("port1", nbe::PortType::TYPE_DOUBLE, nbe::PortDirection::DIR_IN), 1.0);

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
    nbe::MemoryNodeLibrary nodeLib;
    auto* port = new nbe::ValuePort(nodeLib.nextPortID(), new nbe::MetaPort("port1", nbe::PortType::TYPE_DOUBLE, nbe::PortDirection::DIR_IN), nbe::Value(1.0));

    for (auto _ : state)
    {
        nbe::Value value = port->value();

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
    nbe::MemoryNodeLibrary nodeLib;
    auto g = new nbe::Graph();
    g->setNodeLibrary(&nodeLib);
    auto foo = dynamic_cast<nbe::Foo*>(g->createNode("Foo", "foo1"));
    auto bar = dynamic_cast<nbe::Bar*>(g->createNode("Bar", "bar1"));
    nbe::PortTransfer transfer(dynamic_cast<nbe::ValuePort*>(bar->dynamicPort(0)), dynamic_cast<nbe::ValuePort*>(foo->dynamicPort(0)));

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
    InputNode(nbe::NodeLibrary& nodeLib)
        :
        in1(nodeLib.nextPortID(), nullptr, new nbe::MetaPort("in1", nbe::PortType::TYPE_DOUBLE, nbe::PortDirection::DIR_IN), 1.0)
    {
        // Do nothing.
    }
    nbe::TypedPort<double> in1;
};

class OutputNode
{
public:
    OutputNode(nbe::NodeLibrary& nodeLib)
        :
        out1(nodeLib.nextPortID(), nullptr, new nbe::MetaPort("_out1", nbe::PortType::TYPE_DOUBLE, nbe::PortDirection::DIR_OUT), 1.0)
    {
        // Do nothing.
    }

    nbe::TypedPort<double> out1;
};

static void BM_TypedPortTransfer(benchmark::State& state)
{
    nbe::MemoryNodeLibrary nodeLib;
    InputNode foo(nodeLib);
    OutputNode bar(nodeLib);
    nbe::Transfer* transfer = bar.out1.connectTo(foo.in1);

    for (auto _ : state)
    {
        transfer->makeItSo();
    }
    
    delete transfer;
}

BENCHMARK(BM_TypedPortTransfer);

class SpooVariant : public nbe::Node
{
public:
    SpooVariant(nbe::KeyGenerator& nodeLib, const std::string& name)
        :
        Node(nodeLib, name, nbe::NodeCategory::CAT_SOURCE),
        _foo(nodeLib.nextPortID(), new nbe::MetaPort("foo1", nbe::PortType::TYPE_DOUBLE, nbe::PortDirection::DIR_OUT), 1.0),
        _bar(nodeLib.nextPortID(), new nbe::MetaPort("bar1", nbe::PortType::TYPE_DOUBLE, nbe::PortDirection::DIR_IN), 0.0)
    {
        // Do nothing.
    }

    explicit SpooVariant(nbe::InputStream& str, nbe::NodeLibrary& nodeLib)
    :
    Node(str, nodeLib),
    _foo(str, nodeLib),
    _bar(str, nodeLib)
    {
        // Do nothing.
    }

    explicit SpooVariant(const SpooVariant& other, nbe::CopyOp copyOp, nbe::KeyGenerator* keyGen);

    void setFoo(const nbe::VariantPort::ValueType& value)
    {
        _foo.setValue(value);
    }

    double foo() const
    {
        return std::get<double>(_foo.value());
    }

    nbe::VariantPort* foo()
    {
        return &_foo;
    }
    
    void setBar(double value)
    {
        _foo.setValue(value);
    }

    nbe::VariantPort* bar()
    {
        return &_bar;
    }
    
    [[nodiscard]]const char* className() const override
    {
        return "_p_SpooVariant";
    }

    SpooVariant* clone(nbe::CopyOp copyOp, nbe::KeyGenerator* keyGen) override
    {
        return new SpooVariant(*this, copyOp, keyGen);
    }

    SpooVariant* create(nbe::InputStream& str, nbe::NodeLibrary& nodeLib) override
    {
        return new SpooVariant(str, nodeLib);
    }

    bool equals(const nbe::Node& other) const override
    {
        if (!Node::operator==(other))
        {
            return false;
        }
        auto spoo = dynamic_cast<const SpooVariant&>(other);

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
    nbe::OutputStream& write(nbe::OutputStream& str) const override
    {
        Node::write(str);
        _foo.write(str);
        _bar.write(str);

        return str;
    }

    void describe(nbe::NodeDescriptor& descriptor) const override
    {
        Node::describe(descriptor);
        descriptor.ports.emplace_back(_foo.id(), _foo.name(), _foo.type(), _foo.dir());
        descriptor.ports.emplace_back(_bar.id(), _bar.name(), _bar.type(), _bar.dir());
    }

    nbe::Port* dynamicPort(size_t index) override
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
    
    static nbe::MetaPort const* metaPort(size_t index)
    {
        if (index < firstPort + numPorts)
        {
            return &ports[index-firstPort];
        }

        return nullptr;
    }
    
    const nbe::MetaPort * dynamicMetaPort(size_t index) const override
    {
        return metaPort(index);
    }
protected:
    static const std::array<nbe::MetaPort, 2> ports;
    static constexpr size_t firstPort = 0;
    static constexpr size_t numPorts = 2;
private:
    nbe::VariantPort _foo;
    nbe::VariantPort _bar;
};

const std::array<nbe::MetaPort, 2> SpooVariant::ports =
{
    nbe::MetaPort("foo1", nbe::PortType::TYPE_DOUBLE, nbe::PortDirection::DIR_OUT),
    nbe::MetaPort("bar1", nbe::PortType::TYPE_DOUBLE, nbe::PortDirection::DIR_IN)
};

SpooVariant::SpooVariant(const SpooVariant &other, nbe::CopyOp copyOp, nbe::KeyGenerator *keyGen)
:
Node(other,copyOp,keyGen),
_foo(other._foo),
_bar(other._bar)
{
    // Do nothing.
}

static void BM_SetVariantPort(benchmark::State& state)
{
    nbe::MemoryNodeLibrary nodeLib;
    nbe::VariantPort port(nodeLib.nextPortID(), new nbe::MetaPort("_out1", nbe::PortType::TYPE_DOUBLE, nbe::PortDirection::DIR_OUT), 1.0);

    for (auto _ : state)
    {
        port.setValue(std::get<double>(port.value()) + 1.0);
    }
}

BENCHMARK(BM_SetVariantPort);

static void BM_VariantPortTransfer(benchmark::State& state)
{
    nbe::MemoryNodeLibrary nodeLib;
    SpooVariant * source = new SpooVariant(nodeLib, "source");
    SpooVariant * dest = new SpooVariant(nodeLib, "dest");
    source->foo()->setValue(1.0);
    nbe::Transfer * sut = new nbe::VariantPortTransfer(source->foo(), dest->bar());
    
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
    nbe::MemoryNodeLibrary nodeLib;
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
    nbe::Transfer* transfer = new FooTransfer(*source,*sink);
    
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
    nbe::MemoryNodeLibrary nodeLib;
    nbe::FooTyped const* foo = new nbe::FooTyped(nodeLib, "foo1", nbe::NodeCategory::CAT_SINK);

    for (auto _ : state)
    {
        nbe::NodeDescriptor d;

        foo->describe(d);
    }
    delete foo;
}

BENCHMARK(BM_NodeDescribe);

static void BM_VirtualPortArray(benchmark::State& state)
{
    nbe::MemoryNodeLibrary nodeLib;
    auto g = new nbe::Graph();
    g->setNodeLibrary(&nodeLib);
    auto sut = dynamic_cast<nbe::Final*>(g->createNode("Final", "final1"));
    sut->addDynamicPort(new nbe::TypedPort<double>(nodeLib.nextPortID(), nullptr, new nbe::MetaPort("test1", nbe::PortType::TYPE_DOUBLE, nbe::PortDirection::DIR_OUT), 1.0));

    for (auto _ : state)
    {
        auto port = static_cast<nbe::TypedPort<double>*>(sut->dynamicPort(0));

        port->setValue(2.0);
    }
    delete g;
}

BENCHMARK(BM_VirtualPortArray);

static void BM_VirtualMetaPortArray(benchmark::State& state)
{
    nbe::MemoryNodeLibrary nodeLib;
    auto g = new nbe::Graph();
    g->setNodeLibrary(&nodeLib);
    auto sut = dynamic_cast<nbe::Final*>(g->createNode("Final", "final1"));
    sut->addDynamicPort(new nbe::TypedPort<double>(nodeLib.nextPortID(), nullptr, new nbe::MetaPort("test1", nbe::PortType::TYPE_DOUBLE, nbe::PortDirection::DIR_OUT), 1.0));
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
    nbe::MemoryNodeLibrary nodeLib;
    auto g = new nbe::Graph();
    g->setNodeLibrary(&nodeLib);
    auto node1 = dynamic_cast<nbe::FooTyped*>(g->createNode("FooTyped", "foo1"));
    auto node2 = dynamic_cast<nbe::BarTyped*>(g->createNode("BarTyped", "bar1"));
    nbe::SelectionInterface::Cont a;
    a.insert(node1);
    a.insert(node2);
    for (auto _ : state)
    {
        nbe::SelectionLive sut;
        sut.add(a.begin(), a.end());
    }

    delete node2;
    delete node1;
    delete g;
}

BENCHMARK(BM_SelectionLiveAdd);

static void BM_SelectionLiveSubtract(benchmark::State& state)
{
    nbe::MemoryNodeLibrary nodeLib;
    auto g = new nbe::Graph();
    g->setNodeLibrary(&nodeLib);
    auto node1 = dynamic_cast<nbe::FooTyped*>(g->createNode("FooTyped", "foo1"));
    auto node2 = dynamic_cast<nbe::BarTyped*>(g->createNode("BarTyped", "bar1"));
    nbe::SelectionInterface::Cont a;
    a.insert(node1);
    a.insert(node2);
    auto it = a.begin();
    std::advance(it, 1);
    for (auto _ : state)
    {
        nbe::SelectionLive sut;
        sut.subtract(a.begin(), it);
    }

    delete node2;
    delete node1;
    delete g;
}

BENCHMARK(BM_SelectionLiveSubtract);

static void BM_SelectionLiveSet(benchmark::State& state)
{
    nbe::MemoryNodeLibrary nodeLib;
    auto g = new nbe::Graph();
    g->setNodeLibrary(&nodeLib);
    auto node1 = dynamic_cast<nbe::FooTyped*>(g->createNode("FooTyped", "foo1"));
    auto node2 = dynamic_cast<nbe::BarTyped*>(g->createNode("BarTyped", "bar1"));
    nbe::SelectionInterface::Cont a;
    a.insert(node1);
    a.insert(node2);
    for (auto _ : state)
    {
        nbe::SelectionLive sut;
        sut.set(a.begin(), a.end());
    }

    delete node2;
    delete node1;
    delete g;
}

BENCHMARK(BM_SelectionLiveSet);

static void BM_SelectionLiveToggle(benchmark::State& state)
{
    nbe::MemoryNodeLibrary nodeLib;
    auto g = new nbe::Graph();
    auto node1 = dynamic_cast<nbe::FooTyped*>(g->createNode("FooTyped", "foo1"));
    auto node2 = dynamic_cast<nbe::BarTyped*>(g->createNode("BarTyped", "bar1"));
    nbe::SelectionInterface::Cont a;
    a.insert(node1);
    a.insert(node2);
    auto it = a.begin();
    std::advance(it, 1);
    nbe::SelectionLive sut;
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
    nbe::Port* p = new nbe::TypedPort<double>(0, "port1", nbe::PortType::TYPE_DOUBLE, nbe::PortDirection::DIR_INTERNAL, 0.0, nullptr);
    for (auto _ : state)
    {
        auto typed = dynamic_cast<nbe::TypedPort<double>*>(p);
        // Avoid optimising out the cast.
        typed->setValue(1.0);
    }
}

BENCHMARK(BM_DynamicCastPort);

static void BM_StaticCastPort(benchmark::State& state)
{
    nbe::Port* p = new nbe::TypedPort<double>(0, "port1", nbe::PortType::TYPE_DOUBLE, nbe::PortDirection::DIR_INTERNAL, 0.0, nullptr);
    for (auto _ : state)
    {
        auto typed = static_cast<nbe::TypedPort<double>*>(p);
        typed->setValue(1.0);
    }
}

BENCHMARK(BM_StaticCastPort);

BENCHMARK_MAIN();
