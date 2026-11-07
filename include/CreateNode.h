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

    class DAG_API CreateNode : public dagbase::Command
    {
    public:
        CreateNode() = default;

        void setGraph(dagbase::Graph* graph)
        {
            _graph = graph;
        }

        void setClassName(std::string const & className)
        {
            _className = className;
        }

        void setName(std::string const & name)
        {
            _name = name;
        }

        dagbase::Node* node()
        {
            return _node;
        }

        void reset() override;

        void makeItSo(const std::pmr::vector<dagbase::Variant>& args, std::pmr::vector<dagbase::Variant>* results) override;

        void undo() override;

        void redo() override;

        [[nodiscard]]CreateNode* clone() const override;
    private:
        dagbase::Graph* _graph{nullptr};
        std::string _className;
        std::string _name;
        dagbase::Node* _node{nullptr};
    };
}
