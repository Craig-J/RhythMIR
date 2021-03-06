#include "RhythMIR_aubio.h"

namespace
{
	sf::Time song_duration;

	const float min_BPM = 40.0f;
	const float max_BPM = 240.0f;
	const size_t min_overlap = 2;
	const size_t max_overlap = 8;
	const size_t min_hop = 16;
	const size_t max_hop = 2048;
	const float hist_min_range = 1.0f;

	namespace SpecDesc
	{
		const auto energy = std::make_pair("Time Domain Magnitude", "energy");
		const auto hfc = std::make_pair("High Frequency Content", "hfc");
		const auto complex = std::make_pair("Complex Domain", "complex");
		const auto phase = std::make_pair("Phase Deviation", "phase");
		const auto specdiff = std::make_pair("Spectral Difference", "specdiff");
		const auto kl = std::make_pair("Kullback-Liebler", "kl");
		const auto mkl = std::make_pair("Modified Kullback-Liebler", "mkl");
		const auto specflux = std::make_pair("Spectral Flux", "specflux");
	}

	float FloatGetter(void* values, int index)
	{
		return (float)(((float*)values)[index]);
	}

	const char* LabelStringGetter(void* values, int index)
	{
		return ((const char **)values)[index];
	}

	std::vector<std::string> CovertToStringVector(std::vector<float> _vector)
	{
		std::vector<std::string> strings;
		for (auto t : _vector)
		{
			strings.emplace_back(std::to_string(t));
		}
		return std::move(strings);
	}
}

Aubio::Aubio(std::atomic<bool>& _generating, std::atomic<bool>& _canceling) :
	aubio_thread_(nullptr),
	source_(nullptr),
	beatmap_(nullptr),
	progress_(0),
	generating_(_generating),
	canceling_(_canceling)
{
	settings_.display_window = false;
	settings_.samplerate = 44100;
	settings_.hop_size = 128;
	settings_.play_mode = SINGLE;
	settings_.generate_mode = 0;
	settings_.test_mode = false;
	settings_.train_functions = true;
	settings_.training_threshold = 200;

	// Tempo
	tempo_function_.name = "default";
	tempo_function_.window_size = 512;
	settings_.assume_constant_tempo = true;
	settings_.store_beats = false;
	settings_.subdivide_beat_interval = false;
	settings_.BPM_epsilon = 0.5f;

	// Onset
	settings_.onset_function_count = 1;
	FFTFunction default_function;
	default_function.name = "complex";
	default_function.window_size = 512;
	onset_functions_.push_back(default_function);

	settings_.use_delay = true;
	settings_.delay_threshold = 550;
	settings_.onset_threshold = 0.3f;
	settings_.onset_minioi = 20.0f;
	settings_.silence_threshold = -70.0f;
	settings_.filterbank_type = Settings::FOUR;
	settings_.filter_count = 4;
	settings_.filterbank_window_size = 512;
	// GUI
	gui_.Reset();
}

Aubio::~Aubio()
{
	canceling_ = true;
}

void Aubio::MainWindow()
{
	if (generating_)
	{
		ImGui::ProgressBar(progress_);
		if (progress_ < 1.0f)
		{
			if (ImGui::Button("Cancel"))
			{
				beatmap_ = nullptr;
				canceling_ = true;
				if (aubio_thread_->joinable())
					aubio_thread_->join();
				beats_.clear();
				gui_.Reset();
				generating_ = false;
			}
		}
	}
	else
	{
		ImGui::SameLine();
		if (ImGui::Button("Settings"))
			settings_.display_window = !settings_.display_window;
	}
}

void Aubio::ExtraWindow()
{
	if (!generating_)
		SettingsWindow();
	else
	{
		settings_.display_window = false;
		GeneratingWindow();
	}
}

