//
// Created by tony on 10/02/24.
//

#pragma  once

#include "config/Export.h"

#include "Types.h"
#include "config/config.h"
#include "Node.h"

#include <cstdint>
#include <vector>
#include <algorithm>

namespace nbe
{
    class Node;

    class NBE_API SelectionInterface
    {
    public:
        typedef NodeSet Cont;
        typedef std::vector<Node*> NodeArray;

    public:
        virtual ~SelectionInterface() = default;

        virtual std::size_t count() = 0;

        virtual void add(Cont::iterator begin, Cont::iterator end) = 0;

        virtual void add(Node* node) = 0;

        virtual void subtract(Cont::iterator begin, Cont::iterator end) = 0;

        virtual void set(Cont::iterator begin, Cont::iterator end) = 0;

        virtual void toggle(Cont::iterator begin, Cont::iterator end) = 0;

        virtual bool isSelected(Node* node) = 0;

        virtual void computeBoundaryNodes(NodeArray* inputs, NodeArray* outputs, NodeArray* internals) = 0;

        virtual void reconnectInputs(NodeArray& inputs, Node* newSource) = 0;

        virtual void reconnectOutputs(NodeArray& outputs, Node* newSink) = 0;
    };
}
