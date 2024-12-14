//
// Created by tony on 03/02/24.
//

#include "config/config.h"

#include "MetaPort.h"
#include "InputStream.h"
#include "OutputStream.h"
#include "DebugPrinter.h"
#include <iostream>

namespace dag
{
    dag::MetaPort::MetaPort(dag::InputStream &str)
    :
    type(PortType::TYPE_UNKNOWN),
    direction(PortDirection::DIR_UNKNOWN)
    {
        str.addObj(this);
        str.read(&name);
        str.read(&type);
        str.read(&direction);
    }

    OutputStream& MetaPort::write(OutputStream& str) const
    {
        str.write(name);
        str.write(type);
        str.write(direction);

        return str;
    }

    InputStream &MetaPort::read(InputStream &str)
    {
        str.read(&name);
        str.read(&type);
        str.read(&direction);

        return str;
    }

    bool MetaPort::operator==(const MetaPort &other) const
    {
        if (name != other.name)
        {
            return false;
        }

        if (type != other.type)
        {
            return false;
        }

        if (direction != other.direction)
        {
            return false;
        }

        return true;
    }

    void MetaPort::debug(DebugPrinter &printer) const
    {
        printer.println("name: " + name);
        printer.println("type: " + std::to_string(type));
        printer.println("direction: " + std::to_string(direction));
    }

    std::ostream &MetaPort::toLua(std::ostream &str)
    {
        str << "name = \"" << name << "\", ";
        str << "type = \"" << PortType::toString(type) << "\", ";
        str << "direction = \"" << PortDirection::toString(direction) << "\", ";

        return str;
    }
}
