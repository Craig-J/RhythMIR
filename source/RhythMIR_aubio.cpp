#include "RhythMIR_aubio.h"

Aubio::Aubio() :
	onset(new_aubio_onset("default", 1024, 256, 44100))
{}

Aubio::~Aubio()
{
	del_aubio_onset(onset);
	aubio_cleanup();
}

void Aubio::GenerateBeatmap(const std::string& _filename)
{
	source = new_aubio_source(const_cast<char*>(_filename.c_str()), 44100, window_size / 4);
}
