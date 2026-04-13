//
// Created by tony on 29/02/24.
//

#pragma  once

#include "config/Export.h"

#include "Command.h"

#include <string>

namespace dagbase
{
    class Node;
    class Graph;
}

namespace dag
{

    class DAG_API CreateNode : public Command
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

        void makeItSo() override;

        void undo() override;

        [[nodiscard]]CreateNode* clone() const override;
    private:
        dagbase::Graph* _graph{nullptr};
        std::string _className;
        std::string _name;
        dagbase::Node* _node{nullptr};
    };
}
