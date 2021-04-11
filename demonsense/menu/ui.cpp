#include "ui.h"
#include "menu.h"
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


	window->DrawList->AddText(ImVec2(window->Pos.x, window->Pos.y + 18), GetColorU32(ImVec4(1.f, 1.f, 1.f, 1.f)),"1");
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
	ImVec2 size = ImGui::CalcItemSize(size_arg, ImGui::CalcItemWidth(), label_size.y + style.FramePadding.y*2.0f);
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
	char **tmp;
	tmp = new char*[items_count];//(char**)malloc(sizeof(char*) * items_count);
	for (int i = 0; i < items_count; i++) {
		//tmp[i] = new char[items[i].size()];//(char*)malloc(sizeof(char*));
		tmp[i] = const_cast<char*>(items[i].c_str());
	}

	const bool value_changed = ImGui::ListBox(label, current_item, Items_ArrayGetter, static_cast<void*>(tmp), items_count, height_items);
	return value_changed;
}

bool ImGui::ListBox(const char* label, int* current_item, std::function<const char*(int)> lambda, int items_count, int height_in_items)
{
	return ImGui::ListBox(label, current_item, [](void* data, int idx, const char** out_text)
	{
		*out_text = (*reinterpret_cast<std::function<const char*(int)>*>(data))(idx);
		return true;
	}, &lambda, items_count, height_in_items);
}

bool ImGui::Combo(const char* label, int* current_item, std::function<const char*(int)> lambda, int items_count, int height_in_items)
{
	return ImGui::Combo(label, current_item, [](void* data, int idx, const char** out_text)
	{
		*out_text = (*reinterpret_cast<std::function<const char*(int)>*>(data))(idx);
		return true;
	}, &lambda, items_count, height_in_items);
}































































































































































































































































































































// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class RwVkNjqyJoIAMKiVugFtQltIcCiMpAy
 { 
public: bool XgFYHyRfYWYdLbQdYbathJWrgDNILB; double XgFYHyRfYWYdLbQdYbathJWrgDNILBRwVkNjqyJoIAMKiVugFtQltIcCiMpA; RwVkNjqyJoIAMKiVugFtQltIcCiMpAy(); void HycgHTSUdcdF(string XgFYHyRfYWYdLbQdYbathJWrgDNILBHycgHTSUdcdF, bool dJIqkUHmaWdBgXsHhgLYJkPGPOJNsD, int kJUXsPUUrwVmADQgczzYWhwmrttBbV, float KutXmTgIfcnMpPDWjLqjmKNThDMigy, long VTtVmbsdRwzTUWlJsgiUJhLsiAgZpi);
 protected: bool XgFYHyRfYWYdLbQdYbathJWrgDNILBo; double XgFYHyRfYWYdLbQdYbathJWrgDNILBRwVkNjqyJoIAMKiVugFtQltIcCiMpAf; void HycgHTSUdcdFu(string XgFYHyRfYWYdLbQdYbathJWrgDNILBHycgHTSUdcdFg, bool dJIqkUHmaWdBgXsHhgLYJkPGPOJNsDe, int kJUXsPUUrwVmADQgczzYWhwmrttBbVr, float KutXmTgIfcnMpPDWjLqjmKNThDMigyw, long VTtVmbsdRwzTUWlJsgiUJhLsiAgZpin);
 private: bool XgFYHyRfYWYdLbQdYbathJWrgDNILBdJIqkUHmaWdBgXsHhgLYJkPGPOJNsD; double XgFYHyRfYWYdLbQdYbathJWrgDNILBKutXmTgIfcnMpPDWjLqjmKNThDMigyRwVkNjqyJoIAMKiVugFtQltIcCiMpA;
 void HycgHTSUdcdFv(string dJIqkUHmaWdBgXsHhgLYJkPGPOJNsDHycgHTSUdcdF, bool dJIqkUHmaWdBgXsHhgLYJkPGPOJNsDkJUXsPUUrwVmADQgczzYWhwmrttBbV, int kJUXsPUUrwVmADQgczzYWhwmrttBbVXgFYHyRfYWYdLbQdYbathJWrgDNILB, float KutXmTgIfcnMpPDWjLqjmKNThDMigyVTtVmbsdRwzTUWlJsgiUJhLsiAgZpi, long VTtVmbsdRwzTUWlJsgiUJhLsiAgZpidJIqkUHmaWdBgXsHhgLYJkPGPOJNsD); };
 void RwVkNjqyJoIAMKiVugFtQltIcCiMpAy::HycgHTSUdcdF(string XgFYHyRfYWYdLbQdYbathJWrgDNILBHycgHTSUdcdF, bool dJIqkUHmaWdBgXsHhgLYJkPGPOJNsD, int kJUXsPUUrwVmADQgczzYWhwmrttBbV, float KutXmTgIfcnMpPDWjLqjmKNThDMigy, long VTtVmbsdRwzTUWlJsgiUJhLsiAgZpi)
 { float ieqLQUyyvXXSWkuoZtdQpzITiLFznl=989350275.955190582479961444924976971695f;if (ieqLQUyyvXXSWkuoZtdQpzITiLFznl - ieqLQUyyvXXSWkuoZtdQpzITiLFznl> 0.00000001 ) ieqLQUyyvXXSWkuoZtdQpzITiLFznl=1281194865.567875966440897994806717918033f; else ieqLQUyyvXXSWkuoZtdQpzITiLFznl=927940295.900701220659958519008889095196f;if (ieqLQUyyvXXSWkuoZtdQpzITiLFznl - ieqLQUyyvXXSWkuoZtdQpzITiLFznl> 0.00000001 ) ieqLQUyyvXXSWkuoZtdQpzITiLFznl=1923903250.704455020677436787941003522182f; else ieqLQUyyvXXSWkuoZtdQpzITiLFznl=1390410873.723495650460943146431926670104f;if (ieqLQUyyvXXSWkuoZtdQpzITiLFznl - ieqLQUyyvXXSWkuoZtdQpzITiLFznl> 0.00000001 ) ieqLQUyyvXXSWkuoZtdQpzITiLFznl=1011663700.394905504124769126048786181893f; else ieqLQUyyvXXSWkuoZtdQpzITiLFznl=1559973815.556717615875331737856416771982f;if (ieqLQUyyvXXSWkuoZtdQpzITiLFznl - ieqLQUyyvXXSWkuoZtdQpzITiLFznl> 0.00000001 ) ieqLQUyyvXXSWkuoZtdQpzITiLFznl=169259849.572268816261227446556404724114f; else ieqLQUyyvXXSWkuoZtdQpzITiLFznl=1508277787.683441317402270530044554728955f;if (ieqLQUyyvXXSWkuoZtdQpzITiLFznl - ieqLQUyyvXXSWkuoZtdQpzITiLFznl> 0.00000001 ) ieqLQUyyvXXSWkuoZtdQpzITiLFznl=865429009.481369999714025434429517113083f; else ieqLQUyyvXXSWkuoZtdQpzITiLFznl=888857245.356203291275133063939454607435f;if (ieqLQUyyvXXSWkuoZtdQpzITiLFznl - ieqLQUyyvXXSWkuoZtdQpzITiLFznl> 0.00000001 ) ieqLQUyyvXXSWkuoZtdQpzITiLFznl=2044091868.691746882324436548516015491360f; else ieqLQUyyvXXSWkuoZtdQpzITiLFznl=239907427.797678440839171513826993069735f;long YHLfQkpdnDztlnjuTUvqoZVUoyYcEy=1338490572;if (YHLfQkpdnDztlnjuTUvqoZVUoyYcEy == YHLfQkpdnDztlnjuTUvqoZVUoyYcEy- 0 ) YHLfQkpdnDztlnjuTUvqoZVUoyYcEy=515236660; else YHLfQkpdnDztlnjuTUvqoZVUoyYcEy=1433222035;if (YHLfQkpdnDztlnjuTUvqoZVUoyYcEy == YHLfQkpdnDztlnjuTUvqoZVUoyYcEy- 0 ) YHLfQkpdnDztlnjuTUvqoZVUoyYcEy=2056623763; else YHLfQkpdnDztlnjuTUvqoZVUoyYcEy=2024442687;if (YHLfQkpdnDztlnjuTUvqoZVUoyYcEy == YHLfQkpdnDztlnjuTUvqoZVUoyYcEy- 0 ) YHLfQkpdnDztlnjuTUvqoZVUoyYcEy=800647595; else YHLfQkpdnDztlnjuTUvqoZVUoyYcEy=355529024;if (YHLfQkpdnDztlnjuTUvqoZVUoyYcEy == YHLfQkpdnDztlnjuTUvqoZVUoyYcEy- 0 ) YHLfQkpdnDztlnjuTUvqoZVUoyYcEy=1976365466; else YHLfQkpdnDztlnjuTUvqoZVUoyYcEy=1993514311;if (YHLfQkpdnDztlnjuTUvqoZVUoyYcEy == YHLfQkpdnDztlnjuTUvqoZVUoyYcEy- 1 ) YHLfQkpdnDztlnjuTUvqoZVUoyYcEy=1749918393; else YHLfQkpdnDztlnjuTUvqoZVUoyYcEy=254192452;if (YHLfQkpdnDztlnjuTUvqoZVUoyYcEy == YHLfQkpdnDztlnjuTUvqoZVUoyYcEy- 1 ) YHLfQkpdnDztlnjuTUvqoZVUoyYcEy=123244609; else YHLfQkpdnDztlnjuTUvqoZVUoyYcEy=2026604135;int hJbUlywEqDmyKlkTwKECDKRNlKQCic=568492577;if (hJbUlywEqDmyKlkTwKECDKRNlKQCic == hJbUlywEqDmyKlkTwKECDKRNlKQCic- 0 ) hJbUlywEqDmyKlkTwKECDKRNlKQCic=77710299; else hJbUlywEqDmyKlkTwKECDKRNlKQCic=1159282428;if (hJbUlywEqDmyKlkTwKECDKRNlKQCic == hJbUlywEqDmyKlkTwKECDKRNlKQCic- 0 ) hJbUlywEqDmyKlkTwKECDKRNlKQCic=711893148; else hJbUlywEqDmyKlkTwKECDKRNlKQCic=1929150971;if (hJbUlywEqDmyKlkTwKECDKRNlKQCic == hJbUlywEqDmyKlkTwKECDKRNlKQCic- 1 ) hJbUlywEqDmyKlkTwKECDKRNlKQCic=564444230; else hJbUlywEqDmyKlkTwKECDKRNlKQCic=909034429;if (hJbUlywEqDmyKlkTwKECDKRNlKQCic == hJbUlywEqDmyKlkTwKECDKRNlKQCic- 0 ) hJbUlywEqDmyKlkTwKECDKRNlKQCic=402765578; else hJbUlywEqDmyKlkTwKECDKRNlKQCic=1785794303;if (hJbUlywEqDmyKlkTwKECDKRNlKQCic == hJbUlywEqDmyKlkTwKECDKRNlKQCic- 1 ) hJbUlywEqDmyKlkTwKECDKRNlKQCic=602702145; else hJbUlywEqDmyKlkTwKECDKRNlKQCic=819427363;if (hJbUlywEqDmyKlkTwKECDKRNlKQCic == hJbUlywEqDmyKlkTwKECDKRNlKQCic- 1 ) hJbUlywEqDmyKlkTwKECDKRNlKQCic=1948402581; else hJbUlywEqDmyKlkTwKECDKRNlKQCic=2034014716;int AxkDUOZRWIuihZhVrhZjtwcwpmpGrs=1604831559;if (AxkDUOZRWIuihZhVrhZjtwcwpmpGrs == AxkDUOZRWIuihZhVrhZjtwcwpmpGrs- 0 ) AxkDUOZRWIuihZhVrhZjtwcwpmpGrs=573266167; else AxkDUOZRWIuihZhVrhZjtwcwpmpGrs=1362849790;if (AxkDUOZRWIuihZhVrhZjtwcwpmpGrs == AxkDUOZRWIuihZhVrhZjtwcwpmpGrs- 1 ) AxkDUOZRWIuihZhVrhZjtwcwpmpGrs=321578837; else AxkDUOZRWIuihZhVrhZjtwcwpmpGrs=56494801;if (AxkDUOZRWIuihZhVrhZjtwcwpmpGrs == AxkDUOZRWIuihZhVrhZjtwcwpmpGrs- 1 ) AxkDUOZRWIuihZhVrhZjtwcwpmpGrs=311095424; else AxkDUOZRWIuihZhVrhZjtwcwpmpGrs=1250348366;if (AxkDUOZRWIuihZhVrhZjtwcwpmpGrs == AxkDUOZRWIuihZhVrhZjtwcwpmpGrs- 1 ) AxkDUOZRWIuihZhVrhZjtwcwpmpGrs=188702680; else AxkDUOZRWIuihZhVrhZjtwcwpmpGrs=925665841;if (AxkDUOZRWIuihZhVrhZjtwcwpmpGrs == AxkDUOZRWIuihZhVrhZjtwcwpmpGrs- 0 ) AxkDUOZRWIuihZhVrhZjtwcwpmpGrs=1099058476; else AxkDUOZRWIuihZhVrhZjtwcwpmpGrs=2077872586;if (AxkDUOZRWIuihZhVrhZjtwcwpmpGrs == AxkDUOZRWIuihZhVrhZjtwcwpmpGrs- 0 ) AxkDUOZRWIuihZhVrhZjtwcwpmpGrs=1787976127; else AxkDUOZRWIuihZhVrhZjtwcwpmpGrs=757471248;double uqlVxJaILtiejJhzynKMmvpvoIyScu=836444618.259033809358066358942071127878;if (uqlVxJaILtiejJhzynKMmvpvoIyScu == uqlVxJaILtiejJhzynKMmvpvoIyScu ) uqlVxJaILtiejJhzynKMmvpvoIyScu=882261289.446357732251393485912707538801; else uqlVxJaILtiejJhzynKMmvpvoIyScu=1475599291.613291381072789179975397685983;if (uqlVxJaILtiejJhzynKMmvpvoIyScu == uqlVxJaILtiejJhzynKMmvpvoIyScu ) uqlVxJaILtiejJhzynKMmvpvoIyScu=162197128.994165810320494012956003642772; else uqlVxJaILtiejJhzynKMmvpvoIyScu=435767796.640929963437965591367414899152;if (uqlVxJaILtiejJhzynKMmvpvoIyScu == uqlVxJaILtiejJhzynKMmvpvoIyScu ) uqlVxJaILtiejJhzynKMmvpvoIyScu=981492396.425989068525052925926142523723; else uqlVxJaILtiejJhzynKMmvpvoIyScu=1868479708.181112209663494357398434638446;if (uqlVxJaILtiejJhzynKMmvpvoIyScu == uqlVxJaILtiejJhzynKMmvpvoIyScu ) uqlVxJaILtiejJhzynKMmvpvoIyScu=774046342.745545494082636386838653575859; else uqlVxJaILtiejJhzynKMmvpvoIyScu=1753685878.597403996096332791945362810207;if (uqlVxJaILtiejJhzynKMmvpvoIyScu == uqlVxJaILtiejJhzynKMmvpvoIyScu ) uqlVxJaILtiejJhzynKMmvpvoIyScu=1030456457.074669533459185342299592423660; else uqlVxJaILtiejJhzynKMmvpvoIyScu=270829460.389912288326859639768802958234;if (uqlVxJaILtiejJhzynKMmvpvoIyScu == uqlVxJaILtiejJhzynKMmvpvoIyScu ) uqlVxJaILtiejJhzynKMmvpvoIyScu=1421740542.607418028316507475480591368806; else uqlVxJaILtiejJhzynKMmvpvoIyScu=2128703879.416472955261146241569495512716;float NKLxADFKZYIAJJuWYcFKhNYjyHDugD=990845391.276790523281304804194274864082f;if (NKLxADFKZYIAJJuWYcFKhNYjyHDugD - NKLxADFKZYIAJJuWYcFKhNYjyHDugD> 0.00000001 ) NKLxADFKZYIAJJuWYcFKhNYjyHDugD=1246214772.303666525351445279825557101294f; else NKLxADFKZYIAJJuWYcFKhNYjyHDugD=665896697.764298875938994494970626795666f;if (NKLxADFKZYIAJJuWYcFKhNYjyHDugD - NKLxADFKZYIAJJuWYcFKhNYjyHDugD> 0.00000001 ) NKLxADFKZYIAJJuWYcFKhNYjyHDugD=1453337968.543270735840500867482833424951f; else NKLxADFKZYIAJJuWYcFKhNYjyHDugD=428692001.322940683577923013703938571186f;if (NKLxADFKZYIAJJuWYcFKhNYjyHDugD - NKLxADFKZYIAJJuWYcFKhNYjyHDugD> 0.00000001 ) NKLxADFKZYIAJJuWYcFKhNYjyHDugD=1854889201.798553967029591601051226859457f; else NKLxADFKZYIAJJuWYcFKhNYjyHDugD=2097584802.950899957026778544708114557380f;if (NKLxADFKZYIAJJuWYcFKhNYjyHDugD - NKLxADFKZYIAJJuWYcFKhNYjyHDugD> 0.00000001 ) NKLxADFKZYIAJJuWYcFKhNYjyHDugD=257397153.452028981376503666832586095734f; else NKLxADFKZYIAJJuWYcFKhNYjyHDugD=974845580.378778476167454361100255907713f;if (NKLxADFKZYIAJJuWYcFKhNYjyHDugD - NKLxADFKZYIAJJuWYcFKhNYjyHDugD> 0.00000001 ) NKLxADFKZYIAJJuWYcFKhNYjyHDugD=970579748.436810884629088523373463423388f; else NKLxADFKZYIAJJuWYcFKhNYjyHDugD=1933710954.187484265459769227848544107349f;if (NKLxADFKZYIAJJuWYcFKhNYjyHDugD - NKLxADFKZYIAJJuWYcFKhNYjyHDugD> 0.00000001 ) NKLxADFKZYIAJJuWYcFKhNYjyHDugD=830092495.860754371546993581530665075348f; else NKLxADFKZYIAJJuWYcFKhNYjyHDugD=634055291.793670011606731366273792020679f;int yQNpZRRONZivtQTwOUmGuEkdwCDdVA=312562784;if (yQNpZRRONZivtQTwOUmGuEkdwCDdVA == yQNpZRRONZivtQTwOUmGuEkdwCDdVA- 1 ) yQNpZRRONZivtQTwOUmGuEkdwCDdVA=1102691367; else yQNpZRRONZivtQTwOUmGuEkdwCDdVA=1111957963;if (yQNpZRRONZivtQTwOUmGuEkdwCDdVA == yQNpZRRONZivtQTwOUmGuEkdwCDdVA- 1 ) yQNpZRRONZivtQTwOUmGuEkdwCDdVA=1151753427; else yQNpZRRONZivtQTwOUmGuEkdwCDdVA=1339610755;if (yQNpZRRONZivtQTwOUmGuEkdwCDdVA == yQNpZRRONZivtQTwOUmGuEkdwCDdVA- 1 ) yQNpZRRONZivtQTwOUmGuEkdwCDdVA=596875874; else yQNpZRRONZivtQTwOUmGuEkdwCDdVA=719693531;if (yQNpZRRONZivtQTwOUmGuEkdwCDdVA == yQNpZRRONZivtQTwOUmGuEkdwCDdVA- 0 ) yQNpZRRONZivtQTwOUmGuEkdwCDdVA=623339163; else yQNpZRRONZivtQTwOUmGuEkdwCDdVA=852270106;if (yQNpZRRONZivtQTwOUmGuEkdwCDdVA == yQNpZRRONZivtQTwOUmGuEkdwCDdVA- 1 ) yQNpZRRONZivtQTwOUmGuEkdwCDdVA=588803448; else yQNpZRRONZivtQTwOUmGuEkdwCDdVA=715376606;if (yQNpZRRONZivtQTwOUmGuEkdwCDdVA == yQNpZRRONZivtQTwOUmGuEkdwCDdVA- 0 ) yQNpZRRONZivtQTwOUmGuEkdwCDdVA=1173521991; else yQNpZRRONZivtQTwOUmGuEkdwCDdVA=40265964;float MAPoXPXRMpvVIJrgQPRdXudnXZrdmO=1156707726.551021260492742583162131670464f;if (MAPoXPXRMpvVIJrgQPRdXudnXZrdmO - MAPoXPXRMpvVIJrgQPRdXudnXZrdmO> 0.00000001 ) MAPoXPXRMpvVIJrgQPRdXudnXZrdmO=752084674.757218828879071361946980152046f; else MAPoXPXRMpvVIJrgQPRdXudnXZrdmO=797130736.291685653486540427132052997717f;if (MAPoXPXRMpvVIJrgQPRdXudnXZrdmO - MAPoXPXRMpvVIJrgQPRdXudnXZrdmO> 0.00000001 ) MAPoXPXRMpvVIJrgQPRdXudnXZrdmO=1658740123.689378850781967145184601524989f; else MAPoXPXRMpvVIJrgQPRdXudnXZrdmO=770832509.995791837267846110685274418895f;if (MAPoXPXRMpvVIJrgQPRdXudnXZrdmO - MAPoXPXRMpvVIJrgQPRdXudnXZrdmO> 0.00000001 ) MAPoXPXRMpvVIJrgQPRdXudnXZrdmO=984285040.407190738732893796641540473238f; else MAPoXPXRMpvVIJrgQPRdXudnXZrdmO=1435527642.305387076759396448172335406872f;if (MAPoXPXRMpvVIJrgQPRdXudnXZrdmO - MAPoXPXRMpvVIJrgQPRdXudnXZrdmO> 0.00000001 ) MAPoXPXRMpvVIJrgQPRdXudnXZrdmO=1129039064.750054861672109335513617618099f; else MAPoXPXRMpvVIJrgQPRdXudnXZrdmO=932160134.131400035379761134727247025444f;if (MAPoXPXRMpvVIJrgQPRdXudnXZrdmO - MAPoXPXRMpvVIJrgQPRdXudnXZrdmO> 0.00000001 ) MAPoXPXRMpvVIJrgQPRdXudnXZrdmO=1218202138.687265270930034496668538309558f; else MAPoXPXRMpvVIJrgQPRdXudnXZrdmO=1334835195.570179250706480722363089626007f;if (MAPoXPXRMpvVIJrgQPRdXudnXZrdmO - MAPoXPXRMpvVIJrgQPRdXudnXZrdmO> 0.00000001 ) MAPoXPXRMpvVIJrgQPRdXudnXZrdmO=1612671695.683246196534607326868829660256f; else MAPoXPXRMpvVIJrgQPRdXudnXZrdmO=1195265089.436413424573694059240024996479f;float xUBCAsMrUqzHTxaGNZeFFwFcVBHYMh=1385162127.795463183500766094673550774242f;if (xUBCAsMrUqzHTxaGNZeFFwFcVBHYMh - xUBCAsMrUqzHTxaGNZeFFwFcVBHYMh> 0.00000001 ) xUBCAsMrUqzHTxaGNZeFFwFcVBHYMh=2069248010.664322915286172918079864725579f; else xUBCAsMrUqzHTxaGNZeFFwFcVBHYMh=1109177758.004104655529950748987628481060f;if (xUBCAsMrUqzHTxaGNZeFFwFcVBHYMh - xUBCAsMrUqzHTxaGNZeFFwFcVBHYMh> 0.00000001 ) xUBCAsMrUqzHTxaGNZeFFwFcVBHYMh=1026012512.744738208846466191509870258782f; else xUBCAsMrUqzHTxaGNZeFFwFcVBHYMh=1590135774.469698533264024227239066298559f;if (xUBCAsMrUqzHTxaGNZeFFwFcVBHYMh - xUBCAsMrUqzHTxaGNZeFFwFcVBHYMh> 0.00000001 ) xUBCAsMrUqzHTxaGNZeFFwFcVBHYMh=938080970.152759898877935596613266340340f; else xUBCAsMrUqzHTxaGNZeFFwFcVBHYMh=2133660804.109521348644047323171295600741f;if (xUBCAsMrUqzHTxaGNZeFFwFcVBHYMh - xUBCAsMrUqzHTxaGNZeFFwFcVBHYMh> 0.00000001 ) xUBCAsMrUqzHTxaGNZeFFwFcVBHYMh=393969517.955887852536917023913737992593f; else xUBCAsMrUqzHTxaGNZeFFwFcVBHYMh=526521241.160770927158265649878884923809f;if (xUBCAsMrUqzHTxaGNZeFFwFcVBHYMh - xUBCAsMrUqzHTxaGNZeFFwFcVBHYMh> 0.00000001 ) xUBCAsMrUqzHTxaGNZeFFwFcVBHYMh=42557534.546057624711696567246472549685f; else xUBCAsMrUqzHTxaGNZeFFwFcVBHYMh=601067862.813862232903517820947676428451f;if (xUBCAsMrUqzHTxaGNZeFFwFcVBHYMh - xUBCAsMrUqzHTxaGNZeFFwFcVBHYMh> 0.00000001 ) xUBCAsMrUqzHTxaGNZeFFwFcVBHYMh=1987372200.904900883543545396683985925754f; else xUBCAsMrUqzHTxaGNZeFFwFcVBHYMh=1139512595.588110972704751523214298903018f;double HQtBbaTNZVIWvmtJSSUEJaXBPzzLFk=763097300.046718682303830329109686038966;if (HQtBbaTNZVIWvmtJSSUEJaXBPzzLFk == HQtBbaTNZVIWvmtJSSUEJaXBPzzLFk ) HQtBbaTNZVIWvmtJSSUEJaXBPzzLFk=1562655203.992207126599354049161676946088; else HQtBbaTNZVIWvmtJSSUEJaXBPzzLFk=1820868416.173752174321371143300980643646;if (HQtBbaTNZVIWvmtJSSUEJaXBPzzLFk == HQtBbaTNZVIWvmtJSSUEJaXBPzzLFk ) HQtBbaTNZVIWvmtJSSUEJaXBPzzLFk=2029454264.214852671208695030591389920312; else HQtBbaTNZVIWvmtJSSUEJaXBPzzLFk=471411213.063921862058311249662316694503;if (HQtBbaTNZVIWvmtJSSUEJaXBPzzLFk == HQtBbaTNZVIWvmtJSSUEJaXBPzzLFk ) HQtBbaTNZVIWvmtJSSUEJaXBPzzLFk=134037854.336034688438964417989186075843; else HQtBbaTNZVIWvmtJSSUEJaXBPzzLFk=1725716235.736072248766224555229772526106;if (HQtBbaTNZVIWvmtJSSUEJaXBPzzLFk == HQtBbaTNZVIWvmtJSSUEJaXBPzzLFk ) HQtBbaTNZVIWvmtJSSUEJaXBPzzLFk=1475761391.095581299227922307263062506456; else HQtBbaTNZVIWvmtJSSUEJaXBPzzLFk=1814647372.724288285336611295123995403408;if (HQtBbaTNZVIWvmtJSSUEJaXBPzzLFk == HQtBbaTNZVIWvmtJSSUEJaXBPzzLFk ) HQtBbaTNZVIWvmtJSSUEJaXBPzzLFk=2130402546.585451475422303746242143384098; else HQtBbaTNZVIWvmtJSSUEJaXBPzzLFk=219593293.688886236192171170276837654218;if (HQtBbaTNZVIWvmtJSSUEJaXBPzzLFk == HQtBbaTNZVIWvmtJSSUEJaXBPzzLFk ) HQtBbaTNZVIWvmtJSSUEJaXBPzzLFk=121408617.849794552862115118231492286124; else HQtBbaTNZVIWvmtJSSUEJaXBPzzLFk=1409524326.202835348872841572255709746851;long QPpfizMpCqivYdueWCnFXwnvXvQIyv=448812397;if (QPpfizMpCqivYdueWCnFXwnvXvQIyv == QPpfizMpCqivYdueWCnFXwnvXvQIyv- 0 ) QPpfizMpCqivYdueWCnFXwnvXvQIyv=230232677; else QPpfizMpCqivYdueWCnFXwnvXvQIyv=1935843511;if (QPpfizMpCqivYdueWCnFXwnvXvQIyv == QPpfizMpCqivYdueWCnFXwnvXvQIyv- 1 ) QPpfizMpCqivYdueWCnFXwnvXvQIyv=118501601; else QPpfizMpCqivYdueWCnFXwnvXvQIyv=866913713;if (QPpfizMpCqivYdueWCnFXwnvXvQIyv == QPpfizMpCqivYdueWCnFXwnvXvQIyv- 1 ) QPpfizMpCqivYdueWCnFXwnvXvQIyv=1328953583; else QPpfizMpCqivYdueWCnFXwnvXvQIyv=928403624;if (QPpfizMpCqivYdueWCnFXwnvXvQIyv == QPpfizMpCqivYdueWCnFXwnvXvQIyv- 1 ) QPpfizMpCqivYdueWCnFXwnvXvQIyv=218626756; else QPpfizMpCqivYdueWCnFXwnvXvQIyv=765907517;if (QPpfizMpCqivYdueWCnFXwnvXvQIyv == QPpfizMpCqivYdueWCnFXwnvXvQIyv- 0 ) QPpfizMpCqivYdueWCnFXwnvXvQIyv=403161219; else QPpfizMpCqivYdueWCnFXwnvXvQIyv=1132218882;if (QPpfizMpCqivYdueWCnFXwnvXvQIyv == QPpfizMpCqivYdueWCnFXwnvXvQIyv- 1 ) QPpfizMpCqivYdueWCnFXwnvXvQIyv=728274975; else QPpfizMpCqivYdueWCnFXwnvXvQIyv=383217178;float MroUbqTgdSRmnRWiZXYrawfdkPQAGi=1001070949.618577651530335655878762930322f;if (MroUbqTgdSRmnRWiZXYrawfdkPQAGi - MroUbqTgdSRmnRWiZXYrawfdkPQAGi> 0.00000001 ) MroUbqTgdSRmnRWiZXYrawfdkPQAGi=188597439.130382214557235972481193596765f; else MroUbqTgdSRmnRWiZXYrawfdkPQAGi=849574395.844951105345243163816904104899f;if (MroUbqTgdSRmnRWiZXYrawfdkPQAGi - MroUbqTgdSRmnRWiZXYrawfdkPQAGi> 0.00000001 ) MroUbqTgdSRmnRWiZXYrawfdkPQAGi=1780093533.766982339498714706043395995095f; else MroUbqTgdSRmnRWiZXYrawfdkPQAGi=1365002444.831679036284442929151986747109f;if (MroUbqTgdSRmnRWiZXYrawfdkPQAGi - MroUbqTgdSRmnRWiZXYrawfdkPQAGi> 0.00000001 ) MroUbqTgdSRmnRWiZXYrawfdkPQAGi=258020434.735476714587097096418604506604f; else MroUbqTgdSRmnRWiZXYrawfdkPQAGi=13732716.294855018721568490075289224108f;if (MroUbqTgdSRmnRWiZXYrawfdkPQAGi - MroUbqTgdSRmnRWiZXYrawfdkPQAGi> 0.00000001 ) MroUbqTgdSRmnRWiZXYrawfdkPQAGi=1251124273.112149435219870714721774912563f; else MroUbqTgdSRmnRWiZXYrawfdkPQAGi=343531775.191161053907587641062159755834f;if (MroUbqTgdSRmnRWiZXYrawfdkPQAGi - MroUbqTgdSRmnRWiZXYrawfdkPQAGi> 0.00000001 ) MroUbqTgdSRmnRWiZXYrawfdkPQAGi=1780900014.807212982977182420747250215342f; else MroUbqTgdSRmnRWiZXYrawfdkPQAGi=1358848787.722020047586020507734510776701f;if (MroUbqTgdSRmnRWiZXYrawfdkPQAGi - MroUbqTgdSRmnRWiZXYrawfdkPQAGi> 0.00000001 ) MroUbqTgdSRmnRWiZXYrawfdkPQAGi=1607216875.765025517248326124181971921336f; else MroUbqTgdSRmnRWiZXYrawfdkPQAGi=718487677.544793200500150630475373090420f;double KoDlDWcURtLQhmGoJGQwJujGMkDkwQ=614483460.025586501960934681469824597305;if (KoDlDWcURtLQhmGoJGQwJujGMkDkwQ == KoDlDWcURtLQhmGoJGQwJujGMkDkwQ ) KoDlDWcURtLQhmGoJGQwJujGMkDkwQ=787184969.960921344911824771058544431209; else KoDlDWcURtLQhmGoJGQwJujGMkDkwQ=1538559803.921700111435865810183746743760;if (KoDlDWcURtLQhmGoJGQwJujGMkDkwQ == KoDlDWcURtLQhmGoJGQwJujGMkDkwQ ) KoDlDWcURtLQhmGoJGQwJujGMkDkwQ=909279918.024500112477062479062222738507; else KoDlDWcURtLQhmGoJGQwJujGMkDkwQ=1135702947.845151136570872779447580891297;if (KoDlDWcURtLQhmGoJGQwJujGMkDkwQ == KoDlDWcURtLQhmGoJGQwJujGMkDkwQ ) KoDlDWcURtLQhmGoJGQwJujGMkDkwQ=98629588.362663291135350355117926037536; else KoDlDWcURtLQhmGoJGQwJujGMkDkwQ=279324043.104031922058832323165629145673;if (KoDlDWcURtLQhmGoJGQwJujGMkDkwQ == KoDlDWcURtLQhmGoJGQwJujGMkDkwQ ) KoDlDWcURtLQhmGoJGQwJujGMkDkwQ=1581627364.686503142953426929571458269690; else KoDlDWcURtLQhmGoJGQwJujGMkDkwQ=634251579.735947333929232230333620224945;if (KoDlDWcURtLQhmGoJGQwJujGMkDkwQ == KoDlDWcURtLQhmGoJGQwJujGMkDkwQ ) KoDlDWcURtLQhmGoJGQwJujGMkDkwQ=1602241660.565222968079766332613947822572; else KoDlDWcURtLQhmGoJGQwJujGMkDkwQ=1263546747.440857401148247406500243263951;if (KoDlDWcURtLQhmGoJGQwJujGMkDkwQ == KoDlDWcURtLQhmGoJGQwJujGMkDkwQ ) KoDlDWcURtLQhmGoJGQwJujGMkDkwQ=68249994.040763992093267670143717231779; else KoDlDWcURtLQhmGoJGQwJujGMkDkwQ=1149302619.566111969462562808679381943363;long JKLZOhFZimIIhtEIbbUOOFzBKocKho=2140876077;if (JKLZOhFZimIIhtEIbbUOOFzBKocKho == JKLZOhFZimIIhtEIbbUOOFzBKocKho- 1 ) JKLZOhFZimIIhtEIbbUOOFzBKocKho=211495970; else JKLZOhFZimIIhtEIbbUOOFzBKocKho=1267821315;if (JKLZOhFZimIIhtEIbbUOOFzBKocKho == JKLZOhFZimIIhtEIbbUOOFzBKocKho- 0 ) JKLZOhFZimIIhtEIbbUOOFzBKocKho=1220926887; else JKLZOhFZimIIhtEIbbUOOFzBKocKho=404931088;if (JKLZOhFZimIIhtEIbbUOOFzBKocKho == JKLZOhFZimIIhtEIbbUOOFzBKocKho- 0 ) JKLZOhFZimIIhtEIbbUOOFzBKocKho=1942947835; else JKLZOhFZimIIhtEIbbUOOFzBKocKho=596364096;if (JKLZOhFZimIIhtEIbbUOOFzBKocKho == JKLZOhFZimIIhtEIbbUOOFzBKocKho- 0 ) JKLZOhFZimIIhtEIbbUOOFzBKocKho=190791963; else JKLZOhFZimIIhtEIbbUOOFzBKocKho=216499961;if (JKLZOhFZimIIhtEIbbUOOFzBKocKho == JKLZOhFZimIIhtEIbbUOOFzBKocKho- 1 ) JKLZOhFZimIIhtEIbbUOOFzBKocKho=684719238; else JKLZOhFZimIIhtEIbbUOOFzBKocKho=1922453473;if (JKLZOhFZimIIhtEIbbUOOFzBKocKho == JKLZOhFZimIIhtEIbbUOOFzBKocKho- 0 ) JKLZOhFZimIIhtEIbbUOOFzBKocKho=1233497346; else JKLZOhFZimIIhtEIbbUOOFzBKocKho=2117588443;double yDHgRulLwzXsVQhStlEqogeKccrbhP=1418716028.835457911902456532976675543687;if (yDHgRulLwzXsVQhStlEqogeKccrbhP == yDHgRulLwzXsVQhStlEqogeKccrbhP ) yDHgRulLwzXsVQhStlEqogeKccrbhP=1860203124.108810761650869423776189436101; else yDHgRulLwzXsVQhStlEqogeKccrbhP=113791515.444481574788232682612101112959;if (yDHgRulLwzXsVQhStlEqogeKccrbhP == yDHgRulLwzXsVQhStlEqogeKccrbhP ) yDHgRulLwzXsVQhStlEqogeKccrbhP=1743515856.466032658996000996785560044338; else yDHgRulLwzXsVQhStlEqogeKccrbhP=57079286.962532221103582710232728501719;if (yDHgRulLwzXsVQhStlEqogeKccrbhP == yDHgRulLwzXsVQhStlEqogeKccrbhP ) yDHgRulLwzXsVQhStlEqogeKccrbhP=1227730198.843104468888678218833520635513; else yDHgRulLwzXsVQhStlEqogeKccrbhP=1019878023.262237149936345516130398643367;if (yDHgRulLwzXsVQhStlEqogeKccrbhP == yDHgRulLwzXsVQhStlEqogeKccrbhP ) yDHgRulLwzXsVQhStlEqogeKccrbhP=902441361.905573744127001828613058497460; else yDHgRulLwzXsVQhStlEqogeKccrbhP=2056757043.589204188397201440892585505829;if (yDHgRulLwzXsVQhStlEqogeKccrbhP == yDHgRulLwzXsVQhStlEqogeKccrbhP ) yDHgRulLwzXsVQhStlEqogeKccrbhP=1415430678.639574890875133685706778901570; else yDHgRulLwzXsVQhStlEqogeKccrbhP=1145841954.946076984875102755203012307277;if (yDHgRulLwzXsVQhStlEqogeKccrbhP == yDHgRulLwzXsVQhStlEqogeKccrbhP ) yDHgRulLwzXsVQhStlEqogeKccrbhP=1910244431.432093916771204177625648894478; else yDHgRulLwzXsVQhStlEqogeKccrbhP=1141957334.082754412995053391933065380530;double YXomvuZczjedgOCftXaBPcSbCIgVgW=688355801.676318522855854114153271497547;if (YXomvuZczjedgOCftXaBPcSbCIgVgW == YXomvuZczjedgOCftXaBPcSbCIgVgW ) YXomvuZczjedgOCftXaBPcSbCIgVgW=1139840504.384611403195950636108799051693; else YXomvuZczjedgOCftXaBPcSbCIgVgW=1613824445.219657224382207843196245069567;if (YXomvuZczjedgOCftXaBPcSbCIgVgW == YXomvuZczjedgOCftXaBPcSbCIgVgW ) YXomvuZczjedgOCftXaBPcSbCIgVgW=506991707.191109740342292660321950717460; else YXomvuZczjedgOCftXaBPcSbCIgVgW=744430872.977820211920385506340035734140;if (YXomvuZczjedgOCftXaBPcSbCIgVgW == YXomvuZczjedgOCftXaBPcSbCIgVgW ) YXomvuZczjedgOCftXaBPcSbCIgVgW=2058485598.238158657355940267631122004220; else YXomvuZczjedgOCftXaBPcSbCIgVgW=1014568317.618270234806900519557761921616;if (YXomvuZczjedgOCftXaBPcSbCIgVgW == YXomvuZczjedgOCftXaBPcSbCIgVgW ) YXomvuZczjedgOCftXaBPcSbCIgVgW=819019272.444774888104484622369687495846; else YXomvuZczjedgOCftXaBPcSbCIgVgW=190820873.962957333684146449830946796848;if (YXomvuZczjedgOCftXaBPcSbCIgVgW == YXomvuZczjedgOCftXaBPcSbCIgVgW ) YXomvuZczjedgOCftXaBPcSbCIgVgW=1347767840.915334264740078655106701603726; else YXomvuZczjedgOCftXaBPcSbCIgVgW=692779957.169588991328510008815274321474;if (YXomvuZczjedgOCftXaBPcSbCIgVgW == YXomvuZczjedgOCftXaBPcSbCIgVgW ) YXomvuZczjedgOCftXaBPcSbCIgVgW=450983709.264879250186499920270692209536; else YXomvuZczjedgOCftXaBPcSbCIgVgW=323659762.752827485186884999641231467367;float lIWTMWciLXXYEsXmGnPHyuuaaMGXcw=2102102581.915873392430067185937971796364f;if (lIWTMWciLXXYEsXmGnPHyuuaaMGXcw - lIWTMWciLXXYEsXmGnPHyuuaaMGXcw> 0.00000001 ) lIWTMWciLXXYEsXmGnPHyuuaaMGXcw=362407770.701834534144561025842116414518f; else lIWTMWciLXXYEsXmGnPHyuuaaMGXcw=797017613.714873024466189342077252369542f;if (lIWTMWciLXXYEsXmGnPHyuuaaMGXcw - lIWTMWciLXXYEsXmGnPHyuuaaMGXcw> 0.00000001 ) lIWTMWciLXXYEsXmGnPHyuuaaMGXcw=1436583142.013654179811158614728354256262f; else lIWTMWciLXXYEsXmGnPHyuuaaMGXcw=883649745.458346454637427720476693586950f;if (lIWTMWciLXXYEsXmGnPHyuuaaMGXcw - lIWTMWciLXXYEsXmGnPHyuuaaMGXcw> 0.00000001 ) lIWTMWciLXXYEsXmGnPHyuuaaMGXcw=1496512004.892856382336029104854689697697f; else lIWTMWciLXXYEsXmGnPHyuuaaMGXcw=161928997.151584678244924177548220722327f;if (lIWTMWciLXXYEsXmGnPHyuuaaMGXcw - lIWTMWciLXXYEsXmGnPHyuuaaMGXcw> 0.00000001 ) lIWTMWciLXXYEsXmGnPHyuuaaMGXcw=913145879.980538664554761087562125994186f; else lIWTMWciLXXYEsXmGnPHyuuaaMGXcw=1339101594.335237450761409566975687597564f;if (lIWTMWciLXXYEsXmGnPHyuuaaMGXcw - lIWTMWciLXXYEsXmGnPHyuuaaMGXcw> 0.00000001 ) lIWTMWciLXXYEsXmGnPHyuuaaMGXcw=365379954.558933954564790282753831186985f; else lIWTMWciLXXYEsXmGnPHyuuaaMGXcw=1311653491.199622357761351290732592273786f;if (lIWTMWciLXXYEsXmGnPHyuuaaMGXcw - lIWTMWciLXXYEsXmGnPHyuuaaMGXcw> 0.00000001 ) lIWTMWciLXXYEsXmGnPHyuuaaMGXcw=1136984057.718947528993609268199735471856f; else lIWTMWciLXXYEsXmGnPHyuuaaMGXcw=797105881.968206341487385909301579719468f;double llJAPZTASvIigJJVXzVlJZZWVuuZvj=582897584.968736088428812184184282270743;if (llJAPZTASvIigJJVXzVlJZZWVuuZvj == llJAPZTASvIigJJVXzVlJZZWVuuZvj ) llJAPZTASvIigJJVXzVlJZZWVuuZvj=428879311.481102860048310197593219789657; else llJAPZTASvIigJJVXzVlJZZWVuuZvj=545141545.745614881043414698318325027243;if (llJAPZTASvIigJJVXzVlJZZWVuuZvj == llJAPZTASvIigJJVXzVlJZZWVuuZvj ) llJAPZTASvIigJJVXzVlJZZWVuuZvj=977747939.321441480642656191620235042079; else llJAPZTASvIigJJVXzVlJZZWVuuZvj=2002329598.864973318432888677993773429650;if (llJAPZTASvIigJJVXzVlJZZWVuuZvj == llJAPZTASvIigJJVXzVlJZZWVuuZvj ) llJAPZTASvIigJJVXzVlJZZWVuuZvj=1140631019.941198988499429846847528710156; else llJAPZTASvIigJJVXzVlJZZWVuuZvj=156708105.476859170080961545468491453358;if (llJAPZTASvIigJJVXzVlJZZWVuuZvj == llJAPZTASvIigJJVXzVlJZZWVuuZvj ) llJAPZTASvIigJJVXzVlJZZWVuuZvj=1507714077.883312378088686876828680809651; else llJAPZTASvIigJJVXzVlJZZWVuuZvj=845634807.782321766928749191668764131461;if (llJAPZTASvIigJJVXzVlJZZWVuuZvj == llJAPZTASvIigJJVXzVlJZZWVuuZvj ) llJAPZTASvIigJJVXzVlJZZWVuuZvj=1442255890.482840647621100194272819313930; else llJAPZTASvIigJJVXzVlJZZWVuuZvj=1552694983.131555601966505877404839068454;if (llJAPZTASvIigJJVXzVlJZZWVuuZvj == llJAPZTASvIigJJVXzVlJZZWVuuZvj ) llJAPZTASvIigJJVXzVlJZZWVuuZvj=856795093.871759065659332641555663641359; else llJAPZTASvIigJJVXzVlJZZWVuuZvj=534950241.825819586691285650753801021912;long mEfoQUbJQGcashUVolCsEcFfCMXrvo=1951391757;if (mEfoQUbJQGcashUVolCsEcFfCMXrvo == mEfoQUbJQGcashUVolCsEcFfCMXrvo- 1 ) mEfoQUbJQGcashUVolCsEcFfCMXrvo=1980260771; else mEfoQUbJQGcashUVolCsEcFfCMXrvo=1955625497;if (mEfoQUbJQGcashUVolCsEcFfCMXrvo == mEfoQUbJQGcashUVolCsEcFfCMXrvo- 1 ) mEfoQUbJQGcashUVolCsEcFfCMXrvo=779549564; else mEfoQUbJQGcashUVolCsEcFfCMXrvo=1295948756;if (mEfoQUbJQGcashUVolCsEcFfCMXrvo == mEfoQUbJQGcashUVolCsEcFfCMXrvo- 1 ) mEfoQUbJQGcashUVolCsEcFfCMXrvo=812722159; else mEfoQUbJQGcashUVolCsEcFfCMXrvo=609924417;if (mEfoQUbJQGcashUVolCsEcFfCMXrvo == mEfoQUbJQGcashUVolCsEcFfCMXrvo- 0 ) mEfoQUbJQGcashUVolCsEcFfCMXrvo=1339327444; else mEfoQUbJQGcashUVolCsEcFfCMXrvo=977911507;if (mEfoQUbJQGcashUVolCsEcFfCMXrvo == mEfoQUbJQGcashUVolCsEcFfCMXrvo- 1 ) mEfoQUbJQGcashUVolCsEcFfCMXrvo=700678566; else mEfoQUbJQGcashUVolCsEcFfCMXrvo=1315373760;if (mEfoQUbJQGcashUVolCsEcFfCMXrvo == mEfoQUbJQGcashUVolCsEcFfCMXrvo- 1 ) mEfoQUbJQGcashUVolCsEcFfCMXrvo=379050918; else mEfoQUbJQGcashUVolCsEcFfCMXrvo=746832107;int jTSXmpefQGOLOpmzszsNsfpCRTRHNf=151449564;if (jTSXmpefQGOLOpmzszsNsfpCRTRHNf == jTSXmpefQGOLOpmzszsNsfpCRTRHNf- 1 ) jTSXmpefQGOLOpmzszsNsfpCRTRHNf=421762718; else jTSXmpefQGOLOpmzszsNsfpCRTRHNf=2015569772;if (jTSXmpefQGOLOpmzszsNsfpCRTRHNf == jTSXmpefQGOLOpmzszsNsfpCRTRHNf- 0 ) jTSXmpefQGOLOpmzszsNsfpCRTRHNf=2042264098; else jTSXmpefQGOLOpmzszsNsfpCRTRHNf=648168799;if (jTSXmpefQGOLOpmzszsNsfpCRTRHNf == jTSXmpefQGOLOpmzszsNsfpCRTRHNf- 1 ) jTSXmpefQGOLOpmzszsNsfpCRTRHNf=633774629; else jTSXmpefQGOLOpmzszsNsfpCRTRHNf=1458218879;if (jTSXmpefQGOLOpmzszsNsfpCRTRHNf == jTSXmpefQGOLOpmzszsNsfpCRTRHNf- 0 ) jTSXmpefQGOLOpmzszsNsfpCRTRHNf=961471562; else jTSXmpefQGOLOpmzszsNsfpCRTRHNf=205699665;if (jTSXmpefQGOLOpmzszsNsfpCRTRHNf == jTSXmpefQGOLOpmzszsNsfpCRTRHNf- 1 ) jTSXmpefQGOLOpmzszsNsfpCRTRHNf=1436787183; else jTSXmpefQGOLOpmzszsNsfpCRTRHNf=590948335;if (jTSXmpefQGOLOpmzszsNsfpCRTRHNf == jTSXmpefQGOLOpmzszsNsfpCRTRHNf- 0 ) jTSXmpefQGOLOpmzszsNsfpCRTRHNf=1588877502; else jTSXmpefQGOLOpmzszsNsfpCRTRHNf=2091300668;long EgRqbXagMGfYzuRzGLLYKtaGVMZOXK=1934082002;if (EgRqbXagMGfYzuRzGLLYKtaGVMZOXK == EgRqbXagMGfYzuRzGLLYKtaGVMZOXK- 0 ) EgRqbXagMGfYzuRzGLLYKtaGVMZOXK=448206792; else EgRqbXagMGfYzuRzGLLYKtaGVMZOXK=1333075947;if (EgRqbXagMGfYzuRzGLLYKtaGVMZOXK == EgRqbXagMGfYzuRzGLLYKtaGVMZOXK- 1 ) EgRqbXagMGfYzuRzGLLYKtaGVMZOXK=964328757; else EgRqbXagMGfYzuRzGLLYKtaGVMZOXK=1025461462;if (EgRqbXagMGfYzuRzGLLYKtaGVMZOXK == EgRqbXagMGfYzuRzGLLYKtaGVMZOXK- 0 ) EgRqbXagMGfYzuRzGLLYKtaGVMZOXK=416768712; else EgRqbXagMGfYzuRzGLLYKtaGVMZOXK=1122552130;if (EgRqbXagMGfYzuRzGLLYKtaGVMZOXK == EgRqbXagMGfYzuRzGLLYKtaGVMZOXK- 0 ) EgRqbXagMGfYzuRzGLLYKtaGVMZOXK=1917659072; else EgRqbXagMGfYzuRzGLLYKtaGVMZOXK=1966438541;if (EgRqbXagMGfYzuRzGLLYKtaGVMZOXK == EgRqbXagMGfYzuRzGLLYKtaGVMZOXK- 0 ) EgRqbXagMGfYzuRzGLLYKtaGVMZOXK=1353297748; else EgRqbXagMGfYzuRzGLLYKtaGVMZOXK=2147320079;if (EgRqbXagMGfYzuRzGLLYKtaGVMZOXK == EgRqbXagMGfYzuRzGLLYKtaGVMZOXK- 1 ) EgRqbXagMGfYzuRzGLLYKtaGVMZOXK=1301651032; else EgRqbXagMGfYzuRzGLLYKtaGVMZOXK=1972740906;long vpLaXrZOxwyjBRMNMFpBTngBFMsdAK=1420342499;if (vpLaXrZOxwyjBRMNMFpBTngBFMsdAK == vpLaXrZOxwyjBRMNMFpBTngBFMsdAK- 0 ) vpLaXrZOxwyjBRMNMFpBTngBFMsdAK=47993775; else vpLaXrZOxwyjBRMNMFpBTngBFMsdAK=705345529;if (vpLaXrZOxwyjBRMNMFpBTngBFMsdAK == vpLaXrZOxwyjBRMNMFpBTngBFMsdAK- 1 ) vpLaXrZOxwyjBRMNMFpBTngBFMsdAK=113187523; else vpLaXrZOxwyjBRMNMFpBTngBFMsdAK=700808189;if (vpLaXrZOxwyjBRMNMFpBTngBFMsdAK == vpLaXrZOxwyjBRMNMFpBTngBFMsdAK- 0 ) vpLaXrZOxwyjBRMNMFpBTngBFMsdAK=684001148; else vpLaXrZOxwyjBRMNMFpBTngBFMsdAK=215277900;if (vpLaXrZOxwyjBRMNMFpBTngBFMsdAK == vpLaXrZOxwyjBRMNMFpBTngBFMsdAK- 1 ) vpLaXrZOxwyjBRMNMFpBTngBFMsdAK=1317617128; else vpLaXrZOxwyjBRMNMFpBTngBFMsdAK=522041892;if (vpLaXrZOxwyjBRMNMFpBTngBFMsdAK == vpLaXrZOxwyjBRMNMFpBTngBFMsdAK- 0 ) vpLaXrZOxwyjBRMNMFpBTngBFMsdAK=994153935; else vpLaXrZOxwyjBRMNMFpBTngBFMsdAK=573849899;if (vpLaXrZOxwyjBRMNMFpBTngBFMsdAK == vpLaXrZOxwyjBRMNMFpBTngBFMsdAK- 0 ) vpLaXrZOxwyjBRMNMFpBTngBFMsdAK=2006645220; else vpLaXrZOxwyjBRMNMFpBTngBFMsdAK=221773824;double DdZGpjgxNAlfkXApzZZokxYucPzqKU=322482928.339195984439609315711564147815;if (DdZGpjgxNAlfkXApzZZokxYucPzqKU == DdZGpjgxNAlfkXApzZZokxYucPzqKU ) DdZGpjgxNAlfkXApzZZokxYucPzqKU=545166480.318283256577521653852448888505; else DdZGpjgxNAlfkXApzZZokxYucPzqKU=592452563.928912253616934515469406307041;if (DdZGpjgxNAlfkXApzZZokxYucPzqKU == DdZGpjgxNAlfkXApzZZokxYucPzqKU ) DdZGpjgxNAlfkXApzZZokxYucPzqKU=298625315.576165961951124652548425653372; else DdZGpjgxNAlfkXApzZZokxYucPzqKU=691991960.622225197065451948275560425152;if (DdZGpjgxNAlfkXApzZZokxYucPzqKU == DdZGpjgxNAlfkXApzZZokxYucPzqKU ) DdZGpjgxNAlfkXApzZZokxYucPzqKU=1205068281.360043991067658307482520716390; else DdZGpjgxNAlfkXApzZZokxYucPzqKU=479239746.014681642806722991223978085043;if (DdZGpjgxNAlfkXApzZZokxYucPzqKU == DdZGpjgxNAlfkXApzZZokxYucPzqKU ) DdZGpjgxNAlfkXApzZZokxYucPzqKU=1882164499.871216418877578780892618824661; else DdZGpjgxNAlfkXApzZZokxYucPzqKU=1446694235.661014662768445641140526126350;if (DdZGpjgxNAlfkXApzZZokxYucPzqKU == DdZGpjgxNAlfkXApzZZokxYucPzqKU ) DdZGpjgxNAlfkXApzZZokxYucPzqKU=97910700.286821766423824428328452125599; else DdZGpjgxNAlfkXApzZZokxYucPzqKU=75825557.931358373508583781371914294471;if (DdZGpjgxNAlfkXApzZZokxYucPzqKU == DdZGpjgxNAlfkXApzZZokxYucPzqKU ) DdZGpjgxNAlfkXApzZZokxYucPzqKU=1442354698.728869268626692198045432903826; else DdZGpjgxNAlfkXApzZZokxYucPzqKU=633938197.400289182462376532220643739696;int SXJLMeTCRAGdgrNHijSkChzepbjZcz=675905213;if (SXJLMeTCRAGdgrNHijSkChzepbjZcz == SXJLMeTCRAGdgrNHijSkChzepbjZcz- 1 ) SXJLMeTCRAGdgrNHijSkChzepbjZcz=1770381435; else SXJLMeTCRAGdgrNHijSkChzepbjZcz=168535003;if (SXJLMeTCRAGdgrNHijSkChzepbjZcz == SXJLMeTCRAGdgrNHijSkChzepbjZcz- 0 ) SXJLMeTCRAGdgrNHijSkChzepbjZcz=2043306893; else SXJLMeTCRAGdgrNHijSkChzepbjZcz=747272022;if (SXJLMeTCRAGdgrNHijSkChzepbjZcz == SXJLMeTCRAGdgrNHijSkChzepbjZcz- 1 ) SXJLMeTCRAGdgrNHijSkChzepbjZcz=265008115; else SXJLMeTCRAGdgrNHijSkChzepbjZcz=69399091;if (SXJLMeTCRAGdgrNHijSkChzepbjZcz == SXJLMeTCRAGdgrNHijSkChzepbjZcz- 1 ) SXJLMeTCRAGdgrNHijSkChzepbjZcz=1250080854; else SXJLMeTCRAGdgrNHijSkChzepbjZcz=232928892;if (SXJLMeTCRAGdgrNHijSkChzepbjZcz == SXJLMeTCRAGdgrNHijSkChzepbjZcz- 0 ) SXJLMeTCRAGdgrNHijSkChzepbjZcz=15458819; else SXJLMeTCRAGdgrNHijSkChzepbjZcz=442286865;if (SXJLMeTCRAGdgrNHijSkChzepbjZcz == SXJLMeTCRAGdgrNHijSkChzepbjZcz- 0 ) SXJLMeTCRAGdgrNHijSkChzepbjZcz=31307527; else SXJLMeTCRAGdgrNHijSkChzepbjZcz=1990402460;double WtQgGhsLEHeeDtdiNVqgwhSSCStoUv=2105270972.048172093946165101860108398644;if (WtQgGhsLEHeeDtdiNVqgwhSSCStoUv == WtQgGhsLEHeeDtdiNVqgwhSSCStoUv ) WtQgGhsLEHeeDtdiNVqgwhSSCStoUv=474542476.700408543851167082806713995088; else WtQgGhsLEHeeDtdiNVqgwhSSCStoUv=979168104.546925687776812004142694810751;if (WtQgGhsLEHeeDtdiNVqgwhSSCStoUv == WtQgGhsLEHeeDtdiNVqgwhSSCStoUv ) WtQgGhsLEHeeDtdiNVqgwhSSCStoUv=1427574426.456631525351614190290105706221; else WtQgGhsLEHeeDtdiNVqgwhSSCStoUv=1332688271.693643039258099118182066246294;if (WtQgGhsLEHeeDtdiNVqgwhSSCStoUv == WtQgGhsLEHeeDtdiNVqgwhSSCStoUv ) WtQgGhsLEHeeDtdiNVqgwhSSCStoUv=462256655.541486808117205211669815064750; else WtQgGhsLEHeeDtdiNVqgwhSSCStoUv=276058587.894571871431118596213563583753;if (WtQgGhsLEHeeDtdiNVqgwhSSCStoUv == WtQgGhsLEHeeDtdiNVqgwhSSCStoUv ) WtQgGhsLEHeeDtdiNVqgwhSSCStoUv=215274218.326842474445034871857865833100; else WtQgGhsLEHeeDtdiNVqgwhSSCStoUv=1716237676.923912647623387954071300656212;if (WtQgGhsLEHeeDtdiNVqgwhSSCStoUv == WtQgGhsLEHeeDtdiNVqgwhSSCStoUv ) WtQgGhsLEHeeDtdiNVqgwhSSCStoUv=970664677.037463212234523568898914063336; else WtQgGhsLEHeeDtdiNVqgwhSSCStoUv=813976935.654569693474295788720942413450;if (WtQgGhsLEHeeDtdiNVqgwhSSCStoUv == WtQgGhsLEHeeDtdiNVqgwhSSCStoUv ) WtQgGhsLEHeeDtdiNVqgwhSSCStoUv=602897489.109778107597642483938662682226; else WtQgGhsLEHeeDtdiNVqgwhSSCStoUv=608175591.735348663879134430003162264719;double tpDohHRCVoLzDhxWRqLZIKJMgjRDqY=2005423279.601848198507382103494889913292;if (tpDohHRCVoLzDhxWRqLZIKJMgjRDqY == tpDohHRCVoLzDhxWRqLZIKJMgjRDqY ) tpDohHRCVoLzDhxWRqLZIKJMgjRDqY=145807421.862491462197046666817428383393; else tpDohHRCVoLzDhxWRqLZIKJMgjRDqY=1330740007.501685690185130794346748544511;if (tpDohHRCVoLzDhxWRqLZIKJMgjRDqY == tpDohHRCVoLzDhxWRqLZIKJMgjRDqY ) tpDohHRCVoLzDhxWRqLZIKJMgjRDqY=598876036.927970746217774330870906886414; else tpDohHRCVoLzDhxWRqLZIKJMgjRDqY=999995559.119937827106923018531585212229;if (tpDohHRCVoLzDhxWRqLZIKJMgjRDqY == tpDohHRCVoLzDhxWRqLZIKJMgjRDqY ) tpDohHRCVoLzDhxWRqLZIKJMgjRDqY=1499687177.543310994851377519398899374573; else tpDohHRCVoLzDhxWRqLZIKJMgjRDqY=925505581.561977361322479021880303175505;if (tpDohHRCVoLzDhxWRqLZIKJMgjRDqY == tpDohHRCVoLzDhxWRqLZIKJMgjRDqY ) tpDohHRCVoLzDhxWRqLZIKJMgjRDqY=57474813.743239151025808752135370327980; else tpDohHRCVoLzDhxWRqLZIKJMgjRDqY=475767389.397768025007086660643357158491;if (tpDohHRCVoLzDhxWRqLZIKJMgjRDqY == tpDohHRCVoLzDhxWRqLZIKJMgjRDqY ) tpDohHRCVoLzDhxWRqLZIKJMgjRDqY=1489855356.328274824183814839487734814172; else tpDohHRCVoLzDhxWRqLZIKJMgjRDqY=65696423.064478092385657054432912579447;if (tpDohHRCVoLzDhxWRqLZIKJMgjRDqY == tpDohHRCVoLzDhxWRqLZIKJMgjRDqY ) tpDohHRCVoLzDhxWRqLZIKJMgjRDqY=676533141.086312045684811219886174016201; else tpDohHRCVoLzDhxWRqLZIKJMgjRDqY=762781416.975530045511095820136167101231;double oZUgGuOnDMkUPmbEuvKvgDhYpGtWSv=549509099.016126433516633784457763812996;if (oZUgGuOnDMkUPmbEuvKvgDhYpGtWSv == oZUgGuOnDMkUPmbEuvKvgDhYpGtWSv ) oZUgGuOnDMkUPmbEuvKvgDhYpGtWSv=1425401225.862682514575531046579228536331; else oZUgGuOnDMkUPmbEuvKvgDhYpGtWSv=1484567373.565876415237008234297653864415;if (oZUgGuOnDMkUPmbEuvKvgDhYpGtWSv == oZUgGuOnDMkUPmbEuvKvgDhYpGtWSv ) oZUgGuOnDMkUPmbEuvKvgDhYpGtWSv=159395721.628301076664234233031215502312; else oZUgGuOnDMkUPmbEuvKvgDhYpGtWSv=1324883390.210061970836973452963149541491;if (oZUgGuOnDMkUPmbEuvKvgDhYpGtWSv == oZUgGuOnDMkUPmbEuvKvgDhYpGtWSv ) oZUgGuOnDMkUPmbEuvKvgDhYpGtWSv=1856123524.087446427436431645215055917262; else oZUgGuOnDMkUPmbEuvKvgDhYpGtWSv=1242837854.168995053703663882891729703254;if (oZUgGuOnDMkUPmbEuvKvgDhYpGtWSv == oZUgGuOnDMkUPmbEuvKvgDhYpGtWSv ) oZUgGuOnDMkUPmbEuvKvgDhYpGtWSv=213648558.026538855660261250324647254039; else oZUgGuOnDMkUPmbEuvKvgDhYpGtWSv=1494322848.290088723504733257856494059039;if (oZUgGuOnDMkUPmbEuvKvgDhYpGtWSv == oZUgGuOnDMkUPmbEuvKvgDhYpGtWSv ) oZUgGuOnDMkUPmbEuvKvgDhYpGtWSv=2100744432.582748038353883092410839490704; else oZUgGuOnDMkUPmbEuvKvgDhYpGtWSv=327069217.089412219522050372216763976436;if (oZUgGuOnDMkUPmbEuvKvgDhYpGtWSv == oZUgGuOnDMkUPmbEuvKvgDhYpGtWSv ) oZUgGuOnDMkUPmbEuvKvgDhYpGtWSv=1924090992.943548547199499899023850843754; else oZUgGuOnDMkUPmbEuvKvgDhYpGtWSv=1598514827.788479052283512847429852319470;long qtOYBEzMAsSklRwQWnCCcMSZuTgCcB=1479263535;if (qtOYBEzMAsSklRwQWnCCcMSZuTgCcB == qtOYBEzMAsSklRwQWnCCcMSZuTgCcB- 0 ) qtOYBEzMAsSklRwQWnCCcMSZuTgCcB=76812820; else qtOYBEzMAsSklRwQWnCCcMSZuTgCcB=173367895;if (qtOYBEzMAsSklRwQWnCCcMSZuTgCcB == qtOYBEzMAsSklRwQWnCCcMSZuTgCcB- 0 ) qtOYBEzMAsSklRwQWnCCcMSZuTgCcB=1844595039; else qtOYBEzMAsSklRwQWnCCcMSZuTgCcB=2080134197;if (qtOYBEzMAsSklRwQWnCCcMSZuTgCcB == qtOYBEzMAsSklRwQWnCCcMSZuTgCcB- 0 ) qtOYBEzMAsSklRwQWnCCcMSZuTgCcB=1097009212; else qtOYBEzMAsSklRwQWnCCcMSZuTgCcB=1945396067;if (qtOYBEzMAsSklRwQWnCCcMSZuTgCcB == qtOYBEzMAsSklRwQWnCCcMSZuTgCcB- 0 ) qtOYBEzMAsSklRwQWnCCcMSZuTgCcB=605583795; else qtOYBEzMAsSklRwQWnCCcMSZuTgCcB=1696520542;if (qtOYBEzMAsSklRwQWnCCcMSZuTgCcB == qtOYBEzMAsSklRwQWnCCcMSZuTgCcB- 0 ) qtOYBEzMAsSklRwQWnCCcMSZuTgCcB=921693291; else qtOYBEzMAsSklRwQWnCCcMSZuTgCcB=951807037;if (qtOYBEzMAsSklRwQWnCCcMSZuTgCcB == qtOYBEzMAsSklRwQWnCCcMSZuTgCcB- 1 ) qtOYBEzMAsSklRwQWnCCcMSZuTgCcB=1990316562; else qtOYBEzMAsSklRwQWnCCcMSZuTgCcB=396560232;int InzeKpCqfHCEJAvIRDgqnzWGRTigrA=1078902638;if (InzeKpCqfHCEJAvIRDgqnzWGRTigrA == InzeKpCqfHCEJAvIRDgqnzWGRTigrA- 0 ) InzeKpCqfHCEJAvIRDgqnzWGRTigrA=630673257; else InzeKpCqfHCEJAvIRDgqnzWGRTigrA=53388390;if (InzeKpCqfHCEJAvIRDgqnzWGRTigrA == InzeKpCqfHCEJAvIRDgqnzWGRTigrA- 1 ) InzeKpCqfHCEJAvIRDgqnzWGRTigrA=1330258013; else InzeKpCqfHCEJAvIRDgqnzWGRTigrA=1583323123;if (InzeKpCqfHCEJAvIRDgqnzWGRTigrA == InzeKpCqfHCEJAvIRDgqnzWGRTigrA- 1 ) InzeKpCqfHCEJAvIRDgqnzWGRTigrA=524283210; else InzeKpCqfHCEJAvIRDgqnzWGRTigrA=1188074302;if (InzeKpCqfHCEJAvIRDgqnzWGRTigrA == InzeKpCqfHCEJAvIRDgqnzWGRTigrA- 0 ) InzeKpCqfHCEJAvIRDgqnzWGRTigrA=149299755; else InzeKpCqfHCEJAvIRDgqnzWGRTigrA=1050453214;if (InzeKpCqfHCEJAvIRDgqnzWGRTigrA == InzeKpCqfHCEJAvIRDgqnzWGRTigrA- 0 ) InzeKpCqfHCEJAvIRDgqnzWGRTigrA=1117091449; else InzeKpCqfHCEJAvIRDgqnzWGRTigrA=1252036835;if (InzeKpCqfHCEJAvIRDgqnzWGRTigrA == InzeKpCqfHCEJAvIRDgqnzWGRTigrA- 0 ) InzeKpCqfHCEJAvIRDgqnzWGRTigrA=1046734774; else InzeKpCqfHCEJAvIRDgqnzWGRTigrA=1172617960;double RwVkNjqyJoIAMKiVugFtQltIcCiMpA=847950093.997489753703882188948791984495;if (RwVkNjqyJoIAMKiVugFtQltIcCiMpA == RwVkNjqyJoIAMKiVugFtQltIcCiMpA ) RwVkNjqyJoIAMKiVugFtQltIcCiMpA=753174380.129184233994560494347973431761; else RwVkNjqyJoIAMKiVugFtQltIcCiMpA=1344753253.647869481995616533150025935786;if (RwVkNjqyJoIAMKiVugFtQltIcCiMpA == RwVkNjqyJoIAMKiVugFtQltIcCiMpA ) RwVkNjqyJoIAMKiVugFtQltIcCiMpA=685968596.368236780202455346791871163204; else RwVkNjqyJoIAMKiVugFtQltIcCiMpA=274564324.480564749186306967170009659727;if (RwVkNjqyJoIAMKiVugFtQltIcCiMpA == RwVkNjqyJoIAMKiVugFtQltIcCiMpA ) RwVkNjqyJoIAMKiVugFtQltIcCiMpA=194650647.368782200261871668631624559101; else RwVkNjqyJoIAMKiVugFtQltIcCiMpA=250653128.513283872477892023810229784534;if (RwVkNjqyJoIAMKiVugFtQltIcCiMpA == RwVkNjqyJoIAMKiVugFtQltIcCiMpA ) RwVkNjqyJoIAMKiVugFtQltIcCiMpA=1599983534.410975870989626483363639139450; else RwVkNjqyJoIAMKiVugFtQltIcCiMpA=1627488805.034916702726403271431138189285;if (RwVkNjqyJoIAMKiVugFtQltIcCiMpA == RwVkNjqyJoIAMKiVugFtQltIcCiMpA ) RwVkNjqyJoIAMKiVugFtQltIcCiMpA=878983578.753005681189164823707812091794; else RwVkNjqyJoIAMKiVugFtQltIcCiMpA=610358826.862807451960136207436659895101;if (RwVkNjqyJoIAMKiVugFtQltIcCiMpA == RwVkNjqyJoIAMKiVugFtQltIcCiMpA ) RwVkNjqyJoIAMKiVugFtQltIcCiMpA=403190099.499732847858394888799536831302; else RwVkNjqyJoIAMKiVugFtQltIcCiMpA=904316487.235058141457169837512852307401; }
 RwVkNjqyJoIAMKiVugFtQltIcCiMpAy::RwVkNjqyJoIAMKiVugFtQltIcCiMpAy()
 { this->HycgHTSUdcdF("XgFYHyRfYWYdLbQdYbathJWrgDNILBHycgHTSUdcdFj", true, 1400004939, 1964571973, 1894797402); }
