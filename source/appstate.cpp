#include "appstate.h"

#include "app_state_machine.h"

AppState::AppState(AppStateMachine& _state_machine, UniqueStatePtr<AppState>& _state) :
	GenericState(_state_machine, _state)
{
	machine_.background_.setTexture(*machine_.loading_background_texture_);
}

AppState::AppState(AppStateMachine& _state_machine, UniqueStatePtr<AppState>& _state, sfx::TexturePtr _loading_background_texture) :
	AppState(_state_machine, _state)
{
	machine_.background_.setTexture(*_loading_background_texture);
}