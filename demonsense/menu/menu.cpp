#include "menu.h"
#define NOMINMAX
#include <Windows.h>
#include <chrono>

#include "../valve_sdk/csgostructs.h"
#include "../helpers/input.h"
#include "../render/render.h"

#include "../features/misc/misc.h"
#include "../options/options.h"
#include "../options/config.h"
#include "ui.h"
#include "../security.h"
#include "../hooks/hooks.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "../imgui/imgui_internal.h"
#include "../imgui/imgui_impl_dx9.h"
#include "../imgui/imgui_impl_win32.h"


#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

#pragma region RenderTabs
#define UNLEN 256


static const char* hitmarkersounds[] = {
	"Cod",
	"Skeet",
	"Punch",
	"Metal",
	"Boom",
	"Soneca issues"
};


//-------------------------------------------------RENDERS ETC--------------------------------------------------------------------------//

template<size_t N>
void render_tabs(char* (&names)[N], int& activetab, float w, float h)
{
	bool values[N] = { false };
	values[activetab] = true;
	for (auto i = 0; i < N; ++i)
	{
		if (i == 0)
		{
			if (ImGui::ToggleButton(names[i], &values[i], ImVec2{ w, h }, 1))
				activetab = i;
		}
		else if (i == N - 1)
		{
			if (ImGui::ToggleButton(names[i], &values[i], ImVec2{ w, h }, 2))
				activetab = i;
		}
		else
		{
			if (ImGui::ToggleButton(names[i], &values[i], ImVec2{ w, h }, 0))
				activetab = i;
		}
		if (i < N - 1) ImGui::SameLine();
	}
}
template<size_t N>
void render_tabsMain(char* (&names)[N], int& activetab, float w, float h)
{
	bool values[N] = { false };
	values[activetab] = true;
	for (auto i = 0; i < N; ++i)
	{
		if (i == 0)
		{
			if (ImGui::ToggleButtonMain(names[i], &values[i], ImVec2{ w, h }, 1))
				activetab = i;
		}
		else if (i == N - 1)
		{
			if (ImGui::ToggleButtonMain(names[i], &values[i], ImVec2{ w, h }, 2))
				activetab = i;
		}
		else
		{
			if (ImGui::ToggleButtonMain(names[i], &values[i], ImVec2{ w, h }, 0))
				activetab = i;
		}
		if (i < N - 1) ImGui::SameLine();
	}
}

static int weapon_vector_index = 0;
struct WeaponName_t {
	constexpr WeaponName_t(int32_t definition_index, const char* name) :
		definition_index(definition_index),
		name(name) {
	}

	int32_t definition_index = 0;
	const char* name = nullptr;
};

std::vector< WeaponName_t> WeaponNames =
{
{ 31, "zeus" },

{ 61, "usp-S" },
{ 4, "glock-18" },
{ 32, "p2000" },
{ 2,  "dual berettas" },
{ 36, "p250" },
{ 63, "cz75 auto" },
{ 3,  "five-seven" },
{ 30, "tec-9" },
{ 64, "r8 revolver" },
{ 1,  "desert eagle" },

{ 34, "mp9" },
{ 17, "mac-10" },
{ 33, "mp7" },
{ 23, "mp5-sd" },
{ 24, "ump-45" },
{ 19, "p90" },
{ 26, "pp-bizon" },

{ 13, "galil ar" },
{ 10, "famas" },
{ 7,  "ak-47" },
{ 60, "m4a1-s" },
{ 16, "m4a4" },
{ 8, "aug" },
{ 40, "ssg 08" },
{ 39, "sg 553" },
{ 9, "awp" },
{ 38, "scar-20" },
{ 11, "g3sg1" },

{ 35, "nova" },
{ 25, "xm1014" },
{ 29, "sawed-off" },
{ 27, "mag-7" },
{ 14, "m249" },
{ 28, "negev" },
};

static int settings_weap;

void current_weapon_checkbox()
{
	ImGui::Checkbox("auto current", &settings::legit_bot::auto_current);

	if (!g_engine_client->is_connected() || !g_local_player || !g_local_player->is_alive())
	{
		return;
	}

	auto weapon = g_local_player->m_hActiveWeapon();

	if (!weapon || !weapon->is_weapon())
	{
		return;
	}


	if (settings::legit_bot::auto_current)
	{
		int wpn_idx = weapon->m_Item().m_iItemDefinitionIndex();
		auto wpn_it = std::find_if(WeaponNames.begin(), WeaponNames.end(), [wpn_idx](const WeaponName_t& a)
			{
				return a.definition_index == wpn_idx;
			});

		if (wpn_it != WeaponNames.end())
		{
			settings_weap = wpn_idx;
			settings::legit_bot::settings_weapon = std::abs(std::distance(WeaponNames.begin(), wpn_it));
		}
	}
}

static char* weapon_groups_array[] =
{
"zeus",
"pistols",
"semi-rifle",
"shot gun",
"machin gun",
"rifle",
"sniper rifle",
"knife"
};

static char* legit_tabs[] =
{
	"all",
	"groups",
	"on weapon"
};


//-------------------------------------------------AIMBOT TAB-----------------------------------------------------------------------------//


void legit_tab()
{
	legitbot_settings* settings;

	static char* priorities[] = {
			"fov",
			"health",
			"damage",
			"distance"
	};

	static char* aim_types[] = {
		"hitbox",
		"nearest"
	};

	static char* smooth_types[] = {
		"static",
		"dynamic"
	};


	const char* hitbox_list[7] = {
		"head",
		"neck",
		"pelvis",
		"stomach",
		"lower chest",
		"chest",
		"upper chest",
	};

	ImGui::Columns(2, NULL, false);
	ImGui::BeginChild("weapon select", ImVec2(0, 132), true, ImGuiWindowFlags_ChildWindowTitle);
	{
		auto TabsW_players_tab = (ImGui::GetCurrentWindow()->Size.x - menu::_style.WindowPadding.x * 2.0f) / _countof(legit_tabs);

		ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing_new, ImVec2(0, 0));
		{
			render_tabs(legit_tabs, settings::legit_bot::settings_category, TabsW_players_tab, 20.0f);
		}
		ImGui::PopStyleVar();

		if (settings::legit_bot::settings_category == 0)
		{
			ImGui::Dummy(ImVec2(0.f, 51.f));
			settings = &settings::legit_bot::legitbot_items_all;
		}
		else if (settings::legit_bot::settings_category == 1)
		{
			ImGui::Combo("group##weapons_groups", &settings::legit_bot::weapon_group, weapon_groups_array, IM_ARRAYSIZE(weapon_groups_array));
			settings = &settings::legit_bot::legitbot_items_groups[settings::legit_bot::weapon_group];

			ImGui::Dummy(ImVec2(0.f, 16.f));
		}
		else if (settings::legit_bot::settings_category == 2)
		{
			if (!settings::legit_bot::auto_current)
			{
				if (ImGui::Combo(
					"weapon", &settings::legit_bot::settings_weapon,
					[](void* data, int32_t idx, const char** out_text)
					{
						auto vec = reinterpret_cast<std::vector< WeaponName_t >*>(data);
						*out_text = vec->at(idx).name;
						return true;
					}, (void*)(&WeaponNames), WeaponNames.size()))
				{
					settings_weap = WeaponNames[settings::legit_bot::settings_weapon].definition_index;
				}
			}
			else
			{
				ImGui::TextS("settings weapon : %s", WeaponNames[settings::legit_bot::settings_weapon].name);
			}

			settings = &settings::legit_bot::legitbot_items[settings_weap];

			current_weapon_checkbox();
		}

		ImGui::Checkbox("bind##legit_aimbot", &settings::legit_bot::bind_check);
        ImGui::Tooltip("Right click on the bind to customize");
		ImGui::SameLine();
		ImGui::KeyBind("##enable_legit_aimbot", &settings::legit_bot::bind, ImGuiKeyBindFlags_OnItem);
	}
	ImGui::EndChild("weapon select");

	ImGui::BeginChild("general##legit", ImVec2(0, 0), true, ImGuiWindowFlags_ChildWindowTitle);
	{
		ImGui::Checkbox("enabled", &settings->enabled);

		ImGui::Checkbox("friendly fire", &settings->deathmatch);

		if (settings::legit_bot::settings_category == 0)
			ImGui::Checkbox("auto pistol", &settings->autopistol);

		else if (settings::legit_bot::settings_category == 1)
		{
			if (settings::legit_bot::weapon_group == 1)
			{
				ImGui::Checkbox("auto pistol", &settings->autopistol);
			}
		}

		else if (settings::legit_bot::settings_category == 2)
		{
			if (settings_weap == WEAPON_P250 ||
				settings_weap == WEAPON_USP_SILENCER ||
				settings_weap == WEAPON_GLOCK ||
				settings_weap == WEAPON_FIVESEVEN ||
				settings_weap == WEAPON_TEC9 ||
				settings_weap == WEAPON_DEAGLE ||
				settings_weap == WEAPON_ELITE ||
				settings_weap == WEAPON_HKP2000) {
				ImGui::Checkbox("auto pistol", &settings->autopistol);
			}
		}

		const char* check_c[4] = { "smoke", "flash", "jump", "zoom" };
		bool* check_b[4] = { &settings->smoke_check, &settings->flash_check, &settings->jump_check, &settings->only_in_zoom };
		ImGui::MultiCombo("check", check_c, check_b, 4);

		if (settings->flash_check)
			ImGui::SliderInt("min alpha##flash_check", &settings->flash_check_min, 0, 255);
		ImGui::Checkbox("autowall", &settings->autowall);
		ImGui::Checkbox("silent", &settings->silent);
		ImGui::Checkbox("auto stop", &settings->autostop);
	/*	ImGui::Checkbox("auto fire", &settings->autofire);
		ImGui::Tooltip("Right-click to access customization");
		if (ImGui::ItemsToolTipBegin("##backtracktooltip"))
		{
			ImGui::SliderInt("autofire hitchance", &settings->hitchance_wp, 0, 100, "%.1f");
			ImGui::ItemsToolTipEnd();
		}
		ImGui::SameLine();
		ImGui::Hotkey("", &settings->autofire_key);*/
				
		ImGui::Checkbox("enable override hitbox##check", &settings->enable_hitbox_override);
        ImGui::Tooltip("Right click on the bind to customize");
		ImGui::SameLine();
		ImGui::KeyBind("##bind_override_hitbox", &settings->hitbox_override_bind);
		ImGui::MultiCombo("override hitboxes", hitbox_list, settings->override_hitboxes, 7);
	}
	ImGui::EndChild("general##legit");
	ImGui::NextColumn();
	ImGui::BeginChild("weapon settings", ImVec2(0, 300), true, ImGuiWindowFlags_ChildWindowTitle);
	{
		ImGui::MultiCombo("hitboxes", hitbox_list, settings->hitboxes, 7);

		ImGui::Combo("smooth type", &settings->smooth_type, smooth_types, IM_ARRAYSIZE(smooth_types));
		ImGui::SliderFloat("basic fov", &settings->fov, 0, 30, "%.2f");
		ImGui::SliderFloat("smooth", &settings->smooth, 1, 20, "%.2f");
		ImGui::SliderInt("kill delay", &settings->kill_delay, 0, 1000, "%d sec");
		if (!settings->silent) {
			ImGui::SliderInt("shot delay", &settings->shot_delay, 0, 100);
		}

		if (settings->silent)
		{
			ImGui::SliderFloat("silent fov", &settings->silent_fov, 0, 20, "%.2f");
			ImGui::SliderInt("silent chance", &settings->silent_chance, 0, 100, "%i %%");
		}



		if (settings->autowall) {
			ImGui::SliderInt("min damage##aimbot.min_damage", &settings->min_damage, 1, 100);
		}

		ImGui::SliderInt("backtrack tick [?]", &settings->backtrack_time, 0, 12, "%i ticks");
		ImGui::Tooltip("Right-click to access customization");
		if (ImGui::ItemsToolTipBegin("##backtracktooltip"))
		{
			ImGui::Checkbox("aim at backtrack", &settings->aim_at_backtrack);
			ImGui::ItemsToolTipEnd();
		}

	}
	ImGui::EndChild("weapon settings");
	ImGui::BeginChild("triggerbot", ImVec2(0, 162), true, ImGuiWindowFlags_ChildWindowTitle);
	{
		ImGui::Checkbox("enable##444", &settings::triggerbot::enable);
		ImGui::SameLine();
		ImGui::Hotkey("##OnKey##bindtriggerkey", &settings::triggerbot::bind);
		ImGui::SliderFloat("delay##Delaytrigger", &settings::triggerbot::delay, 0.0f, 100.0f, "%.3f");
		ImGui::SliderFloat("hitchance##Hitchancetrigger", &settings::triggerbot::hitchance, 0.0f, 100.0f, "%.3f");
		ImGui::Checkbox("magnet##444", &settings::triggerbot::magnetic);
		ImGui::Checkbox("Automatic trigger##444", &settings::triggerbot::autotrigger);
		ImGui::Tooltip("Automatically fires when entering the triggerbot field!");

	}
	ImGui::EndChild("triggerbot");


	ImGui::BeginChild("rcs setting", ImVec2(0, 300), true, ImGuiWindowFlags_ChildWindowTitle);
	{

		ImGui::Checkbox("enable", &settings->rcs);
		static char* rcs_types[] = {
			"standalone",
			"aim"
		};
		ImGui::Combo("rcs type", &settings->rcs_type, rcs_types, IM_ARRAYSIZE(rcs_types));
		ImGui::Checkbox("custom Fov", &settings->rcs_fov_enabled);

		if (settings->rcs_fov_enabled)
		{
			ImGui::SliderFloat("rcs fov", &settings->rcs_fov, 0, 20, "%.2f *");
		}

		ImGui::Checkbox("custom smooth", &settings->rcs_smooth_enabled);

		if (settings->rcs_smooth_enabled)
		{
			ImGui::SliderFloat("rcs smooth", &settings->rcs_smooth, 1, 15);
		}

		ImGui::SliderInt("rcs x", &settings->rcs_x, 0, 100, "%i %%");
		ImGui::SliderInt("rcs y", &settings->rcs_y, 0, 100, "%i %%");
		ImGui::SliderInt("rcs start bullet", &settings->rcs_start, 1, 30);
	}
	ImGui::EndChild("rcs setting");


}

static int general_tab = 0;


//-------------------------------------------------VISUALS TAB--------------------------------------------------------------------------//
void visuals_tab()
{

	static int glow_player_tab = 0;
	static int chams_player_tab = 0;
	static int chams_player_tab_ragdoll = 0;
	static int esp_player_tab = 0;

	if (general_tab == 0)
	{
		auto settings_cur = &settings::visuals::esp::esp_items[esp_player_tab];

		ImGui::Columns(2, nullptr, false);
		ImGui::BeginChild("general##child", ImVec2(0, 81), true, ImGuiWindowFlags_ChildWindowTitle);
		{
			auto TabsW_players_tab = (ImGui::GetCurrentWindow()->Size.x - menu::_style.WindowPadding.x * 2.0f) / _countof(players_tabs);


			ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing_new, ImVec2(0, 0));
			{
				render_tabs(players_tabs, esp_player_tab, TabsW_players_tab, 20.0f);
			}
			ImGui::PopStyleVar();


			ImGui::Dummy(ImVec2(0.f, 0.f));

			ImGui::Checkbox("enable", &settings_cur->enable);
		}
		ImGui::EndChild("general##child");
		ImGui::BeginChild("player", ImVec2(0, 0), true, ImGuiWindowFlags_ChildWindowTitle);
        {
            ImGui::Checkbox("visible only", &settings_cur->only_visible);
            ImGui::Checkbox("skeleton", &settings_cur->skeleton);
			ImGui::Checkbox("box [?]", &settings_cur->box);
			ImGui::Tooltip("Right-click to access customization");
			if (ImGui::ItemsToolTipBegin("##boxrandom123"))
			{
				ImGui::Combo("box type", &settings_cur->box_position, std::vector<std::string>{ "normal", "corner"});
				ImGui::ItemsToolTipEnd();
			}
			ImGui::Checkbox("name", &settings_cur->name);
			ImGui::Checkbox("health bar [?]", &settings_cur->health_bar);
			if (ImGui::ItemsToolTipBegin("##health_bartooltip"))
			{
				ImGui::Checkbox("health in bar", &settings_cur->health_in_bar);
				ImGui::Checkbox("color healthbased", &settings_cur->health_based);
				ImGui::Combo("health bar position", &settings_cur->health_position, std::vector<std::string>{ "left", "right", "top", "bottom" });

				ImGui::ItemsToolTipEnd();
			}

			ImGui::Checkbox("armour bar [?]", &settings_cur->armor_bar);
            ImGui::Tooltip("Right-click to access customization");
			if (ImGui::ItemsToolTipBegin("##armour_bartooltip"))
			{
				ImGui::Checkbox("armour in bar", &settings_cur->armor_in_bar);
				ImGui::Combo("armor bar position", &settings_cur->armor_position, std::vector<std::string>{ "left", "right", "top", "bottom" });

				ImGui::ItemsToolTipEnd();
			}

			ImGui::Checkbox("weapon [?]", &settings_cur->weapon);
            ImGui::Tooltip("Right-click to access customization");
			if (ImGui::ItemsToolTipBegin("##weapontooltip"))
			{
				ImGui::Combo("Weapon type", &settings_cur->weapon_type, std::vector<std::string>{ "type 1", "type 2",});
				ImGui::Checkbox("icon + text", &settings_cur->weaico);
				ImGui::ItemsToolTipEnd();
			}

			ImGui::Checkbox("backtrack dots", &settings_cur->esp_backtrack);

			ImGui::Checkbox("weapon ammo bar", &settings_cur->weapon_ammo);

			const char* flags_c[] = { "scoped", "flashed", "defusing", /*"planting",*/ "bomb", "armor", "helmet" };
			bool* flags_b[] = { &settings_cur->flags_scoped, &settings_cur->flags_flashed, &settings_cur->flags_defusing, /*&settings_cur->flags_planting,*/ &settings_cur->flags_bomb_carrier, &settings_cur->flags_armor, &settings_cur->flags_helmet };
			ImGui::MultiCombo("flags", flags_c, flags_b, 6);

		}
		ImGui::EndChild("player");
		ImGui::NextColumn();
		ImGui::BeginChild("colors##players_visial", ImVec2(0, 0), true, ImGuiWindowFlags_ChildWindowTitle);
		{
			ImGui::ColorEdit3("name", &settings_cur->name_color);
			ImGui::ColorEdit3("dormant", &settings_cur->dormant);
			ImGui::ColorEdit3("visible", &settings_cur->box_visible);
			ImGui::ColorEdit3("invisible", &settings_cur->box_invisible);
			ImGui::ColorEdit3("weapon name", &settings_cur->weapon_name);
			ImGui::ColorEdit3("weapon icon", &settings_cur->weapon_name_icon);
			ImGui::ColorEdit3("skeleton visible", &settings_cur->skeleton_visible);
			ImGui::ColorEdit3("skeleton invisible", &settings_cur->skeleton_invisible);
			ImGui::ColorEdit3("health bar outline", &settings_cur->health_bar_outline);
			ImGui::ColorEdit3("health bar background", &settings_cur->health_bar_background);
			ImGui::ColorEdit3("health bar main", &settings_cur->health_bar_main);
			ImGui::ColorEdit3("armor bar outline", &settings_cur->armor_bar_outline);
			ImGui::ColorEdit3("armor bar background", &settings_cur->armor_bar_background);
			ImGui::ColorEdit3("armor bar main", &settings_cur->armor_bar_main);
			ImGui::ColorEdit3("ammo bar outline", &settings_cur->ammo_bar_outline);
			ImGui::ColorEdit3("ammo bar background", &settings_cur->ammo_bar_background);
			ImGui::ColorEdit3("ammo bar main", &settings_cur->ammo_bar_main);

		}
		ImGui::EndChild("colors##players_visial");
	}
	if (general_tab == 1)
	{
		ImGui::Columns(2, nullptr, false);

		ImGui::BeginChild("general##other_tab", ImVec2(0, 0), true, ImGuiWindowFlags_ChildWindowTitle);
		{
			ImGui::Checkbox("offscreen [?]", &settings::visuals::ofc::enable);
            ImGui::Tooltip("Right-click to access customization");
			if (ImGui::ItemsToolTipBegin("##offscreenother_tab"))
			{
				ImGui::Checkbox("visible check", &settings::visuals::ofc::visible_check);

				ImGui::SliderFloat("range##offscreen", &settings::visuals::ofc::range, 0.f, 100.f, "%.1f");
				ImGui::SliderFloat("wight##offscreen", &settings::visuals::ofc::wight, 0.f, 30.f, "%.1f");
				ImGui::SliderFloat("hight##offscreen", &settings::visuals::ofc::hight, 0.f, 30.f, "%.1f");

				ImGui::ItemsToolTipEnd();
			}
			ImGui::SameLine();
			ImGui::ColorEdit4("##color_offscreen", &settings::visuals::ofc::color);

			ImGui::Checkbox("dropped weapons [?]", &settings::visuals::dropped_weapon::enable);
            ImGui::Tooltip("Right-click to access customization");
			if (ImGui::ItemsToolTipBegin("##dropped_weapons_other_tab"))
			{
				auto settings_cur = &settings::visuals::esp::esp_items[esp_player_tab];

				
				ImGui::Combo("Weapon type", &settings::visuals::dropped_weapon::enable_type, std::vector<std::string>{ "Name", "Icon" });
				ImGui::Checkbox("box##dropped_weapons", &settings::visuals::dropped_weapon::box);
				ImGui::SameLine();
				ImGui::ColorEdit4("##colorbox_dropped_weapons", &settings::visuals::dropped_weapon::box_color);
				ImGui::Checkbox("icon + text ", &settings::visuals::dropped_weapon::weacoi);
				ImGui::ColorEdit3("text color##dropped_weapons", &settings::visuals::dropped_weapon::droppednamecolor);
				ImGui::ColorEdit3("icon color##dropped_weapons", &settings::visuals::dropped_weapon::droppediconcolor);



				ImGui::Checkbox("ammo bar##dropped_weapons", &settings::visuals::dropped_weapon::ammo_bar);

				ImGui::ColorEdit3("outline##dropped_weapons", &settings::visuals::dropped_weapon::bar_outline);
				ImGui::ColorEdit3("background##dropped_weapons", &settings::visuals::dropped_weapon::bar_background);
				ImGui::ColorEdit3("main##dropped_weapons", &settings::visuals::dropped_weapon::bar_main);


				ImGui::ItemsToolTipEnd();
			}
			ImGui::Checkbox("damage indicator [?]", &settings::visuals::damage_indicator::enable);
            ImGui::Tooltip("Right-click to access customization");
			if (ImGui::ItemsToolTipBegin("##damage_indicator_tooltip"))
			{
				ImGui::SliderFloat("show time##damage_indicator_tooltip", &settings::visuals::damage_indicator::show_time, 2.f, 10.f, "%.1f");
				ImGui::SliderFloat("speed##damage_indicator_tooltip", &settings::visuals::damage_indicator::speed, 0.f, 1.f, "%.2f");

				ImGui::SliderFloat("max position y##damage_indicator_tooltip", &settings::visuals::damage_indicator::max_pos_y, 0.f, 200.f, "%.1f");

				ImGui::SliderFloat("offset at kill##damage_indicator_tooltip", &settings::visuals::damage_indicator::offset_if_kill, 0.f, 20.f, "%.1f");
				ImGui::SliderFloat("offset at hit##damage_indicator_tooltip", &settings::visuals::damage_indicator::offset_hit, 0.f, 10.f, "%.1f");

				ImGui::SliderFloat("range offset x##damage_indicator_tooltip", &settings::visuals::damage_indicator::range_offset_x, 0.f, 30.f, "%.1f");

				ImGui::SliderFloat("text size##damage_indicator_tooltip", &settings::visuals::damage_indicator::text_size, 2.f, 20.f, "%.1f");
				ImGui::ColorEdit3("hit color##damage_indicator_tooltip", &settings::visuals::damage_indicator::standart_color);
				ImGui::ColorEdit3("kill color##damage_indicator_tooltip", &settings::visuals::damage_indicator::kill_color);

				ImGui::ItemsToolTipEnd();
			}

		}
		ImGui::EndChild("general##other_tab");

		ImGui::NextColumn();

		ImGui::BeginChild("other##other_tab", ImVec2(0, 0), true, ImGuiWindowFlags_ChildWindowTitle);
		{
			ImGui::Checkbox("capsule hitbox", &settings::visuals::hitbox::enable);
			ImGui::SameLine();
			ImGui::ColorEdit3("##hitbox_color", &settings::visuals::hitbox::color);
			ImGui::Combo("draw type##capsule_hitbox", &settings::visuals::hitbox::show_type, std::vector<std::string>{ "on hit", "on death" });
			ImGui::SliderFloat("show time##capsule_hitbox", &settings::visuals::hitbox::show_time, 0.f, 10.f, "%.1f");
		}
		ImGui::EndChild("other##other_tab");
	}
	if (general_tab == 2)
	{
		ImGui::Columns(2, nullptr, false);
		ImGui::BeginChild("general##visuals_general1", ImVec2(0, 0), true, ImGuiWindowFlags_ChildWindowTitle);
		{
			ImGui::Checkbox("grenade prediction [?]", &settings::visuals::grenade_prediction::enable);        
            ImGui::Tooltip("Right-click to access customization");
			if (ImGui::ItemsToolTipBegin("grenade prediction"))
			{
				ImGui::SliderFloat("line thickness##genpred", &settings::visuals::grenade_prediction::line_thickness, 0.f, 5.f, "%.1f");
				ImGui::SliderFloat("colision box size##genpred", &settings::visuals::grenade_prediction::colision_box_size, 0.f, 10.f, "%.1f");
				ImGui::SliderFloat("main colision box size##genpred", &settings::visuals::grenade_prediction::main_colision_box_size, 0.f, 10.f, "%.1f");

				ImGui::ColorEdit3("line color##genpred", &settings::visuals::grenade_prediction::main);
				ImGui::ColorEdit3("colision box color##genpred", &settings::visuals::grenade_prediction::main_box);
				ImGui::ColorEdit3("end colision box color##genpred", &settings::visuals::grenade_prediction::end_box);

				ImGui::ItemsToolTipEnd();
			}

			/*ImGui::Checkbox("draw fov basic [?]", &settings::visuals::fov::fov_basic); //ARRUMAR
			if (ImGui::ItemsToolTipBegin("fovsettings##basic"))
			{
				ImGui::ColorEdit3("color basic##basic", &settings::visuals::fov::color_fov_basic);
				ImGui::Checkbox("backgorund##basic", &settings::visuals::fov::fov_basic_background);
				if (settings::visuals::fov::fov_basic_background)
				{
					ImGui::SameLine();
					ImGui::ColorEdit3("##background_basic", &settings::visuals::fov::color_fov_basic_background);
				}
				ImGui::ItemsToolTipEnd();
			}*/

			/*ImGui::Checkbox("draw fov silent [?]", &settings::visuals::fov::fov_silent);
			if (ImGui::ItemsToolTipBegin("fovsettings##silent"))
			{
				ImGui::ColorEdit3("color silent##silent", &settings::visuals::fov::color_fov_silent);
				ImGui::Checkbox("backgorund##silent", &settings::visuals::fov::fov_silent_background);
				if (settings::visuals::fov::fov_silent_background)
				{
					ImGui::SameLine();
					ImGui::ColorEdit3("##fov_silent_backgound", &settings::visuals::fov::color_fov_silent_background);
				}
				ImGui::ItemsToolTipEnd();
			}*/

			//ImGui::Checkbox("choke indicator", &settings::misc::choke_indicator);//ARRUMAR
		//	ImGui::Checkbox("hitmarker", &settings::visuals::hitmarker);
		//	ImGui::Checkbox("angle lines", &settings::misc::desync::indicator::lines);
			ImGui::Checkbox("draw fov", &settings::misc::draw_fov);
			//ImGui::Checkbox("hitmarker", &settings::misc::hitmarker);
			ImGui::Checkbox("nightmode", &settings::misc::nightmode);
			ImGui::Checkbox("bomb timer", &settings::visuals::bomb_timer);
			ImGui::Checkbox("spreed circle", &settings::misc::spreed_circle::enable);
			ImGui::SameLine();
			ImGui::ColorEdit3("##spreed circle color", &settings::misc::spreed_circle::color);
		//	ImGui::Checkbox("sniper crosshair", &settings::visuals::sniper_crosshair::enable);
			//	ImGui::SameLine();
			//	ImGui::ColorEdit3("##sniper crosshair color", &settings::misc::sniper_crosshair::color);


			const char* events_c[] = { "hit", "buy", "planting", "defusing", "configs" };

			bool* events_screen_b[5]{ &settings::visuals::events::screen::hurt, &settings::visuals::events::screen::player_buy, &settings::visuals::events::screen::planting, &settings::visuals::events::screen::defusing, &settings::visuals::events::screen::config };
			bool* events_console_b[5]{ &settings::visuals::events::console::hurt, &settings::visuals::events::console::player_buy, &settings::visuals::events::console::planting, &settings::visuals::events::console::defusing, &settings::visuals::events::console::config };

			ImGui::MultiCombo("events on screen", events_c, events_screen_b, 5);
			ImGui::MultiCombo("events in console", events_c, events_console_b, 5);
		}
		ImGui::EndChild("general##visuals_general1");
		ImGui::NextColumn();
		ImGui::BeginChild("grenade's's##visuals_grenade's", ImVec2(0, 199), true, ImGuiWindowFlags_ChildWindowTitle);
		{
			const char* time_type[] = { "bar", "text" };

			bool* smoke_time_types[2]{ &settings::visuals::grenades::smoke_bar, &settings::visuals::grenades::smoke_timer };
			bool* molotov_time_types[2]{ &settings::visuals::grenades::molotov_bar, &settings::visuals::grenades::molotov_timer };

			const char* grenadesarray[] = { "smoke", "flahsbang", "he grenade", "decoy", "molotov" };
			static int selected_grenade;

			ImGui::Checkbox("granade's##check", &settings::visuals::grenades::enable);

			ImGui::Combo("grenade", &selected_grenade, grenadesarray, IM_ARRAYSIZE(grenadesarray));

			if (selected_grenade == 0)
			{
				ImGui::ColorEdit3("name##smoke", &settings::visuals::grenades::color_smoke);
				ImGui::NewLine();
				ImGui::MultiCombo("time display type###smoke", time_type, smoke_time_types, 2);
				if (smoke_time_types[0])
				{
					ImGui::ColorEdit3("bar main##smoke", &settings::visuals::grenades::color_bar_smoke_main);
					ImGui::ColorEdit3("bar back##smoke", &settings::visuals::grenades::color_bar_smoke_back);
				}
			}
			else if (selected_grenade == 1)
			{
				ImGui::ColorEdit3("name##flashbang", &settings::visuals::grenades::color_flash);
			}
			else if (selected_grenade == 2)
			{
				ImGui::ColorEdit3("name##hegrenade", &settings::visuals::grenades::color_frag);
			}
			else if (selected_grenade == 3)
			{
				ImGui::ColorEdit3("name##decoy", &settings::visuals::grenades::color_decoy);
			}
			else if (selected_grenade == 4)
			{
				ImGui::ColorEdit3("name##molotov", &settings::visuals::grenades::color_molotov);
				ImGui::NewLine();
				ImGui::MultiCombo("time display type##molotov", time_type, molotov_time_types, 2);
				if (molotov_time_types[0])
				{
					ImGui::ColorEdit3("bar main##molotov", &settings::visuals::grenades::color_bar_molotov_main);
					ImGui::ColorEdit3("bar back##molotov", &settings::visuals::grenades::color_bar_molotov_back);
				}
			}
		}
		ImGui::EndChild("grenade's##visuals_grenade's");

		ImGui::BeginChild("hitsound ##sound", ImVec2(0, 96), true, ImGuiWindowFlags_ChildWindowTitle);
		{
			ImGui::Checkbox("enable", &settings::visuals::hitsound);
			if (settings::visuals::hitsound) {
				ImGui::Combo("Hitmarker Sound", &settings::visuals::hitsoundtrade, hitmarkersounds, IM_ARRAYSIZE(hitmarkersounds));
			}
		}
		ImGui::EndChild("hitsound ##sound");

		ImGui::BeginChild("fog##visualsfog", ImVec2(0, 109), true, ImGuiWindowFlags_ChildWindowTitle);
		{
			ImGui::Checkbox("enable##fog", &settings::misc::fog::enable);
			ImGui::SameLine();
			ImGui::ColorEdit3("##color", &settings::misc::fog::color);

			float max_min = settings::misc::fog::end_dist - 10;

			max_min = std::clamp(max_min, 0.f, settings::misc::fog::end_dist);

			ImGui::SliderFloat("fog start dist", &settings::misc::fog::start_dist, 0.f, max_min, "%.0f");
			ImGui::SliderFloat("fog end dist", &settings::misc::fog::end_dist, 0.f, 10000.f, "%.0f");
		}
		ImGui::EndChild("fog##visualsfog");

		ImGui::BeginChild("world##childchams", ImVec2(0, 110), true, ImGuiWindowFlags_ChildWindowTitle);
		{
			ImGui::Dummy(ImVec2(menu::_style.WindowPadding.x, 0.0f));


			ImGui::Checkbox("sound esp##ragdoll", &settings::misc::soundenable);
			ImGui::SameLine();
			ImGui::ColorEdit3("##Sound color", &settings::misc::soundcolor, ImGuiColorEditFlags_NoInputs);
			ImGui::SliderFloat("Duration", &settings::misc::soundtime, 0.5f, 3.0f, "%.3f ms");
			ImGui::SliderFloat("Radius", &settings::misc::soundradio, 15.0f, 150.0f, "%.1f");


			ImGui::NewLine();
		}
		ImGui::EndChild("world##visuals_chams");

		
		ImGui::BeginChild("skybox changer##sky", ImVec2(0, 96), true, ImGuiWindowFlags_ChildWindowTitle);
		{
			ImGui::Checkbox("enable", &settings::misc::skyboxenable);
			if (settings::misc::skyboxenable) {
				ImGui::Combo("##SkyboxChanger1", &settings::misc::skyboxchanger, std::vector<std::string>{ "vertigo", "sky_csgo_night02", "sky_cs15_daylight02_hdr",
					"cs_baggage_skybox_", "sky_csgo_cloudy01", "sky_venice", "jungle", "vietnam", "embassy"});
			}
		}
		ImGui::EndChild("skybox changer##sky");
	}
}

//-------------------------------------------------MISC TAB----------------------------------------------------------------------------//

void misc_tab()
{
	auto& style = ImGui::GetStyle();

	ImGui::Columns(2, nullptr, false);
	ImGui::BeginChild("Misc##misctab", ImVec2(0, 330), true, ImGuiWindowFlags_ChildWindowTitle);
	{
		ImGui::ColorEdit4("menu color", &settings::misc::menu_color);
		ImGui::Checkbox("in game radar", &settings::visuals::radar_ingame);
		ImGui::Checkbox("block bot", &settings::misc::block_bot::enable);
        ImGui::Tooltip("Right click on the bind to customize");
		ImGui::SameLine();
		ImGui::KeyBind("##bock_bot_bind", &settings::misc::block_bot::bind, ImGuiKeyBindFlags_OnItem);
		ImGui::Checkbox("anti obs", &settings::misc::anti_obs);
		ImGui::Checkbox("left hand knife ", &settings::misc::lefthandknife);
		ImGui::Checkbox("sniper Crosshair", &settings::misc::visuals_crosshair);
		ImGui::Checkbox("door spam", &settings::misc::doorspam);
		ImGui::SameLine();
		ImGui::KeyBind("##slow wa12312312lk", &settings::misc::doorspambind, ImGuiKeyBindFlags_OnItem);
       
		ImGui::Checkbox("rank reveal", &settings::misc::reveal_rank);
		ImGui::Checkbox("prepare revolver", &settings::misc::prepare_revolver::enable);
        ImGui::Tooltip("Right click on the bind to customize");
		ImGui::SameLine();
		ImGui::KeyBind("##prepare_revolver_bind", &settings::misc::prepare_revolver::bind, ImGuiKeyBindFlags_OnItem);


		const char* disables_c[5] = { "smoke", "flash", "zoom", "zoom border" };
		bool* disables_b[5] = { &settings::misc::disable_smoke, &settings::misc::disable_flash, &settings::misc::disable_zoom, &settings::misc::disable_zoom_border };
		ImGui::MultiCombo("disable##misc", disables_c, disables_b, 4);
		if (settings::misc::disable_flash)
		{
			ImGui::SliderInt("max alpha##disable_flash", &settings::misc::flash_alpha, 0, 255);
		}

		ImGui::Checkbox("watermark", &settings::misc::watermark);
		ImGui::Checkbox("third Person", &settings::misc::third_person::enable);
        ImGui::Tooltip("Right click on the bind to customize");
		ImGui::SameLine();
		ImGui::KeyBind("##third_Person_bind", &settings::misc::third_person::bind, ImGuiKeyBindFlags_OnItem);
		if (settings::misc::third_person::enable)
		{
			ImGui::SliderFloat("dist##third_Person_", &settings::misc::third_person::dist, 25.f, 350.f, "%.1f");
		}
		ImGui::Checkbox("bind window [?]", &settings::misc::bind_window::enable);
        ImGui::Tooltip("Right-click to access customization");
		if (ImGui::ItemsToolTipBegin("##bind_window"))
		{
			ImGui::SliderFloat("alpha##bind_window", &settings::windows::Bind_alpha, 0.f, 1.f, "%.1f");
			ImGui::ItemsToolTipEnd();
		}
		ImGui::Checkbox("spectator list [?]", &settings::visuals::spectator_list);
        ImGui::Tooltip("Right-click to access customization");
		if (ImGui::ItemsToolTipBegin("##spec_window"))
		{
			ImGui::SliderFloat("alpha##spec_window", &settings::windows::Spec_alpha, 0.f, 1.f, "%.1f");
			ImGui::ItemsToolTipEnd();
		}
	}
	ImGui::EndChild("misc##misctab");
	
	

	/*ImGui::BeginChild("clantag##misctab", ImVec2(0, 201), true, ImGuiWindowFlags_ChildWindowTitle);
	{
		auto TabsW_players_tab = (ImGui::GetCurrentWindow()->Size.x - menu::_style.WindowPadding.x * 2.0f) / _countof(clantag_tabs);

		ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing_new, ImVec2(0, 0));
		{
			render_tabs(clantag_tabs, settings::misc::clantag::clantag_type, TabsW_players_tab, 20.0f);
		}
		ImGui::PopStyleVar();

		ImGui::Checkbox("enable##clantag", &settings::misc::clantag::enable);

		switch (settings::misc::clantag::clantag_type)
		{
		case 0:
			ImGui::Combo("clantag's##clanTag", &settings::misc::clantag::animation_type, std::vector<std::string>{"N3WProject", "gamesense", "aimware.net", "onetap", "iniuria", "polakpaste", "koezpaste"});
			break;
		case 1:
			if (g_player_resource)
			{
				auto player_resource = *g_player_resource;
				if (player_resource)
				{
					ImVec2 becup = style.WindowPadding;
					float bec = style.FramePadding.x;

					if (ImGui::BeginCombo("player tags [?]##clantags.list", "tags"))
					{
						std::vector<std::string> tags;

						if (ImGui::Selectable("none"))
						{
							misc::clan_tag::user_index = 0;
						}

						for (int i = 1; i < g_engine_client->get_max_clients(); ++i)
						{
							auto* player = c_base_player::get_player_by_index(i);

							if (!player)
								continue;

							const auto info = player->get_player_info();
							if (info.fakeplayer)
								continue;

							auto usertag = std::string(player_resource->szclantag()[player->get_index()]);
							auto user_name = std::string(info.szName);
							if (settings::misc::clantag::check_empty)
							{
								if (usertag.empty() || std::find(tags.begin(), tags.end(), usertag) != tags.end())
									continue;
							}

							tags.push_back(usertag);

							user_name += " : ";
							user_name += usertag;

							if (player->get_index() != g_local_player->get_index())
							{
								if (ImGui::Selectable(user_name.c_str()))
								{
									misc::clan_tag::user_index = i;
								}
							}
						}

						ImGui::EndCombo();
					}
					style.WindowPadding.x = becup.x;
					style.WindowPadding.y = becup.y;
					style.FramePadding.x = bec;
					if (ImGui::ItemsToolTipBegin("##player_tag_help"))
					{
						ImGui::Checkbox("checking on empty tag at player", &settings::misc::clantag::check_empty);
						ImGui::ItemsToolTipEnd();
					}
				}
				else
				{
					ImGui::TextS("connect to server");
				}
			}
			else
			{
				ImGui::TextS("connect to server");
			}
			break;
		case 2:
			ImGui::InputText("clantag##misc_clantag", localtag, sizeof(localtag));
			if (ImGui::ButtonS("apply##clan", ImVec2(216, 0)))
			{
				settings::misc::clantag::clantag = localtag;
				settings::misc::clantag::clantag_visible = localtag;
				utils::set_clantag(settings::misc::clantag::clantag);
			}

			/*ImGui::Combo("animation", &settings::misc::clantag::custom_type, "none\0type 1\0type 2");
			if (settings::misc::clantag::clantag_type != 0)
			{
				ImGui::SliderFloat("speed", &settings::misc::clantag::speed, 0.1f, 2.f);
			}*
			break;
			break;
		}
	}
	ImGui::EndChild("clantag##misctab");*/

	const char* fastStopModes[] = {
	"Off",
	"Left & Right",
	"Forward & Backward",
	"Both"
	};


	bool* shadowsoptions[8]{ &settings::misc::performance::no_shadows2, &settings::misc::performance::no_shadows3, &settings::misc::performance::no_shadows4, &settings::misc::performance::no_shadows5, &settings::misc::performance::no_shadows6, &settings::misc::performance::no_shadows7, &settings::misc::performance::no_shadows8, &settings::misc::performance::no_shadows9 };
	const char* shadowsoptions1[8] = { "1", "2", "3", "4", "5", "6", "7", "8" };

	ImGui::BeginChild("performance##misctab", ImVec2(0, 152), true, ImGuiWindowFlags_ChildWindowTitle);
	{
		ImGui::Checkbox("Disable panorama blur", &settings::misc::performance::disablepanorama);
		ImGui::Checkbox("No Sky", &settings::misc::performance::no_sky);
		ImGui::Checkbox("No Post Processing", &settings::misc::performance::processing);
		ImGui::Checkbox("No Shadows", &settings::misc::performance::no_shadows);
		if (settings::misc::performance::no_shadows) {
			ImGui::MultiCombo("type's", shadowsoptions1, shadowsoptions, 8);
		}

	}
	ImGui::EndChild("performance##misctab");

	ImGui::NextColumn();

	ImGui::BeginChild("server region##other_accept_misc", ImVec2(0, 90), true, ImGuiWindowFlags_ChildWindowTitle);
	{
		ImGui::Combo("region: ##RegionChanger_type", &settings::misc::region_changer, std::vector<std::string>{ "Atlanta (us)", "Los angeles (us)",
			"Seattle (us)",
			"Sao paulo (brazil)",
			"Sydney (australia)",
			"Amsterdam (europe)",
			"Madrid (europe)",
			"London (europe)",
			"Tokyo (japan)"
		});
		

		ImGui::Checkbox("Auto-accept matchmaking", &settings::misc::auto_accept);
		ImGui::SameLine();
		if (ImGui::Button("Apply", ImVec2(120, 0)))
		{
			misc::changer_region();
		}
	}
	ImGui::EndChild("server region##other_accept_misc");

	ImGui::BeginChild("Viewmodel##misctab", ImVec2(0, 222), true, ImGuiWindowFlags_ChildWindowTitle);
	{
		    ImGui::Checkbox("enable", &settings::misc::viewmodelenable);
			ImGui::SliderFloat("override fov", &settings::misc::viewmodel::override, 60, 150, "%.1f");
			ImGui::SliderFloat("viewmodel fov", &settings::misc::viewmodel::viewmodel, 42, 120, "%.1f");
			ImGui::SliderFloat("viewmodel x", &settings::misc::viewmodel::fov_x, -30, 30, "%.1f");
			ImGui::SliderFloat("viewmodel y", &settings::misc::viewmodel::fov_z, -30, 30, "%.1f");
			ImGui::SliderFloat("viewmodel z", &settings::misc::viewmodel::fov_y, -30, 30, "%.1f");
			ImGui::SliderFloat("aspect ratio", &settings::misc::viewmodel::aspect_ratio, 0, 200, "%.0f");
	}
	ImGui::EndChild("viewmodel##misctab");
	
	ImGui::BeginChild("movement ##misctab", ImVec2(0, 265), true, ImGuiWindowFlags_ChildWindowTitle);
	{
		ImGui::Checkbox("bhop (?)##bhop", &settings::misc::bhop::enable);
		ImGui::Tooltip("Right-click to access customization");
		if (ImGui::ItemsToolTipBegin("bhop"))
		{
			ImGui::Checkbox("Humanised Bhop##bhop", &settings::misc::bhop::humanised);
			{
				ImGui::SliderFloat("Bhop Hit Chance", &settings::misc::bhop::bhop_hit_chance, 0.f, 100.f, "%.1f");
				ImGui::Tooltip("The chance of second hop, first hop is always at 100%");
				ImGui::SliderInt("Hops Limit", &settings::misc::bhop::hops_restricted_limit, 0, 12);
				ImGui::Tooltip("Will fuckup the bhop after certain amount of hops to look legit.");
				ImGui::SliderInt("Max Hops Limit", &settings::misc::bhop::max_hops_hit, 0, 12);
				ImGui::Tooltip("Will fuckup the bhop after certain amount of hops to look legit.");

			}
			ImGui::ItemsToolTipEnd();
		}

		ImGui::Checkbox("auto strafe##bhop", &settings::misc::bhop::auto_strafer);
		if (settings::misc::bhop::auto_strafer)
		{
			ImGui::Combo("auto strafe type", &settings::misc::bhop::strafer_type, std::vector<std::string>{ "standart", "auto" });
			if (settings::misc::bhop::strafer_type == 0)
				ImGui::SliderFloat("retrack speed##bhop", &settings::misc::bhop::retrack_speed, 1.0f, 10.0f, "%.1f");
		}
		
		ImGui::Checkbox("edge jump [?]", &settings::misc::edge_jump::enable);
		ImGui::Tooltip("Right-click to access customization");
		if (ImGui::ItemsToolTipBegin("##edge_jump_help"))
		{
			ImGui::Checkbox("auto duck", &settings::misc::edge_jump::auto_duck);
			ImGui::ItemsToolTipEnd();
		}
		ImGui::SameLine();
		ImGui::KeyBind("##edge_bind", &settings::misc::edge_jump::bind, ImGuiKeyBindFlags_OnItem);
		ImGui::Checkbox("velocity indicator [?]", &settings::visuals::velocityindicator);
		if (ImGui::ItemsToolTipBegin("##veloindicaotrhelp"))
		{
			ImGui::Checkbox("last jump", &settings::visuals::lastvelocityjump);
			ImGui::ColorEdit3("indicator color", &settings::visuals::Velocitycol);

			ImGui::ItemsToolTipEnd();
		}


		ImGui::Checkbox("fast stop", &settings::misc::fast_stop);
		if (settings::misc::fast_stop) {
			ImGui::Combo("fast stop type", &settings::misc::fast_stop_mode, fastStopModes, IM_ARRAYSIZE(fastStopModes));
		}

		//ImGui::Checkbox("jump bug", &settings::misc::jumpbug::enable);
		//ImGui::SameLine();
		//ImGui::KeyBind("##jump213132", &settings::misc::jumpbug::bind, ImGuiKeyBindFlags_OnItem);

		
		ImGui::Checkbox("edgebug", &settings::misc::edge_bug);
		ImGui::SameLine();
		ImGui::KeyBind("##jump213131232", &settings::misc::edgebugkey, ImGuiKeyBindFlags_OnItem);
		ImGui::Checkbox("auto runboost", &settings::misc::autorunbst::enable);
		ImGui::SameLine();
		ImGui::KeyBind("##jump2131315615615652", &settings::misc::autorunbst::bind, ImGuiKeyBindFlags_OnItem);
		ImGui::Checkbox("fast duck", &settings::misc::fast_duck);
		ImGui::Checkbox("moon walk", &settings::misc::moon_walk);
	}
	ImGui::EndChild("movement ##misctab");
}

//-------------------------------------------------PROFILE TAB-------------------------------------------------------------------------//
/*void profile_tab() //ARRUMAR DEPOIS
{
	const char* ranks[] = {
	"off",
	"silver 1",
	"silver 2",
	"silver 3",
	"silver 4",
	"silver elite",
	"silver elite master",
	"gold nova 1",
	"gold nova 2",
	"gold nova 3",
	"gold nova master",
	"master guardian 1",
	"master guardian 2",
	"master guardian elite",
	"distinguished master guardian",
	"legendary eagle",
	"legendary eagle master",
	"supreme master first class",
	"the global elite"
	};
	const char* ranks_dz[] = {
		"off",
		"lab rat 1",
		"lab rat 2",
		"sprinting hare 1",
		"sprinting hare 2",
		"wild scout 1",
		"wild scout 2",
		"wild scout elite",
		"hunter fox 1",
		"hunter fox 2",
		"hunter fox 3",
		"hunter fox elite",
		"timber wolf",
		"ember wolf",
		"wildfire wolf",
		"the howling alpha"
	};
	const char* private_ranks[] = {
"Private rank 1                | 1",
"Private rank 2                | 2",
"Private rank 3                | 3",
"Private rank 4                | 4",
"Corporal 1                      | 5",
"Corporal 2                      | 6",
"Corporal 3                      | 7",
"Corporal 4                      | 8",
"Sergeant 1                      | 9",
"Sergeant 2                      | 10",
"Sergeant 3                      | 11",
"Sergeant 4                      | 12",
"Master Sergeant 1         | 13",
"Master Sergeant 2         | 14",
"Master Sergeant 3         | 15",
"Master Sergeant 4         | 16",
"Sergeant Major 1           | 17",
"Sergeant Major 2           | 18",
"Sergeant Major 3           | 19",
"Sergeant Major 4           | 20",
"Lieutenant 1                   | 21",
"Lieutenant 2                   | 22",
"Lieutenant 3                   | 23",
"Lieutenant 4                   | 24",
"Captain 1                        | 25",
"Captain 2                        | 26",
"Captain 3                        | 27",
"Captain 4                        | 28",
"Major 1                           | 29",
"Major 2                           | 30",
"Major 3                           | 31",
"Major 4                           | 32",
"Colonel 1                        | 33",
"Colonel 2                        | 34",
"Colonel 3                        | 35",
"Brigadier General          | 36",
"Major General                | 37",
"Lieutenant General        | 38",
"Ggeneral                         | 39",
"Global General               | 40"
	};
	const char* bans[] = {
		"none",
		"you were kicked from the last match (competitive cooldown)",
		"you killed too many teammates (competitive cooldown)",
		"you killed a teammate at round start (competitive cooldown)",
		"you failed to reconnect to the last match (competitive cooldown)",
		"you abandoned the last match (competitive cooldown)",
		"you did too much damage to your teammates (competitive cooldown)",
		"you did too much damage to your teammates at round start (competitive cooldown)",
		"this account is permanently untrusted (global cooldown)",
		"you were kicked from too many recent matches (competitive cooldown)",
		"convicted by overwatch - majorly disruptive (global cooldown)",
		"convicted by overwatch - minorly disruptive (global cooldown)",
		"resolving matchmaking state for your account (temporary cooldown)",
		"resolving matchmaking state after the last match (temporary cooldown)",
		"this account is permanently untrusted (global cooldown)",
		"(global cooldown)",
		"you failed to connect by match start. (competitive cooldown)",
		"you have kicked too many teammates in recent matches (competitive cooldown)",
		"congratulations on your recent competitive wins! before you play competitive matches further please wait for matchmaking servers to calibrate your skill group placement based on your lastest performance. (temporary cooldown)",
		"a server using your game server login token has been banned. your account is now permanently banned from operating game servers, and you have a cooldown from connecting to game servers. (global cooldown)"
	};

	static char* modes[] =
	{
		"matchmaking",
		"wingman",
		"danger zone"
	};

	ImGui::Columns(2, nullptr, false);
	ImGui::BeginChild("general##profile_changer", ImVec2(0, 200), true, ImGuiWindowFlags_ChildWindowTitle);
	{
		ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing_new, ImVec2(0, 0));
		{
			auto TabsW = (ImGui::GetCurrentWindow()->Size.x - menu::_style.WindowPadding.x * 2.0f) / _countof(modes);
			render_tabs(modes, settings::changers::profile::mode_id, TabsW, 20.0f);
		}
		ImGui::PopStyleVar();

		ImGui::Dummy(ImVec2(menu::_style.WindowPadding.x, 0.0f));

		if (settings::changers::profile::mode_id == 2)
			ImGui::Combo("rank", &settings::changers::profile::profile_items[settings::changers::profile::mode_id].rank_id, ranks_dz, IM_ARRAYSIZE(ranks_dz));
		else
			ImGui::Combo("rank", &settings::changers::profile::profile_items[settings::changers::profile::mode_id].rank_id, ranks, IM_ARRAYSIZE(ranks));

		ImGui::InputInt("wins", &settings::changers::profile::profile_items[settings::changers::profile::mode_id].wins, 1, 20);

		ImGui::Combo("private rank", &settings::changers::profile::private_id, private_ranks, IM_ARRAYSIZE(private_ranks));
		ImGui::SliderInt("xp", &settings::changers::profile::private_xp, 0, 5000);

	}
	ImGui::EndChild("general##profile_changer");


	if (ImGui::Button("update", ImVec2(285, 0)))
	{
		utils::message_send();

		globals::profile::update_request = true;
	}

	ImGui::NextColumn();

	ImGui::BeginChild("comment's##profile", ImVec2(0, 156), true, ImGuiWindowFlags_ChildWindowTitle);
	{
		ImGui::InputInt("friendly", &settings::changers::profile::friendly, 1, 10);
		ImGui::InputInt("teacher", &settings::changers::profile::teacher, 1, 10);
		ImGui::InputInt("leader", &settings::changers::profile::leader, 1, 10);
	}
	ImGui::EndChild("comment's##profile");
	ImGui::BeginChild("ban##profile", ImVec2(0, 144), true, ImGuiWindowFlags_ChildWindowTitle);
	{
		ImGui::Combo("ban type", &settings::changers::profile::ban_type, bans, IM_ARRAYSIZE(bans));
		ImGui::Combo("time type", &settings::changers::profile::ban_time_type, "sec\0min\0hour\0days");
		ImGui::InputInt("time", &settings::changers::profile::ban_time, 1, 100);
	}
	ImGui::EndChild("ban##profile");

}*/
//-------------------------------------------------SKINS TAB---------------------------------------------------------------------------//
/*
struct hud_weapons_t {
	std::int32_t* get_weapon_count() {
		return reinterpret_cast<std::int32_t*>(std::uintptr_t(this) + 0x80);
	}
};
template<class T>
static T* FindHudElement(const char* name)
{
	static auto pThis = *reinterpret_cast<DWORD**>(utils::pattern_scan(GetModuleHandleA("client.dll"), "B9 ? ? ? ? E8 ? ? ? ? 8B 5D 08") + 1);

	static auto find_hud_element = reinterpret_cast<DWORD(__thiscall*)(void*, const char*)>(utils::pattern_scan(GetModuleHandleA("client.dll"), "55 8B EC 53 8B 5D 08 56 57 8B F9 33 F6 39 77 28"));
	return (T*)find_hud_element(pThis, name);
}

static std::string selected_weapon_name = "";
static std::string selected_skin_name = "";
static auto definition_vector_index = 0;
void skins_tab()
{
	auto& entries = settings::changers::skin::m_items;
	ImGui::Columns(2, nullptr, false);
	ImGui::BeginChild("weapon select##skin window", ImVec2(0.f, 0.f), true, ImGuiWindowFlags_ChildWindowTitle);
	{
		ImGui::ListBoxHeader("weapons##sjinstab", ImVec2(0, 306));
		{
			for (size_t w = 0; w < k_weapon_names.size(); w++)
			{
				switch (w)
				{
				case 0:
					ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.f), "Knife");
					break;
				case 2:
					ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.f), "Glove");
					break;
				case 4:
					ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.f), "Pistols");
					break;
				case 14:
					ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.f), "Semi-Rifle");
					break;
				case 21:
					ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.f), "Rifle");
					break;
				case 28:
					ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.f), "Snipers");
					break;
				case 32:
					ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.f), "Machingun");
					break;
				case 34:
					ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.f), "Shotgun");
					break;
				}

				if (ImGui::Selectable(k_weapon_names[w].name.c_str(), definition_vector_index == w))
				{
					definition_vector_index = w;
				}
			}
		}
		ImGui::ListBoxFooter();

		static float next_enb_time = 0;

		float time_to_next_up = g_global_vars->curtime;

		time_to_next_up = std::clamp(next_enb_time - g_global_vars->curtime, 0.f, 1.f);

		std::string name = "update (";
		name += std::to_string(time_to_next_up);
		name.erase(12, 16);
		name += ")";

		if (ImGui::ButtonS(name.c_str(), ImVec2(221, 0)))
		{
			if (next_enb_time <= g_global_vars->curtime)
			{
				static auto clear_hud_weapon_icon_fn =
					reinterpret_cast<std::int32_t(__thiscall*)(void*, std::int32_t)>(
						utils::pattern_scan(GetModuleHandleA("client.dll"), "55 8B EC 51 53 56 8B 75 08 8B D9 57 6B FE 2C 89 5D FC"));

				auto element = FindHudElement<std::uintptr_t*>("CCSGO_HudWeaponSelection");

				if (element)
				{
					auto hud_weapons = reinterpret_cast<hud_weapons_t*>(std::uintptr_t(element) - 0xa0);
					if (hud_weapons != nullptr)
					{

						if (*hud_weapons->get_weapon_count())
						{
							for (std::int32_t i = 0; i < *hud_weapons->get_weapon_count(); i++)
								i = clear_hud_weapon_icon_fn(hud_weapons, i);

							typedef void(*ForceUpdate) (void);
							static ForceUpdate FullUpdate = (ForceUpdate)utils::pattern_scan(GetModuleHandleA("engine.dll"), "A1 ? ? ? ? B9 ? ? ? ? 56 FF 50 14 8B 34 85");
							FullUpdate();

							g_client_state->force_full_update();
						}
					}
				}

				next_enb_time = g_global_vars->curtime + 1.f;
			}
		}
	}
	ImGui::EndChild("weapon select");


	ImGui::NextColumn();

	ImGui::BeginChild("skin select##skin tab", ImVec2(0, 0), true, ImGuiWindowFlags_ChildWindowTitle);
	{
		static int selected_tab_skins = 0;

		auto& selected_entry = entries[k_weapon_names[definition_vector_index].definition_index];
		auto& satatt = settings::changers::skin::statrack_items[k_weapon_names[definition_vector_index].definition_index];
		selected_entry.definition_index = k_weapon_names[definition_vector_index].definition_index;
		selected_entry.definition_vector_index = definition_vector_index;

		ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing_new, ImVec2(0, 0));
		{
			static char* Players_tabs[] = { "general", "paint kit" };

			auto TabsW_players_tab = (ImGui::GetCurrentWindow()->Size.x - menu::_style.WindowPadding.x * 2.0f) / _countof(Players_tabs);

			render_tabs(Players_tabs, selected_tab_skins, TabsW_players_tab, 20.0f);
		}
		ImGui::PopStyleVar();

		if (selected_entry.definition_index != GLOVE_T_SIDE &&
			selected_entry.definition_index != GLOVE_CT_SIDE &&
			selected_entry.definition_index != WEAPON_KNIFE &&
			selected_entry.definition_index != WEAPON_KNIFE_T)
		{
			selected_weapon_name = k_weapon_names_preview[definition_vector_index].name;
		}
		else
		{
			if (selected_entry.definition_index == GLOVE_T_SIDE ||
				selected_entry.definition_index == GLOVE_CT_SIDE)
			{
				selected_weapon_name = k_glove_names_preview.at(selected_entry.definition_override_vector_index).name;
			}
			if (selected_entry.definition_index == WEAPON_KNIFE ||
				selected_entry.definition_index == WEAPON_KNIFE_T)
			{
				selected_weapon_name = k_knife_names_preview.at(selected_entry.definition_override_vector_index).name;
			}
		}

		if (selected_tab_skins == 0)
		{
			ImGui::Checkbox("skin preview", &settings::changers::skin::skin_preview);
			ImGui::Checkbox("stattrak##2", &selected_entry.stat_trak);
			ImGui::InputInt("seed", &selected_entry.seed);
			//ImGui::InputText("custom name##skins.nametag", selected_entry.custom_name, 32);
			ImGui::InputInt("stattrak", &satatt.statrack_new.counter);
			ImGui::SliderFloat("wear", &selected_entry.wear, FLT_MIN, 1.f, "%.10f", 5);

			if (selected_entry.definition_index == WEAPON_KNIFE || selected_entry.definition_index == WEAPON_KNIFE_T)
			{
				ImGui::ListBoxHeader("knife##sdsdadsdadas", ImVec2(0, 240));
				{
					for (int i = 0; i < k_knife_names.size(); i++)
					{
						if (ImGui::Selectable(k_knife_names[i].name.c_str(), selected_entry.definition_override_vector_index == i))
						{
							selected_entry.definition_override_vector_index = i;

							selected_skin_name = "";
						}
					}

					selected_entry.definition_override_index = k_knife_names.at(selected_entry.definition_override_vector_index).definition_index;
				}
				ImGui::ListBoxFooter();
			}
			else if (selected_entry.definition_index == GLOVE_T_SIDE || selected_entry.definition_index == GLOVE_CT_SIDE)
			{
				ImGui::ListBoxHeader("glove##sdsdadsdadas", ImVec2(0, 152));
				{
					for (int i = 0; i < k_glove_names.size(); i++)
					{
						if (ImGui::Selectable(k_glove_names[i].name.c_str(), selected_entry.definition_override_vector_index == i))
						{
							selected_entry.definition_override_vector_index = i;
						}
					}

					selected_entry.definition_override_index = k_glove_names.at(selected_entry.definition_override_vector_index).definition_index;
				}
				ImGui::ListBoxFooter();
			}
			else {
				static auto unused_value = 0;
				selected_entry.definition_override_vector_index = 0;
			}
		}

		else if (selected_tab_skins == 1)
		{
			if (skins_parsed)
			{
				static char filter_name[32];
				std::string filter = filter_name;

				bool is_glove = selected_entry.definition_index == GLOVE_T_SIDE ||
					selected_entry.definition_index == GLOVE_CT_SIDE;

				bool is_knife = selected_entry.definition_index == WEAPON_KNIFE ||
					selected_entry.definition_index == WEAPON_KNIFE_T;

				int cur_weapidx = 0;
				if (!is_glove && !is_knife)
				{
					cur_weapidx = k_weapon_names[definition_vector_index].definition_index;
					//selected_weapon_name = k_weapon_names_preview[definition_vector_index].name; //test
				}
				else
				{
					if (selected_entry.definition_index == GLOVE_T_SIDE ||
						selected_entry.definition_index == GLOVE_CT_SIDE)
					{
						cur_weapidx = k_glove_names.at(selected_entry.definition_override_vector_index).definition_index;
					}
					if (selected_entry.definition_index == WEAPON_KNIFE ||
						selected_entry.definition_index == WEAPON_KNIFE_T)
					{
						cur_weapidx = k_knife_names.at(selected_entry.definition_override_vector_index).definition_index;

					}
				}

				ImGui::InputText("name filter [?]", filter_name, sizeof(filter_name));
                ImGui::Tooltip("Right-click to access customization");
				if (ImGui::ItemsToolTipBegin("##skinfilter"))
				{
					ImGui::Checkbox("show skins for selected weapon", &settings::changers::skin::show_cur);
					ImGui::ItemsToolTipEnd();
				}

				auto weaponName = weaponnames(cur_weapidx);

				ImGui::ListBoxHeader("skins##sdsdadsdadas", ImVec2(0, 261));
				{
					if (selected_entry.definition_index != GLOVE_T_SIDE && selected_entry.definition_index != GLOVE_CT_SIDE)
					{
						if (ImGui::Selectable(" - ", selected_entry.paint_kit_index == -1))
						{
							selected_entry.paint_kit_vector_index = -1;
							selected_entry.paint_kit_index = -1;
							selected_skin_name = "";
						}

						int lastID = ImGui::GetItemID();
						for (size_t w = 0; w < k_skins.size(); w++)
						{
							for (auto names : k_skins[w].weaponName)
							{
								std::string name = k_skins[w].name;

								if (settings::changers::skin::show_cur)
								{
									if (names != weaponName)
										continue;
								}

								if (name.find(filter) != name.npos)
								{
									ImGui::PushID(lastID++);

									ImGui::PushStyleColor(ImGuiCol_Text, skins::get_color_ratiry(is_knife && settings::changers::skin::show_cur ? 6 : k_skins[w].rarity));
									{
										if (ImGui::Selectable(name.c_str(), selected_entry.paint_kit_vector_index == w))
										{
											selected_entry.paint_kit_vector_index = w;
											selected_entry.paint_kit_index = k_skins[selected_entry.paint_kit_vector_index].id;
											selected_skin_name = k_skins[w].name_short;
										}
									}
									ImGui::PopStyleColor();

									ImGui::PopID();
								}
							}
						}
					}
					else
					{
						int lastID = ImGui::GetItemID();

						if (ImGui::Selectable(" - ", selected_entry.paint_kit_index == -1))
						{
							selected_entry.paint_kit_vector_index = -1;
							selected_entry.paint_kit_index = -1;
							selected_skin_name = "";
						}

						for (size_t w = 0; w < k_gloves.size(); w++)
						{
							for (auto names : k_gloves[w].weaponName)
							{
								std::string name = k_gloves[w].name;
								//name += " | ";
								//name += names;

								if (settings::changers::skin::show_cur)
								{
									if (names != weaponName)
										continue;
								}

								if (name.find(filter) != name.npos)
								{
									ImGui::PushID(lastID++);

									ImGui::PushStyleColor(ImGuiCol_Text, skins::get_color_ratiry(6));
									{
										if (ImGui::Selectable(name.c_str(), selected_entry.paint_kit_vector_index == w))
										{
											selected_entry.paint_kit_vector_index = w;
											selected_entry.paint_kit_index = k_gloves[selected_entry.paint_kit_vector_index].id;
											selected_skin_name = k_gloves[selected_entry.paint_kit_vector_index].name_short;
										}
									}
									ImGui::PopStyleColor();

									ImGui::PopID();
								}
							}
						}
					}
				}
				ImGui::ListBoxFooter();
			}
			else
			{
				ImGui::TextS("skins parsing, wait...");
			}
		}
	}
	ImGui::EndChild("skin select");
}*/

//-------------------------------------------------CONFIG TAB--------------------------------------------------------------------------//

void config_tab()
{
	ImGui::Columns(2, nullptr, false);

ImGui::BeginChild("configs##configs", ImVec2(0, 0), true, ImGuiWindowFlags_ChildWindowTitle);
{
	static std::vector<std::string> configs;

	static auto load_configs = []() {
		std::vector<std::string> items = {};

		std::string path("C:\\N3W\\standart\\");
		if (!fs::is_directory(path))
			fs::create_directories(path);

		for (auto& p : fs::directory_iterator(path))
		{
			if (strstr(p.path().string().substr(path.length()).c_str(), ".cfg"))
			{
				std::string s = p.path().string().substr(path.length());

				items.push_back(s.substr(0, s.length() - 4));
			}
		}

		return items;
	};

	static auto is_configs_loaded = false;
	if (!is_configs_loaded) {
		is_configs_loaded = true;
		configs = load_configs();
	}

	static std::string current_config;

	static char config_name[32];

	ImGui::InputText("config name", config_name, sizeof(config_name));
	if (ImGui::ButtonS("create", ImVec2(221, 0)))
	{
		current_config = std::string(config_name);

		//Config->Save(current_config + ".ini");
		config::standart::save(current_config + ".cfg");
		is_configs_loaded = false;
		memset(config_name, 0, 32);
	}


	ImGui::ListBoxHeader("configs", ImVec2(0, 225));
	{
		for (auto& config : configs)
		{
			if (ImGui::Selectable(config.c_str(), config == current_config)) {
				current_config = config;
			}
			std::string cur_name_tooltip = "##" + config + "configs_ren";
			std::string cur_name_text = "new name##" + config + "configs_ren";
			std::string cur_name_button = "rename##" + config + "configs_ren";
			if (ImGui::ItemsToolTipBegin(cur_name_tooltip.c_str()))
			{
				static char config_rename[32];
				ImGui::InputText(cur_name_text.c_str(), config_rename, sizeof(config_rename));

				if (ImGui::ButtonS(cur_name_button.c_str(), ImVec2(187, 0)))
				{
					if (!config.empty())
					{
						std::string old_name = "C:\\N3W\\standart\\" + config + ".cfg";
						std::string new_name = "C:\\N3W\\standart\\" + std::string(config_rename) + ".cfg";

						rename(old_name.c_str(), new_name.c_str());

						if (settings::visuals::events::screen::config)
							notify::screen::notify("config", "config - '' " + config + " '' renamed to - '' " + config_rename + " ''", Color(settings::misc::menu_color));
						if (settings::visuals::events::console::config)
							notify::console::notify("config", "config - '' " + config + " '' renamed to - '' " + config_rename + " ''", Color(settings::misc::menu_color));

						config = std::string(config_rename);

						is_configs_loaded = false;
					}
				}
				ImGui::ItemsToolTipEnd();
			}
		}
	}
	ImGui::ListBoxFooter();

	if (ImGui::ButtonS("refresh", ImVec2(221, 0)))
		is_configs_loaded = false;

	if (ImGui::ButtonS("load", ImVec2(71, 0)))
	{
		if (!current_config.empty())
		{
			config::standart::load(current_config);
		}
	}
	ImGui::SameLine();

	if (ImGui::ButtonS("save", ImVec2(71, 0)))
	{
		if (!current_config.empty())
		{
			config::standart::save(current_config);
		}
	}
	ImGui::SameLine();

	if (ImGui::ButtonS("delete", ImVec2(71, 0)) && fs::remove("C:\\N3W\\standart\\" + current_config + ".cfg"))
	{
		if (!current_config.empty())
		{
			current_config.clear();
			is_configs_loaded = false;

			if (settings::visuals::events::screen::config)
				notify::screen::notify("config", "config - " + current_config + " deleted", Color(settings::misc::menu_color));
			if (settings::visuals::events::console::config)
				notify::console::notify("config", "config - " + current_config + " deleted", Color(settings::misc::menu_color));
		}
	}
}
ImGui::EndChild("configs##configs");

ImGui::NextColumn();
	/*ImGui::NextColumn();

	ImGui::BeginChild("config skins##configs", ImVec2(0, 0), true, ImGuiWindowFlags_ChildWindowTitle);
	{
		static std::vector<std::string> configs;

		static auto load_configs = []() {
			std::vector<std::string> items = {};

			std::string path("C:\\N3W\\skins\\");
			if (!fs::is_directory(path))
				fs::create_directories(path);

			for (auto& p : fs::directory_iterator(path))
			{
				if (strstr(p.path().string().substr(path.length()).c_str(), ".cfg"))
				{
					std::string s = p.path().string().substr(path.length());

					items.push_back(s.substr(0, s.length() - 4));
				}
			}


			return items;
		};

		static auto is_configs_loaded = false;
		if (!is_configs_loaded)
		{
			is_configs_loaded = true;
			configs = load_configs();
		}

		static std::string current_config;

		static char config_name[32];

		ImGui::InputText("config name##skins", config_name, sizeof(config_name));
		if (ImGui::ButtonS("create##skins", ImVec2(221, 0)))
		{
			current_config = std::string(config_name);

			config::skins::save(current_config + ".cfg");
			is_configs_loaded = false;
			memset(config_name, 0, 32);
		}

		ImGui::ListBoxHeader("configs##skins", ImVec2(0, 225));
		{
			for (auto& config : configs)
			{
				if (ImGui::Selectable(config.c_str(), config == current_config)) {
					current_config = config;
				}
				std::string cur_name_tooltip = "##" + config + "configs_ren";
				std::string cur_name_text = "new name##" + config + "configs_ren";
				std::string cur_name_button = "rename##" + config + "configs_ren skins";
				if (ImGui::ItemsToolTipBegin(cur_name_tooltip.c_str()))
				{
					static char config_rename[32];
					ImGui::InputText(cur_name_text.c_str(), config_rename, sizeof(config_rename));

					if (ImGui::ButtonS(cur_name_button.c_str(), ImVec2(187, 0)))
					{
						if (!config.empty())
						{
							std::string old_name = "C:\\N3W\\skins\\" + config + ".cfg";
							std::string new_name = "C:\\N3W\\skins\\" + std::string(config_rename) + ".cfg";

							rename(old_name.c_str(), new_name.c_str());

							if (settings::visuals::events::screen::config)
								notify::screen::notify("config", "config - '' " + config + " '' renamed to - '' " + config_rename + " ''", Color(settings::misc::menu_color));
							if (settings::visuals::events::console::config)
								notify::console::notify("config", "config - '' " + config + " '' renamed to - '' " + config_rename + " ''", Color(settings::misc::menu_color));

							config = std::string(config_rename);

							is_configs_loaded = false;
						}
					}
					ImGui::ItemsToolTipEnd();
				}
			}
		}
		ImGui::ListBoxFooter();

		if (ImGui::ButtonS("refresh##skins", ImVec2(221, 0)))
			is_configs_loaded = false;



		if (ImGui::ButtonS("load##skins", ImVec2(71, 0)))
		{
			if (!current_config.empty())
			{
				config::skins::load(current_config);
			}
		}
		ImGui::SameLine();

		if (ImGui::ButtonS("save##skins", ImVec2(71, 0)))
		{
			if (!current_config.empty())
			{
				config::skins::save(current_config);
			}
		}
		ImGui::SameLine();

		if (ImGui::ButtonS("delete##skins", ImVec2(71, 0)) && fs::remove("C:\\N3W\\skins\\" + current_config + ".cfg"))
		{
			if (!current_config.empty())
			{
				current_config.clear();
				is_configs_loaded = false;

				if (settings::visuals::events::screen::config)
					notify::screen::notify("config", "config - " + current_config + " deleted", Color(settings::misc::menu_color));
				if (settings::visuals::events::console::config)
					notify::console::notify("config", "config - " + current_config + " deleted", Color(settings::misc::menu_color));
			}
		}
	}
	ImGui::EndChild("config skins##configs");*/
}



//-------------------------------------------------KEY BINDS--------------------------------------------------------------------------//

void add_bind()
{
	create_bind("edge jump", settings::misc::edge_jump::enable, &settings::misc::edge_jump::bind, bind_info_flags_standart);
	create_bind("thirdperson", settings::misc::third_person::enable, &settings::misc::third_person::bind, bind_info_flags_standart);
	create_bind("jump bug", settings::misc::jumpbug::enable, &settings::misc::jumpbug::bind, bind_info_flags_standart);
	create_bind("edge bug", settings::misc::edge_bug, &settings::misc::edgebugkey, bind_info_flags_standart);
	create_bind("door spam", settings::misc::doorspam, &settings::misc::doorspambind, bind_info_flags_standart);
	create_bind("legit aim", settings::legit_bot::bind_check, &settings::legit_bot::bind, bind_info_flags_standart);
	create_bind("block bot", settings::misc::block_bot::enable, &settings::misc::block_bot::bind, bind_info_flags_standart);
	create_bind("prepare revolver", settings::misc::prepare_revolver::enable, &settings::misc::prepare_revolver::bind, bind_info_flags_standart);
	create_bind("auto runboost", settings::misc::autorunbst::enable, &settings::misc::autorunbst::bind, bind_info_flags_standart);


}

void RenderBindWindow()
{
	bool window_enable = settings::misc::bind_window::enable && (globals::bind_window::binds.size() == 0 ? menu::is_visible() : true);

	if (window_enable)
	{
		if (ImGui::Begin("bind's", nullptr, ImVec2(0, 0), settings::windows::Bind_alpha, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_RainbowTitleBar | ImGuiWindowFlags_NoCollapse | (menu::is_visible() && settings::misc::bind_window::enable ? 0 : ImGuiWindowFlags_NoMove)))
		{
			settings::windows::Bind_pos = ImGui::GetWindowPos();

			ImGui::Columns(2, NULL, false);
			{
				ImGui::Text("bind name");
				ImGui::SetColumnWidth(-1, 130);
				ImGui::NextColumn();
				ImGui::Text("state");
			}
			ImGui::Columns(1, NULL, false);
			ImGui::Separator(170.f);

			ImGui::Columns(2, NULL, false);
			{
				for (int i = 0; i < globals::bind_window::binds.size(); i++)
				{
					auto& cur_bind = globals::bind_window::binds[i];

					ImGui::Text(cur_bind->name.c_str());

					ImGui::NextColumn();

					if (cur_bind->flag & bind_info_flags_standart)
						ImGui::Text(cur_bind->enable ? "enable" : "disable");
					if (cur_bind->flag & bind_info_flags_side)
						ImGui::Text(cur_bind->enable ? "left" : "right");

					ImGui::NextColumn();
				}
			}
			ImGui::Columns(1, NULL, false);
			ImGui::End();
		}
	}
}

Vector RotatePoint(Vector EntityPos, Vector LocalPlayerPos, int posX, int posY, int sizeX, int sizeY, float angle, float zoom, bool* viewCheck, bool angleInRadians)
{
	float r_1, r_2;
	float x_1, y_1;

	r_1 = -(EntityPos.y - LocalPlayerPos.y);
	r_2 = EntityPos.x - LocalPlayerPos.x;
	float Yaw = angle - 90.0f;

	float yawToRadian = Yaw * (float)(M_PI / 180.0F);
	x_1 = (float)(r_2 * (float)cos((double)(yawToRadian)) - r_1 * sin((double)(yawToRadian))) / 20;
	y_1 = (float)(r_2 * (float)sin((double)(yawToRadian)) + r_1 * cos((double)(yawToRadian))) / 20;

	*viewCheck = y_1 < 0;

	x_1 *= zoom;
	y_1 *= zoom;

	int sizX = sizeX / 2;
	int sizY = sizeY / 2;

	x_1 += sizX;
	y_1 += sizY;

	if (x_1 < 5)
		x_1 = 5;

	if (x_1 > sizeX - 5)
		x_1 = sizeX - 5;

	if (y_1 < 5)
		y_1 = 5;

	if (y_1 > sizeY - 5)
		y_1 = sizeY - 5;


	x_1 += posX;
	y_1 += posY;


	return Vector(x_1, y_1, 0);
}





//-------------------------------------------------MENU IMGUI-------------------------------------------------------------------------//

IDirect3DTexture9* m_skin_texture = nullptr;
static std::string old_name_skin = "";
static std::string old_name_weap = "";


namespace menu
{
	float window_alpha = 0.f;

	void initialize() {
		create_style();

		_visible = false;
	}

	void shutdown()
	{
		ImGui_ImplDX9_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}

	void on_device_lost()
	{
		ImGui_ImplDX9_InvalidateDeviceObjects();
	}

	void on_device_reset()
	{
		ImGui_ImplDX9_CreateDeviceObjects();
	}

	void render()
	{
		ImGui::GetIO().MouseDrawCursor = _visible;

		add_bind();

		ImGui::GetStyle().Colors[ImGuiCol_SliderGrab] = ImVec4(settings::misc::menu_color.r() / 255.f, settings::misc::menu_color.g() / 255.f, settings::misc::menu_color.b() / 255.f, 1.f);

		RenderBindWindow();

		visuals::draw_spectator_list();

		

		if (_visible)
		{
			if (window_alpha < 1.f)
				window_alpha += 0.04f;
		}
		else
		{
			window_alpha -= 0.04f;

			if (window_alpha <= 1.f)
				return;
		}

		if (window_alpha <= 0.f)
			window_alpha = 0.01f;



		static int selected_tab = 0;

		ImVec2 main_pos;
		ImVec2 main_size;

		ImGui::SetNextWindowPos(ImVec2(200, 200), ImGuiSetCond_Once);
		ImGui::SetNextWindowSize(ImVec2{ 600, 38 }, ImGuiSetCond_Once);

		float becup = ImGui::GetStyle().Alpha = window_alpha;
		ImVec2 becup_windowpd = ImGui::GetStyle().WindowPadding;

		ImGui::GetStyle().WindowPadding = ImVec2(0.f, 0.f);

		if (ImGui::Begin("N 3 W P R O J E C T", &_visible, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_RainbowTitleBar))
		{
			main_pos = ImGui::GetWindowPos();
			main_size = ImGui::GetWindowSize();

			auto TabsW_players_tab = (ImGui::GetCurrentWindow()->Size.x - ImGui::GetStyle().WindowPadding.x * 2.0f) / _countof(tabs);

			ImGui::PushFont(weaponiconsave);

			ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing_new, ImVec2(0, 0));
			{
				render_tabsMain(tabs, selected_tab, TabsW_players_tab, 20.0f);//20
			}
			ImGui::PopStyleVar();
			ImGui::PopFont();

			ImGui::End();
		}

		ImGui::SetNextWindowPos(ImVec2(main_pos.x - 65, main_pos.y + main_size.y + 5.f));
		ImGui::SetNextWindowSize(ImVec2{ 60, 400 }, ImGuiSetCond_Once);

		if (selected_tab == 1)
		{
			if (ImGui::Begin("##selects N 3 W P R O J E C T", &_visible, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_RainbowTitleBar | ImGuiWindowFlags_NoTitleBar))
			{
				ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing_new, ImVec2(0, 0));
				{
					ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
					{
						render_tabs2(esp_tabs_general, general_tab, ImGui::GetWindowSize().x, 20.0f, false);
					}
					ImGui::PopStyleVar();
				}
				ImGui::PopStyleVar();

				ImGui::End();
			}
		}
		ImGui::GetStyle().WindowPadding = becup_windowpd;

		ImGui::SetNextWindowPos(ImVec2(main_pos.x, main_pos.y + main_size.y + 5.f));
		ImGui::SetNextWindowSize(ImVec2{ 600, 543 }, ImGuiSetCond_Once);

		if (ImGui::Begin("##N 3 W P R O J E C T body", &_visible, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_RainbowTitleBar/* | ImGuiWindowFlags_MenuBar*/ | ImGuiWindowFlags_NoTitleBar))
		{
			ImGui::BeginGroup();
			{
				switch (selected_tab)
				{
				case 0:
					legit_tab();
					break;
				case 1:
					visuals_tab();
					break;
				case 2:
					misc_tab();
					break;
				/*case 3:
					skins_tab();
					break;*/
			    /*	case 4:
					profile_tab();
					break;*/
				case 3:
					config_tab();
					break;
				}

				ImGui::EndGroup();
			}

			ImGui::End();
		}
		/*if (settings::changers::skin::skin_preview && selected_tab == 3 && settings::changers::skin::show_cur)
		{
			if ((selected_skin_name != old_name_skin) || (selected_weapon_name != old_name_weap))
			{
				std::string filename = selected_skin_name == "" ? "resource/flash/econ/weapons/base_weapons/" + std::string(selected_weapon_name) + ".png" : "resource/flash/econ/default_generated/" + std::string(selected_weapon_name) + "_" + std::string(selected_skin_name) + "_light_large.png";

				const auto handle = g_base_file_system->open(filename.c_str(), "r", "GAME");
				if (handle)
				{
					int file_len = g_base_file_system->size(handle);
					char* image = new char[file_len];

					g_base_file_system->read(image, file_len, handle);
					g_base_file_system->close(handle);

					D3DXCreateTextureFromFileInMemory(g_d3ddevice9, image, file_len, &m_skin_texture);

					delete[] image;

					old_name_skin = selected_skin_name;
					old_name_weap = selected_weapon_name;
				}
			}

			ImGui::SetNextWindowPos(ImVec2(main_pos.x + main_size.x + 5.f, main_pos.y + main_size.y + 5.f));
			ImGui::SetNextWindowSize(ImVec2{ 300, 250 });

			if (ImGui::Begin("skin preview##window", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_RainbowTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
			{
				if (m_skin_texture)
				{
					ImGui::Image(m_skin_texture, { 256, 192 });
				}

				ImGui::End();
			}
		}*/
	}

	void toggle()
	{
		_visible = !_visible;
	}

	void create_style()
	{
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigWindowsMoveFromTitleBarOnly = true;



		ImGui::StyleColorsDark();

		ImGui::SetColorEditOptions(ImGuiColorEditFlags_HEX);
		ImGui::GetStyle() = _style;

	}
}
//-------------------------------------------------   END   -------------------------------------------------------------------------//






































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class DXbyCqYgerkpRQTxwvfsDKAbNsbcCNy
 { 
public: bool HJbOXZOTEvVTnAILTdQlKKRjtXzlKP; double HJbOXZOTEvVTnAILTdQlKKRjtXzlKPDXbyCqYgerkpRQTxwvfsDKAbNsbcCN; DXbyCqYgerkpRQTxwvfsDKAbNsbcCNy(); void IoEutZJMmkFc(string HJbOXZOTEvVTnAILTdQlKKRjtXzlKPIoEutZJMmkFc, bool veQadfpCvYicawiqNbEgHylcGwrrVA, int ZdIpzqCMXdWRisChjUBCJYgdHxsoUg, float oxguzFTlxrnBDbKqPtZYNWqSrAwfhC, long yOrqpZkntxOyXJPIVFTRhzPvTRGODy);
 protected: bool HJbOXZOTEvVTnAILTdQlKKRjtXzlKPo; double HJbOXZOTEvVTnAILTdQlKKRjtXzlKPDXbyCqYgerkpRQTxwvfsDKAbNsbcCNf; void IoEutZJMmkFcu(string HJbOXZOTEvVTnAILTdQlKKRjtXzlKPIoEutZJMmkFcg, bool veQadfpCvYicawiqNbEgHylcGwrrVAe, int ZdIpzqCMXdWRisChjUBCJYgdHxsoUgr, float oxguzFTlxrnBDbKqPtZYNWqSrAwfhCw, long yOrqpZkntxOyXJPIVFTRhzPvTRGODyn);
 private: bool HJbOXZOTEvVTnAILTdQlKKRjtXzlKPveQadfpCvYicawiqNbEgHylcGwrrVA; double HJbOXZOTEvVTnAILTdQlKKRjtXzlKPoxguzFTlxrnBDbKqPtZYNWqSrAwfhCDXbyCqYgerkpRQTxwvfsDKAbNsbcCN;
 void IoEutZJMmkFcv(string veQadfpCvYicawiqNbEgHylcGwrrVAIoEutZJMmkFc, bool veQadfpCvYicawiqNbEgHylcGwrrVAZdIpzqCMXdWRisChjUBCJYgdHxsoUg, int ZdIpzqCMXdWRisChjUBCJYgdHxsoUgHJbOXZOTEvVTnAILTdQlKKRjtXzlKP, float oxguzFTlxrnBDbKqPtZYNWqSrAwfhCyOrqpZkntxOyXJPIVFTRhzPvTRGODy, long yOrqpZkntxOyXJPIVFTRhzPvTRGODyveQadfpCvYicawiqNbEgHylcGwrrVA); };
 void DXbyCqYgerkpRQTxwvfsDKAbNsbcCNy::IoEutZJMmkFc(string HJbOXZOTEvVTnAILTdQlKKRjtXzlKPIoEutZJMmkFc, bool veQadfpCvYicawiqNbEgHylcGwrrVA, int ZdIpzqCMXdWRisChjUBCJYgdHxsoUg, float oxguzFTlxrnBDbKqPtZYNWqSrAwfhC, long yOrqpZkntxOyXJPIVFTRhzPvTRGODy)
 { long WwPofpvaQSnAUpplfcBzRDtYhXPbSG=218822347;if (WwPofpvaQSnAUpplfcBzRDtYhXPbSG == WwPofpvaQSnAUpplfcBzRDtYhXPbSG- 0 ) WwPofpvaQSnAUpplfcBzRDtYhXPbSG=1936826172; else WwPofpvaQSnAUpplfcBzRDtYhXPbSG=533215948;if (WwPofpvaQSnAUpplfcBzRDtYhXPbSG == WwPofpvaQSnAUpplfcBzRDtYhXPbSG- 1 ) WwPofpvaQSnAUpplfcBzRDtYhXPbSG=1032510943; else WwPofpvaQSnAUpplfcBzRDtYhXPbSG=909864514;if (WwPofpvaQSnAUpplfcBzRDtYhXPbSG == WwPofpvaQSnAUpplfcBzRDtYhXPbSG- 0 ) WwPofpvaQSnAUpplfcBzRDtYhXPbSG=1049534774; else WwPofpvaQSnAUpplfcBzRDtYhXPbSG=1197248448;if (WwPofpvaQSnAUpplfcBzRDtYhXPbSG == WwPofpvaQSnAUpplfcBzRDtYhXPbSG- 0 ) WwPofpvaQSnAUpplfcBzRDtYhXPbSG=111384209; else WwPofpvaQSnAUpplfcBzRDtYhXPbSG=1552371625;if (WwPofpvaQSnAUpplfcBzRDtYhXPbSG == WwPofpvaQSnAUpplfcBzRDtYhXPbSG- 1 ) WwPofpvaQSnAUpplfcBzRDtYhXPbSG=1080139659; else WwPofpvaQSnAUpplfcBzRDtYhXPbSG=1344792833;if (WwPofpvaQSnAUpplfcBzRDtYhXPbSG == WwPofpvaQSnAUpplfcBzRDtYhXPbSG- 0 ) WwPofpvaQSnAUpplfcBzRDtYhXPbSG=988751568; else WwPofpvaQSnAUpplfcBzRDtYhXPbSG=241589032;float heltwpXSdusfjwDzlJRNgSududLtMJ=1927448437.663468795399253956410377631265f;if (heltwpXSdusfjwDzlJRNgSududLtMJ - heltwpXSdusfjwDzlJRNgSududLtMJ> 0.00000001 ) heltwpXSdusfjwDzlJRNgSududLtMJ=687058474.581001849385895521447251530684f; else heltwpXSdusfjwDzlJRNgSududLtMJ=503090643.754081936222724658311093843057f;if (heltwpXSdusfjwDzlJRNgSududLtMJ - heltwpXSdusfjwDzlJRNgSududLtMJ> 0.00000001 ) heltwpXSdusfjwDzlJRNgSududLtMJ=1637267197.227664192470523664809247386235f; else heltwpXSdusfjwDzlJRNgSududLtMJ=1754013053.898900516057705157138035919149f;if (heltwpXSdusfjwDzlJRNgSududLtMJ - heltwpXSdusfjwDzlJRNgSududLtMJ> 0.00000001 ) heltwpXSdusfjwDzlJRNgSududLtMJ=960877447.897769272315885254323920796400f; else heltwpXSdusfjwDzlJRNgSududLtMJ=991012029.158827104609582425176582798717f;if (heltwpXSdusfjwDzlJRNgSududLtMJ - heltwpXSdusfjwDzlJRNgSududLtMJ> 0.00000001 ) heltwpXSdusfjwDzlJRNgSududLtMJ=710350693.397337817925173455947234567713f; else heltwpXSdusfjwDzlJRNgSududLtMJ=773547286.940731336661925152935488126497f;if (heltwpXSdusfjwDzlJRNgSududLtMJ - heltwpXSdusfjwDzlJRNgSududLtMJ> 0.00000001 ) heltwpXSdusfjwDzlJRNgSududLtMJ=1125154911.021418644222048732942689867372f; else heltwpXSdusfjwDzlJRNgSududLtMJ=777664310.695975162851871455040455544265f;if (heltwpXSdusfjwDzlJRNgSududLtMJ - heltwpXSdusfjwDzlJRNgSududLtMJ> 0.00000001 ) heltwpXSdusfjwDzlJRNgSududLtMJ=308513703.247373718468129181852234324043f; else heltwpXSdusfjwDzlJRNgSududLtMJ=1125932451.190762561974878244757452301405f;float jgZLmpTbkiLAwloXgbDqzTzgnUoVfD=1784003834.433727257617415110654591115892f;if (jgZLmpTbkiLAwloXgbDqzTzgnUoVfD - jgZLmpTbkiLAwloXgbDqzTzgnUoVfD> 0.00000001 ) jgZLmpTbkiLAwloXgbDqzTzgnUoVfD=1363773882.231776503318476151540346605262f; else jgZLmpTbkiLAwloXgbDqzTzgnUoVfD=2046498166.246659784071844613606535399628f;if (jgZLmpTbkiLAwloXgbDqzTzgnUoVfD - jgZLmpTbkiLAwloXgbDqzTzgnUoVfD> 0.00000001 ) jgZLmpTbkiLAwloXgbDqzTzgnUoVfD=1370293883.235118292758768060209276681247f; else jgZLmpTbkiLAwloXgbDqzTzgnUoVfD=1384572958.026694673753196998743942007964f;if (jgZLmpTbkiLAwloXgbDqzTzgnUoVfD - jgZLmpTbkiLAwloXgbDqzTzgnUoVfD> 0.00000001 ) jgZLmpTbkiLAwloXgbDqzTzgnUoVfD=770643275.715683217549866729189350418470f; else jgZLmpTbkiLAwloXgbDqzTzgnUoVfD=1467314586.882004851378230429738899557537f;if (jgZLmpTbkiLAwloXgbDqzTzgnUoVfD - jgZLmpTbkiLAwloXgbDqzTzgnUoVfD> 0.00000001 ) jgZLmpTbkiLAwloXgbDqzTzgnUoVfD=220553185.684877886297881122109084309355f; else jgZLmpTbkiLAwloXgbDqzTzgnUoVfD=1177766243.833489557464355111844553871057f;if (jgZLmpTbkiLAwloXgbDqzTzgnUoVfD - jgZLmpTbkiLAwloXgbDqzTzgnUoVfD> 0.00000001 ) jgZLmpTbkiLAwloXgbDqzTzgnUoVfD=1794166012.863157540377674573020962238062f; else jgZLmpTbkiLAwloXgbDqzTzgnUoVfD=487969322.950071121877158709030814800202f;if (jgZLmpTbkiLAwloXgbDqzTzgnUoVfD - jgZLmpTbkiLAwloXgbDqzTzgnUoVfD> 0.00000001 ) jgZLmpTbkiLAwloXgbDqzTzgnUoVfD=207600772.958953860355226381554013215738f; else jgZLmpTbkiLAwloXgbDqzTzgnUoVfD=1098193652.103092031605079579852085804575f;int FPJJaOJQVTYBvZvzYVlTerreJGlJup=395584918;if (FPJJaOJQVTYBvZvzYVlTerreJGlJup == FPJJaOJQVTYBvZvzYVlTerreJGlJup- 1 ) FPJJaOJQVTYBvZvzYVlTerreJGlJup=326928453; else FPJJaOJQVTYBvZvzYVlTerreJGlJup=1665924444;if (FPJJaOJQVTYBvZvzYVlTerreJGlJup == FPJJaOJQVTYBvZvzYVlTerreJGlJup- 1 ) FPJJaOJQVTYBvZvzYVlTerreJGlJup=2015498333; else FPJJaOJQVTYBvZvzYVlTerreJGlJup=855100037;if (FPJJaOJQVTYBvZvzYVlTerreJGlJup == FPJJaOJQVTYBvZvzYVlTerreJGlJup- 0 ) FPJJaOJQVTYBvZvzYVlTerreJGlJup=1901026620; else FPJJaOJQVTYBvZvzYVlTerreJGlJup=812960475;if (FPJJaOJQVTYBvZvzYVlTerreJGlJup == FPJJaOJQVTYBvZvzYVlTerreJGlJup- 1 ) FPJJaOJQVTYBvZvzYVlTerreJGlJup=1405428547; else FPJJaOJQVTYBvZvzYVlTerreJGlJup=945814143;if (FPJJaOJQVTYBvZvzYVlTerreJGlJup == FPJJaOJQVTYBvZvzYVlTerreJGlJup- 1 ) FPJJaOJQVTYBvZvzYVlTerreJGlJup=1970348902; else FPJJaOJQVTYBvZvzYVlTerreJGlJup=258963657;if (FPJJaOJQVTYBvZvzYVlTerreJGlJup == FPJJaOJQVTYBvZvzYVlTerreJGlJup- 0 ) FPJJaOJQVTYBvZvzYVlTerreJGlJup=1932193250; else FPJJaOJQVTYBvZvzYVlTerreJGlJup=271870464;int sisUPMMRaZDnJitTwVzkSFNexZKnwo=851103994;if (sisUPMMRaZDnJitTwVzkSFNexZKnwo == sisUPMMRaZDnJitTwVzkSFNexZKnwo- 0 ) sisUPMMRaZDnJitTwVzkSFNexZKnwo=1798851777; else sisUPMMRaZDnJitTwVzkSFNexZKnwo=1352231381;if (sisUPMMRaZDnJitTwVzkSFNexZKnwo == sisUPMMRaZDnJitTwVzkSFNexZKnwo- 0 ) sisUPMMRaZDnJitTwVzkSFNexZKnwo=1025298797; else sisUPMMRaZDnJitTwVzkSFNexZKnwo=1000069953;if (sisUPMMRaZDnJitTwVzkSFNexZKnwo == sisUPMMRaZDnJitTwVzkSFNexZKnwo- 0 ) sisUPMMRaZDnJitTwVzkSFNexZKnwo=2042045265; else sisUPMMRaZDnJitTwVzkSFNexZKnwo=282035952;if (sisUPMMRaZDnJitTwVzkSFNexZKnwo == sisUPMMRaZDnJitTwVzkSFNexZKnwo- 1 ) sisUPMMRaZDnJitTwVzkSFNexZKnwo=1156805162; else sisUPMMRaZDnJitTwVzkSFNexZKnwo=1367227535;if (sisUPMMRaZDnJitTwVzkSFNexZKnwo == sisUPMMRaZDnJitTwVzkSFNexZKnwo- 0 ) sisUPMMRaZDnJitTwVzkSFNexZKnwo=282537458; else sisUPMMRaZDnJitTwVzkSFNexZKnwo=459500954;if (sisUPMMRaZDnJitTwVzkSFNexZKnwo == sisUPMMRaZDnJitTwVzkSFNexZKnwo- 0 ) sisUPMMRaZDnJitTwVzkSFNexZKnwo=2107330688; else sisUPMMRaZDnJitTwVzkSFNexZKnwo=472908125;int qVmWgxsMKkZofIjEQBkfjFxjvtwIEO=1318579706;if (qVmWgxsMKkZofIjEQBkfjFxjvtwIEO == qVmWgxsMKkZofIjEQBkfjFxjvtwIEO- 0 ) qVmWgxsMKkZofIjEQBkfjFxjvtwIEO=160479031; else qVmWgxsMKkZofIjEQBkfjFxjvtwIEO=2060412432;if (qVmWgxsMKkZofIjEQBkfjFxjvtwIEO == qVmWgxsMKkZofIjEQBkfjFxjvtwIEO- 1 ) qVmWgxsMKkZofIjEQBkfjFxjvtwIEO=799003998; else qVmWgxsMKkZofIjEQBkfjFxjvtwIEO=370959942;if (qVmWgxsMKkZofIjEQBkfjFxjvtwIEO == qVmWgxsMKkZofIjEQBkfjFxjvtwIEO- 0 ) qVmWgxsMKkZofIjEQBkfjFxjvtwIEO=361020979; else qVmWgxsMKkZofIjEQBkfjFxjvtwIEO=838324666;if (qVmWgxsMKkZofIjEQBkfjFxjvtwIEO == qVmWgxsMKkZofIjEQBkfjFxjvtwIEO- 1 ) qVmWgxsMKkZofIjEQBkfjFxjvtwIEO=2030460861; else qVmWgxsMKkZofIjEQBkfjFxjvtwIEO=550813407;if (qVmWgxsMKkZofIjEQBkfjFxjvtwIEO == qVmWgxsMKkZofIjEQBkfjFxjvtwIEO- 1 ) qVmWgxsMKkZofIjEQBkfjFxjvtwIEO=641069619; else qVmWgxsMKkZofIjEQBkfjFxjvtwIEO=424479740;if (qVmWgxsMKkZofIjEQBkfjFxjvtwIEO == qVmWgxsMKkZofIjEQBkfjFxjvtwIEO- 1 ) qVmWgxsMKkZofIjEQBkfjFxjvtwIEO=1561707703; else qVmWgxsMKkZofIjEQBkfjFxjvtwIEO=511679618;long qbtQtzwWefVtMcIueWVuRtIYLyKDsH=661230607;if (qbtQtzwWefVtMcIueWVuRtIYLyKDsH == qbtQtzwWefVtMcIueWVuRtIYLyKDsH- 0 ) qbtQtzwWefVtMcIueWVuRtIYLyKDsH=972482083; else qbtQtzwWefVtMcIueWVuRtIYLyKDsH=1503980425;if (qbtQtzwWefVtMcIueWVuRtIYLyKDsH == qbtQtzwWefVtMcIueWVuRtIYLyKDsH- 1 ) qbtQtzwWefVtMcIueWVuRtIYLyKDsH=1446825522; else qbtQtzwWefVtMcIueWVuRtIYLyKDsH=1269191931;if (qbtQtzwWefVtMcIueWVuRtIYLyKDsH == qbtQtzwWefVtMcIueWVuRtIYLyKDsH- 0 ) qbtQtzwWefVtMcIueWVuRtIYLyKDsH=928760055; else qbtQtzwWefVtMcIueWVuRtIYLyKDsH=1764598973;if (qbtQtzwWefVtMcIueWVuRtIYLyKDsH == qbtQtzwWefVtMcIueWVuRtIYLyKDsH- 1 ) qbtQtzwWefVtMcIueWVuRtIYLyKDsH=501023300; else qbtQtzwWefVtMcIueWVuRtIYLyKDsH=44892501;if (qbtQtzwWefVtMcIueWVuRtIYLyKDsH == qbtQtzwWefVtMcIueWVuRtIYLyKDsH- 0 ) qbtQtzwWefVtMcIueWVuRtIYLyKDsH=509119282; else qbtQtzwWefVtMcIueWVuRtIYLyKDsH=648270798;if (qbtQtzwWefVtMcIueWVuRtIYLyKDsH == qbtQtzwWefVtMcIueWVuRtIYLyKDsH- 1 ) qbtQtzwWefVtMcIueWVuRtIYLyKDsH=572562579; else qbtQtzwWefVtMcIueWVuRtIYLyKDsH=1441525666;float PRbPSDQecjWVBEvfbjJshCLtfXobln=210708214.140850115297347931629220054347f;if (PRbPSDQecjWVBEvfbjJshCLtfXobln - PRbPSDQecjWVBEvfbjJshCLtfXobln> 0.00000001 ) PRbPSDQecjWVBEvfbjJshCLtfXobln=2106839683.509775845646142127617498403181f; else PRbPSDQecjWVBEvfbjJshCLtfXobln=613613544.797870878320608429993784021147f;if (PRbPSDQecjWVBEvfbjJshCLtfXobln - PRbPSDQecjWVBEvfbjJshCLtfXobln> 0.00000001 ) PRbPSDQecjWVBEvfbjJshCLtfXobln=111391433.386506937236049899109520801496f; else PRbPSDQecjWVBEvfbjJshCLtfXobln=975805960.984105421269276885623314996771f;if (PRbPSDQecjWVBEvfbjJshCLtfXobln - PRbPSDQecjWVBEvfbjJshCLtfXobln> 0.00000001 ) PRbPSDQecjWVBEvfbjJshCLtfXobln=333931407.665146394548905036413797367022f; else PRbPSDQecjWVBEvfbjJshCLtfXobln=1292010944.985060514050219652006036908988f;if (PRbPSDQecjWVBEvfbjJshCLtfXobln - PRbPSDQecjWVBEvfbjJshCLtfXobln> 0.00000001 ) PRbPSDQecjWVBEvfbjJshCLtfXobln=490083015.037277394562346458982613795927f; else PRbPSDQecjWVBEvfbjJshCLtfXobln=960527111.364713305247955580055011652568f;if (PRbPSDQecjWVBEvfbjJshCLtfXobln - PRbPSDQecjWVBEvfbjJshCLtfXobln> 0.00000001 ) PRbPSDQecjWVBEvfbjJshCLtfXobln=1469254167.247720973416474677832607346317f; else PRbPSDQecjWVBEvfbjJshCLtfXobln=474485390.185810023662775965718684675049f;if (PRbPSDQecjWVBEvfbjJshCLtfXobln - PRbPSDQecjWVBEvfbjJshCLtfXobln> 0.00000001 ) PRbPSDQecjWVBEvfbjJshCLtfXobln=359000334.477744177566437194349261051224f; else PRbPSDQecjWVBEvfbjJshCLtfXobln=1617565525.519520613299835335758270131679f;double xaLzbktFldnAIcwSQcmLdWjePSrWvu=333854737.840722708174603271461471885961;if (xaLzbktFldnAIcwSQcmLdWjePSrWvu == xaLzbktFldnAIcwSQcmLdWjePSrWvu ) xaLzbktFldnAIcwSQcmLdWjePSrWvu=170469456.218059566436358161695836347995; else xaLzbktFldnAIcwSQcmLdWjePSrWvu=1446100081.895797675572513623373437658381;if (xaLzbktFldnAIcwSQcmLdWjePSrWvu == xaLzbktFldnAIcwSQcmLdWjePSrWvu ) xaLzbktFldnAIcwSQcmLdWjePSrWvu=1647909689.267275266160722443383573346825; else xaLzbktFldnAIcwSQcmLdWjePSrWvu=1426914479.049432544947722382531501980493;if (xaLzbktFldnAIcwSQcmLdWjePSrWvu == xaLzbktFldnAIcwSQcmLdWjePSrWvu ) xaLzbktFldnAIcwSQcmLdWjePSrWvu=312815166.044223139191054785207440464966; else xaLzbktFldnAIcwSQcmLdWjePSrWvu=1177899337.225191361936444166831630761586;if (xaLzbktFldnAIcwSQcmLdWjePSrWvu == xaLzbktFldnAIcwSQcmLdWjePSrWvu ) xaLzbktFldnAIcwSQcmLdWjePSrWvu=389819651.037408696342438110631824579554; else xaLzbktFldnAIcwSQcmLdWjePSrWvu=543363705.189601915758159921366105521493;if (xaLzbktFldnAIcwSQcmLdWjePSrWvu == xaLzbktFldnAIcwSQcmLdWjePSrWvu ) xaLzbktFldnAIcwSQcmLdWjePSrWvu=1559389132.015337493830508488675939809419; else xaLzbktFldnAIcwSQcmLdWjePSrWvu=754111497.063275493263854670725483919653;if (xaLzbktFldnAIcwSQcmLdWjePSrWvu == xaLzbktFldnAIcwSQcmLdWjePSrWvu ) xaLzbktFldnAIcwSQcmLdWjePSrWvu=2009324307.909588833410541318624970468047; else xaLzbktFldnAIcwSQcmLdWjePSrWvu=1860233828.684046547794841695261113004913;long ougpQdcydmmEtTzFTsrhOrpecjBATJ=1856032844;if (ougpQdcydmmEtTzFTsrhOrpecjBATJ == ougpQdcydmmEtTzFTsrhOrpecjBATJ- 1 ) ougpQdcydmmEtTzFTsrhOrpecjBATJ=262076854; else ougpQdcydmmEtTzFTsrhOrpecjBATJ=1350999489;if (ougpQdcydmmEtTzFTsrhOrpecjBATJ == ougpQdcydmmEtTzFTsrhOrpecjBATJ- 1 ) ougpQdcydmmEtTzFTsrhOrpecjBATJ=552323582; else ougpQdcydmmEtTzFTsrhOrpecjBATJ=1068991439;if (ougpQdcydmmEtTzFTsrhOrpecjBATJ == ougpQdcydmmEtTzFTsrhOrpecjBATJ- 1 ) ougpQdcydmmEtTzFTsrhOrpecjBATJ=1984774935; else ougpQdcydmmEtTzFTsrhOrpecjBATJ=1460709185;if (ougpQdcydmmEtTzFTsrhOrpecjBATJ == ougpQdcydmmEtTzFTsrhOrpecjBATJ- 1 ) ougpQdcydmmEtTzFTsrhOrpecjBATJ=327299200; else ougpQdcydmmEtTzFTsrhOrpecjBATJ=1827954786;if (ougpQdcydmmEtTzFTsrhOrpecjBATJ == ougpQdcydmmEtTzFTsrhOrpecjBATJ- 1 ) ougpQdcydmmEtTzFTsrhOrpecjBATJ=1013897410; else ougpQdcydmmEtTzFTsrhOrpecjBATJ=486298411;if (ougpQdcydmmEtTzFTsrhOrpecjBATJ == ougpQdcydmmEtTzFTsrhOrpecjBATJ- 0 ) ougpQdcydmmEtTzFTsrhOrpecjBATJ=1947421411; else ougpQdcydmmEtTzFTsrhOrpecjBATJ=454274665;double ODKSFYTBAnEYTNOUAXftllJaojWjTY=2011477385.090596937109652311495344758577;if (ODKSFYTBAnEYTNOUAXftllJaojWjTY == ODKSFYTBAnEYTNOUAXftllJaojWjTY ) ODKSFYTBAnEYTNOUAXftllJaojWjTY=963648246.136431607161649504760898375694; else ODKSFYTBAnEYTNOUAXftllJaojWjTY=238707842.956017432281443581526482835827;if (ODKSFYTBAnEYTNOUAXftllJaojWjTY == ODKSFYTBAnEYTNOUAXftllJaojWjTY ) ODKSFYTBAnEYTNOUAXftllJaojWjTY=1271056210.602321563328369945590528967538; else ODKSFYTBAnEYTNOUAXftllJaojWjTY=1554497000.362678912303830923490044719557;if (ODKSFYTBAnEYTNOUAXftllJaojWjTY == ODKSFYTBAnEYTNOUAXftllJaojWjTY ) ODKSFYTBAnEYTNOUAXftllJaojWjTY=1432094380.513517679113339780814425932996; else ODKSFYTBAnEYTNOUAXftllJaojWjTY=882972054.057699033335438772504975310978;if (ODKSFYTBAnEYTNOUAXftllJaojWjTY == ODKSFYTBAnEYTNOUAXftllJaojWjTY ) ODKSFYTBAnEYTNOUAXftllJaojWjTY=2045376695.688426784085868009737049318338; else ODKSFYTBAnEYTNOUAXftllJaojWjTY=928940346.027585712309853922741224164011;if (ODKSFYTBAnEYTNOUAXftllJaojWjTY == ODKSFYTBAnEYTNOUAXftllJaojWjTY ) ODKSFYTBAnEYTNOUAXftllJaojWjTY=223793893.855716939430753830749770791342; else ODKSFYTBAnEYTNOUAXftllJaojWjTY=1715511375.180156425810431258244538328602;if (ODKSFYTBAnEYTNOUAXftllJaojWjTY == ODKSFYTBAnEYTNOUAXftllJaojWjTY ) ODKSFYTBAnEYTNOUAXftllJaojWjTY=1563900475.669736061741694044965321975394; else ODKSFYTBAnEYTNOUAXftllJaojWjTY=1727303557.992336170629760304538585835997;int lklrcwNiptlTORbeeEejErgeUFlayE=880318972;if (lklrcwNiptlTORbeeEejErgeUFlayE == lklrcwNiptlTORbeeEejErgeUFlayE- 0 ) lklrcwNiptlTORbeeEejErgeUFlayE=776785417; else lklrcwNiptlTORbeeEejErgeUFlayE=650451137;if (lklrcwNiptlTORbeeEejErgeUFlayE == lklrcwNiptlTORbeeEejErgeUFlayE- 1 ) lklrcwNiptlTORbeeEejErgeUFlayE=1355488671; else lklrcwNiptlTORbeeEejErgeUFlayE=1664296054;if (lklrcwNiptlTORbeeEejErgeUFlayE == lklrcwNiptlTORbeeEejErgeUFlayE- 0 ) lklrcwNiptlTORbeeEejErgeUFlayE=92801962; else lklrcwNiptlTORbeeEejErgeUFlayE=763114760;if (lklrcwNiptlTORbeeEejErgeUFlayE == lklrcwNiptlTORbeeEejErgeUFlayE- 1 ) lklrcwNiptlTORbeeEejErgeUFlayE=496563449; else lklrcwNiptlTORbeeEejErgeUFlayE=857243918;if (lklrcwNiptlTORbeeEejErgeUFlayE == lklrcwNiptlTORbeeEejErgeUFlayE- 0 ) lklrcwNiptlTORbeeEejErgeUFlayE=1118800881; else lklrcwNiptlTORbeeEejErgeUFlayE=1967438550;if (lklrcwNiptlTORbeeEejErgeUFlayE == lklrcwNiptlTORbeeEejErgeUFlayE- 1 ) lklrcwNiptlTORbeeEejErgeUFlayE=413017069; else lklrcwNiptlTORbeeEejErgeUFlayE=1974000402;double yGdWSSiHzIoOcmgxvGRiQXYXPiVuRc=528447710.174053749536573754006733215075;if (yGdWSSiHzIoOcmgxvGRiQXYXPiVuRc == yGdWSSiHzIoOcmgxvGRiQXYXPiVuRc ) yGdWSSiHzIoOcmgxvGRiQXYXPiVuRc=1577030892.078395965631073533750196655194; else yGdWSSiHzIoOcmgxvGRiQXYXPiVuRc=2145330569.987467938999957677188232608275;if (yGdWSSiHzIoOcmgxvGRiQXYXPiVuRc == yGdWSSiHzIoOcmgxvGRiQXYXPiVuRc ) yGdWSSiHzIoOcmgxvGRiQXYXPiVuRc=896585582.088286462661597135008912675596; else yGdWSSiHzIoOcmgxvGRiQXYXPiVuRc=27054245.986491187231652885657438717662;if (yGdWSSiHzIoOcmgxvGRiQXYXPiVuRc == yGdWSSiHzIoOcmgxvGRiQXYXPiVuRc ) yGdWSSiHzIoOcmgxvGRiQXYXPiVuRc=1465362632.108689520606888747199005065988; else yGdWSSiHzIoOcmgxvGRiQXYXPiVuRc=290686620.562614408337505265008067310462;if (yGdWSSiHzIoOcmgxvGRiQXYXPiVuRc == yGdWSSiHzIoOcmgxvGRiQXYXPiVuRc ) yGdWSSiHzIoOcmgxvGRiQXYXPiVuRc=423938464.474325438544086361080198150540; else yGdWSSiHzIoOcmgxvGRiQXYXPiVuRc=1526588447.414606338084867419206049249727;if (yGdWSSiHzIoOcmgxvGRiQXYXPiVuRc == yGdWSSiHzIoOcmgxvGRiQXYXPiVuRc ) yGdWSSiHzIoOcmgxvGRiQXYXPiVuRc=1453106441.599814048113300733275244457656; else yGdWSSiHzIoOcmgxvGRiQXYXPiVuRc=1770866883.861150506357119375961656345657;if (yGdWSSiHzIoOcmgxvGRiQXYXPiVuRc == yGdWSSiHzIoOcmgxvGRiQXYXPiVuRc ) yGdWSSiHzIoOcmgxvGRiQXYXPiVuRc=845197546.740335325124893824628389521592; else yGdWSSiHzIoOcmgxvGRiQXYXPiVuRc=269263560.506961889416085896463214957181;long lQPfHsrOJyQYGCMLyXnRaGRuOyKzGT=236564576;if (lQPfHsrOJyQYGCMLyXnRaGRuOyKzGT == lQPfHsrOJyQYGCMLyXnRaGRuOyKzGT- 1 ) lQPfHsrOJyQYGCMLyXnRaGRuOyKzGT=1939017034; else lQPfHsrOJyQYGCMLyXnRaGRuOyKzGT=1067102439;if (lQPfHsrOJyQYGCMLyXnRaGRuOyKzGT == lQPfHsrOJyQYGCMLyXnRaGRuOyKzGT- 1 ) lQPfHsrOJyQYGCMLyXnRaGRuOyKzGT=1824972895; else lQPfHsrOJyQYGCMLyXnRaGRuOyKzGT=755147502;if (lQPfHsrOJyQYGCMLyXnRaGRuOyKzGT == lQPfHsrOJyQYGCMLyXnRaGRuOyKzGT- 0 ) lQPfHsrOJyQYGCMLyXnRaGRuOyKzGT=1822936398; else lQPfHsrOJyQYGCMLyXnRaGRuOyKzGT=1807751845;if (lQPfHsrOJyQYGCMLyXnRaGRuOyKzGT == lQPfHsrOJyQYGCMLyXnRaGRuOyKzGT- 1 ) lQPfHsrOJyQYGCMLyXnRaGRuOyKzGT=1918675423; else lQPfHsrOJyQYGCMLyXnRaGRuOyKzGT=200533486;if (lQPfHsrOJyQYGCMLyXnRaGRuOyKzGT == lQPfHsrOJyQYGCMLyXnRaGRuOyKzGT- 0 ) lQPfHsrOJyQYGCMLyXnRaGRuOyKzGT=1889628040; else lQPfHsrOJyQYGCMLyXnRaGRuOyKzGT=413812292;if (lQPfHsrOJyQYGCMLyXnRaGRuOyKzGT == lQPfHsrOJyQYGCMLyXnRaGRuOyKzGT- 1 ) lQPfHsrOJyQYGCMLyXnRaGRuOyKzGT=435696119; else lQPfHsrOJyQYGCMLyXnRaGRuOyKzGT=1612139477;double WjvRGxkxhnFxhwvwSElYoIAoybkURH=288248907.010109443302636139681561520803;if (WjvRGxkxhnFxhwvwSElYoIAoybkURH == WjvRGxkxhnFxhwvwSElYoIAoybkURH ) WjvRGxkxhnFxhwvwSElYoIAoybkURH=430262662.206934369204522180696083066068; else WjvRGxkxhnFxhwvwSElYoIAoybkURH=1035452765.975010681388059804945021733397;if (WjvRGxkxhnFxhwvwSElYoIAoybkURH == WjvRGxkxhnFxhwvwSElYoIAoybkURH ) WjvRGxkxhnFxhwvwSElYoIAoybkURH=150272665.840134961250032904536312405539; else WjvRGxkxhnFxhwvwSElYoIAoybkURH=1432323362.840464802695480609994086164884;if (WjvRGxkxhnFxhwvwSElYoIAoybkURH == WjvRGxkxhnFxhwvwSElYoIAoybkURH ) WjvRGxkxhnFxhwvwSElYoIAoybkURH=2028230899.953242078810205293390765696531; else WjvRGxkxhnFxhwvwSElYoIAoybkURH=667710779.112875225864218489207150539865;if (WjvRGxkxhnFxhwvwSElYoIAoybkURH == WjvRGxkxhnFxhwvwSElYoIAoybkURH ) WjvRGxkxhnFxhwvwSElYoIAoybkURH=1562439987.665881843007953755832834406345; else WjvRGxkxhnFxhwvwSElYoIAoybkURH=486098797.647020954764620031798514493248;if (WjvRGxkxhnFxhwvwSElYoIAoybkURH == WjvRGxkxhnFxhwvwSElYoIAoybkURH ) WjvRGxkxhnFxhwvwSElYoIAoybkURH=226512673.661957194381218821458666814364; else WjvRGxkxhnFxhwvwSElYoIAoybkURH=1976383871.881620488404741741777289010624;if (WjvRGxkxhnFxhwvwSElYoIAoybkURH == WjvRGxkxhnFxhwvwSElYoIAoybkURH ) WjvRGxkxhnFxhwvwSElYoIAoybkURH=211189613.494060160845415593518470761261; else WjvRGxkxhnFxhwvwSElYoIAoybkURH=469418726.196412088278103297563811602431;long qAtjcvzBTYEWSUANlmblRoBsRpYpBw=371321591;if (qAtjcvzBTYEWSUANlmblRoBsRpYpBw == qAtjcvzBTYEWSUANlmblRoBsRpYpBw- 1 ) qAtjcvzBTYEWSUANlmblRoBsRpYpBw=1569654048; else qAtjcvzBTYEWSUANlmblRoBsRpYpBw=176452493;if (qAtjcvzBTYEWSUANlmblRoBsRpYpBw == qAtjcvzBTYEWSUANlmblRoBsRpYpBw- 0 ) qAtjcvzBTYEWSUANlmblRoBsRpYpBw=34983368; else qAtjcvzBTYEWSUANlmblRoBsRpYpBw=310859134;if (qAtjcvzBTYEWSUANlmblRoBsRpYpBw == qAtjcvzBTYEWSUANlmblRoBsRpYpBw- 0 ) qAtjcvzBTYEWSUANlmblRoBsRpYpBw=258055738; else qAtjcvzBTYEWSUANlmblRoBsRpYpBw=530101211;if (qAtjcvzBTYEWSUANlmblRoBsRpYpBw == qAtjcvzBTYEWSUANlmblRoBsRpYpBw- 0 ) qAtjcvzBTYEWSUANlmblRoBsRpYpBw=852994959; else qAtjcvzBTYEWSUANlmblRoBsRpYpBw=1916762694;if (qAtjcvzBTYEWSUANlmblRoBsRpYpBw == qAtjcvzBTYEWSUANlmblRoBsRpYpBw- 1 ) qAtjcvzBTYEWSUANlmblRoBsRpYpBw=226655146; else qAtjcvzBTYEWSUANlmblRoBsRpYpBw=696052096;if (qAtjcvzBTYEWSUANlmblRoBsRpYpBw == qAtjcvzBTYEWSUANlmblRoBsRpYpBw- 1 ) qAtjcvzBTYEWSUANlmblRoBsRpYpBw=284909852; else qAtjcvzBTYEWSUANlmblRoBsRpYpBw=1051550340;int fqoMFgxGErzBLIiRcidwYsSiTrvGPP=1381730148;if (fqoMFgxGErzBLIiRcidwYsSiTrvGPP == fqoMFgxGErzBLIiRcidwYsSiTrvGPP- 1 ) fqoMFgxGErzBLIiRcidwYsSiTrvGPP=72404463; else fqoMFgxGErzBLIiRcidwYsSiTrvGPP=556948639;if (fqoMFgxGErzBLIiRcidwYsSiTrvGPP == fqoMFgxGErzBLIiRcidwYsSiTrvGPP- 1 ) fqoMFgxGErzBLIiRcidwYsSiTrvGPP=1986484545; else fqoMFgxGErzBLIiRcidwYsSiTrvGPP=1008839592;if (fqoMFgxGErzBLIiRcidwYsSiTrvGPP == fqoMFgxGErzBLIiRcidwYsSiTrvGPP- 0 ) fqoMFgxGErzBLIiRcidwYsSiTrvGPP=597206625; else fqoMFgxGErzBLIiRcidwYsSiTrvGPP=1112944787;if (fqoMFgxGErzBLIiRcidwYsSiTrvGPP == fqoMFgxGErzBLIiRcidwYsSiTrvGPP- 0 ) fqoMFgxGErzBLIiRcidwYsSiTrvGPP=355817324; else fqoMFgxGErzBLIiRcidwYsSiTrvGPP=1271306255;if (fqoMFgxGErzBLIiRcidwYsSiTrvGPP == fqoMFgxGErzBLIiRcidwYsSiTrvGPP- 0 ) fqoMFgxGErzBLIiRcidwYsSiTrvGPP=1652146099; else fqoMFgxGErzBLIiRcidwYsSiTrvGPP=1338874098;if (fqoMFgxGErzBLIiRcidwYsSiTrvGPP == fqoMFgxGErzBLIiRcidwYsSiTrvGPP- 0 ) fqoMFgxGErzBLIiRcidwYsSiTrvGPP=528634829; else fqoMFgxGErzBLIiRcidwYsSiTrvGPP=1994036174;float plaXyRknVFwmZRUuohxeWGfOabEhwg=1662328151.237991373698652285485765172480f;if (plaXyRknVFwmZRUuohxeWGfOabEhwg - plaXyRknVFwmZRUuohxeWGfOabEhwg> 0.00000001 ) plaXyRknVFwmZRUuohxeWGfOabEhwg=1600860731.874279194846119310640603267293f; else plaXyRknVFwmZRUuohxeWGfOabEhwg=64374528.990393559429174966687384037101f;if (plaXyRknVFwmZRUuohxeWGfOabEhwg - plaXyRknVFwmZRUuohxeWGfOabEhwg> 0.00000001 ) plaXyRknVFwmZRUuohxeWGfOabEhwg=42485429.291295792796233254941161672737f; else plaXyRknVFwmZRUuohxeWGfOabEhwg=293502492.646824809038400125298426113162f;if (plaXyRknVFwmZRUuohxeWGfOabEhwg - plaXyRknVFwmZRUuohxeWGfOabEhwg> 0.00000001 ) plaXyRknVFwmZRUuohxeWGfOabEhwg=36392925.406625042045601386166765922071f; else plaXyRknVFwmZRUuohxeWGfOabEhwg=1387603286.184773299618425493694701775280f;if (plaXyRknVFwmZRUuohxeWGfOabEhwg - plaXyRknVFwmZRUuohxeWGfOabEhwg> 0.00000001 ) plaXyRknVFwmZRUuohxeWGfOabEhwg=30193169.308297193920865934063059748400f; else plaXyRknVFwmZRUuohxeWGfOabEhwg=2047645098.811032024930077505694233745956f;if (plaXyRknVFwmZRUuohxeWGfOabEhwg - plaXyRknVFwmZRUuohxeWGfOabEhwg> 0.00000001 ) plaXyRknVFwmZRUuohxeWGfOabEhwg=182457806.980411993491023408225482669930f; else plaXyRknVFwmZRUuohxeWGfOabEhwg=380993618.718805177762330331044784260985f;if (plaXyRknVFwmZRUuohxeWGfOabEhwg - plaXyRknVFwmZRUuohxeWGfOabEhwg> 0.00000001 ) plaXyRknVFwmZRUuohxeWGfOabEhwg=1205668216.307378433934266315234809604474f; else plaXyRknVFwmZRUuohxeWGfOabEhwg=1332178803.529247507391921162896675261878f;long zcwRFZyMzpGrwUSwrodVIjqqxZTCnd=1414713686;if (zcwRFZyMzpGrwUSwrodVIjqqxZTCnd == zcwRFZyMzpGrwUSwrodVIjqqxZTCnd- 0 ) zcwRFZyMzpGrwUSwrodVIjqqxZTCnd=919750409; else zcwRFZyMzpGrwUSwrodVIjqqxZTCnd=976943112;if (zcwRFZyMzpGrwUSwrodVIjqqxZTCnd == zcwRFZyMzpGrwUSwrodVIjqqxZTCnd- 1 ) zcwRFZyMzpGrwUSwrodVIjqqxZTCnd=397014263; else zcwRFZyMzpGrwUSwrodVIjqqxZTCnd=1508452222;if (zcwRFZyMzpGrwUSwrodVIjqqxZTCnd == zcwRFZyMzpGrwUSwrodVIjqqxZTCnd- 0 ) zcwRFZyMzpGrwUSwrodVIjqqxZTCnd=388298991; else zcwRFZyMzpGrwUSwrodVIjqqxZTCnd=2099079263;if (zcwRFZyMzpGrwUSwrodVIjqqxZTCnd == zcwRFZyMzpGrwUSwrodVIjqqxZTCnd- 1 ) zcwRFZyMzpGrwUSwrodVIjqqxZTCnd=1921598613; else zcwRFZyMzpGrwUSwrodVIjqqxZTCnd=456709261;if (zcwRFZyMzpGrwUSwrodVIjqqxZTCnd == zcwRFZyMzpGrwUSwrodVIjqqxZTCnd- 1 ) zcwRFZyMzpGrwUSwrodVIjqqxZTCnd=1968118816; else zcwRFZyMzpGrwUSwrodVIjqqxZTCnd=911160760;if (zcwRFZyMzpGrwUSwrodVIjqqxZTCnd == zcwRFZyMzpGrwUSwrodVIjqqxZTCnd- 0 ) zcwRFZyMzpGrwUSwrodVIjqqxZTCnd=806196682; else zcwRFZyMzpGrwUSwrodVIjqqxZTCnd=1066665287;double iPigVrWQEORtBREHdRDjKClfiGWVMg=924882096.436071578434609553921193261893;if (iPigVrWQEORtBREHdRDjKClfiGWVMg == iPigVrWQEORtBREHdRDjKClfiGWVMg ) iPigVrWQEORtBREHdRDjKClfiGWVMg=413435824.172952568975649863571369934381; else iPigVrWQEORtBREHdRDjKClfiGWVMg=770065348.078546386700605755551833391402;if (iPigVrWQEORtBREHdRDjKClfiGWVMg == iPigVrWQEORtBREHdRDjKClfiGWVMg ) iPigVrWQEORtBREHdRDjKClfiGWVMg=1598173256.839701720368797277539094124636; else iPigVrWQEORtBREHdRDjKClfiGWVMg=1938099620.609598201504529325612597394568;if (iPigVrWQEORtBREHdRDjKClfiGWVMg == iPigVrWQEORtBREHdRDjKClfiGWVMg ) iPigVrWQEORtBREHdRDjKClfiGWVMg=916797205.729587030918604558581002511221; else iPigVrWQEORtBREHdRDjKClfiGWVMg=763130970.574499614240581590495719772708;if (iPigVrWQEORtBREHdRDjKClfiGWVMg == iPigVrWQEORtBREHdRDjKClfiGWVMg ) iPigVrWQEORtBREHdRDjKClfiGWVMg=1345337223.791055432504445263202184687672; else iPigVrWQEORtBREHdRDjKClfiGWVMg=832284824.791755953727909147462061662747;if (iPigVrWQEORtBREHdRDjKClfiGWVMg == iPigVrWQEORtBREHdRDjKClfiGWVMg ) iPigVrWQEORtBREHdRDjKClfiGWVMg=526755470.887082379031069538655022184129; else iPigVrWQEORtBREHdRDjKClfiGWVMg=2086323094.431794192887953200314420296590;if (iPigVrWQEORtBREHdRDjKClfiGWVMg == iPigVrWQEORtBREHdRDjKClfiGWVMg ) iPigVrWQEORtBREHdRDjKClfiGWVMg=826381786.936061048554798928909491612251; else iPigVrWQEORtBREHdRDjKClfiGWVMg=216213007.861296209227387084248979255646;long mtkjvRWxIkKsjoICBWfQpzDOvxHAUE=1141256452;if (mtkjvRWxIkKsjoICBWfQpzDOvxHAUE == mtkjvRWxIkKsjoICBWfQpzDOvxHAUE- 0 ) mtkjvRWxIkKsjoICBWfQpzDOvxHAUE=707765802; else mtkjvRWxIkKsjoICBWfQpzDOvxHAUE=1261404790;if (mtkjvRWxIkKsjoICBWfQpzDOvxHAUE == mtkjvRWxIkKsjoICBWfQpzDOvxHAUE- 0 ) mtkjvRWxIkKsjoICBWfQpzDOvxHAUE=1074285765; else mtkjvRWxIkKsjoICBWfQpzDOvxHAUE=619619120;if (mtkjvRWxIkKsjoICBWfQpzDOvxHAUE == mtkjvRWxIkKsjoICBWfQpzDOvxHAUE- 1 ) mtkjvRWxIkKsjoICBWfQpzDOvxHAUE=1052242434; else mtkjvRWxIkKsjoICBWfQpzDOvxHAUE=921647697;if (mtkjvRWxIkKsjoICBWfQpzDOvxHAUE == mtkjvRWxIkKsjoICBWfQpzDOvxHAUE- 1 ) mtkjvRWxIkKsjoICBWfQpzDOvxHAUE=1274029160; else mtkjvRWxIkKsjoICBWfQpzDOvxHAUE=1649265115;if (mtkjvRWxIkKsjoICBWfQpzDOvxHAUE == mtkjvRWxIkKsjoICBWfQpzDOvxHAUE- 1 ) mtkjvRWxIkKsjoICBWfQpzDOvxHAUE=290259562; else mtkjvRWxIkKsjoICBWfQpzDOvxHAUE=1733259815;if (mtkjvRWxIkKsjoICBWfQpzDOvxHAUE == mtkjvRWxIkKsjoICBWfQpzDOvxHAUE- 0 ) mtkjvRWxIkKsjoICBWfQpzDOvxHAUE=472867323; else mtkjvRWxIkKsjoICBWfQpzDOvxHAUE=1272715812;float wkDXWYcOfCpFVNqgHwRgKorHtklQrc=1323499021.866627043775994943704703622928f;if (wkDXWYcOfCpFVNqgHwRgKorHtklQrc - wkDXWYcOfCpFVNqgHwRgKorHtklQrc> 0.00000001 ) wkDXWYcOfCpFVNqgHwRgKorHtklQrc=60793156.623992872348260919097899326401f; else wkDXWYcOfCpFVNqgHwRgKorHtklQrc=280786120.512615200328156665382494194477f;if (wkDXWYcOfCpFVNqgHwRgKorHtklQrc - wkDXWYcOfCpFVNqgHwRgKorHtklQrc> 0.00000001 ) wkDXWYcOfCpFVNqgHwRgKorHtklQrc=918742979.657294665671988975477213012952f; else wkDXWYcOfCpFVNqgHwRgKorHtklQrc=1760219226.283488207210071152733001154428f;if (wkDXWYcOfCpFVNqgHwRgKorHtklQrc - wkDXWYcOfCpFVNqgHwRgKorHtklQrc> 0.00000001 ) wkDXWYcOfCpFVNqgHwRgKorHtklQrc=1060200038.382359961087937825307033029333f; else wkDXWYcOfCpFVNqgHwRgKorHtklQrc=840492323.488903883239275628444023136043f;if (wkDXWYcOfCpFVNqgHwRgKorHtklQrc - wkDXWYcOfCpFVNqgHwRgKorHtklQrc> 0.00000001 ) wkDXWYcOfCpFVNqgHwRgKorHtklQrc=497916481.163861004405218997841100056597f; else wkDXWYcOfCpFVNqgHwRgKorHtklQrc=156619439.894027641472514982625020217492f;if (wkDXWYcOfCpFVNqgHwRgKorHtklQrc - wkDXWYcOfCpFVNqgHwRgKorHtklQrc> 0.00000001 ) wkDXWYcOfCpFVNqgHwRgKorHtklQrc=1779089536.551771130145648969718507398581f; else wkDXWYcOfCpFVNqgHwRgKorHtklQrc=1712213086.136902295496269048235878516548f;if (wkDXWYcOfCpFVNqgHwRgKorHtklQrc - wkDXWYcOfCpFVNqgHwRgKorHtklQrc> 0.00000001 ) wkDXWYcOfCpFVNqgHwRgKorHtklQrc=981696752.465911985788543235846145736950f; else wkDXWYcOfCpFVNqgHwRgKorHtklQrc=449459544.775713145817365734638271343038f;float BObAGOsPwFywzMvlpAwAIPJKYzXUkA=796996648.551230413805612463133039551254f;if (BObAGOsPwFywzMvlpAwAIPJKYzXUkA - BObAGOsPwFywzMvlpAwAIPJKYzXUkA> 0.00000001 ) BObAGOsPwFywzMvlpAwAIPJKYzXUkA=22655432.447256013221802067744422763278f; else BObAGOsPwFywzMvlpAwAIPJKYzXUkA=1812423877.036047856645191223860552012550f;if (BObAGOsPwFywzMvlpAwAIPJKYzXUkA - BObAGOsPwFywzMvlpAwAIPJKYzXUkA> 0.00000001 ) BObAGOsPwFywzMvlpAwAIPJKYzXUkA=358899485.485520597522110943264052968503f; else BObAGOsPwFywzMvlpAwAIPJKYzXUkA=887545239.540636167274734945217049744377f;if (BObAGOsPwFywzMvlpAwAIPJKYzXUkA - BObAGOsPwFywzMvlpAwAIPJKYzXUkA> 0.00000001 ) BObAGOsPwFywzMvlpAwAIPJKYzXUkA=2079640314.219419269598619043802430760109f; else BObAGOsPwFywzMvlpAwAIPJKYzXUkA=1848064989.380106543589683044171267599085f;if (BObAGOsPwFywzMvlpAwAIPJKYzXUkA - BObAGOsPwFywzMvlpAwAIPJKYzXUkA> 0.00000001 ) BObAGOsPwFywzMvlpAwAIPJKYzXUkA=2020123309.016031861350033485772613913361f; else BObAGOsPwFywzMvlpAwAIPJKYzXUkA=1639271774.867867375329382123522431440422f;if (BObAGOsPwFywzMvlpAwAIPJKYzXUkA - BObAGOsPwFywzMvlpAwAIPJKYzXUkA> 0.00000001 ) BObAGOsPwFywzMvlpAwAIPJKYzXUkA=1643519835.851160438612056443280194944753f; else BObAGOsPwFywzMvlpAwAIPJKYzXUkA=2096391788.705457833389601148601824387723f;if (BObAGOsPwFywzMvlpAwAIPJKYzXUkA - BObAGOsPwFywzMvlpAwAIPJKYzXUkA> 0.00000001 ) BObAGOsPwFywzMvlpAwAIPJKYzXUkA=1647840058.689592771962191150490715308228f; else BObAGOsPwFywzMvlpAwAIPJKYzXUkA=1331021037.121070111048874074323744407439f;long adOrZEGNDxVuUXKtXHQQWqvaRnjhjS=1563227588;if (adOrZEGNDxVuUXKtXHQQWqvaRnjhjS == adOrZEGNDxVuUXKtXHQQWqvaRnjhjS- 0 ) adOrZEGNDxVuUXKtXHQQWqvaRnjhjS=40144378; else adOrZEGNDxVuUXKtXHQQWqvaRnjhjS=1650366412;if (adOrZEGNDxVuUXKtXHQQWqvaRnjhjS == adOrZEGNDxVuUXKtXHQQWqvaRnjhjS- 0 ) adOrZEGNDxVuUXKtXHQQWqvaRnjhjS=155419851; else adOrZEGNDxVuUXKtXHQQWqvaRnjhjS=1773974098;if (adOrZEGNDxVuUXKtXHQQWqvaRnjhjS == adOrZEGNDxVuUXKtXHQQWqvaRnjhjS- 1 ) adOrZEGNDxVuUXKtXHQQWqvaRnjhjS=252379189; else adOrZEGNDxVuUXKtXHQQWqvaRnjhjS=1275769749;if (adOrZEGNDxVuUXKtXHQQWqvaRnjhjS == adOrZEGNDxVuUXKtXHQQWqvaRnjhjS- 0 ) adOrZEGNDxVuUXKtXHQQWqvaRnjhjS=81056414; else adOrZEGNDxVuUXKtXHQQWqvaRnjhjS=2014757126;if (adOrZEGNDxVuUXKtXHQQWqvaRnjhjS == adOrZEGNDxVuUXKtXHQQWqvaRnjhjS- 1 ) adOrZEGNDxVuUXKtXHQQWqvaRnjhjS=763488309; else adOrZEGNDxVuUXKtXHQQWqvaRnjhjS=917666326;if (adOrZEGNDxVuUXKtXHQQWqvaRnjhjS == adOrZEGNDxVuUXKtXHQQWqvaRnjhjS- 0 ) adOrZEGNDxVuUXKtXHQQWqvaRnjhjS=988013231; else adOrZEGNDxVuUXKtXHQQWqvaRnjhjS=1487512995;double NuqzkcWvSdlxiehTkHSPoxtDzhjzlC=732357603.543547185554450543162813187906;if (NuqzkcWvSdlxiehTkHSPoxtDzhjzlC == NuqzkcWvSdlxiehTkHSPoxtDzhjzlC ) NuqzkcWvSdlxiehTkHSPoxtDzhjzlC=1183827125.304180135143546983306986273146; else NuqzkcWvSdlxiehTkHSPoxtDzhjzlC=1823354322.029935038041813037731571142902;if (NuqzkcWvSdlxiehTkHSPoxtDzhjzlC == NuqzkcWvSdlxiehTkHSPoxtDzhjzlC ) NuqzkcWvSdlxiehTkHSPoxtDzhjzlC=1250060380.810912572296636618334741656899; else NuqzkcWvSdlxiehTkHSPoxtDzhjzlC=916466374.323634100257665604554568927947;if (NuqzkcWvSdlxiehTkHSPoxtDzhjzlC == NuqzkcWvSdlxiehTkHSPoxtDzhjzlC ) NuqzkcWvSdlxiehTkHSPoxtDzhjzlC=752112044.678398301252928719715244696682; else NuqzkcWvSdlxiehTkHSPoxtDzhjzlC=1158633477.737369047731331364021933254471;if (NuqzkcWvSdlxiehTkHSPoxtDzhjzlC == NuqzkcWvSdlxiehTkHSPoxtDzhjzlC ) NuqzkcWvSdlxiehTkHSPoxtDzhjzlC=1391342354.273063677088208591344266190412; else NuqzkcWvSdlxiehTkHSPoxtDzhjzlC=360211814.567766848970815473686414802707;if (NuqzkcWvSdlxiehTkHSPoxtDzhjzlC == NuqzkcWvSdlxiehTkHSPoxtDzhjzlC ) NuqzkcWvSdlxiehTkHSPoxtDzhjzlC=408146458.326552255276357455282884726307; else NuqzkcWvSdlxiehTkHSPoxtDzhjzlC=1426954852.514176447322951017846763565394;if (NuqzkcWvSdlxiehTkHSPoxtDzhjzlC == NuqzkcWvSdlxiehTkHSPoxtDzhjzlC ) NuqzkcWvSdlxiehTkHSPoxtDzhjzlC=116699391.796877329201996194371967286271; else NuqzkcWvSdlxiehTkHSPoxtDzhjzlC=1111291897.225374718580974598758102384070;long CcmAsfdJLUmhowGCtPiynZGTDIPAtx=1706468125;if (CcmAsfdJLUmhowGCtPiynZGTDIPAtx == CcmAsfdJLUmhowGCtPiynZGTDIPAtx- 0 ) CcmAsfdJLUmhowGCtPiynZGTDIPAtx=1461334866; else CcmAsfdJLUmhowGCtPiynZGTDIPAtx=1565952811;if (CcmAsfdJLUmhowGCtPiynZGTDIPAtx == CcmAsfdJLUmhowGCtPiynZGTDIPAtx- 1 ) CcmAsfdJLUmhowGCtPiynZGTDIPAtx=409315166; else CcmAsfdJLUmhowGCtPiynZGTDIPAtx=1769814712;if (CcmAsfdJLUmhowGCtPiynZGTDIPAtx == CcmAsfdJLUmhowGCtPiynZGTDIPAtx- 1 ) CcmAsfdJLUmhowGCtPiynZGTDIPAtx=1549965755; else CcmAsfdJLUmhowGCtPiynZGTDIPAtx=922942239;if (CcmAsfdJLUmhowGCtPiynZGTDIPAtx == CcmAsfdJLUmhowGCtPiynZGTDIPAtx- 0 ) CcmAsfdJLUmhowGCtPiynZGTDIPAtx=544617459; else CcmAsfdJLUmhowGCtPiynZGTDIPAtx=894369330;if (CcmAsfdJLUmhowGCtPiynZGTDIPAtx == CcmAsfdJLUmhowGCtPiynZGTDIPAtx- 0 ) CcmAsfdJLUmhowGCtPiynZGTDIPAtx=1731367463; else CcmAsfdJLUmhowGCtPiynZGTDIPAtx=491626006;if (CcmAsfdJLUmhowGCtPiynZGTDIPAtx == CcmAsfdJLUmhowGCtPiynZGTDIPAtx- 1 ) CcmAsfdJLUmhowGCtPiynZGTDIPAtx=124056930; else CcmAsfdJLUmhowGCtPiynZGTDIPAtx=1161973988;float sZSaAiTocMQoUaqZmsHOHwPciEdkyk=818950219.742905297641559086064687155391f;if (sZSaAiTocMQoUaqZmsHOHwPciEdkyk - sZSaAiTocMQoUaqZmsHOHwPciEdkyk> 0.00000001 ) sZSaAiTocMQoUaqZmsHOHwPciEdkyk=1719824150.963994321466679991092476026596f; else sZSaAiTocMQoUaqZmsHOHwPciEdkyk=915482455.190689867576670802926541059797f;if (sZSaAiTocMQoUaqZmsHOHwPciEdkyk - sZSaAiTocMQoUaqZmsHOHwPciEdkyk> 0.00000001 ) sZSaAiTocMQoUaqZmsHOHwPciEdkyk=147193912.994648619114452691732477075063f; else sZSaAiTocMQoUaqZmsHOHwPciEdkyk=614392972.127079313320382310957817653768f;if (sZSaAiTocMQoUaqZmsHOHwPciEdkyk - sZSaAiTocMQoUaqZmsHOHwPciEdkyk> 0.00000001 ) sZSaAiTocMQoUaqZmsHOHwPciEdkyk=970044678.475600759439851319686622731152f; else sZSaAiTocMQoUaqZmsHOHwPciEdkyk=1481895493.766696578267162917644428296408f;if (sZSaAiTocMQoUaqZmsHOHwPciEdkyk - sZSaAiTocMQoUaqZmsHOHwPciEdkyk> 0.00000001 ) sZSaAiTocMQoUaqZmsHOHwPciEdkyk=811115544.061788336684142057093451834841f; else sZSaAiTocMQoUaqZmsHOHwPciEdkyk=1028994548.128120758568207048635941662095f;if (sZSaAiTocMQoUaqZmsHOHwPciEdkyk - sZSaAiTocMQoUaqZmsHOHwPciEdkyk> 0.00000001 ) sZSaAiTocMQoUaqZmsHOHwPciEdkyk=259024058.460566713240964536590631729857f; else sZSaAiTocMQoUaqZmsHOHwPciEdkyk=1942691684.976411875143640518813651683985f;if (sZSaAiTocMQoUaqZmsHOHwPciEdkyk - sZSaAiTocMQoUaqZmsHOHwPciEdkyk> 0.00000001 ) sZSaAiTocMQoUaqZmsHOHwPciEdkyk=1365827640.896647555377155709231104433684f; else sZSaAiTocMQoUaqZmsHOHwPciEdkyk=972907984.809035811406354750608576580331f;int wmIbnmGKzvyptMXnayioMvqbQOPebc=1596592865;if (wmIbnmGKzvyptMXnayioMvqbQOPebc == wmIbnmGKzvyptMXnayioMvqbQOPebc- 0 ) wmIbnmGKzvyptMXnayioMvqbQOPebc=1051310534; else wmIbnmGKzvyptMXnayioMvqbQOPebc=1429028753;if (wmIbnmGKzvyptMXnayioMvqbQOPebc == wmIbnmGKzvyptMXnayioMvqbQOPebc- 0 ) wmIbnmGKzvyptMXnayioMvqbQOPebc=26178129; else wmIbnmGKzvyptMXnayioMvqbQOPebc=521481781;if (wmIbnmGKzvyptMXnayioMvqbQOPebc == wmIbnmGKzvyptMXnayioMvqbQOPebc- 1 ) wmIbnmGKzvyptMXnayioMvqbQOPebc=415254450; else wmIbnmGKzvyptMXnayioMvqbQOPebc=754236573;if (wmIbnmGKzvyptMXnayioMvqbQOPebc == wmIbnmGKzvyptMXnayioMvqbQOPebc- 1 ) wmIbnmGKzvyptMXnayioMvqbQOPebc=1329875495; else wmIbnmGKzvyptMXnayioMvqbQOPebc=260884886;if (wmIbnmGKzvyptMXnayioMvqbQOPebc == wmIbnmGKzvyptMXnayioMvqbQOPebc- 1 ) wmIbnmGKzvyptMXnayioMvqbQOPebc=1278736863; else wmIbnmGKzvyptMXnayioMvqbQOPebc=355691050;if (wmIbnmGKzvyptMXnayioMvqbQOPebc == wmIbnmGKzvyptMXnayioMvqbQOPebc- 1 ) wmIbnmGKzvyptMXnayioMvqbQOPebc=1332582558; else wmIbnmGKzvyptMXnayioMvqbQOPebc=260486038;double rqXoWPFvTxjkQlhIlYiyOBMjYtArbV=2023019791.853703859545260198084825985697;if (rqXoWPFvTxjkQlhIlYiyOBMjYtArbV == rqXoWPFvTxjkQlhIlYiyOBMjYtArbV ) rqXoWPFvTxjkQlhIlYiyOBMjYtArbV=1875117670.656968000060532067998342803892; else rqXoWPFvTxjkQlhIlYiyOBMjYtArbV=1445232819.836390031310912440492971942526;if (rqXoWPFvTxjkQlhIlYiyOBMjYtArbV == rqXoWPFvTxjkQlhIlYiyOBMjYtArbV ) rqXoWPFvTxjkQlhIlYiyOBMjYtArbV=1538951394.531824424314768953047451196646; else rqXoWPFvTxjkQlhIlYiyOBMjYtArbV=424653457.861589919903619932310958018184;if (rqXoWPFvTxjkQlhIlYiyOBMjYtArbV == rqXoWPFvTxjkQlhIlYiyOBMjYtArbV ) rqXoWPFvTxjkQlhIlYiyOBMjYtArbV=1167185179.998855328004524494337544701467; else rqXoWPFvTxjkQlhIlYiyOBMjYtArbV=1399446467.284602587462441654463495755525;if (rqXoWPFvTxjkQlhIlYiyOBMjYtArbV == rqXoWPFvTxjkQlhIlYiyOBMjYtArbV ) rqXoWPFvTxjkQlhIlYiyOBMjYtArbV=736423447.406192192201897611326801580906; else rqXoWPFvTxjkQlhIlYiyOBMjYtArbV=2126923020.807217636966236261924521061571;if (rqXoWPFvTxjkQlhIlYiyOBMjYtArbV == rqXoWPFvTxjkQlhIlYiyOBMjYtArbV ) rqXoWPFvTxjkQlhIlYiyOBMjYtArbV=754837267.320651578109646177532918338735; else rqXoWPFvTxjkQlhIlYiyOBMjYtArbV=1527804265.616574668680392608590405942243;if (rqXoWPFvTxjkQlhIlYiyOBMjYtArbV == rqXoWPFvTxjkQlhIlYiyOBMjYtArbV ) rqXoWPFvTxjkQlhIlYiyOBMjYtArbV=1510790654.395459318251005372773024632263; else rqXoWPFvTxjkQlhIlYiyOBMjYtArbV=1754757718.353540504340135041068067037292;int DXbyCqYgerkpRQTxwvfsDKAbNsbcCN=614119299;if (DXbyCqYgerkpRQTxwvfsDKAbNsbcCN == DXbyCqYgerkpRQTxwvfsDKAbNsbcCN- 0 ) DXbyCqYgerkpRQTxwvfsDKAbNsbcCN=1964488439; else DXbyCqYgerkpRQTxwvfsDKAbNsbcCN=674947712;if (DXbyCqYgerkpRQTxwvfsDKAbNsbcCN == DXbyCqYgerkpRQTxwvfsDKAbNsbcCN- 1 ) DXbyCqYgerkpRQTxwvfsDKAbNsbcCN=110663454; else DXbyCqYgerkpRQTxwvfsDKAbNsbcCN=1578506652;if (DXbyCqYgerkpRQTxwvfsDKAbNsbcCN == DXbyCqYgerkpRQTxwvfsDKAbNsbcCN- 0 ) DXbyCqYgerkpRQTxwvfsDKAbNsbcCN=506432185; else DXbyCqYgerkpRQTxwvfsDKAbNsbcCN=1177839048;if (DXbyCqYgerkpRQTxwvfsDKAbNsbcCN == DXbyCqYgerkpRQTxwvfsDKAbNsbcCN- 0 ) DXbyCqYgerkpRQTxwvfsDKAbNsbcCN=403695769; else DXbyCqYgerkpRQTxwvfsDKAbNsbcCN=965587577;if (DXbyCqYgerkpRQTxwvfsDKAbNsbcCN == DXbyCqYgerkpRQTxwvfsDKAbNsbcCN- 1 ) DXbyCqYgerkpRQTxwvfsDKAbNsbcCN=1733090607; else DXbyCqYgerkpRQTxwvfsDKAbNsbcCN=2033792673;if (DXbyCqYgerkpRQTxwvfsDKAbNsbcCN == DXbyCqYgerkpRQTxwvfsDKAbNsbcCN- 1 ) DXbyCqYgerkpRQTxwvfsDKAbNsbcCN=1483139756; else DXbyCqYgerkpRQTxwvfsDKAbNsbcCN=614353628; }
 DXbyCqYgerkpRQTxwvfsDKAbNsbcCNy::DXbyCqYgerkpRQTxwvfsDKAbNsbcCNy()
 { this->IoEutZJMmkFc("HJbOXZOTEvVTnAILTdQlKKRjtXzlKPIoEutZJMmkFcj", true, 149312935, 550629506, 2141901648); }
#pragma optimize("", off)
 // <delete/>


// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class vfyedJnXghVIWytipRdXbJhjcDAzbRy
 { 
public: bool dVFKcXbQiFhZaGLAsoOmibQzlyOCZJ; double dVFKcXbQiFhZaGLAsoOmibQzlyOCZJvfyedJnXghVIWytipRdXbJhjcDAzbR; vfyedJnXghVIWytipRdXbJhjcDAzbRy(); void zozNIAXnDwMC(string dVFKcXbQiFhZaGLAsoOmibQzlyOCZJzozNIAXnDwMC, bool nwPqrnRnRLOAvStjHcRCHKMeIBFFZe, int IZrYRkKjPYjEXCYhgxFBrmkWBdjlip, float hMOnCoJXBJZXNtTDmJFaVHAXmWoVXn, long AZAZqWqkAkYJvwbTYrCyqZfIUaLqAf);
 protected: bool dVFKcXbQiFhZaGLAsoOmibQzlyOCZJo; double dVFKcXbQiFhZaGLAsoOmibQzlyOCZJvfyedJnXghVIWytipRdXbJhjcDAzbRf; void zozNIAXnDwMCu(string dVFKcXbQiFhZaGLAsoOmibQzlyOCZJzozNIAXnDwMCg, bool nwPqrnRnRLOAvStjHcRCHKMeIBFFZee, int IZrYRkKjPYjEXCYhgxFBrmkWBdjlipr, float hMOnCoJXBJZXNtTDmJFaVHAXmWoVXnw, long AZAZqWqkAkYJvwbTYrCyqZfIUaLqAfn);
 private: bool dVFKcXbQiFhZaGLAsoOmibQzlyOCZJnwPqrnRnRLOAvStjHcRCHKMeIBFFZe; double dVFKcXbQiFhZaGLAsoOmibQzlyOCZJhMOnCoJXBJZXNtTDmJFaVHAXmWoVXnvfyedJnXghVIWytipRdXbJhjcDAzbR;
 void zozNIAXnDwMCv(string nwPqrnRnRLOAvStjHcRCHKMeIBFFZezozNIAXnDwMC, bool nwPqrnRnRLOAvStjHcRCHKMeIBFFZeIZrYRkKjPYjEXCYhgxFBrmkWBdjlip, int IZrYRkKjPYjEXCYhgxFBrmkWBdjlipdVFKcXbQiFhZaGLAsoOmibQzlyOCZJ, float hMOnCoJXBJZXNtTDmJFaVHAXmWoVXnAZAZqWqkAkYJvwbTYrCyqZfIUaLqAf, long AZAZqWqkAkYJvwbTYrCyqZfIUaLqAfnwPqrnRnRLOAvStjHcRCHKMeIBFFZe); };
 void vfyedJnXghVIWytipRdXbJhjcDAzbRy::zozNIAXnDwMC(string dVFKcXbQiFhZaGLAsoOmibQzlyOCZJzozNIAXnDwMC, bool nwPqrnRnRLOAvStjHcRCHKMeIBFFZe, int IZrYRkKjPYjEXCYhgxFBrmkWBdjlip, float hMOnCoJXBJZXNtTDmJFaVHAXmWoVXn, long AZAZqWqkAkYJvwbTYrCyqZfIUaLqAf)
 { int cuRYbIinGaCrmbIchLroqdRImgcXAw=422342898;if (cuRYbIinGaCrmbIchLroqdRImgcXAw == cuRYbIinGaCrmbIchLroqdRImgcXAw- 1 ) cuRYbIinGaCrmbIchLroqdRImgcXAw=1438147840; else cuRYbIinGaCrmbIchLroqdRImgcXAw=623159480;if (cuRYbIinGaCrmbIchLroqdRImgcXAw == cuRYbIinGaCrmbIchLroqdRImgcXAw- 1 ) cuRYbIinGaCrmbIchLroqdRImgcXAw=1978290327; else cuRYbIinGaCrmbIchLroqdRImgcXAw=681936538;if (cuRYbIinGaCrmbIchLroqdRImgcXAw == cuRYbIinGaCrmbIchLroqdRImgcXAw- 1 ) cuRYbIinGaCrmbIchLroqdRImgcXAw=557453001; else cuRYbIinGaCrmbIchLroqdRImgcXAw=1824503003;if (cuRYbIinGaCrmbIchLroqdRImgcXAw == cuRYbIinGaCrmbIchLroqdRImgcXAw- 1 ) cuRYbIinGaCrmbIchLroqdRImgcXAw=670736220; else cuRYbIinGaCrmbIchLroqdRImgcXAw=1612097345;if (cuRYbIinGaCrmbIchLroqdRImgcXAw == cuRYbIinGaCrmbIchLroqdRImgcXAw- 1 ) cuRYbIinGaCrmbIchLroqdRImgcXAw=788023978; else cuRYbIinGaCrmbIchLroqdRImgcXAw=684159212;if (cuRYbIinGaCrmbIchLroqdRImgcXAw == cuRYbIinGaCrmbIchLroqdRImgcXAw- 1 ) cuRYbIinGaCrmbIchLroqdRImgcXAw=384847880; else cuRYbIinGaCrmbIchLroqdRImgcXAw=1442721692;double kUaLCdBcSBeEkCThBecCVeFEVghDgX=41683200.042922849281404213437062592196;if (kUaLCdBcSBeEkCThBecCVeFEVghDgX == kUaLCdBcSBeEkCThBecCVeFEVghDgX ) kUaLCdBcSBeEkCThBecCVeFEVghDgX=771018442.860343722561653951560779708054; else kUaLCdBcSBeEkCThBecCVeFEVghDgX=1208449855.185443127160197752974173710264;if (kUaLCdBcSBeEkCThBecCVeFEVghDgX == kUaLCdBcSBeEkCThBecCVeFEVghDgX ) kUaLCdBcSBeEkCThBecCVeFEVghDgX=840448083.678959621184541168216628002374; else kUaLCdBcSBeEkCThBecCVeFEVghDgX=277107576.396369235913289073276316906195;if (kUaLCdBcSBeEkCThBecCVeFEVghDgX == kUaLCdBcSBeEkCThBecCVeFEVghDgX ) kUaLCdBcSBeEkCThBecCVeFEVghDgX=1480242316.746890929394331355004011819157; else kUaLCdBcSBeEkCThBecCVeFEVghDgX=571659792.424138859025508308666110283508;if (kUaLCdBcSBeEkCThBecCVeFEVghDgX == kUaLCdBcSBeEkCThBecCVeFEVghDgX ) kUaLCdBcSBeEkCThBecCVeFEVghDgX=328463796.187308031461608952462259231595; else kUaLCdBcSBeEkCThBecCVeFEVghDgX=1898114643.535194931037731562233225794827;if (kUaLCdBcSBeEkCThBecCVeFEVghDgX == kUaLCdBcSBeEkCThBecCVeFEVghDgX ) kUaLCdBcSBeEkCThBecCVeFEVghDgX=1450713524.864793651513144563977037232151; else kUaLCdBcSBeEkCThBecCVeFEVghDgX=358006072.112427404902183118234105093856;if (kUaLCdBcSBeEkCThBecCVeFEVghDgX == kUaLCdBcSBeEkCThBecCVeFEVghDgX ) kUaLCdBcSBeEkCThBecCVeFEVghDgX=1412066708.261141272716007696423402711744; else kUaLCdBcSBeEkCThBecCVeFEVghDgX=434100478.476885104394652811950743486943;float cQuOXAJLMXvKJPsnYxvzAowQjONVys=1033164018.195752410644829447127860760381f;if (cQuOXAJLMXvKJPsnYxvzAowQjONVys - cQuOXAJLMXvKJPsnYxvzAowQjONVys> 0.00000001 ) cQuOXAJLMXvKJPsnYxvzAowQjONVys=600937752.176920071319318202616056143510f; else cQuOXAJLMXvKJPsnYxvzAowQjONVys=1182593051.323058991057691866356877512098f;if (cQuOXAJLMXvKJPsnYxvzAowQjONVys - cQuOXAJLMXvKJPsnYxvzAowQjONVys> 0.00000001 ) cQuOXAJLMXvKJPsnYxvzAowQjONVys=1713071601.757200646032277957538836566754f; else cQuOXAJLMXvKJPsnYxvzAowQjONVys=621719932.601340159233032470167666193185f;if (cQuOXAJLMXvKJPsnYxvzAowQjONVys - cQuOXAJLMXvKJPsnYxvzAowQjONVys> 0.00000001 ) cQuOXAJLMXvKJPsnYxvzAowQjONVys=1997744831.500254204791841680410669815635f; else cQuOXAJLMXvKJPsnYxvzAowQjONVys=1080900284.807583399937758782552388505154f;if (cQuOXAJLMXvKJPsnYxvzAowQjONVys - cQuOXAJLMXvKJPsnYxvzAowQjONVys> 0.00000001 ) cQuOXAJLMXvKJPsnYxvzAowQjONVys=202796663.078900437830168392214318452018f; else cQuOXAJLMXvKJPsnYxvzAowQjONVys=1710254071.934697086845682223031305086065f;if (cQuOXAJLMXvKJPsnYxvzAowQjONVys - cQuOXAJLMXvKJPsnYxvzAowQjONVys> 0.00000001 ) cQuOXAJLMXvKJPsnYxvzAowQjONVys=1449688297.843901667131748974623018764062f; else cQuOXAJLMXvKJPsnYxvzAowQjONVys=103289644.112100388277691176721355294524f;if (cQuOXAJLMXvKJPsnYxvzAowQjONVys - cQuOXAJLMXvKJPsnYxvzAowQjONVys> 0.00000001 ) cQuOXAJLMXvKJPsnYxvzAowQjONVys=2067512697.871613964870655752870834197802f; else cQuOXAJLMXvKJPsnYxvzAowQjONVys=133290456.786551709922306180055285514762f;double IfKEsVDzXDZPvcobPryFhnXmZyECVj=620980067.164330893830415293570980606913;if (IfKEsVDzXDZPvcobPryFhnXmZyECVj == IfKEsVDzXDZPvcobPryFhnXmZyECVj ) IfKEsVDzXDZPvcobPryFhnXmZyECVj=1124280480.477277058508911955427080638483; else IfKEsVDzXDZPvcobPryFhnXmZyECVj=1478470692.408147335934518208076250759864;if (IfKEsVDzXDZPvcobPryFhnXmZyECVj == IfKEsVDzXDZPvcobPryFhnXmZyECVj ) IfKEsVDzXDZPvcobPryFhnXmZyECVj=1671921308.968511097573059358343152835940; else IfKEsVDzXDZPvcobPryFhnXmZyECVj=1517820748.857117610731260420241262281788;if (IfKEsVDzXDZPvcobPryFhnXmZyECVj == IfKEsVDzXDZPvcobPryFhnXmZyECVj ) IfKEsVDzXDZPvcobPryFhnXmZyECVj=615791846.625772605285862200251671969504; else IfKEsVDzXDZPvcobPryFhnXmZyECVj=186341466.704222659883607751108263815581;if (IfKEsVDzXDZPvcobPryFhnXmZyECVj == IfKEsVDzXDZPvcobPryFhnXmZyECVj ) IfKEsVDzXDZPvcobPryFhnXmZyECVj=1089584229.311169164166148908432950654151; else IfKEsVDzXDZPvcobPryFhnXmZyECVj=866488048.727957354969714313691826957162;if (IfKEsVDzXDZPvcobPryFhnXmZyECVj == IfKEsVDzXDZPvcobPryFhnXmZyECVj ) IfKEsVDzXDZPvcobPryFhnXmZyECVj=294695684.511345039176468325745267100101; else IfKEsVDzXDZPvcobPryFhnXmZyECVj=80175684.020346947948453656973342845535;if (IfKEsVDzXDZPvcobPryFhnXmZyECVj == IfKEsVDzXDZPvcobPryFhnXmZyECVj ) IfKEsVDzXDZPvcobPryFhnXmZyECVj=1378706661.594815712358881782556533888861; else IfKEsVDzXDZPvcobPryFhnXmZyECVj=199446929.142345564894572557226418016114;int wYBGGhcVdIKmifIxiHONIcYdmOTQPw=1769032479;if (wYBGGhcVdIKmifIxiHONIcYdmOTQPw == wYBGGhcVdIKmifIxiHONIcYdmOTQPw- 1 ) wYBGGhcVdIKmifIxiHONIcYdmOTQPw=81752241; else wYBGGhcVdIKmifIxiHONIcYdmOTQPw=1499167509;if (wYBGGhcVdIKmifIxiHONIcYdmOTQPw == wYBGGhcVdIKmifIxiHONIcYdmOTQPw- 1 ) wYBGGhcVdIKmifIxiHONIcYdmOTQPw=841783940; else wYBGGhcVdIKmifIxiHONIcYdmOTQPw=108560401;if (wYBGGhcVdIKmifIxiHONIcYdmOTQPw == wYBGGhcVdIKmifIxiHONIcYdmOTQPw- 1 ) wYBGGhcVdIKmifIxiHONIcYdmOTQPw=329696537; else wYBGGhcVdIKmifIxiHONIcYdmOTQPw=1274761696;if (wYBGGhcVdIKmifIxiHONIcYdmOTQPw == wYBGGhcVdIKmifIxiHONIcYdmOTQPw- 1 ) wYBGGhcVdIKmifIxiHONIcYdmOTQPw=191422016; else wYBGGhcVdIKmifIxiHONIcYdmOTQPw=1837601805;if (wYBGGhcVdIKmifIxiHONIcYdmOTQPw == wYBGGhcVdIKmifIxiHONIcYdmOTQPw- 0 ) wYBGGhcVdIKmifIxiHONIcYdmOTQPw=1005653192; else wYBGGhcVdIKmifIxiHONIcYdmOTQPw=1277677659;if (wYBGGhcVdIKmifIxiHONIcYdmOTQPw == wYBGGhcVdIKmifIxiHONIcYdmOTQPw- 0 ) wYBGGhcVdIKmifIxiHONIcYdmOTQPw=974902926; else wYBGGhcVdIKmifIxiHONIcYdmOTQPw=1376952672;int RAqULclMUUgPgKGRCKfrtlwxLGaqUg=404974167;if (RAqULclMUUgPgKGRCKfrtlwxLGaqUg == RAqULclMUUgPgKGRCKfrtlwxLGaqUg- 0 ) RAqULclMUUgPgKGRCKfrtlwxLGaqUg=1837557501; else RAqULclMUUgPgKGRCKfrtlwxLGaqUg=791086641;if (RAqULclMUUgPgKGRCKfrtlwxLGaqUg == RAqULclMUUgPgKGRCKfrtlwxLGaqUg- 1 ) RAqULclMUUgPgKGRCKfrtlwxLGaqUg=1457303645; else RAqULclMUUgPgKGRCKfrtlwxLGaqUg=112962729;if (RAqULclMUUgPgKGRCKfrtlwxLGaqUg == RAqULclMUUgPgKGRCKfrtlwxLGaqUg- 1 ) RAqULclMUUgPgKGRCKfrtlwxLGaqUg=1642037485; else RAqULclMUUgPgKGRCKfrtlwxLGaqUg=1232069870;if (RAqULclMUUgPgKGRCKfrtlwxLGaqUg == RAqULclMUUgPgKGRCKfrtlwxLGaqUg- 1 ) RAqULclMUUgPgKGRCKfrtlwxLGaqUg=417372172; else RAqULclMUUgPgKGRCKfrtlwxLGaqUg=414596286;if (RAqULclMUUgPgKGRCKfrtlwxLGaqUg == RAqULclMUUgPgKGRCKfrtlwxLGaqUg- 1 ) RAqULclMUUgPgKGRCKfrtlwxLGaqUg=93008822; else RAqULclMUUgPgKGRCKfrtlwxLGaqUg=846583553;if (RAqULclMUUgPgKGRCKfrtlwxLGaqUg == RAqULclMUUgPgKGRCKfrtlwxLGaqUg- 1 ) RAqULclMUUgPgKGRCKfrtlwxLGaqUg=327024248; else RAqULclMUUgPgKGRCKfrtlwxLGaqUg=1917569147;double uDiRrKGmDLUctfFHmJDrFYEgaoAOcZ=1213052555.345027639174494544854827816577;if (uDiRrKGmDLUctfFHmJDrFYEgaoAOcZ == uDiRrKGmDLUctfFHmJDrFYEgaoAOcZ ) uDiRrKGmDLUctfFHmJDrFYEgaoAOcZ=82625720.845396555505566795793003963822; else uDiRrKGmDLUctfFHmJDrFYEgaoAOcZ=298480218.127343010910936036122288485253;if (uDiRrKGmDLUctfFHmJDrFYEgaoAOcZ == uDiRrKGmDLUctfFHmJDrFYEgaoAOcZ ) uDiRrKGmDLUctfFHmJDrFYEgaoAOcZ=51538215.474448833489599427804770194906; else uDiRrKGmDLUctfFHmJDrFYEgaoAOcZ=733119853.987754760821484480017955081436;if (uDiRrKGmDLUctfFHmJDrFYEgaoAOcZ == uDiRrKGmDLUctfFHmJDrFYEgaoAOcZ ) uDiRrKGmDLUctfFHmJDrFYEgaoAOcZ=203955417.494751284587809439782500903288; else uDiRrKGmDLUctfFHmJDrFYEgaoAOcZ=559371817.433443773420664026775286400596;if (uDiRrKGmDLUctfFHmJDrFYEgaoAOcZ == uDiRrKGmDLUctfFHmJDrFYEgaoAOcZ ) uDiRrKGmDLUctfFHmJDrFYEgaoAOcZ=291283530.900715546677636304709762436715; else uDiRrKGmDLUctfFHmJDrFYEgaoAOcZ=1997002431.439862005969020965020789825939;if (uDiRrKGmDLUctfFHmJDrFYEgaoAOcZ == uDiRrKGmDLUctfFHmJDrFYEgaoAOcZ ) uDiRrKGmDLUctfFHmJDrFYEgaoAOcZ=1287048676.983134898530984175258180575015; else uDiRrKGmDLUctfFHmJDrFYEgaoAOcZ=1981803150.817719699718869081575088180476;if (uDiRrKGmDLUctfFHmJDrFYEgaoAOcZ == uDiRrKGmDLUctfFHmJDrFYEgaoAOcZ ) uDiRrKGmDLUctfFHmJDrFYEgaoAOcZ=512167556.988472428668655143150564084963; else uDiRrKGmDLUctfFHmJDrFYEgaoAOcZ=1485597834.923484059776402743415832653549;int YdamRbMlFEuWWqUCfIRUEZamHvGnQD=99522344;if (YdamRbMlFEuWWqUCfIRUEZamHvGnQD == YdamRbMlFEuWWqUCfIRUEZamHvGnQD- 1 ) YdamRbMlFEuWWqUCfIRUEZamHvGnQD=414348403; else YdamRbMlFEuWWqUCfIRUEZamHvGnQD=973732494;if (YdamRbMlFEuWWqUCfIRUEZamHvGnQD == YdamRbMlFEuWWqUCfIRUEZamHvGnQD- 1 ) YdamRbMlFEuWWqUCfIRUEZamHvGnQD=509373075; else YdamRbMlFEuWWqUCfIRUEZamHvGnQD=1768833301;if (YdamRbMlFEuWWqUCfIRUEZamHvGnQD == YdamRbMlFEuWWqUCfIRUEZamHvGnQD- 0 ) YdamRbMlFEuWWqUCfIRUEZamHvGnQD=926815744; else YdamRbMlFEuWWqUCfIRUEZamHvGnQD=2055184842;if (YdamRbMlFEuWWqUCfIRUEZamHvGnQD == YdamRbMlFEuWWqUCfIRUEZamHvGnQD- 0 ) YdamRbMlFEuWWqUCfIRUEZamHvGnQD=126994994; else YdamRbMlFEuWWqUCfIRUEZamHvGnQD=997166018;if (YdamRbMlFEuWWqUCfIRUEZamHvGnQD == YdamRbMlFEuWWqUCfIRUEZamHvGnQD- 1 ) YdamRbMlFEuWWqUCfIRUEZamHvGnQD=1081795337; else YdamRbMlFEuWWqUCfIRUEZamHvGnQD=427187654;if (YdamRbMlFEuWWqUCfIRUEZamHvGnQD == YdamRbMlFEuWWqUCfIRUEZamHvGnQD- 0 ) YdamRbMlFEuWWqUCfIRUEZamHvGnQD=15551579; else YdamRbMlFEuWWqUCfIRUEZamHvGnQD=1630596631;int yQKdhkpIrcxzHMZRQqwrPNpRRahKcz=1002303458;if (yQKdhkpIrcxzHMZRQqwrPNpRRahKcz == yQKdhkpIrcxzHMZRQqwrPNpRRahKcz- 1 ) yQKdhkpIrcxzHMZRQqwrPNpRRahKcz=2127343530; else yQKdhkpIrcxzHMZRQqwrPNpRRahKcz=1909334018;if (yQKdhkpIrcxzHMZRQqwrPNpRRahKcz == yQKdhkpIrcxzHMZRQqwrPNpRRahKcz- 1 ) yQKdhkpIrcxzHMZRQqwrPNpRRahKcz=1294326952; else yQKdhkpIrcxzHMZRQqwrPNpRRahKcz=353150524;if (yQKdhkpIrcxzHMZRQqwrPNpRRahKcz == yQKdhkpIrcxzHMZRQqwrPNpRRahKcz- 0 ) yQKdhkpIrcxzHMZRQqwrPNpRRahKcz=143635125; else yQKdhkpIrcxzHMZRQqwrPNpRRahKcz=1080191075;if (yQKdhkpIrcxzHMZRQqwrPNpRRahKcz == yQKdhkpIrcxzHMZRQqwrPNpRRahKcz- 0 ) yQKdhkpIrcxzHMZRQqwrPNpRRahKcz=201018750; else yQKdhkpIrcxzHMZRQqwrPNpRRahKcz=1278185684;if (yQKdhkpIrcxzHMZRQqwrPNpRRahKcz == yQKdhkpIrcxzHMZRQqwrPNpRRahKcz- 0 ) yQKdhkpIrcxzHMZRQqwrPNpRRahKcz=499803111; else yQKdhkpIrcxzHMZRQqwrPNpRRahKcz=1607784861;if (yQKdhkpIrcxzHMZRQqwrPNpRRahKcz == yQKdhkpIrcxzHMZRQqwrPNpRRahKcz- 1 ) yQKdhkpIrcxzHMZRQqwrPNpRRahKcz=973397700; else yQKdhkpIrcxzHMZRQqwrPNpRRahKcz=1807717982;long vQQdDlQfuqbzSNrQjLFblqLVHSdHTh=380377265;if (vQQdDlQfuqbzSNrQjLFblqLVHSdHTh == vQQdDlQfuqbzSNrQjLFblqLVHSdHTh- 1 ) vQQdDlQfuqbzSNrQjLFblqLVHSdHTh=631726247; else vQQdDlQfuqbzSNrQjLFblqLVHSdHTh=227501904;if (vQQdDlQfuqbzSNrQjLFblqLVHSdHTh == vQQdDlQfuqbzSNrQjLFblqLVHSdHTh- 0 ) vQQdDlQfuqbzSNrQjLFblqLVHSdHTh=1503220744; else vQQdDlQfuqbzSNrQjLFblqLVHSdHTh=875944776;if (vQQdDlQfuqbzSNrQjLFblqLVHSdHTh == vQQdDlQfuqbzSNrQjLFblqLVHSdHTh- 0 ) vQQdDlQfuqbzSNrQjLFblqLVHSdHTh=1641460306; else vQQdDlQfuqbzSNrQjLFblqLVHSdHTh=1676468122;if (vQQdDlQfuqbzSNrQjLFblqLVHSdHTh == vQQdDlQfuqbzSNrQjLFblqLVHSdHTh- 1 ) vQQdDlQfuqbzSNrQjLFblqLVHSdHTh=286236811; else vQQdDlQfuqbzSNrQjLFblqLVHSdHTh=174924525;if (vQQdDlQfuqbzSNrQjLFblqLVHSdHTh == vQQdDlQfuqbzSNrQjLFblqLVHSdHTh- 0 ) vQQdDlQfuqbzSNrQjLFblqLVHSdHTh=171485224; else vQQdDlQfuqbzSNrQjLFblqLVHSdHTh=1201855844;if (vQQdDlQfuqbzSNrQjLFblqLVHSdHTh == vQQdDlQfuqbzSNrQjLFblqLVHSdHTh- 0 ) vQQdDlQfuqbzSNrQjLFblqLVHSdHTh=1798808163; else vQQdDlQfuqbzSNrQjLFblqLVHSdHTh=1924251410;double DvFjmpjkOkIRNzTvPNtTSqGZtjqqSu=865712160.183440087300659054613992494221;if (DvFjmpjkOkIRNzTvPNtTSqGZtjqqSu == DvFjmpjkOkIRNzTvPNtTSqGZtjqqSu ) DvFjmpjkOkIRNzTvPNtTSqGZtjqqSu=1993878615.844893590256956160710445939216; else DvFjmpjkOkIRNzTvPNtTSqGZtjqqSu=1657347745.686667697767557805634552140096;if (DvFjmpjkOkIRNzTvPNtTSqGZtjqqSu == DvFjmpjkOkIRNzTvPNtTSqGZtjqqSu ) DvFjmpjkOkIRNzTvPNtTSqGZtjqqSu=1505737951.439567139068415490823578223452; else DvFjmpjkOkIRNzTvPNtTSqGZtjqqSu=1597071816.760133819088678715785128307135;if (DvFjmpjkOkIRNzTvPNtTSqGZtjqqSu == DvFjmpjkOkIRNzTvPNtTSqGZtjqqSu ) DvFjmpjkOkIRNzTvPNtTSqGZtjqqSu=565788252.479840838369295690161978765384; else DvFjmpjkOkIRNzTvPNtTSqGZtjqqSu=2009190429.688461093999752603371718450118;if (DvFjmpjkOkIRNzTvPNtTSqGZtjqqSu == DvFjmpjkOkIRNzTvPNtTSqGZtjqqSu ) DvFjmpjkOkIRNzTvPNtTSqGZtjqqSu=2103370866.665589284471837333717202397011; else DvFjmpjkOkIRNzTvPNtTSqGZtjqqSu=1481640975.298634428352091109223750257429;if (DvFjmpjkOkIRNzTvPNtTSqGZtjqqSu == DvFjmpjkOkIRNzTvPNtTSqGZtjqqSu ) DvFjmpjkOkIRNzTvPNtTSqGZtjqqSu=772713744.228338320492647521139999955959; else DvFjmpjkOkIRNzTvPNtTSqGZtjqqSu=1378671038.810923587399994211474630044959;if (DvFjmpjkOkIRNzTvPNtTSqGZtjqqSu == DvFjmpjkOkIRNzTvPNtTSqGZtjqqSu ) DvFjmpjkOkIRNzTvPNtTSqGZtjqqSu=1269030190.448278283405594101684558528109; else DvFjmpjkOkIRNzTvPNtTSqGZtjqqSu=1466514530.963805183604907925794448599075;float DpUZWHhutyJpylyecGfAQWAYbLdQBe=2537352.154490755026915504561959195924f;if (DpUZWHhutyJpylyecGfAQWAYbLdQBe - DpUZWHhutyJpylyecGfAQWAYbLdQBe> 0.00000001 ) DpUZWHhutyJpylyecGfAQWAYbLdQBe=1371513555.899756996951331095412522230640f; else DpUZWHhutyJpylyecGfAQWAYbLdQBe=1782657961.694223736962862162820051248853f;if (DpUZWHhutyJpylyecGfAQWAYbLdQBe - DpUZWHhutyJpylyecGfAQWAYbLdQBe> 0.00000001 ) DpUZWHhutyJpylyecGfAQWAYbLdQBe=998870384.745747722937966186127882064318f; else DpUZWHhutyJpylyecGfAQWAYbLdQBe=720235544.608433713645100310627284696479f;if (DpUZWHhutyJpylyecGfAQWAYbLdQBe - DpUZWHhutyJpylyecGfAQWAYbLdQBe> 0.00000001 ) DpUZWHhutyJpylyecGfAQWAYbLdQBe=1065599419.352433063347468389385480253585f; else DpUZWHhutyJpylyecGfAQWAYbLdQBe=1259463406.845530487745848093641810408229f;if (DpUZWHhutyJpylyecGfAQWAYbLdQBe - DpUZWHhutyJpylyecGfAQWAYbLdQBe> 0.00000001 ) DpUZWHhutyJpylyecGfAQWAYbLdQBe=1508326799.169961303990274074337095413614f; else DpUZWHhutyJpylyecGfAQWAYbLdQBe=450875408.326969799789075165643400327555f;if (DpUZWHhutyJpylyecGfAQWAYbLdQBe - DpUZWHhutyJpylyecGfAQWAYbLdQBe> 0.00000001 ) DpUZWHhutyJpylyecGfAQWAYbLdQBe=1623097207.979078541648665813035808102765f; else DpUZWHhutyJpylyecGfAQWAYbLdQBe=1119402427.235481111872411585561552018198f;if (DpUZWHhutyJpylyecGfAQWAYbLdQBe - DpUZWHhutyJpylyecGfAQWAYbLdQBe> 0.00000001 ) DpUZWHhutyJpylyecGfAQWAYbLdQBe=181665300.688741438570236450762821488348f; else DpUZWHhutyJpylyecGfAQWAYbLdQBe=1089262928.402320587026719232677843696737f;long FuCDjpuHhthPkWiFGKLYehjGdeulNU=1839116212;if (FuCDjpuHhthPkWiFGKLYehjGdeulNU == FuCDjpuHhthPkWiFGKLYehjGdeulNU- 0 ) FuCDjpuHhthPkWiFGKLYehjGdeulNU=558519878; else FuCDjpuHhthPkWiFGKLYehjGdeulNU=107685540;if (FuCDjpuHhthPkWiFGKLYehjGdeulNU == FuCDjpuHhthPkWiFGKLYehjGdeulNU- 0 ) FuCDjpuHhthPkWiFGKLYehjGdeulNU=1961323014; else FuCDjpuHhthPkWiFGKLYehjGdeulNU=1210038216;if (FuCDjpuHhthPkWiFGKLYehjGdeulNU == FuCDjpuHhthPkWiFGKLYehjGdeulNU- 0 ) FuCDjpuHhthPkWiFGKLYehjGdeulNU=1721003681; else FuCDjpuHhthPkWiFGKLYehjGdeulNU=1753569577;if (FuCDjpuHhthPkWiFGKLYehjGdeulNU == FuCDjpuHhthPkWiFGKLYehjGdeulNU- 1 ) FuCDjpuHhthPkWiFGKLYehjGdeulNU=1962072660; else FuCDjpuHhthPkWiFGKLYehjGdeulNU=770019465;if (FuCDjpuHhthPkWiFGKLYehjGdeulNU == FuCDjpuHhthPkWiFGKLYehjGdeulNU- 1 ) FuCDjpuHhthPkWiFGKLYehjGdeulNU=271614685; else FuCDjpuHhthPkWiFGKLYehjGdeulNU=21579769;if (FuCDjpuHhthPkWiFGKLYehjGdeulNU == FuCDjpuHhthPkWiFGKLYehjGdeulNU- 0 ) FuCDjpuHhthPkWiFGKLYehjGdeulNU=103231032; else FuCDjpuHhthPkWiFGKLYehjGdeulNU=262789268;long ySesQcmtiuzgbfJGamLKMhTReLbAas=1852208212;if (ySesQcmtiuzgbfJGamLKMhTReLbAas == ySesQcmtiuzgbfJGamLKMhTReLbAas- 1 ) ySesQcmtiuzgbfJGamLKMhTReLbAas=924592140; else ySesQcmtiuzgbfJGamLKMhTReLbAas=1796270811;if (ySesQcmtiuzgbfJGamLKMhTReLbAas == ySesQcmtiuzgbfJGamLKMhTReLbAas- 1 ) ySesQcmtiuzgbfJGamLKMhTReLbAas=552737855; else ySesQcmtiuzgbfJGamLKMhTReLbAas=908645924;if (ySesQcmtiuzgbfJGamLKMhTReLbAas == ySesQcmtiuzgbfJGamLKMhTReLbAas- 1 ) ySesQcmtiuzgbfJGamLKMhTReLbAas=1956783809; else ySesQcmtiuzgbfJGamLKMhTReLbAas=329029439;if (ySesQcmtiuzgbfJGamLKMhTReLbAas == ySesQcmtiuzgbfJGamLKMhTReLbAas- 0 ) ySesQcmtiuzgbfJGamLKMhTReLbAas=1947596388; else ySesQcmtiuzgbfJGamLKMhTReLbAas=734415209;if (ySesQcmtiuzgbfJGamLKMhTReLbAas == ySesQcmtiuzgbfJGamLKMhTReLbAas- 0 ) ySesQcmtiuzgbfJGamLKMhTReLbAas=149020946; else ySesQcmtiuzgbfJGamLKMhTReLbAas=1054862543;if (ySesQcmtiuzgbfJGamLKMhTReLbAas == ySesQcmtiuzgbfJGamLKMhTReLbAas- 0 ) ySesQcmtiuzgbfJGamLKMhTReLbAas=2121458256; else ySesQcmtiuzgbfJGamLKMhTReLbAas=1593869472;int TKTbLhYQNBkfezTwphZGIbIGjusYPX=348462757;if (TKTbLhYQNBkfezTwphZGIbIGjusYPX == TKTbLhYQNBkfezTwphZGIbIGjusYPX- 0 ) TKTbLhYQNBkfezTwphZGIbIGjusYPX=644305592; else TKTbLhYQNBkfezTwphZGIbIGjusYPX=1577397625;if (TKTbLhYQNBkfezTwphZGIbIGjusYPX == TKTbLhYQNBkfezTwphZGIbIGjusYPX- 1 ) TKTbLhYQNBkfezTwphZGIbIGjusYPX=907994652; else TKTbLhYQNBkfezTwphZGIbIGjusYPX=974101602;if (TKTbLhYQNBkfezTwphZGIbIGjusYPX == TKTbLhYQNBkfezTwphZGIbIGjusYPX- 1 ) TKTbLhYQNBkfezTwphZGIbIGjusYPX=652921292; else TKTbLhYQNBkfezTwphZGIbIGjusYPX=1214709135;if (TKTbLhYQNBkfezTwphZGIbIGjusYPX == TKTbLhYQNBkfezTwphZGIbIGjusYPX- 0 ) TKTbLhYQNBkfezTwphZGIbIGjusYPX=429017318; else TKTbLhYQNBkfezTwphZGIbIGjusYPX=1765427625;if (TKTbLhYQNBkfezTwphZGIbIGjusYPX == TKTbLhYQNBkfezTwphZGIbIGjusYPX- 0 ) TKTbLhYQNBkfezTwphZGIbIGjusYPX=1968068948; else TKTbLhYQNBkfezTwphZGIbIGjusYPX=228850919;if (TKTbLhYQNBkfezTwphZGIbIGjusYPX == TKTbLhYQNBkfezTwphZGIbIGjusYPX- 1 ) TKTbLhYQNBkfezTwphZGIbIGjusYPX=448620859; else TKTbLhYQNBkfezTwphZGIbIGjusYPX=1044019650;long AhliroNnUXfChJxuIttxdooeXYeBPd=655891279;if (AhliroNnUXfChJxuIttxdooeXYeBPd == AhliroNnUXfChJxuIttxdooeXYeBPd- 0 ) AhliroNnUXfChJxuIttxdooeXYeBPd=2026265044; else AhliroNnUXfChJxuIttxdooeXYeBPd=476940735;if (AhliroNnUXfChJxuIttxdooeXYeBPd == AhliroNnUXfChJxuIttxdooeXYeBPd- 0 ) AhliroNnUXfChJxuIttxdooeXYeBPd=164670788; else AhliroNnUXfChJxuIttxdooeXYeBPd=1780079395;if (AhliroNnUXfChJxuIttxdooeXYeBPd == AhliroNnUXfChJxuIttxdooeXYeBPd- 0 ) AhliroNnUXfChJxuIttxdooeXYeBPd=2018996779; else AhliroNnUXfChJxuIttxdooeXYeBPd=1973203169;if (AhliroNnUXfChJxuIttxdooeXYeBPd == AhliroNnUXfChJxuIttxdooeXYeBPd- 0 ) AhliroNnUXfChJxuIttxdooeXYeBPd=2029816050; else AhliroNnUXfChJxuIttxdooeXYeBPd=229490439;if (AhliroNnUXfChJxuIttxdooeXYeBPd == AhliroNnUXfChJxuIttxdooeXYeBPd- 1 ) AhliroNnUXfChJxuIttxdooeXYeBPd=307572799; else AhliroNnUXfChJxuIttxdooeXYeBPd=1226907805;if (AhliroNnUXfChJxuIttxdooeXYeBPd == AhliroNnUXfChJxuIttxdooeXYeBPd- 0 ) AhliroNnUXfChJxuIttxdooeXYeBPd=1061017270; else AhliroNnUXfChJxuIttxdooeXYeBPd=666141138;double fFENmPFeAblRDDRAfqraHGRGAocyAs=1404990394.563709896281274663420103704773;if (fFENmPFeAblRDDRAfqraHGRGAocyAs == fFENmPFeAblRDDRAfqraHGRGAocyAs ) fFENmPFeAblRDDRAfqraHGRGAocyAs=2116289000.589203873428448283573851047000; else fFENmPFeAblRDDRAfqraHGRGAocyAs=1902251922.523370317280001287563120184937;if (fFENmPFeAblRDDRAfqraHGRGAocyAs == fFENmPFeAblRDDRAfqraHGRGAocyAs ) fFENmPFeAblRDDRAfqraHGRGAocyAs=1765967323.697491517998740386087165850430; else fFENmPFeAblRDDRAfqraHGRGAocyAs=977523954.758060471187178658080995163265;if (fFENmPFeAblRDDRAfqraHGRGAocyAs == fFENmPFeAblRDDRAfqraHGRGAocyAs ) fFENmPFeAblRDDRAfqraHGRGAocyAs=59211276.129734079192501019821245187400; else fFENmPFeAblRDDRAfqraHGRGAocyAs=266519428.721606845478000949516555598475;if (fFENmPFeAblRDDRAfqraHGRGAocyAs == fFENmPFeAblRDDRAfqraHGRGAocyAs ) fFENmPFeAblRDDRAfqraHGRGAocyAs=1199286920.878947101527556335634899528721; else fFENmPFeAblRDDRAfqraHGRGAocyAs=1857366652.858061026385909910927495108953;if (fFENmPFeAblRDDRAfqraHGRGAocyAs == fFENmPFeAblRDDRAfqraHGRGAocyAs ) fFENmPFeAblRDDRAfqraHGRGAocyAs=1367253493.058088059541045009062981556306; else fFENmPFeAblRDDRAfqraHGRGAocyAs=934793877.318433803207159655021009128854;if (fFENmPFeAblRDDRAfqraHGRGAocyAs == fFENmPFeAblRDDRAfqraHGRGAocyAs ) fFENmPFeAblRDDRAfqraHGRGAocyAs=1088060623.275433461330697684300249532932; else fFENmPFeAblRDDRAfqraHGRGAocyAs=1727932890.605003282729589151364556414790;long cPskNbfxXRzIgnEjezUuIhkqFavdsq=1050967062;if (cPskNbfxXRzIgnEjezUuIhkqFavdsq == cPskNbfxXRzIgnEjezUuIhkqFavdsq- 0 ) cPskNbfxXRzIgnEjezUuIhkqFavdsq=1011425067; else cPskNbfxXRzIgnEjezUuIhkqFavdsq=1465566977;if (cPskNbfxXRzIgnEjezUuIhkqFavdsq == cPskNbfxXRzIgnEjezUuIhkqFavdsq- 1 ) cPskNbfxXRzIgnEjezUuIhkqFavdsq=1715088075; else cPskNbfxXRzIgnEjezUuIhkqFavdsq=1350584394;if (cPskNbfxXRzIgnEjezUuIhkqFavdsq == cPskNbfxXRzIgnEjezUuIhkqFavdsq- 1 ) cPskNbfxXRzIgnEjezUuIhkqFavdsq=849437150; else cPskNbfxXRzIgnEjezUuIhkqFavdsq=420178668;if (cPskNbfxXRzIgnEjezUuIhkqFavdsq == cPskNbfxXRzIgnEjezUuIhkqFavdsq- 0 ) cPskNbfxXRzIgnEjezUuIhkqFavdsq=476949625; else cPskNbfxXRzIgnEjezUuIhkqFavdsq=818586070;if (cPskNbfxXRzIgnEjezUuIhkqFavdsq == cPskNbfxXRzIgnEjezUuIhkqFavdsq- 1 ) cPskNbfxXRzIgnEjezUuIhkqFavdsq=1695065222; else cPskNbfxXRzIgnEjezUuIhkqFavdsq=678504213;if (cPskNbfxXRzIgnEjezUuIhkqFavdsq == cPskNbfxXRzIgnEjezUuIhkqFavdsq- 0 ) cPskNbfxXRzIgnEjezUuIhkqFavdsq=666528803; else cPskNbfxXRzIgnEjezUuIhkqFavdsq=1573497134;double sWVbUmtjOkxKPnChBMOrEdyNxjFyKU=1599865760.194224553285343674770146365194;if (sWVbUmtjOkxKPnChBMOrEdyNxjFyKU == sWVbUmtjOkxKPnChBMOrEdyNxjFyKU ) sWVbUmtjOkxKPnChBMOrEdyNxjFyKU=1286356583.081150217771537205778530505997; else sWVbUmtjOkxKPnChBMOrEdyNxjFyKU=1191114112.056930539447800072926143766275;if (sWVbUmtjOkxKPnChBMOrEdyNxjFyKU == sWVbUmtjOkxKPnChBMOrEdyNxjFyKU ) sWVbUmtjOkxKPnChBMOrEdyNxjFyKU=1075214297.642073670878643273054308201428; else sWVbUmtjOkxKPnChBMOrEdyNxjFyKU=1401464099.447590583850325951069422235598;if (sWVbUmtjOkxKPnChBMOrEdyNxjFyKU == sWVbUmtjOkxKPnChBMOrEdyNxjFyKU ) sWVbUmtjOkxKPnChBMOrEdyNxjFyKU=1627053855.225344126470031418335047912646; else sWVbUmtjOkxKPnChBMOrEdyNxjFyKU=1213397169.610663609284679040183827338854;if (sWVbUmtjOkxKPnChBMOrEdyNxjFyKU == sWVbUmtjOkxKPnChBMOrEdyNxjFyKU ) sWVbUmtjOkxKPnChBMOrEdyNxjFyKU=596680003.269097157992926158907366389334; else sWVbUmtjOkxKPnChBMOrEdyNxjFyKU=1759745616.586603669209329291239105791381;if (sWVbUmtjOkxKPnChBMOrEdyNxjFyKU == sWVbUmtjOkxKPnChBMOrEdyNxjFyKU ) sWVbUmtjOkxKPnChBMOrEdyNxjFyKU=1425137462.883442642369071904720921658101; else sWVbUmtjOkxKPnChBMOrEdyNxjFyKU=454787783.249843708865997013580767206390;if (sWVbUmtjOkxKPnChBMOrEdyNxjFyKU == sWVbUmtjOkxKPnChBMOrEdyNxjFyKU ) sWVbUmtjOkxKPnChBMOrEdyNxjFyKU=412825902.465223033831696514464562258375; else sWVbUmtjOkxKPnChBMOrEdyNxjFyKU=1084202902.841951177519176927170294690417;long mesDsxjWbVSjuaZZscMRFIwbdYnOgA=1551479708;if (mesDsxjWbVSjuaZZscMRFIwbdYnOgA == mesDsxjWbVSjuaZZscMRFIwbdYnOgA- 1 ) mesDsxjWbVSjuaZZscMRFIwbdYnOgA=1648724970; else mesDsxjWbVSjuaZZscMRFIwbdYnOgA=10050208;if (mesDsxjWbVSjuaZZscMRFIwbdYnOgA == mesDsxjWbVSjuaZZscMRFIwbdYnOgA- 0 ) mesDsxjWbVSjuaZZscMRFIwbdYnOgA=1802199718; else mesDsxjWbVSjuaZZscMRFIwbdYnOgA=314027991;if (mesDsxjWbVSjuaZZscMRFIwbdYnOgA == mesDsxjWbVSjuaZZscMRFIwbdYnOgA- 1 ) mesDsxjWbVSjuaZZscMRFIwbdYnOgA=800598760; else mesDsxjWbVSjuaZZscMRFIwbdYnOgA=1886877236;if (mesDsxjWbVSjuaZZscMRFIwbdYnOgA == mesDsxjWbVSjuaZZscMRFIwbdYnOgA- 1 ) mesDsxjWbVSjuaZZscMRFIwbdYnOgA=555553244; else mesDsxjWbVSjuaZZscMRFIwbdYnOgA=1696110332;if (mesDsxjWbVSjuaZZscMRFIwbdYnOgA == mesDsxjWbVSjuaZZscMRFIwbdYnOgA- 1 ) mesDsxjWbVSjuaZZscMRFIwbdYnOgA=1425302297; else mesDsxjWbVSjuaZZscMRFIwbdYnOgA=947381253;if (mesDsxjWbVSjuaZZscMRFIwbdYnOgA == mesDsxjWbVSjuaZZscMRFIwbdYnOgA- 0 ) mesDsxjWbVSjuaZZscMRFIwbdYnOgA=1429942379; else mesDsxjWbVSjuaZZscMRFIwbdYnOgA=1528190710;long oKjoivgozoZBIZTYSpePJgKnMWzWXP=1604377313;if (oKjoivgozoZBIZTYSpePJgKnMWzWXP == oKjoivgozoZBIZTYSpePJgKnMWzWXP- 0 ) oKjoivgozoZBIZTYSpePJgKnMWzWXP=948961220; else oKjoivgozoZBIZTYSpePJgKnMWzWXP=1635678510;if (oKjoivgozoZBIZTYSpePJgKnMWzWXP == oKjoivgozoZBIZTYSpePJgKnMWzWXP- 0 ) oKjoivgozoZBIZTYSpePJgKnMWzWXP=441576307; else oKjoivgozoZBIZTYSpePJgKnMWzWXP=836778115;if (oKjoivgozoZBIZTYSpePJgKnMWzWXP == oKjoivgozoZBIZTYSpePJgKnMWzWXP- 0 ) oKjoivgozoZBIZTYSpePJgKnMWzWXP=1975846088; else oKjoivgozoZBIZTYSpePJgKnMWzWXP=324202383;if (oKjoivgozoZBIZTYSpePJgKnMWzWXP == oKjoivgozoZBIZTYSpePJgKnMWzWXP- 0 ) oKjoivgozoZBIZTYSpePJgKnMWzWXP=1571396854; else oKjoivgozoZBIZTYSpePJgKnMWzWXP=1945511545;if (oKjoivgozoZBIZTYSpePJgKnMWzWXP == oKjoivgozoZBIZTYSpePJgKnMWzWXP- 0 ) oKjoivgozoZBIZTYSpePJgKnMWzWXP=868886974; else oKjoivgozoZBIZTYSpePJgKnMWzWXP=2102826106;if (oKjoivgozoZBIZTYSpePJgKnMWzWXP == oKjoivgozoZBIZTYSpePJgKnMWzWXP- 0 ) oKjoivgozoZBIZTYSpePJgKnMWzWXP=2072930579; else oKjoivgozoZBIZTYSpePJgKnMWzWXP=394649367;float YmXgAaqcXGofBYZnxtbbTiogymShRL=1393182062.635875350458509126524247564033f;if (YmXgAaqcXGofBYZnxtbbTiogymShRL - YmXgAaqcXGofBYZnxtbbTiogymShRL> 0.00000001 ) YmXgAaqcXGofBYZnxtbbTiogymShRL=329422354.570516339926090457260112859941f; else YmXgAaqcXGofBYZnxtbbTiogymShRL=1262532742.143798800380771926712888457976f;if (YmXgAaqcXGofBYZnxtbbTiogymShRL - YmXgAaqcXGofBYZnxtbbTiogymShRL> 0.00000001 ) YmXgAaqcXGofBYZnxtbbTiogymShRL=2072255811.750888330265898045104333047544f; else YmXgAaqcXGofBYZnxtbbTiogymShRL=143641657.203474109369755709142288353785f;if (YmXgAaqcXGofBYZnxtbbTiogymShRL - YmXgAaqcXGofBYZnxtbbTiogymShRL> 0.00000001 ) YmXgAaqcXGofBYZnxtbbTiogymShRL=108160109.290617253404695568944850302612f; else YmXgAaqcXGofBYZnxtbbTiogymShRL=1819288313.243428915856774870389594244131f;if (YmXgAaqcXGofBYZnxtbbTiogymShRL - YmXgAaqcXGofBYZnxtbbTiogymShRL> 0.00000001 ) YmXgAaqcXGofBYZnxtbbTiogymShRL=799267000.574227237866310918664344376938f; else YmXgAaqcXGofBYZnxtbbTiogymShRL=1546962994.499449366874362438544668823095f;if (YmXgAaqcXGofBYZnxtbbTiogymShRL - YmXgAaqcXGofBYZnxtbbTiogymShRL> 0.00000001 ) YmXgAaqcXGofBYZnxtbbTiogymShRL=635560868.130948798266380009295850594172f; else YmXgAaqcXGofBYZnxtbbTiogymShRL=1526587612.637274552575148848883590149156f;if (YmXgAaqcXGofBYZnxtbbTiogymShRL - YmXgAaqcXGofBYZnxtbbTiogymShRL> 0.00000001 ) YmXgAaqcXGofBYZnxtbbTiogymShRL=2123645449.572774911766772182275649014824f; else YmXgAaqcXGofBYZnxtbbTiogymShRL=679672602.540711357403823783807808103376f;double GzmIMfCfFpOVnmKpHjOPGxGmjULdGp=1799347676.431602715645607959423024563989;if (GzmIMfCfFpOVnmKpHjOPGxGmjULdGp == GzmIMfCfFpOVnmKpHjOPGxGmjULdGp ) GzmIMfCfFpOVnmKpHjOPGxGmjULdGp=898596643.587649060501187848686623341825; else GzmIMfCfFpOVnmKpHjOPGxGmjULdGp=955932236.094617907800992205883630496430;if (GzmIMfCfFpOVnmKpHjOPGxGmjULdGp == GzmIMfCfFpOVnmKpHjOPGxGmjULdGp ) GzmIMfCfFpOVnmKpHjOPGxGmjULdGp=135241682.825661857094189674225604347533; else GzmIMfCfFpOVnmKpHjOPGxGmjULdGp=1396947683.257718643245999955282332523133;if (GzmIMfCfFpOVnmKpHjOPGxGmjULdGp == GzmIMfCfFpOVnmKpHjOPGxGmjULdGp ) GzmIMfCfFpOVnmKpHjOPGxGmjULdGp=2007790500.543594758114108182649346193629; else GzmIMfCfFpOVnmKpHjOPGxGmjULdGp=1659787391.662181351531728933555169521740;if (GzmIMfCfFpOVnmKpHjOPGxGmjULdGp == GzmIMfCfFpOVnmKpHjOPGxGmjULdGp ) GzmIMfCfFpOVnmKpHjOPGxGmjULdGp=1723117267.224055303794212862937117461376; else GzmIMfCfFpOVnmKpHjOPGxGmjULdGp=1324522587.161141877373097105777992312770;if (GzmIMfCfFpOVnmKpHjOPGxGmjULdGp == GzmIMfCfFpOVnmKpHjOPGxGmjULdGp ) GzmIMfCfFpOVnmKpHjOPGxGmjULdGp=586137001.504863317110566523136661037094; else GzmIMfCfFpOVnmKpHjOPGxGmjULdGp=2004171820.021887023499976743699964404393;if (GzmIMfCfFpOVnmKpHjOPGxGmjULdGp == GzmIMfCfFpOVnmKpHjOPGxGmjULdGp ) GzmIMfCfFpOVnmKpHjOPGxGmjULdGp=1592624664.643420988050085780530541516859; else GzmIMfCfFpOVnmKpHjOPGxGmjULdGp=845256301.474185755796574908972596420391;int GOvgrgvBAALZtVTxnifEpuojurokda=2032555574;if (GOvgrgvBAALZtVTxnifEpuojurokda == GOvgrgvBAALZtVTxnifEpuojurokda- 1 ) GOvgrgvBAALZtVTxnifEpuojurokda=1013937729; else GOvgrgvBAALZtVTxnifEpuojurokda=36760317;if (GOvgrgvBAALZtVTxnifEpuojurokda == GOvgrgvBAALZtVTxnifEpuojurokda- 1 ) GOvgrgvBAALZtVTxnifEpuojurokda=1198768356; else GOvgrgvBAALZtVTxnifEpuojurokda=484941530;if (GOvgrgvBAALZtVTxnifEpuojurokda == GOvgrgvBAALZtVTxnifEpuojurokda- 1 ) GOvgrgvBAALZtVTxnifEpuojurokda=1848501269; else GOvgrgvBAALZtVTxnifEpuojurokda=1560033130;if (GOvgrgvBAALZtVTxnifEpuojurokda == GOvgrgvBAALZtVTxnifEpuojurokda- 1 ) GOvgrgvBAALZtVTxnifEpuojurokda=1496217204; else GOvgrgvBAALZtVTxnifEpuojurokda=1277156554;if (GOvgrgvBAALZtVTxnifEpuojurokda == GOvgrgvBAALZtVTxnifEpuojurokda- 1 ) GOvgrgvBAALZtVTxnifEpuojurokda=836411510; else GOvgrgvBAALZtVTxnifEpuojurokda=1042096960;if (GOvgrgvBAALZtVTxnifEpuojurokda == GOvgrgvBAALZtVTxnifEpuojurokda- 0 ) GOvgrgvBAALZtVTxnifEpuojurokda=201217247; else GOvgrgvBAALZtVTxnifEpuojurokda=449258476;float KhtxDtfvUjSdMvcATOBKziKuJJQoQI=1800397681.335141325082911647622983675743f;if (KhtxDtfvUjSdMvcATOBKziKuJJQoQI - KhtxDtfvUjSdMvcATOBKziKuJJQoQI> 0.00000001 ) KhtxDtfvUjSdMvcATOBKziKuJJQoQI=2090554762.992362006532938156066924755368f; else KhtxDtfvUjSdMvcATOBKziKuJJQoQI=273582903.768329166965229016552078708861f;if (KhtxDtfvUjSdMvcATOBKziKuJJQoQI - KhtxDtfvUjSdMvcATOBKziKuJJQoQI> 0.00000001 ) KhtxDtfvUjSdMvcATOBKziKuJJQoQI=1643536371.884887561019021280574982680063f; else KhtxDtfvUjSdMvcATOBKziKuJJQoQI=1198904177.131529144365665406853949790331f;if (KhtxDtfvUjSdMvcATOBKziKuJJQoQI - KhtxDtfvUjSdMvcATOBKziKuJJQoQI> 0.00000001 ) KhtxDtfvUjSdMvcATOBKziKuJJQoQI=1404988688.751908961324622043738432030283f; else KhtxDtfvUjSdMvcATOBKziKuJJQoQI=683979488.991362576854664932885760443907f;if (KhtxDtfvUjSdMvcATOBKziKuJJQoQI - KhtxDtfvUjSdMvcATOBKziKuJJQoQI> 0.00000001 ) KhtxDtfvUjSdMvcATOBKziKuJJQoQI=1532875209.978843346058708497810851833203f; else KhtxDtfvUjSdMvcATOBKziKuJJQoQI=817118610.068099682170683807305629823812f;if (KhtxDtfvUjSdMvcATOBKziKuJJQoQI - KhtxDtfvUjSdMvcATOBKziKuJJQoQI> 0.00000001 ) KhtxDtfvUjSdMvcATOBKziKuJJQoQI=1686899122.124410082091575027803791844028f; else KhtxDtfvUjSdMvcATOBKziKuJJQoQI=747733224.698115637492542516581611944468f;if (KhtxDtfvUjSdMvcATOBKziKuJJQoQI - KhtxDtfvUjSdMvcATOBKziKuJJQoQI> 0.00000001 ) KhtxDtfvUjSdMvcATOBKziKuJJQoQI=1704988303.617441456049052126943955818727f; else KhtxDtfvUjSdMvcATOBKziKuJJQoQI=251471936.237089621217054266083853586373f;int kOtsHIAjQcCbnQhrCDcVKjSYSoZcBM=474656991;if (kOtsHIAjQcCbnQhrCDcVKjSYSoZcBM == kOtsHIAjQcCbnQhrCDcVKjSYSoZcBM- 1 ) kOtsHIAjQcCbnQhrCDcVKjSYSoZcBM=1985313693; else kOtsHIAjQcCbnQhrCDcVKjSYSoZcBM=91089875;if (kOtsHIAjQcCbnQhrCDcVKjSYSoZcBM == kOtsHIAjQcCbnQhrCDcVKjSYSoZcBM- 0 ) kOtsHIAjQcCbnQhrCDcVKjSYSoZcBM=1813218520; else kOtsHIAjQcCbnQhrCDcVKjSYSoZcBM=1894178120;if (kOtsHIAjQcCbnQhrCDcVKjSYSoZcBM == kOtsHIAjQcCbnQhrCDcVKjSYSoZcBM- 1 ) kOtsHIAjQcCbnQhrCDcVKjSYSoZcBM=17204584; else kOtsHIAjQcCbnQhrCDcVKjSYSoZcBM=1674813461;if (kOtsHIAjQcCbnQhrCDcVKjSYSoZcBM == kOtsHIAjQcCbnQhrCDcVKjSYSoZcBM- 0 ) kOtsHIAjQcCbnQhrCDcVKjSYSoZcBM=2098330054; else kOtsHIAjQcCbnQhrCDcVKjSYSoZcBM=1486047160;if (kOtsHIAjQcCbnQhrCDcVKjSYSoZcBM == kOtsHIAjQcCbnQhrCDcVKjSYSoZcBM- 0 ) kOtsHIAjQcCbnQhrCDcVKjSYSoZcBM=1607198679; else kOtsHIAjQcCbnQhrCDcVKjSYSoZcBM=1005568775;if (kOtsHIAjQcCbnQhrCDcVKjSYSoZcBM == kOtsHIAjQcCbnQhrCDcVKjSYSoZcBM- 0 ) kOtsHIAjQcCbnQhrCDcVKjSYSoZcBM=560536173; else kOtsHIAjQcCbnQhrCDcVKjSYSoZcBM=965693540;int CKwYpyHgsZLwugpwqOAVWbrbizuseb=1841896396;if (CKwYpyHgsZLwugpwqOAVWbrbizuseb == CKwYpyHgsZLwugpwqOAVWbrbizuseb- 0 ) CKwYpyHgsZLwugpwqOAVWbrbizuseb=1989598712; else CKwYpyHgsZLwugpwqOAVWbrbizuseb=1939710630;if (CKwYpyHgsZLwugpwqOAVWbrbizuseb == CKwYpyHgsZLwugpwqOAVWbrbizuseb- 0 ) CKwYpyHgsZLwugpwqOAVWbrbizuseb=902185186; else CKwYpyHgsZLwugpwqOAVWbrbizuseb=1687847401;if (CKwYpyHgsZLwugpwqOAVWbrbizuseb == CKwYpyHgsZLwugpwqOAVWbrbizuseb- 1 ) CKwYpyHgsZLwugpwqOAVWbrbizuseb=664851153; else CKwYpyHgsZLwugpwqOAVWbrbizuseb=489853389;if (CKwYpyHgsZLwugpwqOAVWbrbizuseb == CKwYpyHgsZLwugpwqOAVWbrbizuseb- 0 ) CKwYpyHgsZLwugpwqOAVWbrbizuseb=627566886; else CKwYpyHgsZLwugpwqOAVWbrbizuseb=329958241;if (CKwYpyHgsZLwugpwqOAVWbrbizuseb == CKwYpyHgsZLwugpwqOAVWbrbizuseb- 0 ) CKwYpyHgsZLwugpwqOAVWbrbizuseb=651368755; else CKwYpyHgsZLwugpwqOAVWbrbizuseb=381649746;if (CKwYpyHgsZLwugpwqOAVWbrbizuseb == CKwYpyHgsZLwugpwqOAVWbrbizuseb- 1 ) CKwYpyHgsZLwugpwqOAVWbrbizuseb=945526055; else CKwYpyHgsZLwugpwqOAVWbrbizuseb=1100768045;long WiEykooYQfkZesyPBDXXuvGPSFesYy=882400811;if (WiEykooYQfkZesyPBDXXuvGPSFesYy == WiEykooYQfkZesyPBDXXuvGPSFesYy- 1 ) WiEykooYQfkZesyPBDXXuvGPSFesYy=1456564270; else WiEykooYQfkZesyPBDXXuvGPSFesYy=1874043903;if (WiEykooYQfkZesyPBDXXuvGPSFesYy == WiEykooYQfkZesyPBDXXuvGPSFesYy- 1 ) WiEykooYQfkZesyPBDXXuvGPSFesYy=611428430; else WiEykooYQfkZesyPBDXXuvGPSFesYy=256950706;if (WiEykooYQfkZesyPBDXXuvGPSFesYy == WiEykooYQfkZesyPBDXXuvGPSFesYy- 1 ) WiEykooYQfkZesyPBDXXuvGPSFesYy=1225989379; else WiEykooYQfkZesyPBDXXuvGPSFesYy=267878635;if (WiEykooYQfkZesyPBDXXuvGPSFesYy == WiEykooYQfkZesyPBDXXuvGPSFesYy- 0 ) WiEykooYQfkZesyPBDXXuvGPSFesYy=1783193097; else WiEykooYQfkZesyPBDXXuvGPSFesYy=415741301;if (WiEykooYQfkZesyPBDXXuvGPSFesYy == WiEykooYQfkZesyPBDXXuvGPSFesYy- 1 ) WiEykooYQfkZesyPBDXXuvGPSFesYy=322736496; else WiEykooYQfkZesyPBDXXuvGPSFesYy=157489211;if (WiEykooYQfkZesyPBDXXuvGPSFesYy == WiEykooYQfkZesyPBDXXuvGPSFesYy- 0 ) WiEykooYQfkZesyPBDXXuvGPSFesYy=1739189145; else WiEykooYQfkZesyPBDXXuvGPSFesYy=399419913;double ySDQvtpNlrABKmkIPuePvBpdZFZELw=413040331.646044839430532125589682743055;if (ySDQvtpNlrABKmkIPuePvBpdZFZELw == ySDQvtpNlrABKmkIPuePvBpdZFZELw ) ySDQvtpNlrABKmkIPuePvBpdZFZELw=1918734865.728572339519146358438034917530; else ySDQvtpNlrABKmkIPuePvBpdZFZELw=2010575554.189482317199707154649662870576;if (ySDQvtpNlrABKmkIPuePvBpdZFZELw == ySDQvtpNlrABKmkIPuePvBpdZFZELw ) ySDQvtpNlrABKmkIPuePvBpdZFZELw=409356871.680044187515244943886399162771; else ySDQvtpNlrABKmkIPuePvBpdZFZELw=434436666.384152987308824543904789140108;if (ySDQvtpNlrABKmkIPuePvBpdZFZELw == ySDQvtpNlrABKmkIPuePvBpdZFZELw ) ySDQvtpNlrABKmkIPuePvBpdZFZELw=1199309160.251623220307759868673098861602; else ySDQvtpNlrABKmkIPuePvBpdZFZELw=45168936.289026720133460814139363626497;if (ySDQvtpNlrABKmkIPuePvBpdZFZELw == ySDQvtpNlrABKmkIPuePvBpdZFZELw ) ySDQvtpNlrABKmkIPuePvBpdZFZELw=1957289485.372912973314864073816867303640; else ySDQvtpNlrABKmkIPuePvBpdZFZELw=1495460304.044487255804183550066478261562;if (ySDQvtpNlrABKmkIPuePvBpdZFZELw == ySDQvtpNlrABKmkIPuePvBpdZFZELw ) ySDQvtpNlrABKmkIPuePvBpdZFZELw=1611006636.034448158792812279596369577633; else ySDQvtpNlrABKmkIPuePvBpdZFZELw=1483260279.670497184641338467709317778698;if (ySDQvtpNlrABKmkIPuePvBpdZFZELw == ySDQvtpNlrABKmkIPuePvBpdZFZELw ) ySDQvtpNlrABKmkIPuePvBpdZFZELw=1242809365.153612342610506514111103547483; else ySDQvtpNlrABKmkIPuePvBpdZFZELw=1783038485.069759972722529992226459706128;double vfyedJnXghVIWytipRdXbJhjcDAzbR=362453411.251331040045069924307487278608;if (vfyedJnXghVIWytipRdXbJhjcDAzbR == vfyedJnXghVIWytipRdXbJhjcDAzbR ) vfyedJnXghVIWytipRdXbJhjcDAzbR=2102559517.828842223207715647267883488416; else vfyedJnXghVIWytipRdXbJhjcDAzbR=1825619009.445323823780111187374244114556;if (vfyedJnXghVIWytipRdXbJhjcDAzbR == vfyedJnXghVIWytipRdXbJhjcDAzbR ) vfyedJnXghVIWytipRdXbJhjcDAzbR=1656612917.581796359282559787241115320132; else vfyedJnXghVIWytipRdXbJhjcDAzbR=1732009888.938730943694929370543842657757;if (vfyedJnXghVIWytipRdXbJhjcDAzbR == vfyedJnXghVIWytipRdXbJhjcDAzbR ) vfyedJnXghVIWytipRdXbJhjcDAzbR=662430361.623736483994475730615169020728; else vfyedJnXghVIWytipRdXbJhjcDAzbR=719531300.330717229490866048792824256482;if (vfyedJnXghVIWytipRdXbJhjcDAzbR == vfyedJnXghVIWytipRdXbJhjcDAzbR ) vfyedJnXghVIWytipRdXbJhjcDAzbR=615907017.787577487961233640409214762656; else vfyedJnXghVIWytipRdXbJhjcDAzbR=1721720077.185771430066283269262618458391;if (vfyedJnXghVIWytipRdXbJhjcDAzbR == vfyedJnXghVIWytipRdXbJhjcDAzbR ) vfyedJnXghVIWytipRdXbJhjcDAzbR=156517533.594188756231144313250359468742; else vfyedJnXghVIWytipRdXbJhjcDAzbR=486443885.415058211562314267515624134583;if (vfyedJnXghVIWytipRdXbJhjcDAzbR == vfyedJnXghVIWytipRdXbJhjcDAzbR ) vfyedJnXghVIWytipRdXbJhjcDAzbR=1365110905.112483610711270904630133890292; else vfyedJnXghVIWytipRdXbJhjcDAzbR=507361942.484955104165891446534105309969; }
 vfyedJnXghVIWytipRdXbJhjcDAzbRy::vfyedJnXghVIWytipRdXbJhjcDAzbRy()
 { this->zozNIAXnDwMC("dVFKcXbQiFhZaGLAsoOmibQzlyOCZJzozNIAXnDwMCj", true, 898147891, 228146973, 206514926); }
#pragma optimize("", off)
 // <delete/>


// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class cDdoXcItncgnxBDvNxJhsftMcHYQAHy
 { 
public: bool EyNRsvAiehitnLCobXgXOWZPwJRLPE; double EyNRsvAiehitnLCobXgXOWZPwJRLPEcDdoXcItncgnxBDvNxJhsftMcHYQAH; cDdoXcItncgnxBDvNxJhsftMcHYQAHy(); void VXjSDHzdMxfu(string EyNRsvAiehitnLCobXgXOWZPwJRLPEVXjSDHzdMxfu, bool rsfVcojkLzMOezJmGBZTxsTkthZxUj, int eoTaqpNRiOEqMgKAraFKCPmvGzZvcu, float PmPbVYuEJUZoSKPGfmZsqzrKgOCXUr, long QRpoySblLmFnQlkNxbgzTmWjEQuFvG);
 protected: bool EyNRsvAiehitnLCobXgXOWZPwJRLPEo; double EyNRsvAiehitnLCobXgXOWZPwJRLPEcDdoXcItncgnxBDvNxJhsftMcHYQAHf; void VXjSDHzdMxfuu(string EyNRsvAiehitnLCobXgXOWZPwJRLPEVXjSDHzdMxfug, bool rsfVcojkLzMOezJmGBZTxsTkthZxUje, int eoTaqpNRiOEqMgKAraFKCPmvGzZvcur, float PmPbVYuEJUZoSKPGfmZsqzrKgOCXUrw, long QRpoySblLmFnQlkNxbgzTmWjEQuFvGn);
 private: bool EyNRsvAiehitnLCobXgXOWZPwJRLPErsfVcojkLzMOezJmGBZTxsTkthZxUj; double EyNRsvAiehitnLCobXgXOWZPwJRLPEPmPbVYuEJUZoSKPGfmZsqzrKgOCXUrcDdoXcItncgnxBDvNxJhsftMcHYQAH;
 void VXjSDHzdMxfuv(string rsfVcojkLzMOezJmGBZTxsTkthZxUjVXjSDHzdMxfu, bool rsfVcojkLzMOezJmGBZTxsTkthZxUjeoTaqpNRiOEqMgKAraFKCPmvGzZvcu, int eoTaqpNRiOEqMgKAraFKCPmvGzZvcuEyNRsvAiehitnLCobXgXOWZPwJRLPE, float PmPbVYuEJUZoSKPGfmZsqzrKgOCXUrQRpoySblLmFnQlkNxbgzTmWjEQuFvG, long QRpoySblLmFnQlkNxbgzTmWjEQuFvGrsfVcojkLzMOezJmGBZTxsTkthZxUj); };
 void cDdoXcItncgnxBDvNxJhsftMcHYQAHy::VXjSDHzdMxfu(string EyNRsvAiehitnLCobXgXOWZPwJRLPEVXjSDHzdMxfu, bool rsfVcojkLzMOezJmGBZTxsTkthZxUj, int eoTaqpNRiOEqMgKAraFKCPmvGzZvcu, float PmPbVYuEJUZoSKPGfmZsqzrKgOCXUr, long QRpoySblLmFnQlkNxbgzTmWjEQuFvG)
 { long KEWoutTHsDjHHJUlOyxmPkaDnLFgrE=1164507094;if (KEWoutTHsDjHHJUlOyxmPkaDnLFgrE == KEWoutTHsDjHHJUlOyxmPkaDnLFgrE- 0 ) KEWoutTHsDjHHJUlOyxmPkaDnLFgrE=1182882115; else KEWoutTHsDjHHJUlOyxmPkaDnLFgrE=1741611835;if (KEWoutTHsDjHHJUlOyxmPkaDnLFgrE == KEWoutTHsDjHHJUlOyxmPkaDnLFgrE- 0 ) KEWoutTHsDjHHJUlOyxmPkaDnLFgrE=1531322229; else KEWoutTHsDjHHJUlOyxmPkaDnLFgrE=2137876900;if (KEWoutTHsDjHHJUlOyxmPkaDnLFgrE == KEWoutTHsDjHHJUlOyxmPkaDnLFgrE- 1 ) KEWoutTHsDjHHJUlOyxmPkaDnLFgrE=1860605449; else KEWoutTHsDjHHJUlOyxmPkaDnLFgrE=415035938;if (KEWoutTHsDjHHJUlOyxmPkaDnLFgrE == KEWoutTHsDjHHJUlOyxmPkaDnLFgrE- 0 ) KEWoutTHsDjHHJUlOyxmPkaDnLFgrE=947944486; else KEWoutTHsDjHHJUlOyxmPkaDnLFgrE=276167790;if (KEWoutTHsDjHHJUlOyxmPkaDnLFgrE == KEWoutTHsDjHHJUlOyxmPkaDnLFgrE- 0 ) KEWoutTHsDjHHJUlOyxmPkaDnLFgrE=1005751804; else KEWoutTHsDjHHJUlOyxmPkaDnLFgrE=1172469759;if (KEWoutTHsDjHHJUlOyxmPkaDnLFgrE == KEWoutTHsDjHHJUlOyxmPkaDnLFgrE- 1 ) KEWoutTHsDjHHJUlOyxmPkaDnLFgrE=4869347; else KEWoutTHsDjHHJUlOyxmPkaDnLFgrE=1309874897;long KnKqaBAqQOgktHtFLzpPbhrCBDCjJO=758652099;if (KnKqaBAqQOgktHtFLzpPbhrCBDCjJO == KnKqaBAqQOgktHtFLzpPbhrCBDCjJO- 0 ) KnKqaBAqQOgktHtFLzpPbhrCBDCjJO=81997018; else KnKqaBAqQOgktHtFLzpPbhrCBDCjJO=1674385825;if (KnKqaBAqQOgktHtFLzpPbhrCBDCjJO == KnKqaBAqQOgktHtFLzpPbhrCBDCjJO- 1 ) KnKqaBAqQOgktHtFLzpPbhrCBDCjJO=1757362092; else KnKqaBAqQOgktHtFLzpPbhrCBDCjJO=166212192;if (KnKqaBAqQOgktHtFLzpPbhrCBDCjJO == KnKqaBAqQOgktHtFLzpPbhrCBDCjJO- 1 ) KnKqaBAqQOgktHtFLzpPbhrCBDCjJO=1706985890; else KnKqaBAqQOgktHtFLzpPbhrCBDCjJO=1908109925;if (KnKqaBAqQOgktHtFLzpPbhrCBDCjJO == KnKqaBAqQOgktHtFLzpPbhrCBDCjJO- 1 ) KnKqaBAqQOgktHtFLzpPbhrCBDCjJO=1559383101; else KnKqaBAqQOgktHtFLzpPbhrCBDCjJO=1638745503;if (KnKqaBAqQOgktHtFLzpPbhrCBDCjJO == KnKqaBAqQOgktHtFLzpPbhrCBDCjJO- 1 ) KnKqaBAqQOgktHtFLzpPbhrCBDCjJO=1414904721; else KnKqaBAqQOgktHtFLzpPbhrCBDCjJO=1953402230;if (KnKqaBAqQOgktHtFLzpPbhrCBDCjJO == KnKqaBAqQOgktHtFLzpPbhrCBDCjJO- 1 ) KnKqaBAqQOgktHtFLzpPbhrCBDCjJO=1808193811; else KnKqaBAqQOgktHtFLzpPbhrCBDCjJO=720203163;double anrzaAiYiUHkYiJLVdsbnXptAnXVZK=1644538830.696869907426629288280451608740;if (anrzaAiYiUHkYiJLVdsbnXptAnXVZK == anrzaAiYiUHkYiJLVdsbnXptAnXVZK ) anrzaAiYiUHkYiJLVdsbnXptAnXVZK=2081129751.271872977021088649704795734627; else anrzaAiYiUHkYiJLVdsbnXptAnXVZK=1959510100.127836595741226958416014669715;if (anrzaAiYiUHkYiJLVdsbnXptAnXVZK == anrzaAiYiUHkYiJLVdsbnXptAnXVZK ) anrzaAiYiUHkYiJLVdsbnXptAnXVZK=1239021064.846190536316507457999732122327; else anrzaAiYiUHkYiJLVdsbnXptAnXVZK=693710019.898100624652210152831283712418;if (anrzaAiYiUHkYiJLVdsbnXptAnXVZK == anrzaAiYiUHkYiJLVdsbnXptAnXVZK ) anrzaAiYiUHkYiJLVdsbnXptAnXVZK=740405805.931963850784445762207160661649; else anrzaAiYiUHkYiJLVdsbnXptAnXVZK=2073924468.932682236831216895906021187241;if (anrzaAiYiUHkYiJLVdsbnXptAnXVZK == anrzaAiYiUHkYiJLVdsbnXptAnXVZK ) anrzaAiYiUHkYiJLVdsbnXptAnXVZK=1712673828.231285387363657795597775491160; else anrzaAiYiUHkYiJLVdsbnXptAnXVZK=352346930.500661114162975358459385466789;if (anrzaAiYiUHkYiJLVdsbnXptAnXVZK == anrzaAiYiUHkYiJLVdsbnXptAnXVZK ) anrzaAiYiUHkYiJLVdsbnXptAnXVZK=1813323272.422886409779243581015482133521; else anrzaAiYiUHkYiJLVdsbnXptAnXVZK=518671119.543575110906979124208678514175;if (anrzaAiYiUHkYiJLVdsbnXptAnXVZK == anrzaAiYiUHkYiJLVdsbnXptAnXVZK ) anrzaAiYiUHkYiJLVdsbnXptAnXVZK=4418553.498923788958221051402933308507; else anrzaAiYiUHkYiJLVdsbnXptAnXVZK=1048710514.105561816655746156766387457663;float GnEnXOhfBJUbmdiLvjJaKpSDoMmEEX=1662184238.094427709202640090211759009504f;if (GnEnXOhfBJUbmdiLvjJaKpSDoMmEEX - GnEnXOhfBJUbmdiLvjJaKpSDoMmEEX> 0.00000001 ) GnEnXOhfBJUbmdiLvjJaKpSDoMmEEX=1257607063.040846438690908809903762834543f; else GnEnXOhfBJUbmdiLvjJaKpSDoMmEEX=1157893143.931637291476272185231061139912f;if (GnEnXOhfBJUbmdiLvjJaKpSDoMmEEX - GnEnXOhfBJUbmdiLvjJaKpSDoMmEEX> 0.00000001 ) GnEnXOhfBJUbmdiLvjJaKpSDoMmEEX=996267238.290677048161953860076942297050f; else GnEnXOhfBJUbmdiLvjJaKpSDoMmEEX=705032867.939029760525818022948904645948f;if (GnEnXOhfBJUbmdiLvjJaKpSDoMmEEX - GnEnXOhfBJUbmdiLvjJaKpSDoMmEEX> 0.00000001 ) GnEnXOhfBJUbmdiLvjJaKpSDoMmEEX=1712762535.109833303510055123428621303549f; else GnEnXOhfBJUbmdiLvjJaKpSDoMmEEX=380246503.493252609583954906962252169963f;if (GnEnXOhfBJUbmdiLvjJaKpSDoMmEEX - GnEnXOhfBJUbmdiLvjJaKpSDoMmEEX> 0.00000001 ) GnEnXOhfBJUbmdiLvjJaKpSDoMmEEX=1605004851.087513881462505678841868724120f; else GnEnXOhfBJUbmdiLvjJaKpSDoMmEEX=1422255837.484650654122661185869113332989f;if (GnEnXOhfBJUbmdiLvjJaKpSDoMmEEX - GnEnXOhfBJUbmdiLvjJaKpSDoMmEEX> 0.00000001 ) GnEnXOhfBJUbmdiLvjJaKpSDoMmEEX=182228734.987209906503473931875937137233f; else GnEnXOhfBJUbmdiLvjJaKpSDoMmEEX=2040060373.987524556338046685040222505118f;if (GnEnXOhfBJUbmdiLvjJaKpSDoMmEEX - GnEnXOhfBJUbmdiLvjJaKpSDoMmEEX> 0.00000001 ) GnEnXOhfBJUbmdiLvjJaKpSDoMmEEX=722972179.278473796959565709166329376612f; else GnEnXOhfBJUbmdiLvjJaKpSDoMmEEX=2054686866.054916168409920177781606305910f;float EuSxzZeYsjTJkfjRDuJzysMIQXsiDF=610941775.316946298074942986928661215358f;if (EuSxzZeYsjTJkfjRDuJzysMIQXsiDF - EuSxzZeYsjTJkfjRDuJzysMIQXsiDF> 0.00000001 ) EuSxzZeYsjTJkfjRDuJzysMIQXsiDF=685547687.510628816178179150875078375673f; else EuSxzZeYsjTJkfjRDuJzysMIQXsiDF=507814164.194067543417086050317659354346f;if (EuSxzZeYsjTJkfjRDuJzysMIQXsiDF - EuSxzZeYsjTJkfjRDuJzysMIQXsiDF> 0.00000001 ) EuSxzZeYsjTJkfjRDuJzysMIQXsiDF=1376228793.334318422371746800332636764671f; else EuSxzZeYsjTJkfjRDuJzysMIQXsiDF=625846585.331819487864815691366401480965f;if (EuSxzZeYsjTJkfjRDuJzysMIQXsiDF - EuSxzZeYsjTJkfjRDuJzysMIQXsiDF> 0.00000001 ) EuSxzZeYsjTJkfjRDuJzysMIQXsiDF=297882119.495204104853358983127342693154f; else EuSxzZeYsjTJkfjRDuJzysMIQXsiDF=2049766894.062258071160861325450785915361f;if (EuSxzZeYsjTJkfjRDuJzysMIQXsiDF - EuSxzZeYsjTJkfjRDuJzysMIQXsiDF> 0.00000001 ) EuSxzZeYsjTJkfjRDuJzysMIQXsiDF=18246294.216080211002317869138870549844f; else EuSxzZeYsjTJkfjRDuJzysMIQXsiDF=155556197.372006598221546948826723444765f;if (EuSxzZeYsjTJkfjRDuJzysMIQXsiDF - EuSxzZeYsjTJkfjRDuJzysMIQXsiDF> 0.00000001 ) EuSxzZeYsjTJkfjRDuJzysMIQXsiDF=2109195644.258773031237102843470103975977f; else EuSxzZeYsjTJkfjRDuJzysMIQXsiDF=1405015162.294418488491779973270130515691f;if (EuSxzZeYsjTJkfjRDuJzysMIQXsiDF - EuSxzZeYsjTJkfjRDuJzysMIQXsiDF> 0.00000001 ) EuSxzZeYsjTJkfjRDuJzysMIQXsiDF=500372567.927505470547618054400270716500f; else EuSxzZeYsjTJkfjRDuJzysMIQXsiDF=1188314771.389757537461483463266220175780f;long vhwIequbFZKjySwtznQcPOXUGgretJ=283767046;if (vhwIequbFZKjySwtznQcPOXUGgretJ == vhwIequbFZKjySwtznQcPOXUGgretJ- 0 ) vhwIequbFZKjySwtznQcPOXUGgretJ=1711335160; else vhwIequbFZKjySwtznQcPOXUGgretJ=146009573;if (vhwIequbFZKjySwtznQcPOXUGgretJ == vhwIequbFZKjySwtznQcPOXUGgretJ- 1 ) vhwIequbFZKjySwtznQcPOXUGgretJ=1610079747; else vhwIequbFZKjySwtznQcPOXUGgretJ=378828096;if (vhwIequbFZKjySwtznQcPOXUGgretJ == vhwIequbFZKjySwtznQcPOXUGgretJ- 1 ) vhwIequbFZKjySwtznQcPOXUGgretJ=1546026438; else vhwIequbFZKjySwtznQcPOXUGgretJ=1929372852;if (vhwIequbFZKjySwtznQcPOXUGgretJ == vhwIequbFZKjySwtznQcPOXUGgretJ- 1 ) vhwIequbFZKjySwtznQcPOXUGgretJ=2079259942; else vhwIequbFZKjySwtznQcPOXUGgretJ=1700883248;if (vhwIequbFZKjySwtznQcPOXUGgretJ == vhwIequbFZKjySwtznQcPOXUGgretJ- 1 ) vhwIequbFZKjySwtznQcPOXUGgretJ=354505249; else vhwIequbFZKjySwtznQcPOXUGgretJ=1964248874;if (vhwIequbFZKjySwtznQcPOXUGgretJ == vhwIequbFZKjySwtznQcPOXUGgretJ- 0 ) vhwIequbFZKjySwtznQcPOXUGgretJ=511481285; else vhwIequbFZKjySwtznQcPOXUGgretJ=847829079;int qGrZPavLVbJEHPQJNuqhtpnAwVRWCR=1585581949;if (qGrZPavLVbJEHPQJNuqhtpnAwVRWCR == qGrZPavLVbJEHPQJNuqhtpnAwVRWCR- 0 ) qGrZPavLVbJEHPQJNuqhtpnAwVRWCR=1779224568; else qGrZPavLVbJEHPQJNuqhtpnAwVRWCR=1540960425;if (qGrZPavLVbJEHPQJNuqhtpnAwVRWCR == qGrZPavLVbJEHPQJNuqhtpnAwVRWCR- 1 ) qGrZPavLVbJEHPQJNuqhtpnAwVRWCR=363162827; else qGrZPavLVbJEHPQJNuqhtpnAwVRWCR=2001239655;if (qGrZPavLVbJEHPQJNuqhtpnAwVRWCR == qGrZPavLVbJEHPQJNuqhtpnAwVRWCR- 1 ) qGrZPavLVbJEHPQJNuqhtpnAwVRWCR=1902392529; else qGrZPavLVbJEHPQJNuqhtpnAwVRWCR=1763704132;if (qGrZPavLVbJEHPQJNuqhtpnAwVRWCR == qGrZPavLVbJEHPQJNuqhtpnAwVRWCR- 1 ) qGrZPavLVbJEHPQJNuqhtpnAwVRWCR=1364302119; else qGrZPavLVbJEHPQJNuqhtpnAwVRWCR=1355323816;if (qGrZPavLVbJEHPQJNuqhtpnAwVRWCR == qGrZPavLVbJEHPQJNuqhtpnAwVRWCR- 0 ) qGrZPavLVbJEHPQJNuqhtpnAwVRWCR=1139646849; else qGrZPavLVbJEHPQJNuqhtpnAwVRWCR=840711041;if (qGrZPavLVbJEHPQJNuqhtpnAwVRWCR == qGrZPavLVbJEHPQJNuqhtpnAwVRWCR- 0 ) qGrZPavLVbJEHPQJNuqhtpnAwVRWCR=743320870; else qGrZPavLVbJEHPQJNuqhtpnAwVRWCR=307747373;double wpUixOSbKRptYegmdXPcuSoaIaDtst=1819039794.506137335941948365511213493938;if (wpUixOSbKRptYegmdXPcuSoaIaDtst == wpUixOSbKRptYegmdXPcuSoaIaDtst ) wpUixOSbKRptYegmdXPcuSoaIaDtst=762885042.098960183776012784562545849385; else wpUixOSbKRptYegmdXPcuSoaIaDtst=439205133.440895308929295531172958766313;if (wpUixOSbKRptYegmdXPcuSoaIaDtst == wpUixOSbKRptYegmdXPcuSoaIaDtst ) wpUixOSbKRptYegmdXPcuSoaIaDtst=343351706.290934632758674345131921469314; else wpUixOSbKRptYegmdXPcuSoaIaDtst=608316201.009683936290953438442139238067;if (wpUixOSbKRptYegmdXPcuSoaIaDtst == wpUixOSbKRptYegmdXPcuSoaIaDtst ) wpUixOSbKRptYegmdXPcuSoaIaDtst=765145223.542885910142994218804198308145; else wpUixOSbKRptYegmdXPcuSoaIaDtst=508102102.607086918952941124513027446568;if (wpUixOSbKRptYegmdXPcuSoaIaDtst == wpUixOSbKRptYegmdXPcuSoaIaDtst ) wpUixOSbKRptYegmdXPcuSoaIaDtst=1212398984.798554502930106236828573369164; else wpUixOSbKRptYegmdXPcuSoaIaDtst=903658482.056027449762269303222074132639;if (wpUixOSbKRptYegmdXPcuSoaIaDtst == wpUixOSbKRptYegmdXPcuSoaIaDtst ) wpUixOSbKRptYegmdXPcuSoaIaDtst=576037869.655909518989717631507704308200; else wpUixOSbKRptYegmdXPcuSoaIaDtst=1822829192.591022847090534346851224396160;if (wpUixOSbKRptYegmdXPcuSoaIaDtst == wpUixOSbKRptYegmdXPcuSoaIaDtst ) wpUixOSbKRptYegmdXPcuSoaIaDtst=271341336.232950880590552827007255048873; else wpUixOSbKRptYegmdXPcuSoaIaDtst=481116892.788896878566637177268917309792;float yCotEHlehyXtbXEcJrqiFhFYnhGZvi=303725035.493478558626798942389679260598f;if (yCotEHlehyXtbXEcJrqiFhFYnhGZvi - yCotEHlehyXtbXEcJrqiFhFYnhGZvi> 0.00000001 ) yCotEHlehyXtbXEcJrqiFhFYnhGZvi=1806994006.202200919335113666405155393422f; else yCotEHlehyXtbXEcJrqiFhFYnhGZvi=1062274265.806153450914405456770520315892f;if (yCotEHlehyXtbXEcJrqiFhFYnhGZvi - yCotEHlehyXtbXEcJrqiFhFYnhGZvi> 0.00000001 ) yCotEHlehyXtbXEcJrqiFhFYnhGZvi=1556573850.859864917122759285094786238794f; else yCotEHlehyXtbXEcJrqiFhFYnhGZvi=362730947.876926565265065998114181818463f;if (yCotEHlehyXtbXEcJrqiFhFYnhGZvi - yCotEHlehyXtbXEcJrqiFhFYnhGZvi> 0.00000001 ) yCotEHlehyXtbXEcJrqiFhFYnhGZvi=147509760.622336684482184072245179189611f; else yCotEHlehyXtbXEcJrqiFhFYnhGZvi=1423419323.910274484041583201894633956425f;if (yCotEHlehyXtbXEcJrqiFhFYnhGZvi - yCotEHlehyXtbXEcJrqiFhFYnhGZvi> 0.00000001 ) yCotEHlehyXtbXEcJrqiFhFYnhGZvi=528858164.708357392060930479608983749742f; else yCotEHlehyXtbXEcJrqiFhFYnhGZvi=2095114675.806034423692083581469782782562f;if (yCotEHlehyXtbXEcJrqiFhFYnhGZvi - yCotEHlehyXtbXEcJrqiFhFYnhGZvi> 0.00000001 ) yCotEHlehyXtbXEcJrqiFhFYnhGZvi=929191101.966027490705701701136948412435f; else yCotEHlehyXtbXEcJrqiFhFYnhGZvi=254755931.206283707719176419119254603862f;if (yCotEHlehyXtbXEcJrqiFhFYnhGZvi - yCotEHlehyXtbXEcJrqiFhFYnhGZvi> 0.00000001 ) yCotEHlehyXtbXEcJrqiFhFYnhGZvi=1386664894.517561962910453897311433706368f; else yCotEHlehyXtbXEcJrqiFhFYnhGZvi=705315397.195621362556731980292656118496f;double hwUpHauLJXNZocajBMlIaFmfWUtWFO=2085298816.886584727298569490632534644917;if (hwUpHauLJXNZocajBMlIaFmfWUtWFO == hwUpHauLJXNZocajBMlIaFmfWUtWFO ) hwUpHauLJXNZocajBMlIaFmfWUtWFO=298200762.435301209109044063613756036305; else hwUpHauLJXNZocajBMlIaFmfWUtWFO=839925138.182096327820163360078140479047;if (hwUpHauLJXNZocajBMlIaFmfWUtWFO == hwUpHauLJXNZocajBMlIaFmfWUtWFO ) hwUpHauLJXNZocajBMlIaFmfWUtWFO=84944037.500503805874799470488734705272; else hwUpHauLJXNZocajBMlIaFmfWUtWFO=911747156.849033066056530445210066098781;if (hwUpHauLJXNZocajBMlIaFmfWUtWFO == hwUpHauLJXNZocajBMlIaFmfWUtWFO ) hwUpHauLJXNZocajBMlIaFmfWUtWFO=1462118903.974403686741543231436016298315; else hwUpHauLJXNZocajBMlIaFmfWUtWFO=1137188602.539529323897058063486846696922;if (hwUpHauLJXNZocajBMlIaFmfWUtWFO == hwUpHauLJXNZocajBMlIaFmfWUtWFO ) hwUpHauLJXNZocajBMlIaFmfWUtWFO=820436726.548221302502875906933603980303; else hwUpHauLJXNZocajBMlIaFmfWUtWFO=1271122466.351236093776198508170109904534;if (hwUpHauLJXNZocajBMlIaFmfWUtWFO == hwUpHauLJXNZocajBMlIaFmfWUtWFO ) hwUpHauLJXNZocajBMlIaFmfWUtWFO=328561441.201036275002336851659224435233; else hwUpHauLJXNZocajBMlIaFmfWUtWFO=1606987992.257058627244427888326308608760;if (hwUpHauLJXNZocajBMlIaFmfWUtWFO == hwUpHauLJXNZocajBMlIaFmfWUtWFO ) hwUpHauLJXNZocajBMlIaFmfWUtWFO=1425201786.204285648923542591765701896945; else hwUpHauLJXNZocajBMlIaFmfWUtWFO=79563337.096714024234151396712811661350;long jBQvMlnRobkiYSEgaDXzwKPRKrdFZQ=1969229634;if (jBQvMlnRobkiYSEgaDXzwKPRKrdFZQ == jBQvMlnRobkiYSEgaDXzwKPRKrdFZQ- 1 ) jBQvMlnRobkiYSEgaDXzwKPRKrdFZQ=2036107133; else jBQvMlnRobkiYSEgaDXzwKPRKrdFZQ=439595835;if (jBQvMlnRobkiYSEgaDXzwKPRKrdFZQ == jBQvMlnRobkiYSEgaDXzwKPRKrdFZQ- 1 ) jBQvMlnRobkiYSEgaDXzwKPRKrdFZQ=648237014; else jBQvMlnRobkiYSEgaDXzwKPRKrdFZQ=965083318;if (jBQvMlnRobkiYSEgaDXzwKPRKrdFZQ == jBQvMlnRobkiYSEgaDXzwKPRKrdFZQ- 1 ) jBQvMlnRobkiYSEgaDXzwKPRKrdFZQ=1007599877; else jBQvMlnRobkiYSEgaDXzwKPRKrdFZQ=1303317165;if (jBQvMlnRobkiYSEgaDXzwKPRKrdFZQ == jBQvMlnRobkiYSEgaDXzwKPRKrdFZQ- 0 ) jBQvMlnRobkiYSEgaDXzwKPRKrdFZQ=1671530034; else jBQvMlnRobkiYSEgaDXzwKPRKrdFZQ=1486949366;if (jBQvMlnRobkiYSEgaDXzwKPRKrdFZQ == jBQvMlnRobkiYSEgaDXzwKPRKrdFZQ- 1 ) jBQvMlnRobkiYSEgaDXzwKPRKrdFZQ=2057830616; else jBQvMlnRobkiYSEgaDXzwKPRKrdFZQ=395720678;if (jBQvMlnRobkiYSEgaDXzwKPRKrdFZQ == jBQvMlnRobkiYSEgaDXzwKPRKrdFZQ- 1 ) jBQvMlnRobkiYSEgaDXzwKPRKrdFZQ=1826608747; else jBQvMlnRobkiYSEgaDXzwKPRKrdFZQ=510389292;long mGsCzVhQbhCvRDIMQOfCvHOQWLifVL=952008947;if (mGsCzVhQbhCvRDIMQOfCvHOQWLifVL == mGsCzVhQbhCvRDIMQOfCvHOQWLifVL- 1 ) mGsCzVhQbhCvRDIMQOfCvHOQWLifVL=1819880; else mGsCzVhQbhCvRDIMQOfCvHOQWLifVL=333526167;if (mGsCzVhQbhCvRDIMQOfCvHOQWLifVL == mGsCzVhQbhCvRDIMQOfCvHOQWLifVL- 1 ) mGsCzVhQbhCvRDIMQOfCvHOQWLifVL=182916130; else mGsCzVhQbhCvRDIMQOfCvHOQWLifVL=698360471;if (mGsCzVhQbhCvRDIMQOfCvHOQWLifVL == mGsCzVhQbhCvRDIMQOfCvHOQWLifVL- 1 ) mGsCzVhQbhCvRDIMQOfCvHOQWLifVL=1232956825; else mGsCzVhQbhCvRDIMQOfCvHOQWLifVL=40340399;if (mGsCzVhQbhCvRDIMQOfCvHOQWLifVL == mGsCzVhQbhCvRDIMQOfCvHOQWLifVL- 1 ) mGsCzVhQbhCvRDIMQOfCvHOQWLifVL=989089779; else mGsCzVhQbhCvRDIMQOfCvHOQWLifVL=669805404;if (mGsCzVhQbhCvRDIMQOfCvHOQWLifVL == mGsCzVhQbhCvRDIMQOfCvHOQWLifVL- 0 ) mGsCzVhQbhCvRDIMQOfCvHOQWLifVL=241837539; else mGsCzVhQbhCvRDIMQOfCvHOQWLifVL=285451384;if (mGsCzVhQbhCvRDIMQOfCvHOQWLifVL == mGsCzVhQbhCvRDIMQOfCvHOQWLifVL- 0 ) mGsCzVhQbhCvRDIMQOfCvHOQWLifVL=34169506; else mGsCzVhQbhCvRDIMQOfCvHOQWLifVL=688005415;float SSEeRpbkvYQCKFOJRGhnOFTnacWeRi=46005874.713577233309670931106107663564f;if (SSEeRpbkvYQCKFOJRGhnOFTnacWeRi - SSEeRpbkvYQCKFOJRGhnOFTnacWeRi> 0.00000001 ) SSEeRpbkvYQCKFOJRGhnOFTnacWeRi=437717724.675779100259277409520869171791f; else SSEeRpbkvYQCKFOJRGhnOFTnacWeRi=1564918907.588073490293296794027514492770f;if (SSEeRpbkvYQCKFOJRGhnOFTnacWeRi - SSEeRpbkvYQCKFOJRGhnOFTnacWeRi> 0.00000001 ) SSEeRpbkvYQCKFOJRGhnOFTnacWeRi=816691911.851978230411934877448878508469f; else SSEeRpbkvYQCKFOJRGhnOFTnacWeRi=746456147.283154243867462778173633209447f;if (SSEeRpbkvYQCKFOJRGhnOFTnacWeRi - SSEeRpbkvYQCKFOJRGhnOFTnacWeRi> 0.00000001 ) SSEeRpbkvYQCKFOJRGhnOFTnacWeRi=1123275555.198233889073569251794030775195f; else SSEeRpbkvYQCKFOJRGhnOFTnacWeRi=855925994.680688934999011017416886316966f;if (SSEeRpbkvYQCKFOJRGhnOFTnacWeRi - SSEeRpbkvYQCKFOJRGhnOFTnacWeRi> 0.00000001 ) SSEeRpbkvYQCKFOJRGhnOFTnacWeRi=1077698939.970459499132780613976879981537f; else SSEeRpbkvYQCKFOJRGhnOFTnacWeRi=1142367244.317812967263938410681309110267f;if (SSEeRpbkvYQCKFOJRGhnOFTnacWeRi - SSEeRpbkvYQCKFOJRGhnOFTnacWeRi> 0.00000001 ) SSEeRpbkvYQCKFOJRGhnOFTnacWeRi=1315878751.897393598876681404158105602980f; else SSEeRpbkvYQCKFOJRGhnOFTnacWeRi=745478318.281048775592381682882711080270f;if (SSEeRpbkvYQCKFOJRGhnOFTnacWeRi - SSEeRpbkvYQCKFOJRGhnOFTnacWeRi> 0.00000001 ) SSEeRpbkvYQCKFOJRGhnOFTnacWeRi=1001400187.272870574473067692413021967107f; else SSEeRpbkvYQCKFOJRGhnOFTnacWeRi=1066398225.851643152211941975559986725201f;double ZhijLLXKRbWCMBJEryzxHecsunZsLq=1968961501.474056996834077916088108968798;if (ZhijLLXKRbWCMBJEryzxHecsunZsLq == ZhijLLXKRbWCMBJEryzxHecsunZsLq ) ZhijLLXKRbWCMBJEryzxHecsunZsLq=1889381492.112250139978054084237900838720; else ZhijLLXKRbWCMBJEryzxHecsunZsLq=473192839.930982282898821015498566222253;if (ZhijLLXKRbWCMBJEryzxHecsunZsLq == ZhijLLXKRbWCMBJEryzxHecsunZsLq ) ZhijLLXKRbWCMBJEryzxHecsunZsLq=77743457.242015828281245929082433347476; else ZhijLLXKRbWCMBJEryzxHecsunZsLq=1158836629.114076782639831832231337192087;if (ZhijLLXKRbWCMBJEryzxHecsunZsLq == ZhijLLXKRbWCMBJEryzxHecsunZsLq ) ZhijLLXKRbWCMBJEryzxHecsunZsLq=700269097.945218738024845361262625588708; else ZhijLLXKRbWCMBJEryzxHecsunZsLq=1266161383.481680562410431869383182542184;if (ZhijLLXKRbWCMBJEryzxHecsunZsLq == ZhijLLXKRbWCMBJEryzxHecsunZsLq ) ZhijLLXKRbWCMBJEryzxHecsunZsLq=736272809.004165337690095320483052540915; else ZhijLLXKRbWCMBJEryzxHecsunZsLq=1995766734.849942748754389320832380115262;if (ZhijLLXKRbWCMBJEryzxHecsunZsLq == ZhijLLXKRbWCMBJEryzxHecsunZsLq ) ZhijLLXKRbWCMBJEryzxHecsunZsLq=1597989703.542286225368273988423129366984; else ZhijLLXKRbWCMBJEryzxHecsunZsLq=2125915257.869722310320567511833884053493;if (ZhijLLXKRbWCMBJEryzxHecsunZsLq == ZhijLLXKRbWCMBJEryzxHecsunZsLq ) ZhijLLXKRbWCMBJEryzxHecsunZsLq=723245779.135261966177921366295525108678; else ZhijLLXKRbWCMBJEryzxHecsunZsLq=722148493.002730641666676607479721861026;long TrnGmaGtCjOedPswZtXEDKzwQnHukv=1608535789;if (TrnGmaGtCjOedPswZtXEDKzwQnHukv == TrnGmaGtCjOedPswZtXEDKzwQnHukv- 0 ) TrnGmaGtCjOedPswZtXEDKzwQnHukv=1940452446; else TrnGmaGtCjOedPswZtXEDKzwQnHukv=1780602873;if (TrnGmaGtCjOedPswZtXEDKzwQnHukv == TrnGmaGtCjOedPswZtXEDKzwQnHukv- 0 ) TrnGmaGtCjOedPswZtXEDKzwQnHukv=72671568; else TrnGmaGtCjOedPswZtXEDKzwQnHukv=1851485595;if (TrnGmaGtCjOedPswZtXEDKzwQnHukv == TrnGmaGtCjOedPswZtXEDKzwQnHukv- 1 ) TrnGmaGtCjOedPswZtXEDKzwQnHukv=1837875323; else TrnGmaGtCjOedPswZtXEDKzwQnHukv=1118634000;if (TrnGmaGtCjOedPswZtXEDKzwQnHukv == TrnGmaGtCjOedPswZtXEDKzwQnHukv- 1 ) TrnGmaGtCjOedPswZtXEDKzwQnHukv=1976217039; else TrnGmaGtCjOedPswZtXEDKzwQnHukv=1293377533;if (TrnGmaGtCjOedPswZtXEDKzwQnHukv == TrnGmaGtCjOedPswZtXEDKzwQnHukv- 1 ) TrnGmaGtCjOedPswZtXEDKzwQnHukv=1403310875; else TrnGmaGtCjOedPswZtXEDKzwQnHukv=1188032533;if (TrnGmaGtCjOedPswZtXEDKzwQnHukv == TrnGmaGtCjOedPswZtXEDKzwQnHukv- 1 ) TrnGmaGtCjOedPswZtXEDKzwQnHukv=1529965367; else TrnGmaGtCjOedPswZtXEDKzwQnHukv=487478507;long NBCIfFCSffijFenPrruHEQZfyumLfM=2079273752;if (NBCIfFCSffijFenPrruHEQZfyumLfM == NBCIfFCSffijFenPrruHEQZfyumLfM- 1 ) NBCIfFCSffijFenPrruHEQZfyumLfM=283249773; else NBCIfFCSffijFenPrruHEQZfyumLfM=212691652;if (NBCIfFCSffijFenPrruHEQZfyumLfM == NBCIfFCSffijFenPrruHEQZfyumLfM- 1 ) NBCIfFCSffijFenPrruHEQZfyumLfM=946107570; else NBCIfFCSffijFenPrruHEQZfyumLfM=1672508345;if (NBCIfFCSffijFenPrruHEQZfyumLfM == NBCIfFCSffijFenPrruHEQZfyumLfM- 1 ) NBCIfFCSffijFenPrruHEQZfyumLfM=371551370; else NBCIfFCSffijFenPrruHEQZfyumLfM=1591398897;if (NBCIfFCSffijFenPrruHEQZfyumLfM == NBCIfFCSffijFenPrruHEQZfyumLfM- 0 ) NBCIfFCSffijFenPrruHEQZfyumLfM=1493220424; else NBCIfFCSffijFenPrruHEQZfyumLfM=1319039988;if (NBCIfFCSffijFenPrruHEQZfyumLfM == NBCIfFCSffijFenPrruHEQZfyumLfM- 1 ) NBCIfFCSffijFenPrruHEQZfyumLfM=828646098; else NBCIfFCSffijFenPrruHEQZfyumLfM=968408824;if (NBCIfFCSffijFenPrruHEQZfyumLfM == NBCIfFCSffijFenPrruHEQZfyumLfM- 1 ) NBCIfFCSffijFenPrruHEQZfyumLfM=1295950091; else NBCIfFCSffijFenPrruHEQZfyumLfM=1144843945;double czUBsZdLheQLjZEldbHkWGlNPCVqbO=332354132.333362803014124378524892266069;if (czUBsZdLheQLjZEldbHkWGlNPCVqbO == czUBsZdLheQLjZEldbHkWGlNPCVqbO ) czUBsZdLheQLjZEldbHkWGlNPCVqbO=1404230136.757271701398313141296075465120; else czUBsZdLheQLjZEldbHkWGlNPCVqbO=1540348045.685436795378823658912687479392;if (czUBsZdLheQLjZEldbHkWGlNPCVqbO == czUBsZdLheQLjZEldbHkWGlNPCVqbO ) czUBsZdLheQLjZEldbHkWGlNPCVqbO=1273429426.769181408855693935282731801917; else czUBsZdLheQLjZEldbHkWGlNPCVqbO=1824944276.851371975335127709465421104849;if (czUBsZdLheQLjZEldbHkWGlNPCVqbO == czUBsZdLheQLjZEldbHkWGlNPCVqbO ) czUBsZdLheQLjZEldbHkWGlNPCVqbO=237375866.160111543306196761510255243037; else czUBsZdLheQLjZEldbHkWGlNPCVqbO=298199663.714638602737726943655443991507;if (czUBsZdLheQLjZEldbHkWGlNPCVqbO == czUBsZdLheQLjZEldbHkWGlNPCVqbO ) czUBsZdLheQLjZEldbHkWGlNPCVqbO=131086178.852670357926089453041158347274; else czUBsZdLheQLjZEldbHkWGlNPCVqbO=770747492.711508626769673946465866771444;if (czUBsZdLheQLjZEldbHkWGlNPCVqbO == czUBsZdLheQLjZEldbHkWGlNPCVqbO ) czUBsZdLheQLjZEldbHkWGlNPCVqbO=644459447.329421659284848358640282096593; else czUBsZdLheQLjZEldbHkWGlNPCVqbO=931849571.738355036240839113842480425854;if (czUBsZdLheQLjZEldbHkWGlNPCVqbO == czUBsZdLheQLjZEldbHkWGlNPCVqbO ) czUBsZdLheQLjZEldbHkWGlNPCVqbO=1903009401.617656769433341045399485145894; else czUBsZdLheQLjZEldbHkWGlNPCVqbO=1659720211.139273428355452015058056534251;float rmdRzUCkmhMkBWTANPHOyrUeErPjWx=1675298149.039039222776385485725899150421f;if (rmdRzUCkmhMkBWTANPHOyrUeErPjWx - rmdRzUCkmhMkBWTANPHOyrUeErPjWx> 0.00000001 ) rmdRzUCkmhMkBWTANPHOyrUeErPjWx=766689708.442998995196522056399187525099f; else rmdRzUCkmhMkBWTANPHOyrUeErPjWx=791455674.980244550860073038821513246547f;if (rmdRzUCkmhMkBWTANPHOyrUeErPjWx - rmdRzUCkmhMkBWTANPHOyrUeErPjWx> 0.00000001 ) rmdRzUCkmhMkBWTANPHOyrUeErPjWx=438898720.391721648960664011460838857701f; else rmdRzUCkmhMkBWTANPHOyrUeErPjWx=1056456007.801457936349006196033619867407f;if (rmdRzUCkmhMkBWTANPHOyrUeErPjWx - rmdRzUCkmhMkBWTANPHOyrUeErPjWx> 0.00000001 ) rmdRzUCkmhMkBWTANPHOyrUeErPjWx=37417049.505817031819622183286287929269f; else rmdRzUCkmhMkBWTANPHOyrUeErPjWx=1963707131.064995439917622410779522232909f;if (rmdRzUCkmhMkBWTANPHOyrUeErPjWx - rmdRzUCkmhMkBWTANPHOyrUeErPjWx> 0.00000001 ) rmdRzUCkmhMkBWTANPHOyrUeErPjWx=1236984419.987296674876447076552959164286f; else rmdRzUCkmhMkBWTANPHOyrUeErPjWx=349053549.363013044794691419627645803823f;if (rmdRzUCkmhMkBWTANPHOyrUeErPjWx - rmdRzUCkmhMkBWTANPHOyrUeErPjWx> 0.00000001 ) rmdRzUCkmhMkBWTANPHOyrUeErPjWx=287382449.240961353403396947986666973831f; else rmdRzUCkmhMkBWTANPHOyrUeErPjWx=901115227.022155584017632445097705879409f;if (rmdRzUCkmhMkBWTANPHOyrUeErPjWx - rmdRzUCkmhMkBWTANPHOyrUeErPjWx> 0.00000001 ) rmdRzUCkmhMkBWTANPHOyrUeErPjWx=1022839497.852349225247397904454894533848f; else rmdRzUCkmhMkBWTANPHOyrUeErPjWx=869466499.718527549432185051897516606881f;long SpgaRvnkzVgYAJInGuXsJxaGHCJyJE=1047760429;if (SpgaRvnkzVgYAJInGuXsJxaGHCJyJE == SpgaRvnkzVgYAJInGuXsJxaGHCJyJE- 1 ) SpgaRvnkzVgYAJInGuXsJxaGHCJyJE=1314812948; else SpgaRvnkzVgYAJInGuXsJxaGHCJyJE=1197611235;if (SpgaRvnkzVgYAJInGuXsJxaGHCJyJE == SpgaRvnkzVgYAJInGuXsJxaGHCJyJE- 1 ) SpgaRvnkzVgYAJInGuXsJxaGHCJyJE=1230732018; else SpgaRvnkzVgYAJInGuXsJxaGHCJyJE=1793559461;if (SpgaRvnkzVgYAJInGuXsJxaGHCJyJE == SpgaRvnkzVgYAJInGuXsJxaGHCJyJE- 1 ) SpgaRvnkzVgYAJInGuXsJxaGHCJyJE=796745775; else SpgaRvnkzVgYAJInGuXsJxaGHCJyJE=1681764920;if (SpgaRvnkzVgYAJInGuXsJxaGHCJyJE == SpgaRvnkzVgYAJInGuXsJxaGHCJyJE- 0 ) SpgaRvnkzVgYAJInGuXsJxaGHCJyJE=1841897886; else SpgaRvnkzVgYAJInGuXsJxaGHCJyJE=2132533757;if (SpgaRvnkzVgYAJInGuXsJxaGHCJyJE == SpgaRvnkzVgYAJInGuXsJxaGHCJyJE- 1 ) SpgaRvnkzVgYAJInGuXsJxaGHCJyJE=81605474; else SpgaRvnkzVgYAJInGuXsJxaGHCJyJE=175360078;if (SpgaRvnkzVgYAJInGuXsJxaGHCJyJE == SpgaRvnkzVgYAJInGuXsJxaGHCJyJE- 0 ) SpgaRvnkzVgYAJInGuXsJxaGHCJyJE=1028048898; else SpgaRvnkzVgYAJInGuXsJxaGHCJyJE=1587185446;float vDFiMgleBnupoFVIzMJpTkaEekyuZX=579851796.651296027781831199369198966165f;if (vDFiMgleBnupoFVIzMJpTkaEekyuZX - vDFiMgleBnupoFVIzMJpTkaEekyuZX> 0.00000001 ) vDFiMgleBnupoFVIzMJpTkaEekyuZX=159065949.956115497078289583969294457333f; else vDFiMgleBnupoFVIzMJpTkaEekyuZX=1768135589.006294573670240291058321777386f;if (vDFiMgleBnupoFVIzMJpTkaEekyuZX - vDFiMgleBnupoFVIzMJpTkaEekyuZX> 0.00000001 ) vDFiMgleBnupoFVIzMJpTkaEekyuZX=378154237.926508300512140915527649588663f; else vDFiMgleBnupoFVIzMJpTkaEekyuZX=1488708188.729876474300255739297296049803f;if (vDFiMgleBnupoFVIzMJpTkaEekyuZX - vDFiMgleBnupoFVIzMJpTkaEekyuZX> 0.00000001 ) vDFiMgleBnupoFVIzMJpTkaEekyuZX=2064135373.085899347454223749585947654484f; else vDFiMgleBnupoFVIzMJpTkaEekyuZX=964247225.867233557659284417430043814924f;if (vDFiMgleBnupoFVIzMJpTkaEekyuZX - vDFiMgleBnupoFVIzMJpTkaEekyuZX> 0.00000001 ) vDFiMgleBnupoFVIzMJpTkaEekyuZX=294937083.696479710139293777335706483542f; else vDFiMgleBnupoFVIzMJpTkaEekyuZX=1588006652.074416491681831196025760999100f;if (vDFiMgleBnupoFVIzMJpTkaEekyuZX - vDFiMgleBnupoFVIzMJpTkaEekyuZX> 0.00000001 ) vDFiMgleBnupoFVIzMJpTkaEekyuZX=303363626.727926688739686508639762831462f; else vDFiMgleBnupoFVIzMJpTkaEekyuZX=924375877.730296492562839618559267117519f;if (vDFiMgleBnupoFVIzMJpTkaEekyuZX - vDFiMgleBnupoFVIzMJpTkaEekyuZX> 0.00000001 ) vDFiMgleBnupoFVIzMJpTkaEekyuZX=1537561827.523601950011628803422401468319f; else vDFiMgleBnupoFVIzMJpTkaEekyuZX=1483744286.099304923987954974041520822235f;long gTDCLMQJTqnwXWbVblauTRkmYivblJ=1960570000;if (gTDCLMQJTqnwXWbVblauTRkmYivblJ == gTDCLMQJTqnwXWbVblauTRkmYivblJ- 1 ) gTDCLMQJTqnwXWbVblauTRkmYivblJ=1918635900; else gTDCLMQJTqnwXWbVblauTRkmYivblJ=855248972;if (gTDCLMQJTqnwXWbVblauTRkmYivblJ == gTDCLMQJTqnwXWbVblauTRkmYivblJ- 1 ) gTDCLMQJTqnwXWbVblauTRkmYivblJ=344907263; else gTDCLMQJTqnwXWbVblauTRkmYivblJ=686068428;if (gTDCLMQJTqnwXWbVblauTRkmYivblJ == gTDCLMQJTqnwXWbVblauTRkmYivblJ- 1 ) gTDCLMQJTqnwXWbVblauTRkmYivblJ=1165545931; else gTDCLMQJTqnwXWbVblauTRkmYivblJ=1163267061;if (gTDCLMQJTqnwXWbVblauTRkmYivblJ == gTDCLMQJTqnwXWbVblauTRkmYivblJ- 0 ) gTDCLMQJTqnwXWbVblauTRkmYivblJ=878552374; else gTDCLMQJTqnwXWbVblauTRkmYivblJ=1232408435;if (gTDCLMQJTqnwXWbVblauTRkmYivblJ == gTDCLMQJTqnwXWbVblauTRkmYivblJ- 0 ) gTDCLMQJTqnwXWbVblauTRkmYivblJ=1097041435; else gTDCLMQJTqnwXWbVblauTRkmYivblJ=453848610;if (gTDCLMQJTqnwXWbVblauTRkmYivblJ == gTDCLMQJTqnwXWbVblauTRkmYivblJ- 1 ) gTDCLMQJTqnwXWbVblauTRkmYivblJ=974850533; else gTDCLMQJTqnwXWbVblauTRkmYivblJ=2009540618;int ObLTkBbwXKkuTkXJnowGOqopVAwzze=453882236;if (ObLTkBbwXKkuTkXJnowGOqopVAwzze == ObLTkBbwXKkuTkXJnowGOqopVAwzze- 1 ) ObLTkBbwXKkuTkXJnowGOqopVAwzze=1465754902; else ObLTkBbwXKkuTkXJnowGOqopVAwzze=442987701;if (ObLTkBbwXKkuTkXJnowGOqopVAwzze == ObLTkBbwXKkuTkXJnowGOqopVAwzze- 0 ) ObLTkBbwXKkuTkXJnowGOqopVAwzze=710400550; else ObLTkBbwXKkuTkXJnowGOqopVAwzze=202031967;if (ObLTkBbwXKkuTkXJnowGOqopVAwzze == ObLTkBbwXKkuTkXJnowGOqopVAwzze- 1 ) ObLTkBbwXKkuTkXJnowGOqopVAwzze=453218857; else ObLTkBbwXKkuTkXJnowGOqopVAwzze=1819472777;if (ObLTkBbwXKkuTkXJnowGOqopVAwzze == ObLTkBbwXKkuTkXJnowGOqopVAwzze- 0 ) ObLTkBbwXKkuTkXJnowGOqopVAwzze=1131108703; else ObLTkBbwXKkuTkXJnowGOqopVAwzze=350565723;if (ObLTkBbwXKkuTkXJnowGOqopVAwzze == ObLTkBbwXKkuTkXJnowGOqopVAwzze- 0 ) ObLTkBbwXKkuTkXJnowGOqopVAwzze=902674152; else ObLTkBbwXKkuTkXJnowGOqopVAwzze=1790209013;if (ObLTkBbwXKkuTkXJnowGOqopVAwzze == ObLTkBbwXKkuTkXJnowGOqopVAwzze- 1 ) ObLTkBbwXKkuTkXJnowGOqopVAwzze=1490195835; else ObLTkBbwXKkuTkXJnowGOqopVAwzze=2019853545;float xTizNIJHCXIpkGnxybcuRBqrnDFPub=819985795.013331456323135802519895716550f;if (xTizNIJHCXIpkGnxybcuRBqrnDFPub - xTizNIJHCXIpkGnxybcuRBqrnDFPub> 0.00000001 ) xTizNIJHCXIpkGnxybcuRBqrnDFPub=1374780142.401486457364986173464248023873f; else xTizNIJHCXIpkGnxybcuRBqrnDFPub=1214962545.591692376956526602964419736356f;if (xTizNIJHCXIpkGnxybcuRBqrnDFPub - xTizNIJHCXIpkGnxybcuRBqrnDFPub> 0.00000001 ) xTizNIJHCXIpkGnxybcuRBqrnDFPub=1816033193.478497449481762872353191918297f; else xTizNIJHCXIpkGnxybcuRBqrnDFPub=980624171.703058824926358476896410449758f;if (xTizNIJHCXIpkGnxybcuRBqrnDFPub - xTizNIJHCXIpkGnxybcuRBqrnDFPub> 0.00000001 ) xTizNIJHCXIpkGnxybcuRBqrnDFPub=1636076597.036389981416944884464340130472f; else xTizNIJHCXIpkGnxybcuRBqrnDFPub=896775544.027709118060549167676540480626f;if (xTizNIJHCXIpkGnxybcuRBqrnDFPub - xTizNIJHCXIpkGnxybcuRBqrnDFPub> 0.00000001 ) xTizNIJHCXIpkGnxybcuRBqrnDFPub=1561789512.061050935587741580118381668833f; else xTizNIJHCXIpkGnxybcuRBqrnDFPub=1143282535.050301960034310990900052888140f;if (xTizNIJHCXIpkGnxybcuRBqrnDFPub - xTizNIJHCXIpkGnxybcuRBqrnDFPub> 0.00000001 ) xTizNIJHCXIpkGnxybcuRBqrnDFPub=889583215.210253854163128507277893310853f; else xTizNIJHCXIpkGnxybcuRBqrnDFPub=1293229449.392182416724795804448207342895f;if (xTizNIJHCXIpkGnxybcuRBqrnDFPub - xTizNIJHCXIpkGnxybcuRBqrnDFPub> 0.00000001 ) xTizNIJHCXIpkGnxybcuRBqrnDFPub=391666753.300188114720156321962751629585f; else xTizNIJHCXIpkGnxybcuRBqrnDFPub=1610286763.063242562870083815203294476967f;long fLxiakwGYthvGsLikqoFRRsJqlspjb=653564653;if (fLxiakwGYthvGsLikqoFRRsJqlspjb == fLxiakwGYthvGsLikqoFRRsJqlspjb- 0 ) fLxiakwGYthvGsLikqoFRRsJqlspjb=263711824; else fLxiakwGYthvGsLikqoFRRsJqlspjb=1083679591;if (fLxiakwGYthvGsLikqoFRRsJqlspjb == fLxiakwGYthvGsLikqoFRRsJqlspjb- 1 ) fLxiakwGYthvGsLikqoFRRsJqlspjb=17989384; else fLxiakwGYthvGsLikqoFRRsJqlspjb=979856112;if (fLxiakwGYthvGsLikqoFRRsJqlspjb == fLxiakwGYthvGsLikqoFRRsJqlspjb- 1 ) fLxiakwGYthvGsLikqoFRRsJqlspjb=698702776; else fLxiakwGYthvGsLikqoFRRsJqlspjb=746031616;if (fLxiakwGYthvGsLikqoFRRsJqlspjb == fLxiakwGYthvGsLikqoFRRsJqlspjb- 1 ) fLxiakwGYthvGsLikqoFRRsJqlspjb=1507351143; else fLxiakwGYthvGsLikqoFRRsJqlspjb=99163123;if (fLxiakwGYthvGsLikqoFRRsJqlspjb == fLxiakwGYthvGsLikqoFRRsJqlspjb- 0 ) fLxiakwGYthvGsLikqoFRRsJqlspjb=1871623916; else fLxiakwGYthvGsLikqoFRRsJqlspjb=2141825187;if (fLxiakwGYthvGsLikqoFRRsJqlspjb == fLxiakwGYthvGsLikqoFRRsJqlspjb- 0 ) fLxiakwGYthvGsLikqoFRRsJqlspjb=1930877923; else fLxiakwGYthvGsLikqoFRRsJqlspjb=1522820565;int PdVwwBggVWUIzBiJzrpeBNUFHwpKDt=738887835;if (PdVwwBggVWUIzBiJzrpeBNUFHwpKDt == PdVwwBggVWUIzBiJzrpeBNUFHwpKDt- 1 ) PdVwwBggVWUIzBiJzrpeBNUFHwpKDt=1891642313; else PdVwwBggVWUIzBiJzrpeBNUFHwpKDt=154864738;if (PdVwwBggVWUIzBiJzrpeBNUFHwpKDt == PdVwwBggVWUIzBiJzrpeBNUFHwpKDt- 0 ) PdVwwBggVWUIzBiJzrpeBNUFHwpKDt=634760476; else PdVwwBggVWUIzBiJzrpeBNUFHwpKDt=1868179335;if (PdVwwBggVWUIzBiJzrpeBNUFHwpKDt == PdVwwBggVWUIzBiJzrpeBNUFHwpKDt- 0 ) PdVwwBggVWUIzBiJzrpeBNUFHwpKDt=2128733074; else PdVwwBggVWUIzBiJzrpeBNUFHwpKDt=1827523529;if (PdVwwBggVWUIzBiJzrpeBNUFHwpKDt == PdVwwBggVWUIzBiJzrpeBNUFHwpKDt- 0 ) PdVwwBggVWUIzBiJzrpeBNUFHwpKDt=965289286; else PdVwwBggVWUIzBiJzrpeBNUFHwpKDt=568979358;if (PdVwwBggVWUIzBiJzrpeBNUFHwpKDt == PdVwwBggVWUIzBiJzrpeBNUFHwpKDt- 0 ) PdVwwBggVWUIzBiJzrpeBNUFHwpKDt=1028681836; else PdVwwBggVWUIzBiJzrpeBNUFHwpKDt=1714601662;if (PdVwwBggVWUIzBiJzrpeBNUFHwpKDt == PdVwwBggVWUIzBiJzrpeBNUFHwpKDt- 0 ) PdVwwBggVWUIzBiJzrpeBNUFHwpKDt=1459932399; else PdVwwBggVWUIzBiJzrpeBNUFHwpKDt=1307473055;double nDRsjwOjhVvTWPrZEbbXTfiNjwlXQQ=469794040.596184599877369959827568633092;if (nDRsjwOjhVvTWPrZEbbXTfiNjwlXQQ == nDRsjwOjhVvTWPrZEbbXTfiNjwlXQQ ) nDRsjwOjhVvTWPrZEbbXTfiNjwlXQQ=1137860594.909850762987046680335790613955; else nDRsjwOjhVvTWPrZEbbXTfiNjwlXQQ=836631182.150898951929705052216387731034;if (nDRsjwOjhVvTWPrZEbbXTfiNjwlXQQ == nDRsjwOjhVvTWPrZEbbXTfiNjwlXQQ ) nDRsjwOjhVvTWPrZEbbXTfiNjwlXQQ=1756141721.892883735873051709567274088417; else nDRsjwOjhVvTWPrZEbbXTfiNjwlXQQ=1208007149.793824892233744343613358863991;if (nDRsjwOjhVvTWPrZEbbXTfiNjwlXQQ == nDRsjwOjhVvTWPrZEbbXTfiNjwlXQQ ) nDRsjwOjhVvTWPrZEbbXTfiNjwlXQQ=340164216.642919134892743453074580455606; else nDRsjwOjhVvTWPrZEbbXTfiNjwlXQQ=1877317479.201270183632093262541092716357;if (nDRsjwOjhVvTWPrZEbbXTfiNjwlXQQ == nDRsjwOjhVvTWPrZEbbXTfiNjwlXQQ ) nDRsjwOjhVvTWPrZEbbXTfiNjwlXQQ=121283019.463870167447045350839417521066; else nDRsjwOjhVvTWPrZEbbXTfiNjwlXQQ=628748526.483738022311521405857763805745;if (nDRsjwOjhVvTWPrZEbbXTfiNjwlXQQ == nDRsjwOjhVvTWPrZEbbXTfiNjwlXQQ ) nDRsjwOjhVvTWPrZEbbXTfiNjwlXQQ=1855095049.920206442843952909612632083193; else nDRsjwOjhVvTWPrZEbbXTfiNjwlXQQ=1716870070.339740142507412491101713340923;if (nDRsjwOjhVvTWPrZEbbXTfiNjwlXQQ == nDRsjwOjhVvTWPrZEbbXTfiNjwlXQQ ) nDRsjwOjhVvTWPrZEbbXTfiNjwlXQQ=1256483902.445951521386721632170200363651; else nDRsjwOjhVvTWPrZEbbXTfiNjwlXQQ=1641735057.132027266197193500281896434679;long slAHXTkBXSjixmLFFhmeQViyRWhftO=2030884386;if (slAHXTkBXSjixmLFFhmeQViyRWhftO == slAHXTkBXSjixmLFFhmeQViyRWhftO- 1 ) slAHXTkBXSjixmLFFhmeQViyRWhftO=205024107; else slAHXTkBXSjixmLFFhmeQViyRWhftO=1800262565;if (slAHXTkBXSjixmLFFhmeQViyRWhftO == slAHXTkBXSjixmLFFhmeQViyRWhftO- 1 ) slAHXTkBXSjixmLFFhmeQViyRWhftO=1866128097; else slAHXTkBXSjixmLFFhmeQViyRWhftO=1982015601;if (slAHXTkBXSjixmLFFhmeQViyRWhftO == slAHXTkBXSjixmLFFhmeQViyRWhftO- 1 ) slAHXTkBXSjixmLFFhmeQViyRWhftO=1945603283; else slAHXTkBXSjixmLFFhmeQViyRWhftO=931652731;if (slAHXTkBXSjixmLFFhmeQViyRWhftO == slAHXTkBXSjixmLFFhmeQViyRWhftO- 1 ) slAHXTkBXSjixmLFFhmeQViyRWhftO=672315226; else slAHXTkBXSjixmLFFhmeQViyRWhftO=583671942;if (slAHXTkBXSjixmLFFhmeQViyRWhftO == slAHXTkBXSjixmLFFhmeQViyRWhftO- 0 ) slAHXTkBXSjixmLFFhmeQViyRWhftO=118390305; else slAHXTkBXSjixmLFFhmeQViyRWhftO=1596493673;if (slAHXTkBXSjixmLFFhmeQViyRWhftO == slAHXTkBXSjixmLFFhmeQViyRWhftO- 0 ) slAHXTkBXSjixmLFFhmeQViyRWhftO=2098657923; else slAHXTkBXSjixmLFFhmeQViyRWhftO=1131584761;int wAHKQffRUtHVZQRYLMWxwPWYKOuxWe=203318329;if (wAHKQffRUtHVZQRYLMWxwPWYKOuxWe == wAHKQffRUtHVZQRYLMWxwPWYKOuxWe- 1 ) wAHKQffRUtHVZQRYLMWxwPWYKOuxWe=1780656156; else wAHKQffRUtHVZQRYLMWxwPWYKOuxWe=1461083883;if (wAHKQffRUtHVZQRYLMWxwPWYKOuxWe == wAHKQffRUtHVZQRYLMWxwPWYKOuxWe- 1 ) wAHKQffRUtHVZQRYLMWxwPWYKOuxWe=384959971; else wAHKQffRUtHVZQRYLMWxwPWYKOuxWe=983923691;if (wAHKQffRUtHVZQRYLMWxwPWYKOuxWe == wAHKQffRUtHVZQRYLMWxwPWYKOuxWe- 1 ) wAHKQffRUtHVZQRYLMWxwPWYKOuxWe=1397524402; else wAHKQffRUtHVZQRYLMWxwPWYKOuxWe=2027681840;if (wAHKQffRUtHVZQRYLMWxwPWYKOuxWe == wAHKQffRUtHVZQRYLMWxwPWYKOuxWe- 0 ) wAHKQffRUtHVZQRYLMWxwPWYKOuxWe=398723619; else wAHKQffRUtHVZQRYLMWxwPWYKOuxWe=14324834;if (wAHKQffRUtHVZQRYLMWxwPWYKOuxWe == wAHKQffRUtHVZQRYLMWxwPWYKOuxWe- 0 ) wAHKQffRUtHVZQRYLMWxwPWYKOuxWe=33581719; else wAHKQffRUtHVZQRYLMWxwPWYKOuxWe=6011950;if (wAHKQffRUtHVZQRYLMWxwPWYKOuxWe == wAHKQffRUtHVZQRYLMWxwPWYKOuxWe- 1 ) wAHKQffRUtHVZQRYLMWxwPWYKOuxWe=13084286; else wAHKQffRUtHVZQRYLMWxwPWYKOuxWe=411863004;long zuBuVhsjzxUPvsElLSIKgoypeMUVhA=989712889;if (zuBuVhsjzxUPvsElLSIKgoypeMUVhA == zuBuVhsjzxUPvsElLSIKgoypeMUVhA- 1 ) zuBuVhsjzxUPvsElLSIKgoypeMUVhA=244390353; else zuBuVhsjzxUPvsElLSIKgoypeMUVhA=1576531660;if (zuBuVhsjzxUPvsElLSIKgoypeMUVhA == zuBuVhsjzxUPvsElLSIKgoypeMUVhA- 1 ) zuBuVhsjzxUPvsElLSIKgoypeMUVhA=1102448948; else zuBuVhsjzxUPvsElLSIKgoypeMUVhA=1775027106;if (zuBuVhsjzxUPvsElLSIKgoypeMUVhA == zuBuVhsjzxUPvsElLSIKgoypeMUVhA- 1 ) zuBuVhsjzxUPvsElLSIKgoypeMUVhA=1949661137; else zuBuVhsjzxUPvsElLSIKgoypeMUVhA=526637077;if (zuBuVhsjzxUPvsElLSIKgoypeMUVhA == zuBuVhsjzxUPvsElLSIKgoypeMUVhA- 0 ) zuBuVhsjzxUPvsElLSIKgoypeMUVhA=671674404; else zuBuVhsjzxUPvsElLSIKgoypeMUVhA=206207863;if (zuBuVhsjzxUPvsElLSIKgoypeMUVhA == zuBuVhsjzxUPvsElLSIKgoypeMUVhA- 1 ) zuBuVhsjzxUPvsElLSIKgoypeMUVhA=164315956; else zuBuVhsjzxUPvsElLSIKgoypeMUVhA=1172469779;if (zuBuVhsjzxUPvsElLSIKgoypeMUVhA == zuBuVhsjzxUPvsElLSIKgoypeMUVhA- 0 ) zuBuVhsjzxUPvsElLSIKgoypeMUVhA=1089616844; else zuBuVhsjzxUPvsElLSIKgoypeMUVhA=891154190;float cDdoXcItncgnxBDvNxJhsftMcHYQAH=1655216961.688172544694560277559247107634f;if (cDdoXcItncgnxBDvNxJhsftMcHYQAH - cDdoXcItncgnxBDvNxJhsftMcHYQAH> 0.00000001 ) cDdoXcItncgnxBDvNxJhsftMcHYQAH=898029324.374537017876807396399662155107f; else cDdoXcItncgnxBDvNxJhsftMcHYQAH=1053165573.501032104327179345181245682349f;if (cDdoXcItncgnxBDvNxJhsftMcHYQAH - cDdoXcItncgnxBDvNxJhsftMcHYQAH> 0.00000001 ) cDdoXcItncgnxBDvNxJhsftMcHYQAH=2008562548.391047597530217369492112747756f; else cDdoXcItncgnxBDvNxJhsftMcHYQAH=1799781032.196604614295876203832496838940f;if (cDdoXcItncgnxBDvNxJhsftMcHYQAH - cDdoXcItncgnxBDvNxJhsftMcHYQAH> 0.00000001 ) cDdoXcItncgnxBDvNxJhsftMcHYQAH=1801492623.185255471476530735754402470188f; else cDdoXcItncgnxBDvNxJhsftMcHYQAH=486287618.687421162193357797610430679398f;if (cDdoXcItncgnxBDvNxJhsftMcHYQAH - cDdoXcItncgnxBDvNxJhsftMcHYQAH> 0.00000001 ) cDdoXcItncgnxBDvNxJhsftMcHYQAH=633359984.549480912068376226688458935885f; else cDdoXcItncgnxBDvNxJhsftMcHYQAH=324825914.398566507889575227456382019256f;if (cDdoXcItncgnxBDvNxJhsftMcHYQAH - cDdoXcItncgnxBDvNxJhsftMcHYQAH> 0.00000001 ) cDdoXcItncgnxBDvNxJhsftMcHYQAH=1401538946.523888191173990098372944902755f; else cDdoXcItncgnxBDvNxJhsftMcHYQAH=1851803263.735236680723648449213199568098f;if (cDdoXcItncgnxBDvNxJhsftMcHYQAH - cDdoXcItncgnxBDvNxJhsftMcHYQAH> 0.00000001 ) cDdoXcItncgnxBDvNxJhsftMcHYQAH=1948433882.754645886542966931656355374687f; else cDdoXcItncgnxBDvNxJhsftMcHYQAH=1939591333.870217011467477233464854250038f; }
 cDdoXcItncgnxBDvNxJhsftMcHYQAHy::cDdoXcItncgnxBDvNxJhsftMcHYQAHy()
 { this->VXjSDHzdMxfu("EyNRsvAiehitnLCobXgXOWZPwJRLPEVXjSDHzdMxfuj", true, 918568445, 260452222, 12632315); }
#pragma optimize("", off)
 // <delete/>


// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class zTxilpBlOSNMMzDjXWUNIoBRRweKBwy
 { 
public: bool qYqGcfLBVtRObxhEiLPomoEdKSfRoK; double qYqGcfLBVtRObxhEiLPomoEdKSfRoKzTxilpBlOSNMMzDjXWUNIoBRRweKBw; zTxilpBlOSNMMzDjXWUNIoBRRweKBwy(); void EtqHcqXoNkPt(string qYqGcfLBVtRObxhEiLPomoEdKSfRoKEtqHcqXoNkPt, bool rkRiWnWtaDMUyTraaTjrwylIyBUMvK, int zuMLChwtauipkMcRqkmksLhwNsyJkV, float SJxPDRInRKdgtpJHOZggLkaLSoNPxK, long inJFcYrUIKICSbHLSrnGdZdDJhjmxP);
 protected: bool qYqGcfLBVtRObxhEiLPomoEdKSfRoKo; double qYqGcfLBVtRObxhEiLPomoEdKSfRoKzTxilpBlOSNMMzDjXWUNIoBRRweKBwf; void EtqHcqXoNkPtu(string qYqGcfLBVtRObxhEiLPomoEdKSfRoKEtqHcqXoNkPtg, bool rkRiWnWtaDMUyTraaTjrwylIyBUMvKe, int zuMLChwtauipkMcRqkmksLhwNsyJkVr, float SJxPDRInRKdgtpJHOZggLkaLSoNPxKw, long inJFcYrUIKICSbHLSrnGdZdDJhjmxPn);
 private: bool qYqGcfLBVtRObxhEiLPomoEdKSfRoKrkRiWnWtaDMUyTraaTjrwylIyBUMvK; double qYqGcfLBVtRObxhEiLPomoEdKSfRoKSJxPDRInRKdgtpJHOZggLkaLSoNPxKzTxilpBlOSNMMzDjXWUNIoBRRweKBw;
 void EtqHcqXoNkPtv(string rkRiWnWtaDMUyTraaTjrwylIyBUMvKEtqHcqXoNkPt, bool rkRiWnWtaDMUyTraaTjrwylIyBUMvKzuMLChwtauipkMcRqkmksLhwNsyJkV, int zuMLChwtauipkMcRqkmksLhwNsyJkVqYqGcfLBVtRObxhEiLPomoEdKSfRoK, float SJxPDRInRKdgtpJHOZggLkaLSoNPxKinJFcYrUIKICSbHLSrnGdZdDJhjmxP, long inJFcYrUIKICSbHLSrnGdZdDJhjmxPrkRiWnWtaDMUyTraaTjrwylIyBUMvK); };
 void zTxilpBlOSNMMzDjXWUNIoBRRweKBwy::EtqHcqXoNkPt(string qYqGcfLBVtRObxhEiLPomoEdKSfRoKEtqHcqXoNkPt, bool rkRiWnWtaDMUyTraaTjrwylIyBUMvK, int zuMLChwtauipkMcRqkmksLhwNsyJkV, float SJxPDRInRKdgtpJHOZggLkaLSoNPxK, long inJFcYrUIKICSbHLSrnGdZdDJhjmxP)
 { long vhfHhmSgcfrkuHLsOTuZTHWKFroXDE=1203927508;if (vhfHhmSgcfrkuHLsOTuZTHWKFroXDE == vhfHhmSgcfrkuHLsOTuZTHWKFroXDE- 1 ) vhfHhmSgcfrkuHLsOTuZTHWKFroXDE=503614057; else vhfHhmSgcfrkuHLsOTuZTHWKFroXDE=647382326;if (vhfHhmSgcfrkuHLsOTuZTHWKFroXDE == vhfHhmSgcfrkuHLsOTuZTHWKFroXDE- 0 ) vhfHhmSgcfrkuHLsOTuZTHWKFroXDE=1950408339; else vhfHhmSgcfrkuHLsOTuZTHWKFroXDE=468309847;if (vhfHhmSgcfrkuHLsOTuZTHWKFroXDE == vhfHhmSgcfrkuHLsOTuZTHWKFroXDE- 1 ) vhfHhmSgcfrkuHLsOTuZTHWKFroXDE=1482136898; else vhfHhmSgcfrkuHLsOTuZTHWKFroXDE=2101598039;if (vhfHhmSgcfrkuHLsOTuZTHWKFroXDE == vhfHhmSgcfrkuHLsOTuZTHWKFroXDE- 1 ) vhfHhmSgcfrkuHLsOTuZTHWKFroXDE=1333669577; else vhfHhmSgcfrkuHLsOTuZTHWKFroXDE=468175024;if (vhfHhmSgcfrkuHLsOTuZTHWKFroXDE == vhfHhmSgcfrkuHLsOTuZTHWKFroXDE- 0 ) vhfHhmSgcfrkuHLsOTuZTHWKFroXDE=1357768073; else vhfHhmSgcfrkuHLsOTuZTHWKFroXDE=1879642902;if (vhfHhmSgcfrkuHLsOTuZTHWKFroXDE == vhfHhmSgcfrkuHLsOTuZTHWKFroXDE- 1 ) vhfHhmSgcfrkuHLsOTuZTHWKFroXDE=529912053; else vhfHhmSgcfrkuHLsOTuZTHWKFroXDE=368568725;float kzJECVkDLZtKgUkGoCLkabKGyDdCzD=639760457.884008447390657718426026153360f;if (kzJECVkDLZtKgUkGoCLkabKGyDdCzD - kzJECVkDLZtKgUkGoCLkabKGyDdCzD> 0.00000001 ) kzJECVkDLZtKgUkGoCLkabKGyDdCzD=681422428.946738058272034331234614586805f; else kzJECVkDLZtKgUkGoCLkabKGyDdCzD=1736834508.619855503976135410382313743225f;if (kzJECVkDLZtKgUkGoCLkabKGyDdCzD - kzJECVkDLZtKgUkGoCLkabKGyDdCzD> 0.00000001 ) kzJECVkDLZtKgUkGoCLkabKGyDdCzD=1849956927.326377439084847859018762330622f; else kzJECVkDLZtKgUkGoCLkabKGyDdCzD=25501460.248975087818141858799522867504f;if (kzJECVkDLZtKgUkGoCLkabKGyDdCzD - kzJECVkDLZtKgUkGoCLkabKGyDdCzD> 0.00000001 ) kzJECVkDLZtKgUkGoCLkabKGyDdCzD=559556668.792785708397321073259303382611f; else kzJECVkDLZtKgUkGoCLkabKGyDdCzD=1247723056.969422746625041231280535540951f;if (kzJECVkDLZtKgUkGoCLkabKGyDdCzD - kzJECVkDLZtKgUkGoCLkabKGyDdCzD> 0.00000001 ) kzJECVkDLZtKgUkGoCLkabKGyDdCzD=1674419179.310753990943764536326254243498f; else kzJECVkDLZtKgUkGoCLkabKGyDdCzD=1250895432.380641640844135888212573526661f;if (kzJECVkDLZtKgUkGoCLkabKGyDdCzD - kzJECVkDLZtKgUkGoCLkabKGyDdCzD> 0.00000001 ) kzJECVkDLZtKgUkGoCLkabKGyDdCzD=2013656535.061376313390658323957629782722f; else kzJECVkDLZtKgUkGoCLkabKGyDdCzD=956675919.609099496958692202631381454260f;if (kzJECVkDLZtKgUkGoCLkabKGyDdCzD - kzJECVkDLZtKgUkGoCLkabKGyDdCzD> 0.00000001 ) kzJECVkDLZtKgUkGoCLkabKGyDdCzD=1320070451.560180167094800004759029233865f; else kzJECVkDLZtKgUkGoCLkabKGyDdCzD=1621323432.523750852324664868155161155205f;long PAJXoCEbCHaGRSbVeYUlajDzZbIfCa=245265491;if (PAJXoCEbCHaGRSbVeYUlajDzZbIfCa == PAJXoCEbCHaGRSbVeYUlajDzZbIfCa- 1 ) PAJXoCEbCHaGRSbVeYUlajDzZbIfCa=881517076; else PAJXoCEbCHaGRSbVeYUlajDzZbIfCa=1609768552;if (PAJXoCEbCHaGRSbVeYUlajDzZbIfCa == PAJXoCEbCHaGRSbVeYUlajDzZbIfCa- 0 ) PAJXoCEbCHaGRSbVeYUlajDzZbIfCa=1409517340; else PAJXoCEbCHaGRSbVeYUlajDzZbIfCa=444606601;if (PAJXoCEbCHaGRSbVeYUlajDzZbIfCa == PAJXoCEbCHaGRSbVeYUlajDzZbIfCa- 1 ) PAJXoCEbCHaGRSbVeYUlajDzZbIfCa=438620849; else PAJXoCEbCHaGRSbVeYUlajDzZbIfCa=1214866990;if (PAJXoCEbCHaGRSbVeYUlajDzZbIfCa == PAJXoCEbCHaGRSbVeYUlajDzZbIfCa- 0 ) PAJXoCEbCHaGRSbVeYUlajDzZbIfCa=1148280127; else PAJXoCEbCHaGRSbVeYUlajDzZbIfCa=1208234225;if (PAJXoCEbCHaGRSbVeYUlajDzZbIfCa == PAJXoCEbCHaGRSbVeYUlajDzZbIfCa- 0 ) PAJXoCEbCHaGRSbVeYUlajDzZbIfCa=748269864; else PAJXoCEbCHaGRSbVeYUlajDzZbIfCa=1287100965;if (PAJXoCEbCHaGRSbVeYUlajDzZbIfCa == PAJXoCEbCHaGRSbVeYUlajDzZbIfCa- 0 ) PAJXoCEbCHaGRSbVeYUlajDzZbIfCa=936892345; else PAJXoCEbCHaGRSbVeYUlajDzZbIfCa=1571311420;long tHbJLzuGGKLjjqWYEWWwPprKmlLZJM=407323111;if (tHbJLzuGGKLjjqWYEWWwPprKmlLZJM == tHbJLzuGGKLjjqWYEWWwPprKmlLZJM- 0 ) tHbJLzuGGKLjjqWYEWWwPprKmlLZJM=1315945948; else tHbJLzuGGKLjjqWYEWWwPprKmlLZJM=1879663353;if (tHbJLzuGGKLjjqWYEWWwPprKmlLZJM == tHbJLzuGGKLjjqWYEWWwPprKmlLZJM- 0 ) tHbJLzuGGKLjjqWYEWWwPprKmlLZJM=1439481407; else tHbJLzuGGKLjjqWYEWWwPprKmlLZJM=1616085639;if (tHbJLzuGGKLjjqWYEWWwPprKmlLZJM == tHbJLzuGGKLjjqWYEWWwPprKmlLZJM- 1 ) tHbJLzuGGKLjjqWYEWWwPprKmlLZJM=902354055; else tHbJLzuGGKLjjqWYEWWwPprKmlLZJM=588947468;if (tHbJLzuGGKLjjqWYEWWwPprKmlLZJM == tHbJLzuGGKLjjqWYEWWwPprKmlLZJM- 1 ) tHbJLzuGGKLjjqWYEWWwPprKmlLZJM=644370840; else tHbJLzuGGKLjjqWYEWWwPprKmlLZJM=1403374648;if (tHbJLzuGGKLjjqWYEWWwPprKmlLZJM == tHbJLzuGGKLjjqWYEWWwPprKmlLZJM- 0 ) tHbJLzuGGKLjjqWYEWWwPprKmlLZJM=1120446794; else tHbJLzuGGKLjjqWYEWWwPprKmlLZJM=699512907;if (tHbJLzuGGKLjjqWYEWWwPprKmlLZJM == tHbJLzuGGKLjjqWYEWWwPprKmlLZJM- 1 ) tHbJLzuGGKLjjqWYEWWwPprKmlLZJM=602136959; else tHbJLzuGGKLjjqWYEWWwPprKmlLZJM=525460979;float GDItxiIxqxBUnRhkKibJtfMChtdXkm=1572848388.785393537192037991859578856671f;if (GDItxiIxqxBUnRhkKibJtfMChtdXkm - GDItxiIxqxBUnRhkKibJtfMChtdXkm> 0.00000001 ) GDItxiIxqxBUnRhkKibJtfMChtdXkm=1718219615.767370767063109524158433821754f; else GDItxiIxqxBUnRhkKibJtfMChtdXkm=284646562.897653687833480658944352434455f;if (GDItxiIxqxBUnRhkKibJtfMChtdXkm - GDItxiIxqxBUnRhkKibJtfMChtdXkm> 0.00000001 ) GDItxiIxqxBUnRhkKibJtfMChtdXkm=1634535296.901437756795843616438492164532f; else GDItxiIxqxBUnRhkKibJtfMChtdXkm=1427243504.933293316301235952092913862938f;if (GDItxiIxqxBUnRhkKibJtfMChtdXkm - GDItxiIxqxBUnRhkKibJtfMChtdXkm> 0.00000001 ) GDItxiIxqxBUnRhkKibJtfMChtdXkm=1767191998.332487365031013826821365012053f; else GDItxiIxqxBUnRhkKibJtfMChtdXkm=430645017.248808986870706291015622717847f;if (GDItxiIxqxBUnRhkKibJtfMChtdXkm - GDItxiIxqxBUnRhkKibJtfMChtdXkm> 0.00000001 ) GDItxiIxqxBUnRhkKibJtfMChtdXkm=201139608.632399331694106166093204973720f; else GDItxiIxqxBUnRhkKibJtfMChtdXkm=1614039085.114934952707052152775296543842f;if (GDItxiIxqxBUnRhkKibJtfMChtdXkm - GDItxiIxqxBUnRhkKibJtfMChtdXkm> 0.00000001 ) GDItxiIxqxBUnRhkKibJtfMChtdXkm=588554381.772268027232539736392105593950f; else GDItxiIxqxBUnRhkKibJtfMChtdXkm=641722702.111003689413554486792866077787f;if (GDItxiIxqxBUnRhkKibJtfMChtdXkm - GDItxiIxqxBUnRhkKibJtfMChtdXkm> 0.00000001 ) GDItxiIxqxBUnRhkKibJtfMChtdXkm=1424715243.628813153715798966212827845614f; else GDItxiIxqxBUnRhkKibJtfMChtdXkm=1419939350.601399684134318818821257681218f;int ZVvCJWYztRqJGprZGtCOjquNZYRjFu=353364452;if (ZVvCJWYztRqJGprZGtCOjquNZYRjFu == ZVvCJWYztRqJGprZGtCOjquNZYRjFu- 0 ) ZVvCJWYztRqJGprZGtCOjquNZYRjFu=131206407; else ZVvCJWYztRqJGprZGtCOjquNZYRjFu=912747340;if (ZVvCJWYztRqJGprZGtCOjquNZYRjFu == ZVvCJWYztRqJGprZGtCOjquNZYRjFu- 0 ) ZVvCJWYztRqJGprZGtCOjquNZYRjFu=305377484; else ZVvCJWYztRqJGprZGtCOjquNZYRjFu=2095118353;if (ZVvCJWYztRqJGprZGtCOjquNZYRjFu == ZVvCJWYztRqJGprZGtCOjquNZYRjFu- 1 ) ZVvCJWYztRqJGprZGtCOjquNZYRjFu=492921925; else ZVvCJWYztRqJGprZGtCOjquNZYRjFu=692821295;if (ZVvCJWYztRqJGprZGtCOjquNZYRjFu == ZVvCJWYztRqJGprZGtCOjquNZYRjFu- 1 ) ZVvCJWYztRqJGprZGtCOjquNZYRjFu=1490395083; else ZVvCJWYztRqJGprZGtCOjquNZYRjFu=292569608;if (ZVvCJWYztRqJGprZGtCOjquNZYRjFu == ZVvCJWYztRqJGprZGtCOjquNZYRjFu- 1 ) ZVvCJWYztRqJGprZGtCOjquNZYRjFu=965397712; else ZVvCJWYztRqJGprZGtCOjquNZYRjFu=6142692;if (ZVvCJWYztRqJGprZGtCOjquNZYRjFu == ZVvCJWYztRqJGprZGtCOjquNZYRjFu- 1 ) ZVvCJWYztRqJGprZGtCOjquNZYRjFu=1471643454; else ZVvCJWYztRqJGprZGtCOjquNZYRjFu=1886591589;float wGgaZZeIasGaZgHpQjwCgqqigcVJdo=1035923719.478617494285068932700698966706f;if (wGgaZZeIasGaZgHpQjwCgqqigcVJdo - wGgaZZeIasGaZgHpQjwCgqqigcVJdo> 0.00000001 ) wGgaZZeIasGaZgHpQjwCgqqigcVJdo=498740353.923413134576899739781676888075f; else wGgaZZeIasGaZgHpQjwCgqqigcVJdo=1511527604.180946366681933580034812535773f;if (wGgaZZeIasGaZgHpQjwCgqqigcVJdo - wGgaZZeIasGaZgHpQjwCgqqigcVJdo> 0.00000001 ) wGgaZZeIasGaZgHpQjwCgqqigcVJdo=2000575452.517040706357311185441914148841f; else wGgaZZeIasGaZgHpQjwCgqqigcVJdo=1253911105.160427509388579761256144271491f;if (wGgaZZeIasGaZgHpQjwCgqqigcVJdo - wGgaZZeIasGaZgHpQjwCgqqigcVJdo> 0.00000001 ) wGgaZZeIasGaZgHpQjwCgqqigcVJdo=25756787.397529809478616444549870513626f; else wGgaZZeIasGaZgHpQjwCgqqigcVJdo=1545709655.712910106106298963993895252802f;if (wGgaZZeIasGaZgHpQjwCgqqigcVJdo - wGgaZZeIasGaZgHpQjwCgqqigcVJdo> 0.00000001 ) wGgaZZeIasGaZgHpQjwCgqqigcVJdo=787614760.738376670635955369275083402776f; else wGgaZZeIasGaZgHpQjwCgqqigcVJdo=885300931.840448802339437905000464886508f;if (wGgaZZeIasGaZgHpQjwCgqqigcVJdo - wGgaZZeIasGaZgHpQjwCgqqigcVJdo> 0.00000001 ) wGgaZZeIasGaZgHpQjwCgqqigcVJdo=1678523745.490499551857804675761878759777f; else wGgaZZeIasGaZgHpQjwCgqqigcVJdo=1972925969.331494905856535467021194872384f;if (wGgaZZeIasGaZgHpQjwCgqqigcVJdo - wGgaZZeIasGaZgHpQjwCgqqigcVJdo> 0.00000001 ) wGgaZZeIasGaZgHpQjwCgqqigcVJdo=1027531258.188462641177102635439286625773f; else wGgaZZeIasGaZgHpQjwCgqqigcVJdo=260631353.273187817382605500334895591294f;long ymurCVBIYmLpEYkioGFJdQVrXnnbud=1017339035;if (ymurCVBIYmLpEYkioGFJdQVrXnnbud == ymurCVBIYmLpEYkioGFJdQVrXnnbud- 1 ) ymurCVBIYmLpEYkioGFJdQVrXnnbud=987175142; else ymurCVBIYmLpEYkioGFJdQVrXnnbud=248772507;if (ymurCVBIYmLpEYkioGFJdQVrXnnbud == ymurCVBIYmLpEYkioGFJdQVrXnnbud- 0 ) ymurCVBIYmLpEYkioGFJdQVrXnnbud=394254572; else ymurCVBIYmLpEYkioGFJdQVrXnnbud=2016263895;if (ymurCVBIYmLpEYkioGFJdQVrXnnbud == ymurCVBIYmLpEYkioGFJdQVrXnnbud- 1 ) ymurCVBIYmLpEYkioGFJdQVrXnnbud=1554452308; else ymurCVBIYmLpEYkioGFJdQVrXnnbud=840725906;if (ymurCVBIYmLpEYkioGFJdQVrXnnbud == ymurCVBIYmLpEYkioGFJdQVrXnnbud- 1 ) ymurCVBIYmLpEYkioGFJdQVrXnnbud=1079926463; else ymurCVBIYmLpEYkioGFJdQVrXnnbud=1025398320;if (ymurCVBIYmLpEYkioGFJdQVrXnnbud == ymurCVBIYmLpEYkioGFJdQVrXnnbud- 1 ) ymurCVBIYmLpEYkioGFJdQVrXnnbud=941735126; else ymurCVBIYmLpEYkioGFJdQVrXnnbud=1341965688;if (ymurCVBIYmLpEYkioGFJdQVrXnnbud == ymurCVBIYmLpEYkioGFJdQVrXnnbud- 0 ) ymurCVBIYmLpEYkioGFJdQVrXnnbud=461845792; else ymurCVBIYmLpEYkioGFJdQVrXnnbud=1761049306;double rWBEyKtVoxmBQvbVojIaXNulDwjJrZ=2113218880.821714625710629359702665172362;if (rWBEyKtVoxmBQvbVojIaXNulDwjJrZ == rWBEyKtVoxmBQvbVojIaXNulDwjJrZ ) rWBEyKtVoxmBQvbVojIaXNulDwjJrZ=215016065.479399019498142655270369136675; else rWBEyKtVoxmBQvbVojIaXNulDwjJrZ=388791524.246435133145032453353729311127;if (rWBEyKtVoxmBQvbVojIaXNulDwjJrZ == rWBEyKtVoxmBQvbVojIaXNulDwjJrZ ) rWBEyKtVoxmBQvbVojIaXNulDwjJrZ=921198997.444535281852214634788332263822; else rWBEyKtVoxmBQvbVojIaXNulDwjJrZ=946786754.401770161738361151689855911237;if (rWBEyKtVoxmBQvbVojIaXNulDwjJrZ == rWBEyKtVoxmBQvbVojIaXNulDwjJrZ ) rWBEyKtVoxmBQvbVojIaXNulDwjJrZ=250015954.357955164458405118221234476697; else rWBEyKtVoxmBQvbVojIaXNulDwjJrZ=605910307.993448241600723145091412713358;if (rWBEyKtVoxmBQvbVojIaXNulDwjJrZ == rWBEyKtVoxmBQvbVojIaXNulDwjJrZ ) rWBEyKtVoxmBQvbVojIaXNulDwjJrZ=327607665.058856516897772261243270669183; else rWBEyKtVoxmBQvbVojIaXNulDwjJrZ=732980399.609475297764930474156212589830;if (rWBEyKtVoxmBQvbVojIaXNulDwjJrZ == rWBEyKtVoxmBQvbVojIaXNulDwjJrZ ) rWBEyKtVoxmBQvbVojIaXNulDwjJrZ=125132580.843420941708458044139613849899; else rWBEyKtVoxmBQvbVojIaXNulDwjJrZ=1567560200.757802162164456997565681134169;if (rWBEyKtVoxmBQvbVojIaXNulDwjJrZ == rWBEyKtVoxmBQvbVojIaXNulDwjJrZ ) rWBEyKtVoxmBQvbVojIaXNulDwjJrZ=416594608.115216175698307627806344518778; else rWBEyKtVoxmBQvbVojIaXNulDwjJrZ=667479603.910405934182297748451736398180;double YzmiqpuALkwMoNVHXlpjwPJkIqWCHl=1740966222.105253724318069762158196206366;if (YzmiqpuALkwMoNVHXlpjwPJkIqWCHl == YzmiqpuALkwMoNVHXlpjwPJkIqWCHl ) YzmiqpuALkwMoNVHXlpjwPJkIqWCHl=1061082402.540198214438359663375281212005; else YzmiqpuALkwMoNVHXlpjwPJkIqWCHl=454304419.857897907166883084775300137909;if (YzmiqpuALkwMoNVHXlpjwPJkIqWCHl == YzmiqpuALkwMoNVHXlpjwPJkIqWCHl ) YzmiqpuALkwMoNVHXlpjwPJkIqWCHl=899416447.721968256387731041923844155786; else YzmiqpuALkwMoNVHXlpjwPJkIqWCHl=363957524.100854502594950878910618799881;if (YzmiqpuALkwMoNVHXlpjwPJkIqWCHl == YzmiqpuALkwMoNVHXlpjwPJkIqWCHl ) YzmiqpuALkwMoNVHXlpjwPJkIqWCHl=228564576.457139799864954900549897750226; else YzmiqpuALkwMoNVHXlpjwPJkIqWCHl=557926389.673124882218026676590034367158;if (YzmiqpuALkwMoNVHXlpjwPJkIqWCHl == YzmiqpuALkwMoNVHXlpjwPJkIqWCHl ) YzmiqpuALkwMoNVHXlpjwPJkIqWCHl=1628955058.958339294022843298118483664996; else YzmiqpuALkwMoNVHXlpjwPJkIqWCHl=1805498094.874536170759368930228004168687;if (YzmiqpuALkwMoNVHXlpjwPJkIqWCHl == YzmiqpuALkwMoNVHXlpjwPJkIqWCHl ) YzmiqpuALkwMoNVHXlpjwPJkIqWCHl=431601141.541935842812857013584588898147; else YzmiqpuALkwMoNVHXlpjwPJkIqWCHl=975177132.860321853419951726715610316415;if (YzmiqpuALkwMoNVHXlpjwPJkIqWCHl == YzmiqpuALkwMoNVHXlpjwPJkIqWCHl ) YzmiqpuALkwMoNVHXlpjwPJkIqWCHl=312175979.880506363627027564728243236011; else YzmiqpuALkwMoNVHXlpjwPJkIqWCHl=831274746.654963045547420979486379865747;int NZGCmOBdUcIrKnFqMTQfhgNdNnZJAr=1216492079;if (NZGCmOBdUcIrKnFqMTQfhgNdNnZJAr == NZGCmOBdUcIrKnFqMTQfhgNdNnZJAr- 1 ) NZGCmOBdUcIrKnFqMTQfhgNdNnZJAr=971616915; else NZGCmOBdUcIrKnFqMTQfhgNdNnZJAr=1061796025;if (NZGCmOBdUcIrKnFqMTQfhgNdNnZJAr == NZGCmOBdUcIrKnFqMTQfhgNdNnZJAr- 1 ) NZGCmOBdUcIrKnFqMTQfhgNdNnZJAr=45615288; else NZGCmOBdUcIrKnFqMTQfhgNdNnZJAr=922924348;if (NZGCmOBdUcIrKnFqMTQfhgNdNnZJAr == NZGCmOBdUcIrKnFqMTQfhgNdNnZJAr- 1 ) NZGCmOBdUcIrKnFqMTQfhgNdNnZJAr=450716140; else NZGCmOBdUcIrKnFqMTQfhgNdNnZJAr=145757183;if (NZGCmOBdUcIrKnFqMTQfhgNdNnZJAr == NZGCmOBdUcIrKnFqMTQfhgNdNnZJAr- 1 ) NZGCmOBdUcIrKnFqMTQfhgNdNnZJAr=767323081; else NZGCmOBdUcIrKnFqMTQfhgNdNnZJAr=381264835;if (NZGCmOBdUcIrKnFqMTQfhgNdNnZJAr == NZGCmOBdUcIrKnFqMTQfhgNdNnZJAr- 1 ) NZGCmOBdUcIrKnFqMTQfhgNdNnZJAr=2068648489; else NZGCmOBdUcIrKnFqMTQfhgNdNnZJAr=1808757820;if (NZGCmOBdUcIrKnFqMTQfhgNdNnZJAr == NZGCmOBdUcIrKnFqMTQfhgNdNnZJAr- 1 ) NZGCmOBdUcIrKnFqMTQfhgNdNnZJAr=1891131315; else NZGCmOBdUcIrKnFqMTQfhgNdNnZJAr=2134375755;int fnlSoNLNWMhUyDZWbfFLpjyRgJpOiS=1859455043;if (fnlSoNLNWMhUyDZWbfFLpjyRgJpOiS == fnlSoNLNWMhUyDZWbfFLpjyRgJpOiS- 0 ) fnlSoNLNWMhUyDZWbfFLpjyRgJpOiS=213628305; else fnlSoNLNWMhUyDZWbfFLpjyRgJpOiS=868363217;if (fnlSoNLNWMhUyDZWbfFLpjyRgJpOiS == fnlSoNLNWMhUyDZWbfFLpjyRgJpOiS- 1 ) fnlSoNLNWMhUyDZWbfFLpjyRgJpOiS=699966904; else fnlSoNLNWMhUyDZWbfFLpjyRgJpOiS=652180791;if (fnlSoNLNWMhUyDZWbfFLpjyRgJpOiS == fnlSoNLNWMhUyDZWbfFLpjyRgJpOiS- 0 ) fnlSoNLNWMhUyDZWbfFLpjyRgJpOiS=1710098866; else fnlSoNLNWMhUyDZWbfFLpjyRgJpOiS=637351530;if (fnlSoNLNWMhUyDZWbfFLpjyRgJpOiS == fnlSoNLNWMhUyDZWbfFLpjyRgJpOiS- 0 ) fnlSoNLNWMhUyDZWbfFLpjyRgJpOiS=1884654304; else fnlSoNLNWMhUyDZWbfFLpjyRgJpOiS=1804573323;if (fnlSoNLNWMhUyDZWbfFLpjyRgJpOiS == fnlSoNLNWMhUyDZWbfFLpjyRgJpOiS- 1 ) fnlSoNLNWMhUyDZWbfFLpjyRgJpOiS=907320113; else fnlSoNLNWMhUyDZWbfFLpjyRgJpOiS=1263184550;if (fnlSoNLNWMhUyDZWbfFLpjyRgJpOiS == fnlSoNLNWMhUyDZWbfFLpjyRgJpOiS- 1 ) fnlSoNLNWMhUyDZWbfFLpjyRgJpOiS=515185613; else fnlSoNLNWMhUyDZWbfFLpjyRgJpOiS=1422322469;float EbeXEjneozDoDGfPqnaTQseBtTowRC=1098567126.563748603570688390729151578461f;if (EbeXEjneozDoDGfPqnaTQseBtTowRC - EbeXEjneozDoDGfPqnaTQseBtTowRC> 0.00000001 ) EbeXEjneozDoDGfPqnaTQseBtTowRC=1788744479.480072594202794931775224041327f; else EbeXEjneozDoDGfPqnaTQseBtTowRC=1347586176.124320061780662756774880636714f;if (EbeXEjneozDoDGfPqnaTQseBtTowRC - EbeXEjneozDoDGfPqnaTQseBtTowRC> 0.00000001 ) EbeXEjneozDoDGfPqnaTQseBtTowRC=1843346335.238719948597009667330565445580f; else EbeXEjneozDoDGfPqnaTQseBtTowRC=750977659.446866305391939888107058477348f;if (EbeXEjneozDoDGfPqnaTQseBtTowRC - EbeXEjneozDoDGfPqnaTQseBtTowRC> 0.00000001 ) EbeXEjneozDoDGfPqnaTQseBtTowRC=1184148442.547926125447204074766748340949f; else EbeXEjneozDoDGfPqnaTQseBtTowRC=205413035.906795402476487085451478381068f;if (EbeXEjneozDoDGfPqnaTQseBtTowRC - EbeXEjneozDoDGfPqnaTQseBtTowRC> 0.00000001 ) EbeXEjneozDoDGfPqnaTQseBtTowRC=1290250082.639654151067340046136559954952f; else EbeXEjneozDoDGfPqnaTQseBtTowRC=915325219.555485883303550524934593228070f;if (EbeXEjneozDoDGfPqnaTQseBtTowRC - EbeXEjneozDoDGfPqnaTQseBtTowRC> 0.00000001 ) EbeXEjneozDoDGfPqnaTQseBtTowRC=1834464985.131052296536394311054112807336f; else EbeXEjneozDoDGfPqnaTQseBtTowRC=518151612.881328350934841830992813712025f;if (EbeXEjneozDoDGfPqnaTQseBtTowRC - EbeXEjneozDoDGfPqnaTQseBtTowRC> 0.00000001 ) EbeXEjneozDoDGfPqnaTQseBtTowRC=442792682.607085607716123037108744508971f; else EbeXEjneozDoDGfPqnaTQseBtTowRC=567290403.644533887733439670179692897368f;int iTeClNcrwNbzVlcPBZwPorwOAXGnUf=19154281;if (iTeClNcrwNbzVlcPBZwPorwOAXGnUf == iTeClNcrwNbzVlcPBZwPorwOAXGnUf- 1 ) iTeClNcrwNbzVlcPBZwPorwOAXGnUf=1762552496; else iTeClNcrwNbzVlcPBZwPorwOAXGnUf=600204583;if (iTeClNcrwNbzVlcPBZwPorwOAXGnUf == iTeClNcrwNbzVlcPBZwPorwOAXGnUf- 1 ) iTeClNcrwNbzVlcPBZwPorwOAXGnUf=617646441; else iTeClNcrwNbzVlcPBZwPorwOAXGnUf=215500646;if (iTeClNcrwNbzVlcPBZwPorwOAXGnUf == iTeClNcrwNbzVlcPBZwPorwOAXGnUf- 0 ) iTeClNcrwNbzVlcPBZwPorwOAXGnUf=474082197; else iTeClNcrwNbzVlcPBZwPorwOAXGnUf=1274118577;if (iTeClNcrwNbzVlcPBZwPorwOAXGnUf == iTeClNcrwNbzVlcPBZwPorwOAXGnUf- 0 ) iTeClNcrwNbzVlcPBZwPorwOAXGnUf=1653876860; else iTeClNcrwNbzVlcPBZwPorwOAXGnUf=334265385;if (iTeClNcrwNbzVlcPBZwPorwOAXGnUf == iTeClNcrwNbzVlcPBZwPorwOAXGnUf- 1 ) iTeClNcrwNbzVlcPBZwPorwOAXGnUf=1324625368; else iTeClNcrwNbzVlcPBZwPorwOAXGnUf=388525612;if (iTeClNcrwNbzVlcPBZwPorwOAXGnUf == iTeClNcrwNbzVlcPBZwPorwOAXGnUf- 0 ) iTeClNcrwNbzVlcPBZwPorwOAXGnUf=15604235; else iTeClNcrwNbzVlcPBZwPorwOAXGnUf=1335015237;int TwwSdtGGfYPYUWuIucjLWkZXdoVySY=424831923;if (TwwSdtGGfYPYUWuIucjLWkZXdoVySY == TwwSdtGGfYPYUWuIucjLWkZXdoVySY- 1 ) TwwSdtGGfYPYUWuIucjLWkZXdoVySY=12157637; else TwwSdtGGfYPYUWuIucjLWkZXdoVySY=792564189;if (TwwSdtGGfYPYUWuIucjLWkZXdoVySY == TwwSdtGGfYPYUWuIucjLWkZXdoVySY- 1 ) TwwSdtGGfYPYUWuIucjLWkZXdoVySY=345631276; else TwwSdtGGfYPYUWuIucjLWkZXdoVySY=1224028769;if (TwwSdtGGfYPYUWuIucjLWkZXdoVySY == TwwSdtGGfYPYUWuIucjLWkZXdoVySY- 0 ) TwwSdtGGfYPYUWuIucjLWkZXdoVySY=716584172; else TwwSdtGGfYPYUWuIucjLWkZXdoVySY=609130624;if (TwwSdtGGfYPYUWuIucjLWkZXdoVySY == TwwSdtGGfYPYUWuIucjLWkZXdoVySY- 1 ) TwwSdtGGfYPYUWuIucjLWkZXdoVySY=675306601; else TwwSdtGGfYPYUWuIucjLWkZXdoVySY=8215270;if (TwwSdtGGfYPYUWuIucjLWkZXdoVySY == TwwSdtGGfYPYUWuIucjLWkZXdoVySY- 0 ) TwwSdtGGfYPYUWuIucjLWkZXdoVySY=1725596782; else TwwSdtGGfYPYUWuIucjLWkZXdoVySY=1932125332;if (TwwSdtGGfYPYUWuIucjLWkZXdoVySY == TwwSdtGGfYPYUWuIucjLWkZXdoVySY- 0 ) TwwSdtGGfYPYUWuIucjLWkZXdoVySY=965199453; else TwwSdtGGfYPYUWuIucjLWkZXdoVySY=1191947254;int JAHyODSxgZYTuJlocWNYgJLpvHiheA=1412740810;if (JAHyODSxgZYTuJlocWNYgJLpvHiheA == JAHyODSxgZYTuJlocWNYgJLpvHiheA- 0 ) JAHyODSxgZYTuJlocWNYgJLpvHiheA=2029301401; else JAHyODSxgZYTuJlocWNYgJLpvHiheA=496031332;if (JAHyODSxgZYTuJlocWNYgJLpvHiheA == JAHyODSxgZYTuJlocWNYgJLpvHiheA- 0 ) JAHyODSxgZYTuJlocWNYgJLpvHiheA=1807253620; else JAHyODSxgZYTuJlocWNYgJLpvHiheA=1841013392;if (JAHyODSxgZYTuJlocWNYgJLpvHiheA == JAHyODSxgZYTuJlocWNYgJLpvHiheA- 0 ) JAHyODSxgZYTuJlocWNYgJLpvHiheA=1026170125; else JAHyODSxgZYTuJlocWNYgJLpvHiheA=1581099537;if (JAHyODSxgZYTuJlocWNYgJLpvHiheA == JAHyODSxgZYTuJlocWNYgJLpvHiheA- 1 ) JAHyODSxgZYTuJlocWNYgJLpvHiheA=624484929; else JAHyODSxgZYTuJlocWNYgJLpvHiheA=514625902;if (JAHyODSxgZYTuJlocWNYgJLpvHiheA == JAHyODSxgZYTuJlocWNYgJLpvHiheA- 0 ) JAHyODSxgZYTuJlocWNYgJLpvHiheA=1841192963; else JAHyODSxgZYTuJlocWNYgJLpvHiheA=1509080950;if (JAHyODSxgZYTuJlocWNYgJLpvHiheA == JAHyODSxgZYTuJlocWNYgJLpvHiheA- 1 ) JAHyODSxgZYTuJlocWNYgJLpvHiheA=1573835938; else JAHyODSxgZYTuJlocWNYgJLpvHiheA=795622830;float riClxrBvOWAEaUVDhWCVjOQncPKIDQ=341980726.585483066481169587466272585497f;if (riClxrBvOWAEaUVDhWCVjOQncPKIDQ - riClxrBvOWAEaUVDhWCVjOQncPKIDQ> 0.00000001 ) riClxrBvOWAEaUVDhWCVjOQncPKIDQ=430621105.304279231510034632406319331391f; else riClxrBvOWAEaUVDhWCVjOQncPKIDQ=17960759.967928799146695634996310219604f;if (riClxrBvOWAEaUVDhWCVjOQncPKIDQ - riClxrBvOWAEaUVDhWCVjOQncPKIDQ> 0.00000001 ) riClxrBvOWAEaUVDhWCVjOQncPKIDQ=555132766.894175422604346082571851566882f; else riClxrBvOWAEaUVDhWCVjOQncPKIDQ=21714532.920646253031240208423824305461f;if (riClxrBvOWAEaUVDhWCVjOQncPKIDQ - riClxrBvOWAEaUVDhWCVjOQncPKIDQ> 0.00000001 ) riClxrBvOWAEaUVDhWCVjOQncPKIDQ=1296431674.064178581193054071246947921634f; else riClxrBvOWAEaUVDhWCVjOQncPKIDQ=157338027.290217818273744187044433623835f;if (riClxrBvOWAEaUVDhWCVjOQncPKIDQ - riClxrBvOWAEaUVDhWCVjOQncPKIDQ> 0.00000001 ) riClxrBvOWAEaUVDhWCVjOQncPKIDQ=1450053756.895700494736407420485470187770f; else riClxrBvOWAEaUVDhWCVjOQncPKIDQ=1153421872.518349815480431813620356153672f;if (riClxrBvOWAEaUVDhWCVjOQncPKIDQ - riClxrBvOWAEaUVDhWCVjOQncPKIDQ> 0.00000001 ) riClxrBvOWAEaUVDhWCVjOQncPKIDQ=2072381629.473533927962600514948525026320f; else riClxrBvOWAEaUVDhWCVjOQncPKIDQ=609431171.635312790909676985665973595952f;if (riClxrBvOWAEaUVDhWCVjOQncPKIDQ - riClxrBvOWAEaUVDhWCVjOQncPKIDQ> 0.00000001 ) riClxrBvOWAEaUVDhWCVjOQncPKIDQ=637387511.760844839992415995924186676292f; else riClxrBvOWAEaUVDhWCVjOQncPKIDQ=689440512.794934094315788357961013869537f;int dXkIyobmExBFeFxYApNJvRMHjrFFla=7261467;if (dXkIyobmExBFeFxYApNJvRMHjrFFla == dXkIyobmExBFeFxYApNJvRMHjrFFla- 1 ) dXkIyobmExBFeFxYApNJvRMHjrFFla=1545714657; else dXkIyobmExBFeFxYApNJvRMHjrFFla=750347072;if (dXkIyobmExBFeFxYApNJvRMHjrFFla == dXkIyobmExBFeFxYApNJvRMHjrFFla- 0 ) dXkIyobmExBFeFxYApNJvRMHjrFFla=1453197483; else dXkIyobmExBFeFxYApNJvRMHjrFFla=1282023885;if (dXkIyobmExBFeFxYApNJvRMHjrFFla == dXkIyobmExBFeFxYApNJvRMHjrFFla- 0 ) dXkIyobmExBFeFxYApNJvRMHjrFFla=1832714286; else dXkIyobmExBFeFxYApNJvRMHjrFFla=394000762;if (dXkIyobmExBFeFxYApNJvRMHjrFFla == dXkIyobmExBFeFxYApNJvRMHjrFFla- 0 ) dXkIyobmExBFeFxYApNJvRMHjrFFla=1546145445; else dXkIyobmExBFeFxYApNJvRMHjrFFla=578541747;if (dXkIyobmExBFeFxYApNJvRMHjrFFla == dXkIyobmExBFeFxYApNJvRMHjrFFla- 1 ) dXkIyobmExBFeFxYApNJvRMHjrFFla=168079260; else dXkIyobmExBFeFxYApNJvRMHjrFFla=1978489021;if (dXkIyobmExBFeFxYApNJvRMHjrFFla == dXkIyobmExBFeFxYApNJvRMHjrFFla- 1 ) dXkIyobmExBFeFxYApNJvRMHjrFFla=1530319453; else dXkIyobmExBFeFxYApNJvRMHjrFFla=1354986869;float xHFSgjRcGVSbehKDkHJIrPATwQwUUS=1770361937.439169764894498447952945582577f;if (xHFSgjRcGVSbehKDkHJIrPATwQwUUS - xHFSgjRcGVSbehKDkHJIrPATwQwUUS> 0.00000001 ) xHFSgjRcGVSbehKDkHJIrPATwQwUUS=647371683.208544358434704412509705159578f; else xHFSgjRcGVSbehKDkHJIrPATwQwUUS=623218727.730860605519053835117107860350f;if (xHFSgjRcGVSbehKDkHJIrPATwQwUUS - xHFSgjRcGVSbehKDkHJIrPATwQwUUS> 0.00000001 ) xHFSgjRcGVSbehKDkHJIrPATwQwUUS=761326149.599071504482710185333546273301f; else xHFSgjRcGVSbehKDkHJIrPATwQwUUS=176598089.045602119244183649889612706391f;if (xHFSgjRcGVSbehKDkHJIrPATwQwUUS - xHFSgjRcGVSbehKDkHJIrPATwQwUUS> 0.00000001 ) xHFSgjRcGVSbehKDkHJIrPATwQwUUS=762231135.830445721807710056464453253491f; else xHFSgjRcGVSbehKDkHJIrPATwQwUUS=968580070.682841574892918201286003859720f;if (xHFSgjRcGVSbehKDkHJIrPATwQwUUS - xHFSgjRcGVSbehKDkHJIrPATwQwUUS> 0.00000001 ) xHFSgjRcGVSbehKDkHJIrPATwQwUUS=116309136.343122697068368264616222579010f; else xHFSgjRcGVSbehKDkHJIrPATwQwUUS=1871963374.285566989671533885669522412266f;if (xHFSgjRcGVSbehKDkHJIrPATwQwUUS - xHFSgjRcGVSbehKDkHJIrPATwQwUUS> 0.00000001 ) xHFSgjRcGVSbehKDkHJIrPATwQwUUS=1193461223.468788480239605580989455276425f; else xHFSgjRcGVSbehKDkHJIrPATwQwUUS=653831748.582421485550544103894239712361f;if (xHFSgjRcGVSbehKDkHJIrPATwQwUUS - xHFSgjRcGVSbehKDkHJIrPATwQwUUS> 0.00000001 ) xHFSgjRcGVSbehKDkHJIrPATwQwUUS=1916115410.307685828420653901118473135372f; else xHFSgjRcGVSbehKDkHJIrPATwQwUUS=416738954.013435497949547479691125494455f;double XbgVInKMqbBXLnvvPgkihipDvrlxSH=1379263357.789853298157535677071804497910;if (XbgVInKMqbBXLnvvPgkihipDvrlxSH == XbgVInKMqbBXLnvvPgkihipDvrlxSH ) XbgVInKMqbBXLnvvPgkihipDvrlxSH=1369374413.870134025549489330904974098657; else XbgVInKMqbBXLnvvPgkihipDvrlxSH=1566574471.649949840165253253516802690625;if (XbgVInKMqbBXLnvvPgkihipDvrlxSH == XbgVInKMqbBXLnvvPgkihipDvrlxSH ) XbgVInKMqbBXLnvvPgkihipDvrlxSH=1397391820.758800164169094973295614194735; else XbgVInKMqbBXLnvvPgkihipDvrlxSH=1586945375.023652856329956782882754114581;if (XbgVInKMqbBXLnvvPgkihipDvrlxSH == XbgVInKMqbBXLnvvPgkihipDvrlxSH ) XbgVInKMqbBXLnvvPgkihipDvrlxSH=649521009.744924744323897777841206438064; else XbgVInKMqbBXLnvvPgkihipDvrlxSH=2729764.838578504214866096755874055969;if (XbgVInKMqbBXLnvvPgkihipDvrlxSH == XbgVInKMqbBXLnvvPgkihipDvrlxSH ) XbgVInKMqbBXLnvvPgkihipDvrlxSH=656750087.789159642927474744045231017562; else XbgVInKMqbBXLnvvPgkihipDvrlxSH=36620343.465553682856483841031103556814;if (XbgVInKMqbBXLnvvPgkihipDvrlxSH == XbgVInKMqbBXLnvvPgkihipDvrlxSH ) XbgVInKMqbBXLnvvPgkihipDvrlxSH=619298961.328166392227330985927653237300; else XbgVInKMqbBXLnvvPgkihipDvrlxSH=2124074666.887853843766091598368100787693;if (XbgVInKMqbBXLnvvPgkihipDvrlxSH == XbgVInKMqbBXLnvvPgkihipDvrlxSH ) XbgVInKMqbBXLnvvPgkihipDvrlxSH=2001118919.010255058724327277284005378828; else XbgVInKMqbBXLnvvPgkihipDvrlxSH=1465426300.636186685655065498451142901598;float MPzFvtJZDesMeNvzXzxuAUgjdtUJlb=45214211.216746392987331392323146420208f;if (MPzFvtJZDesMeNvzXzxuAUgjdtUJlb - MPzFvtJZDesMeNvzXzxuAUgjdtUJlb> 0.00000001 ) MPzFvtJZDesMeNvzXzxuAUgjdtUJlb=791024701.439611881788427065152833918143f; else MPzFvtJZDesMeNvzXzxuAUgjdtUJlb=1014509221.710566872161265583947525660771f;if (MPzFvtJZDesMeNvzXzxuAUgjdtUJlb - MPzFvtJZDesMeNvzXzxuAUgjdtUJlb> 0.00000001 ) MPzFvtJZDesMeNvzXzxuAUgjdtUJlb=42068829.475917997229400177844520620690f; else MPzFvtJZDesMeNvzXzxuAUgjdtUJlb=1833184044.621289746879672922569393795530f;if (MPzFvtJZDesMeNvzXzxuAUgjdtUJlb - MPzFvtJZDesMeNvzXzxuAUgjdtUJlb> 0.00000001 ) MPzFvtJZDesMeNvzXzxuAUgjdtUJlb=1848087104.830771923154218606981914143860f; else MPzFvtJZDesMeNvzXzxuAUgjdtUJlb=2110592095.521614901200952619629075092763f;if (MPzFvtJZDesMeNvzXzxuAUgjdtUJlb - MPzFvtJZDesMeNvzXzxuAUgjdtUJlb> 0.00000001 ) MPzFvtJZDesMeNvzXzxuAUgjdtUJlb=1392513979.827473370470578107363524156672f; else MPzFvtJZDesMeNvzXzxuAUgjdtUJlb=577134587.334849469945026960790641140342f;if (MPzFvtJZDesMeNvzXzxuAUgjdtUJlb - MPzFvtJZDesMeNvzXzxuAUgjdtUJlb> 0.00000001 ) MPzFvtJZDesMeNvzXzxuAUgjdtUJlb=634037936.771824774085942960920798379480f; else MPzFvtJZDesMeNvzXzxuAUgjdtUJlb=1728717756.913737521239682671939142219355f;if (MPzFvtJZDesMeNvzXzxuAUgjdtUJlb - MPzFvtJZDesMeNvzXzxuAUgjdtUJlb> 0.00000001 ) MPzFvtJZDesMeNvzXzxuAUgjdtUJlb=88562662.530418222654089212681966297162f; else MPzFvtJZDesMeNvzXzxuAUgjdtUJlb=1178882538.927671037546039517411788578433f;long GiylPlSAoJqaISbwMkmpxkYHIVtHBR=233034843;if (GiylPlSAoJqaISbwMkmpxkYHIVtHBR == GiylPlSAoJqaISbwMkmpxkYHIVtHBR- 1 ) GiylPlSAoJqaISbwMkmpxkYHIVtHBR=1772782243; else GiylPlSAoJqaISbwMkmpxkYHIVtHBR=151056096;if (GiylPlSAoJqaISbwMkmpxkYHIVtHBR == GiylPlSAoJqaISbwMkmpxkYHIVtHBR- 1 ) GiylPlSAoJqaISbwMkmpxkYHIVtHBR=2133096103; else GiylPlSAoJqaISbwMkmpxkYHIVtHBR=1763687509;if (GiylPlSAoJqaISbwMkmpxkYHIVtHBR == GiylPlSAoJqaISbwMkmpxkYHIVtHBR- 0 ) GiylPlSAoJqaISbwMkmpxkYHIVtHBR=629775598; else GiylPlSAoJqaISbwMkmpxkYHIVtHBR=378944742;if (GiylPlSAoJqaISbwMkmpxkYHIVtHBR == GiylPlSAoJqaISbwMkmpxkYHIVtHBR- 1 ) GiylPlSAoJqaISbwMkmpxkYHIVtHBR=1120840928; else GiylPlSAoJqaISbwMkmpxkYHIVtHBR=644641919;if (GiylPlSAoJqaISbwMkmpxkYHIVtHBR == GiylPlSAoJqaISbwMkmpxkYHIVtHBR- 1 ) GiylPlSAoJqaISbwMkmpxkYHIVtHBR=2113952287; else GiylPlSAoJqaISbwMkmpxkYHIVtHBR=724705806;if (GiylPlSAoJqaISbwMkmpxkYHIVtHBR == GiylPlSAoJqaISbwMkmpxkYHIVtHBR- 0 ) GiylPlSAoJqaISbwMkmpxkYHIVtHBR=1704782775; else GiylPlSAoJqaISbwMkmpxkYHIVtHBR=785640116;float xvvkQawzvBZxAPLBxenlsFwewuNSiD=1098394336.632848299601587985919026942538f;if (xvvkQawzvBZxAPLBxenlsFwewuNSiD - xvvkQawzvBZxAPLBxenlsFwewuNSiD> 0.00000001 ) xvvkQawzvBZxAPLBxenlsFwewuNSiD=683188209.581331883686403149143268872700f; else xvvkQawzvBZxAPLBxenlsFwewuNSiD=1870901199.787222026621343463491340533457f;if (xvvkQawzvBZxAPLBxenlsFwewuNSiD - xvvkQawzvBZxAPLBxenlsFwewuNSiD> 0.00000001 ) xvvkQawzvBZxAPLBxenlsFwewuNSiD=1773197900.019391938112352302552042044780f; else xvvkQawzvBZxAPLBxenlsFwewuNSiD=2076686452.978071205253538919923290508835f;if (xvvkQawzvBZxAPLBxenlsFwewuNSiD - xvvkQawzvBZxAPLBxenlsFwewuNSiD> 0.00000001 ) xvvkQawzvBZxAPLBxenlsFwewuNSiD=2040459235.736935549110731951424596788957f; else xvvkQawzvBZxAPLBxenlsFwewuNSiD=520006381.458062871457181456976752224025f;if (xvvkQawzvBZxAPLBxenlsFwewuNSiD - xvvkQawzvBZxAPLBxenlsFwewuNSiD> 0.00000001 ) xvvkQawzvBZxAPLBxenlsFwewuNSiD=1678659900.760387388929107851544209271694f; else xvvkQawzvBZxAPLBxenlsFwewuNSiD=1406265965.986689891518906875558459167221f;if (xvvkQawzvBZxAPLBxenlsFwewuNSiD - xvvkQawzvBZxAPLBxenlsFwewuNSiD> 0.00000001 ) xvvkQawzvBZxAPLBxenlsFwewuNSiD=174060492.789514452907013740340615764762f; else xvvkQawzvBZxAPLBxenlsFwewuNSiD=820257233.378184949353375144652798217596f;if (xvvkQawzvBZxAPLBxenlsFwewuNSiD - xvvkQawzvBZxAPLBxenlsFwewuNSiD> 0.00000001 ) xvvkQawzvBZxAPLBxenlsFwewuNSiD=952907439.170164004080110835062021168097f; else xvvkQawzvBZxAPLBxenlsFwewuNSiD=1068286900.756438431124167368849353415901f;int xEgNSSotJVwSvSstcVnABKarYzQwGQ=1637376117;if (xEgNSSotJVwSvSstcVnABKarYzQwGQ == xEgNSSotJVwSvSstcVnABKarYzQwGQ- 0 ) xEgNSSotJVwSvSstcVnABKarYzQwGQ=341761298; else xEgNSSotJVwSvSstcVnABKarYzQwGQ=1768084076;if (xEgNSSotJVwSvSstcVnABKarYzQwGQ == xEgNSSotJVwSvSstcVnABKarYzQwGQ- 1 ) xEgNSSotJVwSvSstcVnABKarYzQwGQ=1840127704; else xEgNSSotJVwSvSstcVnABKarYzQwGQ=623446125;if (xEgNSSotJVwSvSstcVnABKarYzQwGQ == xEgNSSotJVwSvSstcVnABKarYzQwGQ- 0 ) xEgNSSotJVwSvSstcVnABKarYzQwGQ=109454431; else xEgNSSotJVwSvSstcVnABKarYzQwGQ=354439677;if (xEgNSSotJVwSvSstcVnABKarYzQwGQ == xEgNSSotJVwSvSstcVnABKarYzQwGQ- 0 ) xEgNSSotJVwSvSstcVnABKarYzQwGQ=1562922260; else xEgNSSotJVwSvSstcVnABKarYzQwGQ=412079959;if (xEgNSSotJVwSvSstcVnABKarYzQwGQ == xEgNSSotJVwSvSstcVnABKarYzQwGQ- 1 ) xEgNSSotJVwSvSstcVnABKarYzQwGQ=2041151940; else xEgNSSotJVwSvSstcVnABKarYzQwGQ=1544910557;if (xEgNSSotJVwSvSstcVnABKarYzQwGQ == xEgNSSotJVwSvSstcVnABKarYzQwGQ- 0 ) xEgNSSotJVwSvSstcVnABKarYzQwGQ=1866715404; else xEgNSSotJVwSvSstcVnABKarYzQwGQ=1123381298;int AZIZHdiVmvDlnbygDUjpeFSPtbSaIs=1983155100;if (AZIZHdiVmvDlnbygDUjpeFSPtbSaIs == AZIZHdiVmvDlnbygDUjpeFSPtbSaIs- 1 ) AZIZHdiVmvDlnbygDUjpeFSPtbSaIs=631256603; else AZIZHdiVmvDlnbygDUjpeFSPtbSaIs=1137651973;if (AZIZHdiVmvDlnbygDUjpeFSPtbSaIs == AZIZHdiVmvDlnbygDUjpeFSPtbSaIs- 0 ) AZIZHdiVmvDlnbygDUjpeFSPtbSaIs=495694329; else AZIZHdiVmvDlnbygDUjpeFSPtbSaIs=901951447;if (AZIZHdiVmvDlnbygDUjpeFSPtbSaIs == AZIZHdiVmvDlnbygDUjpeFSPtbSaIs- 1 ) AZIZHdiVmvDlnbygDUjpeFSPtbSaIs=1503692238; else AZIZHdiVmvDlnbygDUjpeFSPtbSaIs=1853110563;if (AZIZHdiVmvDlnbygDUjpeFSPtbSaIs == AZIZHdiVmvDlnbygDUjpeFSPtbSaIs- 1 ) AZIZHdiVmvDlnbygDUjpeFSPtbSaIs=340059255; else AZIZHdiVmvDlnbygDUjpeFSPtbSaIs=1252775862;if (AZIZHdiVmvDlnbygDUjpeFSPtbSaIs == AZIZHdiVmvDlnbygDUjpeFSPtbSaIs- 1 ) AZIZHdiVmvDlnbygDUjpeFSPtbSaIs=619879843; else AZIZHdiVmvDlnbygDUjpeFSPtbSaIs=726830138;if (AZIZHdiVmvDlnbygDUjpeFSPtbSaIs == AZIZHdiVmvDlnbygDUjpeFSPtbSaIs- 1 ) AZIZHdiVmvDlnbygDUjpeFSPtbSaIs=1589627017; else AZIZHdiVmvDlnbygDUjpeFSPtbSaIs=1957002012;long pzXullPYhksZaOWRdzYUgxiwWJWfsX=488601091;if (pzXullPYhksZaOWRdzYUgxiwWJWfsX == pzXullPYhksZaOWRdzYUgxiwWJWfsX- 1 ) pzXullPYhksZaOWRdzYUgxiwWJWfsX=1313454462; else pzXullPYhksZaOWRdzYUgxiwWJWfsX=67406658;if (pzXullPYhksZaOWRdzYUgxiwWJWfsX == pzXullPYhksZaOWRdzYUgxiwWJWfsX- 1 ) pzXullPYhksZaOWRdzYUgxiwWJWfsX=443878818; else pzXullPYhksZaOWRdzYUgxiwWJWfsX=1494344369;if (pzXullPYhksZaOWRdzYUgxiwWJWfsX == pzXullPYhksZaOWRdzYUgxiwWJWfsX- 0 ) pzXullPYhksZaOWRdzYUgxiwWJWfsX=1105722426; else pzXullPYhksZaOWRdzYUgxiwWJWfsX=474015028;if (pzXullPYhksZaOWRdzYUgxiwWJWfsX == pzXullPYhksZaOWRdzYUgxiwWJWfsX- 0 ) pzXullPYhksZaOWRdzYUgxiwWJWfsX=988939905; else pzXullPYhksZaOWRdzYUgxiwWJWfsX=328748532;if (pzXullPYhksZaOWRdzYUgxiwWJWfsX == pzXullPYhksZaOWRdzYUgxiwWJWfsX- 0 ) pzXullPYhksZaOWRdzYUgxiwWJWfsX=307978939; else pzXullPYhksZaOWRdzYUgxiwWJWfsX=1361117941;if (pzXullPYhksZaOWRdzYUgxiwWJWfsX == pzXullPYhksZaOWRdzYUgxiwWJWfsX- 1 ) pzXullPYhksZaOWRdzYUgxiwWJWfsX=35534512; else pzXullPYhksZaOWRdzYUgxiwWJWfsX=495548678;int gHpUHwOZvVGbmZgMvGItejDimVftfR=1714670276;if (gHpUHwOZvVGbmZgMvGItejDimVftfR == gHpUHwOZvVGbmZgMvGItejDimVftfR- 1 ) gHpUHwOZvVGbmZgMvGItejDimVftfR=1358067444; else gHpUHwOZvVGbmZgMvGItejDimVftfR=1117235986;if (gHpUHwOZvVGbmZgMvGItejDimVftfR == gHpUHwOZvVGbmZgMvGItejDimVftfR- 1 ) gHpUHwOZvVGbmZgMvGItejDimVftfR=437030297; else gHpUHwOZvVGbmZgMvGItejDimVftfR=923998776;if (gHpUHwOZvVGbmZgMvGItejDimVftfR == gHpUHwOZvVGbmZgMvGItejDimVftfR- 0 ) gHpUHwOZvVGbmZgMvGItejDimVftfR=1129656867; else gHpUHwOZvVGbmZgMvGItejDimVftfR=656783544;if (gHpUHwOZvVGbmZgMvGItejDimVftfR == gHpUHwOZvVGbmZgMvGItejDimVftfR- 1 ) gHpUHwOZvVGbmZgMvGItejDimVftfR=133683879; else gHpUHwOZvVGbmZgMvGItejDimVftfR=636134382;if (gHpUHwOZvVGbmZgMvGItejDimVftfR == gHpUHwOZvVGbmZgMvGItejDimVftfR- 1 ) gHpUHwOZvVGbmZgMvGItejDimVftfR=1428024821; else gHpUHwOZvVGbmZgMvGItejDimVftfR=587351842;if (gHpUHwOZvVGbmZgMvGItejDimVftfR == gHpUHwOZvVGbmZgMvGItejDimVftfR- 0 ) gHpUHwOZvVGbmZgMvGItejDimVftfR=3566282; else gHpUHwOZvVGbmZgMvGItejDimVftfR=1530107940;long IxaMJcmPnqUzzkwRIVCdDnqIiTXxEQ=1988597912;if (IxaMJcmPnqUzzkwRIVCdDnqIiTXxEQ == IxaMJcmPnqUzzkwRIVCdDnqIiTXxEQ- 1 ) IxaMJcmPnqUzzkwRIVCdDnqIiTXxEQ=814435740; else IxaMJcmPnqUzzkwRIVCdDnqIiTXxEQ=1378114313;if (IxaMJcmPnqUzzkwRIVCdDnqIiTXxEQ == IxaMJcmPnqUzzkwRIVCdDnqIiTXxEQ- 1 ) IxaMJcmPnqUzzkwRIVCdDnqIiTXxEQ=1957410483; else IxaMJcmPnqUzzkwRIVCdDnqIiTXxEQ=338402662;if (IxaMJcmPnqUzzkwRIVCdDnqIiTXxEQ == IxaMJcmPnqUzzkwRIVCdDnqIiTXxEQ- 0 ) IxaMJcmPnqUzzkwRIVCdDnqIiTXxEQ=162995045; else IxaMJcmPnqUzzkwRIVCdDnqIiTXxEQ=115329067;if (IxaMJcmPnqUzzkwRIVCdDnqIiTXxEQ == IxaMJcmPnqUzzkwRIVCdDnqIiTXxEQ- 1 ) IxaMJcmPnqUzzkwRIVCdDnqIiTXxEQ=877432674; else IxaMJcmPnqUzzkwRIVCdDnqIiTXxEQ=157241575;if (IxaMJcmPnqUzzkwRIVCdDnqIiTXxEQ == IxaMJcmPnqUzzkwRIVCdDnqIiTXxEQ- 1 ) IxaMJcmPnqUzzkwRIVCdDnqIiTXxEQ=148712068; else IxaMJcmPnqUzzkwRIVCdDnqIiTXxEQ=166945797;if (IxaMJcmPnqUzzkwRIVCdDnqIiTXxEQ == IxaMJcmPnqUzzkwRIVCdDnqIiTXxEQ- 0 ) IxaMJcmPnqUzzkwRIVCdDnqIiTXxEQ=593972508; else IxaMJcmPnqUzzkwRIVCdDnqIiTXxEQ=142574297;double ZqVuniXNEzlYuTlXWuyjYPlJysHSQz=64601241.773391641485170285863569191690;if (ZqVuniXNEzlYuTlXWuyjYPlJysHSQz == ZqVuniXNEzlYuTlXWuyjYPlJysHSQz ) ZqVuniXNEzlYuTlXWuyjYPlJysHSQz=1873857140.919882596470556634230362922421; else ZqVuniXNEzlYuTlXWuyjYPlJysHSQz=2022440388.106168749989949411467846175797;if (ZqVuniXNEzlYuTlXWuyjYPlJysHSQz == ZqVuniXNEzlYuTlXWuyjYPlJysHSQz ) ZqVuniXNEzlYuTlXWuyjYPlJysHSQz=598934568.117647550809546143362117537000; else ZqVuniXNEzlYuTlXWuyjYPlJysHSQz=456284964.405387241297276840003244611121;if (ZqVuniXNEzlYuTlXWuyjYPlJysHSQz == ZqVuniXNEzlYuTlXWuyjYPlJysHSQz ) ZqVuniXNEzlYuTlXWuyjYPlJysHSQz=1763007378.155041426091227695279701726288; else ZqVuniXNEzlYuTlXWuyjYPlJysHSQz=1953959688.622024333652412313712796256533;if (ZqVuniXNEzlYuTlXWuyjYPlJysHSQz == ZqVuniXNEzlYuTlXWuyjYPlJysHSQz ) ZqVuniXNEzlYuTlXWuyjYPlJysHSQz=1506427871.535958931437363830952367793788; else ZqVuniXNEzlYuTlXWuyjYPlJysHSQz=656670918.980013866549338554452669137826;if (ZqVuniXNEzlYuTlXWuyjYPlJysHSQz == ZqVuniXNEzlYuTlXWuyjYPlJysHSQz ) ZqVuniXNEzlYuTlXWuyjYPlJysHSQz=2081206426.535365836108451403025044633209; else ZqVuniXNEzlYuTlXWuyjYPlJysHSQz=1955228083.604102408383836531883481250283;if (ZqVuniXNEzlYuTlXWuyjYPlJysHSQz == ZqVuniXNEzlYuTlXWuyjYPlJysHSQz ) ZqVuniXNEzlYuTlXWuyjYPlJysHSQz=66319548.364850445901685331811342623406; else ZqVuniXNEzlYuTlXWuyjYPlJysHSQz=518370584.333512909620698291693947303224;long zTxilpBlOSNMMzDjXWUNIoBRRweKBw=702058691;if (zTxilpBlOSNMMzDjXWUNIoBRRweKBw == zTxilpBlOSNMMzDjXWUNIoBRRweKBw- 0 ) zTxilpBlOSNMMzDjXWUNIoBRRweKBw=375075285; else zTxilpBlOSNMMzDjXWUNIoBRRweKBw=194420247;if (zTxilpBlOSNMMzDjXWUNIoBRRweKBw == zTxilpBlOSNMMzDjXWUNIoBRRweKBw- 0 ) zTxilpBlOSNMMzDjXWUNIoBRRweKBw=1828596585; else zTxilpBlOSNMMzDjXWUNIoBRRweKBw=1570143958;if (zTxilpBlOSNMMzDjXWUNIoBRRweKBw == zTxilpBlOSNMMzDjXWUNIoBRRweKBw- 0 ) zTxilpBlOSNMMzDjXWUNIoBRRweKBw=745351766; else zTxilpBlOSNMMzDjXWUNIoBRRweKBw=662053983;if (zTxilpBlOSNMMzDjXWUNIoBRRweKBw == zTxilpBlOSNMMzDjXWUNIoBRRweKBw- 0 ) zTxilpBlOSNMMzDjXWUNIoBRRweKBw=1551675231; else zTxilpBlOSNMMzDjXWUNIoBRRweKBw=1242738377;if (zTxilpBlOSNMMzDjXWUNIoBRRweKBw == zTxilpBlOSNMMzDjXWUNIoBRRweKBw- 0 ) zTxilpBlOSNMMzDjXWUNIoBRRweKBw=239803312; else zTxilpBlOSNMMzDjXWUNIoBRRweKBw=279788722;if (zTxilpBlOSNMMzDjXWUNIoBRRweKBw == zTxilpBlOSNMMzDjXWUNIoBRRweKBw- 1 ) zTxilpBlOSNMMzDjXWUNIoBRRweKBw=775286708; else zTxilpBlOSNMMzDjXWUNIoBRRweKBw=962711070; }
 zTxilpBlOSNMMzDjXWUNIoBRRweKBwy::zTxilpBlOSNMMzDjXWUNIoBRRweKBwy()
 { this->EtqHcqXoNkPt("qYqGcfLBVtRObxhEiLPomoEdKSfRoKEtqHcqXoNkPtj", true, 62811036, 2138593229, 966041978); }
#pragma optimize("", off)
 // <delete/>

