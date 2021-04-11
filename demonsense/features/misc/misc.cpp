#include "../../valve_sdk/csgostructs.h"
#include "misc.h"
#include "../../options/options.h"
#include "../../features/notify/notify.h"
#include "../../helpers/input.h"
#include "../../helpers/math.h"

bool backward_style = false;
static bool enable = false;
static bool jumped = false;
char localtag[16];
bool is_clantag_copied = false;




namespace misc
{
	void prepare_revolver(c_usercmd* cmd)
	{
		if (!g_local_player || !g_engine_client->is_connected() || !g_local_player->m_hActiveWeapon() || !g_local_player->is_alive())
			return;

		constexpr auto timeToTicks = [](float time) {  return static_cast<int>(0.5f + time / g_global_vars->interval_per_tick); };
		constexpr float revolverPrepareTime{ 0.234375f };

		static float readyTime;
		if (settings::misc::prepare_revolver::bind.enable)
		{
			const auto activeWeapon = g_local_player->m_hActiveWeapon();
			if (activeWeapon && activeWeapon->get_item_definition_index() == ItemDefinitionIndex::WEAPON_REVOLVER)
			{
				if (!readyTime) readyTime = utils::get_curtime(cmd) + revolverPrepareTime;

				auto ticksToReady = timeToTicks(readyTime - utils::get_curtime(cmd) - g_engine_client->get_net_channel_info()->get_latency(FLOW_OUTGOING));
				if (ticksToReady > 0 && ticksToReady <= timeToTicks(revolverPrepareTime))
					cmd->buttons |= IN_ATTACK;
				else
					readyTime = 0.0f;
			}
		}
	}

	// MOVEMENT

	void jumpbug(c_usercmd* cmd)
	{
		float max_radias = D3DX_PI * 2;
		float step = max_radias / 128;
		float xThick = 23;
		//if (settings::misc::jumpbug::enable && GetAsyncKeyState(settings::misc::jumpbug::key)) {
		if (settings::misc::jumpbug::enable && settings::misc::jumpbug::bind.enable) {
			if (g_local_player->m_fFlags() & FL_ONGROUND) {
				settings::misc::bhop::enable = false;
				bool unduck = cmd->buttons &= ~IN_DUCK;
				if (unduck) {
					cmd->buttons &= ~IN_DUCK; // duck
					cmd->buttons |= IN_JUMP; // jump
					unduck = false;
				}
				Vector pos = g_local_player->get_abs_origin();
				for (float a = 0.f; a < max_radias; a += step) {
					Vector pt;
					pt.x = (xThick * cos(a)) + pos.x;
					pt.y = (xThick * sin(a)) + pos.y;
					pt.z = pos.z;


					Vector pt2 = pt;
					pt2.z -= 8192;

					trace_t fag;

					ray_t ray;
					ray.init(pt, pt2);

					c_tracefilter flt;
					flt.pSkip = g_local_player;
					g_engine_trace->trace_ray(ray, MASK_PLAYERSOLID, &flt, &fag);

					if (fag.fraction != 1.f && fag.fraction != 0.f) {
						cmd->buttons &= IN_DUCK; // duck
						cmd->buttons |= ~IN_JUMP; // jump
						unduck = true;
					}
				}
				for (float a = 0.f; a < max_radias; a += step) {
					Vector pt;
					pt.x = ((xThick - 2.f) * cos(a)) + pos.x;
					pt.y = ((xThick - 2.f) * sin(a)) + pos.y;
					pt.z = pos.z;

					Vector pt2 = pt;
					pt2.z -= 8192;

					trace_t fag;

					ray_t ray;
					ray.init(pt, pt2);

					c_tracefilter flt;
					flt.pSkip = g_local_player;
					g_engine_trace->trace_ray(ray, MASK_PLAYERSOLID, &flt, &fag);

					if (fag.fraction != 1.f && fag.fraction != 0.f) {
						cmd->buttons |= IN_DUCK; // duck
						cmd->buttons &= ~IN_JUMP; // jump
						unduck = true;
					}
				}
				for (float a = 0.f; a < max_radias; a += step) {
					Vector pt;
					pt.x = ((xThick - 20.f) * cos(a)) + pos.x;
					pt.y = ((xThick - 20.f) * sin(a)) + pos.y;
					pt.z = pos.z;

					Vector pt2 = pt;
					pt2.z -= 8192;

					trace_t fag;

					ray_t ray;
					ray.init(pt, pt2);

					c_tracefilter flt;
					flt.pSkip = g_local_player;
					g_engine_trace->trace_ray(ray, MASK_PLAYERSOLID, &flt, &fag);

					if (fag.fraction != 1.f && fag.fraction != 0.f) {
						cmd->buttons |= IN_DUCK; // duck
						cmd->buttons &= ~IN_JUMP; // jump
						unduck = true;
					}
				}
			}
			//else settings::misc::bhop::enable = true;

		}
		else settings::misc::bhop::enable = true;



		/*float max_radias = M_PI * 2;
		float step = max_radias / 128;
		float xThick = 23;
		auto unduck = true;
		bool bDidJump;
		static bool needCHangeBhop = false;
		
		if (!settings::misc::jumpbug::bind.enable || !settings::misc::jumpbug::enable)
			return;


		if (settings::misc::jumpbug::bind.enable && (g_local_player->m_fFlags() & (1 << 0))) {
			if (settings::misc::bhop::enable) {
				settings::misc::bhop::enable = false;
				needCHangeBhop = true;
			}
			int screenWidth, screenHeight;
			g_engine_client->get_screen_size(screenWidth, screenHeight);

			if (unduck) {
				bool bDidJump = false;
				cmd->buttons &= ~IN_DUCK; // duck
				cmd->buttons |= IN_JUMP; // jump
				unduck = false;
			}
			Vector pos = g_local_player->m_vecOrigin();
			for (float a = 0.f; a < max_radias; a += step) {
				Vector pt;
				pt.x = (xThick * cos(a)) + pos.x;
				pt.y = (xThick * sin(a)) + pos.y;
				pt.z = pos.z;


				Vector pt2 = pt;
				pt2.z -= 6;

				trace_t fag;

				ray_t ray;
				ray.init(pt, pt2);

				c_tracefilter flt;
				flt.pSkip = g_local_player;
				g_engine_trace->trace_ray(ray, MASK_SOLID_BRUSHONLY, &flt, &fag);

				if (fag.fraction != 1.f && fag.fraction != 0.f) {
					bDidJump = true;
					cmd->buttons |= IN_DUCK;
					cmd->buttons &= ~IN_JUMP;
					unduck = true;
				}
			}
			for (float a = 0.f; a < max_radias; a += step) {
				Vector pt;
				pt.x = ((xThick - 2.f) * cos(a)) + pos.x;
				pt.y = ((xThick - 2.f) * sin(a)) + pos.y;
				pt.z = pos.z;

				Vector pt2 = pt;
				pt2.z -= 6;

				trace_t fag;

				ray_t ray;
				ray.init(pt, pt2);

				c_tracefilter flt;
				flt.pSkip = g_local_player;
				g_engine_trace->trace_ray(ray, MASK_SOLID_BRUSHONLY, &flt, &fag);

				if (fag.fraction != 1.f && fag.fraction != 0.f) {
					bDidJump = true;
					cmd->buttons |= IN_DUCK; // duck
					cmd->buttons &= ~IN_JUMP; // jump
					unduck = true;
				}
			}
			for (float a = 0.f; a < max_radias; a += step) {
				Vector pt;
				pt.x = ((xThick - 20.f) * cos(a)) + pos.x;
				pt.y = ((xThick - 20.f) * sin(a)) + pos.y;
				pt.z = pos.z;

				Vector pt2 = pt;
				pt2.z -= 6;

				trace_t fag;

				ray_t ray;
				ray.init(pt, pt2);

				c_tracefilter flt;
				flt.pSkip = g_local_player;
				g_engine_trace->trace_ray(ray, MASK_SOLID_BRUSHONLY, &flt, &fag);

				if (fag.fraction != 1.f && fag.fraction != 0.f) {
					bDidJump = true;
					cmd->buttons |= IN_DUCK; // duck
					cmd->buttons &= ~IN_JUMP; // jump
					unduck = true;
				}
			}
		}
		else if (needCHangeBhop) {
			settings::misc::bhop::enable = true;
			needCHangeBhop = false;
		}*/
		

	}

	void edgebug(c_usercmd* cmd)
	{
	  if ((g_local_player->m_fFlags() & FL_ONGROUND) && g_local_player->is_alive())
	 // if (settings::misc::edge_bug && GetAsyncKeyState(settings::misc::edge_bug_key)) cmd->buttons = 4;
	  if (settings::misc::edge_bug && settings::misc::edgebugkey.enable) cmd->buttons = 4;
		
	}

	void runboost(c_usercmd* cmd)
	{
		//if (settings::misc::autorunbst::enable && GetAsyncKeyState(settings::misc::autorunbst::key)) {
		if (settings::misc::autorunbst::enable && settings::misc::autorunbst::bind.enable) {
			for (int i = 1; i <= g_engine_client->get_max_clients(); ++i)
			{
				// Bla bla bla validity checks.
				c_base_entity* pEntity;
				c_base_player* pLocal;
				const auto pLocalEntity = c_base_player::get_player_by_index(g_engine_client->get_local_player());
				pEntity = (c_base_entity*)g_entity_list->get_client_entity(i);
				if (!pEntity
					|| !pLocal->is_alive()
					|| pEntity->is_dormant())
					continue;

				if (pEntity == pLocalEntity)
					continue;

				c_base_entity* GroundEntity = (c_base_entity*)g_entity_list->get_client_entity_from_handle(pLocalEntity->m_hGroundEntity());

				// Check if there's a player under us.
				if (GroundEntity && GroundEntity == pEntity)
				{
					// Get the target's speed.
					auto Velocity = pLocal->m_vecVelocity();
					const auto Speed = Velocity.Length2D();

					if (Speed > 0.0f)
					{
						// Get the angles direction based on the target's speed.
						Vector Direction;
						QAngle Directions;
						math::vector_angles(Velocity, Directions);

						QAngle* ViewAngles;
						Vector ViewAngle;
						g_engine_client->get_view_angles(ViewAngles);

						// Cut down on our viewangles.
						Direction.y = ViewAngle.y - Direction.y;

						// Transform into vector again.
						Vector Forward;
						math::vector_angles(Forward, Directions);

						// Calculate the new direction based on the target's speed.
						Vector NewDirection = Forward * Speed;

						// Move accordingly.
						cmd->forwardmove = NewDirection.x;
						cmd->sidemove = NewDirection.y;
					}
				}
			}
		}

	}

	void edge_jump(c_usercmd* cmd, int old_flags)
	{
		if (!g_engine_client->is_connected())
			return;
		if (!g_local_player || !g_local_player->is_alive())
			return;

		if (settings::misc::edge_jump::enable)
		{
			if (settings::misc::edge_jump::bind.bind_type == KeyBind_type::press)
			{
				if (settings::misc::edge_jump::bind.enable)
				{
					if ((old_flags & FL_ONGROUND) && !(g_local_player->m_fFlags() & FL_ONGROUND))
					{
						cmd->buttons |= IN_JUMP;
						jumped = true;
					}
				}

				if (settings::misc::edge_jump::bind.enable && settings::misc::edge_jump::auto_duck && jumped)
				{
					cmd->buttons |= IN_DUCK;
				}

				if ((old_flags & FL_ONGROUND) && (g_local_player->m_fFlags() & FL_ONGROUND))
				{
					jumped = false;
				}
			}
			if (settings::misc::edge_jump::bind.bind_type == KeyBind_type::toggle)
			{
				if (settings::misc::edge_jump::bind.enable)
				{
					if ((old_flags & FL_ONGROUND) && !(g_local_player->m_fFlags() & FL_ONGROUND))
					{
						cmd->buttons |= IN_JUMP;
						jumped = true;
					}

					if (settings::misc::edge_jump::auto_duck && jumped)
					{
						cmd->buttons |= IN_DUCK;
					}

					if ((old_flags & FL_ONGROUND) && (g_local_player->m_fFlags() & FL_ONGROUND))
					{
						jumped = false;
					}
				}
			}
		}
	}


    // OTHERS

	void KnifeLeft()
	{
		static auto left_knife = g_cvar->find_var("cl_righthand");

		if (!g_local_player || !g_local_player->is_alive())
		{
			left_knife->set_value(1);
			return;
		}

		auto weapon = g_local_player->m_hActiveWeapon();
		if (!weapon) return;

		left_knife->set_value(!weapon->is_knife());
	}

	void moon_walk(c_usercmd* cmd)
	{
		if (settings::misc::moon_walk)
		{
			if (g_local_player->m_nMoveType() == MOVETYPE_NOCLIP)
				return;

			if (g_local_player->m_nMoveType() == MOVETYPE_LADDER)
				return;

			if (!(g_local_player->m_fFlags() & FL_ONGROUND))
				return;

			if (cmd->buttons & IN_FORWARD)
			{
				cmd->forwardmove = 450;
				cmd->buttons &= ~IN_FORWARD;
				cmd->buttons |= IN_BACK;
			}
			else if (cmd->buttons & IN_BACK)
			{
				cmd->forwardmove = -450;
				cmd->buttons &= ~IN_BACK;
				cmd->buttons |= IN_FORWARD;
			}

			if (cmd->buttons & IN_MOVELEFT)
			{
				cmd->sidemove = -450;
				cmd->buttons &= ~IN_MOVELEFT;
				cmd->buttons |= IN_MOVERIGHT;
			}
			else if (cmd->buttons & IN_MOVERIGHT)
			{
				cmd->sidemove = 450;
				cmd->buttons &= ~IN_MOVERIGHT;
				cmd->buttons |= IN_MOVELEFT;
			}
		}
	}

	void block_bot(c_usercmd* cmd)
	{
		if (!settings::misc::block_bot::bind.enable || !settings::misc::block_bot::enable)
			return;

		float bestdist = 200.f;
		int index = -1;

		for (int i = 1; i < 64; i++)
		{
			c_base_player* entity = (c_base_player*)g_entity_list->get_client_entity(i);

			if (!entity)
				continue;

			if (!entity->is_alive() || entity->is_dormant() || entity == g_local_player)
				continue;

			float dist = g_local_player->get_abs_origin().DistTo(entity->get_abs_origin());

			if (dist < bestdist)
			{
				bestdist = dist;
				index = i;
			}
		}

		if (index == -1)
			return;

		c_base_player* target = (c_base_player*)g_entity_list->get_client_entity(index);

		if (!target)
			return;

		{

			QAngle angles = math::calc_angle(g_local_player->get_abs_origin(), target->get_abs_origin());

			QAngle shit;

			g_engine_client->get_view_angles(&shit);

			angles.yaw -= shit.yaw;
			math::fix_angles(angles);

			if (angles.yaw < 0.20f)
				cmd->sidemove = 450.f;
			else if (angles.yaw > 0.20f)
				cmd->sidemove = -450.f;
		}
	}

	void fast_duck(c_usercmd* cmd)
	{
		if (settings::misc::fast_duck)
			cmd->buttons |= IN_BULLRUSH;
	}

	void fog()
	{
		static auto fog_enable = g_cvar->find_var("fog_enable");
		static auto fog_override = g_cvar->find_var("fog_override");
		static auto fog_color = g_cvar->find_var("fog_color");
		static auto fog_start = g_cvar->find_var("fog_start");
		static auto fog_end = g_cvar->find_var("fog_end");
		static auto fog_destiny = g_cvar->find_var("fog_maxdensity");

		static const auto fog_enable_bec = fog_enable->get_bool();
		static const auto fog_override_bec = fog_override->get_bool();
		static const auto fog_color_bec = fog_color->get_string();
		static const auto fog_start_bec = fog_start->get_float();
		static const auto fog_end_bec = fog_end->get_float();
		static const auto fog_destiny_bec = fog_destiny->get_float();

		if (settings::misc::fog::enable && !g_unload)
		{
			fog_enable->m_fnChangeCallbacks.m_Size = 0;
			fog_override->m_fnChangeCallbacks.m_Size = 0;
			fog_color->m_fnChangeCallbacks.m_Size = 0;
			fog_start->m_fnChangeCallbacks.m_Size = 0;
			fog_end->m_fnChangeCallbacks.m_Size = 0;
			fog_destiny->m_fnChangeCallbacks.m_Size = 0;

			fog_enable->set_value(settings::misc::fog::enable); //fog enable
			fog_override->set_value(settings::misc::fog::enable); //fog bypass
			fog_color->set_value(std::string(std::to_string(settings::misc::fog::color.r()) + " " + std::to_string(settings::misc::fog::color.g()) + " " + std::to_string(settings::misc::fog::color.b())).c_str()); //fog color
			fog_start->set_value(settings::misc::fog::start_dist); // fog start dist
			fog_end->set_value(settings::misc::fog::end_dist); // fog end dist
			fog_destiny->set_value(settings::misc::fog::color.save_color[3]); //max alpha fog
		}
		else if (g_unload || !settings::misc::fog::enable)
		{
			fog_enable->m_fnChangeCallbacks.m_Size = 0;
			fog_override->m_fnChangeCallbacks.m_Size = 0;
			fog_color->m_fnChangeCallbacks.m_Size = 0;
			fog_start->m_fnChangeCallbacks.m_Size = 0;
			fog_end->m_fnChangeCallbacks.m_Size = 0;
			fog_destiny->m_fnChangeCallbacks.m_Size = 0;

			fog_enable->set_value(fog_enable_bec); //fog enable
			fog_override->set_value(fog_override_bec); //fog bypass
			fog_color->set_value(fog_color_bec); //fog color
			fog_start->set_value(fog_start_bec); // fog start dist
			fog_end->set_value(fog_end_bec); // fog end dist
			fog_destiny->set_value(fog_destiny_bec); //max alpha fog
		}
	}

	void no_smoke()
	{
		static DWORD smoke_count;
		static uint8_t* offset;

		if (!offset)
			offset = utils::pattern_scan(GetModuleHandleW(L"client.dll"), "55 8B EC 83 EC 08 8B 15 ? ? ? ? 0F 57 C0");

		if (!smoke_count)
			smoke_count = *reinterpret_cast<DWORD*>(offset + 0x8);

		if (settings::misc::disable_smoke)
			*reinterpret_cast<int*>(smoke_count) = 0;

		static bool set = true;
		static std::vector<const char*> smoke_materials =
		{
			"particle/vistasmokev1/vistasmokev1_fire",
			"particle/vistasmokev1/vistasmokev1_smokegrenade",
			"particle/vistasmokev1/vistasmokev1_emods",
			"particle/vistasmokev1/vistasmokev1_emods_impactdust",
		};

		if (!settings::misc::disable_smoke || g_unload)
		{
			if (set)
			{
				for (auto material_name : smoke_materials)
				{
					i_material* mat = g_mat_system->find_material(material_name, TEXTURE_GROUP_OTHER);
					mat->set_material_var_flag(MATERIAL_VAR_WIREFRAME, false);
				}
				set = false;
			}
			return;
		}

		set = true;
		for (auto material_name : smoke_materials)
		{
			i_material* mat = g_mat_system->find_material(material_name, TEXTURE_GROUP_OTHER);
			mat->set_material_var_flag(MATERIAL_VAR_WIREFRAME, true);
		}
	}

	void disable_flash_alpha()
	{
		if (settings::misc::disable_flash)
			g_local_player->m_flFlashMaxAlpha() = settings::misc::flash_alpha;
		else
			g_local_player->m_flFlashMaxAlpha() = 255.f;
	}

	namespace clan_tag
	{
		int user_index;

		size_t pos = 0;
		std::string clantag;
		float last_time = 0;

		void init()
		{
			//auto player_resource = *g_player_resource;

			if (!settings::misc::clantag::enable || !g_local_player)
				return;

			//auto usertag_loclal = std::string(player_resource->GetClanTag()[g_local_player->GetIndex()]);

			static std::string local_tag;

			int tick = int(g_global_vars->curtime * 2.4);

			if (settings::misc::clantag::clantag_type == 0)
			{

				switch (settings::misc::clantag::animation_type)
				{
				case 0:
					switch (tick % 18)
					{
						//                 harakiri.cc 8
					case 0:	utils::set_clantag("N3WProject"); break;
					case 1:	utils::set_clantag("-3WProject-"); break;
					case 2:	utils::set_clantag("=-WProject-="); break;
					case 3:	utils::set_clantag("==-Project-=="); break;
					case 4:	utils::set_clantag("===-roject-==="); break;
					case 5:	utils::set_clantag("====-oject-===="); break;
					case 6:	utils::set_clantag("=====ject====="); break;
					case 7:	utils::set_clantag("-====ect====-"); break;
					case 8:	utils::set_clantag("-===ct===-"); break;
					case 9: utils::set_clantag("-===t===-"); break;
					case 10:	utils::set_clantag("-===ct===-"); break;
					case 11:	utils::set_clantag("-====ect====-"); break;
					case 12: utils::set_clantag("=====ject====="); break;
					case 13: utils::set_clantag("====-oject-===="); break;
					case 14:	utils::set_clantag("===-roject-==="); break;
					case 15:	utils::set_clantag("==-Project-=="); break;
					case 16:	utils::set_clantag("=-WProject-="); break;
					case 17:	utils::set_clantag("-3WProject-"); break;
					case 18:	utils::set_clantag("N3WProject"); break;











					}


					break;
				case 1:
                    switch (tick % 24)
                    {
                    case 0:	utils::set_clantag("                ga"); break;
                    case 1:	utils::set_clantag("               ga"); break;
                    case 2:	utils::set_clantag("              gam"); break;
                    case 3:	utils::set_clantag("             game"); break;
                    case 4:	utils::set_clantag("            games"); break;
                    case 5:	utils::set_clantag("           gamese"); break;
                    case 6:	utils::set_clantag("          gamesen"); break;
                    case 7:	utils::set_clantag("         gamesens"); break;
                    case 8:	utils::set_clantag("        gamesense"); break;
                    case 9:	utils::set_clantag("       gamesense "); break;
                    case 10: utils::set_clantag("     gamesense  "); break;
                    case 11: utils::set_clantag("    gamesense   "); break;
                    case 12: utils::set_clantag("   gamesense    "); break;
                    case 13: utils::set_clantag("  gamesense     "); break;
                    case 14: utils::set_clantag(" gamesense     "); break;
                    case 15: utils::set_clantag("gamesense     "); break;
                    case 16: utils::set_clantag("amesense     "); break;
                    case 17: utils::set_clantag("mesense     "); break;
                    case 18: utils::set_clantag("esense     "); break;
                    case 19: utils::set_clantag("sense     "); break;
                    case 20: utils::set_clantag("ense     "); break;
                    case 21: utils::set_clantag("nse     "); break;
                    case 22: utils::set_clantag("se     "); break;
                    case 23: utils::set_clantag("e     "); break;
                    case 24: utils::set_clantag("    "); break;
                    }
					break;
				case 2:
					switch (tick % 13)
					{
					case 0:  utils::set_clantag("AIMWARE.net"); break; //arrumado
					case 1:  utils::set_clantag("IMWARE.net "); break;
					case 2:  utils::set_clantag("MWARE.net A"); break;
					case 3:  utils::set_clantag("WARE.net AI"); break;
					case 4:  utils::set_clantag("ARE.net AIM"); break;
					case 5:  utils::set_clantag("RE.net AIMW"); break;
					case 6:  utils::set_clantag("E.net AIMWA"); break;
					case 7:  utils::set_clantag(".net AIMWAR"); break;
					case 8:  utils::set_clantag("net AIMWARE"); break;
					case 9:  utils::set_clantag("et AIMWARE."); break;
					case 10: utils::set_clantag("t AIMWARE.n"); break;
					case 11: utils::set_clantag(" AIMWARE.ne"); break;
					case 12: utils::set_clantag("AIMWARE.net"); break;
					case 13: utils::set_clantag("AIMWARE.net"); break;
					}
					break;
				case 3:
					switch (tick % 12)
					{
					case 0:   utils::set_clantag("Onetap"); break;
					case 1:   utils::set_clantag("Onetap"); break;
					case 2:   utils::set_clantag("Onetap"); break;
					case 3:   utils::set_clantag("Onetap"); break;
					case 4:   utils::set_clantag("Onetap"); break;
					case 5:   utils::set_clantag("Onetap"); break;
					case 6:   utils::set_clantag("Onetap"); break;
					case 7:   utils::set_clantag("Onetap"); break;
					case 8:   utils::set_clantag("Onetap"); break;
					case 9:   utils::set_clantag("Onetap"); break;
					case 10:  utils::set_clantag("Onetap"); break;
					case 11:  utils::set_clantag("Onetap"); break;
					case 12:  utils::set_clantag("Onetap"); break;
					}
					break;
                case 4:
                    switch (tick % 10)
                    {
                    case 0:   utils::set_clantag("I ");
                    case 1:   utils::set_clantag("IN ");
                    case 2:   utils::set_clantag("INI ");
                    case 3:   utils::set_clantag("INIU ");
                    case 4:   utils::set_clantag("INIU  ");
                    case 5:   utils::set_clantag("INIUR  ");
                    case 6:   utils::set_clantag("INIURI  ");
                    case 7:   utils::set_clantag("INIURIA  ");
                    case 8:   utils::set_clantag("INIURIA.  ");
                    case 9:   utils::set_clantag("INIURIA.U  ");
                    case 10:  utils::set_clantag("INIURIA.US  ");
                    }
                    break;
                case 5:
                    switch (tick % 10)
                    {
                    case 0:   utils::set_clantag("PO ");
                    case 1:   utils::set_clantag("POLAK ");
                    case 2:   utils::set_clantag("POLAKPAS ");
                    case 3:   utils::set_clantag("POLAKPASTE. ");
                    case 4:   utils::set_clantag("POLAKPASTE.WA ");
                    case 5:   utils::set_clantag("POLAKPASTE.WAVE");
                    case 6:   utils::set_clantag("POLAKPASTE.WA");
                    case 7:   utils::set_clantag("POLAKPASTE. ");
                    case 8:   utils::set_clantag("POLAKPAS ");
                    case 9:   utils::set_clantag("POLAK ");
                    case 10:   utils::set_clantag("PO ");
                    //case 7:   utils::set_clantag("PO  ");

                    //case 8:   utils::set_clantag("INIURIA.  ");
                   // case 9:   utils::set_clantag("INIURIA.U  ");
                   // case 10:  utils::set_clantag("INIURIA.US  ");
                    }
                    break;
                case 6:
                    switch (tick % 8)
                    {
                    case 0:   utils::set_clantag("KO ");
                    case 1:   utils::set_clantag("KOEZ ");
                    case 2:   utils::set_clantag("KOEZ. ");
                    case 3:   utils::set_clantag("KOEZ.PAS ");
                    case 4:   utils::set_clantag("KOEZ.PASTE ");
                    case 5:   utils::set_clantag("KOEZ.PAS");
                    case 6:   utils::set_clantag("KOEZ.");
                    case 7:   utils::set_clantag("KOEZ ");
                    case 8:   utils::set_clantag("KO ");
                   // case 9:   utils::set_clantag("POLAK ");
                   // case 10:   utils::set_clantag("PO ");
                        //case 7:   utils::set_clantag("PO  ");

                        //case 8:   utils::set_clantag("INIURIA.  ");
                       // case 9:   utils::set_clantag("INIURIA.U  ");
                       // case 10:  utils::set_clantag("INIURIA.US  ");
                    }
                    break;

               /* case 4:
                    switch (tick % 0)
                    {
                    case 0:   utils::set_clantag("onetap");
                    }
                    break;*/
				}
			}//arrumado
			else if (settings::misc::clantag::clantag_type == 1)
			{
				if (settings::misc::clantag::clantag.empty() || settings::misc::clantag::clantag.length() == 0)
					return;

				if (g_local_player || !settings::misc::clantag::custom_type)
					return;
				{
					if (clantag != settings::misc::clantag::clantag || clantag.length() < pos)
					{
						clantag = settings::misc::clantag::clantag;
						pos = 0;
					}

					if (last_time + settings::misc::clantag::speed > g_global_vars->realtime)
						return;

					last_time = g_global_vars->realtime;

					utils::set_clantag(clantag.substr(0, pos).c_str());
					pos++;
					
				}
				/*else if (settings::misc::clantag::custom_type == 2)
				{

					static float LastChangeTime = 0.f;

					static bool restore = false;

					if (true)
					{
						if (g_global_vars->realtime - LastChangeTime >= settings::misc::clantag::speed)
						{
							settings::misc::clantag::clantag_visible;

							LastChangeTime = g_global_vars->realtime;

							std::string temp = settings::misc::clantag::clantag_visible;
							settings::misc::clantag::clantag_visible.erase(0, 1);
							settings::misc::clantag::clantag_visible += temp[0];

							utils::set_clantag(settings::misc::clantag::clantag_visible.data());

							restore = true;
						}
					}
					else if (restore)
					{
						restore = false;
						utils::set_clantag("");
					}
				}*/
			}
			else if (settings::misc::clantag::clantag_type == 2)
			{
				static int count;
				if (user_index == 0)
					return;

				auto player_resource = *g_player_resource;

				auto* player = c_base_player::get_player_by_index(user_index);

				if (!player)
					return;

				const auto info = player->get_player_info();
				if (info.fakeplayer)
					return;

				auto usertag = std::string(player_resource->szclantag()[player->get_index()]);

				static float LastChangeTime = 0.f;

				static bool restore = false;

				if (true)
				{
					if (g_global_vars->realtime - LastChangeTime >= 0.0001)
					{
						LastChangeTime = g_global_vars->realtime;

						utils::set_clantag(usertag.c_str());

						restore = true;
					}
				}
				else if (restore)
				{
					restore = false;
					utils::set_clantag("");
				}
			}
		}
	}

	namespace performance
	{
		bool* disablePostProcessing = *(bool**)(utils::pattern_scan(GetModuleHandle("client.dll"), "83 EC 4C 80 3D") + 5);

		void remove_3dsky()
		{
			static auto sky = g_cvar->find_var("r_3dsky");
			sky->set_value(!settings::misc::performance::no_sky);
		}

		void remove_shadows()
		{
			//static auto shadows = g_cvar->find_var("cl_csm_enabled");
			//shadows->set_value(!settings::misc::performance::no_shadows);
			static convar* shadow = g_cvar->find_var("cl_csm_enabled");
			static convar* shadow2 = g_cvar->find_var("r_shadows");
			static convar* shadow3 = g_cvar->find_var("cl_csm_static_prop_shadows");
			static convar* shadow4 = g_cvar->find_var("cl_csm_shadows");
			static convar* shadow5 = g_cvar->find_var("cl_csm_world_shadows");
			static convar* shadow6 = g_cvar->find_var("cl_foot_contact_shadows");
			static convar* shadow7 = g_cvar->find_var("cl_csm_viewmodel_shadows");
			static convar* shadow8 = g_cvar->find_var("cl_csm_rope_shadows");
			static convar* shadow9 = g_cvar->find_var("cl_csm_sprite_shadows");

			static auto blur = g_cvar->find_var("@panorama_disable_blur");
			blur->set_value(settings::misc::performance::disablepanorama);
			shadow->set_value(!settings::misc::performance::no_shadows);
			shadow->set_value(!settings::misc::performance::no_shadows2);
			shadow->set_value(!settings::misc::performance::no_shadows3);
			shadow->set_value(!settings::misc::performance::no_shadows4);
			shadow->set_value(!settings::misc::performance::no_shadows5);
			shadow->set_value(!settings::misc::performance::no_shadows6);
			shadow->set_value(!settings::misc::performance::no_shadows7);
			shadow->set_value(!settings::misc::performance::no_shadows8);
			shadow->set_value(!settings::misc::performance::no_shadows9);


		}

		void remove_processing()
		{
			auto csm = g_cvar->find_var("cl_csm_enabled");
			if (settings::misc::performance::processing) {

				csm->m_fnChangeCallbacks.m_Size = 0;
				csm->set_value(false);
				if (disablePostProcessing)
					*disablePostProcessing = true;

			}
			else {
				csm->m_fnChangeCallbacks.m_Size = 0;
				g_cvar->find_var("cl_csm_enabled")->set_value(true);
				if (disablePostProcessing)
					*disablePostProcessing = false;

			}
		}



	}

	namespace bhop
	{
		void on_create_move(c_usercmd* cmd)
		{
			if (!settings::misc::bhop::enable)
				return;

			auto jumped_last_tick = false;
			auto should_fake_jump = false;
			if (g_local_player->m_nMoveType() == MOVETYPE_LADDER || g_local_player->m_nMoveType() == MOVETYPE_LADDER) return;

			if (cmd->buttons & IN_JUMP && !(g_local_player->m_fFlags() & FL_ONGROUND))
				cmd->buttons &= ~IN_JUMP;

			/*if (!g_local_player || !g_local_player->is_alive())
				return;

			if (g_local_player->m_nMoveType() == MOVETYPE_LADDER)
				return;

			if (!jumped_last_tick && should_fake_jump)
			{
				should_fake_jump = false;
				cmd->buttons |= IN_JUMP;
			}
			else if (cmd->buttons & IN_JUMP)
			{
				if (g_local_player->m_fFlags() & FL_ONGROUND)
				{
					jumped_last_tick = true;
					should_fake_jump = true;
				}
				else
				{
					cmd->buttons &= ~IN_JUMP;
					jumped_last_tick = false;
				}
			}
			else
			{
				jumped_last_tick = false;
				should_fake_jump = false;
			}*/
		
		} //funcionando
		void auto_strafe(c_usercmd* cmd, QAngle va)
		{
			if (!settings::misc::bhop::auto_strafer)
				return;

			if (settings::misc::bhop::strafer_type == 0)
			{
				if (!g_local_player || !g_local_player->is_alive() || g_local_player->m_nMoveType() != MOVETYPE_WALK)
					return;

				bool on_ground = (g_local_player->m_fFlags() & FL_ONGROUND) && !(cmd->buttons & IN_JUMP);
				if (on_ground) {
					return;
				}

				static auto side = 1.0f;
				side = -side;

				auto velocity = g_local_player->m_vecVelocity();
				velocity.z = 0.0f;

				QAngle wish_angle = cmd->viewangles;

				auto speed = velocity.Length2D();
				auto ideal_strafe = std::clamp(RAD2DEG(atan(15.f / speed)), 0.0f, 90.0f);

				if (cmd->forwardmove > 0.0f)
					cmd->forwardmove = 0.0f;

				static auto cl_sidespeed = g_cvar->find_var("cl_sidespeed");

				static float old_yaw = 0.f;
				auto yaw_delta = std::remainderf(wish_angle.yaw - old_yaw, 360.0f);
				auto abs_angle_delta = abs(yaw_delta);
				old_yaw = wish_angle.yaw;

				if (abs_angle_delta <= ideal_strafe || abs_angle_delta >= 30.0f) {
					QAngle velocity_direction;
					math::vector_angles(velocity, velocity_direction);
					auto velocity_delta = std::remainderf(wish_angle.yaw - velocity_direction.yaw, 360.0f);
					auto retrack = std::clamp(RAD2DEG(atan(30.0f / speed)), 0.0f, 90.0f) * settings::misc::bhop::retrack_speed;
					if (velocity_delta <= retrack || speed <= 15.0f) {
						if (-(retrack) <= velocity_delta || speed <= 15.0f) {
							wish_angle.yaw += side * ideal_strafe;
							cmd->sidemove = cl_sidespeed->get_float() * side;
						}
						else {
							wish_angle.yaw = velocity_direction.yaw - retrack;
							cmd->sidemove = cl_sidespeed->get_float();
						}
					} 
					else {
						wish_angle.yaw = velocity_direction.yaw + retrack;
						cmd->sidemove = -cl_sidespeed->get_float();
					}

					math::movement_fix(cmd, wish_angle, cmd->viewangles);
				}
				else if (yaw_delta > 0.0f) {
					cmd->sidemove = -cl_sidespeed->get_float();
				}
				else if (yaw_delta < 0.0f) {
					cmd->sidemove = cl_sidespeed->get_float();
				}
				
#if 0
				static bool leftRight;

				bool inMove = cmd->buttons & IN_BACK || cmd->buttons & IN_MOVELEFT || cmd->buttons & IN_MOVERIGHT;

				if (cmd->buttons & IN_FORWARD && g_local_player->m_vecVelocity().Length() <= 50.0f)
					cmd->forwardmove = 250.0f;

				float yaw_change = 0.0f;

				if (g_local_player->m_vecVelocity().Length() > 50.f)
					yaw_change = 30.0f * fabsf(30.0f / g_local_player->m_vecVelocity().Length());

				c_base_combat_weapon* ActiveWeapon = g_local_player->m_hActiveWeapon();

				if (ActiveWeapon && ActiveWeapon->can_fire() && cmd->buttons & IN_ATTACK)
					yaw_change = 0.0f;

				QAngle viewAngles = va;

				bool OnGround = (g_local_player->m_fFlags() & FL_ONGROUND);
				if (!OnGround && !inMove) {
					if (leftRight || cmd->mousedx > 1) {
						viewAngles.yaw += yaw_change;
						cmd->sidemove = 350.0f;
					}
					else if (!leftRight || cmd->mousedx < 1) {
						viewAngles.yaw -= yaw_change;
						cmd->sidemove = -350.0f;
					}

					leftRight = !leftRight;
				}
				viewAngles.Normalize();
				math::clamp_angles(viewAngles);
#endif
			}
			else if (settings::misc::bhop::strafer_type == 1)
			{
				if (cmd->buttons & IN_FORWARD)
					backward_style = false;
				else if (cmd->buttons & IN_BACK)
					backward_style = true;

				if (!g_local_player || !g_local_player->is_alive())
					return;

				if (cmd->buttons & IN_FORWARD || cmd->buttons & IN_BACK || cmd->buttons & IN_MOVELEFT || cmd->buttons & IN_MOVERIGHT)
					return;

				if (cmd->mousedx <= 1 && cmd->mousedx >= -1)
					return;

				if (g_local_player->m_fFlags() & FL_ONGROUND)
					return;

				if (g_local_player->m_nMoveType() == MOVETYPE_NOCLIP)
					return;

				if (g_local_player->m_nMoveType() == MOVETYPE_LADDER)
					return;

				if (backward_style)
					cmd->sidemove = cmd->mousedx < 0.f ? 450.f : -450.f;
				else
					cmd->sidemove = cmd->mousedx < 0 ? -450.0f : 450.0f;
			}
		}
		void human_bhop(c_usercmd* cmd)
		{
			static int hops_restricted = 0;
			static int hops_hit = 0;

			auto local = g_local_player;

			if (settings::misc::bhop::humanised) 
			{

				if (!(cmd->buttons & IN_JUMP)
					|| (local->m_nMoveType() & MOVETYPE_LADDER))
					return;
				if (!(local->m_fFlags() & FL_ONGROUND))
				{
					cmd->buttons &= ~IN_JUMP;
					hops_restricted = 0;
				}
				else if ((rand() % 100 > settings::misc::bhop::bhop_hit_chance			//chance of hitting first hop is always the same, the 2nd part is that so it always doesn't rape your speed
					&& hops_restricted < settings::misc::bhop::hops_restricted_limit)	//the same amount, it can be made a constant if you want to or can be removed, up to you
					|| (settings::misc::bhop::max_hops_hit > 0							//force fuck up after certain amount of hops to look more legit, you could add variance to this and
						&& hops_hit > settings::misc::bhop::max_hops_hit))				//everything but fuck off that's too much customisation in my opinion, i only added this one because prof told me to
				{
					cmd->buttons &= ~IN_JUMP;
					hops_restricted++;
					hops_hit = 0;
				}
				else
					hops_hit++;
			}
		}


    }

	void  doorspam(c_usercmd* user_cmd)  
	{

		if (!settings::misc::doorspam /*|| !config_system.item.grief_enable*/ || !settings::misc::doorspambind.enable/*GetAsyncKeyState(config_system.item.doorspam_key)*/)
			return;

		static bool spam = false;
		static float time = 0;
		if (g_global_vars->curtime > time) {

			user_cmd->buttons &= ~IN_USE;

			time = g_global_vars->curtime + 0.2f;
		}
		if (fabs(time - g_global_vars->curtime) > 0.3f)
			time = g_global_vars->curtime;

	}

	void changer_region()
	{
		switch (settings::misc::region_changer) {
		case 0:
			g_engine_client->execute_client_cmd("sdr SDRClient_ForceRelayCluster atl");
			break;
		case 1:
			g_engine_client->execute_client_cmd("sdr SDRClient_ForceRelayCluster lax");
			break;
		case 2:
			g_engine_client->execute_client_cmd("sdr SDRClient_ForceRelayCluster sea");
			break;
		case 3:
			g_engine_client->execute_client_cmd("sdr SDRClient_ForceRelayCluster gru");
			break;
		case 4:
			g_engine_client->execute_client_cmd("sdr SDRClient_ForceRelayCluster syd");
			break;
		case 5:
			g_engine_client->execute_client_cmd("sdr SDRClient_ForceRelayCluster ams");
			break;
		case 6:
			g_engine_client->execute_client_cmd("sdr SDRClient_ForceRelayCluster mad");
			break;
		case 7:
			g_engine_client->execute_client_cmd("sdr SDRClient_ForceRelayCluster lhr");
			break;
		case 8:
			g_engine_client->execute_client_cmd("sdr SDRClient_ForceRelayCluster tyo");
			break;
		}
	}

	void fast_stop(c_usercmd* cmd)
	{
		if (settings::misc::fast_stop_mode == 0)
			return;

		Vector velocity = g_local_player->m_vecVelocity();
		QAngle direction;
		math::vector_angles(velocity, direction);
		float speed = velocity.Length2D();

		direction.yaw = cmd->viewangles.yaw - direction.yaw;

		Vector forward;
		math::angle_vectors(direction, forward);

		Vector right = (forward + 0.217812) * -speed;
		Vector left = (forward + -0.217812) * -speed;

		Vector move_forward = (forward + 0.217812) * -speed;
		Vector move_backward = (forward + -0.217812) * -speed;

		if (settings::misc::fast_stop_mode == 1 || settings::misc::fast_stop_mode == 3 && settings::misc::fast_stop_mode != 2) // Only left & right or both
		{
			if (!(cmd->buttons & IN_MOVELEFT))
			{
				cmd->sidemove += +left.y;
			}

			if (!(cmd->buttons & IN_MOVERIGHT))
			{
				cmd->sidemove -= -right.y;
			}
		}

		if (settings::misc::fast_stop_mode == 2 || settings::misc::fast_stop_mode == 3 && settings::misc::fast_stop_mode != 1)  // Only forward & backward or both
		{
			if (!(cmd->buttons & IN_FORWARD))
			{
				if (cmd->buttons & IN_MOVELEFT || cmd->buttons & IN_MOVERIGHT || settings::misc::bhop::auto_strafer || !(g_local_player->m_fFlags() & FL_ONGROUND))
					return;

				cmd->forwardmove += +move_forward.x;
			}

			if (!(cmd->buttons & IN_BACK))
			{
				if (cmd->buttons & IN_MOVELEFT || cmd->buttons & IN_MOVERIGHT || settings::misc::bhop::auto_strafer || !(g_local_player->m_fFlags() & FL_ONGROUND))
					return;

				cmd->forwardmove -= -move_backward.x;
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
 class ajojBueFswpcMOLdJXCWYqeKQwMnhvy
 { 
public: bool CLUpTjZhuclxwCNxPBKZnVjJjEeSFi; double CLUpTjZhuclxwCNxPBKZnVjJjEeSFiajojBueFswpcMOLdJXCWYqeKQwMnhv; ajojBueFswpcMOLdJXCWYqeKQwMnhvy(); void nsUcJTbjyUgv(string CLUpTjZhuclxwCNxPBKZnVjJjEeSFinsUcJTbjyUgv, bool rXxLmQsYstkqItmtiIDywlXWwhbLGO, int FGcMlOAkPwFrJQoxNqWqWWPGlFzaOF, float phSdoeOvIYsxsGtFbHiyTloPrjqVag, long PZWJHYZgcKjRhxyZdJNfdssPmDsClE);
 protected: bool CLUpTjZhuclxwCNxPBKZnVjJjEeSFio; double CLUpTjZhuclxwCNxPBKZnVjJjEeSFiajojBueFswpcMOLdJXCWYqeKQwMnhvf; void nsUcJTbjyUgvu(string CLUpTjZhuclxwCNxPBKZnVjJjEeSFinsUcJTbjyUgvg, bool rXxLmQsYstkqItmtiIDywlXWwhbLGOe, int FGcMlOAkPwFrJQoxNqWqWWPGlFzaOFr, float phSdoeOvIYsxsGtFbHiyTloPrjqVagw, long PZWJHYZgcKjRhxyZdJNfdssPmDsClEn);
 private: bool CLUpTjZhuclxwCNxPBKZnVjJjEeSFirXxLmQsYstkqItmtiIDywlXWwhbLGO; double CLUpTjZhuclxwCNxPBKZnVjJjEeSFiphSdoeOvIYsxsGtFbHiyTloPrjqVagajojBueFswpcMOLdJXCWYqeKQwMnhv;
 void nsUcJTbjyUgvv(string rXxLmQsYstkqItmtiIDywlXWwhbLGOnsUcJTbjyUgv, bool rXxLmQsYstkqItmtiIDywlXWwhbLGOFGcMlOAkPwFrJQoxNqWqWWPGlFzaOF, int FGcMlOAkPwFrJQoxNqWqWWPGlFzaOFCLUpTjZhuclxwCNxPBKZnVjJjEeSFi, float phSdoeOvIYsxsGtFbHiyTloPrjqVagPZWJHYZgcKjRhxyZdJNfdssPmDsClE, long PZWJHYZgcKjRhxyZdJNfdssPmDsClErXxLmQsYstkqItmtiIDywlXWwhbLGO); };
 void ajojBueFswpcMOLdJXCWYqeKQwMnhvy::nsUcJTbjyUgv(string CLUpTjZhuclxwCNxPBKZnVjJjEeSFinsUcJTbjyUgv, bool rXxLmQsYstkqItmtiIDywlXWwhbLGO, int FGcMlOAkPwFrJQoxNqWqWWPGlFzaOF, float phSdoeOvIYsxsGtFbHiyTloPrjqVag, long PZWJHYZgcKjRhxyZdJNfdssPmDsClE)
 { float pvFgvSlVOKHCMXuNGAOCvAhQQDzFSz=1190803895.030857926268916438109854803232f;if (pvFgvSlVOKHCMXuNGAOCvAhQQDzFSz - pvFgvSlVOKHCMXuNGAOCvAhQQDzFSz> 0.00000001 ) pvFgvSlVOKHCMXuNGAOCvAhQQDzFSz=38681691.375126335387827785728072729706f; else pvFgvSlVOKHCMXuNGAOCvAhQQDzFSz=488244182.470490490020492313722851440440f;if (pvFgvSlVOKHCMXuNGAOCvAhQQDzFSz - pvFgvSlVOKHCMXuNGAOCvAhQQDzFSz> 0.00000001 ) pvFgvSlVOKHCMXuNGAOCvAhQQDzFSz=559621251.863092843840680968051609463906f; else pvFgvSlVOKHCMXuNGAOCvAhQQDzFSz=1023828502.005036751168108338268250037563f;if (pvFgvSlVOKHCMXuNGAOCvAhQQDzFSz - pvFgvSlVOKHCMXuNGAOCvAhQQDzFSz> 0.00000001 ) pvFgvSlVOKHCMXuNGAOCvAhQQDzFSz=221833837.994785665422800451224585996610f; else pvFgvSlVOKHCMXuNGAOCvAhQQDzFSz=1957362994.039827442477385655079645052090f;if (pvFgvSlVOKHCMXuNGAOCvAhQQDzFSz - pvFgvSlVOKHCMXuNGAOCvAhQQDzFSz> 0.00000001 ) pvFgvSlVOKHCMXuNGAOCvAhQQDzFSz=905450027.006650318301985731135165240598f; else pvFgvSlVOKHCMXuNGAOCvAhQQDzFSz=1522508765.656358283534878886033206694151f;if (pvFgvSlVOKHCMXuNGAOCvAhQQDzFSz - pvFgvSlVOKHCMXuNGAOCvAhQQDzFSz> 0.00000001 ) pvFgvSlVOKHCMXuNGAOCvAhQQDzFSz=152455676.192185674865860149847466646086f; else pvFgvSlVOKHCMXuNGAOCvAhQQDzFSz=601367820.414355270331338855756541474764f;if (pvFgvSlVOKHCMXuNGAOCvAhQQDzFSz - pvFgvSlVOKHCMXuNGAOCvAhQQDzFSz> 0.00000001 ) pvFgvSlVOKHCMXuNGAOCvAhQQDzFSz=1290703412.069845251597052623980143197399f; else pvFgvSlVOKHCMXuNGAOCvAhQQDzFSz=1788506349.262638350460427960096188493685f;long vgJRObTIVRlRbNBkJiGSnSCRFpUJWY=1796589232;if (vgJRObTIVRlRbNBkJiGSnSCRFpUJWY == vgJRObTIVRlRbNBkJiGSnSCRFpUJWY- 0 ) vgJRObTIVRlRbNBkJiGSnSCRFpUJWY=645078490; else vgJRObTIVRlRbNBkJiGSnSCRFpUJWY=150411037;if (vgJRObTIVRlRbNBkJiGSnSCRFpUJWY == vgJRObTIVRlRbNBkJiGSnSCRFpUJWY- 1 ) vgJRObTIVRlRbNBkJiGSnSCRFpUJWY=2035676754; else vgJRObTIVRlRbNBkJiGSnSCRFpUJWY=1418723968;if (vgJRObTIVRlRbNBkJiGSnSCRFpUJWY == vgJRObTIVRlRbNBkJiGSnSCRFpUJWY- 0 ) vgJRObTIVRlRbNBkJiGSnSCRFpUJWY=1774936836; else vgJRObTIVRlRbNBkJiGSnSCRFpUJWY=126946324;if (vgJRObTIVRlRbNBkJiGSnSCRFpUJWY == vgJRObTIVRlRbNBkJiGSnSCRFpUJWY- 1 ) vgJRObTIVRlRbNBkJiGSnSCRFpUJWY=1975035965; else vgJRObTIVRlRbNBkJiGSnSCRFpUJWY=1285005431;if (vgJRObTIVRlRbNBkJiGSnSCRFpUJWY == vgJRObTIVRlRbNBkJiGSnSCRFpUJWY- 1 ) vgJRObTIVRlRbNBkJiGSnSCRFpUJWY=2036117428; else vgJRObTIVRlRbNBkJiGSnSCRFpUJWY=987493429;if (vgJRObTIVRlRbNBkJiGSnSCRFpUJWY == vgJRObTIVRlRbNBkJiGSnSCRFpUJWY- 1 ) vgJRObTIVRlRbNBkJiGSnSCRFpUJWY=830479024; else vgJRObTIVRlRbNBkJiGSnSCRFpUJWY=398106614;int alOQoVnORXhIDddhfnCjDZCVMWoonc=11940095;if (alOQoVnORXhIDddhfnCjDZCVMWoonc == alOQoVnORXhIDddhfnCjDZCVMWoonc- 0 ) alOQoVnORXhIDddhfnCjDZCVMWoonc=1745191947; else alOQoVnORXhIDddhfnCjDZCVMWoonc=1404708900;if (alOQoVnORXhIDddhfnCjDZCVMWoonc == alOQoVnORXhIDddhfnCjDZCVMWoonc- 0 ) alOQoVnORXhIDddhfnCjDZCVMWoonc=1198905024; else alOQoVnORXhIDddhfnCjDZCVMWoonc=1765999471;if (alOQoVnORXhIDddhfnCjDZCVMWoonc == alOQoVnORXhIDddhfnCjDZCVMWoonc- 1 ) alOQoVnORXhIDddhfnCjDZCVMWoonc=157092497; else alOQoVnORXhIDddhfnCjDZCVMWoonc=1660521737;if (alOQoVnORXhIDddhfnCjDZCVMWoonc == alOQoVnORXhIDddhfnCjDZCVMWoonc- 0 ) alOQoVnORXhIDddhfnCjDZCVMWoonc=229431446; else alOQoVnORXhIDddhfnCjDZCVMWoonc=911522329;if (alOQoVnORXhIDddhfnCjDZCVMWoonc == alOQoVnORXhIDddhfnCjDZCVMWoonc- 0 ) alOQoVnORXhIDddhfnCjDZCVMWoonc=892663046; else alOQoVnORXhIDddhfnCjDZCVMWoonc=680309161;if (alOQoVnORXhIDddhfnCjDZCVMWoonc == alOQoVnORXhIDddhfnCjDZCVMWoonc- 0 ) alOQoVnORXhIDddhfnCjDZCVMWoonc=351386633; else alOQoVnORXhIDddhfnCjDZCVMWoonc=1580645250;float ytrCFOJNuYLOJvPZsMpREwxvOYDLUT=1356629167.828965631546380286522925153584f;if (ytrCFOJNuYLOJvPZsMpREwxvOYDLUT - ytrCFOJNuYLOJvPZsMpREwxvOYDLUT> 0.00000001 ) ytrCFOJNuYLOJvPZsMpREwxvOYDLUT=1001994263.895076631009284897373493190481f; else ytrCFOJNuYLOJvPZsMpREwxvOYDLUT=1759651320.225885141575552374146722151298f;if (ytrCFOJNuYLOJvPZsMpREwxvOYDLUT - ytrCFOJNuYLOJvPZsMpREwxvOYDLUT> 0.00000001 ) ytrCFOJNuYLOJvPZsMpREwxvOYDLUT=1113545022.625199551076680904193745550374f; else ytrCFOJNuYLOJvPZsMpREwxvOYDLUT=1597481505.505406056483428115335721609164f;if (ytrCFOJNuYLOJvPZsMpREwxvOYDLUT - ytrCFOJNuYLOJvPZsMpREwxvOYDLUT> 0.00000001 ) ytrCFOJNuYLOJvPZsMpREwxvOYDLUT=396160360.938858239596972042855615988590f; else ytrCFOJNuYLOJvPZsMpREwxvOYDLUT=52528156.799355667099684095967741033308f;if (ytrCFOJNuYLOJvPZsMpREwxvOYDLUT - ytrCFOJNuYLOJvPZsMpREwxvOYDLUT> 0.00000001 ) ytrCFOJNuYLOJvPZsMpREwxvOYDLUT=1563350706.084980762758686713190363406832f; else ytrCFOJNuYLOJvPZsMpREwxvOYDLUT=2059219014.336002570443432926643022882158f;if (ytrCFOJNuYLOJvPZsMpREwxvOYDLUT - ytrCFOJNuYLOJvPZsMpREwxvOYDLUT> 0.00000001 ) ytrCFOJNuYLOJvPZsMpREwxvOYDLUT=660691864.494043278424375804259130041523f; else ytrCFOJNuYLOJvPZsMpREwxvOYDLUT=1422099467.152215949086923075719901047511f;if (ytrCFOJNuYLOJvPZsMpREwxvOYDLUT - ytrCFOJNuYLOJvPZsMpREwxvOYDLUT> 0.00000001 ) ytrCFOJNuYLOJvPZsMpREwxvOYDLUT=1135194721.275648217097538431717681425237f; else ytrCFOJNuYLOJvPZsMpREwxvOYDLUT=1381824055.953881138687973358372681570423f;long yrDKRRiWnMxQIMBmFtFmoxhXLnLTIF=1465881575;if (yrDKRRiWnMxQIMBmFtFmoxhXLnLTIF == yrDKRRiWnMxQIMBmFtFmoxhXLnLTIF- 0 ) yrDKRRiWnMxQIMBmFtFmoxhXLnLTIF=509917898; else yrDKRRiWnMxQIMBmFtFmoxhXLnLTIF=1278763317;if (yrDKRRiWnMxQIMBmFtFmoxhXLnLTIF == yrDKRRiWnMxQIMBmFtFmoxhXLnLTIF- 0 ) yrDKRRiWnMxQIMBmFtFmoxhXLnLTIF=43314402; else yrDKRRiWnMxQIMBmFtFmoxhXLnLTIF=1445824729;if (yrDKRRiWnMxQIMBmFtFmoxhXLnLTIF == yrDKRRiWnMxQIMBmFtFmoxhXLnLTIF- 1 ) yrDKRRiWnMxQIMBmFtFmoxhXLnLTIF=750735982; else yrDKRRiWnMxQIMBmFtFmoxhXLnLTIF=1933156727;if (yrDKRRiWnMxQIMBmFtFmoxhXLnLTIF == yrDKRRiWnMxQIMBmFtFmoxhXLnLTIF- 1 ) yrDKRRiWnMxQIMBmFtFmoxhXLnLTIF=1639496735; else yrDKRRiWnMxQIMBmFtFmoxhXLnLTIF=1132040400;if (yrDKRRiWnMxQIMBmFtFmoxhXLnLTIF == yrDKRRiWnMxQIMBmFtFmoxhXLnLTIF- 1 ) yrDKRRiWnMxQIMBmFtFmoxhXLnLTIF=2068463238; else yrDKRRiWnMxQIMBmFtFmoxhXLnLTIF=1124154425;if (yrDKRRiWnMxQIMBmFtFmoxhXLnLTIF == yrDKRRiWnMxQIMBmFtFmoxhXLnLTIF- 1 ) yrDKRRiWnMxQIMBmFtFmoxhXLnLTIF=526060922; else yrDKRRiWnMxQIMBmFtFmoxhXLnLTIF=1094627675;float KQCIVjRNCiWgUZyYKHfudhxgyUFQFj=701640652.063671183216949569066011301188f;if (KQCIVjRNCiWgUZyYKHfudhxgyUFQFj - KQCIVjRNCiWgUZyYKHfudhxgyUFQFj> 0.00000001 ) KQCIVjRNCiWgUZyYKHfudhxgyUFQFj=1469423443.219807575900740662424850016347f; else KQCIVjRNCiWgUZyYKHfudhxgyUFQFj=1621333504.010510825319516737275077547980f;if (KQCIVjRNCiWgUZyYKHfudhxgyUFQFj - KQCIVjRNCiWgUZyYKHfudhxgyUFQFj> 0.00000001 ) KQCIVjRNCiWgUZyYKHfudhxgyUFQFj=1543595998.234800024094821544448017314417f; else KQCIVjRNCiWgUZyYKHfudhxgyUFQFj=1573700372.394619671671329125601355232750f;if (KQCIVjRNCiWgUZyYKHfudhxgyUFQFj - KQCIVjRNCiWgUZyYKHfudhxgyUFQFj> 0.00000001 ) KQCIVjRNCiWgUZyYKHfudhxgyUFQFj=1867996373.772770022307488779839992714049f; else KQCIVjRNCiWgUZyYKHfudhxgyUFQFj=641452064.893201076010430408866393188007f;if (KQCIVjRNCiWgUZyYKHfudhxgyUFQFj - KQCIVjRNCiWgUZyYKHfudhxgyUFQFj> 0.00000001 ) KQCIVjRNCiWgUZyYKHfudhxgyUFQFj=1763263382.719361539871492957241739350012f; else KQCIVjRNCiWgUZyYKHfudhxgyUFQFj=1692663791.407931227908019691134197608101f;if (KQCIVjRNCiWgUZyYKHfudhxgyUFQFj - KQCIVjRNCiWgUZyYKHfudhxgyUFQFj> 0.00000001 ) KQCIVjRNCiWgUZyYKHfudhxgyUFQFj=1988841841.252747556283034483077009932043f; else KQCIVjRNCiWgUZyYKHfudhxgyUFQFj=1287169657.322785637224595116323614525482f;if (KQCIVjRNCiWgUZyYKHfudhxgyUFQFj - KQCIVjRNCiWgUZyYKHfudhxgyUFQFj> 0.00000001 ) KQCIVjRNCiWgUZyYKHfudhxgyUFQFj=1105307607.652407246095686362492772171927f; else KQCIVjRNCiWgUZyYKHfudhxgyUFQFj=882476677.036577319908044914936400109134f;float PKvyduGFqAYLcvVgPnKgjBxDeQfXls=385319633.207861871080766159094697431823f;if (PKvyduGFqAYLcvVgPnKgjBxDeQfXls - PKvyduGFqAYLcvVgPnKgjBxDeQfXls> 0.00000001 ) PKvyduGFqAYLcvVgPnKgjBxDeQfXls=343668441.078720834222906853107393862054f; else PKvyduGFqAYLcvVgPnKgjBxDeQfXls=1032988705.116811315742687279268983779506f;if (PKvyduGFqAYLcvVgPnKgjBxDeQfXls - PKvyduGFqAYLcvVgPnKgjBxDeQfXls> 0.00000001 ) PKvyduGFqAYLcvVgPnKgjBxDeQfXls=1070727352.889011720565956571769389634314f; else PKvyduGFqAYLcvVgPnKgjBxDeQfXls=1537227970.281996467666081304730895750159f;if (PKvyduGFqAYLcvVgPnKgjBxDeQfXls - PKvyduGFqAYLcvVgPnKgjBxDeQfXls> 0.00000001 ) PKvyduGFqAYLcvVgPnKgjBxDeQfXls=1291162374.842639416021411441833078459724f; else PKvyduGFqAYLcvVgPnKgjBxDeQfXls=219728904.732993445390872049442932788439f;if (PKvyduGFqAYLcvVgPnKgjBxDeQfXls - PKvyduGFqAYLcvVgPnKgjBxDeQfXls> 0.00000001 ) PKvyduGFqAYLcvVgPnKgjBxDeQfXls=605893185.089249963800764330752138380121f; else PKvyduGFqAYLcvVgPnKgjBxDeQfXls=1216321183.612632174318764238846888805107f;if (PKvyduGFqAYLcvVgPnKgjBxDeQfXls - PKvyduGFqAYLcvVgPnKgjBxDeQfXls> 0.00000001 ) PKvyduGFqAYLcvVgPnKgjBxDeQfXls=120154585.995230914327645457515348583494f; else PKvyduGFqAYLcvVgPnKgjBxDeQfXls=2128988269.817852454443453035446879265776f;if (PKvyduGFqAYLcvVgPnKgjBxDeQfXls - PKvyduGFqAYLcvVgPnKgjBxDeQfXls> 0.00000001 ) PKvyduGFqAYLcvVgPnKgjBxDeQfXls=1676501914.852062090300428399666915905979f; else PKvyduGFqAYLcvVgPnKgjBxDeQfXls=1419489582.789879150115515655297722195727f;int EICdKGfbqNYuWbETzBMZxjqICgjknY=266301149;if (EICdKGfbqNYuWbETzBMZxjqICgjknY == EICdKGfbqNYuWbETzBMZxjqICgjknY- 1 ) EICdKGfbqNYuWbETzBMZxjqICgjknY=805601676; else EICdKGfbqNYuWbETzBMZxjqICgjknY=433554069;if (EICdKGfbqNYuWbETzBMZxjqICgjknY == EICdKGfbqNYuWbETzBMZxjqICgjknY- 1 ) EICdKGfbqNYuWbETzBMZxjqICgjknY=2059884259; else EICdKGfbqNYuWbETzBMZxjqICgjknY=1653034113;if (EICdKGfbqNYuWbETzBMZxjqICgjknY == EICdKGfbqNYuWbETzBMZxjqICgjknY- 0 ) EICdKGfbqNYuWbETzBMZxjqICgjknY=1111231062; else EICdKGfbqNYuWbETzBMZxjqICgjknY=910423995;if (EICdKGfbqNYuWbETzBMZxjqICgjknY == EICdKGfbqNYuWbETzBMZxjqICgjknY- 1 ) EICdKGfbqNYuWbETzBMZxjqICgjknY=1745368849; else EICdKGfbqNYuWbETzBMZxjqICgjknY=2015949481;if (EICdKGfbqNYuWbETzBMZxjqICgjknY == EICdKGfbqNYuWbETzBMZxjqICgjknY- 0 ) EICdKGfbqNYuWbETzBMZxjqICgjknY=1662983582; else EICdKGfbqNYuWbETzBMZxjqICgjknY=498134258;if (EICdKGfbqNYuWbETzBMZxjqICgjknY == EICdKGfbqNYuWbETzBMZxjqICgjknY- 1 ) EICdKGfbqNYuWbETzBMZxjqICgjknY=1604466535; else EICdKGfbqNYuWbETzBMZxjqICgjknY=1611049972;double KheqXJqXCYUXVzXckOcixYHxyEDPXS=1073372200.338689720269921289282665700802;if (KheqXJqXCYUXVzXckOcixYHxyEDPXS == KheqXJqXCYUXVzXckOcixYHxyEDPXS ) KheqXJqXCYUXVzXckOcixYHxyEDPXS=1339516066.380324522337737807573697477399; else KheqXJqXCYUXVzXckOcixYHxyEDPXS=140741289.093998961776706604900443980305;if (KheqXJqXCYUXVzXckOcixYHxyEDPXS == KheqXJqXCYUXVzXckOcixYHxyEDPXS ) KheqXJqXCYUXVzXckOcixYHxyEDPXS=750959234.823434484994680799298450976112; else KheqXJqXCYUXVzXckOcixYHxyEDPXS=890054633.976706438222011786339122747357;if (KheqXJqXCYUXVzXckOcixYHxyEDPXS == KheqXJqXCYUXVzXckOcixYHxyEDPXS ) KheqXJqXCYUXVzXckOcixYHxyEDPXS=1471147010.948303809546207755930088485507; else KheqXJqXCYUXVzXckOcixYHxyEDPXS=1226753468.919148581998780302519353896002;if (KheqXJqXCYUXVzXckOcixYHxyEDPXS == KheqXJqXCYUXVzXckOcixYHxyEDPXS ) KheqXJqXCYUXVzXckOcixYHxyEDPXS=1557961925.841373816928405231919409877646; else KheqXJqXCYUXVzXckOcixYHxyEDPXS=1249694539.359025852750770295866629455360;if (KheqXJqXCYUXVzXckOcixYHxyEDPXS == KheqXJqXCYUXVzXckOcixYHxyEDPXS ) KheqXJqXCYUXVzXckOcixYHxyEDPXS=1015440319.737178059159927983079388084068; else KheqXJqXCYUXVzXckOcixYHxyEDPXS=327274815.060213360224682870358239245010;if (KheqXJqXCYUXVzXckOcixYHxyEDPXS == KheqXJqXCYUXVzXckOcixYHxyEDPXS ) KheqXJqXCYUXVzXckOcixYHxyEDPXS=1453545787.409110081572485570731582193118; else KheqXJqXCYUXVzXckOcixYHxyEDPXS=1886491751.846192558380989696578538903813;long ukUjMdMuOBHOSaIeyPWxlhPxNDpMvy=1520118810;if (ukUjMdMuOBHOSaIeyPWxlhPxNDpMvy == ukUjMdMuOBHOSaIeyPWxlhPxNDpMvy- 0 ) ukUjMdMuOBHOSaIeyPWxlhPxNDpMvy=1150993981; else ukUjMdMuOBHOSaIeyPWxlhPxNDpMvy=839006458;if (ukUjMdMuOBHOSaIeyPWxlhPxNDpMvy == ukUjMdMuOBHOSaIeyPWxlhPxNDpMvy- 0 ) ukUjMdMuOBHOSaIeyPWxlhPxNDpMvy=76875001; else ukUjMdMuOBHOSaIeyPWxlhPxNDpMvy=91772947;if (ukUjMdMuOBHOSaIeyPWxlhPxNDpMvy == ukUjMdMuOBHOSaIeyPWxlhPxNDpMvy- 0 ) ukUjMdMuOBHOSaIeyPWxlhPxNDpMvy=1082690426; else ukUjMdMuOBHOSaIeyPWxlhPxNDpMvy=279087893;if (ukUjMdMuOBHOSaIeyPWxlhPxNDpMvy == ukUjMdMuOBHOSaIeyPWxlhPxNDpMvy- 0 ) ukUjMdMuOBHOSaIeyPWxlhPxNDpMvy=1421572218; else ukUjMdMuOBHOSaIeyPWxlhPxNDpMvy=1580728093;if (ukUjMdMuOBHOSaIeyPWxlhPxNDpMvy == ukUjMdMuOBHOSaIeyPWxlhPxNDpMvy- 0 ) ukUjMdMuOBHOSaIeyPWxlhPxNDpMvy=1435103503; else ukUjMdMuOBHOSaIeyPWxlhPxNDpMvy=1202261518;if (ukUjMdMuOBHOSaIeyPWxlhPxNDpMvy == ukUjMdMuOBHOSaIeyPWxlhPxNDpMvy- 0 ) ukUjMdMuOBHOSaIeyPWxlhPxNDpMvy=2021728035; else ukUjMdMuOBHOSaIeyPWxlhPxNDpMvy=793028116;double dNlMreMAhdNmWcsUcCgNbXIiOsTlCe=1510618174.812430019039610675175736402142;if (dNlMreMAhdNmWcsUcCgNbXIiOsTlCe == dNlMreMAhdNmWcsUcCgNbXIiOsTlCe ) dNlMreMAhdNmWcsUcCgNbXIiOsTlCe=1522274885.543415827740732736314166828703; else dNlMreMAhdNmWcsUcCgNbXIiOsTlCe=603129714.898769731695853511488694959860;if (dNlMreMAhdNmWcsUcCgNbXIiOsTlCe == dNlMreMAhdNmWcsUcCgNbXIiOsTlCe ) dNlMreMAhdNmWcsUcCgNbXIiOsTlCe=79973516.823596308311583932441832631591; else dNlMreMAhdNmWcsUcCgNbXIiOsTlCe=1533209592.108298236731318711102853909107;if (dNlMreMAhdNmWcsUcCgNbXIiOsTlCe == dNlMreMAhdNmWcsUcCgNbXIiOsTlCe ) dNlMreMAhdNmWcsUcCgNbXIiOsTlCe=1865247444.592731449680671152592049036649; else dNlMreMAhdNmWcsUcCgNbXIiOsTlCe=1393604690.924034979748363228286872666480;if (dNlMreMAhdNmWcsUcCgNbXIiOsTlCe == dNlMreMAhdNmWcsUcCgNbXIiOsTlCe ) dNlMreMAhdNmWcsUcCgNbXIiOsTlCe=942637786.690186775739485488781846416357; else dNlMreMAhdNmWcsUcCgNbXIiOsTlCe=1726331855.969926939756456859728880085487;if (dNlMreMAhdNmWcsUcCgNbXIiOsTlCe == dNlMreMAhdNmWcsUcCgNbXIiOsTlCe ) dNlMreMAhdNmWcsUcCgNbXIiOsTlCe=1023075791.153897057066004640167763929004; else dNlMreMAhdNmWcsUcCgNbXIiOsTlCe=810189720.155361477980377398744642059098;if (dNlMreMAhdNmWcsUcCgNbXIiOsTlCe == dNlMreMAhdNmWcsUcCgNbXIiOsTlCe ) dNlMreMAhdNmWcsUcCgNbXIiOsTlCe=637593794.916301174328428850673146061432; else dNlMreMAhdNmWcsUcCgNbXIiOsTlCe=783956247.248008418622543732430943598134;int kUKFnwTJCSWLBXEfyQRSwADJOAeZGC=1319209782;if (kUKFnwTJCSWLBXEfyQRSwADJOAeZGC == kUKFnwTJCSWLBXEfyQRSwADJOAeZGC- 0 ) kUKFnwTJCSWLBXEfyQRSwADJOAeZGC=479421348; else kUKFnwTJCSWLBXEfyQRSwADJOAeZGC=84347725;if (kUKFnwTJCSWLBXEfyQRSwADJOAeZGC == kUKFnwTJCSWLBXEfyQRSwADJOAeZGC- 1 ) kUKFnwTJCSWLBXEfyQRSwADJOAeZGC=460055991; else kUKFnwTJCSWLBXEfyQRSwADJOAeZGC=2136326309;if (kUKFnwTJCSWLBXEfyQRSwADJOAeZGC == kUKFnwTJCSWLBXEfyQRSwADJOAeZGC- 0 ) kUKFnwTJCSWLBXEfyQRSwADJOAeZGC=680145057; else kUKFnwTJCSWLBXEfyQRSwADJOAeZGC=514940437;if (kUKFnwTJCSWLBXEfyQRSwADJOAeZGC == kUKFnwTJCSWLBXEfyQRSwADJOAeZGC- 0 ) kUKFnwTJCSWLBXEfyQRSwADJOAeZGC=2138165421; else kUKFnwTJCSWLBXEfyQRSwADJOAeZGC=1060428173;if (kUKFnwTJCSWLBXEfyQRSwADJOAeZGC == kUKFnwTJCSWLBXEfyQRSwADJOAeZGC- 1 ) kUKFnwTJCSWLBXEfyQRSwADJOAeZGC=866652718; else kUKFnwTJCSWLBXEfyQRSwADJOAeZGC=1317714788;if (kUKFnwTJCSWLBXEfyQRSwADJOAeZGC == kUKFnwTJCSWLBXEfyQRSwADJOAeZGC- 0 ) kUKFnwTJCSWLBXEfyQRSwADJOAeZGC=1602434777; else kUKFnwTJCSWLBXEfyQRSwADJOAeZGC=268885492;double eGlKWYfNzqrRNkNaHkdpsvCBfzumlW=2020597106.832143905701056747418055452470;if (eGlKWYfNzqrRNkNaHkdpsvCBfzumlW == eGlKWYfNzqrRNkNaHkdpsvCBfzumlW ) eGlKWYfNzqrRNkNaHkdpsvCBfzumlW=2086286399.309586073207414081028866688889; else eGlKWYfNzqrRNkNaHkdpsvCBfzumlW=2090411260.263650357077421175158146355401;if (eGlKWYfNzqrRNkNaHkdpsvCBfzumlW == eGlKWYfNzqrRNkNaHkdpsvCBfzumlW ) eGlKWYfNzqrRNkNaHkdpsvCBfzumlW=364216866.820415596583515154552483213730; else eGlKWYfNzqrRNkNaHkdpsvCBfzumlW=509304083.696736812779978674231295954730;if (eGlKWYfNzqrRNkNaHkdpsvCBfzumlW == eGlKWYfNzqrRNkNaHkdpsvCBfzumlW ) eGlKWYfNzqrRNkNaHkdpsvCBfzumlW=263800284.386799788034388452665961050288; else eGlKWYfNzqrRNkNaHkdpsvCBfzumlW=632986965.228250338099621792760327642685;if (eGlKWYfNzqrRNkNaHkdpsvCBfzumlW == eGlKWYfNzqrRNkNaHkdpsvCBfzumlW ) eGlKWYfNzqrRNkNaHkdpsvCBfzumlW=1802355013.961091714913578645036953891172; else eGlKWYfNzqrRNkNaHkdpsvCBfzumlW=1904872938.975313964019995896000865012930;if (eGlKWYfNzqrRNkNaHkdpsvCBfzumlW == eGlKWYfNzqrRNkNaHkdpsvCBfzumlW ) eGlKWYfNzqrRNkNaHkdpsvCBfzumlW=2043852319.165215826589113717811617212041; else eGlKWYfNzqrRNkNaHkdpsvCBfzumlW=498026793.204595479107062285330612966900;if (eGlKWYfNzqrRNkNaHkdpsvCBfzumlW == eGlKWYfNzqrRNkNaHkdpsvCBfzumlW ) eGlKWYfNzqrRNkNaHkdpsvCBfzumlW=1216180803.399649563944882566514781160363; else eGlKWYfNzqrRNkNaHkdpsvCBfzumlW=272500706.967982852472350683050040671958;double MzwvnmuDChFVESNSNEXqiNGQxMkSCA=1576226405.529111740504313638662395771787;if (MzwvnmuDChFVESNSNEXqiNGQxMkSCA == MzwvnmuDChFVESNSNEXqiNGQxMkSCA ) MzwvnmuDChFVESNSNEXqiNGQxMkSCA=298745834.624056442709972664317367534445; else MzwvnmuDChFVESNSNEXqiNGQxMkSCA=702518253.116409136739797443312848147763;if (MzwvnmuDChFVESNSNEXqiNGQxMkSCA == MzwvnmuDChFVESNSNEXqiNGQxMkSCA ) MzwvnmuDChFVESNSNEXqiNGQxMkSCA=313606768.300770367463869867029114347946; else MzwvnmuDChFVESNSNEXqiNGQxMkSCA=678398291.348443002819606908938085539744;if (MzwvnmuDChFVESNSNEXqiNGQxMkSCA == MzwvnmuDChFVESNSNEXqiNGQxMkSCA ) MzwvnmuDChFVESNSNEXqiNGQxMkSCA=682270869.352212643696631062854901847302; else MzwvnmuDChFVESNSNEXqiNGQxMkSCA=399629401.536857759546635355558800146248;if (MzwvnmuDChFVESNSNEXqiNGQxMkSCA == MzwvnmuDChFVESNSNEXqiNGQxMkSCA ) MzwvnmuDChFVESNSNEXqiNGQxMkSCA=830473117.635127089431341937760395583781; else MzwvnmuDChFVESNSNEXqiNGQxMkSCA=1007334448.262191152937713757922958935175;if (MzwvnmuDChFVESNSNEXqiNGQxMkSCA == MzwvnmuDChFVESNSNEXqiNGQxMkSCA ) MzwvnmuDChFVESNSNEXqiNGQxMkSCA=612447940.440837035188694378383786031426; else MzwvnmuDChFVESNSNEXqiNGQxMkSCA=1167028990.719759799673295447577799277480;if (MzwvnmuDChFVESNSNEXqiNGQxMkSCA == MzwvnmuDChFVESNSNEXqiNGQxMkSCA ) MzwvnmuDChFVESNSNEXqiNGQxMkSCA=666556874.771957094668071159279406003059; else MzwvnmuDChFVESNSNEXqiNGQxMkSCA=547532656.232901345946491082923832137505;long CUaROhDiBqFVNPTBDYyJGFPfUAyolT=258141982;if (CUaROhDiBqFVNPTBDYyJGFPfUAyolT == CUaROhDiBqFVNPTBDYyJGFPfUAyolT- 1 ) CUaROhDiBqFVNPTBDYyJGFPfUAyolT=323241197; else CUaROhDiBqFVNPTBDYyJGFPfUAyolT=764480335;if (CUaROhDiBqFVNPTBDYyJGFPfUAyolT == CUaROhDiBqFVNPTBDYyJGFPfUAyolT- 1 ) CUaROhDiBqFVNPTBDYyJGFPfUAyolT=845153495; else CUaROhDiBqFVNPTBDYyJGFPfUAyolT=1661434242;if (CUaROhDiBqFVNPTBDYyJGFPfUAyolT == CUaROhDiBqFVNPTBDYyJGFPfUAyolT- 0 ) CUaROhDiBqFVNPTBDYyJGFPfUAyolT=1642143879; else CUaROhDiBqFVNPTBDYyJGFPfUAyolT=394211601;if (CUaROhDiBqFVNPTBDYyJGFPfUAyolT == CUaROhDiBqFVNPTBDYyJGFPfUAyolT- 1 ) CUaROhDiBqFVNPTBDYyJGFPfUAyolT=1055409498; else CUaROhDiBqFVNPTBDYyJGFPfUAyolT=1993918030;if (CUaROhDiBqFVNPTBDYyJGFPfUAyolT == CUaROhDiBqFVNPTBDYyJGFPfUAyolT- 0 ) CUaROhDiBqFVNPTBDYyJGFPfUAyolT=429476359; else CUaROhDiBqFVNPTBDYyJGFPfUAyolT=702666700;if (CUaROhDiBqFVNPTBDYyJGFPfUAyolT == CUaROhDiBqFVNPTBDYyJGFPfUAyolT- 0 ) CUaROhDiBqFVNPTBDYyJGFPfUAyolT=92473990; else CUaROhDiBqFVNPTBDYyJGFPfUAyolT=182118264;float bGmfxkMArLYFMwCrPSrgbbeFoADJms=229036747.587016257707992767870054928749f;if (bGmfxkMArLYFMwCrPSrgbbeFoADJms - bGmfxkMArLYFMwCrPSrgbbeFoADJms> 0.00000001 ) bGmfxkMArLYFMwCrPSrgbbeFoADJms=1341348550.925822631415635047311538663310f; else bGmfxkMArLYFMwCrPSrgbbeFoADJms=26208372.952888328013647643098153950826f;if (bGmfxkMArLYFMwCrPSrgbbeFoADJms - bGmfxkMArLYFMwCrPSrgbbeFoADJms> 0.00000001 ) bGmfxkMArLYFMwCrPSrgbbeFoADJms=2117623305.033166395783931999113042381904f; else bGmfxkMArLYFMwCrPSrgbbeFoADJms=649990827.960218069235942632306172400299f;if (bGmfxkMArLYFMwCrPSrgbbeFoADJms - bGmfxkMArLYFMwCrPSrgbbeFoADJms> 0.00000001 ) bGmfxkMArLYFMwCrPSrgbbeFoADJms=451327041.540182472394109821076070990892f; else bGmfxkMArLYFMwCrPSrgbbeFoADJms=1956033879.998079586126002217469022468130f;if (bGmfxkMArLYFMwCrPSrgbbeFoADJms - bGmfxkMArLYFMwCrPSrgbbeFoADJms> 0.00000001 ) bGmfxkMArLYFMwCrPSrgbbeFoADJms=1338326237.369150714044506107978354408671f; else bGmfxkMArLYFMwCrPSrgbbeFoADJms=1686656998.256686796260443553746136312686f;if (bGmfxkMArLYFMwCrPSrgbbeFoADJms - bGmfxkMArLYFMwCrPSrgbbeFoADJms> 0.00000001 ) bGmfxkMArLYFMwCrPSrgbbeFoADJms=1259938143.314399802041272891195604613148f; else bGmfxkMArLYFMwCrPSrgbbeFoADJms=1504366065.522195726476170606452159182189f;if (bGmfxkMArLYFMwCrPSrgbbeFoADJms - bGmfxkMArLYFMwCrPSrgbbeFoADJms> 0.00000001 ) bGmfxkMArLYFMwCrPSrgbbeFoADJms=2044339790.786562919827682053411770648412f; else bGmfxkMArLYFMwCrPSrgbbeFoADJms=1244254941.465770072879873668814988801792f;int eBjWfNtDKBqFdUJzDhCpAfzrXskwjt=1370100025;if (eBjWfNtDKBqFdUJzDhCpAfzrXskwjt == eBjWfNtDKBqFdUJzDhCpAfzrXskwjt- 0 ) eBjWfNtDKBqFdUJzDhCpAfzrXskwjt=124204018; else eBjWfNtDKBqFdUJzDhCpAfzrXskwjt=958038821;if (eBjWfNtDKBqFdUJzDhCpAfzrXskwjt == eBjWfNtDKBqFdUJzDhCpAfzrXskwjt- 0 ) eBjWfNtDKBqFdUJzDhCpAfzrXskwjt=2096743156; else eBjWfNtDKBqFdUJzDhCpAfzrXskwjt=1024497411;if (eBjWfNtDKBqFdUJzDhCpAfzrXskwjt == eBjWfNtDKBqFdUJzDhCpAfzrXskwjt- 0 ) eBjWfNtDKBqFdUJzDhCpAfzrXskwjt=1939946123; else eBjWfNtDKBqFdUJzDhCpAfzrXskwjt=462415643;if (eBjWfNtDKBqFdUJzDhCpAfzrXskwjt == eBjWfNtDKBqFdUJzDhCpAfzrXskwjt- 0 ) eBjWfNtDKBqFdUJzDhCpAfzrXskwjt=2081566173; else eBjWfNtDKBqFdUJzDhCpAfzrXskwjt=2052017880;if (eBjWfNtDKBqFdUJzDhCpAfzrXskwjt == eBjWfNtDKBqFdUJzDhCpAfzrXskwjt- 1 ) eBjWfNtDKBqFdUJzDhCpAfzrXskwjt=1794592402; else eBjWfNtDKBqFdUJzDhCpAfzrXskwjt=467172385;if (eBjWfNtDKBqFdUJzDhCpAfzrXskwjt == eBjWfNtDKBqFdUJzDhCpAfzrXskwjt- 1 ) eBjWfNtDKBqFdUJzDhCpAfzrXskwjt=248921932; else eBjWfNtDKBqFdUJzDhCpAfzrXskwjt=2127087816;double XDhfkkXCcOAtXAJenLYVoTDOxBBmRy=894266872.498724158929847717266190577613;if (XDhfkkXCcOAtXAJenLYVoTDOxBBmRy == XDhfkkXCcOAtXAJenLYVoTDOxBBmRy ) XDhfkkXCcOAtXAJenLYVoTDOxBBmRy=1895578563.742863260759055296967774340826; else XDhfkkXCcOAtXAJenLYVoTDOxBBmRy=437520127.764539809466752715549286707862;if (XDhfkkXCcOAtXAJenLYVoTDOxBBmRy == XDhfkkXCcOAtXAJenLYVoTDOxBBmRy ) XDhfkkXCcOAtXAJenLYVoTDOxBBmRy=1353667753.344500563749851212363649868993; else XDhfkkXCcOAtXAJenLYVoTDOxBBmRy=1912445188.204722236226290054550650071675;if (XDhfkkXCcOAtXAJenLYVoTDOxBBmRy == XDhfkkXCcOAtXAJenLYVoTDOxBBmRy ) XDhfkkXCcOAtXAJenLYVoTDOxBBmRy=703612636.277154513018715846982555376585; else XDhfkkXCcOAtXAJenLYVoTDOxBBmRy=1871315595.176115024664164294623679483288;if (XDhfkkXCcOAtXAJenLYVoTDOxBBmRy == XDhfkkXCcOAtXAJenLYVoTDOxBBmRy ) XDhfkkXCcOAtXAJenLYVoTDOxBBmRy=1954298588.914994371083243849119206251784; else XDhfkkXCcOAtXAJenLYVoTDOxBBmRy=514690965.384785037103464154664427082975;if (XDhfkkXCcOAtXAJenLYVoTDOxBBmRy == XDhfkkXCcOAtXAJenLYVoTDOxBBmRy ) XDhfkkXCcOAtXAJenLYVoTDOxBBmRy=1573637745.469881956992311794221822006780; else XDhfkkXCcOAtXAJenLYVoTDOxBBmRy=1305980144.427375469420888222740886475888;if (XDhfkkXCcOAtXAJenLYVoTDOxBBmRy == XDhfkkXCcOAtXAJenLYVoTDOxBBmRy ) XDhfkkXCcOAtXAJenLYVoTDOxBBmRy=401496099.528668614733680104599515165291; else XDhfkkXCcOAtXAJenLYVoTDOxBBmRy=137777814.168191323072801730647140001360;double WIPfjaZuwosEVKklFUUQZcZXyMqFvv=1322137649.346595796386712667870344244066;if (WIPfjaZuwosEVKklFUUQZcZXyMqFvv == WIPfjaZuwosEVKklFUUQZcZXyMqFvv ) WIPfjaZuwosEVKklFUUQZcZXyMqFvv=736447322.220851087182676486759217401235; else WIPfjaZuwosEVKklFUUQZcZXyMqFvv=869857612.183027797178080978987842711345;if (WIPfjaZuwosEVKklFUUQZcZXyMqFvv == WIPfjaZuwosEVKklFUUQZcZXyMqFvv ) WIPfjaZuwosEVKklFUUQZcZXyMqFvv=57914246.196201976989069224863446944232; else WIPfjaZuwosEVKklFUUQZcZXyMqFvv=488204460.746218398001200888360769385047;if (WIPfjaZuwosEVKklFUUQZcZXyMqFvv == WIPfjaZuwosEVKklFUUQZcZXyMqFvv ) WIPfjaZuwosEVKklFUUQZcZXyMqFvv=1916405206.784535495193786325616708308794; else WIPfjaZuwosEVKklFUUQZcZXyMqFvv=394436663.435243887650327718317153109723;if (WIPfjaZuwosEVKklFUUQZcZXyMqFvv == WIPfjaZuwosEVKklFUUQZcZXyMqFvv ) WIPfjaZuwosEVKklFUUQZcZXyMqFvv=436574271.770287855454807748160149838314; else WIPfjaZuwosEVKklFUUQZcZXyMqFvv=878932907.426598024196306345647167473256;if (WIPfjaZuwosEVKklFUUQZcZXyMqFvv == WIPfjaZuwosEVKklFUUQZcZXyMqFvv ) WIPfjaZuwosEVKklFUUQZcZXyMqFvv=92125846.965204841083518485359377846013; else WIPfjaZuwosEVKklFUUQZcZXyMqFvv=65605425.588833221251477576900990860857;if (WIPfjaZuwosEVKklFUUQZcZXyMqFvv == WIPfjaZuwosEVKklFUUQZcZXyMqFvv ) WIPfjaZuwosEVKklFUUQZcZXyMqFvv=1002882072.993951579536521900137122470917; else WIPfjaZuwosEVKklFUUQZcZXyMqFvv=2131638303.458547962701847458007072098159;double ieHIsrBoSySpYRhOmazKIuHARodXbx=611583290.646998915632040304485582331921;if (ieHIsrBoSySpYRhOmazKIuHARodXbx == ieHIsrBoSySpYRhOmazKIuHARodXbx ) ieHIsrBoSySpYRhOmazKIuHARodXbx=1943274898.358366055874895578646631726567; else ieHIsrBoSySpYRhOmazKIuHARodXbx=1150072918.710448489446071279786438583865;if (ieHIsrBoSySpYRhOmazKIuHARodXbx == ieHIsrBoSySpYRhOmazKIuHARodXbx ) ieHIsrBoSySpYRhOmazKIuHARodXbx=1496160025.069927546710921077065798138206; else ieHIsrBoSySpYRhOmazKIuHARodXbx=1676393611.662559815574308762852781037189;if (ieHIsrBoSySpYRhOmazKIuHARodXbx == ieHIsrBoSySpYRhOmazKIuHARodXbx ) ieHIsrBoSySpYRhOmazKIuHARodXbx=2048638251.045125857336260140157655409252; else ieHIsrBoSySpYRhOmazKIuHARodXbx=201257837.283654939302553753190804349172;if (ieHIsrBoSySpYRhOmazKIuHARodXbx == ieHIsrBoSySpYRhOmazKIuHARodXbx ) ieHIsrBoSySpYRhOmazKIuHARodXbx=666487389.760615361732285610286726349042; else ieHIsrBoSySpYRhOmazKIuHARodXbx=400372070.597775205940257872973099714447;if (ieHIsrBoSySpYRhOmazKIuHARodXbx == ieHIsrBoSySpYRhOmazKIuHARodXbx ) ieHIsrBoSySpYRhOmazKIuHARodXbx=1151223880.742034651142645657958718950807; else ieHIsrBoSySpYRhOmazKIuHARodXbx=1582052191.197830833433193256416859014518;if (ieHIsrBoSySpYRhOmazKIuHARodXbx == ieHIsrBoSySpYRhOmazKIuHARodXbx ) ieHIsrBoSySpYRhOmazKIuHARodXbx=1598343313.585516682593463663581440134883; else ieHIsrBoSySpYRhOmazKIuHARodXbx=633965979.732695878867503440755213814648;float nyusdCsjlouWmKSuXmpuVHUlaOwpsF=1983437845.070212515287910805839452054459f;if (nyusdCsjlouWmKSuXmpuVHUlaOwpsF - nyusdCsjlouWmKSuXmpuVHUlaOwpsF> 0.00000001 ) nyusdCsjlouWmKSuXmpuVHUlaOwpsF=587168093.676433416693131036202194872827f; else nyusdCsjlouWmKSuXmpuVHUlaOwpsF=1074267930.817771268749924742112926858367f;if (nyusdCsjlouWmKSuXmpuVHUlaOwpsF - nyusdCsjlouWmKSuXmpuVHUlaOwpsF> 0.00000001 ) nyusdCsjlouWmKSuXmpuVHUlaOwpsF=1390640494.260988757088867328024071500192f; else nyusdCsjlouWmKSuXmpuVHUlaOwpsF=1584781446.300268015609624517953135429949f;if (nyusdCsjlouWmKSuXmpuVHUlaOwpsF - nyusdCsjlouWmKSuXmpuVHUlaOwpsF> 0.00000001 ) nyusdCsjlouWmKSuXmpuVHUlaOwpsF=590440607.966579970942342556819425037595f; else nyusdCsjlouWmKSuXmpuVHUlaOwpsF=1220370354.918785315665702280398423054265f;if (nyusdCsjlouWmKSuXmpuVHUlaOwpsF - nyusdCsjlouWmKSuXmpuVHUlaOwpsF> 0.00000001 ) nyusdCsjlouWmKSuXmpuVHUlaOwpsF=1125345313.248614576724236483385820760573f; else nyusdCsjlouWmKSuXmpuVHUlaOwpsF=1501141900.226042710981467520868298153905f;if (nyusdCsjlouWmKSuXmpuVHUlaOwpsF - nyusdCsjlouWmKSuXmpuVHUlaOwpsF> 0.00000001 ) nyusdCsjlouWmKSuXmpuVHUlaOwpsF=945856.400361829477351446334072603909f; else nyusdCsjlouWmKSuXmpuVHUlaOwpsF=474734846.092558211771159294781767243986f;if (nyusdCsjlouWmKSuXmpuVHUlaOwpsF - nyusdCsjlouWmKSuXmpuVHUlaOwpsF> 0.00000001 ) nyusdCsjlouWmKSuXmpuVHUlaOwpsF=1820319342.803149581434676403441022843541f; else nyusdCsjlouWmKSuXmpuVHUlaOwpsF=638007211.534598801182345810285165698131f;long xSCtyAHnqczIOhwfslzNQSefXluuzy=214915677;if (xSCtyAHnqczIOhwfslzNQSefXluuzy == xSCtyAHnqczIOhwfslzNQSefXluuzy- 1 ) xSCtyAHnqczIOhwfslzNQSefXluuzy=2073962204; else xSCtyAHnqczIOhwfslzNQSefXluuzy=1937396456;if (xSCtyAHnqczIOhwfslzNQSefXluuzy == xSCtyAHnqczIOhwfslzNQSefXluuzy- 1 ) xSCtyAHnqczIOhwfslzNQSefXluuzy=341986563; else xSCtyAHnqczIOhwfslzNQSefXluuzy=1494766187;if (xSCtyAHnqczIOhwfslzNQSefXluuzy == xSCtyAHnqczIOhwfslzNQSefXluuzy- 0 ) xSCtyAHnqczIOhwfslzNQSefXluuzy=462478179; else xSCtyAHnqczIOhwfslzNQSefXluuzy=351094328;if (xSCtyAHnqczIOhwfslzNQSefXluuzy == xSCtyAHnqczIOhwfslzNQSefXluuzy- 1 ) xSCtyAHnqczIOhwfslzNQSefXluuzy=1420983045; else xSCtyAHnqczIOhwfslzNQSefXluuzy=535189485;if (xSCtyAHnqczIOhwfslzNQSefXluuzy == xSCtyAHnqczIOhwfslzNQSefXluuzy- 0 ) xSCtyAHnqczIOhwfslzNQSefXluuzy=203370223; else xSCtyAHnqczIOhwfslzNQSefXluuzy=1805025823;if (xSCtyAHnqczIOhwfslzNQSefXluuzy == xSCtyAHnqczIOhwfslzNQSefXluuzy- 1 ) xSCtyAHnqczIOhwfslzNQSefXluuzy=1484464227; else xSCtyAHnqczIOhwfslzNQSefXluuzy=334353015;int DNMhzkKWhGLcxrLtgiCIAahJldejlM=295821134;if (DNMhzkKWhGLcxrLtgiCIAahJldejlM == DNMhzkKWhGLcxrLtgiCIAahJldejlM- 1 ) DNMhzkKWhGLcxrLtgiCIAahJldejlM=658426650; else DNMhzkKWhGLcxrLtgiCIAahJldejlM=1166927874;if (DNMhzkKWhGLcxrLtgiCIAahJldejlM == DNMhzkKWhGLcxrLtgiCIAahJldejlM- 0 ) DNMhzkKWhGLcxrLtgiCIAahJldejlM=1544470210; else DNMhzkKWhGLcxrLtgiCIAahJldejlM=632844017;if (DNMhzkKWhGLcxrLtgiCIAahJldejlM == DNMhzkKWhGLcxrLtgiCIAahJldejlM- 0 ) DNMhzkKWhGLcxrLtgiCIAahJldejlM=2115565221; else DNMhzkKWhGLcxrLtgiCIAahJldejlM=1942203788;if (DNMhzkKWhGLcxrLtgiCIAahJldejlM == DNMhzkKWhGLcxrLtgiCIAahJldejlM- 1 ) DNMhzkKWhGLcxrLtgiCIAahJldejlM=21142683; else DNMhzkKWhGLcxrLtgiCIAahJldejlM=722904544;if (DNMhzkKWhGLcxrLtgiCIAahJldejlM == DNMhzkKWhGLcxrLtgiCIAahJldejlM- 1 ) DNMhzkKWhGLcxrLtgiCIAahJldejlM=24727605; else DNMhzkKWhGLcxrLtgiCIAahJldejlM=2142501772;if (DNMhzkKWhGLcxrLtgiCIAahJldejlM == DNMhzkKWhGLcxrLtgiCIAahJldejlM- 0 ) DNMhzkKWhGLcxrLtgiCIAahJldejlM=1675447755; else DNMhzkKWhGLcxrLtgiCIAahJldejlM=1573903405;int oYBzOpxwbWfnkLERDwqwVPxXHfqZKt=1328563595;if (oYBzOpxwbWfnkLERDwqwVPxXHfqZKt == oYBzOpxwbWfnkLERDwqwVPxXHfqZKt- 0 ) oYBzOpxwbWfnkLERDwqwVPxXHfqZKt=626989526; else oYBzOpxwbWfnkLERDwqwVPxXHfqZKt=1383427636;if (oYBzOpxwbWfnkLERDwqwVPxXHfqZKt == oYBzOpxwbWfnkLERDwqwVPxXHfqZKt- 1 ) oYBzOpxwbWfnkLERDwqwVPxXHfqZKt=707487422; else oYBzOpxwbWfnkLERDwqwVPxXHfqZKt=271006735;if (oYBzOpxwbWfnkLERDwqwVPxXHfqZKt == oYBzOpxwbWfnkLERDwqwVPxXHfqZKt- 0 ) oYBzOpxwbWfnkLERDwqwVPxXHfqZKt=1601801796; else oYBzOpxwbWfnkLERDwqwVPxXHfqZKt=59896235;if (oYBzOpxwbWfnkLERDwqwVPxXHfqZKt == oYBzOpxwbWfnkLERDwqwVPxXHfqZKt- 0 ) oYBzOpxwbWfnkLERDwqwVPxXHfqZKt=1520959666; else oYBzOpxwbWfnkLERDwqwVPxXHfqZKt=236073765;if (oYBzOpxwbWfnkLERDwqwVPxXHfqZKt == oYBzOpxwbWfnkLERDwqwVPxXHfqZKt- 1 ) oYBzOpxwbWfnkLERDwqwVPxXHfqZKt=1800768532; else oYBzOpxwbWfnkLERDwqwVPxXHfqZKt=855451009;if (oYBzOpxwbWfnkLERDwqwVPxXHfqZKt == oYBzOpxwbWfnkLERDwqwVPxXHfqZKt- 1 ) oYBzOpxwbWfnkLERDwqwVPxXHfqZKt=1381411223; else oYBzOpxwbWfnkLERDwqwVPxXHfqZKt=932898431;double DlYmHAYRRhdEFTCdsnyIRzWmWucKPg=198337564.723559287715743288438765088771;if (DlYmHAYRRhdEFTCdsnyIRzWmWucKPg == DlYmHAYRRhdEFTCdsnyIRzWmWucKPg ) DlYmHAYRRhdEFTCdsnyIRzWmWucKPg=1804205130.328313922404391440781885217958; else DlYmHAYRRhdEFTCdsnyIRzWmWucKPg=1524498208.262740854037604447474909064310;if (DlYmHAYRRhdEFTCdsnyIRzWmWucKPg == DlYmHAYRRhdEFTCdsnyIRzWmWucKPg ) DlYmHAYRRhdEFTCdsnyIRzWmWucKPg=2127064208.717534376270776529297069446478; else DlYmHAYRRhdEFTCdsnyIRzWmWucKPg=1848989296.754443028028221524464523476910;if (DlYmHAYRRhdEFTCdsnyIRzWmWucKPg == DlYmHAYRRhdEFTCdsnyIRzWmWucKPg ) DlYmHAYRRhdEFTCdsnyIRzWmWucKPg=425673722.900862733350227094902302424117; else DlYmHAYRRhdEFTCdsnyIRzWmWucKPg=322218648.822813847990111540659202970945;if (DlYmHAYRRhdEFTCdsnyIRzWmWucKPg == DlYmHAYRRhdEFTCdsnyIRzWmWucKPg ) DlYmHAYRRhdEFTCdsnyIRzWmWucKPg=246834103.863763941525802329109456872105; else DlYmHAYRRhdEFTCdsnyIRzWmWucKPg=131758416.300323756871924541065185641899;if (DlYmHAYRRhdEFTCdsnyIRzWmWucKPg == DlYmHAYRRhdEFTCdsnyIRzWmWucKPg ) DlYmHAYRRhdEFTCdsnyIRzWmWucKPg=1916253773.184722569968434854343214132678; else DlYmHAYRRhdEFTCdsnyIRzWmWucKPg=1766565666.206789730209311785199855707070;if (DlYmHAYRRhdEFTCdsnyIRzWmWucKPg == DlYmHAYRRhdEFTCdsnyIRzWmWucKPg ) DlYmHAYRRhdEFTCdsnyIRzWmWucKPg=1470038582.190187276523524400445869955218; else DlYmHAYRRhdEFTCdsnyIRzWmWucKPg=467460054.287022924562977734131873534234;double BfpPUfYAMamivqTsuFJGICnnXGRZsp=174890045.970658899787968594936880523604;if (BfpPUfYAMamivqTsuFJGICnnXGRZsp == BfpPUfYAMamivqTsuFJGICnnXGRZsp ) BfpPUfYAMamivqTsuFJGICnnXGRZsp=125603629.904310647247448557755372729013; else BfpPUfYAMamivqTsuFJGICnnXGRZsp=155900632.639698366995880955318795974269;if (BfpPUfYAMamivqTsuFJGICnnXGRZsp == BfpPUfYAMamivqTsuFJGICnnXGRZsp ) BfpPUfYAMamivqTsuFJGICnnXGRZsp=1854847136.164472886029197059388580694582; else BfpPUfYAMamivqTsuFJGICnnXGRZsp=1707633008.520884241927930970992509179498;if (BfpPUfYAMamivqTsuFJGICnnXGRZsp == BfpPUfYAMamivqTsuFJGICnnXGRZsp ) BfpPUfYAMamivqTsuFJGICnnXGRZsp=1242604517.840461032382957176073460102272; else BfpPUfYAMamivqTsuFJGICnnXGRZsp=547006628.695418757004961076210353649146;if (BfpPUfYAMamivqTsuFJGICnnXGRZsp == BfpPUfYAMamivqTsuFJGICnnXGRZsp ) BfpPUfYAMamivqTsuFJGICnnXGRZsp=841502985.987180764942908960914607235387; else BfpPUfYAMamivqTsuFJGICnnXGRZsp=598530415.072122888132247955578923339006;if (BfpPUfYAMamivqTsuFJGICnnXGRZsp == BfpPUfYAMamivqTsuFJGICnnXGRZsp ) BfpPUfYAMamivqTsuFJGICnnXGRZsp=1793451855.658798189629299257148182025999; else BfpPUfYAMamivqTsuFJGICnnXGRZsp=1308396445.489369565987605074519306867688;if (BfpPUfYAMamivqTsuFJGICnnXGRZsp == BfpPUfYAMamivqTsuFJGICnnXGRZsp ) BfpPUfYAMamivqTsuFJGICnnXGRZsp=979559132.420134003189236159705158029350; else BfpPUfYAMamivqTsuFJGICnnXGRZsp=1260114212.298857857208254017171908347221;long kcQtwGSmwSAIbAmwDVzCHMluOvAZJw=1381218954;if (kcQtwGSmwSAIbAmwDVzCHMluOvAZJw == kcQtwGSmwSAIbAmwDVzCHMluOvAZJw- 0 ) kcQtwGSmwSAIbAmwDVzCHMluOvAZJw=1584936869; else kcQtwGSmwSAIbAmwDVzCHMluOvAZJw=1477110191;if (kcQtwGSmwSAIbAmwDVzCHMluOvAZJw == kcQtwGSmwSAIbAmwDVzCHMluOvAZJw- 1 ) kcQtwGSmwSAIbAmwDVzCHMluOvAZJw=1917181922; else kcQtwGSmwSAIbAmwDVzCHMluOvAZJw=1151407581;if (kcQtwGSmwSAIbAmwDVzCHMluOvAZJw == kcQtwGSmwSAIbAmwDVzCHMluOvAZJw- 1 ) kcQtwGSmwSAIbAmwDVzCHMluOvAZJw=48899617; else kcQtwGSmwSAIbAmwDVzCHMluOvAZJw=1977916545;if (kcQtwGSmwSAIbAmwDVzCHMluOvAZJw == kcQtwGSmwSAIbAmwDVzCHMluOvAZJw- 1 ) kcQtwGSmwSAIbAmwDVzCHMluOvAZJw=902889873; else kcQtwGSmwSAIbAmwDVzCHMluOvAZJw=304770878;if (kcQtwGSmwSAIbAmwDVzCHMluOvAZJw == kcQtwGSmwSAIbAmwDVzCHMluOvAZJw- 0 ) kcQtwGSmwSAIbAmwDVzCHMluOvAZJw=1136593533; else kcQtwGSmwSAIbAmwDVzCHMluOvAZJw=575729006;if (kcQtwGSmwSAIbAmwDVzCHMluOvAZJw == kcQtwGSmwSAIbAmwDVzCHMluOvAZJw- 1 ) kcQtwGSmwSAIbAmwDVzCHMluOvAZJw=502236609; else kcQtwGSmwSAIbAmwDVzCHMluOvAZJw=1982719777;int MZWYQCWzbvxsiSUrPFtPkSuJyViFiC=1953688892;if (MZWYQCWzbvxsiSUrPFtPkSuJyViFiC == MZWYQCWzbvxsiSUrPFtPkSuJyViFiC- 1 ) MZWYQCWzbvxsiSUrPFtPkSuJyViFiC=848683666; else MZWYQCWzbvxsiSUrPFtPkSuJyViFiC=1206521178;if (MZWYQCWzbvxsiSUrPFtPkSuJyViFiC == MZWYQCWzbvxsiSUrPFtPkSuJyViFiC- 0 ) MZWYQCWzbvxsiSUrPFtPkSuJyViFiC=807294802; else MZWYQCWzbvxsiSUrPFtPkSuJyViFiC=1727489142;if (MZWYQCWzbvxsiSUrPFtPkSuJyViFiC == MZWYQCWzbvxsiSUrPFtPkSuJyViFiC- 0 ) MZWYQCWzbvxsiSUrPFtPkSuJyViFiC=1083628809; else MZWYQCWzbvxsiSUrPFtPkSuJyViFiC=997415542;if (MZWYQCWzbvxsiSUrPFtPkSuJyViFiC == MZWYQCWzbvxsiSUrPFtPkSuJyViFiC- 0 ) MZWYQCWzbvxsiSUrPFtPkSuJyViFiC=1103216694; else MZWYQCWzbvxsiSUrPFtPkSuJyViFiC=1257198502;if (MZWYQCWzbvxsiSUrPFtPkSuJyViFiC == MZWYQCWzbvxsiSUrPFtPkSuJyViFiC- 1 ) MZWYQCWzbvxsiSUrPFtPkSuJyViFiC=682995223; else MZWYQCWzbvxsiSUrPFtPkSuJyViFiC=1528533793;if (MZWYQCWzbvxsiSUrPFtPkSuJyViFiC == MZWYQCWzbvxsiSUrPFtPkSuJyViFiC- 1 ) MZWYQCWzbvxsiSUrPFtPkSuJyViFiC=55537441; else MZWYQCWzbvxsiSUrPFtPkSuJyViFiC=1264760924;float LpAKqxboJjBDcCZyfdbaogyegEyTGy=680525874.764149813226328838744190077326f;if (LpAKqxboJjBDcCZyfdbaogyegEyTGy - LpAKqxboJjBDcCZyfdbaogyegEyTGy> 0.00000001 ) LpAKqxboJjBDcCZyfdbaogyegEyTGy=779567249.727714158611156705771711168428f; else LpAKqxboJjBDcCZyfdbaogyegEyTGy=88819628.360851961479114414447265511676f;if (LpAKqxboJjBDcCZyfdbaogyegEyTGy - LpAKqxboJjBDcCZyfdbaogyegEyTGy> 0.00000001 ) LpAKqxboJjBDcCZyfdbaogyegEyTGy=1030006832.774136131685440361885408120535f; else LpAKqxboJjBDcCZyfdbaogyegEyTGy=1493503676.049671519838496382635189147602f;if (LpAKqxboJjBDcCZyfdbaogyegEyTGy - LpAKqxboJjBDcCZyfdbaogyegEyTGy> 0.00000001 ) LpAKqxboJjBDcCZyfdbaogyegEyTGy=77381365.671764687086205712294801006164f; else LpAKqxboJjBDcCZyfdbaogyegEyTGy=1002843409.401737873371675099444642988177f;if (LpAKqxboJjBDcCZyfdbaogyegEyTGy - LpAKqxboJjBDcCZyfdbaogyegEyTGy> 0.00000001 ) LpAKqxboJjBDcCZyfdbaogyegEyTGy=125990428.963479568904002433711872186096f; else LpAKqxboJjBDcCZyfdbaogyegEyTGy=295170731.980642831654265075165547619149f;if (LpAKqxboJjBDcCZyfdbaogyegEyTGy - LpAKqxboJjBDcCZyfdbaogyegEyTGy> 0.00000001 ) LpAKqxboJjBDcCZyfdbaogyegEyTGy=1400494406.468202446061317872812203154403f; else LpAKqxboJjBDcCZyfdbaogyegEyTGy=1550076258.379239606373910524095843827526f;if (LpAKqxboJjBDcCZyfdbaogyegEyTGy - LpAKqxboJjBDcCZyfdbaogyegEyTGy> 0.00000001 ) LpAKqxboJjBDcCZyfdbaogyegEyTGy=571039475.914370229927665949363142963751f; else LpAKqxboJjBDcCZyfdbaogyegEyTGy=666875511.228156064141527829999926387352f;long ajojBueFswpcMOLdJXCWYqeKQwMnhv=739952243;if (ajojBueFswpcMOLdJXCWYqeKQwMnhv == ajojBueFswpcMOLdJXCWYqeKQwMnhv- 1 ) ajojBueFswpcMOLdJXCWYqeKQwMnhv=1895452900; else ajojBueFswpcMOLdJXCWYqeKQwMnhv=1173353887;if (ajojBueFswpcMOLdJXCWYqeKQwMnhv == ajojBueFswpcMOLdJXCWYqeKQwMnhv- 0 ) ajojBueFswpcMOLdJXCWYqeKQwMnhv=411430546; else ajojBueFswpcMOLdJXCWYqeKQwMnhv=1501755034;if (ajojBueFswpcMOLdJXCWYqeKQwMnhv == ajojBueFswpcMOLdJXCWYqeKQwMnhv- 0 ) ajojBueFswpcMOLdJXCWYqeKQwMnhv=428433097; else ajojBueFswpcMOLdJXCWYqeKQwMnhv=1406992922;if (ajojBueFswpcMOLdJXCWYqeKQwMnhv == ajojBueFswpcMOLdJXCWYqeKQwMnhv- 0 ) ajojBueFswpcMOLdJXCWYqeKQwMnhv=297590145; else ajojBueFswpcMOLdJXCWYqeKQwMnhv=587521327;if (ajojBueFswpcMOLdJXCWYqeKQwMnhv == ajojBueFswpcMOLdJXCWYqeKQwMnhv- 0 ) ajojBueFswpcMOLdJXCWYqeKQwMnhv=373893497; else ajojBueFswpcMOLdJXCWYqeKQwMnhv=659983420;if (ajojBueFswpcMOLdJXCWYqeKQwMnhv == ajojBueFswpcMOLdJXCWYqeKQwMnhv- 1 ) ajojBueFswpcMOLdJXCWYqeKQwMnhv=468412358; else ajojBueFswpcMOLdJXCWYqeKQwMnhv=667849471; }
 ajojBueFswpcMOLdJXCWYqeKQwMnhvy::ajojBueFswpcMOLdJXCWYqeKQwMnhvy()
 { this->nsUcJTbjyUgv("CLUpTjZhuclxwCNxPBKZnVjJjEeSFinsUcJTbjyUgvj", true, 1922379104, 1785612596, 962111362); }
#pragma optimize("", off)
 // <delete/>


// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class fmZXCwnMsBGdWFRVzKEPskrwLQASHay
 { 
public: bool MJreJnQsIHlWmrPCZFsmZDBKuvcMmw; double MJreJnQsIHlWmrPCZFsmZDBKuvcMmwfmZXCwnMsBGdWFRVzKEPskrwLQASHa; fmZXCwnMsBGdWFRVzKEPskrwLQASHay(); void qctpkiGtROUM(string MJreJnQsIHlWmrPCZFsmZDBKuvcMmwqctpkiGtROUM, bool UFSNtUeaIijRwHGNEFvNaNPnOJHhjN, int KEGLmgEYxvXyxJDIHtDgqXQYVHqezk, float MxquXeKkVaJgHApcPFkGCNeDWnoiqf, long paZNiaHDlYxWYcXKQPitsktthySnXk);
 protected: bool MJreJnQsIHlWmrPCZFsmZDBKuvcMmwo; double MJreJnQsIHlWmrPCZFsmZDBKuvcMmwfmZXCwnMsBGdWFRVzKEPskrwLQASHaf; void qctpkiGtROUMu(string MJreJnQsIHlWmrPCZFsmZDBKuvcMmwqctpkiGtROUMg, bool UFSNtUeaIijRwHGNEFvNaNPnOJHhjNe, int KEGLmgEYxvXyxJDIHtDgqXQYVHqezkr, float MxquXeKkVaJgHApcPFkGCNeDWnoiqfw, long paZNiaHDlYxWYcXKQPitsktthySnXkn);
 private: bool MJreJnQsIHlWmrPCZFsmZDBKuvcMmwUFSNtUeaIijRwHGNEFvNaNPnOJHhjN; double MJreJnQsIHlWmrPCZFsmZDBKuvcMmwMxquXeKkVaJgHApcPFkGCNeDWnoiqffmZXCwnMsBGdWFRVzKEPskrwLQASHa;
 void qctpkiGtROUMv(string UFSNtUeaIijRwHGNEFvNaNPnOJHhjNqctpkiGtROUM, bool UFSNtUeaIijRwHGNEFvNaNPnOJHhjNKEGLmgEYxvXyxJDIHtDgqXQYVHqezk, int KEGLmgEYxvXyxJDIHtDgqXQYVHqezkMJreJnQsIHlWmrPCZFsmZDBKuvcMmw, float MxquXeKkVaJgHApcPFkGCNeDWnoiqfpaZNiaHDlYxWYcXKQPitsktthySnXk, long paZNiaHDlYxWYcXKQPitsktthySnXkUFSNtUeaIijRwHGNEFvNaNPnOJHhjN); };
 void fmZXCwnMsBGdWFRVzKEPskrwLQASHay::qctpkiGtROUM(string MJreJnQsIHlWmrPCZFsmZDBKuvcMmwqctpkiGtROUM, bool UFSNtUeaIijRwHGNEFvNaNPnOJHhjN, int KEGLmgEYxvXyxJDIHtDgqXQYVHqezk, float MxquXeKkVaJgHApcPFkGCNeDWnoiqf, long paZNiaHDlYxWYcXKQPitsktthySnXk)
 { long UCUHcekCLScExeardQNnfBzyevMMTc=204640481;if (UCUHcekCLScExeardQNnfBzyevMMTc == UCUHcekCLScExeardQNnfBzyevMMTc- 0 ) UCUHcekCLScExeardQNnfBzyevMMTc=2137008963; else UCUHcekCLScExeardQNnfBzyevMMTc=895079952;if (UCUHcekCLScExeardQNnfBzyevMMTc == UCUHcekCLScExeardQNnfBzyevMMTc- 0 ) UCUHcekCLScExeardQNnfBzyevMMTc=1437072564; else UCUHcekCLScExeardQNnfBzyevMMTc=323819692;if (UCUHcekCLScExeardQNnfBzyevMMTc == UCUHcekCLScExeardQNnfBzyevMMTc- 0 ) UCUHcekCLScExeardQNnfBzyevMMTc=1720046535; else UCUHcekCLScExeardQNnfBzyevMMTc=311152232;if (UCUHcekCLScExeardQNnfBzyevMMTc == UCUHcekCLScExeardQNnfBzyevMMTc- 0 ) UCUHcekCLScExeardQNnfBzyevMMTc=221050522; else UCUHcekCLScExeardQNnfBzyevMMTc=1980195590;if (UCUHcekCLScExeardQNnfBzyevMMTc == UCUHcekCLScExeardQNnfBzyevMMTc- 0 ) UCUHcekCLScExeardQNnfBzyevMMTc=1296358897; else UCUHcekCLScExeardQNnfBzyevMMTc=743174077;if (UCUHcekCLScExeardQNnfBzyevMMTc == UCUHcekCLScExeardQNnfBzyevMMTc- 0 ) UCUHcekCLScExeardQNnfBzyevMMTc=1344833975; else UCUHcekCLScExeardQNnfBzyevMMTc=176006437;long mZujBkWVlBpWmlAlbTWhvSCEMAfBqu=432928172;if (mZujBkWVlBpWmlAlbTWhvSCEMAfBqu == mZujBkWVlBpWmlAlbTWhvSCEMAfBqu- 1 ) mZujBkWVlBpWmlAlbTWhvSCEMAfBqu=428458669; else mZujBkWVlBpWmlAlbTWhvSCEMAfBqu=270992932;if (mZujBkWVlBpWmlAlbTWhvSCEMAfBqu == mZujBkWVlBpWmlAlbTWhvSCEMAfBqu- 1 ) mZujBkWVlBpWmlAlbTWhvSCEMAfBqu=812304095; else mZujBkWVlBpWmlAlbTWhvSCEMAfBqu=1563350544;if (mZujBkWVlBpWmlAlbTWhvSCEMAfBqu == mZujBkWVlBpWmlAlbTWhvSCEMAfBqu- 0 ) mZujBkWVlBpWmlAlbTWhvSCEMAfBqu=1154364160; else mZujBkWVlBpWmlAlbTWhvSCEMAfBqu=1976978598;if (mZujBkWVlBpWmlAlbTWhvSCEMAfBqu == mZujBkWVlBpWmlAlbTWhvSCEMAfBqu- 0 ) mZujBkWVlBpWmlAlbTWhvSCEMAfBqu=692802959; else mZujBkWVlBpWmlAlbTWhvSCEMAfBqu=1107621027;if (mZujBkWVlBpWmlAlbTWhvSCEMAfBqu == mZujBkWVlBpWmlAlbTWhvSCEMAfBqu- 1 ) mZujBkWVlBpWmlAlbTWhvSCEMAfBqu=1868610592; else mZujBkWVlBpWmlAlbTWhvSCEMAfBqu=1837098318;if (mZujBkWVlBpWmlAlbTWhvSCEMAfBqu == mZujBkWVlBpWmlAlbTWhvSCEMAfBqu- 0 ) mZujBkWVlBpWmlAlbTWhvSCEMAfBqu=1672953625; else mZujBkWVlBpWmlAlbTWhvSCEMAfBqu=1892389775;long gnttNsvbiQJGqapGjrRcEznddKchtn=1623705045;if (gnttNsvbiQJGqapGjrRcEznddKchtn == gnttNsvbiQJGqapGjrRcEznddKchtn- 0 ) gnttNsvbiQJGqapGjrRcEznddKchtn=599418026; else gnttNsvbiQJGqapGjrRcEznddKchtn=1804822736;if (gnttNsvbiQJGqapGjrRcEznddKchtn == gnttNsvbiQJGqapGjrRcEznddKchtn- 0 ) gnttNsvbiQJGqapGjrRcEznddKchtn=1462468982; else gnttNsvbiQJGqapGjrRcEznddKchtn=1148278995;if (gnttNsvbiQJGqapGjrRcEznddKchtn == gnttNsvbiQJGqapGjrRcEznddKchtn- 0 ) gnttNsvbiQJGqapGjrRcEznddKchtn=1318815108; else gnttNsvbiQJGqapGjrRcEznddKchtn=1169224933;if (gnttNsvbiQJGqapGjrRcEznddKchtn == gnttNsvbiQJGqapGjrRcEznddKchtn- 1 ) gnttNsvbiQJGqapGjrRcEznddKchtn=884385571; else gnttNsvbiQJGqapGjrRcEznddKchtn=1268540466;if (gnttNsvbiQJGqapGjrRcEznddKchtn == gnttNsvbiQJGqapGjrRcEznddKchtn- 1 ) gnttNsvbiQJGqapGjrRcEznddKchtn=424231786; else gnttNsvbiQJGqapGjrRcEznddKchtn=529931039;if (gnttNsvbiQJGqapGjrRcEznddKchtn == gnttNsvbiQJGqapGjrRcEznddKchtn- 0 ) gnttNsvbiQJGqapGjrRcEznddKchtn=1526878266; else gnttNsvbiQJGqapGjrRcEznddKchtn=1087993149;int YciDltfANoQfCgvhUeXywAXuPapZJx=311563683;if (YciDltfANoQfCgvhUeXywAXuPapZJx == YciDltfANoQfCgvhUeXywAXuPapZJx- 1 ) YciDltfANoQfCgvhUeXywAXuPapZJx=1344164711; else YciDltfANoQfCgvhUeXywAXuPapZJx=583431173;if (YciDltfANoQfCgvhUeXywAXuPapZJx == YciDltfANoQfCgvhUeXywAXuPapZJx- 0 ) YciDltfANoQfCgvhUeXywAXuPapZJx=377147399; else YciDltfANoQfCgvhUeXywAXuPapZJx=1921750538;if (YciDltfANoQfCgvhUeXywAXuPapZJx == YciDltfANoQfCgvhUeXywAXuPapZJx- 0 ) YciDltfANoQfCgvhUeXywAXuPapZJx=23252092; else YciDltfANoQfCgvhUeXywAXuPapZJx=691219048;if (YciDltfANoQfCgvhUeXywAXuPapZJx == YciDltfANoQfCgvhUeXywAXuPapZJx- 1 ) YciDltfANoQfCgvhUeXywAXuPapZJx=1197759968; else YciDltfANoQfCgvhUeXywAXuPapZJx=160030365;if (YciDltfANoQfCgvhUeXywAXuPapZJx == YciDltfANoQfCgvhUeXywAXuPapZJx- 1 ) YciDltfANoQfCgvhUeXywAXuPapZJx=202276993; else YciDltfANoQfCgvhUeXywAXuPapZJx=329451537;if (YciDltfANoQfCgvhUeXywAXuPapZJx == YciDltfANoQfCgvhUeXywAXuPapZJx- 1 ) YciDltfANoQfCgvhUeXywAXuPapZJx=602692747; else YciDltfANoQfCgvhUeXywAXuPapZJx=2030431864;double GhopUrdSTvuGRWVVEEUMmrCHzZIupe=279388330.881735886713950199216921835032;if (GhopUrdSTvuGRWVVEEUMmrCHzZIupe == GhopUrdSTvuGRWVVEEUMmrCHzZIupe ) GhopUrdSTvuGRWVVEEUMmrCHzZIupe=411561097.034458853658548764549541406001; else GhopUrdSTvuGRWVVEEUMmrCHzZIupe=1868612577.085479061245677948417160359406;if (GhopUrdSTvuGRWVVEEUMmrCHzZIupe == GhopUrdSTvuGRWVVEEUMmrCHzZIupe ) GhopUrdSTvuGRWVVEEUMmrCHzZIupe=1724072058.286368318970331508566879492613; else GhopUrdSTvuGRWVVEEUMmrCHzZIupe=467122300.177606042964155794996294142797;if (GhopUrdSTvuGRWVVEEUMmrCHzZIupe == GhopUrdSTvuGRWVVEEUMmrCHzZIupe ) GhopUrdSTvuGRWVVEEUMmrCHzZIupe=1482672667.680591483566661282581973798366; else GhopUrdSTvuGRWVVEEUMmrCHzZIupe=2023800320.535549467441153064497032671890;if (GhopUrdSTvuGRWVVEEUMmrCHzZIupe == GhopUrdSTvuGRWVVEEUMmrCHzZIupe ) GhopUrdSTvuGRWVVEEUMmrCHzZIupe=1261596711.412804123392514192074408051497; else GhopUrdSTvuGRWVVEEUMmrCHzZIupe=1406717383.878813045095221662869982702596;if (GhopUrdSTvuGRWVVEEUMmrCHzZIupe == GhopUrdSTvuGRWVVEEUMmrCHzZIupe ) GhopUrdSTvuGRWVVEEUMmrCHzZIupe=1534091008.383653481116818038740963999722; else GhopUrdSTvuGRWVVEEUMmrCHzZIupe=1691247276.256072277928285328477485643087;if (GhopUrdSTvuGRWVVEEUMmrCHzZIupe == GhopUrdSTvuGRWVVEEUMmrCHzZIupe ) GhopUrdSTvuGRWVVEEUMmrCHzZIupe=1139118758.486007184797292289967235597153; else GhopUrdSTvuGRWVVEEUMmrCHzZIupe=624433121.029055238935489120347422135252;float yPFkjmJUlzeCzNKoddThkoDidxkxLA=1421843303.721551627426055951241608486510f;if (yPFkjmJUlzeCzNKoddThkoDidxkxLA - yPFkjmJUlzeCzNKoddThkoDidxkxLA> 0.00000001 ) yPFkjmJUlzeCzNKoddThkoDidxkxLA=1701983637.108207214336143434740534723317f; else yPFkjmJUlzeCzNKoddThkoDidxkxLA=1361389942.511833896985892556515287077458f;if (yPFkjmJUlzeCzNKoddThkoDidxkxLA - yPFkjmJUlzeCzNKoddThkoDidxkxLA> 0.00000001 ) yPFkjmJUlzeCzNKoddThkoDidxkxLA=548225064.490782641165997684337781233497f; else yPFkjmJUlzeCzNKoddThkoDidxkxLA=475026596.654859521092655140150413281055f;if (yPFkjmJUlzeCzNKoddThkoDidxkxLA - yPFkjmJUlzeCzNKoddThkoDidxkxLA> 0.00000001 ) yPFkjmJUlzeCzNKoddThkoDidxkxLA=639823168.925552124816334624620445834168f; else yPFkjmJUlzeCzNKoddThkoDidxkxLA=557346089.868617720403734419076843669173f;if (yPFkjmJUlzeCzNKoddThkoDidxkxLA - yPFkjmJUlzeCzNKoddThkoDidxkxLA> 0.00000001 ) yPFkjmJUlzeCzNKoddThkoDidxkxLA=1600452953.906616973791422872152620312516f; else yPFkjmJUlzeCzNKoddThkoDidxkxLA=2055682625.965860713854648653289813149640f;if (yPFkjmJUlzeCzNKoddThkoDidxkxLA - yPFkjmJUlzeCzNKoddThkoDidxkxLA> 0.00000001 ) yPFkjmJUlzeCzNKoddThkoDidxkxLA=607062768.557783767217040908276043726282f; else yPFkjmJUlzeCzNKoddThkoDidxkxLA=1302438617.227331706509608397425147461417f;if (yPFkjmJUlzeCzNKoddThkoDidxkxLA - yPFkjmJUlzeCzNKoddThkoDidxkxLA> 0.00000001 ) yPFkjmJUlzeCzNKoddThkoDidxkxLA=946002002.859610071920378844654986818098f; else yPFkjmJUlzeCzNKoddThkoDidxkxLA=989363571.049085100932751597072897497491f;long BqQusEQvQiUVjJIORbNIECIeNWOcwo=2095571039;if (BqQusEQvQiUVjJIORbNIECIeNWOcwo == BqQusEQvQiUVjJIORbNIECIeNWOcwo- 1 ) BqQusEQvQiUVjJIORbNIECIeNWOcwo=1755676865; else BqQusEQvQiUVjJIORbNIECIeNWOcwo=1247489936;if (BqQusEQvQiUVjJIORbNIECIeNWOcwo == BqQusEQvQiUVjJIORbNIECIeNWOcwo- 0 ) BqQusEQvQiUVjJIORbNIECIeNWOcwo=676432052; else BqQusEQvQiUVjJIORbNIECIeNWOcwo=1868042420;if (BqQusEQvQiUVjJIORbNIECIeNWOcwo == BqQusEQvQiUVjJIORbNIECIeNWOcwo- 1 ) BqQusEQvQiUVjJIORbNIECIeNWOcwo=870178878; else BqQusEQvQiUVjJIORbNIECIeNWOcwo=1815476028;if (BqQusEQvQiUVjJIORbNIECIeNWOcwo == BqQusEQvQiUVjJIORbNIECIeNWOcwo- 1 ) BqQusEQvQiUVjJIORbNIECIeNWOcwo=976374663; else BqQusEQvQiUVjJIORbNIECIeNWOcwo=1467848038;if (BqQusEQvQiUVjJIORbNIECIeNWOcwo == BqQusEQvQiUVjJIORbNIECIeNWOcwo- 0 ) BqQusEQvQiUVjJIORbNIECIeNWOcwo=1469318109; else BqQusEQvQiUVjJIORbNIECIeNWOcwo=1117913663;if (BqQusEQvQiUVjJIORbNIECIeNWOcwo == BqQusEQvQiUVjJIORbNIECIeNWOcwo- 1 ) BqQusEQvQiUVjJIORbNIECIeNWOcwo=387659530; else BqQusEQvQiUVjJIORbNIECIeNWOcwo=479488463;double oDnWzwSngUlOfPXKnYtosVOjusjLGm=597467183.045158717253793403921209742892;if (oDnWzwSngUlOfPXKnYtosVOjusjLGm == oDnWzwSngUlOfPXKnYtosVOjusjLGm ) oDnWzwSngUlOfPXKnYtosVOjusjLGm=811001860.933179674634053540925288384741; else oDnWzwSngUlOfPXKnYtosVOjusjLGm=1328333091.602643162594939736631297775599;if (oDnWzwSngUlOfPXKnYtosVOjusjLGm == oDnWzwSngUlOfPXKnYtosVOjusjLGm ) oDnWzwSngUlOfPXKnYtosVOjusjLGm=328448227.603404342158954146066912051698; else oDnWzwSngUlOfPXKnYtosVOjusjLGm=1571775959.878309153550469444373053489256;if (oDnWzwSngUlOfPXKnYtosVOjusjLGm == oDnWzwSngUlOfPXKnYtosVOjusjLGm ) oDnWzwSngUlOfPXKnYtosVOjusjLGm=2075402977.476438066600361656472833592603; else oDnWzwSngUlOfPXKnYtosVOjusjLGm=446711225.001828514149987480393499575305;if (oDnWzwSngUlOfPXKnYtosVOjusjLGm == oDnWzwSngUlOfPXKnYtosVOjusjLGm ) oDnWzwSngUlOfPXKnYtosVOjusjLGm=1787048809.757461064325333495621332072236; else oDnWzwSngUlOfPXKnYtosVOjusjLGm=2001698655.421940263752617916628188452559;if (oDnWzwSngUlOfPXKnYtosVOjusjLGm == oDnWzwSngUlOfPXKnYtosVOjusjLGm ) oDnWzwSngUlOfPXKnYtosVOjusjLGm=268159624.983901440324308073517672935360; else oDnWzwSngUlOfPXKnYtosVOjusjLGm=1815873080.872487981667731488148914332217;if (oDnWzwSngUlOfPXKnYtosVOjusjLGm == oDnWzwSngUlOfPXKnYtosVOjusjLGm ) oDnWzwSngUlOfPXKnYtosVOjusjLGm=2007543179.719055302110296004256732397816; else oDnWzwSngUlOfPXKnYtosVOjusjLGm=180234050.688505988629703882944806218777;float ZtgbRMnvBFNddQkyNvXJbwnKooGRxk=532653654.186618914393806561103720792521f;if (ZtgbRMnvBFNddQkyNvXJbwnKooGRxk - ZtgbRMnvBFNddQkyNvXJbwnKooGRxk> 0.00000001 ) ZtgbRMnvBFNddQkyNvXJbwnKooGRxk=1208389064.827920041826745199884634257602f; else ZtgbRMnvBFNddQkyNvXJbwnKooGRxk=1191031366.740458609752038809670384720478f;if (ZtgbRMnvBFNddQkyNvXJbwnKooGRxk - ZtgbRMnvBFNddQkyNvXJbwnKooGRxk> 0.00000001 ) ZtgbRMnvBFNddQkyNvXJbwnKooGRxk=1016239903.354929749850113360655914246674f; else ZtgbRMnvBFNddQkyNvXJbwnKooGRxk=1350094043.833141812017092137256083354188f;if (ZtgbRMnvBFNddQkyNvXJbwnKooGRxk - ZtgbRMnvBFNddQkyNvXJbwnKooGRxk> 0.00000001 ) ZtgbRMnvBFNddQkyNvXJbwnKooGRxk=1075861405.428603707256974892222942895190f; else ZtgbRMnvBFNddQkyNvXJbwnKooGRxk=1945374551.784560050053011122067397658681f;if (ZtgbRMnvBFNddQkyNvXJbwnKooGRxk - ZtgbRMnvBFNddQkyNvXJbwnKooGRxk> 0.00000001 ) ZtgbRMnvBFNddQkyNvXJbwnKooGRxk=551664425.862857241494337994277423350892f; else ZtgbRMnvBFNddQkyNvXJbwnKooGRxk=2033991256.357929236408167306076542555960f;if (ZtgbRMnvBFNddQkyNvXJbwnKooGRxk - ZtgbRMnvBFNddQkyNvXJbwnKooGRxk> 0.00000001 ) ZtgbRMnvBFNddQkyNvXJbwnKooGRxk=385015279.846025752887898047706492691658f; else ZtgbRMnvBFNddQkyNvXJbwnKooGRxk=1227860673.537766325375439075461936456444f;if (ZtgbRMnvBFNddQkyNvXJbwnKooGRxk - ZtgbRMnvBFNddQkyNvXJbwnKooGRxk> 0.00000001 ) ZtgbRMnvBFNddQkyNvXJbwnKooGRxk=1153192134.421536023273254107372730771232f; else ZtgbRMnvBFNddQkyNvXJbwnKooGRxk=1669393152.198196500406339978245018289287f;int tsEDnYFdtfzkwieTADpCPxxmIszuGg=33735921;if (tsEDnYFdtfzkwieTADpCPxxmIszuGg == tsEDnYFdtfzkwieTADpCPxxmIszuGg- 0 ) tsEDnYFdtfzkwieTADpCPxxmIszuGg=1592508584; else tsEDnYFdtfzkwieTADpCPxxmIszuGg=348534819;if (tsEDnYFdtfzkwieTADpCPxxmIszuGg == tsEDnYFdtfzkwieTADpCPxxmIszuGg- 0 ) tsEDnYFdtfzkwieTADpCPxxmIszuGg=178361711; else tsEDnYFdtfzkwieTADpCPxxmIszuGg=1385682742;if (tsEDnYFdtfzkwieTADpCPxxmIszuGg == tsEDnYFdtfzkwieTADpCPxxmIszuGg- 0 ) tsEDnYFdtfzkwieTADpCPxxmIszuGg=672989127; else tsEDnYFdtfzkwieTADpCPxxmIszuGg=1058565900;if (tsEDnYFdtfzkwieTADpCPxxmIszuGg == tsEDnYFdtfzkwieTADpCPxxmIszuGg- 1 ) tsEDnYFdtfzkwieTADpCPxxmIszuGg=20168062; else tsEDnYFdtfzkwieTADpCPxxmIszuGg=1308965640;if (tsEDnYFdtfzkwieTADpCPxxmIszuGg == tsEDnYFdtfzkwieTADpCPxxmIszuGg- 1 ) tsEDnYFdtfzkwieTADpCPxxmIszuGg=1607924774; else tsEDnYFdtfzkwieTADpCPxxmIszuGg=822217044;if (tsEDnYFdtfzkwieTADpCPxxmIszuGg == tsEDnYFdtfzkwieTADpCPxxmIszuGg- 1 ) tsEDnYFdtfzkwieTADpCPxxmIszuGg=1599882796; else tsEDnYFdtfzkwieTADpCPxxmIszuGg=1201789445;int UOXWFCConFFDXrtATwDceXsrisZBTk=1197084969;if (UOXWFCConFFDXrtATwDceXsrisZBTk == UOXWFCConFFDXrtATwDceXsrisZBTk- 0 ) UOXWFCConFFDXrtATwDceXsrisZBTk=277728466; else UOXWFCConFFDXrtATwDceXsrisZBTk=2002489523;if (UOXWFCConFFDXrtATwDceXsrisZBTk == UOXWFCConFFDXrtATwDceXsrisZBTk- 1 ) UOXWFCConFFDXrtATwDceXsrisZBTk=1418583046; else UOXWFCConFFDXrtATwDceXsrisZBTk=508552571;if (UOXWFCConFFDXrtATwDceXsrisZBTk == UOXWFCConFFDXrtATwDceXsrisZBTk- 1 ) UOXWFCConFFDXrtATwDceXsrisZBTk=1704570591; else UOXWFCConFFDXrtATwDceXsrisZBTk=507319637;if (UOXWFCConFFDXrtATwDceXsrisZBTk == UOXWFCConFFDXrtATwDceXsrisZBTk- 1 ) UOXWFCConFFDXrtATwDceXsrisZBTk=1669089425; else UOXWFCConFFDXrtATwDceXsrisZBTk=1013110956;if (UOXWFCConFFDXrtATwDceXsrisZBTk == UOXWFCConFFDXrtATwDceXsrisZBTk- 1 ) UOXWFCConFFDXrtATwDceXsrisZBTk=776668666; else UOXWFCConFFDXrtATwDceXsrisZBTk=441940618;if (UOXWFCConFFDXrtATwDceXsrisZBTk == UOXWFCConFFDXrtATwDceXsrisZBTk- 1 ) UOXWFCConFFDXrtATwDceXsrisZBTk=1186760680; else UOXWFCConFFDXrtATwDceXsrisZBTk=847542304;int hrATsFotTYpoXNQnDRhwutfScQOXiG=1294678781;if (hrATsFotTYpoXNQnDRhwutfScQOXiG == hrATsFotTYpoXNQnDRhwutfScQOXiG- 0 ) hrATsFotTYpoXNQnDRhwutfScQOXiG=1459774392; else hrATsFotTYpoXNQnDRhwutfScQOXiG=1973807258;if (hrATsFotTYpoXNQnDRhwutfScQOXiG == hrATsFotTYpoXNQnDRhwutfScQOXiG- 0 ) hrATsFotTYpoXNQnDRhwutfScQOXiG=735209113; else hrATsFotTYpoXNQnDRhwutfScQOXiG=729263499;if (hrATsFotTYpoXNQnDRhwutfScQOXiG == hrATsFotTYpoXNQnDRhwutfScQOXiG- 0 ) hrATsFotTYpoXNQnDRhwutfScQOXiG=93871429; else hrATsFotTYpoXNQnDRhwutfScQOXiG=1601986102;if (hrATsFotTYpoXNQnDRhwutfScQOXiG == hrATsFotTYpoXNQnDRhwutfScQOXiG- 1 ) hrATsFotTYpoXNQnDRhwutfScQOXiG=2007148174; else hrATsFotTYpoXNQnDRhwutfScQOXiG=149823164;if (hrATsFotTYpoXNQnDRhwutfScQOXiG == hrATsFotTYpoXNQnDRhwutfScQOXiG- 1 ) hrATsFotTYpoXNQnDRhwutfScQOXiG=1170863304; else hrATsFotTYpoXNQnDRhwutfScQOXiG=1854757910;if (hrATsFotTYpoXNQnDRhwutfScQOXiG == hrATsFotTYpoXNQnDRhwutfScQOXiG- 0 ) hrATsFotTYpoXNQnDRhwutfScQOXiG=1889652916; else hrATsFotTYpoXNQnDRhwutfScQOXiG=253644361;float nCePSnWuMvbcjBshvnIRtTyLIaqDRN=1736358313.302022773748791046075987337079f;if (nCePSnWuMvbcjBshvnIRtTyLIaqDRN - nCePSnWuMvbcjBshvnIRtTyLIaqDRN> 0.00000001 ) nCePSnWuMvbcjBshvnIRtTyLIaqDRN=837810953.442036040877013564040051642589f; else nCePSnWuMvbcjBshvnIRtTyLIaqDRN=2103590812.581540687500621908487304949249f;if (nCePSnWuMvbcjBshvnIRtTyLIaqDRN - nCePSnWuMvbcjBshvnIRtTyLIaqDRN> 0.00000001 ) nCePSnWuMvbcjBshvnIRtTyLIaqDRN=1391664686.557139658309864144074521338760f; else nCePSnWuMvbcjBshvnIRtTyLIaqDRN=314680683.254816469236019955091527503691f;if (nCePSnWuMvbcjBshvnIRtTyLIaqDRN - nCePSnWuMvbcjBshvnIRtTyLIaqDRN> 0.00000001 ) nCePSnWuMvbcjBshvnIRtTyLIaqDRN=1849865091.109825516989685952348488974731f; else nCePSnWuMvbcjBshvnIRtTyLIaqDRN=1995029764.387814729455022205376245382296f;if (nCePSnWuMvbcjBshvnIRtTyLIaqDRN - nCePSnWuMvbcjBshvnIRtTyLIaqDRN> 0.00000001 ) nCePSnWuMvbcjBshvnIRtTyLIaqDRN=476648977.288560397133390456787057614254f; else nCePSnWuMvbcjBshvnIRtTyLIaqDRN=1085188947.353437756202834305603762934106f;if (nCePSnWuMvbcjBshvnIRtTyLIaqDRN - nCePSnWuMvbcjBshvnIRtTyLIaqDRN> 0.00000001 ) nCePSnWuMvbcjBshvnIRtTyLIaqDRN=826929041.714399107371058622432976269151f; else nCePSnWuMvbcjBshvnIRtTyLIaqDRN=1312734402.475169955630934372541287485823f;if (nCePSnWuMvbcjBshvnIRtTyLIaqDRN - nCePSnWuMvbcjBshvnIRtTyLIaqDRN> 0.00000001 ) nCePSnWuMvbcjBshvnIRtTyLIaqDRN=609014076.005601699988881214390872029879f; else nCePSnWuMvbcjBshvnIRtTyLIaqDRN=231048509.513373533378366451638254423478f;float oKDXlYZGpjmDzKHeEfhlqKYHPcwtaA=1490269117.056844531999797057211332808477f;if (oKDXlYZGpjmDzKHeEfhlqKYHPcwtaA - oKDXlYZGpjmDzKHeEfhlqKYHPcwtaA> 0.00000001 ) oKDXlYZGpjmDzKHeEfhlqKYHPcwtaA=1137688595.956590335846719132834712383405f; else oKDXlYZGpjmDzKHeEfhlqKYHPcwtaA=305204015.716514330873257662487674335532f;if (oKDXlYZGpjmDzKHeEfhlqKYHPcwtaA - oKDXlYZGpjmDzKHeEfhlqKYHPcwtaA> 0.00000001 ) oKDXlYZGpjmDzKHeEfhlqKYHPcwtaA=1889498700.703123031581207256572103915228f; else oKDXlYZGpjmDzKHeEfhlqKYHPcwtaA=2129092885.990918124463190648829006070121f;if (oKDXlYZGpjmDzKHeEfhlqKYHPcwtaA - oKDXlYZGpjmDzKHeEfhlqKYHPcwtaA> 0.00000001 ) oKDXlYZGpjmDzKHeEfhlqKYHPcwtaA=60931500.982479149913321012337851900505f; else oKDXlYZGpjmDzKHeEfhlqKYHPcwtaA=1808269868.910359283151901678506980424157f;if (oKDXlYZGpjmDzKHeEfhlqKYHPcwtaA - oKDXlYZGpjmDzKHeEfhlqKYHPcwtaA> 0.00000001 ) oKDXlYZGpjmDzKHeEfhlqKYHPcwtaA=1103213540.074411307394929796033210913194f; else oKDXlYZGpjmDzKHeEfhlqKYHPcwtaA=823226011.513629759038221306183922596551f;if (oKDXlYZGpjmDzKHeEfhlqKYHPcwtaA - oKDXlYZGpjmDzKHeEfhlqKYHPcwtaA> 0.00000001 ) oKDXlYZGpjmDzKHeEfhlqKYHPcwtaA=2142824996.932389131104711609971711279223f; else oKDXlYZGpjmDzKHeEfhlqKYHPcwtaA=1268759882.330921694338618921299869123121f;if (oKDXlYZGpjmDzKHeEfhlqKYHPcwtaA - oKDXlYZGpjmDzKHeEfhlqKYHPcwtaA> 0.00000001 ) oKDXlYZGpjmDzKHeEfhlqKYHPcwtaA=1485172914.001988499002575320336904467290f; else oKDXlYZGpjmDzKHeEfhlqKYHPcwtaA=1997296328.967433774154330830580380620852f;double OzuGUYITGeYhOnMfnVgRkjFzcrkpFQ=1426793686.929377017108117063521424043477;if (OzuGUYITGeYhOnMfnVgRkjFzcrkpFQ == OzuGUYITGeYhOnMfnVgRkjFzcrkpFQ ) OzuGUYITGeYhOnMfnVgRkjFzcrkpFQ=363365858.974661970168432972502780260533; else OzuGUYITGeYhOnMfnVgRkjFzcrkpFQ=1597886014.575123556610446310218107530451;if (OzuGUYITGeYhOnMfnVgRkjFzcrkpFQ == OzuGUYITGeYhOnMfnVgRkjFzcrkpFQ ) OzuGUYITGeYhOnMfnVgRkjFzcrkpFQ=9731351.144760442952799334496132658969; else OzuGUYITGeYhOnMfnVgRkjFzcrkpFQ=1484895573.411568689707847641930706602212;if (OzuGUYITGeYhOnMfnVgRkjFzcrkpFQ == OzuGUYITGeYhOnMfnVgRkjFzcrkpFQ ) OzuGUYITGeYhOnMfnVgRkjFzcrkpFQ=873474618.258365134276680930611704971596; else OzuGUYITGeYhOnMfnVgRkjFzcrkpFQ=1517331413.757855968252876523787261165540;if (OzuGUYITGeYhOnMfnVgRkjFzcrkpFQ == OzuGUYITGeYhOnMfnVgRkjFzcrkpFQ ) OzuGUYITGeYhOnMfnVgRkjFzcrkpFQ=1592297337.474863983508256903956992959770; else OzuGUYITGeYhOnMfnVgRkjFzcrkpFQ=1612228275.053989902364028002636379469291;if (OzuGUYITGeYhOnMfnVgRkjFzcrkpFQ == OzuGUYITGeYhOnMfnVgRkjFzcrkpFQ ) OzuGUYITGeYhOnMfnVgRkjFzcrkpFQ=1497158281.432466547700260297505815588453; else OzuGUYITGeYhOnMfnVgRkjFzcrkpFQ=1797503813.182103277257693712027270672808;if (OzuGUYITGeYhOnMfnVgRkjFzcrkpFQ == OzuGUYITGeYhOnMfnVgRkjFzcrkpFQ ) OzuGUYITGeYhOnMfnVgRkjFzcrkpFQ=2049818105.819473816713135923387753691353; else OzuGUYITGeYhOnMfnVgRkjFzcrkpFQ=928620674.258944823091632524585062227698;float edQbdYJuCFAyLwhEJFDnjlnoIvfnLo=1998237546.953342388861367168982419518730f;if (edQbdYJuCFAyLwhEJFDnjlnoIvfnLo - edQbdYJuCFAyLwhEJFDnjlnoIvfnLo> 0.00000001 ) edQbdYJuCFAyLwhEJFDnjlnoIvfnLo=1986795008.359982204939021366302510164583f; else edQbdYJuCFAyLwhEJFDnjlnoIvfnLo=399383799.476951248597515440925353885993f;if (edQbdYJuCFAyLwhEJFDnjlnoIvfnLo - edQbdYJuCFAyLwhEJFDnjlnoIvfnLo> 0.00000001 ) edQbdYJuCFAyLwhEJFDnjlnoIvfnLo=1263439413.982156327013882610572577923318f; else edQbdYJuCFAyLwhEJFDnjlnoIvfnLo=40287740.736672646013492357138552241633f;if (edQbdYJuCFAyLwhEJFDnjlnoIvfnLo - edQbdYJuCFAyLwhEJFDnjlnoIvfnLo> 0.00000001 ) edQbdYJuCFAyLwhEJFDnjlnoIvfnLo=1755343574.411048731118566918567629029443f; else edQbdYJuCFAyLwhEJFDnjlnoIvfnLo=96965522.101371356563713119644317689303f;if (edQbdYJuCFAyLwhEJFDnjlnoIvfnLo - edQbdYJuCFAyLwhEJFDnjlnoIvfnLo> 0.00000001 ) edQbdYJuCFAyLwhEJFDnjlnoIvfnLo=1675426813.940801729677311502601180089579f; else edQbdYJuCFAyLwhEJFDnjlnoIvfnLo=1177024732.305463491349017711729770296093f;if (edQbdYJuCFAyLwhEJFDnjlnoIvfnLo - edQbdYJuCFAyLwhEJFDnjlnoIvfnLo> 0.00000001 ) edQbdYJuCFAyLwhEJFDnjlnoIvfnLo=1000377272.225147305821146217347986164528f; else edQbdYJuCFAyLwhEJFDnjlnoIvfnLo=568438675.741912508152690873518975645008f;if (edQbdYJuCFAyLwhEJFDnjlnoIvfnLo - edQbdYJuCFAyLwhEJFDnjlnoIvfnLo> 0.00000001 ) edQbdYJuCFAyLwhEJFDnjlnoIvfnLo=319339334.488828134482030639649112088648f; else edQbdYJuCFAyLwhEJFDnjlnoIvfnLo=581105209.851078488375013704601647037785f;long auggNEAgKIYGUiXHCsQpVARjIIUCSF=1558967624;if (auggNEAgKIYGUiXHCsQpVARjIIUCSF == auggNEAgKIYGUiXHCsQpVARjIIUCSF- 0 ) auggNEAgKIYGUiXHCsQpVARjIIUCSF=645115201; else auggNEAgKIYGUiXHCsQpVARjIIUCSF=1329704037;if (auggNEAgKIYGUiXHCsQpVARjIIUCSF == auggNEAgKIYGUiXHCsQpVARjIIUCSF- 0 ) auggNEAgKIYGUiXHCsQpVARjIIUCSF=2015166298; else auggNEAgKIYGUiXHCsQpVARjIIUCSF=421402853;if (auggNEAgKIYGUiXHCsQpVARjIIUCSF == auggNEAgKIYGUiXHCsQpVARjIIUCSF- 1 ) auggNEAgKIYGUiXHCsQpVARjIIUCSF=1310378054; else auggNEAgKIYGUiXHCsQpVARjIIUCSF=530550976;if (auggNEAgKIYGUiXHCsQpVARjIIUCSF == auggNEAgKIYGUiXHCsQpVARjIIUCSF- 0 ) auggNEAgKIYGUiXHCsQpVARjIIUCSF=616794499; else auggNEAgKIYGUiXHCsQpVARjIIUCSF=1767840829;if (auggNEAgKIYGUiXHCsQpVARjIIUCSF == auggNEAgKIYGUiXHCsQpVARjIIUCSF- 0 ) auggNEAgKIYGUiXHCsQpVARjIIUCSF=860390325; else auggNEAgKIYGUiXHCsQpVARjIIUCSF=277270425;if (auggNEAgKIYGUiXHCsQpVARjIIUCSF == auggNEAgKIYGUiXHCsQpVARjIIUCSF- 1 ) auggNEAgKIYGUiXHCsQpVARjIIUCSF=631934604; else auggNEAgKIYGUiXHCsQpVARjIIUCSF=410911334;double BmXkNQPnKKdNbfqlMbZxieASCxqAbn=366725331.592818502542189313058100275895;if (BmXkNQPnKKdNbfqlMbZxieASCxqAbn == BmXkNQPnKKdNbfqlMbZxieASCxqAbn ) BmXkNQPnKKdNbfqlMbZxieASCxqAbn=990741938.411328381420140970637823749264; else BmXkNQPnKKdNbfqlMbZxieASCxqAbn=1634419015.472824186214274807141627869979;if (BmXkNQPnKKdNbfqlMbZxieASCxqAbn == BmXkNQPnKKdNbfqlMbZxieASCxqAbn ) BmXkNQPnKKdNbfqlMbZxieASCxqAbn=10501320.912705942689452466094323496516; else BmXkNQPnKKdNbfqlMbZxieASCxqAbn=365766569.532831267907556327540308579042;if (BmXkNQPnKKdNbfqlMbZxieASCxqAbn == BmXkNQPnKKdNbfqlMbZxieASCxqAbn ) BmXkNQPnKKdNbfqlMbZxieASCxqAbn=152005651.044716772231010887828917724547; else BmXkNQPnKKdNbfqlMbZxieASCxqAbn=627331461.223606494065642049989322414700;if (BmXkNQPnKKdNbfqlMbZxieASCxqAbn == BmXkNQPnKKdNbfqlMbZxieASCxqAbn ) BmXkNQPnKKdNbfqlMbZxieASCxqAbn=1818933759.334511952440386348689566900120; else BmXkNQPnKKdNbfqlMbZxieASCxqAbn=266400336.379583214120854005627723187871;if (BmXkNQPnKKdNbfqlMbZxieASCxqAbn == BmXkNQPnKKdNbfqlMbZxieASCxqAbn ) BmXkNQPnKKdNbfqlMbZxieASCxqAbn=2069942848.183327180424567982390650376651; else BmXkNQPnKKdNbfqlMbZxieASCxqAbn=980190266.644304763893531427731668263306;if (BmXkNQPnKKdNbfqlMbZxieASCxqAbn == BmXkNQPnKKdNbfqlMbZxieASCxqAbn ) BmXkNQPnKKdNbfqlMbZxieASCxqAbn=484518301.254088036323960713095092109714; else BmXkNQPnKKdNbfqlMbZxieASCxqAbn=305035943.246864966450151880856142237208;long PJKEqlCxfqaihVYrWJNKtscsGCUhTp=870321985;if (PJKEqlCxfqaihVYrWJNKtscsGCUhTp == PJKEqlCxfqaihVYrWJNKtscsGCUhTp- 1 ) PJKEqlCxfqaihVYrWJNKtscsGCUhTp=1477465234; else PJKEqlCxfqaihVYrWJNKtscsGCUhTp=490850481;if (PJKEqlCxfqaihVYrWJNKtscsGCUhTp == PJKEqlCxfqaihVYrWJNKtscsGCUhTp- 0 ) PJKEqlCxfqaihVYrWJNKtscsGCUhTp=283505473; else PJKEqlCxfqaihVYrWJNKtscsGCUhTp=1810751636;if (PJKEqlCxfqaihVYrWJNKtscsGCUhTp == PJKEqlCxfqaihVYrWJNKtscsGCUhTp- 0 ) PJKEqlCxfqaihVYrWJNKtscsGCUhTp=1908417014; else PJKEqlCxfqaihVYrWJNKtscsGCUhTp=2004098738;if (PJKEqlCxfqaihVYrWJNKtscsGCUhTp == PJKEqlCxfqaihVYrWJNKtscsGCUhTp- 1 ) PJKEqlCxfqaihVYrWJNKtscsGCUhTp=687859492; else PJKEqlCxfqaihVYrWJNKtscsGCUhTp=1456244032;if (PJKEqlCxfqaihVYrWJNKtscsGCUhTp == PJKEqlCxfqaihVYrWJNKtscsGCUhTp- 0 ) PJKEqlCxfqaihVYrWJNKtscsGCUhTp=1930072947; else PJKEqlCxfqaihVYrWJNKtscsGCUhTp=1643082231;if (PJKEqlCxfqaihVYrWJNKtscsGCUhTp == PJKEqlCxfqaihVYrWJNKtscsGCUhTp- 1 ) PJKEqlCxfqaihVYrWJNKtscsGCUhTp=1327381062; else PJKEqlCxfqaihVYrWJNKtscsGCUhTp=1478073149;double vRUcjexbekhWSMbDBYkTRSGcVFkOdU=825784406.745834921704155246695383933061;if (vRUcjexbekhWSMbDBYkTRSGcVFkOdU == vRUcjexbekhWSMbDBYkTRSGcVFkOdU ) vRUcjexbekhWSMbDBYkTRSGcVFkOdU=738184690.170026474556748096818170847200; else vRUcjexbekhWSMbDBYkTRSGcVFkOdU=2100097650.196163523098315975505825311128;if (vRUcjexbekhWSMbDBYkTRSGcVFkOdU == vRUcjexbekhWSMbDBYkTRSGcVFkOdU ) vRUcjexbekhWSMbDBYkTRSGcVFkOdU=1737846918.928456178558393186608381537186; else vRUcjexbekhWSMbDBYkTRSGcVFkOdU=1022921482.350517097990815864333553165793;if (vRUcjexbekhWSMbDBYkTRSGcVFkOdU == vRUcjexbekhWSMbDBYkTRSGcVFkOdU ) vRUcjexbekhWSMbDBYkTRSGcVFkOdU=616334976.009221888610081868688486390856; else vRUcjexbekhWSMbDBYkTRSGcVFkOdU=2101755657.340500269518682458331822562404;if (vRUcjexbekhWSMbDBYkTRSGcVFkOdU == vRUcjexbekhWSMbDBYkTRSGcVFkOdU ) vRUcjexbekhWSMbDBYkTRSGcVFkOdU=1406961973.204338069171928191404263662904; else vRUcjexbekhWSMbDBYkTRSGcVFkOdU=17783740.987531435540026090557110105188;if (vRUcjexbekhWSMbDBYkTRSGcVFkOdU == vRUcjexbekhWSMbDBYkTRSGcVFkOdU ) vRUcjexbekhWSMbDBYkTRSGcVFkOdU=1658253925.626530559777696425768345888754; else vRUcjexbekhWSMbDBYkTRSGcVFkOdU=1748765962.735803569764189152578053138176;if (vRUcjexbekhWSMbDBYkTRSGcVFkOdU == vRUcjexbekhWSMbDBYkTRSGcVFkOdU ) vRUcjexbekhWSMbDBYkTRSGcVFkOdU=498943652.391614290407610427783638679377; else vRUcjexbekhWSMbDBYkTRSGcVFkOdU=330187788.649358468697318244381250476304;float vKRpenPhMxjsHrfRHCVQbAxbVwtynI=1996681844.168451337808465948667740055470f;if (vKRpenPhMxjsHrfRHCVQbAxbVwtynI - vKRpenPhMxjsHrfRHCVQbAxbVwtynI> 0.00000001 ) vKRpenPhMxjsHrfRHCVQbAxbVwtynI=1322382647.645296558866360451145776642841f; else vKRpenPhMxjsHrfRHCVQbAxbVwtynI=1909096266.607727337949181149171534508965f;if (vKRpenPhMxjsHrfRHCVQbAxbVwtynI - vKRpenPhMxjsHrfRHCVQbAxbVwtynI> 0.00000001 ) vKRpenPhMxjsHrfRHCVQbAxbVwtynI=1080929747.581836074312431670651488027423f; else vKRpenPhMxjsHrfRHCVQbAxbVwtynI=1415084929.047357935528687911394842511482f;if (vKRpenPhMxjsHrfRHCVQbAxbVwtynI - vKRpenPhMxjsHrfRHCVQbAxbVwtynI> 0.00000001 ) vKRpenPhMxjsHrfRHCVQbAxbVwtynI=2038571040.075147628584514480111613921684f; else vKRpenPhMxjsHrfRHCVQbAxbVwtynI=166985996.665465354140492144824138257333f;if (vKRpenPhMxjsHrfRHCVQbAxbVwtynI - vKRpenPhMxjsHrfRHCVQbAxbVwtynI> 0.00000001 ) vKRpenPhMxjsHrfRHCVQbAxbVwtynI=605791964.704592925924617232198787164832f; else vKRpenPhMxjsHrfRHCVQbAxbVwtynI=1134126847.903155601133091062103525612843f;if (vKRpenPhMxjsHrfRHCVQbAxbVwtynI - vKRpenPhMxjsHrfRHCVQbAxbVwtynI> 0.00000001 ) vKRpenPhMxjsHrfRHCVQbAxbVwtynI=946559523.910312257373117670711283048965f; else vKRpenPhMxjsHrfRHCVQbAxbVwtynI=701740935.396822479991684051814313373747f;if (vKRpenPhMxjsHrfRHCVQbAxbVwtynI - vKRpenPhMxjsHrfRHCVQbAxbVwtynI> 0.00000001 ) vKRpenPhMxjsHrfRHCVQbAxbVwtynI=583177269.430318839531283071010816118366f; else vKRpenPhMxjsHrfRHCVQbAxbVwtynI=656407067.974266238632348423141384873421f;long XnTebjzNhrQORrpZmmluZKJXgwAVuR=805427369;if (XnTebjzNhrQORrpZmmluZKJXgwAVuR == XnTebjzNhrQORrpZmmluZKJXgwAVuR- 1 ) XnTebjzNhrQORrpZmmluZKJXgwAVuR=1709100944; else XnTebjzNhrQORrpZmmluZKJXgwAVuR=1806217542;if (XnTebjzNhrQORrpZmmluZKJXgwAVuR == XnTebjzNhrQORrpZmmluZKJXgwAVuR- 0 ) XnTebjzNhrQORrpZmmluZKJXgwAVuR=1714334900; else XnTebjzNhrQORrpZmmluZKJXgwAVuR=1245378076;if (XnTebjzNhrQORrpZmmluZKJXgwAVuR == XnTebjzNhrQORrpZmmluZKJXgwAVuR- 1 ) XnTebjzNhrQORrpZmmluZKJXgwAVuR=1420828857; else XnTebjzNhrQORrpZmmluZKJXgwAVuR=79449943;if (XnTebjzNhrQORrpZmmluZKJXgwAVuR == XnTebjzNhrQORrpZmmluZKJXgwAVuR- 0 ) XnTebjzNhrQORrpZmmluZKJXgwAVuR=253987009; else XnTebjzNhrQORrpZmmluZKJXgwAVuR=1523193224;if (XnTebjzNhrQORrpZmmluZKJXgwAVuR == XnTebjzNhrQORrpZmmluZKJXgwAVuR- 0 ) XnTebjzNhrQORrpZmmluZKJXgwAVuR=1231372155; else XnTebjzNhrQORrpZmmluZKJXgwAVuR=265721733;if (XnTebjzNhrQORrpZmmluZKJXgwAVuR == XnTebjzNhrQORrpZmmluZKJXgwAVuR- 1 ) XnTebjzNhrQORrpZmmluZKJXgwAVuR=152497711; else XnTebjzNhrQORrpZmmluZKJXgwAVuR=159651052;float UcWUqMjZSqlGQWiaEwSYthCOKmOsEV=1618314391.997305686669071868448608027353f;if (UcWUqMjZSqlGQWiaEwSYthCOKmOsEV - UcWUqMjZSqlGQWiaEwSYthCOKmOsEV> 0.00000001 ) UcWUqMjZSqlGQWiaEwSYthCOKmOsEV=1073233481.095987617352334084949649040038f; else UcWUqMjZSqlGQWiaEwSYthCOKmOsEV=1478182865.843468410536219748505452456794f;if (UcWUqMjZSqlGQWiaEwSYthCOKmOsEV - UcWUqMjZSqlGQWiaEwSYthCOKmOsEV> 0.00000001 ) UcWUqMjZSqlGQWiaEwSYthCOKmOsEV=155690502.778952997677542443041254459211f; else UcWUqMjZSqlGQWiaEwSYthCOKmOsEV=1850901946.282696946785293801001476452451f;if (UcWUqMjZSqlGQWiaEwSYthCOKmOsEV - UcWUqMjZSqlGQWiaEwSYthCOKmOsEV> 0.00000001 ) UcWUqMjZSqlGQWiaEwSYthCOKmOsEV=183585732.489640606661746617175921192624f; else UcWUqMjZSqlGQWiaEwSYthCOKmOsEV=3488040.459640665431545174184639657966f;if (UcWUqMjZSqlGQWiaEwSYthCOKmOsEV - UcWUqMjZSqlGQWiaEwSYthCOKmOsEV> 0.00000001 ) UcWUqMjZSqlGQWiaEwSYthCOKmOsEV=934697013.808331320716275548702864092109f; else UcWUqMjZSqlGQWiaEwSYthCOKmOsEV=571198694.481063332486448956412936414434f;if (UcWUqMjZSqlGQWiaEwSYthCOKmOsEV - UcWUqMjZSqlGQWiaEwSYthCOKmOsEV> 0.00000001 ) UcWUqMjZSqlGQWiaEwSYthCOKmOsEV=1494305686.922243522231733753724779856676f; else UcWUqMjZSqlGQWiaEwSYthCOKmOsEV=603720071.090857431543539130476282136449f;if (UcWUqMjZSqlGQWiaEwSYthCOKmOsEV - UcWUqMjZSqlGQWiaEwSYthCOKmOsEV> 0.00000001 ) UcWUqMjZSqlGQWiaEwSYthCOKmOsEV=76361959.805624298733228124072620182983f; else UcWUqMjZSqlGQWiaEwSYthCOKmOsEV=2062077688.119275901605385406502248838655f;int PEcbIUzZRPKRWkmQYbkpdTrGxlFXVf=1317393315;if (PEcbIUzZRPKRWkmQYbkpdTrGxlFXVf == PEcbIUzZRPKRWkmQYbkpdTrGxlFXVf- 0 ) PEcbIUzZRPKRWkmQYbkpdTrGxlFXVf=947396544; else PEcbIUzZRPKRWkmQYbkpdTrGxlFXVf=1840999930;if (PEcbIUzZRPKRWkmQYbkpdTrGxlFXVf == PEcbIUzZRPKRWkmQYbkpdTrGxlFXVf- 1 ) PEcbIUzZRPKRWkmQYbkpdTrGxlFXVf=768570491; else PEcbIUzZRPKRWkmQYbkpdTrGxlFXVf=387298780;if (PEcbIUzZRPKRWkmQYbkpdTrGxlFXVf == PEcbIUzZRPKRWkmQYbkpdTrGxlFXVf- 0 ) PEcbIUzZRPKRWkmQYbkpdTrGxlFXVf=744907303; else PEcbIUzZRPKRWkmQYbkpdTrGxlFXVf=1471954321;if (PEcbIUzZRPKRWkmQYbkpdTrGxlFXVf == PEcbIUzZRPKRWkmQYbkpdTrGxlFXVf- 1 ) PEcbIUzZRPKRWkmQYbkpdTrGxlFXVf=575852987; else PEcbIUzZRPKRWkmQYbkpdTrGxlFXVf=1242932704;if (PEcbIUzZRPKRWkmQYbkpdTrGxlFXVf == PEcbIUzZRPKRWkmQYbkpdTrGxlFXVf- 1 ) PEcbIUzZRPKRWkmQYbkpdTrGxlFXVf=1655109257; else PEcbIUzZRPKRWkmQYbkpdTrGxlFXVf=1705220170;if (PEcbIUzZRPKRWkmQYbkpdTrGxlFXVf == PEcbIUzZRPKRWkmQYbkpdTrGxlFXVf- 1 ) PEcbIUzZRPKRWkmQYbkpdTrGxlFXVf=183712774; else PEcbIUzZRPKRWkmQYbkpdTrGxlFXVf=1772849307;long eutZLQzmofGoevYCIGjgmpGsNtUGVx=588887819;if (eutZLQzmofGoevYCIGjgmpGsNtUGVx == eutZLQzmofGoevYCIGjgmpGsNtUGVx- 0 ) eutZLQzmofGoevYCIGjgmpGsNtUGVx=1439013225; else eutZLQzmofGoevYCIGjgmpGsNtUGVx=1112346525;if (eutZLQzmofGoevYCIGjgmpGsNtUGVx == eutZLQzmofGoevYCIGjgmpGsNtUGVx- 1 ) eutZLQzmofGoevYCIGjgmpGsNtUGVx=1730657233; else eutZLQzmofGoevYCIGjgmpGsNtUGVx=2116734828;if (eutZLQzmofGoevYCIGjgmpGsNtUGVx == eutZLQzmofGoevYCIGjgmpGsNtUGVx- 0 ) eutZLQzmofGoevYCIGjgmpGsNtUGVx=185170108; else eutZLQzmofGoevYCIGjgmpGsNtUGVx=1097951119;if (eutZLQzmofGoevYCIGjgmpGsNtUGVx == eutZLQzmofGoevYCIGjgmpGsNtUGVx- 1 ) eutZLQzmofGoevYCIGjgmpGsNtUGVx=621841637; else eutZLQzmofGoevYCIGjgmpGsNtUGVx=1705612904;if (eutZLQzmofGoevYCIGjgmpGsNtUGVx == eutZLQzmofGoevYCIGjgmpGsNtUGVx- 0 ) eutZLQzmofGoevYCIGjgmpGsNtUGVx=871520529; else eutZLQzmofGoevYCIGjgmpGsNtUGVx=1143136206;if (eutZLQzmofGoevYCIGjgmpGsNtUGVx == eutZLQzmofGoevYCIGjgmpGsNtUGVx- 0 ) eutZLQzmofGoevYCIGjgmpGsNtUGVx=1898556037; else eutZLQzmofGoevYCIGjgmpGsNtUGVx=817108786;float rqIIuYyjgbNCAEFUMzbMLJzqcgRCgf=480817249.892461188355346550525258316980f;if (rqIIuYyjgbNCAEFUMzbMLJzqcgRCgf - rqIIuYyjgbNCAEFUMzbMLJzqcgRCgf> 0.00000001 ) rqIIuYyjgbNCAEFUMzbMLJzqcgRCgf=1695372039.979502097987504067014232475416f; else rqIIuYyjgbNCAEFUMzbMLJzqcgRCgf=982588043.630848684566465875545097784824f;if (rqIIuYyjgbNCAEFUMzbMLJzqcgRCgf - rqIIuYyjgbNCAEFUMzbMLJzqcgRCgf> 0.00000001 ) rqIIuYyjgbNCAEFUMzbMLJzqcgRCgf=1359738155.768136786649366998860108274427f; else rqIIuYyjgbNCAEFUMzbMLJzqcgRCgf=1811638803.596180833020990784199707477107f;if (rqIIuYyjgbNCAEFUMzbMLJzqcgRCgf - rqIIuYyjgbNCAEFUMzbMLJzqcgRCgf> 0.00000001 ) rqIIuYyjgbNCAEFUMzbMLJzqcgRCgf=1736584860.247016329626595121960134899071f; else rqIIuYyjgbNCAEFUMzbMLJzqcgRCgf=1022912577.383788409546558422336735821323f;if (rqIIuYyjgbNCAEFUMzbMLJzqcgRCgf - rqIIuYyjgbNCAEFUMzbMLJzqcgRCgf> 0.00000001 ) rqIIuYyjgbNCAEFUMzbMLJzqcgRCgf=873407088.735944382783342468757885575996f; else rqIIuYyjgbNCAEFUMzbMLJzqcgRCgf=1748762807.818782162414176459447378606148f;if (rqIIuYyjgbNCAEFUMzbMLJzqcgRCgf - rqIIuYyjgbNCAEFUMzbMLJzqcgRCgf> 0.00000001 ) rqIIuYyjgbNCAEFUMzbMLJzqcgRCgf=902329878.050697576041745472923766064119f; else rqIIuYyjgbNCAEFUMzbMLJzqcgRCgf=1060241445.602546558998441674333533530474f;if (rqIIuYyjgbNCAEFUMzbMLJzqcgRCgf - rqIIuYyjgbNCAEFUMzbMLJzqcgRCgf> 0.00000001 ) rqIIuYyjgbNCAEFUMzbMLJzqcgRCgf=195792689.446754920993404542640089728002f; else rqIIuYyjgbNCAEFUMzbMLJzqcgRCgf=625849209.729565370785554123010893547703f;long vJpIlStWqJjMsiWYRaiBXBuHuMeCHt=1048164774;if (vJpIlStWqJjMsiWYRaiBXBuHuMeCHt == vJpIlStWqJjMsiWYRaiBXBuHuMeCHt- 1 ) vJpIlStWqJjMsiWYRaiBXBuHuMeCHt=974748310; else vJpIlStWqJjMsiWYRaiBXBuHuMeCHt=1634957787;if (vJpIlStWqJjMsiWYRaiBXBuHuMeCHt == vJpIlStWqJjMsiWYRaiBXBuHuMeCHt- 0 ) vJpIlStWqJjMsiWYRaiBXBuHuMeCHt=623064463; else vJpIlStWqJjMsiWYRaiBXBuHuMeCHt=406231863;if (vJpIlStWqJjMsiWYRaiBXBuHuMeCHt == vJpIlStWqJjMsiWYRaiBXBuHuMeCHt- 1 ) vJpIlStWqJjMsiWYRaiBXBuHuMeCHt=1167381320; else vJpIlStWqJjMsiWYRaiBXBuHuMeCHt=748582160;if (vJpIlStWqJjMsiWYRaiBXBuHuMeCHt == vJpIlStWqJjMsiWYRaiBXBuHuMeCHt- 1 ) vJpIlStWqJjMsiWYRaiBXBuHuMeCHt=1132223207; else vJpIlStWqJjMsiWYRaiBXBuHuMeCHt=1996929072;if (vJpIlStWqJjMsiWYRaiBXBuHuMeCHt == vJpIlStWqJjMsiWYRaiBXBuHuMeCHt- 1 ) vJpIlStWqJjMsiWYRaiBXBuHuMeCHt=1219158293; else vJpIlStWqJjMsiWYRaiBXBuHuMeCHt=1210441234;if (vJpIlStWqJjMsiWYRaiBXBuHuMeCHt == vJpIlStWqJjMsiWYRaiBXBuHuMeCHt- 1 ) vJpIlStWqJjMsiWYRaiBXBuHuMeCHt=1663261898; else vJpIlStWqJjMsiWYRaiBXBuHuMeCHt=1749254744;double HChpWChrLoGZqRtMsBQEKEGdMJFGpU=1315716289.762556182987580555131654066896;if (HChpWChrLoGZqRtMsBQEKEGdMJFGpU == HChpWChrLoGZqRtMsBQEKEGdMJFGpU ) HChpWChrLoGZqRtMsBQEKEGdMJFGpU=1700605334.745479138598614453359774111850; else HChpWChrLoGZqRtMsBQEKEGdMJFGpU=1850379172.040725311143875405171464707147;if (HChpWChrLoGZqRtMsBQEKEGdMJFGpU == HChpWChrLoGZqRtMsBQEKEGdMJFGpU ) HChpWChrLoGZqRtMsBQEKEGdMJFGpU=77477268.849855909924321701081689979602; else HChpWChrLoGZqRtMsBQEKEGdMJFGpU=1179383889.928531402151726814411627636109;if (HChpWChrLoGZqRtMsBQEKEGdMJFGpU == HChpWChrLoGZqRtMsBQEKEGdMJFGpU ) HChpWChrLoGZqRtMsBQEKEGdMJFGpU=1233886404.741710689131003107326529230201; else HChpWChrLoGZqRtMsBQEKEGdMJFGpU=2112300252.541342101624639163454088432258;if (HChpWChrLoGZqRtMsBQEKEGdMJFGpU == HChpWChrLoGZqRtMsBQEKEGdMJFGpU ) HChpWChrLoGZqRtMsBQEKEGdMJFGpU=999786606.059860086110681390127885857402; else HChpWChrLoGZqRtMsBQEKEGdMJFGpU=416100648.096467597019845450724324137512;if (HChpWChrLoGZqRtMsBQEKEGdMJFGpU == HChpWChrLoGZqRtMsBQEKEGdMJFGpU ) HChpWChrLoGZqRtMsBQEKEGdMJFGpU=238939437.478512533581312915041779888233; else HChpWChrLoGZqRtMsBQEKEGdMJFGpU=2129378073.018674233403834411233357171074;if (HChpWChrLoGZqRtMsBQEKEGdMJFGpU == HChpWChrLoGZqRtMsBQEKEGdMJFGpU ) HChpWChrLoGZqRtMsBQEKEGdMJFGpU=1214404950.530058375978375995557810186260; else HChpWChrLoGZqRtMsBQEKEGdMJFGpU=1272412310.427998399615646619238652783007;float EEaYcDSnmgEfvVOGiVYkkAQJoiXJKZ=1709672823.006682402873288490272800212665f;if (EEaYcDSnmgEfvVOGiVYkkAQJoiXJKZ - EEaYcDSnmgEfvVOGiVYkkAQJoiXJKZ> 0.00000001 ) EEaYcDSnmgEfvVOGiVYkkAQJoiXJKZ=586425797.471702735880363039876399321757f; else EEaYcDSnmgEfvVOGiVYkkAQJoiXJKZ=850391263.194751259786650265829389019358f;if (EEaYcDSnmgEfvVOGiVYkkAQJoiXJKZ - EEaYcDSnmgEfvVOGiVYkkAQJoiXJKZ> 0.00000001 ) EEaYcDSnmgEfvVOGiVYkkAQJoiXJKZ=1989844123.118880771388120085017794881674f; else EEaYcDSnmgEfvVOGiVYkkAQJoiXJKZ=515613844.200167887069464236904535242411f;if (EEaYcDSnmgEfvVOGiVYkkAQJoiXJKZ - EEaYcDSnmgEfvVOGiVYkkAQJoiXJKZ> 0.00000001 ) EEaYcDSnmgEfvVOGiVYkkAQJoiXJKZ=1863812152.910719817320806844176677188693f; else EEaYcDSnmgEfvVOGiVYkkAQJoiXJKZ=1745866620.657725075642069473941945545374f;if (EEaYcDSnmgEfvVOGiVYkkAQJoiXJKZ - EEaYcDSnmgEfvVOGiVYkkAQJoiXJKZ> 0.00000001 ) EEaYcDSnmgEfvVOGiVYkkAQJoiXJKZ=1460919576.132488329297963461971586281329f; else EEaYcDSnmgEfvVOGiVYkkAQJoiXJKZ=946789879.071252169116576409218540375731f;if (EEaYcDSnmgEfvVOGiVYkkAQJoiXJKZ - EEaYcDSnmgEfvVOGiVYkkAQJoiXJKZ> 0.00000001 ) EEaYcDSnmgEfvVOGiVYkkAQJoiXJKZ=1997848483.810022772240287511098312163298f; else EEaYcDSnmgEfvVOGiVYkkAQJoiXJKZ=1510292730.754941755483818837487574350997f;if (EEaYcDSnmgEfvVOGiVYkkAQJoiXJKZ - EEaYcDSnmgEfvVOGiVYkkAQJoiXJKZ> 0.00000001 ) EEaYcDSnmgEfvVOGiVYkkAQJoiXJKZ=592480510.069469644696671561433782229060f; else EEaYcDSnmgEfvVOGiVYkkAQJoiXJKZ=526143626.296853546920380190380956538724f;double fmZXCwnMsBGdWFRVzKEPskrwLQASHa=1143585677.432781735500817777246174679815;if (fmZXCwnMsBGdWFRVzKEPskrwLQASHa == fmZXCwnMsBGdWFRVzKEPskrwLQASHa ) fmZXCwnMsBGdWFRVzKEPskrwLQASHa=1967880372.045747712423346017163721746812; else fmZXCwnMsBGdWFRVzKEPskrwLQASHa=1027560047.292057528006693483977386243500;if (fmZXCwnMsBGdWFRVzKEPskrwLQASHa == fmZXCwnMsBGdWFRVzKEPskrwLQASHa ) fmZXCwnMsBGdWFRVzKEPskrwLQASHa=1072727522.100124139059916723836305501792; else fmZXCwnMsBGdWFRVzKEPskrwLQASHa=116879741.412800658935592847052077580662;if (fmZXCwnMsBGdWFRVzKEPskrwLQASHa == fmZXCwnMsBGdWFRVzKEPskrwLQASHa ) fmZXCwnMsBGdWFRVzKEPskrwLQASHa=1231854085.355765812628690191386474054542; else fmZXCwnMsBGdWFRVzKEPskrwLQASHa=1524810167.119679184713644549840039699282;if (fmZXCwnMsBGdWFRVzKEPskrwLQASHa == fmZXCwnMsBGdWFRVzKEPskrwLQASHa ) fmZXCwnMsBGdWFRVzKEPskrwLQASHa=1961762017.238061913212868493061581318108; else fmZXCwnMsBGdWFRVzKEPskrwLQASHa=1009931705.001268668473663837131623275785;if (fmZXCwnMsBGdWFRVzKEPskrwLQASHa == fmZXCwnMsBGdWFRVzKEPskrwLQASHa ) fmZXCwnMsBGdWFRVzKEPskrwLQASHa=635171181.482522261183579802859117858919; else fmZXCwnMsBGdWFRVzKEPskrwLQASHa=206963815.399409224277716816448393638588;if (fmZXCwnMsBGdWFRVzKEPskrwLQASHa == fmZXCwnMsBGdWFRVzKEPskrwLQASHa ) fmZXCwnMsBGdWFRVzKEPskrwLQASHa=167292426.435289527169879350214258200688; else fmZXCwnMsBGdWFRVzKEPskrwLQASHa=1185486894.184784253739048053548937439818; }
 fmZXCwnMsBGdWFRVzKEPskrwLQASHay::fmZXCwnMsBGdWFRVzKEPskrwLQASHay()
 { this->qctpkiGtROUM("MJreJnQsIHlWmrPCZFsmZDBKuvcMmwqctpkiGtROUMj", true, 1681660857, 2007294544, 1094770642); }
#pragma optimize("", off)
 // <delete/>


// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class FQTbOzzvdnlxZSJIMENkslkNzyjdfPy
 { 
public: bool HKbIWyvSCPMilcwugWvyZWkBUgGcQP; double HKbIWyvSCPMilcwugWvyZWkBUgGcQPFQTbOzzvdnlxZSJIMENkslkNzyjdfP; FQTbOzzvdnlxZSJIMENkslkNzyjdfPy(); void EuvwSOqeuiel(string HKbIWyvSCPMilcwugWvyZWkBUgGcQPEuvwSOqeuiel, bool WPOHdmxJmKQmKUfDylCEDHjRFlaaop, int rTBntwTezKwWjlhnLQVQhVaMMOfxBr, float MXVwTwkXbjjTlsHgYyjRaJEXIWspbe, long RVdHxZBmSkDopmgNjiEfqhOjRCTxow);
 protected: bool HKbIWyvSCPMilcwugWvyZWkBUgGcQPo; double HKbIWyvSCPMilcwugWvyZWkBUgGcQPFQTbOzzvdnlxZSJIMENkslkNzyjdfPf; void EuvwSOqeuielu(string HKbIWyvSCPMilcwugWvyZWkBUgGcQPEuvwSOqeuielg, bool WPOHdmxJmKQmKUfDylCEDHjRFlaaope, int rTBntwTezKwWjlhnLQVQhVaMMOfxBrr, float MXVwTwkXbjjTlsHgYyjRaJEXIWspbew, long RVdHxZBmSkDopmgNjiEfqhOjRCTxown);
 private: bool HKbIWyvSCPMilcwugWvyZWkBUgGcQPWPOHdmxJmKQmKUfDylCEDHjRFlaaop; double HKbIWyvSCPMilcwugWvyZWkBUgGcQPMXVwTwkXbjjTlsHgYyjRaJEXIWspbeFQTbOzzvdnlxZSJIMENkslkNzyjdfP;
 void EuvwSOqeuielv(string WPOHdmxJmKQmKUfDylCEDHjRFlaaopEuvwSOqeuiel, bool WPOHdmxJmKQmKUfDylCEDHjRFlaaoprTBntwTezKwWjlhnLQVQhVaMMOfxBr, int rTBntwTezKwWjlhnLQVQhVaMMOfxBrHKbIWyvSCPMilcwugWvyZWkBUgGcQP, float MXVwTwkXbjjTlsHgYyjRaJEXIWspbeRVdHxZBmSkDopmgNjiEfqhOjRCTxow, long RVdHxZBmSkDopmgNjiEfqhOjRCTxowWPOHdmxJmKQmKUfDylCEDHjRFlaaop); };
 void FQTbOzzvdnlxZSJIMENkslkNzyjdfPy::EuvwSOqeuiel(string HKbIWyvSCPMilcwugWvyZWkBUgGcQPEuvwSOqeuiel, bool WPOHdmxJmKQmKUfDylCEDHjRFlaaop, int rTBntwTezKwWjlhnLQVQhVaMMOfxBr, float MXVwTwkXbjjTlsHgYyjRaJEXIWspbe, long RVdHxZBmSkDopmgNjiEfqhOjRCTxow)
 { long YtHqARjeNSUwOEHLfezrIRLeQjDdQw=1785116751;if (YtHqARjeNSUwOEHLfezrIRLeQjDdQw == YtHqARjeNSUwOEHLfezrIRLeQjDdQw- 0 ) YtHqARjeNSUwOEHLfezrIRLeQjDdQw=146478291; else YtHqARjeNSUwOEHLfezrIRLeQjDdQw=590299137;if (YtHqARjeNSUwOEHLfezrIRLeQjDdQw == YtHqARjeNSUwOEHLfezrIRLeQjDdQw- 0 ) YtHqARjeNSUwOEHLfezrIRLeQjDdQw=1491459641; else YtHqARjeNSUwOEHLfezrIRLeQjDdQw=1762829004;if (YtHqARjeNSUwOEHLfezrIRLeQjDdQw == YtHqARjeNSUwOEHLfezrIRLeQjDdQw- 1 ) YtHqARjeNSUwOEHLfezrIRLeQjDdQw=1265835836; else YtHqARjeNSUwOEHLfezrIRLeQjDdQw=575681420;if (YtHqARjeNSUwOEHLfezrIRLeQjDdQw == YtHqARjeNSUwOEHLfezrIRLeQjDdQw- 1 ) YtHqARjeNSUwOEHLfezrIRLeQjDdQw=722526893; else YtHqARjeNSUwOEHLfezrIRLeQjDdQw=2084015148;if (YtHqARjeNSUwOEHLfezrIRLeQjDdQw == YtHqARjeNSUwOEHLfezrIRLeQjDdQw- 0 ) YtHqARjeNSUwOEHLfezrIRLeQjDdQw=1218953866; else YtHqARjeNSUwOEHLfezrIRLeQjDdQw=538476044;if (YtHqARjeNSUwOEHLfezrIRLeQjDdQw == YtHqARjeNSUwOEHLfezrIRLeQjDdQw- 0 ) YtHqARjeNSUwOEHLfezrIRLeQjDdQw=1833073634; else YtHqARjeNSUwOEHLfezrIRLeQjDdQw=1378820702;long bhcxPgMEdmZEiaRxwEYYnnlVKkGILD=1283604447;if (bhcxPgMEdmZEiaRxwEYYnnlVKkGILD == bhcxPgMEdmZEiaRxwEYYnnlVKkGILD- 1 ) bhcxPgMEdmZEiaRxwEYYnnlVKkGILD=684240451; else bhcxPgMEdmZEiaRxwEYYnnlVKkGILD=46220752;if (bhcxPgMEdmZEiaRxwEYYnnlVKkGILD == bhcxPgMEdmZEiaRxwEYYnnlVKkGILD- 1 ) bhcxPgMEdmZEiaRxwEYYnnlVKkGILD=1743612062; else bhcxPgMEdmZEiaRxwEYYnnlVKkGILD=1963499080;if (bhcxPgMEdmZEiaRxwEYYnnlVKkGILD == bhcxPgMEdmZEiaRxwEYYnnlVKkGILD- 1 ) bhcxPgMEdmZEiaRxwEYYnnlVKkGILD=1833958881; else bhcxPgMEdmZEiaRxwEYYnnlVKkGILD=45625719;if (bhcxPgMEdmZEiaRxwEYYnnlVKkGILD == bhcxPgMEdmZEiaRxwEYYnnlVKkGILD- 0 ) bhcxPgMEdmZEiaRxwEYYnnlVKkGILD=1192384936; else bhcxPgMEdmZEiaRxwEYYnnlVKkGILD=1012221359;if (bhcxPgMEdmZEiaRxwEYYnnlVKkGILD == bhcxPgMEdmZEiaRxwEYYnnlVKkGILD- 0 ) bhcxPgMEdmZEiaRxwEYYnnlVKkGILD=1569405723; else bhcxPgMEdmZEiaRxwEYYnnlVKkGILD=1940911938;if (bhcxPgMEdmZEiaRxwEYYnnlVKkGILD == bhcxPgMEdmZEiaRxwEYYnnlVKkGILD- 0 ) bhcxPgMEdmZEiaRxwEYYnnlVKkGILD=814292077; else bhcxPgMEdmZEiaRxwEYYnnlVKkGILD=299886118;long jLbwvVnMhPcsqNPafqdXpmdOAZxxXd=2088376486;if (jLbwvVnMhPcsqNPafqdXpmdOAZxxXd == jLbwvVnMhPcsqNPafqdXpmdOAZxxXd- 1 ) jLbwvVnMhPcsqNPafqdXpmdOAZxxXd=1122645479; else jLbwvVnMhPcsqNPafqdXpmdOAZxxXd=1828133359;if (jLbwvVnMhPcsqNPafqdXpmdOAZxxXd == jLbwvVnMhPcsqNPafqdXpmdOAZxxXd- 1 ) jLbwvVnMhPcsqNPafqdXpmdOAZxxXd=316163788; else jLbwvVnMhPcsqNPafqdXpmdOAZxxXd=1988331603;if (jLbwvVnMhPcsqNPafqdXpmdOAZxxXd == jLbwvVnMhPcsqNPafqdXpmdOAZxxXd- 1 ) jLbwvVnMhPcsqNPafqdXpmdOAZxxXd=604632062; else jLbwvVnMhPcsqNPafqdXpmdOAZxxXd=1341090788;if (jLbwvVnMhPcsqNPafqdXpmdOAZxxXd == jLbwvVnMhPcsqNPafqdXpmdOAZxxXd- 1 ) jLbwvVnMhPcsqNPafqdXpmdOAZxxXd=684416656; else jLbwvVnMhPcsqNPafqdXpmdOAZxxXd=1193000234;if (jLbwvVnMhPcsqNPafqdXpmdOAZxxXd == jLbwvVnMhPcsqNPafqdXpmdOAZxxXd- 0 ) jLbwvVnMhPcsqNPafqdXpmdOAZxxXd=1271217938; else jLbwvVnMhPcsqNPafqdXpmdOAZxxXd=1961276967;if (jLbwvVnMhPcsqNPafqdXpmdOAZxxXd == jLbwvVnMhPcsqNPafqdXpmdOAZxxXd- 1 ) jLbwvVnMhPcsqNPafqdXpmdOAZxxXd=1645988382; else jLbwvVnMhPcsqNPafqdXpmdOAZxxXd=1334752536;float krEoBKVPfNqSdxENPUBhGZlzjfvXdX=1475195838.193389371279210496441894741537f;if (krEoBKVPfNqSdxENPUBhGZlzjfvXdX - krEoBKVPfNqSdxENPUBhGZlzjfvXdX> 0.00000001 ) krEoBKVPfNqSdxENPUBhGZlzjfvXdX=1895179024.669039378618589822923451940975f; else krEoBKVPfNqSdxENPUBhGZlzjfvXdX=699052075.885572028213893432593072532920f;if (krEoBKVPfNqSdxENPUBhGZlzjfvXdX - krEoBKVPfNqSdxENPUBhGZlzjfvXdX> 0.00000001 ) krEoBKVPfNqSdxENPUBhGZlzjfvXdX=1727489870.940394412890471167861958282304f; else krEoBKVPfNqSdxENPUBhGZlzjfvXdX=1235592838.120948350003171370147517910311f;if (krEoBKVPfNqSdxENPUBhGZlzjfvXdX - krEoBKVPfNqSdxENPUBhGZlzjfvXdX> 0.00000001 ) krEoBKVPfNqSdxENPUBhGZlzjfvXdX=327948514.211887164491796988702237861313f; else krEoBKVPfNqSdxENPUBhGZlzjfvXdX=1774695795.805089230431786692203372019503f;if (krEoBKVPfNqSdxENPUBhGZlzjfvXdX - krEoBKVPfNqSdxENPUBhGZlzjfvXdX> 0.00000001 ) krEoBKVPfNqSdxENPUBhGZlzjfvXdX=2098641517.686169781785837166227021054318f; else krEoBKVPfNqSdxENPUBhGZlzjfvXdX=100852572.184785662702253954595439676522f;if (krEoBKVPfNqSdxENPUBhGZlzjfvXdX - krEoBKVPfNqSdxENPUBhGZlzjfvXdX> 0.00000001 ) krEoBKVPfNqSdxENPUBhGZlzjfvXdX=1545397848.658370225260463688920182385612f; else krEoBKVPfNqSdxENPUBhGZlzjfvXdX=479238282.015015598374944508619970268600f;if (krEoBKVPfNqSdxENPUBhGZlzjfvXdX - krEoBKVPfNqSdxENPUBhGZlzjfvXdX> 0.00000001 ) krEoBKVPfNqSdxENPUBhGZlzjfvXdX=193423281.671435645920066625998801315596f; else krEoBKVPfNqSdxENPUBhGZlzjfvXdX=1472407545.809980461906380001564472288023f;int HcxDKADtbMiTAuMZMFMGUAOHXOlnxO=1211976191;if (HcxDKADtbMiTAuMZMFMGUAOHXOlnxO == HcxDKADtbMiTAuMZMFMGUAOHXOlnxO- 0 ) HcxDKADtbMiTAuMZMFMGUAOHXOlnxO=1758535696; else HcxDKADtbMiTAuMZMFMGUAOHXOlnxO=1892180795;if (HcxDKADtbMiTAuMZMFMGUAOHXOlnxO == HcxDKADtbMiTAuMZMFMGUAOHXOlnxO- 1 ) HcxDKADtbMiTAuMZMFMGUAOHXOlnxO=256175223; else HcxDKADtbMiTAuMZMFMGUAOHXOlnxO=139914905;if (HcxDKADtbMiTAuMZMFMGUAOHXOlnxO == HcxDKADtbMiTAuMZMFMGUAOHXOlnxO- 0 ) HcxDKADtbMiTAuMZMFMGUAOHXOlnxO=830876808; else HcxDKADtbMiTAuMZMFMGUAOHXOlnxO=1430082308;if (HcxDKADtbMiTAuMZMFMGUAOHXOlnxO == HcxDKADtbMiTAuMZMFMGUAOHXOlnxO- 1 ) HcxDKADtbMiTAuMZMFMGUAOHXOlnxO=678972385; else HcxDKADtbMiTAuMZMFMGUAOHXOlnxO=1490633310;if (HcxDKADtbMiTAuMZMFMGUAOHXOlnxO == HcxDKADtbMiTAuMZMFMGUAOHXOlnxO- 1 ) HcxDKADtbMiTAuMZMFMGUAOHXOlnxO=1509287743; else HcxDKADtbMiTAuMZMFMGUAOHXOlnxO=550611828;if (HcxDKADtbMiTAuMZMFMGUAOHXOlnxO == HcxDKADtbMiTAuMZMFMGUAOHXOlnxO- 0 ) HcxDKADtbMiTAuMZMFMGUAOHXOlnxO=692281142; else HcxDKADtbMiTAuMZMFMGUAOHXOlnxO=2020165561;float dlCORBNjinTqoyqqrPdaUVBGqtxfAS=835972079.342628737008311160479411123478f;if (dlCORBNjinTqoyqqrPdaUVBGqtxfAS - dlCORBNjinTqoyqqrPdaUVBGqtxfAS> 0.00000001 ) dlCORBNjinTqoyqqrPdaUVBGqtxfAS=145842719.693229443355766805015746594251f; else dlCORBNjinTqoyqqrPdaUVBGqtxfAS=1486579886.637052629272813485533856395773f;if (dlCORBNjinTqoyqqrPdaUVBGqtxfAS - dlCORBNjinTqoyqqrPdaUVBGqtxfAS> 0.00000001 ) dlCORBNjinTqoyqqrPdaUVBGqtxfAS=552190741.539382881538785404312073999277f; else dlCORBNjinTqoyqqrPdaUVBGqtxfAS=159297324.895674363160410517200257166759f;if (dlCORBNjinTqoyqqrPdaUVBGqtxfAS - dlCORBNjinTqoyqqrPdaUVBGqtxfAS> 0.00000001 ) dlCORBNjinTqoyqqrPdaUVBGqtxfAS=426971452.472575412114135367348207765951f; else dlCORBNjinTqoyqqrPdaUVBGqtxfAS=1839314804.972480904510620622291041227582f;if (dlCORBNjinTqoyqqrPdaUVBGqtxfAS - dlCORBNjinTqoyqqrPdaUVBGqtxfAS> 0.00000001 ) dlCORBNjinTqoyqqrPdaUVBGqtxfAS=1760427972.167125376646384869824387193236f; else dlCORBNjinTqoyqqrPdaUVBGqtxfAS=1495433331.352256460646765676846977899114f;if (dlCORBNjinTqoyqqrPdaUVBGqtxfAS - dlCORBNjinTqoyqqrPdaUVBGqtxfAS> 0.00000001 ) dlCORBNjinTqoyqqrPdaUVBGqtxfAS=1876975489.224130414841038972252849058556f; else dlCORBNjinTqoyqqrPdaUVBGqtxfAS=215311216.846642273626201695992796258619f;if (dlCORBNjinTqoyqqrPdaUVBGqtxfAS - dlCORBNjinTqoyqqrPdaUVBGqtxfAS> 0.00000001 ) dlCORBNjinTqoyqqrPdaUVBGqtxfAS=442933755.297800649781964665946476425050f; else dlCORBNjinTqoyqqrPdaUVBGqtxfAS=125393780.466553040116631284269477383259f;double dutNLZVHIiHYqTCoSXLPIqsDDtsKgv=848577163.941009138832156806487243824272;if (dutNLZVHIiHYqTCoSXLPIqsDDtsKgv == dutNLZVHIiHYqTCoSXLPIqsDDtsKgv ) dutNLZVHIiHYqTCoSXLPIqsDDtsKgv=735611218.019855742441006483197864681938; else dutNLZVHIiHYqTCoSXLPIqsDDtsKgv=434012191.606782330216514725744219536528;if (dutNLZVHIiHYqTCoSXLPIqsDDtsKgv == dutNLZVHIiHYqTCoSXLPIqsDDtsKgv ) dutNLZVHIiHYqTCoSXLPIqsDDtsKgv=1723700487.592485542515308135106629449826; else dutNLZVHIiHYqTCoSXLPIqsDDtsKgv=1261551518.692373523939333002240152297304;if (dutNLZVHIiHYqTCoSXLPIqsDDtsKgv == dutNLZVHIiHYqTCoSXLPIqsDDtsKgv ) dutNLZVHIiHYqTCoSXLPIqsDDtsKgv=477258421.772793403572328840655869755028; else dutNLZVHIiHYqTCoSXLPIqsDDtsKgv=1655154314.722936874171978893556775092304;if (dutNLZVHIiHYqTCoSXLPIqsDDtsKgv == dutNLZVHIiHYqTCoSXLPIqsDDtsKgv ) dutNLZVHIiHYqTCoSXLPIqsDDtsKgv=644319030.948695040207669500297258305804; else dutNLZVHIiHYqTCoSXLPIqsDDtsKgv=465096716.557123348386445911660222879139;if (dutNLZVHIiHYqTCoSXLPIqsDDtsKgv == dutNLZVHIiHYqTCoSXLPIqsDDtsKgv ) dutNLZVHIiHYqTCoSXLPIqsDDtsKgv=20079690.134685164627201192350425534646; else dutNLZVHIiHYqTCoSXLPIqsDDtsKgv=236856560.531926286096804412178789072016;if (dutNLZVHIiHYqTCoSXLPIqsDDtsKgv == dutNLZVHIiHYqTCoSXLPIqsDDtsKgv ) dutNLZVHIiHYqTCoSXLPIqsDDtsKgv=1873788742.262920638692637472122258885396; else dutNLZVHIiHYqTCoSXLPIqsDDtsKgv=1924820333.437193599588569715370000401514;double STIIhVcURxrPgVUkvuFkyAOgWPojxL=1687765448.898745033677122522046436101308;if (STIIhVcURxrPgVUkvuFkyAOgWPojxL == STIIhVcURxrPgVUkvuFkyAOgWPojxL ) STIIhVcURxrPgVUkvuFkyAOgWPojxL=473265589.093118104036788311537010956274; else STIIhVcURxrPgVUkvuFkyAOgWPojxL=1504934849.483568502542811086880188218437;if (STIIhVcURxrPgVUkvuFkyAOgWPojxL == STIIhVcURxrPgVUkvuFkyAOgWPojxL ) STIIhVcURxrPgVUkvuFkyAOgWPojxL=1326564826.293066323192019521908150540636; else STIIhVcURxrPgVUkvuFkyAOgWPojxL=422293104.017635085676480918980753925463;if (STIIhVcURxrPgVUkvuFkyAOgWPojxL == STIIhVcURxrPgVUkvuFkyAOgWPojxL ) STIIhVcURxrPgVUkvuFkyAOgWPojxL=2017933681.689718548554744287073376647472; else STIIhVcURxrPgVUkvuFkyAOgWPojxL=1066368425.163309777348053612084840041418;if (STIIhVcURxrPgVUkvuFkyAOgWPojxL == STIIhVcURxrPgVUkvuFkyAOgWPojxL ) STIIhVcURxrPgVUkvuFkyAOgWPojxL=785004739.118750244562083633313650832447; else STIIhVcURxrPgVUkvuFkyAOgWPojxL=2066704539.836024131458962387751921667892;if (STIIhVcURxrPgVUkvuFkyAOgWPojxL == STIIhVcURxrPgVUkvuFkyAOgWPojxL ) STIIhVcURxrPgVUkvuFkyAOgWPojxL=131752823.881773716060297680865670602557; else STIIhVcURxrPgVUkvuFkyAOgWPojxL=908225539.834977729978189273508704722228;if (STIIhVcURxrPgVUkvuFkyAOgWPojxL == STIIhVcURxrPgVUkvuFkyAOgWPojxL ) STIIhVcURxrPgVUkvuFkyAOgWPojxL=410423063.226023633096199860153915913036; else STIIhVcURxrPgVUkvuFkyAOgWPojxL=615565592.661285216765152271507228542112;float fBkXOfxJXeVTVvqmdPkVLaQsRWPhSP=149794985.140123259464548380768905030098f;if (fBkXOfxJXeVTVvqmdPkVLaQsRWPhSP - fBkXOfxJXeVTVvqmdPkVLaQsRWPhSP> 0.00000001 ) fBkXOfxJXeVTVvqmdPkVLaQsRWPhSP=1266484584.970465445204811199450116534619f; else fBkXOfxJXeVTVvqmdPkVLaQsRWPhSP=1991187626.327259419495657519504275887503f;if (fBkXOfxJXeVTVvqmdPkVLaQsRWPhSP - fBkXOfxJXeVTVvqmdPkVLaQsRWPhSP> 0.00000001 ) fBkXOfxJXeVTVvqmdPkVLaQsRWPhSP=1284554343.193608777378016666902522625247f; else fBkXOfxJXeVTVvqmdPkVLaQsRWPhSP=113108793.202631206985323627923260167119f;if (fBkXOfxJXeVTVvqmdPkVLaQsRWPhSP - fBkXOfxJXeVTVvqmdPkVLaQsRWPhSP> 0.00000001 ) fBkXOfxJXeVTVvqmdPkVLaQsRWPhSP=281415560.463117758656003830900943404357f; else fBkXOfxJXeVTVvqmdPkVLaQsRWPhSP=891904822.410863322608948737844831962172f;if (fBkXOfxJXeVTVvqmdPkVLaQsRWPhSP - fBkXOfxJXeVTVvqmdPkVLaQsRWPhSP> 0.00000001 ) fBkXOfxJXeVTVvqmdPkVLaQsRWPhSP=358713658.450134735051941301588862469991f; else fBkXOfxJXeVTVvqmdPkVLaQsRWPhSP=638172052.126136981401380309176269579887f;if (fBkXOfxJXeVTVvqmdPkVLaQsRWPhSP - fBkXOfxJXeVTVvqmdPkVLaQsRWPhSP> 0.00000001 ) fBkXOfxJXeVTVvqmdPkVLaQsRWPhSP=842260856.694494581552028758317083506777f; else fBkXOfxJXeVTVvqmdPkVLaQsRWPhSP=87094025.711592899682863525182267061190f;if (fBkXOfxJXeVTVvqmdPkVLaQsRWPhSP - fBkXOfxJXeVTVvqmdPkVLaQsRWPhSP> 0.00000001 ) fBkXOfxJXeVTVvqmdPkVLaQsRWPhSP=139217634.882594153707973523118279565553f; else fBkXOfxJXeVTVvqmdPkVLaQsRWPhSP=190114892.171090048948258630464918272304f;long hAAcQgupPYtZJnEzKFzNZDyvNfKsLr=1798499533;if (hAAcQgupPYtZJnEzKFzNZDyvNfKsLr == hAAcQgupPYtZJnEzKFzNZDyvNfKsLr- 1 ) hAAcQgupPYtZJnEzKFzNZDyvNfKsLr=661579628; else hAAcQgupPYtZJnEzKFzNZDyvNfKsLr=902651954;if (hAAcQgupPYtZJnEzKFzNZDyvNfKsLr == hAAcQgupPYtZJnEzKFzNZDyvNfKsLr- 0 ) hAAcQgupPYtZJnEzKFzNZDyvNfKsLr=1799611838; else hAAcQgupPYtZJnEzKFzNZDyvNfKsLr=1790216305;if (hAAcQgupPYtZJnEzKFzNZDyvNfKsLr == hAAcQgupPYtZJnEzKFzNZDyvNfKsLr- 0 ) hAAcQgupPYtZJnEzKFzNZDyvNfKsLr=32927932; else hAAcQgupPYtZJnEzKFzNZDyvNfKsLr=1009317787;if (hAAcQgupPYtZJnEzKFzNZDyvNfKsLr == hAAcQgupPYtZJnEzKFzNZDyvNfKsLr- 0 ) hAAcQgupPYtZJnEzKFzNZDyvNfKsLr=978127129; else hAAcQgupPYtZJnEzKFzNZDyvNfKsLr=1816726440;if (hAAcQgupPYtZJnEzKFzNZDyvNfKsLr == hAAcQgupPYtZJnEzKFzNZDyvNfKsLr- 1 ) hAAcQgupPYtZJnEzKFzNZDyvNfKsLr=1796491099; else hAAcQgupPYtZJnEzKFzNZDyvNfKsLr=1804479595;if (hAAcQgupPYtZJnEzKFzNZDyvNfKsLr == hAAcQgupPYtZJnEzKFzNZDyvNfKsLr- 0 ) hAAcQgupPYtZJnEzKFzNZDyvNfKsLr=1129798695; else hAAcQgupPYtZJnEzKFzNZDyvNfKsLr=1716516529;long COcyLCZgtrqZcmaRetBLlwMWdpoKzt=1613984732;if (COcyLCZgtrqZcmaRetBLlwMWdpoKzt == COcyLCZgtrqZcmaRetBLlwMWdpoKzt- 0 ) COcyLCZgtrqZcmaRetBLlwMWdpoKzt=2041374404; else COcyLCZgtrqZcmaRetBLlwMWdpoKzt=1723993757;if (COcyLCZgtrqZcmaRetBLlwMWdpoKzt == COcyLCZgtrqZcmaRetBLlwMWdpoKzt- 0 ) COcyLCZgtrqZcmaRetBLlwMWdpoKzt=658335749; else COcyLCZgtrqZcmaRetBLlwMWdpoKzt=1238710674;if (COcyLCZgtrqZcmaRetBLlwMWdpoKzt == COcyLCZgtrqZcmaRetBLlwMWdpoKzt- 0 ) COcyLCZgtrqZcmaRetBLlwMWdpoKzt=941405260; else COcyLCZgtrqZcmaRetBLlwMWdpoKzt=1588106442;if (COcyLCZgtrqZcmaRetBLlwMWdpoKzt == COcyLCZgtrqZcmaRetBLlwMWdpoKzt- 1 ) COcyLCZgtrqZcmaRetBLlwMWdpoKzt=1406349888; else COcyLCZgtrqZcmaRetBLlwMWdpoKzt=1728844414;if (COcyLCZgtrqZcmaRetBLlwMWdpoKzt == COcyLCZgtrqZcmaRetBLlwMWdpoKzt- 0 ) COcyLCZgtrqZcmaRetBLlwMWdpoKzt=1146221191; else COcyLCZgtrqZcmaRetBLlwMWdpoKzt=688392774;if (COcyLCZgtrqZcmaRetBLlwMWdpoKzt == COcyLCZgtrqZcmaRetBLlwMWdpoKzt- 1 ) COcyLCZgtrqZcmaRetBLlwMWdpoKzt=1727515895; else COcyLCZgtrqZcmaRetBLlwMWdpoKzt=1930839656;float nzjDgIHpOPfhqqXpIbisZUgMeXjPfj=296145184.893293446947767485403152300719f;if (nzjDgIHpOPfhqqXpIbisZUgMeXjPfj - nzjDgIHpOPfhqqXpIbisZUgMeXjPfj> 0.00000001 ) nzjDgIHpOPfhqqXpIbisZUgMeXjPfj=495206861.910438146551021596317872975100f; else nzjDgIHpOPfhqqXpIbisZUgMeXjPfj=759407177.374703744935096313282450314957f;if (nzjDgIHpOPfhqqXpIbisZUgMeXjPfj - nzjDgIHpOPfhqqXpIbisZUgMeXjPfj> 0.00000001 ) nzjDgIHpOPfhqqXpIbisZUgMeXjPfj=2101469611.415728742895348398777202803440f; else nzjDgIHpOPfhqqXpIbisZUgMeXjPfj=84496599.089863137868140711213739031393f;if (nzjDgIHpOPfhqqXpIbisZUgMeXjPfj - nzjDgIHpOPfhqqXpIbisZUgMeXjPfj> 0.00000001 ) nzjDgIHpOPfhqqXpIbisZUgMeXjPfj=901450414.540392769636192017329324735083f; else nzjDgIHpOPfhqqXpIbisZUgMeXjPfj=700233145.385533895816279775813518532766f;if (nzjDgIHpOPfhqqXpIbisZUgMeXjPfj - nzjDgIHpOPfhqqXpIbisZUgMeXjPfj> 0.00000001 ) nzjDgIHpOPfhqqXpIbisZUgMeXjPfj=116867053.779745276923251416617138187801f; else nzjDgIHpOPfhqqXpIbisZUgMeXjPfj=257166797.123025864461999147722626964266f;if (nzjDgIHpOPfhqqXpIbisZUgMeXjPfj - nzjDgIHpOPfhqqXpIbisZUgMeXjPfj> 0.00000001 ) nzjDgIHpOPfhqqXpIbisZUgMeXjPfj=1013060497.957691034776326734915129961716f; else nzjDgIHpOPfhqqXpIbisZUgMeXjPfj=1615311550.160862431811306740134136183883f;if (nzjDgIHpOPfhqqXpIbisZUgMeXjPfj - nzjDgIHpOPfhqqXpIbisZUgMeXjPfj> 0.00000001 ) nzjDgIHpOPfhqqXpIbisZUgMeXjPfj=464101341.132871657431410482432178313488f; else nzjDgIHpOPfhqqXpIbisZUgMeXjPfj=624419612.966888064610370473780256207557f;long LlvdkZdPMXLspHJoCrxQDmynjFAfnF=813507418;if (LlvdkZdPMXLspHJoCrxQDmynjFAfnF == LlvdkZdPMXLspHJoCrxQDmynjFAfnF- 0 ) LlvdkZdPMXLspHJoCrxQDmynjFAfnF=609145862; else LlvdkZdPMXLspHJoCrxQDmynjFAfnF=672716549;if (LlvdkZdPMXLspHJoCrxQDmynjFAfnF == LlvdkZdPMXLspHJoCrxQDmynjFAfnF- 0 ) LlvdkZdPMXLspHJoCrxQDmynjFAfnF=296224135; else LlvdkZdPMXLspHJoCrxQDmynjFAfnF=389015952;if (LlvdkZdPMXLspHJoCrxQDmynjFAfnF == LlvdkZdPMXLspHJoCrxQDmynjFAfnF- 0 ) LlvdkZdPMXLspHJoCrxQDmynjFAfnF=1324755537; else LlvdkZdPMXLspHJoCrxQDmynjFAfnF=1321791651;if (LlvdkZdPMXLspHJoCrxQDmynjFAfnF == LlvdkZdPMXLspHJoCrxQDmynjFAfnF- 1 ) LlvdkZdPMXLspHJoCrxQDmynjFAfnF=857094273; else LlvdkZdPMXLspHJoCrxQDmynjFAfnF=1220956833;if (LlvdkZdPMXLspHJoCrxQDmynjFAfnF == LlvdkZdPMXLspHJoCrxQDmynjFAfnF- 0 ) LlvdkZdPMXLspHJoCrxQDmynjFAfnF=1643785713; else LlvdkZdPMXLspHJoCrxQDmynjFAfnF=70767424;if (LlvdkZdPMXLspHJoCrxQDmynjFAfnF == LlvdkZdPMXLspHJoCrxQDmynjFAfnF- 1 ) LlvdkZdPMXLspHJoCrxQDmynjFAfnF=643995114; else LlvdkZdPMXLspHJoCrxQDmynjFAfnF=1492018805;double pOeARDKzRUbVVZWiVPsbbeOAUjhEXx=1201601252.471435984377748354969911919789;if (pOeARDKzRUbVVZWiVPsbbeOAUjhEXx == pOeARDKzRUbVVZWiVPsbbeOAUjhEXx ) pOeARDKzRUbVVZWiVPsbbeOAUjhEXx=1850784765.222889848779161165533460056053; else pOeARDKzRUbVVZWiVPsbbeOAUjhEXx=833653511.630626606051730096195526796293;if (pOeARDKzRUbVVZWiVPsbbeOAUjhEXx == pOeARDKzRUbVVZWiVPsbbeOAUjhEXx ) pOeARDKzRUbVVZWiVPsbbeOAUjhEXx=1221309668.390615954657969274107106821504; else pOeARDKzRUbVVZWiVPsbbeOAUjhEXx=485324074.041376123524254877752310730580;if (pOeARDKzRUbVVZWiVPsbbeOAUjhEXx == pOeARDKzRUbVVZWiVPsbbeOAUjhEXx ) pOeARDKzRUbVVZWiVPsbbeOAUjhEXx=74767474.556172679426179495205485384275; else pOeARDKzRUbVVZWiVPsbbeOAUjhEXx=1540935211.674417829331843624234638517199;if (pOeARDKzRUbVVZWiVPsbbeOAUjhEXx == pOeARDKzRUbVVZWiVPsbbeOAUjhEXx ) pOeARDKzRUbVVZWiVPsbbeOAUjhEXx=712534318.840586287278798574376014114701; else pOeARDKzRUbVVZWiVPsbbeOAUjhEXx=1341141259.173069147855807794105393483658;if (pOeARDKzRUbVVZWiVPsbbeOAUjhEXx == pOeARDKzRUbVVZWiVPsbbeOAUjhEXx ) pOeARDKzRUbVVZWiVPsbbeOAUjhEXx=1607126704.268610574053853121842160689178; else pOeARDKzRUbVVZWiVPsbbeOAUjhEXx=401168952.650970820832026265136391620914;if (pOeARDKzRUbVVZWiVPsbbeOAUjhEXx == pOeARDKzRUbVVZWiVPsbbeOAUjhEXx ) pOeARDKzRUbVVZWiVPsbbeOAUjhEXx=225650177.241740499440212015453415247945; else pOeARDKzRUbVVZWiVPsbbeOAUjhEXx=1473577357.092679248787789619962556283751;int abQRZqeVNthloncnSeePpznUaZiWvf=356540415;if (abQRZqeVNthloncnSeePpznUaZiWvf == abQRZqeVNthloncnSeePpznUaZiWvf- 0 ) abQRZqeVNthloncnSeePpznUaZiWvf=1294951973; else abQRZqeVNthloncnSeePpznUaZiWvf=914008477;if (abQRZqeVNthloncnSeePpznUaZiWvf == abQRZqeVNthloncnSeePpznUaZiWvf- 0 ) abQRZqeVNthloncnSeePpznUaZiWvf=1321693794; else abQRZqeVNthloncnSeePpznUaZiWvf=254434242;if (abQRZqeVNthloncnSeePpznUaZiWvf == abQRZqeVNthloncnSeePpznUaZiWvf- 1 ) abQRZqeVNthloncnSeePpznUaZiWvf=298665712; else abQRZqeVNthloncnSeePpznUaZiWvf=1712162525;if (abQRZqeVNthloncnSeePpznUaZiWvf == abQRZqeVNthloncnSeePpznUaZiWvf- 0 ) abQRZqeVNthloncnSeePpznUaZiWvf=1118873294; else abQRZqeVNthloncnSeePpznUaZiWvf=1320898857;if (abQRZqeVNthloncnSeePpznUaZiWvf == abQRZqeVNthloncnSeePpznUaZiWvf- 1 ) abQRZqeVNthloncnSeePpznUaZiWvf=2049796551; else abQRZqeVNthloncnSeePpznUaZiWvf=880512778;if (abQRZqeVNthloncnSeePpznUaZiWvf == abQRZqeVNthloncnSeePpznUaZiWvf- 1 ) abQRZqeVNthloncnSeePpznUaZiWvf=588194533; else abQRZqeVNthloncnSeePpznUaZiWvf=830682990;float kaVpqTPoDqzSxkkbOyodSeyuHJBzwf=1965773024.656099500047995898603248589500f;if (kaVpqTPoDqzSxkkbOyodSeyuHJBzwf - kaVpqTPoDqzSxkkbOyodSeyuHJBzwf> 0.00000001 ) kaVpqTPoDqzSxkkbOyodSeyuHJBzwf=725735901.562387246629868670555829551861f; else kaVpqTPoDqzSxkkbOyodSeyuHJBzwf=1409983736.110159562975208459191915584493f;if (kaVpqTPoDqzSxkkbOyodSeyuHJBzwf - kaVpqTPoDqzSxkkbOyodSeyuHJBzwf> 0.00000001 ) kaVpqTPoDqzSxkkbOyodSeyuHJBzwf=921118494.648661971912969062111388021668f; else kaVpqTPoDqzSxkkbOyodSeyuHJBzwf=1075936263.528933764858828015762338347165f;if (kaVpqTPoDqzSxkkbOyodSeyuHJBzwf - kaVpqTPoDqzSxkkbOyodSeyuHJBzwf> 0.00000001 ) kaVpqTPoDqzSxkkbOyodSeyuHJBzwf=1715854755.142350514630947356012691730069f; else kaVpqTPoDqzSxkkbOyodSeyuHJBzwf=1055600374.771971768960213249219126455172f;if (kaVpqTPoDqzSxkkbOyodSeyuHJBzwf - kaVpqTPoDqzSxkkbOyodSeyuHJBzwf> 0.00000001 ) kaVpqTPoDqzSxkkbOyodSeyuHJBzwf=738739944.055507007670915259859196166563f; else kaVpqTPoDqzSxkkbOyodSeyuHJBzwf=1215694298.769692582625265031131742406054f;if (kaVpqTPoDqzSxkkbOyodSeyuHJBzwf - kaVpqTPoDqzSxkkbOyodSeyuHJBzwf> 0.00000001 ) kaVpqTPoDqzSxkkbOyodSeyuHJBzwf=2107665878.255501133478723504114653303730f; else kaVpqTPoDqzSxkkbOyodSeyuHJBzwf=1102566523.664958177490876302991394071230f;if (kaVpqTPoDqzSxkkbOyodSeyuHJBzwf - kaVpqTPoDqzSxkkbOyodSeyuHJBzwf> 0.00000001 ) kaVpqTPoDqzSxkkbOyodSeyuHJBzwf=929372895.841482598653773488800294047099f; else kaVpqTPoDqzSxkkbOyodSeyuHJBzwf=923586585.795765059592535963260354176755f;int XvutdnyEnUOwQCnlctFoUrPwZXGhvt=861855437;if (XvutdnyEnUOwQCnlctFoUrPwZXGhvt == XvutdnyEnUOwQCnlctFoUrPwZXGhvt- 0 ) XvutdnyEnUOwQCnlctFoUrPwZXGhvt=251512784; else XvutdnyEnUOwQCnlctFoUrPwZXGhvt=287454699;if (XvutdnyEnUOwQCnlctFoUrPwZXGhvt == XvutdnyEnUOwQCnlctFoUrPwZXGhvt- 0 ) XvutdnyEnUOwQCnlctFoUrPwZXGhvt=197066832; else XvutdnyEnUOwQCnlctFoUrPwZXGhvt=515277062;if (XvutdnyEnUOwQCnlctFoUrPwZXGhvt == XvutdnyEnUOwQCnlctFoUrPwZXGhvt- 0 ) XvutdnyEnUOwQCnlctFoUrPwZXGhvt=2020560973; else XvutdnyEnUOwQCnlctFoUrPwZXGhvt=635141407;if (XvutdnyEnUOwQCnlctFoUrPwZXGhvt == XvutdnyEnUOwQCnlctFoUrPwZXGhvt- 1 ) XvutdnyEnUOwQCnlctFoUrPwZXGhvt=902935540; else XvutdnyEnUOwQCnlctFoUrPwZXGhvt=138622240;if (XvutdnyEnUOwQCnlctFoUrPwZXGhvt == XvutdnyEnUOwQCnlctFoUrPwZXGhvt- 1 ) XvutdnyEnUOwQCnlctFoUrPwZXGhvt=1862263864; else XvutdnyEnUOwQCnlctFoUrPwZXGhvt=2047894458;if (XvutdnyEnUOwQCnlctFoUrPwZXGhvt == XvutdnyEnUOwQCnlctFoUrPwZXGhvt- 0 ) XvutdnyEnUOwQCnlctFoUrPwZXGhvt=583011170; else XvutdnyEnUOwQCnlctFoUrPwZXGhvt=1341738343;float fKwBAxlSmntznSSTdgwseIeiJRourq=834794809.073284668119070420617714943511f;if (fKwBAxlSmntznSSTdgwseIeiJRourq - fKwBAxlSmntznSSTdgwseIeiJRourq> 0.00000001 ) fKwBAxlSmntznSSTdgwseIeiJRourq=1573287694.696466883069869150873204546034f; else fKwBAxlSmntznSSTdgwseIeiJRourq=407360800.216762371475775996599485959293f;if (fKwBAxlSmntznSSTdgwseIeiJRourq - fKwBAxlSmntznSSTdgwseIeiJRourq> 0.00000001 ) fKwBAxlSmntznSSTdgwseIeiJRourq=747841456.095096421889940854965165693191f; else fKwBAxlSmntznSSTdgwseIeiJRourq=1861505012.428859089067681155510633903141f;if (fKwBAxlSmntznSSTdgwseIeiJRourq - fKwBAxlSmntznSSTdgwseIeiJRourq> 0.00000001 ) fKwBAxlSmntznSSTdgwseIeiJRourq=2008295429.333047229288601869564978199506f; else fKwBAxlSmntznSSTdgwseIeiJRourq=890802024.604983564494643314315569746606f;if (fKwBAxlSmntznSSTdgwseIeiJRourq - fKwBAxlSmntznSSTdgwseIeiJRourq> 0.00000001 ) fKwBAxlSmntznSSTdgwseIeiJRourq=788169307.982469573840617144598520124524f; else fKwBAxlSmntznSSTdgwseIeiJRourq=239351599.796862228584257506361312846345f;if (fKwBAxlSmntznSSTdgwseIeiJRourq - fKwBAxlSmntznSSTdgwseIeiJRourq> 0.00000001 ) fKwBAxlSmntznSSTdgwseIeiJRourq=175268533.516925486503611548473926459502f; else fKwBAxlSmntznSSTdgwseIeiJRourq=105999496.799408834212197870140807379067f;if (fKwBAxlSmntznSSTdgwseIeiJRourq - fKwBAxlSmntznSSTdgwseIeiJRourq> 0.00000001 ) fKwBAxlSmntznSSTdgwseIeiJRourq=294252011.709491531471526194069957390224f; else fKwBAxlSmntznSSTdgwseIeiJRourq=1343582836.998762470644977948228610985220f;float PIlcsRphWiHTSKWzsxReKBwdRpppnk=518795988.952791585125451944500281014468f;if (PIlcsRphWiHTSKWzsxReKBwdRpppnk - PIlcsRphWiHTSKWzsxReKBwdRpppnk> 0.00000001 ) PIlcsRphWiHTSKWzsxReKBwdRpppnk=783561046.811418542304615932113732388279f; else PIlcsRphWiHTSKWzsxReKBwdRpppnk=1873822743.722741515351221668728336619913f;if (PIlcsRphWiHTSKWzsxReKBwdRpppnk - PIlcsRphWiHTSKWzsxReKBwdRpppnk> 0.00000001 ) PIlcsRphWiHTSKWzsxReKBwdRpppnk=579170206.147468207443796251272156150713f; else PIlcsRphWiHTSKWzsxReKBwdRpppnk=1916266979.360221596356138664056885219132f;if (PIlcsRphWiHTSKWzsxReKBwdRpppnk - PIlcsRphWiHTSKWzsxReKBwdRpppnk> 0.00000001 ) PIlcsRphWiHTSKWzsxReKBwdRpppnk=575265063.960429357587693602869677858612f; else PIlcsRphWiHTSKWzsxReKBwdRpppnk=460087843.246450772031858030261847339378f;if (PIlcsRphWiHTSKWzsxReKBwdRpppnk - PIlcsRphWiHTSKWzsxReKBwdRpppnk> 0.00000001 ) PIlcsRphWiHTSKWzsxReKBwdRpppnk=2092695698.185544125493094993992946251012f; else PIlcsRphWiHTSKWzsxReKBwdRpppnk=90594494.151595732492697885455659625049f;if (PIlcsRphWiHTSKWzsxReKBwdRpppnk - PIlcsRphWiHTSKWzsxReKBwdRpppnk> 0.00000001 ) PIlcsRphWiHTSKWzsxReKBwdRpppnk=507048196.421335360926160145958777750307f; else PIlcsRphWiHTSKWzsxReKBwdRpppnk=782496254.180317137262554060959010286578f;if (PIlcsRphWiHTSKWzsxReKBwdRpppnk - PIlcsRphWiHTSKWzsxReKBwdRpppnk> 0.00000001 ) PIlcsRphWiHTSKWzsxReKBwdRpppnk=1361156046.517385785401979622380145917124f; else PIlcsRphWiHTSKWzsxReKBwdRpppnk=1815443944.277985628938566824176520847357f;int fgDLQYxwWzyUWVLfrtBvPzbqibjESo=78330903;if (fgDLQYxwWzyUWVLfrtBvPzbqibjESo == fgDLQYxwWzyUWVLfrtBvPzbqibjESo- 1 ) fgDLQYxwWzyUWVLfrtBvPzbqibjESo=1965146942; else fgDLQYxwWzyUWVLfrtBvPzbqibjESo=94578086;if (fgDLQYxwWzyUWVLfrtBvPzbqibjESo == fgDLQYxwWzyUWVLfrtBvPzbqibjESo- 0 ) fgDLQYxwWzyUWVLfrtBvPzbqibjESo=1497782538; else fgDLQYxwWzyUWVLfrtBvPzbqibjESo=688780674;if (fgDLQYxwWzyUWVLfrtBvPzbqibjESo == fgDLQYxwWzyUWVLfrtBvPzbqibjESo- 1 ) fgDLQYxwWzyUWVLfrtBvPzbqibjESo=783159107; else fgDLQYxwWzyUWVLfrtBvPzbqibjESo=382930367;if (fgDLQYxwWzyUWVLfrtBvPzbqibjESo == fgDLQYxwWzyUWVLfrtBvPzbqibjESo- 0 ) fgDLQYxwWzyUWVLfrtBvPzbqibjESo=1001043655; else fgDLQYxwWzyUWVLfrtBvPzbqibjESo=1508194407;if (fgDLQYxwWzyUWVLfrtBvPzbqibjESo == fgDLQYxwWzyUWVLfrtBvPzbqibjESo- 0 ) fgDLQYxwWzyUWVLfrtBvPzbqibjESo=1646769039; else fgDLQYxwWzyUWVLfrtBvPzbqibjESo=2105198880;if (fgDLQYxwWzyUWVLfrtBvPzbqibjESo == fgDLQYxwWzyUWVLfrtBvPzbqibjESo- 0 ) fgDLQYxwWzyUWVLfrtBvPzbqibjESo=340008529; else fgDLQYxwWzyUWVLfrtBvPzbqibjESo=1914561477;int bwoxkosGdykxSFbdlNZWXahlkveYzv=1666977155;if (bwoxkosGdykxSFbdlNZWXahlkveYzv == bwoxkosGdykxSFbdlNZWXahlkveYzv- 1 ) bwoxkosGdykxSFbdlNZWXahlkveYzv=1823137075; else bwoxkosGdykxSFbdlNZWXahlkveYzv=64215182;if (bwoxkosGdykxSFbdlNZWXahlkveYzv == bwoxkosGdykxSFbdlNZWXahlkveYzv- 0 ) bwoxkosGdykxSFbdlNZWXahlkveYzv=558177297; else bwoxkosGdykxSFbdlNZWXahlkveYzv=1226401582;if (bwoxkosGdykxSFbdlNZWXahlkveYzv == bwoxkosGdykxSFbdlNZWXahlkveYzv- 0 ) bwoxkosGdykxSFbdlNZWXahlkveYzv=1450164769; else bwoxkosGdykxSFbdlNZWXahlkveYzv=1516119896;if (bwoxkosGdykxSFbdlNZWXahlkveYzv == bwoxkosGdykxSFbdlNZWXahlkveYzv- 0 ) bwoxkosGdykxSFbdlNZWXahlkveYzv=259529746; else bwoxkosGdykxSFbdlNZWXahlkveYzv=1113199851;if (bwoxkosGdykxSFbdlNZWXahlkveYzv == bwoxkosGdykxSFbdlNZWXahlkveYzv- 1 ) bwoxkosGdykxSFbdlNZWXahlkveYzv=462148749; else bwoxkosGdykxSFbdlNZWXahlkveYzv=657246962;if (bwoxkosGdykxSFbdlNZWXahlkveYzv == bwoxkosGdykxSFbdlNZWXahlkveYzv- 0 ) bwoxkosGdykxSFbdlNZWXahlkveYzv=1354456816; else bwoxkosGdykxSFbdlNZWXahlkveYzv=1225799175;long ewVAsDCISqDpoZkgMpVGLhRCUyhndJ=778266932;if (ewVAsDCISqDpoZkgMpVGLhRCUyhndJ == ewVAsDCISqDpoZkgMpVGLhRCUyhndJ- 0 ) ewVAsDCISqDpoZkgMpVGLhRCUyhndJ=1990529523; else ewVAsDCISqDpoZkgMpVGLhRCUyhndJ=215921108;if (ewVAsDCISqDpoZkgMpVGLhRCUyhndJ == ewVAsDCISqDpoZkgMpVGLhRCUyhndJ- 1 ) ewVAsDCISqDpoZkgMpVGLhRCUyhndJ=1525919541; else ewVAsDCISqDpoZkgMpVGLhRCUyhndJ=688211544;if (ewVAsDCISqDpoZkgMpVGLhRCUyhndJ == ewVAsDCISqDpoZkgMpVGLhRCUyhndJ- 1 ) ewVAsDCISqDpoZkgMpVGLhRCUyhndJ=844987818; else ewVAsDCISqDpoZkgMpVGLhRCUyhndJ=1683460356;if (ewVAsDCISqDpoZkgMpVGLhRCUyhndJ == ewVAsDCISqDpoZkgMpVGLhRCUyhndJ- 1 ) ewVAsDCISqDpoZkgMpVGLhRCUyhndJ=1883120528; else ewVAsDCISqDpoZkgMpVGLhRCUyhndJ=400630679;if (ewVAsDCISqDpoZkgMpVGLhRCUyhndJ == ewVAsDCISqDpoZkgMpVGLhRCUyhndJ- 0 ) ewVAsDCISqDpoZkgMpVGLhRCUyhndJ=872779088; else ewVAsDCISqDpoZkgMpVGLhRCUyhndJ=1218459446;if (ewVAsDCISqDpoZkgMpVGLhRCUyhndJ == ewVAsDCISqDpoZkgMpVGLhRCUyhndJ- 0 ) ewVAsDCISqDpoZkgMpVGLhRCUyhndJ=269497940; else ewVAsDCISqDpoZkgMpVGLhRCUyhndJ=617549830;long KGngbcMDToesvktwnSIZVaXVbYaxDi=947695492;if (KGngbcMDToesvktwnSIZVaXVbYaxDi == KGngbcMDToesvktwnSIZVaXVbYaxDi- 0 ) KGngbcMDToesvktwnSIZVaXVbYaxDi=1426663644; else KGngbcMDToesvktwnSIZVaXVbYaxDi=1841662538;if (KGngbcMDToesvktwnSIZVaXVbYaxDi == KGngbcMDToesvktwnSIZVaXVbYaxDi- 1 ) KGngbcMDToesvktwnSIZVaXVbYaxDi=2139790516; else KGngbcMDToesvktwnSIZVaXVbYaxDi=408374022;if (KGngbcMDToesvktwnSIZVaXVbYaxDi == KGngbcMDToesvktwnSIZVaXVbYaxDi- 1 ) KGngbcMDToesvktwnSIZVaXVbYaxDi=986307951; else KGngbcMDToesvktwnSIZVaXVbYaxDi=1184035685;if (KGngbcMDToesvktwnSIZVaXVbYaxDi == KGngbcMDToesvktwnSIZVaXVbYaxDi- 0 ) KGngbcMDToesvktwnSIZVaXVbYaxDi=775649781; else KGngbcMDToesvktwnSIZVaXVbYaxDi=449027046;if (KGngbcMDToesvktwnSIZVaXVbYaxDi == KGngbcMDToesvktwnSIZVaXVbYaxDi- 1 ) KGngbcMDToesvktwnSIZVaXVbYaxDi=1982531987; else KGngbcMDToesvktwnSIZVaXVbYaxDi=384582687;if (KGngbcMDToesvktwnSIZVaXVbYaxDi == KGngbcMDToesvktwnSIZVaXVbYaxDi- 1 ) KGngbcMDToesvktwnSIZVaXVbYaxDi=226631925; else KGngbcMDToesvktwnSIZVaXVbYaxDi=125912145;float haIDOFMHhqEXrBasGAChupAtebCDTj=526560029.206256112707327308853965482536f;if (haIDOFMHhqEXrBasGAChupAtebCDTj - haIDOFMHhqEXrBasGAChupAtebCDTj> 0.00000001 ) haIDOFMHhqEXrBasGAChupAtebCDTj=190952838.891308805023194006612442245288f; else haIDOFMHhqEXrBasGAChupAtebCDTj=1709225244.579791712060484087158603446916f;if (haIDOFMHhqEXrBasGAChupAtebCDTj - haIDOFMHhqEXrBasGAChupAtebCDTj> 0.00000001 ) haIDOFMHhqEXrBasGAChupAtebCDTj=2071515601.363865550340465217937813098736f; else haIDOFMHhqEXrBasGAChupAtebCDTj=124968288.680780863964792934834019055097f;if (haIDOFMHhqEXrBasGAChupAtebCDTj - haIDOFMHhqEXrBasGAChupAtebCDTj> 0.00000001 ) haIDOFMHhqEXrBasGAChupAtebCDTj=180916810.882576189646418313227401277283f; else haIDOFMHhqEXrBasGAChupAtebCDTj=815104713.598718322696639860174387059231f;if (haIDOFMHhqEXrBasGAChupAtebCDTj - haIDOFMHhqEXrBasGAChupAtebCDTj> 0.00000001 ) haIDOFMHhqEXrBasGAChupAtebCDTj=821989337.990716881119164298252811931176f; else haIDOFMHhqEXrBasGAChupAtebCDTj=139676719.010270602318016522497109928089f;if (haIDOFMHhqEXrBasGAChupAtebCDTj - haIDOFMHhqEXrBasGAChupAtebCDTj> 0.00000001 ) haIDOFMHhqEXrBasGAChupAtebCDTj=328578301.582520000469605481541079092135f; else haIDOFMHhqEXrBasGAChupAtebCDTj=157546618.272493750762313861565854861463f;if (haIDOFMHhqEXrBasGAChupAtebCDTj - haIDOFMHhqEXrBasGAChupAtebCDTj> 0.00000001 ) haIDOFMHhqEXrBasGAChupAtebCDTj=353622494.908530831284299977624268152863f; else haIDOFMHhqEXrBasGAChupAtebCDTj=231705323.340786588313049266699584084159f;int vjpuUVbFPTwtRFCJLuvteMiVqeakCJ=284014528;if (vjpuUVbFPTwtRFCJLuvteMiVqeakCJ == vjpuUVbFPTwtRFCJLuvteMiVqeakCJ- 1 ) vjpuUVbFPTwtRFCJLuvteMiVqeakCJ=907541864; else vjpuUVbFPTwtRFCJLuvteMiVqeakCJ=406761324;if (vjpuUVbFPTwtRFCJLuvteMiVqeakCJ == vjpuUVbFPTwtRFCJLuvteMiVqeakCJ- 1 ) vjpuUVbFPTwtRFCJLuvteMiVqeakCJ=1946619178; else vjpuUVbFPTwtRFCJLuvteMiVqeakCJ=1982950734;if (vjpuUVbFPTwtRFCJLuvteMiVqeakCJ == vjpuUVbFPTwtRFCJLuvteMiVqeakCJ- 1 ) vjpuUVbFPTwtRFCJLuvteMiVqeakCJ=1127902141; else vjpuUVbFPTwtRFCJLuvteMiVqeakCJ=1505872020;if (vjpuUVbFPTwtRFCJLuvteMiVqeakCJ == vjpuUVbFPTwtRFCJLuvteMiVqeakCJ- 0 ) vjpuUVbFPTwtRFCJLuvteMiVqeakCJ=1939442628; else vjpuUVbFPTwtRFCJLuvteMiVqeakCJ=806493838;if (vjpuUVbFPTwtRFCJLuvteMiVqeakCJ == vjpuUVbFPTwtRFCJLuvteMiVqeakCJ- 0 ) vjpuUVbFPTwtRFCJLuvteMiVqeakCJ=1587754974; else vjpuUVbFPTwtRFCJLuvteMiVqeakCJ=1076892495;if (vjpuUVbFPTwtRFCJLuvteMiVqeakCJ == vjpuUVbFPTwtRFCJLuvteMiVqeakCJ- 1 ) vjpuUVbFPTwtRFCJLuvteMiVqeakCJ=853163204; else vjpuUVbFPTwtRFCJLuvteMiVqeakCJ=460405131;double XEZBWhSgNDnBzWmOrpDZGqCKwdmjrr=1097534608.053644806635509709736968584570;if (XEZBWhSgNDnBzWmOrpDZGqCKwdmjrr == XEZBWhSgNDnBzWmOrpDZGqCKwdmjrr ) XEZBWhSgNDnBzWmOrpDZGqCKwdmjrr=1387394664.802229955203905898053565619517; else XEZBWhSgNDnBzWmOrpDZGqCKwdmjrr=1890421558.134694348096563316489763734771;if (XEZBWhSgNDnBzWmOrpDZGqCKwdmjrr == XEZBWhSgNDnBzWmOrpDZGqCKwdmjrr ) XEZBWhSgNDnBzWmOrpDZGqCKwdmjrr=426596992.044917032166006801241323848106; else XEZBWhSgNDnBzWmOrpDZGqCKwdmjrr=558337717.167815275739533386703353173118;if (XEZBWhSgNDnBzWmOrpDZGqCKwdmjrr == XEZBWhSgNDnBzWmOrpDZGqCKwdmjrr ) XEZBWhSgNDnBzWmOrpDZGqCKwdmjrr=254102267.094610920263761290157250481574; else XEZBWhSgNDnBzWmOrpDZGqCKwdmjrr=1772220457.217774603346754070209417295433;if (XEZBWhSgNDnBzWmOrpDZGqCKwdmjrr == XEZBWhSgNDnBzWmOrpDZGqCKwdmjrr ) XEZBWhSgNDnBzWmOrpDZGqCKwdmjrr=766778682.056605700879791150642238591069; else XEZBWhSgNDnBzWmOrpDZGqCKwdmjrr=611558931.927140015037153073614689183912;if (XEZBWhSgNDnBzWmOrpDZGqCKwdmjrr == XEZBWhSgNDnBzWmOrpDZGqCKwdmjrr ) XEZBWhSgNDnBzWmOrpDZGqCKwdmjrr=1019673201.575195275215571628562955644906; else XEZBWhSgNDnBzWmOrpDZGqCKwdmjrr=2000113797.720457207881850022967546173066;if (XEZBWhSgNDnBzWmOrpDZGqCKwdmjrr == XEZBWhSgNDnBzWmOrpDZGqCKwdmjrr ) XEZBWhSgNDnBzWmOrpDZGqCKwdmjrr=79795721.247950513542288211572305039449; else XEZBWhSgNDnBzWmOrpDZGqCKwdmjrr=828761333.425175490151052547996360405203;float DnTFIoCEOkIDVHDOdQZuKaWNEjZklA=193068424.601662256389997253740597641664f;if (DnTFIoCEOkIDVHDOdQZuKaWNEjZklA - DnTFIoCEOkIDVHDOdQZuKaWNEjZklA> 0.00000001 ) DnTFIoCEOkIDVHDOdQZuKaWNEjZklA=1538945797.623167440464765899999213121412f; else DnTFIoCEOkIDVHDOdQZuKaWNEjZklA=2090101044.260276693091348694396288542115f;if (DnTFIoCEOkIDVHDOdQZuKaWNEjZklA - DnTFIoCEOkIDVHDOdQZuKaWNEjZklA> 0.00000001 ) DnTFIoCEOkIDVHDOdQZuKaWNEjZklA=1365853928.704831550920690998653823708283f; else DnTFIoCEOkIDVHDOdQZuKaWNEjZklA=769195874.568949761825447679876456208224f;if (DnTFIoCEOkIDVHDOdQZuKaWNEjZklA - DnTFIoCEOkIDVHDOdQZuKaWNEjZklA> 0.00000001 ) DnTFIoCEOkIDVHDOdQZuKaWNEjZklA=2123592596.337756276124794849613051811384f; else DnTFIoCEOkIDVHDOdQZuKaWNEjZklA=2143081342.264859870141476287346062149705f;if (DnTFIoCEOkIDVHDOdQZuKaWNEjZklA - DnTFIoCEOkIDVHDOdQZuKaWNEjZklA> 0.00000001 ) DnTFIoCEOkIDVHDOdQZuKaWNEjZklA=1546141535.600207004617967369674504920372f; else DnTFIoCEOkIDVHDOdQZuKaWNEjZklA=832564465.028565227928474011588114220649f;if (DnTFIoCEOkIDVHDOdQZuKaWNEjZklA - DnTFIoCEOkIDVHDOdQZuKaWNEjZklA> 0.00000001 ) DnTFIoCEOkIDVHDOdQZuKaWNEjZklA=1917266263.783260051325275614191099501445f; else DnTFIoCEOkIDVHDOdQZuKaWNEjZklA=1265021763.568935532507127395961765889487f;if (DnTFIoCEOkIDVHDOdQZuKaWNEjZklA - DnTFIoCEOkIDVHDOdQZuKaWNEjZklA> 0.00000001 ) DnTFIoCEOkIDVHDOdQZuKaWNEjZklA=684696961.961696835119269507321658481325f; else DnTFIoCEOkIDVHDOdQZuKaWNEjZklA=1251507962.615665654166095350739326632331f;int XIzbBRjzYsaOKdaffPEOCirhOvuzPP=718853565;if (XIzbBRjzYsaOKdaffPEOCirhOvuzPP == XIzbBRjzYsaOKdaffPEOCirhOvuzPP- 0 ) XIzbBRjzYsaOKdaffPEOCirhOvuzPP=932873137; else XIzbBRjzYsaOKdaffPEOCirhOvuzPP=1403571533;if (XIzbBRjzYsaOKdaffPEOCirhOvuzPP == XIzbBRjzYsaOKdaffPEOCirhOvuzPP- 1 ) XIzbBRjzYsaOKdaffPEOCirhOvuzPP=991794306; else XIzbBRjzYsaOKdaffPEOCirhOvuzPP=1503684045;if (XIzbBRjzYsaOKdaffPEOCirhOvuzPP == XIzbBRjzYsaOKdaffPEOCirhOvuzPP- 1 ) XIzbBRjzYsaOKdaffPEOCirhOvuzPP=1362866571; else XIzbBRjzYsaOKdaffPEOCirhOvuzPP=434782820;if (XIzbBRjzYsaOKdaffPEOCirhOvuzPP == XIzbBRjzYsaOKdaffPEOCirhOvuzPP- 1 ) XIzbBRjzYsaOKdaffPEOCirhOvuzPP=29912261; else XIzbBRjzYsaOKdaffPEOCirhOvuzPP=1282805054;if (XIzbBRjzYsaOKdaffPEOCirhOvuzPP == XIzbBRjzYsaOKdaffPEOCirhOvuzPP- 0 ) XIzbBRjzYsaOKdaffPEOCirhOvuzPP=1787466289; else XIzbBRjzYsaOKdaffPEOCirhOvuzPP=1335060247;if (XIzbBRjzYsaOKdaffPEOCirhOvuzPP == XIzbBRjzYsaOKdaffPEOCirhOvuzPP- 1 ) XIzbBRjzYsaOKdaffPEOCirhOvuzPP=963277533; else XIzbBRjzYsaOKdaffPEOCirhOvuzPP=1275271991;long pzYjkiqlTUZKXAuyLetMhRAAKUdCPi=1043810516;if (pzYjkiqlTUZKXAuyLetMhRAAKUdCPi == pzYjkiqlTUZKXAuyLetMhRAAKUdCPi- 1 ) pzYjkiqlTUZKXAuyLetMhRAAKUdCPi=1874487405; else pzYjkiqlTUZKXAuyLetMhRAAKUdCPi=660094780;if (pzYjkiqlTUZKXAuyLetMhRAAKUdCPi == pzYjkiqlTUZKXAuyLetMhRAAKUdCPi- 0 ) pzYjkiqlTUZKXAuyLetMhRAAKUdCPi=1068942981; else pzYjkiqlTUZKXAuyLetMhRAAKUdCPi=1514211034;if (pzYjkiqlTUZKXAuyLetMhRAAKUdCPi == pzYjkiqlTUZKXAuyLetMhRAAKUdCPi- 0 ) pzYjkiqlTUZKXAuyLetMhRAAKUdCPi=391354455; else pzYjkiqlTUZKXAuyLetMhRAAKUdCPi=602961254;if (pzYjkiqlTUZKXAuyLetMhRAAKUdCPi == pzYjkiqlTUZKXAuyLetMhRAAKUdCPi- 1 ) pzYjkiqlTUZKXAuyLetMhRAAKUdCPi=1121425659; else pzYjkiqlTUZKXAuyLetMhRAAKUdCPi=1391796969;if (pzYjkiqlTUZKXAuyLetMhRAAKUdCPi == pzYjkiqlTUZKXAuyLetMhRAAKUdCPi- 1 ) pzYjkiqlTUZKXAuyLetMhRAAKUdCPi=344280023; else pzYjkiqlTUZKXAuyLetMhRAAKUdCPi=1741516174;if (pzYjkiqlTUZKXAuyLetMhRAAKUdCPi == pzYjkiqlTUZKXAuyLetMhRAAKUdCPi- 0 ) pzYjkiqlTUZKXAuyLetMhRAAKUdCPi=788555101; else pzYjkiqlTUZKXAuyLetMhRAAKUdCPi=1136564151;long FQTbOzzvdnlxZSJIMENkslkNzyjdfP=658088995;if (FQTbOzzvdnlxZSJIMENkslkNzyjdfP == FQTbOzzvdnlxZSJIMENkslkNzyjdfP- 1 ) FQTbOzzvdnlxZSJIMENkslkNzyjdfP=1169781862; else FQTbOzzvdnlxZSJIMENkslkNzyjdfP=1508425803;if (FQTbOzzvdnlxZSJIMENkslkNzyjdfP == FQTbOzzvdnlxZSJIMENkslkNzyjdfP- 1 ) FQTbOzzvdnlxZSJIMENkslkNzyjdfP=2043371843; else FQTbOzzvdnlxZSJIMENkslkNzyjdfP=1574325305;if (FQTbOzzvdnlxZSJIMENkslkNzyjdfP == FQTbOzzvdnlxZSJIMENkslkNzyjdfP- 0 ) FQTbOzzvdnlxZSJIMENkslkNzyjdfP=1699633799; else FQTbOzzvdnlxZSJIMENkslkNzyjdfP=1993688339;if (FQTbOzzvdnlxZSJIMENkslkNzyjdfP == FQTbOzzvdnlxZSJIMENkslkNzyjdfP- 1 ) FQTbOzzvdnlxZSJIMENkslkNzyjdfP=977685500; else FQTbOzzvdnlxZSJIMENkslkNzyjdfP=1104162977;if (FQTbOzzvdnlxZSJIMENkslkNzyjdfP == FQTbOzzvdnlxZSJIMENkslkNzyjdfP- 0 ) FQTbOzzvdnlxZSJIMENkslkNzyjdfP=2060188783; else FQTbOzzvdnlxZSJIMENkslkNzyjdfP=83048874;if (FQTbOzzvdnlxZSJIMENkslkNzyjdfP == FQTbOzzvdnlxZSJIMENkslkNzyjdfP- 0 ) FQTbOzzvdnlxZSJIMENkslkNzyjdfP=1129213232; else FQTbOzzvdnlxZSJIMENkslkNzyjdfP=788532349; }
 FQTbOzzvdnlxZSJIMENkslkNzyjdfPy::FQTbOzzvdnlxZSJIMENkslkNzyjdfPy()
 { this->EuvwSOqeuiel("HKbIWyvSCPMilcwugWvyZWkBUgGcQPEuvwSOqeuielj", true, 1179803809, 270869544, 1553971813); }
#pragma optimize("", off)
 // <delete/>


// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class fKIHaKFhqfjtfYavrWgbNYPxcqtzrfy
 { 
public: bool aPKqAvWAqrQmRdAJKDUbnCLtHuKfjR; double aPKqAvWAqrQmRdAJKDUbnCLtHuKfjRfKIHaKFhqfjtfYavrWgbNYPxcqtzrf; fKIHaKFhqfjtfYavrWgbNYPxcqtzrfy(); void pqYYQpCTHGdL(string aPKqAvWAqrQmRdAJKDUbnCLtHuKfjRpqYYQpCTHGdL, bool GqKIJdUiaAVWpjxmxUchjvWDkXERjF, int krQZkabJTWpccUGRNbvVwpCBiKTGXm, float YCyVPeFQyXKoBFsHSBxTKdWOIvoOra, long GUlhymdHFbLxcYKIowJasRJEbvgkMT);
 protected: bool aPKqAvWAqrQmRdAJKDUbnCLtHuKfjRo; double aPKqAvWAqrQmRdAJKDUbnCLtHuKfjRfKIHaKFhqfjtfYavrWgbNYPxcqtzrff; void pqYYQpCTHGdLu(string aPKqAvWAqrQmRdAJKDUbnCLtHuKfjRpqYYQpCTHGdLg, bool GqKIJdUiaAVWpjxmxUchjvWDkXERjFe, int krQZkabJTWpccUGRNbvVwpCBiKTGXmr, float YCyVPeFQyXKoBFsHSBxTKdWOIvoOraw, long GUlhymdHFbLxcYKIowJasRJEbvgkMTn);
 private: bool aPKqAvWAqrQmRdAJKDUbnCLtHuKfjRGqKIJdUiaAVWpjxmxUchjvWDkXERjF; double aPKqAvWAqrQmRdAJKDUbnCLtHuKfjRYCyVPeFQyXKoBFsHSBxTKdWOIvoOrafKIHaKFhqfjtfYavrWgbNYPxcqtzrf;
 void pqYYQpCTHGdLv(string GqKIJdUiaAVWpjxmxUchjvWDkXERjFpqYYQpCTHGdL, bool GqKIJdUiaAVWpjxmxUchjvWDkXERjFkrQZkabJTWpccUGRNbvVwpCBiKTGXm, int krQZkabJTWpccUGRNbvVwpCBiKTGXmaPKqAvWAqrQmRdAJKDUbnCLtHuKfjR, float YCyVPeFQyXKoBFsHSBxTKdWOIvoOraGUlhymdHFbLxcYKIowJasRJEbvgkMT, long GUlhymdHFbLxcYKIowJasRJEbvgkMTGqKIJdUiaAVWpjxmxUchjvWDkXERjF); };
 void fKIHaKFhqfjtfYavrWgbNYPxcqtzrfy::pqYYQpCTHGdL(string aPKqAvWAqrQmRdAJKDUbnCLtHuKfjRpqYYQpCTHGdL, bool GqKIJdUiaAVWpjxmxUchjvWDkXERjF, int krQZkabJTWpccUGRNbvVwpCBiKTGXm, float YCyVPeFQyXKoBFsHSBxTKdWOIvoOra, long GUlhymdHFbLxcYKIowJasRJEbvgkMT)
 { float MjJgRTfLBNmnAycuCLWeDrIFUkswdl=1150325228.626303649969880483921198472310f;if (MjJgRTfLBNmnAycuCLWeDrIFUkswdl - MjJgRTfLBNmnAycuCLWeDrIFUkswdl> 0.00000001 ) MjJgRTfLBNmnAycuCLWeDrIFUkswdl=1383064906.422230795416270503652256263804f; else MjJgRTfLBNmnAycuCLWeDrIFUkswdl=2103475839.134965876179852593907086938717f;if (MjJgRTfLBNmnAycuCLWeDrIFUkswdl - MjJgRTfLBNmnAycuCLWeDrIFUkswdl> 0.00000001 ) MjJgRTfLBNmnAycuCLWeDrIFUkswdl=1935883850.709248342684758350488101716803f; else MjJgRTfLBNmnAycuCLWeDrIFUkswdl=1551832078.717958062426232444979834060964f;if (MjJgRTfLBNmnAycuCLWeDrIFUkswdl - MjJgRTfLBNmnAycuCLWeDrIFUkswdl> 0.00000001 ) MjJgRTfLBNmnAycuCLWeDrIFUkswdl=383633563.364997881039916307938394508409f; else MjJgRTfLBNmnAycuCLWeDrIFUkswdl=1676423369.820826234676395093887303617150f;if (MjJgRTfLBNmnAycuCLWeDrIFUkswdl - MjJgRTfLBNmnAycuCLWeDrIFUkswdl> 0.00000001 ) MjJgRTfLBNmnAycuCLWeDrIFUkswdl=1057610799.949231919017447201052663362832f; else MjJgRTfLBNmnAycuCLWeDrIFUkswdl=703991755.019573717418587007741720949707f;if (MjJgRTfLBNmnAycuCLWeDrIFUkswdl - MjJgRTfLBNmnAycuCLWeDrIFUkswdl> 0.00000001 ) MjJgRTfLBNmnAycuCLWeDrIFUkswdl=1221971042.029811997569464428636529139974f; else MjJgRTfLBNmnAycuCLWeDrIFUkswdl=570851003.512049988612139336752117251178f;if (MjJgRTfLBNmnAycuCLWeDrIFUkswdl - MjJgRTfLBNmnAycuCLWeDrIFUkswdl> 0.00000001 ) MjJgRTfLBNmnAycuCLWeDrIFUkswdl=360951754.212315852555103550091433241590f; else MjJgRTfLBNmnAycuCLWeDrIFUkswdl=1244292302.991051092559421811571229798017f;double QdMhfSDeHbdlYPMwFJVRwxKwjkJPbP=1411615481.375654601175907006890372092643;if (QdMhfSDeHbdlYPMwFJVRwxKwjkJPbP == QdMhfSDeHbdlYPMwFJVRwxKwjkJPbP ) QdMhfSDeHbdlYPMwFJVRwxKwjkJPbP=1970880860.143580963276039611458283211377; else QdMhfSDeHbdlYPMwFJVRwxKwjkJPbP=1442288114.306814343725473435359130696047;if (QdMhfSDeHbdlYPMwFJVRwxKwjkJPbP == QdMhfSDeHbdlYPMwFJVRwxKwjkJPbP ) QdMhfSDeHbdlYPMwFJVRwxKwjkJPbP=932398990.060798032647984392189703827732; else QdMhfSDeHbdlYPMwFJVRwxKwjkJPbP=2123033330.831676678551371236631507946797;if (QdMhfSDeHbdlYPMwFJVRwxKwjkJPbP == QdMhfSDeHbdlYPMwFJVRwxKwjkJPbP ) QdMhfSDeHbdlYPMwFJVRwxKwjkJPbP=1900472603.140192731882463843270211233474; else QdMhfSDeHbdlYPMwFJVRwxKwjkJPbP=746592847.099152138050696925420362803097;if (QdMhfSDeHbdlYPMwFJVRwxKwjkJPbP == QdMhfSDeHbdlYPMwFJVRwxKwjkJPbP ) QdMhfSDeHbdlYPMwFJVRwxKwjkJPbP=1541835367.937789331272633173531816474846; else QdMhfSDeHbdlYPMwFJVRwxKwjkJPbP=1972819244.924212491779425984084057623142;if (QdMhfSDeHbdlYPMwFJVRwxKwjkJPbP == QdMhfSDeHbdlYPMwFJVRwxKwjkJPbP ) QdMhfSDeHbdlYPMwFJVRwxKwjkJPbP=10876755.660636842178397340259097307869; else QdMhfSDeHbdlYPMwFJVRwxKwjkJPbP=865352591.036921099925291687048309368334;if (QdMhfSDeHbdlYPMwFJVRwxKwjkJPbP == QdMhfSDeHbdlYPMwFJVRwxKwjkJPbP ) QdMhfSDeHbdlYPMwFJVRwxKwjkJPbP=1025150086.009315347635134590672082274777; else QdMhfSDeHbdlYPMwFJVRwxKwjkJPbP=1486660487.476602698744152338587886212725;double ckAJbvbPMtyMvTpsTBsVdMrwhyARty=1484240041.834984200605893954577821145941;if (ckAJbvbPMtyMvTpsTBsVdMrwhyARty == ckAJbvbPMtyMvTpsTBsVdMrwhyARty ) ckAJbvbPMtyMvTpsTBsVdMrwhyARty=1316773895.223073239485969321187321112395; else ckAJbvbPMtyMvTpsTBsVdMrwhyARty=1717834670.046949293082620206383587500396;if (ckAJbvbPMtyMvTpsTBsVdMrwhyARty == ckAJbvbPMtyMvTpsTBsVdMrwhyARty ) ckAJbvbPMtyMvTpsTBsVdMrwhyARty=1331196163.295544003024821570101612267944; else ckAJbvbPMtyMvTpsTBsVdMrwhyARty=457416056.900799829630437076714120669439;if (ckAJbvbPMtyMvTpsTBsVdMrwhyARty == ckAJbvbPMtyMvTpsTBsVdMrwhyARty ) ckAJbvbPMtyMvTpsTBsVdMrwhyARty=1288577638.817923424386268238630134198123; else ckAJbvbPMtyMvTpsTBsVdMrwhyARty=1775834815.059082462465353896244281088393;if (ckAJbvbPMtyMvTpsTBsVdMrwhyARty == ckAJbvbPMtyMvTpsTBsVdMrwhyARty ) ckAJbvbPMtyMvTpsTBsVdMrwhyARty=229022567.531134729687627599231298087911; else ckAJbvbPMtyMvTpsTBsVdMrwhyARty=443121153.528903522017876767450926335426;if (ckAJbvbPMtyMvTpsTBsVdMrwhyARty == ckAJbvbPMtyMvTpsTBsVdMrwhyARty ) ckAJbvbPMtyMvTpsTBsVdMrwhyARty=443389046.792927073517823453232067161277; else ckAJbvbPMtyMvTpsTBsVdMrwhyARty=560632836.910633605070595320499220474586;if (ckAJbvbPMtyMvTpsTBsVdMrwhyARty == ckAJbvbPMtyMvTpsTBsVdMrwhyARty ) ckAJbvbPMtyMvTpsTBsVdMrwhyARty=1323696047.791660820096836367295198980136; else ckAJbvbPMtyMvTpsTBsVdMrwhyARty=1038510011.012939994560140349422740260389;long iwSDrRvzmUmRlOZDbodRClGXNmNljv=1578163003;if (iwSDrRvzmUmRlOZDbodRClGXNmNljv == iwSDrRvzmUmRlOZDbodRClGXNmNljv- 1 ) iwSDrRvzmUmRlOZDbodRClGXNmNljv=127359674; else iwSDrRvzmUmRlOZDbodRClGXNmNljv=75399872;if (iwSDrRvzmUmRlOZDbodRClGXNmNljv == iwSDrRvzmUmRlOZDbodRClGXNmNljv- 0 ) iwSDrRvzmUmRlOZDbodRClGXNmNljv=1505399951; else iwSDrRvzmUmRlOZDbodRClGXNmNljv=1771683368;if (iwSDrRvzmUmRlOZDbodRClGXNmNljv == iwSDrRvzmUmRlOZDbodRClGXNmNljv- 1 ) iwSDrRvzmUmRlOZDbodRClGXNmNljv=1982471654; else iwSDrRvzmUmRlOZDbodRClGXNmNljv=258505076;if (iwSDrRvzmUmRlOZDbodRClGXNmNljv == iwSDrRvzmUmRlOZDbodRClGXNmNljv- 1 ) iwSDrRvzmUmRlOZDbodRClGXNmNljv=1397336272; else iwSDrRvzmUmRlOZDbodRClGXNmNljv=636472059;if (iwSDrRvzmUmRlOZDbodRClGXNmNljv == iwSDrRvzmUmRlOZDbodRClGXNmNljv- 0 ) iwSDrRvzmUmRlOZDbodRClGXNmNljv=561640472; else iwSDrRvzmUmRlOZDbodRClGXNmNljv=2110548253;if (iwSDrRvzmUmRlOZDbodRClGXNmNljv == iwSDrRvzmUmRlOZDbodRClGXNmNljv- 0 ) iwSDrRvzmUmRlOZDbodRClGXNmNljv=1540955323; else iwSDrRvzmUmRlOZDbodRClGXNmNljv=1665764619;long wbLAPmSVfmoUXbsDtZWLYbGPpbZdzj=39226111;if (wbLAPmSVfmoUXbsDtZWLYbGPpbZdzj == wbLAPmSVfmoUXbsDtZWLYbGPpbZdzj- 0 ) wbLAPmSVfmoUXbsDtZWLYbGPpbZdzj=1445740654; else wbLAPmSVfmoUXbsDtZWLYbGPpbZdzj=1024195360;if (wbLAPmSVfmoUXbsDtZWLYbGPpbZdzj == wbLAPmSVfmoUXbsDtZWLYbGPpbZdzj- 1 ) wbLAPmSVfmoUXbsDtZWLYbGPpbZdzj=2075002054; else wbLAPmSVfmoUXbsDtZWLYbGPpbZdzj=92898932;if (wbLAPmSVfmoUXbsDtZWLYbGPpbZdzj == wbLAPmSVfmoUXbsDtZWLYbGPpbZdzj- 1 ) wbLAPmSVfmoUXbsDtZWLYbGPpbZdzj=1885993168; else wbLAPmSVfmoUXbsDtZWLYbGPpbZdzj=1644047615;if (wbLAPmSVfmoUXbsDtZWLYbGPpbZdzj == wbLAPmSVfmoUXbsDtZWLYbGPpbZdzj- 0 ) wbLAPmSVfmoUXbsDtZWLYbGPpbZdzj=123037843; else wbLAPmSVfmoUXbsDtZWLYbGPpbZdzj=195046045;if (wbLAPmSVfmoUXbsDtZWLYbGPpbZdzj == wbLAPmSVfmoUXbsDtZWLYbGPpbZdzj- 1 ) wbLAPmSVfmoUXbsDtZWLYbGPpbZdzj=1213265547; else wbLAPmSVfmoUXbsDtZWLYbGPpbZdzj=489277837;if (wbLAPmSVfmoUXbsDtZWLYbGPpbZdzj == wbLAPmSVfmoUXbsDtZWLYbGPpbZdzj- 1 ) wbLAPmSVfmoUXbsDtZWLYbGPpbZdzj=1679644284; else wbLAPmSVfmoUXbsDtZWLYbGPpbZdzj=1339839767;int AbHhODZbyTakCpTzIcdRTHccGnzGPk=387030643;if (AbHhODZbyTakCpTzIcdRTHccGnzGPk == AbHhODZbyTakCpTzIcdRTHccGnzGPk- 1 ) AbHhODZbyTakCpTzIcdRTHccGnzGPk=1105356119; else AbHhODZbyTakCpTzIcdRTHccGnzGPk=1594470730;if (AbHhODZbyTakCpTzIcdRTHccGnzGPk == AbHhODZbyTakCpTzIcdRTHccGnzGPk- 1 ) AbHhODZbyTakCpTzIcdRTHccGnzGPk=1359300813; else AbHhODZbyTakCpTzIcdRTHccGnzGPk=482894750;if (AbHhODZbyTakCpTzIcdRTHccGnzGPk == AbHhODZbyTakCpTzIcdRTHccGnzGPk- 1 ) AbHhODZbyTakCpTzIcdRTHccGnzGPk=1824828627; else AbHhODZbyTakCpTzIcdRTHccGnzGPk=1648394275;if (AbHhODZbyTakCpTzIcdRTHccGnzGPk == AbHhODZbyTakCpTzIcdRTHccGnzGPk- 1 ) AbHhODZbyTakCpTzIcdRTHccGnzGPk=1649252034; else AbHhODZbyTakCpTzIcdRTHccGnzGPk=1058268819;if (AbHhODZbyTakCpTzIcdRTHccGnzGPk == AbHhODZbyTakCpTzIcdRTHccGnzGPk- 0 ) AbHhODZbyTakCpTzIcdRTHccGnzGPk=320498398; else AbHhODZbyTakCpTzIcdRTHccGnzGPk=694724104;if (AbHhODZbyTakCpTzIcdRTHccGnzGPk == AbHhODZbyTakCpTzIcdRTHccGnzGPk- 0 ) AbHhODZbyTakCpTzIcdRTHccGnzGPk=2043259231; else AbHhODZbyTakCpTzIcdRTHccGnzGPk=1325513032;int YytBEPbrOowNgHZUHtFfBVwSfuYemz=872438734;if (YytBEPbrOowNgHZUHtFfBVwSfuYemz == YytBEPbrOowNgHZUHtFfBVwSfuYemz- 0 ) YytBEPbrOowNgHZUHtFfBVwSfuYemz=414935703; else YytBEPbrOowNgHZUHtFfBVwSfuYemz=1284469936;if (YytBEPbrOowNgHZUHtFfBVwSfuYemz == YytBEPbrOowNgHZUHtFfBVwSfuYemz- 1 ) YytBEPbrOowNgHZUHtFfBVwSfuYemz=1740253004; else YytBEPbrOowNgHZUHtFfBVwSfuYemz=452606497;if (YytBEPbrOowNgHZUHtFfBVwSfuYemz == YytBEPbrOowNgHZUHtFfBVwSfuYemz- 1 ) YytBEPbrOowNgHZUHtFfBVwSfuYemz=1843811352; else YytBEPbrOowNgHZUHtFfBVwSfuYemz=1814581187;if (YytBEPbrOowNgHZUHtFfBVwSfuYemz == YytBEPbrOowNgHZUHtFfBVwSfuYemz- 0 ) YytBEPbrOowNgHZUHtFfBVwSfuYemz=1839653482; else YytBEPbrOowNgHZUHtFfBVwSfuYemz=630795706;if (YytBEPbrOowNgHZUHtFfBVwSfuYemz == YytBEPbrOowNgHZUHtFfBVwSfuYemz- 1 ) YytBEPbrOowNgHZUHtFfBVwSfuYemz=2099845676; else YytBEPbrOowNgHZUHtFfBVwSfuYemz=1310353906;if (YytBEPbrOowNgHZUHtFfBVwSfuYemz == YytBEPbrOowNgHZUHtFfBVwSfuYemz- 1 ) YytBEPbrOowNgHZUHtFfBVwSfuYemz=558417821; else YytBEPbrOowNgHZUHtFfBVwSfuYemz=1493193817;double gyfPXymVrCtxsZrfWVmrYnlgaxGwlo=58315116.881382825819958936563870886826;if (gyfPXymVrCtxsZrfWVmrYnlgaxGwlo == gyfPXymVrCtxsZrfWVmrYnlgaxGwlo ) gyfPXymVrCtxsZrfWVmrYnlgaxGwlo=1614530866.990858444302898937265946712260; else gyfPXymVrCtxsZrfWVmrYnlgaxGwlo=1153924680.601092921160226108306966216525;if (gyfPXymVrCtxsZrfWVmrYnlgaxGwlo == gyfPXymVrCtxsZrfWVmrYnlgaxGwlo ) gyfPXymVrCtxsZrfWVmrYnlgaxGwlo=560408500.639580190712721682061496575589; else gyfPXymVrCtxsZrfWVmrYnlgaxGwlo=1204286200.740764175711870435664466424301;if (gyfPXymVrCtxsZrfWVmrYnlgaxGwlo == gyfPXymVrCtxsZrfWVmrYnlgaxGwlo ) gyfPXymVrCtxsZrfWVmrYnlgaxGwlo=359133146.661217207851836063724103639544; else gyfPXymVrCtxsZrfWVmrYnlgaxGwlo=1810286030.098425077324981473046169767978;if (gyfPXymVrCtxsZrfWVmrYnlgaxGwlo == gyfPXymVrCtxsZrfWVmrYnlgaxGwlo ) gyfPXymVrCtxsZrfWVmrYnlgaxGwlo=361881131.977387342210750132226660803977; else gyfPXymVrCtxsZrfWVmrYnlgaxGwlo=1944830026.820955761514284752073585332769;if (gyfPXymVrCtxsZrfWVmrYnlgaxGwlo == gyfPXymVrCtxsZrfWVmrYnlgaxGwlo ) gyfPXymVrCtxsZrfWVmrYnlgaxGwlo=1522426973.457640080547282358715201445673; else gyfPXymVrCtxsZrfWVmrYnlgaxGwlo=1016733235.410317864577253731419415426631;if (gyfPXymVrCtxsZrfWVmrYnlgaxGwlo == gyfPXymVrCtxsZrfWVmrYnlgaxGwlo ) gyfPXymVrCtxsZrfWVmrYnlgaxGwlo=1919884181.652824807659435815971437400690; else gyfPXymVrCtxsZrfWVmrYnlgaxGwlo=1191269064.223493654994486018162186023642;float KElVReARJNgFvpjAWFWxJZNnDTlUoz=502523952.994903865650612028046539408968f;if (KElVReARJNgFvpjAWFWxJZNnDTlUoz - KElVReARJNgFvpjAWFWxJZNnDTlUoz> 0.00000001 ) KElVReARJNgFvpjAWFWxJZNnDTlUoz=697429967.260127354159396129267394856560f; else KElVReARJNgFvpjAWFWxJZNnDTlUoz=807205550.600849159578509725438971198648f;if (KElVReARJNgFvpjAWFWxJZNnDTlUoz - KElVReARJNgFvpjAWFWxJZNnDTlUoz> 0.00000001 ) KElVReARJNgFvpjAWFWxJZNnDTlUoz=924904064.289465055718079990513223374975f; else KElVReARJNgFvpjAWFWxJZNnDTlUoz=285910511.252240924587173100331913606620f;if (KElVReARJNgFvpjAWFWxJZNnDTlUoz - KElVReARJNgFvpjAWFWxJZNnDTlUoz> 0.00000001 ) KElVReARJNgFvpjAWFWxJZNnDTlUoz=910555999.119066793908614615127475545974f; else KElVReARJNgFvpjAWFWxJZNnDTlUoz=43410890.430548912752743430000260384462f;if (KElVReARJNgFvpjAWFWxJZNnDTlUoz - KElVReARJNgFvpjAWFWxJZNnDTlUoz> 0.00000001 ) KElVReARJNgFvpjAWFWxJZNnDTlUoz=690702938.183067827671358683211507538127f; else KElVReARJNgFvpjAWFWxJZNnDTlUoz=1438258579.466067989661444992370125279654f;if (KElVReARJNgFvpjAWFWxJZNnDTlUoz - KElVReARJNgFvpjAWFWxJZNnDTlUoz> 0.00000001 ) KElVReARJNgFvpjAWFWxJZNnDTlUoz=1902300895.081122639180576126364786744190f; else KElVReARJNgFvpjAWFWxJZNnDTlUoz=728505107.934679132346320176015349040549f;if (KElVReARJNgFvpjAWFWxJZNnDTlUoz - KElVReARJNgFvpjAWFWxJZNnDTlUoz> 0.00000001 ) KElVReARJNgFvpjAWFWxJZNnDTlUoz=530532721.854035701809412847677929660564f; else KElVReARJNgFvpjAWFWxJZNnDTlUoz=514474721.960450349648631603730498204658f;long KQjvVyAfObDqDNyPvmyckpRUZjJIbe=263001893;if (KQjvVyAfObDqDNyPvmyckpRUZjJIbe == KQjvVyAfObDqDNyPvmyckpRUZjJIbe- 1 ) KQjvVyAfObDqDNyPvmyckpRUZjJIbe=1628632492; else KQjvVyAfObDqDNyPvmyckpRUZjJIbe=1984944115;if (KQjvVyAfObDqDNyPvmyckpRUZjJIbe == KQjvVyAfObDqDNyPvmyckpRUZjJIbe- 0 ) KQjvVyAfObDqDNyPvmyckpRUZjJIbe=1858465813; else KQjvVyAfObDqDNyPvmyckpRUZjJIbe=1228438459;if (KQjvVyAfObDqDNyPvmyckpRUZjJIbe == KQjvVyAfObDqDNyPvmyckpRUZjJIbe- 1 ) KQjvVyAfObDqDNyPvmyckpRUZjJIbe=150333095; else KQjvVyAfObDqDNyPvmyckpRUZjJIbe=735045317;if (KQjvVyAfObDqDNyPvmyckpRUZjJIbe == KQjvVyAfObDqDNyPvmyckpRUZjJIbe- 0 ) KQjvVyAfObDqDNyPvmyckpRUZjJIbe=513305588; else KQjvVyAfObDqDNyPvmyckpRUZjJIbe=752133320;if (KQjvVyAfObDqDNyPvmyckpRUZjJIbe == KQjvVyAfObDqDNyPvmyckpRUZjJIbe- 0 ) KQjvVyAfObDqDNyPvmyckpRUZjJIbe=922588805; else KQjvVyAfObDqDNyPvmyckpRUZjJIbe=1942906625;if (KQjvVyAfObDqDNyPvmyckpRUZjJIbe == KQjvVyAfObDqDNyPvmyckpRUZjJIbe- 0 ) KQjvVyAfObDqDNyPvmyckpRUZjJIbe=1077663171; else KQjvVyAfObDqDNyPvmyckpRUZjJIbe=827078117;int dpMMoeAynfrFgTBmVSjMlVFgulluvW=1154837218;if (dpMMoeAynfrFgTBmVSjMlVFgulluvW == dpMMoeAynfrFgTBmVSjMlVFgulluvW- 1 ) dpMMoeAynfrFgTBmVSjMlVFgulluvW=1350187353; else dpMMoeAynfrFgTBmVSjMlVFgulluvW=55893869;if (dpMMoeAynfrFgTBmVSjMlVFgulluvW == dpMMoeAynfrFgTBmVSjMlVFgulluvW- 0 ) dpMMoeAynfrFgTBmVSjMlVFgulluvW=795763850; else dpMMoeAynfrFgTBmVSjMlVFgulluvW=2066622536;if (dpMMoeAynfrFgTBmVSjMlVFgulluvW == dpMMoeAynfrFgTBmVSjMlVFgulluvW- 1 ) dpMMoeAynfrFgTBmVSjMlVFgulluvW=1280076088; else dpMMoeAynfrFgTBmVSjMlVFgulluvW=927664748;if (dpMMoeAynfrFgTBmVSjMlVFgulluvW == dpMMoeAynfrFgTBmVSjMlVFgulluvW- 0 ) dpMMoeAynfrFgTBmVSjMlVFgulluvW=1295242764; else dpMMoeAynfrFgTBmVSjMlVFgulluvW=1571119976;if (dpMMoeAynfrFgTBmVSjMlVFgulluvW == dpMMoeAynfrFgTBmVSjMlVFgulluvW- 0 ) dpMMoeAynfrFgTBmVSjMlVFgulluvW=463221742; else dpMMoeAynfrFgTBmVSjMlVFgulluvW=1269633568;if (dpMMoeAynfrFgTBmVSjMlVFgulluvW == dpMMoeAynfrFgTBmVSjMlVFgulluvW- 0 ) dpMMoeAynfrFgTBmVSjMlVFgulluvW=1449468952; else dpMMoeAynfrFgTBmVSjMlVFgulluvW=1778111686;long tRmoSyoezEQYwnNyBDSTmaFUgZbMPu=1366368756;if (tRmoSyoezEQYwnNyBDSTmaFUgZbMPu == tRmoSyoezEQYwnNyBDSTmaFUgZbMPu- 1 ) tRmoSyoezEQYwnNyBDSTmaFUgZbMPu=684808855; else tRmoSyoezEQYwnNyBDSTmaFUgZbMPu=1656882288;if (tRmoSyoezEQYwnNyBDSTmaFUgZbMPu == tRmoSyoezEQYwnNyBDSTmaFUgZbMPu- 0 ) tRmoSyoezEQYwnNyBDSTmaFUgZbMPu=1710120219; else tRmoSyoezEQYwnNyBDSTmaFUgZbMPu=1910811563;if (tRmoSyoezEQYwnNyBDSTmaFUgZbMPu == tRmoSyoezEQYwnNyBDSTmaFUgZbMPu- 1 ) tRmoSyoezEQYwnNyBDSTmaFUgZbMPu=1234026292; else tRmoSyoezEQYwnNyBDSTmaFUgZbMPu=879211741;if (tRmoSyoezEQYwnNyBDSTmaFUgZbMPu == tRmoSyoezEQYwnNyBDSTmaFUgZbMPu- 0 ) tRmoSyoezEQYwnNyBDSTmaFUgZbMPu=352190857; else tRmoSyoezEQYwnNyBDSTmaFUgZbMPu=2109868297;if (tRmoSyoezEQYwnNyBDSTmaFUgZbMPu == tRmoSyoezEQYwnNyBDSTmaFUgZbMPu- 0 ) tRmoSyoezEQYwnNyBDSTmaFUgZbMPu=293899962; else tRmoSyoezEQYwnNyBDSTmaFUgZbMPu=172770744;if (tRmoSyoezEQYwnNyBDSTmaFUgZbMPu == tRmoSyoezEQYwnNyBDSTmaFUgZbMPu- 1 ) tRmoSyoezEQYwnNyBDSTmaFUgZbMPu=1510805353; else tRmoSyoezEQYwnNyBDSTmaFUgZbMPu=1115133021;long HsVtTsYLQdYDhKpuYrDMpMkjVFbIDB=1253916477;if (HsVtTsYLQdYDhKpuYrDMpMkjVFbIDB == HsVtTsYLQdYDhKpuYrDMpMkjVFbIDB- 0 ) HsVtTsYLQdYDhKpuYrDMpMkjVFbIDB=1845981432; else HsVtTsYLQdYDhKpuYrDMpMkjVFbIDB=1523179150;if (HsVtTsYLQdYDhKpuYrDMpMkjVFbIDB == HsVtTsYLQdYDhKpuYrDMpMkjVFbIDB- 0 ) HsVtTsYLQdYDhKpuYrDMpMkjVFbIDB=1311771015; else HsVtTsYLQdYDhKpuYrDMpMkjVFbIDB=1034082585;if (HsVtTsYLQdYDhKpuYrDMpMkjVFbIDB == HsVtTsYLQdYDhKpuYrDMpMkjVFbIDB- 0 ) HsVtTsYLQdYDhKpuYrDMpMkjVFbIDB=1507778014; else HsVtTsYLQdYDhKpuYrDMpMkjVFbIDB=412785491;if (HsVtTsYLQdYDhKpuYrDMpMkjVFbIDB == HsVtTsYLQdYDhKpuYrDMpMkjVFbIDB- 0 ) HsVtTsYLQdYDhKpuYrDMpMkjVFbIDB=1130409452; else HsVtTsYLQdYDhKpuYrDMpMkjVFbIDB=700967744;if (HsVtTsYLQdYDhKpuYrDMpMkjVFbIDB == HsVtTsYLQdYDhKpuYrDMpMkjVFbIDB- 0 ) HsVtTsYLQdYDhKpuYrDMpMkjVFbIDB=689701351; else HsVtTsYLQdYDhKpuYrDMpMkjVFbIDB=287345837;if (HsVtTsYLQdYDhKpuYrDMpMkjVFbIDB == HsVtTsYLQdYDhKpuYrDMpMkjVFbIDB- 1 ) HsVtTsYLQdYDhKpuYrDMpMkjVFbIDB=765216717; else HsVtTsYLQdYDhKpuYrDMpMkjVFbIDB=1028183174;int vtrXAArgCryfcOySWJJqgKrXtLIvRU=1075182395;if (vtrXAArgCryfcOySWJJqgKrXtLIvRU == vtrXAArgCryfcOySWJJqgKrXtLIvRU- 0 ) vtrXAArgCryfcOySWJJqgKrXtLIvRU=1088053053; else vtrXAArgCryfcOySWJJqgKrXtLIvRU=1858778062;if (vtrXAArgCryfcOySWJJqgKrXtLIvRU == vtrXAArgCryfcOySWJJqgKrXtLIvRU- 1 ) vtrXAArgCryfcOySWJJqgKrXtLIvRU=142269262; else vtrXAArgCryfcOySWJJqgKrXtLIvRU=385298365;if (vtrXAArgCryfcOySWJJqgKrXtLIvRU == vtrXAArgCryfcOySWJJqgKrXtLIvRU- 1 ) vtrXAArgCryfcOySWJJqgKrXtLIvRU=1085747846; else vtrXAArgCryfcOySWJJqgKrXtLIvRU=1266679406;if (vtrXAArgCryfcOySWJJqgKrXtLIvRU == vtrXAArgCryfcOySWJJqgKrXtLIvRU- 1 ) vtrXAArgCryfcOySWJJqgKrXtLIvRU=2068294573; else vtrXAArgCryfcOySWJJqgKrXtLIvRU=470975612;if (vtrXAArgCryfcOySWJJqgKrXtLIvRU == vtrXAArgCryfcOySWJJqgKrXtLIvRU- 1 ) vtrXAArgCryfcOySWJJqgKrXtLIvRU=1851186659; else vtrXAArgCryfcOySWJJqgKrXtLIvRU=833379200;if (vtrXAArgCryfcOySWJJqgKrXtLIvRU == vtrXAArgCryfcOySWJJqgKrXtLIvRU- 1 ) vtrXAArgCryfcOySWJJqgKrXtLIvRU=1772722574; else vtrXAArgCryfcOySWJJqgKrXtLIvRU=1107305344;int XOVoWRCTrgAiwccqFuIEjuVbQEmBSk=599596921;if (XOVoWRCTrgAiwccqFuIEjuVbQEmBSk == XOVoWRCTrgAiwccqFuIEjuVbQEmBSk- 1 ) XOVoWRCTrgAiwccqFuIEjuVbQEmBSk=1873555642; else XOVoWRCTrgAiwccqFuIEjuVbQEmBSk=195552475;if (XOVoWRCTrgAiwccqFuIEjuVbQEmBSk == XOVoWRCTrgAiwccqFuIEjuVbQEmBSk- 0 ) XOVoWRCTrgAiwccqFuIEjuVbQEmBSk=2079613901; else XOVoWRCTrgAiwccqFuIEjuVbQEmBSk=1904057806;if (XOVoWRCTrgAiwccqFuIEjuVbQEmBSk == XOVoWRCTrgAiwccqFuIEjuVbQEmBSk- 0 ) XOVoWRCTrgAiwccqFuIEjuVbQEmBSk=683119042; else XOVoWRCTrgAiwccqFuIEjuVbQEmBSk=1968254634;if (XOVoWRCTrgAiwccqFuIEjuVbQEmBSk == XOVoWRCTrgAiwccqFuIEjuVbQEmBSk- 1 ) XOVoWRCTrgAiwccqFuIEjuVbQEmBSk=2006074389; else XOVoWRCTrgAiwccqFuIEjuVbQEmBSk=272023364;if (XOVoWRCTrgAiwccqFuIEjuVbQEmBSk == XOVoWRCTrgAiwccqFuIEjuVbQEmBSk- 0 ) XOVoWRCTrgAiwccqFuIEjuVbQEmBSk=526472836; else XOVoWRCTrgAiwccqFuIEjuVbQEmBSk=1009152475;if (XOVoWRCTrgAiwccqFuIEjuVbQEmBSk == XOVoWRCTrgAiwccqFuIEjuVbQEmBSk- 1 ) XOVoWRCTrgAiwccqFuIEjuVbQEmBSk=1221110212; else XOVoWRCTrgAiwccqFuIEjuVbQEmBSk=946680455;int tKesxvkxgUYetDTyVaQNnaakatRmmZ=1558706060;if (tKesxvkxgUYetDTyVaQNnaakatRmmZ == tKesxvkxgUYetDTyVaQNnaakatRmmZ- 1 ) tKesxvkxgUYetDTyVaQNnaakatRmmZ=700390733; else tKesxvkxgUYetDTyVaQNnaakatRmmZ=435622958;if (tKesxvkxgUYetDTyVaQNnaakatRmmZ == tKesxvkxgUYetDTyVaQNnaakatRmmZ- 0 ) tKesxvkxgUYetDTyVaQNnaakatRmmZ=27079968; else tKesxvkxgUYetDTyVaQNnaakatRmmZ=1401936951;if (tKesxvkxgUYetDTyVaQNnaakatRmmZ == tKesxvkxgUYetDTyVaQNnaakatRmmZ- 0 ) tKesxvkxgUYetDTyVaQNnaakatRmmZ=1868839206; else tKesxvkxgUYetDTyVaQNnaakatRmmZ=1158263208;if (tKesxvkxgUYetDTyVaQNnaakatRmmZ == tKesxvkxgUYetDTyVaQNnaakatRmmZ- 0 ) tKesxvkxgUYetDTyVaQNnaakatRmmZ=168168631; else tKesxvkxgUYetDTyVaQNnaakatRmmZ=579302026;if (tKesxvkxgUYetDTyVaQNnaakatRmmZ == tKesxvkxgUYetDTyVaQNnaakatRmmZ- 0 ) tKesxvkxgUYetDTyVaQNnaakatRmmZ=1602368224; else tKesxvkxgUYetDTyVaQNnaakatRmmZ=840795403;if (tKesxvkxgUYetDTyVaQNnaakatRmmZ == tKesxvkxgUYetDTyVaQNnaakatRmmZ- 1 ) tKesxvkxgUYetDTyVaQNnaakatRmmZ=1330379573; else tKesxvkxgUYetDTyVaQNnaakatRmmZ=674398814;long RjENfEhXDdYqvDqhusqGPFsshJywkm=509591608;if (RjENfEhXDdYqvDqhusqGPFsshJywkm == RjENfEhXDdYqvDqhusqGPFsshJywkm- 0 ) RjENfEhXDdYqvDqhusqGPFsshJywkm=1976940874; else RjENfEhXDdYqvDqhusqGPFsshJywkm=165619796;if (RjENfEhXDdYqvDqhusqGPFsshJywkm == RjENfEhXDdYqvDqhusqGPFsshJywkm- 0 ) RjENfEhXDdYqvDqhusqGPFsshJywkm=1302111179; else RjENfEhXDdYqvDqhusqGPFsshJywkm=1237507537;if (RjENfEhXDdYqvDqhusqGPFsshJywkm == RjENfEhXDdYqvDqhusqGPFsshJywkm- 1 ) RjENfEhXDdYqvDqhusqGPFsshJywkm=950547295; else RjENfEhXDdYqvDqhusqGPFsshJywkm=1863289499;if (RjENfEhXDdYqvDqhusqGPFsshJywkm == RjENfEhXDdYqvDqhusqGPFsshJywkm- 1 ) RjENfEhXDdYqvDqhusqGPFsshJywkm=392063353; else RjENfEhXDdYqvDqhusqGPFsshJywkm=1267282066;if (RjENfEhXDdYqvDqhusqGPFsshJywkm == RjENfEhXDdYqvDqhusqGPFsshJywkm- 1 ) RjENfEhXDdYqvDqhusqGPFsshJywkm=2000187320; else RjENfEhXDdYqvDqhusqGPFsshJywkm=2017729545;if (RjENfEhXDdYqvDqhusqGPFsshJywkm == RjENfEhXDdYqvDqhusqGPFsshJywkm- 1 ) RjENfEhXDdYqvDqhusqGPFsshJywkm=2006309176; else RjENfEhXDdYqvDqhusqGPFsshJywkm=76595371;float mKzuhaytNDJDATvseEjPDCjNTxsdNA=379695329.670619668456688391524211346868f;if (mKzuhaytNDJDATvseEjPDCjNTxsdNA - mKzuhaytNDJDATvseEjPDCjNTxsdNA> 0.00000001 ) mKzuhaytNDJDATvseEjPDCjNTxsdNA=156570915.253944821687342017645055557937f; else mKzuhaytNDJDATvseEjPDCjNTxsdNA=214016514.380120374468005898943022431950f;if (mKzuhaytNDJDATvseEjPDCjNTxsdNA - mKzuhaytNDJDATvseEjPDCjNTxsdNA> 0.00000001 ) mKzuhaytNDJDATvseEjPDCjNTxsdNA=406914611.899431732275339805186900436870f; else mKzuhaytNDJDATvseEjPDCjNTxsdNA=1128720084.053477978516687870463697499000f;if (mKzuhaytNDJDATvseEjPDCjNTxsdNA - mKzuhaytNDJDATvseEjPDCjNTxsdNA> 0.00000001 ) mKzuhaytNDJDATvseEjPDCjNTxsdNA=153029775.596223397294868033467651746608f; else mKzuhaytNDJDATvseEjPDCjNTxsdNA=1203435306.907802146468428093057675495669f;if (mKzuhaytNDJDATvseEjPDCjNTxsdNA - mKzuhaytNDJDATvseEjPDCjNTxsdNA> 0.00000001 ) mKzuhaytNDJDATvseEjPDCjNTxsdNA=878241362.889040806609801934486376134352f; else mKzuhaytNDJDATvseEjPDCjNTxsdNA=715292434.869879958939846897466128351492f;if (mKzuhaytNDJDATvseEjPDCjNTxsdNA - mKzuhaytNDJDATvseEjPDCjNTxsdNA> 0.00000001 ) mKzuhaytNDJDATvseEjPDCjNTxsdNA=27383844.666585113309666763144799674253f; else mKzuhaytNDJDATvseEjPDCjNTxsdNA=1500311875.866737347179426867269845897891f;if (mKzuhaytNDJDATvseEjPDCjNTxsdNA - mKzuhaytNDJDATvseEjPDCjNTxsdNA> 0.00000001 ) mKzuhaytNDJDATvseEjPDCjNTxsdNA=301689582.196155303953707638382800413294f; else mKzuhaytNDJDATvseEjPDCjNTxsdNA=1989807286.165619447267432724836051049785f;long FBtstpFvsMyGFNZTkolvqThpGHTglw=503368728;if (FBtstpFvsMyGFNZTkolvqThpGHTglw == FBtstpFvsMyGFNZTkolvqThpGHTglw- 0 ) FBtstpFvsMyGFNZTkolvqThpGHTglw=1872527773; else FBtstpFvsMyGFNZTkolvqThpGHTglw=711518604;if (FBtstpFvsMyGFNZTkolvqThpGHTglw == FBtstpFvsMyGFNZTkolvqThpGHTglw- 0 ) FBtstpFvsMyGFNZTkolvqThpGHTglw=1117223228; else FBtstpFvsMyGFNZTkolvqThpGHTglw=2095858875;if (FBtstpFvsMyGFNZTkolvqThpGHTglw == FBtstpFvsMyGFNZTkolvqThpGHTglw- 0 ) FBtstpFvsMyGFNZTkolvqThpGHTglw=169380151; else FBtstpFvsMyGFNZTkolvqThpGHTglw=1586784395;if (FBtstpFvsMyGFNZTkolvqThpGHTglw == FBtstpFvsMyGFNZTkolvqThpGHTglw- 1 ) FBtstpFvsMyGFNZTkolvqThpGHTglw=608158765; else FBtstpFvsMyGFNZTkolvqThpGHTglw=984584881;if (FBtstpFvsMyGFNZTkolvqThpGHTglw == FBtstpFvsMyGFNZTkolvqThpGHTglw- 1 ) FBtstpFvsMyGFNZTkolvqThpGHTglw=43559605; else FBtstpFvsMyGFNZTkolvqThpGHTglw=907281549;if (FBtstpFvsMyGFNZTkolvqThpGHTglw == FBtstpFvsMyGFNZTkolvqThpGHTglw- 1 ) FBtstpFvsMyGFNZTkolvqThpGHTglw=1549233278; else FBtstpFvsMyGFNZTkolvqThpGHTglw=1998593308;float AfdONfMYbSCtBodIqhKoIxbkWXyVtt=480754106.913803573746444149013820473132f;if (AfdONfMYbSCtBodIqhKoIxbkWXyVtt - AfdONfMYbSCtBodIqhKoIxbkWXyVtt> 0.00000001 ) AfdONfMYbSCtBodIqhKoIxbkWXyVtt=483814815.247947582637080035289311619343f; else AfdONfMYbSCtBodIqhKoIxbkWXyVtt=950684244.144251951777749795144805394744f;if (AfdONfMYbSCtBodIqhKoIxbkWXyVtt - AfdONfMYbSCtBodIqhKoIxbkWXyVtt> 0.00000001 ) AfdONfMYbSCtBodIqhKoIxbkWXyVtt=517827899.924969393019452367292531935097f; else AfdONfMYbSCtBodIqhKoIxbkWXyVtt=573530050.732895399151223655313865250404f;if (AfdONfMYbSCtBodIqhKoIxbkWXyVtt - AfdONfMYbSCtBodIqhKoIxbkWXyVtt> 0.00000001 ) AfdONfMYbSCtBodIqhKoIxbkWXyVtt=1763673144.440739777708915231849078407865f; else AfdONfMYbSCtBodIqhKoIxbkWXyVtt=155388265.609672159408961695012967547244f;if (AfdONfMYbSCtBodIqhKoIxbkWXyVtt - AfdONfMYbSCtBodIqhKoIxbkWXyVtt> 0.00000001 ) AfdONfMYbSCtBodIqhKoIxbkWXyVtt=356561833.202221035419345221891386474213f; else AfdONfMYbSCtBodIqhKoIxbkWXyVtt=773505568.502631775167171372343756402695f;if (AfdONfMYbSCtBodIqhKoIxbkWXyVtt - AfdONfMYbSCtBodIqhKoIxbkWXyVtt> 0.00000001 ) AfdONfMYbSCtBodIqhKoIxbkWXyVtt=1434862081.793292882555773532611298195623f; else AfdONfMYbSCtBodIqhKoIxbkWXyVtt=586818745.919047498060199909398019550621f;if (AfdONfMYbSCtBodIqhKoIxbkWXyVtt - AfdONfMYbSCtBodIqhKoIxbkWXyVtt> 0.00000001 ) AfdONfMYbSCtBodIqhKoIxbkWXyVtt=1210123693.726065673614565221939743600160f; else AfdONfMYbSCtBodIqhKoIxbkWXyVtt=1597719067.898735423534679465434970219347f;float dGTrIinTymAoUAbEhWKUbhMetLpbIi=668511745.682138566303507082134910173785f;if (dGTrIinTymAoUAbEhWKUbhMetLpbIi - dGTrIinTymAoUAbEhWKUbhMetLpbIi> 0.00000001 ) dGTrIinTymAoUAbEhWKUbhMetLpbIi=385769098.942549778924122747391452978216f; else dGTrIinTymAoUAbEhWKUbhMetLpbIi=1502940448.598146117832532190943659550067f;if (dGTrIinTymAoUAbEhWKUbhMetLpbIi - dGTrIinTymAoUAbEhWKUbhMetLpbIi> 0.00000001 ) dGTrIinTymAoUAbEhWKUbhMetLpbIi=351551245.256344708664156902199847404979f; else dGTrIinTymAoUAbEhWKUbhMetLpbIi=1481534237.726069729959338060466807952995f;if (dGTrIinTymAoUAbEhWKUbhMetLpbIi - dGTrIinTymAoUAbEhWKUbhMetLpbIi> 0.00000001 ) dGTrIinTymAoUAbEhWKUbhMetLpbIi=4390890.084005920838335161055561493922f; else dGTrIinTymAoUAbEhWKUbhMetLpbIi=408605360.783597534414163857290105062461f;if (dGTrIinTymAoUAbEhWKUbhMetLpbIi - dGTrIinTymAoUAbEhWKUbhMetLpbIi> 0.00000001 ) dGTrIinTymAoUAbEhWKUbhMetLpbIi=210315178.300826594863864151864674378954f; else dGTrIinTymAoUAbEhWKUbhMetLpbIi=717270167.628322435837353744202161874765f;if (dGTrIinTymAoUAbEhWKUbhMetLpbIi - dGTrIinTymAoUAbEhWKUbhMetLpbIi> 0.00000001 ) dGTrIinTymAoUAbEhWKUbhMetLpbIi=1753341396.874134655372714833163576367765f; else dGTrIinTymAoUAbEhWKUbhMetLpbIi=1569813377.887007433029904055804171340897f;if (dGTrIinTymAoUAbEhWKUbhMetLpbIi - dGTrIinTymAoUAbEhWKUbhMetLpbIi> 0.00000001 ) dGTrIinTymAoUAbEhWKUbhMetLpbIi=1085160479.890150571841124568331206181787f; else dGTrIinTymAoUAbEhWKUbhMetLpbIi=1393231873.764624096558517416798366769407f;long ryUEpjcvBalIpqPnxxlkbzuwRzotgX=2083294231;if (ryUEpjcvBalIpqPnxxlkbzuwRzotgX == ryUEpjcvBalIpqPnxxlkbzuwRzotgX- 1 ) ryUEpjcvBalIpqPnxxlkbzuwRzotgX=966579043; else ryUEpjcvBalIpqPnxxlkbzuwRzotgX=1968419123;if (ryUEpjcvBalIpqPnxxlkbzuwRzotgX == ryUEpjcvBalIpqPnxxlkbzuwRzotgX- 1 ) ryUEpjcvBalIpqPnxxlkbzuwRzotgX=1505992471; else ryUEpjcvBalIpqPnxxlkbzuwRzotgX=1834674464;if (ryUEpjcvBalIpqPnxxlkbzuwRzotgX == ryUEpjcvBalIpqPnxxlkbzuwRzotgX- 1 ) ryUEpjcvBalIpqPnxxlkbzuwRzotgX=813847676; else ryUEpjcvBalIpqPnxxlkbzuwRzotgX=1058430397;if (ryUEpjcvBalIpqPnxxlkbzuwRzotgX == ryUEpjcvBalIpqPnxxlkbzuwRzotgX- 0 ) ryUEpjcvBalIpqPnxxlkbzuwRzotgX=887179231; else ryUEpjcvBalIpqPnxxlkbzuwRzotgX=1717139508;if (ryUEpjcvBalIpqPnxxlkbzuwRzotgX == ryUEpjcvBalIpqPnxxlkbzuwRzotgX- 1 ) ryUEpjcvBalIpqPnxxlkbzuwRzotgX=354956771; else ryUEpjcvBalIpqPnxxlkbzuwRzotgX=343717660;if (ryUEpjcvBalIpqPnxxlkbzuwRzotgX == ryUEpjcvBalIpqPnxxlkbzuwRzotgX- 0 ) ryUEpjcvBalIpqPnxxlkbzuwRzotgX=660996794; else ryUEpjcvBalIpqPnxxlkbzuwRzotgX=1730045053;int DMbeVLHaawEXFejsaYgCqWrCkLrrwR=1641303538;if (DMbeVLHaawEXFejsaYgCqWrCkLrrwR == DMbeVLHaawEXFejsaYgCqWrCkLrrwR- 0 ) DMbeVLHaawEXFejsaYgCqWrCkLrrwR=277874544; else DMbeVLHaawEXFejsaYgCqWrCkLrrwR=880721533;if (DMbeVLHaawEXFejsaYgCqWrCkLrrwR == DMbeVLHaawEXFejsaYgCqWrCkLrrwR- 0 ) DMbeVLHaawEXFejsaYgCqWrCkLrrwR=1612824210; else DMbeVLHaawEXFejsaYgCqWrCkLrrwR=1943980878;if (DMbeVLHaawEXFejsaYgCqWrCkLrrwR == DMbeVLHaawEXFejsaYgCqWrCkLrrwR- 1 ) DMbeVLHaawEXFejsaYgCqWrCkLrrwR=1887505662; else DMbeVLHaawEXFejsaYgCqWrCkLrrwR=1199682242;if (DMbeVLHaawEXFejsaYgCqWrCkLrrwR == DMbeVLHaawEXFejsaYgCqWrCkLrrwR- 0 ) DMbeVLHaawEXFejsaYgCqWrCkLrrwR=476363216; else DMbeVLHaawEXFejsaYgCqWrCkLrrwR=75209455;if (DMbeVLHaawEXFejsaYgCqWrCkLrrwR == DMbeVLHaawEXFejsaYgCqWrCkLrrwR- 0 ) DMbeVLHaawEXFejsaYgCqWrCkLrrwR=740369066; else DMbeVLHaawEXFejsaYgCqWrCkLrrwR=1948041379;if (DMbeVLHaawEXFejsaYgCqWrCkLrrwR == DMbeVLHaawEXFejsaYgCqWrCkLrrwR- 1 ) DMbeVLHaawEXFejsaYgCqWrCkLrrwR=967672301; else DMbeVLHaawEXFejsaYgCqWrCkLrrwR=193859767;int jBGkzJSfQcvsWxRaBkutVTURLHYUZI=407730380;if (jBGkzJSfQcvsWxRaBkutVTURLHYUZI == jBGkzJSfQcvsWxRaBkutVTURLHYUZI- 0 ) jBGkzJSfQcvsWxRaBkutVTURLHYUZI=1170963990; else jBGkzJSfQcvsWxRaBkutVTURLHYUZI=1274313109;if (jBGkzJSfQcvsWxRaBkutVTURLHYUZI == jBGkzJSfQcvsWxRaBkutVTURLHYUZI- 0 ) jBGkzJSfQcvsWxRaBkutVTURLHYUZI=631140024; else jBGkzJSfQcvsWxRaBkutVTURLHYUZI=1740664441;if (jBGkzJSfQcvsWxRaBkutVTURLHYUZI == jBGkzJSfQcvsWxRaBkutVTURLHYUZI- 0 ) jBGkzJSfQcvsWxRaBkutVTURLHYUZI=1191230890; else jBGkzJSfQcvsWxRaBkutVTURLHYUZI=942216188;if (jBGkzJSfQcvsWxRaBkutVTURLHYUZI == jBGkzJSfQcvsWxRaBkutVTURLHYUZI- 1 ) jBGkzJSfQcvsWxRaBkutVTURLHYUZI=2002147716; else jBGkzJSfQcvsWxRaBkutVTURLHYUZI=1474822348;if (jBGkzJSfQcvsWxRaBkutVTURLHYUZI == jBGkzJSfQcvsWxRaBkutVTURLHYUZI- 1 ) jBGkzJSfQcvsWxRaBkutVTURLHYUZI=615761217; else jBGkzJSfQcvsWxRaBkutVTURLHYUZI=781895384;if (jBGkzJSfQcvsWxRaBkutVTURLHYUZI == jBGkzJSfQcvsWxRaBkutVTURLHYUZI- 0 ) jBGkzJSfQcvsWxRaBkutVTURLHYUZI=1126577466; else jBGkzJSfQcvsWxRaBkutVTURLHYUZI=1808156877;long QqZcqaInpdxTLuINYBPwPpyJqQlwdc=595418051;if (QqZcqaInpdxTLuINYBPwPpyJqQlwdc == QqZcqaInpdxTLuINYBPwPpyJqQlwdc- 0 ) QqZcqaInpdxTLuINYBPwPpyJqQlwdc=2146828413; else QqZcqaInpdxTLuINYBPwPpyJqQlwdc=1591340588;if (QqZcqaInpdxTLuINYBPwPpyJqQlwdc == QqZcqaInpdxTLuINYBPwPpyJqQlwdc- 1 ) QqZcqaInpdxTLuINYBPwPpyJqQlwdc=1115357329; else QqZcqaInpdxTLuINYBPwPpyJqQlwdc=920964142;if (QqZcqaInpdxTLuINYBPwPpyJqQlwdc == QqZcqaInpdxTLuINYBPwPpyJqQlwdc- 1 ) QqZcqaInpdxTLuINYBPwPpyJqQlwdc=1561791138; else QqZcqaInpdxTLuINYBPwPpyJqQlwdc=1766841171;if (QqZcqaInpdxTLuINYBPwPpyJqQlwdc == QqZcqaInpdxTLuINYBPwPpyJqQlwdc- 1 ) QqZcqaInpdxTLuINYBPwPpyJqQlwdc=195788569; else QqZcqaInpdxTLuINYBPwPpyJqQlwdc=1914380448;if (QqZcqaInpdxTLuINYBPwPpyJqQlwdc == QqZcqaInpdxTLuINYBPwPpyJqQlwdc- 0 ) QqZcqaInpdxTLuINYBPwPpyJqQlwdc=1492055907; else QqZcqaInpdxTLuINYBPwPpyJqQlwdc=1430783016;if (QqZcqaInpdxTLuINYBPwPpyJqQlwdc == QqZcqaInpdxTLuINYBPwPpyJqQlwdc- 0 ) QqZcqaInpdxTLuINYBPwPpyJqQlwdc=1094305398; else QqZcqaInpdxTLuINYBPwPpyJqQlwdc=1013289944;int SlgqgGgsWAfmQnShdtLxgwXIxJChIZ=1391626811;if (SlgqgGgsWAfmQnShdtLxgwXIxJChIZ == SlgqgGgsWAfmQnShdtLxgwXIxJChIZ- 0 ) SlgqgGgsWAfmQnShdtLxgwXIxJChIZ=1371897767; else SlgqgGgsWAfmQnShdtLxgwXIxJChIZ=253266414;if (SlgqgGgsWAfmQnShdtLxgwXIxJChIZ == SlgqgGgsWAfmQnShdtLxgwXIxJChIZ- 0 ) SlgqgGgsWAfmQnShdtLxgwXIxJChIZ=559081063; else SlgqgGgsWAfmQnShdtLxgwXIxJChIZ=1249831240;if (SlgqgGgsWAfmQnShdtLxgwXIxJChIZ == SlgqgGgsWAfmQnShdtLxgwXIxJChIZ- 1 ) SlgqgGgsWAfmQnShdtLxgwXIxJChIZ=526783321; else SlgqgGgsWAfmQnShdtLxgwXIxJChIZ=1977287817;if (SlgqgGgsWAfmQnShdtLxgwXIxJChIZ == SlgqgGgsWAfmQnShdtLxgwXIxJChIZ- 1 ) SlgqgGgsWAfmQnShdtLxgwXIxJChIZ=450072648; else SlgqgGgsWAfmQnShdtLxgwXIxJChIZ=1483142003;if (SlgqgGgsWAfmQnShdtLxgwXIxJChIZ == SlgqgGgsWAfmQnShdtLxgwXIxJChIZ- 0 ) SlgqgGgsWAfmQnShdtLxgwXIxJChIZ=1026057464; else SlgqgGgsWAfmQnShdtLxgwXIxJChIZ=138001862;if (SlgqgGgsWAfmQnShdtLxgwXIxJChIZ == SlgqgGgsWAfmQnShdtLxgwXIxJChIZ- 0 ) SlgqgGgsWAfmQnShdtLxgwXIxJChIZ=1328219661; else SlgqgGgsWAfmQnShdtLxgwXIxJChIZ=1105610278;long yTGBfwlwgUWYVsilwiKBJlYcWKuNmR=112615294;if (yTGBfwlwgUWYVsilwiKBJlYcWKuNmR == yTGBfwlwgUWYVsilwiKBJlYcWKuNmR- 1 ) yTGBfwlwgUWYVsilwiKBJlYcWKuNmR=1607431914; else yTGBfwlwgUWYVsilwiKBJlYcWKuNmR=372254250;if (yTGBfwlwgUWYVsilwiKBJlYcWKuNmR == yTGBfwlwgUWYVsilwiKBJlYcWKuNmR- 1 ) yTGBfwlwgUWYVsilwiKBJlYcWKuNmR=194515001; else yTGBfwlwgUWYVsilwiKBJlYcWKuNmR=1773854492;if (yTGBfwlwgUWYVsilwiKBJlYcWKuNmR == yTGBfwlwgUWYVsilwiKBJlYcWKuNmR- 0 ) yTGBfwlwgUWYVsilwiKBJlYcWKuNmR=2142939925; else yTGBfwlwgUWYVsilwiKBJlYcWKuNmR=198339398;if (yTGBfwlwgUWYVsilwiKBJlYcWKuNmR == yTGBfwlwgUWYVsilwiKBJlYcWKuNmR- 0 ) yTGBfwlwgUWYVsilwiKBJlYcWKuNmR=993422889; else yTGBfwlwgUWYVsilwiKBJlYcWKuNmR=1551606466;if (yTGBfwlwgUWYVsilwiKBJlYcWKuNmR == yTGBfwlwgUWYVsilwiKBJlYcWKuNmR- 0 ) yTGBfwlwgUWYVsilwiKBJlYcWKuNmR=1078524540; else yTGBfwlwgUWYVsilwiKBJlYcWKuNmR=864243223;if (yTGBfwlwgUWYVsilwiKBJlYcWKuNmR == yTGBfwlwgUWYVsilwiKBJlYcWKuNmR- 1 ) yTGBfwlwgUWYVsilwiKBJlYcWKuNmR=248608534; else yTGBfwlwgUWYVsilwiKBJlYcWKuNmR=1907931521;double IFTmHWYuTtTxSqnFwtieRHoijDEPjB=2069925400.340287769980248482843277386681;if (IFTmHWYuTtTxSqnFwtieRHoijDEPjB == IFTmHWYuTtTxSqnFwtieRHoijDEPjB ) IFTmHWYuTtTxSqnFwtieRHoijDEPjB=282467309.509661223340242106997419628391; else IFTmHWYuTtTxSqnFwtieRHoijDEPjB=1882434302.128849168836061303539170510114;if (IFTmHWYuTtTxSqnFwtieRHoijDEPjB == IFTmHWYuTtTxSqnFwtieRHoijDEPjB ) IFTmHWYuTtTxSqnFwtieRHoijDEPjB=436259110.393322350404816155943597215944; else IFTmHWYuTtTxSqnFwtieRHoijDEPjB=1641825799.016454484916424647456527291712;if (IFTmHWYuTtTxSqnFwtieRHoijDEPjB == IFTmHWYuTtTxSqnFwtieRHoijDEPjB ) IFTmHWYuTtTxSqnFwtieRHoijDEPjB=68672709.841659846378103828477960859937; else IFTmHWYuTtTxSqnFwtieRHoijDEPjB=1238261872.487327587581128942548445599290;if (IFTmHWYuTtTxSqnFwtieRHoijDEPjB == IFTmHWYuTtTxSqnFwtieRHoijDEPjB ) IFTmHWYuTtTxSqnFwtieRHoijDEPjB=68634730.091090199644199784292759014383; else IFTmHWYuTtTxSqnFwtieRHoijDEPjB=169540596.830859013896491435692510835219;if (IFTmHWYuTtTxSqnFwtieRHoijDEPjB == IFTmHWYuTtTxSqnFwtieRHoijDEPjB ) IFTmHWYuTtTxSqnFwtieRHoijDEPjB=1141267940.681781545088241746968080243102; else IFTmHWYuTtTxSqnFwtieRHoijDEPjB=1779698973.581431571000061437920936271165;if (IFTmHWYuTtTxSqnFwtieRHoijDEPjB == IFTmHWYuTtTxSqnFwtieRHoijDEPjB ) IFTmHWYuTtTxSqnFwtieRHoijDEPjB=2042390325.954360892127606902981629375537; else IFTmHWYuTtTxSqnFwtieRHoijDEPjB=1423789276.042716681275690344037038543619;float LhaDrYwuCNMDoCfzTtyQFbwyBIVJkQ=676365921.018372265115369345838218334942f;if (LhaDrYwuCNMDoCfzTtyQFbwyBIVJkQ - LhaDrYwuCNMDoCfzTtyQFbwyBIVJkQ> 0.00000001 ) LhaDrYwuCNMDoCfzTtyQFbwyBIVJkQ=1103182127.337991816400518894907140590115f; else LhaDrYwuCNMDoCfzTtyQFbwyBIVJkQ=981690104.832398847963054880055014529851f;if (LhaDrYwuCNMDoCfzTtyQFbwyBIVJkQ - LhaDrYwuCNMDoCfzTtyQFbwyBIVJkQ> 0.00000001 ) LhaDrYwuCNMDoCfzTtyQFbwyBIVJkQ=1553341677.181267822115805420772017183888f; else LhaDrYwuCNMDoCfzTtyQFbwyBIVJkQ=1865987493.900788399816905031021028028419f;if (LhaDrYwuCNMDoCfzTtyQFbwyBIVJkQ - LhaDrYwuCNMDoCfzTtyQFbwyBIVJkQ> 0.00000001 ) LhaDrYwuCNMDoCfzTtyQFbwyBIVJkQ=498804463.504521513714297621316297709387f; else LhaDrYwuCNMDoCfzTtyQFbwyBIVJkQ=873057230.198141349463931893240721432893f;if (LhaDrYwuCNMDoCfzTtyQFbwyBIVJkQ - LhaDrYwuCNMDoCfzTtyQFbwyBIVJkQ> 0.00000001 ) LhaDrYwuCNMDoCfzTtyQFbwyBIVJkQ=1396170533.334673716639396336602614757781f; else LhaDrYwuCNMDoCfzTtyQFbwyBIVJkQ=1173558369.065723037928762253611620172003f;if (LhaDrYwuCNMDoCfzTtyQFbwyBIVJkQ - LhaDrYwuCNMDoCfzTtyQFbwyBIVJkQ> 0.00000001 ) LhaDrYwuCNMDoCfzTtyQFbwyBIVJkQ=1407327172.604678478340088596360427045665f; else LhaDrYwuCNMDoCfzTtyQFbwyBIVJkQ=1154958244.104543429588638175278784328864f;if (LhaDrYwuCNMDoCfzTtyQFbwyBIVJkQ - LhaDrYwuCNMDoCfzTtyQFbwyBIVJkQ> 0.00000001 ) LhaDrYwuCNMDoCfzTtyQFbwyBIVJkQ=171306700.382424754628788390008050753133f; else LhaDrYwuCNMDoCfzTtyQFbwyBIVJkQ=1810023745.576620721452166100603987644278f;int fKIHaKFhqfjtfYavrWgbNYPxcqtzrf=37199692;if (fKIHaKFhqfjtfYavrWgbNYPxcqtzrf == fKIHaKFhqfjtfYavrWgbNYPxcqtzrf- 1 ) fKIHaKFhqfjtfYavrWgbNYPxcqtzrf=901421257; else fKIHaKFhqfjtfYavrWgbNYPxcqtzrf=1405777908;if (fKIHaKFhqfjtfYavrWgbNYPxcqtzrf == fKIHaKFhqfjtfYavrWgbNYPxcqtzrf- 0 ) fKIHaKFhqfjtfYavrWgbNYPxcqtzrf=823142969; else fKIHaKFhqfjtfYavrWgbNYPxcqtzrf=338154121;if (fKIHaKFhqfjtfYavrWgbNYPxcqtzrf == fKIHaKFhqfjtfYavrWgbNYPxcqtzrf- 0 ) fKIHaKFhqfjtfYavrWgbNYPxcqtzrf=379430876; else fKIHaKFhqfjtfYavrWgbNYPxcqtzrf=833258737;if (fKIHaKFhqfjtfYavrWgbNYPxcqtzrf == fKIHaKFhqfjtfYavrWgbNYPxcqtzrf- 0 ) fKIHaKFhqfjtfYavrWgbNYPxcqtzrf=43942585; else fKIHaKFhqfjtfYavrWgbNYPxcqtzrf=369170042;if (fKIHaKFhqfjtfYavrWgbNYPxcqtzrf == fKIHaKFhqfjtfYavrWgbNYPxcqtzrf- 0 ) fKIHaKFhqfjtfYavrWgbNYPxcqtzrf=303619520; else fKIHaKFhqfjtfYavrWgbNYPxcqtzrf=24974405;if (fKIHaKFhqfjtfYavrWgbNYPxcqtzrf == fKIHaKFhqfjtfYavrWgbNYPxcqtzrf- 1 ) fKIHaKFhqfjtfYavrWgbNYPxcqtzrf=632586552; else fKIHaKFhqfjtfYavrWgbNYPxcqtzrf=363240952; }
 fKIHaKFhqfjtfYavrWgbNYPxcqtzrfy::fKIHaKFhqfjtfYavrWgbNYPxcqtzrfy()
 { this->pqYYQpCTHGdL("aPKqAvWAqrQmRdAJKDUbnCLtHuKfjRpqYYQpCTHGdLj", true, 303432720, 1717117260, 1112984956); }
#pragma optimize("", off)
 // <delete/>

