#pragma once
#include "appstate.h"
#include "living_object.h" // lowest in sprite hierarchy - also includes gameobject/animatedsprite/abfw::sprite and Box2D
#include <vector>
#include <list>
#include <queue>
#include "protocol.h"

class GameState :
	public AppState
{
public:
	GameState(const GameApplication* application);
	virtual ~GameState();
	
	void InitializeState();
	void TerminateState();
	GAMESTATE Update(const int& frame_counter, Input& myInput);
	void Render(sf::Font& font, sf::RenderWindow& window);

	// this is used to return a list of netmessages containing the current details for every object in the world (for network syncing)
	std::list<NetMessage> getWorldData();

	// Queue of messages containing details about objects that have been spawned.
	std::queue<NetMessage> GameMessages;
	
private:

	// Weapon Data Structures
	struct Cannon
	{
		int ammo_count_;
		int max_ammo_count_;
		int recharge_time_; // in frames
		float damage_;
	};
	
	/*struct Beam
	{
		static const float max_energy() { return 100.0f; }
		bool overcharged_;
		bool overheat_;
		float energy_;
		float energy_consumption_rate_; // per frame
		float energy_recharge_rate_; // per frame
		float damage_;
	};*/
	
	struct Photon
	{
		static const float full_charge() { return 100.0f; }
		float current_charge_;
		float recharge_rate_; // per frame
		float damage_;
	};

	struct GameObjectHistory	// not part of gameobject because it is only used for certain objects; also, creating a composite class is overkill for this implementation
	{
		// Queue of previous velocities: for this implementation will be used to hold the previous frames only
		// It can be used to track multiple frames (if implementing quadratic interpolation or anything else that requires more parameters)
		std::queue<sf::Vector2f> velocity_;
		
	};

	const float asteroid_damage_ = 1.0f;
	const float enemy_damage_ = 2.0f;
	const float player_speed_ = 4.0f;
	
	// Function Prototypes
	void LoadTextures();
	//void LoadSounds();
	void InputLoop(Input& myInput);
	void UpdateWeapons(const int& frame_counter); // handles updating all the weapon variables
	void UpdateGameObjects(const int& frame_counter); // updates game object - deleting flagged ones, updating positions and spawning new ones
	void CollisionDetectionLoop();
	void SpawnPhoton();
	void SpawnCannonProjectile();
	void SpawnEnemy();
	void SpawnAsteroid();
	void SpawnPowerup();
	//void SpawnExplosion(float explosion_x, float explosion_y);
	
	// Box2D world pointer
	b2World* world_;
	
	// Textures
	sf::Texture player_texture_;
	sf::Texture enemy_texture_;
	sf::Texture asteroid_texture_;
	sf::Texture orange_cannon_projectile_texture_;
	//sf::Texture beam_texture_;
	//sf::Texture overcharged_beam_texture_;
	sf::Texture photon_texture_;
	//sf::Texture explosion_texture_;
	sf::Texture score_powerup_texture_;
	//sf::Texture beam_powerup_texture_;
	
	// Sound effects
	//Int32 explosion_sfx_;
	//Int32 photon_sfx_;
	
	// Game Objects
	LivingObject player_;
	GameObjectHistory player_history_;
	std::vector<LivingObject*> Enemies;
	std::vector<LivingObject*> Asteroids;
	std::vector<GameObject*> CannonProjectiles;
	std::vector<GameObject*> Photons;
	//std::vector<GameObject> Explosions;
	GameObject score_powerup_; // multiplies current score by 1.5
	//GameObject beam_powerup_; // turns your beam into a supermegadeathlaseroftyrannosaurusrekd
	//GameObject beam_;
	
	// Weapon Objects
	Cannon player_cannon_;
	//Beam player_beam_;
	Photon player_photon_;

	// Variables
	int score_;
};

