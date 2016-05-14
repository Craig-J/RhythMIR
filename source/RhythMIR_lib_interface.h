#pragma once

#include "beatmap.h"

#include <memory>

struct FFTFunction
{
	std::string name;
	int hop_size;
	int window_size;

	bool operator==(const FFTFunction& _other) const
	{
		return(std::tie(name, hop_size, window_size) == std::tie(_other.name, _other.hop_size, _other.window_size));
	}
};

template<typename _T>
struct OnsetObject
{
	FFTFunction function;
	_T* object;
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

class MIRLibrary
{
public:

	virtual void SettingsWindow();
	virtual BeatmapPtr GenerateBeatmap(const Song& _song,
									   std::string _beatmap_name,
									   std::string _beatmap_description = std::string()) = 0;

};