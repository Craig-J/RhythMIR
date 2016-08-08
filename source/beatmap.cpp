#include "beatmap.h"

#include <SFML_Extensions/global.h>

Note::Note(NoteType _type, sf::Time & _offset) :
	type(_type),
	offset(_offset)
{}

Beatmap::Beatmap(const Song& _song,
				 const std::string& _name,
				 const std::string& _description,
				 const PLAYMODE& _mode) :
	song_(_song),
	play_mode_(_mode),
	name_(_name),
	description_(_description),
	music_(nullptr)
{}

const std::vector<Section>& Beatmap::Sections() const
{
	return sections_;
}

std::shared_ptr<std::queue<Note>> Beatmap::Beats() const
{
	return beats_;
}

void Beatmap::SetSections(const std::vector<Section>& _sections)
{
	sections_ = _sections;
}

void Beatmap::SetBeats(const std::queue<Note>& _beats)
{
	beats_ = std::make_unique<std::queue<Note>>(_beats);
}

void Beatmap::LoadMusic()
{
	music_ = sfx::Global::AudioManager.LoadMusic(song_.full_file_path());
}

void Beatmap::UnloadMusic()
{
	sfx::Global::AudioManager.UnloadMusic(song_.full_file_path());
}
