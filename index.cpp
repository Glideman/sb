#include "Application.h"
#include <windows.h>

BOOL IsWindowClosed = FALSE;

int main(int argc, char *argv[])
{
	Application::getInstance().run();

	return 0;
}