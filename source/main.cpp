#include <cstdlib> // For EXIT_SUCCESS
#include <agnostic\logger.h>
using agn::Log;
#include "RhythMIR_application.h"

int main()
{
	Log::EnableConsole();
	Log::Message("Successfully started!");
	sf::RenderWindow window(sf::VideoMode::getDesktopMode(), "RhythMIR", sf::Style::Fullscreen);
	RhythMIR application(window);
	application.Run();
	Log::Message("Successfully exited!");
	return EXIT_SUCCESS;
}