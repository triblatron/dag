#include "config/config.h"

#include "Types.h"
#include "util/enums.h"

#include <cstring>

namespace dag
{
	PortType::Type PortType::parseFromString(const char* str)
	{
	    TEST_ENUM(TYPE_UINT8, str);
	    TEST_ENUM(TYPE_INT8, str);
	    TEST_ENUM(TYPE_UINT16, str);
	    TEST_ENUM(TYPE_INT16, str);
	    TEST_ENUM(TYPE_UINT32, str);
	    TEST_ENUM(TYPE_INT32, str);
	    TEST_ENUM(TYPE_UINT64, str);
	    TEST_ENUM(TYPE_INT64, str);
	    TEST_ENUM(TYPE_FLOAT, str);
	    TEST_ENUM(TYPE_DOUBLE, str);
	    TEST_ENUM(TYPE_STRING, str);
	    TEST_ENUM(TYPE_BOOL, str);
	    TEST_ENUM(TYPE_VEC3D, str);
	    TEST_ENUM(TYPE_OPAQUE, str);
	    TEST_ENUM(TYPE_VECTOR, str);
	    TEST_ENUM(TYPE_UNKNOWN, str);

		return TYPE_UNKNOWN;
	}

    const char *PortType::toString(Type type)
    {
        switch (type)
        {
            ENUM_NAME(TYPE_UINT8)
            ENUM_NAME(TYPE_INT8)
            ENUM_NAME(TYPE_UINT16)
            ENUM_NAME(TYPE_INT16)
            ENUM_NAME(TYPE_UINT32)
            ENUM_NAME(TYPE_INT32)
            ENUM_NAME(TYPE_UINT64)
            ENUM_NAME(TYPE_INT64)
            ENUM_NAME(TYPE_FLOAT)
            ENUM_NAME(TYPE_DOUBLE)
            ENUM_NAME(TYPE_STRING)
            ENUM_NAME(TYPE_BOOL)
            ENUM_NAME(TYPE_VEC3D)
            ENUM_NAME(TYPE_OPAQUE)
            ENUM_NAME(TYPE_VECTOR)
            ENUM_NAME(TYPE_UNKNOWN)
        }

        return "<error>";
    }

    std::string copyOpToString(CopyOp op)
    {
        std::string str;

        if (op == DEEP_COPY_NONE)
            return "DEEP_COPY_NONE";

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
