#pragma once
#include "appstate.h"
#include "game_object.h"
#include "beatmap.h"
#include <vector>
#include <SFML_Extensions\System\clock.h>

class GameState : public AppState
{
public:

	GameState(GameStateMachine&, UniqueStatePtr<AppState>&, Song&);
	virtual ~GameState() {}
	
	void InitializeState();
	void TerminateState();
	bool Update(const float _delta_time);
	void Render(const float _delta_time);
	
private:

	void LoadSongData(std::string& _file_name);

	void SpawnNote(NotePath& _path);
	void AttemptNoteHit(NotePath& _path);

	Beatmap beatmap_;

	sfx::Clock play_clock_;
	
	sfx::Sprite pause_background_;
	
	TexturePtr pause_background_texture_;
	TexturePtr white_circle_texture_;

	sf::Text clock_text_;
	sf::Text countdown_text_;
	sf::Text score_text_;
	sf::Text hit_counters_text_;

	int perfect_hits_, great_hits_, good_hits_, misses_;
	int hit_combo_;
	int score_;
	bool paused_;
	bool finished_;
};