#pragma once
#include "../imgui/imgui.h"
#include "../valve_sdk/math/Vector.h"
#include "../valve_sdk/Misc/Color.h"
#include "../valve_sdk/csgostructs.h"
#include "globals.h"
//#include "../features/skin_changer/kit_parser.h"
//#include "../features/skin_changer/item_definitions.h"

#include <string>
#include <map>
#include <vector>
#include <array>

#define N3Wproject_VERSION 2.2




#define FLOAT_TO_CLR(clr) clr[0] * 255.0f, clr[1] * 255.0f, clr[2] * 255.0f, clr[3] * 255.0f

extern std::map<int, std::string> weapon_names;
extern std::map<int, std::string> weapon_groups;
extern std::map<int, std::string> mode_names;
extern std::map<int, std::string> entity_names;
extern std::map<int, std::string> hitbox_names;

struct item_setting
{
	char name[32] = "Default";
	bool enabled = false;
	int stickers_place = 0;
	int definition_vector_index = 0;
	int definition_index = 0;
	bool   enabled_stickers = 0;
	int paint_kit_vector_index = 0;
	int paint_kit_index = 0;
	int definition_override_vector_index = 0;
	int definition_override_index = 0;
	int seed = 0;
	bool stat_trak = 0;
	float wear = FLT_MIN;
	char custom_name[32] = "";
};

struct statrack_setting
{
	int definition_index = 1;
	struct
	{
		int counter = 0;
	}statrack_new;
};

struct legitbot_settings 
{
	bool enabled = false;

	bool enable_hitbox_override = false;
	KeyBind_t hitbox_override_bind = { 0, 0, false };
	bool override_hitboxes[7] = {false, false, false, false, false, false, false};

	bool hitboxes[7] = { false, false, false, false, false, false, false };
	int priority = 0;

	bool smoke_check = false;
	bool flash_check = false;
	int flash_check_min = 100;
	bool jump_check = false;
	bool on_key = true;
	int key = 1;
	bool only_in_zoom = true;

	bool deathmatch = false;
	bool autopistol = false;
	bool autowall = false;
	
	bool silent = false;
	bool autofire = false;
	bool autostop = false;
	bool aim_at_backtrack = true;
	int backtrack_time = 0;
	int autofire_key = 0;
	int aim_type = 1;
	int hitchance_wp;
	int fov_type = 0;
	int smooth_type = 0;
	float fov = 0.f;
	int silent_chance = 100;
	float silent_fov = 0.f;
	float smooth = 1;
	int shot_delay = 0;
	int kill_delay = 0;

	int rcs_type = 0;
	bool rcs = false;
	bool rcs_fov_enabled = false;
	bool rcs_smooth_enabled = false;
	float rcs_fov = 0.f;
	float rcs_smooth = 1;
	int rcs_x = 100;
	int rcs_y = 100;
	int rcs_start = 1;

	int min_damage = 1;
};

struct profilechanger_settings
{
	int wins = 0;
	int rank_id = 0;
};

enum esp_types : int
{
	enemies = 0,
	teammates = 1,
	local_player = 2
};

struct glow_settings
{
	bool enable = false;
	bool visible_only = false;

	Color visible = { 0, 0, 0, 255 };
	Color in_visible = { 0, 0, 0, 255 };
};

struct esp_settings
{
	bool enable = false;
	bool only_visible = false;
	
	bool skeleton = false;
	bool box = false;
	bool name = false;
	int box_position = 0;

	bool esp_backtrack = false;

	bool health_bar = false;
	bool health_in_bar = false;
	bool health_based = false;
	int health_position = 0;

	bool armor_bar = false;
	bool armor_in_bar = false;
	int armor_position = 1;

	bool weapon = false;
	bool wea = false;
	bool weaico = false;

	bool weapon_ammo = false;
	int weapon_type = 0;
	int weapon_drop_type = 0;


