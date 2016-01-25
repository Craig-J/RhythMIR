#include "intro_state.h"

#include "game_application.h"
#include "input.h"


IntroState::IntroState(const GameApplication* application) :
	AppState(application)
{
}


IntroState::~IntroState()
{
}

void IntroState::InitializeState()
{
	LoadTextures();

	// Calculate window dimensions and centre
	float window_width = (float)application_->getWindow().getSize().x;
	float window_height = (float)application_->getWindow().getSize().y;
	sf::Vector2f window_centre(window_width / 2.0f, window_height / 2.0f);

	splash_.initSprite(sf::Vector2f(window_centre.x, window_centre.y), splash_texture_);
	splash_.setDimensions(sf::Vector2f(window_width, window_height));

}

void IntroState::TerminateState()
{
	// State cleanup
}

GAMESTATE IntroState::Update(const int& frame_counter, Input& myInput)
{
	if (myInput.isKeyDown(VK_SPACE))
	{
		myInput.SetKeyUp(VK_SPACE);
		return MENU;
	}
	return INTRO;
}
void IntroState::Render(sf::Font& font, sf::RenderWindow& window)
{
	window.draw(splash_);
}

void IntroState::LoadTextures()
{
	splash_texture_.loadFromFile("bin/Splash Screen.png");
}