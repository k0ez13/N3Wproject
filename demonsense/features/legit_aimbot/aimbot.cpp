#include "aimbot.h"
#include "autowall.h"
#include "../../imgui/imgui.h"
#include "../../render/render.h"
#include "../../menu/menu.h"
#include "backtrack.h"
#include "../../helpers/math.h"
#include "../../helpers/input.h"
#include "backtrack.h"

bool is_delayed = false;
float shot_delay_time;
bool shot_delay = false;

bool HitChance(QAngle angles, c_base_player* ent, float chance)
{
	auto RandomFloat = [](float a, float b) {
		float random = ((float)rand()) / (float)RAND_MAX;
		float diff = b - a;
		float r = random * diff;
		return a + r;
	};
	auto weapon = g_local_player->m_hActiveWeapon().get();

	if (!weapon)
		return false;

	Vector forward, right, up;
	Vector src = g_local_player->get_eye_pos();
	math::angle_vectors(angles, forward, right, up);

	int cHits = 0;
	int cNeededHits = static_cast<int> (150.f * (chance / 100.f));

	weapon->update_accuracy_penalty();
	float weap_spread = weapon->get_spread();
	float weap_inaccuracy = weapon->get_spread();

	for (int i = 0; i < 150; i++)
	{
		float a = RandomFloat(0.f, 1.f);
		float b = RandomFloat(0.f, 2.f * PI_F);
		float c = RandomFloat(0.f, 1.f);
		float d = RandomFloat(0.f, 2.f * PI_F);

		float inaccuracy = a * weap_inaccuracy;
		float spread = c * weap_spread;

		if (weapon->m_Item().m_iItemDefinitionIndex() == 64)
		{
			a = 1.f - a * a;
			a = 1.f - c * c;
		}

		Vector spreadView((cos(b) * inaccuracy) + (cos(d) * spread), (sin(b) * inaccuracy) + (sin(d) * spread), 0), direction;

		direction.x = forward.x + (spreadView.x * right.x) + (spreadView.y * up.x);
		direction.y = forward.y + (spreadView.x * right.y) + (spreadView.y * up.y);
		direction.z = forward.z + (spreadView.x * right.z) + (spreadView.y * up.z);
		direction.Normalized();

		QAngle viewAnglesSpread;
		math::vector_angles(direction, viewAnglesSpread);
		viewAnglesSpread.Normalize();

		Vector viewForward;
		math::angle_vectors(viewAnglesSpread, viewForward);
		viewForward.NormalizeInPlace();

		viewForward = src + (viewForward * weapon->get_cs_weapondata()->flRange);

		trace_t tr;
		ray_t ray;

		ray.init(src, viewForward);
		g_engine_trace->clip_ray_to_entity(ray, MASK_SHOT | CONTENTS_GRATE, ent, &tr);

		if (tr.hit_entity == ent)
			++cHits;

		if (static_cast<int> ((static_cast<float> (cHits) / 150.f) * 100.f) >= chance)
			return true;

		if ((150 - i + cHits) < cNeededHits)
			return false;
	}

	return false;
}


namespace legit_aimbot
{
	bool is_rcs()
	{
		return g_local_player->m_iShotsFired() >= _settings.rcs_start + 1;
	}

	float get_real_distance_fov(float distance, QAngle angle, QAngle viewangles)
	{
		Vector aimingAt;
		math::angle_vectors(viewangles, aimingAt);
		aimingAt *= distance;
		Vector aimAt;
		math::angle_vectors(angle, aimAt);
		aimAt *= distance;
		return aimingAt.DistTo(aimAt) / 5;
	}

	float get_fov_to_player(QAngle viewAngle, QAngle aimAngle)
	{
		QAngle delta = aimAngle - viewAngle;
		math::fix_angles(delta);
		return sqrtf(powf(delta.pitch, 2.0f) + powf(delta.yaw, 2.0f));
	}

	bool is_line_goes_through_smoke(Vector vStartPos, Vector vEndPos)
	{
		static auto LineGoesThroughSmokeFn = (bool(*)(Vector vStartPos, Vector vEndPos))utils::pattern_scan(GetModuleHandleA("client.dll"), "55 8B EC 83 EC 08 8B 15 ? ? ? ? 0F 57 C0");
		return LineGoesThroughSmokeFn(vStartPos, vEndPos);
	}
	int get_group()
	{
		auto pWeapon = g_local_player->m_hActiveWeapon();

		if (!pWeapon || !(pWeapon->is_sniper() || pWeapon->is_pistol() || pWeapon->is_rifle() || pWeapon->is_zeus() || pWeapon->is_knife()))
		{
			return 0;
		}
		if (pWeapon->is_zeus())
			return 0;
		else if (pWeapon->is_pistol())
			return 1;
		else if (pWeapon->get_cs_weapondata()->WeaponType == WEAPONTYPE_SUBMACHINEGUN)
			return 2;
		else if (pWeapon->get_cs_weapondata()->WeaponType == WEAPONTYPE_SHOTGUN)
			return 3;
		else if (pWeapon->get_cs_weapondata()->WeaponType == WEAPONTYPE_MACHINEGUN)
			return 4;
		else if (pWeapon->get_cs_weapondata()->WeaponType == WEAPONTYPE_RIFLE)
			return 5;
		else if (pWeapon->get_cs_weapondata()->WeaponType == WEAPONTYPE_SNIPER_RIFLE)
			return 6;
		else if (pWeapon->get_cs_weapondata()->WeaponType == WEAPONTYPE_KNIFE)
			return 7;
	}
	//--------------------------------------------------------------------------------
	bool is_enabled(c_usercmd* pCmd)
	{
		if (!g_engine_client->is_connected() || !g_local_player || !g_local_player->is_alive()) {
			return false;
		}

		auto pWeapon = g_local_player->m_hActiveWeapon();

		if (!pWeapon || !(pWeapon->is_sniper() || pWeapon->is_pistol() || pWeapon->is_rifle() || pWeapon->is_zeus() || pWeapon->is_knife()))
		{
			return false;
		}

		auto weaponData = pWeapon->get_cs_weapondata();
		auto weapontype = weaponData->WeaponType;

		if (settings::legit_bot::settings_category == 0)
			_settings = settings::legit_bot::legitbot_items_all;
		else if (settings::legit_bot::settings_category == 1)
			_settings = settings::legit_bot::legitbot_items_groups[get_group()];
		else if (settings::legit_bot::settings_category == 2)
			_settings = settings::legit_bot::legitbot_items[pWeapon->m_Item().m_iItemDefinitionIndex()];

		if (!_settings.enabled)
		{
			return false;
		}

		if ((pWeapon->m_Item().m_iItemDefinitionIndex() == WEAPON_AWP || pWeapon->m_Item().m_iItemDefinitionIndex() == WEAPON_SSG08 || pWeapon->m_Item().m_iItemDefinitionIndex() == WEAPON_SCAR20 || pWeapon->m_Item().m_iItemDefinitionIndex() == WEAPON_G3SG1) && _settings.only_in_zoom && !g_local_player->m_bIsScoped()) {
			return false;
		}

		if ((!pWeapon->is_knife() && !pWeapon->has_bullets()) || pWeapon->is_reloading()) {
			return false;
		}

		if (settings::legit_bot::bind_check && !settings::legit_bot::bind.enable)
			return false;
		//return !_settings.on_key || input_sys::is_key_down(_settings.key) || settings::triggerbot::magnetic & input_sys::is_key_down(settings::triggerbot::bind) || settings::triggerbot::autotrigger;

		//return pCmd->buttons & IN_ATTACK || (pWeapon->is_knife() && pCmd->buttons & IN_ATTACK2);
		//--------------------------------------------------------------------- TRIGGER MAG ------------------------------------------------------------------------//
		return !_settings.on_key || input_sys::is_key_down(_settings.key) || settings::triggerbot::magnetic & input_sys::is_key_down(settings::triggerbot::bind) || settings::triggerbot::autotrigger;
	}
	//--------------------------------------------------------------------------------------------------------------------------------------------------------------//
	float get_smooth() {
		float smooth = is_rcs() && _settings.rcs_smooth_enabled ? _settings.rcs_smooth : _settings.smooth;
		return smooth;
	}
	//--------------------------------------------------------------------------------------------------------------------------------------------------------------//
	void smooth(QAngle currentAngle, QAngle aimAngle, QAngle& angle) {
		auto smooth_value = get_smooth();
		if (smooth_value <= 1) {
			return;
		}

		QAngle delta = aimAngle - currentAngle;
		math::fix_angles(delta);

		// ������ �� ������� � �� ������
		if (_settings.smooth_type == 1)
		{
			float deltaLength = fmaxf(sqrtf((delta.pitch * delta.pitch) + (delta.yaw * delta.yaw)), 0.01f);
			delta *= (1.0f / deltaLength);

			RandomSeed(g_global_vars->tickcount);
			float randomize = RandomFloat(-0.1f, 0.1f);
			smooth_value = fminf((g_global_vars->interval_per_tick * 64.0f) / (randomize + smooth_value * 0.15f), deltaLength);
		}
		else
		{
			smooth_value = (g_global_vars->interval_per_tick * 64.0f) / smooth_value;
		}

		delta *= smooth_value;
		angle = currentAngle + delta;
		math::fix_angles(angle);
	}
	//--------------------------------------------------------------------------------
	void rcs(QAngle& angle, c_base_player* target, bool should_run) {
		if (!_settings.rcs) {
			rcs_last_punch.Init();
			return;
		}

		if (_settings.rcs_x == 0 && _settings.rcs_y == 0) {
			rcs_last_punch.Init();
			return;
		}

		QAngle punch = g_local_player->m_aimPunchAngle() * 2.0f;

		auto weapon = g_local_player->m_hActiveWeapon().get();
		if (weapon && weapon->m_flNextPrimaryAttack() > g_global_vars->curtime) {
			auto delta_angles = punch - rcs_last_punch;
			auto delta = weapon->m_flNextPrimaryAttack() - g_global_vars->curtime;
			if (delta >= g_global_vars->interval_per_tick)
				punch = rcs_last_punch + delta_angles / static_cast<float>(TIME_TO_TICKS(delta));
		}

		current_punch = punch;
		if (_settings.rcs_type == 0 && !should_run)
			punch -= { rcs_last_punch.pitch, rcs_last_punch.yaw, 0.f };

		rcs_last_punch = current_punch;
		if (!is_rcs()) {
			return;
		}

		angle.pitch -= punch.pitch * (_settings.rcs_x / 100.0f);
		angle.yaw -= punch.yaw * (_settings.rcs_y / 100.0f);

		math::fix_angles(angle);
	}
	//--------------------------------------------------------------------------------
	float get_fov()
	{
		if (is_rcs() && _settings.rcs && _settings.rcs_fov_enabled)
			return _settings.rcs_fov;

		if (!silent_enabled) return _settings.fov;

		return _settings.silent_fov > _settings.fov ? _settings.silent_fov : _settings.fov;
	}

