#include "filesystem.hpp"
#include "module.hpp"

namespace module
{
	std::string map_file(const std::string& s)
	{
		return s;
	}

	void get_unique_filenames_under_dir(const std::string& path, std::map<std::string,std::string>* res)
	{
		sys::get_unique_files(path, *res);
	}

	std::map<std::string,std::string>::const_iterator find(const std::map<std::string,std::string>& item, const std::string& name)
	{
		return item.find(name);
	}
}
