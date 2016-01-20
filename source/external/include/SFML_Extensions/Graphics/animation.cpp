#include "Animation.h"

namespace sfx
{
	Animation::Animation(
		const float _texture_width,
		const float _texture_height,
		const float _frame_duration = 0.0f,
		const int _frame_count = 1,
		const int _current_frame = 1,
		const bool _looping = false) :
		SpriteAnimation(_texture_width, _texture_height, _frame_duration, _frame_count, _current_frame, _looping)
	{
	}

	Animation::~Animation()
	{
	}

	sf::IntRect Animation::GetFrame() const
	{
		sf::IntRect rect;
		DynamicFrame frame = GetDynamicFrame();
		rect.left = frame.u - frame.uv_width / 2;
		rect.width = frame.uv_width;
		rect.top = frame.v - frame.uv_height / 2;
		rect.height = frame.uv_height;

		return rect;
	}
}