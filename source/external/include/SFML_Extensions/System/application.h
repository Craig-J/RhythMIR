#ifndef _SFX_APPLICATION_H_
#define _SFX_APPLICATION_H_

#include <SFML\Window.hpp>
#include "frame_clock.h"

namespace sfx
{
	class Application
	{
	public:

		Application();
		virtual ~Application();

		bool Run();

	protected:

		virtual bool Initialize() = 0;
		virtual bool CleanUp() = 0;
		virtual void Update() = 0;
		virtual void Render() = 0;

		sfx::FrameClock clock_;
		sf::Window window_;

	private:
		
		bool running_;
	};
}
#endif