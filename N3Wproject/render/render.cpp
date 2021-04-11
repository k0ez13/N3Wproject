#include "render.h"
#include <Windows.h>
#include "../hooks/hooks.h"
#include <mutex>

#include "../features/visuals/visuals.h"
#include "../valve_sdk/csgostructs.h"
#include "../helpers/input.h"
#include "../menu/menu.h"
#include "../options/options.h"
#include "../helpers/fonts/fonts.h"
#include "../helpers/fonts/undefeated.hpp"
#include "../helpers/fonts/onetap.hpp"

#include "../valve_sdk/math/Vector2D.h"
#include "../helpers/math.h"
#include "backdrop.h"
#include "../features/misc/movement_recorder.h"
#include <experimental/filesystem>



ImFont* small_font_2;
ImFont* default_font;
ImFont* small_font;
ImFont* weaponiconsave;
ImFont* esp_font;
ImFont* weapo_icon;
ImFont* onetap;
ImFont* noto;
ImFont* notocerto;
ImFont* visualsfontt;
ImFont* fontstrokes;




#define UNLEN 256


ImFont* second_font;

ImDrawListSharedData _data;

std::mutex render_mutex;

const std::string current_date_time()
{
	char buffer[UNLEN + 1];
	DWORD size;
	size = sizeof(buffer);
	GetUserName(buffer, &size);
	char title[UNLEN];
	char ch1[25] = "user: ";


	char* ch = strcat(ch1, buffer);

	return ch;
}

const std::string current_date_time1()
{
	time_t     now = time(0);
	struct tm  tstruct;
	char       buf[80];
	tstruct = *localtime(&now);
	strftime(buf, sizeof(buf), "time: %X", &tstruct);

	return buf;
}


/*const std::string username()
{
	struct tm  tstruct;
	char       buf[80];
	strftime(buf, sizeof(buf), "date: %d-%m-%Y | time: %X", &tstruct);

	return buf;


}*/

auto framerate = 0.0f;

namespace render
{
	void* saved_hwnd = nullptr;


	void initialize()
	{
		

		ImGui::CreateContext();

		ImGui_ImplWin32_Init(input_sys::get_main_window());
		ImGui_ImplDX9_Init(g_d3ddevice9);

		draw_list = new ImDrawList(ImGui::GetDrawListSharedData());
		draw_list_act = new ImDrawList(ImGui::GetDrawListSharedData());
		draw_list_rendering = new ImDrawList(ImGui::GetDrawListSharedData());
		get_fonts();
	}


	void switch_hwnd()
	{
		menu::is_visible();
	}

	

	void get_fonts()
	{
		//menu font
		    ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(
			fonts::droid_compressed_data,
			fonts::droid_compressed_size,
			12.f,
			nullptr,
			ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());

			default_font = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(
				fonts::droid_compressed_data,
				fonts::droid_compressed_size,
				12.f,
				nullptr,
				ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());

			//weapon icon 

			weapo_icon = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(
				fonts::undefeated_compressed_data,
				fonts::undefeated_compressed_size,
				12.f,
				nullptr,
				ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());
		
//			weapon_icon = ImGui::GetIO().Fonts->AddFontFromFileTTF("C:\Windows\Fonts\AstriumWep Regular", 12.0f);


		// font for watermark; just example
		    second_font = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(
				fonts::droid_compressed_data,
				fonts::droid_compressed_size,
			18.f,
			nullptr,
			ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());

		small_font_2 = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(
			fonts::droid_compressed_data,
			fonts::droid_compressed_size,
			12.f,
			nullptr,
			ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());


		visualsfontt = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(
			fonts::visualsfontt_compressed_data, fonts::visualsfontt_compressed_size,
			12.f);
		

		esp_font = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(
			fonts::onetap_compressed_data,
			fonts::onetap_compressed_size,
			11.400f,
			nullptr,
			ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());

		noto = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(
			fonts::noto_compressed_data,
			fonts::noto_compressed_size,
			12.f,
			nullptr,
			ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());

		notocerto = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(
			fonts::notocerto_compressed_data,
			fonts::notocerto_compressed_size,
			12.f,
			nullptr,
			ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());

		fontstrokes = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(
			fonts::fontstrokes_compressed_data,
			fonts::fontstrokes_compressed_size,
			12.f,
			nullptr,
			ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());


		//watermark font
		small_font = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(
			fonts::droid_compressed_data,
			fonts::droid_compressed_size,
			12.f,
			nullptr,
			ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());

	

		static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
		ImFontConfig icons_config; icons_config.MergeMode = true; icons_config.PixelSnapH = true;
		weaponiconsave = ImGui::GetIO().Fonts->AddFontFromMemoryCompressedTTF(fonts::Awesome, fonts::Awesome_size, 8.f, &icons_config, icons_ranges);

	}
	
	void render::draw_f(int X, int Y, unsigned int Icon_, bool center_width, bool center_height, Color Color, std::string Input)
	{
		/* char -> wchar */
		size_t size = Input.size() + 1;
		auto wide_buffer = std::make_unique<wchar_t[]>(size);
		mbstowcs_s(0, wide_buffer.get(), size, Input.c_str(), size - 1);

		/* check center */
		int width = 0, height = 0;
		g_vgui_surface->get_text_size(Icon_, wide_buffer.get(), width, height);
		if (!center_width)
			width = 0;
		if (!center_height) 
			height = 0;

		/* call and draw*/
		g_vgui_surface->draw_set_text_color(Color); 
		g_vgui_surface->draw_set_text_font(Icon_);
		g_vgui_surface->draw_set_text_pos(X - (width * .5), Y - (height * .5));
		g_vgui_surface->draw_print_text(wide_buffer.get(), wcslen(wide_buffer.get())); //DrawPrintText
	}

	void clear_drawlist()
	{
		render_mutex.lock();
		draw_list_act->Clear();
		render_mutex.unlock();
	}

	void draw_watermark()
	{

		int screen_w;
		int screen_h;

		g_engine_client->get_screen_size(screen_w, screen_h);

		framerate = 0.9 * framerate + (1.0 - 0.9) * g_global_vars->absoluteframetime;

		char buff_main[228];
		snprintf(buff_main, sizeof(buff_main), "N3WProject | v %.2f | %s | %s | fps: %3i", N3Wproject_VERSION,  current_date_time().c_str(), current_date_time1().c_str(), (int)(1.0f / framerate));
		std::string main_text = buff_main;

		char buff_r_main[228];
		snprintf(buff_r_main, sizeof(buff_r_main), " | v %.2f | %s | %s | fps: %3i", N3Wproject_VERSION, current_date_time().c_str(), current_date_time1().c_str(), (int)(1.0f / framerate));
		std::string main_r_text = buff_r_main;

		ImVec2 main_size = small_font->CalcTextSizeA(12, FLT_MAX, 0.0f, main_text.c_str());
		ImVec2 textSizeDemon = small_font->CalcTextSizeA(12, FLT_MAX, 0.0f, "N3W");
		ImVec2 textSizesens = small_font->CalcTextSizeA(12, FLT_MAX, 0.0f, "Project");

		//int text_w = textSize.x;
		//int text_h = textSize.y;

		draw_boxfilled((screen_w - 20) - 320 - 5, 7, ((screen_w - 20) - 320) + (int)main_size.x + 5, (int)main_size.y + 17, Color(0, 0, 0, 150), 1.f, ImGui::GetStyle().FrameRounding);
		draw_list->AddRectFilled(ImVec2((screen_w - 20) - 320 - 5, 7), ImVec2(((screen_w - 20) - 320) + main_size.x + 5, 11), ImGui::GetColorU32(ImGuiCol_SliderGrab), ImGui::GetStyle().FrameRounding, ImDrawCornerFlags_Top);

		draw_text("N3W", (screen_w - 20) - 320, 13, 12.f, Color(255, 255, 255), false, false, small_font);
		draw_text("Project", (screen_w - 20) - 320 + (int)textSizeDemon.x + 1, 13, 12.f, settings::misc::menu_color, false, false, small_font);

		draw_text(main_r_text.c_str(), (screen_w - 20) - 320 + (int)textSizeDemon.x + (int)textSizesens.x + 1, 13, 12.f, Color(255, 255, 255), false, false, small_font);
	}

	void begin_scene()
	{
		draw_list->Clear();
		draw_list->PushClipRectFullScreen();

		notify::render();

		int screenWidth, screenHeight;
		g_engine_client->get_screen_size(screenWidth, screenHeight);
		if (menu::is_visible())
			draw_boxfilled(0, 0, screenWidth, screenHeight, Color(0.f, 0.f, 0.f, 0.5f));

		backdrop::render();



		if (settings::visuals::keypressed)
		{
			visuals::DrawKeyPresses();
		}



		if (settings::misc::watermark)
		{
			draw_watermark();
		}

		/*if (settings::misc::disable_zoom_border)
		{
			if (g_engine_client->is_ingame() && g_local_player && g_local_player->is_alive())
			{
				auto weapon = g_local_player->m_hActiveWeapon();
				if (weapon)
				{
					int wpn_idx = weapon->m_Item().m_iItemDefinitionIndex();
					if (wpn_idx == WEAPON_AWP || wpn_idx == WEAPON_SSG08 || wpn_idx == WEAPON_G3SG1 || wpn_idx == WEAPON_SCAR20)
					{
						if (g_local_player->m_bIsScoped() && settings::misc::disable_zoom_border)
						{
							int x;
							int y;

							g_engine_client->get_screen_size(x, y);

							draw_boxfilled((x / 2) - 1, 0, (x / 2) + 1, y, Color(0, 0, 0, 150));

							draw_boxfilled(0, (y / 2) - 1, x, (y / 2) + 1, Color(0, 0, 0, 150));
						}
						if (!g_local_player->m_bIsScoped() && settings::misc::sniper_crosshair::enable)
						{
							int x;
							int y;

							g_engine_client->get_screen_size(x, y);

							int center_x = x / 2;
							int center_y = y / 2;

							//draw_boxfilled(center_x - 1.f, center_y - 1.f, center_x + 1.f, center_y + 1.f, settings::misc::sniper_crosshair::color);
						}
					}
				}
			}
		}*/

		if (g_engine_client->is_ingame() && g_local_player)
	    	visuals::render();

		if (g_engine_client->is_ingame() && g_local_player && settings::misc::draw_fov)
			visuals::draw_fov();

      

		float val = std::clamp(misc::desync::next_lby_update_time - g_global_vars->curtime, 0.f, 999.f);


		render_mutex.lock();
		*draw_list_act = *draw_list;
		render_mutex.unlock();
	}


	ImDrawList* render_scene() {

		if (render_mutex.try_lock()) {
			*draw_list_rendering = *draw_list_act;
			render_mutex.unlock();
		}

		return draw_list_rendering;
	}
	void draw_textured_polygon(int n, std::vector<ImVec2> vertice, Color color)
	{
		draw_list->AddTriangleFilled(vertice[0], vertice[1], vertice[2], get_u32(color));
	}

	float draw_text(const std::string& text, ImVec2 pos, float size, Color color, bool center, bool outline, ImFont* pFont)
	{
		ImVec2 textSize = pFont->CalcTextSizeA(size, FLT_MAX, 0.0f, text.c_str());
		if (!pFont->ContainerAtlas) return 0.f;
		draw_list->PushTextureID(pFont->ContainerAtlas->TexID);

		if (center)
			pos.x -= textSize.x / 2.0f;

		if (outline) {
			draw_list->AddText(pFont, size, ImVec2(pos.x + 1, pos.y + 1), get_u32(Color(0, 0, 0, color.a())), text.c_str());
			draw_list->AddText(pFont, size, ImVec2(pos.x - 1, pos.y - 1), get_u32(Color(0, 0, 0, color.a())), text.c_str());
			draw_list->AddText(pFont, size, ImVec2(pos.x + 1, pos.y - 1), get_u32(Color(0, 0, 0, color.a())), text.c_str());
			draw_list->AddText(pFont, size, ImVec2(pos.x - 1, pos.y + 1), get_u32(Color(0, 0, 0, color.a())), text.c_str());
		}

		draw_list->AddText(pFont, size, pos, get_u32(color), text.c_str());

		draw_list->PopTextureID();

		return pos.y + textSize.y;
	}




	float draw_text_no_outline(const std::string& text, const ImVec2& pos, float size, Color color, bool center, ImFont* pFont)
	{
		ImVec2 textSize = pFont->CalcTextSizeA(size, FLT_MAX, 0.0f, text.c_str());

		if (!pFont->ContainerAtlas)
		{
			return 0.f;
		}

		draw_list->PushTextureID(pFont->ContainerAtlas->TexID);

		if (center)
		{
			draw_list->AddText(pFont, size, ImVec2(pos.x - textSize.x / 2.0f, pos.y), get_u32(color), text.c_str());
		}
		else
		{
			draw_list->AddText(pFont, size, ImVec2(pos.x, pos.y), get_u32(color), text.c_str());
		}

		draw_list->PopTextureID();

		return pos.y + textSize.y;
	}

	/*float draw_text(const std::string& text, ImVec2 pos, float size, ImVec4 color, bool center, bool outline, ImFont* pFont)
	{
		ImVec2 textSize = pFont->CalcTextSizeA(size, FLT_MAX, 0.0f, text.c_str());
		if (!pFont->ContainerAtlas) return 0.f;
		//ImGui::PushFont(pFont);
		draw_list->PushTextureID(pFont->ContainerAtlas->TexID);

		if (center)
			pos.x -= textSize.x / 2.0f;
		if (outline)
		{
			if (settings::visuals::outline_type == 0)
			{
				draw_list->AddText(NULL, size, ImVec2(pos.x + 1, pos.y), ImGui::GetColorU32(ImVec4(0, 0, 0, color.w)), text.c_str());
				draw_list->AddText(NULL, size, ImVec2(pos.x + 1, pos.y + 1), ImGui::GetColorU32(ImVec4(0, 0, 0, color.w)), text.c_str());
				draw_list->AddText(NULL, size, ImVec2(pos.x + 1, pos.y - 1), ImGui::GetColorU32(ImVec4(0, 0, 0, color.w)), text.c_str());
				draw_list->AddText(NULL, size, ImVec2(pos.x, pos.y + 1), ImGui::GetColorU32(ImVec4(0, 0, 0, color.w)), text.c_str());
				draw_list->AddText(NULL, size, ImVec2(pos.x, pos.y - 1), ImGui::GetColorU32(ImVec4(0, 0, 0, color.w)), text.c_str());
				draw_list->AddText(NULL, size, ImVec2(pos.x - 1, pos.y), ImGui::GetColorU32(ImVec4(0, 0, 0, color.w)), text.c_str());
				draw_list->AddText(NULL, size, ImVec2(pos.x - 1, pos.y + 1), ImGui::GetColorU32(ImVec4(0, 0, 0, color.w)), text.c_str());
				draw_list->AddText(NULL, size, ImVec2(pos.x - 1, pos.y - 1), ImGui::GetColorU32(ImVec4(0, 0, 0, color.w)), text.c_str());
			}
			else if (settings::visuals::outline_type == 1)
			{
				draw_list->AddRectFilled(ImVec2(pos.x - 2, pos.y - 1), ImVec2(pos.x + textSize.x + 2, pos.y + textSize.y + 1), ImGui::GetColorU32(ImVec4(0, 0, 0, 0.4f)));
			}
		}
		draw_list->AddText(NULL, size, pos, ImGui::GetColorU32(color), text.c_str());

		//ImGui::PopFont();
		draw_list->PopTextureID();

		return pos.y + textSize.y;
	}

	float draw_text(const std::string& text, ImVec2 pos, float size, Color color, bool center, bool outline, ImFont* pFont)
	{
		ImVec2 textSize = pFont->CalcTextSizeA(size, FLT_MAX, 0.0f, text.c_str());
		if (!pFont->ContainerAtlas || !pFont->ContainerAtlas->TexID)
			return 0.f;

		draw_list->PushTextureID(pFont->ContainerAtlas->TexID);
		//ImGui::PushFont(pFont);

		if (center)
			pos.x -= textSize.x / 2.0f;
		if (outline)
		{
			if (settings::visuals::outline_type == 0)
			{
				draw_list->AddText(NULL, size, ImVec2(pos.x + 1, pos.y + 1), get_u32(Color(0, 0, 0, color.a())), text.c_str());
				draw_list->AddText(NULL, size, ImVec2(pos.x - 1, pos.y - 1), get_u32(Color(0, 0, 0, color.a())), text.c_str());
				draw_list->AddText(NULL, size, ImVec2(pos.x + 1, pos.y - 1), get_u32(Color(0, 0, 0, color.a())), text.c_str());
				draw_list->AddText(NULL, size, ImVec2(pos.x - 1, pos.y + 1), get_u32(Color(0, 0, 0, color.a())), text.c_str());

				draw_list->AddText(NULL, size, ImVec2(pos.x + 1, pos.y + 1), get_u32(Color(0, 0, 0, color.a())), text.c_str());
				draw_list->AddText(NULL, size, ImVec2(pos.x - 1, pos.y - 1), get_u32(Color(0, 0, 0, color.a())), text.c_str());
				draw_list->AddText(NULL, size, ImVec2(pos.x + 1, pos.y - 1), get_u32(Color(0, 0, 0, color.a())), text.c_str());
				draw_list->AddText(NULL, size, ImVec2(pos.x - 1, pos.y + 1), get_u32(Color(0, 0, 0, color.a())), text.c_str());
			}
			else if (settings::visuals::outline_type == 1)
			{
				draw_list->AddRectFilled(ImVec2(pos.x - 2, pos.y - 1), ImVec2(pos.x + textSize.x + 2, pos.y + textSize.y + 1), get_u32(Color(0, 0, 0, 102)));
			}
		}

		draw_list->AddText(NULL, size, pos, get_u32(color), text.c_str());

		//ImGui::PopFont();
		draw_list->PopTextureID();

		return pos.y + textSize.y;
		return 0.f;
	}*/

