#include "Application.h"

#include <iostream>
#include <string>
#include <set>
#include <map>

int Application::run()
{
	this->init();

	while (!glfwWindowShouldClose(this->device->getWindow()))
	{
		glfwPollEvents();
	}

	this->cleanup();

	return 0;
}

void Application::stop()
{
}

void Application::init()
{
	this->device->init();
}

void Application::cleanup()
{
	this->device->cleanup();
}
