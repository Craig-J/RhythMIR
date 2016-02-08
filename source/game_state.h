#pragma once
#include "appstate.h"
#include <vector>

class GameState : public AppState
{
public:

	using AppState::AppState;
	virtual ~GameState() {}
	
	void InitializeState();
	void TerminateState();
	void Update(const float _delta_time);
	void Render(const float _delta_time);
	
private:
	
	// Function Prototypes
	void LoadTextures();
	//void LoadSounds();
	void InputLoop();
	void SpawnNote(int _key);
	
	// Textures
	//sf::Texture note_texture_;
	
	// Game Objects
	//std::vector<GameObject*> notes_;

	// Variables
	int score_;
};

