#include "config/config.h"

#include "gtest/gtest.h"
#include "LuaInterface.h"
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
#include "ByteBuffer.h"
#include "MemoryOutputStream.h"
#include "MemoryInputStream.h"
#include "FileSystemTraverser.h"
#include "NodePluginScanner.h"

#include <iostream>
#include <fstream>
#include <algorithm>
#include <filesystem>

class MemoryNodeLibraryTest : public ::testing::TestWithParam<std::tuple<const char*, const char*, size_t, const char*, nbe::PortDirection::Direction, double>>
{
};

TEST_P(MemoryNodeLibraryTest, checkInstantiate)
{
    nbe::MemoryNodeLibrary sut;
    const char* className = std::get<0>(GetParam());
    const char* name = std::get<1>(GetParam());
    size_t portIndex = std::get<2>(GetParam());
    const char* portName = std::get<3>(GetParam());
    nbe::PortDirection::Direction portDir = std::get<4>(GetParam());
    double value = std::get<5>(GetParam());
    nbe::Node* actualNode = sut.instantiateNode(0, className, name);
    ASSERT_NE(nullptr, actualNode);
    EXPECT_STREQ(name, actualNode->name().c_str());
    EXPECT_GT(actualNode->totalPorts(), portIndex);
    auto actualPort = dynamic_cast<nbe::ValuePort*>(actualNode->dynamicPort(portIndex));
    ASSERT_NE(nullptr, actualPort);
    EXPECT_STREQ(portName, actualPort->name().c_str());
    EXPECT_EQ(portDir, actualPort->dir());
    EXPECT_EQ(value, double(actualPort->value()));
    
    delete actualNode;
}

INSTANTIATE_TEST_SUITE_P(MemoryNodeLibraryInstantiateTest, MemoryNodeLibraryTest, ::testing::Values(
    std::make_tuple("Foo", "foo1", 0, "in1", nbe::PortDirection::DIR_IN, 1.0),
    std::make_tuple("Bar", "bar1", 0, "out1", nbe::PortDirection::DIR_OUT, 1.0)
));

TEST(MemoryNodeLibraryTest_testClassNotFound, checkClassNotFound)
{
    nbe::MemoryNodeLibrary sut;
    ASSERT_THROW(sut.instantiateNode(0,"NotFound", "notFound1"), std::runtime_error);
}

TEST(ValueTest_testIncrement, checkValueChanges)
{
    nbe::Value sut;
    sut += std::int64_t(1);
    EXPECT_EQ(1, std::int64_t(sut));
}

class NodeCategoryTest : public ::testing::TestWithParam<std::tuple<const char*, const char*, nbe::NodeCategory::Category>>
{

};

TEST_P(NodeCategoryTest, checkCategory)
{
    std::string className = std::get<0>(GetParam());
    std::string name = std::get<1>(GetParam());
    nbe::NodeCategory::Category category = std::get<2>(GetParam());
    nbe::MemoryNodeLibrary nodeLib;
    nbe::Node* actual = nodeLib.instantiateNode(0, className, name);
    ASSERT_NE(nullptr, actual);
    EXPECT_EQ(category, actual->category());
    delete actual;
}

INSTANTIATE_TEST_SUITE_P(NodeCategoryVerifyCategoryTest, NodeCategoryTest, ::testing::Values(
    std::make_tuple("Foo", "foo1", nbe::NodeCategory::CAT_SINK)
));

class PortTypeTest : public ::testing::TestWithParam<std::tuple<nbe::PortType::Type, double, double>>
{

};

TEST_P(PortTypeTest, checkSetValue)
{
    nbe::PortType::Type type = std::get<0>(GetParam());
    double value = std::get<1>(GetParam());
    double newValue = std::get<2>(GetParam());
    auto sut = new nbe::TypedPort<double>(0, "test1", nbe::PortType::TYPE_DOUBLE, nbe::PortDirection::DIR_IN, value);
    EXPECT_EQ(value, double(sut->value()));
    sut->setValue(newValue);
    EXPECT_EQ(newValue, double(sut->value()));
    delete sut;
}

INSTANTIATE_TEST_SUITE_P(PortTypeSetValueTest, PortTypeTest, ::testing::Values(
    std::make_tuple(nbe::PortType::TYPE_DOUBLE, 1.0, 2.0)
));

class CategoryToStringTest : public ::testing::TestWithParam<std::tuple<nbe::NodeCategory::Category, const char*>>
{

};

TEST_P(CategoryToStringTest, checkToString)
{
    nbe::NodeCategory::Category category = std::get<0>(GetParam());
    const char* categoryString = std::get<1>(GetParam());
    EXPECT_STREQ(categoryString, nbe::NodeCategory::toString(category));
}

INSTANTIATE_TEST_SUITE_P(CategoryEnumToStringTest, CategoryToStringTest, ::testing::Values(
    std::make_tuple(nbe::NodeCategory::CAT_NONE, "None"),
    std::make_tuple(nbe::NodeCategory::CAT_SOURCE, "Source"),
    std::make_tuple(nbe::NodeCategory::CAT_SINK, "Sink"),
    std::make_tuple(nbe::NodeCategory::CAT_CONDITION, "Condition"),
    std::make_tuple(nbe::NodeCategory::CAT_ACTION, "Action"),
    std::make_tuple(nbe::NodeCategory::CAT_GROUP, "Group")
));

class CategoryParseTest : public ::testing::TestWithParam<std::tuple<const char*, nbe::NodeCategory::Category>>
{

};

TEST_P(CategoryParseTest, checkParse)
{
    const char* categoryString = std::get<0>(GetParam());
    nbe::NodeCategory::Category category = std::get<1>(GetParam());

    EXPECT_EQ(category, nbe::NodeCategory::parse(categoryString));
}

INSTANTIATE_TEST_SUITE_P(CategoryStringToEnumTest, CategoryParseTest, ::testing::Values(
    std::make_tuple("None",nbe::NodeCategory::CAT_NONE),
    std::make_tuple("Source",nbe::NodeCategory::CAT_SOURCE),
    std::make_tuple("Sink",nbe::NodeCategory::CAT_SINK),
    std::make_tuple("Condition",nbe::NodeCategory::CAT_CONDITION),
    std::make_tuple("Action",nbe::NodeCategory::CAT_ACTION),
    std::make_tuple("Group",nbe::NodeCategory::CAT_GROUP),
    std::make_tuple("Spoo",nbe::NodeCategory::CAT_UNKNOWN)
));

class NodeTest_testClone : public ::testing::TestWithParam<std::tuple<const char*, const char*, size_t>>
{

};
TEST_P(NodeTest_testClone, checkClone)
{
    const char* className = std::get<0>(GetParam());
    const char* nodeName = std::get<1>(GetParam());
    size_t index = std::get<2>(GetParam());
    nbe::MemoryNodeLibrary nodeLib;
    nbe::Node* node = nodeLib.instantiateNode(0, className, nodeName);
    nbe::Node* sut = node->clone();
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
    nbe::MemoryNodeLibrary nodeLib;
    auto input = nodeLib.instantiateNode(0, "FooTyped", "foo1");
    auto output = nodeLib.instantiateNode(1, "BarTyped", "bar1");

    auto typedOutput = dynamic_cast<nbe::TypedPort<double>*>(output->dynamicPort(0));
    ASSERT_NE(nullptr, typedOutput);
    typedOutput->setValue(2.0);
    auto typedInput = dynamic_cast<nbe::TypedPort<double>*>(input->dynamicPort(0));
    ASSERT_NE(nullptr, typedInput);
    nbe::Transfer* transfer = typedOutput->connectTo(*typedInput);
    ASSERT_NE(nullptr, transfer);
    transfer->makeItSo();
    EXPECT_EQ(2.0, typedInput->value());
    delete transfer;
    delete output;
    delete input;
}

TEST(CreateTableTest, checkCreateTable)
{
    nbe::Lua lua;

    lua.eval("t={foo=true}");
    nbe::Table sut = lua.tableForName("t");
    const bool actual = sut.boolean("foo", false);
    ASSERT_EQ(true, actual);
}

TEST(LuaTest, checkExecuteNonExistentFile)
{
    nbe::Lua sut;
    
    sut.execute("NonExistent.lua");
    
    ASSERT_EQ("ScriptNotFound:description:The specified script was not found:\"NonExistent.lua\"",sut.errorMessage());
}

TEST(LuaTest, testNonExistentTable)
{
    nbe::Lua sut;

    sut.eval("t={}");
    ASSERT_FALSE(sut.tableExists("foo"));
}

TEST(LuaTest, testExistingTable)
{
    nbe::Lua sut;

    sut.eval("t={}");
    ASSERT_TRUE(sut.tableExists("t"));
}

