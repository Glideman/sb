#include "graphics/Mesh.h"
#include "graphics/GraphicsProvider.h"
#include "graphics/Vertex.h"

// TODO Create a memory allocator to allocate memory for multiple objects, not for one object like right now
// vkAllocateMemory has a limit maxMemoryAllocationCount that can be low.

void Mesh::create(const GraphicsProviderPtr graphicsPtr)
{
    this->graphicsPtr = graphicsPtr;

    const std::vector<Vertex> vertices = {
        {{0.0f, -0.5f, 0.0f}, {1.0f, 1.0f, 0.0f}},
        {{0.5f, 0.5f, 0.0f}, {0.0f, 1.0f, 1.0f}},
        {{-0.5f, 0.5f, 0.0f}, {1.0f, 0.0f, 1.0f}}};

    this->createVertexBuffer(vertices);
}

void Mesh::destroy()
{
    this->destroyBuffer(this->vertexBuffer, this->vertexBufferMemory);
}

VkBuffer Mesh::getVertexBuffer()
{
    return this->vertexBuffer;
}

void Mesh::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &bufferMemory)
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.pNext = nullptr;
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.size = size;
    bufferInfo.usage = usage;

    if (vkCreateBuffer(this->graphicsPtr->getLogicalDevice(), &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create vertex buffer!");
    }

    // binding

    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(this->graphicsPtr->getLogicalDevice(), buffer, &memoryRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.pNext = nullptr;
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memoryRequirements.size;
    allocInfo.memoryTypeIndex = this->graphicsPtr->findMemoryType(memoryRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(this->graphicsPtr->getLogicalDevice(), &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to allocate vertex buffer memory!");
    }

    vkBindBufferMemory(this->graphicsPtr->getLogicalDevice(), buffer, bufferMemory, 0);
}

void Mesh::createVertexBuffer(const std::vector<Vertex> &vertices)
{
    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    this->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
    this->fillBufferMemory(stagingBufferMemory, vertices.data(), bufferSize);

    this->createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, this->vertexBuffer, this->vertexBufferMemory);
    this->copyBuffer(stagingBuffer, this->vertexBuffer, bufferSize);

    this->destroyBuffer(stagingBuffer, stagingBufferMemory);
}

void Mesh::fillBufferMemory(VkDeviceMemory bufferMemory, const void *bufferData, VkDeviceSize bufferSize)
{
    void *data;
    vkMapMemory(this->graphicsPtr->getLogicalDevice(), bufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, bufferData, (size_t)bufferSize);
    vkUnmapMemory(this->graphicsPtr->getLogicalDevice(), bufferMemory);
}

void Mesh::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.pNext = nullptr;
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = this->graphicsPtr->getCommandPool();
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(this->graphicsPtr->getLogicalDevice(), &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.pNext = nullptr;
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(this->graphicsPtr->getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(this->graphicsPtr->getGraphicsQueue()); // vkWaitForFences
    vkFreeCommandBuffers(this->graphicsPtr->getLogicalDevice(), this->graphicsPtr->getCommandPool(), 1, &commandBuffer);
}

void Mesh::destroyBuffer(VkBuffer buffer, VkDeviceMemory bufferMemory)
{
    vkDestroyBuffer(this->graphicsPtr->getLogicalDevice(), buffer, nullptr);
    vkFreeMemory(this->graphicsPtr->getLogicalDevice(), bufferMemory, nullptr);
}
