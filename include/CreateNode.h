//
// Created by tony on 29/02/24.
//

#pragma  once

#include "config/Export.h"

#include "core/Command.h"

#include <string>

namespace dagbase
{
    class Node;
    class Graph;
}

namespace dag
{
    class NodeEditorLive;

    class DAG_API CreateNode : public dagbase::Command
    {
    public:
        CreateNode() = default;

        void setNodeEditor(dag::NodeEditorLive* nodeEditor)
        {
            _nodeEditor = nodeEditor;
        }

        void setClassName(std::string const & className)
        {
            _className = className;
        }

        void setName(std::string const & name)
        {
            _name = name;
        }

        dagbase::NodeID nodeID()
        {
            return _nodeID;
        }

        void reset() override;

        dagbase::Status makeItSo(const std::pmr::vector<dagbase::Variant>& args, std::pmr::vector<dagbase::Variant>* results) override;

        dagbase::Status undo() override;

        dagbase::Status redo(std::pmr::vector<dagbase::Variant>* results) override;

        [[nodiscard]]CreateNode* clone() const override;
    private:
        NodeEditorLive* _nodeEditor{nullptr};
        std::string _className;
        std::string _name;
        dagbase::NodeID _nodeID{dagbase::NodeID::INVALID_ID};
    };
}
