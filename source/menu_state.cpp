#include "menu_state.h"
#include "game_application.h"
#include "input.h"


MenuState::MenuState(const GameApplication* application) :
	AppState(application)
{
}


MenuState::~MenuState()
{
}

void MenuState::InitializeState()
{
	LoadTextures();

	// calculate window dimensions and centre
	float window_width = (float)application_->getWindow().getSize().x;
	float window_height = (float)application_->getWindow().getSize().y;
	sf::Vector2f window_centre(window_width / 2.0f, window_height / 2.0f);
	
	// Start Button
	start_button_ = GameObject();	// construct start button
	start_button_.initSprite(window_centre, start_texture_);
	start_texture_.setRepeated(true);
	auto start_rect = start_button_.getTextureRect();
	start_button_.setTextureRect(sf::IntRect(start_rect.left - start_rect.width, start_rect.top, -start_rect.width, start_rect.height));
	auto end_rect = start_button_.getTextureRect();

	// Sound Controls
	float sound_button_spacing = window_width * 0.2f;
	music_button_ = GameObject();	// construct music button
	music_button_.initSprite(window_centre, music_on_texture_);
	music_button_.MoveBy(-0.5f * sound_button_spacing, -(window_height * 0.2f)); // moves left by half the button spacing and up by 1/5th the platform height
	sound_effects_button_ = GameObject();	// construct sound effects button
	sound_effects_button_.initSprite(music_button_.getPosition(), sound_on_texture_);
	sound_effects_button_.MoveBy(sound_button_spacing, 0.0f);
	
	// Difficulty Buttons
	// Each buttons position is relative to the last buttons with the exception
	// of the first which is moved to position relative to the centre of the screen.
	float diff_button_spacing = (window_width * 0.5f) / (float)kDifficultySettings;
	difficulty_button[0] = GameObject();	// construct first difficulty button
	difficulty_button[0].initSprite(window_centre, green_square_);
	difficulty_button[0].MoveBy(-((float)kDifficultySettings / 2.5f) * diff_button_spacing, window_height * 0.3f); // moves left by half the number of buttons times spacing and down by 1/5th the platform height
	for(int i = 1; i < kDifficultySettings; i++)
	{
		difficulty_button[i] = GameObject();
		difficulty_button[i].initSprite(difficulty_button[i - 1].getPosition(), blue_square_);
		difficulty_button[i].MoveBy(diff_button_spacing, 0.0f);
	}
	difficulty_button[2].setTexture(yellow_diamond_, true); // set textures to correct textures (2nd button texture is used initially for all buttons after first)
	difficulty_button[3].setTexture(red_diamond_, true);
	difficulty_button[4].setTexture(purple_polygon_, true);
	
	// Selection Pointer
	// Initialized to screen centre position(irrelevant as it gets changed to correct position in update loop before render)
	selection_pointer_ = GameObject();
	selection_pointer_.initSprite(window_centre, selector_);
	
	// Dpad
	dpad_ = GameObject();
	dpad_.initSprite(sf::Vector2f(window_width * 0.1f, window_height * 0.8f), dpad_texture_);
	
	// Cross
	cross_ = GameObject();
	cross_.initSprite(sf::Vector2f(window_width * 0.9f, window_height * 0.8f), cross_texture_);

	selection_ = STARTGAME; // starts selection as startgame button

	// NOTE - Visibility is not set to true for any sprites (it's default initialized to false) as the render function of
	// this state won't check for visibility before rendering anyway because the whole menu will be rendered every frame.
}

void MenuState::TerminateState()
{
	// state cleanup
}