void Aubio::SettingsWindow()
{
	if (settings_.display_window)
	{
		ImGui::SetNextWindowSize(ImVec2(600, 450), ImGuiSetCond_FirstUseEver);
		if (!ImGui::Begin("Generation Settings"))
		{
			// Early out if the window is collapsed, as an optimization.
			ImGui::End();
			return;
		}

		// OVERALL SECTION

		// Locals that are needed in advance
		static int function_type = 4;

		// Beatmap type
		ImGui::Text("Generate Mode");
		ImGui::SameLine();
		ImGui::RadioButton("Single Function", &settings_.generate_mode, 0);
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Use one onset function to generate one vector of onsets.\nRecommended mode for non-expert users.");
		ImGui::SameLine();
		ImGui::RadioButton("Single Function with Filtering", &settings_.generate_mode, 1);
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Use one onset function on different frequency bands to generate several vectors of onsets.\nGenerates a number of lanes equal to the number of filters used.\nNot recommended as filtering is ineffective, very difficult to get good output.\n\nNOTE: Only Complex and Kullback-Liebler functions are recommended when filtering.");
		ImGui::SameLine();
		if (ImGui::RadioButton("Run All Functions", &settings_.generate_mode, 2))
			function_type = 0;
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Use every onset function to generate one vector of onsets each.\nGenerates an 8-lane Visualization beatmap. Not for playing.");
		//ImGui::Text("Gameplay is only available for the single function mode and 4-band filter mode currently.");

		//ImGui::Checkbox("Test Mode", &settings_.test_mode);
		//if (ImGui::IsItemHovered())
			//ImGui::SetTooltip("Don't automatically save the beatmap. Beatmap will be lost when loading/generating another or exiting RhythMIR.");

		ImGui::Spacing();

		// Hop size
		ImGui::Text("Hop Size");
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("The amount (in samples) to move forward between execution of tempo/onset functions.\nLower value generally means more onsets. Must be lower than window size.\nRecommended 128 for most accurate notes, or 16 for most accurate BPM.");
		for (int i = min_hop; i <= max_hop; i *= 2)
		{
			ImGui::SameLine();
			if (ImGui::RadioButton(std::to_string(i).c_str(), &settings_.hop_size, i))
				if (settings_.use_delay)
					settings_.delay_threshold = i * 4.3;
				else
					settings_.delay_threshold = 0;
			if (ImGui::IsItemHovered())
			{
				std::string time_resolution("Time resolution: " + std::to_string(i * 1000 / settings_.samplerate) + "ms");
				ImGui::SetTooltip(time_resolution.c_str());
			}
		}

		ImGui::Separator();



		// TEMPO SECTION
		ImGui::TextColored(ImColor(255, 69, 0), "Tempo");

		ImGui::Spacing();

		ImGui::Text("Window Size");
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("The resolution of the fast fourier transform (FFT) algorithm which splits a time domain signal into frequency domain.\nHigher means more detailed spectral content possibly leading to better results.\nHowever higher also decreases onset timing accuracy.\nRecommended to be 4x hop size in all cases.");
		if (tempo_function_.window_size < settings_.hop_size * min_overlap)
			tempo_function_.window_size = settings_.hop_size * min_overlap;
		if (tempo_function_.window_size > settings_.hop_size * max_overlap)
			tempo_function_.window_size = settings_.hop_size * max_overlap;
		for (int i = settings_.hop_size * min_overlap; i <= settings_.hop_size * max_overlap; i *= 2)
		{
			ImGui::SameLine();
			auto label = std::to_string(i) + "##tempo";
			ImGui::RadioButton(label.c_str(), &tempo_function_.window_size, i);
			if (ImGui::IsItemHovered())
			{
				std::string time_resolution("Time resolution: " + std::to_string(i * 1000 / settings_.samplerate) + "ms");
				std::string frequency_resolution("Frequency resolution: " + std::to_string(settings_.samplerate / (2 * i)) + "hz");
				std::string tooltip(time_resolution + "\n" + frequency_resolution);
				ImGui::SetTooltip(tooltip.c_str());
			}
		}
		ImGui::Spacing();

		ImGui::Checkbox("Assume Constant Tempo", settings_.assume_constant_tempo);
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Automatically enabled for now. In future, variable tempo esimation may be attempted.");
		ImGui::SameLine();
		ImGui::Checkbox("Store Beats", &settings_.store_beats);
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Stores the beats from the tempo estimation phase as part of the beatmap.\nNot recommended.");

		ImGui::Spacing();

		ImGui::Separator();

		// ONSET SECTION
		ImGui::TextColored(ImColor(255, 69, 0), "Onset");
		ImGui::Spacing();

		ImGui::Text("Window Size");
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("The resolution of the fast fourier transform (FFT) algorithm which splits a time domain signal into frequency domain.\nHigher means more detailed spectral content possibly leading to better results.\nHowever higher also decreases onset timing accuracy.\nRecommended to be 4x hop size in all cases.");
		auto& onset_function = onset_functions_.front();
		if (onset_function.window_size < settings_.hop_size * min_overlap)
			onset_function.window_size = settings_.hop_size * min_overlap;
		if (onset_function.window_size > settings_.hop_size * max_overlap)
			onset_function.window_size = settings_.hop_size * max_overlap;
		for (int i = settings_.hop_size * min_overlap; i <= settings_.hop_size * max_overlap; i *= 2)
		{
			ImGui::SameLine();
			auto label = std::to_string(i) + "##onset";
			ImGui::RadioButton(label.c_str(), &onset_function.window_size, i);
			if (ImGui::IsItemHovered())
			{
				std::string time_resolution("Time resolution: " + std::to_string(i * 1000 / settings_.samplerate) + "ms");
				std::string frequency_resolution("Frequency resolution: " + std::to_string(settings_.samplerate / (2 * i)) + "hz");
				std::string tooltip(time_resolution + "\n" + frequency_resolution);
				ImGui::SetTooltip(tooltip.c_str());
			}
		}

		ImGui::SliderFloat("Peak-picking Threshold", &settings_.onset_threshold, 0.0f, 1.0f, "%.1f");
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("The threshold for picking notes from the onset function output.\nRecommendation is dependent on the function. See function tooltips.");
		ImGui::SliderFloat("Minimum Inter Onset Interval", &settings_.onset_minioi, 10.0f, 1000.0f, "%.1fms", 3.0f);
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("The minimum time between onsets. Put higher to force less notes. Too low may allow double detections.");
		ImGui::SliderFloat("Silence Threshold", &settings_.silence_threshold, -90.0f, -50.0f, "%.2fdB");
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("The relative sound level to determine what is considered silence.\nIncrease if a song is outputting many onsets at very quiet sections.");
		if(ImGui::Checkbox("Use Delay Threshold", &settings_.use_delay))
		{
			settings_.delay_threshold = 4.3*settings_.hop_size;
		}
		if (settings_.use_delay)
		{
			ImGui::SameLine();
			ImGui::SliderInt("##delaythreshold", &settings_.delay_threshold, -settings_.hop_size * 5, settings_.hop_size * 5, std::string("%.0f samples (" + std::to_string(settings_.delay_threshold * 1000 / (int)settings_.samplerate) + "ms)").c_str());
		}

		ImGui::Checkbox("Train Functions", &settings_.train_functions);
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Runs the functions at the beginning for a few hops to avoid detections from oversensitivity in the first few seconds.\nRecommended on.");

		ImGui::Spacing();

		if (settings_.generate_mode != 2)
		{
			ImGui::RadioButton(SpecDesc::energy.first, &function_type, 0);
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Uses energy of sound directly.\nVery bad function, not recommended.");

			ImGui::SameLine();
			ImGui::RadioButton(SpecDesc::hfc.first, &function_type, 1);
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Weights higher frequencies more.\nGood at detecting percussive instruments, e.g. drums.\nRecommended 0.3 peak picking threshold.");

			ImGui::SameLine();
			ImGui::RadioButton(SpecDesc::phase.first, &function_type, 2);
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Uses phase of audio signal.\nBad function overall in this implementation, not recommended.");
			
			ImGui::SameLine();
			ImGui::RadioButton(SpecDesc::specdiff.first, &function_type, 3);
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Compares energy difference between frames.\nPoor function, Spectral Flux is similar but better. Not recommended.");

			ImGui::RadioButton(SpecDesc::complex.first, &function_type, 4);
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Compares phase and magnitude using a distance function between frames.\nEssentially phase combined with spectral difference.\nGood function overall. Recommended in general cases.\nRecommended 0.3 peak picking threshold.");

			ImGui::SameLine();
			ImGui::RadioButton(SpecDesc::kl.first, &function_type, 5);
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Similar to Complex but weights differences in distance logarithmically.\nGood function overall, slightly better than Complex. Recommended in general cases.\nRecommended 0.6 peak picking threshold.");

			ImGui::SameLine();
			ImGui::RadioButton(SpecDesc::mkl.first, &function_type, 6);
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Variation of KL. Recommended in general cases.\nRecommended 0.1 peak picking threshold. This function's behaviour with higher PPT is different from others, >0.2 not recommended.");
			
			ImGui::SameLine();
			ImGui::RadioButton(SpecDesc::specflux.first, &function_type, 7);
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Similar to spectral difference but better overall.\nRecommended 0.6 peak picking threshold.");

			switch (function_type)
			{
			case 0:
				onset_function.name = const_cast<char_t*>(SpecDesc::energy.second);
				break;
			case 1:
				onset_function.name = const_cast<char_t*>(SpecDesc::hfc.second);
				break;
			case 2:
				onset_function.name = const_cast<char_t*>(SpecDesc::phase.second);
				break;
			case 3:
				onset_function.name = const_cast<char_t*>(SpecDesc::specdiff.second);
				break;
			case 4:
				onset_function.name = const_cast<char_t*>(SpecDesc::complex.second);
				break;
			case 5:
				onset_function.name = const_cast<char_t*>(SpecDesc::kl.second);
				break;
			case 6:
				onset_function.name = const_cast<char_t*>(SpecDesc::mkl.second);
				break;
			case 7:
				onset_function.name = const_cast<char_t*>(SpecDesc::specflux.second);
				break;
			}
		}
		else
		{
			ImGui::RadioButton(SpecDesc::energy.first, &function_type, 0); ImGui::SameLine();
			ImGui::RadioButton(SpecDesc::hfc.first, &function_type, 0); ImGui::SameLine();
			ImGui::RadioButton(SpecDesc::phase.first, &function_type, 0); ImGui::SameLine();
			ImGui::RadioButton(SpecDesc::specdiff.first, &function_type, 0);
			ImGui::RadioButton(SpecDesc::complex.first, &function_type, 0); ImGui::SameLine();
			ImGui::RadioButton(SpecDesc::kl.first, &function_type, 0); ImGui::SameLine();
			ImGui::RadioButton(SpecDesc::mkl.first, &function_type, 0); ImGui::SameLine();
			ImGui::RadioButton(SpecDesc::specflux.first, &function_type, 0);
		}

		if (settings_.generate_mode == 1)
		{
			static int filterbank_type = (int)settings_.filterbank_type;
			ImGui::RadioButton("1-Band", &filterbank_type, (int)Settings::SINGLE);
			ImGui::SameLine();
			ImGui::RadioButton("4-Band", &filterbank_type, (int)Settings::FOUR);
			ImGui::SameLine();
			ImGui::RadioButton("8-Band", &filterbank_type, (int)Settings::EIGHT);

			settings_.filterbank_type = (Settings::FilterbankType)filterbank_type;

			//static bool customize_filters = false;
			//ImGui::Checkbox("Customize Filter Ranges", &customize_filters);
			switch (settings_.filterbank_type)
			{
			case Settings::SINGLE:
			{
				settings_.filter_count = 1;
				if (settings_.filter_params.size() != settings_.filter_count)
				{
					settings_.filter_params.clear();
					settings_.filter_params.reserve(settings_.filter_count);
				}
				static float single_filter_frequency = 500;
				static float single_filter_width = 600;
				ImGui::SliderFloat("Filter Centre Frequency", &single_filter_frequency, 0.0f, settings_.samplerate * 0.5f, "%.2fHz");
				ImGui::SliderFloat("Filter Width", &single_filter_width, 0.0f, settings_.samplerate * 0.5f, "%.2fHz");
				settings_.filter_params.push_back(std::make_pair(single_filter_frequency, single_filter_width));
				break;
			}
			case Settings::FOUR:
				settings_.filter_count = 4;
				settings_.filter_lowpass_frequency = 300;
				settings_.filter_highpass_frequency = 5000;
				if (settings_.filter_params.size() != settings_.filter_count - 2)
				{
					settings_.filter_params.clear();
					settings_.filter_params.reserve(settings_.filter_count - 2);
				}
				settings_.filter_params.push_back(std::make_pair(500, 600));
				settings_.filter_params.push_back(std::make_pair(1600, 1600));
				break;
			case Settings::EIGHT:
				settings_.filter_count = 8;
				settings_.filter_lowpass_frequency = 42;
				settings_.filter_highpass_frequency = 10000;
				if (settings_.filter_params.size() != settings_.filter_count - 2)
				{
					settings_.filter_params.clear();
					settings_.filter_params.reserve(settings_.filter_count - 2);
				}
				settings_.filter_params.push_back(std::make_pair(100, 120));
				settings_.filter_params.push_back(std::make_pair(230, 140));
				settings_.filter_params.push_back(std::make_pair(500, 600));
				settings_.filter_params.push_back(std::make_pair(1650, 1700));
				settings_.filter_params.push_back(std::make_pair(3750, 3500));
				settings_.filter_params.push_back(std::make_pair(7500, 5000));
				break;
			}
		}

		ImGui::End();
	}
}

