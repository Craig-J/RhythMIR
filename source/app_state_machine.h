#pragma once

#include "appstate.h"

#include <SFML_Extensions/System/frame_clock.h>
#include <SFML_Extensions/Graphics/sprite.h>
#include <SFML_Extensions/settings.h>

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Font.hpp>

class AppStateMachine
{
public:

	AppStateMachine(sf::RenderWindow&, sfx::FrameClock&, sf::Font&, sfx::AppSettings&);
	~AppStateMachine() {}

	bool Start();
	void Exit();
	bool Update(const float _delta_time);
	void Render(const float _delta_time);
	void ProcessEvent(sf::Event& _event);

private:

	void ReloadSkin();

	// Each state is essentially a part of the state machine.
	// Friend is used so that states can be stored in their own files rather than everything here.
	// Indentation shows the state heirarchy.
	friend class AppState;
		friend class IntroState;
		friend class MenuState;
		friend class GameState;

	// Instances of top-level states here.
		// Instances of nested state machines should go in their parent states classes.
	UniqueStatePtr<AppState> current_state_;

	sf::RenderWindow& window_;
	sfx::FrameClock& clock_;
	sf::Font& font_;
	sfx::AppSettings& settings_;

	sfx::TexturePtr loading_background_texture_;
	sfx::TexturePtr background_texture_;
	sfx::Sprite background_;

};