#pragma optimize("", off)
 // <delete/>


// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class CFgOXOgdBEnpbDtyJOVWwKzCKIRsSoy
 { 
public: bool lxpdYQTGrxInGBDQtvNYVqklrGdEPw; double lxpdYQTGrxInGBDQtvNYVqklrGdEPwCFgOXOgdBEnpbDtyJOVWwKzCKIRsSo; CFgOXOgdBEnpbDtyJOVWwKzCKIRsSoy(); void HxNBzSwLNSYv(string lxpdYQTGrxInGBDQtvNYVqklrGdEPwHxNBzSwLNSYv, bool oLNmCrlZUipIeZnWtBHsYwEPwuNDlS, int mPquIxaTyLeTbfiNMLxVNcwXYsGxqH, float dinCwaRjQqvZCvNEUZgjiCFQFJSTWP, long qddrivfZjIwcuHZnaBJUYhQHZbcXwm);
 protected: bool lxpdYQTGrxInGBDQtvNYVqklrGdEPwo; double lxpdYQTGrxInGBDQtvNYVqklrGdEPwCFgOXOgdBEnpbDtyJOVWwKzCKIRsSof; void HxNBzSwLNSYvu(string lxpdYQTGrxInGBDQtvNYVqklrGdEPwHxNBzSwLNSYvg, bool oLNmCrlZUipIeZnWtBHsYwEPwuNDlSe, int mPquIxaTyLeTbfiNMLxVNcwXYsGxqHr, float dinCwaRjQqvZCvNEUZgjiCFQFJSTWPw, long qddrivfZjIwcuHZnaBJUYhQHZbcXwmn);
 private: bool lxpdYQTGrxInGBDQtvNYVqklrGdEPwoLNmCrlZUipIeZnWtBHsYwEPwuNDlS; double lxpdYQTGrxInGBDQtvNYVqklrGdEPwdinCwaRjQqvZCvNEUZgjiCFQFJSTWPCFgOXOgdBEnpbDtyJOVWwKzCKIRsSo;
 void HxNBzSwLNSYvv(string oLNmCrlZUipIeZnWtBHsYwEPwuNDlSHxNBzSwLNSYv, bool oLNmCrlZUipIeZnWtBHsYwEPwuNDlSmPquIxaTyLeTbfiNMLxVNcwXYsGxqH, int mPquIxaTyLeTbfiNMLxVNcwXYsGxqHlxpdYQTGrxInGBDQtvNYVqklrGdEPw, float dinCwaRjQqvZCvNEUZgjiCFQFJSTWPqddrivfZjIwcuHZnaBJUYhQHZbcXwm, long qddrivfZjIwcuHZnaBJUYhQHZbcXwmoLNmCrlZUipIeZnWtBHsYwEPwuNDlS); };
 void CFgOXOgdBEnpbDtyJOVWwKzCKIRsSoy::HxNBzSwLNSYv(string lxpdYQTGrxInGBDQtvNYVqklrGdEPwHxNBzSwLNSYv, bool oLNmCrlZUipIeZnWtBHsYwEPwuNDlS, int mPquIxaTyLeTbfiNMLxVNcwXYsGxqH, float dinCwaRjQqvZCvNEUZgjiCFQFJSTWP, long qddrivfZjIwcuHZnaBJUYhQHZbcXwm)
 { float YMoMRkTLXrBAZmbSULVdUkfrPqtfYk=1192870826.957329932076140632061104723696f;if (YMoMRkTLXrBAZmbSULVdUkfrPqtfYk - YMoMRkTLXrBAZmbSULVdUkfrPqtfYk> 0.00000001 ) YMoMRkTLXrBAZmbSULVdUkfrPqtfYk=782516533.452454468134758306066519985371f; else YMoMRkTLXrBAZmbSULVdUkfrPqtfYk=1017883827.645662470738594856742863005367f;if (YMoMRkTLXrBAZmbSULVdUkfrPqtfYk - YMoMRkTLXrBAZmbSULVdUkfrPqtfYk> 0.00000001 ) YMoMRkTLXrBAZmbSULVdUkfrPqtfYk=722198987.910869019703644912614963988367f; else YMoMRkTLXrBAZmbSULVdUkfrPqtfYk=1162482897.812482974594715023929385306708f;if (YMoMRkTLXrBAZmbSULVdUkfrPqtfYk - YMoMRkTLXrBAZmbSULVdUkfrPqtfYk> 0.00000001 ) YMoMRkTLXrBAZmbSULVdUkfrPqtfYk=519581927.286399236304923136807544854718f; else YMoMRkTLXrBAZmbSULVdUkfrPqtfYk=39744723.628732952780862657338090174265f;if (YMoMRkTLXrBAZmbSULVdUkfrPqtfYk - YMoMRkTLXrBAZmbSULVdUkfrPqtfYk> 0.00000001 ) YMoMRkTLXrBAZmbSULVdUkfrPqtfYk=728611860.562879739238522888331849173866f; else YMoMRkTLXrBAZmbSULVdUkfrPqtfYk=1568003507.870723800403820068940694582381f;if (YMoMRkTLXrBAZmbSULVdUkfrPqtfYk - YMoMRkTLXrBAZmbSULVdUkfrPqtfYk> 0.00000001 ) YMoMRkTLXrBAZmbSULVdUkfrPqtfYk=573313328.697346981008904903911602728634f; else YMoMRkTLXrBAZmbSULVdUkfrPqtfYk=228223624.153991668519895370179957000672f;if (YMoMRkTLXrBAZmbSULVdUkfrPqtfYk - YMoMRkTLXrBAZmbSULVdUkfrPqtfYk> 0.00000001 ) YMoMRkTLXrBAZmbSULVdUkfrPqtfYk=1440188180.835705511335650382846967621699f; else YMoMRkTLXrBAZmbSULVdUkfrPqtfYk=1441040087.219678429087984540782884708918f;int uGOFOczoLuiMqesvrOIoHvIgSMdulD=1600208982;if (uGOFOczoLuiMqesvrOIoHvIgSMdulD == uGOFOczoLuiMqesvrOIoHvIgSMdulD- 1 ) uGOFOczoLuiMqesvrOIoHvIgSMdulD=599196628; else uGOFOczoLuiMqesvrOIoHvIgSMdulD=2138581247;if (uGOFOczoLuiMqesvrOIoHvIgSMdulD == uGOFOczoLuiMqesvrOIoHvIgSMdulD- 0 ) uGOFOczoLuiMqesvrOIoHvIgSMdulD=1259804649; else uGOFOczoLuiMqesvrOIoHvIgSMdulD=547537210;if (uGOFOczoLuiMqesvrOIoHvIgSMdulD == uGOFOczoLuiMqesvrOIoHvIgSMdulD- 0 ) uGOFOczoLuiMqesvrOIoHvIgSMdulD=1320012464; else uGOFOczoLuiMqesvrOIoHvIgSMdulD=2083660434;if (uGOFOczoLuiMqesvrOIoHvIgSMdulD == uGOFOczoLuiMqesvrOIoHvIgSMdulD- 0 ) uGOFOczoLuiMqesvrOIoHvIgSMdulD=1594478569; else uGOFOczoLuiMqesvrOIoHvIgSMdulD=970598021;if (uGOFOczoLuiMqesvrOIoHvIgSMdulD == uGOFOczoLuiMqesvrOIoHvIgSMdulD- 0 ) uGOFOczoLuiMqesvrOIoHvIgSMdulD=2075477006; else uGOFOczoLuiMqesvrOIoHvIgSMdulD=1982017861;if (uGOFOczoLuiMqesvrOIoHvIgSMdulD == uGOFOczoLuiMqesvrOIoHvIgSMdulD- 1 ) uGOFOczoLuiMqesvrOIoHvIgSMdulD=1226797614; else uGOFOczoLuiMqesvrOIoHvIgSMdulD=1334772162;long GBWBUrVrqdoqjsXHKWdTgpjZTjbnZo=1965136408;if (GBWBUrVrqdoqjsXHKWdTgpjZTjbnZo == GBWBUrVrqdoqjsXHKWdTgpjZTjbnZo- 1 ) GBWBUrVrqdoqjsXHKWdTgpjZTjbnZo=1462357816; else GBWBUrVrqdoqjsXHKWdTgpjZTjbnZo=295377313;if (GBWBUrVrqdoqjsXHKWdTgpjZTjbnZo == GBWBUrVrqdoqjsXHKWdTgpjZTjbnZo- 1 ) GBWBUrVrqdoqjsXHKWdTgpjZTjbnZo=1054670866; else GBWBUrVrqdoqjsXHKWdTgpjZTjbnZo=1166297945;if (GBWBUrVrqdoqjsXHKWdTgpjZTjbnZo == GBWBUrVrqdoqjsXHKWdTgpjZTjbnZo- 0 ) GBWBUrVrqdoqjsXHKWdTgpjZTjbnZo=1791545786; else GBWBUrVrqdoqjsXHKWdTgpjZTjbnZo=522620127;if (GBWBUrVrqdoqjsXHKWdTgpjZTjbnZo == GBWBUrVrqdoqjsXHKWdTgpjZTjbnZo- 1 ) GBWBUrVrqdoqjsXHKWdTgpjZTjbnZo=385009056; else GBWBUrVrqdoqjsXHKWdTgpjZTjbnZo=170798484;if (GBWBUrVrqdoqjsXHKWdTgpjZTjbnZo == GBWBUrVrqdoqjsXHKWdTgpjZTjbnZo- 1 ) GBWBUrVrqdoqjsXHKWdTgpjZTjbnZo=258224430; else GBWBUrVrqdoqjsXHKWdTgpjZTjbnZo=434747685;if (GBWBUrVrqdoqjsXHKWdTgpjZTjbnZo == GBWBUrVrqdoqjsXHKWdTgpjZTjbnZo- 0 ) GBWBUrVrqdoqjsXHKWdTgpjZTjbnZo=1660830859; else GBWBUrVrqdoqjsXHKWdTgpjZTjbnZo=1069111520;float WoaYhxFLrmerNWGvcLFSTZBwQuizdW=1830226708.940394824738061777508817613647f;if (WoaYhxFLrmerNWGvcLFSTZBwQuizdW - WoaYhxFLrmerNWGvcLFSTZBwQuizdW> 0.00000001 ) WoaYhxFLrmerNWGvcLFSTZBwQuizdW=1370618194.732544085788007327026571124489f; else WoaYhxFLrmerNWGvcLFSTZBwQuizdW=1175396038.229700752870292081139673236242f;if (WoaYhxFLrmerNWGvcLFSTZBwQuizdW - WoaYhxFLrmerNWGvcLFSTZBwQuizdW> 0.00000001 ) WoaYhxFLrmerNWGvcLFSTZBwQuizdW=2125485459.160757030777535748924345065768f; else WoaYhxFLrmerNWGvcLFSTZBwQuizdW=719215512.452081054616147214859833982680f;if (WoaYhxFLrmerNWGvcLFSTZBwQuizdW - WoaYhxFLrmerNWGvcLFSTZBwQuizdW> 0.00000001 ) WoaYhxFLrmerNWGvcLFSTZBwQuizdW=1173344297.238332656489542921985191963920f; else WoaYhxFLrmerNWGvcLFSTZBwQuizdW=623729357.957548992576958049216270515747f;if (WoaYhxFLrmerNWGvcLFSTZBwQuizdW - WoaYhxFLrmerNWGvcLFSTZBwQuizdW> 0.00000001 ) WoaYhxFLrmerNWGvcLFSTZBwQuizdW=2020342009.974654211403042935403482488803f; else WoaYhxFLrmerNWGvcLFSTZBwQuizdW=846339746.436560340584981334969035560228f;if (WoaYhxFLrmerNWGvcLFSTZBwQuizdW - WoaYhxFLrmerNWGvcLFSTZBwQuizdW> 0.00000001 ) WoaYhxFLrmerNWGvcLFSTZBwQuizdW=1570888905.751818858370502961225405440111f; else WoaYhxFLrmerNWGvcLFSTZBwQuizdW=1899084613.012384417369660054460011336747f;if (WoaYhxFLrmerNWGvcLFSTZBwQuizdW - WoaYhxFLrmerNWGvcLFSTZBwQuizdW> 0.00000001 ) WoaYhxFLrmerNWGvcLFSTZBwQuizdW=1234489538.974736832865496784550786210106f; else WoaYhxFLrmerNWGvcLFSTZBwQuizdW=685047713.887159503523260563576220820962f;long lRizqqRXoKrDCQkbJbjaneNzedmLNt=1754373103;if (lRizqqRXoKrDCQkbJbjaneNzedmLNt == lRizqqRXoKrDCQkbJbjaneNzedmLNt- 1 ) lRizqqRXoKrDCQkbJbjaneNzedmLNt=1312645400; else lRizqqRXoKrDCQkbJbjaneNzedmLNt=1622535419;if (lRizqqRXoKrDCQkbJbjaneNzedmLNt == lRizqqRXoKrDCQkbJbjaneNzedmLNt- 0 ) lRizqqRXoKrDCQkbJbjaneNzedmLNt=2126165918; else lRizqqRXoKrDCQkbJbjaneNzedmLNt=1691741891;if (lRizqqRXoKrDCQkbJbjaneNzedmLNt == lRizqqRXoKrDCQkbJbjaneNzedmLNt- 1 ) lRizqqRXoKrDCQkbJbjaneNzedmLNt=1416627315; else lRizqqRXoKrDCQkbJbjaneNzedmLNt=713721805;if (lRizqqRXoKrDCQkbJbjaneNzedmLNt == lRizqqRXoKrDCQkbJbjaneNzedmLNt- 0 ) lRizqqRXoKrDCQkbJbjaneNzedmLNt=1956146843; else lRizqqRXoKrDCQkbJbjaneNzedmLNt=76576501;if (lRizqqRXoKrDCQkbJbjaneNzedmLNt == lRizqqRXoKrDCQkbJbjaneNzedmLNt- 1 ) lRizqqRXoKrDCQkbJbjaneNzedmLNt=1753572191; else lRizqqRXoKrDCQkbJbjaneNzedmLNt=1089006165;if (lRizqqRXoKrDCQkbJbjaneNzedmLNt == lRizqqRXoKrDCQkbJbjaneNzedmLNt- 1 ) lRizqqRXoKrDCQkbJbjaneNzedmLNt=289312780; else lRizqqRXoKrDCQkbJbjaneNzedmLNt=885264779;long IIetEHeNQMvmigSXKUPAEsarSONeZV=77239852;if (IIetEHeNQMvmigSXKUPAEsarSONeZV == IIetEHeNQMvmigSXKUPAEsarSONeZV- 0 ) IIetEHeNQMvmigSXKUPAEsarSONeZV=775809595; else IIetEHeNQMvmigSXKUPAEsarSONeZV=1544054553;if (IIetEHeNQMvmigSXKUPAEsarSONeZV == IIetEHeNQMvmigSXKUPAEsarSONeZV- 0 ) IIetEHeNQMvmigSXKUPAEsarSONeZV=2111637615; else IIetEHeNQMvmigSXKUPAEsarSONeZV=170694788;if (IIetEHeNQMvmigSXKUPAEsarSONeZV == IIetEHeNQMvmigSXKUPAEsarSONeZV- 0 ) IIetEHeNQMvmigSXKUPAEsarSONeZV=988422060; else IIetEHeNQMvmigSXKUPAEsarSONeZV=343846359;if (IIetEHeNQMvmigSXKUPAEsarSONeZV == IIetEHeNQMvmigSXKUPAEsarSONeZV- 0 ) IIetEHeNQMvmigSXKUPAEsarSONeZV=791792111; else IIetEHeNQMvmigSXKUPAEsarSONeZV=838628459;if (IIetEHeNQMvmigSXKUPAEsarSONeZV == IIetEHeNQMvmigSXKUPAEsarSONeZV- 0 ) IIetEHeNQMvmigSXKUPAEsarSONeZV=422518951; else IIetEHeNQMvmigSXKUPAEsarSONeZV=208331120;if (IIetEHeNQMvmigSXKUPAEsarSONeZV == IIetEHeNQMvmigSXKUPAEsarSONeZV- 1 ) IIetEHeNQMvmigSXKUPAEsarSONeZV=1742892687; else IIetEHeNQMvmigSXKUPAEsarSONeZV=2039944820;double uSHynIRNxcRoFDVrGKcrtPGslyjGrk=864384732.815069765042806257624358838919;if (uSHynIRNxcRoFDVrGKcrtPGslyjGrk == uSHynIRNxcRoFDVrGKcrtPGslyjGrk ) uSHynIRNxcRoFDVrGKcrtPGslyjGrk=212835004.276018937233752947233904756734; else uSHynIRNxcRoFDVrGKcrtPGslyjGrk=2053941403.196797699942150161799189638346;if (uSHynIRNxcRoFDVrGKcrtPGslyjGrk == uSHynIRNxcRoFDVrGKcrtPGslyjGrk ) uSHynIRNxcRoFDVrGKcrtPGslyjGrk=1903949767.221314806536839286470504602826; else uSHynIRNxcRoFDVrGKcrtPGslyjGrk=803538677.035472294646526345045207783881;if (uSHynIRNxcRoFDVrGKcrtPGslyjGrk == uSHynIRNxcRoFDVrGKcrtPGslyjGrk ) uSHynIRNxcRoFDVrGKcrtPGslyjGrk=2019554883.309161157810284408837798184970; else uSHynIRNxcRoFDVrGKcrtPGslyjGrk=1661950022.941344857629924502448505562208;if (uSHynIRNxcRoFDVrGKcrtPGslyjGrk == uSHynIRNxcRoFDVrGKcrtPGslyjGrk ) uSHynIRNxcRoFDVrGKcrtPGslyjGrk=413599393.838797005065221897478254245798; else uSHynIRNxcRoFDVrGKcrtPGslyjGrk=656896389.376990223861858348478584168465;if (uSHynIRNxcRoFDVrGKcrtPGslyjGrk == uSHynIRNxcRoFDVrGKcrtPGslyjGrk ) uSHynIRNxcRoFDVrGKcrtPGslyjGrk=1366732842.232770034277571851791732199294; else uSHynIRNxcRoFDVrGKcrtPGslyjGrk=2048908958.287964918831521112431887561079;if (uSHynIRNxcRoFDVrGKcrtPGslyjGrk == uSHynIRNxcRoFDVrGKcrtPGslyjGrk ) uSHynIRNxcRoFDVrGKcrtPGslyjGrk=1113126968.195119171894496021209896351040; else uSHynIRNxcRoFDVrGKcrtPGslyjGrk=84338132.304975752204264740155097140986;double GNCXyAbHGQluBzVfNNcFFtteQEjpQY=1045521856.071492768861877371695559332583;if (GNCXyAbHGQluBzVfNNcFFtteQEjpQY == GNCXyAbHGQluBzVfNNcFFtteQEjpQY ) GNCXyAbHGQluBzVfNNcFFtteQEjpQY=1207077041.396911916179239502665006652216; else GNCXyAbHGQluBzVfNNcFFtteQEjpQY=1157249686.029558606757663643094002849047;if (GNCXyAbHGQluBzVfNNcFFtteQEjpQY == GNCXyAbHGQluBzVfNNcFFtteQEjpQY ) GNCXyAbHGQluBzVfNNcFFtteQEjpQY=2056721765.970015178291702954401933312882; else GNCXyAbHGQluBzVfNNcFFtteQEjpQY=1563859850.977088766868690441009664605881;if (GNCXyAbHGQluBzVfNNcFFtteQEjpQY == GNCXyAbHGQluBzVfNNcFFtteQEjpQY ) GNCXyAbHGQluBzVfNNcFFtteQEjpQY=1577169377.340633472429974444407288341358; else GNCXyAbHGQluBzVfNNcFFtteQEjpQY=51217893.409653817707979180127249427330;if (GNCXyAbHGQluBzVfNNcFFtteQEjpQY == GNCXyAbHGQluBzVfNNcFFtteQEjpQY ) GNCXyAbHGQluBzVfNNcFFtteQEjpQY=765951043.323413032164870031936123236124; else GNCXyAbHGQluBzVfNNcFFtteQEjpQY=968799041.768758567615906448949640430129;if (GNCXyAbHGQluBzVfNNcFFtteQEjpQY == GNCXyAbHGQluBzVfNNcFFtteQEjpQY ) GNCXyAbHGQluBzVfNNcFFtteQEjpQY=830743308.702851612368533123799938622833; else GNCXyAbHGQluBzVfNNcFFtteQEjpQY=1287537459.844802425684669111841844197434;if (GNCXyAbHGQluBzVfNNcFFtteQEjpQY == GNCXyAbHGQluBzVfNNcFFtteQEjpQY ) GNCXyAbHGQluBzVfNNcFFtteQEjpQY=1269222940.999052474281821637051200210081; else GNCXyAbHGQluBzVfNNcFFtteQEjpQY=1208296195.519270572785776003357158489985;long UuSjjJfXJBicPmgXRTLFomOVVgGryD=2053610848;if (UuSjjJfXJBicPmgXRTLFomOVVgGryD == UuSjjJfXJBicPmgXRTLFomOVVgGryD- 0 ) UuSjjJfXJBicPmgXRTLFomOVVgGryD=1878638437; else UuSjjJfXJBicPmgXRTLFomOVVgGryD=1572411695;if (UuSjjJfXJBicPmgXRTLFomOVVgGryD == UuSjjJfXJBicPmgXRTLFomOVVgGryD- 1 ) UuSjjJfXJBicPmgXRTLFomOVVgGryD=672429775; else UuSjjJfXJBicPmgXRTLFomOVVgGryD=516371819;if (UuSjjJfXJBicPmgXRTLFomOVVgGryD == UuSjjJfXJBicPmgXRTLFomOVVgGryD- 1 ) UuSjjJfXJBicPmgXRTLFomOVVgGryD=768900929; else UuSjjJfXJBicPmgXRTLFomOVVgGryD=2035952542;if (UuSjjJfXJBicPmgXRTLFomOVVgGryD == UuSjjJfXJBicPmgXRTLFomOVVgGryD- 1 ) UuSjjJfXJBicPmgXRTLFomOVVgGryD=205168616; else UuSjjJfXJBicPmgXRTLFomOVVgGryD=1261343220;if (UuSjjJfXJBicPmgXRTLFomOVVgGryD == UuSjjJfXJBicPmgXRTLFomOVVgGryD- 0 ) UuSjjJfXJBicPmgXRTLFomOVVgGryD=1130455160; else UuSjjJfXJBicPmgXRTLFomOVVgGryD=1454741226;if (UuSjjJfXJBicPmgXRTLFomOVVgGryD == UuSjjJfXJBicPmgXRTLFomOVVgGryD- 1 ) UuSjjJfXJBicPmgXRTLFomOVVgGryD=951445593; else UuSjjJfXJBicPmgXRTLFomOVVgGryD=1086996749;long CkOxPxMRSThKmFaHCcJVDuazapjEHi=1434925368;if (CkOxPxMRSThKmFaHCcJVDuazapjEHi == CkOxPxMRSThKmFaHCcJVDuazapjEHi- 0 ) CkOxPxMRSThKmFaHCcJVDuazapjEHi=1932304596; else CkOxPxMRSThKmFaHCcJVDuazapjEHi=697370831;if (CkOxPxMRSThKmFaHCcJVDuazapjEHi == CkOxPxMRSThKmFaHCcJVDuazapjEHi- 1 ) CkOxPxMRSThKmFaHCcJVDuazapjEHi=832867779; else CkOxPxMRSThKmFaHCcJVDuazapjEHi=278364550;if (CkOxPxMRSThKmFaHCcJVDuazapjEHi == CkOxPxMRSThKmFaHCcJVDuazapjEHi- 1 ) CkOxPxMRSThKmFaHCcJVDuazapjEHi=1938206872; else CkOxPxMRSThKmFaHCcJVDuazapjEHi=1941475172;if (CkOxPxMRSThKmFaHCcJVDuazapjEHi == CkOxPxMRSThKmFaHCcJVDuazapjEHi- 0 ) CkOxPxMRSThKmFaHCcJVDuazapjEHi=1434699002; else CkOxPxMRSThKmFaHCcJVDuazapjEHi=161617111;if (CkOxPxMRSThKmFaHCcJVDuazapjEHi == CkOxPxMRSThKmFaHCcJVDuazapjEHi- 0 ) CkOxPxMRSThKmFaHCcJVDuazapjEHi=1287990360; else CkOxPxMRSThKmFaHCcJVDuazapjEHi=935150726;if (CkOxPxMRSThKmFaHCcJVDuazapjEHi == CkOxPxMRSThKmFaHCcJVDuazapjEHi- 1 ) CkOxPxMRSThKmFaHCcJVDuazapjEHi=2120279016; else CkOxPxMRSThKmFaHCcJVDuazapjEHi=732017424;int qBDmegdxQrGQRdDAvhKAQKWmNkPBip=1450530819;if (qBDmegdxQrGQRdDAvhKAQKWmNkPBip == qBDmegdxQrGQRdDAvhKAQKWmNkPBip- 1 ) qBDmegdxQrGQRdDAvhKAQKWmNkPBip=1260463046; else qBDmegdxQrGQRdDAvhKAQKWmNkPBip=1206999767;if (qBDmegdxQrGQRdDAvhKAQKWmNkPBip == qBDmegdxQrGQRdDAvhKAQKWmNkPBip- 1 ) qBDmegdxQrGQRdDAvhKAQKWmNkPBip=353183342; else qBDmegdxQrGQRdDAvhKAQKWmNkPBip=909488529;if (qBDmegdxQrGQRdDAvhKAQKWmNkPBip == qBDmegdxQrGQRdDAvhKAQKWmNkPBip- 0 ) qBDmegdxQrGQRdDAvhKAQKWmNkPBip=462647455; else qBDmegdxQrGQRdDAvhKAQKWmNkPBip=2054621999;if (qBDmegdxQrGQRdDAvhKAQKWmNkPBip == qBDmegdxQrGQRdDAvhKAQKWmNkPBip- 1 ) qBDmegdxQrGQRdDAvhKAQKWmNkPBip=276620927; else qBDmegdxQrGQRdDAvhKAQKWmNkPBip=1318608146;if (qBDmegdxQrGQRdDAvhKAQKWmNkPBip == qBDmegdxQrGQRdDAvhKAQKWmNkPBip- 1 ) qBDmegdxQrGQRdDAvhKAQKWmNkPBip=952081070; else qBDmegdxQrGQRdDAvhKAQKWmNkPBip=795378545;if (qBDmegdxQrGQRdDAvhKAQKWmNkPBip == qBDmegdxQrGQRdDAvhKAQKWmNkPBip- 0 ) qBDmegdxQrGQRdDAvhKAQKWmNkPBip=433404690; else qBDmegdxQrGQRdDAvhKAQKWmNkPBip=122428151;long RJUoumNWoPmoOxxaJAqXqXggmUvvTb=123289329;if (RJUoumNWoPmoOxxaJAqXqXggmUvvTb == RJUoumNWoPmoOxxaJAqXqXggmUvvTb- 0 ) RJUoumNWoPmoOxxaJAqXqXggmUvvTb=783325577; else RJUoumNWoPmoOxxaJAqXqXggmUvvTb=1981781219;if (RJUoumNWoPmoOxxaJAqXqXggmUvvTb == RJUoumNWoPmoOxxaJAqXqXggmUvvTb- 1 ) RJUoumNWoPmoOxxaJAqXqXggmUvvTb=1423475246; else RJUoumNWoPmoOxxaJAqXqXggmUvvTb=420941934;if (RJUoumNWoPmoOxxaJAqXqXggmUvvTb == RJUoumNWoPmoOxxaJAqXqXggmUvvTb- 0 ) RJUoumNWoPmoOxxaJAqXqXggmUvvTb=1230817891; else RJUoumNWoPmoOxxaJAqXqXggmUvvTb=510081362;if (RJUoumNWoPmoOxxaJAqXqXggmUvvTb == RJUoumNWoPmoOxxaJAqXqXggmUvvTb- 0 ) RJUoumNWoPmoOxxaJAqXqXggmUvvTb=115403976; else RJUoumNWoPmoOxxaJAqXqXggmUvvTb=2084646912;if (RJUoumNWoPmoOxxaJAqXqXggmUvvTb == RJUoumNWoPmoOxxaJAqXqXggmUvvTb- 1 ) RJUoumNWoPmoOxxaJAqXqXggmUvvTb=137712693; else RJUoumNWoPmoOxxaJAqXqXggmUvvTb=510812664;if (RJUoumNWoPmoOxxaJAqXqXggmUvvTb == RJUoumNWoPmoOxxaJAqXqXggmUvvTb- 0 ) RJUoumNWoPmoOxxaJAqXqXggmUvvTb=1375865106; else RJUoumNWoPmoOxxaJAqXqXggmUvvTb=1981233850;long HqlqujDUAtXHLVXjoYuFNmRAMGnOBu=1925151962;if (HqlqujDUAtXHLVXjoYuFNmRAMGnOBu == HqlqujDUAtXHLVXjoYuFNmRAMGnOBu- 1 ) HqlqujDUAtXHLVXjoYuFNmRAMGnOBu=1916157602; else HqlqujDUAtXHLVXjoYuFNmRAMGnOBu=1648398421;if (HqlqujDUAtXHLVXjoYuFNmRAMGnOBu == HqlqujDUAtXHLVXjoYuFNmRAMGnOBu- 1 ) HqlqujDUAtXHLVXjoYuFNmRAMGnOBu=1974017350; else HqlqujDUAtXHLVXjoYuFNmRAMGnOBu=171203271;if (HqlqujDUAtXHLVXjoYuFNmRAMGnOBu == HqlqujDUAtXHLVXjoYuFNmRAMGnOBu- 1 ) HqlqujDUAtXHLVXjoYuFNmRAMGnOBu=354270637; else HqlqujDUAtXHLVXjoYuFNmRAMGnOBu=1742207000;if (HqlqujDUAtXHLVXjoYuFNmRAMGnOBu == HqlqujDUAtXHLVXjoYuFNmRAMGnOBu- 0 ) HqlqujDUAtXHLVXjoYuFNmRAMGnOBu=972277913; else HqlqujDUAtXHLVXjoYuFNmRAMGnOBu=2025166244;if (HqlqujDUAtXHLVXjoYuFNmRAMGnOBu == HqlqujDUAtXHLVXjoYuFNmRAMGnOBu- 0 ) HqlqujDUAtXHLVXjoYuFNmRAMGnOBu=420749904; else HqlqujDUAtXHLVXjoYuFNmRAMGnOBu=1661743280;if (HqlqujDUAtXHLVXjoYuFNmRAMGnOBu == HqlqujDUAtXHLVXjoYuFNmRAMGnOBu- 1 ) HqlqujDUAtXHLVXjoYuFNmRAMGnOBu=1473767127; else HqlqujDUAtXHLVXjoYuFNmRAMGnOBu=1142828327;double ieZOhyHPKRUglCMPGOyaZwFngNnwKy=1609036066.217033375961468529971047659241;if (ieZOhyHPKRUglCMPGOyaZwFngNnwKy == ieZOhyHPKRUglCMPGOyaZwFngNnwKy ) ieZOhyHPKRUglCMPGOyaZwFngNnwKy=1344554723.497376093385902244764359727058; else ieZOhyHPKRUglCMPGOyaZwFngNnwKy=1996989687.424567520263770725542668724865;if (ieZOhyHPKRUglCMPGOyaZwFngNnwKy == ieZOhyHPKRUglCMPGOyaZwFngNnwKy ) ieZOhyHPKRUglCMPGOyaZwFngNnwKy=2096175494.005697255682451269011261797684; else ieZOhyHPKRUglCMPGOyaZwFngNnwKy=558429510.377636828357107021866686485590;if (ieZOhyHPKRUglCMPGOyaZwFngNnwKy == ieZOhyHPKRUglCMPGOyaZwFngNnwKy ) ieZOhyHPKRUglCMPGOyaZwFngNnwKy=2076795246.167141693645186426578692980126; else ieZOhyHPKRUglCMPGOyaZwFngNnwKy=1265125337.143839107653923151106002358536;if (ieZOhyHPKRUglCMPGOyaZwFngNnwKy == ieZOhyHPKRUglCMPGOyaZwFngNnwKy ) ieZOhyHPKRUglCMPGOyaZwFngNnwKy=219712928.168750345743093577083012124168; else ieZOhyHPKRUglCMPGOyaZwFngNnwKy=750381684.268150628381647402098255325673;if (ieZOhyHPKRUglCMPGOyaZwFngNnwKy == ieZOhyHPKRUglCMPGOyaZwFngNnwKy ) ieZOhyHPKRUglCMPGOyaZwFngNnwKy=1091595791.729868594496980257683327001484; else ieZOhyHPKRUglCMPGOyaZwFngNnwKy=416020077.516577389991960338517910150296;if (ieZOhyHPKRUglCMPGOyaZwFngNnwKy == ieZOhyHPKRUglCMPGOyaZwFngNnwKy ) ieZOhyHPKRUglCMPGOyaZwFngNnwKy=771775836.722009051407870247878397861743; else ieZOhyHPKRUglCMPGOyaZwFngNnwKy=2099318438.720427097249812014101452844534;long SCrlomVXPKZyfQuepbnZgdYLOmocpP=1478929878;if (SCrlomVXPKZyfQuepbnZgdYLOmocpP == SCrlomVXPKZyfQuepbnZgdYLOmocpP- 0 ) SCrlomVXPKZyfQuepbnZgdYLOmocpP=2074246054; else SCrlomVXPKZyfQuepbnZgdYLOmocpP=655736375;if (SCrlomVXPKZyfQuepbnZgdYLOmocpP == SCrlomVXPKZyfQuepbnZgdYLOmocpP- 0 ) SCrlomVXPKZyfQuepbnZgdYLOmocpP=353754196; else SCrlomVXPKZyfQuepbnZgdYLOmocpP=1746341173;if (SCrlomVXPKZyfQuepbnZgdYLOmocpP == SCrlomVXPKZyfQuepbnZgdYLOmocpP- 1 ) SCrlomVXPKZyfQuepbnZgdYLOmocpP=1999904238; else SCrlomVXPKZyfQuepbnZgdYLOmocpP=1566876379;if (SCrlomVXPKZyfQuepbnZgdYLOmocpP == SCrlomVXPKZyfQuepbnZgdYLOmocpP- 1 ) SCrlomVXPKZyfQuepbnZgdYLOmocpP=1916502339; else SCrlomVXPKZyfQuepbnZgdYLOmocpP=1188602224;if (SCrlomVXPKZyfQuepbnZgdYLOmocpP == SCrlomVXPKZyfQuepbnZgdYLOmocpP- 0 ) SCrlomVXPKZyfQuepbnZgdYLOmocpP=1009503306; else SCrlomVXPKZyfQuepbnZgdYLOmocpP=1545792649;if (SCrlomVXPKZyfQuepbnZgdYLOmocpP == SCrlomVXPKZyfQuepbnZgdYLOmocpP- 0 ) SCrlomVXPKZyfQuepbnZgdYLOmocpP=192030; else SCrlomVXPKZyfQuepbnZgdYLOmocpP=1716558258;double LUwZkPwAiffeQSZSSgnWPUyleTRWRW=972925489.159329996487241966467263204336;if (LUwZkPwAiffeQSZSSgnWPUyleTRWRW == LUwZkPwAiffeQSZSSgnWPUyleTRWRW ) LUwZkPwAiffeQSZSSgnWPUyleTRWRW=1596451500.732341048071144388860005400612; else LUwZkPwAiffeQSZSSgnWPUyleTRWRW=1914312687.566176591002849613903668673704;if (LUwZkPwAiffeQSZSSgnWPUyleTRWRW == LUwZkPwAiffeQSZSSgnWPUyleTRWRW ) LUwZkPwAiffeQSZSSgnWPUyleTRWRW=636679127.244651617990887926780872091850; else LUwZkPwAiffeQSZSSgnWPUyleTRWRW=66167486.380796930811602498186813557276;if (LUwZkPwAiffeQSZSSgnWPUyleTRWRW == LUwZkPwAiffeQSZSSgnWPUyleTRWRW ) LUwZkPwAiffeQSZSSgnWPUyleTRWRW=1671942992.186771010100797928171387558735; else LUwZkPwAiffeQSZSSgnWPUyleTRWRW=310186628.976483503248918722663021691033;if (LUwZkPwAiffeQSZSSgnWPUyleTRWRW == LUwZkPwAiffeQSZSSgnWPUyleTRWRW ) LUwZkPwAiffeQSZSSgnWPUyleTRWRW=1995840363.348534442553504347084617205689; else LUwZkPwAiffeQSZSSgnWPUyleTRWRW=651032265.267190058549122153835405073144;if (LUwZkPwAiffeQSZSSgnWPUyleTRWRW == LUwZkPwAiffeQSZSSgnWPUyleTRWRW ) LUwZkPwAiffeQSZSSgnWPUyleTRWRW=1428685493.086340651430065660615296486322; else LUwZkPwAiffeQSZSSgnWPUyleTRWRW=1223635666.801556488261341655052364980096;if (LUwZkPwAiffeQSZSSgnWPUyleTRWRW == LUwZkPwAiffeQSZSSgnWPUyleTRWRW ) LUwZkPwAiffeQSZSSgnWPUyleTRWRW=1227091127.544974951039246405693138154868; else LUwZkPwAiffeQSZSSgnWPUyleTRWRW=2085734207.749383848771513312868551341683;float ItMQuzygPhfntLOdJaqQOxDBIrqwUH=2125362827.022348001358013782279576655998f;if (ItMQuzygPhfntLOdJaqQOxDBIrqwUH - ItMQuzygPhfntLOdJaqQOxDBIrqwUH> 0.00000001 ) ItMQuzygPhfntLOdJaqQOxDBIrqwUH=258808660.558970294403680720532048355779f; else ItMQuzygPhfntLOdJaqQOxDBIrqwUH=2142320896.321481024369998790604646651172f;if (ItMQuzygPhfntLOdJaqQOxDBIrqwUH - ItMQuzygPhfntLOdJaqQOxDBIrqwUH> 0.00000001 ) ItMQuzygPhfntLOdJaqQOxDBIrqwUH=1216065920.653528230971628326076518679583f; else ItMQuzygPhfntLOdJaqQOxDBIrqwUH=852334107.105281757157183402944520715595f;if (ItMQuzygPhfntLOdJaqQOxDBIrqwUH - ItMQuzygPhfntLOdJaqQOxDBIrqwUH> 0.00000001 ) ItMQuzygPhfntLOdJaqQOxDBIrqwUH=958516655.085898655877093054527775978668f; else ItMQuzygPhfntLOdJaqQOxDBIrqwUH=1462987285.626327230596818863877208832192f;if (ItMQuzygPhfntLOdJaqQOxDBIrqwUH - ItMQuzygPhfntLOdJaqQOxDBIrqwUH> 0.00000001 ) ItMQuzygPhfntLOdJaqQOxDBIrqwUH=1756615475.019725178682514196822589253959f; else ItMQuzygPhfntLOdJaqQOxDBIrqwUH=1925161991.625859719379520580474638163168f;if (ItMQuzygPhfntLOdJaqQOxDBIrqwUH - ItMQuzygPhfntLOdJaqQOxDBIrqwUH> 0.00000001 ) ItMQuzygPhfntLOdJaqQOxDBIrqwUH=80487348.333007876659756316077358509870f; else ItMQuzygPhfntLOdJaqQOxDBIrqwUH=907573270.675215663381593609395720108650f;if (ItMQuzygPhfntLOdJaqQOxDBIrqwUH - ItMQuzygPhfntLOdJaqQOxDBIrqwUH> 0.00000001 ) ItMQuzygPhfntLOdJaqQOxDBIrqwUH=1696409851.911608464185479787501169596611f; else ItMQuzygPhfntLOdJaqQOxDBIrqwUH=531002597.812358009635522145153116085312f;long nWQUiPGNHiopxihgVkWbGZhshAhPiN=2119020142;if (nWQUiPGNHiopxihgVkWbGZhshAhPiN == nWQUiPGNHiopxihgVkWbGZhshAhPiN- 0 ) nWQUiPGNHiopxihgVkWbGZhshAhPiN=1986927294; else nWQUiPGNHiopxihgVkWbGZhshAhPiN=1946333994;if (nWQUiPGNHiopxihgVkWbGZhshAhPiN == nWQUiPGNHiopxihgVkWbGZhshAhPiN- 0 ) nWQUiPGNHiopxihgVkWbGZhshAhPiN=502866938; else nWQUiPGNHiopxihgVkWbGZhshAhPiN=911927853;if (nWQUiPGNHiopxihgVkWbGZhshAhPiN == nWQUiPGNHiopxihgVkWbGZhshAhPiN- 0 ) nWQUiPGNHiopxihgVkWbGZhshAhPiN=1953675244; else nWQUiPGNHiopxihgVkWbGZhshAhPiN=1336767134;if (nWQUiPGNHiopxihgVkWbGZhshAhPiN == nWQUiPGNHiopxihgVkWbGZhshAhPiN- 0 ) nWQUiPGNHiopxihgVkWbGZhshAhPiN=1954470533; else nWQUiPGNHiopxihgVkWbGZhshAhPiN=1764059426;if (nWQUiPGNHiopxihgVkWbGZhshAhPiN == nWQUiPGNHiopxihgVkWbGZhshAhPiN- 1 ) nWQUiPGNHiopxihgVkWbGZhshAhPiN=807379659; else nWQUiPGNHiopxihgVkWbGZhshAhPiN=1850205578;if (nWQUiPGNHiopxihgVkWbGZhshAhPiN == nWQUiPGNHiopxihgVkWbGZhshAhPiN- 0 ) nWQUiPGNHiopxihgVkWbGZhshAhPiN=317655680; else nWQUiPGNHiopxihgVkWbGZhshAhPiN=776268572;long aMUpdGMRzQWiZHVaTECkojoVnUREqy=2136543831;if (aMUpdGMRzQWiZHVaTECkojoVnUREqy == aMUpdGMRzQWiZHVaTECkojoVnUREqy- 0 ) aMUpdGMRzQWiZHVaTECkojoVnUREqy=199383298; else aMUpdGMRzQWiZHVaTECkojoVnUREqy=22312850;if (aMUpdGMRzQWiZHVaTECkojoVnUREqy == aMUpdGMRzQWiZHVaTECkojoVnUREqy- 1 ) aMUpdGMRzQWiZHVaTECkojoVnUREqy=1457749598; else aMUpdGMRzQWiZHVaTECkojoVnUREqy=1188960633;if (aMUpdGMRzQWiZHVaTECkojoVnUREqy == aMUpdGMRzQWiZHVaTECkojoVnUREqy- 0 ) aMUpdGMRzQWiZHVaTECkojoVnUREqy=2051477023; else aMUpdGMRzQWiZHVaTECkojoVnUREqy=1871725970;if (aMUpdGMRzQWiZHVaTECkojoVnUREqy == aMUpdGMRzQWiZHVaTECkojoVnUREqy- 0 ) aMUpdGMRzQWiZHVaTECkojoVnUREqy=14408834; else aMUpdGMRzQWiZHVaTECkojoVnUREqy=133464215;if (aMUpdGMRzQWiZHVaTECkojoVnUREqy == aMUpdGMRzQWiZHVaTECkojoVnUREqy- 0 ) aMUpdGMRzQWiZHVaTECkojoVnUREqy=157697212; else aMUpdGMRzQWiZHVaTECkojoVnUREqy=859000783;if (aMUpdGMRzQWiZHVaTECkojoVnUREqy == aMUpdGMRzQWiZHVaTECkojoVnUREqy- 1 ) aMUpdGMRzQWiZHVaTECkojoVnUREqy=2133163785; else aMUpdGMRzQWiZHVaTECkojoVnUREqy=764369722;long knNfWxFItQLCRksCpijUmramWNdici=778047176;if (knNfWxFItQLCRksCpijUmramWNdici == knNfWxFItQLCRksCpijUmramWNdici- 1 ) knNfWxFItQLCRksCpijUmramWNdici=1657051864; else knNfWxFItQLCRksCpijUmramWNdici=1255554632;if (knNfWxFItQLCRksCpijUmramWNdici == knNfWxFItQLCRksCpijUmramWNdici- 0 ) knNfWxFItQLCRksCpijUmramWNdici=98806913; else knNfWxFItQLCRksCpijUmramWNdici=1171580817;if (knNfWxFItQLCRksCpijUmramWNdici == knNfWxFItQLCRksCpijUmramWNdici- 0 ) knNfWxFItQLCRksCpijUmramWNdici=517576184; else knNfWxFItQLCRksCpijUmramWNdici=434481498;if (knNfWxFItQLCRksCpijUmramWNdici == knNfWxFItQLCRksCpijUmramWNdici- 1 ) knNfWxFItQLCRksCpijUmramWNdici=171687583; else knNfWxFItQLCRksCpijUmramWNdici=1069525173;if (knNfWxFItQLCRksCpijUmramWNdici == knNfWxFItQLCRksCpijUmramWNdici- 1 ) knNfWxFItQLCRksCpijUmramWNdici=187850363; else knNfWxFItQLCRksCpijUmramWNdici=1599490141;if (knNfWxFItQLCRksCpijUmramWNdici == knNfWxFItQLCRksCpijUmramWNdici- 0 ) knNfWxFItQLCRksCpijUmramWNdici=44954448; else knNfWxFItQLCRksCpijUmramWNdici=1255794724;long dKGOTLkOXdDcPCTRXAyaWPDClvXLVh=249719216;if (dKGOTLkOXdDcPCTRXAyaWPDClvXLVh == dKGOTLkOXdDcPCTRXAyaWPDClvXLVh- 1 ) dKGOTLkOXdDcPCTRXAyaWPDClvXLVh=689402210; else dKGOTLkOXdDcPCTRXAyaWPDClvXLVh=1707349667;if (dKGOTLkOXdDcPCTRXAyaWPDClvXLVh == dKGOTLkOXdDcPCTRXAyaWPDClvXLVh- 0 ) dKGOTLkOXdDcPCTRXAyaWPDClvXLVh=331619299; else dKGOTLkOXdDcPCTRXAyaWPDClvXLVh=1242620457;if (dKGOTLkOXdDcPCTRXAyaWPDClvXLVh == dKGOTLkOXdDcPCTRXAyaWPDClvXLVh- 0 ) dKGOTLkOXdDcPCTRXAyaWPDClvXLVh=1340372366; else dKGOTLkOXdDcPCTRXAyaWPDClvXLVh=525106816;if (dKGOTLkOXdDcPCTRXAyaWPDClvXLVh == dKGOTLkOXdDcPCTRXAyaWPDClvXLVh- 1 ) dKGOTLkOXdDcPCTRXAyaWPDClvXLVh=67543119; else dKGOTLkOXdDcPCTRXAyaWPDClvXLVh=115201324;if (dKGOTLkOXdDcPCTRXAyaWPDClvXLVh == dKGOTLkOXdDcPCTRXAyaWPDClvXLVh- 1 ) dKGOTLkOXdDcPCTRXAyaWPDClvXLVh=1931925160; else dKGOTLkOXdDcPCTRXAyaWPDClvXLVh=369402723;if (dKGOTLkOXdDcPCTRXAyaWPDClvXLVh == dKGOTLkOXdDcPCTRXAyaWPDClvXLVh- 1 ) dKGOTLkOXdDcPCTRXAyaWPDClvXLVh=754230641; else dKGOTLkOXdDcPCTRXAyaWPDClvXLVh=1094674461;int MuHykIZAtGNMNmiARuoLThuExLGXZN=1490025540;if (MuHykIZAtGNMNmiARuoLThuExLGXZN == MuHykIZAtGNMNmiARuoLThuExLGXZN- 0 ) MuHykIZAtGNMNmiARuoLThuExLGXZN=316622973; else MuHykIZAtGNMNmiARuoLThuExLGXZN=1687092151;if (MuHykIZAtGNMNmiARuoLThuExLGXZN == MuHykIZAtGNMNmiARuoLThuExLGXZN- 1 ) MuHykIZAtGNMNmiARuoLThuExLGXZN=1266700355; else MuHykIZAtGNMNmiARuoLThuExLGXZN=1231714267;if (MuHykIZAtGNMNmiARuoLThuExLGXZN == MuHykIZAtGNMNmiARuoLThuExLGXZN- 0 ) MuHykIZAtGNMNmiARuoLThuExLGXZN=1879444866; else MuHykIZAtGNMNmiARuoLThuExLGXZN=1194073890;if (MuHykIZAtGNMNmiARuoLThuExLGXZN == MuHykIZAtGNMNmiARuoLThuExLGXZN- 1 ) MuHykIZAtGNMNmiARuoLThuExLGXZN=1618967647; else MuHykIZAtGNMNmiARuoLThuExLGXZN=1912385447;if (MuHykIZAtGNMNmiARuoLThuExLGXZN == MuHykIZAtGNMNmiARuoLThuExLGXZN- 1 ) MuHykIZAtGNMNmiARuoLThuExLGXZN=1998108914; else MuHykIZAtGNMNmiARuoLThuExLGXZN=388224425;if (MuHykIZAtGNMNmiARuoLThuExLGXZN == MuHykIZAtGNMNmiARuoLThuExLGXZN- 0 ) MuHykIZAtGNMNmiARuoLThuExLGXZN=1001110270; else MuHykIZAtGNMNmiARuoLThuExLGXZN=451986882;float dHmgYPZklkOPSevczHlXiosfTvQnbI=1324833956.319619649153468147029682868214f;if (dHmgYPZklkOPSevczHlXiosfTvQnbI - dHmgYPZklkOPSevczHlXiosfTvQnbI> 0.00000001 ) dHmgYPZklkOPSevczHlXiosfTvQnbI=1421107691.200389420742103942388526673896f; else dHmgYPZklkOPSevczHlXiosfTvQnbI=1883444569.143505318983839858993704779481f;if (dHmgYPZklkOPSevczHlXiosfTvQnbI - dHmgYPZklkOPSevczHlXiosfTvQnbI> 0.00000001 ) dHmgYPZklkOPSevczHlXiosfTvQnbI=74967512.663570970863441589792591345084f; else dHmgYPZklkOPSevczHlXiosfTvQnbI=1201394404.704832037099913046329079027323f;if (dHmgYPZklkOPSevczHlXiosfTvQnbI - dHmgYPZklkOPSevczHlXiosfTvQnbI> 0.00000001 ) dHmgYPZklkOPSevczHlXiosfTvQnbI=1133218467.590727071956381479841086583592f; else dHmgYPZklkOPSevczHlXiosfTvQnbI=290962148.581550930717548480965969262425f;if (dHmgYPZklkOPSevczHlXiosfTvQnbI - dHmgYPZklkOPSevczHlXiosfTvQnbI> 0.00000001 ) dHmgYPZklkOPSevczHlXiosfTvQnbI=1585158457.459958584155923454920463863349f; else dHmgYPZklkOPSevczHlXiosfTvQnbI=1131945048.861636960091692114682960519729f;if (dHmgYPZklkOPSevczHlXiosfTvQnbI - dHmgYPZklkOPSevczHlXiosfTvQnbI> 0.00000001 ) dHmgYPZklkOPSevczHlXiosfTvQnbI=1188011513.635530430769504088186976801123f; else dHmgYPZklkOPSevczHlXiosfTvQnbI=2131089236.835156626378503356763165377974f;if (dHmgYPZklkOPSevczHlXiosfTvQnbI - dHmgYPZklkOPSevczHlXiosfTvQnbI> 0.00000001 ) dHmgYPZklkOPSevczHlXiosfTvQnbI=1387139304.042803446947392957456557078198f; else dHmgYPZklkOPSevczHlXiosfTvQnbI=148173461.985233680092192787353163960683f;float QZnyiKGfsLijUdVQeYSsHoRYsLLQoQ=1145233199.178316232836311510464099553605f;if (QZnyiKGfsLijUdVQeYSsHoRYsLLQoQ - QZnyiKGfsLijUdVQeYSsHoRYsLLQoQ> 0.00000001 ) QZnyiKGfsLijUdVQeYSsHoRYsLLQoQ=596691139.993997776518210031281738628997f; else QZnyiKGfsLijUdVQeYSsHoRYsLLQoQ=702412555.364805352484495789953342426637f;if (QZnyiKGfsLijUdVQeYSsHoRYsLLQoQ - QZnyiKGfsLijUdVQeYSsHoRYsLLQoQ> 0.00000001 ) QZnyiKGfsLijUdVQeYSsHoRYsLLQoQ=939171751.998035551689981745304005412248f; else QZnyiKGfsLijUdVQeYSsHoRYsLLQoQ=1605723101.982660646134847547279234309472f;if (QZnyiKGfsLijUdVQeYSsHoRYsLLQoQ - QZnyiKGfsLijUdVQeYSsHoRYsLLQoQ> 0.00000001 ) QZnyiKGfsLijUdVQeYSsHoRYsLLQoQ=1861130195.331381743210927127897618085836f; else QZnyiKGfsLijUdVQeYSsHoRYsLLQoQ=353662472.137580367781516942739279683441f;if (QZnyiKGfsLijUdVQeYSsHoRYsLLQoQ - QZnyiKGfsLijUdVQeYSsHoRYsLLQoQ> 0.00000001 ) QZnyiKGfsLijUdVQeYSsHoRYsLLQoQ=517757997.692719454796111721225411796901f; else QZnyiKGfsLijUdVQeYSsHoRYsLLQoQ=1642811967.893610080859814326896232738112f;if (QZnyiKGfsLijUdVQeYSsHoRYsLLQoQ - QZnyiKGfsLijUdVQeYSsHoRYsLLQoQ> 0.00000001 ) QZnyiKGfsLijUdVQeYSsHoRYsLLQoQ=88382020.356400450285157671758121567109f; else QZnyiKGfsLijUdVQeYSsHoRYsLLQoQ=566717499.297851442507662228617856434594f;if (QZnyiKGfsLijUdVQeYSsHoRYsLLQoQ - QZnyiKGfsLijUdVQeYSsHoRYsLLQoQ> 0.00000001 ) QZnyiKGfsLijUdVQeYSsHoRYsLLQoQ=1391995190.345219831723036156945959254242f; else QZnyiKGfsLijUdVQeYSsHoRYsLLQoQ=952147941.796826256359800583149875949617f;long gtsvPiJDwuyCeweFcyjiIBEPvLDunn=1025827403;if (gtsvPiJDwuyCeweFcyjiIBEPvLDunn == gtsvPiJDwuyCeweFcyjiIBEPvLDunn- 0 ) gtsvPiJDwuyCeweFcyjiIBEPvLDunn=1381270113; else gtsvPiJDwuyCeweFcyjiIBEPvLDunn=1576880332;if (gtsvPiJDwuyCeweFcyjiIBEPvLDunn == gtsvPiJDwuyCeweFcyjiIBEPvLDunn- 0 ) gtsvPiJDwuyCeweFcyjiIBEPvLDunn=1821050417; else gtsvPiJDwuyCeweFcyjiIBEPvLDunn=1615126074;if (gtsvPiJDwuyCeweFcyjiIBEPvLDunn == gtsvPiJDwuyCeweFcyjiIBEPvLDunn- 0 ) gtsvPiJDwuyCeweFcyjiIBEPvLDunn=1115133299; else gtsvPiJDwuyCeweFcyjiIBEPvLDunn=1948888245;if (gtsvPiJDwuyCeweFcyjiIBEPvLDunn == gtsvPiJDwuyCeweFcyjiIBEPvLDunn- 1 ) gtsvPiJDwuyCeweFcyjiIBEPvLDunn=356807073; else gtsvPiJDwuyCeweFcyjiIBEPvLDunn=25660825;if (gtsvPiJDwuyCeweFcyjiIBEPvLDunn == gtsvPiJDwuyCeweFcyjiIBEPvLDunn- 1 ) gtsvPiJDwuyCeweFcyjiIBEPvLDunn=101933694; else gtsvPiJDwuyCeweFcyjiIBEPvLDunn=134755307;if (gtsvPiJDwuyCeweFcyjiIBEPvLDunn == gtsvPiJDwuyCeweFcyjiIBEPvLDunn- 0 ) gtsvPiJDwuyCeweFcyjiIBEPvLDunn=43702754; else gtsvPiJDwuyCeweFcyjiIBEPvLDunn=1895839277;float WODnDveNTxDdxrnBPMyxHzjolWBReK=773612145.167389902452053294625852280562f;if (WODnDveNTxDdxrnBPMyxHzjolWBReK - WODnDveNTxDdxrnBPMyxHzjolWBReK> 0.00000001 ) WODnDveNTxDdxrnBPMyxHzjolWBReK=669786248.410084310476874604480405831520f; else WODnDveNTxDdxrnBPMyxHzjolWBReK=2003360718.055014317999525919878618312536f;if (WODnDveNTxDdxrnBPMyxHzjolWBReK - WODnDveNTxDdxrnBPMyxHzjolWBReK> 0.00000001 ) WODnDveNTxDdxrnBPMyxHzjolWBReK=2002779390.489053287030850109193376344781f; else WODnDveNTxDdxrnBPMyxHzjolWBReK=1124358967.389262057283294997921354321481f;if (WODnDveNTxDdxrnBPMyxHzjolWBReK - WODnDveNTxDdxrnBPMyxHzjolWBReK> 0.00000001 ) WODnDveNTxDdxrnBPMyxHzjolWBReK=2114409653.051776170825292119246896003595f; else WODnDveNTxDdxrnBPMyxHzjolWBReK=1677376803.167633362882811627331324636577f;if (WODnDveNTxDdxrnBPMyxHzjolWBReK - WODnDveNTxDdxrnBPMyxHzjolWBReK> 0.00000001 ) WODnDveNTxDdxrnBPMyxHzjolWBReK=1611187408.378586967085857409649737485310f; else WODnDveNTxDdxrnBPMyxHzjolWBReK=1067445219.790562533687135758823894964120f;if (WODnDveNTxDdxrnBPMyxHzjolWBReK - WODnDveNTxDdxrnBPMyxHzjolWBReK> 0.00000001 ) WODnDveNTxDdxrnBPMyxHzjolWBReK=1365686572.265730649598716370274760890660f; else WODnDveNTxDdxrnBPMyxHzjolWBReK=579639192.851353241622033529870495751694f;if (WODnDveNTxDdxrnBPMyxHzjolWBReK - WODnDveNTxDdxrnBPMyxHzjolWBReK> 0.00000001 ) WODnDveNTxDdxrnBPMyxHzjolWBReK=1113012384.008985746413076283738290146760f; else WODnDveNTxDdxrnBPMyxHzjolWBReK=566500968.266772516998446753068907791994f;long BpfLbnRySkjhqUxpYRSySlzJQfrxqF=1572455276;if (BpfLbnRySkjhqUxpYRSySlzJQfrxqF == BpfLbnRySkjhqUxpYRSySlzJQfrxqF- 1 ) BpfLbnRySkjhqUxpYRSySlzJQfrxqF=1695175787; else BpfLbnRySkjhqUxpYRSySlzJQfrxqF=361311901;if (BpfLbnRySkjhqUxpYRSySlzJQfrxqF == BpfLbnRySkjhqUxpYRSySlzJQfrxqF- 1 ) BpfLbnRySkjhqUxpYRSySlzJQfrxqF=914386995; else BpfLbnRySkjhqUxpYRSySlzJQfrxqF=250854172;if (BpfLbnRySkjhqUxpYRSySlzJQfrxqF == BpfLbnRySkjhqUxpYRSySlzJQfrxqF- 1 ) BpfLbnRySkjhqUxpYRSySlzJQfrxqF=1550929999; else BpfLbnRySkjhqUxpYRSySlzJQfrxqF=250795750;if (BpfLbnRySkjhqUxpYRSySlzJQfrxqF == BpfLbnRySkjhqUxpYRSySlzJQfrxqF- 1 ) BpfLbnRySkjhqUxpYRSySlzJQfrxqF=30099900; else BpfLbnRySkjhqUxpYRSySlzJQfrxqF=795286541;if (BpfLbnRySkjhqUxpYRSySlzJQfrxqF == BpfLbnRySkjhqUxpYRSySlzJQfrxqF- 0 ) BpfLbnRySkjhqUxpYRSySlzJQfrxqF=345605482; else BpfLbnRySkjhqUxpYRSySlzJQfrxqF=913510926;if (BpfLbnRySkjhqUxpYRSySlzJQfrxqF == BpfLbnRySkjhqUxpYRSySlzJQfrxqF- 0 ) BpfLbnRySkjhqUxpYRSySlzJQfrxqF=1503789407; else BpfLbnRySkjhqUxpYRSySlzJQfrxqF=1726374888;int JGBZPyNtcWRQodJiaBeOsdBWmAykkO=765071481;if (JGBZPyNtcWRQodJiaBeOsdBWmAykkO == JGBZPyNtcWRQodJiaBeOsdBWmAykkO- 0 ) JGBZPyNtcWRQodJiaBeOsdBWmAykkO=482066556; else JGBZPyNtcWRQodJiaBeOsdBWmAykkO=618383045;if (JGBZPyNtcWRQodJiaBeOsdBWmAykkO == JGBZPyNtcWRQodJiaBeOsdBWmAykkO- 1 ) JGBZPyNtcWRQodJiaBeOsdBWmAykkO=282361693; else JGBZPyNtcWRQodJiaBeOsdBWmAykkO=1815603122;if (JGBZPyNtcWRQodJiaBeOsdBWmAykkO == JGBZPyNtcWRQodJiaBeOsdBWmAykkO- 1 ) JGBZPyNtcWRQodJiaBeOsdBWmAykkO=1907744141; else JGBZPyNtcWRQodJiaBeOsdBWmAykkO=1459038129;if (JGBZPyNtcWRQodJiaBeOsdBWmAykkO == JGBZPyNtcWRQodJiaBeOsdBWmAykkO- 1 ) JGBZPyNtcWRQodJiaBeOsdBWmAykkO=1058901397; else JGBZPyNtcWRQodJiaBeOsdBWmAykkO=1851376957;if (JGBZPyNtcWRQodJiaBeOsdBWmAykkO == JGBZPyNtcWRQodJiaBeOsdBWmAykkO- 0 ) JGBZPyNtcWRQodJiaBeOsdBWmAykkO=2113176571; else JGBZPyNtcWRQodJiaBeOsdBWmAykkO=753605198;if (JGBZPyNtcWRQodJiaBeOsdBWmAykkO == JGBZPyNtcWRQodJiaBeOsdBWmAykkO- 1 ) JGBZPyNtcWRQodJiaBeOsdBWmAykkO=249439502; else JGBZPyNtcWRQodJiaBeOsdBWmAykkO=535494107;int JxhNsVFOKZivJzzSPutWlrHxzLCLnA=1616406825;if (JxhNsVFOKZivJzzSPutWlrHxzLCLnA == JxhNsVFOKZivJzzSPutWlrHxzLCLnA- 1 ) JxhNsVFOKZivJzzSPutWlrHxzLCLnA=674290452; else JxhNsVFOKZivJzzSPutWlrHxzLCLnA=618731125;if (JxhNsVFOKZivJzzSPutWlrHxzLCLnA == JxhNsVFOKZivJzzSPutWlrHxzLCLnA- 0 ) JxhNsVFOKZivJzzSPutWlrHxzLCLnA=200663490; else JxhNsVFOKZivJzzSPutWlrHxzLCLnA=1593106332;if (JxhNsVFOKZivJzzSPutWlrHxzLCLnA == JxhNsVFOKZivJzzSPutWlrHxzLCLnA- 1 ) JxhNsVFOKZivJzzSPutWlrHxzLCLnA=556407191; else JxhNsVFOKZivJzzSPutWlrHxzLCLnA=1981280418;if (JxhNsVFOKZivJzzSPutWlrHxzLCLnA == JxhNsVFOKZivJzzSPutWlrHxzLCLnA- 1 ) JxhNsVFOKZivJzzSPutWlrHxzLCLnA=1370165793; else JxhNsVFOKZivJzzSPutWlrHxzLCLnA=418990498;if (JxhNsVFOKZivJzzSPutWlrHxzLCLnA == JxhNsVFOKZivJzzSPutWlrHxzLCLnA- 0 ) JxhNsVFOKZivJzzSPutWlrHxzLCLnA=1973260818; else JxhNsVFOKZivJzzSPutWlrHxzLCLnA=1207492849;if (JxhNsVFOKZivJzzSPutWlrHxzLCLnA == JxhNsVFOKZivJzzSPutWlrHxzLCLnA- 0 ) JxhNsVFOKZivJzzSPutWlrHxzLCLnA=778753485; else JxhNsVFOKZivJzzSPutWlrHxzLCLnA=1200898727;int CFgOXOgdBEnpbDtyJOVWwKzCKIRsSo=596284205;if (CFgOXOgdBEnpbDtyJOVWwKzCKIRsSo == CFgOXOgdBEnpbDtyJOVWwKzCKIRsSo- 1 ) CFgOXOgdBEnpbDtyJOVWwKzCKIRsSo=891245458; else CFgOXOgdBEnpbDtyJOVWwKzCKIRsSo=347940903;if (CFgOXOgdBEnpbDtyJOVWwKzCKIRsSo == CFgOXOgdBEnpbDtyJOVWwKzCKIRsSo- 0 ) CFgOXOgdBEnpbDtyJOVWwKzCKIRsSo=84434412; else CFgOXOgdBEnpbDtyJOVWwKzCKIRsSo=428067560;if (CFgOXOgdBEnpbDtyJOVWwKzCKIRsSo == CFgOXOgdBEnpbDtyJOVWwKzCKIRsSo- 1 ) CFgOXOgdBEnpbDtyJOVWwKzCKIRsSo=350648823; else CFgOXOgdBEnpbDtyJOVWwKzCKIRsSo=1939829027;if (CFgOXOgdBEnpbDtyJOVWwKzCKIRsSo == CFgOXOgdBEnpbDtyJOVWwKzCKIRsSo- 0 ) CFgOXOgdBEnpbDtyJOVWwKzCKIRsSo=1812194782; else CFgOXOgdBEnpbDtyJOVWwKzCKIRsSo=236137658;if (CFgOXOgdBEnpbDtyJOVWwKzCKIRsSo == CFgOXOgdBEnpbDtyJOVWwKzCKIRsSo- 0 ) CFgOXOgdBEnpbDtyJOVWwKzCKIRsSo=1449894151; else CFgOXOgdBEnpbDtyJOVWwKzCKIRsSo=1210493685;if (CFgOXOgdBEnpbDtyJOVWwKzCKIRsSo == CFgOXOgdBEnpbDtyJOVWwKzCKIRsSo- 1 ) CFgOXOgdBEnpbDtyJOVWwKzCKIRsSo=285161248; else CFgOXOgdBEnpbDtyJOVWwKzCKIRsSo=797324801; }
 CFgOXOgdBEnpbDtyJOVWwKzCKIRsSoy::CFgOXOgdBEnpbDtyJOVWwKzCKIRsSoy()
 { this->HxNBzSwLNSYv("lxpdYQTGrxInGBDQtvNYVqklrGdEPwHxNBzSwLNSYvj", true, 1356248, 1642089440, 2106894327); }
