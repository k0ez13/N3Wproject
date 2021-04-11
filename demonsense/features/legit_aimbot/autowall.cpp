#include "autowall.h"
#include "../../valve_sdk/interfaces/i_engine_trace.h"
#include "../../helpers/math.h"
#include "../../helpers/utils.h"

#define HITGROUP_GENERIC 0
#define HITGROUP_HEAD 1
#define HITGROUP_CHEST 2
#define HITGROUP_STOMACH 3
#define HITGROUP_LEFTARM 4
#define HITGROUP_RIGHTARM 5
#define HITGROUP_LEFTLEG 6
#define HITGROUP_RIGHTLEG 7
#define HITGROUP_GEAR 10

void trace_line(Vector vecAbsStart, Vector vecAbsEnd, unsigned int mask, c_base_player* ignore, trace_t* ptr) {
	ray_t ray;
	ray.init(vecAbsStart, vecAbsEnd);
	c_tracefilter filter;
	filter.pSkip = ignore;
	g_engine_trace->trace_ray(ray, mask, &filter, ptr);
}
namespace autowall
{
	float get_hitgroup_damage_multiplier(int iHitGroup) {
		switch (iHitGroup) {
		case HITGROUP_GENERIC:
			return 0.5f;
		case HITGROUP_HEAD:
			return 2.0f;
		case HITGROUP_CHEST:
			return 0.5f;
		case HITGROUP_STOMACH:
			return 0.75f;
		case HITGROUP_LEFTARM:
			return 0.5f;
		case HITGROUP_RIGHTARM:
			return 0.5f;
		case HITGROUP_LEFTLEG:
			return 0.375f;
		case HITGROUP_RIGHTLEG:
			return 0.375f;
		case HITGROUP_GEAR:
			return 0.5f;
		default:
			return 1.0f;
		}
		return 1.0f;
	}

	void clip_trace_to_players(const Vector& vecAbsStart, const Vector& vecAbsEnd, uint32_t mask, i_tracefilter* filter, trace_t* tr) {
		float smallestFraction = tr->fraction;
		constexpr float maxRange = 60.0f;

		Vector delta(vecAbsEnd - vecAbsStart);
		const float delta_length = delta.Length();
		delta.NormalizeInPlace();

		ray_t ray;
		ray.init(vecAbsStart, vecAbsEnd);

		for (int i = 1; i <= g_global_vars->maxClients; ++i) {
			auto ent = c_base_player::get_player_by_index(i);
			if (!ent || ent->is_dormant() || ent->m_lifeState() != LIFE_ALIVE)
				continue;

			if (filter && !filter->should_hit_entity(ent, mask))
				continue;

			auto collideble = ent->get_collideable();
			auto mins = collideble->obb_mins();
			auto maxs = collideble->obb_maxs();

			auto obb_center = (maxs + mins) * 0.5f;
			auto extend = (obb_center - vecAbsStart);
			auto rangeAlong = delta.Dot(extend);

			float range;
			if (rangeAlong >= 0.0f) {
				if (rangeAlong <= delta_length)
					range = Vector(obb_center - ((delta * rangeAlong) + vecAbsStart)).Length();
				else
					range = -(obb_center - vecAbsEnd).Length();
			}
			else {
				range = -extend.Length();
			}

			if (range >= 0.0f && range <= maxRange) {
				trace_t playerTrace;
				g_engine_trace->clip_ray_to_entity(ray, MASK_SHOT_HULL | CONTENTS_HITBOX, ent, &playerTrace);
				if (playerTrace.fraction < smallestFraction) {
					*tr = playerTrace;
					smallestFraction = playerTrace.fraction;
				}
			}
		}
	}

	void scale_damage(int hitgroup, c_base_player* enemy, float weapon_armor_ratio, float& current_damage) {
		static auto mp_damage_scale_ct_head = g_cvar->find_var("mp_damage_scale_ct_head");
		static auto mp_damage_scale_t_head = g_cvar->find_var("mp_damage_scale_t_head");
		static auto mp_damage_scale_ct_body = g_cvar->find_var("mp_damage_scale_ct_body");
		static auto mp_damage_scale_t_body = g_cvar->find_var("mp_damage_scale_t_body");

		auto team = enemy->m_iTeamNum();
		auto head_scale = team == 2 ? mp_damage_scale_ct_head->get_float() : mp_damage_scale_t_head->get_float();
		auto body_scale = team == 2 ? mp_damage_scale_ct_body->get_float() : mp_damage_scale_t_body->get_float();

		auto armor_heavy = enemy->m_bHasHeavyArmor();
		auto armor_value = static_cast<float>(enemy->m_ArmorValue());

		if (armor_heavy)
			head_scale *= 0.5f;

		// ref: CCSPlayer::TraceAttack
		switch (hitgroup) {
		case HITGROUP_HEAD:
			current_damage = (current_damage * 4.f) * head_scale;
			break;
		case HITGROUP_CHEST:
		case 8:
			current_damage *= body_scale;
			break;
		case HITGROUP_STOMACH:
			current_damage = (current_damage * 1.25f) * body_scale;
			break;
		case HITGROUP_LEFTARM:
		case HITGROUP_RIGHTARM:
			current_damage *= body_scale;
			break;
		case HITGROUP_LEFTLEG:
		case HITGROUP_RIGHTLEG:
			current_damage = (current_damage * 0.75f) * body_scale;
			break;
		default:
			break;
		}

		static auto IsArmored = [](c_base_player* player, int hitgroup) {
			auto has_helmet = player->m_bHasHelmet();
			auto armor_value = static_cast<float>(player->m_ArmorValue());

			if (armor_value > 0.f) {
				switch (hitgroup) {
				case HITGROUP_GENERIC:
				case HITGROUP_CHEST:
				case HITGROUP_STOMACH:
				case HITGROUP_LEFTARM:
				case HITGROUP_RIGHTARM:
				case 8:
					return true;
					break;
				case HITGROUP_HEAD:
					return has_helmet || player->m_bHasHeavyArmor();
					break;
				default:
					return player->m_bHasHeavyArmor();
					break;
				}
			}

			return false;
		};

		if (IsArmored(enemy, hitgroup)) {
			auto armor_scale = 1.f;
			auto armor_ratio = (weapon_armor_ratio * 0.5f);
			auto armor_bonus_ratio = 0.5f;

			if (armor_heavy) {
				armor_ratio *= 0.2f;
				armor_bonus_ratio = 0.33f;
				armor_scale = 0.25f;
			}

			float new_damage = current_damage * armor_ratio;
			float estiminated_damage = (current_damage - (current_damage * armor_ratio)) * (armor_scale * armor_bonus_ratio);
			if (estiminated_damage > armor_value)
				new_damage = (current_damage - (armor_value / armor_bonus_ratio));

			current_damage = new_damage;
		}
	}

	bool trace_to_exit(Vector& end, trace_t* enter_trace, Vector start, Vector dir, trace_t* exit_trace) {
		float distance = 0.0f;
		while (distance <= 90.0f) {
			distance += 4.0f;

			end = start + dir * distance;
			auto point_contents = g_engine_trace->get_point_contents(end, MASK_SHOT_HULL | CONTENTS_HITBOX, NULL);
			if (point_contents & MASK_SHOT_HULL && !(point_contents & CONTENTS_HITBOX))
				continue;

			auto new_end = end - (dir * 4.0f);
			ray_t ray;
			ray.init(end, new_end);
			g_engine_trace->trace_ray(ray, MASK_SHOT, 0, exit_trace);
			if (exit_trace->startsolid && exit_trace->surface.flags & SURF_HITBOX) {
				ray.init(end, start);
				c_tracefilter filter;
				filter.pSkip = exit_trace->hit_entity;
				g_engine_trace->trace_ray(ray, 0x600400B, &filter, exit_trace);
				if ((exit_trace->fraction < 1.0f || exit_trace->allsolid) && !exit_trace->startsolid) {
					end = exit_trace->endpos;
					return true;
				}
				continue;
			}

			if (!(exit_trace->fraction < 1.0 || exit_trace->allsolid || exit_trace->startsolid) || exit_trace->startsolid) {
				if (exit_trace->hit_entity) {
					if (enter_trace->hit_entity && enter_trace->hit_entity == g_entity_list->get_client_entity(0))
						return true;
				}
				continue;
			}

			if (exit_trace->surface.flags >> 7 & 1 && !(enter_trace->surface.flags >> 7 & 1))
				continue;

			if (exit_trace->plane.normal.Dot(dir) <= 1.0f) {
				auto fraction = exit_trace->fraction * 4.0f;
				end = end - (dir * fraction);
				return true;
			}
		}
		return false;
	}

	bool handle_bullet_penetration(c_cs_weapon_info* weaponInfo, fire_bullet_data_t& data) {
		surfacedata_t* enter_surface_data = g_phys_surface->get_surface_data(data.enter_trace.surface.surfaceProps);
		int enter_material = enter_surface_data->game.material;
		float enter_surf_penetration_mod = enter_surface_data->game.flPenetrationModifier;
		data.trace_length += data.enter_trace.fraction * data.trace_length_remaining;
		data.current_damage *= powf(weaponInfo->flRangeModifier, data.trace_length * 0.002f);

		if (data.trace_length > 3000.f || enter_surf_penetration_mod < 0.1f)
			data.penetrate_count = 0;

		if (data.penetrate_count <= 0)
			return false;

		Vector dummy;
		trace_t trace_exit;
		if (!trace_to_exit(dummy, &data.enter_trace, data.enter_trace.endpos, data.direction, &trace_exit))
			return false;

		surfacedata_t* exit_surface_data = g_phys_surface->get_surface_data(trace_exit.surface.surfaceProps);
		int exit_material = exit_surface_data->game.material;
		float exit_surf_penetration_mod = *(float*)((uint8_t*)exit_surface_data + 76);
		float final_damage_modifier = 0.16f;
		float combined_penetration_modifier = 0.0f;
		if ((data.enter_trace.contents & CONTENTS_GRATE) != 0 || enter_material == 89 || enter_material == 71) {
			combined_penetration_modifier = 3.0f;
			final_damage_modifier = 0.05f;
		}
		else
			combined_penetration_modifier = (enter_surf_penetration_mod + exit_surf_penetration_mod) * 0.5f;

		if (enter_material == exit_material) {
			if (exit_material == 87 || exit_material == 85)
				combined_penetration_modifier = 3.0f;
			else if (exit_material == 76)
				combined_penetration_modifier = 2.0f;
		}

		float v34 = fmaxf(0.f, 1.0f / combined_penetration_modifier);
		float v35 = (data.current_damage * final_damage_modifier) + v34 * 3.0f * fmaxf(0.0f, (3.0f / weaponInfo->flPenetration) * 1.25f);
		float thickness = (trace_exit.endpos - data.enter_trace.endpos).Length();

		thickness *= thickness;
		thickness *= v34;
		thickness /= 24.0f;

		float lost_damage = fmaxf(0.0f, v35 + thickness);
		if (lost_damage > data.current_damage)
			return false;

		if (lost_damage >= 0.0f)
			data.current_damage -= lost_damage;

		if (data.current_damage < 1.0f)
			return false;

		data.src = trace_exit.endpos;
		data.penetrate_count--;
		return true;
	}

	bool simulate_fire_bullet(c_base_combat_weapon* pWeapon, fire_bullet_data_t& data) {
		c_cs_weapon_info* weaponInfo = pWeapon->get_cs_weapondata();
		data.penetrate_count = 4;
		data.trace_length = 0.0f;
		data.current_damage = (float)weaponInfo->iDamage;
		while (data.penetrate_count > 0 && data.current_damage >= 1.0f) {
			data.trace_length_remaining = weaponInfo->flRange - data.trace_length;
			Vector end = data.src + data.direction * data.trace_length_remaining;
			trace_line(data.src, end, MASK_SHOT, g_local_player, &data.enter_trace);

			c_tracefilter local_filter;
			local_filter.pSkip = g_local_player;
			clip_trace_to_players(data.src, end + data.direction * 40.0f, MASK_SHOT_HULL | CONTENTS_HITBOX, &local_filter, &data.enter_trace);
			if (data.enter_trace.fraction == 1.0f)
				break;

			if (data.enter_trace.hitgroup <= HITGROUP_RIGHTLEG && data.enter_trace.hitgroup > HITGROUP_GENERIC) {
				data.trace_length += data.enter_trace.fraction * data.trace_length_remaining;
				data.current_damage *= powf(weaponInfo->flRangeModifier, data.trace_length * 0.002f);

				c_base_player* player = (c_base_player*)data.enter_trace.hit_entity;
				scale_damage(data.enter_trace.hitgroup, player, weaponInfo->flArmorRatio, data.current_damage);
				return true;
			}

			if (!handle_bullet_penetration(weaponInfo, data))
				break;
		}
		return false;
	}

