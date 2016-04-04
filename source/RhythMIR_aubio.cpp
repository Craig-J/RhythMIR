#include "RhythMIR_aubio.h"

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

	namespace SpecDesc
	{
		const auto energy = std::make_pair("Energy", "energy");
		const auto hfc = std::make_pair("High Frequency Content (HFC)", "hfc");
		const auto complex = std::make_pair("Complex Domain", "complex");
		const auto phase = std::make_pair("Phase Based", "phase");
		const auto specdiff = std::make_pair("Spectral Difference", "specdiff");
		const auto kl = std::make_pair("Kullback-Liebler", "kl");
		const auto mkl = std::make_pair("Modified Kullback-Liebler", "mkl");
		const auto specflux = std::make_pair("Spectral Flux", "specflux");
	}
}

Aubio::Aubio(std::atomic<bool>& _generating) :
	aubio_thread_(nullptr),
	source_(nullptr),
	filterbank_(nullptr),
	progress_(0),
	generating_(_generating)
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
	default_function.name = "specdiff";
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

void Aubio::UpdateGUI()
{
	ImGui::ProgressBar(progress_);
}

Beatmap* Aubio::GenerateBeatmap(const Song& _song, std::string _beatmap_name, std::string _beatmap_description)
{
	if (!generating_)
	{
		Beatmap* beatmap = new Beatmap(_song, settings_.play_mode_, _beatmap_name, _beatmap_description);

		beatmap->LoadMusic();

		aubio_thread_ = new std::thread(&Aubio::ThreadFunction, this, beatmap);

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

	//sf::Time tempo_frame_duration = sf::seconds(tempo_function_.window_size / total_samples);

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

		// Source
		fvec_t* source_buffer = new_fvec(settings_.hop_size);

		// Tempo
		auto tempo_object = new_aubio_tempo(tempo_function_.name,
											tempo_function_.window_size,
											tempo_function_.hop_size,
											settings_.samplerate);

		_beatmap->sections_ = new std::vector<TimingSection>();

		// Create first timing section
		_beatmap->sections_->emplace_back(TimingSection(0, sf::Time::Zero));

		// Onset
		for (auto &object : onset_objects_)
		{
			object.object = new_aubio_onset(object.function.name,
											object.function.window_size,
											object.function.hop_size,
											settings_.samplerate);
		}

		auto &section = _beatmap->sections_->front();
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

					Log::Message("Beat at " + agn::to_string_precise(last_beat, 3) + "s BPM : " + agn::to_string_precise(BPM, 2) + " Confidence : " + agn::to_string_precise(confidence, 2));
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
			Beatmap* beatmap = new Beatmap(song, mode, _beatmap.name_);

			xml_node<>* section_node = beatmap_node->first_node("section");

			while (section_node)
			{
				beatmap->sections_ = new std::vector<TimingSection>();
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
						notequeue.emplace(Note(sf::milliseconds(std::stoi(note_node->first_attribute("offset")->value()))));
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

	if (_beatmap.sections_)
	{
		for (auto &section : *_beatmap.sections_)
		{
			xml_node<>* section_node = doc.allocate_node(node_element, "section");
			auto bpm = doc.allocate_string(std::to_string(section.BPM).c_str());
			auto offset = doc.allocate_string(std::to_string(section.offset.asMilliseconds()).c_str());
			section_node->append_attribute(doc.allocate_attribute("BPM", bpm));
			section_node->append_attribute(doc.allocate_attribute("offset", offset));

			for (auto &notequeue : section.notes)
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
	/*if (_file_name == std::string())
	{
		std::stringstream file_name;
		switch (_beatmap->play_mode_)
		{
		case SINGLE:
			file_name << "single";
			break;
		case FOURKEY:
			file_name << "fourkey";
			break;
		case PIANO:
			file_name << "piano";
			break;
		default:
			file_name << "unknown";
			break;
		}
		file_name << "_mode.RhythMIR";
		output_stream = std::ofstream(file_name.str());
	}
	else
	{
		output_stream = std::ofstream(_file_name);
	}*/
		
	output_stream << doc;
	output_stream.close();
	doc.clear();
}
