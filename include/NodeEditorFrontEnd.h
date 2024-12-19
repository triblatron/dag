//
// Created by tony on 08/05/24.
//

#pragma once

#include "config/Export.h"

namespace dag
{
    class NodeEditorInterface;

    struct DAG_API NodeEditorResult
    {
        enum Result
        {
            RESULT_OK,
            RESULT_ERR_INVALID_ARGUMENT,
            RESULT_UNKNOWN
        };

        Result result{RESULT_UNKNOWN};
    };

    class DAG_API NodeEditorFrontEnd
    {
    public:
        virtual ~NodeEditorFrontEnd() = default;

        virtual void getNextCommand(NodeEditorInterface& backEnd) = 0;

        virtual void onCommandCompleted(const NodeEditorResult & result) = 0;
    };
}
