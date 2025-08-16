//
// Created by tony on 04/06/24.
//

#include "config/Export.h"

#include <vector>
#include <cstdint>
#include <cstdlib>

namespace dag
{
    class Command;

    class DAG_API Menu
    {
    public:
        void addCommand(Command* cmd)
        {
            if (cmd != nullptr)
            {
                _commands.emplace_back(cmd);
                _enabled.emplace_back(true);
            }
        }

        void enableCommand(size_t index)
        {
            if (index<_enabled.size())
            {
                _enabled[index] = true;
            }
        }

        void disableCommand(size_t index)
        {
            if (index<_enabled.size())
            {
                _enabled[index] = false;
            }
        }
    private:
        typedef std::vector<Command*> CommandArray;
        CommandArray _commands;
        typedef std::vector<bool> FlagArray;
        FlagArray _enabled;
    };
}
