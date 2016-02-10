#pragma once
#include "appstate.h"
#include <SFML_Extensions\Graphics\sprite.h>
#include <SFML_Extensions\Graphics\texture_manager.h>
#include <vector>

class IntroState : public AppState
{
public:

	using AppState::AppState;
	virtual ~IntroState() {}
	
	void InitializeState();
	void TerminateState();
	void Update(const float _delta_time);
	void Render(const float _delta_time);

private:

	void LoadTextures();

	sfx::Sprite background_;
};