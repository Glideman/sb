#include "Core.h"
#include "Application.h"

BOOL IsWindowClosed = FALSE;

int main(int argc, char *argv[])
{
	Application::getInstance().run();

	return 0;
}