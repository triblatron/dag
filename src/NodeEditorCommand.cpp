//
// Created by Tony Horrobin on 14/11/2026.
//

#include "config/config.h"

#include "NodeEditorCommand.h"

#include "MemoryNodeLibrary.h"
#include "io/StreamFactory.h"
#include "io/BackingStore.h"
#include "io/OutputStream.h"
#include "io/InputStream.h"

namespace dag
{
    NodeEditorCommand::NodeEditorCommand(NodeEditorCommand *decoratee)
        :
    _decoratee(decoratee)
    {
        // Do nothing.
    }

    void NodeEditorCommand::configure(dagbase::ConfigurationElement& config)
    {
        dagbase::ConfigurationElement::readConfig<Command>(config, "cmd", &parseCommand, &cmd);

        switch (cmd)
        {
        case COMMAND_NOP:
            // No parameters.
            break;
        case COMMAND_CREATE_NODE:
            dagbase::ConfigurationElement::readConfig(config, "nodeClass", &nodeClass);
            dagbase::ConfigurationElement::readConfig(config, "nodeName", &nodeName);
            break;
        case COMMAND_CONNECT:
            dagbase::ConfigurationElement::readConfig(config, "fromPort", &fromPort);
            dagbase::ConfigurationElement::readConfig(config, "toPort", &toPort);
            break;
        case COMMAND_DISCONNECT:
            dagbase::ConfigurationElement::readConfig(config, "signalPath", &signalPath);
            dagbase::ConfigurationElement::readConfig(config, "fromPort", &fromPort);
            dagbase::ConfigurationElement::readConfig(config, "toPort", &toPort);

            break;
        case COMMAND_SELECT:
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
            // No parameters.
            break;
        case COMMAND_SET_ACTIVE_GRAPH:
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
            dagbase::ConfigurationElement::readConfig(config, "node", &nodeId);
            break;
        case COMMAND_COPY_NODE:
            dagbase::ConfigurationElement::readConfig(config, "node", &nodeId);
            break;
        case COMMAND_COMPARE_NODES:
            dagbase::ConfigurationElement::readConfig(config, "node", &nodeId);
            dagbase::ConfigurationElement::readConfig(config, "otherNode", &otherNodeId);
            dagbase::ConfigurationElement::readConfig<dagbase::ComparisonFlags>(config, "comparisonFlags", &dagbase::parseComparisonFlags, &cmpFlags);
            break;
        case COMMAND_CREATE_TEMPLATE:
            dagbase::ConfigurationElement::readConfig(config, "nodeClass", &nodeClass);

            break;
        case COMMAND_BROWSE_DOWN:
            break;
        case COMMAND_BROWSE_UP:
            break;
        case COMMAND_SET_POSITION:
            dagbase::ConfigurationElement::readConfig(config, "node", &nodeId);
            dagbase::ConfigurationElement::readConfig(config, "x", &position[0]);
            dagbase::ConfigurationElement::readConfig(config, "y", &position[1]);

            break;
        case COMMAND_SAVE:
            break;
        case COMMAND_LOAD:
            dagbase::ConfigurationElement::readConfig(config, "filename", &filename);
            break;
        case COMMAND_SERIALISE:
            dagbase::ConfigurationElement::readConfig(config, "filename", &filename);
            dagbase::ConfigurationElement::readConfig<dag::NodeEditorLive::SerialiseFormat>(config, "serialiseFormat", &dag::NodeEditorLive::parseSerialiseFormat, &serialiseFormat);

            break;
        case COMMAND_DESERIALISE:
            dagbase::ConfigurationElement::readConfig(config, "filename", &filename);
            dagbase::ConfigurationElement::readConfig<dag::NodeEditorLive::SerialiseFormat>(config, "serialiseFormat", &dag::NodeEditorLive::parseSerialiseFormat, &serialiseFormat);

            break;
        case COMMAND_TOPO_SORT:
            if (auto element = config.findElement("order"); element)
            {
                element->eachChild([this](dagbase::ConfigurationElement& child) {
                    if (child.numChildren() == 2)
                    {
                        decltype(order)::value_type entry;
                        entry.first = child.child(0)->asInteger(-1);
                        entry.second = child.child(1)->asInteger(-1);
                        order.emplace_back(entry);
                    }
                    return true;
                });
            }
            dagbase::ConfigurationElement::readConfigVector(config, "cycle", &cycle);

            break;
        case COMMAND_ADD_PORT:
            dagbase::ConfigurationElement::readConfig(config, "node", &nodeId);
            dagbase::ConfigurationElement::readConfig(config, "name", &portName);
            dagbase::ConfigurationElement::readConfig<dagbase::PortDirection::Direction>(config, "direction", &dagbase::PortDirection::parseFromString, &portDirection);
            dagbase::ConfigurationElement::readConfig<dagbase::Port::PortFlags>(config, "portFlags", &dagbase::Port::parsePortFlags, &portFlags);
            dagbase::ConfigurationElement::readConfig(config, "value", &portValue);

            break;
        case COMMAND_DELETE_PORT:
            dagbase::ConfigurationElement::readConfig(config, "port", &portId);

            break;
        case COMMAND_SET_PORT_VALUE:
            dagbase::ConfigurationElement::readConfig(config, "port", &portId);
            dagbase::ConfigurationElement::readConfig(config, "value", &portValue);

            break;
        case COMMAND_SET_NODE_ACTIVE:
            dagbase::ConfigurationElement::readConfig(config, "node", &nodeId);
            dagbase::ConfigurationElement::readConfig<dagbase::Node::Active>(config, "commented", &dagbase::Node::parseActive, &active);

            break;
        case COMMAND_EVALUATE_GRAPH:

            break;
        case COMMAND_LOAD_SCRIPTS:
            dagbase::ConfigurationElement::readConfig(config, "filename", &filename);

            break;
        case COMMAND_ATTACH_SCRIPT:
            dagbase::ConfigurationElement::readConfig(config, "funcName", &funcName);

            break;
        default:
            break;
        }

    }

