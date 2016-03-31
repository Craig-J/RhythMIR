#include <cstdlib> // For EXIT_SUCCESS
#include "RhythMIR_application.h"

int main()
{
	sf::RenderWindow window(sf::VideoMode::getDesktopMode(), "RhythMIR"/*, sf::Style::Fullscreen*/);
	RhythMIR application(window);
	application.Run();
	return EXIT_SUCCESS;
}