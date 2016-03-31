#include "RhythMIR_aubio.h"
#include <Agnostic\logger.h>
using agn::Log;
#include <Agnostic\string.h>

namespace
{
	sf::Time song_duration;

	struct BPMCompare
	{
		constexpr bool operator()(const Aubio::TempoEstimate& a, const Aubio::TempoEstimate& b) const
		{
			return(a.BPM < b.BPM);
		}
	};

	struct TimeCompare
	{
		constexpr bool operator()(const Aubio::TempoEstimate& a, const Aubio::TempoEstimate& b) const
		{
			return(a.time < b.time);
		}
	};
}

Aubio::Aubio() :
	aubio_thread_(nullptr),
	source_(nullptr),
	filterbank_(nullptr),
	progress_(0),
	generating_(false)
{
	settings_.samplerate = 44100;
	settings_.hop_size = 256;
	settings_.play_mode_ = SINGLE;

	settings_.train_functions = true;
	settings_.training_threshold = 350;

	// Tempo
	tempo_function_.name = "default";
	tempo_function_.window_size = 1024;
	tempo_function_.hop_size = 256;
	settings_.assume_constant_tempo = true;
	settings_.candidate_tempo_count = 3;

	// Onset
	settings_.onset_function_count = 1;
	Function default_function;
	default_function.name = "specflux";
	default_function.window_size = 1024;
	default_function.hop_size = 256;
	onset_functions_.push_back(default_function);

	settings_.filter_count = 1;
	settings_.filterbank_window_size = 1024;
	settings_.filter_ranges.reserve(settings_.filter_count);
	onset_objects_.resize(settings_.filter_count, { onset_functions_.front(), nullptr });
}

Aubio::~Aubio()
{
	if(aubio_thread_)
		aubio_thread_->join();
}

void Aubio::UpdateGUI(bool* _opened)
{
	if (!ImGui::Begin("RhythMIR GUI", _opened, ImVec2(400, 200), -1.f, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar))
	{
		// Early out if the window is collapsed, as an optimization.
		ImGui::End();
		return;
	}

	ImGui::ProgressBar(progress_);

	ImGui::End();
}

Beatmap* Aubio::GenerateBeatmap(const Song& _song)
{
	if (!generating_)
	{
		beatmap_ = new Beatmap(_song, settings_.play_mode_);

		beatmap_->LoadMusic();

		aubio_thread_ = new std::thread(&Aubio::ThreadFunction, this);

		return beatmap_;
	}
	return nullptr;
}

void Aubio::ThreadFunction()
{
	generating_ = true;
	bool trained = !settings_.train_functions;

	song_duration = beatmap_->music_->getDuration();

	unsigned int total_samples;
	if (trained)
		total_samples = settings_.samplerate * song_duration.asSeconds();
	else
		total_samples = settings_.samplerate * song_duration.asSeconds() + settings_.hop_size * settings_.training_threshold;

	//sf::Time tempo_frame_duration = sf::seconds(tempo_function_.window_size / total_samples);

	// Load source with aubio
	source_ = new_aubio_source(const_cast<char*>(beatmap_->song_.source_file_name_.c_str()),
							  settings_.samplerate,
							  settings_.hop_size);

	if (!source_)
	{
		Log::Error("Aubio failed to load source from " + beatmap_->song_.source_file_name_);
		beatmap_ = nullptr;
		del_aubio_source(source_);
		source_ = nullptr;
		return;
	}
	else
	{
		// SETUP
		progress_ = 0.0f;

		// Source
		fvec_t* source_buffer = new_fvec(settings_.hop_size);

		// Tempo
		auto tempo_object = new_aubio_tempo(tempo_function_.name,
											tempo_function_.window_size,
											tempo_function_.hop_size,
											settings_.samplerate);
		// Create first timing section
		beatmap_->sections_.emplace_back(TimingSection(0, sf::Time::Zero));

		// Onset
		for (auto &object : onset_objects_)
		{
			object.object = new_aubio_onset(object.function.name,
											object.function.window_size,
											object.function.hop_size,
											settings_.samplerate);
		}

		auto &section = beatmap_->sections_.front();
		section.notes.resize(settings_.filter_count);

		// Filters
		std::vector<fvec_t*> filter_buffers(settings_.filter_count, new_fvec(settings_.hop_size));
		filterbank_ = new_aubio_filterbank(settings_.filter_count, settings_.filterbank_window_size);
		auto filters = settings_.filter_ranges;
		auto initial_filters = aubio_filterbank_get_coeffs(filterbank_);

		// Buffers
		fvec_t* tempo_buffer = new_fvec(2);
		std::vector<fvec_t*> onset_buffers(settings_.filter_count, new_fvec(2));
			
		// DETECTION
		uint_t frame_count = 0; // Total frame count
		uint_t frames_read = 0; // Frames read this loop
		while (frames_read == settings_.hop_size || frames_read == 0)
		{
			// Read from source to source buffer
			aubio_source_do(source_, source_buffer, &frames_read);

			// Do tempo estimation for this hop
			aubio_tempo_do(tempo_object, source_buffer, tempo_buffer);

			if (trained)
			{
				if (tempo_buffer->data[0] != 0)
				{
					float BPM = aubio_tempo_get_bpm(tempo_object);
					float confidence = aubio_tempo_get_confidence(tempo_object);
					float last_beat = aubio_tempo_get_last_s(tempo_object);

					beats_.push_back({ BPM, last_beat, confidence });

					Log::Message("Beat at " + agn::to_string_precise(last_beat, 3) + "s");
					Log::Message("Estimated BPM: " + agn::to_string_precise(BPM, 2) + " Confidence: " + agn::to_string_precise(confidence, 2));
				}
			}


			int index = 0;
			for (auto &object : onset_objects_)
			{
				aubio_onset_do(object.object, source_buffer, onset_buffers[index]);
				
				if (trained)
				{
					if (onset_buffers[index]->data[0] != 0)
					{
						float last_onset = aubio_onset_get_last_s(object.object);
						section.notes[index].push(sf::seconds(last_onset));
						Log::Message("Onset at " + agn::to_string_precise(last_onset, 3) + "s");
					}
				}

				++index;
			}

			// Check if past training threshold
			if (frame_count >= settings_.hop_size * settings_.training_threshold && !trained)
			{
				// Go back to start
				aubio_source_seek(source_, 0);
				frames_read = 0;
				frame_count = 0;
				trained = true;
			}

			// Update count and increment progress
			frame_count += frames_read;
			progress_ = progress_ + ((float)frames_read / (float)total_samples);
			if (progress_ >= 100.0f)
				progress_ = 100.0f;
		}

		// CLEANUP
		for (auto &buffer : onset_buffers)
		{
			if (buffer)
				del_fvec(buffer);
		}
		if (tempo_buffer)
			del_fvec(tempo_buffer);
		for (auto &buffer : filter_buffers)
		{
			if (buffer)
				del_fvec(buffer);
		}
		if (filterbank_)
			del_aubio_filterbank(filterbank_);
		for (auto &object : onset_objects_)
		{
			if (object.object)
				del_aubio_onset(object.object);
		}
		if(tempo_object)
			del_aubio_tempo(tempo_object);
		if (source_buffer)
			del_fvec(source_buffer);
		if (source_)
			del_aubio_source(source_);
		aubio_cleanup();
		generating_ = false;
	}
}
