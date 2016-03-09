#ifndef _SFX_SFIMGUI_APPLICATION_H_
#define _SFX_SFIMGUI_APPLICATION_H_
#include <ImGui\imgui.h>
#include <SFML_Extensions\Graphics\ImGui\imgui-events-SFML.h>
#include <SFML_Extensions\Graphics\ImGui\imgui-rendering-SFML.h>
#include <SFML_Extensions\System\frame_clock.h>
#include <SFML_Extensions\System\system_HUD.h>

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

	private:

		virtual bool Initialize() = 0;
		virtual void CleanUp() = 0;
		virtual bool Update() = 0;
		virtual void Render() = 0;
		// Default event handling does nothing.
		virtual void ProcessEvent(sf::Event& _event) {}

		void EventLoop();

		SystemHUD hud_;
		bool running_;
	};
}

#endif //_SFX_SFIMGUI_APPLICATION_H_