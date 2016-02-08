#include "application.h"

namespace sfx
{
	Application::Application(sf::VideoMode _video_mode = sf::VideoMode().getDesktopMode(), std::string _application_title = "SFML Application") :
		clock_(),
		window_(_video_mode, _application_title)
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