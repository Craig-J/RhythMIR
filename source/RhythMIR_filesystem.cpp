#include "RhythMIR_filesystem.h"

namespace
{
	// Directories
	const std::string song_directory("songs/");
	const std::string skins_directory("skins/");


	const std::map<BeatmapFileType, std::string> file_extensions =
	{
		{ RhythMIR, ".RhythMIR" },
		{ osu, ".osu" },
		{ StepMania, ".sm" }
	};

	const std::string default_file_extension = file_extensions.find(RhythMIR)->second;

	// Default filenames and extension
	const std::string songlist_default_filename(song_directory + "_songs" + default_file_extension);
	const std::string beatmaplist_default_filename("_beatmaps" + default_file_extension);
	const std::string settings_default_filename("_settings" + default_file_extension);
}

BeatmapFileType Filesystem::current_beatmap_type = RhythMIR;
std::string Filesystem::current_skin_path = skins_directory + "default/";
std::string Filesystem::current_song_path = song_directory;

const std::string Skin(std::string _filename)
{
	return Filesystem::current_skin_path + _filename;
}

bool LoadGameSettings(GameSettings& _game_settings)
{
	using namespace rapidxml;

	std::ifstream input_stream(settings_default_filename);
	if (input_stream)
	{
		file<> file(input_stream);
		xml_document<> doc;
		doc.parse<parse_no_data_nodes>(file.data());

		xml_node<>* settings_node = doc.first_node("settings");

		if (settings_node != 0)
		{
			// TO-DO load these settings
			_game_settings.keybinds.emplace_back(sf::Keyboard::Key::Z);
			_game_settings.keybinds.emplace_back(sf::Keyboard::Key::X);
			_game_settings.keybinds.emplace_back(sf::Keyboard::Key::N);
			_game_settings.keybinds.emplace_back(sf::Keyboard::Key::M);
			_game_settings.path_count = 4;


			xml_node<>* offset_node = settings_node->first_node("play_offset");
			if (offset_node)
			{
				_game_settings.play_offset = sf::milliseconds(std::stoi(offset_node->value()));
			}
			else
			{
				_game_settings.play_offset = sf::Time::Zero;
			}

			xml_node<>* approach_time_node = settings_node->first_node("approach_time");
			if (approach_time_node)
			{
				_game_settings.approach_time = sf::milliseconds(std::stoi(approach_time_node->value()));
			}
			else
			{
				_game_settings.approach_time = sf::milliseconds(700);
			}

			xml_node<>* countdown_time_node = settings_node->first_node("countdown_time");
			if (countdown_time_node)
			{
				_game_settings.countdown_time = sf::milliseconds(std::stoi(countdown_time_node->value()));
			}
			else
			{
				_game_settings.countdown_time = sf::seconds(3);
			}

			xml_node<>* auto_play_node = settings_node->first_node("auto_play");
			if (auto_play_node)
			{
				_game_settings.auto_play = std::stoi(auto_play_node->value());
			}
			else
			{
				_game_settings.auto_play = false;
			}

			xml_node<>* shuffle_node = settings_node->first_node("shuffle");
			if (shuffle_node)
			{
				_game_settings.duncan_factor = std::stoi(shuffle_node->value());
			}
			else
			{
				_game_settings.duncan_factor = true;
			}

			xml_node<>* flipped_node = settings_node->first_node("flip");
			if (flipped_node)
			{
				_game_settings.flipped = std::stoi(flipped_node->value());
			}
			else
			{
				_game_settings.flipped = false;
			}

			xml_node<>* music_volume_node = settings_node->first_node("music_volume");
			if (music_volume_node)
			{
				_game_settings.music_volume = std::stof(music_volume_node->value());
			}
			else
			{
				_game_settings.music_volume = 100;
			}


			xml_node<>* sfx_volume_node = settings_node->first_node("sfx_volume");
			if (sfx_volume_node)
			{
				_game_settings.sfx_volume = std::stof(sfx_volume_node->value());
			}
			else
			{
				_game_settings.sfx_volume = 50;
			}


			xml_node<>* hitsound_node = settings_node->first_node("hitsound");
			if (hitsound_node)
			{
				_game_settings.hitsound = std::stoi(hitsound_node->value());
			}
			else
			{
				_game_settings.hitsound = 1;
			}


			xml_node<>* progress_bar_position_node = settings_node->first_node("progress_bar_position");
			if (progress_bar_position_node)
			{
				_game_settings.progress_bar_position = std::stoi(progress_bar_position_node->value());
			}
			else
			{
				_game_settings.progress_bar_position = 2;
			}


			xml_node<>* beat_style_node = settings_node->first_node("beat_style");
			if (beat_style_node)
			{
				_game_settings.beat_style = std::stoi(beat_style_node->value());
			}
			else
			{
				_game_settings.beat_style = 0;
			}

			doc.clear();
			return true;
		}
		else
		{
			Log::Error("No settings node found. Format of input file is invalid.");
			return false;
		}
	}
	else
	{
		Log::Error("Game Settings file could not be opened.");
		return false;
	}
	return false;
}

