#include "note_object.h"

NoteObject::NoteObject(sf::Vector2f& _start_position,
					   sf::Vector2f& _target_position,
					   sf::Time& _approach_time,
					   sfx::TexturePtr _texture,
					   sf::Color _color) :
	GameObject(_start_position, _texture)
{
	sf::Vector2f difference = _target_position - _start_position;
	Accelerate(difference / _approach_time.asSeconds(), false);
	if (_color != sf::Color::White)
		setColor(_color);
	offset_from_perfect = _approach_time;
}

NotePath::NotePath(sf::Vector2f& _start_position,
				   sf::Vector2f&  _target_position,
				   sf::Time& _approach_time,
				   int _accuracy,
				   sfx::TexturePtr _note_texture,
				   sf::Color _note_color) :
	start_position(_start_position),
	target_position(_target_position),
	approach_time(_approach_time),
	accuracy(_accuracy),
	note_texture(_note_texture),
	note_color(_note_color)
{
	target = GameObject(_target_position, _note_texture);
	target.setColor(sf::Color(_note_color.r,
							  _note_color.g,
							  _note_color.b,
							  128));
}