#pragma once

#include "config/Export.h"

#include "Action.h"

namespace dag
{
    //! A base class for commands using the Command pattern
    //! This is effectively a kind of node but does not inherit Node
    //! An implementation must be default-constructible for a
    //! common interface.  It must also support setting arguments
    //! via a generic interface.  This does not have to be
    //! as efficient as for a Node, so setting arguments by
    //! name is desirable.
    class DAG_API Command : public Action
    {
    public:
        virtual ~Command() = default;

        virtual Command * clone() const = 0;

        virtual bool isUndoable() const
        {
            return true;
        }

        //! Use stored state to undo a call to makeItSo().
        virtual void undo() = 0;

        //! Provides default implementation that just calls makeItSo().
        virtual void redo();
    };
}
