#include "button.h"

namespace sfx
{
	/*Button::Button() :
		selected_(false)
	{
		
	}*/

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
	void Button::SetTextures(TexturePtr _default_texture, TexturePtr _highlighted_texture)
	{
		textures[0] = _default_texture;
		textures[1] = _highlighted_texture;
		setTexture(*textures[0]);
	}
}