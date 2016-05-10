#pragma once

#include <SFML/Graphics/Rect.hpp>

#include <Agnostic/sprite_animation.h>

namespace sfx
{
	class Animation : public agn::SpriteAnimation
	{
	public:

		// Default arguments - must be given a texture to be valid
		using agn::SpriteAnimation::SpriteAnimation;
		virtual ~Animation() {}

		sf::IntRect GetFrame() const;
		
	};
}