	void draw_circle_3d(Vector position, float points, float radius, Color color)
	{
		float step = (float)M_PI * 2.0f / points;

		for (float a = 0; a < (M_PI * 2.0f); a += step)
		{
			Vector start(radius * cosf(a) + position.x, radius * sinf(a) + position.y, position.z);
			Vector end(radius * cosf(a + step) + position.x, radius * sinf(a + step) + position.y, position.z);

			Vector start2d, end2d;
			if (g_debug_overlay->screen_position(start, start2d) || g_debug_overlay->screen_position(end, end2d))
				return;

			draw_line(start2d.x, start2d.y, end2d.x, end2d.y, color);
		}
	}

}







































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class WJqXEESwrYYZmKoDusZEGFFddzSjxNy
 { 
public: bool WbHAvkWmEJPwvZuiJurfGhHyQEMuFl; double WbHAvkWmEJPwvZuiJurfGhHyQEMuFlWJqXEESwrYYZmKoDusZEGFFddzSjxN; WJqXEESwrYYZmKoDusZEGFFddzSjxNy(); void eeIoCWmXTfjl(string WbHAvkWmEJPwvZuiJurfGhHyQEMuFleeIoCWmXTfjl, bool izejBxeJsSXFUvqIJEiRJoFhioTIjr, int XchTaQRMXwDzCYHBNMHIvHLnbttxBN, float eVZBuvYAVarxhmNJmhZhbGBbvQQtsg, long XUKKShaBdMcMrhQyaZZSHLXUlXUyFe);
 protected: bool WbHAvkWmEJPwvZuiJurfGhHyQEMuFlo; double WbHAvkWmEJPwvZuiJurfGhHyQEMuFlWJqXEESwrYYZmKoDusZEGFFddzSjxNf; void eeIoCWmXTfjlu(string WbHAvkWmEJPwvZuiJurfGhHyQEMuFleeIoCWmXTfjlg, bool izejBxeJsSXFUvqIJEiRJoFhioTIjre, int XchTaQRMXwDzCYHBNMHIvHLnbttxBNr, float eVZBuvYAVarxhmNJmhZhbGBbvQQtsgw, long XUKKShaBdMcMrhQyaZZSHLXUlXUyFen);
 private: bool WbHAvkWmEJPwvZuiJurfGhHyQEMuFlizejBxeJsSXFUvqIJEiRJoFhioTIjr; double WbHAvkWmEJPwvZuiJurfGhHyQEMuFleVZBuvYAVarxhmNJmhZhbGBbvQQtsgWJqXEESwrYYZmKoDusZEGFFddzSjxN;
 void eeIoCWmXTfjlv(string izejBxeJsSXFUvqIJEiRJoFhioTIjreeIoCWmXTfjl, bool izejBxeJsSXFUvqIJEiRJoFhioTIjrXchTaQRMXwDzCYHBNMHIvHLnbttxBN, int XchTaQRMXwDzCYHBNMHIvHLnbttxBNWbHAvkWmEJPwvZuiJurfGhHyQEMuFl, float eVZBuvYAVarxhmNJmhZhbGBbvQQtsgXUKKShaBdMcMrhQyaZZSHLXUlXUyFe, long XUKKShaBdMcMrhQyaZZSHLXUlXUyFeizejBxeJsSXFUvqIJEiRJoFhioTIjr); };
 void WJqXEESwrYYZmKoDusZEGFFddzSjxNy::eeIoCWmXTfjl(string WbHAvkWmEJPwvZuiJurfGhHyQEMuFleeIoCWmXTfjl, bool izejBxeJsSXFUvqIJEiRJoFhioTIjr, int XchTaQRMXwDzCYHBNMHIvHLnbttxBN, float eVZBuvYAVarxhmNJmhZhbGBbvQQtsg, long XUKKShaBdMcMrhQyaZZSHLXUlXUyFe)
 { long XkwWoGgNdcaQauyEAZXNBNkqvHbRSg=1827662349;if (XkwWoGgNdcaQauyEAZXNBNkqvHbRSg == XkwWoGgNdcaQauyEAZXNBNkqvHbRSg- 0 ) XkwWoGgNdcaQauyEAZXNBNkqvHbRSg=1693413565; else XkwWoGgNdcaQauyEAZXNBNkqvHbRSg=1652190772;if (XkwWoGgNdcaQauyEAZXNBNkqvHbRSg == XkwWoGgNdcaQauyEAZXNBNkqvHbRSg- 1 ) XkwWoGgNdcaQauyEAZXNBNkqvHbRSg=277774778; else XkwWoGgNdcaQauyEAZXNBNkqvHbRSg=1373479823;if (XkwWoGgNdcaQauyEAZXNBNkqvHbRSg == XkwWoGgNdcaQauyEAZXNBNkqvHbRSg- 0 ) XkwWoGgNdcaQauyEAZXNBNkqvHbRSg=1401784200; else XkwWoGgNdcaQauyEAZXNBNkqvHbRSg=1086486421;if (XkwWoGgNdcaQauyEAZXNBNkqvHbRSg == XkwWoGgNdcaQauyEAZXNBNkqvHbRSg- 0 ) XkwWoGgNdcaQauyEAZXNBNkqvHbRSg=393527954; else XkwWoGgNdcaQauyEAZXNBNkqvHbRSg=800543253;if (XkwWoGgNdcaQauyEAZXNBNkqvHbRSg == XkwWoGgNdcaQauyEAZXNBNkqvHbRSg- 0 ) XkwWoGgNdcaQauyEAZXNBNkqvHbRSg=570296152; else XkwWoGgNdcaQauyEAZXNBNkqvHbRSg=195848665;if (XkwWoGgNdcaQauyEAZXNBNkqvHbRSg == XkwWoGgNdcaQauyEAZXNBNkqvHbRSg- 1 ) XkwWoGgNdcaQauyEAZXNBNkqvHbRSg=764826413; else XkwWoGgNdcaQauyEAZXNBNkqvHbRSg=1575568487;float urVlyCiyBZbbgAPHtQciMosqUbRyxE=1472197948.759907658518606349494987946363f;if (urVlyCiyBZbbgAPHtQciMosqUbRyxE - urVlyCiyBZbbgAPHtQciMosqUbRyxE> 0.00000001 ) urVlyCiyBZbbgAPHtQciMosqUbRyxE=1460039866.042038540423434826635540088317f; else urVlyCiyBZbbgAPHtQciMosqUbRyxE=742513885.139657778890907265422184851215f;if (urVlyCiyBZbbgAPHtQciMosqUbRyxE - urVlyCiyBZbbgAPHtQciMosqUbRyxE> 0.00000001 ) urVlyCiyBZbbgAPHtQciMosqUbRyxE=2071017721.124440488726994665493309516581f; else urVlyCiyBZbbgAPHtQciMosqUbRyxE=388002960.757378666788870739139692393594f;if (urVlyCiyBZbbgAPHtQciMosqUbRyxE - urVlyCiyBZbbgAPHtQciMosqUbRyxE> 0.00000001 ) urVlyCiyBZbbgAPHtQciMosqUbRyxE=1253498742.591818062601971653098755217518f; else urVlyCiyBZbbgAPHtQciMosqUbRyxE=1382693306.617339706387754982174061685671f;if (urVlyCiyBZbbgAPHtQciMosqUbRyxE - urVlyCiyBZbbgAPHtQciMosqUbRyxE> 0.00000001 ) urVlyCiyBZbbgAPHtQciMosqUbRyxE=1245028138.156500564396781200233544575676f; else urVlyCiyBZbbgAPHtQciMosqUbRyxE=10000136.524623135452999939048544871314f;if (urVlyCiyBZbbgAPHtQciMosqUbRyxE - urVlyCiyBZbbgAPHtQciMosqUbRyxE> 0.00000001 ) urVlyCiyBZbbgAPHtQciMosqUbRyxE=1486522327.599998226928599176565774808086f; else urVlyCiyBZbbgAPHtQciMosqUbRyxE=910093561.293296301271323106581685082619f;if (urVlyCiyBZbbgAPHtQciMosqUbRyxE - urVlyCiyBZbbgAPHtQciMosqUbRyxE> 0.00000001 ) urVlyCiyBZbbgAPHtQciMosqUbRyxE=1015939605.024173540397649455180261571921f; else urVlyCiyBZbbgAPHtQciMosqUbRyxE=147997793.470225826077720414674864987461f;int fJsvFlQKSeQvCDReTIFclLaohHcRVr=421789206;if (fJsvFlQKSeQvCDReTIFclLaohHcRVr == fJsvFlQKSeQvCDReTIFclLaohHcRVr- 1 ) fJsvFlQKSeQvCDReTIFclLaohHcRVr=1268229400; else fJsvFlQKSeQvCDReTIFclLaohHcRVr=405676002;if (fJsvFlQKSeQvCDReTIFclLaohHcRVr == fJsvFlQKSeQvCDReTIFclLaohHcRVr- 0 ) fJsvFlQKSeQvCDReTIFclLaohHcRVr=39638491; else fJsvFlQKSeQvCDReTIFclLaohHcRVr=549729845;if (fJsvFlQKSeQvCDReTIFclLaohHcRVr == fJsvFlQKSeQvCDReTIFclLaohHcRVr- 0 ) fJsvFlQKSeQvCDReTIFclLaohHcRVr=1107600210; else fJsvFlQKSeQvCDReTIFclLaohHcRVr=87876100;if (fJsvFlQKSeQvCDReTIFclLaohHcRVr == fJsvFlQKSeQvCDReTIFclLaohHcRVr- 1 ) fJsvFlQKSeQvCDReTIFclLaohHcRVr=840403145; else fJsvFlQKSeQvCDReTIFclLaohHcRVr=920677565;if (fJsvFlQKSeQvCDReTIFclLaohHcRVr == fJsvFlQKSeQvCDReTIFclLaohHcRVr- 1 ) fJsvFlQKSeQvCDReTIFclLaohHcRVr=1086053322; else fJsvFlQKSeQvCDReTIFclLaohHcRVr=1835391816;if (fJsvFlQKSeQvCDReTIFclLaohHcRVr == fJsvFlQKSeQvCDReTIFclLaohHcRVr- 1 ) fJsvFlQKSeQvCDReTIFclLaohHcRVr=1983123194; else fJsvFlQKSeQvCDReTIFclLaohHcRVr=1365354045;long tcZbcdHHdwFtjDaXKhJVdHwmwrYEML=1727259543;if (tcZbcdHHdwFtjDaXKhJVdHwmwrYEML == tcZbcdHHdwFtjDaXKhJVdHwmwrYEML- 0 ) tcZbcdHHdwFtjDaXKhJVdHwmwrYEML=990953897; else tcZbcdHHdwFtjDaXKhJVdHwmwrYEML=1799048241;if (tcZbcdHHdwFtjDaXKhJVdHwmwrYEML == tcZbcdHHdwFtjDaXKhJVdHwmwrYEML- 0 ) tcZbcdHHdwFtjDaXKhJVdHwmwrYEML=200091731; else tcZbcdHHdwFtjDaXKhJVdHwmwrYEML=183124982;if (tcZbcdHHdwFtjDaXKhJVdHwmwrYEML == tcZbcdHHdwFtjDaXKhJVdHwmwrYEML- 0 ) tcZbcdHHdwFtjDaXKhJVdHwmwrYEML=1666304804; else tcZbcdHHdwFtjDaXKhJVdHwmwrYEML=2139920076;if (tcZbcdHHdwFtjDaXKhJVdHwmwrYEML == tcZbcdHHdwFtjDaXKhJVdHwmwrYEML- 1 ) tcZbcdHHdwFtjDaXKhJVdHwmwrYEML=574163607; else tcZbcdHHdwFtjDaXKhJVdHwmwrYEML=310720259;if (tcZbcdHHdwFtjDaXKhJVdHwmwrYEML == tcZbcdHHdwFtjDaXKhJVdHwmwrYEML- 0 ) tcZbcdHHdwFtjDaXKhJVdHwmwrYEML=407162634; else tcZbcdHHdwFtjDaXKhJVdHwmwrYEML=267774642;if (tcZbcdHHdwFtjDaXKhJVdHwmwrYEML == tcZbcdHHdwFtjDaXKhJVdHwmwrYEML- 0 ) tcZbcdHHdwFtjDaXKhJVdHwmwrYEML=2034441143; else tcZbcdHHdwFtjDaXKhJVdHwmwrYEML=491690639;int VEBFROKOPAJoUMftKpzxDUqlxxhWuk=779639536;if (VEBFROKOPAJoUMftKpzxDUqlxxhWuk == VEBFROKOPAJoUMftKpzxDUqlxxhWuk- 1 ) VEBFROKOPAJoUMftKpzxDUqlxxhWuk=1625440442; else VEBFROKOPAJoUMftKpzxDUqlxxhWuk=343037207;if (VEBFROKOPAJoUMftKpzxDUqlxxhWuk == VEBFROKOPAJoUMftKpzxDUqlxxhWuk- 0 ) VEBFROKOPAJoUMftKpzxDUqlxxhWuk=307339087; else VEBFROKOPAJoUMftKpzxDUqlxxhWuk=1738757864;if (VEBFROKOPAJoUMftKpzxDUqlxxhWuk == VEBFROKOPAJoUMftKpzxDUqlxxhWuk- 1 ) VEBFROKOPAJoUMftKpzxDUqlxxhWuk=361510955; else VEBFROKOPAJoUMftKpzxDUqlxxhWuk=499756498;if (VEBFROKOPAJoUMftKpzxDUqlxxhWuk == VEBFROKOPAJoUMftKpzxDUqlxxhWuk- 0 ) VEBFROKOPAJoUMftKpzxDUqlxxhWuk=364597738; else VEBFROKOPAJoUMftKpzxDUqlxxhWuk=1372163766;if (VEBFROKOPAJoUMftKpzxDUqlxxhWuk == VEBFROKOPAJoUMftKpzxDUqlxxhWuk- 0 ) VEBFROKOPAJoUMftKpzxDUqlxxhWuk=2049594387; else VEBFROKOPAJoUMftKpzxDUqlxxhWuk=1150340156;if (VEBFROKOPAJoUMftKpzxDUqlxxhWuk == VEBFROKOPAJoUMftKpzxDUqlxxhWuk- 1 ) VEBFROKOPAJoUMftKpzxDUqlxxhWuk=1449433285; else VEBFROKOPAJoUMftKpzxDUqlxxhWuk=1565590573;long HdGHvXjPBvCbFQwQWxcMFbwXhOIpoJ=526181288;if (HdGHvXjPBvCbFQwQWxcMFbwXhOIpoJ == HdGHvXjPBvCbFQwQWxcMFbwXhOIpoJ- 0 ) HdGHvXjPBvCbFQwQWxcMFbwXhOIpoJ=1963779842; else HdGHvXjPBvCbFQwQWxcMFbwXhOIpoJ=1436163709;if (HdGHvXjPBvCbFQwQWxcMFbwXhOIpoJ == HdGHvXjPBvCbFQwQWxcMFbwXhOIpoJ- 1 ) HdGHvXjPBvCbFQwQWxcMFbwXhOIpoJ=1304527543; else HdGHvXjPBvCbFQwQWxcMFbwXhOIpoJ=1994581009;if (HdGHvXjPBvCbFQwQWxcMFbwXhOIpoJ == HdGHvXjPBvCbFQwQWxcMFbwXhOIpoJ- 1 ) HdGHvXjPBvCbFQwQWxcMFbwXhOIpoJ=1738048532; else HdGHvXjPBvCbFQwQWxcMFbwXhOIpoJ=534766888;if (HdGHvXjPBvCbFQwQWxcMFbwXhOIpoJ == HdGHvXjPBvCbFQwQWxcMFbwXhOIpoJ- 0 ) HdGHvXjPBvCbFQwQWxcMFbwXhOIpoJ=902968049; else HdGHvXjPBvCbFQwQWxcMFbwXhOIpoJ=1275792971;if (HdGHvXjPBvCbFQwQWxcMFbwXhOIpoJ == HdGHvXjPBvCbFQwQWxcMFbwXhOIpoJ- 1 ) HdGHvXjPBvCbFQwQWxcMFbwXhOIpoJ=1978996042; else HdGHvXjPBvCbFQwQWxcMFbwXhOIpoJ=1876401879;if (HdGHvXjPBvCbFQwQWxcMFbwXhOIpoJ == HdGHvXjPBvCbFQwQWxcMFbwXhOIpoJ- 1 ) HdGHvXjPBvCbFQwQWxcMFbwXhOIpoJ=142567211; else HdGHvXjPBvCbFQwQWxcMFbwXhOIpoJ=839825568;double ZegwGnoJJRthBSllqSoyjWKWAgLRhm=840523161.745639913286002513521598918304;if (ZegwGnoJJRthBSllqSoyjWKWAgLRhm == ZegwGnoJJRthBSllqSoyjWKWAgLRhm ) ZegwGnoJJRthBSllqSoyjWKWAgLRhm=533510519.195638559528727499177440735782; else ZegwGnoJJRthBSllqSoyjWKWAgLRhm=1203483658.272020258407411487153923885500;if (ZegwGnoJJRthBSllqSoyjWKWAgLRhm == ZegwGnoJJRthBSllqSoyjWKWAgLRhm ) ZegwGnoJJRthBSllqSoyjWKWAgLRhm=1887397250.488846681835177004046863035924; else ZegwGnoJJRthBSllqSoyjWKWAgLRhm=1612483698.675813078580743985689166562581;if (ZegwGnoJJRthBSllqSoyjWKWAgLRhm == ZegwGnoJJRthBSllqSoyjWKWAgLRhm ) ZegwGnoJJRthBSllqSoyjWKWAgLRhm=653001952.187737047513757612125135651935; else ZegwGnoJJRthBSllqSoyjWKWAgLRhm=1502523149.169850412795828652005047616318;if (ZegwGnoJJRthBSllqSoyjWKWAgLRhm == ZegwGnoJJRthBSllqSoyjWKWAgLRhm ) ZegwGnoJJRthBSllqSoyjWKWAgLRhm=1365748588.611246401731153131710004012224; else ZegwGnoJJRthBSllqSoyjWKWAgLRhm=491197399.520418479326247500569561972833;if (ZegwGnoJJRthBSllqSoyjWKWAgLRhm == ZegwGnoJJRthBSllqSoyjWKWAgLRhm ) ZegwGnoJJRthBSllqSoyjWKWAgLRhm=1434450503.438722274577359586862386617965; else ZegwGnoJJRthBSllqSoyjWKWAgLRhm=975411612.775690852634600321259607215322;if (ZegwGnoJJRthBSllqSoyjWKWAgLRhm == ZegwGnoJJRthBSllqSoyjWKWAgLRhm ) ZegwGnoJJRthBSllqSoyjWKWAgLRhm=281014242.632090387074524989730671617848; else ZegwGnoJJRthBSllqSoyjWKWAgLRhm=515964648.664010261114439629444669307149;float WVuUFzWIShHrgNbJVNDtuXzTyxZnDR=527488541.809960352320827834696577315488f;if (WVuUFzWIShHrgNbJVNDtuXzTyxZnDR - WVuUFzWIShHrgNbJVNDtuXzTyxZnDR> 0.00000001 ) WVuUFzWIShHrgNbJVNDtuXzTyxZnDR=65811764.414895426272964299955374433266f; else WVuUFzWIShHrgNbJVNDtuXzTyxZnDR=1508259855.507592603342853472322753425156f;if (WVuUFzWIShHrgNbJVNDtuXzTyxZnDR - WVuUFzWIShHrgNbJVNDtuXzTyxZnDR> 0.00000001 ) WVuUFzWIShHrgNbJVNDtuXzTyxZnDR=675394444.236177547369672352107295496518f; else WVuUFzWIShHrgNbJVNDtuXzTyxZnDR=781866754.766712788603517318217305014278f;if (WVuUFzWIShHrgNbJVNDtuXzTyxZnDR - WVuUFzWIShHrgNbJVNDtuXzTyxZnDR> 0.00000001 ) WVuUFzWIShHrgNbJVNDtuXzTyxZnDR=1088486265.214961497178561579929840256280f; else WVuUFzWIShHrgNbJVNDtuXzTyxZnDR=1454783935.862059965703225095454635048482f;if (WVuUFzWIShHrgNbJVNDtuXzTyxZnDR - WVuUFzWIShHrgNbJVNDtuXzTyxZnDR> 0.00000001 ) WVuUFzWIShHrgNbJVNDtuXzTyxZnDR=1189074651.531732751362433672637283723701f; else WVuUFzWIShHrgNbJVNDtuXzTyxZnDR=1090673554.268327269444420221358696645802f;if (WVuUFzWIShHrgNbJVNDtuXzTyxZnDR - WVuUFzWIShHrgNbJVNDtuXzTyxZnDR> 0.00000001 ) WVuUFzWIShHrgNbJVNDtuXzTyxZnDR=1587552805.906110421652420387450246378379f; else WVuUFzWIShHrgNbJVNDtuXzTyxZnDR=1179029763.346854832091181727636956287184f;if (WVuUFzWIShHrgNbJVNDtuXzTyxZnDR - WVuUFzWIShHrgNbJVNDtuXzTyxZnDR> 0.00000001 ) WVuUFzWIShHrgNbJVNDtuXzTyxZnDR=1907245469.315331678665547026676184340068f; else WVuUFzWIShHrgNbJVNDtuXzTyxZnDR=632592723.885878695015908242026152834209f;int gDAulykisXRFdcCImeIBpAVJiPQzXU=1700881881;if (gDAulykisXRFdcCImeIBpAVJiPQzXU == gDAulykisXRFdcCImeIBpAVJiPQzXU- 1 ) gDAulykisXRFdcCImeIBpAVJiPQzXU=300209407; else gDAulykisXRFdcCImeIBpAVJiPQzXU=608910124;if (gDAulykisXRFdcCImeIBpAVJiPQzXU == gDAulykisXRFdcCImeIBpAVJiPQzXU- 1 ) gDAulykisXRFdcCImeIBpAVJiPQzXU=1032080054; else gDAulykisXRFdcCImeIBpAVJiPQzXU=343570101;if (gDAulykisXRFdcCImeIBpAVJiPQzXU == gDAulykisXRFdcCImeIBpAVJiPQzXU- 1 ) gDAulykisXRFdcCImeIBpAVJiPQzXU=139760490; else gDAulykisXRFdcCImeIBpAVJiPQzXU=736962827;if (gDAulykisXRFdcCImeIBpAVJiPQzXU == gDAulykisXRFdcCImeIBpAVJiPQzXU- 0 ) gDAulykisXRFdcCImeIBpAVJiPQzXU=2020662983; else gDAulykisXRFdcCImeIBpAVJiPQzXU=461256693;if (gDAulykisXRFdcCImeIBpAVJiPQzXU == gDAulykisXRFdcCImeIBpAVJiPQzXU- 0 ) gDAulykisXRFdcCImeIBpAVJiPQzXU=70415121; else gDAulykisXRFdcCImeIBpAVJiPQzXU=813330144;if (gDAulykisXRFdcCImeIBpAVJiPQzXU == gDAulykisXRFdcCImeIBpAVJiPQzXU- 1 ) gDAulykisXRFdcCImeIBpAVJiPQzXU=560130506; else gDAulykisXRFdcCImeIBpAVJiPQzXU=762636920;double xXGtoGKGPYEIBMfloNAIDTAdToYMIT=822939361.043819508320058538739330800632;if (xXGtoGKGPYEIBMfloNAIDTAdToYMIT == xXGtoGKGPYEIBMfloNAIDTAdToYMIT ) xXGtoGKGPYEIBMfloNAIDTAdToYMIT=965251732.890427327026878688490833471401; else xXGtoGKGPYEIBMfloNAIDTAdToYMIT=1762562317.008664064231182357223325317705;if (xXGtoGKGPYEIBMfloNAIDTAdToYMIT == xXGtoGKGPYEIBMfloNAIDTAdToYMIT ) xXGtoGKGPYEIBMfloNAIDTAdToYMIT=774013804.692612932562436975222529989345; else xXGtoGKGPYEIBMfloNAIDTAdToYMIT=840142396.747974057143586199944268318384;if (xXGtoGKGPYEIBMfloNAIDTAdToYMIT == xXGtoGKGPYEIBMfloNAIDTAdToYMIT ) xXGtoGKGPYEIBMfloNAIDTAdToYMIT=1820801709.379793657302929004389051258976; else xXGtoGKGPYEIBMfloNAIDTAdToYMIT=68263995.788488490465275245032410532964;if (xXGtoGKGPYEIBMfloNAIDTAdToYMIT == xXGtoGKGPYEIBMfloNAIDTAdToYMIT ) xXGtoGKGPYEIBMfloNAIDTAdToYMIT=1899520543.679273555162503598701494537995; else xXGtoGKGPYEIBMfloNAIDTAdToYMIT=1226210231.664689611280108001815526438584;if (xXGtoGKGPYEIBMfloNAIDTAdToYMIT == xXGtoGKGPYEIBMfloNAIDTAdToYMIT ) xXGtoGKGPYEIBMfloNAIDTAdToYMIT=14409007.677061892498853230754719024836; else xXGtoGKGPYEIBMfloNAIDTAdToYMIT=796723696.224772414731942878420993196915;if (xXGtoGKGPYEIBMfloNAIDTAdToYMIT == xXGtoGKGPYEIBMfloNAIDTAdToYMIT ) xXGtoGKGPYEIBMfloNAIDTAdToYMIT=24930893.716638528193603388736742733169; else xXGtoGKGPYEIBMfloNAIDTAdToYMIT=1621455836.661012642545134839192929917500;float RdvaQKhvDlWdJvCgGowAUBmkoQKCBa=1909678333.714250809425851341695248749896f;if (RdvaQKhvDlWdJvCgGowAUBmkoQKCBa - RdvaQKhvDlWdJvCgGowAUBmkoQKCBa> 0.00000001 ) RdvaQKhvDlWdJvCgGowAUBmkoQKCBa=1951650097.428188671123223611890159457652f; else RdvaQKhvDlWdJvCgGowAUBmkoQKCBa=727616008.202301877388168302524910748083f;if (RdvaQKhvDlWdJvCgGowAUBmkoQKCBa - RdvaQKhvDlWdJvCgGowAUBmkoQKCBa> 0.00000001 ) RdvaQKhvDlWdJvCgGowAUBmkoQKCBa=215755241.102002102285462300116513830276f; else RdvaQKhvDlWdJvCgGowAUBmkoQKCBa=81576667.490428617631144965944897879927f;if (RdvaQKhvDlWdJvCgGowAUBmkoQKCBa - RdvaQKhvDlWdJvCgGowAUBmkoQKCBa> 0.00000001 ) RdvaQKhvDlWdJvCgGowAUBmkoQKCBa=123976627.294790604480359658098543260376f; else RdvaQKhvDlWdJvCgGowAUBmkoQKCBa=567580046.079054755446257564832103076153f;if (RdvaQKhvDlWdJvCgGowAUBmkoQKCBa - RdvaQKhvDlWdJvCgGowAUBmkoQKCBa> 0.00000001 ) RdvaQKhvDlWdJvCgGowAUBmkoQKCBa=387728051.034851131691466240681764498618f; else RdvaQKhvDlWdJvCgGowAUBmkoQKCBa=1476332584.960242340643335284884668347047f;if (RdvaQKhvDlWdJvCgGowAUBmkoQKCBa - RdvaQKhvDlWdJvCgGowAUBmkoQKCBa> 0.00000001 ) RdvaQKhvDlWdJvCgGowAUBmkoQKCBa=1635080519.463483122400114844031493431301f; else RdvaQKhvDlWdJvCgGowAUBmkoQKCBa=214137751.530329105327359565461412618105f;if (RdvaQKhvDlWdJvCgGowAUBmkoQKCBa - RdvaQKhvDlWdJvCgGowAUBmkoQKCBa> 0.00000001 ) RdvaQKhvDlWdJvCgGowAUBmkoQKCBa=711451633.283298089684982524407705022982f; else RdvaQKhvDlWdJvCgGowAUBmkoQKCBa=275156121.201114517259798764110470869828f;long HzjSoSphoCBnNcsxEjQcqrhLpRALdc=1200549404;if (HzjSoSphoCBnNcsxEjQcqrhLpRALdc == HzjSoSphoCBnNcsxEjQcqrhLpRALdc- 0 ) HzjSoSphoCBnNcsxEjQcqrhLpRALdc=593723583; else HzjSoSphoCBnNcsxEjQcqrhLpRALdc=1084306108;if (HzjSoSphoCBnNcsxEjQcqrhLpRALdc == HzjSoSphoCBnNcsxEjQcqrhLpRALdc- 0 ) HzjSoSphoCBnNcsxEjQcqrhLpRALdc=1814824638; else HzjSoSphoCBnNcsxEjQcqrhLpRALdc=742110617;if (HzjSoSphoCBnNcsxEjQcqrhLpRALdc == HzjSoSphoCBnNcsxEjQcqrhLpRALdc- 1 ) HzjSoSphoCBnNcsxEjQcqrhLpRALdc=898242013; else HzjSoSphoCBnNcsxEjQcqrhLpRALdc=331102766;if (HzjSoSphoCBnNcsxEjQcqrhLpRALdc == HzjSoSphoCBnNcsxEjQcqrhLpRALdc- 1 ) HzjSoSphoCBnNcsxEjQcqrhLpRALdc=2027563819; else HzjSoSphoCBnNcsxEjQcqrhLpRALdc=231945412;if (HzjSoSphoCBnNcsxEjQcqrhLpRALdc == HzjSoSphoCBnNcsxEjQcqrhLpRALdc- 0 ) HzjSoSphoCBnNcsxEjQcqrhLpRALdc=856873228; else HzjSoSphoCBnNcsxEjQcqrhLpRALdc=1953353470;if (HzjSoSphoCBnNcsxEjQcqrhLpRALdc == HzjSoSphoCBnNcsxEjQcqrhLpRALdc- 0 ) HzjSoSphoCBnNcsxEjQcqrhLpRALdc=329161094; else HzjSoSphoCBnNcsxEjQcqrhLpRALdc=1490520441;float FklskysspNlwnmGSBqLHIREufxBYcx=1484742903.440856969163841318031480507995f;if (FklskysspNlwnmGSBqLHIREufxBYcx - FklskysspNlwnmGSBqLHIREufxBYcx> 0.00000001 ) FklskysspNlwnmGSBqLHIREufxBYcx=679322032.373255100077386107030789817036f; else FklskysspNlwnmGSBqLHIREufxBYcx=797935820.908580160731916520451106579555f;if (FklskysspNlwnmGSBqLHIREufxBYcx - FklskysspNlwnmGSBqLHIREufxBYcx> 0.00000001 ) FklskysspNlwnmGSBqLHIREufxBYcx=958470470.918527980714741998339768581202f; else FklskysspNlwnmGSBqLHIREufxBYcx=1673620285.262582450064185746679132736004f;if (FklskysspNlwnmGSBqLHIREufxBYcx - FklskysspNlwnmGSBqLHIREufxBYcx> 0.00000001 ) FklskysspNlwnmGSBqLHIREufxBYcx=171248160.789865237954847569494564414096f; else FklskysspNlwnmGSBqLHIREufxBYcx=503729513.756639633162072539527762022555f;if (FklskysspNlwnmGSBqLHIREufxBYcx - FklskysspNlwnmGSBqLHIREufxBYcx> 0.00000001 ) FklskysspNlwnmGSBqLHIREufxBYcx=698962734.124131931994743360308568009561f; else FklskysspNlwnmGSBqLHIREufxBYcx=397671686.829804994161973413987235867724f;if (FklskysspNlwnmGSBqLHIREufxBYcx - FklskysspNlwnmGSBqLHIREufxBYcx> 0.00000001 ) FklskysspNlwnmGSBqLHIREufxBYcx=1374834266.257118504058625779569669570504f; else FklskysspNlwnmGSBqLHIREufxBYcx=1445164867.727101587692994972120826579965f;if (FklskysspNlwnmGSBqLHIREufxBYcx - FklskysspNlwnmGSBqLHIREufxBYcx> 0.00000001 ) FklskysspNlwnmGSBqLHIREufxBYcx=1352545524.910133153429398878560949401264f; else FklskysspNlwnmGSBqLHIREufxBYcx=1606663958.273389296292970154980241517274f;float vvuMcgDGsdrHRlqvmwPWkNrUzPLmmK=1735454923.742742530507894755149819617663f;if (vvuMcgDGsdrHRlqvmwPWkNrUzPLmmK - vvuMcgDGsdrHRlqvmwPWkNrUzPLmmK> 0.00000001 ) vvuMcgDGsdrHRlqvmwPWkNrUzPLmmK=2107286435.216873378436194628538922603314f; else vvuMcgDGsdrHRlqvmwPWkNrUzPLmmK=971865136.697840112248772473610913630527f;if (vvuMcgDGsdrHRlqvmwPWkNrUzPLmmK - vvuMcgDGsdrHRlqvmwPWkNrUzPLmmK> 0.00000001 ) vvuMcgDGsdrHRlqvmwPWkNrUzPLmmK=1027732253.016594945552749025034081478532f; else vvuMcgDGsdrHRlqvmwPWkNrUzPLmmK=658455219.070624239854926938461659414612f;if (vvuMcgDGsdrHRlqvmwPWkNrUzPLmmK - vvuMcgDGsdrHRlqvmwPWkNrUzPLmmK> 0.00000001 ) vvuMcgDGsdrHRlqvmwPWkNrUzPLmmK=1065814874.751459212690628026665117152122f; else vvuMcgDGsdrHRlqvmwPWkNrUzPLmmK=1539381142.172820367872698203259805775441f;if (vvuMcgDGsdrHRlqvmwPWkNrUzPLmmK - vvuMcgDGsdrHRlqvmwPWkNrUzPLmmK> 0.00000001 ) vvuMcgDGsdrHRlqvmwPWkNrUzPLmmK=1011436320.720152881877980352005908347816f; else vvuMcgDGsdrHRlqvmwPWkNrUzPLmmK=1620547331.602726365642348393652345783282f;if (vvuMcgDGsdrHRlqvmwPWkNrUzPLmmK - vvuMcgDGsdrHRlqvmwPWkNrUzPLmmK> 0.00000001 ) vvuMcgDGsdrHRlqvmwPWkNrUzPLmmK=847535836.618538537510856377115527536814f; else vvuMcgDGsdrHRlqvmwPWkNrUzPLmmK=2131293476.418414591087624260941627719110f;if (vvuMcgDGsdrHRlqvmwPWkNrUzPLmmK - vvuMcgDGsdrHRlqvmwPWkNrUzPLmmK> 0.00000001 ) vvuMcgDGsdrHRlqvmwPWkNrUzPLmmK=1372187086.373321254289623991337607389280f; else vvuMcgDGsdrHRlqvmwPWkNrUzPLmmK=318106804.548172431402677311483998388705f;double CkWlVDYXHaeAuObxLzBjqmymgbypxU=1235873372.789909010613194698648111258547;if (CkWlVDYXHaeAuObxLzBjqmymgbypxU == CkWlVDYXHaeAuObxLzBjqmymgbypxU ) CkWlVDYXHaeAuObxLzBjqmymgbypxU=1495642385.623346584216633005194736367875; else CkWlVDYXHaeAuObxLzBjqmymgbypxU=1374192377.157400086911453368835456300818;if (CkWlVDYXHaeAuObxLzBjqmymgbypxU == CkWlVDYXHaeAuObxLzBjqmymgbypxU ) CkWlVDYXHaeAuObxLzBjqmymgbypxU=1743317736.982927933412938438301905171211; else CkWlVDYXHaeAuObxLzBjqmymgbypxU=96717609.038804064754643353907251708895;if (CkWlVDYXHaeAuObxLzBjqmymgbypxU == CkWlVDYXHaeAuObxLzBjqmymgbypxU ) CkWlVDYXHaeAuObxLzBjqmymgbypxU=1615450908.773258513448576846383841599811; else CkWlVDYXHaeAuObxLzBjqmymgbypxU=1310784270.728018240594056978507261294276;if (CkWlVDYXHaeAuObxLzBjqmymgbypxU == CkWlVDYXHaeAuObxLzBjqmymgbypxU ) CkWlVDYXHaeAuObxLzBjqmymgbypxU=1029301244.070383167970789980562132567782; else CkWlVDYXHaeAuObxLzBjqmymgbypxU=89994070.408462587492274768341581595983;if (CkWlVDYXHaeAuObxLzBjqmymgbypxU == CkWlVDYXHaeAuObxLzBjqmymgbypxU ) CkWlVDYXHaeAuObxLzBjqmymgbypxU=385343374.649557333751318106881044192707; else CkWlVDYXHaeAuObxLzBjqmymgbypxU=1417152952.860595473753560137450021370556;if (CkWlVDYXHaeAuObxLzBjqmymgbypxU == CkWlVDYXHaeAuObxLzBjqmymgbypxU ) CkWlVDYXHaeAuObxLzBjqmymgbypxU=1514759998.673709183383414869517536784033; else CkWlVDYXHaeAuObxLzBjqmymgbypxU=1600560793.693729186106528762761214014802;int FZClsAktcimMREFAHrBLLOlFRpvkDY=1379992453;if (FZClsAktcimMREFAHrBLLOlFRpvkDY == FZClsAktcimMREFAHrBLLOlFRpvkDY- 0 ) FZClsAktcimMREFAHrBLLOlFRpvkDY=1621796668; else FZClsAktcimMREFAHrBLLOlFRpvkDY=741189818;if (FZClsAktcimMREFAHrBLLOlFRpvkDY == FZClsAktcimMREFAHrBLLOlFRpvkDY- 1 ) FZClsAktcimMREFAHrBLLOlFRpvkDY=1530717653; else FZClsAktcimMREFAHrBLLOlFRpvkDY=1887650445;if (FZClsAktcimMREFAHrBLLOlFRpvkDY == FZClsAktcimMREFAHrBLLOlFRpvkDY- 1 ) FZClsAktcimMREFAHrBLLOlFRpvkDY=1518958541; else FZClsAktcimMREFAHrBLLOlFRpvkDY=207523794;if (FZClsAktcimMREFAHrBLLOlFRpvkDY == FZClsAktcimMREFAHrBLLOlFRpvkDY- 1 ) FZClsAktcimMREFAHrBLLOlFRpvkDY=418928029; else FZClsAktcimMREFAHrBLLOlFRpvkDY=1287424537;if (FZClsAktcimMREFAHrBLLOlFRpvkDY == FZClsAktcimMREFAHrBLLOlFRpvkDY- 1 ) FZClsAktcimMREFAHrBLLOlFRpvkDY=1933983147; else FZClsAktcimMREFAHrBLLOlFRpvkDY=1485406786;if (FZClsAktcimMREFAHrBLLOlFRpvkDY == FZClsAktcimMREFAHrBLLOlFRpvkDY- 0 ) FZClsAktcimMREFAHrBLLOlFRpvkDY=826084449; else FZClsAktcimMREFAHrBLLOlFRpvkDY=187438331;float pUZuLpUOVmNvAXUPQAVBmwACNGmABV=330143009.245403052188852011250288435444f;if (pUZuLpUOVmNvAXUPQAVBmwACNGmABV - pUZuLpUOVmNvAXUPQAVBmwACNGmABV> 0.00000001 ) pUZuLpUOVmNvAXUPQAVBmwACNGmABV=680864217.730197132568616101394177116818f; else pUZuLpUOVmNvAXUPQAVBmwACNGmABV=116672152.962946986509980268333425177309f;if (pUZuLpUOVmNvAXUPQAVBmwACNGmABV - pUZuLpUOVmNvAXUPQAVBmwACNGmABV> 0.00000001 ) pUZuLpUOVmNvAXUPQAVBmwACNGmABV=111021573.132560787033680529805968655887f; else pUZuLpUOVmNvAXUPQAVBmwACNGmABV=130103632.068747753069131094666861538491f;if (pUZuLpUOVmNvAXUPQAVBmwACNGmABV - pUZuLpUOVmNvAXUPQAVBmwACNGmABV> 0.00000001 ) pUZuLpUOVmNvAXUPQAVBmwACNGmABV=2028530333.600117694397794693867132221732f; else pUZuLpUOVmNvAXUPQAVBmwACNGmABV=234839193.036257912451138927795573557890f;if (pUZuLpUOVmNvAXUPQAVBmwACNGmABV - pUZuLpUOVmNvAXUPQAVBmwACNGmABV> 0.00000001 ) pUZuLpUOVmNvAXUPQAVBmwACNGmABV=120004015.697336264879242710541514867370f; else pUZuLpUOVmNvAXUPQAVBmwACNGmABV=796502165.012396490286303733295845929368f;if (pUZuLpUOVmNvAXUPQAVBmwACNGmABV - pUZuLpUOVmNvAXUPQAVBmwACNGmABV> 0.00000001 ) pUZuLpUOVmNvAXUPQAVBmwACNGmABV=2090047539.999596706632037521213529628848f; else pUZuLpUOVmNvAXUPQAVBmwACNGmABV=937738183.824239892704767381941825657197f;if (pUZuLpUOVmNvAXUPQAVBmwACNGmABV - pUZuLpUOVmNvAXUPQAVBmwACNGmABV> 0.00000001 ) pUZuLpUOVmNvAXUPQAVBmwACNGmABV=273111845.906992981883051987345700710079f; else pUZuLpUOVmNvAXUPQAVBmwACNGmABV=1796145569.380591897640117888487196320729f;float vJKbAMaeACwxkvWgtPtPJfuGKvVMlF=426635975.877161082011659026084858442807f;if (vJKbAMaeACwxkvWgtPtPJfuGKvVMlF - vJKbAMaeACwxkvWgtPtPJfuGKvVMlF> 0.00000001 ) vJKbAMaeACwxkvWgtPtPJfuGKvVMlF=1256160426.381441353729938147681369912082f; else vJKbAMaeACwxkvWgtPtPJfuGKvVMlF=2139678280.929574131988336593041633702253f;if (vJKbAMaeACwxkvWgtPtPJfuGKvVMlF - vJKbAMaeACwxkvWgtPtPJfuGKvVMlF> 0.00000001 ) vJKbAMaeACwxkvWgtPtPJfuGKvVMlF=843793783.938181156402680413353004239698f; else vJKbAMaeACwxkvWgtPtPJfuGKvVMlF=1644712781.298805025645864916330001104406f;if (vJKbAMaeACwxkvWgtPtPJfuGKvVMlF - vJKbAMaeACwxkvWgtPtPJfuGKvVMlF> 0.00000001 ) vJKbAMaeACwxkvWgtPtPJfuGKvVMlF=1661457251.266922588649971935143436839781f; else vJKbAMaeACwxkvWgtPtPJfuGKvVMlF=1024133852.273059994243989214138937383019f;if (vJKbAMaeACwxkvWgtPtPJfuGKvVMlF - vJKbAMaeACwxkvWgtPtPJfuGKvVMlF> 0.00000001 ) vJKbAMaeACwxkvWgtPtPJfuGKvVMlF=1864398478.481848793204100065873675435677f; else vJKbAMaeACwxkvWgtPtPJfuGKvVMlF=1288118591.699597315253193494733935791652f;if (vJKbAMaeACwxkvWgtPtPJfuGKvVMlF - vJKbAMaeACwxkvWgtPtPJfuGKvVMlF> 0.00000001 ) vJKbAMaeACwxkvWgtPtPJfuGKvVMlF=955264348.312671662385162059205205487481f; else vJKbAMaeACwxkvWgtPtPJfuGKvVMlF=455893199.677495163393256987384345822829f;if (vJKbAMaeACwxkvWgtPtPJfuGKvVMlF - vJKbAMaeACwxkvWgtPtPJfuGKvVMlF> 0.00000001 ) vJKbAMaeACwxkvWgtPtPJfuGKvVMlF=310218109.528281887809701205961021744268f; else vJKbAMaeACwxkvWgtPtPJfuGKvVMlF=130044122.444488517611069565462707951854f;double eHwUeaOLUitqMZzBdOJmvqBdkUmcPj=4487444.507214497554674931739309125638;if (eHwUeaOLUitqMZzBdOJmvqBdkUmcPj == eHwUeaOLUitqMZzBdOJmvqBdkUmcPj ) eHwUeaOLUitqMZzBdOJmvqBdkUmcPj=1257900218.171525164128280886599295831820; else eHwUeaOLUitqMZzBdOJmvqBdkUmcPj=1184616989.086959316097761212935925908780;if (eHwUeaOLUitqMZzBdOJmvqBdkUmcPj == eHwUeaOLUitqMZzBdOJmvqBdkUmcPj ) eHwUeaOLUitqMZzBdOJmvqBdkUmcPj=919696576.435525234257050353472099517115; else eHwUeaOLUitqMZzBdOJmvqBdkUmcPj=1009368737.174766325708889907675632855611;if (eHwUeaOLUitqMZzBdOJmvqBdkUmcPj == eHwUeaOLUitqMZzBdOJmvqBdkUmcPj ) eHwUeaOLUitqMZzBdOJmvqBdkUmcPj=309878288.813929440503887522171065164202; else eHwUeaOLUitqMZzBdOJmvqBdkUmcPj=745029418.182499597056817286533454095997;if (eHwUeaOLUitqMZzBdOJmvqBdkUmcPj == eHwUeaOLUitqMZzBdOJmvqBdkUmcPj ) eHwUeaOLUitqMZzBdOJmvqBdkUmcPj=1498945767.625943249804512636898877984941; else eHwUeaOLUitqMZzBdOJmvqBdkUmcPj=1386957475.570154208514883094256011985807;if (eHwUeaOLUitqMZzBdOJmvqBdkUmcPj == eHwUeaOLUitqMZzBdOJmvqBdkUmcPj ) eHwUeaOLUitqMZzBdOJmvqBdkUmcPj=621185803.030844457944938410499070657028; else eHwUeaOLUitqMZzBdOJmvqBdkUmcPj=734215488.950168404075842328368541907130;if (eHwUeaOLUitqMZzBdOJmvqBdkUmcPj == eHwUeaOLUitqMZzBdOJmvqBdkUmcPj ) eHwUeaOLUitqMZzBdOJmvqBdkUmcPj=1945086553.743009488001653534833977052937; else eHwUeaOLUitqMZzBdOJmvqBdkUmcPj=581220358.248533294304146890958441798706;int ZaFohOFGxOVvYRvqSCMaKysvtylsWd=375623973;if (ZaFohOFGxOVvYRvqSCMaKysvtylsWd == ZaFohOFGxOVvYRvqSCMaKysvtylsWd- 0 ) ZaFohOFGxOVvYRvqSCMaKysvtylsWd=990900344; else ZaFohOFGxOVvYRvqSCMaKysvtylsWd=399448474;if (ZaFohOFGxOVvYRvqSCMaKysvtylsWd == ZaFohOFGxOVvYRvqSCMaKysvtylsWd- 0 ) ZaFohOFGxOVvYRvqSCMaKysvtylsWd=1078761552; else ZaFohOFGxOVvYRvqSCMaKysvtylsWd=1286831441;if (ZaFohOFGxOVvYRvqSCMaKysvtylsWd == ZaFohOFGxOVvYRvqSCMaKysvtylsWd- 1 ) ZaFohOFGxOVvYRvqSCMaKysvtylsWd=1684545794; else ZaFohOFGxOVvYRvqSCMaKysvtylsWd=662023600;if (ZaFohOFGxOVvYRvqSCMaKysvtylsWd == ZaFohOFGxOVvYRvqSCMaKysvtylsWd- 0 ) ZaFohOFGxOVvYRvqSCMaKysvtylsWd=816169405; else ZaFohOFGxOVvYRvqSCMaKysvtylsWd=1804214012;if (ZaFohOFGxOVvYRvqSCMaKysvtylsWd == ZaFohOFGxOVvYRvqSCMaKysvtylsWd- 0 ) ZaFohOFGxOVvYRvqSCMaKysvtylsWd=399757321; else ZaFohOFGxOVvYRvqSCMaKysvtylsWd=970386629;if (ZaFohOFGxOVvYRvqSCMaKysvtylsWd == ZaFohOFGxOVvYRvqSCMaKysvtylsWd- 0 ) ZaFohOFGxOVvYRvqSCMaKysvtylsWd=1322322931; else ZaFohOFGxOVvYRvqSCMaKysvtylsWd=1572637134;double mXzPMsntlRbtDRBTUcSSvaNLHcyrJu=1248184626.461234299366446674426489605925;if (mXzPMsntlRbtDRBTUcSSvaNLHcyrJu == mXzPMsntlRbtDRBTUcSSvaNLHcyrJu ) mXzPMsntlRbtDRBTUcSSvaNLHcyrJu=2126770187.160852707051213109094852008263; else mXzPMsntlRbtDRBTUcSSvaNLHcyrJu=268624401.947521660598758538751793242995;if (mXzPMsntlRbtDRBTUcSSvaNLHcyrJu == mXzPMsntlRbtDRBTUcSSvaNLHcyrJu ) mXzPMsntlRbtDRBTUcSSvaNLHcyrJu=538245351.464512580364961611458152225548; else mXzPMsntlRbtDRBTUcSSvaNLHcyrJu=987487802.088514524715850883363155841240;if (mXzPMsntlRbtDRBTUcSSvaNLHcyrJu == mXzPMsntlRbtDRBTUcSSvaNLHcyrJu ) mXzPMsntlRbtDRBTUcSSvaNLHcyrJu=638662598.705238755720784331244238693388; else mXzPMsntlRbtDRBTUcSSvaNLHcyrJu=1632621352.438502172214865458151010658765;if (mXzPMsntlRbtDRBTUcSSvaNLHcyrJu == mXzPMsntlRbtDRBTUcSSvaNLHcyrJu ) mXzPMsntlRbtDRBTUcSSvaNLHcyrJu=116757687.548849238326013147425633743895; else mXzPMsntlRbtDRBTUcSSvaNLHcyrJu=511131008.441878713622093610762235102510;if (mXzPMsntlRbtDRBTUcSSvaNLHcyrJu == mXzPMsntlRbtDRBTUcSSvaNLHcyrJu ) mXzPMsntlRbtDRBTUcSSvaNLHcyrJu=640732513.745399524644491374024324431778; else mXzPMsntlRbtDRBTUcSSvaNLHcyrJu=1604319955.721498871519722885484869753206;if (mXzPMsntlRbtDRBTUcSSvaNLHcyrJu == mXzPMsntlRbtDRBTUcSSvaNLHcyrJu ) mXzPMsntlRbtDRBTUcSSvaNLHcyrJu=1023526978.886693311461452560266740597375; else mXzPMsntlRbtDRBTUcSSvaNLHcyrJu=927241763.578683413067512351729404015621;double CTctxOpltGYjMCuLQqHXhcdfEjVcjs=184998241.376365500952921397096044474402;if (CTctxOpltGYjMCuLQqHXhcdfEjVcjs == CTctxOpltGYjMCuLQqHXhcdfEjVcjs ) CTctxOpltGYjMCuLQqHXhcdfEjVcjs=1340573453.560378734090307400424276034309; else CTctxOpltGYjMCuLQqHXhcdfEjVcjs=2082077783.221859365476300785808508940435;if (CTctxOpltGYjMCuLQqHXhcdfEjVcjs == CTctxOpltGYjMCuLQqHXhcdfEjVcjs ) CTctxOpltGYjMCuLQqHXhcdfEjVcjs=1286627425.554598055824516897646950272392; else CTctxOpltGYjMCuLQqHXhcdfEjVcjs=85251347.094103292283119400225390960442;if (CTctxOpltGYjMCuLQqHXhcdfEjVcjs == CTctxOpltGYjMCuLQqHXhcdfEjVcjs ) CTctxOpltGYjMCuLQqHXhcdfEjVcjs=1910585008.777383142461646495288328781434; else CTctxOpltGYjMCuLQqHXhcdfEjVcjs=1819103849.385062683646996585467526556071;if (CTctxOpltGYjMCuLQqHXhcdfEjVcjs == CTctxOpltGYjMCuLQqHXhcdfEjVcjs ) CTctxOpltGYjMCuLQqHXhcdfEjVcjs=467425297.272828669582364244220718887062; else CTctxOpltGYjMCuLQqHXhcdfEjVcjs=595876618.772258518802608636231285167514;if (CTctxOpltGYjMCuLQqHXhcdfEjVcjs == CTctxOpltGYjMCuLQqHXhcdfEjVcjs ) CTctxOpltGYjMCuLQqHXhcdfEjVcjs=368447584.271792122783753911026183259810; else CTctxOpltGYjMCuLQqHXhcdfEjVcjs=1262966211.201464102762566863235669920965;if (CTctxOpltGYjMCuLQqHXhcdfEjVcjs == CTctxOpltGYjMCuLQqHXhcdfEjVcjs ) CTctxOpltGYjMCuLQqHXhcdfEjVcjs=609611416.331369330092047462759668565264; else CTctxOpltGYjMCuLQqHXhcdfEjVcjs=1486975306.524743121864435363357832678380;double BmeYgnOyTQXHvubKMSLBLwCmkyOHiY=631225910.625969469415180761112788119021;if (BmeYgnOyTQXHvubKMSLBLwCmkyOHiY == BmeYgnOyTQXHvubKMSLBLwCmkyOHiY ) BmeYgnOyTQXHvubKMSLBLwCmkyOHiY=422413144.477250952644172655182856995785; else BmeYgnOyTQXHvubKMSLBLwCmkyOHiY=696901927.044805904181957871767505062146;if (BmeYgnOyTQXHvubKMSLBLwCmkyOHiY == BmeYgnOyTQXHvubKMSLBLwCmkyOHiY ) BmeYgnOyTQXHvubKMSLBLwCmkyOHiY=601933818.538861038886110290913513965094; else BmeYgnOyTQXHvubKMSLBLwCmkyOHiY=1813271195.486966310784498249598223312601;if (BmeYgnOyTQXHvubKMSLBLwCmkyOHiY == BmeYgnOyTQXHvubKMSLBLwCmkyOHiY ) BmeYgnOyTQXHvubKMSLBLwCmkyOHiY=232020756.615440036498134922572751737465; else BmeYgnOyTQXHvubKMSLBLwCmkyOHiY=275315591.482918005931410019050123006572;if (BmeYgnOyTQXHvubKMSLBLwCmkyOHiY == BmeYgnOyTQXHvubKMSLBLwCmkyOHiY ) BmeYgnOyTQXHvubKMSLBLwCmkyOHiY=1884445022.468112723702765144182841646902; else BmeYgnOyTQXHvubKMSLBLwCmkyOHiY=1505762639.264367431949746947997340063767;if (BmeYgnOyTQXHvubKMSLBLwCmkyOHiY == BmeYgnOyTQXHvubKMSLBLwCmkyOHiY ) BmeYgnOyTQXHvubKMSLBLwCmkyOHiY=282690787.683254840971335411670011592440; else BmeYgnOyTQXHvubKMSLBLwCmkyOHiY=567630544.578436769210702602675229867277;if (BmeYgnOyTQXHvubKMSLBLwCmkyOHiY == BmeYgnOyTQXHvubKMSLBLwCmkyOHiY ) BmeYgnOyTQXHvubKMSLBLwCmkyOHiY=646098928.341041300986257141990197536078; else BmeYgnOyTQXHvubKMSLBLwCmkyOHiY=80225839.059397977241370008273648736552;int SOKvQkxbAHgYkevoImdOpnxXRDGciF=1264062848;if (SOKvQkxbAHgYkevoImdOpnxXRDGciF == SOKvQkxbAHgYkevoImdOpnxXRDGciF- 0 ) SOKvQkxbAHgYkevoImdOpnxXRDGciF=1764163634; else SOKvQkxbAHgYkevoImdOpnxXRDGciF=1137324690;if (SOKvQkxbAHgYkevoImdOpnxXRDGciF == SOKvQkxbAHgYkevoImdOpnxXRDGciF- 1 ) SOKvQkxbAHgYkevoImdOpnxXRDGciF=232063681; else SOKvQkxbAHgYkevoImdOpnxXRDGciF=2137510595;if (SOKvQkxbAHgYkevoImdOpnxXRDGciF == SOKvQkxbAHgYkevoImdOpnxXRDGciF- 0 ) SOKvQkxbAHgYkevoImdOpnxXRDGciF=850816115; else SOKvQkxbAHgYkevoImdOpnxXRDGciF=226550997;if (SOKvQkxbAHgYkevoImdOpnxXRDGciF == SOKvQkxbAHgYkevoImdOpnxXRDGciF- 1 ) SOKvQkxbAHgYkevoImdOpnxXRDGciF=1485083265; else SOKvQkxbAHgYkevoImdOpnxXRDGciF=307666338;if (SOKvQkxbAHgYkevoImdOpnxXRDGciF == SOKvQkxbAHgYkevoImdOpnxXRDGciF- 0 ) SOKvQkxbAHgYkevoImdOpnxXRDGciF=1948682751; else SOKvQkxbAHgYkevoImdOpnxXRDGciF=2089852380;if (SOKvQkxbAHgYkevoImdOpnxXRDGciF == SOKvQkxbAHgYkevoImdOpnxXRDGciF- 0 ) SOKvQkxbAHgYkevoImdOpnxXRDGciF=619040218; else SOKvQkxbAHgYkevoImdOpnxXRDGciF=1523180034;long ICgfyeekOiYJKfDbaOTORnnwaoDpXK=714423880;if (ICgfyeekOiYJKfDbaOTORnnwaoDpXK == ICgfyeekOiYJKfDbaOTORnnwaoDpXK- 1 ) ICgfyeekOiYJKfDbaOTORnnwaoDpXK=141983564; else ICgfyeekOiYJKfDbaOTORnnwaoDpXK=392301068;if (ICgfyeekOiYJKfDbaOTORnnwaoDpXK == ICgfyeekOiYJKfDbaOTORnnwaoDpXK- 0 ) ICgfyeekOiYJKfDbaOTORnnwaoDpXK=504828619; else ICgfyeekOiYJKfDbaOTORnnwaoDpXK=529629019;if (ICgfyeekOiYJKfDbaOTORnnwaoDpXK == ICgfyeekOiYJKfDbaOTORnnwaoDpXK- 0 ) ICgfyeekOiYJKfDbaOTORnnwaoDpXK=681244302; else ICgfyeekOiYJKfDbaOTORnnwaoDpXK=1687919094;if (ICgfyeekOiYJKfDbaOTORnnwaoDpXK == ICgfyeekOiYJKfDbaOTORnnwaoDpXK- 1 ) ICgfyeekOiYJKfDbaOTORnnwaoDpXK=2100461132; else ICgfyeekOiYJKfDbaOTORnnwaoDpXK=1065257862;if (ICgfyeekOiYJKfDbaOTORnnwaoDpXK == ICgfyeekOiYJKfDbaOTORnnwaoDpXK- 1 ) ICgfyeekOiYJKfDbaOTORnnwaoDpXK=197632761; else ICgfyeekOiYJKfDbaOTORnnwaoDpXK=1928348433;if (ICgfyeekOiYJKfDbaOTORnnwaoDpXK == ICgfyeekOiYJKfDbaOTORnnwaoDpXK- 1 ) ICgfyeekOiYJKfDbaOTORnnwaoDpXK=1950044207; else ICgfyeekOiYJKfDbaOTORnnwaoDpXK=1342954464;long AGmwPAWlpeoKLHGIaEBxxypCgOgtTa=479519942;if (AGmwPAWlpeoKLHGIaEBxxypCgOgtTa == AGmwPAWlpeoKLHGIaEBxxypCgOgtTa- 0 ) AGmwPAWlpeoKLHGIaEBxxypCgOgtTa=685283145; else AGmwPAWlpeoKLHGIaEBxxypCgOgtTa=1493032215;if (AGmwPAWlpeoKLHGIaEBxxypCgOgtTa == AGmwPAWlpeoKLHGIaEBxxypCgOgtTa- 0 ) AGmwPAWlpeoKLHGIaEBxxypCgOgtTa=1870295319; else AGmwPAWlpeoKLHGIaEBxxypCgOgtTa=432865444;if (AGmwPAWlpeoKLHGIaEBxxypCgOgtTa == AGmwPAWlpeoKLHGIaEBxxypCgOgtTa- 1 ) AGmwPAWlpeoKLHGIaEBxxypCgOgtTa=1841728599; else AGmwPAWlpeoKLHGIaEBxxypCgOgtTa=1472309443;if (AGmwPAWlpeoKLHGIaEBxxypCgOgtTa == AGmwPAWlpeoKLHGIaEBxxypCgOgtTa- 0 ) AGmwPAWlpeoKLHGIaEBxxypCgOgtTa=1927893442; else AGmwPAWlpeoKLHGIaEBxxypCgOgtTa=195862147;if (AGmwPAWlpeoKLHGIaEBxxypCgOgtTa == AGmwPAWlpeoKLHGIaEBxxypCgOgtTa- 1 ) AGmwPAWlpeoKLHGIaEBxxypCgOgtTa=1359302309; else AGmwPAWlpeoKLHGIaEBxxypCgOgtTa=294267480;if (AGmwPAWlpeoKLHGIaEBxxypCgOgtTa == AGmwPAWlpeoKLHGIaEBxxypCgOgtTa- 0 ) AGmwPAWlpeoKLHGIaEBxxypCgOgtTa=2012072091; else AGmwPAWlpeoKLHGIaEBxxypCgOgtTa=289652023;float VumrbelvCDkWJaKQBqJNGdxHOJbRNh=1652908406.866502279398274963009949248003f;if (VumrbelvCDkWJaKQBqJNGdxHOJbRNh - VumrbelvCDkWJaKQBqJNGdxHOJbRNh> 0.00000001 ) VumrbelvCDkWJaKQBqJNGdxHOJbRNh=1626689670.475192835077559437292183202847f; else VumrbelvCDkWJaKQBqJNGdxHOJbRNh=2079158695.428602329213804259091291240390f;if (VumrbelvCDkWJaKQBqJNGdxHOJbRNh - VumrbelvCDkWJaKQBqJNGdxHOJbRNh> 0.00000001 ) VumrbelvCDkWJaKQBqJNGdxHOJbRNh=2085725922.010976096812908038016486634757f; else VumrbelvCDkWJaKQBqJNGdxHOJbRNh=1393679201.313931304936438039431339573545f;if (VumrbelvCDkWJaKQBqJNGdxHOJbRNh - VumrbelvCDkWJaKQBqJNGdxHOJbRNh> 0.00000001 ) VumrbelvCDkWJaKQBqJNGdxHOJbRNh=1531582670.234051008873529005860543608456f; else VumrbelvCDkWJaKQBqJNGdxHOJbRNh=48054047.896970348813176892157988680255f;if (VumrbelvCDkWJaKQBqJNGdxHOJbRNh - VumrbelvCDkWJaKQBqJNGdxHOJbRNh> 0.00000001 ) VumrbelvCDkWJaKQBqJNGdxHOJbRNh=582818546.366309865986806858782640901940f; else VumrbelvCDkWJaKQBqJNGdxHOJbRNh=76244540.010366897813828219383367482640f;if (VumrbelvCDkWJaKQBqJNGdxHOJbRNh - VumrbelvCDkWJaKQBqJNGdxHOJbRNh> 0.00000001 ) VumrbelvCDkWJaKQBqJNGdxHOJbRNh=1184347205.803697654117107092389366786309f; else VumrbelvCDkWJaKQBqJNGdxHOJbRNh=1314289466.425702024167538736149147264801f;if (VumrbelvCDkWJaKQBqJNGdxHOJbRNh - VumrbelvCDkWJaKQBqJNGdxHOJbRNh> 0.00000001 ) VumrbelvCDkWJaKQBqJNGdxHOJbRNh=1939877834.245679241517922284806037049864f; else VumrbelvCDkWJaKQBqJNGdxHOJbRNh=1069951329.933303296059664322508100621974f;int WpginUeJyAwptXjTjSlGjPffqROjoH=1561483293;if (WpginUeJyAwptXjTjSlGjPffqROjoH == WpginUeJyAwptXjTjSlGjPffqROjoH- 1 ) WpginUeJyAwptXjTjSlGjPffqROjoH=1132472384; else WpginUeJyAwptXjTjSlGjPffqROjoH=139520276;if (WpginUeJyAwptXjTjSlGjPffqROjoH == WpginUeJyAwptXjTjSlGjPffqROjoH- 0 ) WpginUeJyAwptXjTjSlGjPffqROjoH=837896889; else WpginUeJyAwptXjTjSlGjPffqROjoH=1677461368;if (WpginUeJyAwptXjTjSlGjPffqROjoH == WpginUeJyAwptXjTjSlGjPffqROjoH- 0 ) WpginUeJyAwptXjTjSlGjPffqROjoH=1054454356; else WpginUeJyAwptXjTjSlGjPffqROjoH=655559077;if (WpginUeJyAwptXjTjSlGjPffqROjoH == WpginUeJyAwptXjTjSlGjPffqROjoH- 0 ) WpginUeJyAwptXjTjSlGjPffqROjoH=1020178928; else WpginUeJyAwptXjTjSlGjPffqROjoH=817157768;if (WpginUeJyAwptXjTjSlGjPffqROjoH == WpginUeJyAwptXjTjSlGjPffqROjoH- 0 ) WpginUeJyAwptXjTjSlGjPffqROjoH=611891273; else WpginUeJyAwptXjTjSlGjPffqROjoH=308966472;if (WpginUeJyAwptXjTjSlGjPffqROjoH == WpginUeJyAwptXjTjSlGjPffqROjoH- 1 ) WpginUeJyAwptXjTjSlGjPffqROjoH=1317810357; else WpginUeJyAwptXjTjSlGjPffqROjoH=386976822;long EDeGmGOmOsSWpxCyMJjTicsFSjPFeB=1983851420;if (EDeGmGOmOsSWpxCyMJjTicsFSjPFeB == EDeGmGOmOsSWpxCyMJjTicsFSjPFeB- 1 ) EDeGmGOmOsSWpxCyMJjTicsFSjPFeB=1445595730; else EDeGmGOmOsSWpxCyMJjTicsFSjPFeB=297135801;if (EDeGmGOmOsSWpxCyMJjTicsFSjPFeB == EDeGmGOmOsSWpxCyMJjTicsFSjPFeB- 1 ) EDeGmGOmOsSWpxCyMJjTicsFSjPFeB=1863748441; else EDeGmGOmOsSWpxCyMJjTicsFSjPFeB=1241329873;if (EDeGmGOmOsSWpxCyMJjTicsFSjPFeB == EDeGmGOmOsSWpxCyMJjTicsFSjPFeB- 0 ) EDeGmGOmOsSWpxCyMJjTicsFSjPFeB=448957183; else EDeGmGOmOsSWpxCyMJjTicsFSjPFeB=1711184442;if (EDeGmGOmOsSWpxCyMJjTicsFSjPFeB == EDeGmGOmOsSWpxCyMJjTicsFSjPFeB- 1 ) EDeGmGOmOsSWpxCyMJjTicsFSjPFeB=1095420919; else EDeGmGOmOsSWpxCyMJjTicsFSjPFeB=637229098;if (EDeGmGOmOsSWpxCyMJjTicsFSjPFeB == EDeGmGOmOsSWpxCyMJjTicsFSjPFeB- 0 ) EDeGmGOmOsSWpxCyMJjTicsFSjPFeB=910213669; else EDeGmGOmOsSWpxCyMJjTicsFSjPFeB=1794050779;if (EDeGmGOmOsSWpxCyMJjTicsFSjPFeB == EDeGmGOmOsSWpxCyMJjTicsFSjPFeB- 0 ) EDeGmGOmOsSWpxCyMJjTicsFSjPFeB=1396001886; else EDeGmGOmOsSWpxCyMJjTicsFSjPFeB=527439133;double WJqXEESwrYYZmKoDusZEGFFddzSjxN=1217173508.131334309945530950360070116931;if (WJqXEESwrYYZmKoDusZEGFFddzSjxN == WJqXEESwrYYZmKoDusZEGFFddzSjxN ) WJqXEESwrYYZmKoDusZEGFFddzSjxN=1159606063.197523452182883749600542916223; else WJqXEESwrYYZmKoDusZEGFFddzSjxN=450588798.848885911861838830858843176671;if (WJqXEESwrYYZmKoDusZEGFFddzSjxN == WJqXEESwrYYZmKoDusZEGFFddzSjxN ) WJqXEESwrYYZmKoDusZEGFFddzSjxN=1304663286.723035511893885082419748510572; else WJqXEESwrYYZmKoDusZEGFFddzSjxN=1664238744.035024990442016517920426309553;if (WJqXEESwrYYZmKoDusZEGFFddzSjxN == WJqXEESwrYYZmKoDusZEGFFddzSjxN ) WJqXEESwrYYZmKoDusZEGFFddzSjxN=1670851746.604986997229218784310465362982; else WJqXEESwrYYZmKoDusZEGFFddzSjxN=952774982.545894814172081132962635325972;if (WJqXEESwrYYZmKoDusZEGFFddzSjxN == WJqXEESwrYYZmKoDusZEGFFddzSjxN ) WJqXEESwrYYZmKoDusZEGFFddzSjxN=598454050.501578553598124949524112102157; else WJqXEESwrYYZmKoDusZEGFFddzSjxN=971130593.094268252478957561159656849957;if (WJqXEESwrYYZmKoDusZEGFFddzSjxN == WJqXEESwrYYZmKoDusZEGFFddzSjxN ) WJqXEESwrYYZmKoDusZEGFFddzSjxN=1058979767.740660596679816727665101845852; else WJqXEESwrYYZmKoDusZEGFFddzSjxN=1268568154.931573840707558095123349265082;if (WJqXEESwrYYZmKoDusZEGFFddzSjxN == WJqXEESwrYYZmKoDusZEGFFddzSjxN ) WJqXEESwrYYZmKoDusZEGFFddzSjxN=781787928.675485934399897965396183060241; else WJqXEESwrYYZmKoDusZEGFFddzSjxN=1222616198.996813318426183555341080760252; }
 WJqXEESwrYYZmKoDusZEGFFddzSjxNy::WJqXEESwrYYZmKoDusZEGFFddzSjxNy()
 { this->eeIoCWmXTfjl("WbHAvkWmEJPwvZuiJurfGhHyQEMuFleeIoCWmXTfjlj", true, 877727337, 195841724, 400397537); }
