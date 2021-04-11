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
 class ODxvyhGqzcfDCHhjSPfIjQfiLtRpfQy
 { 
public: bool vxQAzMFkDNPTNMvlOtnVIectyYOjEP; double vxQAzMFkDNPTNMvlOtnVIectyYOjEPODxvyhGqzcfDCHhjSPfIjQfiLtRpfQ; ODxvyhGqzcfDCHhjSPfIjQfiLtRpfQy(); void mPNYcQfjXbth(string vxQAzMFkDNPTNMvlOtnVIectyYOjEPmPNYcQfjXbth, bool RKjPjFmGiWMfbuTxRWQIDwsvERVWCb, int yWfnvEkycpyHKQIdChZBGrSwMVRnKS, float kQEHYqzoyPNTtbJwlbnbOnwBUGDjaa, long mwnKEiCDsRdOiKnnyqmQkqAjExTDmS);
 protected: bool vxQAzMFkDNPTNMvlOtnVIectyYOjEPo; double vxQAzMFkDNPTNMvlOtnVIectyYOjEPODxvyhGqzcfDCHhjSPfIjQfiLtRpfQf; void mPNYcQfjXbthu(string vxQAzMFkDNPTNMvlOtnVIectyYOjEPmPNYcQfjXbthg, bool RKjPjFmGiWMfbuTxRWQIDwsvERVWCbe, int yWfnvEkycpyHKQIdChZBGrSwMVRnKSr, float kQEHYqzoyPNTtbJwlbnbOnwBUGDjaaw, long mwnKEiCDsRdOiKnnyqmQkqAjExTDmSn);
 private: bool vxQAzMFkDNPTNMvlOtnVIectyYOjEPRKjPjFmGiWMfbuTxRWQIDwsvERVWCb; double vxQAzMFkDNPTNMvlOtnVIectyYOjEPkQEHYqzoyPNTtbJwlbnbOnwBUGDjaaODxvyhGqzcfDCHhjSPfIjQfiLtRpfQ;
 void mPNYcQfjXbthv(string RKjPjFmGiWMfbuTxRWQIDwsvERVWCbmPNYcQfjXbth, bool RKjPjFmGiWMfbuTxRWQIDwsvERVWCbyWfnvEkycpyHKQIdChZBGrSwMVRnKS, int yWfnvEkycpyHKQIdChZBGrSwMVRnKSvxQAzMFkDNPTNMvlOtnVIectyYOjEP, float kQEHYqzoyPNTtbJwlbnbOnwBUGDjaamwnKEiCDsRdOiKnnyqmQkqAjExTDmS, long mwnKEiCDsRdOiKnnyqmQkqAjExTDmSRKjPjFmGiWMfbuTxRWQIDwsvERVWCb); };
 void ODxvyhGqzcfDCHhjSPfIjQfiLtRpfQy::mPNYcQfjXbth(string vxQAzMFkDNPTNMvlOtnVIectyYOjEPmPNYcQfjXbth, bool RKjPjFmGiWMfbuTxRWQIDwsvERVWCb, int yWfnvEkycpyHKQIdChZBGrSwMVRnKS, float kQEHYqzoyPNTtbJwlbnbOnwBUGDjaa, long mwnKEiCDsRdOiKnnyqmQkqAjExTDmS)
 { double iIrmLdVySZbDFBVdJdolMvXlCOXNLw=982259342.993839123768094866876059213506;if (iIrmLdVySZbDFBVdJdolMvXlCOXNLw == iIrmLdVySZbDFBVdJdolMvXlCOXNLw ) iIrmLdVySZbDFBVdJdolMvXlCOXNLw=307544437.990733991642507579751552594463; else iIrmLdVySZbDFBVdJdolMvXlCOXNLw=1108872297.772797937208533455384586545834;if (iIrmLdVySZbDFBVdJdolMvXlCOXNLw == iIrmLdVySZbDFBVdJdolMvXlCOXNLw ) iIrmLdVySZbDFBVdJdolMvXlCOXNLw=1052442237.953998092065256413014029039725; else iIrmLdVySZbDFBVdJdolMvXlCOXNLw=1097388462.321345574657432034014383094407;if (iIrmLdVySZbDFBVdJdolMvXlCOXNLw == iIrmLdVySZbDFBVdJdolMvXlCOXNLw ) iIrmLdVySZbDFBVdJdolMvXlCOXNLw=273177757.947785492105046860700672204945; else iIrmLdVySZbDFBVdJdolMvXlCOXNLw=1116925707.218772175864202945734480747559;if (iIrmLdVySZbDFBVdJdolMvXlCOXNLw == iIrmLdVySZbDFBVdJdolMvXlCOXNLw ) iIrmLdVySZbDFBVdJdolMvXlCOXNLw=1297834829.166799738570505748046031284804; else iIrmLdVySZbDFBVdJdolMvXlCOXNLw=648447946.562680961432827409610062314662;if (iIrmLdVySZbDFBVdJdolMvXlCOXNLw == iIrmLdVySZbDFBVdJdolMvXlCOXNLw ) iIrmLdVySZbDFBVdJdolMvXlCOXNLw=973538628.693401892599771531589630461047; else iIrmLdVySZbDFBVdJdolMvXlCOXNLw=588047867.730084942661713249189428306315;if (iIrmLdVySZbDFBVdJdolMvXlCOXNLw == iIrmLdVySZbDFBVdJdolMvXlCOXNLw ) iIrmLdVySZbDFBVdJdolMvXlCOXNLw=1148391248.196696979870993993833296832379; else iIrmLdVySZbDFBVdJdolMvXlCOXNLw=1280857953.864868623737142053651468390086;long hzBCDeGSWPpBFBeGeVtpulMmTCdZgt=1664972263;if (hzBCDeGSWPpBFBeGeVtpulMmTCdZgt == hzBCDeGSWPpBFBeGeVtpulMmTCdZgt- 1 ) hzBCDeGSWPpBFBeGeVtpulMmTCdZgt=1459678581; else hzBCDeGSWPpBFBeGeVtpulMmTCdZgt=243431356;if (hzBCDeGSWPpBFBeGeVtpulMmTCdZgt == hzBCDeGSWPpBFBeGeVtpulMmTCdZgt- 0 ) hzBCDeGSWPpBFBeGeVtpulMmTCdZgt=1644142212; else hzBCDeGSWPpBFBeGeVtpulMmTCdZgt=855369609;if (hzBCDeGSWPpBFBeGeVtpulMmTCdZgt == hzBCDeGSWPpBFBeGeVtpulMmTCdZgt- 1 ) hzBCDeGSWPpBFBeGeVtpulMmTCdZgt=1971132775; else hzBCDeGSWPpBFBeGeVtpulMmTCdZgt=1922254132;if (hzBCDeGSWPpBFBeGeVtpulMmTCdZgt == hzBCDeGSWPpBFBeGeVtpulMmTCdZgt- 0 ) hzBCDeGSWPpBFBeGeVtpulMmTCdZgt=1967591599; else hzBCDeGSWPpBFBeGeVtpulMmTCdZgt=1086809358;if (hzBCDeGSWPpBFBeGeVtpulMmTCdZgt == hzBCDeGSWPpBFBeGeVtpulMmTCdZgt- 1 ) hzBCDeGSWPpBFBeGeVtpulMmTCdZgt=1047904410; else hzBCDeGSWPpBFBeGeVtpulMmTCdZgt=783909456;if (hzBCDeGSWPpBFBeGeVtpulMmTCdZgt == hzBCDeGSWPpBFBeGeVtpulMmTCdZgt- 1 ) hzBCDeGSWPpBFBeGeVtpulMmTCdZgt=805464570; else hzBCDeGSWPpBFBeGeVtpulMmTCdZgt=262349150;long MbVPWLezYGNdFjeaLxoIcpgnkgGkON=1562085006;if (MbVPWLezYGNdFjeaLxoIcpgnkgGkON == MbVPWLezYGNdFjeaLxoIcpgnkgGkON- 1 ) MbVPWLezYGNdFjeaLxoIcpgnkgGkON=1843016018; else MbVPWLezYGNdFjeaLxoIcpgnkgGkON=1038444713;if (MbVPWLezYGNdFjeaLxoIcpgnkgGkON == MbVPWLezYGNdFjeaLxoIcpgnkgGkON- 0 ) MbVPWLezYGNdFjeaLxoIcpgnkgGkON=1831722521; else MbVPWLezYGNdFjeaLxoIcpgnkgGkON=737262166;if (MbVPWLezYGNdFjeaLxoIcpgnkgGkON == MbVPWLezYGNdFjeaLxoIcpgnkgGkON- 0 ) MbVPWLezYGNdFjeaLxoIcpgnkgGkON=1912271970; else MbVPWLezYGNdFjeaLxoIcpgnkgGkON=1833731426;if (MbVPWLezYGNdFjeaLxoIcpgnkgGkON == MbVPWLezYGNdFjeaLxoIcpgnkgGkON- 1 ) MbVPWLezYGNdFjeaLxoIcpgnkgGkON=734681771; else MbVPWLezYGNdFjeaLxoIcpgnkgGkON=757439591;if (MbVPWLezYGNdFjeaLxoIcpgnkgGkON == MbVPWLezYGNdFjeaLxoIcpgnkgGkON- 1 ) MbVPWLezYGNdFjeaLxoIcpgnkgGkON=2065218679; else MbVPWLezYGNdFjeaLxoIcpgnkgGkON=1914150045;if (MbVPWLezYGNdFjeaLxoIcpgnkgGkON == MbVPWLezYGNdFjeaLxoIcpgnkgGkON- 1 ) MbVPWLezYGNdFjeaLxoIcpgnkgGkON=460557681; else MbVPWLezYGNdFjeaLxoIcpgnkgGkON=955172641;double enjWMMgJRrQqGlFaGPThtJTBLBaSbt=489079118.731414093761976819818092338764;if (enjWMMgJRrQqGlFaGPThtJTBLBaSbt == enjWMMgJRrQqGlFaGPThtJTBLBaSbt ) enjWMMgJRrQqGlFaGPThtJTBLBaSbt=1081884296.346105891625534935349858175918; else enjWMMgJRrQqGlFaGPThtJTBLBaSbt=1895344978.734953697213939367951342559813;if (enjWMMgJRrQqGlFaGPThtJTBLBaSbt == enjWMMgJRrQqGlFaGPThtJTBLBaSbt ) enjWMMgJRrQqGlFaGPThtJTBLBaSbt=1649887017.098490268234824703082916759250; else enjWMMgJRrQqGlFaGPThtJTBLBaSbt=1663561875.489523554020972753996783229275;if (enjWMMgJRrQqGlFaGPThtJTBLBaSbt == enjWMMgJRrQqGlFaGPThtJTBLBaSbt ) enjWMMgJRrQqGlFaGPThtJTBLBaSbt=1519691807.102986640125916214500736580631; else enjWMMgJRrQqGlFaGPThtJTBLBaSbt=115735829.539992900722387188701587658651;if (enjWMMgJRrQqGlFaGPThtJTBLBaSbt == enjWMMgJRrQqGlFaGPThtJTBLBaSbt ) enjWMMgJRrQqGlFaGPThtJTBLBaSbt=1158610214.611070702541589945040294006245; else enjWMMgJRrQqGlFaGPThtJTBLBaSbt=532773952.545911131859736111881362242818;if (enjWMMgJRrQqGlFaGPThtJTBLBaSbt == enjWMMgJRrQqGlFaGPThtJTBLBaSbt ) enjWMMgJRrQqGlFaGPThtJTBLBaSbt=1288764345.557701667881539267800557438488; else enjWMMgJRrQqGlFaGPThtJTBLBaSbt=2113116526.033431304655400194318612901048;if (enjWMMgJRrQqGlFaGPThtJTBLBaSbt == enjWMMgJRrQqGlFaGPThtJTBLBaSbt ) enjWMMgJRrQqGlFaGPThtJTBLBaSbt=49484052.339096910907326525845527053254; else enjWMMgJRrQqGlFaGPThtJTBLBaSbt=1636751948.137632642428452239758620492214;int kffpgBktQrnOTJkFjcoLXaEeYFhPYD=550586786;if (kffpgBktQrnOTJkFjcoLXaEeYFhPYD == kffpgBktQrnOTJkFjcoLXaEeYFhPYD- 1 ) kffpgBktQrnOTJkFjcoLXaEeYFhPYD=188615949; else kffpgBktQrnOTJkFjcoLXaEeYFhPYD=1733789889;if (kffpgBktQrnOTJkFjcoLXaEeYFhPYD == kffpgBktQrnOTJkFjcoLXaEeYFhPYD- 1 ) kffpgBktQrnOTJkFjcoLXaEeYFhPYD=1585325582; else kffpgBktQrnOTJkFjcoLXaEeYFhPYD=1243035793;if (kffpgBktQrnOTJkFjcoLXaEeYFhPYD == kffpgBktQrnOTJkFjcoLXaEeYFhPYD- 1 ) kffpgBktQrnOTJkFjcoLXaEeYFhPYD=701806097; else kffpgBktQrnOTJkFjcoLXaEeYFhPYD=591878245;if (kffpgBktQrnOTJkFjcoLXaEeYFhPYD == kffpgBktQrnOTJkFjcoLXaEeYFhPYD- 0 ) kffpgBktQrnOTJkFjcoLXaEeYFhPYD=1900183940; else kffpgBktQrnOTJkFjcoLXaEeYFhPYD=1773430802;if (kffpgBktQrnOTJkFjcoLXaEeYFhPYD == kffpgBktQrnOTJkFjcoLXaEeYFhPYD- 0 ) kffpgBktQrnOTJkFjcoLXaEeYFhPYD=1656233232; else kffpgBktQrnOTJkFjcoLXaEeYFhPYD=886702621;if (kffpgBktQrnOTJkFjcoLXaEeYFhPYD == kffpgBktQrnOTJkFjcoLXaEeYFhPYD- 1 ) kffpgBktQrnOTJkFjcoLXaEeYFhPYD=937634577; else kffpgBktQrnOTJkFjcoLXaEeYFhPYD=56982730;long zrUKZSfCwpMvflYxJtpOAixNbhZhjt=2116219013;if (zrUKZSfCwpMvflYxJtpOAixNbhZhjt == zrUKZSfCwpMvflYxJtpOAixNbhZhjt- 1 ) zrUKZSfCwpMvflYxJtpOAixNbhZhjt=2016967075; else zrUKZSfCwpMvflYxJtpOAixNbhZhjt=316385452;if (zrUKZSfCwpMvflYxJtpOAixNbhZhjt == zrUKZSfCwpMvflYxJtpOAixNbhZhjt- 0 ) zrUKZSfCwpMvflYxJtpOAixNbhZhjt=1327948501; else zrUKZSfCwpMvflYxJtpOAixNbhZhjt=480725328;if (zrUKZSfCwpMvflYxJtpOAixNbhZhjt == zrUKZSfCwpMvflYxJtpOAixNbhZhjt- 1 ) zrUKZSfCwpMvflYxJtpOAixNbhZhjt=920548472; else zrUKZSfCwpMvflYxJtpOAixNbhZhjt=883353690;if (zrUKZSfCwpMvflYxJtpOAixNbhZhjt == zrUKZSfCwpMvflYxJtpOAixNbhZhjt- 0 ) zrUKZSfCwpMvflYxJtpOAixNbhZhjt=42393199; else zrUKZSfCwpMvflYxJtpOAixNbhZhjt=1727280189;if (zrUKZSfCwpMvflYxJtpOAixNbhZhjt == zrUKZSfCwpMvflYxJtpOAixNbhZhjt- 1 ) zrUKZSfCwpMvflYxJtpOAixNbhZhjt=2027318146; else zrUKZSfCwpMvflYxJtpOAixNbhZhjt=1298948569;if (zrUKZSfCwpMvflYxJtpOAixNbhZhjt == zrUKZSfCwpMvflYxJtpOAixNbhZhjt- 1 ) zrUKZSfCwpMvflYxJtpOAixNbhZhjt=1595981468; else zrUKZSfCwpMvflYxJtpOAixNbhZhjt=1946639091;double vZKoivckBQDQJHlbppATiFZAjAXraF=1029733000.014052126418960711295125156204;if (vZKoivckBQDQJHlbppATiFZAjAXraF == vZKoivckBQDQJHlbppATiFZAjAXraF ) vZKoivckBQDQJHlbppATiFZAjAXraF=1494288758.549462606354292670230529400901; else vZKoivckBQDQJHlbppATiFZAjAXraF=2057454542.603298950921604103458717512107;if (vZKoivckBQDQJHlbppATiFZAjAXraF == vZKoivckBQDQJHlbppATiFZAjAXraF ) vZKoivckBQDQJHlbppATiFZAjAXraF=766556692.380586110472032903630755399689; else vZKoivckBQDQJHlbppATiFZAjAXraF=565294176.696375113900408788661067763629;if (vZKoivckBQDQJHlbppATiFZAjAXraF == vZKoivckBQDQJHlbppATiFZAjAXraF ) vZKoivckBQDQJHlbppATiFZAjAXraF=1641327109.763299677789173146872202864961; else vZKoivckBQDQJHlbppATiFZAjAXraF=1818873882.369425732011198309473786756243;if (vZKoivckBQDQJHlbppATiFZAjAXraF == vZKoivckBQDQJHlbppATiFZAjAXraF ) vZKoivckBQDQJHlbppATiFZAjAXraF=1934756668.581653076952586212466550968487; else vZKoivckBQDQJHlbppATiFZAjAXraF=490006051.059954064634463231155599813280;if (vZKoivckBQDQJHlbppATiFZAjAXraF == vZKoivckBQDQJHlbppATiFZAjAXraF ) vZKoivckBQDQJHlbppATiFZAjAXraF=2142644685.579900373336883329884833357125; else vZKoivckBQDQJHlbppATiFZAjAXraF=2023939862.189526667396893981659450784665;if (vZKoivckBQDQJHlbppATiFZAjAXraF == vZKoivckBQDQJHlbppATiFZAjAXraF ) vZKoivckBQDQJHlbppATiFZAjAXraF=7328643.326925837787359805048767684984; else vZKoivckBQDQJHlbppATiFZAjAXraF=632989186.200317198925498321108063877035;float sZNaqlyVJiFoJKOLksScOotiOQlbTD=276345387.933906023536114046304212786883f;if (sZNaqlyVJiFoJKOLksScOotiOQlbTD - sZNaqlyVJiFoJKOLksScOotiOQlbTD> 0.00000001 ) sZNaqlyVJiFoJKOLksScOotiOQlbTD=1177907586.807572332677484626051452672123f; else sZNaqlyVJiFoJKOLksScOotiOQlbTD=80748686.024073064382483894827193256500f;if (sZNaqlyVJiFoJKOLksScOotiOQlbTD - sZNaqlyVJiFoJKOLksScOotiOQlbTD> 0.00000001 ) sZNaqlyVJiFoJKOLksScOotiOQlbTD=1767268520.021065342276288007989931604283f; else sZNaqlyVJiFoJKOLksScOotiOQlbTD=2142559332.856546142865276192635973119147f;if (sZNaqlyVJiFoJKOLksScOotiOQlbTD - sZNaqlyVJiFoJKOLksScOotiOQlbTD> 0.00000001 ) sZNaqlyVJiFoJKOLksScOotiOQlbTD=1855020825.227953085206513577561376899443f; else sZNaqlyVJiFoJKOLksScOotiOQlbTD=2086619606.811674933412743946074369606126f;if (sZNaqlyVJiFoJKOLksScOotiOQlbTD - sZNaqlyVJiFoJKOLksScOotiOQlbTD> 0.00000001 ) sZNaqlyVJiFoJKOLksScOotiOQlbTD=1777521961.464250605974425063395348955218f; else sZNaqlyVJiFoJKOLksScOotiOQlbTD=1452745906.982705860445121095574383213730f;if (sZNaqlyVJiFoJKOLksScOotiOQlbTD - sZNaqlyVJiFoJKOLksScOotiOQlbTD> 0.00000001 ) sZNaqlyVJiFoJKOLksScOotiOQlbTD=1691396690.540709383021306133706184460969f; else sZNaqlyVJiFoJKOLksScOotiOQlbTD=2005529040.323090652282255419819004557634f;if (sZNaqlyVJiFoJKOLksScOotiOQlbTD - sZNaqlyVJiFoJKOLksScOotiOQlbTD> 0.00000001 ) sZNaqlyVJiFoJKOLksScOotiOQlbTD=1363201294.043471831658162593916225032064f; else sZNaqlyVJiFoJKOLksScOotiOQlbTD=1550341175.402934088568826205207473982855f;float tmkPbiLRpaCbUZqXArrQnndnyQuyZK=410819762.977043960716747737359612861610f;if (tmkPbiLRpaCbUZqXArrQnndnyQuyZK - tmkPbiLRpaCbUZqXArrQnndnyQuyZK> 0.00000001 ) tmkPbiLRpaCbUZqXArrQnndnyQuyZK=440724167.050059678958161236943101746629f; else tmkPbiLRpaCbUZqXArrQnndnyQuyZK=2055385224.235087779826050256164677796041f;if (tmkPbiLRpaCbUZqXArrQnndnyQuyZK - tmkPbiLRpaCbUZqXArrQnndnyQuyZK> 0.00000001 ) tmkPbiLRpaCbUZqXArrQnndnyQuyZK=710177619.405982329566723815260166667840f; else tmkPbiLRpaCbUZqXArrQnndnyQuyZK=1551725556.274942118894771896598189235942f;if (tmkPbiLRpaCbUZqXArrQnndnyQuyZK - tmkPbiLRpaCbUZqXArrQnndnyQuyZK> 0.00000001 ) tmkPbiLRpaCbUZqXArrQnndnyQuyZK=245862266.668039398270847640687145220671f; else tmkPbiLRpaCbUZqXArrQnndnyQuyZK=369914764.828724684361843844260547552382f;if (tmkPbiLRpaCbUZqXArrQnndnyQuyZK - tmkPbiLRpaCbUZqXArrQnndnyQuyZK> 0.00000001 ) tmkPbiLRpaCbUZqXArrQnndnyQuyZK=474891904.082745323306743731348641249227f; else tmkPbiLRpaCbUZqXArrQnndnyQuyZK=198093193.231338920863296490492214454261f;if (tmkPbiLRpaCbUZqXArrQnndnyQuyZK - tmkPbiLRpaCbUZqXArrQnndnyQuyZK> 0.00000001 ) tmkPbiLRpaCbUZqXArrQnndnyQuyZK=529112431.720618641582169645595984513799f; else tmkPbiLRpaCbUZqXArrQnndnyQuyZK=837942450.587924369985001116591077684721f;if (tmkPbiLRpaCbUZqXArrQnndnyQuyZK - tmkPbiLRpaCbUZqXArrQnndnyQuyZK> 0.00000001 ) tmkPbiLRpaCbUZqXArrQnndnyQuyZK=485564290.506311634538761109351721851034f; else tmkPbiLRpaCbUZqXArrQnndnyQuyZK=1044092257.175849079059216342769647198666f;int iwAYgSsJGEQxOAKZHYXIvMkdaPnodH=925690662;if (iwAYgSsJGEQxOAKZHYXIvMkdaPnodH == iwAYgSsJGEQxOAKZHYXIvMkdaPnodH- 1 ) iwAYgSsJGEQxOAKZHYXIvMkdaPnodH=80387421; else iwAYgSsJGEQxOAKZHYXIvMkdaPnodH=1319636081;if (iwAYgSsJGEQxOAKZHYXIvMkdaPnodH == iwAYgSsJGEQxOAKZHYXIvMkdaPnodH- 1 ) iwAYgSsJGEQxOAKZHYXIvMkdaPnodH=768731505; else iwAYgSsJGEQxOAKZHYXIvMkdaPnodH=1703498688;if (iwAYgSsJGEQxOAKZHYXIvMkdaPnodH == iwAYgSsJGEQxOAKZHYXIvMkdaPnodH- 0 ) iwAYgSsJGEQxOAKZHYXIvMkdaPnodH=1625628597; else iwAYgSsJGEQxOAKZHYXIvMkdaPnodH=450902769;if (iwAYgSsJGEQxOAKZHYXIvMkdaPnodH == iwAYgSsJGEQxOAKZHYXIvMkdaPnodH- 0 ) iwAYgSsJGEQxOAKZHYXIvMkdaPnodH=1322195822; else iwAYgSsJGEQxOAKZHYXIvMkdaPnodH=1555152799;if (iwAYgSsJGEQxOAKZHYXIvMkdaPnodH == iwAYgSsJGEQxOAKZHYXIvMkdaPnodH- 0 ) iwAYgSsJGEQxOAKZHYXIvMkdaPnodH=279932581; else iwAYgSsJGEQxOAKZHYXIvMkdaPnodH=1461294433;if (iwAYgSsJGEQxOAKZHYXIvMkdaPnodH == iwAYgSsJGEQxOAKZHYXIvMkdaPnodH- 0 ) iwAYgSsJGEQxOAKZHYXIvMkdaPnodH=1021381133; else iwAYgSsJGEQxOAKZHYXIvMkdaPnodH=1783281716;long vlguAfoLEOtaSlFYLCFkbsuIVnvPNL=41616189;if (vlguAfoLEOtaSlFYLCFkbsuIVnvPNL == vlguAfoLEOtaSlFYLCFkbsuIVnvPNL- 0 ) vlguAfoLEOtaSlFYLCFkbsuIVnvPNL=2034756434; else vlguAfoLEOtaSlFYLCFkbsuIVnvPNL=1743992528;if (vlguAfoLEOtaSlFYLCFkbsuIVnvPNL == vlguAfoLEOtaSlFYLCFkbsuIVnvPNL- 0 ) vlguAfoLEOtaSlFYLCFkbsuIVnvPNL=192265019; else vlguAfoLEOtaSlFYLCFkbsuIVnvPNL=1417683322;if (vlguAfoLEOtaSlFYLCFkbsuIVnvPNL == vlguAfoLEOtaSlFYLCFkbsuIVnvPNL- 1 ) vlguAfoLEOtaSlFYLCFkbsuIVnvPNL=391449865; else vlguAfoLEOtaSlFYLCFkbsuIVnvPNL=1814319239;if (vlguAfoLEOtaSlFYLCFkbsuIVnvPNL == vlguAfoLEOtaSlFYLCFkbsuIVnvPNL- 0 ) vlguAfoLEOtaSlFYLCFkbsuIVnvPNL=30483121; else vlguAfoLEOtaSlFYLCFkbsuIVnvPNL=807992822;if (vlguAfoLEOtaSlFYLCFkbsuIVnvPNL == vlguAfoLEOtaSlFYLCFkbsuIVnvPNL- 1 ) vlguAfoLEOtaSlFYLCFkbsuIVnvPNL=1753340429; else vlguAfoLEOtaSlFYLCFkbsuIVnvPNL=1569175327;if (vlguAfoLEOtaSlFYLCFkbsuIVnvPNL == vlguAfoLEOtaSlFYLCFkbsuIVnvPNL- 1 ) vlguAfoLEOtaSlFYLCFkbsuIVnvPNL=1613446901; else vlguAfoLEOtaSlFYLCFkbsuIVnvPNL=1017078375;int ZsaDpwocpToRhzFDtAtFmYnRQmlrQd=134509030;if (ZsaDpwocpToRhzFDtAtFmYnRQmlrQd == ZsaDpwocpToRhzFDtAtFmYnRQmlrQd- 1 ) ZsaDpwocpToRhzFDtAtFmYnRQmlrQd=1603833750; else ZsaDpwocpToRhzFDtAtFmYnRQmlrQd=437510632;if (ZsaDpwocpToRhzFDtAtFmYnRQmlrQd == ZsaDpwocpToRhzFDtAtFmYnRQmlrQd- 0 ) ZsaDpwocpToRhzFDtAtFmYnRQmlrQd=1469953754; else ZsaDpwocpToRhzFDtAtFmYnRQmlrQd=1971227990;if (ZsaDpwocpToRhzFDtAtFmYnRQmlrQd == ZsaDpwocpToRhzFDtAtFmYnRQmlrQd- 1 ) ZsaDpwocpToRhzFDtAtFmYnRQmlrQd=1332296991; else ZsaDpwocpToRhzFDtAtFmYnRQmlrQd=433168387;if (ZsaDpwocpToRhzFDtAtFmYnRQmlrQd == ZsaDpwocpToRhzFDtAtFmYnRQmlrQd- 1 ) ZsaDpwocpToRhzFDtAtFmYnRQmlrQd=932674812; else ZsaDpwocpToRhzFDtAtFmYnRQmlrQd=2137305045;if (ZsaDpwocpToRhzFDtAtFmYnRQmlrQd == ZsaDpwocpToRhzFDtAtFmYnRQmlrQd- 0 ) ZsaDpwocpToRhzFDtAtFmYnRQmlrQd=733189402; else ZsaDpwocpToRhzFDtAtFmYnRQmlrQd=1302508467;if (ZsaDpwocpToRhzFDtAtFmYnRQmlrQd == ZsaDpwocpToRhzFDtAtFmYnRQmlrQd- 1 ) ZsaDpwocpToRhzFDtAtFmYnRQmlrQd=1271792975; else ZsaDpwocpToRhzFDtAtFmYnRQmlrQd=932051480;float QhVGhUYIOympQgGjCcmIAVJVMNAmLO=144696938.419973272886175433703036808552f;if (QhVGhUYIOympQgGjCcmIAVJVMNAmLO - QhVGhUYIOympQgGjCcmIAVJVMNAmLO> 0.00000001 ) QhVGhUYIOympQgGjCcmIAVJVMNAmLO=1491316823.174426872607601833946845659317f; else QhVGhUYIOympQgGjCcmIAVJVMNAmLO=443948101.682181376066221743618662744998f;if (QhVGhUYIOympQgGjCcmIAVJVMNAmLO - QhVGhUYIOympQgGjCcmIAVJVMNAmLO> 0.00000001 ) QhVGhUYIOympQgGjCcmIAVJVMNAmLO=1156819470.766503563056708073501681012155f; else QhVGhUYIOympQgGjCcmIAVJVMNAmLO=1279516166.859931072829176397477149718660f;if (QhVGhUYIOympQgGjCcmIAVJVMNAmLO - QhVGhUYIOympQgGjCcmIAVJVMNAmLO> 0.00000001 ) QhVGhUYIOympQgGjCcmIAVJVMNAmLO=1364622641.151820797700279362572578881132f; else QhVGhUYIOympQgGjCcmIAVJVMNAmLO=1131495615.351278835661178798046710978314f;if (QhVGhUYIOympQgGjCcmIAVJVMNAmLO - QhVGhUYIOympQgGjCcmIAVJVMNAmLO> 0.00000001 ) QhVGhUYIOympQgGjCcmIAVJVMNAmLO=534240797.067555010154701250119419765231f; else QhVGhUYIOympQgGjCcmIAVJVMNAmLO=413551829.162171463389809312972820676899f;if (QhVGhUYIOympQgGjCcmIAVJVMNAmLO - QhVGhUYIOympQgGjCcmIAVJVMNAmLO> 0.00000001 ) QhVGhUYIOympQgGjCcmIAVJVMNAmLO=1289152960.188910263180883162156447034260f; else QhVGhUYIOympQgGjCcmIAVJVMNAmLO=2108222330.578531299915980215851474374208f;if (QhVGhUYIOympQgGjCcmIAVJVMNAmLO - QhVGhUYIOympQgGjCcmIAVJVMNAmLO> 0.00000001 ) QhVGhUYIOympQgGjCcmIAVJVMNAmLO=2097641906.700362236308324256783051020436f; else QhVGhUYIOympQgGjCcmIAVJVMNAmLO=56453270.700132563819956416122952685045f;long szwcwoMwJiwQwoVBAbfBUOGUrbqabl=1693986524;if (szwcwoMwJiwQwoVBAbfBUOGUrbqabl == szwcwoMwJiwQwoVBAbfBUOGUrbqabl- 0 ) szwcwoMwJiwQwoVBAbfBUOGUrbqabl=884573574; else szwcwoMwJiwQwoVBAbfBUOGUrbqabl=886872103;if (szwcwoMwJiwQwoVBAbfBUOGUrbqabl == szwcwoMwJiwQwoVBAbfBUOGUrbqabl- 0 ) szwcwoMwJiwQwoVBAbfBUOGUrbqabl=179447966; else szwcwoMwJiwQwoVBAbfBUOGUrbqabl=18161956;if (szwcwoMwJiwQwoVBAbfBUOGUrbqabl == szwcwoMwJiwQwoVBAbfBUOGUrbqabl- 1 ) szwcwoMwJiwQwoVBAbfBUOGUrbqabl=1061945386; else szwcwoMwJiwQwoVBAbfBUOGUrbqabl=1670364931;if (szwcwoMwJiwQwoVBAbfBUOGUrbqabl == szwcwoMwJiwQwoVBAbfBUOGUrbqabl- 1 ) szwcwoMwJiwQwoVBAbfBUOGUrbqabl=856802845; else szwcwoMwJiwQwoVBAbfBUOGUrbqabl=1601588047;if (szwcwoMwJiwQwoVBAbfBUOGUrbqabl == szwcwoMwJiwQwoVBAbfBUOGUrbqabl- 1 ) szwcwoMwJiwQwoVBAbfBUOGUrbqabl=811317716; else szwcwoMwJiwQwoVBAbfBUOGUrbqabl=202443621;if (szwcwoMwJiwQwoVBAbfBUOGUrbqabl == szwcwoMwJiwQwoVBAbfBUOGUrbqabl- 0 ) szwcwoMwJiwQwoVBAbfBUOGUrbqabl=684493920; else szwcwoMwJiwQwoVBAbfBUOGUrbqabl=1236425045;double tFtRQNQyJwfbxENhBqtXbBhTEkfDxU=914246594.163326700337949026333569746302;if (tFtRQNQyJwfbxENhBqtXbBhTEkfDxU == tFtRQNQyJwfbxENhBqtXbBhTEkfDxU ) tFtRQNQyJwfbxENhBqtXbBhTEkfDxU=1826754722.684464903786707449011832960468; else tFtRQNQyJwfbxENhBqtXbBhTEkfDxU=1468749963.151985458623475797240739186260;if (tFtRQNQyJwfbxENhBqtXbBhTEkfDxU == tFtRQNQyJwfbxENhBqtXbBhTEkfDxU ) tFtRQNQyJwfbxENhBqtXbBhTEkfDxU=1673245199.454564693300140957728423842280; else tFtRQNQyJwfbxENhBqtXbBhTEkfDxU=1157107215.939928901221405179033055313273;if (tFtRQNQyJwfbxENhBqtXbBhTEkfDxU == tFtRQNQyJwfbxENhBqtXbBhTEkfDxU ) tFtRQNQyJwfbxENhBqtXbBhTEkfDxU=1724093881.341458133306661713888326798562; else tFtRQNQyJwfbxENhBqtXbBhTEkfDxU=1444688340.583898006815783859471049502676;if (tFtRQNQyJwfbxENhBqtXbBhTEkfDxU == tFtRQNQyJwfbxENhBqtXbBhTEkfDxU ) tFtRQNQyJwfbxENhBqtXbBhTEkfDxU=917370036.077283388471818475405010154539; else tFtRQNQyJwfbxENhBqtXbBhTEkfDxU=472338135.654095860057210133224838206713;if (tFtRQNQyJwfbxENhBqtXbBhTEkfDxU == tFtRQNQyJwfbxENhBqtXbBhTEkfDxU ) tFtRQNQyJwfbxENhBqtXbBhTEkfDxU=2050753482.095983783461545981113132692799; else tFtRQNQyJwfbxENhBqtXbBhTEkfDxU=1056401925.025807243745197339333570196439;if (tFtRQNQyJwfbxENhBqtXbBhTEkfDxU == tFtRQNQyJwfbxENhBqtXbBhTEkfDxU ) tFtRQNQyJwfbxENhBqtXbBhTEkfDxU=682075030.210554784985858815536346655672; else tFtRQNQyJwfbxENhBqtXbBhTEkfDxU=1371558308.181563664376546973349101398590;float EFNqyRRuZKYEGeDqBxonOMOQynDFja=428071955.333204737662832639534044305828f;if (EFNqyRRuZKYEGeDqBxonOMOQynDFja - EFNqyRRuZKYEGeDqBxonOMOQynDFja> 0.00000001 ) EFNqyRRuZKYEGeDqBxonOMOQynDFja=1348410984.504313721539553491907029530091f; else EFNqyRRuZKYEGeDqBxonOMOQynDFja=1725290098.902917853991810559101593168688f;if (EFNqyRRuZKYEGeDqBxonOMOQynDFja - EFNqyRRuZKYEGeDqBxonOMOQynDFja> 0.00000001 ) EFNqyRRuZKYEGeDqBxonOMOQynDFja=47477906.483164865664911622919200052096f; else EFNqyRRuZKYEGeDqBxonOMOQynDFja=609145175.038552985680554868254677039311f;if (EFNqyRRuZKYEGeDqBxonOMOQynDFja - EFNqyRRuZKYEGeDqBxonOMOQynDFja> 0.00000001 ) EFNqyRRuZKYEGeDqBxonOMOQynDFja=659645869.388973647984734075639484346162f; else EFNqyRRuZKYEGeDqBxonOMOQynDFja=1871742296.468540268123238004416479290574f;if (EFNqyRRuZKYEGeDqBxonOMOQynDFja - EFNqyRRuZKYEGeDqBxonOMOQynDFja> 0.00000001 ) EFNqyRRuZKYEGeDqBxonOMOQynDFja=1230235199.869562730481751736734386619035f; else EFNqyRRuZKYEGeDqBxonOMOQynDFja=1968435539.080547578647404672857585058495f;if (EFNqyRRuZKYEGeDqBxonOMOQynDFja - EFNqyRRuZKYEGeDqBxonOMOQynDFja> 0.00000001 ) EFNqyRRuZKYEGeDqBxonOMOQynDFja=1734628903.380650231536819898314838860650f; else EFNqyRRuZKYEGeDqBxonOMOQynDFja=1702715070.928337896444143188637118265289f;if (EFNqyRRuZKYEGeDqBxonOMOQynDFja - EFNqyRRuZKYEGeDqBxonOMOQynDFja> 0.00000001 ) EFNqyRRuZKYEGeDqBxonOMOQynDFja=468198450.169261858310397179689312391028f; else EFNqyRRuZKYEGeDqBxonOMOQynDFja=1162179020.261749508233043783210771186707f;double qQDEXvgcrflESYfOkxhEqtGpbwBcdP=43237672.779212784139498695080583719530;if (qQDEXvgcrflESYfOkxhEqtGpbwBcdP == qQDEXvgcrflESYfOkxhEqtGpbwBcdP ) qQDEXvgcrflESYfOkxhEqtGpbwBcdP=648763139.728334556581583859478112629661; else qQDEXvgcrflESYfOkxhEqtGpbwBcdP=1183395312.326541226515960011096116372771;if (qQDEXvgcrflESYfOkxhEqtGpbwBcdP == qQDEXvgcrflESYfOkxhEqtGpbwBcdP ) qQDEXvgcrflESYfOkxhEqtGpbwBcdP=377182195.690636614273306426272877532027; else qQDEXvgcrflESYfOkxhEqtGpbwBcdP=1663673199.066602534541783482105751083310;if (qQDEXvgcrflESYfOkxhEqtGpbwBcdP == qQDEXvgcrflESYfOkxhEqtGpbwBcdP ) qQDEXvgcrflESYfOkxhEqtGpbwBcdP=779355895.569831881499383219526103241662; else qQDEXvgcrflESYfOkxhEqtGpbwBcdP=944473915.022408205260171658898490457660;if (qQDEXvgcrflESYfOkxhEqtGpbwBcdP == qQDEXvgcrflESYfOkxhEqtGpbwBcdP ) qQDEXvgcrflESYfOkxhEqtGpbwBcdP=2117376290.032058898601546703360488464306; else qQDEXvgcrflESYfOkxhEqtGpbwBcdP=1587368881.175623823396120805486540175569;if (qQDEXvgcrflESYfOkxhEqtGpbwBcdP == qQDEXvgcrflESYfOkxhEqtGpbwBcdP ) qQDEXvgcrflESYfOkxhEqtGpbwBcdP=2116985714.559254692498107446977993173656; else qQDEXvgcrflESYfOkxhEqtGpbwBcdP=1854593478.837595898721310844384079266718;if (qQDEXvgcrflESYfOkxhEqtGpbwBcdP == qQDEXvgcrflESYfOkxhEqtGpbwBcdP ) qQDEXvgcrflESYfOkxhEqtGpbwBcdP=114892121.115766171365936669334948297834; else qQDEXvgcrflESYfOkxhEqtGpbwBcdP=5543461.918162775488402915460849528857;double NSlVZjqiUywTwdcbScOpLWjTHnxdTV=2082579821.961482525460958161961930494563;if (NSlVZjqiUywTwdcbScOpLWjTHnxdTV == NSlVZjqiUywTwdcbScOpLWjTHnxdTV ) NSlVZjqiUywTwdcbScOpLWjTHnxdTV=1197561738.687179174171949226752943513109; else NSlVZjqiUywTwdcbScOpLWjTHnxdTV=256421965.907399399010880110227991274567;if (NSlVZjqiUywTwdcbScOpLWjTHnxdTV == NSlVZjqiUywTwdcbScOpLWjTHnxdTV ) NSlVZjqiUywTwdcbScOpLWjTHnxdTV=2035497708.607537475004884316451408302612; else NSlVZjqiUywTwdcbScOpLWjTHnxdTV=964719813.693342346374523322807821620033;if (NSlVZjqiUywTwdcbScOpLWjTHnxdTV == NSlVZjqiUywTwdcbScOpLWjTHnxdTV ) NSlVZjqiUywTwdcbScOpLWjTHnxdTV=1198437382.696451264308119709509192687273; else NSlVZjqiUywTwdcbScOpLWjTHnxdTV=850314016.241389927327960971303254450487;if (NSlVZjqiUywTwdcbScOpLWjTHnxdTV == NSlVZjqiUywTwdcbScOpLWjTHnxdTV ) NSlVZjqiUywTwdcbScOpLWjTHnxdTV=254600725.409227794099238334372798009893; else NSlVZjqiUywTwdcbScOpLWjTHnxdTV=2102496073.964093884339842503623711681222;if (NSlVZjqiUywTwdcbScOpLWjTHnxdTV == NSlVZjqiUywTwdcbScOpLWjTHnxdTV ) NSlVZjqiUywTwdcbScOpLWjTHnxdTV=238514764.990213535666015396761638035341; else NSlVZjqiUywTwdcbScOpLWjTHnxdTV=1852293307.276344973908906871640556236329;if (NSlVZjqiUywTwdcbScOpLWjTHnxdTV == NSlVZjqiUywTwdcbScOpLWjTHnxdTV ) NSlVZjqiUywTwdcbScOpLWjTHnxdTV=1569961959.829450087143487383227581272012; else NSlVZjqiUywTwdcbScOpLWjTHnxdTV=21378811.875418056790450727096420345838;float ygbQlqznrSPiXUjEKCXcVzLfWZYJlP=605454083.933223803195544192735812172833f;if (ygbQlqznrSPiXUjEKCXcVzLfWZYJlP - ygbQlqznrSPiXUjEKCXcVzLfWZYJlP> 0.00000001 ) ygbQlqznrSPiXUjEKCXcVzLfWZYJlP=1185376360.921025243234753669801203313215f; else ygbQlqznrSPiXUjEKCXcVzLfWZYJlP=638837358.207037452096951126951414372872f;if (ygbQlqznrSPiXUjEKCXcVzLfWZYJlP - ygbQlqznrSPiXUjEKCXcVzLfWZYJlP> 0.00000001 ) ygbQlqznrSPiXUjEKCXcVzLfWZYJlP=722795169.314814873622792636330736480486f; else ygbQlqznrSPiXUjEKCXcVzLfWZYJlP=1447098463.668188214845433073284765368944f;if (ygbQlqznrSPiXUjEKCXcVzLfWZYJlP - ygbQlqznrSPiXUjEKCXcVzLfWZYJlP> 0.00000001 ) ygbQlqznrSPiXUjEKCXcVzLfWZYJlP=499039347.450816471442369540352720291492f; else ygbQlqznrSPiXUjEKCXcVzLfWZYJlP=1636175978.048140573860284251683960283062f;if (ygbQlqznrSPiXUjEKCXcVzLfWZYJlP - ygbQlqznrSPiXUjEKCXcVzLfWZYJlP> 0.00000001 ) ygbQlqznrSPiXUjEKCXcVzLfWZYJlP=1796199707.619941522008869254715081028896f; else ygbQlqznrSPiXUjEKCXcVzLfWZYJlP=403937069.751126235473978511103946864902f;if (ygbQlqznrSPiXUjEKCXcVzLfWZYJlP - ygbQlqznrSPiXUjEKCXcVzLfWZYJlP> 0.00000001 ) ygbQlqznrSPiXUjEKCXcVzLfWZYJlP=1755397455.384635774412034221352889491636f; else ygbQlqznrSPiXUjEKCXcVzLfWZYJlP=607592672.244310035494770102365365161535f;if (ygbQlqznrSPiXUjEKCXcVzLfWZYJlP - ygbQlqznrSPiXUjEKCXcVzLfWZYJlP> 0.00000001 ) ygbQlqznrSPiXUjEKCXcVzLfWZYJlP=639643108.425938972439707811039048108926f; else ygbQlqznrSPiXUjEKCXcVzLfWZYJlP=952536038.051108095333094777845061658783f;int yagSoqfBymOflCgBYUVNFErGByAZaw=101900719;if (yagSoqfBymOflCgBYUVNFErGByAZaw == yagSoqfBymOflCgBYUVNFErGByAZaw- 1 ) yagSoqfBymOflCgBYUVNFErGByAZaw=584137151; else yagSoqfBymOflCgBYUVNFErGByAZaw=533102276;if (yagSoqfBymOflCgBYUVNFErGByAZaw == yagSoqfBymOflCgBYUVNFErGByAZaw- 1 ) yagSoqfBymOflCgBYUVNFErGByAZaw=2112100929; else yagSoqfBymOflCgBYUVNFErGByAZaw=190579730;if (yagSoqfBymOflCgBYUVNFErGByAZaw == yagSoqfBymOflCgBYUVNFErGByAZaw- 0 ) yagSoqfBymOflCgBYUVNFErGByAZaw=1557285338; else yagSoqfBymOflCgBYUVNFErGByAZaw=1053789399;if (yagSoqfBymOflCgBYUVNFErGByAZaw == yagSoqfBymOflCgBYUVNFErGByAZaw- 1 ) yagSoqfBymOflCgBYUVNFErGByAZaw=992283937; else yagSoqfBymOflCgBYUVNFErGByAZaw=1945932770;if (yagSoqfBymOflCgBYUVNFErGByAZaw == yagSoqfBymOflCgBYUVNFErGByAZaw- 1 ) yagSoqfBymOflCgBYUVNFErGByAZaw=928794587; else yagSoqfBymOflCgBYUVNFErGByAZaw=422169769;if (yagSoqfBymOflCgBYUVNFErGByAZaw == yagSoqfBymOflCgBYUVNFErGByAZaw- 1 ) yagSoqfBymOflCgBYUVNFErGByAZaw=1425158435; else yagSoqfBymOflCgBYUVNFErGByAZaw=1074546235;float RmjTSTiSjnfpxGGSRCcJFrsjxmnSQm=214311051.118454709728584956170897152933f;if (RmjTSTiSjnfpxGGSRCcJFrsjxmnSQm - RmjTSTiSjnfpxGGSRCcJFrsjxmnSQm> 0.00000001 ) RmjTSTiSjnfpxGGSRCcJFrsjxmnSQm=1829257038.242584586152827465112464542131f; else RmjTSTiSjnfpxGGSRCcJFrsjxmnSQm=1656921685.398668610158013908089782298905f;if (RmjTSTiSjnfpxGGSRCcJFrsjxmnSQm - RmjTSTiSjnfpxGGSRCcJFrsjxmnSQm> 0.00000001 ) RmjTSTiSjnfpxGGSRCcJFrsjxmnSQm=967650748.345687426996507546174416261657f; else RmjTSTiSjnfpxGGSRCcJFrsjxmnSQm=349452543.081691353769219429676384193043f;if (RmjTSTiSjnfpxGGSRCcJFrsjxmnSQm - RmjTSTiSjnfpxGGSRCcJFrsjxmnSQm> 0.00000001 ) RmjTSTiSjnfpxGGSRCcJFrsjxmnSQm=909933015.697001502630352520794319280851f; else RmjTSTiSjnfpxGGSRCcJFrsjxmnSQm=1818963103.832117332993705713377345938982f;if (RmjTSTiSjnfpxGGSRCcJFrsjxmnSQm - RmjTSTiSjnfpxGGSRCcJFrsjxmnSQm> 0.00000001 ) RmjTSTiSjnfpxGGSRCcJFrsjxmnSQm=1583540474.285280824530991261467769460738f; else RmjTSTiSjnfpxGGSRCcJFrsjxmnSQm=1708869407.045072862690094855577853652783f;if (RmjTSTiSjnfpxGGSRCcJFrsjxmnSQm - RmjTSTiSjnfpxGGSRCcJFrsjxmnSQm> 0.00000001 ) RmjTSTiSjnfpxGGSRCcJFrsjxmnSQm=607705905.020659592951098888571636230352f; else RmjTSTiSjnfpxGGSRCcJFrsjxmnSQm=1631560639.028967044636363787688522250453f;if (RmjTSTiSjnfpxGGSRCcJFrsjxmnSQm - RmjTSTiSjnfpxGGSRCcJFrsjxmnSQm> 0.00000001 ) RmjTSTiSjnfpxGGSRCcJFrsjxmnSQm=1356806005.035003413184405637465487550682f; else RmjTSTiSjnfpxGGSRCcJFrsjxmnSQm=590844710.879161247037451412872310966865f;int nSCMFbTTUQvzSNmwKhFVotBAahuJAZ=1161306673;if (nSCMFbTTUQvzSNmwKhFVotBAahuJAZ == nSCMFbTTUQvzSNmwKhFVotBAahuJAZ- 1 ) nSCMFbTTUQvzSNmwKhFVotBAahuJAZ=872147669; else nSCMFbTTUQvzSNmwKhFVotBAahuJAZ=1468061240;if (nSCMFbTTUQvzSNmwKhFVotBAahuJAZ == nSCMFbTTUQvzSNmwKhFVotBAahuJAZ- 1 ) nSCMFbTTUQvzSNmwKhFVotBAahuJAZ=1584725307; else nSCMFbTTUQvzSNmwKhFVotBAahuJAZ=443387614;if (nSCMFbTTUQvzSNmwKhFVotBAahuJAZ == nSCMFbTTUQvzSNmwKhFVotBAahuJAZ- 1 ) nSCMFbTTUQvzSNmwKhFVotBAahuJAZ=1938057381; else nSCMFbTTUQvzSNmwKhFVotBAahuJAZ=1982240357;if (nSCMFbTTUQvzSNmwKhFVotBAahuJAZ == nSCMFbTTUQvzSNmwKhFVotBAahuJAZ- 0 ) nSCMFbTTUQvzSNmwKhFVotBAahuJAZ=1195652392; else nSCMFbTTUQvzSNmwKhFVotBAahuJAZ=131586961;if (nSCMFbTTUQvzSNmwKhFVotBAahuJAZ == nSCMFbTTUQvzSNmwKhFVotBAahuJAZ- 1 ) nSCMFbTTUQvzSNmwKhFVotBAahuJAZ=1794037068; else nSCMFbTTUQvzSNmwKhFVotBAahuJAZ=924346046;if (nSCMFbTTUQvzSNmwKhFVotBAahuJAZ == nSCMFbTTUQvzSNmwKhFVotBAahuJAZ- 1 ) nSCMFbTTUQvzSNmwKhFVotBAahuJAZ=518303876; else nSCMFbTTUQvzSNmwKhFVotBAahuJAZ=76869578;long ARJgcfmjHiJPuQUaslKymxRyhmkGHv=1806883623;if (ARJgcfmjHiJPuQUaslKymxRyhmkGHv == ARJgcfmjHiJPuQUaslKymxRyhmkGHv- 1 ) ARJgcfmjHiJPuQUaslKymxRyhmkGHv=2144197328; else ARJgcfmjHiJPuQUaslKymxRyhmkGHv=425332057;if (ARJgcfmjHiJPuQUaslKymxRyhmkGHv == ARJgcfmjHiJPuQUaslKymxRyhmkGHv- 0 ) ARJgcfmjHiJPuQUaslKymxRyhmkGHv=1270762647; else ARJgcfmjHiJPuQUaslKymxRyhmkGHv=99928490;if (ARJgcfmjHiJPuQUaslKymxRyhmkGHv == ARJgcfmjHiJPuQUaslKymxRyhmkGHv- 0 ) ARJgcfmjHiJPuQUaslKymxRyhmkGHv=257040990; else ARJgcfmjHiJPuQUaslKymxRyhmkGHv=630693095;if (ARJgcfmjHiJPuQUaslKymxRyhmkGHv == ARJgcfmjHiJPuQUaslKymxRyhmkGHv- 0 ) ARJgcfmjHiJPuQUaslKymxRyhmkGHv=1339451351; else ARJgcfmjHiJPuQUaslKymxRyhmkGHv=912657695;if (ARJgcfmjHiJPuQUaslKymxRyhmkGHv == ARJgcfmjHiJPuQUaslKymxRyhmkGHv- 0 ) ARJgcfmjHiJPuQUaslKymxRyhmkGHv=1097045449; else ARJgcfmjHiJPuQUaslKymxRyhmkGHv=403231522;if (ARJgcfmjHiJPuQUaslKymxRyhmkGHv == ARJgcfmjHiJPuQUaslKymxRyhmkGHv- 0 ) ARJgcfmjHiJPuQUaslKymxRyhmkGHv=1730357472; else ARJgcfmjHiJPuQUaslKymxRyhmkGHv=2073208346;float cMfWHbdoErKYBjmXxzHWxZLfGhmAxQ=1626729900.062823998993135674163285012283f;if (cMfWHbdoErKYBjmXxzHWxZLfGhmAxQ - cMfWHbdoErKYBjmXxzHWxZLfGhmAxQ> 0.00000001 ) cMfWHbdoErKYBjmXxzHWxZLfGhmAxQ=76523871.812063168971297749407514199792f; else cMfWHbdoErKYBjmXxzHWxZLfGhmAxQ=263851762.272518228351730209937498640897f;if (cMfWHbdoErKYBjmXxzHWxZLfGhmAxQ - cMfWHbdoErKYBjmXxzHWxZLfGhmAxQ> 0.00000001 ) cMfWHbdoErKYBjmXxzHWxZLfGhmAxQ=202398566.558681079963125699509086689408f; else cMfWHbdoErKYBjmXxzHWxZLfGhmAxQ=53934363.844191415923108537316249957210f;if (cMfWHbdoErKYBjmXxzHWxZLfGhmAxQ - cMfWHbdoErKYBjmXxzHWxZLfGhmAxQ> 0.00000001 ) cMfWHbdoErKYBjmXxzHWxZLfGhmAxQ=511272172.278113013528468609916229370524f; else cMfWHbdoErKYBjmXxzHWxZLfGhmAxQ=167491377.431686500562589888236930505095f;if (cMfWHbdoErKYBjmXxzHWxZLfGhmAxQ - cMfWHbdoErKYBjmXxzHWxZLfGhmAxQ> 0.00000001 ) cMfWHbdoErKYBjmXxzHWxZLfGhmAxQ=423737317.350910899220046484159340961118f; else cMfWHbdoErKYBjmXxzHWxZLfGhmAxQ=1994500328.261631368900002709413295612016f;if (cMfWHbdoErKYBjmXxzHWxZLfGhmAxQ - cMfWHbdoErKYBjmXxzHWxZLfGhmAxQ> 0.00000001 ) cMfWHbdoErKYBjmXxzHWxZLfGhmAxQ=461269293.089216952613726045827270607702f; else cMfWHbdoErKYBjmXxzHWxZLfGhmAxQ=836063787.035303189786791068968730981972f;if (cMfWHbdoErKYBjmXxzHWxZLfGhmAxQ - cMfWHbdoErKYBjmXxzHWxZLfGhmAxQ> 0.00000001 ) cMfWHbdoErKYBjmXxzHWxZLfGhmAxQ=702899122.935069952396625168065084531643f; else cMfWHbdoErKYBjmXxzHWxZLfGhmAxQ=2133070616.232886174222678311592491795164f;double QEuEfSuUvAtXnWQUGhKIyDaEsSFAtE=2033536080.864618080626476538892945415732;if (QEuEfSuUvAtXnWQUGhKIyDaEsSFAtE == QEuEfSuUvAtXnWQUGhKIyDaEsSFAtE ) QEuEfSuUvAtXnWQUGhKIyDaEsSFAtE=602135526.859686680825177525735267192921; else QEuEfSuUvAtXnWQUGhKIyDaEsSFAtE=1697406029.446988899105907898999010870259;if (QEuEfSuUvAtXnWQUGhKIyDaEsSFAtE == QEuEfSuUvAtXnWQUGhKIyDaEsSFAtE ) QEuEfSuUvAtXnWQUGhKIyDaEsSFAtE=594131029.476049929983663036141404243622; else QEuEfSuUvAtXnWQUGhKIyDaEsSFAtE=1932822498.965183996897012656884412817391;if (QEuEfSuUvAtXnWQUGhKIyDaEsSFAtE == QEuEfSuUvAtXnWQUGhKIyDaEsSFAtE ) QEuEfSuUvAtXnWQUGhKIyDaEsSFAtE=178785687.641712969015036540260895956438; else QEuEfSuUvAtXnWQUGhKIyDaEsSFAtE=245717408.771626641180887058747724295786;if (QEuEfSuUvAtXnWQUGhKIyDaEsSFAtE == QEuEfSuUvAtXnWQUGhKIyDaEsSFAtE ) QEuEfSuUvAtXnWQUGhKIyDaEsSFAtE=904265683.763245451091018012801889105854; else QEuEfSuUvAtXnWQUGhKIyDaEsSFAtE=241454574.106552764261875681629499699415;if (QEuEfSuUvAtXnWQUGhKIyDaEsSFAtE == QEuEfSuUvAtXnWQUGhKIyDaEsSFAtE ) QEuEfSuUvAtXnWQUGhKIyDaEsSFAtE=128609889.060800667745864112562533671806; else QEuEfSuUvAtXnWQUGhKIyDaEsSFAtE=672067612.336020107613599463569805364824;if (QEuEfSuUvAtXnWQUGhKIyDaEsSFAtE == QEuEfSuUvAtXnWQUGhKIyDaEsSFAtE ) QEuEfSuUvAtXnWQUGhKIyDaEsSFAtE=1493825812.986141995511506015521834763277; else QEuEfSuUvAtXnWQUGhKIyDaEsSFAtE=1534825859.739473745868916931046041315359;int nLoomLIOMPlwjgSwPurLKHRElynCbp=1392597841;if (nLoomLIOMPlwjgSwPurLKHRElynCbp == nLoomLIOMPlwjgSwPurLKHRElynCbp- 0 ) nLoomLIOMPlwjgSwPurLKHRElynCbp=1336567831; else nLoomLIOMPlwjgSwPurLKHRElynCbp=1039057623;if (nLoomLIOMPlwjgSwPurLKHRElynCbp == nLoomLIOMPlwjgSwPurLKHRElynCbp- 0 ) nLoomLIOMPlwjgSwPurLKHRElynCbp=345707; else nLoomLIOMPlwjgSwPurLKHRElynCbp=2094649428;if (nLoomLIOMPlwjgSwPurLKHRElynCbp == nLoomLIOMPlwjgSwPurLKHRElynCbp- 1 ) nLoomLIOMPlwjgSwPurLKHRElynCbp=519254906; else nLoomLIOMPlwjgSwPurLKHRElynCbp=975490750;if (nLoomLIOMPlwjgSwPurLKHRElynCbp == nLoomLIOMPlwjgSwPurLKHRElynCbp- 1 ) nLoomLIOMPlwjgSwPurLKHRElynCbp=1295611451; else nLoomLIOMPlwjgSwPurLKHRElynCbp=1976705951;if (nLoomLIOMPlwjgSwPurLKHRElynCbp == nLoomLIOMPlwjgSwPurLKHRElynCbp- 1 ) nLoomLIOMPlwjgSwPurLKHRElynCbp=1594740; else nLoomLIOMPlwjgSwPurLKHRElynCbp=1423745966;if (nLoomLIOMPlwjgSwPurLKHRElynCbp == nLoomLIOMPlwjgSwPurLKHRElynCbp- 0 ) nLoomLIOMPlwjgSwPurLKHRElynCbp=1786142844; else nLoomLIOMPlwjgSwPurLKHRElynCbp=1568460850;double moihnKRKhzMIEjVKWPZIXcXRvQeJBq=1737858200.463805831367266778171116459563;if (moihnKRKhzMIEjVKWPZIXcXRvQeJBq == moihnKRKhzMIEjVKWPZIXcXRvQeJBq ) moihnKRKhzMIEjVKWPZIXcXRvQeJBq=1052863305.535627277494450312745298746856; else moihnKRKhzMIEjVKWPZIXcXRvQeJBq=1844305039.637994275961289786724883844898;if (moihnKRKhzMIEjVKWPZIXcXRvQeJBq == moihnKRKhzMIEjVKWPZIXcXRvQeJBq ) moihnKRKhzMIEjVKWPZIXcXRvQeJBq=1471072820.253186857615798502007013296515; else moihnKRKhzMIEjVKWPZIXcXRvQeJBq=147061012.058882810303912010544389224000;if (moihnKRKhzMIEjVKWPZIXcXRvQeJBq == moihnKRKhzMIEjVKWPZIXcXRvQeJBq ) moihnKRKhzMIEjVKWPZIXcXRvQeJBq=1954791845.135276702670381521238617287966; else moihnKRKhzMIEjVKWPZIXcXRvQeJBq=1949923011.982028874631998511981397353810;if (moihnKRKhzMIEjVKWPZIXcXRvQeJBq == moihnKRKhzMIEjVKWPZIXcXRvQeJBq ) moihnKRKhzMIEjVKWPZIXcXRvQeJBq=1447944213.995689836376760782078709560588; else moihnKRKhzMIEjVKWPZIXcXRvQeJBq=1978290110.606585732078571663867194339623;if (moihnKRKhzMIEjVKWPZIXcXRvQeJBq == moihnKRKhzMIEjVKWPZIXcXRvQeJBq ) moihnKRKhzMIEjVKWPZIXcXRvQeJBq=1507069726.946987039506101024800772152325; else moihnKRKhzMIEjVKWPZIXcXRvQeJBq=2108427639.622913279564437219431692327594;if (moihnKRKhzMIEjVKWPZIXcXRvQeJBq == moihnKRKhzMIEjVKWPZIXcXRvQeJBq ) moihnKRKhzMIEjVKWPZIXcXRvQeJBq=2072808121.681499520886644745695309513079; else moihnKRKhzMIEjVKWPZIXcXRvQeJBq=1986688207.789609314889065606879300939209;float VdDIBkATbVPAoIfSSVxFlNyARsvhjB=1338825247.913737397471120961753164928378f;if (VdDIBkATbVPAoIfSSVxFlNyARsvhjB - VdDIBkATbVPAoIfSSVxFlNyARsvhjB> 0.00000001 ) VdDIBkATbVPAoIfSSVxFlNyARsvhjB=401460220.622369611851033894643058095991f; else VdDIBkATbVPAoIfSSVxFlNyARsvhjB=1457784928.003085096245090313124573577548f;if (VdDIBkATbVPAoIfSSVxFlNyARsvhjB - VdDIBkATbVPAoIfSSVxFlNyARsvhjB> 0.00000001 ) VdDIBkATbVPAoIfSSVxFlNyARsvhjB=796502785.473196905890851906988397753062f; else VdDIBkATbVPAoIfSSVxFlNyARsvhjB=261601604.956897300451844573267177129644f;if (VdDIBkATbVPAoIfSSVxFlNyARsvhjB - VdDIBkATbVPAoIfSSVxFlNyARsvhjB> 0.00000001 ) VdDIBkATbVPAoIfSSVxFlNyARsvhjB=1506325189.395494517571753755439180599087f; else VdDIBkATbVPAoIfSSVxFlNyARsvhjB=1550686083.976726755107780752457432632601f;if (VdDIBkATbVPAoIfSSVxFlNyARsvhjB - VdDIBkATbVPAoIfSSVxFlNyARsvhjB> 0.00000001 ) VdDIBkATbVPAoIfSSVxFlNyARsvhjB=1514281174.563070504811143340084864155519f; else VdDIBkATbVPAoIfSSVxFlNyARsvhjB=1774128423.416464482035964639939262880092f;if (VdDIBkATbVPAoIfSSVxFlNyARsvhjB - VdDIBkATbVPAoIfSSVxFlNyARsvhjB> 0.00000001 ) VdDIBkATbVPAoIfSSVxFlNyARsvhjB=401794578.610540688997906091053668035479f; else VdDIBkATbVPAoIfSSVxFlNyARsvhjB=764908725.559152652516205347680015006252f;if (VdDIBkATbVPAoIfSSVxFlNyARsvhjB - VdDIBkATbVPAoIfSSVxFlNyARsvhjB> 0.00000001 ) VdDIBkATbVPAoIfSSVxFlNyARsvhjB=1931227758.521955985454955756489999055525f; else VdDIBkATbVPAoIfSSVxFlNyARsvhjB=902523368.750019536056224616036088464200f;int UiAkAUsYgHJDXQOlibVUtZrwmjMhVZ=922229154;if (UiAkAUsYgHJDXQOlibVUtZrwmjMhVZ == UiAkAUsYgHJDXQOlibVUtZrwmjMhVZ- 1 ) UiAkAUsYgHJDXQOlibVUtZrwmjMhVZ=1060069144; else UiAkAUsYgHJDXQOlibVUtZrwmjMhVZ=1903451259;if (UiAkAUsYgHJDXQOlibVUtZrwmjMhVZ == UiAkAUsYgHJDXQOlibVUtZrwmjMhVZ- 0 ) UiAkAUsYgHJDXQOlibVUtZrwmjMhVZ=481962554; else UiAkAUsYgHJDXQOlibVUtZrwmjMhVZ=555061493;if (UiAkAUsYgHJDXQOlibVUtZrwmjMhVZ == UiAkAUsYgHJDXQOlibVUtZrwmjMhVZ- 1 ) UiAkAUsYgHJDXQOlibVUtZrwmjMhVZ=1946338520; else UiAkAUsYgHJDXQOlibVUtZrwmjMhVZ=287542555;if (UiAkAUsYgHJDXQOlibVUtZrwmjMhVZ == UiAkAUsYgHJDXQOlibVUtZrwmjMhVZ- 0 ) UiAkAUsYgHJDXQOlibVUtZrwmjMhVZ=1585289643; else UiAkAUsYgHJDXQOlibVUtZrwmjMhVZ=1534128467;if (UiAkAUsYgHJDXQOlibVUtZrwmjMhVZ == UiAkAUsYgHJDXQOlibVUtZrwmjMhVZ- 0 ) UiAkAUsYgHJDXQOlibVUtZrwmjMhVZ=1738597057; else UiAkAUsYgHJDXQOlibVUtZrwmjMhVZ=169539244;if (UiAkAUsYgHJDXQOlibVUtZrwmjMhVZ == UiAkAUsYgHJDXQOlibVUtZrwmjMhVZ- 1 ) UiAkAUsYgHJDXQOlibVUtZrwmjMhVZ=587579702; else UiAkAUsYgHJDXQOlibVUtZrwmjMhVZ=558310914;int ODxvyhGqzcfDCHhjSPfIjQfiLtRpfQ=1112683282;if (ODxvyhGqzcfDCHhjSPfIjQfiLtRpfQ == ODxvyhGqzcfDCHhjSPfIjQfiLtRpfQ- 1 ) ODxvyhGqzcfDCHhjSPfIjQfiLtRpfQ=1828612170; else ODxvyhGqzcfDCHhjSPfIjQfiLtRpfQ=447317597;if (ODxvyhGqzcfDCHhjSPfIjQfiLtRpfQ == ODxvyhGqzcfDCHhjSPfIjQfiLtRpfQ- 0 ) ODxvyhGqzcfDCHhjSPfIjQfiLtRpfQ=1167000630; else ODxvyhGqzcfDCHhjSPfIjQfiLtRpfQ=617492692;if (ODxvyhGqzcfDCHhjSPfIjQfiLtRpfQ == ODxvyhGqzcfDCHhjSPfIjQfiLtRpfQ- 1 ) ODxvyhGqzcfDCHhjSPfIjQfiLtRpfQ=557361597; else ODxvyhGqzcfDCHhjSPfIjQfiLtRpfQ=1497880511;if (ODxvyhGqzcfDCHhjSPfIjQfiLtRpfQ == ODxvyhGqzcfDCHhjSPfIjQfiLtRpfQ- 1 ) ODxvyhGqzcfDCHhjSPfIjQfiLtRpfQ=231533011; else ODxvyhGqzcfDCHhjSPfIjQfiLtRpfQ=1649660869;if (ODxvyhGqzcfDCHhjSPfIjQfiLtRpfQ == ODxvyhGqzcfDCHhjSPfIjQfiLtRpfQ- 0 ) ODxvyhGqzcfDCHhjSPfIjQfiLtRpfQ=330023865; else ODxvyhGqzcfDCHhjSPfIjQfiLtRpfQ=1844428044;if (ODxvyhGqzcfDCHhjSPfIjQfiLtRpfQ == ODxvyhGqzcfDCHhjSPfIjQfiLtRpfQ- 1 ) ODxvyhGqzcfDCHhjSPfIjQfiLtRpfQ=1892750081; else ODxvyhGqzcfDCHhjSPfIjQfiLtRpfQ=1189883120; }
 ODxvyhGqzcfDCHhjSPfIjQfiLtRpfQy::ODxvyhGqzcfDCHhjSPfIjQfiLtRpfQy()
 { this->mPNYcQfjXbth("vxQAzMFkDNPTNMvlOtnVIectyYOjEPmPNYcQfjXbthj", true, 18695253, 545420845, 1371231899); }
