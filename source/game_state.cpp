#include "game_state.h"
#include <sstream>
#include "game_application.h"
#include "box2d_helpers.h"
#include "input.h"

GameState::GameState(const GameApplication* application) :
	AppState(application)
{
}


GameState::~GameState()
{
}

void GameState::InitializeState()
{
	LoadTextures();
	//LoadSounds();

	// create new box2d world with zero gravity
	world_ = new b2World(b2Vec2(0.0f, 0.0f));

	// player properties
	float player_start_position_x = application_->getWindow().getSize().x*0.1f;
	float player_start_position_y = application_->getWindow().getSize().y*0.5f;
	float player_width = 47.0f;
	float player_height = 32.0f;

	// player box2d body properties
	b2BodyDef playerbody;
	playerbody.type = b2_kinematicBody;
	playerbody.position = b2Vec2(GFX_BOX2D_POS_X(player_start_position_x), GFX_BOX2D_POS_Y(player_start_position_y)); // starting position 1/10th screen width and 1/2 screen height
	playerbody.linearDamping = 0.9f; // very high damping to make movement feel smooth
	playerbody.fixedRotation = true; // player won't rotate based on physics
	b2PolygonShape playerbox;
	playerbox.SetAsBox(GFX_BOX2D_SIZE(player_width) * 0.5f, GFX_BOX2D_SIZE(player_height) * 0.5f);
	b2FixtureDef playerfixture;
	playerfixture.shape = &playerbox;
	playerfixture.density = 5.0f; // very high density
	playerfixture.friction = 0.1f; // standard friction
	playerfixture.restitution = 0.2f; // not very bouncy (it's a spaceship after all)

	// player livingobject properties
	player_ = LivingObject(); // constructs player livingobject
	player_.AddBody(world_, playerbody, playerfixture); // adds its box2d body and fixture
	player_.initSprite(sf::Vector2f(player_start_position_x, player_start_position_y), player_texture_); // init sprite properties
	player_.setDimensions(sf::Vector2f(player_width, player_height));
	player_.RotateTo(90.0f);
	player_.set_type(GameObject::PLAYER); // set it as player object
	player_.set_visibility(true); // make visible

	// adds a message to the message queue to sync client frame counters with server (for when the server enters gamestate while clients are connected, resets the clients counter)
	NetMessage framemessage;
	framemessage.type = MT_WELCOME;
	framemessage.frame = application_->getFrame();
	GameMessages.push(framemessage);

	NetMessage settings;
	settings.type = MT_SETTINGS;
	settings.data = application_->settings_.difficulty_;
	GameMessages.push(settings);
	
	// Cannon Initialization
	CannonProjectiles = std::vector<GameObject*>(); // init vector array
	player_cannon_.ammo_count_ = 10;
	player_cannon_.max_ammo_count_ = 10;
	player_cannon_.damage_ = 80.0f;
	player_cannon_.recharge_time_ = 30; // in frames (assuming 60 fps)
	
	// Photon Initialization
	Photons = std::vector<GameObject*>(); // init vector array
	player_photon_.current_charge_ = 100.0f;
	player_photon_.recharge_rate_ = 100.0f/(60.0f * 2.0f); // 0.83 charge per frame - 120 frames to reach 100 charge (2 seconds)
	player_photon_.damage_ = 200.0f;
	
	// Beam Initialization (No body initialization - use default physics instead of box2d)
	/*beam_ = GameObject();
	beam_.initSprite(sf::Vector2f(player_.getPosition().x, player_.getPosition().y), beam_texture_);
	player_beam_.energy_ = 100.0f;			// starting energy
	player_beam_.energy_consumption_rate_ = 0.55f;	// 0.55 energy/frame - 33 energy/s - 3s deplete time - 6.67 taking recharge into account
	player_beam_.energy_recharge_rate_ = 0.3f; // 0.3 energy/frame generated - 18 energy/s - 5.55s for full charge or 6.67s for deplete when firing
	player_beam_.overcharged_ = false;		// obviously you don't start with the super-overpowered beam mode :D
	player_beam_.overheat_ = false;			// overheats if energy reaches 0, stops overheating when max_energy is recharged
	player_beam_.damage_ = 	3.33f;			// 3.33 damage/frame - 200.000000001 dps to the enemy (assuming 60 fps)*/
	
	// Enemies Initialization
	Enemies = std::vector<LivingObject*>(); // init vector array
	
	// Asteroids Initialization
	Asteroids = std::vector<LivingObject*>(); // init vector array

	// Explosion Initialization
	//Explosions = std::vector<GameObject>(); // init vector array
	
	// Powerups Initialization (No body initialization - use default physics instead of box2d)
	score_powerup_ = GameObject();
	score_powerup_.initSprite(sf::Vector2f(player_.getPosition().x, player_.getPosition().y), score_powerup_texture_);
	score_powerup_.AccelerateTo(sf::Vector2f(-6.0f, 0.0f));
	score_powerup_.set_type(GameObject::SCORE_POWERUP);
	/*beam_powerup_ = GameObject();
	beam_powerup_.initSprite(sf::Vector2f(player_.getPosition().x, player_.getPosition().y), beam_powerup_texture_);
	beam_powerup_.AccelerateTo(sf::Vector2f(-6.0f, 0.0f));*/
	
	// Score Initialization
	score_ = 0;
}

void GameState::TerminateState()
{

	//audio_manager_->UnloadSample(explosion_sfx_);
	//audio_manager_->UnloadSample(photon_sfx_);

	for(int enemycounter = 0; enemycounter < Enemies.size(); enemycounter++) // for every enemy
	{
		Enemies[enemycounter]->DestroyBody();	// destroy its box2d body
		delete Enemies[enemycounter];			// delete it
	}
	Enemies.clear();	// clear enemy vector

	for(int asteroidcounter = 0; asteroidcounter < Asteroids.size(); asteroidcounter++) // for every asteroid
	{
		Asteroids[asteroidcounter]->DestroyBody();	// destroy its box2d body
		delete Asteroids[asteroidcounter];			// delete it
	}
	Asteroids.clear();	// clear asteroid vector
	
	for(int projectilecounter = 0; projectilecounter < CannonProjectiles.size(); projectilecounter++) // for every cannon projectile
	{
		CannonProjectiles[projectilecounter]->DestroyBody();	// destroy its box2d body
		delete CannonProjectiles[projectilecounter];			// delete it
	}
	CannonProjectiles.clear();	// clear cannon projectiles vector

	for(int photoncounter = 0; photoncounter < Photons.size(); photoncounter++) // for every photon
	{
		Photons[photoncounter]->DestroyBody();	// destroy its box2d body
		delete Photons[photoncounter];			// delete it
	}
	Photons.clear(); // clear photons vector

	player_.DestroyBody();

	delete world_;
	world_ = NULL;
}

GAMESTATE GameState::Update(const int& frame_counter, Input& myInput)
{
	// Step box2d world variables
	float time_step = 0.5f / 60.0f;
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
		InputLoop(myInput);

	return GAME; // else continue with gamestate
}

