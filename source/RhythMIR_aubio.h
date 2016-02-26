#ifndef _RHYTHMIR_AUBIO_H_
#define _RHYTHMIR_AUBIO_H_
#include <aubio\aubio.h>
#include <memory>
#include <string>

class Aubio
{
public:

	Aubio();
	~Aubio();

	void GenerateBeatmap(const std::string& _filename);

private:

	unsigned int window_size;


	aubio_source_t* source;
	aubio_onset_t* onset;
	aubio_tempo_t* tempo;
};

#endif // _RHYTHMIR_AUBIO_H_