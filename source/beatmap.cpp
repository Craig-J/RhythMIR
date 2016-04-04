#include "beatmap.h"

Note::Note(sf::Time & _offset) :
	offset(_offset)
{}

TimingSection::TimingSection(float _BPM, sf::Time _offset) :
	BPM(_BPM),
	offset(_offset)
{}

NoteObject::NoteObject(sf::Vector2f & _start_position,
		   sf::Vector2f & _target_position,
		   sf::Time & _approach_time,
		   TexturePtr _texture,
		   sf::Color _color) :
	GameObject(_start_position, _texture)
{
	sf::Vector2f difference = _target_position - _start_position;
	Accelerate(difference / _approach_time.asSeconds(), false);
	if (_color != sf::Color::White)
		setColor(_color);
	offset_from_perfect = _approach_time;
}

NotePath::NotePath(sf::Vector2f & _start_position,
				   sf::Vector2f & _target_position,
				   sf::Time & _approach_time,
				   int _accuracy,
				   TexturePtr _note_texture,
				   sf::Color _note_color) :
	start_position(_start_position),
	target_position(_target_position),
	approach_time(_approach_time),
	accuracy(_accuracy),
	note_texture(_note_texture),
	note_color(_note_color)
{
	target = GameObject(_target_position, _note_texture);
	target.setColor(sf::Color(_note_color.r,
							  _note_color.g,
							  _note_color.b,
							  128));
}

Beatmap::Beatmap(const Song& _song,
				 const PLAYMODE& _mode,
				 const std::string& _name,
				 const std::string& _description) :
	song_(_song),
	play_mode_(_mode),
	name_(_name),
	description_(_description),
	music_(nullptr),
	sections_(nullptr)
{}

Beatmap::~Beatmap()
{
	if(music_)
		Global::AudioManager.UnloadMusic(song_.full_file_path());
	if (sections_)
	{
		delete sections_;
		sections_ = nullptr;
	}
}

std::queue<TimingSection> Beatmap::CopyTimingSections()
{
	std::queue<TimingSection> copy;
	for (auto section : *sections_)
	{
		copy.push(section);
	}
	return copy;
}

void Beatmap::LoadMusic()
{
	music_ = Global::AudioManager.LoadMusic(song_.full_file_path());
}