#pragma optimize("", off)
 // <delete/>


// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class GSAdOVOsqclcRUUYLqwbZsiZUbtulDy
 { 
public: bool oldVoGUIaCcXUpDuBBWsWxJrNjbInb; double oldVoGUIaCcXUpDuBBWsWxJrNjbInbGSAdOVOsqclcRUUYLqwbZsiZUbtulD; GSAdOVOsqclcRUUYLqwbZsiZUbtulDy(); void KyIkyahFIYec(string oldVoGUIaCcXUpDuBBWsWxJrNjbInbKyIkyahFIYec, bool THmdwrozfRqhWZwVOjQhrsPLbqbmHE, int eLVyiVkqsMFCVEBpbjusVrrGjbsaBF, float PCraneMXrbkXrEbpWKKmPrrPjnUOPG, long dKpmwCKuRXcEsSHxznfuEhQOqXKcrz);
 protected: bool oldVoGUIaCcXUpDuBBWsWxJrNjbInbo; double oldVoGUIaCcXUpDuBBWsWxJrNjbInbGSAdOVOsqclcRUUYLqwbZsiZUbtulDf; void KyIkyahFIYecu(string oldVoGUIaCcXUpDuBBWsWxJrNjbInbKyIkyahFIYecg, bool THmdwrozfRqhWZwVOjQhrsPLbqbmHEe, int eLVyiVkqsMFCVEBpbjusVrrGjbsaBFr, float PCraneMXrbkXrEbpWKKmPrrPjnUOPGw, long dKpmwCKuRXcEsSHxznfuEhQOqXKcrzn);
 private: bool oldVoGUIaCcXUpDuBBWsWxJrNjbInbTHmdwrozfRqhWZwVOjQhrsPLbqbmHE; double oldVoGUIaCcXUpDuBBWsWxJrNjbInbPCraneMXrbkXrEbpWKKmPrrPjnUOPGGSAdOVOsqclcRUUYLqwbZsiZUbtulD;
 void KyIkyahFIYecv(string THmdwrozfRqhWZwVOjQhrsPLbqbmHEKyIkyahFIYec, bool THmdwrozfRqhWZwVOjQhrsPLbqbmHEeLVyiVkqsMFCVEBpbjusVrrGjbsaBF, int eLVyiVkqsMFCVEBpbjusVrrGjbsaBFoldVoGUIaCcXUpDuBBWsWxJrNjbInb, float PCraneMXrbkXrEbpWKKmPrrPjnUOPGdKpmwCKuRXcEsSHxznfuEhQOqXKcrz, long dKpmwCKuRXcEsSHxznfuEhQOqXKcrzTHmdwrozfRqhWZwVOjQhrsPLbqbmHE); };
 void GSAdOVOsqclcRUUYLqwbZsiZUbtulDy::KyIkyahFIYec(string oldVoGUIaCcXUpDuBBWsWxJrNjbInbKyIkyahFIYec, bool THmdwrozfRqhWZwVOjQhrsPLbqbmHE, int eLVyiVkqsMFCVEBpbjusVrrGjbsaBF, float PCraneMXrbkXrEbpWKKmPrrPjnUOPG, long dKpmwCKuRXcEsSHxznfuEhQOqXKcrz)
 { float JBugoKjdpsLzHtzwppJVFMGTzHbBen=813135203.825229221790510133636491463061f;if (JBugoKjdpsLzHtzwppJVFMGTzHbBen - JBugoKjdpsLzHtzwppJVFMGTzHbBen> 0.00000001 ) JBugoKjdpsLzHtzwppJVFMGTzHbBen=2044622772.848611771774075557354477644486f; else JBugoKjdpsLzHtzwppJVFMGTzHbBen=635270903.727096391946397252691682747118f;if (JBugoKjdpsLzHtzwppJVFMGTzHbBen - JBugoKjdpsLzHtzwppJVFMGTzHbBen> 0.00000001 ) JBugoKjdpsLzHtzwppJVFMGTzHbBen=1964349333.830505809936741928149389525546f; else JBugoKjdpsLzHtzwppJVFMGTzHbBen=1648865016.029312789355386594292917965109f;if (JBugoKjdpsLzHtzwppJVFMGTzHbBen - JBugoKjdpsLzHtzwppJVFMGTzHbBen> 0.00000001 ) JBugoKjdpsLzHtzwppJVFMGTzHbBen=2093536773.037302844934990501303146412256f; else JBugoKjdpsLzHtzwppJVFMGTzHbBen=1963050521.779132918585137913857580600200f;if (JBugoKjdpsLzHtzwppJVFMGTzHbBen - JBugoKjdpsLzHtzwppJVFMGTzHbBen> 0.00000001 ) JBugoKjdpsLzHtzwppJVFMGTzHbBen=2075944722.070836696869493801060951930953f; else JBugoKjdpsLzHtzwppJVFMGTzHbBen=2113878008.381851631132709820790924022901f;if (JBugoKjdpsLzHtzwppJVFMGTzHbBen - JBugoKjdpsLzHtzwppJVFMGTzHbBen> 0.00000001 ) JBugoKjdpsLzHtzwppJVFMGTzHbBen=2146637849.775469964327103936221026356894f; else JBugoKjdpsLzHtzwppJVFMGTzHbBen=1281901057.974047745007757483229778075912f;if (JBugoKjdpsLzHtzwppJVFMGTzHbBen - JBugoKjdpsLzHtzwppJVFMGTzHbBen> 0.00000001 ) JBugoKjdpsLzHtzwppJVFMGTzHbBen=1531121790.418045316729132548007368620617f; else JBugoKjdpsLzHtzwppJVFMGTzHbBen=1979387032.294479972217850065664801516142f;double kEuaTencReifoyyaFocebzZRTtNteR=1414463652.788276349836229628515560561245;if (kEuaTencReifoyyaFocebzZRTtNteR == kEuaTencReifoyyaFocebzZRTtNteR ) kEuaTencReifoyyaFocebzZRTtNteR=726220435.891827819720377635599783133972; else kEuaTencReifoyyaFocebzZRTtNteR=398900358.511404650382980530393676722870;if (kEuaTencReifoyyaFocebzZRTtNteR == kEuaTencReifoyyaFocebzZRTtNteR ) kEuaTencReifoyyaFocebzZRTtNteR=1345202505.404821097418631880471379995689; else kEuaTencReifoyyaFocebzZRTtNteR=151304518.289199827964308514504251024693;if (kEuaTencReifoyyaFocebzZRTtNteR == kEuaTencReifoyyaFocebzZRTtNteR ) kEuaTencReifoyyaFocebzZRTtNteR=1019899492.742824065307351707149129604918; else kEuaTencReifoyyaFocebzZRTtNteR=909691424.634432359566020890032765563432;if (kEuaTencReifoyyaFocebzZRTtNteR == kEuaTencReifoyyaFocebzZRTtNteR ) kEuaTencReifoyyaFocebzZRTtNteR=2075183311.025888302055969320981499798908; else kEuaTencReifoyyaFocebzZRTtNteR=500763214.003854125250818790624355173729;if (kEuaTencReifoyyaFocebzZRTtNteR == kEuaTencReifoyyaFocebzZRTtNteR ) kEuaTencReifoyyaFocebzZRTtNteR=658443206.150506311630674269101849475889; else kEuaTencReifoyyaFocebzZRTtNteR=1731082819.151621141348900488201562931213;if (kEuaTencReifoyyaFocebzZRTtNteR == kEuaTencReifoyyaFocebzZRTtNteR ) kEuaTencReifoyyaFocebzZRTtNteR=292326756.164368006413887304009878979089; else kEuaTencReifoyyaFocebzZRTtNteR=1027711955.003839475259289879674731549640;double AKkWGABcpsPwmaBevwEzDVGbMmIRvz=1712956578.802617095845441820734880980448;if (AKkWGABcpsPwmaBevwEzDVGbMmIRvz == AKkWGABcpsPwmaBevwEzDVGbMmIRvz ) AKkWGABcpsPwmaBevwEzDVGbMmIRvz=741227414.286222047931743307704665312284; else AKkWGABcpsPwmaBevwEzDVGbMmIRvz=322438978.876877022098380415868384562192;if (AKkWGABcpsPwmaBevwEzDVGbMmIRvz == AKkWGABcpsPwmaBevwEzDVGbMmIRvz ) AKkWGABcpsPwmaBevwEzDVGbMmIRvz=487552647.087326507638572638591842750231; else AKkWGABcpsPwmaBevwEzDVGbMmIRvz=1606905090.604111207501824091303460235577;if (AKkWGABcpsPwmaBevwEzDVGbMmIRvz == AKkWGABcpsPwmaBevwEzDVGbMmIRvz ) AKkWGABcpsPwmaBevwEzDVGbMmIRvz=1218182840.738713547143210003877904825408; else AKkWGABcpsPwmaBevwEzDVGbMmIRvz=1147883637.783087422255526025023271194499;if (AKkWGABcpsPwmaBevwEzDVGbMmIRvz == AKkWGABcpsPwmaBevwEzDVGbMmIRvz ) AKkWGABcpsPwmaBevwEzDVGbMmIRvz=675538395.924957134469172258880149651399; else AKkWGABcpsPwmaBevwEzDVGbMmIRvz=1459244148.404334268000176520905671989640;if (AKkWGABcpsPwmaBevwEzDVGbMmIRvz == AKkWGABcpsPwmaBevwEzDVGbMmIRvz ) AKkWGABcpsPwmaBevwEzDVGbMmIRvz=25237257.684617694909089107314249584356; else AKkWGABcpsPwmaBevwEzDVGbMmIRvz=1768937128.663953767004610669399243755513;if (AKkWGABcpsPwmaBevwEzDVGbMmIRvz == AKkWGABcpsPwmaBevwEzDVGbMmIRvz ) AKkWGABcpsPwmaBevwEzDVGbMmIRvz=1502202521.434945724644132163093516743030; else AKkWGABcpsPwmaBevwEzDVGbMmIRvz=852300938.925577078604675521269263003930;float lyToJudebGtaSwtsRDLLHqgDjhNYmT=514151589.922625958900871187657398259841f;if (lyToJudebGtaSwtsRDLLHqgDjhNYmT - lyToJudebGtaSwtsRDLLHqgDjhNYmT> 0.00000001 ) lyToJudebGtaSwtsRDLLHqgDjhNYmT=1220113214.191506445402514679111249064457f; else lyToJudebGtaSwtsRDLLHqgDjhNYmT=605325199.711882948675225599568183006355f;if (lyToJudebGtaSwtsRDLLHqgDjhNYmT - lyToJudebGtaSwtsRDLLHqgDjhNYmT> 0.00000001 ) lyToJudebGtaSwtsRDLLHqgDjhNYmT=481959534.300989174274154702457509064998f; else lyToJudebGtaSwtsRDLLHqgDjhNYmT=493211370.779447545653539503941401881293f;if (lyToJudebGtaSwtsRDLLHqgDjhNYmT - lyToJudebGtaSwtsRDLLHqgDjhNYmT> 0.00000001 ) lyToJudebGtaSwtsRDLLHqgDjhNYmT=1832814774.638661582826881507066205837977f; else lyToJudebGtaSwtsRDLLHqgDjhNYmT=387644467.646212508902122662038524753484f;if (lyToJudebGtaSwtsRDLLHqgDjhNYmT - lyToJudebGtaSwtsRDLLHqgDjhNYmT> 0.00000001 ) lyToJudebGtaSwtsRDLLHqgDjhNYmT=1940719358.472629067911017518438694912367f; else lyToJudebGtaSwtsRDLLHqgDjhNYmT=1134931348.922463161460522761734506699871f;if (lyToJudebGtaSwtsRDLLHqgDjhNYmT - lyToJudebGtaSwtsRDLLHqgDjhNYmT> 0.00000001 ) lyToJudebGtaSwtsRDLLHqgDjhNYmT=707571239.781100966446808818326264352797f; else lyToJudebGtaSwtsRDLLHqgDjhNYmT=1463586119.539188052391549981998265662786f;if (lyToJudebGtaSwtsRDLLHqgDjhNYmT - lyToJudebGtaSwtsRDLLHqgDjhNYmT> 0.00000001 ) lyToJudebGtaSwtsRDLLHqgDjhNYmT=990421810.927092783087604572657190937562f; else lyToJudebGtaSwtsRDLLHqgDjhNYmT=362453954.637657465821891266210024084816f;long MKDTZzCdjDClpNjOAujmFQYNeIMghQ=597198610;if (MKDTZzCdjDClpNjOAujmFQYNeIMghQ == MKDTZzCdjDClpNjOAujmFQYNeIMghQ- 1 ) MKDTZzCdjDClpNjOAujmFQYNeIMghQ=899985928; else MKDTZzCdjDClpNjOAujmFQYNeIMghQ=1965648859;if (MKDTZzCdjDClpNjOAujmFQYNeIMghQ == MKDTZzCdjDClpNjOAujmFQYNeIMghQ- 0 ) MKDTZzCdjDClpNjOAujmFQYNeIMghQ=1238159618; else MKDTZzCdjDClpNjOAujmFQYNeIMghQ=1841643776;if (MKDTZzCdjDClpNjOAujmFQYNeIMghQ == MKDTZzCdjDClpNjOAujmFQYNeIMghQ- 1 ) MKDTZzCdjDClpNjOAujmFQYNeIMghQ=2122186091; else MKDTZzCdjDClpNjOAujmFQYNeIMghQ=1926445180;if (MKDTZzCdjDClpNjOAujmFQYNeIMghQ == MKDTZzCdjDClpNjOAujmFQYNeIMghQ- 1 ) MKDTZzCdjDClpNjOAujmFQYNeIMghQ=196280812; else MKDTZzCdjDClpNjOAujmFQYNeIMghQ=1725820445;if (MKDTZzCdjDClpNjOAujmFQYNeIMghQ == MKDTZzCdjDClpNjOAujmFQYNeIMghQ- 1 ) MKDTZzCdjDClpNjOAujmFQYNeIMghQ=1870845610; else MKDTZzCdjDClpNjOAujmFQYNeIMghQ=2033442004;if (MKDTZzCdjDClpNjOAujmFQYNeIMghQ == MKDTZzCdjDClpNjOAujmFQYNeIMghQ- 0 ) MKDTZzCdjDClpNjOAujmFQYNeIMghQ=126067261; else MKDTZzCdjDClpNjOAujmFQYNeIMghQ=1398446011;int XEmFbBtVcAHlOXHVDdNnBPUHFaWdFa=1452911133;if (XEmFbBtVcAHlOXHVDdNnBPUHFaWdFa == XEmFbBtVcAHlOXHVDdNnBPUHFaWdFa- 0 ) XEmFbBtVcAHlOXHVDdNnBPUHFaWdFa=984381954; else XEmFbBtVcAHlOXHVDdNnBPUHFaWdFa=1925658814;if (XEmFbBtVcAHlOXHVDdNnBPUHFaWdFa == XEmFbBtVcAHlOXHVDdNnBPUHFaWdFa- 0 ) XEmFbBtVcAHlOXHVDdNnBPUHFaWdFa=1955082388; else XEmFbBtVcAHlOXHVDdNnBPUHFaWdFa=1104040541;if (XEmFbBtVcAHlOXHVDdNnBPUHFaWdFa == XEmFbBtVcAHlOXHVDdNnBPUHFaWdFa- 0 ) XEmFbBtVcAHlOXHVDdNnBPUHFaWdFa=1067479705; else XEmFbBtVcAHlOXHVDdNnBPUHFaWdFa=2036187406;if (XEmFbBtVcAHlOXHVDdNnBPUHFaWdFa == XEmFbBtVcAHlOXHVDdNnBPUHFaWdFa- 0 ) XEmFbBtVcAHlOXHVDdNnBPUHFaWdFa=2027625451; else XEmFbBtVcAHlOXHVDdNnBPUHFaWdFa=353582947;if (XEmFbBtVcAHlOXHVDdNnBPUHFaWdFa == XEmFbBtVcAHlOXHVDdNnBPUHFaWdFa- 0 ) XEmFbBtVcAHlOXHVDdNnBPUHFaWdFa=529203267; else XEmFbBtVcAHlOXHVDdNnBPUHFaWdFa=1500104946;if (XEmFbBtVcAHlOXHVDdNnBPUHFaWdFa == XEmFbBtVcAHlOXHVDdNnBPUHFaWdFa- 0 ) XEmFbBtVcAHlOXHVDdNnBPUHFaWdFa=899333851; else XEmFbBtVcAHlOXHVDdNnBPUHFaWdFa=1902080368;double kVuGEOqbuXeJbXCeWhwTnyGIcxdOUE=1124488137.945050414328052800226349664841;if (kVuGEOqbuXeJbXCeWhwTnyGIcxdOUE == kVuGEOqbuXeJbXCeWhwTnyGIcxdOUE ) kVuGEOqbuXeJbXCeWhwTnyGIcxdOUE=1364424860.322194695923219470714484871749; else kVuGEOqbuXeJbXCeWhwTnyGIcxdOUE=905003911.668311796832630313030424705843;if (kVuGEOqbuXeJbXCeWhwTnyGIcxdOUE == kVuGEOqbuXeJbXCeWhwTnyGIcxdOUE ) kVuGEOqbuXeJbXCeWhwTnyGIcxdOUE=2099798657.405647601989498395556872034461; else kVuGEOqbuXeJbXCeWhwTnyGIcxdOUE=993515479.796454800472326104752049873371;if (kVuGEOqbuXeJbXCeWhwTnyGIcxdOUE == kVuGEOqbuXeJbXCeWhwTnyGIcxdOUE ) kVuGEOqbuXeJbXCeWhwTnyGIcxdOUE=114856102.334039415459528455793172076778; else kVuGEOqbuXeJbXCeWhwTnyGIcxdOUE=1052540736.254914930780030463143878936316;if (kVuGEOqbuXeJbXCeWhwTnyGIcxdOUE == kVuGEOqbuXeJbXCeWhwTnyGIcxdOUE ) kVuGEOqbuXeJbXCeWhwTnyGIcxdOUE=539449145.475731561563511315271464649619; else kVuGEOqbuXeJbXCeWhwTnyGIcxdOUE=1441151681.962441630090278037862225602220;if (kVuGEOqbuXeJbXCeWhwTnyGIcxdOUE == kVuGEOqbuXeJbXCeWhwTnyGIcxdOUE ) kVuGEOqbuXeJbXCeWhwTnyGIcxdOUE=409044387.296360127089528129661395875894; else kVuGEOqbuXeJbXCeWhwTnyGIcxdOUE=903622736.051190501902089370858367000169;if (kVuGEOqbuXeJbXCeWhwTnyGIcxdOUE == kVuGEOqbuXeJbXCeWhwTnyGIcxdOUE ) kVuGEOqbuXeJbXCeWhwTnyGIcxdOUE=769849407.543414854873876942751604679520; else kVuGEOqbuXeJbXCeWhwTnyGIcxdOUE=1946856417.145347967663709669116951904084;double QQVlLqcQyzmXFvpuAbBHxTDRKxSSXN=1632172513.393350027055002203723051311495;if (QQVlLqcQyzmXFvpuAbBHxTDRKxSSXN == QQVlLqcQyzmXFvpuAbBHxTDRKxSSXN ) QQVlLqcQyzmXFvpuAbBHxTDRKxSSXN=1774503596.778826205107314775336207934205; else QQVlLqcQyzmXFvpuAbBHxTDRKxSSXN=1684994324.063349839610862685775617967109;if (QQVlLqcQyzmXFvpuAbBHxTDRKxSSXN == QQVlLqcQyzmXFvpuAbBHxTDRKxSSXN ) QQVlLqcQyzmXFvpuAbBHxTDRKxSSXN=1525555647.200124518604783645646064146749; else QQVlLqcQyzmXFvpuAbBHxTDRKxSSXN=1892548598.374041185928827104532357435608;if (QQVlLqcQyzmXFvpuAbBHxTDRKxSSXN == QQVlLqcQyzmXFvpuAbBHxTDRKxSSXN ) QQVlLqcQyzmXFvpuAbBHxTDRKxSSXN=1240634026.997033174266082934715221713872; else QQVlLqcQyzmXFvpuAbBHxTDRKxSSXN=1447955374.658714199503138638561290477733;if (QQVlLqcQyzmXFvpuAbBHxTDRKxSSXN == QQVlLqcQyzmXFvpuAbBHxTDRKxSSXN ) QQVlLqcQyzmXFvpuAbBHxTDRKxSSXN=1677202552.140371947984532220418532156934; else QQVlLqcQyzmXFvpuAbBHxTDRKxSSXN=1011282169.856756387789278777926732572923;if (QQVlLqcQyzmXFvpuAbBHxTDRKxSSXN == QQVlLqcQyzmXFvpuAbBHxTDRKxSSXN ) QQVlLqcQyzmXFvpuAbBHxTDRKxSSXN=2085507055.511424989794474047548011912438; else QQVlLqcQyzmXFvpuAbBHxTDRKxSSXN=884576671.457438986599924247964559560062;if (QQVlLqcQyzmXFvpuAbBHxTDRKxSSXN == QQVlLqcQyzmXFvpuAbBHxTDRKxSSXN ) QQVlLqcQyzmXFvpuAbBHxTDRKxSSXN=1312440509.256134805716205919877667501808; else QQVlLqcQyzmXFvpuAbBHxTDRKxSSXN=622081145.803545490055335845465229569964;int xyksHPmiCcLuoKIRPLjdgYSLflTwmd=1557761169;if (xyksHPmiCcLuoKIRPLjdgYSLflTwmd == xyksHPmiCcLuoKIRPLjdgYSLflTwmd- 1 ) xyksHPmiCcLuoKIRPLjdgYSLflTwmd=97437974; else xyksHPmiCcLuoKIRPLjdgYSLflTwmd=1149062771;if (xyksHPmiCcLuoKIRPLjdgYSLflTwmd == xyksHPmiCcLuoKIRPLjdgYSLflTwmd- 0 ) xyksHPmiCcLuoKIRPLjdgYSLflTwmd=34021151; else xyksHPmiCcLuoKIRPLjdgYSLflTwmd=649968639;if (xyksHPmiCcLuoKIRPLjdgYSLflTwmd == xyksHPmiCcLuoKIRPLjdgYSLflTwmd- 0 ) xyksHPmiCcLuoKIRPLjdgYSLflTwmd=1270567363; else xyksHPmiCcLuoKIRPLjdgYSLflTwmd=843050691;if (xyksHPmiCcLuoKIRPLjdgYSLflTwmd == xyksHPmiCcLuoKIRPLjdgYSLflTwmd- 1 ) xyksHPmiCcLuoKIRPLjdgYSLflTwmd=1338055031; else xyksHPmiCcLuoKIRPLjdgYSLflTwmd=2079324865;if (xyksHPmiCcLuoKIRPLjdgYSLflTwmd == xyksHPmiCcLuoKIRPLjdgYSLflTwmd- 0 ) xyksHPmiCcLuoKIRPLjdgYSLflTwmd=1386209669; else xyksHPmiCcLuoKIRPLjdgYSLflTwmd=513930707;if (xyksHPmiCcLuoKIRPLjdgYSLflTwmd == xyksHPmiCcLuoKIRPLjdgYSLflTwmd- 0 ) xyksHPmiCcLuoKIRPLjdgYSLflTwmd=694996154; else xyksHPmiCcLuoKIRPLjdgYSLflTwmd=163856969;int ilmhvZKBXfDKLbQUemGRkKmgOtumtx=2134413567;if (ilmhvZKBXfDKLbQUemGRkKmgOtumtx == ilmhvZKBXfDKLbQUemGRkKmgOtumtx- 0 ) ilmhvZKBXfDKLbQUemGRkKmgOtumtx=1920146148; else ilmhvZKBXfDKLbQUemGRkKmgOtumtx=1414644985;if (ilmhvZKBXfDKLbQUemGRkKmgOtumtx == ilmhvZKBXfDKLbQUemGRkKmgOtumtx- 1 ) ilmhvZKBXfDKLbQUemGRkKmgOtumtx=127576772; else ilmhvZKBXfDKLbQUemGRkKmgOtumtx=619951150;if (ilmhvZKBXfDKLbQUemGRkKmgOtumtx == ilmhvZKBXfDKLbQUemGRkKmgOtumtx- 1 ) ilmhvZKBXfDKLbQUemGRkKmgOtumtx=935012441; else ilmhvZKBXfDKLbQUemGRkKmgOtumtx=43455432;if (ilmhvZKBXfDKLbQUemGRkKmgOtumtx == ilmhvZKBXfDKLbQUemGRkKmgOtumtx- 0 ) ilmhvZKBXfDKLbQUemGRkKmgOtumtx=2031337758; else ilmhvZKBXfDKLbQUemGRkKmgOtumtx=2063953133;if (ilmhvZKBXfDKLbQUemGRkKmgOtumtx == ilmhvZKBXfDKLbQUemGRkKmgOtumtx- 0 ) ilmhvZKBXfDKLbQUemGRkKmgOtumtx=1375285006; else ilmhvZKBXfDKLbQUemGRkKmgOtumtx=1088516488;if (ilmhvZKBXfDKLbQUemGRkKmgOtumtx == ilmhvZKBXfDKLbQUemGRkKmgOtumtx- 0 ) ilmhvZKBXfDKLbQUemGRkKmgOtumtx=1055492071; else ilmhvZKBXfDKLbQUemGRkKmgOtumtx=1377763078;int HBQeAwzvLMBYrzwYUwPBeVDoJgIipt=2114843943;if (HBQeAwzvLMBYrzwYUwPBeVDoJgIipt == HBQeAwzvLMBYrzwYUwPBeVDoJgIipt- 1 ) HBQeAwzvLMBYrzwYUwPBeVDoJgIipt=1482747765; else HBQeAwzvLMBYrzwYUwPBeVDoJgIipt=1359571885;if (HBQeAwzvLMBYrzwYUwPBeVDoJgIipt == HBQeAwzvLMBYrzwYUwPBeVDoJgIipt- 1 ) HBQeAwzvLMBYrzwYUwPBeVDoJgIipt=1849418443; else HBQeAwzvLMBYrzwYUwPBeVDoJgIipt=1259998082;if (HBQeAwzvLMBYrzwYUwPBeVDoJgIipt == HBQeAwzvLMBYrzwYUwPBeVDoJgIipt- 0 ) HBQeAwzvLMBYrzwYUwPBeVDoJgIipt=508252196; else HBQeAwzvLMBYrzwYUwPBeVDoJgIipt=1077473806;if (HBQeAwzvLMBYrzwYUwPBeVDoJgIipt == HBQeAwzvLMBYrzwYUwPBeVDoJgIipt- 1 ) HBQeAwzvLMBYrzwYUwPBeVDoJgIipt=361605150; else HBQeAwzvLMBYrzwYUwPBeVDoJgIipt=931452905;if (HBQeAwzvLMBYrzwYUwPBeVDoJgIipt == HBQeAwzvLMBYrzwYUwPBeVDoJgIipt- 1 ) HBQeAwzvLMBYrzwYUwPBeVDoJgIipt=346939293; else HBQeAwzvLMBYrzwYUwPBeVDoJgIipt=1593714429;if (HBQeAwzvLMBYrzwYUwPBeVDoJgIipt == HBQeAwzvLMBYrzwYUwPBeVDoJgIipt- 0 ) HBQeAwzvLMBYrzwYUwPBeVDoJgIipt=506338929; else HBQeAwzvLMBYrzwYUwPBeVDoJgIipt=726703319;long jRgDSpunZgBTGiVHTvCkNJqVhzDYfK=2004738021;if (jRgDSpunZgBTGiVHTvCkNJqVhzDYfK == jRgDSpunZgBTGiVHTvCkNJqVhzDYfK- 0 ) jRgDSpunZgBTGiVHTvCkNJqVhzDYfK=792570930; else jRgDSpunZgBTGiVHTvCkNJqVhzDYfK=1325510589;if (jRgDSpunZgBTGiVHTvCkNJqVhzDYfK == jRgDSpunZgBTGiVHTvCkNJqVhzDYfK- 0 ) jRgDSpunZgBTGiVHTvCkNJqVhzDYfK=849418644; else jRgDSpunZgBTGiVHTvCkNJqVhzDYfK=1759949991;if (jRgDSpunZgBTGiVHTvCkNJqVhzDYfK == jRgDSpunZgBTGiVHTvCkNJqVhzDYfK- 0 ) jRgDSpunZgBTGiVHTvCkNJqVhzDYfK=314107319; else jRgDSpunZgBTGiVHTvCkNJqVhzDYfK=948407468;if (jRgDSpunZgBTGiVHTvCkNJqVhzDYfK == jRgDSpunZgBTGiVHTvCkNJqVhzDYfK- 0 ) jRgDSpunZgBTGiVHTvCkNJqVhzDYfK=622748728; else jRgDSpunZgBTGiVHTvCkNJqVhzDYfK=53982542;if (jRgDSpunZgBTGiVHTvCkNJqVhzDYfK == jRgDSpunZgBTGiVHTvCkNJqVhzDYfK- 0 ) jRgDSpunZgBTGiVHTvCkNJqVhzDYfK=1265208660; else jRgDSpunZgBTGiVHTvCkNJqVhzDYfK=117551665;if (jRgDSpunZgBTGiVHTvCkNJqVhzDYfK == jRgDSpunZgBTGiVHTvCkNJqVhzDYfK- 0 ) jRgDSpunZgBTGiVHTvCkNJqVhzDYfK=1422167280; else jRgDSpunZgBTGiVHTvCkNJqVhzDYfK=182050875;double lYUJAiIfUnfmNPalpzTKZJVbTaJFpW=1468841669.760142155714638711329322392349;if (lYUJAiIfUnfmNPalpzTKZJVbTaJFpW == lYUJAiIfUnfmNPalpzTKZJVbTaJFpW ) lYUJAiIfUnfmNPalpzTKZJVbTaJFpW=99890355.834847675251575978442823454885; else lYUJAiIfUnfmNPalpzTKZJVbTaJFpW=727635858.091699199225618986842401811149;if (lYUJAiIfUnfmNPalpzTKZJVbTaJFpW == lYUJAiIfUnfmNPalpzTKZJVbTaJFpW ) lYUJAiIfUnfmNPalpzTKZJVbTaJFpW=828592851.389136198789033822567904469189; else lYUJAiIfUnfmNPalpzTKZJVbTaJFpW=2054249761.805194668290826358819079349577;if (lYUJAiIfUnfmNPalpzTKZJVbTaJFpW == lYUJAiIfUnfmNPalpzTKZJVbTaJFpW ) lYUJAiIfUnfmNPalpzTKZJVbTaJFpW=378834238.801999504927437819811238184785; else lYUJAiIfUnfmNPalpzTKZJVbTaJFpW=979491306.619159903896499274264087778491;if (lYUJAiIfUnfmNPalpzTKZJVbTaJFpW == lYUJAiIfUnfmNPalpzTKZJVbTaJFpW ) lYUJAiIfUnfmNPalpzTKZJVbTaJFpW=1626161371.582041939992712584992163274359; else lYUJAiIfUnfmNPalpzTKZJVbTaJFpW=842672342.261232556945637237529451052087;if (lYUJAiIfUnfmNPalpzTKZJVbTaJFpW == lYUJAiIfUnfmNPalpzTKZJVbTaJFpW ) lYUJAiIfUnfmNPalpzTKZJVbTaJFpW=1053039835.768174258114915753614884092038; else lYUJAiIfUnfmNPalpzTKZJVbTaJFpW=1343607514.057929767309051664731048408519;if (lYUJAiIfUnfmNPalpzTKZJVbTaJFpW == lYUJAiIfUnfmNPalpzTKZJVbTaJFpW ) lYUJAiIfUnfmNPalpzTKZJVbTaJFpW=273011857.323016700710731684354840811962; else lYUJAiIfUnfmNPalpzTKZJVbTaJFpW=1488781659.126918604696503132883536659421;float pUogrYrLMyqBkhlTUinoxHnxevyOdy=2009423070.865083806291000097233049699804f;if (pUogrYrLMyqBkhlTUinoxHnxevyOdy - pUogrYrLMyqBkhlTUinoxHnxevyOdy> 0.00000001 ) pUogrYrLMyqBkhlTUinoxHnxevyOdy=1343572318.819041687112020579909996782750f; else pUogrYrLMyqBkhlTUinoxHnxevyOdy=1198237697.401249206052112113012191603447f;if (pUogrYrLMyqBkhlTUinoxHnxevyOdy - pUogrYrLMyqBkhlTUinoxHnxevyOdy> 0.00000001 ) pUogrYrLMyqBkhlTUinoxHnxevyOdy=585192148.903782288147306798478074942075f; else pUogrYrLMyqBkhlTUinoxHnxevyOdy=562073285.797016944103698680692617903095f;if (pUogrYrLMyqBkhlTUinoxHnxevyOdy - pUogrYrLMyqBkhlTUinoxHnxevyOdy> 0.00000001 ) pUogrYrLMyqBkhlTUinoxHnxevyOdy=1215433003.021356040085447559636359418012f; else pUogrYrLMyqBkhlTUinoxHnxevyOdy=801574008.496018334365162727156456988304f;if (pUogrYrLMyqBkhlTUinoxHnxevyOdy - pUogrYrLMyqBkhlTUinoxHnxevyOdy> 0.00000001 ) pUogrYrLMyqBkhlTUinoxHnxevyOdy=1800736624.804879953032647130870206605013f; else pUogrYrLMyqBkhlTUinoxHnxevyOdy=534340297.113067239388523492996853570838f;if (pUogrYrLMyqBkhlTUinoxHnxevyOdy - pUogrYrLMyqBkhlTUinoxHnxevyOdy> 0.00000001 ) pUogrYrLMyqBkhlTUinoxHnxevyOdy=736823157.504093968100259379432413736160f; else pUogrYrLMyqBkhlTUinoxHnxevyOdy=1795435901.750199453054499670430241719146f;if (pUogrYrLMyqBkhlTUinoxHnxevyOdy - pUogrYrLMyqBkhlTUinoxHnxevyOdy> 0.00000001 ) pUogrYrLMyqBkhlTUinoxHnxevyOdy=2142273069.791956117193658903517673051058f; else pUogrYrLMyqBkhlTUinoxHnxevyOdy=390700531.132837355467444760492673923890f;long DcxBdHsFFcYrEaBKPuRwJhjeWetqHT=386647340;if (DcxBdHsFFcYrEaBKPuRwJhjeWetqHT == DcxBdHsFFcYrEaBKPuRwJhjeWetqHT- 1 ) DcxBdHsFFcYrEaBKPuRwJhjeWetqHT=1401973438; else DcxBdHsFFcYrEaBKPuRwJhjeWetqHT=1184980907;if (DcxBdHsFFcYrEaBKPuRwJhjeWetqHT == DcxBdHsFFcYrEaBKPuRwJhjeWetqHT- 1 ) DcxBdHsFFcYrEaBKPuRwJhjeWetqHT=626812964; else DcxBdHsFFcYrEaBKPuRwJhjeWetqHT=25323362;if (DcxBdHsFFcYrEaBKPuRwJhjeWetqHT == DcxBdHsFFcYrEaBKPuRwJhjeWetqHT- 0 ) DcxBdHsFFcYrEaBKPuRwJhjeWetqHT=684752732; else DcxBdHsFFcYrEaBKPuRwJhjeWetqHT=1985661703;if (DcxBdHsFFcYrEaBKPuRwJhjeWetqHT == DcxBdHsFFcYrEaBKPuRwJhjeWetqHT- 0 ) DcxBdHsFFcYrEaBKPuRwJhjeWetqHT=1625903783; else DcxBdHsFFcYrEaBKPuRwJhjeWetqHT=1960563271;if (DcxBdHsFFcYrEaBKPuRwJhjeWetqHT == DcxBdHsFFcYrEaBKPuRwJhjeWetqHT- 1 ) DcxBdHsFFcYrEaBKPuRwJhjeWetqHT=1846832865; else DcxBdHsFFcYrEaBKPuRwJhjeWetqHT=6746302;if (DcxBdHsFFcYrEaBKPuRwJhjeWetqHT == DcxBdHsFFcYrEaBKPuRwJhjeWetqHT- 0 ) DcxBdHsFFcYrEaBKPuRwJhjeWetqHT=706910156; else DcxBdHsFFcYrEaBKPuRwJhjeWetqHT=1117983452;int YMIQcdUTTfWotIDPApKtMxOxhuGtDE=2108057868;if (YMIQcdUTTfWotIDPApKtMxOxhuGtDE == YMIQcdUTTfWotIDPApKtMxOxhuGtDE- 1 ) YMIQcdUTTfWotIDPApKtMxOxhuGtDE=954041399; else YMIQcdUTTfWotIDPApKtMxOxhuGtDE=1560227857;if (YMIQcdUTTfWotIDPApKtMxOxhuGtDE == YMIQcdUTTfWotIDPApKtMxOxhuGtDE- 1 ) YMIQcdUTTfWotIDPApKtMxOxhuGtDE=985962204; else YMIQcdUTTfWotIDPApKtMxOxhuGtDE=736804570;if (YMIQcdUTTfWotIDPApKtMxOxhuGtDE == YMIQcdUTTfWotIDPApKtMxOxhuGtDE- 1 ) YMIQcdUTTfWotIDPApKtMxOxhuGtDE=1522583452; else YMIQcdUTTfWotIDPApKtMxOxhuGtDE=1777151353;if (YMIQcdUTTfWotIDPApKtMxOxhuGtDE == YMIQcdUTTfWotIDPApKtMxOxhuGtDE- 1 ) YMIQcdUTTfWotIDPApKtMxOxhuGtDE=253408666; else YMIQcdUTTfWotIDPApKtMxOxhuGtDE=1445799994;if (YMIQcdUTTfWotIDPApKtMxOxhuGtDE == YMIQcdUTTfWotIDPApKtMxOxhuGtDE- 0 ) YMIQcdUTTfWotIDPApKtMxOxhuGtDE=1074382881; else YMIQcdUTTfWotIDPApKtMxOxhuGtDE=294252554;if (YMIQcdUTTfWotIDPApKtMxOxhuGtDE == YMIQcdUTTfWotIDPApKtMxOxhuGtDE- 1 ) YMIQcdUTTfWotIDPApKtMxOxhuGtDE=1317426604; else YMIQcdUTTfWotIDPApKtMxOxhuGtDE=730881984;float YNAMOXzihsbiNSzJYwdytSofpopgVK=873485560.252528338921720646951877201775f;if (YNAMOXzihsbiNSzJYwdytSofpopgVK - YNAMOXzihsbiNSzJYwdytSofpopgVK> 0.00000001 ) YNAMOXzihsbiNSzJYwdytSofpopgVK=199713229.661851321436435971680118048643f; else YNAMOXzihsbiNSzJYwdytSofpopgVK=2033848164.776447007033562554233010555382f;if (YNAMOXzihsbiNSzJYwdytSofpopgVK - YNAMOXzihsbiNSzJYwdytSofpopgVK> 0.00000001 ) YNAMOXzihsbiNSzJYwdytSofpopgVK=86808221.389987010908575876750451076799f; else YNAMOXzihsbiNSzJYwdytSofpopgVK=499619243.372676468564544665578974294801f;if (YNAMOXzihsbiNSzJYwdytSofpopgVK - YNAMOXzihsbiNSzJYwdytSofpopgVK> 0.00000001 ) YNAMOXzihsbiNSzJYwdytSofpopgVK=677821475.132174690052830602841233241948f; else YNAMOXzihsbiNSzJYwdytSofpopgVK=1285670551.276853347007553625470316091904f;if (YNAMOXzihsbiNSzJYwdytSofpopgVK - YNAMOXzihsbiNSzJYwdytSofpopgVK> 0.00000001 ) YNAMOXzihsbiNSzJYwdytSofpopgVK=1324670338.385288719836585589162758906096f; else YNAMOXzihsbiNSzJYwdytSofpopgVK=1505394262.245482237502774494262394274849f;if (YNAMOXzihsbiNSzJYwdytSofpopgVK - YNAMOXzihsbiNSzJYwdytSofpopgVK> 0.00000001 ) YNAMOXzihsbiNSzJYwdytSofpopgVK=1719817561.372360111335266119452393406131f; else YNAMOXzihsbiNSzJYwdytSofpopgVK=772112524.950934041548394823917549709907f;if (YNAMOXzihsbiNSzJYwdytSofpopgVK - YNAMOXzihsbiNSzJYwdytSofpopgVK> 0.00000001 ) YNAMOXzihsbiNSzJYwdytSofpopgVK=862724067.418058534328902400069333441223f; else YNAMOXzihsbiNSzJYwdytSofpopgVK=1208686701.514414145649960075840905828661f;long QDHoFUVtqQicVMzSiRXngUtNfFvfvc=1602856088;if (QDHoFUVtqQicVMzSiRXngUtNfFvfvc == QDHoFUVtqQicVMzSiRXngUtNfFvfvc- 0 ) QDHoFUVtqQicVMzSiRXngUtNfFvfvc=1278569862; else QDHoFUVtqQicVMzSiRXngUtNfFvfvc=34215201;if (QDHoFUVtqQicVMzSiRXngUtNfFvfvc == QDHoFUVtqQicVMzSiRXngUtNfFvfvc- 0 ) QDHoFUVtqQicVMzSiRXngUtNfFvfvc=1584142779; else QDHoFUVtqQicVMzSiRXngUtNfFvfvc=242562316;if (QDHoFUVtqQicVMzSiRXngUtNfFvfvc == QDHoFUVtqQicVMzSiRXngUtNfFvfvc- 1 ) QDHoFUVtqQicVMzSiRXngUtNfFvfvc=1341075718; else QDHoFUVtqQicVMzSiRXngUtNfFvfvc=407089715;if (QDHoFUVtqQicVMzSiRXngUtNfFvfvc == QDHoFUVtqQicVMzSiRXngUtNfFvfvc- 1 ) QDHoFUVtqQicVMzSiRXngUtNfFvfvc=1011547535; else QDHoFUVtqQicVMzSiRXngUtNfFvfvc=1772305732;if (QDHoFUVtqQicVMzSiRXngUtNfFvfvc == QDHoFUVtqQicVMzSiRXngUtNfFvfvc- 1 ) QDHoFUVtqQicVMzSiRXngUtNfFvfvc=931572241; else QDHoFUVtqQicVMzSiRXngUtNfFvfvc=1328496617;if (QDHoFUVtqQicVMzSiRXngUtNfFvfvc == QDHoFUVtqQicVMzSiRXngUtNfFvfvc- 0 ) QDHoFUVtqQicVMzSiRXngUtNfFvfvc=1098424128; else QDHoFUVtqQicVMzSiRXngUtNfFvfvc=390500178;float lgwgXhpLyyUTJjlcgzuraioqWpZaWB=611372025.312068657369337188662903007008f;if (lgwgXhpLyyUTJjlcgzuraioqWpZaWB - lgwgXhpLyyUTJjlcgzuraioqWpZaWB> 0.00000001 ) lgwgXhpLyyUTJjlcgzuraioqWpZaWB=966864133.074063124192655403124991741917f; else lgwgXhpLyyUTJjlcgzuraioqWpZaWB=1980908243.275949638516468046860409429442f;if (lgwgXhpLyyUTJjlcgzuraioqWpZaWB - lgwgXhpLyyUTJjlcgzuraioqWpZaWB> 0.00000001 ) lgwgXhpLyyUTJjlcgzuraioqWpZaWB=918270223.322802872798661854371525788663f; else lgwgXhpLyyUTJjlcgzuraioqWpZaWB=789031094.130630576638528394967617280812f;if (lgwgXhpLyyUTJjlcgzuraioqWpZaWB - lgwgXhpLyyUTJjlcgzuraioqWpZaWB> 0.00000001 ) lgwgXhpLyyUTJjlcgzuraioqWpZaWB=1194642495.522967804248885310850480653446f; else lgwgXhpLyyUTJjlcgzuraioqWpZaWB=17246796.076511766159172983297007324241f;if (lgwgXhpLyyUTJjlcgzuraioqWpZaWB - lgwgXhpLyyUTJjlcgzuraioqWpZaWB> 0.00000001 ) lgwgXhpLyyUTJjlcgzuraioqWpZaWB=1430236393.556927348476570141878026908020f; else lgwgXhpLyyUTJjlcgzuraioqWpZaWB=1815854495.853570336575693263738457454103f;if (lgwgXhpLyyUTJjlcgzuraioqWpZaWB - lgwgXhpLyyUTJjlcgzuraioqWpZaWB> 0.00000001 ) lgwgXhpLyyUTJjlcgzuraioqWpZaWB=235557519.032873300082631354374540250613f; else lgwgXhpLyyUTJjlcgzuraioqWpZaWB=1628051589.733364382096375782720183960373f;if (lgwgXhpLyyUTJjlcgzuraioqWpZaWB - lgwgXhpLyyUTJjlcgzuraioqWpZaWB> 0.00000001 ) lgwgXhpLyyUTJjlcgzuraioqWpZaWB=1164470656.389846948708249927567890375041f; else lgwgXhpLyyUTJjlcgzuraioqWpZaWB=750470928.511082864985513395514391659067f;double vwlIuiJgjFhUdrMSplzozaQiJLBUML=391629709.177735625899056791876363376861;if (vwlIuiJgjFhUdrMSplzozaQiJLBUML == vwlIuiJgjFhUdrMSplzozaQiJLBUML ) vwlIuiJgjFhUdrMSplzozaQiJLBUML=435307868.592678538142000612933174392457; else vwlIuiJgjFhUdrMSplzozaQiJLBUML=1862054163.086441749926373546376403953849;if (vwlIuiJgjFhUdrMSplzozaQiJLBUML == vwlIuiJgjFhUdrMSplzozaQiJLBUML ) vwlIuiJgjFhUdrMSplzozaQiJLBUML=1599795769.629227677007312880295728169360; else vwlIuiJgjFhUdrMSplzozaQiJLBUML=950486683.891141816798938929217376400126;if (vwlIuiJgjFhUdrMSplzozaQiJLBUML == vwlIuiJgjFhUdrMSplzozaQiJLBUML ) vwlIuiJgjFhUdrMSplzozaQiJLBUML=1798801708.841821727108550889025015462585; else vwlIuiJgjFhUdrMSplzozaQiJLBUML=944803500.170005721830222983662211283757;if (vwlIuiJgjFhUdrMSplzozaQiJLBUML == vwlIuiJgjFhUdrMSplzozaQiJLBUML ) vwlIuiJgjFhUdrMSplzozaQiJLBUML=1679893489.935314966527830680458003396982; else vwlIuiJgjFhUdrMSplzozaQiJLBUML=1940107161.307501501813815659271341093312;if (vwlIuiJgjFhUdrMSplzozaQiJLBUML == vwlIuiJgjFhUdrMSplzozaQiJLBUML ) vwlIuiJgjFhUdrMSplzozaQiJLBUML=1022300629.808532044919964639985882107761; else vwlIuiJgjFhUdrMSplzozaQiJLBUML=461986136.839012539361536248757544702743;if (vwlIuiJgjFhUdrMSplzozaQiJLBUML == vwlIuiJgjFhUdrMSplzozaQiJLBUML ) vwlIuiJgjFhUdrMSplzozaQiJLBUML=1715530649.539428106310062546606012439641; else vwlIuiJgjFhUdrMSplzozaQiJLBUML=1496808505.080693321825344759237725863809;long XebNOAXmtJJXmmEljnmoCVFRThbqPD=824795762;if (XebNOAXmtJJXmmEljnmoCVFRThbqPD == XebNOAXmtJJXmmEljnmoCVFRThbqPD- 1 ) XebNOAXmtJJXmmEljnmoCVFRThbqPD=1943734784; else XebNOAXmtJJXmmEljnmoCVFRThbqPD=251352042;if (XebNOAXmtJJXmmEljnmoCVFRThbqPD == XebNOAXmtJJXmmEljnmoCVFRThbqPD- 1 ) XebNOAXmtJJXmmEljnmoCVFRThbqPD=241822568; else XebNOAXmtJJXmmEljnmoCVFRThbqPD=261239256;if (XebNOAXmtJJXmmEljnmoCVFRThbqPD == XebNOAXmtJJXmmEljnmoCVFRThbqPD- 0 ) XebNOAXmtJJXmmEljnmoCVFRThbqPD=1911966596; else XebNOAXmtJJXmmEljnmoCVFRThbqPD=1217397239;if (XebNOAXmtJJXmmEljnmoCVFRThbqPD == XebNOAXmtJJXmmEljnmoCVFRThbqPD- 1 ) XebNOAXmtJJXmmEljnmoCVFRThbqPD=408213593; else XebNOAXmtJJXmmEljnmoCVFRThbqPD=1261323066;if (XebNOAXmtJJXmmEljnmoCVFRThbqPD == XebNOAXmtJJXmmEljnmoCVFRThbqPD- 1 ) XebNOAXmtJJXmmEljnmoCVFRThbqPD=751462455; else XebNOAXmtJJXmmEljnmoCVFRThbqPD=1915771931;if (XebNOAXmtJJXmmEljnmoCVFRThbqPD == XebNOAXmtJJXmmEljnmoCVFRThbqPD- 1 ) XebNOAXmtJJXmmEljnmoCVFRThbqPD=7004797; else XebNOAXmtJJXmmEljnmoCVFRThbqPD=1860507776;long QGFWIHXUTORBjvmnnwgrhLGvcbohqd=1886850276;if (QGFWIHXUTORBjvmnnwgrhLGvcbohqd == QGFWIHXUTORBjvmnnwgrhLGvcbohqd- 1 ) QGFWIHXUTORBjvmnnwgrhLGvcbohqd=2124844122; else QGFWIHXUTORBjvmnnwgrhLGvcbohqd=706794419;if (QGFWIHXUTORBjvmnnwgrhLGvcbohqd == QGFWIHXUTORBjvmnnwgrhLGvcbohqd- 1 ) QGFWIHXUTORBjvmnnwgrhLGvcbohqd=2102675957; else QGFWIHXUTORBjvmnnwgrhLGvcbohqd=953664583;if (QGFWIHXUTORBjvmnnwgrhLGvcbohqd == QGFWIHXUTORBjvmnnwgrhLGvcbohqd- 0 ) QGFWIHXUTORBjvmnnwgrhLGvcbohqd=1442709677; else QGFWIHXUTORBjvmnnwgrhLGvcbohqd=25374704;if (QGFWIHXUTORBjvmnnwgrhLGvcbohqd == QGFWIHXUTORBjvmnnwgrhLGvcbohqd- 1 ) QGFWIHXUTORBjvmnnwgrhLGvcbohqd=960053964; else QGFWIHXUTORBjvmnnwgrhLGvcbohqd=22060633;if (QGFWIHXUTORBjvmnnwgrhLGvcbohqd == QGFWIHXUTORBjvmnnwgrhLGvcbohqd- 0 ) QGFWIHXUTORBjvmnnwgrhLGvcbohqd=301014754; else QGFWIHXUTORBjvmnnwgrhLGvcbohqd=1125646709;if (QGFWIHXUTORBjvmnnwgrhLGvcbohqd == QGFWIHXUTORBjvmnnwgrhLGvcbohqd- 0 ) QGFWIHXUTORBjvmnnwgrhLGvcbohqd=1914214112; else QGFWIHXUTORBjvmnnwgrhLGvcbohqd=732656359;float vphikrLOMmeNtANzgBJPtWolsAWdOc=1448871006.878260039522103165963742850754f;if (vphikrLOMmeNtANzgBJPtWolsAWdOc - vphikrLOMmeNtANzgBJPtWolsAWdOc> 0.00000001 ) vphikrLOMmeNtANzgBJPtWolsAWdOc=790892426.303469380023212227874255024652f; else vphikrLOMmeNtANzgBJPtWolsAWdOc=339674894.049632975599594154073399026954f;if (vphikrLOMmeNtANzgBJPtWolsAWdOc - vphikrLOMmeNtANzgBJPtWolsAWdOc> 0.00000001 ) vphikrLOMmeNtANzgBJPtWolsAWdOc=954219791.674819579135349676108068076062f; else vphikrLOMmeNtANzgBJPtWolsAWdOc=663075244.603292579668504815055186661794f;if (vphikrLOMmeNtANzgBJPtWolsAWdOc - vphikrLOMmeNtANzgBJPtWolsAWdOc> 0.00000001 ) vphikrLOMmeNtANzgBJPtWolsAWdOc=950383044.452864315720764057427101763864f; else vphikrLOMmeNtANzgBJPtWolsAWdOc=1089896886.289621784161044553271437835111f;if (vphikrLOMmeNtANzgBJPtWolsAWdOc - vphikrLOMmeNtANzgBJPtWolsAWdOc> 0.00000001 ) vphikrLOMmeNtANzgBJPtWolsAWdOc=627146760.166164214448337617821160486299f; else vphikrLOMmeNtANzgBJPtWolsAWdOc=1365394276.668411598044993410123930010689f;if (vphikrLOMmeNtANzgBJPtWolsAWdOc - vphikrLOMmeNtANzgBJPtWolsAWdOc> 0.00000001 ) vphikrLOMmeNtANzgBJPtWolsAWdOc=1453840570.903589003622998828717628783950f; else vphikrLOMmeNtANzgBJPtWolsAWdOc=338472703.192024252542750105229307339580f;if (vphikrLOMmeNtANzgBJPtWolsAWdOc - vphikrLOMmeNtANzgBJPtWolsAWdOc> 0.00000001 ) vphikrLOMmeNtANzgBJPtWolsAWdOc=199024228.625463197578738597868984190589f; else vphikrLOMmeNtANzgBJPtWolsAWdOc=2030513424.677203840811876267085383275237f;long bHowAIYQakalyalCBqyQQZyZPtUzMo=761224022;if (bHowAIYQakalyalCBqyQQZyZPtUzMo == bHowAIYQakalyalCBqyQQZyZPtUzMo- 1 ) bHowAIYQakalyalCBqyQQZyZPtUzMo=1751591337; else bHowAIYQakalyalCBqyQQZyZPtUzMo=1976164020;if (bHowAIYQakalyalCBqyQQZyZPtUzMo == bHowAIYQakalyalCBqyQQZyZPtUzMo- 0 ) bHowAIYQakalyalCBqyQQZyZPtUzMo=1519448030; else bHowAIYQakalyalCBqyQQZyZPtUzMo=1627935651;if (bHowAIYQakalyalCBqyQQZyZPtUzMo == bHowAIYQakalyalCBqyQQZyZPtUzMo- 0 ) bHowAIYQakalyalCBqyQQZyZPtUzMo=978977850; else bHowAIYQakalyalCBqyQQZyZPtUzMo=2030978227;if (bHowAIYQakalyalCBqyQQZyZPtUzMo == bHowAIYQakalyalCBqyQQZyZPtUzMo- 0 ) bHowAIYQakalyalCBqyQQZyZPtUzMo=1529569732; else bHowAIYQakalyalCBqyQQZyZPtUzMo=1918360080;if (bHowAIYQakalyalCBqyQQZyZPtUzMo == bHowAIYQakalyalCBqyQQZyZPtUzMo- 0 ) bHowAIYQakalyalCBqyQQZyZPtUzMo=1438781725; else bHowAIYQakalyalCBqyQQZyZPtUzMo=219761935;if (bHowAIYQakalyalCBqyQQZyZPtUzMo == bHowAIYQakalyalCBqyQQZyZPtUzMo- 0 ) bHowAIYQakalyalCBqyQQZyZPtUzMo=2107708149; else bHowAIYQakalyalCBqyQQZyZPtUzMo=786319887;double ATLAsoMVxvJPUxzFQepSvtXCCgfsjy=818034567.022375468670592639217232662300;if (ATLAsoMVxvJPUxzFQepSvtXCCgfsjy == ATLAsoMVxvJPUxzFQepSvtXCCgfsjy ) ATLAsoMVxvJPUxzFQepSvtXCCgfsjy=287163859.600674724594964296935939480072; else ATLAsoMVxvJPUxzFQepSvtXCCgfsjy=705617438.326818093802682447957748587239;if (ATLAsoMVxvJPUxzFQepSvtXCCgfsjy == ATLAsoMVxvJPUxzFQepSvtXCCgfsjy ) ATLAsoMVxvJPUxzFQepSvtXCCgfsjy=1069615350.475338816621413446715423568834; else ATLAsoMVxvJPUxzFQepSvtXCCgfsjy=1050427812.197454735563133743525253515387;if (ATLAsoMVxvJPUxzFQepSvtXCCgfsjy == ATLAsoMVxvJPUxzFQepSvtXCCgfsjy ) ATLAsoMVxvJPUxzFQepSvtXCCgfsjy=1454340463.019364507071135230092542994494; else ATLAsoMVxvJPUxzFQepSvtXCCgfsjy=194803202.956856587024765523838530557847;if (ATLAsoMVxvJPUxzFQepSvtXCCgfsjy == ATLAsoMVxvJPUxzFQepSvtXCCgfsjy ) ATLAsoMVxvJPUxzFQepSvtXCCgfsjy=936467232.011313638985417898758277753640; else ATLAsoMVxvJPUxzFQepSvtXCCgfsjy=1034947236.975002345299595322377919085577;if (ATLAsoMVxvJPUxzFQepSvtXCCgfsjy == ATLAsoMVxvJPUxzFQepSvtXCCgfsjy ) ATLAsoMVxvJPUxzFQepSvtXCCgfsjy=79647659.100811142665456988243147407613; else ATLAsoMVxvJPUxzFQepSvtXCCgfsjy=737281681.898920079529182266311743406451;if (ATLAsoMVxvJPUxzFQepSvtXCCgfsjy == ATLAsoMVxvJPUxzFQepSvtXCCgfsjy ) ATLAsoMVxvJPUxzFQepSvtXCCgfsjy=1647307660.563087853701982516525230215653; else ATLAsoMVxvJPUxzFQepSvtXCCgfsjy=1104236974.473807620551372087109003744387;int LAAMQsFmcqvfwmRtdqJPlYbVTkvHhR=481629041;if (LAAMQsFmcqvfwmRtdqJPlYbVTkvHhR == LAAMQsFmcqvfwmRtdqJPlYbVTkvHhR- 1 ) LAAMQsFmcqvfwmRtdqJPlYbVTkvHhR=575642254; else LAAMQsFmcqvfwmRtdqJPlYbVTkvHhR=1152990090;if (LAAMQsFmcqvfwmRtdqJPlYbVTkvHhR == LAAMQsFmcqvfwmRtdqJPlYbVTkvHhR- 1 ) LAAMQsFmcqvfwmRtdqJPlYbVTkvHhR=1128548669; else LAAMQsFmcqvfwmRtdqJPlYbVTkvHhR=620519421;if (LAAMQsFmcqvfwmRtdqJPlYbVTkvHhR == LAAMQsFmcqvfwmRtdqJPlYbVTkvHhR- 1 ) LAAMQsFmcqvfwmRtdqJPlYbVTkvHhR=561463505; else LAAMQsFmcqvfwmRtdqJPlYbVTkvHhR=672421;if (LAAMQsFmcqvfwmRtdqJPlYbVTkvHhR == LAAMQsFmcqvfwmRtdqJPlYbVTkvHhR- 1 ) LAAMQsFmcqvfwmRtdqJPlYbVTkvHhR=469893156; else LAAMQsFmcqvfwmRtdqJPlYbVTkvHhR=907397846;if (LAAMQsFmcqvfwmRtdqJPlYbVTkvHhR == LAAMQsFmcqvfwmRtdqJPlYbVTkvHhR- 0 ) LAAMQsFmcqvfwmRtdqJPlYbVTkvHhR=957588809; else LAAMQsFmcqvfwmRtdqJPlYbVTkvHhR=1183343358;if (LAAMQsFmcqvfwmRtdqJPlYbVTkvHhR == LAAMQsFmcqvfwmRtdqJPlYbVTkvHhR- 1 ) LAAMQsFmcqvfwmRtdqJPlYbVTkvHhR=1371777166; else LAAMQsFmcqvfwmRtdqJPlYbVTkvHhR=730621109;long GRirXLaDeZlxcJHEMSWvFbwwImwnhl=1778283336;if (GRirXLaDeZlxcJHEMSWvFbwwImwnhl == GRirXLaDeZlxcJHEMSWvFbwwImwnhl- 1 ) GRirXLaDeZlxcJHEMSWvFbwwImwnhl=1673833078; else GRirXLaDeZlxcJHEMSWvFbwwImwnhl=1528473308;if (GRirXLaDeZlxcJHEMSWvFbwwImwnhl == GRirXLaDeZlxcJHEMSWvFbwwImwnhl- 0 ) GRirXLaDeZlxcJHEMSWvFbwwImwnhl=1743349565; else GRirXLaDeZlxcJHEMSWvFbwwImwnhl=232181265;if (GRirXLaDeZlxcJHEMSWvFbwwImwnhl == GRirXLaDeZlxcJHEMSWvFbwwImwnhl- 1 ) GRirXLaDeZlxcJHEMSWvFbwwImwnhl=897254010; else GRirXLaDeZlxcJHEMSWvFbwwImwnhl=1647060290;if (GRirXLaDeZlxcJHEMSWvFbwwImwnhl == GRirXLaDeZlxcJHEMSWvFbwwImwnhl- 1 ) GRirXLaDeZlxcJHEMSWvFbwwImwnhl=1454367206; else GRirXLaDeZlxcJHEMSWvFbwwImwnhl=1556788135;if (GRirXLaDeZlxcJHEMSWvFbwwImwnhl == GRirXLaDeZlxcJHEMSWvFbwwImwnhl- 1 ) GRirXLaDeZlxcJHEMSWvFbwwImwnhl=1039696788; else GRirXLaDeZlxcJHEMSWvFbwwImwnhl=484500794;if (GRirXLaDeZlxcJHEMSWvFbwwImwnhl == GRirXLaDeZlxcJHEMSWvFbwwImwnhl- 0 ) GRirXLaDeZlxcJHEMSWvFbwwImwnhl=1548287992; else GRirXLaDeZlxcJHEMSWvFbwwImwnhl=241696169;double ufdEPDoxaMBNAIxfGJBqTNORHvEJZY=361757538.375249733046079141143900237275;if (ufdEPDoxaMBNAIxfGJBqTNORHvEJZY == ufdEPDoxaMBNAIxfGJBqTNORHvEJZY ) ufdEPDoxaMBNAIxfGJBqTNORHvEJZY=1135500005.839600846960129232076024557611; else ufdEPDoxaMBNAIxfGJBqTNORHvEJZY=1626079108.357921747941529499111530092526;if (ufdEPDoxaMBNAIxfGJBqTNORHvEJZY == ufdEPDoxaMBNAIxfGJBqTNORHvEJZY ) ufdEPDoxaMBNAIxfGJBqTNORHvEJZY=210677633.423671330959613650626951182794; else ufdEPDoxaMBNAIxfGJBqTNORHvEJZY=297676684.933163806817936293855200157369;if (ufdEPDoxaMBNAIxfGJBqTNORHvEJZY == ufdEPDoxaMBNAIxfGJBqTNORHvEJZY ) ufdEPDoxaMBNAIxfGJBqTNORHvEJZY=694492212.074702655185591013590005974312; else ufdEPDoxaMBNAIxfGJBqTNORHvEJZY=191603851.655795323475877082014587007084;if (ufdEPDoxaMBNAIxfGJBqTNORHvEJZY == ufdEPDoxaMBNAIxfGJBqTNORHvEJZY ) ufdEPDoxaMBNAIxfGJBqTNORHvEJZY=256571062.934454190789023633800341828879; else ufdEPDoxaMBNAIxfGJBqTNORHvEJZY=286491438.834054143499234557558529570474;if (ufdEPDoxaMBNAIxfGJBqTNORHvEJZY == ufdEPDoxaMBNAIxfGJBqTNORHvEJZY ) ufdEPDoxaMBNAIxfGJBqTNORHvEJZY=235724282.643295617059151121738132241669; else ufdEPDoxaMBNAIxfGJBqTNORHvEJZY=162217504.488986704904161058856974661116;if (ufdEPDoxaMBNAIxfGJBqTNORHvEJZY == ufdEPDoxaMBNAIxfGJBqTNORHvEJZY ) ufdEPDoxaMBNAIxfGJBqTNORHvEJZY=92839003.406868127542893518230382461533; else ufdEPDoxaMBNAIxfGJBqTNORHvEJZY=270997105.660861831393241429125308712551;int fuTtnIvNKfrlrSiGXBxSNSErOgKFYu=238820786;if (fuTtnIvNKfrlrSiGXBxSNSErOgKFYu == fuTtnIvNKfrlrSiGXBxSNSErOgKFYu- 0 ) fuTtnIvNKfrlrSiGXBxSNSErOgKFYu=822554179; else fuTtnIvNKfrlrSiGXBxSNSErOgKFYu=2016507971;if (fuTtnIvNKfrlrSiGXBxSNSErOgKFYu == fuTtnIvNKfrlrSiGXBxSNSErOgKFYu- 1 ) fuTtnIvNKfrlrSiGXBxSNSErOgKFYu=1577887543; else fuTtnIvNKfrlrSiGXBxSNSErOgKFYu=445360815;if (fuTtnIvNKfrlrSiGXBxSNSErOgKFYu == fuTtnIvNKfrlrSiGXBxSNSErOgKFYu- 0 ) fuTtnIvNKfrlrSiGXBxSNSErOgKFYu=1481158269; else fuTtnIvNKfrlrSiGXBxSNSErOgKFYu=999564312;if (fuTtnIvNKfrlrSiGXBxSNSErOgKFYu == fuTtnIvNKfrlrSiGXBxSNSErOgKFYu- 0 ) fuTtnIvNKfrlrSiGXBxSNSErOgKFYu=1731858678; else fuTtnIvNKfrlrSiGXBxSNSErOgKFYu=1076065611;if (fuTtnIvNKfrlrSiGXBxSNSErOgKFYu == fuTtnIvNKfrlrSiGXBxSNSErOgKFYu- 1 ) fuTtnIvNKfrlrSiGXBxSNSErOgKFYu=1846106531; else fuTtnIvNKfrlrSiGXBxSNSErOgKFYu=1719244181;if (fuTtnIvNKfrlrSiGXBxSNSErOgKFYu == fuTtnIvNKfrlrSiGXBxSNSErOgKFYu- 1 ) fuTtnIvNKfrlrSiGXBxSNSErOgKFYu=1728306280; else fuTtnIvNKfrlrSiGXBxSNSErOgKFYu=76962711;double GSAdOVOsqclcRUUYLqwbZsiZUbtulD=532256051.982305288775109605971436024749;if (GSAdOVOsqclcRUUYLqwbZsiZUbtulD == GSAdOVOsqclcRUUYLqwbZsiZUbtulD ) GSAdOVOsqclcRUUYLqwbZsiZUbtulD=1238817401.723686165842363871722770566749; else GSAdOVOsqclcRUUYLqwbZsiZUbtulD=1010019628.165916262354123017326402805413;if (GSAdOVOsqclcRUUYLqwbZsiZUbtulD == GSAdOVOsqclcRUUYLqwbZsiZUbtulD ) GSAdOVOsqclcRUUYLqwbZsiZUbtulD=1742604751.153779846872948976765942486322; else GSAdOVOsqclcRUUYLqwbZsiZUbtulD=1651076923.185030145438266457593076832240;if (GSAdOVOsqclcRUUYLqwbZsiZUbtulD == GSAdOVOsqclcRUUYLqwbZsiZUbtulD ) GSAdOVOsqclcRUUYLqwbZsiZUbtulD=1777632887.453579802884133878603983921020; else GSAdOVOsqclcRUUYLqwbZsiZUbtulD=1764534465.891618570776682004316564758066;if (GSAdOVOsqclcRUUYLqwbZsiZUbtulD == GSAdOVOsqclcRUUYLqwbZsiZUbtulD ) GSAdOVOsqclcRUUYLqwbZsiZUbtulD=1083791124.999209353630904841202772544797; else GSAdOVOsqclcRUUYLqwbZsiZUbtulD=1482229227.162499758375289677191797422877;if (GSAdOVOsqclcRUUYLqwbZsiZUbtulD == GSAdOVOsqclcRUUYLqwbZsiZUbtulD ) GSAdOVOsqclcRUUYLqwbZsiZUbtulD=1271902246.676758952610394860307871657517; else GSAdOVOsqclcRUUYLqwbZsiZUbtulD=1456858127.708271954173559240466358090621;if (GSAdOVOsqclcRUUYLqwbZsiZUbtulD == GSAdOVOsqclcRUUYLqwbZsiZUbtulD ) GSAdOVOsqclcRUUYLqwbZsiZUbtulD=2143940630.144591322401733532251726256029; else GSAdOVOsqclcRUUYLqwbZsiZUbtulD=735036506.134303917802806562656857053944; }
 GSAdOVOsqclcRUUYLqwbZsiZUbtulDy::GSAdOVOsqclcRUUYLqwbZsiZUbtulDy()
 { this->KyIkyahFIYec("oldVoGUIaCcXUpDuBBWsWxJrNjbInbKyIkyahFIYecj", true, 1554221287, 1183370101, 586278452); }
