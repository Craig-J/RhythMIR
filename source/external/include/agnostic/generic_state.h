#ifndef _AGNOSTIC_GENERIC_STATE_H_
#define _AGNOSTIC_GENERIC_STATE_H_

#include <memory>

namespace agn
{
	template <class _State>
	using StatePtr = std::unique_ptr<_State>;

	template <typename StateMachine, class State>
	class GenericState
	{
	public:
		explicit GenericState(StateMachine &_state_machine, StatePtr<State> &_state) :
			state_machine_(_state_machine), state_(_state)
		{}

		template <class ConcreteState>
		static void Initialize(StateMachine &_state_machine, StatePtr<State> &_state)
		{
			state_ = StatePtr<State>(new ConcreteState(_state_machine, _state));
			state_->InitializeState();
		}

	protected:
		// ChangeState
		// Calls the static initialize for a new state.
		template <class ConcreteState>
		void ChangeState()
		{
			// Terminate current state.
			TerminateState();
			// Initialize new one.
			Initialize<ConcreteState>(_state_machine, _state);
		}

		// RenewState
		// Terminate and re-Initialize the current state.
		void RenewState()
		{
			TerminateState();
			InitializeState();
		}

	private:
		virtual void InitializeState() {}
		virtual void TerminateState() {}

	protected:
		StateMachine &state_machine_;

	private:
		// The pointer to the current state.
		StatePtr<State> &state_;
	};
}

#endif // _AGNOSTIC_GENERIC_STATE_H_