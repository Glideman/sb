#pragma once

#include "ver.h"

#include <optional>
#include <vector>
#include <stdint.h>

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.hpp>

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

class Device
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
    QueueFamilyIndices queueFamilyndices;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    SwapChainSupportDetails swapChainSupportDetails;
    VkSwapchainKHR swapChain;
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;

public:
    Device()
    {
        this->preferedDeviceName = "NVIDIA GeForce RTX 4080";
        this->physicalDevice = VK_NULL_HANDLE;

        this->deviceExtensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    };

    static void error_callback(int error, const char *description);
    static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);

    void init();
    void cleanup();

    void createWindow();
    GLFWwindow *getWindow();
    void createVulkanInstance();
    VkInstance *getVulkanInstance();
    void checkInstanceExtensions();
    void pickPhysicalDevice();
    bool isDeviceSuitable(VkPhysicalDevice device);
    bool checkDevicePropertiesSupport(VkPhysicalDevice device);
    bool checkDeviceFeaturesSupport(VkPhysicalDevice device);
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
    void createLogicalDevice();
    void createSurface();
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);
    void createSwapChain();
};
