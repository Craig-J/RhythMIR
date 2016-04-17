#include "RhythMIR_aubio.h"

namespace
{
	sf::Time song_duration;

	namespace SpecDesc
	{
		const auto energy = std::make_pair("Energy", "energy");
		const auto hfc = std::make_pair("High Frequency Content", "hfc");
		const auto complex = std::make_pair("Complex Domain", "complex");
		const auto phase = std::make_pair("Phase Based", "phase");
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
	//phase_vocoder_(nullptr),
	//fft_(nullptr),
	//filterbank_(nullptr),
	beatmap_(nullptr),
	progress_(0),
	generating_(_generating),
	canceling_(_canceling)
{
	settings_.display_window = false;
	settings_.samplerate = 44100;
	settings_.hop_size = 512;
	settings_.play_mode = SINGLE;
	settings_.generate_mode = 0;
	settings_.test_mode = false;
	settings_.train_functions = true;
	settings_.training_threshold = 350;

	// Tempo
	tempo_function_.name = "default";
	tempo_function_.window_size = 1024;
	settings_.assume_constant_tempo = true;
	settings_.store_beats = false;
	settings_.BPM_epsilon = 0.5f;

	// Onset
	settings_.onset_function_count = 1;
	Function default_function;
	default_function.name = "complex";
	default_function.window_size = 1024;
	onset_functions_.push_back(default_function);

	settings_.use_delay = false;
	settings_.delay_threshold = 0;
	settings_.onset_threshold = 0.3f;
	//settings_.silence_threshold = -70.0f;
	settings_.filterbank_type = Settings::FOUR;
	settings_.filter_count = 2;
	settings_.filterbank_window_size = 1024;
	// GUI
	gui_.Reset();
}

Aubio::~Aubio()
{
	canceling_ = true;
}

void Aubio::UpdateGUI()
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
		static int function_type = 0;

		// Beatmap type
		ImGui::Text("Generate Mode");
		ImGui::SameLine();
		ImGui::RadioButton("Single Function", &settings_.generate_mode, 0);
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Use one onset function to generate one vector of onsets.");
		ImGui::SameLine();
		ImGui::RadioButton("Single Function using Filterbank", &settings_.generate_mode, 1);
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Use one onset function on different frequency bands to generate several vectors of onsets.");
		ImGui::SameLine();
		if (ImGui::RadioButton("Run All Functions", &settings_.generate_mode, 2))
			function_type = 0;
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Use every onset function to generate one vector of onsets each.");
		ImGui::Text("Gameplay is only available for the single function mode currently.\nThis is because other modes generate many vectors of notes.");

		ImGui::Checkbox("Test Mode", &settings_.test_mode);
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Don't automatically save the beatmap. Beatmap will be lost when loading/generating another or exiting RhythMIR.");

		ImGui::Spacing();

		// Hop size
		ImGui::Text("Hop Size");
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("The amount (in samples) to move forward between execution of tempo/onset functions.\nLower value generally means more onsets. Must be lower than window size.");
		for (int i = 128; i <= 4096; i *= 2)
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

		ImGui::Checkbox("Train Functions", &settings_.train_functions);
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Runs the functions at the beginning for a few hops to avoid garbage results in the first few seconds. (Recommended)");
		//ImGui::DragInt("Training Threshold", &settings_.training_threshold, 0, 10);

		ImGui::Separator();



		// TEMPO SECTION
		ImGui::TextColored(ImColor(255, 69, 0), "Tempo");

		ImGui::Spacing();

