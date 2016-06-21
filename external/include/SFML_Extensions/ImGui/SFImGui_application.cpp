#include <SFML_Extensions/ImGui/SFImGui_application.h>

#include <SFML_Extensions/global.h>

namespace sfx
{
	ImGuiApplication::ImGuiApplication(sf::RenderWindow& _window) :
		window_(_window),
		clock_(),
		font_(),
		hud_(clock_, font_),
		console_(_window),
		display_test_window_(false)
	{
		ImGui::SFML::SetRenderTarget(window_);
		ImGui::SFML::InitImGuiRendering();
		ImGui::SFML::SetWindow(window_);
		ImGui::SFML::InitImGuiEvents();

		settings_.limit_framerate_ = true;
		settings_.display_hud_ = true;
		window_.setVerticalSyncEnabled(settings_.limit_framerate_);
	}

	void ImGuiApplication::Run()
	{
		running_ = Initialize();
		static bool vsync = settings_.limit_framerate_;
		while (running_)
		{
			clock_.beginFrame();
			ImGui::SFML::UpdateImGui();
			ImGui::SFML::UpdateImGuiRendering();
			sfx::Global::Input.Update();
			EventLoop();
			if (Global::Input.KeyPressed(sf::Keyboard::F7))
			{
				settings_.limit_framerate_ = !settings_.limit_framerate_;
				/*if (unlimited_framerate_)
					window_.setFramerateLimit(0);
				else
					window_.setFramerateLimit(60);*/
			}
			if (Global::Input.KeyPressed(sf::Keyboard::F10))
			{
				settings_.display_hud_ = !settings_.display_hud_;
			}
			if (Global::Input.KeyPressed(sf::Keyboard::F11))
			{
				display_test_window_ = !display_test_window_;
			}
			if (vsync != settings_.limit_framerate_)
			{
				vsync = settings_.limit_framerate_;
				window_.setVerticalSyncEnabled(settings_.limit_framerate_);
			}
			if (running_)
			{
				if(display_test_window_) ImGui::ShowTestWindow();
				running_ = Update();
				if (running_)
				{
					window_.clear();
					Render();
					if (settings_.display_hud_)
					{
						window_.draw(hud_);
						console_.Draw("Console", &settings_.display_hud_);
					}
					
					ImGui::Render();
					window_.display();
				}
			}
			clock_.endFrame();
		}
		ImGui::SFML::Shutdown();
		CleanUp();
		Global::UnloadGlobalResources();
		window_.close();
	}

	void ImGuiApplication::EventLoop()
	{
		sf::Event event;
		while (window_.pollEvent(event))
		{
			ImGui::SFML::ProcessEvent(event);
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