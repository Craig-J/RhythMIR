#ifndef _APPSTATE_H_
#define _APPSTATE_H_
#include <agnostic\generic_state.h>

class AppState : public agn::GenericState<class GameStateMachine, AppState>
{
public:

	using GenericState::GenericState;
	virtual ~AppState() {}

	virtual void Update(const float _delta_time) = 0;
	virtual void Render(const float _delta_time) = 0;
	virtual void CleanUp() {};
};

#endif // _APPSTATE_H_