	bool flags_scoped = false;
	bool flags_flashed = false;
	bool flags_defusing = false;
	bool flags_planting = false;
	bool flags_bomb_carrier = false;
	bool flags_armor = false;
	bool flags_helmet = false;

	Color dormant = Color(0, 0, 0, 255);
	Color name_color = Color(255, 255, 255, 255);
	Color weapon_name = Color(255, 255, 255, 255);
	Color weapon_name_icon = Color(255, 255, 255, 255);

	Color droppedcoloricon = Color(255, 255, 255, 255);
	Color droppedcolorname = Color(255, 255, 255, 255);




	Color box_visible = Color(0, 0, 0, 255);
	Color box_invisible = Color(0, 0, 0, 255);

	Color skeleton_visible = Color(0, 0, 0, 255);
	Color skeleton_invisible = Color(0, 0, 0, 255);

	Color health_bar_outline = Color(0, 0, 0, 255);
	Color health_bar_background = Color(0, 0, 0, 255);
	Color health_bar_main = Color(0, 0, 0, 255);

	Color armor_bar_outline = Color(0, 0, 0, 255);
	Color armor_bar_background = Color(0, 0, 0, 255);
	Color armor_bar_main = Color(0, 0, 0, 255);

	Color ammo_bar_outline = Color(0, 0, 0, 255);
	Color ammo_bar_background = Color(0, 0, 0, 255);
	Color ammo_bar_main = Color(0, 0, 0, 255);
};

struct chams_settings
{
	bool enable = false;
	bool only_visible = false;

	int chams_type = 0;

	Color visible = Color(0, 0, 0, 255);
	Color invisible = Color(0, 0, 0, 255);

	Color glow_visible = Color(0, 0, 0, 255);
	Color glow_invisible = Color(0, 0, 0, 255);
};

namespace settings
{	
	namespace windows
	{
		ImVec2 Bind_pos = ImVec2(100, 200);
		float Bind_alpha = 1.f;

		ImVec2 Spec_pos = ImVec2(100, 200);
		float Spec_alpha = 1.f;
	}

	namespace legit_bot
	{
		bool auto_current = false;
		int settings_category = 0;
		int weapon_group = 0;
		int settings_weapon = 0;

		//legitbot_settings                legitbot_items_all;
		//std::array<legitbot_settings, 100> legitbot_items;
		//std::array<legitbot_settings, 7> legitbot_items_groups;

		legitbot_settings                legitbot_items_all;
		extern std::map<int, legitbot_settings> legitbot_items;
		extern std::map<int, legitbot_settings> legitbot_items_groups;

		KeyBind_t bind = { 0, 0, false };
		bool bind_check = false;
	}

	namespace triggerbot
	{
		bool enable = false;
		bool flash = false;
		int bind = false;
		bool magnetic = false;
		bool jump = false;
		float hitchance = 0;
		bool friendfire = false;
		float delay = 0;
		bool smoke = false;
		bool head = true;
		bool arms = true;
		bool autotrigger = false;
		bool chest = true;
		bool stomach = true;
		bool legs = false;
	}


	namespace visuals
	{
	//	bool modeltr = false;
		//bool modelct = false;

		
		int sleeves_mat = 0;
		int glow_style = 0;
		bool velocityindicator = false;
		Color Velocitycol = Color(0, 0, 0, 255);

		bool keypressed = false;
		bool lastvelocityjump = false;
		bool glow_enable = false;
		bool sleeves_wireframe = false;
		bool sleeves_refletive = false;
		bool hitsound = false;
		bool hitmarker = false;
		int hitsoundtrade = false;

		bool glow_visible_only = false;
		bool glow_enemy_only = true;

		bool spectator_list = false;
		int chams_1 = 0;
		bool visible_only = false;
		bool radar_ingame = false;
		bool sleeves_enable = false;
		bool chams_enable = false;
		bool chams_flat = false;
		int chams_bt = 0;

