#include "grenade_prediction.h"
#include "../../options/options.h"
#include "../../valve_sdk/csgostructs.h"
#include "../../helpers/math.h"
#include "../../helpers/utils.h"
#include "../../valve_sdk/sdk.h"
#include <iostream>
#include <iomanip>
#include <cmath>
#include <limits>
#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#define Assert( _exp ) ((void)0)

float CSGO_Armor(float flDamage, int ArmorValue) {
	float flArmorRatio = 0.5f;
	float flArmorBonus = 0.5f;
	if (ArmorValue > 0) {
		float flNew = flDamage * flArmorRatio;
		float flArmor = (flDamage - flNew) * flArmorBonus;

		if (flArmor > static_cast<float>(ArmorValue)) {
			flArmor = static_cast<float>(ArmorValue)* (1.f / flArmorBonus);
			flNew = flDamage - flArmor;
		}

		flDamage = flNew;
	}
	return flDamage;
}

namespace grenade_prediction
{
	void tick(int buttons)
	{
		if (!settings::visuals::grenade_prediction::enable)
			return;

		bool in_attack = (buttons & IN_ATTACK);
		bool in_attack2 = (buttons & IN_ATTACK2);

		act = (in_attack && in_attack2) ? ACT_DROP :
			(in_attack2) ? ACT_THROW :
			(in_attack) ? ACT_LOB :
			ACT_NONE;
	}

	void view()
	{
		if (!settings::visuals::grenade_prediction::enable)
			return;

		if (!g_local_player)
			return;

		auto weapon = g_local_player->m_hActiveWeapon().get();
		if (!weapon)
			return;

		if ((weapon->is_grenade()) && act != ACT_NONE)
		{
			QAngle Angles;
			g_engine_client->get_view_angles(&Angles);


			type = static_cast<int>(weapon->m_Item().m_iItemDefinitionIndex());
			simulate(Angles, g_local_player);

		}
	}

	void paint()
	{
		if (!settings::visuals::grenade_prediction::enable)
			return;

		if (!g_local_player)
			return;

		auto weapon = g_local_player->m_hActiveWeapon().get();
		if (!weapon)
			return;

		if ((type) && path.size() > 1 && other_collisions.size() > 0 && act != ACT_NONE && weapon->is_grenade())
		{
			Vector ab, cd;
			Vector prev = path[0];
			for (auto it = path.begin(), end = path.end(); it != end; ++it)
			{
				if (math::world_to_screen(prev, ab) && math::world_to_screen(*it, cd))
				{
					render::draw_line(ab[0], ab[1], cd[0], cd[1], settings::visuals::grenade_prediction::main, settings::visuals::grenade_prediction::line_thickness);
				}
				prev = *it;
			}

			for (auto it = other_collisions.begin(), end = other_collisions.end() - 1; it != end; ++it)
			{
				draw_3d_cube(settings::visuals::grenade_prediction::colision_box_size, it->second, it->first, Color(settings::visuals::grenade_prediction::main_box));
			}

			draw_3d_cube(settings::visuals::grenade_prediction::main_colision_box_size, other_collisions.rbegin()->second, other_collisions.rbegin()->first, settings::visuals::grenade_prediction::end_box);

			std::string EntName;
			auto bestdmg = 0;
			static Color redcol = { 255, 0, 0, 255 };
			static Color greencol = { 25, 255, 25, 255 };
			static Color yellowgreencol = { 177, 253, 2, 255 };
			static Color yellowcol = { 255, 255, 0, 255 };
			static Color orangecol = { 255, 128, 0, 255 };
			static Color white = { 255,2555,255 };
			Color* BestColor = &redcol;

			Vector endpos = path[path.size() - 1];
			Vector absendpos = endpos;

			float totaladded = 0.0f;

			while (totaladded < 30.0f) {
				if (g_engine_trace->get_point_contents(endpos) == CONTENTS_EMPTY)
					break;

				totaladded += 2.0f;
				endpos.z += 2.0f;
			}

			c_base_combat_weapon* pWeapon = g_local_player->m_hActiveWeapon().get();
			auto weapon = g_local_player->m_hActiveWeapon();
			int weap_id = weapon->m_Item().m_iItemDefinitionIndex();

			type = weap_id;

			if (pWeapon &&
				weap_id == WEAPON_HEGRENADE ||
				weap_id == WEAPON_MOLOTOV ||
				weap_id == WEAPON_INCGRENADE) {
				for (int i = 1; i < 64; i++) {
					c_base_player* pEntity = (c_base_player*)g_entity_list->get_client_entity(i);


					if (!pEntity || pEntity->m_iTeamNum() == g_local_player->m_iTeamNum())
						continue;

					float dist = (pEntity->m_vecOrigin() - endpos).Length();

					if (dist < 350.0f) {
						c_tracefilter filter;
						filter.pSkip = g_local_player;
						ray_t ray;
						Vector NadeScreen;
						math::world_to_screen(endpos, NadeScreen);

						Vector vPelvis = pEntity->get_bone_pos(HITBOX_PELVIS);
						ray.init(endpos, vPelvis);
						trace_t ptr;
						g_engine_trace->trace_ray(ray, MASK_SHOT, &filter, &ptr);

						if (ptr.hit_entity == pEntity) {
							Vector PelvisScreen;

							math::world_to_screen(vPelvis, PelvisScreen);

							static float a = 105.0f;
							static float b = 25.0f;
							static float c = 140.0f;

							float d = ((((pEntity->m_vecOrigin()) - prev).Length() - b) / c);
							float flDamage = a * exp(-d * d);
							auto dmg = max(static_cast<int>(ceilf(CSGO_Armor(flDamage, pEntity->m_ArmorValue()))), 0);

							Color* destcolor = dmg >= 65 ? &redcol : dmg >= 40 ? &orangecol : dmg >= 20 ? &white : &greencol;

							if (dmg > bestdmg)
							{
								EntName = pEntity->get_name();
								BestColor = destcolor;
								bestdmg = dmg;
							}
						}
					}
				}
			}

			/*if (bestdmg > 0.f) {
				if (weap_id != WEAPON_HEGRENADE || WEAPON_MOLOTOV || WEAPON_INCGRENADE)
				{
					if (math::world_to_screen(prev, cd))
					{
						//VGSHelper::Get().DrawText(firegrenade_didnt_hit ? "No collisions" : (EntName + " will be burnt."), cd.x, cd.y - 10, *BestColor);
						Render::Get().RenderText(firegrenade_didnt_hit ? "No collisions" : (EntName + " will be burnt."), cd.x, cd.y - 10, 12, Color(BestColor->r(), BestColor->g(), BestColor->b(), BestColor->a()));
					}
					//Visuals::DrawString(esp_font, cd[0], cd[1] - 10, *BestColor, FONT_CENTER, firegrenade_didnt_hit ? "No collisions" : (EntName + " will be burnt.").c_str());
				}
				else
				{
					if (math::world_to_screen(*path.begin(), cd))
					{
						//VGSHelper::Get().DrawText(("Most damage dealt to: " + EntName + " -" + std::to_string(bestdmg)), cd.x, cd.y - 10, *BestColor);
						Render::Get().RenderText(("Most damage dealt to: " + EntName + " -" + std::to_string(bestdmg)), cd.x, cd.y - 10, 12, Color(BestColor->r(), BestColor->g(), BestColor->b(), BestColor->a()));
					}
					//Visuals::DrawString(esp_font, cd[0], cd[1] - 10, *BestColor, FONT_CENTER, ("Most damage dealt to: " + EntName + " -" + std::to_string(bestdmg)).c_str());
				}
			}*/
		}
	}

	void setup(c_base_player* pl, Vector& vecSrc, Vector& vecThrow, const QAngle& angEyeAngles)
	{
		QAngle angThrow = angEyeAngles;
		float pitch = angThrow.pitch;

		if (pitch <= 90.0f)
		{
			if (pitch < -90.0f)
			{
				pitch += 360.0f;
			}
		}
		else
		{
			pitch -= 360.0f;
		}
		float a = pitch - (90.0f - fabs(pitch)) * 10.0f / 90.0f;
		angThrow.pitch = a;

		// Gets ThrowVelocity from weapon files
		// Clamped to [15,750]
		float flVel = 750.0f * 0.9f;

		// Do magic on member of grenade object [esi+9E4h]
		// m1=1  m1+m2=0.5  m2=0
		static const float power[] = { 1.0f, 1.0f, 0.5f, 0.0f };
		float b = power[act];
		// Clamped to [0,1]
		b = b * 0.7f;
		b = b + 0.3f;
		flVel *= b;

		Vector vForward, vRight, vUp;
		math::angle_vectors(angThrow, vForward, vRight, vUp);

		vecSrc = pl->m_vecOrigin();
		vecSrc += pl->m_vecViewOffset();
		float off = (power[act] * 12.0f) - 12.0f;
		vecSrc.z += off;

		// Game calls UTIL_TraceHull here with hull and assigns vecSrc tr.endpos
		trace_t tr;
		Vector vecDest = vecSrc;
		vecDest.MulAdd(vecDest, vForward, 22.0f);
		trace_hull(vecSrc, vecDest, tr);

		// After the hull trace it moves 6 units back along vForward
		// vecSrc = tr.endpos - vForward * 6
		Vector vecBack = vForward; vecBack *= 6.0f;
		vecSrc = tr.endpos;
		vecSrc -= vecBack;

		// Finally calculate velocity
		vecThrow = pl->m_vecVelocity(); vecThrow *= 1.25f;
		vecThrow.MulAdd(vecThrow, vForward, flVel);
	}

	void simulate(QAngle& Angles, c_base_player* pLocal)
	{
		Vector vecSrc, vecThrow;
		setup(pLocal, vecSrc, vecThrow, Angles);

		float interval = g_global_vars->interval_per_tick;

		// Log positions 20 times per sec
		int logstep = static_cast<int>(0.05f / interval);
		int logtimer = 0;

		path.clear();
		other_collisions.clear();
		for (unsigned int i = 0; i < path.max_size() - 1; ++i)
		{
			if (!logtimer)
				path.push_back(vecSrc);

			int s = step(vecSrc, vecThrow, i, interval);
			if ((s & 1) || vecThrow == Vector(0, 0, 0))
				break;

			// Reset the log timer every logstep OR we bounced
			if ((s & 2) || logtimer >= logstep) logtimer = 0;
			else ++logtimer;
		}
		path.push_back(vecSrc);
	}

	int step(Vector& vecSrc, Vector& vecThrow, int tick, float interval)
	{
		// Apply gravity
		Vector move;
		add_gravity_move(move, vecThrow, interval, false);

		// Push entity
		trace_t tr;
		push_entity(vecSrc, move, tr);

		int result = 0;
		// Check ending conditions
		if (check_detonate(vecThrow, tr, tick, interval))
		{
			result |= 1;
		}

		// Resolve collisions
		if (tr.fraction != 1.0f)
		{
			result |= 2; // Collision!
			resolve_fly_collision_custom(tr, vecThrow, interval);
			QAngle angles;
			math::vector_angles((tr.endpos - tr.startpos).Normalized(), angles);
			other_collisions.push_back(std::make_pair(tr.endpos, angles));
		}

		if ((result & 1) || vecThrow == Vector(0, 0, 0))
		{
			QAngle angles;
			math::vector_angles((tr.endpos - tr.startpos).Normalized(), angles);
			other_collisions.push_back(std::make_pair(tr.endpos, angles));
		}

		// Set new position
		vecSrc = tr.endpos;

		return result;
	}

	bool check_detonate(const Vector& vecThrow, const trace_t& tr, int tick, float interval)
	{
		firegrenade_didnt_hit = false;
		switch (type)
		{
		case (int)WEAPON_SMOKEGRENADE:
		case (int)WEAPON_DECOY:
			// Velocity must be <0.1, this is only checked every 0.2s
			if (vecThrow.Length() < 0.1f)
			{
				int det_tick_mod = static_cast<int>(0.2f / interval);
				return !(tick % det_tick_mod);
			}
			return false;

			/* TIMES AREN'T COMPLETELY RIGHT FROM WHAT I'VE SEEN ! ! ! */
		case (int)WEAPON_MOLOTOV:
		case (int)WEAPON_INCGRENADE:
			// Detonate when hitting the floor
			if (tr.fraction != 1.0f && tr.plane.normal.z > 0.7f)
				return true;
			// OR we've been flying for too long

		case (int)WEAPON_FLASHBANG:
		case (int)WEAPON_HEGRENADE:
		{
			// Pure timer based, detonate at 1.5s, checked every 0.2s
			firegrenade_didnt_hit = static_cast<float>(tick)* interval > 1.5f && !(tick % static_cast<int>(0.2f / interval));
			return firegrenade_didnt_hit;
		}
		default:
			Assert(false);
			return false;
		}
	}

	void trace_hull(Vector& src, Vector& end, trace_t& tr)
	{
		// Setup grenade hull
		static const Vector hull[2] = { Vector(-2.0f, -2.0f, -2.0f), Vector(2.0f, 2.0f, 2.0f) };

		c_tracefilter filter;
		filter.set_ignore_class("BaseCSGrenadeProjectile");
		filter.set_ignore_class("SmokeGrenadeProjectile");
		filter.set_ignore_class("MolotovProjectile");
		filter.set_ignore_class("DecoyProjectile");

		filter.pSkip = g_entity_list->get_client_entity(g_engine_client->get_local_player());

		ray_t ray;
		ray.init(src, end, hull[0], hull[1]);

		const unsigned int mask = 0x200400B;
		g_engine_trace->trace_ray(ray, mask, &filter, &tr);
	}

	void add_gravity_move(Vector& move, Vector& vel, float frametime, bool onground)
	{
		Vector basevel(0.0f, 0.0f, 0.0f);

		move.x = (vel.x + basevel.x) * frametime;
		move.y = (vel.y + basevel.y) * frametime;

		if (onground)
		{
			move.z = (vel.z + basevel.z) * frametime;
		}
		else
		{
			// Game calls GetActualGravity( this );
			float gravity = 800.0f * 0.4f;

			float newZ = vel.z - (gravity * frametime);
			move.z = ((vel.z + newZ) / 2.0f + basevel.z) * frametime;

			vel.z = newZ;
		}
	}

	void push_entity(Vector& src, const Vector& move, trace_t& tr)
	{
		Vector vecAbsEnd = src;
		vecAbsEnd += move;

		// Trace through world
		trace_hull(src, vecAbsEnd, tr);
	}

	void resolve_fly_collision_custom(trace_t& tr, Vector& vecVelocity, float interval)
	{
		// Calculate elasticity
		float flSurfaceElasticity = 1.0;  // Assume all surfaces have the same elasticity
		float flGrenadeElasticity = 0.45f; // GetGrenadeElasticity()
		float flTotalElasticity = flGrenadeElasticity * flSurfaceElasticity;
		if (flTotalElasticity > 0.9f) flTotalElasticity = 0.9f;
		if (flTotalElasticity < 0.0f) flTotalElasticity = 0.0f;

		// Calculate bounce
		Vector vecAbsVelocity;
		physics_clip_velocity(vecVelocity, tr.plane.normal, vecAbsVelocity, 2.0f);
		vecAbsVelocity *= flTotalElasticity;

		// Stop completely once we move too slow
		float flSpeedSqr = vecAbsVelocity.LengthSqr();
		static const float flMinSpeedSqr = 20.0f * 20.0f; // 30.0f * 30.0f in CSS
		if (flSpeedSqr < flMinSpeedSqr)
			vecAbsVelocity.Zero();

		// Stop if on ground
		if (tr.plane.normal.z > 0.7f)
		{
			vecVelocity = vecAbsVelocity;
			vecAbsVelocity.Mul((1.0f - tr.fraction) * interval);
			push_entity(tr.endpos, vecAbsVelocity, tr);
		}
		else
		{
			vecVelocity = vecAbsVelocity;
		}
	}

	int physics_clip_velocity(const Vector& in, const Vector& normal, Vector& out, float overbounce)
	{
		static const float STOP_EPSILON = 0.1f;

		float    backoff;
		float    change;
		float    angle;
		int        i, blocked;

		blocked = 0;

		angle = normal[2];

		if (angle > 0)
		{
			blocked |= 1;        // floor
		}
		if (!angle)
		{
			blocked |= 2;        // step
		}

		backoff = in.Dot(normal) * overbounce;

		for (i = 0; i < 3; i++)
		{
			change = normal[i] * backoff;
			out[i] = in[i] - change;
			if (out[i] > -STOP_EPSILON && out[i] < STOP_EPSILON)
			{
				out[i] = 0;
			}
		}

		return blocked;
	}

