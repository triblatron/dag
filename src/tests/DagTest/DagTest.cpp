#include "config/config.h"

#include "gtest/gtest.h"
#include "core/LuaInterface.h"
#include "core/Node.h"
#include "core/TypedPort.h"
#include "MemoryNodeLibrary.h"
#include "Nodes.h"
#include "core/NodeCategory.h"
#include "core/Graph.h"
#include "SelectionLive.h"
#include "NodeEditorLive.h"
#include "Boundary.h"
#include "core/SignalPath.h"
#include "CreateNode.h"
#include "core/ByteBuffer.h"
#include "FileSystemTraverser.h"
#include "NodePluginScanner.h"
#include "core/CloningFacility.h"
#include "io/MemoryBackingStore.h"
#include "core/Class.h"
#include "core/MetaClass.h"
#include "io/BinaryInputStream.h"
#include "io/BinaryOutputStream.h"
#include "io/TextInputStream.h"
#include "io/TextOutputStream.h"
#include "test/TestUtils.h"

#include <iostream>
#include <algorithm>
#include <filesystem>

class MemoryNodeLibraryTest : public ::testing::TestWithParam<std::tuple<const char*, const char*, size_t, const char*, dagbase::PortDirection::Direction, double>>
{
};

TEST_P(MemoryNodeLibraryTest, checkInstantiate)
{
    dag::MemoryNodeLibrary sut;
    const char* className = std::get<0>(GetParam());
    const char* name = std::get<1>(GetParam());
    size_t portIndex = std::get<2>(GetParam());
    const char* portName = std::get<3>(GetParam());
    dagbase::PortDirection::Direction portDir = std::get<4>(GetParam());
    double value = std::get<5>(GetParam());
    dagbase::Node* actualNode = sut.instantiateNode(sut, className, name);
    ASSERT_NE(nullptr, actualNode);
    EXPECT_STREQ(name, actualNode->name().c_str());
    EXPECT_GT(actualNode->totalPorts(), portIndex);
    auto actualPort = dynamic_cast<dagbase::TypedPort<double>*>(actualNode->dynamicPort(portIndex));
    ASSERT_NE(nullptr, actualPort);
    EXPECT_STREQ(portName, actualPort->name().c_str());
    EXPECT_EQ(portDir, actualPort->dir());
    EXPECT_EQ(value, double(actualPort->value()));
    
    delete actualNode;
}

INSTANTIATE_TEST_SUITE_P(MemoryNodeLibraryInstantiateTest, MemoryNodeLibraryTest, ::testing::Values(
    std::make_tuple("FooTyped", "foo1", 0, "in1", dagbase::PortDirection::DIR_IN, 1.0),
    std::make_tuple("BarTyped", "bar1", 0, "out1", dagbase::PortDirection::DIR_OUT, 1.0)
));

TEST(MemoryNodeLibraryTest_testClassNotFound, checkClassNotFound)
{
    dag::MemoryNodeLibrary sut;
    ASSERT_THROW(sut.instantiateNode(sut, "NotFound", "notFound1"), std::runtime_error);
}

TEST(ValueTest_testIncrement, checkValueChanges)
{
    dagbase::Value sut;
    sut += std::int64_t(1);
    EXPECT_EQ(1, std::int64_t(sut));
}

class NodeCategoryTest : public ::testing::TestWithParam<std::tuple<const char*, const char*, dagbase::NodeCategory::Category>>
{

};

TEST_P(NodeCategoryTest, checkCategory)
{
    std::string className = std::get<0>(GetParam());
    std::string name = std::get<1>(GetParam());
    dagbase::NodeCategory::Category category = std::get<2>(GetParam());
    dag::MemoryNodeLibrary nodeLib;
    dagbase::Node* actual = nodeLib.instantiateNode(nodeLib, className, name);
    ASSERT_NE(nullptr, actual);
    EXPECT_EQ(category, actual->category());
    delete actual;
}

INSTANTIATE_TEST_SUITE_P(NodeCategoryVerifyCategoryTest, NodeCategoryTest, ::testing::Values(
    std::make_tuple("FooTyped", "foo1", dagbase::NodeCategory::CAT_SINK)
));

class PortTypeTest : public ::testing::TestWithParam<std::tuple<double, double>>
{

};

TEST_P(PortTypeTest, checkSetValue)
{
    double value = std::get<0>(GetParam());
    double newValue = std::get<1>(GetParam());
    auto sut = new dagbase::TypedPort<double>(0, "test1", dagbase::PortType::TYPE_DOUBLE, dagbase::PortDirection::DIR_IN, value);
    EXPECT_EQ(value, double(sut->value()));
    sut->setValue(newValue);
    EXPECT_EQ(newValue, double(sut->value()));
    delete sut;
}

