#define STB_IMAGE_IMPLEMENTATION

#include "graphics/GraphicsProvider.h"
#include "core/Loader.h"
#include "core/Logger.h"
#include "core/Math.h"
#include "graphics/Shader.h"
#include "graphics/Vertex.h"
#include "graphics/Mesh.h"
#include "primitives/Grid.h"

bool QueueFamilyIndices::isComplete()
{
    return graphicsFamily.has_value() && presentFamily.has_value();
}

void GraphicsProvider::error_callback(int error, const char *description)
{
    fprintf(stderr, "Error: %s\n", description);
}

void GraphicsProvider::key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void GraphicsProvider::framebuffer_resize_callback(GLFWwindow *window, int width, int height)
{
    auto provider = reinterpret_cast<GraphicsProvider *>(glfwGetWindowUserPointer(window));
    provider->frameBufferResized = true;
}

void GraphicsProvider::init()
{
    this->createWindow();
    this->checkInstanceExtensions();
    this->createVulkanInstance();
    this->createSurface();
    this->pickPhysicalDevice();
    this->createLogicalDevice();
    this->createSwapChainNecessities();
    this->createRenderPass();
    this->createDescriptorSetLayout();
    this->createGraphicsPipeline();
    this->createCommandBuffer();
    this->createSyncObjects();

    this->testoGrid = new Grid();
    this->testoGrid->create(this);

    this->createTextureImage();

    this->createUniformBuffer();
    this->createDescriptorPool();
    this->createDescriptorSet();
}

void GraphicsProvider::cleanup()
{
    this->waitUntilDeviceIdle();

    this->destroyDescriptorSet();
    this->destroyDescriptorPool();
    this->destroyUniformBuffer();

    this->destroyTextureImage();

    // this->testoCube->destroy();
    // delete this->testoCube;

    this->testoGrid->destroy();
    delete this->testoGrid;

    this->destroySyncObjects();
    this->destroyCommandBuffer();
    this->destroyGraphicsPipeline();
    this->destroyDescriptorSetLayout();
    this->destroyRenderPass();
    this->destroySwapChainNecessities();
    this->destroyLogicalDevice();
    this->destroySurface();
    this->destroyVulkanInstance();
    this->destroyWindow();
}

void GraphicsProvider::waitUntilDeviceIdle()
{
    vkDeviceWaitIdle(this->logicalDevice);
}

void GraphicsProvider::createWindow()
{
    if (!glfwInit())
    {
        throw std::runtime_error("Cannot initialize GLFW!");
    }

    glfwSetErrorCallback(error_callback);

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    // TODO move to config file
    this->windowWidth = 800;
    this->windowHeight = 600;

    this->window = glfwCreateWindow(this->windowWidth, this->windowHeight, APPLICATION_NAME, nullptr, nullptr);

    if (!this->window)
    {
        throw std::runtime_error("Cannot create window!");
    }

    glfwSetWindowUserPointer(this->window, this);

    glfwSetFramebufferSizeCallback(this->window, framebuffer_resize_callback);
    glfwSetKeyCallback(this->window, key_callback);
}

GLFWwindow *GraphicsProvider::getWindow()
{
    return this->window;
}

void GraphicsProvider::destroyWindow()
{
    glfwDestroyWindow(this->window);
    glfwTerminate();
}

void GraphicsProvider::createVulkanInstance()
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

VkInstance *GraphicsProvider::getVulkanInstance()
{
    return &this->vulkanInstance;
}

void GraphicsProvider::checkInstanceExtensions()
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

void GraphicsProvider::destroyVulkanInstance()
{
    vkDestroyInstance(this->vulkanInstance, nullptr);
}

void GraphicsProvider::pickPhysicalDevice()
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

VkPhysicalDevice GraphicsProvider::getPhysicalDevice()
{
    return this->physicalDevice;
}

bool GraphicsProvider::isDeviceSuitable(VkPhysicalDevice device)
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

bool GraphicsProvider::checkDevicePropertiesSupport(VkPhysicalDevice device)
{
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);

    return (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU || deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU);
}

bool GraphicsProvider::checkDeviceFeaturesSupport(VkPhysicalDevice device)
{
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    return deviceFeatures.geometryShader;
}

bool GraphicsProvider::checkDeviceExtensionSupport(VkPhysicalDevice device)
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

