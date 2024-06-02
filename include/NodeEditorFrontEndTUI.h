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
            //! Create a node with a given name, class
            OP_CREATE_NODE = 1<<2,
            //! Delete a Node by name
            OP_DELETE_NODE = 1<<3,
            //! Add a dynamic Port to a Node
            OP_CREATE_PORT = 1<<4,
            //! Delete a dynamic Port from a Node
            OP_DELETE_PORT = 1<<5,
            //! Create a SignalPath linking two Ports
            OP_CREATE_SIGNALPATH = 1<<6,
            //! Delete a SignalPath
            OP_DELETE_SIGNALPATH = 1<<7,
            //! Create a child Graph from the selection
            OP_CREATE_CHILD = 1<<8,
            //! Delete a child Graph
            OP_DELETE_CHILD = 1<<9,
            //! Select a list of Nodes using the given mode
            OP_SELECT = 1<<10,
            //! Run the graph.
            OP_RUN_BIT  = 1<<11,
            //! Undo the last operation
            OP_UNDO_BIT = 1<<12,
            //! Redo the last undone operation
            OP_REDO_BIT = 1<<13,
            //! All operations.
            OP_ALL_BITS = ~0U
        };
    public:
        void getNextCommand(NodeEditorInterface& backEnd) override;

        void onCommandCompleted(const NodeEditorResult& result) override;
    private:
        void allow(std::uint32_t mask)
        {
            _allowedOperations |= mask;
        }

        void disallow(std::uint32_t mask)
        {
            _allowedOperations &= ~mask;
        }

        bool isOperationAllowed(std::uint32_t mask) const
        {
            return (_allowedOperations & mask)!=0x0;
        }

        std::uint32_t _allowedOperations{OP_ALL_BITS};

        void setInProgress(std::uint32_t mask)
        {
            _inProgressOperation = mask;
        }

        bool isInProgres(std::uint32_t mask) const
        {
            return _inProgressOperation == mask;
        }

        std::uint32_t _inProgressOperation{0x0};
    };
}