void GameState::Render(sf::Font& font, sf::RenderWindow& window)
{	
	// Explosions (rendered first to be behind everything else)
	/*for(std::vector<GameObject>::iterator explosioncounter = Explosions.begin(); explosioncounter != Explosions.end(); explosioncounter++)
	{
		if(explosioncounter->visibility() == true)
		{
			window.draw(*explosioncounter);
		}
	}*/
	
	// Powerups
	if(score_powerup_.visibility() == true)
	{
		window.draw(score_powerup_);
	}
	/*if(beam_powerup_.visibility() == true)
	{
		window.draw(beam_powerup_);
	}*/
	
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
	/*if(beam_.visibility() == true)
	{
		window.draw(beam_);
	}*/
	
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

		/*ui_text << "Beam Energy: " << player_beam_.energy_;
		sf::Text beam_energy(std::string(ui_text.str()), font, 24);
		beam_energy.setPosition(sf::Vector2f(0.0f, 40.0f));
		beam_energy.setColor(sf::Color(0, 0, 255, 255)); // blue
		window.draw(beam_energy);
		std::ostringstream().swap(ui_text);*/

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
	window.draw(game_text);
}

void GameState::LoadTextures()
{
	player_texture_.loadFromFile("bin/player.png");
	enemy_texture_.loadFromFile("bin/enemy.png");
	orange_cannon_projectile_texture_.loadFromFile("bin/orange_cannon_projectile.png");
	asteroid_texture_.loadFromFile("bin/small_asteroid_grey.png");
	//beam_texture_.loadFromFile("bin/beam.png");
	//overcharged_beam_texture_.loadFromFile("bin/overcharged_beam.png");
	photon_texture_.loadFromFile("bin/photon.png");
	//explosion_texture_.loadFromFile("bin/explosion.png");
	score_powerup_texture_.loadFromFile("bin/star_gold.png");
	//beam_powerup_texture_.loadFromFile("bin/bolt_gold.png");
}

/*void GameState::LoadSounds()
{
	explosion_sfx_ = audio_manager_->LoadSample("explosion.wav", platform_);
	photon_sfx_ = audio_manager_->LoadSample("photon.wav", platform_);
}*/

void GameState::InputLoop(Input& myInput)
{
	player_.AccelerateTo(sf::Vector2f(0.0f, 0.0f)); // reset player velocity to 0 to begin with (discard previous velocity)

	if (myInput.isKeyDown('W')) // while up button is down
	{
		if(player_.visibility() == true) // if player is visible
		{
			if (player_.getPosition().y > 0.0f) // can only move up if player will not exceed screen boundary
			{
				player_.AccelerateBy(sf::Vector2f(0.0f, player_speed_)); // add 6.0f in up direction to acceleration
			}
		}
	}

	if (myInput.isKeyDown('D')) // while right button is down
	{
		if(player_.visibility() == true) // if player is visible
		{
			if (player_.getPosition().x < application_->getWindow().getSize().x * 0.5f) // can only move right if player wull not exceed 0.5 * screen width
			{
				player_.AccelerateBy(sf::Vector2f(player_speed_, 0.0f)); // add 6.0f in right direction to acceleration
			}
		}
	}

	if (myInput.isKeyDown('S')) // while down button is down
	{
		if(player_.visibility() == true) // if player is visible
		{
			if (player_.getPosition().y < application_->getWindow().getSize().y) // can only move down if player will not exceed screen boundary
			{
				player_.AccelerateBy(sf::Vector2f(0.0f, -player_speed_)); // add 6.0f in down direction to acceleration
			}
		}
	}

	if (myInput.isKeyDown('A')) // while left button is down
	{
		if(player_.visibility() == true) // if player is visible
		{
			if (player_.getPosition().x > 0.0f) // can only move left if player will not exceed screen boundary
			{
				player_.AccelerateBy(sf::Vector2f(-player_speed_, 0.0f)); // add 6.0f in left direction to acceleration
			}
		}
	}

	if (myInput.isMouseDown(Input::MouseButtons::LEFT)) // when LMB gets pressed
	{
		myInput.SetButtonUp(Input::MouseButtons::LEFT);
		if(player_.visibility() == true)
		{
			if(player_cannon_.ammo_count_ > 0) // requires ammo
			{
				player_cannon_.ammo_count_ -= 1; // reduce ammo by 1
				SpawnCannonProjectile();

				// adds a message to the queue to update clients with cannon ammo
				NetMessage projectile;
				projectile.type = MT_AMMOMESSAGE;
				projectile.frame = application_->getFrame();
				projectile.data = player_cannon_.ammo_count_;
				GameMessages.push(projectile);
			}
		}
	}

	if (myInput.isKeyDown(VK_SPACE)) // when space gets pressed
	{
		myInput.SetKeyUp(VK_SPACE);
		if(player_.visibility() == true)
		{
			if(player_photon_.current_charge_ >= player_photon_.full_charge())
			{
				SpawnPhoton();
				player_photon_.current_charge_ = 0.0f;

				// adds a message to the queue to update clients with photon charge
				NetMessage photon;
				photon.type = MT_PHOTONMESSAGE;
				photon.frame = application_->getFrame();
				photon.data = 0;
				GameMessages.push(photon);
			}
		}
	}

	/*if (myInput.isMouseDown(Input::MouseButtons::RIGHT)) // while right shoulder is down
	{
		if(player_.visibility() == true) // if player is visible
		{
			if(player_beam_.overcharged_ == true) // if beam is overcharged
			{
				beam_.setDimensions(sf::Vector2f(beam_.getLocalBounds().width, 24.0f)); // set to 24.0f height (double height)
				beam_.setTexture(overcharged_beam_texture_); // set to overcharged texture
				beam_.set_visibility(true); // set visible
				player_beam_.energy_ -= player_beam_.energy_consumption_rate_; // consume beam energy
				if(player_beam_.energy_ <= 0.0f) // if the energy goes below 0
				{
					player_beam_.overcharged_ = false;	// beam is out of energy (used up the powerup)
					player_beam_.energy_ = 100.0f;		// adds max energy back at end of powerup
				}
			}
			else if((player_beam_.energy_ > 0.0f) && (player_beam_.overheat_ == false)) // needs energy to fire and must not be overheating
			{
				beam_.setDimensions(sf::Vector2f(beam_.getLocalBounds().width, 12.0f)); // set to 12.0f height (normal height)
				beam_.setTexture(beam_texture_); // set to normal texture
				beam_.set_visibility(true); // sets beam to visible (flag for collision detection and rendering)
				player_beam_.energy_ -= player_beam_.energy_consumption_rate_; // consume beam energy
				if(player_beam_.energy_ <= 0.0f) // if the energy goes below 0
				{
					player_beam_.overheat_ = true;	// beam is overheating
				}
			}
		}
	}*/
}

