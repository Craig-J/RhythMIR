#pragma once
#include "song.h"
#include "game_object.h"
#include <SFML_Extensions\global.h>

struct NoteLane
{
	explicit NoteLane() = default;
	NoteLane(sf::Vector2f& _start_position,
			 sf::Vector2f& _target_position,
			 sf::Time& _approach_time,
			 int _accuracy) :
		start_position(_start_position),
		target_position(_target_position),
		approach_time(_approach_time),
		accuracy(_accuracy)
	{
	}

	sf::Vector2f start_position;
	sf::Vector2f target_position;
	sf::Time approach_time;
	int accuracy;
};

struct TimingSection
{
	explicit TimingSection() = default;
	TimingSection(int _BPM, int _offset) :
		BPM(_BPM),
		offset(_offset)
	{}

	int BPM;
	int offset;
};

class Note : public GameObject
{
public:



private:
	const NoteLane& lane;
};

class Beatmap
{
public:

	Beatmap(const Song& _song) :
		song_(_song),
		music_(Global::AudioManager.LoadMusic(_song.file_name_))
	{}

	const Song& song_;
	MusicPtr music_;
	std::vector<TimingSection> sections_;
	std::vector<Note> notes_;
	std::vector<NoteLane> lanes_;
};