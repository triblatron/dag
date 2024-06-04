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
        enum : std::uint32_t
        {
            MAX_COMMANDS = 32
        };
    public:
        void addCommand(Command* cmd)
        {
            if (cmd != nullptr)
            {
                _commands.push_back(cmd);
            }
        }

        void enableCommands(std::uint64_t mask)
        {
            _enabled |= mask;
        }

        void disableCommands(std::uint64_t mask)
        {
            _enabled &= ~mask;
        }
    private:
        typedef std::vector<Command*> CommandArray;
        CommandArray _commands;
        std::uint64_t _enabled;
    };
}