void GameState::UpdateWeapons(const int& frame_counter)
{
	// Beam Logic
	/*beam_.set_visibility(false); // Beam defaults to not firing every frame
	if(player_beam_.energy_ < player_beam_.max_energy())
	{
		if(player_beam_.overcharged_ == false) // if beam isn't overcharged
		{
			player_beam_.energy_ += player_beam_.energy_recharge_rate_; // recharge beam energy if its not full
		}
		if(player_beam_.energy_ > player_beam_.max_energy())		// prevent energy going over max_energy
		{
			player_beam_.energy_ = player_beam_.max_energy();
			player_beam_.overheat_ = false;						// beam stops overheating when it reaches max_energy
		}
	}*/
	
	// Photon Logic
	if(player_photon_.current_charge_ < player_photon_.full_charge()) // if photon has not recharged
	{
		player_photon_.current_charge_ += player_photon_.recharge_rate_; // generates recharge_rate_ charge per frame
		if(player_photon_.current_charge_ > player_photon_.full_charge())
			player_photon_.current_charge_ = player_photon_.full_charge(); // stops value from going over full charge
	}
	
	// Cannon Logic
	if ((frame_counter%player_cannon_.recharge_time_ == 0) && (player_cannon_.ammo_count_ < player_cannon_.max_ammo_count_)) // entered when player isn't at max ammo and the frame counter is a factor of the cannon recharge time in frames
	{
		player_cannon_.ammo_count_ += 1; // generate 1 cannon ammo
	}
}

void GameState::UpdateGameObjects(const int& frame_counter)
{
	if(player_.visibility() == true)
	{
		player_.UpdatePosition(); // updates position according to current velocity
		if (player_.getPosition().y < 0.0f) // if player moved offscreen
		{
			player_.MoveTo(player_.getPosition().x, 0.0f); // move player back onscreen
		}
		if (player_.getPosition().x > application_->getWindow().getSize().x * 0.5f) // if player moved further than 0.5 * screen width
		{
			player_.MoveTo(application_->getWindow().getSize().x * 0.5f, player_.getPosition().y); // move player back onscreen
		}
		if (player_.getPosition().y > application_->getWindow().getSize().y) // if player moved offscreen
		{
			player_.MoveTo(player_.getPosition().x, application_->getWindow().getSize().y); // move player back onscreen
		}
		if (player_.getPosition().x < 0.0f) // if player moved offscreen
		{
			player_.MoveTo(0.0f, player_.getPosition().y); // move player back onscreen
		}
		
		
		if (!player_history_.velocity_.empty()) // if there is a previous velocity (avoid unintialized access at first frame)
		{
			sf::Vector2f previous_velocity = player_history_.velocity_.front();		// retreive the previous velocity
			player_history_.velocity_.pop();										// remove it from the history queue
			if (previous_velocity != player_.velocity())							// if previous velocity is different from 
			{
				NetMessage playermessage;											// send a message to update player
				playermessage.type = MT_GAMEMESSAGE;
				playermessage.data = player_.type();
				playermessage.x = player_.getPosition().x;
				playermessage.y = player_.getPosition().y;
				playermessage.frame = application_->getFrame();
				playermessage.velo_x = player_.velocity().x;
				playermessage.velo_y = player_.velocity().y;
				playermessage.health = player_.health();
				GameMessages.push(playermessage);
			}
		}
		player_history_.velocity_.push(player_.velocity()); // push current velocity to velocity history queue

		/*NetMessage playermessage; // send a message to update player
		playermessage.type = MT_GAMEMESSAGE;
		playermessage.data = player_.type();
		playermessage.x = player_.getPosition().x;
		playermessage.y = player_.getPosition().y;
		playermessage.frame = application_->getFrame();
		playermessage.velo_x = player_.velocity().x;
		playermessage.velo_y = player_.velocity().y;
		playermessage.health = player_.health();
		GameMessages.push(playermessage);*/
	}
	else if(player_.visibility() == false) // if player has died (result of player not being visible)
	{
		player_.MoveTo(-100.0f, 0.0f); // moves the position (and therefore box2d body) offscreen - prevents needing to add player visibility verification to collision detection
	}

	for(std::vector<LivingObject*>::iterator enemycounter = Enemies.begin(); enemycounter != Enemies.end();) 	// for every enemy
	{
		(*enemycounter)->VerifyPosition(application_->getWindow());	// verifies position on screen
		if((*enemycounter)->visibility() == true) 	// if it is visible
		{
			(*enemycounter)->UpdatePosition(); 	// update position
			enemycounter++;						// increments for loop
		}
		else if((*enemycounter)->visibility() == false) // else if its not
		{
			(*enemycounter)->DestroyBody();
			delete (*enemycounter);
			enemycounter = Enemies.erase(enemycounter);	// delete it and move enemycounter to element after deletion
		}
	}

	for(std::vector<LivingObject*>::iterator asteroidcounter = Asteroids.begin(); asteroidcounter != Asteroids.end();) 	// for every asteroid
	{
		(*asteroidcounter)->VerifyPosition(application_->getWindow());	// verifies position on screen
		if((*asteroidcounter)->visibility() == true) 	// if it is visible
		{
			(*asteroidcounter)->UpdatePosition();		// update position
			asteroidcounter++;						// increments for loop
		}
		else if((*asteroidcounter)->visibility() == false) // else if its not
		{
			(*asteroidcounter)->DestroyBody();
			delete (*asteroidcounter);
			asteroidcounter = Asteroids.erase(asteroidcounter);	// delete it and move asteroidcounter to element after deletion
		}
	}

	for(std::vector<GameObject*>::iterator projectilecounter = CannonProjectiles.begin(); projectilecounter != CannonProjectiles.end();) 	// for every cannon projectile
	{
		(*projectilecounter)->VerifyPosition(application_->getWindow());	// verifies position on screen
		if((*projectilecounter)->visibility() == true) // if it is visible
		{
			(*projectilecounter)->UpdatePosition();	// update position
			projectilecounter++;					// increment for loop
		}
		else if((*projectilecounter)->visibility() == false)	// else if its not
		{
			(*projectilecounter)->DestroyBody();
			delete (*projectilecounter);
			projectilecounter = CannonProjectiles.erase(projectilecounter); // delete it and move projectilecounter to element after deletion
		}
	}

	for(std::vector<GameObject*>::iterator photoncounter = Photons.begin(); photoncounter != Photons.end();) 	// for every photon
	{
		(*photoncounter)->VerifyPosition(application_->getWindow());	// verifies position on screen
		if((*photoncounter)->visibility() == true) // if it is visible
		{
			(*photoncounter)->UpdatePosition();	// update position
			photoncounter++;					// increment for loop
		}
		else if((*photoncounter)->visibility() == false)	// else if its not
		{
			(*photoncounter)->DestroyBody();
			delete (*photoncounter);
			photoncounter = Photons.erase(photoncounter); // delete it and move photoncounter to element after deletion
		}
	}

	/*for(std::vector<GameObject>::iterator explosioncounter = Explosions.begin(); explosioncounter != Explosions.end();) 	// for every explosion
	{
		if(explosioncounter->Animate(ticks_) == true)	// animate explosion, if the animation has finished
		{
			explosioncounter = Explosions.erase(explosioncounter);	// delete the explosion and move explosioncounter to the element after deletion
		}
		else
		{
			explosioncounter++;	// increment for loop if nothing is deleted
		}
	}*/

	if(score_powerup_.visibility() == true)
	{
		score_powerup_.UpdatePosition();
		score_powerup_.VerifyPosition(application_->getWindow());
	}
	/*if(beam_powerup_.visibility() == true)
	{
		beam_powerup_.UpdatePosition();
		beam_powerup_.VerifyPosition(application_->getWindow());
	}*/

	switch(application_->settings_.difficulty_)	// difficulty based spawns
	{
	case GameSettings::EASY:
		if (frame_counter % 20 == 0) 		// every 20 frames (3 times per second assuming 60 fps)
		{
			SpawnAsteroid();
		}
		if (frame_counter % 120 == 0)		// every 120 frames (2 times per second assuming 60 fps)
		{
			SpawnEnemy();
		}
		if (frame_counter % 300 == 0)		// every 300 frames (5 seconds assuming 60 fps)
		{
			SpawnPowerup();
		}
		break;
	case GameSettings::MEDIUM:
		if (frame_counter % 18 == 0) 		// every 18 frames (3.333 times per second assuming 60 fps)
		{
			SpawnAsteroid();
		}
		if (frame_counter % 120 == 0)		// every 120 frames (2 seconds assuming 60 fps)
		{
			SpawnEnemy();
		}
		if (frame_counter % 600 == 0)		// every 600 frames (10 seconds assuming 60 fps)
		{
			SpawnPowerup();
		}
		break;
	case GameSettings::HARD:
		if (frame_counter % 14 == 0) 		// every 14 frames (4.2 times per second assuming 60 fps)
		{
			SpawnAsteroid();
		}
		if (frame_counter % 120 == 0)		// every 120 frames (2 seconds assuming 60 fps)
		{
			SpawnEnemy();
		}
		if (frame_counter % 600 == 0)		// every 600 frames (10 seconds assuming 60 fps)
		{
			SpawnPowerup();
		}
		break;
	case GameSettings::EXTREME:
		if (frame_counter % 10 == 0) 		// every 10 frames (6 times per second assuming 60 fps)
		{
			SpawnAsteroid();
		}
		if (frame_counter % 120 == 0)		// every 120 frames (2 seconds assuming 60 fps)
		{
			SpawnEnemy();
		}
		if (frame_counter % 600 == 0)	// every 600 frames (10 seconds assuming 60 fps)
		{
			SpawnPowerup();
		}
		break;
	case GameSettings::TRANSCENDENT:
		if (frame_counter % 6 == 0) 		// every 6 frames (10 times per second assuming 60 fps)
		{
			SpawnAsteroid();
		}
		if (frame_counter % 60 == 0)		// every 60 frames (1 second assuming 60 fps)
		{
			SpawnEnemy();
		}
		if (frame_counter % 600 == 0)	// every 600 frames (10 seconds assuming 60 fps)
		{
			SpawnPowerup();
		}
		break;
	}
}

