#include "application.h"

namespace sfx
{
	Application::Application(sf::VideoMode _video_mode = sf::VideoMode().getDesktopMode(), std::string _application_title = "SFML Application") :
		clock_(),
		window_(_video_mode, _application_title)
	{
	}

	void Application::Run()
	{
		running_ = Initialize();
		while (running_)
		{
			clock_.beginFrame();
			EventLoop();
			if (running_)
			{
				running_ = Update();
				if (running_)
				{
					window_.clear();
					Render();
					window_.display();
				}
			}
			clock_.endFrame();
		}
		window_.close();
		CleanUp();
	}

	void Application::EventLoop()
	{
		sf::Event event;
		while (window_.pollEvent(event))
		{
			switch (event.type)
			{
			case sf::Event::Closed:
				running_ = false;
				break;
			default:
				// Delegate all other event handling to virtual ProcessEvent function.
				ProcessEvent(event);
				break;
			}
		}
	}
}