#include "menu_state.h"

#include "app_state_machine.h"
#include "game_state.h"

#include <SFML_Extensions/global.h>

namespace
{
	// Implementation details

	const std::vector<std::string> headings_ = { "Songs", "Beatmaps" };

	float context_vertical_padding;
	float context_horizontal_spacing;
	float context_vertical_cuttoff;
	float songs_x;
	float song_vertical_spacing;
	float song_vertical_offset;
	float beatmaps_x;
	float beatmaps_vertical_spacing;
	float beatmaps_vertical_offset;
	sf::Vector2f window_centre;
	sf::Vector2f window_size;

	sfx::TexturePtr selector_texture_;


	const char alphanum[] =
		"0123456789"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz";

	const std::vector<std::string> illegal_filename_characters{
		"/",
		"\\",
		",",
		"*",
		".",
		"[",
		"]",
		":",
		";",
		"|",
		"=",
		","
	};

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

MenuState::MenuState(AppStateMachine& _state_machine,
					 UniqueStatePtr<AppState>& _state,
					 BeatmapPtr _beatmap) :
	AppState(_state_machine, _state),
	loaded_beatmap_(std::move(_beatmap))
{}

void MenuState::InitializeState()
{
	machine_.settings_.limit_framerate_ = true;
	ReloadSkin();

	// Calculate various layout stuff
	window_size = sf::Vector2f(machine_.window_.getSize().x, machine_.window_.getSize().y);
	window_centre = sf::Vector2f(window_size.x * 0.5, window_size.y * 0.5);

	context_horizontal_spacing = window_size.x * 0.35f;
	context_vertical_padding = window_size.y * 0.2f;
	context_vertical_cuttoff = context_vertical_padding + window_size.y * 0.08f;

	songs_x = 10;
	song_vertical_spacing = 40.0f;
	song_vertical_offset = 0.0f;

	beatmaps_x = songs_x + context_horizontal_spacing;
	beatmaps_vertical_spacing = 40.0f;
	beatmaps_vertical_offset = 0.0f;

	// Graphics stuff
	selector_ = sfx::Sprite(sf::Vector2f(songs_x, window_centre.y),
							selector_texture_);

	heading_.setCharacterSize(60);
	heading_.setColor(sf::Color::Color(255, 69, 0));
	heading_.setPosition(sf::Vector2f(context_horizontal_spacing, window_size.y * 0.1f));

	song_text_.setCharacterSize(24);
	song_text_.setColor(sf::Color::Color(192, 192, 192));
	song_text_.setPosition(sf::Vector2f(songs_x, window_centre.y));
	
	beatmap_text_.setCharacterSize(24);
	beatmap_text_.setColor(sf::Color::Color(192, 192, 192));
	beatmap_text_.setPosition(sf::Vector2f(beatmaps_x, window_centre.y));

	// Song list and beatmap objects
	selected_.context = SONGS;

	LoadSongList(songs_);
	selected_.song = songs_.begin();

	if (!songs_.empty())
	{
		GetSongBeatmaps();
	}

	// Aubio/Beatmap state stuff
	generating_beatmap_ = false;
	canceling_generating_ = false;
	aubio_ = std::make_unique<Aubio>(generating_beatmap_, canceling_generating_);
	loaded_beatmap_ = nullptr;

	// GUI initialization
	if (machine_.settings_.display_hud_ == false)
		machine_.settings_.display_hud_ = true;
	gui_ = {}; // Default initialize everything
	gui_.main_window = true;
	gui_.deleting_enabled = true;

	// Play Settings initialization
	display_settings_window_ = false;

	if(!LoadGameSettings(game_settings_))
	{
		// Default settings if we couldn't load the settings file
		game_settings_.play_offset = sf::Time::Zero;
		game_settings_.approach_time = sf::milliseconds(700);
		game_settings_.countdown_time = sf::seconds(3);
		game_settings_.keybinds.emplace_back(sf::Keyboard::Key::Z);
		game_settings_.keybinds.emplace_back(sf::Keyboard::Key::X);
		game_settings_.keybinds.emplace_back(sf::Keyboard::Key::N);
		game_settings_.keybinds.emplace_back(sf::Keyboard::Key::M);
		game_settings_.path_count = 4;
		game_settings_.auto_play = false;
		game_settings_.duncan_factor = true;
		game_settings_.flipped = false;
		game_settings_.music_volume = 100;
		game_settings_.sfx_volume = 50;
		game_settings_.hitsound = 1;
		game_settings_.progress_bar_position = 2;
		game_settings_.beat_style = 0;
	}
}

void MenuState::TerminateState()
{
	sfx::Global::TextureManager.Unload(textures_);
	textures_.clear();

	for (auto song = songs_.begin(); song != songs_.end(); ++song)
	{
		if (song->first.source_file_name_ != ".test") // skip test songs
		{
			// Only refresh the beatmap list if beatmaps isn't empty
			// Not doing this causes songs empty beatmap lists to overwrite when a song hasn't been interacted with and we change state
			if (!song->second.empty())
			{
				LoadBeatmapList(song); // Preserve unloaded beatmaps lists by loading them before saving
				SaveBeatmapList(song);
			}
		}
	}

	SaveSongList(songs_);
	SaveGameSettings(game_settings_);
}

bool MenuState::Update(const float _delta_time)
{
	if (sfx::Global::Input.KeyPressed(sf::Keyboard::F10))
	{
		if (machine_.settings_.display_hud_)
			context_vertical_padding = window_size.y * 0.2f;
		else
			context_vertical_padding = 10;
		context_vertical_cuttoff = context_vertical_padding + window_size.y * 0.08f;
	}
	SettingsMenu();
	bool running = MainWindow();
	if (!gui_.input_focus)
	{
		if (sfx::Global::Input.KeyPressed(sf::Keyboard::Num1))
		{
			GenerateTestSong();
		}

		if (sfx::Global::Input.KeyPressed(sf::Keyboard::Num2))
		{
			GenerateTestBeatmap();
		}
		switch (selected_.context)
		{
		case SONGS:
			if (!generating_beatmap_)
			{
				if (sfx::Global::Input.KeyPressed(sf::Keyboard::D) || sfx::Global::Input.KeyPressed(sf::Keyboard::Right))
				{
					selected_.context = BEATMAPS;
					selector_.move(context_horizontal_spacing, 0.0f);
					if (!selected_.song->second.empty())
					{
						LoadBeatmap(*selected_.beatmap);
					}
					break;
				}

				if (!songs_.empty()) // Only allow up and down movement if the map isn't empty
				{
					if (sfx::Global::Input.KeyPressed(sf::Keyboard::S) || sfx::Global::Input.KeyPressed(sf::Keyboard::Down))
					{
						if (selected_.song != --songs_.end())
						{
							selected_.song++;
							GetSongBeatmaps();
							song_vertical_offset = std::distance(songs_.begin(), selected_.song) * song_vertical_spacing;
						}
						if (!selected_.song->second.empty())
						{
							LoadBeatmap(*selected_.beatmap);
						}
					}
					if (sfx::Global::Input.KeyPressed(sf::Keyboard::W) || sfx::Global::Input.KeyPressed(sf::Keyboard::Up))
					{
						if (selected_.song != songs_.begin())
						{
							selected_.song--;
							GetSongBeatmaps();
							song_vertical_offset = std::distance(songs_.begin(), selected_.song) * song_vertical_spacing;
						}
						if (!selected_.song->second.empty())
						{
							LoadBeatmap(*selected_.beatmap);
						}
					}
				}
			}
			break;

		case BEATMAPS:
			if (!generating_beatmap_)
			{
				if (sfx::Global::Input.KeyPressed(sf::Keyboard::A) || sfx::Global::Input.KeyPressed(sf::Keyboard::Left))
				{
					selected_.context = SONGS;
					selector_.move(-context_horizontal_spacing, 0.0f);
					break;
				}

				if (!songs_.empty())
				{
					if (!selected_.song->second.empty()) // Only allow beatmap input if there are beatmaps
					{
						if (sfx::Global::Input.KeyPressed(sf::Keyboard::S) || sfx::Global::Input.KeyPressed(sf::Keyboard::Down))
						{
							if (selected_.beatmap != --selected_.song->second.end())
							{
								selected_.beatmap++;
								beatmaps_vertical_offset = std::distance(selected_.song->second.begin(), selected_.beatmap) * beatmaps_vertical_spacing;
								LoadBeatmap(*selected_.beatmap);
							}
						}
						if (sfx::Global::Input.KeyPressed(sf::Keyboard::W) || sfx::Global::Input.KeyPressed(sf::Keyboard::Up))
						{
							if (selected_.beatmap != selected_.song->second.begin())
							{
								selected_.beatmap--;
								beatmaps_vertical_offset = std::distance(selected_.song->second.begin(), selected_.beatmap) * beatmaps_vertical_spacing;
								LoadBeatmap(*selected_.beatmap);
							}
						}
						if (sfx::Global::Input.KeyPressed(sf::Keyboard::Space))
						{
							Play();
						}
					}
				}
			}
			break;
		}

		/*if (sfx::Global::Input.KeyPressed(sf::Keyboard::F9) && sfx::Global::Input.KeyDown(sf::Keyboard::LControl))
		{
			gui_.deleting_enabled = !gui_.deleting_enabled;
		}*/
	}
	return running;
}

void MenuState::Render(const float _delta_time)
{
	machine_.window_.draw(selector_);

	heading_.setPosition(songs_x, context_vertical_padding);
	for (auto &heading : headings_)
	{
		heading_.setString(heading);
		machine_.window_.draw(heading_);
		heading_.move(context_horizontal_spacing, 0.0f);
	}

	song_text_.setPosition(songs_x, window_centre.y - song_vertical_offset);
	for (auto &song : songs_)
	{
		// Don't draw song text unless it's past the vertical cutoff value
		if (song_text_.getPosition().y > context_vertical_cuttoff)
		{
			auto song_name = song.first.song_name();
			if (song_name.size() > 40)
			{
				song_name.resize(40);
				song_name += "...";
			}
			song_text_.setString(song_name);
			song_text_.move(0.0f, -(song_text_.getGlobalBounds().height * 0.5f));
			machine_.window_.draw(song_text_);
			song_text_.move(0.0f, (song_text_.getGlobalBounds().height * 0.5f));
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
				// Don't draw beatmap text unless it's past the vertical cutoff value
				if (beatmap_text_.getPosition().y > context_vertical_cuttoff)
				{
					beatmap_text_.setString(beatmap.name_);
					beatmap_text_.move(0.0f, -(beatmap_text_.getGlobalBounds().height * 0.5f));
					machine_.window_.draw(beatmap_text_);
					beatmap_text_.move(0.0f, (beatmap_text_.getGlobalBounds().height * 0.5f));
				}
				beatmap_text_.move(0.0f, beatmaps_vertical_spacing);
			}
		}
	}
}

