#pragma once

#include "config/Export.h"

#include "core/LuaInterface.h"

#include <vector>
#include <sstream>
#include <map>
#include <typeinfo>

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
        Class(MetaClass * metaClass, dagbase::Lua & lua);

        virtual ~Class();

        std::ostringstream & raiseError(int code);

        void setField( size_t index, lua_Integer value );
        void setField( size_t index, lua_Number value );

        void getField( size_t index, lua_Integer * value );
        void getField( size_t index, lua_Number * value );

    protected:

    private:
        Error _errod;
        std::ostringstream _errorStr;
        MetaClass * _metaClass;
        dagbase::Lua & _lua;
        typedef std::vector<Field*> FieldArray;
        FieldArray _fields;
    };
}
