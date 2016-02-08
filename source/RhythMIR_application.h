#pragma once
#include <SFML_Extensions\System\application.h>
#include "game_state_machine.h"

class RhythMIR : public sfx::Application
{
public:

	using sfx::Application::Application;
	~RhythMIR();

	bool Initialize();
	bool CleanUp();
	bool Update();
	void Render();

private:

	GameStateMachine machine_;
};