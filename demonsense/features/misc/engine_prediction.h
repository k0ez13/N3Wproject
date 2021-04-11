#pragma once
#include "../../valve_sdk/csgostructs.h"


namespace engine_prediction
{
	namespace type2
	{
		c_move_data bMoveData[200];
		float flOldCurtime;
		float flOldFrametime;
		uintptr_t uRandomSeed = NULL;

		void start(c_usercmd* cmd)
		{
			static auto oldorigin = g_local_player->m_vecOrigin();
			oldorigin = g_local_player->m_vecOrigin();

			static int flTickBase;
			static c_usercmd* pLastCmd;

			if (!g_local_player) return;

			if (pLastCmd)
			{
				if (pLastCmd->hasbeenpredicted)
					flTickBase = g_local_player->m_nTickBase();
				else
					++flTickBase;
			}

			pLastCmd = cmd;
			flOldCurtime = g_global_vars->curtime;
			flOldFrametime = g_global_vars->frametime;
			g_global_vars->curtime = flTickBase * g_global_vars->interval_per_tick;
			g_global_vars->frametime = g_global_vars->interval_per_tick;

			g_game_movement->start_track_prediction_errors(g_local_player);

			c_move_data data;
			memset(&data, 0, sizeof(c_move_data));

			g_move_helper->set_host(g_local_player);
			g_prediction->setup_move(g_local_player, cmd, g_move_helper, &data);
			g_game_movement->process_movement(g_local_player, &data);
			g_prediction->finish_move(g_local_player, cmd, &data);
			static auto pred_oldorigin = g_local_player->m_vecOrigin();
			pred_oldorigin = g_local_player->m_vecOrigin();
		}

		void finish(c_usercmd * cmd)
		{
			auto local_player = g_local_player;
			if (!local_player) return

			g_game_movement->finish_track_prediction_errors(local_player);
			g_move_helper->set_host(nullptr);
			g_global_vars->curtime = flOldCurtime;
			g_global_vars->frametime = flOldFrametime;
		}
	}

	namespace type1
	{

		float _curtime_backup;
		float _frametime_backup;
		c_move_data _movedata;
		c_usercmd* _prevcmd;
		int _fixedtick;

		int32_t* _prediction_seed;
		c_base_player*** _prediction_player;

		void begin(c_usercmd* cmd)
		{
			_curtime_backup = g_global_vars->curtime;
			_frametime_backup = g_global_vars->frametime;

			if (!_prevcmd || _prevcmd->hasbeenpredicted) {
				_fixedtick = g_local_player->m_nTickBase();
			}
			else {
				_fixedtick++;
			}

			if (!_prediction_seed || !_prediction_player) {
				auto client = GetModuleHandle(TEXT("client.dll"));

				_prediction_seed = *(int32_t * *)(utils::pattern_scan(client, "8B 0D ? ? ? ? BA ? ? ? ? E8 ? ? ? ? 83 C4 04") + 0x2);
				_prediction_player = (c_base_player * **)(utils::pattern_scan(client, "89 35 ? ? ? ? F3 0F 10 48 20") + 0x2);
			}

			if (_prediction_seed) {
				*_prediction_seed = cmd->random_seed;
			}

			if (_prediction_player) {
				**_prediction_player = g_local_player;
			}

			g_local_player->m_pCurrentCommand() = cmd;

			g_global_vars->curtime = static_cast<float>(_fixedtick) * g_global_vars->interval_per_tick;
			g_global_vars->frametime = g_global_vars->interval_per_tick;

			bool _inpred_backup = *(bool*)((uintptr_t)g_prediction + 0x8);

			*(bool*)((uintptr_t)g_prediction + 0x8) = true;

			g_move_helper->set_host(g_local_player);

			g_game_movement->start_track_prediction_errors(g_local_player);
			g_prediction->setup_move(g_local_player, cmd, g_move_helper, &_movedata);
			g_game_movement->process_movement(g_local_player, &_movedata);
			g_prediction->finish_move(g_local_player, cmd, &_movedata);
			g_game_movement->finish_track_prediction_errors(g_local_player);

			*(bool*)((uintptr_t)g_prediction + 0x8) = _inpred_backup;

			g_move_helper->set_host(nullptr);

			if (_prediction_seed) {
				*_prediction_seed = -1;
			}

			if (_prediction_player) {
				**_prediction_player = nullptr;
			}

			g_local_player->m_pCurrentCommand() = nullptr;

			_prevcmd = cmd;
		}

		void end() {
			g_global_vars->curtime = _curtime_backup;
			g_global_vars->frametime = _frametime_backup;
		}
	}
}
