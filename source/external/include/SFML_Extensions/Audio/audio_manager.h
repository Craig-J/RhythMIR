#ifndef _SFX_AUDIOMANAGER_H_
#define _SFX_AUDIOMANAGER_H_

#include <agnostic\logger.h>
using agn::Log;
#include <agnostic\resource_manager.h>
#include <SFML\Audio.hpp>

namespace sfx
{
	class SoundFile : public sf::SoundBuffer
	{
	public:
		SoundFile(const std::string& _file_name)
		{
			if (!loadFromFile(_file_name))
			{
				std::string error;
				error.append(_file_name);
				error.append(" failed to load from file in SoundFile constructor.");
				Log::Error(error);
			}
		}
	};

	class MusicFile : public sf::Music
	{
	public:
		MusicFile(const std::string& _file_name)
		{
			if (!openFromFile(_file_name))
			{
				std::string error;
				error.append(_file_name);
				error.append(" failed to load from file in MusicFile constructor.");
				Log::Error(error);
			}
		}
	};

	class AudioManager
	{
		// LoadMusic
		// IN:		String key - file name of music to load
		// OUT:		Pointer to sf::Music instance using the music file
		sf::Music* LoadMusic(const std::string& _file_name)
		{
			return music_manager_.Load(_file_name);
		}

		// LoadSound
		// IN:		String key - file name of sound to load
		// OUT:		sf::Sound object loaded with sound buffer
		sf::Sound LoadSound(const std::string& _file_name)
		{
			// Construct a sound from the SoundFile buffer pointer and return it.
			return sf::Sound(*sound_manager_.Load(_file_name));
		}

		// UnloadMusic
		// IN:		String key - file name of music to unload
		void UnloadMusic(const std::string& _file_name)
		{
			music_manager_.Unload(_file_name);
		}

		// UnloadSound
		// IN:		String key - file name of sound to unload
		void UnloadSound(const std::string& _file_name)
		{
			sound_manager_.Unload(_file_name);
		}

		// UnloadAllAudio
		// Clears both the sound and music caches
		void UnloadAllAudio()
		{
			sound_manager_.Clear();
			music_manager_.Clear();
		}

	private:

		// Assuming only one manager caches each sound/music so cleaning everything up on destruction will be safe.
		~AudioManager()
		{
			UnloadAllAudio();
		}

		agn::ResourceManager <std::string, SoundFile > sound_manager_;
		agn::ResourceManager <std::string, MusicFile > music_manager_;
	};

}

#endif // _SFX_AUDIOMANAGER_H_