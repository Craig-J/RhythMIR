#include "Button.h"
#include <cstddef> // For NULL definition
#include <cja/utils.h>

Button::Button(sf::Texture* default_texture, sf::Texture* highlighted_texture) :
selected_(false)
{
	textures[0] = default_texture;
	textures[1] = highlighted_texture;
	setTexture(*textures[0]);
}


Button::~Button()
{
	DeleteNull(textures[0]);
	DeleteNull(textures[1]);
}

void Button::Select(bool state)
{
	selected_ = state;
	switch (selected_)
	{
	case true:
		setTexture(*textures[1]);
		break;
	case false:
		setTexture(*textures[0]);
		break;
	}
}