#include "game_state.h"

#include "app_state_machine.h"
#include "menu_state.h"

#include <SFML_Extensions/global.h>

namespace
{
	sf::Vector2f window_centre;
	sf::Vector2f window_size;

	float path_start_y;
	float path_end_y;
	float path_x;
	float path_x_offset;
	float note_length;

	bool interpolate_beats;
	sf::Time beat_interval;
	sf::Time beat_time;

	const int performance_statistic_count = 10;
	const int performance_y_offset = 24.0f;
	
}

GameState::GameState(AppStateMachine& _state_machine,
					 UniqueStatePtr<AppState>& _state,
					 BeatmapPtr _beatmap,
					 GameSettings _settings) :
	AppState(_state_machine, _state),
	beatmap_(std::move(_beatmap)),
	settings_(std::move(_settings)),
	play_clock_(),
	paused_(false),
	finished_(false),
	hit_counters_(true),
	current_section_(nullptr)
{
}

void GameState::InitializeState()
{
	textures_ = sfx::TextureFileVector
	{
		{ machine_.background_texture_, "skins/default/play_background.png" },
		{ pause_background_texture_, "skins/default/pause_background.png" },
		{ white_circle_texture_, "skins/default/circle_white.png", },
		{ beat_texture_, "skins/default/beat.png" }
	};
	sfx::Global::TextureManager.Load(textures_);

	machine_.background_.setTexture(*machine_.background_texture_);

	sounds_ = sfx::SoundFileVector
	{
		{ deep_hit_sound_, "skins/default/deep_hit.wav" },
		{ soft_hit_sound_, "skins/default/soft_hit.wav" },
		{ miss_sound_, "skins/default/combobreak.wav" }
	};
	sfx::Global::AudioManager.Load(sounds_);

	window_centre = sf::Vector2f(machine_.window_.getSize().x * 0.5, machine_.window_.getSize().y * 0.5);
	window_size = sf::Vector2f(machine_.window_.getSize());

	pause_background_ = sfx::Sprite(window_centre, pause_background_texture_);

	// Clock text initialization
	clock_text_.setFont(machine_.font_);
	clock_text_.setCharacterSize(45);
	clock_text_.setColor(sf::Color::Color(255, 69, 0));
	clock_text_.setPosition(sf::Vector2f(window_size.x, 0.0f));

	// Countdown text initialization
	countdown_text_.setFont(machine_.font_);
	countdown_text_.setCharacterSize(120);
	countdown_text_.setColor(sf::Color::Color(255, 69, 0));
	countdown_text_.setPosition(window_centre);

	// Score initialization
	score_text_.setFont(machine_.font_);
	score_text_.setCharacterSize(60);
	score_text_.setColor(sf::Color::Color(255, 69, 0));
	score_text_.setPosition(sf::Vector2f(window_size.x, 0.0f));
	score_ = 0;

	// Hit counters initialization
	performance_text_.setFont(machine_.font_);
	performance_text_.setCharacterSize(30);
	performance_text_.setPosition(sf::Vector2f(5.0f, window_size.y - performance_y_offset * 10 - 5.0f));
	perfect_hits_ = 0;
	great_hits_ = 0;
	good_hits_ = 0;
	misses_ = 0;
	hit_combo_ = 0;
	max_combo_ = 0;
	average_hit_ = 0.0f;
	unstable_rate_ = 0.0f;
	earliest_hit_ = -300;
	latest_hit_ = 300;
	hits_.clear();

	// Beatmap initialization
	sections_ = beatmap_->CopyTimingSections();

	interpolate_beats = false;
	switch (settings_.beat_style)
	{
	case GameSettings::HIDDEN:
		break;
	case GameSettings::INTERPOLATED:
		if (sections_.front().BPM != 0.0f && sections_.front().offset != sf::Time::Zero)
		{
			interpolate_beats = true;
			beat_interval = sf::milliseconds(1000 * 60 / sections_.front().BPM);
			beat_time = beat_interval;
		}
		break;
	case GameSettings::GENERATED:
		beatqueue_ = beatmap_->CopyBeats();
		break;
	}
	
	if (!beatmap_->music_)
		beatmap_->LoadMusic();
	beatmap_->music_->setPlayingOffset(sf::Time::Zero);
	srand(time(0));

	machine_.settings_.limit_framerate_ = false;
	machine_.settings_.display_hud_ = false;

	switch (beatmap_->play_mode_)
	{
	case VISUALIZATION:
		settings_.path_count = sections_.front().notes.size();
		InitializeVisualizationMode();
		break;
	case SINGLE:
		InitializeFourKeyMode();
		break;
	case FOURKEY:
		InitializeFourKeyMode();
		break;
	}
}

