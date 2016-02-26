#pragma once
#include "song.h"
#include "game_object.h"
#include <SFML_Extensions\global.h>
#include <queue>

class Note : public GameObject
{
public:
	Note(sf::Vector2f& _start_position,
		 sf::Vector2f& _target_position,
		 sf::Time& _approach_time,
		 TexturePtr _texture,
		 sf::Color _color = sf::Color::White) :
		GameObject(_start_position, _texture)
	{
		sf::Vector2f difference = _target_position - _start_position;
		Accelerate(difference / _approach_time.asSeconds(), false);
		if(_color != sf::Color::White)
			setColor(_color);
		offset_from_perfect = _approach_time;
	}

	sf::Time offset_from_perfect;
};

struct NotePath
{
	NotePath(sf::Vector2f& _start_position,
			 sf::Vector2f& _target_position,
			 sf::Time& _approach_time,
			 int _accuracy,
			 TexturePtr _note_texture,
			 sf::Color _note_color = sf::Color::White) :
		start_position(_start_position),
		target_position(_target_position),
		approach_time(_approach_time),
		accuracy(_accuracy),
		note_texture(_note_texture),
		note_color(_note_color)
	{
		target = GameObject(_target_position, _note_texture);
		target.setColor(sf::Color(_note_color.r,
								  _note_color.g,
								  _note_color.b,
								  128));
	}

	sf::Vector2f start_position;
	sf::Vector2f target_position;
	sf::Time approach_time;
	int accuracy;
	TexturePtr note_texture;
	sf::Color note_color;
	std::vector<Note> notes;
	GameObject target;
};

// TimingSection abstracts a section of constant BPM (or the whole song if BPM doesn't vary)
struct TimingSection
{
	TimingSection(int _BPM, int _offset) :
		BPM(_BPM),
		offset(_offset)
	{
	}

	int BPM;
	int offset;
	std::queue<Note> notes;
};

class Beatmap
{
public:

	Beatmap(const Song& _song) :
		song_(_song),
		music_(Global::AudioManager.LoadMusic(_song.file_name_))
	{
	}

	const Song& song_;
	MusicPtr music_;
	std::vector<TimingSection> sections_;
	std::vector<NotePath> note_paths_;
};