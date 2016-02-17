#include "appstate.h"
#include "game_state_machine.h"

AppState::AppState(GameStateMachine& _state_machine, UniqueStatePtr<AppState>& _state) :
	GenericState(_state_machine, _state)
{
	machine_.background_.setTexture(*machine_.loading_background_texture_);
}

AppState::AppState(GameStateMachine& _state_machine, UniqueStatePtr<AppState>& _state, TexturePtr _loading_background_texture) :
	AppState(_state_machine, _state)
{
	machine_.background_.setTexture(*_loading_background_texture);
}