	float without_silent()
	{
		if (is_rcs() && _settings.rcs && _settings.rcs_fov_enabled)
			return _settings.rcs_fov;

		return _settings.fov;
	}
	//--------------------------------------------------------------------------------
	c_base_player* get_closest_player(c_usercmd* cmd, int& bestBone) {
		QAngle ang;
		Vector eVecTarget;
		Vector pVecTarget = g_local_player->get_eye_pos();
		if (target && !kill_delay && _settings.kill_delay > 0 && target->is_not_target())
		{
			target = NULL;
			shot_delay = false;
			kill_delay = true;
			kill_delay_time = (int)GetTickCount() + _settings.kill_delay;
		}
		if (kill_delay) {
			if (kill_delay_time <= (int)GetTickCount()) kill_delay = false;
			else return NULL;
		}

		c_base_player* player;
		aim_position = Vector{ 0, 0, 0 };
		target = NULL;
		int bestHealth = 100.f;
		float bestFov = 9999.f;
		float bestDamage = 0.f;
		float bestBoneFov = 9999.f;
		float bestDistance = 9999.f;
		int health;
		float fov;
		float damage;
		float distance;

		bool hitbox_overrided = _settings.enable_hitbox_override && _settings.hitbox_override_bind.enable;

		int fromBone = 0;
		int toBone = 7;

		for (int i = 1; i < g_engine_client->get_max_clients(); ++i)
		{
			damage = 0.f;
			player = (c_base_player*)g_entity_list->get_client_entity(i);
			if (player->is_not_target()) {
				continue;
			}
			if (!_settings.deathmatch && player->m_iTeamNum() == g_local_player->m_iTeamNum()) {
				continue;
			}
			for (int bone = fromBone; bone <= toBone; bone++)
			{
				if (hitbox_overrided ? !_settings.override_hitboxes[bone] : !_settings.hitboxes[bone])
				{
					continue;
				}

				eVecTarget = player->get_hitbox_pos(bone);
				math::vector_angles(eVecTarget - pVecTarget, ang);
				math::fix_angles(ang);
				distance = pVecTarget.DistTo(eVecTarget);
				if (_settings.fov_type == 1)
					fov = get_real_distance_fov(distance, ang, cmd->viewangles + rcs_last_punch);
				else
					fov = get_fov_to_player(cmd->viewangles + rcs_last_punch, ang);

				bool backtrack_enable = backtrack::data.count(player->ent_index()) > 0 && _settings.aim_at_backtrack;

				QAngle backtrack_ang;
				Vector backtrack_eVecTarget;
				Vector backtrack_pVecTarget = g_local_player->get_eye_pos();
				float backtrack_fov = 0.f;
				float beastbacktrack_fov = 999999;
				Vector local_bb_bone_pos = Vector{ 0,0,0 };
				Vector bb_bone_pos = Vector{ 0,0,0 };

				if (backtrack_enable && !_settings.autowall && fov > get_fov())
				{
					auto& data = backtrack::data.at(player->ent_index());
					if (data.size() > 0)
					{
						Vector best_bb_bone_pos = Vector{ 0,0,0 };

						for (auto& record : data)
						{
							auto hitbox_head = record.hitboxset->get_hitbox(bone);
							auto hitbox_center = (hitbox_head->bbmin + hitbox_head->bbmax) * 0.5f;

							math::vector_transform(hitbox_center, record.boneMatrix[hitbox_head->bone], best_bb_bone_pos);

							backtrack_eVecTarget = best_bb_bone_pos;
							math::vector_angles(backtrack_eVecTarget - backtrack_pVecTarget, backtrack_ang);
							math::fix_angles(backtrack_ang);

							backtrack_fov = get_fov_to_player(cmd->viewangles + rcs_last_punch, backtrack_ang);

							if (backtrack_fov > get_fov())
								continue;

							if (beastbacktrack_fov > backtrack_fov)
							{
								beastbacktrack_fov = backtrack_fov;
								local_bb_bone_pos = best_bb_bone_pos;
							}
						}

						if (g_local_player->can_see_player(local_bb_bone_pos) > 0.9f)
						{
							bb_bone_pos = local_bb_bone_pos;
						}
					}
				}

				if (fov > get_fov() && (bb_bone_pos == Vector{ 0,0,0 } || !backtrack_enable))
					continue;

				if (!g_local_player->can_see_player(player, eVecTarget))
				{
					eVecTarget = player->get_hitbox_pos(bone);

					damage = autowall::get_damage(eVecTarget);

					if (bb_bone_pos == Vector{ 0,0,0 })
					{
						if (!_settings.autowall)
							continue;

						if (damage < _settings.min_damage)
							continue;
					}
				}
				if ((_settings.priority == 1 || _settings.priority == 2) && damage == 0.f)
					damage = autowall::get_damage(eVecTarget);

				health = player->m_iHealth() - damage;
				if (_settings.smoke_check && is_line_goes_through_smoke(pVecTarget, eVecTarget))
					continue;

				bool on_ground = (g_local_player->m_fFlags() & FL_ONGROUND);
				if (_settings.jump_check && !on_ground)
					continue;

				if (_settings.aim_type == 1 && bestBoneFov < fov)
				{
					continue;
				}
				bestBoneFov = fov;

				if (bb_bone_pos != Vector{ 0,0,0 })
				{
					aim_position = bb_bone_pos;
				}


				if (
					(_settings.priority == 0 && bestFov > fov) ||
					(_settings.priority == 1 && bestHealth > health) ||
					(_settings.priority == 2 && bestDamage < damage) ||
					(_settings.priority == 3 && distance < bestDistance)
					)
				{
					bestBone = bone;
					target = player;
					bestFov = fov;
					bestHealth = health;
					bestDamage = damage;
					bestDistance = distance;
				}

				//autowall on trigger
				if (settings::triggerbot::enable) {
					if (!settings::triggerbot::enable || input_sys::is_key_down(settings::triggerbot::bind))
					{

						if (HitChance(cmd->viewangles, player, settings::triggerbot::hitchance))
						{
							//Math::FixAngles(angles);
							cmd->buttons |= IN_ATTACK;
						}
					}
					else {
						if (HitChance(cmd->viewangles, player, settings::triggerbot::hitchance))
						{
							cmd->buttons |= IN_ATTACK;
						}
					}
				}


			}
		}
		return target;
	}
	//--------------------------------------------------------------------------------
	bool is_not_silent(float fov)
	{
		return is_rcs() || !silent_enabled || (silent_enabled && fov > _settings.silent_fov);
	}
	//--------------------------------------------------------------------------------
	void on_move(c_usercmd* pCmd)
	{
		if (!is_enabled(pCmd))
		{
			if (g_local_player && g_engine_client->is_ingame() && g_local_player->is_alive() && _settings.enabled && _settings.rcs_type == 0)
			{
				auto pWeapon = g_local_player->m_hActiveWeapon();
				if (pWeapon && (pWeapon->is_sniper() || pWeapon->is_pistol() || pWeapon->is_rifle()))
				{
					rcs(pCmd->viewangles, target, false);
					math::fix_angles(pCmd->viewangles);
					g_engine_client->set_view_angles(&pCmd->viewangles);
				}
			}
			else
			{
				rcs_last_punch = { 0, 0, 0 };
			}

			is_delayed = false;
			shot_delay = false;
			kill_delay = false;
			silent_enabled = _settings.silent && _settings.silent_fov > 0;
			target = NULL;
			return;
		}

		RandomSeed(pCmd->command_number);

		auto weapon = g_local_player->m_hActiveWeapon().get();
		if (!weapon)
			return;

		auto weapon_data = weapon->get_cs_weapondata();
		if (!weapon_data)
			return;

		bool should_do_rcs = false;
		QAngle angles = pCmd->viewangles;
		QAngle current = angles;
		float fov = 180.f;
		if (!(_settings.flash_check && g_local_player->is_flashed(_settings.flash_check_min)))
		{
			int bestBone = -1;
			if (get_closest_player(pCmd, bestBone))
			{
				Vector aim_pos = target->get_hitbox_pos(bestBone);

				if (aim_position != Vector{ 0,0,0 })
					aim_pos = aim_position;

				math::vector_angles(aim_pos - g_local_player->get_eye_pos(), angles);
				math::fix_angles(angles);
				if (_settings.fov_type == 1)
					fov = get_real_distance_fov(g_local_player->get_eye_pos().DistTo(target->get_hitbox_pos(bestBone)), angles, pCmd->viewangles);
				else
					fov = get_fov_to_player(pCmd->viewangles, angles);

				should_do_rcs = true;

				if (!_settings.silent && !is_delayed && !shot_delay && _settings.shot_delay > 0) {
					is_delayed = true;
					shot_delay = true;
					shot_delay_time = GetTickCount() + _settings.shot_delay;
				}

				if (shot_delay && shot_delay_time <= GetTickCount()) {
					shot_delay = false;
				}

				if (shot_delay) {
					pCmd->buttons &= ~IN_ATTACK;
				}

				if (_settings.autostop) {
					pCmd->forwardmove = pCmd->sidemove = 0;
				}
			}
		}

		if (is_not_silent(fov) && (should_do_rcs || _settings.rcs_type == 0)) {
			rcs(angles, target, should_do_rcs);
		}

		if (target && is_not_silent(fov))
		{
			smooth(current, angles, angles);
		}
		math::fix_angles(angles);
		if (_settings.silent_chance > utils::random(0, 100))
		{
			pCmd->viewangles = angles;
		}

		if (is_not_silent(fov))
		{
			g_engine_client->set_view_angles(&angles);
		}

		silent_enabled = false;
		if (g_local_player->m_hActiveWeapon()->is_pistol() && _settings.autopistol)
		{
			float server_time = g_local_player->m_nTickBase() * g_global_vars->interval_per_tick;
			float next_shot = g_local_player->m_hActiveWeapon()->m_flNextPrimaryAttack() - server_time;
			if (next_shot > 0) {
				pCmd->buttons &= ~IN_ATTACK;
			}
		}
	}
	//--------------------------------------------------------------------------------
	bool gitchance(float hitchance) {

		if (1 > hitchance)
			return true;

		float inaccuracy = g_local_player->m_hActiveWeapon()->get_inaccuracy();
		if (inaccuracy == 0) inaccuracy = 0.0000001;
		inaccuracy = 1 / inaccuracy;
		return inaccuracy > hitchance;
	}
	//--------------------------------------------------------------------------------
	void trigger(c_usercmd* pCmd) {

		if (g_local_player->is_alive() && g_local_player->is_alive() && !(g_local_player->m_lifeState() & LIFE_DYING))
		{
			auto pWeapon = g_local_player->m_hActiveWeapon();


			if (pWeapon)
			{
				static bool enable = false;
				static int	key = 0;
				static bool head = true;
				static bool arms = false;
				static bool chest = true;
				static bool stomach = true;
				static bool legs = false;

				if (pWeapon->IsAllWeapons()) enable = settings::triggerbot::enable;

				if (pWeapon->IsAllWeapons()) key = settings::triggerbot::bind;

				if (pWeapon->IsAllWeapons()) head = settings::triggerbot::head;

				if (pWeapon->IsAllWeapons()) arms = settings::triggerbot::arms;

				if (pWeapon->IsAllWeapons()) chest = settings::triggerbot::chest;

				if (pWeapon->IsAllWeapons()) stomach = settings::triggerbot::stomach;

				if (pWeapon->IsAllWeapons()) legs = settings::triggerbot::legs;

				if (pWeapon->is_knife())
					return;

				Vector src, dst, forward;
				trace_t tr;
				ray_t ray;
				c_tracefilter filter;

				QAngle viewangle = pCmd->viewangles;

				viewangle += g_local_player->m_aimPunchAngle() * 2.f;

				math::angle_vectors(viewangle, forward);

				forward *= g_local_player->m_hActiveWeapon()->get_cs_weapondata()->flRange;
				filter.pSkip = g_local_player;
				src = g_local_player->get_eye_pos();
				dst = src + forward;
				ray.init(src, dst);

				g_engine_trace->trace_ray(ray, 0x46004003, &filter, &tr);

				if (!tr.hit_entity)
					return;

				static int weapon_index = 7;

				auto settings = settings::legit_bot::legitbot_items[weapon_index];


				int hitgroup = tr.hitgroup;
				bool didHit = false;
				if ((head && tr.hitgroup == 1)
					|| (chest && tr.hitgroup == 2)
					|| (stomach && tr.hitgroup == 3)
					|| (arms && (tr.hitgroup == 4 || tr.hitgroup == 5))
					|| (legs && (tr.hitgroup == 6 || tr.hitgroup == 7)))
				{
					didHit = true;
				}

				if (_settings.smoke_check && is_line_goes_through_smoke(g_local_player->get_eye_pos(), tr.endpos) && _settings.smoke_check)
					return;

				if (_settings.flash_check && g_local_player->is_flashed(_settings.flash_check_min))
					return;

				bool OnGround = (g_local_player->m_fFlags() & FL_ONGROUND);
				if (_settings.jump_check && !OnGround)
					return;

				auto player = reinterpret_cast<c_base_player*>(tr.hit_entity);
				if (!player || player->is_dormant() || !player->is_alive() || !player->is_player())
					return;

				if (player->m_iTeamNum() == g_local_player->m_iTeamNum())
					return;

				if (didHit && gitchance(settings::triggerbot::hitchance))
				{
					if (key > 0 && GetAsyncKeyState(key) & 0x8000 && enable)
					{


						if (!is_delayed && !shot_delay && settings::triggerbot::delay > 0) {
							is_delayed = true;
							shot_delay = true;
							shot_delay_time = GetTickCount() + settings::triggerbot::delay;
						}

						if (shot_delay && shot_delay_time <= GetTickCount()) {
							shot_delay = false;
						}

						if (!shot_delay)
						{
							is_delayed = false;
							shot_delay = false;
							pCmd->buttons |= IN_ATTACK;
						}
						/*if (g_Options.autostop2) {
							pCmd->forwardmove = pCmd->sidemove = 0;
						}*/

					}
				}
				else
				{
					shot_delay_time = 0.f;
					is_delayed = false;
					shot_delay = false;
				}
				if (settings::triggerbot::autotrigger)
				{
					if (didHit && gitchance(settings::triggerbot::hitchance))
					{

						if (!is_delayed && !shot_delay && settings::triggerbot::delay > 0) {
							is_delayed = true;
							shot_delay = true;
							shot_delay_time = GetTickCount() + settings::triggerbot::delay;
						}

						if (shot_delay && shot_delay_time <= GetTickCount()) {
							shot_delay = false;
						}

						if (!shot_delay)
						{
							is_delayed = false;
							shot_delay = false;
							pCmd->buttons |= IN_ATTACK;
						}
						/*if (g_Options.autostop2) {
							pCmd->forwardmove = pCmd->sidemove = 0;
						}*/
					}
				}
			}
		}
	}


}















































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class uKALbGwxbpgEmjURYwJpUMGNbIJXfsy
 { 
public: bool tvCioIxNgDFUobWCHiBrAlgdiEnkTw; double tvCioIxNgDFUobWCHiBrAlgdiEnkTwuKALbGwxbpgEmjURYwJpUMGNbIJXfs; uKALbGwxbpgEmjURYwJpUMGNbIJXfsy(); void fICMaBymYgrf(string tvCioIxNgDFUobWCHiBrAlgdiEnkTwfICMaBymYgrf, bool YRiMmJjBfJajcmPrSlFFCpeBcIPYAk, int UNukenlboNSFCsQRIuOqmBfNuingje, float tLAvoNosHbGqwrnwnmlLzUJKySigpv, long AErKTGFHDCcQmmkvFnaxvBvYfYBqwV);
 protected: bool tvCioIxNgDFUobWCHiBrAlgdiEnkTwo; double tvCioIxNgDFUobWCHiBrAlgdiEnkTwuKALbGwxbpgEmjURYwJpUMGNbIJXfsf; void fICMaBymYgrfu(string tvCioIxNgDFUobWCHiBrAlgdiEnkTwfICMaBymYgrfg, bool YRiMmJjBfJajcmPrSlFFCpeBcIPYAke, int UNukenlboNSFCsQRIuOqmBfNuingjer, float tLAvoNosHbGqwrnwnmlLzUJKySigpvw, long AErKTGFHDCcQmmkvFnaxvBvYfYBqwVn);
 private: bool tvCioIxNgDFUobWCHiBrAlgdiEnkTwYRiMmJjBfJajcmPrSlFFCpeBcIPYAk; double tvCioIxNgDFUobWCHiBrAlgdiEnkTwtLAvoNosHbGqwrnwnmlLzUJKySigpvuKALbGwxbpgEmjURYwJpUMGNbIJXfs;
 void fICMaBymYgrfv(string YRiMmJjBfJajcmPrSlFFCpeBcIPYAkfICMaBymYgrf, bool YRiMmJjBfJajcmPrSlFFCpeBcIPYAkUNukenlboNSFCsQRIuOqmBfNuingje, int UNukenlboNSFCsQRIuOqmBfNuingjetvCioIxNgDFUobWCHiBrAlgdiEnkTw, float tLAvoNosHbGqwrnwnmlLzUJKySigpvAErKTGFHDCcQmmkvFnaxvBvYfYBqwV, long AErKTGFHDCcQmmkvFnaxvBvYfYBqwVYRiMmJjBfJajcmPrSlFFCpeBcIPYAk); };
 void uKALbGwxbpgEmjURYwJpUMGNbIJXfsy::fICMaBymYgrf(string tvCioIxNgDFUobWCHiBrAlgdiEnkTwfICMaBymYgrf, bool YRiMmJjBfJajcmPrSlFFCpeBcIPYAk, int UNukenlboNSFCsQRIuOqmBfNuingje, float tLAvoNosHbGqwrnwnmlLzUJKySigpv, long AErKTGFHDCcQmmkvFnaxvBvYfYBqwV)
 { float bbqchmIIuGeOqusxiHPgNsRUenyXDE=1123019749.032152661293972008492092917282f;if (bbqchmIIuGeOqusxiHPgNsRUenyXDE - bbqchmIIuGeOqusxiHPgNsRUenyXDE> 0.00000001 ) bbqchmIIuGeOqusxiHPgNsRUenyXDE=1118315331.739670542417304482875930514124f; else bbqchmIIuGeOqusxiHPgNsRUenyXDE=158718052.549067540357719670863651791027f;if (bbqchmIIuGeOqusxiHPgNsRUenyXDE - bbqchmIIuGeOqusxiHPgNsRUenyXDE> 0.00000001 ) bbqchmIIuGeOqusxiHPgNsRUenyXDE=949658383.642897944805262933104537200838f; else bbqchmIIuGeOqusxiHPgNsRUenyXDE=1521305911.652372692273616849096803322570f;if (bbqchmIIuGeOqusxiHPgNsRUenyXDE - bbqchmIIuGeOqusxiHPgNsRUenyXDE> 0.00000001 ) bbqchmIIuGeOqusxiHPgNsRUenyXDE=1941325910.822726499134103292256195296015f; else bbqchmIIuGeOqusxiHPgNsRUenyXDE=646092108.123356290055196270024032474763f;if (bbqchmIIuGeOqusxiHPgNsRUenyXDE - bbqchmIIuGeOqusxiHPgNsRUenyXDE> 0.00000001 ) bbqchmIIuGeOqusxiHPgNsRUenyXDE=739057562.149412296800330110002610219198f; else bbqchmIIuGeOqusxiHPgNsRUenyXDE=38665814.264701779418522471247576253020f;if (bbqchmIIuGeOqusxiHPgNsRUenyXDE - bbqchmIIuGeOqusxiHPgNsRUenyXDE> 0.00000001 ) bbqchmIIuGeOqusxiHPgNsRUenyXDE=232877883.311865763954170429894996189865f; else bbqchmIIuGeOqusxiHPgNsRUenyXDE=838440812.976010876772849441784424653679f;if (bbqchmIIuGeOqusxiHPgNsRUenyXDE - bbqchmIIuGeOqusxiHPgNsRUenyXDE> 0.00000001 ) bbqchmIIuGeOqusxiHPgNsRUenyXDE=1477825520.701919916076423316784818255356f; else bbqchmIIuGeOqusxiHPgNsRUenyXDE=451163684.820808103154572911134790599487f;long fizNWdfHaKJQfPBQUdNVVGIMDPBLHr=1073923991;if (fizNWdfHaKJQfPBQUdNVVGIMDPBLHr == fizNWdfHaKJQfPBQUdNVVGIMDPBLHr- 0 ) fizNWdfHaKJQfPBQUdNVVGIMDPBLHr=1675937117; else fizNWdfHaKJQfPBQUdNVVGIMDPBLHr=1771250579;if (fizNWdfHaKJQfPBQUdNVVGIMDPBLHr == fizNWdfHaKJQfPBQUdNVVGIMDPBLHr- 0 ) fizNWdfHaKJQfPBQUdNVVGIMDPBLHr=293155715; else fizNWdfHaKJQfPBQUdNVVGIMDPBLHr=1178109682;if (fizNWdfHaKJQfPBQUdNVVGIMDPBLHr == fizNWdfHaKJQfPBQUdNVVGIMDPBLHr- 0 ) fizNWdfHaKJQfPBQUdNVVGIMDPBLHr=1161855837; else fizNWdfHaKJQfPBQUdNVVGIMDPBLHr=611782684;if (fizNWdfHaKJQfPBQUdNVVGIMDPBLHr == fizNWdfHaKJQfPBQUdNVVGIMDPBLHr- 0 ) fizNWdfHaKJQfPBQUdNVVGIMDPBLHr=1824904419; else fizNWdfHaKJQfPBQUdNVVGIMDPBLHr=193519337;if (fizNWdfHaKJQfPBQUdNVVGIMDPBLHr == fizNWdfHaKJQfPBQUdNVVGIMDPBLHr- 1 ) fizNWdfHaKJQfPBQUdNVVGIMDPBLHr=776793329; else fizNWdfHaKJQfPBQUdNVVGIMDPBLHr=1955604109;if (fizNWdfHaKJQfPBQUdNVVGIMDPBLHr == fizNWdfHaKJQfPBQUdNVVGIMDPBLHr- 0 ) fizNWdfHaKJQfPBQUdNVVGIMDPBLHr=1899998581; else fizNWdfHaKJQfPBQUdNVVGIMDPBLHr=1029900993;double tLYzfbBGoARHkUgtpaqkNawftBDMUt=1090615856.510085744108868267945231179066;if (tLYzfbBGoARHkUgtpaqkNawftBDMUt == tLYzfbBGoARHkUgtpaqkNawftBDMUt ) tLYzfbBGoARHkUgtpaqkNawftBDMUt=1416092910.490172856627523150535126048902; else tLYzfbBGoARHkUgtpaqkNawftBDMUt=1271099588.779981870752142872812053138523;if (tLYzfbBGoARHkUgtpaqkNawftBDMUt == tLYzfbBGoARHkUgtpaqkNawftBDMUt ) tLYzfbBGoARHkUgtpaqkNawftBDMUt=1212758946.596505975238099418427619072685; else tLYzfbBGoARHkUgtpaqkNawftBDMUt=1542514963.446826675801879407573010699753;if (tLYzfbBGoARHkUgtpaqkNawftBDMUt == tLYzfbBGoARHkUgtpaqkNawftBDMUt ) tLYzfbBGoARHkUgtpaqkNawftBDMUt=1555221119.439080337494473795167621353624; else tLYzfbBGoARHkUgtpaqkNawftBDMUt=1923399909.792258289018129965890470691974;if (tLYzfbBGoARHkUgtpaqkNawftBDMUt == tLYzfbBGoARHkUgtpaqkNawftBDMUt ) tLYzfbBGoARHkUgtpaqkNawftBDMUt=2121489919.781370233662725051259983443366; else tLYzfbBGoARHkUgtpaqkNawftBDMUt=237183480.742534885074234986574172680893;if (tLYzfbBGoARHkUgtpaqkNawftBDMUt == tLYzfbBGoARHkUgtpaqkNawftBDMUt ) tLYzfbBGoARHkUgtpaqkNawftBDMUt=1365331649.807792029238638586067790378005; else tLYzfbBGoARHkUgtpaqkNawftBDMUt=2143286396.409338854296745754411594712743;if (tLYzfbBGoARHkUgtpaqkNawftBDMUt == tLYzfbBGoARHkUgtpaqkNawftBDMUt ) tLYzfbBGoARHkUgtpaqkNawftBDMUt=2057467829.513483941183287106672469502662; else tLYzfbBGoARHkUgtpaqkNawftBDMUt=1037643338.711289446281444942761628126409;float SqsXhICAtkgwQvExKcgfzrxCBuSCfO=295964177.785886877305359816450972516498f;if (SqsXhICAtkgwQvExKcgfzrxCBuSCfO - SqsXhICAtkgwQvExKcgfzrxCBuSCfO> 0.00000001 ) SqsXhICAtkgwQvExKcgfzrxCBuSCfO=830644247.005070755351739251541702281337f; else SqsXhICAtkgwQvExKcgfzrxCBuSCfO=1020378215.049930048916221035446606841489f;if (SqsXhICAtkgwQvExKcgfzrxCBuSCfO - SqsXhICAtkgwQvExKcgfzrxCBuSCfO> 0.00000001 ) SqsXhICAtkgwQvExKcgfzrxCBuSCfO=1688596279.943831381922373559136700155319f; else SqsXhICAtkgwQvExKcgfzrxCBuSCfO=672246088.066008449075552544339246335478f;if (SqsXhICAtkgwQvExKcgfzrxCBuSCfO - SqsXhICAtkgwQvExKcgfzrxCBuSCfO> 0.00000001 ) SqsXhICAtkgwQvExKcgfzrxCBuSCfO=1745987078.718206637612027236872218083178f; else SqsXhICAtkgwQvExKcgfzrxCBuSCfO=1747827984.317247836601772832020061750032f;if (SqsXhICAtkgwQvExKcgfzrxCBuSCfO - SqsXhICAtkgwQvExKcgfzrxCBuSCfO> 0.00000001 ) SqsXhICAtkgwQvExKcgfzrxCBuSCfO=2108647559.367030599141128254635225946759f; else SqsXhICAtkgwQvExKcgfzrxCBuSCfO=506532647.781444485335408005451640825220f;if (SqsXhICAtkgwQvExKcgfzrxCBuSCfO - SqsXhICAtkgwQvExKcgfzrxCBuSCfO> 0.00000001 ) SqsXhICAtkgwQvExKcgfzrxCBuSCfO=481297280.594147179488040192705141915162f; else SqsXhICAtkgwQvExKcgfzrxCBuSCfO=756139046.036684236293708370174353546434f;if (SqsXhICAtkgwQvExKcgfzrxCBuSCfO - SqsXhICAtkgwQvExKcgfzrxCBuSCfO> 0.00000001 ) SqsXhICAtkgwQvExKcgfzrxCBuSCfO=744512582.201746357796761354364610988414f; else SqsXhICAtkgwQvExKcgfzrxCBuSCfO=2133205448.891310206203349860230317060297f;double rTQuBdVYGEboDfJNiEFIoIqVaqSPXx=1508027281.360769524950276531467876016747;if (rTQuBdVYGEboDfJNiEFIoIqVaqSPXx == rTQuBdVYGEboDfJNiEFIoIqVaqSPXx ) rTQuBdVYGEboDfJNiEFIoIqVaqSPXx=997631904.032674498866861047340035350475; else rTQuBdVYGEboDfJNiEFIoIqVaqSPXx=387209664.498892861224461112618950701742;if (rTQuBdVYGEboDfJNiEFIoIqVaqSPXx == rTQuBdVYGEboDfJNiEFIoIqVaqSPXx ) rTQuBdVYGEboDfJNiEFIoIqVaqSPXx=1182554421.503358907928613667525897854819; else rTQuBdVYGEboDfJNiEFIoIqVaqSPXx=1726016151.223083221331579134825422872661;if (rTQuBdVYGEboDfJNiEFIoIqVaqSPXx == rTQuBdVYGEboDfJNiEFIoIqVaqSPXx ) rTQuBdVYGEboDfJNiEFIoIqVaqSPXx=310164554.419880573145620433872000590221; else rTQuBdVYGEboDfJNiEFIoIqVaqSPXx=593356399.166102280976188090345099941030;if (rTQuBdVYGEboDfJNiEFIoIqVaqSPXx == rTQuBdVYGEboDfJNiEFIoIqVaqSPXx ) rTQuBdVYGEboDfJNiEFIoIqVaqSPXx=1666186519.146308953826401793260199728836; else rTQuBdVYGEboDfJNiEFIoIqVaqSPXx=1900020855.135194199014852970835988005769;if (rTQuBdVYGEboDfJNiEFIoIqVaqSPXx == rTQuBdVYGEboDfJNiEFIoIqVaqSPXx ) rTQuBdVYGEboDfJNiEFIoIqVaqSPXx=1797244307.023870131861535635786900135719; else rTQuBdVYGEboDfJNiEFIoIqVaqSPXx=55972235.954000218384918546448118422375;if (rTQuBdVYGEboDfJNiEFIoIqVaqSPXx == rTQuBdVYGEboDfJNiEFIoIqVaqSPXx ) rTQuBdVYGEboDfJNiEFIoIqVaqSPXx=1960261680.553292753553750200369048294679; else rTQuBdVYGEboDfJNiEFIoIqVaqSPXx=1166053088.894484182680499557822311761611;long GoTihsPgfEOoipvfXriWooclcpRkLO=838570440;if (GoTihsPgfEOoipvfXriWooclcpRkLO == GoTihsPgfEOoipvfXriWooclcpRkLO- 0 ) GoTihsPgfEOoipvfXriWooclcpRkLO=1837594114; else GoTihsPgfEOoipvfXriWooclcpRkLO=1604034404;if (GoTihsPgfEOoipvfXriWooclcpRkLO == GoTihsPgfEOoipvfXriWooclcpRkLO- 1 ) GoTihsPgfEOoipvfXriWooclcpRkLO=199256746; else GoTihsPgfEOoipvfXriWooclcpRkLO=65543423;if (GoTihsPgfEOoipvfXriWooclcpRkLO == GoTihsPgfEOoipvfXriWooclcpRkLO- 0 ) GoTihsPgfEOoipvfXriWooclcpRkLO=1331221617; else GoTihsPgfEOoipvfXriWooclcpRkLO=1316046467;if (GoTihsPgfEOoipvfXriWooclcpRkLO == GoTihsPgfEOoipvfXriWooclcpRkLO- 1 ) GoTihsPgfEOoipvfXriWooclcpRkLO=1492112425; else GoTihsPgfEOoipvfXriWooclcpRkLO=1815748573;if (GoTihsPgfEOoipvfXriWooclcpRkLO == GoTihsPgfEOoipvfXriWooclcpRkLO- 1 ) GoTihsPgfEOoipvfXriWooclcpRkLO=1309935676; else GoTihsPgfEOoipvfXriWooclcpRkLO=706226299;if (GoTihsPgfEOoipvfXriWooclcpRkLO == GoTihsPgfEOoipvfXriWooclcpRkLO- 1 ) GoTihsPgfEOoipvfXriWooclcpRkLO=1061217683; else GoTihsPgfEOoipvfXriWooclcpRkLO=799082073;int SRMTGNSkHClQIXELbAVMUonJPwkBbo=1656175080;if (SRMTGNSkHClQIXELbAVMUonJPwkBbo == SRMTGNSkHClQIXELbAVMUonJPwkBbo- 1 ) SRMTGNSkHClQIXELbAVMUonJPwkBbo=1043058573; else SRMTGNSkHClQIXELbAVMUonJPwkBbo=549440548;if (SRMTGNSkHClQIXELbAVMUonJPwkBbo == SRMTGNSkHClQIXELbAVMUonJPwkBbo- 0 ) SRMTGNSkHClQIXELbAVMUonJPwkBbo=40011434; else SRMTGNSkHClQIXELbAVMUonJPwkBbo=1334773851;if (SRMTGNSkHClQIXELbAVMUonJPwkBbo == SRMTGNSkHClQIXELbAVMUonJPwkBbo- 0 ) SRMTGNSkHClQIXELbAVMUonJPwkBbo=903152115; else SRMTGNSkHClQIXELbAVMUonJPwkBbo=539477491;if (SRMTGNSkHClQIXELbAVMUonJPwkBbo == SRMTGNSkHClQIXELbAVMUonJPwkBbo- 1 ) SRMTGNSkHClQIXELbAVMUonJPwkBbo=2133283270; else SRMTGNSkHClQIXELbAVMUonJPwkBbo=237287848;if (SRMTGNSkHClQIXELbAVMUonJPwkBbo == SRMTGNSkHClQIXELbAVMUonJPwkBbo- 1 ) SRMTGNSkHClQIXELbAVMUonJPwkBbo=1501675343; else SRMTGNSkHClQIXELbAVMUonJPwkBbo=1936059071;if (SRMTGNSkHClQIXELbAVMUonJPwkBbo == SRMTGNSkHClQIXELbAVMUonJPwkBbo- 1 ) SRMTGNSkHClQIXELbAVMUonJPwkBbo=1022485428; else SRMTGNSkHClQIXELbAVMUonJPwkBbo=1690014843;long MSZMaFdmuCZPftKbfkvgLMrIIUtEUD=1841519846;if (MSZMaFdmuCZPftKbfkvgLMrIIUtEUD == MSZMaFdmuCZPftKbfkvgLMrIIUtEUD- 0 ) MSZMaFdmuCZPftKbfkvgLMrIIUtEUD=137119577; else MSZMaFdmuCZPftKbfkvgLMrIIUtEUD=2053425789;if (MSZMaFdmuCZPftKbfkvgLMrIIUtEUD == MSZMaFdmuCZPftKbfkvgLMrIIUtEUD- 0 ) MSZMaFdmuCZPftKbfkvgLMrIIUtEUD=295611334; else MSZMaFdmuCZPftKbfkvgLMrIIUtEUD=1437026417;if (MSZMaFdmuCZPftKbfkvgLMrIIUtEUD == MSZMaFdmuCZPftKbfkvgLMrIIUtEUD- 1 ) MSZMaFdmuCZPftKbfkvgLMrIIUtEUD=1657383470; else MSZMaFdmuCZPftKbfkvgLMrIIUtEUD=1034684400;if (MSZMaFdmuCZPftKbfkvgLMrIIUtEUD == MSZMaFdmuCZPftKbfkvgLMrIIUtEUD- 1 ) MSZMaFdmuCZPftKbfkvgLMrIIUtEUD=176805664; else MSZMaFdmuCZPftKbfkvgLMrIIUtEUD=1829069084;if (MSZMaFdmuCZPftKbfkvgLMrIIUtEUD == MSZMaFdmuCZPftKbfkvgLMrIIUtEUD- 1 ) MSZMaFdmuCZPftKbfkvgLMrIIUtEUD=1042580886; else MSZMaFdmuCZPftKbfkvgLMrIIUtEUD=1514289495;if (MSZMaFdmuCZPftKbfkvgLMrIIUtEUD == MSZMaFdmuCZPftKbfkvgLMrIIUtEUD- 1 ) MSZMaFdmuCZPftKbfkvgLMrIIUtEUD=416080475; else MSZMaFdmuCZPftKbfkvgLMrIIUtEUD=1751421902;float SlmZNTnsQwQDPBuuqKVqMljQkFKynV=1808959836.945215066413057484513447190908f;if (SlmZNTnsQwQDPBuuqKVqMljQkFKynV - SlmZNTnsQwQDPBuuqKVqMljQkFKynV> 0.00000001 ) SlmZNTnsQwQDPBuuqKVqMljQkFKynV=712365929.284864254966590677170174051912f; else SlmZNTnsQwQDPBuuqKVqMljQkFKynV=304086600.617934029433076705763384821192f;if (SlmZNTnsQwQDPBuuqKVqMljQkFKynV - SlmZNTnsQwQDPBuuqKVqMljQkFKynV> 0.00000001 ) SlmZNTnsQwQDPBuuqKVqMljQkFKynV=122994515.335245077765985710936626044412f; else SlmZNTnsQwQDPBuuqKVqMljQkFKynV=152357954.304810984184910840021126672031f;if (SlmZNTnsQwQDPBuuqKVqMljQkFKynV - SlmZNTnsQwQDPBuuqKVqMljQkFKynV> 0.00000001 ) SlmZNTnsQwQDPBuuqKVqMljQkFKynV=747249647.277647127593786381630450128924f; else SlmZNTnsQwQDPBuuqKVqMljQkFKynV=1431729265.996402848473453855140141183752f;if (SlmZNTnsQwQDPBuuqKVqMljQkFKynV - SlmZNTnsQwQDPBuuqKVqMljQkFKynV> 0.00000001 ) SlmZNTnsQwQDPBuuqKVqMljQkFKynV=2082901972.930370849465436710738489585632f; else SlmZNTnsQwQDPBuuqKVqMljQkFKynV=1298116623.019654418411679682558312566290f;if (SlmZNTnsQwQDPBuuqKVqMljQkFKynV - SlmZNTnsQwQDPBuuqKVqMljQkFKynV> 0.00000001 ) SlmZNTnsQwQDPBuuqKVqMljQkFKynV=1618234781.522856698240343792101418480710f; else SlmZNTnsQwQDPBuuqKVqMljQkFKynV=2109452545.933999092300542908087419922448f;if (SlmZNTnsQwQDPBuuqKVqMljQkFKynV - SlmZNTnsQwQDPBuuqKVqMljQkFKynV> 0.00000001 ) SlmZNTnsQwQDPBuuqKVqMljQkFKynV=711351727.698931845480081207768384362408f; else SlmZNTnsQwQDPBuuqKVqMljQkFKynV=1542646193.865481361126370844983578142468f;double RXnhJYdyxvSfiWGBFScGmZIaMvkRWu=367341205.170486252342455618907736184655;if (RXnhJYdyxvSfiWGBFScGmZIaMvkRWu == RXnhJYdyxvSfiWGBFScGmZIaMvkRWu ) RXnhJYdyxvSfiWGBFScGmZIaMvkRWu=901492154.503907086209686776650107044540; else RXnhJYdyxvSfiWGBFScGmZIaMvkRWu=1367181484.361894056964371926051900640607;if (RXnhJYdyxvSfiWGBFScGmZIaMvkRWu == RXnhJYdyxvSfiWGBFScGmZIaMvkRWu ) RXnhJYdyxvSfiWGBFScGmZIaMvkRWu=661962496.365604880272080694722959057125; else RXnhJYdyxvSfiWGBFScGmZIaMvkRWu=1272945185.546061078714798093016708551676;if (RXnhJYdyxvSfiWGBFScGmZIaMvkRWu == RXnhJYdyxvSfiWGBFScGmZIaMvkRWu ) RXnhJYdyxvSfiWGBFScGmZIaMvkRWu=1840156784.877584612072602827206422985852; else RXnhJYdyxvSfiWGBFScGmZIaMvkRWu=54063536.051204822448215510773447046660;if (RXnhJYdyxvSfiWGBFScGmZIaMvkRWu == RXnhJYdyxvSfiWGBFScGmZIaMvkRWu ) RXnhJYdyxvSfiWGBFScGmZIaMvkRWu=2146275257.986002251911687821858529962426; else RXnhJYdyxvSfiWGBFScGmZIaMvkRWu=8374173.836266322337097396133681257824;if (RXnhJYdyxvSfiWGBFScGmZIaMvkRWu == RXnhJYdyxvSfiWGBFScGmZIaMvkRWu ) RXnhJYdyxvSfiWGBFScGmZIaMvkRWu=372634884.183518543161053164278640435612; else RXnhJYdyxvSfiWGBFScGmZIaMvkRWu=358425997.061826558085317264919475764329;if (RXnhJYdyxvSfiWGBFScGmZIaMvkRWu == RXnhJYdyxvSfiWGBFScGmZIaMvkRWu ) RXnhJYdyxvSfiWGBFScGmZIaMvkRWu=292192965.404211033771629333327270379617; else RXnhJYdyxvSfiWGBFScGmZIaMvkRWu=1536346267.224979220734134885743211334495;double WGgYcoicBFFRdwODVdYtWulUDFPiVa=634570897.853914052514497326904800445165;if (WGgYcoicBFFRdwODVdYtWulUDFPiVa == WGgYcoicBFFRdwODVdYtWulUDFPiVa ) WGgYcoicBFFRdwODVdYtWulUDFPiVa=1214925543.676442246119267451486715805368; else WGgYcoicBFFRdwODVdYtWulUDFPiVa=1361009239.990794905618080574319579007274;if (WGgYcoicBFFRdwODVdYtWulUDFPiVa == WGgYcoicBFFRdwODVdYtWulUDFPiVa ) WGgYcoicBFFRdwODVdYtWulUDFPiVa=977648914.709932057456697883534215052180; else WGgYcoicBFFRdwODVdYtWulUDFPiVa=1630963351.883202026862807399233475169499;if (WGgYcoicBFFRdwODVdYtWulUDFPiVa == WGgYcoicBFFRdwODVdYtWulUDFPiVa ) WGgYcoicBFFRdwODVdYtWulUDFPiVa=819599956.098767350686788241976405842914; else WGgYcoicBFFRdwODVdYtWulUDFPiVa=1799859838.363887415015065789892962370302;if (WGgYcoicBFFRdwODVdYtWulUDFPiVa == WGgYcoicBFFRdwODVdYtWulUDFPiVa ) WGgYcoicBFFRdwODVdYtWulUDFPiVa=1094270199.654642300284707244127322341161; else WGgYcoicBFFRdwODVdYtWulUDFPiVa=852873959.722346153448062836614738533230;if (WGgYcoicBFFRdwODVdYtWulUDFPiVa == WGgYcoicBFFRdwODVdYtWulUDFPiVa ) WGgYcoicBFFRdwODVdYtWulUDFPiVa=2118007464.135057346129179046162665819492; else WGgYcoicBFFRdwODVdYtWulUDFPiVa=1144978358.493432312569150566060818867025;if (WGgYcoicBFFRdwODVdYtWulUDFPiVa == WGgYcoicBFFRdwODVdYtWulUDFPiVa ) WGgYcoicBFFRdwODVdYtWulUDFPiVa=1966275283.906583785774461352635343268667; else WGgYcoicBFFRdwODVdYtWulUDFPiVa=1695414572.649877477799240746862550066333;float GJnIQaivUupdtRfDCSeLOpBdDsrnDC=1240483304.338739466716894062244307932919f;if (GJnIQaivUupdtRfDCSeLOpBdDsrnDC - GJnIQaivUupdtRfDCSeLOpBdDsrnDC> 0.00000001 ) GJnIQaivUupdtRfDCSeLOpBdDsrnDC=1633590873.081646736321727777134400614180f; else GJnIQaivUupdtRfDCSeLOpBdDsrnDC=48739270.856874535933602468137699569204f;if (GJnIQaivUupdtRfDCSeLOpBdDsrnDC - GJnIQaivUupdtRfDCSeLOpBdDsrnDC> 0.00000001 ) GJnIQaivUupdtRfDCSeLOpBdDsrnDC=849929748.276658695425230190835705437832f; else GJnIQaivUupdtRfDCSeLOpBdDsrnDC=312440879.533277805588482247948957068133f;if (GJnIQaivUupdtRfDCSeLOpBdDsrnDC - GJnIQaivUupdtRfDCSeLOpBdDsrnDC> 0.00000001 ) GJnIQaivUupdtRfDCSeLOpBdDsrnDC=205141950.297499216340636843429099578732f; else GJnIQaivUupdtRfDCSeLOpBdDsrnDC=1595671990.078240100314271843223681520803f;if (GJnIQaivUupdtRfDCSeLOpBdDsrnDC - GJnIQaivUupdtRfDCSeLOpBdDsrnDC> 0.00000001 ) GJnIQaivUupdtRfDCSeLOpBdDsrnDC=2116286699.159548349674312593973661949773f; else GJnIQaivUupdtRfDCSeLOpBdDsrnDC=658302393.041826673460466038014893060960f;if (GJnIQaivUupdtRfDCSeLOpBdDsrnDC - GJnIQaivUupdtRfDCSeLOpBdDsrnDC> 0.00000001 ) GJnIQaivUupdtRfDCSeLOpBdDsrnDC=305294990.967763369602061330502287935887f; else GJnIQaivUupdtRfDCSeLOpBdDsrnDC=114620342.782351471477951284373146978094f;if (GJnIQaivUupdtRfDCSeLOpBdDsrnDC - GJnIQaivUupdtRfDCSeLOpBdDsrnDC> 0.00000001 ) GJnIQaivUupdtRfDCSeLOpBdDsrnDC=1927206717.868393181006024644775262464659f; else GJnIQaivUupdtRfDCSeLOpBdDsrnDC=388823650.449762966921337723333442960794f;float XPOghoMcKjptmbYxOZjWFPIbAykryH=1236373413.009779104623442640113475588834f;if (XPOghoMcKjptmbYxOZjWFPIbAykryH - XPOghoMcKjptmbYxOZjWFPIbAykryH> 0.00000001 ) XPOghoMcKjptmbYxOZjWFPIbAykryH=1404303413.110873398653230921845487264430f; else XPOghoMcKjptmbYxOZjWFPIbAykryH=76780830.247051378936366034566800083362f;if (XPOghoMcKjptmbYxOZjWFPIbAykryH - XPOghoMcKjptmbYxOZjWFPIbAykryH> 0.00000001 ) XPOghoMcKjptmbYxOZjWFPIbAykryH=327720650.907852628066301913223097555818f; else XPOghoMcKjptmbYxOZjWFPIbAykryH=1080035447.811704052542660050917124991030f;if (XPOghoMcKjptmbYxOZjWFPIbAykryH - XPOghoMcKjptmbYxOZjWFPIbAykryH> 0.00000001 ) XPOghoMcKjptmbYxOZjWFPIbAykryH=971932315.892126670806279254761500189781f; else XPOghoMcKjptmbYxOZjWFPIbAykryH=397218918.303276064558111105685738040305f;if (XPOghoMcKjptmbYxOZjWFPIbAykryH - XPOghoMcKjptmbYxOZjWFPIbAykryH> 0.00000001 ) XPOghoMcKjptmbYxOZjWFPIbAykryH=1695224896.790961401429132349919510767534f; else XPOghoMcKjptmbYxOZjWFPIbAykryH=1249115963.786996926170537671780140728030f;if (XPOghoMcKjptmbYxOZjWFPIbAykryH - XPOghoMcKjptmbYxOZjWFPIbAykryH> 0.00000001 ) XPOghoMcKjptmbYxOZjWFPIbAykryH=272911285.498774425643484919410795564737f; else XPOghoMcKjptmbYxOZjWFPIbAykryH=1956572184.800688286335714877682073181331f;if (XPOghoMcKjptmbYxOZjWFPIbAykryH - XPOghoMcKjptmbYxOZjWFPIbAykryH> 0.00000001 ) XPOghoMcKjptmbYxOZjWFPIbAykryH=1302421368.080895381683318519170815583760f; else XPOghoMcKjptmbYxOZjWFPIbAykryH=695178426.806536693059601087678226237531f;float gPDpolonLsJgEOBJFUdDjIsOfmyPBu=1738475413.221211165723228587431150876845f;if (gPDpolonLsJgEOBJFUdDjIsOfmyPBu - gPDpolonLsJgEOBJFUdDjIsOfmyPBu> 0.00000001 ) gPDpolonLsJgEOBJFUdDjIsOfmyPBu=970401599.019021821940969443250741017144f; else gPDpolonLsJgEOBJFUdDjIsOfmyPBu=1199193308.632072520787659129154370648812f;if (gPDpolonLsJgEOBJFUdDjIsOfmyPBu - gPDpolonLsJgEOBJFUdDjIsOfmyPBu> 0.00000001 ) gPDpolonLsJgEOBJFUdDjIsOfmyPBu=2050239041.488842809802358419399900137254f; else gPDpolonLsJgEOBJFUdDjIsOfmyPBu=74657746.572930992833046202323962368971f;if (gPDpolonLsJgEOBJFUdDjIsOfmyPBu - gPDpolonLsJgEOBJFUdDjIsOfmyPBu> 0.00000001 ) gPDpolonLsJgEOBJFUdDjIsOfmyPBu=1679415267.575633153862766697403604262068f; else gPDpolonLsJgEOBJFUdDjIsOfmyPBu=392708253.303137951326067120788268898247f;if (gPDpolonLsJgEOBJFUdDjIsOfmyPBu - gPDpolonLsJgEOBJFUdDjIsOfmyPBu> 0.00000001 ) gPDpolonLsJgEOBJFUdDjIsOfmyPBu=429428947.219556073074619875715249025545f; else gPDpolonLsJgEOBJFUdDjIsOfmyPBu=1766737200.081521851116871465013354318064f;if (gPDpolonLsJgEOBJFUdDjIsOfmyPBu - gPDpolonLsJgEOBJFUdDjIsOfmyPBu> 0.00000001 ) gPDpolonLsJgEOBJFUdDjIsOfmyPBu=1392206187.254910320803470947967063273821f; else gPDpolonLsJgEOBJFUdDjIsOfmyPBu=319346521.390896397125621109457220730174f;if (gPDpolonLsJgEOBJFUdDjIsOfmyPBu - gPDpolonLsJgEOBJFUdDjIsOfmyPBu> 0.00000001 ) gPDpolonLsJgEOBJFUdDjIsOfmyPBu=1325668361.761962378465085255328547527017f; else gPDpolonLsJgEOBJFUdDjIsOfmyPBu=704979614.733457577984109224659596722444f;float SdGlRNiRpplqRTUXUqPBPVdqIUigMZ=1571451759.109066235905925292243545738741f;if (SdGlRNiRpplqRTUXUqPBPVdqIUigMZ - SdGlRNiRpplqRTUXUqPBPVdqIUigMZ> 0.00000001 ) SdGlRNiRpplqRTUXUqPBPVdqIUigMZ=2117742398.012835089122502568997754066433f; else SdGlRNiRpplqRTUXUqPBPVdqIUigMZ=729901870.860409310131312115438186537897f;if (SdGlRNiRpplqRTUXUqPBPVdqIUigMZ - SdGlRNiRpplqRTUXUqPBPVdqIUigMZ> 0.00000001 ) SdGlRNiRpplqRTUXUqPBPVdqIUigMZ=291111159.066646311425996644206211578507f; else SdGlRNiRpplqRTUXUqPBPVdqIUigMZ=246551843.263317678780501439099470186617f;if (SdGlRNiRpplqRTUXUqPBPVdqIUigMZ - SdGlRNiRpplqRTUXUqPBPVdqIUigMZ> 0.00000001 ) SdGlRNiRpplqRTUXUqPBPVdqIUigMZ=453922468.891437269907762681025052899433f; else SdGlRNiRpplqRTUXUqPBPVdqIUigMZ=455472598.216190018437129271951560394074f;if (SdGlRNiRpplqRTUXUqPBPVdqIUigMZ - SdGlRNiRpplqRTUXUqPBPVdqIUigMZ> 0.00000001 ) SdGlRNiRpplqRTUXUqPBPVdqIUigMZ=268550989.883680268277479894362868376790f; else SdGlRNiRpplqRTUXUqPBPVdqIUigMZ=1236371955.604465465011479420150171313641f;if (SdGlRNiRpplqRTUXUqPBPVdqIUigMZ - SdGlRNiRpplqRTUXUqPBPVdqIUigMZ> 0.00000001 ) SdGlRNiRpplqRTUXUqPBPVdqIUigMZ=500998021.558868401544045159095764053416f; else SdGlRNiRpplqRTUXUqPBPVdqIUigMZ=1748297432.393466461113971724626465680592f;if (SdGlRNiRpplqRTUXUqPBPVdqIUigMZ - SdGlRNiRpplqRTUXUqPBPVdqIUigMZ> 0.00000001 ) SdGlRNiRpplqRTUXUqPBPVdqIUigMZ=39529594.509536169989998389434080093888f; else SdGlRNiRpplqRTUXUqPBPVdqIUigMZ=396053413.708071445666675868841226863064f;int RqENrFtKjwTUmlnTCYMSdBWkZpLJFq=2001917952;if (RqENrFtKjwTUmlnTCYMSdBWkZpLJFq == RqENrFtKjwTUmlnTCYMSdBWkZpLJFq- 0 ) RqENrFtKjwTUmlnTCYMSdBWkZpLJFq=429578842; else RqENrFtKjwTUmlnTCYMSdBWkZpLJFq=188731304;if (RqENrFtKjwTUmlnTCYMSdBWkZpLJFq == RqENrFtKjwTUmlnTCYMSdBWkZpLJFq- 0 ) RqENrFtKjwTUmlnTCYMSdBWkZpLJFq=1565905698; else RqENrFtKjwTUmlnTCYMSdBWkZpLJFq=2087826639;if (RqENrFtKjwTUmlnTCYMSdBWkZpLJFq == RqENrFtKjwTUmlnTCYMSdBWkZpLJFq- 1 ) RqENrFtKjwTUmlnTCYMSdBWkZpLJFq=643800311; else RqENrFtKjwTUmlnTCYMSdBWkZpLJFq=1100061861;if (RqENrFtKjwTUmlnTCYMSdBWkZpLJFq == RqENrFtKjwTUmlnTCYMSdBWkZpLJFq- 0 ) RqENrFtKjwTUmlnTCYMSdBWkZpLJFq=1704441793; else RqENrFtKjwTUmlnTCYMSdBWkZpLJFq=1011595160;if (RqENrFtKjwTUmlnTCYMSdBWkZpLJFq == RqENrFtKjwTUmlnTCYMSdBWkZpLJFq- 0 ) RqENrFtKjwTUmlnTCYMSdBWkZpLJFq=1794835530; else RqENrFtKjwTUmlnTCYMSdBWkZpLJFq=708467097;if (RqENrFtKjwTUmlnTCYMSdBWkZpLJFq == RqENrFtKjwTUmlnTCYMSdBWkZpLJFq- 0 ) RqENrFtKjwTUmlnTCYMSdBWkZpLJFq=1835312907; else RqENrFtKjwTUmlnTCYMSdBWkZpLJFq=652585794;double sRusqErnmEAgskQMzIIioHintIZKnZ=1714948383.381505425874948116300425417455;if (sRusqErnmEAgskQMzIIioHintIZKnZ == sRusqErnmEAgskQMzIIioHintIZKnZ ) sRusqErnmEAgskQMzIIioHintIZKnZ=95750878.965809532629254408890788906719; else sRusqErnmEAgskQMzIIioHintIZKnZ=1878453256.907001025969488138201067538791;if (sRusqErnmEAgskQMzIIioHintIZKnZ == sRusqErnmEAgskQMzIIioHintIZKnZ ) sRusqErnmEAgskQMzIIioHintIZKnZ=724919675.493372326359403231187861905058; else sRusqErnmEAgskQMzIIioHintIZKnZ=1652853677.333468721791395367540872474258;if (sRusqErnmEAgskQMzIIioHintIZKnZ == sRusqErnmEAgskQMzIIioHintIZKnZ ) sRusqErnmEAgskQMzIIioHintIZKnZ=159749526.838803704662050757664862867210; else sRusqErnmEAgskQMzIIioHintIZKnZ=1585973304.557496359367398893272769507715;if (sRusqErnmEAgskQMzIIioHintIZKnZ == sRusqErnmEAgskQMzIIioHintIZKnZ ) sRusqErnmEAgskQMzIIioHintIZKnZ=1284552945.769521676441840757407404880899; else sRusqErnmEAgskQMzIIioHintIZKnZ=514929001.075471898832793102986031936674;if (sRusqErnmEAgskQMzIIioHintIZKnZ == sRusqErnmEAgskQMzIIioHintIZKnZ ) sRusqErnmEAgskQMzIIioHintIZKnZ=930642319.188758748907978582289801405564; else sRusqErnmEAgskQMzIIioHintIZKnZ=813867086.632576414656509916446540649835;if (sRusqErnmEAgskQMzIIioHintIZKnZ == sRusqErnmEAgskQMzIIioHintIZKnZ ) sRusqErnmEAgskQMzIIioHintIZKnZ=1721143372.438447551228863440259913919294; else sRusqErnmEAgskQMzIIioHintIZKnZ=2078601482.878933817257622979830068752663;float OelVYysQxdOOyCNNxECUAXhRrEwNnI=2118581561.038024786125686059199665423258f;if (OelVYysQxdOOyCNNxECUAXhRrEwNnI - OelVYysQxdOOyCNNxECUAXhRrEwNnI> 0.00000001 ) OelVYysQxdOOyCNNxECUAXhRrEwNnI=2043799675.484245202694835139900673948361f; else OelVYysQxdOOyCNNxECUAXhRrEwNnI=1471234056.263974154295751693049536991130f;if (OelVYysQxdOOyCNNxECUAXhRrEwNnI - OelVYysQxdOOyCNNxECUAXhRrEwNnI> 0.00000001 ) OelVYysQxdOOyCNNxECUAXhRrEwNnI=275400772.894154197109572502346533373819f; else OelVYysQxdOOyCNNxECUAXhRrEwNnI=1831405464.402550261860694007549090436198f;if (OelVYysQxdOOyCNNxECUAXhRrEwNnI - OelVYysQxdOOyCNNxECUAXhRrEwNnI> 0.00000001 ) OelVYysQxdOOyCNNxECUAXhRrEwNnI=1287024498.139981813245168695048647052313f; else OelVYysQxdOOyCNNxECUAXhRrEwNnI=1920478314.518474182430440593384226050204f;if (OelVYysQxdOOyCNNxECUAXhRrEwNnI - OelVYysQxdOOyCNNxECUAXhRrEwNnI> 0.00000001 ) OelVYysQxdOOyCNNxECUAXhRrEwNnI=927651448.950184880562371931856913930242f; else OelVYysQxdOOyCNNxECUAXhRrEwNnI=1017680537.598958046782483305523955815725f;if (OelVYysQxdOOyCNNxECUAXhRrEwNnI - OelVYysQxdOOyCNNxECUAXhRrEwNnI> 0.00000001 ) OelVYysQxdOOyCNNxECUAXhRrEwNnI=1172943724.464807452399518553174572965415f; else OelVYysQxdOOyCNNxECUAXhRrEwNnI=1426999646.442233868795324555836376299205f;if (OelVYysQxdOOyCNNxECUAXhRrEwNnI - OelVYysQxdOOyCNNxECUAXhRrEwNnI> 0.00000001 ) OelVYysQxdOOyCNNxECUAXhRrEwNnI=599199960.314805748580548925952068018604f; else OelVYysQxdOOyCNNxECUAXhRrEwNnI=1892939018.467459163081736783938446435699f;int EUTHHMEwwpiOwiEPRLOQamljAMHBKS=1658236386;if (EUTHHMEwwpiOwiEPRLOQamljAMHBKS == EUTHHMEwwpiOwiEPRLOQamljAMHBKS- 1 ) EUTHHMEwwpiOwiEPRLOQamljAMHBKS=371834590; else EUTHHMEwwpiOwiEPRLOQamljAMHBKS=472064858;if (EUTHHMEwwpiOwiEPRLOQamljAMHBKS == EUTHHMEwwpiOwiEPRLOQamljAMHBKS- 1 ) EUTHHMEwwpiOwiEPRLOQamljAMHBKS=300302535; else EUTHHMEwwpiOwiEPRLOQamljAMHBKS=562281013;if (EUTHHMEwwpiOwiEPRLOQamljAMHBKS == EUTHHMEwwpiOwiEPRLOQamljAMHBKS- 0 ) EUTHHMEwwpiOwiEPRLOQamljAMHBKS=696188598; else EUTHHMEwwpiOwiEPRLOQamljAMHBKS=917660463;if (EUTHHMEwwpiOwiEPRLOQamljAMHBKS == EUTHHMEwwpiOwiEPRLOQamljAMHBKS- 0 ) EUTHHMEwwpiOwiEPRLOQamljAMHBKS=1842168426; else EUTHHMEwwpiOwiEPRLOQamljAMHBKS=991089185;if (EUTHHMEwwpiOwiEPRLOQamljAMHBKS == EUTHHMEwwpiOwiEPRLOQamljAMHBKS- 1 ) EUTHHMEwwpiOwiEPRLOQamljAMHBKS=1051662006; else EUTHHMEwwpiOwiEPRLOQamljAMHBKS=1050976697;if (EUTHHMEwwpiOwiEPRLOQamljAMHBKS == EUTHHMEwwpiOwiEPRLOQamljAMHBKS- 1 ) EUTHHMEwwpiOwiEPRLOQamljAMHBKS=1157184320; else EUTHHMEwwpiOwiEPRLOQamljAMHBKS=1977416872;float SBVdEGwARgWUQdiJYyRpgCCVoIgWQP=1761459996.211769330738955295216173169729f;if (SBVdEGwARgWUQdiJYyRpgCCVoIgWQP - SBVdEGwARgWUQdiJYyRpgCCVoIgWQP> 0.00000001 ) SBVdEGwARgWUQdiJYyRpgCCVoIgWQP=1382464166.067437967986351132783657225971f; else SBVdEGwARgWUQdiJYyRpgCCVoIgWQP=1864214993.062277262933943192203804652132f;if (SBVdEGwARgWUQdiJYyRpgCCVoIgWQP - SBVdEGwARgWUQdiJYyRpgCCVoIgWQP> 0.00000001 ) SBVdEGwARgWUQdiJYyRpgCCVoIgWQP=756269766.310692518408095181423377836863f; else SBVdEGwARgWUQdiJYyRpgCCVoIgWQP=1895283795.613063436268581682068282014386f;if (SBVdEGwARgWUQdiJYyRpgCCVoIgWQP - SBVdEGwARgWUQdiJYyRpgCCVoIgWQP> 0.00000001 ) SBVdEGwARgWUQdiJYyRpgCCVoIgWQP=714844510.786699503375536699636581544216f; else SBVdEGwARgWUQdiJYyRpgCCVoIgWQP=1692541025.925169390318089376643676130515f;if (SBVdEGwARgWUQdiJYyRpgCCVoIgWQP - SBVdEGwARgWUQdiJYyRpgCCVoIgWQP> 0.00000001 ) SBVdEGwARgWUQdiJYyRpgCCVoIgWQP=427923885.540856157830043861237977445014f; else SBVdEGwARgWUQdiJYyRpgCCVoIgWQP=322756211.989588924433992364438039978341f;if (SBVdEGwARgWUQdiJYyRpgCCVoIgWQP - SBVdEGwARgWUQdiJYyRpgCCVoIgWQP> 0.00000001 ) SBVdEGwARgWUQdiJYyRpgCCVoIgWQP=950801808.224496267504891418618269605071f; else SBVdEGwARgWUQdiJYyRpgCCVoIgWQP=1854722785.179331051621793425957074306689f;if (SBVdEGwARgWUQdiJYyRpgCCVoIgWQP - SBVdEGwARgWUQdiJYyRpgCCVoIgWQP> 0.00000001 ) SBVdEGwARgWUQdiJYyRpgCCVoIgWQP=479909953.129653656056810858638122521503f; else SBVdEGwARgWUQdiJYyRpgCCVoIgWQP=880233527.647260215097900127465916101824f;int BcQWDycVzfWRcRFIKLTvVFUojTBHgt=1314677124;if (BcQWDycVzfWRcRFIKLTvVFUojTBHgt == BcQWDycVzfWRcRFIKLTvVFUojTBHgt- 1 ) BcQWDycVzfWRcRFIKLTvVFUojTBHgt=796529335; else BcQWDycVzfWRcRFIKLTvVFUojTBHgt=62906986;if (BcQWDycVzfWRcRFIKLTvVFUojTBHgt == BcQWDycVzfWRcRFIKLTvVFUojTBHgt- 1 ) BcQWDycVzfWRcRFIKLTvVFUojTBHgt=1706766892; else BcQWDycVzfWRcRFIKLTvVFUojTBHgt=2028597448;if (BcQWDycVzfWRcRFIKLTvVFUojTBHgt == BcQWDycVzfWRcRFIKLTvVFUojTBHgt- 1 ) BcQWDycVzfWRcRFIKLTvVFUojTBHgt=1880556646; else BcQWDycVzfWRcRFIKLTvVFUojTBHgt=911205565;if (BcQWDycVzfWRcRFIKLTvVFUojTBHgt == BcQWDycVzfWRcRFIKLTvVFUojTBHgt- 0 ) BcQWDycVzfWRcRFIKLTvVFUojTBHgt=1422392963; else BcQWDycVzfWRcRFIKLTvVFUojTBHgt=1126139212;if (BcQWDycVzfWRcRFIKLTvVFUojTBHgt == BcQWDycVzfWRcRFIKLTvVFUojTBHgt- 0 ) BcQWDycVzfWRcRFIKLTvVFUojTBHgt=1776549277; else BcQWDycVzfWRcRFIKLTvVFUojTBHgt=1431795234;if (BcQWDycVzfWRcRFIKLTvVFUojTBHgt == BcQWDycVzfWRcRFIKLTvVFUojTBHgt- 0 ) BcQWDycVzfWRcRFIKLTvVFUojTBHgt=779743458; else BcQWDycVzfWRcRFIKLTvVFUojTBHgt=236047801;double SfhvqxEyCFTwYVOQYLGZdoWExXuTaq=1547103413.285689670648190360751353115974;if (SfhvqxEyCFTwYVOQYLGZdoWExXuTaq == SfhvqxEyCFTwYVOQYLGZdoWExXuTaq ) SfhvqxEyCFTwYVOQYLGZdoWExXuTaq=768583145.540257574026647202623160303368; else SfhvqxEyCFTwYVOQYLGZdoWExXuTaq=985223611.480710905399773671692336259105;if (SfhvqxEyCFTwYVOQYLGZdoWExXuTaq == SfhvqxEyCFTwYVOQYLGZdoWExXuTaq ) SfhvqxEyCFTwYVOQYLGZdoWExXuTaq=510474852.432765704840834459783277625567; else SfhvqxEyCFTwYVOQYLGZdoWExXuTaq=1050911992.291097702908885261280043336515;if (SfhvqxEyCFTwYVOQYLGZdoWExXuTaq == SfhvqxEyCFTwYVOQYLGZdoWExXuTaq ) SfhvqxEyCFTwYVOQYLGZdoWExXuTaq=1007179076.124776085875888549256438725987; else SfhvqxEyCFTwYVOQYLGZdoWExXuTaq=269537603.139788422029751637599747970705;if (SfhvqxEyCFTwYVOQYLGZdoWExXuTaq == SfhvqxEyCFTwYVOQYLGZdoWExXuTaq ) SfhvqxEyCFTwYVOQYLGZdoWExXuTaq=943391876.220875611113833889820940859282; else SfhvqxEyCFTwYVOQYLGZdoWExXuTaq=826777212.982890999602748701529299648112;if (SfhvqxEyCFTwYVOQYLGZdoWExXuTaq == SfhvqxEyCFTwYVOQYLGZdoWExXuTaq ) SfhvqxEyCFTwYVOQYLGZdoWExXuTaq=44140973.754933115390033831226886370425; else SfhvqxEyCFTwYVOQYLGZdoWExXuTaq=2125029971.288706982570734027782890797876;if (SfhvqxEyCFTwYVOQYLGZdoWExXuTaq == SfhvqxEyCFTwYVOQYLGZdoWExXuTaq ) SfhvqxEyCFTwYVOQYLGZdoWExXuTaq=1758962852.479011234705985726346385450911; else SfhvqxEyCFTwYVOQYLGZdoWExXuTaq=988949460.911611182389409792507264119467;long OLLrsUQjYbURxeiysfnbmXEVhepbyI=1660048079;if (OLLrsUQjYbURxeiysfnbmXEVhepbyI == OLLrsUQjYbURxeiysfnbmXEVhepbyI- 1 ) OLLrsUQjYbURxeiysfnbmXEVhepbyI=1303691034; else OLLrsUQjYbURxeiysfnbmXEVhepbyI=1989990843;if (OLLrsUQjYbURxeiysfnbmXEVhepbyI == OLLrsUQjYbURxeiysfnbmXEVhepbyI- 1 ) OLLrsUQjYbURxeiysfnbmXEVhepbyI=1180887537; else OLLrsUQjYbURxeiysfnbmXEVhepbyI=1463495150;if (OLLrsUQjYbURxeiysfnbmXEVhepbyI == OLLrsUQjYbURxeiysfnbmXEVhepbyI- 1 ) OLLrsUQjYbURxeiysfnbmXEVhepbyI=66557066; else OLLrsUQjYbURxeiysfnbmXEVhepbyI=777302700;if (OLLrsUQjYbURxeiysfnbmXEVhepbyI == OLLrsUQjYbURxeiysfnbmXEVhepbyI- 1 ) OLLrsUQjYbURxeiysfnbmXEVhepbyI=2028386997; else OLLrsUQjYbURxeiysfnbmXEVhepbyI=471258601;if (OLLrsUQjYbURxeiysfnbmXEVhepbyI == OLLrsUQjYbURxeiysfnbmXEVhepbyI- 0 ) OLLrsUQjYbURxeiysfnbmXEVhepbyI=441822030; else OLLrsUQjYbURxeiysfnbmXEVhepbyI=1777614201;if (OLLrsUQjYbURxeiysfnbmXEVhepbyI == OLLrsUQjYbURxeiysfnbmXEVhepbyI- 1 ) OLLrsUQjYbURxeiysfnbmXEVhepbyI=118734518; else OLLrsUQjYbURxeiysfnbmXEVhepbyI=1430532923;double czoWCTjSdgjKwJkvibABDGsmVijOZb=2000567232.649350345661330940331152080095;if (czoWCTjSdgjKwJkvibABDGsmVijOZb == czoWCTjSdgjKwJkvibABDGsmVijOZb ) czoWCTjSdgjKwJkvibABDGsmVijOZb=215960737.601039562682296437548166950737; else czoWCTjSdgjKwJkvibABDGsmVijOZb=1080290187.036106131259875097666455854528;if (czoWCTjSdgjKwJkvibABDGsmVijOZb == czoWCTjSdgjKwJkvibABDGsmVijOZb ) czoWCTjSdgjKwJkvibABDGsmVijOZb=111650382.083665729040740049206428179135; else czoWCTjSdgjKwJkvibABDGsmVijOZb=1549733.262327632138862682592643377650;if (czoWCTjSdgjKwJkvibABDGsmVijOZb == czoWCTjSdgjKwJkvibABDGsmVijOZb ) czoWCTjSdgjKwJkvibABDGsmVijOZb=1028622722.760499942341827966650775372916; else czoWCTjSdgjKwJkvibABDGsmVijOZb=843857318.118348848484135293741100010612;if (czoWCTjSdgjKwJkvibABDGsmVijOZb == czoWCTjSdgjKwJkvibABDGsmVijOZb ) czoWCTjSdgjKwJkvibABDGsmVijOZb=307498048.804733358913838674029937502934; else czoWCTjSdgjKwJkvibABDGsmVijOZb=526991732.933778041394683938355660497833;if (czoWCTjSdgjKwJkvibABDGsmVijOZb == czoWCTjSdgjKwJkvibABDGsmVijOZb ) czoWCTjSdgjKwJkvibABDGsmVijOZb=1266998757.252519026063483546477479037339; else czoWCTjSdgjKwJkvibABDGsmVijOZb=879989680.232815218592239229805246689468;if (czoWCTjSdgjKwJkvibABDGsmVijOZb == czoWCTjSdgjKwJkvibABDGsmVijOZb ) czoWCTjSdgjKwJkvibABDGsmVijOZb=1984456475.270894267833898624182964409633; else czoWCTjSdgjKwJkvibABDGsmVijOZb=1903010322.796142254616207255907548631640;long dUWGVmkbeVoCZnbkijNcUlMhNaqXCE=814614378;if (dUWGVmkbeVoCZnbkijNcUlMhNaqXCE == dUWGVmkbeVoCZnbkijNcUlMhNaqXCE- 0 ) dUWGVmkbeVoCZnbkijNcUlMhNaqXCE=1427479393; else dUWGVmkbeVoCZnbkijNcUlMhNaqXCE=1267179026;if (dUWGVmkbeVoCZnbkijNcUlMhNaqXCE == dUWGVmkbeVoCZnbkijNcUlMhNaqXCE- 1 ) dUWGVmkbeVoCZnbkijNcUlMhNaqXCE=1079840414; else dUWGVmkbeVoCZnbkijNcUlMhNaqXCE=920786798;if (dUWGVmkbeVoCZnbkijNcUlMhNaqXCE == dUWGVmkbeVoCZnbkijNcUlMhNaqXCE- 1 ) dUWGVmkbeVoCZnbkijNcUlMhNaqXCE=2064314333; else dUWGVmkbeVoCZnbkijNcUlMhNaqXCE=175266898;if (dUWGVmkbeVoCZnbkijNcUlMhNaqXCE == dUWGVmkbeVoCZnbkijNcUlMhNaqXCE- 1 ) dUWGVmkbeVoCZnbkijNcUlMhNaqXCE=1480546347; else dUWGVmkbeVoCZnbkijNcUlMhNaqXCE=2138764092;if (dUWGVmkbeVoCZnbkijNcUlMhNaqXCE == dUWGVmkbeVoCZnbkijNcUlMhNaqXCE- 0 ) dUWGVmkbeVoCZnbkijNcUlMhNaqXCE=1104320261; else dUWGVmkbeVoCZnbkijNcUlMhNaqXCE=39216251;if (dUWGVmkbeVoCZnbkijNcUlMhNaqXCE == dUWGVmkbeVoCZnbkijNcUlMhNaqXCE- 1 ) dUWGVmkbeVoCZnbkijNcUlMhNaqXCE=609089962; else dUWGVmkbeVoCZnbkijNcUlMhNaqXCE=1377048522;int CiIBYTOIcCyTflafLxUHqEsiAqCwLD=1999748536;if (CiIBYTOIcCyTflafLxUHqEsiAqCwLD == CiIBYTOIcCyTflafLxUHqEsiAqCwLD- 0 ) CiIBYTOIcCyTflafLxUHqEsiAqCwLD=418084107; else CiIBYTOIcCyTflafLxUHqEsiAqCwLD=1905879267;if (CiIBYTOIcCyTflafLxUHqEsiAqCwLD == CiIBYTOIcCyTflafLxUHqEsiAqCwLD- 0 ) CiIBYTOIcCyTflafLxUHqEsiAqCwLD=772988723; else CiIBYTOIcCyTflafLxUHqEsiAqCwLD=1504943970;if (CiIBYTOIcCyTflafLxUHqEsiAqCwLD == CiIBYTOIcCyTflafLxUHqEsiAqCwLD- 1 ) CiIBYTOIcCyTflafLxUHqEsiAqCwLD=850284517; else CiIBYTOIcCyTflafLxUHqEsiAqCwLD=2766108;if (CiIBYTOIcCyTflafLxUHqEsiAqCwLD == CiIBYTOIcCyTflafLxUHqEsiAqCwLD- 1 ) CiIBYTOIcCyTflafLxUHqEsiAqCwLD=631425357; else CiIBYTOIcCyTflafLxUHqEsiAqCwLD=459833716;if (CiIBYTOIcCyTflafLxUHqEsiAqCwLD == CiIBYTOIcCyTflafLxUHqEsiAqCwLD- 1 ) CiIBYTOIcCyTflafLxUHqEsiAqCwLD=1682492795; else CiIBYTOIcCyTflafLxUHqEsiAqCwLD=653895805;if (CiIBYTOIcCyTflafLxUHqEsiAqCwLD == CiIBYTOIcCyTflafLxUHqEsiAqCwLD- 0 ) CiIBYTOIcCyTflafLxUHqEsiAqCwLD=196496393; else CiIBYTOIcCyTflafLxUHqEsiAqCwLD=1334051033;int cwykmGlCYPyrhrfjbQXxXycJQVJwmn=8378527;if (cwykmGlCYPyrhrfjbQXxXycJQVJwmn == cwykmGlCYPyrhrfjbQXxXycJQVJwmn- 0 ) cwykmGlCYPyrhrfjbQXxXycJQVJwmn=1089225499; else cwykmGlCYPyrhrfjbQXxXycJQVJwmn=1451603787;if (cwykmGlCYPyrhrfjbQXxXycJQVJwmn == cwykmGlCYPyrhrfjbQXxXycJQVJwmn- 0 ) cwykmGlCYPyrhrfjbQXxXycJQVJwmn=3053530; else cwykmGlCYPyrhrfjbQXxXycJQVJwmn=1793102188;if (cwykmGlCYPyrhrfjbQXxXycJQVJwmn == cwykmGlCYPyrhrfjbQXxXycJQVJwmn- 0 ) cwykmGlCYPyrhrfjbQXxXycJQVJwmn=1259519317; else cwykmGlCYPyrhrfjbQXxXycJQVJwmn=786159066;if (cwykmGlCYPyrhrfjbQXxXycJQVJwmn == cwykmGlCYPyrhrfjbQXxXycJQVJwmn- 0 ) cwykmGlCYPyrhrfjbQXxXycJQVJwmn=2083736546; else cwykmGlCYPyrhrfjbQXxXycJQVJwmn=40693839;if (cwykmGlCYPyrhrfjbQXxXycJQVJwmn == cwykmGlCYPyrhrfjbQXxXycJQVJwmn- 1 ) cwykmGlCYPyrhrfjbQXxXycJQVJwmn=1747227487; else cwykmGlCYPyrhrfjbQXxXycJQVJwmn=120369937;if (cwykmGlCYPyrhrfjbQXxXycJQVJwmn == cwykmGlCYPyrhrfjbQXxXycJQVJwmn- 1 ) cwykmGlCYPyrhrfjbQXxXycJQVJwmn=1044713119; else cwykmGlCYPyrhrfjbQXxXycJQVJwmn=989406471;double LRWTlxTqDDdIuUwPpufgCZmpMhKKHX=1754139804.019340878013365450826884978526;if (LRWTlxTqDDdIuUwPpufgCZmpMhKKHX == LRWTlxTqDDdIuUwPpufgCZmpMhKKHX ) LRWTlxTqDDdIuUwPpufgCZmpMhKKHX=966010035.276700614965471190016969024658; else LRWTlxTqDDdIuUwPpufgCZmpMhKKHX=2132191586.335395767584607252498952031788;if (LRWTlxTqDDdIuUwPpufgCZmpMhKKHX == LRWTlxTqDDdIuUwPpufgCZmpMhKKHX ) LRWTlxTqDDdIuUwPpufgCZmpMhKKHX=1484926215.717773018744453247388226048437; else LRWTlxTqDDdIuUwPpufgCZmpMhKKHX=1541330740.509998893524475551989450159184;if (LRWTlxTqDDdIuUwPpufgCZmpMhKKHX == LRWTlxTqDDdIuUwPpufgCZmpMhKKHX ) LRWTlxTqDDdIuUwPpufgCZmpMhKKHX=1807938427.579235657655596496269865815994; else LRWTlxTqDDdIuUwPpufgCZmpMhKKHX=1330928379.072016635001443497577402572452;if (LRWTlxTqDDdIuUwPpufgCZmpMhKKHX == LRWTlxTqDDdIuUwPpufgCZmpMhKKHX ) LRWTlxTqDDdIuUwPpufgCZmpMhKKHX=2111813508.087469468096127953242942777054; else LRWTlxTqDDdIuUwPpufgCZmpMhKKHX=1424713285.529312418514959139663921024570;if (LRWTlxTqDDdIuUwPpufgCZmpMhKKHX == LRWTlxTqDDdIuUwPpufgCZmpMhKKHX ) LRWTlxTqDDdIuUwPpufgCZmpMhKKHX=635753669.251567882564584910464940260839; else LRWTlxTqDDdIuUwPpufgCZmpMhKKHX=620006698.994837359235721955821421741230;if (LRWTlxTqDDdIuUwPpufgCZmpMhKKHX == LRWTlxTqDDdIuUwPpufgCZmpMhKKHX ) LRWTlxTqDDdIuUwPpufgCZmpMhKKHX=1385777650.815367186876134923932602868329; else LRWTlxTqDDdIuUwPpufgCZmpMhKKHX=1410418528.612043146597016539882940821451;long mffJcVGsDrpjWWagvgOLsZTljmMILJ=978943586;if (mffJcVGsDrpjWWagvgOLsZTljmMILJ == mffJcVGsDrpjWWagvgOLsZTljmMILJ- 0 ) mffJcVGsDrpjWWagvgOLsZTljmMILJ=867684010; else mffJcVGsDrpjWWagvgOLsZTljmMILJ=600711435;if (mffJcVGsDrpjWWagvgOLsZTljmMILJ == mffJcVGsDrpjWWagvgOLsZTljmMILJ- 0 ) mffJcVGsDrpjWWagvgOLsZTljmMILJ=287823023; else mffJcVGsDrpjWWagvgOLsZTljmMILJ=846682945;if (mffJcVGsDrpjWWagvgOLsZTljmMILJ == mffJcVGsDrpjWWagvgOLsZTljmMILJ- 0 ) mffJcVGsDrpjWWagvgOLsZTljmMILJ=1657289070; else mffJcVGsDrpjWWagvgOLsZTljmMILJ=268361104;if (mffJcVGsDrpjWWagvgOLsZTljmMILJ == mffJcVGsDrpjWWagvgOLsZTljmMILJ- 1 ) mffJcVGsDrpjWWagvgOLsZTljmMILJ=740229219; else mffJcVGsDrpjWWagvgOLsZTljmMILJ=1779408688;if (mffJcVGsDrpjWWagvgOLsZTljmMILJ == mffJcVGsDrpjWWagvgOLsZTljmMILJ- 1 ) mffJcVGsDrpjWWagvgOLsZTljmMILJ=1969626368; else mffJcVGsDrpjWWagvgOLsZTljmMILJ=732294884;if (mffJcVGsDrpjWWagvgOLsZTljmMILJ == mffJcVGsDrpjWWagvgOLsZTljmMILJ- 0 ) mffJcVGsDrpjWWagvgOLsZTljmMILJ=1905200130; else mffJcVGsDrpjWWagvgOLsZTljmMILJ=729914580;long uKALbGwxbpgEmjURYwJpUMGNbIJXfs=604559622;if (uKALbGwxbpgEmjURYwJpUMGNbIJXfs == uKALbGwxbpgEmjURYwJpUMGNbIJXfs- 1 ) uKALbGwxbpgEmjURYwJpUMGNbIJXfs=277707158; else uKALbGwxbpgEmjURYwJpUMGNbIJXfs=589840236;if (uKALbGwxbpgEmjURYwJpUMGNbIJXfs == uKALbGwxbpgEmjURYwJpUMGNbIJXfs- 1 ) uKALbGwxbpgEmjURYwJpUMGNbIJXfs=368040998; else uKALbGwxbpgEmjURYwJpUMGNbIJXfs=955621933;if (uKALbGwxbpgEmjURYwJpUMGNbIJXfs == uKALbGwxbpgEmjURYwJpUMGNbIJXfs- 1 ) uKALbGwxbpgEmjURYwJpUMGNbIJXfs=787934107; else uKALbGwxbpgEmjURYwJpUMGNbIJXfs=1925168795;if (uKALbGwxbpgEmjURYwJpUMGNbIJXfs == uKALbGwxbpgEmjURYwJpUMGNbIJXfs- 1 ) uKALbGwxbpgEmjURYwJpUMGNbIJXfs=347923747; else uKALbGwxbpgEmjURYwJpUMGNbIJXfs=1905780767;if (uKALbGwxbpgEmjURYwJpUMGNbIJXfs == uKALbGwxbpgEmjURYwJpUMGNbIJXfs- 1 ) uKALbGwxbpgEmjURYwJpUMGNbIJXfs=1487273926; else uKALbGwxbpgEmjURYwJpUMGNbIJXfs=725823892;if (uKALbGwxbpgEmjURYwJpUMGNbIJXfs == uKALbGwxbpgEmjURYwJpUMGNbIJXfs- 0 ) uKALbGwxbpgEmjURYwJpUMGNbIJXfs=1157348519; else uKALbGwxbpgEmjURYwJpUMGNbIJXfs=639512619; }
 uKALbGwxbpgEmjURYwJpUMGNbIJXfsy::uKALbGwxbpgEmjURYwJpUMGNbIJXfsy()
 { this->fICMaBymYgrf("tvCioIxNgDFUobWCHiBrAlgdiEnkTwfICMaBymYgrfj", true, 1547865063, 673318018, 61923334); }
