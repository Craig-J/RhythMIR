#include "intro_state.h"
#include <agnostic\logger.h>
#include "game_state_machine.h"
#include <utility>

namespace
{
	std::vector<std::shared_ptr<sfx::Texture>> textures;
	std::vector<agn::ResourceID> texture_files =
	{
		{"intro_background.jpg", 0}
	};
}

void IntroState::InitializeState()
{
	LoadTextures();

	// Calculate window dimensions and centre
	float window_width = (float)state_machine_.application_.window_.getSize().x;
	float window_height = (float)state_machine_.application_.window_.getSize().y;
	sf::Vector2f window_centre(window_width / 2.0f, window_height / 2.0f);

	background_ = sfx::Sprite(sf::Vector2f(window_centre.x, window_centre.y), textures[0].get());
	background_.SetDimensions(sf::Vector2f(window_width, window_height));

}

void IntroState::TerminateState()
{
	
	textures.clear();
}

void IntroState::Update(const float _delta_time)
{
	/*if (myInput.isKeyDown(VK_SPACE))
	{
		myInput.SetKeyUp(VK_SPACE);
		return MENU;
	}
	return INTRO;*/
}
void IntroState::Render(const float _delta_time)
{
	//window.draw(splash_);
}

void IntroState::LoadTextures()
{
	textures.reserve(texture_files.size());
	for (auto file : texture_files)
	{
		textures.push_back(state_machine_.texture_manager_.LoadTexture(file.first));
	}
}