		ImGui::Text("Window Size");
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("The resolution of the fast fourier transform (FFT) algorithm which splits a time domain signal into frequency domain.\nHigher means more detailed spectral content possibly leading to better results.\n However higher also decreases onset timing accuracy.");
		if (tempo_function_.window_size < settings_.hop_size)
			tempo_function_.window_size = settings_.hop_size;
		for (int i = settings_.hop_size; i <= 4096; i *= 2)
		{
			ImGui::SameLine();
			auto label = std::to_string(i) + "##tempo";
			ImGui::RadioButton(label.c_str(), &tempo_function_.window_size, i);
			if (ImGui::IsItemHovered())
			{
				std::string time_resolution("Frequency resolution: " + std::to_string(settings_.samplerate / (2 * i)) + "hz");
				ImGui::SetTooltip(time_resolution.c_str());
			}
		}
		ImGui::Spacing();

		ImGui::Checkbox("Assume Constant Tempo", settings_.assume_constant_tempo);	ImGui::SameLine();
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Automatically enabled for now. In future, variable tempo esimation may be implemented.");
		ImGui::Checkbox("Store Beats", &settings_.store_beats);						ImGui::SameLine();
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Stores the beats from the tempo estimation phase as part of the beatmap.");

		ImGui::Spacing();

		ImGui::Separator();

		// ONSET SECTION
		ImGui::TextColored(ImColor(255, 69, 0), "Onset");
		ImGui::Spacing();

		ImGui::Text("Window Size");
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("The resolution of the fast fourier transform (FFT) algorithm which splits a time domain signal into frequency domain.\nHigher means more detailed spectral content possibly leading to better results.\nHowever higher also decreases onset timing accuracy.");
		auto& onset_function = onset_functions_.front();
		if (onset_function.window_size < settings_.hop_size)
			onset_function.window_size = settings_.hop_size;
		for (int i = settings_.hop_size; i <= 4096; i *= 2)
		{
			ImGui::SameLine();
			auto label = std::to_string(i) + "##onset";
			ImGui::RadioButton(label.c_str(), &onset_function.window_size, i);
			if (ImGui::IsItemHovered())
			{
				std::string time_resolution("Frequency resolution: " + std::to_string(settings_.samplerate / (2 * i)) + "hz");
				ImGui::SetTooltip(time_resolution.c_str());
			}
		}

		ImGui::SliderFloat("Peak-picking Threshold", &settings_.onset_threshold, 0.0f, 1.0f, "%.1f");
		//ImGui::SliderFloat("Silence Threshold", &settings_.silence_threshold, -90.0f, 0.0f, "%.2f");
		if(ImGui::Checkbox("Use Delay Threshold", &settings_.use_delay))
		{
			settings_.delay_threshold = 4.3*settings_.hop_size;
		}
		if (settings_.use_delay)
		{
			ImGui::SameLine();
			ImGui::SliderInt("##delaythreshold", &settings_.delay_threshold, -settings_.hop_size * 5, settings_.hop_size * 5, std::string("%.0f samples (" + std::to_string(settings_.delay_threshold * 1000 / (int)settings_.samplerate) + "ms)").c_str());
		}

		ImGui::Spacing();

