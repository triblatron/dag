//
// Created by tony on 03/02/24.
//

#pragma once

#include "config/Export.h"


#include "Types.h"

#include <string>

namespace dagbase
{
    class InputStream;
    class OutputStream;
}

namespace dag
{
    class DebugPrinter;

    struct DAG_API MetaPort
    {
        std::string name;
        PortType::Type type;
        PortDirection::Direction direction;

        MetaPort()
        :
        name("<unnamed>"),
        type(PortType::TYPE_UNKNOWN),
        direction(dag::PortDirection::DIR_UNKNOWN)
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

        explicit MetaPort(dagbase::InputStream& str);

        bool operator==(const MetaPort& other) const;

        dagbase::InputStream& read(dagbase::InputStream& str);

        dagbase::OutputStream& write(dagbase::OutputStream& str) const;

        void debug(DebugPrinter& printer) const;

        std::ostream& toLua(std::ostream& str);
    };
}
