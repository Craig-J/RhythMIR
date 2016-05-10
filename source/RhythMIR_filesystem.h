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

struct Filesystem
{
	static std::string beatmap_extension;
	static std::string current_skin_path;
};

static const std::string Skin(std::string _filename);


// Game Settings I/O
bool LoadGameSettings(GameSettings& _game_settings);
void SaveGameSettings(GameSettings& _game_settings);

// Song List I/O
void LoadSongList(SongList& _song_list);
void SaveSongList(SongList& _song_list);

// Beatmap List I/O
void LoadBeatmapList(const Song&, bool _force_load = false);
void SaveBeatmapList(const Song&);
void GetSongBeatmaps();

// Individual beatmap I/O is delegated to the aubio object.
void LoadBeatmap(const Beatmap&, bool _partial_load = true, bool _force_load = false);
void SaveBeatmap(const Beatmap&);
