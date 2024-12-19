//
// Created by tony on 29/02/24.
//
#include "config/config.h"

#include "CreateNode.h"
#include "Graph.h"
#include "Node.h"

namespace dag
{
    void CreateNode::makeItSo()
    {
        if (_graph!=nullptr)
        {
            _node = _graph->createNode(_className, _name);
            if (_node != nullptr)
            {
                _graph->addNode(_node);
            }
        }
    }

    void CreateNode::undo()
    {
        if (_graph!=nullptr && _node!=nullptr)
        {
            _graph->removeNode(_node);
            delete _node;
        }
    }

    CreateNode *CreateNode::clone() const
    {
        return new CreateNode(*this);
    }
}
