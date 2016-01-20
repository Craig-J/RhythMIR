#include "application.h"

namespace sfx
{
	Application::Application() :
		clock_()
	{

	}

	Application::~Application()
	{
	}

	void Application::Run()
	{
		while (running_)
		{
			clock_.beginFrame();
			sf::Time delta_time = clock_.getLastFrameTime();
			// HANDLE INPUT
			running_ = HandleInput(delta_time);
			if (running_)
			{
				// UPDATE
				Update(delta_time.asSeconds());

				// RENDER
				PreRender();
				Render();
				PostRender();
			}
			clock_.endFrame();
		}
	}
}