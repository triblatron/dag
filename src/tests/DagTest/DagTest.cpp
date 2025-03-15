#include "config/config.h"

#include "gtest/gtest.h"
#include "core/LuaInterface.h"
#include "MetaOperation.h"
#include "MetaCoroutine.h"
#include "Node.h"
#include "Port.h"
#include "TypedPort.h"
#include "MemoryNodeLibrary.h"
#include "Nodes.h"
#include "NodeCategory.h"
#include "NodeDescriptor.h"
#include "Graph.h"
#include "SelectionLive.h"
#include "NodeEditorLive.h"
#include "Boundary.h"
#include "SignalPath.h"
#include "CreateNode.h"
#include "core/ByteBuffer.h"
#include "io/MemoryOutputStream.h"
#include "io/MemoryInputStream.h"
#include "FileSystemTraverser.h"
#include "NodePluginScanner.h"
#include "CloningFacility.h"
#include "io/MemoryBackingStore.h"
#include "io/FormatAgnosticOutputStream.h"
#include "io/FormatAgnosticInputStream.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <filesystem>

#include "core/Class.h"
#include "core/MetaClass.h"
#include "io/BinaryFormat.h"
#include "io/TextFormat.h"

class MemoryNodeLibraryTest : public ::testing::TestWithParam<std::tuple<const char*, const char*, size_t, const char*, dag::PortDirection::Direction, double>>
{
};

TEST_P(MemoryNodeLibraryTest, checkInstantiate)
{
    dag::MemoryNodeLibrary sut;
    const char* className = std::get<0>(GetParam());
    const char* name = std::get<1>(GetParam());
    size_t portIndex = std::get<2>(GetParam());
    const char* portName = std::get<3>(GetParam());
    dag::PortDirection::Direction portDir = std::get<4>(GetParam());
    double value = std::get<5>(GetParam());
    dag::Node* actualNode = sut.instantiateNode(0, className, name);
    ASSERT_NE(nullptr, actualNode);
    EXPECT_STREQ(name, actualNode->name().c_str());
    EXPECT_GT(actualNode->totalPorts(), portIndex);
    auto actualPort = dynamic_cast<dag::ValuePort*>(actualNode->dynamicPort(portIndex));
    ASSERT_NE(nullptr, actualPort);
    EXPECT_STREQ(portName, actualPort->name().c_str());
    EXPECT_EQ(portDir, actualPort->dir());
    EXPECT_EQ(value, double(actualPort->value()));
    
    delete actualNode;
}

INSTANTIATE_TEST_SUITE_P(MemoryNodeLibraryInstantiateTest, MemoryNodeLibraryTest, ::testing::Values(
    std::make_tuple("Foo", "foo1", 0, "in1", dag::PortDirection::DIR_IN, 1.0),
    std::make_tuple("Bar", "bar1", 0, "out1", dag::PortDirection::DIR_OUT, 1.0)
));

TEST(MemoryNodeLibraryTest_testClassNotFound, checkClassNotFound)
{
    dag::MemoryNodeLibrary sut;
    ASSERT_THROW(sut.instantiateNode(0,"NotFound", "notFound1"), std::runtime_error);
}

TEST(ValueTest_testIncrement, checkValueChanges)
{
    dag::Value sut;
    sut += std::int64_t(1);
    EXPECT_EQ(1, std::int64_t(sut));
}

class NodeCategoryTest : public ::testing::TestWithParam<std::tuple<const char*, const char*, dag::NodeCategory::Category>>
{

};

TEST_P(NodeCategoryTest, checkCategory)
{
    std::string className = std::get<0>(GetParam());
    std::string name = std::get<1>(GetParam());
    dag::NodeCategory::Category category = std::get<2>(GetParam());
    dag::MemoryNodeLibrary nodeLib;
    dag::Node* actual = nodeLib.instantiateNode(0, className, name);
    ASSERT_NE(nullptr, actual);
    EXPECT_EQ(category, actual->category());
    delete actual;
}

INSTANTIATE_TEST_SUITE_P(NodeCategoryVerifyCategoryTest, NodeCategoryTest, ::testing::Values(
    std::make_tuple("Foo", "foo1", dag::NodeCategory::CAT_SINK)
));

class PortTypeTest : public ::testing::TestWithParam<std::tuple<dag::PortType::Type, double, double>>
{

};

TEST_P(PortTypeTest, checkSetValue)
{
    dag::PortType::Type type = std::get<0>(GetParam());
    double value = std::get<1>(GetParam());
    double newValue = std::get<2>(GetParam());
    auto sut = new dag::TypedPort<double>(0, "test1", dag::PortType::TYPE_DOUBLE, dag::PortDirection::DIR_IN, value);
    EXPECT_EQ(value, double(sut->value()));
    sut->setValue(newValue);
    EXPECT_EQ(newValue, double(sut->value()));
    delete sut;
}

INSTANTIATE_TEST_SUITE_P(PortTypeSetValueTest, PortTypeTest, ::testing::Values(
    std::make_tuple(dag::PortType::TYPE_DOUBLE, 1.0, 2.0)
));

class CategoryToStringTest : public ::testing::TestWithParam<std::tuple<dag::NodeCategory::Category, const char*>>
{

};

TEST_P(CategoryToStringTest, checkToString)
{
    dag::NodeCategory::Category category = std::get<0>(GetParam());
    const char* categoryString = std::get<1>(GetParam());
    EXPECT_STREQ(categoryString, dag::NodeCategory::toString(category));
}

INSTANTIATE_TEST_SUITE_P(CategoryEnumToStringTest, CategoryToStringTest, ::testing::Values(
    std::make_tuple(dag::NodeCategory::CAT_NONE, "None"),
    std::make_tuple(dag::NodeCategory::CAT_SOURCE, "Source"),
    std::make_tuple(dag::NodeCategory::CAT_SINK, "Sink"),
    std::make_tuple(dag::NodeCategory::CAT_CONDITION, "Condition"),
    std::make_tuple(dag::NodeCategory::CAT_ACTION, "Action"),
    std::make_tuple(dag::NodeCategory::CAT_GROUP, "Group")
));

class CategoryParseTest : public ::testing::TestWithParam<std::tuple<const char*, dag::NodeCategory::Category>>
{

};

TEST_P(CategoryParseTest, checkParse)
{
    const char* categoryString = std::get<0>(GetParam());
    dag::NodeCategory::Category category = std::get<1>(GetParam());

    EXPECT_EQ(category, dag::NodeCategory::parse(categoryString));
}

INSTANTIATE_TEST_SUITE_P(CategoryStringToEnumTest, CategoryParseTest, ::testing::Values(
    std::make_tuple("None",dag::NodeCategory::CAT_NONE),
    std::make_tuple("Source",dag::NodeCategory::CAT_SOURCE),
    std::make_tuple("Sink",dag::NodeCategory::CAT_SINK),
    std::make_tuple("Condition",dag::NodeCategory::CAT_CONDITION),
    std::make_tuple("Action",dag::NodeCategory::CAT_ACTION),
    std::make_tuple("Group",dag::NodeCategory::CAT_GROUP),
    std::make_tuple("Spoo",dag::NodeCategory::CAT_UNKNOWN)
));

class NodeTest_testClone : public ::testing::TestWithParam<std::tuple<const char*, const char*, size_t>>
{

};
TEST_P(NodeTest_testClone, checkClone)
{
    const char* className = std::get<0>(GetParam());
    const char* nodeName = std::get<1>(GetParam());
    size_t index = std::get<2>(GetParam());
    dag::MemoryNodeLibrary nodeLib;
    dag::Node* node = nodeLib.instantiateNode(0, className, nodeName);
    dag::CloningFacility facility;
    dag::Node* sut = node->clone(facility, dag::CopyOp{0}, nullptr);
    ASSERT_EQ(node->totalPorts(), sut->totalPorts());
    ASSERT_NE(node->dynamicPort(index), sut->dynamicPort(index));
    ASSERT_NE(node->dynamicPort(index)->parent(), sut->dynamicPort(index)->parent());
    delete sut;
    delete node;
}

INSTANTIATE_TEST_SUITE_P(NodeTestClone, NodeTest_testClone, ::testing::Values(
    std::make_tuple("Foo", "foo1", 0),
    std::make_tuple("Bar", "bar1", 0),
    std::make_tuple("Base", "base1", 0),
    std::make_tuple("Derived", "derived1", 0),
    std::make_tuple("Final", "final1", 0),
    std::make_tuple("Final", "final1", 1),
    std::make_tuple("Final", "final1", 2)
));
TEST(TypedTransferTest, checkMakeItSo)
{
    dag::MemoryNodeLibrary nodeLib;
    auto input = nodeLib.instantiateNode(0, "FooTyped", "foo1");
    auto output = nodeLib.instantiateNode(1, "BarTyped", "bar1");

    auto typedOutput = dynamic_cast<dag::TypedPort<double>*>(output->dynamicPort(0));
    ASSERT_NE(nullptr, typedOutput);
    typedOutput->setValue(2.0);
    auto typedInput = dynamic_cast<dag::TypedPort<double>*>(input->dynamicPort(0));
    ASSERT_NE(nullptr, typedInput);
    dag::Transfer* transfer = typedOutput->connectTo(*typedInput);
    ASSERT_NE(nullptr, transfer);
    transfer->makeItSo();
    EXPECT_EQ(2.0, typedInput->value());
    delete transfer;
    delete output;
    delete input;
}

TEST(CreateTableTest, checkCreateTable)
{
    dagbase::Lua lua;

    lua.eval("t={foo=true}");
    dagbase::Table sut = lua.tableForName("t");
    const bool actual = sut.booleanForNameOrDefault("foo", false);
    ASSERT_EQ(true, actual);
}

TEST(LuaTest, checkExecuteNonExistentFile)
{
    dagbase::Lua sut;
    
    sut.execute("NonExistent.lua");
    
    ASSERT_EQ("ScriptNotFound:description:The specified script was not found:\"NonExistent.lua\"",sut.errorMessage());
}

TEST(LuaTest, testNonExistentTable)
{
    dagbase::Lua sut;

    sut.eval("t={}");
    ASSERT_FALSE(sut.tableExists("foo"));
}

TEST(LuaTest, testExistingTable)
{
    dagbase::Lua sut;

    sut.eval("t={}");
    ASSERT_TRUE(sut.tableExists("t"));
}

TEST(TableTraversalTest, testSimple)
{
    dagbase::Lua lua;
    lua.eval("t={foo=true}");
    dagbase::Table t = lua.tableForName("t");
    dagbase::TableTraversal trav(lua.get());
    bool foo = false;
    trav([&foo](lua_State* L, size_t level)
        {
            if (lua_isboolean(L, -1))
            {
                foo = lua_toboolean(L, -1);
            }

            return 0;
        });
    ASSERT_TRUE(foo);
}

TEST(TableTraversalTest, testNested)
{
    dagbase::Lua lua;
    lua.eval("t={spoo={foo=true}}");
    ASSERT_TRUE(lua.ok());
    dagbase::Table t = lua.tableForName("t");
    dagbase::TableTraversal trav(lua.get());
    bool foo = false;
    int result = trav([&foo](lua_State* L, size_t level)
        {
            if (lua_isboolean(L, -1))
            {
                foo = lua_toboolean(L, -1);
            }

            return 0;
        });
    ASSERT_EQ(0, result);
    ASSERT_TRUE(foo);
}

TEST(TableTraversalTest, testErrorInCallback)
{
    dagbase::Lua lua;
    lua.eval("t={spoo={foo=true}}");
    ASSERT_TRUE(lua.ok());
    dagbase::Table t = lua.tableForName("t");
    dagbase::TableTraversal trav(lua.get());
    bool foo = false;
    int result = trav([&foo](lua_State* L, size_t level)
        {
            if (lua_isboolean(L, -1))
            {
                foo = lua_toboolean(L, -1);
            }

            return 1;
        });
    ASSERT_EQ(1, result);
    ASSERT_FALSE(foo);
}

TEST(VariantPortTransferTest, testConnectToDifferentTypes)
{
    dag::VariantPort* source = new dag::VariantPort(0, "out1", dag::PortType::TYPE_DOUBLE, dag::PortDirection::DIR_OUT, 1.0);
    dag::VariantPort* dest = new dag::VariantPort(1, "in1", dag::PortType::TYPE_INT, dag::PortDirection::DIR_IN, std::int64_t{ 0 });
    dag::Transfer * transfer = nullptr;
    ASSERT_NO_THROW(transfer = source->connectTo(*dest));
    ASSERT_EQ(nullptr, transfer);
    delete dest;
    delete source;
}

TEST(VariantPortTransferTest, testConnectToSameType)
{
    dag::VariantPort* source = new dag::VariantPort(0, "out1", dag::PortType::TYPE_DOUBLE, dag::PortDirection::DIR_OUT, 1.0);
    dag::VariantPort* dest = new dag::VariantPort(1, "in1", dag::PortType::TYPE_DOUBLE, dag::PortDirection::DIR_IN, 0.0);
    dag::Transfer * transfer = source->connectTo(*dest);
    ASSERT_NE(nullptr, transfer);
    transfer->makeItSo();
    ASSERT_EQ(1.0, std::get<double>(dest->value()));
    delete transfer;
    delete dest;
    delete source;
}

