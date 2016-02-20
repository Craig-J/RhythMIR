#include "game_state.h"
#include "game_state_machine.h"
#include "menu_state.h"
#include <aubio\aubio.h>

namespace
{
	sf::Vector2f window_centre;
}

GameState::GameState(GameStateMachine& _state_machine, UniqueStatePtr<AppState>& _state, Song& _song) :
	AppState(_state_machine, _state),
	beatmap_(_song)
{
}

void GameState::InitializeState()
{
	textures_ = TextureFileVector
	{
		{ machine_.background_texture_, "play_background.jpg" },
		{ red_circle_texture_, "red_circle.png"},
		{ blue_circle_texture_, "blue_circle.png"},
		{ green_circle_texture_, "green_circle.png" },
		{ orange_circle_texture_, "orange_circle.png" },
		{ purple_circle_texture_, "purple_circle.png" }
	};
	Global::TextureManager.Load(textures_);

	machine_.background_.setTexture(*machine_.background_texture_);

	window_centre = sf::Vector2f(machine_.window_.getSize().x * 0.5, machine_.window_.getSize().y * 0.5);
	
	// Score Initialization
	score_text_.setFont(machine_.font_);
	score_text_.setCharacterSize(60);
	score_text_.setColor(sf::Color::Color(255, 69, 0));
	score_text_.setPosition(sf::Vector2f(window_centre.x, machine_.window_.getSize().y * 0.1f));
	score_ = 0;
}

void GameState::TerminateState()
{
	Global::TextureManager.Unload(textures_);
	textures_.clear();
}

bool GameState::Update(const float _delta_time)
{


	return true;
}

void GameState::Render(const float _delta_time)
{	
	for (auto note : beatmap_.notes_)
	{
		machine_.window_.draw(note);
	}

	machine_.window_.draw(score_text_);
}