//
// Created by tony on 10/02/24.
//

#pragma  once

#include "config/Export.h"

#include "../thirdparty/dagbase/include/core/Types.h"
#include "config/config.h"
#include "../thirdparty/dagbase/include/core/Node.h"

#include <cstdint>
#include <vector>
#include <algorithm>

namespace dagbase
{
    class Node;
}

namespace dag
{
    class DAG_API SelectionInterface
    {
    public:
        typedef dagbase::NodeSet Cont;
        typedef std::vector<dagbase::Node*> NodeArray;

    public:
        virtual ~SelectionInterface() = default;

        virtual std::size_t count() = 0;

        virtual void add(Cont::iterator begin, Cont::iterator end) = 0;

        virtual void add(dagbase::Node* node) = 0;

        virtual void subtract(Cont::iterator begin, Cont::iterator end) = 0;

        virtual void set(Cont::iterator begin, Cont::iterator end) = 0;

        virtual void toggle(Cont::iterator begin, Cont::iterator end) = 0;

        virtual bool isSelected(dagbase::Node* node) = 0;

        virtual void computeBoundaryNodes(NodeArray* inputs, NodeArray* outputs, NodeArray* internals) = 0;

        virtual void reconnectInputs(NodeArray& inputs, dagbase::Node* newSource) = 0;

        virtual void reconnectOutputs(NodeArray& outputs, dagbase::Node* newSink) = 0;
    };
}
