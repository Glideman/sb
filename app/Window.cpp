#include "Application.h"
#include "base/Window.h"
#include "base/Logger.h"

#include <stdexcept>
#include <string>

void Window::openWindow(unsigned int width, unsigned int height)
{
	WNDCLASS wndClass;
	DWORD wndStyle = WS_BORDER | WS_MINIMIZEBOX | WS_SYSMENU;
	RECT windowRect;

	this->instanceHandle = GetModuleHandle(NULL);

	if (this->instanceHandle)
	{
		wndClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
		wndClass.lpfnWndProc = (WNDPROC)(Window::windowCallback);
		wndClass.cbClsExtra = 0;
		wndClass.cbWndExtra = 0;
		wndClass.hInstance = this->instanceHandle;
		wndClass.hIcon = LoadIcon(NULL, IDI_WINLOGO);
		wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
		wndClass.hbrBackground = NULL;
		wndClass.lpszMenuName = NULL;
		wndClass.lpszClassName = this->windowClassName;

		if (RegisterClass(&wndClass))
		{
			this->windowHandle = CreateWindow(this->windowClassName, this->windowTitle, wndStyle, 0, 0, width, height, NULL, NULL, this->instanceHandle, NULL);

			if (!this->windowHandle)
			{
				throw std::runtime_error("Can't create window");
			}

			GetClientRect(this->windowHandle, &windowRect);
			SetWindowPos(this->windowHandle, NULL, 70, 5, 2 * width - windowRect.right, 2 * height - windowRect.bottom, 0);
			ShowWindow(this->windowHandle, SW_SHOW);
			SetForegroundWindow(this->windowHandle);
			SetFocus(this->windowHandle);
		}
		else
		{
			throw std::runtime_error("Can't reg wnd class");
		}
	}
	else
	{
		throw std::runtime_error("Can't get instance");
	}
}

void Window::closeWindow()
{
	SendMessage(this->windowHandle, WM_CLOSE, 0, 0);
}

void Window::event()
{
	MSG message;

	if (PeekMessage(&message, this->windowHandle, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&message);
		DispatchMessage(&message);
	}
}

HWND Window::getWindowHandle()
{
	return this->windowHandle;
}

LRESULT CALLBACK Window::windowCallback(HWND handle, UINT message, WPARAM first, LPARAM second)
{
	LRESULT result = FALSE;
	Window *wnd = NULL;

	switch (message)
	{
	case WM_DESTROY:
		LOG("wm_destroy\n");
		result = FALSE;
		break;

	case WM_CLOSE:
		LOG("wm_close\n");
		wnd = Application::getInstance().getWindow("main");
		if (wnd)
		{
			wnd->setIsWindowClosed(true);
		}
		result = FALSE;
		break;

	case WM_QUIT:
		LOG("wm_quit\n");
		result = FALSE;
		break;

	case WM_SETFOCUS:
		result = FALSE;
		break;

		// case WM_KILLFOCUS:
		//   memset(Keyboard, 0, 256);
		//   return FALSE;

	case WM_PAINT:
		result = FALSE;
		break;
		/*
			case WM_CHAR:
			  //LOG("input %i char\n", (UINT)first);
			  return FALSE;

			case WM_KEYDOWN:
			  if(Keyboard[first] != SB_KEYACTION_PRESS) Keyboard[first] = SB_KEYACTION_CLICK;
			  return FALSE;
			case WM_LBUTTONDOWN:
			  if(Keyboard[SB_KEY_LBUTTON] != SB_KEYACTION_PRESS) Keyboard[SB_KEY_LBUTTON] = SB_KEYACTION_CLICK;
			  return FALSE;
			case WM_RBUTTONDOWN:
			  if(Keyboard[SB_KEY_RBUTTON] != SB_KEYACTION_PRESS) Keyboard[SB_KEY_RBUTTON] = SB_KEYACTION_CLICK;
			  return FALSE;
			case WM_MBUTTONDOWN:
			  if(Keyboard[SB_KEY_MBUTTON] != SB_KEYACTION_PRESS) Keyboard[SB_KEY_MBUTTON] = SB_KEYACTION_CLICK;
			  return FALSE;

			case WM_KEYUP:
			  if(Keyboard[first] != SB_KEYACTION_NONE) Keyboard[first] = SB_KEYACTION_UP;
			  return FALSE;
			case WM_LBUTTONUP:
			  if(Keyboard[SB_KEY_LBUTTON] != SB_KEYACTION_NONE) Keyboard[SB_KEY_LBUTTON] = SB_KEYACTION_UP;
			  return FALSE;
			case WM_RBUTTONUP:
			  if(Keyboard[SB_KEY_RBUTTON] != SB_KEYACTION_NONE) Keyboard[SB_KEY_RBUTTON] = SB_KEYACTION_UP;
			  return FALSE;
			case WM_MBUTTONUP:
			  if(Keyboard[SB_KEY_MBUTTON] != SB_KEYACTION_NONE) Keyboard[SB_KEY_MBUTTON] = SB_KEYACTION_UP;
			  return FALSE;

			case WM_MOUSEWHEEL:
			  if(GET_WHEEL_DELTA_WPARAM(first) > 0) MouseWheel += 1;
			  else MouseWheel -= 1;
			  return FALSE;
		*/

	default:
		result = DefWindowProc(handle, message, first, second);
		break;
	};

	return result;
}

void Window::setIsWindowClosed(bool isClosed)
{
	this->isClosed = isClosed;
}

bool Window::isWindowClosed()
{
	return this->isClosed == true;
}

void Window::setCode(std::string code)
{
	this->code = code;
}

std::string Window::getCode()
{
	return this->code;
}