TEST(TypedPortTransfer, testConnectToDifferentTypes)
{
    auto* source = new dag::TypedPort(0, "out1", dag::PortType::TYPE_INT, dag::PortDirection::DIR_OUT, std::int64_t{ 1 });
    auto* dest = new dag::TypedPort(1, "in1", dag::PortType::TYPE_DOUBLE, dag::PortDirection::DIR_IN, 0.0);
    dag::Transfer const * transfer = source->connectTo(*dest);
    ASSERT_EQ(nullptr, transfer);
    delete transfer;
    delete dest;
    delete source;
}

TEST(TypedPortTransfer, testConnectToMatchingType)
{
    dag::TypedPort<std::int64_t>* source = new dag::TypedPort(0, "out1", dag::PortType::TYPE_INT, dag::PortDirection::DIR_OUT, std::int64_t{ 1 });
    dag::TypedPort<std::int64_t>* dest = new dag::TypedPort(1, "in1", dag::PortType::TYPE_INT, dag::PortDirection::DIR_IN, std::int64_t{ 0 });
    dag::Transfer * transfer = source->connectTo(*dest);
    ASSERT_NE(nullptr, transfer);
    ASSERT_TRUE(transfer->valid());
    transfer->makeItSo();
    ASSERT_EQ(1, dest->value());
    delete dest;
    delete source;
    delete transfer;
}

TEST(NodeTest, testDescribe)
{
    dag::MemoryNodeLibrary nodeLib;
    dag::Foo* foo = dynamic_cast<dag::Foo*>(nodeLib.instantiateNode(0, "Foo", "foo1"));
    ASSERT_NE(nullptr, foo);
    dag::NodeDescriptor sut;
    foo->describe(sut);
    ASSERT_EQ(dag::NodeCategory::CAT_SINK, sut.category);
    ASSERT_EQ("foo1", sut.name);
    ASSERT_EQ(size_t{ 1 }, sut.ports.size());
    ASSERT_EQ("in1", sut.ports[0].name);
    ASSERT_EQ(dag::PortType::TYPE_DOUBLE, sut.ports[0].type);
    ASSERT_EQ(dag::PortDirection::DIR_IN, sut.ports[0].direction);
    delete foo;
}

TEST(GraphTest, testGraphInitiallyHasNoNodes)
{
    auto* sut = new dag::Graph();

    ASSERT_EQ(size_t{ 0 }, sut->numNodes());
    delete sut;
}

TEST(GraphTest, testGraphInitiallyHasNoSignalPaths)
{
    auto* sut = new dag::Graph();

    EXPECT_EQ(size_t{ 0 }, sut->numSignalPaths());

    delete sut;
}

TEST(GraphTest, testCannotAddANullNode)
{
    auto sut = new dag::Graph();

    sut->addNode(nullptr);
    ASSERT_EQ(size_t{ 0 }, sut->numNodes());
    ASSERT_EQ(nullptr, sut->node(0));

    delete sut;
}

TEST(GraphTest, testWhenAddingANodeThenQueryReturnsIt)
{
    dag::MemoryNodeLibrary nodeLib;
    auto* sut = new dag::Graph();
    sut->setNodeLibrary(&nodeLib);
    auto const node = nodeLib.instantiateNode(sut->nextNodeID(), "Foo", "foo1");
    sut->addNode(node);
    ASSERT_EQ(size_t{ 1 }, sut->numNodes());
    ASSERT_EQ(node, sut->node(node->id()));

    delete sut;
}

TEST(GraphTest, testCannotAddANullSignalPath)
{
    auto const sut = new dag::Graph();

    sut->addSignalPath(nullptr);
    ASSERT_EQ(size_t{ 0 }, sut->numSignalPaths());
    ASSERT_EQ(nullptr, sut->signalPath(0));
    
    delete sut;
}

TEST(GraphTest, testAfterAddingASignalPathCanQueryIt)
{
    dag::MemoryNodeLibrary nodeLib;
    auto const sut = new dag::Graph();
    sut->setNodeLibrary(&nodeLib);
    auto n1 = sut->createNode("BarTyped","out1");
    auto n2 = sut->createNode("FooTyped","in1");
    sut->addNode(n1);
    sut->addNode(n2);
    auto const path = new dag::SignalPath(n1->dynamicPort(0), n2->dynamicPort(0));
    sut->addSignalPath(path);

    ASSERT_EQ(path, sut->signalPath(path->id()));

    delete sut;
}

TEST(PortTest, testCannotConnectTwoOutputs)
{
    dag::MemoryNodeLibrary nodeLib;
    auto output1 = dynamic_cast<dag::BarTyped*>(nodeLib.instantiateNode(0, "BarTyped", "bar1"));
    auto output2 = dynamic_cast<dag::BarTyped*>(nodeLib.instantiateNode(1, "BarTyped", "bar2"));
    auto t = output1->out1()->connectTo(*output2->out1());
    ASSERT_EQ(nullptr, t);
    ASSERT_EQ(size_t{ 0 }, output1->out1()->numOutgoingConnections());
    delete output2;
    delete output1;
}

class TestNodeWithStringPort
{
public:
    TestNodeWithStringPort()
	    :
    _str(0, "out1", dag::PortType::TYPE_STRING, dag::PortDirection::DIR_OUT, "test")
    {
	    // Do nothing.
    }

    dag::TypedPort<std::string>& stringPort()
    {
        return _str;
    }
private:
    dag::TypedPort<std::string> _str;
};

TEST(PortTest, testCannotConnectDifferentTypes)
{
    dag::MemoryNodeLibrary nodeLib;
    auto output = new TestNodeWithStringPort();
    auto foo = dynamic_cast<dag::FooTyped*>(nodeLib.instantiateNode(0, "FooTyped", "foo1"));
    auto t1 = output->stringPort().connectTo(foo->in1());
    ASSERT_EQ(size_t{ 0 }, output->stringPort().numOutgoingConnections());
    ASSERT_EQ(size_t{ 0 }, foo->in1().numIncomingConnections());
    delete t1;
    delete foo;
    delete output;
}

TEST(PortTestCompatibleTypes, testCompatibleTypesIntToDouble)
{
    auto output = new dag::TypedPort(0, "output", dag::PortType::TYPE_INT, dag::PortDirection::DIR_OUT, std::int64_t{1});
    auto input = new dag::TypedPort(1, "input", dag::PortType::TYPE_DOUBLE, dag::PortDirection::DIR_IN, 2.0);
    ASSERT_EQ(false, output->isCompatibleWith(*input));
    delete input;
    delete output;
}

TEST(PortTestCompatibleTypes, testCompatibleExactMatch)
{
    dag::TypedPort<double> const* output = new dag::TypedPort(0, "output", dag::PortType::TYPE_DOUBLE, dag::PortDirection::DIR_OUT, 1.0);
    auto* input = new dag::TypedPort(1, "input", dag::PortType::TYPE_DOUBLE, dag::PortDirection::DIR_IN, 2.0);
    ASSERT_EQ(true, output->isCompatibleWith(*input));
    delete input;
    delete output;
}

TEST(PortTestCompatibleTypes, testCompatibleBoolToInt)
{
    auto const output = new dag::TypedPort(0, "output", dag::PortType::TYPE_BOOL, dag::PortDirection::DIR_OUT, true);
    auto input = new dag::TypedPort(1, "input", dag::PortType::TYPE_INT, dag::PortDirection::DIR_IN, std::int64_t{2});
    ASSERT_EQ(false, output->isCompatibleWith(*input));
    delete input;
    delete output;
}

TEST(PortTestCompatibleTypes, testCompatibleBoolToDouble)
{
    auto const * output = new dag::TypedPort(0, "output", dag::PortType::TYPE_BOOL, dag::PortDirection::DIR_OUT, true);
    auto input = new dag::TypedPort(1, "input", dag::PortType::TYPE_DOUBLE, dag::PortDirection::DIR_IN, 2.0);
    ASSERT_EQ(false, output->isCompatibleWith(*input));
    delete input;
    delete output;
}

TEST(PortTest, testDisconnectRemovesConnection)
{
    auto* output = new dag::TypedPort(0, "output", dag::PortType::TYPE_BOOL, dag::PortDirection::DIR_OUT, true);
    auto* input = new dag::TypedPort(1, "input", dag::PortType::TYPE_DOUBLE, dag::PortDirection::DIR_IN, 2.0);
    auto t1 = output->connectTo(*input);
    output->disconnect(*input);
    ASSERT_EQ(size_t{ 0 }, output->numOutgoingConnections());
    ASSERT_EQ(size_t{ 0 }, input->numIncomingConnections());
    delete t1;
    delete input;
    delete output;
}

TEST(TransferTest, testTransferToCompatiblePort)
{
    auto output = new dag::TypedPort<std::int64_t>(0, "output", dag::PortType::TYPE_INT, dag::PortDirection::DIR_OUT, 1);
    auto* input = new dag::TypedPort(1, "input", dag::PortType::TYPE_DOUBLE, dag::PortDirection::DIR_IN, 2.0);
    dag::Transfer* transfer = output->connectTo(*input);
    ASSERT_EQ(nullptr, transfer);
    delete transfer;
    delete input;
    delete output;
}

TEST(PortTest, testEachOutgoingConnection)
{
    auto output = new dag::TypedPort<std::int64_t>(0, "output", dag::PortType::TYPE_DOUBLE, dag::PortDirection::DIR_OUT, 1.0);
    auto* input = new dag::TypedPort(1, "input", dag::PortType::TYPE_DOUBLE, dag::PortDirection::DIR_IN, 2.0);
    size_t outgoingCount = 0;
    size_t incomingCount = 0;
    auto t = output->connectTo(*input);
    output->eachOutgoingConnection([&outgoingCount](dag::Port* item)
        {
            ++outgoingCount;
        });
    input->eachIncomingConnection([&incomingCount](dag::Port* item)
        {
            ++incomingCount;
        });
    ASSERT_EQ(size_t{ 1 }, outgoingCount);
    ASSERT_EQ(size_t{ 1 }, incomingCount);
    delete t;
    delete input;
    delete output;
}

TEST(NodeTest, testDeleteAnOutputNode)
{
    dag::MemoryNodeLibrary nodeLib;

    auto const input = dynamic_cast<dag::FooTyped*>(nodeLib.instantiateNode(0, "FooTyped", "foo1"));
    auto const output = dynamic_cast<dag::BarTyped*>(nodeLib.instantiateNode(1, "BarTyped", "bar1"));

    auto t = output->out1()->connectTo(input->in1());
    ASSERT_EQ(size_t{ 1 }, input->in1().numIncomingConnections());
    delete output;
    ASSERT_EQ(size_t{ 0 }, input->in1().numIncomingConnections());
    delete t;
    delete input;
}

TEST(NodeTest, testDeleteAnInputNode)
{
    dag::MemoryNodeLibrary nodeLib;

    auto const input = dynamic_cast<dag::FooTyped*>(nodeLib.instantiateNode(0, "FooTyped", "foo1"));
    auto const output = dynamic_cast<dag::BarTyped*>(nodeLib.instantiateNode(1, "BarTyped", "bar1"));

    auto t = output->out1()->connectTo(input->in1());
    ASSERT_EQ(size_t{ 1 }, output->out1()->numOutgoingConnections());
    delete input;
    ASSERT_EQ(size_t{ 0 }, output->out1()->numOutgoingConnections());
    delete output;
    delete t;
}

TEST(NodeTest, testDeleteInputOutputNode)
{
    dag::MemoryNodeLibrary nodeLib;

    auto const input = dynamic_cast<dag::FooTyped*>(nodeLib.instantiateNode(0, "FooTyped", "foo1"));
    auto const output = dynamic_cast<dag::BarTyped*>(nodeLib.instantiateNode(1, "BarTyped", "bar1"));
    auto group = dynamic_cast<dag::GroupTyped*>(nodeLib.instantiateNode(2, "GroupTyped", "group1"));
    auto t1 = group->out1().connectTo(input->in1());
    auto t2 = output->out1()->connectTo(group->in1());
    ASSERT_EQ(size_t{ 1 }, input->in1().numIncomingConnections());
    ASSERT_EQ(size_t{ 1 }, output->out1()->numOutgoingConnections());
    delete group;
    ASSERT_EQ(size_t{ 0 }, input->in1().numIncomingConnections());
    ASSERT_EQ(size_t{ 0 }, output->out1()->numOutgoingConnections());
    delete output;
    delete input;
    delete t2;
    delete t1;
}

TEST(NodeTest, testPortsForDerived)
{
    ASSERT_NE(nullptr, dag::Derived::metaPort(0));
	ASSERT_EQ("direction", dag::Derived::metaPort(0)->name);
    ASSERT_NE(nullptr, dag::Derived::metaPort(1));
    ASSERT_EQ("trigger", dag::Derived::metaPort(1)->name);
}

TEST(NodeTest, testDynamicPortDescriptorReturnsStaticPortsInRange)
{
    dag::MemoryNodeLibrary nodeLib;
    auto const sut = dynamic_cast<dag::Final*>(nodeLib.instantiateNode(0, "Final", "final1"));
    ASSERT_NE(nullptr, sut);
    ASSERT_NE(nullptr, sut->dynamicMetaPort(0));
    ASSERT_EQ("direction", sut->dynamicMetaPort(0)->name);
    ASSERT_NE(nullptr, sut->dynamicMetaPort(1));
    ASSERT_EQ("trigger", sut->dynamicMetaPort(1)->name);
    delete sut;
}

