#include "menu_state.h"
#include "game_state_machine.h"
#include "intro_state.h"

void MenuState::InitializeState()
{
	textures_ = TextureFileVector
	{
		{machine_.background_texture_, "menu_background.jpg" }
	};
	Global::TextureManager.Load(textures_);
	

	machine_.background_.setTexture(default_texture_());
}

void MenuState::TerminateState()
{
	Global::TextureManager.Unload(textures_);
	textures_.clear();
}

bool MenuState::Update(const float _delta_time)
{
	if (Global::Input.KeyReleased(sf::Keyboard::Space))
	{
		ChangeState<IntroState>();
	}
	return true;
}

void MenuState::Render(const float _delta_time)
{
}