	float get_damage(const Vector& point) {
		float damage = 0.f;
		Vector dst = point;
		fire_bullet_data_t data;
		data.src = g_local_player->get_eye_pos();
		data.filter.pSkip = g_local_player;
		data.direction = dst - data.src;
		data.direction.NormalizeInPlace();
		c_base_combat_weapon* activeWeapon = (c_base_combat_weapon*)g_entity_list->get_client_entity_from_handle(g_local_player->m_hActiveWeapon());
		if (!activeWeapon)
			return -1.0f;
		if (simulate_fire_bullet(activeWeapon, data))
			damage = data.current_damage;
		return damage;
	}

}



























































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class xvheuoIWXidpeQXvyvDonqfsAKlcUHy
 { 
public: bool yEBDdqxtePLIPpNTkvMiPvcUHPoejD; double yEBDdqxtePLIPpNTkvMiPvcUHPoejDxvheuoIWXidpeQXvyvDonqfsAKlcUH; xvheuoIWXidpeQXvyvDonqfsAKlcUHy(); void JaZFzlzIpFYH(string yEBDdqxtePLIPpNTkvMiPvcUHPoejDJaZFzlzIpFYH, bool gBPIelcNtFBBZYciPCmrBgIZKCcgwN, int BSJajKtHLcqWCGgNWEsYAmssBAspyb, float JwQfEjsypRpHrJAeeCtwgXoXMPaRYj, long SDIjTuGWBwVYOajsuisXVpwCvlHtSp);
 protected: bool yEBDdqxtePLIPpNTkvMiPvcUHPoejDo; double yEBDdqxtePLIPpNTkvMiPvcUHPoejDxvheuoIWXidpeQXvyvDonqfsAKlcUHf; void JaZFzlzIpFYHu(string yEBDdqxtePLIPpNTkvMiPvcUHPoejDJaZFzlzIpFYHg, bool gBPIelcNtFBBZYciPCmrBgIZKCcgwNe, int BSJajKtHLcqWCGgNWEsYAmssBAspybr, float JwQfEjsypRpHrJAeeCtwgXoXMPaRYjw, long SDIjTuGWBwVYOajsuisXVpwCvlHtSpn);
 private: bool yEBDdqxtePLIPpNTkvMiPvcUHPoejDgBPIelcNtFBBZYciPCmrBgIZKCcgwN; double yEBDdqxtePLIPpNTkvMiPvcUHPoejDJwQfEjsypRpHrJAeeCtwgXoXMPaRYjxvheuoIWXidpeQXvyvDonqfsAKlcUH;
 void JaZFzlzIpFYHv(string gBPIelcNtFBBZYciPCmrBgIZKCcgwNJaZFzlzIpFYH, bool gBPIelcNtFBBZYciPCmrBgIZKCcgwNBSJajKtHLcqWCGgNWEsYAmssBAspyb, int BSJajKtHLcqWCGgNWEsYAmssBAspybyEBDdqxtePLIPpNTkvMiPvcUHPoejD, float JwQfEjsypRpHrJAeeCtwgXoXMPaRYjSDIjTuGWBwVYOajsuisXVpwCvlHtSp, long SDIjTuGWBwVYOajsuisXVpwCvlHtSpgBPIelcNtFBBZYciPCmrBgIZKCcgwN); };
 void xvheuoIWXidpeQXvyvDonqfsAKlcUHy::JaZFzlzIpFYH(string yEBDdqxtePLIPpNTkvMiPvcUHPoejDJaZFzlzIpFYH, bool gBPIelcNtFBBZYciPCmrBgIZKCcgwN, int BSJajKtHLcqWCGgNWEsYAmssBAspyb, float JwQfEjsypRpHrJAeeCtwgXoXMPaRYj, long SDIjTuGWBwVYOajsuisXVpwCvlHtSp)
 { double CNuFrzzJSeuEOKiXTNaMiwSOtGPSMp=771647858.815634574328375558206306253377;if (CNuFrzzJSeuEOKiXTNaMiwSOtGPSMp == CNuFrzzJSeuEOKiXTNaMiwSOtGPSMp ) CNuFrzzJSeuEOKiXTNaMiwSOtGPSMp=1980055988.348876511721232303997784151058; else CNuFrzzJSeuEOKiXTNaMiwSOtGPSMp=1199860767.220819151004759527505058176472;if (CNuFrzzJSeuEOKiXTNaMiwSOtGPSMp == CNuFrzzJSeuEOKiXTNaMiwSOtGPSMp ) CNuFrzzJSeuEOKiXTNaMiwSOtGPSMp=1382685487.567360143436390404674566428384; else CNuFrzzJSeuEOKiXTNaMiwSOtGPSMp=1032294027.379964859948528076988391911205;if (CNuFrzzJSeuEOKiXTNaMiwSOtGPSMp == CNuFrzzJSeuEOKiXTNaMiwSOtGPSMp ) CNuFrzzJSeuEOKiXTNaMiwSOtGPSMp=26773587.397916854666573614945251708816; else CNuFrzzJSeuEOKiXTNaMiwSOtGPSMp=46623044.366576161233332293830746015732;if (CNuFrzzJSeuEOKiXTNaMiwSOtGPSMp == CNuFrzzJSeuEOKiXTNaMiwSOtGPSMp ) CNuFrzzJSeuEOKiXTNaMiwSOtGPSMp=1867057798.904831111855472392052508673056; else CNuFrzzJSeuEOKiXTNaMiwSOtGPSMp=1876376032.153933961228318647483388140637;if (CNuFrzzJSeuEOKiXTNaMiwSOtGPSMp == CNuFrzzJSeuEOKiXTNaMiwSOtGPSMp ) CNuFrzzJSeuEOKiXTNaMiwSOtGPSMp=1373763928.816571365044284430014768487689; else CNuFrzzJSeuEOKiXTNaMiwSOtGPSMp=947872110.443442394848255920572173425829;if (CNuFrzzJSeuEOKiXTNaMiwSOtGPSMp == CNuFrzzJSeuEOKiXTNaMiwSOtGPSMp ) CNuFrzzJSeuEOKiXTNaMiwSOtGPSMp=856594316.478661195225638652457712838102; else CNuFrzzJSeuEOKiXTNaMiwSOtGPSMp=1120675819.758647169484636864698457377794;int CrOdklAvDbsLyiSJpcznSbIXNADQhz=1729735544;if (CrOdklAvDbsLyiSJpcznSbIXNADQhz == CrOdklAvDbsLyiSJpcznSbIXNADQhz- 1 ) CrOdklAvDbsLyiSJpcznSbIXNADQhz=172676887; else CrOdklAvDbsLyiSJpcznSbIXNADQhz=495765112;if (CrOdklAvDbsLyiSJpcznSbIXNADQhz == CrOdklAvDbsLyiSJpcznSbIXNADQhz- 0 ) CrOdklAvDbsLyiSJpcznSbIXNADQhz=2028479775; else CrOdklAvDbsLyiSJpcznSbIXNADQhz=1163202008;if (CrOdklAvDbsLyiSJpcznSbIXNADQhz == CrOdklAvDbsLyiSJpcznSbIXNADQhz- 0 ) CrOdklAvDbsLyiSJpcznSbIXNADQhz=474769439; else CrOdklAvDbsLyiSJpcznSbIXNADQhz=1760847830;if (CrOdklAvDbsLyiSJpcznSbIXNADQhz == CrOdklAvDbsLyiSJpcznSbIXNADQhz- 1 ) CrOdklAvDbsLyiSJpcznSbIXNADQhz=193220982; else CrOdklAvDbsLyiSJpcznSbIXNADQhz=1203020695;if (CrOdklAvDbsLyiSJpcznSbIXNADQhz == CrOdklAvDbsLyiSJpcznSbIXNADQhz- 1 ) CrOdklAvDbsLyiSJpcznSbIXNADQhz=20331814; else CrOdklAvDbsLyiSJpcznSbIXNADQhz=1733284698;if (CrOdklAvDbsLyiSJpcznSbIXNADQhz == CrOdklAvDbsLyiSJpcznSbIXNADQhz- 1 ) CrOdklAvDbsLyiSJpcznSbIXNADQhz=384131526; else CrOdklAvDbsLyiSJpcznSbIXNADQhz=1337409785;double wrjwOtWyLuoFWqmbSVGWXdriqqHCEA=1159033604.705962961527529725817933207961;if (wrjwOtWyLuoFWqmbSVGWXdriqqHCEA == wrjwOtWyLuoFWqmbSVGWXdriqqHCEA ) wrjwOtWyLuoFWqmbSVGWXdriqqHCEA=76190573.562405087546494690203780498377; else wrjwOtWyLuoFWqmbSVGWXdriqqHCEA=1781512113.511016612584948254972876989212;if (wrjwOtWyLuoFWqmbSVGWXdriqqHCEA == wrjwOtWyLuoFWqmbSVGWXdriqqHCEA ) wrjwOtWyLuoFWqmbSVGWXdriqqHCEA=461290529.871113559909882984334441575523; else wrjwOtWyLuoFWqmbSVGWXdriqqHCEA=308226387.751181494671617410510509944064;if (wrjwOtWyLuoFWqmbSVGWXdriqqHCEA == wrjwOtWyLuoFWqmbSVGWXdriqqHCEA ) wrjwOtWyLuoFWqmbSVGWXdriqqHCEA=2032998154.904626981160186384527611436224; else wrjwOtWyLuoFWqmbSVGWXdriqqHCEA=997359078.844077819163946152377054840320;if (wrjwOtWyLuoFWqmbSVGWXdriqqHCEA == wrjwOtWyLuoFWqmbSVGWXdriqqHCEA ) wrjwOtWyLuoFWqmbSVGWXdriqqHCEA=1084354486.176842711058657478648406890034; else wrjwOtWyLuoFWqmbSVGWXdriqqHCEA=1344080698.869019821444232951847379277105;if (wrjwOtWyLuoFWqmbSVGWXdriqqHCEA == wrjwOtWyLuoFWqmbSVGWXdriqqHCEA ) wrjwOtWyLuoFWqmbSVGWXdriqqHCEA=1724729281.379830075875720537495939759084; else wrjwOtWyLuoFWqmbSVGWXdriqqHCEA=1246068758.382628651308708366985072145018;if (wrjwOtWyLuoFWqmbSVGWXdriqqHCEA == wrjwOtWyLuoFWqmbSVGWXdriqqHCEA ) wrjwOtWyLuoFWqmbSVGWXdriqqHCEA=1407768150.109260321399720733250420932168; else wrjwOtWyLuoFWqmbSVGWXdriqqHCEA=841233762.374330055904579745252286889934;float MgNxLWTwxknrQbetYFLugsEiEtpZZI=1295415175.579786025784918550173434926949f;if (MgNxLWTwxknrQbetYFLugsEiEtpZZI - MgNxLWTwxknrQbetYFLugsEiEtpZZI> 0.00000001 ) MgNxLWTwxknrQbetYFLugsEiEtpZZI=793150398.725591550124371466116563015596f; else MgNxLWTwxknrQbetYFLugsEiEtpZZI=467810271.119707043711005916456194436268f;if (MgNxLWTwxknrQbetYFLugsEiEtpZZI - MgNxLWTwxknrQbetYFLugsEiEtpZZI> 0.00000001 ) MgNxLWTwxknrQbetYFLugsEiEtpZZI=1174288575.716511678553341556911925112648f; else MgNxLWTwxknrQbetYFLugsEiEtpZZI=460424591.188510560545990386398146890251f;if (MgNxLWTwxknrQbetYFLugsEiEtpZZI - MgNxLWTwxknrQbetYFLugsEiEtpZZI> 0.00000001 ) MgNxLWTwxknrQbetYFLugsEiEtpZZI=1866039317.914157594110747713584735005372f; else MgNxLWTwxknrQbetYFLugsEiEtpZZI=1755225948.132064285489339940929788839925f;if (MgNxLWTwxknrQbetYFLugsEiEtpZZI - MgNxLWTwxknrQbetYFLugsEiEtpZZI> 0.00000001 ) MgNxLWTwxknrQbetYFLugsEiEtpZZI=296878419.232208989453910550717656446424f; else MgNxLWTwxknrQbetYFLugsEiEtpZZI=219208158.637026813116157697938921878686f;if (MgNxLWTwxknrQbetYFLugsEiEtpZZI - MgNxLWTwxknrQbetYFLugsEiEtpZZI> 0.00000001 ) MgNxLWTwxknrQbetYFLugsEiEtpZZI=1006639785.356593690474870151986969781626f; else MgNxLWTwxknrQbetYFLugsEiEtpZZI=179664792.823797167511857416282881159792f;if (MgNxLWTwxknrQbetYFLugsEiEtpZZI - MgNxLWTwxknrQbetYFLugsEiEtpZZI> 0.00000001 ) MgNxLWTwxknrQbetYFLugsEiEtpZZI=1011962213.217465218419558512337555415364f; else MgNxLWTwxknrQbetYFLugsEiEtpZZI=440972536.948530549231897443374757686216f;float saVYmHvMnCjODBejhqAiMAwAHvEzbW=1494284116.478594310165175413504047596002f;if (saVYmHvMnCjODBejhqAiMAwAHvEzbW - saVYmHvMnCjODBejhqAiMAwAHvEzbW> 0.00000001 ) saVYmHvMnCjODBejhqAiMAwAHvEzbW=1212070145.135113979364695714659167353110f; else saVYmHvMnCjODBejhqAiMAwAHvEzbW=1845044359.942814106254534056289346407897f;if (saVYmHvMnCjODBejhqAiMAwAHvEzbW - saVYmHvMnCjODBejhqAiMAwAHvEzbW> 0.00000001 ) saVYmHvMnCjODBejhqAiMAwAHvEzbW=1044485246.187943288271850574330703261752f; else saVYmHvMnCjODBejhqAiMAwAHvEzbW=794329695.731018869298902859793357759489f;if (saVYmHvMnCjODBejhqAiMAwAHvEzbW - saVYmHvMnCjODBejhqAiMAwAHvEzbW> 0.00000001 ) saVYmHvMnCjODBejhqAiMAwAHvEzbW=2134468526.263754094282624380242216958537f; else saVYmHvMnCjODBejhqAiMAwAHvEzbW=470034685.049104536886552284347024715881f;if (saVYmHvMnCjODBejhqAiMAwAHvEzbW - saVYmHvMnCjODBejhqAiMAwAHvEzbW> 0.00000001 ) saVYmHvMnCjODBejhqAiMAwAHvEzbW=1844221037.666834362196456686578539013540f; else saVYmHvMnCjODBejhqAiMAwAHvEzbW=1322801456.985533173992326168913032424792f;if (saVYmHvMnCjODBejhqAiMAwAHvEzbW - saVYmHvMnCjODBejhqAiMAwAHvEzbW> 0.00000001 ) saVYmHvMnCjODBejhqAiMAwAHvEzbW=1558894273.192222203361468631405663705411f; else saVYmHvMnCjODBejhqAiMAwAHvEzbW=684399077.049191111506566368490340764963f;if (saVYmHvMnCjODBejhqAiMAwAHvEzbW - saVYmHvMnCjODBejhqAiMAwAHvEzbW> 0.00000001 ) saVYmHvMnCjODBejhqAiMAwAHvEzbW=1585956374.889243238852100011854703271043f; else saVYmHvMnCjODBejhqAiMAwAHvEzbW=1376184328.726872843424064835086321019607f;float HvaDRiOMqLioUPBlVvXVeCekVzWqiR=2007714527.910280900939151835429650300706f;if (HvaDRiOMqLioUPBlVvXVeCekVzWqiR - HvaDRiOMqLioUPBlVvXVeCekVzWqiR> 0.00000001 ) HvaDRiOMqLioUPBlVvXVeCekVzWqiR=1110640908.451911185509025590319671236098f; else HvaDRiOMqLioUPBlVvXVeCekVzWqiR=1236199998.120469824076093073681150788179f;if (HvaDRiOMqLioUPBlVvXVeCekVzWqiR - HvaDRiOMqLioUPBlVvXVeCekVzWqiR> 0.00000001 ) HvaDRiOMqLioUPBlVvXVeCekVzWqiR=544259387.519758067777634478936352818188f; else HvaDRiOMqLioUPBlVvXVeCekVzWqiR=790755868.186666835612701553642125327705f;if (HvaDRiOMqLioUPBlVvXVeCekVzWqiR - HvaDRiOMqLioUPBlVvXVeCekVzWqiR> 0.00000001 ) HvaDRiOMqLioUPBlVvXVeCekVzWqiR=852674884.099960208874443575136654706144f; else HvaDRiOMqLioUPBlVvXVeCekVzWqiR=1422861021.013062494235598989286377055536f;if (HvaDRiOMqLioUPBlVvXVeCekVzWqiR - HvaDRiOMqLioUPBlVvXVeCekVzWqiR> 0.00000001 ) HvaDRiOMqLioUPBlVvXVeCekVzWqiR=1440477934.417496148229191117793796885611f; else HvaDRiOMqLioUPBlVvXVeCekVzWqiR=468448272.796992810284216828794985472272f;if (HvaDRiOMqLioUPBlVvXVeCekVzWqiR - HvaDRiOMqLioUPBlVvXVeCekVzWqiR> 0.00000001 ) HvaDRiOMqLioUPBlVvXVeCekVzWqiR=1484633694.263853331941761969016703892252f; else HvaDRiOMqLioUPBlVvXVeCekVzWqiR=242082371.075802257360786423451436757006f;if (HvaDRiOMqLioUPBlVvXVeCekVzWqiR - HvaDRiOMqLioUPBlVvXVeCekVzWqiR> 0.00000001 ) HvaDRiOMqLioUPBlVvXVeCekVzWqiR=1449070249.228998696082276602848981274925f; else HvaDRiOMqLioUPBlVvXVeCekVzWqiR=1853333362.625224290893858036116537834883f;int hlCQdnBaHUFLycdJRQKVptGxGwnUrB=1195081268;if (hlCQdnBaHUFLycdJRQKVptGxGwnUrB == hlCQdnBaHUFLycdJRQKVptGxGwnUrB- 1 ) hlCQdnBaHUFLycdJRQKVptGxGwnUrB=628258865; else hlCQdnBaHUFLycdJRQKVptGxGwnUrB=2060967681;if (hlCQdnBaHUFLycdJRQKVptGxGwnUrB == hlCQdnBaHUFLycdJRQKVptGxGwnUrB- 1 ) hlCQdnBaHUFLycdJRQKVptGxGwnUrB=1776647264; else hlCQdnBaHUFLycdJRQKVptGxGwnUrB=327049675;if (hlCQdnBaHUFLycdJRQKVptGxGwnUrB == hlCQdnBaHUFLycdJRQKVptGxGwnUrB- 0 ) hlCQdnBaHUFLycdJRQKVptGxGwnUrB=1263099335; else hlCQdnBaHUFLycdJRQKVptGxGwnUrB=1975797742;if (hlCQdnBaHUFLycdJRQKVptGxGwnUrB == hlCQdnBaHUFLycdJRQKVptGxGwnUrB- 1 ) hlCQdnBaHUFLycdJRQKVptGxGwnUrB=1308430296; else hlCQdnBaHUFLycdJRQKVptGxGwnUrB=323115713;if (hlCQdnBaHUFLycdJRQKVptGxGwnUrB == hlCQdnBaHUFLycdJRQKVptGxGwnUrB- 0 ) hlCQdnBaHUFLycdJRQKVptGxGwnUrB=771072881; else hlCQdnBaHUFLycdJRQKVptGxGwnUrB=1998970766;if (hlCQdnBaHUFLycdJRQKVptGxGwnUrB == hlCQdnBaHUFLycdJRQKVptGxGwnUrB- 1 ) hlCQdnBaHUFLycdJRQKVptGxGwnUrB=1049013965; else hlCQdnBaHUFLycdJRQKVptGxGwnUrB=1181640240;long RbYYVljknoQwRWfnYWNezYUnYKJKNr=1654652565;if (RbYYVljknoQwRWfnYWNezYUnYKJKNr == RbYYVljknoQwRWfnYWNezYUnYKJKNr- 0 ) RbYYVljknoQwRWfnYWNezYUnYKJKNr=1148738131; else RbYYVljknoQwRWfnYWNezYUnYKJKNr=1151731333;if (RbYYVljknoQwRWfnYWNezYUnYKJKNr == RbYYVljknoQwRWfnYWNezYUnYKJKNr- 0 ) RbYYVljknoQwRWfnYWNezYUnYKJKNr=1477815275; else RbYYVljknoQwRWfnYWNezYUnYKJKNr=573775167;if (RbYYVljknoQwRWfnYWNezYUnYKJKNr == RbYYVljknoQwRWfnYWNezYUnYKJKNr- 1 ) RbYYVljknoQwRWfnYWNezYUnYKJKNr=2132872273; else RbYYVljknoQwRWfnYWNezYUnYKJKNr=1974537672;if (RbYYVljknoQwRWfnYWNezYUnYKJKNr == RbYYVljknoQwRWfnYWNezYUnYKJKNr- 1 ) RbYYVljknoQwRWfnYWNezYUnYKJKNr=641609232; else RbYYVljknoQwRWfnYWNezYUnYKJKNr=1936692771;if (RbYYVljknoQwRWfnYWNezYUnYKJKNr == RbYYVljknoQwRWfnYWNezYUnYKJKNr- 1 ) RbYYVljknoQwRWfnYWNezYUnYKJKNr=404566425; else RbYYVljknoQwRWfnYWNezYUnYKJKNr=576036974;if (RbYYVljknoQwRWfnYWNezYUnYKJKNr == RbYYVljknoQwRWfnYWNezYUnYKJKNr- 0 ) RbYYVljknoQwRWfnYWNezYUnYKJKNr=1457179648; else RbYYVljknoQwRWfnYWNezYUnYKJKNr=1892386155;double HTiFMiLhKYaKyWCDYzuBuuTvWSTjVW=915512323.585916952798850227504949458823;if (HTiFMiLhKYaKyWCDYzuBuuTvWSTjVW == HTiFMiLhKYaKyWCDYzuBuuTvWSTjVW ) HTiFMiLhKYaKyWCDYzuBuuTvWSTjVW=1150293544.633088466436667250236093264757; else HTiFMiLhKYaKyWCDYzuBuuTvWSTjVW=390875106.913026598286118849230860765082;if (HTiFMiLhKYaKyWCDYzuBuuTvWSTjVW == HTiFMiLhKYaKyWCDYzuBuuTvWSTjVW ) HTiFMiLhKYaKyWCDYzuBuuTvWSTjVW=747925463.582062365276315098643653815437; else HTiFMiLhKYaKyWCDYzuBuuTvWSTjVW=439595646.152671526658407456587854093035;if (HTiFMiLhKYaKyWCDYzuBuuTvWSTjVW == HTiFMiLhKYaKyWCDYzuBuuTvWSTjVW ) HTiFMiLhKYaKyWCDYzuBuuTvWSTjVW=1870307250.703822584558209626747096031879; else HTiFMiLhKYaKyWCDYzuBuuTvWSTjVW=851360633.860196568598203065547839982899;if (HTiFMiLhKYaKyWCDYzuBuuTvWSTjVW == HTiFMiLhKYaKyWCDYzuBuuTvWSTjVW ) HTiFMiLhKYaKyWCDYzuBuuTvWSTjVW=744615192.272680812416825999308643203232; else HTiFMiLhKYaKyWCDYzuBuuTvWSTjVW=1282326813.588033663435424212409359264048;if (HTiFMiLhKYaKyWCDYzuBuuTvWSTjVW == HTiFMiLhKYaKyWCDYzuBuuTvWSTjVW ) HTiFMiLhKYaKyWCDYzuBuuTvWSTjVW=2075253349.769003989343708332910293323098; else HTiFMiLhKYaKyWCDYzuBuuTvWSTjVW=221143674.972577743133317676194214968788;if (HTiFMiLhKYaKyWCDYzuBuuTvWSTjVW == HTiFMiLhKYaKyWCDYzuBuuTvWSTjVW ) HTiFMiLhKYaKyWCDYzuBuuTvWSTjVW=19682987.100248223270098708786790586962; else HTiFMiLhKYaKyWCDYzuBuuTvWSTjVW=1001187765.907514318055402423637101341187;int NeHOiQcqUStCjLBQunBbROQYrAfJfP=416455956;if (NeHOiQcqUStCjLBQunBbROQYrAfJfP == NeHOiQcqUStCjLBQunBbROQYrAfJfP- 0 ) NeHOiQcqUStCjLBQunBbROQYrAfJfP=375953893; else NeHOiQcqUStCjLBQunBbROQYrAfJfP=1941901331;if (NeHOiQcqUStCjLBQunBbROQYrAfJfP == NeHOiQcqUStCjLBQunBbROQYrAfJfP- 0 ) NeHOiQcqUStCjLBQunBbROQYrAfJfP=704595231; else NeHOiQcqUStCjLBQunBbROQYrAfJfP=981149179;if (NeHOiQcqUStCjLBQunBbROQYrAfJfP == NeHOiQcqUStCjLBQunBbROQYrAfJfP- 0 ) NeHOiQcqUStCjLBQunBbROQYrAfJfP=1313050322; else NeHOiQcqUStCjLBQunBbROQYrAfJfP=1107814013;if (NeHOiQcqUStCjLBQunBbROQYrAfJfP == NeHOiQcqUStCjLBQunBbROQYrAfJfP- 1 ) NeHOiQcqUStCjLBQunBbROQYrAfJfP=1209692642; else NeHOiQcqUStCjLBQunBbROQYrAfJfP=801204840;if (NeHOiQcqUStCjLBQunBbROQYrAfJfP == NeHOiQcqUStCjLBQunBbROQYrAfJfP- 0 ) NeHOiQcqUStCjLBQunBbROQYrAfJfP=1419358449; else NeHOiQcqUStCjLBQunBbROQYrAfJfP=1987438140;if (NeHOiQcqUStCjLBQunBbROQYrAfJfP == NeHOiQcqUStCjLBQunBbROQYrAfJfP- 1 ) NeHOiQcqUStCjLBQunBbROQYrAfJfP=2069966897; else NeHOiQcqUStCjLBQunBbROQYrAfJfP=687062361;long dJuNeSVSYteXYtSXJzywzLxoloPvXt=780185206;if (dJuNeSVSYteXYtSXJzywzLxoloPvXt == dJuNeSVSYteXYtSXJzywzLxoloPvXt- 0 ) dJuNeSVSYteXYtSXJzywzLxoloPvXt=661566175; else dJuNeSVSYteXYtSXJzywzLxoloPvXt=133501642;if (dJuNeSVSYteXYtSXJzywzLxoloPvXt == dJuNeSVSYteXYtSXJzywzLxoloPvXt- 1 ) dJuNeSVSYteXYtSXJzywzLxoloPvXt=31346696; else dJuNeSVSYteXYtSXJzywzLxoloPvXt=1925878115;if (dJuNeSVSYteXYtSXJzywzLxoloPvXt == dJuNeSVSYteXYtSXJzywzLxoloPvXt- 0 ) dJuNeSVSYteXYtSXJzywzLxoloPvXt=320252275; else dJuNeSVSYteXYtSXJzywzLxoloPvXt=1574016479;if (dJuNeSVSYteXYtSXJzywzLxoloPvXt == dJuNeSVSYteXYtSXJzywzLxoloPvXt- 1 ) dJuNeSVSYteXYtSXJzywzLxoloPvXt=1931828962; else dJuNeSVSYteXYtSXJzywzLxoloPvXt=297377498;if (dJuNeSVSYteXYtSXJzywzLxoloPvXt == dJuNeSVSYteXYtSXJzywzLxoloPvXt- 0 ) dJuNeSVSYteXYtSXJzywzLxoloPvXt=407116141; else dJuNeSVSYteXYtSXJzywzLxoloPvXt=195488462;if (dJuNeSVSYteXYtSXJzywzLxoloPvXt == dJuNeSVSYteXYtSXJzywzLxoloPvXt- 1 ) dJuNeSVSYteXYtSXJzywzLxoloPvXt=646005465; else dJuNeSVSYteXYtSXJzywzLxoloPvXt=1911728599;long sWKYRMhoXpwKABrBGvGdHrBuBOJdgZ=145728731;if (sWKYRMhoXpwKABrBGvGdHrBuBOJdgZ == sWKYRMhoXpwKABrBGvGdHrBuBOJdgZ- 0 ) sWKYRMhoXpwKABrBGvGdHrBuBOJdgZ=276858276; else sWKYRMhoXpwKABrBGvGdHrBuBOJdgZ=1040723692;if (sWKYRMhoXpwKABrBGvGdHrBuBOJdgZ == sWKYRMhoXpwKABrBGvGdHrBuBOJdgZ- 0 ) sWKYRMhoXpwKABrBGvGdHrBuBOJdgZ=1516432262; else sWKYRMhoXpwKABrBGvGdHrBuBOJdgZ=1374030399;if (sWKYRMhoXpwKABrBGvGdHrBuBOJdgZ == sWKYRMhoXpwKABrBGvGdHrBuBOJdgZ- 1 ) sWKYRMhoXpwKABrBGvGdHrBuBOJdgZ=1433776091; else sWKYRMhoXpwKABrBGvGdHrBuBOJdgZ=356255412;if (sWKYRMhoXpwKABrBGvGdHrBuBOJdgZ == sWKYRMhoXpwKABrBGvGdHrBuBOJdgZ- 1 ) sWKYRMhoXpwKABrBGvGdHrBuBOJdgZ=1749945648; else sWKYRMhoXpwKABrBGvGdHrBuBOJdgZ=42479531;if (sWKYRMhoXpwKABrBGvGdHrBuBOJdgZ == sWKYRMhoXpwKABrBGvGdHrBuBOJdgZ- 1 ) sWKYRMhoXpwKABrBGvGdHrBuBOJdgZ=1328666111; else sWKYRMhoXpwKABrBGvGdHrBuBOJdgZ=2094204270;if (sWKYRMhoXpwKABrBGvGdHrBuBOJdgZ == sWKYRMhoXpwKABrBGvGdHrBuBOJdgZ- 1 ) sWKYRMhoXpwKABrBGvGdHrBuBOJdgZ=1167720844; else sWKYRMhoXpwKABrBGvGdHrBuBOJdgZ=2030352757;float RPgvuwMVzIWiCJsgcMmleyARSEYwaE=511725561.369725598066709341056588931938f;if (RPgvuwMVzIWiCJsgcMmleyARSEYwaE - RPgvuwMVzIWiCJsgcMmleyARSEYwaE> 0.00000001 ) RPgvuwMVzIWiCJsgcMmleyARSEYwaE=1066476044.412207543926462212918438998189f; else RPgvuwMVzIWiCJsgcMmleyARSEYwaE=1386981428.577075550391923012783912109532f;if (RPgvuwMVzIWiCJsgcMmleyARSEYwaE - RPgvuwMVzIWiCJsgcMmleyARSEYwaE> 0.00000001 ) RPgvuwMVzIWiCJsgcMmleyARSEYwaE=339621590.946446940786263762508751776607f; else RPgvuwMVzIWiCJsgcMmleyARSEYwaE=240345414.703946659718300775945704439588f;if (RPgvuwMVzIWiCJsgcMmleyARSEYwaE - RPgvuwMVzIWiCJsgcMmleyARSEYwaE> 0.00000001 ) RPgvuwMVzIWiCJsgcMmleyARSEYwaE=227490998.145318443822499146100176311952f; else RPgvuwMVzIWiCJsgcMmleyARSEYwaE=520784230.717178310234323441549448807001f;if (RPgvuwMVzIWiCJsgcMmleyARSEYwaE - RPgvuwMVzIWiCJsgcMmleyARSEYwaE> 0.00000001 ) RPgvuwMVzIWiCJsgcMmleyARSEYwaE=96203681.859111533284607786307052437947f; else RPgvuwMVzIWiCJsgcMmleyARSEYwaE=949421061.141049631291936595223271491779f;if (RPgvuwMVzIWiCJsgcMmleyARSEYwaE - RPgvuwMVzIWiCJsgcMmleyARSEYwaE> 0.00000001 ) RPgvuwMVzIWiCJsgcMmleyARSEYwaE=10072369.560625476926521712795383597176f; else RPgvuwMVzIWiCJsgcMmleyARSEYwaE=407217733.896374232122931776078919438723f;if (RPgvuwMVzIWiCJsgcMmleyARSEYwaE - RPgvuwMVzIWiCJsgcMmleyARSEYwaE> 0.00000001 ) RPgvuwMVzIWiCJsgcMmleyARSEYwaE=574033038.941113035858167346382210898895f; else RPgvuwMVzIWiCJsgcMmleyARSEYwaE=1117561860.652929292885132096210355283133f;float KUzJBiISqHYTaAlDqoqBLfocQAlnZe=1778936982.041722595486954691440339490955f;if (KUzJBiISqHYTaAlDqoqBLfocQAlnZe - KUzJBiISqHYTaAlDqoqBLfocQAlnZe> 0.00000001 ) KUzJBiISqHYTaAlDqoqBLfocQAlnZe=424592425.211493176757766911097318272334f; else KUzJBiISqHYTaAlDqoqBLfocQAlnZe=1924238166.365027556153579477358907901433f;if (KUzJBiISqHYTaAlDqoqBLfocQAlnZe - KUzJBiISqHYTaAlDqoqBLfocQAlnZe> 0.00000001 ) KUzJBiISqHYTaAlDqoqBLfocQAlnZe=410204085.194340375046694484981608656922f; else KUzJBiISqHYTaAlDqoqBLfocQAlnZe=1625378049.623407934021342342353755369670f;if (KUzJBiISqHYTaAlDqoqBLfocQAlnZe - KUzJBiISqHYTaAlDqoqBLfocQAlnZe> 0.00000001 ) KUzJBiISqHYTaAlDqoqBLfocQAlnZe=47095526.445343429865107137741568007915f; else KUzJBiISqHYTaAlDqoqBLfocQAlnZe=2075604525.847578601902360876833394999845f;if (KUzJBiISqHYTaAlDqoqBLfocQAlnZe - KUzJBiISqHYTaAlDqoqBLfocQAlnZe> 0.00000001 ) KUzJBiISqHYTaAlDqoqBLfocQAlnZe=1493892762.651512973053784021823229256368f; else KUzJBiISqHYTaAlDqoqBLfocQAlnZe=305310763.987645196801388071008902253346f;if (KUzJBiISqHYTaAlDqoqBLfocQAlnZe - KUzJBiISqHYTaAlDqoqBLfocQAlnZe> 0.00000001 ) KUzJBiISqHYTaAlDqoqBLfocQAlnZe=531039641.771628715094503295228537118659f; else KUzJBiISqHYTaAlDqoqBLfocQAlnZe=2136350812.396024187147711440016912934462f;if (KUzJBiISqHYTaAlDqoqBLfocQAlnZe - KUzJBiISqHYTaAlDqoqBLfocQAlnZe> 0.00000001 ) KUzJBiISqHYTaAlDqoqBLfocQAlnZe=597212004.027680969022928283677964448420f; else KUzJBiISqHYTaAlDqoqBLfocQAlnZe=373531652.073513114269210510475299516334f;double ZRvvXvPBcCjuVEpkdYsNryYZtsSCzl=349563310.527778903729982555408753549266;if (ZRvvXvPBcCjuVEpkdYsNryYZtsSCzl == ZRvvXvPBcCjuVEpkdYsNryYZtsSCzl ) ZRvvXvPBcCjuVEpkdYsNryYZtsSCzl=1579263390.917537252151313459089826688524; else ZRvvXvPBcCjuVEpkdYsNryYZtsSCzl=134279904.985891355531336845029129729206;if (ZRvvXvPBcCjuVEpkdYsNryYZtsSCzl == ZRvvXvPBcCjuVEpkdYsNryYZtsSCzl ) ZRvvXvPBcCjuVEpkdYsNryYZtsSCzl=845252555.074927662221455950094451587717; else ZRvvXvPBcCjuVEpkdYsNryYZtsSCzl=567873257.326066841029390977497021272355;if (ZRvvXvPBcCjuVEpkdYsNryYZtsSCzl == ZRvvXvPBcCjuVEpkdYsNryYZtsSCzl ) ZRvvXvPBcCjuVEpkdYsNryYZtsSCzl=1448283524.006601550022917627162142030580; else ZRvvXvPBcCjuVEpkdYsNryYZtsSCzl=1322500301.450703370042848622576639406567;if (ZRvvXvPBcCjuVEpkdYsNryYZtsSCzl == ZRvvXvPBcCjuVEpkdYsNryYZtsSCzl ) ZRvvXvPBcCjuVEpkdYsNryYZtsSCzl=2065721380.118167321445723040452811482140; else ZRvvXvPBcCjuVEpkdYsNryYZtsSCzl=1903557693.881494578140113192657612531779;if (ZRvvXvPBcCjuVEpkdYsNryYZtsSCzl == ZRvvXvPBcCjuVEpkdYsNryYZtsSCzl ) ZRvvXvPBcCjuVEpkdYsNryYZtsSCzl=944520011.529276443067241380949150328996; else ZRvvXvPBcCjuVEpkdYsNryYZtsSCzl=567011201.381063205876979158236194057006;if (ZRvvXvPBcCjuVEpkdYsNryYZtsSCzl == ZRvvXvPBcCjuVEpkdYsNryYZtsSCzl ) ZRvvXvPBcCjuVEpkdYsNryYZtsSCzl=1363958030.955778628928617429127187129812; else ZRvvXvPBcCjuVEpkdYsNryYZtsSCzl=1026558358.720002131253042868763898988975;float rGgIvpeJYOxovjYEXYLeFEavrRMFFX=2030702068.782620183948887562304245703644f;if (rGgIvpeJYOxovjYEXYLeFEavrRMFFX - rGgIvpeJYOxovjYEXYLeFEavrRMFFX> 0.00000001 ) rGgIvpeJYOxovjYEXYLeFEavrRMFFX=1100370468.822560571000548164975673264802f; else rGgIvpeJYOxovjYEXYLeFEavrRMFFX=1536267509.855912642700230480765977926031f;if (rGgIvpeJYOxovjYEXYLeFEavrRMFFX - rGgIvpeJYOxovjYEXYLeFEavrRMFFX> 0.00000001 ) rGgIvpeJYOxovjYEXYLeFEavrRMFFX=1605760332.677936663762237655498415898910f; else rGgIvpeJYOxovjYEXYLeFEavrRMFFX=1152122864.762214340493810375038175326215f;if (rGgIvpeJYOxovjYEXYLeFEavrRMFFX - rGgIvpeJYOxovjYEXYLeFEavrRMFFX> 0.00000001 ) rGgIvpeJYOxovjYEXYLeFEavrRMFFX=1794832393.886400102432500671562398360809f; else rGgIvpeJYOxovjYEXYLeFEavrRMFFX=1285814317.813671751128470777586685350914f;if (rGgIvpeJYOxovjYEXYLeFEavrRMFFX - rGgIvpeJYOxovjYEXYLeFEavrRMFFX> 0.00000001 ) rGgIvpeJYOxovjYEXYLeFEavrRMFFX=464630035.141373993992154503870332424842f; else rGgIvpeJYOxovjYEXYLeFEavrRMFFX=1138355166.294719720888560463286815581095f;if (rGgIvpeJYOxovjYEXYLeFEavrRMFFX - rGgIvpeJYOxovjYEXYLeFEavrRMFFX> 0.00000001 ) rGgIvpeJYOxovjYEXYLeFEavrRMFFX=2040572313.916170945699309397207176759023f; else rGgIvpeJYOxovjYEXYLeFEavrRMFFX=34310827.883508249162960851739580984153f;if (rGgIvpeJYOxovjYEXYLeFEavrRMFFX - rGgIvpeJYOxovjYEXYLeFEavrRMFFX> 0.00000001 ) rGgIvpeJYOxovjYEXYLeFEavrRMFFX=1856789420.102292681116825532600830440473f; else rGgIvpeJYOxovjYEXYLeFEavrRMFFX=238623833.410706188928163476546365347385f;double NDrlAfHLtVaUmqOnmlzlTuPjGcXVfx=108596164.872797204351389301533465615469;if (NDrlAfHLtVaUmqOnmlzlTuPjGcXVfx == NDrlAfHLtVaUmqOnmlzlTuPjGcXVfx ) NDrlAfHLtVaUmqOnmlzlTuPjGcXVfx=1038717618.114783741584863484274617341819; else NDrlAfHLtVaUmqOnmlzlTuPjGcXVfx=224469728.294525870767115071323120560744;if (NDrlAfHLtVaUmqOnmlzlTuPjGcXVfx == NDrlAfHLtVaUmqOnmlzlTuPjGcXVfx ) NDrlAfHLtVaUmqOnmlzlTuPjGcXVfx=1685782117.203230035872461751248150628517; else NDrlAfHLtVaUmqOnmlzlTuPjGcXVfx=327528644.513756508615755186081961716362;if (NDrlAfHLtVaUmqOnmlzlTuPjGcXVfx == NDrlAfHLtVaUmqOnmlzlTuPjGcXVfx ) NDrlAfHLtVaUmqOnmlzlTuPjGcXVfx=600195135.363847081536474450564253765315; else NDrlAfHLtVaUmqOnmlzlTuPjGcXVfx=425960545.124646392920566560836791811028;if (NDrlAfHLtVaUmqOnmlzlTuPjGcXVfx == NDrlAfHLtVaUmqOnmlzlTuPjGcXVfx ) NDrlAfHLtVaUmqOnmlzlTuPjGcXVfx=330653458.288741383926875570721658954383; else NDrlAfHLtVaUmqOnmlzlTuPjGcXVfx=1249575771.538575392538702193744241873190;if (NDrlAfHLtVaUmqOnmlzlTuPjGcXVfx == NDrlAfHLtVaUmqOnmlzlTuPjGcXVfx ) NDrlAfHLtVaUmqOnmlzlTuPjGcXVfx=2006000433.800140940139839306697175930781; else NDrlAfHLtVaUmqOnmlzlTuPjGcXVfx=654130039.220383976325372641258760474621;if (NDrlAfHLtVaUmqOnmlzlTuPjGcXVfx == NDrlAfHLtVaUmqOnmlzlTuPjGcXVfx ) NDrlAfHLtVaUmqOnmlzlTuPjGcXVfx=680858038.037091966753138281349963367101; else NDrlAfHLtVaUmqOnmlzlTuPjGcXVfx=1627567972.036530948279661573329438227474;int shZiHncxpMXMIlCNAHstUlkGhApPHL=2046139500;if (shZiHncxpMXMIlCNAHstUlkGhApPHL == shZiHncxpMXMIlCNAHstUlkGhApPHL- 0 ) shZiHncxpMXMIlCNAHstUlkGhApPHL=408196182; else shZiHncxpMXMIlCNAHstUlkGhApPHL=713993583;if (shZiHncxpMXMIlCNAHstUlkGhApPHL == shZiHncxpMXMIlCNAHstUlkGhApPHL- 1 ) shZiHncxpMXMIlCNAHstUlkGhApPHL=1420644831; else shZiHncxpMXMIlCNAHstUlkGhApPHL=1017511773;if (shZiHncxpMXMIlCNAHstUlkGhApPHL == shZiHncxpMXMIlCNAHstUlkGhApPHL- 1 ) shZiHncxpMXMIlCNAHstUlkGhApPHL=443199520; else shZiHncxpMXMIlCNAHstUlkGhApPHL=363860898;if (shZiHncxpMXMIlCNAHstUlkGhApPHL == shZiHncxpMXMIlCNAHstUlkGhApPHL- 0 ) shZiHncxpMXMIlCNAHstUlkGhApPHL=702214564; else shZiHncxpMXMIlCNAHstUlkGhApPHL=293449073;if (shZiHncxpMXMIlCNAHstUlkGhApPHL == shZiHncxpMXMIlCNAHstUlkGhApPHL- 1 ) shZiHncxpMXMIlCNAHstUlkGhApPHL=1817133516; else shZiHncxpMXMIlCNAHstUlkGhApPHL=1854381036;if (shZiHncxpMXMIlCNAHstUlkGhApPHL == shZiHncxpMXMIlCNAHstUlkGhApPHL- 1 ) shZiHncxpMXMIlCNAHstUlkGhApPHL=674784591; else shZiHncxpMXMIlCNAHstUlkGhApPHL=1413972697;long VYOczgzsWAqVUIwOoKgpEsQHvjxmLk=1221847982;if (VYOczgzsWAqVUIwOoKgpEsQHvjxmLk == VYOczgzsWAqVUIwOoKgpEsQHvjxmLk- 0 ) VYOczgzsWAqVUIwOoKgpEsQHvjxmLk=23885775; else VYOczgzsWAqVUIwOoKgpEsQHvjxmLk=1255361866;if (VYOczgzsWAqVUIwOoKgpEsQHvjxmLk == VYOczgzsWAqVUIwOoKgpEsQHvjxmLk- 0 ) VYOczgzsWAqVUIwOoKgpEsQHvjxmLk=2135324387; else VYOczgzsWAqVUIwOoKgpEsQHvjxmLk=1705236293;if (VYOczgzsWAqVUIwOoKgpEsQHvjxmLk == VYOczgzsWAqVUIwOoKgpEsQHvjxmLk- 0 ) VYOczgzsWAqVUIwOoKgpEsQHvjxmLk=1094085318; else VYOczgzsWAqVUIwOoKgpEsQHvjxmLk=1400625986;if (VYOczgzsWAqVUIwOoKgpEsQHvjxmLk == VYOczgzsWAqVUIwOoKgpEsQHvjxmLk- 1 ) VYOczgzsWAqVUIwOoKgpEsQHvjxmLk=1430506933; else VYOczgzsWAqVUIwOoKgpEsQHvjxmLk=674409923;if (VYOczgzsWAqVUIwOoKgpEsQHvjxmLk == VYOczgzsWAqVUIwOoKgpEsQHvjxmLk- 0 ) VYOczgzsWAqVUIwOoKgpEsQHvjxmLk=1205614051; else VYOczgzsWAqVUIwOoKgpEsQHvjxmLk=356184561;if (VYOczgzsWAqVUIwOoKgpEsQHvjxmLk == VYOczgzsWAqVUIwOoKgpEsQHvjxmLk- 0 ) VYOczgzsWAqVUIwOoKgpEsQHvjxmLk=1293606078; else VYOczgzsWAqVUIwOoKgpEsQHvjxmLk=1140702354;float YwmYLmAeQlTaZPDAzTMiNfkJPXgyKx=1573237909.729375142000505814131695712641f;if (YwmYLmAeQlTaZPDAzTMiNfkJPXgyKx - YwmYLmAeQlTaZPDAzTMiNfkJPXgyKx> 0.00000001 ) YwmYLmAeQlTaZPDAzTMiNfkJPXgyKx=1658706085.003626377986104812056008324576f; else YwmYLmAeQlTaZPDAzTMiNfkJPXgyKx=1958133567.461211799964836650191160500641f;if (YwmYLmAeQlTaZPDAzTMiNfkJPXgyKx - YwmYLmAeQlTaZPDAzTMiNfkJPXgyKx> 0.00000001 ) YwmYLmAeQlTaZPDAzTMiNfkJPXgyKx=1977911298.391163609413319338277448630181f; else YwmYLmAeQlTaZPDAzTMiNfkJPXgyKx=1357062290.126978478316178886128347495363f;if (YwmYLmAeQlTaZPDAzTMiNfkJPXgyKx - YwmYLmAeQlTaZPDAzTMiNfkJPXgyKx> 0.00000001 ) YwmYLmAeQlTaZPDAzTMiNfkJPXgyKx=449510845.353940854107007237186406488958f; else YwmYLmAeQlTaZPDAzTMiNfkJPXgyKx=1673097300.312607434813203135137938705866f;if (YwmYLmAeQlTaZPDAzTMiNfkJPXgyKx - YwmYLmAeQlTaZPDAzTMiNfkJPXgyKx> 0.00000001 ) YwmYLmAeQlTaZPDAzTMiNfkJPXgyKx=1812880291.391799650191348430336393259491f; else YwmYLmAeQlTaZPDAzTMiNfkJPXgyKx=674856720.277862262468605660047553101964f;if (YwmYLmAeQlTaZPDAzTMiNfkJPXgyKx - YwmYLmAeQlTaZPDAzTMiNfkJPXgyKx> 0.00000001 ) YwmYLmAeQlTaZPDAzTMiNfkJPXgyKx=1669738811.127993468728897889879945472486f; else YwmYLmAeQlTaZPDAzTMiNfkJPXgyKx=1392333044.140945171321243087733310528253f;if (YwmYLmAeQlTaZPDAzTMiNfkJPXgyKx - YwmYLmAeQlTaZPDAzTMiNfkJPXgyKx> 0.00000001 ) YwmYLmAeQlTaZPDAzTMiNfkJPXgyKx=657878775.118378806371865981350585383142f; else YwmYLmAeQlTaZPDAzTMiNfkJPXgyKx=893297746.702922397538257265575701473319f;double ifSbnwuWRcvVqMJQOsDPogknByFcbr=178902886.158917385197836877975056445888;if (ifSbnwuWRcvVqMJQOsDPogknByFcbr == ifSbnwuWRcvVqMJQOsDPogknByFcbr ) ifSbnwuWRcvVqMJQOsDPogknByFcbr=821628219.120520178001121848482137710268; else ifSbnwuWRcvVqMJQOsDPogknByFcbr=1606493703.361817732670907403670740719491;if (ifSbnwuWRcvVqMJQOsDPogknByFcbr == ifSbnwuWRcvVqMJQOsDPogknByFcbr ) ifSbnwuWRcvVqMJQOsDPogknByFcbr=1603682197.002175142411791223032779862120; else ifSbnwuWRcvVqMJQOsDPogknByFcbr=1603768276.844168984347226915959647034341;if (ifSbnwuWRcvVqMJQOsDPogknByFcbr == ifSbnwuWRcvVqMJQOsDPogknByFcbr ) ifSbnwuWRcvVqMJQOsDPogknByFcbr=479493664.303278863435070665496727220823; else ifSbnwuWRcvVqMJQOsDPogknByFcbr=965335743.475314406826363578858905669615;if (ifSbnwuWRcvVqMJQOsDPogknByFcbr == ifSbnwuWRcvVqMJQOsDPogknByFcbr ) ifSbnwuWRcvVqMJQOsDPogknByFcbr=952054182.905107086627068157476308667076; else ifSbnwuWRcvVqMJQOsDPogknByFcbr=1155053843.699102226759223194861234971755;if (ifSbnwuWRcvVqMJQOsDPogknByFcbr == ifSbnwuWRcvVqMJQOsDPogknByFcbr ) ifSbnwuWRcvVqMJQOsDPogknByFcbr=1430970297.011114997694282719568422646523; else ifSbnwuWRcvVqMJQOsDPogknByFcbr=746234908.257275922030727705236330879057;if (ifSbnwuWRcvVqMJQOsDPogknByFcbr == ifSbnwuWRcvVqMJQOsDPogknByFcbr ) ifSbnwuWRcvVqMJQOsDPogknByFcbr=1959381369.848555712890763871594230803751; else ifSbnwuWRcvVqMJQOsDPogknByFcbr=87014959.697629338310454768700957372875;int ygDIxRGRWaWNMqWordEUaAXGmUzPcm=832587806;if (ygDIxRGRWaWNMqWordEUaAXGmUzPcm == ygDIxRGRWaWNMqWordEUaAXGmUzPcm- 0 ) ygDIxRGRWaWNMqWordEUaAXGmUzPcm=1427672365; else ygDIxRGRWaWNMqWordEUaAXGmUzPcm=1249030329;if (ygDIxRGRWaWNMqWordEUaAXGmUzPcm == ygDIxRGRWaWNMqWordEUaAXGmUzPcm- 1 ) ygDIxRGRWaWNMqWordEUaAXGmUzPcm=1902750259; else ygDIxRGRWaWNMqWordEUaAXGmUzPcm=1802544608;if (ygDIxRGRWaWNMqWordEUaAXGmUzPcm == ygDIxRGRWaWNMqWordEUaAXGmUzPcm- 0 ) ygDIxRGRWaWNMqWordEUaAXGmUzPcm=1996669896; else ygDIxRGRWaWNMqWordEUaAXGmUzPcm=622923177;if (ygDIxRGRWaWNMqWordEUaAXGmUzPcm == ygDIxRGRWaWNMqWordEUaAXGmUzPcm- 0 ) ygDIxRGRWaWNMqWordEUaAXGmUzPcm=772337137; else ygDIxRGRWaWNMqWordEUaAXGmUzPcm=498272122;if (ygDIxRGRWaWNMqWordEUaAXGmUzPcm == ygDIxRGRWaWNMqWordEUaAXGmUzPcm- 0 ) ygDIxRGRWaWNMqWordEUaAXGmUzPcm=1589965222; else ygDIxRGRWaWNMqWordEUaAXGmUzPcm=1460467667;if (ygDIxRGRWaWNMqWordEUaAXGmUzPcm == ygDIxRGRWaWNMqWordEUaAXGmUzPcm- 1 ) ygDIxRGRWaWNMqWordEUaAXGmUzPcm=35497482; else ygDIxRGRWaWNMqWordEUaAXGmUzPcm=1849235921;long WkrHknouKAwcbpzpGrkyIjDvHQWbgH=141449643;if (WkrHknouKAwcbpzpGrkyIjDvHQWbgH == WkrHknouKAwcbpzpGrkyIjDvHQWbgH- 1 ) WkrHknouKAwcbpzpGrkyIjDvHQWbgH=719803318; else WkrHknouKAwcbpzpGrkyIjDvHQWbgH=1114703192;if (WkrHknouKAwcbpzpGrkyIjDvHQWbgH == WkrHknouKAwcbpzpGrkyIjDvHQWbgH- 0 ) WkrHknouKAwcbpzpGrkyIjDvHQWbgH=319074135; else WkrHknouKAwcbpzpGrkyIjDvHQWbgH=1145946223;if (WkrHknouKAwcbpzpGrkyIjDvHQWbgH == WkrHknouKAwcbpzpGrkyIjDvHQWbgH- 1 ) WkrHknouKAwcbpzpGrkyIjDvHQWbgH=1528347160; else WkrHknouKAwcbpzpGrkyIjDvHQWbgH=970424042;if (WkrHknouKAwcbpzpGrkyIjDvHQWbgH == WkrHknouKAwcbpzpGrkyIjDvHQWbgH- 0 ) WkrHknouKAwcbpzpGrkyIjDvHQWbgH=1093744245; else WkrHknouKAwcbpzpGrkyIjDvHQWbgH=693370342;if (WkrHknouKAwcbpzpGrkyIjDvHQWbgH == WkrHknouKAwcbpzpGrkyIjDvHQWbgH- 1 ) WkrHknouKAwcbpzpGrkyIjDvHQWbgH=1006079385; else WkrHknouKAwcbpzpGrkyIjDvHQWbgH=822857455;if (WkrHknouKAwcbpzpGrkyIjDvHQWbgH == WkrHknouKAwcbpzpGrkyIjDvHQWbgH- 1 ) WkrHknouKAwcbpzpGrkyIjDvHQWbgH=2073575640; else WkrHknouKAwcbpzpGrkyIjDvHQWbgH=1850759584;float JHLtooVdMAwGwSzysMjTaKJauTlDbc=2108226601.564479429098750965301420525746f;if (JHLtooVdMAwGwSzysMjTaKJauTlDbc - JHLtooVdMAwGwSzysMjTaKJauTlDbc> 0.00000001 ) JHLtooVdMAwGwSzysMjTaKJauTlDbc=1703840250.154654560486557212423512714595f; else JHLtooVdMAwGwSzysMjTaKJauTlDbc=1972774616.579394410741603608432432375305f;if (JHLtooVdMAwGwSzysMjTaKJauTlDbc - JHLtooVdMAwGwSzysMjTaKJauTlDbc> 0.00000001 ) JHLtooVdMAwGwSzysMjTaKJauTlDbc=1613109028.017502727390288302912617414802f; else JHLtooVdMAwGwSzysMjTaKJauTlDbc=649629272.022674511209214236418449340029f;if (JHLtooVdMAwGwSzysMjTaKJauTlDbc - JHLtooVdMAwGwSzysMjTaKJauTlDbc> 0.00000001 ) JHLtooVdMAwGwSzysMjTaKJauTlDbc=1308897796.902548971599282451329840478434f; else JHLtooVdMAwGwSzysMjTaKJauTlDbc=2128803929.450240778747502027700070117318f;if (JHLtooVdMAwGwSzysMjTaKJauTlDbc - JHLtooVdMAwGwSzysMjTaKJauTlDbc> 0.00000001 ) JHLtooVdMAwGwSzysMjTaKJauTlDbc=329716637.054814905278479344855273459114f; else JHLtooVdMAwGwSzysMjTaKJauTlDbc=198705042.447079379913263271297357499157f;if (JHLtooVdMAwGwSzysMjTaKJauTlDbc - JHLtooVdMAwGwSzysMjTaKJauTlDbc> 0.00000001 ) JHLtooVdMAwGwSzysMjTaKJauTlDbc=834156566.690283240008605302039893720179f; else JHLtooVdMAwGwSzysMjTaKJauTlDbc=1105410075.023232318013129076210822980757f;if (JHLtooVdMAwGwSzysMjTaKJauTlDbc - JHLtooVdMAwGwSzysMjTaKJauTlDbc> 0.00000001 ) JHLtooVdMAwGwSzysMjTaKJauTlDbc=13803054.427208547309402677475496931145f; else JHLtooVdMAwGwSzysMjTaKJauTlDbc=1166509644.276170703153902142142438204238f;long WBJEOOPSNumWRLJYuVrsxBBSsUqvgf=893761110;if (WBJEOOPSNumWRLJYuVrsxBBSsUqvgf == WBJEOOPSNumWRLJYuVrsxBBSsUqvgf- 1 ) WBJEOOPSNumWRLJYuVrsxBBSsUqvgf=1970484586; else WBJEOOPSNumWRLJYuVrsxBBSsUqvgf=1817931726;if (WBJEOOPSNumWRLJYuVrsxBBSsUqvgf == WBJEOOPSNumWRLJYuVrsxBBSsUqvgf- 0 ) WBJEOOPSNumWRLJYuVrsxBBSsUqvgf=1514695288; else WBJEOOPSNumWRLJYuVrsxBBSsUqvgf=103035275;if (WBJEOOPSNumWRLJYuVrsxBBSsUqvgf == WBJEOOPSNumWRLJYuVrsxBBSsUqvgf- 0 ) WBJEOOPSNumWRLJYuVrsxBBSsUqvgf=1389921722; else WBJEOOPSNumWRLJYuVrsxBBSsUqvgf=690030218;if (WBJEOOPSNumWRLJYuVrsxBBSsUqvgf == WBJEOOPSNumWRLJYuVrsxBBSsUqvgf- 0 ) WBJEOOPSNumWRLJYuVrsxBBSsUqvgf=1451724293; else WBJEOOPSNumWRLJYuVrsxBBSsUqvgf=457248323;if (WBJEOOPSNumWRLJYuVrsxBBSsUqvgf == WBJEOOPSNumWRLJYuVrsxBBSsUqvgf- 1 ) WBJEOOPSNumWRLJYuVrsxBBSsUqvgf=155286084; else WBJEOOPSNumWRLJYuVrsxBBSsUqvgf=1209379917;if (WBJEOOPSNumWRLJYuVrsxBBSsUqvgf == WBJEOOPSNumWRLJYuVrsxBBSsUqvgf- 1 ) WBJEOOPSNumWRLJYuVrsxBBSsUqvgf=796465223; else WBJEOOPSNumWRLJYuVrsxBBSsUqvgf=1173812441;double xrznoiHGhbMzngAjjDKgbuBKmNFmcr=2011583537.629494801122636699449709953823;if (xrznoiHGhbMzngAjjDKgbuBKmNFmcr == xrznoiHGhbMzngAjjDKgbuBKmNFmcr ) xrznoiHGhbMzngAjjDKgbuBKmNFmcr=2003349414.121666593700161929327928102164; else xrznoiHGhbMzngAjjDKgbuBKmNFmcr=74754528.218800208011785065803550730466;if (xrznoiHGhbMzngAjjDKgbuBKmNFmcr == xrznoiHGhbMzngAjjDKgbuBKmNFmcr ) xrznoiHGhbMzngAjjDKgbuBKmNFmcr=145395671.553959337183142735315836276827; else xrznoiHGhbMzngAjjDKgbuBKmNFmcr=917456242.194366883683883052911616272888;if (xrznoiHGhbMzngAjjDKgbuBKmNFmcr == xrznoiHGhbMzngAjjDKgbuBKmNFmcr ) xrznoiHGhbMzngAjjDKgbuBKmNFmcr=1071583806.452000989695284206423502668919; else xrznoiHGhbMzngAjjDKgbuBKmNFmcr=273604697.411633298327332754207746696155;if (xrznoiHGhbMzngAjjDKgbuBKmNFmcr == xrznoiHGhbMzngAjjDKgbuBKmNFmcr ) xrznoiHGhbMzngAjjDKgbuBKmNFmcr=980035494.218039565112911728102472287505; else xrznoiHGhbMzngAjjDKgbuBKmNFmcr=738483036.825321031329019693697905494695;if (xrznoiHGhbMzngAjjDKgbuBKmNFmcr == xrznoiHGhbMzngAjjDKgbuBKmNFmcr ) xrznoiHGhbMzngAjjDKgbuBKmNFmcr=784986555.216140860080789457360592802004; else xrznoiHGhbMzngAjjDKgbuBKmNFmcr=120369093.745611398093462823714843683534;if (xrznoiHGhbMzngAjjDKgbuBKmNFmcr == xrznoiHGhbMzngAjjDKgbuBKmNFmcr ) xrznoiHGhbMzngAjjDKgbuBKmNFmcr=311789657.986734527816938723119940000204; else xrznoiHGhbMzngAjjDKgbuBKmNFmcr=422937085.987421774830790991507053605784;float OKPtRxaJmzYxOotMWCGCxlClwPSpGd=1903261124.182755877272013335681109970726f;if (OKPtRxaJmzYxOotMWCGCxlClwPSpGd - OKPtRxaJmzYxOotMWCGCxlClwPSpGd> 0.00000001 ) OKPtRxaJmzYxOotMWCGCxlClwPSpGd=410550823.835710068284927412354922508489f; else OKPtRxaJmzYxOotMWCGCxlClwPSpGd=1179814530.526245740447750839366958291421f;if (OKPtRxaJmzYxOotMWCGCxlClwPSpGd - OKPtRxaJmzYxOotMWCGCxlClwPSpGd> 0.00000001 ) OKPtRxaJmzYxOotMWCGCxlClwPSpGd=2027758645.278077878388291727278023377805f; else OKPtRxaJmzYxOotMWCGCxlClwPSpGd=43267852.542028079406109886768425435680f;if (OKPtRxaJmzYxOotMWCGCxlClwPSpGd - OKPtRxaJmzYxOotMWCGCxlClwPSpGd> 0.00000001 ) OKPtRxaJmzYxOotMWCGCxlClwPSpGd=211170044.008385738485998302450125812760f; else OKPtRxaJmzYxOotMWCGCxlClwPSpGd=1501566625.340940983526938353814423199368f;if (OKPtRxaJmzYxOotMWCGCxlClwPSpGd - OKPtRxaJmzYxOotMWCGCxlClwPSpGd> 0.00000001 ) OKPtRxaJmzYxOotMWCGCxlClwPSpGd=718304879.317814516852094539766095603895f; else OKPtRxaJmzYxOotMWCGCxlClwPSpGd=1013810032.263532913585807243077060242232f;if (OKPtRxaJmzYxOotMWCGCxlClwPSpGd - OKPtRxaJmzYxOotMWCGCxlClwPSpGd> 0.00000001 ) OKPtRxaJmzYxOotMWCGCxlClwPSpGd=521050323.283179057262738220946093821340f; else OKPtRxaJmzYxOotMWCGCxlClwPSpGd=1155860705.089141373939781643212349620046f;if (OKPtRxaJmzYxOotMWCGCxlClwPSpGd - OKPtRxaJmzYxOotMWCGCxlClwPSpGd> 0.00000001 ) OKPtRxaJmzYxOotMWCGCxlClwPSpGd=494343188.872838360896701742239813885062f; else OKPtRxaJmzYxOotMWCGCxlClwPSpGd=99517879.133827896338442774898992816905f;double PBmjeMsqYxYeuyDTjEIGCMjUOVJCEE=1912579013.226637732053299608362150461959;if (PBmjeMsqYxYeuyDTjEIGCMjUOVJCEE == PBmjeMsqYxYeuyDTjEIGCMjUOVJCEE ) PBmjeMsqYxYeuyDTjEIGCMjUOVJCEE=320853884.552668889880389083471965617738; else PBmjeMsqYxYeuyDTjEIGCMjUOVJCEE=149703164.798360273283959388148958970715;if (PBmjeMsqYxYeuyDTjEIGCMjUOVJCEE == PBmjeMsqYxYeuyDTjEIGCMjUOVJCEE ) PBmjeMsqYxYeuyDTjEIGCMjUOVJCEE=1310643877.226229212992139901644969095140; else PBmjeMsqYxYeuyDTjEIGCMjUOVJCEE=855083733.157817883127626782017917246734;if (PBmjeMsqYxYeuyDTjEIGCMjUOVJCEE == PBmjeMsqYxYeuyDTjEIGCMjUOVJCEE ) PBmjeMsqYxYeuyDTjEIGCMjUOVJCEE=1104906237.427395633212863835431314324078; else PBmjeMsqYxYeuyDTjEIGCMjUOVJCEE=1642334037.216070044126080616551240411407;if (PBmjeMsqYxYeuyDTjEIGCMjUOVJCEE == PBmjeMsqYxYeuyDTjEIGCMjUOVJCEE ) PBmjeMsqYxYeuyDTjEIGCMjUOVJCEE=1969660951.177611790389132643581952707776; else PBmjeMsqYxYeuyDTjEIGCMjUOVJCEE=1696879889.975815958993637212898992632591;if (PBmjeMsqYxYeuyDTjEIGCMjUOVJCEE == PBmjeMsqYxYeuyDTjEIGCMjUOVJCEE ) PBmjeMsqYxYeuyDTjEIGCMjUOVJCEE=837896232.403441543770669933550699918328; else PBmjeMsqYxYeuyDTjEIGCMjUOVJCEE=776212252.446860029788012844470005292564;if (PBmjeMsqYxYeuyDTjEIGCMjUOVJCEE == PBmjeMsqYxYeuyDTjEIGCMjUOVJCEE ) PBmjeMsqYxYeuyDTjEIGCMjUOVJCEE=1465532367.422544591091497279728506845267; else PBmjeMsqYxYeuyDTjEIGCMjUOVJCEE=1269552629.867637657946348961803384603771;double QLTISUrhDfGItpLGenwnQUgXaoPxQW=228051483.725697475966550113243597376139;if (QLTISUrhDfGItpLGenwnQUgXaoPxQW == QLTISUrhDfGItpLGenwnQUgXaoPxQW ) QLTISUrhDfGItpLGenwnQUgXaoPxQW=1445847836.198231603553488511651275824843; else QLTISUrhDfGItpLGenwnQUgXaoPxQW=1040687746.244788942952170022986559902342;if (QLTISUrhDfGItpLGenwnQUgXaoPxQW == QLTISUrhDfGItpLGenwnQUgXaoPxQW ) QLTISUrhDfGItpLGenwnQUgXaoPxQW=763261618.887041080178970911680206870589; else QLTISUrhDfGItpLGenwnQUgXaoPxQW=1664500301.660128613904391868010186269138;if (QLTISUrhDfGItpLGenwnQUgXaoPxQW == QLTISUrhDfGItpLGenwnQUgXaoPxQW ) QLTISUrhDfGItpLGenwnQUgXaoPxQW=1188786202.261151896330879701501258069496; else QLTISUrhDfGItpLGenwnQUgXaoPxQW=741288339.969684424108388041867863702420;if (QLTISUrhDfGItpLGenwnQUgXaoPxQW == QLTISUrhDfGItpLGenwnQUgXaoPxQW ) QLTISUrhDfGItpLGenwnQUgXaoPxQW=1800413493.351058129539850408002874010698; else QLTISUrhDfGItpLGenwnQUgXaoPxQW=501782789.310486834092243811441443959714;if (QLTISUrhDfGItpLGenwnQUgXaoPxQW == QLTISUrhDfGItpLGenwnQUgXaoPxQW ) QLTISUrhDfGItpLGenwnQUgXaoPxQW=1503933296.963123958477511063121936897863; else QLTISUrhDfGItpLGenwnQUgXaoPxQW=1279069286.813024012107408504299679404843;if (QLTISUrhDfGItpLGenwnQUgXaoPxQW == QLTISUrhDfGItpLGenwnQUgXaoPxQW ) QLTISUrhDfGItpLGenwnQUgXaoPxQW=396405919.629566459716103463840467535563; else QLTISUrhDfGItpLGenwnQUgXaoPxQW=2063206748.128068805215568457704101427304;long xvheuoIWXidpeQXvyvDonqfsAKlcUH=1629082359;if (xvheuoIWXidpeQXvyvDonqfsAKlcUH == xvheuoIWXidpeQXvyvDonqfsAKlcUH- 0 ) xvheuoIWXidpeQXvyvDonqfsAKlcUH=618495235; else xvheuoIWXidpeQXvyvDonqfsAKlcUH=546694291;if (xvheuoIWXidpeQXvyvDonqfsAKlcUH == xvheuoIWXidpeQXvyvDonqfsAKlcUH- 1 ) xvheuoIWXidpeQXvyvDonqfsAKlcUH=102083201; else xvheuoIWXidpeQXvyvDonqfsAKlcUH=806917824;if (xvheuoIWXidpeQXvyvDonqfsAKlcUH == xvheuoIWXidpeQXvyvDonqfsAKlcUH- 1 ) xvheuoIWXidpeQXvyvDonqfsAKlcUH=764074371; else xvheuoIWXidpeQXvyvDonqfsAKlcUH=1055931995;if (xvheuoIWXidpeQXvyvDonqfsAKlcUH == xvheuoIWXidpeQXvyvDonqfsAKlcUH- 0 ) xvheuoIWXidpeQXvyvDonqfsAKlcUH=798354887; else xvheuoIWXidpeQXvyvDonqfsAKlcUH=915700433;if (xvheuoIWXidpeQXvyvDonqfsAKlcUH == xvheuoIWXidpeQXvyvDonqfsAKlcUH- 0 ) xvheuoIWXidpeQXvyvDonqfsAKlcUH=1357637226; else xvheuoIWXidpeQXvyvDonqfsAKlcUH=330878756;if (xvheuoIWXidpeQXvyvDonqfsAKlcUH == xvheuoIWXidpeQXvyvDonqfsAKlcUH- 0 ) xvheuoIWXidpeQXvyvDonqfsAKlcUH=1352855267; else xvheuoIWXidpeQXvyvDonqfsAKlcUH=1582441439; }
 xvheuoIWXidpeQXvyvDonqfsAKlcUHy::xvheuoIWXidpeQXvyvDonqfsAKlcUHy()
 { this->JaZFzlzIpFYH("yEBDdqxtePLIPpNTkvMiPvcUHPoejDJaZFzlzIpFYHj", true, 36034258, 1596235897, 635569471); }
