#include "game_state.h"
#include "game_state_machine.h"
#include "menu_state.h"

namespace
{
	
	sf::Time play_offset = sf::seconds(3);
	sf::Time approach_time = sf::milliseconds(900);
	sf::Vector2f window_centre;

	int path_count;
	float path_start_y;
	float path_end_y;
	float path_x;
	float path_x_offset;

	const int hit_counter_statistic_count = 5;
	const int hit_counter_y_offset = 24.0f;
	
}

GameState::GameState(GameStateMachine& _state_machine, UniqueStatePtr<AppState>& _state, Beatmap* _beatmap) :
	AppState(_state_machine, _state),
	beatmap_(_beatmap),
	play_clock_(),
	paused_(false),
	finished_(false),
	hit_counters_(true),
	duncan_factor_(true),
	current_section_(nullptr)
{
}

void GameState::InitializeState()
{
	textures_ = TextureFileVector
	{
		{ machine_.background_texture_, "play_background.png" },
		{ pause_background_texture_, "pause_background.png" },
		{ white_circle_texture_, "circle_white.png" }
	};
	Global::TextureManager.Load(textures_);

	machine_.background_.setTexture(*machine_.background_texture_);

	window_centre = sf::Vector2f(machine_.window_.getSize().x * 0.5, machine_.window_.getSize().y * 0.5);

	pause_background_ = sfx::Sprite(window_centre, pause_background_texture_);

	// Clock text initialization
	clock_text_.setFont(machine_.font_);
	clock_text_.setCharacterSize(45);
	clock_text_.setColor(sf::Color::Color(255, 69, 0));
	clock_text_.setPosition(sf::Vector2f(machine_.window_.getSize().x, 0.0f));

	// Countdown text initialization
	countdown_text_.setFont(machine_.font_);
	countdown_text_.setCharacterSize(120);
	countdown_text_.setColor(sf::Color::Color(255, 69, 0));
	countdown_text_.setPosition(window_centre);

	// Score initialization
	score_text_.setFont(machine_.font_);
	score_text_.setCharacterSize(60);
	score_text_.setColor(sf::Color::Color(255, 69, 0));
	score_text_.setPosition(sf::Vector2f(machine_.window_.getSize().x, 0.0f));
	score_ = 0;

	// Hit counters initialization
	hit_counters_text_.setFont(machine_.font_);
	hit_counters_text_.setCharacterSize(30);
	hit_counters_text_.setPosition(sf::Vector2f(5.0f, machine_.window_.getSize().y - hit_counter_y_offset * 5 - 5.0f));
	perfect_hits_ = 0;
	great_hits_ = 0;
	good_hits_ = 0;
	misses_ = 0;
	hit_combo_ = 0;

	switch (beatmap_->play_mode_)
	{
	case FOURKEY:
		InitializeFourKeyMode();
		break;
	case PIANO:
		InitializePianoMode();
		break;
	default:
		InitializeFourKeyMode();
		break;
	}

	// Beatmap initialization
	sections_ = beatmap_->CopyTimingSections();

	if (!beatmap_->music_)
		beatmap_->LoadMusic();
	beatmap_->music_->setPlayingOffset(sf::Time::Zero);

	srand(time(0));
}

void GameState::TerminateState()
{
	Global::TextureManager.Unload(textures_);
	textures_.clear();

	delete current_section_;
}