void Aubio::GeneratingWindow()
{
	ImGui::SetNextWindowSize(ImVec2(400, 500), ImGuiSetCond_FirstUseEver);
	if (!ImGui::Begin("Generation In Progress"))
	{
		// Early out if the window is collapsed, as an optimization.
		ImGui::End();
		return;
	}

	ImGui::TextWrapped("This window helps with tempo estimation. When generation is done, press finish generating.");

	if (!beats_.empty())
	{
		std::lock_guard<std::mutex> lock(gui_.bpm_mutex);
		
		ImGui::PlotLines("##BPMTimeline", &beats_.data()->BPM, beats_.size(), 0,
							"BPM Timeline", gui_.min_BPM, gui_.max_BPM, ImVec2(ImGui::GetWindowContentRegionWidth(), 200), sizeof(TempoEstimate));
		

		static float hist_range[2] = { min_BPM, max_BPM };
		if (hist_range[0] > hist_range[1] - hist_min_range)
			hist_range[0] = hist_range[1] - hist_min_range;
		if (hist_range[0] < min_BPM)
		{
			hist_range[0] = min_BPM;
			if (hist_range[1] < min_BPM + hist_min_range)
				hist_range[1] = min_BPM + hist_min_range;
		}

		//auto histogram = agn::SortedVectorToHistogram<float>(gui_.bpms, 200, (float)gui_.min_BPM, (float)gui_.max_BPM);
		auto histogram = agn::SortedVectorToHistogram<float>(gui_.bpms, 200, hist_range[0], hist_range[1]);
		
		auto counts = std::get<0>(histogram);
		auto labels = std::get<1>(histogram);
		auto max_count = std::get<2>(histogram);
		ImGui::PlotHistogram("##BPMHistogram", FloatGetter, FloatGetter,
								counts.data(), labels.data(), counts.size(), 0,
								"BPM Histogram", 0, max_count, ImVec2(ImGui::GetWindowContentRegionWidth(), 200));

		ImGui::PushItemWidth(ImGui::GetWindowContentRegionWidth());
		ImGui::SliderFloat2("##BPMHistogramRange", hist_range, min_BPM, max_BPM, "%.2f");
		ImGui::PopItemWidth();

		static bool autoselect_tempo = true;
		ImGui::Checkbox("Autoselect Tempo Estimate", &autoselect_tempo);
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Automatically selects the highest confidence BPM.\nRecommended for non-expert users.");
		static bool autoselect_offset = true;
		ImGui::Checkbox("Autoselect First Beat Offset", &autoselect_offset);
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Estimates the first beat location by interpolating back in intervals of the closest BPM.\nRecommended for non-expert users.");
		
		if (!autoselect_tempo)
		{
			ImGui::PushItemWidth(ImGui::GetWindowContentRegionWidth());
			ImGui::SliderFloat("##SliderBPM", &gui_.selected_BPM, min_BPM, max_BPM, "Selected BPM = %.3f");
			ImGui::DragFloat("##DragBPM", &gui_.selected_BPM, ((max_BPM - min_BPM) / 5000.0f), min_BPM, max_BPM, "Drag here for finer control.");
			ImGui::PopItemWidth();
			//ImGui::SliderFloat("##SliderBPM", &gui_.selected_BPM, gui_.min_BPM, gui_.max_BPM, "Selected BPM = %.3f");
			//ImGui::DragFloat("##DragBPM", &gui_.selected_BPM, ((gui_.max_BPM - gui_.min_BPM) / 5000.0f), gui_.min_BPM, gui_.max_BPM, "Drag here for finer control.");
		}
		else
		{
			gui_.selected_BPM = gui_.highest_confidence_BPM.first;
			ImGui::Text(std::string("Current Selected BPM: " + agn::to_string_precise(gui_.highest_confidence_BPM.first, 2)).c_str());
		}

		
		if (autoselect_offset)
		{
			//ImGui::Checkbox("Subdivide Beat Interval", &settings_.subdivide_beat_interval);
			//if (ImGui::IsItemHovered())
			//	ImGui::SetTooltip("The beat interval for generating offset values is subdivided by 4 to account for detected beats potentially being half beats or quarter beats.\nWARNING: Can make offset results more random if the detected beats are not aligned with the correct BPM.\nDon't use this unless you understand what this means.");

			auto BPM = gui_.selected_BPM;
			auto closest_bpm_past = std::lower_bound(gui_.bpms.begin(), gui_.bpms.end(), BPM);
			//auto closest_bpm_before = std::lower_bound(gui_.bpms.rbegin(), gui_.bpms.rend(), BPM, [BPM](const float& other)
			//{ return BPM > other; });
			if (closest_bpm_past != gui_.bpms.end())
			{
				auto closest_bpm_before = closest_bpm_past;
				if (closest_bpm_before != gui_.bpms.begin())
					--closest_bpm_before;

				bool first_past = false;
				bool first_before = false;

				// Check if first BPM past is within 0.5f
				if (std::abs(*closest_bpm_past - BPM) < 0.5f) first_past = true;

				// Check if first BPM before is within 0.5f
				if (std::abs(*closest_bpm_before - BPM) < 0.5f) first_before = true;

				if (first_past || first_before)
				{
					float closest_bpm;
					if (first_past && first_before && closest_bpm_before != closest_bpm_past)
					{
						// Both are within 0.5f, find closest
						float past_difference = *closest_bpm_past - BPM;
						float before_difference = *closest_bpm_before - BPM;
						auto smallest_difference = std::min(past_difference, before_difference);
						if (smallest_difference == past_difference)
							closest_bpm = *closest_bpm_past;
						else if (smallest_difference == before_difference)
							closest_bpm = *closest_bpm_before;
						else
							Log::Error("Something crazy happened when comparing past difference and before difference.");
					}
					else if (first_past)
					{
						closest_bpm = *closest_bpm_past;
					}
					else
					{
						closest_bpm = *closest_bpm_before;
					}

					auto ms_interval = (float)(1000 * 60) / closest_bpm;
					if(settings_.subdivide_beat_interval)
						ms_interval /= 4;

					auto closest_note_time = std::find(beats_.begin(), beats_.end(), closest_bpm);
					if (closest_note_time != beats_.end())
					{
						auto closest_note_time_ms = sf::seconds(*closest_note_time).asMilliseconds();
						while (closest_note_time_ms > 0)
						{
							closest_note_time_ms -= ms_interval;
						}
						gui_.selected_offset = closest_note_time_ms + ms_interval;
					}
					ImGui::Text(std::string("Current First Beat Offset: " + agn::to_string_precise(gui_.selected_offset, 2) + "ms").c_str());
				}
				else
				{
					ImGui::Text("No first beat could be interpolated. Select a different bpm or enter manually.");
					ImGui::InputInt("Current First Beat Offset", &gui_.selected_offset);
				}
			}
			else
			{
				ImGui::Text("No first beat could be interpolated. Select a different bpm or enter manually.");
				ImGui::InputInt("Current First Beat Offset", &gui_.selected_offset);
			}
		}
		else
		{
			ImGui::InputInt("Current First Beat Offset", &gui_.selected_offset);
		}
	}
	else
	{
		ImGui::Text("Beats vector is empty. Either no beats were detected or an error occurred.\nTry putting hop size or window size down.");
	}

	if (gui_.selected_BPM == 0.0f)
	{
		ImGui::TextColored(ImColor(255, 69, 0),"BPM has not been selected yet.");
	}
	else
	{
		beatmap_->sections_->front().BPM = gui_.selected_BPM;
		beatmap_->sections_->front().offset = sf::milliseconds(gui_.selected_offset);
	}
	
	if (progress_ == 1.0f)
	{
		if (ImGui::Button("Finish Generating"))
		{
			beatmap_->sections_->front().BPM = gui_.selected_BPM;
			beatmap_->sections_->front().offset = sf::milliseconds(gui_.selected_offset);
			if (!settings_.test_mode)
			{
				Filesystem::SaveBeatmap(*beatmap_); // Saves the beatmap to disk
			}
			beats_.clear();
			gui_.Reset();
			generating_ = false;
			beatmap_.reset(); // Don't delete the beatmap we just made. Just reset this objects pointer.
		}
	}
	ImGui::End();
}

