#include "RhythMIR_filesystem.h"

namespace
{
	// Directories
	const std::string song_directory("songs/");
	const std::string skins_directory("skins/");

	// Default filenames and extension
	const std::string default_file_extension(".RhythMIR");
	const std::string songlist_default_filename(song_directory + "_songs" + default_file_extension);
	const std::string beatmaplist_default_filename("_beatmaps" + default_file_extension);
	const std::string settings_default_filename("_settings" + default_file_extension);
}

std::string Filesystem::beatmap_extension = default_file_extension;
std::string Filesystem::current_skin_path = skins_directory + "default/";

const std::string Skin(std::string _filename)
{
	return Filesystem::current_skin_path + _filename;
}

bool LoadSettings(GameSettings& _game_settings)
{
	using namespace rapidxml;

	std::ifstream input_stream(settings_filename);
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

void SaveSettings(GameSettings& _game_settings)
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

	std::ofstream output_stream(settings_filename);

	output_stream << doc;
	output_stream.close();
	doc.clear();
}

void LoadSongList()
{
	using namespace rapidxml;

	std::ifstream input_stream(_file_name);
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
				auto pair = songs_.emplace(Song(artist, title, source_file/*, path_overwrite*/), std::set<Beatmap>());
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
			if (boost::filesystem::exists(song_list_default_filename))
				Log::Error("Song list file exists but could not be opened.");
			else
				Log::Message("No song list file exists. The song list is saved automatically upon exit.");
		}
	}
}

void SaveSongList()
{
	using namespace rapidxml;

	xml_document<> doc;

	xml_node<>* decl = doc.allocate_node(node_declaration);
	decl->append_attribute(doc.allocate_attribute("version", "1.0"));
	decl->append_attribute(doc.allocate_attribute("encoding", "utf-8"));
	doc.append_node(decl);

	xml_node<>* root_node = doc.allocate_node(node_element, "songlist");
	doc.append_node(root_node);

	for (auto& song : songs_)
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

	std::ofstream output_stream(_file_name);
	output_stream << doc;
	output_stream.close();
	doc.clear();
}

void LoadBeatmapList(const Song& _song, bool _force_load)
{
	using namespace rapidxml;

	if (songs_.find(_song)->second.empty() || _force_load) // Only load list if the beatmap list is empty or forced
	{
		std::ifstream input_stream(_song.relative_path() + beatmap_list_default_filename);
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
					auto pair = selected_.song->second.emplace(_song, beatmap_name);
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
			if (!boost::filesystem::exists(boost::filesystem::path(_song.relative_path())))
			{
				Log::Warning("Beatmap list file could not be opened. Directory does not exist.");
				Log::Message("Saving a beatmap automatically updates the beatmap list file.");
			}
			else
			{
				if (boost::filesystem::exists(_song.relative_path() + beatmap_list_default_filename))
					Log::Error("Beatmap list file for " + selected_.song->first.song_name() + " exists but could not be opened.");
				else
				{
					Log::Message("No Beatmap list file exists for " + selected_.song->first.song_name() + ".");
					Log::Message("Creating an empty one now. The beatmap list is updated every time a beatmap is saved or generated.");
					SaveBeatmapList(_song);
				}
			}
		}
	}
}

void SaveBeatmapList(const Song& _song)
{
	using namespace rapidxml;

	xml_document<> doc;

	xml_node<>* decl = doc.allocate_node(node_declaration);
	decl->append_attribute(doc.allocate_attribute("version", "1.0"));
	decl->append_attribute(doc.allocate_attribute("encoding", "utf-8"));
	doc.append_node(decl);

	xml_node<>* root_node = doc.allocate_node(node_element, "beatmaplist");
	doc.append_node(root_node);

	for (auto& beatmap : songs_.find(_song)->second)
	{
		if (beatmap.name_.find("__test__") == std::string::npos) // Skip test beatmaps
		{
			xml_node<>* beatmap_node = doc.allocate_node(node_element, "beatmap");
			auto name = doc.allocate_string(beatmap.name_.c_str());
			beatmap_node->append_attribute(doc.allocate_attribute("name", name));
			root_node->append_node(beatmap_node);
		}
	}

	if (!boost::filesystem::exists(_song.relative_path()))
	{
		Log::Message("Creating directory " + _song.relative_path() + ".");
		boost::filesystem::create_directory(_song.relative_path());
	}

	std::ofstream output_stream(_song.relative_path() + beatmap_list_default_filename);
	output_stream << doc;
	output_stream.close();
	doc.clear();
}