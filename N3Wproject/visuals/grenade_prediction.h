#pragma once
#include "../../valve_sdk/csgostructs.h"
#include "../../valve_sdk/sdk.h"
#include "../../render/render.h"

enum ACT
{
	ACT_NONE,
	ACT_THROW,
	ACT_LOB,
	ACT_DROP,
};

namespace grenade_prediction
{
	void tick(int buttons);
	void view();
	void paint();

	void setup(c_base_player* pl, Vector& vecSrc, Vector& vecThrow, const QAngle& angEyeAngles);
	void simulate(QAngle& Angles, c_base_player* pLocal);
	int  step(Vector& vecSrc, Vector& vecThrow, int tick, float interval);
	bool check_detonate(const Vector& vecThrow, const trace_t& tr, int tick, float interval);
	void trace_hull(Vector& src, Vector& end, trace_t& tr);
	void add_gravity_move(Vector& move, Vector& vel, float frametime, bool onground);
	void push_entity(Vector& src, const Vector& move, trace_t& tr);
	void resolve_fly_collision_custom(trace_t& tr, Vector& vecVelocity, float interval);
	int  physics_clip_velocity(const Vector& in, const Vector& normal, Vector& out, float overbounce);
	void draw_3d_cube(float scalar, QAngle angles, Vector middle_origin, Color outline);

	enum
	{
		ACT_NONE,
		ACT_LOB,
		ACT_DROP,
		ACT_THROW
	};

	int act = 0;
	int type = 0;
	std::vector<Vector> path;
	std::vector<std::pair<Vector, QAngle>> other_collisions;
	Color tracer_color = Color(105, 158, 244, 255);
	bool firegrenade_didnt_hit = false;
};
