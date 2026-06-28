//
// Created by tony on 10/02/24.
//

#pragma  once

#include "config/Export.h"

#include "core/Node.h"
#include "core/Types.h"

#include <vector>

#include "util/SearchableArray.h"

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
        typedef dagbase::SearchableArray<std::vector<dagbase::Node*>>    NodeArray;

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

        virtual void reconnectInputs(NodeArray& inputs, dagbase::Node* newSource, dagbase::KeyGenerator& keyGen) = 0;

        virtual void reconnectOutputs(NodeArray& outputs, dagbase::Node* newSink, dagbase::KeyGenerator& keyGen) = 0;
    };
}
