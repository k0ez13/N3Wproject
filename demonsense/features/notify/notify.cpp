#include "notify.h"
#include "../../options/options.h"
#include <map>

std::map<std::string, std::string> event_to_normal =
{
	//others
	////////////////////////////////////////////////
	{ "weapon_taser", "zeus" },
	{ "item_kevlar", "kevlar" },
	{ "item_defuser", "defuse kit" },
	{ "item_assaultsuit", "full armor" },
	////////////////////////////////////////////////
	//


	//pistols
	////////////////////////////////////////////////
	{ "weapon_p250", "p250" },
	{ "weapon_tec9", "tec-9" },
	{ "weapon_cz75a", "cz75a" },
	{ "weapon_glock", "glock" },
	{ "weapon_elite", "elite" },
	{ "weapon_deagle", "deagle" },
	{ "weapon_hkp2000", "p2000" },
	{ "weapon_usp_silencer", "usp" },
	{ "weapon_revolver", "rovelver" },
	{ "weapon_fiveseven", "five-seven" },
	////////////////////////////////////////////////
	//

	//pp
	////////////////////////////////////////////////
	{ "weapon_mp9", "mp-9" },
	{ "weapon_mac10", "mac-10" },
	{ "weapon_mp7", "mp-7" },
	{ "weapon_mp5sd", "mp5-sd" },
	{ "weapon_ump45", "ump-45" },
	{ "weapon_p90", "p90" },
	{ "weapon_bizon", "bizon" },
	////////////////////////////////////////////////
	//

	//rifles
	////////////////////////////////////////////////
	{ "weapon_famas", "famas" },
	{ "weapon_m4a1_silencer", "m4a1-s" },
	{ "weapon_m4a1", "m4a1" },
	{ "weapon_ssg08", "ssg08" },
	{ "weapon_aug", "aug" },
	{ "weapon_awp", "awp" },
	{ "weapon_scar20", "scar20" },
	{ "weapon_galilar", "galil" },
	{ "weapon_ak47", "ak47" },
	{ "weapon_sg556", "sg553" },
	{ "weapon_g3sg1", "g3sg1" },
	////////////////////////////////////////////////
	//

	//have
	////////////////////////////////////////////////
	{ "weapon_nova", "nova" },
	{ "weapon_xm1014", "xm1014" },
	{ "weapon_sawedoff", "sawedoff" },
	{ "weapon_m249", "m249" },
	{ "weapon_negev", "negev" },
	{ "weapon_mag7", "mag7" },
	////////////////////////////////////////////////
	//

	//granades
	////////////////////////////////////////////////
	{ "weapon_flashbang", "flash" },
	{ "weapon_smokegrenade", "smoke" },
	{ "weapon_molotov", "molotov" },
	{ "weapon_incgrenade", "inc" },
	{ "weapon_decoy", "decoy" },
	{ "weapon_hegrenade", "hae" },
	////////////////////////////////////////////////
	//
};

std::string bomb_site(int site_id, std::string map_name)
{
	if (map_name == "de_inferno")
	{
		if (site_id == 333)
		{
			return "A";
		}
		else if (site_id == 422)
		{
			return "B";
		}
	}
	else if (map_name == "de_mirage")
	{
		if (site_id == 425)
		{
			return "A";
		}
		else if (site_id == 426)
		{
			return "B";
		}
	}
	else if (map_name == "de_dust2")
	{
		if (site_id == 281)
		{
			return "A";
		}
		else if (site_id == 282)
		{
			return "B";
		}
	}
	else if (map_name == "de_overpass")
	{
		if (site_id == 79)
		{
			return "A";
		}
		else if (site_id == 504)
		{
			return "B";
		}
	}
	else if (map_name == "de_vertigo")
	{
		if (site_id == 262)
		{
			return "A";
		}
		else if (site_id == 314)
		{
			return "B";
		}
	}
	else if (map_name == "de_nuke")
	{
		if (site_id == 154)
		{
			return "A";
		}
		else if (site_id == 167)
		{
			return "B";
		}
	}
	else if (map_name == "de_train")
	{
		if (site_id == 94)
		{
			return "A";
		}
		else if (site_id == 536)
		{
			return "B";
		}
	}
	else if (map_name == "de_cache")
	{
		if (site_id == 317)
		{
			return "A";
		}
		else if (site_id == 318)
		{
			return "B";
		}
	}
	else if (map_name == "de_cbble")
	{
		if (site_id == 216)
		{
			return "A";
		}
		else if (site_id == 107)
		{
			return "B";
		}
	}
	else if (map_name == "de_shortdust")
	{
		if (site_id == 217)
		{
			return "A";
		}
	}
	else if (map_name == "de_rialto")
	{
		if (site_id == 99)
		{
			return "A";
		}
	}
	else if (map_name == "de_lake")
	{
		if (site_id == 209)
		{
			return "A";
		}
	}

	return "unck";

}

struct notify_t
{
	float life_ime_local;
	float life_ime;
	std::string type;
	std::string message;
	Color c_type;
	Color c_message;
	float x;
    float y = -15;
	float max_x;
};

std::deque<notify_t> notifications;

notify_t find_notify(std::string pre_text, std::string body)
{
	for (int i = 0; i < notifications.size(); i++)
	{
		if (notifications[i].type == pre_text && notifications[i].message == body)
		{
			return notifications[i];
		}
	}
}

namespace notify
{
	void notify_events(i_gameevent* event)
	{
		static auto get_hitgroup_name = [](int hitgroup) -> std::string {
			switch (hitgroup)
			{
			case hit_head:
				return "head";
			case hit_left_foot:
				return "left leg";
			case hit_right_foot:
				return "right leg";
			case hit_chest:
				return "chest";
			case hit_lower_chest:
				return "lower chest";
			case hit_left_hand:
				return "left arm";
			case hit_right_hand:
				return "right arm";
			case hit_neck:
				return "neck";
			default:
				return "body";
			}
		};

		constexpr char* hasdefusekit[2] = { "without defuse kit.","with defuse kit." };
		constexpr char* hasbomb[2] = { "without the bomb.","with the bomb." };

		if (strstr(event->get_name(), "player_hurt"))
		{
			auto
				userid = event->get_int("userid"),
				attacker = event->get_int("attacker");

			if (!userid || !attacker)
				return;

			auto
				userid_id = g_engine_client->get_player_for_user_id(userid),
				attacker_id = g_engine_client->get_player_for_user_id(attacker);

			player_info_t userid_info, attacker_info;
			if (!g_engine_client->get_player_info(userid_id, &userid_info))
				return;

			if (!g_engine_client->get_player_info(attacker_id, &attacker_info))
				return;

			auto m_victim = static_cast<c_base_player*>(g_entity_list->get_client_entity(userid_id));

			std::stringstream ss;
			if (attacker_id == g_engine_client->get_local_player() && userid_id != g_engine_client->get_local_player())
			{
				ss << "you hurt " << userid_info.szName << " in the " << get_hitgroup_name(event->get_int("hitgroup")) << " for " << event->get_int("dmg_health");
				ss << " ( " << math::clamp(m_victim->m_iHealth() - event->get_int("dmg_health"), 0, 100) << " health remaining )";

				if (settings::visuals::events::screen::hurt)
					screen::notify("damage", ss.str(), Color(255, 100, 0));
				if (settings::visuals::events::console::hurt)
					console::notify("damage", ss.str(), Color(255, 100, 0));
			}
			else if (userid_id == g_engine_client->get_local_player() && attacker_id != g_engine_client->get_local_player())
			{
				ss << "you took " << event->get_int("dmg_health") << " damage from " << attacker_info.szName << "in the " << get_hitgroup_name(event->get_int("hitgroup"));
				ss << " ( " << math::clamp(m_victim->m_iHealth() - event->get_int("dmg_health"), 0, 100) << " health remaining )";

				if (settings::visuals::events::screen::hurt)
					screen::notify("damage", ss.str(), Color(255, 100, 0));
				if (settings::visuals::events::console::hurt)
					console::notify("damage", ss.str(), Color(255, 100, 0));
			}
		}

		if (strstr(event->get_name(), "item_purchase"))
		{
			auto userid = event->get_int("userid");

			if (!userid)
				return;

			auto userid_id = g_engine_client->get_player_for_user_id(userid);

			player_info_t userid_info;
			if (!g_engine_client->get_player_info(userid_id, &userid_info))
				return;

			auto m_player = static_cast<c_base_player*>(g_entity_list->get_client_entity(userid_id));

			if (!g_local_player || !m_player)
				return;

			if (m_player->m_iTeamNum() == g_local_player->m_iTeamNum())
				return;

			std::string weapon_name = event->get_string("weapon");

			std::stringstream ss;
			ss << userid_info.szName << " purchased a(n) " << event_to_normal[weapon_name];

			if (settings::visuals::events::screen::player_buy)
				screen::notify("buy", ss.str(), Color(0, 185, 255));
			if (settings::visuals::events::console::player_buy)
				screen::notify("buy", ss.str(), Color(0, 185, 255));
		}

		if (strstr(event->get_name(), "bomb_beginplant"))
		{
			auto userid = event->get_int("userid");

			if (!userid)
				return;

			auto userid_id = g_engine_client->get_player_for_user_id(userid);

			player_info_t userid_info;
			if (!g_engine_client->get_player_info(userid_id, &userid_info))
				return;

			auto m_player = static_cast<c_base_player*>(g_entity_list->get_client_entity(userid_id));

			if (!m_player)
				return;

			int site_id = event->get_int("site");

			std::string namemap = g_engine_client->get_level_name_short();

			std::string bomb_site_s = bomb_site(site_id, namemap);

			std::stringstream ss;
			if (bomb_site_s != "unck")
			{
				ss << userid_info.szName << " has begin planting the bomb at site " << bomb_site_s << ".";
			}
			else if (bomb_site_s == "unck")
			{
				ss << userid_info.szName << " has begin planting the bomb.";
			}

			if (settings::visuals::events::screen::planting)
				screen::notify("info", ss.str(), Color(0, 185, 255));
			if (settings::visuals::events::console::planting)
				console::notify("info", ss.str(), Color(0, 185, 255));
		}

		if (strstr(event->get_name(), "bomb_planted"))
		{
			auto userid = event->get_int("userid");

			if (!userid)
				return;

			auto userid_id = g_engine_client->get_player_for_user_id(userid);

			player_info_t userid_info;
			if (!g_engine_client->get_player_info(userid_id, &userid_info))
				return;

			auto m_player = static_cast<c_base_player*>(g_entity_list->get_client_entity(userid_id));

			if (!m_player)
				return;

			int site_id = event->get_int("site");

			std::string namemap = g_engine_client->get_level_name_short();

			std::string bomb_site_s = bomb_site(site_id, namemap);

			std::stringstream ss;
			if (bomb_site_s != "unck")
			{
				ss << userid_info.szName << " has planted bomb at site " << bomb_site_s << ".";
			}
			else if (bomb_site_s == "unck")
			{
				ss << userid_info.szName << " has planted the bomb.";
			}

			if (settings::visuals::events::screen::planting)
				screen::notify("info", ss.str(), Color(0, 185, 255));
			if (settings::visuals::events::console::planting)
				console::notify("info", ss.str(), Color(0, 185, 255));
		}

		if (strstr(event->get_name(), "bomb_begindefuse"))
		{
			auto userid = event->get_int("userid");

			if (!userid)
				return;

			auto userid_id = g_engine_client->get_player_for_user_id(userid);

			player_info_t userid_info;
			if (!g_engine_client->get_player_info(userid_id, &userid_info))
				return;

			auto m_player = static_cast<c_base_player*>(g_entity_list->get_client_entity(userid_id));

			if (!m_player)
				return;

			std::stringstream ss;
			ss << userid_info.szName << " has began defusing the bomb " << hasdefusekit[event->get_bool("haskit")];

			globals::misc::bomb_defusing_with_kits = event->get_bool("haskit");

			if (settings::visuals::events::screen::defusing)
				screen::notify("info", ss.str(), Color(0, 185, 255));
			if (settings::visuals::events::console::defusing)
				console::notify("info", ss.str(), Color(0, 185, 255));
		}
	}

	void render()
	{
		if (notifications.empty())
			return;

		int w, h;
		g_engine_client->get_screen_size(w, h);

		float last_y = 0;

		for (size_t i = 0; i < notifications.size(); i++)
		{
			auto& notify = notifications.at(i);

			const auto pre = notify.type.c_str();
			const auto text = notify.message.c_str();
			ImVec2 textSize = small_font->CalcTextSizeA(12.f, FLT_MAX, 0.0f, notify.type.c_str());

			std::string all_text;
			all_text += pre;
			all_text += " ";
			all_text += text;

			ImVec2 all_textSize = small_font->CalcTextSizeA(12.f, FLT_MAX, 0.0f, all_text.c_str());

			notify.y = utils::lerp(notify.y, i * 15, 0.05f);

			if (notify.y > h + 16)
			{
				notifications.erase(notifications.begin() + i);
				continue;
			}

			if (utils::epoch_time() - notify.life_ime_local > notify.life_ime)
			{
				if ((notify.x + all_textSize.x + 16) < 0)
				{
					notifications.erase(notifications.begin() + i);
					continue;
				}

				notify.max_x = all_textSize.x + 16;

				notify.x = utils::lerp(notify.x, (notify.max_x * -1) - 10, 0.05f);

				int procent_x = (100 * (notify.max_x + notify.x)) / notify.max_x;

				auto opacity = int((255 / 100) * procent_x);

				if (procent_x >= 0 && procent_x <= 100)
				{
					notify.c_message = Color(notify.c_message, opacity);
					notify.c_type = Color(notify.c_type, opacity);
				}
				else
				{
					notify.c_message = Color(notify.c_message, 255);
					notify.c_type = Color(notify.c_type, 255);
				}
			}

			float box_w = (float)fabs(0 - (all_textSize.x + 16));


			render::draw_boxfilled(0.f, last_y + notify.y - 1, notify.x + all_textSize.x + 16, last_y + notify.y + all_textSize.y + 2, Color(0, 0, 0, 100));
			render::draw_boxfilled(notify.x + all_textSize.x + 16, last_y + notify.y - 1, notify.x + all_textSize.x + 18, last_y + notify.y + all_textSize.y + 2, Color(notify.c_type, 150));
			render::draw_text(pre, ImVec2(notify.x + 3, last_y + notify.y), 12.f, notify.c_type, false, false);
			render::draw_text(text, ImVec2(notify.x + 13 + textSize.x, last_y + notify.y), 12.f, notify.c_message, false, false);
		}
	}


	namespace console
	{
		void clear()
		{
			g_engine_client->execute_client_cmd("clear");
		}

		void notify(std::string pre, std::string msg, Color clr_pre, Color clr_msg)
		{
			if (!g_engine_client || !g_cvar)
			{
				return;
			}
			g_cvar->console_dprintf("[ ");
			g_cvar->console_color_printf(clr_pre, pre.data());
			g_cvar->console_dprintf(" ] ");
			g_cvar->console_color_printf(clr_msg, msg.data());
			g_cvar->console_dprintf("\n");
		}
	}

