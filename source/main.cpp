#include <cstdlib>
#include <agnostic\logger.h>
using agn::Log;
#include "RhythMIR_application.h"

int main()
{
	Log::EnableConsole();
	Log::Message("Successfully started!");
	RhythMIR application(sf::VideoMode::getDesktopMode(), "RhythMIR");
	application.Run();
	Log::Message("Successfully exited!");
	while (true)
	{ }
	return EXIT_SUCCESS;
}