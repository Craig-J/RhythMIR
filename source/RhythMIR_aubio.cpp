#include "RhythMIR_aubio.h"
#include <Agnostic\logger.h>
using agn::Log;


Aubio::Aubio() :
	window_size_(1024 * 2)
{}

Aubio::~Aubio()
{
	del_aubio_source(source);
	del_aubio_onset(onset);
	aubio_cleanup();
}

Beatmap* Aubio::GenerateBeatmap(const Song& _song)
{
	// Load source with aubio
	source = new_aubio_source(const_cast<char*>(_song.file_name_.c_str()), 44100, window_size_ / 4);
	
	if (!source)
	{
		Log::Error("Aubio failed to load source from " + _song.file_name_);
		return nullptr;
	}
	else
	{
		onset = new_aubio_onset("complex", window_size_, window_size_ / 4, 44100);

		Beatmap* beatmap = new Beatmap(_song);

		// Create timing sections
		beatmap->sections_.emplace_back(TimingSection(0, sf::Time::Zero));

		// For each section, do onset detection
		for (auto &section : beatmap->sections_)
		{
			fvec_t* source_buffer = new_fvec(window_size_ / 4);
			fvec_t* onset_buffer = new_fvec(2);
			uint_t frame_count = 0;
			uint_t frames_read = 0;
			do
			{
				// Read from source to source buffer
				aubio_source_do(source, source_buffer, &frames_read);

				// Perform onset detection!
				aubio_onset_do(onset, source_buffer, onset_buffer);

				// Put resulting onsets in this TimingSection notes queue
				if (onset_buffer->data[0] != 0)
				{
					auto last_onset = aubio_onset_get_last_ms(onset);
					section.notes.push(sf::milliseconds(last_onset));
					Log::Message("Onset at " + std::to_string(last_onset) + "ms.");
				}
				frame_count += frames_read;
			} while (frames_read == window_size_ / 4);
		}

		return beatmap;
	}
}
