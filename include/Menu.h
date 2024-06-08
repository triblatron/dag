//
// Created by tony on 04/06/24.
//

#include "config/Export.h"

#include <vector>
#include <cstdint>

namespace nbe
{
    class Command;

    class NBE_API Menu
    {
    public:
        void addCommand(Command* cmd)
        {
            if (cmd != nullptr)
            {
                _commands.push_back(cmd);
                _enabled.push_back(true);
            }
        }

        void enableCommand(size_t index)
        {
            _enabled[index] = true;
        }

        void disableCommand(size_t index)
        {
            _enabled[index] = false;
        }
    private:
        typedef std::vector<Command*> CommandArray;
        CommandArray _commands;
        typedef std::vector<bool> FlagArray;
        FlagArray _enabled;
    };
}