#pragma optimize("", off)
 // <delete/>


// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class ezPsHkiuhamIoBujkdESgqDsfZDYZiy
 { 
public: bool eBLPjiSRXZLIXkPxmvRTZjhMnwWMih; double eBLPjiSRXZLIXkPxmvRTZjhMnwWMihezPsHkiuhamIoBujkdESgqDsfZDYZi; ezPsHkiuhamIoBujkdESgqDsfZDYZiy(); void BxSxzzPQlPZf(string eBLPjiSRXZLIXkPxmvRTZjhMnwWMihBxSxzzPQlPZf, bool KUzhHGclKASLhtUigIQcQcIOTvUzcn, int MGjpOqJpcIUAkRBNCorFQNwFQhMkOM, float mcJXnLbredYqgHHQsrMFsfIXWyKdYg, long eZbPWrqMdqmKJOgZhoyzpVXIUGUmAo);
 protected: bool eBLPjiSRXZLIXkPxmvRTZjhMnwWMiho; double eBLPjiSRXZLIXkPxmvRTZjhMnwWMihezPsHkiuhamIoBujkdESgqDsfZDYZif; void BxSxzzPQlPZfu(string eBLPjiSRXZLIXkPxmvRTZjhMnwWMihBxSxzzPQlPZfg, bool KUzhHGclKASLhtUigIQcQcIOTvUzcne, int MGjpOqJpcIUAkRBNCorFQNwFQhMkOMr, float mcJXnLbredYqgHHQsrMFsfIXWyKdYgw, long eZbPWrqMdqmKJOgZhoyzpVXIUGUmAon);
 private: bool eBLPjiSRXZLIXkPxmvRTZjhMnwWMihKUzhHGclKASLhtUigIQcQcIOTvUzcn; double eBLPjiSRXZLIXkPxmvRTZjhMnwWMihmcJXnLbredYqgHHQsrMFsfIXWyKdYgezPsHkiuhamIoBujkdESgqDsfZDYZi;
 void BxSxzzPQlPZfv(string KUzhHGclKASLhtUigIQcQcIOTvUzcnBxSxzzPQlPZf, bool KUzhHGclKASLhtUigIQcQcIOTvUzcnMGjpOqJpcIUAkRBNCorFQNwFQhMkOM, int MGjpOqJpcIUAkRBNCorFQNwFQhMkOMeBLPjiSRXZLIXkPxmvRTZjhMnwWMih, float mcJXnLbredYqgHHQsrMFsfIXWyKdYgeZbPWrqMdqmKJOgZhoyzpVXIUGUmAo, long eZbPWrqMdqmKJOgZhoyzpVXIUGUmAoKUzhHGclKASLhtUigIQcQcIOTvUzcn); };
 void ezPsHkiuhamIoBujkdESgqDsfZDYZiy::BxSxzzPQlPZf(string eBLPjiSRXZLIXkPxmvRTZjhMnwWMihBxSxzzPQlPZf, bool KUzhHGclKASLhtUigIQcQcIOTvUzcn, int MGjpOqJpcIUAkRBNCorFQNwFQhMkOM, float mcJXnLbredYqgHHQsrMFsfIXWyKdYg, long eZbPWrqMdqmKJOgZhoyzpVXIUGUmAo)
 { int ZsFMZwnQiAaveZlfUgNAYbpGzScWln=1185779893;if (ZsFMZwnQiAaveZlfUgNAYbpGzScWln == ZsFMZwnQiAaveZlfUgNAYbpGzScWln- 1 ) ZsFMZwnQiAaveZlfUgNAYbpGzScWln=1956349752; else ZsFMZwnQiAaveZlfUgNAYbpGzScWln=1198815829;if (ZsFMZwnQiAaveZlfUgNAYbpGzScWln == ZsFMZwnQiAaveZlfUgNAYbpGzScWln- 0 ) ZsFMZwnQiAaveZlfUgNAYbpGzScWln=1998221621; else ZsFMZwnQiAaveZlfUgNAYbpGzScWln=869460486;if (ZsFMZwnQiAaveZlfUgNAYbpGzScWln == ZsFMZwnQiAaveZlfUgNAYbpGzScWln- 0 ) ZsFMZwnQiAaveZlfUgNAYbpGzScWln=1928579631; else ZsFMZwnQiAaveZlfUgNAYbpGzScWln=1744180262;if (ZsFMZwnQiAaveZlfUgNAYbpGzScWln == ZsFMZwnQiAaveZlfUgNAYbpGzScWln- 0 ) ZsFMZwnQiAaveZlfUgNAYbpGzScWln=1857186840; else ZsFMZwnQiAaveZlfUgNAYbpGzScWln=708173666;if (ZsFMZwnQiAaveZlfUgNAYbpGzScWln == ZsFMZwnQiAaveZlfUgNAYbpGzScWln- 1 ) ZsFMZwnQiAaveZlfUgNAYbpGzScWln=681422947; else ZsFMZwnQiAaveZlfUgNAYbpGzScWln=2074897893;if (ZsFMZwnQiAaveZlfUgNAYbpGzScWln == ZsFMZwnQiAaveZlfUgNAYbpGzScWln- 1 ) ZsFMZwnQiAaveZlfUgNAYbpGzScWln=544487560; else ZsFMZwnQiAaveZlfUgNAYbpGzScWln=334506966;int PTLDQviAoLnWukPcDUsVhXJAMctwae=1926690673;if (PTLDQviAoLnWukPcDUsVhXJAMctwae == PTLDQviAoLnWukPcDUsVhXJAMctwae- 1 ) PTLDQviAoLnWukPcDUsVhXJAMctwae=1543638549; else PTLDQviAoLnWukPcDUsVhXJAMctwae=948790568;if (PTLDQviAoLnWukPcDUsVhXJAMctwae == PTLDQviAoLnWukPcDUsVhXJAMctwae- 1 ) PTLDQviAoLnWukPcDUsVhXJAMctwae=847323098; else PTLDQviAoLnWukPcDUsVhXJAMctwae=1525947779;if (PTLDQviAoLnWukPcDUsVhXJAMctwae == PTLDQviAoLnWukPcDUsVhXJAMctwae- 1 ) PTLDQviAoLnWukPcDUsVhXJAMctwae=343013997; else PTLDQviAoLnWukPcDUsVhXJAMctwae=1502901895;if (PTLDQviAoLnWukPcDUsVhXJAMctwae == PTLDQviAoLnWukPcDUsVhXJAMctwae- 1 ) PTLDQviAoLnWukPcDUsVhXJAMctwae=1585704702; else PTLDQviAoLnWukPcDUsVhXJAMctwae=63893068;if (PTLDQviAoLnWukPcDUsVhXJAMctwae == PTLDQviAoLnWukPcDUsVhXJAMctwae- 0 ) PTLDQviAoLnWukPcDUsVhXJAMctwae=1373463023; else PTLDQviAoLnWukPcDUsVhXJAMctwae=364251218;if (PTLDQviAoLnWukPcDUsVhXJAMctwae == PTLDQviAoLnWukPcDUsVhXJAMctwae- 1 ) PTLDQviAoLnWukPcDUsVhXJAMctwae=1909017575; else PTLDQviAoLnWukPcDUsVhXJAMctwae=1718000824;long xjDxXzOagxemwamPFNaRyqmnjTjkuv=811245190;if (xjDxXzOagxemwamPFNaRyqmnjTjkuv == xjDxXzOagxemwamPFNaRyqmnjTjkuv- 0 ) xjDxXzOagxemwamPFNaRyqmnjTjkuv=1080179888; else xjDxXzOagxemwamPFNaRyqmnjTjkuv=174539598;if (xjDxXzOagxemwamPFNaRyqmnjTjkuv == xjDxXzOagxemwamPFNaRyqmnjTjkuv- 0 ) xjDxXzOagxemwamPFNaRyqmnjTjkuv=27016592; else xjDxXzOagxemwamPFNaRyqmnjTjkuv=2121892787;if (xjDxXzOagxemwamPFNaRyqmnjTjkuv == xjDxXzOagxemwamPFNaRyqmnjTjkuv- 0 ) xjDxXzOagxemwamPFNaRyqmnjTjkuv=991889879; else xjDxXzOagxemwamPFNaRyqmnjTjkuv=1447317124;if (xjDxXzOagxemwamPFNaRyqmnjTjkuv == xjDxXzOagxemwamPFNaRyqmnjTjkuv- 0 ) xjDxXzOagxemwamPFNaRyqmnjTjkuv=716925249; else xjDxXzOagxemwamPFNaRyqmnjTjkuv=1289927419;if (xjDxXzOagxemwamPFNaRyqmnjTjkuv == xjDxXzOagxemwamPFNaRyqmnjTjkuv- 1 ) xjDxXzOagxemwamPFNaRyqmnjTjkuv=1720740964; else xjDxXzOagxemwamPFNaRyqmnjTjkuv=1529470367;if (xjDxXzOagxemwamPFNaRyqmnjTjkuv == xjDxXzOagxemwamPFNaRyqmnjTjkuv- 1 ) xjDxXzOagxemwamPFNaRyqmnjTjkuv=172985959; else xjDxXzOagxemwamPFNaRyqmnjTjkuv=2137753092;double FMgSqKDdMpUkQMYInbWOzznYogBBVn=714474267.227690920886553535913886495579;if (FMgSqKDdMpUkQMYInbWOzznYogBBVn == FMgSqKDdMpUkQMYInbWOzznYogBBVn ) FMgSqKDdMpUkQMYInbWOzznYogBBVn=1879236323.011096001225713557010071855220; else FMgSqKDdMpUkQMYInbWOzznYogBBVn=1707891226.656197566853470917395957005035;if (FMgSqKDdMpUkQMYInbWOzznYogBBVn == FMgSqKDdMpUkQMYInbWOzznYogBBVn ) FMgSqKDdMpUkQMYInbWOzznYogBBVn=1306309992.298277963610136770294894680029; else FMgSqKDdMpUkQMYInbWOzznYogBBVn=178798939.354277621305479582233594140570;if (FMgSqKDdMpUkQMYInbWOzznYogBBVn == FMgSqKDdMpUkQMYInbWOzznYogBBVn ) FMgSqKDdMpUkQMYInbWOzznYogBBVn=234457033.805546707926090953510880686013; else FMgSqKDdMpUkQMYInbWOzznYogBBVn=1636600467.715160104399631851429807003596;if (FMgSqKDdMpUkQMYInbWOzznYogBBVn == FMgSqKDdMpUkQMYInbWOzznYogBBVn ) FMgSqKDdMpUkQMYInbWOzznYogBBVn=842765896.079421193196932236993679501917; else FMgSqKDdMpUkQMYInbWOzznYogBBVn=453447857.861833021186834398598771450103;if (FMgSqKDdMpUkQMYInbWOzznYogBBVn == FMgSqKDdMpUkQMYInbWOzznYogBBVn ) FMgSqKDdMpUkQMYInbWOzznYogBBVn=1760594774.038743520021592794522443010159; else FMgSqKDdMpUkQMYInbWOzznYogBBVn=1934328553.393916819297967729077679179684;if (FMgSqKDdMpUkQMYInbWOzznYogBBVn == FMgSqKDdMpUkQMYInbWOzznYogBBVn ) FMgSqKDdMpUkQMYInbWOzznYogBBVn=1643481110.458184979698643559739229273584; else FMgSqKDdMpUkQMYInbWOzznYogBBVn=1564328413.834695088496048013672005523001;float TsXtgVAatbmFQVNHsFOdxuFKoSsBgK=1468515271.118474897512942353609697411937f;if (TsXtgVAatbmFQVNHsFOdxuFKoSsBgK - TsXtgVAatbmFQVNHsFOdxuFKoSsBgK> 0.00000001 ) TsXtgVAatbmFQVNHsFOdxuFKoSsBgK=619000060.456229314655050574191137153206f; else TsXtgVAatbmFQVNHsFOdxuFKoSsBgK=1880726017.407026782315905574805895642918f;if (TsXtgVAatbmFQVNHsFOdxuFKoSsBgK - TsXtgVAatbmFQVNHsFOdxuFKoSsBgK> 0.00000001 ) TsXtgVAatbmFQVNHsFOdxuFKoSsBgK=1401810725.711936804499288724279951172248f; else TsXtgVAatbmFQVNHsFOdxuFKoSsBgK=351526241.587570418978041671792885087087f;if (TsXtgVAatbmFQVNHsFOdxuFKoSsBgK - TsXtgVAatbmFQVNHsFOdxuFKoSsBgK> 0.00000001 ) TsXtgVAatbmFQVNHsFOdxuFKoSsBgK=1136941016.576701372832740101161155771369f; else TsXtgVAatbmFQVNHsFOdxuFKoSsBgK=1584603989.453151245140007124699703742113f;if (TsXtgVAatbmFQVNHsFOdxuFKoSsBgK - TsXtgVAatbmFQVNHsFOdxuFKoSsBgK> 0.00000001 ) TsXtgVAatbmFQVNHsFOdxuFKoSsBgK=934800794.101359969906702126966788086152f; else TsXtgVAatbmFQVNHsFOdxuFKoSsBgK=96321425.024204076826670310173625843347f;if (TsXtgVAatbmFQVNHsFOdxuFKoSsBgK - TsXtgVAatbmFQVNHsFOdxuFKoSsBgK> 0.00000001 ) TsXtgVAatbmFQVNHsFOdxuFKoSsBgK=231865319.382808351825124012978609040957f; else TsXtgVAatbmFQVNHsFOdxuFKoSsBgK=1704879326.274955497216045861183157089497f;if (TsXtgVAatbmFQVNHsFOdxuFKoSsBgK - TsXtgVAatbmFQVNHsFOdxuFKoSsBgK> 0.00000001 ) TsXtgVAatbmFQVNHsFOdxuFKoSsBgK=1952690462.941465227063687277428987422798f; else TsXtgVAatbmFQVNHsFOdxuFKoSsBgK=961027277.611147668164523168191207907415f;float bPrSaLZUweGxQiWWgVPdWgEmXDUITD=1202613474.972841726463235590705466805205f;if (bPrSaLZUweGxQiWWgVPdWgEmXDUITD - bPrSaLZUweGxQiWWgVPdWgEmXDUITD> 0.00000001 ) bPrSaLZUweGxQiWWgVPdWgEmXDUITD=1546561898.571823180720548854504653752714f; else bPrSaLZUweGxQiWWgVPdWgEmXDUITD=1194543308.663109252252009692806270293817f;if (bPrSaLZUweGxQiWWgVPdWgEmXDUITD - bPrSaLZUweGxQiWWgVPdWgEmXDUITD> 0.00000001 ) bPrSaLZUweGxQiWWgVPdWgEmXDUITD=1986248148.247716755971236375527092949401f; else bPrSaLZUweGxQiWWgVPdWgEmXDUITD=222728115.754611639305803515402940035698f;if (bPrSaLZUweGxQiWWgVPdWgEmXDUITD - bPrSaLZUweGxQiWWgVPdWgEmXDUITD> 0.00000001 ) bPrSaLZUweGxQiWWgVPdWgEmXDUITD=54081331.845433873574623330944501982469f; else bPrSaLZUweGxQiWWgVPdWgEmXDUITD=1277098894.605266047185293932880676435555f;if (bPrSaLZUweGxQiWWgVPdWgEmXDUITD - bPrSaLZUweGxQiWWgVPdWgEmXDUITD> 0.00000001 ) bPrSaLZUweGxQiWWgVPdWgEmXDUITD=576788157.241508946730486031915079913083f; else bPrSaLZUweGxQiWWgVPdWgEmXDUITD=1591063068.269204480500615139050475128369f;if (bPrSaLZUweGxQiWWgVPdWgEmXDUITD - bPrSaLZUweGxQiWWgVPdWgEmXDUITD> 0.00000001 ) bPrSaLZUweGxQiWWgVPdWgEmXDUITD=1479257349.507468129266242646489514399529f; else bPrSaLZUweGxQiWWgVPdWgEmXDUITD=1721052382.722111336811180534459399119318f;if (bPrSaLZUweGxQiWWgVPdWgEmXDUITD - bPrSaLZUweGxQiWWgVPdWgEmXDUITD> 0.00000001 ) bPrSaLZUweGxQiWWgVPdWgEmXDUITD=361298013.971164413131078843497628043411f; else bPrSaLZUweGxQiWWgVPdWgEmXDUITD=1205044973.213994113612132833138703236412f;long EALvOtHnVnMBsnWcteOGALoRcFzmdo=1581554948;if (EALvOtHnVnMBsnWcteOGALoRcFzmdo == EALvOtHnVnMBsnWcteOGALoRcFzmdo- 1 ) EALvOtHnVnMBsnWcteOGALoRcFzmdo=604432395; else EALvOtHnVnMBsnWcteOGALoRcFzmdo=851954335;if (EALvOtHnVnMBsnWcteOGALoRcFzmdo == EALvOtHnVnMBsnWcteOGALoRcFzmdo- 1 ) EALvOtHnVnMBsnWcteOGALoRcFzmdo=1518753032; else EALvOtHnVnMBsnWcteOGALoRcFzmdo=29222098;if (EALvOtHnVnMBsnWcteOGALoRcFzmdo == EALvOtHnVnMBsnWcteOGALoRcFzmdo- 0 ) EALvOtHnVnMBsnWcteOGALoRcFzmdo=916522471; else EALvOtHnVnMBsnWcteOGALoRcFzmdo=613646726;if (EALvOtHnVnMBsnWcteOGALoRcFzmdo == EALvOtHnVnMBsnWcteOGALoRcFzmdo- 0 ) EALvOtHnVnMBsnWcteOGALoRcFzmdo=1725016898; else EALvOtHnVnMBsnWcteOGALoRcFzmdo=294632334;if (EALvOtHnVnMBsnWcteOGALoRcFzmdo == EALvOtHnVnMBsnWcteOGALoRcFzmdo- 0 ) EALvOtHnVnMBsnWcteOGALoRcFzmdo=773090432; else EALvOtHnVnMBsnWcteOGALoRcFzmdo=1209988567;if (EALvOtHnVnMBsnWcteOGALoRcFzmdo == EALvOtHnVnMBsnWcteOGALoRcFzmdo- 0 ) EALvOtHnVnMBsnWcteOGALoRcFzmdo=2094417267; else EALvOtHnVnMBsnWcteOGALoRcFzmdo=677061354;long qbesHmWCLegxKeWWWBoNuhNKjjxzyF=165159517;if (qbesHmWCLegxKeWWWBoNuhNKjjxzyF == qbesHmWCLegxKeWWWBoNuhNKjjxzyF- 1 ) qbesHmWCLegxKeWWWBoNuhNKjjxzyF=1632899953; else qbesHmWCLegxKeWWWBoNuhNKjjxzyF=440867636;if (qbesHmWCLegxKeWWWBoNuhNKjjxzyF == qbesHmWCLegxKeWWWBoNuhNKjjxzyF- 0 ) qbesHmWCLegxKeWWWBoNuhNKjjxzyF=17766515; else qbesHmWCLegxKeWWWBoNuhNKjjxzyF=788103026;if (qbesHmWCLegxKeWWWBoNuhNKjjxzyF == qbesHmWCLegxKeWWWBoNuhNKjjxzyF- 0 ) qbesHmWCLegxKeWWWBoNuhNKjjxzyF=300421515; else qbesHmWCLegxKeWWWBoNuhNKjjxzyF=702309857;if (qbesHmWCLegxKeWWWBoNuhNKjjxzyF == qbesHmWCLegxKeWWWBoNuhNKjjxzyF- 1 ) qbesHmWCLegxKeWWWBoNuhNKjjxzyF=1414433940; else qbesHmWCLegxKeWWWBoNuhNKjjxzyF=1489384813;if (qbesHmWCLegxKeWWWBoNuhNKjjxzyF == qbesHmWCLegxKeWWWBoNuhNKjjxzyF- 0 ) qbesHmWCLegxKeWWWBoNuhNKjjxzyF=1303937860; else qbesHmWCLegxKeWWWBoNuhNKjjxzyF=1958231304;if (qbesHmWCLegxKeWWWBoNuhNKjjxzyF == qbesHmWCLegxKeWWWBoNuhNKjjxzyF- 1 ) qbesHmWCLegxKeWWWBoNuhNKjjxzyF=1019752539; else qbesHmWCLegxKeWWWBoNuhNKjjxzyF=1563372281;long vJoPMJLhmQYgfLxQWzBMsuSwPCdnob=1079268483;if (vJoPMJLhmQYgfLxQWzBMsuSwPCdnob == vJoPMJLhmQYgfLxQWzBMsuSwPCdnob- 0 ) vJoPMJLhmQYgfLxQWzBMsuSwPCdnob=250114594; else vJoPMJLhmQYgfLxQWzBMsuSwPCdnob=371135514;if (vJoPMJLhmQYgfLxQWzBMsuSwPCdnob == vJoPMJLhmQYgfLxQWzBMsuSwPCdnob- 0 ) vJoPMJLhmQYgfLxQWzBMsuSwPCdnob=356594882; else vJoPMJLhmQYgfLxQWzBMsuSwPCdnob=477961601;if (vJoPMJLhmQYgfLxQWzBMsuSwPCdnob == vJoPMJLhmQYgfLxQWzBMsuSwPCdnob- 1 ) vJoPMJLhmQYgfLxQWzBMsuSwPCdnob=76682225; else vJoPMJLhmQYgfLxQWzBMsuSwPCdnob=272206502;if (vJoPMJLhmQYgfLxQWzBMsuSwPCdnob == vJoPMJLhmQYgfLxQWzBMsuSwPCdnob- 1 ) vJoPMJLhmQYgfLxQWzBMsuSwPCdnob=286091003; else vJoPMJLhmQYgfLxQWzBMsuSwPCdnob=932915172;if (vJoPMJLhmQYgfLxQWzBMsuSwPCdnob == vJoPMJLhmQYgfLxQWzBMsuSwPCdnob- 0 ) vJoPMJLhmQYgfLxQWzBMsuSwPCdnob=1617010057; else vJoPMJLhmQYgfLxQWzBMsuSwPCdnob=1691615814;if (vJoPMJLhmQYgfLxQWzBMsuSwPCdnob == vJoPMJLhmQYgfLxQWzBMsuSwPCdnob- 0 ) vJoPMJLhmQYgfLxQWzBMsuSwPCdnob=1597121330; else vJoPMJLhmQYgfLxQWzBMsuSwPCdnob=991576411;double swtTNYSllvQHQIaqNdRatNubXZllev=1597518730.940585733624657634449817073085;if (swtTNYSllvQHQIaqNdRatNubXZllev == swtTNYSllvQHQIaqNdRatNubXZllev ) swtTNYSllvQHQIaqNdRatNubXZllev=450036814.832407240227167900750801024352; else swtTNYSllvQHQIaqNdRatNubXZllev=196138496.275990336159533375127434513158;if (swtTNYSllvQHQIaqNdRatNubXZllev == swtTNYSllvQHQIaqNdRatNubXZllev ) swtTNYSllvQHQIaqNdRatNubXZllev=1719628667.399905238878471138975034595249; else swtTNYSllvQHQIaqNdRatNubXZllev=1510452025.828896163925736007358538336651;if (swtTNYSllvQHQIaqNdRatNubXZllev == swtTNYSllvQHQIaqNdRatNubXZllev ) swtTNYSllvQHQIaqNdRatNubXZllev=1282313968.109906069518606140950800239056; else swtTNYSllvQHQIaqNdRatNubXZllev=666661706.061292784114984004330670231988;if (swtTNYSllvQHQIaqNdRatNubXZllev == swtTNYSllvQHQIaqNdRatNubXZllev ) swtTNYSllvQHQIaqNdRatNubXZllev=1281133433.702355741456795702624683749217; else swtTNYSllvQHQIaqNdRatNubXZllev=2049606185.280641373946398409564937423961;if (swtTNYSllvQHQIaqNdRatNubXZllev == swtTNYSllvQHQIaqNdRatNubXZllev ) swtTNYSllvQHQIaqNdRatNubXZllev=1585004042.764537615543979939535176336433; else swtTNYSllvQHQIaqNdRatNubXZllev=29368219.941338287199412116869443779931;if (swtTNYSllvQHQIaqNdRatNubXZllev == swtTNYSllvQHQIaqNdRatNubXZllev ) swtTNYSllvQHQIaqNdRatNubXZllev=872767885.229361375186345677433068460967; else swtTNYSllvQHQIaqNdRatNubXZllev=1105774814.036707125194289320810250936191;double UDrEWBdXrccJOWqrtwFTzCbgqocrkC=1043334611.135816692233541691253176097051;if (UDrEWBdXrccJOWqrtwFTzCbgqocrkC == UDrEWBdXrccJOWqrtwFTzCbgqocrkC ) UDrEWBdXrccJOWqrtwFTzCbgqocrkC=917503156.472904172266390760341575417082; else UDrEWBdXrccJOWqrtwFTzCbgqocrkC=1015148784.075186265862910898643771190869;if (UDrEWBdXrccJOWqrtwFTzCbgqocrkC == UDrEWBdXrccJOWqrtwFTzCbgqocrkC ) UDrEWBdXrccJOWqrtwFTzCbgqocrkC=426946760.720020121550569967563054930519; else UDrEWBdXrccJOWqrtwFTzCbgqocrkC=1460258138.651714399177729332234430788629;if (UDrEWBdXrccJOWqrtwFTzCbgqocrkC == UDrEWBdXrccJOWqrtwFTzCbgqocrkC ) UDrEWBdXrccJOWqrtwFTzCbgqocrkC=1672627384.071291095667950634561949632104; else UDrEWBdXrccJOWqrtwFTzCbgqocrkC=793053967.778011763945777758116516923376;if (UDrEWBdXrccJOWqrtwFTzCbgqocrkC == UDrEWBdXrccJOWqrtwFTzCbgqocrkC ) UDrEWBdXrccJOWqrtwFTzCbgqocrkC=88477292.999299095908173728208510836329; else UDrEWBdXrccJOWqrtwFTzCbgqocrkC=1360693451.849805658887321845330617859946;if (UDrEWBdXrccJOWqrtwFTzCbgqocrkC == UDrEWBdXrccJOWqrtwFTzCbgqocrkC ) UDrEWBdXrccJOWqrtwFTzCbgqocrkC=154776633.478626948918052230919200024205; else UDrEWBdXrccJOWqrtwFTzCbgqocrkC=1232334990.771351071501752277673914727870;if (UDrEWBdXrccJOWqrtwFTzCbgqocrkC == UDrEWBdXrccJOWqrtwFTzCbgqocrkC ) UDrEWBdXrccJOWqrtwFTzCbgqocrkC=1318576616.374324880126041966886773387763; else UDrEWBdXrccJOWqrtwFTzCbgqocrkC=756289348.521123743850755707497200470215;long BLljzasJoIHMCNhCMpfozTghMJGjdP=1404211057;if (BLljzasJoIHMCNhCMpfozTghMJGjdP == BLljzasJoIHMCNhCMpfozTghMJGjdP- 0 ) BLljzasJoIHMCNhCMpfozTghMJGjdP=51078241; else BLljzasJoIHMCNhCMpfozTghMJGjdP=1569717456;if (BLljzasJoIHMCNhCMpfozTghMJGjdP == BLljzasJoIHMCNhCMpfozTghMJGjdP- 0 ) BLljzasJoIHMCNhCMpfozTghMJGjdP=1113335467; else BLljzasJoIHMCNhCMpfozTghMJGjdP=1027167480;if (BLljzasJoIHMCNhCMpfozTghMJGjdP == BLljzasJoIHMCNhCMpfozTghMJGjdP- 1 ) BLljzasJoIHMCNhCMpfozTghMJGjdP=157610801; else BLljzasJoIHMCNhCMpfozTghMJGjdP=929517033;if (BLljzasJoIHMCNhCMpfozTghMJGjdP == BLljzasJoIHMCNhCMpfozTghMJGjdP- 1 ) BLljzasJoIHMCNhCMpfozTghMJGjdP=1944438162; else BLljzasJoIHMCNhCMpfozTghMJGjdP=1730936535;if (BLljzasJoIHMCNhCMpfozTghMJGjdP == BLljzasJoIHMCNhCMpfozTghMJGjdP- 1 ) BLljzasJoIHMCNhCMpfozTghMJGjdP=1237485728; else BLljzasJoIHMCNhCMpfozTghMJGjdP=454472344;if (BLljzasJoIHMCNhCMpfozTghMJGjdP == BLljzasJoIHMCNhCMpfozTghMJGjdP- 0 ) BLljzasJoIHMCNhCMpfozTghMJGjdP=1040441206; else BLljzasJoIHMCNhCMpfozTghMJGjdP=47314006;double hTvshiMbeGKHnYkAoZRLPgylwUqdQJ=1455365440.333494832698534236530500161112;if (hTvshiMbeGKHnYkAoZRLPgylwUqdQJ == hTvshiMbeGKHnYkAoZRLPgylwUqdQJ ) hTvshiMbeGKHnYkAoZRLPgylwUqdQJ=472805809.323633470339837262736345793434; else hTvshiMbeGKHnYkAoZRLPgylwUqdQJ=553786719.482147590296630188300049719905;if (hTvshiMbeGKHnYkAoZRLPgylwUqdQJ == hTvshiMbeGKHnYkAoZRLPgylwUqdQJ ) hTvshiMbeGKHnYkAoZRLPgylwUqdQJ=74073255.004705889178255071572524545443; else hTvshiMbeGKHnYkAoZRLPgylwUqdQJ=315016490.979639204540933389485854469136;if (hTvshiMbeGKHnYkAoZRLPgylwUqdQJ == hTvshiMbeGKHnYkAoZRLPgylwUqdQJ ) hTvshiMbeGKHnYkAoZRLPgylwUqdQJ=1620263690.144721963787803260127607912637; else hTvshiMbeGKHnYkAoZRLPgylwUqdQJ=446894925.127950631352680816120901964881;if (hTvshiMbeGKHnYkAoZRLPgylwUqdQJ == hTvshiMbeGKHnYkAoZRLPgylwUqdQJ ) hTvshiMbeGKHnYkAoZRLPgylwUqdQJ=2072374993.934777096364098502721751722709; else hTvshiMbeGKHnYkAoZRLPgylwUqdQJ=1804466494.795052902482186702456317028305;if (hTvshiMbeGKHnYkAoZRLPgylwUqdQJ == hTvshiMbeGKHnYkAoZRLPgylwUqdQJ ) hTvshiMbeGKHnYkAoZRLPgylwUqdQJ=107661204.540778904761045078602046448903; else hTvshiMbeGKHnYkAoZRLPgylwUqdQJ=358935216.461894242890979638315180361533;if (hTvshiMbeGKHnYkAoZRLPgylwUqdQJ == hTvshiMbeGKHnYkAoZRLPgylwUqdQJ ) hTvshiMbeGKHnYkAoZRLPgylwUqdQJ=1355675392.111141036183782227792160939153; else hTvshiMbeGKHnYkAoZRLPgylwUqdQJ=49978978.382989084960223351925485205131;double vsMoAGowRLURqQLtUjennNPrvBmlpQ=1162146513.878195521486372543690360890122;if (vsMoAGowRLURqQLtUjennNPrvBmlpQ == vsMoAGowRLURqQLtUjennNPrvBmlpQ ) vsMoAGowRLURqQLtUjennNPrvBmlpQ=2017632327.138220568031571286575989051941; else vsMoAGowRLURqQLtUjennNPrvBmlpQ=1616040475.550945925218667990961466835927;if (vsMoAGowRLURqQLtUjennNPrvBmlpQ == vsMoAGowRLURqQLtUjennNPrvBmlpQ ) vsMoAGowRLURqQLtUjennNPrvBmlpQ=1054696573.972768264737137733227208019522; else vsMoAGowRLURqQLtUjennNPrvBmlpQ=171660378.000844691974734534010371187585;if (vsMoAGowRLURqQLtUjennNPrvBmlpQ == vsMoAGowRLURqQLtUjennNPrvBmlpQ ) vsMoAGowRLURqQLtUjennNPrvBmlpQ=1195792797.084273840314245468558652283986; else vsMoAGowRLURqQLtUjennNPrvBmlpQ=191642525.821700057180571624683904743309;if (vsMoAGowRLURqQLtUjennNPrvBmlpQ == vsMoAGowRLURqQLtUjennNPrvBmlpQ ) vsMoAGowRLURqQLtUjennNPrvBmlpQ=885723810.096459455641147829277031492816; else vsMoAGowRLURqQLtUjennNPrvBmlpQ=2135469770.785810513237914287793524316616;if (vsMoAGowRLURqQLtUjennNPrvBmlpQ == vsMoAGowRLURqQLtUjennNPrvBmlpQ ) vsMoAGowRLURqQLtUjennNPrvBmlpQ=1218194269.865097734841605012722612337161; else vsMoAGowRLURqQLtUjennNPrvBmlpQ=843493872.491757203866395392030409526386;if (vsMoAGowRLURqQLtUjennNPrvBmlpQ == vsMoAGowRLURqQLtUjennNPrvBmlpQ ) vsMoAGowRLURqQLtUjennNPrvBmlpQ=222772410.168355106892097733704524100233; else vsMoAGowRLURqQLtUjennNPrvBmlpQ=1218155040.293781815130625952399378364536;int ujOixsODerMpioVvAvKZqtTUrObXNi=974460444;if (ujOixsODerMpioVvAvKZqtTUrObXNi == ujOixsODerMpioVvAvKZqtTUrObXNi- 1 ) ujOixsODerMpioVvAvKZqtTUrObXNi=2040797652; else ujOixsODerMpioVvAvKZqtTUrObXNi=2010694823;if (ujOixsODerMpioVvAvKZqtTUrObXNi == ujOixsODerMpioVvAvKZqtTUrObXNi- 1 ) ujOixsODerMpioVvAvKZqtTUrObXNi=283483539; else ujOixsODerMpioVvAvKZqtTUrObXNi=698885455;if (ujOixsODerMpioVvAvKZqtTUrObXNi == ujOixsODerMpioVvAvKZqtTUrObXNi- 0 ) ujOixsODerMpioVvAvKZqtTUrObXNi=348784274; else ujOixsODerMpioVvAvKZqtTUrObXNi=1991686696;if (ujOixsODerMpioVvAvKZqtTUrObXNi == ujOixsODerMpioVvAvKZqtTUrObXNi- 1 ) ujOixsODerMpioVvAvKZqtTUrObXNi=1931431014; else ujOixsODerMpioVvAvKZqtTUrObXNi=1751666963;if (ujOixsODerMpioVvAvKZqtTUrObXNi == ujOixsODerMpioVvAvKZqtTUrObXNi- 0 ) ujOixsODerMpioVvAvKZqtTUrObXNi=1644826110; else ujOixsODerMpioVvAvKZqtTUrObXNi=1456352620;if (ujOixsODerMpioVvAvKZqtTUrObXNi == ujOixsODerMpioVvAvKZqtTUrObXNi- 0 ) ujOixsODerMpioVvAvKZqtTUrObXNi=919506276; else ujOixsODerMpioVvAvKZqtTUrObXNi=1946159232;float vkXkUzgABrcmhUoMKovByIaVdokbdW=712641643.379749525411144932048805318682f;if (vkXkUzgABrcmhUoMKovByIaVdokbdW - vkXkUzgABrcmhUoMKovByIaVdokbdW> 0.00000001 ) vkXkUzgABrcmhUoMKovByIaVdokbdW=1805021980.503410182443586757741464967554f; else vkXkUzgABrcmhUoMKovByIaVdokbdW=2025778340.104392973595171643967195224966f;if (vkXkUzgABrcmhUoMKovByIaVdokbdW - vkXkUzgABrcmhUoMKovByIaVdokbdW> 0.00000001 ) vkXkUzgABrcmhUoMKovByIaVdokbdW=177165326.373935020598536640333560811927f; else vkXkUzgABrcmhUoMKovByIaVdokbdW=2078365436.902823235672448976969473491412f;if (vkXkUzgABrcmhUoMKovByIaVdokbdW - vkXkUzgABrcmhUoMKovByIaVdokbdW> 0.00000001 ) vkXkUzgABrcmhUoMKovByIaVdokbdW=273103263.411369772442687123294506954054f; else vkXkUzgABrcmhUoMKovByIaVdokbdW=1167360607.292337219931562799638923901542f;if (vkXkUzgABrcmhUoMKovByIaVdokbdW - vkXkUzgABrcmhUoMKovByIaVdokbdW> 0.00000001 ) vkXkUzgABrcmhUoMKovByIaVdokbdW=259572643.673826136191547405426542747043f; else vkXkUzgABrcmhUoMKovByIaVdokbdW=281163284.788746875776465759660612624228f;if (vkXkUzgABrcmhUoMKovByIaVdokbdW - vkXkUzgABrcmhUoMKovByIaVdokbdW> 0.00000001 ) vkXkUzgABrcmhUoMKovByIaVdokbdW=1815546556.929352380264076118800949417623f; else vkXkUzgABrcmhUoMKovByIaVdokbdW=86087132.275896722357940674761643808195f;if (vkXkUzgABrcmhUoMKovByIaVdokbdW - vkXkUzgABrcmhUoMKovByIaVdokbdW> 0.00000001 ) vkXkUzgABrcmhUoMKovByIaVdokbdW=1244305868.586800939040756401084407273309f; else vkXkUzgABrcmhUoMKovByIaVdokbdW=776769818.973898124185672200851812118064f;float hNxJiivgpLrhvBxrQWEjTWbsFeIjmw=66497918.399205795092813208917695058878f;if (hNxJiivgpLrhvBxrQWEjTWbsFeIjmw - hNxJiivgpLrhvBxrQWEjTWbsFeIjmw> 0.00000001 ) hNxJiivgpLrhvBxrQWEjTWbsFeIjmw=545164029.855177773035339007450070781526f; else hNxJiivgpLrhvBxrQWEjTWbsFeIjmw=381214948.792734884754608236616253989540f;if (hNxJiivgpLrhvBxrQWEjTWbsFeIjmw - hNxJiivgpLrhvBxrQWEjTWbsFeIjmw> 0.00000001 ) hNxJiivgpLrhvBxrQWEjTWbsFeIjmw=156664973.635735371623370747255502165095f; else hNxJiivgpLrhvBxrQWEjTWbsFeIjmw=1632357561.634513997497509398424230623477f;if (hNxJiivgpLrhvBxrQWEjTWbsFeIjmw - hNxJiivgpLrhvBxrQWEjTWbsFeIjmw> 0.00000001 ) hNxJiivgpLrhvBxrQWEjTWbsFeIjmw=241360546.049709097886196910388777141500f; else hNxJiivgpLrhvBxrQWEjTWbsFeIjmw=98048556.062045981141265925471429038276f;if (hNxJiivgpLrhvBxrQWEjTWbsFeIjmw - hNxJiivgpLrhvBxrQWEjTWbsFeIjmw> 0.00000001 ) hNxJiivgpLrhvBxrQWEjTWbsFeIjmw=813362239.960783415103035761233613974302f; else hNxJiivgpLrhvBxrQWEjTWbsFeIjmw=25945631.872528169252037959538805435882f;if (hNxJiivgpLrhvBxrQWEjTWbsFeIjmw - hNxJiivgpLrhvBxrQWEjTWbsFeIjmw> 0.00000001 ) hNxJiivgpLrhvBxrQWEjTWbsFeIjmw=1832093108.911444251770308279612597407729f; else hNxJiivgpLrhvBxrQWEjTWbsFeIjmw=1450513257.448744169480987860737638043414f;if (hNxJiivgpLrhvBxrQWEjTWbsFeIjmw - hNxJiivgpLrhvBxrQWEjTWbsFeIjmw> 0.00000001 ) hNxJiivgpLrhvBxrQWEjTWbsFeIjmw=674317915.853343367511023272165700282170f; else hNxJiivgpLrhvBxrQWEjTWbsFeIjmw=1886923824.286574765460432671587245015416f;int JwhQSbacwZEkhAaPmoeGEbsQzTDbdt=1471218732;if (JwhQSbacwZEkhAaPmoeGEbsQzTDbdt == JwhQSbacwZEkhAaPmoeGEbsQzTDbdt- 1 ) JwhQSbacwZEkhAaPmoeGEbsQzTDbdt=608126074; else JwhQSbacwZEkhAaPmoeGEbsQzTDbdt=1657614414;if (JwhQSbacwZEkhAaPmoeGEbsQzTDbdt == JwhQSbacwZEkhAaPmoeGEbsQzTDbdt- 0 ) JwhQSbacwZEkhAaPmoeGEbsQzTDbdt=1560616707; else JwhQSbacwZEkhAaPmoeGEbsQzTDbdt=2021801715;if (JwhQSbacwZEkhAaPmoeGEbsQzTDbdt == JwhQSbacwZEkhAaPmoeGEbsQzTDbdt- 1 ) JwhQSbacwZEkhAaPmoeGEbsQzTDbdt=2011481607; else JwhQSbacwZEkhAaPmoeGEbsQzTDbdt=2030373045;if (JwhQSbacwZEkhAaPmoeGEbsQzTDbdt == JwhQSbacwZEkhAaPmoeGEbsQzTDbdt- 0 ) JwhQSbacwZEkhAaPmoeGEbsQzTDbdt=701357181; else JwhQSbacwZEkhAaPmoeGEbsQzTDbdt=873437045;if (JwhQSbacwZEkhAaPmoeGEbsQzTDbdt == JwhQSbacwZEkhAaPmoeGEbsQzTDbdt- 0 ) JwhQSbacwZEkhAaPmoeGEbsQzTDbdt=1751122180; else JwhQSbacwZEkhAaPmoeGEbsQzTDbdt=2320255;if (JwhQSbacwZEkhAaPmoeGEbsQzTDbdt == JwhQSbacwZEkhAaPmoeGEbsQzTDbdt- 0 ) JwhQSbacwZEkhAaPmoeGEbsQzTDbdt=1030822546; else JwhQSbacwZEkhAaPmoeGEbsQzTDbdt=262697142;float QsMmgcrzWTSZDSXdwqBgQXufVOCmIi=790606157.947056401659503376870515149912f;if (QsMmgcrzWTSZDSXdwqBgQXufVOCmIi - QsMmgcrzWTSZDSXdwqBgQXufVOCmIi> 0.00000001 ) QsMmgcrzWTSZDSXdwqBgQXufVOCmIi=1551982534.130081129724652930879603854202f; else QsMmgcrzWTSZDSXdwqBgQXufVOCmIi=853008358.850063953225715697321014363823f;if (QsMmgcrzWTSZDSXdwqBgQXufVOCmIi - QsMmgcrzWTSZDSXdwqBgQXufVOCmIi> 0.00000001 ) QsMmgcrzWTSZDSXdwqBgQXufVOCmIi=1400995203.793807479018766490836898184671f; else QsMmgcrzWTSZDSXdwqBgQXufVOCmIi=1340110340.024063391823842336907300800583f;if (QsMmgcrzWTSZDSXdwqBgQXufVOCmIi - QsMmgcrzWTSZDSXdwqBgQXufVOCmIi> 0.00000001 ) QsMmgcrzWTSZDSXdwqBgQXufVOCmIi=1737794211.491800581321901406795988572426f; else QsMmgcrzWTSZDSXdwqBgQXufVOCmIi=750493884.482954214386568650050599961433f;if (QsMmgcrzWTSZDSXdwqBgQXufVOCmIi - QsMmgcrzWTSZDSXdwqBgQXufVOCmIi> 0.00000001 ) QsMmgcrzWTSZDSXdwqBgQXufVOCmIi=471281097.826504016720351052003258700300f; else QsMmgcrzWTSZDSXdwqBgQXufVOCmIi=1706973424.498156007474533024801944178991f;if (QsMmgcrzWTSZDSXdwqBgQXufVOCmIi - QsMmgcrzWTSZDSXdwqBgQXufVOCmIi> 0.00000001 ) QsMmgcrzWTSZDSXdwqBgQXufVOCmIi=1212416101.471744320605663982588761955419f; else QsMmgcrzWTSZDSXdwqBgQXufVOCmIi=151219695.912710490081430748129642482180f;if (QsMmgcrzWTSZDSXdwqBgQXufVOCmIi - QsMmgcrzWTSZDSXdwqBgQXufVOCmIi> 0.00000001 ) QsMmgcrzWTSZDSXdwqBgQXufVOCmIi=246272328.992279496460361733131749038143f; else QsMmgcrzWTSZDSXdwqBgQXufVOCmIi=970073653.931019937136387049176006926424f;long QdRZPuOZInxfAvcYcJAxuwghvQWGvO=728498331;if (QdRZPuOZInxfAvcYcJAxuwghvQWGvO == QdRZPuOZInxfAvcYcJAxuwghvQWGvO- 1 ) QdRZPuOZInxfAvcYcJAxuwghvQWGvO=1819426297; else QdRZPuOZInxfAvcYcJAxuwghvQWGvO=1920570783;if (QdRZPuOZInxfAvcYcJAxuwghvQWGvO == QdRZPuOZInxfAvcYcJAxuwghvQWGvO- 0 ) QdRZPuOZInxfAvcYcJAxuwghvQWGvO=168643744; else QdRZPuOZInxfAvcYcJAxuwghvQWGvO=713991748;if (QdRZPuOZInxfAvcYcJAxuwghvQWGvO == QdRZPuOZInxfAvcYcJAxuwghvQWGvO- 0 ) QdRZPuOZInxfAvcYcJAxuwghvQWGvO=1441086893; else QdRZPuOZInxfAvcYcJAxuwghvQWGvO=30052018;if (QdRZPuOZInxfAvcYcJAxuwghvQWGvO == QdRZPuOZInxfAvcYcJAxuwghvQWGvO- 1 ) QdRZPuOZInxfAvcYcJAxuwghvQWGvO=202499958; else QdRZPuOZInxfAvcYcJAxuwghvQWGvO=662274631;if (QdRZPuOZInxfAvcYcJAxuwghvQWGvO == QdRZPuOZInxfAvcYcJAxuwghvQWGvO- 1 ) QdRZPuOZInxfAvcYcJAxuwghvQWGvO=1827341414; else QdRZPuOZInxfAvcYcJAxuwghvQWGvO=1430711575;if (QdRZPuOZInxfAvcYcJAxuwghvQWGvO == QdRZPuOZInxfAvcYcJAxuwghvQWGvO- 0 ) QdRZPuOZInxfAvcYcJAxuwghvQWGvO=2028719028; else QdRZPuOZInxfAvcYcJAxuwghvQWGvO=239040291;long ZCWmQNPULcOyFLkNKeBdxKAFxtOhfa=677431912;if (ZCWmQNPULcOyFLkNKeBdxKAFxtOhfa == ZCWmQNPULcOyFLkNKeBdxKAFxtOhfa- 0 ) ZCWmQNPULcOyFLkNKeBdxKAFxtOhfa=2070452456; else ZCWmQNPULcOyFLkNKeBdxKAFxtOhfa=2031195405;if (ZCWmQNPULcOyFLkNKeBdxKAFxtOhfa == ZCWmQNPULcOyFLkNKeBdxKAFxtOhfa- 0 ) ZCWmQNPULcOyFLkNKeBdxKAFxtOhfa=334941290; else ZCWmQNPULcOyFLkNKeBdxKAFxtOhfa=566611538;if (ZCWmQNPULcOyFLkNKeBdxKAFxtOhfa == ZCWmQNPULcOyFLkNKeBdxKAFxtOhfa- 0 ) ZCWmQNPULcOyFLkNKeBdxKAFxtOhfa=1833536669; else ZCWmQNPULcOyFLkNKeBdxKAFxtOhfa=1221517789;if (ZCWmQNPULcOyFLkNKeBdxKAFxtOhfa == ZCWmQNPULcOyFLkNKeBdxKAFxtOhfa- 0 ) ZCWmQNPULcOyFLkNKeBdxKAFxtOhfa=1880908168; else ZCWmQNPULcOyFLkNKeBdxKAFxtOhfa=2005115837;if (ZCWmQNPULcOyFLkNKeBdxKAFxtOhfa == ZCWmQNPULcOyFLkNKeBdxKAFxtOhfa- 0 ) ZCWmQNPULcOyFLkNKeBdxKAFxtOhfa=1186333317; else ZCWmQNPULcOyFLkNKeBdxKAFxtOhfa=2001126930;if (ZCWmQNPULcOyFLkNKeBdxKAFxtOhfa == ZCWmQNPULcOyFLkNKeBdxKAFxtOhfa- 0 ) ZCWmQNPULcOyFLkNKeBdxKAFxtOhfa=809385614; else ZCWmQNPULcOyFLkNKeBdxKAFxtOhfa=1860261912;double RCVUrekYTYOAhIrkDcRqUdFZvdQpRj=1230989714.142980071575311609756398113944;if (RCVUrekYTYOAhIrkDcRqUdFZvdQpRj == RCVUrekYTYOAhIrkDcRqUdFZvdQpRj ) RCVUrekYTYOAhIrkDcRqUdFZvdQpRj=1140776867.261655861181478154019752085497; else RCVUrekYTYOAhIrkDcRqUdFZvdQpRj=302324215.823732476785977602006993561077;if (RCVUrekYTYOAhIrkDcRqUdFZvdQpRj == RCVUrekYTYOAhIrkDcRqUdFZvdQpRj ) RCVUrekYTYOAhIrkDcRqUdFZvdQpRj=590754492.157437875679844598549340193038; else RCVUrekYTYOAhIrkDcRqUdFZvdQpRj=974293692.150064854083853826008454981835;if (RCVUrekYTYOAhIrkDcRqUdFZvdQpRj == RCVUrekYTYOAhIrkDcRqUdFZvdQpRj ) RCVUrekYTYOAhIrkDcRqUdFZvdQpRj=986017452.595763998184457650748713080718; else RCVUrekYTYOAhIrkDcRqUdFZvdQpRj=813552700.403060919003803500744110788278;if (RCVUrekYTYOAhIrkDcRqUdFZvdQpRj == RCVUrekYTYOAhIrkDcRqUdFZvdQpRj ) RCVUrekYTYOAhIrkDcRqUdFZvdQpRj=1497002911.438088250774185501069008539348; else RCVUrekYTYOAhIrkDcRqUdFZvdQpRj=1521930516.285177593636117004629721043165;if (RCVUrekYTYOAhIrkDcRqUdFZvdQpRj == RCVUrekYTYOAhIrkDcRqUdFZvdQpRj ) RCVUrekYTYOAhIrkDcRqUdFZvdQpRj=650508400.991360001801954300153682956424; else RCVUrekYTYOAhIrkDcRqUdFZvdQpRj=738720572.453874586810492898874411342051;if (RCVUrekYTYOAhIrkDcRqUdFZvdQpRj == RCVUrekYTYOAhIrkDcRqUdFZvdQpRj ) RCVUrekYTYOAhIrkDcRqUdFZvdQpRj=1660252573.579746929093535329017965014364; else RCVUrekYTYOAhIrkDcRqUdFZvdQpRj=307082636.354545275707998208702304471583;long egZmUtyxzHbVSCyvkioHfecIwZwDkQ=661751004;if (egZmUtyxzHbVSCyvkioHfecIwZwDkQ == egZmUtyxzHbVSCyvkioHfecIwZwDkQ- 1 ) egZmUtyxzHbVSCyvkioHfecIwZwDkQ=872654892; else egZmUtyxzHbVSCyvkioHfecIwZwDkQ=1716324063;if (egZmUtyxzHbVSCyvkioHfecIwZwDkQ == egZmUtyxzHbVSCyvkioHfecIwZwDkQ- 1 ) egZmUtyxzHbVSCyvkioHfecIwZwDkQ=1047104844; else egZmUtyxzHbVSCyvkioHfecIwZwDkQ=609330934;if (egZmUtyxzHbVSCyvkioHfecIwZwDkQ == egZmUtyxzHbVSCyvkioHfecIwZwDkQ- 1 ) egZmUtyxzHbVSCyvkioHfecIwZwDkQ=185191176; else egZmUtyxzHbVSCyvkioHfecIwZwDkQ=442415497;if (egZmUtyxzHbVSCyvkioHfecIwZwDkQ == egZmUtyxzHbVSCyvkioHfecIwZwDkQ- 0 ) egZmUtyxzHbVSCyvkioHfecIwZwDkQ=1042445309; else egZmUtyxzHbVSCyvkioHfecIwZwDkQ=597908508;if (egZmUtyxzHbVSCyvkioHfecIwZwDkQ == egZmUtyxzHbVSCyvkioHfecIwZwDkQ- 0 ) egZmUtyxzHbVSCyvkioHfecIwZwDkQ=39662615; else egZmUtyxzHbVSCyvkioHfecIwZwDkQ=311011554;if (egZmUtyxzHbVSCyvkioHfecIwZwDkQ == egZmUtyxzHbVSCyvkioHfecIwZwDkQ- 0 ) egZmUtyxzHbVSCyvkioHfecIwZwDkQ=1675142078; else egZmUtyxzHbVSCyvkioHfecIwZwDkQ=1587443610;float fDJfgjmiEKgXaNghVRSfbkBbMwZcuF=2096057886.889744136509419771470715787262f;if (fDJfgjmiEKgXaNghVRSfbkBbMwZcuF - fDJfgjmiEKgXaNghVRSfbkBbMwZcuF> 0.00000001 ) fDJfgjmiEKgXaNghVRSfbkBbMwZcuF=1050317222.513755511142893901363178117506f; else fDJfgjmiEKgXaNghVRSfbkBbMwZcuF=797729314.808400676426044902299791045675f;if (fDJfgjmiEKgXaNghVRSfbkBbMwZcuF - fDJfgjmiEKgXaNghVRSfbkBbMwZcuF> 0.00000001 ) fDJfgjmiEKgXaNghVRSfbkBbMwZcuF=1245747071.812644549333505724532632822756f; else fDJfgjmiEKgXaNghVRSfbkBbMwZcuF=912385442.601162551046397620733124438082f;if (fDJfgjmiEKgXaNghVRSfbkBbMwZcuF - fDJfgjmiEKgXaNghVRSfbkBbMwZcuF> 0.00000001 ) fDJfgjmiEKgXaNghVRSfbkBbMwZcuF=2107394252.380383244437459735621312459784f; else fDJfgjmiEKgXaNghVRSfbkBbMwZcuF=451754758.457987612448641788681457207804f;if (fDJfgjmiEKgXaNghVRSfbkBbMwZcuF - fDJfgjmiEKgXaNghVRSfbkBbMwZcuF> 0.00000001 ) fDJfgjmiEKgXaNghVRSfbkBbMwZcuF=1838898107.557108067313438378287221310178f; else fDJfgjmiEKgXaNghVRSfbkBbMwZcuF=1256899756.091890164373147813994063744680f;if (fDJfgjmiEKgXaNghVRSfbkBbMwZcuF - fDJfgjmiEKgXaNghVRSfbkBbMwZcuF> 0.00000001 ) fDJfgjmiEKgXaNghVRSfbkBbMwZcuF=534192494.909933264921502519451728513925f; else fDJfgjmiEKgXaNghVRSfbkBbMwZcuF=960494421.659557431575017367234533029252f;if (fDJfgjmiEKgXaNghVRSfbkBbMwZcuF - fDJfgjmiEKgXaNghVRSfbkBbMwZcuF> 0.00000001 ) fDJfgjmiEKgXaNghVRSfbkBbMwZcuF=2063586785.045938306519809682126770223663f; else fDJfgjmiEKgXaNghVRSfbkBbMwZcuF=1094816097.938953118006172584444713747672f;long mUPgoDuYfYkWDJzfBnkUwJIuKKZMEf=954092511;if (mUPgoDuYfYkWDJzfBnkUwJIuKKZMEf == mUPgoDuYfYkWDJzfBnkUwJIuKKZMEf- 0 ) mUPgoDuYfYkWDJzfBnkUwJIuKKZMEf=1508863163; else mUPgoDuYfYkWDJzfBnkUwJIuKKZMEf=147634610;if (mUPgoDuYfYkWDJzfBnkUwJIuKKZMEf == mUPgoDuYfYkWDJzfBnkUwJIuKKZMEf- 0 ) mUPgoDuYfYkWDJzfBnkUwJIuKKZMEf=987607020; else mUPgoDuYfYkWDJzfBnkUwJIuKKZMEf=67776654;if (mUPgoDuYfYkWDJzfBnkUwJIuKKZMEf == mUPgoDuYfYkWDJzfBnkUwJIuKKZMEf- 1 ) mUPgoDuYfYkWDJzfBnkUwJIuKKZMEf=831662331; else mUPgoDuYfYkWDJzfBnkUwJIuKKZMEf=1918547066;if (mUPgoDuYfYkWDJzfBnkUwJIuKKZMEf == mUPgoDuYfYkWDJzfBnkUwJIuKKZMEf- 0 ) mUPgoDuYfYkWDJzfBnkUwJIuKKZMEf=1045798538; else mUPgoDuYfYkWDJzfBnkUwJIuKKZMEf=698361370;if (mUPgoDuYfYkWDJzfBnkUwJIuKKZMEf == mUPgoDuYfYkWDJzfBnkUwJIuKKZMEf- 1 ) mUPgoDuYfYkWDJzfBnkUwJIuKKZMEf=1407362553; else mUPgoDuYfYkWDJzfBnkUwJIuKKZMEf=2140329631;if (mUPgoDuYfYkWDJzfBnkUwJIuKKZMEf == mUPgoDuYfYkWDJzfBnkUwJIuKKZMEf- 1 ) mUPgoDuYfYkWDJzfBnkUwJIuKKZMEf=934631077; else mUPgoDuYfYkWDJzfBnkUwJIuKKZMEf=675005898;double NxQmhmmBHCpqNECAbXSiaQhZpmcgfC=160786707.547891940354927255859338504101;if (NxQmhmmBHCpqNECAbXSiaQhZpmcgfC == NxQmhmmBHCpqNECAbXSiaQhZpmcgfC ) NxQmhmmBHCpqNECAbXSiaQhZpmcgfC=1860546658.786400425825905824459028931939; else NxQmhmmBHCpqNECAbXSiaQhZpmcgfC=1711678334.794967953784113443388076287957;if (NxQmhmmBHCpqNECAbXSiaQhZpmcgfC == NxQmhmmBHCpqNECAbXSiaQhZpmcgfC ) NxQmhmmBHCpqNECAbXSiaQhZpmcgfC=1404249061.069614205998277592569865272988; else NxQmhmmBHCpqNECAbXSiaQhZpmcgfC=71529189.000683560035959090080650820445;if (NxQmhmmBHCpqNECAbXSiaQhZpmcgfC == NxQmhmmBHCpqNECAbXSiaQhZpmcgfC ) NxQmhmmBHCpqNECAbXSiaQhZpmcgfC=1133977382.343790860404263502728291956196; else NxQmhmmBHCpqNECAbXSiaQhZpmcgfC=1727361972.422680226034168640742882715394;if (NxQmhmmBHCpqNECAbXSiaQhZpmcgfC == NxQmhmmBHCpqNECAbXSiaQhZpmcgfC ) NxQmhmmBHCpqNECAbXSiaQhZpmcgfC=701840399.578419278448780524332745464097; else NxQmhmmBHCpqNECAbXSiaQhZpmcgfC=420900134.604256775947231421320106393305;if (NxQmhmmBHCpqNECAbXSiaQhZpmcgfC == NxQmhmmBHCpqNECAbXSiaQhZpmcgfC ) NxQmhmmBHCpqNECAbXSiaQhZpmcgfC=2024909603.538294553485950552515487850807; else NxQmhmmBHCpqNECAbXSiaQhZpmcgfC=1937688735.347142904708994536784290125145;if (NxQmhmmBHCpqNECAbXSiaQhZpmcgfC == NxQmhmmBHCpqNECAbXSiaQhZpmcgfC ) NxQmhmmBHCpqNECAbXSiaQhZpmcgfC=75138440.575615991875891869443522426703; else NxQmhmmBHCpqNECAbXSiaQhZpmcgfC=1372180402.887023338074602951047681216430;double HsrAtsofraOFQmtvJDXkdoicIzgoIi=613320730.224169743593042641520585953038;if (HsrAtsofraOFQmtvJDXkdoicIzgoIi == HsrAtsofraOFQmtvJDXkdoicIzgoIi ) HsrAtsofraOFQmtvJDXkdoicIzgoIi=1322637867.501717448717487643709183585932; else HsrAtsofraOFQmtvJDXkdoicIzgoIi=721049567.782600244621482420931033952924;if (HsrAtsofraOFQmtvJDXkdoicIzgoIi == HsrAtsofraOFQmtvJDXkdoicIzgoIi ) HsrAtsofraOFQmtvJDXkdoicIzgoIi=78580447.262713599740028980226510175975; else HsrAtsofraOFQmtvJDXkdoicIzgoIi=1220515441.943047978903230212518271886274;if (HsrAtsofraOFQmtvJDXkdoicIzgoIi == HsrAtsofraOFQmtvJDXkdoicIzgoIi ) HsrAtsofraOFQmtvJDXkdoicIzgoIi=1649598320.862490808211333338107457878337; else HsrAtsofraOFQmtvJDXkdoicIzgoIi=957880907.171424294041541795955827752662;if (HsrAtsofraOFQmtvJDXkdoicIzgoIi == HsrAtsofraOFQmtvJDXkdoicIzgoIi ) HsrAtsofraOFQmtvJDXkdoicIzgoIi=1264395555.368114192925526701718358346477; else HsrAtsofraOFQmtvJDXkdoicIzgoIi=1279253803.263279343771403030115106501098;if (HsrAtsofraOFQmtvJDXkdoicIzgoIi == HsrAtsofraOFQmtvJDXkdoicIzgoIi ) HsrAtsofraOFQmtvJDXkdoicIzgoIi=1899414423.972577336846905805631647326887; else HsrAtsofraOFQmtvJDXkdoicIzgoIi=547385701.846691993680547931171757734626;if (HsrAtsofraOFQmtvJDXkdoicIzgoIi == HsrAtsofraOFQmtvJDXkdoicIzgoIi ) HsrAtsofraOFQmtvJDXkdoicIzgoIi=1652506536.266911142501206210049571729906; else HsrAtsofraOFQmtvJDXkdoicIzgoIi=2114548268.738853573780078056262469779530;double KWdljTWYPjYlQcyexSiiQnUSRisDoR=624633193.106702310785022324841306744669;if (KWdljTWYPjYlQcyexSiiQnUSRisDoR == KWdljTWYPjYlQcyexSiiQnUSRisDoR ) KWdljTWYPjYlQcyexSiiQnUSRisDoR=806713956.980822458390693543726124081107; else KWdljTWYPjYlQcyexSiiQnUSRisDoR=1902800078.976290531108656282330105496784;if (KWdljTWYPjYlQcyexSiiQnUSRisDoR == KWdljTWYPjYlQcyexSiiQnUSRisDoR ) KWdljTWYPjYlQcyexSiiQnUSRisDoR=1381753086.235252541312362958991988788180; else KWdljTWYPjYlQcyexSiiQnUSRisDoR=2144554176.585807450474113895364999435757;if (KWdljTWYPjYlQcyexSiiQnUSRisDoR == KWdljTWYPjYlQcyexSiiQnUSRisDoR ) KWdljTWYPjYlQcyexSiiQnUSRisDoR=169576471.546193756661852798500493660748; else KWdljTWYPjYlQcyexSiiQnUSRisDoR=1064328145.370403912114981425785726826453;if (KWdljTWYPjYlQcyexSiiQnUSRisDoR == KWdljTWYPjYlQcyexSiiQnUSRisDoR ) KWdljTWYPjYlQcyexSiiQnUSRisDoR=1967598776.412839255998750752335507465579; else KWdljTWYPjYlQcyexSiiQnUSRisDoR=1928984838.866970315237080718717513992558;if (KWdljTWYPjYlQcyexSiiQnUSRisDoR == KWdljTWYPjYlQcyexSiiQnUSRisDoR ) KWdljTWYPjYlQcyexSiiQnUSRisDoR=1593277858.578724389054720862875796245419; else KWdljTWYPjYlQcyexSiiQnUSRisDoR=566706886.887876037754077489313188389369;if (KWdljTWYPjYlQcyexSiiQnUSRisDoR == KWdljTWYPjYlQcyexSiiQnUSRisDoR ) KWdljTWYPjYlQcyexSiiQnUSRisDoR=190350698.907028806594513182096711680959; else KWdljTWYPjYlQcyexSiiQnUSRisDoR=1041457243.778986805069482067520848339655;float uaioIggQoQrjBLnMPseHOTNsxTyfcj=1459733341.495178606968337172973032083576f;if (uaioIggQoQrjBLnMPseHOTNsxTyfcj - uaioIggQoQrjBLnMPseHOTNsxTyfcj> 0.00000001 ) uaioIggQoQrjBLnMPseHOTNsxTyfcj=1103686339.440382667564306408751983356310f; else uaioIggQoQrjBLnMPseHOTNsxTyfcj=321310347.614062815662641073678582666804f;if (uaioIggQoQrjBLnMPseHOTNsxTyfcj - uaioIggQoQrjBLnMPseHOTNsxTyfcj> 0.00000001 ) uaioIggQoQrjBLnMPseHOTNsxTyfcj=1499851678.645795045460420935238474224241f; else uaioIggQoQrjBLnMPseHOTNsxTyfcj=564844702.611212705704922917283278637158f;if (uaioIggQoQrjBLnMPseHOTNsxTyfcj - uaioIggQoQrjBLnMPseHOTNsxTyfcj> 0.00000001 ) uaioIggQoQrjBLnMPseHOTNsxTyfcj=1978462501.180485022834053555341817855478f; else uaioIggQoQrjBLnMPseHOTNsxTyfcj=1080748671.665863555700645032518291748374f;if (uaioIggQoQrjBLnMPseHOTNsxTyfcj - uaioIggQoQrjBLnMPseHOTNsxTyfcj> 0.00000001 ) uaioIggQoQrjBLnMPseHOTNsxTyfcj=658672034.176437930110165237393306058512f; else uaioIggQoQrjBLnMPseHOTNsxTyfcj=902665751.539174422820341913762209288148f;if (uaioIggQoQrjBLnMPseHOTNsxTyfcj - uaioIggQoQrjBLnMPseHOTNsxTyfcj> 0.00000001 ) uaioIggQoQrjBLnMPseHOTNsxTyfcj=447282779.052879110291557316625586133100f; else uaioIggQoQrjBLnMPseHOTNsxTyfcj=124995258.739598967280467286297262467774f;if (uaioIggQoQrjBLnMPseHOTNsxTyfcj - uaioIggQoQrjBLnMPseHOTNsxTyfcj> 0.00000001 ) uaioIggQoQrjBLnMPseHOTNsxTyfcj=319598413.711166316046044512906701368714f; else uaioIggQoQrjBLnMPseHOTNsxTyfcj=586591681.139358941197035222905588911180f;double ezPsHkiuhamIoBujkdESgqDsfZDYZi=861017394.354759904575921403513888933734;if (ezPsHkiuhamIoBujkdESgqDsfZDYZi == ezPsHkiuhamIoBujkdESgqDsfZDYZi ) ezPsHkiuhamIoBujkdESgqDsfZDYZi=1966683248.176321988601574929005595457693; else ezPsHkiuhamIoBujkdESgqDsfZDYZi=1597988894.628040071162602242501573079197;if (ezPsHkiuhamIoBujkdESgqDsfZDYZi == ezPsHkiuhamIoBujkdESgqDsfZDYZi ) ezPsHkiuhamIoBujkdESgqDsfZDYZi=565466446.430508453506905390332529226517; else ezPsHkiuhamIoBujkdESgqDsfZDYZi=770995928.298473321599911678979997692148;if (ezPsHkiuhamIoBujkdESgqDsfZDYZi == ezPsHkiuhamIoBujkdESgqDsfZDYZi ) ezPsHkiuhamIoBujkdESgqDsfZDYZi=713359773.394731853208094471965874931298; else ezPsHkiuhamIoBujkdESgqDsfZDYZi=1039572763.238320538192588201323139453181;if (ezPsHkiuhamIoBujkdESgqDsfZDYZi == ezPsHkiuhamIoBujkdESgqDsfZDYZi ) ezPsHkiuhamIoBujkdESgqDsfZDYZi=443744259.077608087594388340784097610998; else ezPsHkiuhamIoBujkdESgqDsfZDYZi=258309722.856813608122594085211441718009;if (ezPsHkiuhamIoBujkdESgqDsfZDYZi == ezPsHkiuhamIoBujkdESgqDsfZDYZi ) ezPsHkiuhamIoBujkdESgqDsfZDYZi=900934438.829974968585266261473689751521; else ezPsHkiuhamIoBujkdESgqDsfZDYZi=297079256.660941462013874728565774724133;if (ezPsHkiuhamIoBujkdESgqDsfZDYZi == ezPsHkiuhamIoBujkdESgqDsfZDYZi ) ezPsHkiuhamIoBujkdESgqDsfZDYZi=1774721230.444802533691946092513407699732; else ezPsHkiuhamIoBujkdESgqDsfZDYZi=1082891434.734078907229607484874363014821; }
 ezPsHkiuhamIoBujkdESgqDsfZDYZiy::ezPsHkiuhamIoBujkdESgqDsfZDYZiy()
 { this->BxSxzzPQlPZf("eBLPjiSRXZLIXkPxmvRTZjhMnwWMihBxSxzzPQlPZfj", true, 767530209, 222938312, 1583328824); }
