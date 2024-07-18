#include "Core.h"

#include <optional>
#include <stdint.h>

bool QueueFamilyIndices::isComplete()
{
	return graphicsFamily.has_value();
}