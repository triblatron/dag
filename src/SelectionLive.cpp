//
// Created by tony on 10/02/24.
//

#include "config/config.h"

#include "SelectionLive.h"
#include "../thirdparty/dagbase/include/core/Node.h"
#include "SelectionInterface.h"

#include <algorithm>

namespace dag
{
    std::size_t SelectionLive::count()
    {
        return _selection.size();
    }

    void SelectionLive::add(Cont::iterator begin, Cont::iterator end)
    {
        _selection.m.insert(begin, end);
    }

    //! \note begin and end cannot be from _selection because each erase() would invalidate the iterators
    void SelectionLive::subtract(Cont::iterator begin, Cont::iterator end)
    {
        for (auto it=begin; it!=end; ++it)
        {
            auto itFind = _selection.m.find(*it);

            if (itFind != _selection.end())
            {
                _selection.m.erase(itFind);
            }
        }
    }

    void SelectionLive::set(Cont::iterator begin, Cont::iterator end)
    {
        _selection.m.clear();
        add(begin, end);
        computeBoundaryNodes(&_inputs, &_outputs, &_internals);
    }

    void SelectionLive::toggle(Cont::iterator begin, Cont::iterator end)
    {
        for (auto it=begin; it!=end; ++it)
        {
            auto itFind = std::find(_selection.begin(), _selection.end(), *it);

            if (itFind!=_selection.end())
            {
                _selection.m.erase(itFind);
            }
            else
            {
                _selection.m.emplace(*it);
            }
        }
    }

    bool SelectionLive::isSelected(dagbase::Node *node)
    {
        return std::find(_selection.begin(), _selection.end(), node) != _selection.end();
    }

    void SelectionLive::add(dagbase::Node *node)
    {
        if (node!=nullptr)
        {
            _selection.m.insert(node);
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
                    for (std::size_t index=0; index<node->totalPorts(); ++index)
                    {
                        node->dynamicPort(index)->eachIncomingConnection([this,&isInput](dagbase::Port* p)
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
                        inputs->a.emplace_back(node);
                    }
                }

                if (node->hasOutputs())
                {
                    for (std::size_t index=0; index<node->totalPorts(); ++index)
                    {
                        node->dynamicPort(index)->eachOutgoingConnection([this,&isOutput](dagbase::Port* p)
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
                        outputs->a.emplace_back(node);
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
                        internals->a.emplace_back(node);
                    }
                }
            }
        }
    }

    void dag::SelectionLive::reconnectInputs(NodeArray& inputs, dagbase::Node* newSource, dagbase::KeyGenerator& keyGen)
    {
        for (auto input : inputs)
        {
            input->reconnectInputs(_selection.m, newSource, keyGen);
        }
    }

    void dag::SelectionLive::reconnectOutputs(NodeArray& outputs, dagbase::Node* newSink, dagbase::KeyGenerator& keyGen)
    {
        for (auto output : outputs)
        {
            output->reconnectOutputs(_selection.m, newSink, keyGen);
        }
    }

    dagbase::Variant SelectionLive::find(std::string_view path) const
    {
        dagbase::Variant retval;

        retval = dagbase::findInternal(path, "inputs", _inputs);
        if (retval.has_value())
            return retval;

        retval = dagbase::findInternal(path, "outputs", _outputs);
        if (retval.has_value())
            return retval;

        retval = dagbase::findInternal(path, "internals", _internals);
        if (retval.has_value())
            return retval;

        return {};
    }
}