void GameState::CollisionDetectionLoop()
{
	b2Contact* contact = world_->GetContactList();
	int contact_count = world_->GetContactCount();
	for(int contact_num = 0; contact_num < contact_count; contact_num++)
	{
		// get pointer to objects that are colliding (must cast void ptr to correct type)
		GameObject* objectA = (GameObject*)contact->GetFixtureA()->GetBody()->GetUserData();
		GameObject* objectB = (GameObject*)contact->GetFixtureB()->GetBody()->GetUserData();
		
		if(contact->IsTouching())
		{
			switch(objectA->type())
			{

			case GameObject::PLAYER: // A - PLAYER
				switch(objectB->type())
				{
				case GameObject::ENEMY: // A - PLAYER, B - ENEMY
					{
						LivingObject* enemy = static_cast<LivingObject*>(objectB); // B is a livingobject, cast pointer to correct type
						player_.UpdateHealth(-enemy_damage_); 			// player takes 20 damage
						if(player_.health() <= 0) 				// if player dies
						{
							//SpawnExplosion(player_.position().x, player_.position().y);		// spawn player explosion
							player_.set_visibility(false); 		// set player visibility to false
						}
						//SpawnExplosion(enemy->position().x, enemy->position().y);		// spawn enemy explosion
						enemy->set_visibility(false); 	// set enemy to not visible - flags for delete
						break;
					}

				case GameObject::ASTEROID: // A - PLAYER, B - ASTEROID
					{
						LivingObject* asteroid = static_cast<LivingObject*>(objectB); // B is a livingobject, cast pointer to correct type
						player_.UpdateHealth(-asteroid_damage_); 			// player takes 20 damage
						if(player_.health() <= 0) 				// if player dies
						{
							//SpawnExplosion(player_.position().x, player_.position().y);		// spawn player explosion
							player_.set_visibility(false); 		// set player visibility to false
						}
						//SpawnExplosion(asteroid->position().x, asteroid->position().y);		// spawn asteroid explosion
						asteroid->set_visibility(false); 	// set asteroid to not visible - flags for delete
						break;
					}
				}
				break;

			case GameObject::PLAYER_PROJECTILE: // A - PLAYER_PROJECTILE
				switch(objectB->type())
				{
				case GameObject::ENEMY:	// A - PLAYER_PROJECTILE, B - ENEMY
					{
						LivingObject* enemy = static_cast<LivingObject*>(objectB); // B is a livingobject, cast pointer to correct type
						enemy->UpdateHealth(-(player_cannon_.damage_));		// enemy is damaged by cannon
						if(enemy->health() <= 0.0f) 						// if enemy dies
						{
							score_ += 100;									// add some score
							//SpawnExplosion(enemy->position().x, enemy->position().y);		// spawn explosion
							enemy->set_visibility(false);					// sets enemy to not visible (flags for delete)
						}
						objectA->set_visibility(false);						// sets projectile not visible (flags for delete)
						break;
					}

				case GameObject::ASTEROID: // A - PLAYER_PROJECTILE, B - ASTEROID
					{
						LivingObject* asteroid = static_cast<LivingObject*>(objectB);	// B is a livingobject, cast pointer to correct type
						asteroid->UpdateHealth(-(player_cannon_.damage_));		// asteroid is damaged by cannon
						if(asteroid->health() <= 0.0f) 							// if asteroid dies
						{
							score_ += 20;										// add some score
							//SpawnExplosion(asteroid->position().x, asteroid->position().y);		// spawn explosion
							asteroid->set_visibility(false);					// sets asteroid to not visible (flags for delete)
						}
						objectA->set_visibility(false);							// sets projectile not visible (flags for delete)
						break;
					}
				}
				break;

			case GameObject::PLAYER_PHOTON: // A - PLAYER_PHOTON
				switch(objectB->type())
				{
				case GameObject::ENEMY: // A - PLAYER_PHOTON, B - ENEMY
					{
						LivingObject* enemy = static_cast<LivingObject*>(objectB); // B is a livingobject, cast pointer to correct type
						enemy->UpdateHealth(-(player_photon_.damage_));
						if(enemy->health() <= 0.0f)
						{
							score_ += 100;												// add some score
							//SpawnExplosion(enemy->position().x, enemy->position().y);	// spawn explosion
							enemy->set_visibility(false);								// sets enemy to not visible (flags for delete)
						}
						break;
					}

				case GameObject::ASTEROID: // A - PLAYER_PHOTON, B - ASTEROID
					{
						LivingObject* asteroid = static_cast<LivingObject*>(objectB); // B is a livingobject, cast pointer to correct type
						asteroid->UpdateHealth(-(player_photon_.damage_));
						if(asteroid->health() <= 0.0f)
						{
							score_ += 20;													// add some score
							//SpawnExplosion(asteroid->position().x, asteroid->position().y);	// spawn explosion
							asteroid->set_visibility(false);								// sets enemy to not visible (flags for delete)
						}
						break;
					}
				}
				break;

			case GameObject::ENEMY: // A - ENEMY
				{
					LivingObject* enemy = static_cast<LivingObject*>(objectA); // A is a livingobject, cast pointer to correct type
					switch(objectB->type())
					{

					case GameObject::PLAYER: // A - ENEMY, B - PLAYER
						player_.UpdateHealth(-enemy_damage_); 			// player takes 20 damage
						if(player_.health() <= 0) 				// if player dies
						{
							//SpawnExplosion(player_.position().x, player_.position().y);		// spawn player explosion
							player_.set_visibility(false); 		// set player visibility to false
						}
						//SpawnExplosion(enemy->position().x, enemy->position().y);		// spawn enemy explosion
						enemy->set_visibility(false); 	// set enemy to not visible - flags for delete
						break;

					case GameObject::PLAYER_PROJECTILE: // A - ENEMY, B - PLAYER_PROJECTILE
						enemy->UpdateHealth(-(player_cannon_.damage_));		// enemy is damaged by cannon
						if(enemy->health() <= 0.0f) 						// if enemy dies
						{
							score_ += 100;									// add some score
							//SpawnExplosion(enemy->position().x, enemy->position().y);		// spawn explosion
							enemy->set_visibility(false);					// sets enemy to not visible (flags for delete)
						}
						objectB->set_visibility(false);						// sets projectile not visible (flags for delete)
						break;

					case GameObject::PLAYER_PHOTON:  // A - ENEMY, B - PLAYER_PHOTON
						enemy->UpdateHealth(-(player_photon_.damage_));
						if(enemy->health() <= 0.0f)
						{
							score_ += 100;												// add some score
							//SpawnExplosion(enemy->position().x, enemy->position().y);	// spawn explosion
							enemy->set_visibility(false);								// sets enemy to not visible (flags for delete)
						}
						break;
					}
					break;
				}

			case GameObject::ASTEROID: // A - ASTEROID
				{
					LivingObject* asteroid = static_cast<LivingObject*>(objectA); // A is a livingobject, cast pointer to correct type
					switch(objectB->type())
					{

					case GameObject::PLAYER: // A - ASTEROID, B - PLAYER
						player_.UpdateHealth(-asteroid_damage_); 		// player takes 20 damage
						if(player_.health() <= 0) 			// if player dies
						{
							//SpawnExplosion(player_.position().x, player_.position().y);		// spawn player explosion
							player_.set_visibility(false); 		// set player visibility to false
						}
						//SpawnExplosion(asteroid->position().x, asteroid->position().y);		// spawn asteroid explosion
						asteroid->set_visibility(false); 	// set asteroid to not visible - flags for delete
						break;

					case GameObject::PLAYER_PROJECTILE: // A - ASTEROID, B - PLAYER_PROJECTILE
						asteroid->UpdateHealth(-(player_cannon_.damage_));		// asteroid is damaged by cannon
						if(asteroid->health() <= 0.0f) 							// if asteroid dies
						{
							score_ += 20;										// add some score
							//SpawnExplosion(asteroid->position().x, asteroid->position().y);		// spawn explosion
							asteroid->set_visibility(false);					// sets asteroid to not visible (flags for delete)
						}
						objectB->set_visibility(false);							// sets projectile not visible (flags for delete)
						break;

					case GameObject::PLAYER_PHOTON: // A - ASTEROID, B- PLAYER_PHOTON
						asteroid->UpdateHealth(-(player_photon_.damage_));
						if(asteroid->health() <= 0.0f)
						{
							score_ += 20;													// add some score
							//SpawnExplosion(asteroid->position().x, asteroid->position().y);	// spawn explosion
							asteroid->set_visibility(false);								// sets enemy to not visible (flags for delete)
						}
						break;
					}
					break;
				}
			}
			contact = contact->GetNext();
		}
	}
	
	// Pickups collision detection
	if(score_powerup_.visibility() == true)
	{
		if(score_powerup_.CollisionTest(player_) == true)
		{
			score_powerup_.set_visibility(false);	// remove pickup
			score_ *= 1.5;							// multiplies score by 1.5
		}
	}
	/*if(beam_powerup_.visibility() == true)
	{
		if(beam_powerup_.CollisionTest(player_) == true)
		{
			beam_powerup_.set_visibility(false);	// remove pickup
			player_beam_.energy_ = 100.0f;			// grant 100 beam energy
			player_beam_.overcharged_ = true;		// set beam to overcharged
		}
	}

	// Beam collision detection - still uses my own AABB collision detection and not box2D
	if(beam_.visibility() == true)
	{
		GameObject beam_tracer_ = GameObject(); 		// creates a gameobject to trace beams path
		beam_tracer_.AccelerateTo(sf::Vector2f(8.0f, 0.0f));		// sets velocity to 18 in +X direction (velocity is essentially step or iteration length here, higher velocity means less iterations per frame but more accuracy)
		beam_tracer_.setDimensions(sf::Vector2f(8.0f, beam_.getLocalBounds().height));
		beam_tracer_.setPosition(sf::Vector2f(player_.getPosition().x, player_.getPosition().y)); // start trace at centre of player
		beam_tracer_.set_visibility(true); 			// set visible

		while(beam_tracer_.visibility() == true) // while tracer is visible
		{
			beam_tracer_.UpdatePosition(); 	// moves the tracer according to its velocity per loop iteration
			for(std::vector<LivingObject*>::iterator enemycounter = Enemies.begin(); enemycounter != Enemies.end(); enemycounter++) 	// for every enemy
			{
				if((*enemycounter)->visibility() == true) 						// if enemy is visible
				{
					if(beam_tracer_.CollisionTest(**enemycounter) == true)		// if tracer collides with the enemy
					{
						(*enemycounter)->UpdateHealth(-(player_beam_.damage_));	// damage enemy for beams damage
						if((*enemycounter)->health() <= 0) 						// if enemy dies
						{
							score_ += 100;										// add some score
							//SpawnExplosion((*enemycounter)->position().x, (*enemycounter)->position().y);	// spawn explosion
							(*enemycounter)->set_visibility(false);				// sets enemy to not visible (flags for delete)
						}
						if(player_beam_.overcharged_ == false)		// if beam isn't overcharging - set beam width and position appropriately and make tracer invisible/break from loop
						{
							// sets beam width to the length of the gap between the player and the enemy
							beam_.setDimensions(sf::Vector2f(((*enemycounter)->getPosition().x) - (player_.getPosition().x), beam_.getLocalBounds().height));
							// sets beam position to the centre of the gap between the player and the enemy
							beam_.MoveTo((player_.getPosition().x + (*enemycounter)->getPosition().x) / 2.0f, player_.getPosition().y);
							beam_tracer_.set_visibility(false); // enemy has been hit - no further need for tracing
							break; // breaks loop if beam isn't overcharged (beam pierces when overcharged)
						}
					}
				}
			}
			if(beam_tracer_.visibility() == true) // if the tracer is still visible continue with asteroid collision detection
			{
				for(std::vector<LivingObject*>::iterator asteroidcounter = Asteroids.begin(); asteroidcounter != Asteroids.end(); asteroidcounter++) 	// for every asteroid
				{
					if((*asteroidcounter)->visibility() == true) 		// if asteroid is visible
					{
						if(beam_tracer_.CollisionTest(**asteroidcounter) == true) // if tracer collides with the asteroid
						{
							(*asteroidcounter)->UpdateHealth(-(player_beam_.damage_)); // damage asteroid for beams damage
							if((*asteroidcounter)->health() <= 0) 		// if asteroid dies
							{
								score_ += 20;							// add some score
								//SpawnExplosion((*asteroidcounter)->position().x, (*asteroidcounter)->position().y);		// spawn explosion
								(*asteroidcounter)->set_visibility(false);	// sets asteroid to not visible (flags for delete)
							}
							if(player_beam_.overcharged_ == false)
							{
								// sets beam width to the length of the gap between the player and the asteroid
								beam_.setDimensions(sf::Vector2f(((*asteroidcounter)->getPosition().x) - (player_.getPosition().x), beam_.getLocalBounds().height));
								// sets beam position to the centre of the gap between the player and the asteroid
								beam_.MoveTo((player_.getPosition().x + (*asteroidcounter)->getPosition().x) / 2.0f, player_.getPosition().y);
								beam_tracer_.set_visibility(false);
								break; // breaks loop if beam isn't overcharged (beam pierces when overcharged)
							}
						}
					}
				}
			}
			if(beam_tracer_.visibility() == true) // if the tracer is still visible then either nothing was hit or the beam is overcharged and pierced everything
			{
				beam_tracer_.VerifyPosition(application_->getWindow()); // verify tracer is in screen boundary
				if(beam_tracer_.visibility() == false)	// if tracer has moved out of screen boundary
				{
					// sets beam width to the length of the gap between the player and the right of the screen
					beam_.setDimensions(sf::Vector2f(application_->getWindow().getSize().x - player_.getPosition().x, beam_.getLocalBounds().height));
					// sets beam position to the centre of the gap between the player and the right of the screen
					beam_.MoveTo((player_.getPosition().x + application_->getWindow().getSize().x) / 2.0f, player_.getPosition().y);
				}
			}
		}
	}*/
}

