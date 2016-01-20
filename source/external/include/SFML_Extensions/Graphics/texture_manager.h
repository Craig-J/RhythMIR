#ifndef _SFX_TEXTURE_MANAGER_H_
#define _SFX_TEXTURE_MANAGER_H_

#include <cja\resource_manager.h>
#include <SFML\Graphics\Texture.hpp>
#include <sstream>
#include <cja\utils.h>

namespace sfx
{
	class TextureFile : public sf::Texture
	{
		TextureFile(const string& file_name)
		{
			if (!loadFromFile(file_name))
			{
				std::stringstream message;
				message << "ERROR: " << file_name << " failed to load from file in TextureFile constructor.";
				die(message.str().c_str());
			}
		}
	};

	class TextureManager : public cja::ResourceManager < string, TextureFile >
	{
		sf::Texture* LoadTexture(const string& _file_name)
		{
			return static_cast<sf::Texture*>(Load(_file_name));
		}

		void UnloadTexture(const string& _file_name)
		{
			Unload(_file_name);
		}

		inline void UnloadAllTextures()
		{
			Clear();
		}
	};

}

#endif