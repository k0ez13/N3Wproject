#include "../demonsense/menu/menu.h"
#include "../demonsense/menu/ui.h"
#include <deque>
#include <algorithm>
#include <vector>

//template <class T>
//bool ComboForSkins(const char* label, std::vector current_item, bool(*items_getter)(void* data, int idx, const char** out_text), void* data, int items_count, int popup_max_height_in_items = -1) {
//	
//}
void ImGui::resetcurspos()
{
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	window->DC.CursorPos = ImVec2(window->Pos.x, window->Pos.y + 18);


	window->DrawList->AddText(ImVec2(window->Pos.x, window->Pos.y + 18), GetColorU32(ImVec4(1.f, 1.f, 1.f, 1.f)), "1");
}

bool ImGui::ToggleButtonMain(const char* label, bool* v, const ImVec2& size_arg, int side)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return false;

	int flags = 0;
	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(label);
	const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

	ImVec2 pos = window->DC.CursorPos;
	ImVec2 size = ImGui::CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

	const ImRect bb(pos, pos + size);
	ImGui::ItemSize(bb, style.FramePadding.y);
	if (!ImGui::ItemAdd(bb, id))
		return false;

	if (window->DC.ItemFlags & ImGuiItemFlags_ButtonRepeat) flags |= ImGuiButtonFlags_Repeat;
	bool hovered, held;
	bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, flags);

	// Render
	const ImU32 col = ImGui::GetColorU32(*v ? ImGuiCol_TitleBg : (hovered ? ImGuiCol_ChildBg : ImGuiCol_WindowBg));
	//ImGui::RenderFrame(bb.Min, bb.Max, col, true, style.FrameRounding);
	switch (side)
	{
	case 0:
		window->DrawList->AddRectFilled(bb.Min, bb.Max, col, 0.f);
		break;
	case 1:
		window->DrawList->AddRectFilled(bb.Min, bb.Max, col, style.FrameRounding, ImDrawCornerFlags_BotLeft);
		break;
	case 2:
		window->DrawList->AddRectFilled(bb.Min, bb.Max, col, style.FrameRounding, ImDrawCornerFlags_BotRight);
		break;
	}

	if (*v)
	{
		ImGui::RenderTextClipped(bb.Min + style.FramePadding, bb.Max - style.FramePadding - ImVec2(0, 8), label, NULL, &label_size, style.ButtonTextAlign, &bb);
	}
	else
	{
		ImGui::RenderTextClipped(bb.Min + style.FramePadding, bb.Max - style.FramePadding, label, NULL, &label_size, style.ButtonTextAlign, &bb);
	}

	if (*v)
	{
		switch (side)
		{
		case 0:
			window->DrawList->AddRectFilled(ImVec2(bb.Min.x, bb.Max.y - 4), bb.Max, ImGui::GetColorU32(ImGuiCol_SliderGrab), 0.f);
			break;
		case 1:
			window->DrawList->AddRectFilled(ImVec2(bb.Min.x, bb.Max.y - 4), bb.Max, ImGui::GetColorU32(ImGuiCol_SliderGrab), style.FrameRounding, ImDrawCornerFlags_BotLeft);
			break;
		case 2:
			window->DrawList->AddRectFilled(ImVec2(bb.Min.x, bb.Max.y - 4), bb.Max, ImGui::GetColorU32(ImGuiCol_SliderGrab), style.FrameRounding, ImDrawCornerFlags_BotRight);
			break;
		}
		//window->DrawList->AddRectFilledMultiColor(ImVec2(bb.Min.x, bb.Max.y - 1), bb.Max, ImGui::GetColorU32(ImGuiCol_SliderGrab), ImGui::GetColorU32(ImGuiCol_SliderGrabActive), ImGui::GetColorU32(ImGuiCol_SliderGrabActive), ImGui::GetColorU32(ImGuiCol_SliderGrab));
	}
	if (pressed)
		*v = !*v;

	return pressed;

	/*ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return false;

	int flags = 0;
	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(label);
	const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

	ImVec2 pos = window->DC.CursorPos;
	ImVec2 size = ImGui::CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

	const ImRect bb(pos, pos + size);
	ImGui::ItemSize(bb, style.FramePadding.y);
	if (!ImGui::ItemAdd(bb, id))
		return false;

	if (window->DC.ItemFlags & ImGuiItemFlags_ButtonRepeat) flags |= ImGuiButtonFlags_Repeat;
	bool hovered, held;
	bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, flags);

	// Render
	const ImU32 col = ImGui::GetColorU32(*v ? ImGuiCol_WindowBg : ((hovered || held) ? ImGuiCol_ButtonHovered : ImGuiCol_Button));

	ImGui::RenderFrame(bb.Min, bb.Max, col, true, style.FrameRounding);
	ImGui::RenderTextClipped(bb.Min + style.FramePadding, bb.Max - style.FramePadding, label, NULL, &label_size, style.ButtonTextAlign, &bb);

	if (pressed)
		*v = !*v;

	return pressed;*/
}

bool ImGui::ToggleButtonSelect(const char* label, bool* v, const ImVec2& size_arg, int side, bool KeyBind)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return false;

	int flags = 0;
	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(label);
	const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

	ImVec2 pos = window->DC.CursorPos;
	ImVec2 size = ImGui::CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

	const ImRect bb(pos, pos + size);
	ImGui::ItemSize(bb, style.FramePadding.y);
	if (!ImGui::ItemAdd(bb, id))
		return false;

	if (window->DC.ItemFlags & ImGuiItemFlags_ButtonRepeat) flags |= ImGuiButtonFlags_Repeat;
	bool hovered, held;
	bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, flags);

	// Render
	const ImU32 col = KeyBind ? (ImGui::GetColorU32(*v ? ImGuiCol_ButtonActive : (hovered ? ImGuiCol_TitleBg : ImGuiCol_ButtonHovered))) : (ImGui::GetColorU32(*v ? ImGuiCol_TitleBg : (hovered ? ImGuiCol_ChildBg : ImGuiCol_WindowBg)));
	//ImGui::RenderFrame(bb.Min, bb.Max, col, true, style.FrameRounding);
	switch (side)
	{
	case 0:
		window->DrawList->AddRectFilled(bb.Min, bb.Max, col, 0.f);
		break;
	case 1:
		window->DrawList->AddRectFilled(bb.Min, bb.Max, col, style.FrameRounding, ImDrawCornerFlags_Top);
		break;
	case 2:
		window->DrawList->AddRectFilled(bb.Min, bb.Max, col, style.FrameRounding, ImDrawCornerFlags_Bot);
		break;
	}

	if (!KeyBind)
	{
		if (*v)
		{
			ImGui::RenderTextClipped(ImVec2(bb.Min.x + 16.f, bb.Min.y + style.FramePadding.y), ImVec2(bb.Min.x + 16.f, bb.Max.y - style.FramePadding.y), label, NULL, &label_size, style.ButtonTextAlign, &bb);
		}
		else
		{
			ImGui::RenderTextClipped(ImVec2(bb.Min.x + 8.f, bb.Min.y + style.FramePadding.y), ImVec2(bb.Min.x + 8.f, bb.Max.y - style.FramePadding.y), label, NULL, &label_size, style.ButtonTextAlign, &bb);
		}
	}
	else
	{
		if (*v)
		{
			ImGui::RenderTextClipped(ImVec2(bb.Min.x, bb.Min.y + style.FramePadding.y), ImVec2(bb.Max.x, bb.Max.y - style.FramePadding.y), label, NULL, &label_size, style.ButtonTextAlign, &bb);
		}
		else
		{
			ImGui::RenderTextClipped(ImVec2(bb.Min.x, bb.Min.y + style.FramePadding.y), ImVec2(bb.Max.x, bb.Max.y - style.FramePadding.y), label, NULL, &label_size, style.ButtonTextAlign, &bb);
		}
	}

	if (*v)
	{
		switch (side)
		{
		case 0:
			window->DrawList->AddRectFilled(ImVec2(bb.Min.x, bb.Min.y), ImVec2(bb.Min.x + 4.f, bb.Max.y), ImGui::GetColorU32(ImGuiCol_SliderGrab), 0.f);
			break;
		case 1:
			window->DrawList->AddRectFilled(ImVec2(bb.Min.x, bb.Min.y), ImVec2(bb.Min.x + 4.f, bb.Max.y), ImGui::GetColorU32(ImGuiCol_SliderGrab), style.FrameRounding, ImDrawCornerFlags_TopLeft);
			break;
		case 2:
			window->DrawList->AddRectFilled(ImVec2(bb.Min.x, bb.Min.y), ImVec2(bb.Min.x + 4.f, bb.Max.y), ImGui::GetColorU32(ImGuiCol_SliderGrab), style.FrameRounding, ImDrawCornerFlags_BotLeft);
			break;
		}
		//window->DrawList->AddRectFilledMultiColor(ImVec2(bb.Min.x, bb.Max.y - 1), bb.Max, ImGui::GetColorU32(ImGuiCol_SliderGrab), ImGui::GetColorU32(ImGuiCol_SliderGrabActive), ImGui::GetColorU32(ImGuiCol_SliderGrabActive), ImGui::GetColorU32(ImGuiCol_SliderGrab));
	}
	if (pressed)
		*v = !*v;

	return pressed;

	/*ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return false;

	int flags = 0;
	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(label);
	const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

	ImVec2 pos = window->DC.CursorPos;
	ImVec2 size = ImGui::CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

	const ImRect bb(pos, pos + size);
	ImGui::ItemSize(bb, style.FramePadding.y);
	if (!ImGui::ItemAdd(bb, id))
		return false;

	if (window->DC.ItemFlags & ImGuiItemFlags_ButtonRepeat) flags |= ImGuiButtonFlags_Repeat;
	bool hovered, held;
	bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, flags);

	// Render
	const ImU32 col = ImGui::GetColorU32(*v ? ImGuiCol_WindowBg : ((hovered || held) ? ImGuiCol_ButtonHovered : ImGuiCol_Button));

	ImGui::RenderFrame(bb.Min, bb.Max, col, true, style.FrameRounding);
	ImGui::RenderTextClipped(bb.Min + style.FramePadding, bb.Max - style.FramePadding, label, NULL, &label_size, style.ButtonTextAlign, &bb);

	if (pressed)
		*v = !*v;

	return pressed;*/
}

bool ImGui::ToggleButton(const char* label, bool* v, const ImVec2& size_arg, int side /* 1 - left; 2 - right*/)
{

	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return false;

	int flags = 0;
	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(label);
	const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

	ImVec2 pos = window->DC.CursorPos;
	ImVec2 size = ImGui::CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

	const ImRect bb(pos, pos + size);
	ImGui::ItemSize(bb, style.FramePadding.y);
	if (!ImGui::ItemAdd(bb, id))
		return false;

	if (window->DC.ItemFlags & ImGuiItemFlags_ButtonRepeat) flags |= ImGuiButtonFlags_Repeat;
	bool hovered, held;
	bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, flags);

	// Render
	const ImU32 col = ImGui::GetColorU32((hovered && held || *v) ? ImGuiCol_ButtonActive : (hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button));
	//ImGui::RenderFrame(bb.Min, bb.Max, col, true, style.FrameRounding);
	switch (side)
	{
	case 0:
		window->DrawList->AddRectFilled(bb.Min, bb.Max, col, 0.f);
		break;
	case 1:
		window->DrawList->AddRectFilled(bb.Min, bb.Max, col, style.FrameRounding, ImDrawCornerFlags_Left);
		break;
	case 2:
		window->DrawList->AddRectFilled(bb.Min, bb.Max, col, style.FrameRounding, ImDrawCornerFlags_Right);
		break;
	}

	if (*v)
	{
		ImGui::RenderTextClipped(bb.Min + style.FramePadding, bb.Max - style.FramePadding - ImVec2(0, 8), label, NULL, &label_size, style.ButtonTextAlign, &bb);
	}
	else
	{
		ImGui::RenderTextClipped(bb.Min + style.FramePadding, bb.Max - style.FramePadding, label, NULL, &label_size, style.ButtonTextAlign, &bb);
	}

	if (*v)
	{
		switch (side)
		{
		case 0:
			window->DrawList->AddRectFilled(ImVec2(bb.Min.x, bb.Max.y - 4), bb.Max, ImGui::GetColorU32(ImGuiCol_SliderGrab), 0.f);
			break;
		case 1:
			window->DrawList->AddRectFilled(ImVec2(bb.Min.x, bb.Max.y - 4), bb.Max, ImGui::GetColorU32(ImGuiCol_SliderGrab), style.FrameRounding, ImDrawCornerFlags_BotLeft);
			break;
		case 2:
			window->DrawList->AddRectFilled(ImVec2(bb.Min.x, bb.Max.y - 4), bb.Max, ImGui::GetColorU32(ImGuiCol_SliderGrab), style.FrameRounding, ImDrawCornerFlags_BotRight);
			break;
		}
		//window->DrawList->AddRectFilledMultiColor(ImVec2(bb.Min.x, bb.Max.y - 1), bb.Max, ImGui::GetColorU32(ImGuiCol_SliderGrab), ImGui::GetColorU32(ImGuiCol_SliderGrabActive), ImGui::GetColorU32(ImGuiCol_SliderGrabActive), ImGui::GetColorU32(ImGuiCol_SliderGrab));
	}
	if (pressed)
		*v = !*v;

	return pressed;
}

static bool Items_ArrayGetter(void* data, int idx, const char** out_text)
{
	const char* const* items = (const char* const*)data;
	if (out_text)
		*out_text = items[idx];
	return true;
}

static auto vector_getter = [](void* vec, int idx, const char** out_text) {
	auto& vector = *static_cast<std::vector<std::string>*>(vec);
	if (idx < 0 || idx >= static_cast<int>(vector.size())) { return false; }
	*out_text = vector.at(idx).c_str();
	return true;
};

bool ImGui::Combo(const char* label, int* currIndex, std::vector<std::string>& values) {
	if (values.empty()) { return false; }
	return ImGui::Combo(label, currIndex, vector_getter,
		static_cast<void*>(&values), values.size());
}

bool ImGui::BeginGroupBox(const char* name, const ImVec2& size_arg)
{
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_ChildWindow;

	window->DC.CursorPos.y += GImGui->FontSize / 2;
	const ImVec2 content_avail = ImGui::GetContentRegionAvail();
	ImVec2 size = ImFloor(size_arg);
	if (size.x <= 0.0f) {
		size.x = ImMax(content_avail.x, 4.0f) - fabsf(size.x); // Arbitrary minimum zero-ish child size of 4.0f (0.0f causing too much issues)
	}
	if (size.y <= 0.0f) {
		size.y = ImMax(content_avail.y, 4.0f) - fabsf(size.y);
	}

	ImVec4 becup_color_body = g.Style.Colors[ImGuiCol_ChildBg];
	ImVec4 becup_color_bord = g.Style.Colors[ImGuiCol_Border];
	//float becup_child_bord = g.Style.ChildBorderSize;

	g.Style.Colors[ImGuiCol_Border] = ImVec4(1.00f, 1.00f, 1.00f, 0.00f);
	g.Style.Colors[ImGuiCol_ChildBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.00f);
	//g.Style.ChildBorderSize = 0.f;

	ImGui::SetNextWindowSize(size);
	bool ret = ImGui::BeginChild(name, size, true);
	//ImGui::Begin(name, &ret, flags);

	//g.Style.ChildBorderSize = becup_child_bord;
	g.Style.Colors[ImGuiCol_Border] = becup_color_bord;
	g.Style.Colors[ImGuiCol_ChildBg] = becup_color_body;

	window = ImGui::GetCurrentWindow();

	auto padding = ImGui::GetStyle().WindowPadding;

	auto text_size = ImGui::CalcTextSize(name, NULL, true);

	if (text_size.x > 1.0f) {
		window->DrawList->PushClipRectFullScreen();
		//window->DrawList->AddRectFilled(window->DC.CursorPos - ImVec2{ 4, 0 }, window->DC.CursorPos + (text_size + ImVec2{ 4, 0 }), GetColorU32(ImGuiCol_ChildWindowBg));
		//RenderTextClipped(pos, pos + text_size, name, NULL, NULL, GetColorU32(ImGuiCol_Text));
		window->DrawList->PopClipRect();
	}
	//if (!(window->Flags & ImGuiWindowFlags_ShowBorders))
	//	ImGui::GetCurrentWindow()->Flags &= ~ImGuiWindowFlags_ShowBorders;

	return ret;
}

void ImGui::EndGroupBox()
{
	ImGui::EndChild();
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	window->DC.CursorPosPrevLine.y -= GImGui->FontSize / 2;
}

bool ImGui::Hotkey(const char* label, int* k, const ImVec2& size_arg)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	ImGuiIO& io = g.IO;
	const ImGuiStyle& style = g.Style;

	const ImGuiID id = window->GetID(label);
	const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);
	ImVec2 size = ImGui::CalcItemSize(size_arg, ImGui::CalcItemWidth(), label_size.y + style.FramePadding.y * 2.0f);
	const ImRect frame_bb(window->DC.CursorPos + ImVec2(label_size.x + style.ItemInnerSpacing.x, 0.0f), window->DC.CursorPos + size);
	const ImRect total_bb(window->DC.CursorPos, frame_bb.Max);

	ImGui::ItemSize(total_bb, style.FramePadding.y);
	if (!ImGui::ItemAdd(total_bb, id))
		return false;

	const bool focus_requested = ImGui::FocusableItemRegister(window, g.ActiveId == id, false);
	const bool focus_requested_by_code = focus_requested && (window->FocusIdxAllCounter == window->FocusIdxAllRequestCurrent);
	const bool focus_requested_by_tab = focus_requested && !focus_requested_by_code;

	const bool hovered = ImGui::ItemHoverable(frame_bb, id);

	if (hovered) {
		ImGui::SetHoveredID(id);
		g.MouseCursor = ImGuiMouseCursor_TextInput;
	}

	const bool user_clicked = hovered && io.MouseClicked[0];

	if (focus_requested || user_clicked) {
		if (g.ActiveId != id) {
			// Start edition
			memset(io.MouseDown, 0, sizeof(io.MouseDown));
			memset(io.KeysDown, 0, sizeof(io.KeysDown));
			*k = 0;
		}
		ImGui::SetActiveID(id, window);
		ImGui::FocusWindow(window);
	}
	else if (io.MouseClicked[0]) {
		// Release focus when we click outside
		if (g.ActiveId == id)
			ImGui::ClearActiveID();
	}

	bool value_changed = false;
	int key = *k;

	if (g.ActiveId == id) {
		for (auto i = 0; i < 5; i++) {
			if (io.MouseDown[i]) {
				switch (i) {
				case 0:
					key = VK_LBUTTON;
					break;
				case 1:
					key = VK_RBUTTON;
					break;
				case 2:
					key = VK_MBUTTON;
					break;
				case 3:
					key = VK_XBUTTON1;
					break;
				case 4:
					key = VK_XBUTTON2;
					break;
				}
				value_changed = true;
				ImGui::ClearActiveID();
			}
		}
		if (!value_changed) {
			for (auto i = VK_BACK; i <= VK_RMENU; i++) {
				if (io.KeysDown[i]) {
					key = i;
					value_changed = true;
					ImGui::ClearActiveID();
				}
			}
		}

		if (IsKeyPressedMap(ImGuiKey_Escape)) {
			*k = 0;
			ImGui::ClearActiveID();
		}
		else {
			*k = key;
		}
	}

	// Render
	// Select which buffer we are going to display. When ImGuiInputTextFlags_NoLiveEdit is Set 'buf' might still be the old value. We Set buf to NULL to prevent accidental usage from now on.

	char buf_display[64] = "None";

	ImGui::RenderFrame(frame_bb.Min, frame_bb.Max, ImGui::GetColorU32(ImVec4(0.20f, 0.25f, 0.30f, 1.0f)), true, style.FrameRounding);

	if (*k != 0 && g.ActiveId != id) {
		strcpy_s(buf_display, KeyNames[*k]);
	}
	else if (g.ActiveId == id) {
		strcpy_s(buf_display, "<Press a key>");
	}

	const ImRect clip_rect(frame_bb.Min.x, frame_bb.Min.y, frame_bb.Min.x + size.x, frame_bb.Min.y + size.y); // Not using frame_bb.Max because we have adjusted size
	ImVec2 render_pos = frame_bb.Min + style.FramePadding;
	ImGui::RenderTextClipped(frame_bb.Min + style.FramePadding, frame_bb.Max - style.FramePadding, buf_display, NULL, NULL, style.ButtonTextAlign, &clip_rect);
	//RenderTextClipped(frame_bb.Min + style.FramePadding, frame_bb.Max - style.FramePadding, buf_display, NULL, NULL, GetColorU32(ImGuiCol_Text), style.ButtonTextAlign, &clip_rect);
	//draw_window->DrawList->AddText(g.Font, g.FontSize, render_pos, GetColorU32(ImGuiCol_Text), buf_display, NULL, 0.0f, &clip_rect);

	if (label_size.x > 0)
		ImGui::RenderText(ImVec2(total_bb.Min.x, frame_bb.Min.y + style.FramePadding.y), label);

	return value_changed;
}


bool ImGui::ListBox(const char* label, int* current_item, std::string items[], int items_count, int height_items) {
	char** tmp;
	tmp = new char* [items_count];//(char**)malloc(sizeof(char*) * items_count);
	for (int i = 0; i < items_count; i++) {
		//tmp[i] = new char[items[i].size()];//(char*)malloc(sizeof(char*));
		tmp[i] = const_cast<char*>(items[i].c_str());
	}

	const bool value_changed = ImGui::ListBox(label, current_item, Items_ArrayGetter, static_cast<void*>(tmp), items_count, height_items);
	return value_changed;
}

bool ImGui::ListBox(const char* label, int* current_item, std::function<const char* (int)> lambda, int items_count, int height_in_items)
{
	return ImGui::ListBox(label, current_item, [](void* data, int idx, const char** out_text)
		{
			*out_text = (*reinterpret_cast<std::function<const char* (int)>*>(data))(idx);
			return true;
		}, &lambda, items_count, height_in_items);
}

bool ImGui::Combo(const char* label, int* current_item, std::function<const char* (int)> lambda, int items_count, int height_in_items)
{
	return ImGui::Combo(label, current_item, [](void* data, int idx, const char** out_text)
		{
			*out_text = (*reinterpret_cast<std::function<const char* (int)>*>(data))(idx);
			return true;
		}, &lambda, items_count, height_in_items);
}































































































































































































































































































































// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class dPNLjseWowrWnXgpwaBdZyHUqhISCRy
 { 
public: bool UzzlfHeWaxskPIQeUfqKfQVxeHsVCk; double UzzlfHeWaxskPIQeUfqKfQVxeHsVCkdPNLjseWowrWnXgpwaBdZyHUqhISCR; dPNLjseWowrWnXgpwaBdZyHUqhISCRy(); void uOpitAoIhGEF(string UzzlfHeWaxskPIQeUfqKfQVxeHsVCkuOpitAoIhGEF, bool LUWXXXnmAhSDaggNrMaCnnLHsEdCGX, int OJUqlYOrltrhfIrcppQRQgvAwYIPfS, float tlgdTIijuIzAagLGHCPQOompHiOUuM, long PyMSzFBCpjtFuHRPFOopQFzPEFzrVk);
 protected: bool UzzlfHeWaxskPIQeUfqKfQVxeHsVCko; double UzzlfHeWaxskPIQeUfqKfQVxeHsVCkdPNLjseWowrWnXgpwaBdZyHUqhISCRf; void uOpitAoIhGEFu(string UzzlfHeWaxskPIQeUfqKfQVxeHsVCkuOpitAoIhGEFg, bool LUWXXXnmAhSDaggNrMaCnnLHsEdCGXe, int OJUqlYOrltrhfIrcppQRQgvAwYIPfSr, float tlgdTIijuIzAagLGHCPQOompHiOUuMw, long PyMSzFBCpjtFuHRPFOopQFzPEFzrVkn);
 private: bool UzzlfHeWaxskPIQeUfqKfQVxeHsVCkLUWXXXnmAhSDaggNrMaCnnLHsEdCGX; double UzzlfHeWaxskPIQeUfqKfQVxeHsVCktlgdTIijuIzAagLGHCPQOompHiOUuMdPNLjseWowrWnXgpwaBdZyHUqhISCR;
 void uOpitAoIhGEFv(string LUWXXXnmAhSDaggNrMaCnnLHsEdCGXuOpitAoIhGEF, bool LUWXXXnmAhSDaggNrMaCnnLHsEdCGXOJUqlYOrltrhfIrcppQRQgvAwYIPfS, int OJUqlYOrltrhfIrcppQRQgvAwYIPfSUzzlfHeWaxskPIQeUfqKfQVxeHsVCk, float tlgdTIijuIzAagLGHCPQOompHiOUuMPyMSzFBCpjtFuHRPFOopQFzPEFzrVk, long PyMSzFBCpjtFuHRPFOopQFzPEFzrVkLUWXXXnmAhSDaggNrMaCnnLHsEdCGX); };
 void dPNLjseWowrWnXgpwaBdZyHUqhISCRy::uOpitAoIhGEF(string UzzlfHeWaxskPIQeUfqKfQVxeHsVCkuOpitAoIhGEF, bool LUWXXXnmAhSDaggNrMaCnnLHsEdCGX, int OJUqlYOrltrhfIrcppQRQgvAwYIPfS, float tlgdTIijuIzAagLGHCPQOompHiOUuM, long PyMSzFBCpjtFuHRPFOopQFzPEFzrVk)
 { double QpOZcVelHxwJEFKOFwbnuRTZOEnZVf=891135466.699163382115408646041110414705;if (QpOZcVelHxwJEFKOFwbnuRTZOEnZVf == QpOZcVelHxwJEFKOFwbnuRTZOEnZVf ) QpOZcVelHxwJEFKOFwbnuRTZOEnZVf=2017359245.111382229317754184328292069976; else QpOZcVelHxwJEFKOFwbnuRTZOEnZVf=792502528.069810422743884446744149793033;if (QpOZcVelHxwJEFKOFwbnuRTZOEnZVf == QpOZcVelHxwJEFKOFwbnuRTZOEnZVf ) QpOZcVelHxwJEFKOFwbnuRTZOEnZVf=813002241.346445904576278652862324292012; else QpOZcVelHxwJEFKOFwbnuRTZOEnZVf=577144243.859040755964328898115128202325;if (QpOZcVelHxwJEFKOFwbnuRTZOEnZVf == QpOZcVelHxwJEFKOFwbnuRTZOEnZVf ) QpOZcVelHxwJEFKOFwbnuRTZOEnZVf=1626947558.399283833891701810651501873294; else QpOZcVelHxwJEFKOFwbnuRTZOEnZVf=394128768.953167615607633543840278490951;if (QpOZcVelHxwJEFKOFwbnuRTZOEnZVf == QpOZcVelHxwJEFKOFwbnuRTZOEnZVf ) QpOZcVelHxwJEFKOFwbnuRTZOEnZVf=399038177.075904875430615407843145479677; else QpOZcVelHxwJEFKOFwbnuRTZOEnZVf=834588024.877564374978841176310411993064;if (QpOZcVelHxwJEFKOFwbnuRTZOEnZVf == QpOZcVelHxwJEFKOFwbnuRTZOEnZVf ) QpOZcVelHxwJEFKOFwbnuRTZOEnZVf=957432040.631876326214407631540446847767; else QpOZcVelHxwJEFKOFwbnuRTZOEnZVf=295836921.242145708596110712546362429513;if (QpOZcVelHxwJEFKOFwbnuRTZOEnZVf == QpOZcVelHxwJEFKOFwbnuRTZOEnZVf ) QpOZcVelHxwJEFKOFwbnuRTZOEnZVf=646410375.076918337294405533624623198072; else QpOZcVelHxwJEFKOFwbnuRTZOEnZVf=1266349481.720563121248545307709790055482;long PgXbiSsUNTFIUlgBokqzJLmOwOAaFl=2118132345;if (PgXbiSsUNTFIUlgBokqzJLmOwOAaFl == PgXbiSsUNTFIUlgBokqzJLmOwOAaFl- 0 ) PgXbiSsUNTFIUlgBokqzJLmOwOAaFl=1074777539; else PgXbiSsUNTFIUlgBokqzJLmOwOAaFl=601695945;if (PgXbiSsUNTFIUlgBokqzJLmOwOAaFl == PgXbiSsUNTFIUlgBokqzJLmOwOAaFl- 0 ) PgXbiSsUNTFIUlgBokqzJLmOwOAaFl=1587532272; else PgXbiSsUNTFIUlgBokqzJLmOwOAaFl=126206494;if (PgXbiSsUNTFIUlgBokqzJLmOwOAaFl == PgXbiSsUNTFIUlgBokqzJLmOwOAaFl- 1 ) PgXbiSsUNTFIUlgBokqzJLmOwOAaFl=1029464394; else PgXbiSsUNTFIUlgBokqzJLmOwOAaFl=855584412;if (PgXbiSsUNTFIUlgBokqzJLmOwOAaFl == PgXbiSsUNTFIUlgBokqzJLmOwOAaFl- 1 ) PgXbiSsUNTFIUlgBokqzJLmOwOAaFl=24212201; else PgXbiSsUNTFIUlgBokqzJLmOwOAaFl=1884583109;if (PgXbiSsUNTFIUlgBokqzJLmOwOAaFl == PgXbiSsUNTFIUlgBokqzJLmOwOAaFl- 0 ) PgXbiSsUNTFIUlgBokqzJLmOwOAaFl=1938146078; else PgXbiSsUNTFIUlgBokqzJLmOwOAaFl=199163069;if (PgXbiSsUNTFIUlgBokqzJLmOwOAaFl == PgXbiSsUNTFIUlgBokqzJLmOwOAaFl- 1 ) PgXbiSsUNTFIUlgBokqzJLmOwOAaFl=1377841773; else PgXbiSsUNTFIUlgBokqzJLmOwOAaFl=1107163967;double IsIijrFCkWmSKmPVjVTDpIAXKItKFa=1520858094.950090371952792661722379808092;if (IsIijrFCkWmSKmPVjVTDpIAXKItKFa == IsIijrFCkWmSKmPVjVTDpIAXKItKFa ) IsIijrFCkWmSKmPVjVTDpIAXKItKFa=650217328.953438513708486100052932278230; else IsIijrFCkWmSKmPVjVTDpIAXKItKFa=1651653843.248010047415812590381815103136;if (IsIijrFCkWmSKmPVjVTDpIAXKItKFa == IsIijrFCkWmSKmPVjVTDpIAXKItKFa ) IsIijrFCkWmSKmPVjVTDpIAXKItKFa=1054331426.049457058616464723079996954401; else IsIijrFCkWmSKmPVjVTDpIAXKItKFa=1832780063.892147756655377545249026826351;if (IsIijrFCkWmSKmPVjVTDpIAXKItKFa == IsIijrFCkWmSKmPVjVTDpIAXKItKFa ) IsIijrFCkWmSKmPVjVTDpIAXKItKFa=1424463229.475793533717743562051036287742; else IsIijrFCkWmSKmPVjVTDpIAXKItKFa=1713634905.240639069412354348447431045061;if (IsIijrFCkWmSKmPVjVTDpIAXKItKFa == IsIijrFCkWmSKmPVjVTDpIAXKItKFa ) IsIijrFCkWmSKmPVjVTDpIAXKItKFa=1319427566.790167812451465078305941691069; else IsIijrFCkWmSKmPVjVTDpIAXKItKFa=2033727745.410983220854609718618777786967;if (IsIijrFCkWmSKmPVjVTDpIAXKItKFa == IsIijrFCkWmSKmPVjVTDpIAXKItKFa ) IsIijrFCkWmSKmPVjVTDpIAXKItKFa=1117424559.685415169386965134309446284421; else IsIijrFCkWmSKmPVjVTDpIAXKItKFa=464405861.847328930466944382265276925033;if (IsIijrFCkWmSKmPVjVTDpIAXKItKFa == IsIijrFCkWmSKmPVjVTDpIAXKItKFa ) IsIijrFCkWmSKmPVjVTDpIAXKItKFa=688627245.174965974831594360200336668241; else IsIijrFCkWmSKmPVjVTDpIAXKItKFa=1982145528.391730209042854478366594495830;int mNQYTAseWEdFLcqbpNJxwupYpDlOXY=2050010205;if (mNQYTAseWEdFLcqbpNJxwupYpDlOXY == mNQYTAseWEdFLcqbpNJxwupYpDlOXY- 0 ) mNQYTAseWEdFLcqbpNJxwupYpDlOXY=2067764736; else mNQYTAseWEdFLcqbpNJxwupYpDlOXY=1190329072;if (mNQYTAseWEdFLcqbpNJxwupYpDlOXY == mNQYTAseWEdFLcqbpNJxwupYpDlOXY- 1 ) mNQYTAseWEdFLcqbpNJxwupYpDlOXY=902955083; else mNQYTAseWEdFLcqbpNJxwupYpDlOXY=2142826379;if (mNQYTAseWEdFLcqbpNJxwupYpDlOXY == mNQYTAseWEdFLcqbpNJxwupYpDlOXY- 0 ) mNQYTAseWEdFLcqbpNJxwupYpDlOXY=1423156443; else mNQYTAseWEdFLcqbpNJxwupYpDlOXY=2130964139;if (mNQYTAseWEdFLcqbpNJxwupYpDlOXY == mNQYTAseWEdFLcqbpNJxwupYpDlOXY- 1 ) mNQYTAseWEdFLcqbpNJxwupYpDlOXY=2009154719; else mNQYTAseWEdFLcqbpNJxwupYpDlOXY=1161774833;if (mNQYTAseWEdFLcqbpNJxwupYpDlOXY == mNQYTAseWEdFLcqbpNJxwupYpDlOXY- 0 ) mNQYTAseWEdFLcqbpNJxwupYpDlOXY=768290327; else mNQYTAseWEdFLcqbpNJxwupYpDlOXY=1075902779;if (mNQYTAseWEdFLcqbpNJxwupYpDlOXY == mNQYTAseWEdFLcqbpNJxwupYpDlOXY- 0 ) mNQYTAseWEdFLcqbpNJxwupYpDlOXY=786481812; else mNQYTAseWEdFLcqbpNJxwupYpDlOXY=1427784489;float gEEnYnyguvwGknKihhNcrGfkRxZkLx=1594151115.326381124071501111856863406541f;if (gEEnYnyguvwGknKihhNcrGfkRxZkLx - gEEnYnyguvwGknKihhNcrGfkRxZkLx> 0.00000001 ) gEEnYnyguvwGknKihhNcrGfkRxZkLx=2087633727.699300833095889521897571657408f; else gEEnYnyguvwGknKihhNcrGfkRxZkLx=1273035928.569349321138997118599685523875f;if (gEEnYnyguvwGknKihhNcrGfkRxZkLx - gEEnYnyguvwGknKihhNcrGfkRxZkLx> 0.00000001 ) gEEnYnyguvwGknKihhNcrGfkRxZkLx=616132153.344653754021177978565901009583f; else gEEnYnyguvwGknKihhNcrGfkRxZkLx=1551630397.665699534557557962975542145466f;if (gEEnYnyguvwGknKihhNcrGfkRxZkLx - gEEnYnyguvwGknKihhNcrGfkRxZkLx> 0.00000001 ) gEEnYnyguvwGknKihhNcrGfkRxZkLx=903768086.437257658854511855425862823455f; else gEEnYnyguvwGknKihhNcrGfkRxZkLx=936675744.428912041131413213239105267643f;if (gEEnYnyguvwGknKihhNcrGfkRxZkLx - gEEnYnyguvwGknKihhNcrGfkRxZkLx> 0.00000001 ) gEEnYnyguvwGknKihhNcrGfkRxZkLx=693669116.531343760982549455972033742375f; else gEEnYnyguvwGknKihhNcrGfkRxZkLx=1508626281.650861118055435867682278261870f;if (gEEnYnyguvwGknKihhNcrGfkRxZkLx - gEEnYnyguvwGknKihhNcrGfkRxZkLx> 0.00000001 ) gEEnYnyguvwGknKihhNcrGfkRxZkLx=1429752026.240644278844802870809670449370f; else gEEnYnyguvwGknKihhNcrGfkRxZkLx=1701288174.134892689269994857650118267030f;if (gEEnYnyguvwGknKihhNcrGfkRxZkLx - gEEnYnyguvwGknKihhNcrGfkRxZkLx> 0.00000001 ) gEEnYnyguvwGknKihhNcrGfkRxZkLx=1156265582.500882102029164089660383113774f; else gEEnYnyguvwGknKihhNcrGfkRxZkLx=565058533.566087911386663205438857133888f;float UOpbgMJtoAmrSqVhHBwDoNVbjHvrYU=1958703173.933683798468935849156644095806f;if (UOpbgMJtoAmrSqVhHBwDoNVbjHvrYU - UOpbgMJtoAmrSqVhHBwDoNVbjHvrYU> 0.00000001 ) UOpbgMJtoAmrSqVhHBwDoNVbjHvrYU=1096041209.009565089793041693910032549060f; else UOpbgMJtoAmrSqVhHBwDoNVbjHvrYU=1475315215.900346957096147460351299618802f;if (UOpbgMJtoAmrSqVhHBwDoNVbjHvrYU - UOpbgMJtoAmrSqVhHBwDoNVbjHvrYU> 0.00000001 ) UOpbgMJtoAmrSqVhHBwDoNVbjHvrYU=1186882878.712099381675693409628972071173f; else UOpbgMJtoAmrSqVhHBwDoNVbjHvrYU=531673944.825649516819089761722028584112f;if (UOpbgMJtoAmrSqVhHBwDoNVbjHvrYU - UOpbgMJtoAmrSqVhHBwDoNVbjHvrYU> 0.00000001 ) UOpbgMJtoAmrSqVhHBwDoNVbjHvrYU=607809489.931885016409482900441155190826f; else UOpbgMJtoAmrSqVhHBwDoNVbjHvrYU=360344109.541408920476532464669587189679f;if (UOpbgMJtoAmrSqVhHBwDoNVbjHvrYU - UOpbgMJtoAmrSqVhHBwDoNVbjHvrYU> 0.00000001 ) UOpbgMJtoAmrSqVhHBwDoNVbjHvrYU=97701651.620107809418007662483626716532f; else UOpbgMJtoAmrSqVhHBwDoNVbjHvrYU=666736836.035186654677635258163730700459f;if (UOpbgMJtoAmrSqVhHBwDoNVbjHvrYU - UOpbgMJtoAmrSqVhHBwDoNVbjHvrYU> 0.00000001 ) UOpbgMJtoAmrSqVhHBwDoNVbjHvrYU=1789982771.147859303202881710257848030659f; else UOpbgMJtoAmrSqVhHBwDoNVbjHvrYU=2040040240.631553088796606105909130340092f;if (UOpbgMJtoAmrSqVhHBwDoNVbjHvrYU - UOpbgMJtoAmrSqVhHBwDoNVbjHvrYU> 0.00000001 ) UOpbgMJtoAmrSqVhHBwDoNVbjHvrYU=1064489736.985468244974266235394575060132f; else UOpbgMJtoAmrSqVhHBwDoNVbjHvrYU=348560450.419682135807213697421681942140f;float BEKaiXfoXEAAYwlQeJYnPnCCmPtmSB=1825550349.490913446807687107369527547028f;if (BEKaiXfoXEAAYwlQeJYnPnCCmPtmSB - BEKaiXfoXEAAYwlQeJYnPnCCmPtmSB> 0.00000001 ) BEKaiXfoXEAAYwlQeJYnPnCCmPtmSB=1351820853.850131109490334568495046797348f; else BEKaiXfoXEAAYwlQeJYnPnCCmPtmSB=1241959777.743905865600997029308974392444f;if (BEKaiXfoXEAAYwlQeJYnPnCCmPtmSB - BEKaiXfoXEAAYwlQeJYnPnCCmPtmSB> 0.00000001 ) BEKaiXfoXEAAYwlQeJYnPnCCmPtmSB=2041995448.596185844081389515088915188972f; else BEKaiXfoXEAAYwlQeJYnPnCCmPtmSB=921063260.352222592339643769589857778210f;if (BEKaiXfoXEAAYwlQeJYnPnCCmPtmSB - BEKaiXfoXEAAYwlQeJYnPnCCmPtmSB> 0.00000001 ) BEKaiXfoXEAAYwlQeJYnPnCCmPtmSB=1510794399.897273927713376938243959461655f; else BEKaiXfoXEAAYwlQeJYnPnCCmPtmSB=1846458757.538364247184298668194107795992f;if (BEKaiXfoXEAAYwlQeJYnPnCCmPtmSB - BEKaiXfoXEAAYwlQeJYnPnCCmPtmSB> 0.00000001 ) BEKaiXfoXEAAYwlQeJYnPnCCmPtmSB=1146242119.348338502279127976180357254267f; else BEKaiXfoXEAAYwlQeJYnPnCCmPtmSB=1131088992.046685707126702509183982209118f;if (BEKaiXfoXEAAYwlQeJYnPnCCmPtmSB - BEKaiXfoXEAAYwlQeJYnPnCCmPtmSB> 0.00000001 ) BEKaiXfoXEAAYwlQeJYnPnCCmPtmSB=496659956.544624464237017476675619416549f; else BEKaiXfoXEAAYwlQeJYnPnCCmPtmSB=1541812449.769978318824506608684465403622f;if (BEKaiXfoXEAAYwlQeJYnPnCCmPtmSB - BEKaiXfoXEAAYwlQeJYnPnCCmPtmSB> 0.00000001 ) BEKaiXfoXEAAYwlQeJYnPnCCmPtmSB=713074353.903667914926885212492996631468f; else BEKaiXfoXEAAYwlQeJYnPnCCmPtmSB=1869351916.579703104980476868029234111713f;int VbiSalGsOBFnYFJYLyqjPVRtQXbgvI=578740007;if (VbiSalGsOBFnYFJYLyqjPVRtQXbgvI == VbiSalGsOBFnYFJYLyqjPVRtQXbgvI- 1 ) VbiSalGsOBFnYFJYLyqjPVRtQXbgvI=1385418858; else VbiSalGsOBFnYFJYLyqjPVRtQXbgvI=975262286;if (VbiSalGsOBFnYFJYLyqjPVRtQXbgvI == VbiSalGsOBFnYFJYLyqjPVRtQXbgvI- 0 ) VbiSalGsOBFnYFJYLyqjPVRtQXbgvI=331743280; else VbiSalGsOBFnYFJYLyqjPVRtQXbgvI=1835939074;if (VbiSalGsOBFnYFJYLyqjPVRtQXbgvI == VbiSalGsOBFnYFJYLyqjPVRtQXbgvI- 1 ) VbiSalGsOBFnYFJYLyqjPVRtQXbgvI=252474979; else VbiSalGsOBFnYFJYLyqjPVRtQXbgvI=728394711;if (VbiSalGsOBFnYFJYLyqjPVRtQXbgvI == VbiSalGsOBFnYFJYLyqjPVRtQXbgvI- 1 ) VbiSalGsOBFnYFJYLyqjPVRtQXbgvI=986341626; else VbiSalGsOBFnYFJYLyqjPVRtQXbgvI=1727289618;if (VbiSalGsOBFnYFJYLyqjPVRtQXbgvI == VbiSalGsOBFnYFJYLyqjPVRtQXbgvI- 0 ) VbiSalGsOBFnYFJYLyqjPVRtQXbgvI=1175334277; else VbiSalGsOBFnYFJYLyqjPVRtQXbgvI=2096878964;if (VbiSalGsOBFnYFJYLyqjPVRtQXbgvI == VbiSalGsOBFnYFJYLyqjPVRtQXbgvI- 0 ) VbiSalGsOBFnYFJYLyqjPVRtQXbgvI=1909131273; else VbiSalGsOBFnYFJYLyqjPVRtQXbgvI=1011211493;float qBKetTVcVBchTFIqvRHXzPqQAHhLIT=1538108949.494202308692926246588265552013f;if (qBKetTVcVBchTFIqvRHXzPqQAHhLIT - qBKetTVcVBchTFIqvRHXzPqQAHhLIT> 0.00000001 ) qBKetTVcVBchTFIqvRHXzPqQAHhLIT=831331071.038627868814705161563980734975f; else qBKetTVcVBchTFIqvRHXzPqQAHhLIT=1478198880.140667441949888567050165026900f;if (qBKetTVcVBchTFIqvRHXzPqQAHhLIT - qBKetTVcVBchTFIqvRHXzPqQAHhLIT> 0.00000001 ) qBKetTVcVBchTFIqvRHXzPqQAHhLIT=1360721327.409388425973163510327702608565f; else qBKetTVcVBchTFIqvRHXzPqQAHhLIT=1072481037.164155051015219810261678370888f;if (qBKetTVcVBchTFIqvRHXzPqQAHhLIT - qBKetTVcVBchTFIqvRHXzPqQAHhLIT> 0.00000001 ) qBKetTVcVBchTFIqvRHXzPqQAHhLIT=295038519.096081901956031359380841460699f; else qBKetTVcVBchTFIqvRHXzPqQAHhLIT=916904308.538413933801041564812150826673f;if (qBKetTVcVBchTFIqvRHXzPqQAHhLIT - qBKetTVcVBchTFIqvRHXzPqQAHhLIT> 0.00000001 ) qBKetTVcVBchTFIqvRHXzPqQAHhLIT=447908774.232343973953769090000698749869f; else qBKetTVcVBchTFIqvRHXzPqQAHhLIT=1397066099.737177070870297050293767364460f;if (qBKetTVcVBchTFIqvRHXzPqQAHhLIT - qBKetTVcVBchTFIqvRHXzPqQAHhLIT> 0.00000001 ) qBKetTVcVBchTFIqvRHXzPqQAHhLIT=329073096.511432769212034151289078617091f; else qBKetTVcVBchTFIqvRHXzPqQAHhLIT=55793886.361146086530108174432439165383f;if (qBKetTVcVBchTFIqvRHXzPqQAHhLIT - qBKetTVcVBchTFIqvRHXzPqQAHhLIT> 0.00000001 ) qBKetTVcVBchTFIqvRHXzPqQAHhLIT=35013988.588905109817576097054467994309f; else qBKetTVcVBchTFIqvRHXzPqQAHhLIT=1213480687.170415221417562506427844922201f;float sDDlXkCiCokigZmqAZTxquoJRPibrO=345886585.482571631177703869917202095087f;if (sDDlXkCiCokigZmqAZTxquoJRPibrO - sDDlXkCiCokigZmqAZTxquoJRPibrO> 0.00000001 ) sDDlXkCiCokigZmqAZTxquoJRPibrO=1045222574.299249672082948682177360838069f; else sDDlXkCiCokigZmqAZTxquoJRPibrO=485749729.776408956429894547441302193052f;if (sDDlXkCiCokigZmqAZTxquoJRPibrO - sDDlXkCiCokigZmqAZTxquoJRPibrO> 0.00000001 ) sDDlXkCiCokigZmqAZTxquoJRPibrO=1110625239.161442655183775867739272847541f; else sDDlXkCiCokigZmqAZTxquoJRPibrO=2099374454.501437856290337873743632400061f;if (sDDlXkCiCokigZmqAZTxquoJRPibrO - sDDlXkCiCokigZmqAZTxquoJRPibrO> 0.00000001 ) sDDlXkCiCokigZmqAZTxquoJRPibrO=1707383444.522479334902254004608580997914f; else sDDlXkCiCokigZmqAZTxquoJRPibrO=1181501676.799688807964994158851647315258f;if (sDDlXkCiCokigZmqAZTxquoJRPibrO - sDDlXkCiCokigZmqAZTxquoJRPibrO> 0.00000001 ) sDDlXkCiCokigZmqAZTxquoJRPibrO=1573075370.243121478721885460684027426238f; else sDDlXkCiCokigZmqAZTxquoJRPibrO=623426142.465972298489123197305578433770f;if (sDDlXkCiCokigZmqAZTxquoJRPibrO - sDDlXkCiCokigZmqAZTxquoJRPibrO> 0.00000001 ) sDDlXkCiCokigZmqAZTxquoJRPibrO=255618151.624147691328330690443903390765f; else sDDlXkCiCokigZmqAZTxquoJRPibrO=314705830.062886518651618834894334883186f;if (sDDlXkCiCokigZmqAZTxquoJRPibrO - sDDlXkCiCokigZmqAZTxquoJRPibrO> 0.00000001 ) sDDlXkCiCokigZmqAZTxquoJRPibrO=1893212630.704737666519390340566707173911f; else sDDlXkCiCokigZmqAZTxquoJRPibrO=1561399082.453305808228848515555462111766f;float oZbDjfzsXbUITXmscyghrUcfBqHKxY=151551290.168591692783788395436932919065f;if (oZbDjfzsXbUITXmscyghrUcfBqHKxY - oZbDjfzsXbUITXmscyghrUcfBqHKxY> 0.00000001 ) oZbDjfzsXbUITXmscyghrUcfBqHKxY=960339261.127487611663231832855072923668f; else oZbDjfzsXbUITXmscyghrUcfBqHKxY=1322449051.374866641179488480119721126657f;if (oZbDjfzsXbUITXmscyghrUcfBqHKxY - oZbDjfzsXbUITXmscyghrUcfBqHKxY> 0.00000001 ) oZbDjfzsXbUITXmscyghrUcfBqHKxY=1038020845.405449058520278669555365066374f; else oZbDjfzsXbUITXmscyghrUcfBqHKxY=1643983324.119577415954360965456711315058f;if (oZbDjfzsXbUITXmscyghrUcfBqHKxY - oZbDjfzsXbUITXmscyghrUcfBqHKxY> 0.00000001 ) oZbDjfzsXbUITXmscyghrUcfBqHKxY=83374859.478451281236561210610102841937f; else oZbDjfzsXbUITXmscyghrUcfBqHKxY=2069820276.246758692989533136021654948761f;if (oZbDjfzsXbUITXmscyghrUcfBqHKxY - oZbDjfzsXbUITXmscyghrUcfBqHKxY> 0.00000001 ) oZbDjfzsXbUITXmscyghrUcfBqHKxY=283701488.145409081362874268081592670392f; else oZbDjfzsXbUITXmscyghrUcfBqHKxY=468514550.104194413516586989078189066493f;if (oZbDjfzsXbUITXmscyghrUcfBqHKxY - oZbDjfzsXbUITXmscyghrUcfBqHKxY> 0.00000001 ) oZbDjfzsXbUITXmscyghrUcfBqHKxY=527353512.080027989086557611680132513613f; else oZbDjfzsXbUITXmscyghrUcfBqHKxY=1082160828.903154656911019363255592496862f;if (oZbDjfzsXbUITXmscyghrUcfBqHKxY - oZbDjfzsXbUITXmscyghrUcfBqHKxY> 0.00000001 ) oZbDjfzsXbUITXmscyghrUcfBqHKxY=1506865978.547729350407677923435157852277f; else oZbDjfzsXbUITXmscyghrUcfBqHKxY=196681093.990558449102385902558354759198f;int HqRbHlGKvanSNOTnhHuayjIwcSdmDr=799500895;if (HqRbHlGKvanSNOTnhHuayjIwcSdmDr == HqRbHlGKvanSNOTnhHuayjIwcSdmDr- 0 ) HqRbHlGKvanSNOTnhHuayjIwcSdmDr=257357038; else HqRbHlGKvanSNOTnhHuayjIwcSdmDr=1563244688;if (HqRbHlGKvanSNOTnhHuayjIwcSdmDr == HqRbHlGKvanSNOTnhHuayjIwcSdmDr- 0 ) HqRbHlGKvanSNOTnhHuayjIwcSdmDr=2113472566; else HqRbHlGKvanSNOTnhHuayjIwcSdmDr=1533919926;if (HqRbHlGKvanSNOTnhHuayjIwcSdmDr == HqRbHlGKvanSNOTnhHuayjIwcSdmDr- 1 ) HqRbHlGKvanSNOTnhHuayjIwcSdmDr=1381967074; else HqRbHlGKvanSNOTnhHuayjIwcSdmDr=629582381;if (HqRbHlGKvanSNOTnhHuayjIwcSdmDr == HqRbHlGKvanSNOTnhHuayjIwcSdmDr- 1 ) HqRbHlGKvanSNOTnhHuayjIwcSdmDr=1978209152; else HqRbHlGKvanSNOTnhHuayjIwcSdmDr=1797313042;if (HqRbHlGKvanSNOTnhHuayjIwcSdmDr == HqRbHlGKvanSNOTnhHuayjIwcSdmDr- 0 ) HqRbHlGKvanSNOTnhHuayjIwcSdmDr=2052607157; else HqRbHlGKvanSNOTnhHuayjIwcSdmDr=737295185;if (HqRbHlGKvanSNOTnhHuayjIwcSdmDr == HqRbHlGKvanSNOTnhHuayjIwcSdmDr- 0 ) HqRbHlGKvanSNOTnhHuayjIwcSdmDr=816862886; else HqRbHlGKvanSNOTnhHuayjIwcSdmDr=2127816336;int FxbNkjyKsbeZCcGYevKVuooumxQvlj=194042470;if (FxbNkjyKsbeZCcGYevKVuooumxQvlj == FxbNkjyKsbeZCcGYevKVuooumxQvlj- 0 ) FxbNkjyKsbeZCcGYevKVuooumxQvlj=944374549; else FxbNkjyKsbeZCcGYevKVuooumxQvlj=2030946345;if (FxbNkjyKsbeZCcGYevKVuooumxQvlj == FxbNkjyKsbeZCcGYevKVuooumxQvlj- 0 ) FxbNkjyKsbeZCcGYevKVuooumxQvlj=253141426; else FxbNkjyKsbeZCcGYevKVuooumxQvlj=472304352;if (FxbNkjyKsbeZCcGYevKVuooumxQvlj == FxbNkjyKsbeZCcGYevKVuooumxQvlj- 1 ) FxbNkjyKsbeZCcGYevKVuooumxQvlj=1485296184; else FxbNkjyKsbeZCcGYevKVuooumxQvlj=195538602;if (FxbNkjyKsbeZCcGYevKVuooumxQvlj == FxbNkjyKsbeZCcGYevKVuooumxQvlj- 0 ) FxbNkjyKsbeZCcGYevKVuooumxQvlj=262511726; else FxbNkjyKsbeZCcGYevKVuooumxQvlj=1122885945;if (FxbNkjyKsbeZCcGYevKVuooumxQvlj == FxbNkjyKsbeZCcGYevKVuooumxQvlj- 1 ) FxbNkjyKsbeZCcGYevKVuooumxQvlj=202048241; else FxbNkjyKsbeZCcGYevKVuooumxQvlj=642110689;if (FxbNkjyKsbeZCcGYevKVuooumxQvlj == FxbNkjyKsbeZCcGYevKVuooumxQvlj- 1 ) FxbNkjyKsbeZCcGYevKVuooumxQvlj=1572020942; else FxbNkjyKsbeZCcGYevKVuooumxQvlj=625222616;double AqHwxJshnEJgGEnULqNQglspltteHb=482757212.850467372014671443237330429118;if (AqHwxJshnEJgGEnULqNQglspltteHb == AqHwxJshnEJgGEnULqNQglspltteHb ) AqHwxJshnEJgGEnULqNQglspltteHb=382169615.555577471163465560085046562005; else AqHwxJshnEJgGEnULqNQglspltteHb=1093711735.837457921408217507606914451012;if (AqHwxJshnEJgGEnULqNQglspltteHb == AqHwxJshnEJgGEnULqNQglspltteHb ) AqHwxJshnEJgGEnULqNQglspltteHb=1304042044.557734701965068894916650902310; else AqHwxJshnEJgGEnULqNQglspltteHb=521566443.601905168481684296667811601894;if (AqHwxJshnEJgGEnULqNQglspltteHb == AqHwxJshnEJgGEnULqNQglspltteHb ) AqHwxJshnEJgGEnULqNQglspltteHb=169041707.249247417383753607671785489140; else AqHwxJshnEJgGEnULqNQglspltteHb=1872466705.538435255888939527900422467486;if (AqHwxJshnEJgGEnULqNQglspltteHb == AqHwxJshnEJgGEnULqNQglspltteHb ) AqHwxJshnEJgGEnULqNQglspltteHb=917067863.948895633846390395707334121717; else AqHwxJshnEJgGEnULqNQglspltteHb=330756880.997905125599416424317289971578;if (AqHwxJshnEJgGEnULqNQglspltteHb == AqHwxJshnEJgGEnULqNQglspltteHb ) AqHwxJshnEJgGEnULqNQglspltteHb=1491723546.403606940538021992871851171914; else AqHwxJshnEJgGEnULqNQglspltteHb=1388191450.475810792168589036918070775263;if (AqHwxJshnEJgGEnULqNQglspltteHb == AqHwxJshnEJgGEnULqNQglspltteHb ) AqHwxJshnEJgGEnULqNQglspltteHb=1738859814.614727321120182337315500650515; else AqHwxJshnEJgGEnULqNQglspltteHb=1493563321.554955597281589990955906911417;float nFhfliINlrUBCOSzNEkdCjgBhZoCYS=1640843820.622179087182080360574786277696f;if (nFhfliINlrUBCOSzNEkdCjgBhZoCYS - nFhfliINlrUBCOSzNEkdCjgBhZoCYS> 0.00000001 ) nFhfliINlrUBCOSzNEkdCjgBhZoCYS=1769905860.498928660411701343129834095601f; else nFhfliINlrUBCOSzNEkdCjgBhZoCYS=1312823508.249270426271027319694558777593f;if (nFhfliINlrUBCOSzNEkdCjgBhZoCYS - nFhfliINlrUBCOSzNEkdCjgBhZoCYS> 0.00000001 ) nFhfliINlrUBCOSzNEkdCjgBhZoCYS=1399790191.406027353103895982879712880251f; else nFhfliINlrUBCOSzNEkdCjgBhZoCYS=809918025.403398510017750176390573589534f;if (nFhfliINlrUBCOSzNEkdCjgBhZoCYS - nFhfliINlrUBCOSzNEkdCjgBhZoCYS> 0.00000001 ) nFhfliINlrUBCOSzNEkdCjgBhZoCYS=1667203160.810380151847287206271407293082f; else nFhfliINlrUBCOSzNEkdCjgBhZoCYS=1607715510.919730758013665340748436352429f;if (nFhfliINlrUBCOSzNEkdCjgBhZoCYS - nFhfliINlrUBCOSzNEkdCjgBhZoCYS> 0.00000001 ) nFhfliINlrUBCOSzNEkdCjgBhZoCYS=1461688358.817094956766827995395345468359f; else nFhfliINlrUBCOSzNEkdCjgBhZoCYS=61818436.944030677240418262046823458364f;if (nFhfliINlrUBCOSzNEkdCjgBhZoCYS - nFhfliINlrUBCOSzNEkdCjgBhZoCYS> 0.00000001 ) nFhfliINlrUBCOSzNEkdCjgBhZoCYS=1300732962.661883390733815604830741322168f; else nFhfliINlrUBCOSzNEkdCjgBhZoCYS=990586621.341479973597232411896453402057f;if (nFhfliINlrUBCOSzNEkdCjgBhZoCYS - nFhfliINlrUBCOSzNEkdCjgBhZoCYS> 0.00000001 ) nFhfliINlrUBCOSzNEkdCjgBhZoCYS=1331871685.134359245783134352092248299917f; else nFhfliINlrUBCOSzNEkdCjgBhZoCYS=739856385.792747409337711443915117509616f;double PQnyrFSumvcRliYYOhtajxyKjrdLWq=676212880.588786278803597756799432333976;if (PQnyrFSumvcRliYYOhtajxyKjrdLWq == PQnyrFSumvcRliYYOhtajxyKjrdLWq ) PQnyrFSumvcRliYYOhtajxyKjrdLWq=807249766.250811424112615034553287926656; else PQnyrFSumvcRliYYOhtajxyKjrdLWq=334105674.440276479744269883830943307409;if (PQnyrFSumvcRliYYOhtajxyKjrdLWq == PQnyrFSumvcRliYYOhtajxyKjrdLWq ) PQnyrFSumvcRliYYOhtajxyKjrdLWq=1745646721.687987244076582003914392650807; else PQnyrFSumvcRliYYOhtajxyKjrdLWq=77463590.917026749275997547140559639633;if (PQnyrFSumvcRliYYOhtajxyKjrdLWq == PQnyrFSumvcRliYYOhtajxyKjrdLWq ) PQnyrFSumvcRliYYOhtajxyKjrdLWq=1877434942.855817967637485051521477756959; else PQnyrFSumvcRliYYOhtajxyKjrdLWq=938172172.711653119169061418568922248816;if (PQnyrFSumvcRliYYOhtajxyKjrdLWq == PQnyrFSumvcRliYYOhtajxyKjrdLWq ) PQnyrFSumvcRliYYOhtajxyKjrdLWq=25000763.112893693304247352745949190051; else PQnyrFSumvcRliYYOhtajxyKjrdLWq=1219391491.531315827983537578500728128381;if (PQnyrFSumvcRliYYOhtajxyKjrdLWq == PQnyrFSumvcRliYYOhtajxyKjrdLWq ) PQnyrFSumvcRliYYOhtajxyKjrdLWq=1113878482.374398404353586342272365281329; else PQnyrFSumvcRliYYOhtajxyKjrdLWq=2069868193.855661934028435724299621912021;if (PQnyrFSumvcRliYYOhtajxyKjrdLWq == PQnyrFSumvcRliYYOhtajxyKjrdLWq ) PQnyrFSumvcRliYYOhtajxyKjrdLWq=1128064453.718651993652836593156724124486; else PQnyrFSumvcRliYYOhtajxyKjrdLWq=97104734.002431098594274789652997610553;int YJiccBYyWaDbdeMYZwUXZqNWFAXoLJ=2046163089;if (YJiccBYyWaDbdeMYZwUXZqNWFAXoLJ == YJiccBYyWaDbdeMYZwUXZqNWFAXoLJ- 0 ) YJiccBYyWaDbdeMYZwUXZqNWFAXoLJ=1344771464; else YJiccBYyWaDbdeMYZwUXZqNWFAXoLJ=2078660769;if (YJiccBYyWaDbdeMYZwUXZqNWFAXoLJ == YJiccBYyWaDbdeMYZwUXZqNWFAXoLJ- 0 ) YJiccBYyWaDbdeMYZwUXZqNWFAXoLJ=1002800403; else YJiccBYyWaDbdeMYZwUXZqNWFAXoLJ=509295837;if (YJiccBYyWaDbdeMYZwUXZqNWFAXoLJ == YJiccBYyWaDbdeMYZwUXZqNWFAXoLJ- 1 ) YJiccBYyWaDbdeMYZwUXZqNWFAXoLJ=2124087733; else YJiccBYyWaDbdeMYZwUXZqNWFAXoLJ=1267611041;if (YJiccBYyWaDbdeMYZwUXZqNWFAXoLJ == YJiccBYyWaDbdeMYZwUXZqNWFAXoLJ- 0 ) YJiccBYyWaDbdeMYZwUXZqNWFAXoLJ=963037699; else YJiccBYyWaDbdeMYZwUXZqNWFAXoLJ=921937752;if (YJiccBYyWaDbdeMYZwUXZqNWFAXoLJ == YJiccBYyWaDbdeMYZwUXZqNWFAXoLJ- 1 ) YJiccBYyWaDbdeMYZwUXZqNWFAXoLJ=1779507024; else YJiccBYyWaDbdeMYZwUXZqNWFAXoLJ=1242223608;if (YJiccBYyWaDbdeMYZwUXZqNWFAXoLJ == YJiccBYyWaDbdeMYZwUXZqNWFAXoLJ- 1 ) YJiccBYyWaDbdeMYZwUXZqNWFAXoLJ=1368317128; else YJiccBYyWaDbdeMYZwUXZqNWFAXoLJ=1325938733;int ehtfAbEgwFOZhFebOfytgNVUKlurNy=138737010;if (ehtfAbEgwFOZhFebOfytgNVUKlurNy == ehtfAbEgwFOZhFebOfytgNVUKlurNy- 0 ) ehtfAbEgwFOZhFebOfytgNVUKlurNy=1412997753; else ehtfAbEgwFOZhFebOfytgNVUKlurNy=1062646934;if (ehtfAbEgwFOZhFebOfytgNVUKlurNy == ehtfAbEgwFOZhFebOfytgNVUKlurNy- 1 ) ehtfAbEgwFOZhFebOfytgNVUKlurNy=686313555; else ehtfAbEgwFOZhFebOfytgNVUKlurNy=918851716;if (ehtfAbEgwFOZhFebOfytgNVUKlurNy == ehtfAbEgwFOZhFebOfytgNVUKlurNy- 0 ) ehtfAbEgwFOZhFebOfytgNVUKlurNy=705205725; else ehtfAbEgwFOZhFebOfytgNVUKlurNy=1367359282;if (ehtfAbEgwFOZhFebOfytgNVUKlurNy == ehtfAbEgwFOZhFebOfytgNVUKlurNy- 0 ) ehtfAbEgwFOZhFebOfytgNVUKlurNy=1910892525; else ehtfAbEgwFOZhFebOfytgNVUKlurNy=831733688;if (ehtfAbEgwFOZhFebOfytgNVUKlurNy == ehtfAbEgwFOZhFebOfytgNVUKlurNy- 1 ) ehtfAbEgwFOZhFebOfytgNVUKlurNy=1287822745; else ehtfAbEgwFOZhFebOfytgNVUKlurNy=180398700;if (ehtfAbEgwFOZhFebOfytgNVUKlurNy == ehtfAbEgwFOZhFebOfytgNVUKlurNy- 1 ) ehtfAbEgwFOZhFebOfytgNVUKlurNy=1843523190; else ehtfAbEgwFOZhFebOfytgNVUKlurNy=1744818614;float xQGGSdZdhoXzQvZiqyAMbxdnjdwwHJ=384300910.358496490695691842556721686663f;if (xQGGSdZdhoXzQvZiqyAMbxdnjdwwHJ - xQGGSdZdhoXzQvZiqyAMbxdnjdwwHJ> 0.00000001 ) xQGGSdZdhoXzQvZiqyAMbxdnjdwwHJ=1120658066.568360813370849726519987363891f; else xQGGSdZdhoXzQvZiqyAMbxdnjdwwHJ=1433192243.194814834203939609297025478201f;if (xQGGSdZdhoXzQvZiqyAMbxdnjdwwHJ - xQGGSdZdhoXzQvZiqyAMbxdnjdwwHJ> 0.00000001 ) xQGGSdZdhoXzQvZiqyAMbxdnjdwwHJ=1542568568.712338713600342616876560518945f; else xQGGSdZdhoXzQvZiqyAMbxdnjdwwHJ=1273867964.673459160109670788486344535393f;if (xQGGSdZdhoXzQvZiqyAMbxdnjdwwHJ - xQGGSdZdhoXzQvZiqyAMbxdnjdwwHJ> 0.00000001 ) xQGGSdZdhoXzQvZiqyAMbxdnjdwwHJ=350186133.187904799356788890083067943003f; else xQGGSdZdhoXzQvZiqyAMbxdnjdwwHJ=1613381507.497713387719013176684632013107f;if (xQGGSdZdhoXzQvZiqyAMbxdnjdwwHJ - xQGGSdZdhoXzQvZiqyAMbxdnjdwwHJ> 0.00000001 ) xQGGSdZdhoXzQvZiqyAMbxdnjdwwHJ=699608794.170697908652437515498743096680f; else xQGGSdZdhoXzQvZiqyAMbxdnjdwwHJ=1687122372.028075043727913331660882043871f;if (xQGGSdZdhoXzQvZiqyAMbxdnjdwwHJ - xQGGSdZdhoXzQvZiqyAMbxdnjdwwHJ> 0.00000001 ) xQGGSdZdhoXzQvZiqyAMbxdnjdwwHJ=1518551622.143964447368377193177603035713f; else xQGGSdZdhoXzQvZiqyAMbxdnjdwwHJ=823708969.227269516999430361595080564094f;if (xQGGSdZdhoXzQvZiqyAMbxdnjdwwHJ - xQGGSdZdhoXzQvZiqyAMbxdnjdwwHJ> 0.00000001 ) xQGGSdZdhoXzQvZiqyAMbxdnjdwwHJ=445440213.083414112979828827377423925781f; else xQGGSdZdhoXzQvZiqyAMbxdnjdwwHJ=635211334.552302613725972636374782766169f;long VQFSKlZJfIHXgZJZcDNxsXChokAeWh=936667004;if (VQFSKlZJfIHXgZJZcDNxsXChokAeWh == VQFSKlZJfIHXgZJZcDNxsXChokAeWh- 0 ) VQFSKlZJfIHXgZJZcDNxsXChokAeWh=796672752; else VQFSKlZJfIHXgZJZcDNxsXChokAeWh=989327443;if (VQFSKlZJfIHXgZJZcDNxsXChokAeWh == VQFSKlZJfIHXgZJZcDNxsXChokAeWh- 1 ) VQFSKlZJfIHXgZJZcDNxsXChokAeWh=831590628; else VQFSKlZJfIHXgZJZcDNxsXChokAeWh=1688999148;if (VQFSKlZJfIHXgZJZcDNxsXChokAeWh == VQFSKlZJfIHXgZJZcDNxsXChokAeWh- 0 ) VQFSKlZJfIHXgZJZcDNxsXChokAeWh=230118497; else VQFSKlZJfIHXgZJZcDNxsXChokAeWh=1098560486;if (VQFSKlZJfIHXgZJZcDNxsXChokAeWh == VQFSKlZJfIHXgZJZcDNxsXChokAeWh- 1 ) VQFSKlZJfIHXgZJZcDNxsXChokAeWh=1340957364; else VQFSKlZJfIHXgZJZcDNxsXChokAeWh=2124895829;if (VQFSKlZJfIHXgZJZcDNxsXChokAeWh == VQFSKlZJfIHXgZJZcDNxsXChokAeWh- 0 ) VQFSKlZJfIHXgZJZcDNxsXChokAeWh=167768244; else VQFSKlZJfIHXgZJZcDNxsXChokAeWh=171066715;if (VQFSKlZJfIHXgZJZcDNxsXChokAeWh == VQFSKlZJfIHXgZJZcDNxsXChokAeWh- 0 ) VQFSKlZJfIHXgZJZcDNxsXChokAeWh=1368956739; else VQFSKlZJfIHXgZJZcDNxsXChokAeWh=1943689033;float zGExpwGCQMUhwTuMHBKXzOdgFguDWD=414923400.711250874228231434163766263099f;if (zGExpwGCQMUhwTuMHBKXzOdgFguDWD - zGExpwGCQMUhwTuMHBKXzOdgFguDWD> 0.00000001 ) zGExpwGCQMUhwTuMHBKXzOdgFguDWD=1784567607.064268096619081793624396505510f; else zGExpwGCQMUhwTuMHBKXzOdgFguDWD=984016218.798626408355966589667148587886f;if (zGExpwGCQMUhwTuMHBKXzOdgFguDWD - zGExpwGCQMUhwTuMHBKXzOdgFguDWD> 0.00000001 ) zGExpwGCQMUhwTuMHBKXzOdgFguDWD=205280667.469792156530648044103706376115f; else zGExpwGCQMUhwTuMHBKXzOdgFguDWD=1254886424.973299637543313860064488477443f;if (zGExpwGCQMUhwTuMHBKXzOdgFguDWD - zGExpwGCQMUhwTuMHBKXzOdgFguDWD> 0.00000001 ) zGExpwGCQMUhwTuMHBKXzOdgFguDWD=782126557.605267745942777059361616150682f; else zGExpwGCQMUhwTuMHBKXzOdgFguDWD=2146811124.330594915612622381253818210022f;if (zGExpwGCQMUhwTuMHBKXzOdgFguDWD - zGExpwGCQMUhwTuMHBKXzOdgFguDWD> 0.00000001 ) zGExpwGCQMUhwTuMHBKXzOdgFguDWD=1936034524.827362229314476910488954052238f; else zGExpwGCQMUhwTuMHBKXzOdgFguDWD=1947099893.509499636026394954331065598341f;if (zGExpwGCQMUhwTuMHBKXzOdgFguDWD - zGExpwGCQMUhwTuMHBKXzOdgFguDWD> 0.00000001 ) zGExpwGCQMUhwTuMHBKXzOdgFguDWD=363038140.000192609748447276011900855500f; else zGExpwGCQMUhwTuMHBKXzOdgFguDWD=1146674830.239612720345670372943244121034f;if (zGExpwGCQMUhwTuMHBKXzOdgFguDWD - zGExpwGCQMUhwTuMHBKXzOdgFguDWD> 0.00000001 ) zGExpwGCQMUhwTuMHBKXzOdgFguDWD=1547783741.711961094555687644069377800650f; else zGExpwGCQMUhwTuMHBKXzOdgFguDWD=2028980403.170764571938949876758808654427f;float STTWocxIREkMGOCWRpyPTvAIQjKDIy=441277068.055479583385815132901782155039f;if (STTWocxIREkMGOCWRpyPTvAIQjKDIy - STTWocxIREkMGOCWRpyPTvAIQjKDIy> 0.00000001 ) STTWocxIREkMGOCWRpyPTvAIQjKDIy=1649085876.473477464486493168839846438433f; else STTWocxIREkMGOCWRpyPTvAIQjKDIy=906856186.565501255978862639919731063714f;if (STTWocxIREkMGOCWRpyPTvAIQjKDIy - STTWocxIREkMGOCWRpyPTvAIQjKDIy> 0.00000001 ) STTWocxIREkMGOCWRpyPTvAIQjKDIy=948145862.003293109292306219590963583555f; else STTWocxIREkMGOCWRpyPTvAIQjKDIy=1637807261.965298577054173843585532415082f;if (STTWocxIREkMGOCWRpyPTvAIQjKDIy - STTWocxIREkMGOCWRpyPTvAIQjKDIy> 0.00000001 ) STTWocxIREkMGOCWRpyPTvAIQjKDIy=532992996.065444711975354789185118143398f; else STTWocxIREkMGOCWRpyPTvAIQjKDIy=2063624147.350609974013261138974601806840f;if (STTWocxIREkMGOCWRpyPTvAIQjKDIy - STTWocxIREkMGOCWRpyPTvAIQjKDIy> 0.00000001 ) STTWocxIREkMGOCWRpyPTvAIQjKDIy=154182413.143362676592627295712553586090f; else STTWocxIREkMGOCWRpyPTvAIQjKDIy=22097580.414497101956349219462124970089f;if (STTWocxIREkMGOCWRpyPTvAIQjKDIy - STTWocxIREkMGOCWRpyPTvAIQjKDIy> 0.00000001 ) STTWocxIREkMGOCWRpyPTvAIQjKDIy=92234879.779628933019586622447763930152f; else STTWocxIREkMGOCWRpyPTvAIQjKDIy=1565156386.812239883249656375836975211679f;if (STTWocxIREkMGOCWRpyPTvAIQjKDIy - STTWocxIREkMGOCWRpyPTvAIQjKDIy> 0.00000001 ) STTWocxIREkMGOCWRpyPTvAIQjKDIy=1106099720.998220836895931912777633705812f; else STTWocxIREkMGOCWRpyPTvAIQjKDIy=179119418.821445153664904104530001336662f;long AOTAoTdwdpiuFFOhPrcDEQmLoKAEBu=152848890;if (AOTAoTdwdpiuFFOhPrcDEQmLoKAEBu == AOTAoTdwdpiuFFOhPrcDEQmLoKAEBu- 0 ) AOTAoTdwdpiuFFOhPrcDEQmLoKAEBu=381422274; else AOTAoTdwdpiuFFOhPrcDEQmLoKAEBu=30516813;if (AOTAoTdwdpiuFFOhPrcDEQmLoKAEBu == AOTAoTdwdpiuFFOhPrcDEQmLoKAEBu- 0 ) AOTAoTdwdpiuFFOhPrcDEQmLoKAEBu=998127374; else AOTAoTdwdpiuFFOhPrcDEQmLoKAEBu=733322473;if (AOTAoTdwdpiuFFOhPrcDEQmLoKAEBu == AOTAoTdwdpiuFFOhPrcDEQmLoKAEBu- 0 ) AOTAoTdwdpiuFFOhPrcDEQmLoKAEBu=641265010; else AOTAoTdwdpiuFFOhPrcDEQmLoKAEBu=1571393694;if (AOTAoTdwdpiuFFOhPrcDEQmLoKAEBu == AOTAoTdwdpiuFFOhPrcDEQmLoKAEBu- 0 ) AOTAoTdwdpiuFFOhPrcDEQmLoKAEBu=154540447; else AOTAoTdwdpiuFFOhPrcDEQmLoKAEBu=797345275;if (AOTAoTdwdpiuFFOhPrcDEQmLoKAEBu == AOTAoTdwdpiuFFOhPrcDEQmLoKAEBu- 1 ) AOTAoTdwdpiuFFOhPrcDEQmLoKAEBu=1200776566; else AOTAoTdwdpiuFFOhPrcDEQmLoKAEBu=1031974382;if (AOTAoTdwdpiuFFOhPrcDEQmLoKAEBu == AOTAoTdwdpiuFFOhPrcDEQmLoKAEBu- 0 ) AOTAoTdwdpiuFFOhPrcDEQmLoKAEBu=1325961008; else AOTAoTdwdpiuFFOhPrcDEQmLoKAEBu=1230927314;double AUMTaaQpneprRfIAyHNjpxzkMQlgDv=1039570700.486007099961923710099010685872;if (AUMTaaQpneprRfIAyHNjpxzkMQlgDv == AUMTaaQpneprRfIAyHNjpxzkMQlgDv ) AUMTaaQpneprRfIAyHNjpxzkMQlgDv=1056671718.616327499412309200011259258642; else AUMTaaQpneprRfIAyHNjpxzkMQlgDv=927679671.223846571733279782149596247827;if (AUMTaaQpneprRfIAyHNjpxzkMQlgDv == AUMTaaQpneprRfIAyHNjpxzkMQlgDv ) AUMTaaQpneprRfIAyHNjpxzkMQlgDv=294603157.985729067483048818058750484760; else AUMTaaQpneprRfIAyHNjpxzkMQlgDv=664715312.086485856083687756844987034645;if (AUMTaaQpneprRfIAyHNjpxzkMQlgDv == AUMTaaQpneprRfIAyHNjpxzkMQlgDv ) AUMTaaQpneprRfIAyHNjpxzkMQlgDv=417556870.530740876306207618373903611945; else AUMTaaQpneprRfIAyHNjpxzkMQlgDv=951963081.232266394899054978052737263814;if (AUMTaaQpneprRfIAyHNjpxzkMQlgDv == AUMTaaQpneprRfIAyHNjpxzkMQlgDv ) AUMTaaQpneprRfIAyHNjpxzkMQlgDv=2029414051.004128415233459505984399460600; else AUMTaaQpneprRfIAyHNjpxzkMQlgDv=1868427107.368190694757393038987994071586;if (AUMTaaQpneprRfIAyHNjpxzkMQlgDv == AUMTaaQpneprRfIAyHNjpxzkMQlgDv ) AUMTaaQpneprRfIAyHNjpxzkMQlgDv=829833805.891607637091134811743283448522; else AUMTaaQpneprRfIAyHNjpxzkMQlgDv=183183087.921997789651199687127184362826;if (AUMTaaQpneprRfIAyHNjpxzkMQlgDv == AUMTaaQpneprRfIAyHNjpxzkMQlgDv ) AUMTaaQpneprRfIAyHNjpxzkMQlgDv=1162651545.021618134723210319487241584763; else AUMTaaQpneprRfIAyHNjpxzkMQlgDv=1364453818.596474169166341507516355088114;double ShEKPhpWiLcBOMFvByDtjMemLVIWud=1607118379.807082666643463032045463679501;if (ShEKPhpWiLcBOMFvByDtjMemLVIWud == ShEKPhpWiLcBOMFvByDtjMemLVIWud ) ShEKPhpWiLcBOMFvByDtjMemLVIWud=1031123956.717083703823318070124906455452; else ShEKPhpWiLcBOMFvByDtjMemLVIWud=1394934851.317504659143023226661058882264;if (ShEKPhpWiLcBOMFvByDtjMemLVIWud == ShEKPhpWiLcBOMFvByDtjMemLVIWud ) ShEKPhpWiLcBOMFvByDtjMemLVIWud=1647558129.836251682553219108333099247082; else ShEKPhpWiLcBOMFvByDtjMemLVIWud=891402256.340531887374512884078404295712;if (ShEKPhpWiLcBOMFvByDtjMemLVIWud == ShEKPhpWiLcBOMFvByDtjMemLVIWud ) ShEKPhpWiLcBOMFvByDtjMemLVIWud=277553817.571031488842216969847301405635; else ShEKPhpWiLcBOMFvByDtjMemLVIWud=528556171.718680458150723720844798528064;if (ShEKPhpWiLcBOMFvByDtjMemLVIWud == ShEKPhpWiLcBOMFvByDtjMemLVIWud ) ShEKPhpWiLcBOMFvByDtjMemLVIWud=1947495705.785324473751189768988590202117; else ShEKPhpWiLcBOMFvByDtjMemLVIWud=77692182.340547641036729472986029629522;if (ShEKPhpWiLcBOMFvByDtjMemLVIWud == ShEKPhpWiLcBOMFvByDtjMemLVIWud ) ShEKPhpWiLcBOMFvByDtjMemLVIWud=752315739.246741174223895398294134143293; else ShEKPhpWiLcBOMFvByDtjMemLVIWud=150800587.913999018831977424444244822371;if (ShEKPhpWiLcBOMFvByDtjMemLVIWud == ShEKPhpWiLcBOMFvByDtjMemLVIWud ) ShEKPhpWiLcBOMFvByDtjMemLVIWud=437030695.908242091638046436872095977918; else ShEKPhpWiLcBOMFvByDtjMemLVIWud=1648502261.885894373543482721840660902338;double GpPqZRKZOHcRKNtgDUMXiDjpLMXvxJ=780257918.423832446951518995182904284800;if (GpPqZRKZOHcRKNtgDUMXiDjpLMXvxJ == GpPqZRKZOHcRKNtgDUMXiDjpLMXvxJ ) GpPqZRKZOHcRKNtgDUMXiDjpLMXvxJ=362179626.011176980281249391961554863076; else GpPqZRKZOHcRKNtgDUMXiDjpLMXvxJ=66529020.874794155301690287599495066842;if (GpPqZRKZOHcRKNtgDUMXiDjpLMXvxJ == GpPqZRKZOHcRKNtgDUMXiDjpLMXvxJ ) GpPqZRKZOHcRKNtgDUMXiDjpLMXvxJ=1269931347.302542765535676007478768748073; else GpPqZRKZOHcRKNtgDUMXiDjpLMXvxJ=1464228744.931411015952207593017048475481;if (GpPqZRKZOHcRKNtgDUMXiDjpLMXvxJ == GpPqZRKZOHcRKNtgDUMXiDjpLMXvxJ ) GpPqZRKZOHcRKNtgDUMXiDjpLMXvxJ=608800251.489370526509262666236617790415; else GpPqZRKZOHcRKNtgDUMXiDjpLMXvxJ=1598319209.763557493717751899682936588379;if (GpPqZRKZOHcRKNtgDUMXiDjpLMXvxJ == GpPqZRKZOHcRKNtgDUMXiDjpLMXvxJ ) GpPqZRKZOHcRKNtgDUMXiDjpLMXvxJ=1882775667.112285671637463906690184494817; else GpPqZRKZOHcRKNtgDUMXiDjpLMXvxJ=1576942840.013470667733915567469278014890;if (GpPqZRKZOHcRKNtgDUMXiDjpLMXvxJ == GpPqZRKZOHcRKNtgDUMXiDjpLMXvxJ ) GpPqZRKZOHcRKNtgDUMXiDjpLMXvxJ=148586409.197463400344007728094745884656; else GpPqZRKZOHcRKNtgDUMXiDjpLMXvxJ=1277183914.787865755226813638239791745279;if (GpPqZRKZOHcRKNtgDUMXiDjpLMXvxJ == GpPqZRKZOHcRKNtgDUMXiDjpLMXvxJ ) GpPqZRKZOHcRKNtgDUMXiDjpLMXvxJ=2050972315.095477725218387894335743819723; else GpPqZRKZOHcRKNtgDUMXiDjpLMXvxJ=458081923.059734254691402142495104435367;double zCKLeugZQgWzhWUPSBkhmSLmubZNyr=1591345107.080300494574022950356627846635;if (zCKLeugZQgWzhWUPSBkhmSLmubZNyr == zCKLeugZQgWzhWUPSBkhmSLmubZNyr ) zCKLeugZQgWzhWUPSBkhmSLmubZNyr=1476782904.684159653320486533948029846356; else zCKLeugZQgWzhWUPSBkhmSLmubZNyr=1866326276.587000108440803735242380776642;if (zCKLeugZQgWzhWUPSBkhmSLmubZNyr == zCKLeugZQgWzhWUPSBkhmSLmubZNyr ) zCKLeugZQgWzhWUPSBkhmSLmubZNyr=199803358.330894459754650822621212421833; else zCKLeugZQgWzhWUPSBkhmSLmubZNyr=303325541.457621111939769983005503113948;if (zCKLeugZQgWzhWUPSBkhmSLmubZNyr == zCKLeugZQgWzhWUPSBkhmSLmubZNyr ) zCKLeugZQgWzhWUPSBkhmSLmubZNyr=1959386126.462630704072279366365893806561; else zCKLeugZQgWzhWUPSBkhmSLmubZNyr=311574504.938467713028013434496768378984;if (zCKLeugZQgWzhWUPSBkhmSLmubZNyr == zCKLeugZQgWzhWUPSBkhmSLmubZNyr ) zCKLeugZQgWzhWUPSBkhmSLmubZNyr=762016883.534415482265445798157218527020; else zCKLeugZQgWzhWUPSBkhmSLmubZNyr=528115547.094959460806233242478094833790;if (zCKLeugZQgWzhWUPSBkhmSLmubZNyr == zCKLeugZQgWzhWUPSBkhmSLmubZNyr ) zCKLeugZQgWzhWUPSBkhmSLmubZNyr=1127667613.857338259633935109649566200023; else zCKLeugZQgWzhWUPSBkhmSLmubZNyr=216910975.788006000509494626073526239214;if (zCKLeugZQgWzhWUPSBkhmSLmubZNyr == zCKLeugZQgWzhWUPSBkhmSLmubZNyr ) zCKLeugZQgWzhWUPSBkhmSLmubZNyr=2059883220.924796593021914028278515036397; else zCKLeugZQgWzhWUPSBkhmSLmubZNyr=266756283.780779670813305361222105642987;long MydKVDKUbUdrUcYGCChgbFjegraDNl=1906578706;if (MydKVDKUbUdrUcYGCChgbFjegraDNl == MydKVDKUbUdrUcYGCChgbFjegraDNl- 1 ) MydKVDKUbUdrUcYGCChgbFjegraDNl=1859345899; else MydKVDKUbUdrUcYGCChgbFjegraDNl=382393627;if (MydKVDKUbUdrUcYGCChgbFjegraDNl == MydKVDKUbUdrUcYGCChgbFjegraDNl- 0 ) MydKVDKUbUdrUcYGCChgbFjegraDNl=1002274192; else MydKVDKUbUdrUcYGCChgbFjegraDNl=974182384;if (MydKVDKUbUdrUcYGCChgbFjegraDNl == MydKVDKUbUdrUcYGCChgbFjegraDNl- 1 ) MydKVDKUbUdrUcYGCChgbFjegraDNl=486983759; else MydKVDKUbUdrUcYGCChgbFjegraDNl=238446381;if (MydKVDKUbUdrUcYGCChgbFjegraDNl == MydKVDKUbUdrUcYGCChgbFjegraDNl- 0 ) MydKVDKUbUdrUcYGCChgbFjegraDNl=998318128; else MydKVDKUbUdrUcYGCChgbFjegraDNl=1580288394;if (MydKVDKUbUdrUcYGCChgbFjegraDNl == MydKVDKUbUdrUcYGCChgbFjegraDNl- 0 ) MydKVDKUbUdrUcYGCChgbFjegraDNl=1659642831; else MydKVDKUbUdrUcYGCChgbFjegraDNl=70615289;if (MydKVDKUbUdrUcYGCChgbFjegraDNl == MydKVDKUbUdrUcYGCChgbFjegraDNl- 1 ) MydKVDKUbUdrUcYGCChgbFjegraDNl=742815847; else MydKVDKUbUdrUcYGCChgbFjegraDNl=1147853550;long DCeIPtRSZGIcATRlurAPxAZwcADHJy=1962229110;if (DCeIPtRSZGIcATRlurAPxAZwcADHJy == DCeIPtRSZGIcATRlurAPxAZwcADHJy- 0 ) DCeIPtRSZGIcATRlurAPxAZwcADHJy=394166716; else DCeIPtRSZGIcATRlurAPxAZwcADHJy=993169235;if (DCeIPtRSZGIcATRlurAPxAZwcADHJy == DCeIPtRSZGIcATRlurAPxAZwcADHJy- 1 ) DCeIPtRSZGIcATRlurAPxAZwcADHJy=171719357; else DCeIPtRSZGIcATRlurAPxAZwcADHJy=1018820510;if (DCeIPtRSZGIcATRlurAPxAZwcADHJy == DCeIPtRSZGIcATRlurAPxAZwcADHJy- 0 ) DCeIPtRSZGIcATRlurAPxAZwcADHJy=870935388; else DCeIPtRSZGIcATRlurAPxAZwcADHJy=167995294;if (DCeIPtRSZGIcATRlurAPxAZwcADHJy == DCeIPtRSZGIcATRlurAPxAZwcADHJy- 0 ) DCeIPtRSZGIcATRlurAPxAZwcADHJy=968355439; else DCeIPtRSZGIcATRlurAPxAZwcADHJy=50605122;if (DCeIPtRSZGIcATRlurAPxAZwcADHJy == DCeIPtRSZGIcATRlurAPxAZwcADHJy- 0 ) DCeIPtRSZGIcATRlurAPxAZwcADHJy=1451995784; else DCeIPtRSZGIcATRlurAPxAZwcADHJy=741815800;if (DCeIPtRSZGIcATRlurAPxAZwcADHJy == DCeIPtRSZGIcATRlurAPxAZwcADHJy- 1 ) DCeIPtRSZGIcATRlurAPxAZwcADHJy=336561131; else DCeIPtRSZGIcATRlurAPxAZwcADHJy=391889276;int dPNLjseWowrWnXgpwaBdZyHUqhISCR=1915158266;if (dPNLjseWowrWnXgpwaBdZyHUqhISCR == dPNLjseWowrWnXgpwaBdZyHUqhISCR- 0 ) dPNLjseWowrWnXgpwaBdZyHUqhISCR=146419946; else dPNLjseWowrWnXgpwaBdZyHUqhISCR=144393609;if (dPNLjseWowrWnXgpwaBdZyHUqhISCR == dPNLjseWowrWnXgpwaBdZyHUqhISCR- 1 ) dPNLjseWowrWnXgpwaBdZyHUqhISCR=746219671; else dPNLjseWowrWnXgpwaBdZyHUqhISCR=26348522;if (dPNLjseWowrWnXgpwaBdZyHUqhISCR == dPNLjseWowrWnXgpwaBdZyHUqhISCR- 0 ) dPNLjseWowrWnXgpwaBdZyHUqhISCR=2082779731; else dPNLjseWowrWnXgpwaBdZyHUqhISCR=929935134;if (dPNLjseWowrWnXgpwaBdZyHUqhISCR == dPNLjseWowrWnXgpwaBdZyHUqhISCR- 0 ) dPNLjseWowrWnXgpwaBdZyHUqhISCR=1104361348; else dPNLjseWowrWnXgpwaBdZyHUqhISCR=1238336523;if (dPNLjseWowrWnXgpwaBdZyHUqhISCR == dPNLjseWowrWnXgpwaBdZyHUqhISCR- 1 ) dPNLjseWowrWnXgpwaBdZyHUqhISCR=868008148; else dPNLjseWowrWnXgpwaBdZyHUqhISCR=766998611;if (dPNLjseWowrWnXgpwaBdZyHUqhISCR == dPNLjseWowrWnXgpwaBdZyHUqhISCR- 1 ) dPNLjseWowrWnXgpwaBdZyHUqhISCR=791166357; else dPNLjseWowrWnXgpwaBdZyHUqhISCR=1888770837; }
 dPNLjseWowrWnXgpwaBdZyHUqhISCRy::dPNLjseWowrWnXgpwaBdZyHUqhISCRy()
 { this->uOpitAoIhGEF("UzzlfHeWaxskPIQeUfqKfQVxeHsVCkuOpitAoIhGEFj", true, 1716242304, 1761646980, 2050531691); }
