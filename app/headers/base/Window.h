#pragma once

#include <windows.h>
#include <string>

class Window
{
private:
	HINSTANCE instanceHandle;
	HWND windowHandle;
	LPCWSTR windowClassName = L"EWNDCLASS";
	LPCWSTR windowTitle = L"Виндов";
	bool isClosed = false;
	std::string code;

	static LRESULT CALLBACK windowCallback(HWND handle, UINT message, WPARAM first, LPARAM second);

public:
	void openWindow(unsigned int width, unsigned int height);
	void closeWindow();
	void resizeWindow(unsigned int width, unsigned int height);
	void event();
	HWND getWindowHandle();

	void setIsWindowClosed(bool isClosed);
	bool isWindowClosed();
	void setCode(std::string code);
	std::string getCode();
};