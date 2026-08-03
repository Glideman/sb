#pragma once

#include "core/Core.h"
#include "core/Vulkan.h"
#include "core/Math.h"

struct Vertex
{
    Vector3 pos;
    Vector3 color;
    Vector2 tex;

    static VkVertexInputBindingDescription getBindingDescription();
    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions();
};