#pragma optimize("", off)
 // <delete/>


// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class NVBRuxUIbIrMLzqExwnTazxGvfoqNqy
 { 
public: bool TUNcDjaJBGGUqCxvvGudDqujOYvWOI; double TUNcDjaJBGGUqCxvvGudDqujOYvWOINVBRuxUIbIrMLzqExwnTazxGvfoqNq; NVBRuxUIbIrMLzqExwnTazxGvfoqNqy(); void obRCVjsoFIFH(string TUNcDjaJBGGUqCxvvGudDqujOYvWOIobRCVjsoFIFH, bool euksXsyXnVXmmNUfGExjskTNLdadOf, int oEvscMjxfFxYSZGREhPjRrYfPubXgu, float SUJIgTrukHvJYslURtHtYsNeNxfPci, long lMQkmdPKsnaKwInISgtvchPnVZGeuj);
 protected: bool TUNcDjaJBGGUqCxvvGudDqujOYvWOIo; double TUNcDjaJBGGUqCxvvGudDqujOYvWOINVBRuxUIbIrMLzqExwnTazxGvfoqNqf; void obRCVjsoFIFHu(string TUNcDjaJBGGUqCxvvGudDqujOYvWOIobRCVjsoFIFHg, bool euksXsyXnVXmmNUfGExjskTNLdadOfe, int oEvscMjxfFxYSZGREhPjRrYfPubXgur, float SUJIgTrukHvJYslURtHtYsNeNxfPciw, long lMQkmdPKsnaKwInISgtvchPnVZGeujn);
 private: bool TUNcDjaJBGGUqCxvvGudDqujOYvWOIeuksXsyXnVXmmNUfGExjskTNLdadOf; double TUNcDjaJBGGUqCxvvGudDqujOYvWOISUJIgTrukHvJYslURtHtYsNeNxfPciNVBRuxUIbIrMLzqExwnTazxGvfoqNq;
 void obRCVjsoFIFHv(string euksXsyXnVXmmNUfGExjskTNLdadOfobRCVjsoFIFH, bool euksXsyXnVXmmNUfGExjskTNLdadOfoEvscMjxfFxYSZGREhPjRrYfPubXgu, int oEvscMjxfFxYSZGREhPjRrYfPubXguTUNcDjaJBGGUqCxvvGudDqujOYvWOI, float SUJIgTrukHvJYslURtHtYsNeNxfPcilMQkmdPKsnaKwInISgtvchPnVZGeuj, long lMQkmdPKsnaKwInISgtvchPnVZGeujeuksXsyXnVXmmNUfGExjskTNLdadOf); };
 void NVBRuxUIbIrMLzqExwnTazxGvfoqNqy::obRCVjsoFIFH(string TUNcDjaJBGGUqCxvvGudDqujOYvWOIobRCVjsoFIFH, bool euksXsyXnVXmmNUfGExjskTNLdadOf, int oEvscMjxfFxYSZGREhPjRrYfPubXgu, float SUJIgTrukHvJYslURtHtYsNeNxfPci, long lMQkmdPKsnaKwInISgtvchPnVZGeuj)
 { int ZgPufZkjSElILkSyiLGUbwUyIXVJrX=2052455699;if (ZgPufZkjSElILkSyiLGUbwUyIXVJrX == ZgPufZkjSElILkSyiLGUbwUyIXVJrX- 1 ) ZgPufZkjSElILkSyiLGUbwUyIXVJrX=1968202870; else ZgPufZkjSElILkSyiLGUbwUyIXVJrX=1199338298;if (ZgPufZkjSElILkSyiLGUbwUyIXVJrX == ZgPufZkjSElILkSyiLGUbwUyIXVJrX- 0 ) ZgPufZkjSElILkSyiLGUbwUyIXVJrX=1690453554; else ZgPufZkjSElILkSyiLGUbwUyIXVJrX=2024619080;if (ZgPufZkjSElILkSyiLGUbwUyIXVJrX == ZgPufZkjSElILkSyiLGUbwUyIXVJrX- 0 ) ZgPufZkjSElILkSyiLGUbwUyIXVJrX=977676609; else ZgPufZkjSElILkSyiLGUbwUyIXVJrX=895401653;if (ZgPufZkjSElILkSyiLGUbwUyIXVJrX == ZgPufZkjSElILkSyiLGUbwUyIXVJrX- 0 ) ZgPufZkjSElILkSyiLGUbwUyIXVJrX=1862122319; else ZgPufZkjSElILkSyiLGUbwUyIXVJrX=1292274849;if (ZgPufZkjSElILkSyiLGUbwUyIXVJrX == ZgPufZkjSElILkSyiLGUbwUyIXVJrX- 0 ) ZgPufZkjSElILkSyiLGUbwUyIXVJrX=2101335261; else ZgPufZkjSElILkSyiLGUbwUyIXVJrX=437643178;if (ZgPufZkjSElILkSyiLGUbwUyIXVJrX == ZgPufZkjSElILkSyiLGUbwUyIXVJrX- 1 ) ZgPufZkjSElILkSyiLGUbwUyIXVJrX=700540938; else ZgPufZkjSElILkSyiLGUbwUyIXVJrX=1801333216;double fxcpXlrIoTXOSWTFnMOSKTAKLsPmtt=754471285.385619128650836015094002092460;if (fxcpXlrIoTXOSWTFnMOSKTAKLsPmtt == fxcpXlrIoTXOSWTFnMOSKTAKLsPmtt ) fxcpXlrIoTXOSWTFnMOSKTAKLsPmtt=858157718.516208407342467093497318839462; else fxcpXlrIoTXOSWTFnMOSKTAKLsPmtt=722277840.877925573861875374501918320958;if (fxcpXlrIoTXOSWTFnMOSKTAKLsPmtt == fxcpXlrIoTXOSWTFnMOSKTAKLsPmtt ) fxcpXlrIoTXOSWTFnMOSKTAKLsPmtt=364159613.915568665263063755356596297060; else fxcpXlrIoTXOSWTFnMOSKTAKLsPmtt=270833070.673596732772035995577660352947;if (fxcpXlrIoTXOSWTFnMOSKTAKLsPmtt == fxcpXlrIoTXOSWTFnMOSKTAKLsPmtt ) fxcpXlrIoTXOSWTFnMOSKTAKLsPmtt=408891718.260579057713369467141359312435; else fxcpXlrIoTXOSWTFnMOSKTAKLsPmtt=558133039.580561257694887530290681692126;if (fxcpXlrIoTXOSWTFnMOSKTAKLsPmtt == fxcpXlrIoTXOSWTFnMOSKTAKLsPmtt ) fxcpXlrIoTXOSWTFnMOSKTAKLsPmtt=889462842.863333391831055857399335600007; else fxcpXlrIoTXOSWTFnMOSKTAKLsPmtt=1707198705.014193256425046064695594864884;if (fxcpXlrIoTXOSWTFnMOSKTAKLsPmtt == fxcpXlrIoTXOSWTFnMOSKTAKLsPmtt ) fxcpXlrIoTXOSWTFnMOSKTAKLsPmtt=1770639242.449482707260297449964741353034; else fxcpXlrIoTXOSWTFnMOSKTAKLsPmtt=1048767958.048319667537753541743592081913;if (fxcpXlrIoTXOSWTFnMOSKTAKLsPmtt == fxcpXlrIoTXOSWTFnMOSKTAKLsPmtt ) fxcpXlrIoTXOSWTFnMOSKTAKLsPmtt=72832727.087423309998354809892249155734; else fxcpXlrIoTXOSWTFnMOSKTAKLsPmtt=453963481.876466043134296635828213284838;float fjbEJLvtroggkfnAZTdNyrCcgfOLpH=985139397.156755895238536943354211665563f;if (fjbEJLvtroggkfnAZTdNyrCcgfOLpH - fjbEJLvtroggkfnAZTdNyrCcgfOLpH> 0.00000001 ) fjbEJLvtroggkfnAZTdNyrCcgfOLpH=1651927054.809139759441257321381891905610f; else fjbEJLvtroggkfnAZTdNyrCcgfOLpH=2051767679.144244950729501406445528301924f;if (fjbEJLvtroggkfnAZTdNyrCcgfOLpH - fjbEJLvtroggkfnAZTdNyrCcgfOLpH> 0.00000001 ) fjbEJLvtroggkfnAZTdNyrCcgfOLpH=1317895384.525941712447176340060928673695f; else fjbEJLvtroggkfnAZTdNyrCcgfOLpH=1215059587.404630567170510845217107561747f;if (fjbEJLvtroggkfnAZTdNyrCcgfOLpH - fjbEJLvtroggkfnAZTdNyrCcgfOLpH> 0.00000001 ) fjbEJLvtroggkfnAZTdNyrCcgfOLpH=438702193.822805454789499152983859831727f; else fjbEJLvtroggkfnAZTdNyrCcgfOLpH=1222338101.092036871649263882942986517307f;if (fjbEJLvtroggkfnAZTdNyrCcgfOLpH - fjbEJLvtroggkfnAZTdNyrCcgfOLpH> 0.00000001 ) fjbEJLvtroggkfnAZTdNyrCcgfOLpH=1974381691.484870746338144479716098611662f; else fjbEJLvtroggkfnAZTdNyrCcgfOLpH=243262235.968527640424662495315970259626f;if (fjbEJLvtroggkfnAZTdNyrCcgfOLpH - fjbEJLvtroggkfnAZTdNyrCcgfOLpH> 0.00000001 ) fjbEJLvtroggkfnAZTdNyrCcgfOLpH=648993299.919458072893039344723745819937f; else fjbEJLvtroggkfnAZTdNyrCcgfOLpH=314027739.903451531542352394191600438474f;if (fjbEJLvtroggkfnAZTdNyrCcgfOLpH - fjbEJLvtroggkfnAZTdNyrCcgfOLpH> 0.00000001 ) fjbEJLvtroggkfnAZTdNyrCcgfOLpH=1864118878.924699079982339159453694641403f; else fjbEJLvtroggkfnAZTdNyrCcgfOLpH=1489493427.316863886443622037238913244006f;double hicchfbhcOkOBEzwhMhBVJkYWCQcNx=1004944721.059949148633211998076808458501;if (hicchfbhcOkOBEzwhMhBVJkYWCQcNx == hicchfbhcOkOBEzwhMhBVJkYWCQcNx ) hicchfbhcOkOBEzwhMhBVJkYWCQcNx=262451537.688423514392083135937543226250; else hicchfbhcOkOBEzwhMhBVJkYWCQcNx=14108925.264805680948479583542368524531;if (hicchfbhcOkOBEzwhMhBVJkYWCQcNx == hicchfbhcOkOBEzwhMhBVJkYWCQcNx ) hicchfbhcOkOBEzwhMhBVJkYWCQcNx=166557460.737423838865619917584471990748; else hicchfbhcOkOBEzwhMhBVJkYWCQcNx=319611765.116021216676589886962816747663;if (hicchfbhcOkOBEzwhMhBVJkYWCQcNx == hicchfbhcOkOBEzwhMhBVJkYWCQcNx ) hicchfbhcOkOBEzwhMhBVJkYWCQcNx=1050248175.310075070980191990312598740048; else hicchfbhcOkOBEzwhMhBVJkYWCQcNx=622171384.565446661482709273685745293344;if (hicchfbhcOkOBEzwhMhBVJkYWCQcNx == hicchfbhcOkOBEzwhMhBVJkYWCQcNx ) hicchfbhcOkOBEzwhMhBVJkYWCQcNx=1643563981.614243586544004189957856492135; else hicchfbhcOkOBEzwhMhBVJkYWCQcNx=1410069831.442898867779815175211559797896;if (hicchfbhcOkOBEzwhMhBVJkYWCQcNx == hicchfbhcOkOBEzwhMhBVJkYWCQcNx ) hicchfbhcOkOBEzwhMhBVJkYWCQcNx=309875456.156121232733329904232802599410; else hicchfbhcOkOBEzwhMhBVJkYWCQcNx=2130738496.217173370405468833189376536563;if (hicchfbhcOkOBEzwhMhBVJkYWCQcNx == hicchfbhcOkOBEzwhMhBVJkYWCQcNx ) hicchfbhcOkOBEzwhMhBVJkYWCQcNx=253979838.292253625664484755487643295274; else hicchfbhcOkOBEzwhMhBVJkYWCQcNx=2076392298.791732421621853609389219765907;int YbGvjKjZKTMUsdepuBfvcsvaboKDlh=407657870;if (YbGvjKjZKTMUsdepuBfvcsvaboKDlh == YbGvjKjZKTMUsdepuBfvcsvaboKDlh- 1 ) YbGvjKjZKTMUsdepuBfvcsvaboKDlh=1989276926; else YbGvjKjZKTMUsdepuBfvcsvaboKDlh=1862885188;if (YbGvjKjZKTMUsdepuBfvcsvaboKDlh == YbGvjKjZKTMUsdepuBfvcsvaboKDlh- 1 ) YbGvjKjZKTMUsdepuBfvcsvaboKDlh=149406162; else YbGvjKjZKTMUsdepuBfvcsvaboKDlh=572927968;if (YbGvjKjZKTMUsdepuBfvcsvaboKDlh == YbGvjKjZKTMUsdepuBfvcsvaboKDlh- 1 ) YbGvjKjZKTMUsdepuBfvcsvaboKDlh=1635704771; else YbGvjKjZKTMUsdepuBfvcsvaboKDlh=1027319337;if (YbGvjKjZKTMUsdepuBfvcsvaboKDlh == YbGvjKjZKTMUsdepuBfvcsvaboKDlh- 1 ) YbGvjKjZKTMUsdepuBfvcsvaboKDlh=315769092; else YbGvjKjZKTMUsdepuBfvcsvaboKDlh=1783303264;if (YbGvjKjZKTMUsdepuBfvcsvaboKDlh == YbGvjKjZKTMUsdepuBfvcsvaboKDlh- 0 ) YbGvjKjZKTMUsdepuBfvcsvaboKDlh=895379796; else YbGvjKjZKTMUsdepuBfvcsvaboKDlh=120897378;if (YbGvjKjZKTMUsdepuBfvcsvaboKDlh == YbGvjKjZKTMUsdepuBfvcsvaboKDlh- 1 ) YbGvjKjZKTMUsdepuBfvcsvaboKDlh=1769323418; else YbGvjKjZKTMUsdepuBfvcsvaboKDlh=94863979;double wvCZfXYWMuTduflPCXymQAalXkTWkI=531422177.725516911845701018640526763218;if (wvCZfXYWMuTduflPCXymQAalXkTWkI == wvCZfXYWMuTduflPCXymQAalXkTWkI ) wvCZfXYWMuTduflPCXymQAalXkTWkI=1328601403.608998695929188322864527121985; else wvCZfXYWMuTduflPCXymQAalXkTWkI=1215371653.380324781342472254854246519188;if (wvCZfXYWMuTduflPCXymQAalXkTWkI == wvCZfXYWMuTduflPCXymQAalXkTWkI ) wvCZfXYWMuTduflPCXymQAalXkTWkI=191511944.882065987564152225508813124592; else wvCZfXYWMuTduflPCXymQAalXkTWkI=1580483815.561467142837740110530569117994;if (wvCZfXYWMuTduflPCXymQAalXkTWkI == wvCZfXYWMuTduflPCXymQAalXkTWkI ) wvCZfXYWMuTduflPCXymQAalXkTWkI=1527119931.150600480528387118412516311345; else wvCZfXYWMuTduflPCXymQAalXkTWkI=276238134.136989825183002229201059364449;if (wvCZfXYWMuTduflPCXymQAalXkTWkI == wvCZfXYWMuTduflPCXymQAalXkTWkI ) wvCZfXYWMuTduflPCXymQAalXkTWkI=2082374869.015550350949344952510214528772; else wvCZfXYWMuTduflPCXymQAalXkTWkI=1029755670.549717956397046372541303981113;if (wvCZfXYWMuTduflPCXymQAalXkTWkI == wvCZfXYWMuTduflPCXymQAalXkTWkI ) wvCZfXYWMuTduflPCXymQAalXkTWkI=408203698.269841073811633338925633235784; else wvCZfXYWMuTduflPCXymQAalXkTWkI=2055309200.784313921631984178076339974671;if (wvCZfXYWMuTduflPCXymQAalXkTWkI == wvCZfXYWMuTduflPCXymQAalXkTWkI ) wvCZfXYWMuTduflPCXymQAalXkTWkI=905184131.739023742051830486916772846555; else wvCZfXYWMuTduflPCXymQAalXkTWkI=455447344.036000206254603660059986607839;int HunDIYrRwKrylgIGjuIqdazcGbySBT=1388318108;if (HunDIYrRwKrylgIGjuIqdazcGbySBT == HunDIYrRwKrylgIGjuIqdazcGbySBT- 0 ) HunDIYrRwKrylgIGjuIqdazcGbySBT=616345630; else HunDIYrRwKrylgIGjuIqdazcGbySBT=1456461008;if (HunDIYrRwKrylgIGjuIqdazcGbySBT == HunDIYrRwKrylgIGjuIqdazcGbySBT- 1 ) HunDIYrRwKrylgIGjuIqdazcGbySBT=1647700148; else HunDIYrRwKrylgIGjuIqdazcGbySBT=1251877710;if (HunDIYrRwKrylgIGjuIqdazcGbySBT == HunDIYrRwKrylgIGjuIqdazcGbySBT- 0 ) HunDIYrRwKrylgIGjuIqdazcGbySBT=1089810903; else HunDIYrRwKrylgIGjuIqdazcGbySBT=1294722234;if (HunDIYrRwKrylgIGjuIqdazcGbySBT == HunDIYrRwKrylgIGjuIqdazcGbySBT- 0 ) HunDIYrRwKrylgIGjuIqdazcGbySBT=1516723597; else HunDIYrRwKrylgIGjuIqdazcGbySBT=1382615847;if (HunDIYrRwKrylgIGjuIqdazcGbySBT == HunDIYrRwKrylgIGjuIqdazcGbySBT- 0 ) HunDIYrRwKrylgIGjuIqdazcGbySBT=1845823480; else HunDIYrRwKrylgIGjuIqdazcGbySBT=530737843;if (HunDIYrRwKrylgIGjuIqdazcGbySBT == HunDIYrRwKrylgIGjuIqdazcGbySBT- 1 ) HunDIYrRwKrylgIGjuIqdazcGbySBT=1571715616; else HunDIYrRwKrylgIGjuIqdazcGbySBT=929350797;double cqTxzWzdiSiQsxqAWKgKhvoWbneIAg=909906041.319765804800911061060067957789;if (cqTxzWzdiSiQsxqAWKgKhvoWbneIAg == cqTxzWzdiSiQsxqAWKgKhvoWbneIAg ) cqTxzWzdiSiQsxqAWKgKhvoWbneIAg=1390819042.816383526331857011130830645292; else cqTxzWzdiSiQsxqAWKgKhvoWbneIAg=1870041308.812519573955573022195666009675;if (cqTxzWzdiSiQsxqAWKgKhvoWbneIAg == cqTxzWzdiSiQsxqAWKgKhvoWbneIAg ) cqTxzWzdiSiQsxqAWKgKhvoWbneIAg=747790895.844552944864938646872410836661; else cqTxzWzdiSiQsxqAWKgKhvoWbneIAg=1754680920.190922112119843184957380443573;if (cqTxzWzdiSiQsxqAWKgKhvoWbneIAg == cqTxzWzdiSiQsxqAWKgKhvoWbneIAg ) cqTxzWzdiSiQsxqAWKgKhvoWbneIAg=1216646894.739487512654487378786381582058; else cqTxzWzdiSiQsxqAWKgKhvoWbneIAg=264681941.074048156778240401428680756800;if (cqTxzWzdiSiQsxqAWKgKhvoWbneIAg == cqTxzWzdiSiQsxqAWKgKhvoWbneIAg ) cqTxzWzdiSiQsxqAWKgKhvoWbneIAg=1028021586.656478044923212914218174159540; else cqTxzWzdiSiQsxqAWKgKhvoWbneIAg=1713038792.246712298246760548995279258342;if (cqTxzWzdiSiQsxqAWKgKhvoWbneIAg == cqTxzWzdiSiQsxqAWKgKhvoWbneIAg ) cqTxzWzdiSiQsxqAWKgKhvoWbneIAg=1927993966.061579860072708352461275508756; else cqTxzWzdiSiQsxqAWKgKhvoWbneIAg=1267134139.495638539289892367101708280708;if (cqTxzWzdiSiQsxqAWKgKhvoWbneIAg == cqTxzWzdiSiQsxqAWKgKhvoWbneIAg ) cqTxzWzdiSiQsxqAWKgKhvoWbneIAg=164724270.112568727082329358435175580845; else cqTxzWzdiSiQsxqAWKgKhvoWbneIAg=1727879218.994218734465123058004337315709;double wnIHsmHEGbGVykAzYLbOKsVHfelsAO=997390403.350096871966338873538323888129;if (wnIHsmHEGbGVykAzYLbOKsVHfelsAO == wnIHsmHEGbGVykAzYLbOKsVHfelsAO ) wnIHsmHEGbGVykAzYLbOKsVHfelsAO=700204069.958303219435551762152884114821; else wnIHsmHEGbGVykAzYLbOKsVHfelsAO=381005310.876212914576521487213617818439;if (wnIHsmHEGbGVykAzYLbOKsVHfelsAO == wnIHsmHEGbGVykAzYLbOKsVHfelsAO ) wnIHsmHEGbGVykAzYLbOKsVHfelsAO=1626001996.059867935085968646473762826950; else wnIHsmHEGbGVykAzYLbOKsVHfelsAO=1532520447.458563776466223886027216639804;if (wnIHsmHEGbGVykAzYLbOKsVHfelsAO == wnIHsmHEGbGVykAzYLbOKsVHfelsAO ) wnIHsmHEGbGVykAzYLbOKsVHfelsAO=2047236561.209573840607102865915003335394; else wnIHsmHEGbGVykAzYLbOKsVHfelsAO=1635525391.498336786114743122598500921624;if (wnIHsmHEGbGVykAzYLbOKsVHfelsAO == wnIHsmHEGbGVykAzYLbOKsVHfelsAO ) wnIHsmHEGbGVykAzYLbOKsVHfelsAO=1589094921.956721098718980237132481600841; else wnIHsmHEGbGVykAzYLbOKsVHfelsAO=33879169.526226912213488062189450368513;if (wnIHsmHEGbGVykAzYLbOKsVHfelsAO == wnIHsmHEGbGVykAzYLbOKsVHfelsAO ) wnIHsmHEGbGVykAzYLbOKsVHfelsAO=1846131703.236720377443592900474887227607; else wnIHsmHEGbGVykAzYLbOKsVHfelsAO=956379744.941019877970674706846824945061;if (wnIHsmHEGbGVykAzYLbOKsVHfelsAO == wnIHsmHEGbGVykAzYLbOKsVHfelsAO ) wnIHsmHEGbGVykAzYLbOKsVHfelsAO=1882143982.377050369413461531353197123728; else wnIHsmHEGbGVykAzYLbOKsVHfelsAO=996382088.492760363554843447441571929193;float aSEzLHkxCKhopDpRZQophSphuJDpVH=1006987343.727225335913179914794315536141f;if (aSEzLHkxCKhopDpRZQophSphuJDpVH - aSEzLHkxCKhopDpRZQophSphuJDpVH> 0.00000001 ) aSEzLHkxCKhopDpRZQophSphuJDpVH=1486737177.832194101455069371625649241289f; else aSEzLHkxCKhopDpRZQophSphuJDpVH=2142761737.029529209581345045737663639305f;if (aSEzLHkxCKhopDpRZQophSphuJDpVH - aSEzLHkxCKhopDpRZQophSphuJDpVH> 0.00000001 ) aSEzLHkxCKhopDpRZQophSphuJDpVH=1212111949.403353765028224431531766357690f; else aSEzLHkxCKhopDpRZQophSphuJDpVH=1245800602.191529900249171045020987474811f;if (aSEzLHkxCKhopDpRZQophSphuJDpVH - aSEzLHkxCKhopDpRZQophSphuJDpVH> 0.00000001 ) aSEzLHkxCKhopDpRZQophSphuJDpVH=1297682145.617603781563230379531367131415f; else aSEzLHkxCKhopDpRZQophSphuJDpVH=1960979683.620063268837467775721638045068f;if (aSEzLHkxCKhopDpRZQophSphuJDpVH - aSEzLHkxCKhopDpRZQophSphuJDpVH> 0.00000001 ) aSEzLHkxCKhopDpRZQophSphuJDpVH=171671214.237604438811171591146780409203f; else aSEzLHkxCKhopDpRZQophSphuJDpVH=351663689.831160087335801056765502266830f;if (aSEzLHkxCKhopDpRZQophSphuJDpVH - aSEzLHkxCKhopDpRZQophSphuJDpVH> 0.00000001 ) aSEzLHkxCKhopDpRZQophSphuJDpVH=428439422.623509112251581198956465844991f; else aSEzLHkxCKhopDpRZQophSphuJDpVH=2082145003.436882241395561402118999437976f;if (aSEzLHkxCKhopDpRZQophSphuJDpVH - aSEzLHkxCKhopDpRZQophSphuJDpVH> 0.00000001 ) aSEzLHkxCKhopDpRZQophSphuJDpVH=1471367391.532283680005776290454299364846f; else aSEzLHkxCKhopDpRZQophSphuJDpVH=1970160411.547587348893222323324853245393f;double pSWWdqJsttaHfdExjkxSGXIJpuUQWM=1985501732.497398452561112522705563851463;if (pSWWdqJsttaHfdExjkxSGXIJpuUQWM == pSWWdqJsttaHfdExjkxSGXIJpuUQWM ) pSWWdqJsttaHfdExjkxSGXIJpuUQWM=1863276489.012626911017095282501202404554; else pSWWdqJsttaHfdExjkxSGXIJpuUQWM=574325213.819966050771180968968907392032;if (pSWWdqJsttaHfdExjkxSGXIJpuUQWM == pSWWdqJsttaHfdExjkxSGXIJpuUQWM ) pSWWdqJsttaHfdExjkxSGXIJpuUQWM=229146728.356720814033294012255660792277; else pSWWdqJsttaHfdExjkxSGXIJpuUQWM=619326303.413953546015504681439089879714;if (pSWWdqJsttaHfdExjkxSGXIJpuUQWM == pSWWdqJsttaHfdExjkxSGXIJpuUQWM ) pSWWdqJsttaHfdExjkxSGXIJpuUQWM=2070181653.974763145267734027622194953418; else pSWWdqJsttaHfdExjkxSGXIJpuUQWM=1183535223.583325194254728246586220398182;if (pSWWdqJsttaHfdExjkxSGXIJpuUQWM == pSWWdqJsttaHfdExjkxSGXIJpuUQWM ) pSWWdqJsttaHfdExjkxSGXIJpuUQWM=1010153127.073788452094347847671804337403; else pSWWdqJsttaHfdExjkxSGXIJpuUQWM=1902777298.811339215199569398465161128055;if (pSWWdqJsttaHfdExjkxSGXIJpuUQWM == pSWWdqJsttaHfdExjkxSGXIJpuUQWM ) pSWWdqJsttaHfdExjkxSGXIJpuUQWM=280946387.115939312864369345514005975131; else pSWWdqJsttaHfdExjkxSGXIJpuUQWM=713911726.237009473109660291299354221106;if (pSWWdqJsttaHfdExjkxSGXIJpuUQWM == pSWWdqJsttaHfdExjkxSGXIJpuUQWM ) pSWWdqJsttaHfdExjkxSGXIJpuUQWM=2056032864.438274836198251545579594298399; else pSWWdqJsttaHfdExjkxSGXIJpuUQWM=260267150.847878159657520220478763704509;long ZRlOZFUpPVwbwNMyjUClquOAGpQDnV=774969894;if (ZRlOZFUpPVwbwNMyjUClquOAGpQDnV == ZRlOZFUpPVwbwNMyjUClquOAGpQDnV- 0 ) ZRlOZFUpPVwbwNMyjUClquOAGpQDnV=1237710082; else ZRlOZFUpPVwbwNMyjUClquOAGpQDnV=1305220574;if (ZRlOZFUpPVwbwNMyjUClquOAGpQDnV == ZRlOZFUpPVwbwNMyjUClquOAGpQDnV- 0 ) ZRlOZFUpPVwbwNMyjUClquOAGpQDnV=241142041; else ZRlOZFUpPVwbwNMyjUClquOAGpQDnV=126857116;if (ZRlOZFUpPVwbwNMyjUClquOAGpQDnV == ZRlOZFUpPVwbwNMyjUClquOAGpQDnV- 0 ) ZRlOZFUpPVwbwNMyjUClquOAGpQDnV=795693446; else ZRlOZFUpPVwbwNMyjUClquOAGpQDnV=1716628046;if (ZRlOZFUpPVwbwNMyjUClquOAGpQDnV == ZRlOZFUpPVwbwNMyjUClquOAGpQDnV- 1 ) ZRlOZFUpPVwbwNMyjUClquOAGpQDnV=1847191905; else ZRlOZFUpPVwbwNMyjUClquOAGpQDnV=886708033;if (ZRlOZFUpPVwbwNMyjUClquOAGpQDnV == ZRlOZFUpPVwbwNMyjUClquOAGpQDnV- 0 ) ZRlOZFUpPVwbwNMyjUClquOAGpQDnV=209334096; else ZRlOZFUpPVwbwNMyjUClquOAGpQDnV=1274338307;if (ZRlOZFUpPVwbwNMyjUClquOAGpQDnV == ZRlOZFUpPVwbwNMyjUClquOAGpQDnV- 0 ) ZRlOZFUpPVwbwNMyjUClquOAGpQDnV=1104081025; else ZRlOZFUpPVwbwNMyjUClquOAGpQDnV=2112575205;long vhsOpGnnlITLMNvPJzfyWCcURCKhIG=2057287324;if (vhsOpGnnlITLMNvPJzfyWCcURCKhIG == vhsOpGnnlITLMNvPJzfyWCcURCKhIG- 1 ) vhsOpGnnlITLMNvPJzfyWCcURCKhIG=1843382750; else vhsOpGnnlITLMNvPJzfyWCcURCKhIG=2044125897;if (vhsOpGnnlITLMNvPJzfyWCcURCKhIG == vhsOpGnnlITLMNvPJzfyWCcURCKhIG- 0 ) vhsOpGnnlITLMNvPJzfyWCcURCKhIG=1280589246; else vhsOpGnnlITLMNvPJzfyWCcURCKhIG=277680952;if (vhsOpGnnlITLMNvPJzfyWCcURCKhIG == vhsOpGnnlITLMNvPJzfyWCcURCKhIG- 1 ) vhsOpGnnlITLMNvPJzfyWCcURCKhIG=923877344; else vhsOpGnnlITLMNvPJzfyWCcURCKhIG=1557581401;if (vhsOpGnnlITLMNvPJzfyWCcURCKhIG == vhsOpGnnlITLMNvPJzfyWCcURCKhIG- 1 ) vhsOpGnnlITLMNvPJzfyWCcURCKhIG=1084289337; else vhsOpGnnlITLMNvPJzfyWCcURCKhIG=303201954;if (vhsOpGnnlITLMNvPJzfyWCcURCKhIG == vhsOpGnnlITLMNvPJzfyWCcURCKhIG- 0 ) vhsOpGnnlITLMNvPJzfyWCcURCKhIG=1132608610; else vhsOpGnnlITLMNvPJzfyWCcURCKhIG=1456818298;if (vhsOpGnnlITLMNvPJzfyWCcURCKhIG == vhsOpGnnlITLMNvPJzfyWCcURCKhIG- 0 ) vhsOpGnnlITLMNvPJzfyWCcURCKhIG=964854215; else vhsOpGnnlITLMNvPJzfyWCcURCKhIG=583770419;double OiIunDBfVorJwyLOmrSFdiXsREyTGX=396799924.859007049871108776895756414965;if (OiIunDBfVorJwyLOmrSFdiXsREyTGX == OiIunDBfVorJwyLOmrSFdiXsREyTGX ) OiIunDBfVorJwyLOmrSFdiXsREyTGX=1221112376.312705933281694661599148620469; else OiIunDBfVorJwyLOmrSFdiXsREyTGX=696397497.328556950495462426296448637419;if (OiIunDBfVorJwyLOmrSFdiXsREyTGX == OiIunDBfVorJwyLOmrSFdiXsREyTGX ) OiIunDBfVorJwyLOmrSFdiXsREyTGX=732450329.568406325872220589720527409192; else OiIunDBfVorJwyLOmrSFdiXsREyTGX=1972261037.964194706610243422826989914111;if (OiIunDBfVorJwyLOmrSFdiXsREyTGX == OiIunDBfVorJwyLOmrSFdiXsREyTGX ) OiIunDBfVorJwyLOmrSFdiXsREyTGX=1695185985.833392814788321138408747718528; else OiIunDBfVorJwyLOmrSFdiXsREyTGX=1133623525.170689895261925877898671429769;if (OiIunDBfVorJwyLOmrSFdiXsREyTGX == OiIunDBfVorJwyLOmrSFdiXsREyTGX ) OiIunDBfVorJwyLOmrSFdiXsREyTGX=1189808286.440661974335334793295274150937; else OiIunDBfVorJwyLOmrSFdiXsREyTGX=146648443.123783236400366435873757537662;if (OiIunDBfVorJwyLOmrSFdiXsREyTGX == OiIunDBfVorJwyLOmrSFdiXsREyTGX ) OiIunDBfVorJwyLOmrSFdiXsREyTGX=874616955.215832276156890691882616447735; else OiIunDBfVorJwyLOmrSFdiXsREyTGX=1489922342.678430564527973807805798070049;if (OiIunDBfVorJwyLOmrSFdiXsREyTGX == OiIunDBfVorJwyLOmrSFdiXsREyTGX ) OiIunDBfVorJwyLOmrSFdiXsREyTGX=409992207.565169846113846674375194108056; else OiIunDBfVorJwyLOmrSFdiXsREyTGX=795843346.063607382506179396340677547216;float pvACazJUwSwKsmsDKWqDKnBfVudFby=662011877.237076127266891119895232299364f;if (pvACazJUwSwKsmsDKWqDKnBfVudFby - pvACazJUwSwKsmsDKWqDKnBfVudFby> 0.00000001 ) pvACazJUwSwKsmsDKWqDKnBfVudFby=1810030521.421277419904116394228287800699f; else pvACazJUwSwKsmsDKWqDKnBfVudFby=2146229187.797485573580190123690624734547f;if (pvACazJUwSwKsmsDKWqDKnBfVudFby - pvACazJUwSwKsmsDKWqDKnBfVudFby> 0.00000001 ) pvACazJUwSwKsmsDKWqDKnBfVudFby=564368047.783131773282284810779700928486f; else pvACazJUwSwKsmsDKWqDKnBfVudFby=633379356.214680817955152944192587909331f;if (pvACazJUwSwKsmsDKWqDKnBfVudFby - pvACazJUwSwKsmsDKWqDKnBfVudFby> 0.00000001 ) pvACazJUwSwKsmsDKWqDKnBfVudFby=898533899.440717960401393576908439483788f; else pvACazJUwSwKsmsDKWqDKnBfVudFby=583351675.269751545519579220444353311143f;if (pvACazJUwSwKsmsDKWqDKnBfVudFby - pvACazJUwSwKsmsDKWqDKnBfVudFby> 0.00000001 ) pvACazJUwSwKsmsDKWqDKnBfVudFby=1998576197.529335364304240007136444501645f; else pvACazJUwSwKsmsDKWqDKnBfVudFby=1827612328.041210035497624111342511829683f;if (pvACazJUwSwKsmsDKWqDKnBfVudFby - pvACazJUwSwKsmsDKWqDKnBfVudFby> 0.00000001 ) pvACazJUwSwKsmsDKWqDKnBfVudFby=220931237.572036314117375082049041364040f; else pvACazJUwSwKsmsDKWqDKnBfVudFby=2085423734.155202579981833535794765554278f;if (pvACazJUwSwKsmsDKWqDKnBfVudFby - pvACazJUwSwKsmsDKWqDKnBfVudFby> 0.00000001 ) pvACazJUwSwKsmsDKWqDKnBfVudFby=1141732153.347507317361327831894036659186f; else pvACazJUwSwKsmsDKWqDKnBfVudFby=1486358795.930264543722853445873349441105f;int NQGVMEFeOSvqEiHKsPKGVNERcvIoMP=297930032;if (NQGVMEFeOSvqEiHKsPKGVNERcvIoMP == NQGVMEFeOSvqEiHKsPKGVNERcvIoMP- 1 ) NQGVMEFeOSvqEiHKsPKGVNERcvIoMP=1452696224; else NQGVMEFeOSvqEiHKsPKGVNERcvIoMP=707281101;if (NQGVMEFeOSvqEiHKsPKGVNERcvIoMP == NQGVMEFeOSvqEiHKsPKGVNERcvIoMP- 1 ) NQGVMEFeOSvqEiHKsPKGVNERcvIoMP=891462829; else NQGVMEFeOSvqEiHKsPKGVNERcvIoMP=1615244150;if (NQGVMEFeOSvqEiHKsPKGVNERcvIoMP == NQGVMEFeOSvqEiHKsPKGVNERcvIoMP- 0 ) NQGVMEFeOSvqEiHKsPKGVNERcvIoMP=1033967828; else NQGVMEFeOSvqEiHKsPKGVNERcvIoMP=1226587462;if (NQGVMEFeOSvqEiHKsPKGVNERcvIoMP == NQGVMEFeOSvqEiHKsPKGVNERcvIoMP- 1 ) NQGVMEFeOSvqEiHKsPKGVNERcvIoMP=362101339; else NQGVMEFeOSvqEiHKsPKGVNERcvIoMP=709759225;if (NQGVMEFeOSvqEiHKsPKGVNERcvIoMP == NQGVMEFeOSvqEiHKsPKGVNERcvIoMP- 0 ) NQGVMEFeOSvqEiHKsPKGVNERcvIoMP=854317611; else NQGVMEFeOSvqEiHKsPKGVNERcvIoMP=1133940803;if (NQGVMEFeOSvqEiHKsPKGVNERcvIoMP == NQGVMEFeOSvqEiHKsPKGVNERcvIoMP- 0 ) NQGVMEFeOSvqEiHKsPKGVNERcvIoMP=1550547644; else NQGVMEFeOSvqEiHKsPKGVNERcvIoMP=1581438649;double LiNuHtfTpGFdViMSUodqNUJKAIrtNw=87547041.305114073608974568325330489899;if (LiNuHtfTpGFdViMSUodqNUJKAIrtNw == LiNuHtfTpGFdViMSUodqNUJKAIrtNw ) LiNuHtfTpGFdViMSUodqNUJKAIrtNw=1865682647.422508776763620724418730408574; else LiNuHtfTpGFdViMSUodqNUJKAIrtNw=302842338.629539747699276381177548180221;if (LiNuHtfTpGFdViMSUodqNUJKAIrtNw == LiNuHtfTpGFdViMSUodqNUJKAIrtNw ) LiNuHtfTpGFdViMSUodqNUJKAIrtNw=921223545.130324510218893057422741829156; else LiNuHtfTpGFdViMSUodqNUJKAIrtNw=2053684926.052757905387206986623481959060;if (LiNuHtfTpGFdViMSUodqNUJKAIrtNw == LiNuHtfTpGFdViMSUodqNUJKAIrtNw ) LiNuHtfTpGFdViMSUodqNUJKAIrtNw=1494519664.647437614715224655595363367291; else LiNuHtfTpGFdViMSUodqNUJKAIrtNw=1335746374.805675197047201837840457639417;if (LiNuHtfTpGFdViMSUodqNUJKAIrtNw == LiNuHtfTpGFdViMSUodqNUJKAIrtNw ) LiNuHtfTpGFdViMSUodqNUJKAIrtNw=1645749672.053392176957856827354156245905; else LiNuHtfTpGFdViMSUodqNUJKAIrtNw=637760701.778109501087199471192227300725;if (LiNuHtfTpGFdViMSUodqNUJKAIrtNw == LiNuHtfTpGFdViMSUodqNUJKAIrtNw ) LiNuHtfTpGFdViMSUodqNUJKAIrtNw=845304947.837239985733638806788481356281; else LiNuHtfTpGFdViMSUodqNUJKAIrtNw=1052321648.988636006150657079060514845799;if (LiNuHtfTpGFdViMSUodqNUJKAIrtNw == LiNuHtfTpGFdViMSUodqNUJKAIrtNw ) LiNuHtfTpGFdViMSUodqNUJKAIrtNw=1751329800.480585501291173994548733158590; else LiNuHtfTpGFdViMSUodqNUJKAIrtNw=1757245898.132778017257988054195225186854;float BGHFbvQzbtplNwHqYbFCTSxYvFigxN=1758679116.829112757638982300848732564080f;if (BGHFbvQzbtplNwHqYbFCTSxYvFigxN - BGHFbvQzbtplNwHqYbFCTSxYvFigxN> 0.00000001 ) BGHFbvQzbtplNwHqYbFCTSxYvFigxN=508161128.590290777738429846377986729775f; else BGHFbvQzbtplNwHqYbFCTSxYvFigxN=112062175.887825970364422132372982361458f;if (BGHFbvQzbtplNwHqYbFCTSxYvFigxN - BGHFbvQzbtplNwHqYbFCTSxYvFigxN> 0.00000001 ) BGHFbvQzbtplNwHqYbFCTSxYvFigxN=1490630769.100295170507701457579851301656f; else BGHFbvQzbtplNwHqYbFCTSxYvFigxN=1519656744.452227423806297538101509936900f;if (BGHFbvQzbtplNwHqYbFCTSxYvFigxN - BGHFbvQzbtplNwHqYbFCTSxYvFigxN> 0.00000001 ) BGHFbvQzbtplNwHqYbFCTSxYvFigxN=153598740.563525658503487477587455596440f; else BGHFbvQzbtplNwHqYbFCTSxYvFigxN=123375148.253435477209071823267052311803f;if (BGHFbvQzbtplNwHqYbFCTSxYvFigxN - BGHFbvQzbtplNwHqYbFCTSxYvFigxN> 0.00000001 ) BGHFbvQzbtplNwHqYbFCTSxYvFigxN=1775527696.021935105300039849578429331223f; else BGHFbvQzbtplNwHqYbFCTSxYvFigxN=583443059.478538915683806803458334671371f;if (BGHFbvQzbtplNwHqYbFCTSxYvFigxN - BGHFbvQzbtplNwHqYbFCTSxYvFigxN> 0.00000001 ) BGHFbvQzbtplNwHqYbFCTSxYvFigxN=1784127848.599303361103509662353897525406f; else BGHFbvQzbtplNwHqYbFCTSxYvFigxN=2002092469.865520561465336584511903870288f;if (BGHFbvQzbtplNwHqYbFCTSxYvFigxN - BGHFbvQzbtplNwHqYbFCTSxYvFigxN> 0.00000001 ) BGHFbvQzbtplNwHqYbFCTSxYvFigxN=1926545392.997534919011209966204200697375f; else BGHFbvQzbtplNwHqYbFCTSxYvFigxN=1912076743.038358991278560180492518589795f;float dSSxrLvbFymczOffWyYHivpReJzriu=2079968893.613012433893085070561088740996f;if (dSSxrLvbFymczOffWyYHivpReJzriu - dSSxrLvbFymczOffWyYHivpReJzriu> 0.00000001 ) dSSxrLvbFymczOffWyYHivpReJzriu=1861675978.263518673790147007026809263537f; else dSSxrLvbFymczOffWyYHivpReJzriu=1054185112.355873067740788295184497157535f;if (dSSxrLvbFymczOffWyYHivpReJzriu - dSSxrLvbFymczOffWyYHivpReJzriu> 0.00000001 ) dSSxrLvbFymczOffWyYHivpReJzriu=1768159795.625654547405039613117946407972f; else dSSxrLvbFymczOffWyYHivpReJzriu=448931493.763323623576879046955443978936f;if (dSSxrLvbFymczOffWyYHivpReJzriu - dSSxrLvbFymczOffWyYHivpReJzriu> 0.00000001 ) dSSxrLvbFymczOffWyYHivpReJzriu=342197941.777684382710064117746212833055f; else dSSxrLvbFymczOffWyYHivpReJzriu=1075559935.579881930587435558987239066585f;if (dSSxrLvbFymczOffWyYHivpReJzriu - dSSxrLvbFymczOffWyYHivpReJzriu> 0.00000001 ) dSSxrLvbFymczOffWyYHivpReJzriu=950894015.234725129429827988839527935083f; else dSSxrLvbFymczOffWyYHivpReJzriu=116949850.556545471053066851229985771790f;if (dSSxrLvbFymczOffWyYHivpReJzriu - dSSxrLvbFymczOffWyYHivpReJzriu> 0.00000001 ) dSSxrLvbFymczOffWyYHivpReJzriu=1209015076.115698439351889657219487477197f; else dSSxrLvbFymczOffWyYHivpReJzriu=253702128.489204947852207910613568174833f;if (dSSxrLvbFymczOffWyYHivpReJzriu - dSSxrLvbFymczOffWyYHivpReJzriu> 0.00000001 ) dSSxrLvbFymczOffWyYHivpReJzriu=769939203.134298279682047839166869502114f; else dSSxrLvbFymczOffWyYHivpReJzriu=2129129827.112582902975213153244588748689f;int HcYakzbgasNiIrJeYbSZxuowcPVrSS=1150868120;if (HcYakzbgasNiIrJeYbSZxuowcPVrSS == HcYakzbgasNiIrJeYbSZxuowcPVrSS- 1 ) HcYakzbgasNiIrJeYbSZxuowcPVrSS=1739066191; else HcYakzbgasNiIrJeYbSZxuowcPVrSS=1939352175;if (HcYakzbgasNiIrJeYbSZxuowcPVrSS == HcYakzbgasNiIrJeYbSZxuowcPVrSS- 1 ) HcYakzbgasNiIrJeYbSZxuowcPVrSS=1073277521; else HcYakzbgasNiIrJeYbSZxuowcPVrSS=1035527019;if (HcYakzbgasNiIrJeYbSZxuowcPVrSS == HcYakzbgasNiIrJeYbSZxuowcPVrSS- 0 ) HcYakzbgasNiIrJeYbSZxuowcPVrSS=945298869; else HcYakzbgasNiIrJeYbSZxuowcPVrSS=851574659;if (HcYakzbgasNiIrJeYbSZxuowcPVrSS == HcYakzbgasNiIrJeYbSZxuowcPVrSS- 0 ) HcYakzbgasNiIrJeYbSZxuowcPVrSS=2081431948; else HcYakzbgasNiIrJeYbSZxuowcPVrSS=1742307499;if (HcYakzbgasNiIrJeYbSZxuowcPVrSS == HcYakzbgasNiIrJeYbSZxuowcPVrSS- 1 ) HcYakzbgasNiIrJeYbSZxuowcPVrSS=674798289; else HcYakzbgasNiIrJeYbSZxuowcPVrSS=337780486;if (HcYakzbgasNiIrJeYbSZxuowcPVrSS == HcYakzbgasNiIrJeYbSZxuowcPVrSS- 0 ) HcYakzbgasNiIrJeYbSZxuowcPVrSS=269557078; else HcYakzbgasNiIrJeYbSZxuowcPVrSS=1639910842;float dUOmVAlyoRcdrMjqwCHwoSHlIfaOgu=428167399.477126782117950527755564459024f;if (dUOmVAlyoRcdrMjqwCHwoSHlIfaOgu - dUOmVAlyoRcdrMjqwCHwoSHlIfaOgu> 0.00000001 ) dUOmVAlyoRcdrMjqwCHwoSHlIfaOgu=372298514.604574153262887745581363392387f; else dUOmVAlyoRcdrMjqwCHwoSHlIfaOgu=1818844554.786861448657469479772555345238f;if (dUOmVAlyoRcdrMjqwCHwoSHlIfaOgu - dUOmVAlyoRcdrMjqwCHwoSHlIfaOgu> 0.00000001 ) dUOmVAlyoRcdrMjqwCHwoSHlIfaOgu=2043053567.388769602640794590493471623263f; else dUOmVAlyoRcdrMjqwCHwoSHlIfaOgu=1085189907.732048080087714286195584601724f;if (dUOmVAlyoRcdrMjqwCHwoSHlIfaOgu - dUOmVAlyoRcdrMjqwCHwoSHlIfaOgu> 0.00000001 ) dUOmVAlyoRcdrMjqwCHwoSHlIfaOgu=82773343.755001986752052677844718826855f; else dUOmVAlyoRcdrMjqwCHwoSHlIfaOgu=1093426766.737743371341154454303598015729f;if (dUOmVAlyoRcdrMjqwCHwoSHlIfaOgu - dUOmVAlyoRcdrMjqwCHwoSHlIfaOgu> 0.00000001 ) dUOmVAlyoRcdrMjqwCHwoSHlIfaOgu=1416481175.613112542940651367457265325073f; else dUOmVAlyoRcdrMjqwCHwoSHlIfaOgu=1580084840.966283915292357395786948346989f;if (dUOmVAlyoRcdrMjqwCHwoSHlIfaOgu - dUOmVAlyoRcdrMjqwCHwoSHlIfaOgu> 0.00000001 ) dUOmVAlyoRcdrMjqwCHwoSHlIfaOgu=1308651807.868918548855336501804130813805f; else dUOmVAlyoRcdrMjqwCHwoSHlIfaOgu=1373680919.803744545096803171384519583811f;if (dUOmVAlyoRcdrMjqwCHwoSHlIfaOgu - dUOmVAlyoRcdrMjqwCHwoSHlIfaOgu> 0.00000001 ) dUOmVAlyoRcdrMjqwCHwoSHlIfaOgu=310641668.685316269216792834884262149527f; else dUOmVAlyoRcdrMjqwCHwoSHlIfaOgu=2047380259.174552259891318883885014742337f;double lZNGWPNUzhtXAsTxYYQcglTGFewoFw=1031788760.267090593539481448582343728249;if (lZNGWPNUzhtXAsTxYYQcglTGFewoFw == lZNGWPNUzhtXAsTxYYQcglTGFewoFw ) lZNGWPNUzhtXAsTxYYQcglTGFewoFw=1284224616.880613725010784865527677193537; else lZNGWPNUzhtXAsTxYYQcglTGFewoFw=775677272.235575088983358277006383222452;if (lZNGWPNUzhtXAsTxYYQcglTGFewoFw == lZNGWPNUzhtXAsTxYYQcglTGFewoFw ) lZNGWPNUzhtXAsTxYYQcglTGFewoFw=173010552.130988621999875543902664106336; else lZNGWPNUzhtXAsTxYYQcglTGFewoFw=1388419150.464696357175611851979955058789;if (lZNGWPNUzhtXAsTxYYQcglTGFewoFw == lZNGWPNUzhtXAsTxYYQcglTGFewoFw ) lZNGWPNUzhtXAsTxYYQcglTGFewoFw=1491343674.230653529930784715345556862964; else lZNGWPNUzhtXAsTxYYQcglTGFewoFw=1791979762.978894606844305937244344479490;if (lZNGWPNUzhtXAsTxYYQcglTGFewoFw == lZNGWPNUzhtXAsTxYYQcglTGFewoFw ) lZNGWPNUzhtXAsTxYYQcglTGFewoFw=1134670024.267517390021951236194583163453; else lZNGWPNUzhtXAsTxYYQcglTGFewoFw=1010101319.622778789974832350435331779456;if (lZNGWPNUzhtXAsTxYYQcglTGFewoFw == lZNGWPNUzhtXAsTxYYQcglTGFewoFw ) lZNGWPNUzhtXAsTxYYQcglTGFewoFw=1120236811.495740028355732076182402949110; else lZNGWPNUzhtXAsTxYYQcglTGFewoFw=25852296.934358029343218061070447510391;if (lZNGWPNUzhtXAsTxYYQcglTGFewoFw == lZNGWPNUzhtXAsTxYYQcglTGFewoFw ) lZNGWPNUzhtXAsTxYYQcglTGFewoFw=1921616851.192952158406795929598924060895; else lZNGWPNUzhtXAsTxYYQcglTGFewoFw=4417455.983925247692368586269190437890;int RdQYLQrhBnsNtAadegeqWcPdqwRYxm=1475342147;if (RdQYLQrhBnsNtAadegeqWcPdqwRYxm == RdQYLQrhBnsNtAadegeqWcPdqwRYxm- 1 ) RdQYLQrhBnsNtAadegeqWcPdqwRYxm=796229105; else RdQYLQrhBnsNtAadegeqWcPdqwRYxm=341771804;if (RdQYLQrhBnsNtAadegeqWcPdqwRYxm == RdQYLQrhBnsNtAadegeqWcPdqwRYxm- 0 ) RdQYLQrhBnsNtAadegeqWcPdqwRYxm=1756831313; else RdQYLQrhBnsNtAadegeqWcPdqwRYxm=1951380402;if (RdQYLQrhBnsNtAadegeqWcPdqwRYxm == RdQYLQrhBnsNtAadegeqWcPdqwRYxm- 0 ) RdQYLQrhBnsNtAadegeqWcPdqwRYxm=856769168; else RdQYLQrhBnsNtAadegeqWcPdqwRYxm=1780161593;if (RdQYLQrhBnsNtAadegeqWcPdqwRYxm == RdQYLQrhBnsNtAadegeqWcPdqwRYxm- 0 ) RdQYLQrhBnsNtAadegeqWcPdqwRYxm=1068094777; else RdQYLQrhBnsNtAadegeqWcPdqwRYxm=645639425;if (RdQYLQrhBnsNtAadegeqWcPdqwRYxm == RdQYLQrhBnsNtAadegeqWcPdqwRYxm- 1 ) RdQYLQrhBnsNtAadegeqWcPdqwRYxm=522871000; else RdQYLQrhBnsNtAadegeqWcPdqwRYxm=1640676328;if (RdQYLQrhBnsNtAadegeqWcPdqwRYxm == RdQYLQrhBnsNtAadegeqWcPdqwRYxm- 0 ) RdQYLQrhBnsNtAadegeqWcPdqwRYxm=1874358859; else RdQYLQrhBnsNtAadegeqWcPdqwRYxm=1719101597;long wmDMznmdSrwVMZQaDeAUkrvSOOlZQm=1028400420;if (wmDMznmdSrwVMZQaDeAUkrvSOOlZQm == wmDMznmdSrwVMZQaDeAUkrvSOOlZQm- 1 ) wmDMznmdSrwVMZQaDeAUkrvSOOlZQm=1377078736; else wmDMznmdSrwVMZQaDeAUkrvSOOlZQm=1385251965;if (wmDMznmdSrwVMZQaDeAUkrvSOOlZQm == wmDMznmdSrwVMZQaDeAUkrvSOOlZQm- 0 ) wmDMznmdSrwVMZQaDeAUkrvSOOlZQm=355686226; else wmDMznmdSrwVMZQaDeAUkrvSOOlZQm=781007357;if (wmDMznmdSrwVMZQaDeAUkrvSOOlZQm == wmDMznmdSrwVMZQaDeAUkrvSOOlZQm- 0 ) wmDMznmdSrwVMZQaDeAUkrvSOOlZQm=1708146024; else wmDMznmdSrwVMZQaDeAUkrvSOOlZQm=216537520;if (wmDMznmdSrwVMZQaDeAUkrvSOOlZQm == wmDMznmdSrwVMZQaDeAUkrvSOOlZQm- 1 ) wmDMznmdSrwVMZQaDeAUkrvSOOlZQm=1084307372; else wmDMznmdSrwVMZQaDeAUkrvSOOlZQm=727802399;if (wmDMznmdSrwVMZQaDeAUkrvSOOlZQm == wmDMznmdSrwVMZQaDeAUkrvSOOlZQm- 1 ) wmDMznmdSrwVMZQaDeAUkrvSOOlZQm=684174530; else wmDMznmdSrwVMZQaDeAUkrvSOOlZQm=1032952248;if (wmDMznmdSrwVMZQaDeAUkrvSOOlZQm == wmDMznmdSrwVMZQaDeAUkrvSOOlZQm- 1 ) wmDMznmdSrwVMZQaDeAUkrvSOOlZQm=2112436743; else wmDMznmdSrwVMZQaDeAUkrvSOOlZQm=56921047;double uMlaJGBkeSlNRAizBDlIYjRxwlSrjR=1997668634.598662829105546649906423132297;if (uMlaJGBkeSlNRAizBDlIYjRxwlSrjR == uMlaJGBkeSlNRAizBDlIYjRxwlSrjR ) uMlaJGBkeSlNRAizBDlIYjRxwlSrjR=665932051.432942259334516901485358318564; else uMlaJGBkeSlNRAizBDlIYjRxwlSrjR=982783168.173288461440077201723282318473;if (uMlaJGBkeSlNRAizBDlIYjRxwlSrjR == uMlaJGBkeSlNRAizBDlIYjRxwlSrjR ) uMlaJGBkeSlNRAizBDlIYjRxwlSrjR=1251151154.570537234965014639111794200107; else uMlaJGBkeSlNRAizBDlIYjRxwlSrjR=1159147788.010907054118896927979036289644;if (uMlaJGBkeSlNRAizBDlIYjRxwlSrjR == uMlaJGBkeSlNRAizBDlIYjRxwlSrjR ) uMlaJGBkeSlNRAizBDlIYjRxwlSrjR=37050203.714806521768511400848333908472; else uMlaJGBkeSlNRAizBDlIYjRxwlSrjR=1304288642.852140400141722163699081658290;if (uMlaJGBkeSlNRAizBDlIYjRxwlSrjR == uMlaJGBkeSlNRAizBDlIYjRxwlSrjR ) uMlaJGBkeSlNRAizBDlIYjRxwlSrjR=175019592.815092757100080862185699497150; else uMlaJGBkeSlNRAizBDlIYjRxwlSrjR=1651546670.331033683152659901199712633761;if (uMlaJGBkeSlNRAizBDlIYjRxwlSrjR == uMlaJGBkeSlNRAizBDlIYjRxwlSrjR ) uMlaJGBkeSlNRAizBDlIYjRxwlSrjR=1855066142.560898915829914404059717163050; else uMlaJGBkeSlNRAizBDlIYjRxwlSrjR=1674854774.102369297210647190824972286373;if (uMlaJGBkeSlNRAizBDlIYjRxwlSrjR == uMlaJGBkeSlNRAizBDlIYjRxwlSrjR ) uMlaJGBkeSlNRAizBDlIYjRxwlSrjR=865548150.506673866622894927118323734028; else uMlaJGBkeSlNRAizBDlIYjRxwlSrjR=1998150993.302472518443531328837937638394;float lWlLNyQJOYHwimXnuLWGbLtHUiYrVU=1086185122.830710705707366363395179374940f;if (lWlLNyQJOYHwimXnuLWGbLtHUiYrVU - lWlLNyQJOYHwimXnuLWGbLtHUiYrVU> 0.00000001 ) lWlLNyQJOYHwimXnuLWGbLtHUiYrVU=1931948036.911276015927227025433263887154f; else lWlLNyQJOYHwimXnuLWGbLtHUiYrVU=893216431.796589610145947805336061786959f;if (lWlLNyQJOYHwimXnuLWGbLtHUiYrVU - lWlLNyQJOYHwimXnuLWGbLtHUiYrVU> 0.00000001 ) lWlLNyQJOYHwimXnuLWGbLtHUiYrVU=774822366.997134174851530942608292973680f; else lWlLNyQJOYHwimXnuLWGbLtHUiYrVU=1568234539.487126882474259553295545280536f;if (lWlLNyQJOYHwimXnuLWGbLtHUiYrVU - lWlLNyQJOYHwimXnuLWGbLtHUiYrVU> 0.00000001 ) lWlLNyQJOYHwimXnuLWGbLtHUiYrVU=1102780594.169458552516033504443140604204f; else lWlLNyQJOYHwimXnuLWGbLtHUiYrVU=2074225158.645632581201188952279720413995f;if (lWlLNyQJOYHwimXnuLWGbLtHUiYrVU - lWlLNyQJOYHwimXnuLWGbLtHUiYrVU> 0.00000001 ) lWlLNyQJOYHwimXnuLWGbLtHUiYrVU=1914679770.445908240213334306276314693632f; else lWlLNyQJOYHwimXnuLWGbLtHUiYrVU=579691585.960787639181207649611782023231f;if (lWlLNyQJOYHwimXnuLWGbLtHUiYrVU - lWlLNyQJOYHwimXnuLWGbLtHUiYrVU> 0.00000001 ) lWlLNyQJOYHwimXnuLWGbLtHUiYrVU=1404948079.662945375777820511705413129877f; else lWlLNyQJOYHwimXnuLWGbLtHUiYrVU=1029028914.551852047338495506351364636982f;if (lWlLNyQJOYHwimXnuLWGbLtHUiYrVU - lWlLNyQJOYHwimXnuLWGbLtHUiYrVU> 0.00000001 ) lWlLNyQJOYHwimXnuLWGbLtHUiYrVU=1267205872.661933707868457802873452620833f; else lWlLNyQJOYHwimXnuLWGbLtHUiYrVU=1971300567.716678867281960492793512931931f;double wlWDbHonvBWXgtLYvOKNqERBKxYkUj=1258290927.281567013448338560169522253505;if (wlWDbHonvBWXgtLYvOKNqERBKxYkUj == wlWDbHonvBWXgtLYvOKNqERBKxYkUj ) wlWDbHonvBWXgtLYvOKNqERBKxYkUj=866594345.685383366679051791566264786031; else wlWDbHonvBWXgtLYvOKNqERBKxYkUj=2024173872.462897315531560058167304042161;if (wlWDbHonvBWXgtLYvOKNqERBKxYkUj == wlWDbHonvBWXgtLYvOKNqERBKxYkUj ) wlWDbHonvBWXgtLYvOKNqERBKxYkUj=1053169546.843522837559604581583998526895; else wlWDbHonvBWXgtLYvOKNqERBKxYkUj=1705633470.525480904823271880157278905511;if (wlWDbHonvBWXgtLYvOKNqERBKxYkUj == wlWDbHonvBWXgtLYvOKNqERBKxYkUj ) wlWDbHonvBWXgtLYvOKNqERBKxYkUj=930384182.714203423579776173774684845769; else wlWDbHonvBWXgtLYvOKNqERBKxYkUj=1229723766.549422907857869961383285713047;if (wlWDbHonvBWXgtLYvOKNqERBKxYkUj == wlWDbHonvBWXgtLYvOKNqERBKxYkUj ) wlWDbHonvBWXgtLYvOKNqERBKxYkUj=991350217.876403035528792882609558747582; else wlWDbHonvBWXgtLYvOKNqERBKxYkUj=72790094.906019147693687569303245942887;if (wlWDbHonvBWXgtLYvOKNqERBKxYkUj == wlWDbHonvBWXgtLYvOKNqERBKxYkUj ) wlWDbHonvBWXgtLYvOKNqERBKxYkUj=1210232373.766473279385512698228829976227; else wlWDbHonvBWXgtLYvOKNqERBKxYkUj=851623203.837194832440003974345576994760;if (wlWDbHonvBWXgtLYvOKNqERBKxYkUj == wlWDbHonvBWXgtLYvOKNqERBKxYkUj ) wlWDbHonvBWXgtLYvOKNqERBKxYkUj=1073424862.806365952887689993701399336121; else wlWDbHonvBWXgtLYvOKNqERBKxYkUj=33291250.117374484812266890811216962804;long qNJmnzQDbXCrsysEROaUhNIvuxJSIL=1268606103;if (qNJmnzQDbXCrsysEROaUhNIvuxJSIL == qNJmnzQDbXCrsysEROaUhNIvuxJSIL- 1 ) qNJmnzQDbXCrsysEROaUhNIvuxJSIL=563783920; else qNJmnzQDbXCrsysEROaUhNIvuxJSIL=1026251621;if (qNJmnzQDbXCrsysEROaUhNIvuxJSIL == qNJmnzQDbXCrsysEROaUhNIvuxJSIL- 1 ) qNJmnzQDbXCrsysEROaUhNIvuxJSIL=272456658; else qNJmnzQDbXCrsysEROaUhNIvuxJSIL=426077131;if (qNJmnzQDbXCrsysEROaUhNIvuxJSIL == qNJmnzQDbXCrsysEROaUhNIvuxJSIL- 1 ) qNJmnzQDbXCrsysEROaUhNIvuxJSIL=637241354; else qNJmnzQDbXCrsysEROaUhNIvuxJSIL=1353331091;if (qNJmnzQDbXCrsysEROaUhNIvuxJSIL == qNJmnzQDbXCrsysEROaUhNIvuxJSIL- 1 ) qNJmnzQDbXCrsysEROaUhNIvuxJSIL=894888040; else qNJmnzQDbXCrsysEROaUhNIvuxJSIL=739190540;if (qNJmnzQDbXCrsysEROaUhNIvuxJSIL == qNJmnzQDbXCrsysEROaUhNIvuxJSIL- 0 ) qNJmnzQDbXCrsysEROaUhNIvuxJSIL=1215587045; else qNJmnzQDbXCrsysEROaUhNIvuxJSIL=671459569;if (qNJmnzQDbXCrsysEROaUhNIvuxJSIL == qNJmnzQDbXCrsysEROaUhNIvuxJSIL- 1 ) qNJmnzQDbXCrsysEROaUhNIvuxJSIL=1901683356; else qNJmnzQDbXCrsysEROaUhNIvuxJSIL=1155504936;int hEdSSeEYreTgnIsromeeTzAzsMEJEX=843892412;if (hEdSSeEYreTgnIsromeeTzAzsMEJEX == hEdSSeEYreTgnIsromeeTzAzsMEJEX- 1 ) hEdSSeEYreTgnIsromeeTzAzsMEJEX=201025264; else hEdSSeEYreTgnIsromeeTzAzsMEJEX=1754740870;if (hEdSSeEYreTgnIsromeeTzAzsMEJEX == hEdSSeEYreTgnIsromeeTzAzsMEJEX- 1 ) hEdSSeEYreTgnIsromeeTzAzsMEJEX=1131556648; else hEdSSeEYreTgnIsromeeTzAzsMEJEX=40930678;if (hEdSSeEYreTgnIsromeeTzAzsMEJEX == hEdSSeEYreTgnIsromeeTzAzsMEJEX- 1 ) hEdSSeEYreTgnIsromeeTzAzsMEJEX=509882528; else hEdSSeEYreTgnIsromeeTzAzsMEJEX=911018505;if (hEdSSeEYreTgnIsromeeTzAzsMEJEX == hEdSSeEYreTgnIsromeeTzAzsMEJEX- 1 ) hEdSSeEYreTgnIsromeeTzAzsMEJEX=155800940; else hEdSSeEYreTgnIsromeeTzAzsMEJEX=702224270;if (hEdSSeEYreTgnIsromeeTzAzsMEJEX == hEdSSeEYreTgnIsromeeTzAzsMEJEX- 1 ) hEdSSeEYreTgnIsromeeTzAzsMEJEX=2049349861; else hEdSSeEYreTgnIsromeeTzAzsMEJEX=702032272;if (hEdSSeEYreTgnIsromeeTzAzsMEJEX == hEdSSeEYreTgnIsromeeTzAzsMEJEX- 0 ) hEdSSeEYreTgnIsromeeTzAzsMEJEX=358002166; else hEdSSeEYreTgnIsromeeTzAzsMEJEX=251157391;int NVBRuxUIbIrMLzqExwnTazxGvfoqNq=171308053;if (NVBRuxUIbIrMLzqExwnTazxGvfoqNq == NVBRuxUIbIrMLzqExwnTazxGvfoqNq- 0 ) NVBRuxUIbIrMLzqExwnTazxGvfoqNq=218847418; else NVBRuxUIbIrMLzqExwnTazxGvfoqNq=2146083416;if (NVBRuxUIbIrMLzqExwnTazxGvfoqNq == NVBRuxUIbIrMLzqExwnTazxGvfoqNq- 0 ) NVBRuxUIbIrMLzqExwnTazxGvfoqNq=1407516647; else NVBRuxUIbIrMLzqExwnTazxGvfoqNq=788956876;if (NVBRuxUIbIrMLzqExwnTazxGvfoqNq == NVBRuxUIbIrMLzqExwnTazxGvfoqNq- 0 ) NVBRuxUIbIrMLzqExwnTazxGvfoqNq=738717072; else NVBRuxUIbIrMLzqExwnTazxGvfoqNq=1047752379;if (NVBRuxUIbIrMLzqExwnTazxGvfoqNq == NVBRuxUIbIrMLzqExwnTazxGvfoqNq- 0 ) NVBRuxUIbIrMLzqExwnTazxGvfoqNq=621049573; else NVBRuxUIbIrMLzqExwnTazxGvfoqNq=1660746901;if (NVBRuxUIbIrMLzqExwnTazxGvfoqNq == NVBRuxUIbIrMLzqExwnTazxGvfoqNq- 1 ) NVBRuxUIbIrMLzqExwnTazxGvfoqNq=1129285832; else NVBRuxUIbIrMLzqExwnTazxGvfoqNq=313979006;if (NVBRuxUIbIrMLzqExwnTazxGvfoqNq == NVBRuxUIbIrMLzqExwnTazxGvfoqNq- 0 ) NVBRuxUIbIrMLzqExwnTazxGvfoqNq=490046425; else NVBRuxUIbIrMLzqExwnTazxGvfoqNq=258924613; }
 NVBRuxUIbIrMLzqExwnTazxGvfoqNqy::NVBRuxUIbIrMLzqExwnTazxGvfoqNqy()
 { this->obRCVjsoFIFH("TUNcDjaJBGGUqCxvvGudDqujOYvWOIobRCVjsoFIFHj", true, 1475524057, 1983328928, 35707324); }