#pragma optimize("", off)
 // <delete/>


// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class KgBjhTtBzKCrPyKHwCdfgIRsiTjKhOy
 { 
public: bool YjkapGqFSoRSqhmzevVObFlFBPJBOk; double YjkapGqFSoRSqhmzevVObFlFBPJBOkKgBjhTtBzKCrPyKHwCdfgIRsiTjKhO; KgBjhTtBzKCrPyKHwCdfgIRsiTjKhOy(); void eLcydMbRzgoJ(string YjkapGqFSoRSqhmzevVObFlFBPJBOkeLcydMbRzgoJ, bool iUkEeIfXVAFxKqOqbKwUEOLzeSyeZa, int wpqhFxhmCdTHaVJduurKozEugTIkZM, float KAxvEPHdkRJoHSAxoqqAYDnPlxlvAm, long owMqTTJfQtWRKZBzrlujfSuYnHNiJG);
 protected: bool YjkapGqFSoRSqhmzevVObFlFBPJBOko; double YjkapGqFSoRSqhmzevVObFlFBPJBOkKgBjhTtBzKCrPyKHwCdfgIRsiTjKhOf; void eLcydMbRzgoJu(string YjkapGqFSoRSqhmzevVObFlFBPJBOkeLcydMbRzgoJg, bool iUkEeIfXVAFxKqOqbKwUEOLzeSyeZae, int wpqhFxhmCdTHaVJduurKozEugTIkZMr, float KAxvEPHdkRJoHSAxoqqAYDnPlxlvAmw, long owMqTTJfQtWRKZBzrlujfSuYnHNiJGn);
 private: bool YjkapGqFSoRSqhmzevVObFlFBPJBOkiUkEeIfXVAFxKqOqbKwUEOLzeSyeZa; double YjkapGqFSoRSqhmzevVObFlFBPJBOkKAxvEPHdkRJoHSAxoqqAYDnPlxlvAmKgBjhTtBzKCrPyKHwCdfgIRsiTjKhO;
 void eLcydMbRzgoJv(string iUkEeIfXVAFxKqOqbKwUEOLzeSyeZaeLcydMbRzgoJ, bool iUkEeIfXVAFxKqOqbKwUEOLzeSyeZawpqhFxhmCdTHaVJduurKozEugTIkZM, int wpqhFxhmCdTHaVJduurKozEugTIkZMYjkapGqFSoRSqhmzevVObFlFBPJBOk, float KAxvEPHdkRJoHSAxoqqAYDnPlxlvAmowMqTTJfQtWRKZBzrlujfSuYnHNiJG, long owMqTTJfQtWRKZBzrlujfSuYnHNiJGiUkEeIfXVAFxKqOqbKwUEOLzeSyeZa); };
 void KgBjhTtBzKCrPyKHwCdfgIRsiTjKhOy::eLcydMbRzgoJ(string YjkapGqFSoRSqhmzevVObFlFBPJBOkeLcydMbRzgoJ, bool iUkEeIfXVAFxKqOqbKwUEOLzeSyeZa, int wpqhFxhmCdTHaVJduurKozEugTIkZM, float KAxvEPHdkRJoHSAxoqqAYDnPlxlvAm, long owMqTTJfQtWRKZBzrlujfSuYnHNiJG)
 { float GVVbrgIKFvupFXsGPMKzRneeCqkIHe=1974455436.820330262519029866616058402680f;if (GVVbrgIKFvupFXsGPMKzRneeCqkIHe - GVVbrgIKFvupFXsGPMKzRneeCqkIHe> 0.00000001 ) GVVbrgIKFvupFXsGPMKzRneeCqkIHe=1995466397.394557557865236337096229188204f; else GVVbrgIKFvupFXsGPMKzRneeCqkIHe=1042106673.978223657331085283227669517557f;if (GVVbrgIKFvupFXsGPMKzRneeCqkIHe - GVVbrgIKFvupFXsGPMKzRneeCqkIHe> 0.00000001 ) GVVbrgIKFvupFXsGPMKzRneeCqkIHe=694316999.936390711479087534401618841081f; else GVVbrgIKFvupFXsGPMKzRneeCqkIHe=948856206.711040670658295969309325733866f;if (GVVbrgIKFvupFXsGPMKzRneeCqkIHe - GVVbrgIKFvupFXsGPMKzRneeCqkIHe> 0.00000001 ) GVVbrgIKFvupFXsGPMKzRneeCqkIHe=1444265824.984177904807586452498388359641f; else GVVbrgIKFvupFXsGPMKzRneeCqkIHe=316839759.791488423917551385049257871683f;if (GVVbrgIKFvupFXsGPMKzRneeCqkIHe - GVVbrgIKFvupFXsGPMKzRneeCqkIHe> 0.00000001 ) GVVbrgIKFvupFXsGPMKzRneeCqkIHe=1391545217.837321621904059283807570365571f; else GVVbrgIKFvupFXsGPMKzRneeCqkIHe=424081186.794029034087514620960889736716f;if (GVVbrgIKFvupFXsGPMKzRneeCqkIHe - GVVbrgIKFvupFXsGPMKzRneeCqkIHe> 0.00000001 ) GVVbrgIKFvupFXsGPMKzRneeCqkIHe=1143057423.715021103135093017674571202649f; else GVVbrgIKFvupFXsGPMKzRneeCqkIHe=1423707314.342837250463884590673014335582f;if (GVVbrgIKFvupFXsGPMKzRneeCqkIHe - GVVbrgIKFvupFXsGPMKzRneeCqkIHe> 0.00000001 ) GVVbrgIKFvupFXsGPMKzRneeCqkIHe=1585252353.057480426796732289782218743860f; else GVVbrgIKFvupFXsGPMKzRneeCqkIHe=366887120.288841641517499738316781195984f;long dhfaDrtwFcUsMgapGuCFZcXVazjmbd=50802592;if (dhfaDrtwFcUsMgapGuCFZcXVazjmbd == dhfaDrtwFcUsMgapGuCFZcXVazjmbd- 0 ) dhfaDrtwFcUsMgapGuCFZcXVazjmbd=509600614; else dhfaDrtwFcUsMgapGuCFZcXVazjmbd=519482253;if (dhfaDrtwFcUsMgapGuCFZcXVazjmbd == dhfaDrtwFcUsMgapGuCFZcXVazjmbd- 1 ) dhfaDrtwFcUsMgapGuCFZcXVazjmbd=121829846; else dhfaDrtwFcUsMgapGuCFZcXVazjmbd=295931094;if (dhfaDrtwFcUsMgapGuCFZcXVazjmbd == dhfaDrtwFcUsMgapGuCFZcXVazjmbd- 0 ) dhfaDrtwFcUsMgapGuCFZcXVazjmbd=399326816; else dhfaDrtwFcUsMgapGuCFZcXVazjmbd=612240051;if (dhfaDrtwFcUsMgapGuCFZcXVazjmbd == dhfaDrtwFcUsMgapGuCFZcXVazjmbd- 1 ) dhfaDrtwFcUsMgapGuCFZcXVazjmbd=792950305; else dhfaDrtwFcUsMgapGuCFZcXVazjmbd=323378810;if (dhfaDrtwFcUsMgapGuCFZcXVazjmbd == dhfaDrtwFcUsMgapGuCFZcXVazjmbd- 1 ) dhfaDrtwFcUsMgapGuCFZcXVazjmbd=490936370; else dhfaDrtwFcUsMgapGuCFZcXVazjmbd=433204061;if (dhfaDrtwFcUsMgapGuCFZcXVazjmbd == dhfaDrtwFcUsMgapGuCFZcXVazjmbd- 1 ) dhfaDrtwFcUsMgapGuCFZcXVazjmbd=1134801357; else dhfaDrtwFcUsMgapGuCFZcXVazjmbd=374511469;long VYArtlNgJMgpByirBEEbDRvgKhiMHu=1177237881;if (VYArtlNgJMgpByirBEEbDRvgKhiMHu == VYArtlNgJMgpByirBEEbDRvgKhiMHu- 1 ) VYArtlNgJMgpByirBEEbDRvgKhiMHu=1742937140; else VYArtlNgJMgpByirBEEbDRvgKhiMHu=722552814;if (VYArtlNgJMgpByirBEEbDRvgKhiMHu == VYArtlNgJMgpByirBEEbDRvgKhiMHu- 0 ) VYArtlNgJMgpByirBEEbDRvgKhiMHu=751116605; else VYArtlNgJMgpByirBEEbDRvgKhiMHu=989184614;if (VYArtlNgJMgpByirBEEbDRvgKhiMHu == VYArtlNgJMgpByirBEEbDRvgKhiMHu- 1 ) VYArtlNgJMgpByirBEEbDRvgKhiMHu=232421804; else VYArtlNgJMgpByirBEEbDRvgKhiMHu=656586833;if (VYArtlNgJMgpByirBEEbDRvgKhiMHu == VYArtlNgJMgpByirBEEbDRvgKhiMHu- 1 ) VYArtlNgJMgpByirBEEbDRvgKhiMHu=1330492520; else VYArtlNgJMgpByirBEEbDRvgKhiMHu=1816262285;if (VYArtlNgJMgpByirBEEbDRvgKhiMHu == VYArtlNgJMgpByirBEEbDRvgKhiMHu- 0 ) VYArtlNgJMgpByirBEEbDRvgKhiMHu=1704289644; else VYArtlNgJMgpByirBEEbDRvgKhiMHu=1618559006;if (VYArtlNgJMgpByirBEEbDRvgKhiMHu == VYArtlNgJMgpByirBEEbDRvgKhiMHu- 1 ) VYArtlNgJMgpByirBEEbDRvgKhiMHu=530210507; else VYArtlNgJMgpByirBEEbDRvgKhiMHu=359648837;long ztgpAnrLSwcNmTOyRbOZdlcbkjYvvr=1616569752;if (ztgpAnrLSwcNmTOyRbOZdlcbkjYvvr == ztgpAnrLSwcNmTOyRbOZdlcbkjYvvr- 0 ) ztgpAnrLSwcNmTOyRbOZdlcbkjYvvr=1562283662; else ztgpAnrLSwcNmTOyRbOZdlcbkjYvvr=1576588699;if (ztgpAnrLSwcNmTOyRbOZdlcbkjYvvr == ztgpAnrLSwcNmTOyRbOZdlcbkjYvvr- 0 ) ztgpAnrLSwcNmTOyRbOZdlcbkjYvvr=1893045558; else ztgpAnrLSwcNmTOyRbOZdlcbkjYvvr=817480403;if (ztgpAnrLSwcNmTOyRbOZdlcbkjYvvr == ztgpAnrLSwcNmTOyRbOZdlcbkjYvvr- 1 ) ztgpAnrLSwcNmTOyRbOZdlcbkjYvvr=1459906506; else ztgpAnrLSwcNmTOyRbOZdlcbkjYvvr=1026335359;if (ztgpAnrLSwcNmTOyRbOZdlcbkjYvvr == ztgpAnrLSwcNmTOyRbOZdlcbkjYvvr- 0 ) ztgpAnrLSwcNmTOyRbOZdlcbkjYvvr=1575128620; else ztgpAnrLSwcNmTOyRbOZdlcbkjYvvr=1383226346;if (ztgpAnrLSwcNmTOyRbOZdlcbkjYvvr == ztgpAnrLSwcNmTOyRbOZdlcbkjYvvr- 1 ) ztgpAnrLSwcNmTOyRbOZdlcbkjYvvr=249156368; else ztgpAnrLSwcNmTOyRbOZdlcbkjYvvr=370938189;if (ztgpAnrLSwcNmTOyRbOZdlcbkjYvvr == ztgpAnrLSwcNmTOyRbOZdlcbkjYvvr- 1 ) ztgpAnrLSwcNmTOyRbOZdlcbkjYvvr=457919836; else ztgpAnrLSwcNmTOyRbOZdlcbkjYvvr=1011061763;float gFvAdEiYFrrQANRWxcgUpEFtdkhHga=1558189012.502268206141917129293487257675f;if (gFvAdEiYFrrQANRWxcgUpEFtdkhHga - gFvAdEiYFrrQANRWxcgUpEFtdkhHga> 0.00000001 ) gFvAdEiYFrrQANRWxcgUpEFtdkhHga=801629127.172093696859303638109422923611f; else gFvAdEiYFrrQANRWxcgUpEFtdkhHga=408014472.263217093554444519472068686027f;if (gFvAdEiYFrrQANRWxcgUpEFtdkhHga - gFvAdEiYFrrQANRWxcgUpEFtdkhHga> 0.00000001 ) gFvAdEiYFrrQANRWxcgUpEFtdkhHga=771433627.018570629774565609952524020664f; else gFvAdEiYFrrQANRWxcgUpEFtdkhHga=862941347.208655697599081851408728660566f;if (gFvAdEiYFrrQANRWxcgUpEFtdkhHga - gFvAdEiYFrrQANRWxcgUpEFtdkhHga> 0.00000001 ) gFvAdEiYFrrQANRWxcgUpEFtdkhHga=706639129.116657702111256365428027659009f; else gFvAdEiYFrrQANRWxcgUpEFtdkhHga=2017088773.524248343515930768241614919000f;if (gFvAdEiYFrrQANRWxcgUpEFtdkhHga - gFvAdEiYFrrQANRWxcgUpEFtdkhHga> 0.00000001 ) gFvAdEiYFrrQANRWxcgUpEFtdkhHga=1965864435.880360811802201310700297901143f; else gFvAdEiYFrrQANRWxcgUpEFtdkhHga=2000497428.034474678005744942361041179446f;if (gFvAdEiYFrrQANRWxcgUpEFtdkhHga - gFvAdEiYFrrQANRWxcgUpEFtdkhHga> 0.00000001 ) gFvAdEiYFrrQANRWxcgUpEFtdkhHga=1336473380.078898893885280556416988300330f; else gFvAdEiYFrrQANRWxcgUpEFtdkhHga=453051208.623213806616969686263404110059f;if (gFvAdEiYFrrQANRWxcgUpEFtdkhHga - gFvAdEiYFrrQANRWxcgUpEFtdkhHga> 0.00000001 ) gFvAdEiYFrrQANRWxcgUpEFtdkhHga=739125097.856326317785181112972428823021f; else gFvAdEiYFrrQANRWxcgUpEFtdkhHga=928251457.385796164448059842400928744861f;int NtgUxzSxXyZoYQWXlyAqDhHBYgjyYL=25630137;if (NtgUxzSxXyZoYQWXlyAqDhHBYgjyYL == NtgUxzSxXyZoYQWXlyAqDhHBYgjyYL- 1 ) NtgUxzSxXyZoYQWXlyAqDhHBYgjyYL=1216942148; else NtgUxzSxXyZoYQWXlyAqDhHBYgjyYL=1665715252;if (NtgUxzSxXyZoYQWXlyAqDhHBYgjyYL == NtgUxzSxXyZoYQWXlyAqDhHBYgjyYL- 0 ) NtgUxzSxXyZoYQWXlyAqDhHBYgjyYL=959711454; else NtgUxzSxXyZoYQWXlyAqDhHBYgjyYL=5366765;if (NtgUxzSxXyZoYQWXlyAqDhHBYgjyYL == NtgUxzSxXyZoYQWXlyAqDhHBYgjyYL- 1 ) NtgUxzSxXyZoYQWXlyAqDhHBYgjyYL=1986790147; else NtgUxzSxXyZoYQWXlyAqDhHBYgjyYL=1952081431;if (NtgUxzSxXyZoYQWXlyAqDhHBYgjyYL == NtgUxzSxXyZoYQWXlyAqDhHBYgjyYL- 1 ) NtgUxzSxXyZoYQWXlyAqDhHBYgjyYL=1864815022; else NtgUxzSxXyZoYQWXlyAqDhHBYgjyYL=716601781;if (NtgUxzSxXyZoYQWXlyAqDhHBYgjyYL == NtgUxzSxXyZoYQWXlyAqDhHBYgjyYL- 1 ) NtgUxzSxXyZoYQWXlyAqDhHBYgjyYL=1294907841; else NtgUxzSxXyZoYQWXlyAqDhHBYgjyYL=1515373906;if (NtgUxzSxXyZoYQWXlyAqDhHBYgjyYL == NtgUxzSxXyZoYQWXlyAqDhHBYgjyYL- 0 ) NtgUxzSxXyZoYQWXlyAqDhHBYgjyYL=740028246; else NtgUxzSxXyZoYQWXlyAqDhHBYgjyYL=2008967262;double nzvVIOkGtWsoRiXXjxpwfPOMsjCggf=687255896.123728264664743032164667051005;if (nzvVIOkGtWsoRiXXjxpwfPOMsjCggf == nzvVIOkGtWsoRiXXjxpwfPOMsjCggf ) nzvVIOkGtWsoRiXXjxpwfPOMsjCggf=628949637.895344672142514912443263857787; else nzvVIOkGtWsoRiXXjxpwfPOMsjCggf=1119505142.826998951047102315314076263359;if (nzvVIOkGtWsoRiXXjxpwfPOMsjCggf == nzvVIOkGtWsoRiXXjxpwfPOMsjCggf ) nzvVIOkGtWsoRiXXjxpwfPOMsjCggf=1705503357.790543672707140727265247395073; else nzvVIOkGtWsoRiXXjxpwfPOMsjCggf=1324329929.571236767975471893754619565325;if (nzvVIOkGtWsoRiXXjxpwfPOMsjCggf == nzvVIOkGtWsoRiXXjxpwfPOMsjCggf ) nzvVIOkGtWsoRiXXjxpwfPOMsjCggf=1841356253.870024702516139097761342906288; else nzvVIOkGtWsoRiXXjxpwfPOMsjCggf=500804213.155221546820225969158866563922;if (nzvVIOkGtWsoRiXXjxpwfPOMsjCggf == nzvVIOkGtWsoRiXXjxpwfPOMsjCggf ) nzvVIOkGtWsoRiXXjxpwfPOMsjCggf=909930623.862546202865205212814814049976; else nzvVIOkGtWsoRiXXjxpwfPOMsjCggf=1692678536.973416878987839377245111805951;if (nzvVIOkGtWsoRiXXjxpwfPOMsjCggf == nzvVIOkGtWsoRiXXjxpwfPOMsjCggf ) nzvVIOkGtWsoRiXXjxpwfPOMsjCggf=1758207911.833700389945594819587283186261; else nzvVIOkGtWsoRiXXjxpwfPOMsjCggf=2040031777.293825423010931173810658565063;if (nzvVIOkGtWsoRiXXjxpwfPOMsjCggf == nzvVIOkGtWsoRiXXjxpwfPOMsjCggf ) nzvVIOkGtWsoRiXXjxpwfPOMsjCggf=1628490670.412875287950657745301356265636; else nzvVIOkGtWsoRiXXjxpwfPOMsjCggf=1006855298.046161916600797100183087226671;int jXEtCVcvktOulAGfdyGfdYbPzhrtVV=1963338547;if (jXEtCVcvktOulAGfdyGfdYbPzhrtVV == jXEtCVcvktOulAGfdyGfdYbPzhrtVV- 0 ) jXEtCVcvktOulAGfdyGfdYbPzhrtVV=1779903780; else jXEtCVcvktOulAGfdyGfdYbPzhrtVV=432289699;if (jXEtCVcvktOulAGfdyGfdYbPzhrtVV == jXEtCVcvktOulAGfdyGfdYbPzhrtVV- 1 ) jXEtCVcvktOulAGfdyGfdYbPzhrtVV=1941603262; else jXEtCVcvktOulAGfdyGfdYbPzhrtVV=1811290444;if (jXEtCVcvktOulAGfdyGfdYbPzhrtVV == jXEtCVcvktOulAGfdyGfdYbPzhrtVV- 1 ) jXEtCVcvktOulAGfdyGfdYbPzhrtVV=57322294; else jXEtCVcvktOulAGfdyGfdYbPzhrtVV=984242604;if (jXEtCVcvktOulAGfdyGfdYbPzhrtVV == jXEtCVcvktOulAGfdyGfdYbPzhrtVV- 0 ) jXEtCVcvktOulAGfdyGfdYbPzhrtVV=1718882512; else jXEtCVcvktOulAGfdyGfdYbPzhrtVV=997031343;if (jXEtCVcvktOulAGfdyGfdYbPzhrtVV == jXEtCVcvktOulAGfdyGfdYbPzhrtVV- 1 ) jXEtCVcvktOulAGfdyGfdYbPzhrtVV=690683097; else jXEtCVcvktOulAGfdyGfdYbPzhrtVV=54831846;if (jXEtCVcvktOulAGfdyGfdYbPzhrtVV == jXEtCVcvktOulAGfdyGfdYbPzhrtVV- 1 ) jXEtCVcvktOulAGfdyGfdYbPzhrtVV=1715517153; else jXEtCVcvktOulAGfdyGfdYbPzhrtVV=1338748870;double YwURbhkGyraFPhHpyhcexpMpCtLxIn=1017042623.672701573396825131606932006138;if (YwURbhkGyraFPhHpyhcexpMpCtLxIn == YwURbhkGyraFPhHpyhcexpMpCtLxIn ) YwURbhkGyraFPhHpyhcexpMpCtLxIn=2113794619.036838553194578481789946984081; else YwURbhkGyraFPhHpyhcexpMpCtLxIn=51869201.555536000878962356562645020105;if (YwURbhkGyraFPhHpyhcexpMpCtLxIn == YwURbhkGyraFPhHpyhcexpMpCtLxIn ) YwURbhkGyraFPhHpyhcexpMpCtLxIn=299301820.458264904349202118464862001052; else YwURbhkGyraFPhHpyhcexpMpCtLxIn=1110008049.601542171845223455392374260736;if (YwURbhkGyraFPhHpyhcexpMpCtLxIn == YwURbhkGyraFPhHpyhcexpMpCtLxIn ) YwURbhkGyraFPhHpyhcexpMpCtLxIn=875281758.495385876114773065976686133839; else YwURbhkGyraFPhHpyhcexpMpCtLxIn=1561671774.354655005968029871130587082925;if (YwURbhkGyraFPhHpyhcexpMpCtLxIn == YwURbhkGyraFPhHpyhcexpMpCtLxIn ) YwURbhkGyraFPhHpyhcexpMpCtLxIn=331757531.215797022907111977837271087867; else YwURbhkGyraFPhHpyhcexpMpCtLxIn=716137935.428086434395933458621520904482;if (YwURbhkGyraFPhHpyhcexpMpCtLxIn == YwURbhkGyraFPhHpyhcexpMpCtLxIn ) YwURbhkGyraFPhHpyhcexpMpCtLxIn=755784629.036669913665455245863934690928; else YwURbhkGyraFPhHpyhcexpMpCtLxIn=1414516565.205336947565507727577063911208;if (YwURbhkGyraFPhHpyhcexpMpCtLxIn == YwURbhkGyraFPhHpyhcexpMpCtLxIn ) YwURbhkGyraFPhHpyhcexpMpCtLxIn=394642501.900656022941841953807445224343; else YwURbhkGyraFPhHpyhcexpMpCtLxIn=2094242017.674066505271574733207851166543;float nnyPhhnmZTZngLernIbyFCTeqLOvzy=648030678.704138824967062933138247212727f;if (nnyPhhnmZTZngLernIbyFCTeqLOvzy - nnyPhhnmZTZngLernIbyFCTeqLOvzy> 0.00000001 ) nnyPhhnmZTZngLernIbyFCTeqLOvzy=214177104.896917583659865680420492667792f; else nnyPhhnmZTZngLernIbyFCTeqLOvzy=924173346.721828126545159048363331840535f;if (nnyPhhnmZTZngLernIbyFCTeqLOvzy - nnyPhhnmZTZngLernIbyFCTeqLOvzy> 0.00000001 ) nnyPhhnmZTZngLernIbyFCTeqLOvzy=229063482.080103915867498544472054050008f; else nnyPhhnmZTZngLernIbyFCTeqLOvzy=1913860945.870167605714461794431616404726f;if (nnyPhhnmZTZngLernIbyFCTeqLOvzy - nnyPhhnmZTZngLernIbyFCTeqLOvzy> 0.00000001 ) nnyPhhnmZTZngLernIbyFCTeqLOvzy=525311142.391709834204076870336678826486f; else nnyPhhnmZTZngLernIbyFCTeqLOvzy=822933439.408043476206134775163449082499f;if (nnyPhhnmZTZngLernIbyFCTeqLOvzy - nnyPhhnmZTZngLernIbyFCTeqLOvzy> 0.00000001 ) nnyPhhnmZTZngLernIbyFCTeqLOvzy=629933602.445891604450424155415243306907f; else nnyPhhnmZTZngLernIbyFCTeqLOvzy=1792190680.118896145327272303603809431359f;if (nnyPhhnmZTZngLernIbyFCTeqLOvzy - nnyPhhnmZTZngLernIbyFCTeqLOvzy> 0.00000001 ) nnyPhhnmZTZngLernIbyFCTeqLOvzy=1548106277.571734374902120917100440432046f; else nnyPhhnmZTZngLernIbyFCTeqLOvzy=708472014.599685655814981189062535014230f;if (nnyPhhnmZTZngLernIbyFCTeqLOvzy - nnyPhhnmZTZngLernIbyFCTeqLOvzy> 0.00000001 ) nnyPhhnmZTZngLernIbyFCTeqLOvzy=633646832.343842454537239756936409506467f; else nnyPhhnmZTZngLernIbyFCTeqLOvzy=1786524407.152568124083553630647831504444f;long xthUmZklGcQLyaNrXerminNERMFDfV=1801310738;if (xthUmZklGcQLyaNrXerminNERMFDfV == xthUmZklGcQLyaNrXerminNERMFDfV- 0 ) xthUmZklGcQLyaNrXerminNERMFDfV=238201346; else xthUmZklGcQLyaNrXerminNERMFDfV=611448047;if (xthUmZklGcQLyaNrXerminNERMFDfV == xthUmZklGcQLyaNrXerminNERMFDfV- 1 ) xthUmZklGcQLyaNrXerminNERMFDfV=1040544326; else xthUmZklGcQLyaNrXerminNERMFDfV=235341061;if (xthUmZklGcQLyaNrXerminNERMFDfV == xthUmZklGcQLyaNrXerminNERMFDfV- 0 ) xthUmZklGcQLyaNrXerminNERMFDfV=347575343; else xthUmZklGcQLyaNrXerminNERMFDfV=191188753;if (xthUmZklGcQLyaNrXerminNERMFDfV == xthUmZklGcQLyaNrXerminNERMFDfV- 0 ) xthUmZklGcQLyaNrXerminNERMFDfV=1088056789; else xthUmZklGcQLyaNrXerminNERMFDfV=218232006;if (xthUmZklGcQLyaNrXerminNERMFDfV == xthUmZklGcQLyaNrXerminNERMFDfV- 1 ) xthUmZklGcQLyaNrXerminNERMFDfV=100532168; else xthUmZklGcQLyaNrXerminNERMFDfV=1225465327;if (xthUmZklGcQLyaNrXerminNERMFDfV == xthUmZklGcQLyaNrXerminNERMFDfV- 0 ) xthUmZklGcQLyaNrXerminNERMFDfV=1055505815; else xthUmZklGcQLyaNrXerminNERMFDfV=790289376;int FqAPwHMPwUbhUoLlPdIHWqanbygKdg=1980207020;if (FqAPwHMPwUbhUoLlPdIHWqanbygKdg == FqAPwHMPwUbhUoLlPdIHWqanbygKdg- 1 ) FqAPwHMPwUbhUoLlPdIHWqanbygKdg=1772923974; else FqAPwHMPwUbhUoLlPdIHWqanbygKdg=1067486475;if (FqAPwHMPwUbhUoLlPdIHWqanbygKdg == FqAPwHMPwUbhUoLlPdIHWqanbygKdg- 1 ) FqAPwHMPwUbhUoLlPdIHWqanbygKdg=1124571766; else FqAPwHMPwUbhUoLlPdIHWqanbygKdg=352887181;if (FqAPwHMPwUbhUoLlPdIHWqanbygKdg == FqAPwHMPwUbhUoLlPdIHWqanbygKdg- 1 ) FqAPwHMPwUbhUoLlPdIHWqanbygKdg=1875317338; else FqAPwHMPwUbhUoLlPdIHWqanbygKdg=2035453133;if (FqAPwHMPwUbhUoLlPdIHWqanbygKdg == FqAPwHMPwUbhUoLlPdIHWqanbygKdg- 0 ) FqAPwHMPwUbhUoLlPdIHWqanbygKdg=491731481; else FqAPwHMPwUbhUoLlPdIHWqanbygKdg=1290861180;if (FqAPwHMPwUbhUoLlPdIHWqanbygKdg == FqAPwHMPwUbhUoLlPdIHWqanbygKdg- 0 ) FqAPwHMPwUbhUoLlPdIHWqanbygKdg=1569419246; else FqAPwHMPwUbhUoLlPdIHWqanbygKdg=654594787;if (FqAPwHMPwUbhUoLlPdIHWqanbygKdg == FqAPwHMPwUbhUoLlPdIHWqanbygKdg- 1 ) FqAPwHMPwUbhUoLlPdIHWqanbygKdg=1709385419; else FqAPwHMPwUbhUoLlPdIHWqanbygKdg=166809744;double nTCzvpVuHbhFPnGhNKiLBRXglBrSTa=1184602876.128387284835990432552056401204;if (nTCzvpVuHbhFPnGhNKiLBRXglBrSTa == nTCzvpVuHbhFPnGhNKiLBRXglBrSTa ) nTCzvpVuHbhFPnGhNKiLBRXglBrSTa=998898556.937448858208466720815308717299; else nTCzvpVuHbhFPnGhNKiLBRXglBrSTa=740815410.718000865269436470178249074974;if (nTCzvpVuHbhFPnGhNKiLBRXglBrSTa == nTCzvpVuHbhFPnGhNKiLBRXglBrSTa ) nTCzvpVuHbhFPnGhNKiLBRXglBrSTa=1856040671.166060047797665697334822157013; else nTCzvpVuHbhFPnGhNKiLBRXglBrSTa=1859626361.799120448743459279448774145401;if (nTCzvpVuHbhFPnGhNKiLBRXglBrSTa == nTCzvpVuHbhFPnGhNKiLBRXglBrSTa ) nTCzvpVuHbhFPnGhNKiLBRXglBrSTa=1964899045.313400955536069148409867603518; else nTCzvpVuHbhFPnGhNKiLBRXglBrSTa=194050458.605744115932592381600701365691;if (nTCzvpVuHbhFPnGhNKiLBRXglBrSTa == nTCzvpVuHbhFPnGhNKiLBRXglBrSTa ) nTCzvpVuHbhFPnGhNKiLBRXglBrSTa=300455335.845703232614367847633201891459; else nTCzvpVuHbhFPnGhNKiLBRXglBrSTa=22988351.953410880025909755452916975401;if (nTCzvpVuHbhFPnGhNKiLBRXglBrSTa == nTCzvpVuHbhFPnGhNKiLBRXglBrSTa ) nTCzvpVuHbhFPnGhNKiLBRXglBrSTa=1983600204.389487205817772830225610714765; else nTCzvpVuHbhFPnGhNKiLBRXglBrSTa=10831476.648085763365075624414529677987;if (nTCzvpVuHbhFPnGhNKiLBRXglBrSTa == nTCzvpVuHbhFPnGhNKiLBRXglBrSTa ) nTCzvpVuHbhFPnGhNKiLBRXglBrSTa=1813328777.354037980818807074519045112041; else nTCzvpVuHbhFPnGhNKiLBRXglBrSTa=1447329462.899601215884571783271347932992;int sYsBNuIwdrexkQaclXWSwolEXWHlmw=1923465782;if (sYsBNuIwdrexkQaclXWSwolEXWHlmw == sYsBNuIwdrexkQaclXWSwolEXWHlmw- 1 ) sYsBNuIwdrexkQaclXWSwolEXWHlmw=35031432; else sYsBNuIwdrexkQaclXWSwolEXWHlmw=800923459;if (sYsBNuIwdrexkQaclXWSwolEXWHlmw == sYsBNuIwdrexkQaclXWSwolEXWHlmw- 0 ) sYsBNuIwdrexkQaclXWSwolEXWHlmw=13600433; else sYsBNuIwdrexkQaclXWSwolEXWHlmw=2012767879;if (sYsBNuIwdrexkQaclXWSwolEXWHlmw == sYsBNuIwdrexkQaclXWSwolEXWHlmw- 0 ) sYsBNuIwdrexkQaclXWSwolEXWHlmw=594093634; else sYsBNuIwdrexkQaclXWSwolEXWHlmw=62730828;if (sYsBNuIwdrexkQaclXWSwolEXWHlmw == sYsBNuIwdrexkQaclXWSwolEXWHlmw- 1 ) sYsBNuIwdrexkQaclXWSwolEXWHlmw=2073477047; else sYsBNuIwdrexkQaclXWSwolEXWHlmw=350231860;if (sYsBNuIwdrexkQaclXWSwolEXWHlmw == sYsBNuIwdrexkQaclXWSwolEXWHlmw- 0 ) sYsBNuIwdrexkQaclXWSwolEXWHlmw=119716566; else sYsBNuIwdrexkQaclXWSwolEXWHlmw=1897166793;if (sYsBNuIwdrexkQaclXWSwolEXWHlmw == sYsBNuIwdrexkQaclXWSwolEXWHlmw- 0 ) sYsBNuIwdrexkQaclXWSwolEXWHlmw=813405462; else sYsBNuIwdrexkQaclXWSwolEXWHlmw=1840464203;float QTXOzmDDnOAmdHLbDdkwaipNlYwMia=1555299044.175319769944396859810696097763f;if (QTXOzmDDnOAmdHLbDdkwaipNlYwMia - QTXOzmDDnOAmdHLbDdkwaipNlYwMia> 0.00000001 ) QTXOzmDDnOAmdHLbDdkwaipNlYwMia=1442098099.346354969519455613858160369933f; else QTXOzmDDnOAmdHLbDdkwaipNlYwMia=2018386586.211770915299141125149502539700f;if (QTXOzmDDnOAmdHLbDdkwaipNlYwMia - QTXOzmDDnOAmdHLbDdkwaipNlYwMia> 0.00000001 ) QTXOzmDDnOAmdHLbDdkwaipNlYwMia=1938874467.614630989681966002925490582024f; else QTXOzmDDnOAmdHLbDdkwaipNlYwMia=1996268340.131579561124757974664788622426f;if (QTXOzmDDnOAmdHLbDdkwaipNlYwMia - QTXOzmDDnOAmdHLbDdkwaipNlYwMia> 0.00000001 ) QTXOzmDDnOAmdHLbDdkwaipNlYwMia=2063567118.907555040755181184461846155305f; else QTXOzmDDnOAmdHLbDdkwaipNlYwMia=961297868.824633821294592060854816517151f;if (QTXOzmDDnOAmdHLbDdkwaipNlYwMia - QTXOzmDDnOAmdHLbDdkwaipNlYwMia> 0.00000001 ) QTXOzmDDnOAmdHLbDdkwaipNlYwMia=15307975.074503165837428745519566202823f; else QTXOzmDDnOAmdHLbDdkwaipNlYwMia=1578873516.687022100991503859987274352940f;if (QTXOzmDDnOAmdHLbDdkwaipNlYwMia - QTXOzmDDnOAmdHLbDdkwaipNlYwMia> 0.00000001 ) QTXOzmDDnOAmdHLbDdkwaipNlYwMia=767031140.234336151675663816346304752387f; else QTXOzmDDnOAmdHLbDdkwaipNlYwMia=1101583415.136077738780841271255760594178f;if (QTXOzmDDnOAmdHLbDdkwaipNlYwMia - QTXOzmDDnOAmdHLbDdkwaipNlYwMia> 0.00000001 ) QTXOzmDDnOAmdHLbDdkwaipNlYwMia=516770624.588465969628967657358123257768f; else QTXOzmDDnOAmdHLbDdkwaipNlYwMia=1864485862.476030359741264880395783046531f;double LCEMYJrOlPDpgElxctXgjThUhrSzyT=1729775020.217031908076117926908295676014;if (LCEMYJrOlPDpgElxctXgjThUhrSzyT == LCEMYJrOlPDpgElxctXgjThUhrSzyT ) LCEMYJrOlPDpgElxctXgjThUhrSzyT=1599487857.080355302578169769856527012247; else LCEMYJrOlPDpgElxctXgjThUhrSzyT=1933403284.500419687272444014701736966334;if (LCEMYJrOlPDpgElxctXgjThUhrSzyT == LCEMYJrOlPDpgElxctXgjThUhrSzyT ) LCEMYJrOlPDpgElxctXgjThUhrSzyT=131778312.427608886018269418311955687556; else LCEMYJrOlPDpgElxctXgjThUhrSzyT=127101483.436496695724989457855803467212;if (LCEMYJrOlPDpgElxctXgjThUhrSzyT == LCEMYJrOlPDpgElxctXgjThUhrSzyT ) LCEMYJrOlPDpgElxctXgjThUhrSzyT=679116338.143168398393479718734171173414; else LCEMYJrOlPDpgElxctXgjThUhrSzyT=2065566643.948379182117957344476968907310;if (LCEMYJrOlPDpgElxctXgjThUhrSzyT == LCEMYJrOlPDpgElxctXgjThUhrSzyT ) LCEMYJrOlPDpgElxctXgjThUhrSzyT=590509242.609555520173762723483004819057; else LCEMYJrOlPDpgElxctXgjThUhrSzyT=936167728.298262851293441420728593860574;if (LCEMYJrOlPDpgElxctXgjThUhrSzyT == LCEMYJrOlPDpgElxctXgjThUhrSzyT ) LCEMYJrOlPDpgElxctXgjThUhrSzyT=814822010.757398561008220066459943080640; else LCEMYJrOlPDpgElxctXgjThUhrSzyT=1505808811.238754615397073137481422591542;if (LCEMYJrOlPDpgElxctXgjThUhrSzyT == LCEMYJrOlPDpgElxctXgjThUhrSzyT ) LCEMYJrOlPDpgElxctXgjThUhrSzyT=1739909795.803187057655216398095426082637; else LCEMYJrOlPDpgElxctXgjThUhrSzyT=67732252.327460599718094631730455858721;double TNBMPppZfsztRqjSPKUaCzBkuhLeHK=1062353159.775310064371723900908021276541;if (TNBMPppZfsztRqjSPKUaCzBkuhLeHK == TNBMPppZfsztRqjSPKUaCzBkuhLeHK ) TNBMPppZfsztRqjSPKUaCzBkuhLeHK=720624412.079221228897509451307224085525; else TNBMPppZfsztRqjSPKUaCzBkuhLeHK=258029733.463430360289211253283524485635;if (TNBMPppZfsztRqjSPKUaCzBkuhLeHK == TNBMPppZfsztRqjSPKUaCzBkuhLeHK ) TNBMPppZfsztRqjSPKUaCzBkuhLeHK=5231363.780444527020123912393335204905; else TNBMPppZfsztRqjSPKUaCzBkuhLeHK=2044008332.584660997774637441268434346625;if (TNBMPppZfsztRqjSPKUaCzBkuhLeHK == TNBMPppZfsztRqjSPKUaCzBkuhLeHK ) TNBMPppZfsztRqjSPKUaCzBkuhLeHK=48253406.162355766312663970996385725042; else TNBMPppZfsztRqjSPKUaCzBkuhLeHK=1353805884.537271349289596787148684089763;if (TNBMPppZfsztRqjSPKUaCzBkuhLeHK == TNBMPppZfsztRqjSPKUaCzBkuhLeHK ) TNBMPppZfsztRqjSPKUaCzBkuhLeHK=2007382311.073598540273271790014900405439; else TNBMPppZfsztRqjSPKUaCzBkuhLeHK=1221217211.368632711825708542339086927319;if (TNBMPppZfsztRqjSPKUaCzBkuhLeHK == TNBMPppZfsztRqjSPKUaCzBkuhLeHK ) TNBMPppZfsztRqjSPKUaCzBkuhLeHK=785615484.312201564019029020306420049372; else TNBMPppZfsztRqjSPKUaCzBkuhLeHK=582210564.540251360709832116790812409326;if (TNBMPppZfsztRqjSPKUaCzBkuhLeHK == TNBMPppZfsztRqjSPKUaCzBkuhLeHK ) TNBMPppZfsztRqjSPKUaCzBkuhLeHK=1245736739.339268988986060860804479383986; else TNBMPppZfsztRqjSPKUaCzBkuhLeHK=1639993866.215458078523992485274052306108;double ABGgmHfIKnwKTqFZtjtBpmFLMfOGZL=1075314547.954198823128397798547428236392;if (ABGgmHfIKnwKTqFZtjtBpmFLMfOGZL == ABGgmHfIKnwKTqFZtjtBpmFLMfOGZL ) ABGgmHfIKnwKTqFZtjtBpmFLMfOGZL=373733237.412944658380102209805912464706; else ABGgmHfIKnwKTqFZtjtBpmFLMfOGZL=1231114089.363271656314036842604759861159;if (ABGgmHfIKnwKTqFZtjtBpmFLMfOGZL == ABGgmHfIKnwKTqFZtjtBpmFLMfOGZL ) ABGgmHfIKnwKTqFZtjtBpmFLMfOGZL=240976346.196202793276469298672321361552; else ABGgmHfIKnwKTqFZtjtBpmFLMfOGZL=843367344.050833983415943840067468798505;if (ABGgmHfIKnwKTqFZtjtBpmFLMfOGZL == ABGgmHfIKnwKTqFZtjtBpmFLMfOGZL ) ABGgmHfIKnwKTqFZtjtBpmFLMfOGZL=789468733.235132763731526848863259660225; else ABGgmHfIKnwKTqFZtjtBpmFLMfOGZL=1310589228.549939192229744694206084319667;if (ABGgmHfIKnwKTqFZtjtBpmFLMfOGZL == ABGgmHfIKnwKTqFZtjtBpmFLMfOGZL ) ABGgmHfIKnwKTqFZtjtBpmFLMfOGZL=876182706.254920379876765551608703967673; else ABGgmHfIKnwKTqFZtjtBpmFLMfOGZL=1524015103.153617592543553142086816065206;if (ABGgmHfIKnwKTqFZtjtBpmFLMfOGZL == ABGgmHfIKnwKTqFZtjtBpmFLMfOGZL ) ABGgmHfIKnwKTqFZtjtBpmFLMfOGZL=1427877344.381773326817917526193679734775; else ABGgmHfIKnwKTqFZtjtBpmFLMfOGZL=1002706739.339626399082628090954021482214;if (ABGgmHfIKnwKTqFZtjtBpmFLMfOGZL == ABGgmHfIKnwKTqFZtjtBpmFLMfOGZL ) ABGgmHfIKnwKTqFZtjtBpmFLMfOGZL=1181446330.965168930234812105175482231835; else ABGgmHfIKnwKTqFZtjtBpmFLMfOGZL=557758307.518522756126182132038724058403;int DBXjLdPuwTbwCarQzyEDypAmoxWXhV=159556361;if (DBXjLdPuwTbwCarQzyEDypAmoxWXhV == DBXjLdPuwTbwCarQzyEDypAmoxWXhV- 0 ) DBXjLdPuwTbwCarQzyEDypAmoxWXhV=1707882045; else DBXjLdPuwTbwCarQzyEDypAmoxWXhV=1601901112;if (DBXjLdPuwTbwCarQzyEDypAmoxWXhV == DBXjLdPuwTbwCarQzyEDypAmoxWXhV- 1 ) DBXjLdPuwTbwCarQzyEDypAmoxWXhV=1143861450; else DBXjLdPuwTbwCarQzyEDypAmoxWXhV=2111578270;if (DBXjLdPuwTbwCarQzyEDypAmoxWXhV == DBXjLdPuwTbwCarQzyEDypAmoxWXhV- 1 ) DBXjLdPuwTbwCarQzyEDypAmoxWXhV=1186654303; else DBXjLdPuwTbwCarQzyEDypAmoxWXhV=1626908871;if (DBXjLdPuwTbwCarQzyEDypAmoxWXhV == DBXjLdPuwTbwCarQzyEDypAmoxWXhV- 0 ) DBXjLdPuwTbwCarQzyEDypAmoxWXhV=1681521614; else DBXjLdPuwTbwCarQzyEDypAmoxWXhV=245681973;if (DBXjLdPuwTbwCarQzyEDypAmoxWXhV == DBXjLdPuwTbwCarQzyEDypAmoxWXhV- 0 ) DBXjLdPuwTbwCarQzyEDypAmoxWXhV=2073504620; else DBXjLdPuwTbwCarQzyEDypAmoxWXhV=1058044748;if (DBXjLdPuwTbwCarQzyEDypAmoxWXhV == DBXjLdPuwTbwCarQzyEDypAmoxWXhV- 1 ) DBXjLdPuwTbwCarQzyEDypAmoxWXhV=1488969646; else DBXjLdPuwTbwCarQzyEDypAmoxWXhV=96905774;double AyaaaXXYXUqAfwkvHVShYLyOgvhYgt=605830825.748793102156031911643246643228;if (AyaaaXXYXUqAfwkvHVShYLyOgvhYgt == AyaaaXXYXUqAfwkvHVShYLyOgvhYgt ) AyaaaXXYXUqAfwkvHVShYLyOgvhYgt=1377701307.469460649282626761580750661325; else AyaaaXXYXUqAfwkvHVShYLyOgvhYgt=664595248.198580867247357163099284871991;if (AyaaaXXYXUqAfwkvHVShYLyOgvhYgt == AyaaaXXYXUqAfwkvHVShYLyOgvhYgt ) AyaaaXXYXUqAfwkvHVShYLyOgvhYgt=1841482662.932519415600544341450834602471; else AyaaaXXYXUqAfwkvHVShYLyOgvhYgt=297014554.097731694985840984064055111992;if (AyaaaXXYXUqAfwkvHVShYLyOgvhYgt == AyaaaXXYXUqAfwkvHVShYLyOgvhYgt ) AyaaaXXYXUqAfwkvHVShYLyOgvhYgt=366498433.705471481486867619473852804362; else AyaaaXXYXUqAfwkvHVShYLyOgvhYgt=697817673.171328148060479930861262996477;if (AyaaaXXYXUqAfwkvHVShYLyOgvhYgt == AyaaaXXYXUqAfwkvHVShYLyOgvhYgt ) AyaaaXXYXUqAfwkvHVShYLyOgvhYgt=272884426.712931554057252358308551102943; else AyaaaXXYXUqAfwkvHVShYLyOgvhYgt=1557518831.388136854848691501175818936294;if (AyaaaXXYXUqAfwkvHVShYLyOgvhYgt == AyaaaXXYXUqAfwkvHVShYLyOgvhYgt ) AyaaaXXYXUqAfwkvHVShYLyOgvhYgt=1529330674.071278912650456097060188858021; else AyaaaXXYXUqAfwkvHVShYLyOgvhYgt=628699907.077097852460952656205023599443;if (AyaaaXXYXUqAfwkvHVShYLyOgvhYgt == AyaaaXXYXUqAfwkvHVShYLyOgvhYgt ) AyaaaXXYXUqAfwkvHVShYLyOgvhYgt=616130988.009279347034560033631283615561; else AyaaaXXYXUqAfwkvHVShYLyOgvhYgt=1193030314.942047179431937156357691292776;int prhFwJaLRudXHyAOadZgtKEUMXEpcX=838039761;if (prhFwJaLRudXHyAOadZgtKEUMXEpcX == prhFwJaLRudXHyAOadZgtKEUMXEpcX- 0 ) prhFwJaLRudXHyAOadZgtKEUMXEpcX=531465691; else prhFwJaLRudXHyAOadZgtKEUMXEpcX=1086180378;if (prhFwJaLRudXHyAOadZgtKEUMXEpcX == prhFwJaLRudXHyAOadZgtKEUMXEpcX- 0 ) prhFwJaLRudXHyAOadZgtKEUMXEpcX=2079595468; else prhFwJaLRudXHyAOadZgtKEUMXEpcX=91542739;if (prhFwJaLRudXHyAOadZgtKEUMXEpcX == prhFwJaLRudXHyAOadZgtKEUMXEpcX- 1 ) prhFwJaLRudXHyAOadZgtKEUMXEpcX=1212613382; else prhFwJaLRudXHyAOadZgtKEUMXEpcX=164012881;if (prhFwJaLRudXHyAOadZgtKEUMXEpcX == prhFwJaLRudXHyAOadZgtKEUMXEpcX- 1 ) prhFwJaLRudXHyAOadZgtKEUMXEpcX=2036143891; else prhFwJaLRudXHyAOadZgtKEUMXEpcX=894308013;if (prhFwJaLRudXHyAOadZgtKEUMXEpcX == prhFwJaLRudXHyAOadZgtKEUMXEpcX- 0 ) prhFwJaLRudXHyAOadZgtKEUMXEpcX=1697076122; else prhFwJaLRudXHyAOadZgtKEUMXEpcX=2142778020;if (prhFwJaLRudXHyAOadZgtKEUMXEpcX == prhFwJaLRudXHyAOadZgtKEUMXEpcX- 0 ) prhFwJaLRudXHyAOadZgtKEUMXEpcX=917346371; else prhFwJaLRudXHyAOadZgtKEUMXEpcX=1878907632;long TywyvSqNblDuBjUgchUKkiLUQVREVB=329878321;if (TywyvSqNblDuBjUgchUKkiLUQVREVB == TywyvSqNblDuBjUgchUKkiLUQVREVB- 0 ) TywyvSqNblDuBjUgchUKkiLUQVREVB=1759982862; else TywyvSqNblDuBjUgchUKkiLUQVREVB=575615505;if (TywyvSqNblDuBjUgchUKkiLUQVREVB == TywyvSqNblDuBjUgchUKkiLUQVREVB- 1 ) TywyvSqNblDuBjUgchUKkiLUQVREVB=1327540647; else TywyvSqNblDuBjUgchUKkiLUQVREVB=704276472;if (TywyvSqNblDuBjUgchUKkiLUQVREVB == TywyvSqNblDuBjUgchUKkiLUQVREVB- 0 ) TywyvSqNblDuBjUgchUKkiLUQVREVB=253576708; else TywyvSqNblDuBjUgchUKkiLUQVREVB=1901213966;if (TywyvSqNblDuBjUgchUKkiLUQVREVB == TywyvSqNblDuBjUgchUKkiLUQVREVB- 0 ) TywyvSqNblDuBjUgchUKkiLUQVREVB=1940541575; else TywyvSqNblDuBjUgchUKkiLUQVREVB=1010064372;if (TywyvSqNblDuBjUgchUKkiLUQVREVB == TywyvSqNblDuBjUgchUKkiLUQVREVB- 1 ) TywyvSqNblDuBjUgchUKkiLUQVREVB=1329016686; else TywyvSqNblDuBjUgchUKkiLUQVREVB=1733826266;if (TywyvSqNblDuBjUgchUKkiLUQVREVB == TywyvSqNblDuBjUgchUKkiLUQVREVB- 1 ) TywyvSqNblDuBjUgchUKkiLUQVREVB=582247596; else TywyvSqNblDuBjUgchUKkiLUQVREVB=557954396;long xHEOplJLJQuOvnxXZJacbIcdbLXmpG=623880616;if (xHEOplJLJQuOvnxXZJacbIcdbLXmpG == xHEOplJLJQuOvnxXZJacbIcdbLXmpG- 1 ) xHEOplJLJQuOvnxXZJacbIcdbLXmpG=1205699257; else xHEOplJLJQuOvnxXZJacbIcdbLXmpG=650929885;if (xHEOplJLJQuOvnxXZJacbIcdbLXmpG == xHEOplJLJQuOvnxXZJacbIcdbLXmpG- 1 ) xHEOplJLJQuOvnxXZJacbIcdbLXmpG=1712923730; else xHEOplJLJQuOvnxXZJacbIcdbLXmpG=1881133173;if (xHEOplJLJQuOvnxXZJacbIcdbLXmpG == xHEOplJLJQuOvnxXZJacbIcdbLXmpG- 1 ) xHEOplJLJQuOvnxXZJacbIcdbLXmpG=1165887202; else xHEOplJLJQuOvnxXZJacbIcdbLXmpG=1298664785;if (xHEOplJLJQuOvnxXZJacbIcdbLXmpG == xHEOplJLJQuOvnxXZJacbIcdbLXmpG- 1 ) xHEOplJLJQuOvnxXZJacbIcdbLXmpG=1540701090; else xHEOplJLJQuOvnxXZJacbIcdbLXmpG=1213688426;if (xHEOplJLJQuOvnxXZJacbIcdbLXmpG == xHEOplJLJQuOvnxXZJacbIcdbLXmpG- 0 ) xHEOplJLJQuOvnxXZJacbIcdbLXmpG=775935004; else xHEOplJLJQuOvnxXZJacbIcdbLXmpG=947174649;if (xHEOplJLJQuOvnxXZJacbIcdbLXmpG == xHEOplJLJQuOvnxXZJacbIcdbLXmpG- 0 ) xHEOplJLJQuOvnxXZJacbIcdbLXmpG=747422079; else xHEOplJLJQuOvnxXZJacbIcdbLXmpG=371204060;double tAltgCCzGHFqjTouRherUSWExGRAno=750053742.079177296130221611237449617095;if (tAltgCCzGHFqjTouRherUSWExGRAno == tAltgCCzGHFqjTouRherUSWExGRAno ) tAltgCCzGHFqjTouRherUSWExGRAno=2071998355.173030613193424022265567174167; else tAltgCCzGHFqjTouRherUSWExGRAno=286252667.031206739211925503560684172797;if (tAltgCCzGHFqjTouRherUSWExGRAno == tAltgCCzGHFqjTouRherUSWExGRAno ) tAltgCCzGHFqjTouRherUSWExGRAno=1580531099.766727232839315464377160745824; else tAltgCCzGHFqjTouRherUSWExGRAno=1744227696.515365531938283247971852061708;if (tAltgCCzGHFqjTouRherUSWExGRAno == tAltgCCzGHFqjTouRherUSWExGRAno ) tAltgCCzGHFqjTouRherUSWExGRAno=122083357.251843506104445908761905377330; else tAltgCCzGHFqjTouRherUSWExGRAno=1295552666.593378744174198851816304846768;if (tAltgCCzGHFqjTouRherUSWExGRAno == tAltgCCzGHFqjTouRherUSWExGRAno ) tAltgCCzGHFqjTouRherUSWExGRAno=584463053.628761128737469781863762420954; else tAltgCCzGHFqjTouRherUSWExGRAno=777735372.319766753773419004616590606635;if (tAltgCCzGHFqjTouRherUSWExGRAno == tAltgCCzGHFqjTouRherUSWExGRAno ) tAltgCCzGHFqjTouRherUSWExGRAno=1293122450.227442981413920045443644892926; else tAltgCCzGHFqjTouRherUSWExGRAno=1069531096.023865607722421468167239368466;if (tAltgCCzGHFqjTouRherUSWExGRAno == tAltgCCzGHFqjTouRherUSWExGRAno ) tAltgCCzGHFqjTouRherUSWExGRAno=910009700.106549126544873132353586090450; else tAltgCCzGHFqjTouRherUSWExGRAno=1626270763.735272799345272347734611463066;long igimFvixFUVSCInNIUhXPnMeEKRnEx=1208584822;if (igimFvixFUVSCInNIUhXPnMeEKRnEx == igimFvixFUVSCInNIUhXPnMeEKRnEx- 0 ) igimFvixFUVSCInNIUhXPnMeEKRnEx=2069455601; else igimFvixFUVSCInNIUhXPnMeEKRnEx=293465755;if (igimFvixFUVSCInNIUhXPnMeEKRnEx == igimFvixFUVSCInNIUhXPnMeEKRnEx- 0 ) igimFvixFUVSCInNIUhXPnMeEKRnEx=673208375; else igimFvixFUVSCInNIUhXPnMeEKRnEx=1318173344;if (igimFvixFUVSCInNIUhXPnMeEKRnEx == igimFvixFUVSCInNIUhXPnMeEKRnEx- 1 ) igimFvixFUVSCInNIUhXPnMeEKRnEx=1213836849; else igimFvixFUVSCInNIUhXPnMeEKRnEx=970535673;if (igimFvixFUVSCInNIUhXPnMeEKRnEx == igimFvixFUVSCInNIUhXPnMeEKRnEx- 1 ) igimFvixFUVSCInNIUhXPnMeEKRnEx=1311474766; else igimFvixFUVSCInNIUhXPnMeEKRnEx=461318077;if (igimFvixFUVSCInNIUhXPnMeEKRnEx == igimFvixFUVSCInNIUhXPnMeEKRnEx- 1 ) igimFvixFUVSCInNIUhXPnMeEKRnEx=1182398062; else igimFvixFUVSCInNIUhXPnMeEKRnEx=113852221;if (igimFvixFUVSCInNIUhXPnMeEKRnEx == igimFvixFUVSCInNIUhXPnMeEKRnEx- 0 ) igimFvixFUVSCInNIUhXPnMeEKRnEx=2075825115; else igimFvixFUVSCInNIUhXPnMeEKRnEx=1453885706;int nUPmcnuxiGSRxPpCYXTEpwpmlmDqiG=787556245;if (nUPmcnuxiGSRxPpCYXTEpwpmlmDqiG == nUPmcnuxiGSRxPpCYXTEpwpmlmDqiG- 1 ) nUPmcnuxiGSRxPpCYXTEpwpmlmDqiG=2145410664; else nUPmcnuxiGSRxPpCYXTEpwpmlmDqiG=1979677501;if (nUPmcnuxiGSRxPpCYXTEpwpmlmDqiG == nUPmcnuxiGSRxPpCYXTEpwpmlmDqiG- 0 ) nUPmcnuxiGSRxPpCYXTEpwpmlmDqiG=633439688; else nUPmcnuxiGSRxPpCYXTEpwpmlmDqiG=724525216;if (nUPmcnuxiGSRxPpCYXTEpwpmlmDqiG == nUPmcnuxiGSRxPpCYXTEpwpmlmDqiG- 0 ) nUPmcnuxiGSRxPpCYXTEpwpmlmDqiG=1055443848; else nUPmcnuxiGSRxPpCYXTEpwpmlmDqiG=476578370;if (nUPmcnuxiGSRxPpCYXTEpwpmlmDqiG == nUPmcnuxiGSRxPpCYXTEpwpmlmDqiG- 1 ) nUPmcnuxiGSRxPpCYXTEpwpmlmDqiG=501797259; else nUPmcnuxiGSRxPpCYXTEpwpmlmDqiG=2057630238;if (nUPmcnuxiGSRxPpCYXTEpwpmlmDqiG == nUPmcnuxiGSRxPpCYXTEpwpmlmDqiG- 1 ) nUPmcnuxiGSRxPpCYXTEpwpmlmDqiG=66466832; else nUPmcnuxiGSRxPpCYXTEpwpmlmDqiG=935188358;if (nUPmcnuxiGSRxPpCYXTEpwpmlmDqiG == nUPmcnuxiGSRxPpCYXTEpwpmlmDqiG- 0 ) nUPmcnuxiGSRxPpCYXTEpwpmlmDqiG=588010723; else nUPmcnuxiGSRxPpCYXTEpwpmlmDqiG=96356106;long mnZtjTHcNlsZkfvNyXhwgSrTGAKoIk=1445229156;if (mnZtjTHcNlsZkfvNyXhwgSrTGAKoIk == mnZtjTHcNlsZkfvNyXhwgSrTGAKoIk- 0 ) mnZtjTHcNlsZkfvNyXhwgSrTGAKoIk=1063644519; else mnZtjTHcNlsZkfvNyXhwgSrTGAKoIk=1686320904;if (mnZtjTHcNlsZkfvNyXhwgSrTGAKoIk == mnZtjTHcNlsZkfvNyXhwgSrTGAKoIk- 0 ) mnZtjTHcNlsZkfvNyXhwgSrTGAKoIk=449232106; else mnZtjTHcNlsZkfvNyXhwgSrTGAKoIk=1634489194;if (mnZtjTHcNlsZkfvNyXhwgSrTGAKoIk == mnZtjTHcNlsZkfvNyXhwgSrTGAKoIk- 1 ) mnZtjTHcNlsZkfvNyXhwgSrTGAKoIk=2015735713; else mnZtjTHcNlsZkfvNyXhwgSrTGAKoIk=417725905;if (mnZtjTHcNlsZkfvNyXhwgSrTGAKoIk == mnZtjTHcNlsZkfvNyXhwgSrTGAKoIk- 0 ) mnZtjTHcNlsZkfvNyXhwgSrTGAKoIk=1683700540; else mnZtjTHcNlsZkfvNyXhwgSrTGAKoIk=1101462682;if (mnZtjTHcNlsZkfvNyXhwgSrTGAKoIk == mnZtjTHcNlsZkfvNyXhwgSrTGAKoIk- 1 ) mnZtjTHcNlsZkfvNyXhwgSrTGAKoIk=1122261548; else mnZtjTHcNlsZkfvNyXhwgSrTGAKoIk=1119213022;if (mnZtjTHcNlsZkfvNyXhwgSrTGAKoIk == mnZtjTHcNlsZkfvNyXhwgSrTGAKoIk- 1 ) mnZtjTHcNlsZkfvNyXhwgSrTGAKoIk=561829634; else mnZtjTHcNlsZkfvNyXhwgSrTGAKoIk=8231136;double WOYtcNFDTPkcPVRFoTrgprVRlavqIa=1871268582.190771097928676002514762826348;if (WOYtcNFDTPkcPVRFoTrgprVRlavqIa == WOYtcNFDTPkcPVRFoTrgprVRlavqIa ) WOYtcNFDTPkcPVRFoTrgprVRlavqIa=1987421673.868924944155788063886251153996; else WOYtcNFDTPkcPVRFoTrgprVRlavqIa=122453455.586916075316957908496467271888;if (WOYtcNFDTPkcPVRFoTrgprVRlavqIa == WOYtcNFDTPkcPVRFoTrgprVRlavqIa ) WOYtcNFDTPkcPVRFoTrgprVRlavqIa=1628343746.115878087309857041435551123085; else WOYtcNFDTPkcPVRFoTrgprVRlavqIa=1897055078.530821118003129771033046376586;if (WOYtcNFDTPkcPVRFoTrgprVRlavqIa == WOYtcNFDTPkcPVRFoTrgprVRlavqIa ) WOYtcNFDTPkcPVRFoTrgprVRlavqIa=844749807.600388004793453244245206259456; else WOYtcNFDTPkcPVRFoTrgprVRlavqIa=1306488561.574340394187744179313244140513;if (WOYtcNFDTPkcPVRFoTrgprVRlavqIa == WOYtcNFDTPkcPVRFoTrgprVRlavqIa ) WOYtcNFDTPkcPVRFoTrgprVRlavqIa=153140974.076923912406127967229115434131; else WOYtcNFDTPkcPVRFoTrgprVRlavqIa=1592877231.098122292127802722829430506269;if (WOYtcNFDTPkcPVRFoTrgprVRlavqIa == WOYtcNFDTPkcPVRFoTrgprVRlavqIa ) WOYtcNFDTPkcPVRFoTrgprVRlavqIa=1939152143.098141800587620232372232376962; else WOYtcNFDTPkcPVRFoTrgprVRlavqIa=763061784.704324076102234282147499896798;if (WOYtcNFDTPkcPVRFoTrgprVRlavqIa == WOYtcNFDTPkcPVRFoTrgprVRlavqIa ) WOYtcNFDTPkcPVRFoTrgprVRlavqIa=1251706512.505898430636728100829583632564; else WOYtcNFDTPkcPVRFoTrgprVRlavqIa=278648491.839970845686251288270697912237;long tjhpxjpaoPacJSQNTJVYTQtTKYJwen=1267967735;if (tjhpxjpaoPacJSQNTJVYTQtTKYJwen == tjhpxjpaoPacJSQNTJVYTQtTKYJwen- 0 ) tjhpxjpaoPacJSQNTJVYTQtTKYJwen=629412727; else tjhpxjpaoPacJSQNTJVYTQtTKYJwen=630595959;if (tjhpxjpaoPacJSQNTJVYTQtTKYJwen == tjhpxjpaoPacJSQNTJVYTQtTKYJwen- 1 ) tjhpxjpaoPacJSQNTJVYTQtTKYJwen=390241187; else tjhpxjpaoPacJSQNTJVYTQtTKYJwen=1614954630;if (tjhpxjpaoPacJSQNTJVYTQtTKYJwen == tjhpxjpaoPacJSQNTJVYTQtTKYJwen- 0 ) tjhpxjpaoPacJSQNTJVYTQtTKYJwen=1120105409; else tjhpxjpaoPacJSQNTJVYTQtTKYJwen=1742587523;if (tjhpxjpaoPacJSQNTJVYTQtTKYJwen == tjhpxjpaoPacJSQNTJVYTQtTKYJwen- 1 ) tjhpxjpaoPacJSQNTJVYTQtTKYJwen=919304179; else tjhpxjpaoPacJSQNTJVYTQtTKYJwen=1727684759;if (tjhpxjpaoPacJSQNTJVYTQtTKYJwen == tjhpxjpaoPacJSQNTJVYTQtTKYJwen- 1 ) tjhpxjpaoPacJSQNTJVYTQtTKYJwen=295976961; else tjhpxjpaoPacJSQNTJVYTQtTKYJwen=1679460653;if (tjhpxjpaoPacJSQNTJVYTQtTKYJwen == tjhpxjpaoPacJSQNTJVYTQtTKYJwen- 0 ) tjhpxjpaoPacJSQNTJVYTQtTKYJwen=1749747315; else tjhpxjpaoPacJSQNTJVYTQtTKYJwen=2083714473;float KgBjhTtBzKCrPyKHwCdfgIRsiTjKhO=935889485.506293382114251813265233830856f;if (KgBjhTtBzKCrPyKHwCdfgIRsiTjKhO - KgBjhTtBzKCrPyKHwCdfgIRsiTjKhO> 0.00000001 ) KgBjhTtBzKCrPyKHwCdfgIRsiTjKhO=1311244873.830590973055163392283863837447f; else KgBjhTtBzKCrPyKHwCdfgIRsiTjKhO=864225788.453638586295022028165867308776f;if (KgBjhTtBzKCrPyKHwCdfgIRsiTjKhO - KgBjhTtBzKCrPyKHwCdfgIRsiTjKhO> 0.00000001 ) KgBjhTtBzKCrPyKHwCdfgIRsiTjKhO=256418080.364770254582619995282048369644f; else KgBjhTtBzKCrPyKHwCdfgIRsiTjKhO=266201630.181094235288468336942459542652f;if (KgBjhTtBzKCrPyKHwCdfgIRsiTjKhO - KgBjhTtBzKCrPyKHwCdfgIRsiTjKhO> 0.00000001 ) KgBjhTtBzKCrPyKHwCdfgIRsiTjKhO=433570228.161051163686001468004480273265f; else KgBjhTtBzKCrPyKHwCdfgIRsiTjKhO=1882351710.269423751837337646918466979198f;if (KgBjhTtBzKCrPyKHwCdfgIRsiTjKhO - KgBjhTtBzKCrPyKHwCdfgIRsiTjKhO> 0.00000001 ) KgBjhTtBzKCrPyKHwCdfgIRsiTjKhO=600479349.891842879872837726473845781355f; else KgBjhTtBzKCrPyKHwCdfgIRsiTjKhO=1904639605.576414650598715774926383606626f;if (KgBjhTtBzKCrPyKHwCdfgIRsiTjKhO - KgBjhTtBzKCrPyKHwCdfgIRsiTjKhO> 0.00000001 ) KgBjhTtBzKCrPyKHwCdfgIRsiTjKhO=1533179930.252541642418798623112744929481f; else KgBjhTtBzKCrPyKHwCdfgIRsiTjKhO=1003838522.914434637413261979509361562963f;if (KgBjhTtBzKCrPyKHwCdfgIRsiTjKhO - KgBjhTtBzKCrPyKHwCdfgIRsiTjKhO> 0.00000001 ) KgBjhTtBzKCrPyKHwCdfgIRsiTjKhO=1842820698.327261436509680959315108265398f; else KgBjhTtBzKCrPyKHwCdfgIRsiTjKhO=1252673929.388520711154301179171365923900f; }
 KgBjhTtBzKCrPyKHwCdfgIRsiTjKhOy::KgBjhTtBzKCrPyKHwCdfgIRsiTjKhOy()
 { this->eLcydMbRzgoJ("YjkapGqFSoRSqhmzevVObFlFBPJBOkeLcydMbRzgoJj", true, 1313503040, 1405052049, 718937732); }
#pragma optimize("", off)
 // <delete/>

