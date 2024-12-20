#pragma once

#include "config/Export.h"

#include "core/LuaInterface.h"

#include <lua.h>
#include <vector>

namespace dag
{
    class MetaField;
    class Route;

    class DAG_API Field
    {
    public:
        //! Assumes that a value is on the top of the Lus stack.
        Field( MetaField * metaField, dagbase::Lua & lua )
        :
        _metaField(metaField),
        _lua(lua)
        {
            _ref = luaL_ref(_lua.get(), LUA_REGISTRYINDEX);
        }

        ~Field()
        {
            luaL_unref(_lua.get(), LUA_REGISTRYINDEX, _ref);
        }

        const MetaField * metaField() const
        {
            return _metaField;
        }

        void setValue( lua_Integer value )
        {
            lua_pushinteger(_lua.get(), value);
            lua_rawseti(_lua.get(), LUA_REGISTRYINDEX, _ref);
        }

        void setValue( lua_Number value )
        {
            lua_pushnumber(_lua.get(), value);
            lua_rawseti(_lua.get(), LUA_REGISTRYINDEX, _ref);
        }

        lua_Integer toInteger()
        {
            lua_rawgeti(_lua.get(), LUA_REGISTRYINDEX, _ref);
            lua_Integer retval = lua_tointeger(_lua.get(), -1);
            lua_pop(_lua.get(),1);

            return retval;
        }

        lua_Number toNumber()
        {
            lua_rawgeti(_lua.get(), LUA_REGISTRYINDEX, _ref);
            lua_Number retval = lua_tonumber(_lua.get(), -1);
            lua_pop(_lua.get(),1);

            return retval;
        }
    private:
        //! Meta-data.
        MetaField * _metaField;
        dagbase::Lua & _lua;
        //! Lua reference to value.
        int _ref;
        typedef std::vector<Route *> RouteArray;
        RouteArray _outgoingRoutes;
        RouteArray _incomingRoutes;
    };
}
