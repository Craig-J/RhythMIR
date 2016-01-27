#include "application.h"

namespace sfx
{
	Application::Application() :
		clock_(),
		window_(sf::VideoMode().getFullscreenModes()[0], "")
	{
	}

	Application::~Application()
	{
		window_.close();
	}

	bool Application::Run()
	{
		running_ = Initialize();
		while (running_)
		{
			clock_.beginFrame();
			if (running_)
			{
				// UPDATE
				Update();

				// RENDER
				Render();
			}
			clock_.endFrame();
		}
		return CleanUp();
	}
}