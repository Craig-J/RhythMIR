#include <SFML_Extensions/Graphics/hsl.h>

namespace
{
	const double D_EPSILON = 0.00000000000001;
}

namespace sfx
{
	HSL::HSL() :
		hue_(0),
		saturation_(0),
		luminance_(0)
	{}

	HSL::HSL(int _hue, int _saturation, int _luminance)
	{
		// Range control for Hue.
		if (_hue <= 360 && _hue >= 0) { hue_ = _hue; }
		else
		{
			if (_hue > 360) { hue_ = _hue % 360; }
			else if (_hue < 0 && _hue > -360) { hue_ = -_hue; }
			else if (_hue < 0 && _hue < -360) { hue_ = -(_hue % 360); }
		}

		// Range control for Saturation.
		if (_saturation <= 100 && _saturation >= 0) { saturation_ = _saturation; }
		else
		{
			if (_saturation > 100) { saturation_ = _saturation % 100; }
			else if (_saturation < 0 && _saturation > -100) { saturation_ = -_saturation; }
			else if (_saturation < 0 && _saturation < -100) { saturation_ = -(_saturation % 100); }
		}

		// Range control for Luminance
		if (_luminance <= 100 && _luminance >= 0) { luminance_ = _luminance; }
		else
		{
			if (_luminance > 100) { luminance_ = _luminance % 100; }
			if (_luminance < 0 && _luminance > -100) { luminance_ = -_luminance; }
			if (_luminance < 0 && _luminance < -100) { luminance_ = -(_luminance % 100); }
		}

	}

	double HSL::HueToRGB(double arg1, double arg2, double _hue)
	{
		if (_hue < 0) _hue += 1;
		if (_hue > 1) _hue -= 1;
		if ((6 * _hue) < 1) { return (arg1 + (arg2 - arg1) * 6 * _hue); }
		if ((2 * _hue) < 1) { return arg2; }
		if ((3 * _hue) < 2) { return (arg1 + (arg2 - arg1) * ((2.0 / 3.0) - _hue) * 6); }
		return arg1;
	}

	sf::Color HSL::HSLToRGB()
	{
		// Reconvert to range [0,1]
		double hue = hue_ / 360.0;
		double saturation = saturation_ / 100.0;
		double luminance = luminance_ / 100.0;

		double arg1, arg2;

		if (saturation <= D_EPSILON)
		{
			sf::Color C(static_cast<int>(luminance) * 255, static_cast<int>(luminance) * 255, static_cast<int>(luminance) * 255);
			return C;
		}
		else
		{
			if (luminance < 0.5) { arg2 = luminance * (1 + saturation); }
			else { arg2 = (luminance + saturation) - (saturation * luminance); }
			arg1 = 2 * luminance - arg2;

			sf::Uint8 r = static_cast<sf::Uint8>((255 * HueToRGB(arg1, arg2, (hue + 1.0 / 3.0))));
			sf::Uint8 g = static_cast<sf::Uint8>((255 * HueToRGB(arg1, arg2, hue)));
			sf::Uint8 b = static_cast<sf::Uint8>((255 * HueToRGB(arg1, arg2, (hue - 1.0 / 3.0))));
			sf::Color C(r, g, b);
			return C;
		}

	}

	HSL RGBToHSL(const sf::Color& C)
	{
		// Trivial cases.
		if (C == sf::Color::White)
		{
			return HSL(0, 0, 100);
		}

		if (C == sf::Color::Black)
		{
			return HSL(0, 0, 0);
		}

		if (C == sf::Color::Red)
		{
			return HSL(0, 100, 50);
		}

		if (C == sf::Color::Yellow)
		{
			return HSL(60, 100, 50);
		}

		if (C == sf::Color::Green)
		{
			return HSL(120, 100, 50);
		}

		if (C == sf::Color::Cyan)
		{
			return HSL(180, 100, 50);
		}

		if (C == sf::Color::Blue)
		{
			return HSL(240, 100, 50);
		}

		if (C == sf::Color::Cyan)
		{
			return HSL(300, 100, 50);
		}

		double R, G, B;
		R = C.r / 255.0;
		G = C.g / 255.0;
		B = C.b / 255.0;
		// Casos no triviales.
		double max, min, l, s;

		// Maximos
		max = std::max(std::max(R, G), B);

		// Minimos
		min = std::min(std::min(R, G), B);


		HSL A;
		l = ((max + min) / 2.0);

		if (max - min <= D_EPSILON)
		{
			A.hue_ = 0;
			A.saturation_ = 0;
		}
		else
		{
			double diff = max - min;

			if (A.luminance_ < 0.5)
			{
				s = diff / (max + min);
			}
			else
			{
				s = diff / (2 - max - min);
			}

			double diffR = (((max - R) * 60) + (diff / 2.0)) / diff;
			double diffG = (((max - G) * 60) + (diff / 2.0)) / diff;
			double diffB = (((max - B) * 60) + (diff / 2.0)) / diff;


			if (max - R <= D_EPSILON) { A.hue_ = diffB - diffG; }
			else if (max - G <= D_EPSILON) { A.hue_ = (1 * 360) / 3.0 + (diffR - diffB); }
			else if (max - B <= D_EPSILON) { A.hue_ = (2 * 360) / 3.0 + (diffG - diffR); }

			if (A.hue_ <= 0 || A.hue_ >= 360) { fmod(A.hue_, 360); }

			s *= 100;
		}

		l *= 100;
		A.saturation_ = s;
		A.luminance_ = l;
		return A;
	}
}