void GameState::SpawnPhoton()
{
	if(application_->settings_.sound_effects_ == true) // if sound effects are enabled
	{
		/*if(photon_sfx_ != -1) // and the sound effect exists
		{
			audio_manager_->PlaySample(photon_sfx_); // play photon sound effect
		}*/
	}

	// object properties
	float x_position = player_.getPosition().x + (player_.getLocalBounds().height / 2.0f);		// right edge of player
	float y_position = player_.getPosition().y;						// player y position
	float length = 30.0f; // square - use for both width and height

	// box2d body properties
	b2BodyDef body;
	body.type = b2_dynamicBody;
	body.position = b2Vec2(GFX_BOX2D_POS_X(x_position), GFX_BOX2D_POS_Y(y_position));
	body.bullet = true; // flag as bullet for more accurate collision detection
	b2CircleShape circle;
	circle.m_p.Set(0, 0); // centre is centre of body, no offset
	circle.m_radius = GFX_BOX2D_SIZE(length) * 0.5f;
	b2FixtureDef fixture;
	fixture.shape = &circle;
	fixture.density = 15.0f; // very high density
	fixture.friction = 0.1f; // standard friction
	fixture.restitution = 0.0f; // no bounce

	// gameobject properties
	GameObject* photon = new GameObject; // creates pointer to new gameobject constructed on the heap
	photon->AddBody(world_, body, fixture); // adds body to object and changes object to using box2d physics
	photon->initSprite(sf::Vector2f(x_position, y_position), photon_texture_); // init sprite properties
	photon->setDimensions(sf::Vector2f(length, length));
	photon->RotateTo(90.0f); // rotate to facing right
	photon->set_type(GameObject::PLAYER_PHOTON);
	photon->AccelerateTo(sf::Vector2f(4.0f, 0.0f));	// 4.0f velocity in positive x direction
	photon->set_visibility(true);			// Makes projectile visible

	// adds it to the cannon projectile vector
	Photons.push_back(photon);

	// adds a projectilemessage to the spawn messages queue for this cannon projectile
	NetMessage photonmessage;
	photonmessage.type = MT_GAMEMESSAGE;
	photonmessage.data = photon->type();
	photonmessage.x = x_position;
	photonmessage.y = y_position;
	photonmessage.frame = application_->getFrame();
	photonmessage.velo_x = photon->velocity().x;
	photonmessage.velo_y = photon->velocity().y;
	GameMessages.push(photonmessage);
}

