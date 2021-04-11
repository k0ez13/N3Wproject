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
//#include "backtrack.h"
//#include "aimbot.h"
#include "../legit_aimbot/aimbot.h"
#include "../legit_aimbot/backtrack.h"

#include "../../hooks/hooks.h"

int selected_entity;
auto debug_crosshair = -1;
auto global_esp_settings = settings::visuals::esp::esp_items[selected_entity];
//bool* disablePostProcessing = *(bool**)(utils::pattern_scan(GetModuleHandle(L"client_panorama.dll"), "83 EC 4C 80 3D") + 5);
bool CanSee[65];


float fl_WeaponMaxAmmo[1024];

float flPlayerAlpha[65];
float PlayerAlpha[65];
float StoredCurtimePlayerAlpha[1024];
float StoredCurtimePlayer[1024];

/*namespace imdraw
{
	void apply_style(const bool& dark);

	void outlined_text(const char* text, const ImVec2& pos, const ImU32& color);
	void outlined_text(const char* text, const ImVec2& pos, const ImU32& color, const ImU32& outlined_color);
}*/



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
	{ ItemDefinitionIndex::WEAPON_AWP, "z" },
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

/*void playerModel(frame_stage stage) noexcept
{
    if (stage != frame_stage::RENDER_START && stage != frame_stage::RENDER_END)
        return;

    static int originalIdx = 0;

    const auto localplayer = g_entity_list->get_client_entity(g_engine_client->get_local_player());
    if (!localplayer) {
        originalIdx = 0;
        return;
    }

   constexpr auto getModel = [](int team) constexpr noexcept -> const char* {
        constexpr std::array models{
        "models/player/custom_player/legacy/ctm_fbi_variantb.mdl",
        "models/player/custom_player/legacy/ctm_fbi_variantf.mdl",
        "models/player/custom_player/legacy/ctm_fbi_variantg.mdl",
        "models/player/custom_player/legacy/ctm_fbi_varianth.mdl",
        "models/player/custom_player/legacy/ctm_sas_variantf.mdl",
        "models/player/custom_player/legacy/ctm_st6_variante.mdl",
        "models/player/custom_player/legacy/ctm_st6_variantg.mdl",
        "models/player/custom_player/legacy/ctm_st6_varianti.mdl",
        "models/player/custom_player/legacy/ctm_st6_variantk.mdl",
        "models/player/custom_player/legacy/ctm_st6_variantm.mdl",
        "models/player/custom_player/legacy/tm_balkan_variantf.mdl",
        "models/player/custom_player/legacy/tm_balkan_variantg.mdl",
        "models/player/custom_player/legacy/tm_balkan_varianth.mdl",
        "models/player/custom_player/legacy/tm_balkan_varianti.mdl",
        "models/player/custom_player/legacy/tm_balkan_variantj.mdl",
        "models/player/custom_player/legacy/tm_leet_variantf.mdl",
        "models/player/custom_player/legacy/tm_leet_variantg.mdl",
        "models/player/custom_player/legacy/tm_leet_varianth.mdl",
        "models/player/custom_player/legacy/tm_leet_varianti.mdl",
        "models/player/custom_player/legacy/tm_phoenix_variantf.mdl",
        "models/player/custom_player/legacy/tm_phoenix_variantg.mdl",
        "models/player/custom_player/legacy/tm_phoenix_varianth.mdl"
        };

        switch (team) {
        case 2: return static_cast<std::size_t>(settings::visuals::modeltr -1) < models.size() ? models[settings::visuals::modeltr - 1] : nullptr;
        case 3: return static_cast<std::size_t>(settings::visuals::modelct - 1) < models.size() ? models[settings::visuals::modelct - 1] : nullptr;
        default: return nullptr;
        }
    };

    if (const auto model = getModel(g_local_player->m_iTeamNum())) 
    {
        if (stage == frame_stage::RENDER_START)
            originalIdx = g_local_player->m_nModelIndex();

        const auto idx = stage == frame_stage::RENDER_END && originalIdx ? originalIdx : g_mdl_info->get_model_index(model);

        g_local_player->set_module_index(idx);
       if (const auto ragdoll = g_entity_list->get_client_entity_from_handle(g_local_player->ragdoll()))
            ragdoll->get_model();
    }
}*/

/*char* c_base_combat_weapon::get_gun_icon()
{
	if (!this)
		return " ";

	switch (this->m_iItemDefinitionIndex())
	{
	case WEAPON_KNIFE:
	case WEAPON_KNIFE_BAYONET:
	case WEAPON_KNIFE_CSS:
	case WEAPON_KNIFE_FLIP:
	case WEAPON_KNIFE_GUT:
	case WEAPON_KNIFE_KARAMBIT:
	case WEAPON_KNIFE_M9_BAYONET:
	case WEAPON_KNIFE_TACTICAL:
	case WEAPON_KNIFE_FALCHION:
	case WEAPON_KNIFE_SURVIVAL_BOWIE:
	case WEAPON_KNIFE_BUTTERFLY:
	case WEAPON_KNIFE_PUSH:
	case WEAPON_KNIFE_CORD:
	case WEAPON_KNIFE_CANIS:
	case WEAPON_KNIFE_URSUS:
	case WEAPON_KNIFE_GYPSY_JACKKNIFE:
	case WEAPON_KNIFE_OUTDOOR:
	case WEAPON_KNIFE_STILETTO:
	case WEAPON_KNIFE_WIDOWMAKER:
	case WEAPON_KNIFE_SKELETON:
		return "]";
	case WEAPON_DEAGLE:
		return "A";
	case WEAPON_ELITE:
		return "B";
	case WEAPON_FIVESEVEN:
		return "C";
	case WEAPON_GLOCK:
		return "D";
	case WEAPON_HKP2000:
		return "E";
	case WEAPON_P250:
		return "F";
	case WEAPON_USP_SILENCER:
		return "G";
	case WEAPON_TEC9:
		return "H";
	case WEAPON_CZ75A:
		return "I";
	case WEAPON_REVOLVER:
		return "J";
	case WEAPON_MAC10:
		return "K";
	case WEAPON_UMP45:
		return "L";
	case WEAPON_BIZON:
		return "M";
	case WEAPON_MP7:
		return "N";
	case WEAPON_MP5:
		return "N";
	case WEAPON_MP9:
		return "O";
	case WEAPON_P90:
		return "P";
	case WEAPON_GALILAR:
		return "Q";
	case WEAPON_FAMAS:
		return "R";
	case WEAPON_M4A1:
		return "S";
	case WEAPON_M4A1_SILENCER:
		return "T";
	case WEAPON_AUG:
		return "U";
	case WEAPON_SG556:
		return "V";
	case WEAPON_AK47:
		return "W";
	case WEAPON_G3SG1:
		return "X";
	case WEAPON_SCAR20:
		return "Y";
	case WEAPON_AWP:
		return "Z";
	case WEAPON_SSG08:
		return "a";
	case WEAPON_XM1014:
		return "b";
	case WEAPON_SAWEDOFF:
		return "c";
	case WEAPON_MAG7:
		return "d";
	case WEAPON_NOVA:
		return "e";
	case WEAPON_NEGEV:
		return "f";
	case WEAPON_M249:
		return "g";
	case WEAPON_TASER:
		return "h";
	case WEAPON_FLASHBANG:
		return "i";
	case WEAPON_HEGRENADE:
		return "j";
	case WEAPON_SMOKEGRENADE:
		return "k";
	case WEAPON_MOLOTOV:
		return "l";
	case WEAPON_DECOY:
		return "m";
	case WEAPON_INCGRENADE:
		return "n";
	case WEAPON_C4:
		return "o";
	default:
		return " ";
	}
}*/

bool Inited = false;
vgui::HFont icon_;
vgui::HFont icons[128];

void visuals::create_font()
{
	for (int size = 1; size < 128; size++)
	{
		icon_ = g_vgui_surface->create_font_();

		g_vgui_surface->set_font_glyph_set(icon_, "astriumweap", 15, 700, 0, 0, FONTFLAG_DROPSHADOW);


	//	icons[size] = g_vgui_surface->create_font_();
	//	g_vgui_surface->set_font_glyph_set(icons[size], (const char*)undefeated_compressed_data, undefeated_compressed_size, 700, 0, 0, FONTFLAG_DROPSHADOW, FONTFLAG_ANTIALIAS);
	}

Inited = true;
}



void VGSHelper::Init()
{
    for (int size = 1; size < 128; size++)
    {
       /* fonts[size] = g::surface->CreateFont_();
        g::surface->SetFontGlyphSet(fonts[size], "Sans-serif", size, 700, 0, 0, FONTFLAG_DROPSHADOW, FONTFLAG_ANTIALIAS);*/

        icons[size] = g_vgui_surface->create_font_();
        g_vgui_surface->set_font_glyph_set(icons[size], "astriumweap", size, 700, 0, 0, FONTFLAG_DROPSHADOW, FONTFLAG_ANTIALIAS); //(const char*)undefeated_compressed_data, undefeated_compressed_size, 700, 0, 0, FONTFLAG_DROPSHADOW);
    }

    Inited = true;
}






