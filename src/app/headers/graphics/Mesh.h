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
    size_t vertexBufferOffset;
    size_t indexBufferOffset;

    void createVertexBuffer(const std::vector<Vertex> &vertices);
    void createIndexBuffer(const std::vector<uint16_t> &indices);

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
        this->vertexBufferOffset = 0;
        this->indexBufferOffset = 0;
    };

    void create(const GraphicsProviderPtr graphicsPtr, const std::vector<Vertex> &vertices, const std::vector<uint16_t> &indices);
    void destroy();
    VkBuffer getVertexBuffer();
    VkBuffer getIndexBuffer();
    size_t getVertexBufferSize();
    size_t getIndexBufferSize();
    size_t getVertexBufferOffset();
    size_t getIndexBufferOffset();
};