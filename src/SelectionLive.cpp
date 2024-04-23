//
// Created by tony on 10/02/24.
//

#include "config/config.h"

#include "SelectionLive.h"
#include "Node.h"
#include "SelectionInterface.h"

#include <algorithm>

namespace nbe
{
    std::size_t SelectionLive::count()
    {
        return _selection.size();
    }

    void SelectionLive::add(Cont::iterator begin, Cont::iterator end)
    {
        _selection.insert(begin, end);
    }

    void SelectionLive::subtract(Cont::iterator begin, Cont::iterator end)
    {
        for (auto it=begin; it!=end; ++it)
        {
            auto itFind = _selection.find(*it);

            if (itFind != _selection.end())
            {
                _selection.erase(itFind);
            }
        }
    }

    void SelectionLive::set(Cont::iterator begin, Cont::iterator end)
    {
        _selection.clear();
        add(begin, end);
    }

    void SelectionLive::toggle(Cont::iterator begin, Cont::iterator end)
    {
        for (auto it=begin; it!=end; ++it)
        {
            auto itFind = std::find(_selection.begin(), _selection.end(), *it);

            if (itFind!=_selection.end())
            {
                _selection.erase(itFind);
            }
            else
            {
                _selection.insert(*it);
            }
        }
    }

    bool SelectionLive::isSelected(Node *node)
    {
        return std::find(_selection.begin(), _selection.end(), node) != _selection.end();
    }

    void SelectionLive::add(Node *node)
    {
        if (node!=nullptr)
        {
            _selection.insert(node);
        }
    }

    void SelectionLive::computeBoundaryNodes(NodeArray* inputs, NodeArray* outputs, NodeArray* internals)
    {
        if (inputs != nullptr && outputs!=nullptr && internals != nullptr)
        {
            inputs->clear();
            outputs->clear();
            internals->clear();

            for (auto node : _selection)
            {
                bool isInput = false, isOutput = false;
                if (node->hasInputs())
                {
                    for (auto index=0; index<node->totalPorts(); ++index)
                    {
                        node->dynamicPort(index)->eachIncomingConnection([this,&isInput](Port* p)
                                                                         {
                            if (p->parent() != nullptr && !isSelected(p->parent()))
                            {
                                isInput = true;

                                return false;
                            }
                            return true;
                                                                         });
                    }
                    if (isInput)
                    {
                        inputs->push_back(node);
                    }
                }

                if (node->hasOutputs())
                {
                    for (auto index=0; index<node->totalPorts(); ++index)
                    {
                        node->dynamicPort(index)->eachOutgoingConnection([this,&isOutput](Port* p)
                                                                         {
                            if (p->parent() != nullptr && !isSelected(p->parent()))
                            {
                                isOutput = true;

                                return false;
                            }
                            return true;
                                                                         });
                    }
                    if (isOutput)
                    {
                        outputs->push_back(node);
                    }
                }

                if (true)
                {
                    bool isInternal = true;
/*
                    for (auto index=0; index<node->totalPorts(); ++index)
                    {
                        if (node->dynamicPort(index)->numIncomingConnections()==0 && node->dynamicPort(index)->numOutgoingConnections()==0)
                        {
                            isInternal = false;

                            continue;
                        }
                        node->dynamicPort(index)->eachIncomingConnection([this,&isInternal](Port* p)
                                                                         {
                            if (p->parent() != nullptr && !isSelected(p->parent()))
                            {
                                isInternal = false;

                                return false;
                            }
                            return true;
                                                                         });
                        node->dynamicPort(index)->eachOutgoingConnection([this,&isInternal](Port* p)
                                                                         {
                            if (p->parent() != nullptr && !isSelected(p->parent()))
                            {
                                isInternal = false;

                                return false;
                            }
                            return true;
                                                                         });
                    }
*/
                    if (isInternal)
                    {
                        internals->push_back(node);
                    }
                }
            }
        }
    }

    void SelectionLive::reconnectInputs(NodeArray& inputs, Node* newSource)
    {
        for (auto input : inputs)
        {
            input->reconnectInputs(_selection, newSource);
        }
    }

    void SelectionLive::reconnectOutputs(NodeArray& outputs, Node* newSink)
    {
        for (auto output : outputs)
        {
            output->reconnectOutputs(_selection, newSink);
        }
    }

}
