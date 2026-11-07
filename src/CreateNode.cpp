//
// Created by tony on 29/02/24.
//
#include "config/config.h"

#include "CreateNode.h"
#include "NodeEditorLive.h"

namespace dag
{
    void CreateNode::reset()
    {
        Command::reset();
        _nodeID = dagbase::NodeID::INVALID_ID;
        _className.clear();
        _name.clear();
    }

    dagbase::Status CreateNode::makeItSo(const std::pmr::vector<dagbase::Variant>& args, std::pmr::vector<dagbase::Variant>* results)
    {
        dagbase::Status status{dagbase::Status::STATUS_UNKNOWN};
        if (_nodeEditor!=nullptr)
        {
            _className = args[0].asString();
            _name = args[1].asString();
            if (_className.empty() || _name.empty())
                return dagbase::Status{dagbase::Status::STATUS_SYNTAX_ERROR};

            status = _nodeEditor->createNode(_className, _name);

            if (status.status == dagbase::Status::STATUS_OK && status.resultType == dagbase::Status::RESULT_NODE_ID && status.result.has_value())
            {
                _nodeID = std::get<dagbase::Status::RESULT_NODE_ID>(status.result.value());

                status.status = dagbase::Status::STATUS_OK;
            }
        }
        else
        {
            status.status = dagbase::Status::STATUS_INTERNAL_ERROR;
        }

        return status;
    }

    dagbase::Status CreateNode::undo()
    {
        dagbase::Status status{dagbase::Status::STATUS_UNKNOWN};
        if (_nodeEditor!=nullptr && _nodeID!=dagbase::NodeID::INVALID_ID)
        {
            status = _nodeEditor->deleteNode(_nodeID);
        }
        else
        {
            status = dagbase::Status{dagbase::Status::STATUS_INTERNAL_ERROR};
        }

        return status;
    }

    dagbase::Status CreateNode::redo(std::pmr::vector<dagbase::Variant>* results)
    {
        return makeItSo(std::pmr::vector<dagbase::Variant>{_className, _name}, results);
    }

    CreateNode *CreateNode::clone() const
    {
        return new CreateNode(*this);
    }
}