GAMESTATE MenuState::Update(const int& frame_counter, Input& myInput)
{
	switch (selection_) // do input based on what button is currently selected
	{
	case STARTGAME:
		if (myInput.isKeyDown(VK_SPACE))	// if spacebar is pressed while startgame is selected
		{
			myInput.SetKeyUp(VK_SPACE);
			return GAME;					// return state to change to as GAME
		}
		if (myInput.isKeyDown('S'))		// if S is pressed while startgame is selected
		{
			myInput.SetKeyUp('S');
			selection_ = DIFFICULTY;	// change selection to difficulty
		}
		if (myInput.isKeyDown('W'))			// if W is pressed while startgame is selected
		{
			myInput.SetKeyUp('W');
			selection_ = MUSIC;				// change selection to music
		}
		selection_pointer_.MoveTo(start_button_.getPosition().x, start_button_.getPosition().y);
		selection_pointer_.setDimensions(start_button_);
		break;
	case MUSIC:
		if (myInput.isKeyDown(VK_SPACE))	// if cross is pressed while music is selected
		{
			myInput.SetKeyUp(VK_SPACE);
			switch (application_->settings_.music_)
			{
			case true:
				application_->settings_.music_ = false;				// set music to false if true
				music_button_.setTexture(music_off_texture_);
				break;
			case false:
				application_->settings_.music_ = true;				// set music to true if false
				music_button_.setTexture(music_on_texture_);
				break;
			}
		}
		if (myInput.isKeyDown('D'))	// if right is pressed while music is selected
		{
			myInput.SetKeyUp('D');
			selection_ = SOUNDEFFECTS;								// change selection to soundeffects
		}
		if (myInput.isKeyDown('S'))	// if down is pressed while music is selected
		{
			myInput.SetKeyUp('S');
			selection_ = STARTGAME;									// change selection to startgame
		}
		selection_pointer_.MoveTo(music_button_.getPosition().x, music_button_.getPosition().y);
		selection_pointer_.setDimensions(music_button_);
		break;
	case SOUNDEFFECTS:
		if (myInput.isKeyDown(VK_SPACE))	// if cross is pressed while soundeffects is selected
		{
			myInput.SetKeyUp(VK_SPACE);
			switch (application_->settings_.sound_effects_)
			{
			case true:
				application_->settings_.sound_effects_ = false;		// set sound_effects_ to false if true
				sound_effects_button_.setTexture(sound_off_texture_);
				break;
			case false:
				application_->settings_.sound_effects_ = true;		// set sound_effects_ to true if false
				sound_effects_button_.setTexture(sound_on_texture_);
				break;
			}
		}
		if (myInput.isKeyDown('S'))	// if down is pressed while soundeffects is selected
		{
			myInput.SetKeyUp('S');
			selection_ = STARTGAME;									// change selection to startgame
		}
		if (myInput.isKeyDown('A'))	// if left is pressed while soundeffects is selected
		{
			myInput.SetKeyUp('A');
			selection_ = MUSIC;										// change selection to music
		}
		selection_pointer_.MoveTo(sound_effects_button_.getPosition().x, sound_effects_button_.getPosition().y);
		selection_pointer_.setDimensions(sound_effects_button_);
		break;
	case DIFFICULTY:
		switch (application_->settings_.difficulty_) // do input based on which difficulty is selected
		{
		case GameSettings::EASY:
			if (myInput.isKeyDown('D'))
			{
				myInput.SetKeyUp('D');
				application_->settings_.difficulty_ = GameSettings::MEDIUM;
			}
			selection_pointer_.setDimensions(difficulty_button[0]);
			selection_pointer_.MoveTo(difficulty_button[0].getPosition().x, difficulty_button[0].getPosition().y);
			break;
		case GameSettings::MEDIUM:
			if (myInput.isKeyDown('A'))
			{
				myInput.SetKeyUp('A');
				application_->settings_.difficulty_ = GameSettings::EASY;
			}
			if (myInput.isKeyDown('D'))
			{
				myInput.SetKeyUp('D');
				application_->settings_.difficulty_ = GameSettings::HARD;
			}
			selection_pointer_.setDimensions(difficulty_button[1]);
			selection_pointer_.MoveTo(difficulty_button[1].getPosition().x, difficulty_button[1].getPosition().y);
			break;
		case GameSettings::HARD:
			if (myInput.isKeyDown('A'))
			{
				myInput.SetKeyUp('A');
				application_->settings_.difficulty_ = GameSettings::MEDIUM;
			}
			if (myInput.isKeyDown('D'))
			{
				myInput.SetKeyUp('D');
				application_->settings_.difficulty_ = GameSettings::EXTREME;
			}
			selection_pointer_.setDimensions(difficulty_button[2]);
			selection_pointer_.MoveTo(difficulty_button[2].getPosition().x, difficulty_button[2].getPosition().y);
			break;
		case GameSettings::EXTREME:
			if (myInput.isKeyDown('A'))
			{
				myInput.SetKeyUp('A');
				application_->settings_.difficulty_ = GameSettings::HARD;
			}
			if (myInput.isKeyDown('D'))
			{
				myInput.SetKeyUp('D');
				application_->settings_.difficulty_ = GameSettings::TRANSCENDENT;
			}
			selection_pointer_.setDimensions(difficulty_button[3]);
			selection_pointer_.MoveTo(difficulty_button[3].getPosition().x, difficulty_button[3].getPosition().y);
			break;
		case GameSettings::TRANSCENDENT:
			if (myInput.isKeyDown('A'))
			{
				myInput.SetKeyUp('A');
				application_->settings_.difficulty_ = GameSettings::EXTREME;
			}
			selection_pointer_.setDimensions(difficulty_button[4]);
			selection_pointer_.MoveTo(difficulty_button[4].getPosition().x, difficulty_button[4].getPosition().y);
			break;
		}
		if (myInput.isKeyDown('W'))		// if up is pressed while difficulty is selected
		{
			myInput.SetKeyUp('W');
			selection_ = STARTGAME;									// change selection to gamestart
		}
		break;
	}
	return MENU;
}

