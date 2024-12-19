#pragma once

#include "config/Export.h"

#include "PortDescriptor.h"
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
		typedef std::vector<PortDescriptor> PortDescriptorArray;
		PortDescriptorArray ports;

		NodeDescriptor()
			:
		id(-1),
		category(NodeCategory::CAT_UNKNOWN)
		{
			// Do nothing.
		}
	};
}