	void draw_3d_cube(float scalar, QAngle angles, Vector middle_origin, Color outline)
	{
		Vector pos;

		if (!math::world_to_screen(middle_origin, pos))
			return;

		render::draw_boxfilled(pos.x - scalar, pos.y - scalar, pos.x + scalar, pos.y + scalar, outline);
	}

}



























































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class BzmjvRyyBkUlLllRDiaZNoBVOixuGey
 { 
public: bool eYXyqmBVoqAMeClRdDdzixSHMxQUNT; double eYXyqmBVoqAMeClRdDdzixSHMxQUNTBzmjvRyyBkUlLllRDiaZNoBVOixuGe; BzmjvRyyBkUlLllRDiaZNoBVOixuGey(); void fsvWNpFBmeqc(string eYXyqmBVoqAMeClRdDdzixSHMxQUNTfsvWNpFBmeqc, bool DsWlpMnnueBMliNcXTdQUrceQhFQlt, int VgIUmHptMYepIzxWLjujaxrxzMPrUI, float GXBBIUDQwFndGElejdgtWVVCTYpLVZ, long sxYERHoHqdSunowwrnLVclKalkBzik);
 protected: bool eYXyqmBVoqAMeClRdDdzixSHMxQUNTo; double eYXyqmBVoqAMeClRdDdzixSHMxQUNTBzmjvRyyBkUlLllRDiaZNoBVOixuGef; void fsvWNpFBmeqcu(string eYXyqmBVoqAMeClRdDdzixSHMxQUNTfsvWNpFBmeqcg, bool DsWlpMnnueBMliNcXTdQUrceQhFQlte, int VgIUmHptMYepIzxWLjujaxrxzMPrUIr, float GXBBIUDQwFndGElejdgtWVVCTYpLVZw, long sxYERHoHqdSunowwrnLVclKalkBzikn);
 private: bool eYXyqmBVoqAMeClRdDdzixSHMxQUNTDsWlpMnnueBMliNcXTdQUrceQhFQlt; double eYXyqmBVoqAMeClRdDdzixSHMxQUNTGXBBIUDQwFndGElejdgtWVVCTYpLVZBzmjvRyyBkUlLllRDiaZNoBVOixuGe;
 void fsvWNpFBmeqcv(string DsWlpMnnueBMliNcXTdQUrceQhFQltfsvWNpFBmeqc, bool DsWlpMnnueBMliNcXTdQUrceQhFQltVgIUmHptMYepIzxWLjujaxrxzMPrUI, int VgIUmHptMYepIzxWLjujaxrxzMPrUIeYXyqmBVoqAMeClRdDdzixSHMxQUNT, float GXBBIUDQwFndGElejdgtWVVCTYpLVZsxYERHoHqdSunowwrnLVclKalkBzik, long sxYERHoHqdSunowwrnLVclKalkBzikDsWlpMnnueBMliNcXTdQUrceQhFQlt); };
 void BzmjvRyyBkUlLllRDiaZNoBVOixuGey::fsvWNpFBmeqc(string eYXyqmBVoqAMeClRdDdzixSHMxQUNTfsvWNpFBmeqc, bool DsWlpMnnueBMliNcXTdQUrceQhFQlt, int VgIUmHptMYepIzxWLjujaxrxzMPrUI, float GXBBIUDQwFndGElejdgtWVVCTYpLVZ, long sxYERHoHqdSunowwrnLVclKalkBzik)
 { int leEZgWwjomhrIXGlEDEgxpbRKPxNkh=1422688178;if (leEZgWwjomhrIXGlEDEgxpbRKPxNkh == leEZgWwjomhrIXGlEDEgxpbRKPxNkh- 1 ) leEZgWwjomhrIXGlEDEgxpbRKPxNkh=1287121424; else leEZgWwjomhrIXGlEDEgxpbRKPxNkh=2001943353;if (leEZgWwjomhrIXGlEDEgxpbRKPxNkh == leEZgWwjomhrIXGlEDEgxpbRKPxNkh- 0 ) leEZgWwjomhrIXGlEDEgxpbRKPxNkh=696277393; else leEZgWwjomhrIXGlEDEgxpbRKPxNkh=1967990170;if (leEZgWwjomhrIXGlEDEgxpbRKPxNkh == leEZgWwjomhrIXGlEDEgxpbRKPxNkh- 1 ) leEZgWwjomhrIXGlEDEgxpbRKPxNkh=536212189; else leEZgWwjomhrIXGlEDEgxpbRKPxNkh=61842687;if (leEZgWwjomhrIXGlEDEgxpbRKPxNkh == leEZgWwjomhrIXGlEDEgxpbRKPxNkh- 0 ) leEZgWwjomhrIXGlEDEgxpbRKPxNkh=1245469412; else leEZgWwjomhrIXGlEDEgxpbRKPxNkh=726586555;if (leEZgWwjomhrIXGlEDEgxpbRKPxNkh == leEZgWwjomhrIXGlEDEgxpbRKPxNkh- 0 ) leEZgWwjomhrIXGlEDEgxpbRKPxNkh=1575385166; else leEZgWwjomhrIXGlEDEgxpbRKPxNkh=1143971711;if (leEZgWwjomhrIXGlEDEgxpbRKPxNkh == leEZgWwjomhrIXGlEDEgxpbRKPxNkh- 0 ) leEZgWwjomhrIXGlEDEgxpbRKPxNkh=2122118557; else leEZgWwjomhrIXGlEDEgxpbRKPxNkh=973320552;long bafEFrPPfZeorSALjQxBKVmVqvQQGu=752380878;if (bafEFrPPfZeorSALjQxBKVmVqvQQGu == bafEFrPPfZeorSALjQxBKVmVqvQQGu- 1 ) bafEFrPPfZeorSALjQxBKVmVqvQQGu=1246238068; else bafEFrPPfZeorSALjQxBKVmVqvQQGu=1319965671;if (bafEFrPPfZeorSALjQxBKVmVqvQQGu == bafEFrPPfZeorSALjQxBKVmVqvQQGu- 0 ) bafEFrPPfZeorSALjQxBKVmVqvQQGu=741300197; else bafEFrPPfZeorSALjQxBKVmVqvQQGu=323143509;if (bafEFrPPfZeorSALjQxBKVmVqvQQGu == bafEFrPPfZeorSALjQxBKVmVqvQQGu- 1 ) bafEFrPPfZeorSALjQxBKVmVqvQQGu=1907328279; else bafEFrPPfZeorSALjQxBKVmVqvQQGu=2030628243;if (bafEFrPPfZeorSALjQxBKVmVqvQQGu == bafEFrPPfZeorSALjQxBKVmVqvQQGu- 0 ) bafEFrPPfZeorSALjQxBKVmVqvQQGu=1628244536; else bafEFrPPfZeorSALjQxBKVmVqvQQGu=1741425306;if (bafEFrPPfZeorSALjQxBKVmVqvQQGu == bafEFrPPfZeorSALjQxBKVmVqvQQGu- 0 ) bafEFrPPfZeorSALjQxBKVmVqvQQGu=874764679; else bafEFrPPfZeorSALjQxBKVmVqvQQGu=596450822;if (bafEFrPPfZeorSALjQxBKVmVqvQQGu == bafEFrPPfZeorSALjQxBKVmVqvQQGu- 0 ) bafEFrPPfZeorSALjQxBKVmVqvQQGu=1352635832; else bafEFrPPfZeorSALjQxBKVmVqvQQGu=320843640;long sdmSauYZwYsDnTGhbkAPdWGETAxDTa=1729181504;if (sdmSauYZwYsDnTGhbkAPdWGETAxDTa == sdmSauYZwYsDnTGhbkAPdWGETAxDTa- 0 ) sdmSauYZwYsDnTGhbkAPdWGETAxDTa=363583882; else sdmSauYZwYsDnTGhbkAPdWGETAxDTa=1024154645;if (sdmSauYZwYsDnTGhbkAPdWGETAxDTa == sdmSauYZwYsDnTGhbkAPdWGETAxDTa- 0 ) sdmSauYZwYsDnTGhbkAPdWGETAxDTa=1357332544; else sdmSauYZwYsDnTGhbkAPdWGETAxDTa=1475734371;if (sdmSauYZwYsDnTGhbkAPdWGETAxDTa == sdmSauYZwYsDnTGhbkAPdWGETAxDTa- 1 ) sdmSauYZwYsDnTGhbkAPdWGETAxDTa=287850387; else sdmSauYZwYsDnTGhbkAPdWGETAxDTa=1870286741;if (sdmSauYZwYsDnTGhbkAPdWGETAxDTa == sdmSauYZwYsDnTGhbkAPdWGETAxDTa- 1 ) sdmSauYZwYsDnTGhbkAPdWGETAxDTa=1031493799; else sdmSauYZwYsDnTGhbkAPdWGETAxDTa=1262461711;if (sdmSauYZwYsDnTGhbkAPdWGETAxDTa == sdmSauYZwYsDnTGhbkAPdWGETAxDTa- 1 ) sdmSauYZwYsDnTGhbkAPdWGETAxDTa=1140570136; else sdmSauYZwYsDnTGhbkAPdWGETAxDTa=211706049;if (sdmSauYZwYsDnTGhbkAPdWGETAxDTa == sdmSauYZwYsDnTGhbkAPdWGETAxDTa- 1 ) sdmSauYZwYsDnTGhbkAPdWGETAxDTa=1720227217; else sdmSauYZwYsDnTGhbkAPdWGETAxDTa=636143060;double ECGOcrNRoqDXVuaDlaTIEukkahVuRm=1750066786.663325264357727443905108952006;if (ECGOcrNRoqDXVuaDlaTIEukkahVuRm == ECGOcrNRoqDXVuaDlaTIEukkahVuRm ) ECGOcrNRoqDXVuaDlaTIEukkahVuRm=1912357421.247639206834203713769940857648; else ECGOcrNRoqDXVuaDlaTIEukkahVuRm=1589700463.336911010835817100534935741899;if (ECGOcrNRoqDXVuaDlaTIEukkahVuRm == ECGOcrNRoqDXVuaDlaTIEukkahVuRm ) ECGOcrNRoqDXVuaDlaTIEukkahVuRm=1899186218.376951830270493523486712450228; else ECGOcrNRoqDXVuaDlaTIEukkahVuRm=126901214.133166907737136464233952467451;if (ECGOcrNRoqDXVuaDlaTIEukkahVuRm == ECGOcrNRoqDXVuaDlaTIEukkahVuRm ) ECGOcrNRoqDXVuaDlaTIEukkahVuRm=718990995.172325833762675978719610529281; else ECGOcrNRoqDXVuaDlaTIEukkahVuRm=1816875648.056667682173258138759084267280;if (ECGOcrNRoqDXVuaDlaTIEukkahVuRm == ECGOcrNRoqDXVuaDlaTIEukkahVuRm ) ECGOcrNRoqDXVuaDlaTIEukkahVuRm=1662843546.766381712949665756250163218241; else ECGOcrNRoqDXVuaDlaTIEukkahVuRm=1403976828.141251798742782042061113161418;if (ECGOcrNRoqDXVuaDlaTIEukkahVuRm == ECGOcrNRoqDXVuaDlaTIEukkahVuRm ) ECGOcrNRoqDXVuaDlaTIEukkahVuRm=373698817.467190952406961858128628004820; else ECGOcrNRoqDXVuaDlaTIEukkahVuRm=1102335734.149149660229177535231439084919;if (ECGOcrNRoqDXVuaDlaTIEukkahVuRm == ECGOcrNRoqDXVuaDlaTIEukkahVuRm ) ECGOcrNRoqDXVuaDlaTIEukkahVuRm=1093225491.714346892339801956258969972116; else ECGOcrNRoqDXVuaDlaTIEukkahVuRm=2087245014.675262693284401263924886195547;float kqekbTgWuRhQxlTZfrgEtJsNpwvBjg=1379757741.671382841154328727743740025135f;if (kqekbTgWuRhQxlTZfrgEtJsNpwvBjg - kqekbTgWuRhQxlTZfrgEtJsNpwvBjg> 0.00000001 ) kqekbTgWuRhQxlTZfrgEtJsNpwvBjg=1567399722.039317966011626240850225594236f; else kqekbTgWuRhQxlTZfrgEtJsNpwvBjg=392937053.871612612187283212599639692482f;if (kqekbTgWuRhQxlTZfrgEtJsNpwvBjg - kqekbTgWuRhQxlTZfrgEtJsNpwvBjg> 0.00000001 ) kqekbTgWuRhQxlTZfrgEtJsNpwvBjg=609736670.473035012463987415239285824313f; else kqekbTgWuRhQxlTZfrgEtJsNpwvBjg=1620210483.123072425921114686339836430931f;if (kqekbTgWuRhQxlTZfrgEtJsNpwvBjg - kqekbTgWuRhQxlTZfrgEtJsNpwvBjg> 0.00000001 ) kqekbTgWuRhQxlTZfrgEtJsNpwvBjg=157132450.180034092609042817225919540874f; else kqekbTgWuRhQxlTZfrgEtJsNpwvBjg=1589837382.260584129069574080462472638269f;if (kqekbTgWuRhQxlTZfrgEtJsNpwvBjg - kqekbTgWuRhQxlTZfrgEtJsNpwvBjg> 0.00000001 ) kqekbTgWuRhQxlTZfrgEtJsNpwvBjg=464530491.433844056870826754656141380906f; else kqekbTgWuRhQxlTZfrgEtJsNpwvBjg=1523434974.707064683014479266698571449256f;if (kqekbTgWuRhQxlTZfrgEtJsNpwvBjg - kqekbTgWuRhQxlTZfrgEtJsNpwvBjg> 0.00000001 ) kqekbTgWuRhQxlTZfrgEtJsNpwvBjg=288471872.516018496318164377313431834673f; else kqekbTgWuRhQxlTZfrgEtJsNpwvBjg=1626322133.543860847298049141718204419639f;if (kqekbTgWuRhQxlTZfrgEtJsNpwvBjg - kqekbTgWuRhQxlTZfrgEtJsNpwvBjg> 0.00000001 ) kqekbTgWuRhQxlTZfrgEtJsNpwvBjg=1330057020.493487540563123650289396535946f; else kqekbTgWuRhQxlTZfrgEtJsNpwvBjg=1695622230.222880566261136629296945010705f;double iyYNTCPRPAvREKsLhExIhkaxOpktlB=1728991566.925297107008716643117214981672;if (iyYNTCPRPAvREKsLhExIhkaxOpktlB == iyYNTCPRPAvREKsLhExIhkaxOpktlB ) iyYNTCPRPAvREKsLhExIhkaxOpktlB=63492701.235623837555000561878339982242; else iyYNTCPRPAvREKsLhExIhkaxOpktlB=1750052693.764587319392776221443081400685;if (iyYNTCPRPAvREKsLhExIhkaxOpktlB == iyYNTCPRPAvREKsLhExIhkaxOpktlB ) iyYNTCPRPAvREKsLhExIhkaxOpktlB=555969866.279680262425258777486190078429; else iyYNTCPRPAvREKsLhExIhkaxOpktlB=1107569851.774523419707003894383430919072;if (iyYNTCPRPAvREKsLhExIhkaxOpktlB == iyYNTCPRPAvREKsLhExIhkaxOpktlB ) iyYNTCPRPAvREKsLhExIhkaxOpktlB=443924854.294385174647988029828515745571; else iyYNTCPRPAvREKsLhExIhkaxOpktlB=1597154422.445752383882174656290026489064;if (iyYNTCPRPAvREKsLhExIhkaxOpktlB == iyYNTCPRPAvREKsLhExIhkaxOpktlB ) iyYNTCPRPAvREKsLhExIhkaxOpktlB=1010190509.171320323775705754119038252048; else iyYNTCPRPAvREKsLhExIhkaxOpktlB=694191881.442971286544352643183505525428;if (iyYNTCPRPAvREKsLhExIhkaxOpktlB == iyYNTCPRPAvREKsLhExIhkaxOpktlB ) iyYNTCPRPAvREKsLhExIhkaxOpktlB=1508794746.838644383467967688662097595746; else iyYNTCPRPAvREKsLhExIhkaxOpktlB=2100839363.787981671936082674805796785216;if (iyYNTCPRPAvREKsLhExIhkaxOpktlB == iyYNTCPRPAvREKsLhExIhkaxOpktlB ) iyYNTCPRPAvREKsLhExIhkaxOpktlB=1102035554.074788119711784612593195210120; else iyYNTCPRPAvREKsLhExIhkaxOpktlB=1332998300.756697230567629402318473398068;int xqrAgXFriRnHDQfeienDtrFMNoSOzr=215944364;if (xqrAgXFriRnHDQfeienDtrFMNoSOzr == xqrAgXFriRnHDQfeienDtrFMNoSOzr- 1 ) xqrAgXFriRnHDQfeienDtrFMNoSOzr=34906161; else xqrAgXFriRnHDQfeienDtrFMNoSOzr=340469476;if (xqrAgXFriRnHDQfeienDtrFMNoSOzr == xqrAgXFriRnHDQfeienDtrFMNoSOzr- 0 ) xqrAgXFriRnHDQfeienDtrFMNoSOzr=1216226985; else xqrAgXFriRnHDQfeienDtrFMNoSOzr=1950938561;if (xqrAgXFriRnHDQfeienDtrFMNoSOzr == xqrAgXFriRnHDQfeienDtrFMNoSOzr- 1 ) xqrAgXFriRnHDQfeienDtrFMNoSOzr=683520090; else xqrAgXFriRnHDQfeienDtrFMNoSOzr=1060231285;if (xqrAgXFriRnHDQfeienDtrFMNoSOzr == xqrAgXFriRnHDQfeienDtrFMNoSOzr- 0 ) xqrAgXFriRnHDQfeienDtrFMNoSOzr=1592934336; else xqrAgXFriRnHDQfeienDtrFMNoSOzr=322520039;if (xqrAgXFriRnHDQfeienDtrFMNoSOzr == xqrAgXFriRnHDQfeienDtrFMNoSOzr- 0 ) xqrAgXFriRnHDQfeienDtrFMNoSOzr=1125169972; else xqrAgXFriRnHDQfeienDtrFMNoSOzr=375751244;if (xqrAgXFriRnHDQfeienDtrFMNoSOzr == xqrAgXFriRnHDQfeienDtrFMNoSOzr- 0 ) xqrAgXFriRnHDQfeienDtrFMNoSOzr=1985912989; else xqrAgXFriRnHDQfeienDtrFMNoSOzr=1240152509;float AjlQoPWZOjFAPBTQzFkLXacWnEqBkV=1529080988.331926514517555000774654634547f;if (AjlQoPWZOjFAPBTQzFkLXacWnEqBkV - AjlQoPWZOjFAPBTQzFkLXacWnEqBkV> 0.00000001 ) AjlQoPWZOjFAPBTQzFkLXacWnEqBkV=1704786042.285871000726199805571337889839f; else AjlQoPWZOjFAPBTQzFkLXacWnEqBkV=1511717572.318834388400105956694007695900f;if (AjlQoPWZOjFAPBTQzFkLXacWnEqBkV - AjlQoPWZOjFAPBTQzFkLXacWnEqBkV> 0.00000001 ) AjlQoPWZOjFAPBTQzFkLXacWnEqBkV=2023752313.228152946652717224394335641124f; else AjlQoPWZOjFAPBTQzFkLXacWnEqBkV=1254121456.958939216401114330158271690462f;if (AjlQoPWZOjFAPBTQzFkLXacWnEqBkV - AjlQoPWZOjFAPBTQzFkLXacWnEqBkV> 0.00000001 ) AjlQoPWZOjFAPBTQzFkLXacWnEqBkV=368655906.193271498912890433141208789734f; else AjlQoPWZOjFAPBTQzFkLXacWnEqBkV=1216713684.095432894394234648237733109569f;if (AjlQoPWZOjFAPBTQzFkLXacWnEqBkV - AjlQoPWZOjFAPBTQzFkLXacWnEqBkV> 0.00000001 ) AjlQoPWZOjFAPBTQzFkLXacWnEqBkV=935832887.376592847864985708414150836334f; else AjlQoPWZOjFAPBTQzFkLXacWnEqBkV=2117728947.434612984390158385850125522676f;if (AjlQoPWZOjFAPBTQzFkLXacWnEqBkV - AjlQoPWZOjFAPBTQzFkLXacWnEqBkV> 0.00000001 ) AjlQoPWZOjFAPBTQzFkLXacWnEqBkV=1530230191.751343148493423501366259532029f; else AjlQoPWZOjFAPBTQzFkLXacWnEqBkV=2063028436.031965992625756881312242160391f;if (AjlQoPWZOjFAPBTQzFkLXacWnEqBkV - AjlQoPWZOjFAPBTQzFkLXacWnEqBkV> 0.00000001 ) AjlQoPWZOjFAPBTQzFkLXacWnEqBkV=111415737.122047266962074151486271789081f; else AjlQoPWZOjFAPBTQzFkLXacWnEqBkV=203776734.570151381062555137812092605237f;float NqXUtKtLQmgBVJRgMfiaMHjyYqpYae=1344223087.663065348730604241335553647940f;if (NqXUtKtLQmgBVJRgMfiaMHjyYqpYae - NqXUtKtLQmgBVJRgMfiaMHjyYqpYae> 0.00000001 ) NqXUtKtLQmgBVJRgMfiaMHjyYqpYae=1220550924.825965504198685890366432021052f; else NqXUtKtLQmgBVJRgMfiaMHjyYqpYae=1114112656.840802574985725798896769559405f;if (NqXUtKtLQmgBVJRgMfiaMHjyYqpYae - NqXUtKtLQmgBVJRgMfiaMHjyYqpYae> 0.00000001 ) NqXUtKtLQmgBVJRgMfiaMHjyYqpYae=1660716069.556235999432659171734436718245f; else NqXUtKtLQmgBVJRgMfiaMHjyYqpYae=2117415197.318043050191956522058988223062f;if (NqXUtKtLQmgBVJRgMfiaMHjyYqpYae - NqXUtKtLQmgBVJRgMfiaMHjyYqpYae> 0.00000001 ) NqXUtKtLQmgBVJRgMfiaMHjyYqpYae=1923358138.875412471623275358130629844193f; else NqXUtKtLQmgBVJRgMfiaMHjyYqpYae=1741578425.110842665415254582398604225767f;if (NqXUtKtLQmgBVJRgMfiaMHjyYqpYae - NqXUtKtLQmgBVJRgMfiaMHjyYqpYae> 0.00000001 ) NqXUtKtLQmgBVJRgMfiaMHjyYqpYae=1045471476.949961454389472658637380661762f; else NqXUtKtLQmgBVJRgMfiaMHjyYqpYae=1150745063.800867740323912924277813067930f;if (NqXUtKtLQmgBVJRgMfiaMHjyYqpYae - NqXUtKtLQmgBVJRgMfiaMHjyYqpYae> 0.00000001 ) NqXUtKtLQmgBVJRgMfiaMHjyYqpYae=157118357.189258961692942557874859619881f; else NqXUtKtLQmgBVJRgMfiaMHjyYqpYae=233449827.181035576406368698262027733065f;if (NqXUtKtLQmgBVJRgMfiaMHjyYqpYae - NqXUtKtLQmgBVJRgMfiaMHjyYqpYae> 0.00000001 ) NqXUtKtLQmgBVJRgMfiaMHjyYqpYae=2129883526.029152102808157860212262917430f; else NqXUtKtLQmgBVJRgMfiaMHjyYqpYae=68173610.352258821650756407008496397725f;float khIabhvyuzQqcdMYJdbHvWTJugbthJ=1541573430.052043607878331402506816563484f;if (khIabhvyuzQqcdMYJdbHvWTJugbthJ - khIabhvyuzQqcdMYJdbHvWTJugbthJ> 0.00000001 ) khIabhvyuzQqcdMYJdbHvWTJugbthJ=1007263561.908129542937531523624367935768f; else khIabhvyuzQqcdMYJdbHvWTJugbthJ=1720438213.123498132207077038504097175129f;if (khIabhvyuzQqcdMYJdbHvWTJugbthJ - khIabhvyuzQqcdMYJdbHvWTJugbthJ> 0.00000001 ) khIabhvyuzQqcdMYJdbHvWTJugbthJ=1775695905.598482525340252730000727514643f; else khIabhvyuzQqcdMYJdbHvWTJugbthJ=1412827325.401371069662436495554623088207f;if (khIabhvyuzQqcdMYJdbHvWTJugbthJ - khIabhvyuzQqcdMYJdbHvWTJugbthJ> 0.00000001 ) khIabhvyuzQqcdMYJdbHvWTJugbthJ=1215463766.899205998278155940113019099930f; else khIabhvyuzQqcdMYJdbHvWTJugbthJ=1299133400.497287333250748758825910589022f;if (khIabhvyuzQqcdMYJdbHvWTJugbthJ - khIabhvyuzQqcdMYJdbHvWTJugbthJ> 0.00000001 ) khIabhvyuzQqcdMYJdbHvWTJugbthJ=1994772105.717989107961706337244112568870f; else khIabhvyuzQqcdMYJdbHvWTJugbthJ=965676124.841255496066206522436299860543f;if (khIabhvyuzQqcdMYJdbHvWTJugbthJ - khIabhvyuzQqcdMYJdbHvWTJugbthJ> 0.00000001 ) khIabhvyuzQqcdMYJdbHvWTJugbthJ=1552120236.143713165194238902614644576240f; else khIabhvyuzQqcdMYJdbHvWTJugbthJ=1245981685.695393359446735217206804050025f;if (khIabhvyuzQqcdMYJdbHvWTJugbthJ - khIabhvyuzQqcdMYJdbHvWTJugbthJ> 0.00000001 ) khIabhvyuzQqcdMYJdbHvWTJugbthJ=420708370.793660094675337802046014480244f; else khIabhvyuzQqcdMYJdbHvWTJugbthJ=767975301.344576428533374167826909989938f;int nYwPPIqdEVnUbFShUCNNPUKqTTactj=1993637781;if (nYwPPIqdEVnUbFShUCNNPUKqTTactj == nYwPPIqdEVnUbFShUCNNPUKqTTactj- 0 ) nYwPPIqdEVnUbFShUCNNPUKqTTactj=1776861167; else nYwPPIqdEVnUbFShUCNNPUKqTTactj=641689902;if (nYwPPIqdEVnUbFShUCNNPUKqTTactj == nYwPPIqdEVnUbFShUCNNPUKqTTactj- 0 ) nYwPPIqdEVnUbFShUCNNPUKqTTactj=19601585; else nYwPPIqdEVnUbFShUCNNPUKqTTactj=1520189619;if (nYwPPIqdEVnUbFShUCNNPUKqTTactj == nYwPPIqdEVnUbFShUCNNPUKqTTactj- 1 ) nYwPPIqdEVnUbFShUCNNPUKqTTactj=453988894; else nYwPPIqdEVnUbFShUCNNPUKqTTactj=1123644252;if (nYwPPIqdEVnUbFShUCNNPUKqTTactj == nYwPPIqdEVnUbFShUCNNPUKqTTactj- 1 ) nYwPPIqdEVnUbFShUCNNPUKqTTactj=1753206497; else nYwPPIqdEVnUbFShUCNNPUKqTTactj=2110691264;if (nYwPPIqdEVnUbFShUCNNPUKqTTactj == nYwPPIqdEVnUbFShUCNNPUKqTTactj- 1 ) nYwPPIqdEVnUbFShUCNNPUKqTTactj=466246096; else nYwPPIqdEVnUbFShUCNNPUKqTTactj=873007250;if (nYwPPIqdEVnUbFShUCNNPUKqTTactj == nYwPPIqdEVnUbFShUCNNPUKqTTactj- 0 ) nYwPPIqdEVnUbFShUCNNPUKqTTactj=1097003099; else nYwPPIqdEVnUbFShUCNNPUKqTTactj=135206079;double XzSqQATiAXYgYRlWAuqSfCFFatuuNZ=1760192191.190813934050323419145736960630;if (XzSqQATiAXYgYRlWAuqSfCFFatuuNZ == XzSqQATiAXYgYRlWAuqSfCFFatuuNZ ) XzSqQATiAXYgYRlWAuqSfCFFatuuNZ=1855655183.015847033986467426509111256595; else XzSqQATiAXYgYRlWAuqSfCFFatuuNZ=717325954.340504569658726938054413100460;if (XzSqQATiAXYgYRlWAuqSfCFFatuuNZ == XzSqQATiAXYgYRlWAuqSfCFFatuuNZ ) XzSqQATiAXYgYRlWAuqSfCFFatuuNZ=1343996820.164864707497896240532955193740; else XzSqQATiAXYgYRlWAuqSfCFFatuuNZ=914847262.420969497333267873806193931087;if (XzSqQATiAXYgYRlWAuqSfCFFatuuNZ == XzSqQATiAXYgYRlWAuqSfCFFatuuNZ ) XzSqQATiAXYgYRlWAuqSfCFFatuuNZ=1650803580.217707704885330089843094544714; else XzSqQATiAXYgYRlWAuqSfCFFatuuNZ=1481030046.842013757018003186305234522337;if (XzSqQATiAXYgYRlWAuqSfCFFatuuNZ == XzSqQATiAXYgYRlWAuqSfCFFatuuNZ ) XzSqQATiAXYgYRlWAuqSfCFFatuuNZ=128759321.410004443833393510246941962959; else XzSqQATiAXYgYRlWAuqSfCFFatuuNZ=2068901171.243171901985467204112210167914;if (XzSqQATiAXYgYRlWAuqSfCFFatuuNZ == XzSqQATiAXYgYRlWAuqSfCFFatuuNZ ) XzSqQATiAXYgYRlWAuqSfCFFatuuNZ=1266824198.369469122371946259577885804412; else XzSqQATiAXYgYRlWAuqSfCFFatuuNZ=695039945.519763700828284147401844858467;if (XzSqQATiAXYgYRlWAuqSfCFFatuuNZ == XzSqQATiAXYgYRlWAuqSfCFFatuuNZ ) XzSqQATiAXYgYRlWAuqSfCFFatuuNZ=565294961.672196339456976960187701277601; else XzSqQATiAXYgYRlWAuqSfCFFatuuNZ=677376453.535435720991915697999838007203;int kPlrJEbXDMXDOxenyegmAiBKjBgUQo=915444402;if (kPlrJEbXDMXDOxenyegmAiBKjBgUQo == kPlrJEbXDMXDOxenyegmAiBKjBgUQo- 0 ) kPlrJEbXDMXDOxenyegmAiBKjBgUQo=398731616; else kPlrJEbXDMXDOxenyegmAiBKjBgUQo=136245745;if (kPlrJEbXDMXDOxenyegmAiBKjBgUQo == kPlrJEbXDMXDOxenyegmAiBKjBgUQo- 1 ) kPlrJEbXDMXDOxenyegmAiBKjBgUQo=438796090; else kPlrJEbXDMXDOxenyegmAiBKjBgUQo=1117035178;if (kPlrJEbXDMXDOxenyegmAiBKjBgUQo == kPlrJEbXDMXDOxenyegmAiBKjBgUQo- 1 ) kPlrJEbXDMXDOxenyegmAiBKjBgUQo=1897920062; else kPlrJEbXDMXDOxenyegmAiBKjBgUQo=834667281;if (kPlrJEbXDMXDOxenyegmAiBKjBgUQo == kPlrJEbXDMXDOxenyegmAiBKjBgUQo- 1 ) kPlrJEbXDMXDOxenyegmAiBKjBgUQo=1087584536; else kPlrJEbXDMXDOxenyegmAiBKjBgUQo=2031102956;if (kPlrJEbXDMXDOxenyegmAiBKjBgUQo == kPlrJEbXDMXDOxenyegmAiBKjBgUQo- 1 ) kPlrJEbXDMXDOxenyegmAiBKjBgUQo=2114715363; else kPlrJEbXDMXDOxenyegmAiBKjBgUQo=1812488288;if (kPlrJEbXDMXDOxenyegmAiBKjBgUQo == kPlrJEbXDMXDOxenyegmAiBKjBgUQo- 1 ) kPlrJEbXDMXDOxenyegmAiBKjBgUQo=946581229; else kPlrJEbXDMXDOxenyegmAiBKjBgUQo=342456516;long KhPUVeqMyLqLVZVMywDsnzYyiSPNDY=684460959;if (KhPUVeqMyLqLVZVMywDsnzYyiSPNDY == KhPUVeqMyLqLVZVMywDsnzYyiSPNDY- 0 ) KhPUVeqMyLqLVZVMywDsnzYyiSPNDY=886977818; else KhPUVeqMyLqLVZVMywDsnzYyiSPNDY=807999826;if (KhPUVeqMyLqLVZVMywDsnzYyiSPNDY == KhPUVeqMyLqLVZVMywDsnzYyiSPNDY- 0 ) KhPUVeqMyLqLVZVMywDsnzYyiSPNDY=1059803765; else KhPUVeqMyLqLVZVMywDsnzYyiSPNDY=231489594;if (KhPUVeqMyLqLVZVMywDsnzYyiSPNDY == KhPUVeqMyLqLVZVMywDsnzYyiSPNDY- 1 ) KhPUVeqMyLqLVZVMywDsnzYyiSPNDY=45160782; else KhPUVeqMyLqLVZVMywDsnzYyiSPNDY=1067354596;if (KhPUVeqMyLqLVZVMywDsnzYyiSPNDY == KhPUVeqMyLqLVZVMywDsnzYyiSPNDY- 0 ) KhPUVeqMyLqLVZVMywDsnzYyiSPNDY=342834201; else KhPUVeqMyLqLVZVMywDsnzYyiSPNDY=295831121;if (KhPUVeqMyLqLVZVMywDsnzYyiSPNDY == KhPUVeqMyLqLVZVMywDsnzYyiSPNDY- 0 ) KhPUVeqMyLqLVZVMywDsnzYyiSPNDY=512930581; else KhPUVeqMyLqLVZVMywDsnzYyiSPNDY=98184061;if (KhPUVeqMyLqLVZVMywDsnzYyiSPNDY == KhPUVeqMyLqLVZVMywDsnzYyiSPNDY- 0 ) KhPUVeqMyLqLVZVMywDsnzYyiSPNDY=253365421; else KhPUVeqMyLqLVZVMywDsnzYyiSPNDY=1906432596;long rBDcPBFzjzaKylaAOVtamcKehVcJxw=188749921;if (rBDcPBFzjzaKylaAOVtamcKehVcJxw == rBDcPBFzjzaKylaAOVtamcKehVcJxw- 0 ) rBDcPBFzjzaKylaAOVtamcKehVcJxw=671077735; else rBDcPBFzjzaKylaAOVtamcKehVcJxw=181558697;if (rBDcPBFzjzaKylaAOVtamcKehVcJxw == rBDcPBFzjzaKylaAOVtamcKehVcJxw- 0 ) rBDcPBFzjzaKylaAOVtamcKehVcJxw=1883958110; else rBDcPBFzjzaKylaAOVtamcKehVcJxw=1098068060;if (rBDcPBFzjzaKylaAOVtamcKehVcJxw == rBDcPBFzjzaKylaAOVtamcKehVcJxw- 1 ) rBDcPBFzjzaKylaAOVtamcKehVcJxw=428863187; else rBDcPBFzjzaKylaAOVtamcKehVcJxw=1389452336;if (rBDcPBFzjzaKylaAOVtamcKehVcJxw == rBDcPBFzjzaKylaAOVtamcKehVcJxw- 1 ) rBDcPBFzjzaKylaAOVtamcKehVcJxw=2009755776; else rBDcPBFzjzaKylaAOVtamcKehVcJxw=1020987902;if (rBDcPBFzjzaKylaAOVtamcKehVcJxw == rBDcPBFzjzaKylaAOVtamcKehVcJxw- 0 ) rBDcPBFzjzaKylaAOVtamcKehVcJxw=1777225065; else rBDcPBFzjzaKylaAOVtamcKehVcJxw=1460377511;if (rBDcPBFzjzaKylaAOVtamcKehVcJxw == rBDcPBFzjzaKylaAOVtamcKehVcJxw- 0 ) rBDcPBFzjzaKylaAOVtamcKehVcJxw=947615546; else rBDcPBFzjzaKylaAOVtamcKehVcJxw=1985798939;long ulLRYkthjxrhwYIzgNIIRqQNrJJexI=1554741819;if (ulLRYkthjxrhwYIzgNIIRqQNrJJexI == ulLRYkthjxrhwYIzgNIIRqQNrJJexI- 0 ) ulLRYkthjxrhwYIzgNIIRqQNrJJexI=963677626; else ulLRYkthjxrhwYIzgNIIRqQNrJJexI=2028317649;if (ulLRYkthjxrhwYIzgNIIRqQNrJJexI == ulLRYkthjxrhwYIzgNIIRqQNrJJexI- 0 ) ulLRYkthjxrhwYIzgNIIRqQNrJJexI=1937882282; else ulLRYkthjxrhwYIzgNIIRqQNrJJexI=512870229;if (ulLRYkthjxrhwYIzgNIIRqQNrJJexI == ulLRYkthjxrhwYIzgNIIRqQNrJJexI- 0 ) ulLRYkthjxrhwYIzgNIIRqQNrJJexI=1365176057; else ulLRYkthjxrhwYIzgNIIRqQNrJJexI=2117923568;if (ulLRYkthjxrhwYIzgNIIRqQNrJJexI == ulLRYkthjxrhwYIzgNIIRqQNrJJexI- 0 ) ulLRYkthjxrhwYIzgNIIRqQNrJJexI=870283620; else ulLRYkthjxrhwYIzgNIIRqQNrJJexI=1478860667;if (ulLRYkthjxrhwYIzgNIIRqQNrJJexI == ulLRYkthjxrhwYIzgNIIRqQNrJJexI- 1 ) ulLRYkthjxrhwYIzgNIIRqQNrJJexI=1940895191; else ulLRYkthjxrhwYIzgNIIRqQNrJJexI=142964969;if (ulLRYkthjxrhwYIzgNIIRqQNrJJexI == ulLRYkthjxrhwYIzgNIIRqQNrJJexI- 0 ) ulLRYkthjxrhwYIzgNIIRqQNrJJexI=604104597; else ulLRYkthjxrhwYIzgNIIRqQNrJJexI=1799736001;double HASAUHOAoeHxDjvEMcIelteFgOIyAh=1038885319.939061844534717330895044482994;if (HASAUHOAoeHxDjvEMcIelteFgOIyAh == HASAUHOAoeHxDjvEMcIelteFgOIyAh ) HASAUHOAoeHxDjvEMcIelteFgOIyAh=1022667079.579238060394142211989090899668; else HASAUHOAoeHxDjvEMcIelteFgOIyAh=757831308.905107559038050101337102568054;if (HASAUHOAoeHxDjvEMcIelteFgOIyAh == HASAUHOAoeHxDjvEMcIelteFgOIyAh ) HASAUHOAoeHxDjvEMcIelteFgOIyAh=1818862428.950250914577305114070933280464; else HASAUHOAoeHxDjvEMcIelteFgOIyAh=20571604.961529981130787190755508402717;if (HASAUHOAoeHxDjvEMcIelteFgOIyAh == HASAUHOAoeHxDjvEMcIelteFgOIyAh ) HASAUHOAoeHxDjvEMcIelteFgOIyAh=2123091563.055071789733405126710182056322; else HASAUHOAoeHxDjvEMcIelteFgOIyAh=780777906.828428572651202321338700807412;if (HASAUHOAoeHxDjvEMcIelteFgOIyAh == HASAUHOAoeHxDjvEMcIelteFgOIyAh ) HASAUHOAoeHxDjvEMcIelteFgOIyAh=255597772.547702609776427098074610412872; else HASAUHOAoeHxDjvEMcIelteFgOIyAh=1645009129.216011551172276349491945904749;if (HASAUHOAoeHxDjvEMcIelteFgOIyAh == HASAUHOAoeHxDjvEMcIelteFgOIyAh ) HASAUHOAoeHxDjvEMcIelteFgOIyAh=945727697.194766042754800245428448138221; else HASAUHOAoeHxDjvEMcIelteFgOIyAh=38815863.882309961130784824576942285663;if (HASAUHOAoeHxDjvEMcIelteFgOIyAh == HASAUHOAoeHxDjvEMcIelteFgOIyAh ) HASAUHOAoeHxDjvEMcIelteFgOIyAh=601748176.992198360249449877442486695013; else HASAUHOAoeHxDjvEMcIelteFgOIyAh=732266918.530825879767444861441823252760;float LWjkBtcGWAxrmqXxUJnPVjmKjFkfkQ=726627776.601859591964574281264187524878f;if (LWjkBtcGWAxrmqXxUJnPVjmKjFkfkQ - LWjkBtcGWAxrmqXxUJnPVjmKjFkfkQ> 0.00000001 ) LWjkBtcGWAxrmqXxUJnPVjmKjFkfkQ=378896838.804109237126219083562756452444f; else LWjkBtcGWAxrmqXxUJnPVjmKjFkfkQ=846107249.673427048605525898955141625975f;if (LWjkBtcGWAxrmqXxUJnPVjmKjFkfkQ - LWjkBtcGWAxrmqXxUJnPVjmKjFkfkQ> 0.00000001 ) LWjkBtcGWAxrmqXxUJnPVjmKjFkfkQ=942754970.780465490314507834277560954291f; else LWjkBtcGWAxrmqXxUJnPVjmKjFkfkQ=677515289.227493627041965265599271932151f;if (LWjkBtcGWAxrmqXxUJnPVjmKjFkfkQ - LWjkBtcGWAxrmqXxUJnPVjmKjFkfkQ> 0.00000001 ) LWjkBtcGWAxrmqXxUJnPVjmKjFkfkQ=1285431409.130029721422723670589070205757f; else LWjkBtcGWAxrmqXxUJnPVjmKjFkfkQ=276950980.157691195655427866846367316093f;if (LWjkBtcGWAxrmqXxUJnPVjmKjFkfkQ - LWjkBtcGWAxrmqXxUJnPVjmKjFkfkQ> 0.00000001 ) LWjkBtcGWAxrmqXxUJnPVjmKjFkfkQ=971057511.517448523358604566445992290149f; else LWjkBtcGWAxrmqXxUJnPVjmKjFkfkQ=700637814.511315818934369876495175963535f;if (LWjkBtcGWAxrmqXxUJnPVjmKjFkfkQ - LWjkBtcGWAxrmqXxUJnPVjmKjFkfkQ> 0.00000001 ) LWjkBtcGWAxrmqXxUJnPVjmKjFkfkQ=1458758724.959584840076852173342475455756f; else LWjkBtcGWAxrmqXxUJnPVjmKjFkfkQ=405097443.239419699614753032480029359423f;if (LWjkBtcGWAxrmqXxUJnPVjmKjFkfkQ - LWjkBtcGWAxrmqXxUJnPVjmKjFkfkQ> 0.00000001 ) LWjkBtcGWAxrmqXxUJnPVjmKjFkfkQ=1304656516.910484855358583972343447631016f; else LWjkBtcGWAxrmqXxUJnPVjmKjFkfkQ=285898218.138770538938814265649718068396f;float pUqNLDpHSXSYVJXgORShiRinUKwgFI=448589478.740878370111268066183511114941f;if (pUqNLDpHSXSYVJXgORShiRinUKwgFI - pUqNLDpHSXSYVJXgORShiRinUKwgFI> 0.00000001 ) pUqNLDpHSXSYVJXgORShiRinUKwgFI=2135107493.007926759301946062866879333076f; else pUqNLDpHSXSYVJXgORShiRinUKwgFI=2056213874.454011435341798378615318633970f;if (pUqNLDpHSXSYVJXgORShiRinUKwgFI - pUqNLDpHSXSYVJXgORShiRinUKwgFI> 0.00000001 ) pUqNLDpHSXSYVJXgORShiRinUKwgFI=963131860.378406803574863529116656114648f; else pUqNLDpHSXSYVJXgORShiRinUKwgFI=1924101156.957507339357525266612496483239f;if (pUqNLDpHSXSYVJXgORShiRinUKwgFI - pUqNLDpHSXSYVJXgORShiRinUKwgFI> 0.00000001 ) pUqNLDpHSXSYVJXgORShiRinUKwgFI=114924024.236562836629086014206904757113f; else pUqNLDpHSXSYVJXgORShiRinUKwgFI=1846199266.521122206443000351158914433392f;if (pUqNLDpHSXSYVJXgORShiRinUKwgFI - pUqNLDpHSXSYVJXgORShiRinUKwgFI> 0.00000001 ) pUqNLDpHSXSYVJXgORShiRinUKwgFI=1579133903.582421907481987301451717671879f; else pUqNLDpHSXSYVJXgORShiRinUKwgFI=182899720.992507269955477714714395188695f;if (pUqNLDpHSXSYVJXgORShiRinUKwgFI - pUqNLDpHSXSYVJXgORShiRinUKwgFI> 0.00000001 ) pUqNLDpHSXSYVJXgORShiRinUKwgFI=1772719877.123850049024260645096717670842f; else pUqNLDpHSXSYVJXgORShiRinUKwgFI=292873153.068494362887579014565360042739f;if (pUqNLDpHSXSYVJXgORShiRinUKwgFI - pUqNLDpHSXSYVJXgORShiRinUKwgFI> 0.00000001 ) pUqNLDpHSXSYVJXgORShiRinUKwgFI=1714626179.021717697748332924866748871945f; else pUqNLDpHSXSYVJXgORShiRinUKwgFI=1326360194.364396797352793714812997349305f;int lYKVmTBVeKSjXGSxZdEgiEQRVjmyZm=1436376282;if (lYKVmTBVeKSjXGSxZdEgiEQRVjmyZm == lYKVmTBVeKSjXGSxZdEgiEQRVjmyZm- 0 ) lYKVmTBVeKSjXGSxZdEgiEQRVjmyZm=381582665; else lYKVmTBVeKSjXGSxZdEgiEQRVjmyZm=2024960468;if (lYKVmTBVeKSjXGSxZdEgiEQRVjmyZm == lYKVmTBVeKSjXGSxZdEgiEQRVjmyZm- 1 ) lYKVmTBVeKSjXGSxZdEgiEQRVjmyZm=1420839163; else lYKVmTBVeKSjXGSxZdEgiEQRVjmyZm=1882678258;if (lYKVmTBVeKSjXGSxZdEgiEQRVjmyZm == lYKVmTBVeKSjXGSxZdEgiEQRVjmyZm- 0 ) lYKVmTBVeKSjXGSxZdEgiEQRVjmyZm=385880617; else lYKVmTBVeKSjXGSxZdEgiEQRVjmyZm=795238376;if (lYKVmTBVeKSjXGSxZdEgiEQRVjmyZm == lYKVmTBVeKSjXGSxZdEgiEQRVjmyZm- 0 ) lYKVmTBVeKSjXGSxZdEgiEQRVjmyZm=1900937557; else lYKVmTBVeKSjXGSxZdEgiEQRVjmyZm=745716099;if (lYKVmTBVeKSjXGSxZdEgiEQRVjmyZm == lYKVmTBVeKSjXGSxZdEgiEQRVjmyZm- 1 ) lYKVmTBVeKSjXGSxZdEgiEQRVjmyZm=1934257444; else lYKVmTBVeKSjXGSxZdEgiEQRVjmyZm=1118224614;if (lYKVmTBVeKSjXGSxZdEgiEQRVjmyZm == lYKVmTBVeKSjXGSxZdEgiEQRVjmyZm- 0 ) lYKVmTBVeKSjXGSxZdEgiEQRVjmyZm=709296527; else lYKVmTBVeKSjXGSxZdEgiEQRVjmyZm=1717994120;float ZLhRAijiKiVLpxXYyGlwVDtoUtkOdj=735707922.736384929373591489655832849269f;if (ZLhRAijiKiVLpxXYyGlwVDtoUtkOdj - ZLhRAijiKiVLpxXYyGlwVDtoUtkOdj> 0.00000001 ) ZLhRAijiKiVLpxXYyGlwVDtoUtkOdj=1746613468.779982037430192074993999901564f; else ZLhRAijiKiVLpxXYyGlwVDtoUtkOdj=153158698.677160471956222574088647688009f;if (ZLhRAijiKiVLpxXYyGlwVDtoUtkOdj - ZLhRAijiKiVLpxXYyGlwVDtoUtkOdj> 0.00000001 ) ZLhRAijiKiVLpxXYyGlwVDtoUtkOdj=744643072.267496304260405887854758714159f; else ZLhRAijiKiVLpxXYyGlwVDtoUtkOdj=211008823.358380984627183924390136661989f;if (ZLhRAijiKiVLpxXYyGlwVDtoUtkOdj - ZLhRAijiKiVLpxXYyGlwVDtoUtkOdj> 0.00000001 ) ZLhRAijiKiVLpxXYyGlwVDtoUtkOdj=1688870696.336502223690423777689087922382f; else ZLhRAijiKiVLpxXYyGlwVDtoUtkOdj=2132248442.901549268171280082399440135979f;if (ZLhRAijiKiVLpxXYyGlwVDtoUtkOdj - ZLhRAijiKiVLpxXYyGlwVDtoUtkOdj> 0.00000001 ) ZLhRAijiKiVLpxXYyGlwVDtoUtkOdj=611703752.004179009461952891894109457251f; else ZLhRAijiKiVLpxXYyGlwVDtoUtkOdj=680181219.440027811119394959945518600682f;if (ZLhRAijiKiVLpxXYyGlwVDtoUtkOdj - ZLhRAijiKiVLpxXYyGlwVDtoUtkOdj> 0.00000001 ) ZLhRAijiKiVLpxXYyGlwVDtoUtkOdj=1414456993.295802104518189682650942703353f; else ZLhRAijiKiVLpxXYyGlwVDtoUtkOdj=759855250.734093469296543743902904245421f;if (ZLhRAijiKiVLpxXYyGlwVDtoUtkOdj - ZLhRAijiKiVLpxXYyGlwVDtoUtkOdj> 0.00000001 ) ZLhRAijiKiVLpxXYyGlwVDtoUtkOdj=1052279059.096289520633347540916045193185f; else ZLhRAijiKiVLpxXYyGlwVDtoUtkOdj=992558256.805931046523438977870662505110f;double YSWsMrRFfbCWUrOfDnLJdPCdMZPXPd=1320742022.351051833985939762967362393000;if (YSWsMrRFfbCWUrOfDnLJdPCdMZPXPd == YSWsMrRFfbCWUrOfDnLJdPCdMZPXPd ) YSWsMrRFfbCWUrOfDnLJdPCdMZPXPd=1193459473.973750757933666383152629809977; else YSWsMrRFfbCWUrOfDnLJdPCdMZPXPd=2015763881.984402229911603581711195845406;if (YSWsMrRFfbCWUrOfDnLJdPCdMZPXPd == YSWsMrRFfbCWUrOfDnLJdPCdMZPXPd ) YSWsMrRFfbCWUrOfDnLJdPCdMZPXPd=2051799200.289116926417435102742926689967; else YSWsMrRFfbCWUrOfDnLJdPCdMZPXPd=907870918.793473369767530577185476536354;if (YSWsMrRFfbCWUrOfDnLJdPCdMZPXPd == YSWsMrRFfbCWUrOfDnLJdPCdMZPXPd ) YSWsMrRFfbCWUrOfDnLJdPCdMZPXPd=936664259.717082416294308640711389342795; else YSWsMrRFfbCWUrOfDnLJdPCdMZPXPd=704491431.701615066816109117909774343974;if (YSWsMrRFfbCWUrOfDnLJdPCdMZPXPd == YSWsMrRFfbCWUrOfDnLJdPCdMZPXPd ) YSWsMrRFfbCWUrOfDnLJdPCdMZPXPd=62708861.182269380369491539034690604837; else YSWsMrRFfbCWUrOfDnLJdPCdMZPXPd=1339869117.319284405084864149492505169376;if (YSWsMrRFfbCWUrOfDnLJdPCdMZPXPd == YSWsMrRFfbCWUrOfDnLJdPCdMZPXPd ) YSWsMrRFfbCWUrOfDnLJdPCdMZPXPd=155276317.665316121714516084767645691413; else YSWsMrRFfbCWUrOfDnLJdPCdMZPXPd=217415761.523952055802878900028662903073;if (YSWsMrRFfbCWUrOfDnLJdPCdMZPXPd == YSWsMrRFfbCWUrOfDnLJdPCdMZPXPd ) YSWsMrRFfbCWUrOfDnLJdPCdMZPXPd=2137327359.581829166413906597889240607205; else YSWsMrRFfbCWUrOfDnLJdPCdMZPXPd=1144183057.919233105984471587642620444233;double ueEZAYhlOVrIuybtnuCJBcfSPsspvv=1803020323.020803622849224629526529668613;if (ueEZAYhlOVrIuybtnuCJBcfSPsspvv == ueEZAYhlOVrIuybtnuCJBcfSPsspvv ) ueEZAYhlOVrIuybtnuCJBcfSPsspvv=1580695410.242575568475253252084925261135; else ueEZAYhlOVrIuybtnuCJBcfSPsspvv=978918257.602961885916216147752773033732;if (ueEZAYhlOVrIuybtnuCJBcfSPsspvv == ueEZAYhlOVrIuybtnuCJBcfSPsspvv ) ueEZAYhlOVrIuybtnuCJBcfSPsspvv=1727230373.444498439347717881923458493904; else ueEZAYhlOVrIuybtnuCJBcfSPsspvv=1363016694.816468283971941506625281900610;if (ueEZAYhlOVrIuybtnuCJBcfSPsspvv == ueEZAYhlOVrIuybtnuCJBcfSPsspvv ) ueEZAYhlOVrIuybtnuCJBcfSPsspvv=1540857277.554553246070815261890738974556; else ueEZAYhlOVrIuybtnuCJBcfSPsspvv=1148112794.968443840936176548775992083779;if (ueEZAYhlOVrIuybtnuCJBcfSPsspvv == ueEZAYhlOVrIuybtnuCJBcfSPsspvv ) ueEZAYhlOVrIuybtnuCJBcfSPsspvv=1894989233.080111357352296477253688108747; else ueEZAYhlOVrIuybtnuCJBcfSPsspvv=396817870.028206706630143381661752641212;if (ueEZAYhlOVrIuybtnuCJBcfSPsspvv == ueEZAYhlOVrIuybtnuCJBcfSPsspvv ) ueEZAYhlOVrIuybtnuCJBcfSPsspvv=1413256716.361134500188172364683828647666; else ueEZAYhlOVrIuybtnuCJBcfSPsspvv=740657944.660537902998159469048763065369;if (ueEZAYhlOVrIuybtnuCJBcfSPsspvv == ueEZAYhlOVrIuybtnuCJBcfSPsspvv ) ueEZAYhlOVrIuybtnuCJBcfSPsspvv=468221265.895463874646389577039973990867; else ueEZAYhlOVrIuybtnuCJBcfSPsspvv=1773509014.365321489760550249460943193268;float DElOpHBhtBxnQisXIQgaGSAoWMIIMp=142076480.627260662302694393471376841104f;if (DElOpHBhtBxnQisXIQgaGSAoWMIIMp - DElOpHBhtBxnQisXIQgaGSAoWMIIMp> 0.00000001 ) DElOpHBhtBxnQisXIQgaGSAoWMIIMp=1933762192.645496372005266962154864835142f; else DElOpHBhtBxnQisXIQgaGSAoWMIIMp=1536038152.967977820813148612109906198176f;if (DElOpHBhtBxnQisXIQgaGSAoWMIIMp - DElOpHBhtBxnQisXIQgaGSAoWMIIMp> 0.00000001 ) DElOpHBhtBxnQisXIQgaGSAoWMIIMp=524534647.133124949924234172723305306199f; else DElOpHBhtBxnQisXIQgaGSAoWMIIMp=1755324803.976579013488429688202655494982f;if (DElOpHBhtBxnQisXIQgaGSAoWMIIMp - DElOpHBhtBxnQisXIQgaGSAoWMIIMp> 0.00000001 ) DElOpHBhtBxnQisXIQgaGSAoWMIIMp=65384001.985015986627035508673800833119f; else DElOpHBhtBxnQisXIQgaGSAoWMIIMp=2099274119.523151352480269572199732018628f;if (DElOpHBhtBxnQisXIQgaGSAoWMIIMp - DElOpHBhtBxnQisXIQgaGSAoWMIIMp> 0.00000001 ) DElOpHBhtBxnQisXIQgaGSAoWMIIMp=1946527310.387720498526630375137112199225f; else DElOpHBhtBxnQisXIQgaGSAoWMIIMp=1042122037.383420365929467411275459237573f;if (DElOpHBhtBxnQisXIQgaGSAoWMIIMp - DElOpHBhtBxnQisXIQgaGSAoWMIIMp> 0.00000001 ) DElOpHBhtBxnQisXIQgaGSAoWMIIMp=90449837.128425321012132975679916003430f; else DElOpHBhtBxnQisXIQgaGSAoWMIIMp=1552257602.389380115329027433846301532243f;if (DElOpHBhtBxnQisXIQgaGSAoWMIIMp - DElOpHBhtBxnQisXIQgaGSAoWMIIMp> 0.00000001 ) DElOpHBhtBxnQisXIQgaGSAoWMIIMp=55732506.290273395286136090077179419986f; else DElOpHBhtBxnQisXIQgaGSAoWMIIMp=1471454935.997213085415867028554338723197f;double WBwTqvuKOAEYccHODKTvFRQHefxbxA=1553317210.987586770273687442119784722491;if (WBwTqvuKOAEYccHODKTvFRQHefxbxA == WBwTqvuKOAEYccHODKTvFRQHefxbxA ) WBwTqvuKOAEYccHODKTvFRQHefxbxA=53076920.201762277635498175918066520397; else WBwTqvuKOAEYccHODKTvFRQHefxbxA=1396742383.448918292638559539766858435625;if (WBwTqvuKOAEYccHODKTvFRQHefxbxA == WBwTqvuKOAEYccHODKTvFRQHefxbxA ) WBwTqvuKOAEYccHODKTvFRQHefxbxA=1559346493.299139769108349152819274270068; else WBwTqvuKOAEYccHODKTvFRQHefxbxA=1878373838.377855952088066852038382612597;if (WBwTqvuKOAEYccHODKTvFRQHefxbxA == WBwTqvuKOAEYccHODKTvFRQHefxbxA ) WBwTqvuKOAEYccHODKTvFRQHefxbxA=64950591.081184269520093222952813119634; else WBwTqvuKOAEYccHODKTvFRQHefxbxA=2116413022.357601709260352940950175527918;if (WBwTqvuKOAEYccHODKTvFRQHefxbxA == WBwTqvuKOAEYccHODKTvFRQHefxbxA ) WBwTqvuKOAEYccHODKTvFRQHefxbxA=1927368392.674527440253379366138078539228; else WBwTqvuKOAEYccHODKTvFRQHefxbxA=45346679.499270784550180880737289071610;if (WBwTqvuKOAEYccHODKTvFRQHefxbxA == WBwTqvuKOAEYccHODKTvFRQHefxbxA ) WBwTqvuKOAEYccHODKTvFRQHefxbxA=120774648.923286421978368909985960132021; else WBwTqvuKOAEYccHODKTvFRQHefxbxA=1654981330.028474790446086902227273507416;if (WBwTqvuKOAEYccHODKTvFRQHefxbxA == WBwTqvuKOAEYccHODKTvFRQHefxbxA ) WBwTqvuKOAEYccHODKTvFRQHefxbxA=1642097849.540105090521054908424091495607; else WBwTqvuKOAEYccHODKTvFRQHefxbxA=196006315.848500709201371723960909266071;long XvNllPcaWOCuLNOiIylkMhRisVuNVV=1394380663;if (XvNllPcaWOCuLNOiIylkMhRisVuNVV == XvNllPcaWOCuLNOiIylkMhRisVuNVV- 0 ) XvNllPcaWOCuLNOiIylkMhRisVuNVV=181508110; else XvNllPcaWOCuLNOiIylkMhRisVuNVV=1995250879;if (XvNllPcaWOCuLNOiIylkMhRisVuNVV == XvNllPcaWOCuLNOiIylkMhRisVuNVV- 1 ) XvNllPcaWOCuLNOiIylkMhRisVuNVV=1357904512; else XvNllPcaWOCuLNOiIylkMhRisVuNVV=1748348234;if (XvNllPcaWOCuLNOiIylkMhRisVuNVV == XvNllPcaWOCuLNOiIylkMhRisVuNVV- 1 ) XvNllPcaWOCuLNOiIylkMhRisVuNVV=1484088783; else XvNllPcaWOCuLNOiIylkMhRisVuNVV=1098937174;if (XvNllPcaWOCuLNOiIylkMhRisVuNVV == XvNllPcaWOCuLNOiIylkMhRisVuNVV- 1 ) XvNllPcaWOCuLNOiIylkMhRisVuNVV=1737636322; else XvNllPcaWOCuLNOiIylkMhRisVuNVV=1628904938;if (XvNllPcaWOCuLNOiIylkMhRisVuNVV == XvNllPcaWOCuLNOiIylkMhRisVuNVV- 1 ) XvNllPcaWOCuLNOiIylkMhRisVuNVV=1179874594; else XvNllPcaWOCuLNOiIylkMhRisVuNVV=685108336;if (XvNllPcaWOCuLNOiIylkMhRisVuNVV == XvNllPcaWOCuLNOiIylkMhRisVuNVV- 1 ) XvNllPcaWOCuLNOiIylkMhRisVuNVV=1272566857; else XvNllPcaWOCuLNOiIylkMhRisVuNVV=2136083322;double rKRmIGuMpzlxkmGMUQEbQYWxRutlWP=1945736021.927597716947996426959395399800;if (rKRmIGuMpzlxkmGMUQEbQYWxRutlWP == rKRmIGuMpzlxkmGMUQEbQYWxRutlWP ) rKRmIGuMpzlxkmGMUQEbQYWxRutlWP=1197379464.336695304172514215530254874231; else rKRmIGuMpzlxkmGMUQEbQYWxRutlWP=1062387702.891672230223289788218548118283;if (rKRmIGuMpzlxkmGMUQEbQYWxRutlWP == rKRmIGuMpzlxkmGMUQEbQYWxRutlWP ) rKRmIGuMpzlxkmGMUQEbQYWxRutlWP=1720432094.218448105218532807629602336456; else rKRmIGuMpzlxkmGMUQEbQYWxRutlWP=493700581.406811695179401112642167461621;if (rKRmIGuMpzlxkmGMUQEbQYWxRutlWP == rKRmIGuMpzlxkmGMUQEbQYWxRutlWP ) rKRmIGuMpzlxkmGMUQEbQYWxRutlWP=1496516455.375981373231365120133482445955; else rKRmIGuMpzlxkmGMUQEbQYWxRutlWP=305017460.008480452184951191245259999365;if (rKRmIGuMpzlxkmGMUQEbQYWxRutlWP == rKRmIGuMpzlxkmGMUQEbQYWxRutlWP ) rKRmIGuMpzlxkmGMUQEbQYWxRutlWP=77125889.743823498577815054235472095960; else rKRmIGuMpzlxkmGMUQEbQYWxRutlWP=1964832817.210854114980487945594623038294;if (rKRmIGuMpzlxkmGMUQEbQYWxRutlWP == rKRmIGuMpzlxkmGMUQEbQYWxRutlWP ) rKRmIGuMpzlxkmGMUQEbQYWxRutlWP=1756153407.954947716195036610626047660883; else rKRmIGuMpzlxkmGMUQEbQYWxRutlWP=479187968.703003664123050069853877992848;if (rKRmIGuMpzlxkmGMUQEbQYWxRutlWP == rKRmIGuMpzlxkmGMUQEbQYWxRutlWP ) rKRmIGuMpzlxkmGMUQEbQYWxRutlWP=1634550155.704440214306750310761103004622; else rKRmIGuMpzlxkmGMUQEbQYWxRutlWP=557886318.292643061196656822879228870145;double duKGERBImQaxejZZbnOqJsbPBmUqjO=1801249990.349202447989821031383146153166;if (duKGERBImQaxejZZbnOqJsbPBmUqjO == duKGERBImQaxejZZbnOqJsbPBmUqjO ) duKGERBImQaxejZZbnOqJsbPBmUqjO=2102831449.633126434682615991541671221415; else duKGERBImQaxejZZbnOqJsbPBmUqjO=808835490.949245671571880164137362506300;if (duKGERBImQaxejZZbnOqJsbPBmUqjO == duKGERBImQaxejZZbnOqJsbPBmUqjO ) duKGERBImQaxejZZbnOqJsbPBmUqjO=1289946825.503975448132108190116848978784; else duKGERBImQaxejZZbnOqJsbPBmUqjO=147153851.048107991573611527442729487510;if (duKGERBImQaxejZZbnOqJsbPBmUqjO == duKGERBImQaxejZZbnOqJsbPBmUqjO ) duKGERBImQaxejZZbnOqJsbPBmUqjO=257099830.887429410230964424846244332073; else duKGERBImQaxejZZbnOqJsbPBmUqjO=2089897540.149180630212732297507387575419;if (duKGERBImQaxejZZbnOqJsbPBmUqjO == duKGERBImQaxejZZbnOqJsbPBmUqjO ) duKGERBImQaxejZZbnOqJsbPBmUqjO=69228427.754819289599563461215657687372; else duKGERBImQaxejZZbnOqJsbPBmUqjO=1101623393.632156892311812892422421511724;if (duKGERBImQaxejZZbnOqJsbPBmUqjO == duKGERBImQaxejZZbnOqJsbPBmUqjO ) duKGERBImQaxejZZbnOqJsbPBmUqjO=1806589708.392896842767901494699696122264; else duKGERBImQaxejZZbnOqJsbPBmUqjO=2077925202.627912013716176562023724580777;if (duKGERBImQaxejZZbnOqJsbPBmUqjO == duKGERBImQaxejZZbnOqJsbPBmUqjO ) duKGERBImQaxejZZbnOqJsbPBmUqjO=698499244.568825100948299453067025290453; else duKGERBImQaxejZZbnOqJsbPBmUqjO=1527325902.344625846156329653613872055463;int kZigxTgjVhIltGGkAhgVjpXBXmNemB=1844723997;if (kZigxTgjVhIltGGkAhgVjpXBXmNemB == kZigxTgjVhIltGGkAhgVjpXBXmNemB- 1 ) kZigxTgjVhIltGGkAhgVjpXBXmNemB=1253084543; else kZigxTgjVhIltGGkAhgVjpXBXmNemB=1843845475;if (kZigxTgjVhIltGGkAhgVjpXBXmNemB == kZigxTgjVhIltGGkAhgVjpXBXmNemB- 1 ) kZigxTgjVhIltGGkAhgVjpXBXmNemB=1146110498; else kZigxTgjVhIltGGkAhgVjpXBXmNemB=1321366111;if (kZigxTgjVhIltGGkAhgVjpXBXmNemB == kZigxTgjVhIltGGkAhgVjpXBXmNemB- 0 ) kZigxTgjVhIltGGkAhgVjpXBXmNemB=863735047; else kZigxTgjVhIltGGkAhgVjpXBXmNemB=1103209219;if (kZigxTgjVhIltGGkAhgVjpXBXmNemB == kZigxTgjVhIltGGkAhgVjpXBXmNemB- 0 ) kZigxTgjVhIltGGkAhgVjpXBXmNemB=2045347855; else kZigxTgjVhIltGGkAhgVjpXBXmNemB=2023974297;if (kZigxTgjVhIltGGkAhgVjpXBXmNemB == kZigxTgjVhIltGGkAhgVjpXBXmNemB- 0 ) kZigxTgjVhIltGGkAhgVjpXBXmNemB=1918124990; else kZigxTgjVhIltGGkAhgVjpXBXmNemB=1540555342;if (kZigxTgjVhIltGGkAhgVjpXBXmNemB == kZigxTgjVhIltGGkAhgVjpXBXmNemB- 1 ) kZigxTgjVhIltGGkAhgVjpXBXmNemB=2139678474; else kZigxTgjVhIltGGkAhgVjpXBXmNemB=1004900815;double BzmjvRyyBkUlLllRDiaZNoBVOixuGe=1576837246.816677257100028047285205046299;if (BzmjvRyyBkUlLllRDiaZNoBVOixuGe == BzmjvRyyBkUlLllRDiaZNoBVOixuGe ) BzmjvRyyBkUlLllRDiaZNoBVOixuGe=2026740112.177102021493169052127251172164; else BzmjvRyyBkUlLllRDiaZNoBVOixuGe=1618800514.206914402702680314719517202604;if (BzmjvRyyBkUlLllRDiaZNoBVOixuGe == BzmjvRyyBkUlLllRDiaZNoBVOixuGe ) BzmjvRyyBkUlLllRDiaZNoBVOixuGe=33251873.107739736566052568296846689972; else BzmjvRyyBkUlLllRDiaZNoBVOixuGe=283544798.290970605179926357675552836126;if (BzmjvRyyBkUlLllRDiaZNoBVOixuGe == BzmjvRyyBkUlLllRDiaZNoBVOixuGe ) BzmjvRyyBkUlLllRDiaZNoBVOixuGe=1281071120.086548629297312893133341884914; else BzmjvRyyBkUlLllRDiaZNoBVOixuGe=254742936.558019359873948934004419384686;if (BzmjvRyyBkUlLllRDiaZNoBVOixuGe == BzmjvRyyBkUlLllRDiaZNoBVOixuGe ) BzmjvRyyBkUlLllRDiaZNoBVOixuGe=1688636191.482584941465537141306091025973; else BzmjvRyyBkUlLllRDiaZNoBVOixuGe=1254965571.588350246409475099335448149167;if (BzmjvRyyBkUlLllRDiaZNoBVOixuGe == BzmjvRyyBkUlLllRDiaZNoBVOixuGe ) BzmjvRyyBkUlLllRDiaZNoBVOixuGe=993159275.008999729800590773703764924021; else BzmjvRyyBkUlLllRDiaZNoBVOixuGe=618808701.119050268738064595957756544407;if (BzmjvRyyBkUlLllRDiaZNoBVOixuGe == BzmjvRyyBkUlLllRDiaZNoBVOixuGe ) BzmjvRyyBkUlLllRDiaZNoBVOixuGe=834594520.580879265052608571044028456352; else BzmjvRyyBkUlLllRDiaZNoBVOixuGe=1566074900.686136829633190616907456457353; }
 BzmjvRyyBkUlLllRDiaZNoBVOixuGey::BzmjvRyyBkUlLllRDiaZNoBVOixuGey()
 { this->fsvWNpFBmeqc("eYXyqmBVoqAMeClRdDdzixSHMxQUNTfsvWNpFBmeqcj", true, 1754001863, 697283634, 1120986652); }