#pragma optimize("", off)
 // <delete/>


// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class rCqcEKHIqUGFrAwDTCRaoqPfVfHzVuy
 { 
public: bool TYFcrcwvAOZGfhnHyHoVvjKRAnKHYD; double TYFcrcwvAOZGfhnHyHoVvjKRAnKHYDrCqcEKHIqUGFrAwDTCRaoqPfVfHzVu; rCqcEKHIqUGFrAwDTCRaoqPfVfHzVuy(); void TOONylhLnfDn(string TYFcrcwvAOZGfhnHyHoVvjKRAnKHYDTOONylhLnfDn, bool HnvingmXFaWGQMPjjyhYIrgYqkNncE, int RoWxHlLwuGVaCOyoSyBQDMZAuPGXrC, float SyvAIikSFdYmDbCipRZCRPlLBFOVDr, long enFroXeBFQEAtXrQIyyLaoxplDMkzD);
 protected: bool TYFcrcwvAOZGfhnHyHoVvjKRAnKHYDo; double TYFcrcwvAOZGfhnHyHoVvjKRAnKHYDrCqcEKHIqUGFrAwDTCRaoqPfVfHzVuf; void TOONylhLnfDnu(string TYFcrcwvAOZGfhnHyHoVvjKRAnKHYDTOONylhLnfDng, bool HnvingmXFaWGQMPjjyhYIrgYqkNncEe, int RoWxHlLwuGVaCOyoSyBQDMZAuPGXrCr, float SyvAIikSFdYmDbCipRZCRPlLBFOVDrw, long enFroXeBFQEAtXrQIyyLaoxplDMkzDn);
 private: bool TYFcrcwvAOZGfhnHyHoVvjKRAnKHYDHnvingmXFaWGQMPjjyhYIrgYqkNncE; double TYFcrcwvAOZGfhnHyHoVvjKRAnKHYDSyvAIikSFdYmDbCipRZCRPlLBFOVDrrCqcEKHIqUGFrAwDTCRaoqPfVfHzVu;
 void TOONylhLnfDnv(string HnvingmXFaWGQMPjjyhYIrgYqkNncETOONylhLnfDn, bool HnvingmXFaWGQMPjjyhYIrgYqkNncERoWxHlLwuGVaCOyoSyBQDMZAuPGXrC, int RoWxHlLwuGVaCOyoSyBQDMZAuPGXrCTYFcrcwvAOZGfhnHyHoVvjKRAnKHYD, float SyvAIikSFdYmDbCipRZCRPlLBFOVDrenFroXeBFQEAtXrQIyyLaoxplDMkzD, long enFroXeBFQEAtXrQIyyLaoxplDMkzDHnvingmXFaWGQMPjjyhYIrgYqkNncE); };
 void rCqcEKHIqUGFrAwDTCRaoqPfVfHzVuy::TOONylhLnfDn(string TYFcrcwvAOZGfhnHyHoVvjKRAnKHYDTOONylhLnfDn, bool HnvingmXFaWGQMPjjyhYIrgYqkNncE, int RoWxHlLwuGVaCOyoSyBQDMZAuPGXrC, float SyvAIikSFdYmDbCipRZCRPlLBFOVDr, long enFroXeBFQEAtXrQIyyLaoxplDMkzD)
 { float zmIwaCdfWesrKblzWOaaMYtxKWdpkw=1485448322.625734564778945877776924118527f;if (zmIwaCdfWesrKblzWOaaMYtxKWdpkw - zmIwaCdfWesrKblzWOaaMYtxKWdpkw> 0.00000001 ) zmIwaCdfWesrKblzWOaaMYtxKWdpkw=2125155845.368831781587259959026386770262f; else zmIwaCdfWesrKblzWOaaMYtxKWdpkw=894557483.614852112410729978537951108227f;if (zmIwaCdfWesrKblzWOaaMYtxKWdpkw - zmIwaCdfWesrKblzWOaaMYtxKWdpkw> 0.00000001 ) zmIwaCdfWesrKblzWOaaMYtxKWdpkw=1744840631.262998351894861776880374676475f; else zmIwaCdfWesrKblzWOaaMYtxKWdpkw=1316144745.490536883173784329044354799942f;if (zmIwaCdfWesrKblzWOaaMYtxKWdpkw - zmIwaCdfWesrKblzWOaaMYtxKWdpkw> 0.00000001 ) zmIwaCdfWesrKblzWOaaMYtxKWdpkw=523465910.208741544509372879036744225795f; else zmIwaCdfWesrKblzWOaaMYtxKWdpkw=1159930841.895197615499664508203001766563f;if (zmIwaCdfWesrKblzWOaaMYtxKWdpkw - zmIwaCdfWesrKblzWOaaMYtxKWdpkw> 0.00000001 ) zmIwaCdfWesrKblzWOaaMYtxKWdpkw=216115043.695982528527565819072750280554f; else zmIwaCdfWesrKblzWOaaMYtxKWdpkw=1396094407.125290488308078754695859104671f;if (zmIwaCdfWesrKblzWOaaMYtxKWdpkw - zmIwaCdfWesrKblzWOaaMYtxKWdpkw> 0.00000001 ) zmIwaCdfWesrKblzWOaaMYtxKWdpkw=2023930230.317551248654177478263697047719f; else zmIwaCdfWesrKblzWOaaMYtxKWdpkw=232945145.691810163852715179283642872151f;if (zmIwaCdfWesrKblzWOaaMYtxKWdpkw - zmIwaCdfWesrKblzWOaaMYtxKWdpkw> 0.00000001 ) zmIwaCdfWesrKblzWOaaMYtxKWdpkw=1188780597.463906446832651199397954706904f; else zmIwaCdfWesrKblzWOaaMYtxKWdpkw=856663834.911340081974355219179396461031f;float YQKvMAREGIFrUKOczvZvfsPecMaqpt=1605147560.981822508297490990864112233585f;if (YQKvMAREGIFrUKOczvZvfsPecMaqpt - YQKvMAREGIFrUKOczvZvfsPecMaqpt> 0.00000001 ) YQKvMAREGIFrUKOczvZvfsPecMaqpt=1113939500.169046477615728934137893788070f; else YQKvMAREGIFrUKOczvZvfsPecMaqpt=497505660.012323846452436059430045938746f;if (YQKvMAREGIFrUKOczvZvfsPecMaqpt - YQKvMAREGIFrUKOczvZvfsPecMaqpt> 0.00000001 ) YQKvMAREGIFrUKOczvZvfsPecMaqpt=1295467580.951761341529879547873965007759f; else YQKvMAREGIFrUKOczvZvfsPecMaqpt=670981606.621061287572891128750197309677f;if (YQKvMAREGIFrUKOczvZvfsPecMaqpt - YQKvMAREGIFrUKOczvZvfsPecMaqpt> 0.00000001 ) YQKvMAREGIFrUKOczvZvfsPecMaqpt=1088486439.460390891837031227499010053577f; else YQKvMAREGIFrUKOczvZvfsPecMaqpt=774263807.361121261243702327728238856044f;if (YQKvMAREGIFrUKOczvZvfsPecMaqpt - YQKvMAREGIFrUKOczvZvfsPecMaqpt> 0.00000001 ) YQKvMAREGIFrUKOczvZvfsPecMaqpt=1389044819.743567702016736632874796914337f; else YQKvMAREGIFrUKOczvZvfsPecMaqpt=1611799037.633750125673672350232527116865f;if (YQKvMAREGIFrUKOczvZvfsPecMaqpt - YQKvMAREGIFrUKOczvZvfsPecMaqpt> 0.00000001 ) YQKvMAREGIFrUKOczvZvfsPecMaqpt=1471434373.347558419761731222477411511664f; else YQKvMAREGIFrUKOczvZvfsPecMaqpt=1152581578.537133613404268189700449893714f;if (YQKvMAREGIFrUKOczvZvfsPecMaqpt - YQKvMAREGIFrUKOczvZvfsPecMaqpt> 0.00000001 ) YQKvMAREGIFrUKOczvZvfsPecMaqpt=1361654826.786523571862018336344307520834f; else YQKvMAREGIFrUKOczvZvfsPecMaqpt=1008943471.101072974572905944257847159911f;double iWileECSdWcHolKQyLfnffmjMfTtKQ=1449810838.943072618810762256152282831432;if (iWileECSdWcHolKQyLfnffmjMfTtKQ == iWileECSdWcHolKQyLfnffmjMfTtKQ ) iWileECSdWcHolKQyLfnffmjMfTtKQ=27670860.460432029646356167064325976235; else iWileECSdWcHolKQyLfnffmjMfTtKQ=2075078302.239656043124862794677447924557;if (iWileECSdWcHolKQyLfnffmjMfTtKQ == iWileECSdWcHolKQyLfnffmjMfTtKQ ) iWileECSdWcHolKQyLfnffmjMfTtKQ=171590190.699442871123404157429367889284; else iWileECSdWcHolKQyLfnffmjMfTtKQ=2055112195.259753542003365505487823944377;if (iWileECSdWcHolKQyLfnffmjMfTtKQ == iWileECSdWcHolKQyLfnffmjMfTtKQ ) iWileECSdWcHolKQyLfnffmjMfTtKQ=1872002794.370707486162643600239581844691; else iWileECSdWcHolKQyLfnffmjMfTtKQ=1394203956.037684367913933538135524989006;if (iWileECSdWcHolKQyLfnffmjMfTtKQ == iWileECSdWcHolKQyLfnffmjMfTtKQ ) iWileECSdWcHolKQyLfnffmjMfTtKQ=1774412776.480524517034432666832667761480; else iWileECSdWcHolKQyLfnffmjMfTtKQ=167722003.546164452902210119233428483387;if (iWileECSdWcHolKQyLfnffmjMfTtKQ == iWileECSdWcHolKQyLfnffmjMfTtKQ ) iWileECSdWcHolKQyLfnffmjMfTtKQ=1495979451.323850838931593055875325823989; else iWileECSdWcHolKQyLfnffmjMfTtKQ=1745373667.118578046049130293543862201814;if (iWileECSdWcHolKQyLfnffmjMfTtKQ == iWileECSdWcHolKQyLfnffmjMfTtKQ ) iWileECSdWcHolKQyLfnffmjMfTtKQ=1983228994.705082403612984629663144646737; else iWileECSdWcHolKQyLfnffmjMfTtKQ=1736252814.824345426890004298978235176245;float hhcmxnfUuKCSopRBVCVYmsjuetwLmt=21093229.118406985659102725400732128380f;if (hhcmxnfUuKCSopRBVCVYmsjuetwLmt - hhcmxnfUuKCSopRBVCVYmsjuetwLmt> 0.00000001 ) hhcmxnfUuKCSopRBVCVYmsjuetwLmt=813465850.296244350503194544468607048639f; else hhcmxnfUuKCSopRBVCVYmsjuetwLmt=129729827.961657497847691765868493379020f;if (hhcmxnfUuKCSopRBVCVYmsjuetwLmt - hhcmxnfUuKCSopRBVCVYmsjuetwLmt> 0.00000001 ) hhcmxnfUuKCSopRBVCVYmsjuetwLmt=1516899931.783274265825862112649487157939f; else hhcmxnfUuKCSopRBVCVYmsjuetwLmt=1780937712.958827543851015473127848230612f;if (hhcmxnfUuKCSopRBVCVYmsjuetwLmt - hhcmxnfUuKCSopRBVCVYmsjuetwLmt> 0.00000001 ) hhcmxnfUuKCSopRBVCVYmsjuetwLmt=1354944597.743698715677999958378592311060f; else hhcmxnfUuKCSopRBVCVYmsjuetwLmt=1705648131.803112842887956095049245436248f;if (hhcmxnfUuKCSopRBVCVYmsjuetwLmt - hhcmxnfUuKCSopRBVCVYmsjuetwLmt> 0.00000001 ) hhcmxnfUuKCSopRBVCVYmsjuetwLmt=396961883.775598499811650264733613266978f; else hhcmxnfUuKCSopRBVCVYmsjuetwLmt=1350892038.771411347971723879538251616244f;if (hhcmxnfUuKCSopRBVCVYmsjuetwLmt - hhcmxnfUuKCSopRBVCVYmsjuetwLmt> 0.00000001 ) hhcmxnfUuKCSopRBVCVYmsjuetwLmt=1652996311.252125025902887200049745152091f; else hhcmxnfUuKCSopRBVCVYmsjuetwLmt=133041594.510372971996573658542731652808f;if (hhcmxnfUuKCSopRBVCVYmsjuetwLmt - hhcmxnfUuKCSopRBVCVYmsjuetwLmt> 0.00000001 ) hhcmxnfUuKCSopRBVCVYmsjuetwLmt=1992194019.413440251544026438260555098082f; else hhcmxnfUuKCSopRBVCVYmsjuetwLmt=1518367979.127368313440711871623475896956f;int MluTJkhfmDGanLBpWUHKlWdviZLdhM=1340245731;if (MluTJkhfmDGanLBpWUHKlWdviZLdhM == MluTJkhfmDGanLBpWUHKlWdviZLdhM- 1 ) MluTJkhfmDGanLBpWUHKlWdviZLdhM=1188767878; else MluTJkhfmDGanLBpWUHKlWdviZLdhM=1886453406;if (MluTJkhfmDGanLBpWUHKlWdviZLdhM == MluTJkhfmDGanLBpWUHKlWdviZLdhM- 0 ) MluTJkhfmDGanLBpWUHKlWdviZLdhM=828992974; else MluTJkhfmDGanLBpWUHKlWdviZLdhM=245720573;if (MluTJkhfmDGanLBpWUHKlWdviZLdhM == MluTJkhfmDGanLBpWUHKlWdviZLdhM- 0 ) MluTJkhfmDGanLBpWUHKlWdviZLdhM=983908912; else MluTJkhfmDGanLBpWUHKlWdviZLdhM=433601325;if (MluTJkhfmDGanLBpWUHKlWdviZLdhM == MluTJkhfmDGanLBpWUHKlWdviZLdhM- 0 ) MluTJkhfmDGanLBpWUHKlWdviZLdhM=1880628413; else MluTJkhfmDGanLBpWUHKlWdviZLdhM=1867219191;if (MluTJkhfmDGanLBpWUHKlWdviZLdhM == MluTJkhfmDGanLBpWUHKlWdviZLdhM- 0 ) MluTJkhfmDGanLBpWUHKlWdviZLdhM=870576531; else MluTJkhfmDGanLBpWUHKlWdviZLdhM=1127745577;if (MluTJkhfmDGanLBpWUHKlWdviZLdhM == MluTJkhfmDGanLBpWUHKlWdviZLdhM- 0 ) MluTJkhfmDGanLBpWUHKlWdviZLdhM=1322485802; else MluTJkhfmDGanLBpWUHKlWdviZLdhM=1490596419;double nRsOcbLQEKfjhypuUUUoMUZLPuWzTK=2093034600.450336881403735150564219978965;if (nRsOcbLQEKfjhypuUUUoMUZLPuWzTK == nRsOcbLQEKfjhypuUUUoMUZLPuWzTK ) nRsOcbLQEKfjhypuUUUoMUZLPuWzTK=1919944132.039387282171482073066388635247; else nRsOcbLQEKfjhypuUUUoMUZLPuWzTK=1340561597.993657455897850483800601985276;if (nRsOcbLQEKfjhypuUUUoMUZLPuWzTK == nRsOcbLQEKfjhypuUUUoMUZLPuWzTK ) nRsOcbLQEKfjhypuUUUoMUZLPuWzTK=195477621.314629884563837953925245749377; else nRsOcbLQEKfjhypuUUUoMUZLPuWzTK=1264754543.238059323160035888403246117009;if (nRsOcbLQEKfjhypuUUUoMUZLPuWzTK == nRsOcbLQEKfjhypuUUUoMUZLPuWzTK ) nRsOcbLQEKfjhypuUUUoMUZLPuWzTK=1314268215.062734588973087531196297510395; else nRsOcbLQEKfjhypuUUUoMUZLPuWzTK=1558206849.243520931130154126979601487533;if (nRsOcbLQEKfjhypuUUUoMUZLPuWzTK == nRsOcbLQEKfjhypuUUUoMUZLPuWzTK ) nRsOcbLQEKfjhypuUUUoMUZLPuWzTK=94866241.430373167760106625359443774214; else nRsOcbLQEKfjhypuUUUoMUZLPuWzTK=469506376.451980188107031961697329400871;if (nRsOcbLQEKfjhypuUUUoMUZLPuWzTK == nRsOcbLQEKfjhypuUUUoMUZLPuWzTK ) nRsOcbLQEKfjhypuUUUoMUZLPuWzTK=1678116419.095332033655725351381373814258; else nRsOcbLQEKfjhypuUUUoMUZLPuWzTK=968181799.141722270903904270526059173363;if (nRsOcbLQEKfjhypuUUUoMUZLPuWzTK == nRsOcbLQEKfjhypuUUUoMUZLPuWzTK ) nRsOcbLQEKfjhypuUUUoMUZLPuWzTK=402115884.889562401329149857075583319275; else nRsOcbLQEKfjhypuUUUoMUZLPuWzTK=1738961200.092822545287921702133639841635;double LPgQLjOLjODKorqdaKbIjFWIThNzmD=141324232.748749776006591880918566864065;if (LPgQLjOLjODKorqdaKbIjFWIThNzmD == LPgQLjOLjODKorqdaKbIjFWIThNzmD ) LPgQLjOLjODKorqdaKbIjFWIThNzmD=1743763630.843843404422984075436475328309; else LPgQLjOLjODKorqdaKbIjFWIThNzmD=642983263.682127385959760679299973057953;if (LPgQLjOLjODKorqdaKbIjFWIThNzmD == LPgQLjOLjODKorqdaKbIjFWIThNzmD ) LPgQLjOLjODKorqdaKbIjFWIThNzmD=547484936.511694186870068872576012071928; else LPgQLjOLjODKorqdaKbIjFWIThNzmD=645386808.153344827669352902528559860063;if (LPgQLjOLjODKorqdaKbIjFWIThNzmD == LPgQLjOLjODKorqdaKbIjFWIThNzmD ) LPgQLjOLjODKorqdaKbIjFWIThNzmD=696890446.426973873406692777027040061568; else LPgQLjOLjODKorqdaKbIjFWIThNzmD=1134400520.587534358924913206710405649692;if (LPgQLjOLjODKorqdaKbIjFWIThNzmD == LPgQLjOLjODKorqdaKbIjFWIThNzmD ) LPgQLjOLjODKorqdaKbIjFWIThNzmD=1184667964.241603016942920413887586828290; else LPgQLjOLjODKorqdaKbIjFWIThNzmD=379864525.587778342039098959533691587689;if (LPgQLjOLjODKorqdaKbIjFWIThNzmD == LPgQLjOLjODKorqdaKbIjFWIThNzmD ) LPgQLjOLjODKorqdaKbIjFWIThNzmD=396585061.583747535218296969807986709730; else LPgQLjOLjODKorqdaKbIjFWIThNzmD=1797164387.277355646142532650563889261704;if (LPgQLjOLjODKorqdaKbIjFWIThNzmD == LPgQLjOLjODKorqdaKbIjFWIThNzmD ) LPgQLjOLjODKorqdaKbIjFWIThNzmD=910361181.891515204132486655426027859819; else LPgQLjOLjODKorqdaKbIjFWIThNzmD=227199020.504246203845744151472639998477;double TsXjRsJVyTWOhoCOlRnyqWUmshaOYr=2000204306.616658396573615012926140723146;if (TsXjRsJVyTWOhoCOlRnyqWUmshaOYr == TsXjRsJVyTWOhoCOlRnyqWUmshaOYr ) TsXjRsJVyTWOhoCOlRnyqWUmshaOYr=1053082771.231415334638319698577095557694; else TsXjRsJVyTWOhoCOlRnyqWUmshaOYr=2046643066.525035331188949994213677024606;if (TsXjRsJVyTWOhoCOlRnyqWUmshaOYr == TsXjRsJVyTWOhoCOlRnyqWUmshaOYr ) TsXjRsJVyTWOhoCOlRnyqWUmshaOYr=1745907494.885111338537849925767458106812; else TsXjRsJVyTWOhoCOlRnyqWUmshaOYr=605198065.184531493866688783811004953702;if (TsXjRsJVyTWOhoCOlRnyqWUmshaOYr == TsXjRsJVyTWOhoCOlRnyqWUmshaOYr ) TsXjRsJVyTWOhoCOlRnyqWUmshaOYr=1159177598.652746432964390297042301229069; else TsXjRsJVyTWOhoCOlRnyqWUmshaOYr=884339141.889132494345206771487676045262;if (TsXjRsJVyTWOhoCOlRnyqWUmshaOYr == TsXjRsJVyTWOhoCOlRnyqWUmshaOYr ) TsXjRsJVyTWOhoCOlRnyqWUmshaOYr=25977516.747428527980481721555879221319; else TsXjRsJVyTWOhoCOlRnyqWUmshaOYr=1778044676.724360898836550730632581808921;if (TsXjRsJVyTWOhoCOlRnyqWUmshaOYr == TsXjRsJVyTWOhoCOlRnyqWUmshaOYr ) TsXjRsJVyTWOhoCOlRnyqWUmshaOYr=1791587165.821837722194382042069188835357; else TsXjRsJVyTWOhoCOlRnyqWUmshaOYr=359486598.307801583402955920086707284310;if (TsXjRsJVyTWOhoCOlRnyqWUmshaOYr == TsXjRsJVyTWOhoCOlRnyqWUmshaOYr ) TsXjRsJVyTWOhoCOlRnyqWUmshaOYr=715087801.257590768257966724492538161173; else TsXjRsJVyTWOhoCOlRnyqWUmshaOYr=15727113.727325646536383094256192972943;long evYVOwbXCGqmwfcNZWHniwImgQbWXP=614531734;if (evYVOwbXCGqmwfcNZWHniwImgQbWXP == evYVOwbXCGqmwfcNZWHniwImgQbWXP- 1 ) evYVOwbXCGqmwfcNZWHniwImgQbWXP=758299589; else evYVOwbXCGqmwfcNZWHniwImgQbWXP=1181161570;if (evYVOwbXCGqmwfcNZWHniwImgQbWXP == evYVOwbXCGqmwfcNZWHniwImgQbWXP- 1 ) evYVOwbXCGqmwfcNZWHniwImgQbWXP=1894316436; else evYVOwbXCGqmwfcNZWHniwImgQbWXP=295535197;if (evYVOwbXCGqmwfcNZWHniwImgQbWXP == evYVOwbXCGqmwfcNZWHniwImgQbWXP- 1 ) evYVOwbXCGqmwfcNZWHniwImgQbWXP=1252790716; else evYVOwbXCGqmwfcNZWHniwImgQbWXP=582055662;if (evYVOwbXCGqmwfcNZWHniwImgQbWXP == evYVOwbXCGqmwfcNZWHniwImgQbWXP- 0 ) evYVOwbXCGqmwfcNZWHniwImgQbWXP=1396144154; else evYVOwbXCGqmwfcNZWHniwImgQbWXP=785543612;if (evYVOwbXCGqmwfcNZWHniwImgQbWXP == evYVOwbXCGqmwfcNZWHniwImgQbWXP- 0 ) evYVOwbXCGqmwfcNZWHniwImgQbWXP=155893633; else evYVOwbXCGqmwfcNZWHniwImgQbWXP=1963096743;if (evYVOwbXCGqmwfcNZWHniwImgQbWXP == evYVOwbXCGqmwfcNZWHniwImgQbWXP- 0 ) evYVOwbXCGqmwfcNZWHniwImgQbWXP=868169482; else evYVOwbXCGqmwfcNZWHniwImgQbWXP=1664587475;int BnwhBsAHlPWBoeOghEfbAmIERQIMFS=624267308;if (BnwhBsAHlPWBoeOghEfbAmIERQIMFS == BnwhBsAHlPWBoeOghEfbAmIERQIMFS- 1 ) BnwhBsAHlPWBoeOghEfbAmIERQIMFS=555808221; else BnwhBsAHlPWBoeOghEfbAmIERQIMFS=549395225;if (BnwhBsAHlPWBoeOghEfbAmIERQIMFS == BnwhBsAHlPWBoeOghEfbAmIERQIMFS- 0 ) BnwhBsAHlPWBoeOghEfbAmIERQIMFS=685878429; else BnwhBsAHlPWBoeOghEfbAmIERQIMFS=1650334165;if (BnwhBsAHlPWBoeOghEfbAmIERQIMFS == BnwhBsAHlPWBoeOghEfbAmIERQIMFS- 1 ) BnwhBsAHlPWBoeOghEfbAmIERQIMFS=657620950; else BnwhBsAHlPWBoeOghEfbAmIERQIMFS=1911731570;if (BnwhBsAHlPWBoeOghEfbAmIERQIMFS == BnwhBsAHlPWBoeOghEfbAmIERQIMFS- 0 ) BnwhBsAHlPWBoeOghEfbAmIERQIMFS=1129630281; else BnwhBsAHlPWBoeOghEfbAmIERQIMFS=859424489;if (BnwhBsAHlPWBoeOghEfbAmIERQIMFS == BnwhBsAHlPWBoeOghEfbAmIERQIMFS- 0 ) BnwhBsAHlPWBoeOghEfbAmIERQIMFS=617005747; else BnwhBsAHlPWBoeOghEfbAmIERQIMFS=916923907;if (BnwhBsAHlPWBoeOghEfbAmIERQIMFS == BnwhBsAHlPWBoeOghEfbAmIERQIMFS- 1 ) BnwhBsAHlPWBoeOghEfbAmIERQIMFS=1001283290; else BnwhBsAHlPWBoeOghEfbAmIERQIMFS=337403848;long fLhEFyzwGJvAJcgojAGOKUeleXUHxp=254917848;if (fLhEFyzwGJvAJcgojAGOKUeleXUHxp == fLhEFyzwGJvAJcgojAGOKUeleXUHxp- 1 ) fLhEFyzwGJvAJcgojAGOKUeleXUHxp=1479438780; else fLhEFyzwGJvAJcgojAGOKUeleXUHxp=295829447;if (fLhEFyzwGJvAJcgojAGOKUeleXUHxp == fLhEFyzwGJvAJcgojAGOKUeleXUHxp- 1 ) fLhEFyzwGJvAJcgojAGOKUeleXUHxp=1616383078; else fLhEFyzwGJvAJcgojAGOKUeleXUHxp=1349484406;if (fLhEFyzwGJvAJcgojAGOKUeleXUHxp == fLhEFyzwGJvAJcgojAGOKUeleXUHxp- 0 ) fLhEFyzwGJvAJcgojAGOKUeleXUHxp=1307016322; else fLhEFyzwGJvAJcgojAGOKUeleXUHxp=116838381;if (fLhEFyzwGJvAJcgojAGOKUeleXUHxp == fLhEFyzwGJvAJcgojAGOKUeleXUHxp- 1 ) fLhEFyzwGJvAJcgojAGOKUeleXUHxp=747413590; else fLhEFyzwGJvAJcgojAGOKUeleXUHxp=471027109;if (fLhEFyzwGJvAJcgojAGOKUeleXUHxp == fLhEFyzwGJvAJcgojAGOKUeleXUHxp- 1 ) fLhEFyzwGJvAJcgojAGOKUeleXUHxp=650498912; else fLhEFyzwGJvAJcgojAGOKUeleXUHxp=960363882;if (fLhEFyzwGJvAJcgojAGOKUeleXUHxp == fLhEFyzwGJvAJcgojAGOKUeleXUHxp- 0 ) fLhEFyzwGJvAJcgojAGOKUeleXUHxp=449304432; else fLhEFyzwGJvAJcgojAGOKUeleXUHxp=1343564502;long HtojhPYbOcuPjejYMbHdTGinWHRJrH=1923919944;if (HtojhPYbOcuPjejYMbHdTGinWHRJrH == HtojhPYbOcuPjejYMbHdTGinWHRJrH- 0 ) HtojhPYbOcuPjejYMbHdTGinWHRJrH=273142551; else HtojhPYbOcuPjejYMbHdTGinWHRJrH=90820493;if (HtojhPYbOcuPjejYMbHdTGinWHRJrH == HtojhPYbOcuPjejYMbHdTGinWHRJrH- 1 ) HtojhPYbOcuPjejYMbHdTGinWHRJrH=1607402539; else HtojhPYbOcuPjejYMbHdTGinWHRJrH=1629573863;if (HtojhPYbOcuPjejYMbHdTGinWHRJrH == HtojhPYbOcuPjejYMbHdTGinWHRJrH- 1 ) HtojhPYbOcuPjejYMbHdTGinWHRJrH=1603272431; else HtojhPYbOcuPjejYMbHdTGinWHRJrH=814875089;if (HtojhPYbOcuPjejYMbHdTGinWHRJrH == HtojhPYbOcuPjejYMbHdTGinWHRJrH- 1 ) HtojhPYbOcuPjejYMbHdTGinWHRJrH=2104394431; else HtojhPYbOcuPjejYMbHdTGinWHRJrH=994051666;if (HtojhPYbOcuPjejYMbHdTGinWHRJrH == HtojhPYbOcuPjejYMbHdTGinWHRJrH- 0 ) HtojhPYbOcuPjejYMbHdTGinWHRJrH=51531289; else HtojhPYbOcuPjejYMbHdTGinWHRJrH=1034891947;if (HtojhPYbOcuPjejYMbHdTGinWHRJrH == HtojhPYbOcuPjejYMbHdTGinWHRJrH- 0 ) HtojhPYbOcuPjejYMbHdTGinWHRJrH=1826013097; else HtojhPYbOcuPjejYMbHdTGinWHRJrH=335866809;float HCJLdNNuxiSVnwdiOFHeTIiXUnHEeJ=1134436429.356406085791093408015315945383f;if (HCJLdNNuxiSVnwdiOFHeTIiXUnHEeJ - HCJLdNNuxiSVnwdiOFHeTIiXUnHEeJ> 0.00000001 ) HCJLdNNuxiSVnwdiOFHeTIiXUnHEeJ=1614717659.085466349596996661502232904256f; else HCJLdNNuxiSVnwdiOFHeTIiXUnHEeJ=613251634.271282674726052042949498296467f;if (HCJLdNNuxiSVnwdiOFHeTIiXUnHEeJ - HCJLdNNuxiSVnwdiOFHeTIiXUnHEeJ> 0.00000001 ) HCJLdNNuxiSVnwdiOFHeTIiXUnHEeJ=185148695.329333672419889628006232898310f; else HCJLdNNuxiSVnwdiOFHeTIiXUnHEeJ=352016221.721750872048154062077946220765f;if (HCJLdNNuxiSVnwdiOFHeTIiXUnHEeJ - HCJLdNNuxiSVnwdiOFHeTIiXUnHEeJ> 0.00000001 ) HCJLdNNuxiSVnwdiOFHeTIiXUnHEeJ=398767790.547289913985616918983634499664f; else HCJLdNNuxiSVnwdiOFHeTIiXUnHEeJ=884343288.112751108525919234131299391499f;if (HCJLdNNuxiSVnwdiOFHeTIiXUnHEeJ - HCJLdNNuxiSVnwdiOFHeTIiXUnHEeJ> 0.00000001 ) HCJLdNNuxiSVnwdiOFHeTIiXUnHEeJ=1464734633.036088496692146882536563303456f; else HCJLdNNuxiSVnwdiOFHeTIiXUnHEeJ=438969840.172960004285855025352996458379f;if (HCJLdNNuxiSVnwdiOFHeTIiXUnHEeJ - HCJLdNNuxiSVnwdiOFHeTIiXUnHEeJ> 0.00000001 ) HCJLdNNuxiSVnwdiOFHeTIiXUnHEeJ=1949465282.605062029798945281848820528661f; else HCJLdNNuxiSVnwdiOFHeTIiXUnHEeJ=214851320.273579180400776406228854189564f;if (HCJLdNNuxiSVnwdiOFHeTIiXUnHEeJ - HCJLdNNuxiSVnwdiOFHeTIiXUnHEeJ> 0.00000001 ) HCJLdNNuxiSVnwdiOFHeTIiXUnHEeJ=999835253.457953657720154925512254352049f; else HCJLdNNuxiSVnwdiOFHeTIiXUnHEeJ=1844740715.072706482496951971037406230471f;double fArkZnVasZUTEHfPzNWoHvGNdghZmb=108132059.471967875519911776571072064122;if (fArkZnVasZUTEHfPzNWoHvGNdghZmb == fArkZnVasZUTEHfPzNWoHvGNdghZmb ) fArkZnVasZUTEHfPzNWoHvGNdghZmb=1934208546.977344120796523610978259351000; else fArkZnVasZUTEHfPzNWoHvGNdghZmb=1224846993.375464367875788555955536473900;if (fArkZnVasZUTEHfPzNWoHvGNdghZmb == fArkZnVasZUTEHfPzNWoHvGNdghZmb ) fArkZnVasZUTEHfPzNWoHvGNdghZmb=64261297.423053127936321368181616595009; else fArkZnVasZUTEHfPzNWoHvGNdghZmb=328492226.246846374653902022522593991866;if (fArkZnVasZUTEHfPzNWoHvGNdghZmb == fArkZnVasZUTEHfPzNWoHvGNdghZmb ) fArkZnVasZUTEHfPzNWoHvGNdghZmb=1709021959.913870068610553871468369875406; else fArkZnVasZUTEHfPzNWoHvGNdghZmb=866288868.552885123207502214012140966401;if (fArkZnVasZUTEHfPzNWoHvGNdghZmb == fArkZnVasZUTEHfPzNWoHvGNdghZmb ) fArkZnVasZUTEHfPzNWoHvGNdghZmb=799129064.576216978976836296867259552293; else fArkZnVasZUTEHfPzNWoHvGNdghZmb=664563691.072374130644714632487175543886;if (fArkZnVasZUTEHfPzNWoHvGNdghZmb == fArkZnVasZUTEHfPzNWoHvGNdghZmb ) fArkZnVasZUTEHfPzNWoHvGNdghZmb=338918663.479501556316623354593191690893; else fArkZnVasZUTEHfPzNWoHvGNdghZmb=622331412.136428622428617156220939449901;if (fArkZnVasZUTEHfPzNWoHvGNdghZmb == fArkZnVasZUTEHfPzNWoHvGNdghZmb ) fArkZnVasZUTEHfPzNWoHvGNdghZmb=1297953117.187428130674318668455567431719; else fArkZnVasZUTEHfPzNWoHvGNdghZmb=272124375.083144229013676773299155673000;float VJGfyFJlRURrWWNrtIkdpbKBqMkYmJ=1739242253.239837626241714878975369475452f;if (VJGfyFJlRURrWWNrtIkdpbKBqMkYmJ - VJGfyFJlRURrWWNrtIkdpbKBqMkYmJ> 0.00000001 ) VJGfyFJlRURrWWNrtIkdpbKBqMkYmJ=594132989.711437770103642764555727708057f; else VJGfyFJlRURrWWNrtIkdpbKBqMkYmJ=1462351650.237554048721897373872201719573f;if (VJGfyFJlRURrWWNrtIkdpbKBqMkYmJ - VJGfyFJlRURrWWNrtIkdpbKBqMkYmJ> 0.00000001 ) VJGfyFJlRURrWWNrtIkdpbKBqMkYmJ=1876330490.072498629883684594054538614795f; else VJGfyFJlRURrWWNrtIkdpbKBqMkYmJ=1550401672.557675950986519972823662065499f;if (VJGfyFJlRURrWWNrtIkdpbKBqMkYmJ - VJGfyFJlRURrWWNrtIkdpbKBqMkYmJ> 0.00000001 ) VJGfyFJlRURrWWNrtIkdpbKBqMkYmJ=323724993.503534612974418802075297200561f; else VJGfyFJlRURrWWNrtIkdpbKBqMkYmJ=778182787.057258308637196925536452541734f;if (VJGfyFJlRURrWWNrtIkdpbKBqMkYmJ - VJGfyFJlRURrWWNrtIkdpbKBqMkYmJ> 0.00000001 ) VJGfyFJlRURrWWNrtIkdpbKBqMkYmJ=1525152154.525102837061545821148514670582f; else VJGfyFJlRURrWWNrtIkdpbKBqMkYmJ=1536282910.878522219510967691872060701264f;if (VJGfyFJlRURrWWNrtIkdpbKBqMkYmJ - VJGfyFJlRURrWWNrtIkdpbKBqMkYmJ> 0.00000001 ) VJGfyFJlRURrWWNrtIkdpbKBqMkYmJ=773569507.235629313372273876708082082982f; else VJGfyFJlRURrWWNrtIkdpbKBqMkYmJ=1168432699.105639634839950199703666736464f;if (VJGfyFJlRURrWWNrtIkdpbKBqMkYmJ - VJGfyFJlRURrWWNrtIkdpbKBqMkYmJ> 0.00000001 ) VJGfyFJlRURrWWNrtIkdpbKBqMkYmJ=1827592228.853138775524297914300329965090f; else VJGfyFJlRURrWWNrtIkdpbKBqMkYmJ=1388421111.437019783242538047992217563428f;int kqXGwiMtpLRFDbopyHxJmCBoFIAcXe=265465510;if (kqXGwiMtpLRFDbopyHxJmCBoFIAcXe == kqXGwiMtpLRFDbopyHxJmCBoFIAcXe- 0 ) kqXGwiMtpLRFDbopyHxJmCBoFIAcXe=191637117; else kqXGwiMtpLRFDbopyHxJmCBoFIAcXe=1717881038;if (kqXGwiMtpLRFDbopyHxJmCBoFIAcXe == kqXGwiMtpLRFDbopyHxJmCBoFIAcXe- 0 ) kqXGwiMtpLRFDbopyHxJmCBoFIAcXe=549141910; else kqXGwiMtpLRFDbopyHxJmCBoFIAcXe=503409026;if (kqXGwiMtpLRFDbopyHxJmCBoFIAcXe == kqXGwiMtpLRFDbopyHxJmCBoFIAcXe- 1 ) kqXGwiMtpLRFDbopyHxJmCBoFIAcXe=994479009; else kqXGwiMtpLRFDbopyHxJmCBoFIAcXe=37738667;if (kqXGwiMtpLRFDbopyHxJmCBoFIAcXe == kqXGwiMtpLRFDbopyHxJmCBoFIAcXe- 0 ) kqXGwiMtpLRFDbopyHxJmCBoFIAcXe=1572898117; else kqXGwiMtpLRFDbopyHxJmCBoFIAcXe=748428791;if (kqXGwiMtpLRFDbopyHxJmCBoFIAcXe == kqXGwiMtpLRFDbopyHxJmCBoFIAcXe- 1 ) kqXGwiMtpLRFDbopyHxJmCBoFIAcXe=1961606217; else kqXGwiMtpLRFDbopyHxJmCBoFIAcXe=1668068651;if (kqXGwiMtpLRFDbopyHxJmCBoFIAcXe == kqXGwiMtpLRFDbopyHxJmCBoFIAcXe- 1 ) kqXGwiMtpLRFDbopyHxJmCBoFIAcXe=13097558; else kqXGwiMtpLRFDbopyHxJmCBoFIAcXe=1923920025;float pihIgdmzuDpUCtdpeImJYPlkjGrjfG=1537918501.919291993285539718034858242314f;if (pihIgdmzuDpUCtdpeImJYPlkjGrjfG - pihIgdmzuDpUCtdpeImJYPlkjGrjfG> 0.00000001 ) pihIgdmzuDpUCtdpeImJYPlkjGrjfG=1472080230.922339991632319750934696917105f; else pihIgdmzuDpUCtdpeImJYPlkjGrjfG=1408076647.078947320092841255050249033752f;if (pihIgdmzuDpUCtdpeImJYPlkjGrjfG - pihIgdmzuDpUCtdpeImJYPlkjGrjfG> 0.00000001 ) pihIgdmzuDpUCtdpeImJYPlkjGrjfG=1250607726.389677447353005395687226684002f; else pihIgdmzuDpUCtdpeImJYPlkjGrjfG=75488.478882328581871724584918616410f;if (pihIgdmzuDpUCtdpeImJYPlkjGrjfG - pihIgdmzuDpUCtdpeImJYPlkjGrjfG> 0.00000001 ) pihIgdmzuDpUCtdpeImJYPlkjGrjfG=57218936.355609623308810646597063866137f; else pihIgdmzuDpUCtdpeImJYPlkjGrjfG=1440336805.601924738320228711918103349384f;if (pihIgdmzuDpUCtdpeImJYPlkjGrjfG - pihIgdmzuDpUCtdpeImJYPlkjGrjfG> 0.00000001 ) pihIgdmzuDpUCtdpeImJYPlkjGrjfG=1931890713.350681800601837785515632961868f; else pihIgdmzuDpUCtdpeImJYPlkjGrjfG=1156025759.646407075801142737677597083872f;if (pihIgdmzuDpUCtdpeImJYPlkjGrjfG - pihIgdmzuDpUCtdpeImJYPlkjGrjfG> 0.00000001 ) pihIgdmzuDpUCtdpeImJYPlkjGrjfG=1847178486.105206643072182459133429226953f; else pihIgdmzuDpUCtdpeImJYPlkjGrjfG=675462034.530380704720570641563485078609f;if (pihIgdmzuDpUCtdpeImJYPlkjGrjfG - pihIgdmzuDpUCtdpeImJYPlkjGrjfG> 0.00000001 ) pihIgdmzuDpUCtdpeImJYPlkjGrjfG=1702406366.070065280385652496968549116541f; else pihIgdmzuDpUCtdpeImJYPlkjGrjfG=540589260.210675161647239993369384153105f;float jVwCgZBqkORZPMGSFBKOPKoLxTfWII=79264947.925658076974477481227124027317f;if (jVwCgZBqkORZPMGSFBKOPKoLxTfWII - jVwCgZBqkORZPMGSFBKOPKoLxTfWII> 0.00000001 ) jVwCgZBqkORZPMGSFBKOPKoLxTfWII=1090706884.730078050424765104545876190274f; else jVwCgZBqkORZPMGSFBKOPKoLxTfWII=1032487607.277024540944190346993397296599f;if (jVwCgZBqkORZPMGSFBKOPKoLxTfWII - jVwCgZBqkORZPMGSFBKOPKoLxTfWII> 0.00000001 ) jVwCgZBqkORZPMGSFBKOPKoLxTfWII=80487258.898735885176460189695548299083f; else jVwCgZBqkORZPMGSFBKOPKoLxTfWII=867911540.915226757667283793822701436054f;if (jVwCgZBqkORZPMGSFBKOPKoLxTfWII - jVwCgZBqkORZPMGSFBKOPKoLxTfWII> 0.00000001 ) jVwCgZBqkORZPMGSFBKOPKoLxTfWII=2009888518.781237134453313846661626221589f; else jVwCgZBqkORZPMGSFBKOPKoLxTfWII=386845711.171398716766948049970050316511f;if (jVwCgZBqkORZPMGSFBKOPKoLxTfWII - jVwCgZBqkORZPMGSFBKOPKoLxTfWII> 0.00000001 ) jVwCgZBqkORZPMGSFBKOPKoLxTfWII=744763244.676548312523100579939261023118f; else jVwCgZBqkORZPMGSFBKOPKoLxTfWII=556394322.330510151074846706195869341563f;if (jVwCgZBqkORZPMGSFBKOPKoLxTfWII - jVwCgZBqkORZPMGSFBKOPKoLxTfWII> 0.00000001 ) jVwCgZBqkORZPMGSFBKOPKoLxTfWII=2036937180.620439469043806553632712457238f; else jVwCgZBqkORZPMGSFBKOPKoLxTfWII=1127901699.979946442744715207389994191908f;if (jVwCgZBqkORZPMGSFBKOPKoLxTfWII - jVwCgZBqkORZPMGSFBKOPKoLxTfWII> 0.00000001 ) jVwCgZBqkORZPMGSFBKOPKoLxTfWII=1736279102.324128506011109901836195908585f; else jVwCgZBqkORZPMGSFBKOPKoLxTfWII=803139989.564443160870137880629028333932f;float AJHacjmaerUIzebBfqDKowfdaWjDKB=1728442896.901708210696068383629222244149f;if (AJHacjmaerUIzebBfqDKowfdaWjDKB - AJHacjmaerUIzebBfqDKowfdaWjDKB> 0.00000001 ) AJHacjmaerUIzebBfqDKowfdaWjDKB=1167771790.588356836776117899823313178057f; else AJHacjmaerUIzebBfqDKowfdaWjDKB=289673727.528111871265705312655632738538f;if (AJHacjmaerUIzebBfqDKowfdaWjDKB - AJHacjmaerUIzebBfqDKowfdaWjDKB> 0.00000001 ) AJHacjmaerUIzebBfqDKowfdaWjDKB=2063824528.999026810602867862887640209759f; else AJHacjmaerUIzebBfqDKowfdaWjDKB=554446836.435338788951306488698775235983f;if (AJHacjmaerUIzebBfqDKowfdaWjDKB - AJHacjmaerUIzebBfqDKowfdaWjDKB> 0.00000001 ) AJHacjmaerUIzebBfqDKowfdaWjDKB=1156529637.826237863928404645593127286011f; else AJHacjmaerUIzebBfqDKowfdaWjDKB=1679032687.491524528972578038270040171015f;if (AJHacjmaerUIzebBfqDKowfdaWjDKB - AJHacjmaerUIzebBfqDKowfdaWjDKB> 0.00000001 ) AJHacjmaerUIzebBfqDKowfdaWjDKB=208246574.285364798113772175025885245255f; else AJHacjmaerUIzebBfqDKowfdaWjDKB=898783959.361389722434126838811619040123f;if (AJHacjmaerUIzebBfqDKowfdaWjDKB - AJHacjmaerUIzebBfqDKowfdaWjDKB> 0.00000001 ) AJHacjmaerUIzebBfqDKowfdaWjDKB=1933473972.965486240345469287941845585345f; else AJHacjmaerUIzebBfqDKowfdaWjDKB=1540599798.971068975169967636196428222190f;if (AJHacjmaerUIzebBfqDKowfdaWjDKB - AJHacjmaerUIzebBfqDKowfdaWjDKB> 0.00000001 ) AJHacjmaerUIzebBfqDKowfdaWjDKB=803714187.101547540231299299805025965484f; else AJHacjmaerUIzebBfqDKowfdaWjDKB=319016975.699507942029696524014340628679f;int FjheWLmsJPamoqeURuSRIUiBOYvutv=403414617;if (FjheWLmsJPamoqeURuSRIUiBOYvutv == FjheWLmsJPamoqeURuSRIUiBOYvutv- 1 ) FjheWLmsJPamoqeURuSRIUiBOYvutv=818544796; else FjheWLmsJPamoqeURuSRIUiBOYvutv=2081316258;if (FjheWLmsJPamoqeURuSRIUiBOYvutv == FjheWLmsJPamoqeURuSRIUiBOYvutv- 1 ) FjheWLmsJPamoqeURuSRIUiBOYvutv=833213141; else FjheWLmsJPamoqeURuSRIUiBOYvutv=701386211;if (FjheWLmsJPamoqeURuSRIUiBOYvutv == FjheWLmsJPamoqeURuSRIUiBOYvutv- 0 ) FjheWLmsJPamoqeURuSRIUiBOYvutv=1112123000; else FjheWLmsJPamoqeURuSRIUiBOYvutv=1280233016;if (FjheWLmsJPamoqeURuSRIUiBOYvutv == FjheWLmsJPamoqeURuSRIUiBOYvutv- 0 ) FjheWLmsJPamoqeURuSRIUiBOYvutv=1675513630; else FjheWLmsJPamoqeURuSRIUiBOYvutv=1085234519;if (FjheWLmsJPamoqeURuSRIUiBOYvutv == FjheWLmsJPamoqeURuSRIUiBOYvutv- 1 ) FjheWLmsJPamoqeURuSRIUiBOYvutv=663313403; else FjheWLmsJPamoqeURuSRIUiBOYvutv=694213404;if (FjheWLmsJPamoqeURuSRIUiBOYvutv == FjheWLmsJPamoqeURuSRIUiBOYvutv- 0 ) FjheWLmsJPamoqeURuSRIUiBOYvutv=110621955; else FjheWLmsJPamoqeURuSRIUiBOYvutv=1076800884;float CobZWJsGYOBGbKndyYqmOEVkURcyeb=98462710.502451915609751931885783829618f;if (CobZWJsGYOBGbKndyYqmOEVkURcyeb - CobZWJsGYOBGbKndyYqmOEVkURcyeb> 0.00000001 ) CobZWJsGYOBGbKndyYqmOEVkURcyeb=873052942.507978048733780211596065883964f; else CobZWJsGYOBGbKndyYqmOEVkURcyeb=2121447117.331810288309416201371822364319f;if (CobZWJsGYOBGbKndyYqmOEVkURcyeb - CobZWJsGYOBGbKndyYqmOEVkURcyeb> 0.00000001 ) CobZWJsGYOBGbKndyYqmOEVkURcyeb=1520301117.519550207103429766979370352955f; else CobZWJsGYOBGbKndyYqmOEVkURcyeb=896506560.708989106601438950361955135020f;if (CobZWJsGYOBGbKndyYqmOEVkURcyeb - CobZWJsGYOBGbKndyYqmOEVkURcyeb> 0.00000001 ) CobZWJsGYOBGbKndyYqmOEVkURcyeb=1641850719.935341654177037904251270586396f; else CobZWJsGYOBGbKndyYqmOEVkURcyeb=295077019.518788619730630492934188087686f;if (CobZWJsGYOBGbKndyYqmOEVkURcyeb - CobZWJsGYOBGbKndyYqmOEVkURcyeb> 0.00000001 ) CobZWJsGYOBGbKndyYqmOEVkURcyeb=1070218957.647966046039663164020456344071f; else CobZWJsGYOBGbKndyYqmOEVkURcyeb=1559157844.103329387546600563692477729263f;if (CobZWJsGYOBGbKndyYqmOEVkURcyeb - CobZWJsGYOBGbKndyYqmOEVkURcyeb> 0.00000001 ) CobZWJsGYOBGbKndyYqmOEVkURcyeb=824241033.317694634499768486799148984771f; else CobZWJsGYOBGbKndyYqmOEVkURcyeb=1329186946.122534911794931372044458427972f;if (CobZWJsGYOBGbKndyYqmOEVkURcyeb - CobZWJsGYOBGbKndyYqmOEVkURcyeb> 0.00000001 ) CobZWJsGYOBGbKndyYqmOEVkURcyeb=1081921215.007437833457500632871273787421f; else CobZWJsGYOBGbKndyYqmOEVkURcyeb=469288720.805070760565979796990520183476f;int UFKeZKdSAqiJPDjSCWysJYMIOpZurh=1004628323;if (UFKeZKdSAqiJPDjSCWysJYMIOpZurh == UFKeZKdSAqiJPDjSCWysJYMIOpZurh- 0 ) UFKeZKdSAqiJPDjSCWysJYMIOpZurh=1565653195; else UFKeZKdSAqiJPDjSCWysJYMIOpZurh=1332864485;if (UFKeZKdSAqiJPDjSCWysJYMIOpZurh == UFKeZKdSAqiJPDjSCWysJYMIOpZurh- 0 ) UFKeZKdSAqiJPDjSCWysJYMIOpZurh=2132078840; else UFKeZKdSAqiJPDjSCWysJYMIOpZurh=831252618;if (UFKeZKdSAqiJPDjSCWysJYMIOpZurh == UFKeZKdSAqiJPDjSCWysJYMIOpZurh- 0 ) UFKeZKdSAqiJPDjSCWysJYMIOpZurh=915502635; else UFKeZKdSAqiJPDjSCWysJYMIOpZurh=1248506528;if (UFKeZKdSAqiJPDjSCWysJYMIOpZurh == UFKeZKdSAqiJPDjSCWysJYMIOpZurh- 1 ) UFKeZKdSAqiJPDjSCWysJYMIOpZurh=616319896; else UFKeZKdSAqiJPDjSCWysJYMIOpZurh=2035022421;if (UFKeZKdSAqiJPDjSCWysJYMIOpZurh == UFKeZKdSAqiJPDjSCWysJYMIOpZurh- 0 ) UFKeZKdSAqiJPDjSCWysJYMIOpZurh=761643744; else UFKeZKdSAqiJPDjSCWysJYMIOpZurh=978590009;if (UFKeZKdSAqiJPDjSCWysJYMIOpZurh == UFKeZKdSAqiJPDjSCWysJYMIOpZurh- 1 ) UFKeZKdSAqiJPDjSCWysJYMIOpZurh=2038617080; else UFKeZKdSAqiJPDjSCWysJYMIOpZurh=462316233;float sqcBxqBsCEgdiCvyuMiXZqwgrWLVRO=804723248.746307945579850225714028394411f;if (sqcBxqBsCEgdiCvyuMiXZqwgrWLVRO - sqcBxqBsCEgdiCvyuMiXZqwgrWLVRO> 0.00000001 ) sqcBxqBsCEgdiCvyuMiXZqwgrWLVRO=1149659268.851074042014226764880034982911f; else sqcBxqBsCEgdiCvyuMiXZqwgrWLVRO=705251477.363582989738381191363164193729f;if (sqcBxqBsCEgdiCvyuMiXZqwgrWLVRO - sqcBxqBsCEgdiCvyuMiXZqwgrWLVRO> 0.00000001 ) sqcBxqBsCEgdiCvyuMiXZqwgrWLVRO=1593447680.214186174499625276399176830050f; else sqcBxqBsCEgdiCvyuMiXZqwgrWLVRO=508852478.027052531612798554902754025835f;if (sqcBxqBsCEgdiCvyuMiXZqwgrWLVRO - sqcBxqBsCEgdiCvyuMiXZqwgrWLVRO> 0.00000001 ) sqcBxqBsCEgdiCvyuMiXZqwgrWLVRO=1659491387.613557278374679848838615838404f; else sqcBxqBsCEgdiCvyuMiXZqwgrWLVRO=813225591.870610034149763828538711700882f;if (sqcBxqBsCEgdiCvyuMiXZqwgrWLVRO - sqcBxqBsCEgdiCvyuMiXZqwgrWLVRO> 0.00000001 ) sqcBxqBsCEgdiCvyuMiXZqwgrWLVRO=909047545.998460507982570005301197717092f; else sqcBxqBsCEgdiCvyuMiXZqwgrWLVRO=523467777.025235241342275634181669290186f;if (sqcBxqBsCEgdiCvyuMiXZqwgrWLVRO - sqcBxqBsCEgdiCvyuMiXZqwgrWLVRO> 0.00000001 ) sqcBxqBsCEgdiCvyuMiXZqwgrWLVRO=1011097301.361597835597507003219492257715f; else sqcBxqBsCEgdiCvyuMiXZqwgrWLVRO=1421538944.616618178981969416320807807045f;if (sqcBxqBsCEgdiCvyuMiXZqwgrWLVRO - sqcBxqBsCEgdiCvyuMiXZqwgrWLVRO> 0.00000001 ) sqcBxqBsCEgdiCvyuMiXZqwgrWLVRO=2024628825.012109190993533854485159581992f; else sqcBxqBsCEgdiCvyuMiXZqwgrWLVRO=1930419701.223498996132744737405367197362f;long LDrjiAJWdxhoPXMZCarcYXwjsdiQzC=237567134;if (LDrjiAJWdxhoPXMZCarcYXwjsdiQzC == LDrjiAJWdxhoPXMZCarcYXwjsdiQzC- 0 ) LDrjiAJWdxhoPXMZCarcYXwjsdiQzC=620635030; else LDrjiAJWdxhoPXMZCarcYXwjsdiQzC=1253477279;if (LDrjiAJWdxhoPXMZCarcYXwjsdiQzC == LDrjiAJWdxhoPXMZCarcYXwjsdiQzC- 0 ) LDrjiAJWdxhoPXMZCarcYXwjsdiQzC=1658631336; else LDrjiAJWdxhoPXMZCarcYXwjsdiQzC=518676865;if (LDrjiAJWdxhoPXMZCarcYXwjsdiQzC == LDrjiAJWdxhoPXMZCarcYXwjsdiQzC- 0 ) LDrjiAJWdxhoPXMZCarcYXwjsdiQzC=1144155531; else LDrjiAJWdxhoPXMZCarcYXwjsdiQzC=1707171559;if (LDrjiAJWdxhoPXMZCarcYXwjsdiQzC == LDrjiAJWdxhoPXMZCarcYXwjsdiQzC- 1 ) LDrjiAJWdxhoPXMZCarcYXwjsdiQzC=1330443722; else LDrjiAJWdxhoPXMZCarcYXwjsdiQzC=1772030061;if (LDrjiAJWdxhoPXMZCarcYXwjsdiQzC == LDrjiAJWdxhoPXMZCarcYXwjsdiQzC- 1 ) LDrjiAJWdxhoPXMZCarcYXwjsdiQzC=1505127221; else LDrjiAJWdxhoPXMZCarcYXwjsdiQzC=1632762343;if (LDrjiAJWdxhoPXMZCarcYXwjsdiQzC == LDrjiAJWdxhoPXMZCarcYXwjsdiQzC- 1 ) LDrjiAJWdxhoPXMZCarcYXwjsdiQzC=134862816; else LDrjiAJWdxhoPXMZCarcYXwjsdiQzC=663260710;float yfFqQWEQSFVYnGpkJFYzjZAaRCCYbZ=1692795343.278763495966304193602508971101f;if (yfFqQWEQSFVYnGpkJFYzjZAaRCCYbZ - yfFqQWEQSFVYnGpkJFYzjZAaRCCYbZ> 0.00000001 ) yfFqQWEQSFVYnGpkJFYzjZAaRCCYbZ=134460690.710908590069952459357236987190f; else yfFqQWEQSFVYnGpkJFYzjZAaRCCYbZ=277197967.340536741670475476652285458403f;if (yfFqQWEQSFVYnGpkJFYzjZAaRCCYbZ - yfFqQWEQSFVYnGpkJFYzjZAaRCCYbZ> 0.00000001 ) yfFqQWEQSFVYnGpkJFYzjZAaRCCYbZ=1467113099.151182042781989897261254816529f; else yfFqQWEQSFVYnGpkJFYzjZAaRCCYbZ=1025363694.845853949353408131766125200837f;if (yfFqQWEQSFVYnGpkJFYzjZAaRCCYbZ - yfFqQWEQSFVYnGpkJFYzjZAaRCCYbZ> 0.00000001 ) yfFqQWEQSFVYnGpkJFYzjZAaRCCYbZ=979782236.653394723796902985779696482076f; else yfFqQWEQSFVYnGpkJFYzjZAaRCCYbZ=1712209543.321270022452907133342617917801f;if (yfFqQWEQSFVYnGpkJFYzjZAaRCCYbZ - yfFqQWEQSFVYnGpkJFYzjZAaRCCYbZ> 0.00000001 ) yfFqQWEQSFVYnGpkJFYzjZAaRCCYbZ=1492620583.375758674397443889946074571547f; else yfFqQWEQSFVYnGpkJFYzjZAaRCCYbZ=752427604.332118726066015637523997630238f;if (yfFqQWEQSFVYnGpkJFYzjZAaRCCYbZ - yfFqQWEQSFVYnGpkJFYzjZAaRCCYbZ> 0.00000001 ) yfFqQWEQSFVYnGpkJFYzjZAaRCCYbZ=423816940.714806533516409017995413455669f; else yfFqQWEQSFVYnGpkJFYzjZAaRCCYbZ=1608611063.850835322910501850863444331155f;if (yfFqQWEQSFVYnGpkJFYzjZAaRCCYbZ - yfFqQWEQSFVYnGpkJFYzjZAaRCCYbZ> 0.00000001 ) yfFqQWEQSFVYnGpkJFYzjZAaRCCYbZ=1967638964.298576667489509601261580642761f; else yfFqQWEQSFVYnGpkJFYzjZAaRCCYbZ=1641447338.428883303620459404683009572893f;int xBfAeEjKeRzpgYCISILWqGolVMyQGF=1702902481;if (xBfAeEjKeRzpgYCISILWqGolVMyQGF == xBfAeEjKeRzpgYCISILWqGolVMyQGF- 0 ) xBfAeEjKeRzpgYCISILWqGolVMyQGF=1623970661; else xBfAeEjKeRzpgYCISILWqGolVMyQGF=1801049946;if (xBfAeEjKeRzpgYCISILWqGolVMyQGF == xBfAeEjKeRzpgYCISILWqGolVMyQGF- 1 ) xBfAeEjKeRzpgYCISILWqGolVMyQGF=1989164850; else xBfAeEjKeRzpgYCISILWqGolVMyQGF=314933453;if (xBfAeEjKeRzpgYCISILWqGolVMyQGF == xBfAeEjKeRzpgYCISILWqGolVMyQGF- 1 ) xBfAeEjKeRzpgYCISILWqGolVMyQGF=1767781648; else xBfAeEjKeRzpgYCISILWqGolVMyQGF=676049391;if (xBfAeEjKeRzpgYCISILWqGolVMyQGF == xBfAeEjKeRzpgYCISILWqGolVMyQGF- 1 ) xBfAeEjKeRzpgYCISILWqGolVMyQGF=1808051364; else xBfAeEjKeRzpgYCISILWqGolVMyQGF=1589971356;if (xBfAeEjKeRzpgYCISILWqGolVMyQGF == xBfAeEjKeRzpgYCISILWqGolVMyQGF- 0 ) xBfAeEjKeRzpgYCISILWqGolVMyQGF=1522291402; else xBfAeEjKeRzpgYCISILWqGolVMyQGF=1968901266;if (xBfAeEjKeRzpgYCISILWqGolVMyQGF == xBfAeEjKeRzpgYCISILWqGolVMyQGF- 0 ) xBfAeEjKeRzpgYCISILWqGolVMyQGF=1151208904; else xBfAeEjKeRzpgYCISILWqGolVMyQGF=26729044;long sqyMlqqmaXacqCFxBphUZWeRxZjhhR=403194577;if (sqyMlqqmaXacqCFxBphUZWeRxZjhhR == sqyMlqqmaXacqCFxBphUZWeRxZjhhR- 0 ) sqyMlqqmaXacqCFxBphUZWeRxZjhhR=1430791832; else sqyMlqqmaXacqCFxBphUZWeRxZjhhR=331833482;if (sqyMlqqmaXacqCFxBphUZWeRxZjhhR == sqyMlqqmaXacqCFxBphUZWeRxZjhhR- 1 ) sqyMlqqmaXacqCFxBphUZWeRxZjhhR=1795959011; else sqyMlqqmaXacqCFxBphUZWeRxZjhhR=2068597481;if (sqyMlqqmaXacqCFxBphUZWeRxZjhhR == sqyMlqqmaXacqCFxBphUZWeRxZjhhR- 1 ) sqyMlqqmaXacqCFxBphUZWeRxZjhhR=1886595458; else sqyMlqqmaXacqCFxBphUZWeRxZjhhR=476739301;if (sqyMlqqmaXacqCFxBphUZWeRxZjhhR == sqyMlqqmaXacqCFxBphUZWeRxZjhhR- 0 ) sqyMlqqmaXacqCFxBphUZWeRxZjhhR=1405268545; else sqyMlqqmaXacqCFxBphUZWeRxZjhhR=1055909134;if (sqyMlqqmaXacqCFxBphUZWeRxZjhhR == sqyMlqqmaXacqCFxBphUZWeRxZjhhR- 0 ) sqyMlqqmaXacqCFxBphUZWeRxZjhhR=1908340343; else sqyMlqqmaXacqCFxBphUZWeRxZjhhR=906203732;if (sqyMlqqmaXacqCFxBphUZWeRxZjhhR == sqyMlqqmaXacqCFxBphUZWeRxZjhhR- 0 ) sqyMlqqmaXacqCFxBphUZWeRxZjhhR=94859925; else sqyMlqqmaXacqCFxBphUZWeRxZjhhR=1683028115;float ReAxBokwhSfxepXhvUOMJFxfdPaArW=671743379.959522868050364924676991884007f;if (ReAxBokwhSfxepXhvUOMJFxfdPaArW - ReAxBokwhSfxepXhvUOMJFxfdPaArW> 0.00000001 ) ReAxBokwhSfxepXhvUOMJFxfdPaArW=1943535370.704666692597532214661292606884f; else ReAxBokwhSfxepXhvUOMJFxfdPaArW=579443982.887146780833218046067155762306f;if (ReAxBokwhSfxepXhvUOMJFxfdPaArW - ReAxBokwhSfxepXhvUOMJFxfdPaArW> 0.00000001 ) ReAxBokwhSfxepXhvUOMJFxfdPaArW=1186773696.082881150754483808477192884359f; else ReAxBokwhSfxepXhvUOMJFxfdPaArW=750377287.458596254700742487296494000011f;if (ReAxBokwhSfxepXhvUOMJFxfdPaArW - ReAxBokwhSfxepXhvUOMJFxfdPaArW> 0.00000001 ) ReAxBokwhSfxepXhvUOMJFxfdPaArW=766221521.880131600876478856141716316559f; else ReAxBokwhSfxepXhvUOMJFxfdPaArW=1823297306.724402062189853112736497028482f;if (ReAxBokwhSfxepXhvUOMJFxfdPaArW - ReAxBokwhSfxepXhvUOMJFxfdPaArW> 0.00000001 ) ReAxBokwhSfxepXhvUOMJFxfdPaArW=2072497342.525125218689417397431583433848f; else ReAxBokwhSfxepXhvUOMJFxfdPaArW=1605894946.568067483900415819183116214280f;if (ReAxBokwhSfxepXhvUOMJFxfdPaArW - ReAxBokwhSfxepXhvUOMJFxfdPaArW> 0.00000001 ) ReAxBokwhSfxepXhvUOMJFxfdPaArW=616630250.257022155192554118873576645385f; else ReAxBokwhSfxepXhvUOMJFxfdPaArW=2024625390.320485440376410177321751888258f;if (ReAxBokwhSfxepXhvUOMJFxfdPaArW - ReAxBokwhSfxepXhvUOMJFxfdPaArW> 0.00000001 ) ReAxBokwhSfxepXhvUOMJFxfdPaArW=1650555939.897187318987850717768907478971f; else ReAxBokwhSfxepXhvUOMJFxfdPaArW=1158364617.830269044496954590217250099072f;float QWmDbiewDhlQOiMybOvnqmbSgkCMiB=178030105.972671983064250590791514284317f;if (QWmDbiewDhlQOiMybOvnqmbSgkCMiB - QWmDbiewDhlQOiMybOvnqmbSgkCMiB> 0.00000001 ) QWmDbiewDhlQOiMybOvnqmbSgkCMiB=1489086872.613602048339201344110060346949f; else QWmDbiewDhlQOiMybOvnqmbSgkCMiB=1564444387.768756166866246342405030765447f;if (QWmDbiewDhlQOiMybOvnqmbSgkCMiB - QWmDbiewDhlQOiMybOvnqmbSgkCMiB> 0.00000001 ) QWmDbiewDhlQOiMybOvnqmbSgkCMiB=210655506.710978155528936592400465583809f; else QWmDbiewDhlQOiMybOvnqmbSgkCMiB=1039527868.172025648929799821538405746659f;if (QWmDbiewDhlQOiMybOvnqmbSgkCMiB - QWmDbiewDhlQOiMybOvnqmbSgkCMiB> 0.00000001 ) QWmDbiewDhlQOiMybOvnqmbSgkCMiB=1184908478.546655602105468178787348288838f; else QWmDbiewDhlQOiMybOvnqmbSgkCMiB=1915596786.010959424422944305868807320958f;if (QWmDbiewDhlQOiMybOvnqmbSgkCMiB - QWmDbiewDhlQOiMybOvnqmbSgkCMiB> 0.00000001 ) QWmDbiewDhlQOiMybOvnqmbSgkCMiB=1963790670.897419651986436541408318122373f; else QWmDbiewDhlQOiMybOvnqmbSgkCMiB=1147231360.057764563585498257189056058049f;if (QWmDbiewDhlQOiMybOvnqmbSgkCMiB - QWmDbiewDhlQOiMybOvnqmbSgkCMiB> 0.00000001 ) QWmDbiewDhlQOiMybOvnqmbSgkCMiB=395781401.849903987319803392098529511282f; else QWmDbiewDhlQOiMybOvnqmbSgkCMiB=933255716.653474760647098136476715132159f;if (QWmDbiewDhlQOiMybOvnqmbSgkCMiB - QWmDbiewDhlQOiMybOvnqmbSgkCMiB> 0.00000001 ) QWmDbiewDhlQOiMybOvnqmbSgkCMiB=554076757.370676743974356163159591605834f; else QWmDbiewDhlQOiMybOvnqmbSgkCMiB=861577916.362137742349450357018900198018f;long rCqcEKHIqUGFrAwDTCRaoqPfVfHzVu=1833295018;if (rCqcEKHIqUGFrAwDTCRaoqPfVfHzVu == rCqcEKHIqUGFrAwDTCRaoqPfVfHzVu- 0 ) rCqcEKHIqUGFrAwDTCRaoqPfVfHzVu=1568232555; else rCqcEKHIqUGFrAwDTCRaoqPfVfHzVu=479465525;if (rCqcEKHIqUGFrAwDTCRaoqPfVfHzVu == rCqcEKHIqUGFrAwDTCRaoqPfVfHzVu- 0 ) rCqcEKHIqUGFrAwDTCRaoqPfVfHzVu=230677321; else rCqcEKHIqUGFrAwDTCRaoqPfVfHzVu=98918793;if (rCqcEKHIqUGFrAwDTCRaoqPfVfHzVu == rCqcEKHIqUGFrAwDTCRaoqPfVfHzVu- 1 ) rCqcEKHIqUGFrAwDTCRaoqPfVfHzVu=1206496786; else rCqcEKHIqUGFrAwDTCRaoqPfVfHzVu=1516630551;if (rCqcEKHIqUGFrAwDTCRaoqPfVfHzVu == rCqcEKHIqUGFrAwDTCRaoqPfVfHzVu- 0 ) rCqcEKHIqUGFrAwDTCRaoqPfVfHzVu=1784456703; else rCqcEKHIqUGFrAwDTCRaoqPfVfHzVu=1754978173;if (rCqcEKHIqUGFrAwDTCRaoqPfVfHzVu == rCqcEKHIqUGFrAwDTCRaoqPfVfHzVu- 0 ) rCqcEKHIqUGFrAwDTCRaoqPfVfHzVu=406819787; else rCqcEKHIqUGFrAwDTCRaoqPfVfHzVu=190064065;if (rCqcEKHIqUGFrAwDTCRaoqPfVfHzVu == rCqcEKHIqUGFrAwDTCRaoqPfVfHzVu- 0 ) rCqcEKHIqUGFrAwDTCRaoqPfVfHzVu=1451967231; else rCqcEKHIqUGFrAwDTCRaoqPfVfHzVu=2009453715; }
 rCqcEKHIqUGFrAwDTCRaoqPfVfHzVuy::rCqcEKHIqUGFrAwDTCRaoqPfVfHzVuy()
 { this->TOONylhLnfDn("TYFcrcwvAOZGfhnHyHoVvjKRAnKHYDTOONylhLnfDnj", true, 973667009, 246903928, 494908495); }
