#include "game_object.h"

GameObject::GameObject() :
	visibility_(false),
	velocity_(0.0f, 0.0f),
	physicsengine_(DEFAULT)
{}

GameObject::~GameObject()
{}

void GameObject::MoveTo(const float x, const float y)
{
	switch (physicsengine_)
	{
	case DEFAULT:
		setPosition(sf::Vector2f(x, y));
		break;
	}
}

void GameObject::MoveBy(const float x, const float y)
{
	switch (physicsengine_)
	{
	case DEFAULT:
		move((sf::Vector2f(x, y)));
		break;
	}
}

void GameObject::UpdatePosition()
{
	switch (physicsengine_)
	{
	case DEFAULT:
		move(velocity_);
		break;
	}
}

bool GameObject::BoxCollisionTest(const GameObject& _object)
{
	if (getGlobalBounds().intersects(_object.getGlobalBounds())) // if the objects bounding boxes are intersecting
	{
		return true;
	}
	return false;
}

bool GameObject::CircleCollisionTest(const GameObject& _object)
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
}

void GameObject::VerifyPosition(const sf::RenderWindow& window)
{
	float width, height;
	width = window.getSize().x;
	height = window.getSize().y;

	if ((getPosition().x > width) || (getPosition().x < 0) || (getPosition().y > height) || (getPosition().y < 0)) // If out of screen bounds
	{
		set_visibility(false); // sets object to not visible, essentially removing it from scene
	}
}

void GameObject::AccelerateTo(const sf::Vector2f& velocity)
{
	switch (physicsengine_)
	{
	case DEFAULT:
		velocity_ = velocity;
		break;
	}
}

void GameObject::AccelerateBy(const sf::Vector2f& deltavelocity)
{
	switch (physicsengine_)
	{
	case DEFAULT:
		velocity_ += deltavelocity;
		break;
	}
}

void GameObject::RotateTo(const float degrees)
{
	switch (physicsengine_)
	{
	case DEFAULT:
		setRotation(degrees);
		break;
	}
}

void GameObject::RotateBy(const float degrees)
{
	switch (physicsengine_)
	{
	case DEFAULT:
		rotate(degrees);
		break;
	}
}