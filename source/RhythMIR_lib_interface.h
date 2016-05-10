#pragma once

#include "beatmap.h"

#include <memory>

struct Function
{
	std::string name;
	int window_size;

	bool operator==(const Function& _other) const
	{
		return(*name == *_other.name && window_size == _other.window_size);
	}
};

template<typename _T>
struct OnsetObject
{
	Function function;
	std::unique_ptr<_T> object;
};

struct TempoEstimate
{
	float BPM;
	float time;
	float confidence;

	operator float() const { return BPM; }

	bool operator<(const float& _other) const
	{
		return (BPM < _other);
	}

	bool operator<(const TempoEstimate& _other) const
	{
		return (BPM < _other.BPM);
	}

	bool operator==(const float& _other) const
	{
		return (BPM == _other);
	}

	bool operator==(const TempoEstimate& _other) const
	{
		return (std::tie(BPM, time) == std::tie(_other.BPM, _other.time));
	}
};