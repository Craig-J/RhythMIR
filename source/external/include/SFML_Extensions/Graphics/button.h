#ifndef _SFX_BUTTON_H_
#define _SFX_BUTTON_H_
#include <SFML_Extensions\Graphics\sprite.h>
#include <SFML_Extensions\Graphics\texture_manager.h>

// TO-DO make a button derived class that executes a command
namespace sfx
{
	class Button : public Sprite
	{
	public:

		//Button();
		using Sprite::Sprite;
		virtual ~Button() {}

		void Select(bool state);
		void SetTextures(TexturePtr default_texture, TexturePtr highlighted_texture);

	private:

		TexturePtr textures[2];
		bool selected_;
	};
}
#endif // _SFX_BUTTON_H_