void GameState::SpawnCannonProjectile()
{
	// object properties
	float x_position = player_.getPosition().x + (player_.getLocalBounds().height / 2.0f);	// right edge of player
	float y_position = player_.getPosition().y;					// player y position
	float width = 7.8f;
	float height = 22.2f;

	// box2d body properties
	b2BodyDef body;
	body.type = b2_dynamicBody;
	body.position = b2Vec2(GFX_BOX2D_POS_X(x_position), GFX_BOX2D_POS_Y(y_position));
	body.bullet = true; // flag as bullet for more accurate collision detection
	b2PolygonShape box;
	box.SetAsBox(GFX_BOX2D_SIZE(width) * 0.5f, GFX_BOX2D_SIZE(height) * 0.5f); // box2d body is bigger than the projectile width-wise
	b2FixtureDef fixture;
	fixture.shape = &box;
	fixture.density = 8.0f; // high density
	fixture.friction = 0.1f; // standard friction
	fixture.restitution = 0.0f; // no bounce

	// gameobject properties
	GameObject* cannon_projectile = new GameObject(); // creates pointer to new gameobject constructed on the heap
	// adds height to the x because the projectile must be rotated by 90 degrees since the player is (this does not dynamically rotate with player right now, future improvement idea)
	cannon_projectile->AddBody(world_, body, fixture); // adds body to object and changes object to using box2d physics
	cannon_projectile->initSprite(sf::Vector2f(x_position, y_position), orange_cannon_projectile_texture_); // init sprite properties
	cannon_projectile->setDimensions(sf::Vector2f(width, height));
	cannon_projectile->RotateTo(90.0f); // rotate so that facing right
	cannon_projectile->set_type(GameObject::PLAYER_PROJECTILE); // set to playerprojectile
	cannon_projectile->AccelerateTo(sf::Vector2f(8.0f, 0.0f));	// 8.0f velocity in positive x direction
	cannon_projectile->set_visibility(true);	// Makes projectile visible

	// adds the new pointer to the cannon projectile vector array
	CannonProjectiles.push_back(cannon_projectile);

	// adds a projectilemessage to the spawn messages queue for this cannon projectile
	NetMessage projectilemessage;
	projectilemessage.type = MT_GAMEMESSAGE;
	projectilemessage.data = cannon_projectile->type();
	projectilemessage.x = x_position;
	projectilemessage.y = y_position;
	projectilemessage.frame = application_->getFrame();
	projectilemessage.velo_x = cannon_projectile->velocity().x;
	projectilemessage.velo_y = cannon_projectile->velocity().y;
	GameMessages.push(projectilemessage);
}

