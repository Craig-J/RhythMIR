#pragma once

// This file is used to include everything for an audio-visual application using
// SFML and SFML Extensions. SFML Extensions is dependent on the Agnostic library.
// This file also serves as a home for global objects.

// Graphics, Window and System includes.
#include <SFML_Extensions/Window/input_manager.h>
#include <SFML_Extensions/Graphics/texture_manager.h>
#include <SFML_Extensions/Graphics/sprite.h>
#include <SFML/Graphics.hpp>

// Audio includes.
#include <SFML_Extensions/Audio/audio_manager.h>
#include <SFML/Audio.hpp>

namespace sfx
{
	class Global
	{
	public:
		static InputManager Input;
		static TextureManager TextureManager;
		static AudioManager AudioManager;
		static void UnloadGlobalResources();
	};
}

