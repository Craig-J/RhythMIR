#ifndef _SFX_HSL_COLOR_H_
#define _SFX_HSL_COLOR_H_

#include <SFML/Graphics/Color.hpp>
#include <algorithm>
#include <cmath>

namespace sfx
{
	struct HSL
	{
		double hue_;
		double saturation_;
		double luminance_;

		HSL();
		HSL(int _hue, int _saturation, int _luminance);

		sf::Color HSLToRGB();

	private:

		double HueToRGB(double _arg1, double _arg2, double _hue);

	};

	HSL RGBToHSL(const sf::Color& _color);

}
#endif // _SFX_HSL_COLOR_H_