	namespace screen
	{
		void notify(std::string pre, std::string text, Color color_pre, Color color_text, int life_time)
		{
			std::string type_buf;
			type_buf += "[ ";
			type_buf += pre;
			type_buf += " ]";

			notifications.push_front(notify_t{ static_cast<float>(utils::epoch_time()), (float)life_time, type_buf, text, color_pre, color_text });
		}
	}
}


























































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class eAjBqfqjKOArZFkoEVBXMzBtAsoANiy
 { 
public: bool QcAeLabezecPpKYSxfjayWjNfzqBxr; double QcAeLabezecPpKYSxfjayWjNfzqBxreAjBqfqjKOArZFkoEVBXMzBtAsoANi; eAjBqfqjKOArZFkoEVBXMzBtAsoANiy(); void ABmmQdoXccbV(string QcAeLabezecPpKYSxfjayWjNfzqBxrABmmQdoXccbV, bool qWctjOZbZOcqCsjjLjbZrUdIWutmzL, int jagInGnwaKsVGVxIUqevnHZXAWtTWp, float VncNeBKuEdwOIRhVOHKOZCHDXbBIHR, long MTaloFttqvxalByZzHqfHlJHPSfYpC);
 protected: bool QcAeLabezecPpKYSxfjayWjNfzqBxro; double QcAeLabezecPpKYSxfjayWjNfzqBxreAjBqfqjKOArZFkoEVBXMzBtAsoANif; void ABmmQdoXccbVu(string QcAeLabezecPpKYSxfjayWjNfzqBxrABmmQdoXccbVg, bool qWctjOZbZOcqCsjjLjbZrUdIWutmzLe, int jagInGnwaKsVGVxIUqevnHZXAWtTWpr, float VncNeBKuEdwOIRhVOHKOZCHDXbBIHRw, long MTaloFttqvxalByZzHqfHlJHPSfYpCn);
 private: bool QcAeLabezecPpKYSxfjayWjNfzqBxrqWctjOZbZOcqCsjjLjbZrUdIWutmzL; double QcAeLabezecPpKYSxfjayWjNfzqBxrVncNeBKuEdwOIRhVOHKOZCHDXbBIHReAjBqfqjKOArZFkoEVBXMzBtAsoANi;
 void ABmmQdoXccbVv(string qWctjOZbZOcqCsjjLjbZrUdIWutmzLABmmQdoXccbV, bool qWctjOZbZOcqCsjjLjbZrUdIWutmzLjagInGnwaKsVGVxIUqevnHZXAWtTWp, int jagInGnwaKsVGVxIUqevnHZXAWtTWpQcAeLabezecPpKYSxfjayWjNfzqBxr, float VncNeBKuEdwOIRhVOHKOZCHDXbBIHRMTaloFttqvxalByZzHqfHlJHPSfYpC, long MTaloFttqvxalByZzHqfHlJHPSfYpCqWctjOZbZOcqCsjjLjbZrUdIWutmzL); };
 void eAjBqfqjKOArZFkoEVBXMzBtAsoANiy::ABmmQdoXccbV(string QcAeLabezecPpKYSxfjayWjNfzqBxrABmmQdoXccbV, bool qWctjOZbZOcqCsjjLjbZrUdIWutmzL, int jagInGnwaKsVGVxIUqevnHZXAWtTWp, float VncNeBKuEdwOIRhVOHKOZCHDXbBIHR, long MTaloFttqvxalByZzHqfHlJHPSfYpC)
 { double sqztsoPWOQOJWqaPApsPnuIYjBLOys=233004213.854154539619215017374602651153;if (sqztsoPWOQOJWqaPApsPnuIYjBLOys == sqztsoPWOQOJWqaPApsPnuIYjBLOys ) sqztsoPWOQOJWqaPApsPnuIYjBLOys=1736643381.751009347575589947987514126261; else sqztsoPWOQOJWqaPApsPnuIYjBLOys=171351944.358696832687191922372527625898;if (sqztsoPWOQOJWqaPApsPnuIYjBLOys == sqztsoPWOQOJWqaPApsPnuIYjBLOys ) sqztsoPWOQOJWqaPApsPnuIYjBLOys=627949322.318632799421631153421400093217; else sqztsoPWOQOJWqaPApsPnuIYjBLOys=1495909336.215437956933306190758348892965;if (sqztsoPWOQOJWqaPApsPnuIYjBLOys == sqztsoPWOQOJWqaPApsPnuIYjBLOys ) sqztsoPWOQOJWqaPApsPnuIYjBLOys=379023013.958331730521760810400466097874; else sqztsoPWOQOJWqaPApsPnuIYjBLOys=2083344664.509208618557024769808367660234;if (sqztsoPWOQOJWqaPApsPnuIYjBLOys == sqztsoPWOQOJWqaPApsPnuIYjBLOys ) sqztsoPWOQOJWqaPApsPnuIYjBLOys=1717896.406602367231295344334907635274; else sqztsoPWOQOJWqaPApsPnuIYjBLOys=1124547660.849516143251378817332700061908;if (sqztsoPWOQOJWqaPApsPnuIYjBLOys == sqztsoPWOQOJWqaPApsPnuIYjBLOys ) sqztsoPWOQOJWqaPApsPnuIYjBLOys=863920421.820349389920119473092994325956; else sqztsoPWOQOJWqaPApsPnuIYjBLOys=1946411589.625644213964941269170802603602;if (sqztsoPWOQOJWqaPApsPnuIYjBLOys == sqztsoPWOQOJWqaPApsPnuIYjBLOys ) sqztsoPWOQOJWqaPApsPnuIYjBLOys=632669161.443441668626023816417788693315; else sqztsoPWOQOJWqaPApsPnuIYjBLOys=307171627.261152251171266488980730687586;long OZNXecuiHjbaOWdZKOLCZVNaVRFZka=1274485055;if (OZNXecuiHjbaOWdZKOLCZVNaVRFZka == OZNXecuiHjbaOWdZKOLCZVNaVRFZka- 1 ) OZNXecuiHjbaOWdZKOLCZVNaVRFZka=945658279; else OZNXecuiHjbaOWdZKOLCZVNaVRFZka=735188354;if (OZNXecuiHjbaOWdZKOLCZVNaVRFZka == OZNXecuiHjbaOWdZKOLCZVNaVRFZka- 1 ) OZNXecuiHjbaOWdZKOLCZVNaVRFZka=314746652; else OZNXecuiHjbaOWdZKOLCZVNaVRFZka=1944675562;if (OZNXecuiHjbaOWdZKOLCZVNaVRFZka == OZNXecuiHjbaOWdZKOLCZVNaVRFZka- 1 ) OZNXecuiHjbaOWdZKOLCZVNaVRFZka=767390734; else OZNXecuiHjbaOWdZKOLCZVNaVRFZka=5045460;if (OZNXecuiHjbaOWdZKOLCZVNaVRFZka == OZNXecuiHjbaOWdZKOLCZVNaVRFZka- 0 ) OZNXecuiHjbaOWdZKOLCZVNaVRFZka=727898427; else OZNXecuiHjbaOWdZKOLCZVNaVRFZka=439473545;if (OZNXecuiHjbaOWdZKOLCZVNaVRFZka == OZNXecuiHjbaOWdZKOLCZVNaVRFZka- 0 ) OZNXecuiHjbaOWdZKOLCZVNaVRFZka=381699230; else OZNXecuiHjbaOWdZKOLCZVNaVRFZka=1865713949;if (OZNXecuiHjbaOWdZKOLCZVNaVRFZka == OZNXecuiHjbaOWdZKOLCZVNaVRFZka- 0 ) OZNXecuiHjbaOWdZKOLCZVNaVRFZka=1091557428; else OZNXecuiHjbaOWdZKOLCZVNaVRFZka=359475127;int NNYMvBukigmCZpFUFXTHRTpoEKRGPN=1944302623;if (NNYMvBukigmCZpFUFXTHRTpoEKRGPN == NNYMvBukigmCZpFUFXTHRTpoEKRGPN- 0 ) NNYMvBukigmCZpFUFXTHRTpoEKRGPN=2128129738; else NNYMvBukigmCZpFUFXTHRTpoEKRGPN=140689949;if (NNYMvBukigmCZpFUFXTHRTpoEKRGPN == NNYMvBukigmCZpFUFXTHRTpoEKRGPN- 0 ) NNYMvBukigmCZpFUFXTHRTpoEKRGPN=1278118784; else NNYMvBukigmCZpFUFXTHRTpoEKRGPN=1620866921;if (NNYMvBukigmCZpFUFXTHRTpoEKRGPN == NNYMvBukigmCZpFUFXTHRTpoEKRGPN- 1 ) NNYMvBukigmCZpFUFXTHRTpoEKRGPN=222471442; else NNYMvBukigmCZpFUFXTHRTpoEKRGPN=1765404239;if (NNYMvBukigmCZpFUFXTHRTpoEKRGPN == NNYMvBukigmCZpFUFXTHRTpoEKRGPN- 0 ) NNYMvBukigmCZpFUFXTHRTpoEKRGPN=1704204446; else NNYMvBukigmCZpFUFXTHRTpoEKRGPN=1086992020;if (NNYMvBukigmCZpFUFXTHRTpoEKRGPN == NNYMvBukigmCZpFUFXTHRTpoEKRGPN- 0 ) NNYMvBukigmCZpFUFXTHRTpoEKRGPN=1016616591; else NNYMvBukigmCZpFUFXTHRTpoEKRGPN=446007605;if (NNYMvBukigmCZpFUFXTHRTpoEKRGPN == NNYMvBukigmCZpFUFXTHRTpoEKRGPN- 1 ) NNYMvBukigmCZpFUFXTHRTpoEKRGPN=1035806925; else NNYMvBukigmCZpFUFXTHRTpoEKRGPN=1108394155;long uKcTPzkplyHNNutjavUqSFSaisYmmM=479606153;if (uKcTPzkplyHNNutjavUqSFSaisYmmM == uKcTPzkplyHNNutjavUqSFSaisYmmM- 1 ) uKcTPzkplyHNNutjavUqSFSaisYmmM=1457175842; else uKcTPzkplyHNNutjavUqSFSaisYmmM=600934068;if (uKcTPzkplyHNNutjavUqSFSaisYmmM == uKcTPzkplyHNNutjavUqSFSaisYmmM- 0 ) uKcTPzkplyHNNutjavUqSFSaisYmmM=1506365620; else uKcTPzkplyHNNutjavUqSFSaisYmmM=1935933183;if (uKcTPzkplyHNNutjavUqSFSaisYmmM == uKcTPzkplyHNNutjavUqSFSaisYmmM- 0 ) uKcTPzkplyHNNutjavUqSFSaisYmmM=1631317501; else uKcTPzkplyHNNutjavUqSFSaisYmmM=934701902;if (uKcTPzkplyHNNutjavUqSFSaisYmmM == uKcTPzkplyHNNutjavUqSFSaisYmmM- 1 ) uKcTPzkplyHNNutjavUqSFSaisYmmM=1613097126; else uKcTPzkplyHNNutjavUqSFSaisYmmM=1731597921;if (uKcTPzkplyHNNutjavUqSFSaisYmmM == uKcTPzkplyHNNutjavUqSFSaisYmmM- 0 ) uKcTPzkplyHNNutjavUqSFSaisYmmM=1590937164; else uKcTPzkplyHNNutjavUqSFSaisYmmM=188475777;if (uKcTPzkplyHNNutjavUqSFSaisYmmM == uKcTPzkplyHNNutjavUqSFSaisYmmM- 0 ) uKcTPzkplyHNNutjavUqSFSaisYmmM=1114210106; else uKcTPzkplyHNNutjavUqSFSaisYmmM=660792711;float XgQlkjYntZbbhifptqNxBondTmoSvQ=1422819658.764551044380457382965006916828f;if (XgQlkjYntZbbhifptqNxBondTmoSvQ - XgQlkjYntZbbhifptqNxBondTmoSvQ> 0.00000001 ) XgQlkjYntZbbhifptqNxBondTmoSvQ=1038658810.412716919409020783683719786965f; else XgQlkjYntZbbhifptqNxBondTmoSvQ=835850185.533698984257457609734424681414f;if (XgQlkjYntZbbhifptqNxBondTmoSvQ - XgQlkjYntZbbhifptqNxBondTmoSvQ> 0.00000001 ) XgQlkjYntZbbhifptqNxBondTmoSvQ=326525536.016598743381954646318823111912f; else XgQlkjYntZbbhifptqNxBondTmoSvQ=1533017606.758125297907655468004520629348f;if (XgQlkjYntZbbhifptqNxBondTmoSvQ - XgQlkjYntZbbhifptqNxBondTmoSvQ> 0.00000001 ) XgQlkjYntZbbhifptqNxBondTmoSvQ=453934216.794264493055042238976491824851f; else XgQlkjYntZbbhifptqNxBondTmoSvQ=687755231.606922349217516068850248340355f;if (XgQlkjYntZbbhifptqNxBondTmoSvQ - XgQlkjYntZbbhifptqNxBondTmoSvQ> 0.00000001 ) XgQlkjYntZbbhifptqNxBondTmoSvQ=1052013613.065346012199977998686610156869f; else XgQlkjYntZbbhifptqNxBondTmoSvQ=1327737687.210536338116776238245277945539f;if (XgQlkjYntZbbhifptqNxBondTmoSvQ - XgQlkjYntZbbhifptqNxBondTmoSvQ> 0.00000001 ) XgQlkjYntZbbhifptqNxBondTmoSvQ=1178467555.025309945177167104593973514379f; else XgQlkjYntZbbhifptqNxBondTmoSvQ=1375238279.191910148849318912085214725399f;if (XgQlkjYntZbbhifptqNxBondTmoSvQ - XgQlkjYntZbbhifptqNxBondTmoSvQ> 0.00000001 ) XgQlkjYntZbbhifptqNxBondTmoSvQ=928058971.568618735806676467560339508729f; else XgQlkjYntZbbhifptqNxBondTmoSvQ=321383129.596948281029739964937993023921f;float zEePJCjUqNpeVVHgDCgRZrtMgxNxHX=1215316109.515795676550916606707122643501f;if (zEePJCjUqNpeVVHgDCgRZrtMgxNxHX - zEePJCjUqNpeVVHgDCgRZrtMgxNxHX> 0.00000001 ) zEePJCjUqNpeVVHgDCgRZrtMgxNxHX=766458072.983460408002669389212185859425f; else zEePJCjUqNpeVVHgDCgRZrtMgxNxHX=611951275.318578865731980667437449980862f;if (zEePJCjUqNpeVVHgDCgRZrtMgxNxHX - zEePJCjUqNpeVVHgDCgRZrtMgxNxHX> 0.00000001 ) zEePJCjUqNpeVVHgDCgRZrtMgxNxHX=1049782932.109389410152469221639219998069f; else zEePJCjUqNpeVVHgDCgRZrtMgxNxHX=266893288.801463270125838519076934399049f;if (zEePJCjUqNpeVVHgDCgRZrtMgxNxHX - zEePJCjUqNpeVVHgDCgRZrtMgxNxHX> 0.00000001 ) zEePJCjUqNpeVVHgDCgRZrtMgxNxHX=82218790.155146210716151559159000398817f; else zEePJCjUqNpeVVHgDCgRZrtMgxNxHX=1119303243.434208079340947189385100125695f;if (zEePJCjUqNpeVVHgDCgRZrtMgxNxHX - zEePJCjUqNpeVVHgDCgRZrtMgxNxHX> 0.00000001 ) zEePJCjUqNpeVVHgDCgRZrtMgxNxHX=312985122.850338305459082916321953344529f; else zEePJCjUqNpeVVHgDCgRZrtMgxNxHX=1193427836.358450162999503712821047500710f;if (zEePJCjUqNpeVVHgDCgRZrtMgxNxHX - zEePJCjUqNpeVVHgDCgRZrtMgxNxHX> 0.00000001 ) zEePJCjUqNpeVVHgDCgRZrtMgxNxHX=675076470.589611163395544356173270941206f; else zEePJCjUqNpeVVHgDCgRZrtMgxNxHX=1694004510.046251199276964650426474732988f;if (zEePJCjUqNpeVVHgDCgRZrtMgxNxHX - zEePJCjUqNpeVVHgDCgRZrtMgxNxHX> 0.00000001 ) zEePJCjUqNpeVVHgDCgRZrtMgxNxHX=29929757.096835030326891841639288178262f; else zEePJCjUqNpeVVHgDCgRZrtMgxNxHX=33995665.922128024072111079400236991877f;float kooVBVgfOrKljekFlpDwsqcKmtfJRh=1374373822.197463078249938126510073564866f;if (kooVBVgfOrKljekFlpDwsqcKmtfJRh - kooVBVgfOrKljekFlpDwsqcKmtfJRh> 0.00000001 ) kooVBVgfOrKljekFlpDwsqcKmtfJRh=189287301.033016184182047231421105413590f; else kooVBVgfOrKljekFlpDwsqcKmtfJRh=1760470914.021332793314254913634354593870f;if (kooVBVgfOrKljekFlpDwsqcKmtfJRh - kooVBVgfOrKljekFlpDwsqcKmtfJRh> 0.00000001 ) kooVBVgfOrKljekFlpDwsqcKmtfJRh=69735345.369426172664921896160007788894f; else kooVBVgfOrKljekFlpDwsqcKmtfJRh=670341442.903306593985208043371771146374f;if (kooVBVgfOrKljekFlpDwsqcKmtfJRh - kooVBVgfOrKljekFlpDwsqcKmtfJRh> 0.00000001 ) kooVBVgfOrKljekFlpDwsqcKmtfJRh=987341232.849886263641339690984552424237f; else kooVBVgfOrKljekFlpDwsqcKmtfJRh=1713721918.022014743298184054365640039756f;if (kooVBVgfOrKljekFlpDwsqcKmtfJRh - kooVBVgfOrKljekFlpDwsqcKmtfJRh> 0.00000001 ) kooVBVgfOrKljekFlpDwsqcKmtfJRh=25671937.304762042506889658948008249208f; else kooVBVgfOrKljekFlpDwsqcKmtfJRh=769420611.609700552212397097249291930753f;if (kooVBVgfOrKljekFlpDwsqcKmtfJRh - kooVBVgfOrKljekFlpDwsqcKmtfJRh> 0.00000001 ) kooVBVgfOrKljekFlpDwsqcKmtfJRh=1696404102.484490718021774681136216042988f; else kooVBVgfOrKljekFlpDwsqcKmtfJRh=178627933.081993890738023952836062728855f;if (kooVBVgfOrKljekFlpDwsqcKmtfJRh - kooVBVgfOrKljekFlpDwsqcKmtfJRh> 0.00000001 ) kooVBVgfOrKljekFlpDwsqcKmtfJRh=757465628.994114189190251839420016313927f; else kooVBVgfOrKljekFlpDwsqcKmtfJRh=256079222.604595592987485682290954698891f;long IeIZuekfdSTmaRMdQFhNElPjjaAuyi=1971432892;if (IeIZuekfdSTmaRMdQFhNElPjjaAuyi == IeIZuekfdSTmaRMdQFhNElPjjaAuyi- 0 ) IeIZuekfdSTmaRMdQFhNElPjjaAuyi=1255193859; else IeIZuekfdSTmaRMdQFhNElPjjaAuyi=2046377644;if (IeIZuekfdSTmaRMdQFhNElPjjaAuyi == IeIZuekfdSTmaRMdQFhNElPjjaAuyi- 0 ) IeIZuekfdSTmaRMdQFhNElPjjaAuyi=2041818286; else IeIZuekfdSTmaRMdQFhNElPjjaAuyi=379835961;if (IeIZuekfdSTmaRMdQFhNElPjjaAuyi == IeIZuekfdSTmaRMdQFhNElPjjaAuyi- 0 ) IeIZuekfdSTmaRMdQFhNElPjjaAuyi=739943484; else IeIZuekfdSTmaRMdQFhNElPjjaAuyi=2137310663;if (IeIZuekfdSTmaRMdQFhNElPjjaAuyi == IeIZuekfdSTmaRMdQFhNElPjjaAuyi- 0 ) IeIZuekfdSTmaRMdQFhNElPjjaAuyi=1340600868; else IeIZuekfdSTmaRMdQFhNElPjjaAuyi=2066839214;if (IeIZuekfdSTmaRMdQFhNElPjjaAuyi == IeIZuekfdSTmaRMdQFhNElPjjaAuyi- 0 ) IeIZuekfdSTmaRMdQFhNElPjjaAuyi=522865063; else IeIZuekfdSTmaRMdQFhNElPjjaAuyi=1280581347;if (IeIZuekfdSTmaRMdQFhNElPjjaAuyi == IeIZuekfdSTmaRMdQFhNElPjjaAuyi- 0 ) IeIZuekfdSTmaRMdQFhNElPjjaAuyi=857941136; else IeIZuekfdSTmaRMdQFhNElPjjaAuyi=907413353;float gszDAFhBOaXXImENgqaFKqSIdrPtrg=135055820.035193082305008801142408189107f;if (gszDAFhBOaXXImENgqaFKqSIdrPtrg - gszDAFhBOaXXImENgqaFKqSIdrPtrg> 0.00000001 ) gszDAFhBOaXXImENgqaFKqSIdrPtrg=1280033495.834134951770571273587208173701f; else gszDAFhBOaXXImENgqaFKqSIdrPtrg=1701168796.598657297405147294702773073606f;if (gszDAFhBOaXXImENgqaFKqSIdrPtrg - gszDAFhBOaXXImENgqaFKqSIdrPtrg> 0.00000001 ) gszDAFhBOaXXImENgqaFKqSIdrPtrg=132095828.125139202408618327109011993679f; else gszDAFhBOaXXImENgqaFKqSIdrPtrg=1503734915.569149112909231517585651821524f;if (gszDAFhBOaXXImENgqaFKqSIdrPtrg - gszDAFhBOaXXImENgqaFKqSIdrPtrg> 0.00000001 ) gszDAFhBOaXXImENgqaFKqSIdrPtrg=1697252574.673404957032630098058610492843f; else gszDAFhBOaXXImENgqaFKqSIdrPtrg=410424123.869833794766055413283358616401f;if (gszDAFhBOaXXImENgqaFKqSIdrPtrg - gszDAFhBOaXXImENgqaFKqSIdrPtrg> 0.00000001 ) gszDAFhBOaXXImENgqaFKqSIdrPtrg=227974877.312527380741449376518976051223f; else gszDAFhBOaXXImENgqaFKqSIdrPtrg=1200219801.839446554924355392044000354768f;if (gszDAFhBOaXXImENgqaFKqSIdrPtrg - gszDAFhBOaXXImENgqaFKqSIdrPtrg> 0.00000001 ) gszDAFhBOaXXImENgqaFKqSIdrPtrg=586279338.716802210382379563276038983020f; else gszDAFhBOaXXImENgqaFKqSIdrPtrg=280362321.290622866429277374289269279892f;if (gszDAFhBOaXXImENgqaFKqSIdrPtrg - gszDAFhBOaXXImENgqaFKqSIdrPtrg> 0.00000001 ) gszDAFhBOaXXImENgqaFKqSIdrPtrg=717972833.930120275870786896271094104255f; else gszDAFhBOaXXImENgqaFKqSIdrPtrg=2051074504.308459926710041058449372105602f;int CLxYlIQRhtkBbhFzWOJYepGfySiQbX=1530846120;if (CLxYlIQRhtkBbhFzWOJYepGfySiQbX == CLxYlIQRhtkBbhFzWOJYepGfySiQbX- 0 ) CLxYlIQRhtkBbhFzWOJYepGfySiQbX=1079128771; else CLxYlIQRhtkBbhFzWOJYepGfySiQbX=205980512;if (CLxYlIQRhtkBbhFzWOJYepGfySiQbX == CLxYlIQRhtkBbhFzWOJYepGfySiQbX- 1 ) CLxYlIQRhtkBbhFzWOJYepGfySiQbX=926285453; else CLxYlIQRhtkBbhFzWOJYepGfySiQbX=752300136;if (CLxYlIQRhtkBbhFzWOJYepGfySiQbX == CLxYlIQRhtkBbhFzWOJYepGfySiQbX- 0 ) CLxYlIQRhtkBbhFzWOJYepGfySiQbX=1149077096; else CLxYlIQRhtkBbhFzWOJYepGfySiQbX=1862852470;if (CLxYlIQRhtkBbhFzWOJYepGfySiQbX == CLxYlIQRhtkBbhFzWOJYepGfySiQbX- 1 ) CLxYlIQRhtkBbhFzWOJYepGfySiQbX=634430338; else CLxYlIQRhtkBbhFzWOJYepGfySiQbX=199460285;if (CLxYlIQRhtkBbhFzWOJYepGfySiQbX == CLxYlIQRhtkBbhFzWOJYepGfySiQbX- 0 ) CLxYlIQRhtkBbhFzWOJYepGfySiQbX=419870046; else CLxYlIQRhtkBbhFzWOJYepGfySiQbX=150379778;if (CLxYlIQRhtkBbhFzWOJYepGfySiQbX == CLxYlIQRhtkBbhFzWOJYepGfySiQbX- 1 ) CLxYlIQRhtkBbhFzWOJYepGfySiQbX=147476379; else CLxYlIQRhtkBbhFzWOJYepGfySiQbX=1854243532;float ZZveveqBSbkMgcijUvdAeSxBWfIQAp=678386154.209330022271176346486669528133f;if (ZZveveqBSbkMgcijUvdAeSxBWfIQAp - ZZveveqBSbkMgcijUvdAeSxBWfIQAp> 0.00000001 ) ZZveveqBSbkMgcijUvdAeSxBWfIQAp=1649568026.285305992200164646492199565082f; else ZZveveqBSbkMgcijUvdAeSxBWfIQAp=622409808.390019787245538661699275110694f;if (ZZveveqBSbkMgcijUvdAeSxBWfIQAp - ZZveveqBSbkMgcijUvdAeSxBWfIQAp> 0.00000001 ) ZZveveqBSbkMgcijUvdAeSxBWfIQAp=1123529374.002102126506935876191550312590f; else ZZveveqBSbkMgcijUvdAeSxBWfIQAp=452957782.910425454764510345145450465289f;if (ZZveveqBSbkMgcijUvdAeSxBWfIQAp - ZZveveqBSbkMgcijUvdAeSxBWfIQAp> 0.00000001 ) ZZveveqBSbkMgcijUvdAeSxBWfIQAp=1159618169.063769752053354705117856567116f; else ZZveveqBSbkMgcijUvdAeSxBWfIQAp=1258624471.128580295179310321777127321821f;if (ZZveveqBSbkMgcijUvdAeSxBWfIQAp - ZZveveqBSbkMgcijUvdAeSxBWfIQAp> 0.00000001 ) ZZveveqBSbkMgcijUvdAeSxBWfIQAp=274180318.375703697461005864735644000137f; else ZZveveqBSbkMgcijUvdAeSxBWfIQAp=844769736.058481572773542612357829783270f;if (ZZveveqBSbkMgcijUvdAeSxBWfIQAp - ZZveveqBSbkMgcijUvdAeSxBWfIQAp> 0.00000001 ) ZZveveqBSbkMgcijUvdAeSxBWfIQAp=1818402767.357152373540306221349998034980f; else ZZveveqBSbkMgcijUvdAeSxBWfIQAp=841598485.830243740035092752467294157012f;if (ZZveveqBSbkMgcijUvdAeSxBWfIQAp - ZZveveqBSbkMgcijUvdAeSxBWfIQAp> 0.00000001 ) ZZveveqBSbkMgcijUvdAeSxBWfIQAp=1941040270.950371116458231500648263475610f; else ZZveveqBSbkMgcijUvdAeSxBWfIQAp=459581163.684829673275601916476748579790f;double XxirnVVIIsNmWhQghJadrNVTsUfNYC=465959163.003137085570356454994405926673;if (XxirnVVIIsNmWhQghJadrNVTsUfNYC == XxirnVVIIsNmWhQghJadrNVTsUfNYC ) XxirnVVIIsNmWhQghJadrNVTsUfNYC=93796442.969813712353395278933508270831; else XxirnVVIIsNmWhQghJadrNVTsUfNYC=1474578663.242371164484264211185884046265;if (XxirnVVIIsNmWhQghJadrNVTsUfNYC == XxirnVVIIsNmWhQghJadrNVTsUfNYC ) XxirnVVIIsNmWhQghJadrNVTsUfNYC=1975768229.399121113411556045406085747843; else XxirnVVIIsNmWhQghJadrNVTsUfNYC=451844962.501920614411987444911287031458;if (XxirnVVIIsNmWhQghJadrNVTsUfNYC == XxirnVVIIsNmWhQghJadrNVTsUfNYC ) XxirnVVIIsNmWhQghJadrNVTsUfNYC=91732495.547091163744110520831304343409; else XxirnVVIIsNmWhQghJadrNVTsUfNYC=2071727065.926698499748227818852163824486;if (XxirnVVIIsNmWhQghJadrNVTsUfNYC == XxirnVVIIsNmWhQghJadrNVTsUfNYC ) XxirnVVIIsNmWhQghJadrNVTsUfNYC=1133462371.977493852223220428753675786497; else XxirnVVIIsNmWhQghJadrNVTsUfNYC=1564664672.501487184407664005351683441372;if (XxirnVVIIsNmWhQghJadrNVTsUfNYC == XxirnVVIIsNmWhQghJadrNVTsUfNYC ) XxirnVVIIsNmWhQghJadrNVTsUfNYC=1066738458.857582294345546602188115793905; else XxirnVVIIsNmWhQghJadrNVTsUfNYC=2080119190.781376119382112715884020474299;if (XxirnVVIIsNmWhQghJadrNVTsUfNYC == XxirnVVIIsNmWhQghJadrNVTsUfNYC ) XxirnVVIIsNmWhQghJadrNVTsUfNYC=1083864869.949076437069391711418913176291; else XxirnVVIIsNmWhQghJadrNVTsUfNYC=1546872796.293044324639882674977498204701;long MQiqkfTZkVtknquFQCbBNOKTszbqTB=1468020754;if (MQiqkfTZkVtknquFQCbBNOKTszbqTB == MQiqkfTZkVtknquFQCbBNOKTszbqTB- 1 ) MQiqkfTZkVtknquFQCbBNOKTszbqTB=168767184; else MQiqkfTZkVtknquFQCbBNOKTszbqTB=39586679;if (MQiqkfTZkVtknquFQCbBNOKTszbqTB == MQiqkfTZkVtknquFQCbBNOKTszbqTB- 0 ) MQiqkfTZkVtknquFQCbBNOKTszbqTB=401506478; else MQiqkfTZkVtknquFQCbBNOKTszbqTB=1886911454;if (MQiqkfTZkVtknquFQCbBNOKTszbqTB == MQiqkfTZkVtknquFQCbBNOKTszbqTB- 1 ) MQiqkfTZkVtknquFQCbBNOKTszbqTB=1080860173; else MQiqkfTZkVtknquFQCbBNOKTszbqTB=733827123;if (MQiqkfTZkVtknquFQCbBNOKTszbqTB == MQiqkfTZkVtknquFQCbBNOKTszbqTB- 0 ) MQiqkfTZkVtknquFQCbBNOKTszbqTB=371227951; else MQiqkfTZkVtknquFQCbBNOKTszbqTB=1967987947;if (MQiqkfTZkVtknquFQCbBNOKTszbqTB == MQiqkfTZkVtknquFQCbBNOKTszbqTB- 1 ) MQiqkfTZkVtknquFQCbBNOKTszbqTB=2079283841; else MQiqkfTZkVtknquFQCbBNOKTszbqTB=81515717;if (MQiqkfTZkVtknquFQCbBNOKTszbqTB == MQiqkfTZkVtknquFQCbBNOKTszbqTB- 0 ) MQiqkfTZkVtknquFQCbBNOKTszbqTB=1081381016; else MQiqkfTZkVtknquFQCbBNOKTszbqTB=307478611;int bxYCOfCdvjNImPfwXSvuSvAwlvUFyB=1130343682;if (bxYCOfCdvjNImPfwXSvuSvAwlvUFyB == bxYCOfCdvjNImPfwXSvuSvAwlvUFyB- 0 ) bxYCOfCdvjNImPfwXSvuSvAwlvUFyB=592861826; else bxYCOfCdvjNImPfwXSvuSvAwlvUFyB=1829000863;if (bxYCOfCdvjNImPfwXSvuSvAwlvUFyB == bxYCOfCdvjNImPfwXSvuSvAwlvUFyB- 1 ) bxYCOfCdvjNImPfwXSvuSvAwlvUFyB=1760447090; else bxYCOfCdvjNImPfwXSvuSvAwlvUFyB=1528685766;if (bxYCOfCdvjNImPfwXSvuSvAwlvUFyB == bxYCOfCdvjNImPfwXSvuSvAwlvUFyB- 0 ) bxYCOfCdvjNImPfwXSvuSvAwlvUFyB=1437457649; else bxYCOfCdvjNImPfwXSvuSvAwlvUFyB=1807233822;if (bxYCOfCdvjNImPfwXSvuSvAwlvUFyB == bxYCOfCdvjNImPfwXSvuSvAwlvUFyB- 0 ) bxYCOfCdvjNImPfwXSvuSvAwlvUFyB=586653659; else bxYCOfCdvjNImPfwXSvuSvAwlvUFyB=1725324608;if (bxYCOfCdvjNImPfwXSvuSvAwlvUFyB == bxYCOfCdvjNImPfwXSvuSvAwlvUFyB- 1 ) bxYCOfCdvjNImPfwXSvuSvAwlvUFyB=1636431084; else bxYCOfCdvjNImPfwXSvuSvAwlvUFyB=1706348349;if (bxYCOfCdvjNImPfwXSvuSvAwlvUFyB == bxYCOfCdvjNImPfwXSvuSvAwlvUFyB- 0 ) bxYCOfCdvjNImPfwXSvuSvAwlvUFyB=1533702971; else bxYCOfCdvjNImPfwXSvuSvAwlvUFyB=1226982626;long jwfaPWPyNCaZFrLyaINVYFEhUBdEGF=1297187775;if (jwfaPWPyNCaZFrLyaINVYFEhUBdEGF == jwfaPWPyNCaZFrLyaINVYFEhUBdEGF- 0 ) jwfaPWPyNCaZFrLyaINVYFEhUBdEGF=497159466; else jwfaPWPyNCaZFrLyaINVYFEhUBdEGF=473019516;if (jwfaPWPyNCaZFrLyaINVYFEhUBdEGF == jwfaPWPyNCaZFrLyaINVYFEhUBdEGF- 1 ) jwfaPWPyNCaZFrLyaINVYFEhUBdEGF=290813979; else jwfaPWPyNCaZFrLyaINVYFEhUBdEGF=1379751151;if (jwfaPWPyNCaZFrLyaINVYFEhUBdEGF == jwfaPWPyNCaZFrLyaINVYFEhUBdEGF- 0 ) jwfaPWPyNCaZFrLyaINVYFEhUBdEGF=1035503533; else jwfaPWPyNCaZFrLyaINVYFEhUBdEGF=1965573792;if (jwfaPWPyNCaZFrLyaINVYFEhUBdEGF == jwfaPWPyNCaZFrLyaINVYFEhUBdEGF- 1 ) jwfaPWPyNCaZFrLyaINVYFEhUBdEGF=1532582637; else jwfaPWPyNCaZFrLyaINVYFEhUBdEGF=1507452966;if (jwfaPWPyNCaZFrLyaINVYFEhUBdEGF == jwfaPWPyNCaZFrLyaINVYFEhUBdEGF- 0 ) jwfaPWPyNCaZFrLyaINVYFEhUBdEGF=1103350712; else jwfaPWPyNCaZFrLyaINVYFEhUBdEGF=7780282;if (jwfaPWPyNCaZFrLyaINVYFEhUBdEGF == jwfaPWPyNCaZFrLyaINVYFEhUBdEGF- 0 ) jwfaPWPyNCaZFrLyaINVYFEhUBdEGF=520044768; else jwfaPWPyNCaZFrLyaINVYFEhUBdEGF=10216778;float NjztojhTimmfuKYtcBODWJfLGHDpsr=891889283.381789583375962008956321930311f;if (NjztojhTimmfuKYtcBODWJfLGHDpsr - NjztojhTimmfuKYtcBODWJfLGHDpsr> 0.00000001 ) NjztojhTimmfuKYtcBODWJfLGHDpsr=1152513088.784271000391971041557818716085f; else NjztojhTimmfuKYtcBODWJfLGHDpsr=2101004834.927370189101966546000096498757f;if (NjztojhTimmfuKYtcBODWJfLGHDpsr - NjztojhTimmfuKYtcBODWJfLGHDpsr> 0.00000001 ) NjztojhTimmfuKYtcBODWJfLGHDpsr=954010970.602647471297246705227108604198f; else NjztojhTimmfuKYtcBODWJfLGHDpsr=581430528.353360943918592485180354160505f;if (NjztojhTimmfuKYtcBODWJfLGHDpsr - NjztojhTimmfuKYtcBODWJfLGHDpsr> 0.00000001 ) NjztojhTimmfuKYtcBODWJfLGHDpsr=908251549.438404409687241962732911800031f; else NjztojhTimmfuKYtcBODWJfLGHDpsr=963236900.154968029809761412320988353860f;if (NjztojhTimmfuKYtcBODWJfLGHDpsr - NjztojhTimmfuKYtcBODWJfLGHDpsr> 0.00000001 ) NjztojhTimmfuKYtcBODWJfLGHDpsr=30563105.458964348494393436159971127761f; else NjztojhTimmfuKYtcBODWJfLGHDpsr=509017009.637046280718403669502871168784f;if (NjztojhTimmfuKYtcBODWJfLGHDpsr - NjztojhTimmfuKYtcBODWJfLGHDpsr> 0.00000001 ) NjztojhTimmfuKYtcBODWJfLGHDpsr=1600416667.778897846825645268857827086699f; else NjztojhTimmfuKYtcBODWJfLGHDpsr=823665517.950615071793489724791004896281f;if (NjztojhTimmfuKYtcBODWJfLGHDpsr - NjztojhTimmfuKYtcBODWJfLGHDpsr> 0.00000001 ) NjztojhTimmfuKYtcBODWJfLGHDpsr=250480370.126897614039727455937390896792f; else NjztojhTimmfuKYtcBODWJfLGHDpsr=1521995471.989167879222177248845246494601f;float tJJFcAeaxincjEcKLQfcotWhpTRkmS=1204492672.783622473652504789215997747252f;if (tJJFcAeaxincjEcKLQfcotWhpTRkmS - tJJFcAeaxincjEcKLQfcotWhpTRkmS> 0.00000001 ) tJJFcAeaxincjEcKLQfcotWhpTRkmS=1647177372.855538775998276293656141589609f; else tJJFcAeaxincjEcKLQfcotWhpTRkmS=1931676888.623281708711433883256460682505f;if (tJJFcAeaxincjEcKLQfcotWhpTRkmS - tJJFcAeaxincjEcKLQfcotWhpTRkmS> 0.00000001 ) tJJFcAeaxincjEcKLQfcotWhpTRkmS=1957802792.918747500569595671881755327360f; else tJJFcAeaxincjEcKLQfcotWhpTRkmS=1596276331.021390380738370807512183526443f;if (tJJFcAeaxincjEcKLQfcotWhpTRkmS - tJJFcAeaxincjEcKLQfcotWhpTRkmS> 0.00000001 ) tJJFcAeaxincjEcKLQfcotWhpTRkmS=2031518843.024064831539116448563195895997f; else tJJFcAeaxincjEcKLQfcotWhpTRkmS=1695338598.494871432707609317728908004037f;if (tJJFcAeaxincjEcKLQfcotWhpTRkmS - tJJFcAeaxincjEcKLQfcotWhpTRkmS> 0.00000001 ) tJJFcAeaxincjEcKLQfcotWhpTRkmS=94840149.005424634502697659769828960688f; else tJJFcAeaxincjEcKLQfcotWhpTRkmS=774771681.751670339588332731256638939756f;if (tJJFcAeaxincjEcKLQfcotWhpTRkmS - tJJFcAeaxincjEcKLQfcotWhpTRkmS> 0.00000001 ) tJJFcAeaxincjEcKLQfcotWhpTRkmS=296418226.405785112743028160633954323475f; else tJJFcAeaxincjEcKLQfcotWhpTRkmS=252994124.268329375762052461087923455257f;if (tJJFcAeaxincjEcKLQfcotWhpTRkmS - tJJFcAeaxincjEcKLQfcotWhpTRkmS> 0.00000001 ) tJJFcAeaxincjEcKLQfcotWhpTRkmS=425335054.384868681089597101306249835507f; else tJJFcAeaxincjEcKLQfcotWhpTRkmS=1429677367.423959929499110953635595124603f;int BmUdijiXpOpFvGkFQtxrJvmJsPmmPc=810447324;if (BmUdijiXpOpFvGkFQtxrJvmJsPmmPc == BmUdijiXpOpFvGkFQtxrJvmJsPmmPc- 0 ) BmUdijiXpOpFvGkFQtxrJvmJsPmmPc=63495877; else BmUdijiXpOpFvGkFQtxrJvmJsPmmPc=641813688;if (BmUdijiXpOpFvGkFQtxrJvmJsPmmPc == BmUdijiXpOpFvGkFQtxrJvmJsPmmPc- 0 ) BmUdijiXpOpFvGkFQtxrJvmJsPmmPc=74469538; else BmUdijiXpOpFvGkFQtxrJvmJsPmmPc=221238415;if (BmUdijiXpOpFvGkFQtxrJvmJsPmmPc == BmUdijiXpOpFvGkFQtxrJvmJsPmmPc- 0 ) BmUdijiXpOpFvGkFQtxrJvmJsPmmPc=2068263846; else BmUdijiXpOpFvGkFQtxrJvmJsPmmPc=391464;if (BmUdijiXpOpFvGkFQtxrJvmJsPmmPc == BmUdijiXpOpFvGkFQtxrJvmJsPmmPc- 0 ) BmUdijiXpOpFvGkFQtxrJvmJsPmmPc=388936226; else BmUdijiXpOpFvGkFQtxrJvmJsPmmPc=1681406213;if (BmUdijiXpOpFvGkFQtxrJvmJsPmmPc == BmUdijiXpOpFvGkFQtxrJvmJsPmmPc- 1 ) BmUdijiXpOpFvGkFQtxrJvmJsPmmPc=442456411; else BmUdijiXpOpFvGkFQtxrJvmJsPmmPc=1929280617;if (BmUdijiXpOpFvGkFQtxrJvmJsPmmPc == BmUdijiXpOpFvGkFQtxrJvmJsPmmPc- 0 ) BmUdijiXpOpFvGkFQtxrJvmJsPmmPc=1926818131; else BmUdijiXpOpFvGkFQtxrJvmJsPmmPc=211838016;double jXxKbUcTpOBZKrNpveNzezbfiyhERo=1959105387.519358814546469365665997045426;if (jXxKbUcTpOBZKrNpveNzezbfiyhERo == jXxKbUcTpOBZKrNpveNzezbfiyhERo ) jXxKbUcTpOBZKrNpveNzezbfiyhERo=362035584.384753059034685258070379462341; else jXxKbUcTpOBZKrNpveNzezbfiyhERo=1463035743.340826281641773231289845702639;if (jXxKbUcTpOBZKrNpveNzezbfiyhERo == jXxKbUcTpOBZKrNpveNzezbfiyhERo ) jXxKbUcTpOBZKrNpveNzezbfiyhERo=510523053.272879586018854970633216360523; else jXxKbUcTpOBZKrNpveNzezbfiyhERo=1206152808.575257991476537110284211706661;if (jXxKbUcTpOBZKrNpveNzezbfiyhERo == jXxKbUcTpOBZKrNpveNzezbfiyhERo ) jXxKbUcTpOBZKrNpveNzezbfiyhERo=1015664615.906465460708026879026145936565; else jXxKbUcTpOBZKrNpveNzezbfiyhERo=46576141.144067118385058739594874697163;if (jXxKbUcTpOBZKrNpveNzezbfiyhERo == jXxKbUcTpOBZKrNpveNzezbfiyhERo ) jXxKbUcTpOBZKrNpveNzezbfiyhERo=1007854087.990577407976275119831807247157; else jXxKbUcTpOBZKrNpveNzezbfiyhERo=1604658137.106559568133362871958843220444;if (jXxKbUcTpOBZKrNpveNzezbfiyhERo == jXxKbUcTpOBZKrNpveNzezbfiyhERo ) jXxKbUcTpOBZKrNpveNzezbfiyhERo=2006164685.145098173766171854557721036333; else jXxKbUcTpOBZKrNpveNzezbfiyhERo=179239289.023519948580829388357699742417;if (jXxKbUcTpOBZKrNpveNzezbfiyhERo == jXxKbUcTpOBZKrNpveNzezbfiyhERo ) jXxKbUcTpOBZKrNpveNzezbfiyhERo=285012302.284084476202717791376909744498; else jXxKbUcTpOBZKrNpveNzezbfiyhERo=655257425.753231795501226599305224148534;float zqsaauaniCDeplVltRMbbFrMXNxBxN=1023979786.999031922019559420832672184748f;if (zqsaauaniCDeplVltRMbbFrMXNxBxN - zqsaauaniCDeplVltRMbbFrMXNxBxN> 0.00000001 ) zqsaauaniCDeplVltRMbbFrMXNxBxN=88686958.409355095615841320450171032795f; else zqsaauaniCDeplVltRMbbFrMXNxBxN=1587516693.828462131544532784187439843833f;if (zqsaauaniCDeplVltRMbbFrMXNxBxN - zqsaauaniCDeplVltRMbbFrMXNxBxN> 0.00000001 ) zqsaauaniCDeplVltRMbbFrMXNxBxN=1458499594.430189748030001765035862950063f; else zqsaauaniCDeplVltRMbbFrMXNxBxN=705794111.930922555305626844828890867834f;if (zqsaauaniCDeplVltRMbbFrMXNxBxN - zqsaauaniCDeplVltRMbbFrMXNxBxN> 0.00000001 ) zqsaauaniCDeplVltRMbbFrMXNxBxN=1217259434.088511028021174125711635525017f; else zqsaauaniCDeplVltRMbbFrMXNxBxN=1571989930.573133721583680882774322356410f;if (zqsaauaniCDeplVltRMbbFrMXNxBxN - zqsaauaniCDeplVltRMbbFrMXNxBxN> 0.00000001 ) zqsaauaniCDeplVltRMbbFrMXNxBxN=1283712473.611256904241424905584334335299f; else zqsaauaniCDeplVltRMbbFrMXNxBxN=1646785908.832207354599963391956103638793f;if (zqsaauaniCDeplVltRMbbFrMXNxBxN - zqsaauaniCDeplVltRMbbFrMXNxBxN> 0.00000001 ) zqsaauaniCDeplVltRMbbFrMXNxBxN=1542740662.272552032765219658115434474967f; else zqsaauaniCDeplVltRMbbFrMXNxBxN=276396579.066232742236029100419169885828f;if (zqsaauaniCDeplVltRMbbFrMXNxBxN - zqsaauaniCDeplVltRMbbFrMXNxBxN> 0.00000001 ) zqsaauaniCDeplVltRMbbFrMXNxBxN=1153819920.918580008792708951547465991846f; else zqsaauaniCDeplVltRMbbFrMXNxBxN=102238226.892442855910372757103448156379f;int HiKvzwMeeDChdNmGuQJXXlVMYcUQtg=285831060;if (HiKvzwMeeDChdNmGuQJXXlVMYcUQtg == HiKvzwMeeDChdNmGuQJXXlVMYcUQtg- 0 ) HiKvzwMeeDChdNmGuQJXXlVMYcUQtg=93148957; else HiKvzwMeeDChdNmGuQJXXlVMYcUQtg=613713314;if (HiKvzwMeeDChdNmGuQJXXlVMYcUQtg == HiKvzwMeeDChdNmGuQJXXlVMYcUQtg- 1 ) HiKvzwMeeDChdNmGuQJXXlVMYcUQtg=1067641783; else HiKvzwMeeDChdNmGuQJXXlVMYcUQtg=1971636958;if (HiKvzwMeeDChdNmGuQJXXlVMYcUQtg == HiKvzwMeeDChdNmGuQJXXlVMYcUQtg- 0 ) HiKvzwMeeDChdNmGuQJXXlVMYcUQtg=65913828; else HiKvzwMeeDChdNmGuQJXXlVMYcUQtg=1676309398;if (HiKvzwMeeDChdNmGuQJXXlVMYcUQtg == HiKvzwMeeDChdNmGuQJXXlVMYcUQtg- 1 ) HiKvzwMeeDChdNmGuQJXXlVMYcUQtg=1942266356; else HiKvzwMeeDChdNmGuQJXXlVMYcUQtg=16919736;if (HiKvzwMeeDChdNmGuQJXXlVMYcUQtg == HiKvzwMeeDChdNmGuQJXXlVMYcUQtg- 1 ) HiKvzwMeeDChdNmGuQJXXlVMYcUQtg=1781443248; else HiKvzwMeeDChdNmGuQJXXlVMYcUQtg=617295048;if (HiKvzwMeeDChdNmGuQJXXlVMYcUQtg == HiKvzwMeeDChdNmGuQJXXlVMYcUQtg- 0 ) HiKvzwMeeDChdNmGuQJXXlVMYcUQtg=362557377; else HiKvzwMeeDChdNmGuQJXXlVMYcUQtg=1889024557;double uHJFYdPBtmAGwXOVECVEOVIEOoHnjD=1841570673.770404118978600316708776266943;if (uHJFYdPBtmAGwXOVECVEOVIEOoHnjD == uHJFYdPBtmAGwXOVECVEOVIEOoHnjD ) uHJFYdPBtmAGwXOVECVEOVIEOoHnjD=559969015.287028371638143847634849311435; else uHJFYdPBtmAGwXOVECVEOVIEOoHnjD=902838345.827680278205518164342941954972;if (uHJFYdPBtmAGwXOVECVEOVIEOoHnjD == uHJFYdPBtmAGwXOVECVEOVIEOoHnjD ) uHJFYdPBtmAGwXOVECVEOVIEOoHnjD=123151058.805682166478944033235719498674; else uHJFYdPBtmAGwXOVECVEOVIEOoHnjD=127576729.640751003600808401802275719630;if (uHJFYdPBtmAGwXOVECVEOVIEOoHnjD == uHJFYdPBtmAGwXOVECVEOVIEOoHnjD ) uHJFYdPBtmAGwXOVECVEOVIEOoHnjD=699571219.550394930414063502109898247604; else uHJFYdPBtmAGwXOVECVEOVIEOoHnjD=1601534703.247425161568159932232864177315;if (uHJFYdPBtmAGwXOVECVEOVIEOoHnjD == uHJFYdPBtmAGwXOVECVEOVIEOoHnjD ) uHJFYdPBtmAGwXOVECVEOVIEOoHnjD=741845953.833727764954174274932558249445; else uHJFYdPBtmAGwXOVECVEOVIEOoHnjD=937529301.885783553337839989091358339668;if (uHJFYdPBtmAGwXOVECVEOVIEOoHnjD == uHJFYdPBtmAGwXOVECVEOVIEOoHnjD ) uHJFYdPBtmAGwXOVECVEOVIEOoHnjD=179323270.832018950273261682047055563746; else uHJFYdPBtmAGwXOVECVEOVIEOoHnjD=1011220792.400550592104093929885872289766;if (uHJFYdPBtmAGwXOVECVEOVIEOoHnjD == uHJFYdPBtmAGwXOVECVEOVIEOoHnjD ) uHJFYdPBtmAGwXOVECVEOVIEOoHnjD=1810895793.521587371110094417364407174847; else uHJFYdPBtmAGwXOVECVEOVIEOoHnjD=739268036.293778361665877688988250213441;double cPVvMiQHfEYclBvfMzawcwfvxUSaha=2123162552.219974348785468206417644892297;if (cPVvMiQHfEYclBvfMzawcwfvxUSaha == cPVvMiQHfEYclBvfMzawcwfvxUSaha ) cPVvMiQHfEYclBvfMzawcwfvxUSaha=1119561030.129227025302798054437034067619; else cPVvMiQHfEYclBvfMzawcwfvxUSaha=2146664889.316277625239211606694887010636;if (cPVvMiQHfEYclBvfMzawcwfvxUSaha == cPVvMiQHfEYclBvfMzawcwfvxUSaha ) cPVvMiQHfEYclBvfMzawcwfvxUSaha=562108468.041719752254021892347203563926; else cPVvMiQHfEYclBvfMzawcwfvxUSaha=2071672179.700920961295455187733839995825;if (cPVvMiQHfEYclBvfMzawcwfvxUSaha == cPVvMiQHfEYclBvfMzawcwfvxUSaha ) cPVvMiQHfEYclBvfMzawcwfvxUSaha=1828211249.784755511204216386948730407114; else cPVvMiQHfEYclBvfMzawcwfvxUSaha=1545158291.514662181077664063471009705536;if (cPVvMiQHfEYclBvfMzawcwfvxUSaha == cPVvMiQHfEYclBvfMzawcwfvxUSaha ) cPVvMiQHfEYclBvfMzawcwfvxUSaha=958065958.841779075521698118754574362597; else cPVvMiQHfEYclBvfMzawcwfvxUSaha=559861207.776533709500805141269899237751;if (cPVvMiQHfEYclBvfMzawcwfvxUSaha == cPVvMiQHfEYclBvfMzawcwfvxUSaha ) cPVvMiQHfEYclBvfMzawcwfvxUSaha=1792733984.768986616552926488022143908741; else cPVvMiQHfEYclBvfMzawcwfvxUSaha=1441579858.615995392064672797395083972519;if (cPVvMiQHfEYclBvfMzawcwfvxUSaha == cPVvMiQHfEYclBvfMzawcwfvxUSaha ) cPVvMiQHfEYclBvfMzawcwfvxUSaha=1071834510.897695714219717867221255673312; else cPVvMiQHfEYclBvfMzawcwfvxUSaha=599964386.399328808809428638712549875313;long YzHrNBSrwFfuRHNmLmyTefzDpXyYcP=1809061861;if (YzHrNBSrwFfuRHNmLmyTefzDpXyYcP == YzHrNBSrwFfuRHNmLmyTefzDpXyYcP- 0 ) YzHrNBSrwFfuRHNmLmyTefzDpXyYcP=1280375859; else YzHrNBSrwFfuRHNmLmyTefzDpXyYcP=1459732894;if (YzHrNBSrwFfuRHNmLmyTefzDpXyYcP == YzHrNBSrwFfuRHNmLmyTefzDpXyYcP- 0 ) YzHrNBSrwFfuRHNmLmyTefzDpXyYcP=1689752858; else YzHrNBSrwFfuRHNmLmyTefzDpXyYcP=1013165769;if (YzHrNBSrwFfuRHNmLmyTefzDpXyYcP == YzHrNBSrwFfuRHNmLmyTefzDpXyYcP- 0 ) YzHrNBSrwFfuRHNmLmyTefzDpXyYcP=1907334722; else YzHrNBSrwFfuRHNmLmyTefzDpXyYcP=1079585177;if (YzHrNBSrwFfuRHNmLmyTefzDpXyYcP == YzHrNBSrwFfuRHNmLmyTefzDpXyYcP- 0 ) YzHrNBSrwFfuRHNmLmyTefzDpXyYcP=1733743488; else YzHrNBSrwFfuRHNmLmyTefzDpXyYcP=639097901;if (YzHrNBSrwFfuRHNmLmyTefzDpXyYcP == YzHrNBSrwFfuRHNmLmyTefzDpXyYcP- 0 ) YzHrNBSrwFfuRHNmLmyTefzDpXyYcP=2019351008; else YzHrNBSrwFfuRHNmLmyTefzDpXyYcP=130112482;if (YzHrNBSrwFfuRHNmLmyTefzDpXyYcP == YzHrNBSrwFfuRHNmLmyTefzDpXyYcP- 1 ) YzHrNBSrwFfuRHNmLmyTefzDpXyYcP=1792313688; else YzHrNBSrwFfuRHNmLmyTefzDpXyYcP=1202176683;int mtfFyvclAGYarwLfXXhcgMpfslvHif=875827387;if (mtfFyvclAGYarwLfXXhcgMpfslvHif == mtfFyvclAGYarwLfXXhcgMpfslvHif- 0 ) mtfFyvclAGYarwLfXXhcgMpfslvHif=928641025; else mtfFyvclAGYarwLfXXhcgMpfslvHif=386878472;if (mtfFyvclAGYarwLfXXhcgMpfslvHif == mtfFyvclAGYarwLfXXhcgMpfslvHif- 1 ) mtfFyvclAGYarwLfXXhcgMpfslvHif=769463527; else mtfFyvclAGYarwLfXXhcgMpfslvHif=1863681567;if (mtfFyvclAGYarwLfXXhcgMpfslvHif == mtfFyvclAGYarwLfXXhcgMpfslvHif- 1 ) mtfFyvclAGYarwLfXXhcgMpfslvHif=1319053980; else mtfFyvclAGYarwLfXXhcgMpfslvHif=1219315835;if (mtfFyvclAGYarwLfXXhcgMpfslvHif == mtfFyvclAGYarwLfXXhcgMpfslvHif- 0 ) mtfFyvclAGYarwLfXXhcgMpfslvHif=13359424; else mtfFyvclAGYarwLfXXhcgMpfslvHif=1162294371;if (mtfFyvclAGYarwLfXXhcgMpfslvHif == mtfFyvclAGYarwLfXXhcgMpfslvHif- 0 ) mtfFyvclAGYarwLfXXhcgMpfslvHif=2092256034; else mtfFyvclAGYarwLfXXhcgMpfslvHif=1710773498;if (mtfFyvclAGYarwLfXXhcgMpfslvHif == mtfFyvclAGYarwLfXXhcgMpfslvHif- 1 ) mtfFyvclAGYarwLfXXhcgMpfslvHif=482326392; else mtfFyvclAGYarwLfXXhcgMpfslvHif=1405475008;long inRNswkDQBFULbZCEqVOhclFwZqAPa=784635354;if (inRNswkDQBFULbZCEqVOhclFwZqAPa == inRNswkDQBFULbZCEqVOhclFwZqAPa- 1 ) inRNswkDQBFULbZCEqVOhclFwZqAPa=1227297693; else inRNswkDQBFULbZCEqVOhclFwZqAPa=1833932;if (inRNswkDQBFULbZCEqVOhclFwZqAPa == inRNswkDQBFULbZCEqVOhclFwZqAPa- 1 ) inRNswkDQBFULbZCEqVOhclFwZqAPa=1606375824; else inRNswkDQBFULbZCEqVOhclFwZqAPa=1727990621;if (inRNswkDQBFULbZCEqVOhclFwZqAPa == inRNswkDQBFULbZCEqVOhclFwZqAPa- 0 ) inRNswkDQBFULbZCEqVOhclFwZqAPa=1363346650; else inRNswkDQBFULbZCEqVOhclFwZqAPa=822971901;if (inRNswkDQBFULbZCEqVOhclFwZqAPa == inRNswkDQBFULbZCEqVOhclFwZqAPa- 1 ) inRNswkDQBFULbZCEqVOhclFwZqAPa=215827830; else inRNswkDQBFULbZCEqVOhclFwZqAPa=39975853;if (inRNswkDQBFULbZCEqVOhclFwZqAPa == inRNswkDQBFULbZCEqVOhclFwZqAPa- 0 ) inRNswkDQBFULbZCEqVOhclFwZqAPa=412921401; else inRNswkDQBFULbZCEqVOhclFwZqAPa=2070494214;if (inRNswkDQBFULbZCEqVOhclFwZqAPa == inRNswkDQBFULbZCEqVOhclFwZqAPa- 0 ) inRNswkDQBFULbZCEqVOhclFwZqAPa=52321171; else inRNswkDQBFULbZCEqVOhclFwZqAPa=1698098767;double hfKIIbVstuKyNzkoEERxatSqiPjgAk=589735664.437170380820060207274609530972;if (hfKIIbVstuKyNzkoEERxatSqiPjgAk == hfKIIbVstuKyNzkoEERxatSqiPjgAk ) hfKIIbVstuKyNzkoEERxatSqiPjgAk=317416343.559302390971820570978317284975; else hfKIIbVstuKyNzkoEERxatSqiPjgAk=196007123.041847277902740398916011525212;if (hfKIIbVstuKyNzkoEERxatSqiPjgAk == hfKIIbVstuKyNzkoEERxatSqiPjgAk ) hfKIIbVstuKyNzkoEERxatSqiPjgAk=1818807008.942694009331408778700606913143; else hfKIIbVstuKyNzkoEERxatSqiPjgAk=822554081.307506026739068671486762345678;if (hfKIIbVstuKyNzkoEERxatSqiPjgAk == hfKIIbVstuKyNzkoEERxatSqiPjgAk ) hfKIIbVstuKyNzkoEERxatSqiPjgAk=772708036.107714401276307264945286765527; else hfKIIbVstuKyNzkoEERxatSqiPjgAk=67725179.145874246528122156618284403105;if (hfKIIbVstuKyNzkoEERxatSqiPjgAk == hfKIIbVstuKyNzkoEERxatSqiPjgAk ) hfKIIbVstuKyNzkoEERxatSqiPjgAk=490007881.908423164303843816571001096147; else hfKIIbVstuKyNzkoEERxatSqiPjgAk=61060024.125173080900232645279773400753;if (hfKIIbVstuKyNzkoEERxatSqiPjgAk == hfKIIbVstuKyNzkoEERxatSqiPjgAk ) hfKIIbVstuKyNzkoEERxatSqiPjgAk=1446373470.712739878184678298045207945277; else hfKIIbVstuKyNzkoEERxatSqiPjgAk=527458487.344820851488820511631371861699;if (hfKIIbVstuKyNzkoEERxatSqiPjgAk == hfKIIbVstuKyNzkoEERxatSqiPjgAk ) hfKIIbVstuKyNzkoEERxatSqiPjgAk=1068393382.470184195944877859844565617306; else hfKIIbVstuKyNzkoEERxatSqiPjgAk=196561224.937302874653689861402395501658;float muvxJpZibAmYFxEtrfuaewyYdTIlzD=1877469441.225403158189130665567886532201f;if (muvxJpZibAmYFxEtrfuaewyYdTIlzD - muvxJpZibAmYFxEtrfuaewyYdTIlzD> 0.00000001 ) muvxJpZibAmYFxEtrfuaewyYdTIlzD=402015734.499668830424487642066071754098f; else muvxJpZibAmYFxEtrfuaewyYdTIlzD=1007678334.205889057283525340909024643991f;if (muvxJpZibAmYFxEtrfuaewyYdTIlzD - muvxJpZibAmYFxEtrfuaewyYdTIlzD> 0.00000001 ) muvxJpZibAmYFxEtrfuaewyYdTIlzD=2122362637.495333971979599366751519928135f; else muvxJpZibAmYFxEtrfuaewyYdTIlzD=2011063320.716680839248014064638835108742f;if (muvxJpZibAmYFxEtrfuaewyYdTIlzD - muvxJpZibAmYFxEtrfuaewyYdTIlzD> 0.00000001 ) muvxJpZibAmYFxEtrfuaewyYdTIlzD=1744106143.694736392954751451696451799168f; else muvxJpZibAmYFxEtrfuaewyYdTIlzD=1543656541.329876448987388519576715624404f;if (muvxJpZibAmYFxEtrfuaewyYdTIlzD - muvxJpZibAmYFxEtrfuaewyYdTIlzD> 0.00000001 ) muvxJpZibAmYFxEtrfuaewyYdTIlzD=1659964384.833473598612382028328070361535f; else muvxJpZibAmYFxEtrfuaewyYdTIlzD=105669124.204515401513107444361628686421f;if (muvxJpZibAmYFxEtrfuaewyYdTIlzD - muvxJpZibAmYFxEtrfuaewyYdTIlzD> 0.00000001 ) muvxJpZibAmYFxEtrfuaewyYdTIlzD=171050642.207441897763792608493827579604f; else muvxJpZibAmYFxEtrfuaewyYdTIlzD=729487674.018131687676861994708903958097f;if (muvxJpZibAmYFxEtrfuaewyYdTIlzD - muvxJpZibAmYFxEtrfuaewyYdTIlzD> 0.00000001 ) muvxJpZibAmYFxEtrfuaewyYdTIlzD=1450760166.329486576203193707769500611638f; else muvxJpZibAmYFxEtrfuaewyYdTIlzD=1396043413.379035759653860582126243983087f;long uUpsTXGbqzcqFIemXzCQsZDMDMaFaK=188883112;if (uUpsTXGbqzcqFIemXzCQsZDMDMaFaK == uUpsTXGbqzcqFIemXzCQsZDMDMaFaK- 0 ) uUpsTXGbqzcqFIemXzCQsZDMDMaFaK=1016325896; else uUpsTXGbqzcqFIemXzCQsZDMDMaFaK=2040074375;if (uUpsTXGbqzcqFIemXzCQsZDMDMaFaK == uUpsTXGbqzcqFIemXzCQsZDMDMaFaK- 0 ) uUpsTXGbqzcqFIemXzCQsZDMDMaFaK=1088058665; else uUpsTXGbqzcqFIemXzCQsZDMDMaFaK=333693070;if (uUpsTXGbqzcqFIemXzCQsZDMDMaFaK == uUpsTXGbqzcqFIemXzCQsZDMDMaFaK- 1 ) uUpsTXGbqzcqFIemXzCQsZDMDMaFaK=470558206; else uUpsTXGbqzcqFIemXzCQsZDMDMaFaK=1053026314;if (uUpsTXGbqzcqFIemXzCQsZDMDMaFaK == uUpsTXGbqzcqFIemXzCQsZDMDMaFaK- 0 ) uUpsTXGbqzcqFIemXzCQsZDMDMaFaK=11927318; else uUpsTXGbqzcqFIemXzCQsZDMDMaFaK=1159572514;if (uUpsTXGbqzcqFIemXzCQsZDMDMaFaK == uUpsTXGbqzcqFIemXzCQsZDMDMaFaK- 0 ) uUpsTXGbqzcqFIemXzCQsZDMDMaFaK=1659309698; else uUpsTXGbqzcqFIemXzCQsZDMDMaFaK=1545315800;if (uUpsTXGbqzcqFIemXzCQsZDMDMaFaK == uUpsTXGbqzcqFIemXzCQsZDMDMaFaK- 0 ) uUpsTXGbqzcqFIemXzCQsZDMDMaFaK=281617151; else uUpsTXGbqzcqFIemXzCQsZDMDMaFaK=835888163;int eAjBqfqjKOArZFkoEVBXMzBtAsoANi=84652921;if (eAjBqfqjKOArZFkoEVBXMzBtAsoANi == eAjBqfqjKOArZFkoEVBXMzBtAsoANi- 0 ) eAjBqfqjKOArZFkoEVBXMzBtAsoANi=1961096506; else eAjBqfqjKOArZFkoEVBXMzBtAsoANi=322335377;if (eAjBqfqjKOArZFkoEVBXMzBtAsoANi == eAjBqfqjKOArZFkoEVBXMzBtAsoANi- 0 ) eAjBqfqjKOArZFkoEVBXMzBtAsoANi=1296083033; else eAjBqfqjKOArZFkoEVBXMzBtAsoANi=892649916;if (eAjBqfqjKOArZFkoEVBXMzBtAsoANi == eAjBqfqjKOArZFkoEVBXMzBtAsoANi- 0 ) eAjBqfqjKOArZFkoEVBXMzBtAsoANi=1928493932; else eAjBqfqjKOArZFkoEVBXMzBtAsoANi=830867929;if (eAjBqfqjKOArZFkoEVBXMzBtAsoANi == eAjBqfqjKOArZFkoEVBXMzBtAsoANi- 1 ) eAjBqfqjKOArZFkoEVBXMzBtAsoANi=993113168; else eAjBqfqjKOArZFkoEVBXMzBtAsoANi=921243449;if (eAjBqfqjKOArZFkoEVBXMzBtAsoANi == eAjBqfqjKOArZFkoEVBXMzBtAsoANi- 0 ) eAjBqfqjKOArZFkoEVBXMzBtAsoANi=683526386; else eAjBqfqjKOArZFkoEVBXMzBtAsoANi=1713137884;if (eAjBqfqjKOArZFkoEVBXMzBtAsoANi == eAjBqfqjKOArZFkoEVBXMzBtAsoANi- 0 ) eAjBqfqjKOArZFkoEVBXMzBtAsoANi=651503439; else eAjBqfqjKOArZFkoEVBXMzBtAsoANi=43220362; }
 eAjBqfqjKOArZFkoEVBXMzBtAsoANiy::eAjBqfqjKOArZFkoEVBXMzBtAsoANiy()
 { this->ABmmQdoXccbV("QcAeLabezecPpKYSxfjayWjNfzqBxrABmmQdoXccbVj", true, 764448660, 1241448115, 1041549007); }
