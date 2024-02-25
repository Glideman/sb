#include "Application.h"
#include "base/Window.h"

#include <iostream>
#include <string>

void Application::run()
{
	Window *mainWindow = new Window();
	mainWindow->setCode("main");
	mainWindow->openWindow(640, 480);

	this->addWindow(mainWindow);

	while (true)
	{
		mainWindow->event();
		// for( int i = 0; i < 256; i++) {
		//     if(Keyboard[i] == SB_KEYACTION_CLICK) Keyboard[i] = SB_KEYACTION_PRESS;
		//     else if(Keyboard[i] == SB_KEYACTION_UP) Keyboard[i] = SB_KEYACTION_NONE;}

		if (mainWindow->isWindowClosed())
		{
			break;
		}
	}
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