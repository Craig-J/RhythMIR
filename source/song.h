#pragma once
#include <string>

class Song
{
public:
	Song(const std::string& _file_name, const std::string& _artist, const std::string& _title) :
		file_name_(_file_name),
		artist_(_artist),
		title_(_title)
	{
	}

	std::string file_name_;
	std::string artist_;
	std::string title_;
};