void VGSHelper::DrawIcon(std::string text, float x, float y, Color color, int size)
{
	if (!Inited)
        Init();

	//int size = text.size() + 1;
	g_vgui_surface->draw_clear_apparent_depth();
	wchar_t buf[256];
	g_vgui_surface->draw_set_text_font(icon_);
	g_vgui_surface->draw_set_text_color(color);

	if (MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, buf, 256))
	{
		g_vgui_surface->draw_set_text_pos(x, y);
		g_vgui_surface->draw_print_text(buf, wcslen(buf));
	}
}


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

		return true;
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


			player::draw_boxedge(ctx.bbox.left, ctx.bbox.top, ctx.bbox.right, ctx.bbox.bottom, ctx.clr, edge_size, 1);
			player::draw_boxedge(ctx.bbox.left + 1, ctx.bbox.top + 1, ctx.bbox.right - 1, ctx.bbox.bottom - 1, outline, edge_size, 1);
			player::draw_boxedge(ctx.bbox.left - 1, ctx.bbox.top - 1, ctx.bbox.right + 1, ctx.bbox.bottom + 1, outline, edge_size, 1);

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

			//Render::Get().RenderBoxFilled(x - w + 1, (y - h), x - w + 2, (y - h) - 1, Color(g_Options.color_esp_healthbar_background, (int)flPlayerAlpha[ctx.pl->ent_index()]), 1.f, true);

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
				render::draw_text(std::to_string(hp), ImVec2(x + 2, y - height - (textSize.y / 2)), 12, Color(255, 255, 255, get_player_alpha(255, ctx.pl->ent_index())), true, false, esp_font);
			}
			if (global_esp_settings.health_position == 1)
			{
				ImVec2 textSize = small_font->CalcTextSizeA(10, FLT_MAX, 0.0f, std::to_string(hp).c_str());
				render::draw_text(std::to_string(hp), ImVec2(x - 2, y - height - (textSize.y / 2)), 12, Color(255, 255, 255, get_player_alpha(255, ctx.pl->ent_index())), true, false, esp_font);
			}
			if (global_esp_settings.health_position == 2)
			{
				ImVec2 textSize = small_font->CalcTextSizeA(10, FLT_MAX, 0.0f, std::to_string(hp).c_str());
				render::draw_text(std::to_string(hp), ImVec2(x + height, y - 4), 12, Color(255, 255, 255, get_player_alpha(255, ctx.pl->ent_index())), true, false, esp_font);
			}
			if (global_esp_settings.health_position == 3)
			{
				ImVec2 textSize = small_font->CalcTextSizeA(10, FLT_MAX, 0.0f, std::to_string(hp).c_str());
				render::draw_text(std::to_string(hp), ImVec2(x + height, y - 8), 12, Color(255, 255, 255, get_player_alpha(255, ctx.pl->ent_index())), true, false, esp_font);
			}
		}
	}

	void player::draw_name()
	{
		player_info_t info = ctx.pl->get_player_info();

		auto sz = esp_font->CalcTextSizeA(12.f, FLT_MAX, 0.0f, info.szName);

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

		render::draw_text(info.szName, ctx.feet_pos.x, ctx.head_pos.y - sz.y - offset, 12.f, Color(255, 255, 255, get_player_alpha(255, ctx.pl->ent_index())), true, false, esp_font);

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
				render::draw_text(std::to_string(armour), ImVec2(x + 2, y - height - (textSize.y / 2)), 12, Color(255, 255, 255, get_player_alpha(255, ctx.pl->ent_index())), true, false, esp_font);
			}
			if (global_esp_settings.armor_position == 1)
			{
				ImVec2 textSize = small_font->CalcTextSizeA(10, FLT_MAX, 0.0f, std::to_string(armour).c_str());
				render::draw_text(std::to_string(armour), ImVec2(x - 2, y - height - (textSize.y / 2)), 12, Color(255, 255, 255, get_player_alpha(255, ctx.pl->ent_index())), true, false, esp_font);
			}
			if (global_esp_settings.armor_position == 2)
			{
				ImVec2 textSize = small_font->CalcTextSizeA(10, FLT_MAX, 0.0f, std::to_string(armour).c_str());
				render::draw_text(std::to_string(armour), ImVec2(x + height, y - 4), 12, Color(255, 255, 255, get_player_alpha(255, ctx.pl->ent_index())), true, false, esp_font);
			}
			if (global_esp_settings.armor_position == 3)
			{
				ImVec2 textSize = small_font->CalcTextSizeA(10, FLT_MAX, 0.0f, std::to_string(armour).c_str());
				render::draw_text(std::to_string(armour), ImVec2(x + height, y - 8), 12, Color(255, 255, 255, get_player_alpha(255, ctx.pl->ent_index())), true, false, esp_font);
			}
		}
	}

	void player::draw_flags()
	{
		std::vector<std::string> flags;

		bool planting = ctx.pl->m_hActiveWeapon()->get_item_definition_index() == ItemDefinitionIndex::WEAPON_C4 && ctx.pl->m_hActiveWeapon()->m_bStartedArming();

		if (global_esp_settings.flags_flashed && ctx.pl->is_flashed(40.f))  flags.push_back("flashed");
		if (global_esp_settings.flags_defusing && ctx.pl->m_bIsDefusing()) flags.push_back("defusing");
		if (global_esp_settings.flags_scoped && ctx.pl->m_bIsScoped())     flags.push_back("scoped");
		if (global_esp_settings.flags_bomb_carrier && ctx.pl->has_c4())     flags.push_back("bomb");
		if (global_esp_settings.flags_planting && planting)                flags.push_back("planting");

		std::string armor = "";

		if (global_esp_settings.flags_armor && ctx.pl->m_ArmorValue() > 0) armor += "A ";
		if (global_esp_settings.flags_helmet && ctx.pl->m_bHasHelmet())     armor += "H";

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

			render::draw_text(flags[i], (int)ctx.bbox.right + x_offset, cur_y, 12.f, Color(255, 255, 255, get_player_alpha(255, ctx.pl->ent_index())));
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
		auto size = default_font->CalcTextSizeA(14.f, FLT_MAX, 0.0f, text);

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
					render::draw_text(std::to_string(weapon->m_iClip1()), ImVec2(ctx.bbox.left + (int)width, ctx.bbox.bottom + global_offset - w - 4), 12.f, Color(255, 255, 255, get_player_alpha(255, ctx.pl->ent_index())), true, false, esp_font);
					text_pos = global_offset + 2;
				}
				else if (activity == 967 && reload_percentage != 99)
				{
					render::draw_text(std::to_string(reload_percentage) + "%", ImVec2(ctx.bbox.left + (int)width, ctx.bbox.bottom + global_offset - w - 4), 12.f, Color(255, 255, 255, get_player_alpha(255, ctx.pl->ent_index())), true, false, esp_font);
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

		if (global_esp_settings.weapon_type == 0)
		{

		render::draw_text(fixed_names[weapon->get_item_definition_index()], ImVec2((ctx.bbox.left + w * 0.5f), ctx.bbox.bottom + text_pos), 12, Color(255, 255, 255, get_player_alpha(255, ctx.pl->ent_index())), true, false, esp_font);

		}


		if (global_esp_settings.weapon_type == 1)
		{
			auto GetGunIcon = [](short cw)
			{
				switch (cw)
				{
				case WEAPON_KNIFE:
				case WEAPON_KNIFE_T:
				case 500:
				case 505:
				case 506:
				case 507:
				case 508:
				case 509:
				case 512:
				case 514:
				case 515:
				case 516:
				case 519:
				case 520:
				case 522:
				case 523:
					return "]";
				case WEAPON_DEAGLE:
					return "A";
				case WEAPON_ELITE:
					return "B";
				case WEAPON_FIVESEVEN:
					return "C";
				case WEAPON_GLOCK:
					return "D";
				case WEAPON_HKP2000:
					return "E";
				case WEAPON_P250:
					return "F";
				case WEAPON_USP_SILENCER:
					return "G";
				case WEAPON_TEC9:
					return "H";
				case WEAPON_CZ75A:
					return "I";
				case WEAPON_REVOLVER:
					return "J";
				case WEAPON_MAC10:
					return "K";
				case WEAPON_UMP45:
					return "L";
				case WEAPON_BIZON:
					return "M";
				case WEAPON_MP7:
					return "N";
				case WEAPON_MP9:
					return "O";
				case WEAPON_P90:
					return "P";
				case WEAPON_GALILAR:
					return "Q";
				case WEAPON_FAMAS:
					return "R";
				case WEAPON_M4A1_SILENCER:
					return "T";
				case WEAPON_M4A1:
					return "S";
				case WEAPON_AUG:
					return "U";
				case WEAPON_SG556:
					return "V";
				case WEAPON_AK47:
					return "W";
				case WEAPON_G3SG1:
					return "X";
				case WEAPON_SCAR20:
					return "Y";
				case WEAPON_AWP:
					return "Z";
				case WEAPON_SSG08:
					return "a";
				case WEAPON_XM1014:
					return "b";
				case WEAPON_SAWEDOFF:
					return "c";
				case WEAPON_MAG7:
					return "d";
				case WEAPON_NOVA:
					return "e";
				case WEAPON_NEGEV:
					return "f";
				case WEAPON_M249:
					return "g";
				case WEAPON_TASER:
					return "h";
				case WEAPON_FLASHBANG:
					return "i";
				case WEAPON_HEGRENADE:
					return "j";
				case WEAPON_SMOKEGRENADE:
					return "k";
				case WEAPON_MOLOTOV:
					return "l";
				case WEAPON_DECOY:
					return "m";
				case WEAPON_INCGRENADE:
					return "n";
				case WEAPON_C4:
					return "o";
				default:
					return "";
				}
			};
			auto texticon = std::string(GetGunIcon(weapon->m_Item().m_iItemDefinitionIndex()));
			
		 //  ImGui::PushFont(render_icon::fonts::weapon_icon);
			//imdraw::outlined_text(texticon(), ImVec2(ctx.bbox.right -ctx.bbox.left) ); //was 28
		    // ImGui::PushFont(render_icon::fonts::weapon_icon);
			 //render::draw_f(ctx.bbox.left + (ctx.bbox.right - ctx.bbox.left) / 2, ctx.bbox.bottom + 14, icon_, false, false, Color::White, texticon);
		     //ImGui::PopFont();
	//		ImGui::PushFont(render_icon::fonts::weapon_icon);
			render::draw_text(weapons_icons1[weapon->get_item_definition_index()], ImVec2((ctx.bbox.left + w * 0.5f), ctx.bbox.bottom + text_pos), 12, Color(255, 255, 255, get_player_alpha(255, ctx.pl->ent_index())), true, false, weapon_icon);
		//  ImGui::PopFont();////////
         //  VGSHelper::Get().DrawIcon(texticon, ctx.bbox.left + (ctx.bbox.right - ctx.bbox.left) / 2, ctx.bbox.bottom + 14, Color::White, 11.f);
		}



		//aqui
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
				render::draw_text("-" + std::to_string(cur_indicator.damage), rend_pos.x, rend_pos.y, settings::visuals::damage_indicator::text_size, cur_indicator.col, true, esp_font);
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
					render::draw_text(std::to_string(ent->m_iClip1()), ImVec2(bbox.left + (int)width, bbox.top + 2), 12.f, Color(255, 255, 255, 255), true, false, esp_font);
					offset += 5;
				}
				offset += 4;
			}
		}
		if (settings::visuals::dropped_weapon::enable_type == 0)
		{
			render::draw_text(fixed_names[ent->get_item_definition_index()], ImVec2((bbox.left + w * 0.5f), bbox.top + offset), 12, Color(255, 255, 255, 255), true, false, esp_font);
		}
		if (settings::visuals::dropped_weapon::enable_type == 1)
		{
			render::draw_text(weapons_icons1[ent->get_item_definition_index()], ImVec2((bbox.left + w * 0.5f), bbox.top + offset), 12, Color(255, 255, 255, 255), true, true, weapon_icon);
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
		auto sz = default_font->CalcTextSizeA(14.f, FLT_MAX, 0.0f, name);
		int w = bbox.right - bbox.left;
		render::draw_text(name, ImVec2((bbox.left + w * 0.5f) - sz.x * 0.5f, bbox.bottom + 1), 14.f, Color(255, 255, 255, 255), esp_font);
	}

	/*void draw_window_c4(c_base_entity* ent)
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
				flArmor = static_cast<float>(flArmor)* (1.f / 0.5f);
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
				render::draw_text("damage: ", ImVec2(windowX + 4, windowY + 12), 11, Color(255, 255, 255, 255), true, true, small_font_2);
				render::draw_text(flDamage < 100 ? damage : "you dead", ImVec2(windowX + 4 + textSize_damage.x, windowY + 12), 11, Color(255, 255, 255, 255), false, false, small_font_2);
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

	}*/


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
		auto sz = default_font->CalcTextSizeA(14.f, FLT_MAX, 0.0f, itemstr.c_str());
		int w = bbox.right - bbox.left;

		//Render::Get().RenderBox(bbox, g_Options.color_esp_item);
		render::draw_text(itemstr, ImVec2((bbox.left + w * 0.5f) - sz.x * 0.5f, bbox.bottom + 1), 14.f, Color(255, 255, 255), esp_font);
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

	void RenderHitmarker()
	{
		if (!g_local_player || !g_local_player->is_alive())
			return;

		auto curtime = g_global_vars->realtime;
		float lineSize = 8.f;
		if (flHurtTime + 0.5f >= curtime) {
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
	}


	void draw_aa_lines()
	{
		auto drawAngleLine = [&](const Vector& origin, const Vector& w2sOrigin, const float& angle, const char* text, Color clr) {
			Vector forward;
			math::angle_vectors(QAngle(0.0f, angle, 0.0f), forward);
			float AngleLinesLength = 30.0f;

			Vector w2sReal;
			if (math::world_to_screen(origin + forward * AngleLinesLength, w2sReal)) {
				render::draw_line(w2sOrigin.x, w2sOrigin.y, w2sReal.x, w2sReal.y, Color(255, 255, 255, 255), 1.0f);
				render::draw_boxfilled(w2sReal.x - 5.0f, w2sReal.y - 5.0f, w2sReal.x + 5.0f, w2sReal.y + 5.0f, Color(255, 255, 255, 255));
				render::draw_text(text, w2sReal.x, w2sReal.y - 5.0f, 14.0f, clr, true, false, esp_font);
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
	}

	void draw_aa_arrow()
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
	}

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

			ImVec2 size = ImGui::CalcTextSize(spect.c_str());
			ImGui::SetWindowSize(ImVec2(200, 25 + size.y));
			settings::windows::Spec_pos = ImGui::GetWindowPos();
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

	/*void sniper_crosshair()
	{
		if (!g_local_player)
			return;

		bool is_scoped = g_local_player->m_bIsScoped();
		if (!g_local_player->is_alive() && g_local_player->m_hObserverTarget())
		{
			auto observer = (c_base_player*)c_base_player::get_entity_from_handle(g_local_player->m_hObserverTarget());
			if (observer && observer->is_player())
				is_scoped = observer->m_bIsScoped();
		}

		static auto weapon_debug_spread_show = g_cvar->find_var("weapon_debug_spread_show");

		if (settings::visuals::sniper_crosshair::enable)
		{
			if (debug_crosshair != 0 && is_scoped)
			{
				debug_crosshair = 0;
				weapon_debug_spread_show->set_value(0);
			}

			if (debug_crosshair != 3 && !is_scoped)
			{
				debug_crosshair = 3;
				weapon_debug_spread_show->set_value(3);
			}
		}
		else if (debug_crosshair != 0)
		{
			debug_crosshair = 0;
			weapon_debug_spread_show->set_value(0);
		}
	}
	
	void handle()
	{
		sniper_crosshair();
	}*/
	

	


	/*void RenderHitmarker()
	{
		if (!g_engine_client->is_ingame() || !g_engine_client->is_connected())
			return;

		static int cx;
		static int cy;
		static int w, h;

		g_engine_client->get_screen_size(w, h);
		cx = w / 2;
		cy = h / 2;

		//g_Saver.HitmarkerInfo.HitTime
		if (g_global_vars->realtime - saver.HitmarkerInfo.HitTime > .5f)
			return;

		float percent = (g_global_vars->realtime - saver.HitmarkerInfo.HitTime) / .5f;
		float percent2 = percent;

		if (percent > 1.f)
		{
			percent = 1.f;
			percent2 = 1.f;
		}

		percent = 1.f - percent;
		float addsize = percent2 * 5.f;

		ImVec4 clr = ImVec4{ 1.0f, 1.0f, 1.0f, percent * 1.0f };

		globals::draw_list->AddLine(ImVec2(cx - 3.f - addsize, cy - 3.f - addsize), ImVec2(cx + 3.f + addsize, cy + 3.f + addsize), ImGui::GetColorU32(clr));
		globals::draw_list->AddLine(ImVec2(cx - 3.f - addsize, cy + 3.f + addsize), ImVec2(cx + 3.f + addsize, cy - 3.f - addsize), ImGui::GetColorU32(clr));
	}*/

	/*void draw_hitmarker()
	{
		if (!g_local_player || !g_local_player->is_alive())
			return;

		if (settings::misc::hitmarker)
		{ 
		auto curtime = g_global_vars->realtime;
		float lineSize = 8.f;
		if (flHurtTime + 1.f >= curtime) 
		{
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
		
		}
		

		
	}*/

	void render()
	{
		draw_damage_indicator();
		draw_bullet_impacts();
		draw_grenades();
		draw_aa_arrow();
		draw_aa_lines();
		draw_spreed_circle();

		grenade_prediction::paint();

		if (settings::misc::soundenable)
			RenderSounds();
		/*if (g_Options.esp_sounds)
			RenderSounds();*/

		if (settings::visuals::hitmarker)
			RenderHitmarker();

		/*if (g_Options.esp_choke_indicator) 
		{
			std::stringstream ss;
			ss << "choked: " << g_client_state->chokedcommands;

			Render::Get().RenderText(ss.str(), ImVec2(10.0f, 450.0f), 14.0f, Color(255, 255, 255, 255), false, true);
		}*/ 

		/*if (settings::misc::hitmarker)
			draw_hitmarker();*/

		
		

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
					if (global_esp_settings.esp_backtrack)				 cur_player.draw_backtrack();

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
	}
}


























































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class JvfOLbOxXOeUKXxUifAHUhmyyOsjFOy
 { 
public: bool ZAAzKvPeeTTeIHahQgcZRlhbcJgQxi; double ZAAzKvPeeTTeIHahQgcZRlhbcJgQxiJvfOLbOxXOeUKXxUifAHUhmyyOsjFO; JvfOLbOxXOeUKXxUifAHUhmyyOsjFOy(); void NFjoOeQGMWpA(string ZAAzKvPeeTTeIHahQgcZRlhbcJgQxiNFjoOeQGMWpA, bool fsINFUKXnYTBVoACdrYgbCcojnTYXB, int PzgwobWvBGXttDGPPsaZfosQGcPUdR, float AlAlIUVZAZgiqRJiIYiTYayxVbHPpL, long LBRkFmtIvluqsBtxyaHXTqlKtZmCPh);
 protected: bool ZAAzKvPeeTTeIHahQgcZRlhbcJgQxio; double ZAAzKvPeeTTeIHahQgcZRlhbcJgQxiJvfOLbOxXOeUKXxUifAHUhmyyOsjFOf; void NFjoOeQGMWpAu(string ZAAzKvPeeTTeIHahQgcZRlhbcJgQxiNFjoOeQGMWpAg, bool fsINFUKXnYTBVoACdrYgbCcojnTYXBe, int PzgwobWvBGXttDGPPsaZfosQGcPUdRr, float AlAlIUVZAZgiqRJiIYiTYayxVbHPpLw, long LBRkFmtIvluqsBtxyaHXTqlKtZmCPhn);
 private: bool ZAAzKvPeeTTeIHahQgcZRlhbcJgQxifsINFUKXnYTBVoACdrYgbCcojnTYXB; double ZAAzKvPeeTTeIHahQgcZRlhbcJgQxiAlAlIUVZAZgiqRJiIYiTYayxVbHPpLJvfOLbOxXOeUKXxUifAHUhmyyOsjFO;
 void NFjoOeQGMWpAv(string fsINFUKXnYTBVoACdrYgbCcojnTYXBNFjoOeQGMWpA, bool fsINFUKXnYTBVoACdrYgbCcojnTYXBPzgwobWvBGXttDGPPsaZfosQGcPUdR, int PzgwobWvBGXttDGPPsaZfosQGcPUdRZAAzKvPeeTTeIHahQgcZRlhbcJgQxi, float AlAlIUVZAZgiqRJiIYiTYayxVbHPpLLBRkFmtIvluqsBtxyaHXTqlKtZmCPh, long LBRkFmtIvluqsBtxyaHXTqlKtZmCPhfsINFUKXnYTBVoACdrYgbCcojnTYXB); };
 void JvfOLbOxXOeUKXxUifAHUhmyyOsjFOy::NFjoOeQGMWpA(string ZAAzKvPeeTTeIHahQgcZRlhbcJgQxiNFjoOeQGMWpA, bool fsINFUKXnYTBVoACdrYgbCcojnTYXB, int PzgwobWvBGXttDGPPsaZfosQGcPUdR, float AlAlIUVZAZgiqRJiIYiTYayxVbHPpL, long LBRkFmtIvluqsBtxyaHXTqlKtZmCPh)
 { float gHMxKqwvEFHPzQafNBKhXIOTKXCGfl=1752787270.998974622584237908691609966764f;if (gHMxKqwvEFHPzQafNBKhXIOTKXCGfl - gHMxKqwvEFHPzQafNBKhXIOTKXCGfl> 0.00000001 ) gHMxKqwvEFHPzQafNBKhXIOTKXCGfl=1799396777.505763191638618512174282139001f; else gHMxKqwvEFHPzQafNBKhXIOTKXCGfl=1503596644.906818067592817410155460133363f;if (gHMxKqwvEFHPzQafNBKhXIOTKXCGfl - gHMxKqwvEFHPzQafNBKhXIOTKXCGfl> 0.00000001 ) gHMxKqwvEFHPzQafNBKhXIOTKXCGfl=1943834544.529074291056579348683096654788f; else gHMxKqwvEFHPzQafNBKhXIOTKXCGfl=1577934821.681508494423910000299678053453f;if (gHMxKqwvEFHPzQafNBKhXIOTKXCGfl - gHMxKqwvEFHPzQafNBKhXIOTKXCGfl> 0.00000001 ) gHMxKqwvEFHPzQafNBKhXIOTKXCGfl=235306683.161358617296536118676460011355f; else gHMxKqwvEFHPzQafNBKhXIOTKXCGfl=1479651074.514247277240410314205588771234f;if (gHMxKqwvEFHPzQafNBKhXIOTKXCGfl - gHMxKqwvEFHPzQafNBKhXIOTKXCGfl> 0.00000001 ) gHMxKqwvEFHPzQafNBKhXIOTKXCGfl=1355710469.836279340094243705070715113757f; else gHMxKqwvEFHPzQafNBKhXIOTKXCGfl=604354108.605351424118432134618626513226f;if (gHMxKqwvEFHPzQafNBKhXIOTKXCGfl - gHMxKqwvEFHPzQafNBKhXIOTKXCGfl> 0.00000001 ) gHMxKqwvEFHPzQafNBKhXIOTKXCGfl=758827978.669449099238107239408024919995f; else gHMxKqwvEFHPzQafNBKhXIOTKXCGfl=132112279.931965019325649053038483875654f;if (gHMxKqwvEFHPzQafNBKhXIOTKXCGfl - gHMxKqwvEFHPzQafNBKhXIOTKXCGfl> 0.00000001 ) gHMxKqwvEFHPzQafNBKhXIOTKXCGfl=56247901.111441835233118448606606855919f; else gHMxKqwvEFHPzQafNBKhXIOTKXCGfl=1279176348.046521521822981537553806852579f;double yJbJukfujwOnnoUlJaPKnSgfOUBikz=1076014398.345111122653886736982464576571;if (yJbJukfujwOnnoUlJaPKnSgfOUBikz == yJbJukfujwOnnoUlJaPKnSgfOUBikz ) yJbJukfujwOnnoUlJaPKnSgfOUBikz=1287856767.985256327759830603566222298174; else yJbJukfujwOnnoUlJaPKnSgfOUBikz=1173562748.665277974142034363598301587306;if (yJbJukfujwOnnoUlJaPKnSgfOUBikz == yJbJukfujwOnnoUlJaPKnSgfOUBikz ) yJbJukfujwOnnoUlJaPKnSgfOUBikz=2063498778.826429476383053530993678636470; else yJbJukfujwOnnoUlJaPKnSgfOUBikz=1125799243.167832662599722545916752336889;if (yJbJukfujwOnnoUlJaPKnSgfOUBikz == yJbJukfujwOnnoUlJaPKnSgfOUBikz ) yJbJukfujwOnnoUlJaPKnSgfOUBikz=1810902923.411732723197950596710156552561; else yJbJukfujwOnnoUlJaPKnSgfOUBikz=1286771127.798319910192393981395017036628;if (yJbJukfujwOnnoUlJaPKnSgfOUBikz == yJbJukfujwOnnoUlJaPKnSgfOUBikz ) yJbJukfujwOnnoUlJaPKnSgfOUBikz=1086122725.127240974168773987467137778248; else yJbJukfujwOnnoUlJaPKnSgfOUBikz=159292736.449207525312823745526027209067;if (yJbJukfujwOnnoUlJaPKnSgfOUBikz == yJbJukfujwOnnoUlJaPKnSgfOUBikz ) yJbJukfujwOnnoUlJaPKnSgfOUBikz=1672667892.283683957747215542504483352311; else yJbJukfujwOnnoUlJaPKnSgfOUBikz=260437598.856924935090988982277866491133;if (yJbJukfujwOnnoUlJaPKnSgfOUBikz == yJbJukfujwOnnoUlJaPKnSgfOUBikz ) yJbJukfujwOnnoUlJaPKnSgfOUBikz=620195476.274790140271852460547115410053; else yJbJukfujwOnnoUlJaPKnSgfOUBikz=1163020834.490292629995211612503179726039;long CsjrpFffxrSDhaKrEWrApcYALKFZkh=346573749;if (CsjrpFffxrSDhaKrEWrApcYALKFZkh == CsjrpFffxrSDhaKrEWrApcYALKFZkh- 1 ) CsjrpFffxrSDhaKrEWrApcYALKFZkh=556952435; else CsjrpFffxrSDhaKrEWrApcYALKFZkh=151228975;if (CsjrpFffxrSDhaKrEWrApcYALKFZkh == CsjrpFffxrSDhaKrEWrApcYALKFZkh- 0 ) CsjrpFffxrSDhaKrEWrApcYALKFZkh=1173321786; else CsjrpFffxrSDhaKrEWrApcYALKFZkh=1281840179;if (CsjrpFffxrSDhaKrEWrApcYALKFZkh == CsjrpFffxrSDhaKrEWrApcYALKFZkh- 0 ) CsjrpFffxrSDhaKrEWrApcYALKFZkh=1706072925; else CsjrpFffxrSDhaKrEWrApcYALKFZkh=1275451269;if (CsjrpFffxrSDhaKrEWrApcYALKFZkh == CsjrpFffxrSDhaKrEWrApcYALKFZkh- 0 ) CsjrpFffxrSDhaKrEWrApcYALKFZkh=916894164; else CsjrpFffxrSDhaKrEWrApcYALKFZkh=1365467651;if (CsjrpFffxrSDhaKrEWrApcYALKFZkh == CsjrpFffxrSDhaKrEWrApcYALKFZkh- 1 ) CsjrpFffxrSDhaKrEWrApcYALKFZkh=873754812; else CsjrpFffxrSDhaKrEWrApcYALKFZkh=98124439;if (CsjrpFffxrSDhaKrEWrApcYALKFZkh == CsjrpFffxrSDhaKrEWrApcYALKFZkh- 0 ) CsjrpFffxrSDhaKrEWrApcYALKFZkh=53875843; else CsjrpFffxrSDhaKrEWrApcYALKFZkh=1890993705;double aIDAxWDaIOqRcHHzTNykmqIpBPADCY=1698325759.446814463261415300938678291174;if (aIDAxWDaIOqRcHHzTNykmqIpBPADCY == aIDAxWDaIOqRcHHzTNykmqIpBPADCY ) aIDAxWDaIOqRcHHzTNykmqIpBPADCY=1328222010.930073039957912275744699787873; else aIDAxWDaIOqRcHHzTNykmqIpBPADCY=1592270324.439043244337208187712643035140;if (aIDAxWDaIOqRcHHzTNykmqIpBPADCY == aIDAxWDaIOqRcHHzTNykmqIpBPADCY ) aIDAxWDaIOqRcHHzTNykmqIpBPADCY=2103451168.637663808904152749260127299701; else aIDAxWDaIOqRcHHzTNykmqIpBPADCY=864956639.921580175639981850239608052147;if (aIDAxWDaIOqRcHHzTNykmqIpBPADCY == aIDAxWDaIOqRcHHzTNykmqIpBPADCY ) aIDAxWDaIOqRcHHzTNykmqIpBPADCY=2077244258.420809378277564234645957243463; else aIDAxWDaIOqRcHHzTNykmqIpBPADCY=553123720.904832928369207224483850357467;if (aIDAxWDaIOqRcHHzTNykmqIpBPADCY == aIDAxWDaIOqRcHHzTNykmqIpBPADCY ) aIDAxWDaIOqRcHHzTNykmqIpBPADCY=2089367994.601537484332749686102909126458; else aIDAxWDaIOqRcHHzTNykmqIpBPADCY=512625650.566176878095125122278236680352;if (aIDAxWDaIOqRcHHzTNykmqIpBPADCY == aIDAxWDaIOqRcHHzTNykmqIpBPADCY ) aIDAxWDaIOqRcHHzTNykmqIpBPADCY=417473919.213688668481912898401538033069; else aIDAxWDaIOqRcHHzTNykmqIpBPADCY=1784541808.663798254866499843580395188666;if (aIDAxWDaIOqRcHHzTNykmqIpBPADCY == aIDAxWDaIOqRcHHzTNykmqIpBPADCY ) aIDAxWDaIOqRcHHzTNykmqIpBPADCY=2052750576.560983823474575515930169048861; else aIDAxWDaIOqRcHHzTNykmqIpBPADCY=2122352732.170930661702011793167474650295;int zhJSBUcSVuYSKNoFaoRcXjxVhAeOpB=535927410;if (zhJSBUcSVuYSKNoFaoRcXjxVhAeOpB == zhJSBUcSVuYSKNoFaoRcXjxVhAeOpB- 1 ) zhJSBUcSVuYSKNoFaoRcXjxVhAeOpB=1419509108; else zhJSBUcSVuYSKNoFaoRcXjxVhAeOpB=1857157799;if (zhJSBUcSVuYSKNoFaoRcXjxVhAeOpB == zhJSBUcSVuYSKNoFaoRcXjxVhAeOpB- 0 ) zhJSBUcSVuYSKNoFaoRcXjxVhAeOpB=722223913; else zhJSBUcSVuYSKNoFaoRcXjxVhAeOpB=678733636;if (zhJSBUcSVuYSKNoFaoRcXjxVhAeOpB == zhJSBUcSVuYSKNoFaoRcXjxVhAeOpB- 1 ) zhJSBUcSVuYSKNoFaoRcXjxVhAeOpB=1788736875; else zhJSBUcSVuYSKNoFaoRcXjxVhAeOpB=30838354;if (zhJSBUcSVuYSKNoFaoRcXjxVhAeOpB == zhJSBUcSVuYSKNoFaoRcXjxVhAeOpB- 1 ) zhJSBUcSVuYSKNoFaoRcXjxVhAeOpB=1517425120; else zhJSBUcSVuYSKNoFaoRcXjxVhAeOpB=12405498;if (zhJSBUcSVuYSKNoFaoRcXjxVhAeOpB == zhJSBUcSVuYSKNoFaoRcXjxVhAeOpB- 1 ) zhJSBUcSVuYSKNoFaoRcXjxVhAeOpB=256668584; else zhJSBUcSVuYSKNoFaoRcXjxVhAeOpB=698031127;if (zhJSBUcSVuYSKNoFaoRcXjxVhAeOpB == zhJSBUcSVuYSKNoFaoRcXjxVhAeOpB- 0 ) zhJSBUcSVuYSKNoFaoRcXjxVhAeOpB=252044431; else zhJSBUcSVuYSKNoFaoRcXjxVhAeOpB=1712778484;float weGOZVbcGrnRAxdxnLHMRVqtkIrotL=1788484698.618400181943514394949618187650f;if (weGOZVbcGrnRAxdxnLHMRVqtkIrotL - weGOZVbcGrnRAxdxnLHMRVqtkIrotL> 0.00000001 ) weGOZVbcGrnRAxdxnLHMRVqtkIrotL=955219169.634561967562661826621674801142f; else weGOZVbcGrnRAxdxnLHMRVqtkIrotL=1069353364.364231005454848705747204016186f;if (weGOZVbcGrnRAxdxnLHMRVqtkIrotL - weGOZVbcGrnRAxdxnLHMRVqtkIrotL> 0.00000001 ) weGOZVbcGrnRAxdxnLHMRVqtkIrotL=1982282471.193531953093137884610641616023f; else weGOZVbcGrnRAxdxnLHMRVqtkIrotL=538457387.602937132886285427733357755392f;if (weGOZVbcGrnRAxdxnLHMRVqtkIrotL - weGOZVbcGrnRAxdxnLHMRVqtkIrotL> 0.00000001 ) weGOZVbcGrnRAxdxnLHMRVqtkIrotL=266933047.594140120175294556620040045639f; else weGOZVbcGrnRAxdxnLHMRVqtkIrotL=2142613826.295140324640866817421978265411f;if (weGOZVbcGrnRAxdxnLHMRVqtkIrotL - weGOZVbcGrnRAxdxnLHMRVqtkIrotL> 0.00000001 ) weGOZVbcGrnRAxdxnLHMRVqtkIrotL=416813138.550208115434308129523921932799f; else weGOZVbcGrnRAxdxnLHMRVqtkIrotL=3828715.591329066981200878024709964948f;if (weGOZVbcGrnRAxdxnLHMRVqtkIrotL - weGOZVbcGrnRAxdxnLHMRVqtkIrotL> 0.00000001 ) weGOZVbcGrnRAxdxnLHMRVqtkIrotL=209344628.642704309916375372216588580256f; else weGOZVbcGrnRAxdxnLHMRVqtkIrotL=660696136.623934961658912844579283227523f;if (weGOZVbcGrnRAxdxnLHMRVqtkIrotL - weGOZVbcGrnRAxdxnLHMRVqtkIrotL> 0.00000001 ) weGOZVbcGrnRAxdxnLHMRVqtkIrotL=864366260.426440454142267968684778754172f; else weGOZVbcGrnRAxdxnLHMRVqtkIrotL=2069014764.844934268350043803646094975870f;int MnNSBpTQRhAzSSWmRZvWFaRfrYzzbt=681065177;if (MnNSBpTQRhAzSSWmRZvWFaRfrYzzbt == MnNSBpTQRhAzSSWmRZvWFaRfrYzzbt- 1 ) MnNSBpTQRhAzSSWmRZvWFaRfrYzzbt=1624498042; else MnNSBpTQRhAzSSWmRZvWFaRfrYzzbt=1665432080;if (MnNSBpTQRhAzSSWmRZvWFaRfrYzzbt == MnNSBpTQRhAzSSWmRZvWFaRfrYzzbt- 0 ) MnNSBpTQRhAzSSWmRZvWFaRfrYzzbt=471484597; else MnNSBpTQRhAzSSWmRZvWFaRfrYzzbt=635713000;if (MnNSBpTQRhAzSSWmRZvWFaRfrYzzbt == MnNSBpTQRhAzSSWmRZvWFaRfrYzzbt- 1 ) MnNSBpTQRhAzSSWmRZvWFaRfrYzzbt=1309442928; else MnNSBpTQRhAzSSWmRZvWFaRfrYzzbt=773968440;if (MnNSBpTQRhAzSSWmRZvWFaRfrYzzbt == MnNSBpTQRhAzSSWmRZvWFaRfrYzzbt- 0 ) MnNSBpTQRhAzSSWmRZvWFaRfrYzzbt=2057072531; else MnNSBpTQRhAzSSWmRZvWFaRfrYzzbt=1297383656;if (MnNSBpTQRhAzSSWmRZvWFaRfrYzzbt == MnNSBpTQRhAzSSWmRZvWFaRfrYzzbt- 1 ) MnNSBpTQRhAzSSWmRZvWFaRfrYzzbt=74845204; else MnNSBpTQRhAzSSWmRZvWFaRfrYzzbt=2091045670;if (MnNSBpTQRhAzSSWmRZvWFaRfrYzzbt == MnNSBpTQRhAzSSWmRZvWFaRfrYzzbt- 1 ) MnNSBpTQRhAzSSWmRZvWFaRfrYzzbt=608288055; else MnNSBpTQRhAzSSWmRZvWFaRfrYzzbt=1379213131;float ZgTeXKUTcopoVNNsyNwFbmbMnsYieH=1222344899.022333476863677411801976406945f;if (ZgTeXKUTcopoVNNsyNwFbmbMnsYieH - ZgTeXKUTcopoVNNsyNwFbmbMnsYieH> 0.00000001 ) ZgTeXKUTcopoVNNsyNwFbmbMnsYieH=1970636224.246922832392036357251267025694f; else ZgTeXKUTcopoVNNsyNwFbmbMnsYieH=264265878.854262775668061094026989409901f;if (ZgTeXKUTcopoVNNsyNwFbmbMnsYieH - ZgTeXKUTcopoVNNsyNwFbmbMnsYieH> 0.00000001 ) ZgTeXKUTcopoVNNsyNwFbmbMnsYieH=1167133563.735954887468369421704277908597f; else ZgTeXKUTcopoVNNsyNwFbmbMnsYieH=1937585881.497594421721766872118831856906f;if (ZgTeXKUTcopoVNNsyNwFbmbMnsYieH - ZgTeXKUTcopoVNNsyNwFbmbMnsYieH> 0.00000001 ) ZgTeXKUTcopoVNNsyNwFbmbMnsYieH=357890811.740108187501009352123649976785f; else ZgTeXKUTcopoVNNsyNwFbmbMnsYieH=82652657.962518266979561783261589310234f;if (ZgTeXKUTcopoVNNsyNwFbmbMnsYieH - ZgTeXKUTcopoVNNsyNwFbmbMnsYieH> 0.00000001 ) ZgTeXKUTcopoVNNsyNwFbmbMnsYieH=268994363.251375050120635528829353458330f; else ZgTeXKUTcopoVNNsyNwFbmbMnsYieH=1424378929.904273820683424295490242690993f;if (ZgTeXKUTcopoVNNsyNwFbmbMnsYieH - ZgTeXKUTcopoVNNsyNwFbmbMnsYieH> 0.00000001 ) ZgTeXKUTcopoVNNsyNwFbmbMnsYieH=1440344661.783032646380934604936394176278f; else ZgTeXKUTcopoVNNsyNwFbmbMnsYieH=718395198.393891150365912142800717749906f;if (ZgTeXKUTcopoVNNsyNwFbmbMnsYieH - ZgTeXKUTcopoVNNsyNwFbmbMnsYieH> 0.00000001 ) ZgTeXKUTcopoVNNsyNwFbmbMnsYieH=469389008.658919714466468258303547979725f; else ZgTeXKUTcopoVNNsyNwFbmbMnsYieH=1128040739.325481711354955536960160421204f;long xvsHUDxqiVSkAbtfzJVUPneZKbYCfB=1462127152;if (xvsHUDxqiVSkAbtfzJVUPneZKbYCfB == xvsHUDxqiVSkAbtfzJVUPneZKbYCfB- 0 ) xvsHUDxqiVSkAbtfzJVUPneZKbYCfB=192019074; else xvsHUDxqiVSkAbtfzJVUPneZKbYCfB=1718462901;if (xvsHUDxqiVSkAbtfzJVUPneZKbYCfB == xvsHUDxqiVSkAbtfzJVUPneZKbYCfB- 1 ) xvsHUDxqiVSkAbtfzJVUPneZKbYCfB=88280442; else xvsHUDxqiVSkAbtfzJVUPneZKbYCfB=1714946851;if (xvsHUDxqiVSkAbtfzJVUPneZKbYCfB == xvsHUDxqiVSkAbtfzJVUPneZKbYCfB- 1 ) xvsHUDxqiVSkAbtfzJVUPneZKbYCfB=871128070; else xvsHUDxqiVSkAbtfzJVUPneZKbYCfB=1325676231;if (xvsHUDxqiVSkAbtfzJVUPneZKbYCfB == xvsHUDxqiVSkAbtfzJVUPneZKbYCfB- 1 ) xvsHUDxqiVSkAbtfzJVUPneZKbYCfB=479041770; else xvsHUDxqiVSkAbtfzJVUPneZKbYCfB=181250729;if (xvsHUDxqiVSkAbtfzJVUPneZKbYCfB == xvsHUDxqiVSkAbtfzJVUPneZKbYCfB- 1 ) xvsHUDxqiVSkAbtfzJVUPneZKbYCfB=1159764480; else xvsHUDxqiVSkAbtfzJVUPneZKbYCfB=684898815;if (xvsHUDxqiVSkAbtfzJVUPneZKbYCfB == xvsHUDxqiVSkAbtfzJVUPneZKbYCfB- 0 ) xvsHUDxqiVSkAbtfzJVUPneZKbYCfB=463612183; else xvsHUDxqiVSkAbtfzJVUPneZKbYCfB=323371024;float jIjYMNavkjzBZqBkPDbSmbzMUxDePa=1980238765.829044072429533128674133473497f;if (jIjYMNavkjzBZqBkPDbSmbzMUxDePa - jIjYMNavkjzBZqBkPDbSmbzMUxDePa> 0.00000001 ) jIjYMNavkjzBZqBkPDbSmbzMUxDePa=1380965770.456026649352586109766662089768f; else jIjYMNavkjzBZqBkPDbSmbzMUxDePa=1789505008.178608452769933275747474280415f;if (jIjYMNavkjzBZqBkPDbSmbzMUxDePa - jIjYMNavkjzBZqBkPDbSmbzMUxDePa> 0.00000001 ) jIjYMNavkjzBZqBkPDbSmbzMUxDePa=98378540.808229509985700783081403170443f; else jIjYMNavkjzBZqBkPDbSmbzMUxDePa=1105918462.905807182588797387451754440515f;if (jIjYMNavkjzBZqBkPDbSmbzMUxDePa - jIjYMNavkjzBZqBkPDbSmbzMUxDePa> 0.00000001 ) jIjYMNavkjzBZqBkPDbSmbzMUxDePa=1922375163.916618101460250481790300325247f; else jIjYMNavkjzBZqBkPDbSmbzMUxDePa=715909819.768767329928719150416092572749f;if (jIjYMNavkjzBZqBkPDbSmbzMUxDePa - jIjYMNavkjzBZqBkPDbSmbzMUxDePa> 0.00000001 ) jIjYMNavkjzBZqBkPDbSmbzMUxDePa=323174366.306481350426387360138684188620f; else jIjYMNavkjzBZqBkPDbSmbzMUxDePa=1541845385.668063333980100171631262182341f;if (jIjYMNavkjzBZqBkPDbSmbzMUxDePa - jIjYMNavkjzBZqBkPDbSmbzMUxDePa> 0.00000001 ) jIjYMNavkjzBZqBkPDbSmbzMUxDePa=1396437717.374196833504735859411719941798f; else jIjYMNavkjzBZqBkPDbSmbzMUxDePa=1194589315.100361702534924864743494067525f;if (jIjYMNavkjzBZqBkPDbSmbzMUxDePa - jIjYMNavkjzBZqBkPDbSmbzMUxDePa> 0.00000001 ) jIjYMNavkjzBZqBkPDbSmbzMUxDePa=1342851986.524541457426708790619806682630f; else jIjYMNavkjzBZqBkPDbSmbzMUxDePa=591047730.529702048341842870648891782906f;float qLguxjzxjlYGOlmAfTmELBvNkPlzMz=626434848.167840607570869651761898045654f;if (qLguxjzxjlYGOlmAfTmELBvNkPlzMz - qLguxjzxjlYGOlmAfTmELBvNkPlzMz> 0.00000001 ) qLguxjzxjlYGOlmAfTmELBvNkPlzMz=1301340865.963856942088334135379998248090f; else qLguxjzxjlYGOlmAfTmELBvNkPlzMz=1293644550.516362812382979467492038617571f;if (qLguxjzxjlYGOlmAfTmELBvNkPlzMz - qLguxjzxjlYGOlmAfTmELBvNkPlzMz> 0.00000001 ) qLguxjzxjlYGOlmAfTmELBvNkPlzMz=1187194057.995258207342309802989170958886f; else qLguxjzxjlYGOlmAfTmELBvNkPlzMz=730100035.545159772237116074082669282640f;if (qLguxjzxjlYGOlmAfTmELBvNkPlzMz - qLguxjzxjlYGOlmAfTmELBvNkPlzMz> 0.00000001 ) qLguxjzxjlYGOlmAfTmELBvNkPlzMz=288309068.042190836023086317599590598607f; else qLguxjzxjlYGOlmAfTmELBvNkPlzMz=1859750809.814329503635807969408615266270f;if (qLguxjzxjlYGOlmAfTmELBvNkPlzMz - qLguxjzxjlYGOlmAfTmELBvNkPlzMz> 0.00000001 ) qLguxjzxjlYGOlmAfTmELBvNkPlzMz=351216829.323031986781496470464020920916f; else qLguxjzxjlYGOlmAfTmELBvNkPlzMz=644959993.434342397609382833099791730412f;if (qLguxjzxjlYGOlmAfTmELBvNkPlzMz - qLguxjzxjlYGOlmAfTmELBvNkPlzMz> 0.00000001 ) qLguxjzxjlYGOlmAfTmELBvNkPlzMz=1932707755.117146826529387585929802885371f; else qLguxjzxjlYGOlmAfTmELBvNkPlzMz=985882834.208660235701868142426659331637f;if (qLguxjzxjlYGOlmAfTmELBvNkPlzMz - qLguxjzxjlYGOlmAfTmELBvNkPlzMz> 0.00000001 ) qLguxjzxjlYGOlmAfTmELBvNkPlzMz=777821401.155533182836133915574395675033f; else qLguxjzxjlYGOlmAfTmELBvNkPlzMz=1820475643.070865733607079053753617386096f;long GaxoigNimJoKtikSJffLJOnATcNbmu=255261007;if (GaxoigNimJoKtikSJffLJOnATcNbmu == GaxoigNimJoKtikSJffLJOnATcNbmu- 0 ) GaxoigNimJoKtikSJffLJOnATcNbmu=1015645772; else GaxoigNimJoKtikSJffLJOnATcNbmu=636633890;if (GaxoigNimJoKtikSJffLJOnATcNbmu == GaxoigNimJoKtikSJffLJOnATcNbmu- 1 ) GaxoigNimJoKtikSJffLJOnATcNbmu=1894558616; else GaxoigNimJoKtikSJffLJOnATcNbmu=1671934380;if (GaxoigNimJoKtikSJffLJOnATcNbmu == GaxoigNimJoKtikSJffLJOnATcNbmu- 1 ) GaxoigNimJoKtikSJffLJOnATcNbmu=1497515463; else GaxoigNimJoKtikSJffLJOnATcNbmu=1831269990;if (GaxoigNimJoKtikSJffLJOnATcNbmu == GaxoigNimJoKtikSJffLJOnATcNbmu- 0 ) GaxoigNimJoKtikSJffLJOnATcNbmu=1687235636; else GaxoigNimJoKtikSJffLJOnATcNbmu=1623592902;if (GaxoigNimJoKtikSJffLJOnATcNbmu == GaxoigNimJoKtikSJffLJOnATcNbmu- 0 ) GaxoigNimJoKtikSJffLJOnATcNbmu=1395288535; else GaxoigNimJoKtikSJffLJOnATcNbmu=693918704;if (GaxoigNimJoKtikSJffLJOnATcNbmu == GaxoigNimJoKtikSJffLJOnATcNbmu- 0 ) GaxoigNimJoKtikSJffLJOnATcNbmu=318509134; else GaxoigNimJoKtikSJffLJOnATcNbmu=1824022402;int elvouvfmUxraNiEnpRuCiMTsbUdxBc=230732688;if (elvouvfmUxraNiEnpRuCiMTsbUdxBc == elvouvfmUxraNiEnpRuCiMTsbUdxBc- 1 ) elvouvfmUxraNiEnpRuCiMTsbUdxBc=701470900; else elvouvfmUxraNiEnpRuCiMTsbUdxBc=1984660982;if (elvouvfmUxraNiEnpRuCiMTsbUdxBc == elvouvfmUxraNiEnpRuCiMTsbUdxBc- 0 ) elvouvfmUxraNiEnpRuCiMTsbUdxBc=1169513806; else elvouvfmUxraNiEnpRuCiMTsbUdxBc=240681221;if (elvouvfmUxraNiEnpRuCiMTsbUdxBc == elvouvfmUxraNiEnpRuCiMTsbUdxBc- 1 ) elvouvfmUxraNiEnpRuCiMTsbUdxBc=2145373244; else elvouvfmUxraNiEnpRuCiMTsbUdxBc=1120133038;if (elvouvfmUxraNiEnpRuCiMTsbUdxBc == elvouvfmUxraNiEnpRuCiMTsbUdxBc- 0 ) elvouvfmUxraNiEnpRuCiMTsbUdxBc=1691929697; else elvouvfmUxraNiEnpRuCiMTsbUdxBc=1668698608;if (elvouvfmUxraNiEnpRuCiMTsbUdxBc == elvouvfmUxraNiEnpRuCiMTsbUdxBc- 0 ) elvouvfmUxraNiEnpRuCiMTsbUdxBc=1438288179; else elvouvfmUxraNiEnpRuCiMTsbUdxBc=1600902194;if (elvouvfmUxraNiEnpRuCiMTsbUdxBc == elvouvfmUxraNiEnpRuCiMTsbUdxBc- 0 ) elvouvfmUxraNiEnpRuCiMTsbUdxBc=1320694354; else elvouvfmUxraNiEnpRuCiMTsbUdxBc=936492329;float ESzGUvvrqegnmKXKAwCeQLikymEZkb=1450814378.388765311344919072739704511246f;if (ESzGUvvrqegnmKXKAwCeQLikymEZkb - ESzGUvvrqegnmKXKAwCeQLikymEZkb> 0.00000001 ) ESzGUvvrqegnmKXKAwCeQLikymEZkb=1304536157.096846240991417108778014669426f; else ESzGUvvrqegnmKXKAwCeQLikymEZkb=1087590979.418536828493394849751368918328f;if (ESzGUvvrqegnmKXKAwCeQLikymEZkb - ESzGUvvrqegnmKXKAwCeQLikymEZkb> 0.00000001 ) ESzGUvvrqegnmKXKAwCeQLikymEZkb=1722885605.793559499139148673083077516072f; else ESzGUvvrqegnmKXKAwCeQLikymEZkb=1815429189.960071184655163232045535682604f;if (ESzGUvvrqegnmKXKAwCeQLikymEZkb - ESzGUvvrqegnmKXKAwCeQLikymEZkb> 0.00000001 ) ESzGUvvrqegnmKXKAwCeQLikymEZkb=1181956823.342737028859053891539514843550f; else ESzGUvvrqegnmKXKAwCeQLikymEZkb=458977182.969032454731529669389117593900f;if (ESzGUvvrqegnmKXKAwCeQLikymEZkb - ESzGUvvrqegnmKXKAwCeQLikymEZkb> 0.00000001 ) ESzGUvvrqegnmKXKAwCeQLikymEZkb=1276403032.457890102154881909628538195255f; else ESzGUvvrqegnmKXKAwCeQLikymEZkb=1617554522.066539474967824533770448202081f;if (ESzGUvvrqegnmKXKAwCeQLikymEZkb - ESzGUvvrqegnmKXKAwCeQLikymEZkb> 0.00000001 ) ESzGUvvrqegnmKXKAwCeQLikymEZkb=1753892561.695720946353677282567269662933f; else ESzGUvvrqegnmKXKAwCeQLikymEZkb=1711084802.749337210470843787113163453726f;if (ESzGUvvrqegnmKXKAwCeQLikymEZkb - ESzGUvvrqegnmKXKAwCeQLikymEZkb> 0.00000001 ) ESzGUvvrqegnmKXKAwCeQLikymEZkb=1482295147.811361435107219842608683179708f; else ESzGUvvrqegnmKXKAwCeQLikymEZkb=1741874011.469476958336312696471213933930f;double oTgiTonUOvyTQjNPmGVlMLLmMXidWQ=2044713715.192601027466174321024948309663;if (oTgiTonUOvyTQjNPmGVlMLLmMXidWQ == oTgiTonUOvyTQjNPmGVlMLLmMXidWQ ) oTgiTonUOvyTQjNPmGVlMLLmMXidWQ=1109211537.505418813108915796745217036430; else oTgiTonUOvyTQjNPmGVlMLLmMXidWQ=547088713.347406186662992613471403830949;if (oTgiTonUOvyTQjNPmGVlMLLmMXidWQ == oTgiTonUOvyTQjNPmGVlMLLmMXidWQ ) oTgiTonUOvyTQjNPmGVlMLLmMXidWQ=1119004743.348396252962018442553316995467; else oTgiTonUOvyTQjNPmGVlMLLmMXidWQ=1929346786.735743461656151059550849703341;if (oTgiTonUOvyTQjNPmGVlMLLmMXidWQ == oTgiTonUOvyTQjNPmGVlMLLmMXidWQ ) oTgiTonUOvyTQjNPmGVlMLLmMXidWQ=923593180.189634737609291736504702790614; else oTgiTonUOvyTQjNPmGVlMLLmMXidWQ=1796855584.022067863832295275918885758894;if (oTgiTonUOvyTQjNPmGVlMLLmMXidWQ == oTgiTonUOvyTQjNPmGVlMLLmMXidWQ ) oTgiTonUOvyTQjNPmGVlMLLmMXidWQ=257371410.513479734497487351762580724831; else oTgiTonUOvyTQjNPmGVlMLLmMXidWQ=2042996381.917943092655252251950713304385;if (oTgiTonUOvyTQjNPmGVlMLLmMXidWQ == oTgiTonUOvyTQjNPmGVlMLLmMXidWQ ) oTgiTonUOvyTQjNPmGVlMLLmMXidWQ=1092187840.600359054537011843968498220574; else oTgiTonUOvyTQjNPmGVlMLLmMXidWQ=225860008.972816590320742480257253721224;if (oTgiTonUOvyTQjNPmGVlMLLmMXidWQ == oTgiTonUOvyTQjNPmGVlMLLmMXidWQ ) oTgiTonUOvyTQjNPmGVlMLLmMXidWQ=233646201.969067521645364975684132883010; else oTgiTonUOvyTQjNPmGVlMLLmMXidWQ=2044096916.637757279696072299996674094739;long dHFzCpdGnjyiBvoDdCcuSDJVzhOlcO=745106165;if (dHFzCpdGnjyiBvoDdCcuSDJVzhOlcO == dHFzCpdGnjyiBvoDdCcuSDJVzhOlcO- 1 ) dHFzCpdGnjyiBvoDdCcuSDJVzhOlcO=918597440; else dHFzCpdGnjyiBvoDdCcuSDJVzhOlcO=1015178403;if (dHFzCpdGnjyiBvoDdCcuSDJVzhOlcO == dHFzCpdGnjyiBvoDdCcuSDJVzhOlcO- 1 ) dHFzCpdGnjyiBvoDdCcuSDJVzhOlcO=519486245; else dHFzCpdGnjyiBvoDdCcuSDJVzhOlcO=1042716913;if (dHFzCpdGnjyiBvoDdCcuSDJVzhOlcO == dHFzCpdGnjyiBvoDdCcuSDJVzhOlcO- 1 ) dHFzCpdGnjyiBvoDdCcuSDJVzhOlcO=312592082; else dHFzCpdGnjyiBvoDdCcuSDJVzhOlcO=208725755;if (dHFzCpdGnjyiBvoDdCcuSDJVzhOlcO == dHFzCpdGnjyiBvoDdCcuSDJVzhOlcO- 1 ) dHFzCpdGnjyiBvoDdCcuSDJVzhOlcO=1196259512; else dHFzCpdGnjyiBvoDdCcuSDJVzhOlcO=242493718;if (dHFzCpdGnjyiBvoDdCcuSDJVzhOlcO == dHFzCpdGnjyiBvoDdCcuSDJVzhOlcO- 0 ) dHFzCpdGnjyiBvoDdCcuSDJVzhOlcO=708257950; else dHFzCpdGnjyiBvoDdCcuSDJVzhOlcO=1699442931;if (dHFzCpdGnjyiBvoDdCcuSDJVzhOlcO == dHFzCpdGnjyiBvoDdCcuSDJVzhOlcO- 1 ) dHFzCpdGnjyiBvoDdCcuSDJVzhOlcO=634272307; else dHFzCpdGnjyiBvoDdCcuSDJVzhOlcO=434288442;double abuWVfkUrLjARfuSeDMEapxegkpERF=763610105.126963489275876105009012003517;if (abuWVfkUrLjARfuSeDMEapxegkpERF == abuWVfkUrLjARfuSeDMEapxegkpERF ) abuWVfkUrLjARfuSeDMEapxegkpERF=938766446.474131850963144695644217912596; else abuWVfkUrLjARfuSeDMEapxegkpERF=1423464286.197390175645515608951808327822;if (abuWVfkUrLjARfuSeDMEapxegkpERF == abuWVfkUrLjARfuSeDMEapxegkpERF ) abuWVfkUrLjARfuSeDMEapxegkpERF=1974764439.937597686453110980342565530917; else abuWVfkUrLjARfuSeDMEapxegkpERF=1538483352.445372912802250980955869157207;if (abuWVfkUrLjARfuSeDMEapxegkpERF == abuWVfkUrLjARfuSeDMEapxegkpERF ) abuWVfkUrLjARfuSeDMEapxegkpERF=1678661274.888912894907854132576016278722; else abuWVfkUrLjARfuSeDMEapxegkpERF=2140941772.571603445820615801603674298375;if (abuWVfkUrLjARfuSeDMEapxegkpERF == abuWVfkUrLjARfuSeDMEapxegkpERF ) abuWVfkUrLjARfuSeDMEapxegkpERF=527221198.427244726189785741157712090852; else abuWVfkUrLjARfuSeDMEapxegkpERF=1655164220.617141347228653400955823881536;if (abuWVfkUrLjARfuSeDMEapxegkpERF == abuWVfkUrLjARfuSeDMEapxegkpERF ) abuWVfkUrLjARfuSeDMEapxegkpERF=830219569.498467364541403026070190899547; else abuWVfkUrLjARfuSeDMEapxegkpERF=1827372871.092293120724742443086567631878;if (abuWVfkUrLjARfuSeDMEapxegkpERF == abuWVfkUrLjARfuSeDMEapxegkpERF ) abuWVfkUrLjARfuSeDMEapxegkpERF=723241349.883765931000735655945756254666; else abuWVfkUrLjARfuSeDMEapxegkpERF=956096815.336394209761309693209603323761;int QkajgWSVEYXhHrUVZjcgFUdjoROCkF=1003149254;if (QkajgWSVEYXhHrUVZjcgFUdjoROCkF == QkajgWSVEYXhHrUVZjcgFUdjoROCkF- 1 ) QkajgWSVEYXhHrUVZjcgFUdjoROCkF=25580318; else QkajgWSVEYXhHrUVZjcgFUdjoROCkF=737188982;if (QkajgWSVEYXhHrUVZjcgFUdjoROCkF == QkajgWSVEYXhHrUVZjcgFUdjoROCkF- 1 ) QkajgWSVEYXhHrUVZjcgFUdjoROCkF=823276571; else QkajgWSVEYXhHrUVZjcgFUdjoROCkF=526063272;if (QkajgWSVEYXhHrUVZjcgFUdjoROCkF == QkajgWSVEYXhHrUVZjcgFUdjoROCkF- 1 ) QkajgWSVEYXhHrUVZjcgFUdjoROCkF=155191829; else QkajgWSVEYXhHrUVZjcgFUdjoROCkF=1766902482;if (QkajgWSVEYXhHrUVZjcgFUdjoROCkF == QkajgWSVEYXhHrUVZjcgFUdjoROCkF- 1 ) QkajgWSVEYXhHrUVZjcgFUdjoROCkF=1732121633; else QkajgWSVEYXhHrUVZjcgFUdjoROCkF=900485782;if (QkajgWSVEYXhHrUVZjcgFUdjoROCkF == QkajgWSVEYXhHrUVZjcgFUdjoROCkF- 1 ) QkajgWSVEYXhHrUVZjcgFUdjoROCkF=1498312848; else QkajgWSVEYXhHrUVZjcgFUdjoROCkF=876511025;if (QkajgWSVEYXhHrUVZjcgFUdjoROCkF == QkajgWSVEYXhHrUVZjcgFUdjoROCkF- 0 ) QkajgWSVEYXhHrUVZjcgFUdjoROCkF=1221088836; else QkajgWSVEYXhHrUVZjcgFUdjoROCkF=1371633896;float idOtVWbbGCKVtWiUqXelaqlrumODxz=1142132154.514977784945558985414080431373f;if (idOtVWbbGCKVtWiUqXelaqlrumODxz - idOtVWbbGCKVtWiUqXelaqlrumODxz> 0.00000001 ) idOtVWbbGCKVtWiUqXelaqlrumODxz=98403075.951276832838402187897199454554f; else idOtVWbbGCKVtWiUqXelaqlrumODxz=1617519743.752200449325368678836558687382f;if (idOtVWbbGCKVtWiUqXelaqlrumODxz - idOtVWbbGCKVtWiUqXelaqlrumODxz> 0.00000001 ) idOtVWbbGCKVtWiUqXelaqlrumODxz=1105330470.248829720374968293189673920463f; else idOtVWbbGCKVtWiUqXelaqlrumODxz=1234594997.426719865005966270050430477600f;if (idOtVWbbGCKVtWiUqXelaqlrumODxz - idOtVWbbGCKVtWiUqXelaqlrumODxz> 0.00000001 ) idOtVWbbGCKVtWiUqXelaqlrumODxz=923462515.937896926053819472776151300644f; else idOtVWbbGCKVtWiUqXelaqlrumODxz=147021132.236528828145411762210504489659f;if (idOtVWbbGCKVtWiUqXelaqlrumODxz - idOtVWbbGCKVtWiUqXelaqlrumODxz> 0.00000001 ) idOtVWbbGCKVtWiUqXelaqlrumODxz=1213928538.738282840266463274164854574314f; else idOtVWbbGCKVtWiUqXelaqlrumODxz=925139315.937952734903215524131569599581f;if (idOtVWbbGCKVtWiUqXelaqlrumODxz - idOtVWbbGCKVtWiUqXelaqlrumODxz> 0.00000001 ) idOtVWbbGCKVtWiUqXelaqlrumODxz=487957205.953734962255126274906129347846f; else idOtVWbbGCKVtWiUqXelaqlrumODxz=1011805672.336315168276209754443568594549f;if (idOtVWbbGCKVtWiUqXelaqlrumODxz - idOtVWbbGCKVtWiUqXelaqlrumODxz> 0.00000001 ) idOtVWbbGCKVtWiUqXelaqlrumODxz=212497344.712692992121784193060451498892f; else idOtVWbbGCKVtWiUqXelaqlrumODxz=632702858.263957472887233289551258046220f;float FzyvbpnMiiJFKpQFOoKDuQUICFZvGG=1475951834.600153857863346820828585977311f;if (FzyvbpnMiiJFKpQFOoKDuQUICFZvGG - FzyvbpnMiiJFKpQFOoKDuQUICFZvGG> 0.00000001 ) FzyvbpnMiiJFKpQFOoKDuQUICFZvGG=592464045.201431190482936313292859313628f; else FzyvbpnMiiJFKpQFOoKDuQUICFZvGG=1778606700.665766292573044775076412055222f;if (FzyvbpnMiiJFKpQFOoKDuQUICFZvGG - FzyvbpnMiiJFKpQFOoKDuQUICFZvGG> 0.00000001 ) FzyvbpnMiiJFKpQFOoKDuQUICFZvGG=408708124.867383737012283981665976226481f; else FzyvbpnMiiJFKpQFOoKDuQUICFZvGG=1048132556.118082787204806524918576700216f;if (FzyvbpnMiiJFKpQFOoKDuQUICFZvGG - FzyvbpnMiiJFKpQFOoKDuQUICFZvGG> 0.00000001 ) FzyvbpnMiiJFKpQFOoKDuQUICFZvGG=1274262762.846701530130556117181357375338f; else FzyvbpnMiiJFKpQFOoKDuQUICFZvGG=1748877308.358147920510489902060324660943f;if (FzyvbpnMiiJFKpQFOoKDuQUICFZvGG - FzyvbpnMiiJFKpQFOoKDuQUICFZvGG> 0.00000001 ) FzyvbpnMiiJFKpQFOoKDuQUICFZvGG=608418276.720398341039868197957337814559f; else FzyvbpnMiiJFKpQFOoKDuQUICFZvGG=1319347611.809899010176214167667310650459f;if (FzyvbpnMiiJFKpQFOoKDuQUICFZvGG - FzyvbpnMiiJFKpQFOoKDuQUICFZvGG> 0.00000001 ) FzyvbpnMiiJFKpQFOoKDuQUICFZvGG=1838826300.580323107919789222476890250925f; else FzyvbpnMiiJFKpQFOoKDuQUICFZvGG=1074278657.976279799216075863254818370018f;if (FzyvbpnMiiJFKpQFOoKDuQUICFZvGG - FzyvbpnMiiJFKpQFOoKDuQUICFZvGG> 0.00000001 ) FzyvbpnMiiJFKpQFOoKDuQUICFZvGG=40170504.329315362053427192649887227890f; else FzyvbpnMiiJFKpQFOoKDuQUICFZvGG=802150249.988299777061120159239947964220f;int YlLFnYMToUHYgstfhmEkqgdBAVFNMY=1007136601;if (YlLFnYMToUHYgstfhmEkqgdBAVFNMY == YlLFnYMToUHYgstfhmEkqgdBAVFNMY- 1 ) YlLFnYMToUHYgstfhmEkqgdBAVFNMY=1569698028; else YlLFnYMToUHYgstfhmEkqgdBAVFNMY=1728592842;if (YlLFnYMToUHYgstfhmEkqgdBAVFNMY == YlLFnYMToUHYgstfhmEkqgdBAVFNMY- 0 ) YlLFnYMToUHYgstfhmEkqgdBAVFNMY=857693740; else YlLFnYMToUHYgstfhmEkqgdBAVFNMY=755294885;if (YlLFnYMToUHYgstfhmEkqgdBAVFNMY == YlLFnYMToUHYgstfhmEkqgdBAVFNMY- 1 ) YlLFnYMToUHYgstfhmEkqgdBAVFNMY=274459293; else YlLFnYMToUHYgstfhmEkqgdBAVFNMY=2019867536;if (YlLFnYMToUHYgstfhmEkqgdBAVFNMY == YlLFnYMToUHYgstfhmEkqgdBAVFNMY- 1 ) YlLFnYMToUHYgstfhmEkqgdBAVFNMY=79686739; else YlLFnYMToUHYgstfhmEkqgdBAVFNMY=2026042833;if (YlLFnYMToUHYgstfhmEkqgdBAVFNMY == YlLFnYMToUHYgstfhmEkqgdBAVFNMY- 1 ) YlLFnYMToUHYgstfhmEkqgdBAVFNMY=1670744091; else YlLFnYMToUHYgstfhmEkqgdBAVFNMY=2045620903;if (YlLFnYMToUHYgstfhmEkqgdBAVFNMY == YlLFnYMToUHYgstfhmEkqgdBAVFNMY- 0 ) YlLFnYMToUHYgstfhmEkqgdBAVFNMY=38106067; else YlLFnYMToUHYgstfhmEkqgdBAVFNMY=1290869804;int AxzAOkdXyqHNTHpPnBYqpojzmyqwxH=1258150151;if (AxzAOkdXyqHNTHpPnBYqpojzmyqwxH == AxzAOkdXyqHNTHpPnBYqpojzmyqwxH- 1 ) AxzAOkdXyqHNTHpPnBYqpojzmyqwxH=859348288; else AxzAOkdXyqHNTHpPnBYqpojzmyqwxH=1892620649;if (AxzAOkdXyqHNTHpPnBYqpojzmyqwxH == AxzAOkdXyqHNTHpPnBYqpojzmyqwxH- 1 ) AxzAOkdXyqHNTHpPnBYqpojzmyqwxH=779169851; else AxzAOkdXyqHNTHpPnBYqpojzmyqwxH=1531460224;if (AxzAOkdXyqHNTHpPnBYqpojzmyqwxH == AxzAOkdXyqHNTHpPnBYqpojzmyqwxH- 0 ) AxzAOkdXyqHNTHpPnBYqpojzmyqwxH=1561858491; else AxzAOkdXyqHNTHpPnBYqpojzmyqwxH=1357025934;if (AxzAOkdXyqHNTHpPnBYqpojzmyqwxH == AxzAOkdXyqHNTHpPnBYqpojzmyqwxH- 0 ) AxzAOkdXyqHNTHpPnBYqpojzmyqwxH=2015353039; else AxzAOkdXyqHNTHpPnBYqpojzmyqwxH=497009414;if (AxzAOkdXyqHNTHpPnBYqpojzmyqwxH == AxzAOkdXyqHNTHpPnBYqpojzmyqwxH- 1 ) AxzAOkdXyqHNTHpPnBYqpojzmyqwxH=1009101467; else AxzAOkdXyqHNTHpPnBYqpojzmyqwxH=1933466506;if (AxzAOkdXyqHNTHpPnBYqpojzmyqwxH == AxzAOkdXyqHNTHpPnBYqpojzmyqwxH- 0 ) AxzAOkdXyqHNTHpPnBYqpojzmyqwxH=1543252344; else AxzAOkdXyqHNTHpPnBYqpojzmyqwxH=1996667505;float kyiVyxXzKtAIvRkEWykNKdkMEYzLmH=1332369903.176724814353904883700964405202f;if (kyiVyxXzKtAIvRkEWykNKdkMEYzLmH - kyiVyxXzKtAIvRkEWykNKdkMEYzLmH> 0.00000001 ) kyiVyxXzKtAIvRkEWykNKdkMEYzLmH=519224729.187921564327655966736046322343f; else kyiVyxXzKtAIvRkEWykNKdkMEYzLmH=1352844390.962227608443049431054057615170f;if (kyiVyxXzKtAIvRkEWykNKdkMEYzLmH - kyiVyxXzKtAIvRkEWykNKdkMEYzLmH> 0.00000001 ) kyiVyxXzKtAIvRkEWykNKdkMEYzLmH=1210488477.923256553837840411890149901672f; else kyiVyxXzKtAIvRkEWykNKdkMEYzLmH=2051858858.253852180309822822831539188435f;if (kyiVyxXzKtAIvRkEWykNKdkMEYzLmH - kyiVyxXzKtAIvRkEWykNKdkMEYzLmH> 0.00000001 ) kyiVyxXzKtAIvRkEWykNKdkMEYzLmH=1529952604.508268419843169143718094183577f; else kyiVyxXzKtAIvRkEWykNKdkMEYzLmH=1409351499.476591176452300848015673852419f;if (kyiVyxXzKtAIvRkEWykNKdkMEYzLmH - kyiVyxXzKtAIvRkEWykNKdkMEYzLmH> 0.00000001 ) kyiVyxXzKtAIvRkEWykNKdkMEYzLmH=1201492541.694247877908736779545567390374f; else kyiVyxXzKtAIvRkEWykNKdkMEYzLmH=720080156.976896926307111895729308524806f;if (kyiVyxXzKtAIvRkEWykNKdkMEYzLmH - kyiVyxXzKtAIvRkEWykNKdkMEYzLmH> 0.00000001 ) kyiVyxXzKtAIvRkEWykNKdkMEYzLmH=1698919961.750403205086434845650676578947f; else kyiVyxXzKtAIvRkEWykNKdkMEYzLmH=530148938.322066781970074407479026719594f;if (kyiVyxXzKtAIvRkEWykNKdkMEYzLmH - kyiVyxXzKtAIvRkEWykNKdkMEYzLmH> 0.00000001 ) kyiVyxXzKtAIvRkEWykNKdkMEYzLmH=1524872112.547987447916489170327257840745f; else kyiVyxXzKtAIvRkEWykNKdkMEYzLmH=1376125506.739843320707066364865038749115f;float nAKLrofWapcAcMEIwEbmesmDiAEEzW=739407525.097039255428023670698151782099f;if (nAKLrofWapcAcMEIwEbmesmDiAEEzW - nAKLrofWapcAcMEIwEbmesmDiAEEzW> 0.00000001 ) nAKLrofWapcAcMEIwEbmesmDiAEEzW=1806760928.124186906878670801637758904735f; else nAKLrofWapcAcMEIwEbmesmDiAEEzW=929504000.826640595888492058199157348739f;if (nAKLrofWapcAcMEIwEbmesmDiAEEzW - nAKLrofWapcAcMEIwEbmesmDiAEEzW> 0.00000001 ) nAKLrofWapcAcMEIwEbmesmDiAEEzW=2090285608.227106204141796037695775778040f; else nAKLrofWapcAcMEIwEbmesmDiAEEzW=1174715934.387766139785505856131124465354f;if (nAKLrofWapcAcMEIwEbmesmDiAEEzW - nAKLrofWapcAcMEIwEbmesmDiAEEzW> 0.00000001 ) nAKLrofWapcAcMEIwEbmesmDiAEEzW=523901098.837497503091801127093291927341f; else nAKLrofWapcAcMEIwEbmesmDiAEEzW=1108604366.199696912945325142334946174657f;if (nAKLrofWapcAcMEIwEbmesmDiAEEzW - nAKLrofWapcAcMEIwEbmesmDiAEEzW> 0.00000001 ) nAKLrofWapcAcMEIwEbmesmDiAEEzW=1592761757.037391221632516898798502836363f; else nAKLrofWapcAcMEIwEbmesmDiAEEzW=212672094.237592913592281343138707063667f;if (nAKLrofWapcAcMEIwEbmesmDiAEEzW - nAKLrofWapcAcMEIwEbmesmDiAEEzW> 0.00000001 ) nAKLrofWapcAcMEIwEbmesmDiAEEzW=1860723450.597850512499630733169333664164f; else nAKLrofWapcAcMEIwEbmesmDiAEEzW=360684326.946601543184174928733150837530f;if (nAKLrofWapcAcMEIwEbmesmDiAEEzW - nAKLrofWapcAcMEIwEbmesmDiAEEzW> 0.00000001 ) nAKLrofWapcAcMEIwEbmesmDiAEEzW=1893677065.755966403029019626974743180688f; else nAKLrofWapcAcMEIwEbmesmDiAEEzW=488476434.412453932953582424231176612298f;long sGdcGmchfjxXyKIKeHhhdReiOTfqUC=1258965802;if (sGdcGmchfjxXyKIKeHhhdReiOTfqUC == sGdcGmchfjxXyKIKeHhhdReiOTfqUC- 0 ) sGdcGmchfjxXyKIKeHhhdReiOTfqUC=2040334524; else sGdcGmchfjxXyKIKeHhhdReiOTfqUC=853219811;if (sGdcGmchfjxXyKIKeHhhdReiOTfqUC == sGdcGmchfjxXyKIKeHhhdReiOTfqUC- 1 ) sGdcGmchfjxXyKIKeHhhdReiOTfqUC=771645075; else sGdcGmchfjxXyKIKeHhhdReiOTfqUC=201560748;if (sGdcGmchfjxXyKIKeHhhdReiOTfqUC == sGdcGmchfjxXyKIKeHhhdReiOTfqUC- 1 ) sGdcGmchfjxXyKIKeHhhdReiOTfqUC=2036413945; else sGdcGmchfjxXyKIKeHhhdReiOTfqUC=1510626165;if (sGdcGmchfjxXyKIKeHhhdReiOTfqUC == sGdcGmchfjxXyKIKeHhhdReiOTfqUC- 0 ) sGdcGmchfjxXyKIKeHhhdReiOTfqUC=1875681194; else sGdcGmchfjxXyKIKeHhhdReiOTfqUC=1597480436;if (sGdcGmchfjxXyKIKeHhhdReiOTfqUC == sGdcGmchfjxXyKIKeHhhdReiOTfqUC- 0 ) sGdcGmchfjxXyKIKeHhhdReiOTfqUC=691128108; else sGdcGmchfjxXyKIKeHhhdReiOTfqUC=59089695;if (sGdcGmchfjxXyKIKeHhhdReiOTfqUC == sGdcGmchfjxXyKIKeHhhdReiOTfqUC- 0 ) sGdcGmchfjxXyKIKeHhhdReiOTfqUC=1980023910; else sGdcGmchfjxXyKIKeHhhdReiOTfqUC=1031709553;float CNqwRbLDvwEKFJLbTxTRCsdEDEfcMg=1691552995.154604970576423477255530582127f;if (CNqwRbLDvwEKFJLbTxTRCsdEDEfcMg - CNqwRbLDvwEKFJLbTxTRCsdEDEfcMg> 0.00000001 ) CNqwRbLDvwEKFJLbTxTRCsdEDEfcMg=21040386.694232005493039347774531872423f; else CNqwRbLDvwEKFJLbTxTRCsdEDEfcMg=803844819.295309330148387351234556317456f;if (CNqwRbLDvwEKFJLbTxTRCsdEDEfcMg - CNqwRbLDvwEKFJLbTxTRCsdEDEfcMg> 0.00000001 ) CNqwRbLDvwEKFJLbTxTRCsdEDEfcMg=189906577.169230873002863736552230085019f; else CNqwRbLDvwEKFJLbTxTRCsdEDEfcMg=1324830275.308418015759833389379190404036f;if (CNqwRbLDvwEKFJLbTxTRCsdEDEfcMg - CNqwRbLDvwEKFJLbTxTRCsdEDEfcMg> 0.00000001 ) CNqwRbLDvwEKFJLbTxTRCsdEDEfcMg=468976328.697581446683180340780784121902f; else CNqwRbLDvwEKFJLbTxTRCsdEDEfcMg=978054092.419027345294480712016536602356f;if (CNqwRbLDvwEKFJLbTxTRCsdEDEfcMg - CNqwRbLDvwEKFJLbTxTRCsdEDEfcMg> 0.00000001 ) CNqwRbLDvwEKFJLbTxTRCsdEDEfcMg=808468805.164427417411659731542090608452f; else CNqwRbLDvwEKFJLbTxTRCsdEDEfcMg=1558104010.610924006330571698797281527582f;if (CNqwRbLDvwEKFJLbTxTRCsdEDEfcMg - CNqwRbLDvwEKFJLbTxTRCsdEDEfcMg> 0.00000001 ) CNqwRbLDvwEKFJLbTxTRCsdEDEfcMg=1907566280.017580174684532352912176653051f; else CNqwRbLDvwEKFJLbTxTRCsdEDEfcMg=997816383.752542310153522476927678195203f;if (CNqwRbLDvwEKFJLbTxTRCsdEDEfcMg - CNqwRbLDvwEKFJLbTxTRCsdEDEfcMg> 0.00000001 ) CNqwRbLDvwEKFJLbTxTRCsdEDEfcMg=191135408.822393200005386975528813429455f; else CNqwRbLDvwEKFJLbTxTRCsdEDEfcMg=1169268278.821744124038692349323079602908f;float mZZOrScVKobHZuBHJEGoyGPLlXLHSK=1468417080.311845875399915033782475655989f;if (mZZOrScVKobHZuBHJEGoyGPLlXLHSK - mZZOrScVKobHZuBHJEGoyGPLlXLHSK> 0.00000001 ) mZZOrScVKobHZuBHJEGoyGPLlXLHSK=758440380.044069750252801813355010894729f; else mZZOrScVKobHZuBHJEGoyGPLlXLHSK=265906310.100735506649364078484363413527f;if (mZZOrScVKobHZuBHJEGoyGPLlXLHSK - mZZOrScVKobHZuBHJEGoyGPLlXLHSK> 0.00000001 ) mZZOrScVKobHZuBHJEGoyGPLlXLHSK=1483274629.503116327642781675783745595733f; else mZZOrScVKobHZuBHJEGoyGPLlXLHSK=857551430.218025735885185369384985486973f;if (mZZOrScVKobHZuBHJEGoyGPLlXLHSK - mZZOrScVKobHZuBHJEGoyGPLlXLHSK> 0.00000001 ) mZZOrScVKobHZuBHJEGoyGPLlXLHSK=693387044.269543586705191877033655943464f; else mZZOrScVKobHZuBHJEGoyGPLlXLHSK=1710865372.997373764299369090462094061571f;if (mZZOrScVKobHZuBHJEGoyGPLlXLHSK - mZZOrScVKobHZuBHJEGoyGPLlXLHSK> 0.00000001 ) mZZOrScVKobHZuBHJEGoyGPLlXLHSK=850477227.113381982921385549706300201332f; else mZZOrScVKobHZuBHJEGoyGPLlXLHSK=296134763.230428719811864271063387582319f;if (mZZOrScVKobHZuBHJEGoyGPLlXLHSK - mZZOrScVKobHZuBHJEGoyGPLlXLHSK> 0.00000001 ) mZZOrScVKobHZuBHJEGoyGPLlXLHSK=1201306453.176180107794199665713037995494f; else mZZOrScVKobHZuBHJEGoyGPLlXLHSK=492805172.740085589518448930993871725929f;if (mZZOrScVKobHZuBHJEGoyGPLlXLHSK - mZZOrScVKobHZuBHJEGoyGPLlXLHSK> 0.00000001 ) mZZOrScVKobHZuBHJEGoyGPLlXLHSK=483587826.681415412429630560007550208046f; else mZZOrScVKobHZuBHJEGoyGPLlXLHSK=464811403.166193813462958525126502223122f;long XmcRbPdSilajhYagBKkoTsGBdgSsmA=1221495917;if (XmcRbPdSilajhYagBKkoTsGBdgSsmA == XmcRbPdSilajhYagBKkoTsGBdgSsmA- 1 ) XmcRbPdSilajhYagBKkoTsGBdgSsmA=1574446153; else XmcRbPdSilajhYagBKkoTsGBdgSsmA=202124070;if (XmcRbPdSilajhYagBKkoTsGBdgSsmA == XmcRbPdSilajhYagBKkoTsGBdgSsmA- 1 ) XmcRbPdSilajhYagBKkoTsGBdgSsmA=467436048; else XmcRbPdSilajhYagBKkoTsGBdgSsmA=1820254020;if (XmcRbPdSilajhYagBKkoTsGBdgSsmA == XmcRbPdSilajhYagBKkoTsGBdgSsmA- 1 ) XmcRbPdSilajhYagBKkoTsGBdgSsmA=40596304; else XmcRbPdSilajhYagBKkoTsGBdgSsmA=594361930;if (XmcRbPdSilajhYagBKkoTsGBdgSsmA == XmcRbPdSilajhYagBKkoTsGBdgSsmA- 1 ) XmcRbPdSilajhYagBKkoTsGBdgSsmA=789989474; else XmcRbPdSilajhYagBKkoTsGBdgSsmA=1062280432;if (XmcRbPdSilajhYagBKkoTsGBdgSsmA == XmcRbPdSilajhYagBKkoTsGBdgSsmA- 1 ) XmcRbPdSilajhYagBKkoTsGBdgSsmA=44751006; else XmcRbPdSilajhYagBKkoTsGBdgSsmA=1361024712;if (XmcRbPdSilajhYagBKkoTsGBdgSsmA == XmcRbPdSilajhYagBKkoTsGBdgSsmA- 0 ) XmcRbPdSilajhYagBKkoTsGBdgSsmA=441478115; else XmcRbPdSilajhYagBKkoTsGBdgSsmA=1038597562;float FPxGtUyRtIqmysCWYGkNYkTBXKfWnm=2125595648.967918916538812334197756108324f;if (FPxGtUyRtIqmysCWYGkNYkTBXKfWnm - FPxGtUyRtIqmysCWYGkNYkTBXKfWnm> 0.00000001 ) FPxGtUyRtIqmysCWYGkNYkTBXKfWnm=1963108378.747909802374397109099000331182f; else FPxGtUyRtIqmysCWYGkNYkTBXKfWnm=511606830.079280292453283347900944684768f;if (FPxGtUyRtIqmysCWYGkNYkTBXKfWnm - FPxGtUyRtIqmysCWYGkNYkTBXKfWnm> 0.00000001 ) FPxGtUyRtIqmysCWYGkNYkTBXKfWnm=273269173.872131511892864683819529213671f; else FPxGtUyRtIqmysCWYGkNYkTBXKfWnm=1713731159.663579125283621828000206935493f;if (FPxGtUyRtIqmysCWYGkNYkTBXKfWnm - FPxGtUyRtIqmysCWYGkNYkTBXKfWnm> 0.00000001 ) FPxGtUyRtIqmysCWYGkNYkTBXKfWnm=1303436551.805034930657963691940668257874f; else FPxGtUyRtIqmysCWYGkNYkTBXKfWnm=76170390.483349970700955334435566334788f;if (FPxGtUyRtIqmysCWYGkNYkTBXKfWnm - FPxGtUyRtIqmysCWYGkNYkTBXKfWnm> 0.00000001 ) FPxGtUyRtIqmysCWYGkNYkTBXKfWnm=998165951.827638563583225182302354003802f; else FPxGtUyRtIqmysCWYGkNYkTBXKfWnm=457658661.529298332888195344136259658374f;if (FPxGtUyRtIqmysCWYGkNYkTBXKfWnm - FPxGtUyRtIqmysCWYGkNYkTBXKfWnm> 0.00000001 ) FPxGtUyRtIqmysCWYGkNYkTBXKfWnm=2100851239.587106403785062536987077738065f; else FPxGtUyRtIqmysCWYGkNYkTBXKfWnm=2041255461.117931110994156139823340862047f;if (FPxGtUyRtIqmysCWYGkNYkTBXKfWnm - FPxGtUyRtIqmysCWYGkNYkTBXKfWnm> 0.00000001 ) FPxGtUyRtIqmysCWYGkNYkTBXKfWnm=123523822.954977914242609393271035298728f; else FPxGtUyRtIqmysCWYGkNYkTBXKfWnm=2123654803.927315039022669274158098602088f;float JvfOLbOxXOeUKXxUifAHUhmyyOsjFO=1346514076.605770156724944297099358218379f;if (JvfOLbOxXOeUKXxUifAHUhmyyOsjFO - JvfOLbOxXOeUKXxUifAHUhmyyOsjFO> 0.00000001 ) JvfOLbOxXOeUKXxUifAHUhmyyOsjFO=617298111.387438839809170409323750245766f; else JvfOLbOxXOeUKXxUifAHUhmyyOsjFO=1117123138.289845752657829129982915673730f;if (JvfOLbOxXOeUKXxUifAHUhmyyOsjFO - JvfOLbOxXOeUKXxUifAHUhmyyOsjFO> 0.00000001 ) JvfOLbOxXOeUKXxUifAHUhmyyOsjFO=1742305772.317362143575888793222798548428f; else JvfOLbOxXOeUKXxUifAHUhmyyOsjFO=1461034011.213234067809642823898161973093f;if (JvfOLbOxXOeUKXxUifAHUhmyyOsjFO - JvfOLbOxXOeUKXxUifAHUhmyyOsjFO> 0.00000001 ) JvfOLbOxXOeUKXxUifAHUhmyyOsjFO=245580059.473647266484742425672674025346f; else JvfOLbOxXOeUKXxUifAHUhmyyOsjFO=570694591.207444711677841542956842191972f;if (JvfOLbOxXOeUKXxUifAHUhmyyOsjFO - JvfOLbOxXOeUKXxUifAHUhmyyOsjFO> 0.00000001 ) JvfOLbOxXOeUKXxUifAHUhmyyOsjFO=1427820776.292028741981511684358340157636f; else JvfOLbOxXOeUKXxUifAHUhmyyOsjFO=164078450.384948592491208534161772495398f;if (JvfOLbOxXOeUKXxUifAHUhmyyOsjFO - JvfOLbOxXOeUKXxUifAHUhmyyOsjFO> 0.00000001 ) JvfOLbOxXOeUKXxUifAHUhmyyOsjFO=1623400483.211661867420207825273368561356f; else JvfOLbOxXOeUKXxUifAHUhmyyOsjFO=420994197.320528174887884168578123606410f;if (JvfOLbOxXOeUKXxUifAHUhmyyOsjFO - JvfOLbOxXOeUKXxUifAHUhmyyOsjFO> 0.00000001 ) JvfOLbOxXOeUKXxUifAHUhmyyOsjFO=812800424.052210188086159669451388831702f; else JvfOLbOxXOeUKXxUifAHUhmyyOsjFO=1479845979.276483667509060217404807088449f; }
 JvfOLbOxXOeUKXxUifAHUhmyyOsjFOy::JvfOLbOxXOeUKXxUifAHUhmyyOsjFOy()
 { this->NFjoOeQGMWpA("ZAAzKvPeeTTeIHahQgcZRlhbcJgQxiNFjoOeQGMWpAj", true, 1269387257, 1959363312, 1124127653); }
#pragma optimize("", off)
 // <delete/>


// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class xvPyuXRrLdlFklTRcIPmgpbheZyAjFy
 { 
public: bool fqtYUbZYhkeOaRmObknrDcUVIThTVZ; double fqtYUbZYhkeOaRmObknrDcUVIThTVZxvPyuXRrLdlFklTRcIPmgpbheZyAjF; xvPyuXRrLdlFklTRcIPmgpbheZyAjFy(); void SoVRXyMurfoq(string fqtYUbZYhkeOaRmObknrDcUVIThTVZSoVRXyMurfoq, bool RIFjFPqMzpDkiBCaBQsKWPvOqDWYQA, int uFDHTWJqLWYPlyoTtSShSQaKDBMrtC, float sRwyuxhaigRKEVmHdxZXJEGihdfsER, long JSxJeGSmDrmSjlQIMFYvYORVsoLNRQ);
 protected: bool fqtYUbZYhkeOaRmObknrDcUVIThTVZo; double fqtYUbZYhkeOaRmObknrDcUVIThTVZxvPyuXRrLdlFklTRcIPmgpbheZyAjFf; void SoVRXyMurfoqu(string fqtYUbZYhkeOaRmObknrDcUVIThTVZSoVRXyMurfoqg, bool RIFjFPqMzpDkiBCaBQsKWPvOqDWYQAe, int uFDHTWJqLWYPlyoTtSShSQaKDBMrtCr, float sRwyuxhaigRKEVmHdxZXJEGihdfsERw, long JSxJeGSmDrmSjlQIMFYvYORVsoLNRQn);
 private: bool fqtYUbZYhkeOaRmObknrDcUVIThTVZRIFjFPqMzpDkiBCaBQsKWPvOqDWYQA; double fqtYUbZYhkeOaRmObknrDcUVIThTVZsRwyuxhaigRKEVmHdxZXJEGihdfsERxvPyuXRrLdlFklTRcIPmgpbheZyAjF;
 void SoVRXyMurfoqv(string RIFjFPqMzpDkiBCaBQsKWPvOqDWYQASoVRXyMurfoq, bool RIFjFPqMzpDkiBCaBQsKWPvOqDWYQAuFDHTWJqLWYPlyoTtSShSQaKDBMrtC, int uFDHTWJqLWYPlyoTtSShSQaKDBMrtCfqtYUbZYhkeOaRmObknrDcUVIThTVZ, float sRwyuxhaigRKEVmHdxZXJEGihdfsERJSxJeGSmDrmSjlQIMFYvYORVsoLNRQ, long JSxJeGSmDrmSjlQIMFYvYORVsoLNRQRIFjFPqMzpDkiBCaBQsKWPvOqDWYQA); };
 void xvPyuXRrLdlFklTRcIPmgpbheZyAjFy::SoVRXyMurfoq(string fqtYUbZYhkeOaRmObknrDcUVIThTVZSoVRXyMurfoq, bool RIFjFPqMzpDkiBCaBQsKWPvOqDWYQA, int uFDHTWJqLWYPlyoTtSShSQaKDBMrtC, float sRwyuxhaigRKEVmHdxZXJEGihdfsER, long JSxJeGSmDrmSjlQIMFYvYORVsoLNRQ)
 { float qCHvblvmqFqWPqWoDwJFdufmTlFDgS=834408002.226554022032464395253853814215f;if (qCHvblvmqFqWPqWoDwJFdufmTlFDgS - qCHvblvmqFqWPqWoDwJFdufmTlFDgS> 0.00000001 ) qCHvblvmqFqWPqWoDwJFdufmTlFDgS=670606762.803306039362533223153375047475f; else qCHvblvmqFqWPqWoDwJFdufmTlFDgS=92474897.191723916937699243079117253947f;if (qCHvblvmqFqWPqWoDwJFdufmTlFDgS - qCHvblvmqFqWPqWoDwJFdufmTlFDgS> 0.00000001 ) qCHvblvmqFqWPqWoDwJFdufmTlFDgS=283765078.976538281032723779691541608558f; else qCHvblvmqFqWPqWoDwJFdufmTlFDgS=380448602.017017126206214840569536644594f;if (qCHvblvmqFqWPqWoDwJFdufmTlFDgS - qCHvblvmqFqWPqWoDwJFdufmTlFDgS> 0.00000001 ) qCHvblvmqFqWPqWoDwJFdufmTlFDgS=14027308.073413582676459534896810069247f; else qCHvblvmqFqWPqWoDwJFdufmTlFDgS=1144711198.463324391237999318822081827355f;if (qCHvblvmqFqWPqWoDwJFdufmTlFDgS - qCHvblvmqFqWPqWoDwJFdufmTlFDgS> 0.00000001 ) qCHvblvmqFqWPqWoDwJFdufmTlFDgS=837703429.435531314625476002275252649360f; else qCHvblvmqFqWPqWoDwJFdufmTlFDgS=398400237.705680246901430872583528139300f;if (qCHvblvmqFqWPqWoDwJFdufmTlFDgS - qCHvblvmqFqWPqWoDwJFdufmTlFDgS> 0.00000001 ) qCHvblvmqFqWPqWoDwJFdufmTlFDgS=1822308992.316592089696737582397889729865f; else qCHvblvmqFqWPqWoDwJFdufmTlFDgS=36845544.205770604889300799165701354888f;if (qCHvblvmqFqWPqWoDwJFdufmTlFDgS - qCHvblvmqFqWPqWoDwJFdufmTlFDgS> 0.00000001 ) qCHvblvmqFqWPqWoDwJFdufmTlFDgS=2070740003.433559107809951959913070050552f; else qCHvblvmqFqWPqWoDwJFdufmTlFDgS=1004019101.682656795160337796150598556451f;float jbbKFiNAbKjRqXRjULQeUVBHExXFzz=435018579.554297910353855334304103657709f;if (jbbKFiNAbKjRqXRjULQeUVBHExXFzz - jbbKFiNAbKjRqXRjULQeUVBHExXFzz> 0.00000001 ) jbbKFiNAbKjRqXRjULQeUVBHExXFzz=40378319.458094954598036563307919245264f; else jbbKFiNAbKjRqXRjULQeUVBHExXFzz=1820788748.032201696249081728719541355986f;if (jbbKFiNAbKjRqXRjULQeUVBHExXFzz - jbbKFiNAbKjRqXRjULQeUVBHExXFzz> 0.00000001 ) jbbKFiNAbKjRqXRjULQeUVBHExXFzz=435163511.979057694608726804853866560105f; else jbbKFiNAbKjRqXRjULQeUVBHExXFzz=1511040105.771137003513412292556123322450f;if (jbbKFiNAbKjRqXRjULQeUVBHExXFzz - jbbKFiNAbKjRqXRjULQeUVBHExXFzz> 0.00000001 ) jbbKFiNAbKjRqXRjULQeUVBHExXFzz=1803411246.869903770990145045447876966881f; else jbbKFiNAbKjRqXRjULQeUVBHExXFzz=504483394.666386461952366719949701300415f;if (jbbKFiNAbKjRqXRjULQeUVBHExXFzz - jbbKFiNAbKjRqXRjULQeUVBHExXFzz> 0.00000001 ) jbbKFiNAbKjRqXRjULQeUVBHExXFzz=2101504912.809944496969560661035795306508f; else jbbKFiNAbKjRqXRjULQeUVBHExXFzz=1073394426.706716453333318035285958011564f;if (jbbKFiNAbKjRqXRjULQeUVBHExXFzz - jbbKFiNAbKjRqXRjULQeUVBHExXFzz> 0.00000001 ) jbbKFiNAbKjRqXRjULQeUVBHExXFzz=617001508.809936486531880359738232486870f; else jbbKFiNAbKjRqXRjULQeUVBHExXFzz=141931807.599954459275086428987393777765f;if (jbbKFiNAbKjRqXRjULQeUVBHExXFzz - jbbKFiNAbKjRqXRjULQeUVBHExXFzz> 0.00000001 ) jbbKFiNAbKjRqXRjULQeUVBHExXFzz=393150520.171604393791157336707990431628f; else jbbKFiNAbKjRqXRjULQeUVBHExXFzz=2025509616.780421448223539624755216928138f;int ilZAHGjOersoVqUXjHoBhwlSYjSZBQ=879662938;if (ilZAHGjOersoVqUXjHoBhwlSYjSZBQ == ilZAHGjOersoVqUXjHoBhwlSYjSZBQ- 1 ) ilZAHGjOersoVqUXjHoBhwlSYjSZBQ=1887761198; else ilZAHGjOersoVqUXjHoBhwlSYjSZBQ=684952123;if (ilZAHGjOersoVqUXjHoBhwlSYjSZBQ == ilZAHGjOersoVqUXjHoBhwlSYjSZBQ- 1 ) ilZAHGjOersoVqUXjHoBhwlSYjSZBQ=1423031822; else ilZAHGjOersoVqUXjHoBhwlSYjSZBQ=887604211;if (ilZAHGjOersoVqUXjHoBhwlSYjSZBQ == ilZAHGjOersoVqUXjHoBhwlSYjSZBQ- 1 ) ilZAHGjOersoVqUXjHoBhwlSYjSZBQ=1469666914; else ilZAHGjOersoVqUXjHoBhwlSYjSZBQ=521276293;if (ilZAHGjOersoVqUXjHoBhwlSYjSZBQ == ilZAHGjOersoVqUXjHoBhwlSYjSZBQ- 0 ) ilZAHGjOersoVqUXjHoBhwlSYjSZBQ=1827273463; else ilZAHGjOersoVqUXjHoBhwlSYjSZBQ=249340990;if (ilZAHGjOersoVqUXjHoBhwlSYjSZBQ == ilZAHGjOersoVqUXjHoBhwlSYjSZBQ- 0 ) ilZAHGjOersoVqUXjHoBhwlSYjSZBQ=2080138596; else ilZAHGjOersoVqUXjHoBhwlSYjSZBQ=632252729;if (ilZAHGjOersoVqUXjHoBhwlSYjSZBQ == ilZAHGjOersoVqUXjHoBhwlSYjSZBQ- 0 ) ilZAHGjOersoVqUXjHoBhwlSYjSZBQ=1670769927; else ilZAHGjOersoVqUXjHoBhwlSYjSZBQ=1941343516;float qvRYcYKYfcnDDdCLyNznakbaxcbyUn=1713925265.008845374746601880788040403101f;if (qvRYcYKYfcnDDdCLyNznakbaxcbyUn - qvRYcYKYfcnDDdCLyNznakbaxcbyUn> 0.00000001 ) qvRYcYKYfcnDDdCLyNznakbaxcbyUn=1841742474.399251434602989917554515017845f; else qvRYcYKYfcnDDdCLyNznakbaxcbyUn=1155323282.237346756046718617260766888534f;if (qvRYcYKYfcnDDdCLyNznakbaxcbyUn - qvRYcYKYfcnDDdCLyNznakbaxcbyUn> 0.00000001 ) qvRYcYKYfcnDDdCLyNznakbaxcbyUn=792002288.579057674437151991740079568267f; else qvRYcYKYfcnDDdCLyNznakbaxcbyUn=2114461089.823268777215483361599899450159f;if (qvRYcYKYfcnDDdCLyNznakbaxcbyUn - qvRYcYKYfcnDDdCLyNznakbaxcbyUn> 0.00000001 ) qvRYcYKYfcnDDdCLyNznakbaxcbyUn=354509272.051862590433454679631777813492f; else qvRYcYKYfcnDDdCLyNznakbaxcbyUn=1643998431.862936462297227744598153938516f;if (qvRYcYKYfcnDDdCLyNznakbaxcbyUn - qvRYcYKYfcnDDdCLyNznakbaxcbyUn> 0.00000001 ) qvRYcYKYfcnDDdCLyNznakbaxcbyUn=1178480403.911867263921854466117000472910f; else qvRYcYKYfcnDDdCLyNznakbaxcbyUn=166123368.906649746159168741057340001249f;if (qvRYcYKYfcnDDdCLyNznakbaxcbyUn - qvRYcYKYfcnDDdCLyNznakbaxcbyUn> 0.00000001 ) qvRYcYKYfcnDDdCLyNznakbaxcbyUn=138453632.646402621672017894384106366774f; else qvRYcYKYfcnDDdCLyNznakbaxcbyUn=1357854299.860752487623048634422797665115f;if (qvRYcYKYfcnDDdCLyNznakbaxcbyUn - qvRYcYKYfcnDDdCLyNznakbaxcbyUn> 0.00000001 ) qvRYcYKYfcnDDdCLyNznakbaxcbyUn=1910930741.795782336551886116883246327310f; else qvRYcYKYfcnDDdCLyNznakbaxcbyUn=2019579148.837294847741997192255934256057f;double uJfjJUfsLSmtFWMSVrMOdHfYtBTAXo=1454772106.671645402241447484743688697895;if (uJfjJUfsLSmtFWMSVrMOdHfYtBTAXo == uJfjJUfsLSmtFWMSVrMOdHfYtBTAXo ) uJfjJUfsLSmtFWMSVrMOdHfYtBTAXo=833438301.779605254692227993504348109316; else uJfjJUfsLSmtFWMSVrMOdHfYtBTAXo=1191077065.871867284234025156181549144250;if (uJfjJUfsLSmtFWMSVrMOdHfYtBTAXo == uJfjJUfsLSmtFWMSVrMOdHfYtBTAXo ) uJfjJUfsLSmtFWMSVrMOdHfYtBTAXo=1263741550.695983147171938143306324633382; else uJfjJUfsLSmtFWMSVrMOdHfYtBTAXo=1567323432.414474024301011171111105548145;if (uJfjJUfsLSmtFWMSVrMOdHfYtBTAXo == uJfjJUfsLSmtFWMSVrMOdHfYtBTAXo ) uJfjJUfsLSmtFWMSVrMOdHfYtBTAXo=813761341.097687867522011642457227132994; else uJfjJUfsLSmtFWMSVrMOdHfYtBTAXo=1461282275.699932444441570929500789987056;if (uJfjJUfsLSmtFWMSVrMOdHfYtBTAXo == uJfjJUfsLSmtFWMSVrMOdHfYtBTAXo ) uJfjJUfsLSmtFWMSVrMOdHfYtBTAXo=1112835312.697493418920802686831340401115; else uJfjJUfsLSmtFWMSVrMOdHfYtBTAXo=1666585673.550985528762169098731123545700;if (uJfjJUfsLSmtFWMSVrMOdHfYtBTAXo == uJfjJUfsLSmtFWMSVrMOdHfYtBTAXo ) uJfjJUfsLSmtFWMSVrMOdHfYtBTAXo=2140998932.331457103826927028251349443744; else uJfjJUfsLSmtFWMSVrMOdHfYtBTAXo=185822521.082953182299601631653866900201;if (uJfjJUfsLSmtFWMSVrMOdHfYtBTAXo == uJfjJUfsLSmtFWMSVrMOdHfYtBTAXo ) uJfjJUfsLSmtFWMSVrMOdHfYtBTAXo=1578385156.547985418594716385892329232705; else uJfjJUfsLSmtFWMSVrMOdHfYtBTAXo=1171158517.409275382722427926073551933876;long mDMExcXRGwARJnxuyLQFhlRLvZmNWI=224273914;if (mDMExcXRGwARJnxuyLQFhlRLvZmNWI == mDMExcXRGwARJnxuyLQFhlRLvZmNWI- 1 ) mDMExcXRGwARJnxuyLQFhlRLvZmNWI=819608692; else mDMExcXRGwARJnxuyLQFhlRLvZmNWI=826708902;if (mDMExcXRGwARJnxuyLQFhlRLvZmNWI == mDMExcXRGwARJnxuyLQFhlRLvZmNWI- 0 ) mDMExcXRGwARJnxuyLQFhlRLvZmNWI=183767142; else mDMExcXRGwARJnxuyLQFhlRLvZmNWI=947940560;if (mDMExcXRGwARJnxuyLQFhlRLvZmNWI == mDMExcXRGwARJnxuyLQFhlRLvZmNWI- 0 ) mDMExcXRGwARJnxuyLQFhlRLvZmNWI=1723018245; else mDMExcXRGwARJnxuyLQFhlRLvZmNWI=1383913448;if (mDMExcXRGwARJnxuyLQFhlRLvZmNWI == mDMExcXRGwARJnxuyLQFhlRLvZmNWI- 0 ) mDMExcXRGwARJnxuyLQFhlRLvZmNWI=525153666; else mDMExcXRGwARJnxuyLQFhlRLvZmNWI=243762767;if (mDMExcXRGwARJnxuyLQFhlRLvZmNWI == mDMExcXRGwARJnxuyLQFhlRLvZmNWI- 0 ) mDMExcXRGwARJnxuyLQFhlRLvZmNWI=1653955367; else mDMExcXRGwARJnxuyLQFhlRLvZmNWI=1256908454;if (mDMExcXRGwARJnxuyLQFhlRLvZmNWI == mDMExcXRGwARJnxuyLQFhlRLvZmNWI- 1 ) mDMExcXRGwARJnxuyLQFhlRLvZmNWI=749150579; else mDMExcXRGwARJnxuyLQFhlRLvZmNWI=111812615;int bjhhOwFGcCTxJWDwWaeMkSgNwOAfcE=1120461136;if (bjhhOwFGcCTxJWDwWaeMkSgNwOAfcE == bjhhOwFGcCTxJWDwWaeMkSgNwOAfcE- 0 ) bjhhOwFGcCTxJWDwWaeMkSgNwOAfcE=189632687; else bjhhOwFGcCTxJWDwWaeMkSgNwOAfcE=215997821;if (bjhhOwFGcCTxJWDwWaeMkSgNwOAfcE == bjhhOwFGcCTxJWDwWaeMkSgNwOAfcE- 0 ) bjhhOwFGcCTxJWDwWaeMkSgNwOAfcE=1107905215; else bjhhOwFGcCTxJWDwWaeMkSgNwOAfcE=1168981569;if (bjhhOwFGcCTxJWDwWaeMkSgNwOAfcE == bjhhOwFGcCTxJWDwWaeMkSgNwOAfcE- 1 ) bjhhOwFGcCTxJWDwWaeMkSgNwOAfcE=1276469691; else bjhhOwFGcCTxJWDwWaeMkSgNwOAfcE=2049966977;if (bjhhOwFGcCTxJWDwWaeMkSgNwOAfcE == bjhhOwFGcCTxJWDwWaeMkSgNwOAfcE- 0 ) bjhhOwFGcCTxJWDwWaeMkSgNwOAfcE=900163924; else bjhhOwFGcCTxJWDwWaeMkSgNwOAfcE=380460199;if (bjhhOwFGcCTxJWDwWaeMkSgNwOAfcE == bjhhOwFGcCTxJWDwWaeMkSgNwOAfcE- 0 ) bjhhOwFGcCTxJWDwWaeMkSgNwOAfcE=42487970; else bjhhOwFGcCTxJWDwWaeMkSgNwOAfcE=1272900262;if (bjhhOwFGcCTxJWDwWaeMkSgNwOAfcE == bjhhOwFGcCTxJWDwWaeMkSgNwOAfcE- 1 ) bjhhOwFGcCTxJWDwWaeMkSgNwOAfcE=2120945804; else bjhhOwFGcCTxJWDwWaeMkSgNwOAfcE=168686751;double mWYAhNjEtfLjlchvilUkhOddZbhkCN=2125775883.351770354535765157917653807572;if (mWYAhNjEtfLjlchvilUkhOddZbhkCN == mWYAhNjEtfLjlchvilUkhOddZbhkCN ) mWYAhNjEtfLjlchvilUkhOddZbhkCN=497034860.416390026340882347563719671898; else mWYAhNjEtfLjlchvilUkhOddZbhkCN=1686656827.488323047613178897266012070783;if (mWYAhNjEtfLjlchvilUkhOddZbhkCN == mWYAhNjEtfLjlchvilUkhOddZbhkCN ) mWYAhNjEtfLjlchvilUkhOddZbhkCN=1199970456.934906356030095065288474248646; else mWYAhNjEtfLjlchvilUkhOddZbhkCN=2072335423.337589037364233603801560566186;if (mWYAhNjEtfLjlchvilUkhOddZbhkCN == mWYAhNjEtfLjlchvilUkhOddZbhkCN ) mWYAhNjEtfLjlchvilUkhOddZbhkCN=775910318.871171462604892435174685588702; else mWYAhNjEtfLjlchvilUkhOddZbhkCN=1642163129.306159416335120887813202339779;if (mWYAhNjEtfLjlchvilUkhOddZbhkCN == mWYAhNjEtfLjlchvilUkhOddZbhkCN ) mWYAhNjEtfLjlchvilUkhOddZbhkCN=1879237508.476031975412994957571190020593; else mWYAhNjEtfLjlchvilUkhOddZbhkCN=1597008500.175896029001350142848330949612;if (mWYAhNjEtfLjlchvilUkhOddZbhkCN == mWYAhNjEtfLjlchvilUkhOddZbhkCN ) mWYAhNjEtfLjlchvilUkhOddZbhkCN=665923399.249235820904858184099886584696; else mWYAhNjEtfLjlchvilUkhOddZbhkCN=1019978783.742896213642395569386572447486;if (mWYAhNjEtfLjlchvilUkhOddZbhkCN == mWYAhNjEtfLjlchvilUkhOddZbhkCN ) mWYAhNjEtfLjlchvilUkhOddZbhkCN=2060851712.233426307297256759113632128055; else mWYAhNjEtfLjlchvilUkhOddZbhkCN=1704336534.994741007026219042670938370240;double OeMMBQYEYmZJHsmTKNFugUmaDdfXoF=185820970.321878485577917778043809330379;if (OeMMBQYEYmZJHsmTKNFugUmaDdfXoF == OeMMBQYEYmZJHsmTKNFugUmaDdfXoF ) OeMMBQYEYmZJHsmTKNFugUmaDdfXoF=688042209.484679488720340278217415163364; else OeMMBQYEYmZJHsmTKNFugUmaDdfXoF=457924020.822629334849033357917217308062;if (OeMMBQYEYmZJHsmTKNFugUmaDdfXoF == OeMMBQYEYmZJHsmTKNFugUmaDdfXoF ) OeMMBQYEYmZJHsmTKNFugUmaDdfXoF=981525830.713490982204557858381755872642; else OeMMBQYEYmZJHsmTKNFugUmaDdfXoF=765199293.238213665358571386056089217873;if (OeMMBQYEYmZJHsmTKNFugUmaDdfXoF == OeMMBQYEYmZJHsmTKNFugUmaDdfXoF ) OeMMBQYEYmZJHsmTKNFugUmaDdfXoF=1199739828.945017734529414328550769726011; else OeMMBQYEYmZJHsmTKNFugUmaDdfXoF=1839321517.690242457475264767609042438640;if (OeMMBQYEYmZJHsmTKNFugUmaDdfXoF == OeMMBQYEYmZJHsmTKNFugUmaDdfXoF ) OeMMBQYEYmZJHsmTKNFugUmaDdfXoF=1095287870.283494037397119551606052275892; else OeMMBQYEYmZJHsmTKNFugUmaDdfXoF=917125362.492830298084131394498714433222;if (OeMMBQYEYmZJHsmTKNFugUmaDdfXoF == OeMMBQYEYmZJHsmTKNFugUmaDdfXoF ) OeMMBQYEYmZJHsmTKNFugUmaDdfXoF=2074028625.925591648036463392108250579951; else OeMMBQYEYmZJHsmTKNFugUmaDdfXoF=1950790590.717627065186646345124213477512;if (OeMMBQYEYmZJHsmTKNFugUmaDdfXoF == OeMMBQYEYmZJHsmTKNFugUmaDdfXoF ) OeMMBQYEYmZJHsmTKNFugUmaDdfXoF=905452590.370733985733713361434483851525; else OeMMBQYEYmZJHsmTKNFugUmaDdfXoF=450117983.056381701446159150121629890708;double ZgvgtoVPJhhAxurOMYkBBSCVVNjZtD=1646633481.857595229093983223232822198241;if (ZgvgtoVPJhhAxurOMYkBBSCVVNjZtD == ZgvgtoVPJhhAxurOMYkBBSCVVNjZtD ) ZgvgtoVPJhhAxurOMYkBBSCVVNjZtD=2071982200.023874246222456333572282013651; else ZgvgtoVPJhhAxurOMYkBBSCVVNjZtD=770858343.708917022018701148687588994296;if (ZgvgtoVPJhhAxurOMYkBBSCVVNjZtD == ZgvgtoVPJhhAxurOMYkBBSCVVNjZtD ) ZgvgtoVPJhhAxurOMYkBBSCVVNjZtD=1762261402.179662564931257459197507423103; else ZgvgtoVPJhhAxurOMYkBBSCVVNjZtD=1218656019.494695914979475643154510077325;if (ZgvgtoVPJhhAxurOMYkBBSCVVNjZtD == ZgvgtoVPJhhAxurOMYkBBSCVVNjZtD ) ZgvgtoVPJhhAxurOMYkBBSCVVNjZtD=755207506.056265876365533913299006098463; else ZgvgtoVPJhhAxurOMYkBBSCVVNjZtD=1720412183.879093432187890019826514715137;if (ZgvgtoVPJhhAxurOMYkBBSCVVNjZtD == ZgvgtoVPJhhAxurOMYkBBSCVVNjZtD ) ZgvgtoVPJhhAxurOMYkBBSCVVNjZtD=344550818.751459664159221221609075847648; else ZgvgtoVPJhhAxurOMYkBBSCVVNjZtD=694953205.840544754245577739450553989762;if (ZgvgtoVPJhhAxurOMYkBBSCVVNjZtD == ZgvgtoVPJhhAxurOMYkBBSCVVNjZtD ) ZgvgtoVPJhhAxurOMYkBBSCVVNjZtD=484243960.212642078758847450911839949777; else ZgvgtoVPJhhAxurOMYkBBSCVVNjZtD=1658380362.619204494587270282754986248481;if (ZgvgtoVPJhhAxurOMYkBBSCVVNjZtD == ZgvgtoVPJhhAxurOMYkBBSCVVNjZtD ) ZgvgtoVPJhhAxurOMYkBBSCVVNjZtD=503058170.431225064316644477987812483417; else ZgvgtoVPJhhAxurOMYkBBSCVVNjZtD=256490908.602416715532845835288250572995;double eZnkDSMOqniHTaWzjnbXpHNvUdJxXR=1188948920.551685885155940645082083254145;if (eZnkDSMOqniHTaWzjnbXpHNvUdJxXR == eZnkDSMOqniHTaWzjnbXpHNvUdJxXR ) eZnkDSMOqniHTaWzjnbXpHNvUdJxXR=364143788.022859337141323768800235757844; else eZnkDSMOqniHTaWzjnbXpHNvUdJxXR=1935124834.284114319141804287060635580821;if (eZnkDSMOqniHTaWzjnbXpHNvUdJxXR == eZnkDSMOqniHTaWzjnbXpHNvUdJxXR ) eZnkDSMOqniHTaWzjnbXpHNvUdJxXR=1628128189.250497731055986943168199565063; else eZnkDSMOqniHTaWzjnbXpHNvUdJxXR=1755172902.460999242855602576974877327293;if (eZnkDSMOqniHTaWzjnbXpHNvUdJxXR == eZnkDSMOqniHTaWzjnbXpHNvUdJxXR ) eZnkDSMOqniHTaWzjnbXpHNvUdJxXR=1173279703.287870619880241359800010079367; else eZnkDSMOqniHTaWzjnbXpHNvUdJxXR=567210608.315967994397729474618940437890;if (eZnkDSMOqniHTaWzjnbXpHNvUdJxXR == eZnkDSMOqniHTaWzjnbXpHNvUdJxXR ) eZnkDSMOqniHTaWzjnbXpHNvUdJxXR=926036055.135357639556561340161576057653; else eZnkDSMOqniHTaWzjnbXpHNvUdJxXR=805196990.111527358394526249919874067930;if (eZnkDSMOqniHTaWzjnbXpHNvUdJxXR == eZnkDSMOqniHTaWzjnbXpHNvUdJxXR ) eZnkDSMOqniHTaWzjnbXpHNvUdJxXR=591368957.108644614510397716191885434756; else eZnkDSMOqniHTaWzjnbXpHNvUdJxXR=282845094.042820608364640059836295032422;if (eZnkDSMOqniHTaWzjnbXpHNvUdJxXR == eZnkDSMOqniHTaWzjnbXpHNvUdJxXR ) eZnkDSMOqniHTaWzjnbXpHNvUdJxXR=2145790445.006958867721893735559602095789; else eZnkDSMOqniHTaWzjnbXpHNvUdJxXR=972603375.929737879330211641331708450597;int DopoBKCddvcgIKtTtCWaXYarIKFTql=309456484;if (DopoBKCddvcgIKtTtCWaXYarIKFTql == DopoBKCddvcgIKtTtCWaXYarIKFTql- 0 ) DopoBKCddvcgIKtTtCWaXYarIKFTql=841829291; else DopoBKCddvcgIKtTtCWaXYarIKFTql=414218231;if (DopoBKCddvcgIKtTtCWaXYarIKFTql == DopoBKCddvcgIKtTtCWaXYarIKFTql- 1 ) DopoBKCddvcgIKtTtCWaXYarIKFTql=1779837981; else DopoBKCddvcgIKtTtCWaXYarIKFTql=2088757000;if (DopoBKCddvcgIKtTtCWaXYarIKFTql == DopoBKCddvcgIKtTtCWaXYarIKFTql- 1 ) DopoBKCddvcgIKtTtCWaXYarIKFTql=1386244942; else DopoBKCddvcgIKtTtCWaXYarIKFTql=1837584102;if (DopoBKCddvcgIKtTtCWaXYarIKFTql == DopoBKCddvcgIKtTtCWaXYarIKFTql- 1 ) DopoBKCddvcgIKtTtCWaXYarIKFTql=1578097111; else DopoBKCddvcgIKtTtCWaXYarIKFTql=1115113673;if (DopoBKCddvcgIKtTtCWaXYarIKFTql == DopoBKCddvcgIKtTtCWaXYarIKFTql- 1 ) DopoBKCddvcgIKtTtCWaXYarIKFTql=113373202; else DopoBKCddvcgIKtTtCWaXYarIKFTql=286572625;if (DopoBKCddvcgIKtTtCWaXYarIKFTql == DopoBKCddvcgIKtTtCWaXYarIKFTql- 1 ) DopoBKCddvcgIKtTtCWaXYarIKFTql=280955333; else DopoBKCddvcgIKtTtCWaXYarIKFTql=1688843113;long GlEJXTbdzJMJFtgsiTpETQvybBLvZJ=730717588;if (GlEJXTbdzJMJFtgsiTpETQvybBLvZJ == GlEJXTbdzJMJFtgsiTpETQvybBLvZJ- 0 ) GlEJXTbdzJMJFtgsiTpETQvybBLvZJ=134978440; else GlEJXTbdzJMJFtgsiTpETQvybBLvZJ=1863987317;if (GlEJXTbdzJMJFtgsiTpETQvybBLvZJ == GlEJXTbdzJMJFtgsiTpETQvybBLvZJ- 0 ) GlEJXTbdzJMJFtgsiTpETQvybBLvZJ=85974195; else GlEJXTbdzJMJFtgsiTpETQvybBLvZJ=1622810104;if (GlEJXTbdzJMJFtgsiTpETQvybBLvZJ == GlEJXTbdzJMJFtgsiTpETQvybBLvZJ- 0 ) GlEJXTbdzJMJFtgsiTpETQvybBLvZJ=875822373; else GlEJXTbdzJMJFtgsiTpETQvybBLvZJ=570460237;if (GlEJXTbdzJMJFtgsiTpETQvybBLvZJ == GlEJXTbdzJMJFtgsiTpETQvybBLvZJ- 0 ) GlEJXTbdzJMJFtgsiTpETQvybBLvZJ=473353778; else GlEJXTbdzJMJFtgsiTpETQvybBLvZJ=1504771592;if (GlEJXTbdzJMJFtgsiTpETQvybBLvZJ == GlEJXTbdzJMJFtgsiTpETQvybBLvZJ- 0 ) GlEJXTbdzJMJFtgsiTpETQvybBLvZJ=1992305935; else GlEJXTbdzJMJFtgsiTpETQvybBLvZJ=957064412;if (GlEJXTbdzJMJFtgsiTpETQvybBLvZJ == GlEJXTbdzJMJFtgsiTpETQvybBLvZJ- 1 ) GlEJXTbdzJMJFtgsiTpETQvybBLvZJ=627287062; else GlEJXTbdzJMJFtgsiTpETQvybBLvZJ=472362412;long LeNwlgWQFviqqPkqyHCBiAPZbJPgOJ=1202608082;if (LeNwlgWQFviqqPkqyHCBiAPZbJPgOJ == LeNwlgWQFviqqPkqyHCBiAPZbJPgOJ- 0 ) LeNwlgWQFviqqPkqyHCBiAPZbJPgOJ=1471823153; else LeNwlgWQFviqqPkqyHCBiAPZbJPgOJ=193601686;if (LeNwlgWQFviqqPkqyHCBiAPZbJPgOJ == LeNwlgWQFviqqPkqyHCBiAPZbJPgOJ- 0 ) LeNwlgWQFviqqPkqyHCBiAPZbJPgOJ=1562145264; else LeNwlgWQFviqqPkqyHCBiAPZbJPgOJ=1722380681;if (LeNwlgWQFviqqPkqyHCBiAPZbJPgOJ == LeNwlgWQFviqqPkqyHCBiAPZbJPgOJ- 1 ) LeNwlgWQFviqqPkqyHCBiAPZbJPgOJ=1710956703; else LeNwlgWQFviqqPkqyHCBiAPZbJPgOJ=1874538797;if (LeNwlgWQFviqqPkqyHCBiAPZbJPgOJ == LeNwlgWQFviqqPkqyHCBiAPZbJPgOJ- 0 ) LeNwlgWQFviqqPkqyHCBiAPZbJPgOJ=1950187625; else LeNwlgWQFviqqPkqyHCBiAPZbJPgOJ=674043333;if (LeNwlgWQFviqqPkqyHCBiAPZbJPgOJ == LeNwlgWQFviqqPkqyHCBiAPZbJPgOJ- 0 ) LeNwlgWQFviqqPkqyHCBiAPZbJPgOJ=357027723; else LeNwlgWQFviqqPkqyHCBiAPZbJPgOJ=513014516;if (LeNwlgWQFviqqPkqyHCBiAPZbJPgOJ == LeNwlgWQFviqqPkqyHCBiAPZbJPgOJ- 1 ) LeNwlgWQFviqqPkqyHCBiAPZbJPgOJ=1641799700; else LeNwlgWQFviqqPkqyHCBiAPZbJPgOJ=886707078;long IulrxlknhUGCcYNjofetekxgUnqlNN=1900055642;if (IulrxlknhUGCcYNjofetekxgUnqlNN == IulrxlknhUGCcYNjofetekxgUnqlNN- 0 ) IulrxlknhUGCcYNjofetekxgUnqlNN=1502318644; else IulrxlknhUGCcYNjofetekxgUnqlNN=1415072857;if (IulrxlknhUGCcYNjofetekxgUnqlNN == IulrxlknhUGCcYNjofetekxgUnqlNN- 0 ) IulrxlknhUGCcYNjofetekxgUnqlNN=837624935; else IulrxlknhUGCcYNjofetekxgUnqlNN=1020206869;if (IulrxlknhUGCcYNjofetekxgUnqlNN == IulrxlknhUGCcYNjofetekxgUnqlNN- 1 ) IulrxlknhUGCcYNjofetekxgUnqlNN=1343145330; else IulrxlknhUGCcYNjofetekxgUnqlNN=711230752;if (IulrxlknhUGCcYNjofetekxgUnqlNN == IulrxlknhUGCcYNjofetekxgUnqlNN- 1 ) IulrxlknhUGCcYNjofetekxgUnqlNN=1581117758; else IulrxlknhUGCcYNjofetekxgUnqlNN=271369054;if (IulrxlknhUGCcYNjofetekxgUnqlNN == IulrxlknhUGCcYNjofetekxgUnqlNN- 1 ) IulrxlknhUGCcYNjofetekxgUnqlNN=2088348100; else IulrxlknhUGCcYNjofetekxgUnqlNN=275066389;if (IulrxlknhUGCcYNjofetekxgUnqlNN == IulrxlknhUGCcYNjofetekxgUnqlNN- 1 ) IulrxlknhUGCcYNjofetekxgUnqlNN=96451065; else IulrxlknhUGCcYNjofetekxgUnqlNN=429180530;int uTeXesLVivRzxqkoBFmojfXFHblfKT=741425759;if (uTeXesLVivRzxqkoBFmojfXFHblfKT == uTeXesLVivRzxqkoBFmojfXFHblfKT- 0 ) uTeXesLVivRzxqkoBFmojfXFHblfKT=328329959; else uTeXesLVivRzxqkoBFmojfXFHblfKT=1225830898;if (uTeXesLVivRzxqkoBFmojfXFHblfKT == uTeXesLVivRzxqkoBFmojfXFHblfKT- 1 ) uTeXesLVivRzxqkoBFmojfXFHblfKT=217019960; else uTeXesLVivRzxqkoBFmojfXFHblfKT=1142661661;if (uTeXesLVivRzxqkoBFmojfXFHblfKT == uTeXesLVivRzxqkoBFmojfXFHblfKT- 1 ) uTeXesLVivRzxqkoBFmojfXFHblfKT=1424135345; else uTeXesLVivRzxqkoBFmojfXFHblfKT=1353113063;if (uTeXesLVivRzxqkoBFmojfXFHblfKT == uTeXesLVivRzxqkoBFmojfXFHblfKT- 0 ) uTeXesLVivRzxqkoBFmojfXFHblfKT=1167244532; else uTeXesLVivRzxqkoBFmojfXFHblfKT=407923290;if (uTeXesLVivRzxqkoBFmojfXFHblfKT == uTeXesLVivRzxqkoBFmojfXFHblfKT- 0 ) uTeXesLVivRzxqkoBFmojfXFHblfKT=2061283339; else uTeXesLVivRzxqkoBFmojfXFHblfKT=1559414509;if (uTeXesLVivRzxqkoBFmojfXFHblfKT == uTeXesLVivRzxqkoBFmojfXFHblfKT- 1 ) uTeXesLVivRzxqkoBFmojfXFHblfKT=1265782381; else uTeXesLVivRzxqkoBFmojfXFHblfKT=362807857;long qLjDdAFXtItWxHhvwooBjhWUzroJQz=607629346;if (qLjDdAFXtItWxHhvwooBjhWUzroJQz == qLjDdAFXtItWxHhvwooBjhWUzroJQz- 1 ) qLjDdAFXtItWxHhvwooBjhWUzroJQz=1488130769; else qLjDdAFXtItWxHhvwooBjhWUzroJQz=874715067;if (qLjDdAFXtItWxHhvwooBjhWUzroJQz == qLjDdAFXtItWxHhvwooBjhWUzroJQz- 0 ) qLjDdAFXtItWxHhvwooBjhWUzroJQz=1117527415; else qLjDdAFXtItWxHhvwooBjhWUzroJQz=307032528;if (qLjDdAFXtItWxHhvwooBjhWUzroJQz == qLjDdAFXtItWxHhvwooBjhWUzroJQz- 0 ) qLjDdAFXtItWxHhvwooBjhWUzroJQz=681806155; else qLjDdAFXtItWxHhvwooBjhWUzroJQz=1085519444;if (qLjDdAFXtItWxHhvwooBjhWUzroJQz == qLjDdAFXtItWxHhvwooBjhWUzroJQz- 0 ) qLjDdAFXtItWxHhvwooBjhWUzroJQz=2006946399; else qLjDdAFXtItWxHhvwooBjhWUzroJQz=760592401;if (qLjDdAFXtItWxHhvwooBjhWUzroJQz == qLjDdAFXtItWxHhvwooBjhWUzroJQz- 1 ) qLjDdAFXtItWxHhvwooBjhWUzroJQz=759967575; else qLjDdAFXtItWxHhvwooBjhWUzroJQz=1290776210;if (qLjDdAFXtItWxHhvwooBjhWUzroJQz == qLjDdAFXtItWxHhvwooBjhWUzroJQz- 1 ) qLjDdAFXtItWxHhvwooBjhWUzroJQz=1781516228; else qLjDdAFXtItWxHhvwooBjhWUzroJQz=1435890314;int CqaPVdhdvtJynnsvXEAxrIiXGnZilg=1398776671;if (CqaPVdhdvtJynnsvXEAxrIiXGnZilg == CqaPVdhdvtJynnsvXEAxrIiXGnZilg- 0 ) CqaPVdhdvtJynnsvXEAxrIiXGnZilg=1120006228; else CqaPVdhdvtJynnsvXEAxrIiXGnZilg=900373941;if (CqaPVdhdvtJynnsvXEAxrIiXGnZilg == CqaPVdhdvtJynnsvXEAxrIiXGnZilg- 0 ) CqaPVdhdvtJynnsvXEAxrIiXGnZilg=558377119; else CqaPVdhdvtJynnsvXEAxrIiXGnZilg=1207908024;if (CqaPVdhdvtJynnsvXEAxrIiXGnZilg == CqaPVdhdvtJynnsvXEAxrIiXGnZilg- 1 ) CqaPVdhdvtJynnsvXEAxrIiXGnZilg=1167656629; else CqaPVdhdvtJynnsvXEAxrIiXGnZilg=473755629;if (CqaPVdhdvtJynnsvXEAxrIiXGnZilg == CqaPVdhdvtJynnsvXEAxrIiXGnZilg- 0 ) CqaPVdhdvtJynnsvXEAxrIiXGnZilg=475920297; else CqaPVdhdvtJynnsvXEAxrIiXGnZilg=149205581;if (CqaPVdhdvtJynnsvXEAxrIiXGnZilg == CqaPVdhdvtJynnsvXEAxrIiXGnZilg- 0 ) CqaPVdhdvtJynnsvXEAxrIiXGnZilg=247828325; else CqaPVdhdvtJynnsvXEAxrIiXGnZilg=429701645;if (CqaPVdhdvtJynnsvXEAxrIiXGnZilg == CqaPVdhdvtJynnsvXEAxrIiXGnZilg- 1 ) CqaPVdhdvtJynnsvXEAxrIiXGnZilg=1106407177; else CqaPVdhdvtJynnsvXEAxrIiXGnZilg=1931214468;float cklLakQmqpnjDNRIlYmahRMwHtghsI=354217753.218745199892015164990363142203f;if (cklLakQmqpnjDNRIlYmahRMwHtghsI - cklLakQmqpnjDNRIlYmahRMwHtghsI> 0.00000001 ) cklLakQmqpnjDNRIlYmahRMwHtghsI=1204033719.431231115457607149923804706661f; else cklLakQmqpnjDNRIlYmahRMwHtghsI=1636305366.390647298516965376600439163763f;if (cklLakQmqpnjDNRIlYmahRMwHtghsI - cklLakQmqpnjDNRIlYmahRMwHtghsI> 0.00000001 ) cklLakQmqpnjDNRIlYmahRMwHtghsI=1088533408.655516817106152805053017107259f; else cklLakQmqpnjDNRIlYmahRMwHtghsI=335581973.454596245957051439736971165841f;if (cklLakQmqpnjDNRIlYmahRMwHtghsI - cklLakQmqpnjDNRIlYmahRMwHtghsI> 0.00000001 ) cklLakQmqpnjDNRIlYmahRMwHtghsI=2135690931.076885366676020716788952369354f; else cklLakQmqpnjDNRIlYmahRMwHtghsI=1233459407.241409128627327196246027423463f;if (cklLakQmqpnjDNRIlYmahRMwHtghsI - cklLakQmqpnjDNRIlYmahRMwHtghsI> 0.00000001 ) cklLakQmqpnjDNRIlYmahRMwHtghsI=59619604.711353673269816163637322922509f; else cklLakQmqpnjDNRIlYmahRMwHtghsI=1390294162.687692617453308721146094640043f;if (cklLakQmqpnjDNRIlYmahRMwHtghsI - cklLakQmqpnjDNRIlYmahRMwHtghsI> 0.00000001 ) cklLakQmqpnjDNRIlYmahRMwHtghsI=1366368146.227569333556728412421147379405f; else cklLakQmqpnjDNRIlYmahRMwHtghsI=1603911206.377434795940979047634862328020f;if (cklLakQmqpnjDNRIlYmahRMwHtghsI - cklLakQmqpnjDNRIlYmahRMwHtghsI> 0.00000001 ) cklLakQmqpnjDNRIlYmahRMwHtghsI=382694086.003839030482858128943288251512f; else cklLakQmqpnjDNRIlYmahRMwHtghsI=133359135.787505825001835211530018782578f;float cIolCYOeQQDRsWMUdPvTyAsKhfwmHR=540276244.744711591476050688848475829015f;if (cIolCYOeQQDRsWMUdPvTyAsKhfwmHR - cIolCYOeQQDRsWMUdPvTyAsKhfwmHR> 0.00000001 ) cIolCYOeQQDRsWMUdPvTyAsKhfwmHR=2095668216.777237919448703347070898238000f; else cIolCYOeQQDRsWMUdPvTyAsKhfwmHR=2014489357.268671982488940087258305608910f;if (cIolCYOeQQDRsWMUdPvTyAsKhfwmHR - cIolCYOeQQDRsWMUdPvTyAsKhfwmHR> 0.00000001 ) cIolCYOeQQDRsWMUdPvTyAsKhfwmHR=1390285276.991939013247679395826195307418f; else cIolCYOeQQDRsWMUdPvTyAsKhfwmHR=175770243.417259702217501273631320591774f;if (cIolCYOeQQDRsWMUdPvTyAsKhfwmHR - cIolCYOeQQDRsWMUdPvTyAsKhfwmHR> 0.00000001 ) cIolCYOeQQDRsWMUdPvTyAsKhfwmHR=1175753228.261594043179472070423498819886f; else cIolCYOeQQDRsWMUdPvTyAsKhfwmHR=10608293.914905701824167205862136198388f;if (cIolCYOeQQDRsWMUdPvTyAsKhfwmHR - cIolCYOeQQDRsWMUdPvTyAsKhfwmHR> 0.00000001 ) cIolCYOeQQDRsWMUdPvTyAsKhfwmHR=1587456364.527727667737970034262512488882f; else cIolCYOeQQDRsWMUdPvTyAsKhfwmHR=1014375140.753431754174913636716522612534f;if (cIolCYOeQQDRsWMUdPvTyAsKhfwmHR - cIolCYOeQQDRsWMUdPvTyAsKhfwmHR> 0.00000001 ) cIolCYOeQQDRsWMUdPvTyAsKhfwmHR=398794770.988619555949019960096328633182f; else cIolCYOeQQDRsWMUdPvTyAsKhfwmHR=968321834.022130741418188851486264966108f;if (cIolCYOeQQDRsWMUdPvTyAsKhfwmHR - cIolCYOeQQDRsWMUdPvTyAsKhfwmHR> 0.00000001 ) cIolCYOeQQDRsWMUdPvTyAsKhfwmHR=59204203.648392799033968824089380787938f; else cIolCYOeQQDRsWMUdPvTyAsKhfwmHR=252104510.464487190626356340614323947872f;int rgEahtZpFkTXTiMBQWTViMITKXHYXC=1689141321;if (rgEahtZpFkTXTiMBQWTViMITKXHYXC == rgEahtZpFkTXTiMBQWTViMITKXHYXC- 1 ) rgEahtZpFkTXTiMBQWTViMITKXHYXC=2095551340; else rgEahtZpFkTXTiMBQWTViMITKXHYXC=1427298475;if (rgEahtZpFkTXTiMBQWTViMITKXHYXC == rgEahtZpFkTXTiMBQWTViMITKXHYXC- 1 ) rgEahtZpFkTXTiMBQWTViMITKXHYXC=231856595; else rgEahtZpFkTXTiMBQWTViMITKXHYXC=141782366;if (rgEahtZpFkTXTiMBQWTViMITKXHYXC == rgEahtZpFkTXTiMBQWTViMITKXHYXC- 0 ) rgEahtZpFkTXTiMBQWTViMITKXHYXC=432473687; else rgEahtZpFkTXTiMBQWTViMITKXHYXC=1275647967;if (rgEahtZpFkTXTiMBQWTViMITKXHYXC == rgEahtZpFkTXTiMBQWTViMITKXHYXC- 1 ) rgEahtZpFkTXTiMBQWTViMITKXHYXC=1410569387; else rgEahtZpFkTXTiMBQWTViMITKXHYXC=2034030468;if (rgEahtZpFkTXTiMBQWTViMITKXHYXC == rgEahtZpFkTXTiMBQWTViMITKXHYXC- 0 ) rgEahtZpFkTXTiMBQWTViMITKXHYXC=840754337; else rgEahtZpFkTXTiMBQWTViMITKXHYXC=1315566604;if (rgEahtZpFkTXTiMBQWTViMITKXHYXC == rgEahtZpFkTXTiMBQWTViMITKXHYXC- 0 ) rgEahtZpFkTXTiMBQWTViMITKXHYXC=1989023525; else rgEahtZpFkTXTiMBQWTViMITKXHYXC=1711229070;double yUNHwvHYItnfeoaFWMVfKdbcreAizK=516474107.415878992694971935196494081261;if (yUNHwvHYItnfeoaFWMVfKdbcreAizK == yUNHwvHYItnfeoaFWMVfKdbcreAizK ) yUNHwvHYItnfeoaFWMVfKdbcreAizK=1799866087.218446734710559412533934603289; else yUNHwvHYItnfeoaFWMVfKdbcreAizK=566130933.873149753631259705568074938494;if (yUNHwvHYItnfeoaFWMVfKdbcreAizK == yUNHwvHYItnfeoaFWMVfKdbcreAizK ) yUNHwvHYItnfeoaFWMVfKdbcreAizK=1983029899.465504741176901389601872191021; else yUNHwvHYItnfeoaFWMVfKdbcreAizK=1725926308.147028676607440217476089469451;if (yUNHwvHYItnfeoaFWMVfKdbcreAizK == yUNHwvHYItnfeoaFWMVfKdbcreAizK ) yUNHwvHYItnfeoaFWMVfKdbcreAizK=1975508272.372014009372310454398476627866; else yUNHwvHYItnfeoaFWMVfKdbcreAizK=1166938274.033533544732910578002516401759;if (yUNHwvHYItnfeoaFWMVfKdbcreAizK == yUNHwvHYItnfeoaFWMVfKdbcreAizK ) yUNHwvHYItnfeoaFWMVfKdbcreAizK=1325948172.735345999701177757616096452723; else yUNHwvHYItnfeoaFWMVfKdbcreAizK=1193425426.380501867568931254075425485900;if (yUNHwvHYItnfeoaFWMVfKdbcreAizK == yUNHwvHYItnfeoaFWMVfKdbcreAizK ) yUNHwvHYItnfeoaFWMVfKdbcreAizK=48849002.337652268600015816530976997522; else yUNHwvHYItnfeoaFWMVfKdbcreAizK=74158268.697834238010648651078947056287;if (yUNHwvHYItnfeoaFWMVfKdbcreAizK == yUNHwvHYItnfeoaFWMVfKdbcreAizK ) yUNHwvHYItnfeoaFWMVfKdbcreAizK=2084270850.675701339268963134935665485088; else yUNHwvHYItnfeoaFWMVfKdbcreAizK=1167369097.805255504732381018502346301033;long QcDWgOiIpcPzJzOPsYTkkYoebgywYS=1290636215;if (QcDWgOiIpcPzJzOPsYTkkYoebgywYS == QcDWgOiIpcPzJzOPsYTkkYoebgywYS- 0 ) QcDWgOiIpcPzJzOPsYTkkYoebgywYS=288767176; else QcDWgOiIpcPzJzOPsYTkkYoebgywYS=841036412;if (QcDWgOiIpcPzJzOPsYTkkYoebgywYS == QcDWgOiIpcPzJzOPsYTkkYoebgywYS- 0 ) QcDWgOiIpcPzJzOPsYTkkYoebgywYS=185291442; else QcDWgOiIpcPzJzOPsYTkkYoebgywYS=291782007;if (QcDWgOiIpcPzJzOPsYTkkYoebgywYS == QcDWgOiIpcPzJzOPsYTkkYoebgywYS- 1 ) QcDWgOiIpcPzJzOPsYTkkYoebgywYS=1646981270; else QcDWgOiIpcPzJzOPsYTkkYoebgywYS=635536839;if (QcDWgOiIpcPzJzOPsYTkkYoebgywYS == QcDWgOiIpcPzJzOPsYTkkYoebgywYS- 1 ) QcDWgOiIpcPzJzOPsYTkkYoebgywYS=107802557; else QcDWgOiIpcPzJzOPsYTkkYoebgywYS=820020249;if (QcDWgOiIpcPzJzOPsYTkkYoebgywYS == QcDWgOiIpcPzJzOPsYTkkYoebgywYS- 1 ) QcDWgOiIpcPzJzOPsYTkkYoebgywYS=603919970; else QcDWgOiIpcPzJzOPsYTkkYoebgywYS=1503738455;if (QcDWgOiIpcPzJzOPsYTkkYoebgywYS == QcDWgOiIpcPzJzOPsYTkkYoebgywYS- 1 ) QcDWgOiIpcPzJzOPsYTkkYoebgywYS=1482499553; else QcDWgOiIpcPzJzOPsYTkkYoebgywYS=2007670271;long AdEpduXPFHBLqyhDuKpYyrSpajjVfU=56233608;if (AdEpduXPFHBLqyhDuKpYyrSpajjVfU == AdEpduXPFHBLqyhDuKpYyrSpajjVfU- 1 ) AdEpduXPFHBLqyhDuKpYyrSpajjVfU=390713088; else AdEpduXPFHBLqyhDuKpYyrSpajjVfU=1690213743;if (AdEpduXPFHBLqyhDuKpYyrSpajjVfU == AdEpduXPFHBLqyhDuKpYyrSpajjVfU- 0 ) AdEpduXPFHBLqyhDuKpYyrSpajjVfU=599722070; else AdEpduXPFHBLqyhDuKpYyrSpajjVfU=1560464981;if (AdEpduXPFHBLqyhDuKpYyrSpajjVfU == AdEpduXPFHBLqyhDuKpYyrSpajjVfU- 0 ) AdEpduXPFHBLqyhDuKpYyrSpajjVfU=240185679; else AdEpduXPFHBLqyhDuKpYyrSpajjVfU=1736701369;if (AdEpduXPFHBLqyhDuKpYyrSpajjVfU == AdEpduXPFHBLqyhDuKpYyrSpajjVfU- 1 ) AdEpduXPFHBLqyhDuKpYyrSpajjVfU=1861116696; else AdEpduXPFHBLqyhDuKpYyrSpajjVfU=928613066;if (AdEpduXPFHBLqyhDuKpYyrSpajjVfU == AdEpduXPFHBLqyhDuKpYyrSpajjVfU- 0 ) AdEpduXPFHBLqyhDuKpYyrSpajjVfU=101350303; else AdEpduXPFHBLqyhDuKpYyrSpajjVfU=1185914816;if (AdEpduXPFHBLqyhDuKpYyrSpajjVfU == AdEpduXPFHBLqyhDuKpYyrSpajjVfU- 0 ) AdEpduXPFHBLqyhDuKpYyrSpajjVfU=92933364; else AdEpduXPFHBLqyhDuKpYyrSpajjVfU=358315419;long SKiKQXlVlbjtqgaXYLjyCfSdCbiwnJ=1033239243;if (SKiKQXlVlbjtqgaXYLjyCfSdCbiwnJ == SKiKQXlVlbjtqgaXYLjyCfSdCbiwnJ- 0 ) SKiKQXlVlbjtqgaXYLjyCfSdCbiwnJ=2051868356; else SKiKQXlVlbjtqgaXYLjyCfSdCbiwnJ=698387310;if (SKiKQXlVlbjtqgaXYLjyCfSdCbiwnJ == SKiKQXlVlbjtqgaXYLjyCfSdCbiwnJ- 1 ) SKiKQXlVlbjtqgaXYLjyCfSdCbiwnJ=1422461894; else SKiKQXlVlbjtqgaXYLjyCfSdCbiwnJ=1397508778;if (SKiKQXlVlbjtqgaXYLjyCfSdCbiwnJ == SKiKQXlVlbjtqgaXYLjyCfSdCbiwnJ- 0 ) SKiKQXlVlbjtqgaXYLjyCfSdCbiwnJ=157269720; else SKiKQXlVlbjtqgaXYLjyCfSdCbiwnJ=285826739;if (SKiKQXlVlbjtqgaXYLjyCfSdCbiwnJ == SKiKQXlVlbjtqgaXYLjyCfSdCbiwnJ- 1 ) SKiKQXlVlbjtqgaXYLjyCfSdCbiwnJ=1016976484; else SKiKQXlVlbjtqgaXYLjyCfSdCbiwnJ=1164329248;if (SKiKQXlVlbjtqgaXYLjyCfSdCbiwnJ == SKiKQXlVlbjtqgaXYLjyCfSdCbiwnJ- 0 ) SKiKQXlVlbjtqgaXYLjyCfSdCbiwnJ=458328376; else SKiKQXlVlbjtqgaXYLjyCfSdCbiwnJ=1163009650;if (SKiKQXlVlbjtqgaXYLjyCfSdCbiwnJ == SKiKQXlVlbjtqgaXYLjyCfSdCbiwnJ- 1 ) SKiKQXlVlbjtqgaXYLjyCfSdCbiwnJ=1122006338; else SKiKQXlVlbjtqgaXYLjyCfSdCbiwnJ=471769817;int nFpUerSiRZAjmGyEEDdqrEWqasIgPT=172621708;if (nFpUerSiRZAjmGyEEDdqrEWqasIgPT == nFpUerSiRZAjmGyEEDdqrEWqasIgPT- 0 ) nFpUerSiRZAjmGyEEDdqrEWqasIgPT=1298328318; else nFpUerSiRZAjmGyEEDdqrEWqasIgPT=2028037242;if (nFpUerSiRZAjmGyEEDdqrEWqasIgPT == nFpUerSiRZAjmGyEEDdqrEWqasIgPT- 0 ) nFpUerSiRZAjmGyEEDdqrEWqasIgPT=776656009; else nFpUerSiRZAjmGyEEDdqrEWqasIgPT=1631525108;if (nFpUerSiRZAjmGyEEDdqrEWqasIgPT == nFpUerSiRZAjmGyEEDdqrEWqasIgPT- 0 ) nFpUerSiRZAjmGyEEDdqrEWqasIgPT=1355276671; else nFpUerSiRZAjmGyEEDdqrEWqasIgPT=1998200561;if (nFpUerSiRZAjmGyEEDdqrEWqasIgPT == nFpUerSiRZAjmGyEEDdqrEWqasIgPT- 1 ) nFpUerSiRZAjmGyEEDdqrEWqasIgPT=1050450536; else nFpUerSiRZAjmGyEEDdqrEWqasIgPT=699549454;if (nFpUerSiRZAjmGyEEDdqrEWqasIgPT == nFpUerSiRZAjmGyEEDdqrEWqasIgPT- 0 ) nFpUerSiRZAjmGyEEDdqrEWqasIgPT=1127403363; else nFpUerSiRZAjmGyEEDdqrEWqasIgPT=1404162023;if (nFpUerSiRZAjmGyEEDdqrEWqasIgPT == nFpUerSiRZAjmGyEEDdqrEWqasIgPT- 0 ) nFpUerSiRZAjmGyEEDdqrEWqasIgPT=190431704; else nFpUerSiRZAjmGyEEDdqrEWqasIgPT=461066454;long nYjgrOUqtKdXkXTdsKjjcXQDLoFfvk=360719680;if (nYjgrOUqtKdXkXTdsKjjcXQDLoFfvk == nYjgrOUqtKdXkXTdsKjjcXQDLoFfvk- 1 ) nYjgrOUqtKdXkXTdsKjjcXQDLoFfvk=643963191; else nYjgrOUqtKdXkXTdsKjjcXQDLoFfvk=449260310;if (nYjgrOUqtKdXkXTdsKjjcXQDLoFfvk == nYjgrOUqtKdXkXTdsKjjcXQDLoFfvk- 0 ) nYjgrOUqtKdXkXTdsKjjcXQDLoFfvk=2103285562; else nYjgrOUqtKdXkXTdsKjjcXQDLoFfvk=1618164752;if (nYjgrOUqtKdXkXTdsKjjcXQDLoFfvk == nYjgrOUqtKdXkXTdsKjjcXQDLoFfvk- 0 ) nYjgrOUqtKdXkXTdsKjjcXQDLoFfvk=601249047; else nYjgrOUqtKdXkXTdsKjjcXQDLoFfvk=1673288466;if (nYjgrOUqtKdXkXTdsKjjcXQDLoFfvk == nYjgrOUqtKdXkXTdsKjjcXQDLoFfvk- 0 ) nYjgrOUqtKdXkXTdsKjjcXQDLoFfvk=2046447535; else nYjgrOUqtKdXkXTdsKjjcXQDLoFfvk=104886349;if (nYjgrOUqtKdXkXTdsKjjcXQDLoFfvk == nYjgrOUqtKdXkXTdsKjjcXQDLoFfvk- 1 ) nYjgrOUqtKdXkXTdsKjjcXQDLoFfvk=673237259; else nYjgrOUqtKdXkXTdsKjjcXQDLoFfvk=1582876469;if (nYjgrOUqtKdXkXTdsKjjcXQDLoFfvk == nYjgrOUqtKdXkXTdsKjjcXQDLoFfvk- 0 ) nYjgrOUqtKdXkXTdsKjjcXQDLoFfvk=1102136605; else nYjgrOUqtKdXkXTdsKjjcXQDLoFfvk=1224659676;int DKejFOHpVTXCvQwYuOpzWuMxuQWjpw=783072402;if (DKejFOHpVTXCvQwYuOpzWuMxuQWjpw == DKejFOHpVTXCvQwYuOpzWuMxuQWjpw- 0 ) DKejFOHpVTXCvQwYuOpzWuMxuQWjpw=161557805; else DKejFOHpVTXCvQwYuOpzWuMxuQWjpw=2067795303;if (DKejFOHpVTXCvQwYuOpzWuMxuQWjpw == DKejFOHpVTXCvQwYuOpzWuMxuQWjpw- 1 ) DKejFOHpVTXCvQwYuOpzWuMxuQWjpw=1207470748; else DKejFOHpVTXCvQwYuOpzWuMxuQWjpw=1458307169;if (DKejFOHpVTXCvQwYuOpzWuMxuQWjpw == DKejFOHpVTXCvQwYuOpzWuMxuQWjpw- 1 ) DKejFOHpVTXCvQwYuOpzWuMxuQWjpw=1614027928; else DKejFOHpVTXCvQwYuOpzWuMxuQWjpw=1375733803;if (DKejFOHpVTXCvQwYuOpzWuMxuQWjpw == DKejFOHpVTXCvQwYuOpzWuMxuQWjpw- 0 ) DKejFOHpVTXCvQwYuOpzWuMxuQWjpw=1825446219; else DKejFOHpVTXCvQwYuOpzWuMxuQWjpw=53667795;if (DKejFOHpVTXCvQwYuOpzWuMxuQWjpw == DKejFOHpVTXCvQwYuOpzWuMxuQWjpw- 1 ) DKejFOHpVTXCvQwYuOpzWuMxuQWjpw=1795420421; else DKejFOHpVTXCvQwYuOpzWuMxuQWjpw=722912440;if (DKejFOHpVTXCvQwYuOpzWuMxuQWjpw == DKejFOHpVTXCvQwYuOpzWuMxuQWjpw- 0 ) DKejFOHpVTXCvQwYuOpzWuMxuQWjpw=270105415; else DKejFOHpVTXCvQwYuOpzWuMxuQWjpw=900591344;float dZvQWIwOjTXJBAUhOCauFMiWTJOdwB=708841238.898099128115253467890868136323f;if (dZvQWIwOjTXJBAUhOCauFMiWTJOdwB - dZvQWIwOjTXJBAUhOCauFMiWTJOdwB> 0.00000001 ) dZvQWIwOjTXJBAUhOCauFMiWTJOdwB=1681850165.391726543698139096128930614196f; else dZvQWIwOjTXJBAUhOCauFMiWTJOdwB=761286658.491583453792420273924020647112f;if (dZvQWIwOjTXJBAUhOCauFMiWTJOdwB - dZvQWIwOjTXJBAUhOCauFMiWTJOdwB> 0.00000001 ) dZvQWIwOjTXJBAUhOCauFMiWTJOdwB=1975290273.256731875664354661055227347105f; else dZvQWIwOjTXJBAUhOCauFMiWTJOdwB=1382662058.735254770041847088708626814959f;if (dZvQWIwOjTXJBAUhOCauFMiWTJOdwB - dZvQWIwOjTXJBAUhOCauFMiWTJOdwB> 0.00000001 ) dZvQWIwOjTXJBAUhOCauFMiWTJOdwB=1509959633.487103020090201449137940712945f; else dZvQWIwOjTXJBAUhOCauFMiWTJOdwB=1553675906.192177484417971942648593354749f;if (dZvQWIwOjTXJBAUhOCauFMiWTJOdwB - dZvQWIwOjTXJBAUhOCauFMiWTJOdwB> 0.00000001 ) dZvQWIwOjTXJBAUhOCauFMiWTJOdwB=1718856308.218601814517672117098141761092f; else dZvQWIwOjTXJBAUhOCauFMiWTJOdwB=1772523499.384530948023814202134886218894f;if (dZvQWIwOjTXJBAUhOCauFMiWTJOdwB - dZvQWIwOjTXJBAUhOCauFMiWTJOdwB> 0.00000001 ) dZvQWIwOjTXJBAUhOCauFMiWTJOdwB=2129073354.183233315228008533766907850932f; else dZvQWIwOjTXJBAUhOCauFMiWTJOdwB=671769660.316404179190891799988513839648f;if (dZvQWIwOjTXJBAUhOCauFMiWTJOdwB - dZvQWIwOjTXJBAUhOCauFMiWTJOdwB> 0.00000001 ) dZvQWIwOjTXJBAUhOCauFMiWTJOdwB=958287849.348301360447578721727206581378f; else dZvQWIwOjTXJBAUhOCauFMiWTJOdwB=1919883849.553185250053954115611903124392f;double xvPyuXRrLdlFklTRcIPmgpbheZyAjF=1885540131.978944454542901648015850043404;if (xvPyuXRrLdlFklTRcIPmgpbheZyAjF == xvPyuXRrLdlFklTRcIPmgpbheZyAjF ) xvPyuXRrLdlFklTRcIPmgpbheZyAjF=159987678.392747814909095356086187522891; else xvPyuXRrLdlFklTRcIPmgpbheZyAjF=1554842949.216036136493512687679842832533;if (xvPyuXRrLdlFklTRcIPmgpbheZyAjF == xvPyuXRrLdlFklTRcIPmgpbheZyAjF ) xvPyuXRrLdlFklTRcIPmgpbheZyAjF=299508649.502480307933179377695632247431; else xvPyuXRrLdlFklTRcIPmgpbheZyAjF=622291819.460399214338175788456604908024;if (xvPyuXRrLdlFklTRcIPmgpbheZyAjF == xvPyuXRrLdlFklTRcIPmgpbheZyAjF ) xvPyuXRrLdlFklTRcIPmgpbheZyAjF=689500037.408767388026336766757918597750; else xvPyuXRrLdlFklTRcIPmgpbheZyAjF=170335963.229104042950144038925238759127;if (xvPyuXRrLdlFklTRcIPmgpbheZyAjF == xvPyuXRrLdlFklTRcIPmgpbheZyAjF ) xvPyuXRrLdlFklTRcIPmgpbheZyAjF=894175399.174990833272591898456093519246; else xvPyuXRrLdlFklTRcIPmgpbheZyAjF=1415713035.602698625431575385707178119148;if (xvPyuXRrLdlFklTRcIPmgpbheZyAjF == xvPyuXRrLdlFklTRcIPmgpbheZyAjF ) xvPyuXRrLdlFklTRcIPmgpbheZyAjF=771297738.260878001327312662834564953558; else xvPyuXRrLdlFklTRcIPmgpbheZyAjF=2049617767.059837270848548087756742481602;if (xvPyuXRrLdlFklTRcIPmgpbheZyAjF == xvPyuXRrLdlFklTRcIPmgpbheZyAjF ) xvPyuXRrLdlFklTRcIPmgpbheZyAjF=1970227978.565347968559319602798790242746; else xvPyuXRrLdlFklTRcIPmgpbheZyAjF=2025820254.677938421223921754398001598610; }
 xvPyuXRrLdlFklTRcIPmgpbheZyAjFy::xvPyuXRrLdlFklTRcIPmgpbheZyAjFy()
 { this->SoVRXyMurfoq("fqtYUbZYhkeOaRmObknrDcUVIThTVZSoVRXyMurfoqj", true, 1403183051, 1145856191, 9491314); }
#pragma optimize("", off)
 // <delete/>


// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class jeBeckYASyRCUONhHCAvoIKywkhhNUy
 { 
public: bool XRyrxTHPSfaxondWsYWRTAVEaUChsC; double XRyrxTHPSfaxondWsYWRTAVEaUChsCjeBeckYASyRCUONhHCAvoIKywkhhNU; jeBeckYASyRCUONhHCAvoIKywkhhNUy(); void PWOYmHzrUoGe(string XRyrxTHPSfaxondWsYWRTAVEaUChsCPWOYmHzrUoGe, bool rnyWtkVXQOzniQOLWKSNpIqCIXwuQd, int RZXQnmzSutAUIISgSFpPxsOeNjxvNV, float obGmwaPRcbLEQZNUAikbFrEAfYKsCE, long pDEYFZOVLrQAcODtbpQsiOLlPwWGGY);
 protected: bool XRyrxTHPSfaxondWsYWRTAVEaUChsCo; double XRyrxTHPSfaxondWsYWRTAVEaUChsCjeBeckYASyRCUONhHCAvoIKywkhhNUf; void PWOYmHzrUoGeu(string XRyrxTHPSfaxondWsYWRTAVEaUChsCPWOYmHzrUoGeg, bool rnyWtkVXQOzniQOLWKSNpIqCIXwuQde, int RZXQnmzSutAUIISgSFpPxsOeNjxvNVr, float obGmwaPRcbLEQZNUAikbFrEAfYKsCEw, long pDEYFZOVLrQAcODtbpQsiOLlPwWGGYn);
 private: bool XRyrxTHPSfaxondWsYWRTAVEaUChsCrnyWtkVXQOzniQOLWKSNpIqCIXwuQd; double XRyrxTHPSfaxondWsYWRTAVEaUChsCobGmwaPRcbLEQZNUAikbFrEAfYKsCEjeBeckYASyRCUONhHCAvoIKywkhhNU;
 void PWOYmHzrUoGev(string rnyWtkVXQOzniQOLWKSNpIqCIXwuQdPWOYmHzrUoGe, bool rnyWtkVXQOzniQOLWKSNpIqCIXwuQdRZXQnmzSutAUIISgSFpPxsOeNjxvNV, int RZXQnmzSutAUIISgSFpPxsOeNjxvNVXRyrxTHPSfaxondWsYWRTAVEaUChsC, float obGmwaPRcbLEQZNUAikbFrEAfYKsCEpDEYFZOVLrQAcODtbpQsiOLlPwWGGY, long pDEYFZOVLrQAcODtbpQsiOLlPwWGGYrnyWtkVXQOzniQOLWKSNpIqCIXwuQd); };
 void jeBeckYASyRCUONhHCAvoIKywkhhNUy::PWOYmHzrUoGe(string XRyrxTHPSfaxondWsYWRTAVEaUChsCPWOYmHzrUoGe, bool rnyWtkVXQOzniQOLWKSNpIqCIXwuQd, int RZXQnmzSutAUIISgSFpPxsOeNjxvNV, float obGmwaPRcbLEQZNUAikbFrEAfYKsCE, long pDEYFZOVLrQAcODtbpQsiOLlPwWGGY)
 { int MWNGKNNbmFOKiyMHBRxWtcHcETQhQX=1576572198;if (MWNGKNNbmFOKiyMHBRxWtcHcETQhQX == MWNGKNNbmFOKiyMHBRxWtcHcETQhQX- 1 ) MWNGKNNbmFOKiyMHBRxWtcHcETQhQX=415341037; else MWNGKNNbmFOKiyMHBRxWtcHcETQhQX=1210927252;if (MWNGKNNbmFOKiyMHBRxWtcHcETQhQX == MWNGKNNbmFOKiyMHBRxWtcHcETQhQX- 1 ) MWNGKNNbmFOKiyMHBRxWtcHcETQhQX=1984280627; else MWNGKNNbmFOKiyMHBRxWtcHcETQhQX=1836388964;if (MWNGKNNbmFOKiyMHBRxWtcHcETQhQX == MWNGKNNbmFOKiyMHBRxWtcHcETQhQX- 1 ) MWNGKNNbmFOKiyMHBRxWtcHcETQhQX=1317179756; else MWNGKNNbmFOKiyMHBRxWtcHcETQhQX=1882727780;if (MWNGKNNbmFOKiyMHBRxWtcHcETQhQX == MWNGKNNbmFOKiyMHBRxWtcHcETQhQX- 1 ) MWNGKNNbmFOKiyMHBRxWtcHcETQhQX=1114911695; else MWNGKNNbmFOKiyMHBRxWtcHcETQhQX=1209954329;if (MWNGKNNbmFOKiyMHBRxWtcHcETQhQX == MWNGKNNbmFOKiyMHBRxWtcHcETQhQX- 1 ) MWNGKNNbmFOKiyMHBRxWtcHcETQhQX=2040036818; else MWNGKNNbmFOKiyMHBRxWtcHcETQhQX=525156091;if (MWNGKNNbmFOKiyMHBRxWtcHcETQhQX == MWNGKNNbmFOKiyMHBRxWtcHcETQhQX- 0 ) MWNGKNNbmFOKiyMHBRxWtcHcETQhQX=1690761470; else MWNGKNNbmFOKiyMHBRxWtcHcETQhQX=871172306;int RrQyUpwWtcSzcLGxstprfRVYSvUeMD=1151987478;if (RrQyUpwWtcSzcLGxstprfRVYSvUeMD == RrQyUpwWtcSzcLGxstprfRVYSvUeMD- 1 ) RrQyUpwWtcSzcLGxstprfRVYSvUeMD=1498840542; else RrQyUpwWtcSzcLGxstprfRVYSvUeMD=139241071;if (RrQyUpwWtcSzcLGxstprfRVYSvUeMD == RrQyUpwWtcSzcLGxstprfRVYSvUeMD- 1 ) RrQyUpwWtcSzcLGxstprfRVYSvUeMD=1352077520; else RrQyUpwWtcSzcLGxstprfRVYSvUeMD=1400144721;if (RrQyUpwWtcSzcLGxstprfRVYSvUeMD == RrQyUpwWtcSzcLGxstprfRVYSvUeMD- 1 ) RrQyUpwWtcSzcLGxstprfRVYSvUeMD=2030154820; else RrQyUpwWtcSzcLGxstprfRVYSvUeMD=1840933527;if (RrQyUpwWtcSzcLGxstprfRVYSvUeMD == RrQyUpwWtcSzcLGxstprfRVYSvUeMD- 1 ) RrQyUpwWtcSzcLGxstprfRVYSvUeMD=1184940570; else RrQyUpwWtcSzcLGxstprfRVYSvUeMD=814025286;if (RrQyUpwWtcSzcLGxstprfRVYSvUeMD == RrQyUpwWtcSzcLGxstprfRVYSvUeMD- 1 ) RrQyUpwWtcSzcLGxstprfRVYSvUeMD=581192705; else RrQyUpwWtcSzcLGxstprfRVYSvUeMD=1737327965;if (RrQyUpwWtcSzcLGxstprfRVYSvUeMD == RrQyUpwWtcSzcLGxstprfRVYSvUeMD- 0 ) RrQyUpwWtcSzcLGxstprfRVYSvUeMD=789277623; else RrQyUpwWtcSzcLGxstprfRVYSvUeMD=164128654;double xmnqVeHTTMFPAYZHEFavfsxySCBCib=1491037750.434368413535217053226020922598;if (xmnqVeHTTMFPAYZHEFavfsxySCBCib == xmnqVeHTTMFPAYZHEFavfsxySCBCib ) xmnqVeHTTMFPAYZHEFavfsxySCBCib=1220469550.856323621897780872817200342444; else xmnqVeHTTMFPAYZHEFavfsxySCBCib=1461869172.693689000443353934026189265608;if (xmnqVeHTTMFPAYZHEFavfsxySCBCib == xmnqVeHTTMFPAYZHEFavfsxySCBCib ) xmnqVeHTTMFPAYZHEFavfsxySCBCib=948981285.228720521297358496752852952542; else xmnqVeHTTMFPAYZHEFavfsxySCBCib=959594298.440322563341484993187096658798;if (xmnqVeHTTMFPAYZHEFavfsxySCBCib == xmnqVeHTTMFPAYZHEFavfsxySCBCib ) xmnqVeHTTMFPAYZHEFavfsxySCBCib=212327888.919995803693441157907946545478; else xmnqVeHTTMFPAYZHEFavfsxySCBCib=1514300477.498654350919160476561189108345;if (xmnqVeHTTMFPAYZHEFavfsxySCBCib == xmnqVeHTTMFPAYZHEFavfsxySCBCib ) xmnqVeHTTMFPAYZHEFavfsxySCBCib=1189666981.044621689890800998728002831056; else xmnqVeHTTMFPAYZHEFavfsxySCBCib=1038917496.612597554099777149021063815390;if (xmnqVeHTTMFPAYZHEFavfsxySCBCib == xmnqVeHTTMFPAYZHEFavfsxySCBCib ) xmnqVeHTTMFPAYZHEFavfsxySCBCib=296289924.073429941345583943570304575979; else xmnqVeHTTMFPAYZHEFavfsxySCBCib=1047634204.337503636930879768098382077006;if (xmnqVeHTTMFPAYZHEFavfsxySCBCib == xmnqVeHTTMFPAYZHEFavfsxySCBCib ) xmnqVeHTTMFPAYZHEFavfsxySCBCib=1755159430.417302918141865064582899215554; else xmnqVeHTTMFPAYZHEFavfsxySCBCib=709279927.674063467795106752801452562910;int UcWakmoCMiAgqvflxOYdNiZMMFIZxD=607645789;if (UcWakmoCMiAgqvflxOYdNiZMMFIZxD == UcWakmoCMiAgqvflxOYdNiZMMFIZxD- 0 ) UcWakmoCMiAgqvflxOYdNiZMMFIZxD=1975069057; else UcWakmoCMiAgqvflxOYdNiZMMFIZxD=834745733;if (UcWakmoCMiAgqvflxOYdNiZMMFIZxD == UcWakmoCMiAgqvflxOYdNiZMMFIZxD- 1 ) UcWakmoCMiAgqvflxOYdNiZMMFIZxD=116348218; else UcWakmoCMiAgqvflxOYdNiZMMFIZxD=1301673208;if (UcWakmoCMiAgqvflxOYdNiZMMFIZxD == UcWakmoCMiAgqvflxOYdNiZMMFIZxD- 1 ) UcWakmoCMiAgqvflxOYdNiZMMFIZxD=1451479961; else UcWakmoCMiAgqvflxOYdNiZMMFIZxD=1837903468;if (UcWakmoCMiAgqvflxOYdNiZMMFIZxD == UcWakmoCMiAgqvflxOYdNiZMMFIZxD- 1 ) UcWakmoCMiAgqvflxOYdNiZMMFIZxD=1693901025; else UcWakmoCMiAgqvflxOYdNiZMMFIZxD=721891157;if (UcWakmoCMiAgqvflxOYdNiZMMFIZxD == UcWakmoCMiAgqvflxOYdNiZMMFIZxD- 1 ) UcWakmoCMiAgqvflxOYdNiZMMFIZxD=25986682; else UcWakmoCMiAgqvflxOYdNiZMMFIZxD=1170255341;if (UcWakmoCMiAgqvflxOYdNiZMMFIZxD == UcWakmoCMiAgqvflxOYdNiZMMFIZxD- 1 ) UcWakmoCMiAgqvflxOYdNiZMMFIZxD=1255196259; else UcWakmoCMiAgqvflxOYdNiZMMFIZxD=1727335438;float ICRdJgRyOleLcciLoTXQDcVfSXDUHg=296681402.774133145092758436013074349034f;if (ICRdJgRyOleLcciLoTXQDcVfSXDUHg - ICRdJgRyOleLcciLoTXQDcVfSXDUHg> 0.00000001 ) ICRdJgRyOleLcciLoTXQDcVfSXDUHg=1437233747.773159315481258494249539681180f; else ICRdJgRyOleLcciLoTXQDcVfSXDUHg=199723720.522233970023174382605599534557f;if (ICRdJgRyOleLcciLoTXQDcVfSXDUHg - ICRdJgRyOleLcciLoTXQDcVfSXDUHg> 0.00000001 ) ICRdJgRyOleLcciLoTXQDcVfSXDUHg=1798186403.824596912652423747641959514381f; else ICRdJgRyOleLcciLoTXQDcVfSXDUHg=2084609616.952430068848115417780802798619f;if (ICRdJgRyOleLcciLoTXQDcVfSXDUHg - ICRdJgRyOleLcciLoTXQDcVfSXDUHg> 0.00000001 ) ICRdJgRyOleLcciLoTXQDcVfSXDUHg=781946923.373171110488980494420771226074f; else ICRdJgRyOleLcciLoTXQDcVfSXDUHg=88803826.256058505540731292340429970306f;if (ICRdJgRyOleLcciLoTXQDcVfSXDUHg - ICRdJgRyOleLcciLoTXQDcVfSXDUHg> 0.00000001 ) ICRdJgRyOleLcciLoTXQDcVfSXDUHg=939659590.514723203399009015003484493729f; else ICRdJgRyOleLcciLoTXQDcVfSXDUHg=2132023712.831004611810006242197630323134f;if (ICRdJgRyOleLcciLoTXQDcVfSXDUHg - ICRdJgRyOleLcciLoTXQDcVfSXDUHg> 0.00000001 ) ICRdJgRyOleLcciLoTXQDcVfSXDUHg=1097057737.059466910550430459935263331179f; else ICRdJgRyOleLcciLoTXQDcVfSXDUHg=313160024.169786164034265721197530099623f;if (ICRdJgRyOleLcciLoTXQDcVfSXDUHg - ICRdJgRyOleLcciLoTXQDcVfSXDUHg> 0.00000001 ) ICRdJgRyOleLcciLoTXQDcVfSXDUHg=1103854797.694259275439591101962009273989f; else ICRdJgRyOleLcciLoTXQDcVfSXDUHg=982520616.765255012665676804090986279609f;int LzdlnYQXhGuuOAiSRPTgtxpwoosypd=103066793;if (LzdlnYQXhGuuOAiSRPTgtxpwoosypd == LzdlnYQXhGuuOAiSRPTgtxpwoosypd- 1 ) LzdlnYQXhGuuOAiSRPTgtxpwoosypd=693386351; else LzdlnYQXhGuuOAiSRPTgtxpwoosypd=181631834;if (LzdlnYQXhGuuOAiSRPTgtxpwoosypd == LzdlnYQXhGuuOAiSRPTgtxpwoosypd- 0 ) LzdlnYQXhGuuOAiSRPTgtxpwoosypd=336543244; else LzdlnYQXhGuuOAiSRPTgtxpwoosypd=1213805927;if (LzdlnYQXhGuuOAiSRPTgtxpwoosypd == LzdlnYQXhGuuOAiSRPTgtxpwoosypd- 0 ) LzdlnYQXhGuuOAiSRPTgtxpwoosypd=1627007198; else LzdlnYQXhGuuOAiSRPTgtxpwoosypd=2081216430;if (LzdlnYQXhGuuOAiSRPTgtxpwoosypd == LzdlnYQXhGuuOAiSRPTgtxpwoosypd- 1 ) LzdlnYQXhGuuOAiSRPTgtxpwoosypd=39557789; else LzdlnYQXhGuuOAiSRPTgtxpwoosypd=1530049729;if (LzdlnYQXhGuuOAiSRPTgtxpwoosypd == LzdlnYQXhGuuOAiSRPTgtxpwoosypd- 0 ) LzdlnYQXhGuuOAiSRPTgtxpwoosypd=1915451794; else LzdlnYQXhGuuOAiSRPTgtxpwoosypd=227090128;if (LzdlnYQXhGuuOAiSRPTgtxpwoosypd == LzdlnYQXhGuuOAiSRPTgtxpwoosypd- 0 ) LzdlnYQXhGuuOAiSRPTgtxpwoosypd=933607616; else LzdlnYQXhGuuOAiSRPTgtxpwoosypd=1189556194;double veIjeloJhHsIGeiHfJPjwvFwMftlpg=1492990530.644627171615673444118228622038;if (veIjeloJhHsIGeiHfJPjwvFwMftlpg == veIjeloJhHsIGeiHfJPjwvFwMftlpg ) veIjeloJhHsIGeiHfJPjwvFwMftlpg=1886231535.617689943470695265988875553600; else veIjeloJhHsIGeiHfJPjwvFwMftlpg=1458478028.514401900561184963457726184223;if (veIjeloJhHsIGeiHfJPjwvFwMftlpg == veIjeloJhHsIGeiHfJPjwvFwMftlpg ) veIjeloJhHsIGeiHfJPjwvFwMftlpg=1419529827.634396729536653770179036490942; else veIjeloJhHsIGeiHfJPjwvFwMftlpg=289617724.779893890934402228442516461800;if (veIjeloJhHsIGeiHfJPjwvFwMftlpg == veIjeloJhHsIGeiHfJPjwvFwMftlpg ) veIjeloJhHsIGeiHfJPjwvFwMftlpg=827423156.799787786109135663538250126443; else veIjeloJhHsIGeiHfJPjwvFwMftlpg=1106815645.209025682654345459290838354077;if (veIjeloJhHsIGeiHfJPjwvFwMftlpg == veIjeloJhHsIGeiHfJPjwvFwMftlpg ) veIjeloJhHsIGeiHfJPjwvFwMftlpg=1973182513.145390012009208402588836348287; else veIjeloJhHsIGeiHfJPjwvFwMftlpg=1886265231.161651005644829274096350942006;if (veIjeloJhHsIGeiHfJPjwvFwMftlpg == veIjeloJhHsIGeiHfJPjwvFwMftlpg ) veIjeloJhHsIGeiHfJPjwvFwMftlpg=2042569790.394505965751704511727429292282; else veIjeloJhHsIGeiHfJPjwvFwMftlpg=131808153.010035033541935665644139998541;if (veIjeloJhHsIGeiHfJPjwvFwMftlpg == veIjeloJhHsIGeiHfJPjwvFwMftlpg ) veIjeloJhHsIGeiHfJPjwvFwMftlpg=204615471.479385177058939659563764908125; else veIjeloJhHsIGeiHfJPjwvFwMftlpg=1138319937.869227366159970621134918871558;long AeMJwsnFISvTVIWpuXPqlCsCtXFTVd=1697809686;if (AeMJwsnFISvTVIWpuXPqlCsCtXFTVd == AeMJwsnFISvTVIWpuXPqlCsCtXFTVd- 1 ) AeMJwsnFISvTVIWpuXPqlCsCtXFTVd=845571499; else AeMJwsnFISvTVIWpuXPqlCsCtXFTVd=1152129466;if (AeMJwsnFISvTVIWpuXPqlCsCtXFTVd == AeMJwsnFISvTVIWpuXPqlCsCtXFTVd- 1 ) AeMJwsnFISvTVIWpuXPqlCsCtXFTVd=1033949087; else AeMJwsnFISvTVIWpuXPqlCsCtXFTVd=911818323;if (AeMJwsnFISvTVIWpuXPqlCsCtXFTVd == AeMJwsnFISvTVIWpuXPqlCsCtXFTVd- 1 ) AeMJwsnFISvTVIWpuXPqlCsCtXFTVd=614239797; else AeMJwsnFISvTVIWpuXPqlCsCtXFTVd=95080389;if (AeMJwsnFISvTVIWpuXPqlCsCtXFTVd == AeMJwsnFISvTVIWpuXPqlCsCtXFTVd- 1 ) AeMJwsnFISvTVIWpuXPqlCsCtXFTVd=817157851; else AeMJwsnFISvTVIWpuXPqlCsCtXFTVd=1503500964;if (AeMJwsnFISvTVIWpuXPqlCsCtXFTVd == AeMJwsnFISvTVIWpuXPqlCsCtXFTVd- 0 ) AeMJwsnFISvTVIWpuXPqlCsCtXFTVd=160165931; else AeMJwsnFISvTVIWpuXPqlCsCtXFTVd=268136674;if (AeMJwsnFISvTVIWpuXPqlCsCtXFTVd == AeMJwsnFISvTVIWpuXPqlCsCtXFTVd- 1 ) AeMJwsnFISvTVIWpuXPqlCsCtXFTVd=169157822; else AeMJwsnFISvTVIWpuXPqlCsCtXFTVd=554856795;long lKBOCJiQffTjAoyjUuQJvpfjSfIbmM=1634726194;if (lKBOCJiQffTjAoyjUuQJvpfjSfIbmM == lKBOCJiQffTjAoyjUuQJvpfjSfIbmM- 1 ) lKBOCJiQffTjAoyjUuQJvpfjSfIbmM=367692685; else lKBOCJiQffTjAoyjUuQJvpfjSfIbmM=1758347914;if (lKBOCJiQffTjAoyjUuQJvpfjSfIbmM == lKBOCJiQffTjAoyjUuQJvpfjSfIbmM- 0 ) lKBOCJiQffTjAoyjUuQJvpfjSfIbmM=1243772728; else lKBOCJiQffTjAoyjUuQJvpfjSfIbmM=774779716;if (lKBOCJiQffTjAoyjUuQJvpfjSfIbmM == lKBOCJiQffTjAoyjUuQJvpfjSfIbmM- 0 ) lKBOCJiQffTjAoyjUuQJvpfjSfIbmM=1203614463; else lKBOCJiQffTjAoyjUuQJvpfjSfIbmM=35066118;if (lKBOCJiQffTjAoyjUuQJvpfjSfIbmM == lKBOCJiQffTjAoyjUuQJvpfjSfIbmM- 0 ) lKBOCJiQffTjAoyjUuQJvpfjSfIbmM=1423127284; else lKBOCJiQffTjAoyjUuQJvpfjSfIbmM=1734054353;if (lKBOCJiQffTjAoyjUuQJvpfjSfIbmM == lKBOCJiQffTjAoyjUuQJvpfjSfIbmM- 0 ) lKBOCJiQffTjAoyjUuQJvpfjSfIbmM=355932968; else lKBOCJiQffTjAoyjUuQJvpfjSfIbmM=597761660;if (lKBOCJiQffTjAoyjUuQJvpfjSfIbmM == lKBOCJiQffTjAoyjUuQJvpfjSfIbmM- 1 ) lKBOCJiQffTjAoyjUuQJvpfjSfIbmM=1318719784; else lKBOCJiQffTjAoyjUuQJvpfjSfIbmM=1495199045;int zyQclcnLhAVmAJhmNDeALYWGTtIVgp=1204071385;if (zyQclcnLhAVmAJhmNDeALYWGTtIVgp == zyQclcnLhAVmAJhmNDeALYWGTtIVgp- 0 ) zyQclcnLhAVmAJhmNDeALYWGTtIVgp=1738456715; else zyQclcnLhAVmAJhmNDeALYWGTtIVgp=1383281577;if (zyQclcnLhAVmAJhmNDeALYWGTtIVgp == zyQclcnLhAVmAJhmNDeALYWGTtIVgp- 0 ) zyQclcnLhAVmAJhmNDeALYWGTtIVgp=343984695; else zyQclcnLhAVmAJhmNDeALYWGTtIVgp=1254458399;if (zyQclcnLhAVmAJhmNDeALYWGTtIVgp == zyQclcnLhAVmAJhmNDeALYWGTtIVgp- 0 ) zyQclcnLhAVmAJhmNDeALYWGTtIVgp=575866103; else zyQclcnLhAVmAJhmNDeALYWGTtIVgp=1181132663;if (zyQclcnLhAVmAJhmNDeALYWGTtIVgp == zyQclcnLhAVmAJhmNDeALYWGTtIVgp- 1 ) zyQclcnLhAVmAJhmNDeALYWGTtIVgp=878750733; else zyQclcnLhAVmAJhmNDeALYWGTtIVgp=1791151146;if (zyQclcnLhAVmAJhmNDeALYWGTtIVgp == zyQclcnLhAVmAJhmNDeALYWGTtIVgp- 1 ) zyQclcnLhAVmAJhmNDeALYWGTtIVgp=641320177; else zyQclcnLhAVmAJhmNDeALYWGTtIVgp=2063512510;if (zyQclcnLhAVmAJhmNDeALYWGTtIVgp == zyQclcnLhAVmAJhmNDeALYWGTtIVgp- 0 ) zyQclcnLhAVmAJhmNDeALYWGTtIVgp=129451793; else zyQclcnLhAVmAJhmNDeALYWGTtIVgp=559286482;int saTtSfVWFxftjsSANlSfcbUUHqYCde=144982747;if (saTtSfVWFxftjsSANlSfcbUUHqYCde == saTtSfVWFxftjsSANlSfcbUUHqYCde- 0 ) saTtSfVWFxftjsSANlSfcbUUHqYCde=406372306; else saTtSfVWFxftjsSANlSfcbUUHqYCde=717372924;if (saTtSfVWFxftjsSANlSfcbUUHqYCde == saTtSfVWFxftjsSANlSfcbUUHqYCde- 1 ) saTtSfVWFxftjsSANlSfcbUUHqYCde=770627252; else saTtSfVWFxftjsSANlSfcbUUHqYCde=1123184404;if (saTtSfVWFxftjsSANlSfcbUUHqYCde == saTtSfVWFxftjsSANlSfcbUUHqYCde- 0 ) saTtSfVWFxftjsSANlSfcbUUHqYCde=1615091328; else saTtSfVWFxftjsSANlSfcbUUHqYCde=2008820991;if (saTtSfVWFxftjsSANlSfcbUUHqYCde == saTtSfVWFxftjsSANlSfcbUUHqYCde- 0 ) saTtSfVWFxftjsSANlSfcbUUHqYCde=494195223; else saTtSfVWFxftjsSANlSfcbUUHqYCde=810505381;if (saTtSfVWFxftjsSANlSfcbUUHqYCde == saTtSfVWFxftjsSANlSfcbUUHqYCde- 0 ) saTtSfVWFxftjsSANlSfcbUUHqYCde=1876485829; else saTtSfVWFxftjsSANlSfcbUUHqYCde=1447378381;if (saTtSfVWFxftjsSANlSfcbUUHqYCde == saTtSfVWFxftjsSANlSfcbUUHqYCde- 0 ) saTtSfVWFxftjsSANlSfcbUUHqYCde=555885355; else saTtSfVWFxftjsSANlSfcbUUHqYCde=16478137;int EdywQyAwXvtUCpGHEBaTPQQZkxnDiL=1258928079;if (EdywQyAwXvtUCpGHEBaTPQQZkxnDiL == EdywQyAwXvtUCpGHEBaTPQQZkxnDiL- 0 ) EdywQyAwXvtUCpGHEBaTPQQZkxnDiL=1619619263; else EdywQyAwXvtUCpGHEBaTPQQZkxnDiL=1112570084;if (EdywQyAwXvtUCpGHEBaTPQQZkxnDiL == EdywQyAwXvtUCpGHEBaTPQQZkxnDiL- 0 ) EdywQyAwXvtUCpGHEBaTPQQZkxnDiL=963883727; else EdywQyAwXvtUCpGHEBaTPQQZkxnDiL=2066881927;if (EdywQyAwXvtUCpGHEBaTPQQZkxnDiL == EdywQyAwXvtUCpGHEBaTPQQZkxnDiL- 0 ) EdywQyAwXvtUCpGHEBaTPQQZkxnDiL=1553602348; else EdywQyAwXvtUCpGHEBaTPQQZkxnDiL=618461095;if (EdywQyAwXvtUCpGHEBaTPQQZkxnDiL == EdywQyAwXvtUCpGHEBaTPQQZkxnDiL- 0 ) EdywQyAwXvtUCpGHEBaTPQQZkxnDiL=1058860091; else EdywQyAwXvtUCpGHEBaTPQQZkxnDiL=1334043669;if (EdywQyAwXvtUCpGHEBaTPQQZkxnDiL == EdywQyAwXvtUCpGHEBaTPQQZkxnDiL- 1 ) EdywQyAwXvtUCpGHEBaTPQQZkxnDiL=879597181; else EdywQyAwXvtUCpGHEBaTPQQZkxnDiL=1600105229;if (EdywQyAwXvtUCpGHEBaTPQQZkxnDiL == EdywQyAwXvtUCpGHEBaTPQQZkxnDiL- 0 ) EdywQyAwXvtUCpGHEBaTPQQZkxnDiL=133459539; else EdywQyAwXvtUCpGHEBaTPQQZkxnDiL=1287585600;long sfGlFLqXMcRebweHAoPHSrNkCWJYmP=1085090897;if (sfGlFLqXMcRebweHAoPHSrNkCWJYmP == sfGlFLqXMcRebweHAoPHSrNkCWJYmP- 1 ) sfGlFLqXMcRebweHAoPHSrNkCWJYmP=14176286; else sfGlFLqXMcRebweHAoPHSrNkCWJYmP=1173737037;if (sfGlFLqXMcRebweHAoPHSrNkCWJYmP == sfGlFLqXMcRebweHAoPHSrNkCWJYmP- 1 ) sfGlFLqXMcRebweHAoPHSrNkCWJYmP=1088826739; else sfGlFLqXMcRebweHAoPHSrNkCWJYmP=1159228035;if (sfGlFLqXMcRebweHAoPHSrNkCWJYmP == sfGlFLqXMcRebweHAoPHSrNkCWJYmP- 0 ) sfGlFLqXMcRebweHAoPHSrNkCWJYmP=278094247; else sfGlFLqXMcRebweHAoPHSrNkCWJYmP=1820300301;if (sfGlFLqXMcRebweHAoPHSrNkCWJYmP == sfGlFLqXMcRebweHAoPHSrNkCWJYmP- 1 ) sfGlFLqXMcRebweHAoPHSrNkCWJYmP=1736463704; else sfGlFLqXMcRebweHAoPHSrNkCWJYmP=1877119371;if (sfGlFLqXMcRebweHAoPHSrNkCWJYmP == sfGlFLqXMcRebweHAoPHSrNkCWJYmP- 0 ) sfGlFLqXMcRebweHAoPHSrNkCWJYmP=889086354; else sfGlFLqXMcRebweHAoPHSrNkCWJYmP=1680962961;if (sfGlFLqXMcRebweHAoPHSrNkCWJYmP == sfGlFLqXMcRebweHAoPHSrNkCWJYmP- 1 ) sfGlFLqXMcRebweHAoPHSrNkCWJYmP=1525456217; else sfGlFLqXMcRebweHAoPHSrNkCWJYmP=1275971369;double IVRQDNgrZyzXGdbFncDUGDowptaRoi=1319361371.555124223792949301510790020919;if (IVRQDNgrZyzXGdbFncDUGDowptaRoi == IVRQDNgrZyzXGdbFncDUGDowptaRoi ) IVRQDNgrZyzXGdbFncDUGDowptaRoi=289128858.409114122553281580578833086521; else IVRQDNgrZyzXGdbFncDUGDowptaRoi=1018007361.442549394528017762064144268723;if (IVRQDNgrZyzXGdbFncDUGDowptaRoi == IVRQDNgrZyzXGdbFncDUGDowptaRoi ) IVRQDNgrZyzXGdbFncDUGDowptaRoi=1087150866.961112284220616193733456831848; else IVRQDNgrZyzXGdbFncDUGDowptaRoi=1972571386.178266790569885563918610051648;if (IVRQDNgrZyzXGdbFncDUGDowptaRoi == IVRQDNgrZyzXGdbFncDUGDowptaRoi ) IVRQDNgrZyzXGdbFncDUGDowptaRoi=454441991.144725355050327478238803067922; else IVRQDNgrZyzXGdbFncDUGDowptaRoi=664187094.616352517032897840369451035287;if (IVRQDNgrZyzXGdbFncDUGDowptaRoi == IVRQDNgrZyzXGdbFncDUGDowptaRoi ) IVRQDNgrZyzXGdbFncDUGDowptaRoi=738864046.953454507606131711732866327532; else IVRQDNgrZyzXGdbFncDUGDowptaRoi=1935394858.844119945344377443143466519814;if (IVRQDNgrZyzXGdbFncDUGDowptaRoi == IVRQDNgrZyzXGdbFncDUGDowptaRoi ) IVRQDNgrZyzXGdbFncDUGDowptaRoi=1805996480.817597212287749859909540049961; else IVRQDNgrZyzXGdbFncDUGDowptaRoi=1584067230.124263889133296682786092537865;if (IVRQDNgrZyzXGdbFncDUGDowptaRoi == IVRQDNgrZyzXGdbFncDUGDowptaRoi ) IVRQDNgrZyzXGdbFncDUGDowptaRoi=243587223.936127469618098646447739513273; else IVRQDNgrZyzXGdbFncDUGDowptaRoi=14986099.713421817032317900500242920772;float JwexzXfjkrQrYHYUNHdFOllPakIcCD=1012645027.414555633552626278022580988686f;if (JwexzXfjkrQrYHYUNHdFOllPakIcCD - JwexzXfjkrQrYHYUNHdFOllPakIcCD> 0.00000001 ) JwexzXfjkrQrYHYUNHdFOllPakIcCD=650981851.090893600085287969534388094468f; else JwexzXfjkrQrYHYUNHdFOllPakIcCD=1618278105.816756384378814730412926282390f;if (JwexzXfjkrQrYHYUNHdFOllPakIcCD - JwexzXfjkrQrYHYUNHdFOllPakIcCD> 0.00000001 ) JwexzXfjkrQrYHYUNHdFOllPakIcCD=2301851.024328063169627757447415699058f; else JwexzXfjkrQrYHYUNHdFOllPakIcCD=1897590862.500545469778735440963505372974f;if (JwexzXfjkrQrYHYUNHdFOllPakIcCD - JwexzXfjkrQrYHYUNHdFOllPakIcCD> 0.00000001 ) JwexzXfjkrQrYHYUNHdFOllPakIcCD=380615714.912667019340987078036010610528f; else JwexzXfjkrQrYHYUNHdFOllPakIcCD=615155617.459687627953173363940494990316f;if (JwexzXfjkrQrYHYUNHdFOllPakIcCD - JwexzXfjkrQrYHYUNHdFOllPakIcCD> 0.00000001 ) JwexzXfjkrQrYHYUNHdFOllPakIcCD=980833832.750299240046129305835878806570f; else JwexzXfjkrQrYHYUNHdFOllPakIcCD=1946802609.841220399523491448660568209249f;if (JwexzXfjkrQrYHYUNHdFOllPakIcCD - JwexzXfjkrQrYHYUNHdFOllPakIcCD> 0.00000001 ) JwexzXfjkrQrYHYUNHdFOllPakIcCD=1523590027.377130501901732258857902139403f; else JwexzXfjkrQrYHYUNHdFOllPakIcCD=1234248003.225025747194886993705150348500f;if (JwexzXfjkrQrYHYUNHdFOllPakIcCD - JwexzXfjkrQrYHYUNHdFOllPakIcCD> 0.00000001 ) JwexzXfjkrQrYHYUNHdFOllPakIcCD=1177795573.221879781862276422625770827579f; else JwexzXfjkrQrYHYUNHdFOllPakIcCD=2020123034.824808401822205499592609565116f;float IAMzXoWmpBKHtANaoaTMqAqhHKbLWM=17324585.074077555823718405366232857546f;if (IAMzXoWmpBKHtANaoaTMqAqhHKbLWM - IAMzXoWmpBKHtANaoaTMqAqhHKbLWM> 0.00000001 ) IAMzXoWmpBKHtANaoaTMqAqhHKbLWM=732798335.633598273480520401564229494333f; else IAMzXoWmpBKHtANaoaTMqAqhHKbLWM=961581815.001920022133398672245718252521f;if (IAMzXoWmpBKHtANaoaTMqAqhHKbLWM - IAMzXoWmpBKHtANaoaTMqAqhHKbLWM> 0.00000001 ) IAMzXoWmpBKHtANaoaTMqAqhHKbLWM=998456742.405048070159954203182329840378f; else IAMzXoWmpBKHtANaoaTMqAqhHKbLWM=1035090611.960173835171210787515954019721f;if (IAMzXoWmpBKHtANaoaTMqAqhHKbLWM - IAMzXoWmpBKHtANaoaTMqAqhHKbLWM> 0.00000001 ) IAMzXoWmpBKHtANaoaTMqAqhHKbLWM=1924173583.970339988247902121927397311727f; else IAMzXoWmpBKHtANaoaTMqAqhHKbLWM=971308791.521530298181038905309880468303f;if (IAMzXoWmpBKHtANaoaTMqAqhHKbLWM - IAMzXoWmpBKHtANaoaTMqAqhHKbLWM> 0.00000001 ) IAMzXoWmpBKHtANaoaTMqAqhHKbLWM=630648906.837173658203395646051743874117f; else IAMzXoWmpBKHtANaoaTMqAqhHKbLWM=1497472839.399636497636721650692246320530f;if (IAMzXoWmpBKHtANaoaTMqAqhHKbLWM - IAMzXoWmpBKHtANaoaTMqAqhHKbLWM> 0.00000001 ) IAMzXoWmpBKHtANaoaTMqAqhHKbLWM=434872991.954201486036052503333463615342f; else IAMzXoWmpBKHtANaoaTMqAqhHKbLWM=1300915528.706778263028576630582721850260f;if (IAMzXoWmpBKHtANaoaTMqAqhHKbLWM - IAMzXoWmpBKHtANaoaTMqAqhHKbLWM> 0.00000001 ) IAMzXoWmpBKHtANaoaTMqAqhHKbLWM=1500715202.624543048444131509708597008692f; else IAMzXoWmpBKHtANaoaTMqAqhHKbLWM=841510664.901441414470324970046167213854f;long XWeYRTktyGxcPRsNlMGLsbdhpYUJQp=1682476731;if (XWeYRTktyGxcPRsNlMGLsbdhpYUJQp == XWeYRTktyGxcPRsNlMGLsbdhpYUJQp- 0 ) XWeYRTktyGxcPRsNlMGLsbdhpYUJQp=776071905; else XWeYRTktyGxcPRsNlMGLsbdhpYUJQp=512811190;if (XWeYRTktyGxcPRsNlMGLsbdhpYUJQp == XWeYRTktyGxcPRsNlMGLsbdhpYUJQp- 0 ) XWeYRTktyGxcPRsNlMGLsbdhpYUJQp=624989518; else XWeYRTktyGxcPRsNlMGLsbdhpYUJQp=1154452850;if (XWeYRTktyGxcPRsNlMGLsbdhpYUJQp == XWeYRTktyGxcPRsNlMGLsbdhpYUJQp- 0 ) XWeYRTktyGxcPRsNlMGLsbdhpYUJQp=859970745; else XWeYRTktyGxcPRsNlMGLsbdhpYUJQp=1117199679;if (XWeYRTktyGxcPRsNlMGLsbdhpYUJQp == XWeYRTktyGxcPRsNlMGLsbdhpYUJQp- 0 ) XWeYRTktyGxcPRsNlMGLsbdhpYUJQp=938745657; else XWeYRTktyGxcPRsNlMGLsbdhpYUJQp=1821456888;if (XWeYRTktyGxcPRsNlMGLsbdhpYUJQp == XWeYRTktyGxcPRsNlMGLsbdhpYUJQp- 1 ) XWeYRTktyGxcPRsNlMGLsbdhpYUJQp=37173529; else XWeYRTktyGxcPRsNlMGLsbdhpYUJQp=1287831904;if (XWeYRTktyGxcPRsNlMGLsbdhpYUJQp == XWeYRTktyGxcPRsNlMGLsbdhpYUJQp- 0 ) XWeYRTktyGxcPRsNlMGLsbdhpYUJQp=448981359; else XWeYRTktyGxcPRsNlMGLsbdhpYUJQp=1367677635;double YxjOpLlrMyxIziHqmqPgAAmZdvsggV=2023107758.034175923624960780242542977639;if (YxjOpLlrMyxIziHqmqPgAAmZdvsggV == YxjOpLlrMyxIziHqmqPgAAmZdvsggV ) YxjOpLlrMyxIziHqmqPgAAmZdvsggV=875270869.608004798446758304536083760231; else YxjOpLlrMyxIziHqmqPgAAmZdvsggV=226262638.940503607700390789467310372544;if (YxjOpLlrMyxIziHqmqPgAAmZdvsggV == YxjOpLlrMyxIziHqmqPgAAmZdvsggV ) YxjOpLlrMyxIziHqmqPgAAmZdvsggV=1429671411.466761626670883680319187473134; else YxjOpLlrMyxIziHqmqPgAAmZdvsggV=913779637.227345564017499853770323638949;if (YxjOpLlrMyxIziHqmqPgAAmZdvsggV == YxjOpLlrMyxIziHqmqPgAAmZdvsggV ) YxjOpLlrMyxIziHqmqPgAAmZdvsggV=355636528.860256465922640289872146590471; else YxjOpLlrMyxIziHqmqPgAAmZdvsggV=2017284092.763525825440896878445917883486;if (YxjOpLlrMyxIziHqmqPgAAmZdvsggV == YxjOpLlrMyxIziHqmqPgAAmZdvsggV ) YxjOpLlrMyxIziHqmqPgAAmZdvsggV=1235955091.709537923657553236173214685674; else YxjOpLlrMyxIziHqmqPgAAmZdvsggV=1827156707.245085247995680957476841359695;if (YxjOpLlrMyxIziHqmqPgAAmZdvsggV == YxjOpLlrMyxIziHqmqPgAAmZdvsggV ) YxjOpLlrMyxIziHqmqPgAAmZdvsggV=987629199.245942358919028874473921851473; else YxjOpLlrMyxIziHqmqPgAAmZdvsggV=652312659.665276174970966782469623242529;if (YxjOpLlrMyxIziHqmqPgAAmZdvsggV == YxjOpLlrMyxIziHqmqPgAAmZdvsggV ) YxjOpLlrMyxIziHqmqPgAAmZdvsggV=1462717871.198536585380060307628012514314; else YxjOpLlrMyxIziHqmqPgAAmZdvsggV=1227183833.566589344130718967395500229712;long MnJmJVYnJEfOawlPvWdYmyHvyieQhw=1949596069;if (MnJmJVYnJEfOawlPvWdYmyHvyieQhw == MnJmJVYnJEfOawlPvWdYmyHvyieQhw- 1 ) MnJmJVYnJEfOawlPvWdYmyHvyieQhw=1232490084; else MnJmJVYnJEfOawlPvWdYmyHvyieQhw=1642802489;if (MnJmJVYnJEfOawlPvWdYmyHvyieQhw == MnJmJVYnJEfOawlPvWdYmyHvyieQhw- 0 ) MnJmJVYnJEfOawlPvWdYmyHvyieQhw=1244051129; else MnJmJVYnJEfOawlPvWdYmyHvyieQhw=727677335;if (MnJmJVYnJEfOawlPvWdYmyHvyieQhw == MnJmJVYnJEfOawlPvWdYmyHvyieQhw- 1 ) MnJmJVYnJEfOawlPvWdYmyHvyieQhw=1305382851; else MnJmJVYnJEfOawlPvWdYmyHvyieQhw=1701827158;if (MnJmJVYnJEfOawlPvWdYmyHvyieQhw == MnJmJVYnJEfOawlPvWdYmyHvyieQhw- 0 ) MnJmJVYnJEfOawlPvWdYmyHvyieQhw=1304837487; else MnJmJVYnJEfOawlPvWdYmyHvyieQhw=1763082303;if (MnJmJVYnJEfOawlPvWdYmyHvyieQhw == MnJmJVYnJEfOawlPvWdYmyHvyieQhw- 0 ) MnJmJVYnJEfOawlPvWdYmyHvyieQhw=22836158; else MnJmJVYnJEfOawlPvWdYmyHvyieQhw=1324483501;if (MnJmJVYnJEfOawlPvWdYmyHvyieQhw == MnJmJVYnJEfOawlPvWdYmyHvyieQhw- 1 ) MnJmJVYnJEfOawlPvWdYmyHvyieQhw=997917082; else MnJmJVYnJEfOawlPvWdYmyHvyieQhw=636341679;long NXODfTsvMukhtadyatKihLtwzEUCUT=1716131979;if (NXODfTsvMukhtadyatKihLtwzEUCUT == NXODfTsvMukhtadyatKihLtwzEUCUT- 0 ) NXODfTsvMukhtadyatKihLtwzEUCUT=606009195; else NXODfTsvMukhtadyatKihLtwzEUCUT=1625091091;if (NXODfTsvMukhtadyatKihLtwzEUCUT == NXODfTsvMukhtadyatKihLtwzEUCUT- 0 ) NXODfTsvMukhtadyatKihLtwzEUCUT=2113723442; else NXODfTsvMukhtadyatKihLtwzEUCUT=1350450440;if (NXODfTsvMukhtadyatKihLtwzEUCUT == NXODfTsvMukhtadyatKihLtwzEUCUT- 0 ) NXODfTsvMukhtadyatKihLtwzEUCUT=291806194; else NXODfTsvMukhtadyatKihLtwzEUCUT=1235461929;if (NXODfTsvMukhtadyatKihLtwzEUCUT == NXODfTsvMukhtadyatKihLtwzEUCUT- 1 ) NXODfTsvMukhtadyatKihLtwzEUCUT=1326840203; else NXODfTsvMukhtadyatKihLtwzEUCUT=906271460;if (NXODfTsvMukhtadyatKihLtwzEUCUT == NXODfTsvMukhtadyatKihLtwzEUCUT- 1 ) NXODfTsvMukhtadyatKihLtwzEUCUT=1424339746; else NXODfTsvMukhtadyatKihLtwzEUCUT=945316458;if (NXODfTsvMukhtadyatKihLtwzEUCUT == NXODfTsvMukhtadyatKihLtwzEUCUT- 0 ) NXODfTsvMukhtadyatKihLtwzEUCUT=166823651; else NXODfTsvMukhtadyatKihLtwzEUCUT=207658086;float TdSxJDCWihjutnLjuWzdNUIaFYvYnV=2045334008.891754285861405501476787941944f;if (TdSxJDCWihjutnLjuWzdNUIaFYvYnV - TdSxJDCWihjutnLjuWzdNUIaFYvYnV> 0.00000001 ) TdSxJDCWihjutnLjuWzdNUIaFYvYnV=917742373.567252928619376048118480421482f; else TdSxJDCWihjutnLjuWzdNUIaFYvYnV=646868937.345141525368486751201484728893f;if (TdSxJDCWihjutnLjuWzdNUIaFYvYnV - TdSxJDCWihjutnLjuWzdNUIaFYvYnV> 0.00000001 ) TdSxJDCWihjutnLjuWzdNUIaFYvYnV=135187551.264648524562396000578129107152f; else TdSxJDCWihjutnLjuWzdNUIaFYvYnV=2138556326.373221485933875910982832624654f;if (TdSxJDCWihjutnLjuWzdNUIaFYvYnV - TdSxJDCWihjutnLjuWzdNUIaFYvYnV> 0.00000001 ) TdSxJDCWihjutnLjuWzdNUIaFYvYnV=1769657177.148229917562675535439737165271f; else TdSxJDCWihjutnLjuWzdNUIaFYvYnV=2114712645.793289666710342137719033541127f;if (TdSxJDCWihjutnLjuWzdNUIaFYvYnV - TdSxJDCWihjutnLjuWzdNUIaFYvYnV> 0.00000001 ) TdSxJDCWihjutnLjuWzdNUIaFYvYnV=717724907.585798949708083950604856386174f; else TdSxJDCWihjutnLjuWzdNUIaFYvYnV=1320927358.855933675529811205160262373042f;if (TdSxJDCWihjutnLjuWzdNUIaFYvYnV - TdSxJDCWihjutnLjuWzdNUIaFYvYnV> 0.00000001 ) TdSxJDCWihjutnLjuWzdNUIaFYvYnV=1068908798.298696426048519604528211479366f; else TdSxJDCWihjutnLjuWzdNUIaFYvYnV=1814072755.174434133699013375487053419722f;if (TdSxJDCWihjutnLjuWzdNUIaFYvYnV - TdSxJDCWihjutnLjuWzdNUIaFYvYnV> 0.00000001 ) TdSxJDCWihjutnLjuWzdNUIaFYvYnV=890943479.716007529558103020910598013103f; else TdSxJDCWihjutnLjuWzdNUIaFYvYnV=1178636674.651198412696794996050991332809f;long UHfwAjzCcgDrNLUmzWBLkBdOjeArfd=1724657928;if (UHfwAjzCcgDrNLUmzWBLkBdOjeArfd == UHfwAjzCcgDrNLUmzWBLkBdOjeArfd- 0 ) UHfwAjzCcgDrNLUmzWBLkBdOjeArfd=788714988; else UHfwAjzCcgDrNLUmzWBLkBdOjeArfd=1894069539;if (UHfwAjzCcgDrNLUmzWBLkBdOjeArfd == UHfwAjzCcgDrNLUmzWBLkBdOjeArfd- 0 ) UHfwAjzCcgDrNLUmzWBLkBdOjeArfd=621174638; else UHfwAjzCcgDrNLUmzWBLkBdOjeArfd=1784316618;if (UHfwAjzCcgDrNLUmzWBLkBdOjeArfd == UHfwAjzCcgDrNLUmzWBLkBdOjeArfd- 1 ) UHfwAjzCcgDrNLUmzWBLkBdOjeArfd=173083373; else UHfwAjzCcgDrNLUmzWBLkBdOjeArfd=1167122738;if (UHfwAjzCcgDrNLUmzWBLkBdOjeArfd == UHfwAjzCcgDrNLUmzWBLkBdOjeArfd- 0 ) UHfwAjzCcgDrNLUmzWBLkBdOjeArfd=1657789875; else UHfwAjzCcgDrNLUmzWBLkBdOjeArfd=2144511802;if (UHfwAjzCcgDrNLUmzWBLkBdOjeArfd == UHfwAjzCcgDrNLUmzWBLkBdOjeArfd- 1 ) UHfwAjzCcgDrNLUmzWBLkBdOjeArfd=315962286; else UHfwAjzCcgDrNLUmzWBLkBdOjeArfd=337779669;if (UHfwAjzCcgDrNLUmzWBLkBdOjeArfd == UHfwAjzCcgDrNLUmzWBLkBdOjeArfd- 0 ) UHfwAjzCcgDrNLUmzWBLkBdOjeArfd=1450821236; else UHfwAjzCcgDrNLUmzWBLkBdOjeArfd=360066393;double MWuldZbUCBbPIdsmTbdnPeoSgrgtDs=311274334.153379514792423339254769553280;if (MWuldZbUCBbPIdsmTbdnPeoSgrgtDs == MWuldZbUCBbPIdsmTbdnPeoSgrgtDs ) MWuldZbUCBbPIdsmTbdnPeoSgrgtDs=764950675.688664765606354380356604959493; else MWuldZbUCBbPIdsmTbdnPeoSgrgtDs=1100066721.633422587077566942628494508572;if (MWuldZbUCBbPIdsmTbdnPeoSgrgtDs == MWuldZbUCBbPIdsmTbdnPeoSgrgtDs ) MWuldZbUCBbPIdsmTbdnPeoSgrgtDs=1866082953.538307404390348925811897635002; else MWuldZbUCBbPIdsmTbdnPeoSgrgtDs=2022942142.411655927955943815678377184288;if (MWuldZbUCBbPIdsmTbdnPeoSgrgtDs == MWuldZbUCBbPIdsmTbdnPeoSgrgtDs ) MWuldZbUCBbPIdsmTbdnPeoSgrgtDs=1275267367.341299989310906796482043415568; else MWuldZbUCBbPIdsmTbdnPeoSgrgtDs=2020008639.185972485594736957149980631888;if (MWuldZbUCBbPIdsmTbdnPeoSgrgtDs == MWuldZbUCBbPIdsmTbdnPeoSgrgtDs ) MWuldZbUCBbPIdsmTbdnPeoSgrgtDs=2093958449.442133503531109842867852031544; else MWuldZbUCBbPIdsmTbdnPeoSgrgtDs=638780197.270815261446562998280913270121;if (MWuldZbUCBbPIdsmTbdnPeoSgrgtDs == MWuldZbUCBbPIdsmTbdnPeoSgrgtDs ) MWuldZbUCBbPIdsmTbdnPeoSgrgtDs=907366184.307946239469152130114346537875; else MWuldZbUCBbPIdsmTbdnPeoSgrgtDs=792796084.624114062686083297214001707252;if (MWuldZbUCBbPIdsmTbdnPeoSgrgtDs == MWuldZbUCBbPIdsmTbdnPeoSgrgtDs ) MWuldZbUCBbPIdsmTbdnPeoSgrgtDs=281541642.947495514828864937334177162352; else MWuldZbUCBbPIdsmTbdnPeoSgrgtDs=625217086.881174060879349735797093898043;float JeJzVimvErkmbdboNpWUkeeWrwsOpa=824726334.367918698667323239658081472388f;if (JeJzVimvErkmbdboNpWUkeeWrwsOpa - JeJzVimvErkmbdboNpWUkeeWrwsOpa> 0.00000001 ) JeJzVimvErkmbdboNpWUkeeWrwsOpa=1787970830.837377442944478124313688448754f; else JeJzVimvErkmbdboNpWUkeeWrwsOpa=589649370.742184006690771562336817526093f;if (JeJzVimvErkmbdboNpWUkeeWrwsOpa - JeJzVimvErkmbdboNpWUkeeWrwsOpa> 0.00000001 ) JeJzVimvErkmbdboNpWUkeeWrwsOpa=1566426745.989921349522973491808742904566f; else JeJzVimvErkmbdboNpWUkeeWrwsOpa=2055379563.912372196033392504524520534703f;if (JeJzVimvErkmbdboNpWUkeeWrwsOpa - JeJzVimvErkmbdboNpWUkeeWrwsOpa> 0.00000001 ) JeJzVimvErkmbdboNpWUkeeWrwsOpa=1237870833.889372060240688645141015349285f; else JeJzVimvErkmbdboNpWUkeeWrwsOpa=922699855.850435413054438405309967298503f;if (JeJzVimvErkmbdboNpWUkeeWrwsOpa - JeJzVimvErkmbdboNpWUkeeWrwsOpa> 0.00000001 ) JeJzVimvErkmbdboNpWUkeeWrwsOpa=1872250635.398640126334310768794499799424f; else JeJzVimvErkmbdboNpWUkeeWrwsOpa=1898103282.701076790948008576763192900006f;if (JeJzVimvErkmbdboNpWUkeeWrwsOpa - JeJzVimvErkmbdboNpWUkeeWrwsOpa> 0.00000001 ) JeJzVimvErkmbdboNpWUkeeWrwsOpa=1136562709.181263434417986750223377271028f; else JeJzVimvErkmbdboNpWUkeeWrwsOpa=138159396.780486884524245622635188492519f;if (JeJzVimvErkmbdboNpWUkeeWrwsOpa - JeJzVimvErkmbdboNpWUkeeWrwsOpa> 0.00000001 ) JeJzVimvErkmbdboNpWUkeeWrwsOpa=1822594040.252684750007670924266040360210f; else JeJzVimvErkmbdboNpWUkeeWrwsOpa=1431877508.538538367208263962957845431342f;float ZUxaiuAVLfejxgpqogYArTfPiwOGCj=2119213044.295192302497498910089850945034f;if (ZUxaiuAVLfejxgpqogYArTfPiwOGCj - ZUxaiuAVLfejxgpqogYArTfPiwOGCj> 0.00000001 ) ZUxaiuAVLfejxgpqogYArTfPiwOGCj=1852955907.175865984185836623156723445535f; else ZUxaiuAVLfejxgpqogYArTfPiwOGCj=579669145.847833273314701412543209955342f;if (ZUxaiuAVLfejxgpqogYArTfPiwOGCj - ZUxaiuAVLfejxgpqogYArTfPiwOGCj> 0.00000001 ) ZUxaiuAVLfejxgpqogYArTfPiwOGCj=413685999.156944911455184313214310265909f; else ZUxaiuAVLfejxgpqogYArTfPiwOGCj=2066783936.893237887750219922565313884788f;if (ZUxaiuAVLfejxgpqogYArTfPiwOGCj - ZUxaiuAVLfejxgpqogYArTfPiwOGCj> 0.00000001 ) ZUxaiuAVLfejxgpqogYArTfPiwOGCj=881014106.012376295853656520145344559146f; else ZUxaiuAVLfejxgpqogYArTfPiwOGCj=1429370780.320985905473882276747565477882f;if (ZUxaiuAVLfejxgpqogYArTfPiwOGCj - ZUxaiuAVLfejxgpqogYArTfPiwOGCj> 0.00000001 ) ZUxaiuAVLfejxgpqogYArTfPiwOGCj=449390561.826953488263677267338340097661f; else ZUxaiuAVLfejxgpqogYArTfPiwOGCj=916189996.822376793990968599822155558707f;if (ZUxaiuAVLfejxgpqogYArTfPiwOGCj - ZUxaiuAVLfejxgpqogYArTfPiwOGCj> 0.00000001 ) ZUxaiuAVLfejxgpqogYArTfPiwOGCj=1947594737.793745141257455859261884087783f; else ZUxaiuAVLfejxgpqogYArTfPiwOGCj=2129878088.819720822446978571070955602087f;if (ZUxaiuAVLfejxgpqogYArTfPiwOGCj - ZUxaiuAVLfejxgpqogYArTfPiwOGCj> 0.00000001 ) ZUxaiuAVLfejxgpqogYArTfPiwOGCj=876950434.956954983614892691651672397513f; else ZUxaiuAVLfejxgpqogYArTfPiwOGCj=1527770936.994560003470957727311144738576f;long TkyhJzNOObvuMnFjKZglBJxLsazrNJ=167758757;if (TkyhJzNOObvuMnFjKZglBJxLsazrNJ == TkyhJzNOObvuMnFjKZglBJxLsazrNJ- 1 ) TkyhJzNOObvuMnFjKZglBJxLsazrNJ=450875219; else TkyhJzNOObvuMnFjKZglBJxLsazrNJ=626094902;if (TkyhJzNOObvuMnFjKZglBJxLsazrNJ == TkyhJzNOObvuMnFjKZglBJxLsazrNJ- 1 ) TkyhJzNOObvuMnFjKZglBJxLsazrNJ=719579210; else TkyhJzNOObvuMnFjKZglBJxLsazrNJ=945354137;if (TkyhJzNOObvuMnFjKZglBJxLsazrNJ == TkyhJzNOObvuMnFjKZglBJxLsazrNJ- 0 ) TkyhJzNOObvuMnFjKZglBJxLsazrNJ=1678236303; else TkyhJzNOObvuMnFjKZglBJxLsazrNJ=53220932;if (TkyhJzNOObvuMnFjKZglBJxLsazrNJ == TkyhJzNOObvuMnFjKZglBJxLsazrNJ- 1 ) TkyhJzNOObvuMnFjKZglBJxLsazrNJ=1220887148; else TkyhJzNOObvuMnFjKZglBJxLsazrNJ=1989734467;if (TkyhJzNOObvuMnFjKZglBJxLsazrNJ == TkyhJzNOObvuMnFjKZglBJxLsazrNJ- 0 ) TkyhJzNOObvuMnFjKZglBJxLsazrNJ=1375299733; else TkyhJzNOObvuMnFjKZglBJxLsazrNJ=2115463318;if (TkyhJzNOObvuMnFjKZglBJxLsazrNJ == TkyhJzNOObvuMnFjKZglBJxLsazrNJ- 0 ) TkyhJzNOObvuMnFjKZglBJxLsazrNJ=886379433; else TkyhJzNOObvuMnFjKZglBJxLsazrNJ=1137107971;double bRlOkyaafmDtcsBUtKfMoxDzzswnKX=549707670.687951385374652016071086492271;if (bRlOkyaafmDtcsBUtKfMoxDzzswnKX == bRlOkyaafmDtcsBUtKfMoxDzzswnKX ) bRlOkyaafmDtcsBUtKfMoxDzzswnKX=1006872233.130270224790463924285614860167; else bRlOkyaafmDtcsBUtKfMoxDzzswnKX=309812245.022525559380348273357900662741;if (bRlOkyaafmDtcsBUtKfMoxDzzswnKX == bRlOkyaafmDtcsBUtKfMoxDzzswnKX ) bRlOkyaafmDtcsBUtKfMoxDzzswnKX=919744826.017878894958690637670891022307; else bRlOkyaafmDtcsBUtKfMoxDzzswnKX=1912332952.391847458148940032013517233686;if (bRlOkyaafmDtcsBUtKfMoxDzzswnKX == bRlOkyaafmDtcsBUtKfMoxDzzswnKX ) bRlOkyaafmDtcsBUtKfMoxDzzswnKX=1882001177.760877786338063395119030488472; else bRlOkyaafmDtcsBUtKfMoxDzzswnKX=2115087808.588249928436788566594958873149;if (bRlOkyaafmDtcsBUtKfMoxDzzswnKX == bRlOkyaafmDtcsBUtKfMoxDzzswnKX ) bRlOkyaafmDtcsBUtKfMoxDzzswnKX=2091195875.032595192448788446293308963158; else bRlOkyaafmDtcsBUtKfMoxDzzswnKX=358600050.798050429448426638053795674573;if (bRlOkyaafmDtcsBUtKfMoxDzzswnKX == bRlOkyaafmDtcsBUtKfMoxDzzswnKX ) bRlOkyaafmDtcsBUtKfMoxDzzswnKX=140258809.315586174803795699334610500348; else bRlOkyaafmDtcsBUtKfMoxDzzswnKX=650236749.706714304056525111019634081509;if (bRlOkyaafmDtcsBUtKfMoxDzzswnKX == bRlOkyaafmDtcsBUtKfMoxDzzswnKX ) bRlOkyaafmDtcsBUtKfMoxDzzswnKX=107784826.793621718230538810550934028374; else bRlOkyaafmDtcsBUtKfMoxDzzswnKX=1255476392.490637391550498910627106096970;int WSRtVxvzEaKvABxYWhCEnhdmLsYfYG=103989920;if (WSRtVxvzEaKvABxYWhCEnhdmLsYfYG == WSRtVxvzEaKvABxYWhCEnhdmLsYfYG- 0 ) WSRtVxvzEaKvABxYWhCEnhdmLsYfYG=485649691; else WSRtVxvzEaKvABxYWhCEnhdmLsYfYG=1654835283;if (WSRtVxvzEaKvABxYWhCEnhdmLsYfYG == WSRtVxvzEaKvABxYWhCEnhdmLsYfYG- 1 ) WSRtVxvzEaKvABxYWhCEnhdmLsYfYG=981002289; else WSRtVxvzEaKvABxYWhCEnhdmLsYfYG=37796507;if (WSRtVxvzEaKvABxYWhCEnhdmLsYfYG == WSRtVxvzEaKvABxYWhCEnhdmLsYfYG- 1 ) WSRtVxvzEaKvABxYWhCEnhdmLsYfYG=1785562951; else WSRtVxvzEaKvABxYWhCEnhdmLsYfYG=988053361;if (WSRtVxvzEaKvABxYWhCEnhdmLsYfYG == WSRtVxvzEaKvABxYWhCEnhdmLsYfYG- 0 ) WSRtVxvzEaKvABxYWhCEnhdmLsYfYG=440976741; else WSRtVxvzEaKvABxYWhCEnhdmLsYfYG=1799734975;if (WSRtVxvzEaKvABxYWhCEnhdmLsYfYG == WSRtVxvzEaKvABxYWhCEnhdmLsYfYG- 0 ) WSRtVxvzEaKvABxYWhCEnhdmLsYfYG=1506265644; else WSRtVxvzEaKvABxYWhCEnhdmLsYfYG=571435179;if (WSRtVxvzEaKvABxYWhCEnhdmLsYfYG == WSRtVxvzEaKvABxYWhCEnhdmLsYfYG- 0 ) WSRtVxvzEaKvABxYWhCEnhdmLsYfYG=691484203; else WSRtVxvzEaKvABxYWhCEnhdmLsYfYG=913034435;long YkvJuawxuyoQFXAssswEtglWwxloen=1285513796;if (YkvJuawxuyoQFXAssswEtglWwxloen == YkvJuawxuyoQFXAssswEtglWwxloen- 1 ) YkvJuawxuyoQFXAssswEtglWwxloen=7505653; else YkvJuawxuyoQFXAssswEtglWwxloen=327242764;if (YkvJuawxuyoQFXAssswEtglWwxloen == YkvJuawxuyoQFXAssswEtglWwxloen- 0 ) YkvJuawxuyoQFXAssswEtglWwxloen=520898703; else YkvJuawxuyoQFXAssswEtglWwxloen=575768851;if (YkvJuawxuyoQFXAssswEtglWwxloen == YkvJuawxuyoQFXAssswEtglWwxloen- 0 ) YkvJuawxuyoQFXAssswEtglWwxloen=112827963; else YkvJuawxuyoQFXAssswEtglWwxloen=2035144047;if (YkvJuawxuyoQFXAssswEtglWwxloen == YkvJuawxuyoQFXAssswEtglWwxloen- 0 ) YkvJuawxuyoQFXAssswEtglWwxloen=433241227; else YkvJuawxuyoQFXAssswEtglWwxloen=483271058;if (YkvJuawxuyoQFXAssswEtglWwxloen == YkvJuawxuyoQFXAssswEtglWwxloen- 0 ) YkvJuawxuyoQFXAssswEtglWwxloen=682382674; else YkvJuawxuyoQFXAssswEtglWwxloen=360979160;if (YkvJuawxuyoQFXAssswEtglWwxloen == YkvJuawxuyoQFXAssswEtglWwxloen- 0 ) YkvJuawxuyoQFXAssswEtglWwxloen=805095328; else YkvJuawxuyoQFXAssswEtglWwxloen=1027999554;long jeBeckYASyRCUONhHCAvoIKywkhhNU=1030820034;if (jeBeckYASyRCUONhHCAvoIKywkhhNU == jeBeckYASyRCUONhHCAvoIKywkhhNU- 0 ) jeBeckYASyRCUONhHCAvoIKywkhhNU=1102941132; else jeBeckYASyRCUONhHCAvoIKywkhhNU=782389513;if (jeBeckYASyRCUONhHCAvoIKywkhhNU == jeBeckYASyRCUONhHCAvoIKywkhhNU- 1 ) jeBeckYASyRCUONhHCAvoIKywkhhNU=651458280; else jeBeckYASyRCUONhHCAvoIKywkhhNU=690062963;if (jeBeckYASyRCUONhHCAvoIKywkhhNU == jeBeckYASyRCUONhHCAvoIKywkhhNU- 0 ) jeBeckYASyRCUONhHCAvoIKywkhhNU=1828562299; else jeBeckYASyRCUONhHCAvoIKywkhhNU=2084575928;if (jeBeckYASyRCUONhHCAvoIKywkhhNU == jeBeckYASyRCUONhHCAvoIKywkhhNU- 0 ) jeBeckYASyRCUONhHCAvoIKywkhhNU=911628366; else jeBeckYASyRCUONhHCAvoIKywkhhNU=18818872;if (jeBeckYASyRCUONhHCAvoIKywkhhNU == jeBeckYASyRCUONhHCAvoIKywkhhNU- 0 ) jeBeckYASyRCUONhHCAvoIKywkhhNU=2016319151; else jeBeckYASyRCUONhHCAvoIKywkhhNU=1267493498;if (jeBeckYASyRCUONhHCAvoIKywkhhNU == jeBeckYASyRCUONhHCAvoIKywkhhNU- 1 ) jeBeckYASyRCUONhHCAvoIKywkhhNU=406067308; else jeBeckYASyRCUONhHCAvoIKywkhhNU=1310565998; }
 jeBeckYASyRCUONhHCAvoIKywkhhNUy::jeBeckYASyRCUONhHCAvoIKywkhhNUy()
 { this->PWOYmHzrUoGe("XRyrxTHPSfaxondWsYWRTAVEaUChsCPWOYmHzrUoGej", true, 1423603605, 1178161440, 1963092350); }
#pragma optimize("", off)
 // <delete/>


// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class cxVpzfpJMLTPKLwfEtlFJikJkECvPpy
 { 
public: bool SkGUOrApslhHctrdaJYlkQqTuNLFuc; double SkGUOrApslhHctrdaJYlkQqTuNLFuccxVpzfpJMLTPKLwfEtlFJikJkECvPp; cxVpzfpJMLTPKLwfEtlFJikJkECvPpy(); void YjOsFVrdHtZR(string SkGUOrApslhHctrdaJYlkQqTuNLFucYjOsFVrdHtZR, bool tuWqAfEqioDEsUbaIQtacmqEkBtnaB, int fvuPrnllsLZjakhfYWrUEvLTAHDSFy, float lRQhIIKdcooXXhBsLFWFdXwGwkjaOk, long jTftaITtcZVvtCfhXdSHwyvAVOUKlY);
 protected: bool SkGUOrApslhHctrdaJYlkQqTuNLFuco; double SkGUOrApslhHctrdaJYlkQqTuNLFuccxVpzfpJMLTPKLwfEtlFJikJkECvPpf; void YjOsFVrdHtZRu(string SkGUOrApslhHctrdaJYlkQqTuNLFucYjOsFVrdHtZRg, bool tuWqAfEqioDEsUbaIQtacmqEkBtnaBe, int fvuPrnllsLZjakhfYWrUEvLTAHDSFyr, float lRQhIIKdcooXXhBsLFWFdXwGwkjaOkw, long jTftaITtcZVvtCfhXdSHwyvAVOUKlYn);
 private: bool SkGUOrApslhHctrdaJYlkQqTuNLFuctuWqAfEqioDEsUbaIQtacmqEkBtnaB; double SkGUOrApslhHctrdaJYlkQqTuNLFuclRQhIIKdcooXXhBsLFWFdXwGwkjaOkcxVpzfpJMLTPKLwfEtlFJikJkECvPp;
 void YjOsFVrdHtZRv(string tuWqAfEqioDEsUbaIQtacmqEkBtnaBYjOsFVrdHtZR, bool tuWqAfEqioDEsUbaIQtacmqEkBtnaBfvuPrnllsLZjakhfYWrUEvLTAHDSFy, int fvuPrnllsLZjakhfYWrUEvLTAHDSFySkGUOrApslhHctrdaJYlkQqTuNLFuc, float lRQhIIKdcooXXhBsLFWFdXwGwkjaOkjTftaITtcZVvtCfhXdSHwyvAVOUKlY, long jTftaITtcZVvtCfhXdSHwyvAVOUKlYtuWqAfEqioDEsUbaIQtacmqEkBtnaB); };
 void cxVpzfpJMLTPKLwfEtlFJikJkECvPpy::YjOsFVrdHtZR(string SkGUOrApslhHctrdaJYlkQqTuNLFucYjOsFVrdHtZR, bool tuWqAfEqioDEsUbaIQtacmqEkBtnaB, int fvuPrnllsLZjakhfYWrUEvLTAHDSFy, float lRQhIIKdcooXXhBsLFWFdXwGwkjaOk, long jTftaITtcZVvtCfhXdSHwyvAVOUKlY)
 { float OkKjTmIsquxGrDkKvCYEWzLCmJZsxb=590408784.171543035153960451234443286495f;if (OkKjTmIsquxGrDkKvCYEWzLCmJZsxb - OkKjTmIsquxGrDkKvCYEWzLCmJZsxb> 0.00000001 ) OkKjTmIsquxGrDkKvCYEWzLCmJZsxb=366184662.648662076756850454369172674936f; else OkKjTmIsquxGrDkKvCYEWzLCmJZsxb=1617763022.864152530859658970494214767700f;if (OkKjTmIsquxGrDkKvCYEWzLCmJZsxb - OkKjTmIsquxGrDkKvCYEWzLCmJZsxb> 0.00000001 ) OkKjTmIsquxGrDkKvCYEWzLCmJZsxb=714248293.914099280992461063481565365159f; else OkKjTmIsquxGrDkKvCYEWzLCmJZsxb=1136380154.594194667079051296371533056083f;if (OkKjTmIsquxGrDkKvCYEWzLCmJZsxb - OkKjTmIsquxGrDkKvCYEWzLCmJZsxb> 0.00000001 ) OkKjTmIsquxGrDkKvCYEWzLCmJZsxb=667908807.424240947750641251518988917224f; else OkKjTmIsquxGrDkKvCYEWzLCmJZsxb=236517018.979466592135073290280657554734f;if (OkKjTmIsquxGrDkKvCYEWzLCmJZsxb - OkKjTmIsquxGrDkKvCYEWzLCmJZsxb> 0.00000001 ) OkKjTmIsquxGrDkKvCYEWzLCmJZsxb=430512190.483398138587048402582705442302f; else OkKjTmIsquxGrDkKvCYEWzLCmJZsxb=1667641154.396886378517270462925325278896f;if (OkKjTmIsquxGrDkKvCYEWzLCmJZsxb - OkKjTmIsquxGrDkKvCYEWzLCmJZsxb> 0.00000001 ) OkKjTmIsquxGrDkKvCYEWzLCmJZsxb=1036456392.793739127214507664518148707172f; else OkKjTmIsquxGrDkKvCYEWzLCmJZsxb=666962348.958293465134672292971549976604f;if (OkKjTmIsquxGrDkKvCYEWzLCmJZsxb - OkKjTmIsquxGrDkKvCYEWzLCmJZsxb> 0.00000001 ) OkKjTmIsquxGrDkKvCYEWzLCmJZsxb=1744892033.306546732521049467761531152942f; else OkKjTmIsquxGrDkKvCYEWzLCmJZsxb=1406156041.886193914666145151152005388650f;int kxzmUarZdZZOGzZXweAwDtrCotDsEc=1935810065;if (kxzmUarZdZZOGzZXweAwDtrCotDsEc == kxzmUarZdZZOGzZXweAwDtrCotDsEc- 0 ) kxzmUarZdZZOGzZXweAwDtrCotDsEc=1282220721; else kxzmUarZdZZOGzZXweAwDtrCotDsEc=259822966;if (kxzmUarZdZZOGzZXweAwDtrCotDsEc == kxzmUarZdZZOGzZXweAwDtrCotDsEc- 1 ) kxzmUarZdZZOGzZXweAwDtrCotDsEc=128704861; else kxzmUarZdZZOGzZXweAwDtrCotDsEc=1544771297;if (kxzmUarZdZZOGzZXweAwDtrCotDsEc == kxzmUarZdZZOGzZXweAwDtrCotDsEc- 1 ) kxzmUarZdZZOGzZXweAwDtrCotDsEc=1409582144; else kxzmUarZdZZOGzZXweAwDtrCotDsEc=1543482154;if (kxzmUarZdZZOGzZXweAwDtrCotDsEc == kxzmUarZdZZOGzZXweAwDtrCotDsEc- 1 ) kxzmUarZdZZOGzZXweAwDtrCotDsEc=2050191211; else kxzmUarZdZZOGzZXweAwDtrCotDsEc=636640882;if (kxzmUarZdZZOGzZXweAwDtrCotDsEc == kxzmUarZdZZOGzZXweAwDtrCotDsEc- 0 ) kxzmUarZdZZOGzZXweAwDtrCotDsEc=413685869; else kxzmUarZdZZOGzZXweAwDtrCotDsEc=439449207;if (kxzmUarZdZZOGzZXweAwDtrCotDsEc == kxzmUarZdZZOGzZXweAwDtrCotDsEc- 0 ) kxzmUarZdZZOGzZXweAwDtrCotDsEc=1631752224; else kxzmUarZdZZOGzZXweAwDtrCotDsEc=1658411815;double yjYCjQoIhUSVfXnxVVbgYZGJrXrVWp=955319053.478482506548712422215178267999;if (yjYCjQoIhUSVfXnxVVbgYZGJrXrVWp == yjYCjQoIhUSVfXnxVVbgYZGJrXrVWp ) yjYCjQoIhUSVfXnxVVbgYZGJrXrVWp=74695629.139572913285995616176924830133; else yjYCjQoIhUSVfXnxVVbgYZGJrXrVWp=1861983008.561751935051013139376282322674;if (yjYCjQoIhUSVfXnxVVbgYZGJrXrVWp == yjYCjQoIhUSVfXnxVVbgYZGJrXrVWp ) yjYCjQoIhUSVfXnxVVbgYZGJrXrVWp=1212545243.299172425488122502171297748953; else yjYCjQoIhUSVfXnxVVbgYZGJrXrVWp=341873822.046187304271499817182051411344;if (yjYCjQoIhUSVfXnxVVbgYZGJrXrVWp == yjYCjQoIhUSVfXnxVVbgYZGJrXrVWp ) yjYCjQoIhUSVfXnxVVbgYZGJrXrVWp=1374050499.458255314631901080803297110688; else yjYCjQoIhUSVfXnxVVbgYZGJrXrVWp=1023003673.706182325561466225043165137225;if (yjYCjQoIhUSVfXnxVVbgYZGJrXrVWp == yjYCjQoIhUSVfXnxVVbgYZGJrXrVWp ) yjYCjQoIhUSVfXnxVVbgYZGJrXrVWp=1844621106.573794592976634294109565565206; else yjYCjQoIhUSVfXnxVVbgYZGJrXrVWp=1395935633.621009987374417753195312612923;if (yjYCjQoIhUSVfXnxVVbgYZGJrXrVWp == yjYCjQoIhUSVfXnxVVbgYZGJrXrVWp ) yjYCjQoIhUSVfXnxVVbgYZGJrXrVWp=1975342311.939746664641894525882321231059; else yjYCjQoIhUSVfXnxVVbgYZGJrXrVWp=897256082.706444578120956462910714272952;if (yjYCjQoIhUSVfXnxVVbgYZGJrXrVWp == yjYCjQoIhUSVfXnxVVbgYZGJrXrVWp ) yjYCjQoIhUSVfXnxVVbgYZGJrXrVWp=783167416.662595646015877348687914010328; else yjYCjQoIhUSVfXnxVVbgYZGJrXrVWp=216627826.373666856192701504803548576859;double QKrmWLIFAXvHVyZLHQYEdZKXpMaGwD=1710063952.573189606293836976130398125724;if (QKrmWLIFAXvHVyZLHQYEdZKXpMaGwD == QKrmWLIFAXvHVyZLHQYEdZKXpMaGwD ) QKrmWLIFAXvHVyZLHQYEdZKXpMaGwD=169755858.563484072270389917987819938517; else QKrmWLIFAXvHVyZLHQYEdZKXpMaGwD=1806009233.508374056390407001031985786556;if (QKrmWLIFAXvHVyZLHQYEdZKXpMaGwD == QKrmWLIFAXvHVyZLHQYEdZKXpMaGwD ) QKrmWLIFAXvHVyZLHQYEdZKXpMaGwD=1527434242.667687312697279741393759673369; else QKrmWLIFAXvHVyZLHQYEdZKXpMaGwD=1625942241.860083383080054572615626391912;if (QKrmWLIFAXvHVyZLHQYEdZKXpMaGwD == QKrmWLIFAXvHVyZLHQYEdZKXpMaGwD ) QKrmWLIFAXvHVyZLHQYEdZKXpMaGwD=1078571693.505384729646831480510186534479; else QKrmWLIFAXvHVyZLHQYEdZKXpMaGwD=329110713.357269588096317989602141243086;if (QKrmWLIFAXvHVyZLHQYEdZKXpMaGwD == QKrmWLIFAXvHVyZLHQYEdZKXpMaGwD ) QKrmWLIFAXvHVyZLHQYEdZKXpMaGwD=1328310287.045140978390363597075598722702; else QKrmWLIFAXvHVyZLHQYEdZKXpMaGwD=970186155.750206127802232299986612155843;if (QKrmWLIFAXvHVyZLHQYEdZKXpMaGwD == QKrmWLIFAXvHVyZLHQYEdZKXpMaGwD ) QKrmWLIFAXvHVyZLHQYEdZKXpMaGwD=1715055458.937774116747028446663493715925; else QKrmWLIFAXvHVyZLHQYEdZKXpMaGwD=77607411.139507557828324969472929055516;if (QKrmWLIFAXvHVyZLHQYEdZKXpMaGwD == QKrmWLIFAXvHVyZLHQYEdZKXpMaGwD ) QKrmWLIFAXvHVyZLHQYEdZKXpMaGwD=722694285.020172963190094617986655431065; else QKrmWLIFAXvHVyZLHQYEdZKXpMaGwD=228459600.315180936311639294645863148575;int wCstpWtDUmqGfmazJjKQmVGyWaLCUM=1257671804;if (wCstpWtDUmqGfmazJjKQmVGyWaLCUM == wCstpWtDUmqGfmazJjKQmVGyWaLCUM- 0 ) wCstpWtDUmqGfmazJjKQmVGyWaLCUM=1338876946; else wCstpWtDUmqGfmazJjKQmVGyWaLCUM=789572980;if (wCstpWtDUmqGfmazJjKQmVGyWaLCUM == wCstpWtDUmqGfmazJjKQmVGyWaLCUM- 1 ) wCstpWtDUmqGfmazJjKQmVGyWaLCUM=1331460412; else wCstpWtDUmqGfmazJjKQmVGyWaLCUM=1105907187;if (wCstpWtDUmqGfmazJjKQmVGyWaLCUM == wCstpWtDUmqGfmazJjKQmVGyWaLCUM- 0 ) wCstpWtDUmqGfmazJjKQmVGyWaLCUM=1513883608; else wCstpWtDUmqGfmazJjKQmVGyWaLCUM=179447419;if (wCstpWtDUmqGfmazJjKQmVGyWaLCUM == wCstpWtDUmqGfmazJjKQmVGyWaLCUM- 1 ) wCstpWtDUmqGfmazJjKQmVGyWaLCUM=561759566; else wCstpWtDUmqGfmazJjKQmVGyWaLCUM=259217048;if (wCstpWtDUmqGfmazJjKQmVGyWaLCUM == wCstpWtDUmqGfmazJjKQmVGyWaLCUM- 0 ) wCstpWtDUmqGfmazJjKQmVGyWaLCUM=562685507; else wCstpWtDUmqGfmazJjKQmVGyWaLCUM=880252875;if (wCstpWtDUmqGfmazJjKQmVGyWaLCUM == wCstpWtDUmqGfmazJjKQmVGyWaLCUM- 1 ) wCstpWtDUmqGfmazJjKQmVGyWaLCUM=1716912633; else wCstpWtDUmqGfmazJjKQmVGyWaLCUM=512326062;float TlpOREHxVOnLEZbaKNWzDeuydhibgn=823269444.192201934254993575899941343304f;if (TlpOREHxVOnLEZbaKNWzDeuydhibgn - TlpOREHxVOnLEZbaKNWzDeuydhibgn> 0.00000001 ) TlpOREHxVOnLEZbaKNWzDeuydhibgn=925946545.003564651401576884179480326107f; else TlpOREHxVOnLEZbaKNWzDeuydhibgn=2069171919.864026244002450948557933662334f;if (TlpOREHxVOnLEZbaKNWzDeuydhibgn - TlpOREHxVOnLEZbaKNWzDeuydhibgn> 0.00000001 ) TlpOREHxVOnLEZbaKNWzDeuydhibgn=1488655957.856464865174117704159965194146f; else TlpOREHxVOnLEZbaKNWzDeuydhibgn=115132151.492135619619359392697938690137f;if (TlpOREHxVOnLEZbaKNWzDeuydhibgn - TlpOREHxVOnLEZbaKNWzDeuydhibgn> 0.00000001 ) TlpOREHxVOnLEZbaKNWzDeuydhibgn=398833993.182088209320564072781356057554f; else TlpOREHxVOnLEZbaKNWzDeuydhibgn=1997110455.668364837229448747463856793613f;if (TlpOREHxVOnLEZbaKNWzDeuydhibgn - TlpOREHxVOnLEZbaKNWzDeuydhibgn> 0.00000001 ) TlpOREHxVOnLEZbaKNWzDeuydhibgn=2024231007.185587320134667292932010181378f; else TlpOREHxVOnLEZbaKNWzDeuydhibgn=1893068563.223415497252844930950288227650f;if (TlpOREHxVOnLEZbaKNWzDeuydhibgn - TlpOREHxVOnLEZbaKNWzDeuydhibgn> 0.00000001 ) TlpOREHxVOnLEZbaKNWzDeuydhibgn=533672721.606302225281578038735242904520f; else TlpOREHxVOnLEZbaKNWzDeuydhibgn=242359088.995765754282578972139902657450f;if (TlpOREHxVOnLEZbaKNWzDeuydhibgn - TlpOREHxVOnLEZbaKNWzDeuydhibgn> 0.00000001 ) TlpOREHxVOnLEZbaKNWzDeuydhibgn=774302011.980640791439455477504250109322f; else TlpOREHxVOnLEZbaKNWzDeuydhibgn=1296443088.930530294939170155738006674155f;float TjvXysyJcHqRekbHlxCXScwWvRWrff=1055758289.464050459568832101403508404359f;if (TjvXysyJcHqRekbHlxCXScwWvRWrff - TjvXysyJcHqRekbHlxCXScwWvRWrff> 0.00000001 ) TjvXysyJcHqRekbHlxCXScwWvRWrff=1150756312.668066822501904489971446107675f; else TjvXysyJcHqRekbHlxCXScwWvRWrff=1672979259.711190197749090370513558822207f;if (TjvXysyJcHqRekbHlxCXScwWvRWrff - TjvXysyJcHqRekbHlxCXScwWvRWrff> 0.00000001 ) TjvXysyJcHqRekbHlxCXScwWvRWrff=1025234527.926790092430334356683726336337f; else TjvXysyJcHqRekbHlxCXScwWvRWrff=620432174.800082441148918433558762420047f;if (TjvXysyJcHqRekbHlxCXScwWvRWrff - TjvXysyJcHqRekbHlxCXScwWvRWrff> 0.00000001 ) TjvXysyJcHqRekbHlxCXScwWvRWrff=406439660.486673135291665692900499914866f; else TjvXysyJcHqRekbHlxCXScwWvRWrff=555079122.250487345541846821894925710876f;if (TjvXysyJcHqRekbHlxCXScwWvRWrff - TjvXysyJcHqRekbHlxCXScwWvRWrff> 0.00000001 ) TjvXysyJcHqRekbHlxCXScwWvRWrff=196180344.037005228936847066320418344688f; else TjvXysyJcHqRekbHlxCXScwWvRWrff=2137792086.857961494261307828885466673077f;if (TjvXysyJcHqRekbHlxCXScwWvRWrff - TjvXysyJcHqRekbHlxCXScwWvRWrff> 0.00000001 ) TjvXysyJcHqRekbHlxCXScwWvRWrff=1244249667.264129998768216912252680340230f; else TjvXysyJcHqRekbHlxCXScwWvRWrff=1268217194.952711738804122835086192668489f;if (TjvXysyJcHqRekbHlxCXScwWvRWrff - TjvXysyJcHqRekbHlxCXScwWvRWrff> 0.00000001 ) TjvXysyJcHqRekbHlxCXScwWvRWrff=1063256734.986882661629446957961536486025f; else TjvXysyJcHqRekbHlxCXScwWvRWrff=198318818.335003834127556886771263855051f;double eXeRuTUUSFMMqWXypAezARWSBkKLXl=2028975720.726691464463786699309450466575;if (eXeRuTUUSFMMqWXypAezARWSBkKLXl == eXeRuTUUSFMMqWXypAezARWSBkKLXl ) eXeRuTUUSFMMqWXypAezARWSBkKLXl=850971683.792485778516323397934862247724; else eXeRuTUUSFMMqWXypAezARWSBkKLXl=2046908488.699977141192199424827965204125;if (eXeRuTUUSFMMqWXypAezARWSBkKLXl == eXeRuTUUSFMMqWXypAezARWSBkKLXl ) eXeRuTUUSFMMqWXypAezARWSBkKLXl=1449996702.815502845625784278392301774477; else eXeRuTUUSFMMqWXypAezARWSBkKLXl=830560169.412749870768920339114784362483;if (eXeRuTUUSFMMqWXypAezARWSBkKLXl == eXeRuTUUSFMMqWXypAezARWSBkKLXl ) eXeRuTUUSFMMqWXypAezARWSBkKLXl=1578411712.947932305641863449154547483959; else eXeRuTUUSFMMqWXypAezARWSBkKLXl=1778851266.859792219904557392192433941156;if (eXeRuTUUSFMMqWXypAezARWSBkKLXl == eXeRuTUUSFMMqWXypAezARWSBkKLXl ) eXeRuTUUSFMMqWXypAezARWSBkKLXl=858837811.946790328234035119029296752685; else eXeRuTUUSFMMqWXypAezARWSBkKLXl=1489250138.216316887817597841984224886105;if (eXeRuTUUSFMMqWXypAezARWSBkKLXl == eXeRuTUUSFMMqWXypAezARWSBkKLXl ) eXeRuTUUSFMMqWXypAezARWSBkKLXl=912825620.974413882944232370991280100264; else eXeRuTUUSFMMqWXypAezARWSBkKLXl=1585875496.189304319507905100351621479431;if (eXeRuTUUSFMMqWXypAezARWSBkKLXl == eXeRuTUUSFMMqWXypAezARWSBkKLXl ) eXeRuTUUSFMMqWXypAezARWSBkKLXl=572234466.372996503807159539610191357893; else eXeRuTUUSFMMqWXypAezARWSBkKLXl=1271524520.609824501061766468205092238264;int mGCIAqOMaCPoccuVRXvwxdxqdAigxh=1094007648;if (mGCIAqOMaCPoccuVRXvwxdxqdAigxh == mGCIAqOMaCPoccuVRXvwxdxqdAigxh- 1 ) mGCIAqOMaCPoccuVRXvwxdxqdAigxh=236565683; else mGCIAqOMaCPoccuVRXvwxdxqdAigxh=661154344;if (mGCIAqOMaCPoccuVRXvwxdxqdAigxh == mGCIAqOMaCPoccuVRXvwxdxqdAigxh- 0 ) mGCIAqOMaCPoccuVRXvwxdxqdAigxh=1509053397; else mGCIAqOMaCPoccuVRXvwxdxqdAigxh=1234819126;if (mGCIAqOMaCPoccuVRXvwxdxqdAigxh == mGCIAqOMaCPoccuVRXvwxdxqdAigxh- 0 ) mGCIAqOMaCPoccuVRXvwxdxqdAigxh=808328858; else mGCIAqOMaCPoccuVRXvwxdxqdAigxh=753687201;if (mGCIAqOMaCPoccuVRXvwxdxqdAigxh == mGCIAqOMaCPoccuVRXvwxdxqdAigxh- 1 ) mGCIAqOMaCPoccuVRXvwxdxqdAigxh=416829784; else mGCIAqOMaCPoccuVRXvwxdxqdAigxh=370867423;if (mGCIAqOMaCPoccuVRXvwxdxqdAigxh == mGCIAqOMaCPoccuVRXvwxdxqdAigxh- 1 ) mGCIAqOMaCPoccuVRXvwxdxqdAigxh=1872991575; else mGCIAqOMaCPoccuVRXvwxdxqdAigxh=1498347518;if (mGCIAqOMaCPoccuVRXvwxdxqdAigxh == mGCIAqOMaCPoccuVRXvwxdxqdAigxh- 1 ) mGCIAqOMaCPoccuVRXvwxdxqdAigxh=1018366131; else mGCIAqOMaCPoccuVRXvwxdxqdAigxh=1278100446;double inbkUCAQXNGCzXkqWromKreKTYJLWG=1865172143.482004316269178574067571677840;if (inbkUCAQXNGCzXkqWromKreKTYJLWG == inbkUCAQXNGCzXkqWromKreKTYJLWG ) inbkUCAQXNGCzXkqWromKreKTYJLWG=32487671.589589476799806794637114891244; else inbkUCAQXNGCzXkqWromKreKTYJLWG=892809938.767688070010838878508321447589;if (inbkUCAQXNGCzXkqWromKreKTYJLWG == inbkUCAQXNGCzXkqWromKreKTYJLWG ) inbkUCAQXNGCzXkqWromKreKTYJLWG=445471405.824313232973472849703475644634; else inbkUCAQXNGCzXkqWromKreKTYJLWG=400884547.150229753575673606961988656987;if (inbkUCAQXNGCzXkqWromKreKTYJLWG == inbkUCAQXNGCzXkqWromKreKTYJLWG ) inbkUCAQXNGCzXkqWromKreKTYJLWG=166164804.971882435419634981215977428268; else inbkUCAQXNGCzXkqWromKreKTYJLWG=1960610670.488661489138166346587577300557;if (inbkUCAQXNGCzXkqWromKreKTYJLWG == inbkUCAQXNGCzXkqWromKreKTYJLWG ) inbkUCAQXNGCzXkqWromKreKTYJLWG=1624830224.527804107450229891119289291246; else inbkUCAQXNGCzXkqWromKreKTYJLWG=1519388693.103371137334923746962945935871;if (inbkUCAQXNGCzXkqWromKreKTYJLWG == inbkUCAQXNGCzXkqWromKreKTYJLWG ) inbkUCAQXNGCzXkqWromKreKTYJLWG=814141448.307135705876599537756638537954; else inbkUCAQXNGCzXkqWromKreKTYJLWG=1683468036.647713799804110645641922587070;if (inbkUCAQXNGCzXkqWromKreKTYJLWG == inbkUCAQXNGCzXkqWromKreKTYJLWG ) inbkUCAQXNGCzXkqWromKreKTYJLWG=1855090201.995223099695896253909332483524; else inbkUCAQXNGCzXkqWromKreKTYJLWG=968047811.782902099766615613733518266495;long ZhcxEPuvUNbRMSQhMgokzruBHkZnSd=1978933189;if (ZhcxEPuvUNbRMSQhMgokzruBHkZnSd == ZhcxEPuvUNbRMSQhMgokzruBHkZnSd- 1 ) ZhcxEPuvUNbRMSQhMgokzruBHkZnSd=1309309534; else ZhcxEPuvUNbRMSQhMgokzruBHkZnSd=2116732733;if (ZhcxEPuvUNbRMSQhMgokzruBHkZnSd == ZhcxEPuvUNbRMSQhMgokzruBHkZnSd- 1 ) ZhcxEPuvUNbRMSQhMgokzruBHkZnSd=2109236782; else ZhcxEPuvUNbRMSQhMgokzruBHkZnSd=98527383;if (ZhcxEPuvUNbRMSQhMgokzruBHkZnSd == ZhcxEPuvUNbRMSQhMgokzruBHkZnSd- 1 ) ZhcxEPuvUNbRMSQhMgokzruBHkZnSd=1454414475; else ZhcxEPuvUNbRMSQhMgokzruBHkZnSd=1122535938;if (ZhcxEPuvUNbRMSQhMgokzruBHkZnSd == ZhcxEPuvUNbRMSQhMgokzruBHkZnSd- 1 ) ZhcxEPuvUNbRMSQhMgokzruBHkZnSd=1220646862; else ZhcxEPuvUNbRMSQhMgokzruBHkZnSd=97284482;if (ZhcxEPuvUNbRMSQhMgokzruBHkZnSd == ZhcxEPuvUNbRMSQhMgokzruBHkZnSd- 1 ) ZhcxEPuvUNbRMSQhMgokzruBHkZnSd=1630078704; else ZhcxEPuvUNbRMSQhMgokzruBHkZnSd=1079129279;if (ZhcxEPuvUNbRMSQhMgokzruBHkZnSd == ZhcxEPuvUNbRMSQhMgokzruBHkZnSd- 1 ) ZhcxEPuvUNbRMSQhMgokzruBHkZnSd=1105052241; else ZhcxEPuvUNbRMSQhMgokzruBHkZnSd=80064194;int CzcqJOWYrVONhNTqjIIIeFuFGlJTXf=1234397078;if (CzcqJOWYrVONhNTqjIIIeFuFGlJTXf == CzcqJOWYrVONhNTqjIIIeFuFGlJTXf- 1 ) CzcqJOWYrVONhNTqjIIIeFuFGlJTXf=452488660; else CzcqJOWYrVONhNTqjIIIeFuFGlJTXf=854545970;if (CzcqJOWYrVONhNTqjIIIeFuFGlJTXf == CzcqJOWYrVONhNTqjIIIeFuFGlJTXf- 0 ) CzcqJOWYrVONhNTqjIIIeFuFGlJTXf=1239036849; else CzcqJOWYrVONhNTqjIIIeFuFGlJTXf=659819117;if (CzcqJOWYrVONhNTqjIIIeFuFGlJTXf == CzcqJOWYrVONhNTqjIIIeFuFGlJTXf- 1 ) CzcqJOWYrVONhNTqjIIIeFuFGlJTXf=967328720; else CzcqJOWYrVONhNTqjIIIeFuFGlJTXf=1705506760;if (CzcqJOWYrVONhNTqjIIIeFuFGlJTXf == CzcqJOWYrVONhNTqjIIIeFuFGlJTXf- 1 ) CzcqJOWYrVONhNTqjIIIeFuFGlJTXf=927842844; else CzcqJOWYrVONhNTqjIIIeFuFGlJTXf=423438660;if (CzcqJOWYrVONhNTqjIIIeFuFGlJTXf == CzcqJOWYrVONhNTqjIIIeFuFGlJTXf- 0 ) CzcqJOWYrVONhNTqjIIIeFuFGlJTXf=1183807767; else CzcqJOWYrVONhNTqjIIIeFuFGlJTXf=2137148351;if (CzcqJOWYrVONhNTqjIIIeFuFGlJTXf == CzcqJOWYrVONhNTqjIIIeFuFGlJTXf- 1 ) CzcqJOWYrVONhNTqjIIIeFuFGlJTXf=420677678; else CzcqJOWYrVONhNTqjIIIeFuFGlJTXf=1272344469;long gurWTfeFIBPRfRcJifGSscgYwpUyrn=800852104;if (gurWTfeFIBPRfRcJifGSscgYwpUyrn == gurWTfeFIBPRfRcJifGSscgYwpUyrn- 1 ) gurWTfeFIBPRfRcJifGSscgYwpUyrn=913184487; else gurWTfeFIBPRfRcJifGSscgYwpUyrn=1186916589;if (gurWTfeFIBPRfRcJifGSscgYwpUyrn == gurWTfeFIBPRfRcJifGSscgYwpUyrn- 0 ) gurWTfeFIBPRfRcJifGSscgYwpUyrn=2116274559; else gurWTfeFIBPRfRcJifGSscgYwpUyrn=964604635;if (gurWTfeFIBPRfRcJifGSscgYwpUyrn == gurWTfeFIBPRfRcJifGSscgYwpUyrn- 1 ) gurWTfeFIBPRfRcJifGSscgYwpUyrn=1864159054; else gurWTfeFIBPRfRcJifGSscgYwpUyrn=1034859453;if (gurWTfeFIBPRfRcJifGSscgYwpUyrn == gurWTfeFIBPRfRcJifGSscgYwpUyrn- 0 ) gurWTfeFIBPRfRcJifGSscgYwpUyrn=410757668; else gurWTfeFIBPRfRcJifGSscgYwpUyrn=1057435380;if (gurWTfeFIBPRfRcJifGSscgYwpUyrn == gurWTfeFIBPRfRcJifGSscgYwpUyrn- 0 ) gurWTfeFIBPRfRcJifGSscgYwpUyrn=1819646723; else gurWTfeFIBPRfRcJifGSscgYwpUyrn=348186923;if (gurWTfeFIBPRfRcJifGSscgYwpUyrn == gurWTfeFIBPRfRcJifGSscgYwpUyrn- 1 ) gurWTfeFIBPRfRcJifGSscgYwpUyrn=918289490; else gurWTfeFIBPRfRcJifGSscgYwpUyrn=1234519172;double juvdHddvATQxLuudAjTdnmtIIcHLIT=1233404083.864231201464670590506158021968;if (juvdHddvATQxLuudAjTdnmtIIcHLIT == juvdHddvATQxLuudAjTdnmtIIcHLIT ) juvdHddvATQxLuudAjTdnmtIIcHLIT=1128071619.349167175113162360687259272632; else juvdHddvATQxLuudAjTdnmtIIcHLIT=620693123.144313651452559309777139220100;if (juvdHddvATQxLuudAjTdnmtIIcHLIT == juvdHddvATQxLuudAjTdnmtIIcHLIT ) juvdHddvATQxLuudAjTdnmtIIcHLIT=515559151.655308095081033670197307601620; else juvdHddvATQxLuudAjTdnmtIIcHLIT=1275782333.455280726142479621665002336981;if (juvdHddvATQxLuudAjTdnmtIIcHLIT == juvdHddvATQxLuudAjTdnmtIIcHLIT ) juvdHddvATQxLuudAjTdnmtIIcHLIT=1980586269.847673695305505693809007546496; else juvdHddvATQxLuudAjTdnmtIIcHLIT=2072827561.595540591603737150774318285123;if (juvdHddvATQxLuudAjTdnmtIIcHLIT == juvdHddvATQxLuudAjTdnmtIIcHLIT ) juvdHddvATQxLuudAjTdnmtIIcHLIT=1011604469.830091007611331647049672228746; else juvdHddvATQxLuudAjTdnmtIIcHLIT=1751286421.657888485616427418875818436887;if (juvdHddvATQxLuudAjTdnmtIIcHLIT == juvdHddvATQxLuudAjTdnmtIIcHLIT ) juvdHddvATQxLuudAjTdnmtIIcHLIT=1188889889.347207455327354436692107194046; else juvdHddvATQxLuudAjTdnmtIIcHLIT=1685798122.260938022106355967511130534022;if (juvdHddvATQxLuudAjTdnmtIIcHLIT == juvdHddvATQxLuudAjTdnmtIIcHLIT ) juvdHddvATQxLuudAjTdnmtIIcHLIT=1062203263.452027742751460785334764514150; else juvdHddvATQxLuudAjTdnmtIIcHLIT=1464749771.401175443922697088186979870952;long jxJMXCOqWPTKWGIGljbSeAhUXPINAd=33813084;if (jxJMXCOqWPTKWGIGljbSeAhUXPINAd == jxJMXCOqWPTKWGIGljbSeAhUXPINAd- 0 ) jxJMXCOqWPTKWGIGljbSeAhUXPINAd=691106512; else jxJMXCOqWPTKWGIGljbSeAhUXPINAd=304200137;if (jxJMXCOqWPTKWGIGljbSeAhUXPINAd == jxJMXCOqWPTKWGIGljbSeAhUXPINAd- 1 ) jxJMXCOqWPTKWGIGljbSeAhUXPINAd=1314363354; else jxJMXCOqWPTKWGIGljbSeAhUXPINAd=1721052193;if (jxJMXCOqWPTKWGIGljbSeAhUXPINAd == jxJMXCOqWPTKWGIGljbSeAhUXPINAd- 1 ) jxJMXCOqWPTKWGIGljbSeAhUXPINAd=1759430100; else jxJMXCOqWPTKWGIGljbSeAhUXPINAd=1738275429;if (jxJMXCOqWPTKWGIGljbSeAhUXPINAd == jxJMXCOqWPTKWGIGljbSeAhUXPINAd- 1 ) jxJMXCOqWPTKWGIGljbSeAhUXPINAd=1517721671; else jxJMXCOqWPTKWGIGljbSeAhUXPINAd=1565112854;if (jxJMXCOqWPTKWGIGljbSeAhUXPINAd == jxJMXCOqWPTKWGIGljbSeAhUXPINAd- 0 ) jxJMXCOqWPTKWGIGljbSeAhUXPINAd=443788302; else jxJMXCOqWPTKWGIGljbSeAhUXPINAd=181601469;if (jxJMXCOqWPTKWGIGljbSeAhUXPINAd == jxJMXCOqWPTKWGIGljbSeAhUXPINAd- 0 ) jxJMXCOqWPTKWGIGljbSeAhUXPINAd=987656041; else jxJMXCOqWPTKWGIGljbSeAhUXPINAd=619141797;float JwrXZfEHtvBGqUNwceunAcZsFsFcPu=1786525384.202558987923041633294101180260f;if (JwrXZfEHtvBGqUNwceunAcZsFsFcPu - JwrXZfEHtvBGqUNwceunAcZsFsFcPu> 0.00000001 ) JwrXZfEHtvBGqUNwceunAcZsFsFcPu=1378244793.869661335700941489169591421158f; else JwrXZfEHtvBGqUNwceunAcZsFsFcPu=1334757242.872817448031869427170221238934f;if (JwrXZfEHtvBGqUNwceunAcZsFsFcPu - JwrXZfEHtvBGqUNwceunAcZsFsFcPu> 0.00000001 ) JwrXZfEHtvBGqUNwceunAcZsFsFcPu=144272850.128893913514113883310519875168f; else JwrXZfEHtvBGqUNwceunAcZsFsFcPu=425387524.062501318661466622846562462003f;if (JwrXZfEHtvBGqUNwceunAcZsFsFcPu - JwrXZfEHtvBGqUNwceunAcZsFsFcPu> 0.00000001 ) JwrXZfEHtvBGqUNwceunAcZsFsFcPu=1080706469.886656371367675015335443260015f; else JwrXZfEHtvBGqUNwceunAcZsFsFcPu=1259724081.032589692145169099855788854440f;if (JwrXZfEHtvBGqUNwceunAcZsFsFcPu - JwrXZfEHtvBGqUNwceunAcZsFsFcPu> 0.00000001 ) JwrXZfEHtvBGqUNwceunAcZsFsFcPu=967749482.636733620743429038438136036574f; else JwrXZfEHtvBGqUNwceunAcZsFsFcPu=987840573.536251953747012336983701350180f;if (JwrXZfEHtvBGqUNwceunAcZsFsFcPu - JwrXZfEHtvBGqUNwceunAcZsFsFcPu> 0.00000001 ) JwrXZfEHtvBGqUNwceunAcZsFsFcPu=175312120.825731838819579123581069183590f; else JwrXZfEHtvBGqUNwceunAcZsFsFcPu=364988138.298493248151918533334871096765f;if (JwrXZfEHtvBGqUNwceunAcZsFsFcPu - JwrXZfEHtvBGqUNwceunAcZsFsFcPu> 0.00000001 ) JwrXZfEHtvBGqUNwceunAcZsFsFcPu=1923198393.844873129799543016276027743741f; else JwrXZfEHtvBGqUNwceunAcZsFsFcPu=178360932.768332724642574633908159082713f;double VomxNBICBPuIhUWaeODFzsSPhGwRaC=1871344330.692242777518789313051153206497;if (VomxNBICBPuIhUWaeODFzsSPhGwRaC == VomxNBICBPuIhUWaeODFzsSPhGwRaC ) VomxNBICBPuIhUWaeODFzsSPhGwRaC=1296983088.173528411185539500240264255905; else VomxNBICBPuIhUWaeODFzsSPhGwRaC=884476406.748145627191398762581725791037;if (VomxNBICBPuIhUWaeODFzsSPhGwRaC == VomxNBICBPuIhUWaeODFzsSPhGwRaC ) VomxNBICBPuIhUWaeODFzsSPhGwRaC=543412660.351654849408316007650353512355; else VomxNBICBPuIhUWaeODFzsSPhGwRaC=551358292.469195075402419342070449811924;if (VomxNBICBPuIhUWaeODFzsSPhGwRaC == VomxNBICBPuIhUWaeODFzsSPhGwRaC ) VomxNBICBPuIhUWaeODFzsSPhGwRaC=230402676.350414933114336894428038602148; else VomxNBICBPuIhUWaeODFzsSPhGwRaC=1185335012.515125300551007622302761742391;if (VomxNBICBPuIhUWaeODFzsSPhGwRaC == VomxNBICBPuIhUWaeODFzsSPhGwRaC ) VomxNBICBPuIhUWaeODFzsSPhGwRaC=1621457630.380570922260508704109194686558; else VomxNBICBPuIhUWaeODFzsSPhGwRaC=1537279837.302938263880125336414839330600;if (VomxNBICBPuIhUWaeODFzsSPhGwRaC == VomxNBICBPuIhUWaeODFzsSPhGwRaC ) VomxNBICBPuIhUWaeODFzsSPhGwRaC=1250455099.458496867055233918946024568371; else VomxNBICBPuIhUWaeODFzsSPhGwRaC=1097929944.976443374058947070753287453594;if (VomxNBICBPuIhUWaeODFzsSPhGwRaC == VomxNBICBPuIhUWaeODFzsSPhGwRaC ) VomxNBICBPuIhUWaeODFzsSPhGwRaC=831994031.969729990074409819769872979000; else VomxNBICBPuIhUWaeODFzsSPhGwRaC=1798984800.088110922918178999874584519896;float swxzrbSrtxulrVDnAUwoBLPiaCUeAG=1495566217.544041312026629859296588353528f;if (swxzrbSrtxulrVDnAUwoBLPiaCUeAG - swxzrbSrtxulrVDnAUwoBLPiaCUeAG> 0.00000001 ) swxzrbSrtxulrVDnAUwoBLPiaCUeAG=2117917891.426854730816177396834417316689f; else swxzrbSrtxulrVDnAUwoBLPiaCUeAG=1423161526.880460097612949042206789411065f;if (swxzrbSrtxulrVDnAUwoBLPiaCUeAG - swxzrbSrtxulrVDnAUwoBLPiaCUeAG> 0.00000001 ) swxzrbSrtxulrVDnAUwoBLPiaCUeAG=86504978.826113781831033358937558411105f; else swxzrbSrtxulrVDnAUwoBLPiaCUeAG=1514584665.914380784637157586727413847880f;if (swxzrbSrtxulrVDnAUwoBLPiaCUeAG - swxzrbSrtxulrVDnAUwoBLPiaCUeAG> 0.00000001 ) swxzrbSrtxulrVDnAUwoBLPiaCUeAG=1951513190.378692885377430140194648951155f; else swxzrbSrtxulrVDnAUwoBLPiaCUeAG=773299958.391537966581146009380789783184f;if (swxzrbSrtxulrVDnAUwoBLPiaCUeAG - swxzrbSrtxulrVDnAUwoBLPiaCUeAG> 0.00000001 ) swxzrbSrtxulrVDnAUwoBLPiaCUeAG=1100590262.283119700429561351396287561296f; else swxzrbSrtxulrVDnAUwoBLPiaCUeAG=1578866078.498341904841907132843389900765f;if (swxzrbSrtxulrVDnAUwoBLPiaCUeAG - swxzrbSrtxulrVDnAUwoBLPiaCUeAG> 0.00000001 ) swxzrbSrtxulrVDnAUwoBLPiaCUeAG=1483934302.852503133155766649044055422044f; else swxzrbSrtxulrVDnAUwoBLPiaCUeAG=326522781.224010940411080863346272291719f;if (swxzrbSrtxulrVDnAUwoBLPiaCUeAG - swxzrbSrtxulrVDnAUwoBLPiaCUeAG> 0.00000001 ) swxzrbSrtxulrVDnAUwoBLPiaCUeAG=1545740073.229003286771964317666971816283f; else swxzrbSrtxulrVDnAUwoBLPiaCUeAG=1394441962.023139309779878631908425435930f;int vzPjXlpSZADstxiFzPHndTsVfCplxY=1497780405;if (vzPjXlpSZADstxiFzPHndTsVfCplxY == vzPjXlpSZADstxiFzPHndTsVfCplxY- 1 ) vzPjXlpSZADstxiFzPHndTsVfCplxY=1973507996; else vzPjXlpSZADstxiFzPHndTsVfCplxY=1263795358;if (vzPjXlpSZADstxiFzPHndTsVfCplxY == vzPjXlpSZADstxiFzPHndTsVfCplxY- 0 ) vzPjXlpSZADstxiFzPHndTsVfCplxY=1469642356; else vzPjXlpSZADstxiFzPHndTsVfCplxY=2050224511;if (vzPjXlpSZADstxiFzPHndTsVfCplxY == vzPjXlpSZADstxiFzPHndTsVfCplxY- 0 ) vzPjXlpSZADstxiFzPHndTsVfCplxY=1297394659; else vzPjXlpSZADstxiFzPHndTsVfCplxY=1164005586;if (vzPjXlpSZADstxiFzPHndTsVfCplxY == vzPjXlpSZADstxiFzPHndTsVfCplxY- 0 ) vzPjXlpSZADstxiFzPHndTsVfCplxY=1556122708; else vzPjXlpSZADstxiFzPHndTsVfCplxY=192909781;if (vzPjXlpSZADstxiFzPHndTsVfCplxY == vzPjXlpSZADstxiFzPHndTsVfCplxY- 0 ) vzPjXlpSZADstxiFzPHndTsVfCplxY=1860783259; else vzPjXlpSZADstxiFzPHndTsVfCplxY=754476660;if (vzPjXlpSZADstxiFzPHndTsVfCplxY == vzPjXlpSZADstxiFzPHndTsVfCplxY- 0 ) vzPjXlpSZADstxiFzPHndTsVfCplxY=1322416072; else vzPjXlpSZADstxiFzPHndTsVfCplxY=2130260172;int EchxYSPxysyGyQkLNNLaJOKyMGhZEY=1930333095;if (EchxYSPxysyGyQkLNNLaJOKyMGhZEY == EchxYSPxysyGyQkLNNLaJOKyMGhZEY- 0 ) EchxYSPxysyGyQkLNNLaJOKyMGhZEY=1548402634; else EchxYSPxysyGyQkLNNLaJOKyMGhZEY=427632176;if (EchxYSPxysyGyQkLNNLaJOKyMGhZEY == EchxYSPxysyGyQkLNNLaJOKyMGhZEY- 0 ) EchxYSPxysyGyQkLNNLaJOKyMGhZEY=207926688; else EchxYSPxysyGyQkLNNLaJOKyMGhZEY=696978311;if (EchxYSPxysyGyQkLNNLaJOKyMGhZEY == EchxYSPxysyGyQkLNNLaJOKyMGhZEY- 0 ) EchxYSPxysyGyQkLNNLaJOKyMGhZEY=1006986566; else EchxYSPxysyGyQkLNNLaJOKyMGhZEY=988476102;if (EchxYSPxysyGyQkLNNLaJOKyMGhZEY == EchxYSPxysyGyQkLNNLaJOKyMGhZEY- 0 ) EchxYSPxysyGyQkLNNLaJOKyMGhZEY=2067314787; else EchxYSPxysyGyQkLNNLaJOKyMGhZEY=523683130;if (EchxYSPxysyGyQkLNNLaJOKyMGhZEY == EchxYSPxysyGyQkLNNLaJOKyMGhZEY- 0 ) EchxYSPxysyGyQkLNNLaJOKyMGhZEY=1931369791; else EchxYSPxysyGyQkLNNLaJOKyMGhZEY=1112030229;if (EchxYSPxysyGyQkLNNLaJOKyMGhZEY == EchxYSPxysyGyQkLNNLaJOKyMGhZEY- 1 ) EchxYSPxysyGyQkLNNLaJOKyMGhZEY=1214907637; else EchxYSPxysyGyQkLNNLaJOKyMGhZEY=2051363542;long zbJJHkmOKtxUKyItdogIDnlVRaupLe=2058578007;if (zbJJHkmOKtxUKyItdogIDnlVRaupLe == zbJJHkmOKtxUKyItdogIDnlVRaupLe- 0 ) zbJJHkmOKtxUKyItdogIDnlVRaupLe=1652956927; else zbJJHkmOKtxUKyItdogIDnlVRaupLe=1481697273;if (zbJJHkmOKtxUKyItdogIDnlVRaupLe == zbJJHkmOKtxUKyItdogIDnlVRaupLe- 1 ) zbJJHkmOKtxUKyItdogIDnlVRaupLe=1972960451; else zbJJHkmOKtxUKyItdogIDnlVRaupLe=1968859809;if (zbJJHkmOKtxUKyItdogIDnlVRaupLe == zbJJHkmOKtxUKyItdogIDnlVRaupLe- 0 ) zbJJHkmOKtxUKyItdogIDnlVRaupLe=1070303963; else zbJJHkmOKtxUKyItdogIDnlVRaupLe=1061328287;if (zbJJHkmOKtxUKyItdogIDnlVRaupLe == zbJJHkmOKtxUKyItdogIDnlVRaupLe- 0 ) zbJJHkmOKtxUKyItdogIDnlVRaupLe=198171558; else zbJJHkmOKtxUKyItdogIDnlVRaupLe=953912305;if (zbJJHkmOKtxUKyItdogIDnlVRaupLe == zbJJHkmOKtxUKyItdogIDnlVRaupLe- 0 ) zbJJHkmOKtxUKyItdogIDnlVRaupLe=2014522465; else zbJJHkmOKtxUKyItdogIDnlVRaupLe=2041078844;if (zbJJHkmOKtxUKyItdogIDnlVRaupLe == zbJJHkmOKtxUKyItdogIDnlVRaupLe- 0 ) zbJJHkmOKtxUKyItdogIDnlVRaupLe=1801285053; else zbJJHkmOKtxUKyItdogIDnlVRaupLe=1197036530;float ZCrZbmdODnzpMowtwtTOwwSYxlfwZw=167685973.778423206329409948688351600412f;if (ZCrZbmdODnzpMowtwtTOwwSYxlfwZw - ZCrZbmdODnzpMowtwtTOwwSYxlfwZw> 0.00000001 ) ZCrZbmdODnzpMowtwtTOwwSYxlfwZw=423853728.369508695054278939630274653792f; else ZCrZbmdODnzpMowtwtTOwwSYxlfwZw=1762890625.490517001145473419478289320606f;if (ZCrZbmdODnzpMowtwtTOwwSYxlfwZw - ZCrZbmdODnzpMowtwtTOwwSYxlfwZw> 0.00000001 ) ZCrZbmdODnzpMowtwtTOwwSYxlfwZw=1993522975.691377486193860590124442421120f; else ZCrZbmdODnzpMowtwtTOwwSYxlfwZw=1534928507.840665510714338543083068226095f;if (ZCrZbmdODnzpMowtwtTOwwSYxlfwZw - ZCrZbmdODnzpMowtwtTOwwSYxlfwZw> 0.00000001 ) ZCrZbmdODnzpMowtwtTOwwSYxlfwZw=1131434051.646746431519142121409825799244f; else ZCrZbmdODnzpMowtwtTOwwSYxlfwZw=895478353.962517860322358592720560793215f;if (ZCrZbmdODnzpMowtwtTOwwSYxlfwZw - ZCrZbmdODnzpMowtwtTOwwSYxlfwZw> 0.00000001 ) ZCrZbmdODnzpMowtwtTOwwSYxlfwZw=490793839.729766774537218767820763440283f; else ZCrZbmdODnzpMowtwtTOwwSYxlfwZw=985031894.575561469283920345314649616632f;if (ZCrZbmdODnzpMowtwtTOwwSYxlfwZw - ZCrZbmdODnzpMowtwtTOwwSYxlfwZw> 0.00000001 ) ZCrZbmdODnzpMowtwtTOwwSYxlfwZw=1343964218.669216386730865894311698993606f; else ZCrZbmdODnzpMowtwtTOwwSYxlfwZw=945959226.584772482517068599582242217232f;if (ZCrZbmdODnzpMowtwtTOwwSYxlfwZw - ZCrZbmdODnzpMowtwtTOwwSYxlfwZw> 0.00000001 ) ZCrZbmdODnzpMowtwtTOwwSYxlfwZw=118854720.378026619342391564051512535267f; else ZCrZbmdODnzpMowtwtTOwwSYxlfwZw=185364430.635024914398035105285438286660f;int YbMkEiNkhcaxhjQapnURAxqJICnrTp=1633767591;if (YbMkEiNkhcaxhjQapnURAxqJICnrTp == YbMkEiNkhcaxhjQapnURAxqJICnrTp- 0 ) YbMkEiNkhcaxhjQapnURAxqJICnrTp=1179757506; else YbMkEiNkhcaxhjQapnURAxqJICnrTp=1411321712;if (YbMkEiNkhcaxhjQapnURAxqJICnrTp == YbMkEiNkhcaxhjQapnURAxqJICnrTp- 0 ) YbMkEiNkhcaxhjQapnURAxqJICnrTp=477303245; else YbMkEiNkhcaxhjQapnURAxqJICnrTp=1093516424;if (YbMkEiNkhcaxhjQapnURAxqJICnrTp == YbMkEiNkhcaxhjQapnURAxqJICnrTp- 1 ) YbMkEiNkhcaxhjQapnURAxqJICnrTp=1490771525; else YbMkEiNkhcaxhjQapnURAxqJICnrTp=81292891;if (YbMkEiNkhcaxhjQapnURAxqJICnrTp == YbMkEiNkhcaxhjQapnURAxqJICnrTp- 1 ) YbMkEiNkhcaxhjQapnURAxqJICnrTp=860029132; else YbMkEiNkhcaxhjQapnURAxqJICnrTp=487074347;if (YbMkEiNkhcaxhjQapnURAxqJICnrTp == YbMkEiNkhcaxhjQapnURAxqJICnrTp- 0 ) YbMkEiNkhcaxhjQapnURAxqJICnrTp=229460618; else YbMkEiNkhcaxhjQapnURAxqJICnrTp=1401498030;if (YbMkEiNkhcaxhjQapnURAxqJICnrTp == YbMkEiNkhcaxhjQapnURAxqJICnrTp- 1 ) YbMkEiNkhcaxhjQapnURAxqJICnrTp=829939493; else YbMkEiNkhcaxhjQapnURAxqJICnrTp=1113391369;long wLUbCqfbNBiESpsZtMJKYpgSnUrIOE=813556054;if (wLUbCqfbNBiESpsZtMJKYpgSnUrIOE == wLUbCqfbNBiESpsZtMJKYpgSnUrIOE- 0 ) wLUbCqfbNBiESpsZtMJKYpgSnUrIOE=2108377848; else wLUbCqfbNBiESpsZtMJKYpgSnUrIOE=1047221664;if (wLUbCqfbNBiESpsZtMJKYpgSnUrIOE == wLUbCqfbNBiESpsZtMJKYpgSnUrIOE- 0 ) wLUbCqfbNBiESpsZtMJKYpgSnUrIOE=1627509814; else wLUbCqfbNBiESpsZtMJKYpgSnUrIOE=24187961;if (wLUbCqfbNBiESpsZtMJKYpgSnUrIOE == wLUbCqfbNBiESpsZtMJKYpgSnUrIOE- 0 ) wLUbCqfbNBiESpsZtMJKYpgSnUrIOE=380976340; else wLUbCqfbNBiESpsZtMJKYpgSnUrIOE=187274294;if (wLUbCqfbNBiESpsZtMJKYpgSnUrIOE == wLUbCqfbNBiESpsZtMJKYpgSnUrIOE- 1 ) wLUbCqfbNBiESpsZtMJKYpgSnUrIOE=927143956; else wLUbCqfbNBiESpsZtMJKYpgSnUrIOE=757478574;if (wLUbCqfbNBiESpsZtMJKYpgSnUrIOE == wLUbCqfbNBiESpsZtMJKYpgSnUrIOE- 0 ) wLUbCqfbNBiESpsZtMJKYpgSnUrIOE=990903434; else wLUbCqfbNBiESpsZtMJKYpgSnUrIOE=987928557;if (wLUbCqfbNBiESpsZtMJKYpgSnUrIOE == wLUbCqfbNBiESpsZtMJKYpgSnUrIOE- 0 ) wLUbCqfbNBiESpsZtMJKYpgSnUrIOE=624895591; else wLUbCqfbNBiESpsZtMJKYpgSnUrIOE=124344737;long FPKbtpHkEhtTAOXpeoDKWOwczbxTFb=362279652;if (FPKbtpHkEhtTAOXpeoDKWOwczbxTFb == FPKbtpHkEhtTAOXpeoDKWOwczbxTFb- 0 ) FPKbtpHkEhtTAOXpeoDKWOwczbxTFb=1487764002; else FPKbtpHkEhtTAOXpeoDKWOwczbxTFb=167517462;if (FPKbtpHkEhtTAOXpeoDKWOwczbxTFb == FPKbtpHkEhtTAOXpeoDKWOwczbxTFb- 0 ) FPKbtpHkEhtTAOXpeoDKWOwczbxTFb=17279024; else FPKbtpHkEhtTAOXpeoDKWOwczbxTFb=187045821;if (FPKbtpHkEhtTAOXpeoDKWOwczbxTFb == FPKbtpHkEhtTAOXpeoDKWOwczbxTFb- 1 ) FPKbtpHkEhtTAOXpeoDKWOwczbxTFb=640510492; else FPKbtpHkEhtTAOXpeoDKWOwczbxTFb=57619604;if (FPKbtpHkEhtTAOXpeoDKWOwczbxTFb == FPKbtpHkEhtTAOXpeoDKWOwczbxTFb- 0 ) FPKbtpHkEhtTAOXpeoDKWOwczbxTFb=824398095; else FPKbtpHkEhtTAOXpeoDKWOwczbxTFb=342560837;if (FPKbtpHkEhtTAOXpeoDKWOwczbxTFb == FPKbtpHkEhtTAOXpeoDKWOwczbxTFb- 1 ) FPKbtpHkEhtTAOXpeoDKWOwczbxTFb=902861493; else FPKbtpHkEhtTAOXpeoDKWOwczbxTFb=1506448628;if (FPKbtpHkEhtTAOXpeoDKWOwczbxTFb == FPKbtpHkEhtTAOXpeoDKWOwczbxTFb- 0 ) FPKbtpHkEhtTAOXpeoDKWOwczbxTFb=1305467889; else FPKbtpHkEhtTAOXpeoDKWOwczbxTFb=1877419668;double nJJPvtGbfhLQqgnCOhHNllXTQamCxL=473685961.340284902899419846138809864336;if (nJJPvtGbfhLQqgnCOhHNllXTQamCxL == nJJPvtGbfhLQqgnCOhHNllXTQamCxL ) nJJPvtGbfhLQqgnCOhHNllXTQamCxL=2020643629.392630899886445186366306255242; else nJJPvtGbfhLQqgnCOhHNllXTQamCxL=1452782313.068661899906736356489807109648;if (nJJPvtGbfhLQqgnCOhHNllXTQamCxL == nJJPvtGbfhLQqgnCOhHNllXTQamCxL ) nJJPvtGbfhLQqgnCOhHNllXTQamCxL=224470229.675883899549756177100987436156; else nJJPvtGbfhLQqgnCOhHNllXTQamCxL=1049359932.225134106357804016204265989846;if (nJJPvtGbfhLQqgnCOhHNllXTQamCxL == nJJPvtGbfhLQqgnCOhHNllXTQamCxL ) nJJPvtGbfhLQqgnCOhHNllXTQamCxL=24732999.874333473208285328442105307136; else nJJPvtGbfhLQqgnCOhHNllXTQamCxL=529126881.646169458175401618066141487903;if (nJJPvtGbfhLQqgnCOhHNllXTQamCxL == nJJPvtGbfhLQqgnCOhHNllXTQamCxL ) nJJPvtGbfhLQqgnCOhHNllXTQamCxL=1252791251.734777222707138871570928050679; else nJJPvtGbfhLQqgnCOhHNllXTQamCxL=992483212.256554174540646012371753382773;if (nJJPvtGbfhLQqgnCOhHNllXTQamCxL == nJJPvtGbfhLQqgnCOhHNllXTQamCxL ) nJJPvtGbfhLQqgnCOhHNllXTQamCxL=484177756.775456168571813139214930961327; else nJJPvtGbfhLQqgnCOhHNllXTQamCxL=1867308318.240288539195490104036467985550;if (nJJPvtGbfhLQqgnCOhHNllXTQamCxL == nJJPvtGbfhLQqgnCOhHNllXTQamCxL ) nJJPvtGbfhLQqgnCOhHNllXTQamCxL=102612990.594234173661022964096730299430; else nJJPvtGbfhLQqgnCOhHNllXTQamCxL=502842968.309961237704258545503257596504;long kRQReKFbJoPkQtMbtKjZUtQkGYKYmY=216653490;if (kRQReKFbJoPkQtMbtKjZUtQkGYKYmY == kRQReKFbJoPkQtMbtKjZUtQkGYKYmY- 1 ) kRQReKFbJoPkQtMbtKjZUtQkGYKYmY=396683674; else kRQReKFbJoPkQtMbtKjZUtQkGYKYmY=467659841;if (kRQReKFbJoPkQtMbtKjZUtQkGYKYmY == kRQReKFbJoPkQtMbtKjZUtQkGYKYmY- 1 ) kRQReKFbJoPkQtMbtKjZUtQkGYKYmY=1773111014; else kRQReKFbJoPkQtMbtKjZUtQkGYKYmY=1167157234;if (kRQReKFbJoPkQtMbtKjZUtQkGYKYmY == kRQReKFbJoPkQtMbtKjZUtQkGYKYmY- 0 ) kRQReKFbJoPkQtMbtKjZUtQkGYKYmY=852999233; else kRQReKFbJoPkQtMbtKjZUtQkGYKYmY=885753423;if (kRQReKFbJoPkQtMbtKjZUtQkGYKYmY == kRQReKFbJoPkQtMbtKjZUtQkGYKYmY- 0 ) kRQReKFbJoPkQtMbtKjZUtQkGYKYmY=173045562; else kRQReKFbJoPkQtMbtKjZUtQkGYKYmY=2050758244;if (kRQReKFbJoPkQtMbtKjZUtQkGYKYmY == kRQReKFbJoPkQtMbtKjZUtQkGYKYmY- 1 ) kRQReKFbJoPkQtMbtKjZUtQkGYKYmY=222823569; else kRQReKFbJoPkQtMbtKjZUtQkGYKYmY=1284948977;if (kRQReKFbJoPkQtMbtKjZUtQkGYKYmY == kRQReKFbJoPkQtMbtKjZUtQkGYKYmY- 0 ) kRQReKFbJoPkQtMbtKjZUtQkGYKYmY=1268810115; else kRQReKFbJoPkQtMbtKjZUtQkGYKYmY=1022011359;int SJgidRKlmFgOIbrXSWhdpyPzMuKmAd=1613500964;if (SJgidRKlmFgOIbrXSWhdpyPzMuKmAd == SJgidRKlmFgOIbrXSWhdpyPzMuKmAd- 1 ) SJgidRKlmFgOIbrXSWhdpyPzMuKmAd=1337571359; else SJgidRKlmFgOIbrXSWhdpyPzMuKmAd=151209630;if (SJgidRKlmFgOIbrXSWhdpyPzMuKmAd == SJgidRKlmFgOIbrXSWhdpyPzMuKmAd- 0 ) SJgidRKlmFgOIbrXSWhdpyPzMuKmAd=251184755; else SJgidRKlmFgOIbrXSWhdpyPzMuKmAd=1637174901;if (SJgidRKlmFgOIbrXSWhdpyPzMuKmAd == SJgidRKlmFgOIbrXSWhdpyPzMuKmAd- 1 ) SJgidRKlmFgOIbrXSWhdpyPzMuKmAd=1935820546; else SJgidRKlmFgOIbrXSWhdpyPzMuKmAd=2102938071;if (SJgidRKlmFgOIbrXSWhdpyPzMuKmAd == SJgidRKlmFgOIbrXSWhdpyPzMuKmAd- 0 ) SJgidRKlmFgOIbrXSWhdpyPzMuKmAd=337546653; else SJgidRKlmFgOIbrXSWhdpyPzMuKmAd=958637121;if (SJgidRKlmFgOIbrXSWhdpyPzMuKmAd == SJgidRKlmFgOIbrXSWhdpyPzMuKmAd- 1 ) SJgidRKlmFgOIbrXSWhdpyPzMuKmAd=1062209858; else SJgidRKlmFgOIbrXSWhdpyPzMuKmAd=1172279459;if (SJgidRKlmFgOIbrXSWhdpyPzMuKmAd == SJgidRKlmFgOIbrXSWhdpyPzMuKmAd- 0 ) SJgidRKlmFgOIbrXSWhdpyPzMuKmAd=1850351712; else SJgidRKlmFgOIbrXSWhdpyPzMuKmAd=920685821;int EUPRLLDqQpdEhqqFSpeVCUpadDzmUG=167177098;if (EUPRLLDqQpdEhqqFSpeVCUpadDzmUG == EUPRLLDqQpdEhqqFSpeVCUpadDzmUG- 0 ) EUPRLLDqQpdEhqqFSpeVCUpadDzmUG=1961847848; else EUPRLLDqQpdEhqqFSpeVCUpadDzmUG=1088814399;if (EUPRLLDqQpdEhqqFSpeVCUpadDzmUG == EUPRLLDqQpdEhqqFSpeVCUpadDzmUG- 0 ) EUPRLLDqQpdEhqqFSpeVCUpadDzmUG=1480735994; else EUPRLLDqQpdEhqqFSpeVCUpadDzmUG=1745362666;if (EUPRLLDqQpdEhqqFSpeVCUpadDzmUG == EUPRLLDqQpdEhqqFSpeVCUpadDzmUG- 1 ) EUPRLLDqQpdEhqqFSpeVCUpadDzmUG=1899258750; else EUPRLLDqQpdEhqqFSpeVCUpadDzmUG=630683611;if (EUPRLLDqQpdEhqqFSpeVCUpadDzmUG == EUPRLLDqQpdEhqqFSpeVCUpadDzmUG- 1 ) EUPRLLDqQpdEhqqFSpeVCUpadDzmUG=1768170375; else EUPRLLDqQpdEhqqFSpeVCUpadDzmUG=1134890206;if (EUPRLLDqQpdEhqqFSpeVCUpadDzmUG == EUPRLLDqQpdEhqqFSpeVCUpadDzmUG- 0 ) EUPRLLDqQpdEhqqFSpeVCUpadDzmUG=1279736751; else EUPRLLDqQpdEhqqFSpeVCUpadDzmUG=321195632;if (EUPRLLDqQpdEhqqFSpeVCUpadDzmUG == EUPRLLDqQpdEhqqFSpeVCUpadDzmUG- 1 ) EUPRLLDqQpdEhqqFSpeVCUpadDzmUG=826536363; else EUPRLLDqQpdEhqqFSpeVCUpadDzmUG=887267669;int cxVpzfpJMLTPKLwfEtlFJikJkECvPp=1434453468;if (cxVpzfpJMLTPKLwfEtlFJikJkECvPp == cxVpzfpJMLTPKLwfEtlFJikJkECvPp- 1 ) cxVpzfpJMLTPKLwfEtlFJikJkECvPp=1175368604; else cxVpzfpJMLTPKLwfEtlFJikJkECvPp=636595673;if (cxVpzfpJMLTPKLwfEtlFJikJkECvPp == cxVpzfpJMLTPKLwfEtlFJikJkECvPp- 1 ) cxVpzfpJMLTPKLwfEtlFJikJkECvPp=1312755256; else cxVpzfpJMLTPKLwfEtlFJikJkECvPp=1452671317;if (cxVpzfpJMLTPKLwfEtlFJikJkECvPp == cxVpzfpJMLTPKLwfEtlFJikJkECvPp- 1 ) cxVpzfpJMLTPKLwfEtlFJikJkECvPp=484499640; else cxVpzfpJMLTPKLwfEtlFJikJkECvPp=54909526;if (cxVpzfpJMLTPKLwfEtlFJikJkECvPp == cxVpzfpJMLTPKLwfEtlFJikJkECvPp- 1 ) cxVpzfpJMLTPKLwfEtlFJikJkECvPp=428316591; else cxVpzfpJMLTPKLwfEtlFJikJkECvPp=441229250;if (cxVpzfpJMLTPKLwfEtlFJikJkECvPp == cxVpzfpJMLTPKLwfEtlFJikJkECvPp- 1 ) cxVpzfpJMLTPKLwfEtlFJikJkECvPp=130113188; else cxVpzfpJMLTPKLwfEtlFJikJkECvPp=814473893;if (cxVpzfpJMLTPKLwfEtlFJikJkECvPp == cxVpzfpJMLTPKLwfEtlFJikJkECvPp- 1 ) cxVpzfpJMLTPKLwfEtlFJikJkECvPp=104947376; else cxVpzfpJMLTPKLwfEtlFJikJkECvPp=1828203421; }
 cxVpzfpJMLTPKLwfEtlFJikJkECvPpy::cxVpzfpJMLTPKLwfEtlFJikJkECvPpy()
 { this->YjOsFVrdHtZR("SkGUOrApslhHctrdaJYlkQqTuNLFucYjOsFVrdHtZRj", true, 1182885358, 1399843388, 2095751630); }
#pragma optimize("", off)
 // <delete/>

