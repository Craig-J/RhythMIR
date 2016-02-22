#include "game_state.h"
#include "game_state_machine.h"
#include "menu_state.h"

namespace
{
	sf::Vector2f window_centre;
	float path_start_y;
	float path_end_y;
	float path_x;
	float path_x_offset;
	sf::Time approach_time;
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
		{ white_circle_texture_, "circle_white.png" },
		{ red_circle_texture_, "circle_red.png"},
		{ blue_circle_texture_, "circle_blue.png"},
		{ green_circle_texture_, "circle_green.png" },
		{ purple_circle_texture_, "circle_purple.png" }
	};
	Global::TextureManager.Load(textures_);

	machine_.background_.setTexture(*machine_.background_texture_);

	window_centre = sf::Vector2f(machine_.window_.getSize().x * 0.5, machine_.window_.getSize().y * 0.5);
	
	// Score Initialization
	score_text_.setFont(machine_.font_);
	score_text_.setCharacterSize(60);
	score_text_.setColor(sf::Color::Color(255, 69, 0));
	score_text_.setPosition(sf::Vector2f(window_centre.x, 0.0f));
	score_ = 0;

	// Beatmap and note path initialization
	approach_time = sf::milliseconds(1500);
	beatmap_.path_count_ = 4;
	path_start_y = 0.0f;
	path_end_y = machine_.window_.getSize().y * 0.9f;
	path_x = machine_.window_.getSize().x * 0.2f;
	path_x_offset = machine_.window_.getSize().x * 0.1f;
	for (int index = 0; index < beatmap_.path_count_; index++)
	{
		TexturePtr texture;
		switch (index)
		{
		case 0:
			texture = red_circle_texture_;
			break;
		case 1:
			texture = green_circle_texture_;
			break;
		case 2:
			texture = blue_circle_texture_;
			break;
		case 3:
			texture = purple_circle_texture_;
			break;
		default:
			texture = red_circle_texture_;
			break;
		}
		beatmap_.note_paths_.emplace_back(NotePath(sf::Vector2f(path_x + path_x_offset * index, path_start_y),
												   sf::Vector2f(path_x + path_x_offset * index, path_end_y),
												   approach_time,
												   1,
												   texture));
	}
	
	srand(1337);
	// 
}

void GameState::TerminateState()
{
	Global::TextureManager.Unload(textures_);
	textures_.clear();
}

bool GameState::Update(const float _delta_time)
{
	if (rand() % 200 == 0)
	{
		SpawnNote(beatmap_.note_paths_[rand() % 4]);
	}

	for (auto &path : beatmap_.note_paths_)
	{
		for (auto &note : path.notes)
		{
			note.UpdatePosition(_delta_time);
			note.VerifyPosition(machine_.window_);
		}
		auto path_index = path.notes.begin();
		while (path_index != path.notes.end())
		{
			if (path_index->visibility() == false)
				path_index = path.notes.erase(path_index);
			else
				++path_index;
		}
	}
	return true;
}

void GameState::Render(const float _delta_time)
{	
	for (auto path : beatmap_.note_paths_)
	{
		machine_.window_.draw(path.target);
		for (auto note : path.notes)
		{
			machine_.window_.draw(note);
		}
	}

	score_text_.setString("Score: " + std::to_string(score_));
	score_text_.move(-(score_text_.getGlobalBounds().width * 0.5f), -(score_text_.getGlobalBounds().height * 0.5f));
	machine_.window_.draw(score_text_);
	score_text_.move((score_text_.getGlobalBounds().width * 0.5f), (score_text_.getGlobalBounds().height * 0.5f));
}

void GameState::SpawnNote(NotePath& _path)
{
	_path.notes.emplace_back(Note(_path.start_position,
								  _path.target_position,
								  _path.approach_time,
								  _path.note_texture));
}