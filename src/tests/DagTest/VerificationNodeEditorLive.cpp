//
// Created by Tony Horrobin on 14/11/2026.
//

#include "io/StreamFactory.h"
#include "io/BackingStore.h"
#include "VerificationNodeEditorLive.h"

#include "MemoryNodeLibrary.h"
#include "test/TestUtils.h"

VerificationNodeEditorLive::VerificationNodeEditorLive(NodeEditorCommand *decoratee)
    :
    NodeEditorCommand(decoratee)
{
    // Do nothing.
}

void VerificationNodeEditorLive::configure(dagbase::ConfigurationElement &config)
{
    if (decoratee())
        decoratee()->configure(config);

    dagbase::ConfigurationElement::readConfigVector(config, "existing", &existing);
    dagbase::ConfigurationElement::readConfigVector(config, "assertions", &assertions);
    dagbase::ConfigurationElement::readConfig(config, "status", &status);
    dagbase::ConfigurationElement::readConfig<Command>(config, "cmd", &parseCommand, &cmd);

    switch (cmd)
    {
    case COMMAND_NOP:
        // No parameters.
        break;
    case COMMAND_UNKNOWN:
        FAIL() << "Creating unknown command";
        break;
    default:
        break;
    }
}

dagbase::Status VerificationNodeEditorLive::makeItSo(const std::pmr::vector<dagbase::Variant> &args,
    std::pmr::vector<dagbase::Variant> *results)
{
    actualStatus = decoratee()->makeItSo(args, results);

    verify();

    for (const auto& a : existing)
    {
        a.makeItSo(*nodeEditor());
    }
    for (const auto& a : assertions)
    {
        a.makeItSo(*nodeEditor(), _caseName + ':' + commandToString(cmd));
    }

    return actualStatus;
}