void MenuState::ProcessEvent(sf::Event& _event)
{
	switch (_event.type)
	{
	case sf::Event::Resized:
		window_size = sf::Vector2f(_event.size.width, _event.size.height);
		break;
	}
}

void MenuState::ReloadSkin()
{
	textures_ = sfx::TextureFileVector
	{
		{ machine_.background_texture_, Skin("menu_background.jpg") },
		{ selector_texture_, Skin("selector.png") }
	};
	sfx::Global::TextureManager.Load(textures_);
	machine_.background_.setTexture(*machine_.background_texture_);

	heading_.setFont(machine_.font_);
	song_text_.setFont(machine_.font_);
	beatmap_text_.setFont(machine_.font_);
}

void MenuState::SettingsMenu()
{
	if (display_settings_window_)
	{
		ImGui::SetNextWindowSize(ImVec2(600, 450), ImGuiSetCond_FirstUseEver);
		if (!ImGui::Begin("Play Settings"))
		{
			// Early out if the window is collapsed, as an optimization.
			ImGui::End();
			return;
		}

		ImGui::TextColored(ImColor(255, 69, 0), "Mods");
		ImGui::SameLine();
		//ImGui::Checkbox("Shuffle", &game_settings_.duncan_factor);
		//if (ImGui::IsItemHovered())
			//ImGui::SetTooltip("Randomizes the lanes that notes go down.\nWill currently be forced to on for Single beatmaps and off for Four Key beatmaps.");
		//ImGui::SameLine();
		ImGui::Checkbox("Autoplay", &game_settings_.auto_play);
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Disables input keys while playing, the computer automatically clicks the circles.\na.k.a. Cookiezi.exe");
		ImGui::SameLine();
		ImGui::Checkbox("Flip", &game_settings_.flipped);
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Notes go from bottom to top instead of top to bottom.");

		ImGui::Spacing();

		static int play_offset = game_settings_.play_offset.asMilliseconds();
		ImGui::SliderInt("Play Offset", &play_offset, -100, 100, "%.0fms");
		game_settings_.play_offset = sf::milliseconds(play_offset);
		if (game_settings_.play_offset != sf::Time::Zero)
		{
			ImGui::SameLine();
			if (ImGui::Button("Default##playoffset"))
			{
				play_offset = 0.0f;
			}
		}

		static float approach_time = game_settings_.approach_time.asSeconds();
		ImGui::SliderFloat("Approach Time", &approach_time, 0.3, 3, "%.2fs", 1.5f);
		game_settings_.approach_time = sf::seconds(approach_time);
		if (game_settings_.approach_time != sf::milliseconds(700))
		{
			ImGui::SameLine();
			if (ImGui::Button("Default##approachtime"))
			{
				approach_time = 0.7f;
			}
		}

		static float countdown_time = game_settings_.countdown_time.asSeconds();
		ImGui::SliderFloat("Countdown Time", &countdown_time, approach_time, 5, "%.2fs", 1.0f);
		game_settings_.countdown_time = sf::seconds(countdown_time);
		if (game_settings_.countdown_time != sf::seconds(3))
		{
			ImGui::SameLine();
			if (ImGui::Button("Default##countdowntime"))
			{
				countdown_time = 3.0f;
			}
		}

		ImGui::Separator();
		ImGui::Spacing();

		const char* beatstyle[] = { "Hidden", "Interpolated", "Generated" };
		ImGui::Combo("Beat ", &game_settings_.beat_style, beatstyle, 3);
		const char* hitsounds[] = { "None", "Soft", "Deep" };
		ImGui::Combo("Hitsound", &game_settings_.hitsound, hitsounds, 3);
		ImGui::SliderFloat("Music Volume", &game_settings_.music_volume, 0, 100);
		if (game_settings_.music_volume != 100)
		{
			ImGui::SameLine();
			if (ImGui::Button("Default##musicvolume"))
			{
				game_settings_.music_volume = 100;
			}
		}
		ImGui::SliderFloat("SFX Volume", &game_settings_.sfx_volume, 0, 100);
		if (game_settings_.sfx_volume != 50)
		{
			ImGui::SameLine();
			if (ImGui::Button("Default##sfxvolume"))
			{
				game_settings_.sfx_volume = 50;
			}
		}

		ImGui::Spacing();

		const char* barpositions[] = { "Top Right", "Along Top", "Along Bottom" };
		ImGui::Combo("Progress Bar Position", &game_settings_.progress_bar_position, barpositions, 3);

		if (ImGui::Button("Done"))
		{
			display_settings_window_ = false;
		}

		ImGui::End();
	}
}