void SaveGameSettings(GameSettings& _game_settings)
{
	using namespace rapidxml;

	xml_document<> doc;

	xml_node<>* decl = doc.allocate_node(node_declaration);
	decl->append_attribute(doc.allocate_attribute("version", "1.0"));
	decl->append_attribute(doc.allocate_attribute("encoding", "utf-8"));
	doc.append_node(decl);

	xml_node<>* root_node = doc.allocate_node(node_element, "settings");
	doc.append_node(root_node);

	// TO-DO: Save these settings
	/*game_settings_.keybinds.emplace_back(Keyboard::Key::Z);
	game_settings_.keybinds.emplace_back(Keyboard::Key::X);
	game_settings_.keybinds.emplace_back(Keyboard::Key::N);
	game_settings_.keybinds.emplace_back(Keyboard::Key::M);
	game_settings_.path_count = 4;*/

	if (_game_settings.play_offset != sf::Time::Zero)
	{
		auto value = doc.allocate_string(std::to_string(_game_settings.play_offset.asMilliseconds()).c_str());
		xml_node<>* node = doc.allocate_node(node_element, "play_offset", value);
		root_node->append_node(node);
	}

	if (_game_settings.approach_time != sf::milliseconds(700))
	{
		auto value = doc.allocate_string(std::to_string(_game_settings.approach_time.asMilliseconds()).c_str());
		xml_node<>* node = doc.allocate_node(node_element, "approach_time", value);
		root_node->append_node(node);
	}

	if (_game_settings.countdown_time != sf::seconds(3))
	{
		auto value = doc.allocate_string(std::to_string(_game_settings.countdown_time.asMilliseconds()).c_str());
		xml_node<>* node = doc.allocate_node(node_element, "countdown_time", value);
		root_node->append_node(node);
	}

	if (_game_settings.auto_play != false)
	{
		auto value = doc.allocate_string(std::to_string(_game_settings.auto_play).c_str());
		xml_node<>* node = doc.allocate_node(node_element, "auto_play", value);
		root_node->append_node(node);
	}

	if (_game_settings.duncan_factor != true)
	{
		auto value = doc.allocate_string(std::to_string(_game_settings.duncan_factor).c_str());
		xml_node<>* node = doc.allocate_node(node_element, "shuffle", value);
		root_node->append_node(node);
	}

	if (_game_settings.flipped != false)
	{
		auto value = doc.allocate_string(std::to_string(_game_settings.flipped).c_str());
		xml_node<>* node = doc.allocate_node(node_element, "flip", value);
		root_node->append_node(node);
	}

	if (_game_settings.music_volume != 100)
	{
		auto value = doc.allocate_string(std::to_string(_game_settings.music_volume).c_str());
		xml_node<>* node = doc.allocate_node(node_element, "music_volume", value);
		root_node->append_node(node);
	}

	if (_game_settings.sfx_volume != 50)
	{
		auto value = doc.allocate_string(std::to_string(_game_settings.sfx_volume).c_str());
		xml_node<>* node = doc.allocate_node(node_element, "sfx_volume", value);
		root_node->append_node(node);
	}

	if (_game_settings.hitsound != 1)
	{
		auto value = doc.allocate_string(std::to_string(_game_settings.hitsound).c_str());
		xml_node<>* node = doc.allocate_node(node_element, "hitsound", value);
		root_node->append_node(node);
	}

	if (_game_settings.progress_bar_position != 2)
	{
		auto value = doc.allocate_string(std::to_string(_game_settings.progress_bar_position).c_str());
		xml_node<>* node = doc.allocate_node(node_element, "progress_bar_position", value);
		root_node->append_node(node);
	}

	if (_game_settings.beat_style != 0)
	{
		auto value = doc.allocate_string(std::to_string(_game_settings.beat_style).c_str());
		xml_node<>* node = doc.allocate_node(node_element, "beat_style", value);
		root_node->append_node(node);
	}

	std::ofstream output_stream(settings_default_filename);

	output_stream << doc;
	output_stream.close();
	doc.clear();
}

