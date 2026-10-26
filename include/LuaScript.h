//
// Created by Tony Horrobin on 11/10/2026.
//

#pragma once

#include "config/Export.h"

#include "core/Script.h"

#include <string>
#include <cstdint>
#include <vector>

namespace dagbase
{
    class ConfigurationElement;
    class Coroutine;
    class Function;
    class Lua;
    class ScriptProtocol;
}

namespace dag
{
    class NodeEditorLive;

    class DAG_API LuaScript : public dagbase::Script
    {
    public:
        enum State : std::uint32_t
        {
            STATE_UNKNOWN,
            STATE_INITIAL,
            STATE_THREAD_INITED,
            STATE_THREAD_FINISHED,
            STATE_THREAD_ERROR
        };
    public:
        LuaScript(dagbase::Lua* lua, dagbase::ScriptProtocol* proto);

        LuaScript(dagbase::Lua* lua, dagbase::Function* func, dagbase::ScriptProtocol* proto);

        LuaScript(const LuaScript& other);

        ~LuaScript() override;

        void configure(dagbase::ConfigurationElement& config) override;

        void setMainFunction(const std::string& name) override
        {
            _mainName = name;
        }

        void setPortValues(dagbase::Node* self, int numResults);

        dagbase::Coroutine* thread()
        {
            return _thread;
        }

        void step(dagbase::Node* self) override;

        bool isFinished() const override
        {
            return _state == STATE_THREAD_FINISHED || _state == STATE_THREAD_ERROR;
        }

        void reset() override;

        LuaScript* clone() const override;

        dagbase::Variant find(std::string_view path) const override;

        static const char* stateName(State state);

        static State parseState(const char* str);
    private:
        dagbase::Lua* _lua{nullptr};
        dagbase::Function* _func{nullptr};
        dagbase::Coroutine* _thread{nullptr};
        dagbase::ScriptProtocol* _proto{nullptr};
        State _state{STATE_INITIAL};
        std::string _mainName;
        std::vector<dagbase::Variant> _results;
        void setState(State newState);
    };
}