void Aubio::FilterSource(fvec_t * _source_buffer, std::vector<fvec_t*>& _filter_buffers)
{
	std::vector<float*> source_buffers;
	for (int filter_index = 0; filter_index < settings_.filter_count; ++filter_index)
	{
		auto source_buffer = new float[_source_buffer->length];
		for (int sample_index = 0; sample_index < _source_buffer->length; ++sample_index)
		{
			source_buffer[sample_index] = _source_buffer->data[sample_index];
		}
		source_buffers.push_back(source_buffer);
	}
	
	Dsp::FilterDesign<Dsp::Butterworth::Design::LowPass<2>, 1> left;
	Dsp::FilterDesign<Dsp::Butterworth::Design::HighPass<2>, 1> right;
	Dsp::FilterDesign<Dsp::Butterworth::Design::BandPass<2>, 1> mid;
	Dsp::Params params;

	params[0] = settings_.samplerate;
	params[1] = 2;		// order

	if (settings_.filter_count != 1)
	{
		params[2] = settings_.filter_lowpass_frequency;
		left.setParams(params);
		left.process(_source_buffer->length, &source_buffers[0]);

		params[2] = settings_.filter_highpass_frequency;
		right.setParams(params);
		right.process(_source_buffer->length, &source_buffers[settings_.filter_count - 1]);

		for (int mid_index = 0; mid_index < settings_.filter_count - 2; ++mid_index)
		{
			params[2] = settings_.filter_params[mid_index].first;	// frequency centre
			params[3] = settings_.filter_params[mid_index].second;	// band width
			mid.setParams(params);
			mid.process(_source_buffer->length, &source_buffers[mid_index + 1]);
		}
	}
	else
	{
		params[2] = settings_.filter_params[0].first;
		params[3] = settings_.filter_params[0].second;
		mid.setParams(params);
		mid.process(_source_buffer->length, &source_buffers[0]);
	}

	for (int i = 0; i < settings_.filter_count; ++i)
	{
		for (int sample = 0; sample < settings_.hop_size; ++sample)
		{
			_filter_buffers[i]->data[sample] = source_buffers[i][sample];
		}
	}
}

