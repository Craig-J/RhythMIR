#pragma once
#include "appstate.h"
#include "game_object.h" // sfml graphics included implicitly (by gameobject)

class IntroState :
	public AppState
{
public:
	IntroState(const GameApplication* application);
	virtual ~IntroState();
	
	void InitializeState();
	void TerminateState();
	GAMESTATE Update(const int& frame_counter, Input& myInput);
	void Render(sf::Font& font, sf::RenderWindow& window);

private:
	void LoadTextures();

	sf::Texture splash_texture_;
	Sprite splash_;
};