TEST(NodeTest, testDynamicsPortDescriptorsForFinal)
{
    dag::MemoryNodeLibrary nodeLib;
    auto const sut = dynamic_cast<dag::Final*>(nodeLib.instantiateNode(0, "Final", "final1"));
    ASSERT_NE(nullptr, sut);
    sut->addDynamicPort(new dag::TypedPort<double>(0, "output1", dag::PortType::TYPE_DOUBLE, dag::PortDirection::DIR_OUT, 1.0));
    ASSERT_NE(nullptr, sut->dynamicMetaPort(2));
    ASSERT_EQ("int1", sut->dynamicMetaPort(2)->name);
    ASSERT_EQ(size_t{ 4 }, sut->totalPorts());
    delete sut;
}

class NodeTestDynamicPortsForNode : public ::testing::TestWithParam<std::tuple<const char*, size_t, const char*>>
{
	
};

TEST_P(NodeTestDynamicPortsForNode, testDynamicPortsForFinal)
{
    std::string const className = std::get<0>(GetParam());
    size_t const index = std::get<1>(GetParam());
    std::string const nodeName = std::get<2>(GetParam());
    dag::MemoryNodeLibrary nodeLib;
    auto const sut = nodeLib.instantiateNode(0, className, "node1");
    ASSERT_NE(nullptr, sut);
    sut->addDynamicPort(new dag::TypedPort<double>(0, "output1", dag::PortType::TYPE_DOUBLE, dag::PortDirection::DIR_OUT, 1.0));
    auto const actualPort = sut->dynamicPort(index);
    ASSERT_NE(nullptr, actualPort);
    ASSERT_EQ(nodeName, actualPort->name());
    auto const actualDescriptor = sut->dynamicMetaPort(index);
    ASSERT_NE(nullptr, actualDescriptor);
    ASSERT_EQ(nodeName, actualDescriptor->name);
    delete sut;
}

INSTANTIATE_TEST_SUITE_P(NodeTest, NodeTestDynamicPortsForNode, ::testing::Values(
    std::make_tuple("Final", 0, "direction"),
    std::make_tuple("Final", 1, "trigger"),
    std::make_tuple("Final", 2, "int1"),
    std::make_tuple("Final", 3, "output1")
));

class NodeTestDynamicPortDescriptorForNode : public ::testing::TestWithParam<std::tuple<const char*, size_t, const char*, dag::PortType::Type, dag::PortDirection::Direction>>
{
};

TEST_P(NodeTestDynamicPortDescriptorForNode, testDynamicPortDescriptor)
{
    std::string const className = std::get<0>(GetParam());
    size_t const index = std::get<1>(GetParam());
    std::string const nodeName = std::get<2>(GetParam());
    dag::PortType::Type type = std::get<3>(GetParam());
    dag::PortDirection::Direction dir = std::get<4>(GetParam());
    dag::MemoryNodeLibrary nodeLib;
    auto const sut = nodeLib.instantiateNode(0, className, "node1");
    ASSERT_NE(nullptr, sut);
    auto const actualPort = sut->dynamicPort(index);
    ASSERT_NE(nullptr, actualPort);
    ASSERT_EQ(nodeName, actualPort->name());
    auto const actualDescriptor = sut->dynamicMetaPort(index);
    ASSERT_NE(nullptr, actualDescriptor);
    ASSERT_EQ(nodeName, actualDescriptor->name);
    ASSERT_EQ(type, actualDescriptor->type);
    ASSERT_EQ(dir, actualDescriptor->direction);
    delete sut;
}

INSTANTIATE_TEST_SUITE_P(NodeTest, NodeTestDynamicPortDescriptorForNode, ::testing::Values(
    std::make_tuple("Foo", 0, "in1", dag::PortType::TYPE_DOUBLE, dag::PortDirection::DIR_IN),
    std::make_tuple("Bar", 0, "out1", dag::PortType::TYPE_DOUBLE, dag::PortDirection::DIR_OUT),
    std::make_tuple("Base", 0, "direction", dag::PortType::TYPE_DOUBLE, dag::PortDirection::DIR_OUT),
    std::make_tuple("Derived", 1, "trigger", dag::PortType::TYPE_BOOL, dag::PortDirection::DIR_IN),
    std::make_tuple("Final", 0, "direction", dag::PortType::TYPE_DOUBLE, dag::PortDirection::DIR_OUT),
    std::make_tuple("Final", 1, "trigger", dag::PortType::TYPE_BOOL, dag::PortDirection::DIR_IN),
    std::make_tuple("FooTyped", 0, "in1", dag::PortType::TYPE_DOUBLE, dag::PortDirection::DIR_IN),
    std::make_tuple("BarTyped", 0, "out1", dag::PortType::TYPE_DOUBLE, dag::PortDirection::DIR_OUT),
    std::make_tuple("GroupTyped", 0, "out1", dag::PortType::TYPE_DOUBLE, dag::PortDirection::DIR_OUT),
    std::make_tuple("GroupTyped", 1, "in1", dag::PortType::TYPE_DOUBLE, dag::PortDirection::DIR_IN)
));

TEST(GraphTest, testTopologicalSortEmptyGraphReturnsEmptyArray)
{
    auto sut = new dag::Graph();

    dag::NodeArray actual;
    sut->topologicalSort(&actual);

    ASSERT_TRUE(actual.empty());

    delete sut;
}

bool before(const dag::NodeArray& a, dag::Node* first, dag::Node* last)
{
    auto firstPos = std::find(a.begin(), a.end(), first);
    auto lastPos = std::find(a.begin(), a.end(), last);
    if (firstPos!=a.end() && lastPos!=a.end())
    {
        return firstPos<lastPos;
    }

    return false;
}

TEST(GraphTest, testTopologicalSortSimpleDependency)
{
    dag::MemoryNodeLibrary nodeLib;
    auto sut = new dag::Graph();
    sut->setNodeLibrary(&nodeLib);
    auto a = dynamic_cast<dag::FooTyped*>(sut->createNode("FooTyped", "foo1"));
    auto b = dynamic_cast<dag::BarTyped*>(sut->createNode("BarTyped", "bar1"));
    auto t = b->out1()->connectTo(a->in1());
    sut->addNode(a);
    sut->addPort(&a->in1());
    auto path = new dag::SignalPath(b->out1(), &a->in1());
/*    path->removed = false;
    path->source.node = b->id();
    path->source.port = b->out1()->id();
    path->dest.node = a->id();
    path->dest.port = a->in1().id();*/
    sut->addSignalPath(path);
    sut->addNode(b);
    sut->addPort(b->out1());
    dag::NodeArray actual;
    auto result = sut->topologicalSort(&actual);
    ASSERT_EQ(dag::Graph::TopoSortResult::OK, result);
    ASSERT_EQ(size_t{2}, actual.size());
    ASSERT_TRUE(before(actual, b, a));

    delete t;
    delete sut;
}

TEST(GraphTest, testTopologicalSortTransitiveDependency)
{
    dag::MemoryNodeLibrary nodeLib;
    auto sut = new dag::Graph();
    sut->setNodeLibrary(&nodeLib);
    auto a = dynamic_cast<dag::GroupTyped*>(sut->createNode("GroupTyped", "foo1"));
    auto b = dynamic_cast<dag::GroupTyped*>(sut->createNode("GroupTyped", "bar1"));
    auto c = dynamic_cast<dag::GroupTyped*>(sut->createNode("GroupTyped", "baz1"));
    dag::Transfer* t1 = nullptr;
    dag::Transfer* t2 = nullptr;

    {
        t1 = a->out1().connectTo(b->in1());
        sut->addNode(a);
        sut->addPort(&a->in1());
        sut->addPort(&a->out1());
        auto path = new dag::SignalPath(&a->out1(), &b->in1());
        sut->addSignalPath(path);
    }
    {
        t2 = b->out1().connectTo(c->in1());
        sut->addNode(b);
        sut->addPort(&b->in1());
        sut->addPort(&b->out1());
        auto path = new dag::SignalPath(&b->out1(), &c->in1());
        sut->addSignalPath(path);
    }
    {
        sut->addNode(c);
        sut->addPort(&c->in1());
        sut->addPort(&c->out1());
    }
    dag::NodeArray actual;
    auto result = sut->topologicalSort(&actual);
    ASSERT_EQ(dag::Graph::TopoSortResult::OK, result);
    ASSERT_EQ(size_t{3}, actual.size());
    // The order is not unique so we cannot predict its contents.
    // However, we can predict the ordering of pairs of elements.
    ASSERT_TRUE(before(actual, a,b));
    ASSERT_TRUE(before(actual, b,c));
    ASSERT_TRUE(before(actual, a,c));

    delete t2;
    delete t1;
    delete sut;
}

TEST(GraphTest, testTopologicalSortCyclicDependency)
{
    dag::MemoryNodeLibrary nodeLib;
    auto sut = new dag::Graph();
    sut->setNodeLibrary(&nodeLib);
    auto a = dynamic_cast<dag::GroupTyped*>(sut->createNode("GroupTyped", "foo1"));
    auto b = dynamic_cast<dag::GroupTyped*>(sut->createNode("GroupTyped", "bar1"));
    auto t1 = b->out1().connectTo(a->in1());
    sut->addNode(a);
    sut->addPort(&a->in1());
    sut->addPort(&a->out1());
    auto path = new dag::SignalPath(&b->out1(), &a->in1());
    sut->addSignalPath(path);
    auto t2 = a->out1().connectTo(b->in1());
    sut->addNode(b);
    sut->addPort(&b->out1());
    sut->addPort(&b->in1());
    path = new dag::SignalPath(&a->out1(), &b->in1());
    sut->addSignalPath(path);
    dag::NodeArray actual;
    auto result = sut->topologicalSort(&actual);
    ASSERT_EQ(dag::Graph::TopoSortResult::CYCLES_DETECTED, result);
    ASSERT_EQ(size_t{0}, actual.size());

    delete t2;
    delete t1;
    delete sut;
}

class TopologicalSort_testPersistent : public ::testing::TestWithParam<std::tuple<const char*, dag::Graph::TopoSortResult, std::size_t, const char*, const char*>>
{

};

TEST_P(TopologicalSort_testPersistent, testSort)
{
    const char* graphFilename = std::get<0>(GetParam());
    dag::Graph::TopoSortResult result = std::get<1>(GetParam());
    std::size_t numNodesInResult = std::get<2>(GetParam());
    std::string lhsPath = std::get<3>(GetParam());
    std::string rhsPath = std::get<4>(GetParam());

    dag::MemoryNodeLibrary nodeLib;
    auto sut = dag::Graph::fromFile(nodeLib, graphFilename);
    ASSERT_NE(nullptr, sut);
    dag::NodeArray order;
    dag::Graph::TopoSortResult actualResult = sut->topologicalSort(&order);
    EXPECT_EQ(result, actualResult);
    EXPECT_EQ(numNodesInResult, order.size());
    if (lhsPath.empty()==false && rhsPath.empty()==false)
    {
        auto lhs = sut->findNode(lhsPath);
        ASSERT_NE(nullptr, lhs);
        auto rhs = sut->findNode(rhsPath);
        ASSERT_NE(nullptr, rhs);
        EXPECT_TRUE(before(order, lhs, rhs));
    }
    delete sut;
}

INSTANTIATE_TEST_SUITE_P(TopologicalSort, TopologicalSort_testPersistent, ::testing::Values(
        std::make_tuple("etc/tests/Graph/empty.lua", dag::Graph::OK, std::size_t{0}, "", ""),
        std::make_tuple("etc/tests/Graph/onenode.lua", dag::Graph::OK, std::size_t{1}, "", ""),
        std::make_tuple("etc/tests/Graph/connectednodes.lua", dag::Graph::OK, std::size_t{2}, "bar1", "foo1"),
        std::make_tuple("etc/tests/Graph/withchildgraph.lua", dag::Graph::OK, std::size_t{2}, "", ""),
        std::make_tuple("etc/tests/Graph/withnestedchildgraph.lua", dag::Graph::OK, std::size_t{3}, "", ""),
        std::make_tuple("etc/tests/Graph/connectednestedchildgraph.lua", dag::Graph::OK, std::size_t{3}, "child[0].bar1", "child[0].child[0].bound1")
        ));

class Graph_testFindAllNodes : public ::testing::TestWithParam<std::tuple<const char*, std::size_t>>
{

};

TEST_P(Graph_testFindAllNodes, testFindAllNodes)
{
    auto graphFilename = std::get<0>(GetParam());
    std::size_t numNodes = std::get<1>(GetParam());
    dag::MemoryNodeLibrary nodeLib;
    auto sut = dag::Graph::fromFile(nodeLib, graphFilename);
    ASSERT_NE(nullptr, sut);
    dag::NodeArray actual;
    sut->findAllNodes(&actual);
    EXPECT_EQ(numNodes, actual.size());
    delete sut;
}

INSTANTIATE_TEST_SUITE_P(Graph, Graph_testFindAllNodes, ::testing::Values(
        std::make_tuple("etc/tests/Graph/empty.lua", std::size_t{0}),
        std::make_tuple("etc/tests/Graph/onenode.lua", std::size_t{1}),
        std::make_tuple("etc/tests/Graph/connectednodes.lua", std::size_t{2}),
        std::make_tuple("etc/tests/Graph/withchildgraph.lua", std::size_t{2}),
        std::make_tuple("etc/tests/Graph/withnestedchildgraph.lua", std::size_t{3})
));

class Graph_testFindNode : public ::testing::TestWithParam<std::tuple<const char*, const char*, const char*>>
{

};

TEST_P(Graph_testFindNode, testFindNode)
{
    const char* graphFilename = std::get<0>(GetParam());
    const char* path = std::get<1>(GetParam());
    std::string nodeName = std::get<2>(GetParam());
    dag::MemoryNodeLibrary nodeLib;
    auto sut = dag::Graph::fromFile(nodeLib,graphFilename);
    ASSERT_NE(nullptr, sut);
    auto actual = sut->findNode(path);
    ASSERT_NE(nullptr, actual);
    EXPECT_EQ(nodeName, actual->name());
    delete sut;
}

