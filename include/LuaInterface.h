#pragma once

#include "config/Export.h"

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

#include "errod.h"

#include <filesystem>

#include <stdexcept>
#include <iostream>
#include <cassert>
#include <sstream>
#include <type_traits>
#include <functional>

namespace nbe
{
    class NBE_API Function
    {
    public:
        //! Push the function onto the stack if it exists.
        //! \note balance defaults to false because we will typically want
        //! to call the function or store it as a reference which will pop the value.
        Function(lua_State * L, const char * name, bool balance=false)
            :
            _lua(L),
            _balance(balance)
        {
            lua_pushstring(_lua, name);
            lua_rawget(_lua, -2);
            if (!lua_isfunction(_lua, -1))
            {
                throw std::runtime_error(name);
            }
        }

        //! Pop the Lua stack if balance is true.
        ~Function()
        {
            if (_balance)
            {
                lua_pop(_lua, 1);
            }
        }
    private:
        lua_State * _lua;
        bool _balance;
    };

    class NBE_API TableTraversal
    {
    public:
        explicit TableTraversal( lua_State * lua )
            :
        _lua(lua)
        {
            assert( lua_istable( _lua, -1 ) );

            _oldTop = lua_gettop(_lua);
        }

        //! We want a very loud failure if the stack is not balanced
        //! at this point.
        ~TableTraversal()
        {
            assert( _oldTop == lua_gettop(_lua) );
        }

        typedef int (*Handler)(lua_State *, void *);
        int operator()( Handler handler, void * userData );

        template<typename F>
        int operator()(F f)
        {        // Dummy key to prepare for first call to lua_next().
            lua_pushnil(_lua);

            while (lua_next(_lua, -2) != 0)
            {
                int resultCode = std::invoke(f, _lua);
                if (resultCode != 0)
                {
                    // Pop the remaining key and value.
                    lua_pop(_lua, 2);
                    return resultCode;
                }
                if (lua_istable(_lua, -1))
                {
                    resultCode = (*this)(f);
                }
                // Pop the value.
                lua_pop(_lua, 1);
                if (resultCode != 0)
                {
                    // Pop the remaining key.
                    lua_pop(_lua, 1);
                    return resultCode;
                }
            }

            return 0;
        }
    private:
        lua_State * _lua;
        int _oldTop;
    };

    class NBE_API Table
    {
    public:
        explicit Table(lua_State *L,const char *name, bool cleanup = true);

        explicit Table(lua_State *L, int index, bool cleanup = true)
            :
            _lua(L),
            _cleanup(cleanup)
        {
            lua_pushinteger(L, index);
            lua_rawget(L, -2);
            if (!lua_istable(L, -1))
            {
                // Restore balance.
                lua_pop(L, 1);
                lua_newtable(L);
            }
        }

        explicit Table(lua_State *L, bool cleanup)
            :
            _lua(L),
            _cleanup(cleanup)
        {
            assert( _lua != 0 );
            assert( lua_istable( _lua, -1 ) );
        }

        static Table global(lua_State* L, const char* name)
        {
            lua_getglobal(L, name);
            if (!lua_istable(L, -1))
            {
                lua_newtable(L);
                lua_setglobal(L, name);
            }
            return Table(L, true);
        }

        ~Table()
        {
            // Restore balance to the stack.
            // We do not own the lua_State instance.
            if (_cleanup)
            {
                lua_pop(_lua, 1);
            }
        }

        lua_State * get()
        {
            return _lua;
        }

        int length() const
        {
            lua_len(_lua, -1);
            int len = lua_tointeger(_lua, -1);
            lua_pop(_lua, 1);

            return len;
        }

        bool isTable(const char * name)
        {
            lua_pushstring(_lua, name);
            lua_rawget(_lua, -2);
            bool retval = lua_istable(_lua, -1);

            lua_pop(_lua, 1);

            return retval;
        }

        Table tableForName(const char * name)
        {
            return Table(_lua,name);
        }

        Table tableForIndex(int index)
        {
            return Table(_lua, index);
        }

        double number(int i, double defaultValue)
        {
            lua_pushinteger(_lua, i);
            lua_rawget(_lua, -2);
            if (lua_isnumber(_lua, -1))
            {
                double value = lua_tonumber(_lua, -1);
                lua_pop(_lua, 1);

                return value;
            }
            else
            {
                lua_pop(_lua, 1);
                return defaultValue;
            }
        }

        double number(const char *key, double defaultValue)
        {
            lua_pushstring(_lua, key);
            lua_rawget(_lua, -2);
            if (lua_isnumber(_lua, -1))
            {
                double value = lua_tonumber(_lua, -1);
                lua_pop(_lua, 1);

                return value;
            }
            else
            {
                lua_pop(_lua, 1);

                return defaultValue;
            }
        }

        int integer(int i, int defaultValue)
        {
            lua_pushinteger(_lua, i);
            lua_rawget(_lua, -2);
            if (lua_isinteger(_lua, -1))
            {
                int val = lua_tointeger(_lua, -1);
                lua_pop(_lua, 1);
                return val;
            }
            else
            {
                lua_pop(_lua, 1);
                return defaultValue;
            }
        }

