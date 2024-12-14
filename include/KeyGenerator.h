//
// Created by tony on 23/03/24.
//

#pragma once

#include "config/Export.h"

#include "Types.h"

namespace dag
{
    class DAG_API KeyGenerator
    {
    public:
        virtual ~KeyGenerator() = default;

        virtual NodeID nextNodeID() = 0;

        virtual PortID nextPortID() = 0;
    };
}
