#pragma once
#include "appstate.h"
#include "hit_object.h"
#include <vector>

class GameState : public AppState
{
public:

	GameState(GameStateMachine&, UniqueStatePtr<AppState>&, std::string&);
	virtual ~GameState() {}
	
	void InitializeState();
	void TerminateState();
	bool Update(const float _delta_time);
	void Render(const float _delta_time);
	
private:

	struct NoteType
	{
		sf::Vector2f start_pos;
		sf::Vector2f target_pos;
		sf::Time approach_time;
		sf::Time accuracy_tolerance;
	};

	void LoadSongData(std::string& _file_name);

	void InputLoop();
	void SpawnHitObject(const NoteType& _type);
	
	TexturePtr red_circle_texture_;
	TexturePtr blue_circle_texture_;
	TexturePtr green_circle_texture_;
	TexturePtr orange_circle_texture_;
	TexturePtr purple_circle_texture_;
	
	std::vector<std::unique_ptr<HitObject>> notes_;

	// Variables
	int score_;
};

