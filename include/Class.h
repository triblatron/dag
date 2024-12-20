#pragma once

#include "config/Export.h"

#include "core/LuaInterface.h"

#include <iosfwd>
#include <string>

namespace dagbase
{
    class Lua;
}

namespace dag
{
    class Field;
    class MetaClass;
    class Operation;

    //! Base class for classes that have fields and operations.
    class DAG_API Class
    {
    public:
        enum Error
        {
            NoError,
            TypeNotFound
        };
    public:
        explicit Class(MetaClass * metaClass);

        virtual ~Class();

        std::ostringstream & raiseError(int code);

        std::string errorMessage() const;
    private:
        Error _errod;
        std::ostringstream* _errorStr{ nullptr };
        MetaClass * _metaClass{ nullptr };
    };
}
