#pragma once

#include "RhythMIR_lib_interface.h"


#include <ImGui/imgui.h>

#include <DspFilters/Dsp.h>

#include <aubio/aubio.h>


#include <Agnostic/math.h>
#include <Agnostic/string.h>
#include <Agnostic/histogram.h>
#include <Agnostic/logger.h>
using agn::Log;

#include <atomic>
#include <thread>
#include <mutex>

class Aubio
{
public:

	Aubio(std::atomic<bool>& _generating, std::atomic<bool>& _canceling);
	~Aubio();

	void UpdateGUI();
	void ExtraWindow();
	Beatmap* GenerateBeatmap(const Song& _song, std::string _beatmap_name, std::string _beatmap_description = std::string());
	Beatmap* LoadBeatmap(const Beatmap& _beatmap, bool _partial_load);
	void SaveBeatmap(const Beatmap& _beatmap);

private:

	struct Settings
	{
		bool display_window;
		unsigned int samplerate;
		int hop_size;
		int play_mode;
		int generate_mode;
		bool test_mode;

		bool train_functions;
		int training_threshold;

		// Tempo
		bool assume_constant_tempo;
		bool store_beats;
		bool subdivide_beat_interval;
		float BPM_epsilon;

		// Onset
		enum FilterbankType
		{
			SINGLE,
			FOUR,
			EIGHT
		} filterbank_type;

		int filter_count;
		std::vector<std::pair<float, float>> filter_params;
		float filter_lowpass_frequency;
		float filter_highpass_frequency;
		bool use_delay;
		int delay_threshold;
		float onset_threshold;
		float onset_minioi;
		float silence_threshold;
		unsigned int filterbank_window_size;
		unsigned int onset_function_count;
	} settings_;

	struct GUI
	{
		float selected_BPM;
		int selected_offset;
		std::atomic<float> max_BPM;
		std::atomic<float> min_BPM;
		std::pair<std::atomic<float>, std::atomic<float>> highest_confidence_BPM;
		std::vector<float> bpms;
		std::mutex bpm_mutex;

		void Reset();
	} gui_;

	std::unique_ptr<Beatmap> beatmap_;

	Function tempo_function_;
	std::vector<TempoEstimate> beats_;

	std::vector<Function> onset_functions_;
	std::vector<OnsetObject<aubio_onset_t>> onset_objects_;

	aubio_source_t* source_;

	std::atomic<bool>& generating_;
	std::atomic<bool>& canceling_;
	std::atomic<float> progress_;
	std::thread* aubio_thread_;

	void ThreadFunction(Beatmap*);

	void SettingsWindow();
	void GeneratingWindow();

	void FilterSource(fvec_t* _source_buffer, std::vector<fvec_t*>& _filter_buffers);
};
