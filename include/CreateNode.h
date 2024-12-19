//
// Created by tony on 29/02/24.
//

#pragma  once

#include "config/Export.h"

#include "Command.h"

#include <string>

namespace dag
{
    class Graph;
    class Node;

    class DAG_API CreateNode : public Command
    {
    public:
        CreateNode() = default;

        void setGraph(Graph* graph)
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

        Node* node()
        {
            return _node;
        }

        void makeItSo() override;

        void undo() override;

        [[nodiscard]]CreateNode* clone() const override;
    private:
        Graph* _graph{nullptr};
        std::string _className;
        std::string _name;
        Node* _node{nullptr};
    };
}
