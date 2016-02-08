#ifndef _SFX_TEXTURE_MANAGER_H_
#define _SFX_TEXTURE_MANAGER_H_

#include <agnostic\resource_manager.h>
#include <SFML\Graphics\Texture.hpp>
#include <agnostic\logger.h>

namespace sfx
{
	class TextureFile : public sf::Texture
	{
	public:
		TextureFile(const std::string& file_name_)
		{
			if (!loadFromFile(file_name_))
			{
				std::string error;
				error.append(file_name_);
				error.append(" failed to load from file in TextureFile constructor.");
				agn::Log::Error(error);
			}
		}
	};

	class TextureManager : public agn::ResourceManager < std::string, TextureFile >
	{
		sf::Texture* LoadTexture(const std::string& _file_name)
		{
			return static_cast<sf::Texture*>(Load(_file_name));
		}

		void UnloadTexture(const std::string& _file_name)
		{
			Unload(_file_name);
		}

		inline void UnloadAllTextures()
		{
			Clear();
		}
	};
}

#endif // _SFX_TEXTURE_MANAGER_H_