void GameState::SpawnEnemy()
{
	// object properties
	// calculate position before as theres a random element and it needs to be the same for the box2d body and object
	float x_position = application_->getWindow().getSize().x;			// right edge of screen
	float y_position = rand() % application_->getWindow().getSize().y;	// random y position
	float width = 52.0f;
	float height = 42.0f;

	// box2d body definition
	b2BodyDef body;
	body.type = b2_dynamicBody;
	body.position = b2Vec2(GFX_BOX2D_POS_X(x_position), GFX_BOX2D_POS_Y(y_position));
	b2PolygonShape box;
	box.SetAsBox(GFX_BOX2D_SIZE(width) * 0.5f, GFX_BOX2D_SIZE(height) * 0.5f);
	b2FixtureDef fixture;
	fixture.shape = &box;
	fixture.density = 5.0f; // medium density
	fixture.friction = 0.1f; // standard friction
	fixture.restitution = 0.1f; // very low bouncyness

	// livingobject properties
	LivingObject* enemy = new LivingObject(); // create pointer to new livingobject constructed on the heap
	enemy->AddBody(world_, body, fixture); // adds body to object and changes object to using box2d physics
	enemy->initSprite(sf::Vector2f(x_position, y_position), enemy_texture_); // init sprite properties
	enemy->setDimensions(sf::Vector2f(width, height));
	enemy->RotateTo(-90.0f);				// rotate object so that its facing left
	enemy->set_type(GameObject::ENEMY);		// set object type to ENEMY
	enemy->set_visibility(true);			// set to visible
	
	switch(application_->settings_.difficulty_) // difficulty specific settings (health and velocity)
	{
	case GameSettings::EASY:
		enemy->set_max_health(200.0f);
		enemy->AccelerateTo(sf::Vector2f(rand()%4 + (-6.0f), 0.0f));	// random velocity between 3 and 6 in -x direction
		break;
	case GameSettings::MEDIUM:
		enemy->set_max_health(200.0f);
		enemy->AccelerateTo(sf::Vector2f(rand()%4 + (-6.0f), 0.0f));	// random velocity between 3 and 6 in -x direction
		break;
	case GameSettings::HARD:
		enemy->set_max_health(300.0f);
		enemy->AccelerateTo(sf::Vector2f(rand()%4 + (-7.0f), 0.0f));	// random velocity between 4 and 7 in -x direction
		break;
	case GameSettings::EXTREME:
		enemy->set_max_health(400.0f);
		enemy->AccelerateTo(sf::Vector2f(rand()%4 + (-7.0f), 0.0f));	// random velocity between 4 and 7 in -x direction
		break;
	case GameSettings::TRANSCENDENT:
		enemy->set_max_health(400.0f);
		enemy->AccelerateTo(sf::Vector2f(rand()%4 + (-8.0f), 0.0f));	// random velocity between 5 and 8 in -x direction
		break;
	}
	enemy->set_health(enemy->max_health());			// set enemy to max health

	// adds enemy pointer to enemies vector array
	Enemies.push_back(enemy);

	// adds an enemymessage for this enemy to the spawn messages queue
	NetMessage enemymessage;
	enemymessage.type = MT_GAMEMESSAGE;
	enemymessage.data = enemy->type();
	enemymessage.x = x_position;
	enemymessage.y = y_position;
	enemymessage.frame = application_->getFrame();
	enemymessage.velo_x = enemy->velocity().x;
	enemymessage.velo_y = enemy->velocity().y;
	enemymessage.health = enemy->max_health();
	GameMessages.push(enemymessage);
}

void GameState::SpawnAsteroid()
{
	// object properties
	// calculate position before as theres a random element and it needs to be the same for the box2d body and object
	float x_position = application_->getWindow().getSize().x;			// right edge of screen
	float y_position = rand() % application_->getWindow().getSize().y;	// random y position
	float length = 32.0f; // square - use for both width and height

	// box2d body definition
	b2BodyDef body;
	body.type = b2_dynamicBody;
	body.position = b2Vec2(GFX_BOX2D_POS_X(x_position), GFX_BOX2D_POS_Y(y_position));
	b2CircleShape circle;
	circle.m_p.Set(0, 0); // centre is centre of body, no offset
	circle.m_radius = GFX_BOX2D_SIZE(length) * 0.5f;
	b2FixtureDef fixture;
	fixture.shape = &circle;
	fixture.density = 3.0f; // low-medium density
	fixture.friction = 0.1f; // standard friction
	fixture.restitution = 0.2f; // low bouncyness

	// livingobject properties
	LivingObject* asteroid = new LivingObject(); // create pointer to new livingobject constructed on the heap
	asteroid->AddBody(world_, body, fixture);	// adds body to object and changes object to using box2d physics
	asteroid->initSprite(sf::Vector2f(x_position, y_position), asteroid_texture_); // init sprite properties
	asteroid->setDimensions(sf::Vector2f(length, length));
	asteroid->set_type(GameObject::ASTEROID);
	asteroid->set_visibility(true);
	
	switch(application_->settings_.difficulty_) // difficulty specific settings (max health and velocity)
	{
	case GameSettings::EASY:
		asteroid->set_max_health(80.0f);
		asteroid->AccelerateTo(sf::Vector2f(rand()%3 + (-6.0f), 0.0f));	// random velocity between 4 and 6 in -x direction
		break;
	case GameSettings::MEDIUM:
		asteroid->set_max_health(80.0f);
		asteroid->AccelerateTo(sf::Vector2f(rand()%4 + (-7.0f), 0.0f));	// random velocity between 4 and 7 in -x direction
		break;
	case GameSettings::HARD:
		asteroid->set_max_health(80.0f);
		asteroid->AccelerateTo(sf::Vector2f(rand()%4 + (-7.0f), 0.0f));	// random velocity between 4 and 7 in -x direction
		break;
	case GameSettings::EXTREME:
		asteroid->set_max_health(150.0f);
		asteroid->AccelerateTo(sf::Vector2f(rand()%4 + (-9.0f), 0.0f));	// random velocity between 5 and 8 in -x direction
		break;
	case GameSettings::TRANSCENDENT:
		asteroid->set_max_health(150.0f);
		asteroid->AccelerateTo(sf::Vector2f(rand()%6 + (-10.0f), 0.0f));	// random velocity between 5 and 10 in -x direction
		break;
	}
	asteroid->set_health(asteroid->max_health());		// set asteroid to max health
	
	// adds asteroid pointer to asteroids vector array
	Asteroids.push_back(asteroid);

	// adds an asteroidmessage for this asteroid to the spawn messages queue
	NetMessage asteroidmessage;
	asteroidmessage.type = MT_GAMEMESSAGE;
	asteroidmessage.data = asteroid->type();
	asteroidmessage.x = x_position;
	asteroidmessage.y = y_position;
	asteroidmessage.frame = application_->getFrame();
	asteroidmessage.velo_x = asteroid->velocity().x;
	asteroidmessage.velo_y = asteroid->velocity().y;
	asteroidmessage.health = asteroid->max_health();
	GameMessages.push(asteroidmessage);
}

