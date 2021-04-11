#pragma once
#include "../../options/options.h"
#include "../../valve_sdk/csgostructs.h"

namespace legit_aimbot
{
	void on_move(c_usercmd* pCmd);
	//void triggerboton_createmove(c_usercmd* cmd);
	bool is_enabled(c_usercmd* pCmd);
	bool is_trigger();

	float get_fov_to_player(QAngle viewAngle, QAngle aimAngle);
	float get_real_distance_fov(float distance, QAngle angle, QAngle viewangles);

	bool  is_rcs();
	float get_smooth();
	float get_fov();
	int   get_group();
	float without_silent();

	int hitchance_wp;


	void rcs(QAngle& angle, c_base_player* target, bool should_run);
	bool is_line_goes_through_smoke(Vector vStartPos, Vector vEndPos);
	void smooth(QAngle currentAngle, QAngle aimAngle, QAngle& angle);
	bool is_not_silent(float fov);
	c_base_player* get_closest_player(c_usercmd* cmd, int& bestBone);

	legitbot_settings _settings;
	bool _is_trigger = false;

	bool _is_backshot = false;
	//float shot_delay_time;
	//bool shot_delay = false;
	//bool trigger_delay = false;
	//bool trigger = false;
	int trigger_delay_time = 0;
	int trigger_delay_between_shots_time = 0;
	int key = 1;
	bool silent_enabled = false;
	QAngle current_punch = { 0,0,0 };
	QAngle rcs_last_punch = { 0,0,0 };
	bool is_delayed = false;
	int kill_delay_time;
	bool kill_delay = false;
	Vector aim_position = { 0,0,0 };
	c_base_player* target = NULL;
	void trigger(c_usercmd* pCmd);

}