TEST(TableTraversalTest, testSimple)
{
    nbe::Lua lua;
    lua.eval("t={foo=true}");
    nbe::Table t = lua.tableForName("t");
    nbe::TableTraversal trav(lua.get());
    bool foo = false;
    trav([&foo](lua_State* L)
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
    nbe::Lua lua;
    lua.eval("t={spoo={foo=true}}");
    ASSERT_TRUE(lua.ok());
    nbe::Table t = lua.tableForName("t");
    nbe::TableTraversal trav(lua.get());
    bool foo = false;
    int result = trav([&foo](lua_State* L)
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
    nbe::Lua lua;
    lua.eval("t={spoo={foo=true}}");
    ASSERT_TRUE(lua.ok());
    nbe::Table t = lua.tableForName("t");
    nbe::TableTraversal trav(lua.get());
    bool foo = false;
    int result = trav([&foo](lua_State* L)
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
    nbe::VariantPort* source = new nbe::VariantPort(0, "out1", nbe::PortType::TYPE_DOUBLE, nbe::PortDirection::DIR_OUT, 1.0);
    nbe::VariantPort* dest = new nbe::VariantPort(1, "in1", nbe::PortType::TYPE_INT, nbe::PortDirection::DIR_IN, std::int64_t{ 0 });
    nbe::Transfer * transfer = nullptr;
    ASSERT_NO_THROW(transfer = source->connectTo(*dest));
    ASSERT_EQ(nullptr, transfer);
    delete dest;
    delete source;
}

TEST(VariantPortTransferTest, testConnectToSameType)
{
    nbe::VariantPort* source = new nbe::VariantPort(0, "out1", nbe::PortType::TYPE_DOUBLE, nbe::PortDirection::DIR_OUT, 1.0);
    nbe::VariantPort* dest = new nbe::VariantPort(1, "in1", nbe::PortType::TYPE_DOUBLE, nbe::PortDirection::DIR_IN, 0.0);
    nbe::Transfer * transfer = source->connectTo(*dest);
    ASSERT_NE(nullptr, transfer);
    transfer->makeItSo();
    ASSERT_EQ(1.0, std::get<double>(dest->value()));
    delete transfer;
    delete dest;
    delete source;
}

TEST(TypedPortTransfer, testConnectToDifferentTypes)
{
    auto* source = new nbe::TypedPort(0, "out1", nbe::PortType::TYPE_INT, nbe::PortDirection::DIR_OUT, std::int64_t{ 1 });
    auto* dest = new nbe::TypedPort(1, "in1", nbe::PortType::TYPE_DOUBLE, nbe::PortDirection::DIR_IN, 0.0);
    nbe::Transfer const * transfer = source->connectTo(*dest);
    ASSERT_EQ(nullptr, transfer);
    delete transfer;
    delete dest;
    delete source;
}

TEST(TypedPortTransfer, testConnectToMatchingType)
{
    nbe::TypedPort<std::int64_t>* source = new nbe::TypedPort(0, "out1", nbe::PortType::TYPE_INT, nbe::PortDirection::DIR_OUT, std::int64_t{ 1 });
    nbe::TypedPort<std::int64_t>* dest = new nbe::TypedPort(1, "in1", nbe::PortType::TYPE_INT, nbe::PortDirection::DIR_IN, std::int64_t{ 0 });
    nbe::Transfer * transfer = source->connectTo(*dest);
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
    nbe::MemoryNodeLibrary nodeLib;
    nbe::Foo* foo = dynamic_cast<nbe::Foo*>(nodeLib.instantiateNode(0, "Foo", "foo1"));
    ASSERT_NE(nullptr, foo);
    nbe::NodeDescriptor sut;
    foo->describe(sut);
    ASSERT_EQ(nbe::NodeCategory::CAT_SINK, sut.category);
    ASSERT_EQ("foo1", sut.name);
    ASSERT_EQ(size_t{ 1 }, sut.ports.size());
    ASSERT_EQ("in1", sut.ports[0].name);
    ASSERT_EQ(nbe::PortType::TYPE_DOUBLE, sut.ports[0].type);
    ASSERT_EQ(nbe::PortDirection::DIR_IN, sut.ports[0].direction);
    delete foo;
}

TEST(GraphTest, testGraphInitiallyHasNoNodes)
{
    auto* sut = new nbe::Graph();

    ASSERT_EQ(size_t{ 0 }, sut->numNodes());
    delete sut;
}

TEST(GraphTest, testGraphInitiallyHasNoSignalPaths)
{
    auto* sut = new nbe::Graph();

    EXPECT_EQ(size_t{ 0 }, sut->numSignalPaths());

    delete sut;
}

TEST(GraphTest, testCannotAddANullNode)
{
    auto sut = new nbe::Graph();

    sut->addNode(nullptr);
    ASSERT_EQ(size_t{ 0 }, sut->numNodes());
    ASSERT_EQ(nullptr, sut->node(0));

    delete sut;
}

TEST(GraphTest, testWhenAddingANodeThenQueryReturnsIt)
{
    nbe::MemoryNodeLibrary nodeLib;
    auto* sut = new nbe::Graph();
    sut->setNodeLibrary(&nodeLib);
    auto const node = nodeLib.instantiateNode(sut->nextNodeID(), "Foo", "foo1");
    sut->addNode(node);
    ASSERT_EQ(size_t{ 1 }, sut->numNodes());
    ASSERT_EQ(node, sut->node(node->id()));

    delete sut;
}

TEST(GraphTest, testCannotAddANullSignalPath)
{
    auto const sut = new nbe::Graph();

    sut->addSignalPath(nullptr);
    ASSERT_EQ(size_t{ 0 }, sut->numSignalPaths());
    ASSERT_EQ(nullptr, sut->signalPath(0));
    
    delete sut;
}

TEST(GraphTest, testAfterAddingASignalPathCanQueryIt)
{
    nbe::MemoryNodeLibrary nodeLib;
    auto const sut = new nbe::Graph();
    sut->setNodeLibrary(&nodeLib);
    auto n1 = sut->createNode("BarTyped","out1");
    auto n2 = sut->createNode("FooTyped","in1");
    sut->addNode(n1);
    sut->addNode(n2);
    auto const path = new nbe::SignalPath(n1->dynamicPort(0), n2->dynamicPort(0));
    sut->addSignalPath(path);

    ASSERT_EQ(path, sut->signalPath(path->id()));

    delete sut;
}

TEST(PortTest, testCannotConnectTwoOutputs)
{
    nbe::MemoryNodeLibrary nodeLib;
    auto output1 = dynamic_cast<nbe::BarTyped*>(nodeLib.instantiateNode(0, "BarTyped", "bar1"));
    auto output2 = dynamic_cast<nbe::BarTyped*>(nodeLib.instantiateNode(1, "BarTyped", "bar2"));
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
    _str(0, "out1", nbe::PortType::TYPE_STRING, nbe::PortDirection::DIR_OUT, "test")
    {
	    // Do nothing.
    }

    nbe::TypedPort<std::string>& stringPort()
    {
        return _str;
    }
private:
    nbe::TypedPort<std::string> _str;
};

TEST(PortTest, testCannotConnectDifferentTypes)
{
    nbe::MemoryNodeLibrary nodeLib;
    auto output = new TestNodeWithStringPort();
    auto foo = dynamic_cast<nbe::FooTyped*>(nodeLib.instantiateNode(0, "FooTyped", "foo1"));
    auto t1 = output->stringPort().connectTo(foo->in1());
    ASSERT_EQ(size_t{ 0 }, output->stringPort().numOutgoingConnections());
    ASSERT_EQ(size_t{ 0 }, foo->in1().numIncomingConnections());
    delete t1;
    delete foo;
    delete output;
}

TEST(PortTestCompatibleTypes, testCompatibleTypesIntToDouble)
{
    auto output = new nbe::TypedPort(0, "output", nbe::PortType::TYPE_INT, nbe::PortDirection::DIR_OUT, std::int64_t{1});
    auto input = new nbe::TypedPort(1, "input", nbe::PortType::TYPE_DOUBLE, nbe::PortDirection::DIR_IN, 2.0);
    ASSERT_EQ(false, output->isCompatibleWith(*input));
    delete input;
    delete output;
}

TEST(PortTestCompatibleTypes, testCompatibleExactMatch)
{
    nbe::TypedPort<double> const* output = new nbe::TypedPort(0, "output", nbe::PortType::TYPE_DOUBLE, nbe::PortDirection::DIR_OUT, 1.0);
    auto* input = new nbe::TypedPort(1, "input", nbe::PortType::TYPE_DOUBLE, nbe::PortDirection::DIR_IN, 2.0);
    ASSERT_EQ(true, output->isCompatibleWith(*input));
    delete input;
    delete output;
}

TEST(PortTestCompatibleTypes, testCompatibleBoolToInt)
{
    auto const output = new nbe::TypedPort(0, "output", nbe::PortType::TYPE_BOOL, nbe::PortDirection::DIR_OUT, true);
    auto input = new nbe::TypedPort(1, "input", nbe::PortType::TYPE_INT, nbe::PortDirection::DIR_IN, std::int64_t{2});
    ASSERT_EQ(false, output->isCompatibleWith(*input));
    delete input;
    delete output;
}

TEST(PortTestCompatibleTypes, testCompatibleBoolToDouble)
{
    auto const * output = new nbe::TypedPort(0, "output", nbe::PortType::TYPE_BOOL, nbe::PortDirection::DIR_OUT, true);
    auto input = new nbe::TypedPort(1, "input", nbe::PortType::TYPE_DOUBLE, nbe::PortDirection::DIR_IN, 2.0);
    ASSERT_EQ(false, output->isCompatibleWith(*input));
    delete input;
    delete output;
}

TEST(PortTest, testDisconnectRemovesConnection)
{
    auto* output = new nbe::TypedPort(0, "output", nbe::PortType::TYPE_BOOL, nbe::PortDirection::DIR_OUT, true);
    auto* input = new nbe::TypedPort(1, "input", nbe::PortType::TYPE_DOUBLE, nbe::PortDirection::DIR_IN, 2.0);
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
    auto output = new nbe::TypedPort<std::int64_t>(0, "output", nbe::PortType::TYPE_INT, nbe::PortDirection::DIR_OUT, 1);
    auto* input = new nbe::TypedPort(1, "input", nbe::PortType::TYPE_DOUBLE, nbe::PortDirection::DIR_IN, 2.0);
    nbe::Transfer* transfer = output->connectTo(*input);
    ASSERT_EQ(nullptr, transfer);
    delete transfer;
    delete input;
    delete output;
}

TEST(PortTest, testEachOutgoingConnection)
{
    auto output = new nbe::TypedPort<std::int64_t>(0, "output", nbe::PortType::TYPE_DOUBLE, nbe::PortDirection::DIR_OUT, 1.0);
    auto* input = new nbe::TypedPort(1, "input", nbe::PortType::TYPE_DOUBLE, nbe::PortDirection::DIR_IN, 2.0);
    size_t outgoingCount = 0;
    size_t incomingCount = 0;
    auto t = output->connectTo(*input);
    output->eachOutgoingConnection([&outgoingCount](nbe::Port* item)
        {
            ++outgoingCount;
        });
    input->eachIncomingConnection([&incomingCount](nbe::Port* item)
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
    nbe::MemoryNodeLibrary nodeLib;

    auto const input = dynamic_cast<nbe::FooTyped*>(nodeLib.instantiateNode(0, "FooTyped", "foo1"));
    auto const output = dynamic_cast<nbe::BarTyped*>(nodeLib.instantiateNode(1, "BarTyped", "bar1"));

    auto t = output->out1()->connectTo(input->in1());
    ASSERT_EQ(size_t{ 1 }, input->in1().numIncomingConnections());
    delete output;
    ASSERT_EQ(size_t{ 0 }, input->in1().numIncomingConnections());
    delete t;
    delete input;
}

TEST(NodeTest, testDeleteAnInputNode)
{
    nbe::MemoryNodeLibrary nodeLib;

    auto const input = dynamic_cast<nbe::FooTyped*>(nodeLib.instantiateNode(0, "FooTyped", "foo1"));
    auto const output = dynamic_cast<nbe::BarTyped*>(nodeLib.instantiateNode(1, "BarTyped", "bar1"));

    auto t = output->out1()->connectTo(input->in1());
    ASSERT_EQ(size_t{ 1 }, output->out1()->numOutgoingConnections());
    delete input;
    ASSERT_EQ(size_t{ 0 }, output->out1()->numOutgoingConnections());
    delete output;
    delete t;
}

TEST(NodeTest, testDeleteInputOutputNode)
{
    nbe::MemoryNodeLibrary nodeLib;

    auto const input = dynamic_cast<nbe::FooTyped*>(nodeLib.instantiateNode(0, "FooTyped", "foo1"));
    auto const output = dynamic_cast<nbe::BarTyped*>(nodeLib.instantiateNode(1, "BarTyped", "bar1"));
    auto group = dynamic_cast<nbe::GroupTyped*>(nodeLib.instantiateNode(2, "GroupTyped", "group1"));
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
    ASSERT_NE(nullptr, nbe::Derived::metaPort(0));
	ASSERT_EQ("direction", nbe::Derived::metaPort(0)->name);
    ASSERT_NE(nullptr, nbe::Derived::metaPort(1));
    ASSERT_EQ("trigger", nbe::Derived::metaPort(1)->name);
}

TEST(NodeTest, testDynamicPortDescriptorReturnsStaticPortsInRange)
{
    nbe::MemoryNodeLibrary nodeLib;
    auto const sut = dynamic_cast<nbe::Final*>(nodeLib.instantiateNode(0, "Final", "final1"));
    ASSERT_NE(nullptr, sut);
    ASSERT_NE(nullptr, sut->dynamicMetaPort(0));
    ASSERT_EQ("direction", sut->dynamicMetaPort(0)->name);
    ASSERT_NE(nullptr, sut->dynamicMetaPort(1));
    ASSERT_EQ("trigger", sut->dynamicMetaPort(1)->name);
    delete sut;
}

TEST(NodeTest, testDynamicsPortDescriptorsForFinal)
{
    nbe::MemoryNodeLibrary nodeLib;
    auto const sut = dynamic_cast<nbe::Final*>(nodeLib.instantiateNode(0, "Final", "final1"));
    ASSERT_NE(nullptr, sut);
    sut->addDynamicPort(new nbe::TypedPort<double>(0, "output1", nbe::PortType::TYPE_DOUBLE, nbe::PortDirection::DIR_OUT, 1.0));
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
    nbe::MemoryNodeLibrary nodeLib;
    auto const sut = nodeLib.instantiateNode(0, className, "node1");
    ASSERT_NE(nullptr, sut);
    sut->addDynamicPort(new nbe::TypedPort<double>(0, "output1", nbe::PortType::TYPE_DOUBLE, nbe::PortDirection::DIR_OUT, 1.0));
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

class NodeTestDynamicPortDescriptorForNode : public ::testing::TestWithParam<std::tuple<const char*, size_t, const char*, nbe::PortType::Type, nbe::PortDirection::Direction>>
{
};

TEST_P(NodeTestDynamicPortDescriptorForNode, testDynamicPortDescriptor)
{
    std::string const className = std::get<0>(GetParam());
    size_t const index = std::get<1>(GetParam());
    std::string const nodeName = std::get<2>(GetParam());
    nbe::PortType::Type type = std::get<3>(GetParam());
    nbe::PortDirection::Direction dir = std::get<4>(GetParam());
    nbe::MemoryNodeLibrary nodeLib;
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
    std::make_tuple("Foo", 0, "in1", nbe::PortType::TYPE_DOUBLE, nbe::PortDirection::DIR_IN),
    std::make_tuple("Bar", 0, "out1", nbe::PortType::TYPE_DOUBLE, nbe::PortDirection::DIR_OUT),
    std::make_tuple("Base", 0, "direction", nbe::PortType::TYPE_DOUBLE, nbe::PortDirection::DIR_OUT),
    std::make_tuple("Derived", 1, "trigger", nbe::PortType::TYPE_BOOL, nbe::PortDirection::DIR_IN),
    std::make_tuple("Final", 0, "direction", nbe::PortType::TYPE_DOUBLE, nbe::PortDirection::DIR_OUT),
    std::make_tuple("Final", 1, "trigger", nbe::PortType::TYPE_BOOL, nbe::PortDirection::DIR_IN),
    std::make_tuple("FooTyped", 0, "in1", nbe::PortType::TYPE_DOUBLE, nbe::PortDirection::DIR_IN),
    std::make_tuple("BarTyped", 0, "out1", nbe::PortType::TYPE_DOUBLE, nbe::PortDirection::DIR_OUT),
    std::make_tuple("GroupTyped", 0, "out1", nbe::PortType::TYPE_DOUBLE, nbe::PortDirection::DIR_OUT),
    std::make_tuple("GroupTyped", 1, "in1", nbe::PortType::TYPE_DOUBLE, nbe::PortDirection::DIR_IN)
));

TEST(GraphTest, testTopologicalSortEmptyGraphReturnsEmptyArray)
{
    auto sut = new nbe::Graph();

    nbe::NodeArray actual;
    sut->topologicalSort(&actual);

    ASSERT_TRUE(actual.empty());

    delete sut;
}

bool before(const nbe::NodeArray& a, nbe::Node* first, nbe::Node* last)
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
    nbe::MemoryNodeLibrary nodeLib;
    auto sut = new nbe::Graph();
    sut->setNodeLibrary(&nodeLib);
    auto a = dynamic_cast<nbe::FooTyped*>(sut->createNode("FooTyped", "foo1"));
    auto b = dynamic_cast<nbe::BarTyped*>(sut->createNode("BarTyped", "bar1"));
    auto t = b->out1()->connectTo(a->in1());
    sut->addNode(a);
    sut->addPort(&a->in1());
    auto path = new nbe::SignalPath(b->out1(), &a->in1());
/*    path->removed = false;
    path->source.node = b->id();
    path->source.port = b->out1()->id();
    path->dest.node = a->id();
    path->dest.port = a->in1().id();*/
    sut->addSignalPath(path);
    sut->addNode(b);
    sut->addPort(b->out1());
    nbe::NodeArray actual;
    auto result = sut->topologicalSort(&actual);
    ASSERT_EQ(nbe::Graph::TopoSortResult::OK, result);
    ASSERT_EQ(size_t{2}, actual.size());
    ASSERT_TRUE(before(actual, b, a));

    delete t;
    delete sut;
}

TEST(GraphTest, testTopologicalSortTransitiveDependency)
{
    nbe::MemoryNodeLibrary nodeLib;
    auto sut = new nbe::Graph();
    sut->setNodeLibrary(&nodeLib);
    auto a = dynamic_cast<nbe::GroupTyped*>(sut->createNode("GroupTyped", "foo1"));
    auto b = dynamic_cast<nbe::GroupTyped*>(sut->createNode("GroupTyped", "bar1"));
    auto c = dynamic_cast<nbe::GroupTyped*>(sut->createNode("GroupTyped", "baz1"));
    nbe::Transfer* t1 = nullptr;
    nbe::Transfer* t2 = nullptr;

    {
        t1 = a->out1().connectTo(b->in1());
        sut->addNode(a);
        sut->addPort(&a->in1());
        sut->addPort(&a->out1());
        auto path = new nbe::SignalPath(&a->out1(), &b->in1());
        sut->addSignalPath(path);
    }
    {
        t2 = b->out1().connectTo(c->in1());
        sut->addNode(b);
        sut->addPort(&b->in1());
        sut->addPort(&b->out1());
        auto path = new nbe::SignalPath(&b->out1(), &c->in1());
        sut->addSignalPath(path);
    }
    {
        sut->addNode(c);
        sut->addPort(&c->in1());
        sut->addPort(&c->out1());
    }
    nbe::NodeArray actual;
    auto result = sut->topologicalSort(&actual);
    ASSERT_EQ(nbe::Graph::TopoSortResult::OK, result);
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
    nbe::MemoryNodeLibrary nodeLib;
    auto sut = new nbe::Graph();
    sut->setNodeLibrary(&nodeLib);
    auto a = dynamic_cast<nbe::GroupTyped*>(sut->createNode("GroupTyped", "foo1"));
    auto b = dynamic_cast<nbe::GroupTyped*>(sut->createNode("GroupTyped", "bar1"));
    auto t1 = b->out1().connectTo(a->in1());
    sut->addNode(a);
    sut->addPort(&a->in1());
    sut->addPort(&a->out1());
    auto path = new nbe::SignalPath(&b->out1(), &a->in1());
    sut->addSignalPath(path);
    auto t2 = a->out1().connectTo(b->in1());
    sut->addNode(b);
    sut->addPort(&b->out1());
    sut->addPort(&b->in1());
    path = new nbe::SignalPath(&a->out1(), &b->in1());
    sut->addSignalPath(path);
    nbe::NodeArray actual;
    auto result = sut->topologicalSort(&actual);
    ASSERT_EQ(nbe::Graph::TopoSortResult::CYCLES_DETECTED, result);
    ASSERT_EQ(size_t{0}, actual.size());

    delete t2;
    delete t1;
    delete sut;
}

class TopologicalSort_testPersistent : public ::testing::TestWithParam<std::tuple<const char*, nbe::Graph::TopoSortResult, std::size_t, const char*, const char*>>
{

};

TEST_P(TopologicalSort_testPersistent, testSort)
{
    const char* graphFilename = std::get<0>(GetParam());
    nbe::Graph::TopoSortResult result = std::get<1>(GetParam());
    std::size_t numNodesInResult = std::get<2>(GetParam());
    std::string lhsPath = std::get<3>(GetParam());
    std::string rhsPath = std::get<4>(GetParam());

    nbe::MemoryNodeLibrary nodeLib;
    auto sut = nbe::Graph::fromFile(nodeLib, graphFilename);
    ASSERT_NE(nullptr, sut);
    nbe::NodeArray order;
    nbe::Graph::TopoSortResult actualResult = sut->topologicalSort(&order);
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
}

INSTANTIATE_TEST_SUITE_P(TopologicalSort, TopologicalSort_testPersistent, ::testing::Values(
        std::make_tuple("etc/tests/Graph/empty.lua", nbe::Graph::OK, std::size_t{0}, "", ""),
        std::make_tuple("etc/tests/Graph/onenode.lua", nbe::Graph::OK, std::size_t{1}, "", ""),
        std::make_tuple("etc/tests/Graph/connectednodes.lua", nbe::Graph::OK, std::size_t{2}, "bar1", "foo1"),
        std::make_tuple("etc/tests/Graph/withchildgraph.lua", nbe::Graph::OK, std::size_t{2}, "", ""),
        std::make_tuple("etc/tests/Graph/withnestedchildgraph.lua", nbe::Graph::OK, std::size_t{3}, "", ""),
        std::make_tuple("etc/tests/Graph/connectednestedchildgraph.lua", nbe::Graph::OK, std::size_t{3}, "child[0].bar1", "child[0].child[0].bound1")
        ));

class Graph_testFindAllNodes : public ::testing::TestWithParam<std::tuple<const char*, std::size_t>>
{

};

TEST_P(Graph_testFindAllNodes, testFindAllNodes)
{
    auto graphFilename = std::get<0>(GetParam());
    std::size_t numNodes = std::get<1>(GetParam());
    nbe::MemoryNodeLibrary nodeLib;
    auto sut = nbe::Graph::fromFile(nodeLib, graphFilename);
    ASSERT_NE(nullptr, sut);
    nbe::NodeArray actual;
    sut->findAllNodes(&actual);
    EXPECT_EQ(numNodes, actual.size());
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
    nbe::MemoryNodeLibrary nodeLib;
    auto sut = nbe::Graph::fromFile(nodeLib,graphFilename);
    ASSERT_NE(nullptr, sut);
    auto actual = sut->findNode(path);
    ASSERT_NE(nullptr, actual);
    EXPECT_EQ(nodeName, actual->name());
}

INSTANTIATE_TEST_SUITE_P(Graph, Graph_testFindNode, ::testing::Values(
        std::make_tuple("etc/tests/Graph/onenode.lua", "foo1", "foo1"),
        std::make_tuple("etc/tests/Graph/withchildgraph.lua", "child[0].bar1", "bar1"),
        std::make_tuple("etc/tests/Graph/withnestedchildgraph.lua", "child[0].child[0].bound1", "bound1")
        ));

TEST(PortTest, testConnectToExistingPortGivesTransfer)
{
    auto source = new nbe::TypedPort<double>(0, nullptr, new nbe::MetaPort("out", nbe::PortType::TYPE_DOUBLE, nbe::PortDirection::DIR_OUT), 1.0, std::int32_t (nbe::Port::OWN_META_PORT_BIT));
    auto dest = new nbe::TypedPort<double>(1, nullptr, new nbe::MetaPort("in", nbe::PortType::TYPE_DOUBLE, nbe::PortDirection::DIR_IN), 0.0, nbe::Port::OWN_META_PORT_BIT);
    auto* visitor = new nbe::SetValueVisitor(nbe::Value(2.0));
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
    auto source = new nbe::TypedPort<double>(0, nullptr, new nbe::MetaPort("out", nbe::PortType::TYPE_DOUBLE, nbe::PortDirection::DIR_OUT), 1.0, nbe::Port::OWN_META_PORT_BIT);
    auto dest = new nbe::TypedPort<double>(1, nullptr, new nbe::MetaPort("in", nbe::PortType::TYPE_DOUBLE, nbe::PortDirection::DIR_IN), 0.0, nbe::Port::OWN_META_PORT_BIT);
    auto transfer = source->connectTo(*dest);
    ASSERT_NE(nullptr, transfer);
    source->disconnect(*dest);
    ASSERT_FALSE(source->isConnectedTo(dest));
    ASSERT_FALSE(dest->isConnectedTo(source));
    auto visitor = new nbe::ValueVisitor();
    dest->accept(*visitor);
    ASSERT_EQ(0.0, double(visitor->value()));
    delete visitor;
    delete transfer;
    delete dest;
    delete source;
}

TEST(SelectionLiveTest, testAdd)
{
    auto sut = new nbe::SelectionLive();
    nbe::SelectionInterface::Cont a;
    nbe::MemoryNodeLibrary nodeLib;
    auto node = nodeLib.instantiateNode(0, "FooTyped", "foo1");
    a.insert(node);
    sut->add(a.begin(), a.end());
    ASSERT_EQ(size_t{1}, sut->count());
    delete node;
    delete sut;
}

TEST(SelectionLiveTest, testSubtract)
{
    auto sut = new nbe::SelectionLive();
    nbe::SelectionInterface::Cont a;
    nbe::MemoryNodeLibrary nodeLib;
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
    auto sut = new nbe::SelectionLive();
    nbe::SelectionInterface::Cont a;
    nbe::MemoryNodeLibrary nodeLib;
    auto node = nodeLib.instantiateNode(0, "FooTyped", "foo1");
    a.insert(node);
    sut->add(a.begin(), a.end());
    nbe::SelectionInterface::Cont b;
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
    auto sut = new nbe::SelectionLive();
    nbe::SelectionInterface::Cont a;
    nbe::MemoryNodeLibrary nodeLib;
    auto node = nodeLib.instantiateNode(0, "FooTyped", "foo1");
    auto node2 = nodeLib.instantiateNode(1, "BarTyped", "bar1");
    a.insert(node);
    a.insert(node2);
    sut->add(a.begin(), a.end());
    ASSERT_EQ(size_t{2}, sut->count());
    nbe::SelectionInterface::Cont b;
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
    auto sut = new nbe::NodeEditorLive();
    auto actual = sut->createNode("FooTyped", "foo1");
    ASSERT_EQ(nbe::Status::StatusCode::STATUS_OK, actual.status);
    ASSERT_EQ(nbe::Status::RESULT_NODE, actual.resultType);
    ASSERT_NE(nullptr, actual.result.node);

    delete sut;
}

TEST(NodeEditorLiveTest, testConnectionBetweenExistingNodesSucceeds)
{
    auto sut = new nbe::NodeEditorLive();
    auto s1 = sut->createNode("FooTyped", "foo1");
    ASSERT_EQ(nbe::Status::RESULT_NODE, s1.resultType);
    auto s2 = sut->createNode("BarTyped", "bar1");
    ASSERT_EQ(nbe::Status::RESULT_NODE, s2.resultType);

    auto actual = sut->connect(s2.result.node->dynamicPort(0)->id(), s1.result.node->dynamicPort(0)->id());
    ASSERT_EQ(nbe::Status::StatusCode::STATUS_OK, actual.status);
    ASSERT_EQ(nbe::Status::RESULT_SIGNAL_PATH_ID, actual.resultType);
    ASSERT_TRUE(actual.result.signalPathId.valid());

    delete sut;
}

TEST(NodeEditorLiveTest, testConnectionBetweenInputAndOutputFails)
{
    auto sut = new nbe::NodeEditorLive();
    auto s1 = sut->createNode("FooTyped", "foo1");
    ASSERT_EQ(nbe::Status::RESULT_NODE, s1.resultType);
    auto s2 = sut->createNode("BarTyped", "bar1");
    ASSERT_EQ(nbe::Status::RESULT_NODE, s2.resultType);

    auto actual = sut->connect(s1.result.node->dynamicPort(0)->id(), s2.result.node->dynamicPort(0)->id());
    ASSERT_EQ(nbe::Status::StatusCode::STATUS_INVALID_PORT, actual.status);
    ASSERT_EQ(nbe::Status::RESULT_PORT, actual.resultType);
    ASSERT_NE(nullptr, actual.result.port);
    ASSERT_EQ(s1.result.node->dynamicPort(0)->id(), actual.result.port->id());

    delete sut;
}

TEST(NodeEditorLiveTest, testConnectionBetweenNonExistentFromPortFails)
{
    auto sut = new nbe::NodeEditorLive();

    auto actual = sut->connect(nbe::PortID{0}, nbe::PortID{1});
    ASSERT_EQ(nbe::Status::StatusCode::STATUS_OBJECT_NOT_FOUND, actual.status);
    ASSERT_EQ(nbe::Status::RESULT_PORT_ID, actual.resultType);
    ASSERT_EQ(0, actual.result.portId);

    delete sut;
}

TEST(NodeEditorLiveTest, testConnectionBetweenNonExistentToPortFails)
{
    auto sut = new nbe::NodeEditorLive();
    auto s1 = sut->createNode("BarTyped", "bar1");
    ASSERT_EQ(nbe::Status::STATUS_OK, s1.status);
    auto actual = sut->connect(s1.result.node->dynamicPort(0)->id(), nbe::PortID{1});
    ASSERT_EQ(nbe::Status::StatusCode::STATUS_OBJECT_NOT_FOUND, actual.status);
    ASSERT_EQ(nbe::Status::RESULT_PORT_ID, actual.resultType);
    ASSERT_EQ(1, actual.result.portId);

    delete sut;
}

TEST(NodeEditorLiveTest, testCreateChildWithAnEmptySelectionFails)
{
    auto sut = new nbe::NodeEditorLive();

    auto actual = sut->createChild();
    ASSERT_EQ(nbe::Status::STATUS_INVALID_SELECTION, actual.status);
    ASSERT_EQ(nbe::Status::RESULT_NONE, actual.resultType);

    delete sut;
}

TEST(NodeEditorLiveTest, testSelectAll)
{
    auto sut = new nbe::NodeEditorLive();
    auto s1 = sut->createNode("GroupTyped", "group1");
    ASSERT_EQ(nbe::Status::STATUS_OK, s1.status);
    ASSERT_EQ(nbe::Status::RESULT_NODE, s1.resultType);
    ASSERT_NE(nullptr, s1.result.node);
    sut->selectAll();
    ASSERT_EQ(size_t{1}, sut->selectionCount());
    delete sut;
}

TEST(NodeEditorLiveTest, testCreateChildWithSingleChildSucceeds)
{
    auto sut = new nbe::NodeEditorLive();
    auto s1 = sut->createNode("GroupTyped", "group1");
    auto s2 = sut->createNode("FooTyped", "foo1");
    auto s3 = sut->createNode("BarTyped", "bar1");
    auto t1 = s3.result.node->dynamicPort(0)->connectTo(*s1.result.node->dynamicPort(1));
    auto t2 = s1.result.node->dynamicPort(0)->connectTo(*s2.result.node->dynamicPort(0));
    ASSERT_EQ(nbe::Status::STATUS_OK, s1.status);
    ASSERT_EQ(nbe::Status::RESULT_NODE, s1.resultType);
    ASSERT_NE(nullptr, s1.result.node);
    nbe::SelectionInterface::Cont c;
    c.insert(s1.result.node);
    auto s4 = sut->select(nbe::NodeEditorInterface::SELECTION_SET, c);
    ASSERT_EQ(size_t{1}, sut->selectionCount());
    auto actual = sut->createChild();
    ASSERT_EQ(nbe::Status::STATUS_OK, actual.status);
    ASSERT_EQ(nbe::Status::RESULT_GRAPH, actual.resultType);
    ASSERT_NE(nullptr, actual.result.graph);
    ASSERT_EQ(size_t{3}, actual.result.graph->numNodes());
    ASSERT_NE(nullptr, actual.result.graph->parent());
    delete t2;
    delete t1;
    delete sut;
}

void createNode(nbe::NodeEditorInterface* sut, const char* className, const char* nodeName, nbe::Node** node)
{
    auto s1 = sut->createNode(className, nodeName);
    ASSERT_EQ(nbe::Status::STATUS_OK, s1.status);
    ASSERT_EQ(nbe::Status::ResultType::RESULT_NODE, s1.resultType);
    ASSERT_NE(nullptr, s1.result.node);
    if (node != nullptr)
    {
        *node = s1.result.node;
    }
}

template <typename T>
void createPort(nbe::PortID id, nbe::Node* parent, const char* name, nbe::PortType::Type type, nbe::PortDirection::Direction dir, T value)
{
    auto port = new nbe::TypedPort<T>(id, parent, new nbe::MetaPort(name, type, dir), value, nbe::Port::OWN_META_PORT_BIT);
    parent->addDynamicPort(port);
}

TEST(NodeEditorLiveTest, testCreateChildWithMultipleChildrenSucceeds)
{
    auto sut = new nbe::NodeEditorLive();

    nbe::Node* multi1 = nullptr;
    createNode(sut,"Final", "multi1", &multi1);
    createPort(0, multi1, "out1", nbe::PortType::TYPE_DOUBLE, nbe::PortDirection::DIR_OUT, 1.0);
    createPort(1, multi1, "in1", nbe::PortType::TYPE_DOUBLE, nbe::PortDirection::DIR_IN, 2.0);
    nbe::Node* multi2 = nullptr;
    createNode(sut,"Final", "multi2", &multi2);
    createPort(2, multi2, "in1", nbe::PortType::TYPE_DOUBLE, nbe::PortDirection::DIR_IN, 2.0);

    createPort(4, multi2, "out1", nbe::PortType::TYPE_DOUBLE, nbe::PortDirection::DIR_OUT, 3.0);
    nbe::Node* output1 = nullptr;
    createNode(sut, "BarTyped", "output1",&output1);
    auto t1 = output1->dynamicPort(0)->connectTo(*multi1->dynamicPort(4));
    auto t2 = multi1->dynamicPort(3)->connectTo(*multi2->dynamicPort(3));
    nbe::Node* input1 = nullptr;
    createNode(sut, "FooTyped", "input1",&input1);
    auto t3 = multi2->dynamicPort(4)->connectTo(*input1->dynamicPort(0));

    nbe::NodeSet selection;
    selection.insert(multi1);
    selection.insert(multi2);
    sut->select(nbe::NodeEditorInterface::SELECTION_SET, selection);
    auto s = sut->createChild();
    ASSERT_EQ(nbe::Status::STATUS_OK, s.status);
    ASSERT_EQ(nbe::Status::RESULT_GRAPH, s.resultType);
    ASSERT_NE(nullptr, s.result.graph);
    ASSERT_EQ(size_t{4}, s.result.graph->numNodes());

    delete t3;
    delete t2;
    delete t1;
    delete sut;
}

TEST(SelectionLiveTest, testComputeBoundaryOfEmptySetGivesEmptyOutputs)
{
    auto sut = new nbe::SelectionLive();
    nbe::NodeArray inputs, outputs, internals;
    sut->computeBoundaryNodes(&inputs, &outputs, &internals);
    ASSERT_TRUE(inputs.empty());
    ASSERT_TRUE(outputs.empty());
    delete sut;
}

TEST(SelectionLiveTest, testComputeBoundaryOnNodeWithInputsAndOutputsGivesTheNode)
{
    auto graph = new nbe::Graph();
    nbe::MemoryNodeLibrary nodeLib;
    auto sut = new nbe::SelectionLive();
    auto g1 = dynamic_cast<nbe::GroupTyped*>(nodeLib.instantiateNode(0, "GroupTyped", "group1"));
    auto f1 = dynamic_cast<nbe::FooTyped*>(nodeLib.instantiateNode(1, "FooTyped", "foo1"));
    auto b1 = dynamic_cast<nbe::BarTyped*>(nodeLib.instantiateNode(2, "BarTyped", "bar1"));
    ASSERT_NE(nullptr, g1);
    auto t1 = b1->out1()->connectTo(g1->in1());
    auto t2 = g1->out1().connectTo(f1->in1());
    sut->add(g1);
    nbe::NodeArray inputs, outputs, internals;
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
    nbe::MemoryNodeLibrary nodeLib;
    auto sut = new nbe::Boundary(nodeLib, "sut", nbe::NodeCategory::CAT_SOURCE);
    auto metaPort = new nbe::MetaPort("input1", nbe::PortType::TYPE_DOUBLE, nbe::PortDirection::DIR_IN);
    auto input = new nbe::TypedPort<double>(0, nullptr, metaPort, 1.0, nbe::Port::OWN_META_PORT_BIT);
    ASSERT_NO_THROW(sut->addDynamicPort(input));
    ASSERT_EQ(sut, input->parent());
    ASSERT_EQ(size_t{1}, sut->totalPorts());
    ASSERT_EQ(input, sut->dynamicPort(0));
    // input now owned by sut
    delete sut;
}

TEST(BoundaryNode, testClone)
{
    nbe::MemoryNodeLibrary nodeLib;
    auto sut = new nbe::Boundary(nodeLib, "sut", nbe::NodeCategory::CAT_SOURCE);
    auto metaPort = new nbe::MetaPort("input1", nbe::PortType::TYPE_DOUBLE, nbe::PortDirection::DIR_IN);
    auto input = new nbe::TypedPort<double>(0, nullptr, metaPort, 1.0, nbe::Port::OWN_META_PORT_BIT);
    ASSERT_NO_THROW(sut->addDynamicPort(input));
    auto clone = sut->clone();
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
    nbe::MemoryNodeLibrary nodeLib;

    auto newInput = new nbe::Boundary(nodeLib, "b1", nbe::NodeCategory::CAT_SOURCE);
    auto oldInput = new nbe::FooTyped(nodeLib, "foo1", nbe::NodeCategory::CAT_SINK);
    auto output = new nbe::BarTyped(nodeLib, "bar1", nbe::NodeCategory::CAT_SOURCE);
    auto transfer = output->out1()->connectTo(oldInput->in1());
    nbe::NodeSet selection;
    selection.insert(output);
    output->reconnectOutputs(selection, newInput);
    ASSERT_EQ(size_t{2}, newInput->totalPorts());
    ASSERT_EQ(size_t{1}, oldInput->dynamicPort(0)->numIncomingConnections());
    ASSERT_EQ(nbe::PortDirection::DIR_OUT, newInput->dynamicPort(1)->dir());
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
    nbe::MemoryNodeLibrary nodeLib;

    auto newInput = new nbe::Boundary(nodeLib, "b1", nbe::NodeCategory::CAT_SOURCE);
    auto oldInput = new nbe::FooTyped(nodeLib, "foo1", nbe::NodeCategory::CAT_SINK);
    auto output = new nbe::BarTyped(nodeLib, "bar1", nbe::NodeCategory::CAT_SOURCE);
    auto s1 = new nbe::FooTyped(nodeLib, "foo2", nbe::NodeCategory::CAT_SINK);
    auto t1 = output->out1()->connectTo(oldInput->in1());
    auto t2 = output->out1()->connectTo(s1->in1());
    nbe::NodeSet selection;
    selection.insert(output);
    output->reconnectOutputs(selection, newInput);
    ASSERT_EQ(size_t{4}, newInput->totalPorts());
    ASSERT_EQ(size_t{1}, oldInput->dynamicPort(0)->numIncomingConnections());
    ASSERT_EQ(nbe::PortDirection::DIR_OUT, newInput->dynamicPort(1)->dir());
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
    nbe::MemoryNodeLibrary nodeLib;

    auto newOutput = new nbe::Boundary(nodeLib, "b1", nbe::NodeCategory::CAT_SOURCE);
    auto oldOutput = new nbe::BarTyped(nodeLib, "bar1", nbe::NodeCategory::CAT_SOURCE);
    auto input = new nbe::FooTyped(nodeLib, "foo1", nbe::NodeCategory::CAT_SINK);

    auto t = oldOutput->out1()->connectTo(input->in1());
    nbe::NodeSet selection;
    selection.insert(input);
    input->reconnectInputs(selection, newOutput);
    ASSERT_EQ(size_t{ 2 }, newOutput->totalPorts());
    ASSERT_EQ(nbe::PortDirection::DIR_IN, newOutput->dynamicPort(1)->dir());
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
    nbe::MemoryNodeLibrary nodeLib;

    auto newOutput = new nbe::Boundary(nodeLib, "b1", nbe::NodeCategory::CAT_SOURCE);
    auto oldOutput = new nbe::BarTyped(nodeLib, "bar1", nbe::NodeCategory::CAT_SOURCE);
    auto input = new nbe::Final(nodeLib, "foo1", nbe::NodeCategory::CAT_SINK);
    auto s1 = new nbe::BarTyped(nodeLib, "bar2", nbe::NodeCategory::CAT_SOURCE);
    auto mp1 = new nbe::MetaPort("in1", nbe::PortType::TYPE_DOUBLE, nbe::PortDirection::DIR_IN);
    auto p1 = new nbe::TypedPort<double>(0, nullptr, mp1, 1.0);
    input->addDynamicPort(p1);
    auto t1 = oldOutput->out1()->connectTo(*input->dynamicPort(3));
    auto mp2 = new nbe::MetaPort("in2", nbe::PortType::TYPE_DOUBLE, nbe::PortDirection::DIR_IN);
    auto p2 = new nbe::TypedPort<double>(1, nullptr, mp2, 2.0);
    input->addDynamicPort(p2);
    auto t2 = s1->out1()->connectTo(*input->dynamicPort(4));

    nbe::NodeSet selection;
    selection.insert(input);
    input->reconnectInputs(selection, newOutput);
    ASSERT_EQ(size_t{ 4 }, newOutput->totalPorts());
    ASSERT_EQ(nbe::PortDirection::DIR_IN, newOutput->dynamicPort(1)->dir());
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
    auto sut = new nbe::NodeEditorLive();

    auto s = sut->createNode("Final", "test1");
    sut->selectAll();
    ASSERT_EQ(size_t{1}, sut->selectionCount());
    auto s2 = sut->selectNone();
    ASSERT_EQ(nbe::Status::STATUS_OK, s2.status);
    ASSERT_EQ(nbe::Status::RESULT_NONE, s2.resultType);
    ASSERT_EQ(size_t{0}, sut->selectionCount());

    delete sut;
}

TEST(NodeEditorLiveTest, testDisconnectNonExistentConnectionFails)
{
    auto sut = new nbe::NodeEditorLive();

    auto s1 = sut->disconnect(0);
    ASSERT_EQ(nbe::Status::STATUS_OBJECT_NOT_FOUND, s1.status);
    ASSERT_EQ(nbe::Status::RESULT_SIGNAL_PATH_ID, s1.resultType);
    ASSERT_EQ(0,s1.result.signalPathId);

    delete sut;
}

TEST(NodeEditorLiveTest, testDisconnectExistingConnectionSucceeds)
{
    auto sut = new nbe::NodeEditorLive();
    auto s1 = sut->createNode("BarTyped", "bar1");
    auto s2 = sut->createNode("FooTyped", "foo1");
    auto s3 = sut->connect(s1.result.node->dynamicPort(0)->id(), s2.result.node->dynamicPort(0)->id());
    ASSERT_EQ(nbe::Status::STATUS_OK, s3.status);
    ASSERT_EQ(nbe::Status::RESULT_SIGNAL_PATH_ID, s3.resultType);
    auto s4 = sut->disconnect(s3.result.signalPathId);
    ASSERT_EQ(nbe::Status::STATUS_OK, s4.status);
    ASSERT_EQ(nbe::Status::RESULT_NONE, s4.resultType);
    ASSERT_EQ(size_t{0}, s2.result.node->dynamicPort(0)->numIncomingConnections());
    ASSERT_EQ(size_t{0}, s1.result.node->dynamicPort(0)->numOutgoingConnections());
    auto s5 = sut->disconnect(s3.result.signalPathId);
    ASSERT_EQ(nbe::Status::STATUS_OBJECT_NOT_FOUND, s5.status);
    ASSERT_EQ(nbe::Status::RESULT_SIGNAL_PATH_ID, s5.resultType);
    ASSERT_EQ(s3.result.signalPathId, s5.result.signalPathId);

    delete sut;
}

TEST(NodeEditorLiveTest, testDeleteNonExistentNodeFails)
{
    auto sut = new nbe::NodeEditorLive();

    auto status = sut->deleteNode(0);
    ASSERT_EQ(nbe::Status::STATUS_OBJECT_NOT_FOUND, status.status);
    ASSERT_EQ(nbe::Status::RESULT_NODE_ID, status.resultType);
    ASSERT_EQ(0, status.result.nodeId);

    delete sut;
}

TEST(NodeEditorLiveTest, testDeleteExistingNodeSucceeds)
{
    auto sut = new nbe::NodeEditorLive();
    auto s1 = sut->createNode("FooTyped", "test1");
    auto id = s1.result.node->id();
    auto status = sut->deleteNode(s1.result.node->id());
    ASSERT_EQ(nbe::Status::STATUS_OK, status.status);
    ASSERT_EQ(nbe::Status::RESULT_NODE, status.resultType);
    ASSERT_NE(nullptr, status.result.node);
    auto s2 = sut->deleteNode(id);
    ASSERT_EQ(nbe::Status::STATUS_OBJECT_NOT_FOUND, s2.status);
    ASSERT_EQ(nbe::Status::RESULT_NODE_ID, s2.resultType);
    ASSERT_EQ(id, s2.result.nodeId);

    delete sut;
}

TEST(GraphTest, testCreateNode)
{
    auto sut = new nbe::Graph();
    auto nodeLib = new nbe::MemoryNodeLibrary();
    sut->setNodeLibrary(nodeLib);
    auto node = sut->createNode("FooTyped", "foo1");
    ASSERT_NE(nullptr, node);
    sut->addNode(node);
    delete nodeLib;
    delete sut;
}

TEST(CommandTest, testCreateNode)
{
    auto graph = new nbe::Graph();
    auto nodeLib = new nbe::MemoryNodeLibrary();
    graph->setNodeLibrary(nodeLib);
    auto sut = new nbe::CreateNode();
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
    auto sut = new nbe::Graph();
    auto nodeLib = new nbe::MemoryNodeLibrary();
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
    nbe::ByteBuffer sut;

    sut.put(101.234f);
    float f = 0.0f;
    sut.get(&f);
    ASSERT_EQ(101.234f, f);
}

TEST(ByteBufferTest, testRelativeInt)
{
    nbe::ByteBuffer sut;

    sut.put(101);
    std::int32_t f = 0;
    sut.get(&f);
    ASSERT_EQ(101, f);
}

TEST(ByteBufferTest, testRelativeDouble)
{
    nbe::ByteBuffer sut;

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
    nbe::ByteBuffer sut;

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
    nbe::ByteBuffer buf;
    nbe::MemoryOutputStream sut(&buf);
    float f = 123.456f;
    sut.writeBuf(reinterpret_cast<nbe::MemoryOutputStream::value_type*>(&f), sizeof(float));
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

    explicit TestObj(nbe::InputStream& str)
    {
        str.addObj(this);

        str.read(&_i);
    }

    [[nodiscard]]int i() const
    {
        return _i;
    }

    nbe::OutputStream& write(nbe::OutputStream& str) const
    {
        str.write(_i);

        return str;
    }
private:
    int _i{0};
};

TEST(MemoryOutputStreamTest, testWriteRef)
{
    nbe::ByteBuffer buf;
    nbe::MemoryOutputStream sut(&buf);
    TestObj obj(1);
    sut.writeRef(&obj);
    std::uint32_t id;
    buf.get(&id);
    ASSERT_EQ(id, 1);
}

TEST(MemoryOutputStreamTest, testWrite)
{
    nbe::ByteBuffer buf;
    nbe::MemoryOutputStream sut(&buf);
    double d = 123.456;
    sut.write(d);
    double actual = 0.0;
    nbe::MemoryInputStream str(&buf);
    str.read(&actual);
    ASSERT_EQ(d,actual);
}

TEST(ByteBufferTest, testBulkGet)
{
    nbe::ByteBuffer sut;
    int i=100;
    sut.put(i);
    int actual=0;
    sut.get(reinterpret_cast<nbe::ByteBuffer::BufferType::value_type*>(&actual), sizeof(int));
    ASSERT_EQ(i,actual);
}

TEST(MemoryInputStreamTest, testReadRef)
{
    nbe::ByteBuffer buf;
    nbe::MemoryOutputStream str(&buf);
    auto s = new TestObj(1);
    str.writeRef(&s);
    s->write(str);
    nbe::MemoryInputStream sut(&buf);
    nbe::InputStream::ObjId id = -1;
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

    explicit TestNode(nbe::InputStream& str)
    :
    _parent(nullptr)
    {
        str.addObj(this);
        nbe::Stream::ObjId parentId{0};
        auto parentRef = str.readRef<TestObj>(&parentId);

        str.read(&_value);

        std::size_t numChildren = 0;
        str.read(&numChildren);

        for (auto i=0; i<numChildren; ++i)
        {
            nbe::Stream::ObjId childId = 0;
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

    nbe::OutputStream& write(nbe::OutputStream& str) const
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
    nbe::ByteBuffer buf;
    nbe::MemoryOutputStream str(&buf);
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

    nbe::MemoryInputStream sut(&buf);

    nbe::Stream::ObjId rootId = 0;
    auto* actual = sut.readRef<TestNode>(&rootId);
    ASSERT_EQ(parent->value(), actual->value());
    ASSERT_EQ(std::size_t{2}, actual->numChildren());
    ASSERT_EQ(2, actual->child(0)->value());
    delete actual;
    delete parent;
}

TEST(GraphTest, testSerialisationEmptyGraph)
{
    nbe::MemoryNodeLibrary nodeLib;
    auto g1 = new nbe::Graph();
    g1->setNodeLibrary(&nodeLib);
    auto buf = new nbe::ByteBuffer();
    auto out = new nbe::MemoryOutputStream(buf);
    if (out->writeRef(g1))
    {
        g1->write(*out);
    }
    auto in = new nbe::MemoryInputStream(buf);
    nbe::Stream::ObjId id = 0;
    nbe::Graph* g2 = nullptr;
    nbe::Stream::Ref ref = in->readRef(&id);
    if (id != 0)
    {
        if (ref != nullptr)
        {
            g2 = static_cast<nbe::Graph*>(ref);
        }
        else
        {
            g2 = new nbe::Graph(*in, nodeLib);
        }
    }

    ASSERT_EQ(*g1, *g2);
    delete g2;
    delete in;
    delete out;
    delete buf;
    delete g1;
}

TEST(GraphTest, testSerialisationOneNode)
{
    nbe::MemoryNodeLibrary nodeLib;
    auto g1 = new nbe::Graph();
    g1->setNodeLibrary(&nodeLib);
    auto n1 = g1->createNode("FooTyped", "foo1");
    g1->addNode(n1);
    auto buf = new nbe::ByteBuffer();
    auto out = new nbe::MemoryOutputStream(buf);
    if (out->writeRef(g1))
    {
        g1->write(*out);
    }
    g1->debug();
    auto in = new nbe::MemoryInputStream(buf);
    nbe::Stream::ObjId id = 0;
    nbe::Graph* g2 = nullptr;
    nbe::Stream::Ref ref = in->readRef(&id);
    if (id != 0)
    {
        if (ref != nullptr)
        {
            g2 = static_cast<nbe::Graph*>(ref);
        }
        else
        {
            g2 = new nbe::Graph(*in, nodeLib);
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
    nbe::MemoryNodeLibrary nodeLib;
    auto g1 = new nbe::Graph();
    g1->setNodeLibrary(&nodeLib);
    auto n1 = g1->createNode("FooTyped", "foo1");
    g1->addNode(n1);
    auto n2 = g1->createNode("BarTyped", "bar1");
    g1->addNode(n2);
    auto t1 = n2->dynamicPort(0)->connectTo(*n1->dynamicPort(0));
    auto buf = new nbe::ByteBuffer();
    auto out = new nbe::MemoryOutputStream(buf);
    if (out->writeRef(g1))
    {
        g1->write(*out);
    }
    g1->debug();
    auto in = new nbe::MemoryInputStream(buf);
    nbe::Stream::ObjId id = 0;
    nbe::Graph* g2 = nullptr;
    nbe::Stream::Ref ref = in->readRef(&id);
    if (id != 0)
    {
        if (ref != nullptr)
        {
            g2 = static_cast<nbe::Graph*>(ref);
        }
        else
        {
            g2 = new nbe::Graph(*in, nodeLib);
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
    nbe::ByteBuffer buf;
    auto str = new nbe::MemoryOutputStream(&buf);
    std::string expected="test";
    str->write(expected);
    auto sut = new nbe::MemoryInputStream(&buf);
    std::string actual;
    sut->read(&actual);
    ASSERT_EQ(expected, actual);
    delete sut;
    delete str;
}

class GraphTest_fromLua : public ::testing::TestWithParam<std::tuple<const char*, std::size_t, std::size_t, nbe::NodeID, std::size_t, nbe::Value>>
{

};

TEST_P(GraphTest_fromLua, testFromString)
{
    const char* graphStr = std::get<0>(GetParam());
    std::size_t numNodes = std::get<1>(GetParam());
    std::size_t numSignalPaths = std::get<2>(GetParam());
    nbe::NodeID nodeId = std::get<3>(GetParam());
    std::size_t portIndex = std::get<4>(GetParam());
    nbe::Value value = std::get<5>(GetParam());

    nbe::MemoryNodeLibrary nodeLib;
    auto sut = nbe::Graph::fromString(nodeLib, graphStr);
    ASSERT_NE(nullptr, sut);
    EXPECT_EQ(numNodes, sut->numNodes());
    EXPECT_EQ(numSignalPaths, sut->numSignalPaths());
    auto actualNode = sut->lastAddedNode();
    ASSERT_NE(nullptr, actualNode);
    auto actualPort = actualNode->dynamicPort(portIndex);
    ASSERT_NE(nullptr, actualPort);
    nbe::ValueVisitor valueVisitor;
    actualPort->accept(valueVisitor);
    nbe::Value actualValue = valueVisitor.value();

    EXPECT_EQ(value, actualValue);
    delete sut;
}

INSTANTIATE_TEST_SUITE_P(GraphTest, GraphTest_fromLua, ::testing::Values(
    std::make_tuple("graph={ nodes={ { name=\"foo\", class=\"FooTyped\", category=\"CATEGORY_SINK\", ports={ { name=\"in1\", class=\"TypedPort<double>\", type=\"TYPE_DOUBLE\", dir=\"DIR_IN\", value=2.0 } } } } }", std::size_t{ 1 }, std::size_t{ 0 }, nbe::NodeID{ 0 }, std::size_t{ 0 }, nbe::Value{2.0} ),
    std::make_tuple("graph={ nodes={ { name=\"foo\", class=\"Boundary\", category=\"CATEGORY_GROUP\", ports={ { name=\"in1\", class=\"TypedPort<double>\", type=\"TYPE_DOUBLE\", dir=\"DIR_IN\", value=2.0 } } } } }", std::size_t{ 1 }, std::size_t{ 0 }, nbe::NodeID{ 0 }, std::size_t{ 0 }, nbe::Value{2.0} ),
    std::make_tuple("graph={ nodes={ { name=\"foo\", class=\"Boundary\", category=\"CATEGORY_GROUP\", ports={ { name=\"in1\", class=\"TypedPort<int64_t>\", type=\"TYPE_INT\", dir=\"DIR_IN\", value=2 } } } } }", std::size_t{ 1 }, std::size_t{ 0 }, nbe::NodeID{ 0 }, std::size_t{ 0 }, nbe::Value{std::int64_t(2)} ),
    std::make_tuple("graph={ nodes={ { name=\"foo\", class=\"Boundary\", category=\"CATEGORY_GROUP\", ports={ { name=\"in1\", class=\"TypedPort<string>\", type=\"TYPE_STRING\", dir=\"DIR_IN\", value=\"wibble\" } } } } }", std::size_t{ 1 }, std::size_t{ 0 }, nbe::NodeID{ 0 }, std::size_t{ 0 }, nbe::Value{std::string("wibble")} ),
    std::make_tuple("graph={ nodes={ { name=\"foo\", class=\"Boundary\", category=\"CATEGORY_GROUP\", ports={ { name=\"in1\", class=\"TypedPort<bool>\", type=\"TYPE_BOOL\", dir=\"DIR_IN\", value=true } } } } }", std::size_t{ 1 }, std::size_t{ 0 }, nbe::NodeID{ 0 }, std::size_t{ 0 }, nbe::Value{true} )
));

TEST(GraphTest, testDuplciateNodeID)
{
    nbe::MemoryNodeLibrary nodeLib;
    const char* graphFilename = "etc/tests/Graph/duplicateID.lua";
    auto sut = nbe::Graph::fromFile(nodeLib, graphFilename);

    ASSERT_EQ(nullptr, sut);
}

class GraphTest_fromLuaFile : public ::testing::TestWithParam<std::tuple<const char*, std::size_t, std::size_t, nbe::NodeID, std::size_t, nbe::Value, std::size_t, std::size_t, std::size_t>>
{

};

TEST_P(GraphTest_fromLuaFile, testFromFile)
{
    const char* graphStr = std::get<0>(GetParam());
    std::size_t numNodes = std::get<1>(GetParam());
    std::size_t numSignalPaths = std::get<2>(GetParam());
    nbe::NodeID nodeId = std::get<3>(GetParam());
    std::size_t portIndex = std::get<4>(GetParam());
    nbe::Value value = std::get<5>(GetParam());
    std::size_t numIncomingPorts = std::get<6>(GetParam());
    std::size_t numOutgoingPorts = std::get<7>(GetParam());
    std::size_t numChildren = std::get<8>(GetParam());

    nbe::MemoryNodeLibrary nodeLib;
    auto sut = nbe::Graph::fromFile(nodeLib, graphStr);
    ASSERT_NE(nullptr, sut);
    EXPECT_EQ(numNodes, sut->numNodes());
    EXPECT_EQ(numSignalPaths, sut->numSignalPaths());
    auto actualNode = sut->node(nodeId);
    ASSERT_NE(nullptr, actualNode);
    auto actualPort = actualNode->dynamicPort(portIndex);
    ASSERT_NE(nullptr, actualPort);
    nbe::ValueVisitor valueVisitor;
    actualPort->accept(valueVisitor);
    nbe::Value actualValue = valueVisitor.value();
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
    std::make_tuple("etc/tests/Graph/onenode.lua", std::size_t{ 1 }, std::size_t{ 0 }, nbe::NodeID{ 0 }, std::size_t{ 0 }, nbe::Value{2.0}, std::size_t{0}, std::size_t{0}, std::size_t{0} ),
    std::make_tuple("etc/tests/Graph/connectednodes.lua", std::size_t{ 2 }, std::size_t{ 1 }, nbe::NodeID{ 0 }, std::size_t{ 0 }, nbe::Value{1.0}, std::size_t{1}, std::size_t{0}, std::size_t{ 0 }),
    std::make_tuple("etc/tests/Graph/withchildgraph.lua", std::size_t{ 1 }, std::size_t{ 0 }, nbe::NodeID{ 0 }, std::size_t{ 0 }, nbe::Value{2.0}, std::size_t{0}, std::size_t{0}, std::size_t{ 1 }),
    std::make_tuple("etc/tests/Graph/withnestedchildgraph.lua", std::size_t{ 1 }, std::size_t{ 0 }, nbe::NodeID{ 0 }, std::size_t{ 0 }, nbe::Value{2.0}, std::size_t{0}, std::size_t{0}, std::size_t{ 2 })
));

class GraphTest_toLuaRoundTrip : public ::testing::TestWithParam<std::tuple<const char*>>
{

};

TEST_P(GraphTest_toLuaRoundTrip, testRoundTrip)
{
    auto graphFilename = std::get<0>(GetParam());
    nbe::MemoryNodeLibrary nodeLib;
    auto sut = nbe::Graph::fromFile(nodeLib, graphFilename);
    ASSERT_NE(nullptr, sut);
    std::string graphString;
    std::ostringstream graphStr;
    sut->toLua(graphStr);
    graphString = graphStr.str();
    std::cerr << graphString << '\n';
    auto actual = nbe::Graph::fromString(nodeLib, graphString.c_str());
    ASSERT_NE(nullptr, actual);
    ASSERT_EQ(*sut,*actual);
}

INSTANTIATE_TEST_SUITE_P(GraphTest, GraphTest_toLuaRoundTrip, ::testing::Values(
    std::make_tuple("etc/tests/Graph/empty.lua"),
    std::make_tuple("etc/tests/Graph/onenode.lua"),
    std::make_tuple("etc/tests/Graph/connectednodes.lua"),
    std::make_tuple("etc/tests/Graph/withchildgraph.lua"),
    std::make_tuple("etc/tests/Graph/withnestedchildgraph.lua"),
    std::make_tuple("etc/tests/Graph/typedport.lua")
));

class PortType_testParseFromString : public ::testing::TestWithParam<std::tuple<const char*, nbe::PortType::Type>>
{

};

TEST_P(PortType_testParseFromString, testParseFromString)
{
    const char* str = std::get<0>(GetParam());
    nbe::PortType::Type type = std::get<1>(GetParam());
    auto actualType = nbe::PortType::parseFromString(str);
    EXPECT_EQ(type, actualType);
}

INSTANTIATE_TEST_SUITE_P(PortType, PortType_testParseFromString, ::testing::Values(
    std::make_tuple("TYPE_UNKNOWN", nbe::PortType::TYPE_UNKNOWN),
    std::make_tuple("TYPE_INT", nbe::PortType::TYPE_INT),
    std::make_tuple("TYPE_DOUBLE", nbe::PortType::TYPE_DOUBLE),
    std::make_tuple("TYPE_STRING", nbe::PortType::TYPE_STRING),
    std::make_tuple("TYPE_BOOL", nbe::PortType::TYPE_BOOL),
    std::make_tuple("TYPE_VEC3D", nbe::PortType::TYPE_VEC3D),
    std::make_tuple("TYPE_OPAQUE", nbe::PortType::TYPE_OPAQUE),
    std::make_tuple("TYPE_VECTOR", nbe::PortType::TYPE_VECTOR)
));

class PortType_testToString : public ::testing::TestWithParam<std::tuple<const char*, nbe::PortType::Type>>
{

};

TEST_P(PortType_testToString, testParseFromString)
{
    std::string str = std::get<0>(GetParam());
    nbe::PortType::Type type = std::get<1>(GetParam());
    auto actualStr = nbe::PortType::toString(type);
    EXPECT_EQ(str, actualStr);
}

INSTANTIATE_TEST_SUITE_P(PortType, PortType_testToString, ::testing::Values(
    std::make_tuple("TYPE_UNKNOWN", nbe::PortType::TYPE_UNKNOWN),
    std::make_tuple("TYPE_INT", nbe::PortType::TYPE_INT),
    std::make_tuple("TYPE_DOUBLE", nbe::PortType::TYPE_DOUBLE),
    std::make_tuple("TYPE_STRING", nbe::PortType::TYPE_STRING),
    std::make_tuple("TYPE_BOOL", nbe::PortType::TYPE_BOOL),
    std::make_tuple("TYPE_VEC3D", nbe::PortType::TYPE_VEC3D),
    std::make_tuple("TYPE_OPAQUE", nbe::PortType::TYPE_OPAQUE),
    std::make_tuple("TYPE_VECTOR", nbe::PortType::TYPE_VECTOR)
));

class PortDirection_testParseFromString : public ::testing::TestWithParam<std::tuple<const char*, nbe::PortDirection::Direction>>
{

};

TEST_P(PortDirection_testParseFromString, testParseFromString)
{
    const char* str = std::get<0>(GetParam());
    nbe::PortDirection::Direction dir = std::get<1>(GetParam());
    auto actualDir = nbe::PortDirection::parseFromString(str);
    EXPECT_EQ(dir, actualDir);
}

INSTANTIATE_TEST_SUITE_P(PortDirection, PortDirection_testParseFromString, ::testing::Values(
    std::make_tuple("DIR_UNKNOWN", nbe::PortDirection::DIR_UNKNOWN),
    std::make_tuple("DIR_IN", nbe::PortDirection::DIR_IN),
    std::make_tuple("DIR_OUT", nbe::PortDirection::DIR_OUT),
    std::make_tuple("DIR_INTERNAL", nbe::PortDirection::DIR_INTERNAL)

));

class PortDirection_testToString : public ::testing::TestWithParam<std::tuple<const char*, nbe::PortDirection::Direction>>
{

};

TEST_P(PortDirection_testToString, testParseFromString)
{
    std::string str = std::get<0>(GetParam());
    nbe::PortDirection::Direction dir = std::get<1>(GetParam());
    auto actualStr = nbe::PortDirection::toString(dir);
    EXPECT_EQ(str, actualStr);
}

INSTANTIATE_TEST_SUITE_P(PortDirection, PortDirection_testToString, ::testing::Values(
    std::make_tuple("DIR_UNKNOWN", nbe::PortDirection::DIR_UNKNOWN),
    std::make_tuple("DIR_IN", nbe::PortDirection::DIR_IN),
    std::make_tuple("DIR_OUT", nbe::PortDirection::DIR_OUT),
    std::make_tuple("DIR_INTERNAL", nbe::PortDirection::DIR_INTERNAL)

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
    nbe::FileSystemTraverser trav(dir);
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
    nbe::NodePluginScanner sut;
    nbe::MemoryNodeLibrary nodeLib;
    sut.scan(nodeLib, nodeLib);
    std::size_t actualTotalNodes = sut.totalNodes();
    EXPECT_EQ(totalNodes, actualTotalNodes);
}

INSTANTIATE_TEST_SUITE_P(NodePluginScanner, NodePluginScannerTest_testFindPlugins, ::testing::Values(
    std::make_tuple(std::size_t{1})
));

TEST(NodeLibraryTest, testRegisterNodeGivenANullNodeNothingHappens)
{
    nbe::MemoryNodeLibrary nodeLib;

    std::size_t numNodesBefore = nodeLib.numNodes();
    nodeLib.registerNode(nullptr);
    EXPECT_EQ(numNodesBefore, nodeLib.numNodes());
}

TEST(NodeLibraryTest, testRegisterDuplicateNodeHasNoEffect)
{
    nbe::MemoryNodeLibrary nodeLib;

    std::size_t numNodesBefore = nodeLib.numNodes();
    auto existingNode = nodeLib.instantiateNode(nodeLib.nextNodeID(), "FooTyped", "foo1");
    ASSERT_NE(nullptr, existingNode);
    nodeLib.registerNode(existingNode);
    EXPECT_EQ(numNodesBefore, nodeLib.numNodes());
}

TEST(NodeLibraryTest, testRegisterNewNodeSucceeds)
{
    nbe::NodePluginScanner scanner;
    nbe::MemoryNodeLibrary nodeLib;

    std::size_t numNodesBefore = nodeLib.numNodes();
    scanner.scan(nodeLib, nodeLib);
    EXPECT_EQ(numNodesBefore+1, nodeLib.numNodes());

}

TEST(GraphTest, testPersistNodeFromPlugin)
{
    nbe::NodePluginScanner scanner;
    nbe::MemoryNodeLibrary nodeLib;

    scanner.scan(nodeLib, nodeLib);
    auto node = nodeLib.instantiateNode(nodeLib.nextNodeID(), "NodePlugin.DynamicNode", "node1");
    ASSERT_NE(nullptr, node);
    nbe::Graph* graph = new nbe::Graph();
    graph->setNodeLibrary(&nodeLib);
    graph->addNode(node);
    ASSERT_EQ(size_t{1}, graph->numNodes());
    std::ostringstream  str;
    graph->toLua(str);
    auto graph2 = nbe::Graph::fromString(nodeLib, str.str().c_str());
    EXPECT_EQ(*graph,*graph2);
    delete graph2;
    delete graph;
}

TEST(GraphTest, testLoadGraphWithNodesFromPlugin)
{
    nbe::NodePluginScanner scanner;
    nbe::MemoryNodeLibrary nodeLib;

    scanner.scan(nodeLib, nodeLib);
    auto graph = nbe::Graph::fromFile(nodeLib, "etc/tests/Graph/nodesFromPlugin.lua");
    ASSERT_NE(nullptr, graph);
    auto node = graph->node(0);
    ASSERT_NE(nullptr, node);
    nbe::TypedPort<double>* port = dynamic_cast<nbe::TypedPort<double>*>(node->dynamicPort(0));
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
    nbe::MemoryNodeLibrary nodeLib;
    auto sut = nbe::Graph::fromFile(nodeLib, graphFilename);
    ASSERT_NE(nullptr, sut);
    auto* buf = new nbe::ByteBuffer();
    nbe::MemoryOutputStream ostr(buf);
    if (ostr.writeRef(sut))
    {
        sut->write(ostr);
    }
    nbe::MemoryInputStream istr(buf);
    nbe::Stream::ObjId id{~0U};
    nbe::Graph* actual = nullptr;
    nbe::Stream::Ref ref = istr.readRef(&id);
    if (id != 0)
    {
        if (ref != nullptr)
        {
            actual = static_cast<nbe::Graph*>(ref);
        }
        else
        {
            actual = new nbe::Graph(istr, nodeLib);
        }
    }
    ASSERT_NE(nullptr, actual);
    EXPECT_EQ(*sut, *actual);
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

class GraphTest_testEvaluate : public ::testing::TestWithParam<std::tuple<const char*, nbe::NodeID, std::size_t, double>>
{
public:
    void TearDown()
    {
        delete _sut;
    }
protected:
    nbe::MemoryNodeLibrary _nodeLib;
    nbe::Graph* _sut;
};

TEST_P(GraphTest_testEvaluate, testEvaluate)
{
    const char* graphFilename = std::get<0>(GetParam());
    nbe::NodeID nodeId = std::get<1>(GetParam());
    std::size_t portIndex = std::get<2>(GetParam());
    double value = std::get<3>(GetParam());

    _sut = nbe::Graph::fromFile(_nodeLib, graphFilename);
    ASSERT_NE(nullptr, _sut);
    nbe::NodeArray order;
    _sut->topologicalSort(&order);
    _sut->evaluate(order);
    nbe::Node* actualNode = _sut->node(nodeId);
    ASSERT_NE(nullptr, actualNode);
    nbe::Port* actualPort = actualNode->dynamicPort(portIndex);
    ASSERT_NE(nullptr, actualPort);
    nbe::ValueVisitor visitor;
    ASSERT_EQ(nbe::PortType::TYPE_DOUBLE, actualPort->type());
    actualPort->accept(visitor);
    EXPECT_EQ(value, visitor.value().operator double());
}

INSTANTIATE_TEST_SUITE_P(Graph, GraphTest_testEvaluate, ::testing::Values(
        std::make_tuple("etc/tests/Graph/constraints.lua", 0, 2, 1.0)
        ));