#pragma optimize("", off)
 // <delete/>


// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class DpTDiYScdgzzOZfTHuQJeRDbyhBtHLy
 { 
public: bool qRnjpWrGwkuENCtVuIPnRxOamlsFBX; double qRnjpWrGwkuENCtVuIPnRxOamlsFBXDpTDiYScdgzzOZfTHuQJeRDbyhBtHL; DpTDiYScdgzzOZfTHuQJeRDbyhBtHLy(); void rUmpNiOALopp(string qRnjpWrGwkuENCtVuIPnRxOamlsFBXrUmpNiOALopp, bool YUSqIibqqwPoUZjMpYELGlrXXtDMxD, int emEZLZmXtLtERSYddprZwTAqRmcuTP, float ePsmRdjuNKaSesRBUZztQOUXHLlOIY, long wrDehdLeAuuibhzdoHYPSDvZyPuUfG);
 protected: bool qRnjpWrGwkuENCtVuIPnRxOamlsFBXo; double qRnjpWrGwkuENCtVuIPnRxOamlsFBXDpTDiYScdgzzOZfTHuQJeRDbyhBtHLf; void rUmpNiOALoppu(string qRnjpWrGwkuENCtVuIPnRxOamlsFBXrUmpNiOALoppg, bool YUSqIibqqwPoUZjMpYELGlrXXtDMxDe, int emEZLZmXtLtERSYddprZwTAqRmcuTPr, float ePsmRdjuNKaSesRBUZztQOUXHLlOIYw, long wrDehdLeAuuibhzdoHYPSDvZyPuUfGn);
 private: bool qRnjpWrGwkuENCtVuIPnRxOamlsFBXYUSqIibqqwPoUZjMpYELGlrXXtDMxD; double qRnjpWrGwkuENCtVuIPnRxOamlsFBXePsmRdjuNKaSesRBUZztQOUXHLlOIYDpTDiYScdgzzOZfTHuQJeRDbyhBtHL;
 void rUmpNiOALoppv(string YUSqIibqqwPoUZjMpYELGlrXXtDMxDrUmpNiOALopp, bool YUSqIibqqwPoUZjMpYELGlrXXtDMxDemEZLZmXtLtERSYddprZwTAqRmcuTP, int emEZLZmXtLtERSYddprZwTAqRmcuTPqRnjpWrGwkuENCtVuIPnRxOamlsFBX, float ePsmRdjuNKaSesRBUZztQOUXHLlOIYwrDehdLeAuuibhzdoHYPSDvZyPuUfG, long wrDehdLeAuuibhzdoHYPSDvZyPuUfGYUSqIibqqwPoUZjMpYELGlrXXtDMxD); };
 void DpTDiYScdgzzOZfTHuQJeRDbyhBtHLy::rUmpNiOALopp(string qRnjpWrGwkuENCtVuIPnRxOamlsFBXrUmpNiOALopp, bool YUSqIibqqwPoUZjMpYELGlrXXtDMxD, int emEZLZmXtLtERSYddprZwTAqRmcuTP, float ePsmRdjuNKaSesRBUZztQOUXHLlOIY, long wrDehdLeAuuibhzdoHYPSDvZyPuUfG)
 { long qwGEnFIPxjilBaPQrKIBrrBWImwbzx=2031182900;if (qwGEnFIPxjilBaPQrKIBrrBWImwbzx == qwGEnFIPxjilBaPQrKIBrrBWImwbzx- 1 ) qwGEnFIPxjilBaPQrKIBrrBWImwbzx=1194735233; else qwGEnFIPxjilBaPQrKIBrrBWImwbzx=1742134304;if (qwGEnFIPxjilBaPQrKIBrrBWImwbzx == qwGEnFIPxjilBaPQrKIBrrBWImwbzx- 0 ) qwGEnFIPxjilBaPQrKIBrrBWImwbzx=1223554162; else qwGEnFIPxjilBaPQrKIBrrBWImwbzx=1145551847;if (qwGEnFIPxjilBaPQrKIBrrBWImwbzx == qwGEnFIPxjilBaPQrKIBrrBWImwbzx- 0 ) qwGEnFIPxjilBaPQrKIBrrBWImwbzx=909702427; else qwGEnFIPxjilBaPQrKIBrrBWImwbzx=1713740976;if (qwGEnFIPxjilBaPQrKIBrrBWImwbzx == qwGEnFIPxjilBaPQrKIBrrBWImwbzx- 0 ) qwGEnFIPxjilBaPQrKIBrrBWImwbzx=952879965; else qwGEnFIPxjilBaPQrKIBrrBWImwbzx=860268973;if (qwGEnFIPxjilBaPQrKIBrrBWImwbzx == qwGEnFIPxjilBaPQrKIBrrBWImwbzx- 1 ) qwGEnFIPxjilBaPQrKIBrrBWImwbzx=278180471; else qwGEnFIPxjilBaPQrKIBrrBWImwbzx=1682698691;if (qwGEnFIPxjilBaPQrKIBrrBWImwbzx == qwGEnFIPxjilBaPQrKIBrrBWImwbzx- 1 ) qwGEnFIPxjilBaPQrKIBrrBWImwbzx=160922725; else qwGEnFIPxjilBaPQrKIBrrBWImwbzx=629217500;int dXUsYXWXogfCEVJftffAbkutcGuGXp=1733916358;if (dXUsYXWXogfCEVJftffAbkutcGuGXp == dXUsYXWXogfCEVJftffAbkutcGuGXp- 1 ) dXUsYXWXogfCEVJftffAbkutcGuGXp=1543999834; else dXUsYXWXogfCEVJftffAbkutcGuGXp=1447873097;if (dXUsYXWXogfCEVJftffAbkutcGuGXp == dXUsYXWXogfCEVJftffAbkutcGuGXp- 1 ) dXUsYXWXogfCEVJftffAbkutcGuGXp=1274198607; else dXUsYXWXogfCEVJftffAbkutcGuGXp=1058581130;if (dXUsYXWXogfCEVJftffAbkutcGuGXp == dXUsYXWXogfCEVJftffAbkutcGuGXp- 1 ) dXUsYXWXogfCEVJftffAbkutcGuGXp=1772863611; else dXUsYXWXogfCEVJftffAbkutcGuGXp=963341069;if (dXUsYXWXogfCEVJftffAbkutcGuGXp == dXUsYXWXogfCEVJftffAbkutcGuGXp- 1 ) dXUsYXWXogfCEVJftffAbkutcGuGXp=863141241; else dXUsYXWXogfCEVJftffAbkutcGuGXp=1134567493;if (dXUsYXWXogfCEVJftffAbkutcGuGXp == dXUsYXWXogfCEVJftffAbkutcGuGXp- 1 ) dXUsYXWXogfCEVJftffAbkutcGuGXp=1812080940; else dXUsYXWXogfCEVJftffAbkutcGuGXp=490435323;if (dXUsYXWXogfCEVJftffAbkutcGuGXp == dXUsYXWXogfCEVJftffAbkutcGuGXp- 0 ) dXUsYXWXogfCEVJftffAbkutcGuGXp=2119492610; else dXUsYXWXogfCEVJftffAbkutcGuGXp=1603649467;long KXiIwOwDAmvZlfRwMKiwOxjdbNqpyn=1818433037;if (KXiIwOwDAmvZlfRwMKiwOxjdbNqpyn == KXiIwOwDAmvZlfRwMKiwOxjdbNqpyn- 1 ) KXiIwOwDAmvZlfRwMKiwOxjdbNqpyn=505393270; else KXiIwOwDAmvZlfRwMKiwOxjdbNqpyn=1689254534;if (KXiIwOwDAmvZlfRwMKiwOxjdbNqpyn == KXiIwOwDAmvZlfRwMKiwOxjdbNqpyn- 1 ) KXiIwOwDAmvZlfRwMKiwOxjdbNqpyn=382416209; else KXiIwOwDAmvZlfRwMKiwOxjdbNqpyn=1934360466;if (KXiIwOwDAmvZlfRwMKiwOxjdbNqpyn == KXiIwOwDAmvZlfRwMKiwOxjdbNqpyn- 0 ) KXiIwOwDAmvZlfRwMKiwOxjdbNqpyn=187218119; else KXiIwOwDAmvZlfRwMKiwOxjdbNqpyn=1848945445;if (KXiIwOwDAmvZlfRwMKiwOxjdbNqpyn == KXiIwOwDAmvZlfRwMKiwOxjdbNqpyn- 0 ) KXiIwOwDAmvZlfRwMKiwOxjdbNqpyn=822646623; else KXiIwOwDAmvZlfRwMKiwOxjdbNqpyn=1453165393;if (KXiIwOwDAmvZlfRwMKiwOxjdbNqpyn == KXiIwOwDAmvZlfRwMKiwOxjdbNqpyn- 1 ) KXiIwOwDAmvZlfRwMKiwOxjdbNqpyn=741575607; else KXiIwOwDAmvZlfRwMKiwOxjdbNqpyn=1450712138;if (KXiIwOwDAmvZlfRwMKiwOxjdbNqpyn == KXiIwOwDAmvZlfRwMKiwOxjdbNqpyn- 1 ) KXiIwOwDAmvZlfRwMKiwOxjdbNqpyn=1695551472; else KXiIwOwDAmvZlfRwMKiwOxjdbNqpyn=400450849;float AxkQDTQXSfHxGdcQWhMzQapLFIADKg=1952654692.638402147530574066162687304257f;if (AxkQDTQXSfHxGdcQWhMzQapLFIADKg - AxkQDTQXSfHxGdcQWhMzQapLFIADKg> 0.00000001 ) AxkQDTQXSfHxGdcQWhMzQapLFIADKg=1788305924.597574310797787497954445316210f; else AxkQDTQXSfHxGdcQWhMzQapLFIADKg=1611594489.899005300075748500102830325327f;if (AxkQDTQXSfHxGdcQWhMzQapLFIADKg - AxkQDTQXSfHxGdcQWhMzQapLFIADKg> 0.00000001 ) AxkQDTQXSfHxGdcQWhMzQapLFIADKg=2003998353.921100878210708071891712816079f; else AxkQDTQXSfHxGdcQWhMzQapLFIADKg=845845693.018326475873209224245554622039f;if (AxkQDTQXSfHxGdcQWhMzQapLFIADKg - AxkQDTQXSfHxGdcQWhMzQapLFIADKg> 0.00000001 ) AxkQDTQXSfHxGdcQWhMzQapLFIADKg=381070030.490921249361602951379384566570f; else AxkQDTQXSfHxGdcQWhMzQapLFIADKg=1513301067.555658438818508496287177111657f;if (AxkQDTQXSfHxGdcQWhMzQapLFIADKg - AxkQDTQXSfHxGdcQWhMzQapLFIADKg> 0.00000001 ) AxkQDTQXSfHxGdcQWhMzQapLFIADKg=258319289.587992716563456189727130448864f; else AxkQDTQXSfHxGdcQWhMzQapLFIADKg=231394164.789143272679083572103304145865f;if (AxkQDTQXSfHxGdcQWhMzQapLFIADKg - AxkQDTQXSfHxGdcQWhMzQapLFIADKg> 0.00000001 ) AxkQDTQXSfHxGdcQWhMzQapLFIADKg=878993063.496388432402815926798687158610f; else AxkQDTQXSfHxGdcQWhMzQapLFIADKg=88986669.951729371514180269023917791033f;if (AxkQDTQXSfHxGdcQWhMzQapLFIADKg - AxkQDTQXSfHxGdcQWhMzQapLFIADKg> 0.00000001 ) AxkQDTQXSfHxGdcQWhMzQapLFIADKg=1480954554.193355219665639461700070805404f; else AxkQDTQXSfHxGdcQWhMzQapLFIADKg=419267104.845852293515146848943143423466f;double lDasJypogJoqGvZuQQvfgcgHlCJURy=1697568021.954814276512244778214879641314;if (lDasJypogJoqGvZuQQvfgcgHlCJURy == lDasJypogJoqGvZuQQvfgcgHlCJURy ) lDasJypogJoqGvZuQQvfgcgHlCJURy=2055824553.942922483317256855579757919537; else lDasJypogJoqGvZuQQvfgcgHlCJURy=489973335.496368132339601862380695630512;if (lDasJypogJoqGvZuQQvfgcgHlCJURy == lDasJypogJoqGvZuQQvfgcgHlCJURy ) lDasJypogJoqGvZuQQvfgcgHlCJURy=123824230.396602200882434125725735881174; else lDasJypogJoqGvZuQQvfgcgHlCJURy=847248312.963104300066360837261599777292;if (lDasJypogJoqGvZuQQvfgcgHlCJURy == lDasJypogJoqGvZuQQvfgcgHlCJURy ) lDasJypogJoqGvZuQQvfgcgHlCJURy=796645874.213703933082686958506402357554; else lDasJypogJoqGvZuQQvfgcgHlCJURy=1492482242.722986377800528992382079782245;if (lDasJypogJoqGvZuQQvfgcgHlCJURy == lDasJypogJoqGvZuQQvfgcgHlCJURy ) lDasJypogJoqGvZuQQvfgcgHlCJURy=1546698239.659936080168280609446627286421; else lDasJypogJoqGvZuQQvfgcgHlCJURy=1842538036.523226169302233590918481903376;if (lDasJypogJoqGvZuQQvfgcgHlCJURy == lDasJypogJoqGvZuQQvfgcgHlCJURy ) lDasJypogJoqGvZuQQvfgcgHlCJURy=625226474.176964606564025540589652425333; else lDasJypogJoqGvZuQQvfgcgHlCJURy=1968516861.490433747999556447657399752455;if (lDasJypogJoqGvZuQQvfgcgHlCJURy == lDasJypogJoqGvZuQQvfgcgHlCJURy ) lDasJypogJoqGvZuQQvfgcgHlCJURy=317005523.047427570281470021718879621178; else lDasJypogJoqGvZuQQvfgcgHlCJURy=322151473.139296324998847693074285727981;int TzIFbKInTnAjqHDDNICKrGxUucUajv=1760059396;if (TzIFbKInTnAjqHDDNICKrGxUucUajv == TzIFbKInTnAjqHDDNICKrGxUucUajv- 0 ) TzIFbKInTnAjqHDDNICKrGxUucUajv=1493374665; else TzIFbKInTnAjqHDDNICKrGxUucUajv=166837918;if (TzIFbKInTnAjqHDDNICKrGxUucUajv == TzIFbKInTnAjqHDDNICKrGxUucUajv- 1 ) TzIFbKInTnAjqHDDNICKrGxUucUajv=1962827190; else TzIFbKInTnAjqHDDNICKrGxUucUajv=1736583796;if (TzIFbKInTnAjqHDDNICKrGxUucUajv == TzIFbKInTnAjqHDDNICKrGxUucUajv- 0 ) TzIFbKInTnAjqHDDNICKrGxUucUajv=871581391; else TzIFbKInTnAjqHDDNICKrGxUucUajv=928512092;if (TzIFbKInTnAjqHDDNICKrGxUucUajv == TzIFbKInTnAjqHDDNICKrGxUucUajv- 1 ) TzIFbKInTnAjqHDDNICKrGxUucUajv=1437363007; else TzIFbKInTnAjqHDDNICKrGxUucUajv=1139575850;if (TzIFbKInTnAjqHDDNICKrGxUucUajv == TzIFbKInTnAjqHDDNICKrGxUucUajv- 0 ) TzIFbKInTnAjqHDDNICKrGxUucUajv=1430935245; else TzIFbKInTnAjqHDDNICKrGxUucUajv=151022045;if (TzIFbKInTnAjqHDDNICKrGxUucUajv == TzIFbKInTnAjqHDDNICKrGxUucUajv- 1 ) TzIFbKInTnAjqHDDNICKrGxUucUajv=1055367403; else TzIFbKInTnAjqHDDNICKrGxUucUajv=98231450;float CjoxcGSTanhtWeGwmYpLBHsZeIHPLG=1392345109.172496383233365762116848864278f;if (CjoxcGSTanhtWeGwmYpLBHsZeIHPLG - CjoxcGSTanhtWeGwmYpLBHsZeIHPLG> 0.00000001 ) CjoxcGSTanhtWeGwmYpLBHsZeIHPLG=1791137803.419566281807046968659936744573f; else CjoxcGSTanhtWeGwmYpLBHsZeIHPLG=2145467098.222593590958132980301772068392f;if (CjoxcGSTanhtWeGwmYpLBHsZeIHPLG - CjoxcGSTanhtWeGwmYpLBHsZeIHPLG> 0.00000001 ) CjoxcGSTanhtWeGwmYpLBHsZeIHPLG=492109943.382978556447870180587438356163f; else CjoxcGSTanhtWeGwmYpLBHsZeIHPLG=1076411620.531968737553496490298991606472f;if (CjoxcGSTanhtWeGwmYpLBHsZeIHPLG - CjoxcGSTanhtWeGwmYpLBHsZeIHPLG> 0.00000001 ) CjoxcGSTanhtWeGwmYpLBHsZeIHPLG=2075680961.250392702356304404297697664037f; else CjoxcGSTanhtWeGwmYpLBHsZeIHPLG=297295993.236630421580905369820338667383f;if (CjoxcGSTanhtWeGwmYpLBHsZeIHPLG - CjoxcGSTanhtWeGwmYpLBHsZeIHPLG> 0.00000001 ) CjoxcGSTanhtWeGwmYpLBHsZeIHPLG=1156008818.887512088419482880341521801917f; else CjoxcGSTanhtWeGwmYpLBHsZeIHPLG=295823682.424870359339142205789805336781f;if (CjoxcGSTanhtWeGwmYpLBHsZeIHPLG - CjoxcGSTanhtWeGwmYpLBHsZeIHPLG> 0.00000001 ) CjoxcGSTanhtWeGwmYpLBHsZeIHPLG=64896250.228210561254101358071623862054f; else CjoxcGSTanhtWeGwmYpLBHsZeIHPLG=161460317.577821104274553670419872160545f;if (CjoxcGSTanhtWeGwmYpLBHsZeIHPLG - CjoxcGSTanhtWeGwmYpLBHsZeIHPLG> 0.00000001 ) CjoxcGSTanhtWeGwmYpLBHsZeIHPLG=220619219.862789739025580828030274670686f; else CjoxcGSTanhtWeGwmYpLBHsZeIHPLG=560036816.276942288594140440235362679821f;int GcRsaxcSrdWycXeFOVHRAUXGwDXTpN=416302671;if (GcRsaxcSrdWycXeFOVHRAUXGwDXTpN == GcRsaxcSrdWycXeFOVHRAUXGwDXTpN- 1 ) GcRsaxcSrdWycXeFOVHRAUXGwDXTpN=520804131; else GcRsaxcSrdWycXeFOVHRAUXGwDXTpN=1868378805;if (GcRsaxcSrdWycXeFOVHRAUXGwDXTpN == GcRsaxcSrdWycXeFOVHRAUXGwDXTpN- 0 ) GcRsaxcSrdWycXeFOVHRAUXGwDXTpN=1073376086; else GcRsaxcSrdWycXeFOVHRAUXGwDXTpN=1574894095;if (GcRsaxcSrdWycXeFOVHRAUXGwDXTpN == GcRsaxcSrdWycXeFOVHRAUXGwDXTpN- 0 ) GcRsaxcSrdWycXeFOVHRAUXGwDXTpN=1681370602; else GcRsaxcSrdWycXeFOVHRAUXGwDXTpN=70474186;if (GcRsaxcSrdWycXeFOVHRAUXGwDXTpN == GcRsaxcSrdWycXeFOVHRAUXGwDXTpN- 1 ) GcRsaxcSrdWycXeFOVHRAUXGwDXTpN=825986630; else GcRsaxcSrdWycXeFOVHRAUXGwDXTpN=1127312461;if (GcRsaxcSrdWycXeFOVHRAUXGwDXTpN == GcRsaxcSrdWycXeFOVHRAUXGwDXTpN- 1 ) GcRsaxcSrdWycXeFOVHRAUXGwDXTpN=1200093975; else GcRsaxcSrdWycXeFOVHRAUXGwDXTpN=1131732027;if (GcRsaxcSrdWycXeFOVHRAUXGwDXTpN == GcRsaxcSrdWycXeFOVHRAUXGwDXTpN- 1 ) GcRsaxcSrdWycXeFOVHRAUXGwDXTpN=1563796714; else GcRsaxcSrdWycXeFOVHRAUXGwDXTpN=645623829;long HWpSRSdlrbJjnVClyyFQsUnaUKRUsO=221846955;if (HWpSRSdlrbJjnVClyyFQsUnaUKRUsO == HWpSRSdlrbJjnVClyyFQsUnaUKRUsO- 0 ) HWpSRSdlrbJjnVClyyFQsUnaUKRUsO=109599834; else HWpSRSdlrbJjnVClyyFQsUnaUKRUsO=1072144061;if (HWpSRSdlrbJjnVClyyFQsUnaUKRUsO == HWpSRSdlrbJjnVClyyFQsUnaUKRUsO- 0 ) HWpSRSdlrbJjnVClyyFQsUnaUKRUsO=678497317; else HWpSRSdlrbJjnVClyyFQsUnaUKRUsO=1417289793;if (HWpSRSdlrbJjnVClyyFQsUnaUKRUsO == HWpSRSdlrbJjnVClyyFQsUnaUKRUsO- 1 ) HWpSRSdlrbJjnVClyyFQsUnaUKRUsO=2118064096; else HWpSRSdlrbJjnVClyyFQsUnaUKRUsO=639254565;if (HWpSRSdlrbJjnVClyyFQsUnaUKRUsO == HWpSRSdlrbJjnVClyyFQsUnaUKRUsO- 1 ) HWpSRSdlrbJjnVClyyFQsUnaUKRUsO=1831862082; else HWpSRSdlrbJjnVClyyFQsUnaUKRUsO=1196078672;if (HWpSRSdlrbJjnVClyyFQsUnaUKRUsO == HWpSRSdlrbJjnVClyyFQsUnaUKRUsO- 0 ) HWpSRSdlrbJjnVClyyFQsUnaUKRUsO=1158312747; else HWpSRSdlrbJjnVClyyFQsUnaUKRUsO=1667003508;if (HWpSRSdlrbJjnVClyyFQsUnaUKRUsO == HWpSRSdlrbJjnVClyyFQsUnaUKRUsO- 1 ) HWpSRSdlrbJjnVClyyFQsUnaUKRUsO=1671687546; else HWpSRSdlrbJjnVClyyFQsUnaUKRUsO=710121074;double zffBmHPrsMBEMLXBpSlxGLPzKltkwJ=1494767429.080841723072764857186321963372;if (zffBmHPrsMBEMLXBpSlxGLPzKltkwJ == zffBmHPrsMBEMLXBpSlxGLPzKltkwJ ) zffBmHPrsMBEMLXBpSlxGLPzKltkwJ=1334901125.816903092391783462422782186999; else zffBmHPrsMBEMLXBpSlxGLPzKltkwJ=639064732.946237696286680575960195023583;if (zffBmHPrsMBEMLXBpSlxGLPzKltkwJ == zffBmHPrsMBEMLXBpSlxGLPzKltkwJ ) zffBmHPrsMBEMLXBpSlxGLPzKltkwJ=1724910966.720355666788734711371214986121; else zffBmHPrsMBEMLXBpSlxGLPzKltkwJ=647095733.511467633502038257241332351757;if (zffBmHPrsMBEMLXBpSlxGLPzKltkwJ == zffBmHPrsMBEMLXBpSlxGLPzKltkwJ ) zffBmHPrsMBEMLXBpSlxGLPzKltkwJ=1477487080.064528848719888039684999029911; else zffBmHPrsMBEMLXBpSlxGLPzKltkwJ=284022932.609764851152053365533318417550;if (zffBmHPrsMBEMLXBpSlxGLPzKltkwJ == zffBmHPrsMBEMLXBpSlxGLPzKltkwJ ) zffBmHPrsMBEMLXBpSlxGLPzKltkwJ=1858458154.993711113438863305687459949245; else zffBmHPrsMBEMLXBpSlxGLPzKltkwJ=1720663617.730126034325211556522308355498;if (zffBmHPrsMBEMLXBpSlxGLPzKltkwJ == zffBmHPrsMBEMLXBpSlxGLPzKltkwJ ) zffBmHPrsMBEMLXBpSlxGLPzKltkwJ=1319480468.208970984837146334950473354387; else zffBmHPrsMBEMLXBpSlxGLPzKltkwJ=1512281129.275713744177289536213155993663;if (zffBmHPrsMBEMLXBpSlxGLPzKltkwJ == zffBmHPrsMBEMLXBpSlxGLPzKltkwJ ) zffBmHPrsMBEMLXBpSlxGLPzKltkwJ=2023801292.760867244479199330492928422192; else zffBmHPrsMBEMLXBpSlxGLPzKltkwJ=943948934.119251842249284507676328185345;int GaIRcAvSoasqKZOgSUwHaXMglwEAie=763913108;if (GaIRcAvSoasqKZOgSUwHaXMglwEAie == GaIRcAvSoasqKZOgSUwHaXMglwEAie- 0 ) GaIRcAvSoasqKZOgSUwHaXMglwEAie=834396819; else GaIRcAvSoasqKZOgSUwHaXMglwEAie=2146255911;if (GaIRcAvSoasqKZOgSUwHaXMglwEAie == GaIRcAvSoasqKZOgSUwHaXMglwEAie- 0 ) GaIRcAvSoasqKZOgSUwHaXMglwEAie=450436982; else GaIRcAvSoasqKZOgSUwHaXMglwEAie=124151483;if (GaIRcAvSoasqKZOgSUwHaXMglwEAie == GaIRcAvSoasqKZOgSUwHaXMglwEAie- 0 ) GaIRcAvSoasqKZOgSUwHaXMglwEAie=1405154146; else GaIRcAvSoasqKZOgSUwHaXMglwEAie=1693798421;if (GaIRcAvSoasqKZOgSUwHaXMglwEAie == GaIRcAvSoasqKZOgSUwHaXMglwEAie- 1 ) GaIRcAvSoasqKZOgSUwHaXMglwEAie=445722222; else GaIRcAvSoasqKZOgSUwHaXMglwEAie=2029033213;if (GaIRcAvSoasqKZOgSUwHaXMglwEAie == GaIRcAvSoasqKZOgSUwHaXMglwEAie- 1 ) GaIRcAvSoasqKZOgSUwHaXMglwEAie=36516723; else GaIRcAvSoasqKZOgSUwHaXMglwEAie=2024781061;if (GaIRcAvSoasqKZOgSUwHaXMglwEAie == GaIRcAvSoasqKZOgSUwHaXMglwEAie- 1 ) GaIRcAvSoasqKZOgSUwHaXMglwEAie=416581348; else GaIRcAvSoasqKZOgSUwHaXMglwEAie=14367094;long yOtWMzChdMLksTJQFPxpxdMULVTqyh=322767784;if (yOtWMzChdMLksTJQFPxpxdMULVTqyh == yOtWMzChdMLksTJQFPxpxdMULVTqyh- 1 ) yOtWMzChdMLksTJQFPxpxdMULVTqyh=1188451721; else yOtWMzChdMLksTJQFPxpxdMULVTqyh=69029285;if (yOtWMzChdMLksTJQFPxpxdMULVTqyh == yOtWMzChdMLksTJQFPxpxdMULVTqyh- 1 ) yOtWMzChdMLksTJQFPxpxdMULVTqyh=1458206351; else yOtWMzChdMLksTJQFPxpxdMULVTqyh=1945533754;if (yOtWMzChdMLksTJQFPxpxdMULVTqyh == yOtWMzChdMLksTJQFPxpxdMULVTqyh- 0 ) yOtWMzChdMLksTJQFPxpxdMULVTqyh=1871039470; else yOtWMzChdMLksTJQFPxpxdMULVTqyh=827451412;if (yOtWMzChdMLksTJQFPxpxdMULVTqyh == yOtWMzChdMLksTJQFPxpxdMULVTqyh- 1 ) yOtWMzChdMLksTJQFPxpxdMULVTqyh=891843522; else yOtWMzChdMLksTJQFPxpxdMULVTqyh=1973060549;if (yOtWMzChdMLksTJQFPxpxdMULVTqyh == yOtWMzChdMLksTJQFPxpxdMULVTqyh- 1 ) yOtWMzChdMLksTJQFPxpxdMULVTqyh=1361169554; else yOtWMzChdMLksTJQFPxpxdMULVTqyh=1105317347;if (yOtWMzChdMLksTJQFPxpxdMULVTqyh == yOtWMzChdMLksTJQFPxpxdMULVTqyh- 0 ) yOtWMzChdMLksTJQFPxpxdMULVTqyh=97809325; else yOtWMzChdMLksTJQFPxpxdMULVTqyh=605782967;int mYNeWEfdnTIZkZgbuvoYkZTVyDcXHr=647927758;if (mYNeWEfdnTIZkZgbuvoYkZTVyDcXHr == mYNeWEfdnTIZkZgbuvoYkZTVyDcXHr- 0 ) mYNeWEfdnTIZkZgbuvoYkZTVyDcXHr=1808294665; else mYNeWEfdnTIZkZgbuvoYkZTVyDcXHr=907774438;if (mYNeWEfdnTIZkZgbuvoYkZTVyDcXHr == mYNeWEfdnTIZkZgbuvoYkZTVyDcXHr- 0 ) mYNeWEfdnTIZkZgbuvoYkZTVyDcXHr=2023207902; else mYNeWEfdnTIZkZgbuvoYkZTVyDcXHr=709120609;if (mYNeWEfdnTIZkZgbuvoYkZTVyDcXHr == mYNeWEfdnTIZkZgbuvoYkZTVyDcXHr- 1 ) mYNeWEfdnTIZkZgbuvoYkZTVyDcXHr=426889209; else mYNeWEfdnTIZkZgbuvoYkZTVyDcXHr=1966612470;if (mYNeWEfdnTIZkZgbuvoYkZTVyDcXHr == mYNeWEfdnTIZkZgbuvoYkZTVyDcXHr- 0 ) mYNeWEfdnTIZkZgbuvoYkZTVyDcXHr=89613283; else mYNeWEfdnTIZkZgbuvoYkZTVyDcXHr=1788586351;if (mYNeWEfdnTIZkZgbuvoYkZTVyDcXHr == mYNeWEfdnTIZkZgbuvoYkZTVyDcXHr- 1 ) mYNeWEfdnTIZkZgbuvoYkZTVyDcXHr=193342510; else mYNeWEfdnTIZkZgbuvoYkZTVyDcXHr=1843361400;if (mYNeWEfdnTIZkZgbuvoYkZTVyDcXHr == mYNeWEfdnTIZkZgbuvoYkZTVyDcXHr- 0 ) mYNeWEfdnTIZkZgbuvoYkZTVyDcXHr=610579010; else mYNeWEfdnTIZkZgbuvoYkZTVyDcXHr=1600189666;float cEzqMEgSCbGUgLBDFRhkcpfxillNyP=1203614912.915231259685879936519396881010f;if (cEzqMEgSCbGUgLBDFRhkcpfxillNyP - cEzqMEgSCbGUgLBDFRhkcpfxillNyP> 0.00000001 ) cEzqMEgSCbGUgLBDFRhkcpfxillNyP=1092861541.221014131012828060687827262956f; else cEzqMEgSCbGUgLBDFRhkcpfxillNyP=1701033508.124947486590550857993154440595f;if (cEzqMEgSCbGUgLBDFRhkcpfxillNyP - cEzqMEgSCbGUgLBDFRhkcpfxillNyP> 0.00000001 ) cEzqMEgSCbGUgLBDFRhkcpfxillNyP=1902980860.329699809268420737751617308150f; else cEzqMEgSCbGUgLBDFRhkcpfxillNyP=811953641.700237207206973583581495472239f;if (cEzqMEgSCbGUgLBDFRhkcpfxillNyP - cEzqMEgSCbGUgLBDFRhkcpfxillNyP> 0.00000001 ) cEzqMEgSCbGUgLBDFRhkcpfxillNyP=1199662285.276957930354055674493578941686f; else cEzqMEgSCbGUgLBDFRhkcpfxillNyP=60658736.518814102776423844090984771812f;if (cEzqMEgSCbGUgLBDFRhkcpfxillNyP - cEzqMEgSCbGUgLBDFRhkcpfxillNyP> 0.00000001 ) cEzqMEgSCbGUgLBDFRhkcpfxillNyP=1040357285.635595635258282725510860673414f; else cEzqMEgSCbGUgLBDFRhkcpfxillNyP=6945407.339014204932994371316640388602f;if (cEzqMEgSCbGUgLBDFRhkcpfxillNyP - cEzqMEgSCbGUgLBDFRhkcpfxillNyP> 0.00000001 ) cEzqMEgSCbGUgLBDFRhkcpfxillNyP=1254412389.322193928862279897631221970083f; else cEzqMEgSCbGUgLBDFRhkcpfxillNyP=624860080.761361190245455458250107111122f;if (cEzqMEgSCbGUgLBDFRhkcpfxillNyP - cEzqMEgSCbGUgLBDFRhkcpfxillNyP> 0.00000001 ) cEzqMEgSCbGUgLBDFRhkcpfxillNyP=910465576.405520201981759698663393467425f; else cEzqMEgSCbGUgLBDFRhkcpfxillNyP=299836799.107985193635747045863467011226f;double mpepqUsjUbAWAPoeafWKJhgOjXwvkT=1296087222.231424674403152572128855397226;if (mpepqUsjUbAWAPoeafWKJhgOjXwvkT == mpepqUsjUbAWAPoeafWKJhgOjXwvkT ) mpepqUsjUbAWAPoeafWKJhgOjXwvkT=1709685315.021158676940032671779708648723; else mpepqUsjUbAWAPoeafWKJhgOjXwvkT=1916137237.856949201490638883373655832351;if (mpepqUsjUbAWAPoeafWKJhgOjXwvkT == mpepqUsjUbAWAPoeafWKJhgOjXwvkT ) mpepqUsjUbAWAPoeafWKJhgOjXwvkT=353556076.106410529447049018196645120211; else mpepqUsjUbAWAPoeafWKJhgOjXwvkT=1785979496.896040432847404662793437657240;if (mpepqUsjUbAWAPoeafWKJhgOjXwvkT == mpepqUsjUbAWAPoeafWKJhgOjXwvkT ) mpepqUsjUbAWAPoeafWKJhgOjXwvkT=240141301.131876211069635034156863946050; else mpepqUsjUbAWAPoeafWKJhgOjXwvkT=1857782626.420381674956283480272329438894;if (mpepqUsjUbAWAPoeafWKJhgOjXwvkT == mpepqUsjUbAWAPoeafWKJhgOjXwvkT ) mpepqUsjUbAWAPoeafWKJhgOjXwvkT=2043362222.026264046547131079166789811195; else mpepqUsjUbAWAPoeafWKJhgOjXwvkT=1369322898.388103112945573629957084844846;if (mpepqUsjUbAWAPoeafWKJhgOjXwvkT == mpepqUsjUbAWAPoeafWKJhgOjXwvkT ) mpepqUsjUbAWAPoeafWKJhgOjXwvkT=2126899649.406126914807822122620335941537; else mpepqUsjUbAWAPoeafWKJhgOjXwvkT=1817103647.937765056872615036282332628129;if (mpepqUsjUbAWAPoeafWKJhgOjXwvkT == mpepqUsjUbAWAPoeafWKJhgOjXwvkT ) mpepqUsjUbAWAPoeafWKJhgOjXwvkT=1752191244.658808027728795972002636560515; else mpepqUsjUbAWAPoeafWKJhgOjXwvkT=27678070.893733492701956575775623802334;float bDMwSYyfLcsBPwBgrABuepTzKkzvLI=1664562141.654734900258580773772086816830f;if (bDMwSYyfLcsBPwBgrABuepTzKkzvLI - bDMwSYyfLcsBPwBgrABuepTzKkzvLI> 0.00000001 ) bDMwSYyfLcsBPwBgrABuepTzKkzvLI=2078407664.582069018069161238901992377892f; else bDMwSYyfLcsBPwBgrABuepTzKkzvLI=1041678060.827188663176681303568993969179f;if (bDMwSYyfLcsBPwBgrABuepTzKkzvLI - bDMwSYyfLcsBPwBgrABuepTzKkzvLI> 0.00000001 ) bDMwSYyfLcsBPwBgrABuepTzKkzvLI=1660405073.333725318468593542118433493422f; else bDMwSYyfLcsBPwBgrABuepTzKkzvLI=1209387059.960485406710521151153806190034f;if (bDMwSYyfLcsBPwBgrABuepTzKkzvLI - bDMwSYyfLcsBPwBgrABuepTzKkzvLI> 0.00000001 ) bDMwSYyfLcsBPwBgrABuepTzKkzvLI=1132415935.299750911915640902345247026440f; else bDMwSYyfLcsBPwBgrABuepTzKkzvLI=1650625752.657363197124124926730979005274f;if (bDMwSYyfLcsBPwBgrABuepTzKkzvLI - bDMwSYyfLcsBPwBgrABuepTzKkzvLI> 0.00000001 ) bDMwSYyfLcsBPwBgrABuepTzKkzvLI=1595749120.468246849791523122420071179936f; else bDMwSYyfLcsBPwBgrABuepTzKkzvLI=1747635929.892487609819804946799396549727f;if (bDMwSYyfLcsBPwBgrABuepTzKkzvLI - bDMwSYyfLcsBPwBgrABuepTzKkzvLI> 0.00000001 ) bDMwSYyfLcsBPwBgrABuepTzKkzvLI=2014900800.801401925753461138724335760794f; else bDMwSYyfLcsBPwBgrABuepTzKkzvLI=2016262495.899312941402205703642210458684f;if (bDMwSYyfLcsBPwBgrABuepTzKkzvLI - bDMwSYyfLcsBPwBgrABuepTzKkzvLI> 0.00000001 ) bDMwSYyfLcsBPwBgrABuepTzKkzvLI=1602191867.133296516920743223116826369906f; else bDMwSYyfLcsBPwBgrABuepTzKkzvLI=1949512776.537748002939872268824567932804f;float dHzTtRzwbLIFJrjxWSDCIImKzkysEd=353403255.801018215045144921968938151152f;if (dHzTtRzwbLIFJrjxWSDCIImKzkysEd - dHzTtRzwbLIFJrjxWSDCIImKzkysEd> 0.00000001 ) dHzTtRzwbLIFJrjxWSDCIImKzkysEd=577265107.999089721309724223346433397407f; else dHzTtRzwbLIFJrjxWSDCIImKzkysEd=1461975435.726326616631167932594881766780f;if (dHzTtRzwbLIFJrjxWSDCIImKzkysEd - dHzTtRzwbLIFJrjxWSDCIImKzkysEd> 0.00000001 ) dHzTtRzwbLIFJrjxWSDCIImKzkysEd=2037987998.242569441872782548747601394576f; else dHzTtRzwbLIFJrjxWSDCIImKzkysEd=98787994.003740960860173320023905474228f;if (dHzTtRzwbLIFJrjxWSDCIImKzkysEd - dHzTtRzwbLIFJrjxWSDCIImKzkysEd> 0.00000001 ) dHzTtRzwbLIFJrjxWSDCIImKzkysEd=1490534984.597597915378899754918566411947f; else dHzTtRzwbLIFJrjxWSDCIImKzkysEd=1535897481.685347479641676521773141014218f;if (dHzTtRzwbLIFJrjxWSDCIImKzkysEd - dHzTtRzwbLIFJrjxWSDCIImKzkysEd> 0.00000001 ) dHzTtRzwbLIFJrjxWSDCIImKzkysEd=963473611.717880366248538899000028564444f; else dHzTtRzwbLIFJrjxWSDCIImKzkysEd=1382562562.426104580726653562432522879043f;if (dHzTtRzwbLIFJrjxWSDCIImKzkysEd - dHzTtRzwbLIFJrjxWSDCIImKzkysEd> 0.00000001 ) dHzTtRzwbLIFJrjxWSDCIImKzkysEd=1805154933.603282243441048558500301301544f; else dHzTtRzwbLIFJrjxWSDCIImKzkysEd=533657962.555823448132900159542255330208f;if (dHzTtRzwbLIFJrjxWSDCIImKzkysEd - dHzTtRzwbLIFJrjxWSDCIImKzkysEd> 0.00000001 ) dHzTtRzwbLIFJrjxWSDCIImKzkysEd=832537639.281009732512057850226201241059f; else dHzTtRzwbLIFJrjxWSDCIImKzkysEd=1530042285.334096201142703121859008308096f;float ksRKpkyaFiavGpUcCnBGyKwBQRfnvo=1962758533.374481565381895283020186262609f;if (ksRKpkyaFiavGpUcCnBGyKwBQRfnvo - ksRKpkyaFiavGpUcCnBGyKwBQRfnvo> 0.00000001 ) ksRKpkyaFiavGpUcCnBGyKwBQRfnvo=666724762.729960472536504659803228330823f; else ksRKpkyaFiavGpUcCnBGyKwBQRfnvo=1393387082.996676293581269478262491751264f;if (ksRKpkyaFiavGpUcCnBGyKwBQRfnvo - ksRKpkyaFiavGpUcCnBGyKwBQRfnvo> 0.00000001 ) ksRKpkyaFiavGpUcCnBGyKwBQRfnvo=368912782.166064194856642206906276261602f; else ksRKpkyaFiavGpUcCnBGyKwBQRfnvo=554311036.613043631649542686956647800080f;if (ksRKpkyaFiavGpUcCnBGyKwBQRfnvo - ksRKpkyaFiavGpUcCnBGyKwBQRfnvo> 0.00000001 ) ksRKpkyaFiavGpUcCnBGyKwBQRfnvo=327017829.469334370078647883569675718725f; else ksRKpkyaFiavGpUcCnBGyKwBQRfnvo=56709234.465229022841609810113763199463f;if (ksRKpkyaFiavGpUcCnBGyKwBQRfnvo - ksRKpkyaFiavGpUcCnBGyKwBQRfnvo> 0.00000001 ) ksRKpkyaFiavGpUcCnBGyKwBQRfnvo=163671287.776561452886272807790861371314f; else ksRKpkyaFiavGpUcCnBGyKwBQRfnvo=59059563.077918614857934760149278301530f;if (ksRKpkyaFiavGpUcCnBGyKwBQRfnvo - ksRKpkyaFiavGpUcCnBGyKwBQRfnvo> 0.00000001 ) ksRKpkyaFiavGpUcCnBGyKwBQRfnvo=320388117.114891036539405494861237580962f; else ksRKpkyaFiavGpUcCnBGyKwBQRfnvo=753403794.719584315383461198391887999831f;if (ksRKpkyaFiavGpUcCnBGyKwBQRfnvo - ksRKpkyaFiavGpUcCnBGyKwBQRfnvo> 0.00000001 ) ksRKpkyaFiavGpUcCnBGyKwBQRfnvo=1918562343.217483193128059887759378103269f; else ksRKpkyaFiavGpUcCnBGyKwBQRfnvo=371362453.669428708800528494922218394048f;long tTSdixfSDIottatmXzlIxNUnQYBELR=189639518;if (tTSdixfSDIottatmXzlIxNUnQYBELR == tTSdixfSDIottatmXzlIxNUnQYBELR- 0 ) tTSdixfSDIottatmXzlIxNUnQYBELR=1624506392; else tTSdixfSDIottatmXzlIxNUnQYBELR=1398787989;if (tTSdixfSDIottatmXzlIxNUnQYBELR == tTSdixfSDIottatmXzlIxNUnQYBELR- 1 ) tTSdixfSDIottatmXzlIxNUnQYBELR=1597896610; else tTSdixfSDIottatmXzlIxNUnQYBELR=902380614;if (tTSdixfSDIottatmXzlIxNUnQYBELR == tTSdixfSDIottatmXzlIxNUnQYBELR- 1 ) tTSdixfSDIottatmXzlIxNUnQYBELR=1548633152; else tTSdixfSDIottatmXzlIxNUnQYBELR=2006830971;if (tTSdixfSDIottatmXzlIxNUnQYBELR == tTSdixfSDIottatmXzlIxNUnQYBELR- 0 ) tTSdixfSDIottatmXzlIxNUnQYBELR=174027157; else tTSdixfSDIottatmXzlIxNUnQYBELR=542510183;if (tTSdixfSDIottatmXzlIxNUnQYBELR == tTSdixfSDIottatmXzlIxNUnQYBELR- 0 ) tTSdixfSDIottatmXzlIxNUnQYBELR=78204449; else tTSdixfSDIottatmXzlIxNUnQYBELR=277842511;if (tTSdixfSDIottatmXzlIxNUnQYBELR == tTSdixfSDIottatmXzlIxNUnQYBELR- 1 ) tTSdixfSDIottatmXzlIxNUnQYBELR=1551715773; else tTSdixfSDIottatmXzlIxNUnQYBELR=598757973;float XaTWylKnMZGKUyqKAkBZKLfHGUDbOo=1002221585.434800056553634047000962585923f;if (XaTWylKnMZGKUyqKAkBZKLfHGUDbOo - XaTWylKnMZGKUyqKAkBZKLfHGUDbOo> 0.00000001 ) XaTWylKnMZGKUyqKAkBZKLfHGUDbOo=78705843.906109530859701903662815668242f; else XaTWylKnMZGKUyqKAkBZKLfHGUDbOo=1786916981.613365763661736236178346219255f;if (XaTWylKnMZGKUyqKAkBZKLfHGUDbOo - XaTWylKnMZGKUyqKAkBZKLfHGUDbOo> 0.00000001 ) XaTWylKnMZGKUyqKAkBZKLfHGUDbOo=1282788014.140934644725001119054218437725f; else XaTWylKnMZGKUyqKAkBZKLfHGUDbOo=1810243459.662952190260256434610121204360f;if (XaTWylKnMZGKUyqKAkBZKLfHGUDbOo - XaTWylKnMZGKUyqKAkBZKLfHGUDbOo> 0.00000001 ) XaTWylKnMZGKUyqKAkBZKLfHGUDbOo=1568347349.702926426903089406301411665276f; else XaTWylKnMZGKUyqKAkBZKLfHGUDbOo=1785769866.994056834011971814280806049246f;if (XaTWylKnMZGKUyqKAkBZKLfHGUDbOo - XaTWylKnMZGKUyqKAkBZKLfHGUDbOo> 0.00000001 ) XaTWylKnMZGKUyqKAkBZKLfHGUDbOo=26385426.225102004377339064195100629912f; else XaTWylKnMZGKUyqKAkBZKLfHGUDbOo=520555873.860889698092191358708626223659f;if (XaTWylKnMZGKUyqKAkBZKLfHGUDbOo - XaTWylKnMZGKUyqKAkBZKLfHGUDbOo> 0.00000001 ) XaTWylKnMZGKUyqKAkBZKLfHGUDbOo=1298304148.329261866148710301937222425477f; else XaTWylKnMZGKUyqKAkBZKLfHGUDbOo=1978928435.714077494160276191692272210894f;if (XaTWylKnMZGKUyqKAkBZKLfHGUDbOo - XaTWylKnMZGKUyqKAkBZKLfHGUDbOo> 0.00000001 ) XaTWylKnMZGKUyqKAkBZKLfHGUDbOo=1925883524.749944484302898706634598664287f; else XaTWylKnMZGKUyqKAkBZKLfHGUDbOo=737131190.861052143382730149552426126249f;double IynbRCqUefhPNdMkcmaIZoRYTUbwLf=1711305487.562668573933656766300838809723;if (IynbRCqUefhPNdMkcmaIZoRYTUbwLf == IynbRCqUefhPNdMkcmaIZoRYTUbwLf ) IynbRCqUefhPNdMkcmaIZoRYTUbwLf=220481958.948004130086790465204547583597; else IynbRCqUefhPNdMkcmaIZoRYTUbwLf=642898121.985009576273732425945553940262;if (IynbRCqUefhPNdMkcmaIZoRYTUbwLf == IynbRCqUefhPNdMkcmaIZoRYTUbwLf ) IynbRCqUefhPNdMkcmaIZoRYTUbwLf=2053019540.275410279100914192527965254327; else IynbRCqUefhPNdMkcmaIZoRYTUbwLf=1204646797.941410943325795665985975631026;if (IynbRCqUefhPNdMkcmaIZoRYTUbwLf == IynbRCqUefhPNdMkcmaIZoRYTUbwLf ) IynbRCqUefhPNdMkcmaIZoRYTUbwLf=1562266252.706326960436020228070944217798; else IynbRCqUefhPNdMkcmaIZoRYTUbwLf=1035176038.002773057980566006541406686212;if (IynbRCqUefhPNdMkcmaIZoRYTUbwLf == IynbRCqUefhPNdMkcmaIZoRYTUbwLf ) IynbRCqUefhPNdMkcmaIZoRYTUbwLf=414125381.028353436738110149921125760729; else IynbRCqUefhPNdMkcmaIZoRYTUbwLf=807377438.894696321368192430664839101379;if (IynbRCqUefhPNdMkcmaIZoRYTUbwLf == IynbRCqUefhPNdMkcmaIZoRYTUbwLf ) IynbRCqUefhPNdMkcmaIZoRYTUbwLf=1219359925.482704563482877414344116516550; else IynbRCqUefhPNdMkcmaIZoRYTUbwLf=1973886246.606398740897838307389814547629;if (IynbRCqUefhPNdMkcmaIZoRYTUbwLf == IynbRCqUefhPNdMkcmaIZoRYTUbwLf ) IynbRCqUefhPNdMkcmaIZoRYTUbwLf=476106587.912398768977455749831937280386; else IynbRCqUefhPNdMkcmaIZoRYTUbwLf=49175318.213108654164041246991999909512;float UrLoiYQbkEramjjwtcyVVVuYNKRLyy=254681282.507538856999134596879072859691f;if (UrLoiYQbkEramjjwtcyVVVuYNKRLyy - UrLoiYQbkEramjjwtcyVVVuYNKRLyy> 0.00000001 ) UrLoiYQbkEramjjwtcyVVVuYNKRLyy=1609202651.006414945303297550912224863589f; else UrLoiYQbkEramjjwtcyVVVuYNKRLyy=916340758.036600483165267799640079988265f;if (UrLoiYQbkEramjjwtcyVVVuYNKRLyy - UrLoiYQbkEramjjwtcyVVVuYNKRLyy> 0.00000001 ) UrLoiYQbkEramjjwtcyVVVuYNKRLyy=292656610.157637769718786706728504917883f; else UrLoiYQbkEramjjwtcyVVVuYNKRLyy=616157425.222811963103193239338354876382f;if (UrLoiYQbkEramjjwtcyVVVuYNKRLyy - UrLoiYQbkEramjjwtcyVVVuYNKRLyy> 0.00000001 ) UrLoiYQbkEramjjwtcyVVVuYNKRLyy=958545079.109760798584372551279856507814f; else UrLoiYQbkEramjjwtcyVVVuYNKRLyy=650416192.679201544464195409233511445426f;if (UrLoiYQbkEramjjwtcyVVVuYNKRLyy - UrLoiYQbkEramjjwtcyVVVuYNKRLyy> 0.00000001 ) UrLoiYQbkEramjjwtcyVVVuYNKRLyy=768775816.399592628211892677111979226987f; else UrLoiYQbkEramjjwtcyVVVuYNKRLyy=1986220173.723818670398112373097832652038f;if (UrLoiYQbkEramjjwtcyVVVuYNKRLyy - UrLoiYQbkEramjjwtcyVVVuYNKRLyy> 0.00000001 ) UrLoiYQbkEramjjwtcyVVVuYNKRLyy=1372402563.785160819401893486805390199236f; else UrLoiYQbkEramjjwtcyVVVuYNKRLyy=1077340737.664372416257579614038734349517f;if (UrLoiYQbkEramjjwtcyVVVuYNKRLyy - UrLoiYQbkEramjjwtcyVVVuYNKRLyy> 0.00000001 ) UrLoiYQbkEramjjwtcyVVVuYNKRLyy=1751560113.178385763118166993258521903113f; else UrLoiYQbkEramjjwtcyVVVuYNKRLyy=1717188364.390918929406293168845646966859f;float fZyoIkKXrwkVlbNSTroTdOholYVyaK=1633576938.485060159948325954660678610885f;if (fZyoIkKXrwkVlbNSTroTdOholYVyaK - fZyoIkKXrwkVlbNSTroTdOholYVyaK> 0.00000001 ) fZyoIkKXrwkVlbNSTroTdOholYVyaK=1298354355.977526941821606643861504681745f; else fZyoIkKXrwkVlbNSTroTdOholYVyaK=1987893933.037508180001494918308802457404f;if (fZyoIkKXrwkVlbNSTroTdOholYVyaK - fZyoIkKXrwkVlbNSTroTdOholYVyaK> 0.00000001 ) fZyoIkKXrwkVlbNSTroTdOholYVyaK=378276015.520689420856396012078455824616f; else fZyoIkKXrwkVlbNSTroTdOholYVyaK=175189992.189669281669855362644643718696f;if (fZyoIkKXrwkVlbNSTroTdOholYVyaK - fZyoIkKXrwkVlbNSTroTdOholYVyaK> 0.00000001 ) fZyoIkKXrwkVlbNSTroTdOholYVyaK=160170942.591588773894242970067307264706f; else fZyoIkKXrwkVlbNSTroTdOholYVyaK=87037993.160518848869996150285873030543f;if (fZyoIkKXrwkVlbNSTroTdOholYVyaK - fZyoIkKXrwkVlbNSTroTdOholYVyaK> 0.00000001 ) fZyoIkKXrwkVlbNSTroTdOholYVyaK=1587438980.418873474523320942212553140768f; else fZyoIkKXrwkVlbNSTroTdOholYVyaK=1191013452.841656212378787266310933296684f;if (fZyoIkKXrwkVlbNSTroTdOholYVyaK - fZyoIkKXrwkVlbNSTroTdOholYVyaK> 0.00000001 ) fZyoIkKXrwkVlbNSTroTdOholYVyaK=1372791600.482778377483005488708455901856f; else fZyoIkKXrwkVlbNSTroTdOholYVyaK=475410576.174990711999994794982898703787f;if (fZyoIkKXrwkVlbNSTroTdOholYVyaK - fZyoIkKXrwkVlbNSTroTdOholYVyaK> 0.00000001 ) fZyoIkKXrwkVlbNSTroTdOholYVyaK=590883534.763141921588882956122707548729f; else fZyoIkKXrwkVlbNSTroTdOholYVyaK=1741944750.703524796398440512786867516660f;float NyLpnYNepKhzpQWThjBSexURxUdyIn=1733390834.079836115412375137168014883342f;if (NyLpnYNepKhzpQWThjBSexURxUdyIn - NyLpnYNepKhzpQWThjBSexURxUdyIn> 0.00000001 ) NyLpnYNepKhzpQWThjBSexURxUdyIn=590473338.977276581751904690390250116506f; else NyLpnYNepKhzpQWThjBSexURxUdyIn=1671202242.487355620976741711139316272726f;if (NyLpnYNepKhzpQWThjBSexURxUdyIn - NyLpnYNepKhzpQWThjBSexURxUdyIn> 0.00000001 ) NyLpnYNepKhzpQWThjBSexURxUdyIn=1275412186.018637576008143561941658728229f; else NyLpnYNepKhzpQWThjBSexURxUdyIn=848478027.401596811524613237381188557102f;if (NyLpnYNepKhzpQWThjBSexURxUdyIn - NyLpnYNepKhzpQWThjBSexURxUdyIn> 0.00000001 ) NyLpnYNepKhzpQWThjBSexURxUdyIn=299454548.046771776885153704559195832304f; else NyLpnYNepKhzpQWThjBSexURxUdyIn=510814378.072059995357260713696365395204f;if (NyLpnYNepKhzpQWThjBSexURxUdyIn - NyLpnYNepKhzpQWThjBSexURxUdyIn> 0.00000001 ) NyLpnYNepKhzpQWThjBSexURxUdyIn=752760408.947281311491920394522081972876f; else NyLpnYNepKhzpQWThjBSexURxUdyIn=1717549413.480152018040320644511179306642f;if (NyLpnYNepKhzpQWThjBSexURxUdyIn - NyLpnYNepKhzpQWThjBSexURxUdyIn> 0.00000001 ) NyLpnYNepKhzpQWThjBSexURxUdyIn=2021605952.065501198939768277243837641296f; else NyLpnYNepKhzpQWThjBSexURxUdyIn=66799367.648708682056778252113050924907f;if (NyLpnYNepKhzpQWThjBSexURxUdyIn - NyLpnYNepKhzpQWThjBSexURxUdyIn> 0.00000001 ) NyLpnYNepKhzpQWThjBSexURxUdyIn=1979727881.000340206079520726959019369889f; else NyLpnYNepKhzpQWThjBSexURxUdyIn=484925515.589605754806625564131805910208f;long YKnZuAwMaAjMDZQTwUsBcgdmmmTktY=1782463958;if (YKnZuAwMaAjMDZQTwUsBcgdmmmTktY == YKnZuAwMaAjMDZQTwUsBcgdmmmTktY- 1 ) YKnZuAwMaAjMDZQTwUsBcgdmmmTktY=1048711201; else YKnZuAwMaAjMDZQTwUsBcgdmmmTktY=990013296;if (YKnZuAwMaAjMDZQTwUsBcgdmmmTktY == YKnZuAwMaAjMDZQTwUsBcgdmmmTktY- 1 ) YKnZuAwMaAjMDZQTwUsBcgdmmmTktY=898304610; else YKnZuAwMaAjMDZQTwUsBcgdmmmTktY=812066822;if (YKnZuAwMaAjMDZQTwUsBcgdmmmTktY == YKnZuAwMaAjMDZQTwUsBcgdmmmTktY- 1 ) YKnZuAwMaAjMDZQTwUsBcgdmmmTktY=1334120946; else YKnZuAwMaAjMDZQTwUsBcgdmmmTktY=1213265105;if (YKnZuAwMaAjMDZQTwUsBcgdmmmTktY == YKnZuAwMaAjMDZQTwUsBcgdmmmTktY- 1 ) YKnZuAwMaAjMDZQTwUsBcgdmmmTktY=94510340; else YKnZuAwMaAjMDZQTwUsBcgdmmmTktY=1522164658;if (YKnZuAwMaAjMDZQTwUsBcgdmmmTktY == YKnZuAwMaAjMDZQTwUsBcgdmmmTktY- 0 ) YKnZuAwMaAjMDZQTwUsBcgdmmmTktY=1476385425; else YKnZuAwMaAjMDZQTwUsBcgdmmmTktY=1249126805;if (YKnZuAwMaAjMDZQTwUsBcgdmmmTktY == YKnZuAwMaAjMDZQTwUsBcgdmmmTktY- 1 ) YKnZuAwMaAjMDZQTwUsBcgdmmmTktY=1390849472; else YKnZuAwMaAjMDZQTwUsBcgdmmmTktY=483134503;int piSBdvWJGHnODOjLwkWzmudfFGxSOP=1395192455;if (piSBdvWJGHnODOjLwkWzmudfFGxSOP == piSBdvWJGHnODOjLwkWzmudfFGxSOP- 0 ) piSBdvWJGHnODOjLwkWzmudfFGxSOP=1209261972; else piSBdvWJGHnODOjLwkWzmudfFGxSOP=18169279;if (piSBdvWJGHnODOjLwkWzmudfFGxSOP == piSBdvWJGHnODOjLwkWzmudfFGxSOP- 1 ) piSBdvWJGHnODOjLwkWzmudfFGxSOP=1126715026; else piSBdvWJGHnODOjLwkWzmudfFGxSOP=557228852;if (piSBdvWJGHnODOjLwkWzmudfFGxSOP == piSBdvWJGHnODOjLwkWzmudfFGxSOP- 0 ) piSBdvWJGHnODOjLwkWzmudfFGxSOP=308967428; else piSBdvWJGHnODOjLwkWzmudfFGxSOP=76697018;if (piSBdvWJGHnODOjLwkWzmudfFGxSOP == piSBdvWJGHnODOjLwkWzmudfFGxSOP- 1 ) piSBdvWJGHnODOjLwkWzmudfFGxSOP=1334122390; else piSBdvWJGHnODOjLwkWzmudfFGxSOP=787539977;if (piSBdvWJGHnODOjLwkWzmudfFGxSOP == piSBdvWJGHnODOjLwkWzmudfFGxSOP- 0 ) piSBdvWJGHnODOjLwkWzmudfFGxSOP=1235133525; else piSBdvWJGHnODOjLwkWzmudfFGxSOP=808210249;if (piSBdvWJGHnODOjLwkWzmudfFGxSOP == piSBdvWJGHnODOjLwkWzmudfFGxSOP- 0 ) piSBdvWJGHnODOjLwkWzmudfFGxSOP=301067687; else piSBdvWJGHnODOjLwkWzmudfFGxSOP=93371575;long IVYCQIsGaUrUnQdzlmywELSezkUZrC=528370936;if (IVYCQIsGaUrUnQdzlmywELSezkUZrC == IVYCQIsGaUrUnQdzlmywELSezkUZrC- 1 ) IVYCQIsGaUrUnQdzlmywELSezkUZrC=1896464232; else IVYCQIsGaUrUnQdzlmywELSezkUZrC=955903223;if (IVYCQIsGaUrUnQdzlmywELSezkUZrC == IVYCQIsGaUrUnQdzlmywELSezkUZrC- 0 ) IVYCQIsGaUrUnQdzlmywELSezkUZrC=693233549; else IVYCQIsGaUrUnQdzlmywELSezkUZrC=319649983;if (IVYCQIsGaUrUnQdzlmywELSezkUZrC == IVYCQIsGaUrUnQdzlmywELSezkUZrC- 0 ) IVYCQIsGaUrUnQdzlmywELSezkUZrC=1226389145; else IVYCQIsGaUrUnQdzlmywELSezkUZrC=1203495590;if (IVYCQIsGaUrUnQdzlmywELSezkUZrC == IVYCQIsGaUrUnQdzlmywELSezkUZrC- 1 ) IVYCQIsGaUrUnQdzlmywELSezkUZrC=399269888; else IVYCQIsGaUrUnQdzlmywELSezkUZrC=1524691880;if (IVYCQIsGaUrUnQdzlmywELSezkUZrC == IVYCQIsGaUrUnQdzlmywELSezkUZrC- 0 ) IVYCQIsGaUrUnQdzlmywELSezkUZrC=1576691902; else IVYCQIsGaUrUnQdzlmywELSezkUZrC=1900731175;if (IVYCQIsGaUrUnQdzlmywELSezkUZrC == IVYCQIsGaUrUnQdzlmywELSezkUZrC- 1 ) IVYCQIsGaUrUnQdzlmywELSezkUZrC=1519576249; else IVYCQIsGaUrUnQdzlmywELSezkUZrC=1197811390;long HEdEsHRWrDxbHqnBFotFRjUbZNkYiX=847291239;if (HEdEsHRWrDxbHqnBFotFRjUbZNkYiX == HEdEsHRWrDxbHqnBFotFRjUbZNkYiX- 0 ) HEdEsHRWrDxbHqnBFotFRjUbZNkYiX=1537726120; else HEdEsHRWrDxbHqnBFotFRjUbZNkYiX=584535426;if (HEdEsHRWrDxbHqnBFotFRjUbZNkYiX == HEdEsHRWrDxbHqnBFotFRjUbZNkYiX- 0 ) HEdEsHRWrDxbHqnBFotFRjUbZNkYiX=1423147190; else HEdEsHRWrDxbHqnBFotFRjUbZNkYiX=1412930293;if (HEdEsHRWrDxbHqnBFotFRjUbZNkYiX == HEdEsHRWrDxbHqnBFotFRjUbZNkYiX- 0 ) HEdEsHRWrDxbHqnBFotFRjUbZNkYiX=1865189285; else HEdEsHRWrDxbHqnBFotFRjUbZNkYiX=169201139;if (HEdEsHRWrDxbHqnBFotFRjUbZNkYiX == HEdEsHRWrDxbHqnBFotFRjUbZNkYiX- 0 ) HEdEsHRWrDxbHqnBFotFRjUbZNkYiX=1473496530; else HEdEsHRWrDxbHqnBFotFRjUbZNkYiX=972014183;if (HEdEsHRWrDxbHqnBFotFRjUbZNkYiX == HEdEsHRWrDxbHqnBFotFRjUbZNkYiX- 1 ) HEdEsHRWrDxbHqnBFotFRjUbZNkYiX=1803374553; else HEdEsHRWrDxbHqnBFotFRjUbZNkYiX=110764633;if (HEdEsHRWrDxbHqnBFotFRjUbZNkYiX == HEdEsHRWrDxbHqnBFotFRjUbZNkYiX- 1 ) HEdEsHRWrDxbHqnBFotFRjUbZNkYiX=1724416944; else HEdEsHRWrDxbHqnBFotFRjUbZNkYiX=525910697;float QugBJYysBiwGerHvHuRehucPrhpHKN=373871960.195153757397719192586082610735f;if (QugBJYysBiwGerHvHuRehucPrhpHKN - QugBJYysBiwGerHvHuRehucPrhpHKN> 0.00000001 ) QugBJYysBiwGerHvHuRehucPrhpHKN=1489212925.208024739980873838490342821455f; else QugBJYysBiwGerHvHuRehucPrhpHKN=862478536.412220989557471732098235506721f;if (QugBJYysBiwGerHvHuRehucPrhpHKN - QugBJYysBiwGerHvHuRehucPrhpHKN> 0.00000001 ) QugBJYysBiwGerHvHuRehucPrhpHKN=734153918.448467965395905498129047729773f; else QugBJYysBiwGerHvHuRehucPrhpHKN=1251113082.436822901587770334121961240285f;if (QugBJYysBiwGerHvHuRehucPrhpHKN - QugBJYysBiwGerHvHuRehucPrhpHKN> 0.00000001 ) QugBJYysBiwGerHvHuRehucPrhpHKN=481081164.829397028359774347749676267678f; else QugBJYysBiwGerHvHuRehucPrhpHKN=356906911.282777949442688229800355636424f;if (QugBJYysBiwGerHvHuRehucPrhpHKN - QugBJYysBiwGerHvHuRehucPrhpHKN> 0.00000001 ) QugBJYysBiwGerHvHuRehucPrhpHKN=168803310.155027811741940700804887804814f; else QugBJYysBiwGerHvHuRehucPrhpHKN=5766382.262530767152826042503763037922f;if (QugBJYysBiwGerHvHuRehucPrhpHKN - QugBJYysBiwGerHvHuRehucPrhpHKN> 0.00000001 ) QugBJYysBiwGerHvHuRehucPrhpHKN=1766383038.400869951005385207197232161951f; else QugBJYysBiwGerHvHuRehucPrhpHKN=1749506793.394586217720150949984194627970f;if (QugBJYysBiwGerHvHuRehucPrhpHKN - QugBJYysBiwGerHvHuRehucPrhpHKN> 0.00000001 ) QugBJYysBiwGerHvHuRehucPrhpHKN=1128020597.900072180019148718232317831106f; else QugBJYysBiwGerHvHuRehucPrhpHKN=555719900.945792118986908515791984870335f;double DpTDiYScdgzzOZfTHuQJeRDbyhBtHL=965507620.510788565699290674790915084438;if (DpTDiYScdgzzOZfTHuQJeRDbyhBtHL == DpTDiYScdgzzOZfTHuQJeRDbyhBtHL ) DpTDiYScdgzzOZfTHuQJeRDbyhBtHL=1297677141.075679646201736974874920961581; else DpTDiYScdgzzOZfTHuQJeRDbyhBtHL=1601260095.423772463114860039011166516681;if (DpTDiYScdgzzOZfTHuQJeRDbyhBtHL == DpTDiYScdgzzOZfTHuQJeRDbyhBtHL ) DpTDiYScdgzzOZfTHuQJeRDbyhBtHL=703129102.916688817562277116194267046736; else DpTDiYScdgzzOZfTHuQJeRDbyhBtHL=1817741980.698331838233356990040150775064;if (DpTDiYScdgzzOZfTHuQJeRDbyhBtHL == DpTDiYScdgzzOZfTHuQJeRDbyhBtHL ) DpTDiYScdgzzOZfTHuQJeRDbyhBtHL=1826849922.218107494649807202215780764967; else DpTDiYScdgzzOZfTHuQJeRDbyhBtHL=494467234.798502525961651236367385964955;if (DpTDiYScdgzzOZfTHuQJeRDbyhBtHL == DpTDiYScdgzzOZfTHuQJeRDbyhBtHL ) DpTDiYScdgzzOZfTHuQJeRDbyhBtHL=810665298.364895240679736513953971051364; else DpTDiYScdgzzOZfTHuQJeRDbyhBtHL=1727263093.978161797619689966056200259485;if (DpTDiYScdgzzOZfTHuQJeRDbyhBtHL == DpTDiYScdgzzOZfTHuQJeRDbyhBtHL ) DpTDiYScdgzzOZfTHuQJeRDbyhBtHL=1629890340.599781167692744549229642978802; else DpTDiYScdgzzOZfTHuQJeRDbyhBtHL=1868703013.242222001981946456772126606881;if (DpTDiYScdgzzOZfTHuQJeRDbyhBtHL == DpTDiYScdgzzOZfTHuQJeRDbyhBtHL ) DpTDiYScdgzzOZfTHuQJeRDbyhBtHL=663759077.337655153354944970653223113662; else DpTDiYScdgzzOZfTHuQJeRDbyhBtHL=1115624512.722165440240442829384734822454; }
 DpTDiYScdgzzOZfTHuQJeRDbyhBtHLy::DpTDiYScdgzzOZfTHuQJeRDbyhBtHLy()
 { this->rUmpNiOALopp("qRnjpWrGwkuENCtVuIPnRxOamlsFBXrUmpNiOALoppj", true, 1626562293, 2020842838, 612494462); }
