#include "base/Core.h"
#include "Application.h"

#include <iostream>
#include <string>
#include <set>
#include <map>

void Application::error_callback(int error, const char *description)
{
	fprintf(stderr, "Error: %s\n", description);
}

void Application::key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void Application::run()
{
	this->init();

	while (!glfwWindowShouldClose(this->window))
	{
		glfwPollEvents();
	}

	this->cleanup();
}

void Application::stop()
{
}

void Application::init()
{
	this->createWindow();
	this->checkInstanceExtensions();
	this->createVulkanInstance();
	this->createSurface();
	this->pickPhysicalDevice();
	this->createLogicalDevice();
}

void Application::cleanup()
{
	vkDestroyDevice(this->logicalDevice, nullptr);
	vkDestroySurfaceKHR(this->vulkanInstance, this->vulkanSurface, nullptr);
	vkDestroyInstance(this->vulkanInstance, nullptr);
	glfwDestroyWindow(this->window);
	glfwTerminate();
}

void Application::createWindow()
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

void Application::createVulkanInstance()
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

VkInstance *Application::getVulkanInstance()
{
	return &this->vulkanInstance;
}

void Application::checkInstanceExtensions()
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

void Application::pickPhysicalDevice()
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

bool Application::isDeviceSuitable(VkPhysicalDevice device)
{
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);

	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	QueueFamilyIndices indices = this->findQueueFamilies(device);

	return indices.isComplete() &&
		   (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU || deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) &&
		   deviceFeatures.geometryShader;
}

QueueFamilyIndices Application::findQueueFamilies(VkPhysicalDevice device)
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

	return indices;
}

void Application::createLogicalDevice()
{
	QueueFamilyIndices indices = findQueueFamilies(this->physicalDevice);

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = {
		indices.graphicsFamily.value(),
		indices.presentFamily.value()};

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
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pEnabledFeatures = &deviceFeatures;
	createInfo.enabledExtensionCount = 0;
	createInfo.enabledLayerCount = 0;

	VkResult result = vkCreateDevice(this->physicalDevice, &createInfo, nullptr, &this->logicalDevice);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error(std::format("Failed to create logical device! Code {}", (int)result));
	}

	vkGetDeviceQueue(this->logicalDevice, indices.graphicsFamily.value(), 0, &this->graphicsQueue);
	vkGetDeviceQueue(this->logicalDevice, indices.presentFamily.value(), 0, &this->presentQueue);
}

void Application::createSurface()
{
	VkResult result = glfwCreateWindowSurface(this->vulkanInstance, this->window, nullptr, &this->vulkanSurface);
	if (result != VK_SUCCESS)
	{
		throw std::runtime_error(std::format("Failed to create window surface! Code {}", (int)result));
	}
}