INSTANTIATE_TEST_SUITE_P(Graph, Graph_testFindNode, ::testing::Values(
        std::make_tuple("etc/tests/Graph/onenode.lua", "foo1", "foo1"),
        std::make_tuple("etc/tests/Graph/withchildgraph.lua", "child[0].bar1", "bar1"),
        std::make_tuple("etc/tests/Graph/withnestedchildgraph.lua", "child[0].child[0].bound1", "bound1")
        ));

TEST(PortTest, testConnectToExistingPortGivesTransfer)
{
    auto source = new dag::TypedPort<double>(0, nullptr, new dag::MetaPort("out", dag::PortType::TYPE_DOUBLE, dag::PortDirection::DIR_OUT), 1.0, std::int32_t (dag::Port::OWN_META_PORT_BIT));
    auto dest = new dag::TypedPort<double>(1, nullptr, new dag::MetaPort("in", dag::PortType::TYPE_DOUBLE, dag::PortDirection::DIR_IN), 0.0, dag::Port::OWN_META_PORT_BIT);
    auto* visitor = new dag::SetValueVisitor(dag::Value(2.0));
    source->accept(*visitor);
    auto transfer = source->connectTo(*dest);
    ASSERT_NE(nullptr, transfer);
    transfer->makeItSo();
    ASSERT_EQ(2.0, dest->value());
    delete transfer;
    delete visitor;
    delete dest;
    delete source;
}

TEST(PortTest, testDisconnectPreventsTransfer)
{
    auto source = new dag::TypedPort<double>(0, nullptr, new dag::MetaPort("out", dag::PortType::TYPE_DOUBLE, dag::PortDirection::DIR_OUT), 1.0, dag::Port::OWN_META_PORT_BIT);
    auto dest = new dag::TypedPort<double>(1, nullptr, new dag::MetaPort("in", dag::PortType::TYPE_DOUBLE, dag::PortDirection::DIR_IN), 0.0, dag::Port::OWN_META_PORT_BIT);
    auto transfer = source->connectTo(*dest);
    ASSERT_NE(nullptr, transfer);
    source->disconnect(*dest);
    ASSERT_FALSE(source->isConnectedTo(dest));
    ASSERT_FALSE(dest->isConnectedTo(source));
    auto visitor = new dag::ValueVisitor();
    dest->accept(*visitor);
    ASSERT_EQ(0.0, double(visitor->value()));
    delete visitor;
    delete transfer;
    delete dest;
    delete source;
}

TEST(SelectionLiveTest, testAdd)
{
    auto sut = new dag::SelectionLive();
    dag::SelectionInterface::Cont a;
    dag::MemoryNodeLibrary nodeLib;
    auto node = nodeLib.instantiateNode(0, "FooTyped", "foo1");
    a.insert(node);
    sut->add(a.begin(), a.end());
    ASSERT_EQ(size_t{1}, sut->count());
    delete node;
    delete sut;
}

TEST(SelectionLiveTest, testSubtract)
{
    auto sut = new dag::SelectionLive();
    dag::SelectionInterface::Cont a;
    dag::MemoryNodeLibrary nodeLib;
    auto node = nodeLib.instantiateNode(0, "FooTyped", "foo1");
    a.insert(node);
    sut->add(a.begin(), a.end());
    sut->subtract(a.begin(), a.end());
    ASSERT_EQ(size_t{0}, sut->count());
    delete node;
    delete sut;
}

TEST(SelectionLiveTest, testSet)
{
    auto sut = new dag::SelectionLive();
    dag::SelectionInterface::Cont a;
    dag::MemoryNodeLibrary nodeLib;
    auto node = nodeLib.instantiateNode(0, "FooTyped", "foo1");
    a.insert(node);
    sut->add(a.begin(), a.end());
    dag::SelectionInterface::Cont b;
    auto node2 = nodeLib.instantiateNode(1, "BarTyped", "bar1");
    b.insert(node2);
    sut->set(b.begin(), b.end());
    ASSERT_EQ(size_t{1}, sut->count());
    ASSERT_FALSE(sut->isSelected(node));
    ASSERT_TRUE(sut->isSelected(node2));
    delete node2;
    delete node;
    delete sut;
}

TEST(SelectionLiveTest, testToggle)
{
    auto sut = new dag::SelectionLive();
    dag::SelectionInterface::Cont a;
    dag::MemoryNodeLibrary nodeLib;
    auto node = nodeLib.instantiateNode(0, "FooTyped", "foo1");
    auto node2 = nodeLib.instantiateNode(1, "BarTyped", "bar1");
    a.insert(node);
    a.insert(node2);
    sut->add(a.begin(), a.end());
    ASSERT_EQ(size_t{2}, sut->count());
    dag::SelectionInterface::Cont b;
    b.insert(node);
    sut->toggle(b.begin(), b.end());
    ASSERT_EQ(size_t{1}, sut->count());
    ASSERT_FALSE(sut->isSelected(node));
    ASSERT_TRUE(sut->isSelected(node2));
    delete node2;
    delete node;
    delete sut;
}

TEST(NodeEditorLiveTest, testCreateNode)
{
    auto sut = new dag::NodeEditorLive();
    auto actual = sut->createNode("FooTyped", "foo1");
    ASSERT_EQ(dag::Status::StatusCode::STATUS_OK, actual.status);
    ASSERT_EQ(dag::Status::RESULT_NODE, actual.resultType);
    ASSERT_NE(nullptr, actual.result.node);

    delete sut;
}

TEST(NodeEditorLiveTest, testConnectionBetweenExistingNodesSucceeds)
{
    auto sut = new dag::NodeEditorLive();
    auto s1 = sut->createNode("FooTyped", "foo1");
    ASSERT_EQ(dag::Status::RESULT_NODE, s1.resultType);
    auto s2 = sut->createNode("BarTyped", "bar1");
    ASSERT_EQ(dag::Status::RESULT_NODE, s2.resultType);

    auto actual = sut->connect(s2.result.node->dynamicPort(0)->id(), s1.result.node->dynamicPort(0)->id());
    ASSERT_EQ(dag::Status::StatusCode::STATUS_OK, actual.status);
    ASSERT_EQ(dag::Status::RESULT_SIGNAL_PATH_ID, actual.resultType);
    ASSERT_TRUE(actual.result.signalPathId.valid());

    delete sut;
}

TEST(NodeEditorLiveTest, testConnectionBetweenInputAndOutputFails)
{
    auto sut = new dag::NodeEditorLive();
    auto s1 = sut->createNode("FooTyped", "foo1");
    ASSERT_EQ(dag::Status::RESULT_NODE, s1.resultType);
    auto s2 = sut->createNode("BarTyped", "bar1");
    ASSERT_EQ(dag::Status::RESULT_NODE, s2.resultType);

    auto actual = sut->connect(s1.result.node->dynamicPort(0)->id(), s2.result.node->dynamicPort(0)->id());
    ASSERT_EQ(dag::Status::StatusCode::STATUS_INVALID_PORT, actual.status);
    ASSERT_EQ(dag::Status::RESULT_PORT, actual.resultType);
    ASSERT_NE(nullptr, actual.result.port);
    ASSERT_EQ(s1.result.node->dynamicPort(0)->id(), actual.result.port->id());

    delete sut;
}

TEST(NodeEditorLiveTest, testConnectionBetweenNonExistentFromPortFails)
{
    auto sut = new dag::NodeEditorLive();

    auto actual = sut->connect(dag::PortID{0}, dag::PortID{1});
    ASSERT_EQ(dag::Status::StatusCode::STATUS_OBJECT_NOT_FOUND, actual.status);
    ASSERT_EQ(dag::Status::RESULT_PORT_ID, actual.resultType);
    ASSERT_EQ(0, actual.result.portId);

    delete sut;
}

TEST(NodeEditorLiveTest, testConnectionBetweenNonExistentToPortFails)
{
    auto sut = new dag::NodeEditorLive();
    auto s1 = sut->createNode("BarTyped", "bar1");
    ASSERT_EQ(dag::Status::STATUS_OK, s1.status);
    auto actual = sut->connect(s1.result.node->dynamicPort(0)->id(), dag::PortID{1});
    ASSERT_EQ(dag::Status::StatusCode::STATUS_OBJECT_NOT_FOUND, actual.status);
    ASSERT_EQ(dag::Status::RESULT_PORT_ID, actual.resultType);
    ASSERT_EQ(1, actual.result.portId);

    delete sut;
}

TEST(NodeEditorLiveTest, testCreateChildWithAnEmptySelectionFails)
{
    auto sut = new dag::NodeEditorLive();

    auto actual = sut->createChild();
    ASSERT_EQ(dag::Status::STATUS_INVALID_SELECTION, actual.status);
    ASSERT_EQ(dag::Status::RESULT_NONE, actual.resultType);

    delete sut;
}

TEST(NodeEditorLiveTest, testSelectAll)
{
    auto sut = new dag::NodeEditorLive();
    auto s1 = sut->createNode("GroupTyped", "group1");
    ASSERT_EQ(dag::Status::STATUS_OK, s1.status);
    ASSERT_EQ(dag::Status::RESULT_NODE, s1.resultType);
    ASSERT_NE(nullptr, s1.result.node);
    sut->selectAll();
    ASSERT_EQ(size_t{1}, sut->selectionCount());
    delete sut;
}

TEST(NodeEditorLiveTest, testCreateChildWithSingleChildSucceeds)
{
    auto sut = new dag::NodeEditorLive();
    auto s1 = sut->createNode("GroupTyped", "group1");
    auto s2 = sut->createNode("FooTyped", "foo1");
    auto s3 = sut->createNode("BarTyped", "bar1");
    auto t1 = s3.result.node->dynamicPort(0)->connectTo(*s1.result.node->dynamicPort(1));
    auto t2 = s1.result.node->dynamicPort(0)->connectTo(*s2.result.node->dynamicPort(0));
    ASSERT_EQ(dag::Status::STATUS_OK, s1.status);
    ASSERT_EQ(dag::Status::RESULT_NODE, s1.resultType);
    ASSERT_NE(nullptr, s1.result.node);
    dag::SelectionInterface::Cont c;
    c.insert(s1.result.node);
    auto s4 = sut->select(dag::NodeEditorInterface::SELECTION_SET, c);
    ASSERT_EQ(size_t{1}, sut->selectionCount());
    auto actual = sut->createChild();
    ASSERT_EQ(dag::Status::STATUS_OK, actual.status);
    ASSERT_EQ(dag::Status::RESULT_GRAPH, actual.resultType);
    ASSERT_NE(nullptr, actual.result.graph);
    ASSERT_EQ(size_t{3}, actual.result.graph->numNodes());
    ASSERT_NE(nullptr, actual.result.graph->parent());
    delete t2;
    delete t1;
    delete sut;
}

void createNode(dag::NodeEditorInterface* sut, const char* className, const char* nodeName, dag::Node** node)
{
    auto s1 = sut->createNode(className, nodeName);
    ASSERT_EQ(dag::Status::STATUS_OK, s1.status);
    ASSERT_EQ(dag::Status::ResultType::RESULT_NODE, s1.resultType);
    ASSERT_NE(nullptr, s1.result.node);
    if (node != nullptr)
    {
        *node = s1.result.node;
    }
}

template <typename T>
void createPort(dag::PortID id, dag::Node* parent, const char* name, dag::PortType::Type type, dag::PortDirection::Direction dir, T value)
{
    auto port = new dag::TypedPort<T>(id, parent, new dag::MetaPort(name, type, dir), value, dag::Port::OWN_META_PORT_BIT);
    parent->addDynamicPort(port);
}

TEST(NodeEditorLiveTest, testCreateChildWithMultipleChildrenSucceeds)
{
    auto sut = new dag::NodeEditorLive();

    dag::Node* multi1 = nullptr;
    createNode(sut,"Final", "multi1", &multi1);
    createPort(0, multi1, "out1", dag::PortType::TYPE_DOUBLE, dag::PortDirection::DIR_OUT, 1.0);
    createPort(1, multi1, "in1", dag::PortType::TYPE_DOUBLE, dag::PortDirection::DIR_IN, 2.0);
    dag::Node* multi2 = nullptr;
    createNode(sut,"Final", "multi2", &multi2);
    createPort(2, multi2, "in1", dag::PortType::TYPE_DOUBLE, dag::PortDirection::DIR_IN, 2.0);

    createPort(4, multi2, "out1", dag::PortType::TYPE_DOUBLE, dag::PortDirection::DIR_OUT, 3.0);
    dag::Node* output1 = nullptr;
    createNode(sut, "BarTyped", "output1",&output1);
    auto t1 = output1->dynamicPort(0)->connectTo(*multi1->dynamicPort(4));
    auto t2 = multi1->dynamicPort(3)->connectTo(*multi2->dynamicPort(3));
    dag::Node* input1 = nullptr;
    createNode(sut, "FooTyped", "input1",&input1);
    auto t3 = multi2->dynamicPort(4)->connectTo(*input1->dynamicPort(0));

    dag::NodeSet selection;
    selection.insert(multi1);
    selection.insert(multi2);
    sut->select(dag::NodeEditorInterface::SELECTION_SET, selection);
    auto s = sut->createChild();
    ASSERT_EQ(dag::Status::STATUS_OK, s.status);
    ASSERT_EQ(dag::Status::RESULT_GRAPH, s.resultType);
    ASSERT_NE(nullptr, s.result.graph);
    ASSERT_EQ(size_t{4}, s.result.graph->numNodes());

    delete t3;
    delete t2;
    delete t1;
    delete sut;
}

