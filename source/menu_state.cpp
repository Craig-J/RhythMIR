#include "menu_state.h"
#include "game_state_machine.h"
//#include "game_state.h"

namespace
{
	float context_horizontal_spacing;
	float options_x;
	float options_vertical_spacing;
	//float options_vertical_cuttoff;
	sf::Vector2f window_centre;
}

void MenuState::InitializeState()
{
	textures_ = TextureFileVector
	{
		{ machine_.background_texture_, "menu_background.jpg" },
		{ start_texture_, "transparent_start.png" },
		{ music_on_texture_, "transparent_music_on.png" },
		{ music_off_texture_, "transparent_music_off.png" },
		{ sound_on_texture_, "transparent_sound_on.png" },
		{ sound_off_texture_, "transparent_sound_off.png" }
	};
	Global::TextureManager.Load(textures_);
	
	machine_.background_.setTexture(*machine_.background_texture_);

	//LoadSongList();

	window_centre = sf::Vector2f(machine_.window_.getSize().x * 0.5, machine_.window_.getSize().y * 0.5);
	context_horizontal_spacing = machine_.window_.getSize().x * 0.25f;
	options_x = context_horizontal_spacing;
	options_vertical_spacing = machine_.window_.getSize().y * 0.1f;
	//options_vertical_cuttoff = machine_.window_.getSize().y * 0.3f;

	start_button_ = sfx::Button(sf::Vector2f(context_horizontal_spacing * 3, window_centre.y), start_texture_.get());

	music_button_ = sfx::Button(sf::Vector2f(options_x, window_centre.y), music_on_texture_.get());

	sound_effects_button_ = sfx::Button(sf::Vector2f(options_x, music_button_.getPosition().y + options_vertical_spacing), sound_on_texture_.get());

	heading_.setFont(machine_.font_);
	heading_.setCharacterSize(60);
	heading_.setColor(sf::Color::Color(255, 69, 0));
	heading_.setPosition(sf::Vector2f(context_horizontal_spacing, machine_.window_.getSize().y * 0.2f));

	headings_.push_back("Options");
	headings_.push_back("Songs");
	headings_.push_back("Do Stuff");

}

void MenuState::TerminateState()
{
	Global::TextureManager.Unload(textures_);
	textures_.clear();
}

bool MenuState::Update(const float _delta_time)
{
	switch (selected_.context)
	{
	case OPTIONS:
		break;
	case SONGS:
		break;
	case START:
		if (Global::Input.KeyReleased(Keyboard::Space))
		{
			// Check we actually have a string
			if (selected_.song != std::string())
			{
				// TO-DO validate the song filename
				//ChangeState<GameState>(selected_.song);
			}
			else
			{
				Log::Error("Song file name is invalid.");
			}
		}
		break;
	}
	return true;
}

void MenuState::Render(const float _delta_time)
{
	heading_.setPosition(context_horizontal_spacing, machine_.window_.getSize().y * 0.2f);
	for (auto heading : headings_)
	{
		heading_.setString(heading);
		heading_.move(-(heading_.getGlobalBounds().width * 0.5f), 0.0f);
		machine_.window_.draw(heading_);
		heading_.move((heading_.getGlobalBounds().width * 0.5f), 0.0f);
		heading_.move(context_horizontal_spacing, 0.0f);
	}

	machine_.window_.draw(start_button_);
	machine_.window_.draw(music_button_);
	machine_.window_.draw(sound_effects_button_);
}