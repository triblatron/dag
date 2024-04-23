#pragma once

#include "config/Export.h"

#include "Action.h"

namespace nbe
{
    class NODEBACKEND_API Command : public Action
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
