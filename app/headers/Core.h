#pragma once

#include <optional>
#include <stdint.h>

typedef struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily;

    bool isComplete();
} QueueFamilyIndices;