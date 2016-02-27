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

	unsigned int window_size_;

	aubio_source_t* source;
	aubio_onset_t* onset;
	aubio_tempo_t* tempo;
};

#endif // _RHYTHMIR_AUBIO_H_