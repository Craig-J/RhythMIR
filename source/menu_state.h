#pragma once
#include "appstate.h"
#include <SFML_Extensions\Graphics\button.h>
#include "RhythMIR_aubio.h"

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
	enum MENUCONTEXT { SONGS, ACTIONS };
	enum ACTIONS_SELECTIONS { PLAY, GENERATE, EDIT };
	struct MenuContext
	{
		std::set<Song>::iterator song;
		ACTIONS_SELECTIONS action;
		MENUCONTEXT context;
	} selected_;
	Sprite selector_;
	sf::Text heading_;

	// Song objects
	std::set<Song> songs_;
	std::set<std::pair<std::string, PLAYMODE>> beatmaps_;
	sf::Text song_text_;

	// Actions objects
	PLAYMODE play_mode_;
	Sprite play_button_;
	Sprite generate_button_;

	void PlayGUI(bool*);
	
	// Song List I/O
	void LoadSongList(const std::string&);
	void SaveSongList(const std::string&);

	// Beatmap List I/O
	void LoadBeatmapList(const std::string&);
	void SaveBeatmapList(const std::string&);

	// Individual beatmap I/O is done by the aubio object.
};