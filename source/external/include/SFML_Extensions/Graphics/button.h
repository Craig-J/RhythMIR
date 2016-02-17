#ifndef _SFX_BUTTON_H_
#define _SFX_BUTTON_H_
#include <SFML_Extensions\Graphics\sprite.h>

// TO-DO make a button derived class that executes a command
namespace sfx
{
	class Button : public Sprite
	{
	public:

		Button(sf::Vector2f _initial_position = sf::Vector2f(0.0f, 0.0f), bool _active = true, TexturePtr _active_texture = nullptr, TexturePtr _inactive_texture = nullptr);
		virtual ~Button() {}

		void Activate();
		void Deactivate();
		void Toggle();

	private:

		bool active_;
		TexturePtr active_texture_;
		TexturePtr inactive_texture_;

	};
}
#endif // _SFX_BUTTON_H_

/*struct BooleanState : public GenericState<Button, BooleanState>
{
BooleanState(Button& _machine, UniqueStatePtr<BooleanState>& _state, bool& _flag) :
GenericState::GenericState(_machine, _state),
state_(_flag)
{
}

void SetTrue()
{
state_ = true;
}
void SetFalse()
{
state_ = false;
}
void Toggle()
{
state_ != state_;
}

private:
bool& state_;
};

struct ActiveState : public GenericState<Button, ActiveState>
{
using GenericState::GenericState;
virtual void Activate() = 0;
virtual void Deactivate() = 0;
};
UniqueStatePtr<ActiveState> active_state_;

struct Active : public ActiveState
{
using ActiveState::ActiveState;
void InitializeState()
{
machine_.active_ = true;
machine_.setTexture(*machine_.active_texture_);
}
void TerminateState()
{
machine_.active_ = false;
machine_.setTexture(*machine_.inactive_texture_);
}
};

struct Inactive : public ActiveState
{
using ActiveState::ActiveState;
void InitializeState()
{
machine_.active_ = false;
machine_.setTexture(*machine_.inactive_texture_);
}
void TerminateState()
{
machine_.active_ = true;
machine_.setTexture(*machine_.active_texture_);

}
};*/