void MenuState::Render(sf::Font& font, sf::RenderWindow& window)
{
	float window_width = (float)application_->getWindow().getSize().x;
	float window_height = (float)application_->getWindow().getSize().y;
	sf::Vector2f window_centre(window_width * 0.5f, window_height * 0.5f);

	// Start Button
	window.draw(start_button_);

	// Sound Buttons
	window.draw(music_button_);
	sf::Text music_text;
	music_text.setFont(font);
	music_text.setCharacterSize(24);
	music_text.setString("Music:");
	music_text.setColor(sf::Color(255, 255, 255, 255)); // white
	music_text.setPosition(sf::Vector2f(music_button_.getPosition().x - (music_button_.getLocalBounds().width * 0.5f), music_button_.getPosition().y - (music_button_.getLocalBounds().height)));
	window.draw(music_text);

	window.draw(sound_effects_button_);
	sf::Text sound_text;
	sound_text.setFont(font);
	sound_text.setCharacterSize(24);
	sound_text.setString("Sound Effects:");
	sound_text.setColor(sf::Color(255, 255, 255, 255)); // white
	sound_text.setPosition(sf::Vector2f(sound_effects_button_.getPosition().x - (sound_effects_button_.getLocalBounds().width * 0.5f), sound_effects_button_.getPosition().y - (sound_effects_button_.getLocalBounds().height)));
	window.draw(sound_text);

	// Difficulty Buttons
	for(int i = 0; i < kDifficultySettings; i++)
	{
		window.draw(difficulty_button[i]);
	}
	// Difficulty Text
	sf::Text diff_text;
	diff_text.setFont(font);
	diff_text.setCharacterSize(24);
	diff_text.setPosition(sf::Vector2f(window_width * 0.35f, difficulty_button[0].getPosition().y - 2.0f * (difficulty_button[0].getLocalBounds().height)));
	switch(application_->settings_.difficulty_)
	{
	case GameSettings::EASY:
		diff_text.setString("Current Difficulty: Easy");
		diff_text.setColor(sf::Color(0, 255, 0, 255)); // green
		break;
	case GameSettings::MEDIUM:
		diff_text.setString("Current Difficulty: Medium");
		diff_text.setColor(sf::Color(0, 0, 255, 255)); // blue
		break;
	case GameSettings::HARD:
		diff_text.setString("Current Difficulty: Hard");
		diff_text.setColor(sf::Color(255, 255, 0, 255)); // yellow
		break;
	case GameSettings::EXTREME:
		diff_text.setString("Current Difficulty: Extreme");
		diff_text.setColor(sf::Color(255, 0, 0, 255)); // red
		break;
	case GameSettings::TRANSCENDENT:
		diff_text.setString("Current Difficulty: Transcendent");
		diff_text.setColor(sf::Color(255, 0, 255, 255)); // purple
		break;
	}
	window.draw(diff_text);

	// Selection Pointer
	window.draw(selection_pointer_);
	
	// Dpad
	window.draw(dpad_);
	
	// Cross
	window.draw(cross_);
}

void MenuState::LoadTextures()
{
	start_texture_.loadFromFile("bin/transparent_start.png");
	cross_texture_.loadFromFile("bin/transparent_cross.png");
	music_on_texture_.loadFromFile("bin/transparent_music_on.png");
	music_off_texture_.loadFromFile("bin/transparent_music_off.png");
	sound_on_texture_.loadFromFile("bin/transparent_sound_on.png");
	sound_off_texture_.loadFromFile("bin/transparent_sound_off.png");
	dpad_texture_.loadFromFile("bin/transparent_dpad.png");
	blue_square_.loadFromFile("bin/element_blue_square.png");
	green_square_.loadFromFile("bin/element_green_square.png");
	yellow_diamond_.loadFromFile("bin/element_yellow_diamond.png");
	red_diamond_.loadFromFile("bin/element_red_diamond.png");
	purple_polygon_.loadFromFile("bin/element_purple_polygon.png");
	selector_.loadFromFile("bin/selector.png");
}