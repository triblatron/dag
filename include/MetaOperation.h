#pragma once

#include "config/Export.h"

struct lua_State;

namespace dagbase
{
    class Lua;
}

namespace dag
{
    class DAG_API MetaOperation
    {
    public:
        enum Error
        {
            NoError,
            FunctionNotFound
        };
    public:
        //! Assumes that the function is at the top of the stack.
        //! Creates a reference to the function in the registry.
        MetaOperation(dagbase::Lua & lua, int numArgs, int numResults);

        ~MetaOperation();

        bool ok() const
        {
            return _errod == NoError;
        }

        lua_State * thread()
        {
            return _thread;
        }

        //! Put the function on the top of the Lua stack.
        void put();

        //! Expects the arguments on the stack, pushes the results.
        void invoke();

        //! Expects the function and arguments on the stack.
        int resume();
    private:
        Error _errod;
        dagbase::Lua & _lua;
        //! Lua reference to the function.
        int _ref;
        int _numArgs;
        int _numResults;
        lua_State * _thread;
    };
}