void GameState::SpawnPowerup()
{
	int random_powerup_ = 0; //rand()%2; // picks a powerup at random between the 2 (0 or 1)
	switch(random_powerup_)
	{
	case 0:
		if(score_powerup_.visibility() == false)
		{
			int x_position = application_->getWindow().getSize().x;
			int y_position = rand() % application_->getWindow().getSize().y;

			score_powerup_.setPosition(sf::Vector2f(x_position, y_position));	// sets powerup to a random height at the right edge of the screen
			score_powerup_.set_visibility(true);	// flags for collision detection and rendering
			
			// Add spawn netmessage to queue for the score powerup
			NetMessage scorepowerupmessage;
			scorepowerupmessage.type = MT_GAMEMESSAGE;
			scorepowerupmessage.data = score_powerup_.type();
			scorepowerupmessage.x = x_position;
			scorepowerupmessage.y = y_position;
			scorepowerupmessage.frame = application_->getFrame();
			GameMessages.push(scorepowerupmessage);
		}
		break;
	/*case 1:
		if(beam_powerup_.visibility() == false)
		{
			beam_powerup_.setPosition(sf::Vector2f(application_->getWindow().getSize().x, rand() % application_->getWindow().getSize().y));
			beam_powerup_.set_visibility(true);
		}
		break;*/
	}
}

std::list<NetMessage> GameState::getWorldData()
{
	std::list<NetMessage> WorldData;
	NetMessage player;
	player.type = MT_GAMEMESSAGE;
	player.data = player_.type();
	player.x = player_.getPosition().x;
	player.y = player_.getPosition().y;
	player.frame = application_->getFrame();
	player.velo_x = player_.velocity().x;
	player.velo_y = player_.velocity().y;
	player.health = player_.health();
	WorldData.push_back(player);

	for (auto object : CannonProjectiles)
	{
		NetMessage objectmessage;
		objectmessage.type = MT_GAMEMESSAGE;
		objectmessage.data = object->type();
		objectmessage.x = object->getPosition().x;
		objectmessage.y = object->getPosition().y;
		objectmessage.frame = application_->getFrame();
		objectmessage.velo_x = object->velocity().x;
		objectmessage.velo_y = object->velocity().y;
		WorldData.push_back(objectmessage);
	}

	for (auto object : Photons)
	{
		NetMessage objectmessage;
		objectmessage.type = MT_GAMEMESSAGE;
		objectmessage.data = object->type();
		objectmessage.x = object->getPosition().x;
		objectmessage.y = object->getPosition().y;
		objectmessage.frame = application_->getFrame();
		objectmessage.velo_x = object->velocity().x;
		objectmessage.velo_y = object->velocity().y;
		WorldData.push_back(objectmessage);
	}

	for (auto object : Enemies)
	{
		NetMessage objectmessage;
		objectmessage.type = MT_GAMEMESSAGE;
		objectmessage.data = object->type();
		objectmessage.x = object->getPosition().x;
		objectmessage.y = object->getPosition().y;
		objectmessage.frame = application_->getFrame();
		objectmessage.velo_x = object->velocity().x;
		objectmessage.velo_y = object->velocity().y;
		objectmessage.health = object->health();
		WorldData.push_back(objectmessage);
	}

	for (auto object : Asteroids)
	{
		NetMessage objectmessage;
		objectmessage.type = MT_GAMEMESSAGE;
		objectmessage.data = object->type();
		objectmessage.x = object->getPosition().x;
		objectmessage.y = object->getPosition().y;
		objectmessage.frame = application_->getFrame();
		objectmessage.velo_x = object->velocity().x;
		objectmessage.velo_y = object->velocity().y;
		objectmessage.health = object->health();
		WorldData.push_back(objectmessage);
	}

	if (score_powerup_.visibility() == true)
	{
		NetMessage powerupmessage;
		powerupmessage.type = MT_GAMEMESSAGE;
		powerupmessage.data = score_powerup_.type();
		powerupmessage.x = score_powerup_.getPosition().x;
		powerupmessage.y = score_powerup_.getPosition().y;
		powerupmessage.frame = application_->getFrame();
		WorldData.push_back(powerupmessage);
	}

	NetMessage score;
	score.type = MT_SCOREMESSAGE;
	score.data = score_;
	score.frame = application_->getFrame();
	WorldData.push_back(score);

	NetMessage ammo;
	ammo.type = MT_AMMOMESSAGE;
	ammo.data = player_cannon_.ammo_count_;
	ammo.frame = application_->getFrame();
	WorldData.push_back(ammo);

	NetMessage photon;
	photon.type = MT_PHOTONMESSAGE;
	photon.data = player_photon_.current_charge_;
	photon.frame = application_->getFrame();
	WorldData.push_back(photon);

	NetMessage settings;
	settings.type = MT_SETTINGS;
	settings.data = application_->settings_.difficulty_;
	WorldData.push_back(settings);

	return WorldData;
}

/*void GameState::SpawnExplosion(float explosion_x, float explosion_y)
{
	if(application_->settings_.sound_effects_ == true) // if sound effects are enabled
	{
		if(explosion_sfx_ != -1) // and the sound effect exists
		{
			audio_manager_->PlaySample(explosion_sfx_); // play explosion sound
		}
	}

	// creates an explosion gameobject at enemy position
	GameObject explosion = GameObject();
	explosion.InitSprite(32.0f, 32.0f, abfw::Vector3(explosion_x, explosion_y, 0.0f), explosion_texture_);
	explosion.set_uv_width(0.25f);
	explosion.set_uv_height(0.25f);
	explosion.InitSpriteAnimation(0.1f, 16, false, Sprite::SCROLL_XY, 4, 4); // 0.1 second per frame, 16 frames, not looping, scroll x then y on spritesheet, 4 columns, 4 rows
	explosion.set_visibility(true);

	// adds to explosions vector array
	Explosions.push_back(explosion);
}*/