		bool no_hands = false;
		bool arms_enable = false;
		int arms_mat = 0;
		bool arms_wireframe = false;
		bool chams_weapon_enable = false;
		int weapon_mat = 0;
		bool chams_wireframe = false;
		bool chams_glass = false;
		bool bomb_timer = false;
		bool weapon_wireframe = false;
		bool weapon_chams_refletive= false;
		bool arma_chams_refletive = false;
		bool arma_wireframe = false;
		int arma_mat = 0;
		bool chams_arma_enable = false;

		bool auto_direction = false;
		bool arms_refletive = false;
		bool chams_enemy = true;
		bool optimize = false;
		bool no_sky = false;
		Color glow_color = Color(0, 0, 0, 255);

		Color chams_visible_color = Color(0, 0, 0, 255);
		Color chams_invisible_color = Color(0, 0, 0, 255);
		Color chams_visible_colorBT = Color(0, 0, 0, 166);
		Color chams_invisible_colorBT = Color(0, 0, 0, 166);
		Color chams_sleeves_color = Color(0, 0, 0, 255);
		Color arms_color = Color(0, 0, 0, 255);
		Color weapon_chams_color = Color(0, 0, 0, 255);
		Color arma_chams_color = Color(0, 0, 0, 255);
		Color ColorWeaponDroppedChams = Color(0, 0, 0, 255);

		Color colorNadeChams = Color(0, 0, 0, 255);
		Color colorPlantedC4Chams = Color(0, 0, 0, 255);

		bool wep_droppedchams = false;
		bool nade_chams = false;
		bool plantedc4_chams = false;

		
		bool no_shadows = false; 



		int outline_type = 0;

		namespace hitbox
		{
			bool enable = false;
			Color color = Color(0, 0, 0, 255);
			float show_time = 1.f;
			int show_type = 0;
		}
		namespace events
		{
			namespace screen
			{
				bool hurt = true;
				bool player_buy = false;
				bool planting = true;
				bool defusing = true;
				bool config = true;
			}
			namespace console
			{
				bool hurt = true;
				bool player_buy = false;
				bool planting = false;
				bool defusing = false;
				bool config = false;
			}
		}

		namespace bullet_impact
		{
			namespace box
			{
				bool enable;
				Color color = Color(0, 0, 0, 255);
				float time = 2.f;
				float size = 2.f;
			}

			namespace line
			{
				bool enable = false;
				bool local_player = false;
				bool teammates = false;
				bool enemies = false;

				Color color_local = Color(0, 0, 0, 255);
				Color color_teammates = Color(0, 0, 0, 255);
				Color color_enemies = Color(0, 0, 0, 255);
				float time = 2.f;
			}
		}

		namespace damage_indicator
		{
			bool enable = false;

			float show_time = 5.f;
			float speed = 0.2f;
			float max_pos_y = 20.f;
			float range_offset_x = 10.f;
			float offset_if_kill = 10.f;
			float offset_hit = 0.f;

			float text_size = 12.f;
			Color standart_color = Color(255, 255, 255, 255);
			Color kill_color = Color(255, 50, 50, 255);
		}

		namespace grenade_prediction
		{
			bool enable = false;

			float line_thickness = 2.f;
			float colision_box_size = 2.f;
			float main_colision_box_size = 3.f;

			Color main = Color(0, 0, 0, 255);
			Color main_box = Color(0, 0, 0, 255);
			Color end_box = Color(0, 0, 0, 255);
		}
		
		namespace grenades
		{
			bool enable = false;

			bool smoke_timer = false;
			bool smoke_bar = true;
			bool molotov_timer = false;
			bool molotov_bar = true;

			Color color_smoke = Color(255, 255, 255, 255);
			Color color_molotov = Color(255, 255, 255, 255);
			Color color_decoy = Color(255, 255, 255, 255);
			Color color_flash = Color(255, 255, 255, 255);
			Color color_frag = Color(255, 255, 255, 255);

			Color color_bar_smoke_back = Color(20, 20, 20, 100);
			Color color_bar_smoke_main = Color(100, 100, 255, 255);

