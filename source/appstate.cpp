#include "appstate.h"
#include "game_state_machine.h"

AppState::AppState(GameStateMachine& _state_machine, agn::StateInstance<AppState>& _state) :
	GenericState(_state_machine, _state)
{
	state_machine_.background_.setTexture(*state_machine_.loading_background_texture_);
}

AppState::AppState(GameStateMachine& _state_machine, agn::StateInstance<AppState>& _state, sfx::TexturePtr _loading_background_texture) :
	GenericState(_state_machine, _state)
{
	state_machine_.background_.setTexture(*_loading_background_texture);
}