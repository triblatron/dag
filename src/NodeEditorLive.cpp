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

    dagbase::Status NodeEditorLive::load(const char* filename)
    {
        dagbase::Status status{ dagbase::Status::STATUS_UNKNOWN };
        auto g = dagbase::Graph::fromFile(*_nodeLib, filename, &status);

        if (g)
        {
            delete _graph;
            _graph = g;
        }

        return status;
    }

    void NodeEditorLive::eachClass(std::function<bool(dagbase::Node&)> f)
    {
        if (_nodeLib)
            _nodeLib->eachNode(f);
    }

    void NodeEditorLive::eachNode(std::function<bool(dagbase::Node*)> f)
    {
        if (_graph)
            _graph->eachNode(f);
    }

    void NodeEditorLive::eachSignalPath(std::function<bool(dagbase::SignalPath*)> f)
    {
        if (_graph)
            _graph->eachSignalPath(f);
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
            _graph->deleteNode(node);
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
            if (fromPort->parent() == toPort->parent())
            {
                auto status = dagbase::Status{ dagbase::Status::STATUS_CYCLE_DETECTED };
                status.resultType = dagbase::Status::RESULT_NODE;
                status.result.node = fromPort->parent();
                return status;
            }
            if (fromPort->dir() != dagbase::PortDirection::DIR_OUT)
            {
                std::swap(fromPort, toPort);
            }
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


            _selection->reconnectInputs(inputs, boundaryInput, *_graph);
            _selection->reconnectOutputs(outputs, boundaryOutput, *_graph);

            child->addNode(boundaryInput);
            child->addNode(boundaryOutput);

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

    void NodeEditorLive::debug()
    {
        if (_graph)
        {
            _graph->debug();
        }
    }

    size_t NodeEditorLive::selectionCount()
    {
        return _selection->count();
    }

    dagbase::Variant NodeEditorLive::find(std::string_view path) const
    {
        dagbase::Variant retval;

        retval = dagbase::findEndpoint(path, "numSelectedNodes", std::uint32_t(_selection->count()));
        if (retval.has_value())
            return retval;

        if (_graph)
        {
            retval = dagbase::findInternal(path, "graph", _graph);
            if (retval.has_value())
                return retval;
        }

        return {};
    }
}
