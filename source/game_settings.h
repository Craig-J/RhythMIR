#pragma once

#include <SFML/System/Time.hpp>
#include <SFML/Window/Keyboard.hpp>

#include <vector>

struct GameSettings
{
	sf::Time play_offset;
	sf::Time approach_time;
	sf::Time countdown_time;

	std::vector<sf::Keyboard::Key> keybinds;
	size_t path_count;

	bool auto_play;
	bool duncan_factor;
	bool flipped;

	float music_volume;
	float sfx_volume;

	enum HitSoundType
	{
		NONE,
		SOFT,
		DEEP
	};
	int hitsound;

	bool show_progress_bar;
	enum ProgressBarPosition
	{
		TOPRIGHT,
		ALONGTOP,
		ALONGBOTTOM
	};
	int progress_bar_position;

	enum BeatStyle
	{
		HIDDEN,
		INTERPOLATED,
		GENERATED
	};
	int beat_style;
};
