#pragma once

#include "config/Export.h"

namespace dag
{
    struct DAG_API ErrorDescriptor
    {
        int code;
        const char * name;
        const char * description;
    };
}