			Color color_bar_molotov_back = Color(20, 20, 20, 100);
			Color color_bar_molotov_main = Color(255, 100, 100, 255);
		}

		namespace dropped_weapon
		{
			bool enable = false;
			bool weacoi = false;
			bool box = false;
			bool chams = false;
			int chams_type = 0;
			bool ammo_bar = false;
			int enable_type = 0;

			Color droppediconcolor = Color(255, 255, 255, 255);
			Color droppednamecolor = Color(255, 255, 255, 255);



			Color box_color = Color(0, 0, 0, 255);
			Color chams_color = Color(255, 255, 255, 255);

			Color bar_outline = Color(0, 0, 0, 255);
			Color bar_background = Color(0, 0, 0, 255);
			Color bar_main = Color(0, 0, 0, 255);
		}

		namespace ofc
		{
			bool enable = false;
			bool visible_check = false;

			float range = 26.f;
			float hight = 6.f;
			float wight = 12.f;

			Color color = Color(255, 255, 255, 255);
		}

		namespace fov
		{
			bool fov_basic = false;
			bool fov_basic_background = false;
			bool fov_silent = false;
			bool fov_silent_background = false;

			Color color_fov_basic = Color(0, 0, 0, 255);
			Color color_fov_basic_background = Color(0, 0, 0, 255);
			Color color_fov_silent = Color(0, 0, 0, 255);
			Color color_fov_silent_background = Color(0, 0, 0, 255);
		}
		
		namespace sniper_crosshair
		{
			bool enable = false;
		}

		namespace esp
		{
			std::array<esp_settings, 3> esp_items;
		}

		namespace glow
		{
			bool enable = false;
			bool c4 = false;
			std::array<glow_settings, 3> glow_items;
			bool glow_players = false;
			bool glow_enemy = true;
			bool glow_c4carrier = false;
			bool planted_c4 = false;
			bool weaponsglow = false;
			bool galinha = false;
			int pulsestyle = false;
			bool defusekit = false;
			Color c4colorcarrier = Color(0, 0, 0, 255);
			Color defusekitcolor = Color(0, 0, 0, 255);
			Color weaponsglowcolor = Color(0, 0, 0, 255);
			Color planted_c4color = Color(0, 0, 0, 255);
			Color glow_enemycolor = Color(0, 0, 0, 255);
			Color glow_allycolor = Color(0, 0, 0, 255);
			Color galinhacor = Color(0, 0, 0, 255);


		}

		namespace chams
		{
			std::array<chams_settings, 3> chams_items;
			std::array<chams_settings, 2> chams_items_ragdoll;

			namespace local_model
			{
				bool hands = false;
				bool no_hands = false;
				int hands_type = 0;
				Color hands_color = Color(0, 0, 0, 255);

				bool weapon = false;
				int weapon_type = 0;
				Color weapon_color = Color(0, 0, 0, 255);

				bool real = false;
				int real_type = 0;
				Color real_color = Color(255, 255, 255, 255);
			}

			namespace backtrack
			{
				bool enable = false;
				bool only_visible = true;
				int type = 0;
				int type_draw = 0;
				Color color_visible = Color(255, 255, 255, 255);
				Color color_invisible = Color(255, 255, 255, 255);
			}
		}
	}

	namespace misc
	{
		Color menu_color = Color(42, 185, 216, 255);

		int model_index = 0;
		bool penetrationcircle = false;
		bool ow_reveal = false;
		bool doorspam = false;

		KeyBind_t doorspambind = { 0, 0,  false };
		bool auto_accept = false;
		bool fakeduck = false;
		bool svpure_bypass = false;
		bool keystrokes = false;
		bool slowwalk = false;
		bool chat_filter_bypass = false;
		KeyBind_t slowbind = { 0, 0,  false };

		KeyBind_t fakeduckbind = { 0, 0,  false };

		float fakeduckmontage = 0;

		bool movement_recording = false;