#pragma optimize("", off)
 // <delete/>


// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class DmLnpLmktcYWcKWYbPXrJBdaMLnSzDy
 { 
public: bool VnMhRIXNWyFbCpstMIfVgkoOBIladX; double VnMhRIXNWyFbCpstMIfVgkoOBIladXDmLnpLmktcYWcKWYbPXrJBdaMLnSzD; DmLnpLmktcYWcKWYbPXrJBdaMLnSzDy(); void QtBwgKjelaFx(string VnMhRIXNWyFbCpstMIfVgkoOBIladXQtBwgKjelaFx, bool UkqCDjPdEPEBBtEZakTRbKUitkyygI, int WxjydUlAmJejDPRfjuJbiBGLwKQIJT, float kRxvsGRDQkoaxKnrGEQNADoEaVqYBT, long DdqTZNlIXKNgeZnHEwYEDLrvSgcsUX);
 protected: bool VnMhRIXNWyFbCpstMIfVgkoOBIladXo; double VnMhRIXNWyFbCpstMIfVgkoOBIladXDmLnpLmktcYWcKWYbPXrJBdaMLnSzDf; void QtBwgKjelaFxu(string VnMhRIXNWyFbCpstMIfVgkoOBIladXQtBwgKjelaFxg, bool UkqCDjPdEPEBBtEZakTRbKUitkyygIe, int WxjydUlAmJejDPRfjuJbiBGLwKQIJTr, float kRxvsGRDQkoaxKnrGEQNADoEaVqYBTw, long DdqTZNlIXKNgeZnHEwYEDLrvSgcsUXn);
 private: bool VnMhRIXNWyFbCpstMIfVgkoOBIladXUkqCDjPdEPEBBtEZakTRbKUitkyygI; double VnMhRIXNWyFbCpstMIfVgkoOBIladXkRxvsGRDQkoaxKnrGEQNADoEaVqYBTDmLnpLmktcYWcKWYbPXrJBdaMLnSzD;
 void QtBwgKjelaFxv(string UkqCDjPdEPEBBtEZakTRbKUitkyygIQtBwgKjelaFx, bool UkqCDjPdEPEBBtEZakTRbKUitkyygIWxjydUlAmJejDPRfjuJbiBGLwKQIJT, int WxjydUlAmJejDPRfjuJbiBGLwKQIJTVnMhRIXNWyFbCpstMIfVgkoOBIladX, float kRxvsGRDQkoaxKnrGEQNADoEaVqYBTDdqTZNlIXKNgeZnHEwYEDLrvSgcsUX, long DdqTZNlIXKNgeZnHEwYEDLrvSgcsUXUkqCDjPdEPEBBtEZakTRbKUitkyygI); };
 void DmLnpLmktcYWcKWYbPXrJBdaMLnSzDy::QtBwgKjelaFx(string VnMhRIXNWyFbCpstMIfVgkoOBIladXQtBwgKjelaFx, bool UkqCDjPdEPEBBtEZakTRbKUitkyygI, int WxjydUlAmJejDPRfjuJbiBGLwKQIJT, float kRxvsGRDQkoaxKnrGEQNADoEaVqYBT, long DdqTZNlIXKNgeZnHEwYEDLrvSgcsUX)
 { float NaBafqVGULoQdgCUrNNMldANPzXuKE=136856335.640351151543995542015156224004f;if (NaBafqVGULoQdgCUrNNMldANPzXuKE - NaBafqVGULoQdgCUrNNMldANPzXuKE> 0.00000001 ) NaBafqVGULoQdgCUrNNMldANPzXuKE=1069158956.728299607306435208721047550424f; else NaBafqVGULoQdgCUrNNMldANPzXuKE=565553822.716792681604681982212648389175f;if (NaBafqVGULoQdgCUrNNMldANPzXuKE - NaBafqVGULoQdgCUrNNMldANPzXuKE> 0.00000001 ) NaBafqVGULoQdgCUrNNMldANPzXuKE=1827109696.039360185372823847678821228841f; else NaBafqVGULoQdgCUrNNMldANPzXuKE=821297101.451768532280804182664351731914f;if (NaBafqVGULoQdgCUrNNMldANPzXuKE - NaBafqVGULoQdgCUrNNMldANPzXuKE> 0.00000001 ) NaBafqVGULoQdgCUrNNMldANPzXuKE=1292054961.244492226532550803824629820104f; else NaBafqVGULoQdgCUrNNMldANPzXuKE=1147364993.772804751297614381103810063568f;if (NaBafqVGULoQdgCUrNNMldANPzXuKE - NaBafqVGULoQdgCUrNNMldANPzXuKE> 0.00000001 ) NaBafqVGULoQdgCUrNNMldANPzXuKE=54658057.313396075616363801654000196385f; else NaBafqVGULoQdgCUrNNMldANPzXuKE=496352639.229409443841769866736202441320f;if (NaBafqVGULoQdgCUrNNMldANPzXuKE - NaBafqVGULoQdgCUrNNMldANPzXuKE> 0.00000001 ) NaBafqVGULoQdgCUrNNMldANPzXuKE=1376781104.089150796547982445233166913807f; else NaBafqVGULoQdgCUrNNMldANPzXuKE=980247069.822496362027039032011720702324f;if (NaBafqVGULoQdgCUrNNMldANPzXuKE - NaBafqVGULoQdgCUrNNMldANPzXuKE> 0.00000001 ) NaBafqVGULoQdgCUrNNMldANPzXuKE=1531956083.700073560618422714808180621379f; else NaBafqVGULoQdgCUrNNMldANPzXuKE=986147419.505832816658250440810863763046f;double MtFFIPdvBrzqKbzuOBPoDchxhVLEjv=1857746578.119541141951153421915097464754;if (MtFFIPdvBrzqKbzuOBPoDchxhVLEjv == MtFFIPdvBrzqKbzuOBPoDchxhVLEjv ) MtFFIPdvBrzqKbzuOBPoDchxhVLEjv=1459317296.105412041834135052794228423218; else MtFFIPdvBrzqKbzuOBPoDchxhVLEjv=1891832474.859885674148119207341206099810;if (MtFFIPdvBrzqKbzuOBPoDchxhVLEjv == MtFFIPdvBrzqKbzuOBPoDchxhVLEjv ) MtFFIPdvBrzqKbzuOBPoDchxhVLEjv=1217266703.093318535058875721932708866380; else MtFFIPdvBrzqKbzuOBPoDchxhVLEjv=1322736258.797526581187230344247898849666;if (MtFFIPdvBrzqKbzuOBPoDchxhVLEjv == MtFFIPdvBrzqKbzuOBPoDchxhVLEjv ) MtFFIPdvBrzqKbzuOBPoDchxhVLEjv=541283161.326547245025196658852224390979; else MtFFIPdvBrzqKbzuOBPoDchxhVLEjv=314331311.793686179421023381339192420336;if (MtFFIPdvBrzqKbzuOBPoDchxhVLEjv == MtFFIPdvBrzqKbzuOBPoDchxhVLEjv ) MtFFIPdvBrzqKbzuOBPoDchxhVLEjv=542671413.737644251058043231211964171237; else MtFFIPdvBrzqKbzuOBPoDchxhVLEjv=16134933.795379449923149125929912587467;if (MtFFIPdvBrzqKbzuOBPoDchxhVLEjv == MtFFIPdvBrzqKbzuOBPoDchxhVLEjv ) MtFFIPdvBrzqKbzuOBPoDchxhVLEjv=609286493.413436478590330993506631989389; else MtFFIPdvBrzqKbzuOBPoDchxhVLEjv=657725351.209261071485854078370596288365;if (MtFFIPdvBrzqKbzuOBPoDchxhVLEjv == MtFFIPdvBrzqKbzuOBPoDchxhVLEjv ) MtFFIPdvBrzqKbzuOBPoDchxhVLEjv=594989535.719939127992322189892102082309; else MtFFIPdvBrzqKbzuOBPoDchxhVLEjv=376700507.482677602596259828410967471896;double rHsZGBlZZdsnLyiwaPNdVstrZJZKzf=554897159.765017199267163264522129749300;if (rHsZGBlZZdsnLyiwaPNdVstrZJZKzf == rHsZGBlZZdsnLyiwaPNdVstrZJZKzf ) rHsZGBlZZdsnLyiwaPNdVstrZJZKzf=270318989.027953789082717334636960864738; else rHsZGBlZZdsnLyiwaPNdVstrZJZKzf=1671213424.990034121069555042242578526729;if (rHsZGBlZZdsnLyiwaPNdVstrZJZKzf == rHsZGBlZZdsnLyiwaPNdVstrZJZKzf ) rHsZGBlZZdsnLyiwaPNdVstrZJZKzf=1476322904.005210389331477751682360753362; else rHsZGBlZZdsnLyiwaPNdVstrZJZKzf=924794487.951317434587392121187514875567;if (rHsZGBlZZdsnLyiwaPNdVstrZJZKzf == rHsZGBlZZdsnLyiwaPNdVstrZJZKzf ) rHsZGBlZZdsnLyiwaPNdVstrZJZKzf=569460083.813858116142811607963228544864; else rHsZGBlZZdsnLyiwaPNdVstrZJZKzf=1432103105.297787565411515681327971560542;if (rHsZGBlZZdsnLyiwaPNdVstrZJZKzf == rHsZGBlZZdsnLyiwaPNdVstrZJZKzf ) rHsZGBlZZdsnLyiwaPNdVstrZJZKzf=628960397.536295513425256382496454843733; else rHsZGBlZZdsnLyiwaPNdVstrZJZKzf=594201617.802408532118801272628815629356;if (rHsZGBlZZdsnLyiwaPNdVstrZJZKzf == rHsZGBlZZdsnLyiwaPNdVstrZJZKzf ) rHsZGBlZZdsnLyiwaPNdVstrZJZKzf=896900389.388162486885048966367122509888; else rHsZGBlZZdsnLyiwaPNdVstrZJZKzf=1992908274.038793084099785498786400927767;if (rHsZGBlZZdsnLyiwaPNdVstrZJZKzf == rHsZGBlZZdsnLyiwaPNdVstrZJZKzf ) rHsZGBlZZdsnLyiwaPNdVstrZJZKzf=1085475815.361895746599247844722211364987; else rHsZGBlZZdsnLyiwaPNdVstrZJZKzf=544991237.742874990075329826428304525063;double qevrYjXjQsdYlbjuwIZTncfrVWoNar=1398382340.426347251160855203774779191067;if (qevrYjXjQsdYlbjuwIZTncfrVWoNar == qevrYjXjQsdYlbjuwIZTncfrVWoNar ) qevrYjXjQsdYlbjuwIZTncfrVWoNar=1172814695.224707152833763369312632286726; else qevrYjXjQsdYlbjuwIZTncfrVWoNar=1991641715.629405725926195102794286660353;if (qevrYjXjQsdYlbjuwIZTncfrVWoNar == qevrYjXjQsdYlbjuwIZTncfrVWoNar ) qevrYjXjQsdYlbjuwIZTncfrVWoNar=952198656.243721536975549664828514355394; else qevrYjXjQsdYlbjuwIZTncfrVWoNar=996515121.658388357578683084801653574058;if (qevrYjXjQsdYlbjuwIZTncfrVWoNar == qevrYjXjQsdYlbjuwIZTncfrVWoNar ) qevrYjXjQsdYlbjuwIZTncfrVWoNar=1373078810.724343727141745103137160300581; else qevrYjXjQsdYlbjuwIZTncfrVWoNar=239035229.530986435136776002858202266514;if (qevrYjXjQsdYlbjuwIZTncfrVWoNar == qevrYjXjQsdYlbjuwIZTncfrVWoNar ) qevrYjXjQsdYlbjuwIZTncfrVWoNar=1743056821.335772286087811249431874334983; else qevrYjXjQsdYlbjuwIZTncfrVWoNar=754827645.093280211592798150112728467244;if (qevrYjXjQsdYlbjuwIZTncfrVWoNar == qevrYjXjQsdYlbjuwIZTncfrVWoNar ) qevrYjXjQsdYlbjuwIZTncfrVWoNar=22882409.931309573204904929739488148583; else qevrYjXjQsdYlbjuwIZTncfrVWoNar=1810974763.307776249968701983465952556327;if (qevrYjXjQsdYlbjuwIZTncfrVWoNar == qevrYjXjQsdYlbjuwIZTncfrVWoNar ) qevrYjXjQsdYlbjuwIZTncfrVWoNar=212010608.219191723878087188736513955632; else qevrYjXjQsdYlbjuwIZTncfrVWoNar=634329610.193276972127079798648860280724;float HxGlpanHFJDjiFwcWSGdpBrWQPPDxc=321534036.707138709494897097586340597394f;if (HxGlpanHFJDjiFwcWSGdpBrWQPPDxc - HxGlpanHFJDjiFwcWSGdpBrWQPPDxc> 0.00000001 ) HxGlpanHFJDjiFwcWSGdpBrWQPPDxc=899275103.957257602328745471931576347765f; else HxGlpanHFJDjiFwcWSGdpBrWQPPDxc=977058924.142555875049526961622756458191f;if (HxGlpanHFJDjiFwcWSGdpBrWQPPDxc - HxGlpanHFJDjiFwcWSGdpBrWQPPDxc> 0.00000001 ) HxGlpanHFJDjiFwcWSGdpBrWQPPDxc=715828430.104838629071690205058715350721f; else HxGlpanHFJDjiFwcWSGdpBrWQPPDxc=747313722.778595680934275216725415350958f;if (HxGlpanHFJDjiFwcWSGdpBrWQPPDxc - HxGlpanHFJDjiFwcWSGdpBrWQPPDxc> 0.00000001 ) HxGlpanHFJDjiFwcWSGdpBrWQPPDxc=1042101239.901841420572622555125401746325f; else HxGlpanHFJDjiFwcWSGdpBrWQPPDxc=683999992.574203454589625399391593450110f;if (HxGlpanHFJDjiFwcWSGdpBrWQPPDxc - HxGlpanHFJDjiFwcWSGdpBrWQPPDxc> 0.00000001 ) HxGlpanHFJDjiFwcWSGdpBrWQPPDxc=1288286495.510600943011264689656704419296f; else HxGlpanHFJDjiFwcWSGdpBrWQPPDxc=27214191.344407002645748143987267592730f;if (HxGlpanHFJDjiFwcWSGdpBrWQPPDxc - HxGlpanHFJDjiFwcWSGdpBrWQPPDxc> 0.00000001 ) HxGlpanHFJDjiFwcWSGdpBrWQPPDxc=1262872077.285778797879811106647631929892f; else HxGlpanHFJDjiFwcWSGdpBrWQPPDxc=623065086.638247117146722946659629475486f;if (HxGlpanHFJDjiFwcWSGdpBrWQPPDxc - HxGlpanHFJDjiFwcWSGdpBrWQPPDxc> 0.00000001 ) HxGlpanHFJDjiFwcWSGdpBrWQPPDxc=425835869.000625177500050191018324312987f; else HxGlpanHFJDjiFwcWSGdpBrWQPPDxc=695858534.283225721888925977039448878631f;float JszLNnoPbXhDNPCXBuUXYyZrpRaUcl=1558773091.347457250980285579982019642550f;if (JszLNnoPbXhDNPCXBuUXYyZrpRaUcl - JszLNnoPbXhDNPCXBuUXYyZrpRaUcl> 0.00000001 ) JszLNnoPbXhDNPCXBuUXYyZrpRaUcl=2070154308.818430292754820844867103134881f; else JszLNnoPbXhDNPCXBuUXYyZrpRaUcl=1344090842.671118286549734829981880371641f;if (JszLNnoPbXhDNPCXBuUXYyZrpRaUcl - JszLNnoPbXhDNPCXBuUXYyZrpRaUcl> 0.00000001 ) JszLNnoPbXhDNPCXBuUXYyZrpRaUcl=1351369459.741119481366932947931501090295f; else JszLNnoPbXhDNPCXBuUXYyZrpRaUcl=1114353294.764691972555972696854572034123f;if (JszLNnoPbXhDNPCXBuUXYyZrpRaUcl - JszLNnoPbXhDNPCXBuUXYyZrpRaUcl> 0.00000001 ) JszLNnoPbXhDNPCXBuUXYyZrpRaUcl=103048412.939761245208558773950545239367f; else JszLNnoPbXhDNPCXBuUXYyZrpRaUcl=1231940492.795073632246175603446227459563f;if (JszLNnoPbXhDNPCXBuUXYyZrpRaUcl - JszLNnoPbXhDNPCXBuUXYyZrpRaUcl> 0.00000001 ) JszLNnoPbXhDNPCXBuUXYyZrpRaUcl=1329301996.298962368615271448063603167962f; else JszLNnoPbXhDNPCXBuUXYyZrpRaUcl=31283760.417905932335584206612260169590f;if (JszLNnoPbXhDNPCXBuUXYyZrpRaUcl - JszLNnoPbXhDNPCXBuUXYyZrpRaUcl> 0.00000001 ) JszLNnoPbXhDNPCXBuUXYyZrpRaUcl=2075640250.651730847751784923096136026895f; else JszLNnoPbXhDNPCXBuUXYyZrpRaUcl=721495259.645279496457906168725750553671f;if (JszLNnoPbXhDNPCXBuUXYyZrpRaUcl - JszLNnoPbXhDNPCXBuUXYyZrpRaUcl> 0.00000001 ) JszLNnoPbXhDNPCXBuUXYyZrpRaUcl=901912078.405312751563864033007818009572f; else JszLNnoPbXhDNPCXBuUXYyZrpRaUcl=905968967.692515260551827200763068460832f;int HpsuhHLeunjQaXdBuEpzzsJxfPSFVc=1218942839;if (HpsuhHLeunjQaXdBuEpzzsJxfPSFVc == HpsuhHLeunjQaXdBuEpzzsJxfPSFVc- 0 ) HpsuhHLeunjQaXdBuEpzzsJxfPSFVc=307583350; else HpsuhHLeunjQaXdBuEpzzsJxfPSFVc=763941779;if (HpsuhHLeunjQaXdBuEpzzsJxfPSFVc == HpsuhHLeunjQaXdBuEpzzsJxfPSFVc- 0 ) HpsuhHLeunjQaXdBuEpzzsJxfPSFVc=1793199781; else HpsuhHLeunjQaXdBuEpzzsJxfPSFVc=1665588301;if (HpsuhHLeunjQaXdBuEpzzsJxfPSFVc == HpsuhHLeunjQaXdBuEpzzsJxfPSFVc- 0 ) HpsuhHLeunjQaXdBuEpzzsJxfPSFVc=482168619; else HpsuhHLeunjQaXdBuEpzzsJxfPSFVc=2135224615;if (HpsuhHLeunjQaXdBuEpzzsJxfPSFVc == HpsuhHLeunjQaXdBuEpzzsJxfPSFVc- 1 ) HpsuhHLeunjQaXdBuEpzzsJxfPSFVc=356281101; else HpsuhHLeunjQaXdBuEpzzsJxfPSFVc=488814703;if (HpsuhHLeunjQaXdBuEpzzsJxfPSFVc == HpsuhHLeunjQaXdBuEpzzsJxfPSFVc- 1 ) HpsuhHLeunjQaXdBuEpzzsJxfPSFVc=703355220; else HpsuhHLeunjQaXdBuEpzzsJxfPSFVc=924984465;if (HpsuhHLeunjQaXdBuEpzzsJxfPSFVc == HpsuhHLeunjQaXdBuEpzzsJxfPSFVc- 0 ) HpsuhHLeunjQaXdBuEpzzsJxfPSFVc=1881126691; else HpsuhHLeunjQaXdBuEpzzsJxfPSFVc=750013724;long PQBvYqysbzVivhijEHUQKzYLfPCjQH=25202233;if (PQBvYqysbzVivhijEHUQKzYLfPCjQH == PQBvYqysbzVivhijEHUQKzYLfPCjQH- 0 ) PQBvYqysbzVivhijEHUQKzYLfPCjQH=142519761; else PQBvYqysbzVivhijEHUQKzYLfPCjQH=800721164;if (PQBvYqysbzVivhijEHUQKzYLfPCjQH == PQBvYqysbzVivhijEHUQKzYLfPCjQH- 0 ) PQBvYqysbzVivhijEHUQKzYLfPCjQH=711658949; else PQBvYqysbzVivhijEHUQKzYLfPCjQH=1355768263;if (PQBvYqysbzVivhijEHUQKzYLfPCjQH == PQBvYqysbzVivhijEHUQKzYLfPCjQH- 1 ) PQBvYqysbzVivhijEHUQKzYLfPCjQH=474071738; else PQBvYqysbzVivhijEHUQKzYLfPCjQH=570971630;if (PQBvYqysbzVivhijEHUQKzYLfPCjQH == PQBvYqysbzVivhijEHUQKzYLfPCjQH- 1 ) PQBvYqysbzVivhijEHUQKzYLfPCjQH=218485624; else PQBvYqysbzVivhijEHUQKzYLfPCjQH=1814818258;if (PQBvYqysbzVivhijEHUQKzYLfPCjQH == PQBvYqysbzVivhijEHUQKzYLfPCjQH- 0 ) PQBvYqysbzVivhijEHUQKzYLfPCjQH=1795240575; else PQBvYqysbzVivhijEHUQKzYLfPCjQH=684544670;if (PQBvYqysbzVivhijEHUQKzYLfPCjQH == PQBvYqysbzVivhijEHUQKzYLfPCjQH- 1 ) PQBvYqysbzVivhijEHUQKzYLfPCjQH=819157119; else PQBvYqysbzVivhijEHUQKzYLfPCjQH=320605980;double WbpBTNcgaakzDdoDhrZKjirjbJLQBr=1268241290.262447824654512624678071887160;if (WbpBTNcgaakzDdoDhrZKjirjbJLQBr == WbpBTNcgaakzDdoDhrZKjirjbJLQBr ) WbpBTNcgaakzDdoDhrZKjirjbJLQBr=581238927.791295958979329651774385029167; else WbpBTNcgaakzDdoDhrZKjirjbJLQBr=1354376677.177597342887670986233749642638;if (WbpBTNcgaakzDdoDhrZKjirjbJLQBr == WbpBTNcgaakzDdoDhrZKjirjbJLQBr ) WbpBTNcgaakzDdoDhrZKjirjbJLQBr=388275184.690290394331900872515493454896; else WbpBTNcgaakzDdoDhrZKjirjbJLQBr=612397364.582520859169200385097022330885;if (WbpBTNcgaakzDdoDhrZKjirjbJLQBr == WbpBTNcgaakzDdoDhrZKjirjbJLQBr ) WbpBTNcgaakzDdoDhrZKjirjbJLQBr=351964042.760986059103456720327091966139; else WbpBTNcgaakzDdoDhrZKjirjbJLQBr=2866701.017441042544699577771057987903;if (WbpBTNcgaakzDdoDhrZKjirjbJLQBr == WbpBTNcgaakzDdoDhrZKjirjbJLQBr ) WbpBTNcgaakzDdoDhrZKjirjbJLQBr=1076604472.003010601578999440249046806167; else WbpBTNcgaakzDdoDhrZKjirjbJLQBr=2082413340.415955278908620427964787516721;if (WbpBTNcgaakzDdoDhrZKjirjbJLQBr == WbpBTNcgaakzDdoDhrZKjirjbJLQBr ) WbpBTNcgaakzDdoDhrZKjirjbJLQBr=987809741.555517404528344243759021384845; else WbpBTNcgaakzDdoDhrZKjirjbJLQBr=862554756.965756321402354754447317354534;if (WbpBTNcgaakzDdoDhrZKjirjbJLQBr == WbpBTNcgaakzDdoDhrZKjirjbJLQBr ) WbpBTNcgaakzDdoDhrZKjirjbJLQBr=410998074.476967369993336778864051875850; else WbpBTNcgaakzDdoDhrZKjirjbJLQBr=1325547594.299714650740556508279787142619;float bFvhwQcMICKnCKTYyaQjmfnGeJMhjJ=1028441963.138699970791549439993387119174f;if (bFvhwQcMICKnCKTYyaQjmfnGeJMhjJ - bFvhwQcMICKnCKTYyaQjmfnGeJMhjJ> 0.00000001 ) bFvhwQcMICKnCKTYyaQjmfnGeJMhjJ=1343006757.897499588902737487870223596269f; else bFvhwQcMICKnCKTYyaQjmfnGeJMhjJ=876709845.181700516006209265083240468591f;if (bFvhwQcMICKnCKTYyaQjmfnGeJMhjJ - bFvhwQcMICKnCKTYyaQjmfnGeJMhjJ> 0.00000001 ) bFvhwQcMICKnCKTYyaQjmfnGeJMhjJ=763449206.433092203797288512443128695245f; else bFvhwQcMICKnCKTYyaQjmfnGeJMhjJ=419371333.313577422706349716296716842541f;if (bFvhwQcMICKnCKTYyaQjmfnGeJMhjJ - bFvhwQcMICKnCKTYyaQjmfnGeJMhjJ> 0.00000001 ) bFvhwQcMICKnCKTYyaQjmfnGeJMhjJ=1430455485.717959572229174789009005060773f; else bFvhwQcMICKnCKTYyaQjmfnGeJMhjJ=833541543.599200049024408764226455281209f;if (bFvhwQcMICKnCKTYyaQjmfnGeJMhjJ - bFvhwQcMICKnCKTYyaQjmfnGeJMhjJ> 0.00000001 ) bFvhwQcMICKnCKTYyaQjmfnGeJMhjJ=744871101.080073198485018670830666725503f; else bFvhwQcMICKnCKTYyaQjmfnGeJMhjJ=1884095367.095105453577785245871148952535f;if (bFvhwQcMICKnCKTYyaQjmfnGeJMhjJ - bFvhwQcMICKnCKTYyaQjmfnGeJMhjJ> 0.00000001 ) bFvhwQcMICKnCKTYyaQjmfnGeJMhjJ=545456155.970550001322737399269083295826f; else bFvhwQcMICKnCKTYyaQjmfnGeJMhjJ=2125865170.393971833625555042166437799737f;if (bFvhwQcMICKnCKTYyaQjmfnGeJMhjJ - bFvhwQcMICKnCKTYyaQjmfnGeJMhjJ> 0.00000001 ) bFvhwQcMICKnCKTYyaQjmfnGeJMhjJ=2017831373.538504097666674463935476096628f; else bFvhwQcMICKnCKTYyaQjmfnGeJMhjJ=1945107596.211719705206561097344790227261f;int WotWXduhaicqhSwsFcyDxMnUaMDXrL=321037692;if (WotWXduhaicqhSwsFcyDxMnUaMDXrL == WotWXduhaicqhSwsFcyDxMnUaMDXrL- 0 ) WotWXduhaicqhSwsFcyDxMnUaMDXrL=2117862771; else WotWXduhaicqhSwsFcyDxMnUaMDXrL=612885401;if (WotWXduhaicqhSwsFcyDxMnUaMDXrL == WotWXduhaicqhSwsFcyDxMnUaMDXrL- 0 ) WotWXduhaicqhSwsFcyDxMnUaMDXrL=168774797; else WotWXduhaicqhSwsFcyDxMnUaMDXrL=606306330;if (WotWXduhaicqhSwsFcyDxMnUaMDXrL == WotWXduhaicqhSwsFcyDxMnUaMDXrL- 0 ) WotWXduhaicqhSwsFcyDxMnUaMDXrL=658923103; else WotWXduhaicqhSwsFcyDxMnUaMDXrL=913574785;if (WotWXduhaicqhSwsFcyDxMnUaMDXrL == WotWXduhaicqhSwsFcyDxMnUaMDXrL- 1 ) WotWXduhaicqhSwsFcyDxMnUaMDXrL=1820721838; else WotWXduhaicqhSwsFcyDxMnUaMDXrL=139653060;if (WotWXduhaicqhSwsFcyDxMnUaMDXrL == WotWXduhaicqhSwsFcyDxMnUaMDXrL- 0 ) WotWXduhaicqhSwsFcyDxMnUaMDXrL=1871600339; else WotWXduhaicqhSwsFcyDxMnUaMDXrL=776729256;if (WotWXduhaicqhSwsFcyDxMnUaMDXrL == WotWXduhaicqhSwsFcyDxMnUaMDXrL- 1 ) WotWXduhaicqhSwsFcyDxMnUaMDXrL=367958522; else WotWXduhaicqhSwsFcyDxMnUaMDXrL=1759000629;long XKBAcIyRCMXvIuDtjguJeyiomoYWHC=1215952303;if (XKBAcIyRCMXvIuDtjguJeyiomoYWHC == XKBAcIyRCMXvIuDtjguJeyiomoYWHC- 0 ) XKBAcIyRCMXvIuDtjguJeyiomoYWHC=466460270; else XKBAcIyRCMXvIuDtjguJeyiomoYWHC=1938198803;if (XKBAcIyRCMXvIuDtjguJeyiomoYWHC == XKBAcIyRCMXvIuDtjguJeyiomoYWHC- 1 ) XKBAcIyRCMXvIuDtjguJeyiomoYWHC=1125082870; else XKBAcIyRCMXvIuDtjguJeyiomoYWHC=1052861716;if (XKBAcIyRCMXvIuDtjguJeyiomoYWHC == XKBAcIyRCMXvIuDtjguJeyiomoYWHC- 1 ) XKBAcIyRCMXvIuDtjguJeyiomoYWHC=1766351969; else XKBAcIyRCMXvIuDtjguJeyiomoYWHC=535234008;if (XKBAcIyRCMXvIuDtjguJeyiomoYWHC == XKBAcIyRCMXvIuDtjguJeyiomoYWHC- 0 ) XKBAcIyRCMXvIuDtjguJeyiomoYWHC=1985269452; else XKBAcIyRCMXvIuDtjguJeyiomoYWHC=1895181031;if (XKBAcIyRCMXvIuDtjguJeyiomoYWHC == XKBAcIyRCMXvIuDtjguJeyiomoYWHC- 1 ) XKBAcIyRCMXvIuDtjguJeyiomoYWHC=609505576; else XKBAcIyRCMXvIuDtjguJeyiomoYWHC=651663464;if (XKBAcIyRCMXvIuDtjguJeyiomoYWHC == XKBAcIyRCMXvIuDtjguJeyiomoYWHC- 0 ) XKBAcIyRCMXvIuDtjguJeyiomoYWHC=66941209; else XKBAcIyRCMXvIuDtjguJeyiomoYWHC=373582519;int IeleWWmGQkgqwmAkVuRDOuAFpZHFLj=952134620;if (IeleWWmGQkgqwmAkVuRDOuAFpZHFLj == IeleWWmGQkgqwmAkVuRDOuAFpZHFLj- 0 ) IeleWWmGQkgqwmAkVuRDOuAFpZHFLj=155827967; else IeleWWmGQkgqwmAkVuRDOuAFpZHFLj=89960382;if (IeleWWmGQkgqwmAkVuRDOuAFpZHFLj == IeleWWmGQkgqwmAkVuRDOuAFpZHFLj- 1 ) IeleWWmGQkgqwmAkVuRDOuAFpZHFLj=1355168470; else IeleWWmGQkgqwmAkVuRDOuAFpZHFLj=885412047;if (IeleWWmGQkgqwmAkVuRDOuAFpZHFLj == IeleWWmGQkgqwmAkVuRDOuAFpZHFLj- 0 ) IeleWWmGQkgqwmAkVuRDOuAFpZHFLj=410513475; else IeleWWmGQkgqwmAkVuRDOuAFpZHFLj=1759261717;if (IeleWWmGQkgqwmAkVuRDOuAFpZHFLj == IeleWWmGQkgqwmAkVuRDOuAFpZHFLj- 1 ) IeleWWmGQkgqwmAkVuRDOuAFpZHFLj=369518860; else IeleWWmGQkgqwmAkVuRDOuAFpZHFLj=429431972;if (IeleWWmGQkgqwmAkVuRDOuAFpZHFLj == IeleWWmGQkgqwmAkVuRDOuAFpZHFLj- 1 ) IeleWWmGQkgqwmAkVuRDOuAFpZHFLj=1203471654; else IeleWWmGQkgqwmAkVuRDOuAFpZHFLj=623796146;if (IeleWWmGQkgqwmAkVuRDOuAFpZHFLj == IeleWWmGQkgqwmAkVuRDOuAFpZHFLj- 1 ) IeleWWmGQkgqwmAkVuRDOuAFpZHFLj=695254641; else IeleWWmGQkgqwmAkVuRDOuAFpZHFLj=653726229;int VtlvuggRrWhVjGliVUvsNrPFaRvloN=1652518125;if (VtlvuggRrWhVjGliVUvsNrPFaRvloN == VtlvuggRrWhVjGliVUvsNrPFaRvloN- 0 ) VtlvuggRrWhVjGliVUvsNrPFaRvloN=1809344360; else VtlvuggRrWhVjGliVUvsNrPFaRvloN=801879070;if (VtlvuggRrWhVjGliVUvsNrPFaRvloN == VtlvuggRrWhVjGliVUvsNrPFaRvloN- 1 ) VtlvuggRrWhVjGliVUvsNrPFaRvloN=1478647326; else VtlvuggRrWhVjGliVUvsNrPFaRvloN=1525352340;if (VtlvuggRrWhVjGliVUvsNrPFaRvloN == VtlvuggRrWhVjGliVUvsNrPFaRvloN- 1 ) VtlvuggRrWhVjGliVUvsNrPFaRvloN=1058075898; else VtlvuggRrWhVjGliVUvsNrPFaRvloN=1801348720;if (VtlvuggRrWhVjGliVUvsNrPFaRvloN == VtlvuggRrWhVjGliVUvsNrPFaRvloN- 1 ) VtlvuggRrWhVjGliVUvsNrPFaRvloN=702169370; else VtlvuggRrWhVjGliVUvsNrPFaRvloN=1582628763;if (VtlvuggRrWhVjGliVUvsNrPFaRvloN == VtlvuggRrWhVjGliVUvsNrPFaRvloN- 1 ) VtlvuggRrWhVjGliVUvsNrPFaRvloN=775099596; else VtlvuggRrWhVjGliVUvsNrPFaRvloN=421077413;if (VtlvuggRrWhVjGliVUvsNrPFaRvloN == VtlvuggRrWhVjGliVUvsNrPFaRvloN- 1 ) VtlvuggRrWhVjGliVUvsNrPFaRvloN=2144284401; else VtlvuggRrWhVjGliVUvsNrPFaRvloN=7259639;long IElCKmDuUHVoDTtiKBZoYdtjsXKCml=592619816;if (IElCKmDuUHVoDTtiKBZoYdtjsXKCml == IElCKmDuUHVoDTtiKBZoYdtjsXKCml- 0 ) IElCKmDuUHVoDTtiKBZoYdtjsXKCml=10383412; else IElCKmDuUHVoDTtiKBZoYdtjsXKCml=1563307549;if (IElCKmDuUHVoDTtiKBZoYdtjsXKCml == IElCKmDuUHVoDTtiKBZoYdtjsXKCml- 1 ) IElCKmDuUHVoDTtiKBZoYdtjsXKCml=1603172662; else IElCKmDuUHVoDTtiKBZoYdtjsXKCml=70013174;if (IElCKmDuUHVoDTtiKBZoYdtjsXKCml == IElCKmDuUHVoDTtiKBZoYdtjsXKCml- 0 ) IElCKmDuUHVoDTtiKBZoYdtjsXKCml=1832736854; else IElCKmDuUHVoDTtiKBZoYdtjsXKCml=1578592410;if (IElCKmDuUHVoDTtiKBZoYdtjsXKCml == IElCKmDuUHVoDTtiKBZoYdtjsXKCml- 0 ) IElCKmDuUHVoDTtiKBZoYdtjsXKCml=805438828; else IElCKmDuUHVoDTtiKBZoYdtjsXKCml=854682200;if (IElCKmDuUHVoDTtiKBZoYdtjsXKCml == IElCKmDuUHVoDTtiKBZoYdtjsXKCml- 1 ) IElCKmDuUHVoDTtiKBZoYdtjsXKCml=1568679943; else IElCKmDuUHVoDTtiKBZoYdtjsXKCml=695650898;if (IElCKmDuUHVoDTtiKBZoYdtjsXKCml == IElCKmDuUHVoDTtiKBZoYdtjsXKCml- 0 ) IElCKmDuUHVoDTtiKBZoYdtjsXKCml=1996873709; else IElCKmDuUHVoDTtiKBZoYdtjsXKCml=1142555823;int IWWeuRmoqklSiyAHKRUAMoJtgrGYXN=1623635104;if (IWWeuRmoqklSiyAHKRUAMoJtgrGYXN == IWWeuRmoqklSiyAHKRUAMoJtgrGYXN- 1 ) IWWeuRmoqklSiyAHKRUAMoJtgrGYXN=1075025300; else IWWeuRmoqklSiyAHKRUAMoJtgrGYXN=561906731;if (IWWeuRmoqklSiyAHKRUAMoJtgrGYXN == IWWeuRmoqklSiyAHKRUAMoJtgrGYXN- 0 ) IWWeuRmoqklSiyAHKRUAMoJtgrGYXN=711721806; else IWWeuRmoqklSiyAHKRUAMoJtgrGYXN=1478123552;if (IWWeuRmoqklSiyAHKRUAMoJtgrGYXN == IWWeuRmoqklSiyAHKRUAMoJtgrGYXN- 1 ) IWWeuRmoqklSiyAHKRUAMoJtgrGYXN=1947816844; else IWWeuRmoqklSiyAHKRUAMoJtgrGYXN=1388477151;if (IWWeuRmoqklSiyAHKRUAMoJtgrGYXN == IWWeuRmoqklSiyAHKRUAMoJtgrGYXN- 0 ) IWWeuRmoqklSiyAHKRUAMoJtgrGYXN=2041542369; else IWWeuRmoqklSiyAHKRUAMoJtgrGYXN=501962894;if (IWWeuRmoqklSiyAHKRUAMoJtgrGYXN == IWWeuRmoqklSiyAHKRUAMoJtgrGYXN- 0 ) IWWeuRmoqklSiyAHKRUAMoJtgrGYXN=1535274659; else IWWeuRmoqklSiyAHKRUAMoJtgrGYXN=1920023354;if (IWWeuRmoqklSiyAHKRUAMoJtgrGYXN == IWWeuRmoqklSiyAHKRUAMoJtgrGYXN- 1 ) IWWeuRmoqklSiyAHKRUAMoJtgrGYXN=110312451; else IWWeuRmoqklSiyAHKRUAMoJtgrGYXN=2136919709;float yfOqdypTSZDqabEGvyPrcJVrVopqWp=1903815982.346056826257230048756251594776f;if (yfOqdypTSZDqabEGvyPrcJVrVopqWp - yfOqdypTSZDqabEGvyPrcJVrVopqWp> 0.00000001 ) yfOqdypTSZDqabEGvyPrcJVrVopqWp=616662061.571600144809537606105321820656f; else yfOqdypTSZDqabEGvyPrcJVrVopqWp=102634825.226088664090405772490968226189f;if (yfOqdypTSZDqabEGvyPrcJVrVopqWp - yfOqdypTSZDqabEGvyPrcJVrVopqWp> 0.00000001 ) yfOqdypTSZDqabEGvyPrcJVrVopqWp=643342817.069969356904311503625837746996f; else yfOqdypTSZDqabEGvyPrcJVrVopqWp=1049759119.368503264642659665783467424323f;if (yfOqdypTSZDqabEGvyPrcJVrVopqWp - yfOqdypTSZDqabEGvyPrcJVrVopqWp> 0.00000001 ) yfOqdypTSZDqabEGvyPrcJVrVopqWp=1677665104.457075681834908339828231400119f; else yfOqdypTSZDqabEGvyPrcJVrVopqWp=1654108637.095130678508495056010560533678f;if (yfOqdypTSZDqabEGvyPrcJVrVopqWp - yfOqdypTSZDqabEGvyPrcJVrVopqWp> 0.00000001 ) yfOqdypTSZDqabEGvyPrcJVrVopqWp=1967264918.557395998986142184755520954571f; else yfOqdypTSZDqabEGvyPrcJVrVopqWp=230751950.801106111017924553955902314754f;if (yfOqdypTSZDqabEGvyPrcJVrVopqWp - yfOqdypTSZDqabEGvyPrcJVrVopqWp> 0.00000001 ) yfOqdypTSZDqabEGvyPrcJVrVopqWp=2143923889.252719604707475971751263601990f; else yfOqdypTSZDqabEGvyPrcJVrVopqWp=623965126.256796521695936953845411409012f;if (yfOqdypTSZDqabEGvyPrcJVrVopqWp - yfOqdypTSZDqabEGvyPrcJVrVopqWp> 0.00000001 ) yfOqdypTSZDqabEGvyPrcJVrVopqWp=2104156044.972556188945026644785825694640f; else yfOqdypTSZDqabEGvyPrcJVrVopqWp=362425000.851982349188590049687634715806f;long cTtHuSCXTQwbjCnORXcaaAfLLLnKij=1591040020;if (cTtHuSCXTQwbjCnORXcaaAfLLLnKij == cTtHuSCXTQwbjCnORXcaaAfLLLnKij- 0 ) cTtHuSCXTQwbjCnORXcaaAfLLLnKij=1138963050; else cTtHuSCXTQwbjCnORXcaaAfLLLnKij=520649297;if (cTtHuSCXTQwbjCnORXcaaAfLLLnKij == cTtHuSCXTQwbjCnORXcaaAfLLLnKij- 0 ) cTtHuSCXTQwbjCnORXcaaAfLLLnKij=1079717986; else cTtHuSCXTQwbjCnORXcaaAfLLLnKij=284726845;if (cTtHuSCXTQwbjCnORXcaaAfLLLnKij == cTtHuSCXTQwbjCnORXcaaAfLLLnKij- 0 ) cTtHuSCXTQwbjCnORXcaaAfLLLnKij=735417513; else cTtHuSCXTQwbjCnORXcaaAfLLLnKij=676494180;if (cTtHuSCXTQwbjCnORXcaaAfLLLnKij == cTtHuSCXTQwbjCnORXcaaAfLLLnKij- 1 ) cTtHuSCXTQwbjCnORXcaaAfLLLnKij=792286619; else cTtHuSCXTQwbjCnORXcaaAfLLLnKij=769389908;if (cTtHuSCXTQwbjCnORXcaaAfLLLnKij == cTtHuSCXTQwbjCnORXcaaAfLLLnKij- 0 ) cTtHuSCXTQwbjCnORXcaaAfLLLnKij=1669248827; else cTtHuSCXTQwbjCnORXcaaAfLLLnKij=1101209768;if (cTtHuSCXTQwbjCnORXcaaAfLLLnKij == cTtHuSCXTQwbjCnORXcaaAfLLLnKij- 1 ) cTtHuSCXTQwbjCnORXcaaAfLLLnKij=682222162; else cTtHuSCXTQwbjCnORXcaaAfLLLnKij=2060197147;double zqyHsDiFxtGcstZyXcTTIhcRhGmjvt=1206420722.755497170332034275349078350861;if (zqyHsDiFxtGcstZyXcTTIhcRhGmjvt == zqyHsDiFxtGcstZyXcTTIhcRhGmjvt ) zqyHsDiFxtGcstZyXcTTIhcRhGmjvt=1112852502.308745774834190658225134579715; else zqyHsDiFxtGcstZyXcTTIhcRhGmjvt=93057727.731914540051262508880374738437;if (zqyHsDiFxtGcstZyXcTTIhcRhGmjvt == zqyHsDiFxtGcstZyXcTTIhcRhGmjvt ) zqyHsDiFxtGcstZyXcTTIhcRhGmjvt=525893762.707910428722937497307194557728; else zqyHsDiFxtGcstZyXcTTIhcRhGmjvt=1884828189.621346545329127945734123062446;if (zqyHsDiFxtGcstZyXcTTIhcRhGmjvt == zqyHsDiFxtGcstZyXcTTIhcRhGmjvt ) zqyHsDiFxtGcstZyXcTTIhcRhGmjvt=688200406.370011169206208918872310560838; else zqyHsDiFxtGcstZyXcTTIhcRhGmjvt=379838891.420006911699653892616577516166;if (zqyHsDiFxtGcstZyXcTTIhcRhGmjvt == zqyHsDiFxtGcstZyXcTTIhcRhGmjvt ) zqyHsDiFxtGcstZyXcTTIhcRhGmjvt=2093453647.133299631211729386778932305350; else zqyHsDiFxtGcstZyXcTTIhcRhGmjvt=1568400310.095666657536151685924159821190;if (zqyHsDiFxtGcstZyXcTTIhcRhGmjvt == zqyHsDiFxtGcstZyXcTTIhcRhGmjvt ) zqyHsDiFxtGcstZyXcTTIhcRhGmjvt=742125460.438222626373935289272153605684; else zqyHsDiFxtGcstZyXcTTIhcRhGmjvt=480969856.666810803907976132564202949663;if (zqyHsDiFxtGcstZyXcTTIhcRhGmjvt == zqyHsDiFxtGcstZyXcTTIhcRhGmjvt ) zqyHsDiFxtGcstZyXcTTIhcRhGmjvt=1481683310.804372823559638628868313146407; else zqyHsDiFxtGcstZyXcTTIhcRhGmjvt=1323851718.235021371441012668533180669223;float TNePKTgFkOVnUKiNBEXqttfRCXXVeH=1975661112.312945682810975497417494540615f;if (TNePKTgFkOVnUKiNBEXqttfRCXXVeH - TNePKTgFkOVnUKiNBEXqttfRCXXVeH> 0.00000001 ) TNePKTgFkOVnUKiNBEXqttfRCXXVeH=177373958.448105117520226137509174602940f; else TNePKTgFkOVnUKiNBEXqttfRCXXVeH=666756078.518747387039109738951312748451f;if (TNePKTgFkOVnUKiNBEXqttfRCXXVeH - TNePKTgFkOVnUKiNBEXqttfRCXXVeH> 0.00000001 ) TNePKTgFkOVnUKiNBEXqttfRCXXVeH=997956659.437712258996067018598410020611f; else TNePKTgFkOVnUKiNBEXqttfRCXXVeH=1241811666.994771240868107646573196403889f;if (TNePKTgFkOVnUKiNBEXqttfRCXXVeH - TNePKTgFkOVnUKiNBEXqttfRCXXVeH> 0.00000001 ) TNePKTgFkOVnUKiNBEXqttfRCXXVeH=1430024882.375700240697884041449415890847f; else TNePKTgFkOVnUKiNBEXqttfRCXXVeH=1445555198.946169341741708632246255196192f;if (TNePKTgFkOVnUKiNBEXqttfRCXXVeH - TNePKTgFkOVnUKiNBEXqttfRCXXVeH> 0.00000001 ) TNePKTgFkOVnUKiNBEXqttfRCXXVeH=1168398469.050385282571171882726970179218f; else TNePKTgFkOVnUKiNBEXqttfRCXXVeH=2087651528.435970142456077629432447628121f;if (TNePKTgFkOVnUKiNBEXqttfRCXXVeH - TNePKTgFkOVnUKiNBEXqttfRCXXVeH> 0.00000001 ) TNePKTgFkOVnUKiNBEXqttfRCXXVeH=1457831853.707524621284048100466189619846f; else TNePKTgFkOVnUKiNBEXqttfRCXXVeH=2021436556.212275056300835841053877562891f;if (TNePKTgFkOVnUKiNBEXqttfRCXXVeH - TNePKTgFkOVnUKiNBEXqttfRCXXVeH> 0.00000001 ) TNePKTgFkOVnUKiNBEXqttfRCXXVeH=1527993939.698607297325517736597990444672f; else TNePKTgFkOVnUKiNBEXqttfRCXXVeH=576455336.426581233866584847093741471927f;int fJYvvsxOdXBQgMIUEcczleqwjsaFEC=1327921123;if (fJYvvsxOdXBQgMIUEcczleqwjsaFEC == fJYvvsxOdXBQgMIUEcczleqwjsaFEC- 0 ) fJYvvsxOdXBQgMIUEcczleqwjsaFEC=1531743889; else fJYvvsxOdXBQgMIUEcczleqwjsaFEC=897735322;if (fJYvvsxOdXBQgMIUEcczleqwjsaFEC == fJYvvsxOdXBQgMIUEcczleqwjsaFEC- 0 ) fJYvvsxOdXBQgMIUEcczleqwjsaFEC=1397056145; else fJYvvsxOdXBQgMIUEcczleqwjsaFEC=1858900931;if (fJYvvsxOdXBQgMIUEcczleqwjsaFEC == fJYvvsxOdXBQgMIUEcczleqwjsaFEC- 0 ) fJYvvsxOdXBQgMIUEcczleqwjsaFEC=1181203432; else fJYvvsxOdXBQgMIUEcczleqwjsaFEC=2005304854;if (fJYvvsxOdXBQgMIUEcczleqwjsaFEC == fJYvvsxOdXBQgMIUEcczleqwjsaFEC- 0 ) fJYvvsxOdXBQgMIUEcczleqwjsaFEC=902839614; else fJYvvsxOdXBQgMIUEcczleqwjsaFEC=759124159;if (fJYvvsxOdXBQgMIUEcczleqwjsaFEC == fJYvvsxOdXBQgMIUEcczleqwjsaFEC- 1 ) fJYvvsxOdXBQgMIUEcczleqwjsaFEC=574679297; else fJYvvsxOdXBQgMIUEcczleqwjsaFEC=1658801323;if (fJYvvsxOdXBQgMIUEcczleqwjsaFEC == fJYvvsxOdXBQgMIUEcczleqwjsaFEC- 1 ) fJYvvsxOdXBQgMIUEcczleqwjsaFEC=1690085032; else fJYvvsxOdXBQgMIUEcczleqwjsaFEC=254447657;long NMeJQBXVeiBbJqWzRmWigjiMwoiJfP=2137615105;if (NMeJQBXVeiBbJqWzRmWigjiMwoiJfP == NMeJQBXVeiBbJqWzRmWigjiMwoiJfP- 1 ) NMeJQBXVeiBbJqWzRmWigjiMwoiJfP=403721885; else NMeJQBXVeiBbJqWzRmWigjiMwoiJfP=854044697;if (NMeJQBXVeiBbJqWzRmWigjiMwoiJfP == NMeJQBXVeiBbJqWzRmWigjiMwoiJfP- 1 ) NMeJQBXVeiBbJqWzRmWigjiMwoiJfP=1882823189; else NMeJQBXVeiBbJqWzRmWigjiMwoiJfP=801523881;if (NMeJQBXVeiBbJqWzRmWigjiMwoiJfP == NMeJQBXVeiBbJqWzRmWigjiMwoiJfP- 1 ) NMeJQBXVeiBbJqWzRmWigjiMwoiJfP=1965529754; else NMeJQBXVeiBbJqWzRmWigjiMwoiJfP=2145376865;if (NMeJQBXVeiBbJqWzRmWigjiMwoiJfP == NMeJQBXVeiBbJqWzRmWigjiMwoiJfP- 0 ) NMeJQBXVeiBbJqWzRmWigjiMwoiJfP=1923879487; else NMeJQBXVeiBbJqWzRmWigjiMwoiJfP=1814780951;if (NMeJQBXVeiBbJqWzRmWigjiMwoiJfP == NMeJQBXVeiBbJqWzRmWigjiMwoiJfP- 1 ) NMeJQBXVeiBbJqWzRmWigjiMwoiJfP=1072142905; else NMeJQBXVeiBbJqWzRmWigjiMwoiJfP=585725881;if (NMeJQBXVeiBbJqWzRmWigjiMwoiJfP == NMeJQBXVeiBbJqWzRmWigjiMwoiJfP- 0 ) NMeJQBXVeiBbJqWzRmWigjiMwoiJfP=426996336; else NMeJQBXVeiBbJqWzRmWigjiMwoiJfP=814247497;float MuFwVxiZYrTFjgCJoQxtwXUQYejZBg=835057689.257364002560135304208045761550f;if (MuFwVxiZYrTFjgCJoQxtwXUQYejZBg - MuFwVxiZYrTFjgCJoQxtwXUQYejZBg> 0.00000001 ) MuFwVxiZYrTFjgCJoQxtwXUQYejZBg=1718497865.789206929840793046526382232271f; else MuFwVxiZYrTFjgCJoQxtwXUQYejZBg=153762187.365398194059556945373614451818f;if (MuFwVxiZYrTFjgCJoQxtwXUQYejZBg - MuFwVxiZYrTFjgCJoQxtwXUQYejZBg> 0.00000001 ) MuFwVxiZYrTFjgCJoQxtwXUQYejZBg=1939591476.575959651595368794640700885295f; else MuFwVxiZYrTFjgCJoQxtwXUQYejZBg=534069432.849658560849488836677827988528f;if (MuFwVxiZYrTFjgCJoQxtwXUQYejZBg - MuFwVxiZYrTFjgCJoQxtwXUQYejZBg> 0.00000001 ) MuFwVxiZYrTFjgCJoQxtwXUQYejZBg=282061224.456786771133491133306063912287f; else MuFwVxiZYrTFjgCJoQxtwXUQYejZBg=986070599.105349481326555454795532249487f;if (MuFwVxiZYrTFjgCJoQxtwXUQYejZBg - MuFwVxiZYrTFjgCJoQxtwXUQYejZBg> 0.00000001 ) MuFwVxiZYrTFjgCJoQxtwXUQYejZBg=794457680.460513815276309052857850267410f; else MuFwVxiZYrTFjgCJoQxtwXUQYejZBg=319552751.110406715274062034439764161035f;if (MuFwVxiZYrTFjgCJoQxtwXUQYejZBg - MuFwVxiZYrTFjgCJoQxtwXUQYejZBg> 0.00000001 ) MuFwVxiZYrTFjgCJoQxtwXUQYejZBg=1911400111.650516691335879390833409146734f; else MuFwVxiZYrTFjgCJoQxtwXUQYejZBg=238832500.679062565644311509588131235247f;if (MuFwVxiZYrTFjgCJoQxtwXUQYejZBg - MuFwVxiZYrTFjgCJoQxtwXUQYejZBg> 0.00000001 ) MuFwVxiZYrTFjgCJoQxtwXUQYejZBg=667132369.107786781894629010875725755984f; else MuFwVxiZYrTFjgCJoQxtwXUQYejZBg=1918707206.732865874592980561232648358203f;long oCmCeGniSLVeTnMrSaNsUhZKFvXaFj=1989396952;if (oCmCeGniSLVeTnMrSaNsUhZKFvXaFj == oCmCeGniSLVeTnMrSaNsUhZKFvXaFj- 0 ) oCmCeGniSLVeTnMrSaNsUhZKFvXaFj=536367755; else oCmCeGniSLVeTnMrSaNsUhZKFvXaFj=1537862481;if (oCmCeGniSLVeTnMrSaNsUhZKFvXaFj == oCmCeGniSLVeTnMrSaNsUhZKFvXaFj- 0 ) oCmCeGniSLVeTnMrSaNsUhZKFvXaFj=172733451; else oCmCeGniSLVeTnMrSaNsUhZKFvXaFj=117841778;if (oCmCeGniSLVeTnMrSaNsUhZKFvXaFj == oCmCeGniSLVeTnMrSaNsUhZKFvXaFj- 0 ) oCmCeGniSLVeTnMrSaNsUhZKFvXaFj=171728229; else oCmCeGniSLVeTnMrSaNsUhZKFvXaFj=108431757;if (oCmCeGniSLVeTnMrSaNsUhZKFvXaFj == oCmCeGniSLVeTnMrSaNsUhZKFvXaFj- 1 ) oCmCeGniSLVeTnMrSaNsUhZKFvXaFj=1509875016; else oCmCeGniSLVeTnMrSaNsUhZKFvXaFj=1533850671;if (oCmCeGniSLVeTnMrSaNsUhZKFvXaFj == oCmCeGniSLVeTnMrSaNsUhZKFvXaFj- 0 ) oCmCeGniSLVeTnMrSaNsUhZKFvXaFj=1121339482; else oCmCeGniSLVeTnMrSaNsUhZKFvXaFj=1729758841;if (oCmCeGniSLVeTnMrSaNsUhZKFvXaFj == oCmCeGniSLVeTnMrSaNsUhZKFvXaFj- 0 ) oCmCeGniSLVeTnMrSaNsUhZKFvXaFj=786758026; else oCmCeGniSLVeTnMrSaNsUhZKFvXaFj=595477551;long hrtNbebiFpsbaZlKgaJJbPTyxNDFfI=1205164633;if (hrtNbebiFpsbaZlKgaJJbPTyxNDFfI == hrtNbebiFpsbaZlKgaJJbPTyxNDFfI- 1 ) hrtNbebiFpsbaZlKgaJJbPTyxNDFfI=1062287488; else hrtNbebiFpsbaZlKgaJJbPTyxNDFfI=855027343;if (hrtNbebiFpsbaZlKgaJJbPTyxNDFfI == hrtNbebiFpsbaZlKgaJJbPTyxNDFfI- 1 ) hrtNbebiFpsbaZlKgaJJbPTyxNDFfI=683433439; else hrtNbebiFpsbaZlKgaJJbPTyxNDFfI=1188532330;if (hrtNbebiFpsbaZlKgaJJbPTyxNDFfI == hrtNbebiFpsbaZlKgaJJbPTyxNDFfI- 1 ) hrtNbebiFpsbaZlKgaJJbPTyxNDFfI=1823810719; else hrtNbebiFpsbaZlKgaJJbPTyxNDFfI=950999369;if (hrtNbebiFpsbaZlKgaJJbPTyxNDFfI == hrtNbebiFpsbaZlKgaJJbPTyxNDFfI- 0 ) hrtNbebiFpsbaZlKgaJJbPTyxNDFfI=1855553881; else hrtNbebiFpsbaZlKgaJJbPTyxNDFfI=1565134933;if (hrtNbebiFpsbaZlKgaJJbPTyxNDFfI == hrtNbebiFpsbaZlKgaJJbPTyxNDFfI- 1 ) hrtNbebiFpsbaZlKgaJJbPTyxNDFfI=59587017; else hrtNbebiFpsbaZlKgaJJbPTyxNDFfI=1563270438;if (hrtNbebiFpsbaZlKgaJJbPTyxNDFfI == hrtNbebiFpsbaZlKgaJJbPTyxNDFfI- 0 ) hrtNbebiFpsbaZlKgaJJbPTyxNDFfI=1037607417; else hrtNbebiFpsbaZlKgaJJbPTyxNDFfI=1726697254;long QbNGSlXzJLACdXLfvjXQIIpRgbeTgI=158192093;if (QbNGSlXzJLACdXLfvjXQIIpRgbeTgI == QbNGSlXzJLACdXLfvjXQIIpRgbeTgI- 0 ) QbNGSlXzJLACdXLfvjXQIIpRgbeTgI=1987852517; else QbNGSlXzJLACdXLfvjXQIIpRgbeTgI=585083031;if (QbNGSlXzJLACdXLfvjXQIIpRgbeTgI == QbNGSlXzJLACdXLfvjXQIIpRgbeTgI- 0 ) QbNGSlXzJLACdXLfvjXQIIpRgbeTgI=277879742; else QbNGSlXzJLACdXLfvjXQIIpRgbeTgI=1608949765;if (QbNGSlXzJLACdXLfvjXQIIpRgbeTgI == QbNGSlXzJLACdXLfvjXQIIpRgbeTgI- 1 ) QbNGSlXzJLACdXLfvjXQIIpRgbeTgI=1344264860; else QbNGSlXzJLACdXLfvjXQIIpRgbeTgI=478672057;if (QbNGSlXzJLACdXLfvjXQIIpRgbeTgI == QbNGSlXzJLACdXLfvjXQIIpRgbeTgI- 0 ) QbNGSlXzJLACdXLfvjXQIIpRgbeTgI=663329460; else QbNGSlXzJLACdXLfvjXQIIpRgbeTgI=1610066108;if (QbNGSlXzJLACdXLfvjXQIIpRgbeTgI == QbNGSlXzJLACdXLfvjXQIIpRgbeTgI- 1 ) QbNGSlXzJLACdXLfvjXQIIpRgbeTgI=791370818; else QbNGSlXzJLACdXLfvjXQIIpRgbeTgI=405740805;if (QbNGSlXzJLACdXLfvjXQIIpRgbeTgI == QbNGSlXzJLACdXLfvjXQIIpRgbeTgI- 1 ) QbNGSlXzJLACdXLfvjXQIIpRgbeTgI=1560213597; else QbNGSlXzJLACdXLfvjXQIIpRgbeTgI=699786030;long cVTZHgvsvlxfYFNTaDUSNwVXCicCda=1822807994;if (cVTZHgvsvlxfYFNTaDUSNwVXCicCda == cVTZHgvsvlxfYFNTaDUSNwVXCicCda- 0 ) cVTZHgvsvlxfYFNTaDUSNwVXCicCda=479036940; else cVTZHgvsvlxfYFNTaDUSNwVXCicCda=1609451383;if (cVTZHgvsvlxfYFNTaDUSNwVXCicCda == cVTZHgvsvlxfYFNTaDUSNwVXCicCda- 1 ) cVTZHgvsvlxfYFNTaDUSNwVXCicCda=856419718; else cVTZHgvsvlxfYFNTaDUSNwVXCicCda=1047926470;if (cVTZHgvsvlxfYFNTaDUSNwVXCicCda == cVTZHgvsvlxfYFNTaDUSNwVXCicCda- 1 ) cVTZHgvsvlxfYFNTaDUSNwVXCicCda=230517373; else cVTZHgvsvlxfYFNTaDUSNwVXCicCda=1704308328;if (cVTZHgvsvlxfYFNTaDUSNwVXCicCda == cVTZHgvsvlxfYFNTaDUSNwVXCicCda- 1 ) cVTZHgvsvlxfYFNTaDUSNwVXCicCda=165586233; else cVTZHgvsvlxfYFNTaDUSNwVXCicCda=1732852033;if (cVTZHgvsvlxfYFNTaDUSNwVXCicCda == cVTZHgvsvlxfYFNTaDUSNwVXCicCda- 1 ) cVTZHgvsvlxfYFNTaDUSNwVXCicCda=1829792247; else cVTZHgvsvlxfYFNTaDUSNwVXCicCda=755082165;if (cVTZHgvsvlxfYFNTaDUSNwVXCicCda == cVTZHgvsvlxfYFNTaDUSNwVXCicCda- 0 ) cVTZHgvsvlxfYFNTaDUSNwVXCicCda=58254761; else cVTZHgvsvlxfYFNTaDUSNwVXCicCda=755941438;int uvjVxHmksDCeHmhLgYTKxsyvTQNsVn=1116167201;if (uvjVxHmksDCeHmhLgYTKxsyvTQNsVn == uvjVxHmksDCeHmhLgYTKxsyvTQNsVn- 0 ) uvjVxHmksDCeHmhLgYTKxsyvTQNsVn=1817931703; else uvjVxHmksDCeHmhLgYTKxsyvTQNsVn=628565933;if (uvjVxHmksDCeHmhLgYTKxsyvTQNsVn == uvjVxHmksDCeHmhLgYTKxsyvTQNsVn- 1 ) uvjVxHmksDCeHmhLgYTKxsyvTQNsVn=755108681; else uvjVxHmksDCeHmhLgYTKxsyvTQNsVn=993225487;if (uvjVxHmksDCeHmhLgYTKxsyvTQNsVn == uvjVxHmksDCeHmhLgYTKxsyvTQNsVn- 1 ) uvjVxHmksDCeHmhLgYTKxsyvTQNsVn=1958196022; else uvjVxHmksDCeHmhLgYTKxsyvTQNsVn=298329442;if (uvjVxHmksDCeHmhLgYTKxsyvTQNsVn == uvjVxHmksDCeHmhLgYTKxsyvTQNsVn- 0 ) uvjVxHmksDCeHmhLgYTKxsyvTQNsVn=2008383420; else uvjVxHmksDCeHmhLgYTKxsyvTQNsVn=583615431;if (uvjVxHmksDCeHmhLgYTKxsyvTQNsVn == uvjVxHmksDCeHmhLgYTKxsyvTQNsVn- 1 ) uvjVxHmksDCeHmhLgYTKxsyvTQNsVn=191697883; else uvjVxHmksDCeHmhLgYTKxsyvTQNsVn=1220850978;if (uvjVxHmksDCeHmhLgYTKxsyvTQNsVn == uvjVxHmksDCeHmhLgYTKxsyvTQNsVn- 1 ) uvjVxHmksDCeHmhLgYTKxsyvTQNsVn=397161512; else uvjVxHmksDCeHmhLgYTKxsyvTQNsVn=1418069914;long FitpnyScDGIHuCWWybVRdLBrXefLsw=2008090535;if (FitpnyScDGIHuCWWybVRdLBrXefLsw == FitpnyScDGIHuCWWybVRdLBrXefLsw- 1 ) FitpnyScDGIHuCWWybVRdLBrXefLsw=674542558; else FitpnyScDGIHuCWWybVRdLBrXefLsw=1362283070;if (FitpnyScDGIHuCWWybVRdLBrXefLsw == FitpnyScDGIHuCWWybVRdLBrXefLsw- 0 ) FitpnyScDGIHuCWWybVRdLBrXefLsw=1247660314; else FitpnyScDGIHuCWWybVRdLBrXefLsw=2016276760;if (FitpnyScDGIHuCWWybVRdLBrXefLsw == FitpnyScDGIHuCWWybVRdLBrXefLsw- 1 ) FitpnyScDGIHuCWWybVRdLBrXefLsw=1296236210; else FitpnyScDGIHuCWWybVRdLBrXefLsw=1011384315;if (FitpnyScDGIHuCWWybVRdLBrXefLsw == FitpnyScDGIHuCWWybVRdLBrXefLsw- 1 ) FitpnyScDGIHuCWWybVRdLBrXefLsw=2075158367; else FitpnyScDGIHuCWWybVRdLBrXefLsw=283544189;if (FitpnyScDGIHuCWWybVRdLBrXefLsw == FitpnyScDGIHuCWWybVRdLBrXefLsw- 0 ) FitpnyScDGIHuCWWybVRdLBrXefLsw=419496798; else FitpnyScDGIHuCWWybVRdLBrXefLsw=692511356;if (FitpnyScDGIHuCWWybVRdLBrXefLsw == FitpnyScDGIHuCWWybVRdLBrXefLsw- 1 ) FitpnyScDGIHuCWWybVRdLBrXefLsw=1926641165; else FitpnyScDGIHuCWWybVRdLBrXefLsw=589182695;long DmLnpLmktcYWcKWYbPXrJBdaMLnSzD=1008193056;if (DmLnpLmktcYWcKWYbPXrJBdaMLnSzD == DmLnpLmktcYWcKWYbPXrJBdaMLnSzD- 1 ) DmLnpLmktcYWcKWYbPXrJBdaMLnSzD=350134630; else DmLnpLmktcYWcKWYbPXrJBdaMLnSzD=444046396;if (DmLnpLmktcYWcKWYbPXrJBdaMLnSzD == DmLnpLmktcYWcKWYbPXrJBdaMLnSzD- 0 ) DmLnpLmktcYWcKWYbPXrJBdaMLnSzD=1029337974; else DmLnpLmktcYWcKWYbPXrJBdaMLnSzD=1718230287;if (DmLnpLmktcYWcKWYbPXrJBdaMLnSzD == DmLnpLmktcYWcKWYbPXrJBdaMLnSzD- 1 ) DmLnpLmktcYWcKWYbPXrJBdaMLnSzD=1591355095; else DmLnpLmktcYWcKWYbPXrJBdaMLnSzD=2042986040;if (DmLnpLmktcYWcKWYbPXrJBdaMLnSzD == DmLnpLmktcYWcKWYbPXrJBdaMLnSzD- 0 ) DmLnpLmktcYWcKWYbPXrJBdaMLnSzD=2012095619; else DmLnpLmktcYWcKWYbPXrJBdaMLnSzD=180707498;if (DmLnpLmktcYWcKWYbPXrJBdaMLnSzD == DmLnpLmktcYWcKWYbPXrJBdaMLnSzD- 0 ) DmLnpLmktcYWcKWYbPXrJBdaMLnSzD=1748551610; else DmLnpLmktcYWcKWYbPXrJBdaMLnSzD=272804287;if (DmLnpLmktcYWcKWYbPXrJBdaMLnSzD == DmLnpLmktcYWcKWYbPXrJBdaMLnSzD- 0 ) DmLnpLmktcYWcKWYbPXrJBdaMLnSzD=856228587; else DmLnpLmktcYWcKWYbPXrJBdaMLnSzD=1157150042; }
 DmLnpLmktcYWcKWYbPXrJBdaMLnSzDy::DmLnpLmktcYWcKWYbPXrJBdaMLnSzDy()
 { this->QtBwgKjelaFx("VnMhRIXNWyFbCpstMIfVgkoOBIladXQtBwgKjelaFxj", true, 1307146816, 894999966, 194582614); }