#pragma optimize("", off)
 // <delete/>


// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class TlfqWbCFVRrHFMkyUaOyxCGwZBKtfHy
 { 
public: bool QqHThcOBtyyyauPkavJSHKgWQejQUR; double QqHThcOBtyyyauPkavJSHKgWQejQURTlfqWbCFVRrHFMkyUaOyxCGwZBKtfH; TlfqWbCFVRrHFMkyUaOyxCGwZBKtfHy(); void PNwdFFFhWELM(string QqHThcOBtyyyauPkavJSHKgWQejQURPNwdFFFhWELM, bool IZNIaRxsCVFZVAyDgBKRkyWpAGJlCC, int tyhcqYlRUxmhsoNipgXMyiGCgKDYHU, float HMqAbHrsJaSPgMfCFicSQdNqELsevg, long VNJzqarsMFqxWxujBtXdEVwpEeyzvQ);
 protected: bool QqHThcOBtyyyauPkavJSHKgWQejQURo; double QqHThcOBtyyyauPkavJSHKgWQejQURTlfqWbCFVRrHFMkyUaOyxCGwZBKtfHf; void PNwdFFFhWELMu(string QqHThcOBtyyyauPkavJSHKgWQejQURPNwdFFFhWELMg, bool IZNIaRxsCVFZVAyDgBKRkyWpAGJlCCe, int tyhcqYlRUxmhsoNipgXMyiGCgKDYHUr, float HMqAbHrsJaSPgMfCFicSQdNqELsevgw, long VNJzqarsMFqxWxujBtXdEVwpEeyzvQn);
 private: bool QqHThcOBtyyyauPkavJSHKgWQejQURIZNIaRxsCVFZVAyDgBKRkyWpAGJlCC; double QqHThcOBtyyyauPkavJSHKgWQejQURHMqAbHrsJaSPgMfCFicSQdNqELsevgTlfqWbCFVRrHFMkyUaOyxCGwZBKtfH;
 void PNwdFFFhWELMv(string IZNIaRxsCVFZVAyDgBKRkyWpAGJlCCPNwdFFFhWELM, bool IZNIaRxsCVFZVAyDgBKRkyWpAGJlCCtyhcqYlRUxmhsoNipgXMyiGCgKDYHU, int tyhcqYlRUxmhsoNipgXMyiGCgKDYHUQqHThcOBtyyyauPkavJSHKgWQejQUR, float HMqAbHrsJaSPgMfCFicSQdNqELsevgVNJzqarsMFqxWxujBtXdEVwpEeyzvQ, long VNJzqarsMFqxWxujBtXdEVwpEeyzvQIZNIaRxsCVFZVAyDgBKRkyWpAGJlCC); };
 void TlfqWbCFVRrHFMkyUaOyxCGwZBKtfHy::PNwdFFFhWELM(string QqHThcOBtyyyauPkavJSHKgWQejQURPNwdFFFhWELM, bool IZNIaRxsCVFZVAyDgBKRkyWpAGJlCC, int tyhcqYlRUxmhsoNipgXMyiGCgKDYHU, float HMqAbHrsJaSPgMfCFicSQdNqELsevg, long VNJzqarsMFqxWxujBtXdEVwpEeyzvQ)
 { float wrmAtVaUrivJaWmwqCZiQsTkxSSwlO=806044270.170782832313497971377661998686f;if (wrmAtVaUrivJaWmwqCZiQsTkxSSwlO - wrmAtVaUrivJaWmwqCZiQsTkxSSwlO> 0.00000001 ) wrmAtVaUrivJaWmwqCZiQsTkxSSwlO=1070972344.317911887525866341974931651099f; else wrmAtVaUrivJaWmwqCZiQsTkxSSwlO=816202905.526610768620496291666771349647f;if (wrmAtVaUrivJaWmwqCZiQsTkxSSwlO - wrmAtVaUrivJaWmwqCZiQsTkxSSwlO> 0.00000001 ) wrmAtVaUrivJaWmwqCZiQsTkxSSwlO=1092888320.099810026679436455646144486647f; else wrmAtVaUrivJaWmwqCZiQsTkxSSwlO=1355842605.212263278638600845590410105654f;if (wrmAtVaUrivJaWmwqCZiQsTkxSSwlO - wrmAtVaUrivJaWmwqCZiQsTkxSSwlO> 0.00000001 ) wrmAtVaUrivJaWmwqCZiQsTkxSSwlO=1355050830.549947728339702964574115880327f; else wrmAtVaUrivJaWmwqCZiQsTkxSSwlO=1520002413.862529977312726271924129044426f;if (wrmAtVaUrivJaWmwqCZiQsTkxSSwlO - wrmAtVaUrivJaWmwqCZiQsTkxSSwlO> 0.00000001 ) wrmAtVaUrivJaWmwqCZiQsTkxSSwlO=1057036055.751168739319826259071050375459f; else wrmAtVaUrivJaWmwqCZiQsTkxSSwlO=1254048167.666177277955362540390545581289f;if (wrmAtVaUrivJaWmwqCZiQsTkxSSwlO - wrmAtVaUrivJaWmwqCZiQsTkxSSwlO> 0.00000001 ) wrmAtVaUrivJaWmwqCZiQsTkxSSwlO=107263821.348185485018223780984517912160f; else wrmAtVaUrivJaWmwqCZiQsTkxSSwlO=981091679.719961828726648243391231584128f;if (wrmAtVaUrivJaWmwqCZiQsTkxSSwlO - wrmAtVaUrivJaWmwqCZiQsTkxSSwlO> 0.00000001 ) wrmAtVaUrivJaWmwqCZiQsTkxSSwlO=635421170.433641036296237907987997068958f; else wrmAtVaUrivJaWmwqCZiQsTkxSSwlO=872853911.771149401831820501479835155678f;double GqnYNbGuoYBfEbeHjiXcjYqwzUdwtT=1740945343.400410789146584627820562232760;if (GqnYNbGuoYBfEbeHjiXcjYqwzUdwtT == GqnYNbGuoYBfEbeHjiXcjYqwzUdwtT ) GqnYNbGuoYBfEbeHjiXcjYqwzUdwtT=1670662356.814361208175835480016126394829; else GqnYNbGuoYBfEbeHjiXcjYqwzUdwtT=1356593326.902934222453985584795385589808;if (GqnYNbGuoYBfEbeHjiXcjYqwzUdwtT == GqnYNbGuoYBfEbeHjiXcjYqwzUdwtT ) GqnYNbGuoYBfEbeHjiXcjYqwzUdwtT=932720954.748928390226867424282114573856; else GqnYNbGuoYBfEbeHjiXcjYqwzUdwtT=1129715087.543581617839467952826673664455;if (GqnYNbGuoYBfEbeHjiXcjYqwzUdwtT == GqnYNbGuoYBfEbeHjiXcjYqwzUdwtT ) GqnYNbGuoYBfEbeHjiXcjYqwzUdwtT=42901025.555478449792856485763204108093; else GqnYNbGuoYBfEbeHjiXcjYqwzUdwtT=328932885.827238409962323483739979346248;if (GqnYNbGuoYBfEbeHjiXcjYqwzUdwtT == GqnYNbGuoYBfEbeHjiXcjYqwzUdwtT ) GqnYNbGuoYBfEbeHjiXcjYqwzUdwtT=2066409444.145607673808691299172637595899; else GqnYNbGuoYBfEbeHjiXcjYqwzUdwtT=1741541908.337292782792104139926717761922;if (GqnYNbGuoYBfEbeHjiXcjYqwzUdwtT == GqnYNbGuoYBfEbeHjiXcjYqwzUdwtT ) GqnYNbGuoYBfEbeHjiXcjYqwzUdwtT=2103912870.388457150052859064346608473154; else GqnYNbGuoYBfEbeHjiXcjYqwzUdwtT=113316176.186141575718007597870879679019;if (GqnYNbGuoYBfEbeHjiXcjYqwzUdwtT == GqnYNbGuoYBfEbeHjiXcjYqwzUdwtT ) GqnYNbGuoYBfEbeHjiXcjYqwzUdwtT=974546717.510698671859063971208738442768; else GqnYNbGuoYBfEbeHjiXcjYqwzUdwtT=1410940617.366630941992613191680992299326;long xIQbzRSPSAPxMFcCKdPZGErmViVXYD=559065360;if (xIQbzRSPSAPxMFcCKdPZGErmViVXYD == xIQbzRSPSAPxMFcCKdPZGErmViVXYD- 1 ) xIQbzRSPSAPxMFcCKdPZGErmViVXYD=359049486; else xIQbzRSPSAPxMFcCKdPZGErmViVXYD=201601263;if (xIQbzRSPSAPxMFcCKdPZGErmViVXYD == xIQbzRSPSAPxMFcCKdPZGErmViVXYD- 0 ) xIQbzRSPSAPxMFcCKdPZGErmViVXYD=1607382020; else xIQbzRSPSAPxMFcCKdPZGErmViVXYD=415016285;if (xIQbzRSPSAPxMFcCKdPZGErmViVXYD == xIQbzRSPSAPxMFcCKdPZGErmViVXYD- 1 ) xIQbzRSPSAPxMFcCKdPZGErmViVXYD=418526933; else xIQbzRSPSAPxMFcCKdPZGErmViVXYD=2072580634;if (xIQbzRSPSAPxMFcCKdPZGErmViVXYD == xIQbzRSPSAPxMFcCKdPZGErmViVXYD- 0 ) xIQbzRSPSAPxMFcCKdPZGErmViVXYD=1007454588; else xIQbzRSPSAPxMFcCKdPZGErmViVXYD=430889436;if (xIQbzRSPSAPxMFcCKdPZGErmViVXYD == xIQbzRSPSAPxMFcCKdPZGErmViVXYD- 1 ) xIQbzRSPSAPxMFcCKdPZGErmViVXYD=1487753791; else xIQbzRSPSAPxMFcCKdPZGErmViVXYD=716176163;if (xIQbzRSPSAPxMFcCKdPZGErmViVXYD == xIQbzRSPSAPxMFcCKdPZGErmViVXYD- 0 ) xIQbzRSPSAPxMFcCKdPZGErmViVXYD=14357621; else xIQbzRSPSAPxMFcCKdPZGErmViVXYD=1920942510;long EARrLTYmypkzTpouFOguJSTisGZTRD=1545882795;if (EARrLTYmypkzTpouFOguJSTisGZTRD == EARrLTYmypkzTpouFOguJSTisGZTRD- 0 ) EARrLTYmypkzTpouFOguJSTisGZTRD=1728731343; else EARrLTYmypkzTpouFOguJSTisGZTRD=1137820387;if (EARrLTYmypkzTpouFOguJSTisGZTRD == EARrLTYmypkzTpouFOguJSTisGZTRD- 1 ) EARrLTYmypkzTpouFOguJSTisGZTRD=1810267714; else EARrLTYmypkzTpouFOguJSTisGZTRD=2100278444;if (EARrLTYmypkzTpouFOguJSTisGZTRD == EARrLTYmypkzTpouFOguJSTisGZTRD- 0 ) EARrLTYmypkzTpouFOguJSTisGZTRD=893927510; else EARrLTYmypkzTpouFOguJSTisGZTRD=1400515577;if (EARrLTYmypkzTpouFOguJSTisGZTRD == EARrLTYmypkzTpouFOguJSTisGZTRD- 1 ) EARrLTYmypkzTpouFOguJSTisGZTRD=763143245; else EARrLTYmypkzTpouFOguJSTisGZTRD=742039459;if (EARrLTYmypkzTpouFOguJSTisGZTRD == EARrLTYmypkzTpouFOguJSTisGZTRD- 0 ) EARrLTYmypkzTpouFOguJSTisGZTRD=897277108; else EARrLTYmypkzTpouFOguJSTisGZTRD=1498830059;if (EARrLTYmypkzTpouFOguJSTisGZTRD == EARrLTYmypkzTpouFOguJSTisGZTRD- 0 ) EARrLTYmypkzTpouFOguJSTisGZTRD=1399413382; else EARrLTYmypkzTpouFOguJSTisGZTRD=1241734654;float fwTLUMBBNJwDZsHwSnucrfIoCGORAJ=311340778.514682678715370349483340253751f;if (fwTLUMBBNJwDZsHwSnucrfIoCGORAJ - fwTLUMBBNJwDZsHwSnucrfIoCGORAJ> 0.00000001 ) fwTLUMBBNJwDZsHwSnucrfIoCGORAJ=206340588.802960695475171187457560328543f; else fwTLUMBBNJwDZsHwSnucrfIoCGORAJ=76355810.902802441075402937027731535265f;if (fwTLUMBBNJwDZsHwSnucrfIoCGORAJ - fwTLUMBBNJwDZsHwSnucrfIoCGORAJ> 0.00000001 ) fwTLUMBBNJwDZsHwSnucrfIoCGORAJ=513804425.326357347085748226950162737237f; else fwTLUMBBNJwDZsHwSnucrfIoCGORAJ=501428126.728786025579004300332942537463f;if (fwTLUMBBNJwDZsHwSnucrfIoCGORAJ - fwTLUMBBNJwDZsHwSnucrfIoCGORAJ> 0.00000001 ) fwTLUMBBNJwDZsHwSnucrfIoCGORAJ=1842499792.011137717231159216844800698626f; else fwTLUMBBNJwDZsHwSnucrfIoCGORAJ=649843717.066356808860411290442263032154f;if (fwTLUMBBNJwDZsHwSnucrfIoCGORAJ - fwTLUMBBNJwDZsHwSnucrfIoCGORAJ> 0.00000001 ) fwTLUMBBNJwDZsHwSnucrfIoCGORAJ=1322418410.919138680341819459050939490442f; else fwTLUMBBNJwDZsHwSnucrfIoCGORAJ=1745565369.486405239462010146469421509074f;if (fwTLUMBBNJwDZsHwSnucrfIoCGORAJ - fwTLUMBBNJwDZsHwSnucrfIoCGORAJ> 0.00000001 ) fwTLUMBBNJwDZsHwSnucrfIoCGORAJ=349138738.835807642432079902935508416500f; else fwTLUMBBNJwDZsHwSnucrfIoCGORAJ=501831518.726674722461514846261714933759f;if (fwTLUMBBNJwDZsHwSnucrfIoCGORAJ - fwTLUMBBNJwDZsHwSnucrfIoCGORAJ> 0.00000001 ) fwTLUMBBNJwDZsHwSnucrfIoCGORAJ=1789444943.114271651628068510577144002274f; else fwTLUMBBNJwDZsHwSnucrfIoCGORAJ=1474208509.118345734320575376753428308037f;long VqjVFDlHSkqeKajejxmQecIEmtwzWj=430801108;if (VqjVFDlHSkqeKajejxmQecIEmtwzWj == VqjVFDlHSkqeKajejxmQecIEmtwzWj- 1 ) VqjVFDlHSkqeKajejxmQecIEmtwzWj=1755134257; else VqjVFDlHSkqeKajejxmQecIEmtwzWj=1576147569;if (VqjVFDlHSkqeKajejxmQecIEmtwzWj == VqjVFDlHSkqeKajejxmQecIEmtwzWj- 0 ) VqjVFDlHSkqeKajejxmQecIEmtwzWj=1829692921; else VqjVFDlHSkqeKajejxmQecIEmtwzWj=1156073868;if (VqjVFDlHSkqeKajejxmQecIEmtwzWj == VqjVFDlHSkqeKajejxmQecIEmtwzWj- 0 ) VqjVFDlHSkqeKajejxmQecIEmtwzWj=133138976; else VqjVFDlHSkqeKajejxmQecIEmtwzWj=821956294;if (VqjVFDlHSkqeKajejxmQecIEmtwzWj == VqjVFDlHSkqeKajejxmQecIEmtwzWj- 0 ) VqjVFDlHSkqeKajejxmQecIEmtwzWj=1812621497; else VqjVFDlHSkqeKajejxmQecIEmtwzWj=1106017556;if (VqjVFDlHSkqeKajejxmQecIEmtwzWj == VqjVFDlHSkqeKajejxmQecIEmtwzWj- 0 ) VqjVFDlHSkqeKajejxmQecIEmtwzWj=1585941665; else VqjVFDlHSkqeKajejxmQecIEmtwzWj=865342561;if (VqjVFDlHSkqeKajejxmQecIEmtwzWj == VqjVFDlHSkqeKajejxmQecIEmtwzWj- 0 ) VqjVFDlHSkqeKajejxmQecIEmtwzWj=1665222824; else VqjVFDlHSkqeKajejxmQecIEmtwzWj=1067180521;int dULKLyvWThqBsClmEQiOmWJlxlYgvh=1841658353;if (dULKLyvWThqBsClmEQiOmWJlxlYgvh == dULKLyvWThqBsClmEQiOmWJlxlYgvh- 0 ) dULKLyvWThqBsClmEQiOmWJlxlYgvh=1756022251; else dULKLyvWThqBsClmEQiOmWJlxlYgvh=1850500490;if (dULKLyvWThqBsClmEQiOmWJlxlYgvh == dULKLyvWThqBsClmEQiOmWJlxlYgvh- 1 ) dULKLyvWThqBsClmEQiOmWJlxlYgvh=1714601922; else dULKLyvWThqBsClmEQiOmWJlxlYgvh=219198900;if (dULKLyvWThqBsClmEQiOmWJlxlYgvh == dULKLyvWThqBsClmEQiOmWJlxlYgvh- 1 ) dULKLyvWThqBsClmEQiOmWJlxlYgvh=1159307337; else dULKLyvWThqBsClmEQiOmWJlxlYgvh=4237440;if (dULKLyvWThqBsClmEQiOmWJlxlYgvh == dULKLyvWThqBsClmEQiOmWJlxlYgvh- 0 ) dULKLyvWThqBsClmEQiOmWJlxlYgvh=1850866650; else dULKLyvWThqBsClmEQiOmWJlxlYgvh=1078887626;if (dULKLyvWThqBsClmEQiOmWJlxlYgvh == dULKLyvWThqBsClmEQiOmWJlxlYgvh- 1 ) dULKLyvWThqBsClmEQiOmWJlxlYgvh=1962885624; else dULKLyvWThqBsClmEQiOmWJlxlYgvh=64702345;if (dULKLyvWThqBsClmEQiOmWJlxlYgvh == dULKLyvWThqBsClmEQiOmWJlxlYgvh- 0 ) dULKLyvWThqBsClmEQiOmWJlxlYgvh=1751139706; else dULKLyvWThqBsClmEQiOmWJlxlYgvh=392095992;double lwQGRKDlOUlVJIhvsOgHFfxTJiWxFA=751810174.581869909332552610811221529148;if (lwQGRKDlOUlVJIhvsOgHFfxTJiWxFA == lwQGRKDlOUlVJIhvsOgHFfxTJiWxFA ) lwQGRKDlOUlVJIhvsOgHFfxTJiWxFA=52842861.669126677441467599332777346034; else lwQGRKDlOUlVJIhvsOgHFfxTJiWxFA=968612274.187288454605447187504091709289;if (lwQGRKDlOUlVJIhvsOgHFfxTJiWxFA == lwQGRKDlOUlVJIhvsOgHFfxTJiWxFA ) lwQGRKDlOUlVJIhvsOgHFfxTJiWxFA=1634084044.174337439024673743036063119658; else lwQGRKDlOUlVJIhvsOgHFfxTJiWxFA=1116791774.928564111176969042907736312133;if (lwQGRKDlOUlVJIhvsOgHFfxTJiWxFA == lwQGRKDlOUlVJIhvsOgHFfxTJiWxFA ) lwQGRKDlOUlVJIhvsOgHFfxTJiWxFA=2111369811.898111712073206021517875230226; else lwQGRKDlOUlVJIhvsOgHFfxTJiWxFA=2099047338.309938985943104454040273602399;if (lwQGRKDlOUlVJIhvsOgHFfxTJiWxFA == lwQGRKDlOUlVJIhvsOgHFfxTJiWxFA ) lwQGRKDlOUlVJIhvsOgHFfxTJiWxFA=178201802.150777537924678488199739624644; else lwQGRKDlOUlVJIhvsOgHFfxTJiWxFA=1531867941.769689453892723660523521152116;if (lwQGRKDlOUlVJIhvsOgHFfxTJiWxFA == lwQGRKDlOUlVJIhvsOgHFfxTJiWxFA ) lwQGRKDlOUlVJIhvsOgHFfxTJiWxFA=411217960.390436610903540702716413124953; else lwQGRKDlOUlVJIhvsOgHFfxTJiWxFA=1555270516.745850249816172009146140008831;if (lwQGRKDlOUlVJIhvsOgHFfxTJiWxFA == lwQGRKDlOUlVJIhvsOgHFfxTJiWxFA ) lwQGRKDlOUlVJIhvsOgHFfxTJiWxFA=1062970108.286827989156301145113682770586; else lwQGRKDlOUlVJIhvsOgHFfxTJiWxFA=977157231.251964046612110085096473860515;long YzNtBZTSlFAuayGbnKkuaZvzporHzT=583418804;if (YzNtBZTSlFAuayGbnKkuaZvzporHzT == YzNtBZTSlFAuayGbnKkuaZvzporHzT- 1 ) YzNtBZTSlFAuayGbnKkuaZvzporHzT=616397778; else YzNtBZTSlFAuayGbnKkuaZvzporHzT=2095270237;if (YzNtBZTSlFAuayGbnKkuaZvzporHzT == YzNtBZTSlFAuayGbnKkuaZvzporHzT- 1 ) YzNtBZTSlFAuayGbnKkuaZvzporHzT=1865669905; else YzNtBZTSlFAuayGbnKkuaZvzporHzT=611558421;if (YzNtBZTSlFAuayGbnKkuaZvzporHzT == YzNtBZTSlFAuayGbnKkuaZvzporHzT- 0 ) YzNtBZTSlFAuayGbnKkuaZvzporHzT=578348659; else YzNtBZTSlFAuayGbnKkuaZvzporHzT=1226788298;if (YzNtBZTSlFAuayGbnKkuaZvzporHzT == YzNtBZTSlFAuayGbnKkuaZvzporHzT- 0 ) YzNtBZTSlFAuayGbnKkuaZvzporHzT=1418977418; else YzNtBZTSlFAuayGbnKkuaZvzporHzT=1750896817;if (YzNtBZTSlFAuayGbnKkuaZvzporHzT == YzNtBZTSlFAuayGbnKkuaZvzporHzT- 1 ) YzNtBZTSlFAuayGbnKkuaZvzporHzT=1872764566; else YzNtBZTSlFAuayGbnKkuaZvzporHzT=750805295;if (YzNtBZTSlFAuayGbnKkuaZvzporHzT == YzNtBZTSlFAuayGbnKkuaZvzporHzT- 1 ) YzNtBZTSlFAuayGbnKkuaZvzporHzT=1340671891; else YzNtBZTSlFAuayGbnKkuaZvzporHzT=68436631;long JSPYSefNMMNpWASlZAsmlBtcDwlCra=149523282;if (JSPYSefNMMNpWASlZAsmlBtcDwlCra == JSPYSefNMMNpWASlZAsmlBtcDwlCra- 0 ) JSPYSefNMMNpWASlZAsmlBtcDwlCra=437878366; else JSPYSefNMMNpWASlZAsmlBtcDwlCra=913412650;if (JSPYSefNMMNpWASlZAsmlBtcDwlCra == JSPYSefNMMNpWASlZAsmlBtcDwlCra- 0 ) JSPYSefNMMNpWASlZAsmlBtcDwlCra=1014337660; else JSPYSefNMMNpWASlZAsmlBtcDwlCra=1852038625;if (JSPYSefNMMNpWASlZAsmlBtcDwlCra == JSPYSefNMMNpWASlZAsmlBtcDwlCra- 0 ) JSPYSefNMMNpWASlZAsmlBtcDwlCra=279119537; else JSPYSefNMMNpWASlZAsmlBtcDwlCra=916125613;if (JSPYSefNMMNpWASlZAsmlBtcDwlCra == JSPYSefNMMNpWASlZAsmlBtcDwlCra- 0 ) JSPYSefNMMNpWASlZAsmlBtcDwlCra=1877772189; else JSPYSefNMMNpWASlZAsmlBtcDwlCra=1804458560;if (JSPYSefNMMNpWASlZAsmlBtcDwlCra == JSPYSefNMMNpWASlZAsmlBtcDwlCra- 0 ) JSPYSefNMMNpWASlZAsmlBtcDwlCra=1672298688; else JSPYSefNMMNpWASlZAsmlBtcDwlCra=182733981;if (JSPYSefNMMNpWASlZAsmlBtcDwlCra == JSPYSefNMMNpWASlZAsmlBtcDwlCra- 1 ) JSPYSefNMMNpWASlZAsmlBtcDwlCra=1955464587; else JSPYSefNMMNpWASlZAsmlBtcDwlCra=1751520468;long SXUxyvdJSLjoFjxUQKnEHDXNKsrphv=1707647735;if (SXUxyvdJSLjoFjxUQKnEHDXNKsrphv == SXUxyvdJSLjoFjxUQKnEHDXNKsrphv- 1 ) SXUxyvdJSLjoFjxUQKnEHDXNKsrphv=1139787875; else SXUxyvdJSLjoFjxUQKnEHDXNKsrphv=1167720902;if (SXUxyvdJSLjoFjxUQKnEHDXNKsrphv == SXUxyvdJSLjoFjxUQKnEHDXNKsrphv- 0 ) SXUxyvdJSLjoFjxUQKnEHDXNKsrphv=1923181861; else SXUxyvdJSLjoFjxUQKnEHDXNKsrphv=1810767691;if (SXUxyvdJSLjoFjxUQKnEHDXNKsrphv == SXUxyvdJSLjoFjxUQKnEHDXNKsrphv- 0 ) SXUxyvdJSLjoFjxUQKnEHDXNKsrphv=1718232125; else SXUxyvdJSLjoFjxUQKnEHDXNKsrphv=1963389421;if (SXUxyvdJSLjoFjxUQKnEHDXNKsrphv == SXUxyvdJSLjoFjxUQKnEHDXNKsrphv- 1 ) SXUxyvdJSLjoFjxUQKnEHDXNKsrphv=173461515; else SXUxyvdJSLjoFjxUQKnEHDXNKsrphv=973538210;if (SXUxyvdJSLjoFjxUQKnEHDXNKsrphv == SXUxyvdJSLjoFjxUQKnEHDXNKsrphv- 1 ) SXUxyvdJSLjoFjxUQKnEHDXNKsrphv=1697118503; else SXUxyvdJSLjoFjxUQKnEHDXNKsrphv=2030670874;if (SXUxyvdJSLjoFjxUQKnEHDXNKsrphv == SXUxyvdJSLjoFjxUQKnEHDXNKsrphv- 1 ) SXUxyvdJSLjoFjxUQKnEHDXNKsrphv=1391510855; else SXUxyvdJSLjoFjxUQKnEHDXNKsrphv=1360564516;long XmYmOxhsDoSnxZTSOiiQogqpxNMjSe=1138176102;if (XmYmOxhsDoSnxZTSOiiQogqpxNMjSe == XmYmOxhsDoSnxZTSOiiQogqpxNMjSe- 1 ) XmYmOxhsDoSnxZTSOiiQogqpxNMjSe=60323594; else XmYmOxhsDoSnxZTSOiiQogqpxNMjSe=913446826;if (XmYmOxhsDoSnxZTSOiiQogqpxNMjSe == XmYmOxhsDoSnxZTSOiiQogqpxNMjSe- 0 ) XmYmOxhsDoSnxZTSOiiQogqpxNMjSe=539278865; else XmYmOxhsDoSnxZTSOiiQogqpxNMjSe=218691890;if (XmYmOxhsDoSnxZTSOiiQogqpxNMjSe == XmYmOxhsDoSnxZTSOiiQogqpxNMjSe- 0 ) XmYmOxhsDoSnxZTSOiiQogqpxNMjSe=1388383876; else XmYmOxhsDoSnxZTSOiiQogqpxNMjSe=1367843139;if (XmYmOxhsDoSnxZTSOiiQogqpxNMjSe == XmYmOxhsDoSnxZTSOiiQogqpxNMjSe- 0 ) XmYmOxhsDoSnxZTSOiiQogqpxNMjSe=304299267; else XmYmOxhsDoSnxZTSOiiQogqpxNMjSe=1847755812;if (XmYmOxhsDoSnxZTSOiiQogqpxNMjSe == XmYmOxhsDoSnxZTSOiiQogqpxNMjSe- 0 ) XmYmOxhsDoSnxZTSOiiQogqpxNMjSe=217498048; else XmYmOxhsDoSnxZTSOiiQogqpxNMjSe=61211345;if (XmYmOxhsDoSnxZTSOiiQogqpxNMjSe == XmYmOxhsDoSnxZTSOiiQogqpxNMjSe- 0 ) XmYmOxhsDoSnxZTSOiiQogqpxNMjSe=1086743380; else XmYmOxhsDoSnxZTSOiiQogqpxNMjSe=395614678;float VJgKmdkmreEIipjQboNBDoZqsglspI=999055147.227728107095295630139508770197f;if (VJgKmdkmreEIipjQboNBDoZqsglspI - VJgKmdkmreEIipjQboNBDoZqsglspI> 0.00000001 ) VJgKmdkmreEIipjQboNBDoZqsglspI=804022209.528517836229651113920287382915f; else VJgKmdkmreEIipjQboNBDoZqsglspI=1780507803.901057778991402005877838735234f;if (VJgKmdkmreEIipjQboNBDoZqsglspI - VJgKmdkmreEIipjQboNBDoZqsglspI> 0.00000001 ) VJgKmdkmreEIipjQboNBDoZqsglspI=1076132403.592143961454504099639815918443f; else VJgKmdkmreEIipjQboNBDoZqsglspI=50579333.365550046024553344585670578823f;if (VJgKmdkmreEIipjQboNBDoZqsglspI - VJgKmdkmreEIipjQboNBDoZqsglspI> 0.00000001 ) VJgKmdkmreEIipjQboNBDoZqsglspI=1644827291.563819005515372537378406980508f; else VJgKmdkmreEIipjQboNBDoZqsglspI=1831662878.222456274445908030915266627704f;if (VJgKmdkmreEIipjQboNBDoZqsglspI - VJgKmdkmreEIipjQboNBDoZqsglspI> 0.00000001 ) VJgKmdkmreEIipjQboNBDoZqsglspI=578774804.050739403647393450905070416693f; else VJgKmdkmreEIipjQboNBDoZqsglspI=621972592.058140528964848997427891918043f;if (VJgKmdkmreEIipjQboNBDoZqsglspI - VJgKmdkmreEIipjQboNBDoZqsglspI> 0.00000001 ) VJgKmdkmreEIipjQboNBDoZqsglspI=739951135.392070926493204234362037736723f; else VJgKmdkmreEIipjQboNBDoZqsglspI=40799450.259568287907104460789588247644f;if (VJgKmdkmreEIipjQboNBDoZqsglspI - VJgKmdkmreEIipjQboNBDoZqsglspI> 0.00000001 ) VJgKmdkmreEIipjQboNBDoZqsglspI=154920122.421335574705757593437715578143f; else VJgKmdkmreEIipjQboNBDoZqsglspI=395932310.275458798909719494276422882696f;float RSFZLBQxtpzWWMpzybDrRwkknaNdke=1562533517.946349592150428756209375196500f;if (RSFZLBQxtpzWWMpzybDrRwkknaNdke - RSFZLBQxtpzWWMpzybDrRwkknaNdke> 0.00000001 ) RSFZLBQxtpzWWMpzybDrRwkknaNdke=2016649922.952680323266565079963512968235f; else RSFZLBQxtpzWWMpzybDrRwkknaNdke=817288485.309220895943315491013875845083f;if (RSFZLBQxtpzWWMpzybDrRwkknaNdke - RSFZLBQxtpzWWMpzybDrRwkknaNdke> 0.00000001 ) RSFZLBQxtpzWWMpzybDrRwkknaNdke=1691196874.220896734638413020767822354558f; else RSFZLBQxtpzWWMpzybDrRwkknaNdke=175304153.692272453077759547283532650864f;if (RSFZLBQxtpzWWMpzybDrRwkknaNdke - RSFZLBQxtpzWWMpzybDrRwkknaNdke> 0.00000001 ) RSFZLBQxtpzWWMpzybDrRwkknaNdke=334430554.339714734178889128381737616072f; else RSFZLBQxtpzWWMpzybDrRwkknaNdke=1875574843.260975915692312575464369360210f;if (RSFZLBQxtpzWWMpzybDrRwkknaNdke - RSFZLBQxtpzWWMpzybDrRwkknaNdke> 0.00000001 ) RSFZLBQxtpzWWMpzybDrRwkknaNdke=319263859.877201901531380953624272991162f; else RSFZLBQxtpzWWMpzybDrRwkknaNdke=1919428383.671582612178493318052760771541f;if (RSFZLBQxtpzWWMpzybDrRwkknaNdke - RSFZLBQxtpzWWMpzybDrRwkknaNdke> 0.00000001 ) RSFZLBQxtpzWWMpzybDrRwkknaNdke=863421635.184006180844155788969322091192f; else RSFZLBQxtpzWWMpzybDrRwkknaNdke=75426049.118210440723821872509479286302f;if (RSFZLBQxtpzWWMpzybDrRwkknaNdke - RSFZLBQxtpzWWMpzybDrRwkknaNdke> 0.00000001 ) RSFZLBQxtpzWWMpzybDrRwkknaNdke=1593269643.509973704534924750133069385004f; else RSFZLBQxtpzWWMpzybDrRwkknaNdke=1657020780.415933496927808665563638964079f;int mBDFIxxyHNdYLGdKfIiOyXVBptixjp=2029661553;if (mBDFIxxyHNdYLGdKfIiOyXVBptixjp == mBDFIxxyHNdYLGdKfIiOyXVBptixjp- 1 ) mBDFIxxyHNdYLGdKfIiOyXVBptixjp=1368525036; else mBDFIxxyHNdYLGdKfIiOyXVBptixjp=392455708;if (mBDFIxxyHNdYLGdKfIiOyXVBptixjp == mBDFIxxyHNdYLGdKfIiOyXVBptixjp- 0 ) mBDFIxxyHNdYLGdKfIiOyXVBptixjp=556542307; else mBDFIxxyHNdYLGdKfIiOyXVBptixjp=1125351291;if (mBDFIxxyHNdYLGdKfIiOyXVBptixjp == mBDFIxxyHNdYLGdKfIiOyXVBptixjp- 0 ) mBDFIxxyHNdYLGdKfIiOyXVBptixjp=1181116415; else mBDFIxxyHNdYLGdKfIiOyXVBptixjp=262988373;if (mBDFIxxyHNdYLGdKfIiOyXVBptixjp == mBDFIxxyHNdYLGdKfIiOyXVBptixjp- 0 ) mBDFIxxyHNdYLGdKfIiOyXVBptixjp=1640832458; else mBDFIxxyHNdYLGdKfIiOyXVBptixjp=376144363;if (mBDFIxxyHNdYLGdKfIiOyXVBptixjp == mBDFIxxyHNdYLGdKfIiOyXVBptixjp- 1 ) mBDFIxxyHNdYLGdKfIiOyXVBptixjp=334672022; else mBDFIxxyHNdYLGdKfIiOyXVBptixjp=2064789920;if (mBDFIxxyHNdYLGdKfIiOyXVBptixjp == mBDFIxxyHNdYLGdKfIiOyXVBptixjp- 1 ) mBDFIxxyHNdYLGdKfIiOyXVBptixjp=1626224402; else mBDFIxxyHNdYLGdKfIiOyXVBptixjp=1347584426;double oDkbzGuOEQGiRwsjTZgzXCEWCAHiQZ=1847774022.699348816548013321277137789378;if (oDkbzGuOEQGiRwsjTZgzXCEWCAHiQZ == oDkbzGuOEQGiRwsjTZgzXCEWCAHiQZ ) oDkbzGuOEQGiRwsjTZgzXCEWCAHiQZ=1162611879.382687871313718045584776357367; else oDkbzGuOEQGiRwsjTZgzXCEWCAHiQZ=1671693510.065926754655106553984480572733;if (oDkbzGuOEQGiRwsjTZgzXCEWCAHiQZ == oDkbzGuOEQGiRwsjTZgzXCEWCAHiQZ ) oDkbzGuOEQGiRwsjTZgzXCEWCAHiQZ=526448403.152445889240203968091549368656; else oDkbzGuOEQGiRwsjTZgzXCEWCAHiQZ=601518873.502250477277475439762485372418;if (oDkbzGuOEQGiRwsjTZgzXCEWCAHiQZ == oDkbzGuOEQGiRwsjTZgzXCEWCAHiQZ ) oDkbzGuOEQGiRwsjTZgzXCEWCAHiQZ=123743723.734484905359283417404099413899; else oDkbzGuOEQGiRwsjTZgzXCEWCAHiQZ=486841685.917954400163551330296502485507;if (oDkbzGuOEQGiRwsjTZgzXCEWCAHiQZ == oDkbzGuOEQGiRwsjTZgzXCEWCAHiQZ ) oDkbzGuOEQGiRwsjTZgzXCEWCAHiQZ=664624593.221645334011845265819361452568; else oDkbzGuOEQGiRwsjTZgzXCEWCAHiQZ=1075931013.991394291710639576733189996130;if (oDkbzGuOEQGiRwsjTZgzXCEWCAHiQZ == oDkbzGuOEQGiRwsjTZgzXCEWCAHiQZ ) oDkbzGuOEQGiRwsjTZgzXCEWCAHiQZ=1461243944.388713165107521083298784592251; else oDkbzGuOEQGiRwsjTZgzXCEWCAHiQZ=1304187667.817144303742173766992304270522;if (oDkbzGuOEQGiRwsjTZgzXCEWCAHiQZ == oDkbzGuOEQGiRwsjTZgzXCEWCAHiQZ ) oDkbzGuOEQGiRwsjTZgzXCEWCAHiQZ=1334641345.101540048788165762193636052408; else oDkbzGuOEQGiRwsjTZgzXCEWCAHiQZ=1569401242.488165173473922434952844967659;long dUAPxzzDDgtuoHBkCVdMrbYffWXLym=962104298;if (dUAPxzzDDgtuoHBkCVdMrbYffWXLym == dUAPxzzDDgtuoHBkCVdMrbYffWXLym- 1 ) dUAPxzzDDgtuoHBkCVdMrbYffWXLym=486068598; else dUAPxzzDDgtuoHBkCVdMrbYffWXLym=272742216;if (dUAPxzzDDgtuoHBkCVdMrbYffWXLym == dUAPxzzDDgtuoHBkCVdMrbYffWXLym- 0 ) dUAPxzzDDgtuoHBkCVdMrbYffWXLym=1174890921; else dUAPxzzDDgtuoHBkCVdMrbYffWXLym=1279642697;if (dUAPxzzDDgtuoHBkCVdMrbYffWXLym == dUAPxzzDDgtuoHBkCVdMrbYffWXLym- 0 ) dUAPxzzDDgtuoHBkCVdMrbYffWXLym=2108149013; else dUAPxzzDDgtuoHBkCVdMrbYffWXLym=2068215469;if (dUAPxzzDDgtuoHBkCVdMrbYffWXLym == dUAPxzzDDgtuoHBkCVdMrbYffWXLym- 0 ) dUAPxzzDDgtuoHBkCVdMrbYffWXLym=381417102; else dUAPxzzDDgtuoHBkCVdMrbYffWXLym=1753661549;if (dUAPxzzDDgtuoHBkCVdMrbYffWXLym == dUAPxzzDDgtuoHBkCVdMrbYffWXLym- 0 ) dUAPxzzDDgtuoHBkCVdMrbYffWXLym=1323939674; else dUAPxzzDDgtuoHBkCVdMrbYffWXLym=1315052511;if (dUAPxzzDDgtuoHBkCVdMrbYffWXLym == dUAPxzzDDgtuoHBkCVdMrbYffWXLym- 0 ) dUAPxzzDDgtuoHBkCVdMrbYffWXLym=1988115778; else dUAPxzzDDgtuoHBkCVdMrbYffWXLym=417124281;long KNcuSWZepKUZyqcssVfqueVKUJRGeE=955054678;if (KNcuSWZepKUZyqcssVfqueVKUJRGeE == KNcuSWZepKUZyqcssVfqueVKUJRGeE- 0 ) KNcuSWZepKUZyqcssVfqueVKUJRGeE=2047252289; else KNcuSWZepKUZyqcssVfqueVKUJRGeE=1892979268;if (KNcuSWZepKUZyqcssVfqueVKUJRGeE == KNcuSWZepKUZyqcssVfqueVKUJRGeE- 1 ) KNcuSWZepKUZyqcssVfqueVKUJRGeE=494408901; else KNcuSWZepKUZyqcssVfqueVKUJRGeE=1352436178;if (KNcuSWZepKUZyqcssVfqueVKUJRGeE == KNcuSWZepKUZyqcssVfqueVKUJRGeE- 0 ) KNcuSWZepKUZyqcssVfqueVKUJRGeE=1398882081; else KNcuSWZepKUZyqcssVfqueVKUJRGeE=1100695626;if (KNcuSWZepKUZyqcssVfqueVKUJRGeE == KNcuSWZepKUZyqcssVfqueVKUJRGeE- 0 ) KNcuSWZepKUZyqcssVfqueVKUJRGeE=1905917830; else KNcuSWZepKUZyqcssVfqueVKUJRGeE=881683351;if (KNcuSWZepKUZyqcssVfqueVKUJRGeE == KNcuSWZepKUZyqcssVfqueVKUJRGeE- 1 ) KNcuSWZepKUZyqcssVfqueVKUJRGeE=1875314762; else KNcuSWZepKUZyqcssVfqueVKUJRGeE=1628094941;if (KNcuSWZepKUZyqcssVfqueVKUJRGeE == KNcuSWZepKUZyqcssVfqueVKUJRGeE- 0 ) KNcuSWZepKUZyqcssVfqueVKUJRGeE=1811590994; else KNcuSWZepKUZyqcssVfqueVKUJRGeE=2024412395;float nWnGNZFSIFWTVJNZQTfaLVhGwByYLj=1412917998.675731015323793845140162113001f;if (nWnGNZFSIFWTVJNZQTfaLVhGwByYLj - nWnGNZFSIFWTVJNZQTfaLVhGwByYLj> 0.00000001 ) nWnGNZFSIFWTVJNZQTfaLVhGwByYLj=171979722.834354155345369259494382351463f; else nWnGNZFSIFWTVJNZQTfaLVhGwByYLj=664120104.902450944089080580306461856211f;if (nWnGNZFSIFWTVJNZQTfaLVhGwByYLj - nWnGNZFSIFWTVJNZQTfaLVhGwByYLj> 0.00000001 ) nWnGNZFSIFWTVJNZQTfaLVhGwByYLj=861515828.380355401031552540387216764171f; else nWnGNZFSIFWTVJNZQTfaLVhGwByYLj=940180801.484470074529948691032703418479f;if (nWnGNZFSIFWTVJNZQTfaLVhGwByYLj - nWnGNZFSIFWTVJNZQTfaLVhGwByYLj> 0.00000001 ) nWnGNZFSIFWTVJNZQTfaLVhGwByYLj=880959683.143165828267687937927314989117f; else nWnGNZFSIFWTVJNZQTfaLVhGwByYLj=1043498357.361658699398498612768165283028f;if (nWnGNZFSIFWTVJNZQTfaLVhGwByYLj - nWnGNZFSIFWTVJNZQTfaLVhGwByYLj> 0.00000001 ) nWnGNZFSIFWTVJNZQTfaLVhGwByYLj=1887108656.771200613935864003648756920647f; else nWnGNZFSIFWTVJNZQTfaLVhGwByYLj=1241880057.677272729901983341238049112638f;if (nWnGNZFSIFWTVJNZQTfaLVhGwByYLj - nWnGNZFSIFWTVJNZQTfaLVhGwByYLj> 0.00000001 ) nWnGNZFSIFWTVJNZQTfaLVhGwByYLj=1290276408.118636979238484887369941063827f; else nWnGNZFSIFWTVJNZQTfaLVhGwByYLj=920270501.129190335857310067533538027778f;if (nWnGNZFSIFWTVJNZQTfaLVhGwByYLj - nWnGNZFSIFWTVJNZQTfaLVhGwByYLj> 0.00000001 ) nWnGNZFSIFWTVJNZQTfaLVhGwByYLj=1425062846.316211940555262274100739570227f; else nWnGNZFSIFWTVJNZQTfaLVhGwByYLj=956174859.367854210013709838696378878785f;int lIPKsrVEqyeTHskzPBfRIgQHzzuAob=342080864;if (lIPKsrVEqyeTHskzPBfRIgQHzzuAob == lIPKsrVEqyeTHskzPBfRIgQHzzuAob- 0 ) lIPKsrVEqyeTHskzPBfRIgQHzzuAob=597682301; else lIPKsrVEqyeTHskzPBfRIgQHzzuAob=379586667;if (lIPKsrVEqyeTHskzPBfRIgQHzzuAob == lIPKsrVEqyeTHskzPBfRIgQHzzuAob- 0 ) lIPKsrVEqyeTHskzPBfRIgQHzzuAob=1669632600; else lIPKsrVEqyeTHskzPBfRIgQHzzuAob=492897614;if (lIPKsrVEqyeTHskzPBfRIgQHzzuAob == lIPKsrVEqyeTHskzPBfRIgQHzzuAob- 1 ) lIPKsrVEqyeTHskzPBfRIgQHzzuAob=574828770; else lIPKsrVEqyeTHskzPBfRIgQHzzuAob=540374020;if (lIPKsrVEqyeTHskzPBfRIgQHzzuAob == lIPKsrVEqyeTHskzPBfRIgQHzzuAob- 1 ) lIPKsrVEqyeTHskzPBfRIgQHzzuAob=1710705864; else lIPKsrVEqyeTHskzPBfRIgQHzzuAob=1532856619;if (lIPKsrVEqyeTHskzPBfRIgQHzzuAob == lIPKsrVEqyeTHskzPBfRIgQHzzuAob- 0 ) lIPKsrVEqyeTHskzPBfRIgQHzzuAob=514308033; else lIPKsrVEqyeTHskzPBfRIgQHzzuAob=293207570;if (lIPKsrVEqyeTHskzPBfRIgQHzzuAob == lIPKsrVEqyeTHskzPBfRIgQHzzuAob- 1 ) lIPKsrVEqyeTHskzPBfRIgQHzzuAob=1551811582; else lIPKsrVEqyeTHskzPBfRIgQHzzuAob=480054072;double exSutDIEUkpxyiadUPfakuFgnlddGi=1252508458.700793263021029185217612347634;if (exSutDIEUkpxyiadUPfakuFgnlddGi == exSutDIEUkpxyiadUPfakuFgnlddGi ) exSutDIEUkpxyiadUPfakuFgnlddGi=1177301383.682812615715244466922756710856; else exSutDIEUkpxyiadUPfakuFgnlddGi=575197780.773945031757060258998272204621;if (exSutDIEUkpxyiadUPfakuFgnlddGi == exSutDIEUkpxyiadUPfakuFgnlddGi ) exSutDIEUkpxyiadUPfakuFgnlddGi=245144559.387014330054527680656306851175; else exSutDIEUkpxyiadUPfakuFgnlddGi=1732713984.251148005702805475572407519136;if (exSutDIEUkpxyiadUPfakuFgnlddGi == exSutDIEUkpxyiadUPfakuFgnlddGi ) exSutDIEUkpxyiadUPfakuFgnlddGi=257647252.426591449068506451258671385563; else exSutDIEUkpxyiadUPfakuFgnlddGi=1913808212.908039642514761401217908895621;if (exSutDIEUkpxyiadUPfakuFgnlddGi == exSutDIEUkpxyiadUPfakuFgnlddGi ) exSutDIEUkpxyiadUPfakuFgnlddGi=74094995.609617157792084502008535710035; else exSutDIEUkpxyiadUPfakuFgnlddGi=1003753932.412192615545453602530473327400;if (exSutDIEUkpxyiadUPfakuFgnlddGi == exSutDIEUkpxyiadUPfakuFgnlddGi ) exSutDIEUkpxyiadUPfakuFgnlddGi=5870612.713348099309766428312960604988; else exSutDIEUkpxyiadUPfakuFgnlddGi=1400012491.522583311561333271801929996385;if (exSutDIEUkpxyiadUPfakuFgnlddGi == exSutDIEUkpxyiadUPfakuFgnlddGi ) exSutDIEUkpxyiadUPfakuFgnlddGi=62159770.522289729311786263853428871479; else exSutDIEUkpxyiadUPfakuFgnlddGi=478611580.803715926555359024483364211451;long PbvZFnAhFKXmMwqChunYiBwgbXxdzU=1627814450;if (PbvZFnAhFKXmMwqChunYiBwgbXxdzU == PbvZFnAhFKXmMwqChunYiBwgbXxdzU- 1 ) PbvZFnAhFKXmMwqChunYiBwgbXxdzU=801514369; else PbvZFnAhFKXmMwqChunYiBwgbXxdzU=1469510130;if (PbvZFnAhFKXmMwqChunYiBwgbXxdzU == PbvZFnAhFKXmMwqChunYiBwgbXxdzU- 0 ) PbvZFnAhFKXmMwqChunYiBwgbXxdzU=1426730094; else PbvZFnAhFKXmMwqChunYiBwgbXxdzU=696244008;if (PbvZFnAhFKXmMwqChunYiBwgbXxdzU == PbvZFnAhFKXmMwqChunYiBwgbXxdzU- 0 ) PbvZFnAhFKXmMwqChunYiBwgbXxdzU=549282263; else PbvZFnAhFKXmMwqChunYiBwgbXxdzU=1792337161;if (PbvZFnAhFKXmMwqChunYiBwgbXxdzU == PbvZFnAhFKXmMwqChunYiBwgbXxdzU- 1 ) PbvZFnAhFKXmMwqChunYiBwgbXxdzU=838089228; else PbvZFnAhFKXmMwqChunYiBwgbXxdzU=1779089349;if (PbvZFnAhFKXmMwqChunYiBwgbXxdzU == PbvZFnAhFKXmMwqChunYiBwgbXxdzU- 1 ) PbvZFnAhFKXmMwqChunYiBwgbXxdzU=1100897887; else PbvZFnAhFKXmMwqChunYiBwgbXxdzU=1476142856;if (PbvZFnAhFKXmMwqChunYiBwgbXxdzU == PbvZFnAhFKXmMwqChunYiBwgbXxdzU- 0 ) PbvZFnAhFKXmMwqChunYiBwgbXxdzU=425872768; else PbvZFnAhFKXmMwqChunYiBwgbXxdzU=587752113;float goCdpGWWGWzYYbeeBAbCdFTDdNVCHn=785788054.214021677453639338014835152371f;if (goCdpGWWGWzYYbeeBAbCdFTDdNVCHn - goCdpGWWGWzYYbeeBAbCdFTDdNVCHn> 0.00000001 ) goCdpGWWGWzYYbeeBAbCdFTDdNVCHn=242439627.441673923768590434946813449097f; else goCdpGWWGWzYYbeeBAbCdFTDdNVCHn=172554388.930702316652535734583844674824f;if (goCdpGWWGWzYYbeeBAbCdFTDdNVCHn - goCdpGWWGWzYYbeeBAbCdFTDdNVCHn> 0.00000001 ) goCdpGWWGWzYYbeeBAbCdFTDdNVCHn=1926357123.493683899923930538882552520724f; else goCdpGWWGWzYYbeeBAbCdFTDdNVCHn=71011774.950336905394676101514615205217f;if (goCdpGWWGWzYYbeeBAbCdFTDdNVCHn - goCdpGWWGWzYYbeeBAbCdFTDdNVCHn> 0.00000001 ) goCdpGWWGWzYYbeeBAbCdFTDdNVCHn=2355753.428618371464469809652921145513f; else goCdpGWWGWzYYbeeBAbCdFTDdNVCHn=1241350235.189898707984116025650666551857f;if (goCdpGWWGWzYYbeeBAbCdFTDdNVCHn - goCdpGWWGWzYYbeeBAbCdFTDdNVCHn> 0.00000001 ) goCdpGWWGWzYYbeeBAbCdFTDdNVCHn=84433612.486849567220388991512006452335f; else goCdpGWWGWzYYbeeBAbCdFTDdNVCHn=831357736.896050838856854149977134649159f;if (goCdpGWWGWzYYbeeBAbCdFTDdNVCHn - goCdpGWWGWzYYbeeBAbCdFTDdNVCHn> 0.00000001 ) goCdpGWWGWzYYbeeBAbCdFTDdNVCHn=305491672.723344543926386635342457876242f; else goCdpGWWGWzYYbeeBAbCdFTDdNVCHn=665878668.189014351061605186902482848758f;if (goCdpGWWGWzYYbeeBAbCdFTDdNVCHn - goCdpGWWGWzYYbeeBAbCdFTDdNVCHn> 0.00000001 ) goCdpGWWGWzYYbeeBAbCdFTDdNVCHn=487027002.830285848338538186280145170727f; else goCdpGWWGWzYYbeeBAbCdFTDdNVCHn=1322299926.670433252135867406576358970521f;long GIBjgpFAVuORHXWNmpjoWLYFSMKccP=1712048709;if (GIBjgpFAVuORHXWNmpjoWLYFSMKccP == GIBjgpFAVuORHXWNmpjoWLYFSMKccP- 1 ) GIBjgpFAVuORHXWNmpjoWLYFSMKccP=57733773; else GIBjgpFAVuORHXWNmpjoWLYFSMKccP=2071480779;if (GIBjgpFAVuORHXWNmpjoWLYFSMKccP == GIBjgpFAVuORHXWNmpjoWLYFSMKccP- 1 ) GIBjgpFAVuORHXWNmpjoWLYFSMKccP=1826023350; else GIBjgpFAVuORHXWNmpjoWLYFSMKccP=934597992;if (GIBjgpFAVuORHXWNmpjoWLYFSMKccP == GIBjgpFAVuORHXWNmpjoWLYFSMKccP- 0 ) GIBjgpFAVuORHXWNmpjoWLYFSMKccP=1225241907; else GIBjgpFAVuORHXWNmpjoWLYFSMKccP=2129070513;if (GIBjgpFAVuORHXWNmpjoWLYFSMKccP == GIBjgpFAVuORHXWNmpjoWLYFSMKccP- 1 ) GIBjgpFAVuORHXWNmpjoWLYFSMKccP=703226195; else GIBjgpFAVuORHXWNmpjoWLYFSMKccP=1532447869;if (GIBjgpFAVuORHXWNmpjoWLYFSMKccP == GIBjgpFAVuORHXWNmpjoWLYFSMKccP- 1 ) GIBjgpFAVuORHXWNmpjoWLYFSMKccP=1884592199; else GIBjgpFAVuORHXWNmpjoWLYFSMKccP=613538857;if (GIBjgpFAVuORHXWNmpjoWLYFSMKccP == GIBjgpFAVuORHXWNmpjoWLYFSMKccP- 0 ) GIBjgpFAVuORHXWNmpjoWLYFSMKccP=631816097; else GIBjgpFAVuORHXWNmpjoWLYFSMKccP=928988043;long mtmGNYccUHymbrMStAszEKuIRFJKhh=746299675;if (mtmGNYccUHymbrMStAszEKuIRFJKhh == mtmGNYccUHymbrMStAszEKuIRFJKhh- 1 ) mtmGNYccUHymbrMStAszEKuIRFJKhh=414756909; else mtmGNYccUHymbrMStAszEKuIRFJKhh=1423855363;if (mtmGNYccUHymbrMStAszEKuIRFJKhh == mtmGNYccUHymbrMStAszEKuIRFJKhh- 0 ) mtmGNYccUHymbrMStAszEKuIRFJKhh=236171953; else mtmGNYccUHymbrMStAszEKuIRFJKhh=1650562039;if (mtmGNYccUHymbrMStAszEKuIRFJKhh == mtmGNYccUHymbrMStAszEKuIRFJKhh- 0 ) mtmGNYccUHymbrMStAszEKuIRFJKhh=1170869495; else mtmGNYccUHymbrMStAszEKuIRFJKhh=164462023;if (mtmGNYccUHymbrMStAszEKuIRFJKhh == mtmGNYccUHymbrMStAszEKuIRFJKhh- 0 ) mtmGNYccUHymbrMStAszEKuIRFJKhh=1625458697; else mtmGNYccUHymbrMStAszEKuIRFJKhh=1707647781;if (mtmGNYccUHymbrMStAszEKuIRFJKhh == mtmGNYccUHymbrMStAszEKuIRFJKhh- 1 ) mtmGNYccUHymbrMStAszEKuIRFJKhh=1385076518; else mtmGNYccUHymbrMStAszEKuIRFJKhh=595372358;if (mtmGNYccUHymbrMStAszEKuIRFJKhh == mtmGNYccUHymbrMStAszEKuIRFJKhh- 0 ) mtmGNYccUHymbrMStAszEKuIRFJKhh=390752336; else mtmGNYccUHymbrMStAszEKuIRFJKhh=2030887242;long KyRfLgFBdMRIctrkVBXTqONFSsegaV=2016287250;if (KyRfLgFBdMRIctrkVBXTqONFSsegaV == KyRfLgFBdMRIctrkVBXTqONFSsegaV- 1 ) KyRfLgFBdMRIctrkVBXTqONFSsegaV=1766402664; else KyRfLgFBdMRIctrkVBXTqONFSsegaV=861307706;if (KyRfLgFBdMRIctrkVBXTqONFSsegaV == KyRfLgFBdMRIctrkVBXTqONFSsegaV- 0 ) KyRfLgFBdMRIctrkVBXTqONFSsegaV=530018340; else KyRfLgFBdMRIctrkVBXTqONFSsegaV=1715173290;if (KyRfLgFBdMRIctrkVBXTqONFSsegaV == KyRfLgFBdMRIctrkVBXTqONFSsegaV- 1 ) KyRfLgFBdMRIctrkVBXTqONFSsegaV=1728514881; else KyRfLgFBdMRIctrkVBXTqONFSsegaV=50657590;if (KyRfLgFBdMRIctrkVBXTqONFSsegaV == KyRfLgFBdMRIctrkVBXTqONFSsegaV- 1 ) KyRfLgFBdMRIctrkVBXTqONFSsegaV=1708029794; else KyRfLgFBdMRIctrkVBXTqONFSsegaV=260852761;if (KyRfLgFBdMRIctrkVBXTqONFSsegaV == KyRfLgFBdMRIctrkVBXTqONFSsegaV- 0 ) KyRfLgFBdMRIctrkVBXTqONFSsegaV=1616811840; else KyRfLgFBdMRIctrkVBXTqONFSsegaV=393909254;if (KyRfLgFBdMRIctrkVBXTqONFSsegaV == KyRfLgFBdMRIctrkVBXTqONFSsegaV- 1 ) KyRfLgFBdMRIctrkVBXTqONFSsegaV=333903222; else KyRfLgFBdMRIctrkVBXTqONFSsegaV=1536300543;int XLhjCFyGAVodMDnAglIcZbZfiBJcaO=819148790;if (XLhjCFyGAVodMDnAglIcZbZfiBJcaO == XLhjCFyGAVodMDnAglIcZbZfiBJcaO- 0 ) XLhjCFyGAVodMDnAglIcZbZfiBJcaO=1301295158; else XLhjCFyGAVodMDnAglIcZbZfiBJcaO=1888210974;if (XLhjCFyGAVodMDnAglIcZbZfiBJcaO == XLhjCFyGAVodMDnAglIcZbZfiBJcaO- 0 ) XLhjCFyGAVodMDnAglIcZbZfiBJcaO=907543017; else XLhjCFyGAVodMDnAglIcZbZfiBJcaO=1201842534;if (XLhjCFyGAVodMDnAglIcZbZfiBJcaO == XLhjCFyGAVodMDnAglIcZbZfiBJcaO- 0 ) XLhjCFyGAVodMDnAglIcZbZfiBJcaO=995922331; else XLhjCFyGAVodMDnAglIcZbZfiBJcaO=206661800;if (XLhjCFyGAVodMDnAglIcZbZfiBJcaO == XLhjCFyGAVodMDnAglIcZbZfiBJcaO- 0 ) XLhjCFyGAVodMDnAglIcZbZfiBJcaO=541179214; else XLhjCFyGAVodMDnAglIcZbZfiBJcaO=2040755397;if (XLhjCFyGAVodMDnAglIcZbZfiBJcaO == XLhjCFyGAVodMDnAglIcZbZfiBJcaO- 1 ) XLhjCFyGAVodMDnAglIcZbZfiBJcaO=446022082; else XLhjCFyGAVodMDnAglIcZbZfiBJcaO=118375569;if (XLhjCFyGAVodMDnAglIcZbZfiBJcaO == XLhjCFyGAVodMDnAglIcZbZfiBJcaO- 0 ) XLhjCFyGAVodMDnAglIcZbZfiBJcaO=1697005121; else XLhjCFyGAVodMDnAglIcZbZfiBJcaO=629869549;double vEovNBfpxELMMljoxWIFaTbubSBnYa=221319250.527680148562122017862528994994;if (vEovNBfpxELMMljoxWIFaTbubSBnYa == vEovNBfpxELMMljoxWIFaTbubSBnYa ) vEovNBfpxELMMljoxWIFaTbubSBnYa=1460147405.332107907325676025427157111079; else vEovNBfpxELMMljoxWIFaTbubSBnYa=763012494.636533012512165796208057739500;if (vEovNBfpxELMMljoxWIFaTbubSBnYa == vEovNBfpxELMMljoxWIFaTbubSBnYa ) vEovNBfpxELMMljoxWIFaTbubSBnYa=1310069026.703870015302492510352687279980; else vEovNBfpxELMMljoxWIFaTbubSBnYa=626627738.561292030588191666633036922670;if (vEovNBfpxELMMljoxWIFaTbubSBnYa == vEovNBfpxELMMljoxWIFaTbubSBnYa ) vEovNBfpxELMMljoxWIFaTbubSBnYa=1103808189.214594131068087459761346863638; else vEovNBfpxELMMljoxWIFaTbubSBnYa=1944377514.046048571764677506967302907974;if (vEovNBfpxELMMljoxWIFaTbubSBnYa == vEovNBfpxELMMljoxWIFaTbubSBnYa ) vEovNBfpxELMMljoxWIFaTbubSBnYa=1165268441.260303204099900878946878175525; else vEovNBfpxELMMljoxWIFaTbubSBnYa=364099319.170761437530207950223004849223;if (vEovNBfpxELMMljoxWIFaTbubSBnYa == vEovNBfpxELMMljoxWIFaTbubSBnYa ) vEovNBfpxELMMljoxWIFaTbubSBnYa=1863309216.889093034044251777233531327692; else vEovNBfpxELMMljoxWIFaTbubSBnYa=2122802839.231476232403638996428310145988;if (vEovNBfpxELMMljoxWIFaTbubSBnYa == vEovNBfpxELMMljoxWIFaTbubSBnYa ) vEovNBfpxELMMljoxWIFaTbubSBnYa=33750199.644392388665684191343227502244; else vEovNBfpxELMMljoxWIFaTbubSBnYa=776960241.136496945661997473616564553789;float HWUYMuJyrGXgkrIPPyEMwdPpdjJtTN=82147302.978778046153400534907151029462f;if (HWUYMuJyrGXgkrIPPyEMwdPpdjJtTN - HWUYMuJyrGXgkrIPPyEMwdPpdjJtTN> 0.00000001 ) HWUYMuJyrGXgkrIPPyEMwdPpdjJtTN=1251950066.804363446769611097950192283886f; else HWUYMuJyrGXgkrIPPyEMwdPpdjJtTN=1719087193.979445015719935965004351371229f;if (HWUYMuJyrGXgkrIPPyEMwdPpdjJtTN - HWUYMuJyrGXgkrIPPyEMwdPpdjJtTN> 0.00000001 ) HWUYMuJyrGXgkrIPPyEMwdPpdjJtTN=729592084.167189028718234137832998272927f; else HWUYMuJyrGXgkrIPPyEMwdPpdjJtTN=1564582832.589765395605840613835459838992f;if (HWUYMuJyrGXgkrIPPyEMwdPpdjJtTN - HWUYMuJyrGXgkrIPPyEMwdPpdjJtTN> 0.00000001 ) HWUYMuJyrGXgkrIPPyEMwdPpdjJtTN=755729932.721124967985324726977029313194f; else HWUYMuJyrGXgkrIPPyEMwdPpdjJtTN=99032565.841499572675795023512312581282f;if (HWUYMuJyrGXgkrIPPyEMwdPpdjJtTN - HWUYMuJyrGXgkrIPPyEMwdPpdjJtTN> 0.00000001 ) HWUYMuJyrGXgkrIPPyEMwdPpdjJtTN=1020364919.197424594625430233439169264991f; else HWUYMuJyrGXgkrIPPyEMwdPpdjJtTN=1559740864.949169947163000898336886303927f;if (HWUYMuJyrGXgkrIPPyEMwdPpdjJtTN - HWUYMuJyrGXgkrIPPyEMwdPpdjJtTN> 0.00000001 ) HWUYMuJyrGXgkrIPPyEMwdPpdjJtTN=320128492.171389648240747627057776616486f; else HWUYMuJyrGXgkrIPPyEMwdPpdjJtTN=636746590.602010372125010615989608141587f;if (HWUYMuJyrGXgkrIPPyEMwdPpdjJtTN - HWUYMuJyrGXgkrIPPyEMwdPpdjJtTN> 0.00000001 ) HWUYMuJyrGXgkrIPPyEMwdPpdjJtTN=1269151208.337275973835694276701250711507f; else HWUYMuJyrGXgkrIPPyEMwdPpdjJtTN=1610139312.090580995684215511498598728757f;float TlfqWbCFVRrHFMkyUaOyxCGwZBKtfH=796989240.556424052024604403690096199864f;if (TlfqWbCFVRrHFMkyUaOyxCGwZBKtfH - TlfqWbCFVRrHFMkyUaOyxCGwZBKtfH> 0.00000001 ) TlfqWbCFVRrHFMkyUaOyxCGwZBKtfH=166771544.544906385716242612888364367043f; else TlfqWbCFVRrHFMkyUaOyxCGwZBKtfH=112583972.529868071923898586559819739065f;if (TlfqWbCFVRrHFMkyUaOyxCGwZBKtfH - TlfqWbCFVRrHFMkyUaOyxCGwZBKtfH> 0.00000001 ) TlfqWbCFVRrHFMkyUaOyxCGwZBKtfH=76153138.944320748464207657798260582124f; else TlfqWbCFVRrHFMkyUaOyxCGwZBKtfH=1994005291.012608967985819571228247888585f;if (TlfqWbCFVRrHFMkyUaOyxCGwZBKtfH - TlfqWbCFVRrHFMkyUaOyxCGwZBKtfH> 0.00000001 ) TlfqWbCFVRrHFMkyUaOyxCGwZBKtfH=2140343837.972556361970091345102190698582f; else TlfqWbCFVRrHFMkyUaOyxCGwZBKtfH=864278201.158312337331145926655349489397f;if (TlfqWbCFVRrHFMkyUaOyxCGwZBKtfH - TlfqWbCFVRrHFMkyUaOyxCGwZBKtfH> 0.00000001 ) TlfqWbCFVRrHFMkyUaOyxCGwZBKtfH=1862824248.988983288011988105677394881003f; else TlfqWbCFVRrHFMkyUaOyxCGwZBKtfH=1504401291.314155336285875446139473070521f;if (TlfqWbCFVRrHFMkyUaOyxCGwZBKtfH - TlfqWbCFVRrHFMkyUaOyxCGwZBKtfH> 0.00000001 ) TlfqWbCFVRrHFMkyUaOyxCGwZBKtfH=722942533.329680893488940353954597594219f; else TlfqWbCFVRrHFMkyUaOyxCGwZBKtfH=543443698.582606729576385154372777784229f;if (TlfqWbCFVRrHFMkyUaOyxCGwZBKtfH - TlfqWbCFVRrHFMkyUaOyxCGwZBKtfH> 0.00000001 ) TlfqWbCFVRrHFMkyUaOyxCGwZBKtfH=1486016965.768591745474449922244104882502f; else TlfqWbCFVRrHFMkyUaOyxCGwZBKtfH=1020603139.351191890552091943987218567471f; }
 TlfqWbCFVRrHFMkyUaOyxCGwZBKtfHy::TlfqWbCFVRrHFMkyUaOyxCGwZBKtfHy()
 { this->PNwdFFFhWELM("QqHThcOBtyyyauPkavJSHKgWQejQURPNwdFFFhWELMj", true, 172911601, 1911702620, 62712949); }