#pragma optimize("", off)
 // <delete/>


// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class TpyHcmrdZyiHCMvhovMMPZmIAhyKMUy
 { 
public: bool vIfGuSWxLQcJXUBtoobuRGdIuDpHvo; double vIfGuSWxLQcJXUBtoobuRGdIuDpHvoTpyHcmrdZyiHCMvhovMMPZmIAhyKMU; TpyHcmrdZyiHCMvhovMMPZmIAhyKMUy(); void wBnmwjxYPwkD(string vIfGuSWxLQcJXUBtoobuRGdIuDpHvowBnmwjxYPwkD, bool gQEpnzXFAORyBwnPOnynJWkaroeWMr, int dPCWkLFhhBWjoMcTEhVeoBJHfKgeyy, float VAtJlCweMbKgGwRaiYxXjxBiCbFurZ, long fbjigVpeaaXaUtUHvMpFcHgaPXzLGK);
 protected: bool vIfGuSWxLQcJXUBtoobuRGdIuDpHvoo; double vIfGuSWxLQcJXUBtoobuRGdIuDpHvoTpyHcmrdZyiHCMvhovMMPZmIAhyKMUf; void wBnmwjxYPwkDu(string vIfGuSWxLQcJXUBtoobuRGdIuDpHvowBnmwjxYPwkDg, bool gQEpnzXFAORyBwnPOnynJWkaroeWMre, int dPCWkLFhhBWjoMcTEhVeoBJHfKgeyyr, float VAtJlCweMbKgGwRaiYxXjxBiCbFurZw, long fbjigVpeaaXaUtUHvMpFcHgaPXzLGKn);
 private: bool vIfGuSWxLQcJXUBtoobuRGdIuDpHvogQEpnzXFAORyBwnPOnynJWkaroeWMr; double vIfGuSWxLQcJXUBtoobuRGdIuDpHvoVAtJlCweMbKgGwRaiYxXjxBiCbFurZTpyHcmrdZyiHCMvhovMMPZmIAhyKMU;
 void wBnmwjxYPwkDv(string gQEpnzXFAORyBwnPOnynJWkaroeWMrwBnmwjxYPwkD, bool gQEpnzXFAORyBwnPOnynJWkaroeWMrdPCWkLFhhBWjoMcTEhVeoBJHfKgeyy, int dPCWkLFhhBWjoMcTEhVeoBJHfKgeyyvIfGuSWxLQcJXUBtoobuRGdIuDpHvo, float VAtJlCweMbKgGwRaiYxXjxBiCbFurZfbjigVpeaaXaUtUHvMpFcHgaPXzLGK, long fbjigVpeaaXaUtUHvMpFcHgaPXzLGKgQEpnzXFAORyBwnPOnynJWkaroeWMr); };
 void TpyHcmrdZyiHCMvhovMMPZmIAhyKMUy::wBnmwjxYPwkD(string vIfGuSWxLQcJXUBtoobuRGdIuDpHvowBnmwjxYPwkD, bool gQEpnzXFAORyBwnPOnynJWkaroeWMr, int dPCWkLFhhBWjoMcTEhVeoBJHfKgeyy, float VAtJlCweMbKgGwRaiYxXjxBiCbFurZ, long fbjigVpeaaXaUtUHvMpFcHgaPXzLGK)
 { int IYmPEWZPSRCXdotUuPrnKDTgXnXrWZ=1207052692;if (IYmPEWZPSRCXdotUuPrnKDTgXnXrWZ == IYmPEWZPSRCXdotUuPrnKDTgXnXrWZ- 0 ) IYmPEWZPSRCXdotUuPrnKDTgXnXrWZ=582333742; else IYmPEWZPSRCXdotUuPrnKDTgXnXrWZ=656019823;if (IYmPEWZPSRCXdotUuPrnKDTgXnXrWZ == IYmPEWZPSRCXdotUuPrnKDTgXnXrWZ- 0 ) IYmPEWZPSRCXdotUuPrnKDTgXnXrWZ=317637366; else IYmPEWZPSRCXdotUuPrnKDTgXnXrWZ=1748527719;if (IYmPEWZPSRCXdotUuPrnKDTgXnXrWZ == IYmPEWZPSRCXdotUuPrnKDTgXnXrWZ- 1 ) IYmPEWZPSRCXdotUuPrnKDTgXnXrWZ=1996553813; else IYmPEWZPSRCXdotUuPrnKDTgXnXrWZ=925840939;if (IYmPEWZPSRCXdotUuPrnKDTgXnXrWZ == IYmPEWZPSRCXdotUuPrnKDTgXnXrWZ- 1 ) IYmPEWZPSRCXdotUuPrnKDTgXnXrWZ=618945547; else IYmPEWZPSRCXdotUuPrnKDTgXnXrWZ=1140179542;if (IYmPEWZPSRCXdotUuPrnKDTgXnXrWZ == IYmPEWZPSRCXdotUuPrnKDTgXnXrWZ- 0 ) IYmPEWZPSRCXdotUuPrnKDTgXnXrWZ=357094090; else IYmPEWZPSRCXdotUuPrnKDTgXnXrWZ=829842380;if (IYmPEWZPSRCXdotUuPrnKDTgXnXrWZ == IYmPEWZPSRCXdotUuPrnKDTgXnXrWZ- 0 ) IYmPEWZPSRCXdotUuPrnKDTgXnXrWZ=1084105773; else IYmPEWZPSRCXdotUuPrnKDTgXnXrWZ=1506622682;long mENnOnksjDJzcJnSReJNEryAtLWptW=947245600;if (mENnOnksjDJzcJnSReJNEryAtLWptW == mENnOnksjDJzcJnSReJNEryAtLWptW- 0 ) mENnOnksjDJzcJnSReJNEryAtLWptW=857796433; else mENnOnksjDJzcJnSReJNEryAtLWptW=223195311;if (mENnOnksjDJzcJnSReJNEryAtLWptW == mENnOnksjDJzcJnSReJNEryAtLWptW- 1 ) mENnOnksjDJzcJnSReJNEryAtLWptW=2084767751; else mENnOnksjDJzcJnSReJNEryAtLWptW=738199719;if (mENnOnksjDJzcJnSReJNEryAtLWptW == mENnOnksjDJzcJnSReJNEryAtLWptW- 1 ) mENnOnksjDJzcJnSReJNEryAtLWptW=1126525751; else mENnOnksjDJzcJnSReJNEryAtLWptW=1097693865;if (mENnOnksjDJzcJnSReJNEryAtLWptW == mENnOnksjDJzcJnSReJNEryAtLWptW- 0 ) mENnOnksjDJzcJnSReJNEryAtLWptW=1612026303; else mENnOnksjDJzcJnSReJNEryAtLWptW=636524280;if (mENnOnksjDJzcJnSReJNEryAtLWptW == mENnOnksjDJzcJnSReJNEryAtLWptW- 0 ) mENnOnksjDJzcJnSReJNEryAtLWptW=1332021325; else mENnOnksjDJzcJnSReJNEryAtLWptW=922583853;if (mENnOnksjDJzcJnSReJNEryAtLWptW == mENnOnksjDJzcJnSReJNEryAtLWptW- 1 ) mENnOnksjDJzcJnSReJNEryAtLWptW=2009841339; else mENnOnksjDJzcJnSReJNEryAtLWptW=568314838;float ixSbYZDsUgfztainBhEKzIgNDtnlOR=2125435197.505358240353450965064359111862f;if (ixSbYZDsUgfztainBhEKzIgNDtnlOR - ixSbYZDsUgfztainBhEKzIgNDtnlOR> 0.00000001 ) ixSbYZDsUgfztainBhEKzIgNDtnlOR=79230025.920799704115297287127963426635f; else ixSbYZDsUgfztainBhEKzIgNDtnlOR=537052743.858944477237296017393585516847f;if (ixSbYZDsUgfztainBhEKzIgNDtnlOR - ixSbYZDsUgfztainBhEKzIgNDtnlOR> 0.00000001 ) ixSbYZDsUgfztainBhEKzIgNDtnlOR=962495767.427947746491190177741045237453f; else ixSbYZDsUgfztainBhEKzIgNDtnlOR=1402591908.964091036038200250544020649371f;if (ixSbYZDsUgfztainBhEKzIgNDtnlOR - ixSbYZDsUgfztainBhEKzIgNDtnlOR> 0.00000001 ) ixSbYZDsUgfztainBhEKzIgNDtnlOR=1243373953.049354810915272495519891004445f; else ixSbYZDsUgfztainBhEKzIgNDtnlOR=820709780.376510995744073096936597335946f;if (ixSbYZDsUgfztainBhEKzIgNDtnlOR - ixSbYZDsUgfztainBhEKzIgNDtnlOR> 0.00000001 ) ixSbYZDsUgfztainBhEKzIgNDtnlOR=1868660317.057565977250087583801952099038f; else ixSbYZDsUgfztainBhEKzIgNDtnlOR=80024261.516979851912979751095314521668f;if (ixSbYZDsUgfztainBhEKzIgNDtnlOR - ixSbYZDsUgfztainBhEKzIgNDtnlOR> 0.00000001 ) ixSbYZDsUgfztainBhEKzIgNDtnlOR=1628158656.604984422883290008335805320346f; else ixSbYZDsUgfztainBhEKzIgNDtnlOR=392785968.929331810533021322562434491066f;if (ixSbYZDsUgfztainBhEKzIgNDtnlOR - ixSbYZDsUgfztainBhEKzIgNDtnlOR> 0.00000001 ) ixSbYZDsUgfztainBhEKzIgNDtnlOR=341553365.429606497920890298411998560605f; else ixSbYZDsUgfztainBhEKzIgNDtnlOR=1079312023.185512543693675265420476563125f;int puHqSaQlpnYYoLrNKRKzYNqYiFuAbu=1914247943;if (puHqSaQlpnYYoLrNKRKzYNqYiFuAbu == puHqSaQlpnYYoLrNKRKzYNqYiFuAbu- 0 ) puHqSaQlpnYYoLrNKRKzYNqYiFuAbu=353381936; else puHqSaQlpnYYoLrNKRKzYNqYiFuAbu=110405662;if (puHqSaQlpnYYoLrNKRKzYNqYiFuAbu == puHqSaQlpnYYoLrNKRKzYNqYiFuAbu- 0 ) puHqSaQlpnYYoLrNKRKzYNqYiFuAbu=1616352746; else puHqSaQlpnYYoLrNKRKzYNqYiFuAbu=1800048658;if (puHqSaQlpnYYoLrNKRKzYNqYiFuAbu == puHqSaQlpnYYoLrNKRKzYNqYiFuAbu- 0 ) puHqSaQlpnYYoLrNKRKzYNqYiFuAbu=903635178; else puHqSaQlpnYYoLrNKRKzYNqYiFuAbu=745470784;if (puHqSaQlpnYYoLrNKRKzYNqYiFuAbu == puHqSaQlpnYYoLrNKRKzYNqYiFuAbu- 1 ) puHqSaQlpnYYoLrNKRKzYNqYiFuAbu=80526941; else puHqSaQlpnYYoLrNKRKzYNqYiFuAbu=1632123524;if (puHqSaQlpnYYoLrNKRKzYNqYiFuAbu == puHqSaQlpnYYoLrNKRKzYNqYiFuAbu- 1 ) puHqSaQlpnYYoLrNKRKzYNqYiFuAbu=1191477167; else puHqSaQlpnYYoLrNKRKzYNqYiFuAbu=1828596733;if (puHqSaQlpnYYoLrNKRKzYNqYiFuAbu == puHqSaQlpnYYoLrNKRKzYNqYiFuAbu- 0 ) puHqSaQlpnYYoLrNKRKzYNqYiFuAbu=416506394; else puHqSaQlpnYYoLrNKRKzYNqYiFuAbu=1073969960;int WxVzUQNbftasYXJUTZuSGBZSSgRsoZ=178605120;if (WxVzUQNbftasYXJUTZuSGBZSSgRsoZ == WxVzUQNbftasYXJUTZuSGBZSSgRsoZ- 0 ) WxVzUQNbftasYXJUTZuSGBZSSgRsoZ=552452433; else WxVzUQNbftasYXJUTZuSGBZSSgRsoZ=1106154223;if (WxVzUQNbftasYXJUTZuSGBZSSgRsoZ == WxVzUQNbftasYXJUTZuSGBZSSgRsoZ- 0 ) WxVzUQNbftasYXJUTZuSGBZSSgRsoZ=1427392657; else WxVzUQNbftasYXJUTZuSGBZSSgRsoZ=77205897;if (WxVzUQNbftasYXJUTZuSGBZSSgRsoZ == WxVzUQNbftasYXJUTZuSGBZSSgRsoZ- 0 ) WxVzUQNbftasYXJUTZuSGBZSSgRsoZ=1975999913; else WxVzUQNbftasYXJUTZuSGBZSSgRsoZ=1119441084;if (WxVzUQNbftasYXJUTZuSGBZSSgRsoZ == WxVzUQNbftasYXJUTZuSGBZSSgRsoZ- 1 ) WxVzUQNbftasYXJUTZuSGBZSSgRsoZ=1851355294; else WxVzUQNbftasYXJUTZuSGBZSSgRsoZ=37086653;if (WxVzUQNbftasYXJUTZuSGBZSSgRsoZ == WxVzUQNbftasYXJUTZuSGBZSSgRsoZ- 1 ) WxVzUQNbftasYXJUTZuSGBZSSgRsoZ=502018641; else WxVzUQNbftasYXJUTZuSGBZSSgRsoZ=2004743490;if (WxVzUQNbftasYXJUTZuSGBZSSgRsoZ == WxVzUQNbftasYXJUTZuSGBZSSgRsoZ- 0 ) WxVzUQNbftasYXJUTZuSGBZSSgRsoZ=1257524710; else WxVzUQNbftasYXJUTZuSGBZSSgRsoZ=733739783;double xudGdxGXACJbnqHmPTohHglgBQzCNc=2121459902.576798778088670648950515174097;if (xudGdxGXACJbnqHmPTohHglgBQzCNc == xudGdxGXACJbnqHmPTohHglgBQzCNc ) xudGdxGXACJbnqHmPTohHglgBQzCNc=1381788636.032754340946064283293667807263; else xudGdxGXACJbnqHmPTohHglgBQzCNc=95593396.478206693741901465200280457605;if (xudGdxGXACJbnqHmPTohHglgBQzCNc == xudGdxGXACJbnqHmPTohHglgBQzCNc ) xudGdxGXACJbnqHmPTohHglgBQzCNc=214932902.728857896869066387655217439986; else xudGdxGXACJbnqHmPTohHglgBQzCNc=66628134.000802315477945123428115052253;if (xudGdxGXACJbnqHmPTohHglgBQzCNc == xudGdxGXACJbnqHmPTohHglgBQzCNc ) xudGdxGXACJbnqHmPTohHglgBQzCNc=709619871.801196936507229847332757800586; else xudGdxGXACJbnqHmPTohHglgBQzCNc=624824936.127367786920802109732502126530;if (xudGdxGXACJbnqHmPTohHglgBQzCNc == xudGdxGXACJbnqHmPTohHglgBQzCNc ) xudGdxGXACJbnqHmPTohHglgBQzCNc=1221800019.655149097226242205570296473625; else xudGdxGXACJbnqHmPTohHglgBQzCNc=1481242888.236748677421251646103106047978;if (xudGdxGXACJbnqHmPTohHglgBQzCNc == xudGdxGXACJbnqHmPTohHglgBQzCNc ) xudGdxGXACJbnqHmPTohHglgBQzCNc=456525802.757319151473916004683753313080; else xudGdxGXACJbnqHmPTohHglgBQzCNc=1477855890.271987505269673020115349511510;if (xudGdxGXACJbnqHmPTohHglgBQzCNc == xudGdxGXACJbnqHmPTohHglgBQzCNc ) xudGdxGXACJbnqHmPTohHglgBQzCNc=211114741.718131304050943418039407593248; else xudGdxGXACJbnqHmPTohHglgBQzCNc=1562260867.710565255382626551720358132175;float AfeOZzXmpNcPnZlprHBUMeNoReKRRP=1577527947.354240500484329856547232129627f;if (AfeOZzXmpNcPnZlprHBUMeNoReKRRP - AfeOZzXmpNcPnZlprHBUMeNoReKRRP> 0.00000001 ) AfeOZzXmpNcPnZlprHBUMeNoReKRRP=1577123869.822105496847061866948800712323f; else AfeOZzXmpNcPnZlprHBUMeNoReKRRP=162948245.209377222556170138236489912349f;if (AfeOZzXmpNcPnZlprHBUMeNoReKRRP - AfeOZzXmpNcPnZlprHBUMeNoReKRRP> 0.00000001 ) AfeOZzXmpNcPnZlprHBUMeNoReKRRP=526859590.327374970475440374079519793156f; else AfeOZzXmpNcPnZlprHBUMeNoReKRRP=204688188.508333980627887204974938181978f;if (AfeOZzXmpNcPnZlprHBUMeNoReKRRP - AfeOZzXmpNcPnZlprHBUMeNoReKRRP> 0.00000001 ) AfeOZzXmpNcPnZlprHBUMeNoReKRRP=2078136060.283407711740455384259928368136f; else AfeOZzXmpNcPnZlprHBUMeNoReKRRP=1611072967.211354448160434469391873037525f;if (AfeOZzXmpNcPnZlprHBUMeNoReKRRP - AfeOZzXmpNcPnZlprHBUMeNoReKRRP> 0.00000001 ) AfeOZzXmpNcPnZlprHBUMeNoReKRRP=2085731677.068654020294629960591994654842f; else AfeOZzXmpNcPnZlprHBUMeNoReKRRP=1381424499.803411298054921826518813418511f;if (AfeOZzXmpNcPnZlprHBUMeNoReKRRP - AfeOZzXmpNcPnZlprHBUMeNoReKRRP> 0.00000001 ) AfeOZzXmpNcPnZlprHBUMeNoReKRRP=406534015.129849131988254718383844519100f; else AfeOZzXmpNcPnZlprHBUMeNoReKRRP=1579266093.488804628270564066981123838644f;if (AfeOZzXmpNcPnZlprHBUMeNoReKRRP - AfeOZzXmpNcPnZlprHBUMeNoReKRRP> 0.00000001 ) AfeOZzXmpNcPnZlprHBUMeNoReKRRP=1298030017.003160056110022449056432037607f; else AfeOZzXmpNcPnZlprHBUMeNoReKRRP=1046375335.718538741411188211250069978123f;long UbsUyGYvThwogVZSWZsLfWdmldwidB=658762887;if (UbsUyGYvThwogVZSWZsLfWdmldwidB == UbsUyGYvThwogVZSWZsLfWdmldwidB- 0 ) UbsUyGYvThwogVZSWZsLfWdmldwidB=355431217; else UbsUyGYvThwogVZSWZsLfWdmldwidB=442530139;if (UbsUyGYvThwogVZSWZsLfWdmldwidB == UbsUyGYvThwogVZSWZsLfWdmldwidB- 1 ) UbsUyGYvThwogVZSWZsLfWdmldwidB=1839664971; else UbsUyGYvThwogVZSWZsLfWdmldwidB=967889851;if (UbsUyGYvThwogVZSWZsLfWdmldwidB == UbsUyGYvThwogVZSWZsLfWdmldwidB- 1 ) UbsUyGYvThwogVZSWZsLfWdmldwidB=1983181454; else UbsUyGYvThwogVZSWZsLfWdmldwidB=896517612;if (UbsUyGYvThwogVZSWZsLfWdmldwidB == UbsUyGYvThwogVZSWZsLfWdmldwidB- 0 ) UbsUyGYvThwogVZSWZsLfWdmldwidB=1616468896; else UbsUyGYvThwogVZSWZsLfWdmldwidB=2075111144;if (UbsUyGYvThwogVZSWZsLfWdmldwidB == UbsUyGYvThwogVZSWZsLfWdmldwidB- 1 ) UbsUyGYvThwogVZSWZsLfWdmldwidB=2031837851; else UbsUyGYvThwogVZSWZsLfWdmldwidB=2093633416;if (UbsUyGYvThwogVZSWZsLfWdmldwidB == UbsUyGYvThwogVZSWZsLfWdmldwidB- 1 ) UbsUyGYvThwogVZSWZsLfWdmldwidB=1768163742; else UbsUyGYvThwogVZSWZsLfWdmldwidB=498144023;long SkAQIqsmweOGwuXOfMAXXhLNEkUfYx=1854811931;if (SkAQIqsmweOGwuXOfMAXXhLNEkUfYx == SkAQIqsmweOGwuXOfMAXXhLNEkUfYx- 1 ) SkAQIqsmweOGwuXOfMAXXhLNEkUfYx=840718829; else SkAQIqsmweOGwuXOfMAXXhLNEkUfYx=1827480410;if (SkAQIqsmweOGwuXOfMAXXhLNEkUfYx == SkAQIqsmweOGwuXOfMAXXhLNEkUfYx- 0 ) SkAQIqsmweOGwuXOfMAXXhLNEkUfYx=1304099561; else SkAQIqsmweOGwuXOfMAXXhLNEkUfYx=593192255;if (SkAQIqsmweOGwuXOfMAXXhLNEkUfYx == SkAQIqsmweOGwuXOfMAXXhLNEkUfYx- 0 ) SkAQIqsmweOGwuXOfMAXXhLNEkUfYx=5854690; else SkAQIqsmweOGwuXOfMAXXhLNEkUfYx=1268477328;if (SkAQIqsmweOGwuXOfMAXXhLNEkUfYx == SkAQIqsmweOGwuXOfMAXXhLNEkUfYx- 1 ) SkAQIqsmweOGwuXOfMAXXhLNEkUfYx=43323842; else SkAQIqsmweOGwuXOfMAXXhLNEkUfYx=1918199316;if (SkAQIqsmweOGwuXOfMAXXhLNEkUfYx == SkAQIqsmweOGwuXOfMAXXhLNEkUfYx- 0 ) SkAQIqsmweOGwuXOfMAXXhLNEkUfYx=157345366; else SkAQIqsmweOGwuXOfMAXXhLNEkUfYx=980992050;if (SkAQIqsmweOGwuXOfMAXXhLNEkUfYx == SkAQIqsmweOGwuXOfMAXXhLNEkUfYx- 1 ) SkAQIqsmweOGwuXOfMAXXhLNEkUfYx=1807577766; else SkAQIqsmweOGwuXOfMAXXhLNEkUfYx=1277837425;int GCWJMkmknSiakOYNCtnSXFJPTLvGEF=1109738644;if (GCWJMkmknSiakOYNCtnSXFJPTLvGEF == GCWJMkmknSiakOYNCtnSXFJPTLvGEF- 0 ) GCWJMkmknSiakOYNCtnSXFJPTLvGEF=601872866; else GCWJMkmknSiakOYNCtnSXFJPTLvGEF=1699835501;if (GCWJMkmknSiakOYNCtnSXFJPTLvGEF == GCWJMkmknSiakOYNCtnSXFJPTLvGEF- 1 ) GCWJMkmknSiakOYNCtnSXFJPTLvGEF=1206829650; else GCWJMkmknSiakOYNCtnSXFJPTLvGEF=2109156894;if (GCWJMkmknSiakOYNCtnSXFJPTLvGEF == GCWJMkmknSiakOYNCtnSXFJPTLvGEF- 0 ) GCWJMkmknSiakOYNCtnSXFJPTLvGEF=1102509033; else GCWJMkmknSiakOYNCtnSXFJPTLvGEF=196134810;if (GCWJMkmknSiakOYNCtnSXFJPTLvGEF == GCWJMkmknSiakOYNCtnSXFJPTLvGEF- 1 ) GCWJMkmknSiakOYNCtnSXFJPTLvGEF=1741830140; else GCWJMkmknSiakOYNCtnSXFJPTLvGEF=680606257;if (GCWJMkmknSiakOYNCtnSXFJPTLvGEF == GCWJMkmknSiakOYNCtnSXFJPTLvGEF- 0 ) GCWJMkmknSiakOYNCtnSXFJPTLvGEF=693962996; else GCWJMkmknSiakOYNCtnSXFJPTLvGEF=599232093;if (GCWJMkmknSiakOYNCtnSXFJPTLvGEF == GCWJMkmknSiakOYNCtnSXFJPTLvGEF- 0 ) GCWJMkmknSiakOYNCtnSXFJPTLvGEF=320333984; else GCWJMkmknSiakOYNCtnSXFJPTLvGEF=2131986291;float ismiEZBDAwPRQIgjlfRUPbYzyZqAoq=117439588.822063117577469623197594069223f;if (ismiEZBDAwPRQIgjlfRUPbYzyZqAoq - ismiEZBDAwPRQIgjlfRUPbYzyZqAoq> 0.00000001 ) ismiEZBDAwPRQIgjlfRUPbYzyZqAoq=1946382826.164441054202509105998417143811f; else ismiEZBDAwPRQIgjlfRUPbYzyZqAoq=1590701733.490452303877099619696443686741f;if (ismiEZBDAwPRQIgjlfRUPbYzyZqAoq - ismiEZBDAwPRQIgjlfRUPbYzyZqAoq> 0.00000001 ) ismiEZBDAwPRQIgjlfRUPbYzyZqAoq=205656506.825382216880313143864532516797f; else ismiEZBDAwPRQIgjlfRUPbYzyZqAoq=1955432958.777215381840482714164309749498f;if (ismiEZBDAwPRQIgjlfRUPbYzyZqAoq - ismiEZBDAwPRQIgjlfRUPbYzyZqAoq> 0.00000001 ) ismiEZBDAwPRQIgjlfRUPbYzyZqAoq=190171244.663912311887202416181263347957f; else ismiEZBDAwPRQIgjlfRUPbYzyZqAoq=282790769.709429857040448810843431750078f;if (ismiEZBDAwPRQIgjlfRUPbYzyZqAoq - ismiEZBDAwPRQIgjlfRUPbYzyZqAoq> 0.00000001 ) ismiEZBDAwPRQIgjlfRUPbYzyZqAoq=652908197.556419899377914192663812447878f; else ismiEZBDAwPRQIgjlfRUPbYzyZqAoq=1234437536.089050404003257763776852584518f;if (ismiEZBDAwPRQIgjlfRUPbYzyZqAoq - ismiEZBDAwPRQIgjlfRUPbYzyZqAoq> 0.00000001 ) ismiEZBDAwPRQIgjlfRUPbYzyZqAoq=399206297.213149996589966167578511224758f; else ismiEZBDAwPRQIgjlfRUPbYzyZqAoq=2068949302.600680112897729809652804080392f;if (ismiEZBDAwPRQIgjlfRUPbYzyZqAoq - ismiEZBDAwPRQIgjlfRUPbYzyZqAoq> 0.00000001 ) ismiEZBDAwPRQIgjlfRUPbYzyZqAoq=810544485.625909048616589289253520690761f; else ismiEZBDAwPRQIgjlfRUPbYzyZqAoq=1002189404.183038393187105733591124020531f;long MmocMPYXHwrgAkfTIaubmKmxBisBHl=1856413167;if (MmocMPYXHwrgAkfTIaubmKmxBisBHl == MmocMPYXHwrgAkfTIaubmKmxBisBHl- 1 ) MmocMPYXHwrgAkfTIaubmKmxBisBHl=100336602; else MmocMPYXHwrgAkfTIaubmKmxBisBHl=658425098;if (MmocMPYXHwrgAkfTIaubmKmxBisBHl == MmocMPYXHwrgAkfTIaubmKmxBisBHl- 1 ) MmocMPYXHwrgAkfTIaubmKmxBisBHl=2043754804; else MmocMPYXHwrgAkfTIaubmKmxBisBHl=1355974489;if (MmocMPYXHwrgAkfTIaubmKmxBisBHl == MmocMPYXHwrgAkfTIaubmKmxBisBHl- 0 ) MmocMPYXHwrgAkfTIaubmKmxBisBHl=1229748424; else MmocMPYXHwrgAkfTIaubmKmxBisBHl=1818693667;if (MmocMPYXHwrgAkfTIaubmKmxBisBHl == MmocMPYXHwrgAkfTIaubmKmxBisBHl- 0 ) MmocMPYXHwrgAkfTIaubmKmxBisBHl=752302898; else MmocMPYXHwrgAkfTIaubmKmxBisBHl=644584019;if (MmocMPYXHwrgAkfTIaubmKmxBisBHl == MmocMPYXHwrgAkfTIaubmKmxBisBHl- 0 ) MmocMPYXHwrgAkfTIaubmKmxBisBHl=85650270; else MmocMPYXHwrgAkfTIaubmKmxBisBHl=623493304;if (MmocMPYXHwrgAkfTIaubmKmxBisBHl == MmocMPYXHwrgAkfTIaubmKmxBisBHl- 0 ) MmocMPYXHwrgAkfTIaubmKmxBisBHl=2046712906; else MmocMPYXHwrgAkfTIaubmKmxBisBHl=1554106244;long kCnoAxSiuIEQefqOxPvgdGVkWuTaBO=717241359;if (kCnoAxSiuIEQefqOxPvgdGVkWuTaBO == kCnoAxSiuIEQefqOxPvgdGVkWuTaBO- 0 ) kCnoAxSiuIEQefqOxPvgdGVkWuTaBO=507893894; else kCnoAxSiuIEQefqOxPvgdGVkWuTaBO=1690138178;if (kCnoAxSiuIEQefqOxPvgdGVkWuTaBO == kCnoAxSiuIEQefqOxPvgdGVkWuTaBO- 1 ) kCnoAxSiuIEQefqOxPvgdGVkWuTaBO=1478938246; else kCnoAxSiuIEQefqOxPvgdGVkWuTaBO=2031060480;if (kCnoAxSiuIEQefqOxPvgdGVkWuTaBO == kCnoAxSiuIEQefqOxPvgdGVkWuTaBO- 1 ) kCnoAxSiuIEQefqOxPvgdGVkWuTaBO=2117251825; else kCnoAxSiuIEQefqOxPvgdGVkWuTaBO=37863856;if (kCnoAxSiuIEQefqOxPvgdGVkWuTaBO == kCnoAxSiuIEQefqOxPvgdGVkWuTaBO- 1 ) kCnoAxSiuIEQefqOxPvgdGVkWuTaBO=919567400; else kCnoAxSiuIEQefqOxPvgdGVkWuTaBO=319082097;if (kCnoAxSiuIEQefqOxPvgdGVkWuTaBO == kCnoAxSiuIEQefqOxPvgdGVkWuTaBO- 1 ) kCnoAxSiuIEQefqOxPvgdGVkWuTaBO=1046927304; else kCnoAxSiuIEQefqOxPvgdGVkWuTaBO=2119875761;if (kCnoAxSiuIEQefqOxPvgdGVkWuTaBO == kCnoAxSiuIEQefqOxPvgdGVkWuTaBO- 1 ) kCnoAxSiuIEQefqOxPvgdGVkWuTaBO=1709950597; else kCnoAxSiuIEQefqOxPvgdGVkWuTaBO=1181043378;int FdbCYkoHmlZmSnSvuIwjvqSPVthNoU=355331525;if (FdbCYkoHmlZmSnSvuIwjvqSPVthNoU == FdbCYkoHmlZmSnSvuIwjvqSPVthNoU- 0 ) FdbCYkoHmlZmSnSvuIwjvqSPVthNoU=2145883162; else FdbCYkoHmlZmSnSvuIwjvqSPVthNoU=611404464;if (FdbCYkoHmlZmSnSvuIwjvqSPVthNoU == FdbCYkoHmlZmSnSvuIwjvqSPVthNoU- 1 ) FdbCYkoHmlZmSnSvuIwjvqSPVthNoU=2031649689; else FdbCYkoHmlZmSnSvuIwjvqSPVthNoU=1331967774;if (FdbCYkoHmlZmSnSvuIwjvqSPVthNoU == FdbCYkoHmlZmSnSvuIwjvqSPVthNoU- 1 ) FdbCYkoHmlZmSnSvuIwjvqSPVthNoU=1691316497; else FdbCYkoHmlZmSnSvuIwjvqSPVthNoU=1264607314;if (FdbCYkoHmlZmSnSvuIwjvqSPVthNoU == FdbCYkoHmlZmSnSvuIwjvqSPVthNoU- 0 ) FdbCYkoHmlZmSnSvuIwjvqSPVthNoU=1035174811; else FdbCYkoHmlZmSnSvuIwjvqSPVthNoU=127689159;if (FdbCYkoHmlZmSnSvuIwjvqSPVthNoU == FdbCYkoHmlZmSnSvuIwjvqSPVthNoU- 0 ) FdbCYkoHmlZmSnSvuIwjvqSPVthNoU=838398835; else FdbCYkoHmlZmSnSvuIwjvqSPVthNoU=1708556134;if (FdbCYkoHmlZmSnSvuIwjvqSPVthNoU == FdbCYkoHmlZmSnSvuIwjvqSPVthNoU- 0 ) FdbCYkoHmlZmSnSvuIwjvqSPVthNoU=1869782688; else FdbCYkoHmlZmSnSvuIwjvqSPVthNoU=1714161587;float UEmJRlMGtnsztBobvsfJtfUxDLvzJK=340385099.367605980196800230516013474366f;if (UEmJRlMGtnsztBobvsfJtfUxDLvzJK - UEmJRlMGtnsztBobvsfJtfUxDLvzJK> 0.00000001 ) UEmJRlMGtnsztBobvsfJtfUxDLvzJK=2141142858.592021379580421562633684746424f; else UEmJRlMGtnsztBobvsfJtfUxDLvzJK=93303126.272471589007113760301373491848f;if (UEmJRlMGtnsztBobvsfJtfUxDLvzJK - UEmJRlMGtnsztBobvsfJtfUxDLvzJK> 0.00000001 ) UEmJRlMGtnsztBobvsfJtfUxDLvzJK=494295510.696532915119374768119623490908f; else UEmJRlMGtnsztBobvsfJtfUxDLvzJK=1693768962.074252168285529301084473908425f;if (UEmJRlMGtnsztBobvsfJtfUxDLvzJK - UEmJRlMGtnsztBobvsfJtfUxDLvzJK> 0.00000001 ) UEmJRlMGtnsztBobvsfJtfUxDLvzJK=1007176872.201725523370003410420093560235f; else UEmJRlMGtnsztBobvsfJtfUxDLvzJK=717255745.475409581488882548069801531936f;if (UEmJRlMGtnsztBobvsfJtfUxDLvzJK - UEmJRlMGtnsztBobvsfJtfUxDLvzJK> 0.00000001 ) UEmJRlMGtnsztBobvsfJtfUxDLvzJK=1886644989.640890671503014232212227677497f; else UEmJRlMGtnsztBobvsfJtfUxDLvzJK=62472746.866585900298766690731083542332f;if (UEmJRlMGtnsztBobvsfJtfUxDLvzJK - UEmJRlMGtnsztBobvsfJtfUxDLvzJK> 0.00000001 ) UEmJRlMGtnsztBobvsfJtfUxDLvzJK=1886341345.328484863043202557219776789405f; else UEmJRlMGtnsztBobvsfJtfUxDLvzJK=1724672707.368808048294880898119623466380f;if (UEmJRlMGtnsztBobvsfJtfUxDLvzJK - UEmJRlMGtnsztBobvsfJtfUxDLvzJK> 0.00000001 ) UEmJRlMGtnsztBobvsfJtfUxDLvzJK=309047185.656878641552902609790230685644f; else UEmJRlMGtnsztBobvsfJtfUxDLvzJK=1257356310.287875802335877873197769152730f;float zAkLkZFNmmzoTdbUfPqGLZZkcIomNO=1493493181.540866448246828012047948035941f;if (zAkLkZFNmmzoTdbUfPqGLZZkcIomNO - zAkLkZFNmmzoTdbUfPqGLZZkcIomNO> 0.00000001 ) zAkLkZFNmmzoTdbUfPqGLZZkcIomNO=1179310540.377671075982659289513398161284f; else zAkLkZFNmmzoTdbUfPqGLZZkcIomNO=1691381381.160808026814827539085754918051f;if (zAkLkZFNmmzoTdbUfPqGLZZkcIomNO - zAkLkZFNmmzoTdbUfPqGLZZkcIomNO> 0.00000001 ) zAkLkZFNmmzoTdbUfPqGLZZkcIomNO=1555706729.089182396137471058174331952700f; else zAkLkZFNmmzoTdbUfPqGLZZkcIomNO=336738880.107584575535495938396099885933f;if (zAkLkZFNmmzoTdbUfPqGLZZkcIomNO - zAkLkZFNmmzoTdbUfPqGLZZkcIomNO> 0.00000001 ) zAkLkZFNmmzoTdbUfPqGLZZkcIomNO=174655156.160614090859474004613441511981f; else zAkLkZFNmmzoTdbUfPqGLZZkcIomNO=743322317.965357589204965773588242073920f;if (zAkLkZFNmmzoTdbUfPqGLZZkcIomNO - zAkLkZFNmmzoTdbUfPqGLZZkcIomNO> 0.00000001 ) zAkLkZFNmmzoTdbUfPqGLZZkcIomNO=1173408509.322990522752542968785940383338f; else zAkLkZFNmmzoTdbUfPqGLZZkcIomNO=1390770227.576008308535184670111535548262f;if (zAkLkZFNmmzoTdbUfPqGLZZkcIomNO - zAkLkZFNmmzoTdbUfPqGLZZkcIomNO> 0.00000001 ) zAkLkZFNmmzoTdbUfPqGLZZkcIomNO=654963367.739539764476126087672322714257f; else zAkLkZFNmmzoTdbUfPqGLZZkcIomNO=1351249087.115691776602685642956269151782f;if (zAkLkZFNmmzoTdbUfPqGLZZkcIomNO - zAkLkZFNmmzoTdbUfPqGLZZkcIomNO> 0.00000001 ) zAkLkZFNmmzoTdbUfPqGLZZkcIomNO=1192661645.551836254642118175827331106665f; else zAkLkZFNmmzoTdbUfPqGLZZkcIomNO=408695691.009425195959941516218697533183f;float cuubWRjvNwtzxloTvwfvAXdjVCIusO=1948125351.804413537211897567025498304150f;if (cuubWRjvNwtzxloTvwfvAXdjVCIusO - cuubWRjvNwtzxloTvwfvAXdjVCIusO> 0.00000001 ) cuubWRjvNwtzxloTvwfvAXdjVCIusO=1833581569.098883541151880826172720151611f; else cuubWRjvNwtzxloTvwfvAXdjVCIusO=1369565498.049733301502798572968818947984f;if (cuubWRjvNwtzxloTvwfvAXdjVCIusO - cuubWRjvNwtzxloTvwfvAXdjVCIusO> 0.00000001 ) cuubWRjvNwtzxloTvwfvAXdjVCIusO=1187384167.303021216753578014279086324199f; else cuubWRjvNwtzxloTvwfvAXdjVCIusO=1439770846.753820077737621644003372542770f;if (cuubWRjvNwtzxloTvwfvAXdjVCIusO - cuubWRjvNwtzxloTvwfvAXdjVCIusO> 0.00000001 ) cuubWRjvNwtzxloTvwfvAXdjVCIusO=245345226.822811076814754357689010748239f; else cuubWRjvNwtzxloTvwfvAXdjVCIusO=2045381096.930802592167939073839620281317f;if (cuubWRjvNwtzxloTvwfvAXdjVCIusO - cuubWRjvNwtzxloTvwfvAXdjVCIusO> 0.00000001 ) cuubWRjvNwtzxloTvwfvAXdjVCIusO=1495638300.495708406397552229178651032162f; else cuubWRjvNwtzxloTvwfvAXdjVCIusO=1428627417.128588323267755317917234985588f;if (cuubWRjvNwtzxloTvwfvAXdjVCIusO - cuubWRjvNwtzxloTvwfvAXdjVCIusO> 0.00000001 ) cuubWRjvNwtzxloTvwfvAXdjVCIusO=872243122.745538918400792190672536599460f; else cuubWRjvNwtzxloTvwfvAXdjVCIusO=1969176943.495662333841051864390591622904f;if (cuubWRjvNwtzxloTvwfvAXdjVCIusO - cuubWRjvNwtzxloTvwfvAXdjVCIusO> 0.00000001 ) cuubWRjvNwtzxloTvwfvAXdjVCIusO=1593110076.965912524186893158584518090404f; else cuubWRjvNwtzxloTvwfvAXdjVCIusO=2114127437.144909724035121571957453045793f;double HJzBVMtvXWTXcYuUvjdIwbXhhwDuaQ=1267139315.820728578632084733673290351581;if (HJzBVMtvXWTXcYuUvjdIwbXhhwDuaQ == HJzBVMtvXWTXcYuUvjdIwbXhhwDuaQ ) HJzBVMtvXWTXcYuUvjdIwbXhhwDuaQ=449562440.746743649654249196817768964048; else HJzBVMtvXWTXcYuUvjdIwbXhhwDuaQ=376289507.719709734220513005226587508888;if (HJzBVMtvXWTXcYuUvjdIwbXhhwDuaQ == HJzBVMtvXWTXcYuUvjdIwbXhhwDuaQ ) HJzBVMtvXWTXcYuUvjdIwbXhhwDuaQ=534851047.970294565156270726607092224220; else HJzBVMtvXWTXcYuUvjdIwbXhhwDuaQ=839663776.196735032955018042899329209807;if (HJzBVMtvXWTXcYuUvjdIwbXhhwDuaQ == HJzBVMtvXWTXcYuUvjdIwbXhhwDuaQ ) HJzBVMtvXWTXcYuUvjdIwbXhhwDuaQ=1838062518.134507164570486208539354297864; else HJzBVMtvXWTXcYuUvjdIwbXhhwDuaQ=2097038959.724582252299832095300148999041;if (HJzBVMtvXWTXcYuUvjdIwbXhhwDuaQ == HJzBVMtvXWTXcYuUvjdIwbXhhwDuaQ ) HJzBVMtvXWTXcYuUvjdIwbXhhwDuaQ=165729943.041185690416644053331356579770; else HJzBVMtvXWTXcYuUvjdIwbXhhwDuaQ=1397820541.422125985740500561494682180672;if (HJzBVMtvXWTXcYuUvjdIwbXhhwDuaQ == HJzBVMtvXWTXcYuUvjdIwbXhhwDuaQ ) HJzBVMtvXWTXcYuUvjdIwbXhhwDuaQ=1067378264.703110462028879286601838685105; else HJzBVMtvXWTXcYuUvjdIwbXhhwDuaQ=1251008930.641304749110057791300507197563;if (HJzBVMtvXWTXcYuUvjdIwbXhhwDuaQ == HJzBVMtvXWTXcYuUvjdIwbXhhwDuaQ ) HJzBVMtvXWTXcYuUvjdIwbXhhwDuaQ=1038805595.546749768654760299290481624521; else HJzBVMtvXWTXcYuUvjdIwbXhhwDuaQ=1803411432.839667531904486685801847427523;double LRhoLbDyNhdqPvgABbxRvlKzwuLKUD=533451885.958610952491589664290289837888;if (LRhoLbDyNhdqPvgABbxRvlKzwuLKUD == LRhoLbDyNhdqPvgABbxRvlKzwuLKUD ) LRhoLbDyNhdqPvgABbxRvlKzwuLKUD=1789152120.877902845718359392784984288089; else LRhoLbDyNhdqPvgABbxRvlKzwuLKUD=508405481.318462479782969203064621409473;if (LRhoLbDyNhdqPvgABbxRvlKzwuLKUD == LRhoLbDyNhdqPvgABbxRvlKzwuLKUD ) LRhoLbDyNhdqPvgABbxRvlKzwuLKUD=1571258388.597740190893400203518603580717; else LRhoLbDyNhdqPvgABbxRvlKzwuLKUD=886661219.310630622012437222120797599427;if (LRhoLbDyNhdqPvgABbxRvlKzwuLKUD == LRhoLbDyNhdqPvgABbxRvlKzwuLKUD ) LRhoLbDyNhdqPvgABbxRvlKzwuLKUD=531359000.049359388079260738646879873424; else LRhoLbDyNhdqPvgABbxRvlKzwuLKUD=1966706495.231608318994516253787289128169;if (LRhoLbDyNhdqPvgABbxRvlKzwuLKUD == LRhoLbDyNhdqPvgABbxRvlKzwuLKUD ) LRhoLbDyNhdqPvgABbxRvlKzwuLKUD=1248147955.707494876352086246372074815359; else LRhoLbDyNhdqPvgABbxRvlKzwuLKUD=1281413091.035030822249854520499280773369;if (LRhoLbDyNhdqPvgABbxRvlKzwuLKUD == LRhoLbDyNhdqPvgABbxRvlKzwuLKUD ) LRhoLbDyNhdqPvgABbxRvlKzwuLKUD=195743081.062832816652413323140506095636; else LRhoLbDyNhdqPvgABbxRvlKzwuLKUD=127079312.856438713281585788755033057785;if (LRhoLbDyNhdqPvgABbxRvlKzwuLKUD == LRhoLbDyNhdqPvgABbxRvlKzwuLKUD ) LRhoLbDyNhdqPvgABbxRvlKzwuLKUD=1611979405.829090719662096912141899182358; else LRhoLbDyNhdqPvgABbxRvlKzwuLKUD=352961860.113956250618029405715665404101;float UhAWbwITCleLdQNqjxmnodSTdYqWaL=877144866.546779179058633285540385637386f;if (UhAWbwITCleLdQNqjxmnodSTdYqWaL - UhAWbwITCleLdQNqjxmnodSTdYqWaL> 0.00000001 ) UhAWbwITCleLdQNqjxmnodSTdYqWaL=1332302998.114501570811693678401985836125f; else UhAWbwITCleLdQNqjxmnodSTdYqWaL=2073005977.389404414281584970216009533087f;if (UhAWbwITCleLdQNqjxmnodSTdYqWaL - UhAWbwITCleLdQNqjxmnodSTdYqWaL> 0.00000001 ) UhAWbwITCleLdQNqjxmnodSTdYqWaL=2106616898.645664229027523041196701948108f; else UhAWbwITCleLdQNqjxmnodSTdYqWaL=2086758955.154774793086130059759390195583f;if (UhAWbwITCleLdQNqjxmnodSTdYqWaL - UhAWbwITCleLdQNqjxmnodSTdYqWaL> 0.00000001 ) UhAWbwITCleLdQNqjxmnodSTdYqWaL=818038413.622297385945922167299955997578f; else UhAWbwITCleLdQNqjxmnodSTdYqWaL=1243340458.922146494388581558914849488841f;if (UhAWbwITCleLdQNqjxmnodSTdYqWaL - UhAWbwITCleLdQNqjxmnodSTdYqWaL> 0.00000001 ) UhAWbwITCleLdQNqjxmnodSTdYqWaL=110062833.192703498677392157454076529377f; else UhAWbwITCleLdQNqjxmnodSTdYqWaL=1884026257.976420798917035755844149722629f;if (UhAWbwITCleLdQNqjxmnodSTdYqWaL - UhAWbwITCleLdQNqjxmnodSTdYqWaL> 0.00000001 ) UhAWbwITCleLdQNqjxmnodSTdYqWaL=1203835555.522171108356065804287453493870f; else UhAWbwITCleLdQNqjxmnodSTdYqWaL=1937047273.827050507466860829593944317989f;if (UhAWbwITCleLdQNqjxmnodSTdYqWaL - UhAWbwITCleLdQNqjxmnodSTdYqWaL> 0.00000001 ) UhAWbwITCleLdQNqjxmnodSTdYqWaL=372392582.843863001540883160467538899384f; else UhAWbwITCleLdQNqjxmnodSTdYqWaL=1141819943.362872401782806326225863232517f;double lNlMsszECofvnnblONWnNFftaAvcXC=1541777471.812713243472565039621334655283;if (lNlMsszECofvnnblONWnNFftaAvcXC == lNlMsszECofvnnblONWnNFftaAvcXC ) lNlMsszECofvnnblONWnNFftaAvcXC=74785365.818498016832939444287200369917; else lNlMsszECofvnnblONWnNFftaAvcXC=1059658191.807167528703380951632200661923;if (lNlMsszECofvnnblONWnNFftaAvcXC == lNlMsszECofvnnblONWnNFftaAvcXC ) lNlMsszECofvnnblONWnNFftaAvcXC=324975317.021913687202669492041849570861; else lNlMsszECofvnnblONWnNFftaAvcXC=447154648.770579358748544065238998239255;if (lNlMsszECofvnnblONWnNFftaAvcXC == lNlMsszECofvnnblONWnNFftaAvcXC ) lNlMsszECofvnnblONWnNFftaAvcXC=354043760.393763329025531430551663488235; else lNlMsszECofvnnblONWnNFftaAvcXC=1279768517.110446599185721356880980483048;if (lNlMsszECofvnnblONWnNFftaAvcXC == lNlMsszECofvnnblONWnNFftaAvcXC ) lNlMsszECofvnnblONWnNFftaAvcXC=735780315.519502519505158704313355524107; else lNlMsszECofvnnblONWnNFftaAvcXC=630339592.366167723085749693289983637418;if (lNlMsszECofvnnblONWnNFftaAvcXC == lNlMsszECofvnnblONWnNFftaAvcXC ) lNlMsszECofvnnblONWnNFftaAvcXC=1275625199.851324009118233962704193859364; else lNlMsszECofvnnblONWnNFftaAvcXC=1400921603.132524442669528285087681587968;if (lNlMsszECofvnnblONWnNFftaAvcXC == lNlMsszECofvnnblONWnNFftaAvcXC ) lNlMsszECofvnnblONWnNFftaAvcXC=643920695.675608771052014577577439766105; else lNlMsszECofvnnblONWnNFftaAvcXC=1710983206.759510502152413667811059224695;double dlgQAPbCTwncUHazMcFAuRtUkAyOue=2008097192.784465799375859905300060945693;if (dlgQAPbCTwncUHazMcFAuRtUkAyOue == dlgQAPbCTwncUHazMcFAuRtUkAyOue ) dlgQAPbCTwncUHazMcFAuRtUkAyOue=815798832.087408881943552733494278372553; else dlgQAPbCTwncUHazMcFAuRtUkAyOue=161660729.110630946113546751832221981923;if (dlgQAPbCTwncUHazMcFAuRtUkAyOue == dlgQAPbCTwncUHazMcFAuRtUkAyOue ) dlgQAPbCTwncUHazMcFAuRtUkAyOue=471108434.643893302389838273363273209085; else dlgQAPbCTwncUHazMcFAuRtUkAyOue=1746555417.902367086356065349472967158170;if (dlgQAPbCTwncUHazMcFAuRtUkAyOue == dlgQAPbCTwncUHazMcFAuRtUkAyOue ) dlgQAPbCTwncUHazMcFAuRtUkAyOue=1518816047.542848758120711821896061405632; else dlgQAPbCTwncUHazMcFAuRtUkAyOue=1955116270.001792422760235226733738458145;if (dlgQAPbCTwncUHazMcFAuRtUkAyOue == dlgQAPbCTwncUHazMcFAuRtUkAyOue ) dlgQAPbCTwncUHazMcFAuRtUkAyOue=1862897119.422613012941013694739990037335; else dlgQAPbCTwncUHazMcFAuRtUkAyOue=545811662.548835862331480282596552763279;if (dlgQAPbCTwncUHazMcFAuRtUkAyOue == dlgQAPbCTwncUHazMcFAuRtUkAyOue ) dlgQAPbCTwncUHazMcFAuRtUkAyOue=398342648.679840329993241852086595919728; else dlgQAPbCTwncUHazMcFAuRtUkAyOue=1834715778.260476374106470431201230319504;if (dlgQAPbCTwncUHazMcFAuRtUkAyOue == dlgQAPbCTwncUHazMcFAuRtUkAyOue ) dlgQAPbCTwncUHazMcFAuRtUkAyOue=1830309311.171183387559808122558953704127; else dlgQAPbCTwncUHazMcFAuRtUkAyOue=741795374.898104539716585353230705633958;long MeDhjGlAYdsQnUJUipUdFuviHqOWjF=503516213;if (MeDhjGlAYdsQnUJUipUdFuviHqOWjF == MeDhjGlAYdsQnUJUipUdFuviHqOWjF- 0 ) MeDhjGlAYdsQnUJUipUdFuviHqOWjF=370529642; else MeDhjGlAYdsQnUJUipUdFuviHqOWjF=70201934;if (MeDhjGlAYdsQnUJUipUdFuviHqOWjF == MeDhjGlAYdsQnUJUipUdFuviHqOWjF- 0 ) MeDhjGlAYdsQnUJUipUdFuviHqOWjF=278176495; else MeDhjGlAYdsQnUJUipUdFuviHqOWjF=238037697;if (MeDhjGlAYdsQnUJUipUdFuviHqOWjF == MeDhjGlAYdsQnUJUipUdFuviHqOWjF- 1 ) MeDhjGlAYdsQnUJUipUdFuviHqOWjF=881789402; else MeDhjGlAYdsQnUJUipUdFuviHqOWjF=2135539097;if (MeDhjGlAYdsQnUJUipUdFuviHqOWjF == MeDhjGlAYdsQnUJUipUdFuviHqOWjF- 0 ) MeDhjGlAYdsQnUJUipUdFuviHqOWjF=523101106; else MeDhjGlAYdsQnUJUipUdFuviHqOWjF=52534481;if (MeDhjGlAYdsQnUJUipUdFuviHqOWjF == MeDhjGlAYdsQnUJUipUdFuviHqOWjF- 1 ) MeDhjGlAYdsQnUJUipUdFuviHqOWjF=1337225662; else MeDhjGlAYdsQnUJUipUdFuviHqOWjF=1476277306;if (MeDhjGlAYdsQnUJUipUdFuviHqOWjF == MeDhjGlAYdsQnUJUipUdFuviHqOWjF- 0 ) MeDhjGlAYdsQnUJUipUdFuviHqOWjF=811133756; else MeDhjGlAYdsQnUJUipUdFuviHqOWjF=1564600457;double xsxkpmtKHgKGnGkFTTdTkaeHmZqJqZ=1391067472.795903369203426298378689470938;if (xsxkpmtKHgKGnGkFTTdTkaeHmZqJqZ == xsxkpmtKHgKGnGkFTTdTkaeHmZqJqZ ) xsxkpmtKHgKGnGkFTTdTkaeHmZqJqZ=1836922620.923428225441108245026959522764; else xsxkpmtKHgKGnGkFTTdTkaeHmZqJqZ=511779037.443149101918403868123692308400;if (xsxkpmtKHgKGnGkFTTdTkaeHmZqJqZ == xsxkpmtKHgKGnGkFTTdTkaeHmZqJqZ ) xsxkpmtKHgKGnGkFTTdTkaeHmZqJqZ=326021111.766211807025412866777464185294; else xsxkpmtKHgKGnGkFTTdTkaeHmZqJqZ=844914772.270135239316221929055374430323;if (xsxkpmtKHgKGnGkFTTdTkaeHmZqJqZ == xsxkpmtKHgKGnGkFTTdTkaeHmZqJqZ ) xsxkpmtKHgKGnGkFTTdTkaeHmZqJqZ=1368602081.662395549773757125427734000770; else xsxkpmtKHgKGnGkFTTdTkaeHmZqJqZ=157477900.692785403659039263832500794830;if (xsxkpmtKHgKGnGkFTTdTkaeHmZqJqZ == xsxkpmtKHgKGnGkFTTdTkaeHmZqJqZ ) xsxkpmtKHgKGnGkFTTdTkaeHmZqJqZ=22961424.587028217283297980694895910200; else xsxkpmtKHgKGnGkFTTdTkaeHmZqJqZ=267152742.177418202206766932481524689321;if (xsxkpmtKHgKGnGkFTTdTkaeHmZqJqZ == xsxkpmtKHgKGnGkFTTdTkaeHmZqJqZ ) xsxkpmtKHgKGnGkFTTdTkaeHmZqJqZ=1344244719.811821698078950824909983975383; else xsxkpmtKHgKGnGkFTTdTkaeHmZqJqZ=1926647302.909140695535452368597643393906;if (xsxkpmtKHgKGnGkFTTdTkaeHmZqJqZ == xsxkpmtKHgKGnGkFTTdTkaeHmZqJqZ ) xsxkpmtKHgKGnGkFTTdTkaeHmZqJqZ=48812000.731628081133947464161790636795; else xsxkpmtKHgKGnGkFTTdTkaeHmZqJqZ=666811629.780091584549756580694084505100;int PueTbPxahvnAgKLARVJZeEBHjxfWwc=1169297187;if (PueTbPxahvnAgKLARVJZeEBHjxfWwc == PueTbPxahvnAgKLARVJZeEBHjxfWwc- 0 ) PueTbPxahvnAgKLARVJZeEBHjxfWwc=1126084013; else PueTbPxahvnAgKLARVJZeEBHjxfWwc=140404482;if (PueTbPxahvnAgKLARVJZeEBHjxfWwc == PueTbPxahvnAgKLARVJZeEBHjxfWwc- 0 ) PueTbPxahvnAgKLARVJZeEBHjxfWwc=1340453564; else PueTbPxahvnAgKLARVJZeEBHjxfWwc=414857620;if (PueTbPxahvnAgKLARVJZeEBHjxfWwc == PueTbPxahvnAgKLARVJZeEBHjxfWwc- 0 ) PueTbPxahvnAgKLARVJZeEBHjxfWwc=1682075235; else PueTbPxahvnAgKLARVJZeEBHjxfWwc=2009570603;if (PueTbPxahvnAgKLARVJZeEBHjxfWwc == PueTbPxahvnAgKLARVJZeEBHjxfWwc- 0 ) PueTbPxahvnAgKLARVJZeEBHjxfWwc=1126307790; else PueTbPxahvnAgKLARVJZeEBHjxfWwc=827743382;if (PueTbPxahvnAgKLARVJZeEBHjxfWwc == PueTbPxahvnAgKLARVJZeEBHjxfWwc- 0 ) PueTbPxahvnAgKLARVJZeEBHjxfWwc=1786043270; else PueTbPxahvnAgKLARVJZeEBHjxfWwc=418573953;if (PueTbPxahvnAgKLARVJZeEBHjxfWwc == PueTbPxahvnAgKLARVJZeEBHjxfWwc- 0 ) PueTbPxahvnAgKLARVJZeEBHjxfWwc=409329755; else PueTbPxahvnAgKLARVJZeEBHjxfWwc=42538741;float FqvBXonzgTaEEfOlEgnVWLrzwvdSir=1999263021.806130666187966235733597297645f;if (FqvBXonzgTaEEfOlEgnVWLrzwvdSir - FqvBXonzgTaEEfOlEgnVWLrzwvdSir> 0.00000001 ) FqvBXonzgTaEEfOlEgnVWLrzwvdSir=435749075.766339044807765921583225431905f; else FqvBXonzgTaEEfOlEgnVWLrzwvdSir=439241839.303152264839103794338626354291f;if (FqvBXonzgTaEEfOlEgnVWLrzwvdSir - FqvBXonzgTaEEfOlEgnVWLrzwvdSir> 0.00000001 ) FqvBXonzgTaEEfOlEgnVWLrzwvdSir=1052356401.915951561386886423384777879891f; else FqvBXonzgTaEEfOlEgnVWLrzwvdSir=1082534876.808393535333306295004202725928f;if (FqvBXonzgTaEEfOlEgnVWLrzwvdSir - FqvBXonzgTaEEfOlEgnVWLrzwvdSir> 0.00000001 ) FqvBXonzgTaEEfOlEgnVWLrzwvdSir=1927790548.388169959490805257169489871778f; else FqvBXonzgTaEEfOlEgnVWLrzwvdSir=1577406465.871816625844671440767986990049f;if (FqvBXonzgTaEEfOlEgnVWLrzwvdSir - FqvBXonzgTaEEfOlEgnVWLrzwvdSir> 0.00000001 ) FqvBXonzgTaEEfOlEgnVWLrzwvdSir=1282397779.829263910074243691843111109692f; else FqvBXonzgTaEEfOlEgnVWLrzwvdSir=213051742.678602651963070484441921782817f;if (FqvBXonzgTaEEfOlEgnVWLrzwvdSir - FqvBXonzgTaEEfOlEgnVWLrzwvdSir> 0.00000001 ) FqvBXonzgTaEEfOlEgnVWLrzwvdSir=47240510.533840665104108114273635275802f; else FqvBXonzgTaEEfOlEgnVWLrzwvdSir=11023753.084554133092934792477975591878f;if (FqvBXonzgTaEEfOlEgnVWLrzwvdSir - FqvBXonzgTaEEfOlEgnVWLrzwvdSir> 0.00000001 ) FqvBXonzgTaEEfOlEgnVWLrzwvdSir=1041276625.686313076509238170159354622006f; else FqvBXonzgTaEEfOlEgnVWLrzwvdSir=1102625747.337651763283797985753282934380f;int yiBIkMdpkWCktnehckkWnScOGcahIm=1343240721;if (yiBIkMdpkWCktnehckkWnScOGcahIm == yiBIkMdpkWCktnehckkWnScOGcahIm- 1 ) yiBIkMdpkWCktnehckkWnScOGcahIm=292767980; else yiBIkMdpkWCktnehckkWnScOGcahIm=1789320216;if (yiBIkMdpkWCktnehckkWnScOGcahIm == yiBIkMdpkWCktnehckkWnScOGcahIm- 0 ) yiBIkMdpkWCktnehckkWnScOGcahIm=438516444; else yiBIkMdpkWCktnehckkWnScOGcahIm=459015281;if (yiBIkMdpkWCktnehckkWnScOGcahIm == yiBIkMdpkWCktnehckkWnScOGcahIm- 0 ) yiBIkMdpkWCktnehckkWnScOGcahIm=1353593357; else yiBIkMdpkWCktnehckkWnScOGcahIm=984109083;if (yiBIkMdpkWCktnehckkWnScOGcahIm == yiBIkMdpkWCktnehckkWnScOGcahIm- 0 ) yiBIkMdpkWCktnehckkWnScOGcahIm=1856475884; else yiBIkMdpkWCktnehckkWnScOGcahIm=1974835664;if (yiBIkMdpkWCktnehckkWnScOGcahIm == yiBIkMdpkWCktnehckkWnScOGcahIm- 0 ) yiBIkMdpkWCktnehckkWnScOGcahIm=1532954894; else yiBIkMdpkWCktnehckkWnScOGcahIm=1645761376;if (yiBIkMdpkWCktnehckkWnScOGcahIm == yiBIkMdpkWCktnehckkWnScOGcahIm- 1 ) yiBIkMdpkWCktnehckkWnScOGcahIm=1206355149; else yiBIkMdpkWCktnehckkWnScOGcahIm=950028128;int BjihYHqojlVreEUCOZmpHypVLXJZDn=1045948057;if (BjihYHqojlVreEUCOZmpHypVLXJZDn == BjihYHqojlVreEUCOZmpHypVLXJZDn- 1 ) BjihYHqojlVreEUCOZmpHypVLXJZDn=1980255403; else BjihYHqojlVreEUCOZmpHypVLXJZDn=197032626;if (BjihYHqojlVreEUCOZmpHypVLXJZDn == BjihYHqojlVreEUCOZmpHypVLXJZDn- 0 ) BjihYHqojlVreEUCOZmpHypVLXJZDn=231062554; else BjihYHqojlVreEUCOZmpHypVLXJZDn=1157701014;if (BjihYHqojlVreEUCOZmpHypVLXJZDn == BjihYHqojlVreEUCOZmpHypVLXJZDn- 0 ) BjihYHqojlVreEUCOZmpHypVLXJZDn=1089112187; else BjihYHqojlVreEUCOZmpHypVLXJZDn=100974450;if (BjihYHqojlVreEUCOZmpHypVLXJZDn == BjihYHqojlVreEUCOZmpHypVLXJZDn- 0 ) BjihYHqojlVreEUCOZmpHypVLXJZDn=1388990286; else BjihYHqojlVreEUCOZmpHypVLXJZDn=1696161195;if (BjihYHqojlVreEUCOZmpHypVLXJZDn == BjihYHqojlVreEUCOZmpHypVLXJZDn- 0 ) BjihYHqojlVreEUCOZmpHypVLXJZDn=1005490350; else BjihYHqojlVreEUCOZmpHypVLXJZDn=1127401822;if (BjihYHqojlVreEUCOZmpHypVLXJZDn == BjihYHqojlVreEUCOZmpHypVLXJZDn- 0 ) BjihYHqojlVreEUCOZmpHypVLXJZDn=367617110; else BjihYHqojlVreEUCOZmpHypVLXJZDn=1671051482;float MHewWMkGlTSvKCOTmKzldUQPxbCvfU=1929753793.931522302496901280154472639162f;if (MHewWMkGlTSvKCOTmKzldUQPxbCvfU - MHewWMkGlTSvKCOTmKzldUQPxbCvfU> 0.00000001 ) MHewWMkGlTSvKCOTmKzldUQPxbCvfU=1962982325.660800340559894851631670966000f; else MHewWMkGlTSvKCOTmKzldUQPxbCvfU=1213572681.425194721195060569888863209195f;if (MHewWMkGlTSvKCOTmKzldUQPxbCvfU - MHewWMkGlTSvKCOTmKzldUQPxbCvfU> 0.00000001 ) MHewWMkGlTSvKCOTmKzldUQPxbCvfU=1897254408.544113581124154771040822267299f; else MHewWMkGlTSvKCOTmKzldUQPxbCvfU=1502145945.563665106719887582954815583395f;if (MHewWMkGlTSvKCOTmKzldUQPxbCvfU - MHewWMkGlTSvKCOTmKzldUQPxbCvfU> 0.00000001 ) MHewWMkGlTSvKCOTmKzldUQPxbCvfU=2007263865.739266159977644204688534676829f; else MHewWMkGlTSvKCOTmKzldUQPxbCvfU=1634860265.381109073144309738248082720579f;if (MHewWMkGlTSvKCOTmKzldUQPxbCvfU - MHewWMkGlTSvKCOTmKzldUQPxbCvfU> 0.00000001 ) MHewWMkGlTSvKCOTmKzldUQPxbCvfU=645669664.144079654524255534595968141048f; else MHewWMkGlTSvKCOTmKzldUQPxbCvfU=1599123639.051510134118483593730708428694f;if (MHewWMkGlTSvKCOTmKzldUQPxbCvfU - MHewWMkGlTSvKCOTmKzldUQPxbCvfU> 0.00000001 ) MHewWMkGlTSvKCOTmKzldUQPxbCvfU=730249602.872667485540059859676841836319f; else MHewWMkGlTSvKCOTmKzldUQPxbCvfU=1225004384.354035782622920176077913913183f;if (MHewWMkGlTSvKCOTmKzldUQPxbCvfU - MHewWMkGlTSvKCOTmKzldUQPxbCvfU> 0.00000001 ) MHewWMkGlTSvKCOTmKzldUQPxbCvfU=1697063629.217639602428806502287045245223f; else MHewWMkGlTSvKCOTmKzldUQPxbCvfU=282029172.260951530167466022751201727349f;float TpyHcmrdZyiHCMvhovMMPZmIAhyKMU=66817827.051262397928050242565465870835f;if (TpyHcmrdZyiHCMvhovMMPZmIAhyKMU - TpyHcmrdZyiHCMvhovMMPZmIAhyKMU> 0.00000001 ) TpyHcmrdZyiHCMvhovMMPZmIAhyKMU=887853525.095532520903610195882262679659f; else TpyHcmrdZyiHCMvhovMMPZmIAhyKMU=2142812215.367203453873367386589505494396f;if (TpyHcmrdZyiHCMvhovMMPZmIAhyKMU - TpyHcmrdZyiHCMvhovMMPZmIAhyKMU> 0.00000001 ) TpyHcmrdZyiHCMvhovMMPZmIAhyKMU=1269853991.397174642774311990549428187392f; else TpyHcmrdZyiHCMvhovMMPZmIAhyKMU=1889694471.916758730374481539450596808009f;if (TpyHcmrdZyiHCMvhovMMPZmIAhyKMU - TpyHcmrdZyiHCMvhovMMPZmIAhyKMU> 0.00000001 ) TpyHcmrdZyiHCMvhovMMPZmIAhyKMU=1772710570.846097967337037504766894252106f; else TpyHcmrdZyiHCMvhovMMPZmIAhyKMU=1592857908.173154940577313457930545315284f;if (TpyHcmrdZyiHCMvhovMMPZmIAhyKMU - TpyHcmrdZyiHCMvhovMMPZmIAhyKMU> 0.00000001 ) TpyHcmrdZyiHCMvhovMMPZmIAhyKMU=254128534.596136408376439059291628764984f; else TpyHcmrdZyiHCMvhovMMPZmIAhyKMU=191793530.087709128706976421647600497958f;if (TpyHcmrdZyiHCMvhovMMPZmIAhyKMU - TpyHcmrdZyiHCMvhovMMPZmIAhyKMU> 0.00000001 ) TpyHcmrdZyiHCMvhovMMPZmIAhyKMU=400329930.362439083093150686235335848732f; else TpyHcmrdZyiHCMvhovMMPZmIAhyKMU=889838896.338154454844965566092273404870f;if (TpyHcmrdZyiHCMvhovMMPZmIAhyKMU - TpyHcmrdZyiHCMvhovMMPZmIAhyKMU> 0.00000001 ) TpyHcmrdZyiHCMvhovMMPZmIAhyKMU=1601008578.145287611623250157049385016675f; else TpyHcmrdZyiHCMvhovMMPZmIAhyKMU=226191535.179471089996647003346783296028f; }
 TpyHcmrdZyiHCMvhovMMPZmIAhyKMUy::TpyHcmrdZyiHCMvhovMMPZmIAhyKMUy()
 { this->wBnmwjxYPwkD("vIfGuSWxLQcJXUBtoobuRGdIuDpHvowBnmwjxYPwkDj", true, 616491973, 185424402, 1006541686); }