QueueFamilyIndices GraphicsProvider::findQueueFamilies(VkPhysicalDevice device)
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

    this->queueFamilyIndices = indices;

    return indices;
}

void GraphicsProvider::createLogicalDevice()
{
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {
        this->queueFamilyIndices.graphicsFamily.value(),
        this->queueFamilyIndices.presentFamily.value()};

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

    vkGetDeviceQueue(this->logicalDevice, this->queueFamilyIndices.graphicsFamily.value(), 0, &this->graphicsQueue);
    vkGetDeviceQueue(this->logicalDevice, this->queueFamilyIndices.presentFamily.value(), 0, &this->presentQueue);
}

VkDevice GraphicsProvider::getLogicalDevice()
{
    return this->logicalDevice;
}

void GraphicsProvider::destroyLogicalDevice()
{
    vkDestroyDevice(this->logicalDevice, nullptr);
}

VkQueue GraphicsProvider::getGraphicsQueue()
{
    return this->graphicsQueue;
}
VkQueue GraphicsProvider::getPresentQueue()
{
    return this->presentQueue;
}

void GraphicsProvider::createSurface()
{
    VkResult result = glfwCreateWindowSurface(this->vulkanInstance, this->window, nullptr, &this->vulkanSurface);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error(std::format("Failed to create window surface! Code {}", (int)result));
    }
}

void GraphicsProvider::destroySurface()
{
    vkDestroySurfaceKHR(this->vulkanInstance, this->vulkanSurface, nullptr);
}

SwapChainSupportDetails GraphicsProvider::querySwapChainSupport(VkPhysicalDevice device)
{
    SwapChainSupportDetails details{};

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, this->vulkanSurface, &details.capabilities);

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

VkSurfaceFormatKHR GraphicsProvider::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats)
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

VkPresentModeKHR GraphicsProvider::chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes)
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

VkExtent2D GraphicsProvider::chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities)
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

void GraphicsProvider::createSwapChain()
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

    uint32_t queueFamilyIndices[] = {this->queueFamilyIndices.graphicsFamily.value(), this->queueFamilyIndices.presentFamily.value()};

    if (this->queueFamilyIndices.graphicsFamily != this->queueFamilyIndices.presentFamily)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
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

void GraphicsProvider::destroySwapChain()
{
    vkDestroySwapchainKHR(this->logicalDevice, this->swapChain, nullptr);
}

void GraphicsProvider::createImageViews()
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
            throw std::runtime_error("Failed to create image views!");
        }
    }
}

void GraphicsProvider::destroyImageViews()
{
    for (int i = this->swapChainImageViews.size() - 1; i >= 0; i--)
    {
        vkDestroyImageView(this->logicalDevice, this->swapChainImageViews[i], nullptr);
    }
}