#pragma optimize("", off)
 // <delete/>


// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class QLVFbEXRDttODwPDydnHdiTEYsGVPpy
 { 
public: bool USZtiiTdJLvpFWvAFDZoFyPdisQgjx; double USZtiiTdJLvpFWvAFDZoFyPdisQgjxQLVFbEXRDttODwPDydnHdiTEYsGVPp; QLVFbEXRDttODwPDydnHdiTEYsGVPpy(); void ICBKECynXhnO(string USZtiiTdJLvpFWvAFDZoFyPdisQgjxICBKECynXhnO, bool BBNVYvdCgUeAdvhsOhdsBRpASWDwLq, int SmuLKubjyzWCWFJEVTvpdiDqvjLNDC, float shqqCrGRknQfjeWzvkqNBjAWNjjDPa, long iOgvGoMgCSyDiIYyzZUwoaWqBJhUcS);
 protected: bool USZtiiTdJLvpFWvAFDZoFyPdisQgjxo; double USZtiiTdJLvpFWvAFDZoFyPdisQgjxQLVFbEXRDttODwPDydnHdiTEYsGVPpf; void ICBKECynXhnOu(string USZtiiTdJLvpFWvAFDZoFyPdisQgjxICBKECynXhnOg, bool BBNVYvdCgUeAdvhsOhdsBRpASWDwLqe, int SmuLKubjyzWCWFJEVTvpdiDqvjLNDCr, float shqqCrGRknQfjeWzvkqNBjAWNjjDPaw, long iOgvGoMgCSyDiIYyzZUwoaWqBJhUcSn);
 private: bool USZtiiTdJLvpFWvAFDZoFyPdisQgjxBBNVYvdCgUeAdvhsOhdsBRpASWDwLq; double USZtiiTdJLvpFWvAFDZoFyPdisQgjxshqqCrGRknQfjeWzvkqNBjAWNjjDPaQLVFbEXRDttODwPDydnHdiTEYsGVPp;
 void ICBKECynXhnOv(string BBNVYvdCgUeAdvhsOhdsBRpASWDwLqICBKECynXhnO, bool BBNVYvdCgUeAdvhsOhdsBRpASWDwLqSmuLKubjyzWCWFJEVTvpdiDqvjLNDC, int SmuLKubjyzWCWFJEVTvpdiDqvjLNDCUSZtiiTdJLvpFWvAFDZoFyPdisQgjx, float shqqCrGRknQfjeWzvkqNBjAWNjjDPaiOgvGoMgCSyDiIYyzZUwoaWqBJhUcS, long iOgvGoMgCSyDiIYyzZUwoaWqBJhUcSBBNVYvdCgUeAdvhsOhdsBRpASWDwLq); };
 void QLVFbEXRDttODwPDydnHdiTEYsGVPpy::ICBKECynXhnO(string USZtiiTdJLvpFWvAFDZoFyPdisQgjxICBKECynXhnO, bool BBNVYvdCgUeAdvhsOhdsBRpASWDwLq, int SmuLKubjyzWCWFJEVTvpdiDqvjLNDC, float shqqCrGRknQfjeWzvkqNBjAWNjjDPa, long iOgvGoMgCSyDiIYyzZUwoaWqBJhUcS)
 { float csVnsZZBevOExKlJFXUJOsjnegTTnw=1967364503.909668555208276600567080513424f;if (csVnsZZBevOExKlJFXUJOsjnegTTnw - csVnsZZBevOExKlJFXUJOsjnegTTnw> 0.00000001 ) csVnsZZBevOExKlJFXUJOsjnegTTnw=1021815969.369036424227196496739645993926f; else csVnsZZBevOExKlJFXUJOsjnegTTnw=1223038675.613104934128634109412780572415f;if (csVnsZZBevOExKlJFXUJOsjnegTTnw - csVnsZZBevOExKlJFXUJOsjnegTTnw> 0.00000001 ) csVnsZZBevOExKlJFXUJOsjnegTTnw=1970339633.460971400256271201290451494473f; else csVnsZZBevOExKlJFXUJOsjnegTTnw=655833795.253077887370927651947233511068f;if (csVnsZZBevOExKlJFXUJOsjnegTTnw - csVnsZZBevOExKlJFXUJOsjnegTTnw> 0.00000001 ) csVnsZZBevOExKlJFXUJOsjnegTTnw=705779881.804700431543492053585353613056f; else csVnsZZBevOExKlJFXUJOsjnegTTnw=2021275298.792067633733150585166858456220f;if (csVnsZZBevOExKlJFXUJOsjnegTTnw - csVnsZZBevOExKlJFXUJOsjnegTTnw> 0.00000001 ) csVnsZZBevOExKlJFXUJOsjnegTTnw=372636550.849586153553196221323890195572f; else csVnsZZBevOExKlJFXUJOsjnegTTnw=1711734992.251309477544371487223690705940f;if (csVnsZZBevOExKlJFXUJOsjnegTTnw - csVnsZZBevOExKlJFXUJOsjnegTTnw> 0.00000001 ) csVnsZZBevOExKlJFXUJOsjnegTTnw=1251167042.269230299895499963972845977260f; else csVnsZZBevOExKlJFXUJOsjnegTTnw=1122897936.564848614229854820294317843437f;if (csVnsZZBevOExKlJFXUJOsjnegTTnw - csVnsZZBevOExKlJFXUJOsjnegTTnw> 0.00000001 ) csVnsZZBevOExKlJFXUJOsjnegTTnw=689551733.413033275146986310216718625701f; else csVnsZZBevOExKlJFXUJOsjnegTTnw=1407837646.639882782166515944264432180542f;float ehYcFjGyTfMnFYzGzPlsEXXkKAghzI=377284283.262057144675413591662041362976f;if (ehYcFjGyTfMnFYzGzPlsEXXkKAghzI - ehYcFjGyTfMnFYzGzPlsEXXkKAghzI> 0.00000001 ) ehYcFjGyTfMnFYzGzPlsEXXkKAghzI=1454042535.047569654621439185463064832170f; else ehYcFjGyTfMnFYzGzPlsEXXkKAghzI=1477175221.528873805495705271238762419374f;if (ehYcFjGyTfMnFYzGzPlsEXXkKAghzI - ehYcFjGyTfMnFYzGzPlsEXXkKAghzI> 0.00000001 ) ehYcFjGyTfMnFYzGzPlsEXXkKAghzI=1856831942.759510367739007936066294287657f; else ehYcFjGyTfMnFYzGzPlsEXXkKAghzI=1274341663.563284730502461076558332857543f;if (ehYcFjGyTfMnFYzGzPlsEXXkKAghzI - ehYcFjGyTfMnFYzGzPlsEXXkKAghzI> 0.00000001 ) ehYcFjGyTfMnFYzGzPlsEXXkKAghzI=1569811996.471127792431767484421121924243f; else ehYcFjGyTfMnFYzGzPlsEXXkKAghzI=31481512.829723245449422878557413336030f;if (ehYcFjGyTfMnFYzGzPlsEXXkKAghzI - ehYcFjGyTfMnFYzGzPlsEXXkKAghzI> 0.00000001 ) ehYcFjGyTfMnFYzGzPlsEXXkKAghzI=784176438.465061204535019371916670652750f; else ehYcFjGyTfMnFYzGzPlsEXXkKAghzI=1564157504.722509456268402350506622599642f;if (ehYcFjGyTfMnFYzGzPlsEXXkKAghzI - ehYcFjGyTfMnFYzGzPlsEXXkKAghzI> 0.00000001 ) ehYcFjGyTfMnFYzGzPlsEXXkKAghzI=1936406034.065582451093586937841235421002f; else ehYcFjGyTfMnFYzGzPlsEXXkKAghzI=962921065.081531392190951414776281183127f;if (ehYcFjGyTfMnFYzGzPlsEXXkKAghzI - ehYcFjGyTfMnFYzGzPlsEXXkKAghzI> 0.00000001 ) ehYcFjGyTfMnFYzGzPlsEXXkKAghzI=1817021318.146001216575685239411743691145f; else ehYcFjGyTfMnFYzGzPlsEXXkKAghzI=757740131.828606405121921271705571741342f;float IiCmUjAnwmQRUvzqJKXMAUTAMeiueY=23346663.236411658268131813402716989608f;if (IiCmUjAnwmQRUvzqJKXMAUTAMeiueY - IiCmUjAnwmQRUvzqJKXMAUTAMeiueY> 0.00000001 ) IiCmUjAnwmQRUvzqJKXMAUTAMeiueY=1360759212.165152604911918955190109562153f; else IiCmUjAnwmQRUvzqJKXMAUTAMeiueY=601715099.205111308021059280914562916779f;if (IiCmUjAnwmQRUvzqJKXMAUTAMeiueY - IiCmUjAnwmQRUvzqJKXMAUTAMeiueY> 0.00000001 ) IiCmUjAnwmQRUvzqJKXMAUTAMeiueY=1870945978.066182691865727915039235084431f; else IiCmUjAnwmQRUvzqJKXMAUTAMeiueY=1944779456.574007608879015985500175194211f;if (IiCmUjAnwmQRUvzqJKXMAUTAMeiueY - IiCmUjAnwmQRUvzqJKXMAUTAMeiueY> 0.00000001 ) IiCmUjAnwmQRUvzqJKXMAUTAMeiueY=1580249544.827130282142837132010093140092f; else IiCmUjAnwmQRUvzqJKXMAUTAMeiueY=1581283830.449805298581357945806220194500f;if (IiCmUjAnwmQRUvzqJKXMAUTAMeiueY - IiCmUjAnwmQRUvzqJKXMAUTAMeiueY> 0.00000001 ) IiCmUjAnwmQRUvzqJKXMAUTAMeiueY=1662408713.129170303394463075631629623072f; else IiCmUjAnwmQRUvzqJKXMAUTAMeiueY=787907573.180946228764262296722998961056f;if (IiCmUjAnwmQRUvzqJKXMAUTAMeiueY - IiCmUjAnwmQRUvzqJKXMAUTAMeiueY> 0.00000001 ) IiCmUjAnwmQRUvzqJKXMAUTAMeiueY=1019322531.844221563885708847853736130219f; else IiCmUjAnwmQRUvzqJKXMAUTAMeiueY=565798041.628203105174404730122531312920f;if (IiCmUjAnwmQRUvzqJKXMAUTAMeiueY - IiCmUjAnwmQRUvzqJKXMAUTAMeiueY> 0.00000001 ) IiCmUjAnwmQRUvzqJKXMAUTAMeiueY=1189849254.235427850585278043240120720700f; else IiCmUjAnwmQRUvzqJKXMAUTAMeiueY=1428290409.129604909898165836348978012681f;int DcxVacklTHRWTbKBhawvUWfLMZIdsP=500817311;if (DcxVacklTHRWTbKBhawvUWfLMZIdsP == DcxVacklTHRWTbKBhawvUWfLMZIdsP- 0 ) DcxVacklTHRWTbKBhawvUWfLMZIdsP=2070901791; else DcxVacklTHRWTbKBhawvUWfLMZIdsP=2109083887;if (DcxVacklTHRWTbKBhawvUWfLMZIdsP == DcxVacklTHRWTbKBhawvUWfLMZIdsP- 1 ) DcxVacklTHRWTbKBhawvUWfLMZIdsP=1073870091; else DcxVacklTHRWTbKBhawvUWfLMZIdsP=277063830;if (DcxVacklTHRWTbKBhawvUWfLMZIdsP == DcxVacklTHRWTbKBhawvUWfLMZIdsP- 0 ) DcxVacklTHRWTbKBhawvUWfLMZIdsP=521019242; else DcxVacklTHRWTbKBhawvUWfLMZIdsP=2039206469;if (DcxVacklTHRWTbKBhawvUWfLMZIdsP == DcxVacklTHRWTbKBhawvUWfLMZIdsP- 0 ) DcxVacklTHRWTbKBhawvUWfLMZIdsP=397552507; else DcxVacklTHRWTbKBhawvUWfLMZIdsP=990334457;if (DcxVacklTHRWTbKBhawvUWfLMZIdsP == DcxVacklTHRWTbKBhawvUWfLMZIdsP- 0 ) DcxVacklTHRWTbKBhawvUWfLMZIdsP=438862237; else DcxVacklTHRWTbKBhawvUWfLMZIdsP=406182129;if (DcxVacklTHRWTbKBhawvUWfLMZIdsP == DcxVacklTHRWTbKBhawvUWfLMZIdsP- 0 ) DcxVacklTHRWTbKBhawvUWfLMZIdsP=866911408; else DcxVacklTHRWTbKBhawvUWfLMZIdsP=1890342463;double YBLScqZLrdJLUNKSDanUNueSFLQHpq=1272331180.120932968447590296934942651575;if (YBLScqZLrdJLUNKSDanUNueSFLQHpq == YBLScqZLrdJLUNKSDanUNueSFLQHpq ) YBLScqZLrdJLUNKSDanUNueSFLQHpq=107983787.341184547670382833346598929938; else YBLScqZLrdJLUNKSDanUNueSFLQHpq=666205070.249420652776946577192834986436;if (YBLScqZLrdJLUNKSDanUNueSFLQHpq == YBLScqZLrdJLUNKSDanUNueSFLQHpq ) YBLScqZLrdJLUNKSDanUNueSFLQHpq=47078434.671954106229277166246294642655; else YBLScqZLrdJLUNKSDanUNueSFLQHpq=1670209344.423711519357198477792139049731;if (YBLScqZLrdJLUNKSDanUNueSFLQHpq == YBLScqZLrdJLUNKSDanUNueSFLQHpq ) YBLScqZLrdJLUNKSDanUNueSFLQHpq=426952830.265180766766790920728597440478; else YBLScqZLrdJLUNKSDanUNueSFLQHpq=740487310.398910301882213745023901488885;if (YBLScqZLrdJLUNKSDanUNueSFLQHpq == YBLScqZLrdJLUNKSDanUNueSFLQHpq ) YBLScqZLrdJLUNKSDanUNueSFLQHpq=944518386.139858898172905748480656649854; else YBLScqZLrdJLUNKSDanUNueSFLQHpq=2020242352.253776347606795101785051568431;if (YBLScqZLrdJLUNKSDanUNueSFLQHpq == YBLScqZLrdJLUNKSDanUNueSFLQHpq ) YBLScqZLrdJLUNKSDanUNueSFLQHpq=1962250155.004492033940199596155818031232; else YBLScqZLrdJLUNKSDanUNueSFLQHpq=1068924369.801770331328984597317349620656;if (YBLScqZLrdJLUNKSDanUNueSFLQHpq == YBLScqZLrdJLUNKSDanUNueSFLQHpq ) YBLScqZLrdJLUNKSDanUNueSFLQHpq=255019132.291115952926684893554592666315; else YBLScqZLrdJLUNKSDanUNueSFLQHpq=1004013955.079523798075549176593721380966;double fUnuZoCiDCEdbpyzaOYuCqLHTdmNRT=1151003759.133536332251128083432205873866;if (fUnuZoCiDCEdbpyzaOYuCqLHTdmNRT == fUnuZoCiDCEdbpyzaOYuCqLHTdmNRT ) fUnuZoCiDCEdbpyzaOYuCqLHTdmNRT=1987694451.927560484148834237149578494977; else fUnuZoCiDCEdbpyzaOYuCqLHTdmNRT=1316204007.468644566971039548174772845918;if (fUnuZoCiDCEdbpyzaOYuCqLHTdmNRT == fUnuZoCiDCEdbpyzaOYuCqLHTdmNRT ) fUnuZoCiDCEdbpyzaOYuCqLHTdmNRT=834321987.651879129178138864905681389690; else fUnuZoCiDCEdbpyzaOYuCqLHTdmNRT=57400092.975932036142900764508202318150;if (fUnuZoCiDCEdbpyzaOYuCqLHTdmNRT == fUnuZoCiDCEdbpyzaOYuCqLHTdmNRT ) fUnuZoCiDCEdbpyzaOYuCqLHTdmNRT=1052449418.054921321694897389172629387463; else fUnuZoCiDCEdbpyzaOYuCqLHTdmNRT=737850319.598720808954492542603908953675;if (fUnuZoCiDCEdbpyzaOYuCqLHTdmNRT == fUnuZoCiDCEdbpyzaOYuCqLHTdmNRT ) fUnuZoCiDCEdbpyzaOYuCqLHTdmNRT=1649811068.596910628758133834784597060097; else fUnuZoCiDCEdbpyzaOYuCqLHTdmNRT=1469036390.734357858303595698612000210005;if (fUnuZoCiDCEdbpyzaOYuCqLHTdmNRT == fUnuZoCiDCEdbpyzaOYuCqLHTdmNRT ) fUnuZoCiDCEdbpyzaOYuCqLHTdmNRT=204162592.064113588608659201248258584425; else fUnuZoCiDCEdbpyzaOYuCqLHTdmNRT=880611521.796334959311001545111096892663;if (fUnuZoCiDCEdbpyzaOYuCqLHTdmNRT == fUnuZoCiDCEdbpyzaOYuCqLHTdmNRT ) fUnuZoCiDCEdbpyzaOYuCqLHTdmNRT=1505917219.017402343535303835815941480165; else fUnuZoCiDCEdbpyzaOYuCqLHTdmNRT=1174067415.330631471921378670306895918602;double LmqKBclhMUeTttUgiTpFEpfrcWaiKQ=1404426112.101630129351112153906045756694;if (LmqKBclhMUeTttUgiTpFEpfrcWaiKQ == LmqKBclhMUeTttUgiTpFEpfrcWaiKQ ) LmqKBclhMUeTttUgiTpFEpfrcWaiKQ=1020547028.819614976448791928316161927799; else LmqKBclhMUeTttUgiTpFEpfrcWaiKQ=2065001721.553565350829766936493849985880;if (LmqKBclhMUeTttUgiTpFEpfrcWaiKQ == LmqKBclhMUeTttUgiTpFEpfrcWaiKQ ) LmqKBclhMUeTttUgiTpFEpfrcWaiKQ=1320306622.838809523678959741609131672578; else LmqKBclhMUeTttUgiTpFEpfrcWaiKQ=550013350.157110197742383699035539170833;if (LmqKBclhMUeTttUgiTpFEpfrcWaiKQ == LmqKBclhMUeTttUgiTpFEpfrcWaiKQ ) LmqKBclhMUeTttUgiTpFEpfrcWaiKQ=738323841.716663195065496896919455472904; else LmqKBclhMUeTttUgiTpFEpfrcWaiKQ=1599984564.047503783161507015349592446220;if (LmqKBclhMUeTttUgiTpFEpfrcWaiKQ == LmqKBclhMUeTttUgiTpFEpfrcWaiKQ ) LmqKBclhMUeTttUgiTpFEpfrcWaiKQ=73864481.129113809321486896847877655616; else LmqKBclhMUeTttUgiTpFEpfrcWaiKQ=1330414481.756404240431603822766148446366;if (LmqKBclhMUeTttUgiTpFEpfrcWaiKQ == LmqKBclhMUeTttUgiTpFEpfrcWaiKQ ) LmqKBclhMUeTttUgiTpFEpfrcWaiKQ=1164565501.830367609275752627584495172807; else LmqKBclhMUeTttUgiTpFEpfrcWaiKQ=1201111386.909470234632360150782998244520;if (LmqKBclhMUeTttUgiTpFEpfrcWaiKQ == LmqKBclhMUeTttUgiTpFEpfrcWaiKQ ) LmqKBclhMUeTttUgiTpFEpfrcWaiKQ=462297322.681237738249495214513223512133; else LmqKBclhMUeTttUgiTpFEpfrcWaiKQ=1599578520.771516224442254460404112847882;int eCcemovoyByAEiDBjtGcJmKvbfxklu=1082976208;if (eCcemovoyByAEiDBjtGcJmKvbfxklu == eCcemovoyByAEiDBjtGcJmKvbfxklu- 0 ) eCcemovoyByAEiDBjtGcJmKvbfxklu=58243045; else eCcemovoyByAEiDBjtGcJmKvbfxklu=1863391296;if (eCcemovoyByAEiDBjtGcJmKvbfxklu == eCcemovoyByAEiDBjtGcJmKvbfxklu- 1 ) eCcemovoyByAEiDBjtGcJmKvbfxklu=2050131659; else eCcemovoyByAEiDBjtGcJmKvbfxklu=1035533620;if (eCcemovoyByAEiDBjtGcJmKvbfxklu == eCcemovoyByAEiDBjtGcJmKvbfxklu- 1 ) eCcemovoyByAEiDBjtGcJmKvbfxklu=928058079; else eCcemovoyByAEiDBjtGcJmKvbfxklu=1635334568;if (eCcemovoyByAEiDBjtGcJmKvbfxklu == eCcemovoyByAEiDBjtGcJmKvbfxklu- 0 ) eCcemovoyByAEiDBjtGcJmKvbfxklu=219881762; else eCcemovoyByAEiDBjtGcJmKvbfxklu=1517617115;if (eCcemovoyByAEiDBjtGcJmKvbfxklu == eCcemovoyByAEiDBjtGcJmKvbfxklu- 1 ) eCcemovoyByAEiDBjtGcJmKvbfxklu=1163877649; else eCcemovoyByAEiDBjtGcJmKvbfxklu=725525691;if (eCcemovoyByAEiDBjtGcJmKvbfxklu == eCcemovoyByAEiDBjtGcJmKvbfxklu- 1 ) eCcemovoyByAEiDBjtGcJmKvbfxklu=1466046752; else eCcemovoyByAEiDBjtGcJmKvbfxklu=1693824956;long FhAYrXXsqjDsOIvYIlmHUqRioUYxhT=42700258;if (FhAYrXXsqjDsOIvYIlmHUqRioUYxhT == FhAYrXXsqjDsOIvYIlmHUqRioUYxhT- 0 ) FhAYrXXsqjDsOIvYIlmHUqRioUYxhT=485270776; else FhAYrXXsqjDsOIvYIlmHUqRioUYxhT=998076667;if (FhAYrXXsqjDsOIvYIlmHUqRioUYxhT == FhAYrXXsqjDsOIvYIlmHUqRioUYxhT- 0 ) FhAYrXXsqjDsOIvYIlmHUqRioUYxhT=2130950574; else FhAYrXXsqjDsOIvYIlmHUqRioUYxhT=1071597831;if (FhAYrXXsqjDsOIvYIlmHUqRioUYxhT == FhAYrXXsqjDsOIvYIlmHUqRioUYxhT- 0 ) FhAYrXXsqjDsOIvYIlmHUqRioUYxhT=183063054; else FhAYrXXsqjDsOIvYIlmHUqRioUYxhT=1945409381;if (FhAYrXXsqjDsOIvYIlmHUqRioUYxhT == FhAYrXXsqjDsOIvYIlmHUqRioUYxhT- 1 ) FhAYrXXsqjDsOIvYIlmHUqRioUYxhT=412679918; else FhAYrXXsqjDsOIvYIlmHUqRioUYxhT=387709887;if (FhAYrXXsqjDsOIvYIlmHUqRioUYxhT == FhAYrXXsqjDsOIvYIlmHUqRioUYxhT- 1 ) FhAYrXXsqjDsOIvYIlmHUqRioUYxhT=1242339526; else FhAYrXXsqjDsOIvYIlmHUqRioUYxhT=1651391153;if (FhAYrXXsqjDsOIvYIlmHUqRioUYxhT == FhAYrXXsqjDsOIvYIlmHUqRioUYxhT- 0 ) FhAYrXXsqjDsOIvYIlmHUqRioUYxhT=1040318238; else FhAYrXXsqjDsOIvYIlmHUqRioUYxhT=1998821679;int xfOzFbaeZDKCKZdzQYNqnpBqEeHZBS=810624040;if (xfOzFbaeZDKCKZdzQYNqnpBqEeHZBS == xfOzFbaeZDKCKZdzQYNqnpBqEeHZBS- 1 ) xfOzFbaeZDKCKZdzQYNqnpBqEeHZBS=879392969; else xfOzFbaeZDKCKZdzQYNqnpBqEeHZBS=422941011;if (xfOzFbaeZDKCKZdzQYNqnpBqEeHZBS == xfOzFbaeZDKCKZdzQYNqnpBqEeHZBS- 1 ) xfOzFbaeZDKCKZdzQYNqnpBqEeHZBS=1115824370; else xfOzFbaeZDKCKZdzQYNqnpBqEeHZBS=998464773;if (xfOzFbaeZDKCKZdzQYNqnpBqEeHZBS == xfOzFbaeZDKCKZdzQYNqnpBqEeHZBS- 0 ) xfOzFbaeZDKCKZdzQYNqnpBqEeHZBS=2016901885; else xfOzFbaeZDKCKZdzQYNqnpBqEeHZBS=1695603620;if (xfOzFbaeZDKCKZdzQYNqnpBqEeHZBS == xfOzFbaeZDKCKZdzQYNqnpBqEeHZBS- 1 ) xfOzFbaeZDKCKZdzQYNqnpBqEeHZBS=476368033; else xfOzFbaeZDKCKZdzQYNqnpBqEeHZBS=1532696107;if (xfOzFbaeZDKCKZdzQYNqnpBqEeHZBS == xfOzFbaeZDKCKZdzQYNqnpBqEeHZBS- 0 ) xfOzFbaeZDKCKZdzQYNqnpBqEeHZBS=1845119959; else xfOzFbaeZDKCKZdzQYNqnpBqEeHZBS=1950173154;if (xfOzFbaeZDKCKZdzQYNqnpBqEeHZBS == xfOzFbaeZDKCKZdzQYNqnpBqEeHZBS- 0 ) xfOzFbaeZDKCKZdzQYNqnpBqEeHZBS=1533619348; else xfOzFbaeZDKCKZdzQYNqnpBqEeHZBS=12798150;long EyGdSbcJvhiSSldjZMrwDodGysCSEP=1394114530;if (EyGdSbcJvhiSSldjZMrwDodGysCSEP == EyGdSbcJvhiSSldjZMrwDodGysCSEP- 0 ) EyGdSbcJvhiSSldjZMrwDodGysCSEP=2042725103; else EyGdSbcJvhiSSldjZMrwDodGysCSEP=419597064;if (EyGdSbcJvhiSSldjZMrwDodGysCSEP == EyGdSbcJvhiSSldjZMrwDodGysCSEP- 0 ) EyGdSbcJvhiSSldjZMrwDodGysCSEP=1114307744; else EyGdSbcJvhiSSldjZMrwDodGysCSEP=786110670;if (EyGdSbcJvhiSSldjZMrwDodGysCSEP == EyGdSbcJvhiSSldjZMrwDodGysCSEP- 1 ) EyGdSbcJvhiSSldjZMrwDodGysCSEP=1557555272; else EyGdSbcJvhiSSldjZMrwDodGysCSEP=1077104368;if (EyGdSbcJvhiSSldjZMrwDodGysCSEP == EyGdSbcJvhiSSldjZMrwDodGysCSEP- 1 ) EyGdSbcJvhiSSldjZMrwDodGysCSEP=899913154; else EyGdSbcJvhiSSldjZMrwDodGysCSEP=260317311;if (EyGdSbcJvhiSSldjZMrwDodGysCSEP == EyGdSbcJvhiSSldjZMrwDodGysCSEP- 0 ) EyGdSbcJvhiSSldjZMrwDodGysCSEP=1450711378; else EyGdSbcJvhiSSldjZMrwDodGysCSEP=1662421772;if (EyGdSbcJvhiSSldjZMrwDodGysCSEP == EyGdSbcJvhiSSldjZMrwDodGysCSEP- 1 ) EyGdSbcJvhiSSldjZMrwDodGysCSEP=1940677741; else EyGdSbcJvhiSSldjZMrwDodGysCSEP=1424150573;float TUEaTGMYoiyMMfcCaoyzmRhkFUdkKN=1113645101.571377336247946929962793922802f;if (TUEaTGMYoiyMMfcCaoyzmRhkFUdkKN - TUEaTGMYoiyMMfcCaoyzmRhkFUdkKN> 0.00000001 ) TUEaTGMYoiyMMfcCaoyzmRhkFUdkKN=1040676638.946617787582086374009497188845f; else TUEaTGMYoiyMMfcCaoyzmRhkFUdkKN=655422712.654531133935348082776990365953f;if (TUEaTGMYoiyMMfcCaoyzmRhkFUdkKN - TUEaTGMYoiyMMfcCaoyzmRhkFUdkKN> 0.00000001 ) TUEaTGMYoiyMMfcCaoyzmRhkFUdkKN=814431987.995304965624118516011727977428f; else TUEaTGMYoiyMMfcCaoyzmRhkFUdkKN=959112727.628015115522725388395005328843f;if (TUEaTGMYoiyMMfcCaoyzmRhkFUdkKN - TUEaTGMYoiyMMfcCaoyzmRhkFUdkKN> 0.00000001 ) TUEaTGMYoiyMMfcCaoyzmRhkFUdkKN=802110248.855701611351150910260913463480f; else TUEaTGMYoiyMMfcCaoyzmRhkFUdkKN=307405157.837287812587805505642868494245f;if (TUEaTGMYoiyMMfcCaoyzmRhkFUdkKN - TUEaTGMYoiyMMfcCaoyzmRhkFUdkKN> 0.00000001 ) TUEaTGMYoiyMMfcCaoyzmRhkFUdkKN=173282020.824162498311879322997895907999f; else TUEaTGMYoiyMMfcCaoyzmRhkFUdkKN=937150803.477876475311833110028336814037f;if (TUEaTGMYoiyMMfcCaoyzmRhkFUdkKN - TUEaTGMYoiyMMfcCaoyzmRhkFUdkKN> 0.00000001 ) TUEaTGMYoiyMMfcCaoyzmRhkFUdkKN=521708634.551905410395778606280807114207f; else TUEaTGMYoiyMMfcCaoyzmRhkFUdkKN=598254467.355987194981073467822896786489f;if (TUEaTGMYoiyMMfcCaoyzmRhkFUdkKN - TUEaTGMYoiyMMfcCaoyzmRhkFUdkKN> 0.00000001 ) TUEaTGMYoiyMMfcCaoyzmRhkFUdkKN=1373961519.131415199261331084585858567020f; else TUEaTGMYoiyMMfcCaoyzmRhkFUdkKN=380373547.171595066948866156580201539396f;long FmuDuzpWeyKmgXkzrKWBDVTBuoEydf=714816354;if (FmuDuzpWeyKmgXkzrKWBDVTBuoEydf == FmuDuzpWeyKmgXkzrKWBDVTBuoEydf- 0 ) FmuDuzpWeyKmgXkzrKWBDVTBuoEydf=1703030410; else FmuDuzpWeyKmgXkzrKWBDVTBuoEydf=1793687355;if (FmuDuzpWeyKmgXkzrKWBDVTBuoEydf == FmuDuzpWeyKmgXkzrKWBDVTBuoEydf- 0 ) FmuDuzpWeyKmgXkzrKWBDVTBuoEydf=2103580223; else FmuDuzpWeyKmgXkzrKWBDVTBuoEydf=2003439580;if (FmuDuzpWeyKmgXkzrKWBDVTBuoEydf == FmuDuzpWeyKmgXkzrKWBDVTBuoEydf- 1 ) FmuDuzpWeyKmgXkzrKWBDVTBuoEydf=1083408451; else FmuDuzpWeyKmgXkzrKWBDVTBuoEydf=1046222030;if (FmuDuzpWeyKmgXkzrKWBDVTBuoEydf == FmuDuzpWeyKmgXkzrKWBDVTBuoEydf- 0 ) FmuDuzpWeyKmgXkzrKWBDVTBuoEydf=1400552415; else FmuDuzpWeyKmgXkzrKWBDVTBuoEydf=1949772248;if (FmuDuzpWeyKmgXkzrKWBDVTBuoEydf == FmuDuzpWeyKmgXkzrKWBDVTBuoEydf- 1 ) FmuDuzpWeyKmgXkzrKWBDVTBuoEydf=1670511504; else FmuDuzpWeyKmgXkzrKWBDVTBuoEydf=855507059;if (FmuDuzpWeyKmgXkzrKWBDVTBuoEydf == FmuDuzpWeyKmgXkzrKWBDVTBuoEydf- 0 ) FmuDuzpWeyKmgXkzrKWBDVTBuoEydf=1695237042; else FmuDuzpWeyKmgXkzrKWBDVTBuoEydf=354480113;long BkcKgrNDhDozIkNLeVXIfpqwAbLPJo=1476576229;if (BkcKgrNDhDozIkNLeVXIfpqwAbLPJo == BkcKgrNDhDozIkNLeVXIfpqwAbLPJo- 1 ) BkcKgrNDhDozIkNLeVXIfpqwAbLPJo=708109036; else BkcKgrNDhDozIkNLeVXIfpqwAbLPJo=419974247;if (BkcKgrNDhDozIkNLeVXIfpqwAbLPJo == BkcKgrNDhDozIkNLeVXIfpqwAbLPJo- 1 ) BkcKgrNDhDozIkNLeVXIfpqwAbLPJo=1119605159; else BkcKgrNDhDozIkNLeVXIfpqwAbLPJo=1625998747;if (BkcKgrNDhDozIkNLeVXIfpqwAbLPJo == BkcKgrNDhDozIkNLeVXIfpqwAbLPJo- 0 ) BkcKgrNDhDozIkNLeVXIfpqwAbLPJo=1860574832; else BkcKgrNDhDozIkNLeVXIfpqwAbLPJo=1136731663;if (BkcKgrNDhDozIkNLeVXIfpqwAbLPJo == BkcKgrNDhDozIkNLeVXIfpqwAbLPJo- 1 ) BkcKgrNDhDozIkNLeVXIfpqwAbLPJo=592004282; else BkcKgrNDhDozIkNLeVXIfpqwAbLPJo=1735319946;if (BkcKgrNDhDozIkNLeVXIfpqwAbLPJo == BkcKgrNDhDozIkNLeVXIfpqwAbLPJo- 0 ) BkcKgrNDhDozIkNLeVXIfpqwAbLPJo=246315044; else BkcKgrNDhDozIkNLeVXIfpqwAbLPJo=177156941;if (BkcKgrNDhDozIkNLeVXIfpqwAbLPJo == BkcKgrNDhDozIkNLeVXIfpqwAbLPJo- 0 ) BkcKgrNDhDozIkNLeVXIfpqwAbLPJo=264402036; else BkcKgrNDhDozIkNLeVXIfpqwAbLPJo=959300805;float zINejXsrjwEjPooaLedZJaFeSEfhBF=1050829610.313455049336981880336244610732f;if (zINejXsrjwEjPooaLedZJaFeSEfhBF - zINejXsrjwEjPooaLedZJaFeSEfhBF> 0.00000001 ) zINejXsrjwEjPooaLedZJaFeSEfhBF=1408649697.008632722866159626630329828084f; else zINejXsrjwEjPooaLedZJaFeSEfhBF=1225861387.531843946027867110619521288981f;if (zINejXsrjwEjPooaLedZJaFeSEfhBF - zINejXsrjwEjPooaLedZJaFeSEfhBF> 0.00000001 ) zINejXsrjwEjPooaLedZJaFeSEfhBF=1868603810.584933541679161467372857512067f; else zINejXsrjwEjPooaLedZJaFeSEfhBF=948812622.506555303577905146193272074743f;if (zINejXsrjwEjPooaLedZJaFeSEfhBF - zINejXsrjwEjPooaLedZJaFeSEfhBF> 0.00000001 ) zINejXsrjwEjPooaLedZJaFeSEfhBF=412447154.259517361616491505358607444569f; else zINejXsrjwEjPooaLedZJaFeSEfhBF=1386108185.277142949898558788158949666218f;if (zINejXsrjwEjPooaLedZJaFeSEfhBF - zINejXsrjwEjPooaLedZJaFeSEfhBF> 0.00000001 ) zINejXsrjwEjPooaLedZJaFeSEfhBF=30236650.381355946976767015986751133868f; else zINejXsrjwEjPooaLedZJaFeSEfhBF=2141938255.783151730485329239365134274022f;if (zINejXsrjwEjPooaLedZJaFeSEfhBF - zINejXsrjwEjPooaLedZJaFeSEfhBF> 0.00000001 ) zINejXsrjwEjPooaLedZJaFeSEfhBF=1402353944.229839361739560243301560465009f; else zINejXsrjwEjPooaLedZJaFeSEfhBF=1012143386.781927669097873768514283186058f;if (zINejXsrjwEjPooaLedZJaFeSEfhBF - zINejXsrjwEjPooaLedZJaFeSEfhBF> 0.00000001 ) zINejXsrjwEjPooaLedZJaFeSEfhBF=1436084870.547306874369441470780859561984f; else zINejXsrjwEjPooaLedZJaFeSEfhBF=2094086836.171265167890425585382737870884f;int xvzKumKocPDMvtBWSVBmgeCKXWVRtd=1469491174;if (xvzKumKocPDMvtBWSVBmgeCKXWVRtd == xvzKumKocPDMvtBWSVBmgeCKXWVRtd- 0 ) xvzKumKocPDMvtBWSVBmgeCKXWVRtd=1808058337; else xvzKumKocPDMvtBWSVBmgeCKXWVRtd=2044868937;if (xvzKumKocPDMvtBWSVBmgeCKXWVRtd == xvzKumKocPDMvtBWSVBmgeCKXWVRtd- 0 ) xvzKumKocPDMvtBWSVBmgeCKXWVRtd=1819748158; else xvzKumKocPDMvtBWSVBmgeCKXWVRtd=2139299433;if (xvzKumKocPDMvtBWSVBmgeCKXWVRtd == xvzKumKocPDMvtBWSVBmgeCKXWVRtd- 1 ) xvzKumKocPDMvtBWSVBmgeCKXWVRtd=1427760256; else xvzKumKocPDMvtBWSVBmgeCKXWVRtd=775256975;if (xvzKumKocPDMvtBWSVBmgeCKXWVRtd == xvzKumKocPDMvtBWSVBmgeCKXWVRtd- 0 ) xvzKumKocPDMvtBWSVBmgeCKXWVRtd=1001725169; else xvzKumKocPDMvtBWSVBmgeCKXWVRtd=566298747;if (xvzKumKocPDMvtBWSVBmgeCKXWVRtd == xvzKumKocPDMvtBWSVBmgeCKXWVRtd- 0 ) xvzKumKocPDMvtBWSVBmgeCKXWVRtd=1201683073; else xvzKumKocPDMvtBWSVBmgeCKXWVRtd=368260277;if (xvzKumKocPDMvtBWSVBmgeCKXWVRtd == xvzKumKocPDMvtBWSVBmgeCKXWVRtd- 1 ) xvzKumKocPDMvtBWSVBmgeCKXWVRtd=1757124536; else xvzKumKocPDMvtBWSVBmgeCKXWVRtd=906251510;int sWUqlxmmRGxQzuqAlKDmwaoelcZjxT=1150971897;if (sWUqlxmmRGxQzuqAlKDmwaoelcZjxT == sWUqlxmmRGxQzuqAlKDmwaoelcZjxT- 1 ) sWUqlxmmRGxQzuqAlKDmwaoelcZjxT=1006979781; else sWUqlxmmRGxQzuqAlKDmwaoelcZjxT=644407432;if (sWUqlxmmRGxQzuqAlKDmwaoelcZjxT == sWUqlxmmRGxQzuqAlKDmwaoelcZjxT- 0 ) sWUqlxmmRGxQzuqAlKDmwaoelcZjxT=1093314063; else sWUqlxmmRGxQzuqAlKDmwaoelcZjxT=676548139;if (sWUqlxmmRGxQzuqAlKDmwaoelcZjxT == sWUqlxmmRGxQzuqAlKDmwaoelcZjxT- 1 ) sWUqlxmmRGxQzuqAlKDmwaoelcZjxT=1478580944; else sWUqlxmmRGxQzuqAlKDmwaoelcZjxT=2136350802;if (sWUqlxmmRGxQzuqAlKDmwaoelcZjxT == sWUqlxmmRGxQzuqAlKDmwaoelcZjxT- 1 ) sWUqlxmmRGxQzuqAlKDmwaoelcZjxT=1064129075; else sWUqlxmmRGxQzuqAlKDmwaoelcZjxT=1469484498;if (sWUqlxmmRGxQzuqAlKDmwaoelcZjxT == sWUqlxmmRGxQzuqAlKDmwaoelcZjxT- 1 ) sWUqlxmmRGxQzuqAlKDmwaoelcZjxT=389737597; else sWUqlxmmRGxQzuqAlKDmwaoelcZjxT=1125150551;if (sWUqlxmmRGxQzuqAlKDmwaoelcZjxT == sWUqlxmmRGxQzuqAlKDmwaoelcZjxT- 0 ) sWUqlxmmRGxQzuqAlKDmwaoelcZjxT=223644803; else sWUqlxmmRGxQzuqAlKDmwaoelcZjxT=848431446;float RSjZfgORjTobzIXdIglUfcIWUFsqrf=427513137.733125027425871345598393031806f;if (RSjZfgORjTobzIXdIglUfcIWUFsqrf - RSjZfgORjTobzIXdIglUfcIWUFsqrf> 0.00000001 ) RSjZfgORjTobzIXdIglUfcIWUFsqrf=1142415664.062801834195458081925056745119f; else RSjZfgORjTobzIXdIglUfcIWUFsqrf=942394509.433146307062241404516198112616f;if (RSjZfgORjTobzIXdIglUfcIWUFsqrf - RSjZfgORjTobzIXdIglUfcIWUFsqrf> 0.00000001 ) RSjZfgORjTobzIXdIglUfcIWUFsqrf=1298726115.772576331388407482645171444514f; else RSjZfgORjTobzIXdIglUfcIWUFsqrf=1953241206.570249077527324979273118161224f;if (RSjZfgORjTobzIXdIglUfcIWUFsqrf - RSjZfgORjTobzIXdIglUfcIWUFsqrf> 0.00000001 ) RSjZfgORjTobzIXdIglUfcIWUFsqrf=847275096.383891927851512985767534146218f; else RSjZfgORjTobzIXdIglUfcIWUFsqrf=2004195139.986334464306352361638657885985f;if (RSjZfgORjTobzIXdIglUfcIWUFsqrf - RSjZfgORjTobzIXdIglUfcIWUFsqrf> 0.00000001 ) RSjZfgORjTobzIXdIglUfcIWUFsqrf=1770553001.672907629953246239299669134965f; else RSjZfgORjTobzIXdIglUfcIWUFsqrf=633392722.969983445675288541266679702397f;if (RSjZfgORjTobzIXdIglUfcIWUFsqrf - RSjZfgORjTobzIXdIglUfcIWUFsqrf> 0.00000001 ) RSjZfgORjTobzIXdIglUfcIWUFsqrf=224136218.476400547389472448160380039039f; else RSjZfgORjTobzIXdIglUfcIWUFsqrf=1302305063.226930998911509731837433820747f;if (RSjZfgORjTobzIXdIglUfcIWUFsqrf - RSjZfgORjTobzIXdIglUfcIWUFsqrf> 0.00000001 ) RSjZfgORjTobzIXdIglUfcIWUFsqrf=1894613196.251817365265126989241830380421f; else RSjZfgORjTobzIXdIglUfcIWUFsqrf=44186877.136287079347768241804151743264f;double wVSEQyuVutDBwyTHhKFUsgKkznbZVS=961102334.974951822648911383751220823952;if (wVSEQyuVutDBwyTHhKFUsgKkznbZVS == wVSEQyuVutDBwyTHhKFUsgKkznbZVS ) wVSEQyuVutDBwyTHhKFUsgKkznbZVS=912997634.951119340700800828123725086423; else wVSEQyuVutDBwyTHhKFUsgKkznbZVS=285112973.897189881205305242101693924723;if (wVSEQyuVutDBwyTHhKFUsgKkznbZVS == wVSEQyuVutDBwyTHhKFUsgKkznbZVS ) wVSEQyuVutDBwyTHhKFUsgKkznbZVS=1087107055.302701383114893843084542231526; else wVSEQyuVutDBwyTHhKFUsgKkznbZVS=115244330.617801471507900890128275096057;if (wVSEQyuVutDBwyTHhKFUsgKkznbZVS == wVSEQyuVutDBwyTHhKFUsgKkznbZVS ) wVSEQyuVutDBwyTHhKFUsgKkznbZVS=872971491.097760810258269764255240955381; else wVSEQyuVutDBwyTHhKFUsgKkznbZVS=505676785.542452025118108101633381395791;if (wVSEQyuVutDBwyTHhKFUsgKkznbZVS == wVSEQyuVutDBwyTHhKFUsgKkznbZVS ) wVSEQyuVutDBwyTHhKFUsgKkznbZVS=2138393877.664971782874025725386475937126; else wVSEQyuVutDBwyTHhKFUsgKkznbZVS=1819191182.744465523752097937768041620915;if (wVSEQyuVutDBwyTHhKFUsgKkznbZVS == wVSEQyuVutDBwyTHhKFUsgKkznbZVS ) wVSEQyuVutDBwyTHhKFUsgKkznbZVS=980739862.567614167020374448900156970394; else wVSEQyuVutDBwyTHhKFUsgKkznbZVS=350263660.662939549397084888829212552184;if (wVSEQyuVutDBwyTHhKFUsgKkznbZVS == wVSEQyuVutDBwyTHhKFUsgKkznbZVS ) wVSEQyuVutDBwyTHhKFUsgKkznbZVS=1749561836.597446854690062317319268656149; else wVSEQyuVutDBwyTHhKFUsgKkznbZVS=302609705.857590074814433935024597534090;float lAbnXPanIPWQVBboPdjIBDNaPfBwde=556281980.207730883401205039899739648703f;if (lAbnXPanIPWQVBboPdjIBDNaPfBwde - lAbnXPanIPWQVBboPdjIBDNaPfBwde> 0.00000001 ) lAbnXPanIPWQVBboPdjIBDNaPfBwde=1540075740.377548769127589673487299258810f; else lAbnXPanIPWQVBboPdjIBDNaPfBwde=1329611399.328113487738178335117543628939f;if (lAbnXPanIPWQVBboPdjIBDNaPfBwde - lAbnXPanIPWQVBboPdjIBDNaPfBwde> 0.00000001 ) lAbnXPanIPWQVBboPdjIBDNaPfBwde=1911319493.195991957629675022193481388650f; else lAbnXPanIPWQVBboPdjIBDNaPfBwde=1986909132.565875329858850898870785782572f;if (lAbnXPanIPWQVBboPdjIBDNaPfBwde - lAbnXPanIPWQVBboPdjIBDNaPfBwde> 0.00000001 ) lAbnXPanIPWQVBboPdjIBDNaPfBwde=1290009142.331727591170435663159199098029f; else lAbnXPanIPWQVBboPdjIBDNaPfBwde=293388193.395752714341880136112943335373f;if (lAbnXPanIPWQVBboPdjIBDNaPfBwde - lAbnXPanIPWQVBboPdjIBDNaPfBwde> 0.00000001 ) lAbnXPanIPWQVBboPdjIBDNaPfBwde=303696801.067288578396092841906301746353f; else lAbnXPanIPWQVBboPdjIBDNaPfBwde=1150268289.754149560376883706260448776654f;if (lAbnXPanIPWQVBboPdjIBDNaPfBwde - lAbnXPanIPWQVBboPdjIBDNaPfBwde> 0.00000001 ) lAbnXPanIPWQVBboPdjIBDNaPfBwde=1021338078.223665295448274163914129442747f; else lAbnXPanIPWQVBboPdjIBDNaPfBwde=459921341.302615422952183237472475021433f;if (lAbnXPanIPWQVBboPdjIBDNaPfBwde - lAbnXPanIPWQVBboPdjIBDNaPfBwde> 0.00000001 ) lAbnXPanIPWQVBboPdjIBDNaPfBwde=452411921.465815767074116304721537603869f; else lAbnXPanIPWQVBboPdjIBDNaPfBwde=176275881.500960748968151937982782394931f;double uwnanWLcgeihrdQTnhkmftrnpJqgfe=1265752457.118754414378025014430508655636;if (uwnanWLcgeihrdQTnhkmftrnpJqgfe == uwnanWLcgeihrdQTnhkmftrnpJqgfe ) uwnanWLcgeihrdQTnhkmftrnpJqgfe=1912515937.673828626876808491906281961020; else uwnanWLcgeihrdQTnhkmftrnpJqgfe=1410026116.989880530750967471127573149306;if (uwnanWLcgeihrdQTnhkmftrnpJqgfe == uwnanWLcgeihrdQTnhkmftrnpJqgfe ) uwnanWLcgeihrdQTnhkmftrnpJqgfe=2082917459.760699813806136642550100452784; else uwnanWLcgeihrdQTnhkmftrnpJqgfe=1563017467.134419037877842155750885767135;if (uwnanWLcgeihrdQTnhkmftrnpJqgfe == uwnanWLcgeihrdQTnhkmftrnpJqgfe ) uwnanWLcgeihrdQTnhkmftrnpJqgfe=1705777685.823977382772579347831704446986; else uwnanWLcgeihrdQTnhkmftrnpJqgfe=860423854.301315598697419931743030513063;if (uwnanWLcgeihrdQTnhkmftrnpJqgfe == uwnanWLcgeihrdQTnhkmftrnpJqgfe ) uwnanWLcgeihrdQTnhkmftrnpJqgfe=1702025293.240747137495034440562534438180; else uwnanWLcgeihrdQTnhkmftrnpJqgfe=636738879.354545887390535810728450172741;if (uwnanWLcgeihrdQTnhkmftrnpJqgfe == uwnanWLcgeihrdQTnhkmftrnpJqgfe ) uwnanWLcgeihrdQTnhkmftrnpJqgfe=951484279.905715250976338310963957200457; else uwnanWLcgeihrdQTnhkmftrnpJqgfe=1627018580.414649583177186213397288916961;if (uwnanWLcgeihrdQTnhkmftrnpJqgfe == uwnanWLcgeihrdQTnhkmftrnpJqgfe ) uwnanWLcgeihrdQTnhkmftrnpJqgfe=972501344.581366379084890788200826134783; else uwnanWLcgeihrdQTnhkmftrnpJqgfe=497011436.373270682004441340933876287533;float ezegHHhIzBCcHBdQmZmvTnXBZwjyvP=70842495.102156539535471382253906660487f;if (ezegHHhIzBCcHBdQmZmvTnXBZwjyvP - ezegHHhIzBCcHBdQmZmvTnXBZwjyvP> 0.00000001 ) ezegHHhIzBCcHBdQmZmvTnXBZwjyvP=436653109.335520151206028116943931720750f; else ezegHHhIzBCcHBdQmZmvTnXBZwjyvP=1338331216.626330006233116809866464451590f;if (ezegHHhIzBCcHBdQmZmvTnXBZwjyvP - ezegHHhIzBCcHBdQmZmvTnXBZwjyvP> 0.00000001 ) ezegHHhIzBCcHBdQmZmvTnXBZwjyvP=651594784.050168070911182092693299537399f; else ezegHHhIzBCcHBdQmZmvTnXBZwjyvP=446855897.951728405883006548356900796274f;if (ezegHHhIzBCcHBdQmZmvTnXBZwjyvP - ezegHHhIzBCcHBdQmZmvTnXBZwjyvP> 0.00000001 ) ezegHHhIzBCcHBdQmZmvTnXBZwjyvP=1507632941.289106046479782120409905306214f; else ezegHHhIzBCcHBdQmZmvTnXBZwjyvP=1520692776.323261490050398832967788548491f;if (ezegHHhIzBCcHBdQmZmvTnXBZwjyvP - ezegHHhIzBCcHBdQmZmvTnXBZwjyvP> 0.00000001 ) ezegHHhIzBCcHBdQmZmvTnXBZwjyvP=1818576839.194325662184056962858435765673f; else ezegHHhIzBCcHBdQmZmvTnXBZwjyvP=619609441.300096718337089474368775377066f;if (ezegHHhIzBCcHBdQmZmvTnXBZwjyvP - ezegHHhIzBCcHBdQmZmvTnXBZwjyvP> 0.00000001 ) ezegHHhIzBCcHBdQmZmvTnXBZwjyvP=2128899819.109098416036453245640241664682f; else ezegHHhIzBCcHBdQmZmvTnXBZwjyvP=2084322413.075323919207834242224243607844f;if (ezegHHhIzBCcHBdQmZmvTnXBZwjyvP - ezegHHhIzBCcHBdQmZmvTnXBZwjyvP> 0.00000001 ) ezegHHhIzBCcHBdQmZmvTnXBZwjyvP=1241389899.594506223133654451843256994273f; else ezegHHhIzBCcHBdQmZmvTnXBZwjyvP=413050150.717682343813029169036611144862f;long AXCUMfAoIAFpBPCKQlsNaIRlFChcUp=2108281311;if (AXCUMfAoIAFpBPCKQlsNaIRlFChcUp == AXCUMfAoIAFpBPCKQlsNaIRlFChcUp- 1 ) AXCUMfAoIAFpBPCKQlsNaIRlFChcUp=657246458; else AXCUMfAoIAFpBPCKQlsNaIRlFChcUp=483809379;if (AXCUMfAoIAFpBPCKQlsNaIRlFChcUp == AXCUMfAoIAFpBPCKQlsNaIRlFChcUp- 1 ) AXCUMfAoIAFpBPCKQlsNaIRlFChcUp=537577415; else AXCUMfAoIAFpBPCKQlsNaIRlFChcUp=1289069703;if (AXCUMfAoIAFpBPCKQlsNaIRlFChcUp == AXCUMfAoIAFpBPCKQlsNaIRlFChcUp- 0 ) AXCUMfAoIAFpBPCKQlsNaIRlFChcUp=217859911; else AXCUMfAoIAFpBPCKQlsNaIRlFChcUp=1450118134;if (AXCUMfAoIAFpBPCKQlsNaIRlFChcUp == AXCUMfAoIAFpBPCKQlsNaIRlFChcUp- 1 ) AXCUMfAoIAFpBPCKQlsNaIRlFChcUp=997987942; else AXCUMfAoIAFpBPCKQlsNaIRlFChcUp=679651886;if (AXCUMfAoIAFpBPCKQlsNaIRlFChcUp == AXCUMfAoIAFpBPCKQlsNaIRlFChcUp- 0 ) AXCUMfAoIAFpBPCKQlsNaIRlFChcUp=1775069753; else AXCUMfAoIAFpBPCKQlsNaIRlFChcUp=1274580614;if (AXCUMfAoIAFpBPCKQlsNaIRlFChcUp == AXCUMfAoIAFpBPCKQlsNaIRlFChcUp- 1 ) AXCUMfAoIAFpBPCKQlsNaIRlFChcUp=1035424853; else AXCUMfAoIAFpBPCKQlsNaIRlFChcUp=1810474209;double rmysjAzVOXBKxAmgrgCXTLXkTgUDTH=1700878429.751621218508737483360691551777;if (rmysjAzVOXBKxAmgrgCXTLXkTgUDTH == rmysjAzVOXBKxAmgrgCXTLXkTgUDTH ) rmysjAzVOXBKxAmgrgCXTLXkTgUDTH=378140791.152796224006785090659374317017; else rmysjAzVOXBKxAmgrgCXTLXkTgUDTH=381569426.065245714137047592766212889848;if (rmysjAzVOXBKxAmgrgCXTLXkTgUDTH == rmysjAzVOXBKxAmgrgCXTLXkTgUDTH ) rmysjAzVOXBKxAmgrgCXTLXkTgUDTH=1887106419.921705154149663344892645286356; else rmysjAzVOXBKxAmgrgCXTLXkTgUDTH=1050890037.797731644345863866065616639763;if (rmysjAzVOXBKxAmgrgCXTLXkTgUDTH == rmysjAzVOXBKxAmgrgCXTLXkTgUDTH ) rmysjAzVOXBKxAmgrgCXTLXkTgUDTH=368347414.991955203370065898343140129210; else rmysjAzVOXBKxAmgrgCXTLXkTgUDTH=1393644952.309246502866379665195152086183;if (rmysjAzVOXBKxAmgrgCXTLXkTgUDTH == rmysjAzVOXBKxAmgrgCXTLXkTgUDTH ) rmysjAzVOXBKxAmgrgCXTLXkTgUDTH=1905603163.700418192040658797266422526826; else rmysjAzVOXBKxAmgrgCXTLXkTgUDTH=391823161.042016797158316650440902749809;if (rmysjAzVOXBKxAmgrgCXTLXkTgUDTH == rmysjAzVOXBKxAmgrgCXTLXkTgUDTH ) rmysjAzVOXBKxAmgrgCXTLXkTgUDTH=1738932924.347610874210011912707649715038; else rmysjAzVOXBKxAmgrgCXTLXkTgUDTH=1463308018.890892170788623009293720370900;if (rmysjAzVOXBKxAmgrgCXTLXkTgUDTH == rmysjAzVOXBKxAmgrgCXTLXkTgUDTH ) rmysjAzVOXBKxAmgrgCXTLXkTgUDTH=1581601295.695753133095213003672694517062; else rmysjAzVOXBKxAmgrgCXTLXkTgUDTH=1768938919.238110408107721962838833810124;float DgGpdjExPeBZclZvmTgaYLOWWxXQCz=1136849930.272645543785571676303470804699f;if (DgGpdjExPeBZclZvmTgaYLOWWxXQCz - DgGpdjExPeBZclZvmTgaYLOWWxXQCz> 0.00000001 ) DgGpdjExPeBZclZvmTgaYLOWWxXQCz=49565004.689360936155610500022183090395f; else DgGpdjExPeBZclZvmTgaYLOWWxXQCz=1011703680.196510632285036996030577751648f;if (DgGpdjExPeBZclZvmTgaYLOWWxXQCz - DgGpdjExPeBZclZvmTgaYLOWWxXQCz> 0.00000001 ) DgGpdjExPeBZclZvmTgaYLOWWxXQCz=1987248625.064206021098885826811537573987f; else DgGpdjExPeBZclZvmTgaYLOWWxXQCz=1918307571.009522183008614279304593864873f;if (DgGpdjExPeBZclZvmTgaYLOWWxXQCz - DgGpdjExPeBZclZvmTgaYLOWWxXQCz> 0.00000001 ) DgGpdjExPeBZclZvmTgaYLOWWxXQCz=930365881.424410047474392393330569352687f; else DgGpdjExPeBZclZvmTgaYLOWWxXQCz=940194494.831078420534203130157265496627f;if (DgGpdjExPeBZclZvmTgaYLOWWxXQCz - DgGpdjExPeBZclZvmTgaYLOWWxXQCz> 0.00000001 ) DgGpdjExPeBZclZvmTgaYLOWWxXQCz=2000466231.847164333408381637633354078778f; else DgGpdjExPeBZclZvmTgaYLOWWxXQCz=1134018622.391489824980885999486909460410f;if (DgGpdjExPeBZclZvmTgaYLOWWxXQCz - DgGpdjExPeBZclZvmTgaYLOWWxXQCz> 0.00000001 ) DgGpdjExPeBZclZvmTgaYLOWWxXQCz=340343274.767522801292475358918049249203f; else DgGpdjExPeBZclZvmTgaYLOWWxXQCz=2119426545.051433446420930564521251283838f;if (DgGpdjExPeBZclZvmTgaYLOWWxXQCz - DgGpdjExPeBZclZvmTgaYLOWWxXQCz> 0.00000001 ) DgGpdjExPeBZclZvmTgaYLOWWxXQCz=819269791.360925105392259911079688290125f; else DgGpdjExPeBZclZvmTgaYLOWWxXQCz=233052327.071916011556923001247286989273f;double yoAMMBRUQqEyYLNgdZHuwvoopsbyOl=174730807.545819480474538937168805051341;if (yoAMMBRUQqEyYLNgdZHuwvoopsbyOl == yoAMMBRUQqEyYLNgdZHuwvoopsbyOl ) yoAMMBRUQqEyYLNgdZHuwvoopsbyOl=1188687427.136180585050524331113698174197; else yoAMMBRUQqEyYLNgdZHuwvoopsbyOl=1687995117.825044855845311938173133855902;if (yoAMMBRUQqEyYLNgdZHuwvoopsbyOl == yoAMMBRUQqEyYLNgdZHuwvoopsbyOl ) yoAMMBRUQqEyYLNgdZHuwvoopsbyOl=34909359.750934569058637948724602579320; else yoAMMBRUQqEyYLNgdZHuwvoopsbyOl=1819179085.005420649857022551601739815956;if (yoAMMBRUQqEyYLNgdZHuwvoopsbyOl == yoAMMBRUQqEyYLNgdZHuwvoopsbyOl ) yoAMMBRUQqEyYLNgdZHuwvoopsbyOl=75011577.535006321841998778003972938772; else yoAMMBRUQqEyYLNgdZHuwvoopsbyOl=526563539.579442302724097095114796476418;if (yoAMMBRUQqEyYLNgdZHuwvoopsbyOl == yoAMMBRUQqEyYLNgdZHuwvoopsbyOl ) yoAMMBRUQqEyYLNgdZHuwvoopsbyOl=1739933897.975037266658739908605467888408; else yoAMMBRUQqEyYLNgdZHuwvoopsbyOl=1411085153.799595907093066717857982351925;if (yoAMMBRUQqEyYLNgdZHuwvoopsbyOl == yoAMMBRUQqEyYLNgdZHuwvoopsbyOl ) yoAMMBRUQqEyYLNgdZHuwvoopsbyOl=725689863.996807935410262351228276181824; else yoAMMBRUQqEyYLNgdZHuwvoopsbyOl=145754254.518220726089254873111071872182;if (yoAMMBRUQqEyYLNgdZHuwvoopsbyOl == yoAMMBRUQqEyYLNgdZHuwvoopsbyOl ) yoAMMBRUQqEyYLNgdZHuwvoopsbyOl=1697620426.621191205263634013241972354855; else yoAMMBRUQqEyYLNgdZHuwvoopsbyOl=902035540.974457826468906855787995275085;double XbxNswJqkHulpnTzoeZzsBjpypuMlM=486094610.340671464794400866235591070182;if (XbxNswJqkHulpnTzoeZzsBjpypuMlM == XbxNswJqkHulpnTzoeZzsBjpypuMlM ) XbxNswJqkHulpnTzoeZzsBjpypuMlM=691106599.625445864348533631966229895070; else XbxNswJqkHulpnTzoeZzsBjpypuMlM=2046058570.148218907501390816945033650765;if (XbxNswJqkHulpnTzoeZzsBjpypuMlM == XbxNswJqkHulpnTzoeZzsBjpypuMlM ) XbxNswJqkHulpnTzoeZzsBjpypuMlM=1760909205.099431043583456693008013621667; else XbxNswJqkHulpnTzoeZzsBjpypuMlM=456666816.824252297098447269716787818692;if (XbxNswJqkHulpnTzoeZzsBjpypuMlM == XbxNswJqkHulpnTzoeZzsBjpypuMlM ) XbxNswJqkHulpnTzoeZzsBjpypuMlM=2114404034.048776930551702178756396125854; else XbxNswJqkHulpnTzoeZzsBjpypuMlM=1124811062.561605077134937608575095676366;if (XbxNswJqkHulpnTzoeZzsBjpypuMlM == XbxNswJqkHulpnTzoeZzsBjpypuMlM ) XbxNswJqkHulpnTzoeZzsBjpypuMlM=770512548.128619110820172357062849057714; else XbxNswJqkHulpnTzoeZzsBjpypuMlM=1585429944.071202733869885603333941448976;if (XbxNswJqkHulpnTzoeZzsBjpypuMlM == XbxNswJqkHulpnTzoeZzsBjpypuMlM ) XbxNswJqkHulpnTzoeZzsBjpypuMlM=528586842.241795907072521317546548114187; else XbxNswJqkHulpnTzoeZzsBjpypuMlM=753087797.487762910180736242013254181468;if (XbxNswJqkHulpnTzoeZzsBjpypuMlM == XbxNswJqkHulpnTzoeZzsBjpypuMlM ) XbxNswJqkHulpnTzoeZzsBjpypuMlM=710546763.182079597050847691207318434469; else XbxNswJqkHulpnTzoeZzsBjpypuMlM=396404516.166200877776617451750027197574;int HQprwXcYjoKWYQIiAFCnOiCtHUuEDN=1730830294;if (HQprwXcYjoKWYQIiAFCnOiCtHUuEDN == HQprwXcYjoKWYQIiAFCnOiCtHUuEDN- 0 ) HQprwXcYjoKWYQIiAFCnOiCtHUuEDN=164585426; else HQprwXcYjoKWYQIiAFCnOiCtHUuEDN=1406870488;if (HQprwXcYjoKWYQIiAFCnOiCtHUuEDN == HQprwXcYjoKWYQIiAFCnOiCtHUuEDN- 1 ) HQprwXcYjoKWYQIiAFCnOiCtHUuEDN=580251492; else HQprwXcYjoKWYQIiAFCnOiCtHUuEDN=78522485;if (HQprwXcYjoKWYQIiAFCnOiCtHUuEDN == HQprwXcYjoKWYQIiAFCnOiCtHUuEDN- 0 ) HQprwXcYjoKWYQIiAFCnOiCtHUuEDN=1254065784; else HQprwXcYjoKWYQIiAFCnOiCtHUuEDN=911778577;if (HQprwXcYjoKWYQIiAFCnOiCtHUuEDN == HQprwXcYjoKWYQIiAFCnOiCtHUuEDN- 1 ) HQprwXcYjoKWYQIiAFCnOiCtHUuEDN=1061838353; else HQprwXcYjoKWYQIiAFCnOiCtHUuEDN=1670485112;if (HQprwXcYjoKWYQIiAFCnOiCtHUuEDN == HQprwXcYjoKWYQIiAFCnOiCtHUuEDN- 0 ) HQprwXcYjoKWYQIiAFCnOiCtHUuEDN=1419253430; else HQprwXcYjoKWYQIiAFCnOiCtHUuEDN=576163472;if (HQprwXcYjoKWYQIiAFCnOiCtHUuEDN == HQprwXcYjoKWYQIiAFCnOiCtHUuEDN- 0 ) HQprwXcYjoKWYQIiAFCnOiCtHUuEDN=1192617708; else HQprwXcYjoKWYQIiAFCnOiCtHUuEDN=784611627;float EvhxgImcYGCbBtGyjDJGVhPRlNmYNv=1111294251.208585143562576133879538687159f;if (EvhxgImcYGCbBtGyjDJGVhPRlNmYNv - EvhxgImcYGCbBtGyjDJGVhPRlNmYNv> 0.00000001 ) EvhxgImcYGCbBtGyjDJGVhPRlNmYNv=1058808614.717374541757443990471871287091f; else EvhxgImcYGCbBtGyjDJGVhPRlNmYNv=333175181.800496630260213310679788995720f;if (EvhxgImcYGCbBtGyjDJGVhPRlNmYNv - EvhxgImcYGCbBtGyjDJGVhPRlNmYNv> 0.00000001 ) EvhxgImcYGCbBtGyjDJGVhPRlNmYNv=1689429375.765195716847021176807853213250f; else EvhxgImcYGCbBtGyjDJGVhPRlNmYNv=586693000.202061512960484386893992491208f;if (EvhxgImcYGCbBtGyjDJGVhPRlNmYNv - EvhxgImcYGCbBtGyjDJGVhPRlNmYNv> 0.00000001 ) EvhxgImcYGCbBtGyjDJGVhPRlNmYNv=394677072.356764607076404757094757731077f; else EvhxgImcYGCbBtGyjDJGVhPRlNmYNv=842055776.042471583088083760975531561004f;if (EvhxgImcYGCbBtGyjDJGVhPRlNmYNv - EvhxgImcYGCbBtGyjDJGVhPRlNmYNv> 0.00000001 ) EvhxgImcYGCbBtGyjDJGVhPRlNmYNv=207810420.363103504338512787508221261101f; else EvhxgImcYGCbBtGyjDJGVhPRlNmYNv=63876365.168972598037764813089450715295f;if (EvhxgImcYGCbBtGyjDJGVhPRlNmYNv - EvhxgImcYGCbBtGyjDJGVhPRlNmYNv> 0.00000001 ) EvhxgImcYGCbBtGyjDJGVhPRlNmYNv=917482569.240438064039068166845913271836f; else EvhxgImcYGCbBtGyjDJGVhPRlNmYNv=596963062.831480744685916150366955043826f;if (EvhxgImcYGCbBtGyjDJGVhPRlNmYNv - EvhxgImcYGCbBtGyjDJGVhPRlNmYNv> 0.00000001 ) EvhxgImcYGCbBtGyjDJGVhPRlNmYNv=1290592243.834830969853327101722257199645f; else EvhxgImcYGCbBtGyjDJGVhPRlNmYNv=1469407427.702896228729151373904211468785f;int QLVFbEXRDttODwPDydnHdiTEYsGVPp=1200622674;if (QLVFbEXRDttODwPDydnHdiTEYsGVPp == QLVFbEXRDttODwPDydnHdiTEYsGVPp- 1 ) QLVFbEXRDttODwPDydnHdiTEYsGVPp=239199016; else QLVFbEXRDttODwPDydnHdiTEYsGVPp=2114273779;if (QLVFbEXRDttODwPDydnHdiTEYsGVPp == QLVFbEXRDttODwPDydnHdiTEYsGVPp- 0 ) QLVFbEXRDttODwPDydnHdiTEYsGVPp=737450114; else QLVFbEXRDttODwPDydnHdiTEYsGVPp=609129998;if (QLVFbEXRDttODwPDydnHdiTEYsGVPp == QLVFbEXRDttODwPDydnHdiTEYsGVPp- 0 ) QLVFbEXRDttODwPDydnHdiTEYsGVPp=796281178; else QLVFbEXRDttODwPDydnHdiTEYsGVPp=982095446;if (QLVFbEXRDttODwPDydnHdiTEYsGVPp == QLVFbEXRDttODwPDydnHdiTEYsGVPp- 1 ) QLVFbEXRDttODwPDydnHdiTEYsGVPp=1379512473; else QLVFbEXRDttODwPDydnHdiTEYsGVPp=1926811669;if (QLVFbEXRDttODwPDydnHdiTEYsGVPp == QLVFbEXRDttODwPDydnHdiTEYsGVPp- 0 ) QLVFbEXRDttODwPDydnHdiTEYsGVPp=984220217; else QLVFbEXRDttODwPDydnHdiTEYsGVPp=90424093;if (QLVFbEXRDttODwPDydnHdiTEYsGVPp == QLVFbEXRDttODwPDydnHdiTEYsGVPp- 1 ) QLVFbEXRDttODwPDydnHdiTEYsGVPp=1184897033; else QLVFbEXRDttODwPDydnHdiTEYsGVPp=1538240562; }
 QLVFbEXRDttODwPDydnHdiTEYsGVPpy::QLVFbEXRDttODwPDydnHdiTEYsGVPpy()
 { this->ICBKECynXhnO("USZtiiTdJLvpFWvAFDZoFyPdisQgjxICBKECynXhnOj", true, 2079677001, 2133384568, 195372229); }
#pragma optimize("", off)
 // <delete/>

