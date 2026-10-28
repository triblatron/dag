//
// Created by Tony Horrobin on 11/10/2026.
//

#include "config/config.h"

#include "LuaScript.h"
#include "core/ConfigurationElement.h"
#include "core/LuaInterface.h"
#include "lua_runtime.h"
#include "util/enums.h"
#include "NodeEditorLive.h"
#include "core/Node.h"
#include "core/PortDirection.h"
#include "core/ScriptProtocol.h"
#include "util/Searchable.h"

#include <string>
#include <array>

namespace dag
{
    LuaScript::LuaScript(dagbase::Lua *lua, dagbase::ScriptProtocol* proto)
        :
    _lua(lua),
    _proto(proto)
    {
        // There will typically be one result, so pre-allocate it here.
        _results.reserve(1);
    }

    LuaScript::LuaScript(dagbase::Lua *lua, dagbase::Function *func, dagbase::ScriptProtocol *proto)
        :
    _lua(lua),
    _func(func),
    _proto(proto)
    {
        if (_func && _lua)
        {
            _func->push();
            _thread = dagbase::Coroutine::fromFunction(_func->lua());
            //assert(lua_isfunction(_thread->get(), -1));
        }
    }

    LuaScript::LuaScript(const LuaScript &other)
        :
    _lua(other._lua),
    _func(other._func),
    _proto(other._proto)
    {
        if (other._thread && _lua)
        {
            _thread = other._thread->clone(*_lua);
        }
    }

    LuaScript::~LuaScript()
    {
        delete _thread;
    }

    void LuaScript::configure(dagbase::ConfigurationElement &config)
    {
        if (auto element = config.findElement("code"); element)
        {
            if (auto func = element->asFunction(); func)
            {
                func->push();
                _thread = dagbase::Coroutine::fromFunction(_lua->get());
            }
            else if (auto name = element->asString(); !name.empty())
            {
                _mainName = name;
            }
        }
        if (!_mainName.empty())
        {
            if (!_thread)
            {
                _thread = dagbase::Coroutine::fromNamedFunction(_lua->get(), _mainName.c_str());
                if (!_thread)
                {
                    setState(STATE_THREAD_ERROR);
                }
            }
            else
            {
                _thread->setMainFunction(_mainName.c_str());
                if (!_thread->ok())
                {
                    setState(STATE_THREAD_ERROR);
                }
            }
        }
        //if (!_thread || !_thread->ok())
        //{
        //    setState(STATE_THREAD_ERROR);
        //}
    }

    void LuaScript::setPortValues(dagbase::Node* self, int numResults)
    {
        if (_thread)
        {
            std::size_t portIndex=0;
            if (portIndex<self->totalPorts())
            {
                for (int i=1; i<=numResults; ++i)
                {
                    dagbase::Variant v;
                    v.fromLua(*_thread, i);
                    auto value = v.cast(dagbase::Variant::TYPE_VALUE).as<dagbase::Value>();
                    if (portIndex<self->totalPorts() && (self->dynamicPort(portIndex)->dir() == dagbase::PortDirection::DIR_INTERNAL || self->dynamicPort(portIndex)->dir() == dagbase::PortDirection::DIR_OUT))
                    {
                        self->dynamicPort(portIndex)->setValue(value);
                        ++portIndex;
                    }
                }
            }
        }
    }

