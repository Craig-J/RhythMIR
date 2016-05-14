#pragma once

#include "song.h"
#include "game_object.h"

#include <SFML_Extensions/Audio/audio_manager.h>
#include <SFML_Extensions/Graphics/texture_manager.h>

#include <queue>

// STORAGE RELATED OBJECTS
// Note abtracts the properties of a note as stored by the beatmap before gameplay
struct Note
{
public:
	Note(sf::Time& _offset);

	sf::Time offset;
	sf::Time duration;
	float frequency;
};

// TimingSection abstracts a section of constant BPM (or the whole song if BPM doesn't vary)
struct TimingSection
{
	TimingSection(float _BPM, sf::Time _offset);

	float BPM;
	sf::Time offset;
	std::vector<std::queue<Note>> notes;
};

// GAMEPLAY RELATED OBJECTS
enum PLAYMODE		// Generally corresponds to number of note paths for a beatmap
{
	UNKNOWN = -1,
	VISUALIZATION = 0,	// Catch-all for maps that don't have a play mode.
	SINGLE = 1,		// One synchronization path. Simplest case.
	FOURKEY = 4,	// Four note paths, similar to DDR style games.
	PIANO = 88,		// Eighty-eight note paths. Mimics a standard piano.
	

	PLAYMODE_COUNT
};

// NoteObject implements a game object for notes when they are to be used to create gameplay
class NoteObject : public GameObject
{
public:
	NoteObject(sf::Vector2f& _start_position,
			   sf::Vector2f& _target_position,
			   sf::Time& _approach_time,
			   sfx::TexturePtr _texture,
			   sf::Color _color = sf::Color::White);

	sf::Time offset_from_perfect;
};

// NotePath abstracts a lane where notes can be placed
struct NotePath
{
	NotePath(sf::Vector2f& _start_position,
			 sf::Vector2f& _target_position,
			 sf::Time& _approach_time,
			 int _accuracy,
			 sfx::TexturePtr _note_texture,
			 sf::Color _note_color = sf::Color::White);

	sf::Vector2f start_position;
	sf::Vector2f target_position;
	sf::Time approach_time;
	int accuracy;
	sfx::TexturePtr note_texture;
	sf::Color note_color;
	std::vector<NoteObject> notes;
	GameObject target;
};

// Beatmap abstracts the gameplay component of a song, including it's TimingSections and Music shared ptr
class Beatmap
{
public:

	Beatmap(const Song& _song,
			const std::string& _name,
			const std::string& _description = std::string(),
			const PLAYMODE& _mode = SINGLE);

	std::queue<TimingSection> CopyTimingSections() const;
	std::unique_ptr<std::queue<Note>> CopyBeats() const;

	void LoadMusic();
	void UnloadMusic();

	const Song song_;
	PLAYMODE play_mode_;
	sfx::MusicPtr music_;
	std::string name_;
	std::string description_;

	std::unique_ptr<std::queue<Note>> beats_;
	std::unique_ptr<std::vector<TimingSection>> sections_;

	std::string full_file_path() const
	{
		return song_.relative_path() + name_;
	}

	bool operator<(const Beatmap& _other) const
	{
		return(std::tie(song_, name_) < std::tie(_other.song_, _other.name_));
	}
	bool operator==(const Beatmap& _other) const
	{
		return(std::tie(song_, name_) == std::tie(_other.song_, _other.name_));
	}
	bool operator!=(const Beatmap& _other) const
	{
		return(std::tie(song_, name_) != std::tie(_other.song_, _other.name_));
	}
};

using SongList = std::map<Song, std::set<Beatmap>>;
using BeatmapList = std::set<Beatmap>;
using BeatmapPtr = std::shared_ptr<Beatmap>;
using UniqueBeatmapPtr = std::unique_ptr<Beatmap>;