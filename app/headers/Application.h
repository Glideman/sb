#pragma once

#include "Core.h"
#include "base/Logger.h"

#include <map>
#include <string>

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.hpp>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define APPLICATION_VERSION VK_MAKE_VERSION(1, 0, 0)
#define APPLICATION_NAME "App"

#define ENGINE_VERSION VK_MAKE_VERSION(0, 1, 0)
#define ENGINE_NAME "Sandbox"

class Application
{
private:
	Application()
	{
		this->preferedDeviceName = "NVIDIA GeForce RTX 4080";
		this->physicalDevice = VK_NULL_HANDLE;
	};

	std::string preferedDeviceName;
	std::string selectedDeviceName;

	VkInstance vulkanInstance;
	VkPhysicalDevice physicalDevice;
	VkDevice logicalDevice;
	VkQueue graphicsQueue;

public:
	Application(Application const &) = delete;
	void operator=(Application const &) = delete;

	static Application &getInstance()
	{
		static Application pInstance;
		return pInstance;
	}

	static void error_callback(int error, const char *description);
	static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);

	void run();
	void stop();

	void createVulkanInstance();
	VkInstance *getVulkanInstance();
	void checkInstanceExtensions();
	void pickPhysicalDevice();
	bool isDeviceSuitable(VkPhysicalDevice device);
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
	void createLogicalDevice();
};
