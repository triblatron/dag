#pragma once

#include "config/Export.h"

#include "core/Variant.h"

#include <vector>

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

        virtual void makeItSo(const std::pmr::vector<dagbase::Variant>& args, std::pmr::vector<dagbase::Variant>* results) = 0;
    };
}