		if (settings_.generate_mode != 2)
		{
			ImGui::RadioButton(SpecDesc::energy.first, &function_type, 0); ImGui::SameLine();
			ImGui::RadioButton(SpecDesc::hfc.first, &function_type, 1); ImGui::SameLine();
			ImGui::RadioButton(SpecDesc::phase.first, &function_type, 2); ImGui::SameLine();
			ImGui::RadioButton(SpecDesc::specdiff.first, &function_type, 3);
			ImGui::RadioButton(SpecDesc::complex.first, &function_type, 4); ImGui::SameLine();
			ImGui::RadioButton(SpecDesc::kl.first, &function_type, 5); ImGui::SameLine();
			ImGui::RadioButton(SpecDesc::mkl.first, &function_type, 6); ImGui::SameLine();
			ImGui::RadioButton(SpecDesc::specflux.first, &function_type, 7);
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
			//ImGui::RadioButton("Linear Variable", &filterbank_type, (int)Settings::VARIABLE);
			//ImGui::SameLine();
			ImGui::RadioButton("4-Band", &filterbank_type, (int)Settings::FOUR);
			ImGui::SameLine();
			ImGui::RadioButton("8-Band", &filterbank_type, (int)Settings::EIGHT);
			//ImGui::SameLine();
			//ImGui::RadioButton("15-Band", &filterbank_type, (int)Settings::FIFTEEN);

			//ImGui::RadioButton("31-Band", &filterbank_type, (int)Settings::THIRTYONE);
			//ImGui::SameLine();
			//ImGui::RadioButton("Slaney", &filterbank_type, (int)Settings::SLANEY);
			//ImGui::SameLine();
			//ImGui::RadioButton("Standard Piano", &filterbank_type, (int)Settings::PIANO);
			settings_.filterbank_type = (Settings::FilterbankType)filterbank_type;

			//static bool customize_filters = false;
			//ImGui::Checkbox("Customize Filter Ranges", &customize_filters);
			switch (settings_.filterbank_type)
			{
			/*case Settings::VARIABLE:
				ImGui::SliderInt("##filtercount", &settings_.filter_count, 2, 42, "%.0f filters");
				break;*/
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
			/*case Settings::FIFTEEN:
				settings_.filter_count = 15;
				break;
			case Settings::THIRTYONE:
				settings_.filter_count = 31;
				break;
			case Settings::SLANEY:
				settings_.filter_count = 42;
				break;
			case Settings::PIANO:
				settings_.filter_count = 88;
				break;*/
			}
			//ImGui::Text(std::string("Filter Count: " + to_string(settings_.filter_count)).c_str());
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

	if (!beats_.empty())
	{
		std::lock_guard<std::mutex> lock(gui_.bpm_mutex);
		
		ImGui::PlotLines("##BPMTimeline", &beats_.data()->BPM, beats_.size(), 0,
							"BPM Timeline", gui_.min_BPM, gui_.max_BPM, ImVec2(0, 200), sizeof(TempoEstimate));
				
		auto histogram = agn::SortedVectorToHistogram<float>(gui_.bpms, 200, (float)gui_.min_BPM, (float)gui_.max_BPM);
		auto counts = std::get<0>(histogram);
		auto labels = std::get<1>(histogram);
		auto max_count = std::get<2>(histogram);
		ImGui::PlotHistogram("##BPMHistogram", FloatGetter, FloatGetter,
								counts.data(), labels.data(), counts.size(), 0,
								"BPM Histogram", 0, max_count, ImVec2(0, 200));

		static bool autoselect_tempo = false;
		ImGui::Checkbox("Autoselect Tempo Estimate", &autoselect_tempo);
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Automatically selects the highest confidence BPM.");
		static bool autoselect_offset = true;
		ImGui::Checkbox("Autoselect First Beat Offset", &autoselect_offset);
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Estimates the first beat location by interpolating back in intervals of the closest BPM.");
		
		if (!autoselect_tempo)
		{
			ImGui::SliderFloat("##SliderBPM", &gui_.selected_BPM, gui_.min_BPM, gui_.max_BPM, "Selected BPM = %.3f");
			ImGui::DragFloat("##DragBPM", &gui_.selected_BPM, ((gui_.max_BPM - gui_.min_BPM) / 5000.0f), gui_.min_BPM, gui_.max_BPM, "Drag here for finer control.");
		}
		else
		{
			gui_.selected_BPM = gui_.highest_confidence_BPM.first;
			ImGui::Text(std::string("Current Selected BPM: " + agn::to_string_precise(gui_.highest_confidence_BPM.first, 2)).c_str());
		}

		
		if (autoselect_offset)
		{
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
				SaveBeatmap(*beatmap_); // Saves the beatmap to disk
			}
			beats_.clear();
			gui_.Reset();
			generating_ = false;
			beatmap_ = nullptr; // Don't delete the beatmap we just made. Just reset this objects pointer.
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

	params[2] = settings_.filter_lowpass_frequency;
	left.setParams(params);
	left.process(_source_buffer->length, &source_buffers[0]);

	params[2] = settings_.filter_highpass_frequency;
	right.setParams(params);
	right.process(_source_buffer->length, &source_buffers[settings_.filter_count - 1]);

	for (int mid_index = 0; mid_index < settings_.filter_count - 2; ++mid_index)
	{
		params[2] = settings_.filter_params[mid_index].first;
		params[3] = settings_.filter_params[mid_index].second;
		mid.setParams(params);
		mid.process(_source_buffer->length, &source_buffers[mid_index + 1]);
	}

	/*switch (settings_.filterbank_type)
	{
	case Settings::FOUR:
	{
		

		

		break;
	}
	case Settings::EIGHT:
		Dsp::Params params;

		params[0] = settings_.samplerate;
		params[1] = 40;
		params[2] = 2;
		left.setParams(params);
		left.process(_source_buffer->length, &source_buffers[0]);

		params[1] = 10000;
		right.setParams(params);
		right.process(_source_buffer->length, &source_buffers[7]);


		params[2] = 4;
		params[1] = 120;
		mid.setParams(params);
		mid.process(_source_buffer->length, &source_buffers[1]);
		params[1] = 250;
		mid.setParams(params);
		mid.process(_source_buffer->length, &source_buffers[2]);
		params[1] = 500;
		mid.setParams(params);
		mid.process(_source_buffer->length, &source_buffers[3]);
		params[1] = 1600;
		mid.setParams(params);
		mid.process(_source_buffer->length, &source_buffers[4]);
		params[1] = 3200;
		mid.setParams(params);
		mid.process(_source_buffer->length, &source_buffers[5]);
		params[1] = 6400;
		mid.setParams(params);
		mid.process(_source_buffer->length, &source_buffers[6]);

		break;
	}*/

	for (int i = 0; i < settings_.filter_count; ++i)
	{
		for (int sample = 0; sample < settings_.hop_size; ++sample)
		{
			_filter_buffers[i]->data[sample] = source_buffers[i][sample];
		}
	}
}

/*void Aubio::SetLinearFilters(unsigned int _count)
{
	fvec_t* frequencies = new_fvec(_count + 2);
	float spacing = (settings_.samplerate / 2) / _count;
	const float lowest_frequency = 20.0f;
	for (size_t i = 0; i < _count; i++)
	{
		frequencies->data[i] = lowest_frequency + i * spacing;
	}
	frequencies->data[_count] = (float)settings_.samplerate / 2;
	frequencies->data[_count + 1] = ((float)settings_.samplerate / 2) + 1.0f;
	aubio_filterbank_set_triangle_bands(filterbank_, frequencies, settings_.samplerate);
}

void Aubio::SetPianoFilters()
{
	fvec_t* frequencies = new_fvec(88 + 2);
	float spacing = (settings_.samplerate / 2) / 88;
	for (size_t i = 0; i < 88; i++)
	{
		frequencies->data[i] = std::pow(2, ((float)i - 49.0f) / 12.0f) * 440.f;
	}
	frequencies->data[88] = (float)settings_.samplerate / 2;
	frequencies->data[88 + 1] = ((float)settings_.samplerate / 2) + 1.0f;
	aubio_filterbank_set_triangle_bands(filterbank_, frequencies, settings_.samplerate);
}

void Aubio::SetFourBandFilters()
{
	fvec_t* frequencies = new_fvec(6);
	frequencies->data[0] = 300.0f;
	frequencies->data[1] = 800.0f;
	frequencies->data[2] = 2500.0f;
	frequencies->data[3] = 5000.0f;
	frequencies->data[4] = (float)settings_.samplerate / 2;
	frequencies->data[5] = ((float)settings_.samplerate / 2) + 1.0f;
	aubio_filterbank_set_triangle_bands(filterbank_, frequencies, settings_.samplerate);
}

void Aubio::SetEightBandFilters()
{
	fvec_t* frequencies = new_fvec(10);
	frequencies->data[0] = 40.0f;
	frequencies->data[1] = 160.0f;
	frequencies->data[2] = 300.0f;
	frequencies->data[3] = 800.0f;
	frequencies->data[4] = 2500.0f;
	frequencies->data[5] = 5000.0f;
	frequencies->data[6] = 10000.0f;
	frequencies->data[7] = 20000.0f;
	frequencies->data[8] = (float)settings_.samplerate / 2;
	frequencies->data[9] = ((float)settings_.samplerate / 2) + 1.0f;
	aubio_filterbank_set_triangle_bands(filterbank_, frequencies, settings_.samplerate);
}

void Aubio::SetFifteenBandFilters()
{
	fvec_t* frequencies = new_fvec(17);
	frequencies->data[0] = 25.0f;
	frequencies->data[1] = 40.0f;
	frequencies->data[2] = 63.0f;
	frequencies->data[3] = 100.0f;
	frequencies->data[4] = 160.0f;
	frequencies->data[5] = 250.0f;
	frequencies->data[6] = 400.0f;
	frequencies->data[7] = 630.0f;
	frequencies->data[8] = 1000.0f;
	frequencies->data[9] = 1600.0f;
	frequencies->data[10] = 2500.0f;
	frequencies->data[11] = 4000.0f;
	frequencies->data[12] = 6300.0f;
	frequencies->data[13] = 10000.0f;
	frequencies->data[14] = 16000.0f;
	frequencies->data[15] = (float)settings_.samplerate / 2;
	frequencies->data[16] = ((float)settings_.samplerate / 2) + 1.0f;
	aubio_filterbank_set_triangle_bands(filterbank_, frequencies, settings_.samplerate);
}

void Aubio::SetThirtyOneBandFilters()
{
	fvec_t* frequencies = new_fvec(33);
	frequencies->data[0] = 20.0f;
	frequencies->data[1] = 25.0f;
	frequencies->data[2] = 31.5f;
	frequencies->data[3] = 40.0f;
	frequencies->data[4] = 50.0f;
	frequencies->data[5] = 63.0f;
	frequencies->data[6] = 80.0f;
	frequencies->data[7] = 100.0f;
	frequencies->data[8] = 125.0f;
	frequencies->data[9] = 160.0f;
	frequencies->data[10] = 200.0f;
	frequencies->data[11] = 250.0f;
	frequencies->data[12] = 315.0f;
	frequencies->data[13] = 400.0f;
	frequencies->data[14] = 500.0f;
	frequencies->data[15] = 630.0f;
	frequencies->data[16] = 800.0f;
	frequencies->data[17] = 1000.0f;
	frequencies->data[18] = 1250.0f;
	frequencies->data[19] = 1600.0f;
	frequencies->data[20] = 2000.0f;
	frequencies->data[21] = 2500.0f;
	frequencies->data[22] = 3150.0f;
	frequencies->data[23] = 4000.0f;
	frequencies->data[24] = 5000.0f;
	frequencies->data[25] = 6300.0f;
	frequencies->data[26] = 8000.0f;
	frequencies->data[27] = 10000.0f;
	frequencies->data[28] = 12500.0f;
	frequencies->data[29] = 16000.0f;
	frequencies->data[30] = 20000.0f;
	frequencies->data[31] = (float)settings_.samplerate / 2;
	frequencies->data[32] = ((float)settings_.samplerate / 2) + 1.0f;
	aubio_filterbank_set_triangle_bands(filterbank_, frequencies, settings_.samplerate);
}*/

Beatmap* Aubio::GenerateBeatmap(const Song& _song, std::string _beatmap_name, std::string _beatmap_description)
{
	if (!generating_)
	{
		Beatmap* beatmap = new Beatmap(_song, _beatmap_name, _beatmap_description, (PLAYMODE)settings_.play_mode);

		beatmap->LoadMusic();

		aubio_thread_ = new std::thread(&Aubio::ThreadFunction, this, beatmap);
		beatmap_ = beatmap;
		return beatmap;
	}
	return nullptr;
}

void Aubio::ThreadFunction(Beatmap* _beatmap)
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
				_beatmap->play_mode_ = FOURKEY;
				onset_objects_ = std::vector<OnsetObject>(1, { onset_functions_.front(), nullptr });
				settings_.onset_function_count = 1;
			}
			break;
		case 1:
			{
				if (settings_.filter_count != 4)
					_beatmap->play_mode_ = VISUALIZATION;
				else
					_beatmap->play_mode_ = FOURKEY;
				settings_.onset_function_count = settings_.filter_count;
				onset_objects_ = std::vector<OnsetObject>(settings_.filter_count, { onset_functions_.front(), nullptr });
				settings_.filterbank_window_size = onset_functions_[0].window_size;
				//phase_vocoder_ = new_aubio_pvoc(settings_.filterbank_window_size, settings_.hop_size);
				//fft_ = new_aubio_fft(settings_.filterbank_window_size);
				//filterbank_ = new_aubio_filterbank(settings_.filter_count, settings_.filterbank_window_size);
				/*switch (settings_.filterbank_type)
				{
				case Settings::VARIABLE:
					SetLinearFilters(settings_.filter_count);
					break;
				case Settings::FOUR:
					SetFourBandFilters();
					break;
				case Settings::EIGHT:
					SetEightBandFilters();
					break;
				case Settings::FIFTEEN:
					SetFifteenBandFilters();
					break;
				case Settings::THIRTYONE:
					SetThirtyOneBandFilters();
					break;
				case Settings::SLANEY:
					aubio_filterbank_set_mel_coeffs_slaney(filterbank_, settings_.samplerate);
					break;
				case Settings::PIANO:
					SetPianoFilters();
					break;
				}*/
			}
			break;
		case 2:
			{
				_beatmap->play_mode_ = VISUALIZATION;
				settings_.onset_function_count = 8;
				onset_objects_ = std::vector<OnsetObject>(8, { onset_functions_.front(), nullptr });
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
		auto tempo_object = new_aubio_tempo(tempo_function_.name,
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
			object.object = new_aubio_onset(object.function.name,
											object.function.window_size,
											settings_.hop_size,
											settings_.samplerate);
			aubio_onset_set_delay(object.object, settings_.delay_threshold);
			aubio_onset_set_threshold(object.object, settings_.onset_threshold);
			//aubio_onset_set_silence(object.object, settings_.silence_threshold);
		}

		// Buffers
		fvec_t* source_buffer = new_fvec(settings_.hop_size);
		//cvec_t* spectrum_buffer = new_cvec(settings_.filterbank_window_size);
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

			// Do tempo estimation for this hop
			aubio_tempo_do(tempo_object, source_buffer, tempo_buffer);

			if (trained)
			{
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
				//aubio_pvoc_do(phase_vocoder_, source_buffer, spectrum_buffer);

				for (auto &object : onset_objects_)
				{
					//aubio_filterbank_do(filterbank_, spectrum_buffer, filter_buffers[index]);
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

				/*for (int i = 0; i < settings_.hop_size; ++i)
				{
					auto source = new_fvec(1);
					source->data[0] = source_buffer->data[i];
					aubio_fft_do(fft_, source, spectrum_buffer);
					del_fvec(source);
					aubio_filterbank_do(filterbank_, spectrum_buffer, filter_buffers[i]);
				}
				for (int i = 0; i < settings_.filter_count; ++i)
				{
					auto buffer = new_fvec(settings_.hop_size);
					for (int sample_index = 0; sample_index < settings_.hop_size; ++sample_index)
					{
						buffer->data[sample_index] = filter_buffers[i]->data[sample_index];
					}

					for (auto &object : onset_objects_)
					{
						aubio_onset_do(object.object, buffer, onset_buffers[index]);

						if (trained)
						{
							if (onset_buffers[i]->data[0] != 0)
							{
								float last_onset = aubio_onset_get_last_s(object.object);
								section.notes[i].push(sf::seconds(last_onset));
								Log::Message("Onset at " + agn::to_string_precise(last_onset, 3) + "s");
							}
						}
					}
					del_fvec(buffer);
				}*/
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
		}
		progress_ = progress_ + ((float)frames_read / (float)total_samples);
		if (progress_ >= 0.99f)
			progress_ = 1.0f;

		// CLEANUP
		for (auto &buffer : onset_buffers)
		{
			del_fvec(buffer);
		}
		del_fvec(tempo_buffer);
		if (settings_.generate_mode == 1)
		{
			/*for (auto &buffer : filter_buffers)
			{
				del_fvec(buffer);
			}*/
			//del_aubio_filterbank(filterbank_);
			//del_aubio_pvoc(phase_vocoder_);
			//del_aubio_fft(fft_);
		}
		for (auto &object : onset_objects_)
		{
			del_aubio_onset(object.object);
		}
		del_aubio_tempo(tempo_object);
		//del_cvec(spectrum_buffer);
		del_fvec(source_buffer);
		del_aubio_source(source_);
		aubio_cleanup();

		if (canceling_)
		{
			_beatmap = nullptr;
		}
	}
}

Beatmap* Aubio::LoadBeatmap(const Beatmap& _beatmap)
{
	using namespace rapidxml;
	
 	std::ifstream input_stream(_beatmap.full_file_path() + ".RhythMIR");
	if (input_stream)
	{
		file<> file(input_stream);
		xml_document<> doc;
		doc.parse<parse_no_data_nodes>(file.data());

		xml_node<>* beatmap_node = doc.first_node("beatmap");

		if (beatmap_node != 0)
		{
			Song song = Song(beatmap_node->first_attribute("artist")->value(),
							 beatmap_node->first_attribute("title")->value(),
							 beatmap_node->first_attribute("source")->value());
			PLAYMODE mode;
			switch (std::stoi(beatmap_node->first_attribute("type")->value()))
			{
			case 0:
				mode = VISUALIZATION;
				break;
			case 1:
				mode = SINGLE;
				break;
			case 4:
				mode = FOURKEY;
				break;
			case 88:
				mode = PIANO;
				break;
			default:
				Log::Error("Unknown beatmap type.");
				mode = UNKNOWN;
				break;
			}
			xml_node<>* description_node = beatmap_node->first_node("description");
			std::string description;
			if(description_node)
				description = description_node->value();
			Beatmap* beatmap = new Beatmap(song, _beatmap.name_, description, mode);

			xml_node<>* beats_node = beatmap_node->first_node("beats");
			if (beats_node)
			{
				beatmap->beats_.reset(new std::queue<Note>());

				xml_node<>* beat_node = beats_node->first_node("beat");
				while (beat_node)
				{
					beatmap->beats_->emplace(sf::milliseconds(std::stoi(beat_node->first_attribute("offset")->value())));
					beat_node = beat_node->next_sibling();
				}
			}

			xml_node<>* section_node = beatmap_node->first_node("section");

			while (section_node)
			{
				beatmap->sections_.reset(new std::vector<TimingSection>());
				// Append section
				beatmap->sections_->emplace_back(TimingSection(std::stof(section_node->first_attribute("BPM")->value()),
															   sf::milliseconds(std::stoi(section_node->first_attribute("offset")->value()))));
				// Get this notequeue vector. Will always be the last section.
				auto &notequeue_vector = beatmap->sections_->back().notes;

				// Reserve space in the notequeue vector if we know the mode.
				if (mode != UNKNOWN)
				{
					notequeue_vector.reserve(mode);
				}
				else
				{
					Log::Important("Notequeue vector could not be pre-allocated as beatmap type is unknown.");
				}
					
				xml_node<>* notequeue_node = section_node->first_node("notequeue");
				while (notequeue_node)
				{
					//auto frequency_cutoff = stof(notequeue_node->first_attribute("frequency_cutoff")->value());
					notequeue_vector.emplace_back(); // Construct a notequeue at the back of the vector for this node
					auto &notequeue = notequeue_vector.back();

					xml_node<>* note_node = notequeue_node->first_node("note");
					while (note_node)
					{
						notequeue.emplace(sf::milliseconds(std::stoi(note_node->first_attribute("offset")->value())));
						note_node = note_node->next_sibling("note");
					}

					notequeue_node = notequeue_node->next_sibling("notequeue");
				}

				section_node = section_node->next_sibling("section");
			}
			doc.clear();
			return beatmap;
		}
		else
		{
			Log::Error("No beatmap node found. Format of input file is incorrect.");
			return nullptr;
		}
	}
	else
	{
		Log::Error("Beatmap file could not be opened.");
		return nullptr;
	}
}

void Aubio::SaveBeatmap(const Beatmap& _beatmap)
{
	using namespace rapidxml;

	xml_document<> doc;

	xml_node<>* decl = doc.allocate_node(node_declaration);
	decl->append_attribute(doc.allocate_attribute("version", "1.0"));
	decl->append_attribute(doc.allocate_attribute("encoding", "utf-8"));
	doc.append_node(decl);

	xml_node<>* root_node = doc.allocate_node(node_element, "beatmap");
	root_node->append_attribute(doc.allocate_attribute("artist", _beatmap.song_.artist_.c_str()));
	root_node->append_attribute(doc.allocate_attribute("title", _beatmap.song_.title_.c_str()));
	root_node->append_attribute(doc.allocate_attribute("source", _beatmap.song_.source_file_name_.c_str()));
	auto type = doc.allocate_string(std::to_string(_beatmap.play_mode_).c_str());
	root_node->append_attribute(doc.allocate_attribute("type", type));
	doc.append_node(root_node);

	if (_beatmap.description_ != std::string())
	{
		auto description = doc.allocate_string(_beatmap.description_.c_str());
		xml_node<>* description_node = doc.allocate_node(node_element, "description", description);
		root_node->append_node(description_node);
	}

	if (_beatmap.beats_)
	{
		xml_node<>* beats_node = doc.allocate_node(node_element, "beats");

		auto beatqueue = *_beatmap.beats_;
		while (!beatqueue.empty())
		{
			xml_node<>* beat_node = doc.allocate_node(node_element, "beat");
			auto beat_offset = doc.allocate_string(std::to_string(beatqueue.front().offset.asMilliseconds()).c_str());
			beat_node->append_attribute(doc.allocate_attribute("offset", beat_offset));
			beatqueue.pop();
			beats_node->append_node(beat_node);
		}

		root_node->append_node(beats_node);
	}

	if (_beatmap.sections_)
	{
		for (auto section : *_beatmap.sections_)
		{
			xml_node<>* section_node = doc.allocate_node(node_element, "section");
			auto bpm = doc.allocate_string(std::to_string(section.BPM).c_str());
			auto offset = doc.allocate_string(std::to_string(section.offset.asMilliseconds()).c_str());
			section_node->append_attribute(doc.allocate_attribute("BPM", bpm));
			section_node->append_attribute(doc.allocate_attribute("offset", offset));

			for (auto& notequeue : section.notes)
			{
				xml_node<>* notequeue_node = doc.allocate_node(node_element, "notequeue");

				while (!notequeue.empty())
				{
					xml_node<>* note_node = doc.allocate_node(node_element, "note");
					auto note_offset = doc.allocate_string(std::to_string(notequeue.front().offset.asMilliseconds()).c_str());
					note_node->append_attribute(doc.allocate_attribute("offset", note_offset));
					notequeue.pop();
					notequeue_node->append_node(note_node);
				}

				section_node->append_node(notequeue_node);
			}

			root_node->append_node(section_node);
		}
	}

	std::ofstream output_stream(_beatmap.full_file_path() + ".RhythMIR");

	output_stream << doc;
	output_stream.close();
	doc.clear();
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
