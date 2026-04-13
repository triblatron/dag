//
// Created by tony on 10/02/24.
//

#include "config/config.h"

#include "NodeEditorLive.h"
#include "MemoryNodeLibrary.h"
#include "core/Graph.h"
#include "SelectionLive.h"
#include "Boundary.h"
#include "SelectionInterface.h"
#include "core/SignalPath.h"
#include "core/Transfer.h"

namespace dag
{
    NodeEditorLive::NodeEditorLive()
    {
        _nodeLib = new MemoryNodeLibrary();
        _graph = new dagbase::Graph();
        _graph->setNodeLibrary(_nodeLib);
        _selection = new SelectionLive();
    }

    NodeEditorLive::~NodeEditorLive()
    {
        delete _nodeLib;
        delete _graph;
        delete _selection;
        for (auto transfer : _transfers)
        {
            delete transfer;
        }
    }

    dagbase::Status NodeEditorLive::select(NodeEditorInterface::SelectionMode mode, dagbase::NodeSet &a)
    {
        dagbase::Status status;

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

    dagbase::Status NodeEditorLive::selectAll()
    {
        dagbase::Status status;
        SelectionInterface::Cont s;
        _selection->set(s.begin(), s.end());
        _graph->eachNode([this](dagbase::Node* node)
                         {
                             _selection->add(node);

                             return true;
                         });
        return status;
    }

    dagbase::Status NodeEditorLive::selectNone()
    {
        dagbase::Status status{dagbase::Status::STATUS_OK};
        SelectionInterface::Cont s;

        _selection->set(s.begin(), s.end());

        return status;
    }

    dagbase::Status NodeEditorLive::createNode(const std::string &className, const std::string &name)
    {
        if (_graph!=nullptr)
        {
            auto node = _graph->createNode(className, name);

            if (node != nullptr)
            {
                dagbase::Status status;

                status.status = dagbase::Status::STATUS_OK;
                status.resultType = dagbase::Status::RESULT_NODE;
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

    dagbase::Status NodeEditorLive::deleteNode(dagbase::NodeID id)
    {
        dagbase::Status status;
        auto node = _graph->node(id);

        if (node != nullptr)
        {
            _graph->removeNode(node);
            delete node;
            status.status = dagbase::Status::STATUS_OK;
            status.resultType = dagbase::Status::RESULT_NODE_ID;
            status.result.nodeId = id;
        }
        else
        {
            status.status = dagbase::Status::STATUS_OBJECT_NOT_FOUND;
            status.resultType = dagbase::Status::RESULT_NODE_ID;
            status.result.nodeId = id;
        }
        return status;
    }

    dagbase::Status NodeEditorLive::connect(dagbase::PortID from, dagbase::PortID to)
    {
        auto fromPort = _graph->port(from);
        auto toPort = _graph->port(to);

        if (fromPort != nullptr && toPort != nullptr)
        {
            if (fromPort->dir() == dagbase::PortDirection::DIR_OUT && toPort->dir() == dagbase::PortDirection::DIR_IN && fromPort->isCompatibleWith(*toPort))
            {
                auto transfer = fromPort->connectTo(*toPort);
                auto signalPath = new dagbase::SignalPath(fromPort, toPort);

                _graph->addSignalPath(signalPath);

                dagbase::Status status;

                status.status = dagbase::Status::STATUS_OK;
                status.resultType = dagbase::Status::RESULT_SIGNAL_PATH_ID;
                status.result.signalPathId = signalPath->id();

                _transfers.emplace_back(transfer);
                return status;
            }
            else
            {
                dagbase::Status status;

                if (fromPort->dir() != dagbase::PortDirection::DIR_OUT)
                {
                    status.resultType = dagbase::Status::RESULT_PORT;
                    status.status = dagbase::Status::STATUS_INVALID_PORT;
                    status.result.port = fromPort;
                }
                else if (toPort->dir() != dagbase::PortDirection::DIR_IN)
                {
                    status.resultType = dagbase::Status::RESULT_PORT;
                    status.status = dagbase::Status::STATUS_INVALID_PORT;
                    status.result.port = toPort;
                }
                return status;
            }
        }
        else
        {
            dagbase::Status status;

            if (fromPort == nullptr)
            {
                status.resultType = dagbase::Status::RESULT_PORT_ID;
                status.status = dagbase::Status::STATUS_OBJECT_NOT_FOUND;
                status.result.portId = from;
            }
            else
            {
                status.resultType = dagbase::Status::RESULT_PORT_ID;
                status.status = dagbase::Status::STATUS_OBJECT_NOT_FOUND;
                status.result.portId = to;
            }

            return status;
        }
        return {};
    }

    dagbase::Status NodeEditorLive::disconnect(dagbase::SignalPathID id)
    {
        dagbase::Status status;

        dagbase::SignalPath* path = _graph->signalPath(id);

        if (path != nullptr)
        {
            path->source()->disconnect(*path->dest());
            _graph->removeSignalPath(path);
            status.status = dagbase::Status::STATUS_OK;
        }
        else
        {
            status.status = dagbase::Status::STATUS_OBJECT_NOT_FOUND;
            status.resultType = dagbase::Status::RESULT_SIGNAL_PATH_ID;
            status.result.signalPathId = id;
        }
        return status;
    }

    dagbase::Status NodeEditorLive::createChild()
    {
        dagbase::Status status;

        if (_selection->count() == 0)
        {
            status.status = dagbase::Status::STATUS_INVALID_SELECTION;
        }
        else
        {
            NodeArray inputs;
            NodeArray outputs;
            NodeArray internals;
            _selection->computeBoundaryNodes(&inputs, &outputs, &internals);
            auto child = new dagbase::Graph();
            child->setNodeLibrary(_nodeLib);
            auto boundaryInput = child->createNode("Boundary","boundaryInput");
            auto boundaryOutput = child->createNode("Boundary", "boundaryOutput");

            child->addNode(boundaryInput);
            child->addNode(boundaryOutput);

            _selection->reconnectInputs(inputs, boundaryInput);
            _selection->reconnectOutputs(outputs, boundaryOutput);

            for (auto node : internals)
            {
                // Avoid double-free of node in both orignal and child Graph.
                _graph->removeNode(node);
                node->setId(child->nextNodeID());
                child->addNode(node);
            }

            _graph->addChild(child);

            status.status = dagbase::Status::STATUS_OK;
            status.resultType = dagbase::Status::RESULT_GRAPH;
            status.result.graph = child;
        }

        return status;
    }

    dagbase::Status NodeEditorLive::createTemplate(dagbase::NodeID id)
    {
        return {};
    }

    dagbase::Status NodeEditorLive::deleteTemplate(dagbase::TemplateID id)
    {
        return {};
    }

    size_t NodeEditorLive::selectionCount()
    {
        return _selection->count();
    }
}
