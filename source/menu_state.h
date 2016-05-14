#pragma once

#include "game_settings.h"
#include "appstate.h"
#include "RhythMIR_aubio.h"
#include "RhythMIR_filesystem.h"

class MenuState : public AppState
{
public:

	MenuState(AppStateMachine&,
			  UniqueStatePtr<AppState>&,
			  BeatmapPtr = nullptr);
	virtual ~MenuState() {}
	
	void InitializeState();
	void TerminateState();
	bool Update(const float _delta_time);
	void Render(const float _delta_time);
	void ProcessEvent(sf::Event & _event);
	void ReloadSkin();
	
private:

	// Beatmap stuff
	std::unique_ptr<Aubio> aubio_;
	BeatmapPtr loaded_beatmap_;
	std::atomic<bool> generating_beatmap_;
	std::atomic<bool> canceling_generating_;

	// Game settings stuff
	GameSettings game_settings_;
	bool display_settings_window_;
	void SettingsMenu();

	// Menu context objects
	enum MENUCONTEXT { SONGS, BEATMAPS};
	struct MenuContext
	{
		SongList::iterator song;
		BeatmapList::iterator beatmap;
		MENUCONTEXT context;
	} selected_;
	sfx::Sprite selector_;
	sf::Text heading_;

	// Song/beatmap objects
	SongList songs_;
	sf::Text song_text_;
	sf::Text beatmap_text_;

	struct GUI
	{
		bool main_window;
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

	bool MainWindow();
	void Play();

	void LoadBeatmap(const Beatmap&, bool _partial_load = true, bool _force_load = false);
	void SaveBeatmap(const Beatmap&);
	void GetSongBeatmaps();

	void GenerateTestBeatmap();
	void GenerateTestSong();
};
