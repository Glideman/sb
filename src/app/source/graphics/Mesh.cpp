#include "graphics/Mesh.h"
#include "graphics/GraphicsProvider.h"
#include "graphics/Vertex.h"

// TODO Create a memory allocator to allocate memory for multiple objects, not for one object like right now
// vkAllocateMemory has a limit maxMemoryAllocationCount that can be low.
// go beyond and allocate a single buffer for vetrices, indices and etc, use offsets to allocate buffers on render.

void Mesh::create(const GraphicsProviderPtr graphicsPtr, const std::vector<Vertex> &vertices, const std::vector<uint16_t> &indices)
{
    this->graphicsPtr = graphicsPtr;

    this->vertexBufferSize = vertices.size();
    this->indexBufferSize = indices.size();

    this->createVertexBuffer(vertices);
    this->createIndexBuffer(indices);
}

void Mesh::destroy()
{
    this->graphicsPtr->destroyBuffer(this->indexBuffer, this->indexBufferMemory);
    this->graphicsPtr->destroyBuffer(this->vertexBuffer, this->vertexBufferMemory);
}

VkBuffer Mesh::getVertexBuffer()
{
    return this->vertexBuffer;
}

VkBuffer Mesh::getIndexBuffer()
{
    return this->indexBuffer;
}

size_t Mesh::getVertexBufferSize()
{
    return this->vertexBufferSize;
}

size_t Mesh::getIndexBufferSize()
{
    return this->indexBufferSize;
}

size_t Mesh::getVertexBufferOffset()
{
    return this->vertexBufferOffset;
}

size_t Mesh::getIndexBufferOffset()
{
    return this->indexBufferOffset;
}

void Mesh::createVertexBuffer(const std::vector<Vertex> &vertices)
{
    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

    VkBuffer stagingBuffer = nullptr;
    VkDeviceMemory stagingBufferMemory = nullptr;

    this->graphicsPtr->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
    this->graphicsPtr->fillBufferMemory(stagingBufferMemory, vertices.data(), bufferSize);

    this->graphicsPtr->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, this->vertexBuffer, this->vertexBufferMemory);
    this->graphicsPtr->copyBuffer(stagingBuffer, this->vertexBuffer, bufferSize);

    this->graphicsPtr->destroyBuffer(stagingBuffer, stagingBufferMemory);
}

void Mesh::createIndexBuffer(const std::vector<uint16_t> &indices)
{
    VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

    VkBuffer stagingBuffer = nullptr;
    VkDeviceMemory stagingBufferMemory = nullptr;

    this->graphicsPtr->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
    this->graphicsPtr->fillBufferMemory(stagingBufferMemory, indices.data(), bufferSize);

    this->graphicsPtr->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, this->indexBuffer, this->indexBufferMemory);
    this->graphicsPtr->copyBuffer(stagingBuffer, this->indexBuffer, bufferSize);

    this->graphicsPtr->destroyBuffer(stagingBuffer, stagingBufferMemory);
}
