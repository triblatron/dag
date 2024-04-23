//
// Created by tony on 12/03/24.
//

#pragma once

#include "config/Export.h"

#include <iosfwd>
#include <cstdint>
#include <string>

namespace nbe
{
    class NODEBACKEND_API DebugPrinter
    {
    public:
        void setStr(std::ostream* str)
        {
            _str = str;
        }

        void indent()
        {
            ++_indentLevel;
        }

        void outdent()
        {
            if (_indentLevel>0)
            {
                --_indentLevel;
            }
        }

        template<typename T>
        DebugPrinter& print(T value)
        {
            if (_str!=nullptr)
            {
                (*_str) << value;
            }

            return *this;
        }
        DebugPrinter& println(std::string line);
    private:
        std::ostream* _str{nullptr};
        std::uint32_t _indentLevel{0};
        std::uint32_t _spacesPerIndent{2};
    };
}