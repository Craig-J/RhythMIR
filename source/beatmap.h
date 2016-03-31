#pragma once
#include "song.h"
#include "game_object.h"
#include <SFML_Extensions\global.h>
#include <queue>

// STORAGE RELATED OBJECTS
// Note abtracts the properties of a note as stored by the beatmap before gameplay
struct Note
{
public:
	Note(sf::Time& _offset);

	sf::Time offset;
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
	SINGLE = 1,		// One synchronization path. Simplest case.
	FOURKEY = 4,	// Four note paths, similar to DDR style games.
	PIANO = 88		// Eighty-eight note paths. Mimics a standard piano.
};

// NoteObject implements a game object for notes when they are to be used to create gameplay
class NoteObject : public GameObject
{
public:
	NoteObject(sf::Vector2f& _start_position,
			   sf::Vector2f& _target_position,
			   sf::Time& _approach_time,
			   TexturePtr _texture,
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
			 TexturePtr _note_texture,
			 sf::Color _note_color = sf::Color::White);

	sf::Vector2f start_position;
	sf::Vector2f target_position;
	sf::Time approach_time;
	int accuracy;
	TexturePtr note_texture;
	sf::Color note_color;
	std::vector<NoteObject> notes;
	GameObject target;
};

// Beatmap abstracts the gameplay component of a song, including it's TimingSections and Music shared ptr
class Beatmap
{
public:

	// Explicit so that beatmap doesn't implicitly convert to a Song
	explicit Beatmap(const Song& _song, const PLAYMODE& _mode);
	~Beatmap();

	std::queue<TimingSection> CopyTimingSections();

	void LoadMusic();

	const Song song_;
	PLAYMODE play_mode_;
	MusicPtr music_;

private:

	// Aubio has direct access to a beatmaps sections for beatmap generation.
	friend class Aubio;

	std::vector<TimingSection> sections_;
};