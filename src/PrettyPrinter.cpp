#include "config/config.h"

#include "PrettyPrinter.h"

#include <iostream>

namespace nbe
{
    nbe::PrettyPrinter & PrettyPrinter::println(const std::string & line)
    {
        printIndent();
        _str << line << '\n';

        return *this;
    }

    nbe::PrettyPrinter & PrettyPrinter::printIndent()
    {
        for (int i=0; i<_indent; ++i)
        {
            for (int j=0; j<_tabSize; ++j)
            {
                _str << " ";
            }
        }

        return *this;
    }
}
