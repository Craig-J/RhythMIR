#ifndef _SFX_TEXTURE_MANAGER_H_
#define _SFX_TEXTURE_MANAGER_H_

#include <agnostic\resource_manager.h>
#include <SFML\Graphics\Texture.hpp>
#include <agnostic\logger.h>
using agn::Log;
#include <tuple>

namespace sfx
{
	class Texture : public sf::Texture
	{
	public:
		Texture(const std::string& _file_name)
		{
			if (!loadFromFile(_file_name))
			{
				std::string error;
				error.append(_file_name);
				error.append(" failed to load from file in Texture constructor.");
				Log::Error(error);
			}
		}
	};

	class TextureManager
	{
	public:
		std::shared_ptr<Texture> LoadTexture(const std::string& _file_name)
		{
			return manager_.Load(_file_name);
		}

		void UnloadTexture(const std::string& _file_name)
		{
			manager_.Unload(_file_name);
		}

		inline void UnloadAllTextures()
		{
			manager_.Clear();
		}

	private:
		agn::ResourceManager <std::string, Texture> manager_;
	};
}



#endif // _SFX_TEXTURE_MANAGER_H_