#pragma once
#include <string>
#include <tuple>

class Song
{
public:
	Song(const std::string& _source_file_name, const std::string& _artist, const std::string& _title) :
		source_file_name_(_source_file_name),
		artist_(_artist),
		title_(_title),
		beatmap_list_file_name_()
	{
	}

	std::string artist_;
	std::string title_;
	std::string source_file_name_;
	std::string beatmap_list_file_name_;

	bool operator==(const Song& _other) const
	{
		return(artist_ == _other.artist_ && title_ == _other.title_ && source_file_name_ == _other.source_file_name_ && beatmap_list_file_name_ == _other.beatmap_list_file_name_);
	}

	bool operator<(const Song& _other) const
	{
		return(std::tie(artist_, title_, source_file_name_, beatmap_list_file_name_) < std::tie(_other.artist_, _other.title_, _other.source_file_name_, _other.beatmap_list_file_name_));
	}
};