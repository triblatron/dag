#pragma once

#include "config/Export.h"

#include <string>

namespace dag
{
    class Lua;

    class DAG_API MetaField
    {
    public:
        enum DataType
        {
            TYPE_NIL,
            TYPE_INTEGER,
            TYPE_NUMBER,
            TYPE_STRING
        };
    public:
        MetaField( Lua & lua, const std::string & name, DataType type );
    private:
        Lua & _lua;
        std::string _name;
        DataType _type;
    };
}
