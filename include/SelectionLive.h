//
// Created by tony on 10/02/24.
//

#pragma once

#include "config/Export.h"
#include "SelectionInterface.h"

namespace dag
{
    class DAG_API SelectionLive : public SelectionInterface
    {
    public:
        SelectionLive() = default;

        std::size_t count() override;

        void add(Cont::iterator begin, Cont::iterator end) override;

        void add(Node* node) override;

        void subtract(Cont::iterator begin, Cont::iterator end) override;

        void set(Cont::iterator begin, Cont::iterator end) override;

        void toggle(Cont::iterator begin, Cont::iterator end) override;

        bool isSelected(Node* node) override;

        void computeBoundaryNodes(NodeArray* inputs, NodeArray* outputs, NodeArray* internals) override;

        void reconnectInputs(NodeArray& inputs, Node* newSource) override;

        void reconnectOutputs(NodeArray& outputs, Node* newSink) override;
    private:
        Cont _selection;
    };

}