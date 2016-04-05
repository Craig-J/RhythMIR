#ifndef _GAME_STATE_MACHINE_H_
#define _GAME_STATE_MACHINE_H_

#include <SFML_Extensions\System\application.h>
#include "appstate.h"

class GameStateMachine
{
public:

	GameStateMachine(sf::RenderWindow&, sfx::FrameClock&, sf::Font&, bool&);
	~GameStateMachine() {}

	bool Start();
	void Exit();
	bool Update(const float _delta_time);
	void Render(const float _delta_time);
	void ProcessEvent(sf::Event& _event);

private:

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
	bool& display_hud_;

	struct Options
	{
		bool music_;
		bool sound_effects_;
	} options_;

	TexturePtr loading_background_texture_;
	TexturePtr background_texture_;
	Sprite background_;

};
#endif // _GAME_STATE_MACHINE_H_