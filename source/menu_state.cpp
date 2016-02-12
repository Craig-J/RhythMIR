#include "menu_state.h"
#include <agnostic\logger.h>
using agn::Log;
#include "game_state_machine.h"
#include "intro_state.h"

namespace
{
}

void MenuState::InitializeState()
{
	LoadTextures();
	state_machine_.background_.setTexture(*background_texture_);
}

void MenuState::TerminateState()
{
	for (auto texture : textures_)
	{
		state_machine_.texture_manager_.Unload(texture.second);
	}
}

bool MenuState::Update(const float _delta_time)
{
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
	{
		ChangeState<IntroState>();
	}
	return true;
}

void MenuState::Render(const float _delta_time)
{
}

void MenuState::LoadTextures()
{
	for (auto texture : textures_)
	{
		texture.first = state_machine_.texture_manager_.Load(texture.second);
	}
}