void VerificationNodeEditorLive::verify()
{
    switch (cmd)
    {
        switch (cmd)
        {
        case COMMAND_NOP:
            actualStatus = status;
            break;
        case COMMAND_CREATE_NODE:
        {
            break;
        }
        case COMMAND_CONNECT:
        {
            actualStatus = nodeEditor()->connect(fromPort, toPort);
            break;
        }
        case COMMAND_DISCONNECT:
        {
            actualStatus = nodeEditor()->disconnect(signalPath);
            break;
        }
        case COMMAND_SELECT:
        {
            dag::SelectionInterface::Cont a;
            for (auto id : selection)
            {
                if (auto node = nodeEditor()->activeGraph()->node(id); node)
                {
                    a.emplace(node);
                }
            }
            actualStatus = nodeEditor()->select(selectionMode, a);
            break;
        }
        case COMMAND_CREATE_CHILD:
        {
            actualStatus = nodeEditor()->createChild();
            break;
        }
        case COMMAND_SET_ACTIVE_GRAPH:
            actualStatus = nodeEditor()->setActiveGraph(graphChildPath);

            break;
        case COMMAND_DELETE_NODE:
            actualStatus = nodeEditor()->deleteNode(nodeId);

            break;
        case COMMAND_COPY_NODE:
        {
            actualStatus = nodeEditor()->copyNodes();

            break;
        }
        case COMMAND_COMPARE_NODES:
        {
            actualStatus = nodeEditor()->compareNodes(cmpFlags);

            break;
        }
        case COMMAND_CREATE_TEMPLATE:
        {
            actualStatus = nodeEditor()->createTemplate(nodeClass);

            break;
        }
        case COMMAND_BROWSE_DOWN:
        {
            actualStatus = nodeEditor()->browseDown();

            break;
        }
        case COMMAND_BROWSE_UP:
        {
            actualStatus = nodeEditor()->browseUp();

            break;
        }
        case COMMAND_SET_POSITION:
        {
            auto node = nodeEditor()->activeGraph()->node(nodeId);
            if (node)
            {
                node->setPosition(position[0], position[1]);
                actualStatus.status = dagbase::Status::STATUS_OK;
            }
            break;
        }
        case COMMAND_SAVE:
        {
            dagbase::DebugPrinter printer;
            std::ostringstream sstr;
            printer.setStr(&sstr);
            nodeEditor()->activeGraph()->toLua(printer);
            actualStatus = nodeEditor()->save(printer);
            dagbase::Lua lua;
            dag::MemoryNodeLibrary nodeLib;
            auto str = sstr.str();
            auto restored = dagbase::Graph::fromString(nodeLib, str.c_str(), &actualStatus);
            ASSERT_NE(nullptr, restored);
            ASSERT_TRUE(nodeEditor()->activeGraph()->equals(*restored, static_cast<dagbase::ComparisonFlags>(dagbase::CMP_IDENT_BIT|dagbase::CMP_NAME_BIT|dagbase::CMP_CONNECTIONS_BIT))) << _caseName;
            break;
        }
        case COMMAND_LOAD:
        {
            actualStatus = nodeEditor()->load(filename.c_str());
            break;
        }
        case COMMAND_SERIALISE:
        {
            auto backingStore = dagbase::createBackingStore("FileBackingStore");
            ASSERT_NE(nullptr, backingStore);
            dagbase::OutputStream* ostr = dagbase::createOutputStream("TextFormat", *backingStore, filename.c_str());
            ASSERT_NE(nullptr, ostr);
            dagbase::Lua lua;
            actualStatus = nodeEditor()->serialise(serialiseFormat,*ostr, lua);
            dagbase::InputStream* istr = dagbase::createInputStream("TextFormat", *backingStore, filename.c_str());
            dag::MemoryNodeLibrary nodeLib;
            dagbase::Graph* restored = nullptr;
            switch (serialiseFormat)
            {
                case dag::NodeEditorLive::SERIALISE_OBJECT_GRAPH:
                {
                    dagbase::Stream::ObjId id{~0U};
                    istr->readRef(&id);
                    if (id == 0)
                        FAIL() << "Expected a Graph, got null";

                    {
                        restored = new dagbase::Graph(*istr, nodeLib, lua);
                    }
                    break;
                }
                case dag::NodeEditorLive::SERIALISE_FLAT:
                {
                    restored = new dagbase::Graph();
                    restored->setNodeLibrary(&nodeLib);
                    restored->readFlat(*istr, nodeLib, lua);
                    break;
                }
                default:
                    FAIL() << "Cannot deserialise unknown format";
                    break;
            }
            ASSERT_NE(nullptr, restored);
            ASSERT_TRUE(nodeEditor()->activeGraph()->equals(*restored, static_cast<dagbase::ComparisonFlags>(dagbase::CMP_IDENT_BIT|dagbase::CMP_NAME_BIT|dagbase::CMP_CONNECTIONS_BIT))) << _caseName;
            delete istr;
            delete ostr;
            delete backingStore;
            break;
        }
        case COMMAND_DESERIALISE:
        {
            auto backingStore = dagbase::createBackingStore("FileBackingStore");
            ASSERT_NE(nullptr, backingStore);
            dagbase::Lua lua;
            dagbase::InputStream* istr = dagbase::createInputStream("TextFormat", *backingStore, filename.c_str());
            ASSERT_NE(nullptr, istr);
            actualStatus = nodeEditor()->deserialise(serialiseFormat, *istr, lua);
            break;
        }
        case COMMAND_TOPO_SORT:
        {
            dagbase::NodeArray actualOrder;
            dagbase::NodeArray actualCycle;
            actualStatus = nodeEditor()->topologicalSort(&actualOrder, &actualCycle);
            for (std::size_t i=0; i<order.size(); ++i)
            {
                auto first = order[i].first;
                auto second = order[i].second;

                // Assert that first comes before second in the actual order.

                auto itFirst = std::find_if(actualOrder.begin(), actualOrder.end(), [first](dagbase::Node* node) {
                    return node && node->id() == first;
                });
                auto itSecond = std::find_if(actualOrder.begin(), actualOrder.end(), [second](dagbase::Node* node) {
                    return node && node->id() == second;
                });
                ASSERT_NE(actualOrder.end(), itFirst);
                ASSERT_NE(actualOrder.end(), itSecond);
                ASSERT_LT(itFirst, itSecond);
            }
            ASSERT_EQ(cycle.empty(), actualCycle.empty());
            ASSERT_EQ(cycle.size(), actualCycle.size());
            for (std::size_t i=0; i<cycle.size(); ++i)
            {
                EXPECT_EQ(cycle[i], actualCycle.a[i]->id());
            }
            break;
        }
        case COMMAND_ADD_PORT:
        {
            actualStatus = nodeEditor()->addPort(nodeId, portName, portDirection, portFlags, portValue.as<dagbase::Value>());

            break;
        }
        case COMMAND_DELETE_PORT:
        {
            actualStatus = nodeEditor()->deletePort(portId);
            break;
        }
        case COMMAND_SET_PORT_VALUE:
        {
            actualStatus = nodeEditor()->setPortValue(portId, portValue.as<dagbase::Value>());

            break;
        }
        case COMMAND_SET_NODE_ACTIVE:
        {
            actualStatus = nodeEditor()->setNodeActive(nodeId, active);

            break;
        }
        case COMMAND_EVALUATE_GRAPH:
        {
            dagbase::NodeArray evalOrder, evalCycle;
            actualStatus = nodeEditor()->topologicalSort(&evalOrder, &evalCycle);
            ASSERT_EQ(dagbase::Status{dagbase::Status::STATUS_OK}, actualStatus);
            actualStatus = nodeEditor()->evaluate(evalOrder);

            break;
        }
        case COMMAND_LOAD_SCRIPTS:
        {
            actualStatus = nodeEditor()->loadScripts(filename);

            break;
        }
        case COMMAND_ATTACH_SCRIPT:
        {
            actualStatus = nodeEditor()->attachScript(funcName);

            break;
        }
        default:
            FAIL() << "Got into an unhandled command " << commandToString(cmd);
            break;
        }
    }
    
    ASSERT_EQ(status.status, actualStatus.status)  << _caseName << ':' << commandToString(cmd) << ":Expected a status of " << dagbase::Status::statusCodeToString(status.status) << ", got " << dagbase::Status::statusCodeToString(actualStatus.status);
    ASSERT_EQ(status.resultType, actualStatus.resultType) << _caseName << ':' << commandToString(cmd) << ":Expected a resultType of " << dagbase::Status::resultTypeToString(status.resultType) << ", got " << dagbase::Status::resultTypeToString(actualStatus.resultType);
    ASSERT_EQ(status.result, actualStatus.result) << _caseName << ':' << commandToString(cmd);
}
