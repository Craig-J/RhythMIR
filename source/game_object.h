#ifndef _GAME_OBJECT_H
#define _GAME_OBJECT_H

#include <SFML\Graphics.hpp>
#include <SFML_Extensions\Graphics\sprite.h>

class GameObject : public sfx::Sprite
{	// Physics based game object
public:

	GameObject();
	virtual ~GameObject();

	void UpdatePosition(); // updates position_ with velocity_ or with box2d body_ position
	void VerifyPosition(const sf::RenderWindow& window); // Used to make sure the object is within the bounds of the screen region
	bool BoxCollisionTest(const GameObject& Object);
	bool CircleCollisionTest(const GameObject& Object);
	
	void RotateTo(const float degrees);	// Absolute rotation function
	void RotateBy(const float degrees); // Relative rotation function
	inline const bool visibility() const { return visibility_; }
	inline void set_visibility(const bool visibility) { visibility_ = visibility; }
	inline const sf::Vector2f velocity() const { return velocity_; }

	// Default Physics Functions - these work with box2d but shouldn't be used as box2d replaces their functionality
	void MoveTo(const float x, const float y);					// Only real use for these with an object using box2d physics is for teleportation
	void MoveBy(const float x, const float y);
	void AccelerateTo(const sf::Vector2f& velocity);			// Modifies body velocity directly if the object is using box2d physics
	void AccelerateBy(const sf::Vector2f& deltavelocity);

protected:
	enum PHYSICSENGINE { DEFAULT };

	PHYSICSENGINE physicsengine_;
	bool visibility_; // flags object for updates, rendering and occasionally deletion
	sf::Vector2f velocity_;
};

#endif // _GAME_OBJECT_H