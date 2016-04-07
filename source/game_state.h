#pragma once
#include "appstate.h"
#include "game_object.h"
#include "beatmap.h"
#include <SFML_Extensions\System\clock.h>
#include <SFML_Extensions\Graphics\hsl.h>
#include "game_settings.h"

class GameState : public AppState
{
public:

	GameState(GameStateMachine&,
			  UniqueStatePtr<AppState>&,
			  std::unique_ptr<Beatmap>,
			  GameSettings);
	virtual ~GameState() {}
	
	void InitializeState();
	void TerminateState();
	bool Update(const float _delta_time);
	void Render(const float _delta_time);
	void ProcessEvent(sf::Event & _event);
	
private:

	bool PauseMenu();

	GameSettings settings_;

	void SpawnBeat();
	void SpawnNote(NotePath& _path);
	void AttemptNoteHit(NotePath& _path);

	std::unique_ptr<Beatmap> beatmap_;

	std::vector<NotePath> note_paths_;
	std::queue<TimingSection> sections_;
	std::unique_ptr<TimingSection> current_section_;
	std::unique_ptr<std::queue<Note>> beatqueue_;
	std::vector<NoteObject> beats_;

	std::vector<std::unique_ptr<sf::Sound>> active_sounds_;
	SoundPtr deep_hit_sound_;
	SoundPtr soft_hit_sound_;
	SoundPtr miss_sound_;

	sfx::Clock play_clock_;
	
	sfx::Sprite pause_background_;
	
	TexturePtr pause_background_texture_;
	TexturePtr white_circle_texture_;
	TexturePtr beat_texture_;

	sf::Text clock_text_;
	sf::Text countdown_text_;
	sf::Text score_text_;
	sf::Text hit_counters_text_;

	int perfect_hits_, great_hits_, good_hits_, misses_;
	int hit_combo_;
	unsigned long score_;
	bool paused_;
	bool finished_;
	bool hit_counters_;

	void InitializeFourKeyMode();
	void InitializeVisualizerMode();
	void PlayHitSound();
	void PlayMissSound();
};