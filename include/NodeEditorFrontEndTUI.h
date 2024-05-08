//
// Created by tony on 08/05/24.
//

#pragma once

#include "config/Export.h"

#include "NodeEditorFrontEnd.h"

#include <iosfwd>
#include <cstdint>

namespace nbe
{
    class NBE_API NodeEditorFrontEndTUI : public NodeEditorFrontEnd
    {
    public:
        //! The menu we are on
        enum Menu : std::uint32_t
        {
            MENU_MAIN,
            MENU_FILE,
            MENU_EDIT,
            MENU_GRAPH
        };

        //! A set of operations, effectively actions that can be performed
        //! in the current state.
        enum Operation : std::uint32_t
        {
            //! Load a graph from the persistent format.
            OP_LOAD_BIT = 1<<0,
            //! Save a graph to the persistent format.
            OP_SAVE_BIT = 1<<1,
            //! Run the graph.
            OP_RUN_BIT  = 1<<2,
            //! All operations.
            OP_ALL_BITS = ~0U
        };
    public:
        void getNextCommand(NodeEditorInterface& backEnd) override;

        void onCommandCompleted(const NodeEditorResult& result) override;
    private:
        void allow(std::uint32_t mask)
        {
            _availableOperations |= mask;
        }

        void disallow(std::uint32_t mask)
        {
            _availableOperations &= ~mask;
        }

        bool isOperationAvailable(std::uint32_t mask) const
        {
            return (_availableOperations & mask)!=0x0;
        }

        std::uint32_t _availableOperations{OP_ALL_BITS};
    };
}
