#include "intro_state.h"
#include "game_state_machine.h"
#include "menu_state.h"

void IntroState::InitializeState()
{
	textures_ = TextureFileVector
	{
		{machine_.background_texture_, "intro_background.jpg"}
	};
	Global::TextureManager.Load(textures_);

	machine_.background_.setTexture(*machine_.background_texture_);
}

void IntroState::TerminateState()
{
	Global::TextureManager.Unload(textures_);
	textures_.clear();
}

bool IntroState::Update(const float _delta_time)
{
	if (Global::Input.KeyPressed(Keyboard::Space))
	{
		ChangeState<MenuState>();
	}
	return true;
}

void IntroState::Render(const float _delta_time)
{
}