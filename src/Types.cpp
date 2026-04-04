#include "config/config.h"

#include "Types.h"
#include "util/enums.h"

#include <cstring>

namespace dag
{
	PortType::Type PortType::parseFromString(const char* str)
	{
		PortType::Type type = TYPE_UNKNOWN;

		if (std::strcmp(str, "TYPE_INTEGER")==0)
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
        "TYPE_INTEGER",
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

    std::string copyOpToString(CopyOp op)
    {
        std::string str;

        BIT_NAME(op, DEEP_COPY_NODES_BIT, str);
        BIT_NAME(op, DEEP_COPY_INPUTS_BIT, str);
        BIT_NAME(op, DEEP_COPY_OUTPUTS_BIT, str);
        BIT_NAME(op, GENERATE_UNIQUE_ID_BIT, str);
        BIT_NAME(op, DEEP_COPY_PARENT_BIT, str);

        if (!str.empty() && str.back() == ' ')
            str.pop_back();

        return str;
    }

    CopyOp parseCopyOp(std::string_view str)
    {
        CopyOp value{DEEP_COPY_NONE};

        TEST_BIT(DEEP_COPY_NODES_BIT, str, value);
        TEST_BIT(DEEP_COPY_INPUTS_BIT, str, value);
        TEST_BIT(DEEP_COPY_OUTPUTS_BIT, str, value);
        TEST_BIT(GENERATE_UNIQUE_ID_BIT, str, value);
        TEST_BIT(DEEP_COPY_PARENT_BIT, str, value);

        return value;
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
