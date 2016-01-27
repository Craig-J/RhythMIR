#ifndef _AGNOSTIC_APPSTATE_H_
#define _AGNOSTIC_APPSTATE_H_
#include <agnostic/updateable_templated.h>

namespace agn
{
	class StateManager;
	enum class APPSTATE;

	template<typename UpdateMeasureType>
	class AppState : public Updateable_Templated<UpdateMeasureType>
	{
	public:

		virtual ~AppState()
		{
		}

		virtual void InitializeState() = 0;
		virtual void TerminateState() = 0;
		virtual APPSTATE Update(const UpdateMeasureType& _delta_measure) = 0;
		virtual void Render(const UpdateMeasureType& _delta_measure) = 0;

	protected:

		
		AppState(const StateManager* _manager) :
			manager_(_manager)
		{
		}

		// Not smart pointer because appstates are owned by their manager
		const StateManager* manager_;
	};
}

#endif // _AGNOSTIC_APPSTATE_H_