void GraphicsProvider::createFrameBuffers()
{
    this->swapChainFrameBuffers.resize(this->swapChainImageViews.size());

    for (size_t i = 0; i < this->swapChainImageViews.size(); i++)
    {
        VkImageView attachments[] = {
            this->swapChainImageViews[i]};

        VkFramebufferCreateInfo frameBufferInfo{};
        frameBufferInfo.pNext = nullptr;
        frameBufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        frameBufferInfo.renderPass = this->renderPass;
        frameBufferInfo.attachmentCount = 1;
        frameBufferInfo.pAttachments = attachments;
        frameBufferInfo.width = this->swapChainExtent.width;
        frameBufferInfo.height = this->swapChainExtent.height;
        frameBufferInfo.layers = 1;

        if (vkCreateFramebuffer(this->logicalDevice, &frameBufferInfo, nullptr, &this->swapChainFrameBuffers[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create frame buffer!");
        }
    }
}

void GraphicsProvider::destroyFrameBuffers()
{
    for (int i = this->swapChainFrameBuffers.size() - 1; i >= 0; i--)
    {
        vkDestroyFramebuffer(this->logicalDevice, this->swapChainFrameBuffers[i], nullptr);
    }
}

void GraphicsProvider::createSwapChainNecessities()
{
    this->createSwapChain();
    this->createImageViews();
    this->createFrameBuffers();
}

void GraphicsProvider::updateSwapChainNecessities()
{
    int width = 0, height = 0;
    glfwGetFramebufferSize(window, &width, &height);

    if (width == 0 || height == 0)
    {
        return;
    }

    this->waitUntilDeviceIdle();
    this->destroySwapChainNecessities();
    this->createSwapChainNecessities();
}

void GraphicsProvider::destroySwapChainNecessities()
{
    this->destroyFrameBuffers();
    this->destroyImageViews();
    this->destroySwapChain();
}

void GraphicsProvider::createGraphicsPipeline()
{
    this->vertexShader = new Shader();
    this->vertexShader->load(this, "shaders\\basicTriangle.vert.spv");

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.pNext = nullptr;
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = this->vertexShader->getShaderModule();
    vertShaderStageInfo.pName = "main";

    this->fragmentShader = new Shader();
    this->fragmentShader->load(this, "shaders\\basicTriangle.frag.spv");

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.pNext = nullptr;
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = this->fragmentShader->getShaderModule();
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    // Some pipeline stuff

    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescriptions();

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.pNext = nullptr;
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.pVertexBindingDescriptions = nullptr;
    vertexInputInfo.vertexAttributeDescriptionCount = 0;
    vertexInputInfo.pVertexAttributeDescriptions = nullptr;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.pNext = nullptr;
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    // Some viewport stuff

    //    VkViewport viewport{};
    //    viewport.x = 0.0f;
    //    viewport.y = 0.0f;
    //    viewport.width = (float)this->swapChainExtent.width;
    //    viewport.height = (float)this->swapChainExtent.height;
    //    viewport.minDepth = 0.0f;
    //    viewport.maxDepth = 1.0f;
    //
    //    VkRect2D scissor{};
    //    scissor.offset = {0, 0};
    //    scissor.extent = this->swapChainExtent;

    // Some other pipeline stuff

    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR};

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.pNext = nullptr;
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.pNext = nullptr;
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    // Some rasterizer stuff

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.pNext = nullptr;
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL; // VK_POLYGON_MODE_FILL | VK_POLYGON_MODE_LINE | VK_POLYGON_MODE_POINT
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_NONE; // VK_CULL_MODE_NONE VK_CULL_MODE_FRONT_BIT VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f;
    rasterizer.depthBiasClamp = 0.0f;
    rasterizer.depthBiasSlopeFactor = 0.0f;

    // Some multisampling stuff

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.pNext = nullptr;
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f;
    multisampling.pSampleMask = nullptr;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable = VK_FALSE;

    // Some depth stencil stuff

    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.pNext = nullptr;
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;

    // Some color blending stuff

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.pNext = nullptr;
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    // Some pipeline layout stuff

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.pNext = nullptr;
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    //    pipelineLayoutInfo.setLayoutCount = 0;
    //    pipelineLayoutInfo.pSetLayouts = nullptr;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &this->descriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;

    if (vkCreatePipelineLayout(this->logicalDevice, &pipelineLayoutInfo, nullptr, &this->pipelineLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create pipeline layout!");
    }

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.pNext = nullptr;
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = nullptr; // &depthStencil
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = this->pipelineLayout;
    pipelineInfo.renderPass = this->renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;

    if (vkCreateGraphicsPipelines(this->logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &this->graphicsPipeline) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create graphics pipeline!");
    }
}

void GraphicsProvider::destroyGraphicsPipeline()
{
    vkDestroyPipeline(this->logicalDevice, this->graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(this->logicalDevice, this->pipelineLayout, nullptr);

    this->fragmentShader->destroy();
    this->vertexShader->destroy();

    delete this->fragmentShader;
    delete this->vertexShader;
}

void GraphicsProvider::createRenderPass()
{
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = swapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.pNext = nullptr;
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(this->logicalDevice, &renderPassInfo, nullptr, &this->renderPass) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create render pass!");
    }
}

void GraphicsProvider::destroyRenderPass()
{
    vkDestroyRenderPass(this->logicalDevice, this->renderPass, nullptr);
}

void GraphicsProvider::createCommandBuffer()
{
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(this->physicalDevice);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.pNext = nullptr;
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    if (vkCreateCommandPool(this->logicalDevice, &poolInfo, nullptr, &this->commandPool) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create command pool!");
    }

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.pNext = nullptr;
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = this->commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(this->logicalDevice, &allocInfo, &this->commandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to allocate command buffers!");
    }
}

void GraphicsProvider::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.pNext = nullptr;
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;
    beginInfo.pInheritanceInfo = nullptr;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to begin recording command buffer!");
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.pNext = nullptr;
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = this->renderPass;
    renderPassInfo.framebuffer = this->swapChainFrameBuffers[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = this->swapChainExtent;

    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->graphicsPipeline);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(this->swapChainExtent.width);
    viewport.height = static_cast<float>(this->swapChainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = this->swapChainExtent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    MeshPtr meshPointer = this->testoGrid->getMesh();

    VkBuffer vertexBuffers[] = {meshPointer->getVertexBuffer()};
    VkDeviceSize offsets[] = {meshPointer->getVertexBufferOffset()};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

    vkCmdBindIndexBuffer(commandBuffer, meshPointer->getIndexBuffer(), meshPointer->getIndexBufferOffset(), VK_INDEX_TYPE_UINT16);

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->pipelineLayout, 0, 1, &this->descriptorSet, 0, nullptr);

    // vkCmdDraw(commandBuffer, 3, 1, 0, 0);
    vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(meshPointer->getIndexBufferSize()), 1, 0, 0, 0);

    //    MeshPtr meshPointer2 = this->testoCube->getMesh();
    //
    //    VkBuffer vertexBuffers[] = {meshPointer2->getVertexBuffer()};
    //    VkDeviceSize offsets[] = {meshPointer2->getVertexBufferOffset()};
    //    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    //
    //    vkCmdBindIndexBuffer(commandBuffer, meshPointer2->getIndexBuffer(), meshPointer2->getIndexBufferOffset(), VK_INDEX_TYPE_UINT16);
    //
    //    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->pipelineLayout, 0, 1, &this->descriptorSet, 0, nullptr);
    //
    //    // vkCmdDraw(commandBuffer, 3, 1, 0, 0);
    //    vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(meshPointer2->getIndexBufferSize()), 1, 0, 0, 0);

    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to record command buffer!");
    }
}