void LoadSongList(SongList& _songlist)
{
	using namespace rapidxml;

	std::ifstream input_stream(songlist_default_filename);
	if (input_stream)
	{
		file<> file(input_stream);
		xml_document<> doc;
		doc.parse<parse_no_data_nodes>(file.data());

		xml_node<>* song_list_node = doc.first_node("songlist");

		if (song_list_node != 0)
		{
			xml_node<>* song_node = song_list_node->first_node("song");

			while (song_node)
			{
				auto artist = song_node->first_attribute("artist")->value();
				auto title = song_node->first_attribute("title")->value();
				auto source_file = song_node->first_attribute("source")->value();
				/*auto beatmap_list_path = song_node->first_attribute("beatmap_list_path")->value();
				std::string path_overwrite;
				if (!beatmap_list_path)
				path_overwrite = std::string();
				else
				path_overwrite = beatmap_list_path;*/
				auto pair = _songlist.emplace(Song(artist, title, source_file/*, path_overwrite*/), std::set<Beatmap>());
				if (pair.second)
				{
					Log::Message("Added " + pair.first->first.song_name() + " to songs.");
				}
				else
				{
					Log::Warning("Failed emplacing " + std::string(artist) + " - " + std::string(title) + " in songs map.");
				}

				song_node = song_node->next_sibling("song");
			}
		}
		else
		{
			Log::Error("No song list node found. Format of input file is invalid.");
		}
	}
	else
	{
		if (!boost::filesystem::exists(boost::filesystem::path(song_directory)))
		{
			Log::Important("Song list file could not be opened. Directory does not exist.");
			Log::Message("Creating directory " + song_directory + ".");
			boost::filesystem::create_directory(song_directory);
		}
		else
		{
			if (boost::filesystem::exists(songlist_default_filename))
				Log::Error("Song list file exists but could not be opened.");
			else
				Log::Message("No song list file exists. The song list is saved automatically upon exit.");
		}
	}
}

void SaveSongList(SongList& _songlist)
{
	using namespace rapidxml;

	xml_document<> doc;

	xml_node<>* decl = doc.allocate_node(node_declaration);
	decl->append_attribute(doc.allocate_attribute("version", "1.0"));
	decl->append_attribute(doc.allocate_attribute("encoding", "utf-8"));
	doc.append_node(decl);

	xml_node<>* root_node = doc.allocate_node(node_element, "songlist");
	doc.append_node(root_node);

	for (auto& song : _songlist)
	{
		if (song.first.source_file_name_ != ".test") // Skip test songs
		{
			xml_node<>* song_node = doc.allocate_node(node_element, "song");
			auto artist = doc.allocate_string(song.first.artist_.c_str());
			song_node->append_attribute(doc.allocate_attribute("artist", artist));
			auto title = doc.allocate_string(song.first.title_.c_str());
			song_node->append_attribute(doc.allocate_attribute("title", title));
			auto source = doc.allocate_string(song.first.source_file_name_.c_str());
			song_node->append_attribute(doc.allocate_attribute("source", source));
			//song_node->append_attribute(doc.allocate_attribute("beatmap_list_path", song.first.relative_path().c_str()));
			root_node->append_node(song_node);
		}
	}

	// Don't need to check for directory here. If there is no song directory the song list load function will have created one.

	std::ofstream output_stream(songlist_default_filename);
	output_stream << doc;
	output_stream.close();
	doc.clear();
}

