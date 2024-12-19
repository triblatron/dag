#include "config/config.h"

#include "MetaCoroutine.h"

#include "LuaInterface.h"

namespace dag
{
    MetaCoroutine::MetaCoroutine(Lua & lua)
    :
    _errod(NoError),
    _lua(lua)
    {
        _thread = lua_newthread(_lua.get());
        lua_pop(_lua.get(),1);
        // Transfer function to our thread.
        lua_xmove(_lua.get(),_thread,1);
        if ( !lua_isfunction(_thread,-1) )
        {
            _errod = FailedToMoveFunction;
        }
    }

    int MetaCoroutine::resume(int* nres)
    {
        return lua_resume(_thread,NULL,0, nres);
    }
}