    void LuaScript::step(dagbase::Node *self)
    {
        State prevState = _state;
        do
        {
            prevState = _state;
            switch (_state)
            {
                case STATE_INITIAL:
                    if (_thread && _thread->ok())
                    {
                        if (_func)
                        {
                            _func->push();
                            lua_xmove(_func->lua(), _thread->get(), 1);
                        }
                        auto type = SWIG_TypeQuery(_thread->get(), "_p_dagbase__Node");
                        if (type)
                        {
                            _thread->pushObj(self, type, 0);
                            setState(STATE_THREAD_INITED);
                        }
                        else
                        {
                            setState(STATE_THREAD_ERROR);
                        }
                    }
                    else
                    {
                        setState(STATE_THREAD_ERROR);
                    }
                    break;
                case STATE_THREAD_INITED:
                {
                    if (_thread && lua_isfunction(_thread->get(), -2))
                    {
                        int numResults = 0;
                        auto reason = _thread->resume(1, &numResults);
                        switch (reason)
                        {
                            case LUA_OK:
                            {
                                setPortValues(self, numResults);
                                setState(STATE_THREAD_FINISHED);
                                break;
                            }
                            case LUA_YIELD:
                            {
                                int nspace = _thread->integer(1);
                                int action = _thread->integer(2);
                                if (_proto)
                                {
                                    std::array<std::byte, sizeof(dagbase::Variant)*4> buffer;
                                    std::pmr::monotonic_buffer_resource pool{buffer.data(), buffer.size()};
                                    std::pmr::vector<dagbase::Variant> args{&pool};
                                    // Copy args into buffer
                                    for (int i=3; i<=_thread->top(); ++i)
                                    {
                                        dagbase::Variant arg;
                                        arg.fromLua(*_thread, i);
                                        args.emplace_back(arg);
                                    }
                                    _results.clear();
                                    _proto->request(self, action, args, &_results);
                                    for (auto result : _results)
                                    {
                                        result.toLua(*_thread);
                                    }
                                    lua_pop(_thread->get(), numResults);
                                }
                                break;
                            }
                            default:
                            {
                                setState(STATE_THREAD_ERROR);
                                break;
                            }
                        }
                    }
                    else
                    {
                        setState(STATE_THREAD_ERROR);
                    }

                    break;
                }
            }
        } while (_state!=prevState);
    }

    void LuaScript::reset()
    {
        if (_thread)
        {
            _thread->reset();
            if (_thread->top()>=1 && _thread->isFunction(-1))
                _thread->pop(1);
            setState(STATE_INITIAL);
        }
    }

    LuaScript * LuaScript::clone() const
    {
        // Create a new thread
        return new LuaScript(*this);

        // Copy the function to its stack
    }

    dagbase::Variant LuaScript::find(std::string_view path) const
    {
        dagbase::Variant retval;

        retval = dagbase::findEndpoint(path, "state", stateName(_state));
        if (retval.has_value())
            return retval;

        return {};
    }

    dagbase::OutputStream & LuaScript::writeToStream(dagbase::OutputStream &stream) const
    {
        if (_func)
        {
            _func->write(stream);
        }

        return stream;
    }

    dagbase::InputStream & LuaScript::readFromStream(dagbase::InputStream &stream, dagbase::Lua& lua)
    {
        _func = new dagbase::Function(stream, lua);
        _thread = dagbase::Coroutine::fromExistingThread(lua.get());

        return stream;
    }

    const char * LuaScript::stateName(State state)
    {
        switch (state)
        {
            ENUM_NAME(STATE_UNKNOWN)
            ENUM_NAME(STATE_INITIAL)
            ENUM_NAME(STATE_THREAD_INITED)
            ENUM_NAME(STATE_THREAD_FINISHED)
            ENUM_NAME(STATE_THREAD_ERROR)
        }

        return "<error>";
    }

    LuaScript::State LuaScript::parseState(const char *str)
    {
        TEST_ENUM(STATE_UNKNOWN, str)
        TEST_ENUM(STATE_INITIAL, str)
        TEST_ENUM(STATE_THREAD_INITED, str)
        TEST_ENUM(STATE_THREAD_FINISHED, str)
        TEST_ENUM(STATE_THREAD_ERROR, str)

        return STATE_UNKNOWN;
    }

    void LuaScript::setState(State newState)
    {
        std::cout << "LuaScript:" << this << ":Changing state from " << stateName(_state) << " to " << stateName(newState) << std::endl;
        _state = newState;
        switch (_state)
        {
            case STATE_INITIAL:
            {
                if (_thread)
                {
                    _thread->clearError();
                }
                break;
            }
            case STATE_THREAD_ERROR:
            {
                if (_thread && !_thread->ok())
                    std::cerr << "LuaScript:A Lua error occurred executing the script:" << _mainName << ':' << _thread->errorMessage() << '\n';
                else if (!_thread)
                {
                    std::cerr << "LuaScript:An error occurred creating the Coroutine from \"" << _mainName << "\"\n";
                }
                if (_thread)
                {
                    std::cerr << "Lua stack:\n";
                    dagbase::printStack(_thread->get());
                }
                break;
            }
            default:
                break;
        }
    }
}
