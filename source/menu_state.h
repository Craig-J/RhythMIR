#pragma once
#include "appstate.h"
#include "RhythMIR_aubio.h"
#include <SFML_Extensions\Graphics\button.h>
#include <boost/filesystem.hpp>

class MenuState : public AppState
{
public:

	using AppState::AppState;
	virtual ~MenuState() {}
	
	void InitializeState();
	void TerminateState();
	bool Update(const float _delta_time);
	void Render(const float _delta_time);
	
private:

	Aubio aubio_;
	Beatmap* beatmap_; // Pointer to the current beatmap.
	// Gets passed around between states so using naked pointer to avoid dealing with ownership.
	// Should really be a shared ptr but we only ever have one so just passing it around is simpler.

	// Menu context objects
	enum MENUCONTEXT { SONGS, BEATMAPS, ACTIONS };
	enum ACTIONS_SELECTIONS { PLAY, GENERATE };
	struct MenuContext
	{
		std::map<Song, std::set<Beatmap>>::iterator song;
		std::set<Beatmap>::iterator beatmap;
		ACTIONS_SELECTIONS action;
		MENUCONTEXT context;
	} selected_;
	Sprite selector_;
	sf::Text heading_;

	// Song/beatmap objects
	std::map<Song, std::set<Beatmap>> songs_;
	sf::Text song_text_;
	sf::Text beatmap_text_;

	// Actions objects
	Sprite play_button_;
	Sprite generate_button_;

	void GUI();
	
	// Song List I/O
	void LoadSongList(const std::string&);
	void SaveSongList(const std::string&);

	// Beatmap List I/O
	void LoadBeatmapList(const Song&, bool _force_load = false);
	void SaveBeatmapList(const Song&);
	void GetSongBeatmaps();

	// Individual beatmap I/O is delegated to the aubio object.
	void LoadBeatmap(const Beatmap&, bool _force_load = false);
	void SaveBeatmap(const Beatmap&);

	void GenerateTestBeatmaps();
	void GenerateTestSongs();
};