#include "game_state_machine.h"
#include "intro_state.h"
#include "menu_state.h"
#include "game_state.h"

GameStateMachine::GameStateMachine(sf::RenderWindow& _window, sfx::FrameClock& _clock) :
	window_(_window),
	clock_(_clock)
{}

bool GameStateMachine::Start()
{
	// Initialize first state
	AppState::Initialize<IntroState>(*this, current_state_);
	return true;
}

void GameStateMachine::Exit()
{
	// Terminate current state when exiting
	current_state_->TerminateState();
}

bool GameStateMachine::Update(const float _delta_time)
{
	// Delegate update to current state
	return current_state_->Update(_delta_time);
}

void GameStateMachine::Render(const float _delta_time)
{
	// Delegate render to current state
	current_state_->Render(_delta_time);
}

void GameStateMachine::ProcessEvent(sf::Event & _event)
{

}
