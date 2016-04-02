#include "menu_state.h"
#include "game_state_machine.h"
#include "game_state.h"

namespace
{
	// Implementation details here

	const std::vector<std::string> headings_ = { "Songs", "Beatmaps", "Actions" };

	float context_horizontal_spacing;
	float context_vertical_cuttoff;
	float songs_x;
	float song_vertical_spacing;
	float song_vertical_offset;
	float beatmaps_x;
	float beatmaps_vertical_spacing;
	float beatmaps_vertical_offset;
	float actions_x;
	float action_vertical_spacing;
	float action_vertical_offset;
	sf::Vector2f window_centre;

	TexturePtr play_texture_;
	TexturePtr selector_texture_;
	TexturePtr generate_texture_;

	bool* show_GUI;

	const std::string song_directory("songs/");
	const std::string song_list_extension(".RhythMIR");
	const std::string song_list_default_filename(song_directory + "songs" + song_list_extension);
	const std::string beatmap_list_extension(".RhythMIR");
	const std::string beatmap_list_default_filename("_beatmaps" + beatmap_list_extension);
	const std::string beatmap_extension(".RhythMIR");

	static const char alphanum[] =
		"0123456789"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz";

	std::string RandomString(unsigned int _random_chars_to_append)
	{
		std::string string;
		string += "__test__";
		for (int i = 0; i < _random_chars_to_append; ++i)
		{
			string += alphanum[rand() % (sizeof(alphanum) - 1)];
		}
		return string;
	}
}

void MenuState::InitializeState()
{
	textures_ = TextureFileVector
	{
		{ machine_.background_texture_, "menu_background.jpg" },
		{ play_texture_, "play_button.png" },
		{ generate_texture_, "generate_button.png" },
		{ selector_texture_, "circle_red.png" }
	};
	Global::TextureManager.Load(textures_);

	machine_.background_.setTexture(*machine_.background_texture_);

	// Calculate various layout stuff
	window_centre = sf::Vector2f(machine_.window_.getSize().x * 0.5, machine_.window_.getSize().y * 0.5);
	context_horizontal_spacing = machine_.window_.getSize().x * 0.2f;
	context_vertical_cuttoff = machine_.window_.getSize().y * 0.2f;

	songs_x = window_centre.x;
	song_vertical_spacing = 50.0f;
	song_vertical_offset = 0.0f;

	beatmaps_x = songs_x + context_horizontal_spacing;
	beatmaps_vertical_spacing = 50.0f;
	beatmaps_vertical_offset = 0.0f;

	actions_x = beatmaps_x + context_horizontal_spacing;
	action_vertical_spacing = 150.0f;
	action_vertical_offset = 0.0f;

	selector_ = sfx::Sprite(sf::Vector2f(songs_x, window_centre.y),
							selector_texture_);

	play_button_ = sfx::Sprite(sf::Vector2f(actions_x, window_centre.y),
							   play_texture_);

	generate_button_ = sfx::Sprite(sf::Vector2f(actions_x, play_button_.getPosition().y + action_vertical_spacing),
								   generate_texture_);

	heading_.setFont(machine_.font_);
	heading_.setCharacterSize(60);
	heading_.setColor(sf::Color::Color(255, 69, 0));
	heading_.setPosition(sf::Vector2f(context_horizontal_spacing, machine_.window_.getSize().y * 0.1f));

	song_text_.setFont(machine_.font_);
	song_text_.setCharacterSize(30);
	song_text_.setColor(sf::Color::Color(192, 192, 192));
	song_text_.setPosition(sf::Vector2f(songs_x, window_centre.y));
	

	beatmap_text_.setFont(machine_.font_);
	beatmap_text_.setCharacterSize(30);
	beatmap_text_.setColor(sf::Color::Color(192, 192, 192));
	beatmap_text_.setPosition(sf::Vector2f(beatmaps_x, window_centre.y));

	selected_.context = SONGS;

	LoadSongList(song_list_default_filename);
	songs_.emplace(Song{ "guitar_vs_piano.wav", "Goukisan", "Guitar Vs. Piano 1.2" }, std::set<Beatmap>());
	selected_.song = songs_.begin();

	if (!songs_.empty())
	{
		GetSongBeatmaps();
	}

	selected_.action = PLAY;

	beatmap_ = nullptr;
	show_GUI = new bool(true);
}

void MenuState::TerminateState()
{
	Global::TextureManager.Unload(textures_);
	textures_.clear();

	delete show_GUI;

	for (auto &song : songs_)
	{
		SaveBeatmapList(song.first);
	}

	SaveSongList(song_list_default_filename);
}

