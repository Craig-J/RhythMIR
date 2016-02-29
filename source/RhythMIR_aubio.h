#ifndef _RHYTHMIR_AUBIO_H_
#define _RHYTHMIR_AUBIO_H_
#include <aubio\aubio.h>
#include "beatmap.h"

class Aubio
{
public:

	Aubio();
	~Aubio();

	class Beatmap* GenerateBeatmap(const Song& _song);

private:

	uint_t window_size_;
	uint_t samplerate_ = 44100;
	uint_t hop_size_ = window_size_ / 4;

	aubio_source_t* source;
	aubio_onset_t* onset;
	aubio_tempo_t* tempo;
};

#endif // _RHYTHMIR_AUBIO_H_