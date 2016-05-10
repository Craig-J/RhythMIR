#pragma once

#include <SFML_Extensions/Graphics/sprite.h>
#include <SFML_Extensions/System/vector2.h>

#include <SFML/Graphics.hpp>


class GameObject : public sfx::Sprite
{
public:

	GameObject(sf::Vector2f _initial_position = sf::Vector2f(0.0f, 0.0f), sfx::TexturePtr _texture = nullptr);
	virtual ~GameObject();

	// Updates position according to current velocity, factored by delta time
	void UpdatePosition(const float _delta_time);

	// Sets object visibility to false if not within the window
	// Returns true if the object is still visible, false if not
	bool VerifyPosition(const sf::RenderWindow& window);

	// Collision detection functions
	bool BoxCollisionTest(const GameObject& Object);
	//bool CircleCollisionTest(const GameObject& Object); NYI

	// Transform Functions
	void Move(const sf::Vector2f& _vec, bool _relative = true);
	void Rotate(const float _degrees, bool _relative = true);
	void Accelerate(const sf::Vector2f& _velocity, bool _relative = true);
	inline const sf::Vector2f velocity() const { return velocity_; }
	

	inline const bool visibility() const { return visibility_; }
	inline void SetVisibility(const bool visibility) { visibility_ = visibility; }
	

protected:

	bool visibility_; // Usually used to hide objects or flag them for deletion
	sf::Vector2f velocity_;

};
