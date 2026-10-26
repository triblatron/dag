//
// Created by Tony Horrobin on 11/10/2026.
//

#pragma once

#include "config/Export.h"

#include <cstdint>

namespace dagui
{
    enum Action : std::uint32_t
    {
        ACTION_UNKNOWN,
        ACTION_ADD_PORT,
        ACTION_DELETE_PORT
    };

    const char DAG_API * actionToString(Action value);
    Action DAG_API parseAction(const char* str);

    enum Namespace : std::uint32_t
    {
        NAMESPACE_UNKNOWN,
        //! Internal, reserved names
        NAMESPACE_SYSTEM,
        //! Application-level names
        NAMESPACE_USER
    };

    const char DAG_API * namespaceToString(Namespace value);
    Namespace DAG_API parseNamespace(const char* str);
}
