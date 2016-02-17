#ifndef _APPSTATE_H_
#define _APPSTATE_H_

#include <agnostic\generic_state.h>
using agn::UniqueStatePtr;

#include <SFML_Extensions\global.h>
using sfx::Global;

using sf::Keyboard;
using sf::Mouse;

using sfx::Sprite;
using sfx::Texture;
using sfx::UniqueTexturePtr;
using sfx::TexturePtr;
using sfx::TextureFileVector;

using sfx::UniqueMusicPtr;
using sfx::MusicPtr;
using sfx::UniqueSoundPtr;
using sfx::SoundPtr;
using sfx::SoundFileVector;

class AppState : public agn::GenericState<class GameStateMachine, AppState>
{
public:

	AppState(GameStateMachine&, UniqueStatePtr<AppState>&);
	AppState(GameStateMachine&, UniqueStatePtr<AppState>&, sfx::TexturePtr);
	virtual ~AppState() {}

	// Boolean returns indicate success or failure.
	virtual bool Update(const float _delta_time) = 0;
	virtual void Render(const float _delta_time) = 0;
	virtual void TerminateState() {}

protected:

	TextureFileVector textures_;
	SoundFileVector sounds_;
};

#endif // _APPSTATE_H_