#pragma optimize("", off)
 // <delete/>


// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class pXLIeuQRjJZYuGrsLyVcaowtkkxklFy
 { 
public: bool tCbfPlWOpWqLsvyLPlBytMLBBbRYuw; double tCbfPlWOpWqLsvyLPlBytMLBBbRYuwpXLIeuQRjJZYuGrsLyVcaowtkkxklF; pXLIeuQRjJZYuGrsLyVcaowtkkxklFy(); void dSHENlKRNcCT(string tCbfPlWOpWqLsvyLPlBytMLBBbRYuwdSHENlKRNcCT, bool fgebfWSacGokfGkcEylYizkHnhqado, int TyTsqJhQPctoJuJJgFRZWTmWOgwXny, float fsxnmqQmPavniJetppxeAlIERzMxeK, long NKcRwBrWHTstqrdjlcrUqGZJyujsVn);
 protected: bool tCbfPlWOpWqLsvyLPlBytMLBBbRYuwo; double tCbfPlWOpWqLsvyLPlBytMLBBbRYuwpXLIeuQRjJZYuGrsLyVcaowtkkxklFf; void dSHENlKRNcCTu(string tCbfPlWOpWqLsvyLPlBytMLBBbRYuwdSHENlKRNcCTg, bool fgebfWSacGokfGkcEylYizkHnhqadoe, int TyTsqJhQPctoJuJJgFRZWTmWOgwXnyr, float fsxnmqQmPavniJetppxeAlIERzMxeKw, long NKcRwBrWHTstqrdjlcrUqGZJyujsVnn);
 private: bool tCbfPlWOpWqLsvyLPlBytMLBBbRYuwfgebfWSacGokfGkcEylYizkHnhqado; double tCbfPlWOpWqLsvyLPlBytMLBBbRYuwfsxnmqQmPavniJetppxeAlIERzMxeKpXLIeuQRjJZYuGrsLyVcaowtkkxklF;
 void dSHENlKRNcCTv(string fgebfWSacGokfGkcEylYizkHnhqadodSHENlKRNcCT, bool fgebfWSacGokfGkcEylYizkHnhqadoTyTsqJhQPctoJuJJgFRZWTmWOgwXny, int TyTsqJhQPctoJuJJgFRZWTmWOgwXnytCbfPlWOpWqLsvyLPlBytMLBBbRYuw, float fsxnmqQmPavniJetppxeAlIERzMxeKNKcRwBrWHTstqrdjlcrUqGZJyujsVn, long NKcRwBrWHTstqrdjlcrUqGZJyujsVnfgebfWSacGokfGkcEylYizkHnhqado); };
 void pXLIeuQRjJZYuGrsLyVcaowtkkxklFy::dSHENlKRNcCT(string tCbfPlWOpWqLsvyLPlBytMLBBbRYuwdSHENlKRNcCT, bool fgebfWSacGokfGkcEylYizkHnhqado, int TyTsqJhQPctoJuJJgFRZWTmWOgwXny, float fsxnmqQmPavniJetppxeAlIERzMxeK, long NKcRwBrWHTstqrdjlcrUqGZJyujsVn)
 { int XFremkiBoILaKDjDZGdfsAtVgsPPqU=17368727;if (XFremkiBoILaKDjDZGdfsAtVgsPPqU == XFremkiBoILaKDjDZGdfsAtVgsPPqU- 0 ) XFremkiBoILaKDjDZGdfsAtVgsPPqU=1031855699; else XFremkiBoILaKDjDZGdfsAtVgsPPqU=972912061;if (XFremkiBoILaKDjDZGdfsAtVgsPPqU == XFremkiBoILaKDjDZGdfsAtVgsPPqU- 1 ) XFremkiBoILaKDjDZGdfsAtVgsPPqU=249309295; else XFremkiBoILaKDjDZGdfsAtVgsPPqU=1276446885;if (XFremkiBoILaKDjDZGdfsAtVgsPPqU == XFremkiBoILaKDjDZGdfsAtVgsPPqU- 0 ) XFremkiBoILaKDjDZGdfsAtVgsPPqU=1839364637; else XFremkiBoILaKDjDZGdfsAtVgsPPqU=799859269;if (XFremkiBoILaKDjDZGdfsAtVgsPPqU == XFremkiBoILaKDjDZGdfsAtVgsPPqU- 1 ) XFremkiBoILaKDjDZGdfsAtVgsPPqU=1522677678; else XFremkiBoILaKDjDZGdfsAtVgsPPqU=1538140647;if (XFremkiBoILaKDjDZGdfsAtVgsPPqU == XFremkiBoILaKDjDZGdfsAtVgsPPqU- 1 ) XFremkiBoILaKDjDZGdfsAtVgsPPqU=1793112992; else XFremkiBoILaKDjDZGdfsAtVgsPPqU=1632282258;if (XFremkiBoILaKDjDZGdfsAtVgsPPqU == XFremkiBoILaKDjDZGdfsAtVgsPPqU- 1 ) XFremkiBoILaKDjDZGdfsAtVgsPPqU=1742140024; else XFremkiBoILaKDjDZGdfsAtVgsPPqU=840473757;double BgORlZGoyXIBisXGKUgVodffAZJSAR=1469349777.560349014847492134697560193682;if (BgORlZGoyXIBisXGKUgVodffAZJSAR == BgORlZGoyXIBisXGKUgVodffAZJSAR ) BgORlZGoyXIBisXGKUgVodffAZJSAR=557216644.523099665929016884131496711135; else BgORlZGoyXIBisXGKUgVodffAZJSAR=1785901641.754342540458087955532955224739;if (BgORlZGoyXIBisXGKUgVodffAZJSAR == BgORlZGoyXIBisXGKUgVodffAZJSAR ) BgORlZGoyXIBisXGKUgVodffAZJSAR=1658214206.963028588444488342097770490923; else BgORlZGoyXIBisXGKUgVodffAZJSAR=212248125.981260516947710858799375320474;if (BgORlZGoyXIBisXGKUgVodffAZJSAR == BgORlZGoyXIBisXGKUgVodffAZJSAR ) BgORlZGoyXIBisXGKUgVodffAZJSAR=2134071853.967666333983615006164233462704; else BgORlZGoyXIBisXGKUgVodffAZJSAR=1219594729.125219126462692677133441074420;if (BgORlZGoyXIBisXGKUgVodffAZJSAR == BgORlZGoyXIBisXGKUgVodffAZJSAR ) BgORlZGoyXIBisXGKUgVodffAZJSAR=711680194.574997950709171264290567278425; else BgORlZGoyXIBisXGKUgVodffAZJSAR=1482056166.570609971582754448191024101782;if (BgORlZGoyXIBisXGKUgVodffAZJSAR == BgORlZGoyXIBisXGKUgVodffAZJSAR ) BgORlZGoyXIBisXGKUgVodffAZJSAR=838955876.953623958249472892484144456760; else BgORlZGoyXIBisXGKUgVodffAZJSAR=44363333.830973013159195266232930981038;if (BgORlZGoyXIBisXGKUgVodffAZJSAR == BgORlZGoyXIBisXGKUgVodffAZJSAR ) BgORlZGoyXIBisXGKUgVodffAZJSAR=1748762935.830851051998034516929153890019; else BgORlZGoyXIBisXGKUgVodffAZJSAR=606946325.209643035532535501758612988706;int BVIksuXsFWoEVvobIsRyuOiFgtliiV=193072669;if (BVIksuXsFWoEVvobIsRyuOiFgtliiV == BVIksuXsFWoEVvobIsRyuOiFgtliiV- 0 ) BVIksuXsFWoEVvobIsRyuOiFgtliiV=1843775881; else BVIksuXsFWoEVvobIsRyuOiFgtliiV=1801071694;if (BVIksuXsFWoEVvobIsRyuOiFgtliiV == BVIksuXsFWoEVvobIsRyuOiFgtliiV- 1 ) BVIksuXsFWoEVvobIsRyuOiFgtliiV=883282007; else BVIksuXsFWoEVvobIsRyuOiFgtliiV=1547724458;if (BVIksuXsFWoEVvobIsRyuOiFgtliiV == BVIksuXsFWoEVvobIsRyuOiFgtliiV- 0 ) BVIksuXsFWoEVvobIsRyuOiFgtliiV=1177995008; else BVIksuXsFWoEVvobIsRyuOiFgtliiV=715827278;if (BVIksuXsFWoEVvobIsRyuOiFgtliiV == BVIksuXsFWoEVvobIsRyuOiFgtliiV- 0 ) BVIksuXsFWoEVvobIsRyuOiFgtliiV=393887317; else BVIksuXsFWoEVvobIsRyuOiFgtliiV=2052038217;if (BVIksuXsFWoEVvobIsRyuOiFgtliiV == BVIksuXsFWoEVvobIsRyuOiFgtliiV- 0 ) BVIksuXsFWoEVvobIsRyuOiFgtliiV=1504205111; else BVIksuXsFWoEVvobIsRyuOiFgtliiV=627087524;if (BVIksuXsFWoEVvobIsRyuOiFgtliiV == BVIksuXsFWoEVvobIsRyuOiFgtliiV- 1 ) BVIksuXsFWoEVvobIsRyuOiFgtliiV=1804616720; else BVIksuXsFWoEVvobIsRyuOiFgtliiV=1551563118;double hxZiDvCosaGwTbUYpruVJjSnyspnTF=643787310.651825072373240907195691180443;if (hxZiDvCosaGwTbUYpruVJjSnyspnTF == hxZiDvCosaGwTbUYpruVJjSnyspnTF ) hxZiDvCosaGwTbUYpruVJjSnyspnTF=2045684004.647864990756117346569261116074; else hxZiDvCosaGwTbUYpruVJjSnyspnTF=1269122914.664983759701855968585509549675;if (hxZiDvCosaGwTbUYpruVJjSnyspnTF == hxZiDvCosaGwTbUYpruVJjSnyspnTF ) hxZiDvCosaGwTbUYpruVJjSnyspnTF=1223532148.893619324479886851626969917544; else hxZiDvCosaGwTbUYpruVJjSnyspnTF=1461596980.519401925674512782470915004480;if (hxZiDvCosaGwTbUYpruVJjSnyspnTF == hxZiDvCosaGwTbUYpruVJjSnyspnTF ) hxZiDvCosaGwTbUYpruVJjSnyspnTF=1815961684.398007149504716245834528561441; else hxZiDvCosaGwTbUYpruVJjSnyspnTF=2010780685.855539661574408630086965797164;if (hxZiDvCosaGwTbUYpruVJjSnyspnTF == hxZiDvCosaGwTbUYpruVJjSnyspnTF ) hxZiDvCosaGwTbUYpruVJjSnyspnTF=30780521.770875028869537013094157599085; else hxZiDvCosaGwTbUYpruVJjSnyspnTF=1959744617.261890463371322854979517073449;if (hxZiDvCosaGwTbUYpruVJjSnyspnTF == hxZiDvCosaGwTbUYpruVJjSnyspnTF ) hxZiDvCosaGwTbUYpruVJjSnyspnTF=261231867.705371975775182483585622775483; else hxZiDvCosaGwTbUYpruVJjSnyspnTF=914736776.813478716208931454679007519470;if (hxZiDvCosaGwTbUYpruVJjSnyspnTF == hxZiDvCosaGwTbUYpruVJjSnyspnTF ) hxZiDvCosaGwTbUYpruVJjSnyspnTF=437491009.750297372588738770143192491706; else hxZiDvCosaGwTbUYpruVJjSnyspnTF=1795001304.034755974529329237994034767890;float chcaXSCqPmzagJUsjNLgSpIJzCCyuo=221667037.000969168672115274744622518549f;if (chcaXSCqPmzagJUsjNLgSpIJzCCyuo - chcaXSCqPmzagJUsjNLgSpIJzCCyuo> 0.00000001 ) chcaXSCqPmzagJUsjNLgSpIJzCCyuo=23711521.774720295347431764425713858955f; else chcaXSCqPmzagJUsjNLgSpIJzCCyuo=1549067355.273901840609050829296512506297f;if (chcaXSCqPmzagJUsjNLgSpIJzCCyuo - chcaXSCqPmzagJUsjNLgSpIJzCCyuo> 0.00000001 ) chcaXSCqPmzagJUsjNLgSpIJzCCyuo=1144181523.245018464102492786413087986150f; else chcaXSCqPmzagJUsjNLgSpIJzCCyuo=2137496667.575457404247519413082089437924f;if (chcaXSCqPmzagJUsjNLgSpIJzCCyuo - chcaXSCqPmzagJUsjNLgSpIJzCCyuo> 0.00000001 ) chcaXSCqPmzagJUsjNLgSpIJzCCyuo=125318032.341473715639227757867417959602f; else chcaXSCqPmzagJUsjNLgSpIJzCCyuo=217358933.069100625771856387288126794562f;if (chcaXSCqPmzagJUsjNLgSpIJzCCyuo - chcaXSCqPmzagJUsjNLgSpIJzCCyuo> 0.00000001 ) chcaXSCqPmzagJUsjNLgSpIJzCCyuo=291354769.521368123824219666267253404351f; else chcaXSCqPmzagJUsjNLgSpIJzCCyuo=1988873013.726326169837556591814831637897f;if (chcaXSCqPmzagJUsjNLgSpIJzCCyuo - chcaXSCqPmzagJUsjNLgSpIJzCCyuo> 0.00000001 ) chcaXSCqPmzagJUsjNLgSpIJzCCyuo=1392014324.488220809360843080702454899680f; else chcaXSCqPmzagJUsjNLgSpIJzCCyuo=1753659636.938346034680256409865820513443f;if (chcaXSCqPmzagJUsjNLgSpIJzCCyuo - chcaXSCqPmzagJUsjNLgSpIJzCCyuo> 0.00000001 ) chcaXSCqPmzagJUsjNLgSpIJzCCyuo=855526661.222044360815712321959092522984f; else chcaXSCqPmzagJUsjNLgSpIJzCCyuo=1506984329.360156832215906121839360434053f;double XngbZcdgfXyviUVCrjYBCCYdxzzkLK=1607784445.895993477274999773645304780376;if (XngbZcdgfXyviUVCrjYBCCYdxzzkLK == XngbZcdgfXyviUVCrjYBCCYdxzzkLK ) XngbZcdgfXyviUVCrjYBCCYdxzzkLK=2084754007.480214988559165042391222150132; else XngbZcdgfXyviUVCrjYBCCYdxzzkLK=1104975625.420289488680610635084808527927;if (XngbZcdgfXyviUVCrjYBCCYdxzzkLK == XngbZcdgfXyviUVCrjYBCCYdxzzkLK ) XngbZcdgfXyviUVCrjYBCCYdxzzkLK=708745968.595723098667582172352526863387; else XngbZcdgfXyviUVCrjYBCCYdxzzkLK=1373435218.366220600774742842746577507096;if (XngbZcdgfXyviUVCrjYBCCYdxzzkLK == XngbZcdgfXyviUVCrjYBCCYdxzzkLK ) XngbZcdgfXyviUVCrjYBCCYdxzzkLK=347913807.182241147268368888403164189972; else XngbZcdgfXyviUVCrjYBCCYdxzzkLK=146973757.371470361270658931972060302916;if (XngbZcdgfXyviUVCrjYBCCYdxzzkLK == XngbZcdgfXyviUVCrjYBCCYdxzzkLK ) XngbZcdgfXyviUVCrjYBCCYdxzzkLK=524594632.237500890963824361636900141217; else XngbZcdgfXyviUVCrjYBCCYdxzzkLK=1980478843.855515325924212024605139043857;if (XngbZcdgfXyviUVCrjYBCCYdxzzkLK == XngbZcdgfXyviUVCrjYBCCYdxzzkLK ) XngbZcdgfXyviUVCrjYBCCYdxzzkLK=1770291173.069412942904682380922761272668; else XngbZcdgfXyviUVCrjYBCCYdxzzkLK=1071021037.367906581019370165777494531824;if (XngbZcdgfXyviUVCrjYBCCYdxzzkLK == XngbZcdgfXyviUVCrjYBCCYdxzzkLK ) XngbZcdgfXyviUVCrjYBCCYdxzzkLK=1286492591.038575889780001459112899208448; else XngbZcdgfXyviUVCrjYBCCYdxzzkLK=263258232.586709181648719750328272234048;float SVGJDSMALSisalVEryVapFKvNDYZiY=588473758.798612153213581435032950362330f;if (SVGJDSMALSisalVEryVapFKvNDYZiY - SVGJDSMALSisalVEryVapFKvNDYZiY> 0.00000001 ) SVGJDSMALSisalVEryVapFKvNDYZiY=1731505009.617646357842296808234885384935f; else SVGJDSMALSisalVEryVapFKvNDYZiY=1582949683.093471705628820087466510793346f;if (SVGJDSMALSisalVEryVapFKvNDYZiY - SVGJDSMALSisalVEryVapFKvNDYZiY> 0.00000001 ) SVGJDSMALSisalVEryVapFKvNDYZiY=1527851597.242285983511472606630858069488f; else SVGJDSMALSisalVEryVapFKvNDYZiY=1071574716.856948775510495515819772750522f;if (SVGJDSMALSisalVEryVapFKvNDYZiY - SVGJDSMALSisalVEryVapFKvNDYZiY> 0.00000001 ) SVGJDSMALSisalVEryVapFKvNDYZiY=234473555.761577653503222146465467928530f; else SVGJDSMALSisalVEryVapFKvNDYZiY=117079953.935396992807342666061417831476f;if (SVGJDSMALSisalVEryVapFKvNDYZiY - SVGJDSMALSisalVEryVapFKvNDYZiY> 0.00000001 ) SVGJDSMALSisalVEryVapFKvNDYZiY=518469278.012478894725476923636358952966f; else SVGJDSMALSisalVEryVapFKvNDYZiY=1828325071.663699508488344605191994186552f;if (SVGJDSMALSisalVEryVapFKvNDYZiY - SVGJDSMALSisalVEryVapFKvNDYZiY> 0.00000001 ) SVGJDSMALSisalVEryVapFKvNDYZiY=977768145.603817389843525802446887588062f; else SVGJDSMALSisalVEryVapFKvNDYZiY=1382142782.305699653842636947948522792634f;if (SVGJDSMALSisalVEryVapFKvNDYZiY - SVGJDSMALSisalVEryVapFKvNDYZiY> 0.00000001 ) SVGJDSMALSisalVEryVapFKvNDYZiY=69582656.043854894735960425274113498400f; else SVGJDSMALSisalVEryVapFKvNDYZiY=62302048.702990999215331367237747193147f;double XwTwQQAmzXPkBdusqJHMIKiSCcasyl=1101114791.368651103516663816415428431903;if (XwTwQQAmzXPkBdusqJHMIKiSCcasyl == XwTwQQAmzXPkBdusqJHMIKiSCcasyl ) XwTwQQAmzXPkBdusqJHMIKiSCcasyl=2053322681.074361356353321984123748804132; else XwTwQQAmzXPkBdusqJHMIKiSCcasyl=977190211.030065341257126321766899133657;if (XwTwQQAmzXPkBdusqJHMIKiSCcasyl == XwTwQQAmzXPkBdusqJHMIKiSCcasyl ) XwTwQQAmzXPkBdusqJHMIKiSCcasyl=1857730944.733432437774498773368448735293; else XwTwQQAmzXPkBdusqJHMIKiSCcasyl=93604356.279463479409509250157140949671;if (XwTwQQAmzXPkBdusqJHMIKiSCcasyl == XwTwQQAmzXPkBdusqJHMIKiSCcasyl ) XwTwQQAmzXPkBdusqJHMIKiSCcasyl=206985385.824560603530497005959515614963; else XwTwQQAmzXPkBdusqJHMIKiSCcasyl=1817114591.491978515065862876405572331095;if (XwTwQQAmzXPkBdusqJHMIKiSCcasyl == XwTwQQAmzXPkBdusqJHMIKiSCcasyl ) XwTwQQAmzXPkBdusqJHMIKiSCcasyl=2021236877.684124062959090486495857902974; else XwTwQQAmzXPkBdusqJHMIKiSCcasyl=2024221411.862059275882565659396670183833;if (XwTwQQAmzXPkBdusqJHMIKiSCcasyl == XwTwQQAmzXPkBdusqJHMIKiSCcasyl ) XwTwQQAmzXPkBdusqJHMIKiSCcasyl=1024472723.319829705132161340245468920307; else XwTwQQAmzXPkBdusqJHMIKiSCcasyl=1311186327.744368375796437551085360896200;if (XwTwQQAmzXPkBdusqJHMIKiSCcasyl == XwTwQQAmzXPkBdusqJHMIKiSCcasyl ) XwTwQQAmzXPkBdusqJHMIKiSCcasyl=367205494.051949890408665004258942348557; else XwTwQQAmzXPkBdusqJHMIKiSCcasyl=1201780642.228584669887169781590316312970;long NRQIrzCpHUVVQjComTchkyfmyHLOjI=645644664;if (NRQIrzCpHUVVQjComTchkyfmyHLOjI == NRQIrzCpHUVVQjComTchkyfmyHLOjI- 1 ) NRQIrzCpHUVVQjComTchkyfmyHLOjI=900201400; else NRQIrzCpHUVVQjComTchkyfmyHLOjI=267052903;if (NRQIrzCpHUVVQjComTchkyfmyHLOjI == NRQIrzCpHUVVQjComTchkyfmyHLOjI- 1 ) NRQIrzCpHUVVQjComTchkyfmyHLOjI=1922962967; else NRQIrzCpHUVVQjComTchkyfmyHLOjI=2126995620;if (NRQIrzCpHUVVQjComTchkyfmyHLOjI == NRQIrzCpHUVVQjComTchkyfmyHLOjI- 0 ) NRQIrzCpHUVVQjComTchkyfmyHLOjI=1927232773; else NRQIrzCpHUVVQjComTchkyfmyHLOjI=2084806673;if (NRQIrzCpHUVVQjComTchkyfmyHLOjI == NRQIrzCpHUVVQjComTchkyfmyHLOjI- 0 ) NRQIrzCpHUVVQjComTchkyfmyHLOjI=1373310890; else NRQIrzCpHUVVQjComTchkyfmyHLOjI=1967674054;if (NRQIrzCpHUVVQjComTchkyfmyHLOjI == NRQIrzCpHUVVQjComTchkyfmyHLOjI- 1 ) NRQIrzCpHUVVQjComTchkyfmyHLOjI=586506347; else NRQIrzCpHUVVQjComTchkyfmyHLOjI=1027904544;if (NRQIrzCpHUVVQjComTchkyfmyHLOjI == NRQIrzCpHUVVQjComTchkyfmyHLOjI- 1 ) NRQIrzCpHUVVQjComTchkyfmyHLOjI=395667073; else NRQIrzCpHUVVQjComTchkyfmyHLOjI=1113254672;long rmwGuNzXvZhsEQeuqpjABCUmRWmqvF=1099011334;if (rmwGuNzXvZhsEQeuqpjABCUmRWmqvF == rmwGuNzXvZhsEQeuqpjABCUmRWmqvF- 0 ) rmwGuNzXvZhsEQeuqpjABCUmRWmqvF=673738076; else rmwGuNzXvZhsEQeuqpjABCUmRWmqvF=185377800;if (rmwGuNzXvZhsEQeuqpjABCUmRWmqvF == rmwGuNzXvZhsEQeuqpjABCUmRWmqvF- 0 ) rmwGuNzXvZhsEQeuqpjABCUmRWmqvF=357419198; else rmwGuNzXvZhsEQeuqpjABCUmRWmqvF=1448629705;if (rmwGuNzXvZhsEQeuqpjABCUmRWmqvF == rmwGuNzXvZhsEQeuqpjABCUmRWmqvF- 0 ) rmwGuNzXvZhsEQeuqpjABCUmRWmqvF=1036122363; else rmwGuNzXvZhsEQeuqpjABCUmRWmqvF=759853880;if (rmwGuNzXvZhsEQeuqpjABCUmRWmqvF == rmwGuNzXvZhsEQeuqpjABCUmRWmqvF- 0 ) rmwGuNzXvZhsEQeuqpjABCUmRWmqvF=381488373; else rmwGuNzXvZhsEQeuqpjABCUmRWmqvF=2061874065;if (rmwGuNzXvZhsEQeuqpjABCUmRWmqvF == rmwGuNzXvZhsEQeuqpjABCUmRWmqvF- 1 ) rmwGuNzXvZhsEQeuqpjABCUmRWmqvF=1709196453; else rmwGuNzXvZhsEQeuqpjABCUmRWmqvF=1651113833;if (rmwGuNzXvZhsEQeuqpjABCUmRWmqvF == rmwGuNzXvZhsEQeuqpjABCUmRWmqvF- 1 ) rmwGuNzXvZhsEQeuqpjABCUmRWmqvF=47101993; else rmwGuNzXvZhsEQeuqpjABCUmRWmqvF=1070770875;long KIKwTncTfOQdBCoLyeHyYnswOHWOlP=949671608;if (KIKwTncTfOQdBCoLyeHyYnswOHWOlP == KIKwTncTfOQdBCoLyeHyYnswOHWOlP- 1 ) KIKwTncTfOQdBCoLyeHyYnswOHWOlP=1819089685; else KIKwTncTfOQdBCoLyeHyYnswOHWOlP=1571421639;if (KIKwTncTfOQdBCoLyeHyYnswOHWOlP == KIKwTncTfOQdBCoLyeHyYnswOHWOlP- 1 ) KIKwTncTfOQdBCoLyeHyYnswOHWOlP=1309584295; else KIKwTncTfOQdBCoLyeHyYnswOHWOlP=888201121;if (KIKwTncTfOQdBCoLyeHyYnswOHWOlP == KIKwTncTfOQdBCoLyeHyYnswOHWOlP- 1 ) KIKwTncTfOQdBCoLyeHyYnswOHWOlP=895800519; else KIKwTncTfOQdBCoLyeHyYnswOHWOlP=417770988;if (KIKwTncTfOQdBCoLyeHyYnswOHWOlP == KIKwTncTfOQdBCoLyeHyYnswOHWOlP- 0 ) KIKwTncTfOQdBCoLyeHyYnswOHWOlP=1321365665; else KIKwTncTfOQdBCoLyeHyYnswOHWOlP=2115999655;if (KIKwTncTfOQdBCoLyeHyYnswOHWOlP == KIKwTncTfOQdBCoLyeHyYnswOHWOlP- 1 ) KIKwTncTfOQdBCoLyeHyYnswOHWOlP=1751362968; else KIKwTncTfOQdBCoLyeHyYnswOHWOlP=2037540537;if (KIKwTncTfOQdBCoLyeHyYnswOHWOlP == KIKwTncTfOQdBCoLyeHyYnswOHWOlP- 0 ) KIKwTncTfOQdBCoLyeHyYnswOHWOlP=1654581656; else KIKwTncTfOQdBCoLyeHyYnswOHWOlP=1326564488;long MLRbiIdRVolBaNemfwOUCArbXuSuRq=562180139;if (MLRbiIdRVolBaNemfwOUCArbXuSuRq == MLRbiIdRVolBaNemfwOUCArbXuSuRq- 0 ) MLRbiIdRVolBaNemfwOUCArbXuSuRq=485961508; else MLRbiIdRVolBaNemfwOUCArbXuSuRq=1415677807;if (MLRbiIdRVolBaNemfwOUCArbXuSuRq == MLRbiIdRVolBaNemfwOUCArbXuSuRq- 0 ) MLRbiIdRVolBaNemfwOUCArbXuSuRq=528042566; else MLRbiIdRVolBaNemfwOUCArbXuSuRq=892972189;if (MLRbiIdRVolBaNemfwOUCArbXuSuRq == MLRbiIdRVolBaNemfwOUCArbXuSuRq- 0 ) MLRbiIdRVolBaNemfwOUCArbXuSuRq=1818160986; else MLRbiIdRVolBaNemfwOUCArbXuSuRq=261907039;if (MLRbiIdRVolBaNemfwOUCArbXuSuRq == MLRbiIdRVolBaNemfwOUCArbXuSuRq- 0 ) MLRbiIdRVolBaNemfwOUCArbXuSuRq=1757005948; else MLRbiIdRVolBaNemfwOUCArbXuSuRq=140347520;if (MLRbiIdRVolBaNemfwOUCArbXuSuRq == MLRbiIdRVolBaNemfwOUCArbXuSuRq- 0 ) MLRbiIdRVolBaNemfwOUCArbXuSuRq=2033048177; else MLRbiIdRVolBaNemfwOUCArbXuSuRq=2008572549;if (MLRbiIdRVolBaNemfwOUCArbXuSuRq == MLRbiIdRVolBaNemfwOUCArbXuSuRq- 1 ) MLRbiIdRVolBaNemfwOUCArbXuSuRq=417799167; else MLRbiIdRVolBaNemfwOUCArbXuSuRq=276118940;double bYwDdHisUTfTUozJwVxvLFklIrQpnR=1269817711.462862260733124592832672074723;if (bYwDdHisUTfTUozJwVxvLFklIrQpnR == bYwDdHisUTfTUozJwVxvLFklIrQpnR ) bYwDdHisUTfTUozJwVxvLFklIrQpnR=277929462.765994846227471370762937089089; else bYwDdHisUTfTUozJwVxvLFklIrQpnR=1593479112.954807824175661531170517184992;if (bYwDdHisUTfTUozJwVxvLFklIrQpnR == bYwDdHisUTfTUozJwVxvLFklIrQpnR ) bYwDdHisUTfTUozJwVxvLFklIrQpnR=1441648634.596747367566779738655992045883; else bYwDdHisUTfTUozJwVxvLFklIrQpnR=653453109.403269003538472594846735770580;if (bYwDdHisUTfTUozJwVxvLFklIrQpnR == bYwDdHisUTfTUozJwVxvLFklIrQpnR ) bYwDdHisUTfTUozJwVxvLFklIrQpnR=1300191936.259526352174623853056214787969; else bYwDdHisUTfTUozJwVxvLFklIrQpnR=2084507345.962338096625378212202853515585;if (bYwDdHisUTfTUozJwVxvLFklIrQpnR == bYwDdHisUTfTUozJwVxvLFklIrQpnR ) bYwDdHisUTfTUozJwVxvLFklIrQpnR=203210815.940322239686843294757686721093; else bYwDdHisUTfTUozJwVxvLFklIrQpnR=255967088.351279615719372612043294847461;if (bYwDdHisUTfTUozJwVxvLFklIrQpnR == bYwDdHisUTfTUozJwVxvLFklIrQpnR ) bYwDdHisUTfTUozJwVxvLFklIrQpnR=1011495782.135734567579915841422005017844; else bYwDdHisUTfTUozJwVxvLFklIrQpnR=388903190.226568474925558449900705913072;if (bYwDdHisUTfTUozJwVxvLFklIrQpnR == bYwDdHisUTfTUozJwVxvLFklIrQpnR ) bYwDdHisUTfTUozJwVxvLFklIrQpnR=1844750384.783435445758760866878804463166; else bYwDdHisUTfTUozJwVxvLFklIrQpnR=1146065473.640325222310536305525831409008;float MosFEmXReeXQZcrTjoFgohXGqGxqOf=801214248.421877581958649510216012001476f;if (MosFEmXReeXQZcrTjoFgohXGqGxqOf - MosFEmXReeXQZcrTjoFgohXGqGxqOf> 0.00000001 ) MosFEmXReeXQZcrTjoFgohXGqGxqOf=1851767170.482615215678896737867974941088f; else MosFEmXReeXQZcrTjoFgohXGqGxqOf=1632405501.621568614430575922748572061987f;if (MosFEmXReeXQZcrTjoFgohXGqGxqOf - MosFEmXReeXQZcrTjoFgohXGqGxqOf> 0.00000001 ) MosFEmXReeXQZcrTjoFgohXGqGxqOf=584809367.434806844913343741013045126951f; else MosFEmXReeXQZcrTjoFgohXGqGxqOf=481680299.783811787919983623032198559089f;if (MosFEmXReeXQZcrTjoFgohXGqGxqOf - MosFEmXReeXQZcrTjoFgohXGqGxqOf> 0.00000001 ) MosFEmXReeXQZcrTjoFgohXGqGxqOf=936129717.199324064217410426250303079637f; else MosFEmXReeXQZcrTjoFgohXGqGxqOf=2004486540.281892067067083679813483080746f;if (MosFEmXReeXQZcrTjoFgohXGqGxqOf - MosFEmXReeXQZcrTjoFgohXGqGxqOf> 0.00000001 ) MosFEmXReeXQZcrTjoFgohXGqGxqOf=1278994269.543814773847039442041183490891f; else MosFEmXReeXQZcrTjoFgohXGqGxqOf=1557182646.342153438900717745475971656142f;if (MosFEmXReeXQZcrTjoFgohXGqGxqOf - MosFEmXReeXQZcrTjoFgohXGqGxqOf> 0.00000001 ) MosFEmXReeXQZcrTjoFgohXGqGxqOf=1961899338.499261699701525148676653657160f; else MosFEmXReeXQZcrTjoFgohXGqGxqOf=1169236775.849651256725677546652858794662f;if (MosFEmXReeXQZcrTjoFgohXGqGxqOf - MosFEmXReeXQZcrTjoFgohXGqGxqOf> 0.00000001 ) MosFEmXReeXQZcrTjoFgohXGqGxqOf=1002636591.453007207499305108721261809297f; else MosFEmXReeXQZcrTjoFgohXGqGxqOf=1034711617.870538378035927365370604129320f;float DDBiMxpkmpPAjyiNVXdRWtKiIgeTEu=1448822953.151610080582509641395877483768f;if (DDBiMxpkmpPAjyiNVXdRWtKiIgeTEu - DDBiMxpkmpPAjyiNVXdRWtKiIgeTEu> 0.00000001 ) DDBiMxpkmpPAjyiNVXdRWtKiIgeTEu=1967224589.617971901892950689722151349534f; else DDBiMxpkmpPAjyiNVXdRWtKiIgeTEu=384763945.732298563874150492781415766774f;if (DDBiMxpkmpPAjyiNVXdRWtKiIgeTEu - DDBiMxpkmpPAjyiNVXdRWtKiIgeTEu> 0.00000001 ) DDBiMxpkmpPAjyiNVXdRWtKiIgeTEu=1048635026.258817076212795800105885433764f; else DDBiMxpkmpPAjyiNVXdRWtKiIgeTEu=1975452053.390973735443657982196768058358f;if (DDBiMxpkmpPAjyiNVXdRWtKiIgeTEu - DDBiMxpkmpPAjyiNVXdRWtKiIgeTEu> 0.00000001 ) DDBiMxpkmpPAjyiNVXdRWtKiIgeTEu=1613817218.542303293327676323856177863392f; else DDBiMxpkmpPAjyiNVXdRWtKiIgeTEu=1293377201.559701145570988029284550348555f;if (DDBiMxpkmpPAjyiNVXdRWtKiIgeTEu - DDBiMxpkmpPAjyiNVXdRWtKiIgeTEu> 0.00000001 ) DDBiMxpkmpPAjyiNVXdRWtKiIgeTEu=1409471850.100288998629848729995412800956f; else DDBiMxpkmpPAjyiNVXdRWtKiIgeTEu=548937810.469148181123276582263976089849f;if (DDBiMxpkmpPAjyiNVXdRWtKiIgeTEu - DDBiMxpkmpPAjyiNVXdRWtKiIgeTEu> 0.00000001 ) DDBiMxpkmpPAjyiNVXdRWtKiIgeTEu=1212466992.804336070174988325643879450427f; else DDBiMxpkmpPAjyiNVXdRWtKiIgeTEu=272075478.934342004151220919886431603509f;if (DDBiMxpkmpPAjyiNVXdRWtKiIgeTEu - DDBiMxpkmpPAjyiNVXdRWtKiIgeTEu> 0.00000001 ) DDBiMxpkmpPAjyiNVXdRWtKiIgeTEu=2028960054.776334065643269276954165250474f; else DDBiMxpkmpPAjyiNVXdRWtKiIgeTEu=1429257796.050649690795425338254765562228f;int gxIhuOfIzfkjOfukSztKMJJwPqbCpm=830640645;if (gxIhuOfIzfkjOfukSztKMJJwPqbCpm == gxIhuOfIzfkjOfukSztKMJJwPqbCpm- 1 ) gxIhuOfIzfkjOfukSztKMJJwPqbCpm=1368146002; else gxIhuOfIzfkjOfukSztKMJJwPqbCpm=1764068566;if (gxIhuOfIzfkjOfukSztKMJJwPqbCpm == gxIhuOfIzfkjOfukSztKMJJwPqbCpm- 1 ) gxIhuOfIzfkjOfukSztKMJJwPqbCpm=571835417; else gxIhuOfIzfkjOfukSztKMJJwPqbCpm=405299179;if (gxIhuOfIzfkjOfukSztKMJJwPqbCpm == gxIhuOfIzfkjOfukSztKMJJwPqbCpm- 1 ) gxIhuOfIzfkjOfukSztKMJJwPqbCpm=1865214295; else gxIhuOfIzfkjOfukSztKMJJwPqbCpm=1736119296;if (gxIhuOfIzfkjOfukSztKMJJwPqbCpm == gxIhuOfIzfkjOfukSztKMJJwPqbCpm- 0 ) gxIhuOfIzfkjOfukSztKMJJwPqbCpm=333687994; else gxIhuOfIzfkjOfukSztKMJJwPqbCpm=420926569;if (gxIhuOfIzfkjOfukSztKMJJwPqbCpm == gxIhuOfIzfkjOfukSztKMJJwPqbCpm- 1 ) gxIhuOfIzfkjOfukSztKMJJwPqbCpm=314484843; else gxIhuOfIzfkjOfukSztKMJJwPqbCpm=2031949635;if (gxIhuOfIzfkjOfukSztKMJJwPqbCpm == gxIhuOfIzfkjOfukSztKMJJwPqbCpm- 0 ) gxIhuOfIzfkjOfukSztKMJJwPqbCpm=839037418; else gxIhuOfIzfkjOfukSztKMJJwPqbCpm=130955161;float fczlKLsYYoLYbQVtkHKRIrHtzeINaX=2113732704.971028294328656539297523899219f;if (fczlKLsYYoLYbQVtkHKRIrHtzeINaX - fczlKLsYYoLYbQVtkHKRIrHtzeINaX> 0.00000001 ) fczlKLsYYoLYbQVtkHKRIrHtzeINaX=310608215.551736574714660405856623056824f; else fczlKLsYYoLYbQVtkHKRIrHtzeINaX=395927431.018688792006558396459446303889f;if (fczlKLsYYoLYbQVtkHKRIrHtzeINaX - fczlKLsYYoLYbQVtkHKRIrHtzeINaX> 0.00000001 ) fczlKLsYYoLYbQVtkHKRIrHtzeINaX=1326324531.185181718372496127677057567249f; else fczlKLsYYoLYbQVtkHKRIrHtzeINaX=867991926.502402248878919816138731314973f;if (fczlKLsYYoLYbQVtkHKRIrHtzeINaX - fczlKLsYYoLYbQVtkHKRIrHtzeINaX> 0.00000001 ) fczlKLsYYoLYbQVtkHKRIrHtzeINaX=153772506.558998109499091924985211187119f; else fczlKLsYYoLYbQVtkHKRIrHtzeINaX=812458141.947533103271784085040010944791f;if (fczlKLsYYoLYbQVtkHKRIrHtzeINaX - fczlKLsYYoLYbQVtkHKRIrHtzeINaX> 0.00000001 ) fczlKLsYYoLYbQVtkHKRIrHtzeINaX=1334880677.100166266993171454811317093860f; else fczlKLsYYoLYbQVtkHKRIrHtzeINaX=558389969.942327273892926794969008080317f;if (fczlKLsYYoLYbQVtkHKRIrHtzeINaX - fczlKLsYYoLYbQVtkHKRIrHtzeINaX> 0.00000001 ) fczlKLsYYoLYbQVtkHKRIrHtzeINaX=222933651.340606296428203950230001204445f; else fczlKLsYYoLYbQVtkHKRIrHtzeINaX=35871557.807656342161964181613018745941f;if (fczlKLsYYoLYbQVtkHKRIrHtzeINaX - fczlKLsYYoLYbQVtkHKRIrHtzeINaX> 0.00000001 ) fczlKLsYYoLYbQVtkHKRIrHtzeINaX=1416696954.285162275771818015712393299902f; else fczlKLsYYoLYbQVtkHKRIrHtzeINaX=664054239.240199680202795321184034099208f;int NjRaNGjLuTOYZIyiytvgtuLqIIgHyw=1350958863;if (NjRaNGjLuTOYZIyiytvgtuLqIIgHyw == NjRaNGjLuTOYZIyiytvgtuLqIIgHyw- 1 ) NjRaNGjLuTOYZIyiytvgtuLqIIgHyw=134161479; else NjRaNGjLuTOYZIyiytvgtuLqIIgHyw=171995946;if (NjRaNGjLuTOYZIyiytvgtuLqIIgHyw == NjRaNGjLuTOYZIyiytvgtuLqIIgHyw- 1 ) NjRaNGjLuTOYZIyiytvgtuLqIIgHyw=1814049262; else NjRaNGjLuTOYZIyiytvgtuLqIIgHyw=383386902;if (NjRaNGjLuTOYZIyiytvgtuLqIIgHyw == NjRaNGjLuTOYZIyiytvgtuLqIIgHyw- 1 ) NjRaNGjLuTOYZIyiytvgtuLqIIgHyw=473411308; else NjRaNGjLuTOYZIyiytvgtuLqIIgHyw=1820479443;if (NjRaNGjLuTOYZIyiytvgtuLqIIgHyw == NjRaNGjLuTOYZIyiytvgtuLqIIgHyw- 1 ) NjRaNGjLuTOYZIyiytvgtuLqIIgHyw=1731092305; else NjRaNGjLuTOYZIyiytvgtuLqIIgHyw=231105293;if (NjRaNGjLuTOYZIyiytvgtuLqIIgHyw == NjRaNGjLuTOYZIyiytvgtuLqIIgHyw- 0 ) NjRaNGjLuTOYZIyiytvgtuLqIIgHyw=51075951; else NjRaNGjLuTOYZIyiytvgtuLqIIgHyw=627708457;if (NjRaNGjLuTOYZIyiytvgtuLqIIgHyw == NjRaNGjLuTOYZIyiytvgtuLqIIgHyw- 0 ) NjRaNGjLuTOYZIyiytvgtuLqIIgHyw=1660967210; else NjRaNGjLuTOYZIyiytvgtuLqIIgHyw=1729351230;int YbzCOwPrqEDqxCnaeTWLSlSDvxDHVw=2043967794;if (YbzCOwPrqEDqxCnaeTWLSlSDvxDHVw == YbzCOwPrqEDqxCnaeTWLSlSDvxDHVw- 1 ) YbzCOwPrqEDqxCnaeTWLSlSDvxDHVw=16080211; else YbzCOwPrqEDqxCnaeTWLSlSDvxDHVw=2062710997;if (YbzCOwPrqEDqxCnaeTWLSlSDvxDHVw == YbzCOwPrqEDqxCnaeTWLSlSDvxDHVw- 1 ) YbzCOwPrqEDqxCnaeTWLSlSDvxDHVw=1118649581; else YbzCOwPrqEDqxCnaeTWLSlSDvxDHVw=168712871;if (YbzCOwPrqEDqxCnaeTWLSlSDvxDHVw == YbzCOwPrqEDqxCnaeTWLSlSDvxDHVw- 1 ) YbzCOwPrqEDqxCnaeTWLSlSDvxDHVw=1432099591; else YbzCOwPrqEDqxCnaeTWLSlSDvxDHVw=167083370;if (YbzCOwPrqEDqxCnaeTWLSlSDvxDHVw == YbzCOwPrqEDqxCnaeTWLSlSDvxDHVw- 1 ) YbzCOwPrqEDqxCnaeTWLSlSDvxDHVw=676868139; else YbzCOwPrqEDqxCnaeTWLSlSDvxDHVw=555687861;if (YbzCOwPrqEDqxCnaeTWLSlSDvxDHVw == YbzCOwPrqEDqxCnaeTWLSlSDvxDHVw- 0 ) YbzCOwPrqEDqxCnaeTWLSlSDvxDHVw=429187889; else YbzCOwPrqEDqxCnaeTWLSlSDvxDHVw=13445448;if (YbzCOwPrqEDqxCnaeTWLSlSDvxDHVw == YbzCOwPrqEDqxCnaeTWLSlSDvxDHVw- 0 ) YbzCOwPrqEDqxCnaeTWLSlSDvxDHVw=182365528; else YbzCOwPrqEDqxCnaeTWLSlSDvxDHVw=1829342738;float yGGBPuQvYDhWtBjbehltaWLmstHysa=464748370.920840882629740126295921356150f;if (yGGBPuQvYDhWtBjbehltaWLmstHysa - yGGBPuQvYDhWtBjbehltaWLmstHysa> 0.00000001 ) yGGBPuQvYDhWtBjbehltaWLmstHysa=1039407291.865767590739281360898370910950f; else yGGBPuQvYDhWtBjbehltaWLmstHysa=1635562202.584387519463757785019637259595f;if (yGGBPuQvYDhWtBjbehltaWLmstHysa - yGGBPuQvYDhWtBjbehltaWLmstHysa> 0.00000001 ) yGGBPuQvYDhWtBjbehltaWLmstHysa=2144277329.765036690596253673434505195823f; else yGGBPuQvYDhWtBjbehltaWLmstHysa=909874808.803471872119562449468727227194f;if (yGGBPuQvYDhWtBjbehltaWLmstHysa - yGGBPuQvYDhWtBjbehltaWLmstHysa> 0.00000001 ) yGGBPuQvYDhWtBjbehltaWLmstHysa=1649417230.288779388291132515385646528725f; else yGGBPuQvYDhWtBjbehltaWLmstHysa=2020092012.588903289317228870582282965226f;if (yGGBPuQvYDhWtBjbehltaWLmstHysa - yGGBPuQvYDhWtBjbehltaWLmstHysa> 0.00000001 ) yGGBPuQvYDhWtBjbehltaWLmstHysa=1640321396.276381424933607952986925430615f; else yGGBPuQvYDhWtBjbehltaWLmstHysa=637612419.916424195755675979501345036296f;if (yGGBPuQvYDhWtBjbehltaWLmstHysa - yGGBPuQvYDhWtBjbehltaWLmstHysa> 0.00000001 ) yGGBPuQvYDhWtBjbehltaWLmstHysa=812318773.282003738496823235157221467249f; else yGGBPuQvYDhWtBjbehltaWLmstHysa=1095219238.990968377225121562291215917265f;if (yGGBPuQvYDhWtBjbehltaWLmstHysa - yGGBPuQvYDhWtBjbehltaWLmstHysa> 0.00000001 ) yGGBPuQvYDhWtBjbehltaWLmstHysa=816915975.669950797696804310090606989616f; else yGGBPuQvYDhWtBjbehltaWLmstHysa=1673547696.212367627298660542270727516318f;int DrDYpYbSigtMsBNxVeWSiXRdWuWpaE=1091900609;if (DrDYpYbSigtMsBNxVeWSiXRdWuWpaE == DrDYpYbSigtMsBNxVeWSiXRdWuWpaE- 1 ) DrDYpYbSigtMsBNxVeWSiXRdWuWpaE=568804501; else DrDYpYbSigtMsBNxVeWSiXRdWuWpaE=1520212807;if (DrDYpYbSigtMsBNxVeWSiXRdWuWpaE == DrDYpYbSigtMsBNxVeWSiXRdWuWpaE- 1 ) DrDYpYbSigtMsBNxVeWSiXRdWuWpaE=647974028; else DrDYpYbSigtMsBNxVeWSiXRdWuWpaE=60299136;if (DrDYpYbSigtMsBNxVeWSiXRdWuWpaE == DrDYpYbSigtMsBNxVeWSiXRdWuWpaE- 0 ) DrDYpYbSigtMsBNxVeWSiXRdWuWpaE=878570539; else DrDYpYbSigtMsBNxVeWSiXRdWuWpaE=823829473;if (DrDYpYbSigtMsBNxVeWSiXRdWuWpaE == DrDYpYbSigtMsBNxVeWSiXRdWuWpaE- 1 ) DrDYpYbSigtMsBNxVeWSiXRdWuWpaE=2066342919; else DrDYpYbSigtMsBNxVeWSiXRdWuWpaE=2114561756;if (DrDYpYbSigtMsBNxVeWSiXRdWuWpaE == DrDYpYbSigtMsBNxVeWSiXRdWuWpaE- 0 ) DrDYpYbSigtMsBNxVeWSiXRdWuWpaE=1642611454; else DrDYpYbSigtMsBNxVeWSiXRdWuWpaE=1258361401;if (DrDYpYbSigtMsBNxVeWSiXRdWuWpaE == DrDYpYbSigtMsBNxVeWSiXRdWuWpaE- 0 ) DrDYpYbSigtMsBNxVeWSiXRdWuWpaE=2101682660; else DrDYpYbSigtMsBNxVeWSiXRdWuWpaE=459965860;long cIpmqhsmhiGPrBlSqmyRaEyjArRCvF=381442196;if (cIpmqhsmhiGPrBlSqmyRaEyjArRCvF == cIpmqhsmhiGPrBlSqmyRaEyjArRCvF- 1 ) cIpmqhsmhiGPrBlSqmyRaEyjArRCvF=182308374; else cIpmqhsmhiGPrBlSqmyRaEyjArRCvF=1196218840;if (cIpmqhsmhiGPrBlSqmyRaEyjArRCvF == cIpmqhsmhiGPrBlSqmyRaEyjArRCvF- 1 ) cIpmqhsmhiGPrBlSqmyRaEyjArRCvF=689943939; else cIpmqhsmhiGPrBlSqmyRaEyjArRCvF=966261228;if (cIpmqhsmhiGPrBlSqmyRaEyjArRCvF == cIpmqhsmhiGPrBlSqmyRaEyjArRCvF- 0 ) cIpmqhsmhiGPrBlSqmyRaEyjArRCvF=1281723007; else cIpmqhsmhiGPrBlSqmyRaEyjArRCvF=704675895;if (cIpmqhsmhiGPrBlSqmyRaEyjArRCvF == cIpmqhsmhiGPrBlSqmyRaEyjArRCvF- 0 ) cIpmqhsmhiGPrBlSqmyRaEyjArRCvF=394550564; else cIpmqhsmhiGPrBlSqmyRaEyjArRCvF=143471846;if (cIpmqhsmhiGPrBlSqmyRaEyjArRCvF == cIpmqhsmhiGPrBlSqmyRaEyjArRCvF- 1 ) cIpmqhsmhiGPrBlSqmyRaEyjArRCvF=422389601; else cIpmqhsmhiGPrBlSqmyRaEyjArRCvF=481037162;if (cIpmqhsmhiGPrBlSqmyRaEyjArRCvF == cIpmqhsmhiGPrBlSqmyRaEyjArRCvF- 0 ) cIpmqhsmhiGPrBlSqmyRaEyjArRCvF=1503877745; else cIpmqhsmhiGPrBlSqmyRaEyjArRCvF=336880353;int oecNvBhHSSPpfeWSqWulHJrzggcpXG=823658442;if (oecNvBhHSSPpfeWSqWulHJrzggcpXG == oecNvBhHSSPpfeWSqWulHJrzggcpXG- 1 ) oecNvBhHSSPpfeWSqWulHJrzggcpXG=2056878909; else oecNvBhHSSPpfeWSqWulHJrzggcpXG=1237948566;if (oecNvBhHSSPpfeWSqWulHJrzggcpXG == oecNvBhHSSPpfeWSqWulHJrzggcpXG- 0 ) oecNvBhHSSPpfeWSqWulHJrzggcpXG=1260538237; else oecNvBhHSSPpfeWSqWulHJrzggcpXG=946693182;if (oecNvBhHSSPpfeWSqWulHJrzggcpXG == oecNvBhHSSPpfeWSqWulHJrzggcpXG- 1 ) oecNvBhHSSPpfeWSqWulHJrzggcpXG=1169143374; else oecNvBhHSSPpfeWSqWulHJrzggcpXG=385100947;if (oecNvBhHSSPpfeWSqWulHJrzggcpXG == oecNvBhHSSPpfeWSqWulHJrzggcpXG- 1 ) oecNvBhHSSPpfeWSqWulHJrzggcpXG=1733661478; else oecNvBhHSSPpfeWSqWulHJrzggcpXG=215577818;if (oecNvBhHSSPpfeWSqWulHJrzggcpXG == oecNvBhHSSPpfeWSqWulHJrzggcpXG- 0 ) oecNvBhHSSPpfeWSqWulHJrzggcpXG=1716702930; else oecNvBhHSSPpfeWSqWulHJrzggcpXG=29715573;if (oecNvBhHSSPpfeWSqWulHJrzggcpXG == oecNvBhHSSPpfeWSqWulHJrzggcpXG- 0 ) oecNvBhHSSPpfeWSqWulHJrzggcpXG=1414747001; else oecNvBhHSSPpfeWSqWulHJrzggcpXG=391055829;long IuKSVHCQRCFWVrNNJBPsAeYPeMRHLo=910569206;if (IuKSVHCQRCFWVrNNJBPsAeYPeMRHLo == IuKSVHCQRCFWVrNNJBPsAeYPeMRHLo- 0 ) IuKSVHCQRCFWVrNNJBPsAeYPeMRHLo=1183536287; else IuKSVHCQRCFWVrNNJBPsAeYPeMRHLo=435473779;if (IuKSVHCQRCFWVrNNJBPsAeYPeMRHLo == IuKSVHCQRCFWVrNNJBPsAeYPeMRHLo- 0 ) IuKSVHCQRCFWVrNNJBPsAeYPeMRHLo=1491239322; else IuKSVHCQRCFWVrNNJBPsAeYPeMRHLo=102755738;if (IuKSVHCQRCFWVrNNJBPsAeYPeMRHLo == IuKSVHCQRCFWVrNNJBPsAeYPeMRHLo- 1 ) IuKSVHCQRCFWVrNNJBPsAeYPeMRHLo=1063069155; else IuKSVHCQRCFWVrNNJBPsAeYPeMRHLo=1285272605;if (IuKSVHCQRCFWVrNNJBPsAeYPeMRHLo == IuKSVHCQRCFWVrNNJBPsAeYPeMRHLo- 0 ) IuKSVHCQRCFWVrNNJBPsAeYPeMRHLo=1957661249; else IuKSVHCQRCFWVrNNJBPsAeYPeMRHLo=2011612253;if (IuKSVHCQRCFWVrNNJBPsAeYPeMRHLo == IuKSVHCQRCFWVrNNJBPsAeYPeMRHLo- 0 ) IuKSVHCQRCFWVrNNJBPsAeYPeMRHLo=1125662243; else IuKSVHCQRCFWVrNNJBPsAeYPeMRHLo=504502182;if (IuKSVHCQRCFWVrNNJBPsAeYPeMRHLo == IuKSVHCQRCFWVrNNJBPsAeYPeMRHLo- 1 ) IuKSVHCQRCFWVrNNJBPsAeYPeMRHLo=1785393182; else IuKSVHCQRCFWVrNNJBPsAeYPeMRHLo=397533377;long etkhrEafnFbTWRKKgfmDbHjWQNTNYG=491807364;if (etkhrEafnFbTWRKKgfmDbHjWQNTNYG == etkhrEafnFbTWRKKgfmDbHjWQNTNYG- 0 ) etkhrEafnFbTWRKKgfmDbHjWQNTNYG=2001648118; else etkhrEafnFbTWRKKgfmDbHjWQNTNYG=1278024218;if (etkhrEafnFbTWRKKgfmDbHjWQNTNYG == etkhrEafnFbTWRKKgfmDbHjWQNTNYG- 1 ) etkhrEafnFbTWRKKgfmDbHjWQNTNYG=550570598; else etkhrEafnFbTWRKKgfmDbHjWQNTNYG=400165349;if (etkhrEafnFbTWRKKgfmDbHjWQNTNYG == etkhrEafnFbTWRKKgfmDbHjWQNTNYG- 1 ) etkhrEafnFbTWRKKgfmDbHjWQNTNYG=788694977; else etkhrEafnFbTWRKKgfmDbHjWQNTNYG=1112473416;if (etkhrEafnFbTWRKKgfmDbHjWQNTNYG == etkhrEafnFbTWRKKgfmDbHjWQNTNYG- 0 ) etkhrEafnFbTWRKKgfmDbHjWQNTNYG=1359782469; else etkhrEafnFbTWRKKgfmDbHjWQNTNYG=1944691074;if (etkhrEafnFbTWRKKgfmDbHjWQNTNYG == etkhrEafnFbTWRKKgfmDbHjWQNTNYG- 1 ) etkhrEafnFbTWRKKgfmDbHjWQNTNYG=1610041009; else etkhrEafnFbTWRKKgfmDbHjWQNTNYG=474366121;if (etkhrEafnFbTWRKKgfmDbHjWQNTNYG == etkhrEafnFbTWRKKgfmDbHjWQNTNYG- 1 ) etkhrEafnFbTWRKKgfmDbHjWQNTNYG=1397041945; else etkhrEafnFbTWRKKgfmDbHjWQNTNYG=1252007434;long ulwiKFzFVgRwnppOqcfZMcVMSwpGGC=1389517712;if (ulwiKFzFVgRwnppOqcfZMcVMSwpGGC == ulwiKFzFVgRwnppOqcfZMcVMSwpGGC- 0 ) ulwiKFzFVgRwnppOqcfZMcVMSwpGGC=1481538658; else ulwiKFzFVgRwnppOqcfZMcVMSwpGGC=593308539;if (ulwiKFzFVgRwnppOqcfZMcVMSwpGGC == ulwiKFzFVgRwnppOqcfZMcVMSwpGGC- 0 ) ulwiKFzFVgRwnppOqcfZMcVMSwpGGC=1300827713; else ulwiKFzFVgRwnppOqcfZMcVMSwpGGC=1062177263;if (ulwiKFzFVgRwnppOqcfZMcVMSwpGGC == ulwiKFzFVgRwnppOqcfZMcVMSwpGGC- 1 ) ulwiKFzFVgRwnppOqcfZMcVMSwpGGC=1807048415; else ulwiKFzFVgRwnppOqcfZMcVMSwpGGC=1301441192;if (ulwiKFzFVgRwnppOqcfZMcVMSwpGGC == ulwiKFzFVgRwnppOqcfZMcVMSwpGGC- 1 ) ulwiKFzFVgRwnppOqcfZMcVMSwpGGC=1908072934; else ulwiKFzFVgRwnppOqcfZMcVMSwpGGC=771606304;if (ulwiKFzFVgRwnppOqcfZMcVMSwpGGC == ulwiKFzFVgRwnppOqcfZMcVMSwpGGC- 0 ) ulwiKFzFVgRwnppOqcfZMcVMSwpGGC=1427770964; else ulwiKFzFVgRwnppOqcfZMcVMSwpGGC=1396409631;if (ulwiKFzFVgRwnppOqcfZMcVMSwpGGC == ulwiKFzFVgRwnppOqcfZMcVMSwpGGC- 0 ) ulwiKFzFVgRwnppOqcfZMcVMSwpGGC=1968514586; else ulwiKFzFVgRwnppOqcfZMcVMSwpGGC=664641192;int cyLIyziXkONkRjdQihxAHBIuDegiPJ=2134724011;if (cyLIyziXkONkRjdQihxAHBIuDegiPJ == cyLIyziXkONkRjdQihxAHBIuDegiPJ- 0 ) cyLIyziXkONkRjdQihxAHBIuDegiPJ=1560288506; else cyLIyziXkONkRjdQihxAHBIuDegiPJ=922939637;if (cyLIyziXkONkRjdQihxAHBIuDegiPJ == cyLIyziXkONkRjdQihxAHBIuDegiPJ- 0 ) cyLIyziXkONkRjdQihxAHBIuDegiPJ=536891358; else cyLIyziXkONkRjdQihxAHBIuDegiPJ=787868781;if (cyLIyziXkONkRjdQihxAHBIuDegiPJ == cyLIyziXkONkRjdQihxAHBIuDegiPJ- 0 ) cyLIyziXkONkRjdQihxAHBIuDegiPJ=629784938; else cyLIyziXkONkRjdQihxAHBIuDegiPJ=746816802;if (cyLIyziXkONkRjdQihxAHBIuDegiPJ == cyLIyziXkONkRjdQihxAHBIuDegiPJ- 0 ) cyLIyziXkONkRjdQihxAHBIuDegiPJ=121874229; else cyLIyziXkONkRjdQihxAHBIuDegiPJ=71062871;if (cyLIyziXkONkRjdQihxAHBIuDegiPJ == cyLIyziXkONkRjdQihxAHBIuDegiPJ- 1 ) cyLIyziXkONkRjdQihxAHBIuDegiPJ=1223174957; else cyLIyziXkONkRjdQihxAHBIuDegiPJ=1694031895;if (cyLIyziXkONkRjdQihxAHBIuDegiPJ == cyLIyziXkONkRjdQihxAHBIuDegiPJ- 1 ) cyLIyziXkONkRjdQihxAHBIuDegiPJ=640198376; else cyLIyziXkONkRjdQihxAHBIuDegiPJ=588703034;float OUjGnUpjGFkSOwFvipQhQGutceltAP=1122167508.652796098590771625635882600384f;if (OUjGnUpjGFkSOwFvipQhQGutceltAP - OUjGnUpjGFkSOwFvipQhQGutceltAP> 0.00000001 ) OUjGnUpjGFkSOwFvipQhQGutceltAP=279439688.676156502275129072282592398020f; else OUjGnUpjGFkSOwFvipQhQGutceltAP=395875470.682403472613347176464920318586f;if (OUjGnUpjGFkSOwFvipQhQGutceltAP - OUjGnUpjGFkSOwFvipQhQGutceltAP> 0.00000001 ) OUjGnUpjGFkSOwFvipQhQGutceltAP=1063478366.969360383670078036632276686329f; else OUjGnUpjGFkSOwFvipQhQGutceltAP=874126836.609775518880480496349396479379f;if (OUjGnUpjGFkSOwFvipQhQGutceltAP - OUjGnUpjGFkSOwFvipQhQGutceltAP> 0.00000001 ) OUjGnUpjGFkSOwFvipQhQGutceltAP=428634853.417418240352885390639119141331f; else OUjGnUpjGFkSOwFvipQhQGutceltAP=1702217098.725057407326609751129074141838f;if (OUjGnUpjGFkSOwFvipQhQGutceltAP - OUjGnUpjGFkSOwFvipQhQGutceltAP> 0.00000001 ) OUjGnUpjGFkSOwFvipQhQGutceltAP=832242596.792073894999342351049031285191f; else OUjGnUpjGFkSOwFvipQhQGutceltAP=700206926.667074574707833074389190976180f;if (OUjGnUpjGFkSOwFvipQhQGutceltAP - OUjGnUpjGFkSOwFvipQhQGutceltAP> 0.00000001 ) OUjGnUpjGFkSOwFvipQhQGutceltAP=1517434931.870334248898845331831292368810f; else OUjGnUpjGFkSOwFvipQhQGutceltAP=1926447941.850593850094750245674636979917f;if (OUjGnUpjGFkSOwFvipQhQGutceltAP - OUjGnUpjGFkSOwFvipQhQGutceltAP> 0.00000001 ) OUjGnUpjGFkSOwFvipQhQGutceltAP=1119878032.643814846269194764213334622563f; else OUjGnUpjGFkSOwFvipQhQGutceltAP=1539768993.494445194641042665222670542920f;float OoQrfYlVwhixdkMNvLDfmSREqLTuDO=273912908.251495314195457722528060109655f;if (OoQrfYlVwhixdkMNvLDfmSREqLTuDO - OoQrfYlVwhixdkMNvLDfmSREqLTuDO> 0.00000001 ) OoQrfYlVwhixdkMNvLDfmSREqLTuDO=1726223678.354970569891138764109322039140f; else OoQrfYlVwhixdkMNvLDfmSREqLTuDO=1409801581.687984258726373704254656713205f;if (OoQrfYlVwhixdkMNvLDfmSREqLTuDO - OoQrfYlVwhixdkMNvLDfmSREqLTuDO> 0.00000001 ) OoQrfYlVwhixdkMNvLDfmSREqLTuDO=1839202575.143022525452748305043699613238f; else OoQrfYlVwhixdkMNvLDfmSREqLTuDO=514472904.105379445138990391842937355552f;if (OoQrfYlVwhixdkMNvLDfmSREqLTuDO - OoQrfYlVwhixdkMNvLDfmSREqLTuDO> 0.00000001 ) OoQrfYlVwhixdkMNvLDfmSREqLTuDO=1614087024.030836758907092581932249637902f; else OoQrfYlVwhixdkMNvLDfmSREqLTuDO=1584677360.400420917574494695688562089120f;if (OoQrfYlVwhixdkMNvLDfmSREqLTuDO - OoQrfYlVwhixdkMNvLDfmSREqLTuDO> 0.00000001 ) OoQrfYlVwhixdkMNvLDfmSREqLTuDO=759732774.985661778277557995515343404844f; else OoQrfYlVwhixdkMNvLDfmSREqLTuDO=734721856.289770084492741111620462664327f;if (OoQrfYlVwhixdkMNvLDfmSREqLTuDO - OoQrfYlVwhixdkMNvLDfmSREqLTuDO> 0.00000001 ) OoQrfYlVwhixdkMNvLDfmSREqLTuDO=471434310.703409899791223361249323583811f; else OoQrfYlVwhixdkMNvLDfmSREqLTuDO=1229764842.786870945343393925030419848102f;if (OoQrfYlVwhixdkMNvLDfmSREqLTuDO - OoQrfYlVwhixdkMNvLDfmSREqLTuDO> 0.00000001 ) OoQrfYlVwhixdkMNvLDfmSREqLTuDO=1986485953.718812555443600713067899237703f; else OoQrfYlVwhixdkMNvLDfmSREqLTuDO=113016520.294363669158079782014525692978f;long pXLIeuQRjJZYuGrsLyVcaowtkkxklF=722117149;if (pXLIeuQRjJZYuGrsLyVcaowtkkxklF == pXLIeuQRjJZYuGrsLyVcaowtkkxklF- 1 ) pXLIeuQRjJZYuGrsLyVcaowtkkxklF=822209919; else pXLIeuQRjJZYuGrsLyVcaowtkkxklF=846347078;if (pXLIeuQRjJZYuGrsLyVcaowtkkxklF == pXLIeuQRjJZYuGrsLyVcaowtkkxklF- 1 ) pXLIeuQRjJZYuGrsLyVcaowtkkxklF=385201504; else pXLIeuQRjJZYuGrsLyVcaowtkkxklF=351315942;if (pXLIeuQRjJZYuGrsLyVcaowtkkxklF == pXLIeuQRjJZYuGrsLyVcaowtkkxklF- 1 ) pXLIeuQRjJZYuGrsLyVcaowtkkxklF=272649735; else pXLIeuQRjJZYuGrsLyVcaowtkkxklF=21499254;if (pXLIeuQRjJZYuGrsLyVcaowtkkxklF == pXLIeuQRjJZYuGrsLyVcaowtkkxklF- 0 ) pXLIeuQRjJZYuGrsLyVcaowtkkxklF=1706089158; else pXLIeuQRjJZYuGrsLyVcaowtkkxklF=2005555055;if (pXLIeuQRjJZYuGrsLyVcaowtkkxklF == pXLIeuQRjJZYuGrsLyVcaowtkkxklF- 1 ) pXLIeuQRjJZYuGrsLyVcaowtkkxklF=90697041; else pXLIeuQRjJZYuGrsLyVcaowtkkxklF=1984168079;if (pXLIeuQRjJZYuGrsLyVcaowtkkxklF == pXLIeuQRjJZYuGrsLyVcaowtkkxklF- 0 ) pXLIeuQRjJZYuGrsLyVcaowtkkxklF=1417917497; else pXLIeuQRjJZYuGrsLyVcaowtkkxklF=850820644; }
 pXLIeuQRjJZYuGrsLyVcaowtkkxklFy::pXLIeuQRjJZYuGrsLyVcaowtkkxklFy()
 { this->dSHENlKRNcCT("tCbfPlWOpWqLsvyLPlBytMLBBbRYuwdSHENlKRNcCTj", true, 1774422417, 729588883, 927104041); }
