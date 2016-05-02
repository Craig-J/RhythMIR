#pragma once
#include "appstate.h"
#include "RhythMIR_aubio.h"
#include <SFML_Extensions\Graphics\button.h>
#include <boost/filesystem.hpp>
#include "game_settings.h"

class MenuState : public AppState
{
public:

	MenuState(GameStateMachine&,
			  UniqueStatePtr<AppState>&,
			  std::unique_ptr<Beatmap> = nullptr);
	virtual ~MenuState() {}
	
	void InitializeState();
	void TerminateState();
	bool Update(const float _delta_time);
	void Render(const float _delta_time);
	void ProcessEvent(sf::Event & _event);
	
private:

	std::unique_ptr<Aubio> aubio_;
	std::unique_ptr<Beatmap> beatmap_;
	std::atomic<bool> generating_beatmap_;
	std::atomic<bool> canceling_generating_;

	GameSettings play_settings_;
	bool display_settings_window_;
	void SettingsMenu();

	bool LoadSettings();
	void SaveSettings();

	// Menu context objects
	enum MENUCONTEXT { SONGS, BEATMAPS};
	struct MenuContext
	{
		std::map<Song, std::set<Beatmap>>::iterator song;
		std::set<Beatmap>::iterator beatmap;
		MENUCONTEXT context;
	} selected_;
	Sprite selector_;
	sf::Text heading_;

	// Song/beatmap objects
	std::map<Song, std::set<Beatmap>> songs_;
	sf::Text song_text_;
	sf::Text beatmap_text_;

	struct GUI
	{
		bool input_focus;

		char song_artist[256];
		char song_title[256];
		char song_source[256];

		std::map<Song, std::set<Beatmap>>::iterator song_to_delete;
		std::string delete_song_popup;

		char beatmap_name[256];
		char beatmap_description[2048];

		std::set<Beatmap>::iterator beatmap_to_delete;
		std::string delete_beatmap_popup;
		std::string delete_all_beatmap_popup;
		bool deleting_enabled;

	} gui_;
	bool UpdateGUI();
	void Play();
	
	// Song List I/O
	void LoadSongList(const std::string&);
	void SaveSongList(const std::string&);

	// Beatmap List I/O
	void LoadBeatmapList(const Song&, bool _force_load = false);
	void SaveBeatmapList(const Song&);
	void GetSongBeatmaps();

	// Individual beatmap I/O is delegated to the aubio object.
	void LoadBeatmap(const Beatmap&, bool _partial_load = true, bool _force_load = false);
	void SaveBeatmap(const Beatmap&);

	void GenerateTestBeatmap();
	void GenerateTestSong();
};