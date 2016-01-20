#ifndef _SFX_ANIMATION_H_
#define _SFX_ANIMATION_H_
#include <cja\sprite_animation.h>
#include <SFML\Graphics\Rect.hpp>

namespace sfx
{
	class Animation : public cja::SpriteAnimation
	{
	public:

		// Default arguments - must be given a texture to be valid
		Animation(const float texture_width,
				const float texture_height,
				const float frame_duration = 0.0f,
				const int frame_count = 1,
				const int current_frame = 1,
				const bool looping = false);
		~Animation();

		sf::IntRect GetFrame() const;
		
	};
}

#endif