#include "config/config.h"

#include "Class.h"
#include "Fields.h"

namespace dag
{
    Class::Class(MetaClass * metaClass, Lua & lua)
    :
    _errod(NoError),
    _metaClass(metaClass),
    _lua(lua)
    {
        // Do nothing.
    }

    Class::~Class()
    {
        // TODO:Delete fields and operations.
    }

    std::ostringstream & Class::raiseError( int code )
    {
        switch ( code )
        {
        case NoError:
            break;
        case TypeNotFound:
            _errorStr << "TypeNotFound:";
            break;
        }

        return _errorStr;
    }

    void Class::setField( size_t index, lua_Integer value )
    {
        _fields[index]->setValue(value);
    }

    void Class::setField( size_t index, lua_Number value )
    {
        _fields[index]->setValue(value);
    }

    void Class::getField( size_t index, lua_Integer * value )
    {
        *value = _fields[index]->toInteger();
    }

    void Class::getField( size_t index, lua_Number * value )
    {
        *value = _fields[index]->toNumber();
    }
}
