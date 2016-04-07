#ifndef _AGNOSTIC_MATH_H_
#define _AGNOSTIC_MATH_H_
#include <cstdlib>
#include <cmath>

namespace agn
{
	namespace math
	{
		const double E = 2.71828182845904523536;
		const double LOG2E = 1.44269504088896340736;
		const double LOG10E = 0.434294481903251827651;
		const double LN2 = 0.693147180559945309417;
		const double LN10 = 2.30258509299404568402;
		const double PI = 3.14159265358979323846;
		const double PI_2 = 1.57079632679489661923;
		const double PI_4 = 0.785398163397448309616;
		const double PI_1_PI = 0.318309886183790671538;
		const double PI_2_PI = 0.636619772367581343076;
		const double PI_2_SQRTPI = 1.12837916709551257390;
		const double SQRT2 = 1.41421356237309504880;
		const double SQRT1_2 = 0.707106781186547524401;

		inline float DegToRad(const float _deg)
		{
			return _deg * (PI / 180.0f);
		}
		inline float RadToDeg(float _rad)
		{
			return _rad * (180.0f / PI);
		}

		inline float Lerp(float start, float end, float time)
		{
			return start*(1.0f - time) + time*end;
		}

		template<typename _T>
		struct approx
		{
			_T epsilon;

			approx(const _T& _epsilon) :
				epsilon(_epsilon)
			{}

			bool operator()(const _T& _lhs, const _T& _rhs) const
			{
				return (std::abs(_rhs - _lhs) < epsilon);
			}
		};

		template<typename _T>
		struct approx_less_than
		{
			_T epsilon;

			approx_less_than(const _T& _epsilon) :
				epsilon(_epsilon)
			{}

			bool operator()(const _T& _lhs, const _T& _rhs) const
			{
				//return (_lhs - epsilon < _rhs);
				return (_rhs - _lhs) >((std::abs(_lhs) < std::abs(_rhs) ? std::abs(_rhs) : std::abs(_lhs)) * epsilon);
			}
		};
	}
};

#endif // _AGNOSTIC_MATH_H_