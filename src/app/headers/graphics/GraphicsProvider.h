#pragma once

#include "ver.h"
#include "core/Core.h"

#include <stdint.h>
#include <iostream>
#include <string>
#include <set>
#include <map>
#include <optional>
#include <limits>
#include <algorithm>
#include <vector>
#include <chrono>

#include "core/Vulkan.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "core/Math.h"

#include <stb_image.h>

typedef struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete();
} QueueFamilyIndices;

typedef struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
} SwapChainSupportDetails;

typedef struct UniformBufferObject
{
    Matrix4x4 model;
    Matrix4x4 view;
    Matrix4x4 proj;
} UniformBufferObject;

class GraphicsProvider final
{
private:
    std::string preferedDeviceName;
    std::string selectedDeviceName;
    std::vector<const char *> deviceExtensions;

    GLFWwindow *window;
    uint16_t windowWidth;
    uint16_t windowHeight;

    VkInstance vulkanInstance;
    VkPhysicalDevice physicalDevice;
    VkDevice logicalDevice;
    VkSurfaceKHR vulkanSurface;
    QueueFamilyIndices queueFamilyIndices;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    SwapChainSupportDetails swapChainSupportDetails;
    VkSwapchainKHR swapChain;
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    std::vector<VkImageView> swapChainImageViews;
    std::vector<VkFramebuffer> swapChainFrameBuffers;
    VkRenderPass renderPass;
    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorPool descriptorPool;
    VkDescriptorSet descriptorSet;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;
    VkCommandPool commandPool;
    VkCommandBuffer commandBuffer;

    VkSemaphore imageAvailableSemaphore;
    VkSemaphore renderFinishedSemaphore;
    VkFence inFlightFence;

    ShaderPtr vertexShader;
    ShaderPtr fragmentShader;

    GridPtr testoGrid;

    VkImage textureImage;
    VkDeviceMemory textureImageMemory;
    VkImageView textureImageView;

    VkBuffer uniformBuffer;
    VkDeviceMemory uniformBufferMemory;
    void *uniformBufferMapped;

    bool frameBufferResized;

    int counter;

public:
    GraphicsProvider()
    {
        this->windowWidth = 0;
        this->windowHeight = 0;

        // TODO move to config file
        this->preferedDeviceName = "NVIDIA GeForce RTX 4080";
        this->physicalDevice = VK_NULL_HANDLE;

        this->deviceExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME};

        this->frameBufferResized = false;

        this->counter = 0;
    };

    static void error_callback(int error, const char *description);
    static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
    static void framebuffer_resize_callback(GLFWwindow *window, int width, int height);

    void init();
    void cleanup();
    void waitUntilDeviceIdle();

    void createWindow();
    GLFWwindow *getWindow();
    void destroyWindow();

    void createVulkanInstance();
    VkInstance *getVulkanInstance();
    void checkInstanceExtensions();
    void destroyVulkanInstance();

    void pickPhysicalDevice();
    VkPhysicalDevice getPhysicalDevice();
    bool isDeviceSuitable(VkPhysicalDevice device);
    bool checkDevicePropertiesSupport(VkPhysicalDevice device);
    bool checkDeviceFeaturesSupport(VkPhysicalDevice device);
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

    void createLogicalDevice();
    VkDevice getLogicalDevice();
    void destroyLogicalDevice();

    VkQueue getGraphicsQueue();
    VkQueue getPresentQueue();

    void createSurface();
    void destroySurface();

    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);
    void createSwapChain();
    void destroySwapChain();

    VkImageView createImageView(VkImage image, VkFormat format);
    void createImageViews();
    void destroyImageViews();

    void createFrameBuffers();
    void destroyFrameBuffers();

    void createSwapChainNecessities();
    void updateSwapChainNecessities();
    void destroySwapChainNecessities();

    void createGraphicsPipeline();
    void destroyGraphicsPipeline();

    void createRenderPass();
    void destroyRenderPass();

    void createCommandBuffer();
    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
    VkCommandPool getCommandPool();
    void destroyCommandBuffer();

    void createSyncObjects();
    void destroySyncObjects();

    void createDescriptorSetLayout();
    void destroyDescriptorSetLayout();

    void createUniformBuffer();
    void updateUniformBuffer();
    void destroyUniformBuffer();

    void createDescriptorPool();
    void destroyDescriptorPool();

    void createDescriptorSet();
    void destroyDescriptorSet();

    void drawFrame();

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &bufferMemory);
    void fillBufferMemory(VkDeviceMemory bufferMemory, const void *bufferData, VkDeviceSize bufferSize);
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    VkCommandBuffer beginSingleCommandBuffer();
    void endSingleCommandBuffer(VkCommandBuffer commandBuffer);
    void destroyBuffer(VkBuffer buffer, VkDeviceMemory bufferMemory);

    void createTextureImage();
    void destroyTextureImage();
    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
    void createTextureImageView();
    void destroyTextureImageView();
};