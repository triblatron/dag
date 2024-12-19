//
// Created by tony on 09/03/24.
//

#pragma once

#include "config/Export.h"

#include <cstdint>
#include <cstdlib>

namespace dag
{
    class DAG_API Stream
    {
    public:
        typedef std::uint8_t value_type;
        typedef std::size_t ObjId;
        typedef void* Ref;
    public:
        virtual ~Stream() = default;
    };
}