#pragma optimize("", off)
 // <delete/>


// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class QJCLoOqlfodPvnDeIMfuaOKpIOlmXNy
 { 
public: bool qfhSgawUtDxufJhnaKanAuJQByMCZc; double qfhSgawUtDxufJhnaKanAuJQByMCZcQJCLoOqlfodPvnDeIMfuaOKpIOlmXN; QJCLoOqlfodPvnDeIMfuaOKpIOlmXNy(); void BCVWYHbdyZBq(string qfhSgawUtDxufJhnaKanAuJQByMCZcBCVWYHbdyZBq, bool ENRzmkNUWEMsmrdUegbaLbxZpvVRRh, int OtuQykcgWwZGnWbIISngUfbzRcQrKq, float PwCppTxVsssxJfBEoaygnCelNiRvzW, long UkWggcDyhIKnZunWVwxMoptdDNSRcG);
 protected: bool qfhSgawUtDxufJhnaKanAuJQByMCZco; double qfhSgawUtDxufJhnaKanAuJQByMCZcQJCLoOqlfodPvnDeIMfuaOKpIOlmXNf; void BCVWYHbdyZBqu(string qfhSgawUtDxufJhnaKanAuJQByMCZcBCVWYHbdyZBqg, bool ENRzmkNUWEMsmrdUegbaLbxZpvVRRhe, int OtuQykcgWwZGnWbIISngUfbzRcQrKqr, float PwCppTxVsssxJfBEoaygnCelNiRvzWw, long UkWggcDyhIKnZunWVwxMoptdDNSRcGn);
 private: bool qfhSgawUtDxufJhnaKanAuJQByMCZcENRzmkNUWEMsmrdUegbaLbxZpvVRRh; double qfhSgawUtDxufJhnaKanAuJQByMCZcPwCppTxVsssxJfBEoaygnCelNiRvzWQJCLoOqlfodPvnDeIMfuaOKpIOlmXN;
 void BCVWYHbdyZBqv(string ENRzmkNUWEMsmrdUegbaLbxZpvVRRhBCVWYHbdyZBq, bool ENRzmkNUWEMsmrdUegbaLbxZpvVRRhOtuQykcgWwZGnWbIISngUfbzRcQrKq, int OtuQykcgWwZGnWbIISngUfbzRcQrKqqfhSgawUtDxufJhnaKanAuJQByMCZc, float PwCppTxVsssxJfBEoaygnCelNiRvzWUkWggcDyhIKnZunWVwxMoptdDNSRcG, long UkWggcDyhIKnZunWVwxMoptdDNSRcGENRzmkNUWEMsmrdUegbaLbxZpvVRRh); };
 void QJCLoOqlfodPvnDeIMfuaOKpIOlmXNy::BCVWYHbdyZBq(string qfhSgawUtDxufJhnaKanAuJQByMCZcBCVWYHbdyZBq, bool ENRzmkNUWEMsmrdUegbaLbxZpvVRRh, int OtuQykcgWwZGnWbIISngUfbzRcQrKq, float PwCppTxVsssxJfBEoaygnCelNiRvzW, long UkWggcDyhIKnZunWVwxMoptdDNSRcG)
 { long kiwABwDZyWIcVnIzEZohrqkvXRTiUx=625863449;if (kiwABwDZyWIcVnIzEZohrqkvXRTiUx == kiwABwDZyWIcVnIzEZohrqkvXRTiUx- 1 ) kiwABwDZyWIcVnIzEZohrqkvXRTiUx=939469508; else kiwABwDZyWIcVnIzEZohrqkvXRTiUx=713103012;if (kiwABwDZyWIcVnIzEZohrqkvXRTiUx == kiwABwDZyWIcVnIzEZohrqkvXRTiUx- 0 ) kiwABwDZyWIcVnIzEZohrqkvXRTiUx=776586064; else kiwABwDZyWIcVnIzEZohrqkvXRTiUx=454008562;if (kiwABwDZyWIcVnIzEZohrqkvXRTiUx == kiwABwDZyWIcVnIzEZohrqkvXRTiUx- 0 ) kiwABwDZyWIcVnIzEZohrqkvXRTiUx=65371228; else kiwABwDZyWIcVnIzEZohrqkvXRTiUx=304273911;if (kiwABwDZyWIcVnIzEZohrqkvXRTiUx == kiwABwDZyWIcVnIzEZohrqkvXRTiUx- 0 ) kiwABwDZyWIcVnIzEZohrqkvXRTiUx=1230088231; else kiwABwDZyWIcVnIzEZohrqkvXRTiUx=1671823065;if (kiwABwDZyWIcVnIzEZohrqkvXRTiUx == kiwABwDZyWIcVnIzEZohrqkvXRTiUx- 1 ) kiwABwDZyWIcVnIzEZohrqkvXRTiUx=495908297; else kiwABwDZyWIcVnIzEZohrqkvXRTiUx=23525591;if (kiwABwDZyWIcVnIzEZohrqkvXRTiUx == kiwABwDZyWIcVnIzEZohrqkvXRTiUx- 0 ) kiwABwDZyWIcVnIzEZohrqkvXRTiUx=1928427839; else kiwABwDZyWIcVnIzEZohrqkvXRTiUx=496370705;int jPLjVPNdiQyyElTTrShwgvJMAsxsJU=303401610;if (jPLjVPNdiQyyElTTrShwgvJMAsxsJU == jPLjVPNdiQyyElTTrShwgvJMAsxsJU- 1 ) jPLjVPNdiQyyElTTrShwgvJMAsxsJU=854978410; else jPLjVPNdiQyyElTTrShwgvJMAsxsJU=1913809067;if (jPLjVPNdiQyyElTTrShwgvJMAsxsJU == jPLjVPNdiQyyElTTrShwgvJMAsxsJU- 1 ) jPLjVPNdiQyyElTTrShwgvJMAsxsJU=43628969; else jPLjVPNdiQyyElTTrShwgvJMAsxsJU=947685746;if (jPLjVPNdiQyyElTTrShwgvJMAsxsJU == jPLjVPNdiQyyElTTrShwgvJMAsxsJU- 1 ) jPLjVPNdiQyyElTTrShwgvJMAsxsJU=1999607185; else jPLjVPNdiQyyElTTrShwgvJMAsxsJU=152307555;if (jPLjVPNdiQyyElTTrShwgvJMAsxsJU == jPLjVPNdiQyyElTTrShwgvJMAsxsJU- 0 ) jPLjVPNdiQyyElTTrShwgvJMAsxsJU=2094060546; else jPLjVPNdiQyyElTTrShwgvJMAsxsJU=875198353;if (jPLjVPNdiQyyElTTrShwgvJMAsxsJU == jPLjVPNdiQyyElTTrShwgvJMAsxsJU- 1 ) jPLjVPNdiQyyElTTrShwgvJMAsxsJU=1776272137; else jPLjVPNdiQyyElTTrShwgvJMAsxsJU=2085831481;if (jPLjVPNdiQyyElTTrShwgvJMAsxsJU == jPLjVPNdiQyyElTTrShwgvJMAsxsJU- 0 ) jPLjVPNdiQyyElTTrShwgvJMAsxsJU=368136066; else jPLjVPNdiQyyElTTrShwgvJMAsxsJU=1889752152;float OCRafxzZQuKEpZiYHLFsaxWYJKvwpy=282324202.101405578875785661770856522180f;if (OCRafxzZQuKEpZiYHLFsaxWYJKvwpy - OCRafxzZQuKEpZiYHLFsaxWYJKvwpy> 0.00000001 ) OCRafxzZQuKEpZiYHLFsaxWYJKvwpy=1985585269.385056966484146825946055736699f; else OCRafxzZQuKEpZiYHLFsaxWYJKvwpy=318687936.440859183080539771080824444042f;if (OCRafxzZQuKEpZiYHLFsaxWYJKvwpy - OCRafxzZQuKEpZiYHLFsaxWYJKvwpy> 0.00000001 ) OCRafxzZQuKEpZiYHLFsaxWYJKvwpy=2055849319.137714539774207653607662963652f; else OCRafxzZQuKEpZiYHLFsaxWYJKvwpy=2006350553.624205392625232672889144657686f;if (OCRafxzZQuKEpZiYHLFsaxWYJKvwpy - OCRafxzZQuKEpZiYHLFsaxWYJKvwpy> 0.00000001 ) OCRafxzZQuKEpZiYHLFsaxWYJKvwpy=1077362740.199825011237119244130287459175f; else OCRafxzZQuKEpZiYHLFsaxWYJKvwpy=694485982.881764652766175213642466691247f;if (OCRafxzZQuKEpZiYHLFsaxWYJKvwpy - OCRafxzZQuKEpZiYHLFsaxWYJKvwpy> 0.00000001 ) OCRafxzZQuKEpZiYHLFsaxWYJKvwpy=185040141.704746746679247412225653352527f; else OCRafxzZQuKEpZiYHLFsaxWYJKvwpy=95258252.341507253057818322928013325240f;if (OCRafxzZQuKEpZiYHLFsaxWYJKvwpy - OCRafxzZQuKEpZiYHLFsaxWYJKvwpy> 0.00000001 ) OCRafxzZQuKEpZiYHLFsaxWYJKvwpy=1105210582.423933885127037885603170934929f; else OCRafxzZQuKEpZiYHLFsaxWYJKvwpy=1866093613.211364598555880208664164846411f;if (OCRafxzZQuKEpZiYHLFsaxWYJKvwpy - OCRafxzZQuKEpZiYHLFsaxWYJKvwpy> 0.00000001 ) OCRafxzZQuKEpZiYHLFsaxWYJKvwpy=1779940975.906692788449716849084737616753f; else OCRafxzZQuKEpZiYHLFsaxWYJKvwpy=1315870907.829876029953133555910744728919f;float AOYeLDuVykJxEMDeAYBhBnSMDoWDXq=846375216.460114645282025227316807289070f;if (AOYeLDuVykJxEMDeAYBhBnSMDoWDXq - AOYeLDuVykJxEMDeAYBhBnSMDoWDXq> 0.00000001 ) AOYeLDuVykJxEMDeAYBhBnSMDoWDXq=1921632507.312068052836716318011062067393f; else AOYeLDuVykJxEMDeAYBhBnSMDoWDXq=1291016940.568608017727568019198206945000f;if (AOYeLDuVykJxEMDeAYBhBnSMDoWDXq - AOYeLDuVykJxEMDeAYBhBnSMDoWDXq> 0.00000001 ) AOYeLDuVykJxEMDeAYBhBnSMDoWDXq=1328344283.286047821940239451505831636277f; else AOYeLDuVykJxEMDeAYBhBnSMDoWDXq=33057812.311765262120836807859561545641f;if (AOYeLDuVykJxEMDeAYBhBnSMDoWDXq - AOYeLDuVykJxEMDeAYBhBnSMDoWDXq> 0.00000001 ) AOYeLDuVykJxEMDeAYBhBnSMDoWDXq=1478040719.175564058539948600584053895132f; else AOYeLDuVykJxEMDeAYBhBnSMDoWDXq=1707206104.063216648396032162304617822378f;if (AOYeLDuVykJxEMDeAYBhBnSMDoWDXq - AOYeLDuVykJxEMDeAYBhBnSMDoWDXq> 0.00000001 ) AOYeLDuVykJxEMDeAYBhBnSMDoWDXq=773739911.993668784207330803183383955759f; else AOYeLDuVykJxEMDeAYBhBnSMDoWDXq=787161953.448040348549910839703029662181f;if (AOYeLDuVykJxEMDeAYBhBnSMDoWDXq - AOYeLDuVykJxEMDeAYBhBnSMDoWDXq> 0.00000001 ) AOYeLDuVykJxEMDeAYBhBnSMDoWDXq=766526113.313687030066664457322706752958f; else AOYeLDuVykJxEMDeAYBhBnSMDoWDXq=2048871358.937734743480984757522587062894f;if (AOYeLDuVykJxEMDeAYBhBnSMDoWDXq - AOYeLDuVykJxEMDeAYBhBnSMDoWDXq> 0.00000001 ) AOYeLDuVykJxEMDeAYBhBnSMDoWDXq=825220072.528214093181504282200644756177f; else AOYeLDuVykJxEMDeAYBhBnSMDoWDXq=127023394.295097387176445119388070408567f;long yLfusfZBmxCZCRWIytaXkmutLNccws=539477317;if (yLfusfZBmxCZCRWIytaXkmutLNccws == yLfusfZBmxCZCRWIytaXkmutLNccws- 1 ) yLfusfZBmxCZCRWIytaXkmutLNccws=512136352; else yLfusfZBmxCZCRWIytaXkmutLNccws=1646103637;if (yLfusfZBmxCZCRWIytaXkmutLNccws == yLfusfZBmxCZCRWIytaXkmutLNccws- 0 ) yLfusfZBmxCZCRWIytaXkmutLNccws=658269083; else yLfusfZBmxCZCRWIytaXkmutLNccws=1364534496;if (yLfusfZBmxCZCRWIytaXkmutLNccws == yLfusfZBmxCZCRWIytaXkmutLNccws- 1 ) yLfusfZBmxCZCRWIytaXkmutLNccws=764831456; else yLfusfZBmxCZCRWIytaXkmutLNccws=120003793;if (yLfusfZBmxCZCRWIytaXkmutLNccws == yLfusfZBmxCZCRWIytaXkmutLNccws- 1 ) yLfusfZBmxCZCRWIytaXkmutLNccws=1373522517; else yLfusfZBmxCZCRWIytaXkmutLNccws=160492428;if (yLfusfZBmxCZCRWIytaXkmutLNccws == yLfusfZBmxCZCRWIytaXkmutLNccws- 1 ) yLfusfZBmxCZCRWIytaXkmutLNccws=1728768926; else yLfusfZBmxCZCRWIytaXkmutLNccws=2095854364;if (yLfusfZBmxCZCRWIytaXkmutLNccws == yLfusfZBmxCZCRWIytaXkmutLNccws- 0 ) yLfusfZBmxCZCRWIytaXkmutLNccws=1989958811; else yLfusfZBmxCZCRWIytaXkmutLNccws=133513572;float yBfHNlclEZhgENuwuLqVakOGNpVPBh=1638852275.371714376082142140941084160146f;if (yBfHNlclEZhgENuwuLqVakOGNpVPBh - yBfHNlclEZhgENuwuLqVakOGNpVPBh> 0.00000001 ) yBfHNlclEZhgENuwuLqVakOGNpVPBh=1367152324.981001387898853091225739740865f; else yBfHNlclEZhgENuwuLqVakOGNpVPBh=1669244497.423111114186416324128903226247f;if (yBfHNlclEZhgENuwuLqVakOGNpVPBh - yBfHNlclEZhgENuwuLqVakOGNpVPBh> 0.00000001 ) yBfHNlclEZhgENuwuLqVakOGNpVPBh=2115603292.263257585398725942442675853109f; else yBfHNlclEZhgENuwuLqVakOGNpVPBh=2002449163.508893427994984039839490569940f;if (yBfHNlclEZhgENuwuLqVakOGNpVPBh - yBfHNlclEZhgENuwuLqVakOGNpVPBh> 0.00000001 ) yBfHNlclEZhgENuwuLqVakOGNpVPBh=775570344.549699103262656870193437111933f; else yBfHNlclEZhgENuwuLqVakOGNpVPBh=1625815074.569845640593678794470986712368f;if (yBfHNlclEZhgENuwuLqVakOGNpVPBh - yBfHNlclEZhgENuwuLqVakOGNpVPBh> 0.00000001 ) yBfHNlclEZhgENuwuLqVakOGNpVPBh=951767130.837080491234160097043867468314f; else yBfHNlclEZhgENuwuLqVakOGNpVPBh=278379165.448287779831325693673532617173f;if (yBfHNlclEZhgENuwuLqVakOGNpVPBh - yBfHNlclEZhgENuwuLqVakOGNpVPBh> 0.00000001 ) yBfHNlclEZhgENuwuLqVakOGNpVPBh=1692431672.328890799342822364297585191677f; else yBfHNlclEZhgENuwuLqVakOGNpVPBh=1268687366.071752353690534171146498684481f;if (yBfHNlclEZhgENuwuLqVakOGNpVPBh - yBfHNlclEZhgENuwuLqVakOGNpVPBh> 0.00000001 ) yBfHNlclEZhgENuwuLqVakOGNpVPBh=1239824440.913248228101270715102063278318f; else yBfHNlclEZhgENuwuLqVakOGNpVPBh=1175975029.808378830796414622908038996017f;long xCzNBMvCHwpWVFhdYxfYgUHynhDUmK=1764874503;if (xCzNBMvCHwpWVFhdYxfYgUHynhDUmK == xCzNBMvCHwpWVFhdYxfYgUHynhDUmK- 1 ) xCzNBMvCHwpWVFhdYxfYgUHynhDUmK=1340253004; else xCzNBMvCHwpWVFhdYxfYgUHynhDUmK=1240463658;if (xCzNBMvCHwpWVFhdYxfYgUHynhDUmK == xCzNBMvCHwpWVFhdYxfYgUHynhDUmK- 1 ) xCzNBMvCHwpWVFhdYxfYgUHynhDUmK=803734555; else xCzNBMvCHwpWVFhdYxfYgUHynhDUmK=197047775;if (xCzNBMvCHwpWVFhdYxfYgUHynhDUmK == xCzNBMvCHwpWVFhdYxfYgUHynhDUmK- 1 ) xCzNBMvCHwpWVFhdYxfYgUHynhDUmK=1626634426; else xCzNBMvCHwpWVFhdYxfYgUHynhDUmK=1501628308;if (xCzNBMvCHwpWVFhdYxfYgUHynhDUmK == xCzNBMvCHwpWVFhdYxfYgUHynhDUmK- 1 ) xCzNBMvCHwpWVFhdYxfYgUHynhDUmK=81543760; else xCzNBMvCHwpWVFhdYxfYgUHynhDUmK=1801628714;if (xCzNBMvCHwpWVFhdYxfYgUHynhDUmK == xCzNBMvCHwpWVFhdYxfYgUHynhDUmK- 1 ) xCzNBMvCHwpWVFhdYxfYgUHynhDUmK=2064978070; else xCzNBMvCHwpWVFhdYxfYgUHynhDUmK=1167851855;if (xCzNBMvCHwpWVFhdYxfYgUHynhDUmK == xCzNBMvCHwpWVFhdYxfYgUHynhDUmK- 1 ) xCzNBMvCHwpWVFhdYxfYgUHynhDUmK=451772533; else xCzNBMvCHwpWVFhdYxfYgUHynhDUmK=1529670002;int XJSOeKTkJQiOZswbKwYSySASSZfgeV=2135820121;if (XJSOeKTkJQiOZswbKwYSySASSZfgeV == XJSOeKTkJQiOZswbKwYSySASSZfgeV- 0 ) XJSOeKTkJQiOZswbKwYSySASSZfgeV=869340770; else XJSOeKTkJQiOZswbKwYSySASSZfgeV=1333851444;if (XJSOeKTkJQiOZswbKwYSySASSZfgeV == XJSOeKTkJQiOZswbKwYSySASSZfgeV- 1 ) XJSOeKTkJQiOZswbKwYSySASSZfgeV=907354717; else XJSOeKTkJQiOZswbKwYSySASSZfgeV=414376995;if (XJSOeKTkJQiOZswbKwYSySASSZfgeV == XJSOeKTkJQiOZswbKwYSySASSZfgeV- 0 ) XJSOeKTkJQiOZswbKwYSySASSZfgeV=1519700081; else XJSOeKTkJQiOZswbKwYSySASSZfgeV=670875093;if (XJSOeKTkJQiOZswbKwYSySASSZfgeV == XJSOeKTkJQiOZswbKwYSySASSZfgeV- 0 ) XJSOeKTkJQiOZswbKwYSySASSZfgeV=1911390620; else XJSOeKTkJQiOZswbKwYSySASSZfgeV=1033804925;if (XJSOeKTkJQiOZswbKwYSySASSZfgeV == XJSOeKTkJQiOZswbKwYSySASSZfgeV- 1 ) XJSOeKTkJQiOZswbKwYSySASSZfgeV=694336507; else XJSOeKTkJQiOZswbKwYSySASSZfgeV=379889918;if (XJSOeKTkJQiOZswbKwYSySASSZfgeV == XJSOeKTkJQiOZswbKwYSySASSZfgeV- 1 ) XJSOeKTkJQiOZswbKwYSySASSZfgeV=1819586471; else XJSOeKTkJQiOZswbKwYSySASSZfgeV=1643627737;int AQBQfgjoSwLigAAAPUAUuTkQxpOWen=1670752179;if (AQBQfgjoSwLigAAAPUAUuTkQxpOWen == AQBQfgjoSwLigAAAPUAUuTkQxpOWen- 1 ) AQBQfgjoSwLigAAAPUAUuTkQxpOWen=1936733957; else AQBQfgjoSwLigAAAPUAUuTkQxpOWen=225084308;if (AQBQfgjoSwLigAAAPUAUuTkQxpOWen == AQBQfgjoSwLigAAAPUAUuTkQxpOWen- 1 ) AQBQfgjoSwLigAAAPUAUuTkQxpOWen=940744215; else AQBQfgjoSwLigAAAPUAUuTkQxpOWen=1426870216;if (AQBQfgjoSwLigAAAPUAUuTkQxpOWen == AQBQfgjoSwLigAAAPUAUuTkQxpOWen- 1 ) AQBQfgjoSwLigAAAPUAUuTkQxpOWen=2121938731; else AQBQfgjoSwLigAAAPUAUuTkQxpOWen=982482813;if (AQBQfgjoSwLigAAAPUAUuTkQxpOWen == AQBQfgjoSwLigAAAPUAUuTkQxpOWen- 1 ) AQBQfgjoSwLigAAAPUAUuTkQxpOWen=12217849; else AQBQfgjoSwLigAAAPUAUuTkQxpOWen=2013007663;if (AQBQfgjoSwLigAAAPUAUuTkQxpOWen == AQBQfgjoSwLigAAAPUAUuTkQxpOWen- 1 ) AQBQfgjoSwLigAAAPUAUuTkQxpOWen=1587700737; else AQBQfgjoSwLigAAAPUAUuTkQxpOWen=313974578;if (AQBQfgjoSwLigAAAPUAUuTkQxpOWen == AQBQfgjoSwLigAAAPUAUuTkQxpOWen- 0 ) AQBQfgjoSwLigAAAPUAUuTkQxpOWen=2084954740; else AQBQfgjoSwLigAAAPUAUuTkQxpOWen=1755202136;int FzJEBIqMNrnXCIyKrcDbMjblTjGVnQ=1052205333;if (FzJEBIqMNrnXCIyKrcDbMjblTjGVnQ == FzJEBIqMNrnXCIyKrcDbMjblTjGVnQ- 0 ) FzJEBIqMNrnXCIyKrcDbMjblTjGVnQ=1001375640; else FzJEBIqMNrnXCIyKrcDbMjblTjGVnQ=1251487966;if (FzJEBIqMNrnXCIyKrcDbMjblTjGVnQ == FzJEBIqMNrnXCIyKrcDbMjblTjGVnQ- 1 ) FzJEBIqMNrnXCIyKrcDbMjblTjGVnQ=306634259; else FzJEBIqMNrnXCIyKrcDbMjblTjGVnQ=682898113;if (FzJEBIqMNrnXCIyKrcDbMjblTjGVnQ == FzJEBIqMNrnXCIyKrcDbMjblTjGVnQ- 1 ) FzJEBIqMNrnXCIyKrcDbMjblTjGVnQ=1298145677; else FzJEBIqMNrnXCIyKrcDbMjblTjGVnQ=1892227059;if (FzJEBIqMNrnXCIyKrcDbMjblTjGVnQ == FzJEBIqMNrnXCIyKrcDbMjblTjGVnQ- 1 ) FzJEBIqMNrnXCIyKrcDbMjblTjGVnQ=245174422; else FzJEBIqMNrnXCIyKrcDbMjblTjGVnQ=669377911;if (FzJEBIqMNrnXCIyKrcDbMjblTjGVnQ == FzJEBIqMNrnXCIyKrcDbMjblTjGVnQ- 0 ) FzJEBIqMNrnXCIyKrcDbMjblTjGVnQ=1476556685; else FzJEBIqMNrnXCIyKrcDbMjblTjGVnQ=1917413277;if (FzJEBIqMNrnXCIyKrcDbMjblTjGVnQ == FzJEBIqMNrnXCIyKrcDbMjblTjGVnQ- 1 ) FzJEBIqMNrnXCIyKrcDbMjblTjGVnQ=1650194915; else FzJEBIqMNrnXCIyKrcDbMjblTjGVnQ=1246744508;double wgvJGBLRbaWpoEqszRLPRQWwEkxNDu=1867430582.619283457359207946839076112839;if (wgvJGBLRbaWpoEqszRLPRQWwEkxNDu == wgvJGBLRbaWpoEqszRLPRQWwEkxNDu ) wgvJGBLRbaWpoEqszRLPRQWwEkxNDu=876625337.204663527834751703843584774327; else wgvJGBLRbaWpoEqszRLPRQWwEkxNDu=928504001.173825675542011533642909672190;if (wgvJGBLRbaWpoEqszRLPRQWwEkxNDu == wgvJGBLRbaWpoEqszRLPRQWwEkxNDu ) wgvJGBLRbaWpoEqszRLPRQWwEkxNDu=1740419692.931076294624941168424289638435; else wgvJGBLRbaWpoEqszRLPRQWwEkxNDu=1639646632.358661204544902816962588676255;if (wgvJGBLRbaWpoEqszRLPRQWwEkxNDu == wgvJGBLRbaWpoEqszRLPRQWwEkxNDu ) wgvJGBLRbaWpoEqszRLPRQWwEkxNDu=1846965771.987948748097498048883267498000; else wgvJGBLRbaWpoEqszRLPRQWwEkxNDu=987925157.605077716428807619064858272166;if (wgvJGBLRbaWpoEqszRLPRQWwEkxNDu == wgvJGBLRbaWpoEqszRLPRQWwEkxNDu ) wgvJGBLRbaWpoEqszRLPRQWwEkxNDu=13881390.074940253143246647198015796780; else wgvJGBLRbaWpoEqszRLPRQWwEkxNDu=2034341604.576156406847793161373617917936;if (wgvJGBLRbaWpoEqszRLPRQWwEkxNDu == wgvJGBLRbaWpoEqszRLPRQWwEkxNDu ) wgvJGBLRbaWpoEqszRLPRQWwEkxNDu=1321633595.467477820125135132364551707934; else wgvJGBLRbaWpoEqszRLPRQWwEkxNDu=1041830701.958049033644226958388678392687;if (wgvJGBLRbaWpoEqszRLPRQWwEkxNDu == wgvJGBLRbaWpoEqszRLPRQWwEkxNDu ) wgvJGBLRbaWpoEqszRLPRQWwEkxNDu=974159905.199627819711972043685976748086; else wgvJGBLRbaWpoEqszRLPRQWwEkxNDu=1205725503.846656571914967871636910333498;float nPQxkZNLgXkMljByCsvkoWaBsSeQMP=1272239379.668549245657617082753782486750f;if (nPQxkZNLgXkMljByCsvkoWaBsSeQMP - nPQxkZNLgXkMljByCsvkoWaBsSeQMP> 0.00000001 ) nPQxkZNLgXkMljByCsvkoWaBsSeQMP=1966241693.313034142301677074860825998717f; else nPQxkZNLgXkMljByCsvkoWaBsSeQMP=767381138.460534478405055311551299160924f;if (nPQxkZNLgXkMljByCsvkoWaBsSeQMP - nPQxkZNLgXkMljByCsvkoWaBsSeQMP> 0.00000001 ) nPQxkZNLgXkMljByCsvkoWaBsSeQMP=642252097.643695224471646688069727637157f; else nPQxkZNLgXkMljByCsvkoWaBsSeQMP=1923658681.555496356029829065469296693607f;if (nPQxkZNLgXkMljByCsvkoWaBsSeQMP - nPQxkZNLgXkMljByCsvkoWaBsSeQMP> 0.00000001 ) nPQxkZNLgXkMljByCsvkoWaBsSeQMP=2038396876.818792626033215000977280937091f; else nPQxkZNLgXkMljByCsvkoWaBsSeQMP=1755812052.475021213845043609179384074996f;if (nPQxkZNLgXkMljByCsvkoWaBsSeQMP - nPQxkZNLgXkMljByCsvkoWaBsSeQMP> 0.00000001 ) nPQxkZNLgXkMljByCsvkoWaBsSeQMP=372606502.574677939580823334834803210106f; else nPQxkZNLgXkMljByCsvkoWaBsSeQMP=44506898.742738147688179966284116388247f;if (nPQxkZNLgXkMljByCsvkoWaBsSeQMP - nPQxkZNLgXkMljByCsvkoWaBsSeQMP> 0.00000001 ) nPQxkZNLgXkMljByCsvkoWaBsSeQMP=2127393533.893385910529826783445492273656f; else nPQxkZNLgXkMljByCsvkoWaBsSeQMP=271366304.101199599516787640170581240112f;if (nPQxkZNLgXkMljByCsvkoWaBsSeQMP - nPQxkZNLgXkMljByCsvkoWaBsSeQMP> 0.00000001 ) nPQxkZNLgXkMljByCsvkoWaBsSeQMP=2097797178.237542974720404072674468673195f; else nPQxkZNLgXkMljByCsvkoWaBsSeQMP=204525454.048838034587746118738865897908f;float ppujmHejezWtFslkMLVcZRhlNtQBXD=1002301067.646126734190736669038138540743f;if (ppujmHejezWtFslkMLVcZRhlNtQBXD - ppujmHejezWtFslkMLVcZRhlNtQBXD> 0.00000001 ) ppujmHejezWtFslkMLVcZRhlNtQBXD=1687492511.955938847950312414488545151978f; else ppujmHejezWtFslkMLVcZRhlNtQBXD=217524158.610234581992218993613736552088f;if (ppujmHejezWtFslkMLVcZRhlNtQBXD - ppujmHejezWtFslkMLVcZRhlNtQBXD> 0.00000001 ) ppujmHejezWtFslkMLVcZRhlNtQBXD=878576799.897934029893360743737937508478f; else ppujmHejezWtFslkMLVcZRhlNtQBXD=245538540.361310539497482158792945207154f;if (ppujmHejezWtFslkMLVcZRhlNtQBXD - ppujmHejezWtFslkMLVcZRhlNtQBXD> 0.00000001 ) ppujmHejezWtFslkMLVcZRhlNtQBXD=1976644730.775164305121435056656089743612f; else ppujmHejezWtFslkMLVcZRhlNtQBXD=1068968887.359683333885467151135250879652f;if (ppujmHejezWtFslkMLVcZRhlNtQBXD - ppujmHejezWtFslkMLVcZRhlNtQBXD> 0.00000001 ) ppujmHejezWtFslkMLVcZRhlNtQBXD=1352723209.289387296383387272260269820981f; else ppujmHejezWtFslkMLVcZRhlNtQBXD=13450483.479271629007685297470759303156f;if (ppujmHejezWtFslkMLVcZRhlNtQBXD - ppujmHejezWtFslkMLVcZRhlNtQBXD> 0.00000001 ) ppujmHejezWtFslkMLVcZRhlNtQBXD=1237606576.732592522666427378474461282097f; else ppujmHejezWtFslkMLVcZRhlNtQBXD=419776302.967337529477582533204578347142f;if (ppujmHejezWtFslkMLVcZRhlNtQBXD - ppujmHejezWtFslkMLVcZRhlNtQBXD> 0.00000001 ) ppujmHejezWtFslkMLVcZRhlNtQBXD=1508748165.053681929196010799075090350900f; else ppujmHejezWtFslkMLVcZRhlNtQBXD=256314976.058743501162422866966917817434f;float CJyFRmMinNuruGRdkqZtTBUQEXIwSb=1320368201.888129570394058106400396196145f;if (CJyFRmMinNuruGRdkqZtTBUQEXIwSb - CJyFRmMinNuruGRdkqZtTBUQEXIwSb> 0.00000001 ) CJyFRmMinNuruGRdkqZtTBUQEXIwSb=2057650893.622618804542749154254701869213f; else CJyFRmMinNuruGRdkqZtTBUQEXIwSb=377955536.444688344130307966805390202185f;if (CJyFRmMinNuruGRdkqZtTBUQEXIwSb - CJyFRmMinNuruGRdkqZtTBUQEXIwSb> 0.00000001 ) CJyFRmMinNuruGRdkqZtTBUQEXIwSb=1427986462.970120927181022441501451925448f; else CJyFRmMinNuruGRdkqZtTBUQEXIwSb=1062144346.737926129814972448896179563930f;if (CJyFRmMinNuruGRdkqZtTBUQEXIwSb - CJyFRmMinNuruGRdkqZtTBUQEXIwSb> 0.00000001 ) CJyFRmMinNuruGRdkqZtTBUQEXIwSb=2090631220.559956510733389758959765267856f; else CJyFRmMinNuruGRdkqZtTBUQEXIwSb=997790680.420936602793926490092518290479f;if (CJyFRmMinNuruGRdkqZtTBUQEXIwSb - CJyFRmMinNuruGRdkqZtTBUQEXIwSb> 0.00000001 ) CJyFRmMinNuruGRdkqZtTBUQEXIwSb=1976517353.087432356426736695998886592274f; else CJyFRmMinNuruGRdkqZtTBUQEXIwSb=1268296932.623482759514468799270669120105f;if (CJyFRmMinNuruGRdkqZtTBUQEXIwSb - CJyFRmMinNuruGRdkqZtTBUQEXIwSb> 0.00000001 ) CJyFRmMinNuruGRdkqZtTBUQEXIwSb=555897499.029280335594648542991400417436f; else CJyFRmMinNuruGRdkqZtTBUQEXIwSb=1695912794.366560639846488702179832472987f;if (CJyFRmMinNuruGRdkqZtTBUQEXIwSb - CJyFRmMinNuruGRdkqZtTBUQEXIwSb> 0.00000001 ) CJyFRmMinNuruGRdkqZtTBUQEXIwSb=1659736746.491024929891415613382536764429f; else CJyFRmMinNuruGRdkqZtTBUQEXIwSb=1575599467.976255656589348943797970696537f;float XAywKbLezzekmyPLzrsIClOOADceeu=408676607.957312704817593435128412469692f;if (XAywKbLezzekmyPLzrsIClOOADceeu - XAywKbLezzekmyPLzrsIClOOADceeu> 0.00000001 ) XAywKbLezzekmyPLzrsIClOOADceeu=858348522.185601655768804702475167554605f; else XAywKbLezzekmyPLzrsIClOOADceeu=2119342485.311283299388157061877733813816f;if (XAywKbLezzekmyPLzrsIClOOADceeu - XAywKbLezzekmyPLzrsIClOOADceeu> 0.00000001 ) XAywKbLezzekmyPLzrsIClOOADceeu=1665716639.805045763261329802059394979857f; else XAywKbLezzekmyPLzrsIClOOADceeu=515879842.158458321331971641501196752228f;if (XAywKbLezzekmyPLzrsIClOOADceeu - XAywKbLezzekmyPLzrsIClOOADceeu> 0.00000001 ) XAywKbLezzekmyPLzrsIClOOADceeu=1425095332.701711497259182952333846707281f; else XAywKbLezzekmyPLzrsIClOOADceeu=1761707491.620474026035436211580199596602f;if (XAywKbLezzekmyPLzrsIClOOADceeu - XAywKbLezzekmyPLzrsIClOOADceeu> 0.00000001 ) XAywKbLezzekmyPLzrsIClOOADceeu=1443078296.522764982993731951050681163209f; else XAywKbLezzekmyPLzrsIClOOADceeu=897272806.952462488233069856974566304261f;if (XAywKbLezzekmyPLzrsIClOOADceeu - XAywKbLezzekmyPLzrsIClOOADceeu> 0.00000001 ) XAywKbLezzekmyPLzrsIClOOADceeu=1562141576.189405138958492772241743549726f; else XAywKbLezzekmyPLzrsIClOOADceeu=628801614.832816576473902728564203055697f;if (XAywKbLezzekmyPLzrsIClOOADceeu - XAywKbLezzekmyPLzrsIClOOADceeu> 0.00000001 ) XAywKbLezzekmyPLzrsIClOOADceeu=686052105.751323678630687994283126263987f; else XAywKbLezzekmyPLzrsIClOOADceeu=1618620574.298082279016194462717152563516f;long mtEYUNiBYkLcLjjEQNrxGMiBAWfZMv=940460967;if (mtEYUNiBYkLcLjjEQNrxGMiBAWfZMv == mtEYUNiBYkLcLjjEQNrxGMiBAWfZMv- 0 ) mtEYUNiBYkLcLjjEQNrxGMiBAWfZMv=335392393; else mtEYUNiBYkLcLjjEQNrxGMiBAWfZMv=777428977;if (mtEYUNiBYkLcLjjEQNrxGMiBAWfZMv == mtEYUNiBYkLcLjjEQNrxGMiBAWfZMv- 1 ) mtEYUNiBYkLcLjjEQNrxGMiBAWfZMv=294358208; else mtEYUNiBYkLcLjjEQNrxGMiBAWfZMv=1101816009;if (mtEYUNiBYkLcLjjEQNrxGMiBAWfZMv == mtEYUNiBYkLcLjjEQNrxGMiBAWfZMv- 1 ) mtEYUNiBYkLcLjjEQNrxGMiBAWfZMv=1632454173; else mtEYUNiBYkLcLjjEQNrxGMiBAWfZMv=1268821480;if (mtEYUNiBYkLcLjjEQNrxGMiBAWfZMv == mtEYUNiBYkLcLjjEQNrxGMiBAWfZMv- 1 ) mtEYUNiBYkLcLjjEQNrxGMiBAWfZMv=1059153494; else mtEYUNiBYkLcLjjEQNrxGMiBAWfZMv=689701831;if (mtEYUNiBYkLcLjjEQNrxGMiBAWfZMv == mtEYUNiBYkLcLjjEQNrxGMiBAWfZMv- 1 ) mtEYUNiBYkLcLjjEQNrxGMiBAWfZMv=388490452; else mtEYUNiBYkLcLjjEQNrxGMiBAWfZMv=1757763514;if (mtEYUNiBYkLcLjjEQNrxGMiBAWfZMv == mtEYUNiBYkLcLjjEQNrxGMiBAWfZMv- 0 ) mtEYUNiBYkLcLjjEQNrxGMiBAWfZMv=1837124688; else mtEYUNiBYkLcLjjEQNrxGMiBAWfZMv=280731936;long SuJmespPqFeBepHvZxSpUAHtExWBoB=1428250640;if (SuJmespPqFeBepHvZxSpUAHtExWBoB == SuJmespPqFeBepHvZxSpUAHtExWBoB- 0 ) SuJmespPqFeBepHvZxSpUAHtExWBoB=2012689890; else SuJmespPqFeBepHvZxSpUAHtExWBoB=1100071558;if (SuJmespPqFeBepHvZxSpUAHtExWBoB == SuJmespPqFeBepHvZxSpUAHtExWBoB- 0 ) SuJmespPqFeBepHvZxSpUAHtExWBoB=1545450101; else SuJmespPqFeBepHvZxSpUAHtExWBoB=946208316;if (SuJmespPqFeBepHvZxSpUAHtExWBoB == SuJmespPqFeBepHvZxSpUAHtExWBoB- 0 ) SuJmespPqFeBepHvZxSpUAHtExWBoB=1668699574; else SuJmespPqFeBepHvZxSpUAHtExWBoB=1567577716;if (SuJmespPqFeBepHvZxSpUAHtExWBoB == SuJmespPqFeBepHvZxSpUAHtExWBoB- 0 ) SuJmespPqFeBepHvZxSpUAHtExWBoB=2042756516; else SuJmespPqFeBepHvZxSpUAHtExWBoB=295943402;if (SuJmespPqFeBepHvZxSpUAHtExWBoB == SuJmespPqFeBepHvZxSpUAHtExWBoB- 1 ) SuJmespPqFeBepHvZxSpUAHtExWBoB=1082360887; else SuJmespPqFeBepHvZxSpUAHtExWBoB=530713656;if (SuJmespPqFeBepHvZxSpUAHtExWBoB == SuJmespPqFeBepHvZxSpUAHtExWBoB- 1 ) SuJmespPqFeBepHvZxSpUAHtExWBoB=1647486417; else SuJmespPqFeBepHvZxSpUAHtExWBoB=1461829606;float ZzWJNKIIZCJpZiGLXiBHLPmhFOOfmh=439605973.025040019548750350450066322201f;if (ZzWJNKIIZCJpZiGLXiBHLPmhFOOfmh - ZzWJNKIIZCJpZiGLXiBHLPmhFOOfmh> 0.00000001 ) ZzWJNKIIZCJpZiGLXiBHLPmhFOOfmh=421989403.773042223290986097936229106083f; else ZzWJNKIIZCJpZiGLXiBHLPmhFOOfmh=719275779.982887672914737128133648655495f;if (ZzWJNKIIZCJpZiGLXiBHLPmhFOOfmh - ZzWJNKIIZCJpZiGLXiBHLPmhFOOfmh> 0.00000001 ) ZzWJNKIIZCJpZiGLXiBHLPmhFOOfmh=1240207074.634142911953512269318906035434f; else ZzWJNKIIZCJpZiGLXiBHLPmhFOOfmh=260182649.229839060631828114640619774911f;if (ZzWJNKIIZCJpZiGLXiBHLPmhFOOfmh - ZzWJNKIIZCJpZiGLXiBHLPmhFOOfmh> 0.00000001 ) ZzWJNKIIZCJpZiGLXiBHLPmhFOOfmh=1662481375.759874488320088828903289462126f; else ZzWJNKIIZCJpZiGLXiBHLPmhFOOfmh=1600237697.625099315574642073106612967042f;if (ZzWJNKIIZCJpZiGLXiBHLPmhFOOfmh - ZzWJNKIIZCJpZiGLXiBHLPmhFOOfmh> 0.00000001 ) ZzWJNKIIZCJpZiGLXiBHLPmhFOOfmh=923706081.247280510051444065849902012413f; else ZzWJNKIIZCJpZiGLXiBHLPmhFOOfmh=1737010689.237433746267415637801230335528f;if (ZzWJNKIIZCJpZiGLXiBHLPmhFOOfmh - ZzWJNKIIZCJpZiGLXiBHLPmhFOOfmh> 0.00000001 ) ZzWJNKIIZCJpZiGLXiBHLPmhFOOfmh=1060188991.910792401232069369758672301811f; else ZzWJNKIIZCJpZiGLXiBHLPmhFOOfmh=976014808.922945873323677937208688035859f;if (ZzWJNKIIZCJpZiGLXiBHLPmhFOOfmh - ZzWJNKIIZCJpZiGLXiBHLPmhFOOfmh> 0.00000001 ) ZzWJNKIIZCJpZiGLXiBHLPmhFOOfmh=127389390.279141112460754600647461434591f; else ZzWJNKIIZCJpZiGLXiBHLPmhFOOfmh=1814815465.112525464693695804454521282881f;long dMEPamLNuHWhzeYrjgbSzDTocrFLTP=1785017834;if (dMEPamLNuHWhzeYrjgbSzDTocrFLTP == dMEPamLNuHWhzeYrjgbSzDTocrFLTP- 1 ) dMEPamLNuHWhzeYrjgbSzDTocrFLTP=1652962757; else dMEPamLNuHWhzeYrjgbSzDTocrFLTP=1405285112;if (dMEPamLNuHWhzeYrjgbSzDTocrFLTP == dMEPamLNuHWhzeYrjgbSzDTocrFLTP- 0 ) dMEPamLNuHWhzeYrjgbSzDTocrFLTP=1753414331; else dMEPamLNuHWhzeYrjgbSzDTocrFLTP=1294475976;if (dMEPamLNuHWhzeYrjgbSzDTocrFLTP == dMEPamLNuHWhzeYrjgbSzDTocrFLTP- 1 ) dMEPamLNuHWhzeYrjgbSzDTocrFLTP=718325072; else dMEPamLNuHWhzeYrjgbSzDTocrFLTP=327715075;if (dMEPamLNuHWhzeYrjgbSzDTocrFLTP == dMEPamLNuHWhzeYrjgbSzDTocrFLTP- 0 ) dMEPamLNuHWhzeYrjgbSzDTocrFLTP=1419245040; else dMEPamLNuHWhzeYrjgbSzDTocrFLTP=915298324;if (dMEPamLNuHWhzeYrjgbSzDTocrFLTP == dMEPamLNuHWhzeYrjgbSzDTocrFLTP- 0 ) dMEPamLNuHWhzeYrjgbSzDTocrFLTP=882156108; else dMEPamLNuHWhzeYrjgbSzDTocrFLTP=2145898453;if (dMEPamLNuHWhzeYrjgbSzDTocrFLTP == dMEPamLNuHWhzeYrjgbSzDTocrFLTP- 0 ) dMEPamLNuHWhzeYrjgbSzDTocrFLTP=19455122; else dMEPamLNuHWhzeYrjgbSzDTocrFLTP=1101740517;double mAystAPlNHSTkknaBBQUeqTJtdnryc=30593673.567702013116310716483608789364;if (mAystAPlNHSTkknaBBQUeqTJtdnryc == mAystAPlNHSTkknaBBQUeqTJtdnryc ) mAystAPlNHSTkknaBBQUeqTJtdnryc=736530469.169315220200153409823462797913; else mAystAPlNHSTkknaBBQUeqTJtdnryc=1397518715.386006811650739455648054180762;if (mAystAPlNHSTkknaBBQUeqTJtdnryc == mAystAPlNHSTkknaBBQUeqTJtdnryc ) mAystAPlNHSTkknaBBQUeqTJtdnryc=2006226180.931793526870937384445248543293; else mAystAPlNHSTkknaBBQUeqTJtdnryc=837440009.715983005704087823790643879426;if (mAystAPlNHSTkknaBBQUeqTJtdnryc == mAystAPlNHSTkknaBBQUeqTJtdnryc ) mAystAPlNHSTkknaBBQUeqTJtdnryc=684400315.033326606548085136182151318221; else mAystAPlNHSTkknaBBQUeqTJtdnryc=863139855.468977177232835266984445409311;if (mAystAPlNHSTkknaBBQUeqTJtdnryc == mAystAPlNHSTkknaBBQUeqTJtdnryc ) mAystAPlNHSTkknaBBQUeqTJtdnryc=1913252912.958529111807996043615245703468; else mAystAPlNHSTkknaBBQUeqTJtdnryc=412452193.997738531726618855047670682482;if (mAystAPlNHSTkknaBBQUeqTJtdnryc == mAystAPlNHSTkknaBBQUeqTJtdnryc ) mAystAPlNHSTkknaBBQUeqTJtdnryc=176365477.676402037861200073796852232826; else mAystAPlNHSTkknaBBQUeqTJtdnryc=1955923059.715016022793929648162970683438;if (mAystAPlNHSTkknaBBQUeqTJtdnryc == mAystAPlNHSTkknaBBQUeqTJtdnryc ) mAystAPlNHSTkknaBBQUeqTJtdnryc=2033502972.031115639415211403469576595643; else mAystAPlNHSTkknaBBQUeqTJtdnryc=692684766.470576282412976786939007966673;long zuhDUYkUXIJGBbleCCnCukDWTgOdFB=2067498174;if (zuhDUYkUXIJGBbleCCnCukDWTgOdFB == zuhDUYkUXIJGBbleCCnCukDWTgOdFB- 0 ) zuhDUYkUXIJGBbleCCnCukDWTgOdFB=1190156638; else zuhDUYkUXIJGBbleCCnCukDWTgOdFB=2009952230;if (zuhDUYkUXIJGBbleCCnCukDWTgOdFB == zuhDUYkUXIJGBbleCCnCukDWTgOdFB- 0 ) zuhDUYkUXIJGBbleCCnCukDWTgOdFB=1956350496; else zuhDUYkUXIJGBbleCCnCukDWTgOdFB=1053937110;if (zuhDUYkUXIJGBbleCCnCukDWTgOdFB == zuhDUYkUXIJGBbleCCnCukDWTgOdFB- 1 ) zuhDUYkUXIJGBbleCCnCukDWTgOdFB=751966095; else zuhDUYkUXIJGBbleCCnCukDWTgOdFB=1874240716;if (zuhDUYkUXIJGBbleCCnCukDWTgOdFB == zuhDUYkUXIJGBbleCCnCukDWTgOdFB- 0 ) zuhDUYkUXIJGBbleCCnCukDWTgOdFB=1868764548; else zuhDUYkUXIJGBbleCCnCukDWTgOdFB=94274328;if (zuhDUYkUXIJGBbleCCnCukDWTgOdFB == zuhDUYkUXIJGBbleCCnCukDWTgOdFB- 0 ) zuhDUYkUXIJGBbleCCnCukDWTgOdFB=1447514386; else zuhDUYkUXIJGBbleCCnCukDWTgOdFB=324908750;if (zuhDUYkUXIJGBbleCCnCukDWTgOdFB == zuhDUYkUXIJGBbleCCnCukDWTgOdFB- 0 ) zuhDUYkUXIJGBbleCCnCukDWTgOdFB=1525510188; else zuhDUYkUXIJGBbleCCnCukDWTgOdFB=1664066569;int zRXZTUElYOQKsGScpNYadXCcLzlPRM=1462865103;if (zRXZTUElYOQKsGScpNYadXCcLzlPRM == zRXZTUElYOQKsGScpNYadXCcLzlPRM- 1 ) zRXZTUElYOQKsGScpNYadXCcLzlPRM=598051552; else zRXZTUElYOQKsGScpNYadXCcLzlPRM=96795717;if (zRXZTUElYOQKsGScpNYadXCcLzlPRM == zRXZTUElYOQKsGScpNYadXCcLzlPRM- 1 ) zRXZTUElYOQKsGScpNYadXCcLzlPRM=1078284996; else zRXZTUElYOQKsGScpNYadXCcLzlPRM=674547735;if (zRXZTUElYOQKsGScpNYadXCcLzlPRM == zRXZTUElYOQKsGScpNYadXCcLzlPRM- 0 ) zRXZTUElYOQKsGScpNYadXCcLzlPRM=1303603827; else zRXZTUElYOQKsGScpNYadXCcLzlPRM=650600656;if (zRXZTUElYOQKsGScpNYadXCcLzlPRM == zRXZTUElYOQKsGScpNYadXCcLzlPRM- 1 ) zRXZTUElYOQKsGScpNYadXCcLzlPRM=1100617519; else zRXZTUElYOQKsGScpNYadXCcLzlPRM=789822902;if (zRXZTUElYOQKsGScpNYadXCcLzlPRM == zRXZTUElYOQKsGScpNYadXCcLzlPRM- 1 ) zRXZTUElYOQKsGScpNYadXCcLzlPRM=1639515847; else zRXZTUElYOQKsGScpNYadXCcLzlPRM=1340962138;if (zRXZTUElYOQKsGScpNYadXCcLzlPRM == zRXZTUElYOQKsGScpNYadXCcLzlPRM- 0 ) zRXZTUElYOQKsGScpNYadXCcLzlPRM=1118110499; else zRXZTUElYOQKsGScpNYadXCcLzlPRM=909885660;long BiBvEWRqNKqhOwIhpgQbgrDIxDefHR=654215057;if (BiBvEWRqNKqhOwIhpgQbgrDIxDefHR == BiBvEWRqNKqhOwIhpgQbgrDIxDefHR- 1 ) BiBvEWRqNKqhOwIhpgQbgrDIxDefHR=1774537854; else BiBvEWRqNKqhOwIhpgQbgrDIxDefHR=99440595;if (BiBvEWRqNKqhOwIhpgQbgrDIxDefHR == BiBvEWRqNKqhOwIhpgQbgrDIxDefHR- 0 ) BiBvEWRqNKqhOwIhpgQbgrDIxDefHR=2059067526; else BiBvEWRqNKqhOwIhpgQbgrDIxDefHR=1906350127;if (BiBvEWRqNKqhOwIhpgQbgrDIxDefHR == BiBvEWRqNKqhOwIhpgQbgrDIxDefHR- 0 ) BiBvEWRqNKqhOwIhpgQbgrDIxDefHR=1935940686; else BiBvEWRqNKqhOwIhpgQbgrDIxDefHR=1471509793;if (BiBvEWRqNKqhOwIhpgQbgrDIxDefHR == BiBvEWRqNKqhOwIhpgQbgrDIxDefHR- 0 ) BiBvEWRqNKqhOwIhpgQbgrDIxDefHR=1426111225; else BiBvEWRqNKqhOwIhpgQbgrDIxDefHR=1009773400;if (BiBvEWRqNKqhOwIhpgQbgrDIxDefHR == BiBvEWRqNKqhOwIhpgQbgrDIxDefHR- 0 ) BiBvEWRqNKqhOwIhpgQbgrDIxDefHR=1676237814; else BiBvEWRqNKqhOwIhpgQbgrDIxDefHR=1911951852;if (BiBvEWRqNKqhOwIhpgQbgrDIxDefHR == BiBvEWRqNKqhOwIhpgQbgrDIxDefHR- 1 ) BiBvEWRqNKqhOwIhpgQbgrDIxDefHR=1537409270; else BiBvEWRqNKqhOwIhpgQbgrDIxDefHR=359491565;float SlCMlcGfxvBjOAXhXnIFrierLYKgHU=354399913.385452539602966515465959134568f;if (SlCMlcGfxvBjOAXhXnIFrierLYKgHU - SlCMlcGfxvBjOAXhXnIFrierLYKgHU> 0.00000001 ) SlCMlcGfxvBjOAXhXnIFrierLYKgHU=1987731080.182992397842055230185805044642f; else SlCMlcGfxvBjOAXhXnIFrierLYKgHU=570637869.807153107523973300591548449705f;if (SlCMlcGfxvBjOAXhXnIFrierLYKgHU - SlCMlcGfxvBjOAXhXnIFrierLYKgHU> 0.00000001 ) SlCMlcGfxvBjOAXhXnIFrierLYKgHU=94633214.986614591323476203275425147972f; else SlCMlcGfxvBjOAXhXnIFrierLYKgHU=1343392609.541275837981038928334663157167f;if (SlCMlcGfxvBjOAXhXnIFrierLYKgHU - SlCMlcGfxvBjOAXhXnIFrierLYKgHU> 0.00000001 ) SlCMlcGfxvBjOAXhXnIFrierLYKgHU=1297139702.873089321257730459733975898822f; else SlCMlcGfxvBjOAXhXnIFrierLYKgHU=1844296511.978022915060829803471853590920f;if (SlCMlcGfxvBjOAXhXnIFrierLYKgHU - SlCMlcGfxvBjOAXhXnIFrierLYKgHU> 0.00000001 ) SlCMlcGfxvBjOAXhXnIFrierLYKgHU=763894347.160038071964587326710656123564f; else SlCMlcGfxvBjOAXhXnIFrierLYKgHU=539555982.219691199183251737737522061492f;if (SlCMlcGfxvBjOAXhXnIFrierLYKgHU - SlCMlcGfxvBjOAXhXnIFrierLYKgHU> 0.00000001 ) SlCMlcGfxvBjOAXhXnIFrierLYKgHU=909334711.662876609724763840649693889764f; else SlCMlcGfxvBjOAXhXnIFrierLYKgHU=1166527594.080853259523901397932295308910f;if (SlCMlcGfxvBjOAXhXnIFrierLYKgHU - SlCMlcGfxvBjOAXhXnIFrierLYKgHU> 0.00000001 ) SlCMlcGfxvBjOAXhXnIFrierLYKgHU=1561904910.804575401160113689259384399556f; else SlCMlcGfxvBjOAXhXnIFrierLYKgHU=1558487604.535082044700697218383076162128f;double vZdePcJRDhVYabykyjEsGNlQdeqrID=720954112.372547383182595578603071474762;if (vZdePcJRDhVYabykyjEsGNlQdeqrID == vZdePcJRDhVYabykyjEsGNlQdeqrID ) vZdePcJRDhVYabykyjEsGNlQdeqrID=849798752.139940532027936632590600431218; else vZdePcJRDhVYabykyjEsGNlQdeqrID=871295131.472116375604769614695228353051;if (vZdePcJRDhVYabykyjEsGNlQdeqrID == vZdePcJRDhVYabykyjEsGNlQdeqrID ) vZdePcJRDhVYabykyjEsGNlQdeqrID=2037012362.943575683137899695192762425596; else vZdePcJRDhVYabykyjEsGNlQdeqrID=1481341980.342486239404267854987907929126;if (vZdePcJRDhVYabykyjEsGNlQdeqrID == vZdePcJRDhVYabykyjEsGNlQdeqrID ) vZdePcJRDhVYabykyjEsGNlQdeqrID=2057865332.120889642457679961667262954556; else vZdePcJRDhVYabykyjEsGNlQdeqrID=209325499.812449458831253133816856906894;if (vZdePcJRDhVYabykyjEsGNlQdeqrID == vZdePcJRDhVYabykyjEsGNlQdeqrID ) vZdePcJRDhVYabykyjEsGNlQdeqrID=1674408064.510674534152493601992285006664; else vZdePcJRDhVYabykyjEsGNlQdeqrID=1274025406.467533269061086557331500244567;if (vZdePcJRDhVYabykyjEsGNlQdeqrID == vZdePcJRDhVYabykyjEsGNlQdeqrID ) vZdePcJRDhVYabykyjEsGNlQdeqrID=818168139.898070606154393220056933497739; else vZdePcJRDhVYabykyjEsGNlQdeqrID=68511596.971337411706238731803462196568;if (vZdePcJRDhVYabykyjEsGNlQdeqrID == vZdePcJRDhVYabykyjEsGNlQdeqrID ) vZdePcJRDhVYabykyjEsGNlQdeqrID=1145793568.307738407691946181587694662024; else vZdePcJRDhVYabykyjEsGNlQdeqrID=1539135622.196341051143521353611608281317;long WtMYzZfzWHNDXVVElMSBtPDPevnGeC=1390329504;if (WtMYzZfzWHNDXVVElMSBtPDPevnGeC == WtMYzZfzWHNDXVVElMSBtPDPevnGeC- 0 ) WtMYzZfzWHNDXVVElMSBtPDPevnGeC=361808873; else WtMYzZfzWHNDXVVElMSBtPDPevnGeC=763710586;if (WtMYzZfzWHNDXVVElMSBtPDPevnGeC == WtMYzZfzWHNDXVVElMSBtPDPevnGeC- 0 ) WtMYzZfzWHNDXVVElMSBtPDPevnGeC=1069638227; else WtMYzZfzWHNDXVVElMSBtPDPevnGeC=2018541528;if (WtMYzZfzWHNDXVVElMSBtPDPevnGeC == WtMYzZfzWHNDXVVElMSBtPDPevnGeC- 1 ) WtMYzZfzWHNDXVVElMSBtPDPevnGeC=631927060; else WtMYzZfzWHNDXVVElMSBtPDPevnGeC=279201036;if (WtMYzZfzWHNDXVVElMSBtPDPevnGeC == WtMYzZfzWHNDXVVElMSBtPDPevnGeC- 1 ) WtMYzZfzWHNDXVVElMSBtPDPevnGeC=1504559002; else WtMYzZfzWHNDXVVElMSBtPDPevnGeC=2077724990;if (WtMYzZfzWHNDXVVElMSBtPDPevnGeC == WtMYzZfzWHNDXVVElMSBtPDPevnGeC- 0 ) WtMYzZfzWHNDXVVElMSBtPDPevnGeC=1483029895; else WtMYzZfzWHNDXVVElMSBtPDPevnGeC=1519511544;if (WtMYzZfzWHNDXVVElMSBtPDPevnGeC == WtMYzZfzWHNDXVVElMSBtPDPevnGeC- 0 ) WtMYzZfzWHNDXVVElMSBtPDPevnGeC=997015416; else WtMYzZfzWHNDXVVElMSBtPDPevnGeC=769413092;double CnkkmwGxRyaKHiMoEeTyXgLUqbwJoM=717358926.408104610673782632491214934251;if (CnkkmwGxRyaKHiMoEeTyXgLUqbwJoM == CnkkmwGxRyaKHiMoEeTyXgLUqbwJoM ) CnkkmwGxRyaKHiMoEeTyXgLUqbwJoM=111889627.095691630221227524647731529428; else CnkkmwGxRyaKHiMoEeTyXgLUqbwJoM=816455158.147068919111328010852061546427;if (CnkkmwGxRyaKHiMoEeTyXgLUqbwJoM == CnkkmwGxRyaKHiMoEeTyXgLUqbwJoM ) CnkkmwGxRyaKHiMoEeTyXgLUqbwJoM=1657176460.221328536061554752452078618652; else CnkkmwGxRyaKHiMoEeTyXgLUqbwJoM=613818183.260299001058499162890517635372;if (CnkkmwGxRyaKHiMoEeTyXgLUqbwJoM == CnkkmwGxRyaKHiMoEeTyXgLUqbwJoM ) CnkkmwGxRyaKHiMoEeTyXgLUqbwJoM=359657628.108581237575151569065787276377; else CnkkmwGxRyaKHiMoEeTyXgLUqbwJoM=1645294932.483096001993632656727732742232;if (CnkkmwGxRyaKHiMoEeTyXgLUqbwJoM == CnkkmwGxRyaKHiMoEeTyXgLUqbwJoM ) CnkkmwGxRyaKHiMoEeTyXgLUqbwJoM=444018228.507010226657927326573076085236; else CnkkmwGxRyaKHiMoEeTyXgLUqbwJoM=1778405581.498040663343117192543786963214;if (CnkkmwGxRyaKHiMoEeTyXgLUqbwJoM == CnkkmwGxRyaKHiMoEeTyXgLUqbwJoM ) CnkkmwGxRyaKHiMoEeTyXgLUqbwJoM=1043713452.519027099877424096990379382218; else CnkkmwGxRyaKHiMoEeTyXgLUqbwJoM=968091455.755984510120488746397832434084;if (CnkkmwGxRyaKHiMoEeTyXgLUqbwJoM == CnkkmwGxRyaKHiMoEeTyXgLUqbwJoM ) CnkkmwGxRyaKHiMoEeTyXgLUqbwJoM=525224470.589113254068688109275603202466; else CnkkmwGxRyaKHiMoEeTyXgLUqbwJoM=1228628106.054765248669233926544591099692;long qupCIUyIQucJhHdWjyMlhtINXfRjkf=168208757;if (qupCIUyIQucJhHdWjyMlhtINXfRjkf == qupCIUyIQucJhHdWjyMlhtINXfRjkf- 1 ) qupCIUyIQucJhHdWjyMlhtINXfRjkf=1861818006; else qupCIUyIQucJhHdWjyMlhtINXfRjkf=171575406;if (qupCIUyIQucJhHdWjyMlhtINXfRjkf == qupCIUyIQucJhHdWjyMlhtINXfRjkf- 0 ) qupCIUyIQucJhHdWjyMlhtINXfRjkf=1196678731; else qupCIUyIQucJhHdWjyMlhtINXfRjkf=2139903278;if (qupCIUyIQucJhHdWjyMlhtINXfRjkf == qupCIUyIQucJhHdWjyMlhtINXfRjkf- 1 ) qupCIUyIQucJhHdWjyMlhtINXfRjkf=2036724308; else qupCIUyIQucJhHdWjyMlhtINXfRjkf=1929004344;if (qupCIUyIQucJhHdWjyMlhtINXfRjkf == qupCIUyIQucJhHdWjyMlhtINXfRjkf- 0 ) qupCIUyIQucJhHdWjyMlhtINXfRjkf=89027052; else qupCIUyIQucJhHdWjyMlhtINXfRjkf=570597716;if (qupCIUyIQucJhHdWjyMlhtINXfRjkf == qupCIUyIQucJhHdWjyMlhtINXfRjkf- 1 ) qupCIUyIQucJhHdWjyMlhtINXfRjkf=1514219776; else qupCIUyIQucJhHdWjyMlhtINXfRjkf=2106771019;if (qupCIUyIQucJhHdWjyMlhtINXfRjkf == qupCIUyIQucJhHdWjyMlhtINXfRjkf- 0 ) qupCIUyIQucJhHdWjyMlhtINXfRjkf=2145795732; else qupCIUyIQucJhHdWjyMlhtINXfRjkf=538353788;double qcMkDfSpcMtXlWgCYEpEhbveDfScQP=950544518.341208999080743121893370199753;if (qcMkDfSpcMtXlWgCYEpEhbveDfScQP == qcMkDfSpcMtXlWgCYEpEhbveDfScQP ) qcMkDfSpcMtXlWgCYEpEhbveDfScQP=1962352060.151722881261693153121983618221; else qcMkDfSpcMtXlWgCYEpEhbveDfScQP=428434642.024703903134063278704674327912;if (qcMkDfSpcMtXlWgCYEpEhbveDfScQP == qcMkDfSpcMtXlWgCYEpEhbveDfScQP ) qcMkDfSpcMtXlWgCYEpEhbveDfScQP=1427245995.576034700135119793154052856048; else qcMkDfSpcMtXlWgCYEpEhbveDfScQP=444219875.610267813967868956674587863539;if (qcMkDfSpcMtXlWgCYEpEhbveDfScQP == qcMkDfSpcMtXlWgCYEpEhbveDfScQP ) qcMkDfSpcMtXlWgCYEpEhbveDfScQP=1231433141.977352695399029839590058523451; else qcMkDfSpcMtXlWgCYEpEhbveDfScQP=838375052.549519152647148657813404093993;if (qcMkDfSpcMtXlWgCYEpEhbveDfScQP == qcMkDfSpcMtXlWgCYEpEhbveDfScQP ) qcMkDfSpcMtXlWgCYEpEhbveDfScQP=1030671876.035496051178805392007665915562; else qcMkDfSpcMtXlWgCYEpEhbveDfScQP=863997588.627962276360674660070299647168;if (qcMkDfSpcMtXlWgCYEpEhbveDfScQP == qcMkDfSpcMtXlWgCYEpEhbveDfScQP ) qcMkDfSpcMtXlWgCYEpEhbveDfScQP=319692358.150066195422684800136569353932; else qcMkDfSpcMtXlWgCYEpEhbveDfScQP=1438716293.522921172957665862595804370172;if (qcMkDfSpcMtXlWgCYEpEhbveDfScQP == qcMkDfSpcMtXlWgCYEpEhbveDfScQP ) qcMkDfSpcMtXlWgCYEpEhbveDfScQP=974828076.337391186483939990559244882866; else qcMkDfSpcMtXlWgCYEpEhbveDfScQP=1811319252.200063147578622297548073320351;double QJCLoOqlfodPvnDeIMfuaOKpIOlmXN=110787523.995067093508769964285622415709;if (QJCLoOqlfodPvnDeIMfuaOKpIOlmXN == QJCLoOqlfodPvnDeIMfuaOKpIOlmXN ) QJCLoOqlfodPvnDeIMfuaOKpIOlmXN=93146948.053001217748971789471733447376; else QJCLoOqlfodPvnDeIMfuaOKpIOlmXN=828806659.340455677885764776173633942066;if (QJCLoOqlfodPvnDeIMfuaOKpIOlmXN == QJCLoOqlfodPvnDeIMfuaOKpIOlmXN ) QJCLoOqlfodPvnDeIMfuaOKpIOlmXN=1055078733.495251962621360608088230381762; else QJCLoOqlfodPvnDeIMfuaOKpIOlmXN=1885513124.472149152492456238445509937507;if (QJCLoOqlfodPvnDeIMfuaOKpIOlmXN == QJCLoOqlfodPvnDeIMfuaOKpIOlmXN ) QJCLoOqlfodPvnDeIMfuaOKpIOlmXN=818428537.120135022155779788515442807332; else QJCLoOqlfodPvnDeIMfuaOKpIOlmXN=261223552.376537242132630084035910389844;if (QJCLoOqlfodPvnDeIMfuaOKpIOlmXN == QJCLoOqlfodPvnDeIMfuaOKpIOlmXN ) QJCLoOqlfodPvnDeIMfuaOKpIOlmXN=828118265.970098034255748481382331847626; else QJCLoOqlfodPvnDeIMfuaOKpIOlmXN=330368930.941937275216198138200856264035;if (QJCLoOqlfodPvnDeIMfuaOKpIOlmXN == QJCLoOqlfodPvnDeIMfuaOKpIOlmXN ) QJCLoOqlfodPvnDeIMfuaOKpIOlmXN=727428106.241842481221728735382053629677; else QJCLoOqlfodPvnDeIMfuaOKpIOlmXN=1086578744.335017646775708014964975439923;if (QJCLoOqlfodPvnDeIMfuaOKpIOlmXN == QJCLoOqlfodPvnDeIMfuaOKpIOlmXN ) QJCLoOqlfodPvnDeIMfuaOKpIOlmXN=1247082054.392500782110071745571681346068; else QJCLoOqlfodPvnDeIMfuaOKpIOlmXN=400370256.882448421934385500489545282226; }
 QJCLoOqlfodPvnDeIMfuaOKpIOlmXNy::QJCLoOqlfodPvnDeIMfuaOKpIOlmXNy()
 { this->BCVWYHbdyZBq("qfhSgawUtDxufJhnaKanAuJQByMCZcBCVWYHbdyZBqj", true, 1646982847, 2053148087, 418611851); }
