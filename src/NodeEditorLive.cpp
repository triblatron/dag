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
        _activeGraph = _graph;
        _selection = new SelectionLive();
    }

    NodeEditorLive::~NodeEditorLive()
    {
        delete _nodeLib;
        delete _graph;
        // The active graph is a reference to somewhere in the tree of Graph we just deleted.
        delete _selection;
        for (auto transfer : _transfers)
        {
            delete transfer;
        }
    }

    dagbase::Status NodeEditorLive::setActiveGraph(const GraphChildPath &path)
    {
        if (_graph)
        {
            _activeGraph = _graph;
            for (auto childIndex : path)
            {
                _activeGraph = _activeGraph->child(childIndex);
            }

            if (_activeGraph)
                return dagbase::Status{dagbase::Status::STATUS_OK};
        }

        return dagbase::Status{dagbase::Status::STATUS_OBJECT_NOT_FOUND};
    }

    dagbase::Status NodeEditorLive::load(const char* filename)
    {
        dagbase::Status status{ dagbase::Status::STATUS_UNKNOWN };
        auto g = dagbase::Graph::fromFile(*_nodeLib, filename, &status);

        if (g)
        {
            delete _graph;
            _graph = g;
            _activeGraph = _graph;
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
        if (_activeGraph)
            _activeGraph->eachNode(f);
    }

    void NodeEditorLive::eachSignalPath(std::function<bool(dagbase::SignalPath*)> f)
    {
        if (_activeGraph)
            _activeGraph->eachSignalPath(f);
    }

    dagbase::Status NodeEditorLive::select(NodeEditorInterface::SelectionMode mode, SelectionInterface::Cont &a)
    {
        dagbase::Status status{dagbase::Status::STATUS_OK};

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
            case SELECTION_CLEAR:
                _selection->clear();
                break;
            case SELECTION_UNKNOWN:
                status.status = dagbase::Status::STATUS_SYNTAX_ERROR;
                break;
        }
        return status;
    }

    dagbase::Status NodeEditorLive::selectAll()
    {
        if (_activeGraph)
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

        return dagbase::Status{dagbase::Status::STATUS_OBJECT_NOT_FOUND};
    }

    dagbase::Status NodeEditorLive::selectNone()
    {
        dagbase::Status status{dagbase::Status::STATUS_OK};

        _selection->clear();

        return status;
    }

    dagbase::Status NodeEditorLive::createNode(const std::string &className, const std::string &name)
    {
        if (_graph && _activeGraph)
        {
            // Use the root Graph to create the Node so that its ID is unique across all Graphs
            auto node = _graph->createNode(className, name);

            if (node != nullptr)
            {
                dagbase::Status status{dagbase::Status::STATUS_UNKNOWN};

                status.status = dagbase::Status::STATUS_OK;
                status.resultType = dagbase::Status::RESULT_NODE;
                status.result = node;
                // Add the node to the active Graph
                _activeGraph->addNode(node);

                return status;
            }
        }
        return {};
    }

    dagbase::Status NodeEditorLive::deleteNode(dagbase::NodeID id)
    {
        if (_activeGraph)
        {
            dagbase::Status status{dagbase::Status::STATUS_UNKNOWN};
            auto node = _activeGraph->node(id);

            if (node != nullptr)
            {
                _activeGraph->deleteNode(node);
                delete node;
                status.status = dagbase::Status::STATUS_OK;
                status.resultType = dagbase::Status::RESULT_NODE_ID;
                status.result = id;
            }
            else
            {
                status.status = dagbase::Status::STATUS_OBJECT_NOT_FOUND;
                status.resultType = dagbase::Status::RESULT_NODE_ID;
                status.result = id;
            }
            return status;
        }

        return dagbase::Status{dagbase::Status::STATUS_OBJECT_NOT_FOUND};
    }

    dagbase::Status NodeEditorLive::connect(dagbase::PortID from, dagbase::PortID to)
    {
        if (_activeGraph)
        {
            auto fromPort = _activeGraph->port(from);
            auto toPort = _activeGraph->port(to);

            if (fromPort != nullptr && toPort != nullptr)
            {
                if (fromPort->parent() == toPort->parent())
                {
                    auto status = dagbase::Status{ dagbase::Status::STATUS_CYCLE_DETECTED };
                    status.resultType = dagbase::Status::RESULT_NODE;
                    status.result = fromPort->parent();
                    return status;
                }
                if (fromPort->dir() != dagbase::PortDirection::DIR_OUT)
                {
                    std::swap(fromPort, toPort);
                }
                bool isCompatible = fromPort->isCompatibleWith(*toPort);
                if (fromPort->dir() == dagbase::PortDirection::DIR_OUT && toPort->dir() == dagbase::PortDirection::DIR_IN && isCompatible)
                {
                    auto transfer = fromPort->connectTo(*toPort);
                    auto signalPath = new dagbase::SignalPath(fromPort, toPort);

                    _activeGraph->addSignalPath(signalPath);

                    dagbase::Status status{dagbase::Status::STATUS_UNKNOWN};

                    status.status = dagbase::Status::STATUS_OK;
                    status.resultType = dagbase::Status::RESULT_SIGNAL_PATH_ID;
                    status.result = signalPath->id();
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
                        status.result = fromPort;
                    }
                    else if (toPort->dir() != dagbase::PortDirection::DIR_IN)
                    {
                        status.resultType = dagbase::Status::RESULT_PORT;
                        status.status = dagbase::Status::STATUS_INVALID_PORT;
                        status.result = toPort;
                    }
                    else if (!isCompatible)
                    {
                        status.resultType = dagbase::Status::RESULT_PORT;
                        status.status = dagbase::Status::STATUS_SYNTAX_ERROR;
                        status.result = fromPort;
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
                    status.result = from;
                }
                else
                {
                    status.resultType = dagbase::Status::RESULT_PORT_ID;
                    status.status = dagbase::Status::STATUS_OBJECT_NOT_FOUND;
                    status.result = to;
                }

                return status;
            }
        }
        return dagbase::Status{dagbase::Status::STATUS_OBJECT_NOT_FOUND};
    }

    dagbase::Status NodeEditorLive::disconnect(dagbase::SignalPathID id)
    {
        if (_activeGraph)
        {
            dagbase::Status status;

            dagbase::SignalPath* path = _activeGraph->signalPath(id);

            if (path != nullptr)
            {
                path->source()->disconnect(*path->dest());
                _activeGraph->removeSignalPath(path);
                status.status = dagbase::Status::STATUS_OK;
            }
            else
            {
                status.status = dagbase::Status::STATUS_OBJECT_NOT_FOUND;
                status.resultType = dagbase::Status::RESULT_SIGNAL_PATH_ID;
                status.result = id;
            }

            return status;
        }

        return dagbase::Status{dagbase::Status::STATUS_OBJECT_NOT_FOUND};
    }

    dagbase::Status NodeEditorLive::createChild()
    {
        if (_graph && _activeGraph)
        {
            dagbase::Status status;

            if (_selection->count() == 0)
            {
                status.status = dagbase::Status::STATUS_INVALID_SELECTION;
            }
            else
            {
                const NodeArray& internals = _selection->internals();
                auto child = new dagbase::Graph();
                child->setNodeLibrary(_nodeLib);
                // We create Nodes using the parent Graph for consistency of IDs.
                auto boundaryInput = _graph->createNode("Boundary","boundaryInput");
                auto boundaryOutput = _graph->createNode("Boundary", "boundaryOutput");


                // Use the root Graph as the KeyGenerator for unique IDs
                _selection->reconnectInputs(boundaryInput, *_graph);
                _selection->reconnectOutputs(boundaryOutput, *_graph);

                child->addNode(boundaryInput);
                child->addNode(boundaryOutput);

                for (auto node : internals)
                {
                    // Avoid double-free of node in both orignal and child Graph.
                    _activeGraph->removeNode(node);

                    child->addNode(node);
                }

                _activeGraph->addChild(child);

                status.status = dagbase::Status::STATUS_OK;
                status.resultType = dagbase::Status::RESULT_GRAPH;
                status.result = child;
            }

            return status;
        }

        return dagbase::Status{dagbase::Status::STATUS_OBJECT_NOT_FOUND};
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

        retval = dagbase::findInternal(path, "selection", _selection);
        if (retval.has_value())
            return retval;

        if (_graph)
        {
            retval = dagbase::findInternal(path, "graph", _graph);
            if (retval.has_value())
                return retval;
        }

        if (_activeGraph)
        {
            retval = dagbase::findInternal(path, "activeGraph", _activeGraph);
            if (retval.has_value())
                return retval;
        }

        return {};
    }
}
