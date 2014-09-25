#pragma once

#include "variant.hpp"


class variant_builder
{
public:
	template<typename T> variant_builder& add(const std::string& name, const T& value)
	{
		return add_value(name, variant(value));
	}

	template<typename T> variant_builder& add(const std::string& name, T& value)
	{
		return add_value(name, variant(value));
	}

	template<typename T> variant_builder& set(const std::string& name, const T& value)
	{
		return set_value(name, variant(value));
	}

	template<typename T> variant_builder& set(const std::string& name, T& value)
	{
		return set_value(name, variant(value));
	}

	variant build();
	variant_builder& clear();
private:
	variant_builder& add_value(const std::string& name, const variant& value);
	variant_builder& set_value(const std::string& name, const variant& value);

	std::map<variant, std::vector<variant>> attr_;
};


template<> inline variant_builder& variant_builder::add(const std::string& name, const variant& value)
{
	return add_value(name, value);
}

template<> inline variant_builder& variant_builder::add(const std::string& name, variant& value)
{
	return add_value(name, value);
}