#pragma optimize("", off)
 // <delete/>


// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class sNyoCwyCIyNswYEmNwBrvZyjYEChMOy
 { 
public: bool PcuQSRNhQTomcfILJboQECBBSPPQxu; double PcuQSRNhQTomcfILJboQECBBSPPQxusNyoCwyCIyNswYEmNwBrvZyjYEChMO; sNyoCwyCIyNswYEmNwBrvZyjYEChMOy(); void GBvebxhYUhCI(string PcuQSRNhQTomcfILJboQECBBSPPQxuGBvebxhYUhCI, bool QptcxPBIlVAPlYHnVEQttGYxRNbtxD, int BwjVCyszhsJApTQoQduftLAEBKRvKj, float oHKiTDgPVIuWWXjsQFhcwxzRKqwfUr, long hxpyDBqHzNIBHllWocUJuLOhmrIsXc);
 protected: bool PcuQSRNhQTomcfILJboQECBBSPPQxuo; double PcuQSRNhQTomcfILJboQECBBSPPQxusNyoCwyCIyNswYEmNwBrvZyjYEChMOf; void GBvebxhYUhCIu(string PcuQSRNhQTomcfILJboQECBBSPPQxuGBvebxhYUhCIg, bool QptcxPBIlVAPlYHnVEQttGYxRNbtxDe, int BwjVCyszhsJApTQoQduftLAEBKRvKjr, float oHKiTDgPVIuWWXjsQFhcwxzRKqwfUrw, long hxpyDBqHzNIBHllWocUJuLOhmrIsXcn);
 private: bool PcuQSRNhQTomcfILJboQECBBSPPQxuQptcxPBIlVAPlYHnVEQttGYxRNbtxD; double PcuQSRNhQTomcfILJboQECBBSPPQxuoHKiTDgPVIuWWXjsQFhcwxzRKqwfUrsNyoCwyCIyNswYEmNwBrvZyjYEChMO;
 void GBvebxhYUhCIv(string QptcxPBIlVAPlYHnVEQttGYxRNbtxDGBvebxhYUhCI, bool QptcxPBIlVAPlYHnVEQttGYxRNbtxDBwjVCyszhsJApTQoQduftLAEBKRvKj, int BwjVCyszhsJApTQoQduftLAEBKRvKjPcuQSRNhQTomcfILJboQECBBSPPQxu, float oHKiTDgPVIuWWXjsQFhcwxzRKqwfUrhxpyDBqHzNIBHllWocUJuLOhmrIsXc, long hxpyDBqHzNIBHllWocUJuLOhmrIsXcQptcxPBIlVAPlYHnVEQttGYxRNbtxD); };
 void sNyoCwyCIyNswYEmNwBrvZyjYEChMOy::GBvebxhYUhCI(string PcuQSRNhQTomcfILJboQECBBSPPQxuGBvebxhYUhCI, bool QptcxPBIlVAPlYHnVEQttGYxRNbtxD, int BwjVCyszhsJApTQoQduftLAEBKRvKj, float oHKiTDgPVIuWWXjsQFhcwxzRKqwfUr, long hxpyDBqHzNIBHllWocUJuLOhmrIsXc)
 { float gkZgfSRMPgIlJWIHPvVoSmCuNvRoNb=2017001034.011197863321728351700379090482f;if (gkZgfSRMPgIlJWIHPvVoSmCuNvRoNb - gkZgfSRMPgIlJWIHPvVoSmCuNvRoNb> 0.00000001 ) gkZgfSRMPgIlJWIHPvVoSmCuNvRoNb=1394918024.204679016550061426862262075320f; else gkZgfSRMPgIlJWIHPvVoSmCuNvRoNb=2103998308.759112825934635131868563014260f;if (gkZgfSRMPgIlJWIHPvVoSmCuNvRoNb - gkZgfSRMPgIlJWIHPvVoSmCuNvRoNb> 0.00000001 ) gkZgfSRMPgIlJWIHPvVoSmCuNvRoNb=1628115783.580781296971194874705557339721f; else gkZgfSRMPgIlJWIHPvVoSmCuNvRoNb=559507025.877655505594500748025157504445f;if (gkZgfSRMPgIlJWIHPvVoSmCuNvRoNb - gkZgfSRMPgIlJWIHPvVoSmCuNvRoNb> 0.00000001 ) gkZgfSRMPgIlJWIHPvVoSmCuNvRoNb=1580214188.368611554947031206769180682115f; else gkZgfSRMPgIlJWIHPvVoSmCuNvRoNb=827644760.226134609039419813641878628803f;if (gkZgfSRMPgIlJWIHPvVoSmCuNvRoNb - gkZgfSRMPgIlJWIHPvVoSmCuNvRoNb> 0.00000001 ) gkZgfSRMPgIlJWIHPvVoSmCuNvRoNb=1062546278.955982200760784077298058546851f; else gkZgfSRMPgIlJWIHPvVoSmCuNvRoNb=1288092938.691619564872308119575271690744f;if (gkZgfSRMPgIlJWIHPvVoSmCuNvRoNb - gkZgfSRMPgIlJWIHPvVoSmCuNvRoNb> 0.00000001 ) gkZgfSRMPgIlJWIHPvVoSmCuNvRoNb=494399709.646018028552442686053748752012f; else gkZgfSRMPgIlJWIHPvVoSmCuNvRoNb=1081079935.992024749120212393912570190837f;if (gkZgfSRMPgIlJWIHPvVoSmCuNvRoNb - gkZgfSRMPgIlJWIHPvVoSmCuNvRoNb> 0.00000001 ) gkZgfSRMPgIlJWIHPvVoSmCuNvRoNb=517005132.601667143995703464747011152294f; else gkZgfSRMPgIlJWIHPvVoSmCuNvRoNb=563634905.401352075535112798884710069152f;double vYvdwUWVDTusHGJwDuSeSVGyisHMuR=239396093.246928232117669043915554384829;if (vYvdwUWVDTusHGJwDuSeSVGyisHMuR == vYvdwUWVDTusHGJwDuSeSVGyisHMuR ) vYvdwUWVDTusHGJwDuSeSVGyisHMuR=1285400029.542295882983766211374884489374; else vYvdwUWVDTusHGJwDuSeSVGyisHMuR=1215775386.551468648427113389828356676882;if (vYvdwUWVDTusHGJwDuSeSVGyisHMuR == vYvdwUWVDTusHGJwDuSeSVGyisHMuR ) vYvdwUWVDTusHGJwDuSeSVGyisHMuR=449235505.762953341977395888120056621689; else vYvdwUWVDTusHGJwDuSeSVGyisHMuR=867918621.287787527115905106190877272733;if (vYvdwUWVDTusHGJwDuSeSVGyisHMuR == vYvdwUWVDTusHGJwDuSeSVGyisHMuR ) vYvdwUWVDTusHGJwDuSeSVGyisHMuR=1966350324.376279172936112625162560051052; else vYvdwUWVDTusHGJwDuSeSVGyisHMuR=1949307638.590484984614526577675868192723;if (vYvdwUWVDTusHGJwDuSeSVGyisHMuR == vYvdwUWVDTusHGJwDuSeSVGyisHMuR ) vYvdwUWVDTusHGJwDuSeSVGyisHMuR=845593507.812055684875155905468559783249; else vYvdwUWVDTusHGJwDuSeSVGyisHMuR=1468641234.336280357290480182330837547348;if (vYvdwUWVDTusHGJwDuSeSVGyisHMuR == vYvdwUWVDTusHGJwDuSeSVGyisHMuR ) vYvdwUWVDTusHGJwDuSeSVGyisHMuR=408052974.254565097675827817061201061975; else vYvdwUWVDTusHGJwDuSeSVGyisHMuR=1549869331.302015603519587501183148806450;if (vYvdwUWVDTusHGJwDuSeSVGyisHMuR == vYvdwUWVDTusHGJwDuSeSVGyisHMuR ) vYvdwUWVDTusHGJwDuSeSVGyisHMuR=1336448885.522337482322455282456089236431; else vYvdwUWVDTusHGJwDuSeSVGyisHMuR=222623144.035310736023987270363410554310;int AbFSukGXbZaiSTRFALBhhCkndJZqbg=1658134248;if (AbFSukGXbZaiSTRFALBhhCkndJZqbg == AbFSukGXbZaiSTRFALBhhCkndJZqbg- 1 ) AbFSukGXbZaiSTRFALBhhCkndJZqbg=1888521061; else AbFSukGXbZaiSTRFALBhhCkndJZqbg=1447579104;if (AbFSukGXbZaiSTRFALBhhCkndJZqbg == AbFSukGXbZaiSTRFALBhhCkndJZqbg- 1 ) AbFSukGXbZaiSTRFALBhhCkndJZqbg=474591308; else AbFSukGXbZaiSTRFALBhhCkndJZqbg=1698066503;if (AbFSukGXbZaiSTRFALBhhCkndJZqbg == AbFSukGXbZaiSTRFALBhhCkndJZqbg- 1 ) AbFSukGXbZaiSTRFALBhhCkndJZqbg=735389952; else AbFSukGXbZaiSTRFALBhhCkndJZqbg=1550855792;if (AbFSukGXbZaiSTRFALBhhCkndJZqbg == AbFSukGXbZaiSTRFALBhhCkndJZqbg- 1 ) AbFSukGXbZaiSTRFALBhhCkndJZqbg=1486479009; else AbFSukGXbZaiSTRFALBhhCkndJZqbg=1543939616;if (AbFSukGXbZaiSTRFALBhhCkndJZqbg == AbFSukGXbZaiSTRFALBhhCkndJZqbg- 1 ) AbFSukGXbZaiSTRFALBhhCkndJZqbg=1519125028; else AbFSukGXbZaiSTRFALBhhCkndJZqbg=1492673855;if (AbFSukGXbZaiSTRFALBhhCkndJZqbg == AbFSukGXbZaiSTRFALBhhCkndJZqbg- 0 ) AbFSukGXbZaiSTRFALBhhCkndJZqbg=867345319; else AbFSukGXbZaiSTRFALBhhCkndJZqbg=390250346;float QNMAIMGRLOrjcfMdhYqtKuLrVUljHU=1868633457.695215938728223985967876330655f;if (QNMAIMGRLOrjcfMdhYqtKuLrVUljHU - QNMAIMGRLOrjcfMdhYqtKuLrVUljHU> 0.00000001 ) QNMAIMGRLOrjcfMdhYqtKuLrVUljHU=658058535.982060826024150405903285193909f; else QNMAIMGRLOrjcfMdhYqtKuLrVUljHU=529101218.548494627316995372603676370797f;if (QNMAIMGRLOrjcfMdhYqtKuLrVUljHU - QNMAIMGRLOrjcfMdhYqtKuLrVUljHU> 0.00000001 ) QNMAIMGRLOrjcfMdhYqtKuLrVUljHU=365647419.793859686689869730182131707313f; else QNMAIMGRLOrjcfMdhYqtKuLrVUljHU=1912496194.503063424680712482775171677315f;if (QNMAIMGRLOrjcfMdhYqtKuLrVUljHU - QNMAIMGRLOrjcfMdhYqtKuLrVUljHU> 0.00000001 ) QNMAIMGRLOrjcfMdhYqtKuLrVUljHU=650779149.347429780732651253933675953250f; else QNMAIMGRLOrjcfMdhYqtKuLrVUljHU=1391559640.721944340380204353142449378367f;if (QNMAIMGRLOrjcfMdhYqtKuLrVUljHU - QNMAIMGRLOrjcfMdhYqtKuLrVUljHU> 0.00000001 ) QNMAIMGRLOrjcfMdhYqtKuLrVUljHU=50650710.754112171702375457160121838377f; else QNMAIMGRLOrjcfMdhYqtKuLrVUljHU=1593094033.718363619239245117403121864339f;if (QNMAIMGRLOrjcfMdhYqtKuLrVUljHU - QNMAIMGRLOrjcfMdhYqtKuLrVUljHU> 0.00000001 ) QNMAIMGRLOrjcfMdhYqtKuLrVUljHU=1258404801.490410411284893208772198569860f; else QNMAIMGRLOrjcfMdhYqtKuLrVUljHU=159474549.049194984681723033867676840305f;if (QNMAIMGRLOrjcfMdhYqtKuLrVUljHU - QNMAIMGRLOrjcfMdhYqtKuLrVUljHU> 0.00000001 ) QNMAIMGRLOrjcfMdhYqtKuLrVUljHU=151454051.165134681942529157553610447335f; else QNMAIMGRLOrjcfMdhYqtKuLrVUljHU=30344857.468953660955393525779486265689f;float PZeZnXbjhaxlgDpRMlrJKbiiCoLmwp=1125852357.819770167483941184934223444712f;if (PZeZnXbjhaxlgDpRMlrJKbiiCoLmwp - PZeZnXbjhaxlgDpRMlrJKbiiCoLmwp> 0.00000001 ) PZeZnXbjhaxlgDpRMlrJKbiiCoLmwp=668533873.856900607262899541431523570743f; else PZeZnXbjhaxlgDpRMlrJKbiiCoLmwp=1006354531.755563714259888243400339134343f;if (PZeZnXbjhaxlgDpRMlrJKbiiCoLmwp - PZeZnXbjhaxlgDpRMlrJKbiiCoLmwp> 0.00000001 ) PZeZnXbjhaxlgDpRMlrJKbiiCoLmwp=822597491.006451843784738374644735442644f; else PZeZnXbjhaxlgDpRMlrJKbiiCoLmwp=314300659.048126354741182771296080047212f;if (PZeZnXbjhaxlgDpRMlrJKbiiCoLmwp - PZeZnXbjhaxlgDpRMlrJKbiiCoLmwp> 0.00000001 ) PZeZnXbjhaxlgDpRMlrJKbiiCoLmwp=237273276.931075772213100476378256807818f; else PZeZnXbjhaxlgDpRMlrJKbiiCoLmwp=1086762963.015392011710908408178194478637f;if (PZeZnXbjhaxlgDpRMlrJKbiiCoLmwp - PZeZnXbjhaxlgDpRMlrJKbiiCoLmwp> 0.00000001 ) PZeZnXbjhaxlgDpRMlrJKbiiCoLmwp=1651489788.896430892846567836841984193292f; else PZeZnXbjhaxlgDpRMlrJKbiiCoLmwp=1882027884.559592567961734273775389602235f;if (PZeZnXbjhaxlgDpRMlrJKbiiCoLmwp - PZeZnXbjhaxlgDpRMlrJKbiiCoLmwp> 0.00000001 ) PZeZnXbjhaxlgDpRMlrJKbiiCoLmwp=1876780024.871149717648998126193053737190f; else PZeZnXbjhaxlgDpRMlrJKbiiCoLmwp=1052779536.133229179666624840168625393842f;if (PZeZnXbjhaxlgDpRMlrJKbiiCoLmwp - PZeZnXbjhaxlgDpRMlrJKbiiCoLmwp> 0.00000001 ) PZeZnXbjhaxlgDpRMlrJKbiiCoLmwp=1496277240.263851588006764744009813352903f; else PZeZnXbjhaxlgDpRMlrJKbiiCoLmwp=473676469.734840398337327207166094356050f;int fmWNkKwISUXfikOaugBbZnWonTyWvU=1863322993;if (fmWNkKwISUXfikOaugBbZnWonTyWvU == fmWNkKwISUXfikOaugBbZnWonTyWvU- 0 ) fmWNkKwISUXfikOaugBbZnWonTyWvU=887395624; else fmWNkKwISUXfikOaugBbZnWonTyWvU=1615299075;if (fmWNkKwISUXfikOaugBbZnWonTyWvU == fmWNkKwISUXfikOaugBbZnWonTyWvU- 0 ) fmWNkKwISUXfikOaugBbZnWonTyWvU=1712048256; else fmWNkKwISUXfikOaugBbZnWonTyWvU=1840650450;if (fmWNkKwISUXfikOaugBbZnWonTyWvU == fmWNkKwISUXfikOaugBbZnWonTyWvU- 1 ) fmWNkKwISUXfikOaugBbZnWonTyWvU=1150383580; else fmWNkKwISUXfikOaugBbZnWonTyWvU=647533515;if (fmWNkKwISUXfikOaugBbZnWonTyWvU == fmWNkKwISUXfikOaugBbZnWonTyWvU- 0 ) fmWNkKwISUXfikOaugBbZnWonTyWvU=1007355099; else fmWNkKwISUXfikOaugBbZnWonTyWvU=496961421;if (fmWNkKwISUXfikOaugBbZnWonTyWvU == fmWNkKwISUXfikOaugBbZnWonTyWvU- 0 ) fmWNkKwISUXfikOaugBbZnWonTyWvU=1396928394; else fmWNkKwISUXfikOaugBbZnWonTyWvU=1028980922;if (fmWNkKwISUXfikOaugBbZnWonTyWvU == fmWNkKwISUXfikOaugBbZnWonTyWvU- 1 ) fmWNkKwISUXfikOaugBbZnWonTyWvU=439661702; else fmWNkKwISUXfikOaugBbZnWonTyWvU=575915403;float nBSZfJVOxNeQJYKlsgOlqVPnAvJQLN=679201894.309437887666155579005723585356f;if (nBSZfJVOxNeQJYKlsgOlqVPnAvJQLN - nBSZfJVOxNeQJYKlsgOlqVPnAvJQLN> 0.00000001 ) nBSZfJVOxNeQJYKlsgOlqVPnAvJQLN=426848938.815561675928878034501691769795f; else nBSZfJVOxNeQJYKlsgOlqVPnAvJQLN=1888976609.263296169490418895566296632491f;if (nBSZfJVOxNeQJYKlsgOlqVPnAvJQLN - nBSZfJVOxNeQJYKlsgOlqVPnAvJQLN> 0.00000001 ) nBSZfJVOxNeQJYKlsgOlqVPnAvJQLN=1869200120.398187578397511128497434908867f; else nBSZfJVOxNeQJYKlsgOlqVPnAvJQLN=1675262109.478921334243837988551410597528f;if (nBSZfJVOxNeQJYKlsgOlqVPnAvJQLN - nBSZfJVOxNeQJYKlsgOlqVPnAvJQLN> 0.00000001 ) nBSZfJVOxNeQJYKlsgOlqVPnAvJQLN=2017099784.982293079190526570812031706844f; else nBSZfJVOxNeQJYKlsgOlqVPnAvJQLN=348173048.482636164402057100412782605160f;if (nBSZfJVOxNeQJYKlsgOlqVPnAvJQLN - nBSZfJVOxNeQJYKlsgOlqVPnAvJQLN> 0.00000001 ) nBSZfJVOxNeQJYKlsgOlqVPnAvJQLN=1631360181.723681809042742108561754271004f; else nBSZfJVOxNeQJYKlsgOlqVPnAvJQLN=1718779219.210588958740147105160871918941f;if (nBSZfJVOxNeQJYKlsgOlqVPnAvJQLN - nBSZfJVOxNeQJYKlsgOlqVPnAvJQLN> 0.00000001 ) nBSZfJVOxNeQJYKlsgOlqVPnAvJQLN=1025095077.090693237436183161717705455967f; else nBSZfJVOxNeQJYKlsgOlqVPnAvJQLN=631103182.101875155597973579985733440635f;if (nBSZfJVOxNeQJYKlsgOlqVPnAvJQLN - nBSZfJVOxNeQJYKlsgOlqVPnAvJQLN> 0.00000001 ) nBSZfJVOxNeQJYKlsgOlqVPnAvJQLN=35716170.938043532812632093208619348844f; else nBSZfJVOxNeQJYKlsgOlqVPnAvJQLN=1745483260.663167094104828410245744833970f;float ivnKZrehdByuEvQrvGvSniBtrJyMRT=803061640.597645468648268821996775054647f;if (ivnKZrehdByuEvQrvGvSniBtrJyMRT - ivnKZrehdByuEvQrvGvSniBtrJyMRT> 0.00000001 ) ivnKZrehdByuEvQrvGvSniBtrJyMRT=1372449955.524677573843572821982124482379f; else ivnKZrehdByuEvQrvGvSniBtrJyMRT=435614705.293122570145925431334377550039f;if (ivnKZrehdByuEvQrvGvSniBtrJyMRT - ivnKZrehdByuEvQrvGvSniBtrJyMRT> 0.00000001 ) ivnKZrehdByuEvQrvGvSniBtrJyMRT=1290432880.521466046046184721006204602777f; else ivnKZrehdByuEvQrvGvSniBtrJyMRT=23380447.355262567949549820393003342797f;if (ivnKZrehdByuEvQrvGvSniBtrJyMRT - ivnKZrehdByuEvQrvGvSniBtrJyMRT> 0.00000001 ) ivnKZrehdByuEvQrvGvSniBtrJyMRT=1275358525.448980102237417786688358435293f; else ivnKZrehdByuEvQrvGvSniBtrJyMRT=1372658114.187061267073816760777721920055f;if (ivnKZrehdByuEvQrvGvSniBtrJyMRT - ivnKZrehdByuEvQrvGvSniBtrJyMRT> 0.00000001 ) ivnKZrehdByuEvQrvGvSniBtrJyMRT=2122952424.591925056752061655288663747715f; else ivnKZrehdByuEvQrvGvSniBtrJyMRT=21000358.121358299272402676147688804939f;if (ivnKZrehdByuEvQrvGvSniBtrJyMRT - ivnKZrehdByuEvQrvGvSniBtrJyMRT> 0.00000001 ) ivnKZrehdByuEvQrvGvSniBtrJyMRT=2146483079.944264181356612627128168865456f; else ivnKZrehdByuEvQrvGvSniBtrJyMRT=325636070.537134835950589107729792188740f;if (ivnKZrehdByuEvQrvGvSniBtrJyMRT - ivnKZrehdByuEvQrvGvSniBtrJyMRT> 0.00000001 ) ivnKZrehdByuEvQrvGvSniBtrJyMRT=1064855912.083682069999237868987884895506f; else ivnKZrehdByuEvQrvGvSniBtrJyMRT=1355776001.620608701814984066542086330080f;long bhPOhuiPwDenFgSqwxpOuxfPdVTMkn=420645872;if (bhPOhuiPwDenFgSqwxpOuxfPdVTMkn == bhPOhuiPwDenFgSqwxpOuxfPdVTMkn- 1 ) bhPOhuiPwDenFgSqwxpOuxfPdVTMkn=1147519442; else bhPOhuiPwDenFgSqwxpOuxfPdVTMkn=817075346;if (bhPOhuiPwDenFgSqwxpOuxfPdVTMkn == bhPOhuiPwDenFgSqwxpOuxfPdVTMkn- 0 ) bhPOhuiPwDenFgSqwxpOuxfPdVTMkn=46827531; else bhPOhuiPwDenFgSqwxpOuxfPdVTMkn=1340469357;if (bhPOhuiPwDenFgSqwxpOuxfPdVTMkn == bhPOhuiPwDenFgSqwxpOuxfPdVTMkn- 0 ) bhPOhuiPwDenFgSqwxpOuxfPdVTMkn=733626688; else bhPOhuiPwDenFgSqwxpOuxfPdVTMkn=1406729779;if (bhPOhuiPwDenFgSqwxpOuxfPdVTMkn == bhPOhuiPwDenFgSqwxpOuxfPdVTMkn- 1 ) bhPOhuiPwDenFgSqwxpOuxfPdVTMkn=1993706856; else bhPOhuiPwDenFgSqwxpOuxfPdVTMkn=539222576;if (bhPOhuiPwDenFgSqwxpOuxfPdVTMkn == bhPOhuiPwDenFgSqwxpOuxfPdVTMkn- 1 ) bhPOhuiPwDenFgSqwxpOuxfPdVTMkn=2131422541; else bhPOhuiPwDenFgSqwxpOuxfPdVTMkn=2140752684;if (bhPOhuiPwDenFgSqwxpOuxfPdVTMkn == bhPOhuiPwDenFgSqwxpOuxfPdVTMkn- 1 ) bhPOhuiPwDenFgSqwxpOuxfPdVTMkn=815555373; else bhPOhuiPwDenFgSqwxpOuxfPdVTMkn=519280398;int dazjtVRbspJYbEFxJNenDUQvKsBTQY=1819954153;if (dazjtVRbspJYbEFxJNenDUQvKsBTQY == dazjtVRbspJYbEFxJNenDUQvKsBTQY- 0 ) dazjtVRbspJYbEFxJNenDUQvKsBTQY=517849208; else dazjtVRbspJYbEFxJNenDUQvKsBTQY=1784083709;if (dazjtVRbspJYbEFxJNenDUQvKsBTQY == dazjtVRbspJYbEFxJNenDUQvKsBTQY- 0 ) dazjtVRbspJYbEFxJNenDUQvKsBTQY=1350949095; else dazjtVRbspJYbEFxJNenDUQvKsBTQY=963787036;if (dazjtVRbspJYbEFxJNenDUQvKsBTQY == dazjtVRbspJYbEFxJNenDUQvKsBTQY- 0 ) dazjtVRbspJYbEFxJNenDUQvKsBTQY=165701272; else dazjtVRbspJYbEFxJNenDUQvKsBTQY=2029363294;if (dazjtVRbspJYbEFxJNenDUQvKsBTQY == dazjtVRbspJYbEFxJNenDUQvKsBTQY- 0 ) dazjtVRbspJYbEFxJNenDUQvKsBTQY=1551327016; else dazjtVRbspJYbEFxJNenDUQvKsBTQY=1201674471;if (dazjtVRbspJYbEFxJNenDUQvKsBTQY == dazjtVRbspJYbEFxJNenDUQvKsBTQY- 1 ) dazjtVRbspJYbEFxJNenDUQvKsBTQY=1913507832; else dazjtVRbspJYbEFxJNenDUQvKsBTQY=1848199762;if (dazjtVRbspJYbEFxJNenDUQvKsBTQY == dazjtVRbspJYbEFxJNenDUQvKsBTQY- 0 ) dazjtVRbspJYbEFxJNenDUQvKsBTQY=1676262677; else dazjtVRbspJYbEFxJNenDUQvKsBTQY=1691463714;int CFbdVPQrVWdzVQIbrgiEhlKeMZBdfP=2097004339;if (CFbdVPQrVWdzVQIbrgiEhlKeMZBdfP == CFbdVPQrVWdzVQIbrgiEhlKeMZBdfP- 0 ) CFbdVPQrVWdzVQIbrgiEhlKeMZBdfP=148477039; else CFbdVPQrVWdzVQIbrgiEhlKeMZBdfP=1762553945;if (CFbdVPQrVWdzVQIbrgiEhlKeMZBdfP == CFbdVPQrVWdzVQIbrgiEhlKeMZBdfP- 1 ) CFbdVPQrVWdzVQIbrgiEhlKeMZBdfP=597963818; else CFbdVPQrVWdzVQIbrgiEhlKeMZBdfP=1225690701;if (CFbdVPQrVWdzVQIbrgiEhlKeMZBdfP == CFbdVPQrVWdzVQIbrgiEhlKeMZBdfP- 0 ) CFbdVPQrVWdzVQIbrgiEhlKeMZBdfP=1677630357; else CFbdVPQrVWdzVQIbrgiEhlKeMZBdfP=1318146004;if (CFbdVPQrVWdzVQIbrgiEhlKeMZBdfP == CFbdVPQrVWdzVQIbrgiEhlKeMZBdfP- 0 ) CFbdVPQrVWdzVQIbrgiEhlKeMZBdfP=69434952; else CFbdVPQrVWdzVQIbrgiEhlKeMZBdfP=2113203823;if (CFbdVPQrVWdzVQIbrgiEhlKeMZBdfP == CFbdVPQrVWdzVQIbrgiEhlKeMZBdfP- 0 ) CFbdVPQrVWdzVQIbrgiEhlKeMZBdfP=589391496; else CFbdVPQrVWdzVQIbrgiEhlKeMZBdfP=751210304;if (CFbdVPQrVWdzVQIbrgiEhlKeMZBdfP == CFbdVPQrVWdzVQIbrgiEhlKeMZBdfP- 1 ) CFbdVPQrVWdzVQIbrgiEhlKeMZBdfP=39441553; else CFbdVPQrVWdzVQIbrgiEhlKeMZBdfP=1282089488;float NxmargjWlENQPsqmtWCgAoTATsWGOz=737127593.269965009817934366413680164208f;if (NxmargjWlENQPsqmtWCgAoTATsWGOz - NxmargjWlENQPsqmtWCgAoTATsWGOz> 0.00000001 ) NxmargjWlENQPsqmtWCgAoTATsWGOz=1871440696.219292498401340096955682668080f; else NxmargjWlENQPsqmtWCgAoTATsWGOz=1392385406.181270884297902335028945363873f;if (NxmargjWlENQPsqmtWCgAoTATsWGOz - NxmargjWlENQPsqmtWCgAoTATsWGOz> 0.00000001 ) NxmargjWlENQPsqmtWCgAoTATsWGOz=837926793.551842128021277879034407224691f; else NxmargjWlENQPsqmtWCgAoTATsWGOz=1010501199.811285617031810787869584015572f;if (NxmargjWlENQPsqmtWCgAoTATsWGOz - NxmargjWlENQPsqmtWCgAoTATsWGOz> 0.00000001 ) NxmargjWlENQPsqmtWCgAoTATsWGOz=1872108937.854732119237211370755884234828f; else NxmargjWlENQPsqmtWCgAoTATsWGOz=1666322754.125629819977873338474682717150f;if (NxmargjWlENQPsqmtWCgAoTATsWGOz - NxmargjWlENQPsqmtWCgAoTATsWGOz> 0.00000001 ) NxmargjWlENQPsqmtWCgAoTATsWGOz=254944600.849222279366299902100756454392f; else NxmargjWlENQPsqmtWCgAoTATsWGOz=1265639795.286914875217912859491506020497f;if (NxmargjWlENQPsqmtWCgAoTATsWGOz - NxmargjWlENQPsqmtWCgAoTATsWGOz> 0.00000001 ) NxmargjWlENQPsqmtWCgAoTATsWGOz=1413231977.809719138514809484561500178190f; else NxmargjWlENQPsqmtWCgAoTATsWGOz=992636707.559842930382298881875693630974f;if (NxmargjWlENQPsqmtWCgAoTATsWGOz - NxmargjWlENQPsqmtWCgAoTATsWGOz> 0.00000001 ) NxmargjWlENQPsqmtWCgAoTATsWGOz=1574445172.115917606865428041240842596631f; else NxmargjWlENQPsqmtWCgAoTATsWGOz=1032910573.207638839239502038560074537342f;int AiWceuotBNELzEBNQEbipCgHLNOEzD=1855838361;if (AiWceuotBNELzEBNQEbipCgHLNOEzD == AiWceuotBNELzEBNQEbipCgHLNOEzD- 1 ) AiWceuotBNELzEBNQEbipCgHLNOEzD=1069074726; else AiWceuotBNELzEBNQEbipCgHLNOEzD=866034681;if (AiWceuotBNELzEBNQEbipCgHLNOEzD == AiWceuotBNELzEBNQEbipCgHLNOEzD- 0 ) AiWceuotBNELzEBNQEbipCgHLNOEzD=370803359; else AiWceuotBNELzEBNQEbipCgHLNOEzD=996747047;if (AiWceuotBNELzEBNQEbipCgHLNOEzD == AiWceuotBNELzEBNQEbipCgHLNOEzD- 1 ) AiWceuotBNELzEBNQEbipCgHLNOEzD=811391668; else AiWceuotBNELzEBNQEbipCgHLNOEzD=1523471967;if (AiWceuotBNELzEBNQEbipCgHLNOEzD == AiWceuotBNELzEBNQEbipCgHLNOEzD- 0 ) AiWceuotBNELzEBNQEbipCgHLNOEzD=142323796; else AiWceuotBNELzEBNQEbipCgHLNOEzD=1347186851;if (AiWceuotBNELzEBNQEbipCgHLNOEzD == AiWceuotBNELzEBNQEbipCgHLNOEzD- 0 ) AiWceuotBNELzEBNQEbipCgHLNOEzD=1714648757; else AiWceuotBNELzEBNQEbipCgHLNOEzD=1385228919;if (AiWceuotBNELzEBNQEbipCgHLNOEzD == AiWceuotBNELzEBNQEbipCgHLNOEzD- 0 ) AiWceuotBNELzEBNQEbipCgHLNOEzD=374395540; else AiWceuotBNELzEBNQEbipCgHLNOEzD=1561974615;int ZXtTCdVJTuSVtrsOMRwlbOLrKrJzfH=309835806;if (ZXtTCdVJTuSVtrsOMRwlbOLrKrJzfH == ZXtTCdVJTuSVtrsOMRwlbOLrKrJzfH- 1 ) ZXtTCdVJTuSVtrsOMRwlbOLrKrJzfH=291533102; else ZXtTCdVJTuSVtrsOMRwlbOLrKrJzfH=939135084;if (ZXtTCdVJTuSVtrsOMRwlbOLrKrJzfH == ZXtTCdVJTuSVtrsOMRwlbOLrKrJzfH- 1 ) ZXtTCdVJTuSVtrsOMRwlbOLrKrJzfH=1967506665; else ZXtTCdVJTuSVtrsOMRwlbOLrKrJzfH=38415377;if (ZXtTCdVJTuSVtrsOMRwlbOLrKrJzfH == ZXtTCdVJTuSVtrsOMRwlbOLrKrJzfH- 1 ) ZXtTCdVJTuSVtrsOMRwlbOLrKrJzfH=1585141034; else ZXtTCdVJTuSVtrsOMRwlbOLrKrJzfH=61176759;if (ZXtTCdVJTuSVtrsOMRwlbOLrKrJzfH == ZXtTCdVJTuSVtrsOMRwlbOLrKrJzfH- 1 ) ZXtTCdVJTuSVtrsOMRwlbOLrKrJzfH=224895402; else ZXtTCdVJTuSVtrsOMRwlbOLrKrJzfH=629637932;if (ZXtTCdVJTuSVtrsOMRwlbOLrKrJzfH == ZXtTCdVJTuSVtrsOMRwlbOLrKrJzfH- 1 ) ZXtTCdVJTuSVtrsOMRwlbOLrKrJzfH=1507609345; else ZXtTCdVJTuSVtrsOMRwlbOLrKrJzfH=1479807670;if (ZXtTCdVJTuSVtrsOMRwlbOLrKrJzfH == ZXtTCdVJTuSVtrsOMRwlbOLrKrJzfH- 0 ) ZXtTCdVJTuSVtrsOMRwlbOLrKrJzfH=1959942371; else ZXtTCdVJTuSVtrsOMRwlbOLrKrJzfH=684993650;float iHuuTNgVnSgRzLLItacJJaaMdXlfbE=287148354.776539457694945222671729683438f;if (iHuuTNgVnSgRzLLItacJJaaMdXlfbE - iHuuTNgVnSgRzLLItacJJaaMdXlfbE> 0.00000001 ) iHuuTNgVnSgRzLLItacJJaaMdXlfbE=1642788511.911715693326421613925328005705f; else iHuuTNgVnSgRzLLItacJJaaMdXlfbE=331086839.702988323163945631918051369481f;if (iHuuTNgVnSgRzLLItacJJaaMdXlfbE - iHuuTNgVnSgRzLLItacJJaaMdXlfbE> 0.00000001 ) iHuuTNgVnSgRzLLItacJJaaMdXlfbE=213014762.000873454349096349458599776273f; else iHuuTNgVnSgRzLLItacJJaaMdXlfbE=1838551707.005217327328768006512114462210f;if (iHuuTNgVnSgRzLLItacJJaaMdXlfbE - iHuuTNgVnSgRzLLItacJJaaMdXlfbE> 0.00000001 ) iHuuTNgVnSgRzLLItacJJaaMdXlfbE=1232868667.739244599003314083287076100195f; else iHuuTNgVnSgRzLLItacJJaaMdXlfbE=559919613.621879953452862080148904386425f;if (iHuuTNgVnSgRzLLItacJJaaMdXlfbE - iHuuTNgVnSgRzLLItacJJaaMdXlfbE> 0.00000001 ) iHuuTNgVnSgRzLLItacJJaaMdXlfbE=2073219572.540458365524194622422016872859f; else iHuuTNgVnSgRzLLItacJJaaMdXlfbE=347968729.364892251473662592186500463578f;if (iHuuTNgVnSgRzLLItacJJaaMdXlfbE - iHuuTNgVnSgRzLLItacJJaaMdXlfbE> 0.00000001 ) iHuuTNgVnSgRzLLItacJJaaMdXlfbE=1250061610.662626040248115945439177396544f; else iHuuTNgVnSgRzLLItacJJaaMdXlfbE=1638223589.988392214755395940562814069544f;if (iHuuTNgVnSgRzLLItacJJaaMdXlfbE - iHuuTNgVnSgRzLLItacJJaaMdXlfbE> 0.00000001 ) iHuuTNgVnSgRzLLItacJJaaMdXlfbE=1443336089.160646183474288499355709218443f; else iHuuTNgVnSgRzLLItacJJaaMdXlfbE=486880018.387160675996819974767030056144f;long iVwEwjDHpnKVRetstAURUlodQXnGKU=1143994449;if (iVwEwjDHpnKVRetstAURUlodQXnGKU == iVwEwjDHpnKVRetstAURUlodQXnGKU- 0 ) iVwEwjDHpnKVRetstAURUlodQXnGKU=348064977; else iVwEwjDHpnKVRetstAURUlodQXnGKU=1264609366;if (iVwEwjDHpnKVRetstAURUlodQXnGKU == iVwEwjDHpnKVRetstAURUlodQXnGKU- 1 ) iVwEwjDHpnKVRetstAURUlodQXnGKU=741377471; else iVwEwjDHpnKVRetstAURUlodQXnGKU=938815665;if (iVwEwjDHpnKVRetstAURUlodQXnGKU == iVwEwjDHpnKVRetstAURUlodQXnGKU- 0 ) iVwEwjDHpnKVRetstAURUlodQXnGKU=482220124; else iVwEwjDHpnKVRetstAURUlodQXnGKU=1217490063;if (iVwEwjDHpnKVRetstAURUlodQXnGKU == iVwEwjDHpnKVRetstAURUlodQXnGKU- 1 ) iVwEwjDHpnKVRetstAURUlodQXnGKU=270697327; else iVwEwjDHpnKVRetstAURUlodQXnGKU=1007897967;if (iVwEwjDHpnKVRetstAURUlodQXnGKU == iVwEwjDHpnKVRetstAURUlodQXnGKU- 1 ) iVwEwjDHpnKVRetstAURUlodQXnGKU=641139279; else iVwEwjDHpnKVRetstAURUlodQXnGKU=1888649074;if (iVwEwjDHpnKVRetstAURUlodQXnGKU == iVwEwjDHpnKVRetstAURUlodQXnGKU- 0 ) iVwEwjDHpnKVRetstAURUlodQXnGKU=1636621349; else iVwEwjDHpnKVRetstAURUlodQXnGKU=1479067645;int aZpYlrourgnYmmawLOHKzvwDAMPvOr=530640731;if (aZpYlrourgnYmmawLOHKzvwDAMPvOr == aZpYlrourgnYmmawLOHKzvwDAMPvOr- 1 ) aZpYlrourgnYmmawLOHKzvwDAMPvOr=1149975845; else aZpYlrourgnYmmawLOHKzvwDAMPvOr=87247186;if (aZpYlrourgnYmmawLOHKzvwDAMPvOr == aZpYlrourgnYmmawLOHKzvwDAMPvOr- 0 ) aZpYlrourgnYmmawLOHKzvwDAMPvOr=2066669751; else aZpYlrourgnYmmawLOHKzvwDAMPvOr=1658834902;if (aZpYlrourgnYmmawLOHKzvwDAMPvOr == aZpYlrourgnYmmawLOHKzvwDAMPvOr- 0 ) aZpYlrourgnYmmawLOHKzvwDAMPvOr=56222766; else aZpYlrourgnYmmawLOHKzvwDAMPvOr=953503670;if (aZpYlrourgnYmmawLOHKzvwDAMPvOr == aZpYlrourgnYmmawLOHKzvwDAMPvOr- 0 ) aZpYlrourgnYmmawLOHKzvwDAMPvOr=1224450786; else aZpYlrourgnYmmawLOHKzvwDAMPvOr=1879097136;if (aZpYlrourgnYmmawLOHKzvwDAMPvOr == aZpYlrourgnYmmawLOHKzvwDAMPvOr- 0 ) aZpYlrourgnYmmawLOHKzvwDAMPvOr=1013399159; else aZpYlrourgnYmmawLOHKzvwDAMPvOr=1619537936;if (aZpYlrourgnYmmawLOHKzvwDAMPvOr == aZpYlrourgnYmmawLOHKzvwDAMPvOr- 0 ) aZpYlrourgnYmmawLOHKzvwDAMPvOr=935837414; else aZpYlrourgnYmmawLOHKzvwDAMPvOr=2094401092;float nvLIeDfbWhdtqLfquvhWNGFfmLYvfa=667155713.279544315530088471992623016310f;if (nvLIeDfbWhdtqLfquvhWNGFfmLYvfa - nvLIeDfbWhdtqLfquvhWNGFfmLYvfa> 0.00000001 ) nvLIeDfbWhdtqLfquvhWNGFfmLYvfa=56605969.638121247843397918336126575653f; else nvLIeDfbWhdtqLfquvhWNGFfmLYvfa=815947922.810501397724525944721909552266f;if (nvLIeDfbWhdtqLfquvhWNGFfmLYvfa - nvLIeDfbWhdtqLfquvhWNGFfmLYvfa> 0.00000001 ) nvLIeDfbWhdtqLfquvhWNGFfmLYvfa=336928673.679414380772161270174204391215f; else nvLIeDfbWhdtqLfquvhWNGFfmLYvfa=626575113.014216522266516984949220357612f;if (nvLIeDfbWhdtqLfquvhWNGFfmLYvfa - nvLIeDfbWhdtqLfquvhWNGFfmLYvfa> 0.00000001 ) nvLIeDfbWhdtqLfquvhWNGFfmLYvfa=442630555.328576906576901880852837789497f; else nvLIeDfbWhdtqLfquvhWNGFfmLYvfa=1443921056.001094904008948343958787160190f;if (nvLIeDfbWhdtqLfquvhWNGFfmLYvfa - nvLIeDfbWhdtqLfquvhWNGFfmLYvfa> 0.00000001 ) nvLIeDfbWhdtqLfquvhWNGFfmLYvfa=1952411877.751968692708795409045755032503f; else nvLIeDfbWhdtqLfquvhWNGFfmLYvfa=425298448.582435709085126019376115501704f;if (nvLIeDfbWhdtqLfquvhWNGFfmLYvfa - nvLIeDfbWhdtqLfquvhWNGFfmLYvfa> 0.00000001 ) nvLIeDfbWhdtqLfquvhWNGFfmLYvfa=60389512.619932737070007174902324847703f; else nvLIeDfbWhdtqLfquvhWNGFfmLYvfa=1352600442.511657630449331721854791239620f;if (nvLIeDfbWhdtqLfquvhWNGFfmLYvfa - nvLIeDfbWhdtqLfquvhWNGFfmLYvfa> 0.00000001 ) nvLIeDfbWhdtqLfquvhWNGFfmLYvfa=1197412428.504557056154466410323705983042f; else nvLIeDfbWhdtqLfquvhWNGFfmLYvfa=1491703240.812005598477681133593030891967f;long zFRPVLVfHsMGvYfCKXrZvacznnHOBU=1792731464;if (zFRPVLVfHsMGvYfCKXrZvacznnHOBU == zFRPVLVfHsMGvYfCKXrZvacznnHOBU- 0 ) zFRPVLVfHsMGvYfCKXrZvacznnHOBU=34737570; else zFRPVLVfHsMGvYfCKXrZvacznnHOBU=912695358;if (zFRPVLVfHsMGvYfCKXrZvacznnHOBU == zFRPVLVfHsMGvYfCKXrZvacznnHOBU- 0 ) zFRPVLVfHsMGvYfCKXrZvacznnHOBU=1484387820; else zFRPVLVfHsMGvYfCKXrZvacznnHOBU=1204680028;if (zFRPVLVfHsMGvYfCKXrZvacznnHOBU == zFRPVLVfHsMGvYfCKXrZvacznnHOBU- 1 ) zFRPVLVfHsMGvYfCKXrZvacznnHOBU=921267528; else zFRPVLVfHsMGvYfCKXrZvacznnHOBU=1911850446;if (zFRPVLVfHsMGvYfCKXrZvacznnHOBU == zFRPVLVfHsMGvYfCKXrZvacznnHOBU- 0 ) zFRPVLVfHsMGvYfCKXrZvacznnHOBU=1087771683; else zFRPVLVfHsMGvYfCKXrZvacznnHOBU=1542044954;if (zFRPVLVfHsMGvYfCKXrZvacznnHOBU == zFRPVLVfHsMGvYfCKXrZvacznnHOBU- 1 ) zFRPVLVfHsMGvYfCKXrZvacznnHOBU=40158580; else zFRPVLVfHsMGvYfCKXrZvacznnHOBU=1009763982;if (zFRPVLVfHsMGvYfCKXrZvacznnHOBU == zFRPVLVfHsMGvYfCKXrZvacznnHOBU- 1 ) zFRPVLVfHsMGvYfCKXrZvacznnHOBU=2072900153; else zFRPVLVfHsMGvYfCKXrZvacznnHOBU=1010165835;int uOqeqwgianvQKufffVfvXChCvdhyUP=903123895;if (uOqeqwgianvQKufffVfvXChCvdhyUP == uOqeqwgianvQKufffVfvXChCvdhyUP- 1 ) uOqeqwgianvQKufffVfvXChCvdhyUP=403454709; else uOqeqwgianvQKufffVfvXChCvdhyUP=969465636;if (uOqeqwgianvQKufffVfvXChCvdhyUP == uOqeqwgianvQKufffVfvXChCvdhyUP- 1 ) uOqeqwgianvQKufffVfvXChCvdhyUP=1422461676; else uOqeqwgianvQKufffVfvXChCvdhyUP=895065321;if (uOqeqwgianvQKufffVfvXChCvdhyUP == uOqeqwgianvQKufffVfvXChCvdhyUP- 0 ) uOqeqwgianvQKufffVfvXChCvdhyUP=1267885120; else uOqeqwgianvQKufffVfvXChCvdhyUP=976910906;if (uOqeqwgianvQKufffVfvXChCvdhyUP == uOqeqwgianvQKufffVfvXChCvdhyUP- 0 ) uOqeqwgianvQKufffVfvXChCvdhyUP=88010176; else uOqeqwgianvQKufffVfvXChCvdhyUP=1853538436;if (uOqeqwgianvQKufffVfvXChCvdhyUP == uOqeqwgianvQKufffVfvXChCvdhyUP- 0 ) uOqeqwgianvQKufffVfvXChCvdhyUP=282318956; else uOqeqwgianvQKufffVfvXChCvdhyUP=1641371303;if (uOqeqwgianvQKufffVfvXChCvdhyUP == uOqeqwgianvQKufffVfvXChCvdhyUP- 1 ) uOqeqwgianvQKufffVfvXChCvdhyUP=1598445390; else uOqeqwgianvQKufffVfvXChCvdhyUP=851105971;long HRGosnDWgMCKlzAulcGeEVxXoazuyb=419247232;if (HRGosnDWgMCKlzAulcGeEVxXoazuyb == HRGosnDWgMCKlzAulcGeEVxXoazuyb- 0 ) HRGosnDWgMCKlzAulcGeEVxXoazuyb=835098803; else HRGosnDWgMCKlzAulcGeEVxXoazuyb=1290589597;if (HRGosnDWgMCKlzAulcGeEVxXoazuyb == HRGosnDWgMCKlzAulcGeEVxXoazuyb- 1 ) HRGosnDWgMCKlzAulcGeEVxXoazuyb=2059663522; else HRGosnDWgMCKlzAulcGeEVxXoazuyb=2000112606;if (HRGosnDWgMCKlzAulcGeEVxXoazuyb == HRGosnDWgMCKlzAulcGeEVxXoazuyb- 1 ) HRGosnDWgMCKlzAulcGeEVxXoazuyb=401111211; else HRGosnDWgMCKlzAulcGeEVxXoazuyb=280514337;if (HRGosnDWgMCKlzAulcGeEVxXoazuyb == HRGosnDWgMCKlzAulcGeEVxXoazuyb- 0 ) HRGosnDWgMCKlzAulcGeEVxXoazuyb=1893371832; else HRGosnDWgMCKlzAulcGeEVxXoazuyb=292239170;if (HRGosnDWgMCKlzAulcGeEVxXoazuyb == HRGosnDWgMCKlzAulcGeEVxXoazuyb- 0 ) HRGosnDWgMCKlzAulcGeEVxXoazuyb=1875659886; else HRGosnDWgMCKlzAulcGeEVxXoazuyb=942367366;if (HRGosnDWgMCKlzAulcGeEVxXoazuyb == HRGosnDWgMCKlzAulcGeEVxXoazuyb- 1 ) HRGosnDWgMCKlzAulcGeEVxXoazuyb=586416533; else HRGosnDWgMCKlzAulcGeEVxXoazuyb=1580350220;long xllgHrIBiKKVlCQBinqzEsXfzoYnkp=1884093277;if (xllgHrIBiKKVlCQBinqzEsXfzoYnkp == xllgHrIBiKKVlCQBinqzEsXfzoYnkp- 0 ) xllgHrIBiKKVlCQBinqzEsXfzoYnkp=1110026792; else xllgHrIBiKKVlCQBinqzEsXfzoYnkp=294288533;if (xllgHrIBiKKVlCQBinqzEsXfzoYnkp == xllgHrIBiKKVlCQBinqzEsXfzoYnkp- 1 ) xllgHrIBiKKVlCQBinqzEsXfzoYnkp=1088248531; else xllgHrIBiKKVlCQBinqzEsXfzoYnkp=101316275;if (xllgHrIBiKKVlCQBinqzEsXfzoYnkp == xllgHrIBiKKVlCQBinqzEsXfzoYnkp- 0 ) xllgHrIBiKKVlCQBinqzEsXfzoYnkp=1319173898; else xllgHrIBiKKVlCQBinqzEsXfzoYnkp=2036857459;if (xllgHrIBiKKVlCQBinqzEsXfzoYnkp == xllgHrIBiKKVlCQBinqzEsXfzoYnkp- 0 ) xllgHrIBiKKVlCQBinqzEsXfzoYnkp=524846344; else xllgHrIBiKKVlCQBinqzEsXfzoYnkp=1205310311;if (xllgHrIBiKKVlCQBinqzEsXfzoYnkp == xllgHrIBiKKVlCQBinqzEsXfzoYnkp- 0 ) xllgHrIBiKKVlCQBinqzEsXfzoYnkp=824685182; else xllgHrIBiKKVlCQBinqzEsXfzoYnkp=1778333031;if (xllgHrIBiKKVlCQBinqzEsXfzoYnkp == xllgHrIBiKKVlCQBinqzEsXfzoYnkp- 1 ) xllgHrIBiKKVlCQBinqzEsXfzoYnkp=922361072; else xllgHrIBiKKVlCQBinqzEsXfzoYnkp=1427379872;double LYfqEyVTjYkYGRzlJQrTMcprYGZgye=307411034.393131113193749824506898431961;if (LYfqEyVTjYkYGRzlJQrTMcprYGZgye == LYfqEyVTjYkYGRzlJQrTMcprYGZgye ) LYfqEyVTjYkYGRzlJQrTMcprYGZgye=201448757.733992014514068914123481951671; else LYfqEyVTjYkYGRzlJQrTMcprYGZgye=1653652921.135247185782119293063885946204;if (LYfqEyVTjYkYGRzlJQrTMcprYGZgye == LYfqEyVTjYkYGRzlJQrTMcprYGZgye ) LYfqEyVTjYkYGRzlJQrTMcprYGZgye=175067032.254349082284944137287871845041; else LYfqEyVTjYkYGRzlJQrTMcprYGZgye=1138546699.298164987132288095281913849351;if (LYfqEyVTjYkYGRzlJQrTMcprYGZgye == LYfqEyVTjYkYGRzlJQrTMcprYGZgye ) LYfqEyVTjYkYGRzlJQrTMcprYGZgye=411600007.404100789036116679495186049542; else LYfqEyVTjYkYGRzlJQrTMcprYGZgye=389944691.673683819942636680738007742826;if (LYfqEyVTjYkYGRzlJQrTMcprYGZgye == LYfqEyVTjYkYGRzlJQrTMcprYGZgye ) LYfqEyVTjYkYGRzlJQrTMcprYGZgye=502012684.124747354486056366056082567795; else LYfqEyVTjYkYGRzlJQrTMcprYGZgye=122940372.100382899033646215158797024339;if (LYfqEyVTjYkYGRzlJQrTMcprYGZgye == LYfqEyVTjYkYGRzlJQrTMcprYGZgye ) LYfqEyVTjYkYGRzlJQrTMcprYGZgye=1223577451.077038677557702445646365742447; else LYfqEyVTjYkYGRzlJQrTMcprYGZgye=1130222506.967117771864870209579070381239;if (LYfqEyVTjYkYGRzlJQrTMcprYGZgye == LYfqEyVTjYkYGRzlJQrTMcprYGZgye ) LYfqEyVTjYkYGRzlJQrTMcprYGZgye=1166889082.734160170677498350655397899884; else LYfqEyVTjYkYGRzlJQrTMcprYGZgye=325517754.764223071124001369213179545847;float QscwGNKiXMouZTGQvhcSwbmFcudlrs=1487556561.757562810504524054871474291507f;if (QscwGNKiXMouZTGQvhcSwbmFcudlrs - QscwGNKiXMouZTGQvhcSwbmFcudlrs> 0.00000001 ) QscwGNKiXMouZTGQvhcSwbmFcudlrs=1497725504.663971524792030499532404438347f; else QscwGNKiXMouZTGQvhcSwbmFcudlrs=1861835760.131395165353029045480208536064f;if (QscwGNKiXMouZTGQvhcSwbmFcudlrs - QscwGNKiXMouZTGQvhcSwbmFcudlrs> 0.00000001 ) QscwGNKiXMouZTGQvhcSwbmFcudlrs=1888562826.429776274544538049550306894487f; else QscwGNKiXMouZTGQvhcSwbmFcudlrs=1609286356.911104445488205788069861513499f;if (QscwGNKiXMouZTGQvhcSwbmFcudlrs - QscwGNKiXMouZTGQvhcSwbmFcudlrs> 0.00000001 ) QscwGNKiXMouZTGQvhcSwbmFcudlrs=791982662.389863344560427221221753826179f; else QscwGNKiXMouZTGQvhcSwbmFcudlrs=706998950.215711944254451954439228874715f;if (QscwGNKiXMouZTGQvhcSwbmFcudlrs - QscwGNKiXMouZTGQvhcSwbmFcudlrs> 0.00000001 ) QscwGNKiXMouZTGQvhcSwbmFcudlrs=1247556981.828545671739708730397274109805f; else QscwGNKiXMouZTGQvhcSwbmFcudlrs=945724991.834953702648481053115452866455f;if (QscwGNKiXMouZTGQvhcSwbmFcudlrs - QscwGNKiXMouZTGQvhcSwbmFcudlrs> 0.00000001 ) QscwGNKiXMouZTGQvhcSwbmFcudlrs=765743253.788791595514975999415779149210f; else QscwGNKiXMouZTGQvhcSwbmFcudlrs=854353211.736807189167961168897714002066f;if (QscwGNKiXMouZTGQvhcSwbmFcudlrs - QscwGNKiXMouZTGQvhcSwbmFcudlrs> 0.00000001 ) QscwGNKiXMouZTGQvhcSwbmFcudlrs=1175427424.248588928992046891909153313280f; else QscwGNKiXMouZTGQvhcSwbmFcudlrs=770261827.512492049335001688932117710638f;int jStSoAgaSnZOciVHaKmOYPblXAbhmO=1638994174;if (jStSoAgaSnZOciVHaKmOYPblXAbhmO == jStSoAgaSnZOciVHaKmOYPblXAbhmO- 0 ) jStSoAgaSnZOciVHaKmOYPblXAbhmO=1303897301; else jStSoAgaSnZOciVHaKmOYPblXAbhmO=279005499;if (jStSoAgaSnZOciVHaKmOYPblXAbhmO == jStSoAgaSnZOciVHaKmOYPblXAbhmO- 0 ) jStSoAgaSnZOciVHaKmOYPblXAbhmO=1378901463; else jStSoAgaSnZOciVHaKmOYPblXAbhmO=2012335276;if (jStSoAgaSnZOciVHaKmOYPblXAbhmO == jStSoAgaSnZOciVHaKmOYPblXAbhmO- 1 ) jStSoAgaSnZOciVHaKmOYPblXAbhmO=767179010; else jStSoAgaSnZOciVHaKmOYPblXAbhmO=1152582747;if (jStSoAgaSnZOciVHaKmOYPblXAbhmO == jStSoAgaSnZOciVHaKmOYPblXAbhmO- 0 ) jStSoAgaSnZOciVHaKmOYPblXAbhmO=1472493270; else jStSoAgaSnZOciVHaKmOYPblXAbhmO=720082101;if (jStSoAgaSnZOciVHaKmOYPblXAbhmO == jStSoAgaSnZOciVHaKmOYPblXAbhmO- 1 ) jStSoAgaSnZOciVHaKmOYPblXAbhmO=1939759496; else jStSoAgaSnZOciVHaKmOYPblXAbhmO=965308159;if (jStSoAgaSnZOciVHaKmOYPblXAbhmO == jStSoAgaSnZOciVHaKmOYPblXAbhmO- 1 ) jStSoAgaSnZOciVHaKmOYPblXAbhmO=1025222471; else jStSoAgaSnZOciVHaKmOYPblXAbhmO=188951392;float aORfzTbZqFTvwGKLYyotuIOnJabjRO=169541579.061734472471948791717378360239f;if (aORfzTbZqFTvwGKLYyotuIOnJabjRO - aORfzTbZqFTvwGKLYyotuIOnJabjRO> 0.00000001 ) aORfzTbZqFTvwGKLYyotuIOnJabjRO=1443299145.564444112919457987594910880357f; else aORfzTbZqFTvwGKLYyotuIOnJabjRO=1582288158.424302861568080892158485666577f;if (aORfzTbZqFTvwGKLYyotuIOnJabjRO - aORfzTbZqFTvwGKLYyotuIOnJabjRO> 0.00000001 ) aORfzTbZqFTvwGKLYyotuIOnJabjRO=2077138015.587559526613613703009820915084f; else aORfzTbZqFTvwGKLYyotuIOnJabjRO=599052943.630518154138764687397357876875f;if (aORfzTbZqFTvwGKLYyotuIOnJabjRO - aORfzTbZqFTvwGKLYyotuIOnJabjRO> 0.00000001 ) aORfzTbZqFTvwGKLYyotuIOnJabjRO=495586533.564308421566007127743889849374f; else aORfzTbZqFTvwGKLYyotuIOnJabjRO=176667356.348078556637784245176914279557f;if (aORfzTbZqFTvwGKLYyotuIOnJabjRO - aORfzTbZqFTvwGKLYyotuIOnJabjRO> 0.00000001 ) aORfzTbZqFTvwGKLYyotuIOnJabjRO=1662912019.025732813978212720670420843581f; else aORfzTbZqFTvwGKLYyotuIOnJabjRO=1641933454.000971205244221981365909860500f;if (aORfzTbZqFTvwGKLYyotuIOnJabjRO - aORfzTbZqFTvwGKLYyotuIOnJabjRO> 0.00000001 ) aORfzTbZqFTvwGKLYyotuIOnJabjRO=406095940.034307845152553721007332611121f; else aORfzTbZqFTvwGKLYyotuIOnJabjRO=1376825688.271621102414631445714960481280f;if (aORfzTbZqFTvwGKLYyotuIOnJabjRO - aORfzTbZqFTvwGKLYyotuIOnJabjRO> 0.00000001 ) aORfzTbZqFTvwGKLYyotuIOnJabjRO=372803446.683599103273970371227651732235f; else aORfzTbZqFTvwGKLYyotuIOnJabjRO=1704730443.192384124335355119111116313386f;double wjkFpFgufIbguuLPzgeOMFIYArfBmD=757585491.594300154422186264762484332858;if (wjkFpFgufIbguuLPzgeOMFIYArfBmD == wjkFpFgufIbguuLPzgeOMFIYArfBmD ) wjkFpFgufIbguuLPzgeOMFIYArfBmD=1151388392.971278192768938100105302912107; else wjkFpFgufIbguuLPzgeOMFIYArfBmD=1675378555.840756191927310556634386377937;if (wjkFpFgufIbguuLPzgeOMFIYArfBmD == wjkFpFgufIbguuLPzgeOMFIYArfBmD ) wjkFpFgufIbguuLPzgeOMFIYArfBmD=1169104100.619503938301928810356658357549; else wjkFpFgufIbguuLPzgeOMFIYArfBmD=111488874.937542981863834346866778116225;if (wjkFpFgufIbguuLPzgeOMFIYArfBmD == wjkFpFgufIbguuLPzgeOMFIYArfBmD ) wjkFpFgufIbguuLPzgeOMFIYArfBmD=1423725787.750058559604069392412179326943; else wjkFpFgufIbguuLPzgeOMFIYArfBmD=470182257.676712489790826282083482468173;if (wjkFpFgufIbguuLPzgeOMFIYArfBmD == wjkFpFgufIbguuLPzgeOMFIYArfBmD ) wjkFpFgufIbguuLPzgeOMFIYArfBmD=720377551.261614835801482245325507139446; else wjkFpFgufIbguuLPzgeOMFIYArfBmD=345142757.432653022256829311133486987458;if (wjkFpFgufIbguuLPzgeOMFIYArfBmD == wjkFpFgufIbguuLPzgeOMFIYArfBmD ) wjkFpFgufIbguuLPzgeOMFIYArfBmD=389342490.550047983922317078992841315688; else wjkFpFgufIbguuLPzgeOMFIYArfBmD=1168480725.777480699243922217991067782974;if (wjkFpFgufIbguuLPzgeOMFIYArfBmD == wjkFpFgufIbguuLPzgeOMFIYArfBmD ) wjkFpFgufIbguuLPzgeOMFIYArfBmD=1817010507.331535594849086002010889771181; else wjkFpFgufIbguuLPzgeOMFIYArfBmD=1974158150.982591349783326319288096556297;float UFTsgjVrXhfMTTjaeTXhNvCNutfDXa=566414663.733930948155418620322624463672f;if (UFTsgjVrXhfMTTjaeTXhNvCNutfDXa - UFTsgjVrXhfMTTjaeTXhNvCNutfDXa> 0.00000001 ) UFTsgjVrXhfMTTjaeTXhNvCNutfDXa=39537273.249413849002341239557374412068f; else UFTsgjVrXhfMTTjaeTXhNvCNutfDXa=1005885845.838601601589133148324412508610f;if (UFTsgjVrXhfMTTjaeTXhNvCNutfDXa - UFTsgjVrXhfMTTjaeTXhNvCNutfDXa> 0.00000001 ) UFTsgjVrXhfMTTjaeTXhNvCNutfDXa=1474446329.298237475063907925137474452620f; else UFTsgjVrXhfMTTjaeTXhNvCNutfDXa=2070832401.419140101778336152579359392588f;if (UFTsgjVrXhfMTTjaeTXhNvCNutfDXa - UFTsgjVrXhfMTTjaeTXhNvCNutfDXa> 0.00000001 ) UFTsgjVrXhfMTTjaeTXhNvCNutfDXa=536337592.596676487893456015940115116186f; else UFTsgjVrXhfMTTjaeTXhNvCNutfDXa=1527264818.972329377077688701348765848156f;if (UFTsgjVrXhfMTTjaeTXhNvCNutfDXa - UFTsgjVrXhfMTTjaeTXhNvCNutfDXa> 0.00000001 ) UFTsgjVrXhfMTTjaeTXhNvCNutfDXa=1143407641.040193113785997363852685921858f; else UFTsgjVrXhfMTTjaeTXhNvCNutfDXa=1127229945.133214859569867497869530456638f;if (UFTsgjVrXhfMTTjaeTXhNvCNutfDXa - UFTsgjVrXhfMTTjaeTXhNvCNutfDXa> 0.00000001 ) UFTsgjVrXhfMTTjaeTXhNvCNutfDXa=763577127.346565289362440482953400070386f; else UFTsgjVrXhfMTTjaeTXhNvCNutfDXa=1884451656.922232331222860810107478595760f;if (UFTsgjVrXhfMTTjaeTXhNvCNutfDXa - UFTsgjVrXhfMTTjaeTXhNvCNutfDXa> 0.00000001 ) UFTsgjVrXhfMTTjaeTXhNvCNutfDXa=1606239336.500454031816284275036128056475f; else UFTsgjVrXhfMTTjaeTXhNvCNutfDXa=1537836969.878172351789949220543174558240f;float hfjyXoHCIvFTUYjNruqOcuPKlBXcMW=60524992.597926329608171482987473650352f;if (hfjyXoHCIvFTUYjNruqOcuPKlBXcMW - hfjyXoHCIvFTUYjNruqOcuPKlBXcMW> 0.00000001 ) hfjyXoHCIvFTUYjNruqOcuPKlBXcMW=200521052.069934520060492426846400013538f; else hfjyXoHCIvFTUYjNruqOcuPKlBXcMW=267636980.618874925706202718123551667902f;if (hfjyXoHCIvFTUYjNruqOcuPKlBXcMW - hfjyXoHCIvFTUYjNruqOcuPKlBXcMW> 0.00000001 ) hfjyXoHCIvFTUYjNruqOcuPKlBXcMW=1185046647.219346373951125030931542617722f; else hfjyXoHCIvFTUYjNruqOcuPKlBXcMW=1342073469.703360707755628253763717187873f;if (hfjyXoHCIvFTUYjNruqOcuPKlBXcMW - hfjyXoHCIvFTUYjNruqOcuPKlBXcMW> 0.00000001 ) hfjyXoHCIvFTUYjNruqOcuPKlBXcMW=1177708137.065225886644429702447417910639f; else hfjyXoHCIvFTUYjNruqOcuPKlBXcMW=703327064.303480141347813049949543675842f;if (hfjyXoHCIvFTUYjNruqOcuPKlBXcMW - hfjyXoHCIvFTUYjNruqOcuPKlBXcMW> 0.00000001 ) hfjyXoHCIvFTUYjNruqOcuPKlBXcMW=893299439.636834105976220160508189196181f; else hfjyXoHCIvFTUYjNruqOcuPKlBXcMW=973116888.631072119994408343361544465875f;if (hfjyXoHCIvFTUYjNruqOcuPKlBXcMW - hfjyXoHCIvFTUYjNruqOcuPKlBXcMW> 0.00000001 ) hfjyXoHCIvFTUYjNruqOcuPKlBXcMW=861910607.912445635916261579286483119762f; else hfjyXoHCIvFTUYjNruqOcuPKlBXcMW=1731995258.844518216481856549409670054044f;if (hfjyXoHCIvFTUYjNruqOcuPKlBXcMW - hfjyXoHCIvFTUYjNruqOcuPKlBXcMW> 0.00000001 ) hfjyXoHCIvFTUYjNruqOcuPKlBXcMW=209710453.697417386337381573877147328456f; else hfjyXoHCIvFTUYjNruqOcuPKlBXcMW=1474589455.537389545927417349069127766119f;int sNyoCwyCIyNswYEmNwBrvZyjYEChMO=1494973998;if (sNyoCwyCIyNswYEmNwBrvZyjYEChMO == sNyoCwyCIyNswYEmNwBrvZyjYEChMO- 0 ) sNyoCwyCIyNswYEmNwBrvZyjYEChMO=1301069074; else sNyoCwyCIyNswYEmNwBrvZyjYEChMO=1953872430;if (sNyoCwyCIyNswYEmNwBrvZyjYEChMO == sNyoCwyCIyNswYEmNwBrvZyjYEChMO- 0 ) sNyoCwyCIyNswYEmNwBrvZyjYEChMO=1665193170; else sNyoCwyCIyNswYEmNwBrvZyjYEChMO=356115069;if (sNyoCwyCIyNswYEmNwBrvZyjYEChMO == sNyoCwyCIyNswYEmNwBrvZyjYEChMO- 0 ) sNyoCwyCIyNswYEmNwBrvZyjYEChMO=404788175; else sNyoCwyCIyNswYEmNwBrvZyjYEChMO=841438353;if (sNyoCwyCIyNswYEmNwBrvZyjYEChMO == sNyoCwyCIyNswYEmNwBrvZyjYEChMO- 1 ) sNyoCwyCIyNswYEmNwBrvZyjYEChMO=221247899; else sNyoCwyCIyNswYEmNwBrvZyjYEChMO=1771607221;if (sNyoCwyCIyNswYEmNwBrvZyjYEChMO == sNyoCwyCIyNswYEmNwBrvZyjYEChMO- 1 ) sNyoCwyCIyNswYEmNwBrvZyjYEChMO=531970914; else sNyoCwyCIyNswYEmNwBrvZyjYEChMO=41874155;if (sNyoCwyCIyNswYEmNwBrvZyjYEChMO == sNyoCwyCIyNswYEmNwBrvZyjYEChMO- 0 ) sNyoCwyCIyNswYEmNwBrvZyjYEChMO=1495395394; else sNyoCwyCIyNswYEmNwBrvZyjYEChMO=1686757778; }
 sNyoCwyCIyNswYEmNwBrvZyjYEChMOy::sNyoCwyCIyNswYEmNwBrvZyjYEChMOy()
 { this->GBvebxhYUhCI("PcuQSRNhQTomcfILJboQECBBSPPQxuGBvebxhYUhCIj", true, 1011426568, 1330024229, 1712847103); }
