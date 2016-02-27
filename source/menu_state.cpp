#include "menu_state.h"
#include "game_state_machine.h"
#include "game_state.h"

namespace
{
	float context_horizontal_spacing;
	float context_vertical_cuttoff;
	float options_x;
	float options_vertical_spacing;
	float options_vertical_offset;
	float song_vertical_spacing;
	float song_vertical_offset;
	float action_vertical_spacing;
	float action_vertical_offset;
	sf::Vector2f window_centre;
}

void MenuState::InitializeState()
{
	textures_ = TextureFileVector
	{
		{ machine_.background_texture_, "menu_background.jpg" },
		{ play_texture_, "play_button.png" },
		{ generate_texture_, "generate_button.png" },
		{ music_on_texture_, "transparent_music_on.png" },
		{ music_off_texture_, "transparent_music_off.png" },
		{ sound_on_texture_, "transparent_sound_on.png" },
		{ sound_off_texture_, "transparent_sound_off.png" },
		{ selector_texture_, "circle_red.png" }
	};
	Global::TextureManager.Load(textures_);
	
	machine_.background_.setTexture(*machine_.background_texture_);

	// Calculate various layout stuff
	window_centre = sf::Vector2f(machine_.window_.getSize().x * 0.5, machine_.window_.getSize().y * 0.5);
	context_horizontal_spacing = machine_.window_.getSize().x * 0.25f;
	context_vertical_cuttoff = machine_.window_.getSize().y * 0.3f;
	options_x = context_horizontal_spacing;
	options_vertical_spacing = 100.0f;
	options_vertical_offset = 0.0f;
	song_vertical_spacing = 50.0f;
	song_vertical_offset = 0.0f;
	action_vertical_spacing = 150.0f;
	action_vertical_offset = 0.0f;

	selector_ = sfx::Sprite(sf::Vector2f(options_x, window_centre.y),
							selector_texture_);

	music_button_ = sfx::Button(sf::Vector2f(options_x, window_centre.y),
								machine_.options_.music_,
								music_on_texture_,
								music_off_texture_);

	sound_effects_button_ = sfx::Button(sf::Vector2f(options_x, music_button_.getPosition().y + options_vertical_spacing),
										machine_.options_.sound_effects_,
										sound_on_texture_,
										sound_off_texture_);

	play_button_ = sfx::Sprite(sf::Vector2f(context_horizontal_spacing * 3, window_centre.y),
							   play_texture_);

	generate_button_ = sfx::Sprite(sf::Vector2f(context_horizontal_spacing * 3, play_button_.getPosition().y + action_vertical_spacing),
								   generate_texture_);

	heading_.setFont(machine_.font_);
	heading_.setCharacterSize(60);
	heading_.setColor(sf::Color::Color(255, 69, 0));
	heading_.setPosition(sf::Vector2f(context_horizontal_spacing, machine_.window_.getSize().y * 0.2f));

	song_text_.setFont(machine_.font_);
	song_text_.setCharacterSize(24);
	song_text_.setColor(sf::Color::Color(192, 192, 192));
	song_text_.setPosition(sf::Vector2f(context_horizontal_spacing * 2, window_centre.y));

	//LoadSongList();

	songs_.push_back(Song{ "songs/guitar_vs_piano.wav", "Goukisan", "Guitar Vs. Piano 1.2" });

	selected_.context = OPTIONS;
	selected_.option = MUSIC;
	if(!songs_.empty()) // If there are any songs, set selected song to the first element of songs_
		selected_.song = 0;
	selected_.action = PLAY;

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
		{
			if (Global::Input.KeyPressed(Keyboard::D) || Global::Input.KeyPressed(Keyboard::Right))
			{
				selected_.context = SONGS;
				selector_.move(context_horizontal_spacing, 0.0f);
				break;
			}

			bool moved_vertically_this_update = false;
			switch (selected_.option)
			{
				case MUSIC:
					if (Global::Input.KeyPressed(Keyboard::S) || Global::Input.KeyPressed(Keyboard::Down))
					{
						selected_.option = SOUNDEFFECTS;
						options_vertical_offset = -options_vertical_spacing;
						moved_vertically_this_update = true;
						break;
					}
					if (Global::Input.KeyPressed(Keyboard::Space))
					{
						machine_.options_.music_ = !machine_.options_.music_;
						music_button_.Toggle();
					}
					break;
				case SOUNDEFFECTS:
					if (Global::Input.KeyPressed(Keyboard::W) || Global::Input.KeyPressed(Keyboard::Up))
					{
						selected_.option = MUSIC;
						options_vertical_offset = options_vertical_spacing;
						moved_vertically_this_update = true;
						break;
					}
					if (Global::Input.KeyPressed(Keyboard::Space))
					{
						machine_.options_.sound_effects_ = !machine_.options_.sound_effects_;
						sound_effects_button_.Toggle();
					}
					break;
			}
			if (moved_vertically_this_update)
			{
				music_button_.move(0.0f, options_vertical_offset);
				sound_effects_button_.move(0.0f, options_vertical_offset);
			}

			break;
		}

		case SONGS:
			if (Global::Input.KeyPressed(Keyboard::D) || Global::Input.KeyPressed(Keyboard::Right))
			{
				selected_.context = ACTIONS;
				selector_.move(context_horizontal_spacing, 0.0f);
				break;
			}
			if (Global::Input.KeyPressed(Keyboard::A) || Global::Input.KeyPressed(Keyboard::Left))
			{
				selected_.context = OPTIONS;
				selector_.move(-context_horizontal_spacing, 0.0f);
				break;
			}

			if (Global::Input.KeyPressed(Keyboard::S) || Global::Input.KeyPressed(Keyboard::Down))
			{
				if (selected_.song < songs_.size() - 1)
				{
					selected_.song++;
					song_vertical_offset -= song_vertical_spacing;
				}
			}
			if (Global::Input.KeyPressed(Keyboard::W) || Global::Input.KeyPressed(Keyboard::Up))
			{
				if (selected_.song > 0)
				{
					selected_.song--;
					song_vertical_offset += song_vertical_spacing;
				}
			}
			break;

		case ACTIONS:
		{
			
			if (Global::Input.KeyPressed(Keyboard::A) || Global::Input.KeyPressed(Keyboard::Left))
			{
				selected_.context = SONGS;
				selector_.move(-context_horizontal_spacing, 0.0f);
				break;
			}

			bool moved_vertically_this_update = false;
			switch (selected_.action)
			{
				case PLAY:
					if (Global::Input.KeyPressed(Keyboard::S) || Global::Input.KeyPressed(Keyboard::Down))
					{
						selected_.action = GENERATE;
						action_vertical_offset = -action_vertical_spacing;
						moved_vertically_this_update = true;
					}
					if (Global::Input.KeyPressed(Keyboard::Space))
					{
						ChangeState<GameState>(*beatmap_);
					}
					break;
				case GENERATE:
					if (Global::Input.KeyPressed(Keyboard::W) || Global::Input.KeyPressed(Keyboard::Up))
					{
						selected_.action = PLAY;
						action_vertical_offset = action_vertical_spacing;
						moved_vertically_this_update = true;
					}
					if (Global::Input.KeyPressed(Keyboard::Space))
					{
						beatmap_ = aubio_.GenerateBeatmap(songs_[selected_.song]);
					}
					break;
			}
			if (moved_vertically_this_update)
			{
				play_button_.move(0.0f, action_vertical_offset);
				generate_button_.move(0.0f, action_vertical_offset);
			}
			break;
		}
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

	song_text_.setPosition(context_horizontal_spacing * 2, window_centre.y + song_vertical_offset);
	for (auto song : songs_)
	{
		// Don't draw song text unless it's past the vertical cutoff value
		if (song_text_.getPosition().y > context_vertical_cuttoff)
		{
			song_text_.setString(song.artist_ + " - " + song.title_);
			song_text_.move(-(song_text_.getGlobalBounds().width * 0.5f), -(song_text_.getGlobalBounds().height * 0.5f));
			machine_.window_.draw(song_text_);
			song_text_.move((song_text_.getGlobalBounds().width * 0.5f), (song_text_.getGlobalBounds().height * 0.5f));
		}
		song_text_.move(0.0f, song_vertical_spacing);
	}

	// Don't draw options buttons unless they are past the vertical cutoff value
	if(music_button_.getPosition().y > context_vertical_cuttoff)
		machine_.window_.draw(music_button_);

	if (sound_effects_button_.getPosition().y > context_vertical_cuttoff)
		machine_.window_.draw(sound_effects_button_);

	if(play_button_.getPosition().y > context_vertical_cuttoff)
		machine_.window_.draw(play_button_);

	if (generate_button_.getPosition().y > context_vertical_cuttoff)
		machine_.window_.draw(generate_button_);

	machine_.window_.draw(selector_);
}