#pragma optimize("", off)
 // <delete/>


// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class YWsxGlEWTLvIFcaaOzESRClnQvAkZBy
 { 
public: bool mTtELVLNcpdWQwrdGflCVTPnVkFgLl; double mTtELVLNcpdWQwrdGflCVTPnVkFgLlYWsxGlEWTLvIFcaaOzESRClnQvAkZB; YWsxGlEWTLvIFcaaOzESRClnQvAkZBy(); void AbjziqTFErRO(string mTtELVLNcpdWQwrdGflCVTPnVkFgLlAbjziqTFErRO, bool TwSrJoMXPEDmbdnfVqZRtFozxSEFja, int PfCrVgEYnCQiPgyjslApOPGtrNITuq, float kHoooYjjHGbxCWBLpHANZSEtMAYIAH, long YldwSJtbsazIduORNksgXBTgRRCvco);
 protected: bool mTtELVLNcpdWQwrdGflCVTPnVkFgLlo; double mTtELVLNcpdWQwrdGflCVTPnVkFgLlYWsxGlEWTLvIFcaaOzESRClnQvAkZBf; void AbjziqTFErROu(string mTtELVLNcpdWQwrdGflCVTPnVkFgLlAbjziqTFErROg, bool TwSrJoMXPEDmbdnfVqZRtFozxSEFjae, int PfCrVgEYnCQiPgyjslApOPGtrNITuqr, float kHoooYjjHGbxCWBLpHANZSEtMAYIAHw, long YldwSJtbsazIduORNksgXBTgRRCvcon);
 private: bool mTtELVLNcpdWQwrdGflCVTPnVkFgLlTwSrJoMXPEDmbdnfVqZRtFozxSEFja; double mTtELVLNcpdWQwrdGflCVTPnVkFgLlkHoooYjjHGbxCWBLpHANZSEtMAYIAHYWsxGlEWTLvIFcaaOzESRClnQvAkZB;
 void AbjziqTFErROv(string TwSrJoMXPEDmbdnfVqZRtFozxSEFjaAbjziqTFErRO, bool TwSrJoMXPEDmbdnfVqZRtFozxSEFjaPfCrVgEYnCQiPgyjslApOPGtrNITuq, int PfCrVgEYnCQiPgyjslApOPGtrNITuqmTtELVLNcpdWQwrdGflCVTPnVkFgLl, float kHoooYjjHGbxCWBLpHANZSEtMAYIAHYldwSJtbsazIduORNksgXBTgRRCvco, long YldwSJtbsazIduORNksgXBTgRRCvcoTwSrJoMXPEDmbdnfVqZRtFozxSEFja); };
 void YWsxGlEWTLvIFcaaOzESRClnQvAkZBy::AbjziqTFErRO(string mTtELVLNcpdWQwrdGflCVTPnVkFgLlAbjziqTFErRO, bool TwSrJoMXPEDmbdnfVqZRtFozxSEFja, int PfCrVgEYnCQiPgyjslApOPGtrNITuq, float kHoooYjjHGbxCWBLpHANZSEtMAYIAH, long YldwSJtbsazIduORNksgXBTgRRCvco)
 { double pmuAZkXVPpaJuieSXEpgNvtvrLFVMO=665283863.100029266593758500680782008424;if (pmuAZkXVPpaJuieSXEpgNvtvrLFVMO == pmuAZkXVPpaJuieSXEpgNvtvrLFVMO ) pmuAZkXVPpaJuieSXEpgNvtvrLFVMO=260201450.466259771143801544038268253902; else pmuAZkXVPpaJuieSXEpgNvtvrLFVMO=1766357150.284313851084054899266459631901;if (pmuAZkXVPpaJuieSXEpgNvtvrLFVMO == pmuAZkXVPpaJuieSXEpgNvtvrLFVMO ) pmuAZkXVPpaJuieSXEpgNvtvrLFVMO=1195672174.797907959870405521456585684689; else pmuAZkXVPpaJuieSXEpgNvtvrLFVMO=931925156.305040726805049715522156532333;if (pmuAZkXVPpaJuieSXEpgNvtvrLFVMO == pmuAZkXVPpaJuieSXEpgNvtvrLFVMO ) pmuAZkXVPpaJuieSXEpgNvtvrLFVMO=1834386324.348158647681576254608026725962; else pmuAZkXVPpaJuieSXEpgNvtvrLFVMO=1990836012.959456790227894269148874543244;if (pmuAZkXVPpaJuieSXEpgNvtvrLFVMO == pmuAZkXVPpaJuieSXEpgNvtvrLFVMO ) pmuAZkXVPpaJuieSXEpgNvtvrLFVMO=1615813322.379684222056548123366792418592; else pmuAZkXVPpaJuieSXEpgNvtvrLFVMO=1863830299.456696922104710244742008878738;if (pmuAZkXVPpaJuieSXEpgNvtvrLFVMO == pmuAZkXVPpaJuieSXEpgNvtvrLFVMO ) pmuAZkXVPpaJuieSXEpgNvtvrLFVMO=847924566.684060195331227309327595146893; else pmuAZkXVPpaJuieSXEpgNvtvrLFVMO=730698734.820462185026942299267051119003;if (pmuAZkXVPpaJuieSXEpgNvtvrLFVMO == pmuAZkXVPpaJuieSXEpgNvtvrLFVMO ) pmuAZkXVPpaJuieSXEpgNvtvrLFVMO=305986898.531252187523430275396873797945; else pmuAZkXVPpaJuieSXEpgNvtvrLFVMO=1702548180.328359570111274562105571787867;long JVMICWHoGizaOoXcxtKPZpVwrakfmK=184509968;if (JVMICWHoGizaOoXcxtKPZpVwrakfmK == JVMICWHoGizaOoXcxtKPZpVwrakfmK- 1 ) JVMICWHoGizaOoXcxtKPZpVwrakfmK=1454403820; else JVMICWHoGizaOoXcxtKPZpVwrakfmK=1976257750;if (JVMICWHoGizaOoXcxtKPZpVwrakfmK == JVMICWHoGizaOoXcxtKPZpVwrakfmK- 0 ) JVMICWHoGizaOoXcxtKPZpVwrakfmK=136223804; else JVMICWHoGizaOoXcxtKPZpVwrakfmK=806975014;if (JVMICWHoGizaOoXcxtKPZpVwrakfmK == JVMICWHoGizaOoXcxtKPZpVwrakfmK- 1 ) JVMICWHoGizaOoXcxtKPZpVwrakfmK=852177963; else JVMICWHoGizaOoXcxtKPZpVwrakfmK=1639404333;if (JVMICWHoGizaOoXcxtKPZpVwrakfmK == JVMICWHoGizaOoXcxtKPZpVwrakfmK- 0 ) JVMICWHoGizaOoXcxtKPZpVwrakfmK=61612977; else JVMICWHoGizaOoXcxtKPZpVwrakfmK=487348282;if (JVMICWHoGizaOoXcxtKPZpVwrakfmK == JVMICWHoGizaOoXcxtKPZpVwrakfmK- 0 ) JVMICWHoGizaOoXcxtKPZpVwrakfmK=227540304; else JVMICWHoGizaOoXcxtKPZpVwrakfmK=1089105170;if (JVMICWHoGizaOoXcxtKPZpVwrakfmK == JVMICWHoGizaOoXcxtKPZpVwrakfmK- 1 ) JVMICWHoGizaOoXcxtKPZpVwrakfmK=2027496353; else JVMICWHoGizaOoXcxtKPZpVwrakfmK=643388774;float vnXgZfXQRtnSkHHaUIFNaHaeueZtGy=1030534510.427202978777365441843849543325f;if (vnXgZfXQRtnSkHHaUIFNaHaeueZtGy - vnXgZfXQRtnSkHHaUIFNaHaeueZtGy> 0.00000001 ) vnXgZfXQRtnSkHHaUIFNaHaeueZtGy=785972594.528528061627582814085468868977f; else vnXgZfXQRtnSkHHaUIFNaHaeueZtGy=2116430035.975425985758033647883045027176f;if (vnXgZfXQRtnSkHHaUIFNaHaeueZtGy - vnXgZfXQRtnSkHHaUIFNaHaeueZtGy> 0.00000001 ) vnXgZfXQRtnSkHHaUIFNaHaeueZtGy=78861948.020575317793934744632210504150f; else vnXgZfXQRtnSkHHaUIFNaHaeueZtGy=1757247135.991983966834110011249365581108f;if (vnXgZfXQRtnSkHHaUIFNaHaeueZtGy - vnXgZfXQRtnSkHHaUIFNaHaeueZtGy> 0.00000001 ) vnXgZfXQRtnSkHHaUIFNaHaeueZtGy=775577784.136780215188980920059392274751f; else vnXgZfXQRtnSkHHaUIFNaHaeueZtGy=1982912151.296871844081716159577116094552f;if (vnXgZfXQRtnSkHHaUIFNaHaeueZtGy - vnXgZfXQRtnSkHHaUIFNaHaeueZtGy> 0.00000001 ) vnXgZfXQRtnSkHHaUIFNaHaeueZtGy=1768130087.766500211918707879623639419189f; else vnXgZfXQRtnSkHHaUIFNaHaeueZtGy=951145547.839243522794866951032694024819f;if (vnXgZfXQRtnSkHHaUIFNaHaeueZtGy - vnXgZfXQRtnSkHHaUIFNaHaeueZtGy> 0.00000001 ) vnXgZfXQRtnSkHHaUIFNaHaeueZtGy=40157174.223328057435803987367816724633f; else vnXgZfXQRtnSkHHaUIFNaHaeueZtGy=487039812.082457164890561899114765241861f;if (vnXgZfXQRtnSkHHaUIFNaHaeueZtGy - vnXgZfXQRtnSkHHaUIFNaHaeueZtGy> 0.00000001 ) vnXgZfXQRtnSkHHaUIFNaHaeueZtGy=564931120.443456445783467846604652043722f; else vnXgZfXQRtnSkHHaUIFNaHaeueZtGy=1838471813.631446712645453240841636799152f;long iLSBaLyIIDNVzcwFAtpyLvoGBAVjXi=1738997736;if (iLSBaLyIIDNVzcwFAtpyLvoGBAVjXi == iLSBaLyIIDNVzcwFAtpyLvoGBAVjXi- 1 ) iLSBaLyIIDNVzcwFAtpyLvoGBAVjXi=1979971392; else iLSBaLyIIDNVzcwFAtpyLvoGBAVjXi=2012787150;if (iLSBaLyIIDNVzcwFAtpyLvoGBAVjXi == iLSBaLyIIDNVzcwFAtpyLvoGBAVjXi- 0 ) iLSBaLyIIDNVzcwFAtpyLvoGBAVjXi=1771558452; else iLSBaLyIIDNVzcwFAtpyLvoGBAVjXi=944110584;if (iLSBaLyIIDNVzcwFAtpyLvoGBAVjXi == iLSBaLyIIDNVzcwFAtpyLvoGBAVjXi- 1 ) iLSBaLyIIDNVzcwFAtpyLvoGBAVjXi=667632239; else iLSBaLyIIDNVzcwFAtpyLvoGBAVjXi=1915907069;if (iLSBaLyIIDNVzcwFAtpyLvoGBAVjXi == iLSBaLyIIDNVzcwFAtpyLvoGBAVjXi- 0 ) iLSBaLyIIDNVzcwFAtpyLvoGBAVjXi=1960589547; else iLSBaLyIIDNVzcwFAtpyLvoGBAVjXi=768280764;if (iLSBaLyIIDNVzcwFAtpyLvoGBAVjXi == iLSBaLyIIDNVzcwFAtpyLvoGBAVjXi- 0 ) iLSBaLyIIDNVzcwFAtpyLvoGBAVjXi=1704744173; else iLSBaLyIIDNVzcwFAtpyLvoGBAVjXi=708323892;if (iLSBaLyIIDNVzcwFAtpyLvoGBAVjXi == iLSBaLyIIDNVzcwFAtpyLvoGBAVjXi- 0 ) iLSBaLyIIDNVzcwFAtpyLvoGBAVjXi=704384852; else iLSBaLyIIDNVzcwFAtpyLvoGBAVjXi=745281154;float VQYCRYzHyExnyuGaCnKKVdstHYSeBr=1501383930.122852769410591549713125335647f;if (VQYCRYzHyExnyuGaCnKKVdstHYSeBr - VQYCRYzHyExnyuGaCnKKVdstHYSeBr> 0.00000001 ) VQYCRYzHyExnyuGaCnKKVdstHYSeBr=1544808280.226993540599012416500789101862f; else VQYCRYzHyExnyuGaCnKKVdstHYSeBr=1422936035.722339561911932521392025755799f;if (VQYCRYzHyExnyuGaCnKKVdstHYSeBr - VQYCRYzHyExnyuGaCnKKVdstHYSeBr> 0.00000001 ) VQYCRYzHyExnyuGaCnKKVdstHYSeBr=916575586.288888779306569086869057181878f; else VQYCRYzHyExnyuGaCnKKVdstHYSeBr=18447768.587383416486300766687003123180f;if (VQYCRYzHyExnyuGaCnKKVdstHYSeBr - VQYCRYzHyExnyuGaCnKKVdstHYSeBr> 0.00000001 ) VQYCRYzHyExnyuGaCnKKVdstHYSeBr=86657688.801192939767149485843529269536f; else VQYCRYzHyExnyuGaCnKKVdstHYSeBr=648365563.134668532993702096575845839203f;if (VQYCRYzHyExnyuGaCnKKVdstHYSeBr - VQYCRYzHyExnyuGaCnKKVdstHYSeBr> 0.00000001 ) VQYCRYzHyExnyuGaCnKKVdstHYSeBr=1556415831.351463458195305093827306130984f; else VQYCRYzHyExnyuGaCnKKVdstHYSeBr=1618975316.044065334974849055095658605644f;if (VQYCRYzHyExnyuGaCnKKVdstHYSeBr - VQYCRYzHyExnyuGaCnKKVdstHYSeBr> 0.00000001 ) VQYCRYzHyExnyuGaCnKKVdstHYSeBr=208127663.468526244186996367718690940646f; else VQYCRYzHyExnyuGaCnKKVdstHYSeBr=1332561904.180161743392101076345993697978f;if (VQYCRYzHyExnyuGaCnKKVdstHYSeBr - VQYCRYzHyExnyuGaCnKKVdstHYSeBr> 0.00000001 ) VQYCRYzHyExnyuGaCnKKVdstHYSeBr=766817840.771901694573453750997530139867f; else VQYCRYzHyExnyuGaCnKKVdstHYSeBr=365138151.597673130149298355094808134188f;float qPmxodooDfLvnZqOWOrmUokNnsvcUu=1708449681.329206479375281106204735756594f;if (qPmxodooDfLvnZqOWOrmUokNnsvcUu - qPmxodooDfLvnZqOWOrmUokNnsvcUu> 0.00000001 ) qPmxodooDfLvnZqOWOrmUokNnsvcUu=1934507218.253834533177610683645755470555f; else qPmxodooDfLvnZqOWOrmUokNnsvcUu=288498617.099513246767413812517305951957f;if (qPmxodooDfLvnZqOWOrmUokNnsvcUu - qPmxodooDfLvnZqOWOrmUokNnsvcUu> 0.00000001 ) qPmxodooDfLvnZqOWOrmUokNnsvcUu=810901029.519818910854899935266804980466f; else qPmxodooDfLvnZqOWOrmUokNnsvcUu=1571255773.240936870690287351713689271594f;if (qPmxodooDfLvnZqOWOrmUokNnsvcUu - qPmxodooDfLvnZqOWOrmUokNnsvcUu> 0.00000001 ) qPmxodooDfLvnZqOWOrmUokNnsvcUu=1869949478.966562506317678673874976467176f; else qPmxodooDfLvnZqOWOrmUokNnsvcUu=389263517.548684582685641611271882109110f;if (qPmxodooDfLvnZqOWOrmUokNnsvcUu - qPmxodooDfLvnZqOWOrmUokNnsvcUu> 0.00000001 ) qPmxodooDfLvnZqOWOrmUokNnsvcUu=362902271.543097909419239944595601677768f; else qPmxodooDfLvnZqOWOrmUokNnsvcUu=1017549172.658214266482387376427663505352f;if (qPmxodooDfLvnZqOWOrmUokNnsvcUu - qPmxodooDfLvnZqOWOrmUokNnsvcUu> 0.00000001 ) qPmxodooDfLvnZqOWOrmUokNnsvcUu=155840488.134296202679822442540235210672f; else qPmxodooDfLvnZqOWOrmUokNnsvcUu=1458064831.001435919117495885226392200009f;if (qPmxodooDfLvnZqOWOrmUokNnsvcUu - qPmxodooDfLvnZqOWOrmUokNnsvcUu> 0.00000001 ) qPmxodooDfLvnZqOWOrmUokNnsvcUu=52502962.774168859753443485043555672924f; else qPmxodooDfLvnZqOWOrmUokNnsvcUu=67253892.692155443814757405093516622950f;int vNZFfGRynBVOJSzswqJmmwPGCucXOP=1215216273;if (vNZFfGRynBVOJSzswqJmmwPGCucXOP == vNZFfGRynBVOJSzswqJmmwPGCucXOP- 1 ) vNZFfGRynBVOJSzswqJmmwPGCucXOP=59768789; else vNZFfGRynBVOJSzswqJmmwPGCucXOP=1211030837;if (vNZFfGRynBVOJSzswqJmmwPGCucXOP == vNZFfGRynBVOJSzswqJmmwPGCucXOP- 1 ) vNZFfGRynBVOJSzswqJmmwPGCucXOP=293663533; else vNZFfGRynBVOJSzswqJmmwPGCucXOP=1597202872;if (vNZFfGRynBVOJSzswqJmmwPGCucXOP == vNZFfGRynBVOJSzswqJmmwPGCucXOP- 0 ) vNZFfGRynBVOJSzswqJmmwPGCucXOP=1897482331; else vNZFfGRynBVOJSzswqJmmwPGCucXOP=1283633831;if (vNZFfGRynBVOJSzswqJmmwPGCucXOP == vNZFfGRynBVOJSzswqJmmwPGCucXOP- 0 ) vNZFfGRynBVOJSzswqJmmwPGCucXOP=1652340048; else vNZFfGRynBVOJSzswqJmmwPGCucXOP=1331605829;if (vNZFfGRynBVOJSzswqJmmwPGCucXOP == vNZFfGRynBVOJSzswqJmmwPGCucXOP- 0 ) vNZFfGRynBVOJSzswqJmmwPGCucXOP=456371319; else vNZFfGRynBVOJSzswqJmmwPGCucXOP=152583136;if (vNZFfGRynBVOJSzswqJmmwPGCucXOP == vNZFfGRynBVOJSzswqJmmwPGCucXOP- 1 ) vNZFfGRynBVOJSzswqJmmwPGCucXOP=735982921; else vNZFfGRynBVOJSzswqJmmwPGCucXOP=1482553982;long OogNXvnMFNRNcYUMIBsSWLzCRTPXDm=1334119362;if (OogNXvnMFNRNcYUMIBsSWLzCRTPXDm == OogNXvnMFNRNcYUMIBsSWLzCRTPXDm- 0 ) OogNXvnMFNRNcYUMIBsSWLzCRTPXDm=1093630870; else OogNXvnMFNRNcYUMIBsSWLzCRTPXDm=1143418818;if (OogNXvnMFNRNcYUMIBsSWLzCRTPXDm == OogNXvnMFNRNcYUMIBsSWLzCRTPXDm- 0 ) OogNXvnMFNRNcYUMIBsSWLzCRTPXDm=958257583; else OogNXvnMFNRNcYUMIBsSWLzCRTPXDm=1822324689;if (OogNXvnMFNRNcYUMIBsSWLzCRTPXDm == OogNXvnMFNRNcYUMIBsSWLzCRTPXDm- 0 ) OogNXvnMFNRNcYUMIBsSWLzCRTPXDm=161523519; else OogNXvnMFNRNcYUMIBsSWLzCRTPXDm=1003498897;if (OogNXvnMFNRNcYUMIBsSWLzCRTPXDm == OogNXvnMFNRNcYUMIBsSWLzCRTPXDm- 1 ) OogNXvnMFNRNcYUMIBsSWLzCRTPXDm=1778918099; else OogNXvnMFNRNcYUMIBsSWLzCRTPXDm=1155544763;if (OogNXvnMFNRNcYUMIBsSWLzCRTPXDm == OogNXvnMFNRNcYUMIBsSWLzCRTPXDm- 1 ) OogNXvnMFNRNcYUMIBsSWLzCRTPXDm=1060033764; else OogNXvnMFNRNcYUMIBsSWLzCRTPXDm=2046510061;if (OogNXvnMFNRNcYUMIBsSWLzCRTPXDm == OogNXvnMFNRNcYUMIBsSWLzCRTPXDm- 0 ) OogNXvnMFNRNcYUMIBsSWLzCRTPXDm=2010090927; else OogNXvnMFNRNcYUMIBsSWLzCRTPXDm=776076504;long wPfVTgyXKAfsjEjHTrPiXQnXWqMUUk=1332762377;if (wPfVTgyXKAfsjEjHTrPiXQnXWqMUUk == wPfVTgyXKAfsjEjHTrPiXQnXWqMUUk- 0 ) wPfVTgyXKAfsjEjHTrPiXQnXWqMUUk=344756016; else wPfVTgyXKAfsjEjHTrPiXQnXWqMUUk=1699085214;if (wPfVTgyXKAfsjEjHTrPiXQnXWqMUUk == wPfVTgyXKAfsjEjHTrPiXQnXWqMUUk- 1 ) wPfVTgyXKAfsjEjHTrPiXQnXWqMUUk=305369362; else wPfVTgyXKAfsjEjHTrPiXQnXWqMUUk=2010926023;if (wPfVTgyXKAfsjEjHTrPiXQnXWqMUUk == wPfVTgyXKAfsjEjHTrPiXQnXWqMUUk- 0 ) wPfVTgyXKAfsjEjHTrPiXQnXWqMUUk=76961278; else wPfVTgyXKAfsjEjHTrPiXQnXWqMUUk=164973797;if (wPfVTgyXKAfsjEjHTrPiXQnXWqMUUk == wPfVTgyXKAfsjEjHTrPiXQnXWqMUUk- 1 ) wPfVTgyXKAfsjEjHTrPiXQnXWqMUUk=1958450997; else wPfVTgyXKAfsjEjHTrPiXQnXWqMUUk=650873387;if (wPfVTgyXKAfsjEjHTrPiXQnXWqMUUk == wPfVTgyXKAfsjEjHTrPiXQnXWqMUUk- 1 ) wPfVTgyXKAfsjEjHTrPiXQnXWqMUUk=783642216; else wPfVTgyXKAfsjEjHTrPiXQnXWqMUUk=1626778847;if (wPfVTgyXKAfsjEjHTrPiXQnXWqMUUk == wPfVTgyXKAfsjEjHTrPiXQnXWqMUUk- 0 ) wPfVTgyXKAfsjEjHTrPiXQnXWqMUUk=1114884454; else wPfVTgyXKAfsjEjHTrPiXQnXWqMUUk=1717366342;double RUYXNJNQzXMZDDhLBOqxIzhXBYxwcR=707872739.813157057618998743847238810249;if (RUYXNJNQzXMZDDhLBOqxIzhXBYxwcR == RUYXNJNQzXMZDDhLBOqxIzhXBYxwcR ) RUYXNJNQzXMZDDhLBOqxIzhXBYxwcR=1764257280.678735671321386557896300669635; else RUYXNJNQzXMZDDhLBOqxIzhXBYxwcR=865867247.782805126317362705336455337828;if (RUYXNJNQzXMZDDhLBOqxIzhXBYxwcR == RUYXNJNQzXMZDDhLBOqxIzhXBYxwcR ) RUYXNJNQzXMZDDhLBOqxIzhXBYxwcR=1121106669.714765969154158915806431944858; else RUYXNJNQzXMZDDhLBOqxIzhXBYxwcR=135108481.830870460885728340742213249493;if (RUYXNJNQzXMZDDhLBOqxIzhXBYxwcR == RUYXNJNQzXMZDDhLBOqxIzhXBYxwcR ) RUYXNJNQzXMZDDhLBOqxIzhXBYxwcR=64591350.121133539866502271360411663763; else RUYXNJNQzXMZDDhLBOqxIzhXBYxwcR=1312964846.217877460691199781217813371167;if (RUYXNJNQzXMZDDhLBOqxIzhXBYxwcR == RUYXNJNQzXMZDDhLBOqxIzhXBYxwcR ) RUYXNJNQzXMZDDhLBOqxIzhXBYxwcR=1053692754.187676984581278042299366750550; else RUYXNJNQzXMZDDhLBOqxIzhXBYxwcR=1203753539.641361463490783665634097983202;if (RUYXNJNQzXMZDDhLBOqxIzhXBYxwcR == RUYXNJNQzXMZDDhLBOqxIzhXBYxwcR ) RUYXNJNQzXMZDDhLBOqxIzhXBYxwcR=1579596385.654495102778991069419692676548; else RUYXNJNQzXMZDDhLBOqxIzhXBYxwcR=1285602417.937285749967497710693599316282;if (RUYXNJNQzXMZDDhLBOqxIzhXBYxwcR == RUYXNJNQzXMZDDhLBOqxIzhXBYxwcR ) RUYXNJNQzXMZDDhLBOqxIzhXBYxwcR=537169108.966495281742055331607696654898; else RUYXNJNQzXMZDDhLBOqxIzhXBYxwcR=1998455917.057701101253906154409568648194;float UuSkseOYYMMCTbUjsCXAATuvYwsGiO=1114693027.246332857845523750645420773098f;if (UuSkseOYYMMCTbUjsCXAATuvYwsGiO - UuSkseOYYMMCTbUjsCXAATuvYwsGiO> 0.00000001 ) UuSkseOYYMMCTbUjsCXAATuvYwsGiO=1959618766.326214369037598176672074992676f; else UuSkseOYYMMCTbUjsCXAATuvYwsGiO=1550704191.086063042059161903361843635332f;if (UuSkseOYYMMCTbUjsCXAATuvYwsGiO - UuSkseOYYMMCTbUjsCXAATuvYwsGiO> 0.00000001 ) UuSkseOYYMMCTbUjsCXAATuvYwsGiO=1137797966.279415154524463301300942624453f; else UuSkseOYYMMCTbUjsCXAATuvYwsGiO=1597487662.871099186686084848034503333880f;if (UuSkseOYYMMCTbUjsCXAATuvYwsGiO - UuSkseOYYMMCTbUjsCXAATuvYwsGiO> 0.00000001 ) UuSkseOYYMMCTbUjsCXAATuvYwsGiO=1290082034.180185348432673507898642047886f; else UuSkseOYYMMCTbUjsCXAATuvYwsGiO=1977848822.640651690950161294194600264925f;if (UuSkseOYYMMCTbUjsCXAATuvYwsGiO - UuSkseOYYMMCTbUjsCXAATuvYwsGiO> 0.00000001 ) UuSkseOYYMMCTbUjsCXAATuvYwsGiO=1257158084.967399202391804968185479215150f; else UuSkseOYYMMCTbUjsCXAATuvYwsGiO=928657073.778610388200290491640684059928f;if (UuSkseOYYMMCTbUjsCXAATuvYwsGiO - UuSkseOYYMMCTbUjsCXAATuvYwsGiO> 0.00000001 ) UuSkseOYYMMCTbUjsCXAATuvYwsGiO=1332451468.515263439569198639992462110467f; else UuSkseOYYMMCTbUjsCXAATuvYwsGiO=307384196.156080503807218169652104615426f;if (UuSkseOYYMMCTbUjsCXAATuvYwsGiO - UuSkseOYYMMCTbUjsCXAATuvYwsGiO> 0.00000001 ) UuSkseOYYMMCTbUjsCXAATuvYwsGiO=1038682473.307351312924719636405192516357f; else UuSkseOYYMMCTbUjsCXAATuvYwsGiO=682228319.528751006886884949619002132685f;int EfcWAbxfneQAhLuPFpehhiSnaKheMu=32201828;if (EfcWAbxfneQAhLuPFpehhiSnaKheMu == EfcWAbxfneQAhLuPFpehhiSnaKheMu- 0 ) EfcWAbxfneQAhLuPFpehhiSnaKheMu=30566471; else EfcWAbxfneQAhLuPFpehhiSnaKheMu=1302218188;if (EfcWAbxfneQAhLuPFpehhiSnaKheMu == EfcWAbxfneQAhLuPFpehhiSnaKheMu- 1 ) EfcWAbxfneQAhLuPFpehhiSnaKheMu=1159302871; else EfcWAbxfneQAhLuPFpehhiSnaKheMu=1877479001;if (EfcWAbxfneQAhLuPFpehhiSnaKheMu == EfcWAbxfneQAhLuPFpehhiSnaKheMu- 0 ) EfcWAbxfneQAhLuPFpehhiSnaKheMu=368055270; else EfcWAbxfneQAhLuPFpehhiSnaKheMu=205339536;if (EfcWAbxfneQAhLuPFpehhiSnaKheMu == EfcWAbxfneQAhLuPFpehhiSnaKheMu- 1 ) EfcWAbxfneQAhLuPFpehhiSnaKheMu=1268171027; else EfcWAbxfneQAhLuPFpehhiSnaKheMu=1179274817;if (EfcWAbxfneQAhLuPFpehhiSnaKheMu == EfcWAbxfneQAhLuPFpehhiSnaKheMu- 0 ) EfcWAbxfneQAhLuPFpehhiSnaKheMu=645392460; else EfcWAbxfneQAhLuPFpehhiSnaKheMu=1249099470;if (EfcWAbxfneQAhLuPFpehhiSnaKheMu == EfcWAbxfneQAhLuPFpehhiSnaKheMu- 1 ) EfcWAbxfneQAhLuPFpehhiSnaKheMu=431329638; else EfcWAbxfneQAhLuPFpehhiSnaKheMu=938842508;int dDKsaMJdcvwIhHCNtnTUgwBtLhoJWG=2054862319;if (dDKsaMJdcvwIhHCNtnTUgwBtLhoJWG == dDKsaMJdcvwIhHCNtnTUgwBtLhoJWG- 1 ) dDKsaMJdcvwIhHCNtnTUgwBtLhoJWG=891035619; else dDKsaMJdcvwIhHCNtnTUgwBtLhoJWG=191427;if (dDKsaMJdcvwIhHCNtnTUgwBtLhoJWG == dDKsaMJdcvwIhHCNtnTUgwBtLhoJWG- 1 ) dDKsaMJdcvwIhHCNtnTUgwBtLhoJWG=1905231223; else dDKsaMJdcvwIhHCNtnTUgwBtLhoJWG=250060052;if (dDKsaMJdcvwIhHCNtnTUgwBtLhoJWG == dDKsaMJdcvwIhHCNtnTUgwBtLhoJWG- 1 ) dDKsaMJdcvwIhHCNtnTUgwBtLhoJWG=2037517617; else dDKsaMJdcvwIhHCNtnTUgwBtLhoJWG=418455928;if (dDKsaMJdcvwIhHCNtnTUgwBtLhoJWG == dDKsaMJdcvwIhHCNtnTUgwBtLhoJWG- 1 ) dDKsaMJdcvwIhHCNtnTUgwBtLhoJWG=1565274352; else dDKsaMJdcvwIhHCNtnTUgwBtLhoJWG=1933892105;if (dDKsaMJdcvwIhHCNtnTUgwBtLhoJWG == dDKsaMJdcvwIhHCNtnTUgwBtLhoJWG- 0 ) dDKsaMJdcvwIhHCNtnTUgwBtLhoJWG=1756192810; else dDKsaMJdcvwIhHCNtnTUgwBtLhoJWG=192449596;if (dDKsaMJdcvwIhHCNtnTUgwBtLhoJWG == dDKsaMJdcvwIhHCNtnTUgwBtLhoJWG- 0 ) dDKsaMJdcvwIhHCNtnTUgwBtLhoJWG=950140660; else dDKsaMJdcvwIhHCNtnTUgwBtLhoJWG=1904690801;float KdSQofkzZLMUirzFuhmWoNLuKMEmjy=1518044628.925725013062634294442499959995f;if (KdSQofkzZLMUirzFuhmWoNLuKMEmjy - KdSQofkzZLMUirzFuhmWoNLuKMEmjy> 0.00000001 ) KdSQofkzZLMUirzFuhmWoNLuKMEmjy=1930821897.393687614772292281118321453788f; else KdSQofkzZLMUirzFuhmWoNLuKMEmjy=504967280.722887905869187021788888718233f;if (KdSQofkzZLMUirzFuhmWoNLuKMEmjy - KdSQofkzZLMUirzFuhmWoNLuKMEmjy> 0.00000001 ) KdSQofkzZLMUirzFuhmWoNLuKMEmjy=1967889446.593706041538646034444211194528f; else KdSQofkzZLMUirzFuhmWoNLuKMEmjy=118808363.078347658218125025379480039362f;if (KdSQofkzZLMUirzFuhmWoNLuKMEmjy - KdSQofkzZLMUirzFuhmWoNLuKMEmjy> 0.00000001 ) KdSQofkzZLMUirzFuhmWoNLuKMEmjy=1864444320.263457141959633629717026979832f; else KdSQofkzZLMUirzFuhmWoNLuKMEmjy=1005747874.949705677909553186751076535759f;if (KdSQofkzZLMUirzFuhmWoNLuKMEmjy - KdSQofkzZLMUirzFuhmWoNLuKMEmjy> 0.00000001 ) KdSQofkzZLMUirzFuhmWoNLuKMEmjy=746637757.832001003816281967499419198328f; else KdSQofkzZLMUirzFuhmWoNLuKMEmjy=1754279230.192818765219395689677846142392f;if (KdSQofkzZLMUirzFuhmWoNLuKMEmjy - KdSQofkzZLMUirzFuhmWoNLuKMEmjy> 0.00000001 ) KdSQofkzZLMUirzFuhmWoNLuKMEmjy=282533164.479682447321329112551986146788f; else KdSQofkzZLMUirzFuhmWoNLuKMEmjy=2106006796.747484182334235752814094150364f;if (KdSQofkzZLMUirzFuhmWoNLuKMEmjy - KdSQofkzZLMUirzFuhmWoNLuKMEmjy> 0.00000001 ) KdSQofkzZLMUirzFuhmWoNLuKMEmjy=952095202.787078404514123786149498476334f; else KdSQofkzZLMUirzFuhmWoNLuKMEmjy=40982564.583876826736930007247432649971f;double HhEDQFPblwhBUFOadnUmtFufDYFKRh=1372456388.070766661611314423704461334530;if (HhEDQFPblwhBUFOadnUmtFufDYFKRh == HhEDQFPblwhBUFOadnUmtFufDYFKRh ) HhEDQFPblwhBUFOadnUmtFufDYFKRh=1077537360.090837858978277714473310062944; else HhEDQFPblwhBUFOadnUmtFufDYFKRh=1131303801.224173169777584739473027778197;if (HhEDQFPblwhBUFOadnUmtFufDYFKRh == HhEDQFPblwhBUFOadnUmtFufDYFKRh ) HhEDQFPblwhBUFOadnUmtFufDYFKRh=1938676347.691475511575671683734358220850; else HhEDQFPblwhBUFOadnUmtFufDYFKRh=2035906663.518218722932800856610556344335;if (HhEDQFPblwhBUFOadnUmtFufDYFKRh == HhEDQFPblwhBUFOadnUmtFufDYFKRh ) HhEDQFPblwhBUFOadnUmtFufDYFKRh=303804181.687107847307281356858524116208; else HhEDQFPblwhBUFOadnUmtFufDYFKRh=1252204115.449746681401942337519959816409;if (HhEDQFPblwhBUFOadnUmtFufDYFKRh == HhEDQFPblwhBUFOadnUmtFufDYFKRh ) HhEDQFPblwhBUFOadnUmtFufDYFKRh=142167858.165906855688555769288089050104; else HhEDQFPblwhBUFOadnUmtFufDYFKRh=1759594190.028244714768394151415822229605;if (HhEDQFPblwhBUFOadnUmtFufDYFKRh == HhEDQFPblwhBUFOadnUmtFufDYFKRh ) HhEDQFPblwhBUFOadnUmtFufDYFKRh=1884427483.776176042206563070691089920612; else HhEDQFPblwhBUFOadnUmtFufDYFKRh=1372894413.612019323922021047026911580735;if (HhEDQFPblwhBUFOadnUmtFufDYFKRh == HhEDQFPblwhBUFOadnUmtFufDYFKRh ) HhEDQFPblwhBUFOadnUmtFufDYFKRh=121286191.644069465475778006809479771154; else HhEDQFPblwhBUFOadnUmtFufDYFKRh=175605674.070838907368275134368493638530;int QCIUDYuWzGBweVGGATfXhpyfdplNzB=273928025;if (QCIUDYuWzGBweVGGATfXhpyfdplNzB == QCIUDYuWzGBweVGGATfXhpyfdplNzB- 0 ) QCIUDYuWzGBweVGGATfXhpyfdplNzB=2081444021; else QCIUDYuWzGBweVGGATfXhpyfdplNzB=1060768657;if (QCIUDYuWzGBweVGGATfXhpyfdplNzB == QCIUDYuWzGBweVGGATfXhpyfdplNzB- 1 ) QCIUDYuWzGBweVGGATfXhpyfdplNzB=1155504258; else QCIUDYuWzGBweVGGATfXhpyfdplNzB=1270321056;if (QCIUDYuWzGBweVGGATfXhpyfdplNzB == QCIUDYuWzGBweVGGATfXhpyfdplNzB- 1 ) QCIUDYuWzGBweVGGATfXhpyfdplNzB=139589281; else QCIUDYuWzGBweVGGATfXhpyfdplNzB=1258522120;if (QCIUDYuWzGBweVGGATfXhpyfdplNzB == QCIUDYuWzGBweVGGATfXhpyfdplNzB- 1 ) QCIUDYuWzGBweVGGATfXhpyfdplNzB=190417999; else QCIUDYuWzGBweVGGATfXhpyfdplNzB=2032771392;if (QCIUDYuWzGBweVGGATfXhpyfdplNzB == QCIUDYuWzGBweVGGATfXhpyfdplNzB- 1 ) QCIUDYuWzGBweVGGATfXhpyfdplNzB=1401037317; else QCIUDYuWzGBweVGGATfXhpyfdplNzB=150951993;if (QCIUDYuWzGBweVGGATfXhpyfdplNzB == QCIUDYuWzGBweVGGATfXhpyfdplNzB- 0 ) QCIUDYuWzGBweVGGATfXhpyfdplNzB=2115010535; else QCIUDYuWzGBweVGGATfXhpyfdplNzB=2078994468;float flwIQsGYlEpeRiauoQKyYibtjdWQXX=1437877234.578087935970488193186366771205f;if (flwIQsGYlEpeRiauoQKyYibtjdWQXX - flwIQsGYlEpeRiauoQKyYibtjdWQXX> 0.00000001 ) flwIQsGYlEpeRiauoQKyYibtjdWQXX=1039080859.399137393124785724068960132231f; else flwIQsGYlEpeRiauoQKyYibtjdWQXX=1725167919.774068136281107379216734754901f;if (flwIQsGYlEpeRiauoQKyYibtjdWQXX - flwIQsGYlEpeRiauoQKyYibtjdWQXX> 0.00000001 ) flwIQsGYlEpeRiauoQKyYibtjdWQXX=827153441.626255172769998787496302605042f; else flwIQsGYlEpeRiauoQKyYibtjdWQXX=1290462219.524677696221756795717761532465f;if (flwIQsGYlEpeRiauoQKyYibtjdWQXX - flwIQsGYlEpeRiauoQKyYibtjdWQXX> 0.00000001 ) flwIQsGYlEpeRiauoQKyYibtjdWQXX=580271735.114024643120137665702044697314f; else flwIQsGYlEpeRiauoQKyYibtjdWQXX=1426716080.623112784326269764497787418154f;if (flwIQsGYlEpeRiauoQKyYibtjdWQXX - flwIQsGYlEpeRiauoQKyYibtjdWQXX> 0.00000001 ) flwIQsGYlEpeRiauoQKyYibtjdWQXX=1214240447.733961712699258688593492894671f; else flwIQsGYlEpeRiauoQKyYibtjdWQXX=678617782.473069340906391854993066244117f;if (flwIQsGYlEpeRiauoQKyYibtjdWQXX - flwIQsGYlEpeRiauoQKyYibtjdWQXX> 0.00000001 ) flwIQsGYlEpeRiauoQKyYibtjdWQXX=362799422.725132629045969721205179842893f; else flwIQsGYlEpeRiauoQKyYibtjdWQXX=208295256.523439127924505920752401365749f;if (flwIQsGYlEpeRiauoQKyYibtjdWQXX - flwIQsGYlEpeRiauoQKyYibtjdWQXX> 0.00000001 ) flwIQsGYlEpeRiauoQKyYibtjdWQXX=381864527.194752254171549167403214123217f; else flwIQsGYlEpeRiauoQKyYibtjdWQXX=491549907.458994096477779207321844798063f;double zVjjKLeZYHiGAPbcXduMfuugfGWLwc=919052938.930113856832859749899330807986;if (zVjjKLeZYHiGAPbcXduMfuugfGWLwc == zVjjKLeZYHiGAPbcXduMfuugfGWLwc ) zVjjKLeZYHiGAPbcXduMfuugfGWLwc=1201014352.209932694290239059227049261132; else zVjjKLeZYHiGAPbcXduMfuugfGWLwc=678167177.697073403816660921285976927187;if (zVjjKLeZYHiGAPbcXduMfuugfGWLwc == zVjjKLeZYHiGAPbcXduMfuugfGWLwc ) zVjjKLeZYHiGAPbcXduMfuugfGWLwc=107022190.166370327776742259935105782125; else zVjjKLeZYHiGAPbcXduMfuugfGWLwc=485750527.733776030170552023588935217269;if (zVjjKLeZYHiGAPbcXduMfuugfGWLwc == zVjjKLeZYHiGAPbcXduMfuugfGWLwc ) zVjjKLeZYHiGAPbcXduMfuugfGWLwc=1310294965.831214418732987565486974880856; else zVjjKLeZYHiGAPbcXduMfuugfGWLwc=30531328.997134035872696605885389497853;if (zVjjKLeZYHiGAPbcXduMfuugfGWLwc == zVjjKLeZYHiGAPbcXduMfuugfGWLwc ) zVjjKLeZYHiGAPbcXduMfuugfGWLwc=1232867107.566384686364062448839617171984; else zVjjKLeZYHiGAPbcXduMfuugfGWLwc=1966498887.839341642212052233806902595921;if (zVjjKLeZYHiGAPbcXduMfuugfGWLwc == zVjjKLeZYHiGAPbcXduMfuugfGWLwc ) zVjjKLeZYHiGAPbcXduMfuugfGWLwc=940885802.480254582180773776695919804786; else zVjjKLeZYHiGAPbcXduMfuugfGWLwc=2053388602.782194281976182634093043757246;if (zVjjKLeZYHiGAPbcXduMfuugfGWLwc == zVjjKLeZYHiGAPbcXduMfuugfGWLwc ) zVjjKLeZYHiGAPbcXduMfuugfGWLwc=634869346.190751968059673993737170374054; else zVjjKLeZYHiGAPbcXduMfuugfGWLwc=152852188.714599644187614448187452902058;long tHUNSaEpVQZyeXAYhdvNfvhjOgLftk=360135695;if (tHUNSaEpVQZyeXAYhdvNfvhjOgLftk == tHUNSaEpVQZyeXAYhdvNfvhjOgLftk- 1 ) tHUNSaEpVQZyeXAYhdvNfvhjOgLftk=985521492; else tHUNSaEpVQZyeXAYhdvNfvhjOgLftk=830892604;if (tHUNSaEpVQZyeXAYhdvNfvhjOgLftk == tHUNSaEpVQZyeXAYhdvNfvhjOgLftk- 1 ) tHUNSaEpVQZyeXAYhdvNfvhjOgLftk=1284008462; else tHUNSaEpVQZyeXAYhdvNfvhjOgLftk=1824998251;if (tHUNSaEpVQZyeXAYhdvNfvhjOgLftk == tHUNSaEpVQZyeXAYhdvNfvhjOgLftk- 1 ) tHUNSaEpVQZyeXAYhdvNfvhjOgLftk=683810432; else tHUNSaEpVQZyeXAYhdvNfvhjOgLftk=1762013872;if (tHUNSaEpVQZyeXAYhdvNfvhjOgLftk == tHUNSaEpVQZyeXAYhdvNfvhjOgLftk- 0 ) tHUNSaEpVQZyeXAYhdvNfvhjOgLftk=1841139937; else tHUNSaEpVQZyeXAYhdvNfvhjOgLftk=654727941;if (tHUNSaEpVQZyeXAYhdvNfvhjOgLftk == tHUNSaEpVQZyeXAYhdvNfvhjOgLftk- 0 ) tHUNSaEpVQZyeXAYhdvNfvhjOgLftk=1994011857; else tHUNSaEpVQZyeXAYhdvNfvhjOgLftk=476886476;if (tHUNSaEpVQZyeXAYhdvNfvhjOgLftk == tHUNSaEpVQZyeXAYhdvNfvhjOgLftk- 1 ) tHUNSaEpVQZyeXAYhdvNfvhjOgLftk=907521634; else tHUNSaEpVQZyeXAYhdvNfvhjOgLftk=2078777672;float ywVYTkzVsDzOIVgOkFSgOZjINaDfCj=830005234.449611185448045474369586465820f;if (ywVYTkzVsDzOIVgOkFSgOZjINaDfCj - ywVYTkzVsDzOIVgOkFSgOZjINaDfCj> 0.00000001 ) ywVYTkzVsDzOIVgOkFSgOZjINaDfCj=1946838933.009850904603873498514501406326f; else ywVYTkzVsDzOIVgOkFSgOZjINaDfCj=1195957597.736655930568076782317742650946f;if (ywVYTkzVsDzOIVgOkFSgOZjINaDfCj - ywVYTkzVsDzOIVgOkFSgOZjINaDfCj> 0.00000001 ) ywVYTkzVsDzOIVgOkFSgOZjINaDfCj=877980116.568998722957906763582512909510f; else ywVYTkzVsDzOIVgOkFSgOZjINaDfCj=935677196.519410118907057184700847506710f;if (ywVYTkzVsDzOIVgOkFSgOZjINaDfCj - ywVYTkzVsDzOIVgOkFSgOZjINaDfCj> 0.00000001 ) ywVYTkzVsDzOIVgOkFSgOZjINaDfCj=1417269598.932354969665578557592581177291f; else ywVYTkzVsDzOIVgOkFSgOZjINaDfCj=2049106041.551917680323624538906922845128f;if (ywVYTkzVsDzOIVgOkFSgOZjINaDfCj - ywVYTkzVsDzOIVgOkFSgOZjINaDfCj> 0.00000001 ) ywVYTkzVsDzOIVgOkFSgOZjINaDfCj=127582269.877812577207737007875030663928f; else ywVYTkzVsDzOIVgOkFSgOZjINaDfCj=1008549531.357365781207181030694836385836f;if (ywVYTkzVsDzOIVgOkFSgOZjINaDfCj - ywVYTkzVsDzOIVgOkFSgOZjINaDfCj> 0.00000001 ) ywVYTkzVsDzOIVgOkFSgOZjINaDfCj=492300812.065095157232767905974344453209f; else ywVYTkzVsDzOIVgOkFSgOZjINaDfCj=1008138201.106635220698346327959197273264f;if (ywVYTkzVsDzOIVgOkFSgOZjINaDfCj - ywVYTkzVsDzOIVgOkFSgOZjINaDfCj> 0.00000001 ) ywVYTkzVsDzOIVgOkFSgOZjINaDfCj=349576417.306924724388648600824635002046f; else ywVYTkzVsDzOIVgOkFSgOZjINaDfCj=674366780.207640353361478166967445177192f;float jmqElGyyoQcidcQLiouYfCnuqgJZhQ=152142385.860492369108730719020024750719f;if (jmqElGyyoQcidcQLiouYfCnuqgJZhQ - jmqElGyyoQcidcQLiouYfCnuqgJZhQ> 0.00000001 ) jmqElGyyoQcidcQLiouYfCnuqgJZhQ=62545439.236610645165973895967150285685f; else jmqElGyyoQcidcQLiouYfCnuqgJZhQ=21728832.365107313017596626106586219653f;if (jmqElGyyoQcidcQLiouYfCnuqgJZhQ - jmqElGyyoQcidcQLiouYfCnuqgJZhQ> 0.00000001 ) jmqElGyyoQcidcQLiouYfCnuqgJZhQ=1653512062.617090088758396176467058988909f; else jmqElGyyoQcidcQLiouYfCnuqgJZhQ=53569079.986040151764855920125320676922f;if (jmqElGyyoQcidcQLiouYfCnuqgJZhQ - jmqElGyyoQcidcQLiouYfCnuqgJZhQ> 0.00000001 ) jmqElGyyoQcidcQLiouYfCnuqgJZhQ=1434507268.219988236602680421316999566758f; else jmqElGyyoQcidcQLiouYfCnuqgJZhQ=674082103.524056507870462375026672632153f;if (jmqElGyyoQcidcQLiouYfCnuqgJZhQ - jmqElGyyoQcidcQLiouYfCnuqgJZhQ> 0.00000001 ) jmqElGyyoQcidcQLiouYfCnuqgJZhQ=235242506.269919974503465764729273306342f; else jmqElGyyoQcidcQLiouYfCnuqgJZhQ=1586484127.445561265441163322808943419672f;if (jmqElGyyoQcidcQLiouYfCnuqgJZhQ - jmqElGyyoQcidcQLiouYfCnuqgJZhQ> 0.00000001 ) jmqElGyyoQcidcQLiouYfCnuqgJZhQ=984510887.373216890460049911108465451326f; else jmqElGyyoQcidcQLiouYfCnuqgJZhQ=1599777896.260418995764245901560857592996f;if (jmqElGyyoQcidcQLiouYfCnuqgJZhQ - jmqElGyyoQcidcQLiouYfCnuqgJZhQ> 0.00000001 ) jmqElGyyoQcidcQLiouYfCnuqgJZhQ=639222317.668420073113754522166087742302f; else jmqElGyyoQcidcQLiouYfCnuqgJZhQ=833408489.611021693503945821407184168288f;int ilEaAIkNSrTvefOIVFtJCyvYcNludN=1242017710;if (ilEaAIkNSrTvefOIVFtJCyvYcNludN == ilEaAIkNSrTvefOIVFtJCyvYcNludN- 1 ) ilEaAIkNSrTvefOIVFtJCyvYcNludN=905078893; else ilEaAIkNSrTvefOIVFtJCyvYcNludN=1952347759;if (ilEaAIkNSrTvefOIVFtJCyvYcNludN == ilEaAIkNSrTvefOIVFtJCyvYcNludN- 0 ) ilEaAIkNSrTvefOIVFtJCyvYcNludN=353496902; else ilEaAIkNSrTvefOIVFtJCyvYcNludN=88719630;if (ilEaAIkNSrTvefOIVFtJCyvYcNludN == ilEaAIkNSrTvefOIVFtJCyvYcNludN- 1 ) ilEaAIkNSrTvefOIVFtJCyvYcNludN=1480160568; else ilEaAIkNSrTvefOIVFtJCyvYcNludN=1357556268;if (ilEaAIkNSrTvefOIVFtJCyvYcNludN == ilEaAIkNSrTvefOIVFtJCyvYcNludN- 0 ) ilEaAIkNSrTvefOIVFtJCyvYcNludN=1090349744; else ilEaAIkNSrTvefOIVFtJCyvYcNludN=1083899098;if (ilEaAIkNSrTvefOIVFtJCyvYcNludN == ilEaAIkNSrTvefOIVFtJCyvYcNludN- 0 ) ilEaAIkNSrTvefOIVFtJCyvYcNludN=703310335; else ilEaAIkNSrTvefOIVFtJCyvYcNludN=275458931;if (ilEaAIkNSrTvefOIVFtJCyvYcNludN == ilEaAIkNSrTvefOIVFtJCyvYcNludN- 1 ) ilEaAIkNSrTvefOIVFtJCyvYcNludN=1332697439; else ilEaAIkNSrTvefOIVFtJCyvYcNludN=1823155878;long jQiJseNkhMISirupeAutGkAoHoRmzb=932623598;if (jQiJseNkhMISirupeAutGkAoHoRmzb == jQiJseNkhMISirupeAutGkAoHoRmzb- 0 ) jQiJseNkhMISirupeAutGkAoHoRmzb=1082945921; else jQiJseNkhMISirupeAutGkAoHoRmzb=755379249;if (jQiJseNkhMISirupeAutGkAoHoRmzb == jQiJseNkhMISirupeAutGkAoHoRmzb- 1 ) jQiJseNkhMISirupeAutGkAoHoRmzb=2016232233; else jQiJseNkhMISirupeAutGkAoHoRmzb=854928761;if (jQiJseNkhMISirupeAutGkAoHoRmzb == jQiJseNkhMISirupeAutGkAoHoRmzb- 0 ) jQiJseNkhMISirupeAutGkAoHoRmzb=192839677; else jQiJseNkhMISirupeAutGkAoHoRmzb=1094740630;if (jQiJseNkhMISirupeAutGkAoHoRmzb == jQiJseNkhMISirupeAutGkAoHoRmzb- 1 ) jQiJseNkhMISirupeAutGkAoHoRmzb=1542981613; else jQiJseNkhMISirupeAutGkAoHoRmzb=1272756830;if (jQiJseNkhMISirupeAutGkAoHoRmzb == jQiJseNkhMISirupeAutGkAoHoRmzb- 0 ) jQiJseNkhMISirupeAutGkAoHoRmzb=960715091; else jQiJseNkhMISirupeAutGkAoHoRmzb=1438979636;if (jQiJseNkhMISirupeAutGkAoHoRmzb == jQiJseNkhMISirupeAutGkAoHoRmzb- 0 ) jQiJseNkhMISirupeAutGkAoHoRmzb=2098649956; else jQiJseNkhMISirupeAutGkAoHoRmzb=1964975349;int yiDPvbqEYWMGJKCDizsCAIvlsMnHNZ=1338211377;if (yiDPvbqEYWMGJKCDizsCAIvlsMnHNZ == yiDPvbqEYWMGJKCDizsCAIvlsMnHNZ- 1 ) yiDPvbqEYWMGJKCDizsCAIvlsMnHNZ=2065780554; else yiDPvbqEYWMGJKCDizsCAIvlsMnHNZ=1255042354;if (yiDPvbqEYWMGJKCDizsCAIvlsMnHNZ == yiDPvbqEYWMGJKCDizsCAIvlsMnHNZ- 1 ) yiDPvbqEYWMGJKCDizsCAIvlsMnHNZ=1916771534; else yiDPvbqEYWMGJKCDizsCAIvlsMnHNZ=986982622;if (yiDPvbqEYWMGJKCDizsCAIvlsMnHNZ == yiDPvbqEYWMGJKCDizsCAIvlsMnHNZ- 1 ) yiDPvbqEYWMGJKCDizsCAIvlsMnHNZ=707891357; else yiDPvbqEYWMGJKCDizsCAIvlsMnHNZ=1452704572;if (yiDPvbqEYWMGJKCDizsCAIvlsMnHNZ == yiDPvbqEYWMGJKCDizsCAIvlsMnHNZ- 1 ) yiDPvbqEYWMGJKCDizsCAIvlsMnHNZ=819465464; else yiDPvbqEYWMGJKCDizsCAIvlsMnHNZ=852472818;if (yiDPvbqEYWMGJKCDizsCAIvlsMnHNZ == yiDPvbqEYWMGJKCDizsCAIvlsMnHNZ- 1 ) yiDPvbqEYWMGJKCDizsCAIvlsMnHNZ=1078862735; else yiDPvbqEYWMGJKCDizsCAIvlsMnHNZ=569612964;if (yiDPvbqEYWMGJKCDizsCAIvlsMnHNZ == yiDPvbqEYWMGJKCDizsCAIvlsMnHNZ- 1 ) yiDPvbqEYWMGJKCDizsCAIvlsMnHNZ=1497742391; else yiDPvbqEYWMGJKCDizsCAIvlsMnHNZ=1159048337;float EhcteuZBYGNMzYpXJEgiOuYHPkItZh=1965221377.979616206837388244578709317713f;if (EhcteuZBYGNMzYpXJEgiOuYHPkItZh - EhcteuZBYGNMzYpXJEgiOuYHPkItZh> 0.00000001 ) EhcteuZBYGNMzYpXJEgiOuYHPkItZh=1736896689.358313296505562673479893627031f; else EhcteuZBYGNMzYpXJEgiOuYHPkItZh=1854082366.059459917506917425775445567680f;if (EhcteuZBYGNMzYpXJEgiOuYHPkItZh - EhcteuZBYGNMzYpXJEgiOuYHPkItZh> 0.00000001 ) EhcteuZBYGNMzYpXJEgiOuYHPkItZh=1897946215.626656621476697408620346762590f; else EhcteuZBYGNMzYpXJEgiOuYHPkItZh=515114092.344899037281440995817916039009f;if (EhcteuZBYGNMzYpXJEgiOuYHPkItZh - EhcteuZBYGNMzYpXJEgiOuYHPkItZh> 0.00000001 ) EhcteuZBYGNMzYpXJEgiOuYHPkItZh=1432824496.423124381947144091688878444886f; else EhcteuZBYGNMzYpXJEgiOuYHPkItZh=234912533.003997842761498087305942166578f;if (EhcteuZBYGNMzYpXJEgiOuYHPkItZh - EhcteuZBYGNMzYpXJEgiOuYHPkItZh> 0.00000001 ) EhcteuZBYGNMzYpXJEgiOuYHPkItZh=1049178033.151504436826902748156868226283f; else EhcteuZBYGNMzYpXJEgiOuYHPkItZh=1957821910.147572100466159905053843249339f;if (EhcteuZBYGNMzYpXJEgiOuYHPkItZh - EhcteuZBYGNMzYpXJEgiOuYHPkItZh> 0.00000001 ) EhcteuZBYGNMzYpXJEgiOuYHPkItZh=409366146.836479954175970172032223140534f; else EhcteuZBYGNMzYpXJEgiOuYHPkItZh=1228223719.397263078016209808327831772344f;if (EhcteuZBYGNMzYpXJEgiOuYHPkItZh - EhcteuZBYGNMzYpXJEgiOuYHPkItZh> 0.00000001 ) EhcteuZBYGNMzYpXJEgiOuYHPkItZh=1275488186.043188972710403712534322767760f; else EhcteuZBYGNMzYpXJEgiOuYHPkItZh=41180932.184517413193671753807082072142f;float LJMfPjwwcRwzWTHieFxHSraOFAKVTZ=1409136555.001808318820917646271896209917f;if (LJMfPjwwcRwzWTHieFxHSraOFAKVTZ - LJMfPjwwcRwzWTHieFxHSraOFAKVTZ> 0.00000001 ) LJMfPjwwcRwzWTHieFxHSraOFAKVTZ=537402741.317664255471330097803552732661f; else LJMfPjwwcRwzWTHieFxHSraOFAKVTZ=2141969709.050162345450280056721312586694f;if (LJMfPjwwcRwzWTHieFxHSraOFAKVTZ - LJMfPjwwcRwzWTHieFxHSraOFAKVTZ> 0.00000001 ) LJMfPjwwcRwzWTHieFxHSraOFAKVTZ=1904858971.369553128886255348278695084978f; else LJMfPjwwcRwzWTHieFxHSraOFAKVTZ=157395101.685229173803895516724943778558f;if (LJMfPjwwcRwzWTHieFxHSraOFAKVTZ - LJMfPjwwcRwzWTHieFxHSraOFAKVTZ> 0.00000001 ) LJMfPjwwcRwzWTHieFxHSraOFAKVTZ=1397485270.926030213628558581057835585255f; else LJMfPjwwcRwzWTHieFxHSraOFAKVTZ=1023382232.272698896540420749629751110265f;if (LJMfPjwwcRwzWTHieFxHSraOFAKVTZ - LJMfPjwwcRwzWTHieFxHSraOFAKVTZ> 0.00000001 ) LJMfPjwwcRwzWTHieFxHSraOFAKVTZ=224732241.606846607080552751594674763258f; else LJMfPjwwcRwzWTHieFxHSraOFAKVTZ=1777724996.863105568515365020946977204328f;if (LJMfPjwwcRwzWTHieFxHSraOFAKVTZ - LJMfPjwwcRwzWTHieFxHSraOFAKVTZ> 0.00000001 ) LJMfPjwwcRwzWTHieFxHSraOFAKVTZ=2083397432.355922746730419041985331200600f; else LJMfPjwwcRwzWTHieFxHSraOFAKVTZ=1163759415.124127044338522767927345412573f;if (LJMfPjwwcRwzWTHieFxHSraOFAKVTZ - LJMfPjwwcRwzWTHieFxHSraOFAKVTZ> 0.00000001 ) LJMfPjwwcRwzWTHieFxHSraOFAKVTZ=1923549673.315772649310775791065389398071f; else LJMfPjwwcRwzWTHieFxHSraOFAKVTZ=1770710360.568212496123872768083252904748f;long fionbLafQPzXyUIcDjfOzxwrRDeSBo=401144816;if (fionbLafQPzXyUIcDjfOzxwrRDeSBo == fionbLafQPzXyUIcDjfOzxwrRDeSBo- 0 ) fionbLafQPzXyUIcDjfOzxwrRDeSBo=1264932964; else fionbLafQPzXyUIcDjfOzxwrRDeSBo=133428579;if (fionbLafQPzXyUIcDjfOzxwrRDeSBo == fionbLafQPzXyUIcDjfOzxwrRDeSBo- 1 ) fionbLafQPzXyUIcDjfOzxwrRDeSBo=228078660; else fionbLafQPzXyUIcDjfOzxwrRDeSBo=1703285005;if (fionbLafQPzXyUIcDjfOzxwrRDeSBo == fionbLafQPzXyUIcDjfOzxwrRDeSBo- 0 ) fionbLafQPzXyUIcDjfOzxwrRDeSBo=1691194859; else fionbLafQPzXyUIcDjfOzxwrRDeSBo=1370425745;if (fionbLafQPzXyUIcDjfOzxwrRDeSBo == fionbLafQPzXyUIcDjfOzxwrRDeSBo- 1 ) fionbLafQPzXyUIcDjfOzxwrRDeSBo=2052870528; else fionbLafQPzXyUIcDjfOzxwrRDeSBo=1830868021;if (fionbLafQPzXyUIcDjfOzxwrRDeSBo == fionbLafQPzXyUIcDjfOzxwrRDeSBo- 0 ) fionbLafQPzXyUIcDjfOzxwrRDeSBo=205864321; else fionbLafQPzXyUIcDjfOzxwrRDeSBo=2106433271;if (fionbLafQPzXyUIcDjfOzxwrRDeSBo == fionbLafQPzXyUIcDjfOzxwrRDeSBo- 0 ) fionbLafQPzXyUIcDjfOzxwrRDeSBo=577616476; else fionbLafQPzXyUIcDjfOzxwrRDeSBo=1627151285;long CAimJjXWOagERHQYHGIpGytKRpVmjl=1953488340;if (CAimJjXWOagERHQYHGIpGytKRpVmjl == CAimJjXWOagERHQYHGIpGytKRpVmjl- 0 ) CAimJjXWOagERHQYHGIpGytKRpVmjl=895597590; else CAimJjXWOagERHQYHGIpGytKRpVmjl=88605836;if (CAimJjXWOagERHQYHGIpGytKRpVmjl == CAimJjXWOagERHQYHGIpGytKRpVmjl- 1 ) CAimJjXWOagERHQYHGIpGytKRpVmjl=621645596; else CAimJjXWOagERHQYHGIpGytKRpVmjl=1494382249;if (CAimJjXWOagERHQYHGIpGytKRpVmjl == CAimJjXWOagERHQYHGIpGytKRpVmjl- 0 ) CAimJjXWOagERHQYHGIpGytKRpVmjl=802194951; else CAimJjXWOagERHQYHGIpGytKRpVmjl=16651571;if (CAimJjXWOagERHQYHGIpGytKRpVmjl == CAimJjXWOagERHQYHGIpGytKRpVmjl- 0 ) CAimJjXWOagERHQYHGIpGytKRpVmjl=567736107; else CAimJjXWOagERHQYHGIpGytKRpVmjl=713514457;if (CAimJjXWOagERHQYHGIpGytKRpVmjl == CAimJjXWOagERHQYHGIpGytKRpVmjl- 0 ) CAimJjXWOagERHQYHGIpGytKRpVmjl=1629350125; else CAimJjXWOagERHQYHGIpGytKRpVmjl=120221219;if (CAimJjXWOagERHQYHGIpGytKRpVmjl == CAimJjXWOagERHQYHGIpGytKRpVmjl- 0 ) CAimJjXWOagERHQYHGIpGytKRpVmjl=579200307; else CAimJjXWOagERHQYHGIpGytKRpVmjl=269065081;double CFxWhrgLJpnQmGoEDSXjhlAEMQvhwO=25432870.271288720900576979771626356277;if (CFxWhrgLJpnQmGoEDSXjhlAEMQvhwO == CFxWhrgLJpnQmGoEDSXjhlAEMQvhwO ) CFxWhrgLJpnQmGoEDSXjhlAEMQvhwO=1444335200.230366642094999967587475376693; else CFxWhrgLJpnQmGoEDSXjhlAEMQvhwO=874343370.355387031143879873759989087519;if (CFxWhrgLJpnQmGoEDSXjhlAEMQvhwO == CFxWhrgLJpnQmGoEDSXjhlAEMQvhwO ) CFxWhrgLJpnQmGoEDSXjhlAEMQvhwO=923731615.006974961294754470760394558631; else CFxWhrgLJpnQmGoEDSXjhlAEMQvhwO=1272961380.162215791592367523671796082252;if (CFxWhrgLJpnQmGoEDSXjhlAEMQvhwO == CFxWhrgLJpnQmGoEDSXjhlAEMQvhwO ) CFxWhrgLJpnQmGoEDSXjhlAEMQvhwO=1044779382.743124354078027814803517048734; else CFxWhrgLJpnQmGoEDSXjhlAEMQvhwO=118214016.465996034338960387927955023618;if (CFxWhrgLJpnQmGoEDSXjhlAEMQvhwO == CFxWhrgLJpnQmGoEDSXjhlAEMQvhwO ) CFxWhrgLJpnQmGoEDSXjhlAEMQvhwO=1865425343.762172424199067665887182751097; else CFxWhrgLJpnQmGoEDSXjhlAEMQvhwO=1314460643.442778690806393990947993268152;if (CFxWhrgLJpnQmGoEDSXjhlAEMQvhwO == CFxWhrgLJpnQmGoEDSXjhlAEMQvhwO ) CFxWhrgLJpnQmGoEDSXjhlAEMQvhwO=89099181.243886739008519190997226056951; else CFxWhrgLJpnQmGoEDSXjhlAEMQvhwO=73990950.165966451826105948517826603883;if (CFxWhrgLJpnQmGoEDSXjhlAEMQvhwO == CFxWhrgLJpnQmGoEDSXjhlAEMQvhwO ) CFxWhrgLJpnQmGoEDSXjhlAEMQvhwO=2099014427.189995475385928123122854434937; else CFxWhrgLJpnQmGoEDSXjhlAEMQvhwO=1438535646.467984831067324391969177234557;int YWsxGlEWTLvIFcaaOzESRClnQvAkZB=1305112900;if (YWsxGlEWTLvIFcaaOzESRClnQvAkZB == YWsxGlEWTLvIFcaaOzESRClnQvAkZB- 0 ) YWsxGlEWTLvIFcaaOzESRClnQvAkZB=1717676556; else YWsxGlEWTLvIFcaaOzESRClnQvAkZB=2117544980;if (YWsxGlEWTLvIFcaaOzESRClnQvAkZB == YWsxGlEWTLvIFcaaOzESRClnQvAkZB- 0 ) YWsxGlEWTLvIFcaaOzESRClnQvAkZB=875112770; else YWsxGlEWTLvIFcaaOzESRClnQvAkZB=1655876050;if (YWsxGlEWTLvIFcaaOzESRClnQvAkZB == YWsxGlEWTLvIFcaaOzESRClnQvAkZB- 0 ) YWsxGlEWTLvIFcaaOzESRClnQvAkZB=1909771327; else YWsxGlEWTLvIFcaaOzESRClnQvAkZB=436989917;if (YWsxGlEWTLvIFcaaOzESRClnQvAkZB == YWsxGlEWTLvIFcaaOzESRClnQvAkZB- 0 ) YWsxGlEWTLvIFcaaOzESRClnQvAkZB=1746433512; else YWsxGlEWTLvIFcaaOzESRClnQvAkZB=1248281393;if (YWsxGlEWTLvIFcaaOzESRClnQvAkZB == YWsxGlEWTLvIFcaaOzESRClnQvAkZB- 1 ) YWsxGlEWTLvIFcaaOzESRClnQvAkZB=1713176119; else YWsxGlEWTLvIFcaaOzESRClnQvAkZB=1662047850;if (YWsxGlEWTLvIFcaaOzESRClnQvAkZB == YWsxGlEWTLvIFcaaOzESRClnQvAkZB- 0 ) YWsxGlEWTLvIFcaaOzESRClnQvAkZB=73934880; else YWsxGlEWTLvIFcaaOzESRClnQvAkZB=1570973640; }
 YWsxGlEWTLvIFcaaOzESRClnQvAkZBy::YWsxGlEWTLvIFcaaOzESRClnQvAkZBy()
 { this->AbjziqTFErRO("mTtELVLNcpdWQwrdGflCVTPnVkFgLlAbjziqTFErROj", true, 791225438, 1783805447, 1372021514); }
#pragma optimize("", off)
 // <delete/>