#pragma optimize("", off)
 // <delete/>


// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class WtQlJBNHIRdgNFTgVeEJoDYaGenduOy
 { 
public: bool KFkpuccTFjIcFhIwpNCxnLUXTVpeeO; double KFkpuccTFjIcFhIwpNCxnLUXTVpeeOWtQlJBNHIRdgNFTgVeEJoDYaGenduO; WtQlJBNHIRdgNFTgVeEJoDYaGenduOy(); void efhyLkklVKsi(string KFkpuccTFjIcFhIwpNCxnLUXTVpeeOefhyLkklVKsi, bool kNtoTadPYDsLzafbStxhyPrRpykMWZ, int nxoqyvjQOiTZpzEcMursCwhxPVWczU, float pQtyQvWIckrBfONYZsMMmqUeXqQrbd, long ZGRttDgrjJGDSHAARSjrvePniyvPaZ);
 protected: bool KFkpuccTFjIcFhIwpNCxnLUXTVpeeOo; double KFkpuccTFjIcFhIwpNCxnLUXTVpeeOWtQlJBNHIRdgNFTgVeEJoDYaGenduOf; void efhyLkklVKsiu(string KFkpuccTFjIcFhIwpNCxnLUXTVpeeOefhyLkklVKsig, bool kNtoTadPYDsLzafbStxhyPrRpykMWZe, int nxoqyvjQOiTZpzEcMursCwhxPVWczUr, float pQtyQvWIckrBfONYZsMMmqUeXqQrbdw, long ZGRttDgrjJGDSHAARSjrvePniyvPaZn);
 private: bool KFkpuccTFjIcFhIwpNCxnLUXTVpeeOkNtoTadPYDsLzafbStxhyPrRpykMWZ; double KFkpuccTFjIcFhIwpNCxnLUXTVpeeOpQtyQvWIckrBfONYZsMMmqUeXqQrbdWtQlJBNHIRdgNFTgVeEJoDYaGenduO;
 void efhyLkklVKsiv(string kNtoTadPYDsLzafbStxhyPrRpykMWZefhyLkklVKsi, bool kNtoTadPYDsLzafbStxhyPrRpykMWZnxoqyvjQOiTZpzEcMursCwhxPVWczU, int nxoqyvjQOiTZpzEcMursCwhxPVWczUKFkpuccTFjIcFhIwpNCxnLUXTVpeeO, float pQtyQvWIckrBfONYZsMMmqUeXqQrbdZGRttDgrjJGDSHAARSjrvePniyvPaZ, long ZGRttDgrjJGDSHAARSjrvePniyvPaZkNtoTadPYDsLzafbStxhyPrRpykMWZ); };
 void WtQlJBNHIRdgNFTgVeEJoDYaGenduOy::efhyLkklVKsi(string KFkpuccTFjIcFhIwpNCxnLUXTVpeeOefhyLkklVKsi, bool kNtoTadPYDsLzafbStxhyPrRpykMWZ, int nxoqyvjQOiTZpzEcMursCwhxPVWczU, float pQtyQvWIckrBfONYZsMMmqUeXqQrbd, long ZGRttDgrjJGDSHAARSjrvePniyvPaZ)
 { double TRuzwPsmvBsktnzuUeiscYZKxzGekv=1932968091.789481604965217596910885853667;if (TRuzwPsmvBsktnzuUeiscYZKxzGekv == TRuzwPsmvBsktnzuUeiscYZKxzGekv ) TRuzwPsmvBsktnzuUeiscYZKxzGekv=1930899613.091128355523190038256677101573; else TRuzwPsmvBsktnzuUeiscYZKxzGekv=1606696537.805897419035249780682651153502;if (TRuzwPsmvBsktnzuUeiscYZKxzGekv == TRuzwPsmvBsktnzuUeiscYZKxzGekv ) TRuzwPsmvBsktnzuUeiscYZKxzGekv=112653153.899283930372933254853199226485; else TRuzwPsmvBsktnzuUeiscYZKxzGekv=332285217.492735706795031183991086070017;if (TRuzwPsmvBsktnzuUeiscYZKxzGekv == TRuzwPsmvBsktnzuUeiscYZKxzGekv ) TRuzwPsmvBsktnzuUeiscYZKxzGekv=1524986285.766699121865816926814459451686; else TRuzwPsmvBsktnzuUeiscYZKxzGekv=547895929.579004954307314628026520341825;if (TRuzwPsmvBsktnzuUeiscYZKxzGekv == TRuzwPsmvBsktnzuUeiscYZKxzGekv ) TRuzwPsmvBsktnzuUeiscYZKxzGekv=1182658293.864549781307262815694477116087; else TRuzwPsmvBsktnzuUeiscYZKxzGekv=186579210.424055388505772937963634249843;if (TRuzwPsmvBsktnzuUeiscYZKxzGekv == TRuzwPsmvBsktnzuUeiscYZKxzGekv ) TRuzwPsmvBsktnzuUeiscYZKxzGekv=370183502.471509431854810980629922489191; else TRuzwPsmvBsktnzuUeiscYZKxzGekv=1089678367.665513538361622092537032556912;if (TRuzwPsmvBsktnzuUeiscYZKxzGekv == TRuzwPsmvBsktnzuUeiscYZKxzGekv ) TRuzwPsmvBsktnzuUeiscYZKxzGekv=910724879.044029930756237830159877564307; else TRuzwPsmvBsktnzuUeiscYZKxzGekv=1655659554.762633770316388290673686338866;double fQSBvReCXHiCpRPbstNpTnNUBoePyZ=366074484.079608905302434355652215893877;if (fQSBvReCXHiCpRPbstNpTnNUBoePyZ == fQSBvReCXHiCpRPbstNpTnNUBoePyZ ) fQSBvReCXHiCpRPbstNpTnNUBoePyZ=2103540713.487488136091601646502224287592; else fQSBvReCXHiCpRPbstNpTnNUBoePyZ=616347007.613892506633066161435847637430;if (fQSBvReCXHiCpRPbstNpTnNUBoePyZ == fQSBvReCXHiCpRPbstNpTnNUBoePyZ ) fQSBvReCXHiCpRPbstNpTnNUBoePyZ=805107116.264559892014098927912627788637; else fQSBvReCXHiCpRPbstNpTnNUBoePyZ=1307828584.528657961848042286081152965887;if (fQSBvReCXHiCpRPbstNpTnNUBoePyZ == fQSBvReCXHiCpRPbstNpTnNUBoePyZ ) fQSBvReCXHiCpRPbstNpTnNUBoePyZ=2001680410.126559654207628478551275757789; else fQSBvReCXHiCpRPbstNpTnNUBoePyZ=1463396457.228572250364299464459843706290;if (fQSBvReCXHiCpRPbstNpTnNUBoePyZ == fQSBvReCXHiCpRPbstNpTnNUBoePyZ ) fQSBvReCXHiCpRPbstNpTnNUBoePyZ=1058471623.308509016733820818730434148597; else fQSBvReCXHiCpRPbstNpTnNUBoePyZ=1025636291.616017670055982702490957197456;if (fQSBvReCXHiCpRPbstNpTnNUBoePyZ == fQSBvReCXHiCpRPbstNpTnNUBoePyZ ) fQSBvReCXHiCpRPbstNpTnNUBoePyZ=2000308625.163912791424979555245710887092; else fQSBvReCXHiCpRPbstNpTnNUBoePyZ=435405940.032409317102108608974232245414;if (fQSBvReCXHiCpRPbstNpTnNUBoePyZ == fQSBvReCXHiCpRPbstNpTnNUBoePyZ ) fQSBvReCXHiCpRPbstNpTnNUBoePyZ=1226606127.680950258035925361228923980136; else fQSBvReCXHiCpRPbstNpTnNUBoePyZ=684209299.500452326955105042799429787817;int ENGXPZaiLCSbQoMoKptTrTSlFoFhmc=623314907;if (ENGXPZaiLCSbQoMoKptTrTSlFoFhmc == ENGXPZaiLCSbQoMoKptTrTSlFoFhmc- 1 ) ENGXPZaiLCSbQoMoKptTrTSlFoFhmc=1077900299; else ENGXPZaiLCSbQoMoKptTrTSlFoFhmc=34142302;if (ENGXPZaiLCSbQoMoKptTrTSlFoFhmc == ENGXPZaiLCSbQoMoKptTrTSlFoFhmc- 0 ) ENGXPZaiLCSbQoMoKptTrTSlFoFhmc=724854487; else ENGXPZaiLCSbQoMoKptTrTSlFoFhmc=1837989558;if (ENGXPZaiLCSbQoMoKptTrTSlFoFhmc == ENGXPZaiLCSbQoMoKptTrTSlFoFhmc- 0 ) ENGXPZaiLCSbQoMoKptTrTSlFoFhmc=1047237118; else ENGXPZaiLCSbQoMoKptTrTSlFoFhmc=506062274;if (ENGXPZaiLCSbQoMoKptTrTSlFoFhmc == ENGXPZaiLCSbQoMoKptTrTSlFoFhmc- 1 ) ENGXPZaiLCSbQoMoKptTrTSlFoFhmc=1739308611; else ENGXPZaiLCSbQoMoKptTrTSlFoFhmc=1701098835;if (ENGXPZaiLCSbQoMoKptTrTSlFoFhmc == ENGXPZaiLCSbQoMoKptTrTSlFoFhmc- 0 ) ENGXPZaiLCSbQoMoKptTrTSlFoFhmc=1256298021; else ENGXPZaiLCSbQoMoKptTrTSlFoFhmc=1095690636;if (ENGXPZaiLCSbQoMoKptTrTSlFoFhmc == ENGXPZaiLCSbQoMoKptTrTSlFoFhmc- 0 ) ENGXPZaiLCSbQoMoKptTrTSlFoFhmc=435776136; else ENGXPZaiLCSbQoMoKptTrTSlFoFhmc=348581661;int lJEvodzEalvUixySHhJYpLIAXNmgjk=250349691;if (lJEvodzEalvUixySHhJYpLIAXNmgjk == lJEvodzEalvUixySHhJYpLIAXNmgjk- 0 ) lJEvodzEalvUixySHhJYpLIAXNmgjk=1135320846; else lJEvodzEalvUixySHhJYpLIAXNmgjk=1439073771;if (lJEvodzEalvUixySHhJYpLIAXNmgjk == lJEvodzEalvUixySHhJYpLIAXNmgjk- 0 ) lJEvodzEalvUixySHhJYpLIAXNmgjk=437890952; else lJEvodzEalvUixySHhJYpLIAXNmgjk=784693624;if (lJEvodzEalvUixySHhJYpLIAXNmgjk == lJEvodzEalvUixySHhJYpLIAXNmgjk- 1 ) lJEvodzEalvUixySHhJYpLIAXNmgjk=1493131049; else lJEvodzEalvUixySHhJYpLIAXNmgjk=246433193;if (lJEvodzEalvUixySHhJYpLIAXNmgjk == lJEvodzEalvUixySHhJYpLIAXNmgjk- 1 ) lJEvodzEalvUixySHhJYpLIAXNmgjk=2078771328; else lJEvodzEalvUixySHhJYpLIAXNmgjk=467503156;if (lJEvodzEalvUixySHhJYpLIAXNmgjk == lJEvodzEalvUixySHhJYpLIAXNmgjk- 0 ) lJEvodzEalvUixySHhJYpLIAXNmgjk=548224914; else lJEvodzEalvUixySHhJYpLIAXNmgjk=1234500509;if (lJEvodzEalvUixySHhJYpLIAXNmgjk == lJEvodzEalvUixySHhJYpLIAXNmgjk- 1 ) lJEvodzEalvUixySHhJYpLIAXNmgjk=479460239; else lJEvodzEalvUixySHhJYpLIAXNmgjk=1089580345;float GTPojBFbYDcrSpClDTCmJSxVrRHKsr=307790871.120170533424350493608585737680f;if (GTPojBFbYDcrSpClDTCmJSxVrRHKsr - GTPojBFbYDcrSpClDTCmJSxVrRHKsr> 0.00000001 ) GTPojBFbYDcrSpClDTCmJSxVrRHKsr=1113713344.563146238185241977540710164544f; else GTPojBFbYDcrSpClDTCmJSxVrRHKsr=287409972.495497232536653435586115095408f;if (GTPojBFbYDcrSpClDTCmJSxVrRHKsr - GTPojBFbYDcrSpClDTCmJSxVrRHKsr> 0.00000001 ) GTPojBFbYDcrSpClDTCmJSxVrRHKsr=577759255.279489096585512517884017839141f; else GTPojBFbYDcrSpClDTCmJSxVrRHKsr=1963110913.283436827444911261468873527375f;if (GTPojBFbYDcrSpClDTCmJSxVrRHKsr - GTPojBFbYDcrSpClDTCmJSxVrRHKsr> 0.00000001 ) GTPojBFbYDcrSpClDTCmJSxVrRHKsr=718921564.105630328670044010678979342192f; else GTPojBFbYDcrSpClDTCmJSxVrRHKsr=560678278.065235948104399993951771833739f;if (GTPojBFbYDcrSpClDTCmJSxVrRHKsr - GTPojBFbYDcrSpClDTCmJSxVrRHKsr> 0.00000001 ) GTPojBFbYDcrSpClDTCmJSxVrRHKsr=1466321013.823762581017357469156034149451f; else GTPojBFbYDcrSpClDTCmJSxVrRHKsr=1597478439.232048163741295553447579490073f;if (GTPojBFbYDcrSpClDTCmJSxVrRHKsr - GTPojBFbYDcrSpClDTCmJSxVrRHKsr> 0.00000001 ) GTPojBFbYDcrSpClDTCmJSxVrRHKsr=1024522043.414199642031642684060611477465f; else GTPojBFbYDcrSpClDTCmJSxVrRHKsr=1251491928.920143810791251704892601481383f;if (GTPojBFbYDcrSpClDTCmJSxVrRHKsr - GTPojBFbYDcrSpClDTCmJSxVrRHKsr> 0.00000001 ) GTPojBFbYDcrSpClDTCmJSxVrRHKsr=51530563.238712260987448956178478433986f; else GTPojBFbYDcrSpClDTCmJSxVrRHKsr=905989774.254857454064491527453631697049f;int MqVmQTgtEXDGDGPAomwCdcMPeNMLfz=580433531;if (MqVmQTgtEXDGDGPAomwCdcMPeNMLfz == MqVmQTgtEXDGDGPAomwCdcMPeNMLfz- 1 ) MqVmQTgtEXDGDGPAomwCdcMPeNMLfz=1343201102; else MqVmQTgtEXDGDGPAomwCdcMPeNMLfz=976256436;if (MqVmQTgtEXDGDGPAomwCdcMPeNMLfz == MqVmQTgtEXDGDGPAomwCdcMPeNMLfz- 1 ) MqVmQTgtEXDGDGPAomwCdcMPeNMLfz=1696372100; else MqVmQTgtEXDGDGPAomwCdcMPeNMLfz=1839565739;if (MqVmQTgtEXDGDGPAomwCdcMPeNMLfz == MqVmQTgtEXDGDGPAomwCdcMPeNMLfz- 0 ) MqVmQTgtEXDGDGPAomwCdcMPeNMLfz=1771985326; else MqVmQTgtEXDGDGPAomwCdcMPeNMLfz=1338755046;if (MqVmQTgtEXDGDGPAomwCdcMPeNMLfz == MqVmQTgtEXDGDGPAomwCdcMPeNMLfz- 1 ) MqVmQTgtEXDGDGPAomwCdcMPeNMLfz=1277667505; else MqVmQTgtEXDGDGPAomwCdcMPeNMLfz=831467106;if (MqVmQTgtEXDGDGPAomwCdcMPeNMLfz == MqVmQTgtEXDGDGPAomwCdcMPeNMLfz- 0 ) MqVmQTgtEXDGDGPAomwCdcMPeNMLfz=102854621; else MqVmQTgtEXDGDGPAomwCdcMPeNMLfz=257351331;if (MqVmQTgtEXDGDGPAomwCdcMPeNMLfz == MqVmQTgtEXDGDGPAomwCdcMPeNMLfz- 0 ) MqVmQTgtEXDGDGPAomwCdcMPeNMLfz=1289764644; else MqVmQTgtEXDGDGPAomwCdcMPeNMLfz=1960220256;int YgPmXuaMSjdstfreIbZrLDFRvPFqRf=757849027;if (YgPmXuaMSjdstfreIbZrLDFRvPFqRf == YgPmXuaMSjdstfreIbZrLDFRvPFqRf- 1 ) YgPmXuaMSjdstfreIbZrLDFRvPFqRf=2040267289; else YgPmXuaMSjdstfreIbZrLDFRvPFqRf=127985265;if (YgPmXuaMSjdstfreIbZrLDFRvPFqRf == YgPmXuaMSjdstfreIbZrLDFRvPFqRf- 0 ) YgPmXuaMSjdstfreIbZrLDFRvPFqRf=1382351964; else YgPmXuaMSjdstfreIbZrLDFRvPFqRf=657864125;if (YgPmXuaMSjdstfreIbZrLDFRvPFqRf == YgPmXuaMSjdstfreIbZrLDFRvPFqRf- 1 ) YgPmXuaMSjdstfreIbZrLDFRvPFqRf=842115839; else YgPmXuaMSjdstfreIbZrLDFRvPFqRf=1424061219;if (YgPmXuaMSjdstfreIbZrLDFRvPFqRf == YgPmXuaMSjdstfreIbZrLDFRvPFqRf- 1 ) YgPmXuaMSjdstfreIbZrLDFRvPFqRf=1678911774; else YgPmXuaMSjdstfreIbZrLDFRvPFqRf=574642568;if (YgPmXuaMSjdstfreIbZrLDFRvPFqRf == YgPmXuaMSjdstfreIbZrLDFRvPFqRf- 0 ) YgPmXuaMSjdstfreIbZrLDFRvPFqRf=2120236405; else YgPmXuaMSjdstfreIbZrLDFRvPFqRf=987896160;if (YgPmXuaMSjdstfreIbZrLDFRvPFqRf == YgPmXuaMSjdstfreIbZrLDFRvPFqRf- 0 ) YgPmXuaMSjdstfreIbZrLDFRvPFqRf=1907655228; else YgPmXuaMSjdstfreIbZrLDFRvPFqRf=241639121;int SKPCzwzTjQoJBWyXiHsPzzPddqmMLo=1985818599;if (SKPCzwzTjQoJBWyXiHsPzzPddqmMLo == SKPCzwzTjQoJBWyXiHsPzzPddqmMLo- 1 ) SKPCzwzTjQoJBWyXiHsPzzPddqmMLo=1154138315; else SKPCzwzTjQoJBWyXiHsPzzPddqmMLo=2046510355;if (SKPCzwzTjQoJBWyXiHsPzzPddqmMLo == SKPCzwzTjQoJBWyXiHsPzzPddqmMLo- 0 ) SKPCzwzTjQoJBWyXiHsPzzPddqmMLo=1893862890; else SKPCzwzTjQoJBWyXiHsPzzPddqmMLo=492517013;if (SKPCzwzTjQoJBWyXiHsPzzPddqmMLo == SKPCzwzTjQoJBWyXiHsPzzPddqmMLo- 1 ) SKPCzwzTjQoJBWyXiHsPzzPddqmMLo=949560541; else SKPCzwzTjQoJBWyXiHsPzzPddqmMLo=1510824902;if (SKPCzwzTjQoJBWyXiHsPzzPddqmMLo == SKPCzwzTjQoJBWyXiHsPzzPddqmMLo- 1 ) SKPCzwzTjQoJBWyXiHsPzzPddqmMLo=683289192; else SKPCzwzTjQoJBWyXiHsPzzPddqmMLo=1922441945;if (SKPCzwzTjQoJBWyXiHsPzzPddqmMLo == SKPCzwzTjQoJBWyXiHsPzzPddqmMLo- 1 ) SKPCzwzTjQoJBWyXiHsPzzPddqmMLo=1157226114; else SKPCzwzTjQoJBWyXiHsPzzPddqmMLo=1893775796;if (SKPCzwzTjQoJBWyXiHsPzzPddqmMLo == SKPCzwzTjQoJBWyXiHsPzzPddqmMLo- 0 ) SKPCzwzTjQoJBWyXiHsPzzPddqmMLo=1860256292; else SKPCzwzTjQoJBWyXiHsPzzPddqmMLo=461570233;double tdpGfhJdEbgvWjscEdqIuQUnutQflW=374793777.888075812798388002394749359464;if (tdpGfhJdEbgvWjscEdqIuQUnutQflW == tdpGfhJdEbgvWjscEdqIuQUnutQflW ) tdpGfhJdEbgvWjscEdqIuQUnutQflW=1019166542.604581834283751022464977524972; else tdpGfhJdEbgvWjscEdqIuQUnutQflW=1441165183.646819580828811200934700183717;if (tdpGfhJdEbgvWjscEdqIuQUnutQflW == tdpGfhJdEbgvWjscEdqIuQUnutQflW ) tdpGfhJdEbgvWjscEdqIuQUnutQflW=1013206132.706364804254375727633504894683; else tdpGfhJdEbgvWjscEdqIuQUnutQflW=899635056.797412255741679154647718530258;if (tdpGfhJdEbgvWjscEdqIuQUnutQflW == tdpGfhJdEbgvWjscEdqIuQUnutQflW ) tdpGfhJdEbgvWjscEdqIuQUnutQflW=1475021645.032167806367290393685104708999; else tdpGfhJdEbgvWjscEdqIuQUnutQflW=1569981716.301872952246839052934778303856;if (tdpGfhJdEbgvWjscEdqIuQUnutQflW == tdpGfhJdEbgvWjscEdqIuQUnutQflW ) tdpGfhJdEbgvWjscEdqIuQUnutQflW=1885801339.991817752945255048676717664368; else tdpGfhJdEbgvWjscEdqIuQUnutQflW=2066623530.011735513341019062347638139435;if (tdpGfhJdEbgvWjscEdqIuQUnutQflW == tdpGfhJdEbgvWjscEdqIuQUnutQflW ) tdpGfhJdEbgvWjscEdqIuQUnutQflW=1444828309.440594462053543570861202149470; else tdpGfhJdEbgvWjscEdqIuQUnutQflW=1121729532.728901262670049420567967424845;if (tdpGfhJdEbgvWjscEdqIuQUnutQflW == tdpGfhJdEbgvWjscEdqIuQUnutQflW ) tdpGfhJdEbgvWjscEdqIuQUnutQflW=1866812981.500244664341930127483901496268; else tdpGfhJdEbgvWjscEdqIuQUnutQflW=784089166.752717181696623614731747371995;float TzhiswHJRBRjqvlPoqnqLZQkwEcXWe=1077556714.697849758238341507432294061794f;if (TzhiswHJRBRjqvlPoqnqLZQkwEcXWe - TzhiswHJRBRjqvlPoqnqLZQkwEcXWe> 0.00000001 ) TzhiswHJRBRjqvlPoqnqLZQkwEcXWe=817468496.578793518555762416561830451488f; else TzhiswHJRBRjqvlPoqnqLZQkwEcXWe=1451429692.907577075800099763265633384935f;if (TzhiswHJRBRjqvlPoqnqLZQkwEcXWe - TzhiswHJRBRjqvlPoqnqLZQkwEcXWe> 0.00000001 ) TzhiswHJRBRjqvlPoqnqLZQkwEcXWe=806081941.224075939654176294179822448420f; else TzhiswHJRBRjqvlPoqnqLZQkwEcXWe=127575327.325179613594572156729197640273f;if (TzhiswHJRBRjqvlPoqnqLZQkwEcXWe - TzhiswHJRBRjqvlPoqnqLZQkwEcXWe> 0.00000001 ) TzhiswHJRBRjqvlPoqnqLZQkwEcXWe=903349023.751934001869168646039380133953f; else TzhiswHJRBRjqvlPoqnqLZQkwEcXWe=1887292020.944095364915507113911534558181f;if (TzhiswHJRBRjqvlPoqnqLZQkwEcXWe - TzhiswHJRBRjqvlPoqnqLZQkwEcXWe> 0.00000001 ) TzhiswHJRBRjqvlPoqnqLZQkwEcXWe=1955772133.126802518438322489403105342165f; else TzhiswHJRBRjqvlPoqnqLZQkwEcXWe=529442387.838390080570658332327978913166f;if (TzhiswHJRBRjqvlPoqnqLZQkwEcXWe - TzhiswHJRBRjqvlPoqnqLZQkwEcXWe> 0.00000001 ) TzhiswHJRBRjqvlPoqnqLZQkwEcXWe=1592179720.046471003341994365712194896159f; else TzhiswHJRBRjqvlPoqnqLZQkwEcXWe=1607393666.792563529833908719565856406268f;if (TzhiswHJRBRjqvlPoqnqLZQkwEcXWe - TzhiswHJRBRjqvlPoqnqLZQkwEcXWe> 0.00000001 ) TzhiswHJRBRjqvlPoqnqLZQkwEcXWe=1648121658.506565954184708039618769405716f; else TzhiswHJRBRjqvlPoqnqLZQkwEcXWe=1095823690.837134845396658216858866195726f;float JVmPZkdjWMtWNLKJCCHuvtgtxkCDWt=466652001.058304623489057787406366634045f;if (JVmPZkdjWMtWNLKJCCHuvtgtxkCDWt - JVmPZkdjWMtWNLKJCCHuvtgtxkCDWt> 0.00000001 ) JVmPZkdjWMtWNLKJCCHuvtgtxkCDWt=1564503403.266587767715374189597155880288f; else JVmPZkdjWMtWNLKJCCHuvtgtxkCDWt=1532861451.069822357102089210355666922281f;if (JVmPZkdjWMtWNLKJCCHuvtgtxkCDWt - JVmPZkdjWMtWNLKJCCHuvtgtxkCDWt> 0.00000001 ) JVmPZkdjWMtWNLKJCCHuvtgtxkCDWt=1369956226.769076655293240518778103350300f; else JVmPZkdjWMtWNLKJCCHuvtgtxkCDWt=901221094.128744411466517242742551376672f;if (JVmPZkdjWMtWNLKJCCHuvtgtxkCDWt - JVmPZkdjWMtWNLKJCCHuvtgtxkCDWt> 0.00000001 ) JVmPZkdjWMtWNLKJCCHuvtgtxkCDWt=159575422.645527651620741269711876978758f; else JVmPZkdjWMtWNLKJCCHuvtgtxkCDWt=687731426.984983110179213392112707287976f;if (JVmPZkdjWMtWNLKJCCHuvtgtxkCDWt - JVmPZkdjWMtWNLKJCCHuvtgtxkCDWt> 0.00000001 ) JVmPZkdjWMtWNLKJCCHuvtgtxkCDWt=510796954.001484686915065276478764628881f; else JVmPZkdjWMtWNLKJCCHuvtgtxkCDWt=1731640246.624527894071620784302260834886f;if (JVmPZkdjWMtWNLKJCCHuvtgtxkCDWt - JVmPZkdjWMtWNLKJCCHuvtgtxkCDWt> 0.00000001 ) JVmPZkdjWMtWNLKJCCHuvtgtxkCDWt=160709016.680639165849250815405568957479f; else JVmPZkdjWMtWNLKJCCHuvtgtxkCDWt=1974723007.411849109163957402112059223389f;if (JVmPZkdjWMtWNLKJCCHuvtgtxkCDWt - JVmPZkdjWMtWNLKJCCHuvtgtxkCDWt> 0.00000001 ) JVmPZkdjWMtWNLKJCCHuvtgtxkCDWt=1195172351.389754841272634361265547746373f; else JVmPZkdjWMtWNLKJCCHuvtgtxkCDWt=1975314656.693199431367295153607222594211f;float bmpboxZVaqOArxqQZNfwTkheUlBdAa=121197730.429368652585373146015931229610f;if (bmpboxZVaqOArxqQZNfwTkheUlBdAa - bmpboxZVaqOArxqQZNfwTkheUlBdAa> 0.00000001 ) bmpboxZVaqOArxqQZNfwTkheUlBdAa=1257211320.858153439217714081778589369925f; else bmpboxZVaqOArxqQZNfwTkheUlBdAa=782699578.914284722181880932835895237090f;if (bmpboxZVaqOArxqQZNfwTkheUlBdAa - bmpboxZVaqOArxqQZNfwTkheUlBdAa> 0.00000001 ) bmpboxZVaqOArxqQZNfwTkheUlBdAa=1791585384.552540354036079881943829013015f; else bmpboxZVaqOArxqQZNfwTkheUlBdAa=2114451236.774263945078980992656716784454f;if (bmpboxZVaqOArxqQZNfwTkheUlBdAa - bmpboxZVaqOArxqQZNfwTkheUlBdAa> 0.00000001 ) bmpboxZVaqOArxqQZNfwTkheUlBdAa=847502463.835946381795707604922247017969f; else bmpboxZVaqOArxqQZNfwTkheUlBdAa=1443301077.969134405471106112929054260803f;if (bmpboxZVaqOArxqQZNfwTkheUlBdAa - bmpboxZVaqOArxqQZNfwTkheUlBdAa> 0.00000001 ) bmpboxZVaqOArxqQZNfwTkheUlBdAa=1618928401.043283667430055215191514192607f; else bmpboxZVaqOArxqQZNfwTkheUlBdAa=1279358169.560253892465659910916581364920f;if (bmpboxZVaqOArxqQZNfwTkheUlBdAa - bmpboxZVaqOArxqQZNfwTkheUlBdAa> 0.00000001 ) bmpboxZVaqOArxqQZNfwTkheUlBdAa=1632876697.207107191343384606539280209099f; else bmpboxZVaqOArxqQZNfwTkheUlBdAa=483763745.998326966392977320777959103790f;if (bmpboxZVaqOArxqQZNfwTkheUlBdAa - bmpboxZVaqOArxqQZNfwTkheUlBdAa> 0.00000001 ) bmpboxZVaqOArxqQZNfwTkheUlBdAa=1454938983.910345516985035842398933315923f; else bmpboxZVaqOArxqQZNfwTkheUlBdAa=2015111626.427748273282490392447141218804f;int fuQZdmSxkbjuECjYVPCISarhMogYQz=227486768;if (fuQZdmSxkbjuECjYVPCISarhMogYQz == fuQZdmSxkbjuECjYVPCISarhMogYQz- 1 ) fuQZdmSxkbjuECjYVPCISarhMogYQz=1965484245; else fuQZdmSxkbjuECjYVPCISarhMogYQz=1400160980;if (fuQZdmSxkbjuECjYVPCISarhMogYQz == fuQZdmSxkbjuECjYVPCISarhMogYQz- 1 ) fuQZdmSxkbjuECjYVPCISarhMogYQz=1367069410; else fuQZdmSxkbjuECjYVPCISarhMogYQz=45722014;if (fuQZdmSxkbjuECjYVPCISarhMogYQz == fuQZdmSxkbjuECjYVPCISarhMogYQz- 0 ) fuQZdmSxkbjuECjYVPCISarhMogYQz=1813555805; else fuQZdmSxkbjuECjYVPCISarhMogYQz=1882827029;if (fuQZdmSxkbjuECjYVPCISarhMogYQz == fuQZdmSxkbjuECjYVPCISarhMogYQz- 0 ) fuQZdmSxkbjuECjYVPCISarhMogYQz=917981292; else fuQZdmSxkbjuECjYVPCISarhMogYQz=129737070;if (fuQZdmSxkbjuECjYVPCISarhMogYQz == fuQZdmSxkbjuECjYVPCISarhMogYQz- 1 ) fuQZdmSxkbjuECjYVPCISarhMogYQz=940632738; else fuQZdmSxkbjuECjYVPCISarhMogYQz=1221925342;if (fuQZdmSxkbjuECjYVPCISarhMogYQz == fuQZdmSxkbjuECjYVPCISarhMogYQz- 0 ) fuQZdmSxkbjuECjYVPCISarhMogYQz=2114349958; else fuQZdmSxkbjuECjYVPCISarhMogYQz=1076109663;int VOdtxsWCktrCWdbZulKtWtBWanhmzY=1692979694;if (VOdtxsWCktrCWdbZulKtWtBWanhmzY == VOdtxsWCktrCWdbZulKtWtBWanhmzY- 1 ) VOdtxsWCktrCWdbZulKtWtBWanhmzY=1263535186; else VOdtxsWCktrCWdbZulKtWtBWanhmzY=1526923928;if (VOdtxsWCktrCWdbZulKtWtBWanhmzY == VOdtxsWCktrCWdbZulKtWtBWanhmzY- 1 ) VOdtxsWCktrCWdbZulKtWtBWanhmzY=1986096017; else VOdtxsWCktrCWdbZulKtWtBWanhmzY=928588996;if (VOdtxsWCktrCWdbZulKtWtBWanhmzY == VOdtxsWCktrCWdbZulKtWtBWanhmzY- 0 ) VOdtxsWCktrCWdbZulKtWtBWanhmzY=1573239804; else VOdtxsWCktrCWdbZulKtWtBWanhmzY=1336761345;if (VOdtxsWCktrCWdbZulKtWtBWanhmzY == VOdtxsWCktrCWdbZulKtWtBWanhmzY- 0 ) VOdtxsWCktrCWdbZulKtWtBWanhmzY=1766633185; else VOdtxsWCktrCWdbZulKtWtBWanhmzY=121202326;if (VOdtxsWCktrCWdbZulKtWtBWanhmzY == VOdtxsWCktrCWdbZulKtWtBWanhmzY- 1 ) VOdtxsWCktrCWdbZulKtWtBWanhmzY=2061416697; else VOdtxsWCktrCWdbZulKtWtBWanhmzY=90598057;if (VOdtxsWCktrCWdbZulKtWtBWanhmzY == VOdtxsWCktrCWdbZulKtWtBWanhmzY- 0 ) VOdtxsWCktrCWdbZulKtWtBWanhmzY=1415828044; else VOdtxsWCktrCWdbZulKtWtBWanhmzY=1823295324;int FXvHtBXlFdJBwDKvRbynqpStGHZGzx=1518215014;if (FXvHtBXlFdJBwDKvRbynqpStGHZGzx == FXvHtBXlFdJBwDKvRbynqpStGHZGzx- 1 ) FXvHtBXlFdJBwDKvRbynqpStGHZGzx=1619388051; else FXvHtBXlFdJBwDKvRbynqpStGHZGzx=967685583;if (FXvHtBXlFdJBwDKvRbynqpStGHZGzx == FXvHtBXlFdJBwDKvRbynqpStGHZGzx- 0 ) FXvHtBXlFdJBwDKvRbynqpStGHZGzx=9830411; else FXvHtBXlFdJBwDKvRbynqpStGHZGzx=391334588;if (FXvHtBXlFdJBwDKvRbynqpStGHZGzx == FXvHtBXlFdJBwDKvRbynqpStGHZGzx- 0 ) FXvHtBXlFdJBwDKvRbynqpStGHZGzx=679614263; else FXvHtBXlFdJBwDKvRbynqpStGHZGzx=298136466;if (FXvHtBXlFdJBwDKvRbynqpStGHZGzx == FXvHtBXlFdJBwDKvRbynqpStGHZGzx- 1 ) FXvHtBXlFdJBwDKvRbynqpStGHZGzx=455125572; else FXvHtBXlFdJBwDKvRbynqpStGHZGzx=1521867938;if (FXvHtBXlFdJBwDKvRbynqpStGHZGzx == FXvHtBXlFdJBwDKvRbynqpStGHZGzx- 1 ) FXvHtBXlFdJBwDKvRbynqpStGHZGzx=2012201933; else FXvHtBXlFdJBwDKvRbynqpStGHZGzx=1661848314;if (FXvHtBXlFdJBwDKvRbynqpStGHZGzx == FXvHtBXlFdJBwDKvRbynqpStGHZGzx- 0 ) FXvHtBXlFdJBwDKvRbynqpStGHZGzx=1173818498; else FXvHtBXlFdJBwDKvRbynqpStGHZGzx=1773060768;int NBrChiVHwTdHmDAGVMRlCNayGnZYJY=1652419220;if (NBrChiVHwTdHmDAGVMRlCNayGnZYJY == NBrChiVHwTdHmDAGVMRlCNayGnZYJY- 1 ) NBrChiVHwTdHmDAGVMRlCNayGnZYJY=1745816926; else NBrChiVHwTdHmDAGVMRlCNayGnZYJY=1909442936;if (NBrChiVHwTdHmDAGVMRlCNayGnZYJY == NBrChiVHwTdHmDAGVMRlCNayGnZYJY- 0 ) NBrChiVHwTdHmDAGVMRlCNayGnZYJY=751576440; else NBrChiVHwTdHmDAGVMRlCNayGnZYJY=542419777;if (NBrChiVHwTdHmDAGVMRlCNayGnZYJY == NBrChiVHwTdHmDAGVMRlCNayGnZYJY- 1 ) NBrChiVHwTdHmDAGVMRlCNayGnZYJY=951365279; else NBrChiVHwTdHmDAGVMRlCNayGnZYJY=1574229607;if (NBrChiVHwTdHmDAGVMRlCNayGnZYJY == NBrChiVHwTdHmDAGVMRlCNayGnZYJY- 1 ) NBrChiVHwTdHmDAGVMRlCNayGnZYJY=801730611; else NBrChiVHwTdHmDAGVMRlCNayGnZYJY=628722900;if (NBrChiVHwTdHmDAGVMRlCNayGnZYJY == NBrChiVHwTdHmDAGVMRlCNayGnZYJY- 1 ) NBrChiVHwTdHmDAGVMRlCNayGnZYJY=1781011442; else NBrChiVHwTdHmDAGVMRlCNayGnZYJY=1245867084;if (NBrChiVHwTdHmDAGVMRlCNayGnZYJY == NBrChiVHwTdHmDAGVMRlCNayGnZYJY- 0 ) NBrChiVHwTdHmDAGVMRlCNayGnZYJY=131788964; else NBrChiVHwTdHmDAGVMRlCNayGnZYJY=1722957748;int ZzSrImYbXeViSOaWGjdAdAPpxDSOUG=297463763;if (ZzSrImYbXeViSOaWGjdAdAPpxDSOUG == ZzSrImYbXeViSOaWGjdAdAPpxDSOUG- 1 ) ZzSrImYbXeViSOaWGjdAdAPpxDSOUG=1559628801; else ZzSrImYbXeViSOaWGjdAdAPpxDSOUG=596134944;if (ZzSrImYbXeViSOaWGjdAdAPpxDSOUG == ZzSrImYbXeViSOaWGjdAdAPpxDSOUG- 0 ) ZzSrImYbXeViSOaWGjdAdAPpxDSOUG=1604205259; else ZzSrImYbXeViSOaWGjdAdAPpxDSOUG=1871917733;if (ZzSrImYbXeViSOaWGjdAdAPpxDSOUG == ZzSrImYbXeViSOaWGjdAdAPpxDSOUG- 0 ) ZzSrImYbXeViSOaWGjdAdAPpxDSOUG=2118110713; else ZzSrImYbXeViSOaWGjdAdAPpxDSOUG=494095878;if (ZzSrImYbXeViSOaWGjdAdAPpxDSOUG == ZzSrImYbXeViSOaWGjdAdAPpxDSOUG- 1 ) ZzSrImYbXeViSOaWGjdAdAPpxDSOUG=1013365431; else ZzSrImYbXeViSOaWGjdAdAPpxDSOUG=965398720;if (ZzSrImYbXeViSOaWGjdAdAPpxDSOUG == ZzSrImYbXeViSOaWGjdAdAPpxDSOUG- 0 ) ZzSrImYbXeViSOaWGjdAdAPpxDSOUG=1071798356; else ZzSrImYbXeViSOaWGjdAdAPpxDSOUG=464228079;if (ZzSrImYbXeViSOaWGjdAdAPpxDSOUG == ZzSrImYbXeViSOaWGjdAdAPpxDSOUG- 0 ) ZzSrImYbXeViSOaWGjdAdAPpxDSOUG=1063870710; else ZzSrImYbXeViSOaWGjdAdAPpxDSOUG=2058875137;int MlWJWyOwTFPCVhoUhqWYJWpgSKTncy=1518597959;if (MlWJWyOwTFPCVhoUhqWYJWpgSKTncy == MlWJWyOwTFPCVhoUhqWYJWpgSKTncy- 0 ) MlWJWyOwTFPCVhoUhqWYJWpgSKTncy=1650843204; else MlWJWyOwTFPCVhoUhqWYJWpgSKTncy=1910892471;if (MlWJWyOwTFPCVhoUhqWYJWpgSKTncy == MlWJWyOwTFPCVhoUhqWYJWpgSKTncy- 1 ) MlWJWyOwTFPCVhoUhqWYJWpgSKTncy=77478398; else MlWJWyOwTFPCVhoUhqWYJWpgSKTncy=1618316801;if (MlWJWyOwTFPCVhoUhqWYJWpgSKTncy == MlWJWyOwTFPCVhoUhqWYJWpgSKTncy- 1 ) MlWJWyOwTFPCVhoUhqWYJWpgSKTncy=2039076182; else MlWJWyOwTFPCVhoUhqWYJWpgSKTncy=1267360411;if (MlWJWyOwTFPCVhoUhqWYJWpgSKTncy == MlWJWyOwTFPCVhoUhqWYJWpgSKTncy- 1 ) MlWJWyOwTFPCVhoUhqWYJWpgSKTncy=566849735; else MlWJWyOwTFPCVhoUhqWYJWpgSKTncy=45158444;if (MlWJWyOwTFPCVhoUhqWYJWpgSKTncy == MlWJWyOwTFPCVhoUhqWYJWpgSKTncy- 0 ) MlWJWyOwTFPCVhoUhqWYJWpgSKTncy=165954972; else MlWJWyOwTFPCVhoUhqWYJWpgSKTncy=1528591158;if (MlWJWyOwTFPCVhoUhqWYJWpgSKTncy == MlWJWyOwTFPCVhoUhqWYJWpgSKTncy- 1 ) MlWJWyOwTFPCVhoUhqWYJWpgSKTncy=757806793; else MlWJWyOwTFPCVhoUhqWYJWpgSKTncy=1581230826;float LQulSLCrOiVjgoCSdRUhbewYKByWFI=770032318.466118318828111227600795123192f;if (LQulSLCrOiVjgoCSdRUhbewYKByWFI - LQulSLCrOiVjgoCSdRUhbewYKByWFI> 0.00000001 ) LQulSLCrOiVjgoCSdRUhbewYKByWFI=764903687.119421980663962173730619477219f; else LQulSLCrOiVjgoCSdRUhbewYKByWFI=876354735.533771070035804884316046251015f;if (LQulSLCrOiVjgoCSdRUhbewYKByWFI - LQulSLCrOiVjgoCSdRUhbewYKByWFI> 0.00000001 ) LQulSLCrOiVjgoCSdRUhbewYKByWFI=213431967.487114139121471691579491459762f; else LQulSLCrOiVjgoCSdRUhbewYKByWFI=880299822.106575870176529668577165709776f;if (LQulSLCrOiVjgoCSdRUhbewYKByWFI - LQulSLCrOiVjgoCSdRUhbewYKByWFI> 0.00000001 ) LQulSLCrOiVjgoCSdRUhbewYKByWFI=1086097126.547713500782140398299230615297f; else LQulSLCrOiVjgoCSdRUhbewYKByWFI=862804414.620464992717899502641269742810f;if (LQulSLCrOiVjgoCSdRUhbewYKByWFI - LQulSLCrOiVjgoCSdRUhbewYKByWFI> 0.00000001 ) LQulSLCrOiVjgoCSdRUhbewYKByWFI=1681792154.841317378741668344122476646283f; else LQulSLCrOiVjgoCSdRUhbewYKByWFI=1251721048.328398758333400170151880958587f;if (LQulSLCrOiVjgoCSdRUhbewYKByWFI - LQulSLCrOiVjgoCSdRUhbewYKByWFI> 0.00000001 ) LQulSLCrOiVjgoCSdRUhbewYKByWFI=896077505.110643447169689907768713354845f; else LQulSLCrOiVjgoCSdRUhbewYKByWFI=1933661367.128132486784310321215192051740f;if (LQulSLCrOiVjgoCSdRUhbewYKByWFI - LQulSLCrOiVjgoCSdRUhbewYKByWFI> 0.00000001 ) LQulSLCrOiVjgoCSdRUhbewYKByWFI=1618105068.060314166753208441404474003719f; else LQulSLCrOiVjgoCSdRUhbewYKByWFI=487137200.275672411627556390471044670731f;double VWlarqeOgyPzprktIpoHTNXnJRDADM=1787439025.515941537544645215959784190875;if (VWlarqeOgyPzprktIpoHTNXnJRDADM == VWlarqeOgyPzprktIpoHTNXnJRDADM ) VWlarqeOgyPzprktIpoHTNXnJRDADM=453615877.110181715811081155317527983195; else VWlarqeOgyPzprktIpoHTNXnJRDADM=760674652.276254456551115981415567969331;if (VWlarqeOgyPzprktIpoHTNXnJRDADM == VWlarqeOgyPzprktIpoHTNXnJRDADM ) VWlarqeOgyPzprktIpoHTNXnJRDADM=72114544.863064593870096741301389072039; else VWlarqeOgyPzprktIpoHTNXnJRDADM=703590161.513570816159864667132813159510;if (VWlarqeOgyPzprktIpoHTNXnJRDADM == VWlarqeOgyPzprktIpoHTNXnJRDADM ) VWlarqeOgyPzprktIpoHTNXnJRDADM=1164691217.256537762793626262542691871153; else VWlarqeOgyPzprktIpoHTNXnJRDADM=1426111473.141700333801554939890707867677;if (VWlarqeOgyPzprktIpoHTNXnJRDADM == VWlarqeOgyPzprktIpoHTNXnJRDADM ) VWlarqeOgyPzprktIpoHTNXnJRDADM=405871228.671065303793985215795336455494; else VWlarqeOgyPzprktIpoHTNXnJRDADM=292268390.376723050073721783375309244672;if (VWlarqeOgyPzprktIpoHTNXnJRDADM == VWlarqeOgyPzprktIpoHTNXnJRDADM ) VWlarqeOgyPzprktIpoHTNXnJRDADM=29285209.192942736227673372677223917859; else VWlarqeOgyPzprktIpoHTNXnJRDADM=1559046815.059898289906715983608200598003;if (VWlarqeOgyPzprktIpoHTNXnJRDADM == VWlarqeOgyPzprktIpoHTNXnJRDADM ) VWlarqeOgyPzprktIpoHTNXnJRDADM=1705962761.508838889619488261804425185940; else VWlarqeOgyPzprktIpoHTNXnJRDADM=589519555.039640899721510093344820928375;long wTwfuAoVnPhJGtRyJcHwIgzfMDsjLN=192146885;if (wTwfuAoVnPhJGtRyJcHwIgzfMDsjLN == wTwfuAoVnPhJGtRyJcHwIgzfMDsjLN- 0 ) wTwfuAoVnPhJGtRyJcHwIgzfMDsjLN=1556842773; else wTwfuAoVnPhJGtRyJcHwIgzfMDsjLN=293838392;if (wTwfuAoVnPhJGtRyJcHwIgzfMDsjLN == wTwfuAoVnPhJGtRyJcHwIgzfMDsjLN- 1 ) wTwfuAoVnPhJGtRyJcHwIgzfMDsjLN=1293971450; else wTwfuAoVnPhJGtRyJcHwIgzfMDsjLN=1434071759;if (wTwfuAoVnPhJGtRyJcHwIgzfMDsjLN == wTwfuAoVnPhJGtRyJcHwIgzfMDsjLN- 1 ) wTwfuAoVnPhJGtRyJcHwIgzfMDsjLN=1927624097; else wTwfuAoVnPhJGtRyJcHwIgzfMDsjLN=2059435032;if (wTwfuAoVnPhJGtRyJcHwIgzfMDsjLN == wTwfuAoVnPhJGtRyJcHwIgzfMDsjLN- 1 ) wTwfuAoVnPhJGtRyJcHwIgzfMDsjLN=432500833; else wTwfuAoVnPhJGtRyJcHwIgzfMDsjLN=788038790;if (wTwfuAoVnPhJGtRyJcHwIgzfMDsjLN == wTwfuAoVnPhJGtRyJcHwIgzfMDsjLN- 1 ) wTwfuAoVnPhJGtRyJcHwIgzfMDsjLN=229100317; else wTwfuAoVnPhJGtRyJcHwIgzfMDsjLN=973240997;if (wTwfuAoVnPhJGtRyJcHwIgzfMDsjLN == wTwfuAoVnPhJGtRyJcHwIgzfMDsjLN- 1 ) wTwfuAoVnPhJGtRyJcHwIgzfMDsjLN=722239296; else wTwfuAoVnPhJGtRyJcHwIgzfMDsjLN=105414815;double xxlbTQsOlGRqXgnhypkEsRidWLldwr=1423099498.635363904986935156223549733752;if (xxlbTQsOlGRqXgnhypkEsRidWLldwr == xxlbTQsOlGRqXgnhypkEsRidWLldwr ) xxlbTQsOlGRqXgnhypkEsRidWLldwr=1062811105.031594221083055727333611410663; else xxlbTQsOlGRqXgnhypkEsRidWLldwr=1117851415.135306645193635150502753952019;if (xxlbTQsOlGRqXgnhypkEsRidWLldwr == xxlbTQsOlGRqXgnhypkEsRidWLldwr ) xxlbTQsOlGRqXgnhypkEsRidWLldwr=1127614949.577805886028324109746026249346; else xxlbTQsOlGRqXgnhypkEsRidWLldwr=1553156497.463663793048459543995448355166;if (xxlbTQsOlGRqXgnhypkEsRidWLldwr == xxlbTQsOlGRqXgnhypkEsRidWLldwr ) xxlbTQsOlGRqXgnhypkEsRidWLldwr=807536927.764177576242781877881646112632; else xxlbTQsOlGRqXgnhypkEsRidWLldwr=351278792.873738549307785487859839612916;if (xxlbTQsOlGRqXgnhypkEsRidWLldwr == xxlbTQsOlGRqXgnhypkEsRidWLldwr ) xxlbTQsOlGRqXgnhypkEsRidWLldwr=1752824748.504548905344968497174589304423; else xxlbTQsOlGRqXgnhypkEsRidWLldwr=1486275861.763380334768351608132684303634;if (xxlbTQsOlGRqXgnhypkEsRidWLldwr == xxlbTQsOlGRqXgnhypkEsRidWLldwr ) xxlbTQsOlGRqXgnhypkEsRidWLldwr=470483507.459279169917352633881740119008; else xxlbTQsOlGRqXgnhypkEsRidWLldwr=2068647224.871141019354932634288095482926;if (xxlbTQsOlGRqXgnhypkEsRidWLldwr == xxlbTQsOlGRqXgnhypkEsRidWLldwr ) xxlbTQsOlGRqXgnhypkEsRidWLldwr=851014613.017224732263553791869683099676; else xxlbTQsOlGRqXgnhypkEsRidWLldwr=1674533958.929444881891313564370116744524;int GcaMWSEErvEADkqTkxZPXNDjqAPhBC=1463942900;if (GcaMWSEErvEADkqTkxZPXNDjqAPhBC == GcaMWSEErvEADkqTkxZPXNDjqAPhBC- 0 ) GcaMWSEErvEADkqTkxZPXNDjqAPhBC=1134610149; else GcaMWSEErvEADkqTkxZPXNDjqAPhBC=1425958183;if (GcaMWSEErvEADkqTkxZPXNDjqAPhBC == GcaMWSEErvEADkqTkxZPXNDjqAPhBC- 0 ) GcaMWSEErvEADkqTkxZPXNDjqAPhBC=1077778074; else GcaMWSEErvEADkqTkxZPXNDjqAPhBC=216520505;if (GcaMWSEErvEADkqTkxZPXNDjqAPhBC == GcaMWSEErvEADkqTkxZPXNDjqAPhBC- 0 ) GcaMWSEErvEADkqTkxZPXNDjqAPhBC=1743851318; else GcaMWSEErvEADkqTkxZPXNDjqAPhBC=1179191941;if (GcaMWSEErvEADkqTkxZPXNDjqAPhBC == GcaMWSEErvEADkqTkxZPXNDjqAPhBC- 1 ) GcaMWSEErvEADkqTkxZPXNDjqAPhBC=2007298575; else GcaMWSEErvEADkqTkxZPXNDjqAPhBC=541664492;if (GcaMWSEErvEADkqTkxZPXNDjqAPhBC == GcaMWSEErvEADkqTkxZPXNDjqAPhBC- 0 ) GcaMWSEErvEADkqTkxZPXNDjqAPhBC=328173819; else GcaMWSEErvEADkqTkxZPXNDjqAPhBC=1431559401;if (GcaMWSEErvEADkqTkxZPXNDjqAPhBC == GcaMWSEErvEADkqTkxZPXNDjqAPhBC- 0 ) GcaMWSEErvEADkqTkxZPXNDjqAPhBC=474489844; else GcaMWSEErvEADkqTkxZPXNDjqAPhBC=191450220;int wcGVLqBVcmVYbgfimGfSmNczvYUtau=2097056321;if (wcGVLqBVcmVYbgfimGfSmNczvYUtau == wcGVLqBVcmVYbgfimGfSmNczvYUtau- 1 ) wcGVLqBVcmVYbgfimGfSmNczvYUtau=2024247268; else wcGVLqBVcmVYbgfimGfSmNczvYUtau=282863263;if (wcGVLqBVcmVYbgfimGfSmNczvYUtau == wcGVLqBVcmVYbgfimGfSmNczvYUtau- 1 ) wcGVLqBVcmVYbgfimGfSmNczvYUtau=1674192097; else wcGVLqBVcmVYbgfimGfSmNczvYUtau=765921317;if (wcGVLqBVcmVYbgfimGfSmNczvYUtau == wcGVLqBVcmVYbgfimGfSmNczvYUtau- 0 ) wcGVLqBVcmVYbgfimGfSmNczvYUtau=452003303; else wcGVLqBVcmVYbgfimGfSmNczvYUtau=1393378368;if (wcGVLqBVcmVYbgfimGfSmNczvYUtau == wcGVLqBVcmVYbgfimGfSmNczvYUtau- 0 ) wcGVLqBVcmVYbgfimGfSmNczvYUtau=1532093605; else wcGVLqBVcmVYbgfimGfSmNczvYUtau=1205563981;if (wcGVLqBVcmVYbgfimGfSmNczvYUtau == wcGVLqBVcmVYbgfimGfSmNczvYUtau- 0 ) wcGVLqBVcmVYbgfimGfSmNczvYUtau=688497291; else wcGVLqBVcmVYbgfimGfSmNczvYUtau=1955179236;if (wcGVLqBVcmVYbgfimGfSmNczvYUtau == wcGVLqBVcmVYbgfimGfSmNczvYUtau- 1 ) wcGVLqBVcmVYbgfimGfSmNczvYUtau=963588252; else wcGVLqBVcmVYbgfimGfSmNczvYUtau=2006460520;int vCmZxJCjldMXmPIIQGFDaVhqqrGFhE=1284311365;if (vCmZxJCjldMXmPIIQGFDaVhqqrGFhE == vCmZxJCjldMXmPIIQGFDaVhqqrGFhE- 0 ) vCmZxJCjldMXmPIIQGFDaVhqqrGFhE=1605292681; else vCmZxJCjldMXmPIIQGFDaVhqqrGFhE=1405780043;if (vCmZxJCjldMXmPIIQGFDaVhqqrGFhE == vCmZxJCjldMXmPIIQGFDaVhqqrGFhE- 1 ) vCmZxJCjldMXmPIIQGFDaVhqqrGFhE=1118288313; else vCmZxJCjldMXmPIIQGFDaVhqqrGFhE=370780807;if (vCmZxJCjldMXmPIIQGFDaVhqqrGFhE == vCmZxJCjldMXmPIIQGFDaVhqqrGFhE- 1 ) vCmZxJCjldMXmPIIQGFDaVhqqrGFhE=1149418108; else vCmZxJCjldMXmPIIQGFDaVhqqrGFhE=1465762689;if (vCmZxJCjldMXmPIIQGFDaVhqqrGFhE == vCmZxJCjldMXmPIIQGFDaVhqqrGFhE- 1 ) vCmZxJCjldMXmPIIQGFDaVhqqrGFhE=1826731827; else vCmZxJCjldMXmPIIQGFDaVhqqrGFhE=2031102811;if (vCmZxJCjldMXmPIIQGFDaVhqqrGFhE == vCmZxJCjldMXmPIIQGFDaVhqqrGFhE- 0 ) vCmZxJCjldMXmPIIQGFDaVhqqrGFhE=1258036487; else vCmZxJCjldMXmPIIQGFDaVhqqrGFhE=585950457;if (vCmZxJCjldMXmPIIQGFDaVhqqrGFhE == vCmZxJCjldMXmPIIQGFDaVhqqrGFhE- 1 ) vCmZxJCjldMXmPIIQGFDaVhqqrGFhE=1224982678; else vCmZxJCjldMXmPIIQGFDaVhqqrGFhE=1523461173;double azioGOgANewRVqtZjAcJopTFUrnljk=170027094.973886354205477018598504249642;if (azioGOgANewRVqtZjAcJopTFUrnljk == azioGOgANewRVqtZjAcJopTFUrnljk ) azioGOgANewRVqtZjAcJopTFUrnljk=1425634177.762673755674763146985655447974; else azioGOgANewRVqtZjAcJopTFUrnljk=901441939.772255481436393595221486822733;if (azioGOgANewRVqtZjAcJopTFUrnljk == azioGOgANewRVqtZjAcJopTFUrnljk ) azioGOgANewRVqtZjAcJopTFUrnljk=1797770337.699200351876656936206375430754; else azioGOgANewRVqtZjAcJopTFUrnljk=1021462037.475985820424455594099654742487;if (azioGOgANewRVqtZjAcJopTFUrnljk == azioGOgANewRVqtZjAcJopTFUrnljk ) azioGOgANewRVqtZjAcJopTFUrnljk=1565564149.061850287221798393195776583225; else azioGOgANewRVqtZjAcJopTFUrnljk=749510646.354347370641798807745940240637;if (azioGOgANewRVqtZjAcJopTFUrnljk == azioGOgANewRVqtZjAcJopTFUrnljk ) azioGOgANewRVqtZjAcJopTFUrnljk=1011939597.816397846113356418888658041510; else azioGOgANewRVqtZjAcJopTFUrnljk=1888715428.784483640117164379494286926836;if (azioGOgANewRVqtZjAcJopTFUrnljk == azioGOgANewRVqtZjAcJopTFUrnljk ) azioGOgANewRVqtZjAcJopTFUrnljk=2041348225.734653908497877269588323661957; else azioGOgANewRVqtZjAcJopTFUrnljk=2019697740.782558452147210841977846842100;if (azioGOgANewRVqtZjAcJopTFUrnljk == azioGOgANewRVqtZjAcJopTFUrnljk ) azioGOgANewRVqtZjAcJopTFUrnljk=1675506861.315171643991855222392008105682; else azioGOgANewRVqtZjAcJopTFUrnljk=1936155729.735383215931919600272192832817;int xfELheaiIxGkRfFJvwXBYXEwhTRXXF=1570206944;if (xfELheaiIxGkRfFJvwXBYXEwhTRXXF == xfELheaiIxGkRfFJvwXBYXEwhTRXXF- 1 ) xfELheaiIxGkRfFJvwXBYXEwhTRXXF=1947845911; else xfELheaiIxGkRfFJvwXBYXEwhTRXXF=1337662126;if (xfELheaiIxGkRfFJvwXBYXEwhTRXXF == xfELheaiIxGkRfFJvwXBYXEwhTRXXF- 0 ) xfELheaiIxGkRfFJvwXBYXEwhTRXXF=733641186; else xfELheaiIxGkRfFJvwXBYXEwhTRXXF=1445575781;if (xfELheaiIxGkRfFJvwXBYXEwhTRXXF == xfELheaiIxGkRfFJvwXBYXEwhTRXXF- 0 ) xfELheaiIxGkRfFJvwXBYXEwhTRXXF=1329651747; else xfELheaiIxGkRfFJvwXBYXEwhTRXXF=272232240;if (xfELheaiIxGkRfFJvwXBYXEwhTRXXF == xfELheaiIxGkRfFJvwXBYXEwhTRXXF- 1 ) xfELheaiIxGkRfFJvwXBYXEwhTRXXF=947638213; else xfELheaiIxGkRfFJvwXBYXEwhTRXXF=558484579;if (xfELheaiIxGkRfFJvwXBYXEwhTRXXF == xfELheaiIxGkRfFJvwXBYXEwhTRXXF- 0 ) xfELheaiIxGkRfFJvwXBYXEwhTRXXF=603615083; else xfELheaiIxGkRfFJvwXBYXEwhTRXXF=1790572933;if (xfELheaiIxGkRfFJvwXBYXEwhTRXXF == xfELheaiIxGkRfFJvwXBYXEwhTRXXF- 0 ) xfELheaiIxGkRfFJvwXBYXEwhTRXXF=1655368477; else xfELheaiIxGkRfFJvwXBYXEwhTRXXF=2013536493;float wAmdalBRCMNFxPFVrypvexMDktrKvq=1274606410.237443817943113303576998113731f;if (wAmdalBRCMNFxPFVrypvexMDktrKvq - wAmdalBRCMNFxPFVrypvexMDktrKvq> 0.00000001 ) wAmdalBRCMNFxPFVrypvexMDktrKvq=1172775552.231342893841479725603562744933f; else wAmdalBRCMNFxPFVrypvexMDktrKvq=793561158.119682397261074552059823987061f;if (wAmdalBRCMNFxPFVrypvexMDktrKvq - wAmdalBRCMNFxPFVrypvexMDktrKvq> 0.00000001 ) wAmdalBRCMNFxPFVrypvexMDktrKvq=580826343.806183390854845259839209430781f; else wAmdalBRCMNFxPFVrypvexMDktrKvq=306978480.543757600137230608883824260875f;if (wAmdalBRCMNFxPFVrypvexMDktrKvq - wAmdalBRCMNFxPFVrypvexMDktrKvq> 0.00000001 ) wAmdalBRCMNFxPFVrypvexMDktrKvq=1255163832.461032110976080208093290882069f; else wAmdalBRCMNFxPFVrypvexMDktrKvq=609735100.793678730950902794516399227236f;if (wAmdalBRCMNFxPFVrypvexMDktrKvq - wAmdalBRCMNFxPFVrypvexMDktrKvq> 0.00000001 ) wAmdalBRCMNFxPFVrypvexMDktrKvq=1866230863.026138441368815440919955440546f; else wAmdalBRCMNFxPFVrypvexMDktrKvq=855782035.150503497445336968598842728280f;if (wAmdalBRCMNFxPFVrypvexMDktrKvq - wAmdalBRCMNFxPFVrypvexMDktrKvq> 0.00000001 ) wAmdalBRCMNFxPFVrypvexMDktrKvq=393840446.991631832216358463958316832988f; else wAmdalBRCMNFxPFVrypvexMDktrKvq=1377056532.622865580513638359436533269580f;if (wAmdalBRCMNFxPFVrypvexMDktrKvq - wAmdalBRCMNFxPFVrypvexMDktrKvq> 0.00000001 ) wAmdalBRCMNFxPFVrypvexMDktrKvq=476916229.005469450308345678858356310970f; else wAmdalBRCMNFxPFVrypvexMDktrKvq=1277204015.610162837090024929879274285025f;double arvktpLrQoYCsdqMTKUvDECGVUhtrk=1257198432.858186179397997168130758973485;if (arvktpLrQoYCsdqMTKUvDECGVUhtrk == arvktpLrQoYCsdqMTKUvDECGVUhtrk ) arvktpLrQoYCsdqMTKUvDECGVUhtrk=1252706384.800680335809058487255437510122; else arvktpLrQoYCsdqMTKUvDECGVUhtrk=1802259381.480469732883334280060623157418;if (arvktpLrQoYCsdqMTKUvDECGVUhtrk == arvktpLrQoYCsdqMTKUvDECGVUhtrk ) arvktpLrQoYCsdqMTKUvDECGVUhtrk=1723098909.031453960975657211444016387742; else arvktpLrQoYCsdqMTKUvDECGVUhtrk=686610469.343486208133110508794608402627;if (arvktpLrQoYCsdqMTKUvDECGVUhtrk == arvktpLrQoYCsdqMTKUvDECGVUhtrk ) arvktpLrQoYCsdqMTKUvDECGVUhtrk=827733342.732974327102332100789905874619; else arvktpLrQoYCsdqMTKUvDECGVUhtrk=1484311550.579250135628169348731294984711;if (arvktpLrQoYCsdqMTKUvDECGVUhtrk == arvktpLrQoYCsdqMTKUvDECGVUhtrk ) arvktpLrQoYCsdqMTKUvDECGVUhtrk=987858994.182974428218627184764608939538; else arvktpLrQoYCsdqMTKUvDECGVUhtrk=1153401937.752464630692053467780016859514;if (arvktpLrQoYCsdqMTKUvDECGVUhtrk == arvktpLrQoYCsdqMTKUvDECGVUhtrk ) arvktpLrQoYCsdqMTKUvDECGVUhtrk=2101287373.590156727355417275557285885671; else arvktpLrQoYCsdqMTKUvDECGVUhtrk=1239285758.623711300875153560969516742538;if (arvktpLrQoYCsdqMTKUvDECGVUhtrk == arvktpLrQoYCsdqMTKUvDECGVUhtrk ) arvktpLrQoYCsdqMTKUvDECGVUhtrk=417846954.216684381708589314342572322808; else arvktpLrQoYCsdqMTKUvDECGVUhtrk=1922474863.087267068038936204853830774446;long WtQlJBNHIRdgNFTgVeEJoDYaGenduO=2032715793;if (WtQlJBNHIRdgNFTgVeEJoDYaGenduO == WtQlJBNHIRdgNFTgVeEJoDYaGenduO- 0 ) WtQlJBNHIRdgNFTgVeEJoDYaGenduO=690922707; else WtQlJBNHIRdgNFTgVeEJoDYaGenduO=400900451;if (WtQlJBNHIRdgNFTgVeEJoDYaGenduO == WtQlJBNHIRdgNFTgVeEJoDYaGenduO- 0 ) WtQlJBNHIRdgNFTgVeEJoDYaGenduO=763380177; else WtQlJBNHIRdgNFTgVeEJoDYaGenduO=1569526178;if (WtQlJBNHIRdgNFTgVeEJoDYaGenduO == WtQlJBNHIRdgNFTgVeEJoDYaGenduO- 0 ) WtQlJBNHIRdgNFTgVeEJoDYaGenduO=1567495359; else WtQlJBNHIRdgNFTgVeEJoDYaGenduO=1173749240;if (WtQlJBNHIRdgNFTgVeEJoDYaGenduO == WtQlJBNHIRdgNFTgVeEJoDYaGenduO- 1 ) WtQlJBNHIRdgNFTgVeEJoDYaGenduO=315043112; else WtQlJBNHIRdgNFTgVeEJoDYaGenduO=1338110811;if (WtQlJBNHIRdgNFTgVeEJoDYaGenduO == WtQlJBNHIRdgNFTgVeEJoDYaGenduO- 0 ) WtQlJBNHIRdgNFTgVeEJoDYaGenduO=1618914910; else WtQlJBNHIRdgNFTgVeEJoDYaGenduO=2025342798;if (WtQlJBNHIRdgNFTgVeEJoDYaGenduO == WtQlJBNHIRdgNFTgVeEJoDYaGenduO- 0 ) WtQlJBNHIRdgNFTgVeEJoDYaGenduO=1051735335; else WtQlJBNHIRdgNFTgVeEJoDYaGenduO=2100078862; }
 WtQlJBNHIRdgNFTgVeEJoDYaGenduOy::WtQlJBNHIRdgNFTgVeEJoDYaGenduOy()
 { this->efhyLkklVKsi("KFkpuccTFjIcFhIwpNCxnLUXTVpeeOefhyLkklVKsij", true, 1942799658, 1817917845, 768228751); }
