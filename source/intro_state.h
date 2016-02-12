#pragma once
#include "appstate.h"
#include <SFML_Extensions\Graphics\sprite.h>
#include <SFML_Extensions\Graphics\texture_manager.h>

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

	void LoadTextures();

	sfx::TexturePtr background_texture_;

	const std::vector<std::pair<sfx::TexturePtr&, const std::string>> textures_ =
	{
		{ background_texture_, "intro_background.jpg" }
	};
	

	sfx::Sprite background_;
};