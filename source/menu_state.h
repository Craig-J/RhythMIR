#pragma once
#include "appstate.h"
#include <SFML_Extensions\Graphics\button.h>

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

	void LoadSongList(std::string& _file_name);

	std::vector<std::string> songs_;

	enum MENUCONTEXT { OPTIONS, SONGS, START };
	enum OPTION_SELECTIONS { MUSIC, SOUNDEFFECTS };
	struct Selection
	{
		std::string song;
		OPTION_SELECTIONS option;
		MENUCONTEXT context;
	} selected_;
	
	TexturePtr start_texture_;
	TexturePtr music_on_texture_, music_off_texture_;
	TexturePtr sound_on_texture_, sound_off_texture_;

	sfx::Button start_button_;
	sfx::Button	music_button_;
	sfx::Button sound_effects_button_;

	std::vector<std::string> headings_;
	sf::Text heading_;
};