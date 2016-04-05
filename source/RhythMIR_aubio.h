#ifndef _RHYTHMIR_AUBIO_H_
#define _RHYTHMIR_AUBIO_H_
#include <aubio\aubio.h>
#include "beatmap.h"
#include <atomic>
#include <thread>
#include <unordered_set>
#include <ImGui\imgui.h>
#include <Agnostic\logger.h>
using agn::Log;
#include <Agnostic\math.h>
#include <Agnostic\string.h>
#include <RapidXML\rapidxml_utils.hpp>
#include <RapidXML\rapidxml_print.hpp>

class Aubio
{
public:

	Aubio(std::atomic<bool>& _generating);
	~Aubio();

	void UpdateGUI();
	Beatmap* GenerateBeatmap(const Song& _song, PLAYMODE& _play_mode, std::string _beatmap_name, std::string _beatmap_description = std::string());
	Beatmap* LoadBeatmap(const Beatmap& _beatmap);
	void SaveBeatmap(const Beatmap& _beatmap);

private:

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

	struct GUI
	{
		
	} gui_;

	struct Settings
	{
		unsigned int samplerate;
		unsigned int hop_size;

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
	} settings_;

	Function tempo_function_;
	std::vector<TempoEstimate> beats_;

	std::vector<Function> onset_functions_;
	std::vector<OnsetObject> onset_objects_;

	aubio_source_t* source_;
	aubio_filterbank_t* filterbank_;

	std::atomic<bool>& generating_;
	std::atomic<float> progress_;
	std::thread* aubio_thread_;

	void ThreadFunction(Beatmap*);
};
#endif // _RHYTHMIR_AUBIO_H_