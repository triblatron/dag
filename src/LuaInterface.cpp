#include "config/config.h"

#include "LuaInterface.h"
#include "ReptolCommand.h"
#include "CommandRepository.h"

namespace nbe
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

    Coroutine::Coroutine(Lua& lua)
    :
    _lua(lua),
    _commands(nullptr),
    _thread(nullptr),
    _errod(NoError)
    {
        _thread = lua_newthread(lua.get());
        if (!lua_isthread(lua.get(),-1))
        {
            _errod = FailedToCreateThread;
        }
    }

    int Coroutine::run(int nargs, const std::string& name, ReptolInterface& reptol)
    {
        if (name.empty()==false)
        {
            lua_getglobal(_thread, name.c_str());
            if (!lua_isfunction(_thread,-1))
            {
                _errod = FunctionNotFound;
                return LUA_ERRRUN;
            }
            return runImpl(nargs, reptol);
        }
        else
        {
            _errod = FunctionNotSpecified;
            return LUA_ERRRUN;
        }
    }

    int Coroutine::run(int nargs, int ref, ReptolInterface & reptol)
    {
        if (ref!=LUA_REFNIL)
        {
            lua_rawgeti(_thread, LUA_REGISTRYINDEX, ref);
            if (!lua_isfunction(_thread,-1))
            {
                _errod = FunctionNotFound;
                return LUA_ERRRUN;
            }
            return runImpl(nargs, reptol);
        }
        else
        {
            _errod = FunctionNotSpecified;
            return LUA_ERRRUN;
        }
    }

    int Coroutine::runImpl(int nargs, ReptolInterface& reptol)
    {
        bool finished = false;
        while (!finished)
        {
            int nres = 0;
            int returnCode = lua_resume(_thread, nullptr, nargs, &nres);
            switch (returnCode)
            {
            case LUA_YIELD:
                {
                    if (nres >= 1)
                    {
                        int code = lua_tointeger(_thread, 1);
                        if (_commands!=nullptr)
                        {
                            ReptolCommand * cmd = _commands->commandForCode(code);
                            if (cmd != nullptr)
                            {
                                cmd->makeItSo(_thread, reptol);
                            }
                        }
                        lua_pop(_thread,1);
                    }
                    break;
                }
            case LUA_OK:
                return LUA_OK;
            }
        }
        return LUA_OK;
    }
}
