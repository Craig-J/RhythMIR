#include "RhythMIR_aubio.h"
#include <Agnostic\logger.h>
using agn::Log;
#include <Agnostic\string.h>


Aubio::Aubio() :
	samplerate_(44100),
	window_size_(1024 * 2),
	hop_size_(window_size_ / 4)
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
	source = new_aubio_source(const_cast<char*>(_song.file_name_.c_str()), samplerate_, hop_size_);
	
	if (!source)
	{
		Log::Error("Aubio failed to load source from " + _song.file_name_);
		return nullptr;
	}
	else
	{
		
		onset = new_aubio_onset("complex", window_size_, hop_size_, samplerate_);
		tempo = new_aubio_tempo("default", window_size_, hop_size_, samplerate_);

		Beatmap* beatmap = new Beatmap(_song);

		// Create timing sections
		beatmap->sections_.emplace_back(TimingSection(0, sf::Time::Zero));

		// For each section, do onset detection
		for (auto &section : beatmap->sections_)
		{
			fvec_t* source_buffer = new_fvec(hop_size_);
			fvec_t* tempo_buffer = new_fvec(2);
			fvec_t* onset_buffer = new_fvec(2);
			uint_t frame_count = 0;
			uint_t frames_read = 0;
			float highest_confidence = 0;
			do
			{
				// Read from source to source buffer
				aubio_source_do(source, source_buffer, &frames_read);

				// Do beat tracking for this hop
				aubio_tempo_do(tempo, source_buffer, tempo_buffer);

				if (tempo_buffer->data[0] != 0)
				{
					float BPM = aubio_tempo_get_bpm(tempo);
					float confidence = aubio_tempo_get_confidence(tempo);
					float last_beat = aubio_tempo_get_last_s(tempo);

					if (highest_confidence < confidence)
					{
						section.BPM = BPM;
					}

					Log::Message("Beat at " + agn::to_string_precise(last_beat, 3) + "s");
					Log::Message("Estimated BPM: " + agn::to_string_precise(BPM, 2) + " Confidence: " + agn::to_string_precise(confidence, 2));
				}

				// Do onset detection for this hop
				aubio_onset_do(onset, source_buffer, onset_buffer);

				// Put resulting onsets in this TimingSection notes queue
				if (onset_buffer->data[0] != 0)
				{
					float last_onset = aubio_onset_get_last_s(onset);
					section.notes.push(sf::seconds(last_onset));
					Log::Message("Onset at " + agn::to_string_precise(last_onset, 3) + "s");
				}
				frame_count += frames_read;
			} while (frames_read == hop_size_);
		}

		return beatmap;
	}
}
