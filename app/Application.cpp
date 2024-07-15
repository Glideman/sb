#include "Application.h"
#include "base/Window.h"

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
	// Window *mainWindow = new Window();
	// mainWindow->setCode("main");
	// mainWindow->openWindow(640, 480);

	// this->addWindow(mainWindow);

	// vkCreateWin32SurfaceKHR

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

	//	while (true)
	//	{
	//		mainWindow->event();
	//		// for( int i = 0; i < 256; i++) {
	//		//     if(Keyboard[i] == SB_KEYACTION_CLICK) Keyboard[i] = SB_KEYACTION_PRESS;
	//		//     else if(Keyboard[i] == SB_KEYACTION_UP) Keyboard[i] = SB_KEYACTION_NONE;}
	//
	//		if (mainWindow->isWindowClosed())
	//		{
	//			break;
	//		}
	//	}

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
	}

	// cleanup
	vkDestroyInstance(*vulkanInstance, nullptr);
	glfwDestroyWindow(window);
	glfwTerminate();
}

void Application::stop()
{
}

bool Application::isWindowExist(std::string code)
{
	bool found = false;

	for (const auto &[key, value] : this->windowMap)
	{
		if (value->getCode().compare(code) == 0)
		{
			found = true;
		}
	}

	return found;
}

bool Application::isWindowExist(HWND handle)
{
	return this->windowMap.contains(handle);
}

Window *Application::getWindow(std::string code)
{
	for (const auto &[key, value] : this->windowMap)
	{
		if (value->getCode().compare(code) == 0)
		{
			return value;
		}
	}

	return NULL;
}

Window *Application::getWindow(HWND handle)
{
	return this->windowMap.contains(handle) ? this->windowMap[handle] : NULL;
}

void Application::addWindow(Window *window)
{
	if (!this->windowMap.contains(window->getWindowHandle()))
	{
		this->windowMap[window->getWindowHandle()] = window;
	}
}

void Application::deleteWindow(std::string code)
{
	for (const auto &[key, value] : this->windowMap)
	{
		if (value->getCode().compare(code) == 0)
		{
			this->windowMap.erase(value->getWindowHandle());
		}
	}
}

void Application::deleteWindow(HWND handle)
{
	if (this->windowMap.contains(handle))
	{
		this->windowMap.erase(handle);
	}
}

VkInstance *Application::createVulkanInstance()
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

	return &this->vulkanInstance;
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

	return (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU || deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) &&
		   deviceFeatures.geometryShader;
}
