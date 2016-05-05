#include <cstdlib> // For EXIT_SUCCESS
#include "RhythMIR_application.h"

int main()
{
	auto mode = sf::VideoMode::getDesktopMode();
	sf::RenderWindow window(sf::VideoMode(mode.width * 0.9, mode.height * 0.9), "RhythMIR", sf::Style::Fullscreen);
	RhythMIR application(window);
	application.Run();
	return EXIT_SUCCESS;
}