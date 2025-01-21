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

#include "core/Vulkan.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

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

class GraphicsProvider
{
private:
    std::string preferedDeviceName;
    std::string selectedDeviceName;
    std::vector<const char *> deviceExtensions;

    GLFWwindow *window;
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
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;
    VkCommandPool commandPool;
    VkCommandBuffer commandBuffer;

    VkSemaphore imageAvailableSemaphore;
    VkSemaphore renderFinishedSemaphore;
    VkFence inFlightFence;

    ShaderPtr vertexShader;
    ShaderPtr fragmentShader;

    MeshPtr testoMesh;

    bool frameBufferResized;

public:
    GraphicsProvider()
    {
        // TODO move to config file
        this->preferedDeviceName = "NVIDIA GeForce RTX 4080";
        this->physicalDevice = VK_NULL_HANDLE;

        this->deviceExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME};

        this->frameBufferResized = false;
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

    void drawFrame();

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
};