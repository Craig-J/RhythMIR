#pragma once
#include <SFML_Extensions\Graphics\ImGui\SFImGui_application.h>
#include "game_state_machine.h"

class RhythMIR : public sfx::ImGuiApplication
{
public:

	RhythMIR(sf::RenderWindow&);
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