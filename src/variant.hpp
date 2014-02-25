#pragma once

#include <map>
#include <string>
#include <vector>
#include <cstdint>

class variant;
typedef std::map<variant,variant> variant_map;
typedef std::vector<variant> variant_list;

class variant
{
public:
	enum variant_type
	{
		VARIANT_TYPE_NULL,
		VARIANT_TYPE_BOOL,
		VARIANT_TYPE_INTEGER,
		VARIANT_TYPE_FLOAT,
		VARIANT_TYPE_STRING,
		VARIANT_TYPE_MAP,
		VARIANT_TYPE_LIST,
	};

	variant();
	variant(const variant&);
	explicit variant(int64_t);
	explicit variant(float);
	explicit variant(const std::string&);
	explicit variant(const variant_map&);
	explicit variant(const variant_list&);

	variant_type type() const { return type_; }
	std::string type_as_string() const;

	static variant from_bool(bool b);

	std::string as_string() const;
	int64_t as_int() const;
	float as_float() const;
	bool as_bool() const;
	const variant_list& as_list() const;
	const variant_map& as_map() const;

	variant_list& as_mutable_list();
	variant_map& as_mutable_map();

	bool is_string() const { return type_ == VARIANT_TYPE_STRING; }
	bool is_null() const { return type_ == VARIANT_TYPE_NULL; }
	bool is_bool() const { return type_ == VARIANT_TYPE_BOOL; }
	bool is_numeric() const { return is_int() || is_float(); }
	bool is_int() const { return type_ == VARIANT_TYPE_INTEGER; }
	bool is_float() const { return type_ == VARIANT_TYPE_FLOAT; }
	bool is_map() const { return type_ == VARIANT_TYPE_MAP; }
	bool is_list() const { return type_ == VARIANT_TYPE_LIST; }

	bool operator<(const variant&) const;
	bool operator>(const variant&) const;

	bool operator==(const variant&) const;
	bool operator!=(const variant&) const;

	bool operator==(const std::string&) const;
	bool operator==(int64_t) const;

	const variant& operator[](size_t n) const;
	const variant& operator[](const variant& v) const;
	const variant& operator[](const std::string& key) const;

	size_t num_elements() const;

	bool has_key(const variant& v) const;
	bool has_key(const std::string& key) const;
			
	void write_json(std::ostream& s, bool pretty=true, int indent=0) const;
protected:
private:
	variant_type type_;

	bool b_;
	int64_t i_;
	float f_;
	std::string s_;
	variant_map m_;
	variant_list l_;
};

std::ostream& operator<<(std::ostream& os, const variant& n);