bool MenuState::MainWindow()
{
	ImGui::SetNextWindowPos(ImVec2(beatmaps_x + (context_horizontal_spacing * 0.7f), context_vertical_cuttoff - window_size.y * 0.04f));
	ImGui::SetNextWindowSize(ImVec2(window_size.x - (beatmaps_x + (context_horizontal_spacing * 0.7f) + 10), window_size.y - (context_vertical_cuttoff - window_size.y * 0.04f + 10)));
	if (!ImGui::Begin("RhythMIR GUI", &gui_.main_window, ImVec2(0, 0), -1.f,
					  ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
					  ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings))
	{
		// Early out if the window is collapsed, as an optimization.
		ImGui::End();
		return true;
	}

	if (canceling_generating_)
	{
		Log::Message("Cancelled beatmap generation.");
		canceling_generating_ = false;
	}

	if (sfx::Global::Input.KeyPressed(sf::Keyboard::Escape))
	{
		ImGui::OpenPopup("Exit?");
	}
	if (ImGui::BeginPopupModal("Exit?", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{

		if (ImGui::Button("OK", ImVec2(120, 0)))
		{
			ImGui::End();
			ImGui::CloseCurrentPopup();
			return false;
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120, 0)))
			ImGui::CloseCurrentPopup();
		ImGui::EndPopup();
	}

	ImGui::TextWrapped("Welcome to RhythMIR!");
	if (ImGui::CollapsingHeader("Introduction and Controls", nullptr, true, true))
	{
		ImGui::TextColored(ImColor(255, 69, 0), "Introduction");
		ImGui::TextWrapped("There are two main things to do within RhythMIR currently. Generate new beatmaps and play saved beatmaps.\n\nGenerating beatmaps has a large amount of settings, it is very easy to get bad output from the beatmap generator. Recommendations are shown in the tooltips for many settings when hovering the mouse over them.\n\nPlaying saved beatmaps can be done by pressing play button or space when the selector is over a beatmap.\nGame settings can be used to change how the game plays in various ways.");
		
		ImGui::TextColored(ImColor(255, 69, 0), "Controls");
		ImGui::TextWrapped("Changing songs/beatmaps: WASD or Arrow Keys\nClicking circles while playing: Z, X, N and M (left to right)\nEverything else: Point and click using mouse");

		ImGui::TextWrapped("Toggle Frame Limiter: F7");
		ImGui::TextWrapped("Toggle Deleting: LCtrl+F9\nPlease do not delete beatmaps or songs you didn't make/add.");
		ImGui::TextWrapped("Toggle Console: F10");
		//ImGui::TextWrapped("Toggle ImGui Test:");
		ImGui::Separator();
	}

	//ImGui::TextColored(ImColor(255, 69, 0), "Song UI");
	if(ImGui::CollapsingHeader("Song UI"))
	{
		ImGui::TextWrapped("This UI is for adding and deleting songs in RhythMIR.");
		ImGui::TextWrapped("Place the source file for the song in the songs directory (songs/). RhythMIR currently only supports .wav files.");
		ImGui::TextWrapped("Once the song source file is in the songs directory, enter the artist, title and source file name (e.g. \"example.wav\") in the text boxes below and click add.");

		ImGui::InputText("Artist", gui_.song_artist, 256);
		ImGui::InputText("Title", gui_.song_title, 256);
		ImGui::InputText("Source", gui_.song_source, 256);

		if (ImGui::Button("Add Song"))
		{
			if (std::string(gui_.song_artist) != std::string() &&
				std::string(gui_.song_title) != std::string() &&
				std::string(gui_.song_source) != std::string()) // No fields may be empty
			{
				auto artist = std::string(gui_.song_artist);
				agn::trim(artist);
				strcpy(gui_.song_artist, artist.c_str());

				auto title = std::string(gui_.song_title);
				agn::trim(title);
				strcpy(gui_.song_title, title.c_str());

				auto artist_iterator = std::find_if(illegal_filename_characters.begin(), illegal_filename_characters.end(),
					[&](const std::string& s)
				{
					return artist.find(s) != std::string::npos;
				});
				auto title_iterator = std::find_if(illegal_filename_characters.begin(), illegal_filename_characters.end(),
					[&](const std::string& s)
				{
					return title.find(s) != std::string::npos;
				});
				if (artist_iterator == illegal_filename_characters.end() && title_iterator == illegal_filename_characters.end())
				{
					auto song_to_add = Song(gui_.song_artist, gui_.song_title, gui_.song_source);
					if (songs_.count(song_to_add) == 0)
					{
						if (boost::filesystem::exists(Filesystem::current_song_path + song_to_add.source_file_name_))
						{
							auto pair = songs_.emplace(song_to_add, std::set<Beatmap>());
							if (pair.second)
							{
								selected_.song = pair.first;
								song_vertical_offset = std::distance(songs_.begin(), selected_.song) * song_vertical_spacing;
								selected_.beatmap = selected_.song->second.begin();

								SaveSongList(songs_);

								Log::Message("Creating directory " + song_to_add.relative_path() + ".");
								boost::filesystem::create_directory(song_to_add.relative_path());
								boost::filesystem::rename(Filesystem::current_song_path + song_to_add.source_file_name_, song_to_add.full_file_path());
							}
							else
							{
								Log::Error("Failed to emplace " + song_to_add.song_name() + " in the songs map.");
							}
						}
						else
						{
							Log::Error("Could not find the song source in the song directory. Make sure the source file is in the songs folder (not a subfolder). RhythMIR will create the song folder and move the source file to it.");
						}
					}
					else
					{
						Log::Error("Song already exists in song list. Duplicates with the same artist - title are not allowed.");
					}
				}
				else
				{
					Log::Error("Invalid characters. Artist and title may not contain * . \" / \\ [ ] : ; | = ,");
				}
			}
			else
			{
				Log::Error("Artist, Title and Source are all required to add a song.");
			}
		}
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Add a song to the songlist using the text boxes below.");
		if (gui_.deleting_enabled)
		{
			ImGui::SameLine();
			if (ImGui::Button("Delete Song"))
			{
				if (!songs_.empty() && !generating_beatmap_)
				{
					gui_.song_to_delete = selected_.song;
					gui_.delete_song_popup = "Delete " + selected_.song->first.song_name() + "?";
					ImGui::OpenPopup(gui_.delete_song_popup.c_str());
				}
				else
				{
					Log::Message("No songs to delete.");
				}
			}
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Deletes the selected song from the songlist, optionally deleting it from disk too.");
			if (ImGui::BeginPopupModal(gui_.delete_song_popup.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize))
			{
				ImGui::Text("Are you sure?\n");
				ImGui::Separator();

				static bool also_delete_from_disk = true;
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
				ImGui::Checkbox("Also delete from disk", &also_delete_from_disk);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("This will delete the whole folder, including every beatmap and the music file.");
				ImGui::PopStyleVar();

				if (ImGui::Button("OK", ImVec2(120, 0)))
				{
					if (loaded_beatmap_)
					{
						if (gui_.song_to_delete->first == loaded_beatmap_->song_)
						{
							loaded_beatmap_.reset();
						}
					}
					if (also_delete_from_disk)
					{
						// IMPORTANT: NO FILES CAN BE OPENED OR REMOVE_ALL WILL THROW
						boost::filesystem::remove_all(gui_.song_to_delete->first.relative_path());
					}
					if (selected_.song == --songs_.end()) // If its the last song we have to set the selected song to before the end manually
					{
						songs_.erase(gui_.song_to_delete);
						if (!songs_.empty())
							selected_.song = --songs_.end();
					}
					else
						selected_.song = songs_.erase(gui_.song_to_delete);
					if (!songs_.empty())
					{
						song_vertical_offset = std::distance(songs_.begin(), selected_.song) * song_vertical_spacing;
						selected_.beatmap = selected_.song->second.begin();
						beatmaps_vertical_offset = std::distance(selected_.song->second.begin(), selected_.beatmap) * beatmaps_vertical_spacing;
					}

					ImGui::CloseCurrentPopup();
				}
				ImGui::SameLine();
				if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
				ImGui::EndPopup();
			}
			if (generating_beatmap_)
			{
				ImGui::SameLine();
				ImGui::TextDisabled("Deleting songs is disabled while generating a beatmap.");
			}
		}
	}

	if (!songs_.empty())
	{
		ImGui::Separator();
		ImGui::TextColored(ImColor(255, 69, 0), "Beatmap UI");

		ImGui::TextWrapped("This UI is for adding, loading, saving and deleting beatmaps in RhythMIR.");
		ImGui::TextWrapped("Loading, saving and deleting is only enabled when a song has beatmaps.");


		// BEATMAP FILE I/O SECTION
		if (!selected_.song->second.empty()) // Only allow beatmap input if there are beatmaps
		{
			// LOAD BEATMAP
			/*if (ImGui::Button("Load Beatmap"))
			{
				if (!generating_beatmap_)
					LoadBeatmap(*selected_.beatmap);
				else
					Log::Message("Disabled while generating a beatmap.");
			}
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Loads the selected beatmap from file.");*/

			//ImGui::SameLine();

			// SAVE BEATMAP
			/*if (ImGui::Button("Save Beatmap"))
			{
				if (!generating_beatmap_)
				{
					if (beatmap_)
						SaveBeatmap(*beatmap_);
					else
						Log::Message("No beatmap to save.");
				}
				else
				{
					Log::Message("Disabled while generating a beatmap.");
				}
			}
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Saves the current beatmap to file.");*/

			//ImGui::SameLine();

			if (gui_.deleting_enabled)
			{
				// DELETE BEATMAP
				if (ImGui::Button("Delete Beatmap"))
				{
					if (!generating_beatmap_)
					{
						if (!selected_.song->second.empty())
						{
							gui_.beatmap_to_delete = selected_.beatmap;
							gui_.delete_beatmap_popup = "Delete " + selected_.song->first.song_name() + " [" + selected_.beatmap->name_ + "] ?";
							ImGui::OpenPopup(gui_.delete_beatmap_popup.c_str());
						}
						else
						{
							Log::Message("No beatmap to delete.");
						}
					}
					else
					{
						Log::Message("Disabled while generating a beatmap.");
					}
				}
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Deletes the selected beatmap, optionally deleting it from disk too.");
				if (ImGui::BeginPopupModal(gui_.delete_beatmap_popup.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize))
				{
					ImGui::Text("Are you sure?\n");
					ImGui::Separator();

					static bool also_delete_from_disk = true;
					ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
					ImGui::Checkbox("Also delete from disk", &also_delete_from_disk);
					if (ImGui::IsItemHovered())
						ImGui::SetTooltip("This will delete the beatmap file from disk.");
					ImGui::PopStyleVar();

					if (ImGui::Button("OK", ImVec2(120, 0)))
					{
						if (also_delete_from_disk)
						{
							// IMPORTANT: BEATMAP FILE CANNOT BE OPENED OR REMOVE_ALL WILL THROW
							boost::filesystem::remove_all(gui_.beatmap_to_delete->full_file_path() + ".RhythMIR");
						}
						if (selected_.beatmap == --selected_.song->second.end()) // If its the last beatmap we have to set the selected beatmap to before the end manually
						{
							selected_.song->second.erase(selected_.beatmap);
							if (!selected_.song->second.empty())
								selected_.beatmap = --selected_.song->second.end();
						}
						else
							selected_.beatmap = selected_.song->second.erase(selected_.beatmap);
						if (!selected_.song->second.empty())
							beatmaps_vertical_offset = std::distance(selected_.song->second.begin(), selected_.beatmap) * beatmaps_vertical_spacing;
						SaveBeatmapList(selected_.song);
						loaded_beatmap_.reset();

						ImGui::CloseCurrentPopup();
					}
					ImGui::SameLine();
					if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
					ImGui::EndPopup();
				}

				ImGui::SameLine();

				// DELETE ALL BEATMAPS
				if (ImGui::Button("Delete All Beatmaps"))
				{
					if (!generating_beatmap_)
					{
						if (!selected_.song->second.empty())
						{
							gui_.delete_all_beatmap_popup = "Delete all beatmaps for " + selected_.song->first.song_name() + "?";
							ImGui::OpenPopup(gui_.delete_all_beatmap_popup.c_str());
						}
						else
						{
							Log::Message("No beatmaps to delete.");
						}
					}
					else
					{
						Log::Message("Disabled while generating a beatmap.");
					}
				}
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Deletes all songs for the selected beatmap, optionally deleting them from disk too.");
				if (ImGui::BeginPopupModal(gui_.delete_all_beatmap_popup.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize))
				{
					ImGui::Text("Are you sure?\n");
					ImGui::Separator();

					static bool also_delete_from_disk = true;
					ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
					ImGui::Checkbox("Also delete from disk", &also_delete_from_disk);
					if (ImGui::IsItemHovered())
						ImGui::SetTooltip("This will delete the beatmap files from disk.");
					ImGui::PopStyleVar();

					if (ImGui::Button("OK", ImVec2(120, 0)))
					{
						if (also_delete_from_disk)
						{
							for (auto &beatmap : selected_.song->second)
							{
								// IMPORTANT: BEATMAP FILE CANNOT BE OPENED OR REMOVE_ALL WILL THROW
								boost::filesystem::remove_all(beatmap.full_file_path() + ".RhythMIR");
							}
						}
						while (!selected_.song->second.empty())
						{
							selected_.beatmap = --selected_.song->second.end();
							selected_.song->second.erase(selected_.beatmap);

							SaveBeatmapList(selected_.song);
						}
						loaded_beatmap_.reset();
						beatmaps_vertical_offset = 0.0f;
						ImGui::CloseCurrentPopup();
					}
					ImGui::SameLine();
					if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
					ImGui::EndPopup();
				}

				if (generating_beatmap_)
				{
					ImGui::SameLine();
					ImGui::TextDisabled("Disabled while generating a beatmap.");
				}
			}
		}
		else
		{
			ImGui::TextWrapped("There are no beatmaps for the selected song.");
		}

		ImGui::Spacing();

		// GENERATE SECTION
		if (!generating_beatmap_)
		{
			ImGui::InputText("Name", gui_.beatmap_name, 256);
			ImGui::InputTextMultiline("Description (optional)", gui_.beatmap_description, 2048, ImVec2(0.0f, ImGui::GetTextLineHeight() * 8), ImGuiInputTextFlags_AllowTabInput);

			// GENERATE BEATMAP
			if (ImGui::Button("Generate Beatmap"))
			{
				if (gui_.beatmap_name != std::string())
				{
					auto name = std::string(gui_.beatmap_name);
					agn::trim(name);
					strcpy(gui_.beatmap_name, name.c_str());

					auto name_iterator = std::find_if(illegal_filename_characters.begin(), illegal_filename_characters.end(),
													   [&](const std::string& s)
					{
						return name.find(s) != std::string::npos;
					});

					if (name_iterator == illegal_filename_characters.end())
					{
						if (selected_.song->second.count(Beatmap(selected_.song->first, gui_.beatmap_name)) == 0)
						{
							loaded_beatmap_ = aubio_->GenerateBeatmap(selected_.song->first, gui_.beatmap_name, gui_.beatmap_description);
							auto pair = selected_.song->second.emplace(selected_.song->first, gui_.beatmap_name);
							if (pair.second)
							{
								selected_.beatmap = pair.first;
								beatmaps_vertical_offset = std::distance(selected_.song->second.begin(), selected_.beatmap) * beatmaps_vertical_spacing;

								SaveBeatmapList(selected_.song);
							}
							else
							{
								Log::Error("Failed to emplace " + selected_.song->first.song_name() + " [" + selected_.beatmap->name_ + "] in the beatmap set.");
							}

						}
						else
							Log::Error("Choose a different beatmap name. A beatmap with this name already exists.");
					}
					else
						Log::Error("Invalid characters. Beatmap name may not contain * . \" / \\ [ ] : ; | = ,");
				}
				else
					Log::Error("Enter a beatmap name.");
			}
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Generates a beatmap for the selected song using current settings .");
		}

		aubio_->MainWindow();

		ImGui::Separator();
		ImGui::TextColored(ImColor(255, 69, 0), "Play UI");
		if (loaded_beatmap_)
		{
			std::string current_beatmap = "Current Loaded Beatmap: " + loaded_beatmap_->song_.song_name() + " [" + loaded_beatmap_->name_ + "]";
			ImGui::TextWrapped(current_beatmap.c_str());
			std::string beatmap_type("Beatmap Type: ");
			switch (loaded_beatmap_->play_mode_)
			{
			case VISUALIZATION:
				beatmap_type += "Visualization";
				break;
			case SINGLE:
				beatmap_type += "Single";
				break;
			case FOURKEY:
				beatmap_type += "Four Key";
				break;
			default:
				beatmap_type += "Unknown";
				break;
			}
			ImGui::TextWrapped(beatmap_type.c_str());
			ImGui::TextWrapped(loaded_beatmap_->description_.c_str());
			ImGui::TextWrapped("For details about each beatmap type see go to the introduction section.");
		}
		else
		{
			ImGui::TextDisabled("Current Loaded Beatmap: None.");
			ImGui::TextDisabled("Beatmap Type: None");
			ImGui::TextDisabled("For details about each beatmap type see go to the introduction section.");
		}

		if (ImGui::Button("Play"))
		{
			Play();
			ImGui::End();
			return true;
		}
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Play the currently loaded beatmap.");

		ImGui::SameLine();
		if (ImGui::Button("Game Settings"))
			display_settings_window_ = !display_settings_window_;
	}
	else
	{
		ImGui::TextWrapped("It appears you have no songs! Use the song UI to add songs.");
	}
	gui_.input_focus = ImGui::IsAnyItemActive();

	ImGui::End();

	aubio_->ExtraWindow();

	return true;
}

