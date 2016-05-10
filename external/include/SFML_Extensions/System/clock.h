#pragma once

#include <SFML/System/Clock.hpp>

namespace sfx
{
	class Clock
	{
	public:

		Clock();

		void Start();
		void Stop();
		void Restart();
		sf::Time GetTimeElapsed();


	private:
		sf::Clock clock;
		sf::Time total_time;
		bool active_;
	};
}
