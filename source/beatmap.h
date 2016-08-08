#pragma once

#include "song.h"

#include <SFML_Extensions/Audio/audio_manager.h>
#include <SFML_Extensions/Graphics/texture_manager.h>

#include <queue>
#include <map>

// STORAGE RELATED OBJECTS
// Note abtracts the properties of a note as stored by the beatmap before gameplay
enum NoteType { SINGLE, SUSTAINED };
struct Note
{
	Note(NoteType _type, sf::Time& _offset);

	NoteType type;
	sf::Time offset;
	float frequency;
	sf::Time duration; // for sustained notes
};

// Section abstracts an annotated section of the music
struct Section : public TimingSection
{
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

// Beatmap abstracts the gameplay component of a song, including it's TimingSections and Music shared ptr
class Beatmap
{
public:

	Beatmap(const Song& _song,
			const std::string& _name,
			const std::string& _description = std::string(),
			const PLAYMODE& _mode = SINGLE);

	const std::vector<Section>& Sections() const;
	std::shared_ptr<std::queue<Note>> Beats() const;
	void SetSections(const std::vector<Section>& _sections);
	void SetBeats(const std::queue<Note>& _beats);

	void LoadMusic();
	void UnloadMusic();

	const Song& song_;
	PLAYMODE play_mode_;
	sfx::MusicPtr music_;
	std::string name_;
	std::string description_;

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

private:

	std::shared_ptr<std::queue<Note>> beats_;
	std::vector<Section> sections_;
};


using BeatmapSet = std::set<Beatmap>;
using BeatmapPtr = std::shared_ptr<Beatmap>;
using UniqueBeatmapPtr = std::unique_ptr<Beatmap>;