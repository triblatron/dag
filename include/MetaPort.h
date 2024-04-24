//
// Created by tony on 03/02/24.
//

#pragma once

#include "config/Export.h"


#include "Types.h"

#include <string>

namespace nbe
{
    class DebugPrinter;
    class InputStream;
    class OutputStream;
    
    struct NBE_API MetaPort
    {
        std::string name;
        PortType::Type type;
        PortDirection::Direction direction;

        MetaPort()
        :
        name("<unnamed>"),
        type(PortType::TYPE_UNKNOWN),
        direction(nbe::PortDirection::DIR_UNKNOWN)
        {
            // Do nothing.
        }

        MetaPort(std::string name, PortType::Type type, PortDirection::Direction direction)
        :
        name(std::move(name)),
        type(type),
        direction(direction)
        {
            // Do nothing.
        }

        MetaPort(const MetaPort& other)
        {
            name = other.name;
            type = other.type;
            direction = other.direction;
        }

        explicit MetaPort(InputStream& str);

        bool operator==(const MetaPort& other) const;

        InputStream& read(InputStream& str);

        OutputStream& write(OutputStream& str) const;

        void debug(DebugPrinter& printer) const;

        std::ostream& toLua(std::ostream& str);
    };
}