TEST(SelectionLiveTest, testComputeBoundaryOfEmptySetGivesEmptyOutputs)
{
    auto sut = new dag::SelectionLive();
    dag::NodeArray inputs, outputs, internals;
    sut->computeBoundaryNodes(&inputs, &outputs, &internals);
    ASSERT_TRUE(inputs.empty());
    ASSERT_TRUE(outputs.empty());
    delete sut;
}

TEST(SelectionLiveTest, testComputeBoundaryOnNodeWithInputsAndOutputsGivesTheNode)
{
    auto graph = new dag::Graph();
    dag::MemoryNodeLibrary nodeLib;
    auto sut = new dag::SelectionLive();
    auto g1 = dynamic_cast<dag::GroupTyped*>(nodeLib.instantiateNode(0, "GroupTyped", "group1"));
    auto f1 = dynamic_cast<dag::FooTyped*>(nodeLib.instantiateNode(1, "FooTyped", "foo1"));
    auto b1 = dynamic_cast<dag::BarTyped*>(nodeLib.instantiateNode(2, "BarTyped", "bar1"));
    ASSERT_NE(nullptr, g1);
    auto t1 = b1->out1()->connectTo(g1->in1());
    auto t2 = g1->out1().connectTo(f1->in1());
    sut->add(g1);
    dag::NodeArray inputs, outputs, internals;
    sut->computeBoundaryNodes(&inputs, &outputs, &internals);
    ASSERT_FALSE(inputs.empty());
    ASSERT_FALSE(outputs.empty());
    ASSERT_FALSE(internals.empty());
    delete t2;
    delete t1;
    delete b1;
    delete f1;
    delete g1;
    delete sut;
    delete graph;
}

TEST(BoundaryNode, testAddDynamicsPort)
{
    dag::MemoryNodeLibrary nodeLib;
    auto sut = new dag::Boundary(nodeLib, "sut", dag::NodeCategory::CAT_SOURCE);
    auto metaPort = new dag::MetaPort("input1", dag::PortType::TYPE_DOUBLE, dag::PortDirection::DIR_IN);
    auto input = new dag::TypedPort<double>(0, nullptr, metaPort, 1.0, dag::Port::OWN_META_PORT_BIT);
    ASSERT_NO_THROW(sut->addDynamicPort(input));
    ASSERT_EQ(sut, input->parent());
    ASSERT_EQ(size_t{1}, sut->totalPorts());
    ASSERT_EQ(input, sut->dynamicPort(0));
    // input now owned by sut
    delete sut;
}

TEST(BoundaryNode, testClone)
{
    dag::MemoryNodeLibrary nodeLib;
    auto sut = new dag::Boundary(nodeLib, "sut", dag::NodeCategory::CAT_SOURCE);
    auto metaPort = new dag::MetaPort("input1", dag::PortType::TYPE_DOUBLE, dag::PortDirection::DIR_IN);
    auto input = new dag::TypedPort<double>(0, nullptr, metaPort, 1.0, dag::Port::OWN_META_PORT_BIT);
    ASSERT_NO_THROW(sut->addDynamicPort(input));
    dag::CloningFacility facility;
    auto clone = sut->clone(facility, dag::CopyOp{0}, nullptr);
    ASSERT_EQ(size_t{1},sut->totalPorts());
    ASSERT_EQ(sut->totalPorts(), clone->totalPorts());
    ASSERT_NE(sut->dynamicPort(0), clone->dynamicPort(0));
    ASSERT_EQ(sut, sut->dynamicPort(0)->parent());
    ASSERT_EQ(clone, clone->dynamicPort(0)->parent());
    delete clone;
    delete sut;
}

TEST(NodeTest, testReconnectOutputsSimple)
{
    dag::MemoryNodeLibrary nodeLib;

    auto newInput = new dag::Boundary(nodeLib, "b1", dag::NodeCategory::CAT_SOURCE);
    auto oldInput = new dag::FooTyped(nodeLib, "foo1", dag::NodeCategory::CAT_SINK);
    auto output = new dag::BarTyped(nodeLib, "bar1", dag::NodeCategory::CAT_SOURCE);
    auto transfer = output->out1()->connectTo(oldInput->in1());
    dag::NodeSet selection;
    selection.insert(output);
    output->reconnectOutputs(selection, newInput);
    ASSERT_EQ(size_t{2}, newInput->totalPorts());
    ASSERT_EQ(size_t{1}, oldInput->dynamicPort(0)->numIncomingConnections());
    ASSERT_EQ(dag::PortDirection::DIR_OUT, newInput->dynamicPort(1)->dir());
    ASSERT_EQ(oldInput->dynamicPort(0), newInput->dynamicPort(1)->outgoingConnections()[0]);
    ASSERT_GT(newInput->totalPorts(), 1);
    ASSERT_EQ(size_t{1}, oldInput->totalPorts());
    ASSERT_EQ(size_t{1}, oldInput->dynamicPort(0)->numIncomingConnections());
    ASSERT_EQ(newInput->dynamicPort(1), oldInput->dynamicPort(0)->incomingConnections()[0]);
    delete transfer;
    delete output;
    delete oldInput;
    delete newInput;
}

TEST(NodeTest, testReconnectOutputsHairy)
{
    dag::MemoryNodeLibrary nodeLib;

    auto newInput = new dag::Boundary(nodeLib, "b1", dag::NodeCategory::CAT_SOURCE);
    auto oldInput = new dag::FooTyped(nodeLib, "foo1", dag::NodeCategory::CAT_SINK);
    auto output = new dag::BarTyped(nodeLib, "bar1", dag::NodeCategory::CAT_SOURCE);
    auto s1 = new dag::FooTyped(nodeLib, "foo2", dag::NodeCategory::CAT_SINK);
    auto t1 = output->out1()->connectTo(oldInput->in1());
    auto t2 = output->out1()->connectTo(s1->in1());
    dag::NodeSet selection;
    selection.insert(output);
    output->reconnectOutputs(selection, newInput);
    ASSERT_EQ(size_t{4}, newInput->totalPorts());
    ASSERT_EQ(size_t{1}, oldInput->dynamicPort(0)->numIncomingConnections());
    ASSERT_EQ(dag::PortDirection::DIR_OUT, newInput->dynamicPort(1)->dir());
    ASSERT_EQ(oldInput->dynamicPort(0), newInput->dynamicPort(1)->outgoingConnections()[0]);
    ASSERT_EQ(newInput->dynamicPort(1), oldInput->dynamicPort(0)->incomingConnections()[0]);
    ASSERT_EQ(s1->dynamicPort(0), newInput->dynamicPort(3)->outgoingConnections()[0]);
    ASSERT_EQ(newInput->dynamicPort(3), s1->dynamicPort(0)->incomingConnections()[0]);
    delete t2;
    delete t1;
    delete s1;
    delete output;
    delete oldInput;
    delete newInput;
}

TEST(NodeTest, testReconnectInputsSimple)
{
    dag::MemoryNodeLibrary nodeLib;

    auto newOutput = new dag::Boundary(nodeLib, "b1", dag::NodeCategory::CAT_SOURCE);
    auto oldOutput = new dag::BarTyped(nodeLib, "bar1", dag::NodeCategory::CAT_SOURCE);
    auto input = new dag::FooTyped(nodeLib, "foo1", dag::NodeCategory::CAT_SINK);

    auto t = oldOutput->out1()->connectTo(input->in1());
    dag::NodeSet selection;
    selection.insert(input);
    input->reconnectInputs(selection, newOutput);
    ASSERT_EQ(size_t{ 2 }, newOutput->totalPorts());
    ASSERT_EQ(dag::PortDirection::DIR_IN, newOutput->dynamicPort(1)->dir());
    ASSERT_EQ(oldOutput->dynamicPort(0), newOutput->dynamicPort(1)->incomingConnections()[0]);
    ASSERT_EQ(newOutput->dynamicPort(1), oldOutput->dynamicPort(0)->outgoingConnections()[0]);
    ASSERT_EQ(newOutput->dynamicPort(0), input->dynamicPort(0)->incomingConnections()[0]);

    delete t;
    delete input;
    delete oldOutput;
    delete newOutput;
}

TEST(NodeTest, testReconnectInputsHairy)
{
    dag::MemoryNodeLibrary nodeLib;

    auto newOutput = new dag::Boundary(nodeLib, "b1", dag::NodeCategory::CAT_SOURCE);
    auto oldOutput = new dag::BarTyped(nodeLib, "bar1", dag::NodeCategory::CAT_SOURCE);
    auto input = new dag::Final(nodeLib, "foo1", dag::NodeCategory::CAT_SINK);
    auto s1 = new dag::BarTyped(nodeLib, "bar2", dag::NodeCategory::CAT_SOURCE);
    auto mp1 = new dag::MetaPort("in1", dag::PortType::TYPE_DOUBLE, dag::PortDirection::DIR_IN);
    auto p1 = new dag::TypedPort<double>(0, nullptr, mp1, 1.0);
    input->addDynamicPort(p1);
    auto t1 = oldOutput->out1()->connectTo(*input->dynamicPort(3));
    auto mp2 = new dag::MetaPort("in2", dag::PortType::TYPE_DOUBLE, dag::PortDirection::DIR_IN);
    auto p2 = new dag::TypedPort<double>(1, nullptr, mp2, 2.0);
    input->addDynamicPort(p2);
    auto t2 = s1->out1()->connectTo(*input->dynamicPort(4));

    dag::NodeSet selection;
    selection.insert(input);
    input->reconnectInputs(selection, newOutput);
    ASSERT_EQ(size_t{ 4 }, newOutput->totalPorts());
    ASSERT_EQ(dag::PortDirection::DIR_IN, newOutput->dynamicPort(1)->dir());
    ASSERT_EQ(oldOutput->dynamicPort(0), newOutput->dynamicPort(1)->incomingConnections()[0]);
    ASSERT_EQ(newOutput->dynamicPort(1), oldOutput->dynamicPort(0)->outgoingConnections()[0]);
    ASSERT_EQ(s1->dynamicPort(0), newOutput->dynamicPort(3)->incomingConnections()[0]);
    ASSERT_EQ(newOutput->dynamicPort(3), s1->dynamicPort(0)->outgoingConnections()[0]);
    ASSERT_EQ(input->dynamicPort(4)->incomingConnections()[0], newOutput->dynamicPort(2));
    delete t2;
    delete t1;
    delete s1;
    delete input;
    delete mp2;
    delete mp1;
    delete oldOutput;
    delete newOutput;
}

TEST(NodeEditorLiveTest, testSelectNone)
{
    auto sut = new dag::NodeEditorLive();

    auto s = sut->createNode("Final", "test1");
    sut->selectAll();
    ASSERT_EQ(size_t{1}, sut->selectionCount());
    auto s2 = sut->selectNone();
    ASSERT_EQ(dag::Status::STATUS_OK, s2.status);
    ASSERT_EQ(dag::Status::RESULT_NONE, s2.resultType);
    ASSERT_EQ(size_t{0}, sut->selectionCount());

    delete sut;
}

TEST(NodeEditorLiveTest, testDisconnectNonExistentConnectionFails)
{
    auto sut = new dag::NodeEditorLive();

    auto s1 = sut->disconnect(0);
    ASSERT_EQ(dag::Status::STATUS_OBJECT_NOT_FOUND, s1.status);
    ASSERT_EQ(dag::Status::RESULT_SIGNAL_PATH_ID, s1.resultType);
    ASSERT_EQ(0,s1.result.signalPathId);

    delete sut;
}

TEST(NodeEditorLiveTest, testDisconnectExistingConnectionSucceeds)
{
    auto sut = new dag::NodeEditorLive();
    auto s1 = sut->createNode("BarTyped", "bar1");
    auto s2 = sut->createNode("FooTyped", "foo1");
    auto s3 = sut->connect(s1.result.node->dynamicPort(0)->id(), s2.result.node->dynamicPort(0)->id());
    ASSERT_EQ(dag::Status::STATUS_OK, s3.status);
    ASSERT_EQ(dag::Status::RESULT_SIGNAL_PATH_ID, s3.resultType);
    auto s4 = sut->disconnect(s3.result.signalPathId);
    ASSERT_EQ(dag::Status::STATUS_OK, s4.status);
    ASSERT_EQ(dag::Status::RESULT_NONE, s4.resultType);
    ASSERT_EQ(size_t{0}, s2.result.node->dynamicPort(0)->numIncomingConnections());
    ASSERT_EQ(size_t{0}, s1.result.node->dynamicPort(0)->numOutgoingConnections());
    auto s5 = sut->disconnect(s3.result.signalPathId);
    ASSERT_EQ(dag::Status::STATUS_OBJECT_NOT_FOUND, s5.status);
    ASSERT_EQ(dag::Status::RESULT_SIGNAL_PATH_ID, s5.resultType);
    ASSERT_EQ(s3.result.signalPathId, s5.result.signalPathId);

    delete sut;
}

TEST(NodeEditorLiveTest, testDeleteNonExistentNodeFails)
{
    auto sut = new dag::NodeEditorLive();

    auto status = sut->deleteNode(0);
    ASSERT_EQ(dag::Status::STATUS_OBJECT_NOT_FOUND, status.status);
    ASSERT_EQ(dag::Status::RESULT_NODE_ID, status.resultType);
    ASSERT_EQ(0, status.result.nodeId);

    delete sut;
}