		int movement_recorded = 0.f;
		//int movement_play = 0.f;
		//int movement_recorder = 0.f;

		KeyBind_t movement_recorder = { 0, 0,  false };

		KeyBind_t movement_play = { 0, 0,  false };

		
		
		float slowwalkammount = 0;
		bool auto_direction = false;
		bool longesp = false;
		bool fast_stop = false;
		bool lefthandknife = false;
		int radartype = 0;
		float radarzoom = 0;
		int radarsize = 0;

		bool radarativar = false;
		int fast_stop_mode = 0;

		bool resolver = false;
		bool no_animations12 = false;

		int model = false;
		bool soundenable = false;
		int skyboxchanger = false;
		bool skyboxenable = false;
		bool viewmodelenable = false;

		bool force_inventory_open = false;		
		bool anti_obs = false;
		Color soundcolor = Color{ 0.1f, 0.1f, 0.1f, 1.0f };
		bool choke_indicator = false;
		bool hitmarker = false;
		bool draw_fov = false;
		bool reveal_money = false;
		int region_changer = false;
		bool reveal_rank = false;
		bool nightmode = false;
		int nightmodepower = 1;

		bool edge_bug = false;
		int edge_bug_key;
		KeyBind_t edgebugkey = { 0, 0, false };



		bool visuals_crosshair = false;
		bool moon_walk = false;
		bool fast_duck = false;
		float soundtime = 0.5f;
		float soundradio = 15.0f;

		bool namestealer = false;
		bool watermark = true;
		//bool fake_prime = true;


		bool disable_flash = false;
		int flash_alpha = 50;
		bool disable_smoke = false;
		bool disable_zoom = false;
		bool disable_zoom_border = false;

		namespace prepare_revolver
		{
			bool enable = false;
			KeyBind_t bind = { 0, 0, false };
		}

		namespace fog
		{
			bool enable = false;
			Color color = Color(50, 50, 50, 120);
			float start_dist = 20.f;
			float end_dist = 100.f;
		}

		namespace triggerbot
		{
			bool enabled_trigger = false;
			KeyBind_t bind = { 0, 0, false };
			
		}

		namespace inverse_gravity
		{
			bool enable_slow = false;
			bool enable = false;
			int value = 4;
		}

		namespace autorunbst
		{
			bool enable = false;
			KeyBind_t bind = { 0, 0,  false };
			int key;
		}

		namespace bhop
		{
			bool enable = false;
			bool humanised = false;
			bool enabled = false;
			bool auto_strafer = false;
			int strafer_type = 1;
			float bhop_hit_chance = 0;
			int hops_restricted_limit = 0;
			int max_hops_hit = 0;
			float retrack_speed = 0.f;
		}

		namespace edgebug
		{
			bool enable = false;
			KeyBind_t bind = { 0, 0, false };

		}


		namespace performance
		{
			bool disablepanorama = false;

		
			bool no_sky = false;
			bool no_shadows = false;
			bool no_shadows2 = false;
			bool no_shadows3 = false;
			bool no_shadows4 = false;
			bool no_shadows5 = false;
			bool no_shadows6 = false;
			bool no_shadows7 = false;
			bool no_shadows8 = false;
			bool no_shadows9 = false;
			bool processing = false;
		}

		namespace edge_jump
		{
			bool enable = false;
			bool auto_duck = false;
			KeyBind_t bind = { 0, 0, false};
		}

		namespace clantag
		{
			bool enable = false;
			bool check_empty = true;

			std::string clantag = "N3wProject ";
			std::string clantag_visible = "N3wProject ";

			int clantag_type = 0;
			int animation_type = 0;
			float speed = 0.4f;
			bool custom_type;
		}

		namespace viewmodel
		{
			float override = 90;
			bool enablethis = false;
			float viewmodel = 68;
			float fov_x = 0;
			float fov_y = 0;
			float fov_z = 0;
			float aspect_ratio = 0;

		}

		namespace fake_latency
		{
			bool enable = false;
			int amount = 0;
		}

