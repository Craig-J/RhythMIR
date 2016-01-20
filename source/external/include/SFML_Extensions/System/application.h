#ifndef _SFX_APPLICATION_H_
#define _SFX_APPLICATION_H_

#include <cja/updateable.h>
#include "frame_clock.h"

namespace sfx
{
	class Application : public cja::Updateable
	{
	public:

		Application();
		virtual ~Application();

		void Run();

		inline sfx::FrameClock& GetFrameClock()  { return clock_; }

	protected:

		virtual void Render() = 0;

		// Useful elective implementations
		virtual void PreRender() {}
		virtual void PostRender() {}
		virtual bool HandleInput(sf::Time delta_time) { return true; }

	private:
		bool running_;
		sfx::FrameClock clock_;
	};
}
#endif