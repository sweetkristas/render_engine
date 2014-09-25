#include "variant_utils.hpp"

variant_builder& variant_builder::add_value(const std::string& name, const variant& value)
{
	attr_[variant(name)].emplace_back(value);
	return *this;
}

variant_builder& variant_builder::set_value(const std::string& name, const variant& value)
{
	variant key(name);
	attr_.erase(key);
	attr_[key].emplace_back(value);
	return *this;
}

variant variant_builder::build()
{
	variant_map res;
	for(auto& i : attr_) {
		if(i.second.size() == 1) {
			res[i.first] = i.second[0];
		} else {
			res[i.first] = variant(&i.second);
		}
	}
	return variant(&res);
}

variant_builder& variant_builder::clear()
{
	attr_.clear();
	return *this;
}
