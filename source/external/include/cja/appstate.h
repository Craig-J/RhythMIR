#ifndef _CJA_APPSTATE_H_
#define _CJA_APPSTATE_H_
#include <cja/updateable_templated.h>

namespace cja
{

	class StateManager;

	template<typename UpdateMeasureType>
	class AppState : public Updateable_Templated<UpdateMeasureType>
	{
	public:

		virtual ~AppState()
		{
		}

		virtual void InitializeState() = 0;
		virtual void TerminateState() = 0;
		virtual void Update(const UpdateMeasureType& _delta_measure) = 0;
		virtual void Render(const UpdateMeasureType& _delta_measure) = 0;

	protected:

		enum class STATE_PRORITY { LOWEST_PRIORITY, LOW_PRIORITY, NORMAL_PRIORITY, HIGH_PRIORITY, HIGHEST_PRIORITY } state_priority_;

		AppState(const StateManager* _manager, const STATE_PRORITY priority_ = NORMAL_PRIORITY) :
			manager_(_manager),
			state_priority_(priority)
		{
		}

		const StateManager* manager_; // Not smart pointer - appstates are owned by their manager
	};
}

#endif