VkCommandPool GraphicsProvider::getCommandPool()
{
    return this->commandPool;
}

void GraphicsProvider::destroyCommandBuffer()
{
    vkFreeCommandBuffers(this->logicalDevice, this->commandPool, 1, &this->commandBuffer);
    vkDestroyCommandPool(this->logicalDevice, this->commandPool, nullptr);
}

// TODO make a semaphore and fence arrays for easier creation and cleanup
void GraphicsProvider::createSyncObjects()
{
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.pNext = nullptr;
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphoreInfo.flags = 0;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.pNext = nullptr;
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    if (vkCreateSemaphore(this->logicalDevice, &semaphoreInfo, nullptr, &this->imageAvailableSemaphore) != VK_SUCCESS ||
        vkCreateSemaphore(this->logicalDevice, &semaphoreInfo, nullptr, &this->renderFinishedSemaphore) != VK_SUCCESS ||
        vkCreateFence(this->logicalDevice, &fenceInfo, nullptr, &this->inFlightFence) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create sync objects!");
    }
}

void GraphicsProvider::destroySyncObjects()
{
    vkDestroyFence(this->logicalDevice, this->inFlightFence, nullptr);
    vkDestroySemaphore(this->logicalDevice, this->renderFinishedSemaphore, nullptr);
    vkDestroySemaphore(this->logicalDevice, this->imageAvailableSemaphore, nullptr);
}

void GraphicsProvider::createDescriptorSetLayout()
{
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboLayoutBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.pNext = nullptr;
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &uboLayoutBinding;

    if (vkCreateDescriptorSetLayout(this->logicalDevice, &layoutInfo, nullptr, &this->descriptorSetLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create descriptor set layout!");
    }
}

void GraphicsProvider::destroyDescriptorSetLayout()
{
    vkDestroyDescriptorSetLayout(this->logicalDevice, this->descriptorSetLayout, nullptr);
}

void GraphicsProvider::createUniformBuffer()
{
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    this->createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, this->uniformBuffer, this->uniformBufferMemory);
    vkMapMemory(this->logicalDevice, this->uniformBufferMemory, 0, bufferSize, 0, &this->uniformBufferMapped);
}

