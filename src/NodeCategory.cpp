#include "config/config.h"

#include "NodeCategory.h"
#include <cstring>

namespace nbe
{
	const char* NodeCategory::toString(Category category)
	{
		switch (category)
		{
		case CAT_NONE:
			return "None";
		case CAT_SOURCE:
			return "Source";
		case CAT_SINK:
			return "Sink";
		case CAT_CONDITION:
			return "Condition";
		case CAT_ACTION:
			return "Action";
		case CAT_GROUP:
			return "Group";
		}

		return "Unknown";
	}

	NodeCategory::Category NodeCategory::parse(const char* categoryString)
	{
		if (strcmp(categoryString, "None") == 0)
		{
			return CAT_NONE;
		}
		else if (strcmp(categoryString, "Source") == 0)
		{
			return CAT_SOURCE;
		}
		else if (strcmp(categoryString, "Sink") == 0)
		{
			return CAT_SINK;
		}
		else if (strcmp(categoryString, "Condition") == 0)
		{
			return CAT_CONDITION;
		}
		else if (strcmp(categoryString, "Action") == 0)
		{
			return CAT_ACTION;
		}
		else if (strcmp(categoryString, "Group") == 0)
		{
			return CAT_GROUP;
		}

		return CAT_UNKNOWN;
	}
}
