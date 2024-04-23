#include "config/config.h"

#include "Types.h"
#include <cstring>

namespace nbe
{
	PortType::Type PortType::parseFromString(const char* str)
	{
		PortType::Type type = TYPE_UNKNOWN;

		if (std::strcmp(str, "TYPE_INT")==0)
		{
			type = TYPE_INT;
		}
		else if (std::strcmp(str, "TYPE_DOUBLE") == 0)
		{
			type = TYPE_DOUBLE;
		}
		else if (std::strcmp(str, "TYPE_STRING") == 0)
		{
			type = TYPE_STRING;
		}
		else if (std::strcmp(str, "TYPE_BOOL") == 0)
		{
			type = TYPE_BOOL;
		}
		else if (std::strcmp(str, "TYPE_VEC3D") == 0)
		{
			type = TYPE_VEC3D;
		}
		else if (std::strcmp(str, "TYPE_OPAQUE") == 0)
		{
			type = TYPE_OPAQUE;
		}
		else if (std::strcmp(str, "TYPE_VECTOR") == 0)
		{
			type = TYPE_VECTOR;
		}

		return type;
	}

    static const char* portTypeNames[]=
    {
        "TYPE_INT",
        "TYPE_DOUBLE",
        "TYPE_STRING",
        "TYPE_BOOL",
        "TYPE_VEC3D",
        "TYPE_OPAQUE",
        "TYPE_VECTOR",
        "TYPE_UNKNOWN"
    };

    const char *PortType::toString(PortType::Type type)
    {
        return portTypeNames[type];
    }

    PortDirection::Direction PortDirection::parseFromString(const char* str)
	{
		Direction dir = DIR_UNKNOWN;

		if (std::strcmp(str, "DIR_IN") == 0)
		{
			dir = DIR_IN;
		}
		else if (std::strcmp(str, "DIR_OUT") == 0)
		{
			dir = DIR_OUT;
		}
		else if (std::strcmp(str, "DIR_INTERNAL") == 0)
		{
			dir = DIR_INTERNAL;
		}

		return dir;
	}

    static const char* directionNames[]=
    {
        "DIR_UNKNOWN",
        "DIR_IN",
        "DIR_OUT",
        "DIR_INTERNAL"
    };

    const char *PortDirection::toString(PortDirection::Direction dir)
    {
        return directionNames[dir];
    }
}