bool MenuState::Update(const float _delta_time)
{
	GUI();
	if (Global::Input.KeyPressed(Keyboard::Num1))
	{
		GenerateTestSongs();
	}

	if (Global::Input.KeyPressed(Keyboard::Num2))
	{
		GenerateTestBeatmaps();
	}
	switch (selected_.context)
	{
		case SONGS:
			if (Global::Input.KeyPressed(Keyboard::D) || Global::Input.KeyPressed(Keyboard::Right))
			{
				selected_.context = BEATMAPS;
				selector_.move(context_horizontal_spacing, 0.0f);
				break;
			}

			if (!songs_.empty()) // Only allow up and down movement if the map isn't empty
			{
				if (Global::Input.KeyPressed(Keyboard::S) || Global::Input.KeyPressed(Keyboard::Down))
				{
					if (selected_.song != --songs_.end())
					{
						selected_.song++;
						GetSongBeatmaps();
						song_vertical_offset = std::distance(songs_.begin(), selected_.song) * song_vertical_spacing;
					}
				}
				if (Global::Input.KeyPressed(Keyboard::W) || Global::Input.KeyPressed(Keyboard::Up))
				{
					if (selected_.song != songs_.begin())
					{
						selected_.song--;
						GetSongBeatmaps();
						song_vertical_offset = std::distance(songs_.begin(), selected_.song) * song_vertical_spacing;
					}
				}
			}
			break;

		case BEATMAPS:
			if (Global::Input.KeyPressed(Keyboard::A) || Global::Input.KeyPressed(Keyboard::Left))
			{
				selected_.context = SONGS;
				selector_.move(-context_horizontal_spacing, 0.0f);
				break;
			}
			if (Global::Input.KeyPressed(Keyboard::D) || Global::Input.KeyPressed(Keyboard::Right))
			{
				selected_.context = ACTIONS;
				selector_.move(context_horizontal_spacing, 0.0f);
				break;
			}

			if (!songs_.empty()) // Only allow up and down movement if the map isn't empty
			{
				if (Global::Input.KeyPressed(Keyboard::S) || Global::Input.KeyPressed(Keyboard::Down))
				{
					if (selected_.beatmap != --selected_.song->second.end())
					{
						selected_.beatmap++;
						beatmaps_vertical_offset = std::distance(selected_.song->second.begin(), selected_.beatmap) * beatmaps_vertical_spacing;
					}
				}
				if (Global::Input.KeyPressed(Keyboard::W) || Global::Input.KeyPressed(Keyboard::Up))
				{
					if (selected_.beatmap != selected_.song->second.begin())
					{
						selected_.beatmap--;
						beatmaps_vertical_offset = std::distance(selected_.song->second.begin(), selected_.beatmap) * beatmaps_vertical_spacing;
					}
				}

				if (Global::Input.KeyPressed(Keyboard::Space))
				{
					if (!selected_.song->second.empty())
					{
						LoadBeatmap(*selected_.beatmap);
					}
				}

				if (Global::Input.KeyPressed(Keyboard::S))
				{
					if (!selected_.song->second.empty())
					{
						if(beatmap_)
							SaveBeatmap(*beatmap_);
					}
				}
			}
			break;

		case ACTIONS:
		{
			
			if (Global::Input.KeyPressed(Keyboard::A) || Global::Input.KeyPressed(Keyboard::Left))
			{
				selected_.context = BEATMAPS;
				selector_.move(-context_horizontal_spacing, 0.0f);
				break;
			}

			bool moved_vertically_this_update = false;
			switch (selected_.action)
			{
				case PLAY:
					if (Global::Input.KeyPressed(Keyboard::S) || Global::Input.KeyPressed(Keyboard::Down))
					{
						selected_.action = GENERATE;
						action_vertical_offset = -action_vertical_spacing;
						moved_vertically_this_update = true;
					}
					if (Global::Input.KeyPressed(Keyboard::Space))
					{
						if (beatmap_)
						{
							if (!aubio_.IsGenerating())
							{
								ChangeState<GameState>(beatmap_);
							}
							else
								Log::Important("Beatmap generation in progress. Waiting for it to finish.");
						}
						else	
							Log::Important("No beatmap loaded. A beatmap is required to play. Generate or load one.");
					}
					break;
				case GENERATE:
					if (Global::Input.KeyPressed(Keyboard::W) || Global::Input.KeyPressed(Keyboard::Up))
					{
						selected_.action = PLAY;
						action_vertical_offset = action_vertical_spacing;
						moved_vertically_this_update = true;
					}
					if (Global::Input.KeyPressed(Keyboard::Space))
					{
						beatmap_ = aubio_.GenerateBeatmap(selected_.song->first);
					}
					break;
			}
			if (moved_vertically_this_update)
			{
				play_button_.move(0.0f, action_vertical_offset);
				generate_button_.move(0.0f, action_vertical_offset);
			}
			break;
		}
	}
	return true;
}

