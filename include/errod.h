#pragma once

#include "config/Export.h"

namespace nbe
{
    struct NBE_API ErrorDescriptor
    {
        int code;
        const char * name;
        const char * description;
    };
}
