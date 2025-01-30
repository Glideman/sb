#include "Application.h"

int Application::run()
{
	this->init();

	while (!glfwWindowShouldClose(this->graphicsProvider->getWindow()))
	{
		glfwPollEvents();
		this->graphicsProvider->drawFrame();
	}

	this->cleanup();

	return 0;
}

void Application::stop()
{
}

void Application::init()
{
	// TODO Вынести в конфиг
	std::vector<std::string> dataFolders{"data", "..\\data"};
	Loader::getInstance().init(dataFolders);
	this->graphicsProvider->init();
}

void Application::cleanup()
{
	this->graphicsProvider->cleanup();
}