void GameState::TerminateState()
{
	sfx::Global::TextureManager.Unload(textures_);
	textures_.clear();
	sfx::Global::AudioManager.Unload(sounds_);
	sounds_.clear();
}

bool GameState::Update(const float _delta_time)
{
	if (finished_)
	{
		if (sfx::Global::Input.KeyPressed(sf::Keyboard::BackSpace))
		{
			ChangeState<MenuState>(std::move(beatmap_));
			return true;
		}
		if (sfx::Global::Input.KeyPressed(sf::Keyboard::Return))
		{
			ChangeState<GameState>(std::move(beatmap_), std::move(settings_));
			return true;
		}
		if (!PauseMenu())
		{
			return true;
		}
	}
	else
	{
		if (sfx::Global::Input.KeyPressed(sf::Keyboard::Escape))
		{
			paused_ = !paused_;
		}

		if (settings_.music_volume != beatmap_->music_->getVolume())
			beatmap_->music_->setVolume(settings_.music_volume);

		if (paused_)
		{
			play_clock_.Stop();
			if (beatmap_->music_->getStatus() == sf::Music::Playing)
				beatmap_->music_->pause();
			if (sfx::Global::Input.KeyPressed(sf::Keyboard::BackSpace))
			{
				ChangeState<MenuState>(std::move(beatmap_));
				return true;
			}
			if (sfx::Global::Input.KeyPressed(sf::Keyboard::Return))
			{
				ChangeState<GameState>(std::move(beatmap_), std::move(settings_));
				return true;
			}
			if (!PauseMenu())
			{
				return true;
			}
		}
		else
		{
			play_clock_.Start();

			auto time_elapsed = play_clock_.GetTimeElapsed();

			// If more than the play offset (intro time) seconds and
			// less than the duration of the music + play offset seconds has elapsed then...
			if (time_elapsed > settings_.countdown_time && time_elapsed < beatmap_->music_->getDuration() + settings_.countdown_time)
			{
				auto current_offset = time_elapsed - settings_.countdown_time + settings_.play_offset;
				auto current_approach_offset = current_offset + settings_.approach_time;

				// Play music if it's not already playing
				if (beatmap_->music_->getStatus() != sf::Music::Playing)
					beatmap_->music_->play();

				// If sections isn't empty
				if (!sections_.empty())
				{
					// See if the next one's offset is less than the current approach offset
					auto next_section = sections_.front();
					if (current_approach_offset > next_section.offset)
					{
						current_section_.reset(new TimingSection(next_section));
						sections_.pop();
						SpawnBeat();
					}
				}
				
				if (current_section_)
				{
					if (!interpolate_beats)
					{
						if (beatqueue_)
						{
							if (!beatqueue_->empty())
							{
								auto next_beat_offset = beatqueue_->front().offset;

								if (current_approach_offset - settings_.play_offset > next_beat_offset)
								{
									SpawnBeat();
									beatqueue_->pop();
								}
							}
						}
					}
					else
					{
						beat_time -= sf::seconds(_delta_time);
						if (beat_time < sf::Time::Zero)
						{
							SpawnBeat();
							beat_time += beat_interval;
						}
					}

					if (!current_section_->notes.empty())
					{
						for (auto notequeue = current_section_->notes.begin(); notequeue != current_section_->notes.end(); ++notequeue)
						{
							int index = notequeue - current_section_->notes.begin();
							// If there are any notes remaining in the current section
							if (!notequeue->empty())
							{
								// Get the next one
								auto next_onset_offset = notequeue->front().offset;

								// If the next notes offset is less than the current approach offset
								if (current_approach_offset > next_onset_offset)
								{
									// Just spawn a random note
									if (settings_.duncan_factor)
										SpawnNote(note_paths_[rand() % note_paths_.size()]);
									else
										SpawnNote(note_paths_[index]);
									notequeue->pop();
								}
							}
						}
					}
					else
					{
						Log::Error("Notequeue vector is empty.");
					}


					if (!settings_.auto_play)
					{
						for (int index = 0; index < note_paths_.size(); ++index)
						{
							if (sfx::Global::Input.KeyPressed(settings_.keybinds[index]))
							{
								AttemptNoteHit(note_paths_[index]);
							}
						}
					}


					for (auto &beat : beats_)
					{
						beat.UpdatePosition(_delta_time);
						beat.offset_from_perfect -= sf::seconds(_delta_time);
					}
					while (!beats_.empty() && beats_.front().offset_from_perfect < sf::Time::Zero)
					{
						beats_.erase(beats_.begin());
					}

					// Update note positions and remove offscreen notes
					for (auto &path : note_paths_)
					{
						for (auto &note : path.notes)
						{
							note.UpdatePosition(_delta_time);
							note.offset_from_perfect -= sf::seconds(_delta_time);
							if (!note.VerifyPosition(machine_.window_))
							{
								// Note went offscreen, so it's a miss
								if (hit_combo_ > 10)
									PlayMissSound();
								hit_combo_ = 0;
								misses_++;
							}
							if (settings_.auto_play && note.offset_from_perfect < sf::Time::Zero)
							{
								AttemptNoteHit(path);
							}
						}
						auto path_index = path.notes.begin();
						while (path_index != path.notes.end())
						{
							if (path_index->visibility() == false)
							{
								path_index = path.notes.erase(path_index);
							}
							else
								++path_index;
						}
					}
				}
			}
			else if (time_elapsed > beatmap_->music_->getDuration() + settings_.countdown_time)
			{
				beatmap_->music_->stop();
				play_clock_.Stop();
				finished_ = true;
			}
		}

		if (!active_sounds_.empty())
		{
			for (auto iterator = active_sounds_.begin(); iterator != active_sounds_.end();)
			{
				if ((*iterator)->getStatus() == sf::Sound::Stopped)
				{
					iterator = active_sounds_.erase(iterator);
				}
				if (iterator != active_sounds_.end())
					iterator++;
			}
		}

	}

	return true;
}

