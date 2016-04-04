#pragma once
#include <string>
#include <tuple>

class Song
{
public:
	Song(const std::string& _artist,
		 const std::string& _title,
		 const std::string& _source_file_name,
		 const std::string& _path_overwrite = std::string()) :
		artist_(_artist),
		title_(_title),
		source_file_name_(_source_file_name)
	{
		if (_path_overwrite == std::string())
		{
			path_ = "songs/" + song_name() + "/";
		}
		else
		{
			path_ = _path_overwrite;
		}
	}

	std::string artist_;
	std::string title_;
	std::string source_file_name_;

	std::string song_name() const
	{
		return artist_ + " - " + title_;
	}

	std::string relative_path() const
	{
		return path_;
	}

	std::string full_file_path() const
	{
		return relative_path() + source_file_name_;
	}

	bool operator==(const Song& _other) const
	{
		return(std::tie(artist_, title_) == std::tie(_other.artist_, _other.title_));
	}

	bool operator!=(const Song& _other) const
	{
		return(std::tie(artist_, title_) != std::tie(_other.artist_, _other.title_));
	}

	bool operator<(const Song& _other) const
	{
		return(std::tie(artist_, title_) < std::tie(_other.artist_, _other.title_));
	}

private:
	std::string path_;
};