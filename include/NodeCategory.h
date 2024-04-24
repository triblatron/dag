#pragma once

#include "config/Export.h"

namespace nbe
{
	class NBE_API NodeCategory
	{
	public:
		enum  Category
		{
			CAT_NONE,
			CAT_SOURCE,
			CAT_SINK,
			CAT_CONDITION,
			CAT_ACTION,
			CAT_GROUP,
			CAT_UNKNOWN
		};
	public:
		static const char* toString(Category category);

		static Category parse(const char* categoryString);
	};
}