void GameState::Render(const float _delta_time)
{	
	for (auto beat : beats_)
	{
		machine_.window_.draw(beat);
	}
	for (auto path : note_paths_)
	{
		machine_.window_.draw(path.target);
		for (auto note : path.notes)
		{
			machine_.window_.draw(note);
		}
	}

	auto time_elapsed = play_clock_.GetTimeElapsed();
	if (time_elapsed < settings_.countdown_time)
	{
		countdown_text_.setString("Countdown: " + agn::to_string_precise(settings_.countdown_time.asSeconds() - time_elapsed.asSeconds(), 1));
		countdown_text_.setPosition(sf::Vector2f(0.0f, window_centre.y));
		machine_.window_.draw(countdown_text_);
	}

	switch (settings_.progress_bar_position)
	{
	case GameSettings::TOPRIGHT:
		ImGui::SetNextWindowPos(ImVec2(window_size.x * 0.9f, 120));
		ImGui::SetNextWindowSize(ImVec2(window_size.x * 0.1f, 10));
		break;
	case GameSettings::ALONGTOP:
		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(window_size.x, 10));
		break;
	case GameSettings::ALONGBOTTOM:
		ImGui::SetNextWindowPos(ImVec2(0, window_size.y - 30));
		ImGui::SetNextWindowSize(ImVec2(window_size.x , 10));
		break;
	}
	
	if (!ImGui::Begin("Progress", &settings_.show_progress_bar, ImVec2(0.0f, 0.0f), 0.3f, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings))
	{
		ImGui::End();
		return;
	}
	ImGui::ProgressBar(time_elapsed / beatmap_->music_->getDuration());
	ImGui::End();

	score_text_.setString("Score: " + std::to_string(score_));
	score_text_.move(-(score_text_.getGlobalBounds().width + 15.0f), (score_text_.getGlobalBounds().top));
	machine_.window_.draw(score_text_);
	score_text_.move((score_text_.getGlobalBounds().width + 15.0f), -(score_text_.getGlobalBounds().top));

	clock_text_.setString(agn::to_string_precise(play_clock_.GetTimeElapsed().asSeconds(), 3) + "s");
	clock_text_.move(-(clock_text_.getGlobalBounds().width + 15.0f), (score_text_.getLocalBounds().height));
	machine_.window_.draw(clock_text_);
	clock_text_.move((clock_text_.getGlobalBounds().width + 15.0f), -(score_text_.getLocalBounds().height));

	
	performance_text_.setPosition(sf::Vector2f(5.0f, window_size.y - performance_y_offset * 15 - 5.0f));
	for (int index = 0; index < performance_statistic_count; ++index)
	{
		switch (index)
		{
		case 0:
			performance_text_.setString("Combo: " + std::to_string(hit_combo_));
			performance_text_.setColor(sf::Color::White);
			break;
		case 1:
			performance_text_.setString("Max Combo: " + std::to_string(max_combo_));
			performance_text_.setColor(sf::Color::White);
			performance_text_.move(0.0f, (performance_y_offset));
			break;
		case 2:
			performance_text_.setString("Hits: " + std::to_string(hits_.size()));
			performance_text_.setColor(sf::Color::White);
			break;
		case 3:
			performance_text_.setString("Perfect: " + std::to_string(perfect_hits_));
			performance_text_.setColor(sf::Color::Cyan);
			break;
		case 4:
			performance_text_.setString("Great: " + std::to_string(great_hits_));
			performance_text_.setColor(sf::Color::Green);
			break;
		case 5:
			performance_text_.setString("Good: " + std::to_string(good_hits_));
			performance_text_.setColor(sf::Color::Yellow);
			break;
		case 6:
			performance_text_.setString("Miss: " + std::to_string(misses_));
			performance_text_.setColor(sf::Color::Red);
			break;
		case 7:
			performance_text_.setString("-" + std::to_string(std::abs(earliest_hit_)) + "ms ~ +" + std::to_string(std::abs(latest_hit_)) + "ms");
			performance_text_.setColor(sf::Color::White);
			performance_text_.move(0.0f, (performance_y_offset));
			break;
		case 8:
			performance_text_.setString("Average: " + agn::to_string_precise(average_hit_, 2));
			performance_text_.setColor(sf::Color::White);
			break;
		case 9:
			performance_text_.setString("Deviation: " + agn::to_string_precise(unstable_rate_, 2));
			performance_text_.setColor(sf::Color::White);
			break;
		}
		machine_.window_.draw(performance_text_);
		performance_text_.move(0.0f, (performance_y_offset));
	}

	if (paused_)
	{
		machine_.window_.draw(pause_background_);
	}
}

