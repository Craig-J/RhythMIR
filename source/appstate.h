#ifndef _APPSTATE_H_
#define _APPSTATE_H_

#include <agnostic\generic_state.h>
using agn::UniqueStatePtr;

#include <SFML_Extensions\global.h>

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