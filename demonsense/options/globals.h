#pragma once
#include "options.h"

typedef int bind_info_flags;

enum bind_info_flags_
{
	bind_info_flags_none = 0,
	bind_info_flags_standart = 1 << 0,
	bind_info_flags_side = 1 << 1
};

struct bind_info
{
	std::string name;
	bool enable;
	bind_info_flags flag;
};

struct bullet_impact_t
{
	Vector pos;
	float size;
	float delete_time;
	Color col;
};

struct damage_indicator_t
{
	int damage;
	bool initialized;
	float erase_time;
	float last_update;
	c_base_player* player;
	int hit_box;
	Color col;
	Vector position;
	Vector end_position;
};

namespace globals
{
	namespace bind_window
	{
		std::vector<bind_info*> binds;
	}

	namespace bullet_impact
	{
		std::vector<bullet_impact_t> bullet_impacts;
	}

	namespace damage_indicator
	{
		std::vector<damage_indicator_t> indicators;
	}

	namespace profile
	{
		bool update_request = false;
	}

	namespace misc
	{
		bool bSendPacket = true;

		bool bomb_defusing_with_kits = false;
	}

	namespace aa
	{
		float view_angle = 0;
		float real_angle = 0;
	}
}