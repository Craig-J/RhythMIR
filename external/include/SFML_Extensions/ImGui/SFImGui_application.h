#pragma once

#include <SFML_Extensions/System/frame_clock.h>
#include <SFML_Extensions/System/system_HUD.h>

#include <SFML_Extensions/ImGui/imgui-events-SFML.h>
#include <SFML_Extensions/ImGui/imgui-rendering-SFML.h>
#include <SFML_Extensions/ImGui/console.h>
#include <ImGui/imgui.h>

#include "../settings.h"

namespace sfx
{
	class ImGuiApplication
	{
	public:

		ImGuiApplication(sf::RenderWindow& = sf::RenderWindow(sf::VideoMode::getDesktopMode(), "SFImGui Application"));
		virtual ~ImGuiApplication() {}

		void Run();

	protected:

		sfx::FrameClock clock_;
		sf::RenderWindow& window_;
		sf::Font font_;
		AppSettings settings_;

	private:

		virtual bool Initialize() = 0;
		virtual void CleanUp() = 0;
		virtual bool Update() = 0;
		virtual void Render() = 0;
		// Default event handling does nothing.
		virtual void ProcessEvent(sf::Event& _event) {}

		void EventLoop();

		ImGuiConsole console_;
		SystemHUD hud_;
		bool display_test_window_;
		bool running_;
	};
}