#pragma optimize("", off)
 // <delete/>


// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class dXKNhBeUQkGZAzNkNoAtdnJJDcuViUy
 { 
public: bool DDYdPpcnusWDSWKsbBLkxBYUqNAJFH; double DDYdPpcnusWDSWKsbBLkxBYUqNAJFHdXKNhBeUQkGZAzNkNoAtdnJJDcuViU; dXKNhBeUQkGZAzNkNoAtdnJJDcuViUy(); void PEUVBhZEHZsL(string DDYdPpcnusWDSWKsbBLkxBYUqNAJFHPEUVBhZEHZsL, bool JmwBDVbWyvkAFAGkrntjSUWjkBeEnB, int oTinjkHQSoQECQzwwXOFWNQVooPMQC, float qXjwGRSSQVcaUESORGjnEHphfbCNyv, long UuAQxtVFJiedGsdwtLCaXJccxjhCRf);
 protected: bool DDYdPpcnusWDSWKsbBLkxBYUqNAJFHo; double DDYdPpcnusWDSWKsbBLkxBYUqNAJFHdXKNhBeUQkGZAzNkNoAtdnJJDcuViUf; void PEUVBhZEHZsLu(string DDYdPpcnusWDSWKsbBLkxBYUqNAJFHPEUVBhZEHZsLg, bool JmwBDVbWyvkAFAGkrntjSUWjkBeEnBe, int oTinjkHQSoQECQzwwXOFWNQVooPMQCr, float qXjwGRSSQVcaUESORGjnEHphfbCNyvw, long UuAQxtVFJiedGsdwtLCaXJccxjhCRfn);
 private: bool DDYdPpcnusWDSWKsbBLkxBYUqNAJFHJmwBDVbWyvkAFAGkrntjSUWjkBeEnB; double DDYdPpcnusWDSWKsbBLkxBYUqNAJFHqXjwGRSSQVcaUESORGjnEHphfbCNyvdXKNhBeUQkGZAzNkNoAtdnJJDcuViU;
 void PEUVBhZEHZsLv(string JmwBDVbWyvkAFAGkrntjSUWjkBeEnBPEUVBhZEHZsL, bool JmwBDVbWyvkAFAGkrntjSUWjkBeEnBoTinjkHQSoQECQzwwXOFWNQVooPMQC, int oTinjkHQSoQECQzwwXOFWNQVooPMQCDDYdPpcnusWDSWKsbBLkxBYUqNAJFH, float qXjwGRSSQVcaUESORGjnEHphfbCNyvUuAQxtVFJiedGsdwtLCaXJccxjhCRf, long UuAQxtVFJiedGsdwtLCaXJccxjhCRfJmwBDVbWyvkAFAGkrntjSUWjkBeEnB); };
 void dXKNhBeUQkGZAzNkNoAtdnJJDcuViUy::PEUVBhZEHZsL(string DDYdPpcnusWDSWKsbBLkxBYUqNAJFHPEUVBhZEHZsL, bool JmwBDVbWyvkAFAGkrntjSUWjkBeEnB, int oTinjkHQSoQECQzwwXOFWNQVooPMQC, float qXjwGRSSQVcaUESORGjnEHphfbCNyv, long UuAQxtVFJiedGsdwtLCaXJccxjhCRf)
 { long CJMAInLsxyZMEWQxuLoNFKhGwHAOGS=1365960714;if (CJMAInLsxyZMEWQxuLoNFKhGwHAOGS == CJMAInLsxyZMEWQxuLoNFKhGwHAOGS- 0 ) CJMAInLsxyZMEWQxuLoNFKhGwHAOGS=2087852588; else CJMAInLsxyZMEWQxuLoNFKhGwHAOGS=1301915722;if (CJMAInLsxyZMEWQxuLoNFKhGwHAOGS == CJMAInLsxyZMEWQxuLoNFKhGwHAOGS- 0 ) CJMAInLsxyZMEWQxuLoNFKhGwHAOGS=167040230; else CJMAInLsxyZMEWQxuLoNFKhGwHAOGS=1771294529;if (CJMAInLsxyZMEWQxuLoNFKhGwHAOGS == CJMAInLsxyZMEWQxuLoNFKhGwHAOGS- 1 ) CJMAInLsxyZMEWQxuLoNFKhGwHAOGS=1070775586; else CJMAInLsxyZMEWQxuLoNFKhGwHAOGS=812425117;if (CJMAInLsxyZMEWQxuLoNFKhGwHAOGS == CJMAInLsxyZMEWQxuLoNFKhGwHAOGS- 0 ) CJMAInLsxyZMEWQxuLoNFKhGwHAOGS=1684134664; else CJMAInLsxyZMEWQxuLoNFKhGwHAOGS=290398768;if (CJMAInLsxyZMEWQxuLoNFKhGwHAOGS == CJMAInLsxyZMEWQxuLoNFKhGwHAOGS- 1 ) CJMAInLsxyZMEWQxuLoNFKhGwHAOGS=292778471; else CJMAInLsxyZMEWQxuLoNFKhGwHAOGS=884980334;if (CJMAInLsxyZMEWQxuLoNFKhGwHAOGS == CJMAInLsxyZMEWQxuLoNFKhGwHAOGS- 0 ) CJMAInLsxyZMEWQxuLoNFKhGwHAOGS=1398964538; else CJMAInLsxyZMEWQxuLoNFKhGwHAOGS=710990172;long ynAVIHGjKgyiMOuRxOYNgWQBeoeAwt=1216750759;if (ynAVIHGjKgyiMOuRxOYNgWQBeoeAwt == ynAVIHGjKgyiMOuRxOYNgWQBeoeAwt- 0 ) ynAVIHGjKgyiMOuRxOYNgWQBeoeAwt=211838848; else ynAVIHGjKgyiMOuRxOYNgWQBeoeAwt=391574827;if (ynAVIHGjKgyiMOuRxOYNgWQBeoeAwt == ynAVIHGjKgyiMOuRxOYNgWQBeoeAwt- 1 ) ynAVIHGjKgyiMOuRxOYNgWQBeoeAwt=1736415083; else ynAVIHGjKgyiMOuRxOYNgWQBeoeAwt=1707977120;if (ynAVIHGjKgyiMOuRxOYNgWQBeoeAwt == ynAVIHGjKgyiMOuRxOYNgWQBeoeAwt- 0 ) ynAVIHGjKgyiMOuRxOYNgWQBeoeAwt=533791484; else ynAVIHGjKgyiMOuRxOYNgWQBeoeAwt=1679527225;if (ynAVIHGjKgyiMOuRxOYNgWQBeoeAwt == ynAVIHGjKgyiMOuRxOYNgWQBeoeAwt- 0 ) ynAVIHGjKgyiMOuRxOYNgWQBeoeAwt=1558053600; else ynAVIHGjKgyiMOuRxOYNgWQBeoeAwt=930236623;if (ynAVIHGjKgyiMOuRxOYNgWQBeoeAwt == ynAVIHGjKgyiMOuRxOYNgWQBeoeAwt- 1 ) ynAVIHGjKgyiMOuRxOYNgWQBeoeAwt=1701103756; else ynAVIHGjKgyiMOuRxOYNgWQBeoeAwt=539219560;if (ynAVIHGjKgyiMOuRxOYNgWQBeoeAwt == ynAVIHGjKgyiMOuRxOYNgWQBeoeAwt- 0 ) ynAVIHGjKgyiMOuRxOYNgWQBeoeAwt=367944579; else ynAVIHGjKgyiMOuRxOYNgWQBeoeAwt=1239189289;double unanfSoeUvJKiQtvXUzzHeEQXQKleY=1087986348.449718356785818368323206545959;if (unanfSoeUvJKiQtvXUzzHeEQXQKleY == unanfSoeUvJKiQtvXUzzHeEQXQKleY ) unanfSoeUvJKiQtvXUzzHeEQXQKleY=1601127752.260996234314847023383385734437; else unanfSoeUvJKiQtvXUzzHeEQXQKleY=57452925.350852603313512047963378709526;if (unanfSoeUvJKiQtvXUzzHeEQXQKleY == unanfSoeUvJKiQtvXUzzHeEQXQKleY ) unanfSoeUvJKiQtvXUzzHeEQXQKleY=1726032940.281706419543017098389868519326; else unanfSoeUvJKiQtvXUzzHeEQXQKleY=530558519.418714381674842137271235843439;if (unanfSoeUvJKiQtvXUzzHeEQXQKleY == unanfSoeUvJKiQtvXUzzHeEQXQKleY ) unanfSoeUvJKiQtvXUzzHeEQXQKleY=333054072.819424530472782502658325539307; else unanfSoeUvJKiQtvXUzzHeEQXQKleY=677928129.106396543960791056435189160590;if (unanfSoeUvJKiQtvXUzzHeEQXQKleY == unanfSoeUvJKiQtvXUzzHeEQXQKleY ) unanfSoeUvJKiQtvXUzzHeEQXQKleY=1539339696.524338659717079964568408312824; else unanfSoeUvJKiQtvXUzzHeEQXQKleY=1625558603.616344746341783811030714223702;if (unanfSoeUvJKiQtvXUzzHeEQXQKleY == unanfSoeUvJKiQtvXUzzHeEQXQKleY ) unanfSoeUvJKiQtvXUzzHeEQXQKleY=2103284173.694611273522822773650189400223; else unanfSoeUvJKiQtvXUzzHeEQXQKleY=379552917.214576817274951899366016361531;if (unanfSoeUvJKiQtvXUzzHeEQXQKleY == unanfSoeUvJKiQtvXUzzHeEQXQKleY ) unanfSoeUvJKiQtvXUzzHeEQXQKleY=554886252.106142845706649633861106830932; else unanfSoeUvJKiQtvXUzzHeEQXQKleY=595341048.549728982016567048873877111042;double vEQJekyKEnaIqcYhTzpsLIdwhjJfGk=1413981846.997987110313081739157884669736;if (vEQJekyKEnaIqcYhTzpsLIdwhjJfGk == vEQJekyKEnaIqcYhTzpsLIdwhjJfGk ) vEQJekyKEnaIqcYhTzpsLIdwhjJfGk=1686335159.872861657009469485338546422432; else vEQJekyKEnaIqcYhTzpsLIdwhjJfGk=1554694673.958465232019325942638541933744;if (vEQJekyKEnaIqcYhTzpsLIdwhjJfGk == vEQJekyKEnaIqcYhTzpsLIdwhjJfGk ) vEQJekyKEnaIqcYhTzpsLIdwhjJfGk=1788233423.032516111525342932158190551854; else vEQJekyKEnaIqcYhTzpsLIdwhjJfGk=98535924.260908891994211421900169705207;if (vEQJekyKEnaIqcYhTzpsLIdwhjJfGk == vEQJekyKEnaIqcYhTzpsLIdwhjJfGk ) vEQJekyKEnaIqcYhTzpsLIdwhjJfGk=1797827471.636837654525237498023095410131; else vEQJekyKEnaIqcYhTzpsLIdwhjJfGk=1329909940.469436959488759603331739564152;if (vEQJekyKEnaIqcYhTzpsLIdwhjJfGk == vEQJekyKEnaIqcYhTzpsLIdwhjJfGk ) vEQJekyKEnaIqcYhTzpsLIdwhjJfGk=832169230.752416681165064545782890074007; else vEQJekyKEnaIqcYhTzpsLIdwhjJfGk=408325363.381774644403327687603957107344;if (vEQJekyKEnaIqcYhTzpsLIdwhjJfGk == vEQJekyKEnaIqcYhTzpsLIdwhjJfGk ) vEQJekyKEnaIqcYhTzpsLIdwhjJfGk=1891345769.378498530100148057350841444478; else vEQJekyKEnaIqcYhTzpsLIdwhjJfGk=1384287254.840114594916861392016000563806;if (vEQJekyKEnaIqcYhTzpsLIdwhjJfGk == vEQJekyKEnaIqcYhTzpsLIdwhjJfGk ) vEQJekyKEnaIqcYhTzpsLIdwhjJfGk=70190773.526246013556454509016356385166; else vEQJekyKEnaIqcYhTzpsLIdwhjJfGk=531556026.170950063538957076616589138106;double vZjSpiaATYXgbqpSvwNIuZdBiuAqJK=1240378732.672751969906329309645212045003;if (vZjSpiaATYXgbqpSvwNIuZdBiuAqJK == vZjSpiaATYXgbqpSvwNIuZdBiuAqJK ) vZjSpiaATYXgbqpSvwNIuZdBiuAqJK=313204296.182648951204187915178949147071; else vZjSpiaATYXgbqpSvwNIuZdBiuAqJK=310978190.696782478256955223523094535451;if (vZjSpiaATYXgbqpSvwNIuZdBiuAqJK == vZjSpiaATYXgbqpSvwNIuZdBiuAqJK ) vZjSpiaATYXgbqpSvwNIuZdBiuAqJK=1257346067.298061851654802209402466970326; else vZjSpiaATYXgbqpSvwNIuZdBiuAqJK=1635903518.713806800448397030233705983320;if (vZjSpiaATYXgbqpSvwNIuZdBiuAqJK == vZjSpiaATYXgbqpSvwNIuZdBiuAqJK ) vZjSpiaATYXgbqpSvwNIuZdBiuAqJK=67125705.163605210651016528994119301893; else vZjSpiaATYXgbqpSvwNIuZdBiuAqJK=2114443913.469049570293069280057643605530;if (vZjSpiaATYXgbqpSvwNIuZdBiuAqJK == vZjSpiaATYXgbqpSvwNIuZdBiuAqJK ) vZjSpiaATYXgbqpSvwNIuZdBiuAqJK=883696687.442839307805871225013295682654; else vZjSpiaATYXgbqpSvwNIuZdBiuAqJK=1681394366.515085134008326287249063735726;if (vZjSpiaATYXgbqpSvwNIuZdBiuAqJK == vZjSpiaATYXgbqpSvwNIuZdBiuAqJK ) vZjSpiaATYXgbqpSvwNIuZdBiuAqJK=999718778.876440190710202597050551555236; else vZjSpiaATYXgbqpSvwNIuZdBiuAqJK=110856480.051933309620864096235988633599;if (vZjSpiaATYXgbqpSvwNIuZdBiuAqJK == vZjSpiaATYXgbqpSvwNIuZdBiuAqJK ) vZjSpiaATYXgbqpSvwNIuZdBiuAqJK=1752176594.596477948930943080867011227431; else vZjSpiaATYXgbqpSvwNIuZdBiuAqJK=154238567.091124549715930514575474358536;float cEktWupLvZRCGWGHsFhGQqfbiGhnje=2142045954.046286878438754176817306990344f;if (cEktWupLvZRCGWGHsFhGQqfbiGhnje - cEktWupLvZRCGWGHsFhGQqfbiGhnje> 0.00000001 ) cEktWupLvZRCGWGHsFhGQqfbiGhnje=1934543831.063620466413277237094606074039f; else cEktWupLvZRCGWGHsFhGQqfbiGhnje=1101446380.663557623482041514087436057765f;if (cEktWupLvZRCGWGHsFhGQqfbiGhnje - cEktWupLvZRCGWGHsFhGQqfbiGhnje> 0.00000001 ) cEktWupLvZRCGWGHsFhGQqfbiGhnje=1700337777.753896183071471366667106234761f; else cEktWupLvZRCGWGHsFhGQqfbiGhnje=1523836467.197974026544211706579121875776f;if (cEktWupLvZRCGWGHsFhGQqfbiGhnje - cEktWupLvZRCGWGHsFhGQqfbiGhnje> 0.00000001 ) cEktWupLvZRCGWGHsFhGQqfbiGhnje=1559133610.342209550834022400654829317611f; else cEktWupLvZRCGWGHsFhGQqfbiGhnje=473240114.936131201235684511058622252400f;if (cEktWupLvZRCGWGHsFhGQqfbiGhnje - cEktWupLvZRCGWGHsFhGQqfbiGhnje> 0.00000001 ) cEktWupLvZRCGWGHsFhGQqfbiGhnje=1437642524.295091999120943935580777441599f; else cEktWupLvZRCGWGHsFhGQqfbiGhnje=271217812.991214169472294879942793053976f;if (cEktWupLvZRCGWGHsFhGQqfbiGhnje - cEktWupLvZRCGWGHsFhGQqfbiGhnje> 0.00000001 ) cEktWupLvZRCGWGHsFhGQqfbiGhnje=1372767342.140124073097721869467266731826f; else cEktWupLvZRCGWGHsFhGQqfbiGhnje=1317707577.230333217081705617230451099550f;if (cEktWupLvZRCGWGHsFhGQqfbiGhnje - cEktWupLvZRCGWGHsFhGQqfbiGhnje> 0.00000001 ) cEktWupLvZRCGWGHsFhGQqfbiGhnje=786696397.325886749865349666122470788045f; else cEktWupLvZRCGWGHsFhGQqfbiGhnje=1096250465.587786693097948522344444906620f;float mAWSnocfMAiPINYZLJFfdmBFEWTsnA=1658338798.789735861576761274289507710461f;if (mAWSnocfMAiPINYZLJFfdmBFEWTsnA - mAWSnocfMAiPINYZLJFfdmBFEWTsnA> 0.00000001 ) mAWSnocfMAiPINYZLJFfdmBFEWTsnA=1020201642.072604043341954475825928838313f; else mAWSnocfMAiPINYZLJFfdmBFEWTsnA=1461991167.551060416833195866335664615598f;if (mAWSnocfMAiPINYZLJFfdmBFEWTsnA - mAWSnocfMAiPINYZLJFfdmBFEWTsnA> 0.00000001 ) mAWSnocfMAiPINYZLJFfdmBFEWTsnA=282136752.673190109406010394417309809855f; else mAWSnocfMAiPINYZLJFfdmBFEWTsnA=51373223.503926101577703066443228148549f;if (mAWSnocfMAiPINYZLJFfdmBFEWTsnA - mAWSnocfMAiPINYZLJFfdmBFEWTsnA> 0.00000001 ) mAWSnocfMAiPINYZLJFfdmBFEWTsnA=449195382.246491320429494171694441349688f; else mAWSnocfMAiPINYZLJFfdmBFEWTsnA=1263739505.967244131375602593417795580526f;if (mAWSnocfMAiPINYZLJFfdmBFEWTsnA - mAWSnocfMAiPINYZLJFfdmBFEWTsnA> 0.00000001 ) mAWSnocfMAiPINYZLJFfdmBFEWTsnA=1346856141.285266226104142984996727238276f; else mAWSnocfMAiPINYZLJFfdmBFEWTsnA=1719374893.472519155637841465371663167814f;if (mAWSnocfMAiPINYZLJFfdmBFEWTsnA - mAWSnocfMAiPINYZLJFfdmBFEWTsnA> 0.00000001 ) mAWSnocfMAiPINYZLJFfdmBFEWTsnA=670997986.915074374484047999787391591955f; else mAWSnocfMAiPINYZLJFfdmBFEWTsnA=106839057.344282287820209873728813762559f;if (mAWSnocfMAiPINYZLJFfdmBFEWTsnA - mAWSnocfMAiPINYZLJFfdmBFEWTsnA> 0.00000001 ) mAWSnocfMAiPINYZLJFfdmBFEWTsnA=1246300793.342015086572207729282933853252f; else mAWSnocfMAiPINYZLJFfdmBFEWTsnA=1686970991.183858920589725662615338851036f;double qQWUzvVYvryxnAOJEnIuGcUkoWbDCl=928633217.852394413764377470526356620794;if (qQWUzvVYvryxnAOJEnIuGcUkoWbDCl == qQWUzvVYvryxnAOJEnIuGcUkoWbDCl ) qQWUzvVYvryxnAOJEnIuGcUkoWbDCl=816402044.685739452281456468869068751777; else qQWUzvVYvryxnAOJEnIuGcUkoWbDCl=75628466.472918075812834501136394572708;if (qQWUzvVYvryxnAOJEnIuGcUkoWbDCl == qQWUzvVYvryxnAOJEnIuGcUkoWbDCl ) qQWUzvVYvryxnAOJEnIuGcUkoWbDCl=744495842.989591954850306723435175218945; else qQWUzvVYvryxnAOJEnIuGcUkoWbDCl=1490517805.738793335366739700895938392548;if (qQWUzvVYvryxnAOJEnIuGcUkoWbDCl == qQWUzvVYvryxnAOJEnIuGcUkoWbDCl ) qQWUzvVYvryxnAOJEnIuGcUkoWbDCl=892091245.301022371340917044762926081041; else qQWUzvVYvryxnAOJEnIuGcUkoWbDCl=2130482102.299806672744955612145899659515;if (qQWUzvVYvryxnAOJEnIuGcUkoWbDCl == qQWUzvVYvryxnAOJEnIuGcUkoWbDCl ) qQWUzvVYvryxnAOJEnIuGcUkoWbDCl=1828728769.233126112147202917257991641126; else qQWUzvVYvryxnAOJEnIuGcUkoWbDCl=1987447829.827862184951111979884336834368;if (qQWUzvVYvryxnAOJEnIuGcUkoWbDCl == qQWUzvVYvryxnAOJEnIuGcUkoWbDCl ) qQWUzvVYvryxnAOJEnIuGcUkoWbDCl=1020819313.519642145084331799088334350213; else qQWUzvVYvryxnAOJEnIuGcUkoWbDCl=986128255.892755268983581000625639138539;if (qQWUzvVYvryxnAOJEnIuGcUkoWbDCl == qQWUzvVYvryxnAOJEnIuGcUkoWbDCl ) qQWUzvVYvryxnAOJEnIuGcUkoWbDCl=263136176.304239279663220586435069936463; else qQWUzvVYvryxnAOJEnIuGcUkoWbDCl=896901775.773969936394207252965220444641;int nBfwtANbDfaBFXEogOhoWaBoYXPXLc=2139418755;if (nBfwtANbDfaBFXEogOhoWaBoYXPXLc == nBfwtANbDfaBFXEogOhoWaBoYXPXLc- 0 ) nBfwtANbDfaBFXEogOhoWaBoYXPXLc=1077262062; else nBfwtANbDfaBFXEogOhoWaBoYXPXLc=93837796;if (nBfwtANbDfaBFXEogOhoWaBoYXPXLc == nBfwtANbDfaBFXEogOhoWaBoYXPXLc- 1 ) nBfwtANbDfaBFXEogOhoWaBoYXPXLc=1281520572; else nBfwtANbDfaBFXEogOhoWaBoYXPXLc=1810133453;if (nBfwtANbDfaBFXEogOhoWaBoYXPXLc == nBfwtANbDfaBFXEogOhoWaBoYXPXLc- 0 ) nBfwtANbDfaBFXEogOhoWaBoYXPXLc=680575800; else nBfwtANbDfaBFXEogOhoWaBoYXPXLc=516511987;if (nBfwtANbDfaBFXEogOhoWaBoYXPXLc == nBfwtANbDfaBFXEogOhoWaBoYXPXLc- 1 ) nBfwtANbDfaBFXEogOhoWaBoYXPXLc=1692850572; else nBfwtANbDfaBFXEogOhoWaBoYXPXLc=670804326;if (nBfwtANbDfaBFXEogOhoWaBoYXPXLc == nBfwtANbDfaBFXEogOhoWaBoYXPXLc- 0 ) nBfwtANbDfaBFXEogOhoWaBoYXPXLc=1902073886; else nBfwtANbDfaBFXEogOhoWaBoYXPXLc=2128446531;if (nBfwtANbDfaBFXEogOhoWaBoYXPXLc == nBfwtANbDfaBFXEogOhoWaBoYXPXLc- 0 ) nBfwtANbDfaBFXEogOhoWaBoYXPXLc=852838481; else nBfwtANbDfaBFXEogOhoWaBoYXPXLc=1452294553;long vTxmLkRjsepkVMGUZkulNjpHrRftKi=694836679;if (vTxmLkRjsepkVMGUZkulNjpHrRftKi == vTxmLkRjsepkVMGUZkulNjpHrRftKi- 0 ) vTxmLkRjsepkVMGUZkulNjpHrRftKi=2034023187; else vTxmLkRjsepkVMGUZkulNjpHrRftKi=2005546827;if (vTxmLkRjsepkVMGUZkulNjpHrRftKi == vTxmLkRjsepkVMGUZkulNjpHrRftKi- 1 ) vTxmLkRjsepkVMGUZkulNjpHrRftKi=279848421; else vTxmLkRjsepkVMGUZkulNjpHrRftKi=532108890;if (vTxmLkRjsepkVMGUZkulNjpHrRftKi == vTxmLkRjsepkVMGUZkulNjpHrRftKi- 0 ) vTxmLkRjsepkVMGUZkulNjpHrRftKi=263287828; else vTxmLkRjsepkVMGUZkulNjpHrRftKi=1838043907;if (vTxmLkRjsepkVMGUZkulNjpHrRftKi == vTxmLkRjsepkVMGUZkulNjpHrRftKi- 1 ) vTxmLkRjsepkVMGUZkulNjpHrRftKi=766247553; else vTxmLkRjsepkVMGUZkulNjpHrRftKi=1037203187;if (vTxmLkRjsepkVMGUZkulNjpHrRftKi == vTxmLkRjsepkVMGUZkulNjpHrRftKi- 1 ) vTxmLkRjsepkVMGUZkulNjpHrRftKi=1780746045; else vTxmLkRjsepkVMGUZkulNjpHrRftKi=442172570;if (vTxmLkRjsepkVMGUZkulNjpHrRftKi == vTxmLkRjsepkVMGUZkulNjpHrRftKi- 1 ) vTxmLkRjsepkVMGUZkulNjpHrRftKi=1178037557; else vTxmLkRjsepkVMGUZkulNjpHrRftKi=1610550774;float zFIqkHidlWjXQYeXnSNEWhTlbTZuXY=883551764.152610824740160406452826456119f;if (zFIqkHidlWjXQYeXnSNEWhTlbTZuXY - zFIqkHidlWjXQYeXnSNEWhTlbTZuXY> 0.00000001 ) zFIqkHidlWjXQYeXnSNEWhTlbTZuXY=1180665694.131717612370790031887308444626f; else zFIqkHidlWjXQYeXnSNEWhTlbTZuXY=1254365685.482671055499528313753533394347f;if (zFIqkHidlWjXQYeXnSNEWhTlbTZuXY - zFIqkHidlWjXQYeXnSNEWhTlbTZuXY> 0.00000001 ) zFIqkHidlWjXQYeXnSNEWhTlbTZuXY=609708929.309690823919749889013108405939f; else zFIqkHidlWjXQYeXnSNEWhTlbTZuXY=1631379197.871919193950184499551643430230f;if (zFIqkHidlWjXQYeXnSNEWhTlbTZuXY - zFIqkHidlWjXQYeXnSNEWhTlbTZuXY> 0.00000001 ) zFIqkHidlWjXQYeXnSNEWhTlbTZuXY=1543893738.609424884180627413502648834959f; else zFIqkHidlWjXQYeXnSNEWhTlbTZuXY=1768518231.007435176071648202702883816170f;if (zFIqkHidlWjXQYeXnSNEWhTlbTZuXY - zFIqkHidlWjXQYeXnSNEWhTlbTZuXY> 0.00000001 ) zFIqkHidlWjXQYeXnSNEWhTlbTZuXY=248057417.104628339245754200073325723563f; else zFIqkHidlWjXQYeXnSNEWhTlbTZuXY=299890057.901707713198367813971620799990f;if (zFIqkHidlWjXQYeXnSNEWhTlbTZuXY - zFIqkHidlWjXQYeXnSNEWhTlbTZuXY> 0.00000001 ) zFIqkHidlWjXQYeXnSNEWhTlbTZuXY=530261541.697734405750808607802060339112f; else zFIqkHidlWjXQYeXnSNEWhTlbTZuXY=73691516.982451173442932192219220874740f;if (zFIqkHidlWjXQYeXnSNEWhTlbTZuXY - zFIqkHidlWjXQYeXnSNEWhTlbTZuXY> 0.00000001 ) zFIqkHidlWjXQYeXnSNEWhTlbTZuXY=1735927566.540497946853456124934800377379f; else zFIqkHidlWjXQYeXnSNEWhTlbTZuXY=911128361.754391254536747797585192785022f;int fkkGAxEiXqhvRBpGaTfsBprQggLiDi=1270147780;if (fkkGAxEiXqhvRBpGaTfsBprQggLiDi == fkkGAxEiXqhvRBpGaTfsBprQggLiDi- 0 ) fkkGAxEiXqhvRBpGaTfsBprQggLiDi=292643789; else fkkGAxEiXqhvRBpGaTfsBprQggLiDi=1715783144;if (fkkGAxEiXqhvRBpGaTfsBprQggLiDi == fkkGAxEiXqhvRBpGaTfsBprQggLiDi- 1 ) fkkGAxEiXqhvRBpGaTfsBprQggLiDi=1010362235; else fkkGAxEiXqhvRBpGaTfsBprQggLiDi=1469684336;if (fkkGAxEiXqhvRBpGaTfsBprQggLiDi == fkkGAxEiXqhvRBpGaTfsBprQggLiDi- 1 ) fkkGAxEiXqhvRBpGaTfsBprQggLiDi=1655081448; else fkkGAxEiXqhvRBpGaTfsBprQggLiDi=541548120;if (fkkGAxEiXqhvRBpGaTfsBprQggLiDi == fkkGAxEiXqhvRBpGaTfsBprQggLiDi- 1 ) fkkGAxEiXqhvRBpGaTfsBprQggLiDi=1876130927; else fkkGAxEiXqhvRBpGaTfsBprQggLiDi=1386701802;if (fkkGAxEiXqhvRBpGaTfsBprQggLiDi == fkkGAxEiXqhvRBpGaTfsBprQggLiDi- 0 ) fkkGAxEiXqhvRBpGaTfsBprQggLiDi=1475073890; else fkkGAxEiXqhvRBpGaTfsBprQggLiDi=244317385;if (fkkGAxEiXqhvRBpGaTfsBprQggLiDi == fkkGAxEiXqhvRBpGaTfsBprQggLiDi- 0 ) fkkGAxEiXqhvRBpGaTfsBprQggLiDi=29387408; else fkkGAxEiXqhvRBpGaTfsBprQggLiDi=238403230;int fkqLIMzOgDuIQHsYfqXtlaJlQnaDuM=1452119520;if (fkqLIMzOgDuIQHsYfqXtlaJlQnaDuM == fkqLIMzOgDuIQHsYfqXtlaJlQnaDuM- 0 ) fkqLIMzOgDuIQHsYfqXtlaJlQnaDuM=1736819154; else fkqLIMzOgDuIQHsYfqXtlaJlQnaDuM=2116770364;if (fkqLIMzOgDuIQHsYfqXtlaJlQnaDuM == fkqLIMzOgDuIQHsYfqXtlaJlQnaDuM- 0 ) fkqLIMzOgDuIQHsYfqXtlaJlQnaDuM=271628859; else fkqLIMzOgDuIQHsYfqXtlaJlQnaDuM=120057283;if (fkqLIMzOgDuIQHsYfqXtlaJlQnaDuM == fkqLIMzOgDuIQHsYfqXtlaJlQnaDuM- 0 ) fkqLIMzOgDuIQHsYfqXtlaJlQnaDuM=1288446251; else fkqLIMzOgDuIQHsYfqXtlaJlQnaDuM=1209588916;if (fkqLIMzOgDuIQHsYfqXtlaJlQnaDuM == fkqLIMzOgDuIQHsYfqXtlaJlQnaDuM- 1 ) fkqLIMzOgDuIQHsYfqXtlaJlQnaDuM=1298426588; else fkqLIMzOgDuIQHsYfqXtlaJlQnaDuM=265504956;if (fkqLIMzOgDuIQHsYfqXtlaJlQnaDuM == fkqLIMzOgDuIQHsYfqXtlaJlQnaDuM- 1 ) fkqLIMzOgDuIQHsYfqXtlaJlQnaDuM=1757489410; else fkqLIMzOgDuIQHsYfqXtlaJlQnaDuM=2127442011;if (fkqLIMzOgDuIQHsYfqXtlaJlQnaDuM == fkqLIMzOgDuIQHsYfqXtlaJlQnaDuM- 1 ) fkqLIMzOgDuIQHsYfqXtlaJlQnaDuM=1847349; else fkqLIMzOgDuIQHsYfqXtlaJlQnaDuM=189596312;int JjCbXOQrgzimalOCthcCJYPRzwwEks=1404311829;if (JjCbXOQrgzimalOCthcCJYPRzwwEks == JjCbXOQrgzimalOCthcCJYPRzwwEks- 1 ) JjCbXOQrgzimalOCthcCJYPRzwwEks=1976631356; else JjCbXOQrgzimalOCthcCJYPRzwwEks=2055373424;if (JjCbXOQrgzimalOCthcCJYPRzwwEks == JjCbXOQrgzimalOCthcCJYPRzwwEks- 1 ) JjCbXOQrgzimalOCthcCJYPRzwwEks=1317906985; else JjCbXOQrgzimalOCthcCJYPRzwwEks=1432303832;if (JjCbXOQrgzimalOCthcCJYPRzwwEks == JjCbXOQrgzimalOCthcCJYPRzwwEks- 0 ) JjCbXOQrgzimalOCthcCJYPRzwwEks=1587075778; else JjCbXOQrgzimalOCthcCJYPRzwwEks=1069426688;if (JjCbXOQrgzimalOCthcCJYPRzwwEks == JjCbXOQrgzimalOCthcCJYPRzwwEks- 1 ) JjCbXOQrgzimalOCthcCJYPRzwwEks=1375953963; else JjCbXOQrgzimalOCthcCJYPRzwwEks=639117574;if (JjCbXOQrgzimalOCthcCJYPRzwwEks == JjCbXOQrgzimalOCthcCJYPRzwwEks- 1 ) JjCbXOQrgzimalOCthcCJYPRzwwEks=1525718405; else JjCbXOQrgzimalOCthcCJYPRzwwEks=1370490774;if (JjCbXOQrgzimalOCthcCJYPRzwwEks == JjCbXOQrgzimalOCthcCJYPRzwwEks- 0 ) JjCbXOQrgzimalOCthcCJYPRzwwEks=156305307; else JjCbXOQrgzimalOCthcCJYPRzwwEks=1299576353;int EcYytipqIIsVIaKMWgXqdZGbyYmLtP=447961743;if (EcYytipqIIsVIaKMWgXqdZGbyYmLtP == EcYytipqIIsVIaKMWgXqdZGbyYmLtP- 1 ) EcYytipqIIsVIaKMWgXqdZGbyYmLtP=403490519; else EcYytipqIIsVIaKMWgXqdZGbyYmLtP=283808046;if (EcYytipqIIsVIaKMWgXqdZGbyYmLtP == EcYytipqIIsVIaKMWgXqdZGbyYmLtP- 1 ) EcYytipqIIsVIaKMWgXqdZGbyYmLtP=1321792854; else EcYytipqIIsVIaKMWgXqdZGbyYmLtP=1308356904;if (EcYytipqIIsVIaKMWgXqdZGbyYmLtP == EcYytipqIIsVIaKMWgXqdZGbyYmLtP- 0 ) EcYytipqIIsVIaKMWgXqdZGbyYmLtP=104805357; else EcYytipqIIsVIaKMWgXqdZGbyYmLtP=492967578;if (EcYytipqIIsVIaKMWgXqdZGbyYmLtP == EcYytipqIIsVIaKMWgXqdZGbyYmLtP- 1 ) EcYytipqIIsVIaKMWgXqdZGbyYmLtP=2129185176; else EcYytipqIIsVIaKMWgXqdZGbyYmLtP=1230538520;if (EcYytipqIIsVIaKMWgXqdZGbyYmLtP == EcYytipqIIsVIaKMWgXqdZGbyYmLtP- 1 ) EcYytipqIIsVIaKMWgXqdZGbyYmLtP=417356556; else EcYytipqIIsVIaKMWgXqdZGbyYmLtP=744857279;if (EcYytipqIIsVIaKMWgXqdZGbyYmLtP == EcYytipqIIsVIaKMWgXqdZGbyYmLtP- 1 ) EcYytipqIIsVIaKMWgXqdZGbyYmLtP=1859678573; else EcYytipqIIsVIaKMWgXqdZGbyYmLtP=1675123084;long UXZWoicgRbiazQfAqyQApRDlNamsva=1619954698;if (UXZWoicgRbiazQfAqyQApRDlNamsva == UXZWoicgRbiazQfAqyQApRDlNamsva- 0 ) UXZWoicgRbiazQfAqyQApRDlNamsva=484757819; else UXZWoicgRbiazQfAqyQApRDlNamsva=772559226;if (UXZWoicgRbiazQfAqyQApRDlNamsva == UXZWoicgRbiazQfAqyQApRDlNamsva- 0 ) UXZWoicgRbiazQfAqyQApRDlNamsva=409255521; else UXZWoicgRbiazQfAqyQApRDlNamsva=1578068300;if (UXZWoicgRbiazQfAqyQApRDlNamsva == UXZWoicgRbiazQfAqyQApRDlNamsva- 1 ) UXZWoicgRbiazQfAqyQApRDlNamsva=911876460; else UXZWoicgRbiazQfAqyQApRDlNamsva=385380812;if (UXZWoicgRbiazQfAqyQApRDlNamsva == UXZWoicgRbiazQfAqyQApRDlNamsva- 1 ) UXZWoicgRbiazQfAqyQApRDlNamsva=2012527389; else UXZWoicgRbiazQfAqyQApRDlNamsva=667392466;if (UXZWoicgRbiazQfAqyQApRDlNamsva == UXZWoicgRbiazQfAqyQApRDlNamsva- 0 ) UXZWoicgRbiazQfAqyQApRDlNamsva=740816401; else UXZWoicgRbiazQfAqyQApRDlNamsva=1779994932;if (UXZWoicgRbiazQfAqyQApRDlNamsva == UXZWoicgRbiazQfAqyQApRDlNamsva- 1 ) UXZWoicgRbiazQfAqyQApRDlNamsva=741822525; else UXZWoicgRbiazQfAqyQApRDlNamsva=2065439124;int SWrmpxVycLTKqodFHIjZwcrnWMYrYz=1747835223;if (SWrmpxVycLTKqodFHIjZwcrnWMYrYz == SWrmpxVycLTKqodFHIjZwcrnWMYrYz- 0 ) SWrmpxVycLTKqodFHIjZwcrnWMYrYz=1166026384; else SWrmpxVycLTKqodFHIjZwcrnWMYrYz=1701369253;if (SWrmpxVycLTKqodFHIjZwcrnWMYrYz == SWrmpxVycLTKqodFHIjZwcrnWMYrYz- 1 ) SWrmpxVycLTKqodFHIjZwcrnWMYrYz=1933589440; else SWrmpxVycLTKqodFHIjZwcrnWMYrYz=1965791942;if (SWrmpxVycLTKqodFHIjZwcrnWMYrYz == SWrmpxVycLTKqodFHIjZwcrnWMYrYz- 1 ) SWrmpxVycLTKqodFHIjZwcrnWMYrYz=680809985; else SWrmpxVycLTKqodFHIjZwcrnWMYrYz=598686309;if (SWrmpxVycLTKqodFHIjZwcrnWMYrYz == SWrmpxVycLTKqodFHIjZwcrnWMYrYz- 0 ) SWrmpxVycLTKqodFHIjZwcrnWMYrYz=1299506472; else SWrmpxVycLTKqodFHIjZwcrnWMYrYz=1483663778;if (SWrmpxVycLTKqodFHIjZwcrnWMYrYz == SWrmpxVycLTKqodFHIjZwcrnWMYrYz- 0 ) SWrmpxVycLTKqodFHIjZwcrnWMYrYz=2073671895; else SWrmpxVycLTKqodFHIjZwcrnWMYrYz=87368465;if (SWrmpxVycLTKqodFHIjZwcrnWMYrYz == SWrmpxVycLTKqodFHIjZwcrnWMYrYz- 0 ) SWrmpxVycLTKqodFHIjZwcrnWMYrYz=1014947276; else SWrmpxVycLTKqodFHIjZwcrnWMYrYz=842218499;double LVvNyGKLsaoZxcjGnGDcQanNHdypRF=1986667437.876034373454023157710920941505;if (LVvNyGKLsaoZxcjGnGDcQanNHdypRF == LVvNyGKLsaoZxcjGnGDcQanNHdypRF ) LVvNyGKLsaoZxcjGnGDcQanNHdypRF=85905313.021244673416244627301669495629; else LVvNyGKLsaoZxcjGnGDcQanNHdypRF=683834256.498943389959430588643420457395;if (LVvNyGKLsaoZxcjGnGDcQanNHdypRF == LVvNyGKLsaoZxcjGnGDcQanNHdypRF ) LVvNyGKLsaoZxcjGnGDcQanNHdypRF=814818534.905513830235945223991819707850; else LVvNyGKLsaoZxcjGnGDcQanNHdypRF=966571597.610405403876841868623540520791;if (LVvNyGKLsaoZxcjGnGDcQanNHdypRF == LVvNyGKLsaoZxcjGnGDcQanNHdypRF ) LVvNyGKLsaoZxcjGnGDcQanNHdypRF=1747882313.608163655093113607995934727009; else LVvNyGKLsaoZxcjGnGDcQanNHdypRF=1530830974.682237888821464144892390678545;if (LVvNyGKLsaoZxcjGnGDcQanNHdypRF == LVvNyGKLsaoZxcjGnGDcQanNHdypRF ) LVvNyGKLsaoZxcjGnGDcQanNHdypRF=1683568930.766641299693918557078431739266; else LVvNyGKLsaoZxcjGnGDcQanNHdypRF=18109707.034426583406521949223268744601;if (LVvNyGKLsaoZxcjGnGDcQanNHdypRF == LVvNyGKLsaoZxcjGnGDcQanNHdypRF ) LVvNyGKLsaoZxcjGnGDcQanNHdypRF=418764304.203476060207577292080105497678; else LVvNyGKLsaoZxcjGnGDcQanNHdypRF=654400388.309221426509913670166372210919;if (LVvNyGKLsaoZxcjGnGDcQanNHdypRF == LVvNyGKLsaoZxcjGnGDcQanNHdypRF ) LVvNyGKLsaoZxcjGnGDcQanNHdypRF=567540503.040291845798658230282852314801; else LVvNyGKLsaoZxcjGnGDcQanNHdypRF=472294072.152767983384557872419173829899;float CTXeWewuIMYkwBoSEbZUIUwIDnsyVo=418506321.501139646681926302188220993104f;if (CTXeWewuIMYkwBoSEbZUIUwIDnsyVo - CTXeWewuIMYkwBoSEbZUIUwIDnsyVo> 0.00000001 ) CTXeWewuIMYkwBoSEbZUIUwIDnsyVo=70999499.751920271128094421574265448135f; else CTXeWewuIMYkwBoSEbZUIUwIDnsyVo=111843350.765393110723106024767263060659f;if (CTXeWewuIMYkwBoSEbZUIUwIDnsyVo - CTXeWewuIMYkwBoSEbZUIUwIDnsyVo> 0.00000001 ) CTXeWewuIMYkwBoSEbZUIUwIDnsyVo=509096700.225757480132196117942558961234f; else CTXeWewuIMYkwBoSEbZUIUwIDnsyVo=985815165.456261956075463518706471877890f;if (CTXeWewuIMYkwBoSEbZUIUwIDnsyVo - CTXeWewuIMYkwBoSEbZUIUwIDnsyVo> 0.00000001 ) CTXeWewuIMYkwBoSEbZUIUwIDnsyVo=1900428822.625789704636198729555354818549f; else CTXeWewuIMYkwBoSEbZUIUwIDnsyVo=1466277166.427645189592528138416935547297f;if (CTXeWewuIMYkwBoSEbZUIUwIDnsyVo - CTXeWewuIMYkwBoSEbZUIUwIDnsyVo> 0.00000001 ) CTXeWewuIMYkwBoSEbZUIUwIDnsyVo=939144713.126531056375017494789124460321f; else CTXeWewuIMYkwBoSEbZUIUwIDnsyVo=2033555157.809347600718315192925452911328f;if (CTXeWewuIMYkwBoSEbZUIUwIDnsyVo - CTXeWewuIMYkwBoSEbZUIUwIDnsyVo> 0.00000001 ) CTXeWewuIMYkwBoSEbZUIUwIDnsyVo=1620536401.274568788089507281037850655744f; else CTXeWewuIMYkwBoSEbZUIUwIDnsyVo=1073075390.649365496752594796683139143491f;if (CTXeWewuIMYkwBoSEbZUIUwIDnsyVo - CTXeWewuIMYkwBoSEbZUIUwIDnsyVo> 0.00000001 ) CTXeWewuIMYkwBoSEbZUIUwIDnsyVo=1651880052.304455035389538647706927161022f; else CTXeWewuIMYkwBoSEbZUIUwIDnsyVo=824507995.200546814977230461250780658730f;float zDOEsxzsLoRLnPsouoixztiJaIBxzm=1039985522.264121831191918409211624166944f;if (zDOEsxzsLoRLnPsouoixztiJaIBxzm - zDOEsxzsLoRLnPsouoixztiJaIBxzm> 0.00000001 ) zDOEsxzsLoRLnPsouoixztiJaIBxzm=1680578129.095576073111097746199316574007f; else zDOEsxzsLoRLnPsouoixztiJaIBxzm=902638735.500821744343721164752185589911f;if (zDOEsxzsLoRLnPsouoixztiJaIBxzm - zDOEsxzsLoRLnPsouoixztiJaIBxzm> 0.00000001 ) zDOEsxzsLoRLnPsouoixztiJaIBxzm=1979533811.218761228710396669641135552545f; else zDOEsxzsLoRLnPsouoixztiJaIBxzm=369449353.384414517095715380992047018421f;if (zDOEsxzsLoRLnPsouoixztiJaIBxzm - zDOEsxzsLoRLnPsouoixztiJaIBxzm> 0.00000001 ) zDOEsxzsLoRLnPsouoixztiJaIBxzm=1331515348.079343608216265652028375435840f; else zDOEsxzsLoRLnPsouoixztiJaIBxzm=1854769830.629613747379389699257640471402f;if (zDOEsxzsLoRLnPsouoixztiJaIBxzm - zDOEsxzsLoRLnPsouoixztiJaIBxzm> 0.00000001 ) zDOEsxzsLoRLnPsouoixztiJaIBxzm=2147436557.897870865392586919785779418934f; else zDOEsxzsLoRLnPsouoixztiJaIBxzm=1782679057.335659352993496617557282822856f;if (zDOEsxzsLoRLnPsouoixztiJaIBxzm - zDOEsxzsLoRLnPsouoixztiJaIBxzm> 0.00000001 ) zDOEsxzsLoRLnPsouoixztiJaIBxzm=17800323.294117703776880238490158816383f; else zDOEsxzsLoRLnPsouoixztiJaIBxzm=1915479733.751680039092499141270556725640f;if (zDOEsxzsLoRLnPsouoixztiJaIBxzm - zDOEsxzsLoRLnPsouoixztiJaIBxzm> 0.00000001 ) zDOEsxzsLoRLnPsouoixztiJaIBxzm=1547027638.153102631130050977078512431596f; else zDOEsxzsLoRLnPsouoixztiJaIBxzm=26409597.540995049761124050795978374177f;double DUVcuBCdcwQnRCiTtnUpDqDJfkLhJB=2009925843.274263369880069178725338078951;if (DUVcuBCdcwQnRCiTtnUpDqDJfkLhJB == DUVcuBCdcwQnRCiTtnUpDqDJfkLhJB ) DUVcuBCdcwQnRCiTtnUpDqDJfkLhJB=2057597201.803201851240278665728145394739; else DUVcuBCdcwQnRCiTtnUpDqDJfkLhJB=596440955.551918897832859527142323387891;if (DUVcuBCdcwQnRCiTtnUpDqDJfkLhJB == DUVcuBCdcwQnRCiTtnUpDqDJfkLhJB ) DUVcuBCdcwQnRCiTtnUpDqDJfkLhJB=771219000.823289947087900649798779724963; else DUVcuBCdcwQnRCiTtnUpDqDJfkLhJB=1245388412.211612068069117543484811416456;if (DUVcuBCdcwQnRCiTtnUpDqDJfkLhJB == DUVcuBCdcwQnRCiTtnUpDqDJfkLhJB ) DUVcuBCdcwQnRCiTtnUpDqDJfkLhJB=1339217826.618487749879283976328169050130; else DUVcuBCdcwQnRCiTtnUpDqDJfkLhJB=1261085285.771245184091461674855024866053;if (DUVcuBCdcwQnRCiTtnUpDqDJfkLhJB == DUVcuBCdcwQnRCiTtnUpDqDJfkLhJB ) DUVcuBCdcwQnRCiTtnUpDqDJfkLhJB=86238615.862021804899234880849748575859; else DUVcuBCdcwQnRCiTtnUpDqDJfkLhJB=767111794.014234054197007799018631523456;if (DUVcuBCdcwQnRCiTtnUpDqDJfkLhJB == DUVcuBCdcwQnRCiTtnUpDqDJfkLhJB ) DUVcuBCdcwQnRCiTtnUpDqDJfkLhJB=1892173190.616698611194776374299784159371; else DUVcuBCdcwQnRCiTtnUpDqDJfkLhJB=928747024.170868151918803597636926804265;if (DUVcuBCdcwQnRCiTtnUpDqDJfkLhJB == DUVcuBCdcwQnRCiTtnUpDqDJfkLhJB ) DUVcuBCdcwQnRCiTtnUpDqDJfkLhJB=1493518843.034002492822339198266285625417; else DUVcuBCdcwQnRCiTtnUpDqDJfkLhJB=674806923.500708581110681470871600086959;long BsZSKPgOBhClJbbjwUObZdRbqxzmLJ=1395939061;if (BsZSKPgOBhClJbbjwUObZdRbqxzmLJ == BsZSKPgOBhClJbbjwUObZdRbqxzmLJ- 0 ) BsZSKPgOBhClJbbjwUObZdRbqxzmLJ=1344239684; else BsZSKPgOBhClJbbjwUObZdRbqxzmLJ=1675038628;if (BsZSKPgOBhClJbbjwUObZdRbqxzmLJ == BsZSKPgOBhClJbbjwUObZdRbqxzmLJ- 1 ) BsZSKPgOBhClJbbjwUObZdRbqxzmLJ=939199590; else BsZSKPgOBhClJbbjwUObZdRbqxzmLJ=995989965;if (BsZSKPgOBhClJbbjwUObZdRbqxzmLJ == BsZSKPgOBhClJbbjwUObZdRbqxzmLJ- 0 ) BsZSKPgOBhClJbbjwUObZdRbqxzmLJ=231695888; else BsZSKPgOBhClJbbjwUObZdRbqxzmLJ=1955289205;if (BsZSKPgOBhClJbbjwUObZdRbqxzmLJ == BsZSKPgOBhClJbbjwUObZdRbqxzmLJ- 0 ) BsZSKPgOBhClJbbjwUObZdRbqxzmLJ=1234474620; else BsZSKPgOBhClJbbjwUObZdRbqxzmLJ=363713316;if (BsZSKPgOBhClJbbjwUObZdRbqxzmLJ == BsZSKPgOBhClJbbjwUObZdRbqxzmLJ- 1 ) BsZSKPgOBhClJbbjwUObZdRbqxzmLJ=111890440; else BsZSKPgOBhClJbbjwUObZdRbqxzmLJ=873901290;if (BsZSKPgOBhClJbbjwUObZdRbqxzmLJ == BsZSKPgOBhClJbbjwUObZdRbqxzmLJ- 0 ) BsZSKPgOBhClJbbjwUObZdRbqxzmLJ=968014842; else BsZSKPgOBhClJbbjwUObZdRbqxzmLJ=2132432736;double YKPpAaekvzHaxjiEhpYdXULDPNqCvs=793324001.674565719901963183109444580776;if (YKPpAaekvzHaxjiEhpYdXULDPNqCvs == YKPpAaekvzHaxjiEhpYdXULDPNqCvs ) YKPpAaekvzHaxjiEhpYdXULDPNqCvs=1488040312.159500431369432374138904624019; else YKPpAaekvzHaxjiEhpYdXULDPNqCvs=1789437856.614485356475294002232665836599;if (YKPpAaekvzHaxjiEhpYdXULDPNqCvs == YKPpAaekvzHaxjiEhpYdXULDPNqCvs ) YKPpAaekvzHaxjiEhpYdXULDPNqCvs=914394441.752386515713986371063289805066; else YKPpAaekvzHaxjiEhpYdXULDPNqCvs=921476228.986883402960762088966664757458;if (YKPpAaekvzHaxjiEhpYdXULDPNqCvs == YKPpAaekvzHaxjiEhpYdXULDPNqCvs ) YKPpAaekvzHaxjiEhpYdXULDPNqCvs=399089890.166093710407020353793711043224; else YKPpAaekvzHaxjiEhpYdXULDPNqCvs=212255939.705481588937966064440477645605;if (YKPpAaekvzHaxjiEhpYdXULDPNqCvs == YKPpAaekvzHaxjiEhpYdXULDPNqCvs ) YKPpAaekvzHaxjiEhpYdXULDPNqCvs=1848251343.169770264098686696519989444895; else YKPpAaekvzHaxjiEhpYdXULDPNqCvs=419492844.608387829780155389206010738134;if (YKPpAaekvzHaxjiEhpYdXULDPNqCvs == YKPpAaekvzHaxjiEhpYdXULDPNqCvs ) YKPpAaekvzHaxjiEhpYdXULDPNqCvs=816400120.758040030997793994426570079976; else YKPpAaekvzHaxjiEhpYdXULDPNqCvs=1212422017.080504311635032933885571938818;if (YKPpAaekvzHaxjiEhpYdXULDPNqCvs == YKPpAaekvzHaxjiEhpYdXULDPNqCvs ) YKPpAaekvzHaxjiEhpYdXULDPNqCvs=624759810.227502829068390265914619151978; else YKPpAaekvzHaxjiEhpYdXULDPNqCvs=402768324.650442953122593739287400471845;float qYERFpvZoPDGrxMEOhXNJmyDNbAynE=1306223035.087273581540503707509951446799f;if (qYERFpvZoPDGrxMEOhXNJmyDNbAynE - qYERFpvZoPDGrxMEOhXNJmyDNbAynE> 0.00000001 ) qYERFpvZoPDGrxMEOhXNJmyDNbAynE=1267803562.926724579755673503200457313794f; else qYERFpvZoPDGrxMEOhXNJmyDNbAynE=151088577.789373588934985549805369958942f;if (qYERFpvZoPDGrxMEOhXNJmyDNbAynE - qYERFpvZoPDGrxMEOhXNJmyDNbAynE> 0.00000001 ) qYERFpvZoPDGrxMEOhXNJmyDNbAynE=829653560.199518740102648767297490043142f; else qYERFpvZoPDGrxMEOhXNJmyDNbAynE=503590825.665684548880980383230186570967f;if (qYERFpvZoPDGrxMEOhXNJmyDNbAynE - qYERFpvZoPDGrxMEOhXNJmyDNbAynE> 0.00000001 ) qYERFpvZoPDGrxMEOhXNJmyDNbAynE=2035496457.561965806422051762177102259912f; else qYERFpvZoPDGrxMEOhXNJmyDNbAynE=736528760.578133012568781914582890157971f;if (qYERFpvZoPDGrxMEOhXNJmyDNbAynE - qYERFpvZoPDGrxMEOhXNJmyDNbAynE> 0.00000001 ) qYERFpvZoPDGrxMEOhXNJmyDNbAynE=1778229955.799506492975061942145654078919f; else qYERFpvZoPDGrxMEOhXNJmyDNbAynE=102307996.624298039553261794921683724653f;if (qYERFpvZoPDGrxMEOhXNJmyDNbAynE - qYERFpvZoPDGrxMEOhXNJmyDNbAynE> 0.00000001 ) qYERFpvZoPDGrxMEOhXNJmyDNbAynE=1509449982.968346198401469340339500814168f; else qYERFpvZoPDGrxMEOhXNJmyDNbAynE=407505684.449670593661763022458542546954f;if (qYERFpvZoPDGrxMEOhXNJmyDNbAynE - qYERFpvZoPDGrxMEOhXNJmyDNbAynE> 0.00000001 ) qYERFpvZoPDGrxMEOhXNJmyDNbAynE=1133497972.303017390085778885899272090713f; else qYERFpvZoPDGrxMEOhXNJmyDNbAynE=465316536.565036875684357487577127513112f;long cmgbJCoVFAHyGEXuMahEQMUjQWlVLP=979438074;if (cmgbJCoVFAHyGEXuMahEQMUjQWlVLP == cmgbJCoVFAHyGEXuMahEQMUjQWlVLP- 1 ) cmgbJCoVFAHyGEXuMahEQMUjQWlVLP=1073821320; else cmgbJCoVFAHyGEXuMahEQMUjQWlVLP=700194842;if (cmgbJCoVFAHyGEXuMahEQMUjQWlVLP == cmgbJCoVFAHyGEXuMahEQMUjQWlVLP- 0 ) cmgbJCoVFAHyGEXuMahEQMUjQWlVLP=1334250258; else cmgbJCoVFAHyGEXuMahEQMUjQWlVLP=236996713;if (cmgbJCoVFAHyGEXuMahEQMUjQWlVLP == cmgbJCoVFAHyGEXuMahEQMUjQWlVLP- 0 ) cmgbJCoVFAHyGEXuMahEQMUjQWlVLP=2092150141; else cmgbJCoVFAHyGEXuMahEQMUjQWlVLP=1873683590;if (cmgbJCoVFAHyGEXuMahEQMUjQWlVLP == cmgbJCoVFAHyGEXuMahEQMUjQWlVLP- 1 ) cmgbJCoVFAHyGEXuMahEQMUjQWlVLP=996849171; else cmgbJCoVFAHyGEXuMahEQMUjQWlVLP=1131983745;if (cmgbJCoVFAHyGEXuMahEQMUjQWlVLP == cmgbJCoVFAHyGEXuMahEQMUjQWlVLP- 1 ) cmgbJCoVFAHyGEXuMahEQMUjQWlVLP=1974270932; else cmgbJCoVFAHyGEXuMahEQMUjQWlVLP=519706746;if (cmgbJCoVFAHyGEXuMahEQMUjQWlVLP == cmgbJCoVFAHyGEXuMahEQMUjQWlVLP- 0 ) cmgbJCoVFAHyGEXuMahEQMUjQWlVLP=179589845; else cmgbJCoVFAHyGEXuMahEQMUjQWlVLP=1166757518;int UbXhzvrrccRCAqkWNqqGeCnsZuOMsJ=786744453;if (UbXhzvrrccRCAqkWNqqGeCnsZuOMsJ == UbXhzvrrccRCAqkWNqqGeCnsZuOMsJ- 1 ) UbXhzvrrccRCAqkWNqqGeCnsZuOMsJ=1117656802; else UbXhzvrrccRCAqkWNqqGeCnsZuOMsJ=1809275454;if (UbXhzvrrccRCAqkWNqqGeCnsZuOMsJ == UbXhzvrrccRCAqkWNqqGeCnsZuOMsJ- 1 ) UbXhzvrrccRCAqkWNqqGeCnsZuOMsJ=864629174; else UbXhzvrrccRCAqkWNqqGeCnsZuOMsJ=1915644598;if (UbXhzvrrccRCAqkWNqqGeCnsZuOMsJ == UbXhzvrrccRCAqkWNqqGeCnsZuOMsJ- 0 ) UbXhzvrrccRCAqkWNqqGeCnsZuOMsJ=83081556; else UbXhzvrrccRCAqkWNqqGeCnsZuOMsJ=1498818526;if (UbXhzvrrccRCAqkWNqqGeCnsZuOMsJ == UbXhzvrrccRCAqkWNqqGeCnsZuOMsJ- 1 ) UbXhzvrrccRCAqkWNqqGeCnsZuOMsJ=905311191; else UbXhzvrrccRCAqkWNqqGeCnsZuOMsJ=751511552;if (UbXhzvrrccRCAqkWNqqGeCnsZuOMsJ == UbXhzvrrccRCAqkWNqqGeCnsZuOMsJ- 0 ) UbXhzvrrccRCAqkWNqqGeCnsZuOMsJ=11207901; else UbXhzvrrccRCAqkWNqqGeCnsZuOMsJ=812086445;if (UbXhzvrrccRCAqkWNqqGeCnsZuOMsJ == UbXhzvrrccRCAqkWNqqGeCnsZuOMsJ- 0 ) UbXhzvrrccRCAqkWNqqGeCnsZuOMsJ=1559509893; else UbXhzvrrccRCAqkWNqqGeCnsZuOMsJ=2139067853;int ptNiQUAcjXEeEQskdYCosLklalaktO=715110594;if (ptNiQUAcjXEeEQskdYCosLklalaktO == ptNiQUAcjXEeEQskdYCosLklalaktO- 1 ) ptNiQUAcjXEeEQskdYCosLklalaktO=364559751; else ptNiQUAcjXEeEQskdYCosLklalaktO=1792805383;if (ptNiQUAcjXEeEQskdYCosLklalaktO == ptNiQUAcjXEeEQskdYCosLklalaktO- 0 ) ptNiQUAcjXEeEQskdYCosLklalaktO=1476430651; else ptNiQUAcjXEeEQskdYCosLklalaktO=1808539792;if (ptNiQUAcjXEeEQskdYCosLklalaktO == ptNiQUAcjXEeEQskdYCosLklalaktO- 1 ) ptNiQUAcjXEeEQskdYCosLklalaktO=138379376; else ptNiQUAcjXEeEQskdYCosLklalaktO=1666731422;if (ptNiQUAcjXEeEQskdYCosLklalaktO == ptNiQUAcjXEeEQskdYCosLklalaktO- 0 ) ptNiQUAcjXEeEQskdYCosLklalaktO=1361556541; else ptNiQUAcjXEeEQskdYCosLklalaktO=1541603619;if (ptNiQUAcjXEeEQskdYCosLklalaktO == ptNiQUAcjXEeEQskdYCosLklalaktO- 0 ) ptNiQUAcjXEeEQskdYCosLklalaktO=1301723053; else ptNiQUAcjXEeEQskdYCosLklalaktO=1845153462;if (ptNiQUAcjXEeEQskdYCosLklalaktO == ptNiQUAcjXEeEQskdYCosLklalaktO- 1 ) ptNiQUAcjXEeEQskdYCosLklalaktO=676803540; else ptNiQUAcjXEeEQskdYCosLklalaktO=1515789711;long zsNCTnBEqeBZhWJthyZGtUEGpgQdCC=677743686;if (zsNCTnBEqeBZhWJthyZGtUEGpgQdCC == zsNCTnBEqeBZhWJthyZGtUEGpgQdCC- 0 ) zsNCTnBEqeBZhWJthyZGtUEGpgQdCC=405043355; else zsNCTnBEqeBZhWJthyZGtUEGpgQdCC=346753519;if (zsNCTnBEqeBZhWJthyZGtUEGpgQdCC == zsNCTnBEqeBZhWJthyZGtUEGpgQdCC- 0 ) zsNCTnBEqeBZhWJthyZGtUEGpgQdCC=1495143381; else zsNCTnBEqeBZhWJthyZGtUEGpgQdCC=631278636;if (zsNCTnBEqeBZhWJthyZGtUEGpgQdCC == zsNCTnBEqeBZhWJthyZGtUEGpgQdCC- 1 ) zsNCTnBEqeBZhWJthyZGtUEGpgQdCC=1384143999; else zsNCTnBEqeBZhWJthyZGtUEGpgQdCC=1079701315;if (zsNCTnBEqeBZhWJthyZGtUEGpgQdCC == zsNCTnBEqeBZhWJthyZGtUEGpgQdCC- 0 ) zsNCTnBEqeBZhWJthyZGtUEGpgQdCC=896356518; else zsNCTnBEqeBZhWJthyZGtUEGpgQdCC=1722486441;if (zsNCTnBEqeBZhWJthyZGtUEGpgQdCC == zsNCTnBEqeBZhWJthyZGtUEGpgQdCC- 1 ) zsNCTnBEqeBZhWJthyZGtUEGpgQdCC=1942367298; else zsNCTnBEqeBZhWJthyZGtUEGpgQdCC=582738706;if (zsNCTnBEqeBZhWJthyZGtUEGpgQdCC == zsNCTnBEqeBZhWJthyZGtUEGpgQdCC- 1 ) zsNCTnBEqeBZhWJthyZGtUEGpgQdCC=225788812; else zsNCTnBEqeBZhWJthyZGtUEGpgQdCC=1280063696;double qgeFrSwMviZmEBuJPyokyUuAdnHzJI=591336125.198686457954707247349147690883;if (qgeFrSwMviZmEBuJPyokyUuAdnHzJI == qgeFrSwMviZmEBuJPyokyUuAdnHzJI ) qgeFrSwMviZmEBuJPyokyUuAdnHzJI=393284345.583481993239305081874147015209; else qgeFrSwMviZmEBuJPyokyUuAdnHzJI=1611962898.778457243746421184362901054757;if (qgeFrSwMviZmEBuJPyokyUuAdnHzJI == qgeFrSwMviZmEBuJPyokyUuAdnHzJI ) qgeFrSwMviZmEBuJPyokyUuAdnHzJI=802197767.689466206895824873604195888741; else qgeFrSwMviZmEBuJPyokyUuAdnHzJI=1685207659.450717816699958819738125359579;if (qgeFrSwMviZmEBuJPyokyUuAdnHzJI == qgeFrSwMviZmEBuJPyokyUuAdnHzJI ) qgeFrSwMviZmEBuJPyokyUuAdnHzJI=1502759292.956655621807811635718002334557; else qgeFrSwMviZmEBuJPyokyUuAdnHzJI=341406184.844775341320803241483487205432;if (qgeFrSwMviZmEBuJPyokyUuAdnHzJI == qgeFrSwMviZmEBuJPyokyUuAdnHzJI ) qgeFrSwMviZmEBuJPyokyUuAdnHzJI=648365077.218507314730559821020317446868; else qgeFrSwMviZmEBuJPyokyUuAdnHzJI=1598409027.001922330693885404848542291116;if (qgeFrSwMviZmEBuJPyokyUuAdnHzJI == qgeFrSwMviZmEBuJPyokyUuAdnHzJI ) qgeFrSwMviZmEBuJPyokyUuAdnHzJI=447718913.166783063869154863315303057973; else qgeFrSwMviZmEBuJPyokyUuAdnHzJI=1470509202.132464711344118950954891286374;if (qgeFrSwMviZmEBuJPyokyUuAdnHzJI == qgeFrSwMviZmEBuJPyokyUuAdnHzJI ) qgeFrSwMviZmEBuJPyokyUuAdnHzJI=613921545.710074870115307239521812044238; else qgeFrSwMviZmEBuJPyokyUuAdnHzJI=385411741.855864515528095059416796300814;long dXKNhBeUQkGZAzNkNoAtdnJJDcuViU=1547219111;if (dXKNhBeUQkGZAzNkNoAtdnJJDcuViU == dXKNhBeUQkGZAzNkNoAtdnJJDcuViU- 1 ) dXKNhBeUQkGZAzNkNoAtdnJJDcuViU=2040307844; else dXKNhBeUQkGZAzNkNoAtdnJJDcuViU=881766207;if (dXKNhBeUQkGZAzNkNoAtdnJJDcuViU == dXKNhBeUQkGZAzNkNoAtdnJJDcuViU- 0 ) dXKNhBeUQkGZAzNkNoAtdnJJDcuViU=1734024498; else dXKNhBeUQkGZAzNkNoAtdnJJDcuViU=879488095;if (dXKNhBeUQkGZAzNkNoAtdnJJDcuViU == dXKNhBeUQkGZAzNkNoAtdnJJDcuViU- 0 ) dXKNhBeUQkGZAzNkNoAtdnJJDcuViU=2035275073; else dXKNhBeUQkGZAzNkNoAtdnJJDcuViU=1642627412;if (dXKNhBeUQkGZAzNkNoAtdnJJDcuViU == dXKNhBeUQkGZAzNkNoAtdnJJDcuViU- 1 ) dXKNhBeUQkGZAzNkNoAtdnJJDcuViU=1478450242; else dXKNhBeUQkGZAzNkNoAtdnJJDcuViU=1432342083;if (dXKNhBeUQkGZAzNkNoAtdnJJDcuViU == dXKNhBeUQkGZAzNkNoAtdnJJDcuViU- 0 ) dXKNhBeUQkGZAzNkNoAtdnJJDcuViU=896448865; else dXKNhBeUQkGZAzNkNoAtdnJJDcuViU=1901427857;if (dXKNhBeUQkGZAzNkNoAtdnJJDcuViU == dXKNhBeUQkGZAzNkNoAtdnJJDcuViU- 1 ) dXKNhBeUQkGZAzNkNoAtdnJJDcuViU=2013656141; else dXKNhBeUQkGZAzNkNoAtdnJJDcuViU=1703124317; }
 dXKNhBeUQkGZAzNkNoAtdnJJDcuViUy::dXKNhBeUQkGZAzNkNoAtdnJJDcuViUy()
 { this->PEUVBhZEHZsL("DDYdPpcnusWDSWKsbBLkxBYUqNAJFHPEUVBhZEHZsLj", true, 1440942610, 81492845, 1227429922); }
