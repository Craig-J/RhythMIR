#include "game_state_machine.h"
#include "intro_state.h"
#include "menu_state.h"
#include "game_state.h"

void GameStateMachine::Initialize()
{
	AppState::Initialize<IntroState>(*this, game_state_);
}