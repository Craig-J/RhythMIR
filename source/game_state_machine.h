#ifndef _GAME_STATE_MACHINE_H_
#define _GAME_STATE_MACHINE_H_

#include <SFML_Extensions\Audio\audio_manager.h>
#include <SFML_Extensions\Graphics\texture_manager.h>
#include <SFML_Extensions\Window\input_manager.h>
#include <SFML_Extensions\System\application.h>
#include "appstate.h"

class GameStateMachine
{
public:

	GameStateMachine(const sfx::Application&);
	~GameStateMachine() {}

	void Start();
	void Exit();
	void Update(const float _delta_time);
	void Render(const float _delta_time);

private:

	static sfx::AudioManager audio_manager_;
	static sfx::TextureManager texture_manager_;
	static sfx::InputManager input_manager_;

	// Friend states so that they can use private static functions which shouldn't be public.
	// The alternative is putting every state definition here.
	friend class AppState;
	friend class IntroState;
	friend class MenuState;
	friend class GameState;

	// Instances of top-level states here.
	// Instances of nested state machines should go in their parent states classes.
	agn::StateInstance<AppState> current_state_;

	const sfx::Application& application_;

};
#endif // _GAME_STATE_MACHINE_H_