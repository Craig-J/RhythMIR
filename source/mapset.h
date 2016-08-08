#pragma once

#include "beatmap.h"

class MapSet : public BeatmapSet
{
public:

	MapSet(const Song _song);

	std::vector<TimingSection> Sections() const { return sections_; }
	void SetSections(std::vector<TimingSection> _sections) { sections_ = _sections; }

private:

	const Song song_;
	std::vector<TimingSection> sections_;
};


using SongList = std::map<Song, MapSet>;