		namespace third_person
		{
			bool enable = false;
			KeyBind_t bind = { 0, 0, false };
			float dist = 50.f;
		}

		namespace jumpbug
		{
			bool enable = false;
			int key;
			KeyBind_t bind = { 0, 0, false };

		}
		/*namespace fake_prime 
		{
			bool enable = true;
		}*/
		
		namespace bind_window
		{
			bool enable;
		}

		namespace spreed_circle
		{
			bool enable = false;
			Color color = { 255, 255, 255, 50 };
		}

		/*namespace sniper_crosshair
		{
			bool enable = false;
		}*/

		namespace desync
		{			
			namespace indicator
			{
				bool arrow = true;
				//bool enabled1 = false;
				Color real = Color(255, 255, 255, 255);
				Color fake = Color(255, 0, 0, 255);
				float size_x = 20.f;
				float size_y = 5.f;
				float offset_x = 100.f;
				bool lines = false;
			}

			bool enabled1 = false;
			int type = 0;
			bool type1 = false;
			bool dcrefletive = false;
			bool chams = false;
			bool dcwireframe = false;
			Color chama_real = Color(255, 255, 255, 255);
			int chama = 0;
			bool auto_der = false;
			KeyBind_t bind = { 0, 0, false };
			KeyBind_t bind1 = { 0, 0, false };

			float side = 1.f;
		}

		
		/*namespace slowwalk
		{
			bool enable = false;
			float speed = 0.f;
			KeyBind_t bind = { 0, 0, false };
		}*/

		namespace block_bot
		{
			bool enable = false;
			KeyBind_t bind = { 0, 0, false };
		}

		namespace fake_lag
		{
			bool enable = false;

			bool lag_when_stand = false;
			bool lag_when_move = false;

			int type = 0;
			int factor = 0;
			int factor1 = 16;

		}
	}

	namespace changers
	{
		namespace profile
		{
			//std::array<profilechanger_settings, 3> profile_items;
			extern std::map<int, profilechanger_settings> profile_items;

			int mode_id;
			int private_id = 0;
			int private_xp = 0;
			int teacher = 0;
			int leader = 0;
			int friendly = 0;
			int ban_type = 0;
			int ban_time = 0;
			int ban_time_type = 0;
		}

		namespace skin
		{
			bool skin_preview = false;
			bool show_cur = true;

			extern std::map<int, statrack_setting> statrack_items;
			extern std::map<int, item_setting> m_items;
			extern std::map<std::string, std::string> m_icon_overrides;
		}
	}
}

static char* chams_types[] = {
"regular",
"flat",
};

static char* chams_bt[] = {
"regular",
"flat",
};
#include "../render/Icons.h"


static char* tabs_chams[] = { "players", "world" };
//static char* tabs[] = {"legit","visual","misc", "skin", "config"}; //static char* tabs[] = {ICON_FA_CROSSHAIRS "    legit",ICON_FA_EYE "    visual",ICON_FA_COG "    misc",ICON_FA_GIFT "    skin",ICON_FA_USERS_COG "    config"};
//static char* tabs[] = { ICON_FA_CROSSHAIRS "  legit",ICON_FA_EYE "  visual",ICON_FA_USERS_COG "  misc",ICON_FA_HAMMER "  skin",ICON_FA_COG "  config" };
static char* tabs[] = { ICON_FA_CROSSHAIRS "  legit",ICON_FA_EYE "  visual",ICON_FA_USERS_COG "  misc",ICON_FA_COG "  config" };

//static char* esp_tabs_general[] = {"esp", "glow", "chams", "other", "world"};
static char* esp_tabs_general[] = { "esp", "other", "world" };

static char* players_tabs[] = { "enemies", "teammates", "local player" };
//static char* players_tabs[] = { "enemies", "teammates", "local player" };

static char* players_tabs_ragdoll[] = { "enemies", "teammates" };
static char* chams_types_model[] = { "standart", "ragdoll" };
extern bool  g_unload;