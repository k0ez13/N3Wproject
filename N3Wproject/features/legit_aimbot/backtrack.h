#pragma once
#include "../../options/options.h"
#include "../../valve_sdk/csgostructs.h"
#include <map>
#include <deque>

struct backtrack_data
{
	c_base_player* player;
	mstudiohitboxset_t* hitboxset;
	float simTime;
	Vector origin;
	QAngle angle;
	Vector hitboxPos;
	matrix3x4_t boneMatrix[128];
};

namespace backtrack 
{
	void on_move(c_usercmd *pCmd);

	// не совсем нормально делать это паблик
	// но мне похуй
	float correct_time = 0.0f;
	float latency			 = 0.0f;
	float lerp_time		 = 0.0f;	
	extern std::map<int, std::deque<backtrack_data>> data;
};