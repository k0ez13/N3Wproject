#pragma once

#include <string>
#include "../options/options.h"
#include "../imgui/imgui.h"

inline bind_info* find_bind_info(std::string name)
{
	for (int i = 0; i < globals::bind_window::binds.size(); i++)
	{
		if (globals::bind_window::binds[i]->name == name)
		{
			return globals::bind_window::binds[i];
		}
	}
	return NULL;
}
inline bind_info* create_bind_info(std::string name, bool state, bind_info_flags flag)
{
	bind_info* bind_find = find_bind_info(name);

	if (bind_find  != NULL)
	{
		bind_find->enable = state;
		return bind_find;
	}

	bind_info* bind_inf = new bind_info{ name, state, flag };

	globals::bind_window::binds.push_back(bind_inf);

	return bind_inf;
}

inline void delete_bind_info(std::string name)
{
	for (int i = 0; i < globals::bind_window::binds.size(); i++)
	{
		if (globals::bind_window::binds[i]->name == name)
		{
			globals::bind_window::binds.erase(globals::bind_window::binds.begin() + i);
		}
	}
}
template <class T>
inline void create_bind(std::string name, T enable, KeyBind_t* bind, bind_info_flags flag)
{
	if (enable)
	{
		bind_info* bindinfo = create_bind_info(name, bind->enable, flag);

		if (bind->bind_type == KeyBind_type::always)
		{
			bind->enable = true;
		}
		else if (bind->bind_type == KeyBind_type::toggle)
		{
			if (input_sys::was_key_pressed(bind->key_id))
			{
				bind->enable = !bind->enable;
			}
		}
	    else if (bind->bind_type == KeyBind_type::press)
		{
		    if (input_sys::is_key_down(bind->key_id))
		    {
			    bind->enable = true;
		    }

		    else if (!input_sys::is_key_down(bind->key_id))
		    {
			    bind->enable = false;
		    }
		}
		else if (bind->bind_type == KeyBind_type::press_invers)
		{
		    if (input_sys::is_key_down(bind->key_id))
		    {
		    	bind->enable = false;
		    }

		    else if (!input_sys::is_key_down(bind->key_id))
		    {
			    bind->enable = true;
		    }
		}

		if (bind->key_id == 0 && bind->bind_type != KeyBind_type::always)
			bind->enable = false;
	}
	else if (!enable)
	{
		delete_bind_info(name);
		bind->enable = false;
	}
}

template <class T>
inline void create_bind(std::string name, T enable, KeyBind_t* bind, float* side, bind_info_flags flag)
{
	if (enable)
	{
		bind_info* bindinfo = create_bind_info(name, bind->enable, flag);

		if (input_sys::was_key_pressed(bind->key_id))
		{
			bind->enable = !bind->enable;

			*side = -*side;
		}
	}
	else if (!enable)
	{
		delete_bind_info(name);
	}
}

struct IDirect3DDevice9;

namespace menu
{
	bool              _visible;

	void initialize();
	void shutdown();

	void on_device_lost();
	void on_device_reset();

	void render();

	void toggle();

	bool is_visible() { return _visible; }
	ImGuiStyle        _style;

	void create_style();
}