    void NodeEditorCommand::reset()
    {
    }

    dagbase::Status NodeEditorCommand::undo()
    {
        return dagbase::Status{dagbase::Status::STATUS_INTERNAL_ERROR};
    }

    dagbase::Status NodeEditorCommand::redo(std::pmr::vector<dagbase::Variant> *results)
    {
        return dagbase::Status{dagbase::Status::STATUS_INTERNAL_ERROR};
    }

    dagbase::Status NodeEditorCommand::makeItSo(const std::pmr::vector<dagbase::Variant> &args,
                                                std::pmr::vector<dagbase::Variant> *results)
    {
        switch (cmd)
        {
        case COMMAND_NOP:
            return dagbase::Status{dagbase::Status::STATUS_OK};
        case COMMAND_CREATE_NODE:
        {
            return nodeEditor()->createNode(nodeClass, nodeName);
            break;
        }
        case COMMAND_CONNECT:
        {
            return nodeEditor()->connect(fromPort, toPort);
            break;
        }
        case COMMAND_DISCONNECT:
        {
            return nodeEditor()->disconnect(signalPath);
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
            return nodeEditor()->select(selectionMode, a);
            break;
        }
        case COMMAND_CREATE_CHILD:
        {
            return nodeEditor()->createChild();
            break;
        }
        case COMMAND_SET_ACTIVE_GRAPH:
            return nodeEditor()->setActiveGraph(graphChildPath);

            break;
        case COMMAND_DELETE_NODE:
            return nodeEditor()->deleteNode(nodeId);

            break;
        case COMMAND_COPY_NODE:
        {
            return nodeEditor()->copyNodes();

            break;
        }
        case COMMAND_COMPARE_NODES:
        {
            return nodeEditor()->compareNodes(cmpFlags);

            break;
        }
        case COMMAND_CREATE_TEMPLATE:
        {
            return nodeEditor()->createTemplate(nodeClass);

            break;
        }
        case COMMAND_BROWSE_DOWN:
        {
            return nodeEditor()->browseDown();

            break;
        }
        case COMMAND_BROWSE_UP:
        {
            return nodeEditor()->browseUp();

            break;
        }
        case COMMAND_SET_POSITION:
        {
            auto node = nodeEditor()->activeGraph()->node(nodeId);
            if (node)
            {
                node->setPosition(position[0], position[1]);
                return dagbase::Status{dagbase::Status::STATUS_OK};
            }
            break;
        }
        case COMMAND_SAVE:
        {
            dagbase::DebugPrinter printer;
            std::ostringstream sstr;
            printer.setStr(&sstr);
            nodeEditor()->activeGraph()->toLua(printer);
            auto actualStatus = nodeEditor()->save(printer);
            dagbase::Lua lua;
            auto str = sstr.str();
            results->emplace_back(str);
            return actualStatus;
            break;
        }
        case COMMAND_LOAD:
        {
            return nodeEditor()->load(filename.c_str());
            break;
        }
        case COMMAND_SERIALISE:
        {
            auto backingStore = dagbase::createBackingStore("FileBackingStore");
            if (!backingStore)
            {
                return dagbase::Status{dagbase::Status::STATUS_INTERNAL_ERROR};
            }
            dagbase::OutputStream* ostr = dagbase::createOutputStream("TextFormat", *backingStore, filename.c_str());
            if (!ostr)
            {
                return dagbase::Status{dagbase::Status::STATUS_INTERNAL_ERROR};
            }
            dagbase::Lua lua;
            auto actualStatus = nodeEditor()->serialise(serialiseFormat,*ostr, lua);
            delete ostr;
            delete backingStore;
            return actualStatus;
            break;
        }
        case COMMAND_DESERIALISE:
        {
            auto backingStore = dagbase::createBackingStore("FileBackingStore");
            if (!backingStore)
                return dagbase::Status{dagbase::Status::STATUS_INTERNAL_ERROR};
            dagbase::Lua lua;
            dagbase::InputStream* istr = dagbase::createInputStream("TextFormat", *backingStore, filename.c_str());

            if (!istr)
                return dagbase::Status{dagbase::Status::STATUS_INTERNAL_ERROR};
            return nodeEditor()->deserialise(serialiseFormat, *istr, lua);
        }
        case COMMAND_TOPO_SORT:
        {
            dagbase::NodeArray actualOrder;
            dagbase::NodeArray actualCycle;
            return nodeEditor()->topologicalSort(&actualOrder, &actualCycle);
        }
        case COMMAND_ADD_PORT:
        {
            return nodeEditor()->addPort(nodeId, portName, portDirection, portFlags, portValue.as<dagbase::Value>());

            break;
        }
        case COMMAND_DELETE_PORT:
        {
            return nodeEditor()->deletePort(portId);
            break;
        }
        case COMMAND_SET_PORT_VALUE:
        {
            return nodeEditor()->setPortValue(portId, portValue.as<dagbase::Value>());

            break;
        }
        case COMMAND_SET_NODE_ACTIVE:
        {
            return nodeEditor()->setNodeActive(nodeId, active);

            break;
        }
        case COMMAND_EVALUATE_GRAPH:
        {
            dagbase::NodeArray evalOrder, evalCycle;
            auto actualStatus = nodeEditor()->topologicalSort(&evalOrder, &evalCycle);
            if (actualStatus.status != dagbase::Status::STATUS_OK)
                return actualStatus;
            return nodeEditor()->evaluate(evalOrder);

            break;
        }
        case COMMAND_LOAD_SCRIPTS:
        {
            return nodeEditor()->loadScripts(filename);

            break;
        }
        case COMMAND_ATTACH_SCRIPT:
        {
            return nodeEditor()->attachScript(funcName);

            break;
        }
        default:
            return dagbase::Status{dagbase::Status::STATUS_UNKNOWN_OPCODE};// << "Got into an unhandled command " << commandToString(_cmd);
            break;
        }

        return dagbase::Status{dagbase::Status::STATUS_UNKNOWN_OPCODE};
    }
}