TEST(NodeEditorLiveTest, testDeleteExistingNodeSucceeds)
{
    auto sut = new dag::NodeEditorLive();
    auto s1 = sut->createNode("FooTyped", "test1");
    auto id = s1.result.node->id();
    auto status = sut->deleteNode(id);
    ASSERT_EQ(dag::Status::STATUS_OK, status.status);
    ASSERT_EQ(dag::Status::RESULT_NODE_ID, status.resultType);
    ASSERT_EQ(id, status.result.nodeId);

    delete sut;
}

TEST(GraphTest, testCreateNode)
{
    auto sut = new dag::Graph();
    auto nodeLib = new dag::MemoryNodeLibrary();
    sut->setNodeLibrary(nodeLib);
    auto node = sut->createNode("FooTyped", "foo1");
    ASSERT_NE(nullptr, node);
    sut->addNode(node);
    delete nodeLib;
    delete sut;
}

TEST(CommandTest, testCreateNode)
{
    auto graph = new dag::Graph();
    auto nodeLib = new dag::MemoryNodeLibrary();
    graph->setNodeLibrary(nodeLib);
    auto sut = new dag::CreateNode();
    sut->setGraph(graph);
    sut->setClassName("FooTyped");
    sut->setName("foo1");
    sut->makeItSo();
    ASSERT_EQ(size_t{1}, graph->numNodes());
    sut->undo();
    ASSERT_EQ(size_t{0}, graph->numNodes());
    sut->redo();
    ASSERT_EQ(size_t{1}, graph->numNodes());
    delete sut;
    delete nodeLib;
    delete graph;
}

TEST(GraphTest, testSaveToMemento)
{
    auto sut = new dag::Graph();
    auto nodeLib = new dag::MemoryNodeLibrary();
    sut->setNodeLibrary(nodeLib);
    auto memento = sut->save();
    auto n1 = sut->createNode("FooTyped", "foo1");
    sut->addNode(n1);
    sut->restore(memento);
    ASSERT_EQ(size_t{1}, sut->numNodes());
    delete memento;
    delete nodeLib;
    delete sut;
}

TEST(ByteBufferTest, testRelativeFloat)
{
    dagbase::ByteBuffer sut;

    sut.put(101.234f);
    float f = 0.0f;
    sut.get(&f);
    ASSERT_EQ(101.234f, f);
}

TEST(ByteBufferTest, testRelativeInt)
{
    dagbase::ByteBuffer sut;

    sut.put(101);
    std::int32_t f = 0;
    sut.get(&f);
    ASSERT_EQ(101, f);
}

TEST(ByteBufferTest, testRelativeDouble)
{
    dagbase::ByteBuffer sut;

    sut.put(101.234);
    double f = 0;
    sut.get(&f);
    ASSERT_EQ(101.234, f);
}

struct TestStruct
{
    std::int64_t i{0};
    bool foo{false};

    bool operator==(const TestStruct& other) const
    {
        return i==other.i && foo==other.foo;
    }
};

TEST(ByteBufferTest, testRelativeStruct)
{
    dagbase::ByteBuffer sut;

    TestStruct value;
    value.i=100;
    value.foo=true;

    sut.put(value);
    TestStruct actualValue;
    sut.get(&actualValue);
    ASSERT_EQ(value, actualValue);
}

TEST(MemoryOutputStreamTest, testWriteBuf)
{
    dagbase::ByteBuffer buf;
    dagbase::MemoryOutputStream sut(&buf);
    float f = 123.456f;
    sut.write(f);//reinterpret_cast<dagbase::MemoryOutputStream::value_type*>(&f), sizeof(float));
    float actual = 0.0f;
    buf.get(&actual);
    ASSERT_EQ(f, actual);
}

class TestObj
{
public:
    explicit TestObj(int i)
    :
    _i(i)
    {
        // Do nothing.
    }

    explicit TestObj(dagbase::InputStream& str)
    {
        str.addObj(this);

        str.read(&_i);
    }

    [[nodiscard]]int i() const
    {
        return _i;
    }

    dagbase::OutputStream& write(dagbase::OutputStream& str) const
    {
        str.write(_i);

        return str;
    }
private:
    int _i{0};
};

TEST(MemoryOutputStreamTest, testWriteRef)
{
    dagbase::ByteBuffer buf;
    dagbase::MemoryOutputStream sut(&buf);
    TestObj obj(1);
    sut.writeRef(&obj);
    std::uint32_t id;
    buf.get(&id);
    ASSERT_EQ(id, 1);
}

TEST(MemoryOutputStreamTest, testWrite)
{
    dagbase::ByteBuffer buf;
    dagbase::MemoryOutputStream sut(&buf);
    double d = 123.456;
    sut.write(d);
    double actual = 0.0;
    dagbase::MemoryInputStream str(&buf);
    str.read(&actual);
    ASSERT_EQ(d,actual);
}

TEST(ByteBufferTest, testBulkGet)
{
    dagbase::ByteBuffer sut;
    int i=100;
    sut.put(i);
    int actual=0;
    sut.get(reinterpret_cast<dagbase::ByteBuffer::BufferType::value_type*>(&actual), sizeof(int));
    ASSERT_EQ(i,actual);
}

TEST(MemoryInputStreamTest, testReadRef)
{
    dagbase::ByteBuffer buf;
    dagbase::MemoryOutputStream str(&buf);
    auto s = new TestObj(1);
    str.writeRef(&s);
    s->write(str);
    dagbase::MemoryInputStream sut(&buf);
    dagbase::InputStream::ObjId id = -1;
    TestObj* actual = sut.readRef<TestObj>(&id);
//    if (id!=0)
//    {
//        if (ref == nullptr)
//        {
//            actual = new TestObj(sut);
//            ASSERT_EQ(s->i(), actual->i());
//        }
//        else
//        {
//            actual = static_cast<TestObj*>(ref);
//        }
//    }
    ASSERT_NE(nullptr, actual);
    ASSERT_EQ(s->i(), actual->i());
    delete actual;
    delete s;
}

class TestNode
{
public:
    explicit TestNode(TestNode* parent)
    :
    _parent(parent)
    {

    }

    explicit TestNode(dagbase::InputStream& str)
    :
    _parent(nullptr)
    {
        str.addObj(this);
        dagbase::Stream::ObjId parentId{0};
        auto parentRef = str.readRef<TestObj>(&parentId);

        str.read(&_value);

        std::size_t numChildren = 0;
        str.read(&numChildren);

        for (auto i=0; i<numChildren; ++i)
        {
            dagbase::Stream::ObjId childId = 0;
            auto child = str.readRef<TestNode>(&childId);

            addChild(child);
        }
    }

    ~TestNode()
    {
        for (auto child : _children)
        {
            delete child;
        }
    }

    dagbase::OutputStream& write(dagbase::OutputStream& str) const
    {
        if (str.writeRef(_parent))
        {
            _parent->write(str);
        }

        str.write(_value);
        str.write(_children.size());

        for (auto child : _children)
        {
            if (str.writeRef(child))
            {
                child->write(str);
            }
        }
        return str;
    }

    void setValue(int value)
    {
        _value = value;
    }

    [[nodiscard]]int value() const
    {
        return _value;
    }

    void addChild(TestNode* child)
    {
        if (child!=nullptr)
        {
            _children.push_back(child);
            child->_parent = this;
        }
    }

    TestNode* child(std::size_t index)
    {
        if (index<_children.size())
        {
            return _children[index];
        }

        return nullptr;
    }

    [[nodiscard]]std::size_t numChildren() const
    {
        return _children.size();
    }
private:
    TestNode* _parent;
    typedef std::vector<TestNode*> TestNodeArray;
    TestNodeArray _children;
    int _value;
};

TEST(MemoryInputStreamTest, testReadLinked)
{
    dagbase::ByteBuffer buf;
    dagbase::MemoryOutputStream str(&buf);
    auto parent = new TestNode(nullptr);
    auto obj = new TestNode(parent);
    parent->addChild(obj);
    auto obj2 = new TestNode(parent);
    parent->addChild(obj2);
    parent->setValue(1);
    obj->setValue(2);
    if (str.writeRef(parent))
    {
        parent->write(str);
    }

    dagbase::MemoryInputStream sut(&buf);

    dagbase::Stream::ObjId rootId = 0;
    auto* actual = sut.readRef<TestNode>(&rootId);
    ASSERT_EQ(parent->value(), actual->value());
    ASSERT_EQ(std::size_t{2}, actual->numChildren());
    ASSERT_EQ(2, actual->child(0)->value());
    delete actual;
    delete parent;
}

class Graph_testSerialisation : public ::testing::TestWithParam<std::tuple<std::string_view, const char*>>
{

};

TEST_P(Graph_testSerialisation, testRoundTrip)
{
    auto formatClassName = std::get<0>(GetParam());
    auto graphFilename = std::get<1>(GetParam());
    dagbase::StreamFormat* format = nullptr;
    dagbase::MemoryBackingStore store(dagbase::BackingStore::MODE_OUTPUT_BIT);
    if (formatClassName == "TextFormat")
    {
        format = new dagbase::TextFormat(&store);
    }
    else if (formatClassName == "BinaryFormat")
    {
        format = new dagbase::BinaryFormat(&store);
    }
    ASSERT_NE(nullptr, format);
    format->setMode(dagbase::StreamFormat::MODE_OUTPUT);
    dag::MemoryNodeLibrary nodeLib;
    auto g1 = dag::Graph::fromFile(nodeLib, graphFilename);

    auto out = new dagbase::FormatAgnosticOutputStream();
    out->setFormat(format);
    out->setBackingStore(&store);
    if (out->writeRef(g1))
    {
        g1->write(*out);
    }
    format->flush();
    format->debug();
    auto in = new dagbase::FormatAgnosticInputStream();
    in->setFormat(format);
    in->setBackingStore(&store);
    format->setMode(dagbase::StreamFormat::MODE_INPUT);
    store.open(dagbase::BackingStore::MODE_INPUT_BIT);
    dagbase::Stream::ObjId id = 0;
    dag::Graph* g2 = nullptr;
    dagbase::Stream::Ref ref = in->readRef(&id);
    if (id != 0)
    {
        if (ref != nullptr)
        {
            g2 = static_cast<dag::Graph*>(ref);
        }
        else
        {
            g2 = new dag::Graph(*in, nodeLib);
        }
    }

    ASSERT_EQ(*g1, *g2);
    delete g2;
    delete in;
    delete out;
    delete g1;
    delete format;
}

INSTANTIATE_TEST_SUITE_P(Graph, Graph_testSerialisation, ::testing::Values(
    std::make_tuple("TextFormat", "etc/tests/Graph/empty.lua"),
    std::make_tuple("BinaryFormat", "etc/tests/Graph/empty.lua"),
    std::make_tuple("TextFormat", "etc/tests/Graph/onenode.lua"),
    std::make_tuple("BinaryFormat", "etc/tests/Graph/onenode.lua"),
    std::make_tuple("TextFormat", "etc/tests/Graph/connectednodes.lua"),
    std::make_tuple("BinaryFormat", "etc/tests/Graph/connectednodes.lua")
    ));

TEST(GraphTest, testSerialisationOneNode)
{
    dag::MemoryNodeLibrary nodeLib;
    auto g1 = new dag::Graph();
    g1->setNodeLibrary(&nodeLib);
    auto n1 = g1->createNode("FooTyped", "foo1");
    g1->addNode(n1);
    auto buf = new dagbase::ByteBuffer();
    auto out = new dagbase::MemoryOutputStream(buf);
    if (out->writeRef(g1))
    {
        g1->write(*out);
    }
    g1->debug();
    auto in = new dagbase::MemoryInputStream(buf);
    dagbase::Stream::ObjId id = 0;
    dag::Graph* g2 = nullptr;
    dagbase::Stream::Ref ref = in->readRef(&id);
    if (id != 0)
    {
        if (ref != nullptr)
        {
            g2 = static_cast<dag::Graph*>(ref);
        }
        else
        {
            g2 = new dag::Graph(*in, nodeLib);
        }
    }
    ASSERT_EQ(*g1, *g2);
    delete g2;
    delete in;
    delete out;
    delete buf;
    // g1 owns the Nodes.
    delete g1;
}

TEST(GraphTest, testSerialisationTwoConnectedNodes)
{
    dag::MemoryNodeLibrary nodeLib;
    auto g1 = new dag::Graph();
    g1->setNodeLibrary(&nodeLib);
    auto n1 = g1->createNode("FooTyped", "foo1");
    g1->addNode(n1);
    auto n2 = g1->createNode("BarTyped", "bar1");
    g1->addNode(n2);
    auto t1 = n2->dynamicPort(0)->connectTo(*n1->dynamicPort(0));
    auto buf = new dagbase::ByteBuffer();
    auto out = new dagbase::MemoryOutputStream(buf);
    if (out->writeRef(g1))
    {
        g1->write(*out);
    }
    g1->debug();
    auto in = new dagbase::MemoryInputStream(buf);
    dagbase::Stream::ObjId id = 0;
    dag::Graph* g2 = nullptr;
    dagbase::Stream::Ref ref = in->readRef(&id);
    if (id != 0)
    {
        if (ref != nullptr)
        {
            g2 = static_cast<dag::Graph*>(ref);
        }
        else
        {
            g2 = new dag::Graph(*in, nodeLib);
        }
    }
    ASSERT_EQ(*g1, *g2);
    delete g2;
    delete in;
    delete out;
    delete buf;
    delete t1;
    delete g1;
}