void GameState::ProcessEvent(sf::Event& _event)
{
	switch (_event.type)
	{
	case sf::Event::Resized:
		window_size = sf::Vector2f(_event.size.width, _event.size.height);
		break;
	}
}

void GameState::ReloadSkin()
{}

bool GameState::PauseMenu()
{
	ImGui::SetNextWindowPos(ImVec2(window_centre.x - window_size.x * 0.2f, window_centre.y));
	ImGui::SetNextWindowSize(ImVec2(window_size.x * 0.4f, 400));
	if (!ImGui::Begin("Pause Menu", nullptr, ImVec2(0, 0), -1.f,
					  ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
					  ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings))
	{
		// Early out if the window is collapsed, as an optimization.
		ImGui::End();
		return true;
	}
	if (!finished_)
	{
		if (ImGui::Button("Resume (Escape)", ImVec2(ImGui::GetWindowContentRegionWidth(), 60)))
		{
			paused_ = false;
			ImGui::End();
			return true;
		}
	}

	if (ImGui::Button("Restart (Enter)", ImVec2(ImGui::GetWindowContentRegionWidth(), 60)))
	{
		ChangeState<GameState>(std::move(beatmap_), std::move(settings_));
		ImGui::End();
		return false;
	}

	if (ImGui::Button("Quit (Backspace)", ImVec2(ImGui::GetWindowContentRegionWidth(), 60)))
	{
		ChangeState<MenuState>(std::move(beatmap_));
		ImGui::End();
		return false;
	}

	ImGui::Spacing();

	ImGui::Separator();

	ImGui::Spacing();

	const char* hitsounds[] = { "None", "Soft", "Deep" };
	ImGui::Combo("Hitsound", &settings_.hitsound, hitsounds, 3);
	ImGui::SliderFloat("Music Volume", &settings_.music_volume, 0, 100);
	ImGui::SliderFloat("SFX Volume", &settings_.sfx_volume, 0, 100);

	ImGui::Spacing();

	const char* barpositions[] = { "Top Right", "Along Top", "Along Bottom" };
	ImGui::Combo("Progress Bar Position", &settings_.progress_bar_position, barpositions, 3);

	ImGui::End();
	return true;
}

