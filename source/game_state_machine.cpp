#include "game_state_machine.h"
#include "intro_state.h"
#include "menu_state.h"
#include "game_state.h"

sfx::AudioManager GameStateMachine::audio_manager_;
sfx::TextureManager GameStateMachine::texture_manager_;
sfx::InputManager GameStateMachine::input_manager_;

GameStateMachine::GameStateMachine(const sfx::Application& _application) :
	application_(_application)
{}

void GameStateMachine::Start()
{
	// Initialize first state
	AppState::Initialize<IntroState>(*this, current_state_);
}

void GameStateMachine::Exit()
{
	// Delegate cleanup to final state
	current_state_->CleanUp();
}

void GameStateMachine::Update(const float _delta_time)
{
	// Delegate update to current state
	current_state_->Update(_delta_time);
}

void GameStateMachine::Render(const float _delta_time)
{
	// Delegate render to current state
	current_state_->Render(_delta_time);
}

