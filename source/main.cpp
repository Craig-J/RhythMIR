#include "RhythMIR_application.h"

#include <cstdlib> // For EXIT_SUCCESS

int main()
{
	auto mode = sf::VideoMode::getDesktopMode();
	//auto mode = sf::VideoMode(mode.width * 0.9, mode.height * 0.9);
	sf::RenderWindow window(mode, "RhythMIR", sf::Style::Fullscreen);
	RhythMIR application(window);
	application.Run();
	return EXIT_SUCCESS;
}