void GraphicsProvider::updateUniformBuffer()
{
    //    static auto startTime = std::chrono::high_resolution_clock::now();
    //
    //    auto currentTime = std::chrono::high_resolution_clock::now();
    //    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    UniformBufferObject ubo{};

    Vector3 camerapos = Vector3(2.f + (float)this->counter * 0.01f, 2.f + (float)this->counter * 0.01f, 2.f + (float)this->counter * 0.01f);

    // Vulkan defaults to a left-handed coordinate system with +Y down, +Z out of the screen (behind the default view) and +X to the right.

    ubo.view.identity();
    ubo.proj.identity();
    ubo.view.lookAt(Vector3(2.f, 2.f, -2.f), Vector3(0.f, 0.f, 0.f), Vector3(0.f, 0.f, 1.f));
    ubo.proj.perspective(3.14f / 3.f, (float)this->swapChainExtent.width / (float)this->swapChainExtent.height, 0.01f, 100.f);
    // ubo.proj.ortho((float)this->swapChainExtent.width/100.f, (float)this->swapChainExtent.height/100.f, 0.1f, 10.f);

    float rotation = 0.75f * (float)this->counter;

    // LOG("counter is %i, rotation is %f\n", this->counter, rotation);

    ubo.model.identity();
    ubo.model.rotate(0.f, 0.f, rotation);

    memcpy(this->uniformBufferMapped, &ubo, sizeof(UniformBufferObject));
}

void GraphicsProvider::destroyUniformBuffer()
{
    vkUnmapMemory(this->logicalDevice, this->uniformBufferMemory);
    this->destroyBuffer(this->uniformBuffer, this->uniformBufferMemory);
}

void GraphicsProvider::createDescriptorPool()
{
    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = 1;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.pNext = nullptr;
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = 1;

    if (vkCreateDescriptorPool(this->logicalDevice, &poolInfo, nullptr, &this->descriptorPool) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create descriptor pool!");
    }
}

void GraphicsProvider::destroyDescriptorPool()
{
    vkDestroyDescriptorPool(this->logicalDevice, descriptorPool, nullptr);
}

void GraphicsProvider::createDescriptorSet()
{
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.pNext = nullptr;
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = this->descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &this->descriptorSetLayout;

    if (vkAllocateDescriptorSets(this->logicalDevice, &allocInfo, &this->descriptorSet) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to allocate descriptor sets!");
    }

    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = this->uniformBuffer;
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(UniformBufferObject);

    VkWriteDescriptorSet descriptorWrite{};
    descriptorWrite.pNext = nullptr;
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = this->descriptorSet;
    descriptorWrite.dstBinding = 0;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pBufferInfo = &bufferInfo;
    descriptorWrite.pImageInfo = nullptr;
    descriptorWrite.pTexelBufferView = nullptr;

    vkUpdateDescriptorSets(this->logicalDevice, 1, &descriptorWrite, 0, nullptr);
}

void GraphicsProvider::destroyDescriptorSet()
{
    vkFreeDescriptorSets(this->logicalDevice, this->descriptorPool, 1, &this->descriptorSet);
}

