#include "intro_state.h"
#include <agnostic\logger.h>
using agn::Log;
#include "game_state_machine.h"
#include <SFML_Extensions\Graphics\texture_manager.h>

namespace
{
	sfx::TxPtr background_texture;
	std::vector<std::pair<sfx::TxPtr, std::string>> textures =
	{
		{ background_texture, "intro_background.jpg"}
	};
}

void IntroState::InitializeState()
{
	PreLoadTextures();

	// Calculate window dimensions and centre
	float window_width = (float)state_machine_.window_.getSize().x;
	float window_height = (float)state_machine_.window_.getSize().y;
	sf::Vector2f window_centre(window_width / 2.0f, window_height / 2.0f);

	background_ = sfx::Sprite(sf::Vector2f(window_centre.x, window_centre.y), state_machine_.texture_manager_.Load().get());
	background_.SetDimensions(sf::Vector2f(window_width, window_height));

}

void IntroState::TerminateState()
{
}

bool IntroState::Update(const float _delta_time)
{
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape))
		return false;
	else
		return true;
}

void IntroState::Render(const float _delta_time)
{
	state_machine_.window_.draw(background_);
}

void IntroState::PreLoadTextures()
{
	for (auto texture : textures)
	{
		texture.first = state_machine_.texture_manager_.Load(texture.second);
	}
}