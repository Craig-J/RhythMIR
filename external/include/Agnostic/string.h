#ifndef _AGNOSTIC_STRING_H_
#define _AGNOSTIC_STRING_H_
#include <string>
#include <iomanip>

namespace agn
{
	template<typename T>
	std::string to_string_precise(const T _value, const int _precision = 6)
	{
		std::ostringstream out;
		out.precision(_precision);
		out.setf(std::ios::showpoint);
		out << _value;
		return out.str();
	}
}

#endif