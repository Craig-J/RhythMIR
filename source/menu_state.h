#pragma once
#include "appstate.h"

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

	enum SELECTION { STARTGAME, SONG_SELECTED, MUSIC, SOUNDEFFECTS}; // possible menu selections
	SELECTION selection_;	// current menu selection
	
	/*sf::Texture start_texture_;
	sf::Texture music_on_texture_, music_off_texture_;
	sf::Texture sound_on_texture_, sound_off_texture_;
	sf::Texture blue_square_, green_square_, yellow_diamond_, red_diamond_, purple_polygon_;
	sf::Texture cross_texture_, dpad_texture_;
	sf::Texture selector_;
	GameObject start_button_,  music_button_, sound_effects_button_;
	GameObject difficulty_button[kDifficultySettings];
	GameObject dpad_, cross_;
	GameObject selection_pointer_;*/
};

