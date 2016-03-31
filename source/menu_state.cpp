#include "menu_state.h"
#include "game_state_machine.h"
#include "game_state.h"

namespace
{
	// Implementation details here

	const std::vector<std::string> headings_ = { "Songs", "Actions" };

	float context_horizontal_spacing;
	float context_vertical_cuttoff;
	float songs_x;
	float song_vertical_spacing;
	float song_vertical_offset;
	float actions_x;
	float action_vertical_spacing;
	float action_vertical_offset;
	sf::Vector2f window_centre;

	TexturePtr play_texture_;
	TexturePtr selector_texture_;
	TexturePtr generate_texture_;

	bool GUI_;
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
	context_horizontal_spacing = machine_.window_.getSize().x * 0.33f;
	context_vertical_cuttoff = machine_.window_.getSize().y * 0.2f;

	songs_x = context_horizontal_spacing;
	song_vertical_spacing = 50.0f;
	song_vertical_offset = 0.0f;

	actions_x = context_horizontal_spacing * 2;
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
	LoadSongList("songs.rhythMIR");

	songs_.insert(Song{ "songs/guitar_vs_piano.wav", "Goukisan", "Guitar Vs. Piano 1.2" });

	selected_.context = SONGS;
	if(!songs_.empty()) // If there are any songs, set selected song to the first element of songs_
		selected_.song = songs_.begin();
	selected_.action = PLAY;

	GUI_ = true;
	beatmap_ = nullptr;
	play_mode_ = FOURKEY;
}

void MenuState::TerminateState()
{
	Global::TextureManager.Unload(textures_);
	textures_.clear();

	SaveSongList("songs.rhythMIR");
}

bool MenuState::Update(const float _delta_time)
{
	switch (selected_.context)
	{

		case SONGS:
			if (Global::Input.KeyPressed(Keyboard::D) || Global::Input.KeyPressed(Keyboard::Right))
			{
				selected_.context = ACTIONS;
				selector_.move(context_horizontal_spacing, 0.0f);
				break;
			}

			if (Global::Input.KeyPressed(Keyboard::S) || Global::Input.KeyPressed(Keyboard::Down))
			{
				if (std::distance(selected_.song, songs_.end()) < 0)
				{
					selected_.song++;
					song_vertical_offset -= song_vertical_spacing;
				}
			}
			if (Global::Input.KeyPressed(Keyboard::W) || Global::Input.KeyPressed(Keyboard::Up))
			{
				if (std::distance(selected_.song, songs_.begin()) > 0)
				{
					selected_.song--;
					song_vertical_offset += song_vertical_spacing;
				}
			}
			break;

		case ACTIONS:
		{
			
			if (Global::Input.KeyPressed(Keyboard::A) || Global::Input.KeyPressed(Keyboard::Left))
			{
				selected_.context = SONGS;
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
							if (beatmap_->song_ == *selected_.song)
							{
								ChangeState<GameState>(beatmap_);
							}
							else
							{
								Log::Important("Beatmap and song selected are mismatched.");
							}
						}
						else
							Log::Important("No beatmap loaded. A beatmap is required to play. Generate or load one.");
					}
					PlayGUI(&GUI_);
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
						beatmap_ = aubio_.GenerateBeatmap(*selected_.song);
					}
					aubio_.UpdateGUI(&GUI_);
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
	heading_.setPosition(context_horizontal_spacing, machine_.window_.getSize().y * 0.2f);
	for (auto heading : headings_)
	{
		heading_.setString(heading);
		heading_.move(-(heading_.getGlobalBounds().width * 0.5f), 0.0f);
		machine_.window_.draw(heading_);
		heading_.move((heading_.getGlobalBounds().width * 0.5f), 0.0f);
		heading_.move(context_horizontal_spacing, 0.0f);
	}

	song_text_.setPosition(songs_x, window_centre.y + song_vertical_offset);
	for (auto song : songs_)
	{
		// Don't draw song text unless it's past the vertical cutoff value
		if (song_text_.getPosition().y > context_vertical_cuttoff)
		{
			song_text_.setString(song.artist_ + " - " + song.title_);
			song_text_.move(-(song_text_.getGlobalBounds().width * 0.5f), -(song_text_.getGlobalBounds().height * 0.5f));
			machine_.window_.draw(song_text_);
			song_text_.move((song_text_.getGlobalBounds().width * 0.5f), (song_text_.getGlobalBounds().height * 0.5f));
		}
		song_text_.move(0.0f, song_vertical_spacing);
	}

	if(play_button_.getPosition().y > context_vertical_cuttoff && play_button_.getPosition().y < machine_.window_.getSize().y - context_vertical_cuttoff)
		machine_.window_.draw(play_button_);

	if (generate_button_.getPosition().y > context_vertical_cuttoff && generate_button_.getPosition().y < machine_.window_.getSize().y - context_vertical_cuttoff)
		machine_.window_.draw(generate_button_);

	machine_.window_.draw(selector_);
}

