#pragma once

#include "config/Export.h"

#include "Types.h"
#include "Value.h"

#include <cstdint>
#include <string>
#include <utility>

namespace nbe
{
	struct NBE_API PortDescriptor
	{
		std::int64_t id;
		std::string name;
		PortType::Type type;
		PortDirection::Direction direction;

		PortDescriptor()
			:
		id(-1),
		type(PortType::TYPE_UNKNOWN),
		direction(PortDirection::DIR_UNKNOWN)
		{
			// Do nothing.
		}

		PortDescriptor(std::int64_t id, std::string name, PortType::Type type, PortDirection::Direction direction)
			:
		id(id),
		name(std::move(name)),
		type(type),
		direction(direction)
		{
			// Do nothing.
		}

        bool operator==(const PortDescriptor& other) const
        {
            return id == other.id && name == other.name && type == other.type && direction == other.direction;
        }

		[[nodiscard]] bool valid() const
		{
			return id != -1 && !name.empty() && type != PortType::TYPE_UNKNOWN && direction != PortDirection::DIR_UNKNOWN;
		}
	};
}