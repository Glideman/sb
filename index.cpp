#include "Application.h"

BOOL IsWindowClosed = FALSE;

int main(int argc, char *argv[])
{
	return Application::getInstance().run();
}