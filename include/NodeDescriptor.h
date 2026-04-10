#pragma once

#include "config/Export.h"

#include "NodeCategory.h"

#include <cstdint>
#include <string>
#include <vector>

namespace dag
{
	struct DAG_API NodeDescriptor
	{
		std::int64_t id;
		std::string name;
		NodeCategory::Category category;
        using MetaPortArray = std::vector<MetaPort>;
	    MetaPortArray ports;

		NodeDescriptor()
			:
		id(-1),
		category(NodeCategory::CAT_UNKNOWN)
		{
			// Do nothing.
		}
	};
}