bool GameState::Update(const float _delta_time)
{
	if (Global::Input.KeyPressed(Keyboard::Escape))
	{
		paused_ = !paused_;
	}

	if (paused_)
	{
		play_clock_.Stop();
		if (beatmap_->music_->getStatus() == sf::Music::Playing)
			beatmap_->music_->pause();
		if (Global::Input.KeyPressed(Keyboard::BackSpace))
		{
			ChangeState<MenuState>(beatmap_);
		}
		if (Global::Input.KeyPressed(Keyboard::Return))
		{
			ChangeState<GameState>(beatmap_);
		}
	}
	else
	{
		play_clock_.Start();

		auto time_elapsed = play_clock_.GetTimeElapsed();

		// If more than the play offset (intro time) seconds and
		// less than the duration of the music + play offset seconds has elapsed then...
		if (time_elapsed > play_offset && time_elapsed < beatmap_->music_->getDuration() + play_offset)
		{
			auto current_offset = time_elapsed - play_offset;
			auto current_approach_offset = current_offset + approach_time;

			// Play music if it's not already playing
			if(beatmap_->music_->getStatus() != sf::Music::Playing)
				beatmap_->music_->play();

			// If sections isn't empty
			if (!sections_.empty())
			{
				// See if the next one's offset is less than the current approach offset
				auto next_section = sections_.front();
				if (current_approach_offset > next_section.offset)
				{
					current_section_ = new TimingSection(next_section);
					sections_.pop();
				}
			}
			else if (current_section_ == nullptr)
			{
				Log::Error("Current timing section is nullptr. Beatmap is invalid.");
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
							if(duncan_factor_)
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

			// INPUT
			switch (beatmap_->play_mode_)
			{
			case FOURKEY:
				FourKeyInput();
				break;
			case PIANO:
				break;
			default:
				FourKeyInput();
				break;
			}


			// Update note positions and remove offscreen notes
			for (auto &path : note_paths_)
			{
				for (auto &note : path.notes)
				{
					note.UpdatePosition(_delta_time);
					note.offset_from_perfect -= sf::seconds(_delta_time);
					note.VerifyPosition(machine_.window_);
				}
				auto path_index = path.notes.begin();
				while (path_index != path.notes.end())
				{
					if (path_index->visibility() == false)
					{
						path_index = path.notes.erase(path_index);
						hit_combo_ = 0;
						misses_++;
					}
					else
						++path_index;
				}
			}
		}
		else if (time_elapsed > beatmap_->music_->getDuration() + play_offset)
		{
			beatmap_->music_->stop();
			finished_ = true;
		}
	}
	return true;
}

void GameState::Render(const float _delta_time)
{	
	for (auto path : note_paths_)
	{
		machine_.window_.draw(path.target);
		for (auto note : path.notes)
		{
			machine_.window_.draw(note);
		}
	}

	score_text_.setString("Score: " + std::to_string(score_));
	score_text_.move(-(score_text_.getGlobalBounds().width + 15.0f), (score_text_.getGlobalBounds().top));
	machine_.window_.draw(score_text_);
	score_text_.move((score_text_.getGlobalBounds().width + 15.0f), -(score_text_.getGlobalBounds().top));

	clock_text_.setString(agn::to_string_precise(play_clock_.GetTimeElapsed().asSeconds(), 3) + "s");
	clock_text_.move(-(clock_text_.getGlobalBounds().width + 15.0f), (score_text_.getLocalBounds().height));
	machine_.window_.draw(clock_text_);
	clock_text_.move((clock_text_.getGlobalBounds().width + 15.0f), -(score_text_.getLocalBounds().height));

	for (int index = 0; index < hit_counter_statistic_count; ++index)
	{
		switch (index)
		{
		case 0:
			hit_counters_text_.setString("Combo: " + std::to_string(hit_combo_));
			hit_counters_text_.setColor(sf::Color::White);
			break;
		case 1:
			hit_counters_text_.setString("Perfect: " + std::to_string(perfect_hits_));
			hit_counters_text_.setColor(sf::Color::Cyan);
			break;
		case 2:
			hit_counters_text_.setString("Great: " + std::to_string(great_hits_));
			hit_counters_text_.setColor(sf::Color::Green);
			break;
		case 3:
			hit_counters_text_.setString("Good: " + std::to_string(good_hits_));
			hit_counters_text_.setColor(sf::Color::Yellow);
			break;
		case 4:
			hit_counters_text_.setString("Miss: " + std::to_string(misses_));
			hit_counters_text_.setColor(sf::Color::Red);
			break;
		}
		machine_.window_.draw(hit_counters_text_);
		hit_counters_text_.move(0.0f, (hit_counter_y_offset));
	}
	hit_counters_text_.move(0.0f, -(hit_counter_y_offset * 5));

	if (paused_)
	{
		machine_.window_.draw(pause_background_);
	}
}

void GameState::SpawnNote(NotePath& _path)
{
	_path.notes.emplace_back(NoteObject(_path.start_position,
								  _path.target_position,
								  _path.approach_time,
								  _path.note_texture,
								  _path.note_color));
	_path.notes.back().SetDimensions(_path.target);
}

void GameState::AttemptNoteHit(NotePath& _path)
{
	if (!_path.notes.empty())
	{
		auto note = _path.notes.front();

		// Note offset is the time (in ms) that the note is offset from a perfect hit
		int note_offset = abs(note.offset_from_perfect.asMilliseconds());

		// Ignore press when the offset is more than 300ms
		if (note_offset < 300)
		{
			if (note_offset < 30)
			{
				perfect_hits_++;
				hit_combo_++;
				score_ += hit_combo_ * 300;
			}
			else if (note_offset < 80)
			{
				great_hits_++;
				hit_combo_++;
				score_ += hit_combo_ * 100;
			}
			else if (note_offset < 120)
			{
				good_hits_++;
				hit_combo_++;
				score_ += hit_combo_ * 50;
			}
			else
			{
				misses_++;
				hit_combo_ = 0;
			}

			// Erase the front note (the one we are testing)
			_path.notes.erase(_path.notes.begin());
		}
	}
}

void GameState::InitializeFourKeyMode()
{
	path_count = 4;

	// NotePath initialization
	note_paths_.reserve(path_count);
	path_start_y = 0.0f;
	path_end_y = machine_.window_.getSize().y * 0.9f;
	path_x = machine_.window_.getSize().x * 0.2f;
	path_x_offset = machine_.window_.getSize().x * 0.1f;
	for (int index = 0; index < path_count; ++index)
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
			note_color = sf::Color::Yellow;
			break;
		case 3:
			note_color = sf::Color::Blue;
			break;
		}
		note_paths_.emplace_back(NotePath(sf::Vector2f(path_x + path_x_offset * index, path_start_y),
										  sf::Vector2f(path_x + path_x_offset * index, path_end_y),
										  approach_time,
										  1,
										  white_circle_texture_,
										  note_color));
	}
}

