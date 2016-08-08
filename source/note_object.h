#pragma once

#include "game_object.h"

// NoteObject implements a game object for notes when they are to be used to create gameplay
class NoteObject : public GameObject
{
public:
	NoteObject(sf::Vector2f& _start_position,
			   sf::Vector2f& _target_position,
			   sf::Time& _approach_time,
			   sfx::TexturePtr _texture,
			   sf::Color _color = sf::Color::White);

	sf::Time offset_from_perfect;
};

// NotePath abstracts a lane where notes can be placed
struct NotePath
{
	NotePath(sf::Vector2f& _start_position,
			 sf::Vector2f& _target_position,
			 sf::Time& _approach_time,
			 int _accuracy,
			 sfx::TexturePtr _note_texture,
			 sf::Color _note_color = sf::Color::White);

	sf::Vector2f start_position;
	sf::Vector2f target_position;
	sf::Time approach_time;
	int accuracy;
	sfx::TexturePtr note_texture;
	sf::Color note_color;
	std::vector<NoteObject> notes;
	GameObject target;
};