//
// Created by tony on 12/03/24.
//

#include "config/config.h"

#include "DebugPrinter.h"

#include <iostream>

namespace nbe
{
    DebugPrinter &DebugPrinter::println(std::string line)
    {
        if (_str != nullptr)
        {
            for (std::uint32_t i=0; i<_indentLevel; ++i)
            {
                for (std::uint32_t j=0; j<_spacesPerIndent; ++j)
                {
                    (*_str) << ' ';
                }
            }
            (*_str) << line;
            (*_str) << '\n';
        }

        return *this;
    }
}