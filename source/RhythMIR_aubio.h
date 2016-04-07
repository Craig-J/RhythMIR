#ifndef _RHYTHMIR_AUBIO_H_
#define _RHYTHMIR_AUBIO_H_
#include <aubio\aubio.h>
#include "beatmap.h"
#include <atomic>
#include <thread>
#include <mutex>
#include <ImGui\imgui.h>
#include <Agnostic\logger.h>
using agn::Log;
#include <Agnostic\math.h>
#include <Agnostic\string.h>
#include <Agnostic\histogram.h>
#include <RapidXML\rapidxml_utils.hpp>
#include <RapidXML\rapidxml_print.hpp>
#include <DspFilters\Dsp.h>

class Aubio
{
public:

	Aubio(std::atomic<bool>& _generating, std::atomic<bool>& _canceling);
	~Aubio();

	void UpdateGUI();
	void ExtraWindow();
	Beatmap* GenerateBeatmap(const Song& _song, std::string _beatmap_name, std::string _beatmap_description = std::string());
	Beatmap* LoadBeatmap(const Beatmap& _beatmap);
	void SaveBeatmap(const Beatmap& _beatmap);

private:

	struct Function
	{
		char_t* name;
		int window_size;

		bool operator==(const Function& _other) const
		{
			return(*name == *_other.name && window_size == _other.window_size);
		}
	};

	struct OnsetObject
	{
		Function function;
		aubio_onset_t* object;
	};

	struct TempoEstimate
	{
		float BPM;
		float time;
		float confidence;

		operator float() const { return BPM; }

		bool operator<(const float& _other) const
		{
			return (BPM < _other);
		}

		bool operator<(const TempoEstimate& _other) const
		{
			return (BPM < _other.BPM);
		}

		bool operator==(const float& _other) const
		{
			return (BPM == _other);
		}

		bool operator==(const TempoEstimate& _other) const
		{
			return (std::tie(BPM, time) == std::tie(_other.BPM, _other.time));
		}
	};

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
		float BPM_epsilon;

		// Onset
		enum FilterbankType
		{
			//VARIABLE,
			FOUR,
			EIGHT
			//FIFTEEN
			//THIRTYONE,
			//SLANEY,
			//PIANO
		} filterbank_type;
		int filter_count;
		std::vector<std::pair<float, float>> filter_params;
		float filter_lowpass_frequency;
		float filter_highpass_frequency;
		bool use_delay;
		int delay_threshold;
		float onset_threshold;
		//float silence_threshold;
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

	Beatmap* beatmap_;
	Function tempo_function_;
	std::vector<TempoEstimate> beats_;

	std::vector<Function> onset_functions_;
	std::vector<OnsetObject> onset_objects_;

	aubio_source_t* source_;
	//aubio_pvoc_t* phase_vocoder_;
	//aubio_fft_t* fft_;
	//aubio_filterbank_t* filterbank_;

	std::atomic<bool>& generating_;
	std::atomic<bool>& canceling_;
	std::atomic<float> progress_;
	std::thread* aubio_thread_;

	void ThreadFunction(Beatmap*);

	void SettingsWindow();
	void GeneratingWindow();

	void FilterSource(fvec_t* _source_buffer, std::vector<fvec_t*>& _filter_buffers);
	/*void SetLinearFilters(unsigned int);
	void SetPianoFilters();
	void SetFourBandFilters();
	void SetEightBandFilters();
	void SetFifteenBandFilters();
	void SetThirtyOneBandFilters();*/
};
#endif // _RHYTHMIR_AUBIO_H_