void GameState::InitializePianoMode()
{
	path_count = 88;

	// NotePath initialization
	note_paths_.reserve(path_count);
	path_start_y = 0.0f;
	path_end_y = machine_.window_.getSize().y * 0.9f;
	path_x = machine_.window_.getSize().x * 0.2f;
	float length = (machine_.window_.getSize().x - path_x - path_x) / path_count;
	path_x_offset = length + 2.0f;

	for (int index = 0; index < path_count; ++index)
	{
		sf::Color note_color;
		switch (index%2)
		{
		case 0:
			note_color = sf::Color::White;
			break;
		case 1:
			note_color = sf::Color::Black;
			break;
		}
		note_paths_.emplace_back(NotePath(sf::Vector2f(path_x + path_x_offset * index, path_start_y),
										  sf::Vector2f(path_x + path_x_offset * index, path_end_y),
										  approach_time,
										  1,
										  white_circle_texture_,
										  note_color));
		
		note_paths_[index].target.SetDimensions(sf::Vector2f(length, length));
	}
}

void GameState::FourKeyInput()
{
	if (Global::Input.KeyPressed(Keyboard::Z))
	{
		AttemptNoteHit(note_paths_[0]);
	}
	if (Global::Input.KeyPressed(Keyboard::X))
	{
		AttemptNoteHit(note_paths_[1]);
	}
	if (Global::Input.KeyPressed(Keyboard::N))
	{
		AttemptNoteHit(note_paths_[2]);
	}
	if (Global::Input.KeyPressed(Keyboard::M))
	{
		AttemptNoteHit(note_paths_[3]);
	}
}
