#pragma once

#include "beatmap.h"
#include "game_object.h"
#include "game_settings.h"
#include "appstate.h"

#include <SFML_Extensions/System/clock.h>
#include <SFML_Extensions/Graphics/hsl.h>

class GameState : public AppState
{
public:

	GameState(AppStateMachine&,
			  UniqueStatePtr<AppState>&,
			  std::unique_ptr<Beatmap>,
			  GameSettings);
	virtual ~GameState() {}
	
	void InitializeState();
	void TerminateState();
	bool Update(const float _delta_time);
	void Render(const float _delta_time);
	void ProcessEvent(sf::Event & _event);
	void ReloadSkin();
	
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
	sfx::SoundPtr deep_hit_sound_;
	sfx::SoundPtr soft_hit_sound_;
	sfx::SoundPtr miss_sound_;

	sfx::Clock play_clock_;
	
	sfx::Sprite pause_background_;
	
	sfx::TexturePtr pause_background_texture_;
	sfx::TexturePtr white_circle_texture_;
	sfx::TexturePtr beat_texture_;

	sf::Text clock_text_;
	sf::Text countdown_text_;
	sf::Text score_text_;
	sf::Text performance_text_;

	std::vector<int> hits_;
	int perfect_hits_, great_hits_, good_hits_, misses_;
	int hit_combo_, max_combo_;
	unsigned long score_;
	int earliest_hit_, latest_hit_;
	float average_hit_;
	float unstable_rate_;

	bool paused_;
	bool finished_;
	bool hit_counters_;

	void InitializeFourKeyMode();
	void InitializeVisualizationMode();

	void PlayHitSound();
	void PlayMissSound();
};
