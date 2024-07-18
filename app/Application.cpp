#include "Core.h"
#include "Application.h"

#include <iostream>
#include <string>

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
	if (!glfwInit())
	{
		throw std::runtime_error("Cannot initialize GLFW!");
	}

	glfwSetErrorCallback(error_callback);

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	GLFWwindow *window = glfwCreateWindow(800, 600, APPLICATION_NAME, nullptr, nullptr);
	glfwSetKeyCallback(window, key_callback);

	this->checkInstanceExtensions();
	VkInstance *vulkanInstance = this->createVulkanInstance();
	this->pickPhysicalDevice();
	this->createLogicalDevice();

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
	}

	// cleanup
	vkDestroyDevice(this->logicalDevice, nullptr);
	vkDestroyInstance(this->vulkanInstance, nullptr);
	glfwDestroyWindow(window);
	glfwTerminate();
}

void Application::stop()
{
}

void Application::createVulkanInstance()
{
	VkApplicationInfo appInfo;
	appInfo.pNext = nullptr;
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = APPLICATION_NAME;
	appInfo.applicationVersion = APPLICATION_VERSION;
	appInfo.pEngineName = ENGINE_NAME;
	appInfo.engineVersion = ENGINE_VERSION;
	appInfo.apiVersion = VK_HEADER_VERSION_COMPLETE;

	VkInstanceCreateInfo createInfo;
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

	int i = 0;
	for (const auto &queueFamily : queueFamilies)
	{
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			indices.graphicsFamily = i;
			break;
		}

		i++;
	}

	return indices;
}

void Application::createLogicalDevice()
{
	QueueFamilyIndices indices = findQueueFamilies(this->physicalDevice);

	VkDeviceQueueCreateInfo queueCreateInfo;
	queueCreateInfo.pNext = nullptr;
	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
	queueCreateInfo.queueCount = 1;

	float queuePriority = 1.0f;
	queueCreateInfo.pQueuePriorities = &queuePriority;

	VkPhysicalDeviceFeatures deviceFeatures;

	VkDeviceCreateInfo createInfo;
	createInfo.pNext = nullptr;
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pQueueCreateInfos = &queueCreateInfo;
	createInfo.queueCreateInfoCount = 1;
	createInfo.pEnabledFeatures = &deviceFeatures;
	createInfo.enabledExtensionCount = 0;
	createInfo.enabledLayerCount = 0;

	if (vkCreateDevice(this->physicalDevice, &createInfo, nullptr, &this->logicalDevice) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create logical device!");
	}

	vkGetDeviceQueue(this->logicalDevice, indices.graphicsFamily.value(), 0, &this->graphicsQueue);
}