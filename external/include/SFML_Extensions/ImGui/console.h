#pragma once
#include <SFML_Extensions\ImGui\SFImGui.h>
#include <SFML_Extensions\global.h>
#include <sstream>
#include <vector>
namespace sfx
{
	struct LogMsg
	{
		std::string msg;
		enum class MSGTYPE { FATAL_ERROR_MSG, ERROR_MSG, WARNING_MSG, IMPORTANT_MSG, MSG } type;
	};

	class ImGuiConsole
	{
	public:

		ImGuiConsole(sf::RenderWindow&);
		~ImGuiConsole();

		void ClearLog();
		void Log(std::string _message);
		void Draw(const char* title, bool* opened);
		
	private:

		sf::RenderWindow& window_;

		void UpdateStream();

		std::vector<LogMsg> items_;
		bool scroll_to_bottom_;
		std::stringstream& out_;
	};
}