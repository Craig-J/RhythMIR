#ifndef _SFX_BUTTON_H_
#define _SFX_BUTTON_H_
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>

class Button : public sf::Sprite
{
public:

	Button(sf::Texture* default_texture, sf::Texture* highlighted_texture);
	virtual ~Button();

	void Select(bool state);

private:

	sf::Texture* textures[2];
	bool selected_;
};

#endif