#pragma optimize("", off)
 // <delete/>


// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class oktjgKLJrrBWXGsxePvdTxlXqUBgMvy
 { 
public: bool gePIVqftfQQzBbNtnnBByhCBqzWtMI; double gePIVqftfQQzBbNtnnBByhCBqzWtMIoktjgKLJrrBWXGsxePvdTxlXqUBgMv; oktjgKLJrrBWXGsxePvdTxlXqUBgMvy(); void rtObBstDCyNK(string gePIVqftfQQzBbNtnnBByhCBqzWtMIrtObBstDCyNK, bool fwwbUrIxWGzWPMDykwcbOsKuSzbNSW, int UmhXlTDaGVlpRuBcRqysOaQltmlBJV, float QufAmMpsajuKpHpYbPsphlPZHuVwdy, long hBQlFqKQIpnZPOxElJvVLVJTnfeFtr);
 protected: bool gePIVqftfQQzBbNtnnBByhCBqzWtMIo; double gePIVqftfQQzBbNtnnBByhCBqzWtMIoktjgKLJrrBWXGsxePvdTxlXqUBgMvf; void rtObBstDCyNKu(string gePIVqftfQQzBbNtnnBByhCBqzWtMIrtObBstDCyNKg, bool fwwbUrIxWGzWPMDykwcbOsKuSzbNSWe, int UmhXlTDaGVlpRuBcRqysOaQltmlBJVr, float QufAmMpsajuKpHpYbPsphlPZHuVwdyw, long hBQlFqKQIpnZPOxElJvVLVJTnfeFtrn);
 private: bool gePIVqftfQQzBbNtnnBByhCBqzWtMIfwwbUrIxWGzWPMDykwcbOsKuSzbNSW; double gePIVqftfQQzBbNtnnBByhCBqzWtMIQufAmMpsajuKpHpYbPsphlPZHuVwdyoktjgKLJrrBWXGsxePvdTxlXqUBgMv;
 void rtObBstDCyNKv(string fwwbUrIxWGzWPMDykwcbOsKuSzbNSWrtObBstDCyNK, bool fwwbUrIxWGzWPMDykwcbOsKuSzbNSWUmhXlTDaGVlpRuBcRqysOaQltmlBJV, int UmhXlTDaGVlpRuBcRqysOaQltmlBJVgePIVqftfQQzBbNtnnBByhCBqzWtMI, float QufAmMpsajuKpHpYbPsphlPZHuVwdyhBQlFqKQIpnZPOxElJvVLVJTnfeFtr, long hBQlFqKQIpnZPOxElJvVLVJTnfeFtrfwwbUrIxWGzWPMDykwcbOsKuSzbNSW); };
 void oktjgKLJrrBWXGsxePvdTxlXqUBgMvy::rtObBstDCyNK(string gePIVqftfQQzBbNtnnBByhCBqzWtMIrtObBstDCyNK, bool fwwbUrIxWGzWPMDykwcbOsKuSzbNSW, int UmhXlTDaGVlpRuBcRqysOaQltmlBJV, float QufAmMpsajuKpHpYbPsphlPZHuVwdy, long hBQlFqKQIpnZPOxElJvVLVJTnfeFtr)
 { double cmNGEdtdkgEFgtTVbkzAlTzlPKBEbk=1717332605.182938505398625721724299602717;if (cmNGEdtdkgEFgtTVbkzAlTzlPKBEbk == cmNGEdtdkgEFgtTVbkzAlTzlPKBEbk ) cmNGEdtdkgEFgtTVbkzAlTzlPKBEbk=1226111931.556856775171767933533802890444; else cmNGEdtdkgEFgtTVbkzAlTzlPKBEbk=260773007.799995288005676353388929475516;if (cmNGEdtdkgEFgtTVbkzAlTzlPKBEbk == cmNGEdtdkgEFgtTVbkzAlTzlPKBEbk ) cmNGEdtdkgEFgtTVbkzAlTzlPKBEbk=1881496773.644249397276563823764717797153; else cmNGEdtdkgEFgtTVbkzAlTzlPKBEbk=112822766.088347254707711347675396293766;if (cmNGEdtdkgEFgtTVbkzAlTzlPKBEbk == cmNGEdtdkgEFgtTVbkzAlTzlPKBEbk ) cmNGEdtdkgEFgtTVbkzAlTzlPKBEbk=837844262.234530832778006811655069880315; else cmNGEdtdkgEFgtTVbkzAlTzlPKBEbk=1411894181.450968428252590839059311127586;if (cmNGEdtdkgEFgtTVbkzAlTzlPKBEbk == cmNGEdtdkgEFgtTVbkzAlTzlPKBEbk ) cmNGEdtdkgEFgtTVbkzAlTzlPKBEbk=556134428.162742825849350367005137715984; else cmNGEdtdkgEFgtTVbkzAlTzlPKBEbk=600172197.195329185812531007197168104513;if (cmNGEdtdkgEFgtTVbkzAlTzlPKBEbk == cmNGEdtdkgEFgtTVbkzAlTzlPKBEbk ) cmNGEdtdkgEFgtTVbkzAlTzlPKBEbk=1299376073.894275514228220361672889714732; else cmNGEdtdkgEFgtTVbkzAlTzlPKBEbk=775549036.668212946660307995884494201556;if (cmNGEdtdkgEFgtTVbkzAlTzlPKBEbk == cmNGEdtdkgEFgtTVbkzAlTzlPKBEbk ) cmNGEdtdkgEFgtTVbkzAlTzlPKBEbk=2020195742.288377112953502269393995837411; else cmNGEdtdkgEFgtTVbkzAlTzlPKBEbk=41478037.140158214199404299315336430454;double mADmGSWcYyXbJBWbvGgSpEyesERiHo=560939206.068994054925288176452419345151;if (mADmGSWcYyXbJBWbvGgSpEyesERiHo == mADmGSWcYyXbJBWbvGgSpEyesERiHo ) mADmGSWcYyXbJBWbvGgSpEyesERiHo=1715099078.109539651478306340378278065075; else mADmGSWcYyXbJBWbvGgSpEyesERiHo=1667060294.991382785052519422401598404167;if (mADmGSWcYyXbJBWbvGgSpEyesERiHo == mADmGSWcYyXbJBWbvGgSpEyesERiHo ) mADmGSWcYyXbJBWbvGgSpEyesERiHo=1091023.479658803477532677065467551408; else mADmGSWcYyXbJBWbvGgSpEyesERiHo=1722884794.963879496172914416694720301221;if (mADmGSWcYyXbJBWbvGgSpEyesERiHo == mADmGSWcYyXbJBWbvGgSpEyesERiHo ) mADmGSWcYyXbJBWbvGgSpEyesERiHo=1220877882.851561576428190466315582525368; else mADmGSWcYyXbJBWbvGgSpEyesERiHo=530462079.838643155404037673005691737005;if (mADmGSWcYyXbJBWbvGgSpEyesERiHo == mADmGSWcYyXbJBWbvGgSpEyesERiHo ) mADmGSWcYyXbJBWbvGgSpEyesERiHo=1042253390.722960155831102156595246424701; else mADmGSWcYyXbJBWbvGgSpEyesERiHo=2068218912.625178327762938469069252059664;if (mADmGSWcYyXbJBWbvGgSpEyesERiHo == mADmGSWcYyXbJBWbvGgSpEyesERiHo ) mADmGSWcYyXbJBWbvGgSpEyesERiHo=310081624.646421603494994441907585347413; else mADmGSWcYyXbJBWbvGgSpEyesERiHo=761538971.087413752786778647872976716228;if (mADmGSWcYyXbJBWbvGgSpEyesERiHo == mADmGSWcYyXbJBWbvGgSpEyesERiHo ) mADmGSWcYyXbJBWbvGgSpEyesERiHo=1883811634.439152768189922292881165111170; else mADmGSWcYyXbJBWbvGgSpEyesERiHo=931680497.754546762699080405563554747514;float iunWLbYrecSxFuMGtclGceFnJExlBw=1019568600.346248049308072056844767026047f;if (iunWLbYrecSxFuMGtclGceFnJExlBw - iunWLbYrecSxFuMGtclGceFnJExlBw> 0.00000001 ) iunWLbYrecSxFuMGtclGceFnJExlBw=793546442.039312281694092075702833879033f; else iunWLbYrecSxFuMGtclGceFnJExlBw=1694524047.301237977087525177515144621654f;if (iunWLbYrecSxFuMGtclGceFnJExlBw - iunWLbYrecSxFuMGtclGceFnJExlBw> 0.00000001 ) iunWLbYrecSxFuMGtclGceFnJExlBw=330017710.426940497610972541981262827772f; else iunWLbYrecSxFuMGtclGceFnJExlBw=1764847095.633367279579190537219187437413f;if (iunWLbYrecSxFuMGtclGceFnJExlBw - iunWLbYrecSxFuMGtclGceFnJExlBw> 0.00000001 ) iunWLbYrecSxFuMGtclGceFnJExlBw=2002760684.472608862638241227240166366604f; else iunWLbYrecSxFuMGtclGceFnJExlBw=1603968960.096721760387669195632128331735f;if (iunWLbYrecSxFuMGtclGceFnJExlBw - iunWLbYrecSxFuMGtclGceFnJExlBw> 0.00000001 ) iunWLbYrecSxFuMGtclGceFnJExlBw=428991482.327875262156806450838118207849f; else iunWLbYrecSxFuMGtclGceFnJExlBw=518661385.014507080403368643209191822067f;if (iunWLbYrecSxFuMGtclGceFnJExlBw - iunWLbYrecSxFuMGtclGceFnJExlBw> 0.00000001 ) iunWLbYrecSxFuMGtclGceFnJExlBw=1743886541.942315330048143595077475102489f; else iunWLbYrecSxFuMGtclGceFnJExlBw=1276770555.612466106342336115834700158822f;if (iunWLbYrecSxFuMGtclGceFnJExlBw - iunWLbYrecSxFuMGtclGceFnJExlBw> 0.00000001 ) iunWLbYrecSxFuMGtclGceFnJExlBw=1204585931.305254689849074614919766691332f; else iunWLbYrecSxFuMGtclGceFnJExlBw=791750624.848514728354709584764492364066f;long TWnBBCagztvfddXDhgAIleooSYgGZy=414530848;if (TWnBBCagztvfddXDhgAIleooSYgGZy == TWnBBCagztvfddXDhgAIleooSYgGZy- 1 ) TWnBBCagztvfddXDhgAIleooSYgGZy=1723829008; else TWnBBCagztvfddXDhgAIleooSYgGZy=2107262617;if (TWnBBCagztvfddXDhgAIleooSYgGZy == TWnBBCagztvfddXDhgAIleooSYgGZy- 1 ) TWnBBCagztvfddXDhgAIleooSYgGZy=155057480; else TWnBBCagztvfddXDhgAIleooSYgGZy=310357421;if (TWnBBCagztvfddXDhgAIleooSYgGZy == TWnBBCagztvfddXDhgAIleooSYgGZy- 1 ) TWnBBCagztvfddXDhgAIleooSYgGZy=1677775232; else TWnBBCagztvfddXDhgAIleooSYgGZy=1322511976;if (TWnBBCagztvfddXDhgAIleooSYgGZy == TWnBBCagztvfddXDhgAIleooSYgGZy- 1 ) TWnBBCagztvfddXDhgAIleooSYgGZy=496454723; else TWnBBCagztvfddXDhgAIleooSYgGZy=695649852;if (TWnBBCagztvfddXDhgAIleooSYgGZy == TWnBBCagztvfddXDhgAIleooSYgGZy- 0 ) TWnBBCagztvfddXDhgAIleooSYgGZy=1366003264; else TWnBBCagztvfddXDhgAIleooSYgGZy=1960761508;if (TWnBBCagztvfddXDhgAIleooSYgGZy == TWnBBCagztvfddXDhgAIleooSYgGZy- 1 ) TWnBBCagztvfddXDhgAIleooSYgGZy=1950224789; else TWnBBCagztvfddXDhgAIleooSYgGZy=76305291;double xsKuHiyNgafbIfuIZevtunxdLXJjqP=1254121897.705023945918722318729335818274;if (xsKuHiyNgafbIfuIZevtunxdLXJjqP == xsKuHiyNgafbIfuIZevtunxdLXJjqP ) xsKuHiyNgafbIfuIZevtunxdLXJjqP=98766055.927997686657478699329012148122; else xsKuHiyNgafbIfuIZevtunxdLXJjqP=1000627142.688110284584218307832995591586;if (xsKuHiyNgafbIfuIZevtunxdLXJjqP == xsKuHiyNgafbIfuIZevtunxdLXJjqP ) xsKuHiyNgafbIfuIZevtunxdLXJjqP=1395415242.066517037941685691495240079363; else xsKuHiyNgafbIfuIZevtunxdLXJjqP=420106327.971862881137090241100119283754;if (xsKuHiyNgafbIfuIZevtunxdLXJjqP == xsKuHiyNgafbIfuIZevtunxdLXJjqP ) xsKuHiyNgafbIfuIZevtunxdLXJjqP=390305380.877407183392146825194937396279; else xsKuHiyNgafbIfuIZevtunxdLXJjqP=90281980.023747060316982918347876105121;if (xsKuHiyNgafbIfuIZevtunxdLXJjqP == xsKuHiyNgafbIfuIZevtunxdLXJjqP ) xsKuHiyNgafbIfuIZevtunxdLXJjqP=705662169.191437940096589815558994335864; else xsKuHiyNgafbIfuIZevtunxdLXJjqP=111130118.395932656507845149830068331721;if (xsKuHiyNgafbIfuIZevtunxdLXJjqP == xsKuHiyNgafbIfuIZevtunxdLXJjqP ) xsKuHiyNgafbIfuIZevtunxdLXJjqP=1238068812.228780266736015216020704534860; else xsKuHiyNgafbIfuIZevtunxdLXJjqP=1629913285.293539026608411416323680262526;if (xsKuHiyNgafbIfuIZevtunxdLXJjqP == xsKuHiyNgafbIfuIZevtunxdLXJjqP ) xsKuHiyNgafbIfuIZevtunxdLXJjqP=2126481900.741664358739053249312855872447; else xsKuHiyNgafbIfuIZevtunxdLXJjqP=2091590974.751196886826860464803331499680;float wHDzppsciXbQbxcAvYUApeSXTKyjAk=972901867.483630640833128067375388447971f;if (wHDzppsciXbQbxcAvYUApeSXTKyjAk - wHDzppsciXbQbxcAvYUApeSXTKyjAk> 0.00000001 ) wHDzppsciXbQbxcAvYUApeSXTKyjAk=514013390.406444835535290169988114421530f; else wHDzppsciXbQbxcAvYUApeSXTKyjAk=1469280786.477943878423284110839919440202f;if (wHDzppsciXbQbxcAvYUApeSXTKyjAk - wHDzppsciXbQbxcAvYUApeSXTKyjAk> 0.00000001 ) wHDzppsciXbQbxcAvYUApeSXTKyjAk=1355335136.431626835849157603142180157961f; else wHDzppsciXbQbxcAvYUApeSXTKyjAk=798624022.368719267838610197395816136267f;if (wHDzppsciXbQbxcAvYUApeSXTKyjAk - wHDzppsciXbQbxcAvYUApeSXTKyjAk> 0.00000001 ) wHDzppsciXbQbxcAvYUApeSXTKyjAk=2037680343.008458244844642467017068755278f; else wHDzppsciXbQbxcAvYUApeSXTKyjAk=366425560.401184127395365138577035004180f;if (wHDzppsciXbQbxcAvYUApeSXTKyjAk - wHDzppsciXbQbxcAvYUApeSXTKyjAk> 0.00000001 ) wHDzppsciXbQbxcAvYUApeSXTKyjAk=1489277015.418655467845182381463871257618f; else wHDzppsciXbQbxcAvYUApeSXTKyjAk=1618518113.004709771088283396383372764851f;if (wHDzppsciXbQbxcAvYUApeSXTKyjAk - wHDzppsciXbQbxcAvYUApeSXTKyjAk> 0.00000001 ) wHDzppsciXbQbxcAvYUApeSXTKyjAk=1198069324.885984248461049670896233584204f; else wHDzppsciXbQbxcAvYUApeSXTKyjAk=1781851505.503130330925803334420506576341f;if (wHDzppsciXbQbxcAvYUApeSXTKyjAk - wHDzppsciXbQbxcAvYUApeSXTKyjAk> 0.00000001 ) wHDzppsciXbQbxcAvYUApeSXTKyjAk=398843831.104596902045198143135099511766f; else wHDzppsciXbQbxcAvYUApeSXTKyjAk=41999176.409992242744258641843322721618f;long LTdaRdGDezBJSvFoZEtjzrJQFqlNlJ=2119432610;if (LTdaRdGDezBJSvFoZEtjzrJQFqlNlJ == LTdaRdGDezBJSvFoZEtjzrJQFqlNlJ- 1 ) LTdaRdGDezBJSvFoZEtjzrJQFqlNlJ=1435001350; else LTdaRdGDezBJSvFoZEtjzrJQFqlNlJ=2097947681;if (LTdaRdGDezBJSvFoZEtjzrJQFqlNlJ == LTdaRdGDezBJSvFoZEtjzrJQFqlNlJ- 1 ) LTdaRdGDezBJSvFoZEtjzrJQFqlNlJ=692984569; else LTdaRdGDezBJSvFoZEtjzrJQFqlNlJ=1059097399;if (LTdaRdGDezBJSvFoZEtjzrJQFqlNlJ == LTdaRdGDezBJSvFoZEtjzrJQFqlNlJ- 0 ) LTdaRdGDezBJSvFoZEtjzrJQFqlNlJ=89248162; else LTdaRdGDezBJSvFoZEtjzrJQFqlNlJ=1974902901;if (LTdaRdGDezBJSvFoZEtjzrJQFqlNlJ == LTdaRdGDezBJSvFoZEtjzrJQFqlNlJ- 0 ) LTdaRdGDezBJSvFoZEtjzrJQFqlNlJ=24225468; else LTdaRdGDezBJSvFoZEtjzrJQFqlNlJ=1633547028;if (LTdaRdGDezBJSvFoZEtjzrJQFqlNlJ == LTdaRdGDezBJSvFoZEtjzrJQFqlNlJ- 1 ) LTdaRdGDezBJSvFoZEtjzrJQFqlNlJ=1401600448; else LTdaRdGDezBJSvFoZEtjzrJQFqlNlJ=43927362;if (LTdaRdGDezBJSvFoZEtjzrJQFqlNlJ == LTdaRdGDezBJSvFoZEtjzrJQFqlNlJ- 1 ) LTdaRdGDezBJSvFoZEtjzrJQFqlNlJ=1219772256; else LTdaRdGDezBJSvFoZEtjzrJQFqlNlJ=47861947;int mgZIhazzAJhwWAprrcqcEjxbHZTqGc=1115500498;if (mgZIhazzAJhwWAprrcqcEjxbHZTqGc == mgZIhazzAJhwWAprrcqcEjxbHZTqGc- 0 ) mgZIhazzAJhwWAprrcqcEjxbHZTqGc=1952267137; else mgZIhazzAJhwWAprrcqcEjxbHZTqGc=977322922;if (mgZIhazzAJhwWAprrcqcEjxbHZTqGc == mgZIhazzAJhwWAprrcqcEjxbHZTqGc- 0 ) mgZIhazzAJhwWAprrcqcEjxbHZTqGc=1709775548; else mgZIhazzAJhwWAprrcqcEjxbHZTqGc=206285408;if (mgZIhazzAJhwWAprrcqcEjxbHZTqGc == mgZIhazzAJhwWAprrcqcEjxbHZTqGc- 1 ) mgZIhazzAJhwWAprrcqcEjxbHZTqGc=416602442; else mgZIhazzAJhwWAprrcqcEjxbHZTqGc=1190628830;if (mgZIhazzAJhwWAprrcqcEjxbHZTqGc == mgZIhazzAJhwWAprrcqcEjxbHZTqGc- 1 ) mgZIhazzAJhwWAprrcqcEjxbHZTqGc=1363925201; else mgZIhazzAJhwWAprrcqcEjxbHZTqGc=1879824142;if (mgZIhazzAJhwWAprrcqcEjxbHZTqGc == mgZIhazzAJhwWAprrcqcEjxbHZTqGc- 0 ) mgZIhazzAJhwWAprrcqcEjxbHZTqGc=1658833774; else mgZIhazzAJhwWAprrcqcEjxbHZTqGc=1924380776;if (mgZIhazzAJhwWAprrcqcEjxbHZTqGc == mgZIhazzAJhwWAprrcqcEjxbHZTqGc- 1 ) mgZIhazzAJhwWAprrcqcEjxbHZTqGc=1369520650; else mgZIhazzAJhwWAprrcqcEjxbHZTqGc=755937522;long jAUnNFlMMVPVSGpXbvjXyCUxxXCFeg=885382621;if (jAUnNFlMMVPVSGpXbvjXyCUxxXCFeg == jAUnNFlMMVPVSGpXbvjXyCUxxXCFeg- 1 ) jAUnNFlMMVPVSGpXbvjXyCUxxXCFeg=639334447; else jAUnNFlMMVPVSGpXbvjXyCUxxXCFeg=7049290;if (jAUnNFlMMVPVSGpXbvjXyCUxxXCFeg == jAUnNFlMMVPVSGpXbvjXyCUxxXCFeg- 0 ) jAUnNFlMMVPVSGpXbvjXyCUxxXCFeg=656589624; else jAUnNFlMMVPVSGpXbvjXyCUxxXCFeg=1522895761;if (jAUnNFlMMVPVSGpXbvjXyCUxxXCFeg == jAUnNFlMMVPVSGpXbvjXyCUxxXCFeg- 1 ) jAUnNFlMMVPVSGpXbvjXyCUxxXCFeg=1705001844; else jAUnNFlMMVPVSGpXbvjXyCUxxXCFeg=1096880619;if (jAUnNFlMMVPVSGpXbvjXyCUxxXCFeg == jAUnNFlMMVPVSGpXbvjXyCUxxXCFeg- 0 ) jAUnNFlMMVPVSGpXbvjXyCUxxXCFeg=883653705; else jAUnNFlMMVPVSGpXbvjXyCUxxXCFeg=686594136;if (jAUnNFlMMVPVSGpXbvjXyCUxxXCFeg == jAUnNFlMMVPVSGpXbvjXyCUxxXCFeg- 0 ) jAUnNFlMMVPVSGpXbvjXyCUxxXCFeg=1445055318; else jAUnNFlMMVPVSGpXbvjXyCUxxXCFeg=1869271755;if (jAUnNFlMMVPVSGpXbvjXyCUxxXCFeg == jAUnNFlMMVPVSGpXbvjXyCUxxXCFeg- 0 ) jAUnNFlMMVPVSGpXbvjXyCUxxXCFeg=1544507221; else jAUnNFlMMVPVSGpXbvjXyCUxxXCFeg=1993752981;int uPhkkCQUXJYkfOiwHapHOKPjKLrElp=645721928;if (uPhkkCQUXJYkfOiwHapHOKPjKLrElp == uPhkkCQUXJYkfOiwHapHOKPjKLrElp- 0 ) uPhkkCQUXJYkfOiwHapHOKPjKLrElp=412077801; else uPhkkCQUXJYkfOiwHapHOKPjKLrElp=1430826980;if (uPhkkCQUXJYkfOiwHapHOKPjKLrElp == uPhkkCQUXJYkfOiwHapHOKPjKLrElp- 0 ) uPhkkCQUXJYkfOiwHapHOKPjKLrElp=237215686; else uPhkkCQUXJYkfOiwHapHOKPjKLrElp=823904896;if (uPhkkCQUXJYkfOiwHapHOKPjKLrElp == uPhkkCQUXJYkfOiwHapHOKPjKLrElp- 0 ) uPhkkCQUXJYkfOiwHapHOKPjKLrElp=790394290; else uPhkkCQUXJYkfOiwHapHOKPjKLrElp=784293430;if (uPhkkCQUXJYkfOiwHapHOKPjKLrElp == uPhkkCQUXJYkfOiwHapHOKPjKLrElp- 1 ) uPhkkCQUXJYkfOiwHapHOKPjKLrElp=1702830168; else uPhkkCQUXJYkfOiwHapHOKPjKLrElp=244372520;if (uPhkkCQUXJYkfOiwHapHOKPjKLrElp == uPhkkCQUXJYkfOiwHapHOKPjKLrElp- 0 ) uPhkkCQUXJYkfOiwHapHOKPjKLrElp=734022480; else uPhkkCQUXJYkfOiwHapHOKPjKLrElp=960644074;if (uPhkkCQUXJYkfOiwHapHOKPjKLrElp == uPhkkCQUXJYkfOiwHapHOKPjKLrElp- 1 ) uPhkkCQUXJYkfOiwHapHOKPjKLrElp=1547747272; else uPhkkCQUXJYkfOiwHapHOKPjKLrElp=312351033;long emUJZgvJDJnFOrwFqKDKQaZDwyfZHd=737937455;if (emUJZgvJDJnFOrwFqKDKQaZDwyfZHd == emUJZgvJDJnFOrwFqKDKQaZDwyfZHd- 0 ) emUJZgvJDJnFOrwFqKDKQaZDwyfZHd=1734025062; else emUJZgvJDJnFOrwFqKDKQaZDwyfZHd=334389635;if (emUJZgvJDJnFOrwFqKDKQaZDwyfZHd == emUJZgvJDJnFOrwFqKDKQaZDwyfZHd- 1 ) emUJZgvJDJnFOrwFqKDKQaZDwyfZHd=1556011147; else emUJZgvJDJnFOrwFqKDKQaZDwyfZHd=1336464433;if (emUJZgvJDJnFOrwFqKDKQaZDwyfZHd == emUJZgvJDJnFOrwFqKDKQaZDwyfZHd- 0 ) emUJZgvJDJnFOrwFqKDKQaZDwyfZHd=2043241419; else emUJZgvJDJnFOrwFqKDKQaZDwyfZHd=1994361590;if (emUJZgvJDJnFOrwFqKDKQaZDwyfZHd == emUJZgvJDJnFOrwFqKDKQaZDwyfZHd- 1 ) emUJZgvJDJnFOrwFqKDKQaZDwyfZHd=1557982301; else emUJZgvJDJnFOrwFqKDKQaZDwyfZHd=855386518;if (emUJZgvJDJnFOrwFqKDKQaZDwyfZHd == emUJZgvJDJnFOrwFqKDKQaZDwyfZHd- 1 ) emUJZgvJDJnFOrwFqKDKQaZDwyfZHd=93669217; else emUJZgvJDJnFOrwFqKDKQaZDwyfZHd=1023181412;if (emUJZgvJDJnFOrwFqKDKQaZDwyfZHd == emUJZgvJDJnFOrwFqKDKQaZDwyfZHd- 1 ) emUJZgvJDJnFOrwFqKDKQaZDwyfZHd=908713737; else emUJZgvJDJnFOrwFqKDKQaZDwyfZHd=694814334;long OlMzlqAedfrmGwjtLtnOjaPZGAreXD=217418706;if (OlMzlqAedfrmGwjtLtnOjaPZGAreXD == OlMzlqAedfrmGwjtLtnOjaPZGAreXD- 1 ) OlMzlqAedfrmGwjtLtnOjaPZGAreXD=1649376386; else OlMzlqAedfrmGwjtLtnOjaPZGAreXD=723798722;if (OlMzlqAedfrmGwjtLtnOjaPZGAreXD == OlMzlqAedfrmGwjtLtnOjaPZGAreXD- 1 ) OlMzlqAedfrmGwjtLtnOjaPZGAreXD=343859721; else OlMzlqAedfrmGwjtLtnOjaPZGAreXD=408094816;if (OlMzlqAedfrmGwjtLtnOjaPZGAreXD == OlMzlqAedfrmGwjtLtnOjaPZGAreXD- 0 ) OlMzlqAedfrmGwjtLtnOjaPZGAreXD=426447307; else OlMzlqAedfrmGwjtLtnOjaPZGAreXD=1780964698;if (OlMzlqAedfrmGwjtLtnOjaPZGAreXD == OlMzlqAedfrmGwjtLtnOjaPZGAreXD- 1 ) OlMzlqAedfrmGwjtLtnOjaPZGAreXD=94988331; else OlMzlqAedfrmGwjtLtnOjaPZGAreXD=2002524664;if (OlMzlqAedfrmGwjtLtnOjaPZGAreXD == OlMzlqAedfrmGwjtLtnOjaPZGAreXD- 0 ) OlMzlqAedfrmGwjtLtnOjaPZGAreXD=451702769; else OlMzlqAedfrmGwjtLtnOjaPZGAreXD=412217104;if (OlMzlqAedfrmGwjtLtnOjaPZGAreXD == OlMzlqAedfrmGwjtLtnOjaPZGAreXD- 1 ) OlMzlqAedfrmGwjtLtnOjaPZGAreXD=788873281; else OlMzlqAedfrmGwjtLtnOjaPZGAreXD=744357770;int vCVdMHixBsYKOSGMSbSKccFbBvWUCk=29283725;if (vCVdMHixBsYKOSGMSbSKccFbBvWUCk == vCVdMHixBsYKOSGMSbSKccFbBvWUCk- 0 ) vCVdMHixBsYKOSGMSbSKccFbBvWUCk=2074646523; else vCVdMHixBsYKOSGMSbSKccFbBvWUCk=806569766;if (vCVdMHixBsYKOSGMSbSKccFbBvWUCk == vCVdMHixBsYKOSGMSbSKccFbBvWUCk- 1 ) vCVdMHixBsYKOSGMSbSKccFbBvWUCk=259727919; else vCVdMHixBsYKOSGMSbSKccFbBvWUCk=959747316;if (vCVdMHixBsYKOSGMSbSKccFbBvWUCk == vCVdMHixBsYKOSGMSbSKccFbBvWUCk- 0 ) vCVdMHixBsYKOSGMSbSKccFbBvWUCk=2032887568; else vCVdMHixBsYKOSGMSbSKccFbBvWUCk=1086023604;if (vCVdMHixBsYKOSGMSbSKccFbBvWUCk == vCVdMHixBsYKOSGMSbSKccFbBvWUCk- 1 ) vCVdMHixBsYKOSGMSbSKccFbBvWUCk=749964156; else vCVdMHixBsYKOSGMSbSKccFbBvWUCk=565199858;if (vCVdMHixBsYKOSGMSbSKccFbBvWUCk == vCVdMHixBsYKOSGMSbSKccFbBvWUCk- 0 ) vCVdMHixBsYKOSGMSbSKccFbBvWUCk=2020328326; else vCVdMHixBsYKOSGMSbSKccFbBvWUCk=1529312815;if (vCVdMHixBsYKOSGMSbSKccFbBvWUCk == vCVdMHixBsYKOSGMSbSKccFbBvWUCk- 1 ) vCVdMHixBsYKOSGMSbSKccFbBvWUCk=730235679; else vCVdMHixBsYKOSGMSbSKccFbBvWUCk=1914696525;long rVIzgNkjOSXvCMdDveLTSfxyANQCjC=1363850260;if (rVIzgNkjOSXvCMdDveLTSfxyANQCjC == rVIzgNkjOSXvCMdDveLTSfxyANQCjC- 1 ) rVIzgNkjOSXvCMdDveLTSfxyANQCjC=374956883; else rVIzgNkjOSXvCMdDveLTSfxyANQCjC=1330328566;if (rVIzgNkjOSXvCMdDveLTSfxyANQCjC == rVIzgNkjOSXvCMdDveLTSfxyANQCjC- 0 ) rVIzgNkjOSXvCMdDveLTSfxyANQCjC=810458294; else rVIzgNkjOSXvCMdDveLTSfxyANQCjC=2029067176;if (rVIzgNkjOSXvCMdDveLTSfxyANQCjC == rVIzgNkjOSXvCMdDveLTSfxyANQCjC- 0 ) rVIzgNkjOSXvCMdDveLTSfxyANQCjC=1071911872; else rVIzgNkjOSXvCMdDveLTSfxyANQCjC=1534014063;if (rVIzgNkjOSXvCMdDveLTSfxyANQCjC == rVIzgNkjOSXvCMdDveLTSfxyANQCjC- 1 ) rVIzgNkjOSXvCMdDveLTSfxyANQCjC=311490148; else rVIzgNkjOSXvCMdDveLTSfxyANQCjC=2100544011;if (rVIzgNkjOSXvCMdDveLTSfxyANQCjC == rVIzgNkjOSXvCMdDveLTSfxyANQCjC- 0 ) rVIzgNkjOSXvCMdDveLTSfxyANQCjC=239401304; else rVIzgNkjOSXvCMdDveLTSfxyANQCjC=1700970130;if (rVIzgNkjOSXvCMdDveLTSfxyANQCjC == rVIzgNkjOSXvCMdDveLTSfxyANQCjC- 1 ) rVIzgNkjOSXvCMdDveLTSfxyANQCjC=884761664; else rVIzgNkjOSXvCMdDveLTSfxyANQCjC=1631024315;int ikXDTSvGujfarDpLBNKfgHSjpQOnlx=1669850192;if (ikXDTSvGujfarDpLBNKfgHSjpQOnlx == ikXDTSvGujfarDpLBNKfgHSjpQOnlx- 1 ) ikXDTSvGujfarDpLBNKfgHSjpQOnlx=941969527; else ikXDTSvGujfarDpLBNKfgHSjpQOnlx=879430012;if (ikXDTSvGujfarDpLBNKfgHSjpQOnlx == ikXDTSvGujfarDpLBNKfgHSjpQOnlx- 1 ) ikXDTSvGujfarDpLBNKfgHSjpQOnlx=767651458; else ikXDTSvGujfarDpLBNKfgHSjpQOnlx=987035490;if (ikXDTSvGujfarDpLBNKfgHSjpQOnlx == ikXDTSvGujfarDpLBNKfgHSjpQOnlx- 1 ) ikXDTSvGujfarDpLBNKfgHSjpQOnlx=1257927948; else ikXDTSvGujfarDpLBNKfgHSjpQOnlx=1773423522;if (ikXDTSvGujfarDpLBNKfgHSjpQOnlx == ikXDTSvGujfarDpLBNKfgHSjpQOnlx- 0 ) ikXDTSvGujfarDpLBNKfgHSjpQOnlx=332014785; else ikXDTSvGujfarDpLBNKfgHSjpQOnlx=563352782;if (ikXDTSvGujfarDpLBNKfgHSjpQOnlx == ikXDTSvGujfarDpLBNKfgHSjpQOnlx- 0 ) ikXDTSvGujfarDpLBNKfgHSjpQOnlx=2121318213; else ikXDTSvGujfarDpLBNKfgHSjpQOnlx=1926143510;if (ikXDTSvGujfarDpLBNKfgHSjpQOnlx == ikXDTSvGujfarDpLBNKfgHSjpQOnlx- 1 ) ikXDTSvGujfarDpLBNKfgHSjpQOnlx=535250137; else ikXDTSvGujfarDpLBNKfgHSjpQOnlx=1044618139;float tZrRqMUhmfaxjKHEkevxlnlTEKHDNG=1591170582.692132119765631969582689193136f;if (tZrRqMUhmfaxjKHEkevxlnlTEKHDNG - tZrRqMUhmfaxjKHEkevxlnlTEKHDNG> 0.00000001 ) tZrRqMUhmfaxjKHEkevxlnlTEKHDNG=1961449840.536007629309374105217909312415f; else tZrRqMUhmfaxjKHEkevxlnlTEKHDNG=1572506668.019433963435039331509892064779f;if (tZrRqMUhmfaxjKHEkevxlnlTEKHDNG - tZrRqMUhmfaxjKHEkevxlnlTEKHDNG> 0.00000001 ) tZrRqMUhmfaxjKHEkevxlnlTEKHDNG=369400887.632530095224298298108201623757f; else tZrRqMUhmfaxjKHEkevxlnlTEKHDNG=366288428.030591461945639570487667153191f;if (tZrRqMUhmfaxjKHEkevxlnlTEKHDNG - tZrRqMUhmfaxjKHEkevxlnlTEKHDNG> 0.00000001 ) tZrRqMUhmfaxjKHEkevxlnlTEKHDNG=1908328025.835930581794727467326348047344f; else tZrRqMUhmfaxjKHEkevxlnlTEKHDNG=199628356.938021080900079495170697650800f;if (tZrRqMUhmfaxjKHEkevxlnlTEKHDNG - tZrRqMUhmfaxjKHEkevxlnlTEKHDNG> 0.00000001 ) tZrRqMUhmfaxjKHEkevxlnlTEKHDNG=1104855500.334796681759742637965338535147f; else tZrRqMUhmfaxjKHEkevxlnlTEKHDNG=540632460.239178738392553105613394611737f;if (tZrRqMUhmfaxjKHEkevxlnlTEKHDNG - tZrRqMUhmfaxjKHEkevxlnlTEKHDNG> 0.00000001 ) tZrRqMUhmfaxjKHEkevxlnlTEKHDNG=495079618.255452178501968399544164594556f; else tZrRqMUhmfaxjKHEkevxlnlTEKHDNG=306667555.811224810113821254733329420197f;if (tZrRqMUhmfaxjKHEkevxlnlTEKHDNG - tZrRqMUhmfaxjKHEkevxlnlTEKHDNG> 0.00000001 ) tZrRqMUhmfaxjKHEkevxlnlTEKHDNG=720346012.962270085468953135434192450281f; else tZrRqMUhmfaxjKHEkevxlnlTEKHDNG=331917438.573336165238048547639586388083f;float pxRstafInBmyKDjHaXuGLebNywizap=1206703795.084899318480446190988530826118f;if (pxRstafInBmyKDjHaXuGLebNywizap - pxRstafInBmyKDjHaXuGLebNywizap> 0.00000001 ) pxRstafInBmyKDjHaXuGLebNywizap=223059644.336092554054716504013662966418f; else pxRstafInBmyKDjHaXuGLebNywizap=1207869134.321115555474603582433320578414f;if (pxRstafInBmyKDjHaXuGLebNywizap - pxRstafInBmyKDjHaXuGLebNywizap> 0.00000001 ) pxRstafInBmyKDjHaXuGLebNywizap=972726998.724774370215065917058408510264f; else pxRstafInBmyKDjHaXuGLebNywizap=1143633328.815185093022233837677156742727f;if (pxRstafInBmyKDjHaXuGLebNywizap - pxRstafInBmyKDjHaXuGLebNywizap> 0.00000001 ) pxRstafInBmyKDjHaXuGLebNywizap=240364376.421751199092404628678470876955f; else pxRstafInBmyKDjHaXuGLebNywizap=1758699068.447771268285868586312515906199f;if (pxRstafInBmyKDjHaXuGLebNywizap - pxRstafInBmyKDjHaXuGLebNywizap> 0.00000001 ) pxRstafInBmyKDjHaXuGLebNywizap=105922312.524575584356040461330175394287f; else pxRstafInBmyKDjHaXuGLebNywizap=749017008.617086149300306667664596056029f;if (pxRstafInBmyKDjHaXuGLebNywizap - pxRstafInBmyKDjHaXuGLebNywizap> 0.00000001 ) pxRstafInBmyKDjHaXuGLebNywizap=998313781.231785907443304097074773322213f; else pxRstafInBmyKDjHaXuGLebNywizap=247105512.012118963186122239307016657957f;if (pxRstafInBmyKDjHaXuGLebNywizap - pxRstafInBmyKDjHaXuGLebNywizap> 0.00000001 ) pxRstafInBmyKDjHaXuGLebNywizap=2055232610.380443229723490876120391710667f; else pxRstafInBmyKDjHaXuGLebNywizap=1293252009.102780778901179473689293819984f;double dfNbIImDyCCmIKsNKOOWoAnoyHrtoD=2059109498.774832347303914103315721349965;if (dfNbIImDyCCmIKsNKOOWoAnoyHrtoD == dfNbIImDyCCmIKsNKOOWoAnoyHrtoD ) dfNbIImDyCCmIKsNKOOWoAnoyHrtoD=1721508806.940392326308780863398371605450; else dfNbIImDyCCmIKsNKOOWoAnoyHrtoD=1441074729.359346362388801705168777878980;if (dfNbIImDyCCmIKsNKOOWoAnoyHrtoD == dfNbIImDyCCmIKsNKOOWoAnoyHrtoD ) dfNbIImDyCCmIKsNKOOWoAnoyHrtoD=1817058122.797706055551111057000140278257; else dfNbIImDyCCmIKsNKOOWoAnoyHrtoD=1780465288.959360220676162867693198076864;if (dfNbIImDyCCmIKsNKOOWoAnoyHrtoD == dfNbIImDyCCmIKsNKOOWoAnoyHrtoD ) dfNbIImDyCCmIKsNKOOWoAnoyHrtoD=444223644.881871295284753508841267486149; else dfNbIImDyCCmIKsNKOOWoAnoyHrtoD=939964743.557789530178251955657802372449;if (dfNbIImDyCCmIKsNKOOWoAnoyHrtoD == dfNbIImDyCCmIKsNKOOWoAnoyHrtoD ) dfNbIImDyCCmIKsNKOOWoAnoyHrtoD=1909005814.625684808407888402753269375551; else dfNbIImDyCCmIKsNKOOWoAnoyHrtoD=742341171.719165368725825012507180756741;if (dfNbIImDyCCmIKsNKOOWoAnoyHrtoD == dfNbIImDyCCmIKsNKOOWoAnoyHrtoD ) dfNbIImDyCCmIKsNKOOWoAnoyHrtoD=1922058159.036779972315504954437315574052; else dfNbIImDyCCmIKsNKOOWoAnoyHrtoD=227018998.972789015624020379681332573121;if (dfNbIImDyCCmIKsNKOOWoAnoyHrtoD == dfNbIImDyCCmIKsNKOOWoAnoyHrtoD ) dfNbIImDyCCmIKsNKOOWoAnoyHrtoD=491955872.693145383504991339631557274121; else dfNbIImDyCCmIKsNKOOWoAnoyHrtoD=951260393.799504108609619745134460115607;long LhKtCWVqnKRECOHwQpWPvywjLRwVuu=854894725;if (LhKtCWVqnKRECOHwQpWPvywjLRwVuu == LhKtCWVqnKRECOHwQpWPvywjLRwVuu- 0 ) LhKtCWVqnKRECOHwQpWPvywjLRwVuu=418948314; else LhKtCWVqnKRECOHwQpWPvywjLRwVuu=1476029989;if (LhKtCWVqnKRECOHwQpWPvywjLRwVuu == LhKtCWVqnKRECOHwQpWPvywjLRwVuu- 0 ) LhKtCWVqnKRECOHwQpWPvywjLRwVuu=821558495; else LhKtCWVqnKRECOHwQpWPvywjLRwVuu=1990343532;if (LhKtCWVqnKRECOHwQpWPvywjLRwVuu == LhKtCWVqnKRECOHwQpWPvywjLRwVuu- 0 ) LhKtCWVqnKRECOHwQpWPvywjLRwVuu=1502532102; else LhKtCWVqnKRECOHwQpWPvywjLRwVuu=983311643;if (LhKtCWVqnKRECOHwQpWPvywjLRwVuu == LhKtCWVqnKRECOHwQpWPvywjLRwVuu- 0 ) LhKtCWVqnKRECOHwQpWPvywjLRwVuu=1350806206; else LhKtCWVqnKRECOHwQpWPvywjLRwVuu=202750772;if (LhKtCWVqnKRECOHwQpWPvywjLRwVuu == LhKtCWVqnKRECOHwQpWPvywjLRwVuu- 1 ) LhKtCWVqnKRECOHwQpWPvywjLRwVuu=1466584356; else LhKtCWVqnKRECOHwQpWPvywjLRwVuu=1767867526;if (LhKtCWVqnKRECOHwQpWPvywjLRwVuu == LhKtCWVqnKRECOHwQpWPvywjLRwVuu- 0 ) LhKtCWVqnKRECOHwQpWPvywjLRwVuu=1515458294; else LhKtCWVqnKRECOHwQpWPvywjLRwVuu=1661222513;long ZCiVTeZAgxYtrsDqPApMZHkibBvkFm=1228207609;if (ZCiVTeZAgxYtrsDqPApMZHkibBvkFm == ZCiVTeZAgxYtrsDqPApMZHkibBvkFm- 0 ) ZCiVTeZAgxYtrsDqPApMZHkibBvkFm=1404336210; else ZCiVTeZAgxYtrsDqPApMZHkibBvkFm=808720161;if (ZCiVTeZAgxYtrsDqPApMZHkibBvkFm == ZCiVTeZAgxYtrsDqPApMZHkibBvkFm- 0 ) ZCiVTeZAgxYtrsDqPApMZHkibBvkFm=757892279; else ZCiVTeZAgxYtrsDqPApMZHkibBvkFm=907670858;if (ZCiVTeZAgxYtrsDqPApMZHkibBvkFm == ZCiVTeZAgxYtrsDqPApMZHkibBvkFm- 0 ) ZCiVTeZAgxYtrsDqPApMZHkibBvkFm=1596849013; else ZCiVTeZAgxYtrsDqPApMZHkibBvkFm=1874213555;if (ZCiVTeZAgxYtrsDqPApMZHkibBvkFm == ZCiVTeZAgxYtrsDqPApMZHkibBvkFm- 1 ) ZCiVTeZAgxYtrsDqPApMZHkibBvkFm=762480151; else ZCiVTeZAgxYtrsDqPApMZHkibBvkFm=1430578548;if (ZCiVTeZAgxYtrsDqPApMZHkibBvkFm == ZCiVTeZAgxYtrsDqPApMZHkibBvkFm- 1 ) ZCiVTeZAgxYtrsDqPApMZHkibBvkFm=1446346967; else ZCiVTeZAgxYtrsDqPApMZHkibBvkFm=230385827;if (ZCiVTeZAgxYtrsDqPApMZHkibBvkFm == ZCiVTeZAgxYtrsDqPApMZHkibBvkFm- 0 ) ZCiVTeZAgxYtrsDqPApMZHkibBvkFm=1369058816; else ZCiVTeZAgxYtrsDqPApMZHkibBvkFm=13345378;double HidjGfdsEyOJGUfgidynJvKenzKtad=998216434.602504792193469458107821340213;if (HidjGfdsEyOJGUfgidynJvKenzKtad == HidjGfdsEyOJGUfgidynJvKenzKtad ) HidjGfdsEyOJGUfgidynJvKenzKtad=2032498317.213082377581854611985146498126; else HidjGfdsEyOJGUfgidynJvKenzKtad=1200337885.469909148829273608883033917870;if (HidjGfdsEyOJGUfgidynJvKenzKtad == HidjGfdsEyOJGUfgidynJvKenzKtad ) HidjGfdsEyOJGUfgidynJvKenzKtad=874303695.425113295558326697489145353700; else HidjGfdsEyOJGUfgidynJvKenzKtad=1670217584.248941416041333672832059149187;if (HidjGfdsEyOJGUfgidynJvKenzKtad == HidjGfdsEyOJGUfgidynJvKenzKtad ) HidjGfdsEyOJGUfgidynJvKenzKtad=592797161.060632159639244882288107830980; else HidjGfdsEyOJGUfgidynJvKenzKtad=1206955107.343019280263421492636701007391;if (HidjGfdsEyOJGUfgidynJvKenzKtad == HidjGfdsEyOJGUfgidynJvKenzKtad ) HidjGfdsEyOJGUfgidynJvKenzKtad=556577396.327930623671203588744946564301; else HidjGfdsEyOJGUfgidynJvKenzKtad=738197163.290904769297906550853075859158;if (HidjGfdsEyOJGUfgidynJvKenzKtad == HidjGfdsEyOJGUfgidynJvKenzKtad ) HidjGfdsEyOJGUfgidynJvKenzKtad=90268523.633348241083956714001555933516; else HidjGfdsEyOJGUfgidynJvKenzKtad=1614307350.661276567942998112025625718841;if (HidjGfdsEyOJGUfgidynJvKenzKtad == HidjGfdsEyOJGUfgidynJvKenzKtad ) HidjGfdsEyOJGUfgidynJvKenzKtad=313880932.573643279025727067893260215756; else HidjGfdsEyOJGUfgidynJvKenzKtad=823839765.882647945504300090985206942354;long ZJRJTLFoyBOHPurrcBGJYomDfsfnxY=2110454668;if (ZJRJTLFoyBOHPurrcBGJYomDfsfnxY == ZJRJTLFoyBOHPurrcBGJYomDfsfnxY- 0 ) ZJRJTLFoyBOHPurrcBGJYomDfsfnxY=685150464; else ZJRJTLFoyBOHPurrcBGJYomDfsfnxY=1411231910;if (ZJRJTLFoyBOHPurrcBGJYomDfsfnxY == ZJRJTLFoyBOHPurrcBGJYomDfsfnxY- 1 ) ZJRJTLFoyBOHPurrcBGJYomDfsfnxY=1694407830; else ZJRJTLFoyBOHPurrcBGJYomDfsfnxY=244357349;if (ZJRJTLFoyBOHPurrcBGJYomDfsfnxY == ZJRJTLFoyBOHPurrcBGJYomDfsfnxY- 0 ) ZJRJTLFoyBOHPurrcBGJYomDfsfnxY=1389688715; else ZJRJTLFoyBOHPurrcBGJYomDfsfnxY=1601903631;if (ZJRJTLFoyBOHPurrcBGJYomDfsfnxY == ZJRJTLFoyBOHPurrcBGJYomDfsfnxY- 1 ) ZJRJTLFoyBOHPurrcBGJYomDfsfnxY=1405529359; else ZJRJTLFoyBOHPurrcBGJYomDfsfnxY=692218406;if (ZJRJTLFoyBOHPurrcBGJYomDfsfnxY == ZJRJTLFoyBOHPurrcBGJYomDfsfnxY- 1 ) ZJRJTLFoyBOHPurrcBGJYomDfsfnxY=713549838; else ZJRJTLFoyBOHPurrcBGJYomDfsfnxY=1538463594;if (ZJRJTLFoyBOHPurrcBGJYomDfsfnxY == ZJRJTLFoyBOHPurrcBGJYomDfsfnxY- 0 ) ZJRJTLFoyBOHPurrcBGJYomDfsfnxY=543996565; else ZJRJTLFoyBOHPurrcBGJYomDfsfnxY=1272146275;int juocXcRGyojQhISsswNLcTNbDpWKsM=164438790;if (juocXcRGyojQhISsswNLcTNbDpWKsM == juocXcRGyojQhISsswNLcTNbDpWKsM- 0 ) juocXcRGyojQhISsswNLcTNbDpWKsM=2071928028; else juocXcRGyojQhISsswNLcTNbDpWKsM=517241860;if (juocXcRGyojQhISsswNLcTNbDpWKsM == juocXcRGyojQhISsswNLcTNbDpWKsM- 0 ) juocXcRGyojQhISsswNLcTNbDpWKsM=1776207843; else juocXcRGyojQhISsswNLcTNbDpWKsM=1239025155;if (juocXcRGyojQhISsswNLcTNbDpWKsM == juocXcRGyojQhISsswNLcTNbDpWKsM- 1 ) juocXcRGyojQhISsswNLcTNbDpWKsM=1084783443; else juocXcRGyojQhISsswNLcTNbDpWKsM=19134597;if (juocXcRGyojQhISsswNLcTNbDpWKsM == juocXcRGyojQhISsswNLcTNbDpWKsM- 0 ) juocXcRGyojQhISsswNLcTNbDpWKsM=635410448; else juocXcRGyojQhISsswNLcTNbDpWKsM=197381103;if (juocXcRGyojQhISsswNLcTNbDpWKsM == juocXcRGyojQhISsswNLcTNbDpWKsM- 1 ) juocXcRGyojQhISsswNLcTNbDpWKsM=252142765; else juocXcRGyojQhISsswNLcTNbDpWKsM=19695116;if (juocXcRGyojQhISsswNLcTNbDpWKsM == juocXcRGyojQhISsswNLcTNbDpWKsM- 0 ) juocXcRGyojQhISsswNLcTNbDpWKsM=817402335; else juocXcRGyojQhISsswNLcTNbDpWKsM=2130025310;float KtVqfpDaTfbxOHbQdfeMVFWqTSXgHh=1198563666.087801273933473591293992645124f;if (KtVqfpDaTfbxOHbQdfeMVFWqTSXgHh - KtVqfpDaTfbxOHbQdfeMVFWqTSXgHh> 0.00000001 ) KtVqfpDaTfbxOHbQdfeMVFWqTSXgHh=1927407696.987703058900184557232759910537f; else KtVqfpDaTfbxOHbQdfeMVFWqTSXgHh=1406087795.725016687154349381919999224999f;if (KtVqfpDaTfbxOHbQdfeMVFWqTSXgHh - KtVqfpDaTfbxOHbQdfeMVFWqTSXgHh> 0.00000001 ) KtVqfpDaTfbxOHbQdfeMVFWqTSXgHh=1475678561.452239528998028941063973893789f; else KtVqfpDaTfbxOHbQdfeMVFWqTSXgHh=2002994933.379871321197313298248303659055f;if (KtVqfpDaTfbxOHbQdfeMVFWqTSXgHh - KtVqfpDaTfbxOHbQdfeMVFWqTSXgHh> 0.00000001 ) KtVqfpDaTfbxOHbQdfeMVFWqTSXgHh=1755221383.694963935873198776070655207723f; else KtVqfpDaTfbxOHbQdfeMVFWqTSXgHh=1599065796.237750355037448531609309514087f;if (KtVqfpDaTfbxOHbQdfeMVFWqTSXgHh - KtVqfpDaTfbxOHbQdfeMVFWqTSXgHh> 0.00000001 ) KtVqfpDaTfbxOHbQdfeMVFWqTSXgHh=1756011366.631264745852214431878713504935f; else KtVqfpDaTfbxOHbQdfeMVFWqTSXgHh=430594686.748950729499785720519321902887f;if (KtVqfpDaTfbxOHbQdfeMVFWqTSXgHh - KtVqfpDaTfbxOHbQdfeMVFWqTSXgHh> 0.00000001 ) KtVqfpDaTfbxOHbQdfeMVFWqTSXgHh=1942292173.192021546581348903899542642706f; else KtVqfpDaTfbxOHbQdfeMVFWqTSXgHh=182085289.994241877528414819400900975059f;if (KtVqfpDaTfbxOHbQdfeMVFWqTSXgHh - KtVqfpDaTfbxOHbQdfeMVFWqTSXgHh> 0.00000001 ) KtVqfpDaTfbxOHbQdfeMVFWqTSXgHh=956667746.094169820823747114884440600538f; else KtVqfpDaTfbxOHbQdfeMVFWqTSXgHh=1201817214.783954535619979836496237676567f;float RtKESAqwPfptNmXYkDnnQEWMZbdPqr=900291342.210152667365001838700487829412f;if (RtKESAqwPfptNmXYkDnnQEWMZbdPqr - RtKESAqwPfptNmXYkDnnQEWMZbdPqr> 0.00000001 ) RtKESAqwPfptNmXYkDnnQEWMZbdPqr=530816127.616701110782359891303026094433f; else RtKESAqwPfptNmXYkDnnQEWMZbdPqr=149442142.426675031172812913813821960072f;if (RtKESAqwPfptNmXYkDnnQEWMZbdPqr - RtKESAqwPfptNmXYkDnnQEWMZbdPqr> 0.00000001 ) RtKESAqwPfptNmXYkDnnQEWMZbdPqr=899395384.789847778597124232428163061571f; else RtKESAqwPfptNmXYkDnnQEWMZbdPqr=1054748236.606949418624642780784876473688f;if (RtKESAqwPfptNmXYkDnnQEWMZbdPqr - RtKESAqwPfptNmXYkDnnQEWMZbdPqr> 0.00000001 ) RtKESAqwPfptNmXYkDnnQEWMZbdPqr=619059105.720709129582591462149180784168f; else RtKESAqwPfptNmXYkDnnQEWMZbdPqr=1358920270.784000495477230349512154064102f;if (RtKESAqwPfptNmXYkDnnQEWMZbdPqr - RtKESAqwPfptNmXYkDnnQEWMZbdPqr> 0.00000001 ) RtKESAqwPfptNmXYkDnnQEWMZbdPqr=1025671225.607191850107888206054666764015f; else RtKESAqwPfptNmXYkDnnQEWMZbdPqr=666015867.649387115050051547786916915153f;if (RtKESAqwPfptNmXYkDnnQEWMZbdPqr - RtKESAqwPfptNmXYkDnnQEWMZbdPqr> 0.00000001 ) RtKESAqwPfptNmXYkDnnQEWMZbdPqr=775821462.884567851120210472523744983448f; else RtKESAqwPfptNmXYkDnnQEWMZbdPqr=1497026727.206227759479766781655374115069f;if (RtKESAqwPfptNmXYkDnnQEWMZbdPqr - RtKESAqwPfptNmXYkDnnQEWMZbdPqr> 0.00000001 ) RtKESAqwPfptNmXYkDnnQEWMZbdPqr=2139698231.452303409382325418830380404321f; else RtKESAqwPfptNmXYkDnnQEWMZbdPqr=1369993599.279102847438237306971834928200f;long FsoFHSJGPgjEWALCXLGxpTVAYVbOpW=774909452;if (FsoFHSJGPgjEWALCXLGxpTVAYVbOpW == FsoFHSJGPgjEWALCXLGxpTVAYVbOpW- 1 ) FsoFHSJGPgjEWALCXLGxpTVAYVbOpW=1679875142; else FsoFHSJGPgjEWALCXLGxpTVAYVbOpW=1492916546;if (FsoFHSJGPgjEWALCXLGxpTVAYVbOpW == FsoFHSJGPgjEWALCXLGxpTVAYVbOpW- 1 ) FsoFHSJGPgjEWALCXLGxpTVAYVbOpW=1492222226; else FsoFHSJGPgjEWALCXLGxpTVAYVbOpW=355648679;if (FsoFHSJGPgjEWALCXLGxpTVAYVbOpW == FsoFHSJGPgjEWALCXLGxpTVAYVbOpW- 1 ) FsoFHSJGPgjEWALCXLGxpTVAYVbOpW=2009265914; else FsoFHSJGPgjEWALCXLGxpTVAYVbOpW=1227979937;if (FsoFHSJGPgjEWALCXLGxpTVAYVbOpW == FsoFHSJGPgjEWALCXLGxpTVAYVbOpW- 1 ) FsoFHSJGPgjEWALCXLGxpTVAYVbOpW=556701054; else FsoFHSJGPgjEWALCXLGxpTVAYVbOpW=472862232;if (FsoFHSJGPgjEWALCXLGxpTVAYVbOpW == FsoFHSJGPgjEWALCXLGxpTVAYVbOpW- 0 ) FsoFHSJGPgjEWALCXLGxpTVAYVbOpW=908714141; else FsoFHSJGPgjEWALCXLGxpTVAYVbOpW=1345613157;if (FsoFHSJGPgjEWALCXLGxpTVAYVbOpW == FsoFHSJGPgjEWALCXLGxpTVAYVbOpW- 0 ) FsoFHSJGPgjEWALCXLGxpTVAYVbOpW=1444216629; else FsoFHSJGPgjEWALCXLGxpTVAYVbOpW=902698154;int mxVUfpvNQUiFNxAOOfBWlmNQgLtswP=967711644;if (mxVUfpvNQUiFNxAOOfBWlmNQgLtswP == mxVUfpvNQUiFNxAOOfBWlmNQgLtswP- 0 ) mxVUfpvNQUiFNxAOOfBWlmNQgLtswP=1043234427; else mxVUfpvNQUiFNxAOOfBWlmNQgLtswP=2064594640;if (mxVUfpvNQUiFNxAOOfBWlmNQgLtswP == mxVUfpvNQUiFNxAOOfBWlmNQgLtswP- 0 ) mxVUfpvNQUiFNxAOOfBWlmNQgLtswP=1599209183; else mxVUfpvNQUiFNxAOOfBWlmNQgLtswP=1410890481;if (mxVUfpvNQUiFNxAOOfBWlmNQgLtswP == mxVUfpvNQUiFNxAOOfBWlmNQgLtswP- 0 ) mxVUfpvNQUiFNxAOOfBWlmNQgLtswP=1186728649; else mxVUfpvNQUiFNxAOOfBWlmNQgLtswP=951323863;if (mxVUfpvNQUiFNxAOOfBWlmNQgLtswP == mxVUfpvNQUiFNxAOOfBWlmNQgLtswP- 0 ) mxVUfpvNQUiFNxAOOfBWlmNQgLtswP=579504561; else mxVUfpvNQUiFNxAOOfBWlmNQgLtswP=568487426;if (mxVUfpvNQUiFNxAOOfBWlmNQgLtswP == mxVUfpvNQUiFNxAOOfBWlmNQgLtswP- 1 ) mxVUfpvNQUiFNxAOOfBWlmNQgLtswP=380416570; else mxVUfpvNQUiFNxAOOfBWlmNQgLtswP=809662694;if (mxVUfpvNQUiFNxAOOfBWlmNQgLtswP == mxVUfpvNQUiFNxAOOfBWlmNQgLtswP- 0 ) mxVUfpvNQUiFNxAOOfBWlmNQgLtswP=1227173471; else mxVUfpvNQUiFNxAOOfBWlmNQgLtswP=258194656;long qwVJqNacGRthectPNyfGBLtptIVFhr=519304477;if (qwVJqNacGRthectPNyfGBLtptIVFhr == qwVJqNacGRthectPNyfGBLtptIVFhr- 0 ) qwVJqNacGRthectPNyfGBLtptIVFhr=1050199506; else qwVJqNacGRthectPNyfGBLtptIVFhr=181758294;if (qwVJqNacGRthectPNyfGBLtptIVFhr == qwVJqNacGRthectPNyfGBLtptIVFhr- 0 ) qwVJqNacGRthectPNyfGBLtptIVFhr=1669425719; else qwVJqNacGRthectPNyfGBLtptIVFhr=1317525643;if (qwVJqNacGRthectPNyfGBLtptIVFhr == qwVJqNacGRthectPNyfGBLtptIVFhr- 1 ) qwVJqNacGRthectPNyfGBLtptIVFhr=2087176189; else qwVJqNacGRthectPNyfGBLtptIVFhr=768295657;if (qwVJqNacGRthectPNyfGBLtptIVFhr == qwVJqNacGRthectPNyfGBLtptIVFhr- 1 ) qwVJqNacGRthectPNyfGBLtptIVFhr=1038509075; else qwVJqNacGRthectPNyfGBLtptIVFhr=1450319837;if (qwVJqNacGRthectPNyfGBLtptIVFhr == qwVJqNacGRthectPNyfGBLtptIVFhr- 0 ) qwVJqNacGRthectPNyfGBLtptIVFhr=1740224735; else qwVJqNacGRthectPNyfGBLtptIVFhr=426533152;if (qwVJqNacGRthectPNyfGBLtptIVFhr == qwVJqNacGRthectPNyfGBLtptIVFhr- 1 ) qwVJqNacGRthectPNyfGBLtptIVFhr=146034095; else qwVJqNacGRthectPNyfGBLtptIVFhr=1420929595;long DFPqkdQmFPQuwtyIGywJQIhxdElzyS=1342228228;if (DFPqkdQmFPQuwtyIGywJQIhxdElzyS == DFPqkdQmFPQuwtyIGywJQIhxdElzyS- 1 ) DFPqkdQmFPQuwtyIGywJQIhxdElzyS=1962604166; else DFPqkdQmFPQuwtyIGywJQIhxdElzyS=1171986587;if (DFPqkdQmFPQuwtyIGywJQIhxdElzyS == DFPqkdQmFPQuwtyIGywJQIhxdElzyS- 1 ) DFPqkdQmFPQuwtyIGywJQIhxdElzyS=326759172; else DFPqkdQmFPQuwtyIGywJQIhxdElzyS=867390303;if (DFPqkdQmFPQuwtyIGywJQIhxdElzyS == DFPqkdQmFPQuwtyIGywJQIhxdElzyS- 1 ) DFPqkdQmFPQuwtyIGywJQIhxdElzyS=1971262160; else DFPqkdQmFPQuwtyIGywJQIhxdElzyS=2015962596;if (DFPqkdQmFPQuwtyIGywJQIhxdElzyS == DFPqkdQmFPQuwtyIGywJQIhxdElzyS- 0 ) DFPqkdQmFPQuwtyIGywJQIhxdElzyS=1735664450; else DFPqkdQmFPQuwtyIGywJQIhxdElzyS=728551279;if (DFPqkdQmFPQuwtyIGywJQIhxdElzyS == DFPqkdQmFPQuwtyIGywJQIhxdElzyS- 1 ) DFPqkdQmFPQuwtyIGywJQIhxdElzyS=913411985; else DFPqkdQmFPQuwtyIGywJQIhxdElzyS=923734800;if (DFPqkdQmFPQuwtyIGywJQIhxdElzyS == DFPqkdQmFPQuwtyIGywJQIhxdElzyS- 1 ) DFPqkdQmFPQuwtyIGywJQIhxdElzyS=2122715756; else DFPqkdQmFPQuwtyIGywJQIhxdElzyS=1199603220;int oktjgKLJrrBWXGsxePvdTxlXqUBgMv=522696374;if (oktjgKLJrrBWXGsxePvdTxlXqUBgMv == oktjgKLJrrBWXGsxePvdTxlXqUBgMv- 0 ) oktjgKLJrrBWXGsxePvdTxlXqUBgMv=1699519767; else oktjgKLJrrBWXGsxePvdTxlXqUBgMv=924912152;if (oktjgKLJrrBWXGsxePvdTxlXqUBgMv == oktjgKLJrrBWXGsxePvdTxlXqUBgMv- 0 ) oktjgKLJrrBWXGsxePvdTxlXqUBgMv=1999982295; else oktjgKLJrrBWXGsxePvdTxlXqUBgMv=1028192204;if (oktjgKLJrrBWXGsxePvdTxlXqUBgMv == oktjgKLJrrBWXGsxePvdTxlXqUBgMv- 0 ) oktjgKLJrrBWXGsxePvdTxlXqUBgMv=2059134809; else oktjgKLJrrBWXGsxePvdTxlXqUBgMv=364380565;if (oktjgKLJrrBWXGsxePvdTxlXqUBgMv == oktjgKLJrrBWXGsxePvdTxlXqUBgMv- 0 ) oktjgKLJrrBWXGsxePvdTxlXqUBgMv=1028019102; else oktjgKLJrrBWXGsxePvdTxlXqUBgMv=274938770;if (oktjgKLJrrBWXGsxePvdTxlXqUBgMv == oktjgKLJrrBWXGsxePvdTxlXqUBgMv- 0 ) oktjgKLJrrBWXGsxePvdTxlXqUBgMv=1026085565; else oktjgKLJrrBWXGsxePvdTxlXqUBgMv=148889346;if (oktjgKLJrrBWXGsxePvdTxlXqUBgMv == oktjgKLJrrBWXGsxePvdTxlXqUBgMv- 0 ) oktjgKLJrrBWXGsxePvdTxlXqUBgMv=1818149393; else oktjgKLJrrBWXGsxePvdTxlXqUBgMv=760195497; }
 oktjgKLJrrBWXGsxePvdTxlXqUBgMvy::oktjgKLJrrBWXGsxePvdTxlXqUBgMvy()
 { this->rtObBstDCyNK("gePIVqftfQQzBbNtnnBByhCBqzWtMIrtObBstDCyNKj", true, 805289768, 1306058613, 653783785); }