void LoadBeatmapList(const SongList::iterator& _song)
{
	using namespace rapidxml;

	auto song = _song->first;

	std::ifstream input_stream(song.relative_path() + beatmaplist_default_filename);
	if (input_stream)
	{
		file<> file(input_stream);
		xml_document<> doc;
		doc.parse<parse_no_data_nodes>(file.data());

		xml_node<>* beatmap_list_node = doc.first_node("beatmaplist");

		if (beatmap_list_node != 0)
		{
			xml_node<>* beatmap_node = beatmap_list_node->first_node("beatmap");

			while (beatmap_node)
			{
				auto beatmap_name = beatmap_node->first_attribute("name")->value();
				auto pair = _song->second.emplace(_song->first, beatmap_name);
				if (pair.second)
				{
					Log::Message("Added " + std::string(beatmap_name) + " to the beatmaps set.");
				}
				else
				{
					Log::Warning("Failed emplacing " + std::string(beatmap_name) + " in the beatmaps set.");
				}

				beatmap_node = beatmap_node->next_sibling("beatmap");
			}
		}
		else
		{
			Log::Error("No beatmap list node found. Format of input file is invalid.");
		}
	}
	else
	{
		if (!boost::filesystem::exists(boost::filesystem::path(song.relative_path())))
		{
			Log::Warning("Beatmap list file could not be opened. Directory does not exist.");
			Log::Message("Saving a beatmap automatically updates the beatmap list file.");
		}
		else
		{
			if (boost::filesystem::exists(song.relative_path() + beatmaplist_default_filename))
				Log::Error("Beatmap list file for " + song.song_name() + " exists but could not be opened.");
			else
			{
				Log::Message("No Beatmap list file exists for " + song.song_name() + ".");
				Log::Message("Creating an empty one now. The beatmap list is updated every time a beatmap is saved or generated.");
				SaveBeatmapList(_song);
			}
		}
	}
}

void SaveBeatmapList(const SongList::iterator& _song)
{
	using namespace rapidxml;

	xml_document<> doc;

	xml_node<>* decl = doc.allocate_node(node_declaration);
	decl->append_attribute(doc.allocate_attribute("version", "1.0"));
	decl->append_attribute(doc.allocate_attribute("encoding", "utf-8"));
	doc.append_node(decl);

	xml_node<>* root_node = doc.allocate_node(node_element, "beatmaplist");
	doc.append_node(root_node);

	for (auto& beatmap : _song->second)
	{
		if (beatmap.name_.find("__test__") == std::string::npos) // Skip test beatmaps
		{
			xml_node<>* beatmap_node = doc.allocate_node(node_element, "beatmap");
			auto name = doc.allocate_string(beatmap.name_.c_str());
			beatmap_node->append_attribute(doc.allocate_attribute("name", name));
			root_node->append_node(beatmap_node);
		}
	}

	if (!boost::filesystem::exists(_song->first.relative_path()))
	{
		Log::Message("Creating directory " + _song->first.relative_path() + ".");
		boost::filesystem::create_directory(_song->first.relative_path());
	}

	std::ofstream output_stream(_song->first.relative_path() + beatmaplist_default_filename);
	output_stream << doc;
	output_stream.close();
	doc.clear();
}

BeatmapPtr Filesystem::LoadBeatmap(const Beatmap& _beatmap, bool _partial_load)
{
	switch (Filesystem::current_beatmap_type)
	{
	case osu:
		break;
	case StepMania:
		break;
	case RhythMIR:
	default:
		return LoadRhythMIRBeatmap(_beatmap, _partial_load);
		break;
	}
}

void Filesystem::SaveBeatmap(const Beatmap& _beatmap)
{
	switch (Filesystem::current_beatmap_type)
	{
	
	case osu:
		break;
	case StepMania:
		break;
	case RhythMIR:
	default:
		SaveRhythMIRBeatmap(_beatmap);
		break;
	}
}

BeatmapPtr LoadRhythMIRBeatmap(const Beatmap& _beatmap, bool _partial_load)
{
	using namespace rapidxml;

	std::ifstream input_stream(_beatmap.full_file_path() + file_extensions.find(RhythMIR)->second);
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
			if (description_node)
				description = description_node->value();
			BeatmapPtr beatmap = std::make_shared<Beatmap>(song, _beatmap.name_, description, mode);

			if (!_partial_load)
			{
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

void SaveRhythMIRBeatmap(const Beatmap& _beatmap)
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

	std::ofstream output_stream(_beatmap.full_file_path() + file_extensions.find(RhythMIR)->second);

	output_stream << doc;
	output_stream.close();
	doc.clear();
}