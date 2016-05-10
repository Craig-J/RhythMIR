#include "game_object.h"

GameObject::GameObject(sf::Vector2f _initial_position, sfx::TexturePtr _texture) :
	Sprite(_initial_position, _texture),
	visibility_(true),
	velocity_(0.0f, 0.0f)
{}

GameObject::~GameObject()
{}

void GameObject::Move(const sf::Vector2f& _vec, bool _relative)
{
	if (_relative)
		move(_vec);
	else
		setPosition(_vec);
}

void GameObject::UpdatePosition(const float _delta_time)
{
	move(velocity_ * _delta_time);
}

bool GameObject::BoxCollisionTest(const GameObject& _object)
{
	if (getGlobalBounds().intersects(_object.getGlobalBounds())) // if the objects bounding boxes are intersecting
	{
		return true;
	}
	return false;
}

/*bool GameObject::CircleCollisionTest(const GameObject& _object)
{
	auto A = getGlobalBounds();
	sf::Vector2f A_Centre(A.left + (0.5*A.width), A.top + (0.5*A.height));
	float A_radius = getLocalBounds().width * 0.5;

	auto B = _object.getGlobalBounds();
	sf::Vector2f B_Centre(B.left + (0.5*B.width), B.top + (0.5*B.height));
	float B_radius = _object.getLocalBounds().width * 0.5;

	auto difference = B_Centre - A_Centre;
	// TO-DO: Fix when maths functions are imported
	//if ()
	{
		return true;
	}
	return false;
}*/

bool GameObject::VerifyPosition(const sf::RenderWindow& window)
{
	float width, height;
	width = window.getSize().x;
	height = window.getSize().y;

	if ((getPosition().x > width) || (getPosition().x < 0) || (getPosition().y > height) || (getPosition().y < 0)) // If out of screen bounds
	{
		SetVisibility(false); // sets object to not visible, removing it from scene
		return false;
	}
	return true;
}

void GameObject::Accelerate(const sf::Vector2f& _velocity, bool _relative)
{
	if(_relative)
		velocity_ += _velocity;
	else
		velocity_ = _velocity;
}

void GameObject::Rotate(const float _degrees, bool _relative)
{
	if (_relative)
		rotate(_degrees);
	else
		setRotation(_degrees);
}