void GraphicsProvider::drawFrame()
{
    vkWaitForFences(this->logicalDevice, 1, &this->inFlightFence, VK_TRUE, UINT64_MAX);

    this->counter++;

    uint32_t imageIndex = 0;
    VkResult result = vkAcquireNextImageKHR(this->logicalDevice, this->swapChain, UINT64_MAX, this->imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || this->frameBufferResized)
    {
        this->frameBufferResized = false;
        this->updateSwapChainNecessities();
        return;
    }

    if (result != VK_SUCCESS)
    {
        return;
    }

    vkResetFences(this->logicalDevice, 1, &this->inFlightFence);

    vkResetCommandBuffer(this->commandBuffer, 0);
    this->recordCommandBuffer(this->commandBuffer, imageIndex);

    this->updateUniformBuffer();

    VkSubmitInfo submitInfo{};
    submitInfo.pNext = nullptr;
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {this->imageAvailableSemaphore};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &this->commandBuffer;

    VkSemaphore signalSemaphores[] = {this->renderFinishedSemaphore};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(this->graphicsQueue, 1, &submitInfo, this->inFlightFence) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.pNext = nullptr;
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {this->swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr;

    vkQueuePresentKHR(this->presentQueue, &presentInfo);
}

uint32_t GraphicsProvider::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(this->physicalDevice, &memoryProperties);

    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
    {
        if ((typeFilter & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    throw std::runtime_error("Failed to find suitable memory type!");
}

void GraphicsProvider::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &bufferMemory)
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.pNext = nullptr;
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.size = size;
    bufferInfo.usage = usage;

    if (vkCreateBuffer(this->logicalDevice, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create vertex buffer!");
    }

    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(this->logicalDevice, buffer, &memoryRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.pNext = nullptr;
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memoryRequirements.size;
    allocInfo.memoryTypeIndex = this->findMemoryType(memoryRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(this->logicalDevice, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to allocate vertex buffer memory!");
    }

    vkBindBufferMemory(this->logicalDevice, buffer, bufferMemory, 0);
}

void GraphicsProvider::fillBufferMemory(VkDeviceMemory bufferMemory, const void *bufferData, VkDeviceSize bufferSize)
{
    void *data = nullptr;
    vkMapMemory(this->logicalDevice, bufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, bufferData, (size_t)bufferSize);
    vkUnmapMemory(this->logicalDevice, bufferMemory);
}

void GraphicsProvider::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
    commandBuffer = this->beginSingleCommandBuffer();

    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    this->endSingleCommandBuffer(commandBuffer);
}

VkCommandBuffer GraphicsProvider::beginSingleCommandBuffer()
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.pNext = nullptr;
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = this->commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(this->logicalDevice, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.pNext = nullptr;
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void GraphicsProvider::endSingleCommandBuffer(VkCommandBuffer commandBuffer)
{
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(this->graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(this->graphicsQueue); // vkWaitForFences
    vkFreeCommandBuffers(this->logicalDevice, this->commandPool, 1, &commandBuffer);
}

void GraphicsProvider::destroyBuffer(VkBuffer buffer, VkDeviceMemory bufferMemory)
{
    vkDestroyBuffer(this->logicalDevice, buffer, nullptr);
    vkFreeMemory(this->logicalDevice, bufferMemory, nullptr);
}

void GraphicsProvider::createTextureImage()
{
    // loadup image data

    int texWidth, texHeight, texChannels;
    stbi_uc *pixels = stbi_load("data\\textures\\2025-02-14 152530.png", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;

    if (!pixels)
    {
        throw std::runtime_error("Failed to load texture image!");
    }

    std::cout << "\nImage:\n";
    std::cout << "W:" << texWidth << "\n";
    std::cout << "H:" << texHeight << "\n";
    std::cout << "Size:" << imageSize << "\n";

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    this->createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
    this->fillBufferMemory(stagingBufferMemory, pixels, imageSize);

    stbi_image_free(pixels);

    // creatin image

    VkImageCreateInfo imageInfo{};
    imageInfo.pNext = nullptr;
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = static_cast<uint32_t>(texWidth);
    imageInfo.extent.height = static_cast<uint32_t>(texHeight);
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.flags = 0; // Optional

    if (vkCreateImage(this->logicalDevice, &imageInfo, nullptr, &this->textureImage) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create image!");
    }

    // binding memory

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(this->logicalDevice, this->textureImage, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.pNext = nullptr;
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    if (vkAllocateMemory(this->logicalDevice, &allocInfo, nullptr, &this->textureImageMemory) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to allocate image memory!");
    }

    vkBindImageMemory(this->logicalDevice, this->textureImage, this->textureImageMemory, 0);

    // buffers stuff

    this->transitionImageLayout(this->textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    this->copyBufferToImage(stagingBuffer, this->textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
    this->transitionImageLayout(this->textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    // cleanup

    vkDestroyBuffer(this->logicalDevice, stagingBuffer, nullptr);
    vkFreeMemory(this->logicalDevice, stagingBufferMemory, nullptr);
}

void GraphicsProvider::destroyTextureImage()
{
    vkDestroyImage(this->logicalDevice, this->textureImage, nullptr);
    vkFreeMemory(this->logicalDevice, this->textureImageMemory, nullptr);
}

void GraphicsProvider::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
{
    commandBuffer = this->beginSingleCommandBuffer();

    VkImageMemoryBarrier barrier{};
    barrier.pNext = nullptr;
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = 0;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else
    {
        throw std::invalid_argument("Unsupported layout transition!");
    }

    vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier);

    this->endSingleCommandBuffer(commandBuffer);
}

void GraphicsProvider::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
    commandBuffer = this->beginSingleCommandBuffer();

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = {0, 0, 0};
    region.imageExtent = {
        width,
        height,
        1};

    vkCmdCopyBufferToImage(
        commandBuffer,
        buffer,
        image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region);

    this->endSingleCommandBuffer(commandBuffer);
}