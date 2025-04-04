#pragma once

#include "config/Export.h"

namespace dag
{
    class DAG_API Action
    {
    public:
        Action() = default;

        Action(const Action&) = default;

        Action(Action&&) = default;

        Action& operator=(const Action&) = default;

        Action& operator=(Action&&) = default;

        virtual ~Action() = default;


        virtual void makeItSo() = 0;
    };
}
