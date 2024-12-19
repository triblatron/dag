#include "config/config.h"

#include "MetaField.h"

namespace dag
{
    MetaField::MetaField( Lua & lua, const std::string & name, DataType type )
    :
    _lua(lua),
    _name(name),
    _type(type)
    {
        // Do nothing.
    }
}
