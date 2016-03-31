#ifndef _RHYTHMIR_AUBIO_H_
#define _RHYTHMIR_AUBIO_H_
#include <aubio\aubio.h>
#include "beatmap.h"
#include <atomic>
#include <thread>
#include <unordered_set>
#include <ImGui\imgui.h>
#include <Agnostic\math.h>
#include <RapidXML\rapidxml_utils.hpp>
#include <RapidXML\rapidxml_print.hpp>

class Aubio
{
public:

	struct Function
	{
		char_t* name;
		unsigned int window_size;
		unsigned int hop_size;

		bool operator==(const Function& _other) const
		{
			return(*name == *_other.name && window_size == _other.window_size && hop_size == _other.hop_size);
		}
	};

	struct OnsetObject
	{
		Function& function;
		aubio_onset_t* object;
	};

	struct Settings
	{
		unsigned int samplerate;
		unsigned int hop_size;
		PLAYMODE play_mode_;

		bool train_functions;
		int training_threshold;

		// Tempo
		bool assume_constant_tempo;
		unsigned int candidate_tempo_count;
		float BPM_epsilon;

		// Onset
		unsigned int filter_count;
		unsigned int filterbank_window_size;
		unsigned int onset_function_count;
		std::vector<std::pair<float, float>> filter_ranges;
	};

	struct TempoEstimate
	{
		float BPM;
		float time;
		float confidence;

		// Compare equality using approx with epsilon 0.5f
		// i.e. Considered equal if within += 0.5f
		bool operator==(const TempoEstimate& _other) const
		{
			static agn::math::approx approx(0.5f);
			return(approx(BPM, _other.BPM));
		}
	};

	Aubio();
	~Aubio();

	void UpdateGUI(bool* _opened);
	Beatmap* GenerateBeatmap(const Song& _song);

private:

	Settings settings_;

	Function tempo_function_;
	std::vector<TempoEstimate> beats_;

	std::vector<Function> onset_functions_;
	std::vector<OnsetObject> onset_objects_;

	aubio_source_t* source_;
	aubio_filterbank_t* filterbank_;
	Beatmap* beatmap_;

	bool generating_;
	std::atomic<float> progress_;
	std::thread* aubio_thread_;

	void ThreadFunction();

	void LoadBeatmap();
	void SaveBeatmap();
};
#endif // _RHYTHMIR_AUBIO_H_