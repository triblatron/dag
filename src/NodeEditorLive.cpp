//
// Created by tony on 10/02/24.
//

#include "config/config.h"

#include "NodeEditorLive.h"
#include "MemoryNodeLibrary.h"
#include "Graph.h"
#include "SelectionLive.h"
#include "Boundary.h"
#include "SelectionInterface.h"
#include "SignalPath.h"

namespace nbe
{
    NodeEditorLive::NodeEditorLive()
    {
        _nodeLib = new MemoryNodeLibrary();
        _graph = new Graph();
        _graph->setNodeLibrary(_nodeLib);
        _selection = new SelectionLive();
    }

    NodeEditorLive::~NodeEditorLive()
    {
        delete _nodeLib;
        delete _graph;
        delete _selection;
    }

    Status NodeEditorLive::select(NodeEditorInterface::SelectionMode mode, NodeSet &a)
    {
        Status status;

        switch (mode)
        {
            case SELECTION_SET:
                _selection->set(a.begin(), a.end());
                break;
            case SELECTION_ADD:
                _selection->add(a.begin(), a.end());
                break;
            case SELECTION_SUBTRACT:
                _selection->subtract(a.begin(), a.end());
                break;
            case SELECTION_TOGGLE:
                _selection->toggle(a.begin(), a.end());
                break;
        }
        return status;
    }

    Status NodeEditorLive::selectAll()
    {
        Status status;
        SelectionInterface::Cont s;
        _selection->set(s.begin(), s.end());
        _graph->eachNode([this](Node* node)
                         {
                             _selection->add(node);

                             return true;
                         });
        return status;
    }

    Status NodeEditorLive::selectNone()
    {
        Status status;
        SelectionInterface::Cont s;

        _selection->set(s.begin(), s.end());

        return status;
    }

    Status NodeEditorLive::createNode(const std::string &className, const std::string &name)
    {
        if (_graph!=nullptr)
        {
            auto node = _graph->createNode(className, name);

            if (node != nullptr)
            {
                Status status;

                status.status = Status::STATUS_OK;
                status.resultType = Status::RESULT_NODE;
                status.result.node = node;
                _graph->addNode(node);
                for (auto i=0; i<node->totalPorts(); ++i)
                {
                    _graph->addPort(node->dynamicPort(i));
                }
                return status;
            }
        }
        return {};
    }

    Status NodeEditorLive::deleteNode(NodeID id)
    {
        Status status;
        auto node = _graph->node(id);

        if (node != nullptr)
        {
            _graph->removeNode(node);
            delete node;
            status.resultType = Status::RESULT_NODE;
            status.result.node = node;
        }
        else
        {
            status.status = Status::STATUS_OBJECT_NOT_FOUND;
            status.resultType = Status::RESULT_NODE_ID;
            status.result.nodeId = id;
        }
        return status;
    }

    Status NodeEditorLive::connect(PortID from, PortID to)
    {
        auto fromPort = _graph->port(from);
        auto toPort = _graph->port(to);

        if (fromPort != nullptr && toPort != nullptr)
        {
            if (fromPort->dir() == PortDirection::DIR_OUT && toPort->dir() == PortDirection::DIR_IN && fromPort->isCompatibleWith(*toPort))
            {
                fromPort->connectTo(*toPort);
                auto signalPath = new SignalPath(fromPort, toPort);

                _graph->addSignalPath(signalPath);

                Status status;

                status.status = Status::STATUS_OK;
                status.resultType = Status::RESULT_SIGNAL_PATH_ID;
                status.result.signalPathId = signalPath->id();
                return status;
            }
            else
            {
                Status status;

                if (fromPort->dir() != PortDirection::DIR_OUT)
                {
                    status.resultType = Status::RESULT_PORT;
                    status.status = Status::STATUS_INVALID_PORT;
                    status.result.port = fromPort;
                }
                else if (toPort->dir() != PortDirection::DIR_IN)
                {
                    status.resultType = Status::RESULT_PORT;
                    status.status = Status::STATUS_INVALID_PORT;
                    status.result.port = toPort;
                }
                return status;
            }
        }
        else
        {
            Status status;

            if (fromPort == nullptr)
            {
                status.resultType = Status::RESULT_PORT_ID;
                status.status = Status::STATUS_OBJECT_NOT_FOUND;
                status.result.portId = from;
            }
            else
            {
                status.resultType = Status::RESULT_PORT_ID;
                status.status = Status::STATUS_OBJECT_NOT_FOUND;
                status.result.portId = to;
            }

            return status;
        }
        return {};
    }

