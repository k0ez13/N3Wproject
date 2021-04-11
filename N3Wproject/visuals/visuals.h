#pragma once

#include "../../render/render.h"
#include "../../helpers/math.h"
#include "../../valve_sdk/csgostructs.h"

#include <map>
#pragma once

template<typename T>
class Singleton
{
protected:
	Singleton() {}
	~Singleton() {}

	Singleton(const Singleton&) = delete;
	Singleton& operator=(const Singleton&) = delete;

	Singleton(Singleton&&) = delete;
	Singleton& operator=(Singleton&&) = delete;

public:
	static T& Get()
	{
		static T inst{};
		return inst;
	}
};


struct molotov_info_t
{
	Vector position;
	float time_to_expire;
};

struct smoke_info_t
{
	Vector position;
	float time_to_expire;
};

struct sound_info_t {
	int guid;
	float soundTime;
	float alpha;
	Vector soundPos;
};

class VGSHelper : public Singleton<VGSHelper>
{
public:
	void Init();
	void DrawText(std::string text, float x, float y, Color color, int size = 15);
	void DrawIcon(std::string text, float x, float y, Color color, int size);
	void DrawRing3D(int16_t x, int16_t y, int16_t z, int16_t radius, uint16_t points, Color color1, float thickness);
	void DrawLine(float x1, float y1, float x2, float y2, Color color, float size = 1.f);
	void DrawBox(float x1, float y1, float x2, float y2, Color clr, float size = 1.f);
	void DrawFilledBox(float x1, float y1, float x2, float y2, Color clr);
	//void DrawTriangle(int count, Vertex_t* vertexes, Color c);
	void DrawBoxEdges(float x1, float y1, float x2, float y2, Color clr, float edge_size, float size = 1.f);
	void DrawCircle(float x, float y, float r, int seg, Color clr);
	void DrawCircle(Vector2D position, float r, int seg, Color clr);

	ImVec2 GetSize(std::string text, int size = 15);
private:
	bool Inited = false;
	vgui::HFont fonts[128];
	vgui::HFont icons[128];
};

namespace visuals
{
	class player
	{
	public:
		struct
		{
			c_base_player* pl;
			bool          is_enemy;
			bool          is_visible;
			bool          create_font;
			bool          is_localplayer;
			Color         clr;
			Vector        head_pos;
			Vector        feet_pos;
			RECT          bbox;
		} ctx;

		bool begin(c_base_player* pl);
		void draw_box();
		void draw_boxedge(float x1, float y1, float x2, float y2, Color clr, float edge_size, float size = 1.f);
		void draw_name();
		void draw_flags();
		bool create_font();
		void draw_icon(std::string text, float x, float y, Color color, int size);

		void draw_weapon();
		void draw_health();
		void draw_armour();
		void draw_skeleton(c_base_entity* ent);
		void draw_backtrack();
	};

	std::vector<molotov_info_t> molotov_info;
	std::vector<smoke_info_t> smoke_info;
	std::map< int, std::vector< sound_info_t > > m_Sounds;
	CUtlVector<snd_info_t> sounds;


	void events(i_gameevent* event);

	void RenderSounds();
	void UpdateSounds();
	void draw_spreed_circle();
	void draw_aa_arrow();
	void draw_aa_lines();
	void draw_grenades();
	void draw_icon(std::string text, float x, float y, Color color, int size);
	void sniper_crosshair();
	void draw_bullet_impacts();
	void nightmode();
	void create_font();
	void draw_damage_indicator();
	void draw_bullet_tracer(Vector start, Vector end, Color col);
	void draw_granades(c_base_entity* entity);
	void draw_weapons(c_base_combat_weapon* ent);
	void draw_defusekit(c_base_entity* ent);
	void draw_window_c4(c_base_entity* ent);
	void draw_window_c4(c_base_entity* ent);
	void draw_items(c_base_entity* ent);
	void draw_hitmarker();
	void draw_spectator_list();
	void third_person();
	void SetThirdpersonAngles();
	void more_chams()noexcept ;
	void draw_fov();
	float flHurtTime;
	void RenderHitmarker();
	void draw_player_arrow(c_base_player* pl);

	void render();

	void capsule_hitbox(c_base_entity* player, Color col, float duration);
}
