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

	//void LoadSongList(std::string& _file_name);

	// Overall menu context objects
	enum MENUCONTEXT { OPTIONS, SONGS, ACTIONS };
	enum OPTION_SELECTIONS { MUSIC, SOUNDEFFECTS };
	enum ACTIONS_SELECTIONS { PLAY, GENERATE, EDIT };
	struct MenuContext
	{
		int song;
		ACTIONS_SELECTIONS action;
		OPTION_SELECTIONS option;
		MENUCONTEXT context;
	} selected_;
	Sprite selector_;
	TexturePtr selector_texture_;

	Aubio aubio_;
	Beatmap* beatmap_;

	// Heading Objects
	const std::vector<std::string> headings_ = {"Songs", "Actions" };
	sf::Text heading_;

	// Options objects
	/*TexturePtr music_on_texture_, music_off_texture_;
	TexturePtr sound_on_texture_, sound_off_texture_;
	sfx::Button	music_button_;
	sfx::Button sound_effects_button_;*/

	// Songs objects
	std::vector<Song> songs_;
	sf::Text song_text_;

	// Actions objects
	TexturePtr play_texture_;
	Sprite play_button_;
	TexturePtr generate_texture_;
	Sprite generate_button_;
	
};