        int integer(const char * key, int defaultValue)
        {
            lua_pushstring(_lua, key);
            lua_rawget(_lua, -2);
            if ( lua_isinteger(_lua, -1))
            {
                int val = lua_tointeger(_lua, -1);

                lua_pop(_lua, 1);
                return val;
            }
            else
            {
                lua_pop(_lua, 1);

                return defaultValue;
            }
        }

        const char * string(int i, const char * defaultValue)
        {
            lua_pushinteger(_lua, i);
            lua_rawget(_lua, -2);
            if (lua_isstring(_lua, -1))
            {
                const char * value = lua_tostring(_lua, -1);
                lua_pop(_lua, 1);

                return value;
            }
            else
            {
                lua_pop(_lua, 1);

                return defaultValue;
            }
        }

        const char * string(const char * key, const char * defaultValue)
        {
            lua_pushstring(_lua, key);
            lua_rawget(_lua, -2);
            if (lua_isstring(_lua, -1))
            {
                const char * value = lua_tostring(_lua, -1);
                lua_pop(_lua, 1);

                return value;
            }
            else
            {
                lua_pop(_lua, 1);

                return defaultValue;
            }
        }

        bool boolean(int i, bool defaultValue)
        {
            lua_pushinteger(_lua, i);
            lua_rawget(_lua, -2);

            if ( lua_isboolean(_lua,-1) )
            {
                bool b = lua_toboolean(_lua, -1);

                lua_pop(_lua, 1);

                return b;
            }
            else
            {
                lua_pop(_lua,1);

                return defaultValue;
            }
        }

        bool boolean(const char * key, bool defaultValue)
        {
            lua_pushstring(_lua, key);
            lua_rawget(_lua, -2);

            if ( lua_isboolean(_lua,-1) )
            {
                bool b = lua_toboolean(_lua, -1);

                lua_pop(_lua, 1);

                return b;
            }
            else
            {
                lua_pop(_lua,1);

                return defaultValue;
            }
        }

        bool functionExists(const char * key)
        {
            lua_pushstring(_lua, key);
            lua_rawget(_lua, -2);
            bool retVal = lua_isfunction(_lua, -1);
            lua_pop(_lua, 1);

            return retVal;
        }

        //! Put a named function on the top of the stack.
        Function function(const char * key)
        {
            return Function(_lua, key);
        }
    protected:
        explicit Table(lua_State *L)
            :
            _lua(L),
            _cleanup(true)
        {
            assert( _lua != 0 );
        }
    private:
        lua_State * _lua;
        bool _cleanup;
    };

    //class NBE_API LocalTable : public Table
    //{
    //public:
    //  LocalTable(lua_State * L, const char * name)
    //      :
    //      Table(L)
    //  {
    //  }
    //};

    class NBE_API GlobalTable : public Table
    {
    public:
        GlobalTable(lua_State* L, const char* name)
            :
            Table(L)
        {
            lua_getglobal(L, name);
            if (!lua_istable(L, -1))
            {
                lua_newtable(L);
                lua_setglobal(L, name);
            }
        }
    };

    class NBE_API Lua
    {
    public:
        enum Error
        {
            NoError,
            ScriptNotFound,
            FailedToExecute,
            NumErrors
        };
    public:
        Lua()
            :
            _own(true),
            _errod(&_errors[NoError])
        {
            _lua = luaL_newstate();
            luaL_openlibs(_lua);
        }

        explicit Lua(lua_State *L, bool own)
            :
            _lua(L),
            _own(own),
            _errod(&_errors[NoError])
        {
            // Do nothing.
        }

        ~Lua()
        {
            if (_own)
            {
                lua_close(_lua);
            }
        }

        lua_State * get() const
        {
            return _lua;
        }

        void execute(const char * filename)
        {
            if (!std::filesystem::exists(std::string(filename)))
            {
                raiseError(ScriptNotFound) << '\"' << filename << '\"';

                return;
            }

            int errod = luaL_dofile(_lua, filename);

            if (errod != 0)
            {
                raiseError(FailedToExecute) <<  '\"' << filename << "\":" << lua_tostring(_lua,-1) << "\n";
            }
        }

        bool ok() const
        {
            return _errod == &_errors[NoError];
        }

        std::ostringstream & raiseError(Error code)
        {
            _errod = &_errors[code];
            _errorStr << _errod->name << ":description:" << _errod->description;

            return _errorStr;
        }

        std::string errorMessage() const
        {
            return _errorStr.str();
        }

        bool tableExists(const char* name)
        {
            if (name != nullptr)
            {
                lua_getglobal(_lua, name);
                bool exists = lua_istable(_lua, 1);
                lua_pop(_lua, 1);
                return exists;
            }
            return false;
        }

        Table tableForName(const char *name) const
        {
            return Table::global(_lua, name);
        }

        void eval(const char* str)
        {
            if (str!=nullptr)
            {
                bool error = luaL_dostring(_lua, str);

                if (error)
                {
                    _errorStr << lua_tostring(_lua, -1);
                }
            }
        }

    private:
        lua_State * _lua;
        bool _own;
        ErrorDescriptor * _errod;
        std::ostringstream _errorStr;
        static ErrorDescriptor _errors[NumErrors+1];
    };
}
