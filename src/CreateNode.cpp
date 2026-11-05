//
// Created by tony on 29/02/24.
//
#include "config/config.h"

#include "CreateNode.h"
#include "../thirdparty/dagbase/include/core/Graph.h"
#include "../thirdparty/dagbase/include/core/Node.h"

namespace dag
{
    void CreateNode::makeItSo(const std::pmr::vector<dagbase::Variant>& args, std::pmr::vector<dagbase::Variant>* results)
    {
        if (_graph!=nullptr)
        {
            _node = _graph->createNode(args[0].asString(), args[1].asString());
            _className = args[0].asString();
            _name = args[1].asString();
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
            _graph->deleteNode(_node);
            delete _node;
        }
    }

    void CreateNode::redo()
    {
        makeItSo(std::pmr::vector<dagbase::Variant>{_className, _name}, nullptr);
    }

    CreateNode *CreateNode::clone() const
    {
        return new CreateNode(*this);
    }
}