void MenuState::Play()
{
	if (loaded_beatmap_)
	{
		if (!generating_beatmap_)
		{
			LoadBeatmap(*selected_.beatmap, false, true);
			if (loaded_beatmap_->play_mode_ == SINGLE)
				game_settings_.duncan_factor = true;
			else if (loaded_beatmap_->play_mode_ == FOURKEY)
				game_settings_.duncan_factor = false;
			ChangeState<GameState>(std::move(loaded_beatmap_), std::move(game_settings_));
			return;
		}
		else
			Log::Message("Beatmap generation in progress. Waiting for it to finish.");
	}
	else
		Log::Message("No beatmap loaded. A beatmap is required to play. Generate or load one.");
}



void MenuState::GetSongBeatmaps()
{
	if (selected_.song->second.empty() && selected_.song->first.source_file_name_ != ".test")
	{
		LoadBeatmapList(selected_.song);
	}
	selected_.beatmap = selected_.song->second.begin();
	beatmaps_vertical_offset = 0.0f;
}

void MenuState::LoadBeatmap(const Beatmap& _beatmap, bool _partial_load, bool _force_load)
{
	if (!loaded_beatmap_ || *loaded_beatmap_ != _beatmap || _force_load)
	{
		loaded_beatmap_ = Filesystem::LoadBeatmap(_beatmap, _partial_load);
	}
}

void MenuState::SaveBeatmap(const Beatmap& _beatmap)
{
	Filesystem::SaveBeatmap(_beatmap);
}

void MenuState::GenerateTestBeatmap()
{
	if (!songs_.empty())
	{
		auto pair = selected_.song->second.emplace( selected_.song->first, RandomString(6), "", UNKNOWN );
		//if (pair.second)
		{
			//selected_.beatmap = pair.first;
		}
		//beatmaps_vertical_offset = std::distance(selected_.song->second.begin(), selected_.beatmap) * beatmaps_vertical_spacing;
	}
	else
	{
		Log::Warning("Avoided generating beatmaps because songs is empty.");
	}
}

void MenuState::GenerateTestSong()
{
	Song song = Song(RandomString(3), RandomString(6), ".test");
	auto pair = songs_.emplace(song, std::set<Beatmap>());
	//if (pair.second)
	{
		//selected_.song = pair.first;
	}
	//song_vertical_offset = std::distance(songs_.begin(), selected_.song) * song_vertical_spacing;
}