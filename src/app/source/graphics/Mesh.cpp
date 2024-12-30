#include "graphics/Mesh.h"
#include "graphics/GraphicsProvider.h"
#include "graphics/Vertex.h"

void Mesh::create(const GraphicsProviderPtr graphicsPtr)
{
    this->graphicsPtr = graphicsPtr;

    const std::vector<Vertex> vertices = {
        {{0.0f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
        {{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}}};

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.pNext = nullptr;
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.size = sizeof(Vertex) * vertices.size();

    if (vkCreateBuffer(this->graphicsPtr->getLogicalDevice(), &bufferInfo, nullptr, &this->vertexBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create vertex buffer!");
    }

    // binding

    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(this->graphicsPtr->getLogicalDevice(), this->vertexBuffer, &memoryRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.pNext = nullptr;
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memoryRequirements.size;
    allocInfo.memoryTypeIndex = this->graphicsPtr->findMemoryType(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    if (vkAllocateMemory(this->graphicsPtr->getLogicalDevice(), &allocInfo, nullptr, &this->vertexBufferMemory) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to allocate vertex buffer memory!");
    }

    vkBindBufferMemory(this->graphicsPtr->getLogicalDevice(), this->vertexBuffer, this->vertexBufferMemory, 0);

    // filling

    void *data;
    vkMapMemory(this->graphicsPtr->getLogicalDevice(), this->vertexBufferMemory, 0, bufferInfo.size, 0, &data);
    memcpy(data, vertices.data(), (size_t)bufferInfo.size);
    vkUnmapMemory(this->graphicsPtr->getLogicalDevice(), this->vertexBufferMemory);
}

void Mesh::destroy()
{
    vkFreeMemory(this->graphicsPtr->getLogicalDevice(), this->vertexBufferMemory, nullptr);
    vkDestroyBuffer(this->graphicsPtr->getLogicalDevice(), this->vertexBuffer, nullptr);
}

VkBuffer Mesh::getVertexBuffer()
{
    return this->vertexBuffer;
}