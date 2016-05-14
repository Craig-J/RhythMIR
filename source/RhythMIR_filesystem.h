#pragma once

#include "beatmap.h"
#include "game_settings.h"

#include <boost/filesystem.hpp>
#include <RapidXML/rapidxml.hpp>
#include <RapidXML/rapidxml_utils.hpp>
#include <RapidXML/rapidxml_print.hpp>

// None of these are placed in an object or namespace.
// An object needlessly complicates the calling process since they're all going to be globals anyway.
// No point in making a namespace as these functions are all implementation specific.

enum BeatmapFileType
{
	RhythMIR,
	osu,
	StepMania
};


struct Filesystem
{
	static BeatmapFileType current_beatmap_type;
	static std::string current_skin_path;
	static std::string current_song_path;

	// Individual beatmap I/O is delegated to the aubio object.
	static BeatmapPtr LoadBeatmap(const Beatmap& _beatmap, bool _partial_load);
	static void SaveBeatmap(const Beatmap& _beatmap);
};

const std::string Skin(std::string _filename);


// Game Settings I/O
bool LoadGameSettings(GameSettings& _game_settings);
void SaveGameSettings(GameSettings& _game_settings);

// Song List I/O
void LoadSongList(SongList& _song_list);
void SaveSongList(SongList& _song_list);

// Beatmap List I/O
void LoadBeatmapList(const SongList::iterator& _song);
void SaveBeatmapList(const SongList::iterator& _song);

BeatmapPtr LoadRhythMIRBeatmap(const Beatmap& _beatmap, bool _partial_load);
void SaveRhythMIRBeatmap(const Beatmap& _beatmap);