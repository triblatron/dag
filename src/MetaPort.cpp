//
// Created by tony on 03/02/24.
//

#include "config/config.h"

#include "MetaPort.h"
#include "io/InputStream.h"
#include "io/OutputStream.h"
#include "util/DebugPrinter.h"
#include <iostream>

namespace dag
{
    dag::MetaPort::MetaPort(dagbase::InputStream &str)
    :
    type(PortType::TYPE_UNKNOWN),
    direction(PortDirection::DIR_UNKNOWN)
    {
        std::string className;
        std::string fieldName;
        str.readHeader(&className);

        str.addObj(this);
        str.readField(&fieldName);
        str.readString(&name, true);
        std::uint32_t rawType{0};
        str.readField(&fieldName);
        str.readUInt32(&rawType);
        str.readField(&fieldName);
        type = static_cast<PortType::Type>(rawType);
        std::uint32_t rawDirection{0};
        str.readUInt32(&rawDirection);
        direction = static_cast<PortDirection::Direction>(rawDirection);
        str.readFooter();
    }

    dagbase::OutputStream& MetaPort::write(dagbase::OutputStream& str) const
    {
        str.writeHeader("MetaPort");
        str.writeField("name");
        str.writeString(name, true);
        str.writeField("type");
        str.writeUInt32(type);
        str.writeField("direction");
        str.writeUInt32(direction);
        str.writeFooter();

        return str;
    }

    dagbase::InputStream &MetaPort::read(dagbase::InputStream &str)
    {
        std::string className;
        std::string fieldName;
        str.readHeader(&className);
        str.readField(&fieldName);
        str.readString(&name, true);
        std::uint32_t rawType{0};
        str.readField(&fieldName);
        str.readUInt32(&rawType);
        type = static_cast<PortType::Type>(rawType);
        std::uint32_t rawDirection{0};
        str.readField(&fieldName);
        str.readUInt32(&rawDirection);
        direction = static_cast<PortDirection::Direction>(rawDirection);
        str.readFooter();

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

    void MetaPort::debug(dagbase::DebugPrinter& printer) const
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
