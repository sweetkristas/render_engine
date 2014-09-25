#pragma once

#include <map>
#include <string>

namespace module
{
	std::string map_file(const std::string& s);
	std::map<std::string,std::string>::const_iterator find(const std::map<std::string,std::string>& item, const std::string& name);
	void get_unique_filenames_under_dir(const std::string& path, std::map<std::string,std::string>* res);
}
