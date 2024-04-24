#pragma once

#include "config/Export.h"

#include <iosfwd>
#include <string>

namespace nbe
{
    class NBE_API PrettyPrinter
    {
    public:
        PrettyPrinter(std::ostream & str, int tabSize)
        :
        _str(str),
        _indent(0),
        _tabSize(tabSize)
        {
        }

        nbe::PrettyPrinter & indent()
        {
            _indent++;

            return *this;
        }

        nbe::PrettyPrinter & outdent()
        {
            _indent--;

            return *this;
        }

        nbe::PrettyPrinter & println(const std::string & line);

        nbe::PrettyPrinter & printIndent();

        std::ostream & str()
        {
            return _str;
        }
    private:
        std::ostream & _str;
        int _indent;
        int _tabSize;

    };

    template<class T>
    PrettyPrinter & operator<<(PrettyPrinter & printer, const T & value)
    {
        printer.str() << value;

        return printer;
    }
}
