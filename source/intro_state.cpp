#include "intro_state.h"
#include <agnostic\logger.h>
using agn::Log;
#include "game_state_machine.h"
#include "menu_state.h"

namespace
{
}

void IntroState::InitializeState()
{
	LoadTextures();
	state_machine_.background_.setTexture(*background_texture_);
}

void IntroState::TerminateState()
{
	for (auto texture : textures_)
	{
		state_machine_.texture_manager_.Unload(texture.second);
	}
}

bool IntroState::Update(const float _delta_time)
{
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
	{
		ChangeState<MenuState>();
	}
	return true;
}

void IntroState::Render(const float _delta_time)
{
}

void IntroState::LoadTextures()
{
	for (auto texture : textures_)
	{
		texture.first = state_machine_.texture_manager_.Load(texture.second);
	}
}