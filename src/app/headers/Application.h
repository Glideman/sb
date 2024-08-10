#pragma once

#include "core/Logger.h"
#include "graphics/GraphicsProvider.h"

#include <iostream>
#include <string>
#include <set>
#include <map>

class Application
{
private:
	Application()
	{
		this->graphicsProvider = new GraphicsProvider();
	}

	GraphicsProvider *graphicsProvider;

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
