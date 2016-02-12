#include "game_state_machine.h"
#include "intro_state.h"

GameStateMachine::GameStateMachine(sf::RenderWindow& _window, sfx::FrameClock& _clock) :
	window_(_window),
	clock_(_clock)
{}

bool GameStateMachine::Start()
{
	loading_background_texture_ = texture_manager_.Load("loading_background.jpg");
	// Calculate window dimensions and centre
	sf::Vector2f window_dimensions((float)window_.getSize().x, (float)window_.getSize().y);
	sf::Vector2f window_centre(window_dimensions.x / 2.0f, window_dimensions.y / 2.0f);

	background_ = sfx::Sprite(window_centre, loading_background_texture_.get());
	background_.SetDimensions(window_dimensions);

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
	window_.draw(background_);
	// Delegate render to current state
	current_state_->Render(_delta_time);
}

void GameStateMachine::ProcessEvent(sf::Event & _event)
{

}