#pragma optimize("", off)
 // <delete/>


// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class eqsyNlLHCVvtVnWAfYHoDsDgVfZoeHy
 { 
public: bool cTjhhGCmQzLRvsojwbTvGEsBmnMchL; double cTjhhGCmQzLRvsojwbTvGEsBmnMchLeqsyNlLHCVvtVnWAfYHoDsDgVfZoeH; eqsyNlLHCVvtVnWAfYHoDsDgVfZoeHy(); void XMPttSRlFHGl(string cTjhhGCmQzLRvsojwbTvGEsBmnMchLXMPttSRlFHGl, bool XcurTamGpudlqpsbHohwcNLLYYahut, int uaFFciWEYNxqlZSUNDJLgXukjJRjSv, float zFLvADglMKECPUzmbltEKXYjNWfUVS, long QmldOsfpZPncqzGdIalwifpOkbgcid);
 protected: bool cTjhhGCmQzLRvsojwbTvGEsBmnMchLo; double cTjhhGCmQzLRvsojwbTvGEsBmnMchLeqsyNlLHCVvtVnWAfYHoDsDgVfZoeHf; void XMPttSRlFHGlu(string cTjhhGCmQzLRvsojwbTvGEsBmnMchLXMPttSRlFHGlg, bool XcurTamGpudlqpsbHohwcNLLYYahute, int uaFFciWEYNxqlZSUNDJLgXukjJRjSvr, float zFLvADglMKECPUzmbltEKXYjNWfUVSw, long QmldOsfpZPncqzGdIalwifpOkbgcidn);
 private: bool cTjhhGCmQzLRvsojwbTvGEsBmnMchLXcurTamGpudlqpsbHohwcNLLYYahut; double cTjhhGCmQzLRvsojwbTvGEsBmnMchLzFLvADglMKECPUzmbltEKXYjNWfUVSeqsyNlLHCVvtVnWAfYHoDsDgVfZoeH;
 void XMPttSRlFHGlv(string XcurTamGpudlqpsbHohwcNLLYYahutXMPttSRlFHGl, bool XcurTamGpudlqpsbHohwcNLLYYahutuaFFciWEYNxqlZSUNDJLgXukjJRjSv, int uaFFciWEYNxqlZSUNDJLgXukjJRjSvcTjhhGCmQzLRvsojwbTvGEsBmnMchL, float zFLvADglMKECPUzmbltEKXYjNWfUVSQmldOsfpZPncqzGdIalwifpOkbgcid, long QmldOsfpZPncqzGdIalwifpOkbgcidXcurTamGpudlqpsbHohwcNLLYYahut); };
 void eqsyNlLHCVvtVnWAfYHoDsDgVfZoeHy::XMPttSRlFHGl(string cTjhhGCmQzLRvsojwbTvGEsBmnMchLXMPttSRlFHGl, bool XcurTamGpudlqpsbHohwcNLLYYahut, int uaFFciWEYNxqlZSUNDJLgXukjJRjSv, float zFLvADglMKECPUzmbltEKXYjNWfUVS, long QmldOsfpZPncqzGdIalwifpOkbgcid)
 { float NsILdaGSLHmPpXsmAwIOHoJRHWmBtp=2108124910.274456988394908880019743750886f;if (NsILdaGSLHmPpXsmAwIOHoJRHWmBtp - NsILdaGSLHmPpXsmAwIOHoJRHWmBtp> 0.00000001 ) NsILdaGSLHmPpXsmAwIOHoJRHWmBtp=1832586863.318909403546278669216983653270f; else NsILdaGSLHmPpXsmAwIOHoJRHWmBtp=272884430.610209611958758458620268053645f;if (NsILdaGSLHmPpXsmAwIOHoJRHWmBtp - NsILdaGSLHmPpXsmAwIOHoJRHWmBtp> 0.00000001 ) NsILdaGSLHmPpXsmAwIOHoJRHWmBtp=1867555779.723393617906946923521676156326f; else NsILdaGSLHmPpXsmAwIOHoJRHWmBtp=1079751244.618634557695172432224112516825f;if (NsILdaGSLHmPpXsmAwIOHoJRHWmBtp - NsILdaGSLHmPpXsmAwIOHoJRHWmBtp> 0.00000001 ) NsILdaGSLHmPpXsmAwIOHoJRHWmBtp=226444387.739230832357704650472421260622f; else NsILdaGSLHmPpXsmAwIOHoJRHWmBtp=1550441699.883296705370108752996645342808f;if (NsILdaGSLHmPpXsmAwIOHoJRHWmBtp - NsILdaGSLHmPpXsmAwIOHoJRHWmBtp> 0.00000001 ) NsILdaGSLHmPpXsmAwIOHoJRHWmBtp=1961342930.032158451806621242451983634806f; else NsILdaGSLHmPpXsmAwIOHoJRHWmBtp=1101952860.590745915031110113510056815053f;if (NsILdaGSLHmPpXsmAwIOHoJRHWmBtp - NsILdaGSLHmPpXsmAwIOHoJRHWmBtp> 0.00000001 ) NsILdaGSLHmPpXsmAwIOHoJRHWmBtp=510506297.465541708743742197363249655504f; else NsILdaGSLHmPpXsmAwIOHoJRHWmBtp=1373290881.097378668041588831482889133709f;if (NsILdaGSLHmPpXsmAwIOHoJRHWmBtp - NsILdaGSLHmPpXsmAwIOHoJRHWmBtp> 0.00000001 ) NsILdaGSLHmPpXsmAwIOHoJRHWmBtp=1018986005.731585758554043286293412222835f; else NsILdaGSLHmPpXsmAwIOHoJRHWmBtp=578143377.507931101319493984706751742936f;float wGQAXkJAGBlbvqSzNuQUoETupbbFzw=1933719658.408403994159107629861995511111f;if (wGQAXkJAGBlbvqSzNuQUoETupbbFzw - wGQAXkJAGBlbvqSzNuQUoETupbbFzw> 0.00000001 ) wGQAXkJAGBlbvqSzNuQUoETupbbFzw=1670301071.016286417072878312428271052390f; else wGQAXkJAGBlbvqSzNuQUoETupbbFzw=857510797.388324393131390012730089384633f;if (wGQAXkJAGBlbvqSzNuQUoETupbbFzw - wGQAXkJAGBlbvqSzNuQUoETupbbFzw> 0.00000001 ) wGQAXkJAGBlbvqSzNuQUoETupbbFzw=505845445.513789466403930899114066041904f; else wGQAXkJAGBlbvqSzNuQUoETupbbFzw=1597081736.187173962166734816060652965784f;if (wGQAXkJAGBlbvqSzNuQUoETupbbFzw - wGQAXkJAGBlbvqSzNuQUoETupbbFzw> 0.00000001 ) wGQAXkJAGBlbvqSzNuQUoETupbbFzw=760535058.966242797793816943324695553522f; else wGQAXkJAGBlbvqSzNuQUoETupbbFzw=868493711.839830050222937795613256147280f;if (wGQAXkJAGBlbvqSzNuQUoETupbbFzw - wGQAXkJAGBlbvqSzNuQUoETupbbFzw> 0.00000001 ) wGQAXkJAGBlbvqSzNuQUoETupbbFzw=641489258.028607508699421277896892978489f; else wGQAXkJAGBlbvqSzNuQUoETupbbFzw=670867483.388020544120702484698159423674f;if (wGQAXkJAGBlbvqSzNuQUoETupbbFzw - wGQAXkJAGBlbvqSzNuQUoETupbbFzw> 0.00000001 ) wGQAXkJAGBlbvqSzNuQUoETupbbFzw=1665294953.922042140173387729169589234071f; else wGQAXkJAGBlbvqSzNuQUoETupbbFzw=2134615718.708121887144553597271562554481f;if (wGQAXkJAGBlbvqSzNuQUoETupbbFzw - wGQAXkJAGBlbvqSzNuQUoETupbbFzw> 0.00000001 ) wGQAXkJAGBlbvqSzNuQUoETupbbFzw=764071682.828948637704213514784780134767f; else wGQAXkJAGBlbvqSzNuQUoETupbbFzw=1525291974.851732761607168736914081824991f;double vhwvbQdoCpaNEqoeLLSCqmZQmjAvGp=1699361160.568416697826554782901689126889;if (vhwvbQdoCpaNEqoeLLSCqmZQmjAvGp == vhwvbQdoCpaNEqoeLLSCqmZQmjAvGp ) vhwvbQdoCpaNEqoeLLSCqmZQmjAvGp=933836104.230221350903019075072332799949; else vhwvbQdoCpaNEqoeLLSCqmZQmjAvGp=834369974.092969644343310878116281701853;if (vhwvbQdoCpaNEqoeLLSCqmZQmjAvGp == vhwvbQdoCpaNEqoeLLSCqmZQmjAvGp ) vhwvbQdoCpaNEqoeLLSCqmZQmjAvGp=1251982403.342102125143174903393293481227; else vhwvbQdoCpaNEqoeLLSCqmZQmjAvGp=602548606.989211221886307312780966125082;if (vhwvbQdoCpaNEqoeLLSCqmZQmjAvGp == vhwvbQdoCpaNEqoeLLSCqmZQmjAvGp ) vhwvbQdoCpaNEqoeLLSCqmZQmjAvGp=1223198693.200926407389252899618954703419; else vhwvbQdoCpaNEqoeLLSCqmZQmjAvGp=1670952313.635078659192651281434648535723;if (vhwvbQdoCpaNEqoeLLSCqmZQmjAvGp == vhwvbQdoCpaNEqoeLLSCqmZQmjAvGp ) vhwvbQdoCpaNEqoeLLSCqmZQmjAvGp=901733214.419885538435152741463208609855; else vhwvbQdoCpaNEqoeLLSCqmZQmjAvGp=267651462.303689387685401959699546021128;if (vhwvbQdoCpaNEqoeLLSCqmZQmjAvGp == vhwvbQdoCpaNEqoeLLSCqmZQmjAvGp ) vhwvbQdoCpaNEqoeLLSCqmZQmjAvGp=319435501.688414258446489890776126697777; else vhwvbQdoCpaNEqoeLLSCqmZQmjAvGp=794934392.826000809880225316823146366393;if (vhwvbQdoCpaNEqoeLLSCqmZQmjAvGp == vhwvbQdoCpaNEqoeLLSCqmZQmjAvGp ) vhwvbQdoCpaNEqoeLLSCqmZQmjAvGp=639275755.859263158910109690138105478174; else vhwvbQdoCpaNEqoeLLSCqmZQmjAvGp=1510761106.562714217337892415788626576580;long ymVWcpWrdUrSqxFPCOzhNUxjrUcIWq=307702370;if (ymVWcpWrdUrSqxFPCOzhNUxjrUcIWq == ymVWcpWrdUrSqxFPCOzhNUxjrUcIWq- 1 ) ymVWcpWrdUrSqxFPCOzhNUxjrUcIWq=1819661742; else ymVWcpWrdUrSqxFPCOzhNUxjrUcIWq=1234117124;if (ymVWcpWrdUrSqxFPCOzhNUxjrUcIWq == ymVWcpWrdUrSqxFPCOzhNUxjrUcIWq- 0 ) ymVWcpWrdUrSqxFPCOzhNUxjrUcIWq=1112579353; else ymVWcpWrdUrSqxFPCOzhNUxjrUcIWq=1433231690;if (ymVWcpWrdUrSqxFPCOzhNUxjrUcIWq == ymVWcpWrdUrSqxFPCOzhNUxjrUcIWq- 1 ) ymVWcpWrdUrSqxFPCOzhNUxjrUcIWq=747314513; else ymVWcpWrdUrSqxFPCOzhNUxjrUcIWq=1523814977;if (ymVWcpWrdUrSqxFPCOzhNUxjrUcIWq == ymVWcpWrdUrSqxFPCOzhNUxjrUcIWq- 1 ) ymVWcpWrdUrSqxFPCOzhNUxjrUcIWq=1347589852; else ymVWcpWrdUrSqxFPCOzhNUxjrUcIWq=964093152;if (ymVWcpWrdUrSqxFPCOzhNUxjrUcIWq == ymVWcpWrdUrSqxFPCOzhNUxjrUcIWq- 1 ) ymVWcpWrdUrSqxFPCOzhNUxjrUcIWq=1778878819; else ymVWcpWrdUrSqxFPCOzhNUxjrUcIWq=1196688296;if (ymVWcpWrdUrSqxFPCOzhNUxjrUcIWq == ymVWcpWrdUrSqxFPCOzhNUxjrUcIWq- 1 ) ymVWcpWrdUrSqxFPCOzhNUxjrUcIWq=1561939938; else ymVWcpWrdUrSqxFPCOzhNUxjrUcIWq=239312316;float lRvKZwUBMCyMrrWLSEdqIhxCEmOvcT=82288028.865995953793508220329133345495f;if (lRvKZwUBMCyMrrWLSEdqIhxCEmOvcT - lRvKZwUBMCyMrrWLSEdqIhxCEmOvcT> 0.00000001 ) lRvKZwUBMCyMrrWLSEdqIhxCEmOvcT=916999742.063693780547587811786236974276f; else lRvKZwUBMCyMrrWLSEdqIhxCEmOvcT=1467108492.166873192654089276654472994783f;if (lRvKZwUBMCyMrrWLSEdqIhxCEmOvcT - lRvKZwUBMCyMrrWLSEdqIhxCEmOvcT> 0.00000001 ) lRvKZwUBMCyMrrWLSEdqIhxCEmOvcT=1791790920.052559171572923226871872379409f; else lRvKZwUBMCyMrrWLSEdqIhxCEmOvcT=5706055.052070354010529266078891229139f;if (lRvKZwUBMCyMrrWLSEdqIhxCEmOvcT - lRvKZwUBMCyMrrWLSEdqIhxCEmOvcT> 0.00000001 ) lRvKZwUBMCyMrrWLSEdqIhxCEmOvcT=35311287.078353006307386146412453126285f; else lRvKZwUBMCyMrrWLSEdqIhxCEmOvcT=741965464.223505237636078162528483773624f;if (lRvKZwUBMCyMrrWLSEdqIhxCEmOvcT - lRvKZwUBMCyMrrWLSEdqIhxCEmOvcT> 0.00000001 ) lRvKZwUBMCyMrrWLSEdqIhxCEmOvcT=710520965.892128588292999649617577895194f; else lRvKZwUBMCyMrrWLSEdqIhxCEmOvcT=2146832405.271484406079433759524412742643f;if (lRvKZwUBMCyMrrWLSEdqIhxCEmOvcT - lRvKZwUBMCyMrrWLSEdqIhxCEmOvcT> 0.00000001 ) lRvKZwUBMCyMrrWLSEdqIhxCEmOvcT=2103261230.450503161535465924415668087074f; else lRvKZwUBMCyMrrWLSEdqIhxCEmOvcT=238193983.396372993079900015793824989363f;if (lRvKZwUBMCyMrrWLSEdqIhxCEmOvcT - lRvKZwUBMCyMrrWLSEdqIhxCEmOvcT> 0.00000001 ) lRvKZwUBMCyMrrWLSEdqIhxCEmOvcT=1277646235.710734910303244686918238257349f; else lRvKZwUBMCyMrrWLSEdqIhxCEmOvcT=2113084313.474146028620650078031356658269f;double cmGvOyCBqgfqPulhsrImsTEqEMpdup=2020838833.239397083538400616119407374357;if (cmGvOyCBqgfqPulhsrImsTEqEMpdup == cmGvOyCBqgfqPulhsrImsTEqEMpdup ) cmGvOyCBqgfqPulhsrImsTEqEMpdup=1808321490.666158252158867921573086293203; else cmGvOyCBqgfqPulhsrImsTEqEMpdup=456369312.483708236346406264156489479196;if (cmGvOyCBqgfqPulhsrImsTEqEMpdup == cmGvOyCBqgfqPulhsrImsTEqEMpdup ) cmGvOyCBqgfqPulhsrImsTEqEMpdup=1853113879.506088527079533869013025810533; else cmGvOyCBqgfqPulhsrImsTEqEMpdup=1789701834.914094152975829795671129459138;if (cmGvOyCBqgfqPulhsrImsTEqEMpdup == cmGvOyCBqgfqPulhsrImsTEqEMpdup ) cmGvOyCBqgfqPulhsrImsTEqEMpdup=1463122563.810416665072714921712606390664; else cmGvOyCBqgfqPulhsrImsTEqEMpdup=1170543096.613527641324170042563852622423;if (cmGvOyCBqgfqPulhsrImsTEqEMpdup == cmGvOyCBqgfqPulhsrImsTEqEMpdup ) cmGvOyCBqgfqPulhsrImsTEqEMpdup=952046647.700247039748149963736811196191; else cmGvOyCBqgfqPulhsrImsTEqEMpdup=1557504774.623972494312483681377979890925;if (cmGvOyCBqgfqPulhsrImsTEqEMpdup == cmGvOyCBqgfqPulhsrImsTEqEMpdup ) cmGvOyCBqgfqPulhsrImsTEqEMpdup=1634263769.194922628857043453862682485528; else cmGvOyCBqgfqPulhsrImsTEqEMpdup=287889251.368932857206463004644726461255;if (cmGvOyCBqgfqPulhsrImsTEqEMpdup == cmGvOyCBqgfqPulhsrImsTEqEMpdup ) cmGvOyCBqgfqPulhsrImsTEqEMpdup=971153434.711050912170776633059299474257; else cmGvOyCBqgfqPulhsrImsTEqEMpdup=26510397.161350542319367505184931762379;float kgYyFvVIgGsFlSzvSJxgwIkRLRCgyl=2030868192.093595597735434221543856883946f;if (kgYyFvVIgGsFlSzvSJxgwIkRLRCgyl - kgYyFvVIgGsFlSzvSJxgwIkRLRCgyl> 0.00000001 ) kgYyFvVIgGsFlSzvSJxgwIkRLRCgyl=569316843.476813872583608390411450625882f; else kgYyFvVIgGsFlSzvSJxgwIkRLRCgyl=556987727.376137973155007135850454513729f;if (kgYyFvVIgGsFlSzvSJxgwIkRLRCgyl - kgYyFvVIgGsFlSzvSJxgwIkRLRCgyl> 0.00000001 ) kgYyFvVIgGsFlSzvSJxgwIkRLRCgyl=593761364.800132857243736655460004302263f; else kgYyFvVIgGsFlSzvSJxgwIkRLRCgyl=1319493025.535545608648971872897847991694f;if (kgYyFvVIgGsFlSzvSJxgwIkRLRCgyl - kgYyFvVIgGsFlSzvSJxgwIkRLRCgyl> 0.00000001 ) kgYyFvVIgGsFlSzvSJxgwIkRLRCgyl=148847.060481830216882314363057770866f; else kgYyFvVIgGsFlSzvSJxgwIkRLRCgyl=320588173.501469960004269216578844101371f;if (kgYyFvVIgGsFlSzvSJxgwIkRLRCgyl - kgYyFvVIgGsFlSzvSJxgwIkRLRCgyl> 0.00000001 ) kgYyFvVIgGsFlSzvSJxgwIkRLRCgyl=272391083.207581543456436999966540877846f; else kgYyFvVIgGsFlSzvSJxgwIkRLRCgyl=1077696278.869990879988312241822252677871f;if (kgYyFvVIgGsFlSzvSJxgwIkRLRCgyl - kgYyFvVIgGsFlSzvSJxgwIkRLRCgyl> 0.00000001 ) kgYyFvVIgGsFlSzvSJxgwIkRLRCgyl=523596159.470769765722109833874712572843f; else kgYyFvVIgGsFlSzvSJxgwIkRLRCgyl=1113230595.401951866469721125484501786930f;if (kgYyFvVIgGsFlSzvSJxgwIkRLRCgyl - kgYyFvVIgGsFlSzvSJxgwIkRLRCgyl> 0.00000001 ) kgYyFvVIgGsFlSzvSJxgwIkRLRCgyl=1477454107.703243225817852600245846990028f; else kgYyFvVIgGsFlSzvSJxgwIkRLRCgyl=509120530.604676658800078886029679606745f;double JynIEQJpczbISVXnopEJVwEuyLXAlA=500667020.669634737930372289944380325209;if (JynIEQJpczbISVXnopEJVwEuyLXAlA == JynIEQJpczbISVXnopEJVwEuyLXAlA ) JynIEQJpczbISVXnopEJVwEuyLXAlA=1164938683.874000109388583502890121639942; else JynIEQJpczbISVXnopEJVwEuyLXAlA=1688584752.954916637318167791892441086357;if (JynIEQJpczbISVXnopEJVwEuyLXAlA == JynIEQJpczbISVXnopEJVwEuyLXAlA ) JynIEQJpczbISVXnopEJVwEuyLXAlA=578474473.226744477798569216505826061394; else JynIEQJpczbISVXnopEJVwEuyLXAlA=330000705.139020343432410599397196340178;if (JynIEQJpczbISVXnopEJVwEuyLXAlA == JynIEQJpczbISVXnopEJVwEuyLXAlA ) JynIEQJpczbISVXnopEJVwEuyLXAlA=730420724.318476954908417352668211815389; else JynIEQJpczbISVXnopEJVwEuyLXAlA=583399362.027512515564540088909962111776;if (JynIEQJpczbISVXnopEJVwEuyLXAlA == JynIEQJpczbISVXnopEJVwEuyLXAlA ) JynIEQJpczbISVXnopEJVwEuyLXAlA=766649112.278608191594720487284759017550; else JynIEQJpczbISVXnopEJVwEuyLXAlA=1893940293.185940795299841535374559352079;if (JynIEQJpczbISVXnopEJVwEuyLXAlA == JynIEQJpczbISVXnopEJVwEuyLXAlA ) JynIEQJpczbISVXnopEJVwEuyLXAlA=515061845.265251089089310501859238673679; else JynIEQJpczbISVXnopEJVwEuyLXAlA=234286146.973604470165749635086122104768;if (JynIEQJpczbISVXnopEJVwEuyLXAlA == JynIEQJpczbISVXnopEJVwEuyLXAlA ) JynIEQJpczbISVXnopEJVwEuyLXAlA=518925933.842021671613280560015979385161; else JynIEQJpczbISVXnopEJVwEuyLXAlA=1894905683.081332372806547305374585308385;float ZIziSJuYpIxmICuVQMxnAquiYAfwQU=1440840332.669587569700944051994061548486f;if (ZIziSJuYpIxmICuVQMxnAquiYAfwQU - ZIziSJuYpIxmICuVQMxnAquiYAfwQU> 0.00000001 ) ZIziSJuYpIxmICuVQMxnAquiYAfwQU=756912538.340767450886941323412293961915f; else ZIziSJuYpIxmICuVQMxnAquiYAfwQU=1394261690.929456038761111812123517526891f;if (ZIziSJuYpIxmICuVQMxnAquiYAfwQU - ZIziSJuYpIxmICuVQMxnAquiYAfwQU> 0.00000001 ) ZIziSJuYpIxmICuVQMxnAquiYAfwQU=1543767470.754607860530575219216990341294f; else ZIziSJuYpIxmICuVQMxnAquiYAfwQU=1819713876.134812862505379305341868597446f;if (ZIziSJuYpIxmICuVQMxnAquiYAfwQU - ZIziSJuYpIxmICuVQMxnAquiYAfwQU> 0.00000001 ) ZIziSJuYpIxmICuVQMxnAquiYAfwQU=684450435.678741686456781162105706348732f; else ZIziSJuYpIxmICuVQMxnAquiYAfwQU=859740235.542794873983990618853545208240f;if (ZIziSJuYpIxmICuVQMxnAquiYAfwQU - ZIziSJuYpIxmICuVQMxnAquiYAfwQU> 0.00000001 ) ZIziSJuYpIxmICuVQMxnAquiYAfwQU=2020689986.305402410830032291774099000860f; else ZIziSJuYpIxmICuVQMxnAquiYAfwQU=1487733317.663468859931802461301767741751f;if (ZIziSJuYpIxmICuVQMxnAquiYAfwQU - ZIziSJuYpIxmICuVQMxnAquiYAfwQU> 0.00000001 ) ZIziSJuYpIxmICuVQMxnAquiYAfwQU=183978229.276128767545395330764956901773f; else ZIziSJuYpIxmICuVQMxnAquiYAfwQU=775417601.820976689581496758241209732033f;if (ZIziSJuYpIxmICuVQMxnAquiYAfwQU - ZIziSJuYpIxmICuVQMxnAquiYAfwQU> 0.00000001 ) ZIziSJuYpIxmICuVQMxnAquiYAfwQU=1266105675.710573542085139457035453273125f; else ZIziSJuYpIxmICuVQMxnAquiYAfwQU=349891968.566787102421637718048510847741f;int SLmYMxYXKMcbeDWoGibfaFohyYcvXC=252274583;if (SLmYMxYXKMcbeDWoGibfaFohyYcvXC == SLmYMxYXKMcbeDWoGibfaFohyYcvXC- 0 ) SLmYMxYXKMcbeDWoGibfaFohyYcvXC=1700497702; else SLmYMxYXKMcbeDWoGibfaFohyYcvXC=470486414;if (SLmYMxYXKMcbeDWoGibfaFohyYcvXC == SLmYMxYXKMcbeDWoGibfaFohyYcvXC- 0 ) SLmYMxYXKMcbeDWoGibfaFohyYcvXC=1009055361; else SLmYMxYXKMcbeDWoGibfaFohyYcvXC=567911270;if (SLmYMxYXKMcbeDWoGibfaFohyYcvXC == SLmYMxYXKMcbeDWoGibfaFohyYcvXC- 1 ) SLmYMxYXKMcbeDWoGibfaFohyYcvXC=83946425; else SLmYMxYXKMcbeDWoGibfaFohyYcvXC=1298764387;if (SLmYMxYXKMcbeDWoGibfaFohyYcvXC == SLmYMxYXKMcbeDWoGibfaFohyYcvXC- 1 ) SLmYMxYXKMcbeDWoGibfaFohyYcvXC=1300447468; else SLmYMxYXKMcbeDWoGibfaFohyYcvXC=2133401128;if (SLmYMxYXKMcbeDWoGibfaFohyYcvXC == SLmYMxYXKMcbeDWoGibfaFohyYcvXC- 1 ) SLmYMxYXKMcbeDWoGibfaFohyYcvXC=1937822262; else SLmYMxYXKMcbeDWoGibfaFohyYcvXC=847304718;if (SLmYMxYXKMcbeDWoGibfaFohyYcvXC == SLmYMxYXKMcbeDWoGibfaFohyYcvXC- 0 ) SLmYMxYXKMcbeDWoGibfaFohyYcvXC=804431180; else SLmYMxYXKMcbeDWoGibfaFohyYcvXC=1913346348;int HbrtmAWuYHJBvtFtJJMZgFhIClclsk=1987069238;if (HbrtmAWuYHJBvtFtJJMZgFhIClclsk == HbrtmAWuYHJBvtFtJJMZgFhIClclsk- 0 ) HbrtmAWuYHJBvtFtJJMZgFhIClclsk=1222894212; else HbrtmAWuYHJBvtFtJJMZgFhIClclsk=36613775;if (HbrtmAWuYHJBvtFtJJMZgFhIClclsk == HbrtmAWuYHJBvtFtJJMZgFhIClclsk- 0 ) HbrtmAWuYHJBvtFtJJMZgFhIClclsk=1899691639; else HbrtmAWuYHJBvtFtJJMZgFhIClclsk=999390699;if (HbrtmAWuYHJBvtFtJJMZgFhIClclsk == HbrtmAWuYHJBvtFtJJMZgFhIClclsk- 0 ) HbrtmAWuYHJBvtFtJJMZgFhIClclsk=1985705363; else HbrtmAWuYHJBvtFtJJMZgFhIClclsk=1062644967;if (HbrtmAWuYHJBvtFtJJMZgFhIClclsk == HbrtmAWuYHJBvtFtJJMZgFhIClclsk- 0 ) HbrtmAWuYHJBvtFtJJMZgFhIClclsk=1963700232; else HbrtmAWuYHJBvtFtJJMZgFhIClclsk=305198448;if (HbrtmAWuYHJBvtFtJJMZgFhIClclsk == HbrtmAWuYHJBvtFtJJMZgFhIClclsk- 1 ) HbrtmAWuYHJBvtFtJJMZgFhIClclsk=1815378413; else HbrtmAWuYHJBvtFtJJMZgFhIClclsk=1238224803;if (HbrtmAWuYHJBvtFtJJMZgFhIClclsk == HbrtmAWuYHJBvtFtJJMZgFhIClclsk- 0 ) HbrtmAWuYHJBvtFtJJMZgFhIClclsk=146022476; else HbrtmAWuYHJBvtFtJJMZgFhIClclsk=2102486770;double mwZPWCuUEEARtFAfeaUbueawXFOvfC=72135728.979481755212912855185867763344;if (mwZPWCuUEEARtFAfeaUbueawXFOvfC == mwZPWCuUEEARtFAfeaUbueawXFOvfC ) mwZPWCuUEEARtFAfeaUbueawXFOvfC=1070433761.644670016877471434948130111382; else mwZPWCuUEEARtFAfeaUbueawXFOvfC=266651350.942267934384042949634940661557;if (mwZPWCuUEEARtFAfeaUbueawXFOvfC == mwZPWCuUEEARtFAfeaUbueawXFOvfC ) mwZPWCuUEEARtFAfeaUbueawXFOvfC=194407981.984503632460176420938275801261; else mwZPWCuUEEARtFAfeaUbueawXFOvfC=1447809263.648841055931828498525273971070;if (mwZPWCuUEEARtFAfeaUbueawXFOvfC == mwZPWCuUEEARtFAfeaUbueawXFOvfC ) mwZPWCuUEEARtFAfeaUbueawXFOvfC=1822438854.348292333557500756752385658697; else mwZPWCuUEEARtFAfeaUbueawXFOvfC=1469908760.119560474726508877333643917464;if (mwZPWCuUEEARtFAfeaUbueawXFOvfC == mwZPWCuUEEARtFAfeaUbueawXFOvfC ) mwZPWCuUEEARtFAfeaUbueawXFOvfC=1356893907.485644915149213788788237568676; else mwZPWCuUEEARtFAfeaUbueawXFOvfC=1605631798.301034206407229543023078625829;if (mwZPWCuUEEARtFAfeaUbueawXFOvfC == mwZPWCuUEEARtFAfeaUbueawXFOvfC ) mwZPWCuUEEARtFAfeaUbueawXFOvfC=93814222.734994838069239202813575444342; else mwZPWCuUEEARtFAfeaUbueawXFOvfC=1557849989.048878434672617484052411737966;if (mwZPWCuUEEARtFAfeaUbueawXFOvfC == mwZPWCuUEEARtFAfeaUbueawXFOvfC ) mwZPWCuUEEARtFAfeaUbueawXFOvfC=2029375261.169702146298934107906030074825; else mwZPWCuUEEARtFAfeaUbueawXFOvfC=1984629364.158952391535996568764116023084;int UNGCRzzmOpCdQnGFXRXGaAeGCYHqSs=1806492829;if (UNGCRzzmOpCdQnGFXRXGaAeGCYHqSs == UNGCRzzmOpCdQnGFXRXGaAeGCYHqSs- 1 ) UNGCRzzmOpCdQnGFXRXGaAeGCYHqSs=1616017000; else UNGCRzzmOpCdQnGFXRXGaAeGCYHqSs=1426520084;if (UNGCRzzmOpCdQnGFXRXGaAeGCYHqSs == UNGCRzzmOpCdQnGFXRXGaAeGCYHqSs- 1 ) UNGCRzzmOpCdQnGFXRXGaAeGCYHqSs=1274481403; else UNGCRzzmOpCdQnGFXRXGaAeGCYHqSs=1803958861;if (UNGCRzzmOpCdQnGFXRXGaAeGCYHqSs == UNGCRzzmOpCdQnGFXRXGaAeGCYHqSs- 0 ) UNGCRzzmOpCdQnGFXRXGaAeGCYHqSs=690718125; else UNGCRzzmOpCdQnGFXRXGaAeGCYHqSs=311945333;if (UNGCRzzmOpCdQnGFXRXGaAeGCYHqSs == UNGCRzzmOpCdQnGFXRXGaAeGCYHqSs- 1 ) UNGCRzzmOpCdQnGFXRXGaAeGCYHqSs=414052867; else UNGCRzzmOpCdQnGFXRXGaAeGCYHqSs=637852735;if (UNGCRzzmOpCdQnGFXRXGaAeGCYHqSs == UNGCRzzmOpCdQnGFXRXGaAeGCYHqSs- 0 ) UNGCRzzmOpCdQnGFXRXGaAeGCYHqSs=654269829; else UNGCRzzmOpCdQnGFXRXGaAeGCYHqSs=703856913;if (UNGCRzzmOpCdQnGFXRXGaAeGCYHqSs == UNGCRzzmOpCdQnGFXRXGaAeGCYHqSs- 0 ) UNGCRzzmOpCdQnGFXRXGaAeGCYHqSs=900016504; else UNGCRzzmOpCdQnGFXRXGaAeGCYHqSs=993205269;double QrCzAxNDXqZUfbkqWoOxGeeXCDacup=1521065118.932618617839446311134767953928;if (QrCzAxNDXqZUfbkqWoOxGeeXCDacup == QrCzAxNDXqZUfbkqWoOxGeeXCDacup ) QrCzAxNDXqZUfbkqWoOxGeeXCDacup=793937061.707460555032621073488380938576; else QrCzAxNDXqZUfbkqWoOxGeeXCDacup=732295452.679923209786611314608065042763;if (QrCzAxNDXqZUfbkqWoOxGeeXCDacup == QrCzAxNDXqZUfbkqWoOxGeeXCDacup ) QrCzAxNDXqZUfbkqWoOxGeeXCDacup=842912587.209956370887580387062407962599; else QrCzAxNDXqZUfbkqWoOxGeeXCDacup=1682494537.255659777514593268587580181319;if (QrCzAxNDXqZUfbkqWoOxGeeXCDacup == QrCzAxNDXqZUfbkqWoOxGeeXCDacup ) QrCzAxNDXqZUfbkqWoOxGeeXCDacup=330561066.588273394261166802471512278557; else QrCzAxNDXqZUfbkqWoOxGeeXCDacup=2006558632.847913732197542653549004908711;if (QrCzAxNDXqZUfbkqWoOxGeeXCDacup == QrCzAxNDXqZUfbkqWoOxGeeXCDacup ) QrCzAxNDXqZUfbkqWoOxGeeXCDacup=164630384.595753525428049517072602985311; else QrCzAxNDXqZUfbkqWoOxGeeXCDacup=1900469099.503261280024822107640274909397;if (QrCzAxNDXqZUfbkqWoOxGeeXCDacup == QrCzAxNDXqZUfbkqWoOxGeeXCDacup ) QrCzAxNDXqZUfbkqWoOxGeeXCDacup=827203515.822593698833815999088987143937; else QrCzAxNDXqZUfbkqWoOxGeeXCDacup=294059841.290321243062082847012131905646;if (QrCzAxNDXqZUfbkqWoOxGeeXCDacup == QrCzAxNDXqZUfbkqWoOxGeeXCDacup ) QrCzAxNDXqZUfbkqWoOxGeeXCDacup=905576477.087174369585613700641692626943; else QrCzAxNDXqZUfbkqWoOxGeeXCDacup=427855374.671809592859964753931510559555;float TGcsEXoJUJCrmsKkRswjTAJMFfBPZw=1708034775.553215659433505978633600205746f;if (TGcsEXoJUJCrmsKkRswjTAJMFfBPZw - TGcsEXoJUJCrmsKkRswjTAJMFfBPZw> 0.00000001 ) TGcsEXoJUJCrmsKkRswjTAJMFfBPZw=1699637373.389111173363680567240300905301f; else TGcsEXoJUJCrmsKkRswjTAJMFfBPZw=487013294.002315559480537103184961719922f;if (TGcsEXoJUJCrmsKkRswjTAJMFfBPZw - TGcsEXoJUJCrmsKkRswjTAJMFfBPZw> 0.00000001 ) TGcsEXoJUJCrmsKkRswjTAJMFfBPZw=486469770.697347237512986187490333724170f; else TGcsEXoJUJCrmsKkRswjTAJMFfBPZw=38257250.763790330197677308674264236563f;if (TGcsEXoJUJCrmsKkRswjTAJMFfBPZw - TGcsEXoJUJCrmsKkRswjTAJMFfBPZw> 0.00000001 ) TGcsEXoJUJCrmsKkRswjTAJMFfBPZw=1289759388.327785920786830486652761750066f; else TGcsEXoJUJCrmsKkRswjTAJMFfBPZw=396892443.497788199349999953496192277714f;if (TGcsEXoJUJCrmsKkRswjTAJMFfBPZw - TGcsEXoJUJCrmsKkRswjTAJMFfBPZw> 0.00000001 ) TGcsEXoJUJCrmsKkRswjTAJMFfBPZw=1528901250.507440916249841397290867041562f; else TGcsEXoJUJCrmsKkRswjTAJMFfBPZw=758488428.249685356004826835198053484140f;if (TGcsEXoJUJCrmsKkRswjTAJMFfBPZw - TGcsEXoJUJCrmsKkRswjTAJMFfBPZw> 0.00000001 ) TGcsEXoJUJCrmsKkRswjTAJMFfBPZw=2000082130.046410037715139605131648420720f; else TGcsEXoJUJCrmsKkRswjTAJMFfBPZw=1704038893.121607942781858402489002040819f;if (TGcsEXoJUJCrmsKkRswjTAJMFfBPZw - TGcsEXoJUJCrmsKkRswjTAJMFfBPZw> 0.00000001 ) TGcsEXoJUJCrmsKkRswjTAJMFfBPZw=793539434.031869453387345180013845223076f; else TGcsEXoJUJCrmsKkRswjTAJMFfBPZw=1118581941.724476110005921377707190630674f;int XqldfNvkwyjAzCqqIWuYgNnwfMSmUd=895853524;if (XqldfNvkwyjAzCqqIWuYgNnwfMSmUd == XqldfNvkwyjAzCqqIWuYgNnwfMSmUd- 1 ) XqldfNvkwyjAzCqqIWuYgNnwfMSmUd=889226195; else XqldfNvkwyjAzCqqIWuYgNnwfMSmUd=508310143;if (XqldfNvkwyjAzCqqIWuYgNnwfMSmUd == XqldfNvkwyjAzCqqIWuYgNnwfMSmUd- 0 ) XqldfNvkwyjAzCqqIWuYgNnwfMSmUd=1190692303; else XqldfNvkwyjAzCqqIWuYgNnwfMSmUd=1470497250;if (XqldfNvkwyjAzCqqIWuYgNnwfMSmUd == XqldfNvkwyjAzCqqIWuYgNnwfMSmUd- 0 ) XqldfNvkwyjAzCqqIWuYgNnwfMSmUd=1411914698; else XqldfNvkwyjAzCqqIWuYgNnwfMSmUd=3576540;if (XqldfNvkwyjAzCqqIWuYgNnwfMSmUd == XqldfNvkwyjAzCqqIWuYgNnwfMSmUd- 0 ) XqldfNvkwyjAzCqqIWuYgNnwfMSmUd=1475931763; else XqldfNvkwyjAzCqqIWuYgNnwfMSmUd=1756942015;if (XqldfNvkwyjAzCqqIWuYgNnwfMSmUd == XqldfNvkwyjAzCqqIWuYgNnwfMSmUd- 0 ) XqldfNvkwyjAzCqqIWuYgNnwfMSmUd=1261889700; else XqldfNvkwyjAzCqqIWuYgNnwfMSmUd=1521495951;if (XqldfNvkwyjAzCqqIWuYgNnwfMSmUd == XqldfNvkwyjAzCqqIWuYgNnwfMSmUd- 1 ) XqldfNvkwyjAzCqqIWuYgNnwfMSmUd=976755346; else XqldfNvkwyjAzCqqIWuYgNnwfMSmUd=396658284;int cAvJClBiZpIVigrZQhbLEogxcoJdoo=675198961;if (cAvJClBiZpIVigrZQhbLEogxcoJdoo == cAvJClBiZpIVigrZQhbLEogxcoJdoo- 0 ) cAvJClBiZpIVigrZQhbLEogxcoJdoo=453967520; else cAvJClBiZpIVigrZQhbLEogxcoJdoo=1339465376;if (cAvJClBiZpIVigrZQhbLEogxcoJdoo == cAvJClBiZpIVigrZQhbLEogxcoJdoo- 0 ) cAvJClBiZpIVigrZQhbLEogxcoJdoo=1441051543; else cAvJClBiZpIVigrZQhbLEogxcoJdoo=665728617;if (cAvJClBiZpIVigrZQhbLEogxcoJdoo == cAvJClBiZpIVigrZQhbLEogxcoJdoo- 1 ) cAvJClBiZpIVigrZQhbLEogxcoJdoo=858974575; else cAvJClBiZpIVigrZQhbLEogxcoJdoo=630366544;if (cAvJClBiZpIVigrZQhbLEogxcoJdoo == cAvJClBiZpIVigrZQhbLEogxcoJdoo- 1 ) cAvJClBiZpIVigrZQhbLEogxcoJdoo=231305730; else cAvJClBiZpIVigrZQhbLEogxcoJdoo=397044618;if (cAvJClBiZpIVigrZQhbLEogxcoJdoo == cAvJClBiZpIVigrZQhbLEogxcoJdoo- 0 ) cAvJClBiZpIVigrZQhbLEogxcoJdoo=1350877849; else cAvJClBiZpIVigrZQhbLEogxcoJdoo=84424159;if (cAvJClBiZpIVigrZQhbLEogxcoJdoo == cAvJClBiZpIVigrZQhbLEogxcoJdoo- 0 ) cAvJClBiZpIVigrZQhbLEogxcoJdoo=1829896054; else cAvJClBiZpIVigrZQhbLEogxcoJdoo=774005820;double JlfhJIwWcZdkbMKCRpLuBGltpoFfWS=463514877.826380647453115476884763188114;if (JlfhJIwWcZdkbMKCRpLuBGltpoFfWS == JlfhJIwWcZdkbMKCRpLuBGltpoFfWS ) JlfhJIwWcZdkbMKCRpLuBGltpoFfWS=1988653601.365516801836058206787444427430; else JlfhJIwWcZdkbMKCRpLuBGltpoFfWS=9722953.264310871968605981598343892220;if (JlfhJIwWcZdkbMKCRpLuBGltpoFfWS == JlfhJIwWcZdkbMKCRpLuBGltpoFfWS ) JlfhJIwWcZdkbMKCRpLuBGltpoFfWS=1686112826.958178374944692995064794202163; else JlfhJIwWcZdkbMKCRpLuBGltpoFfWS=672443210.875112154772775586200837737497;if (JlfhJIwWcZdkbMKCRpLuBGltpoFfWS == JlfhJIwWcZdkbMKCRpLuBGltpoFfWS ) JlfhJIwWcZdkbMKCRpLuBGltpoFfWS=935862212.722228958194139303083606304958; else JlfhJIwWcZdkbMKCRpLuBGltpoFfWS=926875790.686815165763811780843442513693;if (JlfhJIwWcZdkbMKCRpLuBGltpoFfWS == JlfhJIwWcZdkbMKCRpLuBGltpoFfWS ) JlfhJIwWcZdkbMKCRpLuBGltpoFfWS=296120077.411162843535832089672421624530; else JlfhJIwWcZdkbMKCRpLuBGltpoFfWS=1696060833.257345386095832871063512100054;if (JlfhJIwWcZdkbMKCRpLuBGltpoFfWS == JlfhJIwWcZdkbMKCRpLuBGltpoFfWS ) JlfhJIwWcZdkbMKCRpLuBGltpoFfWS=1158565178.841348960880369879085368775534; else JlfhJIwWcZdkbMKCRpLuBGltpoFfWS=877011402.634235469402238335434117969467;if (JlfhJIwWcZdkbMKCRpLuBGltpoFfWS == JlfhJIwWcZdkbMKCRpLuBGltpoFfWS ) JlfhJIwWcZdkbMKCRpLuBGltpoFfWS=923851197.343625714441069413055120163738; else JlfhJIwWcZdkbMKCRpLuBGltpoFfWS=1915747084.711226972472200415965642333466;int DogmQzqXReXGgpMtKzjaqpoODmvvsO=2013884637;if (DogmQzqXReXGgpMtKzjaqpoODmvvsO == DogmQzqXReXGgpMtKzjaqpoODmvvsO- 1 ) DogmQzqXReXGgpMtKzjaqpoODmvvsO=99455864; else DogmQzqXReXGgpMtKzjaqpoODmvvsO=118340473;if (DogmQzqXReXGgpMtKzjaqpoODmvvsO == DogmQzqXReXGgpMtKzjaqpoODmvvsO- 1 ) DogmQzqXReXGgpMtKzjaqpoODmvvsO=664614421; else DogmQzqXReXGgpMtKzjaqpoODmvvsO=1377910527;if (DogmQzqXReXGgpMtKzjaqpoODmvvsO == DogmQzqXReXGgpMtKzjaqpoODmvvsO- 0 ) DogmQzqXReXGgpMtKzjaqpoODmvvsO=1070120742; else DogmQzqXReXGgpMtKzjaqpoODmvvsO=1934644917;if (DogmQzqXReXGgpMtKzjaqpoODmvvsO == DogmQzqXReXGgpMtKzjaqpoODmvvsO- 0 ) DogmQzqXReXGgpMtKzjaqpoODmvvsO=36878949; else DogmQzqXReXGgpMtKzjaqpoODmvvsO=258859651;if (DogmQzqXReXGgpMtKzjaqpoODmvvsO == DogmQzqXReXGgpMtKzjaqpoODmvvsO- 1 ) DogmQzqXReXGgpMtKzjaqpoODmvvsO=277004413; else DogmQzqXReXGgpMtKzjaqpoODmvvsO=793647685;if (DogmQzqXReXGgpMtKzjaqpoODmvvsO == DogmQzqXReXGgpMtKzjaqpoODmvvsO- 0 ) DogmQzqXReXGgpMtKzjaqpoODmvvsO=119619401; else DogmQzqXReXGgpMtKzjaqpoODmvvsO=1327490539;double NoIqrtfCfJZEblnWrHGyneIQVdfCri=68357610.593423226124257338401081764328;if (NoIqrtfCfJZEblnWrHGyneIQVdfCri == NoIqrtfCfJZEblnWrHGyneIQVdfCri ) NoIqrtfCfJZEblnWrHGyneIQVdfCri=190919108.556457930645888777275270251277; else NoIqrtfCfJZEblnWrHGyneIQVdfCri=513240469.900967980141886721419725349630;if (NoIqrtfCfJZEblnWrHGyneIQVdfCri == NoIqrtfCfJZEblnWrHGyneIQVdfCri ) NoIqrtfCfJZEblnWrHGyneIQVdfCri=555488330.686475502199723540454097838563; else NoIqrtfCfJZEblnWrHGyneIQVdfCri=1544129550.946057551223032001390928769075;if (NoIqrtfCfJZEblnWrHGyneIQVdfCri == NoIqrtfCfJZEblnWrHGyneIQVdfCri ) NoIqrtfCfJZEblnWrHGyneIQVdfCri=447568314.704036487710934216583838302949; else NoIqrtfCfJZEblnWrHGyneIQVdfCri=932139819.858663911773468010566469398673;if (NoIqrtfCfJZEblnWrHGyneIQVdfCri == NoIqrtfCfJZEblnWrHGyneIQVdfCri ) NoIqrtfCfJZEblnWrHGyneIQVdfCri=1886820396.454896237130681057040359459947; else NoIqrtfCfJZEblnWrHGyneIQVdfCri=1674575377.635942068388343699935163321654;if (NoIqrtfCfJZEblnWrHGyneIQVdfCri == NoIqrtfCfJZEblnWrHGyneIQVdfCri ) NoIqrtfCfJZEblnWrHGyneIQVdfCri=1043345299.740161675019086202687111357262; else NoIqrtfCfJZEblnWrHGyneIQVdfCri=1892474357.242465938212493719722007863982;if (NoIqrtfCfJZEblnWrHGyneIQVdfCri == NoIqrtfCfJZEblnWrHGyneIQVdfCri ) NoIqrtfCfJZEblnWrHGyneIQVdfCri=1654647086.452387763238448583965955125560; else NoIqrtfCfJZEblnWrHGyneIQVdfCri=2129446820.315797111605669566039113860837;int JANsMOoeIZgFMfvMucggveREMzMuNi=218634883;if (JANsMOoeIZgFMfvMucggveREMzMuNi == JANsMOoeIZgFMfvMucggveREMzMuNi- 0 ) JANsMOoeIZgFMfvMucggveREMzMuNi=879788234; else JANsMOoeIZgFMfvMucggveREMzMuNi=1963495064;if (JANsMOoeIZgFMfvMucggveREMzMuNi == JANsMOoeIZgFMfvMucggveREMzMuNi- 1 ) JANsMOoeIZgFMfvMucggveREMzMuNi=674549956; else JANsMOoeIZgFMfvMucggveREMzMuNi=1094678725;if (JANsMOoeIZgFMfvMucggveREMzMuNi == JANsMOoeIZgFMfvMucggveREMzMuNi- 0 ) JANsMOoeIZgFMfvMucggveREMzMuNi=528917669; else JANsMOoeIZgFMfvMucggveREMzMuNi=2100149963;if (JANsMOoeIZgFMfvMucggveREMzMuNi == JANsMOoeIZgFMfvMucggveREMzMuNi- 0 ) JANsMOoeIZgFMfvMucggveREMzMuNi=1540877782; else JANsMOoeIZgFMfvMucggveREMzMuNi=54008684;if (JANsMOoeIZgFMfvMucggveREMzMuNi == JANsMOoeIZgFMfvMucggveREMzMuNi- 0 ) JANsMOoeIZgFMfvMucggveREMzMuNi=2120327651; else JANsMOoeIZgFMfvMucggveREMzMuNi=1427253175;if (JANsMOoeIZgFMfvMucggveREMzMuNi == JANsMOoeIZgFMfvMucggveREMzMuNi- 1 ) JANsMOoeIZgFMfvMucggveREMzMuNi=395438797; else JANsMOoeIZgFMfvMucggveREMzMuNi=142214527;float dpkaFJdORXMyynffXCaCBheHhuwlJx=456639235.612699486900005955806673320503f;if (dpkaFJdORXMyynffXCaCBheHhuwlJx - dpkaFJdORXMyynffXCaCBheHhuwlJx> 0.00000001 ) dpkaFJdORXMyynffXCaCBheHhuwlJx=333088585.041411319873977099895349304047f; else dpkaFJdORXMyynffXCaCBheHhuwlJx=855493587.999496079575603381634271880211f;if (dpkaFJdORXMyynffXCaCBheHhuwlJx - dpkaFJdORXMyynffXCaCBheHhuwlJx> 0.00000001 ) dpkaFJdORXMyynffXCaCBheHhuwlJx=1724827976.738299558213043175613341482872f; else dpkaFJdORXMyynffXCaCBheHhuwlJx=1054380275.686984751593443124981661850792f;if (dpkaFJdORXMyynffXCaCBheHhuwlJx - dpkaFJdORXMyynffXCaCBheHhuwlJx> 0.00000001 ) dpkaFJdORXMyynffXCaCBheHhuwlJx=576754636.779673827942412190837909112513f; else dpkaFJdORXMyynffXCaCBheHhuwlJx=1955473669.507093498140704925986006587056f;if (dpkaFJdORXMyynffXCaCBheHhuwlJx - dpkaFJdORXMyynffXCaCBheHhuwlJx> 0.00000001 ) dpkaFJdORXMyynffXCaCBheHhuwlJx=1566316323.132455643071280830639763851251f; else dpkaFJdORXMyynffXCaCBheHhuwlJx=1314799692.654301946048302582686753847960f;if (dpkaFJdORXMyynffXCaCBheHhuwlJx - dpkaFJdORXMyynffXCaCBheHhuwlJx> 0.00000001 ) dpkaFJdORXMyynffXCaCBheHhuwlJx=379003724.459242708090570157138968601256f; else dpkaFJdORXMyynffXCaCBheHhuwlJx=1137522691.408669026088469738480184360275f;if (dpkaFJdORXMyynffXCaCBheHhuwlJx - dpkaFJdORXMyynffXCaCBheHhuwlJx> 0.00000001 ) dpkaFJdORXMyynffXCaCBheHhuwlJx=334565228.266360292124125050907387742430f; else dpkaFJdORXMyynffXCaCBheHhuwlJx=1325130032.086283321054737707941037808972f;int pfGISqvSOhPwzYRfcDyVZdwvyZqSrW=1961445767;if (pfGISqvSOhPwzYRfcDyVZdwvyZqSrW == pfGISqvSOhPwzYRfcDyVZdwvyZqSrW- 1 ) pfGISqvSOhPwzYRfcDyVZdwvyZqSrW=1964223811; else pfGISqvSOhPwzYRfcDyVZdwvyZqSrW=2048468165;if (pfGISqvSOhPwzYRfcDyVZdwvyZqSrW == pfGISqvSOhPwzYRfcDyVZdwvyZqSrW- 0 ) pfGISqvSOhPwzYRfcDyVZdwvyZqSrW=447702305; else pfGISqvSOhPwzYRfcDyVZdwvyZqSrW=505152716;if (pfGISqvSOhPwzYRfcDyVZdwvyZqSrW == pfGISqvSOhPwzYRfcDyVZdwvyZqSrW- 0 ) pfGISqvSOhPwzYRfcDyVZdwvyZqSrW=27375987; else pfGISqvSOhPwzYRfcDyVZdwvyZqSrW=1596727739;if (pfGISqvSOhPwzYRfcDyVZdwvyZqSrW == pfGISqvSOhPwzYRfcDyVZdwvyZqSrW- 0 ) pfGISqvSOhPwzYRfcDyVZdwvyZqSrW=1686923588; else pfGISqvSOhPwzYRfcDyVZdwvyZqSrW=238252792;if (pfGISqvSOhPwzYRfcDyVZdwvyZqSrW == pfGISqvSOhPwzYRfcDyVZdwvyZqSrW- 0 ) pfGISqvSOhPwzYRfcDyVZdwvyZqSrW=1119846334; else pfGISqvSOhPwzYRfcDyVZdwvyZqSrW=501479646;if (pfGISqvSOhPwzYRfcDyVZdwvyZqSrW == pfGISqvSOhPwzYRfcDyVZdwvyZqSrW- 1 ) pfGISqvSOhPwzYRfcDyVZdwvyZqSrW=1571285546; else pfGISqvSOhPwzYRfcDyVZdwvyZqSrW=1167798786;int rKWvMNCuJhepmXmNsPXJmEBFOysryf=2074715761;if (rKWvMNCuJhepmXmNsPXJmEBFOysryf == rKWvMNCuJhepmXmNsPXJmEBFOysryf- 0 ) rKWvMNCuJhepmXmNsPXJmEBFOysryf=517577657; else rKWvMNCuJhepmXmNsPXJmEBFOysryf=1198007851;if (rKWvMNCuJhepmXmNsPXJmEBFOysryf == rKWvMNCuJhepmXmNsPXJmEBFOysryf- 1 ) rKWvMNCuJhepmXmNsPXJmEBFOysryf=1796358235; else rKWvMNCuJhepmXmNsPXJmEBFOysryf=998505407;if (rKWvMNCuJhepmXmNsPXJmEBFOysryf == rKWvMNCuJhepmXmNsPXJmEBFOysryf- 0 ) rKWvMNCuJhepmXmNsPXJmEBFOysryf=885697964; else rKWvMNCuJhepmXmNsPXJmEBFOysryf=2070010893;if (rKWvMNCuJhepmXmNsPXJmEBFOysryf == rKWvMNCuJhepmXmNsPXJmEBFOysryf- 1 ) rKWvMNCuJhepmXmNsPXJmEBFOysryf=1789363894; else rKWvMNCuJhepmXmNsPXJmEBFOysryf=1071798212;if (rKWvMNCuJhepmXmNsPXJmEBFOysryf == rKWvMNCuJhepmXmNsPXJmEBFOysryf- 0 ) rKWvMNCuJhepmXmNsPXJmEBFOysryf=397178741; else rKWvMNCuJhepmXmNsPXJmEBFOysryf=1507233911;if (rKWvMNCuJhepmXmNsPXJmEBFOysryf == rKWvMNCuJhepmXmNsPXJmEBFOysryf- 1 ) rKWvMNCuJhepmXmNsPXJmEBFOysryf=715675001; else rKWvMNCuJhepmXmNsPXJmEBFOysryf=1538878625;float CNycXFNSodqrtefTPHQQfSsStNkDbG=2065411875.435128360000982491291059749000f;if (CNycXFNSodqrtefTPHQQfSsStNkDbG - CNycXFNSodqrtefTPHQQfSsStNkDbG> 0.00000001 ) CNycXFNSodqrtefTPHQQfSsStNkDbG=874908871.609211589818078343224913649659f; else CNycXFNSodqrtefTPHQQfSsStNkDbG=581476677.036041177291258318150699789778f;if (CNycXFNSodqrtefTPHQQfSsStNkDbG - CNycXFNSodqrtefTPHQQfSsStNkDbG> 0.00000001 ) CNycXFNSodqrtefTPHQQfSsStNkDbG=325474363.394088303827713377644161945847f; else CNycXFNSodqrtefTPHQQfSsStNkDbG=906271871.251991837001543498642314524415f;if (CNycXFNSodqrtefTPHQQfSsStNkDbG - CNycXFNSodqrtefTPHQQfSsStNkDbG> 0.00000001 ) CNycXFNSodqrtefTPHQQfSsStNkDbG=668410880.467778682364174363865820604858f; else CNycXFNSodqrtefTPHQQfSsStNkDbG=869743984.742375597582638825027265064717f;if (CNycXFNSodqrtefTPHQQfSsStNkDbG - CNycXFNSodqrtefTPHQQfSsStNkDbG> 0.00000001 ) CNycXFNSodqrtefTPHQQfSsStNkDbG=429263248.493573290987639420298993258831f; else CNycXFNSodqrtefTPHQQfSsStNkDbG=883844493.189029927664515073665563051042f;if (CNycXFNSodqrtefTPHQQfSsStNkDbG - CNycXFNSodqrtefTPHQQfSsStNkDbG> 0.00000001 ) CNycXFNSodqrtefTPHQQfSsStNkDbG=1316053646.556091962050373458939627663617f; else CNycXFNSodqrtefTPHQQfSsStNkDbG=1486575184.909522963412272530161311404943f;if (CNycXFNSodqrtefTPHQQfSsStNkDbG - CNycXFNSodqrtefTPHQQfSsStNkDbG> 0.00000001 ) CNycXFNSodqrtefTPHQQfSsStNkDbG=2082017588.845615285260562845614465669822f; else CNycXFNSodqrtefTPHQQfSsStNkDbG=75274990.802178766745501286066943958106f;long TePQboZuWFrytnwufTTpQqegxjrHIw=781881502;if (TePQboZuWFrytnwufTTpQqegxjrHIw == TePQboZuWFrytnwufTTpQqegxjrHIw- 0 ) TePQboZuWFrytnwufTTpQqegxjrHIw=270203703; else TePQboZuWFrytnwufTTpQqegxjrHIw=407333114;if (TePQboZuWFrytnwufTTpQqegxjrHIw == TePQboZuWFrytnwufTTpQqegxjrHIw- 0 ) TePQboZuWFrytnwufTTpQqegxjrHIw=807552375; else TePQboZuWFrytnwufTTpQqegxjrHIw=1229473627;if (TePQboZuWFrytnwufTTpQqegxjrHIw == TePQboZuWFrytnwufTTpQqegxjrHIw- 0 ) TePQboZuWFrytnwufTTpQqegxjrHIw=406041188; else TePQboZuWFrytnwufTTpQqegxjrHIw=1701322544;if (TePQboZuWFrytnwufTTpQqegxjrHIw == TePQboZuWFrytnwufTTpQqegxjrHIw- 0 ) TePQboZuWFrytnwufTTpQqegxjrHIw=1075747803; else TePQboZuWFrytnwufTTpQqegxjrHIw=2041696565;if (TePQboZuWFrytnwufTTpQqegxjrHIw == TePQboZuWFrytnwufTTpQqegxjrHIw- 0 ) TePQboZuWFrytnwufTTpQqegxjrHIw=259104271; else TePQboZuWFrytnwufTTpQqegxjrHIw=1523387740;if (TePQboZuWFrytnwufTTpQqegxjrHIw == TePQboZuWFrytnwufTTpQqegxjrHIw- 0 ) TePQboZuWFrytnwufTTpQqegxjrHIw=107973975; else TePQboZuWFrytnwufTTpQqegxjrHIw=667625723;long KWsGjHYzfMqVGQAagQCzltAMovvEzR=904098584;if (KWsGjHYzfMqVGQAagQCzltAMovvEzR == KWsGjHYzfMqVGQAagQCzltAMovvEzR- 0 ) KWsGjHYzfMqVGQAagQCzltAMovvEzR=727468793; else KWsGjHYzfMqVGQAagQCzltAMovvEzR=1653357318;if (KWsGjHYzfMqVGQAagQCzltAMovvEzR == KWsGjHYzfMqVGQAagQCzltAMovvEzR- 1 ) KWsGjHYzfMqVGQAagQCzltAMovvEzR=292889915; else KWsGjHYzfMqVGQAagQCzltAMovvEzR=2102707992;if (KWsGjHYzfMqVGQAagQCzltAMovvEzR == KWsGjHYzfMqVGQAagQCzltAMovvEzR- 0 ) KWsGjHYzfMqVGQAagQCzltAMovvEzR=1419131506; else KWsGjHYzfMqVGQAagQCzltAMovvEzR=2108530764;if (KWsGjHYzfMqVGQAagQCzltAMovvEzR == KWsGjHYzfMqVGQAagQCzltAMovvEzR- 0 ) KWsGjHYzfMqVGQAagQCzltAMovvEzR=1406304881; else KWsGjHYzfMqVGQAagQCzltAMovvEzR=1795317320;if (KWsGjHYzfMqVGQAagQCzltAMovvEzR == KWsGjHYzfMqVGQAagQCzltAMovvEzR- 1 ) KWsGjHYzfMqVGQAagQCzltAMovvEzR=768744603; else KWsGjHYzfMqVGQAagQCzltAMovvEzR=912513742;if (KWsGjHYzfMqVGQAagQCzltAMovvEzR == KWsGjHYzfMqVGQAagQCzltAMovvEzR- 1 ) KWsGjHYzfMqVGQAagQCzltAMovvEzR=1829935408; else KWsGjHYzfMqVGQAagQCzltAMovvEzR=1546606427;long LbkPwXsUfRFwJDxKLnQntCvYTClgRq=2146144851;if (LbkPwXsUfRFwJDxKLnQntCvYTClgRq == LbkPwXsUfRFwJDxKLnQntCvYTClgRq- 1 ) LbkPwXsUfRFwJDxKLnQntCvYTClgRq=729135241; else LbkPwXsUfRFwJDxKLnQntCvYTClgRq=2081277146;if (LbkPwXsUfRFwJDxKLnQntCvYTClgRq == LbkPwXsUfRFwJDxKLnQntCvYTClgRq- 1 ) LbkPwXsUfRFwJDxKLnQntCvYTClgRq=1268674922; else LbkPwXsUfRFwJDxKLnQntCvYTClgRq=1358251621;if (LbkPwXsUfRFwJDxKLnQntCvYTClgRq == LbkPwXsUfRFwJDxKLnQntCvYTClgRq- 0 ) LbkPwXsUfRFwJDxKLnQntCvYTClgRq=1555679022; else LbkPwXsUfRFwJDxKLnQntCvYTClgRq=692020873;if (LbkPwXsUfRFwJDxKLnQntCvYTClgRq == LbkPwXsUfRFwJDxKLnQntCvYTClgRq- 1 ) LbkPwXsUfRFwJDxKLnQntCvYTClgRq=1659370687; else LbkPwXsUfRFwJDxKLnQntCvYTClgRq=1321069974;if (LbkPwXsUfRFwJDxKLnQntCvYTClgRq == LbkPwXsUfRFwJDxKLnQntCvYTClgRq- 1 ) LbkPwXsUfRFwJDxKLnQntCvYTClgRq=1653212521; else LbkPwXsUfRFwJDxKLnQntCvYTClgRq=431261445;if (LbkPwXsUfRFwJDxKLnQntCvYTClgRq == LbkPwXsUfRFwJDxKLnQntCvYTClgRq- 1 ) LbkPwXsUfRFwJDxKLnQntCvYTClgRq=647167600; else LbkPwXsUfRFwJDxKLnQntCvYTClgRq=1292506787;double QIBfXVHgFZVZIyeqgqgJgKoBCFIrWH=1168008683.790564586477118229511952134774;if (QIBfXVHgFZVZIyeqgqgJgKoBCFIrWH == QIBfXVHgFZVZIyeqgqgJgKoBCFIrWH ) QIBfXVHgFZVZIyeqgqgJgKoBCFIrWH=866423480.195572160314399115710713185953; else QIBfXVHgFZVZIyeqgqgJgKoBCFIrWH=1177919004.091249265716930776403006548359;if (QIBfXVHgFZVZIyeqgqgJgKoBCFIrWH == QIBfXVHgFZVZIyeqgqgJgKoBCFIrWH ) QIBfXVHgFZVZIyeqgqgJgKoBCFIrWH=1495289844.732832315105011489342055100580; else QIBfXVHgFZVZIyeqgqgJgKoBCFIrWH=878314452.191608520956131800627002888774;if (QIBfXVHgFZVZIyeqgqgJgKoBCFIrWH == QIBfXVHgFZVZIyeqgqgJgKoBCFIrWH ) QIBfXVHgFZVZIyeqgqgJgKoBCFIrWH=105627622.384601451397959693531007410092; else QIBfXVHgFZVZIyeqgqgJgKoBCFIrWH=822874325.195767364156933026330624349820;if (QIBfXVHgFZVZIyeqgqgJgKoBCFIrWH == QIBfXVHgFZVZIyeqgqgJgKoBCFIrWH ) QIBfXVHgFZVZIyeqgqgJgKoBCFIrWH=1510233643.256931330333959527450476357143; else QIBfXVHgFZVZIyeqgqgJgKoBCFIrWH=309156586.865590001380611778692348088178;if (QIBfXVHgFZVZIyeqgqgJgKoBCFIrWH == QIBfXVHgFZVZIyeqgqgJgKoBCFIrWH ) QIBfXVHgFZVZIyeqgqgJgKoBCFIrWH=1148511880.429857956952627186669966163752; else QIBfXVHgFZVZIyeqgqgJgKoBCFIrWH=1159718702.304497197493274173100261056479;if (QIBfXVHgFZVZIyeqgqgJgKoBCFIrWH == QIBfXVHgFZVZIyeqgqgJgKoBCFIrWH ) QIBfXVHgFZVZIyeqgqgJgKoBCFIrWH=460729024.421576933100028689309175215354; else QIBfXVHgFZVZIyeqgqgJgKoBCFIrWH=1641011093.182807281835010134448727046637;long eqsyNlLHCVvtVnWAfYHoDsDgVfZoeH=692499014;if (eqsyNlLHCVvtVnWAfYHoDsDgVfZoeH == eqsyNlLHCVvtVnWAfYHoDsDgVfZoeH- 0 ) eqsyNlLHCVvtVnWAfYHoDsDgVfZoeH=835777651; else eqsyNlLHCVvtVnWAfYHoDsDgVfZoeH=109312771;if (eqsyNlLHCVvtVnWAfYHoDsDgVfZoeH == eqsyNlLHCVvtVnWAfYHoDsDgVfZoeH- 1 ) eqsyNlLHCVvtVnWAfYHoDsDgVfZoeH=2085974129; else eqsyNlLHCVvtVnWAfYHoDsDgVfZoeH=947259239;if (eqsyNlLHCVvtVnWAfYHoDsDgVfZoeH == eqsyNlLHCVvtVnWAfYHoDsDgVfZoeH- 0 ) eqsyNlLHCVvtVnWAfYHoDsDgVfZoeH=1026853688; else eqsyNlLHCVvtVnWAfYHoDsDgVfZoeH=1409383730;if (eqsyNlLHCVvtVnWAfYHoDsDgVfZoeH == eqsyNlLHCVvtVnWAfYHoDsDgVfZoeH- 1 ) eqsyNlLHCVvtVnWAfYHoDsDgVfZoeH=1495903209; else eqsyNlLHCVvtVnWAfYHoDsDgVfZoeH=35447920;if (eqsyNlLHCVvtVnWAfYHoDsDgVfZoeH == eqsyNlLHCVvtVnWAfYHoDsDgVfZoeH- 1 ) eqsyNlLHCVvtVnWAfYHoDsDgVfZoeH=2141470278; else eqsyNlLHCVvtVnWAfYHoDsDgVfZoeH=1119303588;if (eqsyNlLHCVvtVnWAfYHoDsDgVfZoeH == eqsyNlLHCVvtVnWAfYHoDsDgVfZoeH- 0 ) eqsyNlLHCVvtVnWAfYHoDsDgVfZoeH=449495471; else eqsyNlLHCVvtVnWAfYHoDsDgVfZoeH=987870061; }
 eqsyNlLHCVvtVnWAfYHoDsDgVfZoeHy::eqsyNlLHCVvtVnWAfYHoDsDgVfZoeHy()
 { this->XMPttSRlFHGl("cTjhhGCmQzLRvsojwbTvGEsBmnMchLXMPttSRlFHGlj", true, 1461363164, 113798094, 1033547311); }
#pragma optimize("", off)
 // <delete/>

