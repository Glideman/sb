#include "device\Device.h"

#include <stdint.h>
#include <iostream>
#include <string>
#include <set>
#include <map>
#include <optional>
#include <limits>
#include <algorithm>

bool QueueFamilyIndices::isComplete()
{
    return graphicsFamily.has_value() && presentFamily.has_value();
}

void Device::error_callback(int error, const char *description)
{
    fprintf(stderr, "Error: %s\n", description);
}

void Device::key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void Device::init()
{
    this->createWindow();
    this->checkInstanceExtensions();
    this->createVulkanInstance();
    this->createSurface();
    this->pickPhysicalDevice();
    this->createLogicalDevice();
    this->createSwapChain();
    this->createImageViews();

    this->createGraphicsPipeline();
}

void Device::cleanup()
{
    for (auto imageView : this->swapChainImageViews)
    {
        vkDestroyImageView(this->logicalDevice, imageView, nullptr);
    }

    vkDestroySwapchainKHR(this->logicalDevice, this->swapChain, nullptr);
    vkDestroyDevice(this->logicalDevice, nullptr);
    vkDestroySurfaceKHR(this->vulkanInstance, this->vulkanSurface, nullptr);
    vkDestroyInstance(this->vulkanInstance, nullptr);
    glfwDestroyWindow(this->window);
    glfwTerminate();
}

void Device::createWindow()
{
    if (!glfwInit())
    {
        throw std::runtime_error("Cannot initialize GLFW!");
    }

    glfwSetErrorCallback(error_callback);

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    this->window = glfwCreateWindow(800, 600, APPLICATION_NAME, nullptr, nullptr);

    if (!this->window)
    {
        throw std::runtime_error("Cannot create window!");
    }

    glfwSetKeyCallback(this->window, key_callback);
}

GLFWwindow *Device::getWindow()
{
    return this->window;
}

void Device::createVulkanInstance()
{
    VkApplicationInfo appInfo{};
    appInfo.pNext = nullptr;
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = APPLICATION_NAME;
    appInfo.applicationVersion = APPLICATION_VERSION;
    appInfo.pEngineName = ENGINE_NAME;
    appInfo.engineVersion = ENGINE_VERSION;
    appInfo.apiVersion = VK_HEADER_VERSION_COMPLETE;

    VkInstanceCreateInfo createInfo{};
    createInfo.pNext = nullptr;
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions;

    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    createInfo.enabledExtensionCount = glfwExtensionCount;
    createInfo.ppEnabledExtensionNames = glfwExtensions;

    createInfo.enabledLayerCount = 0;

    std::cout << "\nAvailable glfw extensions (" << glfwExtensionCount << "):\n";

    for (int i = 0; i < glfwExtensionCount; i++)
    {
        std::cout << '\t' << glfwExtensions[i] << '\n';
    }

    VkResult result = vkCreateInstance(&createInfo, nullptr, &this->vulkanInstance);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error(std::format("Failed to create vulkan instance! Code {}", (int)result));
    }
}

VkInstance *Device::getVulkanInstance()
{
    return &this->vulkanInstance;
}

void Device::checkInstanceExtensions()
{
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

    std::cout << "\nAvailable extensions:\n";

    for (const auto &extension : extensions)
    {
        std::cout << '\t' << extension.extensionName << '\n';
    }
}