void GameState::SpawnBeat()
{
	if (beatmap_->play_mode_ == VISUALIZATION)
	{
		beats_.emplace_back(NoteObject(sf::Vector2f(path_x + window_size.x * 0.4f, path_start_y),
									   sf::Vector2f(path_x + window_size.x * 0.4f, path_end_y),
									   settings_.approach_time,
									   beat_texture_,
									   sf::Color(255, 69, 0, 128)));
		beats_.back().SetDimensions(sf::Vector2f(window_size.x * 0.8f, 5.0f));
	}
	else
	{
		beats_.emplace_back(NoteObject(sf::Vector2f(path_x + window_size.x * 0.15f, path_start_y),
									   sf::Vector2f(path_x + window_size.x * 0.15f, path_end_y),
									   settings_.approach_time,
									   beat_texture_,
									   sf::Color(255, 69, 0, 128)));
		beats_.back().SetDimensions(sf::Vector2f(window_size.x * 0.4f, 5.0f));
	}
}

void GameState::SpawnNote(NotePath& _path)
{
	_path.notes.emplace_back(NoteObject(_path.start_position,
								  _path.target_position,
								  _path.approach_time,
								  _path.note_texture,
								  _path.note_color));
	if (settings_.path_count > 6)
		_path.notes.back().SetDimensions(sf::Vector2f(note_length, note_length));
}

void GameState::AttemptNoteHit(NotePath& _path)
{
	if (!_path.notes.empty())
	{
		auto& note = _path.notes.front();

		// Note offset is the time (in ms) that the note is offset from a perfect hit
		int note_offset = note.offset_from_perfect.asMilliseconds();
		int abs_note_offset = abs(note.offset_from_perfect.asMilliseconds());

		// Ignore press when the offset is more than 300ms
		if (abs_note_offset < 300)
		{
			if (abs_note_offset < 30)
			{
				perfect_hits_++;
				hit_combo_++;
				score_ += hit_combo_ * 300;

				PlayHitSound();
			}
			else if (abs_note_offset < 60)
			{
				great_hits_++;
				hit_combo_++;
				score_ += hit_combo_ * 100;

				PlayHitSound();
			}
			else if (abs_note_offset < 120)
			{
				good_hits_++;
				hit_combo_++;
				score_ += hit_combo_ * 50;

				PlayHitSound();
			}
			else
			{
				misses_++;
				if (hit_combo_ > max_combo_)
					max_combo_ = hit_combo_;
				if(hit_combo_ > 10)
					PlayMissSound();
				hit_combo_ = 0;
			}
			if (hit_combo_ > max_combo_)
				max_combo_ = hit_combo_;

			if (note_offset < latest_hit_)
				latest_hit_ = note_offset;
			if (note_offset > earliest_hit_)
				earliest_hit_ = note_offset;

			hits_.emplace_back(note_offset);

			auto mean = int();
			for (auto hit : hits_)
			{
				mean += hit;
			}
			average_hit_ = (float)mean / (float)hits_.size();

			std::vector<float> sqdiffs;
			for (auto hit : hits_)
			{
				auto sqdiff = float();
				sqdiff = std::pow((float)hit - average_hit_, 2);
				sqdiffs.push_back(sqdiff);
			}
			for (auto v : sqdiffs)
			{
				unstable_rate_ += v;
			}
			unstable_rate_ /= sqdiffs.size();
			unstable_rate_ = std::sqrt(unstable_rate_);

			// Set not visible (will get erased at end of update)
			note.SetVisibility(false);
		}
	}
}

