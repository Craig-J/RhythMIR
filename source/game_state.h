#pragma once
#include "appstate.h"
#include "game_object.h"
#include "beatmap.h"
#include <vector>

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

	void SpawnNote(const NoteLane& _lane);

	Beatmap beatmap_;
	
	TexturePtr red_circle_texture_;
	TexturePtr blue_circle_texture_;
	TexturePtr green_circle_texture_;
	TexturePtr orange_circle_texture_;
	TexturePtr purple_circle_texture_;

	sf::Text score_text_;
	int score_;
};