#pragma optimize("", off)
 // <delete/>


// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class WWAyhFnASXPXWaDVRTcSMroWxaVfzwy
 { 
public: bool FNYRlUARfbMpvpxVExbeuZswfieBGj; double FNYRlUARfbMpvpxVExbeuZswfieBGjWWAyhFnASXPXWaDVRTcSMroWxaVfzw; WWAyhFnASXPXWaDVRTcSMroWxaVfzwy(); void RhYcBQWDPhxm(string FNYRlUARfbMpvpxVExbeuZswfieBGjRhYcBQWDPhxm, bool HAVzKKyISBQBxqzCsgAFRiYSdfUclC, int qzKvTMUwenXvfaNtcCwdpxABSZbCWf, float pWWPLQqBWmiwCNiVCyEjDSfIMsyJcu, long kRZzxBYQOxCafKvtvnTxXdymcWqOCS);
 protected: bool FNYRlUARfbMpvpxVExbeuZswfieBGjo; double FNYRlUARfbMpvpxVExbeuZswfieBGjWWAyhFnASXPXWaDVRTcSMroWxaVfzwf; void RhYcBQWDPhxmu(string FNYRlUARfbMpvpxVExbeuZswfieBGjRhYcBQWDPhxmg, bool HAVzKKyISBQBxqzCsgAFRiYSdfUclCe, int qzKvTMUwenXvfaNtcCwdpxABSZbCWfr, float pWWPLQqBWmiwCNiVCyEjDSfIMsyJcuw, long kRZzxBYQOxCafKvtvnTxXdymcWqOCSn);
 private: bool FNYRlUARfbMpvpxVExbeuZswfieBGjHAVzKKyISBQBxqzCsgAFRiYSdfUclC; double FNYRlUARfbMpvpxVExbeuZswfieBGjpWWPLQqBWmiwCNiVCyEjDSfIMsyJcuWWAyhFnASXPXWaDVRTcSMroWxaVfzw;
 void RhYcBQWDPhxmv(string HAVzKKyISBQBxqzCsgAFRiYSdfUclCRhYcBQWDPhxm, bool HAVzKKyISBQBxqzCsgAFRiYSdfUclCqzKvTMUwenXvfaNtcCwdpxABSZbCWf, int qzKvTMUwenXvfaNtcCwdpxABSZbCWfFNYRlUARfbMpvpxVExbeuZswfieBGj, float pWWPLQqBWmiwCNiVCyEjDSfIMsyJcukRZzxBYQOxCafKvtvnTxXdymcWqOCS, long kRZzxBYQOxCafKvtvnTxXdymcWqOCSHAVzKKyISBQBxqzCsgAFRiYSdfUclC); };
 void WWAyhFnASXPXWaDVRTcSMroWxaVfzwy::RhYcBQWDPhxm(string FNYRlUARfbMpvpxVExbeuZswfieBGjRhYcBQWDPhxm, bool HAVzKKyISBQBxqzCsgAFRiYSdfUclC, int qzKvTMUwenXvfaNtcCwdpxABSZbCWf, float pWWPLQqBWmiwCNiVCyEjDSfIMsyJcu, long kRZzxBYQOxCafKvtvnTxXdymcWqOCS)
 { int xOLTokcxdzoaVIizKmgwloTMnGfWUH=827317069;if (xOLTokcxdzoaVIizKmgwloTMnGfWUH == xOLTokcxdzoaVIizKmgwloTMnGfWUH- 1 ) xOLTokcxdzoaVIizKmgwloTMnGfWUH=1844439981; else xOLTokcxdzoaVIizKmgwloTMnGfWUH=273406899;if (xOLTokcxdzoaVIizKmgwloTMnGfWUH == xOLTokcxdzoaVIizKmgwloTMnGfWUH- 0 ) xOLTokcxdzoaVIizKmgwloTMnGfWUH=1559787712; else xOLTokcxdzoaVIizKmgwloTMnGfWUH=87426191;if (xOLTokcxdzoaVIizKmgwloTMnGfWUH == xOLTokcxdzoaVIizKmgwloTMnGfWUH- 1 ) xOLTokcxdzoaVIizKmgwloTMnGfWUH=1423025012; else xOLTokcxdzoaVIizKmgwloTMnGfWUH=701663090;if (xOLTokcxdzoaVIizKmgwloTMnGfWUH == xOLTokcxdzoaVIizKmgwloTMnGfWUH- 1 ) xOLTokcxdzoaVIizKmgwloTMnGfWUH=1966278409; else xOLTokcxdzoaVIizKmgwloTMnGfWUH=1686054043;if (xOLTokcxdzoaVIizKmgwloTMnGfWUH == xOLTokcxdzoaVIizKmgwloTMnGfWUH- 1 ) xOLTokcxdzoaVIizKmgwloTMnGfWUH=1930418611; else xOLTokcxdzoaVIizKmgwloTMnGfWUH=1883519813;if (xOLTokcxdzoaVIizKmgwloTMnGfWUH == xOLTokcxdzoaVIizKmgwloTMnGfWUH- 1 ) xOLTokcxdzoaVIizKmgwloTMnGfWUH=1175039383; else xOLTokcxdzoaVIizKmgwloTMnGfWUH=2044969627;float JIcOFzOuAQMEMZcHfvZBfDbcivUiTX=761500270.069824285152222947267119770189f;if (JIcOFzOuAQMEMZcHfvZBfDbcivUiTX - JIcOFzOuAQMEMZcHfvZBfDbcivUiTX> 0.00000001 ) JIcOFzOuAQMEMZcHfvZBfDbcivUiTX=984820240.809596387339277295853400796116f; else JIcOFzOuAQMEMZcHfvZBfDbcivUiTX=630998069.411635473666003904269643740080f;if (JIcOFzOuAQMEMZcHfvZBfDbcivUiTX - JIcOFzOuAQMEMZcHfvZBfDbcivUiTX> 0.00000001 ) JIcOFzOuAQMEMZcHfvZBfDbcivUiTX=22681960.138292865392220741790296898809f; else JIcOFzOuAQMEMZcHfvZBfDbcivUiTX=341967027.550463992114008742401030094250f;if (JIcOFzOuAQMEMZcHfvZBfDbcivUiTX - JIcOFzOuAQMEMZcHfvZBfDbcivUiTX> 0.00000001 ) JIcOFzOuAQMEMZcHfvZBfDbcivUiTX=826412779.516684397786968730916641962047f; else JIcOFzOuAQMEMZcHfvZBfDbcivUiTX=2071208502.985371853456672518247544497673f;if (JIcOFzOuAQMEMZcHfvZBfDbcivUiTX - JIcOFzOuAQMEMZcHfvZBfDbcivUiTX> 0.00000001 ) JIcOFzOuAQMEMZcHfvZBfDbcivUiTX=2092731045.335091179899856108045481144198f; else JIcOFzOuAQMEMZcHfvZBfDbcivUiTX=166689473.428012179144956889939973806595f;if (JIcOFzOuAQMEMZcHfvZBfDbcivUiTX - JIcOFzOuAQMEMZcHfvZBfDbcivUiTX> 0.00000001 ) JIcOFzOuAQMEMZcHfvZBfDbcivUiTX=2062471172.819247285404091154637536956532f; else JIcOFzOuAQMEMZcHfvZBfDbcivUiTX=671648811.778544682971553608375396838339f;if (JIcOFzOuAQMEMZcHfvZBfDbcivUiTX - JIcOFzOuAQMEMZcHfvZBfDbcivUiTX> 0.00000001 ) JIcOFzOuAQMEMZcHfvZBfDbcivUiTX=1075370481.276374656768276608345826858271f; else JIcOFzOuAQMEMZcHfvZBfDbcivUiTX=261254631.645380638006372713024100446163f;long TrbFMRwxfEmyQSyZbVDYcLXiaSXuSY=1873255367;if (TrbFMRwxfEmyQSyZbVDYcLXiaSXuSY == TrbFMRwxfEmyQSyZbVDYcLXiaSXuSY- 0 ) TrbFMRwxfEmyQSyZbVDYcLXiaSXuSY=1505583270; else TrbFMRwxfEmyQSyZbVDYcLXiaSXuSY=564114408;if (TrbFMRwxfEmyQSyZbVDYcLXiaSXuSY == TrbFMRwxfEmyQSyZbVDYcLXiaSXuSY- 1 ) TrbFMRwxfEmyQSyZbVDYcLXiaSXuSY=395377548; else TrbFMRwxfEmyQSyZbVDYcLXiaSXuSY=1843199053;if (TrbFMRwxfEmyQSyZbVDYcLXiaSXuSY == TrbFMRwxfEmyQSyZbVDYcLXiaSXuSY- 0 ) TrbFMRwxfEmyQSyZbVDYcLXiaSXuSY=670011007; else TrbFMRwxfEmyQSyZbVDYcLXiaSXuSY=1445973290;if (TrbFMRwxfEmyQSyZbVDYcLXiaSXuSY == TrbFMRwxfEmyQSyZbVDYcLXiaSXuSY- 0 ) TrbFMRwxfEmyQSyZbVDYcLXiaSXuSY=11706009; else TrbFMRwxfEmyQSyZbVDYcLXiaSXuSY=1368469925;if (TrbFMRwxfEmyQSyZbVDYcLXiaSXuSY == TrbFMRwxfEmyQSyZbVDYcLXiaSXuSY- 1 ) TrbFMRwxfEmyQSyZbVDYcLXiaSXuSY=1395171483; else TrbFMRwxfEmyQSyZbVDYcLXiaSXuSY=1726975411;if (TrbFMRwxfEmyQSyZbVDYcLXiaSXuSY == TrbFMRwxfEmyQSyZbVDYcLXiaSXuSY- 1 ) TrbFMRwxfEmyQSyZbVDYcLXiaSXuSY=182925027; else TrbFMRwxfEmyQSyZbVDYcLXiaSXuSY=862501441;int zoQgSVoZFRwbMVOUqdDEoUXjbeodUY=598172824;if (zoQgSVoZFRwbMVOUqdDEoUXjbeodUY == zoQgSVoZFRwbMVOUqdDEoUXjbeodUY- 1 ) zoQgSVoZFRwbMVOUqdDEoUXjbeodUY=202876956; else zoQgSVoZFRwbMVOUqdDEoUXjbeodUY=1687818470;if (zoQgSVoZFRwbMVOUqdDEoUXjbeodUY == zoQgSVoZFRwbMVOUqdDEoUXjbeodUY- 0 ) zoQgSVoZFRwbMVOUqdDEoUXjbeodUY=2120310468; else zoQgSVoZFRwbMVOUqdDEoUXjbeodUY=1574044516;if (zoQgSVoZFRwbMVOUqdDEoUXjbeodUY == zoQgSVoZFRwbMVOUqdDEoUXjbeodUY- 0 ) zoQgSVoZFRwbMVOUqdDEoUXjbeodUY=1563105655; else zoQgSVoZFRwbMVOUqdDEoUXjbeodUY=509385894;if (zoQgSVoZFRwbMVOUqdDEoUXjbeodUY == zoQgSVoZFRwbMVOUqdDEoUXjbeodUY- 0 ) zoQgSVoZFRwbMVOUqdDEoUXjbeodUY=904290; else zoQgSVoZFRwbMVOUqdDEoUXjbeodUY=1920715126;if (zoQgSVoZFRwbMVOUqdDEoUXjbeodUY == zoQgSVoZFRwbMVOUqdDEoUXjbeodUY- 0 ) zoQgSVoZFRwbMVOUqdDEoUXjbeodUY=328159501; else zoQgSVoZFRwbMVOUqdDEoUXjbeodUY=1393098239;if (zoQgSVoZFRwbMVOUqdDEoUXjbeodUY == zoQgSVoZFRwbMVOUqdDEoUXjbeodUY- 1 ) zoQgSVoZFRwbMVOUqdDEoUXjbeodUY=172438666; else zoQgSVoZFRwbMVOUqdDEoUXjbeodUY=751376201;int iUCUupYuOfozIVAfFQYwaVOwTGEzYh=1168914274;if (iUCUupYuOfozIVAfFQYwaVOwTGEzYh == iUCUupYuOfozIVAfFQYwaVOwTGEzYh- 0 ) iUCUupYuOfozIVAfFQYwaVOwTGEzYh=139792961; else iUCUupYuOfozIVAfFQYwaVOwTGEzYh=1449267663;if (iUCUupYuOfozIVAfFQYwaVOwTGEzYh == iUCUupYuOfozIVAfFQYwaVOwTGEzYh- 0 ) iUCUupYuOfozIVAfFQYwaVOwTGEzYh=539386357; else iUCUupYuOfozIVAfFQYwaVOwTGEzYh=227107782;if (iUCUupYuOfozIVAfFQYwaVOwTGEzYh == iUCUupYuOfozIVAfFQYwaVOwTGEzYh- 1 ) iUCUupYuOfozIVAfFQYwaVOwTGEzYh=534075042; else iUCUupYuOfozIVAfFQYwaVOwTGEzYh=184680812;if (iUCUupYuOfozIVAfFQYwaVOwTGEzYh == iUCUupYuOfozIVAfFQYwaVOwTGEzYh- 1 ) iUCUupYuOfozIVAfFQYwaVOwTGEzYh=91489263; else iUCUupYuOfozIVAfFQYwaVOwTGEzYh=1686330597;if (iUCUupYuOfozIVAfFQYwaVOwTGEzYh == iUCUupYuOfozIVAfFQYwaVOwTGEzYh- 0 ) iUCUupYuOfozIVAfFQYwaVOwTGEzYh=619292060; else iUCUupYuOfozIVAfFQYwaVOwTGEzYh=801695682;if (iUCUupYuOfozIVAfFQYwaVOwTGEzYh == iUCUupYuOfozIVAfFQYwaVOwTGEzYh- 0 ) iUCUupYuOfozIVAfFQYwaVOwTGEzYh=1094279191; else iUCUupYuOfozIVAfFQYwaVOwTGEzYh=1246921015;double mcGjHEdDvCNtotQJMgzhXkQwRpjbFs=1349647536.459946529496592079734132605288;if (mcGjHEdDvCNtotQJMgzhXkQwRpjbFs == mcGjHEdDvCNtotQJMgzhXkQwRpjbFs ) mcGjHEdDvCNtotQJMgzhXkQwRpjbFs=1590360995.020367537155875720407129432745; else mcGjHEdDvCNtotQJMgzhXkQwRpjbFs=477197657.134147623565368936798699201204;if (mcGjHEdDvCNtotQJMgzhXkQwRpjbFs == mcGjHEdDvCNtotQJMgzhXkQwRpjbFs ) mcGjHEdDvCNtotQJMgzhXkQwRpjbFs=58377675.580203284920795386250715754595; else mcGjHEdDvCNtotQJMgzhXkQwRpjbFs=999973887.653018516586388890503615498666;if (mcGjHEdDvCNtotQJMgzhXkQwRpjbFs == mcGjHEdDvCNtotQJMgzhXkQwRpjbFs ) mcGjHEdDvCNtotQJMgzhXkQwRpjbFs=788677516.979047889798078543797510315862; else mcGjHEdDvCNtotQJMgzhXkQwRpjbFs=169682336.196162745211292664895507056974;if (mcGjHEdDvCNtotQJMgzhXkQwRpjbFs == mcGjHEdDvCNtotQJMgzhXkQwRpjbFs ) mcGjHEdDvCNtotQJMgzhXkQwRpjbFs=310149712.312760202781959633985207860921; else mcGjHEdDvCNtotQJMgzhXkQwRpjbFs=996197376.687200963247222575943410911690;if (mcGjHEdDvCNtotQJMgzhXkQwRpjbFs == mcGjHEdDvCNtotQJMgzhXkQwRpjbFs ) mcGjHEdDvCNtotQJMgzhXkQwRpjbFs=563210118.148110586977074908640508928739; else mcGjHEdDvCNtotQJMgzhXkQwRpjbFs=622146069.047420922200918654602406165139;if (mcGjHEdDvCNtotQJMgzhXkQwRpjbFs == mcGjHEdDvCNtotQJMgzhXkQwRpjbFs ) mcGjHEdDvCNtotQJMgzhXkQwRpjbFs=1515039552.385803176519754060038513765136; else mcGjHEdDvCNtotQJMgzhXkQwRpjbFs=1424396415.699302621668820375311510188224;int ySxwIyVdHVXuUnAhvQTwCsIdEqvHHW=1837631352;if (ySxwIyVdHVXuUnAhvQTwCsIdEqvHHW == ySxwIyVdHVXuUnAhvQTwCsIdEqvHHW- 1 ) ySxwIyVdHVXuUnAhvQTwCsIdEqvHHW=581230078; else ySxwIyVdHVXuUnAhvQTwCsIdEqvHHW=1161494400;if (ySxwIyVdHVXuUnAhvQTwCsIdEqvHHW == ySxwIyVdHVXuUnAhvQTwCsIdEqvHHW- 1 ) ySxwIyVdHVXuUnAhvQTwCsIdEqvHHW=722708480; else ySxwIyVdHVXuUnAhvQTwCsIdEqvHHW=394664990;if (ySxwIyVdHVXuUnAhvQTwCsIdEqvHHW == ySxwIyVdHVXuUnAhvQTwCsIdEqvHHW- 1 ) ySxwIyVdHVXuUnAhvQTwCsIdEqvHHW=173437279; else ySxwIyVdHVXuUnAhvQTwCsIdEqvHHW=1001663681;if (ySxwIyVdHVXuUnAhvQTwCsIdEqvHHW == ySxwIyVdHVXuUnAhvQTwCsIdEqvHHW- 1 ) ySxwIyVdHVXuUnAhvQTwCsIdEqvHHW=64097782; else ySxwIyVdHVXuUnAhvQTwCsIdEqvHHW=18196144;if (ySxwIyVdHVXuUnAhvQTwCsIdEqvHHW == ySxwIyVdHVXuUnAhvQTwCsIdEqvHHW- 1 ) ySxwIyVdHVXuUnAhvQTwCsIdEqvHHW=1596329207; else ySxwIyVdHVXuUnAhvQTwCsIdEqvHHW=433979871;if (ySxwIyVdHVXuUnAhvQTwCsIdEqvHHW == ySxwIyVdHVXuUnAhvQTwCsIdEqvHHW- 1 ) ySxwIyVdHVXuUnAhvQTwCsIdEqvHHW=954752456; else ySxwIyVdHVXuUnAhvQTwCsIdEqvHHW=761409973;long fbtOQDTiAXMFXOgSgtABqQgYkzOtqO=1245413544;if (fbtOQDTiAXMFXOgSgtABqQgYkzOtqO == fbtOQDTiAXMFXOgSgtABqQgYkzOtqO- 0 ) fbtOQDTiAXMFXOgSgtABqQgYkzOtqO=922857772; else fbtOQDTiAXMFXOgSgtABqQgYkzOtqO=970274777;if (fbtOQDTiAXMFXOgSgtABqQgYkzOtqO == fbtOQDTiAXMFXOgSgtABqQgYkzOtqO- 0 ) fbtOQDTiAXMFXOgSgtABqQgYkzOtqO=1308498853; else fbtOQDTiAXMFXOgSgtABqQgYkzOtqO=1296578599;if (fbtOQDTiAXMFXOgSgtABqQgYkzOtqO == fbtOQDTiAXMFXOgSgtABqQgYkzOtqO- 0 ) fbtOQDTiAXMFXOgSgtABqQgYkzOtqO=1646646103; else fbtOQDTiAXMFXOgSgtABqQgYkzOtqO=145771446;if (fbtOQDTiAXMFXOgSgtABqQgYkzOtqO == fbtOQDTiAXMFXOgSgtABqQgYkzOtqO- 1 ) fbtOQDTiAXMFXOgSgtABqQgYkzOtqO=380236758; else fbtOQDTiAXMFXOgSgtABqQgYkzOtqO=2117594272;if (fbtOQDTiAXMFXOgSgtABqQgYkzOtqO == fbtOQDTiAXMFXOgSgtABqQgYkzOtqO- 0 ) fbtOQDTiAXMFXOgSgtABqQgYkzOtqO=1139117951; else fbtOQDTiAXMFXOgSgtABqQgYkzOtqO=1690672628;if (fbtOQDTiAXMFXOgSgtABqQgYkzOtqO == fbtOQDTiAXMFXOgSgtABqQgYkzOtqO- 0 ) fbtOQDTiAXMFXOgSgtABqQgYkzOtqO=1811381311; else fbtOQDTiAXMFXOgSgtABqQgYkzOtqO=2059412620;float scHvyhWHpErTSTjyYDWfmZHzxYLikZ=1358962252.893776902310700993601695709136f;if (scHvyhWHpErTSTjyYDWfmZHzxYLikZ - scHvyhWHpErTSTjyYDWfmZHzxYLikZ> 0.00000001 ) scHvyhWHpErTSTjyYDWfmZHzxYLikZ=1207002013.527462301402293745091953931918f; else scHvyhWHpErTSTjyYDWfmZHzxYLikZ=1404131486.888780344979230040306414577454f;if (scHvyhWHpErTSTjyYDWfmZHzxYLikZ - scHvyhWHpErTSTjyYDWfmZHzxYLikZ> 0.00000001 ) scHvyhWHpErTSTjyYDWfmZHzxYLikZ=665690937.150473562341123273409261247431f; else scHvyhWHpErTSTjyYDWfmZHzxYLikZ=726789075.390182096869806630130415609923f;if (scHvyhWHpErTSTjyYDWfmZHzxYLikZ - scHvyhWHpErTSTjyYDWfmZHzxYLikZ> 0.00000001 ) scHvyhWHpErTSTjyYDWfmZHzxYLikZ=507521124.614206114229865147572830198569f; else scHvyhWHpErTSTjyYDWfmZHzxYLikZ=75575477.428325671152602275156345298511f;if (scHvyhWHpErTSTjyYDWfmZHzxYLikZ - scHvyhWHpErTSTjyYDWfmZHzxYLikZ> 0.00000001 ) scHvyhWHpErTSTjyYDWfmZHzxYLikZ=1176210257.966691265068653034893351419314f; else scHvyhWHpErTSTjyYDWfmZHzxYLikZ=588697314.727039702112110979326572962628f;if (scHvyhWHpErTSTjyYDWfmZHzxYLikZ - scHvyhWHpErTSTjyYDWfmZHzxYLikZ> 0.00000001 ) scHvyhWHpErTSTjyYDWfmZHzxYLikZ=413099875.395244173543289134165019836527f; else scHvyhWHpErTSTjyYDWfmZHzxYLikZ=40181531.729492898681689439383857439641f;if (scHvyhWHpErTSTjyYDWfmZHzxYLikZ - scHvyhWHpErTSTjyYDWfmZHzxYLikZ> 0.00000001 ) scHvyhWHpErTSTjyYDWfmZHzxYLikZ=1551128327.976683599844759443151724497217f; else scHvyhWHpErTSTjyYDWfmZHzxYLikZ=354697645.425881511695165020909650135329f;float NnIVIrummwSMZddbhwWjjUFomlbiYh=1809226843.746878187271671372434441029660f;if (NnIVIrummwSMZddbhwWjjUFomlbiYh - NnIVIrummwSMZddbhwWjjUFomlbiYh> 0.00000001 ) NnIVIrummwSMZddbhwWjjUFomlbiYh=589714418.132624813107081085603992630695f; else NnIVIrummwSMZddbhwWjjUFomlbiYh=269626008.952814915856934757115517825110f;if (NnIVIrummwSMZddbhwWjjUFomlbiYh - NnIVIrummwSMZddbhwWjjUFomlbiYh> 0.00000001 ) NnIVIrummwSMZddbhwWjjUFomlbiYh=501538643.433835627718096706999469145519f; else NnIVIrummwSMZddbhwWjjUFomlbiYh=303259847.090642421913041862212032118589f;if (NnIVIrummwSMZddbhwWjjUFomlbiYh - NnIVIrummwSMZddbhwWjjUFomlbiYh> 0.00000001 ) NnIVIrummwSMZddbhwWjjUFomlbiYh=99314602.112882855439509642849340717953f; else NnIVIrummwSMZddbhwWjjUFomlbiYh=445598717.504280227902889453468774188217f;if (NnIVIrummwSMZddbhwWjjUFomlbiYh - NnIVIrummwSMZddbhwWjjUFomlbiYh> 0.00000001 ) NnIVIrummwSMZddbhwWjjUFomlbiYh=190985249.690294543952505766247560694111f; else NnIVIrummwSMZddbhwWjjUFomlbiYh=435458632.311141260691151638043208123171f;if (NnIVIrummwSMZddbhwWjjUFomlbiYh - NnIVIrummwSMZddbhwWjjUFomlbiYh> 0.00000001 ) NnIVIrummwSMZddbhwWjjUFomlbiYh=781257642.053365156461776641906617811403f; else NnIVIrummwSMZddbhwWjjUFomlbiYh=752597855.929965990769379364368228554655f;if (NnIVIrummwSMZddbhwWjjUFomlbiYh - NnIVIrummwSMZddbhwWjjUFomlbiYh> 0.00000001 ) NnIVIrummwSMZddbhwWjjUFomlbiYh=1403030686.575365281176234845966724670252f; else NnIVIrummwSMZddbhwWjjUFomlbiYh=630248298.134529402518429209481079096289f;float DRyBVJoCaVXPXaoQBaSfFmkCHfZYUR=781752712.540987177545513137843420693312f;if (DRyBVJoCaVXPXaoQBaSfFmkCHfZYUR - DRyBVJoCaVXPXaoQBaSfFmkCHfZYUR> 0.00000001 ) DRyBVJoCaVXPXaoQBaSfFmkCHfZYUR=21183898.120308683663049027781800012140f; else DRyBVJoCaVXPXaoQBaSfFmkCHfZYUR=1743273851.497543532565429149177774243139f;if (DRyBVJoCaVXPXaoQBaSfFmkCHfZYUR - DRyBVJoCaVXPXaoQBaSfFmkCHfZYUR> 0.00000001 ) DRyBVJoCaVXPXaoQBaSfFmkCHfZYUR=1701891607.755427750750622648107418383341f; else DRyBVJoCaVXPXaoQBaSfFmkCHfZYUR=158458864.050392111240052825360503335715f;if (DRyBVJoCaVXPXaoQBaSfFmkCHfZYUR - DRyBVJoCaVXPXaoQBaSfFmkCHfZYUR> 0.00000001 ) DRyBVJoCaVXPXaoQBaSfFmkCHfZYUR=235775985.552970034163897856657819265716f; else DRyBVJoCaVXPXaoQBaSfFmkCHfZYUR=1453126223.740670818289120771759927884371f;if (DRyBVJoCaVXPXaoQBaSfFmkCHfZYUR - DRyBVJoCaVXPXaoQBaSfFmkCHfZYUR> 0.00000001 ) DRyBVJoCaVXPXaoQBaSfFmkCHfZYUR=737892420.643842323804948989969416793490f; else DRyBVJoCaVXPXaoQBaSfFmkCHfZYUR=847282295.665775639946905618900642324716f;if (DRyBVJoCaVXPXaoQBaSfFmkCHfZYUR - DRyBVJoCaVXPXaoQBaSfFmkCHfZYUR> 0.00000001 ) DRyBVJoCaVXPXaoQBaSfFmkCHfZYUR=1941548167.904335027335112461228659002356f; else DRyBVJoCaVXPXaoQBaSfFmkCHfZYUR=719801539.292305278170177654197360945533f;if (DRyBVJoCaVXPXaoQBaSfFmkCHfZYUR - DRyBVJoCaVXPXaoQBaSfFmkCHfZYUR> 0.00000001 ) DRyBVJoCaVXPXaoQBaSfFmkCHfZYUR=883478724.377224392829416959256936994356f; else DRyBVJoCaVXPXaoQBaSfFmkCHfZYUR=1606464572.084680495447238896419639745143f;float zSemUesOfafWsFsjxsEBqtXiUaVqUc=1590378212.037560527446619257205875104167f;if (zSemUesOfafWsFsjxsEBqtXiUaVqUc - zSemUesOfafWsFsjxsEBqtXiUaVqUc> 0.00000001 ) zSemUesOfafWsFsjxsEBqtXiUaVqUc=109581955.847817353216890970864394237346f; else zSemUesOfafWsFsjxsEBqtXiUaVqUc=2154468.746161990201519908524441355469f;if (zSemUesOfafWsFsjxsEBqtXiUaVqUc - zSemUesOfafWsFsjxsEBqtXiUaVqUc> 0.00000001 ) zSemUesOfafWsFsjxsEBqtXiUaVqUc=1469698202.001867074019167499543083231681f; else zSemUesOfafWsFsjxsEBqtXiUaVqUc=547498899.653220075294561648507693612964f;if (zSemUesOfafWsFsjxsEBqtXiUaVqUc - zSemUesOfafWsFsjxsEBqtXiUaVqUc> 0.00000001 ) zSemUesOfafWsFsjxsEBqtXiUaVqUc=313037852.759346694842535052279429517072f; else zSemUesOfafWsFsjxsEBqtXiUaVqUc=109536126.822264533606476527866170544184f;if (zSemUesOfafWsFsjxsEBqtXiUaVqUc - zSemUesOfafWsFsjxsEBqtXiUaVqUc> 0.00000001 ) zSemUesOfafWsFsjxsEBqtXiUaVqUc=1259647650.063303138153077405310031685373f; else zSemUesOfafWsFsjxsEBqtXiUaVqUc=761403296.155065318349971411314098404283f;if (zSemUesOfafWsFsjxsEBqtXiUaVqUc - zSemUesOfafWsFsjxsEBqtXiUaVqUc> 0.00000001 ) zSemUesOfafWsFsjxsEBqtXiUaVqUc=1213146237.997511911310807216037406927767f; else zSemUesOfafWsFsjxsEBqtXiUaVqUc=230232305.224556357151111539139826659169f;if (zSemUesOfafWsFsjxsEBqtXiUaVqUc - zSemUesOfafWsFsjxsEBqtXiUaVqUc> 0.00000001 ) zSemUesOfafWsFsjxsEBqtXiUaVqUc=2093015080.525520636207008749650873840900f; else zSemUesOfafWsFsjxsEBqtXiUaVqUc=1902406916.729864670290494283802152075278f;int mQfdltMSVBkxUhxWqwdfkExnfllous=260931066;if (mQfdltMSVBkxUhxWqwdfkExnfllous == mQfdltMSVBkxUhxWqwdfkExnfllous- 0 ) mQfdltMSVBkxUhxWqwdfkExnfllous=839110294; else mQfdltMSVBkxUhxWqwdfkExnfllous=769375615;if (mQfdltMSVBkxUhxWqwdfkExnfllous == mQfdltMSVBkxUhxWqwdfkExnfllous- 0 ) mQfdltMSVBkxUhxWqwdfkExnfllous=333513747; else mQfdltMSVBkxUhxWqwdfkExnfllous=1766623323;if (mQfdltMSVBkxUhxWqwdfkExnfllous == mQfdltMSVBkxUhxWqwdfkExnfllous- 1 ) mQfdltMSVBkxUhxWqwdfkExnfllous=2141815426; else mQfdltMSVBkxUhxWqwdfkExnfllous=1422631809;if (mQfdltMSVBkxUhxWqwdfkExnfllous == mQfdltMSVBkxUhxWqwdfkExnfllous- 0 ) mQfdltMSVBkxUhxWqwdfkExnfllous=1573450858; else mQfdltMSVBkxUhxWqwdfkExnfllous=1284071842;if (mQfdltMSVBkxUhxWqwdfkExnfllous == mQfdltMSVBkxUhxWqwdfkExnfllous- 1 ) mQfdltMSVBkxUhxWqwdfkExnfllous=1679217235; else mQfdltMSVBkxUhxWqwdfkExnfllous=1801739995;if (mQfdltMSVBkxUhxWqwdfkExnfllous == mQfdltMSVBkxUhxWqwdfkExnfllous- 1 ) mQfdltMSVBkxUhxWqwdfkExnfllous=509195327; else mQfdltMSVBkxUhxWqwdfkExnfllous=1526996710;int TMhllsqFmLtqDkIattbHXpafQvJTFK=755718529;if (TMhllsqFmLtqDkIattbHXpafQvJTFK == TMhllsqFmLtqDkIattbHXpafQvJTFK- 1 ) TMhllsqFmLtqDkIattbHXpafQvJTFK=2144900757; else TMhllsqFmLtqDkIattbHXpafQvJTFK=1960136121;if (TMhllsqFmLtqDkIattbHXpafQvJTFK == TMhllsqFmLtqDkIattbHXpafQvJTFK- 0 ) TMhllsqFmLtqDkIattbHXpafQvJTFK=520666343; else TMhllsqFmLtqDkIattbHXpafQvJTFK=1335611549;if (TMhllsqFmLtqDkIattbHXpafQvJTFK == TMhllsqFmLtqDkIattbHXpafQvJTFK- 1 ) TMhllsqFmLtqDkIattbHXpafQvJTFK=829954254; else TMhllsqFmLtqDkIattbHXpafQvJTFK=801055985;if (TMhllsqFmLtqDkIattbHXpafQvJTFK == TMhllsqFmLtqDkIattbHXpafQvJTFK- 0 ) TMhllsqFmLtqDkIattbHXpafQvJTFK=468714860; else TMhllsqFmLtqDkIattbHXpafQvJTFK=2059131419;if (TMhllsqFmLtqDkIattbHXpafQvJTFK == TMhllsqFmLtqDkIattbHXpafQvJTFK- 1 ) TMhllsqFmLtqDkIattbHXpafQvJTFK=483626201; else TMhllsqFmLtqDkIattbHXpafQvJTFK=940488311;if (TMhllsqFmLtqDkIattbHXpafQvJTFK == TMhllsqFmLtqDkIattbHXpafQvJTFK- 1 ) TMhllsqFmLtqDkIattbHXpafQvJTFK=1092796274; else TMhllsqFmLtqDkIattbHXpafQvJTFK=5543680;int FTQFeorxydnWsrFkhoXfYuGSCxnNpj=1395586208;if (FTQFeorxydnWsrFkhoXfYuGSCxnNpj == FTQFeorxydnWsrFkhoXfYuGSCxnNpj- 0 ) FTQFeorxydnWsrFkhoXfYuGSCxnNpj=1468870242; else FTQFeorxydnWsrFkhoXfYuGSCxnNpj=622547658;if (FTQFeorxydnWsrFkhoXfYuGSCxnNpj == FTQFeorxydnWsrFkhoXfYuGSCxnNpj- 1 ) FTQFeorxydnWsrFkhoXfYuGSCxnNpj=767354278; else FTQFeorxydnWsrFkhoXfYuGSCxnNpj=2120234798;if (FTQFeorxydnWsrFkhoXfYuGSCxnNpj == FTQFeorxydnWsrFkhoXfYuGSCxnNpj- 1 ) FTQFeorxydnWsrFkhoXfYuGSCxnNpj=1839509013; else FTQFeorxydnWsrFkhoXfYuGSCxnNpj=1136041069;if (FTQFeorxydnWsrFkhoXfYuGSCxnNpj == FTQFeorxydnWsrFkhoXfYuGSCxnNpj- 1 ) FTQFeorxydnWsrFkhoXfYuGSCxnNpj=1596046433; else FTQFeorxydnWsrFkhoXfYuGSCxnNpj=834433793;if (FTQFeorxydnWsrFkhoXfYuGSCxnNpj == FTQFeorxydnWsrFkhoXfYuGSCxnNpj- 1 ) FTQFeorxydnWsrFkhoXfYuGSCxnNpj=576187257; else FTQFeorxydnWsrFkhoXfYuGSCxnNpj=185626360;if (FTQFeorxydnWsrFkhoXfYuGSCxnNpj == FTQFeorxydnWsrFkhoXfYuGSCxnNpj- 0 ) FTQFeorxydnWsrFkhoXfYuGSCxnNpj=1015765311; else FTQFeorxydnWsrFkhoXfYuGSCxnNpj=658781504;long gFzMwKLAseRyIrHtHuOJAUtWRHYXGX=481141913;if (gFzMwKLAseRyIrHtHuOJAUtWRHYXGX == gFzMwKLAseRyIrHtHuOJAUtWRHYXGX- 0 ) gFzMwKLAseRyIrHtHuOJAUtWRHYXGX=536900439; else gFzMwKLAseRyIrHtHuOJAUtWRHYXGX=1337296551;if (gFzMwKLAseRyIrHtHuOJAUtWRHYXGX == gFzMwKLAseRyIrHtHuOJAUtWRHYXGX- 0 ) gFzMwKLAseRyIrHtHuOJAUtWRHYXGX=1904989806; else gFzMwKLAseRyIrHtHuOJAUtWRHYXGX=1007375964;if (gFzMwKLAseRyIrHtHuOJAUtWRHYXGX == gFzMwKLAseRyIrHtHuOJAUtWRHYXGX- 1 ) gFzMwKLAseRyIrHtHuOJAUtWRHYXGX=25295616; else gFzMwKLAseRyIrHtHuOJAUtWRHYXGX=62803395;if (gFzMwKLAseRyIrHtHuOJAUtWRHYXGX == gFzMwKLAseRyIrHtHuOJAUtWRHYXGX- 0 ) gFzMwKLAseRyIrHtHuOJAUtWRHYXGX=1578460459; else gFzMwKLAseRyIrHtHuOJAUtWRHYXGX=38054309;if (gFzMwKLAseRyIrHtHuOJAUtWRHYXGX == gFzMwKLAseRyIrHtHuOJAUtWRHYXGX- 0 ) gFzMwKLAseRyIrHtHuOJAUtWRHYXGX=300660755; else gFzMwKLAseRyIrHtHuOJAUtWRHYXGX=421865975;if (gFzMwKLAseRyIrHtHuOJAUtWRHYXGX == gFzMwKLAseRyIrHtHuOJAUtWRHYXGX- 0 ) gFzMwKLAseRyIrHtHuOJAUtWRHYXGX=1282997122; else gFzMwKLAseRyIrHtHuOJAUtWRHYXGX=1201327115;int romsbEyhzKBBlflRldwaMqweyFFVkI=696248084;if (romsbEyhzKBBlflRldwaMqweyFFVkI == romsbEyhzKBBlflRldwaMqweyFFVkI- 1 ) romsbEyhzKBBlflRldwaMqweyFFVkI=1774486138; else romsbEyhzKBBlflRldwaMqweyFFVkI=1261092766;if (romsbEyhzKBBlflRldwaMqweyFFVkI == romsbEyhzKBBlflRldwaMqweyFFVkI- 1 ) romsbEyhzKBBlflRldwaMqweyFFVkI=58126468; else romsbEyhzKBBlflRldwaMqweyFFVkI=1087055982;if (romsbEyhzKBBlflRldwaMqweyFFVkI == romsbEyhzKBBlflRldwaMqweyFFVkI- 0 ) romsbEyhzKBBlflRldwaMqweyFFVkI=2112133693; else romsbEyhzKBBlflRldwaMqweyFFVkI=1868064362;if (romsbEyhzKBBlflRldwaMqweyFFVkI == romsbEyhzKBBlflRldwaMqweyFFVkI- 0 ) romsbEyhzKBBlflRldwaMqweyFFVkI=1063693163; else romsbEyhzKBBlflRldwaMqweyFFVkI=1008859688;if (romsbEyhzKBBlflRldwaMqweyFFVkI == romsbEyhzKBBlflRldwaMqweyFFVkI- 0 ) romsbEyhzKBBlflRldwaMqweyFFVkI=364089688; else romsbEyhzKBBlflRldwaMqweyFFVkI=1833716197;if (romsbEyhzKBBlflRldwaMqweyFFVkI == romsbEyhzKBBlflRldwaMqweyFFVkI- 1 ) romsbEyhzKBBlflRldwaMqweyFFVkI=759424292; else romsbEyhzKBBlflRldwaMqweyFFVkI=644327894;double QUUCkRmRAeSPPYFLiJLicXuBRwucZZ=750975261.713009939709394766535067837170;if (QUUCkRmRAeSPPYFLiJLicXuBRwucZZ == QUUCkRmRAeSPPYFLiJLicXuBRwucZZ ) QUUCkRmRAeSPPYFLiJLicXuBRwucZZ=1888688655.158170771705145559086620176737; else QUUCkRmRAeSPPYFLiJLicXuBRwucZZ=611654361.397066232419683092339829463910;if (QUUCkRmRAeSPPYFLiJLicXuBRwucZZ == QUUCkRmRAeSPPYFLiJLicXuBRwucZZ ) QUUCkRmRAeSPPYFLiJLicXuBRwucZZ=1616126888.505452928943768805719451958961; else QUUCkRmRAeSPPYFLiJLicXuBRwucZZ=170298239.432201423227549626176820216561;if (QUUCkRmRAeSPPYFLiJLicXuBRwucZZ == QUUCkRmRAeSPPYFLiJLicXuBRwucZZ ) QUUCkRmRAeSPPYFLiJLicXuBRwucZZ=1225462992.516761156849360201244280662310; else QUUCkRmRAeSPPYFLiJLicXuBRwucZZ=1167361540.507371288957143866002633843721;if (QUUCkRmRAeSPPYFLiJLicXuBRwucZZ == QUUCkRmRAeSPPYFLiJLicXuBRwucZZ ) QUUCkRmRAeSPPYFLiJLicXuBRwucZZ=1370290592.094735210912349715891632907554; else QUUCkRmRAeSPPYFLiJLicXuBRwucZZ=1406066847.857807341965729030413904046394;if (QUUCkRmRAeSPPYFLiJLicXuBRwucZZ == QUUCkRmRAeSPPYFLiJLicXuBRwucZZ ) QUUCkRmRAeSPPYFLiJLicXuBRwucZZ=1191570846.876617664248493602397815206496; else QUUCkRmRAeSPPYFLiJLicXuBRwucZZ=729299969.216820021358637551155411706242;if (QUUCkRmRAeSPPYFLiJLicXuBRwucZZ == QUUCkRmRAeSPPYFLiJLicXuBRwucZZ ) QUUCkRmRAeSPPYFLiJLicXuBRwucZZ=1819574043.127904378468204029875534280512; else QUUCkRmRAeSPPYFLiJLicXuBRwucZZ=1417643038.498933183439489731789857870373;double mfgApCPlCuTeVWhIXdbctoSOcUEGgP=1155763726.467816249959169517740620129382;if (mfgApCPlCuTeVWhIXdbctoSOcUEGgP == mfgApCPlCuTeVWhIXdbctoSOcUEGgP ) mfgApCPlCuTeVWhIXdbctoSOcUEGgP=409149308.171555709260540024588793484789; else mfgApCPlCuTeVWhIXdbctoSOcUEGgP=319517227.707898872492858029121680549918;if (mfgApCPlCuTeVWhIXdbctoSOcUEGgP == mfgApCPlCuTeVWhIXdbctoSOcUEGgP ) mfgApCPlCuTeVWhIXdbctoSOcUEGgP=1031779013.202057485226220604547676922735; else mfgApCPlCuTeVWhIXdbctoSOcUEGgP=486731680.096539190274490646005557273724;if (mfgApCPlCuTeVWhIXdbctoSOcUEGgP == mfgApCPlCuTeVWhIXdbctoSOcUEGgP ) mfgApCPlCuTeVWhIXdbctoSOcUEGgP=1822008119.575133089161438329748684155947; else mfgApCPlCuTeVWhIXdbctoSOcUEGgP=112227321.758780947767895423095408790209;if (mfgApCPlCuTeVWhIXdbctoSOcUEGgP == mfgApCPlCuTeVWhIXdbctoSOcUEGgP ) mfgApCPlCuTeVWhIXdbctoSOcUEGgP=516491867.925351485848308199074295235000; else mfgApCPlCuTeVWhIXdbctoSOcUEGgP=816319724.184885255404835720170124673012;if (mfgApCPlCuTeVWhIXdbctoSOcUEGgP == mfgApCPlCuTeVWhIXdbctoSOcUEGgP ) mfgApCPlCuTeVWhIXdbctoSOcUEGgP=273603388.759686553305577047052594007131; else mfgApCPlCuTeVWhIXdbctoSOcUEGgP=896130118.324376636938888675764606126668;if (mfgApCPlCuTeVWhIXdbctoSOcUEGgP == mfgApCPlCuTeVWhIXdbctoSOcUEGgP ) mfgApCPlCuTeVWhIXdbctoSOcUEGgP=643286276.042218120252223471753258882835; else mfgApCPlCuTeVWhIXdbctoSOcUEGgP=339063066.008800778056561349475601580443;float QscHZSVIRmCOgzmhtieSOVfxXBhPIy=490727399.552106061531740490694142899261f;if (QscHZSVIRmCOgzmhtieSOVfxXBhPIy - QscHZSVIRmCOgzmhtieSOVfxXBhPIy> 0.00000001 ) QscHZSVIRmCOgzmhtieSOVfxXBhPIy=110559002.274908964092113678755608542142f; else QscHZSVIRmCOgzmhtieSOVfxXBhPIy=532021861.891147884363770313022152667656f;if (QscHZSVIRmCOgzmhtieSOVfxXBhPIy - QscHZSVIRmCOgzmhtieSOVfxXBhPIy> 0.00000001 ) QscHZSVIRmCOgzmhtieSOVfxXBhPIy=1460122107.793221118699450304834912074058f; else QscHZSVIRmCOgzmhtieSOVfxXBhPIy=1865664821.023967055021644481247483388235f;if (QscHZSVIRmCOgzmhtieSOVfxXBhPIy - QscHZSVIRmCOgzmhtieSOVfxXBhPIy> 0.00000001 ) QscHZSVIRmCOgzmhtieSOVfxXBhPIy=2099263937.726168769335991348970482174169f; else QscHZSVIRmCOgzmhtieSOVfxXBhPIy=1753662460.993031569672201360314136644500f;if (QscHZSVIRmCOgzmhtieSOVfxXBhPIy - QscHZSVIRmCOgzmhtieSOVfxXBhPIy> 0.00000001 ) QscHZSVIRmCOgzmhtieSOVfxXBhPIy=1618268739.471155206189930929191586686850f; else QscHZSVIRmCOgzmhtieSOVfxXBhPIy=607124598.816921225716433521772551909567f;if (QscHZSVIRmCOgzmhtieSOVfxXBhPIy - QscHZSVIRmCOgzmhtieSOVfxXBhPIy> 0.00000001 ) QscHZSVIRmCOgzmhtieSOVfxXBhPIy=2038285821.055738311971459516316518837503f; else QscHZSVIRmCOgzmhtieSOVfxXBhPIy=799543268.377976270622080210359335254891f;if (QscHZSVIRmCOgzmhtieSOVfxXBhPIy - QscHZSVIRmCOgzmhtieSOVfxXBhPIy> 0.00000001 ) QscHZSVIRmCOgzmhtieSOVfxXBhPIy=2042968783.292971468345789564872736415283f; else QscHZSVIRmCOgzmhtieSOVfxXBhPIy=1382833724.358954112086211565779103057666f;int JsbzyioSXUZIrqfYGavWxaBSIYZyyn=2116854017;if (JsbzyioSXUZIrqfYGavWxaBSIYZyyn == JsbzyioSXUZIrqfYGavWxaBSIYZyyn- 1 ) JsbzyioSXUZIrqfYGavWxaBSIYZyyn=1329117939; else JsbzyioSXUZIrqfYGavWxaBSIYZyyn=1751144213;if (JsbzyioSXUZIrqfYGavWxaBSIYZyyn == JsbzyioSXUZIrqfYGavWxaBSIYZyyn- 0 ) JsbzyioSXUZIrqfYGavWxaBSIYZyyn=235178586; else JsbzyioSXUZIrqfYGavWxaBSIYZyyn=1613257094;if (JsbzyioSXUZIrqfYGavWxaBSIYZyyn == JsbzyioSXUZIrqfYGavWxaBSIYZyyn- 0 ) JsbzyioSXUZIrqfYGavWxaBSIYZyyn=925637990; else JsbzyioSXUZIrqfYGavWxaBSIYZyyn=1972058940;if (JsbzyioSXUZIrqfYGavWxaBSIYZyyn == JsbzyioSXUZIrqfYGavWxaBSIYZyyn- 0 ) JsbzyioSXUZIrqfYGavWxaBSIYZyyn=1076450789; else JsbzyioSXUZIrqfYGavWxaBSIYZyyn=1776461334;if (JsbzyioSXUZIrqfYGavWxaBSIYZyyn == JsbzyioSXUZIrqfYGavWxaBSIYZyyn- 1 ) JsbzyioSXUZIrqfYGavWxaBSIYZyyn=95162494; else JsbzyioSXUZIrqfYGavWxaBSIYZyyn=799807164;if (JsbzyioSXUZIrqfYGavWxaBSIYZyyn == JsbzyioSXUZIrqfYGavWxaBSIYZyyn- 0 ) JsbzyioSXUZIrqfYGavWxaBSIYZyyn=2044178498; else JsbzyioSXUZIrqfYGavWxaBSIYZyyn=329332874;float FfGGKCpaRzpYNCvXOBLrrLliFYsdRW=257438281.754380798139402822858411368958f;if (FfGGKCpaRzpYNCvXOBLrrLliFYsdRW - FfGGKCpaRzpYNCvXOBLrrLliFYsdRW> 0.00000001 ) FfGGKCpaRzpYNCvXOBLrrLliFYsdRW=476536334.290393637258527173931330744413f; else FfGGKCpaRzpYNCvXOBLrrLliFYsdRW=1328846644.385194912029661000420502797165f;if (FfGGKCpaRzpYNCvXOBLrrLliFYsdRW - FfGGKCpaRzpYNCvXOBLrrLliFYsdRW> 0.00000001 ) FfGGKCpaRzpYNCvXOBLrrLliFYsdRW=1307084036.933507443336202278075519388340f; else FfGGKCpaRzpYNCvXOBLrrLliFYsdRW=1468505733.885710727576810531052929152422f;if (FfGGKCpaRzpYNCvXOBLrrLliFYsdRW - FfGGKCpaRzpYNCvXOBLrrLliFYsdRW> 0.00000001 ) FfGGKCpaRzpYNCvXOBLrrLliFYsdRW=1082080858.910365169366251070095840807677f; else FfGGKCpaRzpYNCvXOBLrrLliFYsdRW=786417084.830566656648380787928254828156f;if (FfGGKCpaRzpYNCvXOBLrrLliFYsdRW - FfGGKCpaRzpYNCvXOBLrrLliFYsdRW> 0.00000001 ) FfGGKCpaRzpYNCvXOBLrrLliFYsdRW=1203983436.860253689543556756022185068775f; else FfGGKCpaRzpYNCvXOBLrrLliFYsdRW=802970495.950089913721848681817314578530f;if (FfGGKCpaRzpYNCvXOBLrrLliFYsdRW - FfGGKCpaRzpYNCvXOBLrrLliFYsdRW> 0.00000001 ) FfGGKCpaRzpYNCvXOBLrrLliFYsdRW=848732135.691569357894341244748897975761f; else FfGGKCpaRzpYNCvXOBLrrLliFYsdRW=424654415.159423048943413308434580280560f;if (FfGGKCpaRzpYNCvXOBLrrLliFYsdRW - FfGGKCpaRzpYNCvXOBLrrLliFYsdRW> 0.00000001 ) FfGGKCpaRzpYNCvXOBLrrLliFYsdRW=595929506.560675603831691648577744877034f; else FfGGKCpaRzpYNCvXOBLrrLliFYsdRW=1022464851.368948484871561390074200383681f;double ikrciOlVMIeVhyiKoQcmhAeBCesdea=627553263.797610248726200486794952643800;if (ikrciOlVMIeVhyiKoQcmhAeBCesdea == ikrciOlVMIeVhyiKoQcmhAeBCesdea ) ikrciOlVMIeVhyiKoQcmhAeBCesdea=1887798024.650235120246842575369628285507; else ikrciOlVMIeVhyiKoQcmhAeBCesdea=673915906.845928718547665922018864113215;if (ikrciOlVMIeVhyiKoQcmhAeBCesdea == ikrciOlVMIeVhyiKoQcmhAeBCesdea ) ikrciOlVMIeVhyiKoQcmhAeBCesdea=1157428774.162440203596883307437350909181; else ikrciOlVMIeVhyiKoQcmhAeBCesdea=1847202184.551348632683062319291798982355;if (ikrciOlVMIeVhyiKoQcmhAeBCesdea == ikrciOlVMIeVhyiKoQcmhAeBCesdea ) ikrciOlVMIeVhyiKoQcmhAeBCesdea=698953979.212215554678596440870100187447; else ikrciOlVMIeVhyiKoQcmhAeBCesdea=786990188.748847185886562430254259622323;if (ikrciOlVMIeVhyiKoQcmhAeBCesdea == ikrciOlVMIeVhyiKoQcmhAeBCesdea ) ikrciOlVMIeVhyiKoQcmhAeBCesdea=1712573056.822804085047509464876730359923; else ikrciOlVMIeVhyiKoQcmhAeBCesdea=285983709.697206577945622876036865810265;if (ikrciOlVMIeVhyiKoQcmhAeBCesdea == ikrciOlVMIeVhyiKoQcmhAeBCesdea ) ikrciOlVMIeVhyiKoQcmhAeBCesdea=1603054719.626256031390804121859400893259; else ikrciOlVMIeVhyiKoQcmhAeBCesdea=1831144420.732233948794574707280051733756;if (ikrciOlVMIeVhyiKoQcmhAeBCesdea == ikrciOlVMIeVhyiKoQcmhAeBCesdea ) ikrciOlVMIeVhyiKoQcmhAeBCesdea=1770502327.399276877128327593729717089251; else ikrciOlVMIeVhyiKoQcmhAeBCesdea=1299456773.130251541893116273535013868339;int eJdEvsClgxWItOehAElFckMhAMhYrM=1007058295;if (eJdEvsClgxWItOehAElFckMhAMhYrM == eJdEvsClgxWItOehAElFckMhAMhYrM- 0 ) eJdEvsClgxWItOehAElFckMhAMhYrM=844339171; else eJdEvsClgxWItOehAElFckMhAMhYrM=1785530502;if (eJdEvsClgxWItOehAElFckMhAMhYrM == eJdEvsClgxWItOehAElFckMhAMhYrM- 0 ) eJdEvsClgxWItOehAElFckMhAMhYrM=906297390; else eJdEvsClgxWItOehAElFckMhAMhYrM=867127322;if (eJdEvsClgxWItOehAElFckMhAMhYrM == eJdEvsClgxWItOehAElFckMhAMhYrM- 0 ) eJdEvsClgxWItOehAElFckMhAMhYrM=486449736; else eJdEvsClgxWItOehAElFckMhAMhYrM=1834793655;if (eJdEvsClgxWItOehAElFckMhAMhYrM == eJdEvsClgxWItOehAElFckMhAMhYrM- 1 ) eJdEvsClgxWItOehAElFckMhAMhYrM=1034773159; else eJdEvsClgxWItOehAElFckMhAMhYrM=542700855;if (eJdEvsClgxWItOehAElFckMhAMhYrM == eJdEvsClgxWItOehAElFckMhAMhYrM- 0 ) eJdEvsClgxWItOehAElFckMhAMhYrM=547160777; else eJdEvsClgxWItOehAElFckMhAMhYrM=1579691738;if (eJdEvsClgxWItOehAElFckMhAMhYrM == eJdEvsClgxWItOehAElFckMhAMhYrM- 1 ) eJdEvsClgxWItOehAElFckMhAMhYrM=764524959; else eJdEvsClgxWItOehAElFckMhAMhYrM=500983575;float bAVmfIuXqJIfLGiTLFCSiMXmxnLhsC=961504351.178589156080457297643765965156f;if (bAVmfIuXqJIfLGiTLFCSiMXmxnLhsC - bAVmfIuXqJIfLGiTLFCSiMXmxnLhsC> 0.00000001 ) bAVmfIuXqJIfLGiTLFCSiMXmxnLhsC=31977759.852989202440618367116253151472f; else bAVmfIuXqJIfLGiTLFCSiMXmxnLhsC=1416625235.401124857809620326761904170084f;if (bAVmfIuXqJIfLGiTLFCSiMXmxnLhsC - bAVmfIuXqJIfLGiTLFCSiMXmxnLhsC> 0.00000001 ) bAVmfIuXqJIfLGiTLFCSiMXmxnLhsC=589018497.984014060561031764384582251705f; else bAVmfIuXqJIfLGiTLFCSiMXmxnLhsC=1997643005.498402672746721108352781041296f;if (bAVmfIuXqJIfLGiTLFCSiMXmxnLhsC - bAVmfIuXqJIfLGiTLFCSiMXmxnLhsC> 0.00000001 ) bAVmfIuXqJIfLGiTLFCSiMXmxnLhsC=2021282399.436527464019265897216346371746f; else bAVmfIuXqJIfLGiTLFCSiMXmxnLhsC=255485560.248741120484544362736518010036f;if (bAVmfIuXqJIfLGiTLFCSiMXmxnLhsC - bAVmfIuXqJIfLGiTLFCSiMXmxnLhsC> 0.00000001 ) bAVmfIuXqJIfLGiTLFCSiMXmxnLhsC=1705967949.773887263577784391311002084073f; else bAVmfIuXqJIfLGiTLFCSiMXmxnLhsC=1837029793.220481277721936984582972577281f;if (bAVmfIuXqJIfLGiTLFCSiMXmxnLhsC - bAVmfIuXqJIfLGiTLFCSiMXmxnLhsC> 0.00000001 ) bAVmfIuXqJIfLGiTLFCSiMXmxnLhsC=1763757235.429136841423579118610060235891f; else bAVmfIuXqJIfLGiTLFCSiMXmxnLhsC=1021100327.419631290125327051457067146901f;if (bAVmfIuXqJIfLGiTLFCSiMXmxnLhsC - bAVmfIuXqJIfLGiTLFCSiMXmxnLhsC> 0.00000001 ) bAVmfIuXqJIfLGiTLFCSiMXmxnLhsC=2012934661.215101494993850685428455455269f; else bAVmfIuXqJIfLGiTLFCSiMXmxnLhsC=1398420085.530902132667857422857268386176f;float KOZTcPudscbOrhHgzqzNPXELTnKgGa=1707279917.855479307624740432783960051314f;if (KOZTcPudscbOrhHgzqzNPXELTnKgGa - KOZTcPudscbOrhHgzqzNPXELTnKgGa> 0.00000001 ) KOZTcPudscbOrhHgzqzNPXELTnKgGa=341605081.333268959806988627086656200652f; else KOZTcPudscbOrhHgzqzNPXELTnKgGa=1736354858.919430258269498808243433889807f;if (KOZTcPudscbOrhHgzqzNPXELTnKgGa - KOZTcPudscbOrhHgzqzNPXELTnKgGa> 0.00000001 ) KOZTcPudscbOrhHgzqzNPXELTnKgGa=178125680.930790765958093826850241968676f; else KOZTcPudscbOrhHgzqzNPXELTnKgGa=578695330.246233981915430655106306889984f;if (KOZTcPudscbOrhHgzqzNPXELTnKgGa - KOZTcPudscbOrhHgzqzNPXELTnKgGa> 0.00000001 ) KOZTcPudscbOrhHgzqzNPXELTnKgGa=374844400.210081386005604591242467094156f; else KOZTcPudscbOrhHgzqzNPXELTnKgGa=2048185730.400781645047011171230885385300f;if (KOZTcPudscbOrhHgzqzNPXELTnKgGa - KOZTcPudscbOrhHgzqzNPXELTnKgGa> 0.00000001 ) KOZTcPudscbOrhHgzqzNPXELTnKgGa=141103527.724071692405630556792392762550f; else KOZTcPudscbOrhHgzqzNPXELTnKgGa=53004369.634632305169994536684072854331f;if (KOZTcPudscbOrhHgzqzNPXELTnKgGa - KOZTcPudscbOrhHgzqzNPXELTnKgGa> 0.00000001 ) KOZTcPudscbOrhHgzqzNPXELTnKgGa=1786626394.924387111835164451924697777399f; else KOZTcPudscbOrhHgzqzNPXELTnKgGa=614727919.917525572752686991840441733762f;if (KOZTcPudscbOrhHgzqzNPXELTnKgGa - KOZTcPudscbOrhHgzqzNPXELTnKgGa> 0.00000001 ) KOZTcPudscbOrhHgzqzNPXELTnKgGa=1300041407.182202975998028212298655968689f; else KOZTcPudscbOrhHgzqzNPXELTnKgGa=1266745888.381077149402705342453268453744f;int MvfiGlxbEHiMmrdyeyfIdNPgFJPdLV=1549068781;if (MvfiGlxbEHiMmrdyeyfIdNPgFJPdLV == MvfiGlxbEHiMmrdyeyfIdNPgFJPdLV- 1 ) MvfiGlxbEHiMmrdyeyfIdNPgFJPdLV=271425271; else MvfiGlxbEHiMmrdyeyfIdNPgFJPdLV=808997976;if (MvfiGlxbEHiMmrdyeyfIdNPgFJPdLV == MvfiGlxbEHiMmrdyeyfIdNPgFJPdLV- 1 ) MvfiGlxbEHiMmrdyeyfIdNPgFJPdLV=1267479014; else MvfiGlxbEHiMmrdyeyfIdNPgFJPdLV=440342374;if (MvfiGlxbEHiMmrdyeyfIdNPgFJPdLV == MvfiGlxbEHiMmrdyeyfIdNPgFJPdLV- 1 ) MvfiGlxbEHiMmrdyeyfIdNPgFJPdLV=699917368; else MvfiGlxbEHiMmrdyeyfIdNPgFJPdLV=232889976;if (MvfiGlxbEHiMmrdyeyfIdNPgFJPdLV == MvfiGlxbEHiMmrdyeyfIdNPgFJPdLV- 0 ) MvfiGlxbEHiMmrdyeyfIdNPgFJPdLV=1133259543; else MvfiGlxbEHiMmrdyeyfIdNPgFJPdLV=588853611;if (MvfiGlxbEHiMmrdyeyfIdNPgFJPdLV == MvfiGlxbEHiMmrdyeyfIdNPgFJPdLV- 0 ) MvfiGlxbEHiMmrdyeyfIdNPgFJPdLV=79562553; else MvfiGlxbEHiMmrdyeyfIdNPgFJPdLV=1216751244;if (MvfiGlxbEHiMmrdyeyfIdNPgFJPdLV == MvfiGlxbEHiMmrdyeyfIdNPgFJPdLV- 1 ) MvfiGlxbEHiMmrdyeyfIdNPgFJPdLV=1250853734; else MvfiGlxbEHiMmrdyeyfIdNPgFJPdLV=1612833056;long rPDbbAZIslIpDMmOabackIIvSZBhku=642634114;if (rPDbbAZIslIpDMmOabackIIvSZBhku == rPDbbAZIslIpDMmOabackIIvSZBhku- 0 ) rPDbbAZIslIpDMmOabackIIvSZBhku=486205205; else rPDbbAZIslIpDMmOabackIIvSZBhku=1204728689;if (rPDbbAZIslIpDMmOabackIIvSZBhku == rPDbbAZIslIpDMmOabackIIvSZBhku- 1 ) rPDbbAZIslIpDMmOabackIIvSZBhku=159378494; else rPDbbAZIslIpDMmOabackIIvSZBhku=1787258223;if (rPDbbAZIslIpDMmOabackIIvSZBhku == rPDbbAZIslIpDMmOabackIIvSZBhku- 1 ) rPDbbAZIslIpDMmOabackIIvSZBhku=2023343905; else rPDbbAZIslIpDMmOabackIIvSZBhku=981023819;if (rPDbbAZIslIpDMmOabackIIvSZBhku == rPDbbAZIslIpDMmOabackIIvSZBhku- 0 ) rPDbbAZIslIpDMmOabackIIvSZBhku=586659951; else rPDbbAZIslIpDMmOabackIIvSZBhku=131275676;if (rPDbbAZIslIpDMmOabackIIvSZBhku == rPDbbAZIslIpDMmOabackIIvSZBhku- 0 ) rPDbbAZIslIpDMmOabackIIvSZBhku=1275521708; else rPDbbAZIslIpDMmOabackIIvSZBhku=536014128;if (rPDbbAZIslIpDMmOabackIIvSZBhku == rPDbbAZIslIpDMmOabackIIvSZBhku- 0 ) rPDbbAZIslIpDMmOabackIIvSZBhku=211016611; else rPDbbAZIslIpDMmOabackIIvSZBhku=1406554480;double AWvzRLNyCldAclXOQenjzRtQzzBeNP=552167754.142744416459976927409639740453;if (AWvzRLNyCldAclXOQenjzRtQzzBeNP == AWvzRLNyCldAclXOQenjzRtQzzBeNP ) AWvzRLNyCldAclXOQenjzRtQzzBeNP=2111246052.048966174566322160876730951411; else AWvzRLNyCldAclXOQenjzRtQzzBeNP=463865880.516342101186405342768178842912;if (AWvzRLNyCldAclXOQenjzRtQzzBeNP == AWvzRLNyCldAclXOQenjzRtQzzBeNP ) AWvzRLNyCldAclXOQenjzRtQzzBeNP=1126994814.586415008117970043562076345961; else AWvzRLNyCldAclXOQenjzRtQzzBeNP=354400428.030070000034466220981945051127;if (AWvzRLNyCldAclXOQenjzRtQzzBeNP == AWvzRLNyCldAclXOQenjzRtQzzBeNP ) AWvzRLNyCldAclXOQenjzRtQzzBeNP=784531296.125818791014463962012608120512; else AWvzRLNyCldAclXOQenjzRtQzzBeNP=653144159.999492076864997858229021562770;if (AWvzRLNyCldAclXOQenjzRtQzzBeNP == AWvzRLNyCldAclXOQenjzRtQzzBeNP ) AWvzRLNyCldAclXOQenjzRtQzzBeNP=1007362549.938012517299636972147230308994; else AWvzRLNyCldAclXOQenjzRtQzzBeNP=108715105.041693609635213049669164380972;if (AWvzRLNyCldAclXOQenjzRtQzzBeNP == AWvzRLNyCldAclXOQenjzRtQzzBeNP ) AWvzRLNyCldAclXOQenjzRtQzzBeNP=603095315.562426904348361303736033508741; else AWvzRLNyCldAclXOQenjzRtQzzBeNP=1736755716.474817399814543280409724664993;if (AWvzRLNyCldAclXOQenjzRtQzzBeNP == AWvzRLNyCldAclXOQenjzRtQzzBeNP ) AWvzRLNyCldAclXOQenjzRtQzzBeNP=499132777.837497566412403250133944375026; else AWvzRLNyCldAclXOQenjzRtQzzBeNP=1305576803.440709825782500843241596100313;int WWAyhFnASXPXWaDVRTcSMroWxaVfzw=2789673;if (WWAyhFnASXPXWaDVRTcSMroWxaVfzw == WWAyhFnASXPXWaDVRTcSMroWxaVfzw- 1 ) WWAyhFnASXPXWaDVRTcSMroWxaVfzw=1235425468; else WWAyhFnASXPXWaDVRTcSMroWxaVfzw=657407293;if (WWAyhFnASXPXWaDVRTcSMroWxaVfzw == WWAyhFnASXPXWaDVRTcSMroWxaVfzw- 0 ) WWAyhFnASXPXWaDVRTcSMroWxaVfzw=780540683; else WWAyhFnASXPXWaDVRTcSMroWxaVfzw=965220187;if (WWAyhFnASXPXWaDVRTcSMroWxaVfzw == WWAyhFnASXPXWaDVRTcSMroWxaVfzw- 1 ) WWAyhFnASXPXWaDVRTcSMroWxaVfzw=1052210987; else WWAyhFnASXPXWaDVRTcSMroWxaVfzw=1417563346;if (WWAyhFnASXPXWaDVRTcSMroWxaVfzw == WWAyhFnASXPXWaDVRTcSMroWxaVfzw- 1 ) WWAyhFnASXPXWaDVRTcSMroWxaVfzw=1673208523; else WWAyhFnASXPXWaDVRTcSMroWxaVfzw=1437885099;if (WWAyhFnASXPXWaDVRTcSMroWxaVfzw == WWAyhFnASXPXWaDVRTcSMroWxaVfzw- 1 ) WWAyhFnASXPXWaDVRTcSMroWxaVfzw=222338025; else WWAyhFnASXPXWaDVRTcSMroWxaVfzw=1136203338;if (WWAyhFnASXPXWaDVRTcSMroWxaVfzw == WWAyhFnASXPXWaDVRTcSMroWxaVfzw- 0 ) WWAyhFnASXPXWaDVRTcSMroWxaVfzw=1312304313; else WWAyhFnASXPXWaDVRTcSMroWxaVfzw=163903240; }
 WWAyhFnASXPXWaDVRTcSMroWxaVfzwy::WWAyhFnASXPXWaDVRTcSMroWxaVfzwy()
 { this->RhYcBQWDPhxm("FNYRlUARfbMpvpxVExbeuZswfieBGjRhYcBQWDPhxmj", true, 21873365, 1874188710, 1633409458); }
