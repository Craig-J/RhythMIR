#pragma once
#include "appstate.h"
#include <SFML\Graphics\RenderWindow.hpp>
#include <SFML_Extensions\Graphics\sprite.h>

class IntroState : public AppState
{
public:

	using AppState::AppState;
	virtual ~IntroState() {}
	
	void InitializeState();
	void TerminateState();
	bool Update(const float _delta_time);
	void Render(const float _delta_time);

private:

	void PreLoadTextures();

	sfx::Sprite background_;
};