void MenuState::Render(const float _delta_time)
{
	heading_.setPosition(window_centre.x, machine_.window_.getSize().y * 0.2f);
	for (auto &heading : headings_)
	{
		heading_.setString(heading);
		heading_.move(-(heading_.getGlobalBounds().width * 0.5f), 0.0f);
		machine_.window_.draw(heading_);
		heading_.move((heading_.getGlobalBounds().width * 0.5f), 0.0f);
		heading_.move(context_horizontal_spacing, 0.0f);
	}

	song_text_.setPosition(songs_x, window_centre.y - song_vertical_offset);
	for (auto &song : songs_)
	{
		// Don't draw song text unless it's past the vertical cutoff value
		if (song_text_.getPosition().y > context_vertical_cuttoff)
		{
			song_text_.setString(song.first.song_name());
			song_text_.move(-(song_text_.getGlobalBounds().width * 0.5f), -(song_text_.getGlobalBounds().height * 0.5f));
			machine_.window_.draw(song_text_);
			song_text_.move((song_text_.getGlobalBounds().width * 0.5f), (song_text_.getGlobalBounds().height * 0.5f));
		}
		song_text_.move(0.0f, song_vertical_spacing);
	}

	beatmap_text_.setPosition(beatmaps_x, window_centre.y - beatmaps_vertical_offset);
	if (!songs_.empty())
	{
		if (!selected_.song->second.empty())
		{
			for (auto &beatmap : selected_.song->second)
			{
				// Don't draw song text unless it's past the vertical cutoff value
				if (beatmap_text_.getPosition().y > context_vertical_cuttoff)
				{
					beatmap_text_.setString(beatmap.name_);
					beatmap_text_.move(-(beatmap_text_.getGlobalBounds().width * 0.5f), -(beatmap_text_.getGlobalBounds().height * 0.5f));
					machine_.window_.draw(beatmap_text_);
					beatmap_text_.move((beatmap_text_.getGlobalBounds().width * 0.5f), (beatmap_text_.getGlobalBounds().height * 0.5f));
				}
				beatmap_text_.move(0.0f, beatmaps_vertical_spacing);
			}
		}
	}

	if(play_button_.getPosition().y > context_vertical_cuttoff && play_button_.getPosition().y < machine_.window_.getSize().y - context_vertical_cuttoff)
		machine_.window_.draw(play_button_);

	if (generate_button_.getPosition().y > context_vertical_cuttoff && generate_button_.getPosition().y < machine_.window_.getSize().y - context_vertical_cuttoff)
		machine_.window_.draw(generate_button_);

	machine_.window_.draw(selector_);
}

void MenuState::GUI()
{
	if (!ImGui::Begin("RhythMIR GUI", show_GUI, ImVec2(400, 200), -1.f, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar))
	{
		// Early out if the window is collapsed, as an optimization.
		ImGui::End();
		return;
	}

	switch (selected_.context)
	{
	case SONGS:
		break;
	case BEATMAPS:
		break;
	case ACTIONS:
		aubio_.UpdateGUI();
		break;
	}

	ImGui::End();
}

void MenuState::LoadSongList(const std::string& _file_name)
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
				auto pair = songs_.emplace(Song(source_file, artist, title/*, path_overwrite*/), std::set<Beatmap>());
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

void MenuState::SaveSongList(const std::string& _file_name)
{
	using namespace rapidxml;

	xml_document<> doc;

	xml_node<>* decl = doc.allocate_node(node_declaration);
	decl->append_attribute(doc.allocate_attribute("version", "1.0"));
	decl->append_attribute(doc.allocate_attribute("encoding", "utf-8"));
	doc.append_node(decl);

	xml_node<>* root_node = doc.allocate_node(node_element, "songlist");
	doc.append_node(root_node);

	for (auto &song : songs_)
	{
		xml_node<>* song_node = doc.allocate_node(node_element, "song");
		song_node->append_attribute(doc.allocate_attribute("source", song.first.source_file_name_.c_str()));
		song_node->append_attribute(doc.allocate_attribute("artist", song.first.artist_.c_str()));
		song_node->append_attribute(doc.allocate_attribute("title", song.first.title_.c_str()));
		//song_node->append_attribute(doc.allocate_attribute("beatmap_list_path", song.first.relative_path().c_str()));
		root_node->append_node(song_node);
	}

	// Don't need to check for directory here. If there is no song directory the song list load function will have created one.

	std::ofstream output_stream(_file_name);
	output_stream << doc;
	output_stream.close();
}

