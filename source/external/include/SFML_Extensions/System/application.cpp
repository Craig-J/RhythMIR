#include "application.h"
#include <SFML_Extensions\global.h>

namespace sfx
{
	Application::Application(sf::VideoMode _video_mode = sf::VideoMode().getDesktopMode(), std::string _application_title = "SFML Application") :
		window_(_video_mode, _application_title),
		clock_(),
		font_(),
		hud_(clock_, font_)
	{
	}

	void Application::Run()
	{
		running_ = Initialize();
		while (running_)
		{
			clock_.beginFrame();
			sfx::Global::Input.Update();
			EventLoop();
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::LAlt) && sf::Keyboard::isKeyPressed(sf::Keyboard::Escape))
			{
				running_ = false;
			}
			if (running_)
			{
				running_ = Update();
				if (running_)
				{
					window_.clear();
					Render();
					window_.draw(hud_);
					window_.display();
				}
			}
			clock_.endFrame();
		}
		window_.close();
		CleanUp();
		Global::UnloadGlobalResources();
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

				// BASIC I/O HANDLING
				// Does not handle several SFML events.
			case sf::Event::MouseMoved:
				sfx::Global::Input.SetMouse(sf::Vector2i(event.mouseMove.x, event.mouseMove.y));
				break;
			case sf::Event::MouseButtonPressed:
				sfx::Global::Input.SetButtonDown(event.mouseButton.button);
				sfx::Global::Input.SetMouse(sf::Vector2i(event.mouseMove.x, event.mouseMove.y));
				break;
			case sf::Event::MouseButtonReleased:
				sfx::Global::Input.SetButtonUp(event.mouseButton.button);
				sfx::Global::Input.SetMouse(sf::Vector2i(event.mouseMove.x, event.mouseMove.y));
				break;
			case sf::Event::KeyPressed:
				sfx::Global::Input.SetKeyDown(event.key.code);
				break;
			case sf::Event::KeyReleased:
				sfx::Global::Input.SetKeyUp(event.key.code);
				break;
			default:
				// Delegate all other event handling to virtual ProcessEvent function.
				ProcessEvent(event);
				break;
			}
		}
	}
}