BeatmapPtr Aubio::GenerateBeatmap(const Song& _song, std::string _beatmap_name, std::string _beatmap_description)
{
	if (!generating_)
	{
		auto beatmap = std::make_shared<Beatmap>(_song, _beatmap_name, _beatmap_description, (PLAYMODE)settings_.play_mode);

		beatmap->LoadMusic();

		aubio_thread_ = new std::thread(&Aubio::ThreadFunction, this, beatmap);
		beatmap_ = beatmap;
		return beatmap;
	}
	return nullptr;
}

void Aubio::ThreadFunction(BeatmapPtr _beatmap)
{
	generating_ = true;
	bool trained = !settings_.train_functions;

	auto song_path = _beatmap->song_.full_file_path();

	song_duration = _beatmap->music_->getDuration();

	unsigned int total_samples;
	if (trained)
		total_samples = settings_.samplerate * song_duration.asSeconds();
	else
		total_samples = settings_.samplerate * song_duration.asSeconds() + settings_.hop_size * settings_.training_threshold;

	// Load source with aubio
	source_ = new_aubio_source(const_cast<char*>(song_path.c_str()),
							  settings_.samplerate,
							  settings_.hop_size);

	if (!source_)
	{
		Log::Error("Aubio failed to load source from " + song_path);
		
		del_aubio_source(source_);
		source_ = nullptr;
		generating_ = false;
		_beatmap = nullptr;
	}
	else
	{
		// SETUP
		progress_ = 0.0f;

		// Mode-Specific
		switch (settings_.generate_mode)
		{
		case 0:
			{
				_beatmap->play_mode_ = SINGLE;
				onset_objects_ = std::vector<OnsetObject<aubio_onset_t>>(1, { onset_functions_.front(), nullptr });
				settings_.onset_function_count = 1;
			}
			break;
		case 1:
			{
				if (settings_.filter_count == 4)
					_beatmap->play_mode_ = FOURKEY;
				else if (settings_.filter_count == 1)
					_beatmap->play_mode_ = SINGLE;
				else
					_beatmap->play_mode_ = VISUALIZATION;

				settings_.onset_function_count = settings_.filter_count;
				onset_objects_ = std::vector<OnsetObject<aubio_onset_t>>(settings_.filter_count, { onset_functions_.front(), nullptr });
				settings_.filterbank_window_size = onset_functions_[0].window_size;
			}
			break;
		case 2:
			{
				_beatmap->play_mode_ = VISUALIZATION;
				settings_.onset_function_count = 8;
				onset_objects_ = std::vector<OnsetObject<aubio_onset_t>>(8, { onset_functions_.front(), nullptr });
				onset_objects_[0].function.name = "energy";
				onset_objects_[1].function.name = "hfc";
				onset_objects_[2].function.name = "complex";
				onset_objects_[3].function.name = "phase";
				onset_objects_[4].function.name = "specdiff";
				onset_objects_[5].function.name = "kl";
				onset_objects_[6].function.name = "mkl";
				onset_objects_[7].function.name = "specflux";
			}
			break;
		}

		// Tempo
		auto tempo_object = new_aubio_tempo(const_cast<char_t*>(tempo_function_.name.c_str()),
											tempo_function_.window_size,
											settings_.hop_size,
											settings_.samplerate);

		if (settings_.store_beats)
			_beatmap->beats_.reset(new std::queue<Note>());
		_beatmap->sections_.reset(new std::vector<TimingSection>());

		// Create first timing section
		_beatmap->sections_->emplace_back(TimingSection(0, sf::Time::Zero));

		auto &section = _beatmap->sections_->front();
		section.notes.resize(settings_.onset_function_count);

		// Onset
		for (auto &object : onset_objects_)
		{
			object.object = new_aubio_onset(const_cast<char_t*>(object.function.name.c_str()),
											object.function.window_size,
											settings_.hop_size,
											settings_.samplerate);
			aubio_onset_set_delay(object.object, settings_.delay_threshold);
			aubio_onset_set_threshold(object.object, settings_.onset_threshold);
			aubio_onset_set_silence(object.object, settings_.silence_threshold);
			aubio_onset_set_minioi_ms(object.object, settings_.onset_minioi);
		}

		// Buffers
		fvec_t* source_buffer = new_fvec(settings_.hop_size);
		std::vector<fvec_t*> filter_buffers;
		for (int i = 0; i < settings_.filter_count; ++i)
		{
			auto vec = new_fvec(settings_.hop_size);
			filter_buffers.push_back(vec);
		}
		fvec_t* tempo_buffer = new_fvec(2);
		std::vector<fvec_t*> onset_buffers;
		for (int i = 0; i < settings_.onset_function_count; ++i)
		{
			auto vec = new_fvec(2);
			onset_buffers.push_back(vec);
		}
			
		// DETECTION
		uint_t frame_count = 0; // Total frame count
		uint_t frames_read = 0; // Frames read this loop
		while (!canceling_ && frames_read == settings_.hop_size || frames_read == 0)
		{
			// Read from source to source buffer
			aubio_source_do(source_, source_buffer, &frames_read);

			if (trained)
			{
				// Do tempo estimation for this hop
				aubio_tempo_do(tempo_object, source_buffer, tempo_buffer);

				if (tempo_buffer->data[0] != 0)
				{
					float BPM = aubio_tempo_get_bpm(tempo_object);
					float confidence = aubio_tempo_get_confidence(tempo_object);
					float last_beat = aubio_tempo_get_last_s(tempo_object);

					{
						std::lock_guard<std::mutex> lock(gui_.bpm_mutex);
						gui_.bpms.emplace_back(BPM);
						std::sort(gui_.bpms.begin(), gui_.bpms.end());


						if (confidence > gui_.highest_confidence_BPM.second)
							gui_.highest_confidence_BPM = std::make_pair(BPM, confidence);

						if (BPM > gui_.max_BPM)
							gui_.max_BPM = BPM;
						else if (BPM < gui_.min_BPM)
							gui_.min_BPM = BPM;
						if (settings_.store_beats)
							_beatmap->beats_->emplace(sf::seconds(last_beat));
						beats_.emplace_back(std::move(TempoEstimate{ BPM, last_beat, confidence }));
					}
					Log::Message("Beat at " + agn::to_string_precise(last_beat, 3) + "s BPM : " + agn::to_string_precise(BPM, 2) + " Confidence : " + agn::to_string_precise(confidence, 2));
				}
			}

			int index = 0;
			if (settings_.generate_mode != 1)
			{
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
			}
			else
			{
				
				FilterSource(source_buffer, filter_buffers);

				for (auto &object : onset_objects_)
				{
					aubio_onset_do(object.object, filter_buffers[index], onset_buffers[index]);

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
			}

			// Check if past training threshold
			if (frame_count >= settings_.hop_size * settings_.training_threshold && !trained)
			{
				// Go back to start
				del_aubio_source(source_);
				source_ = new_aubio_source(const_cast<char*>(song_path.c_str()),
										   settings_.samplerate,
										   settings_.hop_size);
				frames_read = 0;
				frame_count = 0;
				trained = true;
			}

			// Update count and increment progress
			frame_count += frames_read;
			progress_ = progress_ + ((float)frames_read / (float)total_samples);
			if (progress_ >= 0.99f)
				progress_ = 1.0f;
			if (progress_ == 1.0f)
				frames_read += 1;
		}
		

		// CLEANUP
		for (auto &buffer : onset_buffers)
		{
			del_fvec(buffer);
		}
		del_fvec(tempo_buffer);
		for (auto &object : onset_objects_)
		{
			del_aubio_onset(object.object);
		}
		del_aubio_tempo(tempo_object);
		for (auto &buffer : filter_buffers)
		{
			del_fvec(buffer);
		}
		del_fvec(source_buffer);
		del_aubio_source(source_);
		aubio_cleanup();

		if (canceling_)
		{
			_beatmap = nullptr;
		}
	}
}

void Aubio::GUI::Reset()
{
	selected_offset = 0;
	selected_BPM = 0.0f;
	max_BPM = 0.0f;
	min_BPM = 9999.0f;
	highest_confidence_BPM = std::make_pair(0.0f, 0.0f);
	bpms.clear();
}