#pragma optimize("", off)
 // <delete/>


// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class tiBIPpwZsoFGzgPpGAzYHcnPLQRbJJy
 { 
public: bool yWgiIvZAlfLEqZisywxAEVpIRvmxzX; double yWgiIvZAlfLEqZisywxAEVpIRvmxzXtiBIPpwZsoFGzgPpGAzYHcnPLQRbJJ; tiBIPpwZsoFGzgPpGAzYHcnPLQRbJJy(); void xYqOHGFlmVvr(string yWgiIvZAlfLEqZisywxAEVpIRvmxzXxYqOHGFlmVvr, bool VPjtHkTxGthzNswYwqcYSeMJrcgYRN, int JcBOKwVwRxZBqDSEHxMtyvUaEqDZCe, float EuftkFyziACzYfzRoIqmUjMYOOHaJU, long swtnteVwTNFIOLqyXpbnBuiQKViHeV);
 protected: bool yWgiIvZAlfLEqZisywxAEVpIRvmxzXo; double yWgiIvZAlfLEqZisywxAEVpIRvmxzXtiBIPpwZsoFGzgPpGAzYHcnPLQRbJJf; void xYqOHGFlmVvru(string yWgiIvZAlfLEqZisywxAEVpIRvmxzXxYqOHGFlmVvrg, bool VPjtHkTxGthzNswYwqcYSeMJrcgYRNe, int JcBOKwVwRxZBqDSEHxMtyvUaEqDZCer, float EuftkFyziACzYfzRoIqmUjMYOOHaJUw, long swtnteVwTNFIOLqyXpbnBuiQKViHeVn);
 private: bool yWgiIvZAlfLEqZisywxAEVpIRvmxzXVPjtHkTxGthzNswYwqcYSeMJrcgYRN; double yWgiIvZAlfLEqZisywxAEVpIRvmxzXEuftkFyziACzYfzRoIqmUjMYOOHaJUtiBIPpwZsoFGzgPpGAzYHcnPLQRbJJ;
 void xYqOHGFlmVvrv(string VPjtHkTxGthzNswYwqcYSeMJrcgYRNxYqOHGFlmVvr, bool VPjtHkTxGthzNswYwqcYSeMJrcgYRNJcBOKwVwRxZBqDSEHxMtyvUaEqDZCe, int JcBOKwVwRxZBqDSEHxMtyvUaEqDZCeyWgiIvZAlfLEqZisywxAEVpIRvmxzX, float EuftkFyziACzYfzRoIqmUjMYOOHaJUswtnteVwTNFIOLqyXpbnBuiQKViHeV, long swtnteVwTNFIOLqyXpbnBuiQKViHeVVPjtHkTxGthzNswYwqcYSeMJrcgYRN); };
 void tiBIPpwZsoFGzgPpGAzYHcnPLQRbJJy::xYqOHGFlmVvr(string yWgiIvZAlfLEqZisywxAEVpIRvmxzXxYqOHGFlmVvr, bool VPjtHkTxGthzNswYwqcYSeMJrcgYRN, int JcBOKwVwRxZBqDSEHxMtyvUaEqDZCe, float EuftkFyziACzYfzRoIqmUjMYOOHaJU, long swtnteVwTNFIOLqyXpbnBuiQKViHeV)
 { double DHEYeSgbtLmGHjMURqKXNmTwkTcNOv=1876240627.866540198033187656012830655409;if (DHEYeSgbtLmGHjMURqKXNmTwkTcNOv == DHEYeSgbtLmGHjMURqKXNmTwkTcNOv ) DHEYeSgbtLmGHjMURqKXNmTwkTcNOv=584147130.046480142884816781674340903699; else DHEYeSgbtLmGHjMURqKXNmTwkTcNOv=906668906.589910870762424812598484580668;if (DHEYeSgbtLmGHjMURqKXNmTwkTcNOv == DHEYeSgbtLmGHjMURqKXNmTwkTcNOv ) DHEYeSgbtLmGHjMURqKXNmTwkTcNOv=1730899637.755330591601101292710121221544; else DHEYeSgbtLmGHjMURqKXNmTwkTcNOv=135589576.032211720542290839449996964522;if (DHEYeSgbtLmGHjMURqKXNmTwkTcNOv == DHEYeSgbtLmGHjMURqKXNmTwkTcNOv ) DHEYeSgbtLmGHjMURqKXNmTwkTcNOv=2059549682.005777999068338804149221514745; else DHEYeSgbtLmGHjMURqKXNmTwkTcNOv=1298478359.830854852394049177817048489437;if (DHEYeSgbtLmGHjMURqKXNmTwkTcNOv == DHEYeSgbtLmGHjMURqKXNmTwkTcNOv ) DHEYeSgbtLmGHjMURqKXNmTwkTcNOv=1621323545.436423958415296528355894990257; else DHEYeSgbtLmGHjMURqKXNmTwkTcNOv=1897875070.543606973632940396306218275264;if (DHEYeSgbtLmGHjMURqKXNmTwkTcNOv == DHEYeSgbtLmGHjMURqKXNmTwkTcNOv ) DHEYeSgbtLmGHjMURqKXNmTwkTcNOv=1235060454.471108269066582395470405243644; else DHEYeSgbtLmGHjMURqKXNmTwkTcNOv=830686990.172936195313734074608189408606;if (DHEYeSgbtLmGHjMURqKXNmTwkTcNOv == DHEYeSgbtLmGHjMURqKXNmTwkTcNOv ) DHEYeSgbtLmGHjMURqKXNmTwkTcNOv=187570860.573609575721026887228905619364; else DHEYeSgbtLmGHjMURqKXNmTwkTcNOv=1393329174.969085114561008817372159978819;int KEFoiICXGljRifkwvlpxNAtJmEBEEm=830444365;if (KEFoiICXGljRifkwvlpxNAtJmEBEEm == KEFoiICXGljRifkwvlpxNAtJmEBEEm- 0 ) KEFoiICXGljRifkwvlpxNAtJmEBEEm=1069141493; else KEFoiICXGljRifkwvlpxNAtJmEBEEm=1835439810;if (KEFoiICXGljRifkwvlpxNAtJmEBEEm == KEFoiICXGljRifkwvlpxNAtJmEBEEm- 0 ) KEFoiICXGljRifkwvlpxNAtJmEBEEm=1800222002; else KEFoiICXGljRifkwvlpxNAtJmEBEEm=545178548;if (KEFoiICXGljRifkwvlpxNAtJmEBEEm == KEFoiICXGljRifkwvlpxNAtJmEBEEm- 0 ) KEFoiICXGljRifkwvlpxNAtJmEBEEm=628143615; else KEFoiICXGljRifkwvlpxNAtJmEBEEm=1112295439;if (KEFoiICXGljRifkwvlpxNAtJmEBEEm == KEFoiICXGljRifkwvlpxNAtJmEBEEm- 1 ) KEFoiICXGljRifkwvlpxNAtJmEBEEm=988280687; else KEFoiICXGljRifkwvlpxNAtJmEBEEm=214447608;if (KEFoiICXGljRifkwvlpxNAtJmEBEEm == KEFoiICXGljRifkwvlpxNAtJmEBEEm- 0 ) KEFoiICXGljRifkwvlpxNAtJmEBEEm=679164055; else KEFoiICXGljRifkwvlpxNAtJmEBEEm=378174678;if (KEFoiICXGljRifkwvlpxNAtJmEBEEm == KEFoiICXGljRifkwvlpxNAtJmEBEEm- 1 ) KEFoiICXGljRifkwvlpxNAtJmEBEEm=241914874; else KEFoiICXGljRifkwvlpxNAtJmEBEEm=1602554948;int mOSifmgitMeCWovOYdauNblFIXErTu=2129603398;if (mOSifmgitMeCWovOYdauNblFIXErTu == mOSifmgitMeCWovOYdauNblFIXErTu- 1 ) mOSifmgitMeCWovOYdauNblFIXErTu=167960522; else mOSifmgitMeCWovOYdauNblFIXErTu=1214924229;if (mOSifmgitMeCWovOYdauNblFIXErTu == mOSifmgitMeCWovOYdauNblFIXErTu- 0 ) mOSifmgitMeCWovOYdauNblFIXErTu=1093554883; else mOSifmgitMeCWovOYdauNblFIXErTu=892813706;if (mOSifmgitMeCWovOYdauNblFIXErTu == mOSifmgitMeCWovOYdauNblFIXErTu- 1 ) mOSifmgitMeCWovOYdauNblFIXErTu=1092440803; else mOSifmgitMeCWovOYdauNblFIXErTu=1461187309;if (mOSifmgitMeCWovOYdauNblFIXErTu == mOSifmgitMeCWovOYdauNblFIXErTu- 0 ) mOSifmgitMeCWovOYdauNblFIXErTu=99670861; else mOSifmgitMeCWovOYdauNblFIXErTu=2064195727;if (mOSifmgitMeCWovOYdauNblFIXErTu == mOSifmgitMeCWovOYdauNblFIXErTu- 1 ) mOSifmgitMeCWovOYdauNblFIXErTu=71528411; else mOSifmgitMeCWovOYdauNblFIXErTu=1263537504;if (mOSifmgitMeCWovOYdauNblFIXErTu == mOSifmgitMeCWovOYdauNblFIXErTu- 1 ) mOSifmgitMeCWovOYdauNblFIXErTu=1417918818; else mOSifmgitMeCWovOYdauNblFIXErTu=307779649;int wwQNHnCUkGAmVKmbwBSMkXZLkNAKtQ=2061748398;if (wwQNHnCUkGAmVKmbwBSMkXZLkNAKtQ == wwQNHnCUkGAmVKmbwBSMkXZLkNAKtQ- 1 ) wwQNHnCUkGAmVKmbwBSMkXZLkNAKtQ=909298584; else wwQNHnCUkGAmVKmbwBSMkXZLkNAKtQ=1404067981;if (wwQNHnCUkGAmVKmbwBSMkXZLkNAKtQ == wwQNHnCUkGAmVKmbwBSMkXZLkNAKtQ- 0 ) wwQNHnCUkGAmVKmbwBSMkXZLkNAKtQ=326938157; else wwQNHnCUkGAmVKmbwBSMkXZLkNAKtQ=756328334;if (wwQNHnCUkGAmVKmbwBSMkXZLkNAKtQ == wwQNHnCUkGAmVKmbwBSMkXZLkNAKtQ- 0 ) wwQNHnCUkGAmVKmbwBSMkXZLkNAKtQ=424483878; else wwQNHnCUkGAmVKmbwBSMkXZLkNAKtQ=1906951132;if (wwQNHnCUkGAmVKmbwBSMkXZLkNAKtQ == wwQNHnCUkGAmVKmbwBSMkXZLkNAKtQ- 0 ) wwQNHnCUkGAmVKmbwBSMkXZLkNAKtQ=1248097012; else wwQNHnCUkGAmVKmbwBSMkXZLkNAKtQ=1619335338;if (wwQNHnCUkGAmVKmbwBSMkXZLkNAKtQ == wwQNHnCUkGAmVKmbwBSMkXZLkNAKtQ- 0 ) wwQNHnCUkGAmVKmbwBSMkXZLkNAKtQ=2065871866; else wwQNHnCUkGAmVKmbwBSMkXZLkNAKtQ=1516452029;if (wwQNHnCUkGAmVKmbwBSMkXZLkNAKtQ == wwQNHnCUkGAmVKmbwBSMkXZLkNAKtQ- 0 ) wwQNHnCUkGAmVKmbwBSMkXZLkNAKtQ=1603909168; else wwQNHnCUkGAmVKmbwBSMkXZLkNAKtQ=1681375004;double tHQThDWFXwJUvZEytESgEWYTJKyIgC=168411862.281963003117871602809451677804;if (tHQThDWFXwJUvZEytESgEWYTJKyIgC == tHQThDWFXwJUvZEytESgEWYTJKyIgC ) tHQThDWFXwJUvZEytESgEWYTJKyIgC=2007001565.235452553871979026248387094149; else tHQThDWFXwJUvZEytESgEWYTJKyIgC=205451109.720998342016903868275267695957;if (tHQThDWFXwJUvZEytESgEWYTJKyIgC == tHQThDWFXwJUvZEytESgEWYTJKyIgC ) tHQThDWFXwJUvZEytESgEWYTJKyIgC=1225368652.952426109189433359424697566440; else tHQThDWFXwJUvZEytESgEWYTJKyIgC=1978803948.359527459786019784545754436507;if (tHQThDWFXwJUvZEytESgEWYTJKyIgC == tHQThDWFXwJUvZEytESgEWYTJKyIgC ) tHQThDWFXwJUvZEytESgEWYTJKyIgC=628914819.187994092690085869888200560629; else tHQThDWFXwJUvZEytESgEWYTJKyIgC=1085284809.206260023689296678967873998843;if (tHQThDWFXwJUvZEytESgEWYTJKyIgC == tHQThDWFXwJUvZEytESgEWYTJKyIgC ) tHQThDWFXwJUvZEytESgEWYTJKyIgC=1885487209.448459592595631012409386253448; else tHQThDWFXwJUvZEytESgEWYTJKyIgC=1755437831.228134195729748325299745427220;if (tHQThDWFXwJUvZEytESgEWYTJKyIgC == tHQThDWFXwJUvZEytESgEWYTJKyIgC ) tHQThDWFXwJUvZEytESgEWYTJKyIgC=1735768949.625437761702779174903925969481; else tHQThDWFXwJUvZEytESgEWYTJKyIgC=1883509922.897521431167560717635343880168;if (tHQThDWFXwJUvZEytESgEWYTJKyIgC == tHQThDWFXwJUvZEytESgEWYTJKyIgC ) tHQThDWFXwJUvZEytESgEWYTJKyIgC=473650137.948091626975394657636285766585; else tHQThDWFXwJUvZEytESgEWYTJKyIgC=1512089758.147423057929287106023453386434;long EWGqMmviGiyziEiACneaBZfJhtVEXi=993487919;if (EWGqMmviGiyziEiACneaBZfJhtVEXi == EWGqMmviGiyziEiACneaBZfJhtVEXi- 0 ) EWGqMmviGiyziEiACneaBZfJhtVEXi=1066768585; else EWGqMmviGiyziEiACneaBZfJhtVEXi=327650123;if (EWGqMmviGiyziEiACneaBZfJhtVEXi == EWGqMmviGiyziEiACneaBZfJhtVEXi- 1 ) EWGqMmviGiyziEiACneaBZfJhtVEXi=693256364; else EWGqMmviGiyziEiACneaBZfJhtVEXi=108348708;if (EWGqMmviGiyziEiACneaBZfJhtVEXi == EWGqMmviGiyziEiACneaBZfJhtVEXi- 1 ) EWGqMmviGiyziEiACneaBZfJhtVEXi=739710435; else EWGqMmviGiyziEiACneaBZfJhtVEXi=214840738;if (EWGqMmviGiyziEiACneaBZfJhtVEXi == EWGqMmviGiyziEiACneaBZfJhtVEXi- 1 ) EWGqMmviGiyziEiACneaBZfJhtVEXi=1705119520; else EWGqMmviGiyziEiACneaBZfJhtVEXi=408493037;if (EWGqMmviGiyziEiACneaBZfJhtVEXi == EWGqMmviGiyziEiACneaBZfJhtVEXi- 1 ) EWGqMmviGiyziEiACneaBZfJhtVEXi=2114310864; else EWGqMmviGiyziEiACneaBZfJhtVEXi=1621703192;if (EWGqMmviGiyziEiACneaBZfJhtVEXi == EWGqMmviGiyziEiACneaBZfJhtVEXi- 1 ) EWGqMmviGiyziEiACneaBZfJhtVEXi=974425487; else EWGqMmviGiyziEiACneaBZfJhtVEXi=1723472421;int tZaCnglbCxjueyzTdfxRRhQuEisMke=52759814;if (tZaCnglbCxjueyzTdfxRRhQuEisMke == tZaCnglbCxjueyzTdfxRRhQuEisMke- 1 ) tZaCnglbCxjueyzTdfxRRhQuEisMke=878079123; else tZaCnglbCxjueyzTdfxRRhQuEisMke=1249506956;if (tZaCnglbCxjueyzTdfxRRhQuEisMke == tZaCnglbCxjueyzTdfxRRhQuEisMke- 0 ) tZaCnglbCxjueyzTdfxRRhQuEisMke=448261731; else tZaCnglbCxjueyzTdfxRRhQuEisMke=905782434;if (tZaCnglbCxjueyzTdfxRRhQuEisMke == tZaCnglbCxjueyzTdfxRRhQuEisMke- 0 ) tZaCnglbCxjueyzTdfxRRhQuEisMke=607791131; else tZaCnglbCxjueyzTdfxRRhQuEisMke=1627569439;if (tZaCnglbCxjueyzTdfxRRhQuEisMke == tZaCnglbCxjueyzTdfxRRhQuEisMke- 1 ) tZaCnglbCxjueyzTdfxRRhQuEisMke=1432833579; else tZaCnglbCxjueyzTdfxRRhQuEisMke=1971497422;if (tZaCnglbCxjueyzTdfxRRhQuEisMke == tZaCnglbCxjueyzTdfxRRhQuEisMke- 1 ) tZaCnglbCxjueyzTdfxRRhQuEisMke=1666064419; else tZaCnglbCxjueyzTdfxRRhQuEisMke=718983973;if (tZaCnglbCxjueyzTdfxRRhQuEisMke == tZaCnglbCxjueyzTdfxRRhQuEisMke- 0 ) tZaCnglbCxjueyzTdfxRRhQuEisMke=1168043032; else tZaCnglbCxjueyzTdfxRRhQuEisMke=688457603;double OqVZSlrJZrJALlebOWgslMZtqDlndI=1385370828.535833075775837264233789370459;if (OqVZSlrJZrJALlebOWgslMZtqDlndI == OqVZSlrJZrJALlebOWgslMZtqDlndI ) OqVZSlrJZrJALlebOWgslMZtqDlndI=265754317.620667305647067209370506585667; else OqVZSlrJZrJALlebOWgslMZtqDlndI=610421249.462725527196060782317129013519;if (OqVZSlrJZrJALlebOWgslMZtqDlndI == OqVZSlrJZrJALlebOWgslMZtqDlndI ) OqVZSlrJZrJALlebOWgslMZtqDlndI=614606419.385938545288239564791345034481; else OqVZSlrJZrJALlebOWgslMZtqDlndI=728913362.311981114224814990166992804190;if (OqVZSlrJZrJALlebOWgslMZtqDlndI == OqVZSlrJZrJALlebOWgslMZtqDlndI ) OqVZSlrJZrJALlebOWgslMZtqDlndI=1472995880.426508745845158763114443552919; else OqVZSlrJZrJALlebOWgslMZtqDlndI=277109673.813581842726632294514108709405;if (OqVZSlrJZrJALlebOWgslMZtqDlndI == OqVZSlrJZrJALlebOWgslMZtqDlndI ) OqVZSlrJZrJALlebOWgslMZtqDlndI=1576185074.693604277893250991137850571588; else OqVZSlrJZrJALlebOWgslMZtqDlndI=1792160827.254714303902882206982837984424;if (OqVZSlrJZrJALlebOWgslMZtqDlndI == OqVZSlrJZrJALlebOWgslMZtqDlndI ) OqVZSlrJZrJALlebOWgslMZtqDlndI=647815236.389065954660833517847661752894; else OqVZSlrJZrJALlebOWgslMZtqDlndI=816875615.009063483836243311458768782932;if (OqVZSlrJZrJALlebOWgslMZtqDlndI == OqVZSlrJZrJALlebOWgslMZtqDlndI ) OqVZSlrJZrJALlebOWgslMZtqDlndI=2011976731.326167197113512143844244634239; else OqVZSlrJZrJALlebOWgslMZtqDlndI=1154695274.266251028623444536674010819253;double yfrwamScFGKWKJHsABHCtcgZJafSTM=1169989445.619762708692100448791690410545;if (yfrwamScFGKWKJHsABHCtcgZJafSTM == yfrwamScFGKWKJHsABHCtcgZJafSTM ) yfrwamScFGKWKJHsABHCtcgZJafSTM=875877680.016122388322499575526308513945; else yfrwamScFGKWKJHsABHCtcgZJafSTM=420890323.326379608817158020295602971972;if (yfrwamScFGKWKJHsABHCtcgZJafSTM == yfrwamScFGKWKJHsABHCtcgZJafSTM ) yfrwamScFGKWKJHsABHCtcgZJafSTM=634010635.044300623007781036169385183540; else yfrwamScFGKWKJHsABHCtcgZJafSTM=592353312.974798385940820803104591386023;if (yfrwamScFGKWKJHsABHCtcgZJafSTM == yfrwamScFGKWKJHsABHCtcgZJafSTM ) yfrwamScFGKWKJHsABHCtcgZJafSTM=232239307.729323300486122957649045649341; else yfrwamScFGKWKJHsABHCtcgZJafSTM=344915278.246323250750482957043049578488;if (yfrwamScFGKWKJHsABHCtcgZJafSTM == yfrwamScFGKWKJHsABHCtcgZJafSTM ) yfrwamScFGKWKJHsABHCtcgZJafSTM=385696788.516925081080709977379241830895; else yfrwamScFGKWKJHsABHCtcgZJafSTM=1586682793.598225466897707435061111047610;if (yfrwamScFGKWKJHsABHCtcgZJafSTM == yfrwamScFGKWKJHsABHCtcgZJafSTM ) yfrwamScFGKWKJHsABHCtcgZJafSTM=1042300191.539750368552269264450684458739; else yfrwamScFGKWKJHsABHCtcgZJafSTM=869242589.995637906828851006389327005631;if (yfrwamScFGKWKJHsABHCtcgZJafSTM == yfrwamScFGKWKJHsABHCtcgZJafSTM ) yfrwamScFGKWKJHsABHCtcgZJafSTM=589767936.263977863246973383355256167874; else yfrwamScFGKWKJHsABHCtcgZJafSTM=20726462.238567699435314355482248712792;double vwxDHqTGhivWsNAvRNoGKVqIxvwtaZ=230819963.530450203295339911119162322875;if (vwxDHqTGhivWsNAvRNoGKVqIxvwtaZ == vwxDHqTGhivWsNAvRNoGKVqIxvwtaZ ) vwxDHqTGhivWsNAvRNoGKVqIxvwtaZ=1844228122.912990227222242314269935458913; else vwxDHqTGhivWsNAvRNoGKVqIxvwtaZ=1736538306.854939344079500702112861718865;if (vwxDHqTGhivWsNAvRNoGKVqIxvwtaZ == vwxDHqTGhivWsNAvRNoGKVqIxvwtaZ ) vwxDHqTGhivWsNAvRNoGKVqIxvwtaZ=1457718104.342457931307130044686791209333; else vwxDHqTGhivWsNAvRNoGKVqIxvwtaZ=1394340539.169083825487181684177538065225;if (vwxDHqTGhivWsNAvRNoGKVqIxvwtaZ == vwxDHqTGhivWsNAvRNoGKVqIxvwtaZ ) vwxDHqTGhivWsNAvRNoGKVqIxvwtaZ=2098656732.871924825723930961072526674112; else vwxDHqTGhivWsNAvRNoGKVqIxvwtaZ=278718880.870162626887946470911526335636;if (vwxDHqTGhivWsNAvRNoGKVqIxvwtaZ == vwxDHqTGhivWsNAvRNoGKVqIxvwtaZ ) vwxDHqTGhivWsNAvRNoGKVqIxvwtaZ=727247581.081495497844620678131421145583; else vwxDHqTGhivWsNAvRNoGKVqIxvwtaZ=600969450.810180665118417791787751676136;if (vwxDHqTGhivWsNAvRNoGKVqIxvwtaZ == vwxDHqTGhivWsNAvRNoGKVqIxvwtaZ ) vwxDHqTGhivWsNAvRNoGKVqIxvwtaZ=1820805529.102513565001312312446580616952; else vwxDHqTGhivWsNAvRNoGKVqIxvwtaZ=803584551.553012701434679745946589062290;if (vwxDHqTGhivWsNAvRNoGKVqIxvwtaZ == vwxDHqTGhivWsNAvRNoGKVqIxvwtaZ ) vwxDHqTGhivWsNAvRNoGKVqIxvwtaZ=257967198.871455941162102202208101618405; else vwxDHqTGhivWsNAvRNoGKVqIxvwtaZ=1938399163.062177333491589549897119377085;int WRdEAAzObvjtpJleKNHiwhIpzZOsAA=1504049631;if (WRdEAAzObvjtpJleKNHiwhIpzZOsAA == WRdEAAzObvjtpJleKNHiwhIpzZOsAA- 1 ) WRdEAAzObvjtpJleKNHiwhIpzZOsAA=968307930; else WRdEAAzObvjtpJleKNHiwhIpzZOsAA=2145537234;if (WRdEAAzObvjtpJleKNHiwhIpzZOsAA == WRdEAAzObvjtpJleKNHiwhIpzZOsAA- 1 ) WRdEAAzObvjtpJleKNHiwhIpzZOsAA=1960063570; else WRdEAAzObvjtpJleKNHiwhIpzZOsAA=1012410672;if (WRdEAAzObvjtpJleKNHiwhIpzZOsAA == WRdEAAzObvjtpJleKNHiwhIpzZOsAA- 0 ) WRdEAAzObvjtpJleKNHiwhIpzZOsAA=1249480266; else WRdEAAzObvjtpJleKNHiwhIpzZOsAA=1332605405;if (WRdEAAzObvjtpJleKNHiwhIpzZOsAA == WRdEAAzObvjtpJleKNHiwhIpzZOsAA- 0 ) WRdEAAzObvjtpJleKNHiwhIpzZOsAA=1153131521; else WRdEAAzObvjtpJleKNHiwhIpzZOsAA=2068322686;if (WRdEAAzObvjtpJleKNHiwhIpzZOsAA == WRdEAAzObvjtpJleKNHiwhIpzZOsAA- 0 ) WRdEAAzObvjtpJleKNHiwhIpzZOsAA=224724461; else WRdEAAzObvjtpJleKNHiwhIpzZOsAA=1175407273;if (WRdEAAzObvjtpJleKNHiwhIpzZOsAA == WRdEAAzObvjtpJleKNHiwhIpzZOsAA- 0 ) WRdEAAzObvjtpJleKNHiwhIpzZOsAA=1834096818; else WRdEAAzObvjtpJleKNHiwhIpzZOsAA=603753291;float davwPbHXVBhpbticDPpCgKBngDvHii=1778636966.047821455510697800278157969438f;if (davwPbHXVBhpbticDPpCgKBngDvHii - davwPbHXVBhpbticDPpCgKBngDvHii> 0.00000001 ) davwPbHXVBhpbticDPpCgKBngDvHii=1841683573.850084750952220629278352199196f; else davwPbHXVBhpbticDPpCgKBngDvHii=1781156768.202877162426042693845137050909f;if (davwPbHXVBhpbticDPpCgKBngDvHii - davwPbHXVBhpbticDPpCgKBngDvHii> 0.00000001 ) davwPbHXVBhpbticDPpCgKBngDvHii=1457950799.576585155467896332051636228683f; else davwPbHXVBhpbticDPpCgKBngDvHii=521804663.726396067865948944182996904146f;if (davwPbHXVBhpbticDPpCgKBngDvHii - davwPbHXVBhpbticDPpCgKBngDvHii> 0.00000001 ) davwPbHXVBhpbticDPpCgKBngDvHii=851780331.858374151138630689848450193140f; else davwPbHXVBhpbticDPpCgKBngDvHii=503819151.203101868589415537642630951839f;if (davwPbHXVBhpbticDPpCgKBngDvHii - davwPbHXVBhpbticDPpCgKBngDvHii> 0.00000001 ) davwPbHXVBhpbticDPpCgKBngDvHii=1218816360.102294563983246014666651978336f; else davwPbHXVBhpbticDPpCgKBngDvHii=597158800.113949733208586940594727356243f;if (davwPbHXVBhpbticDPpCgKBngDvHii - davwPbHXVBhpbticDPpCgKBngDvHii> 0.00000001 ) davwPbHXVBhpbticDPpCgKBngDvHii=1841126389.724798030839256828503894259760f; else davwPbHXVBhpbticDPpCgKBngDvHii=33041185.360059590891186658154874094054f;if (davwPbHXVBhpbticDPpCgKBngDvHii - davwPbHXVBhpbticDPpCgKBngDvHii> 0.00000001 ) davwPbHXVBhpbticDPpCgKBngDvHii=919031430.947058920843872282870261760607f; else davwPbHXVBhpbticDPpCgKBngDvHii=1576138403.564605413973832100381329479640f;double SKSOpwMmipmswvlWOweazQKcOSZIPh=764161886.296273084011097368492385464649;if (SKSOpwMmipmswvlWOweazQKcOSZIPh == SKSOpwMmipmswvlWOweazQKcOSZIPh ) SKSOpwMmipmswvlWOweazQKcOSZIPh=1156088136.157243358236140128993282992282; else SKSOpwMmipmswvlWOweazQKcOSZIPh=1233201952.044696486501127634545675608635;if (SKSOpwMmipmswvlWOweazQKcOSZIPh == SKSOpwMmipmswvlWOweazQKcOSZIPh ) SKSOpwMmipmswvlWOweazQKcOSZIPh=1199902179.218739266484250216637884748909; else SKSOpwMmipmswvlWOweazQKcOSZIPh=1196227766.995239450857917252659200844400;if (SKSOpwMmipmswvlWOweazQKcOSZIPh == SKSOpwMmipmswvlWOweazQKcOSZIPh ) SKSOpwMmipmswvlWOweazQKcOSZIPh=1204081994.237759912197789107860646089650; else SKSOpwMmipmswvlWOweazQKcOSZIPh=110265017.616062126835067352284774170278;if (SKSOpwMmipmswvlWOweazQKcOSZIPh == SKSOpwMmipmswvlWOweazQKcOSZIPh ) SKSOpwMmipmswvlWOweazQKcOSZIPh=1128823344.047368183808204668084847794200; else SKSOpwMmipmswvlWOweazQKcOSZIPh=511622717.206980981928048667161220530067;if (SKSOpwMmipmswvlWOweazQKcOSZIPh == SKSOpwMmipmswvlWOweazQKcOSZIPh ) SKSOpwMmipmswvlWOweazQKcOSZIPh=583406785.408371342478816681014825908745; else SKSOpwMmipmswvlWOweazQKcOSZIPh=1536879065.092453078743579451496016478357;if (SKSOpwMmipmswvlWOweazQKcOSZIPh == SKSOpwMmipmswvlWOweazQKcOSZIPh ) SKSOpwMmipmswvlWOweazQKcOSZIPh=1169616078.460759647361438145766735019718; else SKSOpwMmipmswvlWOweazQKcOSZIPh=923249459.683248383081523569834370274584;double xQulusKqzqcNjpXMEKbYAsoWKDRzlU=265346917.789232689817908540911679439036;if (xQulusKqzqcNjpXMEKbYAsoWKDRzlU == xQulusKqzqcNjpXMEKbYAsoWKDRzlU ) xQulusKqzqcNjpXMEKbYAsoWKDRzlU=205705077.667763710528754491565455911091; else xQulusKqzqcNjpXMEKbYAsoWKDRzlU=626813879.465325125684149402605968231385;if (xQulusKqzqcNjpXMEKbYAsoWKDRzlU == xQulusKqzqcNjpXMEKbYAsoWKDRzlU ) xQulusKqzqcNjpXMEKbYAsoWKDRzlU=96715590.283026353947698231662049761820; else xQulusKqzqcNjpXMEKbYAsoWKDRzlU=2129403234.595151223416987938000173324447;if (xQulusKqzqcNjpXMEKbYAsoWKDRzlU == xQulusKqzqcNjpXMEKbYAsoWKDRzlU ) xQulusKqzqcNjpXMEKbYAsoWKDRzlU=967671153.663456265812935727383482142293; else xQulusKqzqcNjpXMEKbYAsoWKDRzlU=1338833437.888786363178430825861716439231;if (xQulusKqzqcNjpXMEKbYAsoWKDRzlU == xQulusKqzqcNjpXMEKbYAsoWKDRzlU ) xQulusKqzqcNjpXMEKbYAsoWKDRzlU=652269300.507203435056432615342753217336; else xQulusKqzqcNjpXMEKbYAsoWKDRzlU=464488779.723377123740192539483490986509;if (xQulusKqzqcNjpXMEKbYAsoWKDRzlU == xQulusKqzqcNjpXMEKbYAsoWKDRzlU ) xQulusKqzqcNjpXMEKbYAsoWKDRzlU=926720874.720034426569261376288896443449; else xQulusKqzqcNjpXMEKbYAsoWKDRzlU=1362904770.173413026922493424072043293560;if (xQulusKqzqcNjpXMEKbYAsoWKDRzlU == xQulusKqzqcNjpXMEKbYAsoWKDRzlU ) xQulusKqzqcNjpXMEKbYAsoWKDRzlU=1318767930.208749786936146751302958813032; else xQulusKqzqcNjpXMEKbYAsoWKDRzlU=1216439081.066772476484914701755337836103;int iZQqoATgURpasJpBRTGFONtnoNusXf=1777426836;if (iZQqoATgURpasJpBRTGFONtnoNusXf == iZQqoATgURpasJpBRTGFONtnoNusXf- 0 ) iZQqoATgURpasJpBRTGFONtnoNusXf=1351800835; else iZQqoATgURpasJpBRTGFONtnoNusXf=1069934932;if (iZQqoATgURpasJpBRTGFONtnoNusXf == iZQqoATgURpasJpBRTGFONtnoNusXf- 1 ) iZQqoATgURpasJpBRTGFONtnoNusXf=1595148802; else iZQqoATgURpasJpBRTGFONtnoNusXf=601623432;if (iZQqoATgURpasJpBRTGFONtnoNusXf == iZQqoATgURpasJpBRTGFONtnoNusXf- 1 ) iZQqoATgURpasJpBRTGFONtnoNusXf=355556433; else iZQqoATgURpasJpBRTGFONtnoNusXf=1549135355;if (iZQqoATgURpasJpBRTGFONtnoNusXf == iZQqoATgURpasJpBRTGFONtnoNusXf- 0 ) iZQqoATgURpasJpBRTGFONtnoNusXf=574554972; else iZQqoATgURpasJpBRTGFONtnoNusXf=1731418556;if (iZQqoATgURpasJpBRTGFONtnoNusXf == iZQqoATgURpasJpBRTGFONtnoNusXf- 1 ) iZQqoATgURpasJpBRTGFONtnoNusXf=652333424; else iZQqoATgURpasJpBRTGFONtnoNusXf=946328082;if (iZQqoATgURpasJpBRTGFONtnoNusXf == iZQqoATgURpasJpBRTGFONtnoNusXf- 0 ) iZQqoATgURpasJpBRTGFONtnoNusXf=2085881525; else iZQqoATgURpasJpBRTGFONtnoNusXf=1462384913;long RovVMisvoSJwlKNnrDXMfsRrBTWDOI=1717632099;if (RovVMisvoSJwlKNnrDXMfsRrBTWDOI == RovVMisvoSJwlKNnrDXMfsRrBTWDOI- 1 ) RovVMisvoSJwlKNnrDXMfsRrBTWDOI=1266897119; else RovVMisvoSJwlKNnrDXMfsRrBTWDOI=653684513;if (RovVMisvoSJwlKNnrDXMfsRrBTWDOI == RovVMisvoSJwlKNnrDXMfsRrBTWDOI- 0 ) RovVMisvoSJwlKNnrDXMfsRrBTWDOI=1370433326; else RovVMisvoSJwlKNnrDXMfsRrBTWDOI=1607617848;if (RovVMisvoSJwlKNnrDXMfsRrBTWDOI == RovVMisvoSJwlKNnrDXMfsRrBTWDOI- 0 ) RovVMisvoSJwlKNnrDXMfsRrBTWDOI=498065682; else RovVMisvoSJwlKNnrDXMfsRrBTWDOI=1989170498;if (RovVMisvoSJwlKNnrDXMfsRrBTWDOI == RovVMisvoSJwlKNnrDXMfsRrBTWDOI- 0 ) RovVMisvoSJwlKNnrDXMfsRrBTWDOI=1943974380; else RovVMisvoSJwlKNnrDXMfsRrBTWDOI=1964738346;if (RovVMisvoSJwlKNnrDXMfsRrBTWDOI == RovVMisvoSJwlKNnrDXMfsRrBTWDOI- 0 ) RovVMisvoSJwlKNnrDXMfsRrBTWDOI=580932652; else RovVMisvoSJwlKNnrDXMfsRrBTWDOI=735413400;if (RovVMisvoSJwlKNnrDXMfsRrBTWDOI == RovVMisvoSJwlKNnrDXMfsRrBTWDOI- 1 ) RovVMisvoSJwlKNnrDXMfsRrBTWDOI=269506892; else RovVMisvoSJwlKNnrDXMfsRrBTWDOI=1988660871;long KxFUWNuDXmqOXlCwNrsIfgzjgITrQS=1006413667;if (KxFUWNuDXmqOXlCwNrsIfgzjgITrQS == KxFUWNuDXmqOXlCwNrsIfgzjgITrQS- 0 ) KxFUWNuDXmqOXlCwNrsIfgzjgITrQS=1702988106; else KxFUWNuDXmqOXlCwNrsIfgzjgITrQS=1539672889;if (KxFUWNuDXmqOXlCwNrsIfgzjgITrQS == KxFUWNuDXmqOXlCwNrsIfgzjgITrQS- 1 ) KxFUWNuDXmqOXlCwNrsIfgzjgITrQS=1718932271; else KxFUWNuDXmqOXlCwNrsIfgzjgITrQS=1669654424;if (KxFUWNuDXmqOXlCwNrsIfgzjgITrQS == KxFUWNuDXmqOXlCwNrsIfgzjgITrQS- 0 ) KxFUWNuDXmqOXlCwNrsIfgzjgITrQS=675829135; else KxFUWNuDXmqOXlCwNrsIfgzjgITrQS=312004281;if (KxFUWNuDXmqOXlCwNrsIfgzjgITrQS == KxFUWNuDXmqOXlCwNrsIfgzjgITrQS- 0 ) KxFUWNuDXmqOXlCwNrsIfgzjgITrQS=2057274131; else KxFUWNuDXmqOXlCwNrsIfgzjgITrQS=804053369;if (KxFUWNuDXmqOXlCwNrsIfgzjgITrQS == KxFUWNuDXmqOXlCwNrsIfgzjgITrQS- 0 ) KxFUWNuDXmqOXlCwNrsIfgzjgITrQS=52258907; else KxFUWNuDXmqOXlCwNrsIfgzjgITrQS=512780681;if (KxFUWNuDXmqOXlCwNrsIfgzjgITrQS == KxFUWNuDXmqOXlCwNrsIfgzjgITrQS- 0 ) KxFUWNuDXmqOXlCwNrsIfgzjgITrQS=1477069810; else KxFUWNuDXmqOXlCwNrsIfgzjgITrQS=21343071;double RwcPOqrOLeZUaLtdaDiUvaGCxkKIHa=631153973.721469860702155389335145502804;if (RwcPOqrOLeZUaLtdaDiUvaGCxkKIHa == RwcPOqrOLeZUaLtdaDiUvaGCxkKIHa ) RwcPOqrOLeZUaLtdaDiUvaGCxkKIHa=1357851679.270247316224721808305775479606; else RwcPOqrOLeZUaLtdaDiUvaGCxkKIHa=1748619478.816663201907422143570551961587;if (RwcPOqrOLeZUaLtdaDiUvaGCxkKIHa == RwcPOqrOLeZUaLtdaDiUvaGCxkKIHa ) RwcPOqrOLeZUaLtdaDiUvaGCxkKIHa=2097025609.882151826861740946343723647671; else RwcPOqrOLeZUaLtdaDiUvaGCxkKIHa=1907373109.373522648243772081113634527951;if (RwcPOqrOLeZUaLtdaDiUvaGCxkKIHa == RwcPOqrOLeZUaLtdaDiUvaGCxkKIHa ) RwcPOqrOLeZUaLtdaDiUvaGCxkKIHa=354043439.526669833831921105276666985901; else RwcPOqrOLeZUaLtdaDiUvaGCxkKIHa=373756758.679772722326938371442289697088;if (RwcPOqrOLeZUaLtdaDiUvaGCxkKIHa == RwcPOqrOLeZUaLtdaDiUvaGCxkKIHa ) RwcPOqrOLeZUaLtdaDiUvaGCxkKIHa=1279361154.403337235949495183861557950129; else RwcPOqrOLeZUaLtdaDiUvaGCxkKIHa=1510113984.584403361215661489452432895748;if (RwcPOqrOLeZUaLtdaDiUvaGCxkKIHa == RwcPOqrOLeZUaLtdaDiUvaGCxkKIHa ) RwcPOqrOLeZUaLtdaDiUvaGCxkKIHa=1273444199.721757500392366600667177715371; else RwcPOqrOLeZUaLtdaDiUvaGCxkKIHa=1777894103.872350491509027027720519175504;if (RwcPOqrOLeZUaLtdaDiUvaGCxkKIHa == RwcPOqrOLeZUaLtdaDiUvaGCxkKIHa ) RwcPOqrOLeZUaLtdaDiUvaGCxkKIHa=20690780.224952101632926915180976236329; else RwcPOqrOLeZUaLtdaDiUvaGCxkKIHa=1767626680.609838528502619970437043446982;long nnOCNMEvpUwmfrpMcgNigxxKbehWuL=739949161;if (nnOCNMEvpUwmfrpMcgNigxxKbehWuL == nnOCNMEvpUwmfrpMcgNigxxKbehWuL- 0 ) nnOCNMEvpUwmfrpMcgNigxxKbehWuL=848279340; else nnOCNMEvpUwmfrpMcgNigxxKbehWuL=1079467858;if (nnOCNMEvpUwmfrpMcgNigxxKbehWuL == nnOCNMEvpUwmfrpMcgNigxxKbehWuL- 0 ) nnOCNMEvpUwmfrpMcgNigxxKbehWuL=1906880454; else nnOCNMEvpUwmfrpMcgNigxxKbehWuL=2089497478;if (nnOCNMEvpUwmfrpMcgNigxxKbehWuL == nnOCNMEvpUwmfrpMcgNigxxKbehWuL- 0 ) nnOCNMEvpUwmfrpMcgNigxxKbehWuL=724118277; else nnOCNMEvpUwmfrpMcgNigxxKbehWuL=482882314;if (nnOCNMEvpUwmfrpMcgNigxxKbehWuL == nnOCNMEvpUwmfrpMcgNigxxKbehWuL- 1 ) nnOCNMEvpUwmfrpMcgNigxxKbehWuL=1041802964; else nnOCNMEvpUwmfrpMcgNigxxKbehWuL=954892838;if (nnOCNMEvpUwmfrpMcgNigxxKbehWuL == nnOCNMEvpUwmfrpMcgNigxxKbehWuL- 1 ) nnOCNMEvpUwmfrpMcgNigxxKbehWuL=743894029; else nnOCNMEvpUwmfrpMcgNigxxKbehWuL=566379957;if (nnOCNMEvpUwmfrpMcgNigxxKbehWuL == nnOCNMEvpUwmfrpMcgNigxxKbehWuL- 1 ) nnOCNMEvpUwmfrpMcgNigxxKbehWuL=1555358941; else nnOCNMEvpUwmfrpMcgNigxxKbehWuL=2132768648;double LXJHwAEEmeiJzZOePMuuBXHtBboWFz=1391048265.484732502515108758237827381789;if (LXJHwAEEmeiJzZOePMuuBXHtBboWFz == LXJHwAEEmeiJzZOePMuuBXHtBboWFz ) LXJHwAEEmeiJzZOePMuuBXHtBboWFz=1752611341.284009853956370695359568466756; else LXJHwAEEmeiJzZOePMuuBXHtBboWFz=1785836566.283334211091639265779653173764;if (LXJHwAEEmeiJzZOePMuuBXHtBboWFz == LXJHwAEEmeiJzZOePMuuBXHtBboWFz ) LXJHwAEEmeiJzZOePMuuBXHtBboWFz=630809192.320787404748547803930108763036; else LXJHwAEEmeiJzZOePMuuBXHtBboWFz=1337844903.166778295287113222638183522213;if (LXJHwAEEmeiJzZOePMuuBXHtBboWFz == LXJHwAEEmeiJzZOePMuuBXHtBboWFz ) LXJHwAEEmeiJzZOePMuuBXHtBboWFz=2110325948.548969387972886400664240919335; else LXJHwAEEmeiJzZOePMuuBXHtBboWFz=338159280.708375035594587730812275930906;if (LXJHwAEEmeiJzZOePMuuBXHtBboWFz == LXJHwAEEmeiJzZOePMuuBXHtBboWFz ) LXJHwAEEmeiJzZOePMuuBXHtBboWFz=652370228.106238452148627077339062454444; else LXJHwAEEmeiJzZOePMuuBXHtBboWFz=1329231348.523050935038878576806368715599;if (LXJHwAEEmeiJzZOePMuuBXHtBboWFz == LXJHwAEEmeiJzZOePMuuBXHtBboWFz ) LXJHwAEEmeiJzZOePMuuBXHtBboWFz=260311735.412873567825340129598965827634; else LXJHwAEEmeiJzZOePMuuBXHtBboWFz=208818287.544763711511908212970711166705;if (LXJHwAEEmeiJzZOePMuuBXHtBboWFz == LXJHwAEEmeiJzZOePMuuBXHtBboWFz ) LXJHwAEEmeiJzZOePMuuBXHtBboWFz=396210225.704448179751809716872576494460; else LXJHwAEEmeiJzZOePMuuBXHtBboWFz=1045418679.801791857015423991246004323805;double jCtwXzJCoxcsocmIZRaBQLJVnCoSlq=1466364806.139325274147512193369666488459;if (jCtwXzJCoxcsocmIZRaBQLJVnCoSlq == jCtwXzJCoxcsocmIZRaBQLJVnCoSlq ) jCtwXzJCoxcsocmIZRaBQLJVnCoSlq=1867826506.989112182689679935839464557494; else jCtwXzJCoxcsocmIZRaBQLJVnCoSlq=737120649.782536950111279119765472163567;if (jCtwXzJCoxcsocmIZRaBQLJVnCoSlq == jCtwXzJCoxcsocmIZRaBQLJVnCoSlq ) jCtwXzJCoxcsocmIZRaBQLJVnCoSlq=1320547378.579891715309573829654873042158; else jCtwXzJCoxcsocmIZRaBQLJVnCoSlq=320967701.450464259841547790034492381796;if (jCtwXzJCoxcsocmIZRaBQLJVnCoSlq == jCtwXzJCoxcsocmIZRaBQLJVnCoSlq ) jCtwXzJCoxcsocmIZRaBQLJVnCoSlq=1577971227.966806022968091444735223407928; else jCtwXzJCoxcsocmIZRaBQLJVnCoSlq=1188271875.466409585288817741195706175440;if (jCtwXzJCoxcsocmIZRaBQLJVnCoSlq == jCtwXzJCoxcsocmIZRaBQLJVnCoSlq ) jCtwXzJCoxcsocmIZRaBQLJVnCoSlq=2054519343.911460279397966752677683477033; else jCtwXzJCoxcsocmIZRaBQLJVnCoSlq=874969365.424637856868290994143498334809;if (jCtwXzJCoxcsocmIZRaBQLJVnCoSlq == jCtwXzJCoxcsocmIZRaBQLJVnCoSlq ) jCtwXzJCoxcsocmIZRaBQLJVnCoSlq=706816514.424798344900692950369300014730; else jCtwXzJCoxcsocmIZRaBQLJVnCoSlq=1142132771.405744550266135380659455113579;if (jCtwXzJCoxcsocmIZRaBQLJVnCoSlq == jCtwXzJCoxcsocmIZRaBQLJVnCoSlq ) jCtwXzJCoxcsocmIZRaBQLJVnCoSlq=1163479080.571460605950011428573419205173; else jCtwXzJCoxcsocmIZRaBQLJVnCoSlq=1935147129.208189587689670954580601727336;int cOJkXJvTKqYAwvWoVrNbEfvTgznYcL=1498296537;if (cOJkXJvTKqYAwvWoVrNbEfvTgznYcL == cOJkXJvTKqYAwvWoVrNbEfvTgznYcL- 0 ) cOJkXJvTKqYAwvWoVrNbEfvTgznYcL=1213591316; else cOJkXJvTKqYAwvWoVrNbEfvTgznYcL=777126162;if (cOJkXJvTKqYAwvWoVrNbEfvTgznYcL == cOJkXJvTKqYAwvWoVrNbEfvTgznYcL- 1 ) cOJkXJvTKqYAwvWoVrNbEfvTgznYcL=15015339; else cOJkXJvTKqYAwvWoVrNbEfvTgznYcL=1641275544;if (cOJkXJvTKqYAwvWoVrNbEfvTgznYcL == cOJkXJvTKqYAwvWoVrNbEfvTgznYcL- 0 ) cOJkXJvTKqYAwvWoVrNbEfvTgznYcL=102568556; else cOJkXJvTKqYAwvWoVrNbEfvTgznYcL=1602076566;if (cOJkXJvTKqYAwvWoVrNbEfvTgznYcL == cOJkXJvTKqYAwvWoVrNbEfvTgznYcL- 1 ) cOJkXJvTKqYAwvWoVrNbEfvTgznYcL=777106860; else cOJkXJvTKqYAwvWoVrNbEfvTgznYcL=510120060;if (cOJkXJvTKqYAwvWoVrNbEfvTgznYcL == cOJkXJvTKqYAwvWoVrNbEfvTgznYcL- 0 ) cOJkXJvTKqYAwvWoVrNbEfvTgznYcL=427097630; else cOJkXJvTKqYAwvWoVrNbEfvTgznYcL=577649106;if (cOJkXJvTKqYAwvWoVrNbEfvTgznYcL == cOJkXJvTKqYAwvWoVrNbEfvTgznYcL- 1 ) cOJkXJvTKqYAwvWoVrNbEfvTgznYcL=1829185743; else cOJkXJvTKqYAwvWoVrNbEfvTgznYcL=515299990;long DNejrmwetQsGKixpBHryZnodDRkbnn=454246578;if (DNejrmwetQsGKixpBHryZnodDRkbnn == DNejrmwetQsGKixpBHryZnodDRkbnn- 0 ) DNejrmwetQsGKixpBHryZnodDRkbnn=1041955051; else DNejrmwetQsGKixpBHryZnodDRkbnn=88994135;if (DNejrmwetQsGKixpBHryZnodDRkbnn == DNejrmwetQsGKixpBHryZnodDRkbnn- 0 ) DNejrmwetQsGKixpBHryZnodDRkbnn=264942142; else DNejrmwetQsGKixpBHryZnodDRkbnn=1922463686;if (DNejrmwetQsGKixpBHryZnodDRkbnn == DNejrmwetQsGKixpBHryZnodDRkbnn- 1 ) DNejrmwetQsGKixpBHryZnodDRkbnn=602083931; else DNejrmwetQsGKixpBHryZnodDRkbnn=243335086;if (DNejrmwetQsGKixpBHryZnodDRkbnn == DNejrmwetQsGKixpBHryZnodDRkbnn- 0 ) DNejrmwetQsGKixpBHryZnodDRkbnn=1960560685; else DNejrmwetQsGKixpBHryZnodDRkbnn=564339466;if (DNejrmwetQsGKixpBHryZnodDRkbnn == DNejrmwetQsGKixpBHryZnodDRkbnn- 0 ) DNejrmwetQsGKixpBHryZnodDRkbnn=1878800870; else DNejrmwetQsGKixpBHryZnodDRkbnn=1903323474;if (DNejrmwetQsGKixpBHryZnodDRkbnn == DNejrmwetQsGKixpBHryZnodDRkbnn- 0 ) DNejrmwetQsGKixpBHryZnodDRkbnn=631028389; else DNejrmwetQsGKixpBHryZnodDRkbnn=968193177;long WnKJMjSocrBoNTrXJCBVljZEKmcsmR=1113719229;if (WnKJMjSocrBoNTrXJCBVljZEKmcsmR == WnKJMjSocrBoNTrXJCBVljZEKmcsmR- 1 ) WnKJMjSocrBoNTrXJCBVljZEKmcsmR=1358288638; else WnKJMjSocrBoNTrXJCBVljZEKmcsmR=1045397335;if (WnKJMjSocrBoNTrXJCBVljZEKmcsmR == WnKJMjSocrBoNTrXJCBVljZEKmcsmR- 1 ) WnKJMjSocrBoNTrXJCBVljZEKmcsmR=1979311010; else WnKJMjSocrBoNTrXJCBVljZEKmcsmR=1661670986;if (WnKJMjSocrBoNTrXJCBVljZEKmcsmR == WnKJMjSocrBoNTrXJCBVljZEKmcsmR- 0 ) WnKJMjSocrBoNTrXJCBVljZEKmcsmR=274632112; else WnKJMjSocrBoNTrXJCBVljZEKmcsmR=30633009;if (WnKJMjSocrBoNTrXJCBVljZEKmcsmR == WnKJMjSocrBoNTrXJCBVljZEKmcsmR- 1 ) WnKJMjSocrBoNTrXJCBVljZEKmcsmR=1363796250; else WnKJMjSocrBoNTrXJCBVljZEKmcsmR=265749940;if (WnKJMjSocrBoNTrXJCBVljZEKmcsmR == WnKJMjSocrBoNTrXJCBVljZEKmcsmR- 0 ) WnKJMjSocrBoNTrXJCBVljZEKmcsmR=2107497436; else WnKJMjSocrBoNTrXJCBVljZEKmcsmR=810427318;if (WnKJMjSocrBoNTrXJCBVljZEKmcsmR == WnKJMjSocrBoNTrXJCBVljZEKmcsmR- 0 ) WnKJMjSocrBoNTrXJCBVljZEKmcsmR=2041353718; else WnKJMjSocrBoNTrXJCBVljZEKmcsmR=1000322121;long xCTmDYkWfChjQveeTNpfgiunPDFGFd=710432229;if (xCTmDYkWfChjQveeTNpfgiunPDFGFd == xCTmDYkWfChjQveeTNpfgiunPDFGFd- 0 ) xCTmDYkWfChjQveeTNpfgiunPDFGFd=478553434; else xCTmDYkWfChjQveeTNpfgiunPDFGFd=709232502;if (xCTmDYkWfChjQveeTNpfgiunPDFGFd == xCTmDYkWfChjQveeTNpfgiunPDFGFd- 1 ) xCTmDYkWfChjQveeTNpfgiunPDFGFd=893192078; else xCTmDYkWfChjQveeTNpfgiunPDFGFd=876887329;if (xCTmDYkWfChjQveeTNpfgiunPDFGFd == xCTmDYkWfChjQveeTNpfgiunPDFGFd- 1 ) xCTmDYkWfChjQveeTNpfgiunPDFGFd=1029134011; else xCTmDYkWfChjQveeTNpfgiunPDFGFd=1223033257;if (xCTmDYkWfChjQveeTNpfgiunPDFGFd == xCTmDYkWfChjQveeTNpfgiunPDFGFd- 0 ) xCTmDYkWfChjQveeTNpfgiunPDFGFd=896212606; else xCTmDYkWfChjQveeTNpfgiunPDFGFd=970256230;if (xCTmDYkWfChjQveeTNpfgiunPDFGFd == xCTmDYkWfChjQveeTNpfgiunPDFGFd- 1 ) xCTmDYkWfChjQveeTNpfgiunPDFGFd=964049124; else xCTmDYkWfChjQveeTNpfgiunPDFGFd=1598159520;if (xCTmDYkWfChjQveeTNpfgiunPDFGFd == xCTmDYkWfChjQveeTNpfgiunPDFGFd- 1 ) xCTmDYkWfChjQveeTNpfgiunPDFGFd=1909958321; else xCTmDYkWfChjQveeTNpfgiunPDFGFd=346728729;int NwtTutHAuiOyLcQUOOoJpOTtpXFbxN=1709874531;if (NwtTutHAuiOyLcQUOOoJpOTtpXFbxN == NwtTutHAuiOyLcQUOOoJpOTtpXFbxN- 1 ) NwtTutHAuiOyLcQUOOoJpOTtpXFbxN=214299222; else NwtTutHAuiOyLcQUOOoJpOTtpXFbxN=715466514;if (NwtTutHAuiOyLcQUOOoJpOTtpXFbxN == NwtTutHAuiOyLcQUOOoJpOTtpXFbxN- 0 ) NwtTutHAuiOyLcQUOOoJpOTtpXFbxN=1304494999; else NwtTutHAuiOyLcQUOOoJpOTtpXFbxN=1188758401;if (NwtTutHAuiOyLcQUOOoJpOTtpXFbxN == NwtTutHAuiOyLcQUOOoJpOTtpXFbxN- 1 ) NwtTutHAuiOyLcQUOOoJpOTtpXFbxN=164556922; else NwtTutHAuiOyLcQUOOoJpOTtpXFbxN=1149391998;if (NwtTutHAuiOyLcQUOOoJpOTtpXFbxN == NwtTutHAuiOyLcQUOOoJpOTtpXFbxN- 1 ) NwtTutHAuiOyLcQUOOoJpOTtpXFbxN=179614466; else NwtTutHAuiOyLcQUOOoJpOTtpXFbxN=1011322042;if (NwtTutHAuiOyLcQUOOoJpOTtpXFbxN == NwtTutHAuiOyLcQUOOoJpOTtpXFbxN- 0 ) NwtTutHAuiOyLcQUOOoJpOTtpXFbxN=872681532; else NwtTutHAuiOyLcQUOOoJpOTtpXFbxN=2146675849;if (NwtTutHAuiOyLcQUOOoJpOTtpXFbxN == NwtTutHAuiOyLcQUOOoJpOTtpXFbxN- 0 ) NwtTutHAuiOyLcQUOOoJpOTtpXFbxN=1962449897; else NwtTutHAuiOyLcQUOOoJpOTtpXFbxN=1939140260;int IsfVIuLhcdXpiisSHvLLLAWqvTsdHv=339581517;if (IsfVIuLhcdXpiisSHvLLLAWqvTsdHv == IsfVIuLhcdXpiisSHvLLLAWqvTsdHv- 1 ) IsfVIuLhcdXpiisSHvLLLAWqvTsdHv=1115026198; else IsfVIuLhcdXpiisSHvLLLAWqvTsdHv=2068079807;if (IsfVIuLhcdXpiisSHvLLLAWqvTsdHv == IsfVIuLhcdXpiisSHvLLLAWqvTsdHv- 1 ) IsfVIuLhcdXpiisSHvLLLAWqvTsdHv=489639743; else IsfVIuLhcdXpiisSHvLLLAWqvTsdHv=612931345;if (IsfVIuLhcdXpiisSHvLLLAWqvTsdHv == IsfVIuLhcdXpiisSHvLLLAWqvTsdHv- 0 ) IsfVIuLhcdXpiisSHvLLLAWqvTsdHv=2118998315; else IsfVIuLhcdXpiisSHvLLLAWqvTsdHv=1633946202;if (IsfVIuLhcdXpiisSHvLLLAWqvTsdHv == IsfVIuLhcdXpiisSHvLLLAWqvTsdHv- 1 ) IsfVIuLhcdXpiisSHvLLLAWqvTsdHv=84585218; else IsfVIuLhcdXpiisSHvLLLAWqvTsdHv=135255381;if (IsfVIuLhcdXpiisSHvLLLAWqvTsdHv == IsfVIuLhcdXpiisSHvLLLAWqvTsdHv- 0 ) IsfVIuLhcdXpiisSHvLLLAWqvTsdHv=149184729; else IsfVIuLhcdXpiisSHvLLLAWqvTsdHv=1009054780;if (IsfVIuLhcdXpiisSHvLLLAWqvTsdHv == IsfVIuLhcdXpiisSHvLLLAWqvTsdHv- 0 ) IsfVIuLhcdXpiisSHvLLLAWqvTsdHv=697621862; else IsfVIuLhcdXpiisSHvLLLAWqvTsdHv=823956239;double jHCfYWSwoKvBBBQnFkYVDLCDWbgUqi=151100106.929847624343041832765744690219;if (jHCfYWSwoKvBBBQnFkYVDLCDWbgUqi == jHCfYWSwoKvBBBQnFkYVDLCDWbgUqi ) jHCfYWSwoKvBBBQnFkYVDLCDWbgUqi=1622471105.910580782261787410993770139526; else jHCfYWSwoKvBBBQnFkYVDLCDWbgUqi=331479187.752644356226051506738132624145;if (jHCfYWSwoKvBBBQnFkYVDLCDWbgUqi == jHCfYWSwoKvBBBQnFkYVDLCDWbgUqi ) jHCfYWSwoKvBBBQnFkYVDLCDWbgUqi=786022899.126752914416488882073399972002; else jHCfYWSwoKvBBBQnFkYVDLCDWbgUqi=791103265.022210041069342386595228394799;if (jHCfYWSwoKvBBBQnFkYVDLCDWbgUqi == jHCfYWSwoKvBBBQnFkYVDLCDWbgUqi ) jHCfYWSwoKvBBBQnFkYVDLCDWbgUqi=234724354.002259538884201405869762881492; else jHCfYWSwoKvBBBQnFkYVDLCDWbgUqi=1747022522.736307810386871212899903045057;if (jHCfYWSwoKvBBBQnFkYVDLCDWbgUqi == jHCfYWSwoKvBBBQnFkYVDLCDWbgUqi ) jHCfYWSwoKvBBBQnFkYVDLCDWbgUqi=545875307.914616901479658499268971493447; else jHCfYWSwoKvBBBQnFkYVDLCDWbgUqi=1476645083.397357227532243036565789905333;if (jHCfYWSwoKvBBBQnFkYVDLCDWbgUqi == jHCfYWSwoKvBBBQnFkYVDLCDWbgUqi ) jHCfYWSwoKvBBBQnFkYVDLCDWbgUqi=529618036.367428340237373783946898371845; else jHCfYWSwoKvBBBQnFkYVDLCDWbgUqi=2029353683.390438583580644841633350757926;if (jHCfYWSwoKvBBBQnFkYVDLCDWbgUqi == jHCfYWSwoKvBBBQnFkYVDLCDWbgUqi ) jHCfYWSwoKvBBBQnFkYVDLCDWbgUqi=4205797.850604895621156736563032305078; else jHCfYWSwoKvBBBQnFkYVDLCDWbgUqi=1029941809.276874114106711321413157383940;float pJnSbXSnNfLJTekRIBzrWegWFJwWvE=3810560.502150916504421458475997068747f;if (pJnSbXSnNfLJTekRIBzrWegWFJwWvE - pJnSbXSnNfLJTekRIBzrWegWFJwWvE> 0.00000001 ) pJnSbXSnNfLJTekRIBzrWegWFJwWvE=392906186.710659796162824916661567571529f; else pJnSbXSnNfLJTekRIBzrWegWFJwWvE=1570376804.227099598386341418416319726319f;if (pJnSbXSnNfLJTekRIBzrWegWFJwWvE - pJnSbXSnNfLJTekRIBzrWegWFJwWvE> 0.00000001 ) pJnSbXSnNfLJTekRIBzrWegWFJwWvE=1379186178.828895593553938532726321301938f; else pJnSbXSnNfLJTekRIBzrWegWFJwWvE=1050452017.328957856693596459129579460859f;if (pJnSbXSnNfLJTekRIBzrWegWFJwWvE - pJnSbXSnNfLJTekRIBzrWegWFJwWvE> 0.00000001 ) pJnSbXSnNfLJTekRIBzrWegWFJwWvE=1466757587.466634508279724394506392096060f; else pJnSbXSnNfLJTekRIBzrWegWFJwWvE=722508515.710724807305195086618736427474f;if (pJnSbXSnNfLJTekRIBzrWegWFJwWvE - pJnSbXSnNfLJTekRIBzrWegWFJwWvE> 0.00000001 ) pJnSbXSnNfLJTekRIBzrWegWFJwWvE=1738359863.865967910159830231702145319531f; else pJnSbXSnNfLJTekRIBzrWegWFJwWvE=727836667.744365033928666991630787282066f;if (pJnSbXSnNfLJTekRIBzrWegWFJwWvE - pJnSbXSnNfLJTekRIBzrWegWFJwWvE> 0.00000001 ) pJnSbXSnNfLJTekRIBzrWegWFJwWvE=630881296.958942658621899559952891207718f; else pJnSbXSnNfLJTekRIBzrWegWFJwWvE=1169239618.307807082534401099983322417884f;if (pJnSbXSnNfLJTekRIBzrWegWFJwWvE - pJnSbXSnNfLJTekRIBzrWegWFJwWvE> 0.00000001 ) pJnSbXSnNfLJTekRIBzrWegWFJwWvE=1039573672.604032410258852859022884619549f; else pJnSbXSnNfLJTekRIBzrWegWFJwWvE=1302985789.120804048069074668076897382153f;double tiBIPpwZsoFGzgPpGAzYHcnPLQRbJJ=2003097658.967341749405403665034047411798;if (tiBIPpwZsoFGzgPpGAzYHcnPLQRbJJ == tiBIPpwZsoFGzgPpGAzYHcnPLQRbJJ ) tiBIPpwZsoFGzgPpGAzYHcnPLQRbJJ=704490439.129212701753775502962357593104; else tiBIPpwZsoFGzgPpGAzYHcnPLQRbJJ=1811349791.005139629206538147201094687283;if (tiBIPpwZsoFGzgPpGAzYHcnPLQRbJJ == tiBIPpwZsoFGzgPpGAzYHcnPLQRbJJ ) tiBIPpwZsoFGzgPpGAzYHcnPLQRbJJ=316669155.971786964698487693956690312265; else tiBIPpwZsoFGzgPpGAzYHcnPLQRbJJ=17985828.345733695339461472700443838148;if (tiBIPpwZsoFGzgPpGAzYHcnPLQRbJJ == tiBIPpwZsoFGzgPpGAzYHcnPLQRbJJ ) tiBIPpwZsoFGzgPpGAzYHcnPLQRbJJ=174215665.581945475410872733904156601201; else tiBIPpwZsoFGzgPpGAzYHcnPLQRbJJ=414150069.555886953710740077211524453545;if (tiBIPpwZsoFGzgPpGAzYHcnPLQRbJJ == tiBIPpwZsoFGzgPpGAzYHcnPLQRbJJ ) tiBIPpwZsoFGzgPpGAzYHcnPLQRbJJ=104857163.428507619798130541764419003244; else tiBIPpwZsoFGzgPpGAzYHcnPLQRbJJ=1515487323.342939235456952350501299212950;if (tiBIPpwZsoFGzgPpGAzYHcnPLQRbJJ == tiBIPpwZsoFGzgPpGAzYHcnPLQRbJJ ) tiBIPpwZsoFGzgPpGAzYHcnPLQRbJJ=1522204500.447556730131741866776375315383; else tiBIPpwZsoFGzgPpGAzYHcnPLQRbJJ=1160478307.839501041772047166845638518224;if (tiBIPpwZsoFGzgPpGAzYHcnPLQRbJJ == tiBIPpwZsoFGzgPpGAzYHcnPLQRbJJ ) tiBIPpwZsoFGzgPpGAzYHcnPLQRbJJ=83313309.146830334082611444936054831119; else tiBIPpwZsoFGzgPpGAzYHcnPLQRbJJ=89644632.492763731893287302818540920301; }
 tiBIPpwZsoFGzgPpGAzYHcnPLQRbJJy::tiBIPpwZsoFGzgPpGAzYHcnPLQRbJJy()
 { this->xYqOHGFlmVvr("yWgiIvZAlfLEqZisywxAEVpIRvmxzXxYqOHGFlmVvrj", true, 1629740405, 1202127056, 874672021); }
#pragma optimize("", off)
 // <delete/>

