#include "config/config.h"

#include "Class.h"
#include "Fields.h"
#include "lua_runtime.h"

namespace nbe
{
    Class::Class(MetaClass * metaClass, Lua & lua)
    :
    _selfRef(LUA_NOREF),
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

    void Class::createSelfRef()
    {
        swig_type_info * type = ::SWIG_TypeQuery(_lua.get(), typeName());

        if (type == 0)
        {
            raiseError(TypeNotFound) << '\"' << typeName() << '\"';
            return;
        }

        SWIG_NewPointerObj(_lua.get(), this, type, 0);
        // Create a unique key for this instance.
        _selfRef = luaL_ref(_lua.get(), LUA_REGISTRYINDEX);
    }

    void Class::putRef()
    {
        lua_rawgeti(_lua.get(), LUA_REGISTRYINDEX, _selfRef);
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

    void Class::registerEventHandler( const std::type_info & type, EventHandler * handler )
    {
        _handlers.insert( TypeToHandlerMap::value_type( &type, handler ) );
    }
}
