#include "config/config.h"

#include "LuaInterface.h"

namespace dag
{
    ErrorDescriptor Lua::_errors[Lua::NumErrors+1]=
    {
        {
            NoError,
            "NoError",
            "The operation was successful"
        },
        {
            ScriptNotFound,
            "ScriptNotFound",
            "The specified script was not found:"
        },
        {
            FailedToExecute,
            "FailedToExecute",
            "The script failed to execute with error:"
        },
        {
            NumErrors,
            "UnknownError",
            "An unknown error"
        }
    };

    int TableTraversal::operator()( Handler handler, void * userData )
    {
        assert( handler != nullptr );

        // Dummy key to prepare for first call to lua_next().
        lua_pushnil( _lua );

        while ( lua_next( _lua, -2 ) != 0 )
        {
            int resultCode = (handler)(_lua,userData);
            // Pop the value.
            lua_pop(_lua,1);
            if ( resultCode != 0 )
            {
                // Pop the remaining key.
                lua_pop(_lua,1);
                return resultCode;
            }
        }

        return 0;
    }

    Table::Table(lua_State* L, const char* name, bool cleanup):
	    _lua(L),
	    _cleanup(cleanup)
    {
	    lua_pushstring(L, name);
	    lua_rawget(L, -2);
	    if (!lua_istable(L, -1))
	    {
		    // Restore balance.
		    lua_pop(L, 1);
		    lua_newtable(L);
	    }
    }
}
