#include "game_state.h"
#include "game_state_machine.h"
#include "menu_state.h"

namespace
{
}

GameState::GameState(GameStateMachine& _state_machine, StatePtr<AppState>& _state, std::string& _song_name) :
	AppState(_state_machine, _state)
{
	//LoadSongData(_song_name);
}

void GameState::InitializeState()
{
	textures_ = TextureFileVector
	{
		{ machine_.background_texture_, "play_background.jpg" },
		{ red_circle_texture_, "red_circle.png"},
		{ blue_circle_texture_, "blue_circle.png"},
		{ green_circle_texture_, "green_circle.png" },
		{ orange_circle_texture_, "orange_circle.png" },
		{ purple_circle_texture_, "purple_circle.png" }
	};
	Global::TextureManager.Load(textures_);

	machine_.background_.setTexture(*machine_.background_texture_);
	
	// Score Initialization
	score_ = 0;
}

void GameState::TerminateState()
{
	Global::TextureManager.Unload(textures_);
	textures_.clear();
}

bool GameState::Update(const float _delta_time)
{
	// Step box2d world variables
	/*float time_step = 0.5f / 60.0f;
	int32 velocity_iterations = 20;
	int32 position_iterations = 10;

	// Step world and handle collision detection first time
	world_->Step(time_step, velocity_iterations, position_iterations);
	CollisionDetectionLoop();

	// Update weapons and do weapon logic - MUST be done before inputloop because beam visibility is set to false automatically here
	UpdateWeapons(frame_counter);

	// Deletes non-visible objects (visibility is used to flag for deletion and rendering), updates positions and spawns new objects based on game logic
	UpdateGameObjects(frame_counter);

	// Step world and handle collision detection second time
	world_->Step(time_step, velocity_iterations, position_iterations);
	CollisionDetectionLoop();

	// Do input
	if (myInput.isKeyDown(VK_RETURN))
	{
		myInput.SetKeyUp(VK_RETURN);
		return MENU; // go to menu if start is down
	}
	else // do input loop for state if we aren't returning to menustate
		InputLoop(myInput);*/

	return true;
}

void GameState::Render(const float _delta_time)
{	
	// Explosions (rendered first to be behind everything else)
	/*for(std::vector<GameObject>::iterator explosioncounter = Explosions.begin(); explosioncounter != Explosions.end(); explosioncounter++)
	{
		if(explosioncounter->visibility() == true)
		{
			window.draw(*explosioncounter);
		}
	}
	
	// Powerups
	if(score_powerup_.visibility() == true)
	{
		window.draw(score_powerup_);
	}
	if(beam_powerup_.visibility() == true)
	{
		window.draw(beam_powerup_);
	}
	
	// Enemies
	for(std::vector<LivingObject*>::iterator enemycounter = Enemies.begin(); enemycounter != Enemies.end(); enemycounter++)
	{
		if((*enemycounter)->visibility() == true) // verify visibility
		{
			window.draw((**enemycounter));
			(*enemycounter)->DrawHealthBar(window, font);
		}
	}
	
	// Asteroids
	for(std::vector<LivingObject*>::iterator asteroidcounter = Asteroids.begin(); asteroidcounter != Asteroids.end(); asteroidcounter++)
	{
		if((*asteroidcounter)->visibility() == true) // verify visibility
		{
			window.draw((**asteroidcounter));
			(*asteroidcounter)->DrawHealthBar(window, font);
		}
	}
	
	// Photons
	for(std::vector<GameObject*>::iterator photoncounter = Photons.begin(); photoncounter != Photons.end(); photoncounter++)
	{
		if((*photoncounter)->visibility() == true) // verify visibility
		{
			window.draw((**photoncounter));
		}
	}
	
	// Projectiles
	for (auto projectile : CannonProjectiles)
	{
		if ((projectile)->visibility() == true) // verify visibility
		{
			window.draw((*projectile));
		}
	}

	// Beam
	if(beam_.visibility() == true)
	{
		window.draw(beam_);
	}
	
	// Player
	if(player_.visibility() == true)
	{
		window.draw(player_);
		player_.DrawHealthBar(window, font);

		// UI
		std::ostringstream ui_text;

		ui_text << "Cannon Ammo: " << player_cannon_.ammo_count_;
		sf::Text cannon_ammo(std::string(ui_text.str()), font, 24);
		cannon_ammo.setPosition(sf::Vector2f(0.0f, 20.0f));
		cannon_ammo.setColor(sf::Color(255, 255, 0, 255)); // yellow
		window.draw(cannon_ammo);
		std::ostringstream().swap(ui_text); // swaps with a newly default constructed stream (to clear the stream)

		ui_text << "Beam Energy: " << player_beam_.energy_;
		sf::Text beam_energy(std::string(ui_text.str()), font, 24);
		beam_energy.setPosition(sf::Vector2f(0.0f, 40.0f));
		beam_energy.setColor(sf::Color(0, 0, 255, 255)); // blue
		window.draw(beam_energy);
		std::ostringstream().swap(ui_text);

		ui_text << "Photon Recharge: " << player_photon_.current_charge_;
		sf::Text photon_recharge(std::string(ui_text.str()), font, 24);
		photon_recharge.setPosition(sf::Vector2f(0.0f, 40.0f));
		photon_recharge.setColor(sf::Color(255, 0, 255, 255)); // purple
		window.draw(photon_recharge);
		std::ostringstream().swap(ui_text);

		ui_text << "Score: " << score_;
		sf::Text score(std::string(ui_text.str()), font, 36);
		score.setPosition(sf::Vector2f(window.getSize().x * 0.4f, window.getSize().y * 0.9f));
		score.setColor(sf::Color(0, 255, 255, 255)); // teal
		window.draw(score);
		std::ostringstream().swap(ui_text);
	}
	else
	{
		// Game Over UI
		std::ostringstream game_over_text;

		game_over_text << "Final Score: " << score_;
		sf::Text final_score(std::string(game_over_text.str()), font, 48);
		final_score.setPosition(sf::Vector2f(window.getSize().x * 0.3f, window.getSize().y * 0.5f));
		final_score.setColor(sf::Color(255, 0, 0, 255)); // red
		window.draw(final_score);
		std::ostringstream().swap(game_over_text);

		sf::Text return_text("Press Start to return to menu.", font, 48);
		return_text.setPosition(sf::Vector2f(window.getSize().x * 0.2f, window.getSize().y * 0.6f));
		return_text.setColor(sf::Color(255, 255, 255, 255)); // white
		window.draw(return_text);
		std::ostringstream().swap(game_over_text);
	}
	
	sf::Text game_text("Generic Space Game", font, 24);
	game_text.setPosition(sf::Vector2f(0.0f, 0.0f));
	game_text.setColor(sf::Color(0, 255, 255, 255)); // teal
	window.draw(game_text);*/
}