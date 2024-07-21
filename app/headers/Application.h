#pragma once

#include "base/Logger.h"
#include "device/Device.h"

#include <string>

class Application
{
private:
	Application()
	{
		this->device = new Device();
	}

	Device *device;

public:
	Application(Application const &) = delete;
	void operator=(Application const &) = delete;

	static Application &getInstance()
	{
		static Application pInstance;
		return pInstance;
	}

	int run();
	void stop();
	void init();
	void cleanup();
};
