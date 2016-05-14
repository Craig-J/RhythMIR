#pragma once

#include <SFML_Extensions/Audio/audio_manager.h>
#include <SFML_Extensions/Graphics/texture_manager.h>
#include <SFML/Window/Event.hpp>

#include <agnostic/generic_state.h>
using agn::UniqueStatePtr;

class AppState : public agn::GenericState<class AppStateMachine, AppState>
{
public:

	AppState(AppStateMachine&, UniqueStatePtr<AppState>&);
	AppState(AppStateMachine&, UniqueStatePtr<AppState>&, sfx::TexturePtr);
	virtual ~AppState() {}

	// Boolean returns indicate success or failure.
	virtual bool Update(const float _delta_time) = 0;
	virtual void Render(const float _delta_time) = 0;
	virtual void TerminateState() {}
	virtual void ProcessEvent(sf::Event & _event) {}
	virtual void ReloadSkin() {}

protected:

	sfx::TextureFileVector textures_;
	sfx::SoundFileVector sounds_;
};
