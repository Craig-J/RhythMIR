#include "app_state_machine.h"

#include "menu_state.h" // Required to initialize to menu state.
#include "RhythMIR_filesystem.h"

#include <SFML_Extensions/global.h> // Access to global resource managers

AppStateMachine::AppStateMachine(sf::RenderWindow& _window, sfx::FrameClock& _clock, sf::Font& _font, bool& _display_hud) :
	window_(_window),
	clock_(_clock),
	font_(_font),
	display_hud_(_display_hud)
{
}

bool AppStateMachine::Start()
{
	ReloadSkin();

	// Initialize first state
	AppState::Initialize<MenuState>(*this, current_state_);
	return true;
}

void AppStateMachine::Exit()
{
	current_state_->TerminateState();
}

bool AppStateMachine::Update(const float _delta_time)
{
	return current_state_->Update(_delta_time);
}

void AppStateMachine::Render(const float _delta_time)
{
	window_.draw(background_);
	current_state_->Render(_delta_time);
}

void AppStateMachine::ProcessEvent(sf::Event & _event)
{
	current_state_->ProcessEvent(_event);
}

void AppStateMachine::ReloadSkin()
{
	loading_background_texture_ = sfx::Global::TextureManager.Load(Skin("loading_background.jpg"));

	auto window_dimensions = sf::Vector2f(static_cast<float>(window_.getSize().x), static_cast<float>(window_.getSize().y));
	auto window_centre = sf::Vector2f(static_cast<float>(window_.getSize().x) / 2.0f, static_cast<float>(window_.getSize().y) / 2.0f);

	background_ = sfx::Sprite(window_centre, loading_background_texture_);
	background_.SetDimensions(window_dimensions);

	if (font_.loadFromFile(Skin("NovaMono.ttf")))
	{
		Log::Message("NovaMono.ttf loaded successfully.");
	}
	else
	{
		Log::Error("NovaMono.ttf failed to load.");
	}

	if(current_state_)
		current_state_->ReloadSkin();
}
