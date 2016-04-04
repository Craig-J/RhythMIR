#include "game_state_machine.h"
#include "menu_state.h"
#include <SFML_Extensions\global.h>

GameStateMachine::GameStateMachine(sf::RenderWindow& _window, sfx::FrameClock& _clock, sf::Font& _font) :
	window_(_window),
	clock_(_clock),
	font_(_font)
{
	options_.music_ = true;
	options_.sound_effects_ = true;
}

bool GameStateMachine::Start()
{
	loading_background_texture_ = sfx::Global::TextureManager.Load("loading_background.jpg");
	// Calculate window dimensions and centre
	sf::Vector2f window_dimensions((float)window_.getSize().x, (float)window_.getSize().y);
	sf::Vector2f window_centre(window_dimensions.x / 2.0f, window_dimensions.y / 2.0f);

	background_ = sfx::Sprite(window_centre, loading_background_texture_);
	background_.SetDimensions(window_dimensions);

	if (font_.loadFromFile("NovaMono.ttf"))
	{
		Log::Message("Font loaded successfully.");
	}
	else
	{
		Log::Error("Font failed to load.");
	}

	// Initialize first state
	AppState::Initialize<MenuState>(*this, current_state_);
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
	current_state_->ProcessEvent(_event);
}
