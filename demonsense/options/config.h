#pragma once
#include "../helpers/math.h"
#include "../helpers/utils.h"
#include "../menu/menu.h"
#include "../helpers/input.h"
#include <string>
#include <vector>

class int_v
{
public:
	std::string text_full;
	std::string text;
	int& v;
	int line_id;
	int group_id;
};

class float_v
{
public:
	std::string text_full;
	std::string text;
	float& v;
	int line_id;
	int group_id;
};

class char_v
{
public:
	std::string text_full;
	std::string text;
	char& v;
	int line_id;
	int group_id;
};


class bool_v
{
public:
	std::string text_full;
	std::string text;
	bool& v;
	int line_id;
	int group_id;
};

class color_v
{
public:
	std::string text_full;
	std::string text;
	Color& v;
	int line_id;
	int group_id;
};

class bind_v
{
public:
	std::string text_full;
	std::string text;
	KeyBind_t& v;
	int line_id;
	int group_id;
};

class string_v
{
public:
	std::string text_full;
	std::string text;
	std::string& v;
	int line_id;
	int group_id;
};

class window_v
{
public:
	std::string text_full;
	std::string text;
	std::string window_name;
	float& x;
	float& y;
	int line_id;
	int group_id;
};


namespace config
{
	namespace c_utils
	{
		std::string split_line(std::string temp);

		void add_var(int& cur_line, int& cur_group, std::vector<std::string>& config, std::vector<int_v>& vars, std::string name, int& var);
		void add_var(int& cur_line, int& cur_group, std::vector<std::string>& config, std::vector<char_v>& vars, std::string name, char& var);
		void add_var(int& cur_line, int& cur_group, std::vector<std::string>& config, std::vector<bool_v>& vars, std::string name, bool& var);
		void add_var(int& cur_line, int& cur_group, std::vector<std::string>& config, std::vector<float_v>& vars, std::string name, float& var);
		void add_var(int& cur_line, int& cur_group, std::vector<std::string>& config, std::vector<color_v>& vars, std::string name, Color& var);
		void add_var(int& cur_line, int& cur_group, std::vector<std::string>& config, std::vector<bind_v>& vars, std::string name, KeyBind_t& var);
		void add_var(int& cur_line, int& cur_group, std::vector<std::string>& config, std::vector<string_v>& vars, std::string name, std::string& var);
		void add_var(int& cur_line, int& cur_group, std::vector<std::string>& config, std::vector<window_v>& vars, std::string name, std::string wind, float& x, float& y);

		void add_space(int& cur_group, std::vector<std::string>& config);

		void add_group(int& cur_line, int& cur_group, std::vector<std::string>& config, std::string name);
		void end_group(int& cur_group);

		void save(std::string path, std::vector<std::string>& config, std::string name);
		void load(std::string path, std::string name, std::vector<int_v>& ints, std::vector<bool_v>& bools, std::vector<float_v>& floats, std::vector<color_v>& colors, std::vector<char_v>& chars, std::vector<bind_v>& binds, std::vector<string_v>& strings, std::vector<window_v>& windows);
	}

	namespace standart
	{
		int cur_group = 0;
		int cur_line = 0;

		std::vector<std::string> config;

		std::vector<int_v> ints;
		std::vector<bind_v> binds;
		std::vector<bool_v> bools;
		std::vector<char_v> chars;
		std::vector<float_v> floats;
		std::vector<color_v> colors;
		std::vector<string_v> strings;
		std::vector<window_v> windows;

		void setup_set();

		void add_var(std::string name, int& var);
		void add_var(std::string name, char& var);
		void add_var(std::string name, bool& var);
		void add_var(std::string name, float& var);
		void add_var(std::string name, Color& var);
		void add_var(std::string name, KeyBind_t& var);
		void add_var(std::string name, std::string& var);
		void add_var(std::string name, std::string wind, float& x, float& y);

		void add_space();

		void add_group(std::string name);
		void end_group();

		void clear();

		void save(std::string name);
		void load(std::string name);
	}
	namespace skins
	{
		int cur_group = 0;
		int cur_line = 0;

		std::vector<std::string> config;

		std::vector<int_v> ints;
		std::vector<bind_v> binds;
		std::vector<bool_v> bools;
		std::vector<float_v> floats;
		std::vector<color_v> colors;
		std::vector<char_v> chars;

		std::vector<string_v> strings;
		std::vector<window_v> windows;


		void setup_set();

		void add_var(std::string name, int& var);
		void add_var(std::string name, char& var);
		void add_var(std::string name, bool& var);
		void add_var(std::string name, float& var);
		void add_var(std::string name, Color& var);
		void add_var(std::string name, KeyBind_t& var);
		void add_var(std::string name, std::string& var);
		void add_var(std::string name, std::string wind, float& x, float& y);

		void add_space();

		void add_group(std::string name);
		void end_group();

		void clear();

		void save(std::string name);
		void load(std::string name);
	}
}