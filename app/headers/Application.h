#pragma once

#include "base/Logger.h"
#include "base/Window.h"

#include <map>
#include <string>

class Application
{
private:
	Application(){};

	std::map<HWND, Window *> windowMap;

public:
	Application(Application const &) = delete;
	void operator=(Application const &) = delete;

	static Application &getInstance()
	{
		static Application pInstance;
		return pInstance;
	}

	void run();
	void stop();

	bool isWindowExist(std::string code);
	bool isWindowExist(HWND handle);
	Window *getWindow(std::string code);
	Window *getWindow(HWND handle);
	void addWindow(Window *);
	void deleteWindow(std::string code);
	void deleteWindow(HWND handle);
};
