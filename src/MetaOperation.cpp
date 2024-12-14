#include "config/config.h"

#include "MetaOperation.h"

#include "LuaInterface.h"

namespace dag
{
    MetaOperation::MetaOperation(Lua & lua, int numArgs, int numResults)
    :
    _errod(NoError),
    _lua(lua),
    _ref(LUA_NOREF),
    _numArgs(numArgs),
    _numResults(numResults),
    _thread(0)
    {
        if ( !lua_isfunction(_lua.get(), -1 ) )
        {
            _errod = FunctionNotFound;

            return;;
        }
        _ref = luaL_ref(_lua.get(), LUA_REGISTRYINDEX);
    }

    MetaOperation::~MetaOperation()
    {
        luaL_unref(_lua.get(), LUA_REGISTRYINDEX, _ref);
    }

    void MetaOperation::put()
    {
        lua_rawgeti(_lua.get(), LUA_REGISTRYINDEX, _ref);
    }

    void MetaOperation::invoke()
    {
        put();
        lua_pcall(_lua.get(), _numArgs, _numResults,0);
    }

    int MetaOperation::resume()
    {
        if ( _thread == 0 )
        {
            _thread = lua_newthread(_lua.get());
            lua_pop(_lua.get(),1);
        }
        lua_rawgeti(_thread, LUA_REGISTRYINDEX, _ref);
        int numResults;
        return lua_resume(_thread, NULL, 0, &numResults);
    }
}
