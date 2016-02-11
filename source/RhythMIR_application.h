#pragma once
#include <SFML_Extensions\System\application.h>
#include "game_state_machine.h"

class RhythMIR : public sfx::Application
{
public:

	RhythMIR(sf::VideoMode _video_mode, std::string _application_title);
	~RhythMIR();

	bool Initialize();
	void CleanUp();
	bool Update();
	void Render();
	void ProcessEvent(sf::Event& _event);

private:

	// RhythMIR delegates pretty much everything to its state machine.
	GameStateMachine machine_;
};