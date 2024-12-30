#pragma once

#include "core/Core.h"
#include "core/Vulkan.h"

class Mesh
{
private:
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    GraphicsProviderPtr graphicsPtr;

public:
    Mesh()
    {
        this->vertexBuffer = nullptr;
        this->vertexBufferMemory = nullptr;
        this->graphicsPtr = nullptr;
    };

    void create(const GraphicsProviderPtr graphicsPtr);
    void destroy();
    VkBuffer getVertexBuffer();
};