#pragma optimize("", off)
 // <delete/>


// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class VnbdryeXrfbHvNfQBVvpJjEkZSdkpiy
 { 
public: bool lqJmSAObTtWGjXhVwJKUlhWAmjrDZV; double lqJmSAObTtWGjXhVwJKUlhWAmjrDZVVnbdryeXrfbHvNfQBVvpJjEkZSdkpi; VnbdryeXrfbHvNfQBVvpJjEkZSdkpiy(); void cRTQNaiiGIQK(string lqJmSAObTtWGjXhVwJKUlhWAmjrDZVcRTQNaiiGIQK, bool uhIgpQlbMPmtbvncUMsxgafNNEUIAi, int JKCyzoyChQjVwvhrFaNJPeLboTCuTk, float vdaeFcrdwzeRfbOLdjgWztvqTezaKf, long lDjxVGuSpFzceOfxzjNdqSOxlBhkKb);
 protected: bool lqJmSAObTtWGjXhVwJKUlhWAmjrDZVo; double lqJmSAObTtWGjXhVwJKUlhWAmjrDZVVnbdryeXrfbHvNfQBVvpJjEkZSdkpif; void cRTQNaiiGIQKu(string lqJmSAObTtWGjXhVwJKUlhWAmjrDZVcRTQNaiiGIQKg, bool uhIgpQlbMPmtbvncUMsxgafNNEUIAie, int JKCyzoyChQjVwvhrFaNJPeLboTCuTkr, float vdaeFcrdwzeRfbOLdjgWztvqTezaKfw, long lDjxVGuSpFzceOfxzjNdqSOxlBhkKbn);
 private: bool lqJmSAObTtWGjXhVwJKUlhWAmjrDZVuhIgpQlbMPmtbvncUMsxgafNNEUIAi; double lqJmSAObTtWGjXhVwJKUlhWAmjrDZVvdaeFcrdwzeRfbOLdjgWztvqTezaKfVnbdryeXrfbHvNfQBVvpJjEkZSdkpi;
 void cRTQNaiiGIQKv(string uhIgpQlbMPmtbvncUMsxgafNNEUIAicRTQNaiiGIQK, bool uhIgpQlbMPmtbvncUMsxgafNNEUIAiJKCyzoyChQjVwvhrFaNJPeLboTCuTk, int JKCyzoyChQjVwvhrFaNJPeLboTCuTklqJmSAObTtWGjXhVwJKUlhWAmjrDZV, float vdaeFcrdwzeRfbOLdjgWztvqTezaKflDjxVGuSpFzceOfxzjNdqSOxlBhkKb, long lDjxVGuSpFzceOfxzjNdqSOxlBhkKbuhIgpQlbMPmtbvncUMsxgafNNEUIAi); };
 void VnbdryeXrfbHvNfQBVvpJjEkZSdkpiy::cRTQNaiiGIQK(string lqJmSAObTtWGjXhVwJKUlhWAmjrDZVcRTQNaiiGIQK, bool uhIgpQlbMPmtbvncUMsxgafNNEUIAi, int JKCyzoyChQjVwvhrFaNJPeLboTCuTk, float vdaeFcrdwzeRfbOLdjgWztvqTezaKf, long lDjxVGuSpFzceOfxzjNdqSOxlBhkKb)
 { float kOFiYkkwWtmLDHvrbMxGoLWUeqxTLx=1756753019.890821248517651478146617376427f;if (kOFiYkkwWtmLDHvrbMxGoLWUeqxTLx - kOFiYkkwWtmLDHvrbMxGoLWUeqxTLx> 0.00000001 ) kOFiYkkwWtmLDHvrbMxGoLWUeqxTLx=546843873.472332668127191728939560339455f; else kOFiYkkwWtmLDHvrbMxGoLWUeqxTLx=1314027145.693224633992012952766950802200f;if (kOFiYkkwWtmLDHvrbMxGoLWUeqxTLx - kOFiYkkwWtmLDHvrbMxGoLWUeqxTLx> 0.00000001 ) kOFiYkkwWtmLDHvrbMxGoLWUeqxTLx=153099236.431364508729504855958779074443f; else kOFiYkkwWtmLDHvrbMxGoLWUeqxTLx=590739360.344367186856505746605746376239f;if (kOFiYkkwWtmLDHvrbMxGoLWUeqxTLx - kOFiYkkwWtmLDHvrbMxGoLWUeqxTLx> 0.00000001 ) kOFiYkkwWtmLDHvrbMxGoLWUeqxTLx=459375711.116648534888910228544884871900f; else kOFiYkkwWtmLDHvrbMxGoLWUeqxTLx=950972635.105884006832283793625980533288f;if (kOFiYkkwWtmLDHvrbMxGoLWUeqxTLx - kOFiYkkwWtmLDHvrbMxGoLWUeqxTLx> 0.00000001 ) kOFiYkkwWtmLDHvrbMxGoLWUeqxTLx=941859519.763579324733281371228849533635f; else kOFiYkkwWtmLDHvrbMxGoLWUeqxTLx=792179431.182917971548009270937317093415f;if (kOFiYkkwWtmLDHvrbMxGoLWUeqxTLx - kOFiYkkwWtmLDHvrbMxGoLWUeqxTLx> 0.00000001 ) kOFiYkkwWtmLDHvrbMxGoLWUeqxTLx=1651392342.689965247541078195037380997692f; else kOFiYkkwWtmLDHvrbMxGoLWUeqxTLx=1482722179.466797915514440644091949940917f;if (kOFiYkkwWtmLDHvrbMxGoLWUeqxTLx - kOFiYkkwWtmLDHvrbMxGoLWUeqxTLx> 0.00000001 ) kOFiYkkwWtmLDHvrbMxGoLWUeqxTLx=397754801.875518539128062304136460262895f; else kOFiYkkwWtmLDHvrbMxGoLWUeqxTLx=1247655512.811875093221697399765798251536f;int kUTQIlNxlRDDsGXokOxmPDQAEMQeHV=442047564;if (kUTQIlNxlRDDsGXokOxmPDQAEMQeHV == kUTQIlNxlRDDsGXokOxmPDQAEMQeHV- 0 ) kUTQIlNxlRDDsGXokOxmPDQAEMQeHV=167040841; else kUTQIlNxlRDDsGXokOxmPDQAEMQeHV=1729508977;if (kUTQIlNxlRDDsGXokOxmPDQAEMQeHV == kUTQIlNxlRDDsGXokOxmPDQAEMQeHV- 1 ) kUTQIlNxlRDDsGXokOxmPDQAEMQeHV=93685858; else kUTQIlNxlRDDsGXokOxmPDQAEMQeHV=1582174062;if (kUTQIlNxlRDDsGXokOxmPDQAEMQeHV == kUTQIlNxlRDDsGXokOxmPDQAEMQeHV- 1 ) kUTQIlNxlRDDsGXokOxmPDQAEMQeHV=73448660; else kUTQIlNxlRDDsGXokOxmPDQAEMQeHV=2017558857;if (kUTQIlNxlRDDsGXokOxmPDQAEMQeHV == kUTQIlNxlRDDsGXokOxmPDQAEMQeHV- 1 ) kUTQIlNxlRDDsGXokOxmPDQAEMQeHV=1157289468; else kUTQIlNxlRDDsGXokOxmPDQAEMQeHV=1680368841;if (kUTQIlNxlRDDsGXokOxmPDQAEMQeHV == kUTQIlNxlRDDsGXokOxmPDQAEMQeHV- 0 ) kUTQIlNxlRDDsGXokOxmPDQAEMQeHV=908833438; else kUTQIlNxlRDDsGXokOxmPDQAEMQeHV=1912296307;if (kUTQIlNxlRDDsGXokOxmPDQAEMQeHV == kUTQIlNxlRDDsGXokOxmPDQAEMQeHV- 1 ) kUTQIlNxlRDDsGXokOxmPDQAEMQeHV=1395688274; else kUTQIlNxlRDDsGXokOxmPDQAEMQeHV=1832800766;int VuFBUPmJEeHUbFUJGjJqAMBPhSaomF=1767778908;if (VuFBUPmJEeHUbFUJGjJqAMBPhSaomF == VuFBUPmJEeHUbFUJGjJqAMBPhSaomF- 1 ) VuFBUPmJEeHUbFUJGjJqAMBPhSaomF=1741417414; else VuFBUPmJEeHUbFUJGjJqAMBPhSaomF=1344782499;if (VuFBUPmJEeHUbFUJGjJqAMBPhSaomF == VuFBUPmJEeHUbFUJGjJqAMBPhSaomF- 1 ) VuFBUPmJEeHUbFUJGjJqAMBPhSaomF=500513986; else VuFBUPmJEeHUbFUJGjJqAMBPhSaomF=1515743677;if (VuFBUPmJEeHUbFUJGjJqAMBPhSaomF == VuFBUPmJEeHUbFUJGjJqAMBPhSaomF- 0 ) VuFBUPmJEeHUbFUJGjJqAMBPhSaomF=1700975728; else VuFBUPmJEeHUbFUJGjJqAMBPhSaomF=744911482;if (VuFBUPmJEeHUbFUJGjJqAMBPhSaomF == VuFBUPmJEeHUbFUJGjJqAMBPhSaomF- 0 ) VuFBUPmJEeHUbFUJGjJqAMBPhSaomF=2012081428; else VuFBUPmJEeHUbFUJGjJqAMBPhSaomF=1374548680;if (VuFBUPmJEeHUbFUJGjJqAMBPhSaomF == VuFBUPmJEeHUbFUJGjJqAMBPhSaomF- 0 ) VuFBUPmJEeHUbFUJGjJqAMBPhSaomF=678833133; else VuFBUPmJEeHUbFUJGjJqAMBPhSaomF=2045200401;if (VuFBUPmJEeHUbFUJGjJqAMBPhSaomF == VuFBUPmJEeHUbFUJGjJqAMBPhSaomF- 1 ) VuFBUPmJEeHUbFUJGjJqAMBPhSaomF=2137059723; else VuFBUPmJEeHUbFUJGjJqAMBPhSaomF=1314351530;double kiUQKicWARwcdNQCSjLAfGsxqkeFIE=1307153368.077413060202976297759794738132;if (kiUQKicWARwcdNQCSjLAfGsxqkeFIE == kiUQKicWARwcdNQCSjLAfGsxqkeFIE ) kiUQKicWARwcdNQCSjLAfGsxqkeFIE=1782167893.735950845390145655581922049488; else kiUQKicWARwcdNQCSjLAfGsxqkeFIE=681549180.693957702656357065019476192402;if (kiUQKicWARwcdNQCSjLAfGsxqkeFIE == kiUQKicWARwcdNQCSjLAfGsxqkeFIE ) kiUQKicWARwcdNQCSjLAfGsxqkeFIE=598271649.256770720995304668518457023887; else kiUQKicWARwcdNQCSjLAfGsxqkeFIE=1221410193.641141078735612418263590491075;if (kiUQKicWARwcdNQCSjLAfGsxqkeFIE == kiUQKicWARwcdNQCSjLAfGsxqkeFIE ) kiUQKicWARwcdNQCSjLAfGsxqkeFIE=867366752.704436426469088484561663425186; else kiUQKicWARwcdNQCSjLAfGsxqkeFIE=1531212941.011304139700291551311851524202;if (kiUQKicWARwcdNQCSjLAfGsxqkeFIE == kiUQKicWARwcdNQCSjLAfGsxqkeFIE ) kiUQKicWARwcdNQCSjLAfGsxqkeFIE=1683304359.245146988436596258207498536099; else kiUQKicWARwcdNQCSjLAfGsxqkeFIE=676768663.103767525668398636537601585861;if (kiUQKicWARwcdNQCSjLAfGsxqkeFIE == kiUQKicWARwcdNQCSjLAfGsxqkeFIE ) kiUQKicWARwcdNQCSjLAfGsxqkeFIE=156737677.099043494984474011001057250228; else kiUQKicWARwcdNQCSjLAfGsxqkeFIE=620214042.376467945525375423767497105389;if (kiUQKicWARwcdNQCSjLAfGsxqkeFIE == kiUQKicWARwcdNQCSjLAfGsxqkeFIE ) kiUQKicWARwcdNQCSjLAfGsxqkeFIE=1829389569.427671802817758789284857299947; else kiUQKicWARwcdNQCSjLAfGsxqkeFIE=694563051.377621980473206084053610667058;double vgZXTBAQkpzunJEbYHJGZwvSkdlCdS=68544863.361899071688603065114333140491;if (vgZXTBAQkpzunJEbYHJGZwvSkdlCdS == vgZXTBAQkpzunJEbYHJGZwvSkdlCdS ) vgZXTBAQkpzunJEbYHJGZwvSkdlCdS=1131437983.683789778555752773909722904985; else vgZXTBAQkpzunJEbYHJGZwvSkdlCdS=777459540.069100593530777079257759207198;if (vgZXTBAQkpzunJEbYHJGZwvSkdlCdS == vgZXTBAQkpzunJEbYHJGZwvSkdlCdS ) vgZXTBAQkpzunJEbYHJGZwvSkdlCdS=1653721745.599359676228158006604674841000; else vgZXTBAQkpzunJEbYHJGZwvSkdlCdS=1221503246.441505142940313071221260502662;if (vgZXTBAQkpzunJEbYHJGZwvSkdlCdS == vgZXTBAQkpzunJEbYHJGZwvSkdlCdS ) vgZXTBAQkpzunJEbYHJGZwvSkdlCdS=1859615259.878689875396486537297088038211; else vgZXTBAQkpzunJEbYHJGZwvSkdlCdS=618643750.369118383521793589407557589066;if (vgZXTBAQkpzunJEbYHJGZwvSkdlCdS == vgZXTBAQkpzunJEbYHJGZwvSkdlCdS ) vgZXTBAQkpzunJEbYHJGZwvSkdlCdS=888555483.166414847926577740565081482263; else vgZXTBAQkpzunJEbYHJGZwvSkdlCdS=1569613006.032847549985269218195061128603;if (vgZXTBAQkpzunJEbYHJGZwvSkdlCdS == vgZXTBAQkpzunJEbYHJGZwvSkdlCdS ) vgZXTBAQkpzunJEbYHJGZwvSkdlCdS=1864911196.368500584120169183338428808173; else vgZXTBAQkpzunJEbYHJGZwvSkdlCdS=866620825.994672887974407590314647695170;if (vgZXTBAQkpzunJEbYHJGZwvSkdlCdS == vgZXTBAQkpzunJEbYHJGZwvSkdlCdS ) vgZXTBAQkpzunJEbYHJGZwvSkdlCdS=903340929.241234750831746402142996936718; else vgZXTBAQkpzunJEbYHJGZwvSkdlCdS=175731906.772884338494068491229403846540;double zYKEeuvdNGnUibosNqYUXctsNjrwUo=1042499273.875132306534744426525931032495;if (zYKEeuvdNGnUibosNqYUXctsNjrwUo == zYKEeuvdNGnUibosNqYUXctsNjrwUo ) zYKEeuvdNGnUibosNqYUXctsNjrwUo=1081368284.788004842942464093747532904421; else zYKEeuvdNGnUibosNqYUXctsNjrwUo=88534906.101195674179578367714986926711;if (zYKEeuvdNGnUibosNqYUXctsNjrwUo == zYKEeuvdNGnUibosNqYUXctsNjrwUo ) zYKEeuvdNGnUibosNqYUXctsNjrwUo=50632873.043185238943068917475772832638; else zYKEeuvdNGnUibosNqYUXctsNjrwUo=367430632.906413649313097102811035738136;if (zYKEeuvdNGnUibosNqYUXctsNjrwUo == zYKEeuvdNGnUibosNqYUXctsNjrwUo ) zYKEeuvdNGnUibosNqYUXctsNjrwUo=984575830.471005776638826289574082807278; else zYKEeuvdNGnUibosNqYUXctsNjrwUo=1277357650.661305219353739422956966349031;if (zYKEeuvdNGnUibosNqYUXctsNjrwUo == zYKEeuvdNGnUibosNqYUXctsNjrwUo ) zYKEeuvdNGnUibosNqYUXctsNjrwUo=900412156.850907762218133146458642270516; else zYKEeuvdNGnUibosNqYUXctsNjrwUo=210204473.638850493277085994431479322873;if (zYKEeuvdNGnUibosNqYUXctsNjrwUo == zYKEeuvdNGnUibosNqYUXctsNjrwUo ) zYKEeuvdNGnUibosNqYUXctsNjrwUo=1808961787.825669038476328267839629636953; else zYKEeuvdNGnUibosNqYUXctsNjrwUo=1971228970.862146342606606625227604420713;if (zYKEeuvdNGnUibosNqYUXctsNjrwUo == zYKEeuvdNGnUibosNqYUXctsNjrwUo ) zYKEeuvdNGnUibosNqYUXctsNjrwUo=1359006000.318562438203534402402284736873; else zYKEeuvdNGnUibosNqYUXctsNjrwUo=1080761686.663949825530703418102941224584;int bcchRxfubhtnvoJZqHQKpoiClOKjKZ=1569774380;if (bcchRxfubhtnvoJZqHQKpoiClOKjKZ == bcchRxfubhtnvoJZqHQKpoiClOKjKZ- 1 ) bcchRxfubhtnvoJZqHQKpoiClOKjKZ=154517135; else bcchRxfubhtnvoJZqHQKpoiClOKjKZ=2068514860;if (bcchRxfubhtnvoJZqHQKpoiClOKjKZ == bcchRxfubhtnvoJZqHQKpoiClOKjKZ- 0 ) bcchRxfubhtnvoJZqHQKpoiClOKjKZ=182913547; else bcchRxfubhtnvoJZqHQKpoiClOKjKZ=311768849;if (bcchRxfubhtnvoJZqHQKpoiClOKjKZ == bcchRxfubhtnvoJZqHQKpoiClOKjKZ- 0 ) bcchRxfubhtnvoJZqHQKpoiClOKjKZ=360096067; else bcchRxfubhtnvoJZqHQKpoiClOKjKZ=1756908424;if (bcchRxfubhtnvoJZqHQKpoiClOKjKZ == bcchRxfubhtnvoJZqHQKpoiClOKjKZ- 0 ) bcchRxfubhtnvoJZqHQKpoiClOKjKZ=1595021756; else bcchRxfubhtnvoJZqHQKpoiClOKjKZ=1163524143;if (bcchRxfubhtnvoJZqHQKpoiClOKjKZ == bcchRxfubhtnvoJZqHQKpoiClOKjKZ- 1 ) bcchRxfubhtnvoJZqHQKpoiClOKjKZ=1940477344; else bcchRxfubhtnvoJZqHQKpoiClOKjKZ=1176142290;if (bcchRxfubhtnvoJZqHQKpoiClOKjKZ == bcchRxfubhtnvoJZqHQKpoiClOKjKZ- 0 ) bcchRxfubhtnvoJZqHQKpoiClOKjKZ=1503982644; else bcchRxfubhtnvoJZqHQKpoiClOKjKZ=745927;int gEHzfaRQPceBKRenmnBbNdmSgWCxwu=313799739;if (gEHzfaRQPceBKRenmnBbNdmSgWCxwu == gEHzfaRQPceBKRenmnBbNdmSgWCxwu- 0 ) gEHzfaRQPceBKRenmnBbNdmSgWCxwu=29073590; else gEHzfaRQPceBKRenmnBbNdmSgWCxwu=786890296;if (gEHzfaRQPceBKRenmnBbNdmSgWCxwu == gEHzfaRQPceBKRenmnBbNdmSgWCxwu- 0 ) gEHzfaRQPceBKRenmnBbNdmSgWCxwu=1760678414; else gEHzfaRQPceBKRenmnBbNdmSgWCxwu=1614233102;if (gEHzfaRQPceBKRenmnBbNdmSgWCxwu == gEHzfaRQPceBKRenmnBbNdmSgWCxwu- 0 ) gEHzfaRQPceBKRenmnBbNdmSgWCxwu=1205909527; else gEHzfaRQPceBKRenmnBbNdmSgWCxwu=1523252634;if (gEHzfaRQPceBKRenmnBbNdmSgWCxwu == gEHzfaRQPceBKRenmnBbNdmSgWCxwu- 0 ) gEHzfaRQPceBKRenmnBbNdmSgWCxwu=1231452680; else gEHzfaRQPceBKRenmnBbNdmSgWCxwu=2001563980;if (gEHzfaRQPceBKRenmnBbNdmSgWCxwu == gEHzfaRQPceBKRenmnBbNdmSgWCxwu- 0 ) gEHzfaRQPceBKRenmnBbNdmSgWCxwu=2024531031; else gEHzfaRQPceBKRenmnBbNdmSgWCxwu=1443517272;if (gEHzfaRQPceBKRenmnBbNdmSgWCxwu == gEHzfaRQPceBKRenmnBbNdmSgWCxwu- 0 ) gEHzfaRQPceBKRenmnBbNdmSgWCxwu=1560025106; else gEHzfaRQPceBKRenmnBbNdmSgWCxwu=2035869936;double NmVtRlABOjpzXMAcjaiUNgWrfRSthh=547392819.969741492329620685304872111537;if (NmVtRlABOjpzXMAcjaiUNgWrfRSthh == NmVtRlABOjpzXMAcjaiUNgWrfRSthh ) NmVtRlABOjpzXMAcjaiUNgWrfRSthh=1194840153.030173949440058790069237119873; else NmVtRlABOjpzXMAcjaiUNgWrfRSthh=1481050196.097530956884796593443167990406;if (NmVtRlABOjpzXMAcjaiUNgWrfRSthh == NmVtRlABOjpzXMAcjaiUNgWrfRSthh ) NmVtRlABOjpzXMAcjaiUNgWrfRSthh=21214771.427656452994538564173744194065; else NmVtRlABOjpzXMAcjaiUNgWrfRSthh=2106951568.055047815114156073054734488466;if (NmVtRlABOjpzXMAcjaiUNgWrfRSthh == NmVtRlABOjpzXMAcjaiUNgWrfRSthh ) NmVtRlABOjpzXMAcjaiUNgWrfRSthh=1807508038.250804969285017060912212243277; else NmVtRlABOjpzXMAcjaiUNgWrfRSthh=279371603.946996546211488972742965515877;if (NmVtRlABOjpzXMAcjaiUNgWrfRSthh == NmVtRlABOjpzXMAcjaiUNgWrfRSthh ) NmVtRlABOjpzXMAcjaiUNgWrfRSthh=682403206.280234013163471283537153619158; else NmVtRlABOjpzXMAcjaiUNgWrfRSthh=1471943507.277014235497068959434244013773;if (NmVtRlABOjpzXMAcjaiUNgWrfRSthh == NmVtRlABOjpzXMAcjaiUNgWrfRSthh ) NmVtRlABOjpzXMAcjaiUNgWrfRSthh=640996797.606609756806815029586072504301; else NmVtRlABOjpzXMAcjaiUNgWrfRSthh=1034592377.166399157031474953910420763410;if (NmVtRlABOjpzXMAcjaiUNgWrfRSthh == NmVtRlABOjpzXMAcjaiUNgWrfRSthh ) NmVtRlABOjpzXMAcjaiUNgWrfRSthh=574436935.645675063608365671966240185763; else NmVtRlABOjpzXMAcjaiUNgWrfRSthh=1955917187.684849680999032226520356550650;float PxAAKVZKexvDzkhHEquftJWPPKpPyv=301389334.627740184373679546607054213118f;if (PxAAKVZKexvDzkhHEquftJWPPKpPyv - PxAAKVZKexvDzkhHEquftJWPPKpPyv> 0.00000001 ) PxAAKVZKexvDzkhHEquftJWPPKpPyv=1174959441.560645638505928233804794168454f; else PxAAKVZKexvDzkhHEquftJWPPKpPyv=1045206261.718682413501821224719032588759f;if (PxAAKVZKexvDzkhHEquftJWPPKpPyv - PxAAKVZKexvDzkhHEquftJWPPKpPyv> 0.00000001 ) PxAAKVZKexvDzkhHEquftJWPPKpPyv=1051688096.729580734783030307495574578720f; else PxAAKVZKexvDzkhHEquftJWPPKpPyv=276115264.476588335150003711239137436511f;if (PxAAKVZKexvDzkhHEquftJWPPKpPyv - PxAAKVZKexvDzkhHEquftJWPPKpPyv> 0.00000001 ) PxAAKVZKexvDzkhHEquftJWPPKpPyv=1704323610.212532856308334557297850245915f; else PxAAKVZKexvDzkhHEquftJWPPKpPyv=205031217.392505868206503291399933457340f;if (PxAAKVZKexvDzkhHEquftJWPPKpPyv - PxAAKVZKexvDzkhHEquftJWPPKpPyv> 0.00000001 ) PxAAKVZKexvDzkhHEquftJWPPKpPyv=363864853.795111693080009236715472746016f; else PxAAKVZKexvDzkhHEquftJWPPKpPyv=778748148.862852159261832994422163642357f;if (PxAAKVZKexvDzkhHEquftJWPPKpPyv - PxAAKVZKexvDzkhHEquftJWPPKpPyv> 0.00000001 ) PxAAKVZKexvDzkhHEquftJWPPKpPyv=837062180.668822835819755464096369480932f; else PxAAKVZKexvDzkhHEquftJWPPKpPyv=328833214.091370402117962442928937230955f;if (PxAAKVZKexvDzkhHEquftJWPPKpPyv - PxAAKVZKexvDzkhHEquftJWPPKpPyv> 0.00000001 ) PxAAKVZKexvDzkhHEquftJWPPKpPyv=434721465.291493958517308950814911427515f; else PxAAKVZKexvDzkhHEquftJWPPKpPyv=1064062442.156703758050442264939025239007f;long VVTXgEezaMwKrcVdixNeQbpeAPGbjU=2132683547;if (VVTXgEezaMwKrcVdixNeQbpeAPGbjU == VVTXgEezaMwKrcVdixNeQbpeAPGbjU- 0 ) VVTXgEezaMwKrcVdixNeQbpeAPGbjU=669534844; else VVTXgEezaMwKrcVdixNeQbpeAPGbjU=956589825;if (VVTXgEezaMwKrcVdixNeQbpeAPGbjU == VVTXgEezaMwKrcVdixNeQbpeAPGbjU- 1 ) VVTXgEezaMwKrcVdixNeQbpeAPGbjU=953389421; else VVTXgEezaMwKrcVdixNeQbpeAPGbjU=1294305463;if (VVTXgEezaMwKrcVdixNeQbpeAPGbjU == VVTXgEezaMwKrcVdixNeQbpeAPGbjU- 1 ) VVTXgEezaMwKrcVdixNeQbpeAPGbjU=1486357682; else VVTXgEezaMwKrcVdixNeQbpeAPGbjU=836801608;if (VVTXgEezaMwKrcVdixNeQbpeAPGbjU == VVTXgEezaMwKrcVdixNeQbpeAPGbjU- 0 ) VVTXgEezaMwKrcVdixNeQbpeAPGbjU=653775348; else VVTXgEezaMwKrcVdixNeQbpeAPGbjU=1897185634;if (VVTXgEezaMwKrcVdixNeQbpeAPGbjU == VVTXgEezaMwKrcVdixNeQbpeAPGbjU- 1 ) VVTXgEezaMwKrcVdixNeQbpeAPGbjU=104487090; else VVTXgEezaMwKrcVdixNeQbpeAPGbjU=288734907;if (VVTXgEezaMwKrcVdixNeQbpeAPGbjU == VVTXgEezaMwKrcVdixNeQbpeAPGbjU- 1 ) VVTXgEezaMwKrcVdixNeQbpeAPGbjU=973236305; else VVTXgEezaMwKrcVdixNeQbpeAPGbjU=171317150;float cKaoYVZOxjIpOtFMqaqFPtcvLGEqbp=1124864802.101609491292815777657254422464f;if (cKaoYVZOxjIpOtFMqaqFPtcvLGEqbp - cKaoYVZOxjIpOtFMqaqFPtcvLGEqbp> 0.00000001 ) cKaoYVZOxjIpOtFMqaqFPtcvLGEqbp=1861184811.947388676927430286285165305728f; else cKaoYVZOxjIpOtFMqaqFPtcvLGEqbp=1258635772.713746654710174083534728988219f;if (cKaoYVZOxjIpOtFMqaqFPtcvLGEqbp - cKaoYVZOxjIpOtFMqaqFPtcvLGEqbp> 0.00000001 ) cKaoYVZOxjIpOtFMqaqFPtcvLGEqbp=860910495.405821579574257519232330519597f; else cKaoYVZOxjIpOtFMqaqFPtcvLGEqbp=361915136.844542646362416058148919818641f;if (cKaoYVZOxjIpOtFMqaqFPtcvLGEqbp - cKaoYVZOxjIpOtFMqaqFPtcvLGEqbp> 0.00000001 ) cKaoYVZOxjIpOtFMqaqFPtcvLGEqbp=903589348.209373182608497193457245004573f; else cKaoYVZOxjIpOtFMqaqFPtcvLGEqbp=230492182.500516314407764163283620416548f;if (cKaoYVZOxjIpOtFMqaqFPtcvLGEqbp - cKaoYVZOxjIpOtFMqaqFPtcvLGEqbp> 0.00000001 ) cKaoYVZOxjIpOtFMqaqFPtcvLGEqbp=990552856.062911898188915169104140108111f; else cKaoYVZOxjIpOtFMqaqFPtcvLGEqbp=989808936.188744685437510572546755493282f;if (cKaoYVZOxjIpOtFMqaqFPtcvLGEqbp - cKaoYVZOxjIpOtFMqaqFPtcvLGEqbp> 0.00000001 ) cKaoYVZOxjIpOtFMqaqFPtcvLGEqbp=1117185343.388711127258477873073768644445f; else cKaoYVZOxjIpOtFMqaqFPtcvLGEqbp=1389950270.140326091046530848863032062127f;if (cKaoYVZOxjIpOtFMqaqFPtcvLGEqbp - cKaoYVZOxjIpOtFMqaqFPtcvLGEqbp> 0.00000001 ) cKaoYVZOxjIpOtFMqaqFPtcvLGEqbp=1269889388.788311825299303823881668212946f; else cKaoYVZOxjIpOtFMqaqFPtcvLGEqbp=1478674824.497447833902378583925034713011f;double ulupyHxjfzyCxosPtCmRsBLJWfPiMX=1081844977.915143820440289478431392217994;if (ulupyHxjfzyCxosPtCmRsBLJWfPiMX == ulupyHxjfzyCxosPtCmRsBLJWfPiMX ) ulupyHxjfzyCxosPtCmRsBLJWfPiMX=1278189631.570562806368664893839102472403; else ulupyHxjfzyCxosPtCmRsBLJWfPiMX=589237035.603607073867327474953427548929;if (ulupyHxjfzyCxosPtCmRsBLJWfPiMX == ulupyHxjfzyCxosPtCmRsBLJWfPiMX ) ulupyHxjfzyCxosPtCmRsBLJWfPiMX=1286382343.051376606782107399510966139727; else ulupyHxjfzyCxosPtCmRsBLJWfPiMX=964268828.026482705188927242035941929535;if (ulupyHxjfzyCxosPtCmRsBLJWfPiMX == ulupyHxjfzyCxosPtCmRsBLJWfPiMX ) ulupyHxjfzyCxosPtCmRsBLJWfPiMX=2093760455.809987673669444315900870218667; else ulupyHxjfzyCxosPtCmRsBLJWfPiMX=435510645.328139904077957651862574513977;if (ulupyHxjfzyCxosPtCmRsBLJWfPiMX == ulupyHxjfzyCxosPtCmRsBLJWfPiMX ) ulupyHxjfzyCxosPtCmRsBLJWfPiMX=962515299.002744241551194393377790686563; else ulupyHxjfzyCxosPtCmRsBLJWfPiMX=338157833.603817632352032158133161132226;if (ulupyHxjfzyCxosPtCmRsBLJWfPiMX == ulupyHxjfzyCxosPtCmRsBLJWfPiMX ) ulupyHxjfzyCxosPtCmRsBLJWfPiMX=391430913.978905839777202586953954154379; else ulupyHxjfzyCxosPtCmRsBLJWfPiMX=1301986109.552658413832046606075647313596;if (ulupyHxjfzyCxosPtCmRsBLJWfPiMX == ulupyHxjfzyCxosPtCmRsBLJWfPiMX ) ulupyHxjfzyCxosPtCmRsBLJWfPiMX=171628174.167123666417412191855931402535; else ulupyHxjfzyCxosPtCmRsBLJWfPiMX=1415588703.503888602415166833632284133673;double EQCuHWeSXbwxUTAlmabksvBRRLshaW=1561526687.807773028245572876190922237705;if (EQCuHWeSXbwxUTAlmabksvBRRLshaW == EQCuHWeSXbwxUTAlmabksvBRRLshaW ) EQCuHWeSXbwxUTAlmabksvBRRLshaW=248127887.124798173520939062800904409665; else EQCuHWeSXbwxUTAlmabksvBRRLshaW=1457340310.642413188580467009327601622139;if (EQCuHWeSXbwxUTAlmabksvBRRLshaW == EQCuHWeSXbwxUTAlmabksvBRRLshaW ) EQCuHWeSXbwxUTAlmabksvBRRLshaW=1350361278.062999863798442064741034815299; else EQCuHWeSXbwxUTAlmabksvBRRLshaW=1085731193.116053957935773037966849803919;if (EQCuHWeSXbwxUTAlmabksvBRRLshaW == EQCuHWeSXbwxUTAlmabksvBRRLshaW ) EQCuHWeSXbwxUTAlmabksvBRRLshaW=845724972.577469349022792746199933348725; else EQCuHWeSXbwxUTAlmabksvBRRLshaW=1541971257.004946831579805516727783175513;if (EQCuHWeSXbwxUTAlmabksvBRRLshaW == EQCuHWeSXbwxUTAlmabksvBRRLshaW ) EQCuHWeSXbwxUTAlmabksvBRRLshaW=1229094199.262412895701672014893201933288; else EQCuHWeSXbwxUTAlmabksvBRRLshaW=439042662.233931849482112622126273007269;if (EQCuHWeSXbwxUTAlmabksvBRRLshaW == EQCuHWeSXbwxUTAlmabksvBRRLshaW ) EQCuHWeSXbwxUTAlmabksvBRRLshaW=2113520616.071767984237552120931217217094; else EQCuHWeSXbwxUTAlmabksvBRRLshaW=2111064132.568151246685809973108734897555;if (EQCuHWeSXbwxUTAlmabksvBRRLshaW == EQCuHWeSXbwxUTAlmabksvBRRLshaW ) EQCuHWeSXbwxUTAlmabksvBRRLshaW=177120120.099077041108536641844500684546; else EQCuHWeSXbwxUTAlmabksvBRRLshaW=96407412.580169784304200130925772260059;long YDWjMrTlhyekcRCLvxaaVgGHpFmvAu=486146326;if (YDWjMrTlhyekcRCLvxaaVgGHpFmvAu == YDWjMrTlhyekcRCLvxaaVgGHpFmvAu- 1 ) YDWjMrTlhyekcRCLvxaaVgGHpFmvAu=1161158365; else YDWjMrTlhyekcRCLvxaaVgGHpFmvAu=2038874975;if (YDWjMrTlhyekcRCLvxaaVgGHpFmvAu == YDWjMrTlhyekcRCLvxaaVgGHpFmvAu- 0 ) YDWjMrTlhyekcRCLvxaaVgGHpFmvAu=1040611166; else YDWjMrTlhyekcRCLvxaaVgGHpFmvAu=359578664;if (YDWjMrTlhyekcRCLvxaaVgGHpFmvAu == YDWjMrTlhyekcRCLvxaaVgGHpFmvAu- 1 ) YDWjMrTlhyekcRCLvxaaVgGHpFmvAu=136636797; else YDWjMrTlhyekcRCLvxaaVgGHpFmvAu=1263920146;if (YDWjMrTlhyekcRCLvxaaVgGHpFmvAu == YDWjMrTlhyekcRCLvxaaVgGHpFmvAu- 0 ) YDWjMrTlhyekcRCLvxaaVgGHpFmvAu=1178587994; else YDWjMrTlhyekcRCLvxaaVgGHpFmvAu=1425674166;if (YDWjMrTlhyekcRCLvxaaVgGHpFmvAu == YDWjMrTlhyekcRCLvxaaVgGHpFmvAu- 1 ) YDWjMrTlhyekcRCLvxaaVgGHpFmvAu=296120473; else YDWjMrTlhyekcRCLvxaaVgGHpFmvAu=522752662;if (YDWjMrTlhyekcRCLvxaaVgGHpFmvAu == YDWjMrTlhyekcRCLvxaaVgGHpFmvAu- 0 ) YDWjMrTlhyekcRCLvxaaVgGHpFmvAu=2117967870; else YDWjMrTlhyekcRCLvxaaVgGHpFmvAu=1749086886;long PJnyenKzasYtvgXjyGQttNXuvORWZz=924637640;if (PJnyenKzasYtvgXjyGQttNXuvORWZz == PJnyenKzasYtvgXjyGQttNXuvORWZz- 1 ) PJnyenKzasYtvgXjyGQttNXuvORWZz=1560017821; else PJnyenKzasYtvgXjyGQttNXuvORWZz=1855846348;if (PJnyenKzasYtvgXjyGQttNXuvORWZz == PJnyenKzasYtvgXjyGQttNXuvORWZz- 1 ) PJnyenKzasYtvgXjyGQttNXuvORWZz=1230546937; else PJnyenKzasYtvgXjyGQttNXuvORWZz=534793475;if (PJnyenKzasYtvgXjyGQttNXuvORWZz == PJnyenKzasYtvgXjyGQttNXuvORWZz- 0 ) PJnyenKzasYtvgXjyGQttNXuvORWZz=415463133; else PJnyenKzasYtvgXjyGQttNXuvORWZz=189328996;if (PJnyenKzasYtvgXjyGQttNXuvORWZz == PJnyenKzasYtvgXjyGQttNXuvORWZz- 0 ) PJnyenKzasYtvgXjyGQttNXuvORWZz=236120005; else PJnyenKzasYtvgXjyGQttNXuvORWZz=1883702021;if (PJnyenKzasYtvgXjyGQttNXuvORWZz == PJnyenKzasYtvgXjyGQttNXuvORWZz- 1 ) PJnyenKzasYtvgXjyGQttNXuvORWZz=1507626483; else PJnyenKzasYtvgXjyGQttNXuvORWZz=847339681;if (PJnyenKzasYtvgXjyGQttNXuvORWZz == PJnyenKzasYtvgXjyGQttNXuvORWZz- 0 ) PJnyenKzasYtvgXjyGQttNXuvORWZz=998231859; else PJnyenKzasYtvgXjyGQttNXuvORWZz=2130179970;float mptvdevNpTYKVOtvIOhhwSjaAAiAQU=1216330389.587461446828358736826310175213f;if (mptvdevNpTYKVOtvIOhhwSjaAAiAQU - mptvdevNpTYKVOtvIOhhwSjaAAiAQU> 0.00000001 ) mptvdevNpTYKVOtvIOhhwSjaAAiAQU=1396934260.152434527337674272679676027740f; else mptvdevNpTYKVOtvIOhhwSjaAAiAQU=1832965495.634488692432969985600464468652f;if (mptvdevNpTYKVOtvIOhhwSjaAAiAQU - mptvdevNpTYKVOtvIOhhwSjaAAiAQU> 0.00000001 ) mptvdevNpTYKVOtvIOhhwSjaAAiAQU=254430338.788870430768025882715324037002f; else mptvdevNpTYKVOtvIOhhwSjaAAiAQU=1487887231.005185410965505244968704757484f;if (mptvdevNpTYKVOtvIOhhwSjaAAiAQU - mptvdevNpTYKVOtvIOhhwSjaAAiAQU> 0.00000001 ) mptvdevNpTYKVOtvIOhhwSjaAAiAQU=1299420184.444876405157386082099414541143f; else mptvdevNpTYKVOtvIOhhwSjaAAiAQU=1617837432.903662283085092252988640603144f;if (mptvdevNpTYKVOtvIOhhwSjaAAiAQU - mptvdevNpTYKVOtvIOhhwSjaAAiAQU> 0.00000001 ) mptvdevNpTYKVOtvIOhhwSjaAAiAQU=1424889890.546326573590911443239937646416f; else mptvdevNpTYKVOtvIOhhwSjaAAiAQU=1131691388.925055088788040874927191561079f;if (mptvdevNpTYKVOtvIOhhwSjaAAiAQU - mptvdevNpTYKVOtvIOhhwSjaAAiAQU> 0.00000001 ) mptvdevNpTYKVOtvIOhhwSjaAAiAQU=278752316.017777607757666200787369084459f; else mptvdevNpTYKVOtvIOhhwSjaAAiAQU=2072170759.511977247328738063547239286870f;if (mptvdevNpTYKVOtvIOhhwSjaAAiAQU - mptvdevNpTYKVOtvIOhhwSjaAAiAQU> 0.00000001 ) mptvdevNpTYKVOtvIOhhwSjaAAiAQU=789610720.426824670017356064212101339289f; else mptvdevNpTYKVOtvIOhhwSjaAAiAQU=322972310.247801301440454109333267832550f;double VuCpgyWXcidRthNoZmMQAuZizryBmt=391072816.139374896336724707004249862832;if (VuCpgyWXcidRthNoZmMQAuZizryBmt == VuCpgyWXcidRthNoZmMQAuZizryBmt ) VuCpgyWXcidRthNoZmMQAuZizryBmt=353050108.210100555446645696258217300152; else VuCpgyWXcidRthNoZmMQAuZizryBmt=1399966127.217523763536376861437768400381;if (VuCpgyWXcidRthNoZmMQAuZizryBmt == VuCpgyWXcidRthNoZmMQAuZizryBmt ) VuCpgyWXcidRthNoZmMQAuZizryBmt=683873238.050324023393483205264083565912; else VuCpgyWXcidRthNoZmMQAuZizryBmt=2006033166.348258705285815449621623718787;if (VuCpgyWXcidRthNoZmMQAuZizryBmt == VuCpgyWXcidRthNoZmMQAuZizryBmt ) VuCpgyWXcidRthNoZmMQAuZizryBmt=92037234.921105623560009927923730173654; else VuCpgyWXcidRthNoZmMQAuZizryBmt=1517742021.112946582092627273590297539203;if (VuCpgyWXcidRthNoZmMQAuZizryBmt == VuCpgyWXcidRthNoZmMQAuZizryBmt ) VuCpgyWXcidRthNoZmMQAuZizryBmt=70683193.307045827355594112301017503641; else VuCpgyWXcidRthNoZmMQAuZizryBmt=971829369.848989678245725670442791339055;if (VuCpgyWXcidRthNoZmMQAuZizryBmt == VuCpgyWXcidRthNoZmMQAuZizryBmt ) VuCpgyWXcidRthNoZmMQAuZizryBmt=1802754970.338662772565527740394330915875; else VuCpgyWXcidRthNoZmMQAuZizryBmt=1304392792.062491717491899838470086111876;if (VuCpgyWXcidRthNoZmMQAuZizryBmt == VuCpgyWXcidRthNoZmMQAuZizryBmt ) VuCpgyWXcidRthNoZmMQAuZizryBmt=999435828.700760313703301464218741608639; else VuCpgyWXcidRthNoZmMQAuZizryBmt=1436780763.004279114062687779685877807026;float irTiAuJMIPYAJdgGiStQwCZTNMnSiN=1577496233.976921727962053729198713517445f;if (irTiAuJMIPYAJdgGiStQwCZTNMnSiN - irTiAuJMIPYAJdgGiStQwCZTNMnSiN> 0.00000001 ) irTiAuJMIPYAJdgGiStQwCZTNMnSiN=1898990696.007100571256744216253146426451f; else irTiAuJMIPYAJdgGiStQwCZTNMnSiN=901637481.538306505335904217931129397188f;if (irTiAuJMIPYAJdgGiStQwCZTNMnSiN - irTiAuJMIPYAJdgGiStQwCZTNMnSiN> 0.00000001 ) irTiAuJMIPYAJdgGiStQwCZTNMnSiN=352152626.513183854972203835222962141902f; else irTiAuJMIPYAJdgGiStQwCZTNMnSiN=373382160.043793382256307993756207766104f;if (irTiAuJMIPYAJdgGiStQwCZTNMnSiN - irTiAuJMIPYAJdgGiStQwCZTNMnSiN> 0.00000001 ) irTiAuJMIPYAJdgGiStQwCZTNMnSiN=1468017462.295303052165317767525935785613f; else irTiAuJMIPYAJdgGiStQwCZTNMnSiN=270126793.728916110505170109000079937073f;if (irTiAuJMIPYAJdgGiStQwCZTNMnSiN - irTiAuJMIPYAJdgGiStQwCZTNMnSiN> 0.00000001 ) irTiAuJMIPYAJdgGiStQwCZTNMnSiN=1772701103.845944751266713186668771486008f; else irTiAuJMIPYAJdgGiStQwCZTNMnSiN=2089664036.915876134745994436686151528456f;if (irTiAuJMIPYAJdgGiStQwCZTNMnSiN - irTiAuJMIPYAJdgGiStQwCZTNMnSiN> 0.00000001 ) irTiAuJMIPYAJdgGiStQwCZTNMnSiN=430956458.985336204716302389816042387436f; else irTiAuJMIPYAJdgGiStQwCZTNMnSiN=98855549.988897166934308196934965194415f;if (irTiAuJMIPYAJdgGiStQwCZTNMnSiN - irTiAuJMIPYAJdgGiStQwCZTNMnSiN> 0.00000001 ) irTiAuJMIPYAJdgGiStQwCZTNMnSiN=256041159.988894862632398275520151202480f; else irTiAuJMIPYAJdgGiStQwCZTNMnSiN=490776021.376105751534530378792009594860f;double UpISfgJrRaKVzuhgJDwQSirmPnifIG=2027619170.867218746929208645628180588961;if (UpISfgJrRaKVzuhgJDwQSirmPnifIG == UpISfgJrRaKVzuhgJDwQSirmPnifIG ) UpISfgJrRaKVzuhgJDwQSirmPnifIG=467161027.660197310642806496772734079884; else UpISfgJrRaKVzuhgJDwQSirmPnifIG=607159043.188547922144618749519230094713;if (UpISfgJrRaKVzuhgJDwQSirmPnifIG == UpISfgJrRaKVzuhgJDwQSirmPnifIG ) UpISfgJrRaKVzuhgJDwQSirmPnifIG=1777129862.964666437068563922643137596965; else UpISfgJrRaKVzuhgJDwQSirmPnifIG=1005908045.331939106342256287718218093727;if (UpISfgJrRaKVzuhgJDwQSirmPnifIG == UpISfgJrRaKVzuhgJDwQSirmPnifIG ) UpISfgJrRaKVzuhgJDwQSirmPnifIG=182234649.167383774593741299232025813495; else UpISfgJrRaKVzuhgJDwQSirmPnifIG=912696094.649938508462433567880569525267;if (UpISfgJrRaKVzuhgJDwQSirmPnifIG == UpISfgJrRaKVzuhgJDwQSirmPnifIG ) UpISfgJrRaKVzuhgJDwQSirmPnifIG=1124293155.048905482389198160336761529152; else UpISfgJrRaKVzuhgJDwQSirmPnifIG=2026675886.435242453819561248156503249759;if (UpISfgJrRaKVzuhgJDwQSirmPnifIG == UpISfgJrRaKVzuhgJDwQSirmPnifIG ) UpISfgJrRaKVzuhgJDwQSirmPnifIG=1762282302.744195405771752821148100694210; else UpISfgJrRaKVzuhgJDwQSirmPnifIG=1430084616.481518547685349595252861274561;if (UpISfgJrRaKVzuhgJDwQSirmPnifIG == UpISfgJrRaKVzuhgJDwQSirmPnifIG ) UpISfgJrRaKVzuhgJDwQSirmPnifIG=1832615908.130649125539557397111016962045; else UpISfgJrRaKVzuhgJDwQSirmPnifIG=2142511039.419666681057082442599845122478;float DpyEIxrxGkDcGgSxPVYLBbahcnUeSR=1230344292.871862950049462110431118695208f;if (DpyEIxrxGkDcGgSxPVYLBbahcnUeSR - DpyEIxrxGkDcGgSxPVYLBbahcnUeSR> 0.00000001 ) DpyEIxrxGkDcGgSxPVYLBbahcnUeSR=904887118.123295491519177627436006487428f; else DpyEIxrxGkDcGgSxPVYLBbahcnUeSR=1359598134.885302638872903314108897223781f;if (DpyEIxrxGkDcGgSxPVYLBbahcnUeSR - DpyEIxrxGkDcGgSxPVYLBbahcnUeSR> 0.00000001 ) DpyEIxrxGkDcGgSxPVYLBbahcnUeSR=571465261.907374488619967510327343009929f; else DpyEIxrxGkDcGgSxPVYLBbahcnUeSR=669849553.096797664003760003181356567547f;if (DpyEIxrxGkDcGgSxPVYLBbahcnUeSR - DpyEIxrxGkDcGgSxPVYLBbahcnUeSR> 0.00000001 ) DpyEIxrxGkDcGgSxPVYLBbahcnUeSR=1275338334.567072338095743478311685875024f; else DpyEIxrxGkDcGgSxPVYLBbahcnUeSR=6796494.013755770977619771420923708320f;if (DpyEIxrxGkDcGgSxPVYLBbahcnUeSR - DpyEIxrxGkDcGgSxPVYLBbahcnUeSR> 0.00000001 ) DpyEIxrxGkDcGgSxPVYLBbahcnUeSR=1070539001.731418209870955507621571519016f; else DpyEIxrxGkDcGgSxPVYLBbahcnUeSR=82923315.756024621157265018930171937100f;if (DpyEIxrxGkDcGgSxPVYLBbahcnUeSR - DpyEIxrxGkDcGgSxPVYLBbahcnUeSR> 0.00000001 ) DpyEIxrxGkDcGgSxPVYLBbahcnUeSR=1774748671.920413593198768431751229679610f; else DpyEIxrxGkDcGgSxPVYLBbahcnUeSR=741692849.762223896252694252461756981285f;if (DpyEIxrxGkDcGgSxPVYLBbahcnUeSR - DpyEIxrxGkDcGgSxPVYLBbahcnUeSR> 0.00000001 ) DpyEIxrxGkDcGgSxPVYLBbahcnUeSR=1575076708.508444654574650974543726911735f; else DpyEIxrxGkDcGgSxPVYLBbahcnUeSR=2140665332.271186435160899710612874288253f;float vLtNBSXnXaVtXqRvQlNzHHNQXyhwxp=1889607275.098721290869326255919158436671f;if (vLtNBSXnXaVtXqRvQlNzHHNQXyhwxp - vLtNBSXnXaVtXqRvQlNzHHNQXyhwxp> 0.00000001 ) vLtNBSXnXaVtXqRvQlNzHHNQXyhwxp=992177805.816959708606371738080714804040f; else vLtNBSXnXaVtXqRvQlNzHHNQXyhwxp=1119300305.562006348766742399880860110216f;if (vLtNBSXnXaVtXqRvQlNzHHNQXyhwxp - vLtNBSXnXaVtXqRvQlNzHHNQXyhwxp> 0.00000001 ) vLtNBSXnXaVtXqRvQlNzHHNQXyhwxp=969619736.577814537640133418361537180390f; else vLtNBSXnXaVtXqRvQlNzHHNQXyhwxp=1806012891.623339093897335184781306515184f;if (vLtNBSXnXaVtXqRvQlNzHHNQXyhwxp - vLtNBSXnXaVtXqRvQlNzHHNQXyhwxp> 0.00000001 ) vLtNBSXnXaVtXqRvQlNzHHNQXyhwxp=1566245456.424802791452667531300762943782f; else vLtNBSXnXaVtXqRvQlNzHHNQXyhwxp=161375596.592948363349343084009498486397f;if (vLtNBSXnXaVtXqRvQlNzHHNQXyhwxp - vLtNBSXnXaVtXqRvQlNzHHNQXyhwxp> 0.00000001 ) vLtNBSXnXaVtXqRvQlNzHHNQXyhwxp=1395261584.001996172037807253907988411467f; else vLtNBSXnXaVtXqRvQlNzHHNQXyhwxp=986294602.208734845702318279233440023459f;if (vLtNBSXnXaVtXqRvQlNzHHNQXyhwxp - vLtNBSXnXaVtXqRvQlNzHHNQXyhwxp> 0.00000001 ) vLtNBSXnXaVtXqRvQlNzHHNQXyhwxp=1924827973.582693018341920668035323487871f; else vLtNBSXnXaVtXqRvQlNzHHNQXyhwxp=472960387.159723173636199362056643536931f;if (vLtNBSXnXaVtXqRvQlNzHHNQXyhwxp - vLtNBSXnXaVtXqRvQlNzHHNQXyhwxp> 0.00000001 ) vLtNBSXnXaVtXqRvQlNzHHNQXyhwxp=758583505.725830220850651933413592561730f; else vLtNBSXnXaVtXqRvQlNzHHNQXyhwxp=37932846.725201237014230710361452824337f;double ZzBFPWPcAkwdVAxOfJoXHXhxRnSYmh=442847331.562058464461275708584503681546;if (ZzBFPWPcAkwdVAxOfJoXHXhxRnSYmh == ZzBFPWPcAkwdVAxOfJoXHXhxRnSYmh ) ZzBFPWPcAkwdVAxOfJoXHXhxRnSYmh=1380336095.776754492797975100680471706058; else ZzBFPWPcAkwdVAxOfJoXHXhxRnSYmh=1173180514.792880553241699763959207730616;if (ZzBFPWPcAkwdVAxOfJoXHXhxRnSYmh == ZzBFPWPcAkwdVAxOfJoXHXhxRnSYmh ) ZzBFPWPcAkwdVAxOfJoXHXhxRnSYmh=1733372550.855500620565845099204320950626; else ZzBFPWPcAkwdVAxOfJoXHXhxRnSYmh=187603789.104759729025440456275151254565;if (ZzBFPWPcAkwdVAxOfJoXHXhxRnSYmh == ZzBFPWPcAkwdVAxOfJoXHXhxRnSYmh ) ZzBFPWPcAkwdVAxOfJoXHXhxRnSYmh=1489166081.001457110647129008161545105450; else ZzBFPWPcAkwdVAxOfJoXHXhxRnSYmh=1789849081.626412400434764813336650556635;if (ZzBFPWPcAkwdVAxOfJoXHXhxRnSYmh == ZzBFPWPcAkwdVAxOfJoXHXhxRnSYmh ) ZzBFPWPcAkwdVAxOfJoXHXhxRnSYmh=752280836.286012171760974799240223418706; else ZzBFPWPcAkwdVAxOfJoXHXhxRnSYmh=460364533.697625976274367788787840607470;if (ZzBFPWPcAkwdVAxOfJoXHXhxRnSYmh == ZzBFPWPcAkwdVAxOfJoXHXhxRnSYmh ) ZzBFPWPcAkwdVAxOfJoXHXhxRnSYmh=1684103689.558618336992317606683002095803; else ZzBFPWPcAkwdVAxOfJoXHXhxRnSYmh=1694206547.951673785055390554300180766663;if (ZzBFPWPcAkwdVAxOfJoXHXhxRnSYmh == ZzBFPWPcAkwdVAxOfJoXHXhxRnSYmh ) ZzBFPWPcAkwdVAxOfJoXHXhxRnSYmh=1378643021.353304274593274971354151388192; else ZzBFPWPcAkwdVAxOfJoXHXhxRnSYmh=1588025447.712280200639092802799642021312;double EoNnnHwuUkmFiHaFeuYqdZCOImxNbt=34891483.859478642768600189381480930856;if (EoNnnHwuUkmFiHaFeuYqdZCOImxNbt == EoNnnHwuUkmFiHaFeuYqdZCOImxNbt ) EoNnnHwuUkmFiHaFeuYqdZCOImxNbt=2005457170.494943076144673645176083147955; else EoNnnHwuUkmFiHaFeuYqdZCOImxNbt=2090492280.999637577821334892849578151180;if (EoNnnHwuUkmFiHaFeuYqdZCOImxNbt == EoNnnHwuUkmFiHaFeuYqdZCOImxNbt ) EoNnnHwuUkmFiHaFeuYqdZCOImxNbt=1150333234.612136504627946576946622743269; else EoNnnHwuUkmFiHaFeuYqdZCOImxNbt=1646584946.640721031930832264369625500059;if (EoNnnHwuUkmFiHaFeuYqdZCOImxNbt == EoNnnHwuUkmFiHaFeuYqdZCOImxNbt ) EoNnnHwuUkmFiHaFeuYqdZCOImxNbt=1165973038.444509822565602680169090112533; else EoNnnHwuUkmFiHaFeuYqdZCOImxNbt=1207473857.208269643344281413492100156550;if (EoNnnHwuUkmFiHaFeuYqdZCOImxNbt == EoNnnHwuUkmFiHaFeuYqdZCOImxNbt ) EoNnnHwuUkmFiHaFeuYqdZCOImxNbt=1811582483.840046745315782581070024111922; else EoNnnHwuUkmFiHaFeuYqdZCOImxNbt=743511522.420460535587819742877558083693;if (EoNnnHwuUkmFiHaFeuYqdZCOImxNbt == EoNnnHwuUkmFiHaFeuYqdZCOImxNbt ) EoNnnHwuUkmFiHaFeuYqdZCOImxNbt=2111820197.683300643206759421657926507249; else EoNnnHwuUkmFiHaFeuYqdZCOImxNbt=1732654306.805201552021067281976214677781;if (EoNnnHwuUkmFiHaFeuYqdZCOImxNbt == EoNnnHwuUkmFiHaFeuYqdZCOImxNbt ) EoNnnHwuUkmFiHaFeuYqdZCOImxNbt=892505227.340451265997074796942748369163; else EoNnnHwuUkmFiHaFeuYqdZCOImxNbt=802377947.498178162713609034108185971329;double EzLYAyrxeeRWOPwFQuPRawfZSDOMCg=2144558607.441784328115362271182234996427;if (EzLYAyrxeeRWOPwFQuPRawfZSDOMCg == EzLYAyrxeeRWOPwFQuPRawfZSDOMCg ) EzLYAyrxeeRWOPwFQuPRawfZSDOMCg=1417914064.932630184431747149438224631780; else EzLYAyrxeeRWOPwFQuPRawfZSDOMCg=1132229377.801129484310786259895709081800;if (EzLYAyrxeeRWOPwFQuPRawfZSDOMCg == EzLYAyrxeeRWOPwFQuPRawfZSDOMCg ) EzLYAyrxeeRWOPwFQuPRawfZSDOMCg=760329237.940463063887512955858300610225; else EzLYAyrxeeRWOPwFQuPRawfZSDOMCg=88520348.869557142429702830813615265095;if (EzLYAyrxeeRWOPwFQuPRawfZSDOMCg == EzLYAyrxeeRWOPwFQuPRawfZSDOMCg ) EzLYAyrxeeRWOPwFQuPRawfZSDOMCg=2141501916.395812916692354960989151225439; else EzLYAyrxeeRWOPwFQuPRawfZSDOMCg=1384507304.216813490002433247231621635579;if (EzLYAyrxeeRWOPwFQuPRawfZSDOMCg == EzLYAyrxeeRWOPwFQuPRawfZSDOMCg ) EzLYAyrxeeRWOPwFQuPRawfZSDOMCg=400441194.859749764157008773919930970528; else EzLYAyrxeeRWOPwFQuPRawfZSDOMCg=1349812371.903225489558318562397266003098;if (EzLYAyrxeeRWOPwFQuPRawfZSDOMCg == EzLYAyrxeeRWOPwFQuPRawfZSDOMCg ) EzLYAyrxeeRWOPwFQuPRawfZSDOMCg=367019469.925736498340456262241823174733; else EzLYAyrxeeRWOPwFQuPRawfZSDOMCg=509255203.359264479318516892195185721266;if (EzLYAyrxeeRWOPwFQuPRawfZSDOMCg == EzLYAyrxeeRWOPwFQuPRawfZSDOMCg ) EzLYAyrxeeRWOPwFQuPRawfZSDOMCg=121909202.287510139389494442839274927465; else EzLYAyrxeeRWOPwFQuPRawfZSDOMCg=2019522556.321324100677481670248953802379;double ljXyeoGmCMUIGfKsqTPZkMKgTopbHJ=793716503.715185828412649035888816413891;if (ljXyeoGmCMUIGfKsqTPZkMKgTopbHJ == ljXyeoGmCMUIGfKsqTPZkMKgTopbHJ ) ljXyeoGmCMUIGfKsqTPZkMKgTopbHJ=1855469010.565514903246821606925206700078; else ljXyeoGmCMUIGfKsqTPZkMKgTopbHJ=723692022.204795813524275200681807747092;if (ljXyeoGmCMUIGfKsqTPZkMKgTopbHJ == ljXyeoGmCMUIGfKsqTPZkMKgTopbHJ ) ljXyeoGmCMUIGfKsqTPZkMKgTopbHJ=179959323.503604324699440103250140751715; else ljXyeoGmCMUIGfKsqTPZkMKgTopbHJ=641985899.983113178198609246865494256496;if (ljXyeoGmCMUIGfKsqTPZkMKgTopbHJ == ljXyeoGmCMUIGfKsqTPZkMKgTopbHJ ) ljXyeoGmCMUIGfKsqTPZkMKgTopbHJ=627340477.472981734813617355151505150906; else ljXyeoGmCMUIGfKsqTPZkMKgTopbHJ=1972161133.789273783861306298801234598784;if (ljXyeoGmCMUIGfKsqTPZkMKgTopbHJ == ljXyeoGmCMUIGfKsqTPZkMKgTopbHJ ) ljXyeoGmCMUIGfKsqTPZkMKgTopbHJ=1424357940.294813545567330030097320666579; else ljXyeoGmCMUIGfKsqTPZkMKgTopbHJ=172862238.510978859385445400566870797536;if (ljXyeoGmCMUIGfKsqTPZkMKgTopbHJ == ljXyeoGmCMUIGfKsqTPZkMKgTopbHJ ) ljXyeoGmCMUIGfKsqTPZkMKgTopbHJ=1509081678.808605746339272209527612384128; else ljXyeoGmCMUIGfKsqTPZkMKgTopbHJ=989861028.132468884076434770960901020633;if (ljXyeoGmCMUIGfKsqTPZkMKgTopbHJ == ljXyeoGmCMUIGfKsqTPZkMKgTopbHJ ) ljXyeoGmCMUIGfKsqTPZkMKgTopbHJ=223267239.272820882452566128961437924255; else ljXyeoGmCMUIGfKsqTPZkMKgTopbHJ=1903995422.925291561409952854690288385500;float fTkaMmNmbcekXfGamcClweaWhUlxCf=651497534.483806270191611876015812819433f;if (fTkaMmNmbcekXfGamcClweaWhUlxCf - fTkaMmNmbcekXfGamcClweaWhUlxCf> 0.00000001 ) fTkaMmNmbcekXfGamcClweaWhUlxCf=48794117.398072487594827263548658678219f; else fTkaMmNmbcekXfGamcClweaWhUlxCf=1381568061.885880364940963109565609685151f;if (fTkaMmNmbcekXfGamcClweaWhUlxCf - fTkaMmNmbcekXfGamcClweaWhUlxCf> 0.00000001 ) fTkaMmNmbcekXfGamcClweaWhUlxCf=170111383.528881015791960870667975529630f; else fTkaMmNmbcekXfGamcClweaWhUlxCf=352873656.632353328220597584982550575347f;if (fTkaMmNmbcekXfGamcClweaWhUlxCf - fTkaMmNmbcekXfGamcClweaWhUlxCf> 0.00000001 ) fTkaMmNmbcekXfGamcClweaWhUlxCf=370782233.248517072866345859805676928298f; else fTkaMmNmbcekXfGamcClweaWhUlxCf=676454676.908208208161322631683755277360f;if (fTkaMmNmbcekXfGamcClweaWhUlxCf - fTkaMmNmbcekXfGamcClweaWhUlxCf> 0.00000001 ) fTkaMmNmbcekXfGamcClweaWhUlxCf=40873214.988407814765184445430872635404f; else fTkaMmNmbcekXfGamcClweaWhUlxCf=620949866.194829493692792868194304121697f;if (fTkaMmNmbcekXfGamcClweaWhUlxCf - fTkaMmNmbcekXfGamcClweaWhUlxCf> 0.00000001 ) fTkaMmNmbcekXfGamcClweaWhUlxCf=1690051086.252177236605295819097831190181f; else fTkaMmNmbcekXfGamcClweaWhUlxCf=1948004510.981645173573250911931260152119f;if (fTkaMmNmbcekXfGamcClweaWhUlxCf - fTkaMmNmbcekXfGamcClweaWhUlxCf> 0.00000001 ) fTkaMmNmbcekXfGamcClweaWhUlxCf=1279565477.816682575185763568229026073160f; else fTkaMmNmbcekXfGamcClweaWhUlxCf=656717835.316586778845695761869499089172f;long WAaonrJljpyCXnURUCNBINCHwJjWzi=157100413;if (WAaonrJljpyCXnURUCNBINCHwJjWzi == WAaonrJljpyCXnURUCNBINCHwJjWzi- 1 ) WAaonrJljpyCXnURUCNBINCHwJjWzi=83979090; else WAaonrJljpyCXnURUCNBINCHwJjWzi=98788724;if (WAaonrJljpyCXnURUCNBINCHwJjWzi == WAaonrJljpyCXnURUCNBINCHwJjWzi- 0 ) WAaonrJljpyCXnURUCNBINCHwJjWzi=1094392584; else WAaonrJljpyCXnURUCNBINCHwJjWzi=672004614;if (WAaonrJljpyCXnURUCNBINCHwJjWzi == WAaonrJljpyCXnURUCNBINCHwJjWzi- 1 ) WAaonrJljpyCXnURUCNBINCHwJjWzi=852646832; else WAaonrJljpyCXnURUCNBINCHwJjWzi=1003426531;if (WAaonrJljpyCXnURUCNBINCHwJjWzi == WAaonrJljpyCXnURUCNBINCHwJjWzi- 0 ) WAaonrJljpyCXnURUCNBINCHwJjWzi=1517218130; else WAaonrJljpyCXnURUCNBINCHwJjWzi=1593236578;if (WAaonrJljpyCXnURUCNBINCHwJjWzi == WAaonrJljpyCXnURUCNBINCHwJjWzi- 1 ) WAaonrJljpyCXnURUCNBINCHwJjWzi=1855355084; else WAaonrJljpyCXnURUCNBINCHwJjWzi=587466999;if (WAaonrJljpyCXnURUCNBINCHwJjWzi == WAaonrJljpyCXnURUCNBINCHwJjWzi- 0 ) WAaonrJljpyCXnURUCNBINCHwJjWzi=726922317; else WAaonrJljpyCXnURUCNBINCHwJjWzi=1151640888;float nuddjwdapyLcAMipYeEBFYvkpYfJhj=417116580.787532853278077921408336293320f;if (nuddjwdapyLcAMipYeEBFYvkpYfJhj - nuddjwdapyLcAMipYeEBFYvkpYfJhj> 0.00000001 ) nuddjwdapyLcAMipYeEBFYvkpYfJhj=1444587306.670976046068232468598548046145f; else nuddjwdapyLcAMipYeEBFYvkpYfJhj=1617895315.964360921298555011483683314203f;if (nuddjwdapyLcAMipYeEBFYvkpYfJhj - nuddjwdapyLcAMipYeEBFYvkpYfJhj> 0.00000001 ) nuddjwdapyLcAMipYeEBFYvkpYfJhj=1970728439.091210507355841010304686248215f; else nuddjwdapyLcAMipYeEBFYvkpYfJhj=1696131808.782821844562423994331491052508f;if (nuddjwdapyLcAMipYeEBFYvkpYfJhj - nuddjwdapyLcAMipYeEBFYvkpYfJhj> 0.00000001 ) nuddjwdapyLcAMipYeEBFYvkpYfJhj=1784608401.340368303038784336446498549297f; else nuddjwdapyLcAMipYeEBFYvkpYfJhj=1295801560.944732227669231482783627205293f;if (nuddjwdapyLcAMipYeEBFYvkpYfJhj - nuddjwdapyLcAMipYeEBFYvkpYfJhj> 0.00000001 ) nuddjwdapyLcAMipYeEBFYvkpYfJhj=422934270.022619406212088727816862543754f; else nuddjwdapyLcAMipYeEBFYvkpYfJhj=1179014334.813465872208657274968858737128f;if (nuddjwdapyLcAMipYeEBFYvkpYfJhj - nuddjwdapyLcAMipYeEBFYvkpYfJhj> 0.00000001 ) nuddjwdapyLcAMipYeEBFYvkpYfJhj=682818808.786351963406671821538080607674f; else nuddjwdapyLcAMipYeEBFYvkpYfJhj=1706493104.976453340741267238791679180675f;if (nuddjwdapyLcAMipYeEBFYvkpYfJhj - nuddjwdapyLcAMipYeEBFYvkpYfJhj> 0.00000001 ) nuddjwdapyLcAMipYeEBFYvkpYfJhj=1099418460.430963123968180882358998331747f; else nuddjwdapyLcAMipYeEBFYvkpYfJhj=826819614.378541231422694459030843865263f;double VnbdryeXrfbHvNfQBVvpJjEkZSdkpi=1717021751.965537256437848834522367231234;if (VnbdryeXrfbHvNfQBVvpJjEkZSdkpi == VnbdryeXrfbHvNfQBVvpJjEkZSdkpi ) VnbdryeXrfbHvNfQBVvpJjEkZSdkpi=1176565728.623758638168031869805872170831; else VnbdryeXrfbHvNfQBVvpJjEkZSdkpi=66166826.712382429130757444658244995269;if (VnbdryeXrfbHvNfQBVvpJjEkZSdkpi == VnbdryeXrfbHvNfQBVvpJjEkZSdkpi ) VnbdryeXrfbHvNfQBVvpJjEkZSdkpi=1820016332.881845775044077623573729368840; else VnbdryeXrfbHvNfQBVvpJjEkZSdkpi=798555130.837135091387971427747370626259;if (VnbdryeXrfbHvNfQBVvpJjEkZSdkpi == VnbdryeXrfbHvNfQBVvpJjEkZSdkpi ) VnbdryeXrfbHvNfQBVvpJjEkZSdkpi=1002993952.893631805363229696894120366851; else VnbdryeXrfbHvNfQBVvpJjEkZSdkpi=540146930.085360435072358516114156578438;if (VnbdryeXrfbHvNfQBVvpJjEkZSdkpi == VnbdryeXrfbHvNfQBVvpJjEkZSdkpi ) VnbdryeXrfbHvNfQBVvpJjEkZSdkpi=1946334349.615439996026283387295175803989; else VnbdryeXrfbHvNfQBVvpJjEkZSdkpi=1192851233.210857792072019897439125364174;if (VnbdryeXrfbHvNfQBVvpJjEkZSdkpi == VnbdryeXrfbHvNfQBVvpJjEkZSdkpi ) VnbdryeXrfbHvNfQBVvpJjEkZSdkpi=2011833578.602781654660532317711756120550; else VnbdryeXrfbHvNfQBVvpJjEkZSdkpi=724358452.373847133825498060415857406777;if (VnbdryeXrfbHvNfQBVvpJjEkZSdkpi == VnbdryeXrfbHvNfQBVvpJjEkZSdkpi ) VnbdryeXrfbHvNfQBVvpJjEkZSdkpi=645002219.210439889529499637916124663755; else VnbdryeXrfbHvNfQBVvpJjEkZSdkpi=1930798881.256661606929199746897996229697; }
 VnbdryeXrfbHvNfQBVvpJjEkZSdkpiy::VnbdryeXrfbHvNfQBVvpJjEkZSdkpiy()
 { this->cRTQNaiiGIQK("lqJmSAObTtWGjXhVwJKUlhWAmjrDZVcRTQNaiiGIQKj", true, 2097016006, 1036715973, 1607193448); }
#pragma optimize("", off)
 // <delete/>

