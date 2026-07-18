//
// Created by tony on 10/02/24.
//

#pragma once

#include "config/Export.h"
#include "SelectionInterface.h"
#include "core/Variant.h"

#include <string_view>

namespace dag
{
    class DAG_API SelectionLive : public SelectionInterface
    {
    public:
        SelectionLive() = default;

        std::size_t count() override;

        void add(Cont::iterator begin, Cont::iterator end) override;

        void add(dagbase::Node* node) override;

        void subtract(Cont::iterator begin, Cont::iterator end) override;

        void set(Cont::iterator begin, Cont::iterator end) override;

        void toggle(Cont::iterator begin, Cont::iterator end) override;

        void clear() override;

        bool isSelected(dagbase::Node* node) override;

        const NodeArray& inputs() const
        {
            return _inputs;
        }

        const NodeArray& outputs() const
        {
            return _outputs;
        }

        const NodeArray& internals() const
        {
            return _internals;
        }

        const NodeArray& externalInputs() const
        {
            return _externalInputs;
        }

        const NodeArray& externalOutputs() const
        {
            return _externalOutputs;
        }

        void reconnectInputs(dagbase::Node* newSource, dagbase::KeyGenerator& keyGen) override;

        void reconnectOutputs(dagbase::Node* newSink, dagbase::KeyGenerator& keyGen) override;

        dagbase::Variant find(std::string_view path) const;
    private:
        void computeBoundaryNodes();
        Cont _selection;
        NodeArray _inputs;
        NodeArray _outputs;
        NodeArray _internals;
        NodeArray _externalInputs;
        NodeArray _externalOutputs;
    };

}
