#pragma once

#include "core/Core.h"
#include "core/Vulkan.h"

class Mesh
{
private:
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;

    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;

    GraphicsProviderPtr graphicsPtr;

    size_t vertexBufferSize;
    size_t indexBufferSize;

    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &bufferMemory);
    void createVertexBuffer(const std::vector<Vertex> &vertices);
    void createIndexBuffer(const std::vector<uint16_t> &indices);
    void fillBufferMemory(VkDeviceMemory bufferMemory, const void *bufferData, VkDeviceSize bufferSize);
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    void destroyBuffer(VkBuffer buffer, VkDeviceMemory bufferMemory);

public:
    Mesh()
    {
        this->vertexBuffer = nullptr;
        this->vertexBufferMemory = nullptr;
        this->indexBuffer = nullptr;
        this->indexBufferMemory = nullptr;
        this->graphicsPtr = nullptr;
        this->vertexBufferSize = 0;
        this->indexBufferSize = 0;
    };

    void create(const GraphicsProviderPtr graphicsPtr);
    void destroy();
    VkBuffer getVertexBuffer();
    VkBuffer getIndexBuffer();
    size_t getVertexBufferSize();
    size_t getIndexBufferSize();
};