void GameState::InitializeFourKeyMode()
{
	// NotePath initialization
	note_paths_.reserve(settings_.path_count);
	if (settings_.flipped)
	{
		path_start_y = window_size.y;
		path_end_y = window_size.y * 0.1f;
	}
	else
	{
		path_start_y = 0.0f;
		path_end_y = window_size.y * 0.9f;
	}
	path_x = window_size.x * 0.2f;
	path_x_offset = window_size.x * 0.1f;
	for (int index = 0; index < settings_.path_count; ++index)
	{
		sf::Color note_color;
		switch (index)
		{
		case 0:
			note_color = sf::Color::Green;
			break;
		case 1:
			note_color = sf::Color::Red;
			break;
		case 2:
			note_color = sf::Color::Cyan;
			break;
		case 3:
			note_color = sf::Color(255, 69, 0);
			break;
		}
		note_paths_.emplace_back(NotePath(sf::Vector2f(path_x + path_x_offset * index, path_start_y),
										  sf::Vector2f(path_x + path_x_offset * index, path_end_y),
										  settings_.approach_time,
										  1,
										  white_circle_texture_,
										  note_color));
	}
}

void GameState::InitializeVisualizationMode()
{
	// NotePath initialization
	note_paths_.reserve(settings_.path_count);
	if (settings_.flipped)
	{
		path_start_y = window_size.y;
		path_end_y = window_size.y * 0.1f;
	}
	else
	{
		path_start_y = 0.0f;
		path_end_y = window_size.y * 0.9f;
	}
	path_x = window_size.x * 0.1f;
	path_x_offset = window_size.x * 0.8f / settings_.path_count;
	if(settings_.path_count > 5)
		note_length = path_x_offset * 0.75f;
	else if(settings_.path_count > 10)
		note_length = path_x_offset - 5;
	for (int index = 0; index < settings_.path_count; ++index)
	{
		int hue = (int)(((float)index / (float)settings_.path_count) * 360.0f);
		sf::Color note_color(sfx::HSL(hue, 100, 50).HSLToRGB());

		note_paths_.emplace_back(NotePath(sf::Vector2f(path_x + path_x_offset * index, path_start_y),
										  sf::Vector2f(path_x + path_x_offset * index, path_end_y),
										  settings_.approach_time,
										  1,
										  white_circle_texture_,
										  note_color));
		if (settings_.path_count > 5)
			note_paths_.back().target.SetDimensions(sf::Vector2f(note_length, note_length));
	}
}

void GameState::PlayHitSound()
{
	sf::Sound* hitsound;
	switch (settings_.hitsound)
	{
	case GameSettings::SOFT:
		hitsound = new sf::Sound(*soft_hit_sound_);
		break;
	case GameSettings::DEEP:
		hitsound = new sf::Sound(*deep_hit_sound_);
		break;
	}
	if (!settings_.hitsound == GameSettings::NONE)
	{
		hitsound->setVolume(settings_.sfx_volume);
		hitsound->play();
		active_sounds_.emplace_back(hitsound);
	}
}

void GameState::PlayMissSound()
{
	if (!settings_.hitsound == GameSettings::NONE)
	{
		sf::Sound* hitsound = new sf::Sound(*miss_sound_);
		hitsound->setVolume(settings_.sfx_volume);
		hitsound->play();
		active_sounds_.emplace_back(hitsound);
	}
}
