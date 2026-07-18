//
// Created by tony on 10/02/24.
//

#include "config/config.h"

#include "NodeEditorLive.h"

#include <set>

#include "MemoryNodeLibrary.h"
#include "core/Graph.h"
#include "SelectionLive.h"
#include "Boundary.h"
#include "SelectionInterface.h"
#include "core/SignalPath.h"
#include "core/Transfer.h"
#include "core/GraphNode.h"
#include "core/CloningFacility.h"

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

    void NodeEditorLive::eachClass(std::function<bool(const std::string&, dagbase::Node&)> f)
    {
        if (_nodeLib)
            _nodeLib->eachNode(std::move(f));
    }

    void NodeEditorLive::eachNode(std::function<bool(dagbase::Node*)> f)
    {
        if (_activeGraph)
            _activeGraph->eachNode(std::move(f));
    }

    void NodeEditorLive::eachSignalPath(std::function<bool(dagbase::SignalPath*)> f)
    {
        if (_activeGraph)
            _activeGraph->eachSignalPath(std::move(f));
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
            case SELECTION_ALL:
                selectAll();
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
            dagbase::Status status{dagbase::Status::STATUS_OK};
            SelectionInterface::Cont s;
            _activeGraph->eachNode([&s](dagbase::Node* node)
                             {
                                 s.emplace(node);

                                 return true;
                             });
            _selection->set(s.begin(), s.end());
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
                status.resultType = dagbase::Status::RESULT_NODE_ID;
                status.result = node->id();
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
                    status.resultType = dagbase::Status::RESULT_NODE_ID;
                    status.result = fromPort->parent()->id();
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
                        status.resultType = dagbase::Status::RESULT_PORT_ID;
                        status.status = dagbase::Status::STATUS_INVALID_PORT;
                        status.result = fromPort->id();
                    }
                    else if (toPort->dir() != dagbase::PortDirection::DIR_IN)
                    {
                        status.resultType = dagbase::Status::RESULT_PORT_ID;
                        status.status = dagbase::Status::STATUS_INVALID_PORT;
                        status.result = toPort->id();
                    }
                    else if (!isCompatible)
                    {
                        status.resultType = dagbase::Status::RESULT_PORT_ID;
                        status.status = dagbase::Status::STATUS_SYNTAX_ERROR;
                        status.result = fromPort->id();
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
                _activeGraph->deleteSignalPath(path);
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

    void NodeEditorLive::meanPosition(const SelectionInterface::NodeArray &nodes, float pos[2])
    {
        float total[2]{};
        for (auto node : nodes)
        {
            total[0] += node->position()[0];
            total[1] += node->position()[1];
        }

        total[0] /= float(nodes.size());
        total[1] /= float(nodes.size());

        pos[0] = total[0];
        pos[1] = total[1];
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
                float meanInputPos[2]{};
                meanPosition(_selection->externalInputs(), meanInputPos);
                boundaryInput->setPosition(meanInputPos[0], meanInputPos[1]);

                auto boundaryOutput = _graph->createNode("Boundary", "boundaryOutput");
                float meanOutputPos[2]{};
                meanPosition(_selection->externalOutputs(), meanOutputPos);
                boundaryOutput->setPosition(meanOutputPos[0], meanOutputPos[1]);
                std::vector<dagbase::SignalPath*> toRemove;
                const NodeArray& inputs = _selection->inputs();
                for (auto input : inputs)
                {
                    _activeGraph->eachSignalPath([this, input, &toRemove](dagbase::SignalPath* signalPath) {
                        // If the destination is an input and the source is not selected
                        if (signalPath->destNode() == input && !_selection->isSelected(signalPath->sourceNode()))
                        {
                            signalPath->markRemoved();
                            toRemove.emplace_back(signalPath);
                        }
                        return true;
                    });

                }

                const auto& outputs = _selection->outputs();
                for (auto output : outputs)
                {
                    _activeGraph->eachSignalPath([this, output, &toRemove](dagbase::SignalPath* signalPath) {
                        // If the source is an output and the destination is not selected then remove it
                        if (signalPath->sourceNode() == output && !_selection->isSelected(signalPath->destNode()))
                        {
                            signalPath->markRemoved();
                            toRemove.emplace_back(signalPath);
                        }
                        return true;
                    });
                }

                // Remove SignalPaths that have been marked removed.
                for (auto signalPath : toRemove)
                {
                    _activeGraph->deleteSignalPath(signalPath);
                }

                // Add SignalPaths from
                // Use the root Graph as the KeyGenerator for unique IDs
                _selection->reconnectInputs(boundaryInput, *_graph);
                _selection->reconnectOutputs(boundaryOutput, *_graph);

                child->addNode(boundaryInput);
                child->addNode(boundaryOutput);

                // Add SignalPaths from outputs of BoundaryInput to internals
                for (std::size_t i=0; i<boundaryInput->totalPorts(); ++i)
                {
                    auto port = boundaryInput->dynamicPort(i);
                    if (port->dir() == dagbase::PortDirection::DIR_OUT && !port->outgoingConnections().empty())
                    {
                        child->addSignalPath(new dagbase::SignalPath(port, port->outgoingConnections()[0]));
                    }
                }

                // Add SignalPaths from outputs of internals to inputs of BoundaryOutput
                for (std::size_t i=0; i<boundaryOutput->totalPorts(); ++i)
                {
                    auto port = boundaryOutput->dynamicPort(i);

                    if (port->dir() == dagbase::PortDirection::DIR_IN && !port->incomingConnections().empty())
                    {
                        child->addSignalPath(new dagbase::SignalPath(port->incomingConnections()[0], port));
                    }
                }

                // std::cerr << "Moving internals: activeGraph has " << _activeGraph->numNodes() << " nodes" << '\n';
                // std::cerr << "Moving internals: activeGraph has " << _activeGraph->numPorts() << " ports" << '\n';
                for (auto node : internals)
                {
                    // Avoid double-free of node in both original and child Graph.
                    _activeGraph->moveNode(node, child);
                    // std::cerr << "After: activeGraph has " << _activeGraph->numNodes() << " nodes" << '\n';
                    // std::cerr << "After: activeGraph has " << _activeGraph->numPorts() << " ports" << '\n';
                    // _activeGraph->removeNode(node);
                    //
                    // child->addNode(node);
                }

                _activeGraph->addChild(child);

                if (auto graphNode = dynamic_cast<dagbase::GraphNode*>(_graph->createNode("GraphNode", "child" + std::to_string(_graph->numChildren()))); graphNode)
                {
                    graphNode->setGraph(child);
                    float meanInternalPos[2]{};
                    meanPosition(_selection->internals(), meanInternalPos);
                    graphNode->setPosition(meanInternalPos[0], meanInternalPos[1]);
                    // Add the inputs of the Boundary input and the outputs of the Boundary output
                    for (std::size_t i=0; i<boundaryInput->totalPorts(); ++i)
                    {
                        auto sharedPort = boundaryInput->dynamicPort(i);
                        if (sharedPort->dir() == dagbase::PortDirection::DIR_IN)
                        {
                            graphNode->addDynamicPort(sharedPort, dagbase::MetaPort::FLAGS_NONE);
                            // Add a SignalPath from the incoming port to the shared Port.
                            if (sharedPort->numIncomingConnections()>0)
                            {
                                _activeGraph->addSignalPath(
                                    new dagbase::SignalPath(sharedPort->incomingConnections()[0], sharedPort));
                            }
                        }
                    }
                    for (std::size_t i=0; i<boundaryOutput->totalPorts(); ++i)
                    {
                        auto sharedPort = boundaryOutput->dynamicPort(i);
                        if (sharedPort->dir() == dagbase::PortDirection::DIR_OUT)
                        {
                            graphNode->addDynamicPort(sharedPort, dagbase::MetaPort::FLAGS_NONE);
                            // Add a SignalPath from the shared Port to the outgoing Port
                            if (sharedPort->numOutgoingConnections()>0)
                            {
                                _activeGraph->addSignalPath(
                                    new dagbase::SignalPath(sharedPort, sharedPort->outgoingConnections()[0]));
                            }
                        }
                    }
                    _activeGraph->addNode(graphNode);
                    status.status = dagbase::Status::STATUS_OK;
                    status.resultType = dagbase::Status::RESULT_NODE_ID;
                    status.result = graphNode->id();
                }
            }

            return status;
        }

        return dagbase::Status{dagbase::Status::STATUS_OBJECT_NOT_FOUND};
    }

    dagbase::Status NodeEditorLive::copyNodes()
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
                dagbase::CloningFacility facility;
                status = _activeGraph->cloneNodes(internals, *_activeGraph, facility, *_graph);
            }

            return status;
        }

        return dagbase::Status{dagbase::Status::STATUS_SYNTAX_ERROR};
    }

    dagbase::Status NodeEditorLive::compareNodes()
    {
        dagbase::Status status;

        if (_selection->count() != 2)
        {
            status.status = dagbase::Status::STATUS_INVALID_SELECTION;
        }
        else
        {
            auto internals = _selection->internals();
            dagbase::Node* op1 = internals.a[0];
            dagbase::Node* op2 = internals.a[1];
            bool equal = false;
            if (op1 && op2)
            {
                equal = op1->equals(*op2);
            }
            if (equal)
            {
                status.status = dagbase::Status::STATUS_OK;
            }
            else
            {
                status.status = dagbase::Status::STATUS_SYNTAX_ERROR;
            }
        }
        return status;
    }

    dagbase::Status NodeEditorLive::createTemplate(std::string className)
    {
        dagbase::Status status;

        if (_selection->count() != 1)
        {
            status.status = dagbase::Status::STATUS_INVALID_SELECTION;
            return status;
        }

        auto source = _selection->internals().a[0];
        if (!_nodeLib)
        {
            status.status = dagbase::Status::STATUS_SYNTAX_ERROR;
            return status;
        }
        _nodeLib->registerTemplate(className, _selection->internals().a[0]);
        status.status = dagbase::Status::STATUS_OK;

        return status;
    }

    dagbase::Status NodeEditorLive::deleteTemplate(dagbase::TemplateID id)
    {
        return {};
    }

    dagbase::Status NodeEditorLive::browseDown()
    {
        dagbase::Status status;

        if (_selection->count() == 1)
        {
            if (auto graphNode = dynamic_cast<dagbase::GraphNode*>(_selection->internals().a[0]); graphNode)
            {
                _activeGraph = graphNode->graph();

                status.status = dagbase::Status::STATUS_OK;
            }
            else
            {
                status.status = dagbase::Status::STATUS_INVALID_SELECTION;
            }
        }
        else
        {
            status.status = dagbase::Status::STATUS_INVALID_SELECTION;
        }
        return status;
    }

    dagbase::Status NodeEditorLive::browseUp()
    {
        dagbase::Status status;

        if (_activeGraph->parent())
        {
            _activeGraph = _activeGraph->parent();
            status.status = dagbase::Status::STATUS_OK;
        }
        else
        {
            status.status = dagbase::Status::STATUS_SYNTAX_ERROR;
        }

        return status;
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

        if (_nodeLib)
        {
            retval = dagbase::findInternal(path, "nodeLib", _nodeLib);
            if (retval.has_value())
                return retval;
        }
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
