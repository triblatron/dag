//
// Created by Tony Horrobin on 11/10/2026.
//

#include "config/config.h"

#include "../include/Scripting.h"
#include "util/enums.h"

namespace dagui
{
    const char * actionToString(Action value)
    {
        switch (value)
        {
            ENUM_NAME(ACTION_UNKNOWN)
            ENUM_NAME(ACTION_ADD_PORT)
            ENUM_NAME(ACTION_DELETE_PORT)
        }

        return "<error>";
    }

    Action parseAction(const char *str)
    {
        TEST_ENUM(ACTION_UNKNOWN, str)
        TEST_ENUM(ACTION_ADD_PORT, str)
        TEST_ENUM(ACTION_DELETE_PORT, str)

        return ACTION_UNKNOWN;
    }

    const char * namespaceToString(Namespace value)
    {
        switch (value)
        {
            ENUM_NAME(NAMESPACE_UNKNOWN)
            ENUM_NAME(NAMESPACE_SYSTEM)
            ENUM_NAME(NAMESPACE_USER)
        }

        return "<error>";
    }

    Namespace parseNamespace(const char *str)
    {
        TEST_ENUM(NAMESPACE_UNKNOWN, str)
        TEST_ENUM(NAMESPACE_SYSTEM, str)
        TEST_ENUM(NAMESPACE_USER, str)

        return NAMESPACE_UNKNOWN;
    }
}
