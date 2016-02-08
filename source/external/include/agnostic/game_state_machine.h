#ifndef _GAME_STATE_MACHINE_H_
#define _GAME_STATE_MACHINE_H_

#include <string>
#include <iostream>

#include "generic_state.h"

class GameStateMachine
{
public:
	GameStateMachine() {}
	~GameStateMachine() {}

	void Initialize();

private:
	static void UnhandledEvent() { std::cerr << "WARNING: Unhandled FSM event." << std::endl; }

private:
	

	class MenuState : public AppState
	{
		using AppState::AppState;
		void InitializeState();
		void TerminateState();
	};

	class LevelState : public AppState
	{
		using AppState::AppState;
		void InitializeState();
		void TerminateState();
	};
};
#endif // _GAME_STATE_MACHINE_H_