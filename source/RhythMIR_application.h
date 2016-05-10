#pragma once

#include "app_state_machine.h"

#include <SFML_Extensions/ImGui/SFImGui_application.h>

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

	// RhythMIR delegates everything to its state machine.
	AppStateMachine machine_;
};
