#pragma once

#include "config/Export.h"

#include <iosfwd>
#include <string>

namespace nbe
{
    //! \class PrettyPrinter
    //! An aid to debugging node graphs
    //! Supports indentation and printing lines.
    class NBE_API PrettyPrinter
    {
    public:
        PrettyPrinter(std::ostream & str, int tabSize)
        :
        _str(str),
        _indent(0),
        _tabSize(tabSize)
        {
            // Do nothing.
        }

        //! Increase indentation by one.
        nbe::PrettyPrinter & indent()
        {
            _indent++;

            return *this;
        }

        //! Decrease indentation by one.
        nbe::PrettyPrinter & outdent()
        {
            _indent--;

            return *this;
        }

        //! \return The current indentation level.
        [[nodiscard]]int indentation() const
        {
            return _indent;
        }

        //! \return The number of spaces per indentation level.
        [[nodiscard]]int tabSize() const
        {
            return _tabSize;
        }

        //! Print a line indented by the indentation level * tabSize spaces.
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
