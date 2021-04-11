#include <algorithm>

#include "../notify/notify.h"
#include "../../menu/menu.h"

#include "visuals.h"

#include "../../options/options.h"
#include "../../helpers/math.h"
#define M_PI			3.14159265358979323846
#define DEG2RAD2( x  )  ( (float)(x) * (float)(PI_F / 180.f) )
#include "../../helpers/utils.h"

#include "grenade_prediction.h"

#include "../legit_aimbot/aimbot.h"
#include "../legit_aimbot/autowall.h"
#include "../legit_aimbot/backtrack.h"

#include "../../hooks/hooks.h"
#include "../../saver_random.h"

int selected_entity;
auto debug_crosshair = -1;
auto global_esp_settings = settings::visuals::esp::esp_items[selected_entity];
bool CanSee[65];


float fl_WeaponMaxAmmo[1024];

float flPlayerAlpha[65];
float PlayerAlpha[65];
float StoredCurtimePlayerAlpha[1024];
float StoredCurtimePlayer[1024];


std::map<int, std::string> fixed_names =
{
	//pistols
	////////////////////////////////////////////////
	{ ItemDefinitionIndex::WEAPON_USP_SILENCER, "usp" },
	{ ItemDefinitionIndex::WEAPON_GLOCK, "glock" },
	{ ItemDefinitionIndex::WEAPON_HKP2000, "P2000" },
	{ ItemDefinitionIndex::WEAPON_ELITE, "elite" },
	{ ItemDefinitionIndex::WEAPON_P250, "P250" },
	{ ItemDefinitionIndex::WEAPON_FIVESEVEN, "five-Seven" },
	{ ItemDefinitionIndex::WEAPON_CZ75A, "cz75a" },
	{ ItemDefinitionIndex::WEAPON_TEC9, "tec9" },
	{ ItemDefinitionIndex::WEAPON_DEAGLE, "deagle" },
	{ ItemDefinitionIndex::WEAPON_REVOLVER, "revolver" },
	////////////////////////////////////////////////
	//

	//pp
	////////////////////////////////////////////////
    { ItemDefinitionIndex::WEAPON_MP9, "mp9" },
	{ ItemDefinitionIndex::WEAPON_MAC10, "mac10" },
	{ ItemDefinitionIndex::WEAPON_MP7, "mp7" },
	{ ItemDefinitionIndex::WEAPON_MP5, "mp5-sd" },
	{ ItemDefinitionIndex::WEAPON_UMP45, "ump-45" },
	{ ItemDefinitionIndex::WEAPON_P90, "P90" },
	{ ItemDefinitionIndex::WEAPON_BIZON, "bizon" },
	////////////////////////////////////////////////
	//

	//rifles
	////////////////////////////////////////////////
	{ ItemDefinitionIndex::WEAPON_FAMAS, "famas" },
	{ ItemDefinitionIndex::WEAPON_M4A1_SILENCER, "M4a1" },
	{ ItemDefinitionIndex::WEAPON_M4A1, "M4a4" },
	{ ItemDefinitionIndex::WEAPON_SSG08, "ssg 08" },
	{ ItemDefinitionIndex::WEAPON_AUG, "aug" },
	{ ItemDefinitionIndex::WEAPON_AWP, "awp" },
	{ ItemDefinitionIndex::WEAPON_SCAR20, "scar20" },
	{ ItemDefinitionIndex::WEAPON_GALILAR, "galil" },
	{ ItemDefinitionIndex::WEAPON_AK47, "ak-47" },
	{ ItemDefinitionIndex::WEAPON_SG556, "sg 553" },
	{ ItemDefinitionIndex::WEAPON_G3SG1, "g3sg1" },
	////////////////////////////////////////////////
	//

	//have
	////////////////////////////////////////////////
	{ ItemDefinitionIndex::WEAPON_NOVA, "nova" },
	{ ItemDefinitionIndex::WEAPON_XM1014, "xm1014" },
	{ ItemDefinitionIndex::WEAPON_SAWEDOFF, "sawedoff" },
	{ ItemDefinitionIndex::WEAPON_M249, "M249" },
	{ ItemDefinitionIndex::WEAPON_NEGEV, "negev" },
	{ ItemDefinitionIndex::WEAPON_MAG7, "mag7" },
	////////////////////////////////////////////////
	//

	//granades
	////////////////////////////////////////////////
	{ ItemDefinitionIndex::WEAPON_FLASHBANG, "flash" },
	{ ItemDefinitionIndex::WEAPON_SMOKEGRENADE, "smoke" },
	{ ItemDefinitionIndex::WEAPON_MOLOTOV, "molotov" },
	{ ItemDefinitionIndex::WEAPON_INCGRENADE, "inc" },
	{ ItemDefinitionIndex::WEAPON_DECOY, "decoy" },
	{ ItemDefinitionIndex::WEAPON_HEGRENADE, "HE" },
	////////////////////////////////////////////////
	//


	//other
	////////////////////////////////////////////////
	{ ItemDefinitionIndex::WEAPON_C4, "C4" },
	{ ItemDefinitionIndex::WEAPON_KNIFE, "Knife" },
	{ ItemDefinitionIndex::WEAPON_KNIFEGG, "Knife" },
	{ ItemDefinitionIndex::WEAPON_KNIFE_BAYONET, "Knife" },
	{ ItemDefinitionIndex::WEAPON_KNIFE_BUTTERFLY, "Knife" },
	{ ItemDefinitionIndex::WEAPON_KNIFE_FALCHION, "Knife" },
	{ ItemDefinitionIndex::WEAPON_KNIFE_FLIP, "Knife" },
	{ ItemDefinitionIndex::WEAPON_KNIFE_GHOST, "Knife" },
	{ ItemDefinitionIndex::WEAPON_KNIFE_GUT, "Knife" },
	{ ItemDefinitionIndex::WEAPON_KNIFE_GYPSY_JACKKNIFE, "Knife" },
	{ ItemDefinitionIndex::WEAPON_KNIFE_KARAMBIT, "Knife" },
	{ ItemDefinitionIndex::WEAPON_KNIFE_M9_BAYONET, "Knife" },
	{ ItemDefinitionIndex::WEAPON_KNIFE_NAVAJA, "Knife" },
	{ ItemDefinitionIndex::WEAPON_KNIFE_PUSH, "Knife" },
	{ ItemDefinitionIndex::WEAPON_KNIFE_STILETTO, "Knife" },
	{ ItemDefinitionIndex::WEAPON_KNIFE_SURVIVAL_BOWIE, "Knife" },
	{ ItemDefinitionIndex::WEAPON_KNIFE_T, "Knife" },
	{ ItemDefinitionIndex::WEAPON_KNIFE_TACTICAL, "Knife" },
	{ ItemDefinitionIndex::WEAPON_KNIFE_TALON, "Knife" },
	{ ItemDefinitionIndex::WEAPON_KNIFE_URSUS, "Knife" },
	{ ItemDefinitionIndex::WEAPON_KNIFE_WIDOWMAKER, "Knife" },
	{ ItemDefinitionIndex::WEAPON_TASER, "Zeus" }
	////////////////////////////////////////////////
	//
};

std::map<int, std::string> weapons_icons1 =
{
	//pistols
	////////////////////////////////////////////////
	{ ItemDefinitionIndex::WEAPON_USP_SILENCER, "G" },
	{ ItemDefinitionIndex::WEAPON_GLOCK, "D" },
	{ ItemDefinitionIndex::WEAPON_HKP2000, "E" },
	{ ItemDefinitionIndex::WEAPON_ELITE, "B" },
	{ ItemDefinitionIndex::WEAPON_P250, "F" },
	{ ItemDefinitionIndex::WEAPON_FIVESEVEN, "C" },
	{ ItemDefinitionIndex::WEAPON_CZ75A, "I" },
	{ ItemDefinitionIndex::WEAPON_TEC9, "H" },
	{ ItemDefinitionIndex::WEAPON_DEAGLE, "A" },
	{ ItemDefinitionIndex::WEAPON_REVOLVER, "J" },
	////////////////////////////////////////////////
	//

	//pp
	////////////////////////////////////////////////
	{ ItemDefinitionIndex::WEAPON_MP9, "O" },
	{ ItemDefinitionIndex::WEAPON_MAC10, "K" },
	{ ItemDefinitionIndex::WEAPON_MP7, "N" },
	{ ItemDefinitionIndex::WEAPON_MP5, "N" },
	{ ItemDefinitionIndex::WEAPON_UMP45, "L" },
	{ ItemDefinitionIndex::WEAPON_P90, "P" },
	{ ItemDefinitionIndex::WEAPON_BIZON, "M" },
	////////////////////////////////////////////////
	//

	//rifles
	////////////////////////////////////////////////
	{ ItemDefinitionIndex::WEAPON_FAMAS, "R" },
	{ ItemDefinitionIndex::WEAPON_M4A1_SILENCER, "T" },
	{ ItemDefinitionIndex::WEAPON_M4A1, "S" },
	{ ItemDefinitionIndex::WEAPON_SSG08, "a" },
	{ ItemDefinitionIndex::WEAPON_AUG, "U" },
	{ ItemDefinitionIndex::WEAPON_AWP, "Z" },
	{ ItemDefinitionIndex::WEAPON_SCAR20, "Y" },
	{ ItemDefinitionIndex::WEAPON_GALILAR, "Q" },
	{ ItemDefinitionIndex::WEAPON_AK47, "W" },
	{ ItemDefinitionIndex::WEAPON_SG556, "V" },
	{ ItemDefinitionIndex::WEAPON_G3SG1, "X" },
	////////////////////////////////////////////////
	//

	//have
	////////////////////////////////////////////////
	{ ItemDefinitionIndex::WEAPON_NOVA, "e" },
	{ ItemDefinitionIndex::WEAPON_XM1014, "b" },
	{ ItemDefinitionIndex::WEAPON_SAWEDOFF, "c" },
	{ ItemDefinitionIndex::WEAPON_M249, "g" },
	{ ItemDefinitionIndex::WEAPON_NEGEV, "f" },
	{ ItemDefinitionIndex::WEAPON_MAG7, "d" },
	////////////////////////////////////////////////
	//

	//granades
	////////////////////////////////////////////////
	{ ItemDefinitionIndex::WEAPON_FLASHBANG, "i" },
	{ ItemDefinitionIndex::WEAPON_SMOKEGRENADE, "k" },
	{ ItemDefinitionIndex::WEAPON_MOLOTOV, "l" },
	{ ItemDefinitionIndex::WEAPON_INCGRENADE, "n" },
	{ ItemDefinitionIndex::WEAPON_DECOY, "m" },
	{ ItemDefinitionIndex::WEAPON_HEGRENADE, "j" },
	////////////////////////////////////////////////
	//


	//other
	////////////////////////////////////////////////
	{ ItemDefinitionIndex::WEAPON_C4, "o" },
	{ ItemDefinitionIndex::WEAPON_KNIFE, "7" },
	{ ItemDefinitionIndex::WEAPON_KNIFEGG, "7" },
	{ ItemDefinitionIndex::WEAPON_KNIFE_BAYONET, "1" },
	{ ItemDefinitionIndex::WEAPON_KNIFE_BUTTERFLY, "8" },
	{ ItemDefinitionIndex::WEAPON_KNIFE_FALCHION, "10" },
	{ ItemDefinitionIndex::WEAPON_KNIFE_FLIP, "2" },
	{ ItemDefinitionIndex::WEAPON_KNIFE_GHOST, "7" },
	{ ItemDefinitionIndex::WEAPON_KNIFE_GUT, "3" },
	{ ItemDefinitionIndex::WEAPON_KNIFE_GYPSY_JACKKNIFE, "7" },
	{ ItemDefinitionIndex::WEAPON_KNIFE_KARAMBIT, "4" },
	{ ItemDefinitionIndex::WEAPON_KNIFE_M9_BAYONET, "5" },
	{ ItemDefinitionIndex::WEAPON_KNIFE_NAVAJA, "7" },
	{ ItemDefinitionIndex::WEAPON_KNIFE_PUSH, "9" },
	{ ItemDefinitionIndex::WEAPON_KNIFE_STILETTO, "7" },
	{ ItemDefinitionIndex::WEAPON_KNIFE_SURVIVAL_BOWIE, "7" },
	{ ItemDefinitionIndex::WEAPON_KNIFE_T, "7" },
	{ ItemDefinitionIndex::WEAPON_KNIFE_TACTICAL, "6" },
	{ ItemDefinitionIndex::WEAPON_KNIFE_TALON, "4" },
	{ ItemDefinitionIndex::WEAPON_KNIFE_URSUS, "7" },
	{ ItemDefinitionIndex::WEAPON_KNIFE_WIDOWMAKER, "7" },
	{ ItemDefinitionIndex::WEAPON_TASER, "h" }
	////////////////////////////////////////////////
	//
};

int bone(int event_bone)
{
	switch (event_bone)
	{
	case hit_head:
		return HITBOX_HEAD;
	case hit_left_foot:
		return HITBOX_RIGHT_THIGH;
	case hit_right_foot:
		return HITBOX_LEFT_THIGH;
	case hit_chest:
		return HITBOX_UPPER_CHEST;
	case hit_lower_chest:
		return HITBOX_LOWER_CHEST;
	case hit_left_hand:
		return HITBOX_RIGHT_FOREARM;
	case hit_right_hand:
		return HITBOX_LEFT_FOREARM;
	case hit_neck:
		return HITBOX_NECK;
	default:
		return HITBOX_STOMACH;
	}
}

int get_player_alpha(int alpha, int index)
{
	int i = alpha - flPlayerAlpha[index];
	i = std::clamp(i, 0, 255);
	return i;
}

RECT get_bbox(c_base_entity* ent) {
	RECT rect{};
	auto collideable = ent->get_collideable();

	if (!collideable)
		return rect;

	auto min = collideable->obb_mins();
	auto max = collideable->obb_maxs();

	const matrix3x4_t& trans = ent->m_rgflCoordinateFrame();

	Vector points[] = {
		Vector(min.x, min.y, min.z),
		Vector(min.x, max.y, min.z),
		Vector(max.x, max.y, min.z),
		Vector(max.x, min.y, min.z),
		Vector(max.x, max.y, max.z),
		Vector(min.x, max.y, max.z),
		Vector(min.x, min.y, max.z),
		Vector(max.x, min.y, max.z)
	};

	Vector pointsTransformed[8];
	for (int i = 0; i < 8; i++) {
		math::vector_transform(points[i], trans, pointsTransformed[i]);
	}

	Vector screen_points[8] = {};

	for (int i = 0; i < 8; i++) {
		if (!math::world_to_screen(pointsTransformed[i], screen_points[i]))
			return rect;
	}

	auto left = screen_points[0].x;
	auto top = screen_points[0].y;
	auto right = screen_points[0].x;
	auto bottom = screen_points[0].y;

	for (int i = 1; i < 8; i++) {
		if (left > screen_points[i].x)
			left = screen_points[i].x;
		if (top < screen_points[i].y)
			top = screen_points[i].y;
		if (right < screen_points[i].x)
			right = screen_points[i].x;
		if (bottom > screen_points[i].y)
			bottom = screen_points[i].y;
	}
	return RECT{ (long)left, (long)top, (long)right, (long)bottom };
}

float dot_product(const Vector& a, const Vector& b)
{
	return (a.x * b.x
		+ a.y * b.y
		+ a.z * b.z);
}

mstudiobbox_t* get_hitbox(c_base_entity* entity, int hitbox_index)
{
	if (!entity)
		return NULL;
	if (entity->get_health() <= 0)
		return NULL;

	const auto pModel = entity->get_model();
	if (!pModel)
		return NULL;

	auto pStudioHdr = g_mdl_info->get_studiomodel(pModel);
	if (!pStudioHdr)
		return NULL;

	auto pSet = pStudioHdr->get_hitbox_set(0);
	if (!pSet)
		return NULL;

	if (hitbox_index >= pSet->numhitboxes || hitbox_index < 0)
		return NULL;

	return pSet->get_hitbox(hitbox_index);
}

Vector get_hitbox_position(c_base_entity* entity, int hitbox_id)
{
	auto hitbox = get_hitbox(entity, hitbox_id);
	if (!hitbox)
		return Vector(0, 0, 0);

	auto bone_matrix = entity->get_bone_matrix(hitbox->bone);

	Vector bbmin, bbmax;
	math::vector_transform(hitbox->bbmin, bone_matrix, bbmin);
	math::vector_transform(hitbox->bbmax, bone_matrix, bbmax);

	return (bbmin + bbmax) * 0.5f;
}

void add_triangle_filled(std::array<Vector2D, 3>points, Color color)
{
	//std::vector<Vertex_t>vertices{ Vertex_t(points.at(0)), Vertex_t(points.at(1)), Vertex_t(points.at(2)) };
	std::vector<ImVec2>vertices{ ImVec2(points.at(0).x, points.at(0).y), ImVec2(points.at(1).x, points.at(1).y), ImVec2(points.at(2).x, points.at(2).y) };
	render::draw_textured_polygon(3, vertices, color);
}

float get_defuse_time(c_planted_c4* bomb)
{
	static float defuse_time = -1;

	if (!bomb)
		return 0;

	if (!bomb->m_hBombDefuser())
		defuse_time = -1;

	else if (defuse_time == -1)
		defuse_time = g_global_vars->curtime + bomb->m_flDefuseLength();

	if (defuse_time > -1 && bomb->m_hBombDefuser())
		return defuse_time - g_global_vars->curtime;

	return 0;
}

c_base_player* get_bomb_player()
{
	for (auto i = 1; i <= g_entity_list->get_highest_entity_index(); ++i)
	{
		auto* entity = c_base_player::get_player_by_index(i);
		if (!entity || entity->is_player() || entity->is_dormant() || entity == g_local_player)
			continue;

		if (entity->is_planted_c4())
			return entity;
	}

	return nullptr;
}

c_planted_c4* get_bomb()
{
	//if (!interfaces::game_rules_proxy || !interfaces::game_rules_proxy->m_bBombPlanted())
		//return nullptr;

	c_base_entity* entity;
	for (auto i = 1; i <= g_entity_list->get_max_entities(); ++i)
	{
		entity = c_base_entity::get_entity_by_index(i);
		if (entity && !entity->is_dormant() && entity->is_planted_c4())
			return reinterpret_cast<c_planted_c4*>(entity);
	}

	return nullptr;
}

observer_mode bec_person = observer_mode::OBS_MODE_IN_EYE;
bool setted_person = false;

namespace visuals
{
	bool player::begin(c_base_player* pl)
	{
		if (!pl->is_alive())
			return false;

		ctx.pl = pl;
		ctx.is_localplayer = g_local_player->ent_index() == pl->ent_index();
		ctx.is_enemy = g_local_player->m_iTeamNum() != pl->m_iTeamNum();
		ctx.is_visible = g_local_player->can_see_player(pl, HITBOX_CHEST);

		if (g_local_player->m_hObserverTarget() == ctx.pl && !settings::misc::third_person::bind.enable)
		{
			return false;
		}

		if (ctx.is_enemy)
		{
			global_esp_settings = settings::visuals::esp::esp_items[esp_types::enemies];
		}
		else if (!ctx.is_enemy && !ctx.is_localplayer)
		{
			global_esp_settings = settings::visuals::esp::esp_items[esp_types::teammates];
		}
		else if (ctx.is_localplayer)
		{
			if (settings::misc::third_person::bind.enable)
				global_esp_settings = settings::visuals::esp::esp_items[esp_types::local_player];
			else
				return false;
		}

		if (!global_esp_settings.enable)
			return false;

		ctx.clr = ctx.is_visible ? global_esp_settings.box_visible : global_esp_settings.box_invisible;


		ctx.bbox = get_bbox(pl);

		if (!ctx.bbox.bottom || !ctx.bbox.right || !ctx.bbox.left || !ctx.bbox.top)
			return false;


		std::swap(ctx.bbox.top, ctx.bbox.bottom);
		return true;

		/*if (!pl->is_alive())
			return false;

		ctx.pl = pl;
		ctx.is_localplayer = g_local_player->ent_index() == pl->ent_index();
		ctx.is_enemy = g_local_player->m_iTeamNum() != pl->m_iTeamNum();
		ctx.is_visible = g_local_player->can_see_player(pl, HITBOX_CHEST);

		if (g_local_player->m_hObserverTarget() == ctx.pl && !settings::misc::third_person::bind.enable)
		{
			return false;
		}

		if (ctx.is_enemy)
		{
			global_esp_settings = settings::visuals::esp::esp_items[esp_types::enemies];
		}
		else if (!ctx.is_enemy && !ctx.is_localplayer)
		{
			global_esp_settings = settings::visuals::esp::esp_items[esp_types::teammates];
		}
		else if (ctx.is_localplayer)
		{
			if (settings::misc::third_person::bind.enable)
				global_esp_settings = settings::visuals::esp::esp_items[esp_types::local_player];
			else
				return false;
		}

		if (!global_esp_settings.enable)
			return false;

		ctx.clr = ctx.is_visible ? global_esp_settings.box_visible : global_esp_settings.box_invisible;

		auto head = pl->get_hitbox_pos(HITBOX_HEAD);
		auto origin = pl->m_vecOrigin();

		head.z += 6;

		if (!math::world_to_screen(head, ctx.head_pos) ||
			!math::world_to_screen(origin, ctx.feet_pos))
		{
			return false;
		}

		auto h = fabs(ctx.head_pos.y - ctx.feet_pos.y);
		auto w = h / 2.f;

		ctx.bbox.left = static_cast<long>(ctx.feet_pos.x - w * 0.45f);
		ctx.bbox.right = static_cast<long>(ctx.bbox.left + w);
		ctx.bbox.bottom = static_cast<long>(ctx.feet_pos.y);
		ctx.bbox.top = static_cast<long>(ctx.head_pos.y);

		return true;*/
	}

	void player::draw_skeleton(c_base_entity* ent)
	{
		auto model = ent->get_model();
		if (ent && !model)
			return;

		studiohdr_t* pStudioModel = g_mdl_info->get_studiomodel(model);

		if (pStudioModel) {
			static matrix3x4_t pBoneToWorldOut[128];

			if (ent->setup_bones(pBoneToWorldOut, 128, 256, 0.f)) {
				for (int i = 0; i < pStudioModel->numbones; i++) {
					mstudiobone_t* pBone = pStudioModel->get_bone(i);
					if (!pBone || !(pBone->flags & 256) || pBone->parent == -1)
						continue;

					Vector vBonePos1;
					if (!math::world_to_screen(Vector(pBoneToWorldOut[i][0][3], pBoneToWorldOut[i][1][3], pBoneToWorldOut[i][2][3]), vBonePos1))
						continue;

					Vector vBonePos2;
					if (!math::world_to_screen(Vector(pBoneToWorldOut[pBone->parent][0][3], pBoneToWorldOut[pBone->parent][1][3], pBoneToWorldOut[pBone->parent][2][3]), vBonePos2))
						continue;


					if (ctx.is_visible)
						render::draw_line((int)vBonePos1.x, (int)vBonePos1.y, (int)vBonePos2.x, (int)vBonePos2.y, Color(global_esp_settings.skeleton_visible, get_player_alpha(global_esp_settings.skeleton_visible.a(), ctx.pl->ent_index())));
					else
						render::draw_line((int)vBonePos1.x, (int)vBonePos1.y, (int)vBonePos2.x, (int)vBonePos2.y, Color(global_esp_settings.skeleton_invisible, get_player_alpha(global_esp_settings.skeleton_invisible.a(), ctx.pl->ent_index())));
				}
			}
		}
	}

	void player::draw_backtrack()
	{
		Vector localEyePos = g_local_player->get_eye_pos();
		QAngle angles;
		int tick_count = -1;
		float best_fov = 180.0f;
		for (auto& node : backtrack::data) 
		{
			auto& cur_data = node.second;
			if (cur_data.empty())
				return;

			for (auto& bd : cur_data) {
				float deltaTime = backtrack::correct_time - (g_global_vars->curtime - bd.simTime);
				if (std::fabsf(deltaTime) > legit_aimbot::_settings.backtrack_time)
					continue;

				Vector w2s;
				if (!math::world_to_screen(bd.hitboxPos, w2s))
					continue;

				render::draw_boxfilled(w2s.x - 1.0f, w2s.y - 1.0f, w2s.x + 1.0f, w2s.y + 1.0f, Color(1.f, 1.f, 1.f, 1.f));
			}
		}
	}

	void player::draw_boxedge(float x1, float y1, float x2, float y2, Color clr, float edge_size, float size)
	{
		if (fabs(x1 - x2) < (edge_size * 2)) {
			//x2 = x1 + fabs(x1 - x2);
			edge_size = fabs(x1 - x2) / 4.f;
		}

		render::draw_line(x1, y1, x1, y1 + edge_size + (0.5f * edge_size), clr, size);
		render::draw_line(x2, y1, x2, y1 + edge_size + (0.5f * edge_size), clr, size);
		render::draw_line(x1, y2, x1, y2 - edge_size - (0.5f * edge_size), clr, size);
		render::draw_line(x2, y2, x2, y2 - edge_size - (0.5f * edge_size), clr, size);
		render::draw_line(x1, y1, x1 + edge_size, y1, clr, size);
		render::draw_line(x2, y1, x2 - edge_size, y1, clr, size);
		render::draw_line(x1, y2, x1 + edge_size, y2, clr, size);
		render::draw_line(x2, y2, x2 - edge_size, y2, clr, size);
	}
	
	void player::draw_box()
	{
		Color main;
		if (ctx.pl->is_dormant())
			main = Color(global_esp_settings.dormant, get_player_alpha(255, ctx.pl->ent_index()));

		else if (!ctx.pl->is_dormant())
		{
			if (ctx.is_visible)
			{
				main = Color(global_esp_settings.box_visible, get_player_alpha(global_esp_settings.box_visible.a(), ctx.pl->ent_index()));
			}
			else if (!ctx.is_visible)
			{
				main = Color(global_esp_settings.box_invisible, get_player_alpha(global_esp_settings.box_invisible.a(), ctx.pl->ent_index()));
			}
		}
		Color outline = Color(0, 0, 0, get_player_alpha(255, ctx.pl->ent_index()));

		if (global_esp_settings.box_position == 0)
		{
		render::draw_box(ctx.bbox.left, ctx.bbox.top, ctx.bbox.right, ctx.bbox.bottom, main, 1);
		render::draw_box(ctx.bbox.left + 1, ctx.bbox.top + 1, ctx.bbox.right - 1, ctx.bbox.bottom - 1, outline, 1);
		render::draw_box(ctx.bbox.left - 1, ctx.bbox.top - 1, ctx.bbox.right + 1, ctx.bbox.bottom + 1, outline, 1);
	    }


		if (global_esp_settings.box_position == 1)
		{
			float edge_size = 25.f;

			if (ctx.pl != g_local_player)
				edge_size = 4000.f / math::vector_distance(g_local_player->m_vecOrigin(), ctx.pl->m_vecOrigin());

			float length_horizontal = (ctx.bbox.right - ctx.bbox.left) * 0.2f, length_vertical = (ctx.bbox.bottom - ctx.bbox.top) * 0.2f;


			render::draw_boxfilled(ctx.bbox.left - 1.f, ctx.bbox.top - 1.f, ctx.bbox.left + 1.f + length_horizontal, ctx.bbox.top + 2.f, outline);
			render::draw_boxfilled(ctx.bbox.right - 1.f - length_horizontal, ctx.bbox.top - 1.f, ctx.bbox.right + 1.f, ctx.bbox.top + 2.f, outline);
			render::draw_boxfilled(ctx.bbox.left - 1.f, ctx.bbox.bottom - 2.f, ctx.bbox.left + 1.f + length_horizontal, ctx.bbox.bottom + 1.f, outline);
			render::draw_boxfilled(ctx.bbox.right - 1.f - length_horizontal, ctx.bbox.bottom - 2.f, ctx.bbox.right + 1.f, ctx.bbox.bottom + 1.f, outline);

			render::draw_boxfilled(ctx.bbox.left - 1.f, ctx.bbox.top + 2.f, ctx.bbox.left + 2.f, ctx.bbox.top + 1.f + length_vertical, outline);
			render::draw_boxfilled(ctx.bbox.right - 2.f, ctx.bbox.top + 2.f, ctx.bbox.right + 1.f, ctx.bbox.top + 1.f + length_vertical, outline);
			render::draw_boxfilled(ctx.bbox.left - 1.f, ctx.bbox.bottom - 1.f - length_vertical, ctx.bbox.left + 2.f, ctx.bbox.bottom - 2.f, outline);
			render::draw_boxfilled(ctx.bbox.right - 2.f, ctx.bbox.bottom - 1.f - length_vertical, ctx.bbox.right + 1.f, ctx.bbox.bottom - 2.f, outline);


			render::draw_line(float(ctx.bbox.left), float(ctx.bbox.top), ctx.bbox.left + length_horizontal - 1.f, float(ctx.bbox.top), main);
			render::draw_line(ctx.bbox.right - length_horizontal, float(ctx.bbox.top), ctx.bbox.right - 1.f, float(ctx.bbox.top), main);
			render::draw_line(float(ctx.bbox.left), ctx.bbox.bottom - 1.f, ctx.bbox.left + length_horizontal - 1.f, ctx.bbox.bottom - 1.f, main);
			render::draw_line(ctx.bbox.right - length_horizontal, ctx.bbox.bottom - 1.f, ctx.bbox.right - 1.f, ctx.bbox.bottom - 1.f, main);

			render::draw_line(float(ctx.bbox.left), float(ctx.bbox.top), float(ctx.bbox.left), ctx.bbox.top + length_vertical - 1.f, main);
			render::draw_line(ctx.bbox.right - 1.f, float(ctx.bbox.top), ctx.bbox.right - 1.f, ctx.bbox.top + length_vertical - 1.f, main);
			render::draw_line(float(ctx.bbox.left), ctx.bbox.bottom - length_vertical, float(ctx.bbox.left), ctx.bbox.bottom - 1.f, main);
			render::draw_line(ctx.bbox.right - 1.f, ctx.bbox.bottom - length_vertical, ctx.bbox.right - 1.f, ctx.bbox.bottom - 1.f, main);

			/*float edge_size = 25.f;
			if (ctx.pl != g_local_player)
				edge_size = 4000.f / math::vector_distance(g_local_player->m_vecOrigin(), ctx.pl->m_vecOrigin());


			player::draw_boxedge(ctx.bbox.left, ctx.bbox.top, ctx.bbox.right, ctx.bbox.bottom, ctx.clr, edge_size, 1);
			player::draw_boxedge(ctx.bbox.left + 1, ctx.bbox.top + 1, ctx.bbox.right - 1, ctx.bbox.bottom - 1, outline, edge_size, 1);
			player::draw_boxedge(ctx.bbox.left - 1, ctx.bbox.top - 1, ctx.bbox.right + 1, ctx.bbox.bottom + 1, outline, edge_size, 1);*/

		}
	}

	void player::draw_health()
	{
		auto  hp = ctx.pl->m_iHealth();
		float box_w;
		float box_h;
		float off = 7;
		int height;

		float x;
		float y;
		float w;
		float h;

		int green = int(hp * 2.55f);
		int red = 255 - green;

		Color main = global_esp_settings.health_based ? Color(red, green, 0, get_player_alpha(global_esp_settings.health_bar_main.a(), ctx.pl->ent_index())) : Color(global_esp_settings.health_bar_main, get_player_alpha(global_esp_settings.health_bar_main.a(), ctx.pl->ent_index()));

		if (global_esp_settings.health_position == 0)
		{
			box_w = (float)fabs(ctx.bbox.right - ctx.bbox.left);
			box_h = (float)fabs(ctx.bbox.bottom - ctx.bbox.top);
			height = (box_h * hp) / 100;

			x = ctx.bbox.left - off;
			y = ctx.bbox.bottom;
			w = 4;
			h = box_h;
			render::draw_box(x, y - h - 1, x + w, y + 1, Color(global_esp_settings.health_bar_outline, get_player_alpha(global_esp_settings.health_bar_outline.a(), ctx.pl->ent_index())), 1.f, 0.f);
			render::draw_boxfilled(x + 1, y - (int)box_h, x + w - 1, y, Color(global_esp_settings.health_bar_background, get_player_alpha(global_esp_settings.health_bar_background.a(), ctx.pl->ent_index())), 1.f, 0.f);

			render::draw_boxfilled(x + 1, y - height, x + w - 1, y, main, 1.f, 0.f);
		}
		if (global_esp_settings.health_position == 1)
		{
			box_w = (float)fabs(ctx.bbox.right - ctx.bbox.left);
			box_h = (float)fabs(ctx.bbox.bottom - ctx.bbox.top);

			x = ctx.bbox.right + off;
			y = ctx.bbox.bottom;
			w = 4;
			h = box_h;

			height = (box_h * hp) / 100;

			//render::draw_boxfilled(x - w + 1, (y - h), x - w + 2, (y - h) - 1, Color(g_Options.color_esp_healthbar_background, (int)flPlayerAlpha[ctx.pl->ent_index()]), 1.f, true);

			render::draw_box(x - w, (y - h) - 1, x, y + 1, Color(global_esp_settings.health_bar_outline, get_player_alpha(global_esp_settings.health_bar_outline.a(), ctx.pl->ent_index())), 1.f, true);
			render::draw_boxfilled(x - w + 1, (y - h), x - 1, y, Color(global_esp_settings.health_bar_background, get_player_alpha(global_esp_settings.health_bar_background.a(), ctx.pl->ent_index())), 1.f, true);
			render::draw_boxfilled(x - w + 1, (y - height), x - 1, y, main, 1.f, true);
		}

		if (global_esp_settings.health_position == 2)
		{
			box_w = (float)fabs(ctx.bbox.right - ctx.bbox.left);
			box_h = 4.f;

			x = ctx.bbox.left;
			y = ctx.bbox.top - off;
			w = 4;
			h = box_h;

			height = (box_w * hp) / 100;

			render::draw_box(x - 1.f, y, x + box_w + 1, y + w, Color(global_esp_settings.health_bar_outline, get_player_alpha(global_esp_settings.health_bar_outline.a(), ctx.pl->ent_index())), 1.f, 0.f);
			render::draw_boxfilled(x, y + 1.f, x + box_w, y + w - 1.f, Color(global_esp_settings.health_bar_background, get_player_alpha(global_esp_settings.health_bar_background.a(), ctx.pl->ent_index())), 1.f, true);
			render::draw_boxfilled(x, y + 1.f, x + height, y + w - 1.f, main, 1.f, true);
		}
		if (global_esp_settings.health_position == 3)
		{
			box_w = (float)fabs(ctx.bbox.right - ctx.bbox.left);
			box_h = 4.f;

			x = ctx.bbox.left;
			y = ctx.bbox.bottom + off;
			w = 4;
			h = box_h;

			height = (box_w * hp) / 100;

			render::draw_box(x - 1.f, y - w, x + box_w + 1, y, Color(global_esp_settings.health_bar_outline, get_player_alpha(global_esp_settings.health_bar_outline.a(), ctx.pl->ent_index())), 1.f, 0.f);
			render::draw_boxfilled(x, y - w + 1.f, x + box_w, y - 1.f, Color(global_esp_settings.health_bar_background, get_player_alpha(global_esp_settings.health_bar_background.a(), ctx.pl->ent_index())), 1.f, true);
			render::draw_boxfilled(x, y - w + 1.f, x + height, y - 1.f, main, 1.f, true);
		}

		if (global_esp_settings.health_in_bar && hp > 0 && hp < 100)
		{
			if (global_esp_settings.health_position == 0)
			{
				ImVec2 textSize = small_font->CalcTextSizeA(10, FLT_MAX, 0.0f, std::to_string(hp).c_str());
				render::draw_text(std::to_string(hp), ImVec2(x + 2, y - height - (textSize.y / 2)), 10, Color(255, 255, 255, get_player_alpha(255, ctx.pl->ent_index())), true, true, small_font);
			}
			if (global_esp_settings.health_position == 1)
			{
				ImVec2 textSize = small_font->CalcTextSizeA(10, FLT_MAX, 0.0f, std::to_string(hp).c_str());
				render::draw_text(std::to_string(hp), ImVec2(x - 2, y - height - (textSize.y / 2)), 12, Color(255, 255, 255, get_player_alpha(255, ctx.pl->ent_index())), true, true, small_font);
			}
			if (global_esp_settings.health_position == 2)
			{
				ImVec2 textSize = small_font->CalcTextSizeA(10, FLT_MAX, 0.0f, std::to_string(hp).c_str());
				render::draw_text(std::to_string(hp), ImVec2(x + height, y - 4), 12, Color(255, 255, 255, get_player_alpha(255, ctx.pl->ent_index())), true, true, small_font);
			}
			if (global_esp_settings.health_position == 3)
			{
				ImVec2 textSize = small_font->CalcTextSizeA(10, FLT_MAX, 0.0f, std::to_string(hp).c_str());
				render::draw_text(std::to_string(hp), ImVec2(x + height, y - 8), 12, Color(255, 255, 255, get_player_alpha(255, ctx.pl->ent_index())), true, true, small_font);
			}
		}
	}

	void player::draw_name()
	{
		player_info_t info = ctx.pl->get_player_info();

		auto sz = noto->CalcTextSizeA(12.f, FLT_MAX, 0.0f, info.szName);

		int offset = 2;

		bool armor_bar_work = global_esp_settings.armor_bar && ctx.pl->m_ArmorValue() > 0;

		if (global_esp_settings.health_position == 2 && global_esp_settings.health_bar)
		{
			if (ctx.pl->get_health() < 100 && global_esp_settings.health_in_bar)
			{
				offset += 2;
			}
			offset += 6;
		}

		if (global_esp_settings.armor_position == 2 && armor_bar_work && global_esp_settings.health_position == 2 && global_esp_settings.health_bar)
		{
			if ((global_esp_settings.armor_in_bar && ctx.pl->m_ArmorValue() < 100 && ctx.pl->m_ArmorValue() > 0) && (global_esp_settings.health_in_bar && ctx.pl->get_health() < 100))
				offset += 2;
		}

		if (global_esp_settings.armor_position == 2 && armor_bar_work)
		{
			if (ctx.pl->m_ArmorValue() < 100 && global_esp_settings.armor_in_bar)
			{
				offset += 2;
			}
			offset += 6;
		}

		const auto width = ctx.bbox.right - ctx.bbox.left;

		auto x = ctx.bbox.left + width / 2.f - sz.x / 2.f;


		render::draw_text(info.szName, ctx.bbox.left + width * 0.5f, ctx.bbox.top - sz.y - offset, 12.f, Color(global_esp_settings.name_color, get_player_alpha(global_esp_settings.name_color.a(), ctx.pl->ent_index())), true, true, noto);

	}

	void player::draw_armour()
	{
		auto  armour = ctx.pl->m_ArmorValue();
		if (armour <= 0)
			return;

		float box_w;
		float box_h;
		float off = 7;
		int height;

		float x;
		float y;
		float w;
		float h;

		if (global_esp_settings.health_position == global_esp_settings.armor_position && global_esp_settings.health_bar)
		{
			off += 6;

			if (ctx.pl->m_iHealth() < 100 && global_esp_settings.health_in_bar)
			{
				off += 2;
			}

			if (armour < 100 && global_esp_settings.armor_in_bar)
			{
				off += 2;
			}
		}

		Color main = Color(global_esp_settings.armor_bar_main, get_player_alpha(global_esp_settings.armor_bar_main.a(), ctx.pl->ent_index()));

		//"left" 0, "right" 1, "top" 2, "bottom" 3
		if (global_esp_settings.armor_position == 0)
		{
			box_w = (float)fabs(ctx.bbox.right - ctx.bbox.left);
			box_h = (float)fabs(ctx.bbox.bottom - ctx.bbox.top);
			height = (box_h * armour) / 100;

			x = ctx.bbox.left - off;
			y = ctx.bbox.bottom;
			w = 4;
			h = box_h;
			//Color(g_Options.color_esp_weapons_ammobar_outline, (int)flPlayerAlpha[ctx.pl->ent_index()])
			render::draw_box(x, y - h - 1, x + w, y + 1, Color(global_esp_settings.armor_bar_outline, get_player_alpha(global_esp_settings.armor_bar_outline.a(), ctx.pl->ent_index())), 1.f, 0.f);
			render::draw_boxfilled(x + 1, y - (int)box_h, x + w - 1, y, Color(global_esp_settings.armor_bar_background, get_player_alpha(global_esp_settings.armor_bar_background.a(), ctx.pl->ent_index())), 1.f, 0.f);
			render::draw_boxfilled(x + 1, y - height, x + w - 1, y, main, 1.f, 0.f);
		}
		if (global_esp_settings.armor_position == 1)
		{
			box_w = (float)fabs(ctx.bbox.right - ctx.bbox.left);
			box_h = (float)fabs(ctx.bbox.bottom - ctx.bbox.top);

			x = ctx.bbox.right + off;
			y = ctx.bbox.bottom;
			w = 4;
			h = box_h;

			height = (box_h * armour) / 100;

			//Render::Get().RenderBoxFilled(x - w + 1, (y - h), x - w + 2, (y - h) - 1, Color(g_Options.color_esp_healthbar_background, (int)flPlayerAlpha[ctx.pl->ent_index()]), 1.f, true);

			render::draw_box(x - w, (y - h) - 1, x, y + 1, Color(global_esp_settings.armor_bar_outline, get_player_alpha(global_esp_settings.armor_bar_outline.a(), ctx.pl->ent_index())), 1.f, true);
			render::draw_boxfilled(x - w + 1, (y - h), x - 1, y, Color(global_esp_settings.armor_bar_background, get_player_alpha(global_esp_settings.armor_bar_background.a(), ctx.pl->ent_index())), 1.f, true);
			render::draw_boxfilled(x - w + 1, (y - height), x - 1, y, main, 1.f, true);
		}

		if (global_esp_settings.armor_position == 2)
		{
			box_w = (float)fabs(ctx.bbox.right - ctx.bbox.left);
			box_h = 4.f;

			x = ctx.bbox.left;
			y = ctx.bbox.top - off;
			w = 4;
			h = box_h;

			height = (box_w * armour) / 100;

			render::draw_box(x - 1.f, y, x + box_w + 1, y + w, Color(global_esp_settings.armor_bar_outline, get_player_alpha(global_esp_settings.armor_bar_outline.a(), ctx.pl->ent_index())), 1.f, 0.f);
			render::draw_boxfilled(x, y + 1.f, x + box_w, y + w - 1.f, Color(global_esp_settings.armor_bar_background, get_player_alpha(global_esp_settings.armor_bar_background.a(), ctx.pl->ent_index())), 1.f, true);
			render::draw_boxfilled(x, y + 1.f, x + height, y + w - 1.f, main, 1.f, true);
		}
		if (global_esp_settings.armor_position == 3)
		{
			box_w = (float)fabs(ctx.bbox.right - ctx.bbox.left);
			box_h = 4.f;

			x = ctx.bbox.left;
			y = ctx.bbox.bottom + off;
			w = 4;
			h = box_h;

			height = (box_w * armour) / 100;

			render::draw_box(x - 1.f, y - w, x + box_w + 1, y, Color(global_esp_settings.armor_bar_outline, get_player_alpha(global_esp_settings.armor_bar_outline.a(), ctx.pl->ent_index())), 1.f, 0.f);
			render::draw_boxfilled(x, y - w + 1.f, x + box_w, y - 1.f, Color(global_esp_settings.armor_bar_background, get_player_alpha(global_esp_settings.armor_bar_background.a(), ctx.pl->ent_index())), 1.f, true);
			render::draw_boxfilled(x, y - w + 1.f, x + height, y - 1.f, main, 1.f, true);
		}

		if (global_esp_settings.armor_in_bar && armour > 0 && armour < 100)
		{
			if (global_esp_settings.armor_position == 0)
			{
				ImVec2 textSize = small_font->CalcTextSizeA(10, FLT_MAX, 0.0f, std::to_string(armour).c_str());
				render::draw_text(std::to_string(armour), ImVec2(x + 2, y - height - (textSize.y / 2)), 12, Color(255, 255, 255, get_player_alpha(255, ctx.pl->ent_index())), true, false, small_font);
			}
			if (global_esp_settings.armor_position == 1)
			{
				ImVec2 textSize = small_font->CalcTextSizeA(10, FLT_MAX, 0.0f, std::to_string(armour).c_str());
				render::draw_text(std::to_string(armour), ImVec2(x - 2, y - height - (textSize.y / 2)), 12, Color(255, 255, 255, get_player_alpha(255, ctx.pl->ent_index())), true, false, small_font);
			}
			if (global_esp_settings.armor_position == 2)
			{
				ImVec2 textSize = small_font->CalcTextSizeA(10, FLT_MAX, 0.0f, std::to_string(armour).c_str());
				render::draw_text(std::to_string(armour), ImVec2(x + height, y - 4), 12, Color(255, 255, 255, get_player_alpha(255, ctx.pl->ent_index())), true, false, small_font);
			}
			if (global_esp_settings.armor_position == 3)
			{
				ImVec2 textSize = small_font->CalcTextSizeA(10, FLT_MAX, 0.0f, std::to_string(armour).c_str());
				render::draw_text(std::to_string(armour), ImVec2(x + height, y - 8), 12, Color(255, 255, 255, get_player_alpha(255, ctx.pl->ent_index())), true, false, small_font);
			}
		}
	}

	void player::draw_flags()
	{
		std::vector<std::string> flags;

		bool planting = ctx.pl->m_hActiveWeapon()->get_item_definition_index() == ItemDefinitionIndex::WEAPON_C4 && ctx.pl->m_hActiveWeapon()->m_bStartedArming();

		if (global_esp_settings.flags_bomb_carrier && ctx.pl->has_c4())     flags.push_back("Bomb");//bomb
		if (global_esp_settings.flags_scoped && ctx.pl->m_bIsScoped())     flags.push_back("Scoped");//scoped
		if (global_esp_settings.flags_flashed && ctx.pl->is_flashed(1))  flags.push_back("Flashed");//flashed
		if (global_esp_settings.flags_defusing && ctx.pl->m_bIsDefusing()) flags.push_back("Defusing");//defusing
	    if (global_esp_settings.flags_planting && planting)                flags.push_back("planting");//planting

		//if (global_esp_settings.flags_helmet && ctx.pl->m_bHasHelmet())     flags.push_back("f"); //heelmet
		//if (global_esp_settings.flags_armor && ctx.pl->m_ArmorValue() > 0) flags.push_back("e"); //armor
		//if (global_esp_settings.flags_bomb_carrier && ctx.pl->has_c4())     flags.push_back("c");//bomb
		//if (global_esp_settings.flags_scoped && ctx.pl->m_bIsScoped())     flags.push_back("a");//scoped
		//if (global_esp_settings.flags_flashed && ctx.pl->is_flashed(1))  flags.push_back("d");//flashed
		//if (global_esp_settings.flags_defusing && ctx.pl->m_bIsDefusing()) flags.push_back("b");//defusing
		//if (global_esp_settings.flags_planting && planting)                flags.push_back("planting");//planting


		std::string armor = "";

		if (global_esp_settings.flags_helmet && ctx.pl->m_bHasHelmet())     armor += "H";
		if (global_esp_settings.flags_armor && ctx.pl->m_ArmorValue() > 0) armor += "k";

		if (armor != "")
			flags.push_back(armor);
		
		int x_offset = 4;

		bool armor_bar_work = global_esp_settings.armor_bar && ctx.pl->m_ArmorValue() > 0;

		if (global_esp_settings.health_position == 1 && global_esp_settings.health_bar)
		{
			if (ctx.pl->get_health() < 100 && global_esp_settings.health_in_bar)
			{
				x_offset += 2;
			}
			x_offset += 6;
		}

		if (global_esp_settings.armor_position == 1 && armor_bar_work && global_esp_settings.health_position == 1 && global_esp_settings.health_bar)
		{
			if ((global_esp_settings.armor_in_bar && ctx.pl->m_ArmorValue() < 100 && ctx.pl->m_ArmorValue() > 0) && (global_esp_settings.health_in_bar && ctx.pl->get_health() < 100))
				x_offset += 2;
		}

		if (global_esp_settings.armor_position == 1 && armor_bar_work)
		{
			if (ctx.pl->m_ArmorValue() < 100 && global_esp_settings.armor_in_bar)
			{
				if (global_esp_settings.health_position == 1 && global_esp_settings.health_bar && (!global_esp_settings.health_in_bar || ctx.pl->get_health() == 100))
					x_offset += 2;

				x_offset += 2;
			}

			x_offset += 6;
		}

		for (int i = 0; i < flags.size(); i++)
		{
			int cur_y = ctx.bbox.top + (i * 12);

			render::draw_text(flags[i], (int)ctx.bbox.right + x_offset, cur_y, 11.f, Color(255, 255, 255, get_player_alpha(255, ctx.pl->ent_index())), false, true, esp_font);
		}
	}

	void player::draw_weapon()
	{
		if (!g_local_player || !g_engine_client->is_connected())
			return;

		auto weapon = ctx.pl->m_hActiveWeapon();

		if (!weapon) return;

		auto weapon_data = weapon->get_cs_weapondata();
		if (!weapon_data)
			return;

		int ammo = weapon->m_iClip1();

		int global_offset = 7;

		bool armor_bar_work = global_esp_settings.armor_bar && ctx.pl->m_ArmorValue() > 0;

		if (global_esp_settings.health_position == 3 && global_esp_settings.health_bar)
		{
			global_offset += 6;
			if (global_esp_settings.health_in_bar && ctx.pl->get_health() < 100)
				global_offset += 2;
		}

		if (global_esp_settings.armor_position == 3 && armor_bar_work && global_esp_settings.health_position == 3 && global_esp_settings.health_bar)
		{
			if (global_esp_settings.health_bar)
			{
				if (ctx.pl->m_ArmorValue() < 100 && global_esp_settings.armor_in_bar)
				{
					global_offset += 2;
				}
			}
		}

		if (global_esp_settings.armor_position == 3 && armor_bar_work)
		{
			global_offset += 6;
			if (global_esp_settings.armor_in_bar && ctx.pl->m_ArmorValue() < 100)
				global_offset += 2;
		}

		auto animLayer = ctx.pl->get_anim_overlay(1);
		if (!animLayer->m_pOwner)
			return;

		auto activity = ctx.pl->get_sequence_activity(animLayer->m_nSequence);

		bool reload_anim = activity == 967 && animLayer->m_flWeight != 0.f;

		if (weapon->m_iClip1() != weapon_data->iMaxClip1 && weapon->m_iClip1() > 0 || reload_anim)
			global_offset += 2;

		auto text = weapon_data->szHudName + 7;
		auto size = small_font->CalcTextSizeA(14.f, FLT_MAX, 0.0f, text);

		float text_pos = 3.f;
		int w = ctx.bbox.right - ctx.bbox.left;
		//Render::Get().RenderText(g_WeaponIcons[weapon->GetItemDefinitionIndex()], ctx.feet_pos.x, ctx.feet_pos.y + text_pos, 12.f, Color::White, true, g_pDefaultFont);
		if (global_esp_settings.weapon_ammo)
		{

			if (weapon->get_item_definition_index() != ItemDefinitionIndex::WEAPON_FLASHBANG &&
				weapon->get_item_definition_index() != ItemDefinitionIndex::WEAPON_SMOKEGRENADE &&
				weapon->get_item_definition_index() != ItemDefinitionIndex::WEAPON_MOLOTOV &&
				weapon->get_item_definition_index() != ItemDefinitionIndex::WEAPON_INCGRENADE &&
				weapon->get_item_definition_index() != ItemDefinitionIndex::WEAPON_DECOY &&
				weapon->get_item_definition_index() != ItemDefinitionIndex::WEAPON_HEGRENADE &&
				weapon->get_item_definition_index() != ItemDefinitionIndex::WEAPON_C4 &&
				weapon->get_item_definition_index() != ItemDefinitionIndex::WEAPON_KNIFE)
			{


				float box_w = (float)fabs(ctx.bbox.right - ctx.bbox.left);

				float width = 0.f;//(((box_w * weapon->m_iClip1()) / weapon->GetCSWeaponData()->iMaxClip1));

				if (reload_anim)
				{
					float cycle = animLayer->m_flCycle; // 1 = finished 0 = just started
					width = (((box_w * cycle) / 1.f));
				}
				else
				{
					width = (((box_w * weapon->m_iClip1()) / weapon_data->iMaxClip1));
					//weapon->CurMaxReloadTime = 0.f;
				}


				//Render::Get().RenderBox(ctx.bbox.left - 1, ctx.bbox.bottom + 3 + global_offset, ctx.bbox.right + 1, ctx.bbox.bottom + 7 + global_offset, Color(global_esp_settings.ammo_bar_outline, (int)flPlayerAlpha[ctx.pl->ent_index()]));
				//Render::Get().RenderBoxFilled(ctx.bbox.left, ctx.bbox.bottom + 4 + global_offset, ctx.bbox.right, ctx.bbox.bottom + 6 + global_offset, Color(global_esp_settings.ammo_bar_background, (int)flPlayerAlpha[ctx.pl->ent_index()]));
				//Render::Get().RenderBoxFilled(ctx.bbox.left, ctx.bbox.bottom + 4 + global_offset, ctx.bbox.left + (int)width, ctx.bbox.bottom + 6 + global_offset, Color(global_esp_settings.ammo_bar_main, (int)flPlayerAlpha[ctx.pl->ent_index()]));

				box_w = (float)fabs(ctx.bbox.right - ctx.bbox.left);

				float x = ctx.bbox.left;
				float y = ctx.bbox.bottom + global_offset;
				float w = 4;

				render::draw_box(x - 1.f, y - w, x + box_w + 1, y, Color(global_esp_settings.ammo_bar_outline, get_player_alpha(global_esp_settings.ammo_bar_outline.a(), ctx.pl->ent_index())));
				render::draw_boxfilled(x, y - w + 1.f, x + box_w, y - 1.f, Color(global_esp_settings.ammo_bar_background, get_player_alpha(global_esp_settings.ammo_bar_background.a(), ctx.pl->ent_index())));
				render::draw_boxfilled(x, y - w + 1.f, x + width, y - 1.f, Color(global_esp_settings.ammo_bar_main, get_player_alpha(global_esp_settings.ammo_bar_main.a(), ctx.pl->ent_index())));

				text_pos = global_offset;

				int reload_percentage = (100 * animLayer->m_flCycle) / 1.f;
				if (weapon->m_iClip1() != weapon_data->iMaxClip1 && weapon->m_iClip1() > 0 && activity != 967)
				{
					render::draw_text(std::to_string(weapon->m_iClip1()), ImVec2(ctx.bbox.left + (int)width, ctx.bbox.bottom + global_offset - w - 4), 9.f, Color(255, 255, 255, get_player_alpha(255, ctx.pl->ent_index())), true, true, esp_font);
					text_pos = global_offset + 2;
				}
				else if (activity == 967 && reload_percentage != 99)
				{
					render::draw_text(std::to_string(reload_percentage) + "%", ImVec2(ctx.bbox.left + (int)width, ctx.bbox.bottom + global_offset - w - 4), 12.f, Color(255, 255, 255, get_player_alpha(255, ctx.pl->ent_index())), true, true, esp_font);
					text_pos = global_offset + 2;
				}

			}
			else
			{
				if (global_offset == 7)
					text_pos = 3.f;
				else
					text_pos = global_offset - 7;
			}
		}
		else
		{
			if (global_offset == 7)
				text_pos = 3.f;
			else
				text_pos = global_offset - 7;
		}

		//render::draw_box(x, y - h - 1, x + w, y + 1, Color(global_esp_settings.armor_bar_outline, get_player_alpha(global_esp_settings.armor_bar_outline.a(), ctx.pl->ent_index())), 1.f, 0.f);


		if (global_esp_settings.weapon_type == 0)
		{
		render::draw_text(fixed_names[weapon->get_item_definition_index()], ImVec2((ctx.bbox.left + w * 0.5f), ctx.bbox.bottom + text_pos), 10, Color(global_esp_settings.weapon_name, get_player_alpha(global_esp_settings.weapon_name.a(), ctx.pl->ent_index())), true, true, esp_font);
		}

		if (global_esp_settings.weapon_type == 1)
		{
			render::draw_text(weapons_icons1[weapon->get_item_definition_index()], ImVec2((ctx.bbox.left + w * 0.5f), ctx.bbox.bottom + text_pos), 12, Color(global_esp_settings.weapon_name_icon, get_player_alpha(global_esp_settings.weapon_name_icon.a(), ctx.pl->ent_index())), true, false, weapo_icon);
		}


		if (global_esp_settings.weaico)
		{
			if (global_esp_settings.weapon_type == 0)
			{
				render::draw_text(weapons_icons1[weapon->get_item_definition_index()], ImVec2((ctx.bbox.left + w * 0.5f), ctx.bbox.bottom + text_pos + 12), 11, Color(global_esp_settings.weapon_name_icon, get_player_alpha(global_esp_settings.weapon_name_icon.a(), ctx.pl->ent_index())), true, false, weapo_icon);
			}
		}
	}
	
	void DrawKeyPresses() {

		int w, h;
		g_engine_client->get_screen_size(w, h);
		if (GetAsyncKeyState(int('W')))
			render::draw_text(" W ", ImVec2(w / 2, h - h / 5), 20.f, Color(255, 255, 255, 255), true, false, fontstrokes);
		else
			render::draw_text("-", ImVec2(w / 2, h - h / 5), 20.f, Color(255, 255, 255, 255), true, false, fontstrokes);

		if (GetAsyncKeyState(int('S')))
			render::draw_text(" S", ImVec2(w / 2, h - h / 5 + 15), 20.f, Color(255, 255, 255, 255), true, false, fontstrokes);
		else
			render::draw_text("-", ImVec2(w / 2, h - h / 5 + 15), 20.f, Color(255, 255, 255, 255), true, false, fontstrokes);

		if (GetAsyncKeyState(int('A')))
			render::draw_text("A ", ImVec2(w / 2 - 15, h - h / 5), 20.f, Color(255, 255, 255, 255), true, false, fontstrokes);
		else
			render::draw_text("-", ImVec2(w / 2 - 15, h - h / 5), 20.f, Color(255, 255, 255, 255), true, false, fontstrokes);

		if (GetAsyncKeyState(int('D')))
			render::draw_text("D", ImVec2(w / 2 + 15, h - h / 5), 20.f, Color(255, 255, 255, 255), true, false, fontstrokes);
		else
			render::draw_text("-", ImVec2(w / 2 + 15, h - h / 5), 20.f, Color(255, 255, 255, 255), true, false, fontstrokes);
	}

	void draw_granades(c_base_entity* entity)
	{
		if (!g_local_player)
			return;

		std::string name;
		Color clr;

		const model_t* model = entity->get_model();

		if (entity->get_client_class())
		{
			if (model)
			{
				studiohdr_t* hdr = g_mdl_info->get_studiomodel(model);
				if (hdr)
				{
					std::string hdrName = hdr->szName;
					if (hdrName.find("thrown") != std::string::npos || hdrName.find("dropped") != std::string::npos)
					{
						if (hdrName.find("flashbang") != std::string::npos)
						{
							name = "flash";
							clr = settings::visuals::grenades::color_flash;
						}
						else if (hdrName.find("fraggrenade") != std::string::npos)
						{
							name = "frag";
							clr = settings::visuals::grenades::color_frag;
						}
						else if (hdrName.find("molotov") != std::string::npos)
						{
							name = "molotov";
							clr = settings::visuals::grenades::color_molotov;
						}
						else if (hdrName.find("incendiarygrenade") != std::string::npos)
						{
							name = "incendiary";
							clr = settings::visuals::grenades::color_molotov;
						}
						else if (hdrName.find("decoy") != std::string::npos)
						{
							name = "decoy";
							clr = settings::visuals::grenades::color_decoy;
						}
						else if (hdrName.find("smoke") != std::string::npos)
						{
							name = "smoke";
							clr = settings::visuals::grenades::color_smoke;
						}
					}
				}

				if (!name.empty())
				{
					Vector pos;
					if (math::world_to_screen(entity->m_vecOrigin(), pos))
					{
						render::draw_text(name, ImVec2(pos.x, pos.y), 11.f, clr, true, false, small_font);
					}
				}
			}
		}
	}

	void RenderSounds() {
		UpdateSounds();

		auto Add3DCircle = [](const Vector& position, Color color, float radius) {
			float precision = 160.0f;

			const float step = DirectX::XM_2PI / precision;

			for (float a = 0.f; a < DirectX::XM_2PI; a += step) {
				Vector start(radius * cosf(a) + position.x, radius * sinf(a) + position.y, position.z);
				Vector end(radius * cosf(a + step) + position.x, radius * sinf(a + step) + position.y, position.z);

				Vector start2d, end2d;
				if (!math::world_to_screen(start, start2d) || !math::world_to_screen(end, end2d))
					return;

				render::draw_line(start2d.x, start2d.y, end2d.x, end2d.y, color);
			}
		};


		for (auto& [entIndex, sound] : m_Sounds) {
			if (sound.empty())
				continue;

			for (auto& info : sound) {
				if (info.soundTime + settings::misc::soundtime < g_global_vars->realtime)
					info.alpha -= g_global_vars->frametime;

				if (info.alpha <= 0.0f)
					continue;

				float deltaTime = g_global_vars->realtime - info.soundTime;

				auto factor = deltaTime / settings::misc::soundtime;
				if (factor > 1.0f)
					factor = 1.0f;

				float radius = settings::misc::soundradio * factor;
				Color color = Color(settings::misc::soundcolor.r() / 255.f, settings::misc::soundcolor.g() / 255.f, settings::misc::soundcolor.b() / 255.f);

				Add3DCircle(info.soundPos, color, radius);
			}

			while (!sound.empty()) {
				auto& back = sound.back();
				if (back.alpha <= 0.0f)
					sound.pop_back();
				else
					break;
			}
		}
	}

	void UpdateSounds() {

		sounds.RemoveAll();
		g_engine_sound->get_active_sounds(sounds);
		if (sounds.Count() < 1)
			return;

		Vector eye_pos = g_local_player->get_eye_pos();
		for (int i = 0; i < sounds.Count(); ++i) {
			snd_info_t& sound = sounds.Element(i);
			if (sound.m_nSoundSource < 1)
				continue;

			c_base_player* player = c_base_player::get_player_by_index(sound.m_nSoundSource);
			if (!player)
				continue;

			if (player->m_hOwnerEntity().is_valid() && player->is_weapon()) {
				player = (c_base_player*)player->m_hOwnerEntity().get();
			}

			if (!player->is_player() || !player->is_alive())
				continue;

			if ((c_base_player*)g_local_player == player || (settings::visuals::chams_enemy && g_local_player->m_iTeamNum() == player->m_iTeamNum()))
				continue;

			if (player->m_vecOrigin().DistTo(g_local_player->m_vecOrigin()) > 900)
				continue;

			auto& player_sound = m_Sounds[player->ent_index()];
			if (player_sound.size() > 0) {
				bool should_break = false;
				for (const auto& snd : player_sound) {
					if (snd.guid == sound.m_nGuid) {
						should_break = true;
						break;
					}
				}

				if (should_break)
					continue;
			}

			sound_info_t& snd = player_sound.emplace_back();
			snd.guid = sound.m_nGuid;
			snd.soundPos = *sound.m_pOrigin;
			snd.soundTime = g_global_vars->realtime;
			snd.alpha = 1.0f;
		}
	}

	void events(i_gameevent* event)
	{
		if (strstr(event->get_name(), "inferno_startburn"))
		{
			Vector position(event->get_float("x"), event->get_float("y"), event->get_float("z"));
			molotov_info_t temp = { position, g_global_vars->curtime + 7.f };
			molotov_info.emplace_back(temp);
		}

		if (strstr(event->get_name(), "smokegrenade_detonate"))
		{
			Vector position(event->get_float("x"), event->get_float("y"), event->get_float("z"));
			smoke_info_t temp = { position, g_global_vars->curtime + 18.f };
			smoke_info.emplace_back(temp);
		}

		if (strstr(event->get_name(), "round_end") || strstr(event->get_name(), "round_prestart") || strstr(event->get_name(), "round_freeze_end"))
		{
			for (int i = 0; i < molotov_info.size(); i++)
				molotov_info.erase(molotov_info.begin() + i);

			for (int i = 0; i < smoke_info.size(); i++)
				smoke_info.erase(smoke_info.begin() + i);
		}

		if (!strcmp(event->get_name(), "player_hurt"))
		{
			c_base_player* hurt = (c_base_player*)g_entity_list->get_client_entity(g_engine_client->get_player_for_user_id(event->get_int("userid")));
			c_base_player* attacker = (c_base_player*)g_entity_list->get_client_entity(g_engine_client->get_player_for_user_id(event->get_int("attacker")));
			if (attacker == g_local_player)
			{
				//flHurtTime = g_global_vars->realtime;
			}

			bool hurt_dead = event->get_int("health") == 0;

			if (hurt != g_local_player && attacker == g_local_player)
			{
				if (settings::visuals::hitbox::enable && settings::visuals::hitbox::show_type == 0)
				{
					if (g_local_player && hurt)
					{
						capsule_hitbox(hurt, settings::visuals::hitbox::color, settings::visuals::hitbox::show_time);
					}
				}
				else if (settings::visuals::hitbox::enable && settings::visuals::hitbox::show_type == 1)
				{
					if (g_local_player && hurt && hurt_dead)
						capsule_hitbox(hurt, settings::visuals::hitbox::color, settings::visuals::hitbox::show_time);

				}
			}
		}
	}

	void draw_damage_indicator()
	{
		for (int i = 0; i < globals::damage_indicator::indicators.size(); i++)
		{
			auto& cur_indicator = globals::damage_indicator::indicators[i];

			if (cur_indicator.erase_time < g_global_vars->curtime)
			{
				globals::damage_indicator::indicators.erase(globals::damage_indicator::indicators.begin() + i);
				continue;
			}

			if (cur_indicator.erase_time - 1.f < g_global_vars->curtime)
			{
				cur_indicator.col._CColor[3] = utils::lerp(cur_indicator.col._CColor[3], 0, 0.1f);
			}

			if (!cur_indicator.initialized)
			{
				cur_indicator.position = cur_indicator.player->get_hitbox_pos(bone(cur_indicator.hit_box));
				if (!cur_indicator.player->is_alive())
				{
					cur_indicator.position.z -= 39.f;

					cur_indicator.position.z += settings::visuals::damage_indicator::offset_if_kill;
				}
				else
				{
					cur_indicator.position.z += settings::visuals::damage_indicator::offset_hit;
				}
				cur_indicator.end_position = cur_indicator.position + Vector(utils::random(-settings::visuals::damage_indicator::range_offset_x, settings::visuals::damage_indicator::range_offset_x), utils::random(-settings::visuals::damage_indicator::range_offset_x, settings::visuals::damage_indicator::range_offset_x), settings::visuals::damage_indicator::max_pos_y);

				cur_indicator.initialized = true;
			}

			if (cur_indicator.initialized)
			{
				cur_indicator.position.z = utils::lerp(cur_indicator.position.z, cur_indicator.end_position.z, settings::visuals::damage_indicator::speed);
				cur_indicator.position.x = utils::lerp(cur_indicator.position.x, cur_indicator.end_position.x, settings::visuals::damage_indicator::speed);
				cur_indicator.position.y = utils::lerp(cur_indicator.position.y, cur_indicator.end_position.y, settings::visuals::damage_indicator::speed);

				cur_indicator.last_update = g_global_vars->curtime;
			}

			Vector rend_pos;

			if (math::world_to_screen(cur_indicator.position, rend_pos))
			{
				render::draw_text("-" + std::to_string(cur_indicator.damage), rend_pos.x, rend_pos.y, settings::visuals::damage_indicator::text_size, cur_indicator.col, true, small_font);
			}
		}
	}

	void draw_weapons(c_base_combat_weapon* ent)
	{
		auto clean_item_name = [](const char* name) -> const char* {
			if (name[0] == 'C')
				name++;

			auto start = strstr(name, "Weapon");
			if (start != nullptr)
				name = start + 6;

			return name;
		};

		if (ent->m_hOwnerEntity().is_valid())
			return;

		auto bbox = get_bbox(ent);

		if (bbox.right == 0 || bbox.bottom == 0)
			return;

		if (settings::visuals::dropped_weapon::box)
		{
			render::draw_box(RECT{ bbox.left - 1, bbox.top + 1, bbox.right + 1, bbox.bottom - 1 }, Color(0, 0, 0, 255));
			render::draw_box(bbox, settings::visuals::dropped_weapon::box_color);
			render::draw_box(RECT{ bbox.left + 1, bbox.top - 1, bbox.right - 1, bbox.bottom + 1 }, Color(0, 0, 0, 255));
		}

		auto name = clean_item_name(ent->get_client_class()->m_pNetworkName);
		if (!name)
			return;

		int w = bbox.right - bbox.left;

		int offset = 3;

		if (settings::visuals::dropped_weapon::ammo_bar)
		{
			if (ent->get_item_definition_index() != ItemDefinitionIndex::WEAPON_FLASHBANG &&
				ent->get_item_definition_index() != ItemDefinitionIndex::WEAPON_SMOKEGRENADE &&
				ent->get_item_definition_index() != ItemDefinitionIndex::WEAPON_MOLOTOV &&
				ent->get_item_definition_index() != ItemDefinitionIndex::WEAPON_INCGRENADE &&
				ent->get_item_definition_index() != ItemDefinitionIndex::WEAPON_DECOY &&
				ent->get_item_definition_index() != ItemDefinitionIndex::WEAPON_HEGRENADE &&
				ent->get_item_definition_index() != ItemDefinitionIndex::WEAPON_C4)
			{
				float box_w = (float)fabs(bbox.right - bbox.left);

				auto width = (((box_w * ent->m_iClip1()) / ent->get_cs_weapondata()->iMaxClip1));

				render::draw_box(bbox.left - 1, bbox.top + 3, bbox.right + 1, bbox.top + 7, settings::visuals::dropped_weapon::bar_outline);

				render::draw_boxfilled(bbox.left, bbox.top + 4, bbox.right, bbox.top + 6, settings::visuals::dropped_weapon::bar_background);

				render::draw_boxfilled(bbox.left, bbox.top + 4, bbox.left + (int)width, bbox.top + 6, settings::visuals::dropped_weapon::bar_main);
				if (ent->m_iClip1() != ent->get_cs_weapondata()->iMaxClip1 && ent->m_iClip1() > 0)
				{
					render::draw_text(std::to_string(ent->m_iClip1()), ImVec2(bbox.left + (int)width, bbox.top + 2), 10.f, Color(255, 255, 255, 255), true, true, esp_font);
					offset += 5;
				}
				offset += 4;
			}
		}
		if (settings::visuals::dropped_weapon::enable_type == 0)
		{
			render::draw_text(fixed_names[ent->get_item_definition_index()], ImVec2((bbox.left + w * 0.5f), bbox.top + offset), 11, Color(settings::visuals::dropped_weapon::droppednamecolor), true, true, esp_font);
		}
		if (settings::visuals::dropped_weapon::enable_type == 1)
		{
			render::draw_text(weapons_icons1[ent->get_item_definition_index()], ImVec2((bbox.left + w * 0.5f), bbox.top + offset), 12, Color(settings::visuals::dropped_weapon::droppediconcolor), true, false, weapo_icon);
		}

		if (settings::visuals::dropped_weapon::weacoi)
		{
			if (settings::visuals::dropped_weapon::enable_type == 0)
			{
				render::draw_text(weapons_icons1[ent->get_item_definition_index()], ImVec2((bbox.left + w * 0.5f), bbox.top + 20), 12, Color(settings::visuals::dropped_weapon::droppediconcolor), true, false, weapo_icon);
			}
		}
		
	}

	void draw_defusekit(c_base_entity* ent)
	{
		if (ent->m_hOwnerEntity().is_valid())
			return;

		auto bbox = get_bbox(ent);

		if (bbox.right == 0 || bbox.bottom == 0)
			return;

		auto name = "defuse kit";
		auto sz = small_font->CalcTextSizeA(14.f, FLT_MAX, 0.0f, name);
		int w = bbox.right - bbox.left;
		render::draw_text(name, ImVec2((bbox.left + w * 0.5f) - sz.x * 0.5f, bbox.bottom + 1), 14.f, Color(255, 255, 255, 255), small_font);
	}

    void draw_window_c4(c_base_entity* ent)  
{
	if (!ent)
		return;

	float bombTimer = ent->m_flC4Blow() - g_global_vars->curtime;

	int x;
	int y;

	g_engine_client->get_screen_size(x, y);

	int windowX = 0;
	int windowY = y - 430;

	int windowSizeX = 86;
	static int windowSizeY = 15; // 3 item - 35 | 2 item - 25 | 1 item - 15
	const auto bomb = get_bomb();
	if (!bomb)
		return;

	if (bombTimer < 0)
		return;


	render::draw_boxfilled(windowX, windowY, windowX + windowSizeX, windowY + windowSizeY, ImColor(0, 0, 0, 100));
	render::draw_boxfilled(windowX, windowY, windowX + 2, windowY + windowSizeY, settings::misc::menu_color);

	render::draw_text("bomb: ", ImVec2(windowX + 4, windowY + 2), 11, Color(255, 255, 255, 255), false, false, small_font);
	ImVec2 textSize_bomb = small_font_2->CalcTextSizeA(11, FLT_MAX, 0.0f, "bomb: ");
	std::string bomb_timer_text;
	//Math::sprintfs(bomb_timer_text, "%4.3f", bombTimer);

	char buff[228];

	snprintf(buff, sizeof(buff), "%4.3f", bombTimer);

	bomb_timer_text = buff;

	render::draw_text(bombTimer >= 0 ? bomb_timer_text : "0", ImVec2(windowX + 4 + textSize_bomb.x, windowY + 2), 11, Color(255, 255, 255, 255), false, false, small_font_2);

	ImVec2 textSize_defuse = small_font_2->CalcTextSizeA(11, FLT_MAX, 0.0f, "defuse: ");
	float time = get_defuse_time(bomb);

	std::string defuse_timer_text;
	//Math::sprintfs(defuse_timer_text, "%4.3f", time);
	char buff_2[228];

	snprintf(buff_2, sizeof(buff_2), "%4.3f", time);

	defuse_timer_text = buff_2;

	const auto bomb_1 = get_bomb_player();

	if (!bomb_1)
		return;

	float flArmor = g_local_player->m_ArmorValue();
	float flDistance = g_local_player->get_eye_pos().DistTo(bomb_1->get_abs_origin());

	float a = 450.7f;
	float b = 75.68f;
	float c = 789.2f;
	float d = ((flDistance - b) / c);
	float flDamage = a * exp(-d * d);

	float flDmg = flDamage;

	if (flArmor > 0)
	{
		float flNew = flDmg * 0.5f;
		float flArmor = (flDmg - flNew) * 0.5f;

		if (flArmor > static_cast<float>(flArmor))
		{
			flArmor = static_cast<float>(flArmor) * (1.f / 0.5f);
			flNew = flDmg - flArmor;
		}

		flDamage = flNew;
	}

	std::string damage;
	char buff_3[228];

	snprintf(buff_3, sizeof(buff_3), "%i", (int)flDamage);

	damage = buff_3;

	ImVec2 textSize_damage = small_font_2->CalcTextSizeA(11, FLT_MAX, 0.0f, "damage: ");

	if (get_defuse_time(bomb) > 0)
	{
		render::draw_text("defuse: ", ImVec2(windowX + 4, windowY + 12), 11, Color(255, 255, 255, 255), false, false, small_font_2);
		if (bombTimer < time)
		{
			render::draw_text(defuse_timer_text, ImVec2(windowX + 4 + textSize_defuse.x, windowY + 12), 11, Color(255, 50, 50, 255), false, false, small_font_2);
		}
		else if (bombTimer > time)
		{
			render::draw_text(defuse_timer_text, ImVec2(windowX + 4 + textSize_defuse.x, windowY + 12), 11, Color(50, 255, 50, 255), false, false, small_font_2);
		}
		if (flDamage > 1 && bombTimer >= 0)
		{
			render::draw_text("damage: ", ImVec2(windowX + 4, windowY + 22), 11, Color(255, 255, 255, 255), false, 0, small_font_2);
			render::draw_text(flDamage < 100 ? damage : "you dead", ImVec2(windowX + 4 + textSize_damage.x, windowY + 22), 11, Color(255, 255, 255, 255), false, false, small_font_2);
			windowSizeY = 35;
		}
		else
		{
			windowSizeY = 25;
		}

		float box_w = (float)fabs(0 - windowSizeX);

		float max_time;

		float width;

		if (globals::misc::bomb_defusing_with_kits)
		{
			width = (((box_w * time) / 5.f));
		}
		else
		{
			width = (((box_w * time) / 10.f));
		}

		render::draw_boxfilled(windowX, windowY + windowSizeY + 2, windowX + (int)width, windowY + windowSizeY + 4, Color(50, 50, 255, 255));

	}
	else if (get_defuse_time(bomb) <= 0)
	{
		if (flDamage > 1 && bombTimer >= 0)
		{
			render::draw_text("damage: ", ImVec2(windowX + 4, windowY + 12), 11, Color(255, 255, 255, 255), false, false, small_font_2);
			render::draw_text(flDamage < 100 ? damage : "you dead", ImVec2(windowX + 4 + textSize_damage.x, windowY + 12), 11, Color(255, 255, 255, 255), false, false, small_font_2);
			//render::draw_text(flDamage < 100 ? damage : "you dead", ImVec2(windowX + 4 + textSize_damage.x, windowY + 12), 11, Color(255, 255, 255, 255), false, false, small_font_2);
			windowSizeY = 25;
		}
		else
		{
			windowSizeY = 15;
		}
	}
	
	float box_w = (float)fabs(0 - windowSizeX);

	auto width = (((box_w * bombTimer) / 40.f));
	render::draw_boxfilled(windowX, windowY + windowSizeY, windowX + (int)width, windowY + windowSizeY + 2, Color(255, 100, 100, 255));

}

	void draw_items(c_base_entity* ent)
	{
		std::string itemstr = "Undefined";
		const model_t* itemModel = ent->get_model();
		if (!itemModel)
			return;
		studiohdr_t* hdr = g_mdl_info->get_studiomodel(itemModel);
		if (!hdr)
			return;
		itemstr = hdr->szName;
		if (itemstr.find("case_pistol") != std::string::npos)
			itemstr = "Pistol Case";
		else if (itemstr.find("case_light_weapon") != std::string::npos)
			itemstr = "Light Case";
		else if (itemstr.find("case_heavy_weapon") != std::string::npos)
			itemstr = "Heavy Case";
		else if (itemstr.find("case_explosive") != std::string::npos)
			itemstr = "Explosive Case";
		else if (itemstr.find("case_tools") != std::string::npos)
			itemstr = "Tools Case";
		else if (itemstr.find("random") != std::string::npos)
			itemstr = "Airdrop";
		else if (itemstr.find("dz_armor_helmet") != std::string::npos)
			itemstr = "Full Armor";
		else if (itemstr.find("dz_helmet") != std::string::npos)
			itemstr = "Helmet";
		else if (itemstr.find("dz_armor") != std::string::npos)
			itemstr = "Armor";
		else if (itemstr.find("upgrade_tablet") != std::string::npos)
			itemstr = "Tablet Upgrade";
		else if (itemstr.find("briefcase") != std::string::npos)
			itemstr = "Briefcase";
		else if (itemstr.find("parachutepack") != std::string::npos)
			itemstr = "Parachute";
		else if (itemstr.find("dufflebag") != std::string::npos)
			itemstr = "Cash Dufflebag";
		else if (itemstr.find("ammobox") != std::string::npos)
			itemstr = "Ammobox";
		else if (itemstr.find("dronegun") != std::string::npos)
			itemstr = "Turrel";

		auto bbox = get_bbox(ent);
		if (bbox.right == 0 || bbox.bottom == 0)
			return;
		auto sz = small_font->CalcTextSizeA(14.f, FLT_MAX, 0.0f, itemstr.c_str());
		int w = bbox.right - bbox.left;

		//Render::Get().RenderBox(bbox, g_Options.color_esp_item);
		render::draw_text(itemstr, ImVec2((bbox.left + w * 0.5f) - sz.x * 0.5f, bbox.bottom + 1), 14.f, Color(255, 255, 255), small_font);
	}

	void third_person()
	{
		if (!g_local_player)
			return;

		if (settings::misc::third_person::enable && settings::misc::third_person::bind.enable)
		{
			if (g_local_player->is_alive())
			{
				if (!g_input->m_fCameraInThirdPerson)
				{
					g_input->m_fCameraInThirdPerson = true;
				}

				float dist = settings::misc::third_person::dist;

				QAngle* view = g_local_player->get_vangles();
				trace_t tr;
				ray_t ray;

				Vector desiredCamOffset = Vector(cos(DEG2RAD(view->yaw)) * dist,
					sin(DEG2RAD(view->yaw)) * dist,
					sin(DEG2RAD(-view->pitch)) * dist
				);

				//cast a ray from the Current camera Origin to the Desired 3rd person Camera origin
				ray.init(g_local_player->get_eye_pos(), (g_local_player->get_eye_pos() - desiredCamOffset));
				c_tracefilter traceFilter;
				traceFilter.pSkip = g_local_player;
				g_engine_trace->trace_ray(ray, MASK_SHOT, &traceFilter, &tr);

				Vector diff = g_local_player->get_eye_pos() - tr.endpos;

				float distance2D = sqrt(abs(diff.x * diff.x) + abs(diff.y * diff.y));// Pythagorean

				bool horOK = distance2D > (dist - 2.0f);
				bool vertOK = (abs(diff.z) - abs(desiredCamOffset.z) < 3.0f);

				float cameraDistance;

				if (horOK && vertOK)  // If we are clear of obstacles
				{
					cameraDistance = dist; // go ahead and set the distance to the setting
				}
				else {
					if (vertOK) // if the Vertical Axis is OK
					{
						cameraDistance = distance2D * 0.95f;
					}
					else// otherwise we need to move closer to not go into the floor/ceiling
					{
						cameraDistance = abs(diff.z) * 0.95f;
					}
				}
				g_input->m_fCameraInThirdPerson = true;

				g_input->m_vecCameraOffset.z = cameraDistance;

				setted_person = false;
			}
			else
			{
				if (g_local_player->m_hObserverTarget() != nullptr)
				{
					if (g_local_player->m_observer_mode() == observer_mode::OBS_MODE_IN_EYE)
					{
						bec_person = g_local_player->m_observer_mode();
						setted_person = true;
					}

					if (setted_person)
						g_local_player->m_observer_mode() = observer_mode::OBS_MODE_CHASE;
				}

				g_input->m_fCameraInThirdPerson = false;
			}
		}
		else
		{
			if (!g_local_player->is_alive())
			{
				if (setted_person)
					g_local_player->m_observer_mode() = bec_person;

				setted_person = false;
			}
			g_input->m_fCameraInThirdPerson = false;
		}
	}
	
	void SetThirdpersonAngles()
	{

		QAngle LastAngle = QAngle(0, 0, 0);

		if (utils::is_ingame() && g_local_player)
		{
			if (g_local_player->is_alive() && g_input->m_fCameraInThirdPerson)
				g_local_player->set_abs_angles2(Vector(0, g_local_player->get_player_anim_state()->m_flEyeYaw, 0)); //around 90% accurate
		}
	}

	void nightmode()
	{
		std::string fallback_skybox = "";
		static int OldSky = 0;

		auto LoadNamedSky = reinterpret_cast<void(__fastcall*)(const char*)>(utils::pattern_scan(GetModuleHandleA("engine.dll"), "55 8B EC 81 EC ? ? ? ? 56 57 8B F9 C7 45"));


		static bool OldNightmode;
		if (!g_engine_client->is_connected() || !g_engine_client->is_ingame() || !g_local_player || !g_local_player->is_alive())
		{
			fallback_skybox = "";
			OldNightmode = false;
			OldSky = 0;
			return;
		}

		static convar* r_DrawSpecificStaticProp;
		if (OldNightmode != settings::misc::nightmode)
		{

			r_DrawSpecificStaticProp = g_cvar->find_var("r_DrawSpecificStaticProp");
			r_DrawSpecificStaticProp->set_value(0);

			for (material_handle_t i = g_mat_system->first_material(); i != g_mat_system->invalid_material(); i = g_mat_system->next_material(i))
			{
				i_material* pMaterial = g_mat_system->get_material(i);
				if (!pMaterial)
					continue;
				if (strstr(pMaterial->get_texture_group_name(), "World") || strstr(pMaterial->get_texture_group_name(), "StaticProp"))
				{
					if (settings::misc::nightmode) {
						LoadNamedSky("sky_csgo_night02");

						if (strstr(pMaterial->get_texture_group_name(), "StaticProp"))
							pMaterial->color_modulate(0.11f, 0.11f, 0.11f);
						else
							pMaterial->color_modulate(0.05f, 0.05f, 0.05f);
					}
					else {
						LoadNamedSky("sky_cs15_daylight04_hdr");
						pMaterial->color_modulate(1.0f, 1.0f, 1.0f);
					}
				}
			}
			OldNightmode = settings::misc::nightmode;
		}

	}

	void draw_bullet_tracer(Vector start, Vector end, Color col)
	{
		if (!g_local_player || !settings::visuals::bullet_impact::line::enable)
		{
			return;
		}

		if (!start.IsValid() || !end.IsValid() || start.DistTo(end) < 0.1f)
			return;

		beam_info_t beamInfo;
		beamInfo.m_nType = TE_BEAMPOINTS;
		beamInfo.m_pszModelName = "sprites/physbeam.vmt";
		beamInfo.m_nModelIndex = -1;
		beamInfo.m_flHaloScale = 0.0f;
		beamInfo.m_flLife = settings::visuals::bullet_impact::line::time;
		beamInfo.m_flWidth = 0.5f;
		beamInfo.m_flEndWidth = 0.5f;
		beamInfo.m_flFadeLength = 0.0f;
		beamInfo.m_flAmplitude = 2.0f;
		beamInfo.m_flBrightness = col.a();
		beamInfo.m_flSpeed = 0.2f;
		beamInfo.m_nStartFrame = 0;
		beamInfo.m_flFrameRate = 0.f;
		beamInfo.m_flRed = col.r();
		beamInfo.m_flGreen = col.g();
		beamInfo.m_flBlue = col.b();
		beamInfo.m_nSegments = 2;
		beamInfo.m_bRenderable = true;
		beamInfo.m_nFlags = 0;//FBEAM_FADEIN | FBEAM_FADEOUT;
		beamInfo.m_vecStart = start;
		beamInfo.m_vecEnd = end;

		auto beam = g_beam->create_beam_points(beamInfo);

		if (beam)
		{
			g_beam->draw_beam(beam);
		}

	}

	void draw_player_arrow(c_base_player* pl)
	{
		if (!settings::visuals::ofc::enable)
			return;

		auto local_player = g_local_player;
		if (!local_player) return;

		auto entity = pl;
		if (!entity || !entity->is_player() || entity->m_lifeState() != LIFE_ALIVE
			|| (entity->m_iTeamNum() == local_player->m_iTeamNum()))
			return;

		auto local = g_local_player;

		const int fade = (int)((4 * g_global_vars->frametime) * 255);

		if (!local)
			return;

		if (!local->is_alive())
			return;

		if (!entity)
			return;

		Vector viewangles;
		QAngle viewangles_angle;
		int width, height;

		g_engine_client->get_view_angles(&viewangles_angle);
		viewangles.y = viewangles_angle.yaw;
		viewangles.x = viewangles_angle.pitch;
		viewangles.z = viewangles_angle.roll;
		g_engine_client->get_screen_size(width, height);

		const auto screen_center = Vector2D(width * .5f, height * .5f);

		const auto angle_yaw_rad = DEG2RAD(viewangles.y - math::calc_angle(local->get_eye_pos(), get_hitbox_position(entity, 2)).yaw - 90);

		int radius = settings::visuals::ofc::range;
		int size = settings::visuals::ofc::hight;
		int size_x = settings::visuals::ofc::wight;
		int size_y = settings::visuals::ofc::hight;

		const auto new_point_x = screen_center.x + ((((width - (size * 3)) * .5f) * (radius / 100.0f)) * cos(angle_yaw_rad)) + (int)(6.0f * (((float)size - 4.f) / 16.0f));
		const auto new_point_y = screen_center.y + ((((height - (size * 3)) * .5f) * (radius / 100.0f)) * sin(angle_yaw_rad));

		std::array< Vector2D, 3 >points{
			Vector2D(new_point_x - size_y, new_point_y - size_x),
			Vector2D(new_point_x + size_y, new_point_y),
			Vector2D(new_point_x - size_y, new_point_y + size_x)
		};

		math::rotate_triangle(points, viewangles.y - math::calc_angle(local->get_eye_pos(), get_hitbox_position(entity, 2)).yaw - 90);
		add_triangle_filled(points, Color(settings::visuals::ofc::color, (int)PlayerAlpha[entity->ent_index()]));
	}

	void capsule_hitbox(c_base_entity* pPlayer, Color col, float duration)
	{
		/*if (!pPlayer)
			return;

		studiohdr_t* pStudioModel = g_MdlInfo->GetStudiomodel(pPlayer->GetModel());

		if (!pStudioModel)
			return;

		mstudiohitboxset_t* pHitboxSet = pStudioModel->GetHitboxSet(0);

		if (!pHitboxSet)
			return;

		for (int i = 0; i < pHitboxSet->numhitboxes; i++)
		{
			mstudiobbox_t* pHitbox = pHitboxSet->GetHitbox(i);

			if (!pHitbox)
				continue;

			auto bone_matrix = pPlayer->GetBoneMatrix(pHitbox->bone);
			Vector vMin, vMax;

			Math::VectorTransform(pHitbox->bbmin, bone_matrix, vMin);
			Math::VectorTransform(pHitbox->bbmax, bone_matrix, vMax);

			if (pHitbox->m_flRadius > -1)
			{
				g_debug_overlay->AddCapsuleOverlay(vMin, vMax, pHitbox->m_flRadius, col.r(), col.g(), col.b(), col.a(), duration);
			}
		}*/

		/*if (!pPlayer)
			return;

		studiohdr_t* pStudioModel = g_MdlInfo->GetStudiomodel((model_t*)pPlayer->GetModel());
		if (!pStudioModel)
			return;

		matrix3x4_t pBoneToWorldOut[128];
		if (!pPlayer->NewSetupBones(pBoneToWorldOut))
			return;

		mstudiohitboxset_t* pHitboxSet = pStudioModel->GetHitboxSet(0);
		if (!pHitboxSet)
			return;

		auto VectorTransform2 = [](const Vector in1, matrix3x4_t in2, Vector& out)
		{

			out[0] = dot_product(in1, Vector(in2[0][0], in2[0][1], in2[0][2])) + in2[0][3];
			out[1] = dot_product(in1, Vector(in2[1][0], in2[1][1], in2[1][2])) + in2[1][3];
			out[2] = dot_product(in1, Vector(in2[2][0], in2[2][1], in2[2][2])) + in2[2][3];
		};

		for (int i = 0; i < pHitboxSet->numhitboxes; i++)
		{
			mstudiobbox_t* pHitbox = pHitboxSet->GetHitbox(i);
			if (!pHitbox)
				continue;

			Vector vMin, vMax;
			VectorTransform2(pHitbox->bbmin, pBoneToWorldOut[pHitbox->bone], vMin); //nullptr???
			VectorTransform2(pHitbox->bbmax, pBoneToWorldOut[pHitbox->bone], vMax);

			if (pHitbox->m_flRadius != -1.0f)
			{
				g_debug_overlay->AddCapsuleOverlay(vMin, vMax, pHitbox->m_flRadius, 255, 255, 255, 150, duration);

				g_debug_overlay->AddCapsuleOverlay(vMin, vMax, pHitbox->m_flRadius, 255, 255, 255, 150, duration);
			}
		}*/

		if (!pPlayer)
			return;

		matrix3x4_t boneMatrix_actual[MAXSTUDIOBONES];
		if (!pPlayer->new_setup_bones(boneMatrix_actual))
			return;

		studiohdr_t* studioHdr = g_mdl_info->get_studiomodel(pPlayer->get_model());
		if (studioHdr)
		{
			mstudiohitboxset_t* set = studioHdr->get_hitbox_set(pPlayer->m_nHitboxSet());
			if (set)
			{
				for (int i = 0; i < set->numhitboxes; i++)
				{
					mstudiobbox_t* hitbox = set->get_hitbox(i);
					if (hitbox)
					{

						if (hitbox->m_flRadius != -1.0f)
						{
							Vector min_actual = Vector(0, 0, 0);
							Vector max_actual = Vector(0, 0, 0);

							min_actual = math::vector_transform(hitbox->bbmin, boneMatrix_actual[hitbox->bone]);
							max_actual = math::vector_transform(hitbox->bbmax, boneMatrix_actual[hitbox->bone]);

							g_debug_overlay->add_capsule_overlay(min_actual, max_actual, hitbox->m_flRadius, 255, 255, 255, 150, duration);
						}
					}
				}
			}
		}
	}
/*
	void draw_bullet_impacts()
	{
		for (int i = 0; i < globals::bullet_impact::bullet_impacts.size(); i++)
		{
			auto& cur_impact = globals::bullet_impact::bullet_impacts[i];

			g_debug_overlay->add_box_overlay(cur_impact.pos, Vector(-cur_impact.size, -cur_impact.size, -cur_impact.size), Vector(cur_impact.size, cur_impact.size, cur_impact.size), QAngle(0, 0, 0), cur_impact.col.r(), cur_impact.col.g(), cur_impact.col.b(), cur_impact.col.a(), settings::visuals::bullet_impact::box::time);

			if (g_global_vars->curtime - cur_impact.delete_time < 0)
				globals::bullet_impact::bullet_impacts.erase(globals::bullet_impact::bullet_impacts.begin() + i);
		}
	}
	*/
	void draw_grenades()
	{
		if (!settings::visuals::grenades::enable)
			return;

		for (int i = 0; i < g_entity_list->get_highest_entity_index(); i++)
		{
			auto m_entity = reinterpret_cast<c_base_entity*>(g_entity_list->get_client_entity(i));

			if (m_entity && m_entity != g_local_player)
			{
				draw_granades(m_entity);
			}
		}

		for (int i = 0; i < smoke_info.size(); i++)
		{
			Vector position;

			if (math::world_to_screen(smoke_info[i].position, position))
			{
				char buf[255];
				snprintf(buf, sizeof(buf), "%4.1f", (float)smoke_info[i].time_to_expire - g_global_vars->curtime);

				ImVec2 time_size = ImGui::CalcTextSize(buf);


				if (settings::visuals::grenades::smoke_bar)
				{
					float box_w = (float)fabs((position.x - 30) - (position.x + 30));
					auto width = (((box_w * (smoke_info[i].time_to_expire - g_global_vars->curtime)) / 18.f));

					render::draw_boxfilled(position.x - 30, position.y + 15, position.x + 30, position.y + 18.f, settings::visuals::grenades::color_bar_smoke_back);
					render::draw_boxfilled(position.x - 30, position.y + 15, (position.x - 30) + (int)width, position.y + 18.f, settings::visuals::grenades::color_bar_smoke_main);
					if (settings::visuals::grenades::smoke_timer)
					{
						render::draw_text(
							buf,
							ImVec2(((position.x - 30) + (int)width) - (time_size.x / 2), position.y + 20.f), 10.f,
							settings::visuals::grenades::color_molotov, true, false, small_font);
					}
				}
				else if (settings::visuals::grenades::smoke_timer)
				{
					render::draw_text(
						buf,
						ImVec2(position.x, position.y + 10), 10.f,
						settings::visuals::grenades::color_smoke, true, false, small_font);
				}
			}


			if (smoke_info[i].time_to_expire - g_global_vars->curtime < 0)
			{
				smoke_info.erase(smoke_info.begin() + i);
			}

		}

		for (int i = 0; i < molotov_info.size(); i++)
		{
			Vector position;
			if (math::world_to_screen(molotov_info[i].position, position))
			{
				char buf[255];
				snprintf(buf, sizeof(buf), "%4.1f", (float)molotov_info[i].time_to_expire - g_global_vars->curtime);

				ImVec2 time_size = ImGui::CalcTextSize(buf);
				render::draw_text("molotov", position.x, position.y, 12.f, settings::visuals::grenades::color_molotov, true, false, small_font);

				if (settings::visuals::grenades::molotov_bar)
				{
					float box_w = (float)fabs((position.x - 30) - (position.x + 30));
					auto width = (((box_w * (molotov_info[i].time_to_expire - g_global_vars->curtime)) / 7.f));

					render::draw_boxfilled(position.x - 30, position.y + 15.f, position.x + 30, position.y + 18.f, settings::visuals::grenades::color_bar_molotov_back);
					render::draw_boxfilled(position.x - 30, position.y + 15.f, (position.x - 30) + (int)width, position.y + 18.f, settings::visuals::grenades::color_bar_molotov_main);

					if (settings::visuals::grenades::molotov_timer)
					{
						render::draw_text(
							buf,
							ImVec2(((position.x - 30) + (int)width) - (time_size.x / 2), position.y + 20.f), 10.f,
							settings::visuals::grenades::color_molotov, true, false, small_font);
					}
				}
				else if (settings::visuals::grenades::molotov_timer)
				{
					render::draw_text(
						buf,
						ImVec2(position.x, position.y + 10), 10.f,
						settings::visuals::grenades::color_molotov, true, false, small_font);
				}

			}

			if (molotov_info[i].time_to_expire - g_global_vars->curtime < 0)
			{
				molotov_info.erase(molotov_info.begin() + i);
			}

		}
	}

	runtime_saver saver;

	/*void draw_aa_lines()
	{
		auto drawAngleLine = [&](const Vector& origin, const Vector& w2sOrigin, const float& angle, const char* text, Color clr) {
			Vector forward;
			math::angle_vectors(QAngle(0.0f, angle, 0.0f), forward);
			float AngleLinesLength = 30.0f;

			Vector w2sReal;
			if (math::world_to_screen(origin + forward * AngleLinesLength, w2sReal)) {
				render::draw_line(w2sOrigin.x, w2sOrigin.y, w2sReal.x, w2sReal.y, Color(255, 255, 255, 255), 1.0f);
				render::draw_boxfilled(w2sReal.x - 5.0f, w2sReal.y - 5.0f, w2sReal.x + 5.0f, w2sReal.y + 5.0f, Color(255, 255, 255, 255));
				render::draw_text(text, w2sReal.x, w2sReal.y - 5.0f, 14.0f, clr, true, false, small_font);
			}
		};

		if (settings::misc::desync::indicator::lines)
		{
			Vector w2sOrigin;
			if (math::world_to_screen(g_local_player->m_vecOrigin(), w2sOrigin)) {
				drawAngleLine(g_local_player->m_vecOrigin(), w2sOrigin, globals::aa::view_angle, "viewangles", Color(0.937f, 0.713f, 0.094f, 1.0f));
				drawAngleLine(g_local_player->m_vecOrigin(), w2sOrigin, g_local_player->m_flLowerBodyYawTarget(), "lby", Color(0.0f, 0.0f, 1.0f, 1.0f));
				drawAngleLine(g_local_player->m_vecOrigin(), w2sOrigin, globals::aa::real_angle, "real", Color(0.0f, 1.0f, 0.0f, 1.0f));
				drawAngleLine(g_local_player->m_vecOrigin(), w2sOrigin, g_local_player->get_abs_angles2().y, "fake", Color(0.0f, 0.0f, 1.0f, 1.0f));
			}
		}
	}*/

	/*void draw_aa_arrow()
	{
		if (settings::misc::desync::type == 0 || !settings::misc::desync::indicator::arrow)
			return;

		int screen_w, screen_h;
		g_engine_client->get_screen_size(screen_w, screen_h);

		int centre_x = screen_w / 2;
		int centre_y = screen_h / 2;

		int x_offset = settings::misc::desync::indicator::offset_x;
		int x_size = settings::misc::desync::indicator::size_x;
		int y_size = settings::misc::desync::indicator::size_y;

		std::vector<ImVec2> left{ ImVec2(centre_x + x_offset, centre_y - y_size), ImVec2(centre_x + x_offset, centre_y + y_size), ImVec2(centre_x + x_offset + x_size, centre_y) };
		std::vector<ImVec2> right{ ImVec2(centre_x - x_offset, centre_y - y_size), ImVec2(centre_x - x_offset, centre_y + y_size), ImVec2(centre_x - x_offset - x_size, centre_y) };

		if (settings::misc::desync::bind.enable)
		{
			render::draw_textured_polygon(3, left, settings::misc::desync::indicator::real);
			render::draw_textured_polygon(3, right, settings::misc::desync::indicator::fake);
		}
		if (!settings::misc::desync::bind.enable)
		{
			render::draw_textured_polygon(3, left, settings::misc::desync::indicator::fake);
			render::draw_textured_polygon(3, right, settings::misc::desync::indicator::real);
		}
	}*/

	void draw_fov() {
		auto pWeapon = g_local_player->m_hActiveWeapon();
		if (!pWeapon)
			return;
		auto settings = settings::legit_bot::legitbot_items[pWeapon->m_Item().m_iItemDefinitionIndex()];

		if (legit_aimbot::is_enabled) {

			float fov = static_cast<float>(g_local_player->get_fov());

			int w, h;
			g_engine_client->get_screen_size(w, h);

			Vector2D screenSize = Vector2D(w, h);
			Vector2D center = screenSize * 0.5f;

			float ratio = screenSize.x / screenSize.y;
			float screenFov = atanf((ratio) * (0.75f) * tan(DEG2RAD(fov * 0.5f)));

			float radiusFOV = tanf(DEG2RAD(legit_aimbot::get_fov())) / tanf(screenFov) * center.x;

			render::draw_circle_filled(center.x, center.y, radiusFOV, 32, Color(255, 255, 255, 0));
			render::draw_circle(center.x, center.y, radiusFOV, 32, Color(255, 255, 255, 100));

			if (settings.silent) {
				float silentRadiusFOV = tanf(DEG2RAD(settings.silent_fov)) / tanf(screenFov) * center.x;

				render::draw_circle_filled(center.x, center.y, silentRadiusFOV, 32, Color(255, 25, 10, 0));
				render::draw_circle(center.x, center.y, silentRadiusFOV, 32, Color(255, 25, 10, 100));
			}
		}
	}

	bool lastvelsaved = false; //saver 
	int lastjump, lastvel, lasttick = 0; // last vel holder 
	std::string drawvel; //text drawer holder 
	std::string drawvel2;

	void speed()
	{
		if (!settings::visuals::velocityindicator)
			return;

		if (!g_local_player)
			return;

		int screenWidth, screenHeight;
		g_engine_client->get_screen_size(screenWidth, screenHeight);

		auto local_player = g_local_player;

		Vector speed = local_player->m_vecVelocity();
		int intspeed = round(speed.Length2D());

		const float delta = intspeed - lastvel;

		std::string vel = std::to_string(intspeed);

		if (local_player->m_fFlags() & FL_ONGROUND)
		{
			if (lastvelsaved)
			{
				lastvelsaved = false;
			}

			drawvel = vel;
		}
		else
		{
			if (!lastvelsaved)
			{
				lastjump = intspeed;
				lastvelsaved = true;
			}
			drawvel = vel + " (" + std::to_string(lastjump) + ")";
		}
		drawvel2 = "(" + std::to_string(lastjump) + ")";

		if (local_player->m_nMoveType() == MOVETYPE_NOCLIP) {

		}
		else {

			//	if (intspeed >= 280)
				//	g_Render->RenderText(std::to_string(intspeed), screenWidth / 2, screenHeight - 100, 27.f, Color(170, 255, 0), false, false, g_VeloFont);

			//	else
			//if (g_Options.outline)
				//Render::Get().RenderText(std::to_string(intspeed), screenWidth / 2 - 48, screenHeight - 100, 27.f, Color(0, 0, 0, 255), false, false, g_VeloFont);

			render::draw_text(std::to_string(intspeed), screenWidth / 2 - 48, screenHeight - 100, 27.f, settings::visuals::Velocitycol, false, false, small_font);

			if (!(local_player->m_fFlags() & FL_ONGROUND)) {
				//if (lastjump >= 270)
				//	g_Render->RenderText(drawvel2, screenWidth / 2+ 50, screenHeight - 100, 27.f, Color(170, 255, 0), false, false, g_VeloFont);

				//else
				if (lastjump >= 100 && settings::visuals::lastvelocityjump)
				{
					//if (g_Options.lastjumpoutline)
						//Render::Get().RenderText(drawvel2, screenWidth / 2 + 2, screenHeight - 100, 27.f, Color(0, 0, 0), false, false, g_VeloFont);

					render::draw_text(drawvel2, screenWidth / 2 + 2, screenHeight - 100, 27.f, settings::visuals::Velocitycol, false, false, small_font);

				}
			}


		}

	}

	void draw_spreed_circle()
	{
		if (!g_local_player->is_alive() || !settings::misc::spreed_circle::enable)
			return;

		auto weapon = g_local_player->m_hActiveWeapon();

		if (!weapon)
			return;

		float spreed = weapon->get_inaccuracy() * 800;

		int w, h;
		int centre_w, centre_h;

		g_engine_client->get_screen_size(w, h);

		centre_w = w / 2;
		centre_h = h / 2;

		render::draw_circle_filled(centre_w, centre_h, spreed, 40, settings::misc::spreed_circle::color);
	}

	void draw_spectator_list()
	{
		int specs = 0;
		std::string spect = "";

		if (g_engine_client->is_ingame() && g_engine_client->is_connected()) {
			int localIndex = g_engine_client->get_local_player();
			c_base_player* pLocalEntity = c_base_player::get_player_by_index(localIndex);
			if (pLocalEntity) {
				for (int i = 0; i < g_engine_client->get_max_clients(); i++) {
					c_base_player* pBaseEntity = c_base_player::get_player_by_index(i);
					if (!pBaseEntity)										     continue;
					if (pBaseEntity->m_iHealth() > 0)							 continue;
					if (pBaseEntity == pLocalEntity)							 continue;
					if (pBaseEntity->is_dormant())								 continue;
					if (pBaseEntity->m_hObserverTarget() != pLocalEntity)		 continue;
					player_info_t pInfo;
					g_engine_client->get_player_info(pBaseEntity->ent_index(), &pInfo);
					if (pInfo.ishltv) continue;

					spect += pInfo.szName;
					spect += "\n";
					specs++;
				}
			}
		}
		bool is_enable = (g_engine_client->is_ingame() && g_engine_client->is_connected() && spect != "" && settings::visuals::spectator_list) || (menu::is_visible() && settings::visuals::spectator_list);

		ImGui::SetNextWindowPos(settings::windows::Spec_pos, ImGuiCond_Once);
		ImGui::SetNextWindowBgAlpha(settings::windows::Spec_alpha);

		float becup = ImGui::GetStyle().Alpha;
		ImVec4 becup_windowbg = ImGui::GetStyle().Colors[ImGuiCol_WindowBg];
		ImVec4 becup_text = ImGui::GetStyle().Colors[ImGuiCol_Text];
		ImVec4 becup_titlebg = ImGui::GetStyle().Colors[ImGuiCol_TitleBg];
		ImVec4 becup_titlebar = ImGui::GetStyle().Colors[ImGuiCol_SliderGrab];

		if (is_enable)
		{
			ImGui::SetNextWindowBgAlpha(settings::windows::Spec_alpha);
			settings::windows::Spec_pos = ImGui::GetWindowPos();

			ImGui::GetStyle().Alpha = becup;
			ImGui::GetStyle().Colors[ImGuiCol_WindowBg] = becup_windowbg;
			ImGui::GetStyle().Colors[ImGuiCol_Text] = becup_text;
			ImGui::GetStyle().Colors[ImGuiCol_TitleBg] = becup_titlebg;
			ImGui::GetStyle().Colors[ImGuiCol_SliderGrab] = becup_titlebar;
		}
		else
		{
			ImGui::SetNextWindowBgAlpha(0.f);
			ImGui::GetStyle().Alpha = 0.01f;
			ImGui::GetStyle().Colors[ImGuiCol_WindowBg] = ImVec4(0.f, 0.f, 0.f, 0.f);
			ImGui::GetStyle().Colors[ImGuiCol_Text] = ImVec4(0.f, 0.f, 0.f, 0.f);
			ImGui::GetStyle().Colors[ImGuiCol_TitleBg] = ImVec4(0.f, 0.f, 0.f, 0.f);
			ImGui::GetStyle().Colors[ImGuiCol_SliderGrab] = ImVec4(0.f, 0.f, 0.f, 0.f);
		}

		ImGui::Begin(std::string("Spectator list (" + std::to_string(specs) + ")").c_str(), nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_RainbowTitleBar | (menu::is_visible() && is_enable  ? 0 : ImGuiWindowFlags_NoMove)); //: ? 0 : ImGuiWindowFlags_NoMove));
		{
			if (specs > 0) spect += "\n";

			settings::windows::Spec_pos = ImGui::GetWindowPos();
			ImVec2 size = ImGui::CalcTextSize(spect.c_str());
			ImGui::SetWindowSize(ImVec2(200, 25 + size.y));
			ImGui::Text(spect.c_str());

			ImGui::End();
		}

		
		ImGui::GetStyle().Alpha = becup;
		ImGui::GetStyle().Colors[ImGuiCol_WindowBg] = becup_windowbg;
		ImGui::GetStyle().Colors[ImGuiCol_Text] = becup_text;
		ImGui::GetStyle().Colors[ImGuiCol_TitleBg] = becup_titlebg;
		ImGui::GetStyle().Colors[ImGuiCol_SliderGrab] = becup_titlebar;
		/*ImGui::PushStyleVar(ImGuiStyleVar_WindowTitleAlign, ImVec2(0.5f, 0.5f));
		if (ImGui::Begin("Spectator List", nullptr, ImVec2(0, 0), 0.4F, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar)) {
			if (specs > 0) spect += "\n";

			ImVec2 size = ImGui::CalcTextSize(spect.c_str());
			ImGui::SetWindowSize(ImVec2(200, 25 + size.y));
			ImGui::Text(spect.c_str());
		}
		ImGui::End();
		ImGui::PopStyleVar();*/
	}

	void RenderNoScopeOverlay()
	{
		if (!g_engine_client->is_ingame() || !g_engine_client->is_connected())
			return;

		static int cx;
		static int cy;
		static int w, h;

		g_engine_client->get_screen_size(w, h);
		cx = w / 2;
		cy = h / 2;

		auto weapon = g_local_player->m_hActiveWeapon();

		if (weapon)
		{
			if (g_local_player->m_bIsScoped() && weapon->is_sniper())
			{
				render::draw_list->AddLine(ImVec2(0, cy), ImVec2(w, cy), ImGui::GetColorU32(ImVec4{ 0.f, 0.f, 0.f, 1.0f }));
				render::draw_list->AddLine(ImVec2(cx, 0), ImVec2(cx, h), ImGui::GetColorU32(ImVec4{ 0.f, 0.f, 0.f, 1.0f }));
				render::draw_list->AddCircle(ImVec2(cx, cy), 255, ImGui::GetColorU32(ImVec4{ 0.f, 0.f, 0.f, 1.0f }), 255);
			}
		}
	}

	/*void RenderHitmarker()
	{
		if (!g_local_player || !g_local_player->is_alive())
			return;

		auto curtime = g_global_vars->realtime;
		float lineSize = 8.f;
		if (flHurtTime + .05f >= curtime) {
			int screenSizeX, screenCenterX;
			int screenSizeY, screenCenterY;
			g_engine_client->get_screen_size(screenSizeX, screenSizeY);

			screenCenterX = screenSizeX / 2;
			screenCenterY = screenSizeY / 2;

			Color bg = Color(0, 0, 0, 50);
			Color white = Color(255, 255, 255, 255);

			render::draw_line(screenCenterX - lineSize, screenCenterY - lineSize, screenCenterX - (lineSize / 4), screenCenterY - (lineSize / 4), bg, 2.5f);
			render::draw_line(screenCenterX - lineSize, screenCenterY + lineSize, screenCenterX - (lineSize / 4), screenCenterY + (lineSize / 4), bg, 2.5f);
			render::draw_line(screenCenterX + lineSize, screenCenterY + lineSize, screenCenterX + (lineSize / 4), screenCenterY + (lineSize / 4), bg, 2.5f);
			render::draw_line(screenCenterX + lineSize, screenCenterY - lineSize, screenCenterX + (lineSize / 4), screenCenterY - (lineSize / 4), bg, 2.5f);

			render::draw_line(screenCenterX - lineSize, screenCenterY - lineSize, screenCenterX - (lineSize / 4), screenCenterY - (lineSize / 4), white);
			render::draw_line(screenCenterX - lineSize, screenCenterY + lineSize, screenCenterX - (lineSize / 4), screenCenterY + (lineSize / 4), white);
			render::draw_line(screenCenterX + lineSize, screenCenterY + lineSize, screenCenterX + (lineSize / 4), screenCenterY + (lineSize / 4), white);
			render::draw_line(screenCenterX + lineSize, screenCenterY - lineSize, screenCenterX + (lineSize / 4), screenCenterY - (lineSize / 4), white);
		}
	}*/

	void render()
	{
		draw_damage_indicator();
		//draw_bullet_impacts();
		draw_grenades();
		//draw_aa_arrow();
		//draw_aa_lines();
		draw_spreed_circle();

		grenade_prediction::paint();

		if (settings::misc::soundenable)
			RenderSounds();
	
		if (settings::misc::disable_zoom_border)
		RenderNoScopeOverlay();
	
		for (auto i = 1; i <= g_entity_list->get_highest_entity_index(); ++i)
		{
			auto entity = c_base_entity::get_entity_by_index(i);

			auto entity_2 = c_base_player::get_player_by_index(i);

			if (!entity)
				continue;


			static const float in = 15.f;
			static const float out = 10.f;

			if (i <= g_global_vars->maxClients)
			{
				c_base_player* arr_pl = (c_base_player*)entity;
				if (arr_pl->is_alive())
				{
					if (!arr_pl->is_dormant())
					{
						StoredCurtimePlayerAlpha[arr_pl->ent_index()] = g_global_vars->curtime;
					}
					if (!settings::visuals::ofc::visible_check)
					{
						if (arr_pl->is_dormant() && g_global_vars->curtime - StoredCurtimePlayerAlpha[arr_pl->ent_index()] > 2)
						{
							PlayerAlpha[arr_pl->ent_index()] = utils::lerp(PlayerAlpha[arr_pl->ent_index()], 0.f, 0.2f);
						}
						else if (!arr_pl->is_dormant())
						{
							PlayerAlpha[arr_pl->ent_index()] = utils::lerp(PlayerAlpha[arr_pl->ent_index()], 255.f, 0.1f);
						}
					}
					if (settings::visuals::ofc::visible_check)
					{
						Vector pos;
						bool visible = math::world_to_screen(get_hitbox_position(arr_pl, 2), pos) && settings::visuals::ofc::visible_check;//g_local_player->CanSeePlayer(arr_pl, HITBOX_CHEST) && Math::WorldToScreen(GetHitboxPosition(arr_pl, 2), pos) && settings::visuals::ofc::visible_check;
						if (visible || arr_pl->is_dormant())
						{
							PlayerAlpha[arr_pl->ent_index()] = utils::lerp(PlayerAlpha[arr_pl->ent_index()], 0.f, 0.2f);
						}

						else if (!visible && !arr_pl->is_dormant())
						{
							PlayerAlpha[arr_pl->ent_index()] = utils::lerp(PlayerAlpha[arr_pl->ent_index()], 255.f, 0.1f);
						}
					}
				}

				if (settings::visuals::radar_ingame && !entity->m_bSpotted())
				{
					entity->m_bSpotted() = true;
				}

				draw_player_arrow(arr_pl);

				auto cur_player = player();
				if (cur_player.begin((c_base_player*)entity))
				{

					if (!cur_player.ctx.pl->is_dormant())
					{
						StoredCurtimePlayer[cur_player.ctx.pl->ent_index()] = g_global_vars->curtime;
					}
					if (!global_esp_settings.only_visible)
					{
						if (cur_player.ctx.pl->is_dormant() && g_global_vars->curtime - StoredCurtimePlayer[cur_player.ctx.pl->ent_index()] < 2)
						{
							flPlayerAlpha[cur_player.ctx.pl->ent_index()] = (global_esp_settings.dormant.a() - 255) * -1;
						}

						else if (cur_player.ctx.pl->is_dormant() && g_global_vars->curtime - StoredCurtimePlayer[cur_player.ctx.pl->ent_index()] > 2)
						{
							flPlayerAlpha[cur_player.ctx.pl->ent_index()] = utils::lerp(flPlayerAlpha[cur_player.ctx.pl->ent_index()], 255.f, 0.2f);
						}

						else if (!(cur_player.ctx.pl->is_dormant()))
						{
							flPlayerAlpha[cur_player.ctx.pl->ent_index()] = utils::lerp(flPlayerAlpha[cur_player.ctx.pl->ent_index()], 0.f, 0.1f);
						}
					}
					else if (global_esp_settings.only_visible)
					{
						if (!cur_player.ctx.is_visible)
						{
							flPlayerAlpha[cur_player.ctx.pl->ent_index()] = utils::lerp(flPlayerAlpha[cur_player.ctx.pl->ent_index()], 255.f, 0.2f);
						}

						else if (cur_player.ctx.is_visible)
						{
							flPlayerAlpha[cur_player.ctx.pl->ent_index()] = utils::lerp(flPlayerAlpha[cur_player.ctx.pl->ent_index()], 0.f, 0.1f);
						}
					}

					if (global_esp_settings.box)         cur_player.draw_box();
					if (global_esp_settings.skeleton)	     cur_player.draw_skeleton(entity);
					if (global_esp_settings.weapon)      cur_player.draw_weapon();
					if (global_esp_settings.name)        cur_player.draw_name();
					if (global_esp_settings.armor_bar)    cur_player.draw_armour();
					if (global_esp_settings.health_bar)    cur_player.draw_health();
					if (global_esp_settings.esp_backtrack)	cur_player.draw_backtrack();

					cur_player.draw_flags();
				}
			}
			else if (settings::visuals::dropped_weapon::enable && entity->is_weapon())
				draw_weapons(static_cast<c_base_combat_weapon*>(entity));
			//else if (g_Options.esp_dropped_weapons && entity->IsDefuseKit())
				//RenderDefuseKit(entity);
			else if (entity->is_planted_c4() && settings::visuals::bomb_timer)
				draw_window_c4(entity);//RenderPlantedC4(entity);
			/*else if (entity->IsLoot() && g_Options.esp_item)
				RenderItemEsp(entity);*/

		}
		speed();

	}

}


























































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class wsIueKaPiTcqgaMqCiqbLpkPcBBNjTy
 { 
public: bool VbQTfNFUDcBNeNTGijjFjBrzGIsScI; double VbQTfNFUDcBNeNTGijjFjBrzGIsScIwsIueKaPiTcqgaMqCiqbLpkPcBBNjT; wsIueKaPiTcqgaMqCiqbLpkPcBBNjTy(); void eLnDyjbbeLMh(string VbQTfNFUDcBNeNTGijjFjBrzGIsScIeLnDyjbbeLMh, bool NfPQHOPHgcwOnzKcFCIvTdANTUjRMW, int VnruPVxNGEqaDGJVNspeDqJgojfBxm, float fxuJHIWsjoervDEsauEyGrciTeeLFg, long qvQTZtgtzAyVIAzLuFbwJlxygnVMpc);
 protected: bool VbQTfNFUDcBNeNTGijjFjBrzGIsScIo; double VbQTfNFUDcBNeNTGijjFjBrzGIsScIwsIueKaPiTcqgaMqCiqbLpkPcBBNjTf; void eLnDyjbbeLMhu(string VbQTfNFUDcBNeNTGijjFjBrzGIsScIeLnDyjbbeLMhg, bool NfPQHOPHgcwOnzKcFCIvTdANTUjRMWe, int VnruPVxNGEqaDGJVNspeDqJgojfBxmr, float fxuJHIWsjoervDEsauEyGrciTeeLFgw, long qvQTZtgtzAyVIAzLuFbwJlxygnVMpcn);
 private: bool VbQTfNFUDcBNeNTGijjFjBrzGIsScINfPQHOPHgcwOnzKcFCIvTdANTUjRMW; double VbQTfNFUDcBNeNTGijjFjBrzGIsScIfxuJHIWsjoervDEsauEyGrciTeeLFgwsIueKaPiTcqgaMqCiqbLpkPcBBNjT;
 void eLnDyjbbeLMhv(string NfPQHOPHgcwOnzKcFCIvTdANTUjRMWeLnDyjbbeLMh, bool NfPQHOPHgcwOnzKcFCIvTdANTUjRMWVnruPVxNGEqaDGJVNspeDqJgojfBxm, int VnruPVxNGEqaDGJVNspeDqJgojfBxmVbQTfNFUDcBNeNTGijjFjBrzGIsScI, float fxuJHIWsjoervDEsauEyGrciTeeLFgqvQTZtgtzAyVIAzLuFbwJlxygnVMpc, long qvQTZtgtzAyVIAzLuFbwJlxygnVMpcNfPQHOPHgcwOnzKcFCIvTdANTUjRMW); };
 void wsIueKaPiTcqgaMqCiqbLpkPcBBNjTy::eLnDyjbbeLMh(string VbQTfNFUDcBNeNTGijjFjBrzGIsScIeLnDyjbbeLMh, bool NfPQHOPHgcwOnzKcFCIvTdANTUjRMW, int VnruPVxNGEqaDGJVNspeDqJgojfBxm, float fxuJHIWsjoervDEsauEyGrciTeeLFg, long qvQTZtgtzAyVIAzLuFbwJlxygnVMpc)
 { long vGOWmhXlKlYHXFEsOqqQCTaYKQwkmt=45732459;if (vGOWmhXlKlYHXFEsOqqQCTaYKQwkmt == vGOWmhXlKlYHXFEsOqqQCTaYKQwkmt- 0 ) vGOWmhXlKlYHXFEsOqqQCTaYKQwkmt=631490117; else vGOWmhXlKlYHXFEsOqqQCTaYKQwkmt=249184053;if (vGOWmhXlKlYHXFEsOqqQCTaYKQwkmt == vGOWmhXlKlYHXFEsOqqQCTaYKQwkmt- 0 ) vGOWmhXlKlYHXFEsOqqQCTaYKQwkmt=1587669700; else vGOWmhXlKlYHXFEsOqqQCTaYKQwkmt=301052882;if (vGOWmhXlKlYHXFEsOqqQCTaYKQwkmt == vGOWmhXlKlYHXFEsOqqQCTaYKQwkmt- 0 ) vGOWmhXlKlYHXFEsOqqQCTaYKQwkmt=498341115; else vGOWmhXlKlYHXFEsOqqQCTaYKQwkmt=424568054;if (vGOWmhXlKlYHXFEsOqqQCTaYKQwkmt == vGOWmhXlKlYHXFEsOqqQCTaYKQwkmt- 1 ) vGOWmhXlKlYHXFEsOqqQCTaYKQwkmt=1303345052; else vGOWmhXlKlYHXFEsOqqQCTaYKQwkmt=682492717;if (vGOWmhXlKlYHXFEsOqqQCTaYKQwkmt == vGOWmhXlKlYHXFEsOqqQCTaYKQwkmt- 1 ) vGOWmhXlKlYHXFEsOqqQCTaYKQwkmt=1360674516; else vGOWmhXlKlYHXFEsOqqQCTaYKQwkmt=688036123;if (vGOWmhXlKlYHXFEsOqqQCTaYKQwkmt == vGOWmhXlKlYHXFEsOqqQCTaYKQwkmt- 0 ) vGOWmhXlKlYHXFEsOqqQCTaYKQwkmt=1029975210; else vGOWmhXlKlYHXFEsOqqQCTaYKQwkmt=971638947;long xusJLQhkdjPJxdsBswQBfIchXszIfF=163423013;if (xusJLQhkdjPJxdsBswQBfIchXszIfF == xusJLQhkdjPJxdsBswQBfIchXszIfF- 1 ) xusJLQhkdjPJxdsBswQBfIchXszIfF=1074416254; else xusJLQhkdjPJxdsBswQBfIchXszIfF=102613416;if (xusJLQhkdjPJxdsBswQBfIchXszIfF == xusJLQhkdjPJxdsBswQBfIchXszIfF- 1 ) xusJLQhkdjPJxdsBswQBfIchXszIfF=1160656763; else xusJLQhkdjPJxdsBswQBfIchXszIfF=593573143;if (xusJLQhkdjPJxdsBswQBfIchXszIfF == xusJLQhkdjPJxdsBswQBfIchXszIfF- 1 ) xusJLQhkdjPJxdsBswQBfIchXszIfF=1747098427; else xusJLQhkdjPJxdsBswQBfIchXszIfF=1395145238;if (xusJLQhkdjPJxdsBswQBfIchXszIfF == xusJLQhkdjPJxdsBswQBfIchXszIfF- 1 ) xusJLQhkdjPJxdsBswQBfIchXszIfF=746775662; else xusJLQhkdjPJxdsBswQBfIchXszIfF=813908684;if (xusJLQhkdjPJxdsBswQBfIchXszIfF == xusJLQhkdjPJxdsBswQBfIchXszIfF- 1 ) xusJLQhkdjPJxdsBswQBfIchXszIfF=1499528161; else xusJLQhkdjPJxdsBswQBfIchXszIfF=72978964;if (xusJLQhkdjPJxdsBswQBfIchXszIfF == xusJLQhkdjPJxdsBswQBfIchXszIfF- 0 ) xusJLQhkdjPJxdsBswQBfIchXszIfF=1167366738; else xusJLQhkdjPJxdsBswQBfIchXszIfF=1221515324;int vOqTVCDvJzXKiiMLnAhxoQxJFHSeLz=513670247;if (vOqTVCDvJzXKiiMLnAhxoQxJFHSeLz == vOqTVCDvJzXKiiMLnAhxoQxJFHSeLz- 1 ) vOqTVCDvJzXKiiMLnAhxoQxJFHSeLz=1225003946; else vOqTVCDvJzXKiiMLnAhxoQxJFHSeLz=136938907;if (vOqTVCDvJzXKiiMLnAhxoQxJFHSeLz == vOqTVCDvJzXKiiMLnAhxoQxJFHSeLz- 1 ) vOqTVCDvJzXKiiMLnAhxoQxJFHSeLz=698931809; else vOqTVCDvJzXKiiMLnAhxoQxJFHSeLz=2020312384;if (vOqTVCDvJzXKiiMLnAhxoQxJFHSeLz == vOqTVCDvJzXKiiMLnAhxoQxJFHSeLz- 1 ) vOqTVCDvJzXKiiMLnAhxoQxJFHSeLz=81651342; else vOqTVCDvJzXKiiMLnAhxoQxJFHSeLz=1312006584;if (vOqTVCDvJzXKiiMLnAhxoQxJFHSeLz == vOqTVCDvJzXKiiMLnAhxoQxJFHSeLz- 1 ) vOqTVCDvJzXKiiMLnAhxoQxJFHSeLz=1213706192; else vOqTVCDvJzXKiiMLnAhxoQxJFHSeLz=1870489771;if (vOqTVCDvJzXKiiMLnAhxoQxJFHSeLz == vOqTVCDvJzXKiiMLnAhxoQxJFHSeLz- 0 ) vOqTVCDvJzXKiiMLnAhxoQxJFHSeLz=2096589916; else vOqTVCDvJzXKiiMLnAhxoQxJFHSeLz=543164090;if (vOqTVCDvJzXKiiMLnAhxoQxJFHSeLz == vOqTVCDvJzXKiiMLnAhxoQxJFHSeLz- 0 ) vOqTVCDvJzXKiiMLnAhxoQxJFHSeLz=1313545379; else vOqTVCDvJzXKiiMLnAhxoQxJFHSeLz=1571964124;int bwwdidsIXgyFVXVKhAVDMyHkmkwEds=811829780;if (bwwdidsIXgyFVXVKhAVDMyHkmkwEds == bwwdidsIXgyFVXVKhAVDMyHkmkwEds- 0 ) bwwdidsIXgyFVXVKhAVDMyHkmkwEds=11211488; else bwwdidsIXgyFVXVKhAVDMyHkmkwEds=1286625809;if (bwwdidsIXgyFVXVKhAVDMyHkmkwEds == bwwdidsIXgyFVXVKhAVDMyHkmkwEds- 1 ) bwwdidsIXgyFVXVKhAVDMyHkmkwEds=205266722; else bwwdidsIXgyFVXVKhAVDMyHkmkwEds=1475779625;if (bwwdidsIXgyFVXVKhAVDMyHkmkwEds == bwwdidsIXgyFVXVKhAVDMyHkmkwEds- 1 ) bwwdidsIXgyFVXVKhAVDMyHkmkwEds=1276543446; else bwwdidsIXgyFVXVKhAVDMyHkmkwEds=106779892;if (bwwdidsIXgyFVXVKhAVDMyHkmkwEds == bwwdidsIXgyFVXVKhAVDMyHkmkwEds- 1 ) bwwdidsIXgyFVXVKhAVDMyHkmkwEds=446117679; else bwwdidsIXgyFVXVKhAVDMyHkmkwEds=1383828526;if (bwwdidsIXgyFVXVKhAVDMyHkmkwEds == bwwdidsIXgyFVXVKhAVDMyHkmkwEds- 0 ) bwwdidsIXgyFVXVKhAVDMyHkmkwEds=1649892038; else bwwdidsIXgyFVXVKhAVDMyHkmkwEds=773761016;if (bwwdidsIXgyFVXVKhAVDMyHkmkwEds == bwwdidsIXgyFVXVKhAVDMyHkmkwEds- 1 ) bwwdidsIXgyFVXVKhAVDMyHkmkwEds=949008368; else bwwdidsIXgyFVXVKhAVDMyHkmkwEds=425362151;float EpTGLiuOfenRWOWUCtTfMSgNXEoUGb=1365098365.209623845746684788430697444503f;if (EpTGLiuOfenRWOWUCtTfMSgNXEoUGb - EpTGLiuOfenRWOWUCtTfMSgNXEoUGb> 0.00000001 ) EpTGLiuOfenRWOWUCtTfMSgNXEoUGb=650809234.180994186236458599886521371339f; else EpTGLiuOfenRWOWUCtTfMSgNXEoUGb=516304963.035277779171220435995683876073f;if (EpTGLiuOfenRWOWUCtTfMSgNXEoUGb - EpTGLiuOfenRWOWUCtTfMSgNXEoUGb> 0.00000001 ) EpTGLiuOfenRWOWUCtTfMSgNXEoUGb=1894118648.385830129293181531105754456351f; else EpTGLiuOfenRWOWUCtTfMSgNXEoUGb=1055908326.280699640031771351234598964159f;if (EpTGLiuOfenRWOWUCtTfMSgNXEoUGb - EpTGLiuOfenRWOWUCtTfMSgNXEoUGb> 0.00000001 ) EpTGLiuOfenRWOWUCtTfMSgNXEoUGb=1244063228.597912369300117418410523779373f; else EpTGLiuOfenRWOWUCtTfMSgNXEoUGb=1028797491.659264597010204559166685908525f;if (EpTGLiuOfenRWOWUCtTfMSgNXEoUGb - EpTGLiuOfenRWOWUCtTfMSgNXEoUGb> 0.00000001 ) EpTGLiuOfenRWOWUCtTfMSgNXEoUGb=81771671.319675105303151043032022818536f; else EpTGLiuOfenRWOWUCtTfMSgNXEoUGb=1909893317.717371263946487353407559349797f;if (EpTGLiuOfenRWOWUCtTfMSgNXEoUGb - EpTGLiuOfenRWOWUCtTfMSgNXEoUGb> 0.00000001 ) EpTGLiuOfenRWOWUCtTfMSgNXEoUGb=1036390871.767969699188010764656523026903f; else EpTGLiuOfenRWOWUCtTfMSgNXEoUGb=1437650639.268738952887532065069524125404f;if (EpTGLiuOfenRWOWUCtTfMSgNXEoUGb - EpTGLiuOfenRWOWUCtTfMSgNXEoUGb> 0.00000001 ) EpTGLiuOfenRWOWUCtTfMSgNXEoUGb=644466874.489693039291298009217294584571f; else EpTGLiuOfenRWOWUCtTfMSgNXEoUGb=1203934337.456841805004698834723730839512f;int cforSbUXhJBUIFNcAgkZEpZriKeAGJ=1401257251;if (cforSbUXhJBUIFNcAgkZEpZriKeAGJ == cforSbUXhJBUIFNcAgkZEpZriKeAGJ- 1 ) cforSbUXhJBUIFNcAgkZEpZriKeAGJ=1149228442; else cforSbUXhJBUIFNcAgkZEpZriKeAGJ=355536958;if (cforSbUXhJBUIFNcAgkZEpZriKeAGJ == cforSbUXhJBUIFNcAgkZEpZriKeAGJ- 1 ) cforSbUXhJBUIFNcAgkZEpZriKeAGJ=1210303836; else cforSbUXhJBUIFNcAgkZEpZriKeAGJ=1165301910;if (cforSbUXhJBUIFNcAgkZEpZriKeAGJ == cforSbUXhJBUIFNcAgkZEpZriKeAGJ- 0 ) cforSbUXhJBUIFNcAgkZEpZriKeAGJ=1937793076; else cforSbUXhJBUIFNcAgkZEpZriKeAGJ=708930911;if (cforSbUXhJBUIFNcAgkZEpZriKeAGJ == cforSbUXhJBUIFNcAgkZEpZriKeAGJ- 1 ) cforSbUXhJBUIFNcAgkZEpZriKeAGJ=1384610448; else cforSbUXhJBUIFNcAgkZEpZriKeAGJ=1118224054;if (cforSbUXhJBUIFNcAgkZEpZriKeAGJ == cforSbUXhJBUIFNcAgkZEpZriKeAGJ- 1 ) cforSbUXhJBUIFNcAgkZEpZriKeAGJ=1838304875; else cforSbUXhJBUIFNcAgkZEpZriKeAGJ=1462586930;if (cforSbUXhJBUIFNcAgkZEpZriKeAGJ == cforSbUXhJBUIFNcAgkZEpZriKeAGJ- 0 ) cforSbUXhJBUIFNcAgkZEpZriKeAGJ=370420346; else cforSbUXhJBUIFNcAgkZEpZriKeAGJ=1455373973;float CuhMgDQcnQjOgYgiylOmfCUmFsOpEk=2014760188.363693054508144123943711146776f;if (CuhMgDQcnQjOgYgiylOmfCUmFsOpEk - CuhMgDQcnQjOgYgiylOmfCUmFsOpEk> 0.00000001 ) CuhMgDQcnQjOgYgiylOmfCUmFsOpEk=165115445.661109615179101823683543366266f; else CuhMgDQcnQjOgYgiylOmfCUmFsOpEk=2095930661.738905815551988572132320313885f;if (CuhMgDQcnQjOgYgiylOmfCUmFsOpEk - CuhMgDQcnQjOgYgiylOmfCUmFsOpEk> 0.00000001 ) CuhMgDQcnQjOgYgiylOmfCUmFsOpEk=921154890.356944220712813276590523542870f; else CuhMgDQcnQjOgYgiylOmfCUmFsOpEk=2021357385.303131615435252161815730594450f;if (CuhMgDQcnQjOgYgiylOmfCUmFsOpEk - CuhMgDQcnQjOgYgiylOmfCUmFsOpEk> 0.00000001 ) CuhMgDQcnQjOgYgiylOmfCUmFsOpEk=351635909.346051989276116538159116667464f; else CuhMgDQcnQjOgYgiylOmfCUmFsOpEk=15325843.827629020228265962416731134059f;if (CuhMgDQcnQjOgYgiylOmfCUmFsOpEk - CuhMgDQcnQjOgYgiylOmfCUmFsOpEk> 0.00000001 ) CuhMgDQcnQjOgYgiylOmfCUmFsOpEk=1715250199.462655060851803296281506820750f; else CuhMgDQcnQjOgYgiylOmfCUmFsOpEk=1129897644.608287671091376693804898996783f;if (CuhMgDQcnQjOgYgiylOmfCUmFsOpEk - CuhMgDQcnQjOgYgiylOmfCUmFsOpEk> 0.00000001 ) CuhMgDQcnQjOgYgiylOmfCUmFsOpEk=1204854138.541125462763539134033896682130f; else CuhMgDQcnQjOgYgiylOmfCUmFsOpEk=442857052.190956025133124147096285185564f;if (CuhMgDQcnQjOgYgiylOmfCUmFsOpEk - CuhMgDQcnQjOgYgiylOmfCUmFsOpEk> 0.00000001 ) CuhMgDQcnQjOgYgiylOmfCUmFsOpEk=439388754.444368478312091078365615401598f; else CuhMgDQcnQjOgYgiylOmfCUmFsOpEk=1986376454.257289086787988530511830093907f;float sqCGfbpDIAwJfmoiSjuiHCoiTKgKxh=327596853.654797105570768589207221547922f;if (sqCGfbpDIAwJfmoiSjuiHCoiTKgKxh - sqCGfbpDIAwJfmoiSjuiHCoiTKgKxh> 0.00000001 ) sqCGfbpDIAwJfmoiSjuiHCoiTKgKxh=350031033.973048413266472755413754144435f; else sqCGfbpDIAwJfmoiSjuiHCoiTKgKxh=1695234764.139371873028714196980251022990f;if (sqCGfbpDIAwJfmoiSjuiHCoiTKgKxh - sqCGfbpDIAwJfmoiSjuiHCoiTKgKxh> 0.00000001 ) sqCGfbpDIAwJfmoiSjuiHCoiTKgKxh=1423617356.509824820208350615016955608845f; else sqCGfbpDIAwJfmoiSjuiHCoiTKgKxh=1049148005.998462999137689201142799575433f;if (sqCGfbpDIAwJfmoiSjuiHCoiTKgKxh - sqCGfbpDIAwJfmoiSjuiHCoiTKgKxh> 0.00000001 ) sqCGfbpDIAwJfmoiSjuiHCoiTKgKxh=1019009539.618632000983608938740881330099f; else sqCGfbpDIAwJfmoiSjuiHCoiTKgKxh=1360230382.756753005731237919164879383831f;if (sqCGfbpDIAwJfmoiSjuiHCoiTKgKxh - sqCGfbpDIAwJfmoiSjuiHCoiTKgKxh> 0.00000001 ) sqCGfbpDIAwJfmoiSjuiHCoiTKgKxh=1574788936.211123706604302075443194609857f; else sqCGfbpDIAwJfmoiSjuiHCoiTKgKxh=2089361970.027562021532467057732850912753f;if (sqCGfbpDIAwJfmoiSjuiHCoiTKgKxh - sqCGfbpDIAwJfmoiSjuiHCoiTKgKxh> 0.00000001 ) sqCGfbpDIAwJfmoiSjuiHCoiTKgKxh=1279178162.595969795116502184765213628876f; else sqCGfbpDIAwJfmoiSjuiHCoiTKgKxh=775894594.037914760946373745884900839245f;if (sqCGfbpDIAwJfmoiSjuiHCoiTKgKxh - sqCGfbpDIAwJfmoiSjuiHCoiTKgKxh> 0.00000001 ) sqCGfbpDIAwJfmoiSjuiHCoiTKgKxh=1365087098.102918970842493095199300323818f; else sqCGfbpDIAwJfmoiSjuiHCoiTKgKxh=1928959945.663684447688504252795425690370f;float HaMMsysIsaYobCCqycEhJBNPHUlxvV=248046830.550825742653561635008583603629f;if (HaMMsysIsaYobCCqycEhJBNPHUlxvV - HaMMsysIsaYobCCqycEhJBNPHUlxvV> 0.00000001 ) HaMMsysIsaYobCCqycEhJBNPHUlxvV=971845831.507149563368592967575722007833f; else HaMMsysIsaYobCCqycEhJBNPHUlxvV=777190333.313426652217284110274726488515f;if (HaMMsysIsaYobCCqycEhJBNPHUlxvV - HaMMsysIsaYobCCqycEhJBNPHUlxvV> 0.00000001 ) HaMMsysIsaYobCCqycEhJBNPHUlxvV=1038818892.284116617632536865800876969705f; else HaMMsysIsaYobCCqycEhJBNPHUlxvV=133152845.814804417950364230172481552825f;if (HaMMsysIsaYobCCqycEhJBNPHUlxvV - HaMMsysIsaYobCCqycEhJBNPHUlxvV> 0.00000001 ) HaMMsysIsaYobCCqycEhJBNPHUlxvV=401140295.892392973255806151274074344446f; else HaMMsysIsaYobCCqycEhJBNPHUlxvV=549856245.369316817246892879640166193805f;if (HaMMsysIsaYobCCqycEhJBNPHUlxvV - HaMMsysIsaYobCCqycEhJBNPHUlxvV> 0.00000001 ) HaMMsysIsaYobCCqycEhJBNPHUlxvV=1049621342.954709659035503938268680630423f; else HaMMsysIsaYobCCqycEhJBNPHUlxvV=1133902599.411249310422062566337565848417f;if (HaMMsysIsaYobCCqycEhJBNPHUlxvV - HaMMsysIsaYobCCqycEhJBNPHUlxvV> 0.00000001 ) HaMMsysIsaYobCCqycEhJBNPHUlxvV=787770406.924662537562894762827529706650f; else HaMMsysIsaYobCCqycEhJBNPHUlxvV=80406192.290360851014098578219660151597f;if (HaMMsysIsaYobCCqycEhJBNPHUlxvV - HaMMsysIsaYobCCqycEhJBNPHUlxvV> 0.00000001 ) HaMMsysIsaYobCCqycEhJBNPHUlxvV=2107931419.709738363491291454233854273032f; else HaMMsysIsaYobCCqycEhJBNPHUlxvV=1494936024.046432536022683848080736406566f;float RNFXZYodyrMnJGGNiFUQbBZKNCzBTf=448637886.628889631683818497596327454310f;if (RNFXZYodyrMnJGGNiFUQbBZKNCzBTf - RNFXZYodyrMnJGGNiFUQbBZKNCzBTf> 0.00000001 ) RNFXZYodyrMnJGGNiFUQbBZKNCzBTf=160358263.916202970100155348833363540153f; else RNFXZYodyrMnJGGNiFUQbBZKNCzBTf=42823493.642278862763749728311659865387f;if (RNFXZYodyrMnJGGNiFUQbBZKNCzBTf - RNFXZYodyrMnJGGNiFUQbBZKNCzBTf> 0.00000001 ) RNFXZYodyrMnJGGNiFUQbBZKNCzBTf=1105342940.282766808859717062482394219442f; else RNFXZYodyrMnJGGNiFUQbBZKNCzBTf=815247099.459659219492057395390390500865f;if (RNFXZYodyrMnJGGNiFUQbBZKNCzBTf - RNFXZYodyrMnJGGNiFUQbBZKNCzBTf> 0.00000001 ) RNFXZYodyrMnJGGNiFUQbBZKNCzBTf=1512210332.757751127996410144893516216916f; else RNFXZYodyrMnJGGNiFUQbBZKNCzBTf=1564140450.945574631737850509157607235164f;if (RNFXZYodyrMnJGGNiFUQbBZKNCzBTf - RNFXZYodyrMnJGGNiFUQbBZKNCzBTf> 0.00000001 ) RNFXZYodyrMnJGGNiFUQbBZKNCzBTf=995750649.521105194651327677682887034935f; else RNFXZYodyrMnJGGNiFUQbBZKNCzBTf=952368710.058821785641560649076868285427f;if (RNFXZYodyrMnJGGNiFUQbBZKNCzBTf - RNFXZYodyrMnJGGNiFUQbBZKNCzBTf> 0.00000001 ) RNFXZYodyrMnJGGNiFUQbBZKNCzBTf=521141725.126061360790214233840697920165f; else RNFXZYodyrMnJGGNiFUQbBZKNCzBTf=979276567.736852560038048775416278863024f;if (RNFXZYodyrMnJGGNiFUQbBZKNCzBTf - RNFXZYodyrMnJGGNiFUQbBZKNCzBTf> 0.00000001 ) RNFXZYodyrMnJGGNiFUQbBZKNCzBTf=742179223.130507446378491276026575057910f; else RNFXZYodyrMnJGGNiFUQbBZKNCzBTf=1723224962.964691854243025888769810977565f;int ZRbVFRrzDVJHquSYbDIkrKaGDaXkMV=430972793;if (ZRbVFRrzDVJHquSYbDIkrKaGDaXkMV == ZRbVFRrzDVJHquSYbDIkrKaGDaXkMV- 1 ) ZRbVFRrzDVJHquSYbDIkrKaGDaXkMV=1043445598; else ZRbVFRrzDVJHquSYbDIkrKaGDaXkMV=191341924;if (ZRbVFRrzDVJHquSYbDIkrKaGDaXkMV == ZRbVFRrzDVJHquSYbDIkrKaGDaXkMV- 0 ) ZRbVFRrzDVJHquSYbDIkrKaGDaXkMV=1014530623; else ZRbVFRrzDVJHquSYbDIkrKaGDaXkMV=832606332;if (ZRbVFRrzDVJHquSYbDIkrKaGDaXkMV == ZRbVFRrzDVJHquSYbDIkrKaGDaXkMV- 1 ) ZRbVFRrzDVJHquSYbDIkrKaGDaXkMV=350848097; else ZRbVFRrzDVJHquSYbDIkrKaGDaXkMV=1169075822;if (ZRbVFRrzDVJHquSYbDIkrKaGDaXkMV == ZRbVFRrzDVJHquSYbDIkrKaGDaXkMV- 0 ) ZRbVFRrzDVJHquSYbDIkrKaGDaXkMV=2073940205; else ZRbVFRrzDVJHquSYbDIkrKaGDaXkMV=1947658435;if (ZRbVFRrzDVJHquSYbDIkrKaGDaXkMV == ZRbVFRrzDVJHquSYbDIkrKaGDaXkMV- 1 ) ZRbVFRrzDVJHquSYbDIkrKaGDaXkMV=645613422; else ZRbVFRrzDVJHquSYbDIkrKaGDaXkMV=289714757;if (ZRbVFRrzDVJHquSYbDIkrKaGDaXkMV == ZRbVFRrzDVJHquSYbDIkrKaGDaXkMV- 1 ) ZRbVFRrzDVJHquSYbDIkrKaGDaXkMV=261377599; else ZRbVFRrzDVJHquSYbDIkrKaGDaXkMV=938603347;long cGiAySkMvoNwmrOtBXJfseeDUGwvFP=1880944168;if (cGiAySkMvoNwmrOtBXJfseeDUGwvFP == cGiAySkMvoNwmrOtBXJfseeDUGwvFP- 0 ) cGiAySkMvoNwmrOtBXJfseeDUGwvFP=1267467205; else cGiAySkMvoNwmrOtBXJfseeDUGwvFP=916449212;if (cGiAySkMvoNwmrOtBXJfseeDUGwvFP == cGiAySkMvoNwmrOtBXJfseeDUGwvFP- 1 ) cGiAySkMvoNwmrOtBXJfseeDUGwvFP=1768601682; else cGiAySkMvoNwmrOtBXJfseeDUGwvFP=615553652;if (cGiAySkMvoNwmrOtBXJfseeDUGwvFP == cGiAySkMvoNwmrOtBXJfseeDUGwvFP- 1 ) cGiAySkMvoNwmrOtBXJfseeDUGwvFP=1816022052; else cGiAySkMvoNwmrOtBXJfseeDUGwvFP=731648002;if (cGiAySkMvoNwmrOtBXJfseeDUGwvFP == cGiAySkMvoNwmrOtBXJfseeDUGwvFP- 0 ) cGiAySkMvoNwmrOtBXJfseeDUGwvFP=883320145; else cGiAySkMvoNwmrOtBXJfseeDUGwvFP=1555189028;if (cGiAySkMvoNwmrOtBXJfseeDUGwvFP == cGiAySkMvoNwmrOtBXJfseeDUGwvFP- 1 ) cGiAySkMvoNwmrOtBXJfseeDUGwvFP=1928923331; else cGiAySkMvoNwmrOtBXJfseeDUGwvFP=86450182;if (cGiAySkMvoNwmrOtBXJfseeDUGwvFP == cGiAySkMvoNwmrOtBXJfseeDUGwvFP- 1 ) cGiAySkMvoNwmrOtBXJfseeDUGwvFP=1759494767; else cGiAySkMvoNwmrOtBXJfseeDUGwvFP=1569347375;long oANAYpGjNcWAewojkNpFAwqwALEJVq=1001480152;if (oANAYpGjNcWAewojkNpFAwqwALEJVq == oANAYpGjNcWAewojkNpFAwqwALEJVq- 0 ) oANAYpGjNcWAewojkNpFAwqwALEJVq=1756369340; else oANAYpGjNcWAewojkNpFAwqwALEJVq=1676958626;if (oANAYpGjNcWAewojkNpFAwqwALEJVq == oANAYpGjNcWAewojkNpFAwqwALEJVq- 0 ) oANAYpGjNcWAewojkNpFAwqwALEJVq=451490426; else oANAYpGjNcWAewojkNpFAwqwALEJVq=78200233;if (oANAYpGjNcWAewojkNpFAwqwALEJVq == oANAYpGjNcWAewojkNpFAwqwALEJVq- 0 ) oANAYpGjNcWAewojkNpFAwqwALEJVq=531187018; else oANAYpGjNcWAewojkNpFAwqwALEJVq=823304704;if (oANAYpGjNcWAewojkNpFAwqwALEJVq == oANAYpGjNcWAewojkNpFAwqwALEJVq- 0 ) oANAYpGjNcWAewojkNpFAwqwALEJVq=97789789; else oANAYpGjNcWAewojkNpFAwqwALEJVq=1138766088;if (oANAYpGjNcWAewojkNpFAwqwALEJVq == oANAYpGjNcWAewojkNpFAwqwALEJVq- 0 ) oANAYpGjNcWAewojkNpFAwqwALEJVq=116366935; else oANAYpGjNcWAewojkNpFAwqwALEJVq=1305168152;if (oANAYpGjNcWAewojkNpFAwqwALEJVq == oANAYpGjNcWAewojkNpFAwqwALEJVq- 0 ) oANAYpGjNcWAewojkNpFAwqwALEJVq=169633677; else oANAYpGjNcWAewojkNpFAwqwALEJVq=1222495575;float kpLPlnfBFEEAQVrzOqdATNbWEmgyda=441288813.820063270802365960685297464908f;if (kpLPlnfBFEEAQVrzOqdATNbWEmgyda - kpLPlnfBFEEAQVrzOqdATNbWEmgyda> 0.00000001 ) kpLPlnfBFEEAQVrzOqdATNbWEmgyda=1306940401.482560089713934150505818364370f; else kpLPlnfBFEEAQVrzOqdATNbWEmgyda=1008718702.037342081609108336441451468430f;if (kpLPlnfBFEEAQVrzOqdATNbWEmgyda - kpLPlnfBFEEAQVrzOqdATNbWEmgyda> 0.00000001 ) kpLPlnfBFEEAQVrzOqdATNbWEmgyda=455757757.672978679227991671113803079799f; else kpLPlnfBFEEAQVrzOqdATNbWEmgyda=2028756827.566203833076733456190743898901f;if (kpLPlnfBFEEAQVrzOqdATNbWEmgyda - kpLPlnfBFEEAQVrzOqdATNbWEmgyda> 0.00000001 ) kpLPlnfBFEEAQVrzOqdATNbWEmgyda=165172219.099186280895914717162471983342f; else kpLPlnfBFEEAQVrzOqdATNbWEmgyda=2003450494.594218866738730693028185063930f;if (kpLPlnfBFEEAQVrzOqdATNbWEmgyda - kpLPlnfBFEEAQVrzOqdATNbWEmgyda> 0.00000001 ) kpLPlnfBFEEAQVrzOqdATNbWEmgyda=762434388.395840271606243584140143542935f; else kpLPlnfBFEEAQVrzOqdATNbWEmgyda=311797596.594654390052353304269089136240f;if (kpLPlnfBFEEAQVrzOqdATNbWEmgyda - kpLPlnfBFEEAQVrzOqdATNbWEmgyda> 0.00000001 ) kpLPlnfBFEEAQVrzOqdATNbWEmgyda=1455505426.935611270929420999135878662662f; else kpLPlnfBFEEAQVrzOqdATNbWEmgyda=1606825242.926012775214445167925711526805f;if (kpLPlnfBFEEAQVrzOqdATNbWEmgyda - kpLPlnfBFEEAQVrzOqdATNbWEmgyda> 0.00000001 ) kpLPlnfBFEEAQVrzOqdATNbWEmgyda=1051166648.971900341922592244460674731660f; else kpLPlnfBFEEAQVrzOqdATNbWEmgyda=264397915.607044364592611323948355641728f;long luINKYdbpusxtkpAcSUCqgIHOaNVAT=1319217042;if (luINKYdbpusxtkpAcSUCqgIHOaNVAT == luINKYdbpusxtkpAcSUCqgIHOaNVAT- 1 ) luINKYdbpusxtkpAcSUCqgIHOaNVAT=2101018197; else luINKYdbpusxtkpAcSUCqgIHOaNVAT=1407381094;if (luINKYdbpusxtkpAcSUCqgIHOaNVAT == luINKYdbpusxtkpAcSUCqgIHOaNVAT- 0 ) luINKYdbpusxtkpAcSUCqgIHOaNVAT=1329717654; else luINKYdbpusxtkpAcSUCqgIHOaNVAT=1870307631;if (luINKYdbpusxtkpAcSUCqgIHOaNVAT == luINKYdbpusxtkpAcSUCqgIHOaNVAT- 0 ) luINKYdbpusxtkpAcSUCqgIHOaNVAT=1775846133; else luINKYdbpusxtkpAcSUCqgIHOaNVAT=1741619580;if (luINKYdbpusxtkpAcSUCqgIHOaNVAT == luINKYdbpusxtkpAcSUCqgIHOaNVAT- 0 ) luINKYdbpusxtkpAcSUCqgIHOaNVAT=1349757150; else luINKYdbpusxtkpAcSUCqgIHOaNVAT=444162501;if (luINKYdbpusxtkpAcSUCqgIHOaNVAT == luINKYdbpusxtkpAcSUCqgIHOaNVAT- 0 ) luINKYdbpusxtkpAcSUCqgIHOaNVAT=818659423; else luINKYdbpusxtkpAcSUCqgIHOaNVAT=629835594;if (luINKYdbpusxtkpAcSUCqgIHOaNVAT == luINKYdbpusxtkpAcSUCqgIHOaNVAT- 0 ) luINKYdbpusxtkpAcSUCqgIHOaNVAT=499186717; else luINKYdbpusxtkpAcSUCqgIHOaNVAT=510853900;int EclVkHcsYwSlOldiSNUWBvgZZOcFvZ=1871776029;if (EclVkHcsYwSlOldiSNUWBvgZZOcFvZ == EclVkHcsYwSlOldiSNUWBvgZZOcFvZ- 0 ) EclVkHcsYwSlOldiSNUWBvgZZOcFvZ=533864082; else EclVkHcsYwSlOldiSNUWBvgZZOcFvZ=1318205954;if (EclVkHcsYwSlOldiSNUWBvgZZOcFvZ == EclVkHcsYwSlOldiSNUWBvgZZOcFvZ- 1 ) EclVkHcsYwSlOldiSNUWBvgZZOcFvZ=262406974; else EclVkHcsYwSlOldiSNUWBvgZZOcFvZ=946441967;if (EclVkHcsYwSlOldiSNUWBvgZZOcFvZ == EclVkHcsYwSlOldiSNUWBvgZZOcFvZ- 1 ) EclVkHcsYwSlOldiSNUWBvgZZOcFvZ=1018122270; else EclVkHcsYwSlOldiSNUWBvgZZOcFvZ=454907027;if (EclVkHcsYwSlOldiSNUWBvgZZOcFvZ == EclVkHcsYwSlOldiSNUWBvgZZOcFvZ- 0 ) EclVkHcsYwSlOldiSNUWBvgZZOcFvZ=836307933; else EclVkHcsYwSlOldiSNUWBvgZZOcFvZ=1900402493;if (EclVkHcsYwSlOldiSNUWBvgZZOcFvZ == EclVkHcsYwSlOldiSNUWBvgZZOcFvZ- 1 ) EclVkHcsYwSlOldiSNUWBvgZZOcFvZ=914524238; else EclVkHcsYwSlOldiSNUWBvgZZOcFvZ=139692830;if (EclVkHcsYwSlOldiSNUWBvgZZOcFvZ == EclVkHcsYwSlOldiSNUWBvgZZOcFvZ- 0 ) EclVkHcsYwSlOldiSNUWBvgZZOcFvZ=770178454; else EclVkHcsYwSlOldiSNUWBvgZZOcFvZ=1071845423;int HqpybwDQmwLNyNjUpxLxcHMEQdnskV=1759257752;if (HqpybwDQmwLNyNjUpxLxcHMEQdnskV == HqpybwDQmwLNyNjUpxLxcHMEQdnskV- 0 ) HqpybwDQmwLNyNjUpxLxcHMEQdnskV=1312670386; else HqpybwDQmwLNyNjUpxLxcHMEQdnskV=997900282;if (HqpybwDQmwLNyNjUpxLxcHMEQdnskV == HqpybwDQmwLNyNjUpxLxcHMEQdnskV- 0 ) HqpybwDQmwLNyNjUpxLxcHMEQdnskV=1268961025; else HqpybwDQmwLNyNjUpxLxcHMEQdnskV=2042865404;if (HqpybwDQmwLNyNjUpxLxcHMEQdnskV == HqpybwDQmwLNyNjUpxLxcHMEQdnskV- 0 ) HqpybwDQmwLNyNjUpxLxcHMEQdnskV=874913295; else HqpybwDQmwLNyNjUpxLxcHMEQdnskV=1977245763;if (HqpybwDQmwLNyNjUpxLxcHMEQdnskV == HqpybwDQmwLNyNjUpxLxcHMEQdnskV- 0 ) HqpybwDQmwLNyNjUpxLxcHMEQdnskV=812926327; else HqpybwDQmwLNyNjUpxLxcHMEQdnskV=1712804468;if (HqpybwDQmwLNyNjUpxLxcHMEQdnskV == HqpybwDQmwLNyNjUpxLxcHMEQdnskV- 0 ) HqpybwDQmwLNyNjUpxLxcHMEQdnskV=1806445199; else HqpybwDQmwLNyNjUpxLxcHMEQdnskV=11595256;if (HqpybwDQmwLNyNjUpxLxcHMEQdnskV == HqpybwDQmwLNyNjUpxLxcHMEQdnskV- 0 ) HqpybwDQmwLNyNjUpxLxcHMEQdnskV=1210097404; else HqpybwDQmwLNyNjUpxLxcHMEQdnskV=1682820272;double hmlMUhvRtdYECCYEOkgqpoLveuUQms=1794680856.168428489762297880757891871698;if (hmlMUhvRtdYECCYEOkgqpoLveuUQms == hmlMUhvRtdYECCYEOkgqpoLveuUQms ) hmlMUhvRtdYECCYEOkgqpoLveuUQms=1354399065.378642481979846126837750148876; else hmlMUhvRtdYECCYEOkgqpoLveuUQms=1326874266.899259566550989816575587263860;if (hmlMUhvRtdYECCYEOkgqpoLveuUQms == hmlMUhvRtdYECCYEOkgqpoLveuUQms ) hmlMUhvRtdYECCYEOkgqpoLveuUQms=1878017480.471628079867139798244330131309; else hmlMUhvRtdYECCYEOkgqpoLveuUQms=238858748.771628947194781956868672725150;if (hmlMUhvRtdYECCYEOkgqpoLveuUQms == hmlMUhvRtdYECCYEOkgqpoLveuUQms ) hmlMUhvRtdYECCYEOkgqpoLveuUQms=242185856.887020652185115065038655220900; else hmlMUhvRtdYECCYEOkgqpoLveuUQms=1193539446.489831777231488357776022233390;if (hmlMUhvRtdYECCYEOkgqpoLveuUQms == hmlMUhvRtdYECCYEOkgqpoLveuUQms ) hmlMUhvRtdYECCYEOkgqpoLveuUQms=301094772.086280748530981828798423557861; else hmlMUhvRtdYECCYEOkgqpoLveuUQms=1646111170.958726530871935714252561376483;if (hmlMUhvRtdYECCYEOkgqpoLveuUQms == hmlMUhvRtdYECCYEOkgqpoLveuUQms ) hmlMUhvRtdYECCYEOkgqpoLveuUQms=571073161.499769237539892769884362059846; else hmlMUhvRtdYECCYEOkgqpoLveuUQms=1576798808.025858328659873377062760924782;if (hmlMUhvRtdYECCYEOkgqpoLveuUQms == hmlMUhvRtdYECCYEOkgqpoLveuUQms ) hmlMUhvRtdYECCYEOkgqpoLveuUQms=955783393.059161515454234650088398547594; else hmlMUhvRtdYECCYEOkgqpoLveuUQms=1636153303.565670182287362864747580587173;double ALEuuwceTZnnpAFyMjjoRsXjBsXJvS=985267549.402653042313922533250074549962;if (ALEuuwceTZnnpAFyMjjoRsXjBsXJvS == ALEuuwceTZnnpAFyMjjoRsXjBsXJvS ) ALEuuwceTZnnpAFyMjjoRsXjBsXJvS=1048134208.244700000074822039227764754921; else ALEuuwceTZnnpAFyMjjoRsXjBsXJvS=887412612.528118923254338726252074569016;if (ALEuuwceTZnnpAFyMjjoRsXjBsXJvS == ALEuuwceTZnnpAFyMjjoRsXjBsXJvS ) ALEuuwceTZnnpAFyMjjoRsXjBsXJvS=1345667161.728056681412718353703877066566; else ALEuuwceTZnnpAFyMjjoRsXjBsXJvS=1711597690.092853648324585036622641305200;if (ALEuuwceTZnnpAFyMjjoRsXjBsXJvS == ALEuuwceTZnnpAFyMjjoRsXjBsXJvS ) ALEuuwceTZnnpAFyMjjoRsXjBsXJvS=539347192.113652107581659949203094556256; else ALEuuwceTZnnpAFyMjjoRsXjBsXJvS=357044420.854931294404025100246701713920;if (ALEuuwceTZnnpAFyMjjoRsXjBsXJvS == ALEuuwceTZnnpAFyMjjoRsXjBsXJvS ) ALEuuwceTZnnpAFyMjjoRsXjBsXJvS=996862734.606273883985691018045701026422; else ALEuuwceTZnnpAFyMjjoRsXjBsXJvS=704101966.948481985343847111070508600526;if (ALEuuwceTZnnpAFyMjjoRsXjBsXJvS == ALEuuwceTZnnpAFyMjjoRsXjBsXJvS ) ALEuuwceTZnnpAFyMjjoRsXjBsXJvS=505279627.846115822024371397402546344198; else ALEuuwceTZnnpAFyMjjoRsXjBsXJvS=697086153.147060784349016733731413431187;if (ALEuuwceTZnnpAFyMjjoRsXjBsXJvS == ALEuuwceTZnnpAFyMjjoRsXjBsXJvS ) ALEuuwceTZnnpAFyMjjoRsXjBsXJvS=1287480415.184408742262427364072146784464; else ALEuuwceTZnnpAFyMjjoRsXjBsXJvS=1006527014.970736661809258000717811605872;double QpsOuJkwCnAMOlOxrsonTkYhIevDLH=662943750.236067432964827151357069414187;if (QpsOuJkwCnAMOlOxrsonTkYhIevDLH == QpsOuJkwCnAMOlOxrsonTkYhIevDLH ) QpsOuJkwCnAMOlOxrsonTkYhIevDLH=389909559.633065708047711632476207627765; else QpsOuJkwCnAMOlOxrsonTkYhIevDLH=1122981245.336533197346442046988381243654;if (QpsOuJkwCnAMOlOxrsonTkYhIevDLH == QpsOuJkwCnAMOlOxrsonTkYhIevDLH ) QpsOuJkwCnAMOlOxrsonTkYhIevDLH=1864930005.804471228865623197669298716539; else QpsOuJkwCnAMOlOxrsonTkYhIevDLH=592747437.055355217292823476926719561776;if (QpsOuJkwCnAMOlOxrsonTkYhIevDLH == QpsOuJkwCnAMOlOxrsonTkYhIevDLH ) QpsOuJkwCnAMOlOxrsonTkYhIevDLH=102858041.580081147809048757645062445689; else QpsOuJkwCnAMOlOxrsonTkYhIevDLH=1490326285.464271394334023877282414262864;if (QpsOuJkwCnAMOlOxrsonTkYhIevDLH == QpsOuJkwCnAMOlOxrsonTkYhIevDLH ) QpsOuJkwCnAMOlOxrsonTkYhIevDLH=1517071896.666844598757431115078850805102; else QpsOuJkwCnAMOlOxrsonTkYhIevDLH=119130940.953078928939054619325890791614;if (QpsOuJkwCnAMOlOxrsonTkYhIevDLH == QpsOuJkwCnAMOlOxrsonTkYhIevDLH ) QpsOuJkwCnAMOlOxrsonTkYhIevDLH=696805510.049680546430724090730699727349; else QpsOuJkwCnAMOlOxrsonTkYhIevDLH=1770333502.741590047528499617780299556196;if (QpsOuJkwCnAMOlOxrsonTkYhIevDLH == QpsOuJkwCnAMOlOxrsonTkYhIevDLH ) QpsOuJkwCnAMOlOxrsonTkYhIevDLH=1471792243.320780933515432230022067965304; else QpsOuJkwCnAMOlOxrsonTkYhIevDLH=1445598134.495120872078538687832718537493;int ePuDHzNCKXfsAaWdNvQlmdmKlomhuN=1528533472;if (ePuDHzNCKXfsAaWdNvQlmdmKlomhuN == ePuDHzNCKXfsAaWdNvQlmdmKlomhuN- 0 ) ePuDHzNCKXfsAaWdNvQlmdmKlomhuN=1487054458; else ePuDHzNCKXfsAaWdNvQlmdmKlomhuN=224829855;if (ePuDHzNCKXfsAaWdNvQlmdmKlomhuN == ePuDHzNCKXfsAaWdNvQlmdmKlomhuN- 0 ) ePuDHzNCKXfsAaWdNvQlmdmKlomhuN=634686064; else ePuDHzNCKXfsAaWdNvQlmdmKlomhuN=616851165;if (ePuDHzNCKXfsAaWdNvQlmdmKlomhuN == ePuDHzNCKXfsAaWdNvQlmdmKlomhuN- 1 ) ePuDHzNCKXfsAaWdNvQlmdmKlomhuN=1053396974; else ePuDHzNCKXfsAaWdNvQlmdmKlomhuN=185669228;if (ePuDHzNCKXfsAaWdNvQlmdmKlomhuN == ePuDHzNCKXfsAaWdNvQlmdmKlomhuN- 0 ) ePuDHzNCKXfsAaWdNvQlmdmKlomhuN=1255333664; else ePuDHzNCKXfsAaWdNvQlmdmKlomhuN=997354645;if (ePuDHzNCKXfsAaWdNvQlmdmKlomhuN == ePuDHzNCKXfsAaWdNvQlmdmKlomhuN- 1 ) ePuDHzNCKXfsAaWdNvQlmdmKlomhuN=330011532; else ePuDHzNCKXfsAaWdNvQlmdmKlomhuN=1173915514;if (ePuDHzNCKXfsAaWdNvQlmdmKlomhuN == ePuDHzNCKXfsAaWdNvQlmdmKlomhuN- 0 ) ePuDHzNCKXfsAaWdNvQlmdmKlomhuN=1881062768; else ePuDHzNCKXfsAaWdNvQlmdmKlomhuN=1692583350;long rcLJihDrQQQnIvzYVVhoYpgGpgVPSU=1417585500;if (rcLJihDrQQQnIvzYVVhoYpgGpgVPSU == rcLJihDrQQQnIvzYVVhoYpgGpgVPSU- 0 ) rcLJihDrQQQnIvzYVVhoYpgGpgVPSU=1180660092; else rcLJihDrQQQnIvzYVVhoYpgGpgVPSU=292839643;if (rcLJihDrQQQnIvzYVVhoYpgGpgVPSU == rcLJihDrQQQnIvzYVVhoYpgGpgVPSU- 0 ) rcLJihDrQQQnIvzYVVhoYpgGpgVPSU=1246243744; else rcLJihDrQQQnIvzYVVhoYpgGpgVPSU=1995943528;if (rcLJihDrQQQnIvzYVVhoYpgGpgVPSU == rcLJihDrQQQnIvzYVVhoYpgGpgVPSU- 0 ) rcLJihDrQQQnIvzYVVhoYpgGpgVPSU=560465369; else rcLJihDrQQQnIvzYVVhoYpgGpgVPSU=79277008;if (rcLJihDrQQQnIvzYVVhoYpgGpgVPSU == rcLJihDrQQQnIvzYVVhoYpgGpgVPSU- 0 ) rcLJihDrQQQnIvzYVVhoYpgGpgVPSU=882409508; else rcLJihDrQQQnIvzYVVhoYpgGpgVPSU=1705291570;if (rcLJihDrQQQnIvzYVVhoYpgGpgVPSU == rcLJihDrQQQnIvzYVVhoYpgGpgVPSU- 1 ) rcLJihDrQQQnIvzYVVhoYpgGpgVPSU=1517824363; else rcLJihDrQQQnIvzYVVhoYpgGpgVPSU=1226536221;if (rcLJihDrQQQnIvzYVVhoYpgGpgVPSU == rcLJihDrQQQnIvzYVVhoYpgGpgVPSU- 0 ) rcLJihDrQQQnIvzYVVhoYpgGpgVPSU=1014792180; else rcLJihDrQQQnIvzYVVhoYpgGpgVPSU=916497337;long dsJqUOXtnhoPqFVsEUqPihIkcMMvST=1328506603;if (dsJqUOXtnhoPqFVsEUqPihIkcMMvST == dsJqUOXtnhoPqFVsEUqPihIkcMMvST- 0 ) dsJqUOXtnhoPqFVsEUqPihIkcMMvST=2103206458; else dsJqUOXtnhoPqFVsEUqPihIkcMMvST=1906430590;if (dsJqUOXtnhoPqFVsEUqPihIkcMMvST == dsJqUOXtnhoPqFVsEUqPihIkcMMvST- 1 ) dsJqUOXtnhoPqFVsEUqPihIkcMMvST=1666956203; else dsJqUOXtnhoPqFVsEUqPihIkcMMvST=1167463415;if (dsJqUOXtnhoPqFVsEUqPihIkcMMvST == dsJqUOXtnhoPqFVsEUqPihIkcMMvST- 1 ) dsJqUOXtnhoPqFVsEUqPihIkcMMvST=666285244; else dsJqUOXtnhoPqFVsEUqPihIkcMMvST=1926771198;if (dsJqUOXtnhoPqFVsEUqPihIkcMMvST == dsJqUOXtnhoPqFVsEUqPihIkcMMvST- 1 ) dsJqUOXtnhoPqFVsEUqPihIkcMMvST=1757030423; else dsJqUOXtnhoPqFVsEUqPihIkcMMvST=204240331;if (dsJqUOXtnhoPqFVsEUqPihIkcMMvST == dsJqUOXtnhoPqFVsEUqPihIkcMMvST- 1 ) dsJqUOXtnhoPqFVsEUqPihIkcMMvST=2015131228; else dsJqUOXtnhoPqFVsEUqPihIkcMMvST=867575360;if (dsJqUOXtnhoPqFVsEUqPihIkcMMvST == dsJqUOXtnhoPqFVsEUqPihIkcMMvST- 0 ) dsJqUOXtnhoPqFVsEUqPihIkcMMvST=262735905; else dsJqUOXtnhoPqFVsEUqPihIkcMMvST=1076426174;int oHZaIMpLGBvkjYftMjuktJWhtvPvWp=1402237752;if (oHZaIMpLGBvkjYftMjuktJWhtvPvWp == oHZaIMpLGBvkjYftMjuktJWhtvPvWp- 0 ) oHZaIMpLGBvkjYftMjuktJWhtvPvWp=1516515602; else oHZaIMpLGBvkjYftMjuktJWhtvPvWp=54206743;if (oHZaIMpLGBvkjYftMjuktJWhtvPvWp == oHZaIMpLGBvkjYftMjuktJWhtvPvWp- 0 ) oHZaIMpLGBvkjYftMjuktJWhtvPvWp=264938042; else oHZaIMpLGBvkjYftMjuktJWhtvPvWp=728622727;if (oHZaIMpLGBvkjYftMjuktJWhtvPvWp == oHZaIMpLGBvkjYftMjuktJWhtvPvWp- 0 ) oHZaIMpLGBvkjYftMjuktJWhtvPvWp=78012927; else oHZaIMpLGBvkjYftMjuktJWhtvPvWp=892903461;if (oHZaIMpLGBvkjYftMjuktJWhtvPvWp == oHZaIMpLGBvkjYftMjuktJWhtvPvWp- 0 ) oHZaIMpLGBvkjYftMjuktJWhtvPvWp=968068103; else oHZaIMpLGBvkjYftMjuktJWhtvPvWp=1407777450;if (oHZaIMpLGBvkjYftMjuktJWhtvPvWp == oHZaIMpLGBvkjYftMjuktJWhtvPvWp- 0 ) oHZaIMpLGBvkjYftMjuktJWhtvPvWp=1489903994; else oHZaIMpLGBvkjYftMjuktJWhtvPvWp=1076878141;if (oHZaIMpLGBvkjYftMjuktJWhtvPvWp == oHZaIMpLGBvkjYftMjuktJWhtvPvWp- 0 ) oHZaIMpLGBvkjYftMjuktJWhtvPvWp=1246510449; else oHZaIMpLGBvkjYftMjuktJWhtvPvWp=1974344400;float mssnEaLXSwTvCqozvVHBBESYmZVtIL=778746932.515389466386166102184442861178f;if (mssnEaLXSwTvCqozvVHBBESYmZVtIL - mssnEaLXSwTvCqozvVHBBESYmZVtIL> 0.00000001 ) mssnEaLXSwTvCqozvVHBBESYmZVtIL=1491275918.202745183258937179532150528363f; else mssnEaLXSwTvCqozvVHBBESYmZVtIL=552556165.849919542113093752579365726233f;if (mssnEaLXSwTvCqozvVHBBESYmZVtIL - mssnEaLXSwTvCqozvVHBBESYmZVtIL> 0.00000001 ) mssnEaLXSwTvCqozvVHBBESYmZVtIL=1736860539.883042912964753026887799569520f; else mssnEaLXSwTvCqozvVHBBESYmZVtIL=147112088.342043006285062661270155720568f;if (mssnEaLXSwTvCqozvVHBBESYmZVtIL - mssnEaLXSwTvCqozvVHBBESYmZVtIL> 0.00000001 ) mssnEaLXSwTvCqozvVHBBESYmZVtIL=1922578849.459273281003060373800836139743f; else mssnEaLXSwTvCqozvVHBBESYmZVtIL=1233838132.464894690949137719645787962424f;if (mssnEaLXSwTvCqozvVHBBESYmZVtIL - mssnEaLXSwTvCqozvVHBBESYmZVtIL> 0.00000001 ) mssnEaLXSwTvCqozvVHBBESYmZVtIL=751300256.465974038689396271943110879703f; else mssnEaLXSwTvCqozvVHBBESYmZVtIL=1401372541.327768437389207749180168245610f;if (mssnEaLXSwTvCqozvVHBBESYmZVtIL - mssnEaLXSwTvCqozvVHBBESYmZVtIL> 0.00000001 ) mssnEaLXSwTvCqozvVHBBESYmZVtIL=683292867.633636274885567402324163437062f; else mssnEaLXSwTvCqozvVHBBESYmZVtIL=1042003413.492785677268640560285430668121f;if (mssnEaLXSwTvCqozvVHBBESYmZVtIL - mssnEaLXSwTvCqozvVHBBESYmZVtIL> 0.00000001 ) mssnEaLXSwTvCqozvVHBBESYmZVtIL=2128295947.922259488294526753307888816313f; else mssnEaLXSwTvCqozvVHBBESYmZVtIL=1840373656.855634649218209292401593524444f;float GjbVUttqKDDTGrsbpvjKWrGJNgHJHy=1693335817.645940113557924384407844825580f;if (GjbVUttqKDDTGrsbpvjKWrGJNgHJHy - GjbVUttqKDDTGrsbpvjKWrGJNgHJHy> 0.00000001 ) GjbVUttqKDDTGrsbpvjKWrGJNgHJHy=1013464312.720285179030295907322100591212f; else GjbVUttqKDDTGrsbpvjKWrGJNgHJHy=1760038075.563394885922548765903937863180f;if (GjbVUttqKDDTGrsbpvjKWrGJNgHJHy - GjbVUttqKDDTGrsbpvjKWrGJNgHJHy> 0.00000001 ) GjbVUttqKDDTGrsbpvjKWrGJNgHJHy=1547465382.406757801213340176283732074763f; else GjbVUttqKDDTGrsbpvjKWrGJNgHJHy=978529081.180663789107570169880790787588f;if (GjbVUttqKDDTGrsbpvjKWrGJNgHJHy - GjbVUttqKDDTGrsbpvjKWrGJNgHJHy> 0.00000001 ) GjbVUttqKDDTGrsbpvjKWrGJNgHJHy=1433810205.415914388759829737292508359199f; else GjbVUttqKDDTGrsbpvjKWrGJNgHJHy=1101500516.620180366699213298205130792777f;if (GjbVUttqKDDTGrsbpvjKWrGJNgHJHy - GjbVUttqKDDTGrsbpvjKWrGJNgHJHy> 0.00000001 ) GjbVUttqKDDTGrsbpvjKWrGJNgHJHy=1250493676.529023974957170440616209450150f; else GjbVUttqKDDTGrsbpvjKWrGJNgHJHy=1210302997.870018029033436916045316132067f;if (GjbVUttqKDDTGrsbpvjKWrGJNgHJHy - GjbVUttqKDDTGrsbpvjKWrGJNgHJHy> 0.00000001 ) GjbVUttqKDDTGrsbpvjKWrGJNgHJHy=938362487.482528816343168498465412666508f; else GjbVUttqKDDTGrsbpvjKWrGJNgHJHy=259178753.607375192921180326875571318106f;if (GjbVUttqKDDTGrsbpvjKWrGJNgHJHy - GjbVUttqKDDTGrsbpvjKWrGJNgHJHy> 0.00000001 ) GjbVUttqKDDTGrsbpvjKWrGJNgHJHy=1825043068.331875790430765443245183423607f; else GjbVUttqKDDTGrsbpvjKWrGJNgHJHy=1736890750.800669032664755582608507236855f;float BtPcPDCvfSDEdkEUOrvUFzCuNAOYTN=1676294901.217463916694960702207447237589f;if (BtPcPDCvfSDEdkEUOrvUFzCuNAOYTN - BtPcPDCvfSDEdkEUOrvUFzCuNAOYTN> 0.00000001 ) BtPcPDCvfSDEdkEUOrvUFzCuNAOYTN=902956539.365503328348820579305385097024f; else BtPcPDCvfSDEdkEUOrvUFzCuNAOYTN=1631472620.743734947748597062886938519062f;if (BtPcPDCvfSDEdkEUOrvUFzCuNAOYTN - BtPcPDCvfSDEdkEUOrvUFzCuNAOYTN> 0.00000001 ) BtPcPDCvfSDEdkEUOrvUFzCuNAOYTN=1732633903.640561519674277889903535891633f; else BtPcPDCvfSDEdkEUOrvUFzCuNAOYTN=1204190999.609511814666322377273117911678f;if (BtPcPDCvfSDEdkEUOrvUFzCuNAOYTN - BtPcPDCvfSDEdkEUOrvUFzCuNAOYTN> 0.00000001 ) BtPcPDCvfSDEdkEUOrvUFzCuNAOYTN=235111654.098717586708544831462546069370f; else BtPcPDCvfSDEdkEUOrvUFzCuNAOYTN=65959821.034677871580168668329122656286f;if (BtPcPDCvfSDEdkEUOrvUFzCuNAOYTN - BtPcPDCvfSDEdkEUOrvUFzCuNAOYTN> 0.00000001 ) BtPcPDCvfSDEdkEUOrvUFzCuNAOYTN=1627142550.931778507706008359950105101202f; else BtPcPDCvfSDEdkEUOrvUFzCuNAOYTN=282677470.260665959126262413530743684768f;if (BtPcPDCvfSDEdkEUOrvUFzCuNAOYTN - BtPcPDCvfSDEdkEUOrvUFzCuNAOYTN> 0.00000001 ) BtPcPDCvfSDEdkEUOrvUFzCuNAOYTN=1450390134.730565754351194504389987353709f; else BtPcPDCvfSDEdkEUOrvUFzCuNAOYTN=1011049148.220149771876929076929048985331f;if (BtPcPDCvfSDEdkEUOrvUFzCuNAOYTN - BtPcPDCvfSDEdkEUOrvUFzCuNAOYTN> 0.00000001 ) BtPcPDCvfSDEdkEUOrvUFzCuNAOYTN=45329860.305623205594452417939889138227f; else BtPcPDCvfSDEdkEUOrvUFzCuNAOYTN=1183493161.544067141698154560668950353769f;int vosAaxTwYMjeRWFiSZbktffrhuUnDY=1683920660;if (vosAaxTwYMjeRWFiSZbktffrhuUnDY == vosAaxTwYMjeRWFiSZbktffrhuUnDY- 1 ) vosAaxTwYMjeRWFiSZbktffrhuUnDY=1128333735; else vosAaxTwYMjeRWFiSZbktffrhuUnDY=1700658279;if (vosAaxTwYMjeRWFiSZbktffrhuUnDY == vosAaxTwYMjeRWFiSZbktffrhuUnDY- 1 ) vosAaxTwYMjeRWFiSZbktffrhuUnDY=960880088; else vosAaxTwYMjeRWFiSZbktffrhuUnDY=1705806267;if (vosAaxTwYMjeRWFiSZbktffrhuUnDY == vosAaxTwYMjeRWFiSZbktffrhuUnDY- 1 ) vosAaxTwYMjeRWFiSZbktffrhuUnDY=938854592; else vosAaxTwYMjeRWFiSZbktffrhuUnDY=1133573387;if (vosAaxTwYMjeRWFiSZbktffrhuUnDY == vosAaxTwYMjeRWFiSZbktffrhuUnDY- 1 ) vosAaxTwYMjeRWFiSZbktffrhuUnDY=1492420374; else vosAaxTwYMjeRWFiSZbktffrhuUnDY=389775402;if (vosAaxTwYMjeRWFiSZbktffrhuUnDY == vosAaxTwYMjeRWFiSZbktffrhuUnDY- 1 ) vosAaxTwYMjeRWFiSZbktffrhuUnDY=1449546136; else vosAaxTwYMjeRWFiSZbktffrhuUnDY=526557542;if (vosAaxTwYMjeRWFiSZbktffrhuUnDY == vosAaxTwYMjeRWFiSZbktffrhuUnDY- 1 ) vosAaxTwYMjeRWFiSZbktffrhuUnDY=1356233248; else vosAaxTwYMjeRWFiSZbktffrhuUnDY=1663400096;int oIhlSPpCvxdYyHiGAOTkpKTSVLOuwQ=900606844;if (oIhlSPpCvxdYyHiGAOTkpKTSVLOuwQ == oIhlSPpCvxdYyHiGAOTkpKTSVLOuwQ- 1 ) oIhlSPpCvxdYyHiGAOTkpKTSVLOuwQ=8640130; else oIhlSPpCvxdYyHiGAOTkpKTSVLOuwQ=452001046;if (oIhlSPpCvxdYyHiGAOTkpKTSVLOuwQ == oIhlSPpCvxdYyHiGAOTkpKTSVLOuwQ- 0 ) oIhlSPpCvxdYyHiGAOTkpKTSVLOuwQ=937417117; else oIhlSPpCvxdYyHiGAOTkpKTSVLOuwQ=332552130;if (oIhlSPpCvxdYyHiGAOTkpKTSVLOuwQ == oIhlSPpCvxdYyHiGAOTkpKTSVLOuwQ- 0 ) oIhlSPpCvxdYyHiGAOTkpKTSVLOuwQ=220833078; else oIhlSPpCvxdYyHiGAOTkpKTSVLOuwQ=891837054;if (oIhlSPpCvxdYyHiGAOTkpKTSVLOuwQ == oIhlSPpCvxdYyHiGAOTkpKTSVLOuwQ- 0 ) oIhlSPpCvxdYyHiGAOTkpKTSVLOuwQ=1458224163; else oIhlSPpCvxdYyHiGAOTkpKTSVLOuwQ=947504491;if (oIhlSPpCvxdYyHiGAOTkpKTSVLOuwQ == oIhlSPpCvxdYyHiGAOTkpKTSVLOuwQ- 1 ) oIhlSPpCvxdYyHiGAOTkpKTSVLOuwQ=132895525; else oIhlSPpCvxdYyHiGAOTkpKTSVLOuwQ=1264787912;if (oIhlSPpCvxdYyHiGAOTkpKTSVLOuwQ == oIhlSPpCvxdYyHiGAOTkpKTSVLOuwQ- 0 ) oIhlSPpCvxdYyHiGAOTkpKTSVLOuwQ=1675622594; else oIhlSPpCvxdYyHiGAOTkpKTSVLOuwQ=422761057;float wsIueKaPiTcqgaMqCiqbLpkPcBBNjT=1810668040.910876727572520488541558711474f;if (wsIueKaPiTcqgaMqCiqbLpkPcBBNjT - wsIueKaPiTcqgaMqCiqbLpkPcBBNjT> 0.00000001 ) wsIueKaPiTcqgaMqCiqbLpkPcBBNjT=815426053.647699092967926679828989657316f; else wsIueKaPiTcqgaMqCiqbLpkPcBBNjT=141122408.645404693249778564315276676904f;if (wsIueKaPiTcqgaMqCiqbLpkPcBBNjT - wsIueKaPiTcqgaMqCiqbLpkPcBBNjT> 0.00000001 ) wsIueKaPiTcqgaMqCiqbLpkPcBBNjT=608557015.389643262218669032129410148084f; else wsIueKaPiTcqgaMqCiqbLpkPcBBNjT=1127086117.980097113631211861774589607951f;if (wsIueKaPiTcqgaMqCiqbLpkPcBBNjT - wsIueKaPiTcqgaMqCiqbLpkPcBBNjT> 0.00000001 ) wsIueKaPiTcqgaMqCiqbLpkPcBBNjT=969289582.539944978392960821048831040240f; else wsIueKaPiTcqgaMqCiqbLpkPcBBNjT=1475040663.027321347700358363088791380800f;if (wsIueKaPiTcqgaMqCiqbLpkPcBBNjT - wsIueKaPiTcqgaMqCiqbLpkPcBBNjT> 0.00000001 ) wsIueKaPiTcqgaMqCiqbLpkPcBBNjT=737440309.483848674969435959379827233604f; else wsIueKaPiTcqgaMqCiqbLpkPcBBNjT=1916866799.154733762179912047753263276661f;if (wsIueKaPiTcqgaMqCiqbLpkPcBBNjT - wsIueKaPiTcqgaMqCiqbLpkPcBBNjT> 0.00000001 ) wsIueKaPiTcqgaMqCiqbLpkPcBBNjT=139052246.517181670220721002539795100267f; else wsIueKaPiTcqgaMqCiqbLpkPcBBNjT=1342858501.810177283247754454327423382309f;if (wsIueKaPiTcqgaMqCiqbLpkPcBBNjT - wsIueKaPiTcqgaMqCiqbLpkPcBBNjT> 0.00000001 ) wsIueKaPiTcqgaMqCiqbLpkPcBBNjT=1902128510.141451176582049214286704040799f; else wsIueKaPiTcqgaMqCiqbLpkPcBBNjT=1856037759.509511641349984658518813355742f; }
 wsIueKaPiTcqgaMqCiqbLpkPcBBNjTy::wsIueKaPiTcqgaMqCiqbLpkPcBBNjTy()
 { this->eLnDyjbbeLMh("VbQTfNFUDcBNeNTGijjFjBrzGIsScIeLnDyjbbeLMhj", true, 857210220, 2111226101, 873882406); }
#pragma optimize("", off)
 // <delete/>


// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class wACMRLQYnvDRUyyZavqwYxmQNLeVEPy
 { 
public: bool MCEDalHtdJayiWbIEHFePbyzyNlyGh; double MCEDalHtdJayiWbIEHFePbyzyNlyGhwACMRLQYnvDRUyyZavqwYxmQNLeVEP; wACMRLQYnvDRUyyZavqwYxmQNLeVEPy(); void OgcoGkboYdwF(string MCEDalHtdJayiWbIEHFePbyzyNlyGhOgcoGkboYdwF, bool uUYRrPQuXoSBNnyyDtlCZOzlNkDEEM, int zInvPGaQfMsJjRLNNKBzJrMEHfVnEG, float LLpKoKORYrYnOZJQehZzXtDFDIwioj, long lnLZAgTZcBywmJbWdnuqAnNEjSIkHg);
 protected: bool MCEDalHtdJayiWbIEHFePbyzyNlyGho; double MCEDalHtdJayiWbIEHFePbyzyNlyGhwACMRLQYnvDRUyyZavqwYxmQNLeVEPf; void OgcoGkboYdwFu(string MCEDalHtdJayiWbIEHFePbyzyNlyGhOgcoGkboYdwFg, bool uUYRrPQuXoSBNnyyDtlCZOzlNkDEEMe, int zInvPGaQfMsJjRLNNKBzJrMEHfVnEGr, float LLpKoKORYrYnOZJQehZzXtDFDIwiojw, long lnLZAgTZcBywmJbWdnuqAnNEjSIkHgn);
 private: bool MCEDalHtdJayiWbIEHFePbyzyNlyGhuUYRrPQuXoSBNnyyDtlCZOzlNkDEEM; double MCEDalHtdJayiWbIEHFePbyzyNlyGhLLpKoKORYrYnOZJQehZzXtDFDIwiojwACMRLQYnvDRUyyZavqwYxmQNLeVEP;
 void OgcoGkboYdwFv(string uUYRrPQuXoSBNnyyDtlCZOzlNkDEEMOgcoGkboYdwF, bool uUYRrPQuXoSBNnyyDtlCZOzlNkDEEMzInvPGaQfMsJjRLNNKBzJrMEHfVnEG, int zInvPGaQfMsJjRLNNKBzJrMEHfVnEGMCEDalHtdJayiWbIEHFePbyzyNlyGh, float LLpKoKORYrYnOZJQehZzXtDFDIwiojlnLZAgTZcBywmJbWdnuqAnNEjSIkHg, long lnLZAgTZcBywmJbWdnuqAnNEjSIkHguUYRrPQuXoSBNnyyDtlCZOzlNkDEEM); };
 void wACMRLQYnvDRUyyZavqwYxmQNLeVEPy::OgcoGkboYdwF(string MCEDalHtdJayiWbIEHFePbyzyNlyGhOgcoGkboYdwF, bool uUYRrPQuXoSBNnyyDtlCZOzlNkDEEM, int zInvPGaQfMsJjRLNNKBzJrMEHfVnEG, float LLpKoKORYrYnOZJQehZzXtDFDIwioj, long lnLZAgTZcBywmJbWdnuqAnNEjSIkHg)
 { double ARaZZRqpwDkrGYOmTqADROTzWtfOqY=855680801.370637863696232112386076672295;if (ARaZZRqpwDkrGYOmTqADROTzWtfOqY == ARaZZRqpwDkrGYOmTqADROTzWtfOqY ) ARaZZRqpwDkrGYOmTqADROTzWtfOqY=1444074399.016779044849081171404811231506; else ARaZZRqpwDkrGYOmTqADROTzWtfOqY=1697162538.667968057327854719651290131664;if (ARaZZRqpwDkrGYOmTqADROTzWtfOqY == ARaZZRqpwDkrGYOmTqADROTzWtfOqY ) ARaZZRqpwDkrGYOmTqADROTzWtfOqY=750664470.965839312312429706753295658884; else ARaZZRqpwDkrGYOmTqADROTzWtfOqY=1259515835.850347313650834987143591287029;if (ARaZZRqpwDkrGYOmTqADROTzWtfOqY == ARaZZRqpwDkrGYOmTqADROTzWtfOqY ) ARaZZRqpwDkrGYOmTqADROTzWtfOqY=82001490.154967329350411347278175137026; else ARaZZRqpwDkrGYOmTqADROTzWtfOqY=326371875.049450708876990418195086119083;if (ARaZZRqpwDkrGYOmTqADROTzWtfOqY == ARaZZRqpwDkrGYOmTqADROTzWtfOqY ) ARaZZRqpwDkrGYOmTqADROTzWtfOqY=1746945783.131431106175040238572993216445; else ARaZZRqpwDkrGYOmTqADROTzWtfOqY=830406113.282585657042213015609151225833;if (ARaZZRqpwDkrGYOmTqADROTzWtfOqY == ARaZZRqpwDkrGYOmTqADROTzWtfOqY ) ARaZZRqpwDkrGYOmTqADROTzWtfOqY=1497980135.587299467668264189121284624146; else ARaZZRqpwDkrGYOmTqADROTzWtfOqY=939273678.093456502475904299815681331378;if (ARaZZRqpwDkrGYOmTqADROTzWtfOqY == ARaZZRqpwDkrGYOmTqADROTzWtfOqY ) ARaZZRqpwDkrGYOmTqADROTzWtfOqY=462874569.633168591968886648153543251322; else ARaZZRqpwDkrGYOmTqADROTzWtfOqY=28651170.624484385781805891680332520002;int LhtNURQxINbRUkLjVmVeEkYKOvJugX=1603057153;if (LhtNURQxINbRUkLjVmVeEkYKOvJugX == LhtNURQxINbRUkLjVmVeEkYKOvJugX- 0 ) LhtNURQxINbRUkLjVmVeEkYKOvJugX=1502019850; else LhtNURQxINbRUkLjVmVeEkYKOvJugX=1095193491;if (LhtNURQxINbRUkLjVmVeEkYKOvJugX == LhtNURQxINbRUkLjVmVeEkYKOvJugX- 1 ) LhtNURQxINbRUkLjVmVeEkYKOvJugX=1672608164; else LhtNURQxINbRUkLjVmVeEkYKOvJugX=723292045;if (LhtNURQxINbRUkLjVmVeEkYKOvJugX == LhtNURQxINbRUkLjVmVeEkYKOvJugX- 1 ) LhtNURQxINbRUkLjVmVeEkYKOvJugX=439439353; else LhtNURQxINbRUkLjVmVeEkYKOvJugX=99275364;if (LhtNURQxINbRUkLjVmVeEkYKOvJugX == LhtNURQxINbRUkLjVmVeEkYKOvJugX- 0 ) LhtNURQxINbRUkLjVmVeEkYKOvJugX=2127826513; else LhtNURQxINbRUkLjVmVeEkYKOvJugX=1646025638;if (LhtNURQxINbRUkLjVmVeEkYKOvJugX == LhtNURQxINbRUkLjVmVeEkYKOvJugX- 1 ) LhtNURQxINbRUkLjVmVeEkYKOvJugX=575559810; else LhtNURQxINbRUkLjVmVeEkYKOvJugX=700264442;if (LhtNURQxINbRUkLjVmVeEkYKOvJugX == LhtNURQxINbRUkLjVmVeEkYKOvJugX- 1 ) LhtNURQxINbRUkLjVmVeEkYKOvJugX=493974284; else LhtNURQxINbRUkLjVmVeEkYKOvJugX=875823630;int ZTuRidBSzCXKeUsflhnjidXKhlWIyi=46369298;if (ZTuRidBSzCXKeUsflhnjidXKhlWIyi == ZTuRidBSzCXKeUsflhnjidXKhlWIyi- 1 ) ZTuRidBSzCXKeUsflhnjidXKhlWIyi=886811335; else ZTuRidBSzCXKeUsflhnjidXKhlWIyi=1047465268;if (ZTuRidBSzCXKeUsflhnjidXKhlWIyi == ZTuRidBSzCXKeUsflhnjidXKhlWIyi- 1 ) ZTuRidBSzCXKeUsflhnjidXKhlWIyi=211027350; else ZTuRidBSzCXKeUsflhnjidXKhlWIyi=168303332;if (ZTuRidBSzCXKeUsflhnjidXKhlWIyi == ZTuRidBSzCXKeUsflhnjidXKhlWIyi- 0 ) ZTuRidBSzCXKeUsflhnjidXKhlWIyi=1721150988; else ZTuRidBSzCXKeUsflhnjidXKhlWIyi=2042152596;if (ZTuRidBSzCXKeUsflhnjidXKhlWIyi == ZTuRidBSzCXKeUsflhnjidXKhlWIyi- 0 ) ZTuRidBSzCXKeUsflhnjidXKhlWIyi=831524884; else ZTuRidBSzCXKeUsflhnjidXKhlWIyi=1186921479;if (ZTuRidBSzCXKeUsflhnjidXKhlWIyi == ZTuRidBSzCXKeUsflhnjidXKhlWIyi- 1 ) ZTuRidBSzCXKeUsflhnjidXKhlWIyi=1987556288; else ZTuRidBSzCXKeUsflhnjidXKhlWIyi=1643051977;if (ZTuRidBSzCXKeUsflhnjidXKhlWIyi == ZTuRidBSzCXKeUsflhnjidXKhlWIyi- 1 ) ZTuRidBSzCXKeUsflhnjidXKhlWIyi=1839337333; else ZTuRidBSzCXKeUsflhnjidXKhlWIyi=882902447;double YwwBGCmiAgKSceCGDlVofpkbGTxxPd=766215294.446969487523150079766751325663;if (YwwBGCmiAgKSceCGDlVofpkbGTxxPd == YwwBGCmiAgKSceCGDlVofpkbGTxxPd ) YwwBGCmiAgKSceCGDlVofpkbGTxxPd=315888087.297841488770620670436484970575; else YwwBGCmiAgKSceCGDlVofpkbGTxxPd=1705321365.583544641979209724175633781408;if (YwwBGCmiAgKSceCGDlVofpkbGTxxPd == YwwBGCmiAgKSceCGDlVofpkbGTxxPd ) YwwBGCmiAgKSceCGDlVofpkbGTxxPd=1102045042.292330728704957838635640195086; else YwwBGCmiAgKSceCGDlVofpkbGTxxPd=1588227161.335488898901657179706387365234;if (YwwBGCmiAgKSceCGDlVofpkbGTxxPd == YwwBGCmiAgKSceCGDlVofpkbGTxxPd ) YwwBGCmiAgKSceCGDlVofpkbGTxxPd=1023687417.675347068904183510402738116812; else YwwBGCmiAgKSceCGDlVofpkbGTxxPd=752868748.367543641022290231496694204824;if (YwwBGCmiAgKSceCGDlVofpkbGTxxPd == YwwBGCmiAgKSceCGDlVofpkbGTxxPd ) YwwBGCmiAgKSceCGDlVofpkbGTxxPd=416241448.226664864198948155808542688334; else YwwBGCmiAgKSceCGDlVofpkbGTxxPd=1344799035.992781763647740975715003688409;if (YwwBGCmiAgKSceCGDlVofpkbGTxxPd == YwwBGCmiAgKSceCGDlVofpkbGTxxPd ) YwwBGCmiAgKSceCGDlVofpkbGTxxPd=1716819672.753120626750326281381372000021; else YwwBGCmiAgKSceCGDlVofpkbGTxxPd=1252122479.346509472145007589750823849113;if (YwwBGCmiAgKSceCGDlVofpkbGTxxPd == YwwBGCmiAgKSceCGDlVofpkbGTxxPd ) YwwBGCmiAgKSceCGDlVofpkbGTxxPd=683956025.161542482782935748653237262477; else YwwBGCmiAgKSceCGDlVofpkbGTxxPd=1529220695.137008456610708990921266003549;double EGchXbuNjLrHxTOYPxQjOZaeulCyzX=164861955.582344500316828120918415861546;if (EGchXbuNjLrHxTOYPxQjOZaeulCyzX == EGchXbuNjLrHxTOYPxQjOZaeulCyzX ) EGchXbuNjLrHxTOYPxQjOZaeulCyzX=766890674.924606854943742565680782427869; else EGchXbuNjLrHxTOYPxQjOZaeulCyzX=416505271.316559323348708678331797635339;if (EGchXbuNjLrHxTOYPxQjOZaeulCyzX == EGchXbuNjLrHxTOYPxQjOZaeulCyzX ) EGchXbuNjLrHxTOYPxQjOZaeulCyzX=1289323482.919673830434345318495064872092; else EGchXbuNjLrHxTOYPxQjOZaeulCyzX=1293003088.500443914585900746242705506690;if (EGchXbuNjLrHxTOYPxQjOZaeulCyzX == EGchXbuNjLrHxTOYPxQjOZaeulCyzX ) EGchXbuNjLrHxTOYPxQjOZaeulCyzX=1652820238.285893491251591026413786191604; else EGchXbuNjLrHxTOYPxQjOZaeulCyzX=996119370.833441111846609221558038861981;if (EGchXbuNjLrHxTOYPxQjOZaeulCyzX == EGchXbuNjLrHxTOYPxQjOZaeulCyzX ) EGchXbuNjLrHxTOYPxQjOZaeulCyzX=2029389812.390753816101668819797015780678; else EGchXbuNjLrHxTOYPxQjOZaeulCyzX=1607350901.024165605742656857005339438508;if (EGchXbuNjLrHxTOYPxQjOZaeulCyzX == EGchXbuNjLrHxTOYPxQjOZaeulCyzX ) EGchXbuNjLrHxTOYPxQjOZaeulCyzX=263668607.335818861422405657004367489673; else EGchXbuNjLrHxTOYPxQjOZaeulCyzX=485686685.720842794800669404498261417843;if (EGchXbuNjLrHxTOYPxQjOZaeulCyzX == EGchXbuNjLrHxTOYPxQjOZaeulCyzX ) EGchXbuNjLrHxTOYPxQjOZaeulCyzX=883219404.039981266496320570909629566954; else EGchXbuNjLrHxTOYPxQjOZaeulCyzX=943871023.595298189824910395204805541142;double RFKvzIILIVtEDrZkFsOjElbykxAkak=1143120342.482212763504008187736873345292;if (RFKvzIILIVtEDrZkFsOjElbykxAkak == RFKvzIILIVtEDrZkFsOjElbykxAkak ) RFKvzIILIVtEDrZkFsOjElbykxAkak=654835430.628533134580091948866128572098; else RFKvzIILIVtEDrZkFsOjElbykxAkak=1875242637.758199658616336798250853760016;if (RFKvzIILIVtEDrZkFsOjElbykxAkak == RFKvzIILIVtEDrZkFsOjElbykxAkak ) RFKvzIILIVtEDrZkFsOjElbykxAkak=559935543.124683291349136180389595801544; else RFKvzIILIVtEDrZkFsOjElbykxAkak=791840579.965250285360802283778173874089;if (RFKvzIILIVtEDrZkFsOjElbykxAkak == RFKvzIILIVtEDrZkFsOjElbykxAkak ) RFKvzIILIVtEDrZkFsOjElbykxAkak=231073138.386748598791513979372888799348; else RFKvzIILIVtEDrZkFsOjElbykxAkak=731639490.620709516579518766001581750816;if (RFKvzIILIVtEDrZkFsOjElbykxAkak == RFKvzIILIVtEDrZkFsOjElbykxAkak ) RFKvzIILIVtEDrZkFsOjElbykxAkak=1170165528.935018292546491448003335825905; else RFKvzIILIVtEDrZkFsOjElbykxAkak=133942587.971065450168556519142292460120;if (RFKvzIILIVtEDrZkFsOjElbykxAkak == RFKvzIILIVtEDrZkFsOjElbykxAkak ) RFKvzIILIVtEDrZkFsOjElbykxAkak=631223820.439877643609994599941687389971; else RFKvzIILIVtEDrZkFsOjElbykxAkak=1013711962.633183089007765441898239985259;if (RFKvzIILIVtEDrZkFsOjElbykxAkak == RFKvzIILIVtEDrZkFsOjElbykxAkak ) RFKvzIILIVtEDrZkFsOjElbykxAkak=598967307.978307682162094503394928351530; else RFKvzIILIVtEDrZkFsOjElbykxAkak=469028509.983013046896973724649306616006;double GcINsrxsFachYCowVYiLhvqFcnkYym=1116434135.572530647189961084448366896183;if (GcINsrxsFachYCowVYiLhvqFcnkYym == GcINsrxsFachYCowVYiLhvqFcnkYym ) GcINsrxsFachYCowVYiLhvqFcnkYym=1162324161.083177105856535754268574431684; else GcINsrxsFachYCowVYiLhvqFcnkYym=1674475378.556510699499069283215303652074;if (GcINsrxsFachYCowVYiLhvqFcnkYym == GcINsrxsFachYCowVYiLhvqFcnkYym ) GcINsrxsFachYCowVYiLhvqFcnkYym=116011773.663140113526588961535957026406; else GcINsrxsFachYCowVYiLhvqFcnkYym=1344447659.963594793918070982670164981622;if (GcINsrxsFachYCowVYiLhvqFcnkYym == GcINsrxsFachYCowVYiLhvqFcnkYym ) GcINsrxsFachYCowVYiLhvqFcnkYym=290599633.572777300537863898815493790072; else GcINsrxsFachYCowVYiLhvqFcnkYym=899909571.318254641379315330229491819919;if (GcINsrxsFachYCowVYiLhvqFcnkYym == GcINsrxsFachYCowVYiLhvqFcnkYym ) GcINsrxsFachYCowVYiLhvqFcnkYym=1260878703.744922072926855628946085303854; else GcINsrxsFachYCowVYiLhvqFcnkYym=1467252364.683363549347940364720107377963;if (GcINsrxsFachYCowVYiLhvqFcnkYym == GcINsrxsFachYCowVYiLhvqFcnkYym ) GcINsrxsFachYCowVYiLhvqFcnkYym=1823415200.022835369318971925614924558600; else GcINsrxsFachYCowVYiLhvqFcnkYym=1642177788.779742611600541234746246011949;if (GcINsrxsFachYCowVYiLhvqFcnkYym == GcINsrxsFachYCowVYiLhvqFcnkYym ) GcINsrxsFachYCowVYiLhvqFcnkYym=1324558554.786065034941411458333406453373; else GcINsrxsFachYCowVYiLhvqFcnkYym=538000732.288393459630191583667461639172;int heDjeQHLpAwvlyMSCZtiAmGiqLhibW=471895606;if (heDjeQHLpAwvlyMSCZtiAmGiqLhibW == heDjeQHLpAwvlyMSCZtiAmGiqLhibW- 0 ) heDjeQHLpAwvlyMSCZtiAmGiqLhibW=1367049771; else heDjeQHLpAwvlyMSCZtiAmGiqLhibW=1688319330;if (heDjeQHLpAwvlyMSCZtiAmGiqLhibW == heDjeQHLpAwvlyMSCZtiAmGiqLhibW- 0 ) heDjeQHLpAwvlyMSCZtiAmGiqLhibW=874385265; else heDjeQHLpAwvlyMSCZtiAmGiqLhibW=104638601;if (heDjeQHLpAwvlyMSCZtiAmGiqLhibW == heDjeQHLpAwvlyMSCZtiAmGiqLhibW- 1 ) heDjeQHLpAwvlyMSCZtiAmGiqLhibW=311186610; else heDjeQHLpAwvlyMSCZtiAmGiqLhibW=1836370884;if (heDjeQHLpAwvlyMSCZtiAmGiqLhibW == heDjeQHLpAwvlyMSCZtiAmGiqLhibW- 0 ) heDjeQHLpAwvlyMSCZtiAmGiqLhibW=2081272464; else heDjeQHLpAwvlyMSCZtiAmGiqLhibW=35251184;if (heDjeQHLpAwvlyMSCZtiAmGiqLhibW == heDjeQHLpAwvlyMSCZtiAmGiqLhibW- 0 ) heDjeQHLpAwvlyMSCZtiAmGiqLhibW=1393823390; else heDjeQHLpAwvlyMSCZtiAmGiqLhibW=1155380895;if (heDjeQHLpAwvlyMSCZtiAmGiqLhibW == heDjeQHLpAwvlyMSCZtiAmGiqLhibW- 0 ) heDjeQHLpAwvlyMSCZtiAmGiqLhibW=661779268; else heDjeQHLpAwvlyMSCZtiAmGiqLhibW=639108276;int vEdKsjMpTvwyUpBgtrrXOYWNJvGkZB=961364418;if (vEdKsjMpTvwyUpBgtrrXOYWNJvGkZB == vEdKsjMpTvwyUpBgtrrXOYWNJvGkZB- 1 ) vEdKsjMpTvwyUpBgtrrXOYWNJvGkZB=1278646444; else vEdKsjMpTvwyUpBgtrrXOYWNJvGkZB=1914268916;if (vEdKsjMpTvwyUpBgtrrXOYWNJvGkZB == vEdKsjMpTvwyUpBgtrrXOYWNJvGkZB- 0 ) vEdKsjMpTvwyUpBgtrrXOYWNJvGkZB=1929030509; else vEdKsjMpTvwyUpBgtrrXOYWNJvGkZB=880429947;if (vEdKsjMpTvwyUpBgtrrXOYWNJvGkZB == vEdKsjMpTvwyUpBgtrrXOYWNJvGkZB- 1 ) vEdKsjMpTvwyUpBgtrrXOYWNJvGkZB=1128912293; else vEdKsjMpTvwyUpBgtrrXOYWNJvGkZB=688108696;if (vEdKsjMpTvwyUpBgtrrXOYWNJvGkZB == vEdKsjMpTvwyUpBgtrrXOYWNJvGkZB- 1 ) vEdKsjMpTvwyUpBgtrrXOYWNJvGkZB=852520709; else vEdKsjMpTvwyUpBgtrrXOYWNJvGkZB=1902409506;if (vEdKsjMpTvwyUpBgtrrXOYWNJvGkZB == vEdKsjMpTvwyUpBgtrrXOYWNJvGkZB- 1 ) vEdKsjMpTvwyUpBgtrrXOYWNJvGkZB=614363934; else vEdKsjMpTvwyUpBgtrrXOYWNJvGkZB=1240166826;if (vEdKsjMpTvwyUpBgtrrXOYWNJvGkZB == vEdKsjMpTvwyUpBgtrrXOYWNJvGkZB- 1 ) vEdKsjMpTvwyUpBgtrrXOYWNJvGkZB=1115909026; else vEdKsjMpTvwyUpBgtrrXOYWNJvGkZB=736378997;double eVphyiDsRptmnyrofApAyjtABhgcoV=1158853395.208130508565212031913126581962;if (eVphyiDsRptmnyrofApAyjtABhgcoV == eVphyiDsRptmnyrofApAyjtABhgcoV ) eVphyiDsRptmnyrofApAyjtABhgcoV=76334605.414446808149588200021486018365; else eVphyiDsRptmnyrofApAyjtABhgcoV=127071701.065625287634487465849104204843;if (eVphyiDsRptmnyrofApAyjtABhgcoV == eVphyiDsRptmnyrofApAyjtABhgcoV ) eVphyiDsRptmnyrofApAyjtABhgcoV=1249462385.864319507402349700975453697142; else eVphyiDsRptmnyrofApAyjtABhgcoV=1817360888.245588017907854796561498951283;if (eVphyiDsRptmnyrofApAyjtABhgcoV == eVphyiDsRptmnyrofApAyjtABhgcoV ) eVphyiDsRptmnyrofApAyjtABhgcoV=575402571.346526896279318861901179286864; else eVphyiDsRptmnyrofApAyjtABhgcoV=1249885287.894576143800049218110138228844;if (eVphyiDsRptmnyrofApAyjtABhgcoV == eVphyiDsRptmnyrofApAyjtABhgcoV ) eVphyiDsRptmnyrofApAyjtABhgcoV=805247525.103359685273820114017885476645; else eVphyiDsRptmnyrofApAyjtABhgcoV=1473436924.940980789073494832912484733908;if (eVphyiDsRptmnyrofApAyjtABhgcoV == eVphyiDsRptmnyrofApAyjtABhgcoV ) eVphyiDsRptmnyrofApAyjtABhgcoV=1740686561.405134395144505459174583357735; else eVphyiDsRptmnyrofApAyjtABhgcoV=80760589.799277936155898908938483402389;if (eVphyiDsRptmnyrofApAyjtABhgcoV == eVphyiDsRptmnyrofApAyjtABhgcoV ) eVphyiDsRptmnyrofApAyjtABhgcoV=2098107916.877046544862412080656980732688; else eVphyiDsRptmnyrofApAyjtABhgcoV=1282702385.666849886247528617152383399841;double wedMBjemQLzWQHwWfKDLXYQdOoLYWz=263053897.145609774714134881008664960203;if (wedMBjemQLzWQHwWfKDLXYQdOoLYWz == wedMBjemQLzWQHwWfKDLXYQdOoLYWz ) wedMBjemQLzWQHwWfKDLXYQdOoLYWz=1393023458.947912097410375304039121868285; else wedMBjemQLzWQHwWfKDLXYQdOoLYWz=363194136.957188722399570849966727592626;if (wedMBjemQLzWQHwWfKDLXYQdOoLYWz == wedMBjemQLzWQHwWfKDLXYQdOoLYWz ) wedMBjemQLzWQHwWfKDLXYQdOoLYWz=1406837935.581356339879285066202933933160; else wedMBjemQLzWQHwWfKDLXYQdOoLYWz=102864075.721691922461120854353653466943;if (wedMBjemQLzWQHwWfKDLXYQdOoLYWz == wedMBjemQLzWQHwWfKDLXYQdOoLYWz ) wedMBjemQLzWQHwWfKDLXYQdOoLYWz=1838307210.337016870425585192418992727923; else wedMBjemQLzWQHwWfKDLXYQdOoLYWz=56947410.618913511519593917837554555530;if (wedMBjemQLzWQHwWfKDLXYQdOoLYWz == wedMBjemQLzWQHwWfKDLXYQdOoLYWz ) wedMBjemQLzWQHwWfKDLXYQdOoLYWz=1490466960.905892777278101050092010626557; else wedMBjemQLzWQHwWfKDLXYQdOoLYWz=678941075.909261580135177046450564990628;if (wedMBjemQLzWQHwWfKDLXYQdOoLYWz == wedMBjemQLzWQHwWfKDLXYQdOoLYWz ) wedMBjemQLzWQHwWfKDLXYQdOoLYWz=835798621.722037004068097804379110845538; else wedMBjemQLzWQHwWfKDLXYQdOoLYWz=1119459406.296351314104240257829635062330;if (wedMBjemQLzWQHwWfKDLXYQdOoLYWz == wedMBjemQLzWQHwWfKDLXYQdOoLYWz ) wedMBjemQLzWQHwWfKDLXYQdOoLYWz=1637758314.257797284659137597160882977249; else wedMBjemQLzWQHwWfKDLXYQdOoLYWz=1218503431.009596766318327051857538048583;float BUajaDeRIpSEnbKZddwGbUuKXkFYZp=761658594.545711267552883012875750536357f;if (BUajaDeRIpSEnbKZddwGbUuKXkFYZp - BUajaDeRIpSEnbKZddwGbUuKXkFYZp> 0.00000001 ) BUajaDeRIpSEnbKZddwGbUuKXkFYZp=891087652.509759360269741146977163689628f; else BUajaDeRIpSEnbKZddwGbUuKXkFYZp=1650409520.502015424585271612040058144727f;if (BUajaDeRIpSEnbKZddwGbUuKXkFYZp - BUajaDeRIpSEnbKZddwGbUuKXkFYZp> 0.00000001 ) BUajaDeRIpSEnbKZddwGbUuKXkFYZp=562773671.185377774682349200207779866507f; else BUajaDeRIpSEnbKZddwGbUuKXkFYZp=270080362.280751101595947871995553042651f;if (BUajaDeRIpSEnbKZddwGbUuKXkFYZp - BUajaDeRIpSEnbKZddwGbUuKXkFYZp> 0.00000001 ) BUajaDeRIpSEnbKZddwGbUuKXkFYZp=310898918.137751360910981293319662373881f; else BUajaDeRIpSEnbKZddwGbUuKXkFYZp=579277089.056046930662118567286945358548f;if (BUajaDeRIpSEnbKZddwGbUuKXkFYZp - BUajaDeRIpSEnbKZddwGbUuKXkFYZp> 0.00000001 ) BUajaDeRIpSEnbKZddwGbUuKXkFYZp=385961847.859006504546375866481760612170f; else BUajaDeRIpSEnbKZddwGbUuKXkFYZp=28761157.949291422317485068060327720959f;if (BUajaDeRIpSEnbKZddwGbUuKXkFYZp - BUajaDeRIpSEnbKZddwGbUuKXkFYZp> 0.00000001 ) BUajaDeRIpSEnbKZddwGbUuKXkFYZp=1109021391.543570819444528439665412063367f; else BUajaDeRIpSEnbKZddwGbUuKXkFYZp=455593585.267387592525072691949139413161f;if (BUajaDeRIpSEnbKZddwGbUuKXkFYZp - BUajaDeRIpSEnbKZddwGbUuKXkFYZp> 0.00000001 ) BUajaDeRIpSEnbKZddwGbUuKXkFYZp=1287227033.866229724104724155048242813997f; else BUajaDeRIpSEnbKZddwGbUuKXkFYZp=1048151704.544177085497895541547913473441f;long sNghaAgblFhAiVDXZoPkMScCdPImBW=2140077154;if (sNghaAgblFhAiVDXZoPkMScCdPImBW == sNghaAgblFhAiVDXZoPkMScCdPImBW- 0 ) sNghaAgblFhAiVDXZoPkMScCdPImBW=170066525; else sNghaAgblFhAiVDXZoPkMScCdPImBW=852855129;if (sNghaAgblFhAiVDXZoPkMScCdPImBW == sNghaAgblFhAiVDXZoPkMScCdPImBW- 1 ) sNghaAgblFhAiVDXZoPkMScCdPImBW=1490839186; else sNghaAgblFhAiVDXZoPkMScCdPImBW=1191370447;if (sNghaAgblFhAiVDXZoPkMScCdPImBW == sNghaAgblFhAiVDXZoPkMScCdPImBW- 1 ) sNghaAgblFhAiVDXZoPkMScCdPImBW=1372810508; else sNghaAgblFhAiVDXZoPkMScCdPImBW=161429168;if (sNghaAgblFhAiVDXZoPkMScCdPImBW == sNghaAgblFhAiVDXZoPkMScCdPImBW- 1 ) sNghaAgblFhAiVDXZoPkMScCdPImBW=1468029832; else sNghaAgblFhAiVDXZoPkMScCdPImBW=19387195;if (sNghaAgblFhAiVDXZoPkMScCdPImBW == sNghaAgblFhAiVDXZoPkMScCdPImBW- 0 ) sNghaAgblFhAiVDXZoPkMScCdPImBW=784088388; else sNghaAgblFhAiVDXZoPkMScCdPImBW=570521310;if (sNghaAgblFhAiVDXZoPkMScCdPImBW == sNghaAgblFhAiVDXZoPkMScCdPImBW- 1 ) sNghaAgblFhAiVDXZoPkMScCdPImBW=981562267; else sNghaAgblFhAiVDXZoPkMScCdPImBW=1603426812;double WlGRlApgYFTLRtBydJrejHsfeDKWSo=395793094.148114949818278458643159876972;if (WlGRlApgYFTLRtBydJrejHsfeDKWSo == WlGRlApgYFTLRtBydJrejHsfeDKWSo ) WlGRlApgYFTLRtBydJrejHsfeDKWSo=1600073988.383783514811396160376864110944; else WlGRlApgYFTLRtBydJrejHsfeDKWSo=1336449322.918853559604594286471660663077;if (WlGRlApgYFTLRtBydJrejHsfeDKWSo == WlGRlApgYFTLRtBydJrejHsfeDKWSo ) WlGRlApgYFTLRtBydJrejHsfeDKWSo=391614733.368001354372188347193467435929; else WlGRlApgYFTLRtBydJrejHsfeDKWSo=735204430.159334750303311068343608591177;if (WlGRlApgYFTLRtBydJrejHsfeDKWSo == WlGRlApgYFTLRtBydJrejHsfeDKWSo ) WlGRlApgYFTLRtBydJrejHsfeDKWSo=58996756.654632396479880026657869656194; else WlGRlApgYFTLRtBydJrejHsfeDKWSo=800019939.369385078078398593958652090609;if (WlGRlApgYFTLRtBydJrejHsfeDKWSo == WlGRlApgYFTLRtBydJrejHsfeDKWSo ) WlGRlApgYFTLRtBydJrejHsfeDKWSo=2099638626.621221151615878057127434781730; else WlGRlApgYFTLRtBydJrejHsfeDKWSo=813746369.548945828744334389514358330754;if (WlGRlApgYFTLRtBydJrejHsfeDKWSo == WlGRlApgYFTLRtBydJrejHsfeDKWSo ) WlGRlApgYFTLRtBydJrejHsfeDKWSo=2124715936.042117893334058320051333730072; else WlGRlApgYFTLRtBydJrejHsfeDKWSo=1378076778.617571223859629040102070382059;if (WlGRlApgYFTLRtBydJrejHsfeDKWSo == WlGRlApgYFTLRtBydJrejHsfeDKWSo ) WlGRlApgYFTLRtBydJrejHsfeDKWSo=1141326331.883282859820830282479008165720; else WlGRlApgYFTLRtBydJrejHsfeDKWSo=1382713625.027418754839239234110068088522;float BRtrvQRLLcPngVCpkuvOChDxvfuVNz=1265980297.617758027159381409263969160333f;if (BRtrvQRLLcPngVCpkuvOChDxvfuVNz - BRtrvQRLLcPngVCpkuvOChDxvfuVNz> 0.00000001 ) BRtrvQRLLcPngVCpkuvOChDxvfuVNz=1602663850.770431432637850564099881976869f; else BRtrvQRLLcPngVCpkuvOChDxvfuVNz=1645164807.870459005007864612071744590957f;if (BRtrvQRLLcPngVCpkuvOChDxvfuVNz - BRtrvQRLLcPngVCpkuvOChDxvfuVNz> 0.00000001 ) BRtrvQRLLcPngVCpkuvOChDxvfuVNz=1048436906.677396850156009212736580924606f; else BRtrvQRLLcPngVCpkuvOChDxvfuVNz=2015090376.015854168887292584539823129747f;if (BRtrvQRLLcPngVCpkuvOChDxvfuVNz - BRtrvQRLLcPngVCpkuvOChDxvfuVNz> 0.00000001 ) BRtrvQRLLcPngVCpkuvOChDxvfuVNz=2001537928.335424226625506939855017829283f; else BRtrvQRLLcPngVCpkuvOChDxvfuVNz=1584283448.542166387836902443820109342871f;if (BRtrvQRLLcPngVCpkuvOChDxvfuVNz - BRtrvQRLLcPngVCpkuvOChDxvfuVNz> 0.00000001 ) BRtrvQRLLcPngVCpkuvOChDxvfuVNz=1536331733.604328454075202922649656455383f; else BRtrvQRLLcPngVCpkuvOChDxvfuVNz=729658484.552733368519085590852033206908f;if (BRtrvQRLLcPngVCpkuvOChDxvfuVNz - BRtrvQRLLcPngVCpkuvOChDxvfuVNz> 0.00000001 ) BRtrvQRLLcPngVCpkuvOChDxvfuVNz=182379688.317263467077414078106664153954f; else BRtrvQRLLcPngVCpkuvOChDxvfuVNz=543386476.847131383314389672033880275411f;if (BRtrvQRLLcPngVCpkuvOChDxvfuVNz - BRtrvQRLLcPngVCpkuvOChDxvfuVNz> 0.00000001 ) BRtrvQRLLcPngVCpkuvOChDxvfuVNz=1633475621.161423039380589349558193206706f; else BRtrvQRLLcPngVCpkuvOChDxvfuVNz=1887861255.697336306664376341294796307923f;int SQSlrOMaPATLxvUrIitfcbEHvzDvSu=1522277297;if (SQSlrOMaPATLxvUrIitfcbEHvzDvSu == SQSlrOMaPATLxvUrIitfcbEHvzDvSu- 0 ) SQSlrOMaPATLxvUrIitfcbEHvzDvSu=1850102166; else SQSlrOMaPATLxvUrIitfcbEHvzDvSu=891433939;if (SQSlrOMaPATLxvUrIitfcbEHvzDvSu == SQSlrOMaPATLxvUrIitfcbEHvzDvSu- 1 ) SQSlrOMaPATLxvUrIitfcbEHvzDvSu=1595561363; else SQSlrOMaPATLxvUrIitfcbEHvzDvSu=1548518752;if (SQSlrOMaPATLxvUrIitfcbEHvzDvSu == SQSlrOMaPATLxvUrIitfcbEHvzDvSu- 1 ) SQSlrOMaPATLxvUrIitfcbEHvzDvSu=1325687238; else SQSlrOMaPATLxvUrIitfcbEHvzDvSu=929074773;if (SQSlrOMaPATLxvUrIitfcbEHvzDvSu == SQSlrOMaPATLxvUrIitfcbEHvzDvSu- 1 ) SQSlrOMaPATLxvUrIitfcbEHvzDvSu=2081080398; else SQSlrOMaPATLxvUrIitfcbEHvzDvSu=1517530233;if (SQSlrOMaPATLxvUrIitfcbEHvzDvSu == SQSlrOMaPATLxvUrIitfcbEHvzDvSu- 0 ) SQSlrOMaPATLxvUrIitfcbEHvzDvSu=900700150; else SQSlrOMaPATLxvUrIitfcbEHvzDvSu=677092817;if (SQSlrOMaPATLxvUrIitfcbEHvzDvSu == SQSlrOMaPATLxvUrIitfcbEHvzDvSu- 0 ) SQSlrOMaPATLxvUrIitfcbEHvzDvSu=1214138158; else SQSlrOMaPATLxvUrIitfcbEHvzDvSu=2142217377;float nMUhahkbbQCRnUHAMFMHuywBanZUzd=341773132.465253322442941897461102512307f;if (nMUhahkbbQCRnUHAMFMHuywBanZUzd - nMUhahkbbQCRnUHAMFMHuywBanZUzd> 0.00000001 ) nMUhahkbbQCRnUHAMFMHuywBanZUzd=629064662.668671126648863330301484946374f; else nMUhahkbbQCRnUHAMFMHuywBanZUzd=1863065617.203698547077292345029210728474f;if (nMUhahkbbQCRnUHAMFMHuywBanZUzd - nMUhahkbbQCRnUHAMFMHuywBanZUzd> 0.00000001 ) nMUhahkbbQCRnUHAMFMHuywBanZUzd=762962.686244637599333420261235600397f; else nMUhahkbbQCRnUHAMFMHuywBanZUzd=114445813.027450215871864098830922445165f;if (nMUhahkbbQCRnUHAMFMHuywBanZUzd - nMUhahkbbQCRnUHAMFMHuywBanZUzd> 0.00000001 ) nMUhahkbbQCRnUHAMFMHuywBanZUzd=685790835.544157309422540048171257720601f; else nMUhahkbbQCRnUHAMFMHuywBanZUzd=885368337.525144781870951849341800540844f;if (nMUhahkbbQCRnUHAMFMHuywBanZUzd - nMUhahkbbQCRnUHAMFMHuywBanZUzd> 0.00000001 ) nMUhahkbbQCRnUHAMFMHuywBanZUzd=541738813.880901851637740230608247172660f; else nMUhahkbbQCRnUHAMFMHuywBanZUzd=15790540.504810016019872417041025764483f;if (nMUhahkbbQCRnUHAMFMHuywBanZUzd - nMUhahkbbQCRnUHAMFMHuywBanZUzd> 0.00000001 ) nMUhahkbbQCRnUHAMFMHuywBanZUzd=1947601236.039124304023569958587663058015f; else nMUhahkbbQCRnUHAMFMHuywBanZUzd=1809439896.764419075911312866682791747728f;if (nMUhahkbbQCRnUHAMFMHuywBanZUzd - nMUhahkbbQCRnUHAMFMHuywBanZUzd> 0.00000001 ) nMUhahkbbQCRnUHAMFMHuywBanZUzd=552824742.649046016098377636220244308387f; else nMUhahkbbQCRnUHAMFMHuywBanZUzd=1663093927.346838248795179443411250840674f;long iTsoZMdSvTxqbPdGnXtgYtifDudAwn=1194697254;if (iTsoZMdSvTxqbPdGnXtgYtifDudAwn == iTsoZMdSvTxqbPdGnXtgYtifDudAwn- 0 ) iTsoZMdSvTxqbPdGnXtgYtifDudAwn=961442594; else iTsoZMdSvTxqbPdGnXtgYtifDudAwn=1766532681;if (iTsoZMdSvTxqbPdGnXtgYtifDudAwn == iTsoZMdSvTxqbPdGnXtgYtifDudAwn- 1 ) iTsoZMdSvTxqbPdGnXtgYtifDudAwn=1680095106; else iTsoZMdSvTxqbPdGnXtgYtifDudAwn=25770085;if (iTsoZMdSvTxqbPdGnXtgYtifDudAwn == iTsoZMdSvTxqbPdGnXtgYtifDudAwn- 1 ) iTsoZMdSvTxqbPdGnXtgYtifDudAwn=994237540; else iTsoZMdSvTxqbPdGnXtgYtifDudAwn=540421543;if (iTsoZMdSvTxqbPdGnXtgYtifDudAwn == iTsoZMdSvTxqbPdGnXtgYtifDudAwn- 0 ) iTsoZMdSvTxqbPdGnXtgYtifDudAwn=2087776706; else iTsoZMdSvTxqbPdGnXtgYtifDudAwn=673589077;if (iTsoZMdSvTxqbPdGnXtgYtifDudAwn == iTsoZMdSvTxqbPdGnXtgYtifDudAwn- 1 ) iTsoZMdSvTxqbPdGnXtgYtifDudAwn=1711568056; else iTsoZMdSvTxqbPdGnXtgYtifDudAwn=1678390320;if (iTsoZMdSvTxqbPdGnXtgYtifDudAwn == iTsoZMdSvTxqbPdGnXtgYtifDudAwn- 1 ) iTsoZMdSvTxqbPdGnXtgYtifDudAwn=1114390226; else iTsoZMdSvTxqbPdGnXtgYtifDudAwn=1324445111;int qAHLOBXTzEDHQkdxhLDRyXDMBBfQgA=97063481;if (qAHLOBXTzEDHQkdxhLDRyXDMBBfQgA == qAHLOBXTzEDHQkdxhLDRyXDMBBfQgA- 0 ) qAHLOBXTzEDHQkdxhLDRyXDMBBfQgA=1441203305; else qAHLOBXTzEDHQkdxhLDRyXDMBBfQgA=1291702489;if (qAHLOBXTzEDHQkdxhLDRyXDMBBfQgA == qAHLOBXTzEDHQkdxhLDRyXDMBBfQgA- 0 ) qAHLOBXTzEDHQkdxhLDRyXDMBBfQgA=1258796593; else qAHLOBXTzEDHQkdxhLDRyXDMBBfQgA=2029616499;if (qAHLOBXTzEDHQkdxhLDRyXDMBBfQgA == qAHLOBXTzEDHQkdxhLDRyXDMBBfQgA- 0 ) qAHLOBXTzEDHQkdxhLDRyXDMBBfQgA=929255720; else qAHLOBXTzEDHQkdxhLDRyXDMBBfQgA=302188371;if (qAHLOBXTzEDHQkdxhLDRyXDMBBfQgA == qAHLOBXTzEDHQkdxhLDRyXDMBBfQgA- 0 ) qAHLOBXTzEDHQkdxhLDRyXDMBBfQgA=836486462; else qAHLOBXTzEDHQkdxhLDRyXDMBBfQgA=964733829;if (qAHLOBXTzEDHQkdxhLDRyXDMBBfQgA == qAHLOBXTzEDHQkdxhLDRyXDMBBfQgA- 1 ) qAHLOBXTzEDHQkdxhLDRyXDMBBfQgA=349695126; else qAHLOBXTzEDHQkdxhLDRyXDMBBfQgA=1579770823;if (qAHLOBXTzEDHQkdxhLDRyXDMBBfQgA == qAHLOBXTzEDHQkdxhLDRyXDMBBfQgA- 1 ) qAHLOBXTzEDHQkdxhLDRyXDMBBfQgA=1748401163; else qAHLOBXTzEDHQkdxhLDRyXDMBBfQgA=1663730989;int yKqklUunzrvezpcAwjMToCrKJuiHHb=688922779;if (yKqklUunzrvezpcAwjMToCrKJuiHHb == yKqklUunzrvezpcAwjMToCrKJuiHHb- 0 ) yKqklUunzrvezpcAwjMToCrKJuiHHb=1608544917; else yKqklUunzrvezpcAwjMToCrKJuiHHb=19440904;if (yKqklUunzrvezpcAwjMToCrKJuiHHb == yKqklUunzrvezpcAwjMToCrKJuiHHb- 1 ) yKqklUunzrvezpcAwjMToCrKJuiHHb=1180774783; else yKqklUunzrvezpcAwjMToCrKJuiHHb=1548537450;if (yKqklUunzrvezpcAwjMToCrKJuiHHb == yKqklUunzrvezpcAwjMToCrKJuiHHb- 1 ) yKqklUunzrvezpcAwjMToCrKJuiHHb=552704748; else yKqklUunzrvezpcAwjMToCrKJuiHHb=1223896733;if (yKqklUunzrvezpcAwjMToCrKJuiHHb == yKqklUunzrvezpcAwjMToCrKJuiHHb- 0 ) yKqklUunzrvezpcAwjMToCrKJuiHHb=1495019239; else yKqklUunzrvezpcAwjMToCrKJuiHHb=88643119;if (yKqklUunzrvezpcAwjMToCrKJuiHHb == yKqklUunzrvezpcAwjMToCrKJuiHHb- 0 ) yKqklUunzrvezpcAwjMToCrKJuiHHb=1922772558; else yKqklUunzrvezpcAwjMToCrKJuiHHb=1474657532;if (yKqklUunzrvezpcAwjMToCrKJuiHHb == yKqklUunzrvezpcAwjMToCrKJuiHHb- 0 ) yKqklUunzrvezpcAwjMToCrKJuiHHb=550361404; else yKqklUunzrvezpcAwjMToCrKJuiHHb=1154884162;float IweqvKlzzgiFZXrMKbXMAKqpsOchau=406003233.366263121409046952826855471473f;if (IweqvKlzzgiFZXrMKbXMAKqpsOchau - IweqvKlzzgiFZXrMKbXMAKqpsOchau> 0.00000001 ) IweqvKlzzgiFZXrMKbXMAKqpsOchau=99884249.685491792027387760719402061190f; else IweqvKlzzgiFZXrMKbXMAKqpsOchau=455761261.285893565536181057971287529454f;if (IweqvKlzzgiFZXrMKbXMAKqpsOchau - IweqvKlzzgiFZXrMKbXMAKqpsOchau> 0.00000001 ) IweqvKlzzgiFZXrMKbXMAKqpsOchau=221890622.161285260430629342212432911947f; else IweqvKlzzgiFZXrMKbXMAKqpsOchau=22325476.512257690689855266929734374047f;if (IweqvKlzzgiFZXrMKbXMAKqpsOchau - IweqvKlzzgiFZXrMKbXMAKqpsOchau> 0.00000001 ) IweqvKlzzgiFZXrMKbXMAKqpsOchau=1100464425.542303958275363379886502989808f; else IweqvKlzzgiFZXrMKbXMAKqpsOchau=1333898695.476325855232876557200626204142f;if (IweqvKlzzgiFZXrMKbXMAKqpsOchau - IweqvKlzzgiFZXrMKbXMAKqpsOchau> 0.00000001 ) IweqvKlzzgiFZXrMKbXMAKqpsOchau=265441534.425091905962465474754370895022f; else IweqvKlzzgiFZXrMKbXMAKqpsOchau=659254223.171392789565335827284150825631f;if (IweqvKlzzgiFZXrMKbXMAKqpsOchau - IweqvKlzzgiFZXrMKbXMAKqpsOchau> 0.00000001 ) IweqvKlzzgiFZXrMKbXMAKqpsOchau=930046219.799389217489629014607894912257f; else IweqvKlzzgiFZXrMKbXMAKqpsOchau=715361277.726897705671081553192213491658f;if (IweqvKlzzgiFZXrMKbXMAKqpsOchau - IweqvKlzzgiFZXrMKbXMAKqpsOchau> 0.00000001 ) IweqvKlzzgiFZXrMKbXMAKqpsOchau=1823558606.707076384241904691346017661440f; else IweqvKlzzgiFZXrMKbXMAKqpsOchau=1561950364.725897502628096131991031280152f;int tTUbKgIEgEsxlJsBHCdQzZdxlqUbFl=1293581585;if (tTUbKgIEgEsxlJsBHCdQzZdxlqUbFl == tTUbKgIEgEsxlJsBHCdQzZdxlqUbFl- 0 ) tTUbKgIEgEsxlJsBHCdQzZdxlqUbFl=1474888052; else tTUbKgIEgEsxlJsBHCdQzZdxlqUbFl=425467447;if (tTUbKgIEgEsxlJsBHCdQzZdxlqUbFl == tTUbKgIEgEsxlJsBHCdQzZdxlqUbFl- 0 ) tTUbKgIEgEsxlJsBHCdQzZdxlqUbFl=1863383841; else tTUbKgIEgEsxlJsBHCdQzZdxlqUbFl=350704386;if (tTUbKgIEgEsxlJsBHCdQzZdxlqUbFl == tTUbKgIEgEsxlJsBHCdQzZdxlqUbFl- 1 ) tTUbKgIEgEsxlJsBHCdQzZdxlqUbFl=360823220; else tTUbKgIEgEsxlJsBHCdQzZdxlqUbFl=161018197;if (tTUbKgIEgEsxlJsBHCdQzZdxlqUbFl == tTUbKgIEgEsxlJsBHCdQzZdxlqUbFl- 0 ) tTUbKgIEgEsxlJsBHCdQzZdxlqUbFl=1691842380; else tTUbKgIEgEsxlJsBHCdQzZdxlqUbFl=217306572;if (tTUbKgIEgEsxlJsBHCdQzZdxlqUbFl == tTUbKgIEgEsxlJsBHCdQzZdxlqUbFl- 0 ) tTUbKgIEgEsxlJsBHCdQzZdxlqUbFl=1944166897; else tTUbKgIEgEsxlJsBHCdQzZdxlqUbFl=1170153474;if (tTUbKgIEgEsxlJsBHCdQzZdxlqUbFl == tTUbKgIEgEsxlJsBHCdQzZdxlqUbFl- 0 ) tTUbKgIEgEsxlJsBHCdQzZdxlqUbFl=106843941; else tTUbKgIEgEsxlJsBHCdQzZdxlqUbFl=1602081835;float NjJGKsuMEiOWuOpGmHoutaEvyuaIGM=1132401424.101851732734443522188420191372f;if (NjJGKsuMEiOWuOpGmHoutaEvyuaIGM - NjJGKsuMEiOWuOpGmHoutaEvyuaIGM> 0.00000001 ) NjJGKsuMEiOWuOpGmHoutaEvyuaIGM=1934125573.531301765544584111594064779177f; else NjJGKsuMEiOWuOpGmHoutaEvyuaIGM=1342397930.057031324386865405709188968763f;if (NjJGKsuMEiOWuOpGmHoutaEvyuaIGM - NjJGKsuMEiOWuOpGmHoutaEvyuaIGM> 0.00000001 ) NjJGKsuMEiOWuOpGmHoutaEvyuaIGM=1563846740.270859855499731831038547807928f; else NjJGKsuMEiOWuOpGmHoutaEvyuaIGM=2067972417.366212701675042046813884683316f;if (NjJGKsuMEiOWuOpGmHoutaEvyuaIGM - NjJGKsuMEiOWuOpGmHoutaEvyuaIGM> 0.00000001 ) NjJGKsuMEiOWuOpGmHoutaEvyuaIGM=196095849.294813462013046361596221559263f; else NjJGKsuMEiOWuOpGmHoutaEvyuaIGM=181176792.950659123079481184833382018447f;if (NjJGKsuMEiOWuOpGmHoutaEvyuaIGM - NjJGKsuMEiOWuOpGmHoutaEvyuaIGM> 0.00000001 ) NjJGKsuMEiOWuOpGmHoutaEvyuaIGM=1735942001.420017450834962784157543177864f; else NjJGKsuMEiOWuOpGmHoutaEvyuaIGM=274646222.189766058474411053134540008805f;if (NjJGKsuMEiOWuOpGmHoutaEvyuaIGM - NjJGKsuMEiOWuOpGmHoutaEvyuaIGM> 0.00000001 ) NjJGKsuMEiOWuOpGmHoutaEvyuaIGM=1901483017.179775631783778414222573075977f; else NjJGKsuMEiOWuOpGmHoutaEvyuaIGM=521520560.836341584653246287269469370771f;if (NjJGKsuMEiOWuOpGmHoutaEvyuaIGM - NjJGKsuMEiOWuOpGmHoutaEvyuaIGM> 0.00000001 ) NjJGKsuMEiOWuOpGmHoutaEvyuaIGM=618491231.176596753804077033899882948808f; else NjJGKsuMEiOWuOpGmHoutaEvyuaIGM=1984827118.191468786154547550244799239475f;int vYZkGPiCuObnmTZPScSzqifTVyKGNw=1498726841;if (vYZkGPiCuObnmTZPScSzqifTVyKGNw == vYZkGPiCuObnmTZPScSzqifTVyKGNw- 0 ) vYZkGPiCuObnmTZPScSzqifTVyKGNw=1177318486; else vYZkGPiCuObnmTZPScSzqifTVyKGNw=1404263466;if (vYZkGPiCuObnmTZPScSzqifTVyKGNw == vYZkGPiCuObnmTZPScSzqifTVyKGNw- 0 ) vYZkGPiCuObnmTZPScSzqifTVyKGNw=1827479757; else vYZkGPiCuObnmTZPScSzqifTVyKGNw=1492994311;if (vYZkGPiCuObnmTZPScSzqifTVyKGNw == vYZkGPiCuObnmTZPScSzqifTVyKGNw- 1 ) vYZkGPiCuObnmTZPScSzqifTVyKGNw=1648877155; else vYZkGPiCuObnmTZPScSzqifTVyKGNw=1442424511;if (vYZkGPiCuObnmTZPScSzqifTVyKGNw == vYZkGPiCuObnmTZPScSzqifTVyKGNw- 1 ) vYZkGPiCuObnmTZPScSzqifTVyKGNw=45180013; else vYZkGPiCuObnmTZPScSzqifTVyKGNw=2086349699;if (vYZkGPiCuObnmTZPScSzqifTVyKGNw == vYZkGPiCuObnmTZPScSzqifTVyKGNw- 0 ) vYZkGPiCuObnmTZPScSzqifTVyKGNw=911402528; else vYZkGPiCuObnmTZPScSzqifTVyKGNw=4584050;if (vYZkGPiCuObnmTZPScSzqifTVyKGNw == vYZkGPiCuObnmTZPScSzqifTVyKGNw- 0 ) vYZkGPiCuObnmTZPScSzqifTVyKGNw=225642226; else vYZkGPiCuObnmTZPScSzqifTVyKGNw=2077794598;int ywLxULblsaWXGpExRgEiCtqKNUGhyC=1248443919;if (ywLxULblsaWXGpExRgEiCtqKNUGhyC == ywLxULblsaWXGpExRgEiCtqKNUGhyC- 0 ) ywLxULblsaWXGpExRgEiCtqKNUGhyC=1669089206; else ywLxULblsaWXGpExRgEiCtqKNUGhyC=691157182;if (ywLxULblsaWXGpExRgEiCtqKNUGhyC == ywLxULblsaWXGpExRgEiCtqKNUGhyC- 1 ) ywLxULblsaWXGpExRgEiCtqKNUGhyC=1775308438; else ywLxULblsaWXGpExRgEiCtqKNUGhyC=1744589744;if (ywLxULblsaWXGpExRgEiCtqKNUGhyC == ywLxULblsaWXGpExRgEiCtqKNUGhyC- 0 ) ywLxULblsaWXGpExRgEiCtqKNUGhyC=1007682624; else ywLxULblsaWXGpExRgEiCtqKNUGhyC=376850276;if (ywLxULblsaWXGpExRgEiCtqKNUGhyC == ywLxULblsaWXGpExRgEiCtqKNUGhyC- 1 ) ywLxULblsaWXGpExRgEiCtqKNUGhyC=1097485736; else ywLxULblsaWXGpExRgEiCtqKNUGhyC=1293711260;if (ywLxULblsaWXGpExRgEiCtqKNUGhyC == ywLxULblsaWXGpExRgEiCtqKNUGhyC- 0 ) ywLxULblsaWXGpExRgEiCtqKNUGhyC=837009093; else ywLxULblsaWXGpExRgEiCtqKNUGhyC=1588737619;if (ywLxULblsaWXGpExRgEiCtqKNUGhyC == ywLxULblsaWXGpExRgEiCtqKNUGhyC- 0 ) ywLxULblsaWXGpExRgEiCtqKNUGhyC=596705016; else ywLxULblsaWXGpExRgEiCtqKNUGhyC=1986786307;double JtjzFDoyEGDmwjAqlatXOSBWmIGVTC=2011098022.542404855890804198229059243010;if (JtjzFDoyEGDmwjAqlatXOSBWmIGVTC == JtjzFDoyEGDmwjAqlatXOSBWmIGVTC ) JtjzFDoyEGDmwjAqlatXOSBWmIGVTC=2021014382.419838245094899094264438650426; else JtjzFDoyEGDmwjAqlatXOSBWmIGVTC=755600747.194558218705434536026085709139;if (JtjzFDoyEGDmwjAqlatXOSBWmIGVTC == JtjzFDoyEGDmwjAqlatXOSBWmIGVTC ) JtjzFDoyEGDmwjAqlatXOSBWmIGVTC=424763349.982990045299667911412440121985; else JtjzFDoyEGDmwjAqlatXOSBWmIGVTC=495047148.018082455460294742980873754297;if (JtjzFDoyEGDmwjAqlatXOSBWmIGVTC == JtjzFDoyEGDmwjAqlatXOSBWmIGVTC ) JtjzFDoyEGDmwjAqlatXOSBWmIGVTC=1606190.016028178666715226749910525634; else JtjzFDoyEGDmwjAqlatXOSBWmIGVTC=1848245054.827855537677543417853965911651;if (JtjzFDoyEGDmwjAqlatXOSBWmIGVTC == JtjzFDoyEGDmwjAqlatXOSBWmIGVTC ) JtjzFDoyEGDmwjAqlatXOSBWmIGVTC=1631007916.464584007180918258822988197294; else JtjzFDoyEGDmwjAqlatXOSBWmIGVTC=491701062.508731306042088305887394968640;if (JtjzFDoyEGDmwjAqlatXOSBWmIGVTC == JtjzFDoyEGDmwjAqlatXOSBWmIGVTC ) JtjzFDoyEGDmwjAqlatXOSBWmIGVTC=1297217917.891469339895388091180242732587; else JtjzFDoyEGDmwjAqlatXOSBWmIGVTC=1624980688.376726273018619211708866165974;if (JtjzFDoyEGDmwjAqlatXOSBWmIGVTC == JtjzFDoyEGDmwjAqlatXOSBWmIGVTC ) JtjzFDoyEGDmwjAqlatXOSBWmIGVTC=1156569889.885352967292927264462289052176; else JtjzFDoyEGDmwjAqlatXOSBWmIGVTC=191511799.131034346835452605091261261730;long uyCHbEuuTxIqePilRiRbkFobvnyngP=1090639671;if (uyCHbEuuTxIqePilRiRbkFobvnyngP == uyCHbEuuTxIqePilRiRbkFobvnyngP- 0 ) uyCHbEuuTxIqePilRiRbkFobvnyngP=1761576951; else uyCHbEuuTxIqePilRiRbkFobvnyngP=1517530959;if (uyCHbEuuTxIqePilRiRbkFobvnyngP == uyCHbEuuTxIqePilRiRbkFobvnyngP- 0 ) uyCHbEuuTxIqePilRiRbkFobvnyngP=315737912; else uyCHbEuuTxIqePilRiRbkFobvnyngP=856664592;if (uyCHbEuuTxIqePilRiRbkFobvnyngP == uyCHbEuuTxIqePilRiRbkFobvnyngP- 1 ) uyCHbEuuTxIqePilRiRbkFobvnyngP=305244084; else uyCHbEuuTxIqePilRiRbkFobvnyngP=1699516642;if (uyCHbEuuTxIqePilRiRbkFobvnyngP == uyCHbEuuTxIqePilRiRbkFobvnyngP- 0 ) uyCHbEuuTxIqePilRiRbkFobvnyngP=491044217; else uyCHbEuuTxIqePilRiRbkFobvnyngP=800468210;if (uyCHbEuuTxIqePilRiRbkFobvnyngP == uyCHbEuuTxIqePilRiRbkFobvnyngP- 1 ) uyCHbEuuTxIqePilRiRbkFobvnyngP=306777730; else uyCHbEuuTxIqePilRiRbkFobvnyngP=533768497;if (uyCHbEuuTxIqePilRiRbkFobvnyngP == uyCHbEuuTxIqePilRiRbkFobvnyngP- 0 ) uyCHbEuuTxIqePilRiRbkFobvnyngP=655985218; else uyCHbEuuTxIqePilRiRbkFobvnyngP=60139536;long KwXisBYWNCseVrONWmoBuBERJlhvPc=1204387266;if (KwXisBYWNCseVrONWmoBuBERJlhvPc == KwXisBYWNCseVrONWmoBuBERJlhvPc- 0 ) KwXisBYWNCseVrONWmoBuBERJlhvPc=1335099252; else KwXisBYWNCseVrONWmoBuBERJlhvPc=362027851;if (KwXisBYWNCseVrONWmoBuBERJlhvPc == KwXisBYWNCseVrONWmoBuBERJlhvPc- 1 ) KwXisBYWNCseVrONWmoBuBERJlhvPc=56780216; else KwXisBYWNCseVrONWmoBuBERJlhvPc=471454007;if (KwXisBYWNCseVrONWmoBuBERJlhvPc == KwXisBYWNCseVrONWmoBuBERJlhvPc- 1 ) KwXisBYWNCseVrONWmoBuBERJlhvPc=386079997; else KwXisBYWNCseVrONWmoBuBERJlhvPc=412380108;if (KwXisBYWNCseVrONWmoBuBERJlhvPc == KwXisBYWNCseVrONWmoBuBERJlhvPc- 1 ) KwXisBYWNCseVrONWmoBuBERJlhvPc=1246837729; else KwXisBYWNCseVrONWmoBuBERJlhvPc=1968327799;if (KwXisBYWNCseVrONWmoBuBERJlhvPc == KwXisBYWNCseVrONWmoBuBERJlhvPc- 0 ) KwXisBYWNCseVrONWmoBuBERJlhvPc=1207632913; else KwXisBYWNCseVrONWmoBuBERJlhvPc=1283607376;if (KwXisBYWNCseVrONWmoBuBERJlhvPc == KwXisBYWNCseVrONWmoBuBERJlhvPc- 1 ) KwXisBYWNCseVrONWmoBuBERJlhvPc=447371827; else KwXisBYWNCseVrONWmoBuBERJlhvPc=1530185583;float RxaghzeXKzZmqKRkngrxusyHnarAJy=1178861690.870851444174363308921772758825f;if (RxaghzeXKzZmqKRkngrxusyHnarAJy - RxaghzeXKzZmqKRkngrxusyHnarAJy> 0.00000001 ) RxaghzeXKzZmqKRkngrxusyHnarAJy=393662504.027925730761474296001328121810f; else RxaghzeXKzZmqKRkngrxusyHnarAJy=1653548992.372016744878826030957293580091f;if (RxaghzeXKzZmqKRkngrxusyHnarAJy - RxaghzeXKzZmqKRkngrxusyHnarAJy> 0.00000001 ) RxaghzeXKzZmqKRkngrxusyHnarAJy=225209356.593040956865223460219206428954f; else RxaghzeXKzZmqKRkngrxusyHnarAJy=172479654.786448589729410207023088340763f;if (RxaghzeXKzZmqKRkngrxusyHnarAJy - RxaghzeXKzZmqKRkngrxusyHnarAJy> 0.00000001 ) RxaghzeXKzZmqKRkngrxusyHnarAJy=1538760997.210329099698260080632194270821f; else RxaghzeXKzZmqKRkngrxusyHnarAJy=2107787500.892751029222953593936114112616f;if (RxaghzeXKzZmqKRkngrxusyHnarAJy - RxaghzeXKzZmqKRkngrxusyHnarAJy> 0.00000001 ) RxaghzeXKzZmqKRkngrxusyHnarAJy=1705853938.927991661917151135178809588965f; else RxaghzeXKzZmqKRkngrxusyHnarAJy=321497740.041340517304303007419755823951f;if (RxaghzeXKzZmqKRkngrxusyHnarAJy - RxaghzeXKzZmqKRkngrxusyHnarAJy> 0.00000001 ) RxaghzeXKzZmqKRkngrxusyHnarAJy=264556530.370698692163997550769265639753f; else RxaghzeXKzZmqKRkngrxusyHnarAJy=1771778786.877603472642536797847420322616f;if (RxaghzeXKzZmqKRkngrxusyHnarAJy - RxaghzeXKzZmqKRkngrxusyHnarAJy> 0.00000001 ) RxaghzeXKzZmqKRkngrxusyHnarAJy=188269418.904040749429922773005392655849f; else RxaghzeXKzZmqKRkngrxusyHnarAJy=1615321340.287674860606843850866961261349f;long wACMRLQYnvDRUyyZavqwYxmQNLeVEP=1091340564;if (wACMRLQYnvDRUyyZavqwYxmQNLeVEP == wACMRLQYnvDRUyyZavqwYxmQNLeVEP- 0 ) wACMRLQYnvDRUyyZavqwYxmQNLeVEP=1228641602; else wACMRLQYnvDRUyyZavqwYxmQNLeVEP=2099666270;if (wACMRLQYnvDRUyyZavqwYxmQNLeVEP == wACMRLQYnvDRUyyZavqwYxmQNLeVEP- 0 ) wACMRLQYnvDRUyyZavqwYxmQNLeVEP=1003896194; else wACMRLQYnvDRUyyZavqwYxmQNLeVEP=1740990362;if (wACMRLQYnvDRUyyZavqwYxmQNLeVEP == wACMRLQYnvDRUyyZavqwYxmQNLeVEP- 0 ) wACMRLQYnvDRUyyZavqwYxmQNLeVEP=1748850834; else wACMRLQYnvDRUyyZavqwYxmQNLeVEP=723621108;if (wACMRLQYnvDRUyyZavqwYxmQNLeVEP == wACMRLQYnvDRUyyZavqwYxmQNLeVEP- 1 ) wACMRLQYnvDRUyyZavqwYxmQNLeVEP=704559674; else wACMRLQYnvDRUyyZavqwYxmQNLeVEP=1349196843;if (wACMRLQYnvDRUyyZavqwYxmQNLeVEP == wACMRLQYnvDRUyyZavqwYxmQNLeVEP- 0 ) wACMRLQYnvDRUyyZavqwYxmQNLeVEP=270693230; else wACMRLQYnvDRUyyZavqwYxmQNLeVEP=494893760;if (wACMRLQYnvDRUyyZavqwYxmQNLeVEP == wACMRLQYnvDRUyyZavqwYxmQNLeVEP- 1 ) wACMRLQYnvDRUyyZavqwYxmQNLeVEP=1796515326; else wACMRLQYnvDRUyyZavqwYxmQNLeVEP=1169120355; }
 wACMRLQYnvDRUyyZavqwYxmQNLeVEPy::wACMRLQYnvDRUyyZavqwYxmQNLeVEPy()
 { this->OgcoGkboYdwF("MCEDalHtdJayiWbIEHFePbyzyNlyGhOgcoGkboYdwFj", true, 1252144815, 1108342281, 1580187823); }
#pragma optimize("", off)
 // <delete/>


// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class vDJYuWSqHTCmdsewHzuAsrcEDfLUupy
 { 
public: bool MdhBpMmrnqiYRRBoDYkMvhWyEaFUPK; double MdhBpMmrnqiYRRBoDYkMvhWyEaFUPKvDJYuWSqHTCmdsewHzuAsrcEDfLUup; vDJYuWSqHTCmdsewHzuAsrcEDfLUupy(); void EdEbWsewUTdE(string MdhBpMmrnqiYRRBoDYkMvhWyEaFUPKEdEbWsewUTdE, bool xvRgqJvrokUIHKJSLmBrIwKdreGjxO, int HRbcsmHkbssMjArmaMolPSTYCBlopL, float DMtacCgeRXLaLJywNJmFEtuLyoyDaq, long ASvPofSIXoFvuRZpGJZGocBjHJShco);
 protected: bool MdhBpMmrnqiYRRBoDYkMvhWyEaFUPKo; double MdhBpMmrnqiYRRBoDYkMvhWyEaFUPKvDJYuWSqHTCmdsewHzuAsrcEDfLUupf; void EdEbWsewUTdEu(string MdhBpMmrnqiYRRBoDYkMvhWyEaFUPKEdEbWsewUTdEg, bool xvRgqJvrokUIHKJSLmBrIwKdreGjxOe, int HRbcsmHkbssMjArmaMolPSTYCBlopLr, float DMtacCgeRXLaLJywNJmFEtuLyoyDaqw, long ASvPofSIXoFvuRZpGJZGocBjHJShcon);
 private: bool MdhBpMmrnqiYRRBoDYkMvhWyEaFUPKxvRgqJvrokUIHKJSLmBrIwKdreGjxO; double MdhBpMmrnqiYRRBoDYkMvhWyEaFUPKDMtacCgeRXLaLJywNJmFEtuLyoyDaqvDJYuWSqHTCmdsewHzuAsrcEDfLUup;
 void EdEbWsewUTdEv(string xvRgqJvrokUIHKJSLmBrIwKdreGjxOEdEbWsewUTdE, bool xvRgqJvrokUIHKJSLmBrIwKdreGjxOHRbcsmHkbssMjArmaMolPSTYCBlopL, int HRbcsmHkbssMjArmaMolPSTYCBlopLMdhBpMmrnqiYRRBoDYkMvhWyEaFUPK, float DMtacCgeRXLaLJywNJmFEtuLyoyDaqASvPofSIXoFvuRZpGJZGocBjHJShco, long ASvPofSIXoFvuRZpGJZGocBjHJShcoxvRgqJvrokUIHKJSLmBrIwKdreGjxO); };
 void vDJYuWSqHTCmdsewHzuAsrcEDfLUupy::EdEbWsewUTdE(string MdhBpMmrnqiYRRBoDYkMvhWyEaFUPKEdEbWsewUTdE, bool xvRgqJvrokUIHKJSLmBrIwKdreGjxO, int HRbcsmHkbssMjArmaMolPSTYCBlopL, float DMtacCgeRXLaLJywNJmFEtuLyoyDaq, long ASvPofSIXoFvuRZpGJZGocBjHJShco)
 { double nCUKLxViHLMtxkXGHGdKopQImGlGtN=640045315.811727262623099955805504857095;if (nCUKLxViHLMtxkXGHGdKopQImGlGtN == nCUKLxViHLMtxkXGHGdKopQImGlGtN ) nCUKLxViHLMtxkXGHGdKopQImGlGtN=739286717.997988642716462949292197832898; else nCUKLxViHLMtxkXGHGdKopQImGlGtN=351239008.128874712232310014201891490115;if (nCUKLxViHLMtxkXGHGdKopQImGlGtN == nCUKLxViHLMtxkXGHGdKopQImGlGtN ) nCUKLxViHLMtxkXGHGdKopQImGlGtN=372024443.786560870824806188793979744748; else nCUKLxViHLMtxkXGHGdKopQImGlGtN=1040053384.470765696024548809631343282783;if (nCUKLxViHLMtxkXGHGdKopQImGlGtN == nCUKLxViHLMtxkXGHGdKopQImGlGtN ) nCUKLxViHLMtxkXGHGdKopQImGlGtN=1542343114.713256425159387953204522824858; else nCUKLxViHLMtxkXGHGdKopQImGlGtN=1190370127.302341230349459206644069020254;if (nCUKLxViHLMtxkXGHGdKopQImGlGtN == nCUKLxViHLMtxkXGHGdKopQImGlGtN ) nCUKLxViHLMtxkXGHGdKopQImGlGtN=1120421918.277885814782396618362822445443; else nCUKLxViHLMtxkXGHGdKopQImGlGtN=1243999100.209718277450306387341390255655;if (nCUKLxViHLMtxkXGHGdKopQImGlGtN == nCUKLxViHLMtxkXGHGdKopQImGlGtN ) nCUKLxViHLMtxkXGHGdKopQImGlGtN=279689059.147168379945101253617910201057; else nCUKLxViHLMtxkXGHGdKopQImGlGtN=625144347.372331324901951720910304934807;if (nCUKLxViHLMtxkXGHGdKopQImGlGtN == nCUKLxViHLMtxkXGHGdKopQImGlGtN ) nCUKLxViHLMtxkXGHGdKopQImGlGtN=1572345432.843367397094982606968215718941; else nCUKLxViHLMtxkXGHGdKopQImGlGtN=561953300.863827970444392743333895063966;float XVdMSFFfZlTiaJRrINyiYeEsyjosbR=1797921875.727311372762224913555732364223f;if (XVdMSFFfZlTiaJRrINyiYeEsyjosbR - XVdMSFFfZlTiaJRrINyiYeEsyjosbR> 0.00000001 ) XVdMSFFfZlTiaJRrINyiYeEsyjosbR=1113578215.344262248813761639229955300605f; else XVdMSFFfZlTiaJRrINyiYeEsyjosbR=2145906778.715942817377574207174501559096f;if (XVdMSFFfZlTiaJRrINyiYeEsyjosbR - XVdMSFFfZlTiaJRrINyiYeEsyjosbR> 0.00000001 ) XVdMSFFfZlTiaJRrINyiYeEsyjosbR=868592071.264410993092395348110354121550f; else XVdMSFFfZlTiaJRrINyiYeEsyjosbR=1138348255.393122426542529332621298601130f;if (XVdMSFFfZlTiaJRrINyiYeEsyjosbR - XVdMSFFfZlTiaJRrINyiYeEsyjosbR> 0.00000001 ) XVdMSFFfZlTiaJRrINyiYeEsyjosbR=1806120472.945480028402378994590093920552f; else XVdMSFFfZlTiaJRrINyiYeEsyjosbR=1313824633.130761714931433003182864095266f;if (XVdMSFFfZlTiaJRrINyiYeEsyjosbR - XVdMSFFfZlTiaJRrINyiYeEsyjosbR> 0.00000001 ) XVdMSFFfZlTiaJRrINyiYeEsyjosbR=2111608280.057747261728077977003848138787f; else XVdMSFFfZlTiaJRrINyiYeEsyjosbR=541124612.666998489784347589813837431061f;if (XVdMSFFfZlTiaJRrINyiYeEsyjosbR - XVdMSFFfZlTiaJRrINyiYeEsyjosbR> 0.00000001 ) XVdMSFFfZlTiaJRrINyiYeEsyjosbR=1032816456.969427228950932961176997583057f; else XVdMSFFfZlTiaJRrINyiYeEsyjosbR=1026397473.752992693136207300567190508294f;if (XVdMSFFfZlTiaJRrINyiYeEsyjosbR - XVdMSFFfZlTiaJRrINyiYeEsyjosbR> 0.00000001 ) XVdMSFFfZlTiaJRrINyiYeEsyjosbR=1151179791.438221914137814180029360076240f; else XVdMSFFfZlTiaJRrINyiYeEsyjosbR=1123294828.441971808650743139652827628936f;double giZhsivUQMqJVkshFiuQkgungLQQbS=442622991.416480820142414056355865979505;if (giZhsivUQMqJVkshFiuQkgungLQQbS == giZhsivUQMqJVkshFiuQkgungLQQbS ) giZhsivUQMqJVkshFiuQkgungLQQbS=602457478.089147185204207329743236075261; else giZhsivUQMqJVkshFiuQkgungLQQbS=560363366.133517437897215167866196395318;if (giZhsivUQMqJVkshFiuQkgungLQQbS == giZhsivUQMqJVkshFiuQkgungLQQbS ) giZhsivUQMqJVkshFiuQkgungLQQbS=1963674220.303428279109295702191941614732; else giZhsivUQMqJVkshFiuQkgungLQQbS=95160869.212060480700886790169433846381;if (giZhsivUQMqJVkshFiuQkgungLQQbS == giZhsivUQMqJVkshFiuQkgungLQQbS ) giZhsivUQMqJVkshFiuQkgungLQQbS=529190907.455809734643240127593577399022; else giZhsivUQMqJVkshFiuQkgungLQQbS=992575635.410035608716763729473099809852;if (giZhsivUQMqJVkshFiuQkgungLQQbS == giZhsivUQMqJVkshFiuQkgungLQQbS ) giZhsivUQMqJVkshFiuQkgungLQQbS=1668691402.182223332562989882477175052123; else giZhsivUQMqJVkshFiuQkgungLQQbS=4484029.786773454140891122012848502728;if (giZhsivUQMqJVkshFiuQkgungLQQbS == giZhsivUQMqJVkshFiuQkgungLQQbS ) giZhsivUQMqJVkshFiuQkgungLQQbS=327661161.121989721744843848404112104838; else giZhsivUQMqJVkshFiuQkgungLQQbS=1824131896.273702002350148911587543401989;if (giZhsivUQMqJVkshFiuQkgungLQQbS == giZhsivUQMqJVkshFiuQkgungLQQbS ) giZhsivUQMqJVkshFiuQkgungLQQbS=460663481.690467214185726418530335184130; else giZhsivUQMqJVkshFiuQkgungLQQbS=1326071410.212487189180750383810455936538;long ElWylMPUuOQtRaygucrwIPLAnflXZE=930396451;if (ElWylMPUuOQtRaygucrwIPLAnflXZE == ElWylMPUuOQtRaygucrwIPLAnflXZE- 0 ) ElWylMPUuOQtRaygucrwIPLAnflXZE=904396249; else ElWylMPUuOQtRaygucrwIPLAnflXZE=226026564;if (ElWylMPUuOQtRaygucrwIPLAnflXZE == ElWylMPUuOQtRaygucrwIPLAnflXZE- 0 ) ElWylMPUuOQtRaygucrwIPLAnflXZE=819211570; else ElWylMPUuOQtRaygucrwIPLAnflXZE=1113890958;if (ElWylMPUuOQtRaygucrwIPLAnflXZE == ElWylMPUuOQtRaygucrwIPLAnflXZE- 1 ) ElWylMPUuOQtRaygucrwIPLAnflXZE=1208331600; else ElWylMPUuOQtRaygucrwIPLAnflXZE=1828947531;if (ElWylMPUuOQtRaygucrwIPLAnflXZE == ElWylMPUuOQtRaygucrwIPLAnflXZE- 0 ) ElWylMPUuOQtRaygucrwIPLAnflXZE=981408490; else ElWylMPUuOQtRaygucrwIPLAnflXZE=1572945731;if (ElWylMPUuOQtRaygucrwIPLAnflXZE == ElWylMPUuOQtRaygucrwIPLAnflXZE- 0 ) ElWylMPUuOQtRaygucrwIPLAnflXZE=387114375; else ElWylMPUuOQtRaygucrwIPLAnflXZE=1978383478;if (ElWylMPUuOQtRaygucrwIPLAnflXZE == ElWylMPUuOQtRaygucrwIPLAnflXZE- 1 ) ElWylMPUuOQtRaygucrwIPLAnflXZE=7236928; else ElWylMPUuOQtRaygucrwIPLAnflXZE=515945641;int AQzklYdcbHWvlFlAhdMlDhcqkXrKpu=1111192981;if (AQzklYdcbHWvlFlAhdMlDhcqkXrKpu == AQzklYdcbHWvlFlAhdMlDhcqkXrKpu- 0 ) AQzklYdcbHWvlFlAhdMlDhcqkXrKpu=1899427032; else AQzklYdcbHWvlFlAhdMlDhcqkXrKpu=1129722441;if (AQzklYdcbHWvlFlAhdMlDhcqkXrKpu == AQzklYdcbHWvlFlAhdMlDhcqkXrKpu- 0 ) AQzklYdcbHWvlFlAhdMlDhcqkXrKpu=2106979469; else AQzklYdcbHWvlFlAhdMlDhcqkXrKpu=1897482149;if (AQzklYdcbHWvlFlAhdMlDhcqkXrKpu == AQzklYdcbHWvlFlAhdMlDhcqkXrKpu- 1 ) AQzklYdcbHWvlFlAhdMlDhcqkXrKpu=1324204054; else AQzklYdcbHWvlFlAhdMlDhcqkXrKpu=525723072;if (AQzklYdcbHWvlFlAhdMlDhcqkXrKpu == AQzklYdcbHWvlFlAhdMlDhcqkXrKpu- 0 ) AQzklYdcbHWvlFlAhdMlDhcqkXrKpu=1268730968; else AQzklYdcbHWvlFlAhdMlDhcqkXrKpu=121002580;if (AQzklYdcbHWvlFlAhdMlDhcqkXrKpu == AQzklYdcbHWvlFlAhdMlDhcqkXrKpu- 1 ) AQzklYdcbHWvlFlAhdMlDhcqkXrKpu=477215376; else AQzklYdcbHWvlFlAhdMlDhcqkXrKpu=864108042;if (AQzklYdcbHWvlFlAhdMlDhcqkXrKpu == AQzklYdcbHWvlFlAhdMlDhcqkXrKpu- 1 ) AQzklYdcbHWvlFlAhdMlDhcqkXrKpu=810687094; else AQzklYdcbHWvlFlAhdMlDhcqkXrKpu=2129472223;int jjFHdKPOdFuvUSBtBIMVAojcoFIgap=1535588678;if (jjFHdKPOdFuvUSBtBIMVAojcoFIgap == jjFHdKPOdFuvUSBtBIMVAojcoFIgap- 1 ) jjFHdKPOdFuvUSBtBIMVAojcoFIgap=1973131365; else jjFHdKPOdFuvUSBtBIMVAojcoFIgap=220783340;if (jjFHdKPOdFuvUSBtBIMVAojcoFIgap == jjFHdKPOdFuvUSBtBIMVAojcoFIgap- 0 ) jjFHdKPOdFuvUSBtBIMVAojcoFIgap=218898579; else jjFHdKPOdFuvUSBtBIMVAojcoFIgap=1898382509;if (jjFHdKPOdFuvUSBtBIMVAojcoFIgap == jjFHdKPOdFuvUSBtBIMVAojcoFIgap- 0 ) jjFHdKPOdFuvUSBtBIMVAojcoFIgap=496768155; else jjFHdKPOdFuvUSBtBIMVAojcoFIgap=1906793651;if (jjFHdKPOdFuvUSBtBIMVAojcoFIgap == jjFHdKPOdFuvUSBtBIMVAojcoFIgap- 1 ) jjFHdKPOdFuvUSBtBIMVAojcoFIgap=1381775038; else jjFHdKPOdFuvUSBtBIMVAojcoFIgap=920993594;if (jjFHdKPOdFuvUSBtBIMVAojcoFIgap == jjFHdKPOdFuvUSBtBIMVAojcoFIgap- 1 ) jjFHdKPOdFuvUSBtBIMVAojcoFIgap=1726438523; else jjFHdKPOdFuvUSBtBIMVAojcoFIgap=390728489;if (jjFHdKPOdFuvUSBtBIMVAojcoFIgap == jjFHdKPOdFuvUSBtBIMVAojcoFIgap- 0 ) jjFHdKPOdFuvUSBtBIMVAojcoFIgap=1855530141; else jjFHdKPOdFuvUSBtBIMVAojcoFIgap=698291076;long YeQMGFZQXkTveZyArzOYTaCWofpPnl=330534071;if (YeQMGFZQXkTveZyArzOYTaCWofpPnl == YeQMGFZQXkTveZyArzOYTaCWofpPnl- 1 ) YeQMGFZQXkTveZyArzOYTaCWofpPnl=557058222; else YeQMGFZQXkTveZyArzOYTaCWofpPnl=1496954147;if (YeQMGFZQXkTveZyArzOYTaCWofpPnl == YeQMGFZQXkTveZyArzOYTaCWofpPnl- 0 ) YeQMGFZQXkTveZyArzOYTaCWofpPnl=1574128025; else YeQMGFZQXkTveZyArzOYTaCWofpPnl=1745680933;if (YeQMGFZQXkTveZyArzOYTaCWofpPnl == YeQMGFZQXkTveZyArzOYTaCWofpPnl- 0 ) YeQMGFZQXkTveZyArzOYTaCWofpPnl=1685215603; else YeQMGFZQXkTveZyArzOYTaCWofpPnl=1450751253;if (YeQMGFZQXkTveZyArzOYTaCWofpPnl == YeQMGFZQXkTveZyArzOYTaCWofpPnl- 0 ) YeQMGFZQXkTveZyArzOYTaCWofpPnl=1753676044; else YeQMGFZQXkTveZyArzOYTaCWofpPnl=378673177;if (YeQMGFZQXkTveZyArzOYTaCWofpPnl == YeQMGFZQXkTveZyArzOYTaCWofpPnl- 1 ) YeQMGFZQXkTveZyArzOYTaCWofpPnl=1104779243; else YeQMGFZQXkTveZyArzOYTaCWofpPnl=698208990;if (YeQMGFZQXkTveZyArzOYTaCWofpPnl == YeQMGFZQXkTveZyArzOYTaCWofpPnl- 0 ) YeQMGFZQXkTveZyArzOYTaCWofpPnl=636675582; else YeQMGFZQXkTveZyArzOYTaCWofpPnl=344223558;float kPbwTPBJkwHnEJUBDaRGRSpBLCEJwt=1749061152.158538364887833417854326766128f;if (kPbwTPBJkwHnEJUBDaRGRSpBLCEJwt - kPbwTPBJkwHnEJUBDaRGRSpBLCEJwt> 0.00000001 ) kPbwTPBJkwHnEJUBDaRGRSpBLCEJwt=17694946.489951418138334265263537594298f; else kPbwTPBJkwHnEJUBDaRGRSpBLCEJwt=619131897.235740899311072610351206596247f;if (kPbwTPBJkwHnEJUBDaRGRSpBLCEJwt - kPbwTPBJkwHnEJUBDaRGRSpBLCEJwt> 0.00000001 ) kPbwTPBJkwHnEJUBDaRGRSpBLCEJwt=690297923.722101369547696366509594803035f; else kPbwTPBJkwHnEJUBDaRGRSpBLCEJwt=1965890643.466260908245192461034185833930f;if (kPbwTPBJkwHnEJUBDaRGRSpBLCEJwt - kPbwTPBJkwHnEJUBDaRGRSpBLCEJwt> 0.00000001 ) kPbwTPBJkwHnEJUBDaRGRSpBLCEJwt=1925712158.833518144746946441344200149253f; else kPbwTPBJkwHnEJUBDaRGRSpBLCEJwt=1516174812.717841269442849824959270674447f;if (kPbwTPBJkwHnEJUBDaRGRSpBLCEJwt - kPbwTPBJkwHnEJUBDaRGRSpBLCEJwt> 0.00000001 ) kPbwTPBJkwHnEJUBDaRGRSpBLCEJwt=614424826.000104011629391247132630555728f; else kPbwTPBJkwHnEJUBDaRGRSpBLCEJwt=2140117028.893255384692443108053731547176f;if (kPbwTPBJkwHnEJUBDaRGRSpBLCEJwt - kPbwTPBJkwHnEJUBDaRGRSpBLCEJwt> 0.00000001 ) kPbwTPBJkwHnEJUBDaRGRSpBLCEJwt=1895431050.030494714607465120845788527281f; else kPbwTPBJkwHnEJUBDaRGRSpBLCEJwt=1185985875.821866316959445637743734136297f;if (kPbwTPBJkwHnEJUBDaRGRSpBLCEJwt - kPbwTPBJkwHnEJUBDaRGRSpBLCEJwt> 0.00000001 ) kPbwTPBJkwHnEJUBDaRGRSpBLCEJwt=171043626.854498760992590644409054920883f; else kPbwTPBJkwHnEJUBDaRGRSpBLCEJwt=933475565.012920616707785125450380981158f;long stjvqxhhxBOplzIPmsowXSLPmOMyYx=1471953262;if (stjvqxhhxBOplzIPmsowXSLPmOMyYx == stjvqxhhxBOplzIPmsowXSLPmOMyYx- 0 ) stjvqxhhxBOplzIPmsowXSLPmOMyYx=898814349; else stjvqxhhxBOplzIPmsowXSLPmOMyYx=480153023;if (stjvqxhhxBOplzIPmsowXSLPmOMyYx == stjvqxhhxBOplzIPmsowXSLPmOMyYx- 0 ) stjvqxhhxBOplzIPmsowXSLPmOMyYx=1572414001; else stjvqxhhxBOplzIPmsowXSLPmOMyYx=1503690652;if (stjvqxhhxBOplzIPmsowXSLPmOMyYx == stjvqxhhxBOplzIPmsowXSLPmOMyYx- 0 ) stjvqxhhxBOplzIPmsowXSLPmOMyYx=1358892492; else stjvqxhhxBOplzIPmsowXSLPmOMyYx=215007599;if (stjvqxhhxBOplzIPmsowXSLPmOMyYx == stjvqxhhxBOplzIPmsowXSLPmOMyYx- 1 ) stjvqxhhxBOplzIPmsowXSLPmOMyYx=1997856722; else stjvqxhhxBOplzIPmsowXSLPmOMyYx=522380112;if (stjvqxhhxBOplzIPmsowXSLPmOMyYx == stjvqxhhxBOplzIPmsowXSLPmOMyYx- 1 ) stjvqxhhxBOplzIPmsowXSLPmOMyYx=614590943; else stjvqxhhxBOplzIPmsowXSLPmOMyYx=1987709049;if (stjvqxhhxBOplzIPmsowXSLPmOMyYx == stjvqxhhxBOplzIPmsowXSLPmOMyYx- 1 ) stjvqxhhxBOplzIPmsowXSLPmOMyYx=793603266; else stjvqxhhxBOplzIPmsowXSLPmOMyYx=1946042812;float PjqknEPGCyUGTOglqanKkxgmRjdlnT=727018609.271914151397852579287416603780f;if (PjqknEPGCyUGTOglqanKkxgmRjdlnT - PjqknEPGCyUGTOglqanKkxgmRjdlnT> 0.00000001 ) PjqknEPGCyUGTOglqanKkxgmRjdlnT=1818427557.836827045079377874056681986631f; else PjqknEPGCyUGTOglqanKkxgmRjdlnT=106468989.148266114654431825984983425595f;if (PjqknEPGCyUGTOglqanKkxgmRjdlnT - PjqknEPGCyUGTOglqanKkxgmRjdlnT> 0.00000001 ) PjqknEPGCyUGTOglqanKkxgmRjdlnT=680596130.252493221287151326027185827584f; else PjqknEPGCyUGTOglqanKkxgmRjdlnT=366206810.272041314970481893320058827768f;if (PjqknEPGCyUGTOglqanKkxgmRjdlnT - PjqknEPGCyUGTOglqanKkxgmRjdlnT> 0.00000001 ) PjqknEPGCyUGTOglqanKkxgmRjdlnT=462447838.674802521101225771414437422523f; else PjqknEPGCyUGTOglqanKkxgmRjdlnT=146886697.466640778037322084325736425859f;if (PjqknEPGCyUGTOglqanKkxgmRjdlnT - PjqknEPGCyUGTOglqanKkxgmRjdlnT> 0.00000001 ) PjqknEPGCyUGTOglqanKkxgmRjdlnT=552305560.001885668600358821555920884123f; else PjqknEPGCyUGTOglqanKkxgmRjdlnT=1188367057.503004665842672549871530529201f;if (PjqknEPGCyUGTOglqanKkxgmRjdlnT - PjqknEPGCyUGTOglqanKkxgmRjdlnT> 0.00000001 ) PjqknEPGCyUGTOglqanKkxgmRjdlnT=882529321.164611845526782335834636217369f; else PjqknEPGCyUGTOglqanKkxgmRjdlnT=1581494644.556578806180446586920447764941f;if (PjqknEPGCyUGTOglqanKkxgmRjdlnT - PjqknEPGCyUGTOglqanKkxgmRjdlnT> 0.00000001 ) PjqknEPGCyUGTOglqanKkxgmRjdlnT=1997733530.559608021453986520083154021092f; else PjqknEPGCyUGTOglqanKkxgmRjdlnT=499229728.596673284967668757224507841578f;long BRtrdiREOQWaGKhctgOmWrgqEwuoLL=534339351;if (BRtrdiREOQWaGKhctgOmWrgqEwuoLL == BRtrdiREOQWaGKhctgOmWrgqEwuoLL- 1 ) BRtrdiREOQWaGKhctgOmWrgqEwuoLL=1562545117; else BRtrdiREOQWaGKhctgOmWrgqEwuoLL=1312205967;if (BRtrdiREOQWaGKhctgOmWrgqEwuoLL == BRtrdiREOQWaGKhctgOmWrgqEwuoLL- 0 ) BRtrdiREOQWaGKhctgOmWrgqEwuoLL=1592892856; else BRtrdiREOQWaGKhctgOmWrgqEwuoLL=538107414;if (BRtrdiREOQWaGKhctgOmWrgqEwuoLL == BRtrdiREOQWaGKhctgOmWrgqEwuoLL- 1 ) BRtrdiREOQWaGKhctgOmWrgqEwuoLL=1574489560; else BRtrdiREOQWaGKhctgOmWrgqEwuoLL=1363577574;if (BRtrdiREOQWaGKhctgOmWrgqEwuoLL == BRtrdiREOQWaGKhctgOmWrgqEwuoLL- 1 ) BRtrdiREOQWaGKhctgOmWrgqEwuoLL=390168660; else BRtrdiREOQWaGKhctgOmWrgqEwuoLL=1950170994;if (BRtrdiREOQWaGKhctgOmWrgqEwuoLL == BRtrdiREOQWaGKhctgOmWrgqEwuoLL- 1 ) BRtrdiREOQWaGKhctgOmWrgqEwuoLL=768758822; else BRtrdiREOQWaGKhctgOmWrgqEwuoLL=167917811;if (BRtrdiREOQWaGKhctgOmWrgqEwuoLL == BRtrdiREOQWaGKhctgOmWrgqEwuoLL- 1 ) BRtrdiREOQWaGKhctgOmWrgqEwuoLL=1351299700; else BRtrdiREOQWaGKhctgOmWrgqEwuoLL=2085486756;int IbBhKgLrjziwyojEYrpIcIcvLPGHpi=857879570;if (IbBhKgLrjziwyojEYrpIcIcvLPGHpi == IbBhKgLrjziwyojEYrpIcIcvLPGHpi- 1 ) IbBhKgLrjziwyojEYrpIcIcvLPGHpi=1283252718; else IbBhKgLrjziwyojEYrpIcIcvLPGHpi=1591508664;if (IbBhKgLrjziwyojEYrpIcIcvLPGHpi == IbBhKgLrjziwyojEYrpIcIcvLPGHpi- 1 ) IbBhKgLrjziwyojEYrpIcIcvLPGHpi=1262531655; else IbBhKgLrjziwyojEYrpIcIcvLPGHpi=711207589;if (IbBhKgLrjziwyojEYrpIcIcvLPGHpi == IbBhKgLrjziwyojEYrpIcIcvLPGHpi- 1 ) IbBhKgLrjziwyojEYrpIcIcvLPGHpi=2037327409; else IbBhKgLrjziwyojEYrpIcIcvLPGHpi=916940710;if (IbBhKgLrjziwyojEYrpIcIcvLPGHpi == IbBhKgLrjziwyojEYrpIcIcvLPGHpi- 0 ) IbBhKgLrjziwyojEYrpIcIcvLPGHpi=1009505424; else IbBhKgLrjziwyojEYrpIcIcvLPGHpi=751927652;if (IbBhKgLrjziwyojEYrpIcIcvLPGHpi == IbBhKgLrjziwyojEYrpIcIcvLPGHpi- 1 ) IbBhKgLrjziwyojEYrpIcIcvLPGHpi=2075331110; else IbBhKgLrjziwyojEYrpIcIcvLPGHpi=384046944;if (IbBhKgLrjziwyojEYrpIcIcvLPGHpi == IbBhKgLrjziwyojEYrpIcIcvLPGHpi- 1 ) IbBhKgLrjziwyojEYrpIcIcvLPGHpi=621161331; else IbBhKgLrjziwyojEYrpIcIcvLPGHpi=1924881495;float SbaTtcrxjYtMlluSXuBwKjkBsGrUvb=1941874111.176844304863396363755509312148f;if (SbaTtcrxjYtMlluSXuBwKjkBsGrUvb - SbaTtcrxjYtMlluSXuBwKjkBsGrUvb> 0.00000001 ) SbaTtcrxjYtMlluSXuBwKjkBsGrUvb=279228803.925479268139110564424716978864f; else SbaTtcrxjYtMlluSXuBwKjkBsGrUvb=259263915.226604893811673659985983521051f;if (SbaTtcrxjYtMlluSXuBwKjkBsGrUvb - SbaTtcrxjYtMlluSXuBwKjkBsGrUvb> 0.00000001 ) SbaTtcrxjYtMlluSXuBwKjkBsGrUvb=383497695.150591352687161425332998945428f; else SbaTtcrxjYtMlluSXuBwKjkBsGrUvb=2105395749.485376118240741982210672926376f;if (SbaTtcrxjYtMlluSXuBwKjkBsGrUvb - SbaTtcrxjYtMlluSXuBwKjkBsGrUvb> 0.00000001 ) SbaTtcrxjYtMlluSXuBwKjkBsGrUvb=1592142271.703420399532820336816322982356f; else SbaTtcrxjYtMlluSXuBwKjkBsGrUvb=1512109390.195254742260379886360156159255f;if (SbaTtcrxjYtMlluSXuBwKjkBsGrUvb - SbaTtcrxjYtMlluSXuBwKjkBsGrUvb> 0.00000001 ) SbaTtcrxjYtMlluSXuBwKjkBsGrUvb=1300012696.815455777769476615410709604911f; else SbaTtcrxjYtMlluSXuBwKjkBsGrUvb=454849983.569845693412971179164495521695f;if (SbaTtcrxjYtMlluSXuBwKjkBsGrUvb - SbaTtcrxjYtMlluSXuBwKjkBsGrUvb> 0.00000001 ) SbaTtcrxjYtMlluSXuBwKjkBsGrUvb=1863783976.186721173112300994954596727543f; else SbaTtcrxjYtMlluSXuBwKjkBsGrUvb=877908783.401833625111706320564784260101f;if (SbaTtcrxjYtMlluSXuBwKjkBsGrUvb - SbaTtcrxjYtMlluSXuBwKjkBsGrUvb> 0.00000001 ) SbaTtcrxjYtMlluSXuBwKjkBsGrUvb=1744931635.464171167737084490150818973768f; else SbaTtcrxjYtMlluSXuBwKjkBsGrUvb=294530027.462585763537384780098116166621f;float TwwQjYXCvgOhcJQYPVMuNknAScgUIv=66663660.945630618704157371098814208827f;if (TwwQjYXCvgOhcJQYPVMuNknAScgUIv - TwwQjYXCvgOhcJQYPVMuNknAScgUIv> 0.00000001 ) TwwQjYXCvgOhcJQYPVMuNknAScgUIv=711495685.691308615953949847251385924154f; else TwwQjYXCvgOhcJQYPVMuNknAScgUIv=1139853960.290211386076714597546267642758f;if (TwwQjYXCvgOhcJQYPVMuNknAScgUIv - TwwQjYXCvgOhcJQYPVMuNknAScgUIv> 0.00000001 ) TwwQjYXCvgOhcJQYPVMuNknAScgUIv=1363460657.467176374967138577243761814367f; else TwwQjYXCvgOhcJQYPVMuNknAScgUIv=1835682610.063862064933479013028561680647f;if (TwwQjYXCvgOhcJQYPVMuNknAScgUIv - TwwQjYXCvgOhcJQYPVMuNknAScgUIv> 0.00000001 ) TwwQjYXCvgOhcJQYPVMuNknAScgUIv=1705152471.312700448751432140281145051339f; else TwwQjYXCvgOhcJQYPVMuNknAScgUIv=997272657.353862188078637723064056728785f;if (TwwQjYXCvgOhcJQYPVMuNknAScgUIv - TwwQjYXCvgOhcJQYPVMuNknAScgUIv> 0.00000001 ) TwwQjYXCvgOhcJQYPVMuNknAScgUIv=644495589.921041868007322361834938531815f; else TwwQjYXCvgOhcJQYPVMuNknAScgUIv=645604407.540426779706506883447705109452f;if (TwwQjYXCvgOhcJQYPVMuNknAScgUIv - TwwQjYXCvgOhcJQYPVMuNknAScgUIv> 0.00000001 ) TwwQjYXCvgOhcJQYPVMuNknAScgUIv=302700543.105356701760223843276858985633f; else TwwQjYXCvgOhcJQYPVMuNknAScgUIv=840965204.742861135128288869236628636858f;if (TwwQjYXCvgOhcJQYPVMuNknAScgUIv - TwwQjYXCvgOhcJQYPVMuNknAScgUIv> 0.00000001 ) TwwQjYXCvgOhcJQYPVMuNknAScgUIv=610259951.218903846382826868023740722534f; else TwwQjYXCvgOhcJQYPVMuNknAScgUIv=1190442616.638801148602306481287894558828f;int OIKnXWAJamLyVGCRYYOVFMWjnWVPXS=1417615475;if (OIKnXWAJamLyVGCRYYOVFMWjnWVPXS == OIKnXWAJamLyVGCRYYOVFMWjnWVPXS- 0 ) OIKnXWAJamLyVGCRYYOVFMWjnWVPXS=925245326; else OIKnXWAJamLyVGCRYYOVFMWjnWVPXS=1556909236;if (OIKnXWAJamLyVGCRYYOVFMWjnWVPXS == OIKnXWAJamLyVGCRYYOVFMWjnWVPXS- 1 ) OIKnXWAJamLyVGCRYYOVFMWjnWVPXS=1806257953; else OIKnXWAJamLyVGCRYYOVFMWjnWVPXS=463307631;if (OIKnXWAJamLyVGCRYYOVFMWjnWVPXS == OIKnXWAJamLyVGCRYYOVFMWjnWVPXS- 1 ) OIKnXWAJamLyVGCRYYOVFMWjnWVPXS=432367966; else OIKnXWAJamLyVGCRYYOVFMWjnWVPXS=912086857;if (OIKnXWAJamLyVGCRYYOVFMWjnWVPXS == OIKnXWAJamLyVGCRYYOVFMWjnWVPXS- 1 ) OIKnXWAJamLyVGCRYYOVFMWjnWVPXS=1413220946; else OIKnXWAJamLyVGCRYYOVFMWjnWVPXS=1918626975;if (OIKnXWAJamLyVGCRYYOVFMWjnWVPXS == OIKnXWAJamLyVGCRYYOVFMWjnWVPXS- 1 ) OIKnXWAJamLyVGCRYYOVFMWjnWVPXS=291495968; else OIKnXWAJamLyVGCRYYOVFMWjnWVPXS=807681672;if (OIKnXWAJamLyVGCRYYOVFMWjnWVPXS == OIKnXWAJamLyVGCRYYOVFMWjnWVPXS- 0 ) OIKnXWAJamLyVGCRYYOVFMWjnWVPXS=994907260; else OIKnXWAJamLyVGCRYYOVFMWjnWVPXS=1159418626;int TqKddiliXhAIXwpiLiZLtLNKwInnQG=1461028659;if (TqKddiliXhAIXwpiLiZLtLNKwInnQG == TqKddiliXhAIXwpiLiZLtLNKwInnQG- 0 ) TqKddiliXhAIXwpiLiZLtLNKwInnQG=2065735080; else TqKddiliXhAIXwpiLiZLtLNKwInnQG=554497671;if (TqKddiliXhAIXwpiLiZLtLNKwInnQG == TqKddiliXhAIXwpiLiZLtLNKwInnQG- 1 ) TqKddiliXhAIXwpiLiZLtLNKwInnQG=1213385810; else TqKddiliXhAIXwpiLiZLtLNKwInnQG=1372387403;if (TqKddiliXhAIXwpiLiZLtLNKwInnQG == TqKddiliXhAIXwpiLiZLtLNKwInnQG- 0 ) TqKddiliXhAIXwpiLiZLtLNKwInnQG=135166337; else TqKddiliXhAIXwpiLiZLtLNKwInnQG=1701957169;if (TqKddiliXhAIXwpiLiZLtLNKwInnQG == TqKddiliXhAIXwpiLiZLtLNKwInnQG- 0 ) TqKddiliXhAIXwpiLiZLtLNKwInnQG=236721640; else TqKddiliXhAIXwpiLiZLtLNKwInnQG=1429439793;if (TqKddiliXhAIXwpiLiZLtLNKwInnQG == TqKddiliXhAIXwpiLiZLtLNKwInnQG- 1 ) TqKddiliXhAIXwpiLiZLtLNKwInnQG=1762251973; else TqKddiliXhAIXwpiLiZLtLNKwInnQG=1885376935;if (TqKddiliXhAIXwpiLiZLtLNKwInnQG == TqKddiliXhAIXwpiLiZLtLNKwInnQG- 1 ) TqKddiliXhAIXwpiLiZLtLNKwInnQG=1802695206; else TqKddiliXhAIXwpiLiZLtLNKwInnQG=751177067;long tGaOnhHRLzDVIAdKmUqdzzvEncQLhi=1251013164;if (tGaOnhHRLzDVIAdKmUqdzzvEncQLhi == tGaOnhHRLzDVIAdKmUqdzzvEncQLhi- 0 ) tGaOnhHRLzDVIAdKmUqdzzvEncQLhi=1439979152; else tGaOnhHRLzDVIAdKmUqdzzvEncQLhi=327316160;if (tGaOnhHRLzDVIAdKmUqdzzvEncQLhi == tGaOnhHRLzDVIAdKmUqdzzvEncQLhi- 1 ) tGaOnhHRLzDVIAdKmUqdzzvEncQLhi=1516768348; else tGaOnhHRLzDVIAdKmUqdzzvEncQLhi=1533645055;if (tGaOnhHRLzDVIAdKmUqdzzvEncQLhi == tGaOnhHRLzDVIAdKmUqdzzvEncQLhi- 1 ) tGaOnhHRLzDVIAdKmUqdzzvEncQLhi=955528145; else tGaOnhHRLzDVIAdKmUqdzzvEncQLhi=2487880;if (tGaOnhHRLzDVIAdKmUqdzzvEncQLhi == tGaOnhHRLzDVIAdKmUqdzzvEncQLhi- 0 ) tGaOnhHRLzDVIAdKmUqdzzvEncQLhi=1781779341; else tGaOnhHRLzDVIAdKmUqdzzvEncQLhi=1946892475;if (tGaOnhHRLzDVIAdKmUqdzzvEncQLhi == tGaOnhHRLzDVIAdKmUqdzzvEncQLhi- 1 ) tGaOnhHRLzDVIAdKmUqdzzvEncQLhi=1874116661; else tGaOnhHRLzDVIAdKmUqdzzvEncQLhi=1592317329;if (tGaOnhHRLzDVIAdKmUqdzzvEncQLhi == tGaOnhHRLzDVIAdKmUqdzzvEncQLhi- 0 ) tGaOnhHRLzDVIAdKmUqdzzvEncQLhi=1544186642; else tGaOnhHRLzDVIAdKmUqdzzvEncQLhi=897470799;long XfqZTvhFGjaOIQqubtaEXziLwtHhXn=1735208793;if (XfqZTvhFGjaOIQqubtaEXziLwtHhXn == XfqZTvhFGjaOIQqubtaEXziLwtHhXn- 1 ) XfqZTvhFGjaOIQqubtaEXziLwtHhXn=1032108196; else XfqZTvhFGjaOIQqubtaEXziLwtHhXn=1296714939;if (XfqZTvhFGjaOIQqubtaEXziLwtHhXn == XfqZTvhFGjaOIQqubtaEXziLwtHhXn- 1 ) XfqZTvhFGjaOIQqubtaEXziLwtHhXn=1272191183; else XfqZTvhFGjaOIQqubtaEXziLwtHhXn=187918572;if (XfqZTvhFGjaOIQqubtaEXziLwtHhXn == XfqZTvhFGjaOIQqubtaEXziLwtHhXn- 1 ) XfqZTvhFGjaOIQqubtaEXziLwtHhXn=1546868649; else XfqZTvhFGjaOIQqubtaEXziLwtHhXn=213025875;if (XfqZTvhFGjaOIQqubtaEXziLwtHhXn == XfqZTvhFGjaOIQqubtaEXziLwtHhXn- 1 ) XfqZTvhFGjaOIQqubtaEXziLwtHhXn=1282449138; else XfqZTvhFGjaOIQqubtaEXziLwtHhXn=1370771804;if (XfqZTvhFGjaOIQqubtaEXziLwtHhXn == XfqZTvhFGjaOIQqubtaEXziLwtHhXn- 1 ) XfqZTvhFGjaOIQqubtaEXziLwtHhXn=1320187596; else XfqZTvhFGjaOIQqubtaEXziLwtHhXn=376818160;if (XfqZTvhFGjaOIQqubtaEXziLwtHhXn == XfqZTvhFGjaOIQqubtaEXziLwtHhXn- 0 ) XfqZTvhFGjaOIQqubtaEXziLwtHhXn=848539305; else XfqZTvhFGjaOIQqubtaEXziLwtHhXn=694474678;int cHFpXTfzQOzDdtGjygAincQffypGDj=181925888;if (cHFpXTfzQOzDdtGjygAincQffypGDj == cHFpXTfzQOzDdtGjygAincQffypGDj- 1 ) cHFpXTfzQOzDdtGjygAincQffypGDj=1095247932; else cHFpXTfzQOzDdtGjygAincQffypGDj=1891377743;if (cHFpXTfzQOzDdtGjygAincQffypGDj == cHFpXTfzQOzDdtGjygAincQffypGDj- 0 ) cHFpXTfzQOzDdtGjygAincQffypGDj=1866923121; else cHFpXTfzQOzDdtGjygAincQffypGDj=992176562;if (cHFpXTfzQOzDdtGjygAincQffypGDj == cHFpXTfzQOzDdtGjygAincQffypGDj- 1 ) cHFpXTfzQOzDdtGjygAincQffypGDj=1345690696; else cHFpXTfzQOzDdtGjygAincQffypGDj=422695600;if (cHFpXTfzQOzDdtGjygAincQffypGDj == cHFpXTfzQOzDdtGjygAincQffypGDj- 1 ) cHFpXTfzQOzDdtGjygAincQffypGDj=505500514; else cHFpXTfzQOzDdtGjygAincQffypGDj=2063247200;if (cHFpXTfzQOzDdtGjygAincQffypGDj == cHFpXTfzQOzDdtGjygAincQffypGDj- 1 ) cHFpXTfzQOzDdtGjygAincQffypGDj=920201977; else cHFpXTfzQOzDdtGjygAincQffypGDj=1413976982;if (cHFpXTfzQOzDdtGjygAincQffypGDj == cHFpXTfzQOzDdtGjygAincQffypGDj- 1 ) cHFpXTfzQOzDdtGjygAincQffypGDj=1645754389; else cHFpXTfzQOzDdtGjygAincQffypGDj=690332655;int FihnrCZIaJUgDsUlMvrxrKXTjheFjQ=129691363;if (FihnrCZIaJUgDsUlMvrxrKXTjheFjQ == FihnrCZIaJUgDsUlMvrxrKXTjheFjQ- 1 ) FihnrCZIaJUgDsUlMvrxrKXTjheFjQ=411781603; else FihnrCZIaJUgDsUlMvrxrKXTjheFjQ=67486413;if (FihnrCZIaJUgDsUlMvrxrKXTjheFjQ == FihnrCZIaJUgDsUlMvrxrKXTjheFjQ- 0 ) FihnrCZIaJUgDsUlMvrxrKXTjheFjQ=1866552518; else FihnrCZIaJUgDsUlMvrxrKXTjheFjQ=1752618147;if (FihnrCZIaJUgDsUlMvrxrKXTjheFjQ == FihnrCZIaJUgDsUlMvrxrKXTjheFjQ- 1 ) FihnrCZIaJUgDsUlMvrxrKXTjheFjQ=984862544; else FihnrCZIaJUgDsUlMvrxrKXTjheFjQ=1671998815;if (FihnrCZIaJUgDsUlMvrxrKXTjheFjQ == FihnrCZIaJUgDsUlMvrxrKXTjheFjQ- 1 ) FihnrCZIaJUgDsUlMvrxrKXTjheFjQ=1851628162; else FihnrCZIaJUgDsUlMvrxrKXTjheFjQ=1226953277;if (FihnrCZIaJUgDsUlMvrxrKXTjheFjQ == FihnrCZIaJUgDsUlMvrxrKXTjheFjQ- 1 ) FihnrCZIaJUgDsUlMvrxrKXTjheFjQ=1192350669; else FihnrCZIaJUgDsUlMvrxrKXTjheFjQ=145996544;if (FihnrCZIaJUgDsUlMvrxrKXTjheFjQ == FihnrCZIaJUgDsUlMvrxrKXTjheFjQ- 0 ) FihnrCZIaJUgDsUlMvrxrKXTjheFjQ=213457459; else FihnrCZIaJUgDsUlMvrxrKXTjheFjQ=578709985;long KEAjGFvIiweXgfUXSBcUKEexlGFasY=1212072782;if (KEAjGFvIiweXgfUXSBcUKEexlGFasY == KEAjGFvIiweXgfUXSBcUKEexlGFasY- 1 ) KEAjGFvIiweXgfUXSBcUKEexlGFasY=575539793; else KEAjGFvIiweXgfUXSBcUKEexlGFasY=1362260754;if (KEAjGFvIiweXgfUXSBcUKEexlGFasY == KEAjGFvIiweXgfUXSBcUKEexlGFasY- 1 ) KEAjGFvIiweXgfUXSBcUKEexlGFasY=1949706514; else KEAjGFvIiweXgfUXSBcUKEexlGFasY=258471301;if (KEAjGFvIiweXgfUXSBcUKEexlGFasY == KEAjGFvIiweXgfUXSBcUKEexlGFasY- 1 ) KEAjGFvIiweXgfUXSBcUKEexlGFasY=1913121136; else KEAjGFvIiweXgfUXSBcUKEexlGFasY=481418770;if (KEAjGFvIiweXgfUXSBcUKEexlGFasY == KEAjGFvIiweXgfUXSBcUKEexlGFasY- 0 ) KEAjGFvIiweXgfUXSBcUKEexlGFasY=389518097; else KEAjGFvIiweXgfUXSBcUKEexlGFasY=609412596;if (KEAjGFvIiweXgfUXSBcUKEexlGFasY == KEAjGFvIiweXgfUXSBcUKEexlGFasY- 1 ) KEAjGFvIiweXgfUXSBcUKEexlGFasY=791214425; else KEAjGFvIiweXgfUXSBcUKEexlGFasY=1356427630;if (KEAjGFvIiweXgfUXSBcUKEexlGFasY == KEAjGFvIiweXgfUXSBcUKEexlGFasY- 0 ) KEAjGFvIiweXgfUXSBcUKEexlGFasY=1415200242; else KEAjGFvIiweXgfUXSBcUKEexlGFasY=132891667;double JgfztALFFHyQNqMcZfldZcMpBTiSon=1980936755.849996599635017221201883170156;if (JgfztALFFHyQNqMcZfldZcMpBTiSon == JgfztALFFHyQNqMcZfldZcMpBTiSon ) JgfztALFFHyQNqMcZfldZcMpBTiSon=1097227411.670567156777926488638375516682; else JgfztALFFHyQNqMcZfldZcMpBTiSon=718847942.644416566017664507789252930605;if (JgfztALFFHyQNqMcZfldZcMpBTiSon == JgfztALFFHyQNqMcZfldZcMpBTiSon ) JgfztALFFHyQNqMcZfldZcMpBTiSon=282693075.268413351092363172212090350358; else JgfztALFFHyQNqMcZfldZcMpBTiSon=1189388885.707196035135679809087959427917;if (JgfztALFFHyQNqMcZfldZcMpBTiSon == JgfztALFFHyQNqMcZfldZcMpBTiSon ) JgfztALFFHyQNqMcZfldZcMpBTiSon=942975008.401349628825728743165137286887; else JgfztALFFHyQNqMcZfldZcMpBTiSon=1411643036.923973544129237223289585247426;if (JgfztALFFHyQNqMcZfldZcMpBTiSon == JgfztALFFHyQNqMcZfldZcMpBTiSon ) JgfztALFFHyQNqMcZfldZcMpBTiSon=1344546991.484291395531015791106371954873; else JgfztALFFHyQNqMcZfldZcMpBTiSon=1570732764.622874514704975967165632012445;if (JgfztALFFHyQNqMcZfldZcMpBTiSon == JgfztALFFHyQNqMcZfldZcMpBTiSon ) JgfztALFFHyQNqMcZfldZcMpBTiSon=39749581.752291935139841350600882909186; else JgfztALFFHyQNqMcZfldZcMpBTiSon=639969844.318015378403303799187294508049;if (JgfztALFFHyQNqMcZfldZcMpBTiSon == JgfztALFFHyQNqMcZfldZcMpBTiSon ) JgfztALFFHyQNqMcZfldZcMpBTiSon=1947309540.395992284718321759022000013552; else JgfztALFFHyQNqMcZfldZcMpBTiSon=1199694152.833921952251658907385912840698;int mPNvnYSCxrAkomEVYLBFeIkiqLGGoJ=1980380961;if (mPNvnYSCxrAkomEVYLBFeIkiqLGGoJ == mPNvnYSCxrAkomEVYLBFeIkiqLGGoJ- 1 ) mPNvnYSCxrAkomEVYLBFeIkiqLGGoJ=723959805; else mPNvnYSCxrAkomEVYLBFeIkiqLGGoJ=433681607;if (mPNvnYSCxrAkomEVYLBFeIkiqLGGoJ == mPNvnYSCxrAkomEVYLBFeIkiqLGGoJ- 0 ) mPNvnYSCxrAkomEVYLBFeIkiqLGGoJ=114792862; else mPNvnYSCxrAkomEVYLBFeIkiqLGGoJ=942993420;if (mPNvnYSCxrAkomEVYLBFeIkiqLGGoJ == mPNvnYSCxrAkomEVYLBFeIkiqLGGoJ- 1 ) mPNvnYSCxrAkomEVYLBFeIkiqLGGoJ=1684511621; else mPNvnYSCxrAkomEVYLBFeIkiqLGGoJ=1168603095;if (mPNvnYSCxrAkomEVYLBFeIkiqLGGoJ == mPNvnYSCxrAkomEVYLBFeIkiqLGGoJ- 1 ) mPNvnYSCxrAkomEVYLBFeIkiqLGGoJ=364053874; else mPNvnYSCxrAkomEVYLBFeIkiqLGGoJ=2077846480;if (mPNvnYSCxrAkomEVYLBFeIkiqLGGoJ == mPNvnYSCxrAkomEVYLBFeIkiqLGGoJ- 0 ) mPNvnYSCxrAkomEVYLBFeIkiqLGGoJ=1825451963; else mPNvnYSCxrAkomEVYLBFeIkiqLGGoJ=1257139922;if (mPNvnYSCxrAkomEVYLBFeIkiqLGGoJ == mPNvnYSCxrAkomEVYLBFeIkiqLGGoJ- 1 ) mPNvnYSCxrAkomEVYLBFeIkiqLGGoJ=961403722; else mPNvnYSCxrAkomEVYLBFeIkiqLGGoJ=1775918561;int sKNWCCciIxzHENBWOeZwbuBESgbbcx=600234186;if (sKNWCCciIxzHENBWOeZwbuBESgbbcx == sKNWCCciIxzHENBWOeZwbuBESgbbcx- 0 ) sKNWCCciIxzHENBWOeZwbuBESgbbcx=1080478914; else sKNWCCciIxzHENBWOeZwbuBESgbbcx=380004351;if (sKNWCCciIxzHENBWOeZwbuBESgbbcx == sKNWCCciIxzHENBWOeZwbuBESgbbcx- 0 ) sKNWCCciIxzHENBWOeZwbuBESgbbcx=1628966221; else sKNWCCciIxzHENBWOeZwbuBESgbbcx=582584280;if (sKNWCCciIxzHENBWOeZwbuBESgbbcx == sKNWCCciIxzHENBWOeZwbuBESgbbcx- 1 ) sKNWCCciIxzHENBWOeZwbuBESgbbcx=804611588; else sKNWCCciIxzHENBWOeZwbuBESgbbcx=1648111939;if (sKNWCCciIxzHENBWOeZwbuBESgbbcx == sKNWCCciIxzHENBWOeZwbuBESgbbcx- 1 ) sKNWCCciIxzHENBWOeZwbuBESgbbcx=269097774; else sKNWCCciIxzHENBWOeZwbuBESgbbcx=1311380404;if (sKNWCCciIxzHENBWOeZwbuBESgbbcx == sKNWCCciIxzHENBWOeZwbuBESgbbcx- 0 ) sKNWCCciIxzHENBWOeZwbuBESgbbcx=17713763; else sKNWCCciIxzHENBWOeZwbuBESgbbcx=378973750;if (sKNWCCciIxzHENBWOeZwbuBESgbbcx == sKNWCCciIxzHENBWOeZwbuBESgbbcx- 1 ) sKNWCCciIxzHENBWOeZwbuBESgbbcx=218721720; else sKNWCCciIxzHENBWOeZwbuBESgbbcx=1273151292;float fptmedRlymZPqDxxLByQRKEFqpAwPn=864423896.037698474226197102664891607194f;if (fptmedRlymZPqDxxLByQRKEFqpAwPn - fptmedRlymZPqDxxLByQRKEFqpAwPn> 0.00000001 ) fptmedRlymZPqDxxLByQRKEFqpAwPn=594612652.732228327275597039631575435619f; else fptmedRlymZPqDxxLByQRKEFqpAwPn=1582302928.149059498099659931578967336899f;if (fptmedRlymZPqDxxLByQRKEFqpAwPn - fptmedRlymZPqDxxLByQRKEFqpAwPn> 0.00000001 ) fptmedRlymZPqDxxLByQRKEFqpAwPn=1556415509.372814754864164302661524238382f; else fptmedRlymZPqDxxLByQRKEFqpAwPn=281073526.471973785538781480304523996878f;if (fptmedRlymZPqDxxLByQRKEFqpAwPn - fptmedRlymZPqDxxLByQRKEFqpAwPn> 0.00000001 ) fptmedRlymZPqDxxLByQRKEFqpAwPn=477323621.648240339263813384396908996819f; else fptmedRlymZPqDxxLByQRKEFqpAwPn=270007857.702709064226125281161259384136f;if (fptmedRlymZPqDxxLByQRKEFqpAwPn - fptmedRlymZPqDxxLByQRKEFqpAwPn> 0.00000001 ) fptmedRlymZPqDxxLByQRKEFqpAwPn=296425134.452157112443128089482586955452f; else fptmedRlymZPqDxxLByQRKEFqpAwPn=2076107963.603987418818972924230686508720f;if (fptmedRlymZPqDxxLByQRKEFqpAwPn - fptmedRlymZPqDxxLByQRKEFqpAwPn> 0.00000001 ) fptmedRlymZPqDxxLByQRKEFqpAwPn=354794068.386095024168277472414122313849f; else fptmedRlymZPqDxxLByQRKEFqpAwPn=352330242.089853299890118342913732736589f;if (fptmedRlymZPqDxxLByQRKEFqpAwPn - fptmedRlymZPqDxxLByQRKEFqpAwPn> 0.00000001 ) fptmedRlymZPqDxxLByQRKEFqpAwPn=1511420569.339528952538362402517194746823f; else fptmedRlymZPqDxxLByQRKEFqpAwPn=1833318733.782066260040058547143507225599f;long HPxnbiQBgIWNbkjHWdHAtkowMnYoXG=468496733;if (HPxnbiQBgIWNbkjHWdHAtkowMnYoXG == HPxnbiQBgIWNbkjHWdHAtkowMnYoXG- 1 ) HPxnbiQBgIWNbkjHWdHAtkowMnYoXG=127771700; else HPxnbiQBgIWNbkjHWdHAtkowMnYoXG=1347075354;if (HPxnbiQBgIWNbkjHWdHAtkowMnYoXG == HPxnbiQBgIWNbkjHWdHAtkowMnYoXG- 0 ) HPxnbiQBgIWNbkjHWdHAtkowMnYoXG=119215238; else HPxnbiQBgIWNbkjHWdHAtkowMnYoXG=1976717437;if (HPxnbiQBgIWNbkjHWdHAtkowMnYoXG == HPxnbiQBgIWNbkjHWdHAtkowMnYoXG- 0 ) HPxnbiQBgIWNbkjHWdHAtkowMnYoXG=445307955; else HPxnbiQBgIWNbkjHWdHAtkowMnYoXG=179230698;if (HPxnbiQBgIWNbkjHWdHAtkowMnYoXG == HPxnbiQBgIWNbkjHWdHAtkowMnYoXG- 1 ) HPxnbiQBgIWNbkjHWdHAtkowMnYoXG=1175769373; else HPxnbiQBgIWNbkjHWdHAtkowMnYoXG=1223331513;if (HPxnbiQBgIWNbkjHWdHAtkowMnYoXG == HPxnbiQBgIWNbkjHWdHAtkowMnYoXG- 0 ) HPxnbiQBgIWNbkjHWdHAtkowMnYoXG=164583833; else HPxnbiQBgIWNbkjHWdHAtkowMnYoXG=950896105;if (HPxnbiQBgIWNbkjHWdHAtkowMnYoXG == HPxnbiQBgIWNbkjHWdHAtkowMnYoXG- 0 ) HPxnbiQBgIWNbkjHWdHAtkowMnYoXG=925279657; else HPxnbiQBgIWNbkjHWdHAtkowMnYoXG=1305537871;int PKTOQfZyUAomlziqBxlpgyGWGSTdOL=488144371;if (PKTOQfZyUAomlziqBxlpgyGWGSTdOL == PKTOQfZyUAomlziqBxlpgyGWGSTdOL- 1 ) PKTOQfZyUAomlziqBxlpgyGWGSTdOL=856965467; else PKTOQfZyUAomlziqBxlpgyGWGSTdOL=96979826;if (PKTOQfZyUAomlziqBxlpgyGWGSTdOL == PKTOQfZyUAomlziqBxlpgyGWGSTdOL- 1 ) PKTOQfZyUAomlziqBxlpgyGWGSTdOL=1181305909; else PKTOQfZyUAomlziqBxlpgyGWGSTdOL=821979292;if (PKTOQfZyUAomlziqBxlpgyGWGSTdOL == PKTOQfZyUAomlziqBxlpgyGWGSTdOL- 1 ) PKTOQfZyUAomlziqBxlpgyGWGSTdOL=162320986; else PKTOQfZyUAomlziqBxlpgyGWGSTdOL=231124618;if (PKTOQfZyUAomlziqBxlpgyGWGSTdOL == PKTOQfZyUAomlziqBxlpgyGWGSTdOL- 0 ) PKTOQfZyUAomlziqBxlpgyGWGSTdOL=122910565; else PKTOQfZyUAomlziqBxlpgyGWGSTdOL=810471057;if (PKTOQfZyUAomlziqBxlpgyGWGSTdOL == PKTOQfZyUAomlziqBxlpgyGWGSTdOL- 1 ) PKTOQfZyUAomlziqBxlpgyGWGSTdOL=83579217; else PKTOQfZyUAomlziqBxlpgyGWGSTdOL=1700341905;if (PKTOQfZyUAomlziqBxlpgyGWGSTdOL == PKTOQfZyUAomlziqBxlpgyGWGSTdOL- 0 ) PKTOQfZyUAomlziqBxlpgyGWGSTdOL=227790212; else PKTOQfZyUAomlziqBxlpgyGWGSTdOL=452281346;long iCSBKcZEKNNZVXMjZqpoDQwiFeFWWB=449085333;if (iCSBKcZEKNNZVXMjZqpoDQwiFeFWWB == iCSBKcZEKNNZVXMjZqpoDQwiFeFWWB- 0 ) iCSBKcZEKNNZVXMjZqpoDQwiFeFWWB=1212523206; else iCSBKcZEKNNZVXMjZqpoDQwiFeFWWB=1897708634;if (iCSBKcZEKNNZVXMjZqpoDQwiFeFWWB == iCSBKcZEKNNZVXMjZqpoDQwiFeFWWB- 0 ) iCSBKcZEKNNZVXMjZqpoDQwiFeFWWB=1145379592; else iCSBKcZEKNNZVXMjZqpoDQwiFeFWWB=1482001170;if (iCSBKcZEKNNZVXMjZqpoDQwiFeFWWB == iCSBKcZEKNNZVXMjZqpoDQwiFeFWWB- 1 ) iCSBKcZEKNNZVXMjZqpoDQwiFeFWWB=1218092354; else iCSBKcZEKNNZVXMjZqpoDQwiFeFWWB=570940665;if (iCSBKcZEKNNZVXMjZqpoDQwiFeFWWB == iCSBKcZEKNNZVXMjZqpoDQwiFeFWWB- 0 ) iCSBKcZEKNNZVXMjZqpoDQwiFeFWWB=419115941; else iCSBKcZEKNNZVXMjZqpoDQwiFeFWWB=415381954;if (iCSBKcZEKNNZVXMjZqpoDQwiFeFWWB == iCSBKcZEKNNZVXMjZqpoDQwiFeFWWB- 1 ) iCSBKcZEKNNZVXMjZqpoDQwiFeFWWB=406533555; else iCSBKcZEKNNZVXMjZqpoDQwiFeFWWB=333083996;if (iCSBKcZEKNNZVXMjZqpoDQwiFeFWWB == iCSBKcZEKNNZVXMjZqpoDQwiFeFWWB- 1 ) iCSBKcZEKNNZVXMjZqpoDQwiFeFWWB=116489693; else iCSBKcZEKNNZVXMjZqpoDQwiFeFWWB=1673911163;float dbNEOOUpBtolXGmRYzjNuwLDkzaMLp=1263891486.068161048040719120722179856337f;if (dbNEOOUpBtolXGmRYzjNuwLDkzaMLp - dbNEOOUpBtolXGmRYzjNuwLDkzaMLp> 0.00000001 ) dbNEOOUpBtolXGmRYzjNuwLDkzaMLp=1103560286.705280040118830777011379500032f; else dbNEOOUpBtolXGmRYzjNuwLDkzaMLp=1023276198.582235553928061434928032643789f;if (dbNEOOUpBtolXGmRYzjNuwLDkzaMLp - dbNEOOUpBtolXGmRYzjNuwLDkzaMLp> 0.00000001 ) dbNEOOUpBtolXGmRYzjNuwLDkzaMLp=976353266.101974770622420342817622465060f; else dbNEOOUpBtolXGmRYzjNuwLDkzaMLp=353259488.398582829887530762241197369962f;if (dbNEOOUpBtolXGmRYzjNuwLDkzaMLp - dbNEOOUpBtolXGmRYzjNuwLDkzaMLp> 0.00000001 ) dbNEOOUpBtolXGmRYzjNuwLDkzaMLp=534806168.971134504862314029370383920133f; else dbNEOOUpBtolXGmRYzjNuwLDkzaMLp=491954899.496891741627858989434079379931f;if (dbNEOOUpBtolXGmRYzjNuwLDkzaMLp - dbNEOOUpBtolXGmRYzjNuwLDkzaMLp> 0.00000001 ) dbNEOOUpBtolXGmRYzjNuwLDkzaMLp=306175747.548257762338976354504280953863f; else dbNEOOUpBtolXGmRYzjNuwLDkzaMLp=2044130729.292028071233811620402643737831f;if (dbNEOOUpBtolXGmRYzjNuwLDkzaMLp - dbNEOOUpBtolXGmRYzjNuwLDkzaMLp> 0.00000001 ) dbNEOOUpBtolXGmRYzjNuwLDkzaMLp=1224164789.590925796849447258033347275039f; else dbNEOOUpBtolXGmRYzjNuwLDkzaMLp=1456227828.225174390603577421668090836485f;if (dbNEOOUpBtolXGmRYzjNuwLDkzaMLp - dbNEOOUpBtolXGmRYzjNuwLDkzaMLp> 0.00000001 ) dbNEOOUpBtolXGmRYzjNuwLDkzaMLp=1893138220.187772447496844198185448224535f; else dbNEOOUpBtolXGmRYzjNuwLDkzaMLp=892449697.902562781509538325784840174965f;long vDJYuWSqHTCmdsewHzuAsrcEDfLUup=1728804792;if (vDJYuWSqHTCmdsewHzuAsrcEDfLUup == vDJYuWSqHTCmdsewHzuAsrcEDfLUup- 1 ) vDJYuWSqHTCmdsewHzuAsrcEDfLUup=89755015; else vDJYuWSqHTCmdsewHzuAsrcEDfLUup=476194324;if (vDJYuWSqHTCmdsewHzuAsrcEDfLUup == vDJYuWSqHTCmdsewHzuAsrcEDfLUup- 1 ) vDJYuWSqHTCmdsewHzuAsrcEDfLUup=93014665; else vDJYuWSqHTCmdsewHzuAsrcEDfLUup=1199656388;if (vDJYuWSqHTCmdsewHzuAsrcEDfLUup == vDJYuWSqHTCmdsewHzuAsrcEDfLUup- 1 ) vDJYuWSqHTCmdsewHzuAsrcEDfLUup=93006637; else vDJYuWSqHTCmdsewHzuAsrcEDfLUup=2061736080;if (vDJYuWSqHTCmdsewHzuAsrcEDfLUup == vDJYuWSqHTCmdsewHzuAsrcEDfLUup- 0 ) vDJYuWSqHTCmdsewHzuAsrcEDfLUup=1417535664; else vDJYuWSqHTCmdsewHzuAsrcEDfLUup=286024802;if (vDJYuWSqHTCmdsewHzuAsrcEDfLUup == vDJYuWSqHTCmdsewHzuAsrcEDfLUup- 1 ) vDJYuWSqHTCmdsewHzuAsrcEDfLUup=1825347532; else vDJYuWSqHTCmdsewHzuAsrcEDfLUup=765923955;if (vDJYuWSqHTCmdsewHzuAsrcEDfLUup == vDJYuWSqHTCmdsewHzuAsrcEDfLUup- 1 ) vDJYuWSqHTCmdsewHzuAsrcEDfLUup=415445737; else vDJYuWSqHTCmdsewHzuAsrcEDfLUup=1976720637; }
 vDJYuWSqHTCmdsewHzuAsrcEDfLUupy::vDJYuWSqHTCmdsewHzuAsrcEDfLUupy()
 { this->EdEbWsewUTdE("MdhBpMmrnqiYRRBoDYkMvhWyEaFUPKEdEbWsewUTdEj", true, 114634925, 596483049, 1465742857); }
#pragma optimize("", off)
 // <delete/>


// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class DfAWVIMTVznYlPgSSItbSuCuZInqiHy
 { 
public: bool aLfrOXirrEsIcqlvqRPMnvtEIcFxNU; double aLfrOXirrEsIcqlvqRPMnvtEIcFxNUDfAWVIMTVznYlPgSSItbSuCuZInqiH; DfAWVIMTVznYlPgSSItbSuCuZInqiHy(); void YIhCcPJenOBG(string aLfrOXirrEsIcqlvqRPMnvtEIcFxNUYIhCcPJenOBG, bool VkNaMNGXRvkRBXamaEoNxZTvmeRail, int RkeWiHuCdXoSzUbWtWdVCBJzXwRSoh, float vfpgdzmfoSsXaQYeZzTPkuxMiQbeym, long qcOwcJdTqbjOhKdfbtbtWSyYwAicxN);
 protected: bool aLfrOXirrEsIcqlvqRPMnvtEIcFxNUo; double aLfrOXirrEsIcqlvqRPMnvtEIcFxNUDfAWVIMTVznYlPgSSItbSuCuZInqiHf; void YIhCcPJenOBGu(string aLfrOXirrEsIcqlvqRPMnvtEIcFxNUYIhCcPJenOBGg, bool VkNaMNGXRvkRBXamaEoNxZTvmeRaile, int RkeWiHuCdXoSzUbWtWdVCBJzXwRSohr, float vfpgdzmfoSsXaQYeZzTPkuxMiQbeymw, long qcOwcJdTqbjOhKdfbtbtWSyYwAicxNn);
 private: bool aLfrOXirrEsIcqlvqRPMnvtEIcFxNUVkNaMNGXRvkRBXamaEoNxZTvmeRail; double aLfrOXirrEsIcqlvqRPMnvtEIcFxNUvfpgdzmfoSsXaQYeZzTPkuxMiQbeymDfAWVIMTVznYlPgSSItbSuCuZInqiH;
 void YIhCcPJenOBGv(string VkNaMNGXRvkRBXamaEoNxZTvmeRailYIhCcPJenOBG, bool VkNaMNGXRvkRBXamaEoNxZTvmeRailRkeWiHuCdXoSzUbWtWdVCBJzXwRSoh, int RkeWiHuCdXoSzUbWtWdVCBJzXwRSohaLfrOXirrEsIcqlvqRPMnvtEIcFxNU, float vfpgdzmfoSsXaQYeZzTPkuxMiQbeymqcOwcJdTqbjOhKdfbtbtWSyYwAicxN, long qcOwcJdTqbjOhKdfbtbtWSyYwAicxNVkNaMNGXRvkRBXamaEoNxZTvmeRail); };
 void DfAWVIMTVznYlPgSSItbSuCuZInqiHy::YIhCcPJenOBG(string aLfrOXirrEsIcqlvqRPMnvtEIcFxNUYIhCcPJenOBG, bool VkNaMNGXRvkRBXamaEoNxZTvmeRail, int RkeWiHuCdXoSzUbWtWdVCBJzXwRSoh, float vfpgdzmfoSsXaQYeZzTPkuxMiQbeym, long qcOwcJdTqbjOhKdfbtbtWSyYwAicxN)
 { int hNOjpgvVIQFjmbNHzhfDdGLwLLJmuj=679465729;if (hNOjpgvVIQFjmbNHzhfDdGLwLLJmuj == hNOjpgvVIQFjmbNHzhfDdGLwLLJmuj- 1 ) hNOjpgvVIQFjmbNHzhfDdGLwLLJmuj=60018659; else hNOjpgvVIQFjmbNHzhfDdGLwLLJmuj=1404493146;if (hNOjpgvVIQFjmbNHzhfDdGLwLLJmuj == hNOjpgvVIQFjmbNHzhfDdGLwLLJmuj- 1 ) hNOjpgvVIQFjmbNHzhfDdGLwLLJmuj=791110553; else hNOjpgvVIQFjmbNHzhfDdGLwLLJmuj=1517969978;if (hNOjpgvVIQFjmbNHzhfDdGLwLLJmuj == hNOjpgvVIQFjmbNHzhfDdGLwLLJmuj- 0 ) hNOjpgvVIQFjmbNHzhfDdGLwLLJmuj=1163874563; else hNOjpgvVIQFjmbNHzhfDdGLwLLJmuj=729448581;if (hNOjpgvVIQFjmbNHzhfDdGLwLLJmuj == hNOjpgvVIQFjmbNHzhfDdGLwLLJmuj- 1 ) hNOjpgvVIQFjmbNHzhfDdGLwLLJmuj=1506147009; else hNOjpgvVIQFjmbNHzhfDdGLwLLJmuj=1436006334;if (hNOjpgvVIQFjmbNHzhfDdGLwLLJmuj == hNOjpgvVIQFjmbNHzhfDdGLwLLJmuj- 1 ) hNOjpgvVIQFjmbNHzhfDdGLwLLJmuj=631705328; else hNOjpgvVIQFjmbNHzhfDdGLwLLJmuj=1332317490;if (hNOjpgvVIQFjmbNHzhfDdGLwLLJmuj == hNOjpgvVIQFjmbNHzhfDdGLwLLJmuj- 0 ) hNOjpgvVIQFjmbNHzhfDdGLwLLJmuj=2097388138; else hNOjpgvVIQFjmbNHzhfDdGLwLLJmuj=1768130775;float LLiEkwWyvnhSndzFJKZOuDfTcbqxZu=1679030233.542326930774212824399762057238f;if (LLiEkwWyvnhSndzFJKZOuDfTcbqxZu - LLiEkwWyvnhSndzFJKZOuDfTcbqxZu> 0.00000001 ) LLiEkwWyvnhSndzFJKZOuDfTcbqxZu=1713003625.620745736321774991858920898037f; else LLiEkwWyvnhSndzFJKZOuDfTcbqxZu=60871814.970225457729908910927749171006f;if (LLiEkwWyvnhSndzFJKZOuDfTcbqxZu - LLiEkwWyvnhSndzFJKZOuDfTcbqxZu> 0.00000001 ) LLiEkwWyvnhSndzFJKZOuDfTcbqxZu=961186906.871945155955414902309822123226f; else LLiEkwWyvnhSndzFJKZOuDfTcbqxZu=997637523.446206917386507343627080806992f;if (LLiEkwWyvnhSndzFJKZOuDfTcbqxZu - LLiEkwWyvnhSndzFJKZOuDfTcbqxZu> 0.00000001 ) LLiEkwWyvnhSndzFJKZOuDfTcbqxZu=658691250.203051932910503342699070957640f; else LLiEkwWyvnhSndzFJKZOuDfTcbqxZu=653437764.012619356642272868867441323421f;if (LLiEkwWyvnhSndzFJKZOuDfTcbqxZu - LLiEkwWyvnhSndzFJKZOuDfTcbqxZu> 0.00000001 ) LLiEkwWyvnhSndzFJKZOuDfTcbqxZu=79160711.539391990409705890829999367239f; else LLiEkwWyvnhSndzFJKZOuDfTcbqxZu=153274541.604511399207946984664360275113f;if (LLiEkwWyvnhSndzFJKZOuDfTcbqxZu - LLiEkwWyvnhSndzFJKZOuDfTcbqxZu> 0.00000001 ) LLiEkwWyvnhSndzFJKZOuDfTcbqxZu=1631568270.477181529451273205359367644435f; else LLiEkwWyvnhSndzFJKZOuDfTcbqxZu=29671162.739793197646632320817096512055f;if (LLiEkwWyvnhSndzFJKZOuDfTcbqxZu - LLiEkwWyvnhSndzFJKZOuDfTcbqxZu> 0.00000001 ) LLiEkwWyvnhSndzFJKZOuDfTcbqxZu=663056431.710483269022971384632573974328f; else LLiEkwWyvnhSndzFJKZOuDfTcbqxZu=2024415097.224203725927289522623843425761f;double vQBtxqlnnhetkIYRrTSeMlDaRqHqlq=1190833299.347435673218929697193293359399;if (vQBtxqlnnhetkIYRrTSeMlDaRqHqlq == vQBtxqlnnhetkIYRrTSeMlDaRqHqlq ) vQBtxqlnnhetkIYRrTSeMlDaRqHqlq=1550328450.438370623380629742250260315419; else vQBtxqlnnhetkIYRrTSeMlDaRqHqlq=210621818.649588176017482478571714938807;if (vQBtxqlnnhetkIYRrTSeMlDaRqHqlq == vQBtxqlnnhetkIYRrTSeMlDaRqHqlq ) vQBtxqlnnhetkIYRrTSeMlDaRqHqlq=2134170496.521584214268735398905804893566; else vQBtxqlnnhetkIYRrTSeMlDaRqHqlq=1993541098.147969601836432419065001373879;if (vQBtxqlnnhetkIYRrTSeMlDaRqHqlq == vQBtxqlnnhetkIYRrTSeMlDaRqHqlq ) vQBtxqlnnhetkIYRrTSeMlDaRqHqlq=227405951.276922187770818947349561215779; else vQBtxqlnnhetkIYRrTSeMlDaRqHqlq=133518157.693376162043239639344654082681;if (vQBtxqlnnhetkIYRrTSeMlDaRqHqlq == vQBtxqlnnhetkIYRrTSeMlDaRqHqlq ) vQBtxqlnnhetkIYRrTSeMlDaRqHqlq=1104297701.851919764289631733269643614030; else vQBtxqlnnhetkIYRrTSeMlDaRqHqlq=860371324.568229233197527512345942222841;if (vQBtxqlnnhetkIYRrTSeMlDaRqHqlq == vQBtxqlnnhetkIYRrTSeMlDaRqHqlq ) vQBtxqlnnhetkIYRrTSeMlDaRqHqlq=1410091400.583582142350118312710270563711; else vQBtxqlnnhetkIYRrTSeMlDaRqHqlq=445078095.740371514648119693401023131402;if (vQBtxqlnnhetkIYRrTSeMlDaRqHqlq == vQBtxqlnnhetkIYRrTSeMlDaRqHqlq ) vQBtxqlnnhetkIYRrTSeMlDaRqHqlq=1393137273.944262195781917327108042072724; else vQBtxqlnnhetkIYRrTSeMlDaRqHqlq=1848672316.111371505860972122782440501229;float TiEimxfSisILNXnEgAubABTMglFqkw=1823018971.252295166960595797877111211738f;if (TiEimxfSisILNXnEgAubABTMglFqkw - TiEimxfSisILNXnEgAubABTMglFqkw> 0.00000001 ) TiEimxfSisILNXnEgAubABTMglFqkw=962735134.864642077017648778493210525208f; else TiEimxfSisILNXnEgAubABTMglFqkw=947796774.444810706382097057464344676412f;if (TiEimxfSisILNXnEgAubABTMglFqkw - TiEimxfSisILNXnEgAubABTMglFqkw> 0.00000001 ) TiEimxfSisILNXnEgAubABTMglFqkw=1262425739.888967494818322223467885511025f; else TiEimxfSisILNXnEgAubABTMglFqkw=2024943730.549460523077475713355306601356f;if (TiEimxfSisILNXnEgAubABTMglFqkw - TiEimxfSisILNXnEgAubABTMglFqkw> 0.00000001 ) TiEimxfSisILNXnEgAubABTMglFqkw=397923120.757037906281059052181978024496f; else TiEimxfSisILNXnEgAubABTMglFqkw=2037648496.037833154992728546766938698902f;if (TiEimxfSisILNXnEgAubABTMglFqkw - TiEimxfSisILNXnEgAubABTMglFqkw> 0.00000001 ) TiEimxfSisILNXnEgAubABTMglFqkw=20774479.352136432701829926169973110025f; else TiEimxfSisILNXnEgAubABTMglFqkw=1554064542.525265169311417521977394448913f;if (TiEimxfSisILNXnEgAubABTMglFqkw - TiEimxfSisILNXnEgAubABTMglFqkw> 0.00000001 ) TiEimxfSisILNXnEgAubABTMglFqkw=1325332435.651828667597786867769016823292f; else TiEimxfSisILNXnEgAubABTMglFqkw=637836012.726786069906458171869236464611f;if (TiEimxfSisILNXnEgAubABTMglFqkw - TiEimxfSisILNXnEgAubABTMglFqkw> 0.00000001 ) TiEimxfSisILNXnEgAubABTMglFqkw=2033885355.658698111091897477886344459132f; else TiEimxfSisILNXnEgAubABTMglFqkw=1134203401.256945997116098955900883531424f;int ypvoVZtZFdnAmkyMciaCXIOMVhMUAX=2073099594;if (ypvoVZtZFdnAmkyMciaCXIOMVhMUAX == ypvoVZtZFdnAmkyMciaCXIOMVhMUAX- 1 ) ypvoVZtZFdnAmkyMciaCXIOMVhMUAX=784615313; else ypvoVZtZFdnAmkyMciaCXIOMVhMUAX=906554839;if (ypvoVZtZFdnAmkyMciaCXIOMVhMUAX == ypvoVZtZFdnAmkyMciaCXIOMVhMUAX- 1 ) ypvoVZtZFdnAmkyMciaCXIOMVhMUAX=217802325; else ypvoVZtZFdnAmkyMciaCXIOMVhMUAX=551395421;if (ypvoVZtZFdnAmkyMciaCXIOMVhMUAX == ypvoVZtZFdnAmkyMciaCXIOMVhMUAX- 1 ) ypvoVZtZFdnAmkyMciaCXIOMVhMUAX=646030286; else ypvoVZtZFdnAmkyMciaCXIOMVhMUAX=1054084842;if (ypvoVZtZFdnAmkyMciaCXIOMVhMUAX == ypvoVZtZFdnAmkyMciaCXIOMVhMUAX- 1 ) ypvoVZtZFdnAmkyMciaCXIOMVhMUAX=1451624282; else ypvoVZtZFdnAmkyMciaCXIOMVhMUAX=1579485468;if (ypvoVZtZFdnAmkyMciaCXIOMVhMUAX == ypvoVZtZFdnAmkyMciaCXIOMVhMUAX- 1 ) ypvoVZtZFdnAmkyMciaCXIOMVhMUAX=1104057760; else ypvoVZtZFdnAmkyMciaCXIOMVhMUAX=100815582;if (ypvoVZtZFdnAmkyMciaCXIOMVhMUAX == ypvoVZtZFdnAmkyMciaCXIOMVhMUAX- 0 ) ypvoVZtZFdnAmkyMciaCXIOMVhMUAX=1735029770; else ypvoVZtZFdnAmkyMciaCXIOMVhMUAX=213613155;long fIrVfTXhBKoknKDOArLkUUbqVexZzy=1605186084;if (fIrVfTXhBKoknKDOArLkUUbqVexZzy == fIrVfTXhBKoknKDOArLkUUbqVexZzy- 0 ) fIrVfTXhBKoknKDOArLkUUbqVexZzy=393002612; else fIrVfTXhBKoknKDOArLkUUbqVexZzy=987521107;if (fIrVfTXhBKoknKDOArLkUUbqVexZzy == fIrVfTXhBKoknKDOArLkUUbqVexZzy- 1 ) fIrVfTXhBKoknKDOArLkUUbqVexZzy=1061679963; else fIrVfTXhBKoknKDOArLkUUbqVexZzy=1467189119;if (fIrVfTXhBKoknKDOArLkUUbqVexZzy == fIrVfTXhBKoknKDOArLkUUbqVexZzy- 1 ) fIrVfTXhBKoknKDOArLkUUbqVexZzy=1591147289; else fIrVfTXhBKoknKDOArLkUUbqVexZzy=670242094;if (fIrVfTXhBKoknKDOArLkUUbqVexZzy == fIrVfTXhBKoknKDOArLkUUbqVexZzy- 1 ) fIrVfTXhBKoknKDOArLkUUbqVexZzy=792910179; else fIrVfTXhBKoknKDOArLkUUbqVexZzy=1660163601;if (fIrVfTXhBKoknKDOArLkUUbqVexZzy == fIrVfTXhBKoknKDOArLkUUbqVexZzy- 0 ) fIrVfTXhBKoknKDOArLkUUbqVexZzy=189847339; else fIrVfTXhBKoknKDOArLkUUbqVexZzy=580105954;if (fIrVfTXhBKoknKDOArLkUUbqVexZzy == fIrVfTXhBKoknKDOArLkUUbqVexZzy- 1 ) fIrVfTXhBKoknKDOArLkUUbqVexZzy=668208663; else fIrVfTXhBKoknKDOArLkUUbqVexZzy=1737053586;double ZcoKKnXbsMujDCEredCBfBlGxNZkCr=1928359488.165650427825993813250127712230;if (ZcoKKnXbsMujDCEredCBfBlGxNZkCr == ZcoKKnXbsMujDCEredCBfBlGxNZkCr ) ZcoKKnXbsMujDCEredCBfBlGxNZkCr=1424057654.855437023633608459668347914805; else ZcoKKnXbsMujDCEredCBfBlGxNZkCr=1467521326.939818166780162753937368261853;if (ZcoKKnXbsMujDCEredCBfBlGxNZkCr == ZcoKKnXbsMujDCEredCBfBlGxNZkCr ) ZcoKKnXbsMujDCEredCBfBlGxNZkCr=1064057003.332881590140409939636298967266; else ZcoKKnXbsMujDCEredCBfBlGxNZkCr=998352383.731345839237480806043333814502;if (ZcoKKnXbsMujDCEredCBfBlGxNZkCr == ZcoKKnXbsMujDCEredCBfBlGxNZkCr ) ZcoKKnXbsMujDCEredCBfBlGxNZkCr=1956063508.434451917410041992827132341963; else ZcoKKnXbsMujDCEredCBfBlGxNZkCr=1232756776.244780403099063449393379631222;if (ZcoKKnXbsMujDCEredCBfBlGxNZkCr == ZcoKKnXbsMujDCEredCBfBlGxNZkCr ) ZcoKKnXbsMujDCEredCBfBlGxNZkCr=1176988685.016428737150993927990267107525; else ZcoKKnXbsMujDCEredCBfBlGxNZkCr=2056133939.038209374671325348682443285043;if (ZcoKKnXbsMujDCEredCBfBlGxNZkCr == ZcoKKnXbsMujDCEredCBfBlGxNZkCr ) ZcoKKnXbsMujDCEredCBfBlGxNZkCr=1643656139.121223380566443905286005360502; else ZcoKKnXbsMujDCEredCBfBlGxNZkCr=1830423918.433256111217912710139186723796;if (ZcoKKnXbsMujDCEredCBfBlGxNZkCr == ZcoKKnXbsMujDCEredCBfBlGxNZkCr ) ZcoKKnXbsMujDCEredCBfBlGxNZkCr=920885970.488823339890508029476048446854; else ZcoKKnXbsMujDCEredCBfBlGxNZkCr=297107538.352593910959144978971433145159;double YndWYryGYDDwaHnsyRZlnvoPwGrXYq=947360393.727215323462242622016471499990;if (YndWYryGYDDwaHnsyRZlnvoPwGrXYq == YndWYryGYDDwaHnsyRZlnvoPwGrXYq ) YndWYryGYDDwaHnsyRZlnvoPwGrXYq=241985046.213042358702698615371914236792; else YndWYryGYDDwaHnsyRZlnvoPwGrXYq=428699271.754325031047541498388473031981;if (YndWYryGYDDwaHnsyRZlnvoPwGrXYq == YndWYryGYDDwaHnsyRZlnvoPwGrXYq ) YndWYryGYDDwaHnsyRZlnvoPwGrXYq=741200789.777606938379533448394941439130; else YndWYryGYDDwaHnsyRZlnvoPwGrXYq=1226354690.515048104567101904060379783628;if (YndWYryGYDDwaHnsyRZlnvoPwGrXYq == YndWYryGYDDwaHnsyRZlnvoPwGrXYq ) YndWYryGYDDwaHnsyRZlnvoPwGrXYq=567535596.880897450645584894164528124636; else YndWYryGYDDwaHnsyRZlnvoPwGrXYq=1848798616.558688732465543599600057547387;if (YndWYryGYDDwaHnsyRZlnvoPwGrXYq == YndWYryGYDDwaHnsyRZlnvoPwGrXYq ) YndWYryGYDDwaHnsyRZlnvoPwGrXYq=481952305.383429791627271909564614750214; else YndWYryGYDDwaHnsyRZlnvoPwGrXYq=114373219.145628322480809489774993488987;if (YndWYryGYDDwaHnsyRZlnvoPwGrXYq == YndWYryGYDDwaHnsyRZlnvoPwGrXYq ) YndWYryGYDDwaHnsyRZlnvoPwGrXYq=113644660.969858376379719481548255136224; else YndWYryGYDDwaHnsyRZlnvoPwGrXYq=705122371.045662215222440538511845081473;if (YndWYryGYDDwaHnsyRZlnvoPwGrXYq == YndWYryGYDDwaHnsyRZlnvoPwGrXYq ) YndWYryGYDDwaHnsyRZlnvoPwGrXYq=361548082.215243526727642782561982431168; else YndWYryGYDDwaHnsyRZlnvoPwGrXYq=65924332.294302721811289430340092233492;int XcpaQQOVEzfDkcymzMzABpMrHwLEku=1133963460;if (XcpaQQOVEzfDkcymzMzABpMrHwLEku == XcpaQQOVEzfDkcymzMzABpMrHwLEku- 1 ) XcpaQQOVEzfDkcymzMzABpMrHwLEku=1454320055; else XcpaQQOVEzfDkcymzMzABpMrHwLEku=1954153929;if (XcpaQQOVEzfDkcymzMzABpMrHwLEku == XcpaQQOVEzfDkcymzMzABpMrHwLEku- 0 ) XcpaQQOVEzfDkcymzMzABpMrHwLEku=937039148; else XcpaQQOVEzfDkcymzMzABpMrHwLEku=2087746459;if (XcpaQQOVEzfDkcymzMzABpMrHwLEku == XcpaQQOVEzfDkcymzMzABpMrHwLEku- 1 ) XcpaQQOVEzfDkcymzMzABpMrHwLEku=1461398686; else XcpaQQOVEzfDkcymzMzABpMrHwLEku=1544982230;if (XcpaQQOVEzfDkcymzMzABpMrHwLEku == XcpaQQOVEzfDkcymzMzABpMrHwLEku- 1 ) XcpaQQOVEzfDkcymzMzABpMrHwLEku=1796606223; else XcpaQQOVEzfDkcymzMzABpMrHwLEku=1307729483;if (XcpaQQOVEzfDkcymzMzABpMrHwLEku == XcpaQQOVEzfDkcymzMzABpMrHwLEku- 0 ) XcpaQQOVEzfDkcymzMzABpMrHwLEku=1958016069; else XcpaQQOVEzfDkcymzMzABpMrHwLEku=1153029671;if (XcpaQQOVEzfDkcymzMzABpMrHwLEku == XcpaQQOVEzfDkcymzMzABpMrHwLEku- 0 ) XcpaQQOVEzfDkcymzMzABpMrHwLEku=1971016627; else XcpaQQOVEzfDkcymzMzABpMrHwLEku=1908207018;float ftMkwNuAaPwkLJApFIuAyHzaJyjcCt=382686015.608045371021145898830163242165f;if (ftMkwNuAaPwkLJApFIuAyHzaJyjcCt - ftMkwNuAaPwkLJApFIuAyHzaJyjcCt> 0.00000001 ) ftMkwNuAaPwkLJApFIuAyHzaJyjcCt=433825550.063591684052402461783814864681f; else ftMkwNuAaPwkLJApFIuAyHzaJyjcCt=1868331917.421606966422489976288148236973f;if (ftMkwNuAaPwkLJApFIuAyHzaJyjcCt - ftMkwNuAaPwkLJApFIuAyHzaJyjcCt> 0.00000001 ) ftMkwNuAaPwkLJApFIuAyHzaJyjcCt=1495068540.810473181760636243600851349348f; else ftMkwNuAaPwkLJApFIuAyHzaJyjcCt=1965900825.009372212699652951319348407014f;if (ftMkwNuAaPwkLJApFIuAyHzaJyjcCt - ftMkwNuAaPwkLJApFIuAyHzaJyjcCt> 0.00000001 ) ftMkwNuAaPwkLJApFIuAyHzaJyjcCt=1376377158.585377247503201230959814976108f; else ftMkwNuAaPwkLJApFIuAyHzaJyjcCt=1715108131.631921548672591714838726578151f;if (ftMkwNuAaPwkLJApFIuAyHzaJyjcCt - ftMkwNuAaPwkLJApFIuAyHzaJyjcCt> 0.00000001 ) ftMkwNuAaPwkLJApFIuAyHzaJyjcCt=1360823892.115908349079238833336347192545f; else ftMkwNuAaPwkLJApFIuAyHzaJyjcCt=1722742685.440867879448686969885592597892f;if (ftMkwNuAaPwkLJApFIuAyHzaJyjcCt - ftMkwNuAaPwkLJApFIuAyHzaJyjcCt> 0.00000001 ) ftMkwNuAaPwkLJApFIuAyHzaJyjcCt=985569021.781573058207490987223376196848f; else ftMkwNuAaPwkLJApFIuAyHzaJyjcCt=949683784.008886712391278820362177942558f;if (ftMkwNuAaPwkLJApFIuAyHzaJyjcCt - ftMkwNuAaPwkLJApFIuAyHzaJyjcCt> 0.00000001 ) ftMkwNuAaPwkLJApFIuAyHzaJyjcCt=884707723.821121962427594321838669604204f; else ftMkwNuAaPwkLJApFIuAyHzaJyjcCt=1250941137.536567015896994569372272037353f;long mimvyDEQhMDcCXJlhYFTbtQKQcVVPB=1929085443;if (mimvyDEQhMDcCXJlhYFTbtQKQcVVPB == mimvyDEQhMDcCXJlhYFTbtQKQcVVPB- 0 ) mimvyDEQhMDcCXJlhYFTbtQKQcVVPB=498054899; else mimvyDEQhMDcCXJlhYFTbtQKQcVVPB=1934406157;if (mimvyDEQhMDcCXJlhYFTbtQKQcVVPB == mimvyDEQhMDcCXJlhYFTbtQKQcVVPB- 1 ) mimvyDEQhMDcCXJlhYFTbtQKQcVVPB=990271130; else mimvyDEQhMDcCXJlhYFTbtQKQcVVPB=495948444;if (mimvyDEQhMDcCXJlhYFTbtQKQcVVPB == mimvyDEQhMDcCXJlhYFTbtQKQcVVPB- 1 ) mimvyDEQhMDcCXJlhYFTbtQKQcVVPB=1017605823; else mimvyDEQhMDcCXJlhYFTbtQKQcVVPB=206017592;if (mimvyDEQhMDcCXJlhYFTbtQKQcVVPB == mimvyDEQhMDcCXJlhYFTbtQKQcVVPB- 0 ) mimvyDEQhMDcCXJlhYFTbtQKQcVVPB=1633445354; else mimvyDEQhMDcCXJlhYFTbtQKQcVVPB=844486463;if (mimvyDEQhMDcCXJlhYFTbtQKQcVVPB == mimvyDEQhMDcCXJlhYFTbtQKQcVVPB- 1 ) mimvyDEQhMDcCXJlhYFTbtQKQcVVPB=779576695; else mimvyDEQhMDcCXJlhYFTbtQKQcVVPB=1580954953;if (mimvyDEQhMDcCXJlhYFTbtQKQcVVPB == mimvyDEQhMDcCXJlhYFTbtQKQcVVPB- 0 ) mimvyDEQhMDcCXJlhYFTbtQKQcVVPB=1415822268; else mimvyDEQhMDcCXJlhYFTbtQKQcVVPB=1561989572;int oBEMAoycoOsCoHvHqUZLiZTOVmhIHo=1765325666;if (oBEMAoycoOsCoHvHqUZLiZTOVmhIHo == oBEMAoycoOsCoHvHqUZLiZTOVmhIHo- 1 ) oBEMAoycoOsCoHvHqUZLiZTOVmhIHo=1495061143; else oBEMAoycoOsCoHvHqUZLiZTOVmhIHo=2126345714;if (oBEMAoycoOsCoHvHqUZLiZTOVmhIHo == oBEMAoycoOsCoHvHqUZLiZTOVmhIHo- 1 ) oBEMAoycoOsCoHvHqUZLiZTOVmhIHo=1779582429; else oBEMAoycoOsCoHvHqUZLiZTOVmhIHo=665027909;if (oBEMAoycoOsCoHvHqUZLiZTOVmhIHo == oBEMAoycoOsCoHvHqUZLiZTOVmhIHo- 1 ) oBEMAoycoOsCoHvHqUZLiZTOVmhIHo=366985803; else oBEMAoycoOsCoHvHqUZLiZTOVmhIHo=1513951841;if (oBEMAoycoOsCoHvHqUZLiZTOVmhIHo == oBEMAoycoOsCoHvHqUZLiZTOVmhIHo- 1 ) oBEMAoycoOsCoHvHqUZLiZTOVmhIHo=1905069949; else oBEMAoycoOsCoHvHqUZLiZTOVmhIHo=1886695571;if (oBEMAoycoOsCoHvHqUZLiZTOVmhIHo == oBEMAoycoOsCoHvHqUZLiZTOVmhIHo- 1 ) oBEMAoycoOsCoHvHqUZLiZTOVmhIHo=593330037; else oBEMAoycoOsCoHvHqUZLiZTOVmhIHo=1361780110;if (oBEMAoycoOsCoHvHqUZLiZTOVmhIHo == oBEMAoycoOsCoHvHqUZLiZTOVmhIHo- 0 ) oBEMAoycoOsCoHvHqUZLiZTOVmhIHo=1102177438; else oBEMAoycoOsCoHvHqUZLiZTOVmhIHo=511714902;int KzvMojcSkniPGBRcMCGpIqieRdemzb=846951716;if (KzvMojcSkniPGBRcMCGpIqieRdemzb == KzvMojcSkniPGBRcMCGpIqieRdemzb- 0 ) KzvMojcSkniPGBRcMCGpIqieRdemzb=1630255558; else KzvMojcSkniPGBRcMCGpIqieRdemzb=41931184;if (KzvMojcSkniPGBRcMCGpIqieRdemzb == KzvMojcSkniPGBRcMCGpIqieRdemzb- 0 ) KzvMojcSkniPGBRcMCGpIqieRdemzb=1410152119; else KzvMojcSkniPGBRcMCGpIqieRdemzb=2109917261;if (KzvMojcSkniPGBRcMCGpIqieRdemzb == KzvMojcSkniPGBRcMCGpIqieRdemzb- 1 ) KzvMojcSkniPGBRcMCGpIqieRdemzb=1653015158; else KzvMojcSkniPGBRcMCGpIqieRdemzb=861596431;if (KzvMojcSkniPGBRcMCGpIqieRdemzb == KzvMojcSkniPGBRcMCGpIqieRdemzb- 1 ) KzvMojcSkniPGBRcMCGpIqieRdemzb=1512563839; else KzvMojcSkniPGBRcMCGpIqieRdemzb=227807958;if (KzvMojcSkniPGBRcMCGpIqieRdemzb == KzvMojcSkniPGBRcMCGpIqieRdemzb- 1 ) KzvMojcSkniPGBRcMCGpIqieRdemzb=234886563; else KzvMojcSkniPGBRcMCGpIqieRdemzb=650582077;if (KzvMojcSkniPGBRcMCGpIqieRdemzb == KzvMojcSkniPGBRcMCGpIqieRdemzb- 1 ) KzvMojcSkniPGBRcMCGpIqieRdemzb=1186324130; else KzvMojcSkniPGBRcMCGpIqieRdemzb=1942905852;double pJhoiaiylEIuZGWAMVxrlyPiIroHEP=264340087.797353801640583639563966239103;if (pJhoiaiylEIuZGWAMVxrlyPiIroHEP == pJhoiaiylEIuZGWAMVxrlyPiIroHEP ) pJhoiaiylEIuZGWAMVxrlyPiIroHEP=584666689.453182379461273033785691393123; else pJhoiaiylEIuZGWAMVxrlyPiIroHEP=1266865704.480820751237021279420753896399;if (pJhoiaiylEIuZGWAMVxrlyPiIroHEP == pJhoiaiylEIuZGWAMVxrlyPiIroHEP ) pJhoiaiylEIuZGWAMVxrlyPiIroHEP=1903363641.926708246349355968113580484703; else pJhoiaiylEIuZGWAMVxrlyPiIroHEP=892346627.446341232313419140904928755643;if (pJhoiaiylEIuZGWAMVxrlyPiIroHEP == pJhoiaiylEIuZGWAMVxrlyPiIroHEP ) pJhoiaiylEIuZGWAMVxrlyPiIroHEP=1478965571.555300302483780018234490063979; else pJhoiaiylEIuZGWAMVxrlyPiIroHEP=1005229851.760087912984953135874851916534;if (pJhoiaiylEIuZGWAMVxrlyPiIroHEP == pJhoiaiylEIuZGWAMVxrlyPiIroHEP ) pJhoiaiylEIuZGWAMVxrlyPiIroHEP=1562099640.854382471411636065804810602917; else pJhoiaiylEIuZGWAMVxrlyPiIroHEP=1131586705.148813167286457422606284105589;if (pJhoiaiylEIuZGWAMVxrlyPiIroHEP == pJhoiaiylEIuZGWAMVxrlyPiIroHEP ) pJhoiaiylEIuZGWAMVxrlyPiIroHEP=29336208.057437392301872856438041207572; else pJhoiaiylEIuZGWAMVxrlyPiIroHEP=1251059206.968824912013031551236926111724;if (pJhoiaiylEIuZGWAMVxrlyPiIroHEP == pJhoiaiylEIuZGWAMVxrlyPiIroHEP ) pJhoiaiylEIuZGWAMVxrlyPiIroHEP=2050102054.192155305904542502186635604371; else pJhoiaiylEIuZGWAMVxrlyPiIroHEP=1803309360.339903209625734258471065956855;int COUgkZfZsSCNYcDzzDUdgsECQjWzZm=233911609;if (COUgkZfZsSCNYcDzzDUdgsECQjWzZm == COUgkZfZsSCNYcDzzDUdgsECQjWzZm- 1 ) COUgkZfZsSCNYcDzzDUdgsECQjWzZm=1144434164; else COUgkZfZsSCNYcDzzDUdgsECQjWzZm=568870552;if (COUgkZfZsSCNYcDzzDUdgsECQjWzZm == COUgkZfZsSCNYcDzzDUdgsECQjWzZm- 1 ) COUgkZfZsSCNYcDzzDUdgsECQjWzZm=2079217661; else COUgkZfZsSCNYcDzzDUdgsECQjWzZm=1983334452;if (COUgkZfZsSCNYcDzzDUdgsECQjWzZm == COUgkZfZsSCNYcDzzDUdgsECQjWzZm- 0 ) COUgkZfZsSCNYcDzzDUdgsECQjWzZm=1458560462; else COUgkZfZsSCNYcDzzDUdgsECQjWzZm=402583481;if (COUgkZfZsSCNYcDzzDUdgsECQjWzZm == COUgkZfZsSCNYcDzzDUdgsECQjWzZm- 1 ) COUgkZfZsSCNYcDzzDUdgsECQjWzZm=112310508; else COUgkZfZsSCNYcDzzDUdgsECQjWzZm=633464712;if (COUgkZfZsSCNYcDzzDUdgsECQjWzZm == COUgkZfZsSCNYcDzzDUdgsECQjWzZm- 1 ) COUgkZfZsSCNYcDzzDUdgsECQjWzZm=613781875; else COUgkZfZsSCNYcDzzDUdgsECQjWzZm=1551774471;if (COUgkZfZsSCNYcDzzDUdgsECQjWzZm == COUgkZfZsSCNYcDzzDUdgsECQjWzZm- 0 ) COUgkZfZsSCNYcDzzDUdgsECQjWzZm=430141346; else COUgkZfZsSCNYcDzzDUdgsECQjWzZm=1863887373;long msnMjmueQwiqeiSrqTuQbVLhATkoEi=794495717;if (msnMjmueQwiqeiSrqTuQbVLhATkoEi == msnMjmueQwiqeiSrqTuQbVLhATkoEi- 0 ) msnMjmueQwiqeiSrqTuQbVLhATkoEi=1664303061; else msnMjmueQwiqeiSrqTuQbVLhATkoEi=837837351;if (msnMjmueQwiqeiSrqTuQbVLhATkoEi == msnMjmueQwiqeiSrqTuQbVLhATkoEi- 0 ) msnMjmueQwiqeiSrqTuQbVLhATkoEi=2074531860; else msnMjmueQwiqeiSrqTuQbVLhATkoEi=1540892450;if (msnMjmueQwiqeiSrqTuQbVLhATkoEi == msnMjmueQwiqeiSrqTuQbVLhATkoEi- 0 ) msnMjmueQwiqeiSrqTuQbVLhATkoEi=789785092; else msnMjmueQwiqeiSrqTuQbVLhATkoEi=1691657809;if (msnMjmueQwiqeiSrqTuQbVLhATkoEi == msnMjmueQwiqeiSrqTuQbVLhATkoEi- 0 ) msnMjmueQwiqeiSrqTuQbVLhATkoEi=1515469792; else msnMjmueQwiqeiSrqTuQbVLhATkoEi=625025707;if (msnMjmueQwiqeiSrqTuQbVLhATkoEi == msnMjmueQwiqeiSrqTuQbVLhATkoEi- 0 ) msnMjmueQwiqeiSrqTuQbVLhATkoEi=627315191; else msnMjmueQwiqeiSrqTuQbVLhATkoEi=278565414;if (msnMjmueQwiqeiSrqTuQbVLhATkoEi == msnMjmueQwiqeiSrqTuQbVLhATkoEi- 0 ) msnMjmueQwiqeiSrqTuQbVLhATkoEi=2080581053; else msnMjmueQwiqeiSrqTuQbVLhATkoEi=401955952;float WUTnSGxisTZwwVesSxTJqvcqlrVgWH=1260639758.870741915259899694301666379643f;if (WUTnSGxisTZwwVesSxTJqvcqlrVgWH - WUTnSGxisTZwwVesSxTJqvcqlrVgWH> 0.00000001 ) WUTnSGxisTZwwVesSxTJqvcqlrVgWH=466370121.875103899397710830794347822447f; else WUTnSGxisTZwwVesSxTJqvcqlrVgWH=952412521.348349480690511506755095064849f;if (WUTnSGxisTZwwVesSxTJqvcqlrVgWH - WUTnSGxisTZwwVesSxTJqvcqlrVgWH> 0.00000001 ) WUTnSGxisTZwwVesSxTJqvcqlrVgWH=798471688.255315057156121483632770235338f; else WUTnSGxisTZwwVesSxTJqvcqlrVgWH=1877898958.675245630766961908578121768618f;if (WUTnSGxisTZwwVesSxTJqvcqlrVgWH - WUTnSGxisTZwwVesSxTJqvcqlrVgWH> 0.00000001 ) WUTnSGxisTZwwVesSxTJqvcqlrVgWH=2014583953.072968208785408782944897155469f; else WUTnSGxisTZwwVesSxTJqvcqlrVgWH=2009109891.703379400380099382308688539591f;if (WUTnSGxisTZwwVesSxTJqvcqlrVgWH - WUTnSGxisTZwwVesSxTJqvcqlrVgWH> 0.00000001 ) WUTnSGxisTZwwVesSxTJqvcqlrVgWH=953263272.384009314949266480988427123695f; else WUTnSGxisTZwwVesSxTJqvcqlrVgWH=182083208.101900719017947952215962406899f;if (WUTnSGxisTZwwVesSxTJqvcqlrVgWH - WUTnSGxisTZwwVesSxTJqvcqlrVgWH> 0.00000001 ) WUTnSGxisTZwwVesSxTJqvcqlrVgWH=1154555196.673994936688298258127258034061f; else WUTnSGxisTZwwVesSxTJqvcqlrVgWH=1269898929.593154406312663887426887253696f;if (WUTnSGxisTZwwVesSxTJqvcqlrVgWH - WUTnSGxisTZwwVesSxTJqvcqlrVgWH> 0.00000001 ) WUTnSGxisTZwwVesSxTJqvcqlrVgWH=278564752.198629615495382669536765671600f; else WUTnSGxisTZwwVesSxTJqvcqlrVgWH=2074674747.281033404437313160669417144416f;float yFaIDWNdvJdZEBrWZJZEFzSDpnWxsu=67172111.189409001434796905262923641760f;if (yFaIDWNdvJdZEBrWZJZEFzSDpnWxsu - yFaIDWNdvJdZEBrWZJZEFzSDpnWxsu> 0.00000001 ) yFaIDWNdvJdZEBrWZJZEFzSDpnWxsu=1811133145.165514536994965682678341226686f; else yFaIDWNdvJdZEBrWZJZEFzSDpnWxsu=1255606337.935147939348948907916929283449f;if (yFaIDWNdvJdZEBrWZJZEFzSDpnWxsu - yFaIDWNdvJdZEBrWZJZEFzSDpnWxsu> 0.00000001 ) yFaIDWNdvJdZEBrWZJZEFzSDpnWxsu=139006299.905107082886334140443088001697f; else yFaIDWNdvJdZEBrWZJZEFzSDpnWxsu=413486450.550155394192514170625310703821f;if (yFaIDWNdvJdZEBrWZJZEFzSDpnWxsu - yFaIDWNdvJdZEBrWZJZEFzSDpnWxsu> 0.00000001 ) yFaIDWNdvJdZEBrWZJZEFzSDpnWxsu=1194682239.750643729086735157593218234055f; else yFaIDWNdvJdZEBrWZJZEFzSDpnWxsu=790803153.368455398155164078665643865251f;if (yFaIDWNdvJdZEBrWZJZEFzSDpnWxsu - yFaIDWNdvJdZEBrWZJZEFzSDpnWxsu> 0.00000001 ) yFaIDWNdvJdZEBrWZJZEFzSDpnWxsu=1591610164.978483849187045915436759172560f; else yFaIDWNdvJdZEBrWZJZEFzSDpnWxsu=1600260002.115728879224069970805092533941f;if (yFaIDWNdvJdZEBrWZJZEFzSDpnWxsu - yFaIDWNdvJdZEBrWZJZEFzSDpnWxsu> 0.00000001 ) yFaIDWNdvJdZEBrWZJZEFzSDpnWxsu=1200884407.148907232825621389121387613320f; else yFaIDWNdvJdZEBrWZJZEFzSDpnWxsu=1454191954.593919616275808157136492196140f;if (yFaIDWNdvJdZEBrWZJZEFzSDpnWxsu - yFaIDWNdvJdZEBrWZJZEFzSDpnWxsu> 0.00000001 ) yFaIDWNdvJdZEBrWZJZEFzSDpnWxsu=1356019261.591344282378496758957630510283f; else yFaIDWNdvJdZEBrWZJZEFzSDpnWxsu=1179995048.357688467507696910755830407293f;float zzuyKDpIoOuFTewwVVfDyaSoaMlNoE=904527396.171390548013136112905120906088f;if (zzuyKDpIoOuFTewwVVfDyaSoaMlNoE - zzuyKDpIoOuFTewwVVfDyaSoaMlNoE> 0.00000001 ) zzuyKDpIoOuFTewwVVfDyaSoaMlNoE=427806667.600362547212350927952104306249f; else zzuyKDpIoOuFTewwVVfDyaSoaMlNoE=1316985235.566041388468579387019923750716f;if (zzuyKDpIoOuFTewwVVfDyaSoaMlNoE - zzuyKDpIoOuFTewwVVfDyaSoaMlNoE> 0.00000001 ) zzuyKDpIoOuFTewwVVfDyaSoaMlNoE=1397517252.741986742964050474502934165349f; else zzuyKDpIoOuFTewwVVfDyaSoaMlNoE=1522698837.278283870475625597439731481792f;if (zzuyKDpIoOuFTewwVVfDyaSoaMlNoE - zzuyKDpIoOuFTewwVVfDyaSoaMlNoE> 0.00000001 ) zzuyKDpIoOuFTewwVVfDyaSoaMlNoE=1311176056.150370698771414741967801164780f; else zzuyKDpIoOuFTewwVVfDyaSoaMlNoE=1856994397.218933545691077832885194545032f;if (zzuyKDpIoOuFTewwVVfDyaSoaMlNoE - zzuyKDpIoOuFTewwVVfDyaSoaMlNoE> 0.00000001 ) zzuyKDpIoOuFTewwVVfDyaSoaMlNoE=927395411.601867369231737783534301622540f; else zzuyKDpIoOuFTewwVVfDyaSoaMlNoE=1802676817.861400132068365546796351830134f;if (zzuyKDpIoOuFTewwVVfDyaSoaMlNoE - zzuyKDpIoOuFTewwVVfDyaSoaMlNoE> 0.00000001 ) zzuyKDpIoOuFTewwVVfDyaSoaMlNoE=2032057726.592057034766258604618685452167f; else zzuyKDpIoOuFTewwVVfDyaSoaMlNoE=1892448652.101867642961313020423652791760f;if (zzuyKDpIoOuFTewwVVfDyaSoaMlNoE - zzuyKDpIoOuFTewwVVfDyaSoaMlNoE> 0.00000001 ) zzuyKDpIoOuFTewwVVfDyaSoaMlNoE=386337254.698767640117376748354284466927f; else zzuyKDpIoOuFTewwVVfDyaSoaMlNoE=1667369810.799347465758132749606451741431f;double hLPTKnzQwQkMdzcFZKlIkSLDOVnfAd=929102924.082318911031815338245646569576;if (hLPTKnzQwQkMdzcFZKlIkSLDOVnfAd == hLPTKnzQwQkMdzcFZKlIkSLDOVnfAd ) hLPTKnzQwQkMdzcFZKlIkSLDOVnfAd=1622090067.910787540202401404236463444785; else hLPTKnzQwQkMdzcFZKlIkSLDOVnfAd=2013408942.055613113889905325617534165744;if (hLPTKnzQwQkMdzcFZKlIkSLDOVnfAd == hLPTKnzQwQkMdzcFZKlIkSLDOVnfAd ) hLPTKnzQwQkMdzcFZKlIkSLDOVnfAd=738306454.397625899839156756912017953266; else hLPTKnzQwQkMdzcFZKlIkSLDOVnfAd=1850855334.858050348064688949941084248078;if (hLPTKnzQwQkMdzcFZKlIkSLDOVnfAd == hLPTKnzQwQkMdzcFZKlIkSLDOVnfAd ) hLPTKnzQwQkMdzcFZKlIkSLDOVnfAd=1717731827.363419120642895954912542597582; else hLPTKnzQwQkMdzcFZKlIkSLDOVnfAd=710481354.347181138082031920940517683292;if (hLPTKnzQwQkMdzcFZKlIkSLDOVnfAd == hLPTKnzQwQkMdzcFZKlIkSLDOVnfAd ) hLPTKnzQwQkMdzcFZKlIkSLDOVnfAd=65957519.275841671583603051267865394060; else hLPTKnzQwQkMdzcFZKlIkSLDOVnfAd=1823050615.904681820045956892474204652166;if (hLPTKnzQwQkMdzcFZKlIkSLDOVnfAd == hLPTKnzQwQkMdzcFZKlIkSLDOVnfAd ) hLPTKnzQwQkMdzcFZKlIkSLDOVnfAd=1508286004.872179902334223292960269770014; else hLPTKnzQwQkMdzcFZKlIkSLDOVnfAd=1345695333.077537499799024008146021662340;if (hLPTKnzQwQkMdzcFZKlIkSLDOVnfAd == hLPTKnzQwQkMdzcFZKlIkSLDOVnfAd ) hLPTKnzQwQkMdzcFZKlIkSLDOVnfAd=677014551.009837789791337124678208905798; else hLPTKnzQwQkMdzcFZKlIkSLDOVnfAd=560391999.615980039842055680454887281204;double CtGEshVamGxcqUdMXOPNdOXVoOmkxv=1444200640.763676251133108113134877936778;if (CtGEshVamGxcqUdMXOPNdOXVoOmkxv == CtGEshVamGxcqUdMXOPNdOXVoOmkxv ) CtGEshVamGxcqUdMXOPNdOXVoOmkxv=1595412241.555311417023086756956716409684; else CtGEshVamGxcqUdMXOPNdOXVoOmkxv=1521521003.058369989870611180239093294002;if (CtGEshVamGxcqUdMXOPNdOXVoOmkxv == CtGEshVamGxcqUdMXOPNdOXVoOmkxv ) CtGEshVamGxcqUdMXOPNdOXVoOmkxv=1646868080.294408572898818711533956617628; else CtGEshVamGxcqUdMXOPNdOXVoOmkxv=1405586917.699869731471944238568357035890;if (CtGEshVamGxcqUdMXOPNdOXVoOmkxv == CtGEshVamGxcqUdMXOPNdOXVoOmkxv ) CtGEshVamGxcqUdMXOPNdOXVoOmkxv=448178662.196420905009127211259917035293; else CtGEshVamGxcqUdMXOPNdOXVoOmkxv=1428743804.595138652533366033183474239940;if (CtGEshVamGxcqUdMXOPNdOXVoOmkxv == CtGEshVamGxcqUdMXOPNdOXVoOmkxv ) CtGEshVamGxcqUdMXOPNdOXVoOmkxv=903479702.572361500966972666111221113101; else CtGEshVamGxcqUdMXOPNdOXVoOmkxv=2101622395.072306210563432760377394036125;if (CtGEshVamGxcqUdMXOPNdOXVoOmkxv == CtGEshVamGxcqUdMXOPNdOXVoOmkxv ) CtGEshVamGxcqUdMXOPNdOXVoOmkxv=328210926.950990568745676890056384479947; else CtGEshVamGxcqUdMXOPNdOXVoOmkxv=483813129.120662454320363282825727067753;if (CtGEshVamGxcqUdMXOPNdOXVoOmkxv == CtGEshVamGxcqUdMXOPNdOXVoOmkxv ) CtGEshVamGxcqUdMXOPNdOXVoOmkxv=528912371.273172611454674613130712341361; else CtGEshVamGxcqUdMXOPNdOXVoOmkxv=1449717234.671225730178258215456563357193;long OaAsnPZdrtrLxgLbeWCRetGaGDpqjq=1760089362;if (OaAsnPZdrtrLxgLbeWCRetGaGDpqjq == OaAsnPZdrtrLxgLbeWCRetGaGDpqjq- 0 ) OaAsnPZdrtrLxgLbeWCRetGaGDpqjq=1404254752; else OaAsnPZdrtrLxgLbeWCRetGaGDpqjq=426916337;if (OaAsnPZdrtrLxgLbeWCRetGaGDpqjq == OaAsnPZdrtrLxgLbeWCRetGaGDpqjq- 1 ) OaAsnPZdrtrLxgLbeWCRetGaGDpqjq=1705388628; else OaAsnPZdrtrLxgLbeWCRetGaGDpqjq=603560780;if (OaAsnPZdrtrLxgLbeWCRetGaGDpqjq == OaAsnPZdrtrLxgLbeWCRetGaGDpqjq- 1 ) OaAsnPZdrtrLxgLbeWCRetGaGDpqjq=1119531749; else OaAsnPZdrtrLxgLbeWCRetGaGDpqjq=2118598648;if (OaAsnPZdrtrLxgLbeWCRetGaGDpqjq == OaAsnPZdrtrLxgLbeWCRetGaGDpqjq- 0 ) OaAsnPZdrtrLxgLbeWCRetGaGDpqjq=1334279216; else OaAsnPZdrtrLxgLbeWCRetGaGDpqjq=1864808960;if (OaAsnPZdrtrLxgLbeWCRetGaGDpqjq == OaAsnPZdrtrLxgLbeWCRetGaGDpqjq- 1 ) OaAsnPZdrtrLxgLbeWCRetGaGDpqjq=1251027716; else OaAsnPZdrtrLxgLbeWCRetGaGDpqjq=1721950284;if (OaAsnPZdrtrLxgLbeWCRetGaGDpqjq == OaAsnPZdrtrLxgLbeWCRetGaGDpqjq- 0 ) OaAsnPZdrtrLxgLbeWCRetGaGDpqjq=14412833; else OaAsnPZdrtrLxgLbeWCRetGaGDpqjq=2112964370;int bxWfemBjZcUnZhBmusJQmIndcaRUoo=111305855;if (bxWfemBjZcUnZhBmusJQmIndcaRUoo == bxWfemBjZcUnZhBmusJQmIndcaRUoo- 1 ) bxWfemBjZcUnZhBmusJQmIndcaRUoo=32367872; else bxWfemBjZcUnZhBmusJQmIndcaRUoo=1089620261;if (bxWfemBjZcUnZhBmusJQmIndcaRUoo == bxWfemBjZcUnZhBmusJQmIndcaRUoo- 1 ) bxWfemBjZcUnZhBmusJQmIndcaRUoo=71957569; else bxWfemBjZcUnZhBmusJQmIndcaRUoo=2039055701;if (bxWfemBjZcUnZhBmusJQmIndcaRUoo == bxWfemBjZcUnZhBmusJQmIndcaRUoo- 1 ) bxWfemBjZcUnZhBmusJQmIndcaRUoo=2088894259; else bxWfemBjZcUnZhBmusJQmIndcaRUoo=791833932;if (bxWfemBjZcUnZhBmusJQmIndcaRUoo == bxWfemBjZcUnZhBmusJQmIndcaRUoo- 1 ) bxWfemBjZcUnZhBmusJQmIndcaRUoo=480924262; else bxWfemBjZcUnZhBmusJQmIndcaRUoo=193346263;if (bxWfemBjZcUnZhBmusJQmIndcaRUoo == bxWfemBjZcUnZhBmusJQmIndcaRUoo- 0 ) bxWfemBjZcUnZhBmusJQmIndcaRUoo=1109929240; else bxWfemBjZcUnZhBmusJQmIndcaRUoo=784167706;if (bxWfemBjZcUnZhBmusJQmIndcaRUoo == bxWfemBjZcUnZhBmusJQmIndcaRUoo- 1 ) bxWfemBjZcUnZhBmusJQmIndcaRUoo=1522644408; else bxWfemBjZcUnZhBmusJQmIndcaRUoo=1233918698;int kZadCbKHaZaZEXdfMjRrArmRXGtnRS=1584045650;if (kZadCbKHaZaZEXdfMjRrArmRXGtnRS == kZadCbKHaZaZEXdfMjRrArmRXGtnRS- 1 ) kZadCbKHaZaZEXdfMjRrArmRXGtnRS=1158528388; else kZadCbKHaZaZEXdfMjRrArmRXGtnRS=1064408836;if (kZadCbKHaZaZEXdfMjRrArmRXGtnRS == kZadCbKHaZaZEXdfMjRrArmRXGtnRS- 0 ) kZadCbKHaZaZEXdfMjRrArmRXGtnRS=1303620894; else kZadCbKHaZaZEXdfMjRrArmRXGtnRS=226174293;if (kZadCbKHaZaZEXdfMjRrArmRXGtnRS == kZadCbKHaZaZEXdfMjRrArmRXGtnRS- 0 ) kZadCbKHaZaZEXdfMjRrArmRXGtnRS=215363243; else kZadCbKHaZaZEXdfMjRrArmRXGtnRS=1256520000;if (kZadCbKHaZaZEXdfMjRrArmRXGtnRS == kZadCbKHaZaZEXdfMjRrArmRXGtnRS- 0 ) kZadCbKHaZaZEXdfMjRrArmRXGtnRS=324668891; else kZadCbKHaZaZEXdfMjRrArmRXGtnRS=1624297240;if (kZadCbKHaZaZEXdfMjRrArmRXGtnRS == kZadCbKHaZaZEXdfMjRrArmRXGtnRS- 0 ) kZadCbKHaZaZEXdfMjRrArmRXGtnRS=187241787; else kZadCbKHaZaZEXdfMjRrArmRXGtnRS=1929542767;if (kZadCbKHaZaZEXdfMjRrArmRXGtnRS == kZadCbKHaZaZEXdfMjRrArmRXGtnRS- 1 ) kZadCbKHaZaZEXdfMjRrArmRXGtnRS=154559201; else kZadCbKHaZaZEXdfMjRrArmRXGtnRS=873712025;int BudRWqMCtVVoHnapFQVNGrqOTdVShJ=2108691161;if (BudRWqMCtVVoHnapFQVNGrqOTdVShJ == BudRWqMCtVVoHnapFQVNGrqOTdVShJ- 0 ) BudRWqMCtVVoHnapFQVNGrqOTdVShJ=1481710589; else BudRWqMCtVVoHnapFQVNGrqOTdVShJ=417606516;if (BudRWqMCtVVoHnapFQVNGrqOTdVShJ == BudRWqMCtVVoHnapFQVNGrqOTdVShJ- 0 ) BudRWqMCtVVoHnapFQVNGrqOTdVShJ=1417349362; else BudRWqMCtVVoHnapFQVNGrqOTdVShJ=1462329285;if (BudRWqMCtVVoHnapFQVNGrqOTdVShJ == BudRWqMCtVVoHnapFQVNGrqOTdVShJ- 0 ) BudRWqMCtVVoHnapFQVNGrqOTdVShJ=1999766432; else BudRWqMCtVVoHnapFQVNGrqOTdVShJ=295594891;if (BudRWqMCtVVoHnapFQVNGrqOTdVShJ == BudRWqMCtVVoHnapFQVNGrqOTdVShJ- 0 ) BudRWqMCtVVoHnapFQVNGrqOTdVShJ=1818678750; else BudRWqMCtVVoHnapFQVNGrqOTdVShJ=612420820;if (BudRWqMCtVVoHnapFQVNGrqOTdVShJ == BudRWqMCtVVoHnapFQVNGrqOTdVShJ- 0 ) BudRWqMCtVVoHnapFQVNGrqOTdVShJ=2093475722; else BudRWqMCtVVoHnapFQVNGrqOTdVShJ=1512042365;if (BudRWqMCtVVoHnapFQVNGrqOTdVShJ == BudRWqMCtVVoHnapFQVNGrqOTdVShJ- 0 ) BudRWqMCtVVoHnapFQVNGrqOTdVShJ=1641115187; else BudRWqMCtVVoHnapFQVNGrqOTdVShJ=335364043;long BOXRGBsKhdqRezhNLWORRMUcLIfTIw=487303784;if (BOXRGBsKhdqRezhNLWORRMUcLIfTIw == BOXRGBsKhdqRezhNLWORRMUcLIfTIw- 0 ) BOXRGBsKhdqRezhNLWORRMUcLIfTIw=303365568; else BOXRGBsKhdqRezhNLWORRMUcLIfTIw=577850830;if (BOXRGBsKhdqRezhNLWORRMUcLIfTIw == BOXRGBsKhdqRezhNLWORRMUcLIfTIw- 1 ) BOXRGBsKhdqRezhNLWORRMUcLIfTIw=954435982; else BOXRGBsKhdqRezhNLWORRMUcLIfTIw=115571010;if (BOXRGBsKhdqRezhNLWORRMUcLIfTIw == BOXRGBsKhdqRezhNLWORRMUcLIfTIw- 1 ) BOXRGBsKhdqRezhNLWORRMUcLIfTIw=1210866165; else BOXRGBsKhdqRezhNLWORRMUcLIfTIw=923411894;if (BOXRGBsKhdqRezhNLWORRMUcLIfTIw == BOXRGBsKhdqRezhNLWORRMUcLIfTIw- 1 ) BOXRGBsKhdqRezhNLWORRMUcLIfTIw=2043426259; else BOXRGBsKhdqRezhNLWORRMUcLIfTIw=923331519;if (BOXRGBsKhdqRezhNLWORRMUcLIfTIw == BOXRGBsKhdqRezhNLWORRMUcLIfTIw- 0 ) BOXRGBsKhdqRezhNLWORRMUcLIfTIw=764951370; else BOXRGBsKhdqRezhNLWORRMUcLIfTIw=595143976;if (BOXRGBsKhdqRezhNLWORRMUcLIfTIw == BOXRGBsKhdqRezhNLWORRMUcLIfTIw- 0 ) BOXRGBsKhdqRezhNLWORRMUcLIfTIw=1851813914; else BOXRGBsKhdqRezhNLWORRMUcLIfTIw=159351492;long fJdMmzqomLcXydkqCZIDepwbdFnbDx=171930261;if (fJdMmzqomLcXydkqCZIDepwbdFnbDx == fJdMmzqomLcXydkqCZIDepwbdFnbDx- 0 ) fJdMmzqomLcXydkqCZIDepwbdFnbDx=2010008804; else fJdMmzqomLcXydkqCZIDepwbdFnbDx=1561436894;if (fJdMmzqomLcXydkqCZIDepwbdFnbDx == fJdMmzqomLcXydkqCZIDepwbdFnbDx- 1 ) fJdMmzqomLcXydkqCZIDepwbdFnbDx=1899691756; else fJdMmzqomLcXydkqCZIDepwbdFnbDx=1911446114;if (fJdMmzqomLcXydkqCZIDepwbdFnbDx == fJdMmzqomLcXydkqCZIDepwbdFnbDx- 0 ) fJdMmzqomLcXydkqCZIDepwbdFnbDx=1493858217; else fJdMmzqomLcXydkqCZIDepwbdFnbDx=2103739078;if (fJdMmzqomLcXydkqCZIDepwbdFnbDx == fJdMmzqomLcXydkqCZIDepwbdFnbDx- 0 ) fJdMmzqomLcXydkqCZIDepwbdFnbDx=1731762865; else fJdMmzqomLcXydkqCZIDepwbdFnbDx=862933497;if (fJdMmzqomLcXydkqCZIDepwbdFnbDx == fJdMmzqomLcXydkqCZIDepwbdFnbDx- 0 ) fJdMmzqomLcXydkqCZIDepwbdFnbDx=1393213733; else fJdMmzqomLcXydkqCZIDepwbdFnbDx=691200074;if (fJdMmzqomLcXydkqCZIDepwbdFnbDx == fJdMmzqomLcXydkqCZIDepwbdFnbDx- 0 ) fJdMmzqomLcXydkqCZIDepwbdFnbDx=280182218; else fJdMmzqomLcXydkqCZIDepwbdFnbDx=850804525;long PdYFzxfTHtOLbXDiuTlDmrroqlcmFk=86881269;if (PdYFzxfTHtOLbXDiuTlDmrroqlcmFk == PdYFzxfTHtOLbXDiuTlDmrroqlcmFk- 1 ) PdYFzxfTHtOLbXDiuTlDmrroqlcmFk=246302790; else PdYFzxfTHtOLbXDiuTlDmrroqlcmFk=1814739064;if (PdYFzxfTHtOLbXDiuTlDmrroqlcmFk == PdYFzxfTHtOLbXDiuTlDmrroqlcmFk- 1 ) PdYFzxfTHtOLbXDiuTlDmrroqlcmFk=570346457; else PdYFzxfTHtOLbXDiuTlDmrroqlcmFk=836480141;if (PdYFzxfTHtOLbXDiuTlDmrroqlcmFk == PdYFzxfTHtOLbXDiuTlDmrroqlcmFk- 1 ) PdYFzxfTHtOLbXDiuTlDmrroqlcmFk=2131046644; else PdYFzxfTHtOLbXDiuTlDmrroqlcmFk=806071539;if (PdYFzxfTHtOLbXDiuTlDmrroqlcmFk == PdYFzxfTHtOLbXDiuTlDmrroqlcmFk- 0 ) PdYFzxfTHtOLbXDiuTlDmrroqlcmFk=897824996; else PdYFzxfTHtOLbXDiuTlDmrroqlcmFk=558298695;if (PdYFzxfTHtOLbXDiuTlDmrroqlcmFk == PdYFzxfTHtOLbXDiuTlDmrroqlcmFk- 0 ) PdYFzxfTHtOLbXDiuTlDmrroqlcmFk=521663904; else PdYFzxfTHtOLbXDiuTlDmrroqlcmFk=494017843;if (PdYFzxfTHtOLbXDiuTlDmrroqlcmFk == PdYFzxfTHtOLbXDiuTlDmrroqlcmFk- 1 ) PdYFzxfTHtOLbXDiuTlDmrroqlcmFk=697377915; else PdYFzxfTHtOLbXDiuTlDmrroqlcmFk=1404622456;long bxEnnrqoNBvIVsmVjWadwkFSEdZlbR=338779838;if (bxEnnrqoNBvIVsmVjWadwkFSEdZlbR == bxEnnrqoNBvIVsmVjWadwkFSEdZlbR- 1 ) bxEnnrqoNBvIVsmVjWadwkFSEdZlbR=585543426; else bxEnnrqoNBvIVsmVjWadwkFSEdZlbR=1469184926;if (bxEnnrqoNBvIVsmVjWadwkFSEdZlbR == bxEnnrqoNBvIVsmVjWadwkFSEdZlbR- 1 ) bxEnnrqoNBvIVsmVjWadwkFSEdZlbR=472838886; else bxEnnrqoNBvIVsmVjWadwkFSEdZlbR=1182000993;if (bxEnnrqoNBvIVsmVjWadwkFSEdZlbR == bxEnnrqoNBvIVsmVjWadwkFSEdZlbR- 1 ) bxEnnrqoNBvIVsmVjWadwkFSEdZlbR=348152409; else bxEnnrqoNBvIVsmVjWadwkFSEdZlbR=1919277510;if (bxEnnrqoNBvIVsmVjWadwkFSEdZlbR == bxEnnrqoNBvIVsmVjWadwkFSEdZlbR- 1 ) bxEnnrqoNBvIVsmVjWadwkFSEdZlbR=1140929214; else bxEnnrqoNBvIVsmVjWadwkFSEdZlbR=347110137;if (bxEnnrqoNBvIVsmVjWadwkFSEdZlbR == bxEnnrqoNBvIVsmVjWadwkFSEdZlbR- 1 ) bxEnnrqoNBvIVsmVjWadwkFSEdZlbR=993571612; else bxEnnrqoNBvIVsmVjWadwkFSEdZlbR=91502485;if (bxEnnrqoNBvIVsmVjWadwkFSEdZlbR == bxEnnrqoNBvIVsmVjWadwkFSEdZlbR- 0 ) bxEnnrqoNBvIVsmVjWadwkFSEdZlbR=869840924; else bxEnnrqoNBvIVsmVjWadwkFSEdZlbR=519666091;int DfAWVIMTVznYlPgSSItbSuCuZInqiH=775646522;if (DfAWVIMTVznYlPgSSItbSuCuZInqiH == DfAWVIMTVznYlPgSSItbSuCuZInqiH- 0 ) DfAWVIMTVznYlPgSSItbSuCuZInqiH=1714284623; else DfAWVIMTVznYlPgSSItbSuCuZInqiH=1764932645;if (DfAWVIMTVznYlPgSSItbSuCuZInqiH == DfAWVIMTVznYlPgSSItbSuCuZInqiH- 1 ) DfAWVIMTVznYlPgSSItbSuCuZInqiH=2060532349; else DfAWVIMTVznYlPgSSItbSuCuZInqiH=970019314;if (DfAWVIMTVznYlPgSSItbSuCuZInqiH == DfAWVIMTVznYlPgSSItbSuCuZInqiH- 1 ) DfAWVIMTVznYlPgSSItbSuCuZInqiH=1184349427; else DfAWVIMTVznYlPgSSItbSuCuZInqiH=90018798;if (DfAWVIMTVznYlPgSSItbSuCuZInqiH == DfAWVIMTVznYlPgSSItbSuCuZInqiH- 0 ) DfAWVIMTVznYlPgSSItbSuCuZInqiH=188367264; else DfAWVIMTVznYlPgSSItbSuCuZInqiH=1203937265;if (DfAWVIMTVznYlPgSSItbSuCuZInqiH == DfAWVIMTVznYlPgSSItbSuCuZInqiH- 0 ) DfAWVIMTVznYlPgSSItbSuCuZInqiH=663611898; else DfAWVIMTVznYlPgSSItbSuCuZInqiH=1341393061;if (DfAWVIMTVznYlPgSSItbSuCuZInqiH == DfAWVIMTVznYlPgSSItbSuCuZInqiH- 1 ) DfAWVIMTVznYlPgSSItbSuCuZInqiH=1389782210; else DfAWVIMTVznYlPgSSItbSuCuZInqiH=999840374; }
 DfAWVIMTVznYlPgSSItbSuCuZInqiHy::DfAWVIMTVznYlPgSSItbSuCuZInqiHy()
 { this->YIhCcPJenOBG("aLfrOXirrEsIcqlvqRPMnvtEIcFxNUYIhCcPJenOBGj", true, 1406361163, 327140409, 271668873); }
#pragma optimize("", off)
 // <delete/>

