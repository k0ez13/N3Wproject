#pragma once
#include "../../valve_sdk/sdk.h"
#include "../../valve_sdk/csgostructs.h"
namespace autowall
{
	struct fire_bullet_data_t
	{
		Vector src;
		trace_t enter_trace;
		Vector direction;
		c_tracefilter filter;
		float trace_length;
		float trace_length_remaining;
		float current_damage;
		int penetrate_count;
	};

	float get_damage(const Vector& vecPoint);
	bool simulate_fire_bullet(c_base_combat_weapon* pWeapon, fire_bullet_data_t&data);
	bool handle_bullet_penetration(c_cs_weapon_info* weaponInfo, fire_bullet_data_t&data);
	bool trace_to_exit(Vector &end, trace_t *enter_trace, Vector start, Vector dir, trace_t *exit_trace);
	bool did_hit_non_world_entity(c_base_player* player);
	void scale_damage(int hitbox, c_base_player* enemy, float weapon_armor_ratio, float &current_damage);
	float get_hitgroup_damage_multiplier(int hitbox);
	void clip_trace_to_players( const Vector& vecAbsStart, const Vector& vecAbsEnd, uint32_t mask, i_tracefilter* filter, trace_t* tr ) ;
}
