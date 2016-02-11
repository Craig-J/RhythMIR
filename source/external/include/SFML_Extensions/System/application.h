#ifndef _SFX_APPLICATION_H_
#define _SFX_APPLICATION_H_

#include <SFML\Graphics.hpp>
#include "frame_clock.h"

namespace sfx
{
	class Application
	{
	public:

		Application(sf::VideoMode _video_mode, std::string _application_title);
		virtual ~Application() {}

		void Run();

	protected:

		sfx::FrameClock clock_;
		sf::RenderWindow window_;

	private:

		virtual bool Initialize() = 0;
		virtual void CleanUp() = 0;
		virtual bool Update() = 0;
		virtual void Render() = 0;
		// Default event handling does nothing.
		virtual void ProcessEvent(sf::Event& _event) {}

		void EventLoop();
		
		bool running_;
	};
}
#endif // _SFX_APPLICATION_H_