void Device::pickPhysicalDevice()
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(*this->getVulkanInstance(), &deviceCount, nullptr);

    if (deviceCount == 0)
    {
        throw std::runtime_error("Failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(*this->getVulkanInstance(), &deviceCount, devices.data());

    std::map<std::string, VkPhysicalDevice> suitableDevices;

    std::cout << "\nAvailable devices:\n";
    for (const auto &device : devices)
    {
        if (this->isDeviceSuitable(device))
        {

            VkPhysicalDeviceProperties deviceProperties;
            vkGetPhysicalDeviceProperties(device, &deviceProperties);

            std::cout << '\t' << deviceProperties.deviceName << '\n';
            suitableDevices[deviceProperties.deviceName] = device;
        }
    }

    if (!suitableDevices.empty())
    {
        if (!this->preferedDeviceName.empty() && suitableDevices.contains(this->preferedDeviceName))
        {
            this->selectedDeviceName = this->preferedDeviceName;
            this->physicalDevice = suitableDevices[this->preferedDeviceName];
            std::cout << "Prefered device chosen! " << this->selectedDeviceName << '\n';
        }
        else
        {
            this->selectedDeviceName = suitableDevices.begin()->first;
            this->physicalDevice = suitableDevices.begin()->second;
            std::cout << "First device chosen! " << this->selectedDeviceName << '\n';
        }
    }

    if (this->physicalDevice == VK_NULL_HANDLE)
    {
        throw std::runtime_error("Failed to find a suitable GPU!");
    }
}

bool Device::isDeviceSuitable(VkPhysicalDevice device)
{
    QueueFamilyIndices indices = this->findQueueFamilies(device);

    bool extensionSupported = this->checkDeviceExtensionSupport(device);

    bool swapChainAdequate = false;
    if (extensionSupported)
    {
        SwapChainSupportDetails swapChainSupport = this->querySwapChainSupport(device);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    return indices.isComplete() &&
           this->checkDevicePropertiesSupport(device) &&
           this->checkDeviceFeaturesSupport(device) &&
           extensionSupported && swapChainAdequate;
}

bool Device::checkDevicePropertiesSupport(VkPhysicalDevice device)
{
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);

    return (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU || deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU);
}

bool Device::checkDeviceFeaturesSupport(VkPhysicalDevice device)
{
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    return deviceFeatures.geometryShader;
}

bool Device::checkDeviceExtensionSupport(VkPhysicalDevice device)
{
    uint32_t extensionCount = 0;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(this->deviceExtensions.begin(), this->deviceExtensions.end());

    for (const auto &extension : availableExtensions)
    {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

QueueFamilyIndices Device::findQueueFamilies(VkPhysicalDevice device)
{
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    VkBool32 presentSupport = false;

    int i = 0;
    for (const auto &queueFamily : queueFamilies)
    {
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, this->vulkanSurface, &presentSupport);

        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices.graphicsFamily = i;
        }

        if (presentSupport)
        {
            indices.presentFamily = i;
        }

        if (indices.isComplete())
        {
            break;
        }

        i++;
    }

    this->queueFamilyndices = indices;

    return indices;
}

void Device::createLogicalDevice()
{
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {
        this->queueFamilyndices.graphicsFamily.value(),
        this->queueFamilyndices.presentFamily.value()};

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.pNext = nullptr;
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};

    VkDeviceCreateInfo createInfo{};
    createInfo.pNext = nullptr;
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledLayerCount = 0;

    VkResult result = vkCreateDevice(this->physicalDevice, &createInfo, nullptr, &this->logicalDevice);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error(std::format("Failed to create logical device! Code {}", (int)result));
    }

    vkGetDeviceQueue(this->logicalDevice, this->queueFamilyndices.graphicsFamily.value(), 0, &this->graphicsQueue);
    vkGetDeviceQueue(this->logicalDevice, this->queueFamilyndices.presentFamily.value(), 0, &this->presentQueue);
}

void Device::createSurface()
{
    VkResult result = glfwCreateWindowSurface(this->vulkanInstance, this->window, nullptr, &this->vulkanSurface);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error(std::format("Failed to create window surface! Code {}", (int)result));
    }
}

SwapChainSupportDetails Device::querySwapChainSupport(VkPhysicalDevice device)
{
    SwapChainSupportDetails details{};

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, this->vulkanSurface, &formatCount, nullptr);

    if (formatCount != 0)
    {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, this->vulkanSurface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, this->vulkanSurface, &presentModeCount, nullptr);

    if (presentModeCount != 0)
    {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, this->vulkanSurface, &presentModeCount, details.presentModes.data());
    }

    this->swapChainSupportDetails = details;

    return details;
}

VkSurfaceFormatKHR Device::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats)
{
    for (const auto &availableFormat : availableFormats)
    {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

VkPresentModeKHR Device::chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes)
{
    for (const auto &availablePresentMode : availablePresentModes)
    {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Device::chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities)
{
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    VkExtent2D actualExtent = {
        static_cast<uint32_t>(width),
        static_cast<uint32_t>(height)};

    actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

    return actualExtent;
}

void Device::createSwapChain()
{
    VkSurfaceFormatKHR surfaceFormat = this->chooseSwapSurfaceFormat(this->swapChainSupportDetails.formats);
    VkPresentModeKHR presentMode = this->chooseSwapPresentMode(this->swapChainSupportDetails.presentModes);
    VkExtent2D extent = this->chooseSwapExtent(this->swapChainSupportDetails.capabilities);

    uint32_t imageCount = this->swapChainSupportDetails.capabilities.minImageCount + 1;

    if (this->swapChainSupportDetails.capabilities.maxImageCount > 0 && imageCount > this->swapChainSupportDetails.capabilities.maxImageCount)
    {
        imageCount = this->swapChainSupportDetails.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.pNext = nullptr;
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = this->vulkanSurface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    uint32_t queueFamilyIndices[] = {this->queueFamilyndices.graphicsFamily.value(), this->queueFamilyndices.presentFamily.value()};

    if (this->queueFamilyndices.graphicsFamily != this->queueFamilyndices.presentFamily)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;     // Optional
        createInfo.pQueueFamilyIndices = nullptr; // Optional
    }

    createInfo.preTransform = this->swapChainSupportDetails.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(this->logicalDevice, &createInfo, nullptr, &this->swapChain) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create swap chain!");
    }

    vkGetSwapchainImagesKHR(this->logicalDevice, this->swapChain, &imageCount, nullptr);
    this->swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(this->logicalDevice, this->swapChain, &imageCount, this->swapChainImages.data());

    this->swapChainImageFormat = surfaceFormat.format;
    this->swapChainExtent = extent;
}

void Device::createImageViews()
{
    this->swapChainImageViews.resize(this->swapChainImages.size());

    for (size_t i = 0; i < this->swapChainImages.size(); i++)
    {
        VkImageViewCreateInfo createInfo{};
        createInfo.pNext = nullptr;
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = this->swapChainImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = this->swapChainImageFormat;

        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(this->logicalDevice, &createInfo, nullptr, &this->swapChainImageViews[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create image views!");
        }
    }
}

void Device::createGraphicsPipeline()
{
}