TEST(MemoryOutputStreamTest, testWriteString)
{
    dagbase::ByteBuffer buf;
    auto str = new dagbase::MemoryOutputStream(&buf);
    std::string expected="test";
    str->writeString(expected, false);
    auto sut = new dagbase::MemoryInputStream(&buf);
    std::string actual;
    sut->readString(&actual, false);
    ASSERT_EQ(expected, actual);
    delete sut;
    delete str;
}

class GraphTest_fromLua : public ::testing::TestWithParam<std::tuple<const char*, std::size_t, std::size_t, dag::NodeID, std::size_t, dag::Value>>
{

};

TEST_P(GraphTest_fromLua, testFromString)
{
    const char* graphStr = std::get<0>(GetParam());
    std::size_t numNodes = std::get<1>(GetParam());
    std::size_t numSignalPaths = std::get<2>(GetParam());
    dag::NodeID nodeId = std::get<3>(GetParam());
    std::size_t portIndex = std::get<4>(GetParam());
    dag::Value value = std::get<5>(GetParam());

    dag::MemoryNodeLibrary nodeLib;
    auto sut = dag::Graph::fromString(nodeLib, graphStr);
    ASSERT_NE(nullptr, sut);
    EXPECT_EQ(numNodes, sut->numNodes());
    EXPECT_EQ(numSignalPaths, sut->numSignalPaths());
    auto actualNode = sut->lastAddedNode();
    ASSERT_NE(nullptr, actualNode);
    auto actualPort = actualNode->dynamicPort(portIndex);
    ASSERT_NE(nullptr, actualPort);
    dag::ValueVisitor valueVisitor;
    actualPort->accept(valueVisitor);
    dag::Value actualValue = valueVisitor.value();

    EXPECT_EQ(value, actualValue);
    delete sut;
}

INSTANTIATE_TEST_SUITE_P(GraphTest, GraphTest_fromLua, ::testing::Values(
    std::make_tuple("graph={ nodes={ { name=\"foo\", class=\"FooTyped\", category=\"CATEGORY_SINK\", ports={ { name=\"in1\", class=\"TypedPort<double>\", type=\"TYPE_DOUBLE\", dir=\"DIR_IN\", value=2.0 } } } } }", std::size_t{ 1 }, std::size_t{ 0 }, dag::NodeID{ 0 }, std::size_t{ 0 }, dag::Value{2.0} ),
    std::make_tuple("graph={ nodes={ { name=\"foo\", class=\"Boundary\", category=\"CATEGORY_GROUP\", ports={ { name=\"in1\", class=\"TypedPort<double>\", type=\"TYPE_DOUBLE\", dir=\"DIR_IN\", value=2.0 } } } } }", std::size_t{ 1 }, std::size_t{ 0 }, dag::NodeID{ 0 }, std::size_t{ 0 }, dag::Value{2.0} ),
    std::make_tuple("graph={ nodes={ { name=\"foo\", class=\"Boundary\", category=\"CATEGORY_GROUP\", ports={ { name=\"in1\", class=\"TypedPort<int64_t>\", type=\"TYPE_INT\", dir=\"DIR_IN\", value=2 } } } } }", std::size_t{ 1 }, std::size_t{ 0 }, dag::NodeID{ 0 }, std::size_t{ 0 }, dag::Value{std::int64_t(2)} ),
    std::make_tuple("graph={ nodes={ { name=\"foo\", class=\"Boundary\", category=\"CATEGORY_GROUP\", ports={ { name=\"in1\", class=\"TypedPort<string>\", type=\"TYPE_STRING\", dir=\"DIR_IN\", value=\"wibble\" } } } } }", std::size_t{ 1 }, std::size_t{ 0 }, dag::NodeID{ 0 }, std::size_t{ 0 }, dag::Value{std::string("wibble")} ),
    std::make_tuple("graph={ nodes={ { name=\"foo\", class=\"Boundary\", category=\"CATEGORY_GROUP\", ports={ { name=\"in1\", class=\"TypedPort<bool>\", type=\"TYPE_BOOL\", dir=\"DIR_IN\", value=true } } } } }", std::size_t{ 1 }, std::size_t{ 0 }, dag::NodeID{ 0 }, std::size_t{ 0 }, dag::Value{true} )
));

TEST(GraphTest, testDuplciateNodeID)
{
    dag::MemoryNodeLibrary nodeLib;
    const char* graphFilename = "etc/tests/Graph/duplicateID.lua";
    auto sut = dag::Graph::fromFile(nodeLib, graphFilename);

    ASSERT_EQ(nullptr, sut);
}

class GraphTest_fromLuaFile : public ::testing::TestWithParam<std::tuple<const char*, std::size_t, std::size_t, dag::NodeID, std::size_t, dag::Value, std::size_t, std::size_t, std::size_t>>
{

};

TEST_P(GraphTest_fromLuaFile, testFromFile)
{
    const char* graphStr = std::get<0>(GetParam());
    std::size_t numNodes = std::get<1>(GetParam());
    std::size_t numSignalPaths = std::get<2>(GetParam());
    dag::NodeID nodeId = std::get<3>(GetParam());
    std::size_t portIndex = std::get<4>(GetParam());
    dag::Value value = std::get<5>(GetParam());
    std::size_t numIncomingPorts = std::get<6>(GetParam());
    std::size_t numOutgoingPorts = std::get<7>(GetParam());
    std::size_t numChildren = std::get<8>(GetParam());

    dag::MemoryNodeLibrary nodeLib;
    auto sut = dag::Graph::fromFile(nodeLib, graphStr);
    ASSERT_NE(nullptr, sut);
    EXPECT_EQ(numNodes, sut->numNodes());
    EXPECT_EQ(numSignalPaths, sut->numSignalPaths());
    auto actualNode = sut->node(nodeId);
    ASSERT_NE(nullptr, actualNode);
    auto actualPort = actualNode->dynamicPort(portIndex);
    ASSERT_NE(nullptr, actualPort);
    dag::ValueVisitor valueVisitor;
    actualPort->accept(valueVisitor);
    dag::Value actualValue = valueVisitor.value();
    std::size_t actualNumIncomingPorts = actualPort->numIncomingConnections();
    std::size_t actualNumOutgoingPorts = actualPort->numOutgoingConnections();
    std::size_t actualNumChildren = sut->numChildrenRecursive();
    EXPECT_EQ(value, actualValue);
    EXPECT_EQ(numIncomingPorts, actualNumIncomingPorts);
    EXPECT_EQ(numOutgoingPorts, actualNumOutgoingPorts);
    EXPECT_EQ(numChildren, actualNumChildren);
    delete sut;
}

INSTANTIATE_TEST_SUITE_P(GraphTest, GraphTest_fromLuaFile, ::testing::Values(
    std::make_tuple("etc/tests/Graph/onenode.lua", std::size_t{ 1 }, std::size_t{ 0 }, dag::NodeID{ 0 }, std::size_t{ 0 }, dag::Value{2.0}, std::size_t{0}, std::size_t{0}, std::size_t{0} ),
    std::make_tuple("etc/tests/Graph/connectednodes.lua", std::size_t{ 2 }, std::size_t{ 1 }, dag::NodeID{ 0 }, std::size_t{ 0 }, dag::Value{1.0}, std::size_t{1}, std::size_t{0}, std::size_t{ 0 }),
    std::make_tuple("etc/tests/Graph/withchildgraph.lua", std::size_t{ 1 }, std::size_t{ 0 }, dag::NodeID{ 0 }, std::size_t{ 0 }, dag::Value{2.0}, std::size_t{0}, std::size_t{0}, std::size_t{ 1 }),
    std::make_tuple("etc/tests/Graph/withnestedchildgraph.lua", std::size_t{ 1 }, std::size_t{ 0 }, dag::NodeID{ 0 }, std::size_t{ 0 }, dag::Value{2.0}, std::size_t{0}, std::size_t{0}, std::size_t{ 2 })
));

class GraphTest_toLuaRoundTrip : public ::testing::TestWithParam<std::tuple<const char*>>
{

};

TEST_P(GraphTest_toLuaRoundTrip, testRoundTrip)
{
    auto graphFilename = std::get<0>(GetParam());
    dag::MemoryNodeLibrary nodeLib;
    auto sut = dag::Graph::fromFile(nodeLib, graphFilename);
    ASSERT_NE(nullptr, sut);
    std::string graphString;
    std::ostringstream graphStr;
    sut->toLua(graphStr);
    graphString = graphStr.str();
    std::cerr << graphString << '\n';
    auto actual = dag::Graph::fromString(nodeLib, graphString.c_str());
    ASSERT_NE(nullptr, actual);
    ASSERT_EQ(*sut,*actual);
    delete actual;
    delete sut;
}

INSTANTIATE_TEST_SUITE_P(GraphTest, GraphTest_toLuaRoundTrip, ::testing::Values(
    std::make_tuple("etc/tests/Graph/empty.lua"),
    std::make_tuple("etc/tests/Graph/onenode.lua"),
    std::make_tuple("etc/tests/Graph/connectednodes.lua"),
    std::make_tuple("etc/tests/Graph/withchildgraph.lua"),
    std::make_tuple("etc/tests/Graph/withnestedchildgraph.lua"),
    std::make_tuple("etc/tests/Graph/typedport.lua")
));

class PortType_testParseFromString : public ::testing::TestWithParam<std::tuple<const char*, dag::PortType::Type>>
{

};

TEST_P(PortType_testParseFromString, testParseFromString)
{
    const char* str = std::get<0>(GetParam());
    dag::PortType::Type type = std::get<1>(GetParam());
    auto actualType = dag::PortType::parseFromString(str);
    EXPECT_EQ(type, actualType);
}

INSTANTIATE_TEST_SUITE_P(PortType, PortType_testParseFromString, ::testing::Values(
    std::make_tuple("TYPE_UNKNOWN", dag::PortType::TYPE_UNKNOWN),
    std::make_tuple("TYPE_INT", dag::PortType::TYPE_INT),
    std::make_tuple("TYPE_DOUBLE", dag::PortType::TYPE_DOUBLE),
    std::make_tuple("TYPE_STRING", dag::PortType::TYPE_STRING),
    std::make_tuple("TYPE_BOOL", dag::PortType::TYPE_BOOL),
    std::make_tuple("TYPE_VEC3D", dag::PortType::TYPE_VEC3D),
    std::make_tuple("TYPE_OPAQUE", dag::PortType::TYPE_OPAQUE),
    std::make_tuple("TYPE_VECTOR", dag::PortType::TYPE_VECTOR)
));

class PortType_testToString : public ::testing::TestWithParam<std::tuple<const char*, dag::PortType::Type>>
{

};

TEST_P(PortType_testToString, testParseFromString)
{
    std::string str = std::get<0>(GetParam());
    dag::PortType::Type type = std::get<1>(GetParam());
    auto actualStr = dag::PortType::toString(type);
    EXPECT_EQ(str, actualStr);
}

INSTANTIATE_TEST_SUITE_P(PortType, PortType_testToString, ::testing::Values(
    std::make_tuple("TYPE_UNKNOWN", dag::PortType::TYPE_UNKNOWN),
    std::make_tuple("TYPE_INT", dag::PortType::TYPE_INT),
    std::make_tuple("TYPE_DOUBLE", dag::PortType::TYPE_DOUBLE),
    std::make_tuple("TYPE_STRING", dag::PortType::TYPE_STRING),
    std::make_tuple("TYPE_BOOL", dag::PortType::TYPE_BOOL),
    std::make_tuple("TYPE_VEC3D", dag::PortType::TYPE_VEC3D),
    std::make_tuple("TYPE_OPAQUE", dag::PortType::TYPE_OPAQUE),
    std::make_tuple("TYPE_VECTOR", dag::PortType::TYPE_VECTOR)
));

class PortDirection_testParseFromString : public ::testing::TestWithParam<std::tuple<const char*, dag::PortDirection::Direction>>
{

};

TEST_P(PortDirection_testParseFromString, testParseFromString)
{
    const char* str = std::get<0>(GetParam());
    dag::PortDirection::Direction dir = std::get<1>(GetParam());
    auto actualDir = dag::PortDirection::parseFromString(str);
    EXPECT_EQ(dir, actualDir);
}

INSTANTIATE_TEST_SUITE_P(PortDirection, PortDirection_testParseFromString, ::testing::Values(
    std::make_tuple("DIR_UNKNOWN", dag::PortDirection::DIR_UNKNOWN),
    std::make_tuple("DIR_IN", dag::PortDirection::DIR_IN),
    std::make_tuple("DIR_OUT", dag::PortDirection::DIR_OUT),
    std::make_tuple("DIR_INTERNAL", dag::PortDirection::DIR_INTERNAL)

));

class PortDirection_testToString : public ::testing::TestWithParam<std::tuple<const char*, dag::PortDirection::Direction>>
{

};

TEST_P(PortDirection_testToString, testParseFromString)
{
    std::string str = std::get<0>(GetParam());
    dag::PortDirection::Direction dir = std::get<1>(GetParam());
    auto actualStr = dag::PortDirection::toString(dir);
    EXPECT_EQ(str, actualStr);
}

INSTANTIATE_TEST_SUITE_P(PortDirection, PortDirection_testToString, ::testing::Values(
    std::make_tuple("DIR_UNKNOWN", dag::PortDirection::DIR_UNKNOWN),
    std::make_tuple("DIR_IN", dag::PortDirection::DIR_IN),
    std::make_tuple("DIR_OUT", dag::PortDirection::DIR_OUT),
    std::make_tuple("DIR_INTERNAL", dag::PortDirection::DIR_INTERNAL)

));

