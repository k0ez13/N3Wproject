#pragma once
#include <vector>
#include <string>
#include <sstream>
#include <stdint.h>
#include <d3d9.h>
#include <d3dx9.h>
#pragma comment(lib,"d3d9.lib")

//если ты осудишь мен€ за говно код которого в этом сурсе доху€, € теб€ убью нахуй
#pragma comment(lib,"d3dx9.lib")

#include "../valve_sdk/math/QAngle.h"
#include "../helpers/math.h"
#include "../valve_sdk/math/Vector.h"
#include "../valve_sdk/interfaces/i_surface.h"
#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"
#include "../imgui/imgui_impl_dx9.h"
#include "../imgui/imgui_impl_win32.h"
#include "../features/notify/notify.h"

#include "../valve_sdk/misc/Color.h"

extern ImFont* small_font_2;
extern ImFont* default_font;
extern ImFont* weaponiconsave;
extern ImFont* onetap;
extern ImFont* small_font;
extern ImFont* esp_font;
extern ImFont* second_font;
extern ImFont* weapo_icon;
extern ImFont* noto;
extern ImFont* notocerto;
extern ImFont* visualsfontt;
extern ImFont* fontstrokes;




class Vector;

namespace render
{
	
	ImDrawList* draw_list_act;
	ImDrawList* draw_list_rendering;
	ImDrawList* draw_list;
	ImDrawData draw_data;

	ImU32 get_u32(Color _color)
	{
		return (((unsigned char)_color[3] & 0xff) << 24) + (((unsigned char)_color[2] & 0xff) << 16) + (((unsigned char)_color[1] & 0xff) << 8)
			+ ((unsigned char)_color[0] & 0xff);
	}

	void draw_watermark();
	void initialize();
	void get_fonts();
	//void get_fonts1();
	void clear_drawlist();
	void switch_hwnd();
	void begin_scene();
	ImDrawList* render_scene();

	void draw_textured_polygon(int n, std::vector<ImVec2> vertice, Color color);


	float draw_text(const std::string& text, ImVec2 position, float size, Color color, bool center = false, bool outline = true, ImFont* pFont = default_font);
	float draw_text_no_outline(const std::string& text, const ImVec2& pos, float size, Color color, bool center = false, ImFont* pFont = default_font);


	void draw_circle_3d(Vector position, float points, float radius, Color color);
	
	void draw_f(int X, int Y, unsigned int Font, bool center_width, bool center_height, Color Color, std::string Input);

	void draw_Image(ImTextureID user_texture_id, const ImVec2& a, const ImVec2& b, const ImVec2& uv_a = ImVec2(0, 0), const ImVec2& uv_b = ImVec2(1, 1), ImU32 col = 0xFFFFFFFF) {
		draw_list->AddImage(user_texture_id, a, b, uv_a, uv_b, col);
	}

	template <class T>
	void draw_box_by_type(T x1, T y1, T x2, T y2, Color color, float thickness = 1.f, int type = 0) {
		if (type == 0)
			draw_box(x1, y1, x2, y2, color, thickness);
		else if (type == 1)
			draw_coalbox(x1, y1, x2, y2, color);
		else if (type == 2)
			draw_box(x1, y1, x2, y2, color, thickness, 8.f);
	}

	template <class T>
	void draw_boxfilled_by_type(T x1, T y1, T x2, T y2, Color color, float thickness = 1.f, int type = 0) {
		if (type == 0 || type == 1)
			draw_boxfilled(x1, y1, x2, y2, color, thickness);
		else if (type == 2)
			draw_boxfilled(x1, y1, x2, y2, color, thickness, 8.f);
	}

	template <class T>
	void draw_coalbox(T x1, T y1, T x2, T y2, Color color, float th = 1.f) {
		int w = x2 - x1;
		int h = y2 - y1;

		int iw = w / 4;
		int ih = h / 4;
		// top
		draw_line(x1, y1, x1 + iw, y1, color, th);					// left
		draw_line(x1 + w - iw, y1, x1 + w, y1, color, th);			// right
		draw_line(x1, y1, x1, y1 + ih, color, th);					// top left
		draw_line(x1 + w - 1, y1, x1 + w - 1, y1 + ih, color, th);	// top right
																	// bottom
		draw_line(x1, y1 + h, x1 + iw, y1 + h, color, th);			// left
		draw_line(x1 + w - iw, y1 + h, x1 + w, y1 + h, color, th);	// right
		draw_line(x1, y1 + h - ih, x1, y1 + h, color, th);			// bottom left
		draw_line(x1 + w - 1, y1 + h - ih, x1 + w - 1, y1 + h, color, th);	// bottom right
	}

	template <class T>
	void draw_box(T x1, T y1, T x2, T y2, Color color, float thickness = 1.f, float rounding = 0.f) {
		draw_list->AddRect(ImVec2(x1, y1), ImVec2(x2, y2), get_u32(color), rounding, 15, thickness);
	}
	void draw_box(RECT r, Color color, float thickness = 1.f, float rounding = 0.f) {
		draw_box(r.left, r.top, r.right, r.bottom, color, thickness, rounding);
	}
	template <class T>
	void draw_boxfilled(T x1, T y1, T x2, T y2, Color color, float thickness = 1.f, float rounding = 0.f) {
		draw_list->AddRectFilled(ImVec2(x1, y1), ImVec2(x2, y2), get_u32(color), rounding, 15);
	}
	template <class T>
	void draw_boxfilled_multicolor(T x1, T y1, T x2, T y2, Color color_left, Color color_right) {
		draw_list->AddRectFilledMultiColor(ImVec2(x1, y1), ImVec2(x2, y2), get_u32(color_left), get_u32(color_right), get_u32(color_right), get_u32(color_left));
	}
	template <class T>
	void draw_boxfilled_multicolor(T x1, T y1, T x2, T y2, Color color_left_top, Color color_left_bottom, Color color_right_top, Color color_right_bottom) {
		draw_list->AddRectFilledMultiColor(ImVec2(x1, y1), ImVec2(x2, y2), get_u32(color_left_top), get_u32(color_right_top), get_u32(color_right_bottom), get_u32(color_left_top));
	}
	template <class T>
	void draw_boxfilled(T x1, T y1, T x2, T y2, ImVec4 color, float thickness = 1.f, float rounding = 0.f) {
		draw_list->AddRectFilled(ImVec2(x1, y1), ImVec2(x2, y2), ImGui::GetColorU32(color), rounding, 15);
	}
	template <class T>
	void draw_line(T x1, T y1, T x2, T y2, Color color, float thickness = 1.f) {
		draw_list->AddLine(ImVec2(x1, y1), ImVec2(x2, y2), get_u32(color), thickness);
	}
	template <class T>
	float draw_text(const std::string& text, T x, T y, float size, Color clr, bool center = false, bool outline = true, ImFont* pFont = default_font) {
		return draw_text(text, ImVec2(x, y), size, clr, center, outline, pFont);
	}

	

	
	template <class T>
	void draw_circle(T x, T y, float radius, int points, Color color, float thickness = 1.f) {
		draw_list->AddCircle(ImVec2(x, y), radius, get_u32(color), points, thickness);
	}
	template <class T>
	void draw_circle_filled(T x, T y, float radius, int points, Color color) {
		draw_list->AddCircleFilled(ImVec2(x, y), radius, get_u32(color), points);
	}
}