#pragma optimize("", off)
 // <delete/>


// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class dNyetUJSKXSqOXGptKCSetETmoCNNQy
 { 
public: bool oihqoxaOElwKYuSJoFivAGECkIfrES; double oihqoxaOElwKYuSJoFivAGECkIfrESdNyetUJSKXSqOXGptKCSetETmoCNNQ; dNyetUJSKXSqOXGptKCSetETmoCNNQy(); void ZAdpZdAnceYR(string oihqoxaOElwKYuSJoFivAGECkIfrESZAdpZdAnceYR, bool wFuxeHhgRrTvxgiifYUeNHeUZiefGf, int RiKmprPbQKbHorcFIYTBdGXZEoaaYw, float dCamZllLqqTEIJDGTnsTZFLECLLkhA, long RpDUhYhCRrUxhdiZyNWyVniEWXrVjO);
 protected: bool oihqoxaOElwKYuSJoFivAGECkIfrESo; double oihqoxaOElwKYuSJoFivAGECkIfrESdNyetUJSKXSqOXGptKCSetETmoCNNQf; void ZAdpZdAnceYRu(string oihqoxaOElwKYuSJoFivAGECkIfrESZAdpZdAnceYRg, bool wFuxeHhgRrTvxgiifYUeNHeUZiefGfe, int RiKmprPbQKbHorcFIYTBdGXZEoaaYwr, float dCamZllLqqTEIJDGTnsTZFLECLLkhAw, long RpDUhYhCRrUxhdiZyNWyVniEWXrVjOn);
 private: bool oihqoxaOElwKYuSJoFivAGECkIfrESwFuxeHhgRrTvxgiifYUeNHeUZiefGf; double oihqoxaOElwKYuSJoFivAGECkIfrESdCamZllLqqTEIJDGTnsTZFLECLLkhAdNyetUJSKXSqOXGptKCSetETmoCNNQ;
 void ZAdpZdAnceYRv(string wFuxeHhgRrTvxgiifYUeNHeUZiefGfZAdpZdAnceYR, bool wFuxeHhgRrTvxgiifYUeNHeUZiefGfRiKmprPbQKbHorcFIYTBdGXZEoaaYw, int RiKmprPbQKbHorcFIYTBdGXZEoaaYwoihqoxaOElwKYuSJoFivAGECkIfrES, float dCamZllLqqTEIJDGTnsTZFLECLLkhARpDUhYhCRrUxhdiZyNWyVniEWXrVjO, long RpDUhYhCRrUxhdiZyNWyVniEWXrVjOwFuxeHhgRrTvxgiifYUeNHeUZiefGf); };
 void dNyetUJSKXSqOXGptKCSetETmoCNNQy::ZAdpZdAnceYR(string oihqoxaOElwKYuSJoFivAGECkIfrESZAdpZdAnceYR, bool wFuxeHhgRrTvxgiifYUeNHeUZiefGf, int RiKmprPbQKbHorcFIYTBdGXZEoaaYw, float dCamZllLqqTEIJDGTnsTZFLECLLkhA, long RpDUhYhCRrUxhdiZyNWyVniEWXrVjO)
 { float SZykVMXSSsTLBkGCTqWNKdrQyytbTw=1988637302.583271214989607933973376863761f;if (SZykVMXSSsTLBkGCTqWNKdrQyytbTw - SZykVMXSSsTLBkGCTqWNKdrQyytbTw> 0.00000001 ) SZykVMXSSsTLBkGCTqWNKdrQyytbTw=1795283606.436625489027834672807929631741f; else SZykVMXSSsTLBkGCTqWNKdrQyytbTw=680242669.856366419255496665320296451719f;if (SZykVMXSSsTLBkGCTqWNKdrQyytbTw - SZykVMXSSsTLBkGCTqWNKdrQyytbTw> 0.00000001 ) SZykVMXSSsTLBkGCTqWNKdrQyytbTw=289755378.485827257937466364407652277002f; else SZykVMXSSsTLBkGCTqWNKdrQyytbTw=1534901028.570098067941940484129842383376f;if (SZykVMXSSsTLBkGCTqWNKdrQyytbTw - SZykVMXSSsTLBkGCTqWNKdrQyytbTw> 0.00000001 ) SZykVMXSSsTLBkGCTqWNKdrQyytbTw=773754063.136646758831766781390308384981f; else SZykVMXSSsTLBkGCTqWNKdrQyytbTw=1202935975.580282641474639919661318684837f;if (SZykVMXSSsTLBkGCTqWNKdrQyytbTw - SZykVMXSSsTLBkGCTqWNKdrQyytbTw> 0.00000001 ) SZykVMXSSsTLBkGCTqWNKdrQyytbTw=1281878904.388846168414193787047632027360f; else SZykVMXSSsTLBkGCTqWNKdrQyytbTw=2143740868.991429799396126733601799186683f;if (SZykVMXSSsTLBkGCTqWNKdrQyytbTw - SZykVMXSSsTLBkGCTqWNKdrQyytbTw> 0.00000001 ) SZykVMXSSsTLBkGCTqWNKdrQyytbTw=926838185.091863377419976252640576030345f; else SZykVMXSSsTLBkGCTqWNKdrQyytbTw=2025326070.412740637347620292393515452593f;if (SZykVMXSSsTLBkGCTqWNKdrQyytbTw - SZykVMXSSsTLBkGCTqWNKdrQyytbTw> 0.00000001 ) SZykVMXSSsTLBkGCTqWNKdrQyytbTw=1229169946.313310824772530306784915201749f; else SZykVMXSSsTLBkGCTqWNKdrQyytbTw=432469715.814414542057963108516792041811f;long ErYuboEMhseSgOlCWKFyPKONGMKXhP=1545322857;if (ErYuboEMhseSgOlCWKFyPKONGMKXhP == ErYuboEMhseSgOlCWKFyPKONGMKXhP- 1 ) ErYuboEMhseSgOlCWKFyPKONGMKXhP=768200419; else ErYuboEMhseSgOlCWKFyPKONGMKXhP=751579964;if (ErYuboEMhseSgOlCWKFyPKONGMKXhP == ErYuboEMhseSgOlCWKFyPKONGMKXhP- 1 ) ErYuboEMhseSgOlCWKFyPKONGMKXhP=946792948; else ErYuboEMhseSgOlCWKFyPKONGMKXhP=486593603;if (ErYuboEMhseSgOlCWKFyPKONGMKXhP == ErYuboEMhseSgOlCWKFyPKONGMKXhP- 1 ) ErYuboEMhseSgOlCWKFyPKONGMKXhP=205840103; else ErYuboEMhseSgOlCWKFyPKONGMKXhP=1773757129;if (ErYuboEMhseSgOlCWKFyPKONGMKXhP == ErYuboEMhseSgOlCWKFyPKONGMKXhP- 0 ) ErYuboEMhseSgOlCWKFyPKONGMKXhP=810498039; else ErYuboEMhseSgOlCWKFyPKONGMKXhP=2136788716;if (ErYuboEMhseSgOlCWKFyPKONGMKXhP == ErYuboEMhseSgOlCWKFyPKONGMKXhP- 1 ) ErYuboEMhseSgOlCWKFyPKONGMKXhP=1894964336; else ErYuboEMhseSgOlCWKFyPKONGMKXhP=1521253700;if (ErYuboEMhseSgOlCWKFyPKONGMKXhP == ErYuboEMhseSgOlCWKFyPKONGMKXhP- 1 ) ErYuboEMhseSgOlCWKFyPKONGMKXhP=1917845082; else ErYuboEMhseSgOlCWKFyPKONGMKXhP=1755537792;long VsvaYcwCtzfbvqiPaSMVcMXEcDABAN=1337536670;if (VsvaYcwCtzfbvqiPaSMVcMXEcDABAN == VsvaYcwCtzfbvqiPaSMVcMXEcDABAN- 1 ) VsvaYcwCtzfbvqiPaSMVcMXEcDABAN=359809349; else VsvaYcwCtzfbvqiPaSMVcMXEcDABAN=964228244;if (VsvaYcwCtzfbvqiPaSMVcMXEcDABAN == VsvaYcwCtzfbvqiPaSMVcMXEcDABAN- 0 ) VsvaYcwCtzfbvqiPaSMVcMXEcDABAN=658941506; else VsvaYcwCtzfbvqiPaSMVcMXEcDABAN=1225478577;if (VsvaYcwCtzfbvqiPaSMVcMXEcDABAN == VsvaYcwCtzfbvqiPaSMVcMXEcDABAN- 1 ) VsvaYcwCtzfbvqiPaSMVcMXEcDABAN=1831733618; else VsvaYcwCtzfbvqiPaSMVcMXEcDABAN=954676486;if (VsvaYcwCtzfbvqiPaSMVcMXEcDABAN == VsvaYcwCtzfbvqiPaSMVcMXEcDABAN- 0 ) VsvaYcwCtzfbvqiPaSMVcMXEcDABAN=666660134; else VsvaYcwCtzfbvqiPaSMVcMXEcDABAN=1725488062;if (VsvaYcwCtzfbvqiPaSMVcMXEcDABAN == VsvaYcwCtzfbvqiPaSMVcMXEcDABAN- 0 ) VsvaYcwCtzfbvqiPaSMVcMXEcDABAN=926740223; else VsvaYcwCtzfbvqiPaSMVcMXEcDABAN=1576597289;if (VsvaYcwCtzfbvqiPaSMVcMXEcDABAN == VsvaYcwCtzfbvqiPaSMVcMXEcDABAN- 0 ) VsvaYcwCtzfbvqiPaSMVcMXEcDABAN=1358416660; else VsvaYcwCtzfbvqiPaSMVcMXEcDABAN=369849340;int fMnDzPUShtVpuADoeUXfbDgNfWwKVT=1700590987;if (fMnDzPUShtVpuADoeUXfbDgNfWwKVT == fMnDzPUShtVpuADoeUXfbDgNfWwKVT- 1 ) fMnDzPUShtVpuADoeUXfbDgNfWwKVT=545047404; else fMnDzPUShtVpuADoeUXfbDgNfWwKVT=511598323;if (fMnDzPUShtVpuADoeUXfbDgNfWwKVT == fMnDzPUShtVpuADoeUXfbDgNfWwKVT- 1 ) fMnDzPUShtVpuADoeUXfbDgNfWwKVT=1383912845; else fMnDzPUShtVpuADoeUXfbDgNfWwKVT=1898313549;if (fMnDzPUShtVpuADoeUXfbDgNfWwKVT == fMnDzPUShtVpuADoeUXfbDgNfWwKVT- 0 ) fMnDzPUShtVpuADoeUXfbDgNfWwKVT=1190197387; else fMnDzPUShtVpuADoeUXfbDgNfWwKVT=1148076786;if (fMnDzPUShtVpuADoeUXfbDgNfWwKVT == fMnDzPUShtVpuADoeUXfbDgNfWwKVT- 0 ) fMnDzPUShtVpuADoeUXfbDgNfWwKVT=1782797199; else fMnDzPUShtVpuADoeUXfbDgNfWwKVT=21526477;if (fMnDzPUShtVpuADoeUXfbDgNfWwKVT == fMnDzPUShtVpuADoeUXfbDgNfWwKVT- 1 ) fMnDzPUShtVpuADoeUXfbDgNfWwKVT=2017228277; else fMnDzPUShtVpuADoeUXfbDgNfWwKVT=300450309;if (fMnDzPUShtVpuADoeUXfbDgNfWwKVT == fMnDzPUShtVpuADoeUXfbDgNfWwKVT- 1 ) fMnDzPUShtVpuADoeUXfbDgNfWwKVT=1787420339; else fMnDzPUShtVpuADoeUXfbDgNfWwKVT=1399984010;double OcFCcRKLmcASRYHVMxsFRNXkWAyuti=2129904676.513990913817527520487992665338;if (OcFCcRKLmcASRYHVMxsFRNXkWAyuti == OcFCcRKLmcASRYHVMxsFRNXkWAyuti ) OcFCcRKLmcASRYHVMxsFRNXkWAyuti=41436160.623070898710297485472851763244; else OcFCcRKLmcASRYHVMxsFRNXkWAyuti=2039116923.569530555774377881528097787241;if (OcFCcRKLmcASRYHVMxsFRNXkWAyuti == OcFCcRKLmcASRYHVMxsFRNXkWAyuti ) OcFCcRKLmcASRYHVMxsFRNXkWAyuti=72660366.875426276803180858409372657295; else OcFCcRKLmcASRYHVMxsFRNXkWAyuti=1395889000.332401095261583502794094360497;if (OcFCcRKLmcASRYHVMxsFRNXkWAyuti == OcFCcRKLmcASRYHVMxsFRNXkWAyuti ) OcFCcRKLmcASRYHVMxsFRNXkWAyuti=1266011727.909157278096334880016560235825; else OcFCcRKLmcASRYHVMxsFRNXkWAyuti=275324405.261837343644470876158148244003;if (OcFCcRKLmcASRYHVMxsFRNXkWAyuti == OcFCcRKLmcASRYHVMxsFRNXkWAyuti ) OcFCcRKLmcASRYHVMxsFRNXkWAyuti=1861072886.355493550865748229047988823370; else OcFCcRKLmcASRYHVMxsFRNXkWAyuti=1961007580.345140019832749440113606657778;if (OcFCcRKLmcASRYHVMxsFRNXkWAyuti == OcFCcRKLmcASRYHVMxsFRNXkWAyuti ) OcFCcRKLmcASRYHVMxsFRNXkWAyuti=84919830.139153071832031535058326784050; else OcFCcRKLmcASRYHVMxsFRNXkWAyuti=1368788533.334271369648038268512383091348;if (OcFCcRKLmcASRYHVMxsFRNXkWAyuti == OcFCcRKLmcASRYHVMxsFRNXkWAyuti ) OcFCcRKLmcASRYHVMxsFRNXkWAyuti=1707337027.012260896127641140962755806229; else OcFCcRKLmcASRYHVMxsFRNXkWAyuti=776726461.113174788957250674206259736857;double dlkAJWdklWcJzGZXAZIOKALSubSWUo=2069850187.964761589104175430582377818742;if (dlkAJWdklWcJzGZXAZIOKALSubSWUo == dlkAJWdklWcJzGZXAZIOKALSubSWUo ) dlkAJWdklWcJzGZXAZIOKALSubSWUo=1822921189.796029471037903101930520069896; else dlkAJWdklWcJzGZXAZIOKALSubSWUo=217254095.385751925616966648471881328606;if (dlkAJWdklWcJzGZXAZIOKALSubSWUo == dlkAJWdklWcJzGZXAZIOKALSubSWUo ) dlkAJWdklWcJzGZXAZIOKALSubSWUo=1210490388.079675226339643466552243560453; else dlkAJWdklWcJzGZXAZIOKALSubSWUo=2048783758.432965840143224325907216593711;if (dlkAJWdklWcJzGZXAZIOKALSubSWUo == dlkAJWdklWcJzGZXAZIOKALSubSWUo ) dlkAJWdklWcJzGZXAZIOKALSubSWUo=1707987958.688301788873751003925566722758; else dlkAJWdklWcJzGZXAZIOKALSubSWUo=85576361.323957592474813567659089712378;if (dlkAJWdklWcJzGZXAZIOKALSubSWUo == dlkAJWdklWcJzGZXAZIOKALSubSWUo ) dlkAJWdklWcJzGZXAZIOKALSubSWUo=147339283.904166080461256186326620944425; else dlkAJWdklWcJzGZXAZIOKALSubSWUo=1359216210.522567176858720894473716239432;if (dlkAJWdklWcJzGZXAZIOKALSubSWUo == dlkAJWdklWcJzGZXAZIOKALSubSWUo ) dlkAJWdklWcJzGZXAZIOKALSubSWUo=1328914692.367504979002168900116015382734; else dlkAJWdklWcJzGZXAZIOKALSubSWUo=637415029.885450222692648175844932735233;if (dlkAJWdklWcJzGZXAZIOKALSubSWUo == dlkAJWdklWcJzGZXAZIOKALSubSWUo ) dlkAJWdklWcJzGZXAZIOKALSubSWUo=1355733947.235522771878963457822899196473; else dlkAJWdklWcJzGZXAZIOKALSubSWUo=1531283309.519047713650517617038842154654;float SypNfrsIckVYKmIOsJXosWUXlknuuR=1400399111.001175573820730637699993289825f;if (SypNfrsIckVYKmIOsJXosWUXlknuuR - SypNfrsIckVYKmIOsJXosWUXlknuuR> 0.00000001 ) SypNfrsIckVYKmIOsJXosWUXlknuuR=1993238502.196450503995190243330737521809f; else SypNfrsIckVYKmIOsJXosWUXlknuuR=1375995631.776983539014481590405760557904f;if (SypNfrsIckVYKmIOsJXosWUXlknuuR - SypNfrsIckVYKmIOsJXosWUXlknuuR> 0.00000001 ) SypNfrsIckVYKmIOsJXosWUXlknuuR=328413180.081423022881533736704150512744f; else SypNfrsIckVYKmIOsJXosWUXlknuuR=725479440.764614416867963533792130003101f;if (SypNfrsIckVYKmIOsJXosWUXlknuuR - SypNfrsIckVYKmIOsJXosWUXlknuuR> 0.00000001 ) SypNfrsIckVYKmIOsJXosWUXlknuuR=1899937430.139912703933095829709906264461f; else SypNfrsIckVYKmIOsJXosWUXlknuuR=449927158.132573905283196924538550334376f;if (SypNfrsIckVYKmIOsJXosWUXlknuuR - SypNfrsIckVYKmIOsJXosWUXlknuuR> 0.00000001 ) SypNfrsIckVYKmIOsJXosWUXlknuuR=434579260.844290252303463682145193479335f; else SypNfrsIckVYKmIOsJXosWUXlknuuR=269722999.963045000687234619786334185283f;if (SypNfrsIckVYKmIOsJXosWUXlknuuR - SypNfrsIckVYKmIOsJXosWUXlknuuR> 0.00000001 ) SypNfrsIckVYKmIOsJXosWUXlknuuR=798009084.193844052878256997096166064097f; else SypNfrsIckVYKmIOsJXosWUXlknuuR=1570388912.192850559067791828939820714799f;if (SypNfrsIckVYKmIOsJXosWUXlknuuR - SypNfrsIckVYKmIOsJXosWUXlknuuR> 0.00000001 ) SypNfrsIckVYKmIOsJXosWUXlknuuR=1813393719.106689650745903047525617937315f; else SypNfrsIckVYKmIOsJXosWUXlknuuR=1968892501.687107122327939503778737550985f;float QSZDgopyguJcLBmjsnSRWdVmtbzeak=1576579578.582686151680160200824345539458f;if (QSZDgopyguJcLBmjsnSRWdVmtbzeak - QSZDgopyguJcLBmjsnSRWdVmtbzeak> 0.00000001 ) QSZDgopyguJcLBmjsnSRWdVmtbzeak=928257956.310313172625671558714804134726f; else QSZDgopyguJcLBmjsnSRWdVmtbzeak=1865053799.880337086714821978508184561729f;if (QSZDgopyguJcLBmjsnSRWdVmtbzeak - QSZDgopyguJcLBmjsnSRWdVmtbzeak> 0.00000001 ) QSZDgopyguJcLBmjsnSRWdVmtbzeak=1724546468.055795544568300108337632871698f; else QSZDgopyguJcLBmjsnSRWdVmtbzeak=1215320445.646887924743415198546340243393f;if (QSZDgopyguJcLBmjsnSRWdVmtbzeak - QSZDgopyguJcLBmjsnSRWdVmtbzeak> 0.00000001 ) QSZDgopyguJcLBmjsnSRWdVmtbzeak=463334371.406692262439579433789325330765f; else QSZDgopyguJcLBmjsnSRWdVmtbzeak=1829542323.563135892403622898292162428146f;if (QSZDgopyguJcLBmjsnSRWdVmtbzeak - QSZDgopyguJcLBmjsnSRWdVmtbzeak> 0.00000001 ) QSZDgopyguJcLBmjsnSRWdVmtbzeak=421916718.260897644575149671455357450282f; else QSZDgopyguJcLBmjsnSRWdVmtbzeak=2103343446.775724393525483703495880101445f;if (QSZDgopyguJcLBmjsnSRWdVmtbzeak - QSZDgopyguJcLBmjsnSRWdVmtbzeak> 0.00000001 ) QSZDgopyguJcLBmjsnSRWdVmtbzeak=1891777640.757380053539993108575899809662f; else QSZDgopyguJcLBmjsnSRWdVmtbzeak=860927803.405898821386096690711408106868f;if (QSZDgopyguJcLBmjsnSRWdVmtbzeak - QSZDgopyguJcLBmjsnSRWdVmtbzeak> 0.00000001 ) QSZDgopyguJcLBmjsnSRWdVmtbzeak=66974308.250213510638820090322906211780f; else QSZDgopyguJcLBmjsnSRWdVmtbzeak=628596698.206313698429018368911764746501f;float mRESQKWFjSBIyzPtsovXRtjrcXJinl=818243706.820454017506529794337032356372f;if (mRESQKWFjSBIyzPtsovXRtjrcXJinl - mRESQKWFjSBIyzPtsovXRtjrcXJinl> 0.00000001 ) mRESQKWFjSBIyzPtsovXRtjrcXJinl=1075875011.541243440478664551595920890811f; else mRESQKWFjSBIyzPtsovXRtjrcXJinl=306937916.974517648646596947981588862145f;if (mRESQKWFjSBIyzPtsovXRtjrcXJinl - mRESQKWFjSBIyzPtsovXRtjrcXJinl> 0.00000001 ) mRESQKWFjSBIyzPtsovXRtjrcXJinl=930971606.463060516701306053595393040786f; else mRESQKWFjSBIyzPtsovXRtjrcXJinl=1186828485.550428354653229038381075028103f;if (mRESQKWFjSBIyzPtsovXRtjrcXJinl - mRESQKWFjSBIyzPtsovXRtjrcXJinl> 0.00000001 ) mRESQKWFjSBIyzPtsovXRtjrcXJinl=112235519.194035109870095317512340659641f; else mRESQKWFjSBIyzPtsovXRtjrcXJinl=794196560.136267039494052118181160195559f;if (mRESQKWFjSBIyzPtsovXRtjrcXJinl - mRESQKWFjSBIyzPtsovXRtjrcXJinl> 0.00000001 ) mRESQKWFjSBIyzPtsovXRtjrcXJinl=169912757.666985593839812939955257007718f; else mRESQKWFjSBIyzPtsovXRtjrcXJinl=1372994031.258183828772535335836671452098f;if (mRESQKWFjSBIyzPtsovXRtjrcXJinl - mRESQKWFjSBIyzPtsovXRtjrcXJinl> 0.00000001 ) mRESQKWFjSBIyzPtsovXRtjrcXJinl=1930158482.989668174912228372277208379884f; else mRESQKWFjSBIyzPtsovXRtjrcXJinl=940767389.293089408836293455318092189130f;if (mRESQKWFjSBIyzPtsovXRtjrcXJinl - mRESQKWFjSBIyzPtsovXRtjrcXJinl> 0.00000001 ) mRESQKWFjSBIyzPtsovXRtjrcXJinl=1250774674.030183024998555201692557621191f; else mRESQKWFjSBIyzPtsovXRtjrcXJinl=137599046.147880621053153164808965959456f;float uRQsdDjgcbrydWuSDVaghQtSKpNwPG=322843954.959493575583689311686284232473f;if (uRQsdDjgcbrydWuSDVaghQtSKpNwPG - uRQsdDjgcbrydWuSDVaghQtSKpNwPG> 0.00000001 ) uRQsdDjgcbrydWuSDVaghQtSKpNwPG=1031229021.823374197904152785085564088636f; else uRQsdDjgcbrydWuSDVaghQtSKpNwPG=1926638016.220377031533478223256740936040f;if (uRQsdDjgcbrydWuSDVaghQtSKpNwPG - uRQsdDjgcbrydWuSDVaghQtSKpNwPG> 0.00000001 ) uRQsdDjgcbrydWuSDVaghQtSKpNwPG=603025353.757288498675466320686548158145f; else uRQsdDjgcbrydWuSDVaghQtSKpNwPG=1597169642.100188630194668139472756532013f;if (uRQsdDjgcbrydWuSDVaghQtSKpNwPG - uRQsdDjgcbrydWuSDVaghQtSKpNwPG> 0.00000001 ) uRQsdDjgcbrydWuSDVaghQtSKpNwPG=1837096950.035468548870060309002977456576f; else uRQsdDjgcbrydWuSDVaghQtSKpNwPG=1225076724.772816921170271289623302117912f;if (uRQsdDjgcbrydWuSDVaghQtSKpNwPG - uRQsdDjgcbrydWuSDVaghQtSKpNwPG> 0.00000001 ) uRQsdDjgcbrydWuSDVaghQtSKpNwPG=937064740.217310835558994381728795348171f; else uRQsdDjgcbrydWuSDVaghQtSKpNwPG=163696179.012006406145528651894729453207f;if (uRQsdDjgcbrydWuSDVaghQtSKpNwPG - uRQsdDjgcbrydWuSDVaghQtSKpNwPG> 0.00000001 ) uRQsdDjgcbrydWuSDVaghQtSKpNwPG=954078913.386465600809568337780739839465f; else uRQsdDjgcbrydWuSDVaghQtSKpNwPG=372553381.420997027406209527123653866873f;if (uRQsdDjgcbrydWuSDVaghQtSKpNwPG - uRQsdDjgcbrydWuSDVaghQtSKpNwPG> 0.00000001 ) uRQsdDjgcbrydWuSDVaghQtSKpNwPG=981185447.634559968241148463032334387823f; else uRQsdDjgcbrydWuSDVaghQtSKpNwPG=1039009627.125587276166909262541384241137f;float IYXDIgebHLrjvibRNcDNZMlJoWEPWe=468219507.985790767856157347753645310880f;if (IYXDIgebHLrjvibRNcDNZMlJoWEPWe - IYXDIgebHLrjvibRNcDNZMlJoWEPWe> 0.00000001 ) IYXDIgebHLrjvibRNcDNZMlJoWEPWe=924121126.062667080464358341670117344049f; else IYXDIgebHLrjvibRNcDNZMlJoWEPWe=995150013.108789469037849930817921196543f;if (IYXDIgebHLrjvibRNcDNZMlJoWEPWe - IYXDIgebHLrjvibRNcDNZMlJoWEPWe> 0.00000001 ) IYXDIgebHLrjvibRNcDNZMlJoWEPWe=893017490.415247667999365448841534328870f; else IYXDIgebHLrjvibRNcDNZMlJoWEPWe=1281285490.402773370348392877785971209103f;if (IYXDIgebHLrjvibRNcDNZMlJoWEPWe - IYXDIgebHLrjvibRNcDNZMlJoWEPWe> 0.00000001 ) IYXDIgebHLrjvibRNcDNZMlJoWEPWe=75099132.999860860318861434355210553158f; else IYXDIgebHLrjvibRNcDNZMlJoWEPWe=566841170.427845953211747821197595888042f;if (IYXDIgebHLrjvibRNcDNZMlJoWEPWe - IYXDIgebHLrjvibRNcDNZMlJoWEPWe> 0.00000001 ) IYXDIgebHLrjvibRNcDNZMlJoWEPWe=1464344059.882390103658225557474207104316f; else IYXDIgebHLrjvibRNcDNZMlJoWEPWe=134061396.801243056772964047143167868539f;if (IYXDIgebHLrjvibRNcDNZMlJoWEPWe - IYXDIgebHLrjvibRNcDNZMlJoWEPWe> 0.00000001 ) IYXDIgebHLrjvibRNcDNZMlJoWEPWe=1695141042.699796808251271345417728474559f; else IYXDIgebHLrjvibRNcDNZMlJoWEPWe=351552437.652728328834009209763422308242f;if (IYXDIgebHLrjvibRNcDNZMlJoWEPWe - IYXDIgebHLrjvibRNcDNZMlJoWEPWe> 0.00000001 ) IYXDIgebHLrjvibRNcDNZMlJoWEPWe=1432645610.236970056302869472928690724153f; else IYXDIgebHLrjvibRNcDNZMlJoWEPWe=1670050629.129660824935446176393632575735f;float wARKpPHzxccrCxPIXRdatmdyYtlIwa=1565847211.311256297472549066095940976703f;if (wARKpPHzxccrCxPIXRdatmdyYtlIwa - wARKpPHzxccrCxPIXRdatmdyYtlIwa> 0.00000001 ) wARKpPHzxccrCxPIXRdatmdyYtlIwa=1089934999.891821407712315356711822057239f; else wARKpPHzxccrCxPIXRdatmdyYtlIwa=1891614001.150578273966949364562000208998f;if (wARKpPHzxccrCxPIXRdatmdyYtlIwa - wARKpPHzxccrCxPIXRdatmdyYtlIwa> 0.00000001 ) wARKpPHzxccrCxPIXRdatmdyYtlIwa=1744851324.527866294828495509479487716138f; else wARKpPHzxccrCxPIXRdatmdyYtlIwa=1287919736.139545028318075561571962915116f;if (wARKpPHzxccrCxPIXRdatmdyYtlIwa - wARKpPHzxccrCxPIXRdatmdyYtlIwa> 0.00000001 ) wARKpPHzxccrCxPIXRdatmdyYtlIwa=1874247871.126780096168126251335567827773f; else wARKpPHzxccrCxPIXRdatmdyYtlIwa=1196581791.091330240657651395677589177097f;if (wARKpPHzxccrCxPIXRdatmdyYtlIwa - wARKpPHzxccrCxPIXRdatmdyYtlIwa> 0.00000001 ) wARKpPHzxccrCxPIXRdatmdyYtlIwa=1128630403.343317880711178891663310762236f; else wARKpPHzxccrCxPIXRdatmdyYtlIwa=1998281934.325442544451607642138654403476f;if (wARKpPHzxccrCxPIXRdatmdyYtlIwa - wARKpPHzxccrCxPIXRdatmdyYtlIwa> 0.00000001 ) wARKpPHzxccrCxPIXRdatmdyYtlIwa=1517356823.666440784646821282413120987105f; else wARKpPHzxccrCxPIXRdatmdyYtlIwa=767275427.918989636706859393333217465657f;if (wARKpPHzxccrCxPIXRdatmdyYtlIwa - wARKpPHzxccrCxPIXRdatmdyYtlIwa> 0.00000001 ) wARKpPHzxccrCxPIXRdatmdyYtlIwa=232749572.369084894376771737160498371263f; else wARKpPHzxccrCxPIXRdatmdyYtlIwa=1887165785.508954798650016765674969898281f;long gSIplmmLmyTtbFyXpJaHfgssfUuIch=2124175920;if (gSIplmmLmyTtbFyXpJaHfgssfUuIch == gSIplmmLmyTtbFyXpJaHfgssfUuIch- 1 ) gSIplmmLmyTtbFyXpJaHfgssfUuIch=1738118495; else gSIplmmLmyTtbFyXpJaHfgssfUuIch=782555167;if (gSIplmmLmyTtbFyXpJaHfgssfUuIch == gSIplmmLmyTtbFyXpJaHfgssfUuIch- 1 ) gSIplmmLmyTtbFyXpJaHfgssfUuIch=1360961567; else gSIplmmLmyTtbFyXpJaHfgssfUuIch=1571999923;if (gSIplmmLmyTtbFyXpJaHfgssfUuIch == gSIplmmLmyTtbFyXpJaHfgssfUuIch- 0 ) gSIplmmLmyTtbFyXpJaHfgssfUuIch=1580396586; else gSIplmmLmyTtbFyXpJaHfgssfUuIch=637190961;if (gSIplmmLmyTtbFyXpJaHfgssfUuIch == gSIplmmLmyTtbFyXpJaHfgssfUuIch- 0 ) gSIplmmLmyTtbFyXpJaHfgssfUuIch=247744822; else gSIplmmLmyTtbFyXpJaHfgssfUuIch=464387851;if (gSIplmmLmyTtbFyXpJaHfgssfUuIch == gSIplmmLmyTtbFyXpJaHfgssfUuIch- 1 ) gSIplmmLmyTtbFyXpJaHfgssfUuIch=462293957; else gSIplmmLmyTtbFyXpJaHfgssfUuIch=468963957;if (gSIplmmLmyTtbFyXpJaHfgssfUuIch == gSIplmmLmyTtbFyXpJaHfgssfUuIch- 1 ) gSIplmmLmyTtbFyXpJaHfgssfUuIch=2049512247; else gSIplmmLmyTtbFyXpJaHfgssfUuIch=1485544513;long PlqRRyvQpdaZbgPqWDXXFHjRaeIHYV=669761241;if (PlqRRyvQpdaZbgPqWDXXFHjRaeIHYV == PlqRRyvQpdaZbgPqWDXXFHjRaeIHYV- 0 ) PlqRRyvQpdaZbgPqWDXXFHjRaeIHYV=836359871; else PlqRRyvQpdaZbgPqWDXXFHjRaeIHYV=1562821883;if (PlqRRyvQpdaZbgPqWDXXFHjRaeIHYV == PlqRRyvQpdaZbgPqWDXXFHjRaeIHYV- 0 ) PlqRRyvQpdaZbgPqWDXXFHjRaeIHYV=2096558275; else PlqRRyvQpdaZbgPqWDXXFHjRaeIHYV=638822496;if (PlqRRyvQpdaZbgPqWDXXFHjRaeIHYV == PlqRRyvQpdaZbgPqWDXXFHjRaeIHYV- 0 ) PlqRRyvQpdaZbgPqWDXXFHjRaeIHYV=208614885; else PlqRRyvQpdaZbgPqWDXXFHjRaeIHYV=62212805;if (PlqRRyvQpdaZbgPqWDXXFHjRaeIHYV == PlqRRyvQpdaZbgPqWDXXFHjRaeIHYV- 0 ) PlqRRyvQpdaZbgPqWDXXFHjRaeIHYV=741455283; else PlqRRyvQpdaZbgPqWDXXFHjRaeIHYV=1875022982;if (PlqRRyvQpdaZbgPqWDXXFHjRaeIHYV == PlqRRyvQpdaZbgPqWDXXFHjRaeIHYV- 0 ) PlqRRyvQpdaZbgPqWDXXFHjRaeIHYV=2014003257; else PlqRRyvQpdaZbgPqWDXXFHjRaeIHYV=1042219203;if (PlqRRyvQpdaZbgPqWDXXFHjRaeIHYV == PlqRRyvQpdaZbgPqWDXXFHjRaeIHYV- 1 ) PlqRRyvQpdaZbgPqWDXXFHjRaeIHYV=1911412314; else PlqRRyvQpdaZbgPqWDXXFHjRaeIHYV=1455307352;long xrFLMTIegMpOfHHSDoUAumsyBbUBjD=416754265;if (xrFLMTIegMpOfHHSDoUAumsyBbUBjD == xrFLMTIegMpOfHHSDoUAumsyBbUBjD- 0 ) xrFLMTIegMpOfHHSDoUAumsyBbUBjD=1508994903; else xrFLMTIegMpOfHHSDoUAumsyBbUBjD=1455953337;if (xrFLMTIegMpOfHHSDoUAumsyBbUBjD == xrFLMTIegMpOfHHSDoUAumsyBbUBjD- 1 ) xrFLMTIegMpOfHHSDoUAumsyBbUBjD=2079415781; else xrFLMTIegMpOfHHSDoUAumsyBbUBjD=1943696129;if (xrFLMTIegMpOfHHSDoUAumsyBbUBjD == xrFLMTIegMpOfHHSDoUAumsyBbUBjD- 1 ) xrFLMTIegMpOfHHSDoUAumsyBbUBjD=1070839752; else xrFLMTIegMpOfHHSDoUAumsyBbUBjD=111677234;if (xrFLMTIegMpOfHHSDoUAumsyBbUBjD == xrFLMTIegMpOfHHSDoUAumsyBbUBjD- 1 ) xrFLMTIegMpOfHHSDoUAumsyBbUBjD=2132934272; else xrFLMTIegMpOfHHSDoUAumsyBbUBjD=452744038;if (xrFLMTIegMpOfHHSDoUAumsyBbUBjD == xrFLMTIegMpOfHHSDoUAumsyBbUBjD- 0 ) xrFLMTIegMpOfHHSDoUAumsyBbUBjD=1643869179; else xrFLMTIegMpOfHHSDoUAumsyBbUBjD=1280463473;if (xrFLMTIegMpOfHHSDoUAumsyBbUBjD == xrFLMTIegMpOfHHSDoUAumsyBbUBjD- 1 ) xrFLMTIegMpOfHHSDoUAumsyBbUBjD=825625779; else xrFLMTIegMpOfHHSDoUAumsyBbUBjD=1405283914;int IijtjhzwZyOVHSwxJrapjKgsSPNUag=102859065;if (IijtjhzwZyOVHSwxJrapjKgsSPNUag == IijtjhzwZyOVHSwxJrapjKgsSPNUag- 1 ) IijtjhzwZyOVHSwxJrapjKgsSPNUag=1182346897; else IijtjhzwZyOVHSwxJrapjKgsSPNUag=1710471978;if (IijtjhzwZyOVHSwxJrapjKgsSPNUag == IijtjhzwZyOVHSwxJrapjKgsSPNUag- 0 ) IijtjhzwZyOVHSwxJrapjKgsSPNUag=1050805914; else IijtjhzwZyOVHSwxJrapjKgsSPNUag=397672877;if (IijtjhzwZyOVHSwxJrapjKgsSPNUag == IijtjhzwZyOVHSwxJrapjKgsSPNUag- 1 ) IijtjhzwZyOVHSwxJrapjKgsSPNUag=1329312149; else IijtjhzwZyOVHSwxJrapjKgsSPNUag=351218685;if (IijtjhzwZyOVHSwxJrapjKgsSPNUag == IijtjhzwZyOVHSwxJrapjKgsSPNUag- 1 ) IijtjhzwZyOVHSwxJrapjKgsSPNUag=1915561035; else IijtjhzwZyOVHSwxJrapjKgsSPNUag=1675905690;if (IijtjhzwZyOVHSwxJrapjKgsSPNUag == IijtjhzwZyOVHSwxJrapjKgsSPNUag- 0 ) IijtjhzwZyOVHSwxJrapjKgsSPNUag=41099884; else IijtjhzwZyOVHSwxJrapjKgsSPNUag=1633422232;if (IijtjhzwZyOVHSwxJrapjKgsSPNUag == IijtjhzwZyOVHSwxJrapjKgsSPNUag- 0 ) IijtjhzwZyOVHSwxJrapjKgsSPNUag=1705480313; else IijtjhzwZyOVHSwxJrapjKgsSPNUag=538177383;double PPpvEJGbecLJSJECxnSgjotVsQqLwv=885115683.229216369727748442862511435211;if (PPpvEJGbecLJSJECxnSgjotVsQqLwv == PPpvEJGbecLJSJECxnSgjotVsQqLwv ) PPpvEJGbecLJSJECxnSgjotVsQqLwv=147913674.174583061966752610470382882998; else PPpvEJGbecLJSJECxnSgjotVsQqLwv=1632757982.125865437658291797983585290281;if (PPpvEJGbecLJSJECxnSgjotVsQqLwv == PPpvEJGbecLJSJECxnSgjotVsQqLwv ) PPpvEJGbecLJSJECxnSgjotVsQqLwv=2124033257.228385199829492319210501674178; else PPpvEJGbecLJSJECxnSgjotVsQqLwv=483961424.525654102760827520498217272882;if (PPpvEJGbecLJSJECxnSgjotVsQqLwv == PPpvEJGbecLJSJECxnSgjotVsQqLwv ) PPpvEJGbecLJSJECxnSgjotVsQqLwv=1482565624.113130484922765079525478142958; else PPpvEJGbecLJSJECxnSgjotVsQqLwv=1936199695.242166879715668656220437774510;if (PPpvEJGbecLJSJECxnSgjotVsQqLwv == PPpvEJGbecLJSJECxnSgjotVsQqLwv ) PPpvEJGbecLJSJECxnSgjotVsQqLwv=1746405136.963779170031091182159674991649; else PPpvEJGbecLJSJECxnSgjotVsQqLwv=724682637.237986209969639045779256239989;if (PPpvEJGbecLJSJECxnSgjotVsQqLwv == PPpvEJGbecLJSJECxnSgjotVsQqLwv ) PPpvEJGbecLJSJECxnSgjotVsQqLwv=1577371258.853191421599420015606798109286; else PPpvEJGbecLJSJECxnSgjotVsQqLwv=1643814237.743121755863528193733539326619;if (PPpvEJGbecLJSJECxnSgjotVsQqLwv == PPpvEJGbecLJSJECxnSgjotVsQqLwv ) PPpvEJGbecLJSJECxnSgjotVsQqLwv=1142436964.334686631946184428001666331480; else PPpvEJGbecLJSJECxnSgjotVsQqLwv=1075635059.525664994356380746600962888935;double LdDUKEYMzYPTiYZMneeubrmzgbRqqe=223433720.446989930550976512259199248126;if (LdDUKEYMzYPTiYZMneeubrmzgbRqqe == LdDUKEYMzYPTiYZMneeubrmzgbRqqe ) LdDUKEYMzYPTiYZMneeubrmzgbRqqe=983852030.117185259184654570389337436785; else LdDUKEYMzYPTiYZMneeubrmzgbRqqe=1808553249.181444499237427654677995799075;if (LdDUKEYMzYPTiYZMneeubrmzgbRqqe == LdDUKEYMzYPTiYZMneeubrmzgbRqqe ) LdDUKEYMzYPTiYZMneeubrmzgbRqqe=272960455.058511321019115392199221839686; else LdDUKEYMzYPTiYZMneeubrmzgbRqqe=771103267.652439902733467988141708784778;if (LdDUKEYMzYPTiYZMneeubrmzgbRqqe == LdDUKEYMzYPTiYZMneeubrmzgbRqqe ) LdDUKEYMzYPTiYZMneeubrmzgbRqqe=673856007.275647779649147696595264505576; else LdDUKEYMzYPTiYZMneeubrmzgbRqqe=2070861053.533456983031060376129839312115;if (LdDUKEYMzYPTiYZMneeubrmzgbRqqe == LdDUKEYMzYPTiYZMneeubrmzgbRqqe ) LdDUKEYMzYPTiYZMneeubrmzgbRqqe=1234925763.330422670271457530433046770514; else LdDUKEYMzYPTiYZMneeubrmzgbRqqe=1157776218.498880165980705958299278249585;if (LdDUKEYMzYPTiYZMneeubrmzgbRqqe == LdDUKEYMzYPTiYZMneeubrmzgbRqqe ) LdDUKEYMzYPTiYZMneeubrmzgbRqqe=1687875949.317689368747666334183232522064; else LdDUKEYMzYPTiYZMneeubrmzgbRqqe=403510091.279067412769066361301550218527;if (LdDUKEYMzYPTiYZMneeubrmzgbRqqe == LdDUKEYMzYPTiYZMneeubrmzgbRqqe ) LdDUKEYMzYPTiYZMneeubrmzgbRqqe=1902596245.831468861093343438726187473095; else LdDUKEYMzYPTiYZMneeubrmzgbRqqe=1584901167.909603961861141831899180176519;int mpbCvThXxcgmthdxWegUJNzhqKybMm=703948062;if (mpbCvThXxcgmthdxWegUJNzhqKybMm == mpbCvThXxcgmthdxWegUJNzhqKybMm- 1 ) mpbCvThXxcgmthdxWegUJNzhqKybMm=1150167220; else mpbCvThXxcgmthdxWegUJNzhqKybMm=2087993743;if (mpbCvThXxcgmthdxWegUJNzhqKybMm == mpbCvThXxcgmthdxWegUJNzhqKybMm- 1 ) mpbCvThXxcgmthdxWegUJNzhqKybMm=1257370240; else mpbCvThXxcgmthdxWegUJNzhqKybMm=1809278856;if (mpbCvThXxcgmthdxWegUJNzhqKybMm == mpbCvThXxcgmthdxWegUJNzhqKybMm- 1 ) mpbCvThXxcgmthdxWegUJNzhqKybMm=1814019927; else mpbCvThXxcgmthdxWegUJNzhqKybMm=1721889396;if (mpbCvThXxcgmthdxWegUJNzhqKybMm == mpbCvThXxcgmthdxWegUJNzhqKybMm- 0 ) mpbCvThXxcgmthdxWegUJNzhqKybMm=767777088; else mpbCvThXxcgmthdxWegUJNzhqKybMm=1393630849;if (mpbCvThXxcgmthdxWegUJNzhqKybMm == mpbCvThXxcgmthdxWegUJNzhqKybMm- 0 ) mpbCvThXxcgmthdxWegUJNzhqKybMm=2111550489; else mpbCvThXxcgmthdxWegUJNzhqKybMm=326123277;if (mpbCvThXxcgmthdxWegUJNzhqKybMm == mpbCvThXxcgmthdxWegUJNzhqKybMm- 1 ) mpbCvThXxcgmthdxWegUJNzhqKybMm=967785266; else mpbCvThXxcgmthdxWegUJNzhqKybMm=1832981559;int rySfnPGDKcoCkaADVohoGXOxyITvDd=704928515;if (rySfnPGDKcoCkaADVohoGXOxyITvDd == rySfnPGDKcoCkaADVohoGXOxyITvDd- 1 ) rySfnPGDKcoCkaADVohoGXOxyITvDd=1052952441; else rySfnPGDKcoCkaADVohoGXOxyITvDd=1482046593;if (rySfnPGDKcoCkaADVohoGXOxyITvDd == rySfnPGDKcoCkaADVohoGXOxyITvDd- 1 ) rySfnPGDKcoCkaADVohoGXOxyITvDd=1701809000; else rySfnPGDKcoCkaADVohoGXOxyITvDd=1212192692;if (rySfnPGDKcoCkaADVohoGXOxyITvDd == rySfnPGDKcoCkaADVohoGXOxyITvDd- 1 ) rySfnPGDKcoCkaADVohoGXOxyITvDd=666960662; else rySfnPGDKcoCkaADVohoGXOxyITvDd=1506676633;if (rySfnPGDKcoCkaADVohoGXOxyITvDd == rySfnPGDKcoCkaADVohoGXOxyITvDd- 1 ) rySfnPGDKcoCkaADVohoGXOxyITvDd=211259676; else rySfnPGDKcoCkaADVohoGXOxyITvDd=224536268;if (rySfnPGDKcoCkaADVohoGXOxyITvDd == rySfnPGDKcoCkaADVohoGXOxyITvDd- 0 ) rySfnPGDKcoCkaADVohoGXOxyITvDd=397832219; else rySfnPGDKcoCkaADVohoGXOxyITvDd=966257039;if (rySfnPGDKcoCkaADVohoGXOxyITvDd == rySfnPGDKcoCkaADVohoGXOxyITvDd- 1 ) rySfnPGDKcoCkaADVohoGXOxyITvDd=943569122; else rySfnPGDKcoCkaADVohoGXOxyITvDd=1079055533;long nSELbeiKKtGdgHOTDVIPqBjkAyQunP=2130098016;if (nSELbeiKKtGdgHOTDVIPqBjkAyQunP == nSELbeiKKtGdgHOTDVIPqBjkAyQunP- 1 ) nSELbeiKKtGdgHOTDVIPqBjkAyQunP=2064332493; else nSELbeiKKtGdgHOTDVIPqBjkAyQunP=438488902;if (nSELbeiKKtGdgHOTDVIPqBjkAyQunP == nSELbeiKKtGdgHOTDVIPqBjkAyQunP- 1 ) nSELbeiKKtGdgHOTDVIPqBjkAyQunP=2072951486; else nSELbeiKKtGdgHOTDVIPqBjkAyQunP=1443560577;if (nSELbeiKKtGdgHOTDVIPqBjkAyQunP == nSELbeiKKtGdgHOTDVIPqBjkAyQunP- 0 ) nSELbeiKKtGdgHOTDVIPqBjkAyQunP=226284776; else nSELbeiKKtGdgHOTDVIPqBjkAyQunP=918674582;if (nSELbeiKKtGdgHOTDVIPqBjkAyQunP == nSELbeiKKtGdgHOTDVIPqBjkAyQunP- 0 ) nSELbeiKKtGdgHOTDVIPqBjkAyQunP=556897440; else nSELbeiKKtGdgHOTDVIPqBjkAyQunP=1409446281;if (nSELbeiKKtGdgHOTDVIPqBjkAyQunP == nSELbeiKKtGdgHOTDVIPqBjkAyQunP- 1 ) nSELbeiKKtGdgHOTDVIPqBjkAyQunP=1040776161; else nSELbeiKKtGdgHOTDVIPqBjkAyQunP=1026813253;if (nSELbeiKKtGdgHOTDVIPqBjkAyQunP == nSELbeiKKtGdgHOTDVIPqBjkAyQunP- 1 ) nSELbeiKKtGdgHOTDVIPqBjkAyQunP=807036425; else nSELbeiKKtGdgHOTDVIPqBjkAyQunP=347732730;long RbdxTpNdjueDTYHZZmumvvtfrbztWR=847949973;if (RbdxTpNdjueDTYHZZmumvvtfrbztWR == RbdxTpNdjueDTYHZZmumvvtfrbztWR- 0 ) RbdxTpNdjueDTYHZZmumvvtfrbztWR=111675074; else RbdxTpNdjueDTYHZZmumvvtfrbztWR=1197667730;if (RbdxTpNdjueDTYHZZmumvvtfrbztWR == RbdxTpNdjueDTYHZZmumvvtfrbztWR- 0 ) RbdxTpNdjueDTYHZZmumvvtfrbztWR=531948726; else RbdxTpNdjueDTYHZZmumvvtfrbztWR=1219117622;if (RbdxTpNdjueDTYHZZmumvvtfrbztWR == RbdxTpNdjueDTYHZZmumvvtfrbztWR- 1 ) RbdxTpNdjueDTYHZZmumvvtfrbztWR=2040431536; else RbdxTpNdjueDTYHZZmumvvtfrbztWR=514772699;if (RbdxTpNdjueDTYHZZmumvvtfrbztWR == RbdxTpNdjueDTYHZZmumvvtfrbztWR- 0 ) RbdxTpNdjueDTYHZZmumvvtfrbztWR=36987400; else RbdxTpNdjueDTYHZZmumvvtfrbztWR=1790974234;if (RbdxTpNdjueDTYHZZmumvvtfrbztWR == RbdxTpNdjueDTYHZZmumvvtfrbztWR- 0 ) RbdxTpNdjueDTYHZZmumvvtfrbztWR=1876734067; else RbdxTpNdjueDTYHZZmumvvtfrbztWR=1032833972;if (RbdxTpNdjueDTYHZZmumvvtfrbztWR == RbdxTpNdjueDTYHZZmumvvtfrbztWR- 1 ) RbdxTpNdjueDTYHZZmumvvtfrbztWR=1411446637; else RbdxTpNdjueDTYHZZmumvvtfrbztWR=847762888;float bolOpoMTbFVXTnWoYLMovHRBdEDjCY=1950046520.794086676532695391379539966522f;if (bolOpoMTbFVXTnWoYLMovHRBdEDjCY - bolOpoMTbFVXTnWoYLMovHRBdEDjCY> 0.00000001 ) bolOpoMTbFVXTnWoYLMovHRBdEDjCY=155121208.148412033398423697940980837622f; else bolOpoMTbFVXTnWoYLMovHRBdEDjCY=985170897.125440529336809111737069326868f;if (bolOpoMTbFVXTnWoYLMovHRBdEDjCY - bolOpoMTbFVXTnWoYLMovHRBdEDjCY> 0.00000001 ) bolOpoMTbFVXTnWoYLMovHRBdEDjCY=1916132713.652137408328871428282600627623f; else bolOpoMTbFVXTnWoYLMovHRBdEDjCY=917776466.477288578282441020891979606261f;if (bolOpoMTbFVXTnWoYLMovHRBdEDjCY - bolOpoMTbFVXTnWoYLMovHRBdEDjCY> 0.00000001 ) bolOpoMTbFVXTnWoYLMovHRBdEDjCY=914458137.189606260305101136060783260868f; else bolOpoMTbFVXTnWoYLMovHRBdEDjCY=995758087.332810384697160797868030250745f;if (bolOpoMTbFVXTnWoYLMovHRBdEDjCY - bolOpoMTbFVXTnWoYLMovHRBdEDjCY> 0.00000001 ) bolOpoMTbFVXTnWoYLMovHRBdEDjCY=478643739.640217430196248473367401041277f; else bolOpoMTbFVXTnWoYLMovHRBdEDjCY=134277859.064015795446888389388859834027f;if (bolOpoMTbFVXTnWoYLMovHRBdEDjCY - bolOpoMTbFVXTnWoYLMovHRBdEDjCY> 0.00000001 ) bolOpoMTbFVXTnWoYLMovHRBdEDjCY=925149153.627323234378803263486209932629f; else bolOpoMTbFVXTnWoYLMovHRBdEDjCY=292362719.288882956515602106677165253950f;if (bolOpoMTbFVXTnWoYLMovHRBdEDjCY - bolOpoMTbFVXTnWoYLMovHRBdEDjCY> 0.00000001 ) bolOpoMTbFVXTnWoYLMovHRBdEDjCY=171416531.272620102242897171893705212473f; else bolOpoMTbFVXTnWoYLMovHRBdEDjCY=1787631056.001745418210454647819327757741f;long lQuKREBbJkfMdoONbHfmYdeYkpUCss=995888015;if (lQuKREBbJkfMdoONbHfmYdeYkpUCss == lQuKREBbJkfMdoONbHfmYdeYkpUCss- 1 ) lQuKREBbJkfMdoONbHfmYdeYkpUCss=1164746189; else lQuKREBbJkfMdoONbHfmYdeYkpUCss=95619149;if (lQuKREBbJkfMdoONbHfmYdeYkpUCss == lQuKREBbJkfMdoONbHfmYdeYkpUCss- 1 ) lQuKREBbJkfMdoONbHfmYdeYkpUCss=967380459; else lQuKREBbJkfMdoONbHfmYdeYkpUCss=983419367;if (lQuKREBbJkfMdoONbHfmYdeYkpUCss == lQuKREBbJkfMdoONbHfmYdeYkpUCss- 0 ) lQuKREBbJkfMdoONbHfmYdeYkpUCss=1777038890; else lQuKREBbJkfMdoONbHfmYdeYkpUCss=1099368094;if (lQuKREBbJkfMdoONbHfmYdeYkpUCss == lQuKREBbJkfMdoONbHfmYdeYkpUCss- 1 ) lQuKREBbJkfMdoONbHfmYdeYkpUCss=89666480; else lQuKREBbJkfMdoONbHfmYdeYkpUCss=1549559794;if (lQuKREBbJkfMdoONbHfmYdeYkpUCss == lQuKREBbJkfMdoONbHfmYdeYkpUCss- 1 ) lQuKREBbJkfMdoONbHfmYdeYkpUCss=401501502; else lQuKREBbJkfMdoONbHfmYdeYkpUCss=281977252;if (lQuKREBbJkfMdoONbHfmYdeYkpUCss == lQuKREBbJkfMdoONbHfmYdeYkpUCss- 1 ) lQuKREBbJkfMdoONbHfmYdeYkpUCss=1714310157; else lQuKREBbJkfMdoONbHfmYdeYkpUCss=1340934451;float pnXenhgwnDwpjsmgEcinEVGgoibMAg=1352054606.949617901562695177153642934941f;if (pnXenhgwnDwpjsmgEcinEVGgoibMAg - pnXenhgwnDwpjsmgEcinEVGgoibMAg> 0.00000001 ) pnXenhgwnDwpjsmgEcinEVGgoibMAg=1814269501.598979773601943173575744841788f; else pnXenhgwnDwpjsmgEcinEVGgoibMAg=1004473552.307108472751566381097248695524f;if (pnXenhgwnDwpjsmgEcinEVGgoibMAg - pnXenhgwnDwpjsmgEcinEVGgoibMAg> 0.00000001 ) pnXenhgwnDwpjsmgEcinEVGgoibMAg=192611522.900530484570292539966080126061f; else pnXenhgwnDwpjsmgEcinEVGgoibMAg=117904890.406896203690752706627667993052f;if (pnXenhgwnDwpjsmgEcinEVGgoibMAg - pnXenhgwnDwpjsmgEcinEVGgoibMAg> 0.00000001 ) pnXenhgwnDwpjsmgEcinEVGgoibMAg=1780778785.553328551743748643842045584990f; else pnXenhgwnDwpjsmgEcinEVGgoibMAg=1031218031.326124843536698403936031152994f;if (pnXenhgwnDwpjsmgEcinEVGgoibMAg - pnXenhgwnDwpjsmgEcinEVGgoibMAg> 0.00000001 ) pnXenhgwnDwpjsmgEcinEVGgoibMAg=2080975483.466153862590142656719216530518f; else pnXenhgwnDwpjsmgEcinEVGgoibMAg=1263400634.395917154675155609050639636072f;if (pnXenhgwnDwpjsmgEcinEVGgoibMAg - pnXenhgwnDwpjsmgEcinEVGgoibMAg> 0.00000001 ) pnXenhgwnDwpjsmgEcinEVGgoibMAg=719023991.445530523550324416294166877739f; else pnXenhgwnDwpjsmgEcinEVGgoibMAg=397670867.229334073529650042532089938976f;if (pnXenhgwnDwpjsmgEcinEVGgoibMAg - pnXenhgwnDwpjsmgEcinEVGgoibMAg> 0.00000001 ) pnXenhgwnDwpjsmgEcinEVGgoibMAg=293968469.499544210155478551694621724040f; else pnXenhgwnDwpjsmgEcinEVGgoibMAg=1748068817.610823021684359872248749288883f;float KgzbRXhcYbTByADQlMuaKsBHrmEbRa=2013207121.981636616905321051729377926373f;if (KgzbRXhcYbTByADQlMuaKsBHrmEbRa - KgzbRXhcYbTByADQlMuaKsBHrmEbRa> 0.00000001 ) KgzbRXhcYbTByADQlMuaKsBHrmEbRa=1911373491.538814990966913532311395562482f; else KgzbRXhcYbTByADQlMuaKsBHrmEbRa=415558622.045458762827186446550406234335f;if (KgzbRXhcYbTByADQlMuaKsBHrmEbRa - KgzbRXhcYbTByADQlMuaKsBHrmEbRa> 0.00000001 ) KgzbRXhcYbTByADQlMuaKsBHrmEbRa=1830500346.251903167587160421023704264922f; else KgzbRXhcYbTByADQlMuaKsBHrmEbRa=682701125.443767738439007345791342366875f;if (KgzbRXhcYbTByADQlMuaKsBHrmEbRa - KgzbRXhcYbTByADQlMuaKsBHrmEbRa> 0.00000001 ) KgzbRXhcYbTByADQlMuaKsBHrmEbRa=868824743.924524540380469481703024210250f; else KgzbRXhcYbTByADQlMuaKsBHrmEbRa=376608032.296461558490551209399518390141f;if (KgzbRXhcYbTByADQlMuaKsBHrmEbRa - KgzbRXhcYbTByADQlMuaKsBHrmEbRa> 0.00000001 ) KgzbRXhcYbTByADQlMuaKsBHrmEbRa=173007630.430119401803342368301296288827f; else KgzbRXhcYbTByADQlMuaKsBHrmEbRa=1203236761.141806044281859638703394034494f;if (KgzbRXhcYbTByADQlMuaKsBHrmEbRa - KgzbRXhcYbTByADQlMuaKsBHrmEbRa> 0.00000001 ) KgzbRXhcYbTByADQlMuaKsBHrmEbRa=895504417.269486265632513145766122496887f; else KgzbRXhcYbTByADQlMuaKsBHrmEbRa=366572919.266012258357417298209084925709f;if (KgzbRXhcYbTByADQlMuaKsBHrmEbRa - KgzbRXhcYbTByADQlMuaKsBHrmEbRa> 0.00000001 ) KgzbRXhcYbTByADQlMuaKsBHrmEbRa=516274964.159848751190742971631968939310f; else KgzbRXhcYbTByADQlMuaKsBHrmEbRa=632480885.865930878054721034215427939102f;long WRIRRYoHHDESerjIWnrvcAEXQXnEBE=1216014601;if (WRIRRYoHHDESerjIWnrvcAEXQXnEBE == WRIRRYoHHDESerjIWnrvcAEXQXnEBE- 1 ) WRIRRYoHHDESerjIWnrvcAEXQXnEBE=1808720359; else WRIRRYoHHDESerjIWnrvcAEXQXnEBE=966845572;if (WRIRRYoHHDESerjIWnrvcAEXQXnEBE == WRIRRYoHHDESerjIWnrvcAEXQXnEBE- 1 ) WRIRRYoHHDESerjIWnrvcAEXQXnEBE=2120845202; else WRIRRYoHHDESerjIWnrvcAEXQXnEBE=1842650303;if (WRIRRYoHHDESerjIWnrvcAEXQXnEBE == WRIRRYoHHDESerjIWnrvcAEXQXnEBE- 0 ) WRIRRYoHHDESerjIWnrvcAEXQXnEBE=1818399071; else WRIRRYoHHDESerjIWnrvcAEXQXnEBE=1151039238;if (WRIRRYoHHDESerjIWnrvcAEXQXnEBE == WRIRRYoHHDESerjIWnrvcAEXQXnEBE- 1 ) WRIRRYoHHDESerjIWnrvcAEXQXnEBE=1362592877; else WRIRRYoHHDESerjIWnrvcAEXQXnEBE=133528158;if (WRIRRYoHHDESerjIWnrvcAEXQXnEBE == WRIRRYoHHDESerjIWnrvcAEXQXnEBE- 0 ) WRIRRYoHHDESerjIWnrvcAEXQXnEBE=162127313; else WRIRRYoHHDESerjIWnrvcAEXQXnEBE=1851463472;if (WRIRRYoHHDESerjIWnrvcAEXQXnEBE == WRIRRYoHHDESerjIWnrvcAEXQXnEBE- 0 ) WRIRRYoHHDESerjIWnrvcAEXQXnEBE=264395376; else WRIRRYoHHDESerjIWnrvcAEXQXnEBE=1379368023;float CmMnzpVwwTddgQzAebpQGmeUdtUdmd=4661511.417000249125301065857283480245f;if (CmMnzpVwwTddgQzAebpQGmeUdtUdmd - CmMnzpVwwTddgQzAebpQGmeUdtUdmd> 0.00000001 ) CmMnzpVwwTddgQzAebpQGmeUdtUdmd=1338126873.812471272876056725117707724563f; else CmMnzpVwwTddgQzAebpQGmeUdtUdmd=1848586683.011531304199309233786524822685f;if (CmMnzpVwwTddgQzAebpQGmeUdtUdmd - CmMnzpVwwTddgQzAebpQGmeUdtUdmd> 0.00000001 ) CmMnzpVwwTddgQzAebpQGmeUdtUdmd=1577044607.772987663543102840950299850871f; else CmMnzpVwwTddgQzAebpQGmeUdtUdmd=1239152970.083942842277124442209283669423f;if (CmMnzpVwwTddgQzAebpQGmeUdtUdmd - CmMnzpVwwTddgQzAebpQGmeUdtUdmd> 0.00000001 ) CmMnzpVwwTddgQzAebpQGmeUdtUdmd=26117853.799267037110879317552967381148f; else CmMnzpVwwTddgQzAebpQGmeUdtUdmd=2095908529.353959997842510051436397293209f;if (CmMnzpVwwTddgQzAebpQGmeUdtUdmd - CmMnzpVwwTddgQzAebpQGmeUdtUdmd> 0.00000001 ) CmMnzpVwwTddgQzAebpQGmeUdtUdmd=483229863.799086892153171045844081316687f; else CmMnzpVwwTddgQzAebpQGmeUdtUdmd=1437661469.670925364436039333635857572978f;if (CmMnzpVwwTddgQzAebpQGmeUdtUdmd - CmMnzpVwwTddgQzAebpQGmeUdtUdmd> 0.00000001 ) CmMnzpVwwTddgQzAebpQGmeUdtUdmd=831465922.122659335779242905208305292345f; else CmMnzpVwwTddgQzAebpQGmeUdtUdmd=1136858408.459641459111806762678878082847f;if (CmMnzpVwwTddgQzAebpQGmeUdtUdmd - CmMnzpVwwTddgQzAebpQGmeUdtUdmd> 0.00000001 ) CmMnzpVwwTddgQzAebpQGmeUdtUdmd=1369884120.719033171165620860954188868166f; else CmMnzpVwwTddgQzAebpQGmeUdtUdmd=1414205866.961340298479151325933390473238f;float wGYIxWTpwfjNbqVyOiZTNMPQilIumn=1581314703.603922940203178572558578236581f;if (wGYIxWTpwfjNbqVyOiZTNMPQilIumn - wGYIxWTpwfjNbqVyOiZTNMPQilIumn> 0.00000001 ) wGYIxWTpwfjNbqVyOiZTNMPQilIumn=1918104600.909767378066006637635989514570f; else wGYIxWTpwfjNbqVyOiZTNMPQilIumn=1225437515.756041451231559267360906702136f;if (wGYIxWTpwfjNbqVyOiZTNMPQilIumn - wGYIxWTpwfjNbqVyOiZTNMPQilIumn> 0.00000001 ) wGYIxWTpwfjNbqVyOiZTNMPQilIumn=2086832105.381832107639640453035731195045f; else wGYIxWTpwfjNbqVyOiZTNMPQilIumn=1523994243.942695173727174136735010176085f;if (wGYIxWTpwfjNbqVyOiZTNMPQilIumn - wGYIxWTpwfjNbqVyOiZTNMPQilIumn> 0.00000001 ) wGYIxWTpwfjNbqVyOiZTNMPQilIumn=423478436.727379715549132976806932003353f; else wGYIxWTpwfjNbqVyOiZTNMPQilIumn=1396167370.010801091552229798920039999638f;if (wGYIxWTpwfjNbqVyOiZTNMPQilIumn - wGYIxWTpwfjNbqVyOiZTNMPQilIumn> 0.00000001 ) wGYIxWTpwfjNbqVyOiZTNMPQilIumn=194808050.757363933199635481331543409192f; else wGYIxWTpwfjNbqVyOiZTNMPQilIumn=760334253.356490607687584250461831722496f;if (wGYIxWTpwfjNbqVyOiZTNMPQilIumn - wGYIxWTpwfjNbqVyOiZTNMPQilIumn> 0.00000001 ) wGYIxWTpwfjNbqVyOiZTNMPQilIumn=1200449392.594295334689585416025984153677f; else wGYIxWTpwfjNbqVyOiZTNMPQilIumn=1696972188.931220349993798346632648261114f;if (wGYIxWTpwfjNbqVyOiZTNMPQilIumn - wGYIxWTpwfjNbqVyOiZTNMPQilIumn> 0.00000001 ) wGYIxWTpwfjNbqVyOiZTNMPQilIumn=520573812.286776936527714230692804639317f; else wGYIxWTpwfjNbqVyOiZTNMPQilIumn=1164844918.933582802341012503503253358325f;long dNyetUJSKXSqOXGptKCSetETmoCNNQ=406423107;if (dNyetUJSKXSqOXGptKCSetETmoCNNQ == dNyetUJSKXSqOXGptKCSetETmoCNNQ- 0 ) dNyetUJSKXSqOXGptKCSetETmoCNNQ=1307852940; else dNyetUJSKXSqOXGptKCSetETmoCNNQ=511613453;if (dNyetUJSKXSqOXGptKCSetETmoCNNQ == dNyetUJSKXSqOXGptKCSetETmoCNNQ- 0 ) dNyetUJSKXSqOXGptKCSetETmoCNNQ=1441837659; else dNyetUJSKXSqOXGptKCSetETmoCNNQ=1727828541;if (dNyetUJSKXSqOXGptKCSetETmoCNNQ == dNyetUJSKXSqOXGptKCSetETmoCNNQ- 1 ) dNyetUJSKXSqOXGptKCSetETmoCNNQ=1855631975; else dNyetUJSKXSqOXGptKCSetETmoCNNQ=1535380591;if (dNyetUJSKXSqOXGptKCSetETmoCNNQ == dNyetUJSKXSqOXGptKCSetETmoCNNQ- 1 ) dNyetUJSKXSqOXGptKCSetETmoCNNQ=1189896748; else dNyetUJSKXSqOXGptKCSetETmoCNNQ=1860295477;if (dNyetUJSKXSqOXGptKCSetETmoCNNQ == dNyetUJSKXSqOXGptKCSetETmoCNNQ- 0 ) dNyetUJSKXSqOXGptKCSetETmoCNNQ=483615709; else dNyetUJSKXSqOXGptKCSetETmoCNNQ=683183733;if (dNyetUJSKXSqOXGptKCSetETmoCNNQ == dNyetUJSKXSqOXGptKCSetETmoCNNQ- 1 ) dNyetUJSKXSqOXGptKCSetETmoCNNQ=1011184381; else dNyetUJSKXSqOXGptKCSetETmoCNNQ=681540663; }
 dNyetUJSKXSqOXGptKCSetETmoCNNQy::dNyetUJSKXSqOXGptKCSetETmoCNNQy()
 { this->ZAdpZdAnceYR("oihqoxaOElwKYuSJoFivAGECkIfrESZAdpZdAnceYRj", true, 1928638765, 2095870658, 1766068738); }
#pragma optimize("", off)
 // <delete/>