INSTANTIATE_TEST_SUITE_P(PortTypeSetValueTest, PortTypeTest, ::testing::Values(
    std::make_tuple(1.0, 2.0)
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
    dagbase::Node* node = nodeLib.instantiateNode(nodeLib, className, nodeName);
    dagbase::CloningFacility facility;
    dagbase::Node* sut = node->clone(facility, dagbase::CopyOp{0}, nullptr);
    ASSERT_EQ(node->totalPorts(), sut->totalPorts());
    ASSERT_NE(node->dynamicPort(index), sut->dynamicPort(index));
    ASSERT_NE(node->dynamicPort(index)->parent(), sut->dynamicPort(index)->parent());
    delete sut;
    delete node;
}

INSTANTIATE_TEST_SUITE_P(NodeTestClone, NodeTest_testClone, ::testing::Values(
    std::make_tuple("Base", "base1", 0),
    std::make_tuple("Derived", "derived1", 0),
    std::make_tuple("Final", "final1", 0),
    std::make_tuple("Final", "final1", 1),
    std::make_tuple("Final", "final1", 2),
    std::make_tuple("GroupTyped", "group1", 0),
    std::make_tuple("GroupTyped", "group1", 1)
));

TEST(TypedTransferTest, checkMakeItSo)
{
    dag::MemoryNodeLibrary nodeLib;
    auto input = nodeLib.instantiateNode(nodeLib, "FooTyped", "foo1");
    auto output = nodeLib.instantiateNode(nodeLib, "BarTyped", "bar1");

    auto typedOutput = dynamic_cast<dagbase::TypedPort<double>*>(output->dynamicPort(0));
    ASSERT_NE(nullptr, typedOutput);
    typedOutput->setValue(2.0);
    auto typedInput = dynamic_cast<dagbase::TypedPort<double>*>(input->dynamicPort(0));
    ASSERT_NE(nullptr, typedInput);
    dagbase::Transfer* transfer = typedOutput->connectTo(*typedInput);
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

TEST(TypedPortTransfer, testConnectToDifferentTypes)
{
    auto* source = new dagbase::TypedPort(0, "out1", dagbase::PortType::TYPE_INT64, dagbase::PortDirection::DIR_OUT, std::int64_t{ 1 });
    auto* dest = new dagbase::TypedPort(1, "in1", dagbase::PortType::TYPE_DOUBLE, dagbase::PortDirection::DIR_IN, 0.0);
    dagbase::Transfer const * transfer = source->connectTo(*dest);
    ASSERT_EQ(nullptr, transfer);
    delete transfer;
    delete dest;
    delete source;
}

TEST(TypedPortTransfer, testConnectToMatchingType)
{
    dagbase::TypedPort<std::int64_t>* source = new dagbase::TypedPort(0, "out1", dagbase::PortType::TYPE_INT64, dagbase::PortDirection::DIR_OUT, std::int64_t{ 1 });
    dagbase::TypedPort<std::int64_t>* dest = new dagbase::TypedPort(1, "in1", dagbase::PortType::TYPE_INT64, dagbase::PortDirection::DIR_IN, std::int64_t{ 0 });
    dagbase::Transfer * transfer = source->connectTo(*dest);
    ASSERT_NE(nullptr, transfer);
    ASSERT_TRUE(transfer->valid());
    transfer->makeItSo();
    ASSERT_EQ(1, dest->value());
    delete dest;
    delete source;
    delete transfer;
}

TEST(GraphTest, testGraphInitiallyHasNoNodes)
{
    auto* sut = new dagbase::Graph();

    ASSERT_EQ(size_t{ 0 }, sut->numNodes());
    delete sut;
}

TEST(GraphTest, testGraphInitiallyHasNoSignalPaths)
{
    auto* sut = new dagbase::Graph();

    EXPECT_EQ(size_t{ 0 }, sut->numSignalPaths());

    delete sut;
}

TEST(GraphTest, testCannotAddANullNode)
{
    auto sut = new dagbase::Graph();

    sut->addNode(nullptr);
    ASSERT_EQ(size_t{ 0 }, sut->numNodes());
    ASSERT_EQ(nullptr, sut->node(0));

    delete sut;
}

TEST(GraphTest, testWhenAddingANodeThenQueryReturnsIt)
{
    dag::MemoryNodeLibrary nodeLib;
    auto* sut = new dagbase::Graph();
    sut->setNodeLibrary(&nodeLib);
    auto const node = nodeLib.instantiateNode(*sut, "FooTyped", "foo1");
    sut->addNode(node);
    ASSERT_EQ(size_t{ 1 }, sut->numNodes());
    ASSERT_EQ(node, sut->node(node->id()));

    delete sut;
}

TEST(GraphTest, testCannotAddANullSignalPath)
{
    auto const sut = new dagbase::Graph();

    sut->addSignalPath(nullptr);
    ASSERT_EQ(size_t{ 0 }, sut->numSignalPaths());
    ASSERT_EQ(nullptr, sut->signalPath(0));
    
    delete sut;
}

TEST(GraphTest, testAfterAddingASignalPathCanQueryIt)
{
    dag::MemoryNodeLibrary nodeLib;
    auto const sut = new dagbase::Graph();
    sut->setNodeLibrary(&nodeLib);
    auto n1 = sut->createNode("BarTyped","out1");
    auto n2 = sut->createNode("FooTyped","in1");
    sut->addNode(n1);
    sut->addNode(n2);
    auto const path = new dagbase::SignalPath(n1->dynamicPort(0), n2->dynamicPort(0));
    sut->addSignalPath(path);

    ASSERT_EQ(path, sut->signalPath(path->id()));

    delete sut;
}

TEST(PortTest, testCannotConnectTwoOutputs)
{
    dag::MemoryNodeLibrary nodeLib;
    auto output1 = dynamic_cast<dag::BarTyped*>(nodeLib.instantiateNode(nodeLib, "BarTyped", "bar1"));
    auto output2 = dynamic_cast<dag::BarTyped*>(nodeLib.instantiateNode(nodeLib, "BarTyped", "bar2"));
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
    _str(0, "out1", dagbase::PortType::TYPE_STRING, dagbase::PortDirection::DIR_OUT, "test")
    {
	    // Do nothing.
    }

    dagbase::TypedPort<std::string>& stringPort()
    {
        return _str;
    }
private:
    dagbase::TypedPort<std::string> _str;
};

TEST(PortTest, testCannotConnectDifferentTypes)
{
    dag::MemoryNodeLibrary nodeLib;
    auto output = new TestNodeWithStringPort();
    auto foo = dynamic_cast<dag::FooTyped*>(nodeLib.instantiateNode(nodeLib, "FooTyped", "foo1"));
    auto t1 = output->stringPort().connectTo(foo->in1());
    ASSERT_EQ(size_t{ 0 }, output->stringPort().numOutgoingConnections());
    ASSERT_EQ(size_t{ 0 }, foo->in1().numIncomingConnections());
    delete t1;
    delete foo;
    delete output;
}

TEST(PortTestCompatibleTypes, testCompatibleTypesIntToDouble)
{
    auto output = new dagbase::TypedPort(0, "output", dagbase::PortType::TYPE_INT64, dagbase::PortDirection::DIR_OUT, std::int64_t{1});
    auto input = new dagbase::TypedPort(1, "input", dagbase::PortType::TYPE_DOUBLE, dagbase::PortDirection::DIR_IN, 2.0);
    ASSERT_EQ(false, output->isCompatibleWith(*input));
    delete input;
    delete output;
}

TEST(PortTestCompatibleTypes, testCompatibleExactMatch)
{
    dagbase::TypedPort<double> const* output = new dagbase::TypedPort(0, "output", dagbase::PortType::TYPE_DOUBLE, dagbase::PortDirection::DIR_OUT, 1.0);
    auto* input = new dagbase::TypedPort(1, "input", dagbase::PortType::TYPE_DOUBLE, dagbase::PortDirection::DIR_IN, 2.0);
    ASSERT_EQ(true, output->isCompatibleWith(*input));
    delete input;
    delete output;
}

TEST(PortTestCompatibleTypes, testCompatibleBoolToInt)
{
    auto const output = new dagbase::TypedPort(0, "output", dagbase::PortType::TYPE_BOOL, dagbase::PortDirection::DIR_OUT, true);
    auto input = new dagbase::TypedPort(1, "input", dagbase::PortType::TYPE_INT64, dagbase::PortDirection::DIR_IN, std::int64_t{2});
    ASSERT_EQ(false, output->isCompatibleWith(*input));
    delete input;
    delete output;
}

TEST(PortTestCompatibleTypes, testCompatibleBoolToDouble)
{
    auto const * output = new dagbase::TypedPort(0, "output", dagbase::PortType::TYPE_BOOL, dagbase::PortDirection::DIR_OUT, true);
    auto input = new dagbase::TypedPort(1, "input", dagbase::PortType::TYPE_DOUBLE, dagbase::PortDirection::DIR_IN, 2.0);
    ASSERT_EQ(false, output->isCompatibleWith(*input));
    delete input;
    delete output;
}

TEST(PortTest, testDisconnectRemovesConnection)
{
    auto* output = new dagbase::TypedPort(0, "output", dagbase::PortType::TYPE_BOOL, dagbase::PortDirection::DIR_OUT, true);
    auto* input = new dagbase::TypedPort(1, "input", dagbase::PortType::TYPE_DOUBLE, dagbase::PortDirection::DIR_IN, 2.0);
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
    auto output = new dagbase::TypedPort<std::int64_t>(0, "output", dagbase::PortType::TYPE_INT64, dagbase::PortDirection::DIR_OUT, 1);
    auto* input = new dagbase::TypedPort(1, "input", dagbase::PortType::TYPE_DOUBLE, dagbase::PortDirection::DIR_IN, 2.0);
    dagbase::Transfer* transfer = output->connectTo(*input);
    ASSERT_EQ(nullptr, transfer);
    delete transfer;
    delete input;
    delete output;
}

TEST(PortTest, testEachOutgoingConnection)
{
    auto output = new dagbase::TypedPort<std::int64_t>(0, "output", dagbase::PortType::TYPE_DOUBLE, dagbase::PortDirection::DIR_OUT, 1.0);
    auto* input = new dagbase::TypedPort(1, "input", dagbase::PortType::TYPE_DOUBLE, dagbase::PortDirection::DIR_IN, 2.0);
    size_t outgoingCount = 0;
    size_t incomingCount = 0;
    auto t = output->connectTo(*input);
    output->eachOutgoingConnection([&outgoingCount](dagbase::Port* item)
        {
            ++outgoingCount;
        });
    input->eachIncomingConnection([&incomingCount](dagbase::Port* item)
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

    auto const input = dynamic_cast<dag::FooTyped*>(nodeLib.instantiateNode(nodeLib, "FooTyped", "foo1"));
    auto const output = dynamic_cast<dag::BarTyped*>(nodeLib.instantiateNode(nodeLib, "BarTyped", "bar1"));

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

    auto const input = dynamic_cast<dag::FooTyped*>(nodeLib.instantiateNode(nodeLib, "FooTyped", "foo1"));
    auto const output = dynamic_cast<dag::BarTyped*>(nodeLib.instantiateNode(nodeLib, "BarTyped", "bar1"));

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

    auto const input = dynamic_cast<dag::FooTyped*>(nodeLib.instantiateNode(nodeLib, "FooTyped", "foo1"));
    auto const output = dynamic_cast<dag::BarTyped*>(nodeLib.instantiateNode(nodeLib, "BarTyped", "bar1"));
    auto group = dynamic_cast<dag::GroupTyped*>(nodeLib.instantiateNode(nodeLib, "GroupTyped", "group1"));
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

TEST(NodeTest, testDynamicPortDescriptorReturnsStaticPortsInRange)
{
    dag::MemoryNodeLibrary nodeLib;
    auto const sut = dynamic_cast<dag::Final*>(nodeLib.instantiateNode(nodeLib, "Final", "final1"));
    ASSERT_NE(nullptr, sut);
    ASSERT_NE(nullptr, sut->dynamicMetaPort(0));
    ASSERT_TRUE(sut->dynamicMetaPort(0)->isOwned());
    ASSERT_NE(nullptr, sut->dynamicMetaPort(1));
    ASSERT_TRUE(sut->dynamicMetaPort(1)->isOwned());
    delete sut;
}

TEST(NodeTest, testDynamicsPortDescriptorsForFinal)
{
    dag::MemoryNodeLibrary nodeLib;
    auto const sut = dynamic_cast<dag::Final*>(nodeLib.instantiateNode(nodeLib, "Final", "final1"));
    ASSERT_NE(nullptr, sut);
    sut->addDynamicPort(new dagbase::TypedPort<double>(0, "output1", dagbase::PortType::TYPE_DOUBLE, dagbase::PortDirection::DIR_OUT, 1.0), dagbase::MetaPort::FLAGS_OWN_BIT);
    ASSERT_NE(nullptr, sut->dynamicMetaPort(2));
    ASSERT_EQ("int1", sut->dynamicPort(2)->name());
    ASSERT_TRUE(sut->dynamicMetaPort(2)->isOwned());
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
    auto const sut = nodeLib.instantiateNode(nodeLib, className, "node1");
    ASSERT_NE(nullptr, sut);
    sut->addDynamicPort(new dagbase::TypedPort<double>(0, "output1", dagbase::PortType::TYPE_DOUBLE, dagbase::PortDirection::DIR_OUT, 1.0), dagbase::MetaPort::FLAGS_OWN_BIT);
    auto const actualPort = sut->dynamicPort(index);
    ASSERT_NE(nullptr, actualPort);
    ASSERT_EQ(nodeName, actualPort->name());
    auto const actualDescriptor = sut->dynamicMetaPort(index);
    ASSERT_NE(nullptr, actualDescriptor);
    ASSERT_TRUE(actualDescriptor->isOwned());
    delete sut;
}

INSTANTIATE_TEST_SUITE_P(NodeTest, NodeTestDynamicPortsForNode, ::testing::Values(
    std::make_tuple("Final", 0, "direction"),
    std::make_tuple("Final", 1, "trigger"),
    std::make_tuple("Final", 2, "int1"),
    std::make_tuple("Final", 3, "output1")
));

class NodeTestDynamicPortDescriptorForNode : public ::testing::TestWithParam<std::tuple<const char*, size_t, const char*, dagbase::PortType::Type, dagbase::PortDirection::Direction>>
{
};

TEST_P(NodeTestDynamicPortDescriptorForNode, testDynamicPortDescriptor)
{
    std::string const className = std::get<0>(GetParam());
    size_t const index = std::get<1>(GetParam());
    std::string const nodeName = std::get<2>(GetParam());
    dagbase::PortType::Type type = std::get<3>(GetParam());
    dagbase::PortDirection::Direction dir = std::get<4>(GetParam());
    dag::MemoryNodeLibrary nodeLib;
    auto const sut = nodeLib.instantiateNode(nodeLib, className, "node1");
    ASSERT_NE(nullptr, sut);
    auto const actualPort = sut->dynamicPort(index);
    ASSERT_NE(nullptr, actualPort);
    ASSERT_EQ(nodeName, actualPort->name());
    auto const actualDescriptor = sut->dynamicMetaPort(index);
    ASSERT_NE(nullptr, actualDescriptor);
    ASSERT_TRUE(actualDescriptor->isOwned());
    delete sut;
}

INSTANTIATE_TEST_SUITE_P(NodeTest, NodeTestDynamicPortDescriptorForNode, ::testing::Values(
    std::make_tuple("Base", 0, "direction", dagbase::PortType::TYPE_DOUBLE, dagbase::PortDirection::DIR_OUT),
    std::make_tuple("Derived", 1, "trigger", dagbase::PortType::TYPE_BOOL, dagbase::PortDirection::DIR_IN),
    std::make_tuple("Final", 0, "direction", dagbase::PortType::TYPE_DOUBLE, dagbase::PortDirection::DIR_OUT),
    std::make_tuple("Final", 1, "trigger", dagbase::PortType::TYPE_BOOL, dagbase::PortDirection::DIR_IN),
    std::make_tuple("FooTyped", 0, "in1", dagbase::PortType::TYPE_DOUBLE, dagbase::PortDirection::DIR_IN),
    std::make_tuple("BarTyped", 0, "out1", dagbase::PortType::TYPE_DOUBLE, dagbase::PortDirection::DIR_OUT),
    std::make_tuple("GroupTyped", 0, "out1", dagbase::PortType::TYPE_DOUBLE, dagbase::PortDirection::DIR_OUT),
    std::make_tuple("GroupTyped", 1, "in1", dagbase::PortType::TYPE_DOUBLE, dagbase::PortDirection::DIR_IN)
));

TEST(GraphTest, testTopologicalSortEmptyGraphReturnsEmptyArray)
{
    auto sut = new dagbase::Graph();

    dagbase::NodeArray actual;
    sut->topologicalSort(&actual);

    ASSERT_TRUE(actual.empty());

    delete sut;
}

bool before(const dagbase::NodeArray& a, dagbase::Node* first, dagbase::Node* last)
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
    auto sut = new dagbase::Graph();
    sut->setNodeLibrary(&nodeLib);
    auto a = dynamic_cast<dag::FooTyped*>(sut->createNode("FooTyped", "foo1"));
    auto b = dynamic_cast<dag::BarTyped*>(sut->createNode("BarTyped", "bar1"));
    auto t = b->out1()->connectTo(a->in1());
    sut->addNode(a);
    sut->addPort(&a->in1());
    auto path = new dagbase::SignalPath(b->out1(), &a->in1());
/*    path->removed = false;
    path->source.node = b->id();
    path->source.port = b->out1()->id();
    path->dest.node = a->id();
    path->dest.port = a->in1().id();*/
    sut->addSignalPath(path);
    sut->addNode(b);
    sut->addPort(b->out1());
    dagbase::NodeArray actual;
    auto result = sut->topologicalSort(&actual);
    ASSERT_EQ(dagbase::Graph::TopoSortResult::OK, result);
    ASSERT_EQ(size_t{2}, actual.size());
    ASSERT_TRUE(before(actual, b, a));

    delete t;
    delete sut;
}

TEST(GraphTest, testTopologicalSortTransitiveDependency)
{
    dag::MemoryNodeLibrary nodeLib;
    auto sut = new dagbase::Graph();
    sut->setNodeLibrary(&nodeLib);
    auto a = dynamic_cast<dag::GroupTyped*>(sut->createNode("GroupTyped", "foo1"));
    auto b = dynamic_cast<dag::GroupTyped*>(sut->createNode("GroupTyped", "bar1"));
    auto c = dynamic_cast<dag::GroupTyped*>(sut->createNode("GroupTyped", "baz1"));
    dagbase::Transfer* t1 = nullptr;
    dagbase::Transfer* t2 = nullptr;

    {
        t1 = a->out1().connectTo(b->in1());
        sut->addNode(a);
        sut->addPort(&a->in1());
        sut->addPort(&a->out1());
        auto path = new dagbase::SignalPath(&a->out1(), &b->in1());
        sut->addSignalPath(path);
    }
    {
        t2 = b->out1().connectTo(c->in1());
        sut->addNode(b);
        sut->addPort(&b->in1());
        sut->addPort(&b->out1());
        auto path = new dagbase::SignalPath(&b->out1(), &c->in1());
        sut->addSignalPath(path);
    }
    {
        sut->addNode(c);
        sut->addPort(&c->in1());
        sut->addPort(&c->out1());
    }
    dagbase::NodeArray actual;
    auto result = sut->topologicalSort(&actual);
    ASSERT_EQ(dagbase::Graph::TopoSortResult::OK, result);
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
    auto sut = new dagbase::Graph();
    sut->setNodeLibrary(&nodeLib);
    auto a = dynamic_cast<dag::GroupTyped*>(sut->createNode("GroupTyped", "foo1"));
    auto b = dynamic_cast<dag::GroupTyped*>(sut->createNode("GroupTyped", "bar1"));
    auto t1 = b->out1().connectTo(a->in1());
    sut->addNode(a);
    sut->addPort(&a->in1());
    sut->addPort(&a->out1());
    auto path = new dagbase::SignalPath(&b->out1(), &a->in1());
    sut->addSignalPath(path);
    auto t2 = a->out1().connectTo(b->in1());
    sut->addNode(b);
    sut->addPort(&b->out1());
    sut->addPort(&b->in1());
    path = new dagbase::SignalPath(&a->out1(), &b->in1());
    sut->addSignalPath(path);
    dagbase::NodeArray actual;
    auto result = sut->topologicalSort(&actual);
    ASSERT_EQ(dagbase::Graph::TopoSortResult::CYCLES_DETECTED, result);
    ASSERT_EQ(size_t{0}, actual.size());

    delete t2;
    delete t1;
    delete sut;
}

class TopologicalSort_testPersistent : public ::testing::TestWithParam<std::tuple<const char*, dagbase::Graph::TopoSortResult, std::size_t, const char*, const char*>>
{

};

TEST_P(TopologicalSort_testPersistent, testSort)
{
    const char* graphFilename = std::get<0>(GetParam());
    dagbase::Graph::TopoSortResult result = std::get<1>(GetParam());
    std::size_t numNodesInResult = std::get<2>(GetParam());
    std::string lhsPath = std::get<3>(GetParam());
    std::string rhsPath = std::get<4>(GetParam());

    dag::MemoryNodeLibrary nodeLib;
    auto sut = dagbase::Graph::fromFile(nodeLib, graphFilename);
    ASSERT_NE(nullptr, sut);
    dagbase::NodeArray order;
    dagbase::Graph::TopoSortResult actualResult = sut->topologicalSort(&order);
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
        std::make_tuple("etc/tests/Graph/empty.lua", dagbase::Graph::OK, std::size_t{0}, "", ""),
        std::make_tuple("etc/tests/Graph/onenode.lua", dagbase::Graph::OK, std::size_t{1}, "", ""),
        std::make_tuple("etc/tests/Graph/connectednodes.lua", dagbase::Graph::OK, std::size_t{2}, "bar1", "foo1"),
        std::make_tuple("etc/tests/Graph/withchildgraph.lua", dagbase::Graph::OK, std::size_t{2}, "", ""),
        std::make_tuple("etc/tests/Graph/withnestedchildgraph.lua", dagbase::Graph::OK, std::size_t{3}, "", ""),
        std::make_tuple("etc/tests/Graph/connectednestedchildgraph.lua", dagbase::Graph::OK, std::size_t{3}, "child[0].bar1", "child[0].child[0].bound1")
        ));

class Graph_testFindAllNodes : public ::testing::TestWithParam<std::tuple<const char*, std::size_t>>
{

};

TEST_P(Graph_testFindAllNodes, testFindAllNodes)
{
    auto graphFilename = std::get<0>(GetParam());
    std::size_t numNodes = std::get<1>(GetParam());
    dag::MemoryNodeLibrary nodeLib;
    auto sut = dagbase::Graph::fromFile(nodeLib, graphFilename);
    ASSERT_NE(nullptr, sut);
    dagbase::NodeArray actual;
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
    auto sut = dagbase::Graph::fromFile(nodeLib,graphFilename);
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

class Graph_testDeleteNode : public ::testing::TestWithParam<std::tuple<const char*, dagbase::NodeID, const char*, dagbase::Variant, double, dagbase::ConfigurationElement::RelOp>>
{

};

TEST_P(Graph_testDeleteNode, testExpectedValue)
{
    auto graphFilename = std::get<0>(GetParam());
    auto id = std::get<1>(GetParam());
    auto path = std::get<2>(GetParam());
    auto value = std::get<3>(GetParam());
    auto tolerance = std::get<4>(GetParam());
    auto op = std::get<5>(GetParam());

    dagbase::Lua lua;
    dag::MemoryNodeLibrary nodeLib;
    auto sut = dagbase::Graph::fromFile(nodeLib, graphFilename);
    ASSERT_NE(nullptr, sut);
    auto nodeToDelete = sut->node(id);
    sut->deleteNode(nodeToDelete);
    auto actual = sut->find(path);
    assertComparison(value, actual, tolerance, op);
}

INSTANTIATE_TEST_SUITE_P(Graph, Graph_testDeleteNode, ::testing::Values(
    std::make_tuple("etc/tests/Graph/onenode.lua", 0, "numNodes", std::uint32_t(0), 0.0, dagbase::ConfigurationElement::RELOP_EQ),
    std::make_tuple("etc/tests/Graph/onenode.lua", 1, "numNodes", std::uint32_t(1), 0.0, dagbase::ConfigurationElement::RELOP_EQ),
    std::make_tuple("etc/tests/Graph/connectednodes.lua", 0, "numNodes", std::uint32_t(1), 0.0, dagbase::ConfigurationElement::RELOP_EQ),
    std::make_tuple("etc/tests/Graph/connectednodes.lua", 0, "numSignalPaths", std::uint32_t(0), 0.0, dagbase::ConfigurationElement::RELOP_EQ),
    std::make_tuple("etc/tests/Graph/connectednodes.lua", 1, "numNodes", std::uint32_t(1), 0.0, dagbase::ConfigurationElement::RELOP_EQ),
    std::make_tuple("etc/tests/Graph/connectednodes.lua", 0, "numSignalPaths", std::uint32_t(0), 0.0, dagbase::ConfigurationElement::RELOP_EQ),
    std::make_tuple("etc/tests/Graph/connectednestedchildgraph.lua", dagbase::PortID::INVALID_ID, "totalSignalPaths", std::uint32_t(1), 0.0, dagbase::ConfigurationElement::RELOP_EQ),
    std::make_tuple("etc/tests/Graph/connectednestedchildgraph.lua", dagbase::PortID::INVALID_ID, "ports[2].numIncomingConnections", std::uint32_t(1), 0.0, dagbase::ConfigurationElement::RELOP_EQ),
    std::make_tuple("etc/tests/Graph/connectednestedchildgraph.lua", 1, "totalSignalPaths", std::uint32_t(0), 0.0, dagbase::ConfigurationElement::RELOP_EQ),
    std::make_tuple("etc/tests/Graph/connectednestedchildgraph.lua", 1, "ports[2].numIncomingConnections", std::uint32_t(0), 0.0, dagbase::ConfigurationElement::RELOP_EQ)
));

TEST(PortTest, testConnectToExistingPortGivesTransfer)
{
    auto source = new dagbase::TypedPort<double>(0, nullptr, "out", dagbase::PortType::TYPE_DOUBLE, dagbase::PortDirection::DIR_OUT, 1.0, dagbase::Port::OWN_META_PORT_BIT);
    auto dest = new dagbase::TypedPort<double>(1, nullptr, "in", dagbase::PortType::TYPE_DOUBLE, dagbase::PortDirection::DIR_IN, 0.0, dagbase::Port::OWN_META_PORT_BIT);
    auto* visitor = new dagbase::SetValueVisitor(dagbase::Value(2.0));
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
    auto source = new dagbase::TypedPort<double>(0, nullptr, "out", dagbase::PortType::TYPE_DOUBLE, dagbase::PortDirection::DIR_OUT, 1.0, dagbase::Port::OWN_META_PORT_BIT);
    auto dest = new dagbase::TypedPort<double>(1, nullptr, "in", dagbase::PortType::TYPE_DOUBLE, dagbase::PortDirection::DIR_IN, 0.0, dagbase::Port::OWN_META_PORT_BIT);
    auto transfer = source->connectTo(*dest);
    ASSERT_NE(nullptr, transfer);
    source->disconnect(*dest);
    ASSERT_FALSE(source->isConnectedTo(dest));
    ASSERT_FALSE(dest->isConnectedTo(source));
    auto visitor = new dagbase::ValueVisitor();
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
    auto node = nodeLib.instantiateNode(nodeLib, "FooTyped", "foo1");
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
    auto node = nodeLib.instantiateNode(nodeLib, "FooTyped", "foo1");
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
    auto node = nodeLib.instantiateNode(nodeLib, "FooTyped", "foo1");
    a.insert(node);
    sut->add(a.begin(), a.end());
    dag::SelectionInterface::Cont b;
    auto node2 = nodeLib.instantiateNode(nodeLib, "BarTyped", "bar1");
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
    auto node = nodeLib.instantiateNode(nodeLib, "FooTyped", "foo1");
    auto node2 = nodeLib.instantiateNode(nodeLib, "BarTyped", "bar1");
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

class NodeEditorLiveTest_testLoad : public ::testing::TestWithParam<std::tuple<const char*, dagbase::Status::StatusCode>>
{

};

TEST_P(NodeEditorLiveTest_testLoad, testExpectedStatus)
{
    auto filename = std::get<0>(GetParam());
    auto status = std::get<1>(GetParam());

    auto sut = std::make_unique<dag::NodeEditorLive>();
    auto actual = sut->load(filename);
    EXPECT_EQ(status, actual.status);
}

INSTANTIATE_TEST_SUITE_P(NodeEditorLive, NodeEditorLiveTest_testLoad, ::testing::Values(
    std::make_tuple("NotFound.lua", dagbase::Status::STATUS_FILE_NOT_FOUND),
    std::make_tuple("etc/tests/NodeEditorLive/SyntaxError.lua", dagbase::Status::STATUS_SYNTAX_ERROR),
    std::make_tuple("etc/tests/NodeEditorLive/FailedToCreateGraph.lua", dagbase::Status::STATUS_OBJECT_NOT_FOUND),
    std::make_tuple("etc/tests/Graph/onenode.lua", dagbase::Status::STATUS_OK)
));

class NodeEditorLiveTest_testCreateNodeIncrementsPortID : public ::testing::TestWithParam<std::tuple<const char*, dagbase::PortID>>
{

};

TEST_P(NodeEditorLiveTest_testCreateNodeIncrementsPortID, testExpectedPortID)
{
    dagbase::Lua lua;
    auto configFilename = std::get<0>(GetParam());
    auto portID = std::get<1>(GetParam());

    auto config = dagbase::ConfigurationElement::fromFile(lua, configFilename);
    ASSERT_NE(nullptr, config);

    dag::NodeEditorLive sut;
    config->eachChild([&sut](dagbase::ConfigurationElement& child) {
        std::string className;
        dagbase::ConfigurationElement::readConfig(child, "className", &className);
        std::string nodeName;
        dagbase::ConfigurationElement::readConfig(child, "nodeName", &nodeName);
        sut.createNode(className, nodeName);
        return true;
    });
    EXPECT_EQ(portID, sut.activeGraph()->nextPortID());
}

INSTANTIATE_TEST_SUITE_P(NodeEditorLiveTest, NodeEditorLiveTest_testCreateNodeIncrementsPortID, ::testing::Values(
    std::make_tuple("etc/tests/NodeEditorLive/OneFooTyped.lua", dagbase::PortID(1))
));

struct NodeEditorLiveSub
{
    std::uint32_t commandIndex{0};
    std::uint32_t assertionIndex{0};
    std::string name;
    dagbase::Variant value;

    void configure(dagbase::ConfigurationElement& config)
    {
        dagbase::ConfigurationElement::readConfig(config, "commandIndex", &commandIndex);
        dagbase::ConfigurationElement::readConfig(config, "assertionIndex", &assertionIndex);
        dagbase::ConfigurationElement::readConfig(config, "name", &name);
        dagbase::ConfigurationElement::readConfig(config, "value", &value);
    }
};

struct NodeEditorLiveCase
{
    std::string name;
    using SubArray = std::vector<NodeEditorLiveSub>;
    SubArray subs;
    SubArray assertions;

    void configure(dagbase::ConfigurationElement& config)
    {
        dagbase::ConfigurationElement::readConfig(config, "name", &name);
        dagbase::ConfigurationElement::readConfigVector(config, "subs", &subs);
        dagbase::ConfigurationElement::readConfigVector(config, "assertions", &assertions);
    }
};

struct NodeEditorLiveAssertion
{
    std::string path;
    dagbase::Variant value;
    double tolerance{ 0.0 };
    dagbase::ConfigurationElement::RelOp op{ dagbase::ConfigurationElement::RELOP_UNKNOWN };
    dagbase::Variant::Index typeIndex{ dagbase::Variant::TYPE_UNKNOWN };

    void configure(dagbase::ConfigurationElement& config)
    {
        dagbase::ConfigurationElement::readConfig(config, "path", &path);
        dagbase::ConfigurationElement::readConfig(config, "value", &value);
        dagbase::ConfigurationElement::readConfig<dagbase::Variant::Index>(config, "typeIndex", &dagbase::Variant::parseIndex, &typeIndex);
        if (typeIndex != dagbase::Variant::TYPE_UNKNOWN)
        {
            value = value.cast(typeIndex);
        }
        dagbase::ConfigurationElement::readConfig(config, "tolerance", &tolerance);
        dagbase::ConfigurationElement::readConfig<dagbase::ConfigurationElement::RelOp>(config, "op", &dagbase::ConfigurationElement::parseRelOp, &op);
    }

    void setValue(dagbase::Variant newValue)
    {
        if (typeIndex == dagbase::Variant::TYPE_UNKNOWN)
        {
            value = newValue;
        }
        else
        {
            value = newValue.cast(typeIndex);
        }
    }

    void makeItSo(dag::NodeEditorLive& sut, const std::string& cmd) const
    {
        auto actual = sut.find(path);
        assertComparison(value, actual, tolerance, op, (path + " " + cmd).c_str());
    }
};

struct NodeEditorLiveScriptItem
{
    enum Command : std::uint32_t
    {
        COMMAND_UNKNOWN,
        COMMAND_NOP,
        COMMAND_CREATE_NODE,
        COMMAND_CONNECT,
        COMMAND_DISCONNECT,
        COMMAND_SELECT,
        COMMAND_CREATE_CHILD,
        COMMAND_SET_ACTIVE_GRAPH,
        COMMAND_DELETE_NODE,
        COMMAND_COPY_NODE,
        COMMAND_COMPARE_NODES,
    };

    void configure(dagbase::ConfigurationElement& config)
    {
        dagbase::ConfigurationElement::readConfig<Command>(config, "cmd", &parseCommand, &cmd);

        switch (cmd)
        {
        case COMMAND_NOP:
            // No parameters.
            break;
        case COMMAND_CREATE_NODE:
            status.resultType = dagbase::Status::RESULT_NODE_ID;
            dagbase::ConfigurationElement::readConfig(config, "status", &status);
            dagbase::ConfigurationElement::readConfig(config, "nodeClass", &nodeClass);
            dagbase::ConfigurationElement::readConfig(config, "nodeName", &nodeName);
            break;
        case COMMAND_CONNECT:
            status.resultType = dagbase::Status::RESULT_SIGNAL_PATH_ID;
            dagbase::ConfigurationElement::readConfig(config, "status", &status);
            dagbase::ConfigurationElement::readConfig(config, "fromPort", &fromPort);
            dagbase::ConfigurationElement::readConfig(config, "toPort", &toPort);
            break;
        case COMMAND_DISCONNECT:
            dagbase::ConfigurationElement::readConfig(config, "status", &status);
            dagbase::ConfigurationElement::readConfig(config, "signalPath", &signalPath);
            dagbase::ConfigurationElement::readConfig(config, "fromPort", &fromPort);
            dagbase::ConfigurationElement::readConfig(config, "toPort", &toPort);

            break;
        case COMMAND_SELECT:
            dagbase::ConfigurationElement::readConfig(config, "status", &status);
            dagbase::ConfigurationElement::readConfig<dag::NodeEditorInterface::SelectionMode>(config, "selectionMode", &dag::NodeEditorInterface::parseSelectionMode, &selectionMode);
            if (auto element = config.findElement("selection"); element)
            {
                element->eachChild([this](dagbase::ConfigurationElement& child) {
                    if (child.value().index() == dagbase::Variant::TYPE_INTEGER)
                        selection.emplace_back(child.asInteger());

                    return true;
                    });
            }

            break;
        case COMMAND_CREATE_CHILD:
            dagbase::ConfigurationElement::readConfig(config, "status", &status);
            // No parameters.
            break;
        case COMMAND_SET_ACTIVE_GRAPH:
            dagbase::ConfigurationElement::readConfig(config, "status", &status);
            if (auto element = config.findElement("graphPath"); element)
            {
                element->eachChild([this](dagbase::ConfigurationElement& child) {
                    auto value = child.asInteger();

                    graphChildPath.emplace_back(value);

                    return true;
                });
            }
            break;
        case COMMAND_DELETE_NODE:
            dagbase::ConfigurationElement::readConfig(config, "status", &status);
            dagbase::ConfigurationElement::readConfig(config, "node", &node);
            break;
        case COMMAND_COPY_NODE:
            dagbase::ConfigurationElement::readConfig(config, "status", &status);
            dagbase::ConfigurationElement::readConfig(config, "node", &node);
            break;
        case COMMAND_COMPARE_NODES:
            dagbase::ConfigurationElement::readConfig(config, "status", &status);
            dagbase::ConfigurationElement::readConfig(config, "node", &node);
            dagbase::ConfigurationElement::readConfig(config, "otherNode", &otherNode);
            break;
        default:
            FAIL() << "Creating unknown command";
            break;
        }
        dagbase::ConfigurationElement::readConfigVector(config, "assertions", &assertions);
    }

    void makeItSo(dag::NodeEditorLive& sut, const std::string caseName)
    {
        done = true;
        dagbase::Status actualStatus{dagbase::Status::STATUS_UNKNOWN};

        switch (cmd)
        {
        case COMMAND_NOP:
            actualStatus = status;
            break;
        case COMMAND_CREATE_NODE:
        {
            actualStatus = sut.createNode(nodeClass, nodeName);
            break;
        }
        case COMMAND_CONNECT:
        {
            actualStatus = sut.connect(fromPort, toPort);
            break;
        }
        case COMMAND_DISCONNECT:
        {
            actualStatus = sut.disconnect(signalPath);
            break;
        }
        case COMMAND_SELECT:
        {
            dag::SelectionInterface::Cont a;
            for (auto id : selection)
            {
                if (auto node = sut.activeGraph()->node(id); node)
                {
                    a.emplace(node);
                }
            }
            actualStatus = sut.select(selectionMode, a);
            break;
        }
        case COMMAND_CREATE_CHILD:
        {
            actualStatus = sut.createChild();
            break;
        }
        case COMMAND_SET_ACTIVE_GRAPH:
            actualStatus = sut.setActiveGraph(graphChildPath);

            break;
        case COMMAND_DELETE_NODE:
            actualStatus = sut.deleteNode(node);

            break;
        case COMMAND_COPY_NODE:
        {
            actualStatus = sut.copyNodes();

            break;
        }
        case COMMAND_COMPARE_NODES:
        {
            actualStatus = sut.compareNodes();

            break;
        }
        default:
            done = true;
            FAIL() << "Got into an unhandled command " << commandToString(cmd);
            break;
        }

        ASSERT_EQ(status.status, actualStatus.status)  << caseName << ':' << commandToString(cmd) << ":Expected a status of " << dagbase::Status::statusCodeToString(status.status) << ", got " << dagbase::Status::statusCodeToString(actualStatus.status);
        ASSERT_EQ(status.resultType, actualStatus.resultType) << caseName << ':' << commandToString(cmd) << ":Expected a resultType of " << dagbase::Status::resultTypeToString(status.resultType) << ", got " << dagbase::Status::resultTypeToString(actualStatus.resultType);
        ASSERT_EQ(status.result, actualStatus.result) << caseName << ':' << commandToString(cmd);
        for (const auto& a : assertions)
        {
            a.makeItSo(sut, caseName + ':' + commandToString(cmd));
        }

        done = true;
    }

    Command cmd{ COMMAND_UNKNOWN };
    dagbase::Status status{dagbase::Status::STATUS_OK};
    using NodeIDArray = std::vector<dagbase::NodeID>;
    NodeIDArray selection;
    using AssertionArray = std::vector<NodeEditorLiveAssertion>;
    AssertionArray assertions;
    std::string nodeClass;
    std::string nodeName;
    dagbase::PortID fromPort{ dagbase::PortID::INVALID_ID };
    dagbase::PortID toPort{ dagbase::PortID::INVALID_ID };
    dagbase::SignalPathID signalPath{ dagbase::SignalPathID::INVALID_ID };
    dagbase::NodeID node;
    dagbase::NodeID otherNode;
    dag::NodeEditorInterface::SelectionMode selectionMode{ dag::NodeEditorInterface::SELECTION_UNKNOWN };
    dag::NodeEditorLive::GraphChildPath graphChildPath;
    bool done{ false };

    void set(std::string_view name, dagbase::Variant value)
    {
        if (name == "nodeClass")
        {
            nodeClass = value.asString();
        }
        else if (name == "nodeName")
        {
            nodeName = value.asString();
        }
        else if (name == "fromPort")
        {
            fromPort = value.asInteger();
        }
        else if (name == "toPort")
        {
            toPort = value.asInteger();
        }
        else if (name == "signalPath")
        {
            signalPath = value.asInteger();
        }
        else if (name == "node")
        {
            node = value.asInteger();
        }
        else if (name == "otherNode")
        {
            otherNode = value.asInteger();
        }
        else if (name == "selectionMode")
        {
            selectionMode = static_cast<dag::NodeEditorInterface::SelectionMode>(value.asUint32());
        }
    }

    static const char* commandToString(Command value)
    {
        switch (value)
        {
            ENUM_NAME(COMMAND_UNKNOWN)
            ENUM_NAME(COMMAND_NOP)
            ENUM_NAME(COMMAND_CREATE_NODE)
            ENUM_NAME(COMMAND_CONNECT)
            ENUM_NAME(COMMAND_DISCONNECT)
            ENUM_NAME(COMMAND_SELECT)
            ENUM_NAME(COMMAND_CREATE_CHILD)
            ENUM_NAME(COMMAND_SET_ACTIVE_GRAPH)
            ENUM_NAME(COMMAND_DELETE_NODE)
            ENUM_NAME(COMMAND_COPY_NODE)
            ENUM_NAME(COMMAND_COMPARE_NODES)
        }

        return "<error>";
    }

    static Command parseCommand(const char* str)
    {
        TEST_ENUM(COMMAND_UNKNOWN, str);
        TEST_ENUM(COMMAND_NOP, str);
        TEST_ENUM(COMMAND_CREATE_NODE, str);
        TEST_ENUM(COMMAND_CONNECT, str);
        TEST_ENUM(COMMAND_DISCONNECT, str);
        TEST_ENUM(COMMAND_SELECT, str);
        TEST_ENUM(COMMAND_CREATE_CHILD, str);
        TEST_ENUM(COMMAND_SET_ACTIVE_GRAPH, str);
        TEST_ENUM(COMMAND_DELETE_NODE, str);
        TEST_ENUM(COMMAND_COPY_NODE, str);
        TEST_ENUM(COMMAND_COMPARE_NODES, str);

        return COMMAND_UNKNOWN;
    }
};

class NodeEditorLiveScript
{
public:
    void configure(dagbase::ConfigurationElement& config)
    {
        dagbase::ConfigurationElement::readConfigVector(config, "cases", &_cases);
        dagbase::ConfigurationElement::readConfigVector(config, "items", &_items);
    }

    void setUp()
    {
        dagbase::SignalPath::resetID();
        _sut = new dag::NodeEditorLive();
    }

    void tearDown()
    {
        delete _sut;
    }

    void makeItSo();
private:
    using CaseArray = std::vector<NodeEditorLiveCase>;
    CaseArray _cases;
    using ItemArray = std::vector<NodeEditorLiveScriptItem>;
    ItemArray _items;
    dag::NodeEditorLive* _sut{nullptr};
};

void NodeEditorLiveScript::makeItSo()
{
    if (!_cases.empty())
    {

        for (const auto& currentCase : _cases)
        {
            setUp();
            // Make the substitutions
            for (const auto& sub : currentCase.subs)
            {
                _items[sub.commandIndex].set(sub.name, sub.value);
            }

            for (const auto& assertion : currentCase.assertions)
            {
                _items[assertion.commandIndex].assertions[assertion.assertionIndex].setValue(assertion.value);
            }

            for (auto item : _items)
            {
                item.makeItSo(*_sut, currentCase.name);
            }
            tearDown();
        }
    }
    else
    {
        setUp();
        // Original pattern with no explicit cases, just an implicit one with no substitutions
        for (auto item : _items)
        {
            item.makeItSo(*_sut, "<implicit>");
        }
        tearDown();
    }

        // _currentItem->makeItSo(sut);
        // if (_currentItem->done)
        // {
        //     ++_currentItem;
        // }
}

class NodeEditorLive_testScripted : public ::testing::TestWithParam<std::tuple<const char*>>
{
public:
    void SetUp() override;
    void TearDown() override;
protected:
    NodeEditorLiveScript _script;
    dag::NodeEditorLive* _sut{ nullptr };
};

void NodeEditorLive_testScripted::SetUp()
{
    auto scriptFilename = std::get<0>(GetParam());
    dagbase::Lua lua;
    auto scriptConfig = dagbase::ConfigurationElement::fromFile(lua, scriptFilename);
    ASSERT_NE(nullptr, scriptConfig);
    _script.configure(*scriptConfig);
    delete scriptConfig;
}

void NodeEditorLive_testScripted::TearDown()
{
}

TEST_P(NodeEditorLive_testScripted, testExpectedValue)
{
    //auto graphFilename = std::get<1>(GetParam());
    //dagbase::Lua lua;
    //auto graphConfig = dagbase::ConfigurationElement::fromFile(lua, graphFilename);
    //ASSERT_NE(nullptr, graphConfig);
    _script.makeItSo();
}

INSTANTIATE_TEST_SUITE_P(NodeEditorLive, NodeEditorLive_testScripted, ::testing::Values(
    std::make_tuple("etc/tests/NodeEditorLive/CloneSimple.lua"),
    std::make_tuple("etc/tests/NodeEditorLive/CloneConnected.lua"),
    std::make_tuple("etc/tests/NodeEditorLive/CreateChild.lua"),
    std::make_tuple("etc/tests/NodeEditorLive/CreateChildFromEmptySelection.lua"),
    std::make_tuple("etc/tests/NodeEditorLive/ConnectThenDisconnect.lua"),
    std::make_tuple("etc/tests/NodeEditorLive/ConnectToIncompatible.lua"),
    std::make_tuple("etc/tests/NodeEditorLive/ConnectToSelf.lua"),
    std::make_tuple("etc/tests/NodeEditorLive/ConnectSwappedPorts.lua"),
    std::make_tuple("etc/tests/NodeEditorLive/ConnectInvalidFrom.lua"),
    std::make_tuple("etc/tests/NodeEditorLive/ConnectInvalidTo.lua"),
    std::make_tuple("etc/tests/NodeEditorLive/DisconnectInvalidSignalPathID.lua"),
    std::make_tuple("etc/tests/NodeEditorLive/CreateChildInNonRootGraph.lua"),
    std::make_tuple("etc/tests/NodeEditorLive/SelectMultipleInternals.lua"),
    std::make_tuple("etc/tests/NodeEditorLive/SelectMultipleWithAnOnlyInternal.lua"),
    std::make_tuple("etc/tests/NodeEditorLive/CreateChildFromAnOnlyInternal.lua"),
    std::make_tuple("etc/tests/NodeEditorLive/EmptySelection.lua"),
    std::make_tuple("etc/tests/NodeEditorLive/SelectionSetEmpty.lua"),
    std::make_tuple("etc/tests/NodeEditorLive/SelectionSetOne.lua"),
    std::make_tuple("etc/tests/NodeEditorLive/SelectionReplaceOne.lua"),
    std::make_tuple("etc/tests/NodeEditorLive/SelectionToggle.lua"),
    std::make_tuple("etc/tests/NodeEditorLive/SelectionAdd.lua"),
    std::make_tuple("etc/tests/NodeEditorLive/SelectionSubtract.lua"),
    std::make_tuple("etc/tests/NodeEditorLive/SelectionClear.lua"),
    std::make_tuple("etc/tests/NodeEditorLive/SelectionAll.lua"),
    std::make_tuple("etc/tests/NodeEditorLive/SetActiveGraphInvalidPath.lua"),
    std::make_tuple("etc/tests/NodeEditorLive/DeleteValid.lua"),
    std::make_tuple("etc/tests/NodeEditorLive/DeleteInvalid.lua")
));

TEST(BoundaryNode, testAddDynamicPort)
{
    dag::MemoryNodeLibrary nodeLib;
    auto sut = new dag::Boundary(nodeLib, "sut", dagbase::NodeCategory::CAT_SOURCE);
    auto input = new dagbase::TypedPort<double>(0, nullptr, "input1", dagbase::PortType::TYPE_DOUBLE, dagbase::PortDirection::DIR_IN, 1.0, dagbase::Port::OWN_META_PORT_BIT);
    ASSERT_NO_THROW(sut->addDynamicPort(input, dagbase::MetaPort::FLAGS_OWN_BIT));
    ASSERT_EQ(sut, input->parent());
    ASSERT_EQ(size_t{1}, sut->totalPorts());
    ASSERT_EQ(input, sut->dynamicPort(0));
    // input now owned by sut
    delete sut;
}

TEST(BoundaryNode, testClone)
{
    dag::MemoryNodeLibrary nodeLib;
    auto sut = new dag::Boundary(nodeLib, "sut", dagbase::NodeCategory::CAT_SOURCE);
    auto metaPort = new dagbase::MetaPort();
    auto input = new dagbase::TypedPort<double>(0, nullptr, "input1", dagbase::PortType::TYPE_DOUBLE, dagbase::PortDirection::DIR_IN, 1.0, dagbase::Port::OWN_META_PORT_BIT);
    ASSERT_NO_THROW(sut->addDynamicPort(input, dagbase::MetaPort::FLAGS_OWN_BIT));
    dagbase::CloningFacility facility;
    auto clone = sut->clone(facility, dagbase::CopyOp{0}, nullptr);
    ASSERT_EQ(size_t{1},sut->totalPorts());
    ASSERT_EQ(sut->totalPorts(), clone->totalPorts());
    ASSERT_NE(sut->dynamicPort(0), clone->dynamicPort(0));
    ASSERT_EQ(sut, sut->dynamicPort(0)->parent());
    ASSERT_EQ(clone, clone->dynamicPort(0)->parent());
    delete clone;
    delete sut;
}

TEST(GraphTest, testCreateNode)
{
    auto sut = new dagbase::Graph();
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
    auto graph = new dagbase::Graph();
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
    auto sut = new dagbase::Graph();
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
    sut.flip();
    sut.get(&f);
    ASSERT_EQ(101.234f, f);
}

TEST(ByteBufferTest, testRelativeInt)
{
    dagbase::ByteBuffer sut;

    sut.put(101);
    std::int32_t f = 0;
    sut.flip();
    sut.get(&f);
    ASSERT_EQ(101, f);
}

TEST(ByteBufferTest, testRelativeDouble)
{
    dagbase::ByteBuffer sut;

    sut.put(101.234);
    double f = 0;
    sut.flip();
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
    sut.flip();
    sut.get(&actualValue);
    ASSERT_EQ(value, actualValue);
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

TEST(ByteBufferTest, testBulkGet)
{
    dagbase::ByteBuffer sut;
    int i=100;
    sut.put(i);
    int actual=0;
    sut.flip();
    sut.get(reinterpret_cast<dagbase::ByteBuffer::BufferType::value_type*>(&actual), sizeof(int));
    ASSERT_EQ(i,actual);
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
        auto parentRef = str.readRef<TestNode>(&parentId);

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
            _children.emplace_back(child);
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

class Graph_testSerialisation : public ::testing::TestWithParam<std::tuple<std::string_view, const char*>>
{

};

TEST_P(Graph_testSerialisation, testRoundTrip)
{
    dagbase::Lua lua;
    auto formatClassName = std::get<0>(GetParam());
    auto graphFilename = std::get<1>(GetParam());
    //dagbase::StreamFormat* format = nullptr;
    dagbase::MemoryBackingStore store(dagbase::BackingStore::MODE_OUTPUT_BIT);
    dagbase::OutputStream* sut  = nullptr;
    if (formatClassName == "TextFormat")
    {
        sut = new dagbase::TextOutputStream(&store);
    }
    else if (formatClassName == "BinaryFormat")
    {
        sut = new dagbase::BinaryOutputStream(&store);
    }
    ASSERT_NE(nullptr, sut);
    dag::NodePluginScanner scanner;
    dag::MemoryNodeLibrary nodeLib;

    scanner.scan(nodeLib, nodeLib);

    auto g1 = dagbase::Graph::fromFile(nodeLib, graphFilename);

    if (sut->writeRef(g1))
    {
        g1->write(*sut, nodeLib, lua);
    }
    sut->flush();
    store.open(dagbase::BackingStore::MODE_INPUT_BIT);
    dagbase::InputStream* in = nullptr;
    if (formatClassName == "TextFormat")
    {
        in = new dagbase::TextInputStream(&store);
    }
    else if (formatClassName == "BinaryFormat")
    {
        in = new dagbase::BinaryInputStream(&store);
    }
    ASSERT_NE(nullptr, in);

    dagbase::Stream::ObjId id = 0;
    dagbase::Graph* g2 = nullptr;
    dagbase::Stream::Ref ref = in->readRef(&id);
    if (id != 0)
    {
        if (ref != nullptr)
        {
            g2 = static_cast<dagbase::Graph*>(ref);
        }
        else
        {
            g2 = new dagbase::Graph(*in, nodeLib, lua);
        }
    }

    ASSERT_NE(nullptr, g2);
    ASSERT_EQ(*g1, *g2);
    delete g2;
    delete in;
    delete sut;
    delete g1;
}

INSTANTIATE_TEST_SUITE_P(Graph, Graph_testSerialisation, ::testing::Values(
    std::make_tuple("TextFormat", "etc/tests/Graph/empty.lua"),
    std::make_tuple("BinaryFormat", "etc/tests/Graph/empty.lua"),
    std::make_tuple("TextFormat", "etc/tests/Graph/onenode.lua"),
    std::make_tuple("BinaryFormat", "etc/tests/Graph/onenode.lua"),
    std::make_tuple("TextFormat", "etc/tests/Graph/connectednodes.lua"),
    std::make_tuple("BinaryFormat", "etc/tests/Graph/connectednodes.lua"),
    std::make_tuple("TextFormat", "etc/tests/Graph/withchildgraph.lua"),
    std::make_tuple("BinaryFormat", "etc/tests/Graph/withchildgraph.lua"),
    std::make_tuple("TextFormat", "etc/tests/Graph/withmultiplechildren.lua"),
    std::make_tuple("BinaryFormat", "etc/tests/Graph/withmultiplechildren.lua"),
    std::make_tuple("TextFormat", "etc/tests/Graph/withnestedchildgraph.lua"),
    std::make_tuple("BinaryFormat", "etc/tests/Graph/withnestedchildgraph.lua"),
    std::make_tuple("TextFormat", "etc/tests/Graph/nodesFromPlugin.lua"),
    std::make_tuple("BinaryFormat", "etc/tests/Graph/nodesFromPlugin.lua")
    ));

class GraphTest_fromLua : public ::testing::TestWithParam<std::tuple<const char*, std::size_t, std::size_t, dagbase::NodeID, std::size_t, dagbase::Value>>
{

};

TEST_P(GraphTest_fromLua, testFromString)
{
    const char* graphStr = std::get<0>(GetParam());
    std::size_t numNodes = std::get<1>(GetParam());
    std::size_t numSignalPaths = std::get<2>(GetParam());
    dagbase::NodeID nodeId = std::get<3>(GetParam());
    std::size_t portIndex = std::get<4>(GetParam());
    dagbase::Value value = std::get<5>(GetParam());

    dag::MemoryNodeLibrary nodeLib;
    auto sut = dagbase::Graph::fromString(nodeLib, graphStr);
    ASSERT_NE(nullptr, sut);
    EXPECT_EQ(numNodes, sut->numNodes());
    EXPECT_EQ(numSignalPaths, sut->numSignalPaths());
    auto actualNode = sut->lastAddedNode();
    ASSERT_NE(nullptr, actualNode);
    auto actualPort = actualNode->dynamicPort(portIndex);
    ASSERT_NE(nullptr, actualPort);
    dagbase::ValueVisitor valueVisitor;
    actualPort->accept(valueVisitor);
    dagbase::Value actualValue = valueVisitor.value();

    EXPECT_EQ(value, actualValue);
    delete sut;
}

INSTANTIATE_TEST_SUITE_P(GraphTest, GraphTest_fromLua, ::testing::Values(
    std::make_tuple("graph={ nodes={ { name=\"foo\", class=\"FooTyped\", category=\"CATEGORY_SINK\", ports={ { name=\"in1\", class=\"TypedPort<double>\", type=\"TYPE_DOUBLE\", dir=\"DIR_IN\", value=2.0 } } } } }", std::size_t{ 1 }, std::size_t{ 0 }, dagbase::NodeID{ 0 }, std::size_t{ 0 }, dagbase::Value{2.0} ),
    std::make_tuple("graph={ nodes={ { name=\"foo\", class=\"Boundary\", category=\"CATEGORY_GROUP\", ports={ { name=\"in1\", class=\"TypedPort<double>\", type=\"TYPE_DOUBLE\", dir=\"DIR_IN\", value=2.0 } } } } }", std::size_t{ 1 }, std::size_t{ 0 }, dagbase::NodeID{ 0 }, std::size_t{ 0 }, dagbase::Value{2.0} ),
    std::make_tuple("graph={ nodes={ { name=\"foo\", class=\"Boundary\", category=\"CATEGORY_GROUP\", ports={ { name=\"in1\", class=\"TypedPort<int64_t>\", type=\"TYPE_INTEGER\", dir=\"DIR_IN\", value=2 } } } } }", std::size_t{ 1 }, std::size_t{ 0 }, dagbase::NodeID{ 0 }, std::size_t{ 0 }, dagbase::Value{std::int64_t(2)} ),
    std::make_tuple("graph={ nodes={ { name=\"foo\", class=\"Boundary\", category=\"CATEGORY_GROUP\", ports={ { name=\"in1\", class=\"TypedPort<string>\", type=\"TYPE_STRING\", dir=\"DIR_IN\", value=\"wibble\" } } } } }", std::size_t{ 1 }, std::size_t{ 0 }, dagbase::NodeID{ 0 }, std::size_t{ 0 }, dagbase::Value{std::string("wibble")} ),
    std::make_tuple("graph={ nodes={ { name=\"foo\", class=\"Boundary\", category=\"CATEGORY_GROUP\", ports={ { name=\"in1\", class=\"TypedPort<bool>\", type=\"TYPE_BOOL\", dir=\"DIR_IN\", value=true } } } } }", std::size_t{ 1 }, std::size_t{ 0 }, dagbase::NodeID{ 0 }, std::size_t{ 0 }, dagbase::Value{true} )
));

class GraphTest_fromLuaFile : public ::testing::TestWithParam<std::tuple<const char*, std::size_t, std::size_t, dagbase::NodeID, std::size_t, dagbase::Value, std::size_t, std::size_t, std::size_t>>
{

};

TEST_P(GraphTest_fromLuaFile, testFromFile)
{
    const char* graphStr = std::get<0>(GetParam());
    std::size_t numNodes = std::get<1>(GetParam());
    std::size_t numSignalPaths = std::get<2>(GetParam());
    dagbase::NodeID nodeId = std::get<3>(GetParam());
    std::size_t portIndex = std::get<4>(GetParam());
    dagbase::Value value = std::get<5>(GetParam());
    std::size_t numIncomingPorts = std::get<6>(GetParam());
    std::size_t numOutgoingPorts = std::get<7>(GetParam());
    std::size_t numChildren = std::get<8>(GetParam());

    dag::MemoryNodeLibrary nodeLib;
    auto sut = dagbase::Graph::fromFile(nodeLib, graphStr);
    ASSERT_NE(nullptr, sut);
    EXPECT_EQ(numNodes, sut->numNodes());
    EXPECT_EQ(numSignalPaths, sut->numSignalPaths());
    auto actualNode = sut->node(nodeId);
    ASSERT_NE(nullptr, actualNode);
    auto actualPort = actualNode->dynamicPort(portIndex);
    ASSERT_NE(nullptr, actualPort);
    dagbase::ValueVisitor valueVisitor;
    actualPort->accept(valueVisitor);
    dagbase::Value actualValue = valueVisitor.value();
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
    std::make_tuple("etc/tests/Graph/onenode.lua", std::size_t{ 1 }, std::size_t{ 0 }, dagbase::NodeID{ 0 }, std::size_t{ 0 }, dagbase::Value{2.0}, std::size_t{0}, std::size_t{0}, std::size_t{0} ),
    std::make_tuple("etc/tests/Graph/connectednodes.lua", std::size_t{ 2 }, std::size_t{ 1 }, dagbase::NodeID{ 0 }, std::size_t{ 0 }, dagbase::Value{1.0}, std::size_t{1}, std::size_t{0}, std::size_t{ 0 }),
    std::make_tuple("etc/tests/Graph/withchildgraph.lua", std::size_t{ 1 }, std::size_t{ 0 }, dagbase::NodeID{ 0 }, std::size_t{ 0 }, dagbase::Value{2.0}, std::size_t{0}, std::size_t{0}, std::size_t{ 1 }),
    std::make_tuple("etc/tests/Graph/withnestedchildgraph.lua", std::size_t{ 1 }, std::size_t{ 0 }, dagbase::NodeID{ 0 }, std::size_t{ 0 }, dagbase::Value{2.0}, std::size_t{0}, std::size_t{0}, std::size_t{ 2 })
));

class GraphTest_toLuaRoundTrip : public ::testing::TestWithParam<std::tuple<const char*>>
{

};

TEST_P(GraphTest_toLuaRoundTrip, testRoundTrip)
{
    auto graphFilename = std::get<0>(GetParam());
    dag::MemoryNodeLibrary nodeLib;
    auto sut = dagbase::Graph::fromFile(nodeLib, graphFilename);
    ASSERT_NE(nullptr, sut);
    std::string graphString;
    std::ostringstream graphStr;
    sut->toLua(graphStr);
    graphString = graphStr.str();
    std::cerr << graphString << '\n';
    auto actual = dagbase::Graph::fromString(nodeLib, graphString.c_str());
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

class PortDirection_testParseFromString : public ::testing::TestWithParam<std::tuple<const char*, dagbase::PortDirection::Direction>>
{

};

TEST_P(PortDirection_testParseFromString, testParseFromString)
{
    const char* str = std::get<0>(GetParam());
    dagbase::PortDirection::Direction dir = std::get<1>(GetParam());
    auto actualDir = dagbase::PortDirection::parseFromString(str);
    EXPECT_EQ(dir, actualDir);
}

INSTANTIATE_TEST_SUITE_P(PortDirection, PortDirection_testParseFromString, ::testing::Values(
    std::make_tuple("DIR_UNKNOWN", dagbase::PortDirection::DIR_UNKNOWN),
    std::make_tuple("DIR_IN", dagbase::PortDirection::DIR_IN),
    std::make_tuple("DIR_OUT", dagbase::PortDirection::DIR_OUT),
    std::make_tuple("DIR_INTERNAL", dagbase::PortDirection::DIR_INTERNAL)

));

class PortDirection_testToString : public ::testing::TestWithParam<std::tuple<const char*, dagbase::PortDirection::Direction>>
{

};

TEST_P(PortDirection_testToString, testParseFromString)
{
    std::string str = std::get<0>(GetParam());
    dagbase::PortDirection::Direction dir = std::get<1>(GetParam());
    auto actualStr = dagbase::PortDirection::toString(dir);
    EXPECT_EQ(str, actualStr);
}

INSTANTIATE_TEST_SUITE_P(PortDirection, PortDirection_testToString, ::testing::Values(
    std::make_tuple("DIR_UNKNOWN", dagbase::PortDirection::DIR_UNKNOWN),
    std::make_tuple("DIR_IN", dagbase::PortDirection::DIR_IN),
    std::make_tuple("DIR_OUT", dagbase::PortDirection::DIR_OUT),
    std::make_tuple("DIR_INTERNAL", dagbase::PortDirection::DIR_INTERNAL)

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
    auto existingNode = nodeLib.instantiateNode(nodeLib, "FooTyped", "foo1");
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
    dagbase::Graph* graph = new dagbase::Graph();
    auto node = nodeLib.instantiateNode(*graph, "NodePlugin.DynamicNode", "node1");
    ASSERT_NE(nullptr, node);
    graph->setNodeLibrary(&nodeLib);
    graph->addNode(node);
    ASSERT_EQ(size_t{1}, graph->numNodes());
    std::ostringstream  str;
    graph->toLua(str);
    auto graph2 = dagbase::Graph::fromString(nodeLib, str.str().c_str());
    EXPECT_EQ(*graph,*graph2);
    delete graph2;
    delete graph;
}

TEST(GraphTest, testLoadGraphWithNodesFromPlugin)
{
    dag::NodePluginScanner scanner;
    dag::MemoryNodeLibrary nodeLib;

    scanner.scan(nodeLib, nodeLib);
    auto graph = dagbase::Graph::fromFile(nodeLib, "etc/tests/Graph/nodesFromPlugin.lua");
    ASSERT_NE(nullptr, graph);
    auto node = graph->node(0);
    ASSERT_NE(nullptr, node);
    dagbase::TypedPort<double>* port = dynamic_cast<dagbase::TypedPort<double>*>(node->dynamicPort(0));
    ASSERT_NE(nullptr, port);
    EXPECT_EQ(1.0, port->value());
    delete graph;
}

class GraphTest_testReadFromLuaThenSerialise : public ::testing::TestWithParam<std::tuple<const char*>>
{

};

TEST_P(GraphTest_testReadFromLuaThenSerialise, testSerialise)
{
    dagbase::Lua lua;
    const char* graphFilename = std::get<0>(GetParam());
    dag::MemoryNodeLibrary nodeLib;
    auto sut = dagbase::Graph::fromFile(nodeLib, graphFilename);
    ASSERT_NE(nullptr, sut);
    auto* buf = new dagbase::MemoryBackingStore(dagbase::BackingStore::MODE_OUTPUT_BIT);
    dagbase::BinaryOutputStream ostr(buf);
    if (ostr.writeRef(sut))
    {
        sut->write(ostr, nodeLib, lua);
    }
    ostr.flush();
    buf->open(dagbase::BackingStore::MODE_INPUT_BIT);
    dagbase::BinaryInputStream istr(buf);
    dagbase::Stream::ObjId id{~0U};
    dagbase::Graph* actual = nullptr;
    dagbase::Stream::Ref ref = istr.readRef(&id);
    if (id != 0)
    {
        if (ref != nullptr)
        {
            actual = static_cast<dagbase::Graph*>(ref);
        }
        else
        {
            actual = new dagbase::Graph(istr, nodeLib, lua);
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

class GraphTest_testEvaluate : public ::testing::TestWithParam<std::tuple<const char*, dagbase::NodeID, std::size_t, double>>
{
protected:
    void TearDown() override
    {
        delete _sut;
    }
    dag::MemoryNodeLibrary _nodeLib;
    dagbase::Graph* _sut{nullptr};
};

TEST_P(GraphTest_testEvaluate, testEvaluate)
{
    const char* graphFilename = std::get<0>(GetParam());
    dagbase::NodeID nodeId = std::get<1>(GetParam());
    std::size_t portIndex = std::get<2>(GetParam());
    double value = std::get<3>(GetParam());

    _sut = dagbase::Graph::fromFile(_nodeLib, graphFilename);
    ASSERT_NE(nullptr, _sut);
    dagbase::NodeArray order;
    _sut->topologicalSort(&order);
    _sut->evaluate(order);
    dagbase::Node* actualNode = _sut->node(nodeId);
    ASSERT_NE(nullptr, actualNode);
    dagbase::Port* actualPort = actualNode->dynamicPort(portIndex);
    ASSERT_NE(nullptr, actualPort);
    dagbase::ValueVisitor visitor;
    ASSERT_EQ(dagbase::PortType::TYPE_DOUBLE, actualPort->type());
    actualPort->accept(visitor);
    EXPECT_EQ(value, visitor.value().operator double());
}

INSTANTIATE_TEST_SUITE_P(Graph, GraphTest_testEvaluate, ::testing::Values(
        std::make_tuple("etc/tests/Graph/constraints.lua", 0, 2, 1.0)
        ));

class Graph_copy : public ::testing::TestWithParam<std::tuple<const char*, dagbase::CopyOp, bool>>
{

};

TEST_P(Graph_copy, testCopy)
{
    const char* filename = std::get<0>(GetParam());
    dagbase::CopyOp copyOp = std::get<1>(GetParam());
    bool equal = std::get<2>(GetParam());
    dag::MemoryNodeLibrary nodeLib;
    auto sut = dagbase::Graph::fromFile(nodeLib, filename);
    ASSERT_NE(nullptr, sut);
    dagbase::CloningFacility facility;
    auto copy = sut->clone(facility, copyOp, &nodeLib);
    ASSERT_NE(nullptr, copy);
    EXPECT_EQ(equal, *copy == *sut);
    delete copy;
    delete sut;
}

INSTANTIATE_TEST_SUITE_P(Graph, Graph_copy, ::testing::Values(
//        std::make_tuple("etc/tests/Graph/empty.lua", dagbase::CopyOp::GENERATE_UNIQUE_ID_BIT, true),
//        std::make_tuple("etc/tests/Graph/onenode.lua", dagbase::CopyOp{0}, true),
//        std::make_tuple("etc/tests/Graph/onenode.lua", dagbase::CopyOp::GENERATE_UNIQUE_ID_BIT, false),
        std::make_tuple("etc/tests/Graph/connectednodes.lua", dagbase::CopyOp{dagbase::CopyOp::DEEP_COPY_INPUTS_BIT|dagbase::CopyOp::DEEP_COPY_OUTPUTS_BIT}, true)
//        std::make_tuple("etc/tests/Graph/connectednodes.lua", dagbase::CopyOp{dagbase::CopyOp::DEEP_COPY_INPUTS_BIT|dagbase::CopyOp::DEEP_COPY_OUTPUTS_BIT|dagbase::CopyOp::GENERATE_UNIQUE_ID_BIT}, false),
//        std::make_tuple("etc/tests/Graph/withchildgraph.lua", dagbase::CopyOp{dagbase::CopyOp::DEEP_COPY_INPUTS_BIT|dagbase::CopyOp::DEEP_COPY_OUTPUTS_BIT}, true),
//        std::make_tuple("etc/tests/Graph/withmultiplechildren.lua", dagbase::CopyOp{dagbase::CopyOp::DEEP_COPY_INPUTS_BIT|dagbase::CopyOp::DEEP_COPY_OUTPUTS_BIT}, true),
//        std::make_tuple("etc/tests/Graph/withnestedchildgraph.lua", dagbase::CopyOp{dagbase::CopyOp::DEEP_COPY_INPUTS_BIT|dagbase::CopyOp::DEEP_COPY_OUTPUTS_BIT}, true),
//        std::make_tuple("etc/tests/Graph/connectednestedchildgraph.lua", dagbase::CopyOp{dagbase::CopyOp::DEEP_COPY_INPUTS_BIT|dagbase::CopyOp::DEEP_COPY_OUTPUTS_BIT}, true)
        ));

TEST(CloningFacility, testPutNull)
{
    dagbase::CloningFacility sut;
    std::uint64_t id = std::uint64_t {~0U};
    sut.putOrig(nullptr, &id);
    EXPECT_EQ(std::uint64_t {0}, id);
    EXPECT_EQ(std::size_t{0}, sut.numClones());
}

TEST(CloningFacility, testPutValid)
{
    dagbase::CloningFacility sut;

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
    TestLink(const TestLink& other, dagbase::CloningFacility& facility);
};

TestLink::TestLink(const TestLink &other, dagbase::CloningFacility &facility)
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
    dagbase::CloningFacility sut;
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