class FileSystemTraverser_testFilter : public ::testing::TestWithParam<std::tuple<const char*, std::size_t>>
{

};

namespace fs = std::filesystem;

TEST_P(FileSystemTraverser_testFilter, testFilter)
{
    fs::path dir = std::get<0>(GetParam());
    std::size_t numMatches = std::get<1>(GetParam());
    std::size_t actualNumMatches {0};
    dag::FileSystemTraverser trav(dir);
    trav.eachEntry([&actualNumMatches](const fs::directory_entry& entry)
                   {
                       if (entry.is_regular_file() && entry.path().extension().string() == ".lua")
                       {
                           ++actualNumMatches;
                       }
                   });
    EXPECT_EQ(numMatches, actualNumMatches);
}

INSTANTIATE_TEST_SUITE_P(FileSystemTraverser, FileSystemTraverser_testFilter, ::testing::Values(
    std::make_tuple("etc/tests/FileSystemTraverser", std::size_t{2})
));

class NodePluginScannerTest_testFindPlugins : public ::testing::TestWithParam<std::tuple<std::size_t>>
{

};

TEST_P(NodePluginScannerTest_testFindPlugins, testFindPlugins)
{
    std::size_t totalNodes = std::get<0>(GetParam());
    dag::NodePluginScanner sut;
    dag::MemoryNodeLibrary nodeLib;
    sut.scan(nodeLib, nodeLib);
    std::size_t actualTotalNodes = sut.totalNodes();
    EXPECT_EQ(totalNodes, actualTotalNodes);
}

INSTANTIATE_TEST_SUITE_P(NodePluginScanner, NodePluginScannerTest_testFindPlugins, ::testing::Values(
    std::make_tuple(std::size_t{1})
));

TEST(NodeLibraryTest, testRegisterNodeGivenANullNodeNothingHappens)
{
    dag::MemoryNodeLibrary nodeLib;

    std::size_t numNodesBefore = nodeLib.numNodes();
    nodeLib.registerNode(nullptr);
    EXPECT_EQ(numNodesBefore, nodeLib.numNodes());
}

TEST(NodeLibraryTest, testRegisterDuplicateNodeHasNoEffect)
{
    dag::MemoryNodeLibrary nodeLib;

    std::size_t numNodesBefore = nodeLib.numNodes();
    auto existingNode = nodeLib.instantiateNode(nodeLib.nextNodeID(), "FooTyped", "foo1");
    ASSERT_NE(nullptr, existingNode);
    nodeLib.registerNode(existingNode);
    EXPECT_EQ(numNodesBefore, nodeLib.numNodes());
    delete existingNode;
}

TEST(NodeLibraryTest, testRegisterNewNodeSucceeds)
{
    dag::NodePluginScanner scanner;
    dag::MemoryNodeLibrary nodeLib;

    std::size_t numNodesBefore = nodeLib.numNodes();
    scanner.scan(nodeLib, nodeLib);
    EXPECT_EQ(numNodesBefore+1, nodeLib.numNodes());

}

TEST(GraphTest, testPersistNodeFromPlugin)
{
    dag::NodePluginScanner scanner;
    dag::MemoryNodeLibrary nodeLib;

    scanner.scan(nodeLib, nodeLib);
    auto node = nodeLib.instantiateNode(nodeLib.nextNodeID(), "NodePlugin.DynamicNode", "node1");
    ASSERT_NE(nullptr, node);
    dag::Graph* graph = new dag::Graph();
    graph->setNodeLibrary(&nodeLib);
    graph->addNode(node);
    ASSERT_EQ(size_t{1}, graph->numNodes());
    std::ostringstream  str;
    graph->toLua(str);
    auto graph2 = dag::Graph::fromString(nodeLib, str.str().c_str());
    EXPECT_EQ(*graph,*graph2);
    delete graph2;
    delete graph;
}

TEST(GraphTest, testLoadGraphWithNodesFromPlugin)
{
    dag::NodePluginScanner scanner;
    dag::MemoryNodeLibrary nodeLib;

    scanner.scan(nodeLib, nodeLib);
    auto graph = dag::Graph::fromFile(nodeLib, "etc/tests/Graph/nodesFromPlugin.lua");
    ASSERT_NE(nullptr, graph);
    auto node = graph->node(0);
    ASSERT_NE(nullptr, node);
    dag::TypedPort<double>* port = dynamic_cast<dag::TypedPort<double>*>(node->dynamicPort(0));
    ASSERT_NE(nullptr, port);
    EXPECT_EQ(1.0, port->value());
    delete graph;
}

class GraphTest_testReadFromLuaThenSerialise : public ::testing::TestWithParam<std::tuple<const char*>>
{

};

TEST_P(GraphTest_testReadFromLuaThenSerialise, testSerialise)
{
    const char* graphFilename = std::get<0>(GetParam());
    dag::MemoryNodeLibrary nodeLib;
    auto sut = dag::Graph::fromFile(nodeLib, graphFilename);
    ASSERT_NE(nullptr, sut);
    auto* buf = new dagbase::ByteBuffer();
    dagbase::MemoryOutputStream ostr(buf);
    if (ostr.writeRef(sut))
    {
        sut->write(ostr);
    }
    dagbase::MemoryInputStream istr(buf);
    dagbase::Stream::ObjId id{~0U};
    dag::Graph* actual = nullptr;
    dagbase::Stream::Ref ref = istr.readRef(&id);
    if (id != 0)
    {
        if (ref != nullptr)
        {
            actual = static_cast<dag::Graph*>(ref);
        }
        else
        {
            actual = new dag::Graph(istr, nodeLib);
        }
    }
    ASSERT_NE(nullptr, actual);
    EXPECT_EQ(*sut, *actual);
    delete actual;
    delete buf;
    delete sut;
}

INSTANTIATE_TEST_SUITE_P(Graph, GraphTest_testReadFromLuaThenSerialise, ::testing::Values(
        std::make_tuple("etc/tests/Graph/empty.lua"),
        std::make_tuple("etc/tests/Graph/onenode.lua"),
        std::make_tuple("etc/tests/Graph/connectednodes.lua"),
        std::make_tuple("etc/tests/Graph/typedport.lua"),
        std::make_tuple("etc/tests/Graph/withchildgraph.lua"),
        std::make_tuple("etc/tests/Graph/withnestedchildgraph.lua"),
        std::make_tuple("etc/tests/Graph/withmultiplechildren.lua"),
        std::make_tuple("etc/tests/Graph/constraints.lua")
        ));

class GraphTest_testEvaluate : public ::testing::TestWithParam<std::tuple<const char*, dag::NodeID, std::size_t, double>>
{
public:
    void TearDown()
    {
        delete _sut;
    }
protected:
    dag::MemoryNodeLibrary _nodeLib;
    dag::Graph* _sut;
};

TEST_P(GraphTest_testEvaluate, testEvaluate)
{
    const char* graphFilename = std::get<0>(GetParam());
    dag::NodeID nodeId = std::get<1>(GetParam());
    std::size_t portIndex = std::get<2>(GetParam());
    double value = std::get<3>(GetParam());

    _sut = dag::Graph::fromFile(_nodeLib, graphFilename);
    ASSERT_NE(nullptr, _sut);
    dag::NodeArray order;
    _sut->topologicalSort(&order);
    _sut->evaluate(order);
    dag::Node* actualNode = _sut->node(nodeId);
    ASSERT_NE(nullptr, actualNode);
    dag::Port* actualPort = actualNode->dynamicPort(portIndex);
    ASSERT_NE(nullptr, actualPort);
    dag::ValueVisitor visitor;
    ASSERT_EQ(dag::PortType::TYPE_DOUBLE, actualPort->type());
    actualPort->accept(visitor);
    EXPECT_EQ(value, visitor.value().operator double());
}

INSTANTIATE_TEST_SUITE_P(Graph, GraphTest_testEvaluate, ::testing::Values(
        std::make_tuple("etc/tests/Graph/constraints.lua", 0, 2, 1.0)
        ));

class Graph_copy : public ::testing::TestWithParam<std::tuple<const char*, dag::CopyOp, bool>>
{

};

TEST_P(Graph_copy, testCopy)
{
    const char* filename = std::get<0>(GetParam());
    dag::CopyOp copyOp = std::get<1>(GetParam());
    bool equal = std::get<2>(GetParam());
    dag::MemoryNodeLibrary nodeLib;
    auto sut = dag::Graph::fromFile(nodeLib, filename);
    ASSERT_NE(nullptr, sut);
    dag::CloningFacility facility;
    auto copy = sut->clone(facility, copyOp, &nodeLib);
    ASSERT_NE(nullptr, copy);
    EXPECT_EQ(equal, *copy == *sut);
    delete copy;
    delete sut;
}

INSTANTIATE_TEST_SUITE_P(Graph, Graph_copy, ::testing::Values(
//        std::make_tuple("etc/tests/Graph/empty.lua", dag::CopyOp::GENERATE_UNIQUE_ID_BIT, true),
//        std::make_tuple("etc/tests/Graph/onenode.lua", dag::CopyOp{0}, true),
//        std::make_tuple("etc/tests/Graph/onenode.lua", dag::CopyOp::GENERATE_UNIQUE_ID_BIT, false),
        std::make_tuple("etc/tests/Graph/connectednodes.lua", dag::CopyOp{dag::CopyOp::DEEP_COPY_INPUTS_BIT|dag::CopyOp::DEEP_COPY_OUTPUTS_BIT}, true)
//        std::make_tuple("etc/tests/Graph/connectednodes.lua", dag::CopyOp{dag::CopyOp::DEEP_COPY_INPUTS_BIT|dag::CopyOp::DEEP_COPY_OUTPUTS_BIT|dag::CopyOp::GENERATE_UNIQUE_ID_BIT}, false),
//        std::make_tuple("etc/tests/Graph/withchildgraph.lua", dag::CopyOp{dag::CopyOp::DEEP_COPY_INPUTS_BIT|dag::CopyOp::DEEP_COPY_OUTPUTS_BIT}, true),
//        std::make_tuple("etc/tests/Graph/withmultiplechildren.lua", dag::CopyOp{dag::CopyOp::DEEP_COPY_INPUTS_BIT|dag::CopyOp::DEEP_COPY_OUTPUTS_BIT}, true),
//        std::make_tuple("etc/tests/Graph/withnestedchildgraph.lua", dag::CopyOp{dag::CopyOp::DEEP_COPY_INPUTS_BIT|dag::CopyOp::DEEP_COPY_OUTPUTS_BIT}, true),
//        std::make_tuple("etc/tests/Graph/connectednestedchildgraph.lua", dag::CopyOp{dag::CopyOp::DEEP_COPY_INPUTS_BIT|dag::CopyOp::DEEP_COPY_OUTPUTS_BIT}, true)
        ));

TEST(CloningFacility, testPutNull)
{
    dag::CloningFacility sut;
    std::uint64_t id = std::uint64_t {~0U};
    sut.putOrig(nullptr, &id);
    EXPECT_EQ(std::uint64_t {0}, id);
    EXPECT_EQ(std::size_t{0}, sut.numClones());
}

TEST(CloningFacility, testPutValid)
{
    dag::CloningFacility sut;

    int data;
    std::uint64_t id = std::uint64_t {~0U};
    bool shouldClone = sut.putOrig(&data, &id);

    EXPECT_TRUE(shouldClone);
    EXPECT_EQ(std::uint64_t {1}, id);
}

struct TestLink
{
    TestLink* prev{nullptr};
    TestLink* next{nullptr};
    int data{0};

    TestLink(const TestLink& other) = delete;
    TestLink()
    {
        // Do nothing.
    }
    TestLink(const TestLink& other, dag::CloningFacility& facility);
};

TestLink::TestLink(const TestLink &other, dag::CloningFacility &facility)
{
    std::uint64_t otherId = 0;
    bool shouldClone = facility.putOrig(const_cast<TestLink*>(&other), &otherId);
    facility.addClone(otherId, this);
    std::uint64_t prevId = 0;
    if (facility.putOrig(other.prev, &prevId))
    {
        prev = new TestLink(*other.prev, facility);
    }
    else
    {
        prev = static_cast<TestLink*>(facility.getClone(prevId));
    }

    std::uint64_t nextId = 0;
    if (facility.putOrig(other.next, &nextId))
    {
        next = new TestLink(*other.next, facility);
    }
    else
    {
        next = static_cast<TestLink*>(facility.getClone(nextId));
    }
}

TEST(CloningFacility, testLinkedList)
{
    dag::CloningFacility sut;
    TestLink* head = new TestLink();
    head->next = new TestLink();
    head->next->prev = head;
    std::uint64_t id = std::uint64_t {~0U};

    {
        TestLink* headClone = new TestLink(*head, sut);
        ASSERT_NE(nullptr, headClone->next);
        EXPECT_EQ(headClone, headClone->next->prev);

        delete headClone->next;
        delete headClone;
    }

    delete head->next;
    delete head;
}

class TestClass : public dagbase::Class
{
public:
    explicit TestClass(dagbase::MetaClass* metaClass)
        :
    dagbase::Class(metaClass)
    {
        // Do nothing.
    }
};

TEST(Class, testRaiseError)
{
    auto metaClass = std::make_unique<dagbase::MetaClass>();
    auto sut = std::make_unique<TestClass>(metaClass.get());
    auto& str = sut->raiseError(dagbase::Class::TypeNotFound) << "Test";
    EXPECT_EQ("TypeNotFound:Test",sut->errorMessage());
}