void MenuState::PlayGUI(bool * _opened)
{
	if (!ImGui::Begin("RhythMIR GUI", _opened, ImVec2(400, 200), -1.f, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar))
	{
		// Early out if the window is collapsed, as an optimization.
		ImGui::End();
		return;
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
				auto source_file = song_node->first_attribute("source")->value();
				auto artist = song_node->first_attribute("artist")->value();
				auto title = song_node->first_attribute("title")->value();
				auto beatmap_list_file = song_node->first_attribute("beatmaps")->value();
				std::string song(artist + std::string(title));
				auto pair = songs_.emplace(Song(source_file, artist, title));
				if (pair.second)
				{
					Log::Message("Added " + std::string(artist) + " - " + std::string(title) + " to the songs set.");
				}
				else
				{
					Log::Warning("Failed emplacing " + std::string(artist) + " - " + std::string(title) + " in the songs set.");
				}

				song_node = song_node->next_sibling("song");
			}
		}
		else
		{
			Log::Error("No song list node found. Format of input file is incorrect.");
		}
	}
	else
	{
		Log::Error("Song list file could not be opened.");
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
		song_node->append_attribute(doc.allocate_attribute("source", song.source_file_name_.c_str()));
		song_node->append_attribute(doc.allocate_attribute("artist", song.artist_.c_str()));
		song_node->append_attribute(doc.allocate_attribute("title", song.title_.c_str()));
		song_node->append_attribute(doc.allocate_attribute("beatmaps", song.beatmap_list_file_name_.c_str()));
		root_node->append_node(song_node);
	}

	std::ofstream output_stream(_file_name);
	output_stream << doc;
	output_stream.close();
}

void MenuState::LoadBeatmapList(const std::string& _file_name)
{
	using namespace rapidxml;

	std::ifstream input_stream(_file_name);
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
				auto beatmap_filename = beatmap_node->first_attribute("filename")->value();
				PLAYMODE mode;
				switch (*beatmap_node->first_attribute("type")->value())
				{
				case 4:
					mode = FOURKEY;
					break;
				case 88:
					mode = PIANO;
					break;
				default:
					Log::Error("Unknown beatmap type for node " + std::string(beatmap_filename) + ".");
					mode = UNKNOWN;
					break;
				}
				auto pair = beatmaps_.emplace(beatmap_filename, mode);
				if (pair.second)
				{
					Log::Message("Added " + std::string(beatmap_filename) + " to the beatmaps set.");
				}
				else
				{
					Log::Warning("Failed emplacing " + std::string(beatmap_filename) + " in the beatmaps set.");
				}

				beatmap_node = beatmap_node->next_sibling("beatmap");
			}
		}
		else
		{
			Log::Error("No beatmap list node found. Format of input file is incorrect.");
		}
	}
	else
	{
		Log::Error("Beatmap list file could not be opened.");
	}
}

void MenuState::SaveBeatmapList(const std::string& _file_name)
{
	using namespace rapidxml;

	xml_document<> doc;

	xml_node<>* decl = doc.allocate_node(node_declaration);
	decl->append_attribute(doc.allocate_attribute("version", "1.0"));
	decl->append_attribute(doc.allocate_attribute("encoding", "utf-8"));
	doc.append_node(decl);

	xml_node<>* root_node = doc.allocate_node(node_element, "beatmaplist");
	doc.append_node(root_node);

	for (auto &beatmap : beatmaps_)
	{
		xml_node<>* beatmap_node = doc.allocate_node(node_element, "beatmap");
		beatmap_node->append_attribute(doc.allocate_attribute("filename", beatmap.first.c_str()));
		beatmap_node->append_attribute(doc.allocate_attribute("type", std::to_string(beatmap.second).c_str()));
		root_node->append_node(beatmap_node);
	}

	std::ofstream output_stream(_file_name);
	output_stream << doc;
	output_stream.close();
}