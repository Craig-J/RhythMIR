#include "menu_state.h"
#include <array>

namespace
{
	const int state_texture_count = 1;
	std::array<std::string, state_texture_count> texture_filenames =
	{
		"menu_background.png"
	};
}

void MenuState::InitializeState()
{
	LoadTextures();
}

void MenuState::TerminateState()
{
	// state cleanup
}

bool MenuState::Update(const float _delta_time)
{
	return true;
}

void MenuState::Render(const float _delta_time)
{
}

void MenuState::LoadTextures()
{
}