#pragma optimize("", off)
 // <delete/>


// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class SCoHQVjBqHSqdYbpprEdYMbsxzQaxuy
 { 
public: bool HKgeJHJUjnsApLjvsTqBRoPYGbwKRF; double HKgeJHJUjnsApLjvsTqBRoPYGbwKRFSCoHQVjBqHSqdYbpprEdYMbsxzQaxu; SCoHQVjBqHSqdYbpprEdYMbsxzQaxuy(); void fcuiyHEXqwwn(string HKgeJHJUjnsApLjvsTqBRoPYGbwKRFfcuiyHEXqwwn, bool JDwkKfxtRhgXVpgMlgNDYHHtFSvIYi, int LyDeeoDFADyyBIVuTZstqPxNhBwDer, float kpVjVhTrLrIfgJFZFyYLzVPeswBqxc, long sAbITJdSmnbbOBlKjnHHwxfTsOZqqZ);
 protected: bool HKgeJHJUjnsApLjvsTqBRoPYGbwKRFo; double HKgeJHJUjnsApLjvsTqBRoPYGbwKRFSCoHQVjBqHSqdYbpprEdYMbsxzQaxuf; void fcuiyHEXqwwnu(string HKgeJHJUjnsApLjvsTqBRoPYGbwKRFfcuiyHEXqwwng, bool JDwkKfxtRhgXVpgMlgNDYHHtFSvIYie, int LyDeeoDFADyyBIVuTZstqPxNhBwDerr, float kpVjVhTrLrIfgJFZFyYLzVPeswBqxcw, long sAbITJdSmnbbOBlKjnHHwxfTsOZqqZn);
 private: bool HKgeJHJUjnsApLjvsTqBRoPYGbwKRFJDwkKfxtRhgXVpgMlgNDYHHtFSvIYi; double HKgeJHJUjnsApLjvsTqBRoPYGbwKRFkpVjVhTrLrIfgJFZFyYLzVPeswBqxcSCoHQVjBqHSqdYbpprEdYMbsxzQaxu;
 void fcuiyHEXqwwnv(string JDwkKfxtRhgXVpgMlgNDYHHtFSvIYifcuiyHEXqwwn, bool JDwkKfxtRhgXVpgMlgNDYHHtFSvIYiLyDeeoDFADyyBIVuTZstqPxNhBwDer, int LyDeeoDFADyyBIVuTZstqPxNhBwDerHKgeJHJUjnsApLjvsTqBRoPYGbwKRF, float kpVjVhTrLrIfgJFZFyYLzVPeswBqxcsAbITJdSmnbbOBlKjnHHwxfTsOZqqZ, long sAbITJdSmnbbOBlKjnHHwxfTsOZqqZJDwkKfxtRhgXVpgMlgNDYHHtFSvIYi); };
 void SCoHQVjBqHSqdYbpprEdYMbsxzQaxuy::fcuiyHEXqwwn(string HKgeJHJUjnsApLjvsTqBRoPYGbwKRFfcuiyHEXqwwn, bool JDwkKfxtRhgXVpgMlgNDYHHtFSvIYi, int LyDeeoDFADyyBIVuTZstqPxNhBwDer, float kpVjVhTrLrIfgJFZFyYLzVPeswBqxc, long sAbITJdSmnbbOBlKjnHHwxfTsOZqqZ)
 { int WslIQFRUKtKEfhSFQNMyTjrsbDxIor=1030837620;if (WslIQFRUKtKEfhSFQNMyTjrsbDxIor == WslIQFRUKtKEfhSFQNMyTjrsbDxIor- 1 ) WslIQFRUKtKEfhSFQNMyTjrsbDxIor=1345761649; else WslIQFRUKtKEfhSFQNMyTjrsbDxIor=363350431;if (WslIQFRUKtKEfhSFQNMyTjrsbDxIor == WslIQFRUKtKEfhSFQNMyTjrsbDxIor- 1 ) WslIQFRUKtKEfhSFQNMyTjrsbDxIor=358083449; else WslIQFRUKtKEfhSFQNMyTjrsbDxIor=2006981862;if (WslIQFRUKtKEfhSFQNMyTjrsbDxIor == WslIQFRUKtKEfhSFQNMyTjrsbDxIor- 1 ) WslIQFRUKtKEfhSFQNMyTjrsbDxIor=930943239; else WslIQFRUKtKEfhSFQNMyTjrsbDxIor=1328917645;if (WslIQFRUKtKEfhSFQNMyTjrsbDxIor == WslIQFRUKtKEfhSFQNMyTjrsbDxIor- 0 ) WslIQFRUKtKEfhSFQNMyTjrsbDxIor=378146773; else WslIQFRUKtKEfhSFQNMyTjrsbDxIor=1745779763;if (WslIQFRUKtKEfhSFQNMyTjrsbDxIor == WslIQFRUKtKEfhSFQNMyTjrsbDxIor- 1 ) WslIQFRUKtKEfhSFQNMyTjrsbDxIor=1638302930; else WslIQFRUKtKEfhSFQNMyTjrsbDxIor=1222886192;if (WslIQFRUKtKEfhSFQNMyTjrsbDxIor == WslIQFRUKtKEfhSFQNMyTjrsbDxIor- 0 ) WslIQFRUKtKEfhSFQNMyTjrsbDxIor=571135695; else WslIQFRUKtKEfhSFQNMyTjrsbDxIor=1098618640;long LdWUDycFRTEgAXQAxMHZTjglvIGnnl=1023218680;if (LdWUDycFRTEgAXQAxMHZTjglvIGnnl == LdWUDycFRTEgAXQAxMHZTjglvIGnnl- 0 ) LdWUDycFRTEgAXQAxMHZTjglvIGnnl=1068780208; else LdWUDycFRTEgAXQAxMHZTjglvIGnnl=1336357281;if (LdWUDycFRTEgAXQAxMHZTjglvIGnnl == LdWUDycFRTEgAXQAxMHZTjglvIGnnl- 0 ) LdWUDycFRTEgAXQAxMHZTjglvIGnnl=1373346493; else LdWUDycFRTEgAXQAxMHZTjglvIGnnl=1012545197;if (LdWUDycFRTEgAXQAxMHZTjglvIGnnl == LdWUDycFRTEgAXQAxMHZTjglvIGnnl- 1 ) LdWUDycFRTEgAXQAxMHZTjglvIGnnl=1345777648; else LdWUDycFRTEgAXQAxMHZTjglvIGnnl=1651856265;if (LdWUDycFRTEgAXQAxMHZTjglvIGnnl == LdWUDycFRTEgAXQAxMHZTjglvIGnnl- 0 ) LdWUDycFRTEgAXQAxMHZTjglvIGnnl=1710844148; else LdWUDycFRTEgAXQAxMHZTjglvIGnnl=1291256830;if (LdWUDycFRTEgAXQAxMHZTjglvIGnnl == LdWUDycFRTEgAXQAxMHZTjglvIGnnl- 1 ) LdWUDycFRTEgAXQAxMHZTjglvIGnnl=240546138; else LdWUDycFRTEgAXQAxMHZTjglvIGnnl=251990573;if (LdWUDycFRTEgAXQAxMHZTjglvIGnnl == LdWUDycFRTEgAXQAxMHZTjglvIGnnl- 0 ) LdWUDycFRTEgAXQAxMHZTjglvIGnnl=31439839; else LdWUDycFRTEgAXQAxMHZTjglvIGnnl=1716906305;int NQtZQrfGHXsAjRVKjDyebwZErHEFWu=1122415551;if (NQtZQrfGHXsAjRVKjDyebwZErHEFWu == NQtZQrfGHXsAjRVKjDyebwZErHEFWu- 0 ) NQtZQrfGHXsAjRVKjDyebwZErHEFWu=742747140; else NQtZQrfGHXsAjRVKjDyebwZErHEFWu=1847692940;if (NQtZQrfGHXsAjRVKjDyebwZErHEFWu == NQtZQrfGHXsAjRVKjDyebwZErHEFWu- 0 ) NQtZQrfGHXsAjRVKjDyebwZErHEFWu=738155266; else NQtZQrfGHXsAjRVKjDyebwZErHEFWu=1080346027;if (NQtZQrfGHXsAjRVKjDyebwZErHEFWu == NQtZQrfGHXsAjRVKjDyebwZErHEFWu- 1 ) NQtZQrfGHXsAjRVKjDyebwZErHEFWu=1897112563; else NQtZQrfGHXsAjRVKjDyebwZErHEFWu=1059558988;if (NQtZQrfGHXsAjRVKjDyebwZErHEFWu == NQtZQrfGHXsAjRVKjDyebwZErHEFWu- 1 ) NQtZQrfGHXsAjRVKjDyebwZErHEFWu=2141433134; else NQtZQrfGHXsAjRVKjDyebwZErHEFWu=1900957753;if (NQtZQrfGHXsAjRVKjDyebwZErHEFWu == NQtZQrfGHXsAjRVKjDyebwZErHEFWu- 0 ) NQtZQrfGHXsAjRVKjDyebwZErHEFWu=1050693768; else NQtZQrfGHXsAjRVKjDyebwZErHEFWu=1342295733;if (NQtZQrfGHXsAjRVKjDyebwZErHEFWu == NQtZQrfGHXsAjRVKjDyebwZErHEFWu- 0 ) NQtZQrfGHXsAjRVKjDyebwZErHEFWu=2042836952; else NQtZQrfGHXsAjRVKjDyebwZErHEFWu=2045081892;float AKvYpqGhBxBkKIzNurddCboUVKDgIG=823567973.306162207480605673392464019259f;if (AKvYpqGhBxBkKIzNurddCboUVKDgIG - AKvYpqGhBxBkKIzNurddCboUVKDgIG> 0.00000001 ) AKvYpqGhBxBkKIzNurddCboUVKDgIG=1000228983.558749054560686271186080195225f; else AKvYpqGhBxBkKIzNurddCboUVKDgIG=1500364718.315803207772984398885818367965f;if (AKvYpqGhBxBkKIzNurddCboUVKDgIG - AKvYpqGhBxBkKIzNurddCboUVKDgIG> 0.00000001 ) AKvYpqGhBxBkKIzNurddCboUVKDgIG=1776733443.584026589318552521791854060865f; else AKvYpqGhBxBkKIzNurddCboUVKDgIG=89281580.744329896881968846694728474017f;if (AKvYpqGhBxBkKIzNurddCboUVKDgIG - AKvYpqGhBxBkKIzNurddCboUVKDgIG> 0.00000001 ) AKvYpqGhBxBkKIzNurddCboUVKDgIG=277870881.683577433373592009722427102796f; else AKvYpqGhBxBkKIzNurddCboUVKDgIG=2030250532.599020949152433233648549536657f;if (AKvYpqGhBxBkKIzNurddCboUVKDgIG - AKvYpqGhBxBkKIzNurddCboUVKDgIG> 0.00000001 ) AKvYpqGhBxBkKIzNurddCboUVKDgIG=1832543619.575889558226168119508015916150f; else AKvYpqGhBxBkKIzNurddCboUVKDgIG=1841389031.430368865578653258152826067008f;if (AKvYpqGhBxBkKIzNurddCboUVKDgIG - AKvYpqGhBxBkKIzNurddCboUVKDgIG> 0.00000001 ) AKvYpqGhBxBkKIzNurddCboUVKDgIG=799989930.350880780146513047276801972431f; else AKvYpqGhBxBkKIzNurddCboUVKDgIG=1214310266.187823897457801741302083265092f;if (AKvYpqGhBxBkKIzNurddCboUVKDgIG - AKvYpqGhBxBkKIzNurddCboUVKDgIG> 0.00000001 ) AKvYpqGhBxBkKIzNurddCboUVKDgIG=1766435724.836006970026192011034124920466f; else AKvYpqGhBxBkKIzNurddCboUVKDgIG=678952666.990562290587238699135507872219f;double bJTJQieSznLBZcmzsdPMDOMFUwNirC=2086842759.025288917405700967475037977977;if (bJTJQieSznLBZcmzsdPMDOMFUwNirC == bJTJQieSznLBZcmzsdPMDOMFUwNirC ) bJTJQieSznLBZcmzsdPMDOMFUwNirC=570177072.917689178485260390282795510841; else bJTJQieSznLBZcmzsdPMDOMFUwNirC=1596203791.204059812180000097769780623317;if (bJTJQieSznLBZcmzsdPMDOMFUwNirC == bJTJQieSznLBZcmzsdPMDOMFUwNirC ) bJTJQieSznLBZcmzsdPMDOMFUwNirC=355871500.033738222268920389553329353453; else bJTJQieSznLBZcmzsdPMDOMFUwNirC=1483081877.650616838397561197135587472448;if (bJTJQieSznLBZcmzsdPMDOMFUwNirC == bJTJQieSznLBZcmzsdPMDOMFUwNirC ) bJTJQieSznLBZcmzsdPMDOMFUwNirC=969209961.588971831565137797927742918425; else bJTJQieSznLBZcmzsdPMDOMFUwNirC=1177406556.449081069046678045089663284616;if (bJTJQieSznLBZcmzsdPMDOMFUwNirC == bJTJQieSznLBZcmzsdPMDOMFUwNirC ) bJTJQieSznLBZcmzsdPMDOMFUwNirC=1273589764.441712725683899218597355699790; else bJTJQieSznLBZcmzsdPMDOMFUwNirC=9221220.856470506643141787408712910713;if (bJTJQieSznLBZcmzsdPMDOMFUwNirC == bJTJQieSznLBZcmzsdPMDOMFUwNirC ) bJTJQieSznLBZcmzsdPMDOMFUwNirC=1342407794.403594842971396431491239314457; else bJTJQieSznLBZcmzsdPMDOMFUwNirC=1619872387.080434166805554752759033227428;if (bJTJQieSznLBZcmzsdPMDOMFUwNirC == bJTJQieSznLBZcmzsdPMDOMFUwNirC ) bJTJQieSznLBZcmzsdPMDOMFUwNirC=2109335076.049609297247670779677281916721; else bJTJQieSznLBZcmzsdPMDOMFUwNirC=3481915.874886606556552246365996197989;long ZlfXjRnvTFpLBKHjQtklwUgetqrjxw=436041997;if (ZlfXjRnvTFpLBKHjQtklwUgetqrjxw == ZlfXjRnvTFpLBKHjQtklwUgetqrjxw- 0 ) ZlfXjRnvTFpLBKHjQtklwUgetqrjxw=1119955818; else ZlfXjRnvTFpLBKHjQtklwUgetqrjxw=1355355513;if (ZlfXjRnvTFpLBKHjQtklwUgetqrjxw == ZlfXjRnvTFpLBKHjQtklwUgetqrjxw- 1 ) ZlfXjRnvTFpLBKHjQtklwUgetqrjxw=716677322; else ZlfXjRnvTFpLBKHjQtklwUgetqrjxw=741976674;if (ZlfXjRnvTFpLBKHjQtklwUgetqrjxw == ZlfXjRnvTFpLBKHjQtklwUgetqrjxw- 0 ) ZlfXjRnvTFpLBKHjQtklwUgetqrjxw=2069694022; else ZlfXjRnvTFpLBKHjQtklwUgetqrjxw=563427540;if (ZlfXjRnvTFpLBKHjQtklwUgetqrjxw == ZlfXjRnvTFpLBKHjQtklwUgetqrjxw- 1 ) ZlfXjRnvTFpLBKHjQtklwUgetqrjxw=844544670; else ZlfXjRnvTFpLBKHjQtklwUgetqrjxw=859980255;if (ZlfXjRnvTFpLBKHjQtklwUgetqrjxw == ZlfXjRnvTFpLBKHjQtklwUgetqrjxw- 0 ) ZlfXjRnvTFpLBKHjQtklwUgetqrjxw=15149321; else ZlfXjRnvTFpLBKHjQtklwUgetqrjxw=1044249882;if (ZlfXjRnvTFpLBKHjQtklwUgetqrjxw == ZlfXjRnvTFpLBKHjQtklwUgetqrjxw- 1 ) ZlfXjRnvTFpLBKHjQtklwUgetqrjxw=280356097; else ZlfXjRnvTFpLBKHjQtklwUgetqrjxw=682802297;float LXqwlwyWIzRLmRIJfGznOVvdaxKbhf=241969653.116623753829919596604588841677f;if (LXqwlwyWIzRLmRIJfGznOVvdaxKbhf - LXqwlwyWIzRLmRIJfGznOVvdaxKbhf> 0.00000001 ) LXqwlwyWIzRLmRIJfGznOVvdaxKbhf=1838857362.569150127336414588271019473889f; else LXqwlwyWIzRLmRIJfGznOVvdaxKbhf=2103477840.218736096948037928436441665079f;if (LXqwlwyWIzRLmRIJfGznOVvdaxKbhf - LXqwlwyWIzRLmRIJfGznOVvdaxKbhf> 0.00000001 ) LXqwlwyWIzRLmRIJfGznOVvdaxKbhf=1474904820.844922888869243418266645398818f; else LXqwlwyWIzRLmRIJfGznOVvdaxKbhf=2006076559.103721412624078821836882420973f;if (LXqwlwyWIzRLmRIJfGznOVvdaxKbhf - LXqwlwyWIzRLmRIJfGznOVvdaxKbhf> 0.00000001 ) LXqwlwyWIzRLmRIJfGznOVvdaxKbhf=1596116288.517176762200775268868708343240f; else LXqwlwyWIzRLmRIJfGznOVvdaxKbhf=1943920172.255921800317064622726222681984f;if (LXqwlwyWIzRLmRIJfGznOVvdaxKbhf - LXqwlwyWIzRLmRIJfGznOVvdaxKbhf> 0.00000001 ) LXqwlwyWIzRLmRIJfGznOVvdaxKbhf=2001841659.462215685457955285573152403445f; else LXqwlwyWIzRLmRIJfGznOVvdaxKbhf=1970306074.192706586914848048740651131181f;if (LXqwlwyWIzRLmRIJfGznOVvdaxKbhf - LXqwlwyWIzRLmRIJfGznOVvdaxKbhf> 0.00000001 ) LXqwlwyWIzRLmRIJfGznOVvdaxKbhf=226774954.224137526981977941457671554267f; else LXqwlwyWIzRLmRIJfGznOVvdaxKbhf=1767512223.640679181252297088420074476469f;if (LXqwlwyWIzRLmRIJfGznOVvdaxKbhf - LXqwlwyWIzRLmRIJfGznOVvdaxKbhf> 0.00000001 ) LXqwlwyWIzRLmRIJfGznOVvdaxKbhf=894357433.346044141996130558468784768720f; else LXqwlwyWIzRLmRIJfGznOVvdaxKbhf=805482141.482302636404864405403801167338f;int TOBaDUpRqsILDdXtHAMgPOIncpHJQa=1134227674;if (TOBaDUpRqsILDdXtHAMgPOIncpHJQa == TOBaDUpRqsILDdXtHAMgPOIncpHJQa- 0 ) TOBaDUpRqsILDdXtHAMgPOIncpHJQa=1377850139; else TOBaDUpRqsILDdXtHAMgPOIncpHJQa=1330393727;if (TOBaDUpRqsILDdXtHAMgPOIncpHJQa == TOBaDUpRqsILDdXtHAMgPOIncpHJQa- 0 ) TOBaDUpRqsILDdXtHAMgPOIncpHJQa=1706480495; else TOBaDUpRqsILDdXtHAMgPOIncpHJQa=2089605940;if (TOBaDUpRqsILDdXtHAMgPOIncpHJQa == TOBaDUpRqsILDdXtHAMgPOIncpHJQa- 1 ) TOBaDUpRqsILDdXtHAMgPOIncpHJQa=92046793; else TOBaDUpRqsILDdXtHAMgPOIncpHJQa=908945344;if (TOBaDUpRqsILDdXtHAMgPOIncpHJQa == TOBaDUpRqsILDdXtHAMgPOIncpHJQa- 1 ) TOBaDUpRqsILDdXtHAMgPOIncpHJQa=17148737; else TOBaDUpRqsILDdXtHAMgPOIncpHJQa=6749532;if (TOBaDUpRqsILDdXtHAMgPOIncpHJQa == TOBaDUpRqsILDdXtHAMgPOIncpHJQa- 0 ) TOBaDUpRqsILDdXtHAMgPOIncpHJQa=751659121; else TOBaDUpRqsILDdXtHAMgPOIncpHJQa=1643374892;if (TOBaDUpRqsILDdXtHAMgPOIncpHJQa == TOBaDUpRqsILDdXtHAMgPOIncpHJQa- 0 ) TOBaDUpRqsILDdXtHAMgPOIncpHJQa=1467932556; else TOBaDUpRqsILDdXtHAMgPOIncpHJQa=2072443726;float NHFkvmPQkaoeOEGeqyJaXZcdzrGNXo=2027410973.170831016690444448769037457819f;if (NHFkvmPQkaoeOEGeqyJaXZcdzrGNXo - NHFkvmPQkaoeOEGeqyJaXZcdzrGNXo> 0.00000001 ) NHFkvmPQkaoeOEGeqyJaXZcdzrGNXo=1016392440.101403254621695255097256915358f; else NHFkvmPQkaoeOEGeqyJaXZcdzrGNXo=1867365423.530850904220425954220393305669f;if (NHFkvmPQkaoeOEGeqyJaXZcdzrGNXo - NHFkvmPQkaoeOEGeqyJaXZcdzrGNXo> 0.00000001 ) NHFkvmPQkaoeOEGeqyJaXZcdzrGNXo=312108200.901610194645187884107114935856f; else NHFkvmPQkaoeOEGeqyJaXZcdzrGNXo=1800508767.635633981586179531653256389889f;if (NHFkvmPQkaoeOEGeqyJaXZcdzrGNXo - NHFkvmPQkaoeOEGeqyJaXZcdzrGNXo> 0.00000001 ) NHFkvmPQkaoeOEGeqyJaXZcdzrGNXo=338341083.644901166500373115540421359035f; else NHFkvmPQkaoeOEGeqyJaXZcdzrGNXo=2125350862.521204327858626399328434923466f;if (NHFkvmPQkaoeOEGeqyJaXZcdzrGNXo - NHFkvmPQkaoeOEGeqyJaXZcdzrGNXo> 0.00000001 ) NHFkvmPQkaoeOEGeqyJaXZcdzrGNXo=987409356.323109018099880390616071819738f; else NHFkvmPQkaoeOEGeqyJaXZcdzrGNXo=1323519293.106263552848244076563386035240f;if (NHFkvmPQkaoeOEGeqyJaXZcdzrGNXo - NHFkvmPQkaoeOEGeqyJaXZcdzrGNXo> 0.00000001 ) NHFkvmPQkaoeOEGeqyJaXZcdzrGNXo=1500997501.039099372894887984884438582123f; else NHFkvmPQkaoeOEGeqyJaXZcdzrGNXo=893854895.842381035671478961925256671151f;if (NHFkvmPQkaoeOEGeqyJaXZcdzrGNXo - NHFkvmPQkaoeOEGeqyJaXZcdzrGNXo> 0.00000001 ) NHFkvmPQkaoeOEGeqyJaXZcdzrGNXo=515201720.835006260796032406063220879782f; else NHFkvmPQkaoeOEGeqyJaXZcdzrGNXo=302181799.098485658371133033427529972624f;float aprjArcWjqbqmuLDtjrSjPbYYmubJR=333571264.438589943149942274804598503497f;if (aprjArcWjqbqmuLDtjrSjPbYYmubJR - aprjArcWjqbqmuLDtjrSjPbYYmubJR> 0.00000001 ) aprjArcWjqbqmuLDtjrSjPbYYmubJR=959363811.631138262160000762063073404008f; else aprjArcWjqbqmuLDtjrSjPbYYmubJR=1293612070.765651882412385559995244577973f;if (aprjArcWjqbqmuLDtjrSjPbYYmubJR - aprjArcWjqbqmuLDtjrSjPbYYmubJR> 0.00000001 ) aprjArcWjqbqmuLDtjrSjPbYYmubJR=1452435805.426441646908075270906512452128f; else aprjArcWjqbqmuLDtjrSjPbYYmubJR=110213184.592909694591419728041817891510f;if (aprjArcWjqbqmuLDtjrSjPbYYmubJR - aprjArcWjqbqmuLDtjrSjPbYYmubJR> 0.00000001 ) aprjArcWjqbqmuLDtjrSjPbYYmubJR=1903483620.953851727769651521374529889511f; else aprjArcWjqbqmuLDtjrSjPbYYmubJR=661357654.829565966900238227638124654517f;if (aprjArcWjqbqmuLDtjrSjPbYYmubJR - aprjArcWjqbqmuLDtjrSjPbYYmubJR> 0.00000001 ) aprjArcWjqbqmuLDtjrSjPbYYmubJR=149922860.231695029776557439166398480592f; else aprjArcWjqbqmuLDtjrSjPbYYmubJR=929912018.256078075431807368832545204322f;if (aprjArcWjqbqmuLDtjrSjPbYYmubJR - aprjArcWjqbqmuLDtjrSjPbYYmubJR> 0.00000001 ) aprjArcWjqbqmuLDtjrSjPbYYmubJR=2086329103.663763069957584650223387071288f; else aprjArcWjqbqmuLDtjrSjPbYYmubJR=1468155288.575397814530999606265421039789f;if (aprjArcWjqbqmuLDtjrSjPbYYmubJR - aprjArcWjqbqmuLDtjrSjPbYYmubJR> 0.00000001 ) aprjArcWjqbqmuLDtjrSjPbYYmubJR=1254417438.298258220785779349957613315446f; else aprjArcWjqbqmuLDtjrSjPbYYmubJR=2100225043.924544284148759185270093279694f;long qqVqfBFRaEbYoGvsdVPQzLlcuRGQlb=1783471134;if (qqVqfBFRaEbYoGvsdVPQzLlcuRGQlb == qqVqfBFRaEbYoGvsdVPQzLlcuRGQlb- 1 ) qqVqfBFRaEbYoGvsdVPQzLlcuRGQlb=1051414267; else qqVqfBFRaEbYoGvsdVPQzLlcuRGQlb=1014430107;if (qqVqfBFRaEbYoGvsdVPQzLlcuRGQlb == qqVqfBFRaEbYoGvsdVPQzLlcuRGQlb- 1 ) qqVqfBFRaEbYoGvsdVPQzLlcuRGQlb=1936573348; else qqVqfBFRaEbYoGvsdVPQzLlcuRGQlb=201033680;if (qqVqfBFRaEbYoGvsdVPQzLlcuRGQlb == qqVqfBFRaEbYoGvsdVPQzLlcuRGQlb- 1 ) qqVqfBFRaEbYoGvsdVPQzLlcuRGQlb=1516953504; else qqVqfBFRaEbYoGvsdVPQzLlcuRGQlb=431860951;if (qqVqfBFRaEbYoGvsdVPQzLlcuRGQlb == qqVqfBFRaEbYoGvsdVPQzLlcuRGQlb- 0 ) qqVqfBFRaEbYoGvsdVPQzLlcuRGQlb=795886591; else qqVqfBFRaEbYoGvsdVPQzLlcuRGQlb=1399982924;if (qqVqfBFRaEbYoGvsdVPQzLlcuRGQlb == qqVqfBFRaEbYoGvsdVPQzLlcuRGQlb- 1 ) qqVqfBFRaEbYoGvsdVPQzLlcuRGQlb=342984371; else qqVqfBFRaEbYoGvsdVPQzLlcuRGQlb=382961202;if (qqVqfBFRaEbYoGvsdVPQzLlcuRGQlb == qqVqfBFRaEbYoGvsdVPQzLlcuRGQlb- 1 ) qqVqfBFRaEbYoGvsdVPQzLlcuRGQlb=588608439; else qqVqfBFRaEbYoGvsdVPQzLlcuRGQlb=1345675545;long uBBQlIDHhvePBvLtOtvsLCHNZpUutc=712596592;if (uBBQlIDHhvePBvLtOtvsLCHNZpUutc == uBBQlIDHhvePBvLtOtvsLCHNZpUutc- 1 ) uBBQlIDHhvePBvLtOtvsLCHNZpUutc=704310093; else uBBQlIDHhvePBvLtOtvsLCHNZpUutc=1134361292;if (uBBQlIDHhvePBvLtOtvsLCHNZpUutc == uBBQlIDHhvePBvLtOtvsLCHNZpUutc- 0 ) uBBQlIDHhvePBvLtOtvsLCHNZpUutc=1113079915; else uBBQlIDHhvePBvLtOtvsLCHNZpUutc=1750922036;if (uBBQlIDHhvePBvLtOtvsLCHNZpUutc == uBBQlIDHhvePBvLtOtvsLCHNZpUutc- 1 ) uBBQlIDHhvePBvLtOtvsLCHNZpUutc=1285835309; else uBBQlIDHhvePBvLtOtvsLCHNZpUutc=605884772;if (uBBQlIDHhvePBvLtOtvsLCHNZpUutc == uBBQlIDHhvePBvLtOtvsLCHNZpUutc- 0 ) uBBQlIDHhvePBvLtOtvsLCHNZpUutc=123927353; else uBBQlIDHhvePBvLtOtvsLCHNZpUutc=355034786;if (uBBQlIDHhvePBvLtOtvsLCHNZpUutc == uBBQlIDHhvePBvLtOtvsLCHNZpUutc- 1 ) uBBQlIDHhvePBvLtOtvsLCHNZpUutc=1717442563; else uBBQlIDHhvePBvLtOtvsLCHNZpUutc=1529679829;if (uBBQlIDHhvePBvLtOtvsLCHNZpUutc == uBBQlIDHhvePBvLtOtvsLCHNZpUutc- 0 ) uBBQlIDHhvePBvLtOtvsLCHNZpUutc=1861663311; else uBBQlIDHhvePBvLtOtvsLCHNZpUutc=1017669442;float lFiaNJHOMcbbUsCTSybSoQGGyrFPZL=1571599568.677678643684135094993877847765f;if (lFiaNJHOMcbbUsCTSybSoQGGyrFPZL - lFiaNJHOMcbbUsCTSybSoQGGyrFPZL> 0.00000001 ) lFiaNJHOMcbbUsCTSybSoQGGyrFPZL=1968082927.802072350841557111332506580094f; else lFiaNJHOMcbbUsCTSybSoQGGyrFPZL=879214233.109989226474416342073743330750f;if (lFiaNJHOMcbbUsCTSybSoQGGyrFPZL - lFiaNJHOMcbbUsCTSybSoQGGyrFPZL> 0.00000001 ) lFiaNJHOMcbbUsCTSybSoQGGyrFPZL=1398251179.236785268990506819135912156665f; else lFiaNJHOMcbbUsCTSybSoQGGyrFPZL=802123647.968725884118528424863263543564f;if (lFiaNJHOMcbbUsCTSybSoQGGyrFPZL - lFiaNJHOMcbbUsCTSybSoQGGyrFPZL> 0.00000001 ) lFiaNJHOMcbbUsCTSybSoQGGyrFPZL=249972828.970304299203502849583291381629f; else lFiaNJHOMcbbUsCTSybSoQGGyrFPZL=738031119.243135833524666076180409178902f;if (lFiaNJHOMcbbUsCTSybSoQGGyrFPZL - lFiaNJHOMcbbUsCTSybSoQGGyrFPZL> 0.00000001 ) lFiaNJHOMcbbUsCTSybSoQGGyrFPZL=964101407.477106147802606073641555985517f; else lFiaNJHOMcbbUsCTSybSoQGGyrFPZL=527502860.793215867093939891320547103322f;if (lFiaNJHOMcbbUsCTSybSoQGGyrFPZL - lFiaNJHOMcbbUsCTSybSoQGGyrFPZL> 0.00000001 ) lFiaNJHOMcbbUsCTSybSoQGGyrFPZL=497725479.274111059131765962720420726612f; else lFiaNJHOMcbbUsCTSybSoQGGyrFPZL=52452881.355593092884229099234170285550f;if (lFiaNJHOMcbbUsCTSybSoQGGyrFPZL - lFiaNJHOMcbbUsCTSybSoQGGyrFPZL> 0.00000001 ) lFiaNJHOMcbbUsCTSybSoQGGyrFPZL=1914712460.655898738297094730184748839531f; else lFiaNJHOMcbbUsCTSybSoQGGyrFPZL=1520522418.955324954247214718666547601707f;float GwhSUOllAprhtUlcSTFKHvndTEllMR=223878518.087492827590206575380326363868f;if (GwhSUOllAprhtUlcSTFKHvndTEllMR - GwhSUOllAprhtUlcSTFKHvndTEllMR> 0.00000001 ) GwhSUOllAprhtUlcSTFKHvndTEllMR=1130475863.474206380741849137122038987113f; else GwhSUOllAprhtUlcSTFKHvndTEllMR=541820846.006922700130564058824685644733f;if (GwhSUOllAprhtUlcSTFKHvndTEllMR - GwhSUOllAprhtUlcSTFKHvndTEllMR> 0.00000001 ) GwhSUOllAprhtUlcSTFKHvndTEllMR=1395914950.937885602853810257404940043238f; else GwhSUOllAprhtUlcSTFKHvndTEllMR=1489109971.818820489046707197650833000794f;if (GwhSUOllAprhtUlcSTFKHvndTEllMR - GwhSUOllAprhtUlcSTFKHvndTEllMR> 0.00000001 ) GwhSUOllAprhtUlcSTFKHvndTEllMR=963801665.257405609088946834396679918593f; else GwhSUOllAprhtUlcSTFKHvndTEllMR=1469817226.180590930369967290828640969850f;if (GwhSUOllAprhtUlcSTFKHvndTEllMR - GwhSUOllAprhtUlcSTFKHvndTEllMR> 0.00000001 ) GwhSUOllAprhtUlcSTFKHvndTEllMR=497635825.712701956700414781383887554063f; else GwhSUOllAprhtUlcSTFKHvndTEllMR=445529495.229165727634513824729868894525f;if (GwhSUOllAprhtUlcSTFKHvndTEllMR - GwhSUOllAprhtUlcSTFKHvndTEllMR> 0.00000001 ) GwhSUOllAprhtUlcSTFKHvndTEllMR=890502754.800358245016575947583044456138f; else GwhSUOllAprhtUlcSTFKHvndTEllMR=1581538562.120294527578072349306815286691f;if (GwhSUOllAprhtUlcSTFKHvndTEllMR - GwhSUOllAprhtUlcSTFKHvndTEllMR> 0.00000001 ) GwhSUOllAprhtUlcSTFKHvndTEllMR=631074764.312899639142111517221794242611f; else GwhSUOllAprhtUlcSTFKHvndTEllMR=2134757322.904626739471136480577948063865f;double ELpLWsWrYqrpqWzhgoqiHESUxdcfgT=1455800058.495471399128920221092341399809;if (ELpLWsWrYqrpqWzhgoqiHESUxdcfgT == ELpLWsWrYqrpqWzhgoqiHESUxdcfgT ) ELpLWsWrYqrpqWzhgoqiHESUxdcfgT=1682913172.910408321427868395881436015419; else ELpLWsWrYqrpqWzhgoqiHESUxdcfgT=1164492518.250132862300067658300503858559;if (ELpLWsWrYqrpqWzhgoqiHESUxdcfgT == ELpLWsWrYqrpqWzhgoqiHESUxdcfgT ) ELpLWsWrYqrpqWzhgoqiHESUxdcfgT=1525076265.391338783764358248946712985838; else ELpLWsWrYqrpqWzhgoqiHESUxdcfgT=1662013038.542412084098575726572071368055;if (ELpLWsWrYqrpqWzhgoqiHESUxdcfgT == ELpLWsWrYqrpqWzhgoqiHESUxdcfgT ) ELpLWsWrYqrpqWzhgoqiHESUxdcfgT=464199406.405633305726596835826046757115; else ELpLWsWrYqrpqWzhgoqiHESUxdcfgT=1683039425.403979405252185668654434047908;if (ELpLWsWrYqrpqWzhgoqiHESUxdcfgT == ELpLWsWrYqrpqWzhgoqiHESUxdcfgT ) ELpLWsWrYqrpqWzhgoqiHESUxdcfgT=462623764.620043473753067596210056218003; else ELpLWsWrYqrpqWzhgoqiHESUxdcfgT=2113762621.361774315339501147865180977593;if (ELpLWsWrYqrpqWzhgoqiHESUxdcfgT == ELpLWsWrYqrpqWzhgoqiHESUxdcfgT ) ELpLWsWrYqrpqWzhgoqiHESUxdcfgT=170259885.558140330329298156517135828291; else ELpLWsWrYqrpqWzhgoqiHESUxdcfgT=585577055.793289256292854709292108652433;if (ELpLWsWrYqrpqWzhgoqiHESUxdcfgT == ELpLWsWrYqrpqWzhgoqiHESUxdcfgT ) ELpLWsWrYqrpqWzhgoqiHESUxdcfgT=1253196557.225074601091143638122001069564; else ELpLWsWrYqrpqWzhgoqiHESUxdcfgT=1233382428.793407799111830117512532132401;int FRnNBqkSdtVyDBHFmFrcNzdINfFQmd=765711601;if (FRnNBqkSdtVyDBHFmFrcNzdINfFQmd == FRnNBqkSdtVyDBHFmFrcNzdINfFQmd- 0 ) FRnNBqkSdtVyDBHFmFrcNzdINfFQmd=993511435; else FRnNBqkSdtVyDBHFmFrcNzdINfFQmd=1637784793;if (FRnNBqkSdtVyDBHFmFrcNzdINfFQmd == FRnNBqkSdtVyDBHFmFrcNzdINfFQmd- 1 ) FRnNBqkSdtVyDBHFmFrcNzdINfFQmd=2034677226; else FRnNBqkSdtVyDBHFmFrcNzdINfFQmd=329112578;if (FRnNBqkSdtVyDBHFmFrcNzdINfFQmd == FRnNBqkSdtVyDBHFmFrcNzdINfFQmd- 1 ) FRnNBqkSdtVyDBHFmFrcNzdINfFQmd=1786236657; else FRnNBqkSdtVyDBHFmFrcNzdINfFQmd=1505905353;if (FRnNBqkSdtVyDBHFmFrcNzdINfFQmd == FRnNBqkSdtVyDBHFmFrcNzdINfFQmd- 1 ) FRnNBqkSdtVyDBHFmFrcNzdINfFQmd=607797903; else FRnNBqkSdtVyDBHFmFrcNzdINfFQmd=498265701;if (FRnNBqkSdtVyDBHFmFrcNzdINfFQmd == FRnNBqkSdtVyDBHFmFrcNzdINfFQmd- 0 ) FRnNBqkSdtVyDBHFmFrcNzdINfFQmd=381578408; else FRnNBqkSdtVyDBHFmFrcNzdINfFQmd=952721684;if (FRnNBqkSdtVyDBHFmFrcNzdINfFQmd == FRnNBqkSdtVyDBHFmFrcNzdINfFQmd- 0 ) FRnNBqkSdtVyDBHFmFrcNzdINfFQmd=2059104540; else FRnNBqkSdtVyDBHFmFrcNzdINfFQmd=815917913;float CNLJADEpbieSbCAvAMrDopHUxmwUfv=719508330.998444528078365065385045976808f;if (CNLJADEpbieSbCAvAMrDopHUxmwUfv - CNLJADEpbieSbCAvAMrDopHUxmwUfv> 0.00000001 ) CNLJADEpbieSbCAvAMrDopHUxmwUfv=1670887028.713740088128755262598611396457f; else CNLJADEpbieSbCAvAMrDopHUxmwUfv=458912402.873046673509737749082542111245f;if (CNLJADEpbieSbCAvAMrDopHUxmwUfv - CNLJADEpbieSbCAvAMrDopHUxmwUfv> 0.00000001 ) CNLJADEpbieSbCAvAMrDopHUxmwUfv=1985092893.563135672960009098466747643103f; else CNLJADEpbieSbCAvAMrDopHUxmwUfv=1055740344.643814670327822355312388983759f;if (CNLJADEpbieSbCAvAMrDopHUxmwUfv - CNLJADEpbieSbCAvAMrDopHUxmwUfv> 0.00000001 ) CNLJADEpbieSbCAvAMrDopHUxmwUfv=1574138344.258943729553825093549999236593f; else CNLJADEpbieSbCAvAMrDopHUxmwUfv=1021639003.292117908064686957400022147971f;if (CNLJADEpbieSbCAvAMrDopHUxmwUfv - CNLJADEpbieSbCAvAMrDopHUxmwUfv> 0.00000001 ) CNLJADEpbieSbCAvAMrDopHUxmwUfv=1907162759.733747182919569631438020438118f; else CNLJADEpbieSbCAvAMrDopHUxmwUfv=1594920085.014733105088650157451765482431f;if (CNLJADEpbieSbCAvAMrDopHUxmwUfv - CNLJADEpbieSbCAvAMrDopHUxmwUfv> 0.00000001 ) CNLJADEpbieSbCAvAMrDopHUxmwUfv=79197082.917400693654745057038177746046f; else CNLJADEpbieSbCAvAMrDopHUxmwUfv=1429635976.287417226067480812044540485187f;if (CNLJADEpbieSbCAvAMrDopHUxmwUfv - CNLJADEpbieSbCAvAMrDopHUxmwUfv> 0.00000001 ) CNLJADEpbieSbCAvAMrDopHUxmwUfv=1318850086.575748733893218270704400384713f; else CNLJADEpbieSbCAvAMrDopHUxmwUfv=378224610.325959284318925134321658327557f;long xJAantPbtwzbSudarZIZAXtKokigRL=139614172;if (xJAantPbtwzbSudarZIZAXtKokigRL == xJAantPbtwzbSudarZIZAXtKokigRL- 0 ) xJAantPbtwzbSudarZIZAXtKokigRL=1299252991; else xJAantPbtwzbSudarZIZAXtKokigRL=2012846810;if (xJAantPbtwzbSudarZIZAXtKokigRL == xJAantPbtwzbSudarZIZAXtKokigRL- 0 ) xJAantPbtwzbSudarZIZAXtKokigRL=1141245887; else xJAantPbtwzbSudarZIZAXtKokigRL=1227380141;if (xJAantPbtwzbSudarZIZAXtKokigRL == xJAantPbtwzbSudarZIZAXtKokigRL- 0 ) xJAantPbtwzbSudarZIZAXtKokigRL=2038507217; else xJAantPbtwzbSudarZIZAXtKokigRL=199936922;if (xJAantPbtwzbSudarZIZAXtKokigRL == xJAantPbtwzbSudarZIZAXtKokigRL- 0 ) xJAantPbtwzbSudarZIZAXtKokigRL=1817047048; else xJAantPbtwzbSudarZIZAXtKokigRL=177007819;if (xJAantPbtwzbSudarZIZAXtKokigRL == xJAantPbtwzbSudarZIZAXtKokigRL- 1 ) xJAantPbtwzbSudarZIZAXtKokigRL=556694615; else xJAantPbtwzbSudarZIZAXtKokigRL=1026810564;if (xJAantPbtwzbSudarZIZAXtKokigRL == xJAantPbtwzbSudarZIZAXtKokigRL- 1 ) xJAantPbtwzbSudarZIZAXtKokigRL=1280434630; else xJAantPbtwzbSudarZIZAXtKokigRL=1658961369;float ZXiuuuZBmLWPvtHANucNKIgcCsTPxW=1340915800.285546611255603521417179846334f;if (ZXiuuuZBmLWPvtHANucNKIgcCsTPxW - ZXiuuuZBmLWPvtHANucNKIgcCsTPxW> 0.00000001 ) ZXiuuuZBmLWPvtHANucNKIgcCsTPxW=775755482.287310563068290965001998111771f; else ZXiuuuZBmLWPvtHANucNKIgcCsTPxW=533688227.139807839306856909998133430021f;if (ZXiuuuZBmLWPvtHANucNKIgcCsTPxW - ZXiuuuZBmLWPvtHANucNKIgcCsTPxW> 0.00000001 ) ZXiuuuZBmLWPvtHANucNKIgcCsTPxW=1709979047.999393059324670525232389301658f; else ZXiuuuZBmLWPvtHANucNKIgcCsTPxW=379743557.891952866405178567492680071110f;if (ZXiuuuZBmLWPvtHANucNKIgcCsTPxW - ZXiuuuZBmLWPvtHANucNKIgcCsTPxW> 0.00000001 ) ZXiuuuZBmLWPvtHANucNKIgcCsTPxW=913279336.341254373987983721194436631108f; else ZXiuuuZBmLWPvtHANucNKIgcCsTPxW=1374028874.175727592529552671835012692663f;if (ZXiuuuZBmLWPvtHANucNKIgcCsTPxW - ZXiuuuZBmLWPvtHANucNKIgcCsTPxW> 0.00000001 ) ZXiuuuZBmLWPvtHANucNKIgcCsTPxW=1339056904.438582237723077508416147417424f; else ZXiuuuZBmLWPvtHANucNKIgcCsTPxW=2119356079.504020208155421214519426749127f;if (ZXiuuuZBmLWPvtHANucNKIgcCsTPxW - ZXiuuuZBmLWPvtHANucNKIgcCsTPxW> 0.00000001 ) ZXiuuuZBmLWPvtHANucNKIgcCsTPxW=1878105681.681423016841047273621392519839f; else ZXiuuuZBmLWPvtHANucNKIgcCsTPxW=439757141.118167410921987958067761730091f;if (ZXiuuuZBmLWPvtHANucNKIgcCsTPxW - ZXiuuuZBmLWPvtHANucNKIgcCsTPxW> 0.00000001 ) ZXiuuuZBmLWPvtHANucNKIgcCsTPxW=249915496.438873309086370042567187256853f; else ZXiuuuZBmLWPvtHANucNKIgcCsTPxW=356600681.752950321475009249170964445163f;float YXdQPRUJClKDCwJGpsxGikpRsvtaJV=1117325011.873818635173428335831848431071f;if (YXdQPRUJClKDCwJGpsxGikpRsvtaJV - YXdQPRUJClKDCwJGpsxGikpRsvtaJV> 0.00000001 ) YXdQPRUJClKDCwJGpsxGikpRsvtaJV=1345848148.346282357850824236259153113648f; else YXdQPRUJClKDCwJGpsxGikpRsvtaJV=1919490368.777493824708919113531592220328f;if (YXdQPRUJClKDCwJGpsxGikpRsvtaJV - YXdQPRUJClKDCwJGpsxGikpRsvtaJV> 0.00000001 ) YXdQPRUJClKDCwJGpsxGikpRsvtaJV=1664148569.982918444141255234501012729527f; else YXdQPRUJClKDCwJGpsxGikpRsvtaJV=241593192.450783700287059044810693808039f;if (YXdQPRUJClKDCwJGpsxGikpRsvtaJV - YXdQPRUJClKDCwJGpsxGikpRsvtaJV> 0.00000001 ) YXdQPRUJClKDCwJGpsxGikpRsvtaJV=1983065492.002779258363542406379672797808f; else YXdQPRUJClKDCwJGpsxGikpRsvtaJV=729925079.578238639099656825140276143119f;if (YXdQPRUJClKDCwJGpsxGikpRsvtaJV - YXdQPRUJClKDCwJGpsxGikpRsvtaJV> 0.00000001 ) YXdQPRUJClKDCwJGpsxGikpRsvtaJV=828484760.356099532497849103697376830229f; else YXdQPRUJClKDCwJGpsxGikpRsvtaJV=1470950106.653085367739486354805162208126f;if (YXdQPRUJClKDCwJGpsxGikpRsvtaJV - YXdQPRUJClKDCwJGpsxGikpRsvtaJV> 0.00000001 ) YXdQPRUJClKDCwJGpsxGikpRsvtaJV=789349001.042494586484797999069652450188f; else YXdQPRUJClKDCwJGpsxGikpRsvtaJV=1808085074.734180719297842157642017849104f;if (YXdQPRUJClKDCwJGpsxGikpRsvtaJV - YXdQPRUJClKDCwJGpsxGikpRsvtaJV> 0.00000001 ) YXdQPRUJClKDCwJGpsxGikpRsvtaJV=499045729.540986062839187282041465782845f; else YXdQPRUJClKDCwJGpsxGikpRsvtaJV=547327780.161685707710692393166816329620f;float OHkfgpZBLbqYTOIonSVsukstRaaNgh=432491231.520348701215554696680805231524f;if (OHkfgpZBLbqYTOIonSVsukstRaaNgh - OHkfgpZBLbqYTOIonSVsukstRaaNgh> 0.00000001 ) OHkfgpZBLbqYTOIonSVsukstRaaNgh=1570313357.197098862920684386251063883622f; else OHkfgpZBLbqYTOIonSVsukstRaaNgh=2125417933.728712957783587022105647414521f;if (OHkfgpZBLbqYTOIonSVsukstRaaNgh - OHkfgpZBLbqYTOIonSVsukstRaaNgh> 0.00000001 ) OHkfgpZBLbqYTOIonSVsukstRaaNgh=1749952775.535021207253068869696152828240f; else OHkfgpZBLbqYTOIonSVsukstRaaNgh=1830416089.954626956793506368076594659331f;if (OHkfgpZBLbqYTOIonSVsukstRaaNgh - OHkfgpZBLbqYTOIonSVsukstRaaNgh> 0.00000001 ) OHkfgpZBLbqYTOIonSVsukstRaaNgh=1849241644.307731045201300466766037546017f; else OHkfgpZBLbqYTOIonSVsukstRaaNgh=1374613626.863649787633488629429468538030f;if (OHkfgpZBLbqYTOIonSVsukstRaaNgh - OHkfgpZBLbqYTOIonSVsukstRaaNgh> 0.00000001 ) OHkfgpZBLbqYTOIonSVsukstRaaNgh=1373818483.514854886288023476093869951622f; else OHkfgpZBLbqYTOIonSVsukstRaaNgh=2072707764.186813073485325154517304785501f;if (OHkfgpZBLbqYTOIonSVsukstRaaNgh - OHkfgpZBLbqYTOIonSVsukstRaaNgh> 0.00000001 ) OHkfgpZBLbqYTOIonSVsukstRaaNgh=673789906.336520322150362854481711598214f; else OHkfgpZBLbqYTOIonSVsukstRaaNgh=1169373455.857225091593357714303113173604f;if (OHkfgpZBLbqYTOIonSVsukstRaaNgh - OHkfgpZBLbqYTOIonSVsukstRaaNgh> 0.00000001 ) OHkfgpZBLbqYTOIonSVsukstRaaNgh=1496758107.222830415787399846598591174525f; else OHkfgpZBLbqYTOIonSVsukstRaaNgh=1598750076.359848163740972792434812683276f;double uvCSTqceHlpPpbrbIrXuKBsFcRkLbV=327121322.114672812653096957296646485530;if (uvCSTqceHlpPpbrbIrXuKBsFcRkLbV == uvCSTqceHlpPpbrbIrXuKBsFcRkLbV ) uvCSTqceHlpPpbrbIrXuKBsFcRkLbV=745165532.339805459145211075821508957920; else uvCSTqceHlpPpbrbIrXuKBsFcRkLbV=163109619.739342539250916254642181983195;if (uvCSTqceHlpPpbrbIrXuKBsFcRkLbV == uvCSTqceHlpPpbrbIrXuKBsFcRkLbV ) uvCSTqceHlpPpbrbIrXuKBsFcRkLbV=313113221.864051698964667260281338876846; else uvCSTqceHlpPpbrbIrXuKBsFcRkLbV=1999411811.045449607666779862473401454729;if (uvCSTqceHlpPpbrbIrXuKBsFcRkLbV == uvCSTqceHlpPpbrbIrXuKBsFcRkLbV ) uvCSTqceHlpPpbrbIrXuKBsFcRkLbV=130040929.685203022967373676011448465400; else uvCSTqceHlpPpbrbIrXuKBsFcRkLbV=1765213074.608908168794062613999872824894;if (uvCSTqceHlpPpbrbIrXuKBsFcRkLbV == uvCSTqceHlpPpbrbIrXuKBsFcRkLbV ) uvCSTqceHlpPpbrbIrXuKBsFcRkLbV=1505333955.862336948562832849685262283728; else uvCSTqceHlpPpbrbIrXuKBsFcRkLbV=45830403.437593284678644805009621971125;if (uvCSTqceHlpPpbrbIrXuKBsFcRkLbV == uvCSTqceHlpPpbrbIrXuKBsFcRkLbV ) uvCSTqceHlpPpbrbIrXuKBsFcRkLbV=1852687114.333023545967512872959972081663; else uvCSTqceHlpPpbrbIrXuKBsFcRkLbV=239028941.327034573136014222021027747114;if (uvCSTqceHlpPpbrbIrXuKBsFcRkLbV == uvCSTqceHlpPpbrbIrXuKBsFcRkLbV ) uvCSTqceHlpPpbrbIrXuKBsFcRkLbV=1737878203.209375168831983306897065330039; else uvCSTqceHlpPpbrbIrXuKBsFcRkLbV=1252677909.756003885016439015555232834415;int FOuDsVvtvWJFtlAeQgLJZZtUaGVIkw=1629904291;if (FOuDsVvtvWJFtlAeQgLJZZtUaGVIkw == FOuDsVvtvWJFtlAeQgLJZZtUaGVIkw- 0 ) FOuDsVvtvWJFtlAeQgLJZZtUaGVIkw=616255588; else FOuDsVvtvWJFtlAeQgLJZZtUaGVIkw=1964907912;if (FOuDsVvtvWJFtlAeQgLJZZtUaGVIkw == FOuDsVvtvWJFtlAeQgLJZZtUaGVIkw- 1 ) FOuDsVvtvWJFtlAeQgLJZZtUaGVIkw=933770971; else FOuDsVvtvWJFtlAeQgLJZZtUaGVIkw=209120981;if (FOuDsVvtvWJFtlAeQgLJZZtUaGVIkw == FOuDsVvtvWJFtlAeQgLJZZtUaGVIkw- 1 ) FOuDsVvtvWJFtlAeQgLJZZtUaGVIkw=627104165; else FOuDsVvtvWJFtlAeQgLJZZtUaGVIkw=598712590;if (FOuDsVvtvWJFtlAeQgLJZZtUaGVIkw == FOuDsVvtvWJFtlAeQgLJZZtUaGVIkw- 1 ) FOuDsVvtvWJFtlAeQgLJZZtUaGVIkw=1415567014; else FOuDsVvtvWJFtlAeQgLJZZtUaGVIkw=2118718169;if (FOuDsVvtvWJFtlAeQgLJZZtUaGVIkw == FOuDsVvtvWJFtlAeQgLJZZtUaGVIkw- 1 ) FOuDsVvtvWJFtlAeQgLJZZtUaGVIkw=545671885; else FOuDsVvtvWJFtlAeQgLJZZtUaGVIkw=1738924452;if (FOuDsVvtvWJFtlAeQgLJZZtUaGVIkw == FOuDsVvtvWJFtlAeQgLJZZtUaGVIkw- 1 ) FOuDsVvtvWJFtlAeQgLJZZtUaGVIkw=1715286933; else FOuDsVvtvWJFtlAeQgLJZZtUaGVIkw=813692037;float uYxwkErStavifxkYThKRZIFpIwyIen=1476386281.968172074121022058895114869002f;if (uYxwkErStavifxkYThKRZIFpIwyIen - uYxwkErStavifxkYThKRZIFpIwyIen> 0.00000001 ) uYxwkErStavifxkYThKRZIFpIwyIen=1818132522.086458457622708666857827896442f; else uYxwkErStavifxkYThKRZIFpIwyIen=171924407.418325677471227442620326637977f;if (uYxwkErStavifxkYThKRZIFpIwyIen - uYxwkErStavifxkYThKRZIFpIwyIen> 0.00000001 ) uYxwkErStavifxkYThKRZIFpIwyIen=1949645895.301632548400049660055482219280f; else uYxwkErStavifxkYThKRZIFpIwyIen=1725578401.871127070600433049807702202043f;if (uYxwkErStavifxkYThKRZIFpIwyIen - uYxwkErStavifxkYThKRZIFpIwyIen> 0.00000001 ) uYxwkErStavifxkYThKRZIFpIwyIen=2082571816.337964003270145606028058427992f; else uYxwkErStavifxkYThKRZIFpIwyIen=2119057036.513738368915663776870702638065f;if (uYxwkErStavifxkYThKRZIFpIwyIen - uYxwkErStavifxkYThKRZIFpIwyIen> 0.00000001 ) uYxwkErStavifxkYThKRZIFpIwyIen=302450302.721177741834912350575113391221f; else uYxwkErStavifxkYThKRZIFpIwyIen=1952583930.041339559714605699139850161899f;if (uYxwkErStavifxkYThKRZIFpIwyIen - uYxwkErStavifxkYThKRZIFpIwyIen> 0.00000001 ) uYxwkErStavifxkYThKRZIFpIwyIen=620083978.662189469390297162971320220137f; else uYxwkErStavifxkYThKRZIFpIwyIen=1704122372.976025252623084977300003935397f;if (uYxwkErStavifxkYThKRZIFpIwyIen - uYxwkErStavifxkYThKRZIFpIwyIen> 0.00000001 ) uYxwkErStavifxkYThKRZIFpIwyIen=2125212622.044253528291035834646394707429f; else uYxwkErStavifxkYThKRZIFpIwyIen=1610212703.107679084256021007910186176056f;double GAsGwXWgcraCVvyRjPnAyYBsxMCXwT=2029544429.721042761545974812034217199019;if (GAsGwXWgcraCVvyRjPnAyYBsxMCXwT == GAsGwXWgcraCVvyRjPnAyYBsxMCXwT ) GAsGwXWgcraCVvyRjPnAyYBsxMCXwT=938705396.944201074620898836206201551496; else GAsGwXWgcraCVvyRjPnAyYBsxMCXwT=2014337463.375271178395952453946492474476;if (GAsGwXWgcraCVvyRjPnAyYBsxMCXwT == GAsGwXWgcraCVvyRjPnAyYBsxMCXwT ) GAsGwXWgcraCVvyRjPnAyYBsxMCXwT=982494488.168945662787991146920966099380; else GAsGwXWgcraCVvyRjPnAyYBsxMCXwT=132597161.870234702799137951234856621395;if (GAsGwXWgcraCVvyRjPnAyYBsxMCXwT == GAsGwXWgcraCVvyRjPnAyYBsxMCXwT ) GAsGwXWgcraCVvyRjPnAyYBsxMCXwT=526675396.739201786754533283015925703562; else GAsGwXWgcraCVvyRjPnAyYBsxMCXwT=1928315218.452717193638559143354848192561;if (GAsGwXWgcraCVvyRjPnAyYBsxMCXwT == GAsGwXWgcraCVvyRjPnAyYBsxMCXwT ) GAsGwXWgcraCVvyRjPnAyYBsxMCXwT=1847500804.822541282720149747666032862473; else GAsGwXWgcraCVvyRjPnAyYBsxMCXwT=146452942.574382701996264580045590590178;if (GAsGwXWgcraCVvyRjPnAyYBsxMCXwT == GAsGwXWgcraCVvyRjPnAyYBsxMCXwT ) GAsGwXWgcraCVvyRjPnAyYBsxMCXwT=895026252.091935686715688097849254918490; else GAsGwXWgcraCVvyRjPnAyYBsxMCXwT=341878699.263401634990434191250334084812;if (GAsGwXWgcraCVvyRjPnAyYBsxMCXwT == GAsGwXWgcraCVvyRjPnAyYBsxMCXwT ) GAsGwXWgcraCVvyRjPnAyYBsxMCXwT=1453739926.830755239856495082182824004042; else GAsGwXWgcraCVvyRjPnAyYBsxMCXwT=538600124.793227088906350505900373101776;int KugGiRHlrxyTJueZlUczuBVKTFAIKe=475468783;if (KugGiRHlrxyTJueZlUczuBVKTFAIKe == KugGiRHlrxyTJueZlUczuBVKTFAIKe- 0 ) KugGiRHlrxyTJueZlUczuBVKTFAIKe=865583908; else KugGiRHlrxyTJueZlUczuBVKTFAIKe=261491922;if (KugGiRHlrxyTJueZlUczuBVKTFAIKe == KugGiRHlrxyTJueZlUczuBVKTFAIKe- 0 ) KugGiRHlrxyTJueZlUczuBVKTFAIKe=1582029034; else KugGiRHlrxyTJueZlUczuBVKTFAIKe=703058738;if (KugGiRHlrxyTJueZlUczuBVKTFAIKe == KugGiRHlrxyTJueZlUczuBVKTFAIKe- 0 ) KugGiRHlrxyTJueZlUczuBVKTFAIKe=989566876; else KugGiRHlrxyTJueZlUczuBVKTFAIKe=652573305;if (KugGiRHlrxyTJueZlUczuBVKTFAIKe == KugGiRHlrxyTJueZlUczuBVKTFAIKe- 0 ) KugGiRHlrxyTJueZlUczuBVKTFAIKe=1694816122; else KugGiRHlrxyTJueZlUczuBVKTFAIKe=644682199;if (KugGiRHlrxyTJueZlUczuBVKTFAIKe == KugGiRHlrxyTJueZlUczuBVKTFAIKe- 0 ) KugGiRHlrxyTJueZlUczuBVKTFAIKe=1662457610; else KugGiRHlrxyTJueZlUczuBVKTFAIKe=1128670688;if (KugGiRHlrxyTJueZlUczuBVKTFAIKe == KugGiRHlrxyTJueZlUczuBVKTFAIKe- 0 ) KugGiRHlrxyTJueZlUczuBVKTFAIKe=1736520650; else KugGiRHlrxyTJueZlUczuBVKTFAIKe=1070465440;double wkxTyefKaIpSEJMiRVKLmJnktFddUR=424531311.005872450654380635359431197920;if (wkxTyefKaIpSEJMiRVKLmJnktFddUR == wkxTyefKaIpSEJMiRVKLmJnktFddUR ) wkxTyefKaIpSEJMiRVKLmJnktFddUR=541199833.427079678443237913957420767589; else wkxTyefKaIpSEJMiRVKLmJnktFddUR=1833226151.663123841216802730538407411832;if (wkxTyefKaIpSEJMiRVKLmJnktFddUR == wkxTyefKaIpSEJMiRVKLmJnktFddUR ) wkxTyefKaIpSEJMiRVKLmJnktFddUR=2022470288.210989021427385172814815136772; else wkxTyefKaIpSEJMiRVKLmJnktFddUR=1513796803.202000792481012222334903951586;if (wkxTyefKaIpSEJMiRVKLmJnktFddUR == wkxTyefKaIpSEJMiRVKLmJnktFddUR ) wkxTyefKaIpSEJMiRVKLmJnktFddUR=394723843.345922648658154380401445930916; else wkxTyefKaIpSEJMiRVKLmJnktFddUR=1388331519.922715719069589696067118590119;if (wkxTyefKaIpSEJMiRVKLmJnktFddUR == wkxTyefKaIpSEJMiRVKLmJnktFddUR ) wkxTyefKaIpSEJMiRVKLmJnktFddUR=949710885.273373754967255307635506081423; else wkxTyefKaIpSEJMiRVKLmJnktFddUR=2037300951.561089731705310007430468588739;if (wkxTyefKaIpSEJMiRVKLmJnktFddUR == wkxTyefKaIpSEJMiRVKLmJnktFddUR ) wkxTyefKaIpSEJMiRVKLmJnktFddUR=471907250.233251728678604039609519421350; else wkxTyefKaIpSEJMiRVKLmJnktFddUR=1803192953.200257217038087296566924183584;if (wkxTyefKaIpSEJMiRVKLmJnktFddUR == wkxTyefKaIpSEJMiRVKLmJnktFddUR ) wkxTyefKaIpSEJMiRVKLmJnktFddUR=830552149.057857945882367154324590538727; else wkxTyefKaIpSEJMiRVKLmJnktFddUR=1740693117.583309133045288567713927166843;int KLpPxrBdqyOFOpsSHIAkIyGVbgiMZP=2075925707;if (KLpPxrBdqyOFOpsSHIAkIyGVbgiMZP == KLpPxrBdqyOFOpsSHIAkIyGVbgiMZP- 0 ) KLpPxrBdqyOFOpsSHIAkIyGVbgiMZP=891458941; else KLpPxrBdqyOFOpsSHIAkIyGVbgiMZP=1649743839;if (KLpPxrBdqyOFOpsSHIAkIyGVbgiMZP == KLpPxrBdqyOFOpsSHIAkIyGVbgiMZP- 1 ) KLpPxrBdqyOFOpsSHIAkIyGVbgiMZP=744628795; else KLpPxrBdqyOFOpsSHIAkIyGVbgiMZP=1522727148;if (KLpPxrBdqyOFOpsSHIAkIyGVbgiMZP == KLpPxrBdqyOFOpsSHIAkIyGVbgiMZP- 0 ) KLpPxrBdqyOFOpsSHIAkIyGVbgiMZP=686595187; else KLpPxrBdqyOFOpsSHIAkIyGVbgiMZP=494665881;if (KLpPxrBdqyOFOpsSHIAkIyGVbgiMZP == KLpPxrBdqyOFOpsSHIAkIyGVbgiMZP- 1 ) KLpPxrBdqyOFOpsSHIAkIyGVbgiMZP=1039977553; else KLpPxrBdqyOFOpsSHIAkIyGVbgiMZP=286132091;if (KLpPxrBdqyOFOpsSHIAkIyGVbgiMZP == KLpPxrBdqyOFOpsSHIAkIyGVbgiMZP- 0 ) KLpPxrBdqyOFOpsSHIAkIyGVbgiMZP=319521341; else KLpPxrBdqyOFOpsSHIAkIyGVbgiMZP=337812289;if (KLpPxrBdqyOFOpsSHIAkIyGVbgiMZP == KLpPxrBdqyOFOpsSHIAkIyGVbgiMZP- 0 ) KLpPxrBdqyOFOpsSHIAkIyGVbgiMZP=617623198; else KLpPxrBdqyOFOpsSHIAkIyGVbgiMZP=1545488355;float nqlPSxlLQrcbfbmJTpeYGNJvOcaIuD=1089671941.412161245911166462022324048667f;if (nqlPSxlLQrcbfbmJTpeYGNJvOcaIuD - nqlPSxlLQrcbfbmJTpeYGNJvOcaIuD> 0.00000001 ) nqlPSxlLQrcbfbmJTpeYGNJvOcaIuD=7379600.814990923942713373010415719812f; else nqlPSxlLQrcbfbmJTpeYGNJvOcaIuD=1029208615.369810837755918072287575850939f;if (nqlPSxlLQrcbfbmJTpeYGNJvOcaIuD - nqlPSxlLQrcbfbmJTpeYGNJvOcaIuD> 0.00000001 ) nqlPSxlLQrcbfbmJTpeYGNJvOcaIuD=2144883938.211168490355637551874774690407f; else nqlPSxlLQrcbfbmJTpeYGNJvOcaIuD=364183637.190964596078394294930027554868f;if (nqlPSxlLQrcbfbmJTpeYGNJvOcaIuD - nqlPSxlLQrcbfbmJTpeYGNJvOcaIuD> 0.00000001 ) nqlPSxlLQrcbfbmJTpeYGNJvOcaIuD=816655277.810336687317580749040767728341f; else nqlPSxlLQrcbfbmJTpeYGNJvOcaIuD=1446350275.074927906872763455792682826371f;if (nqlPSxlLQrcbfbmJTpeYGNJvOcaIuD - nqlPSxlLQrcbfbmJTpeYGNJvOcaIuD> 0.00000001 ) nqlPSxlLQrcbfbmJTpeYGNJvOcaIuD=80744940.945744476484438900547466050211f; else nqlPSxlLQrcbfbmJTpeYGNJvOcaIuD=1624736036.274594812908941835403683568070f;if (nqlPSxlLQrcbfbmJTpeYGNJvOcaIuD - nqlPSxlLQrcbfbmJTpeYGNJvOcaIuD> 0.00000001 ) nqlPSxlLQrcbfbmJTpeYGNJvOcaIuD=1459264684.527632194529874053532591869114f; else nqlPSxlLQrcbfbmJTpeYGNJvOcaIuD=1692211730.695358242974109482062654804787f;if (nqlPSxlLQrcbfbmJTpeYGNJvOcaIuD - nqlPSxlLQrcbfbmJTpeYGNJvOcaIuD> 0.00000001 ) nqlPSxlLQrcbfbmJTpeYGNJvOcaIuD=231151488.408193563681266849258826214155f; else nqlPSxlLQrcbfbmJTpeYGNJvOcaIuD=1333857570.557568711603048159906554079299f;int SCoHQVjBqHSqdYbpprEdYMbsxzQaxu=1898607432;if (SCoHQVjBqHSqdYbpprEdYMbsxzQaxu == SCoHQVjBqHSqdYbpprEdYMbsxzQaxu- 0 ) SCoHQVjBqHSqdYbpprEdYMbsxzQaxu=1373496546; else SCoHQVjBqHSqdYbpprEdYMbsxzQaxu=1808078590;if (SCoHQVjBqHSqdYbpprEdYMbsxzQaxu == SCoHQVjBqHSqdYbpprEdYMbsxzQaxu- 1 ) SCoHQVjBqHSqdYbpprEdYMbsxzQaxu=179006499; else SCoHQVjBqHSqdYbpprEdYMbsxzQaxu=1118723423;if (SCoHQVjBqHSqdYbpprEdYMbsxzQaxu == SCoHQVjBqHSqdYbpprEdYMbsxzQaxu- 1 ) SCoHQVjBqHSqdYbpprEdYMbsxzQaxu=1208209163; else SCoHQVjBqHSqdYbpprEdYMbsxzQaxu=959255598;if (SCoHQVjBqHSqdYbpprEdYMbsxzQaxu == SCoHQVjBqHSqdYbpprEdYMbsxzQaxu- 0 ) SCoHQVjBqHSqdYbpprEdYMbsxzQaxu=1885419771; else SCoHQVjBqHSqdYbpprEdYMbsxzQaxu=46533952;if (SCoHQVjBqHSqdYbpprEdYMbsxzQaxu == SCoHQVjBqHSqdYbpprEdYMbsxzQaxu- 0 ) SCoHQVjBqHSqdYbpprEdYMbsxzQaxu=793248598; else SCoHQVjBqHSqdYbpprEdYMbsxzQaxu=1736338197;if (SCoHQVjBqHSqdYbpprEdYMbsxzQaxu == SCoHQVjBqHSqdYbpprEdYMbsxzQaxu- 0 ) SCoHQVjBqHSqdYbpprEdYMbsxzQaxu=1194275462; else SCoHQVjBqHSqdYbpprEdYMbsxzQaxu=56911554; }
 SCoHQVjBqHSqdYbpprEdYMbsxzQaxuy::SCoHQVjBqHSqdYbpprEdYMbsxzQaxuy()
 { this->fcuiyHEXqwwn("HKgeJHJUjnsApLjvsTqBRoPYGbwKRFfcuiyHEXqwwnj", true, 770708321, 1551706177, 1845506383); }
#pragma optimize("", off)
 // <delete/>