    Status NodeEditorLive::disconnect(SignalPathID id)
    {
        Status status;

        SignalPath* path = _graph->signalPath(id);

        if (path != nullptr)
        {
            path->source()->disconnect(*path->dest());
            _graph->removeSignalPath(path);
        }
        else
        {
            status.status = Status::STATUS_OBJECT_NOT_FOUND;
            status.resultType = Status::RESULT_SIGNAL_PATH_ID;
            status.result.signalPathId = id;
        }
        return status;
    }

    Status NodeEditorLive::createChild()
    {
        Status status;

        if (_selection->count() == 0)
        {
            status.status = Status::STATUS_INVALID_SELECTION;
        }
        else
        {
            NodeArray inputs;
            NodeArray outputs;
            NodeArray internals;
            _selection->computeBoundaryNodes(&inputs, &outputs, &internals);
            auto child = new Graph();
            child->setNodeLibrary(_nodeLib);
            auto boundaryInput = child->createNode("Boundary","boundaryInput");
            auto boundaryOutput = child->createNode("Boundary", "boundaryOutput");

            child->addNode(boundaryInput);
            child->addNode(boundaryOutput);

            _selection->reconnectInputs(inputs, boundaryInput);
            _selection->reconnectOutputs(outputs, boundaryOutput);
/*
            for (auto node : inputs)
            {
                for (size_t index=0; index<node->totalPorts(); ++index)
                {
                    // TODO:Wire up boundary instead of input ports
                    auto port = node->dynamicPort(index);
                    auto newInput = port->clone();
                    Port::PortArray oldInputs = port->incomingConnections();

                    port->eachIncomingConnection([this, newInput](Port* input)
                                                 {
                                                    input->connectTo(*newInput);
                                                    return true;
                                                 });
                    for (auto it=oldInputs.begin(); it!=oldInputs.end(); ++it)
                    {
                        (*it)->disconnect(*port);
                    }
                    boundaryInput->addDynamicPort(newInput);
                }

            }

            for (auto node : outputs)
            {
                for (size_t index=0; index<node->totalPorts(); ++index)
                {
                    auto port = node->dynamicPort(index);
                    auto newOutput = port->create();
                    Port::PortArray oldOutputs = port->outgoingConnections();
                    port->eachOutgoingConnection([this, port, newOutput](Port* output)
                                                 {
                                                     newOutput->connectTo(*output);
                                                     return true;
                                                 });
                    for (auto oldOutput : oldOutputs )
                    {
                        port->disconnect(*oldOutput);
                    }
                    boundaryOutput->addDynamicPort(newOutput);
                }
            }

*/
            for (auto node : internals)
            {
                // Avoid double-free of node in both orignal and child Graph.
                _graph->removeNode(node);
                node->setId(child->nextNodeID());
                child->addNode(node);
            }

            _graph->addChild(child);

            status.resultType = Status::RESULT_GRAPH;
            status.result.graph = child;
        }

        return status;
    }

    Status NodeEditorLive::createTemplate(NodeID id)
    {
        return {};
    }

    Status NodeEditorLive::deleteTemplate(TemplateID id)
    {
        return {};
    }

    size_t NodeEditorLive::selectionCount()
    {
        return _selection->count();
    }
}