void MenuState::LoadBeatmapList(const Song& _song, bool _force_load)
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
						Log::Warning("Unknown beatmap type for " + std::string(beatmap_name) + ".");
						mode = UNKNOWN;
						break;
					}
					std::string beatmap_description;
					if (beatmap_node->first_attribute("description"))
						beatmap_description = beatmap_node->first_attribute("description")->value();
					else
						beatmap_description = std::string();
					auto pair = selected_.song->second.emplace( _song, mode, beatmap_name, beatmap_description );
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
				if (boost::filesystem::exists(song_list_default_filename))
					Log::Error("Beatmap list file for " + selected_.song->first.song_name() + " exists but could not be opened.");
				else
				{
					Log::Message("No Beatmap list file exists for " + selected_.song->first.song_name() + ".");
					Log::Message("Saving a beatmap automatically updates the beatmap list file.");
				}
			}
		}
	}
}

void MenuState::SaveBeatmapList(const Song& _song)
{
	using namespace rapidxml;

	xml_document<> doc;

	xml_node<>* decl = doc.allocate_node(node_declaration);
	decl->append_attribute(doc.allocate_attribute("version", "1.0"));
	decl->append_attribute(doc.allocate_attribute("encoding", "utf-8"));
	doc.append_node(decl);

	xml_node<>* root_node = doc.allocate_node(node_element, "beatmaplist");
	doc.append_node(root_node);

	for (auto &beatmap : songs_.find(_song)->second)
	{
		xml_node<>* beatmap_node = doc.allocate_node(node_element, "beatmap");
		beatmap_node->append_attribute(doc.allocate_attribute("name", beatmap.name_.c_str()));
		auto type = doc.allocate_string(std::to_string(beatmap.play_mode_).c_str());
		beatmap_node->append_attribute(doc.allocate_attribute("type", type));
		beatmap_node->append_attribute(doc.allocate_attribute("description", beatmap.description_.c_str()));
		root_node->append_node(beatmap_node);
	}

	if (!boost::filesystem::exists(_song.relative_path()))
	{
		Log::Message("Creating directory " + _song.relative_path() + ".");
		boost::filesystem::create_directory(_song.relative_path());
	}
	
	std::ofstream output_stream(_song.relative_path() + beatmap_list_default_filename);
	output_stream << doc;
	output_stream.close();
}

void MenuState::GetSongBeatmaps()
{
	if (selected_.song->second.empty())
	{
		LoadBeatmapList(selected_.song->first);
	}
	selected_.beatmap = selected_.song->second.begin();
	beatmaps_vertical_offset = 0.0f;
}

void MenuState::LoadBeatmap(const Beatmap& _beatmap, bool _force_load)
{
	if (!beatmap_ || *beatmap_ != _beatmap || _force_load)
	{
		beatmap_ = aubio_.LoadBeatmap(_beatmap);
	}
}

void MenuState::SaveBeatmap(const Beatmap& _beatmap)
{
	aubio_.SaveBeatmap(_beatmap);
}

void MenuState::GenerateTestBeatmaps()
{
	if (!songs_.empty())
	{
		for (int i = 0; i < 5; ++i)
		{
			auto pair = selected_.song->second.emplace( selected_.song->first, SINGLE, RandomString(6) );
			if (pair.second)
			{
				selected_.beatmap = pair.first;
			}
		}
		beatmaps_vertical_offset = std::distance(selected_.song->second.begin(), selected_.beatmap) * beatmaps_vertical_spacing;
	}
	else
	{
		Log::Warning("Avoided generating beatmaps because songs is empty.");
	}
}

void MenuState::GenerateTestSongs()
{
	for (int i = 0; i < 5; ++i)
	{
		auto random = RandomString(6);
		Song song = Song(random + ".random", RandomString(3), random);
		auto pair = songs_.emplace(song, std::set<Beatmap>());
		if (pair.second)
		{
			selected_.song = pair.first;
		}
	}
	song_vertical_offset = std::distance(songs_.begin(), selected_.song) * song_vertical_spacing;
}