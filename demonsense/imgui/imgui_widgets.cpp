// dear imgui, v1.69 WIP
// (widgets code)

/*

Index of this file:

// [SECTION] Forward Declarations
// [SECTION] Widgets: Text, etc.
// [SECTION] Widgets: Main (Button, Image, Checkbox, RadioButton, ProgressBar, Bullet, etc.)
// [SECTION] Widgets: Low-level Layout helpers (Spacing, Dummy, NewLine, Separator, etc.)
// [SECTION] Widgets: ComboBox
// [SECTION] Data Type and Data Formatting Helpers
// [SECTION] Widgets: DragScalar, DragFloat, DragInt, etc.
// [SECTION] Widgets: SliderScalar, SliderFloat, SliderInt, etc.
// [SECTION] Widgets: InputScalar, InputFloat, InputInt, etc.
// [SECTION] Widgets: InputText, InputTextMultiline
// [SECTION] Widgets: ColorEdit, ColorPicker, ColorButton, etc.
// [SECTION] Widgets: TreeNode, CollapsingHeader, etc.
// [SECTION] Widgets: Selectable
// [SECTION] Widgets: ListBox
// [SECTION] Widgets: PlotLines, PlotHistogram
// [SECTION] Widgets: Value helpers
// [SECTION] Widgets: MenuItem, BeginMenu, EndMenu, etc.
// [SECTION] Widgets: BeginTabBar, EndTabBar, etc.
// [SECTION] Widgets: BeginTabItem, EndTabItem, etc.

*/

#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "imgui.h"
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "imgui_internal.h"
#include "../menu/ui.h"
#include <ctype.h>      // toupper, isprint
#if defined(_MSC_VER) && _MSC_VER <= 1500 // MSVC 2008 or earlier
#include <stddef.h>     // intptr_t
#else
#include <stdint.h>     // intptr_t
#endif

// Visual Studio warnings
#ifdef _MSC_VER
#pragma warning (disable: 4127) // condition expression is constant
#pragma warning (disable: 4996) // 'This function or variable may be unsafe': strcpy, strdup, sprintf, vsnprintf, sscanf, fopen
#endif

// Clang/GCC warnings with -Weverything
#ifdef __clang__
#pragma clang diagnostic ignored "-Wold-style-cast"         // warning : use of old-style cast                              // yes, they are more terse.
#pragma clang diagnostic ignored "-Wfloat-equal"            // warning : comparing floating point with == or != is unsafe   // storing and comparing against same constants (typically 0.0f) is ok.
#pragma clang diagnostic ignored "-Wformat-nonliteral"      // warning : format string is not a string literal              // passing non-literal to vsnformat(). yes, user passing incorrect format strings can crash the code.
#pragma clang diagnostic ignored "-Wsign-conversion"        // warning : implicit conversion changes signedness             //
#if __has_warning("-Wzero-as-null-pointer-constant")
#pragma clang diagnostic ignored "-Wzero-as-null-pointer-constant"  // warning : zero as null pointer constant              // some standard header variations use #define NULL 0
#endif
#if __has_warning("-Wdouble-promotion")
#pragma clang diagnostic ignored "-Wdouble-promotion"       // warning: implicit conversion from 'float' to 'double' when passing argument to function  // using printf() is a misery with this as C++ va_arg ellipsis changes float to double.
#endif
#elif defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wformat-nonliteral"        // warning: format not a string literal, format string not checked
#if __GNUC__ >= 8
#pragma GCC diagnostic ignored "-Wclass-memaccess"          // warning: 'memset/memcpy' clearing/writing an object of type 'xxxx' with no trivial copy-assignment; use assignment or value-initialization instead
#endif
#endif

//-------------------------------------------------------------------------
// Data
//-------------------------------------------------------------------------

// Those MIN/MAX values are not define because we need to point to them
static const ImS32  IM_S32_MIN = INT_MIN;    // (-2147483647 - 1), (0x80000000);
static const ImS32  IM_S32_MAX = INT_MAX;    // (2147483647), (0x7FFFFFFF)
static const ImU32  IM_U32_MIN = 0;
static const ImU32  IM_U32_MAX = UINT_MAX;   // (0xFFFFFFFF)
#ifdef LLONG_MIN
static const ImS64  IM_S64_MIN = LLONG_MIN;  // (-9223372036854775807ll - 1ll);
static const ImS64  IM_S64_MAX = LLONG_MAX;  // (9223372036854775807ll);
#else
static const ImS64  IM_S64_MIN = -9223372036854775807LL - 1;
static const ImS64  IM_S64_MAX = 9223372036854775807LL;
#endif
static const ImU64  IM_U64_MIN = 0;
#ifdef ULLONG_MAX
static const ImU64  IM_U64_MAX = ULLONG_MAX; // (0xFFFFFFFFFFFFFFFFull);
#else
static const ImU64  IM_U64_MAX = (2ULL * 9223372036854775807LL + 1);
#endif

//-------------------------------------------------------------------------
// [SECTION] Forward Declarations
//-------------------------------------------------------------------------

// Data Type helpers
static int       DataTypeFormatString(char* buf, int buf_size, ImGuiDataType data_type, const void* data_ptr, const char* format);
static void             DataTypeApplyOp(ImGuiDataType data_type, int op, void* output, void* arg_1, const void* arg_2);
static bool             DataTypeApplyOpFromText(const char* buf, const char* initial_value_buf, ImGuiDataType data_type, void* data_ptr, const char* format);

// For InputTextEx()
static bool             InputTextFilterCharacter(unsigned int* p_char, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data);
static int              InputTextCalcTextLenAndLineCount(const char* text_begin, const char** out_text_end);
static ImVec2           InputTextCalcTextSizeW(const ImWchar* text_begin, const ImWchar* text_end, const ImWchar** remaining = NULL, ImVec2* out_offset = NULL, bool stop_on_new_line = false);

//-------------------------------------------------------------------------
// [SECTION] Widgets: Text, etc.
//-------------------------------------------------------------------------
// - TextUnformatted()
// - Text()
// - TextV()
// - TextColored()
// - TextColoredV()
// - TextDisabled()
// - TextDisabledV()
// - TextWrapped()
// - TextWrappedV()
// - LabelText()
// - LabelTextV()
// - BulletText()
// - BulletTextV()
//-------------------------------------------------------------------------

void ImGui::TextEx(const char* text, const char* text_end, ImGuiTextFlags flags)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return;

    ImGuiContext& g = *GImGui;
    IM_ASSERT(text != NULL);
    const char* text_begin = text;
    if (text_end == NULL)
        text_end = text + strlen(text); // FIXME-OPT

    //23 - checkbox + space on checkbox <---> name checkbox
    const ImVec2 Global_Offset(((flags & ImGuiTextFlags_setSpace &&  !(window->Flags & ImGuiWindowFlags_NoSpaceAnyItems)) ? 17 + g.Style.ItemInnerSpacing.x : 0), 0);

    const ImVec2 text_pos(window->DC.CursorPos.x + Global_Offset.x, window->DC.CursorPos.y + window->DC.CurrentLineTextBaseOffset + 2.f);
    const float wrap_pos_x = window->DC.TextWrapPos;
    const bool wrap_enabled = (wrap_pos_x >= 0.0f);
    if (text_end - text > 2000 && !wrap_enabled)
    {
        // Long text!
        // Perform manual coarse clipping to optimize for long multi-line text
        // - From this point we will only compute the width of lines that are visible. Optimization only available when word-wrapping is disabled.
        // - We also don't vertically center the text within the line full height, which is unlikely to matter because we are likely the biggest and only item on the line.
        // - We use memchr(), pay attention that well optimized versions of those str/mem functions are much faster than a casually written loop.
        const char* line = text;
        const float line_height = GetTextLineHeight();
        ImVec2 text_size(0, 0);

        // Lines to skip (can't skip when logging text)
        ImVec2 pos = text_pos;
        if (!g.LogEnabled)
        {
            int lines_skippable = (int)((window->ClipRect.Min.y - text_pos.y) / line_height);
            if (lines_skippable > 0)
            {
                int lines_skipped = 0;
                while (line < text_end && lines_skipped < lines_skippable)
                {
                    const char* line_end = (const char*)memchr(line, '\n', text_end - line);
                    if (!line_end)
                        line_end = text_end;
                    if ((flags & ImGuiTextFlags_NoWidthForLargeClippedText) == 0)
                        text_size.x = ImMax(text_size.x, CalcTextSize(line, line_end).x);
                    line = line_end + 1;
                    lines_skipped++;
                }
                pos.y += lines_skipped * line_height;
            }
        }

        // Lines to render
        if (line < text_end)
        {
            ImRect line_rect(pos, pos + ImVec2(FLT_MAX, line_height));
            while (line < text_end)
            {
                if (IsClippedEx(line_rect, 0, false))
                    break;

                const char* line_end = (const char*)memchr(line, '\n', text_end - line);
                if (!line_end)
                    line_end = text_end;
                text_size.x = ImMax(text_size.x, CalcTextSize(line, line_end).x);
                RenderText(pos, line, line_end, false);
                line = line_end + 1;
                line_rect.Min.y += line_height;
                line_rect.Max.y += line_height;
                pos.y += line_height;
            }

            // Count remaining lines
            int lines_skipped = 0;
            while (line < text_end)
            {
                const char* line_end = (const char*)memchr(line, '\n', text_end - line);
                if (!line_end)
                    line_end = text_end;
                if ((flags & ImGuiTextFlags_NoWidthForLargeClippedText) == 0)
                    text_size.x = ImMax(text_size.x, CalcTextSize(line, line_end).x);
                line = line_end + 1;
                lines_skipped++;
            }
            pos.y += lines_skipped * line_height;
        }
        text_size.y = (pos - text_pos).y;

        ImRect bb(text_pos, text_pos + text_size);
        ItemSize(text_size, 0.0f);
        ItemAdd(bb, 0);
    }
    else
    {
        const float wrap_width = wrap_enabled ? CalcWrapWidthForPos(window->DC.CursorPos, wrap_pos_x) : 0.0f;
        const ImVec2 text_size = CalcTextSize(text_begin, text_end, false, wrap_width);

        ImRect bb(text_pos, text_pos + text_size);
        ItemSize(text_size, 0.0f);
        if (!ItemAdd(bb, 0))
            return;

        // Render (we don't hide text after ## in this end-user function)
        RenderTextWrapped(bb.Min, text_begin, text_end, wrap_width);
    }
}

void ImGui::TextUnformatted(const char* text, const char* text_end)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return;

    ImGuiContext& g = *GImGui;
    IM_ASSERT(text != NULL);
    const char* text_begin = text;
    if (text_end == NULL)
        text_end = text + strlen(text); // FIXME-OPT

    const ImVec2 text_pos(window->DC.CursorPos.x, window->DC.CursorPos.y + window->DC.CurrentLineTextBaseOffset);
    const float wrap_pos_x = window->DC.TextWrapPos;
    const bool wrap_enabled = wrap_pos_x >= 0.0f;
    if (text_end - text > 2000 && !wrap_enabled)
    {
        // Long text!
        // Perform manual coarse clipping to optimize for long multi-line text
        // - From this point we will only compute the width of lines that are visible. Optimization only available when word-wrapping is disabled.
        // - We also don't vertically center the text within the line full height, which is unlikely to matter because we are likely the biggest and only item on the line.
        // - We use memchr(), pay attention that well optimized versions of those str/mem functions are much faster than a casually written loop.
        const char* line = text;
        const float line_height = GetTextLineHeight();
        const ImRect clip_rect = window->ClipRect;
        ImVec2 text_size(0,0);

        if (text_pos.y <= clip_rect.Max.y)
        {
            ImVec2 pos = text_pos;

            // Lines to skip (can't skip when logging text)
            if (!g.LogEnabled)
            {
                int lines_skippable = (int)((clip_rect.Min.y - text_pos.y) / line_height);
                if (lines_skippable > 0)
                {
                    int lines_skipped = 0;
                    while (line < text_end && lines_skipped < lines_skippable)
                    {
                        const char* line_end = (const char*)memchr(line, '\n', text_end - line);
                        if (!line_end)
                            line_end = text_end;
                        line = line_end + 1;
                        lines_skipped++;
                    }
                    pos.y += lines_skipped * line_height;
                }
            }

            // Lines to render
            if (line < text_end)
            {
                ImRect line_rect(pos, pos + ImVec2(FLT_MAX, line_height));
                while (line < text_end)
                {
                    if (IsClippedEx(line_rect, 0, false))
                        break;

                    const char* line_end = (const char*)memchr(line, '\n', text_end - line);
                    if (!line_end)
                        line_end = text_end;
                    const ImVec2 line_size = CalcTextSize(line, line_end, false);
                    text_size.x = ImMax(text_size.x, line_size.x);
                    RenderText(pos, line, line_end, false);
                    line = line_end + 1;
                    line_rect.Min.y += line_height;
                    line_rect.Max.y += line_height;
                    pos.y += line_height;
                }

                // Count remaining lines
                int lines_skipped = 0;
                while (line < text_end)
                {
                    const char* line_end = (const char*)memchr(line, '\n', text_end - line);
                    if (!line_end)
                        line_end = text_end;
                    line = line_end + 1;
                    lines_skipped++;
                }
                pos.y += lines_skipped * line_height;
            }

            text_size.y += (pos - text_pos).y;
        }

        ImRect bb(text_pos, text_pos + text_size);
        ItemSize(text_size);
        ItemAdd(bb, 0);
    }
    else
    {
        const float wrap_width = wrap_enabled ? CalcWrapWidthForPos(window->DC.CursorPos, wrap_pos_x) : 0.0f;
        const ImVec2 text_size = CalcTextSize(text_begin, text_end, false, wrap_width);

        // Account of baseline offset
        ImRect bb(text_pos, text_pos + text_size);
        ItemSize(text_size);
        if (!ItemAdd(bb, 0))
            return;

        // Render (we don't hide text after ## in this end-user function)
        RenderTextWrapped(bb.Min, text_begin, text_end, wrap_width);
    }
}

void ImGui::TextS(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return;

    //flags |= ImGuiTextFlags_NoWidthForLargeClippedText;

    ImGuiContext& g = *GImGui;
    float bec = g.FontSize;
    const char* text_end = g.TempBuffer + ImFormatStringV(g.TempBuffer, IM_ARRAYSIZE(g.TempBuffer), fmt, args);
    TextEx(g.TempBuffer, text_end, ImGuiTextFlags_NoWidthForLargeClippedText | ImGuiTextFlags_setSpace);

    va_end(args);
}

void ImGui::TextSized(float size, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return;

    //flags |= ImGuiTextFlags_NoWidthForLargeClippedText;

    ImGuiContext& g = *GImGui;
    float bec = g.FontSize;
    g.FontSize = size;
    const char* text_end = g.TempBuffer + ImFormatStringV(g.TempBuffer, IM_ARRAYSIZE(g.TempBuffer), fmt, args);
    TextEx(g.TempBuffer, text_end, ImGuiTextFlags_NoWidthForLargeClippedText | ImGuiTextFlags_setSpace);

    g.FontSize = bec;

    va_end(args);
}

void ImGui::Text(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    TextV(fmt, args);
    va_end(args);
}

void ImGui::TextV(const char* fmt, va_list args)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return;

    ImGuiContext& g = *GImGui;
    const char* text_end = g.TempBuffer + ImFormatStringV(g.TempBuffer, IM_ARRAYSIZE(g.TempBuffer), fmt, args);
    TextUnformatted(g.TempBuffer, text_end);
}

void ImGui::TextColored(const ImVec4& col, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    TextColoredV(col, fmt, args);
    va_end(args);
}

void ImGui::TextColoredV(const ImVec4& col, const char* fmt, va_list args)
{
    PushStyleColor(ImGuiCol_Text, col);
    TextV(fmt, args);
    PopStyleColor();
}

void ImGui::TextDisabled(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    TextDisabledV(fmt, args);
    va_end(args);
}

void ImGui::TextDisabledV(const char* fmt, va_list args)
{
    PushStyleColor(ImGuiCol_Text, GImGui->Style.Colors[ImGuiCol_TextDisabled]);
    TextV(fmt, args);
    PopStyleColor();
}

void ImGui::TextWrapped(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    TextWrappedV(fmt, args);
    va_end(args);
}

void ImGui::TextWrappedV(const char* fmt, va_list args)
{
    bool need_backup = (GImGui->CurrentWindow->DC.TextWrapPos < 0.0f);  // Keep existing wrap position if one is already set
    if (need_backup)
        PushTextWrapPos(0.0f);
    TextV(fmt, args);
    if (need_backup)
        PopTextWrapPos();
}

void ImGui::LabelText(const char* label, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    LabelTextV(label, fmt, args);
    va_end(args);
}

// Add a label+text combo aligned to other label+value widgets
void ImGui::LabelTextV(const char* label, const char* fmt, va_list args)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const float w = CalcItemWidth();

    const ImVec2 label_size = CalcTextSize(label, NULL, true);
    const ImRect value_bb(window->DC.CursorPos, window->DC.CursorPos + ImVec2(w, label_size.y + style.FramePadding.y*2));
    const ImRect total_bb(window->DC.CursorPos, window->DC.CursorPos + ImVec2(w + (label_size.x > 0.0f ? style.ItemInnerSpacing.x : 0.0f), style.FramePadding.y*2) + label_size);
    ItemSize(total_bb, style.FramePadding.y);
    if (!ItemAdd(total_bb, 0))
        return;

    // Render
    const char* value_text_begin = &g.TempBuffer[0];
    const char* value_text_end = value_text_begin + ImFormatStringV(g.TempBuffer, IM_ARRAYSIZE(g.TempBuffer), fmt, args);
    RenderTextClipped(value_bb.Min, value_bb.Max, value_text_begin, value_text_end, NULL, ImVec2(0.0f,0.5f));
    if (label_size.x > 0.0f)
        RenderText(ImVec2(value_bb.Max.x + style.ItemInnerSpacing.x, value_bb.Min.y + style.FramePadding.y), label);
}

void ImGui::BulletText(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    BulletTextV(fmt, args);
    va_end(args);
}

// Text with a little bullet aligned to the typical tree node.
void ImGui::BulletTextV(const char* fmt, va_list args)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;

    const char* text_begin = g.TempBuffer;
    const char* text_end = text_begin + ImFormatStringV(g.TempBuffer, IM_ARRAYSIZE(g.TempBuffer), fmt, args);
    const ImVec2 label_size = CalcTextSize(text_begin, text_end, false);
    const float text_base_offset_y = ImMax(0.0f, window->DC.CurrentLineTextBaseOffset); // Latch before ItemSize changes it
    const float line_height = ImMax(ImMin(window->DC.CurrentLineSize.y, g.FontSize + g.Style.FramePadding.y*2), g.FontSize);
    const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + ImVec2(g.FontSize + (label_size.x > 0.0f ? (label_size.x + style.FramePadding.x*2) : 0.0f), ImMax(line_height, label_size.y)));  // Empty text doesn't add padding
    ItemSize(bb);
    if (!ItemAdd(bb, 0))
        return;

    // Render
    RenderBullet(bb.Min + ImVec2(style.FramePadding.x + g.FontSize*0.5f, line_height*0.5f));
    RenderText(bb.Min+ImVec2(g.FontSize + style.FramePadding.x*2, text_base_offset_y), text_begin, text_end, false);
}

//-------------------------------------------------------------------------
// [SECTION] Widgets: Main
//-------------------------------------------------------------------------
// - ButtonBehavior() [Internal]
// - Button()
// - SmallButton()
// - InvisibleButton()
// - ArrowButton()
// - CloseButton() [Internal]
// - CollapseButton() [Internal]
// - Scrollbar() [Internal]
// - Image()
// - ImageButton()
// - Checkbox()
// - CheckboxFlags()
// - RadioButton()
// - ProgressBar()
// - Bullet()
//-------------------------------------------------------------------------

bool ImGui::ButtonBehavior(const ImRect& bb, ImGuiID id, bool* out_hovered, bool* out_held, ImGuiButtonFlags flags)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = GetCurrentWindow();

    if (flags & ImGuiButtonFlags_Disabled)
    {
        if (out_hovered) *out_hovered = false;
        if (out_held) *out_held = false;
        if (g.ActiveId == id) ClearActiveID();
        return false;
    }

    // Default behavior requires click+release on same spot
    if ((flags & (ImGuiButtonFlags_PressedOnClickRelease | ImGuiButtonFlags_PressedOnClick | ImGuiButtonFlags_PressedOnRelease | ImGuiButtonFlags_PressedOnDoubleClick)) == 0)
        flags |= ImGuiButtonFlags_PressedOnClickRelease;

    ImGuiWindow* backup_hovered_window = g.HoveredWindow;
    if ((flags & ImGuiButtonFlags_FlattenChildren) && g.HoveredRootWindow == window)
        g.HoveredWindow = window;

#ifdef IMGUI_ENABLE_TEST_ENGINE
    if (id != 0 && window->DC.LastItemId != id)
        ImGuiTestEngineHook_ItemAdd(&g, bb, id);
#endif

    bool pressed = false;
    bool hovered = ItemHoverable(bb, id);

    // Drag source doesn't report as hovered
    if (hovered && g.DragDropActive && g.DragDropPayload.SourceId == id && !(g.DragDropSourceFlags & ImGuiDragDropFlags_SourceNoDisableHover))
        hovered = false;

    // Special mode for Drag and Drop where holding button pressed for a long time while dragging another item triggers the button
    if (g.DragDropActive && (flags & ImGuiButtonFlags_PressedOnDragDropHold) && !(g.DragDropSourceFlags & ImGuiDragDropFlags_SourceNoHoldToOpenOthers))
        if (IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem))
        {
            hovered = true;
            SetHoveredID(id);
            if (CalcTypematicPressedRepeatAmount(g.HoveredIdTimer + 0.0001f, g.HoveredIdTimer + 0.0001f - g.IO.DeltaTime, 0.01f, 0.70f)) // FIXME: Our formula for CalcTypematicPressedRepeatAmount() is fishy
            {
                pressed = true;
                FocusWindow(window);
            }
        }

    if ((flags & ImGuiButtonFlags_FlattenChildren) && g.HoveredRootWindow == window)
        g.HoveredWindow = backup_hovered_window;

    // AllowOverlap mode (rarely used) requires previous frame HoveredId to be null or to match. This allows using patterns where a later submitted widget overlaps a previous one.
    if (hovered && (flags & ImGuiButtonFlags_AllowItemOverlap) && (g.HoveredIdPreviousFrame != id && g.HoveredIdPreviousFrame != 0))
        hovered = false;

    // Mouse
    if (hovered)
    {
        if (!(flags & ImGuiButtonFlags_NoKeyModifiers) || (!g.IO.KeyCtrl && !g.IO.KeyShift && !g.IO.KeyAlt))
        {
            //                        | CLICKING        | HOLDING with ImGuiButtonFlags_Repeat
            // PressedOnClickRelease  |  <on release>*  |  <on repeat> <on repeat> .. (NOT on release)  <-- MOST COMMON! (*) only if both click/release were over bounds
            // PressedOnClick         |  <on click>     |  <on click> <on repeat> <on repeat> ..
            // PressedOnRelease       |  <on release>   |  <on repeat> <on repeat> .. (NOT on release)
            // PressedOnDoubleClick   |  <on dclick>    |  <on dclick> <on repeat> <on repeat> ..
            // FIXME-NAV: We don't honor those different behaviors.
            if ((flags & ImGuiButtonFlags_PressedOnClickRelease) && g.IO.MouseClicked[0])
            {
                SetActiveID(id, window);
                if (!(flags & ImGuiButtonFlags_NoNavFocus))
                    SetFocusID(id, window);
                FocusWindow(window);
            }
            if (((flags & ImGuiButtonFlags_PressedOnClick) && g.IO.MouseClicked[0]) || ((flags & ImGuiButtonFlags_PressedOnDoubleClick) && g.IO.MouseDoubleClicked[0]))
            {
                pressed = true;
                if (flags & ImGuiButtonFlags_NoHoldingActiveID)
                    ClearActiveID();
                else
                    SetActiveID(id, window); // Hold on ID
                FocusWindow(window);
            }
            if ((flags & ImGuiButtonFlags_PressedOnRelease) && g.IO.MouseReleased[0])
            {
                if (!((flags & ImGuiButtonFlags_Repeat) && g.IO.MouseDownDurationPrev[0] >= g.IO.KeyRepeatDelay))  // Repeat mode trumps <on release>
                    pressed = true;
                ClearActiveID();
            }

            // 'Repeat' mode acts when held regardless of _PressedOn flags (see table above).
            // Relies on repeat logic of IsMouseClicked() but we may as well do it ourselves if we end up exposing finer RepeatDelay/RepeatRate settings.
            if ((flags & ImGuiButtonFlags_Repeat) && g.ActiveId == id && g.IO.MouseDownDuration[0] > 0.0f && IsMouseClicked(0, true))
                pressed = true;
        }

        if (pressed)
            g.NavDisableHighlight = true;
    }

    // Gamepad/Keyboard navigation
    // We report navigated item as hovered but we don't set g.HoveredId to not interfere with mouse.
    if (g.NavId == id && !g.NavDisableHighlight && g.NavDisableMouseHover && (g.ActiveId == 0 || g.ActiveId == id || g.ActiveId == window->MoveId))
        hovered = true;

    if (g.NavActivateDownId == id)
    {
        bool nav_activated_by_code = (g.NavActivateId == id);
        bool nav_activated_by_inputs = IsNavInputPressed(ImGuiNavInput_Activate, (flags & ImGuiButtonFlags_Repeat) ? ImGuiInputReadMode_Repeat : ImGuiInputReadMode_Pressed);
        if (nav_activated_by_code || nav_activated_by_inputs)
            pressed = true;
        if (nav_activated_by_code || nav_activated_by_inputs || g.ActiveId == id)
        {
            // Set active id so it can be queried by user via IsItemActive(), equivalent of holding the mouse button.
            g.NavActivateId = id; // This is so SetActiveId assign a Nav source
            SetActiveID(id, window);
            if ((nav_activated_by_code || nav_activated_by_inputs) && !(flags & ImGuiButtonFlags_NoNavFocus))
                SetFocusID(id, window);
            g.ActiveIdAllowNavDirFlags = (1 << ImGuiDir_Left) | (1 << ImGuiDir_Right) | (1 << ImGuiDir_Up) | (1 << ImGuiDir_Down);
        }
    }

    bool held = false;
    if (g.ActiveId == id)
    {
        if (pressed)
            g.ActiveIdHasBeenPressed = true;
        if (g.ActiveIdSource == ImGuiInputSource_Mouse)
        {
            if (g.ActiveIdIsJustActivated)
                g.ActiveIdClickOffset = g.IO.MousePos - bb.Min;
            if (g.IO.MouseDown[0])
            {
                held = true;
            }
            else
            {
                if (hovered && (flags & ImGuiButtonFlags_PressedOnClickRelease))
                    if (!((flags & ImGuiButtonFlags_Repeat) && g.IO.MouseDownDurationPrev[0] >= g.IO.KeyRepeatDelay))  // Repeat mode trumps <on release>
                        if (!g.DragDropActive)
                            pressed = true;
                ClearActiveID();
            }
            if (!(flags & ImGuiButtonFlags_NoNavFocus))
                g.NavDisableHighlight = true;
        }
        else if (g.ActiveIdSource == ImGuiInputSource_Nav)
        {
            if (g.NavActivateDownId != id)
                ClearActiveID();
        }
    }

    if (out_hovered) *out_hovered = hovered;
    if (out_held) *out_held = held;

    return pressed;
}

bool LabelClick2(const char* concatoff, const char* concaton, const char* label, bool* v, const char* unique_id)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    // The concatoff/on thingies were for my weapon config system so if we're going to make that, we still need this aids.
    char Buf[64];
    _snprintf(Buf, 62, "%s%s", ((*v) ? concatoff : concaton), label);

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(unique_id);
    const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

    const ImRect check_bb(window->DC.CursorPos, window->DC.CursorPos + ImVec2(label_size.y + style.FramePadding.y * 2, label_size.y + style.FramePadding.y * 2));
    ImGui::ItemSize(check_bb, style.FramePadding.y);

    ImRect total_bb = check_bb;
    if (label_size.x > 0)
        ImGui::SameLine(0, style.ItemInnerSpacing.x);

    const ImRect text_bb(window->DC.CursorPos + ImVec2(0, style.FramePadding.y), window->DC.CursorPos + ImVec2(0, style.FramePadding.y) + label_size);
    if (label_size.x > 0)
    {
        ImGui::ItemSize(ImVec2(text_bb.GetWidth(), check_bb.GetHeight()), style.FramePadding.y);
        total_bb = ImRect(ImMin(check_bb.Min, text_bb.Min), ImMax(check_bb.Max, text_bb.Max));
    }

    if (!ImGui::ItemAdd(total_bb, id))
        return false;

    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(total_bb, id, &hovered, &held);
    if (pressed)
        *v = !(*v);

    if (label_size.x > 0.0f)
        ImGui::RenderText(check_bb.GetTL(), Buf);

    return pressed;
}

bool ImGui::LabelClick(const char* concatoff, const char* concaton, const char* label, bool* v, const char* unique_id)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    // The concatoff/on thingies were for my weapon config system so if we're going to make that, we still need this aids.
    char Buf[64];
    _snprintf(Buf, 62, "%s%s", ((*v) ? concatoff : concaton), label);

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(unique_id);
    const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

    const ImRect check_bb(window->DC.CursorPos, window->DC.CursorPos + ImVec2(label_size.y + style.FramePadding.y * 2, label_size.y + style.FramePadding.y * 2));
    ImGui::ItemSize(check_bb, style.FramePadding.y);

    ImRect total_bb = check_bb;
    if (label_size.x > 0)
        ImGui::SameLine(0, style.ItemInnerSpacing.x);

    const ImRect text_bb(window->DC.CursorPos + ImVec2(0, style.FramePadding.y), window->DC.CursorPos + ImVec2(0, style.FramePadding.y) + label_size);
    if (label_size.x > 0)
    {
        ImGui::ItemSize(ImVec2(text_bb.GetWidth(), check_bb.GetHeight()), style.FramePadding.y);
        total_bb = ImRect(ImMin(check_bb.Min, text_bb.Min), ImMax(check_bb.Max, text_bb.Max));
    }

    if (!ImGui::ItemAdd(total_bb, id))
        return false;

    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(total_bb, id, &hovered, &held);
    if (pressed)
        * v = !(*v);

    if (label_size.x > 0.0f)
        ImGui::RenderText(check_bb.GetTL(), Buf);

    return pressed;

}

ImGuiKeyBind* createkeybind(const char* name, std::string key)
{
    ImGuiContext& g = *GImGui;

    for (int i = 0; i < g.keybinds.size(); i++)
    {
        if (g.keybinds[i]->name == name)
        {
            g.keybinds[i]->key = key;
            return g.keybinds[i];
        }
    }

    ImGuiKeyBind* keybind_new = new ImGuiKeyBind{ name, key };

    g.keybinds.push_back(keybind_new);

    return keybind_new;
}

ImGuiKeyBind* findkeybind(const char* name)
{
    ImGuiContext& g = *GImGui;
    for (int i = 0; i < g.keybinds.size(); i++)
    {
        if (g.keybinds[i]->name == name)
        {
            return g.keybinds[i];
        }
    }
    return NULL;
}

bool ImGui::KeyBind(const char* label, KeyBind_t* k, ImGuiKeyBindFlags flags)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    ImGuiIO& io = g.IO;
    const ImGuiStyle& style = g.Style;

    std::string text_display = "";

    float offset_x = 17 + style.ItemInnerSpacing.x;
    const ImVec2 global_offset = ImVec2(offset_x, 0);

    const ImGuiID id = window->GetID(label);
    ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);
    ImVec2 key_size = ImGui::CalcTextSize(text_display.c_str(), NULL, true);

    ImGuiKeyBind* keybind = findkeybind(label);
    if (keybind != NULL)
    {
        ImVec2 keybind_label_size = ImGui::CalcTextSize(keybind->key.c_str(), NULL, true);
        text_display = keybind->key;
        key_size = ImGui::CalcTextSize(keybind->key.c_str());
    }

    const ImVec2 size_wind_max = window->Size;
    const ImVec2 pos_wind = window->Pos;
    float offset_scroll = 0;
    if (window->ScrollbarY)
        offset_scroll = style.ScrollbarSize;

    ImVec2 size = ImGui::CalcItemSize(ImVec2(0, 0), ImGui::CalcItemWidth(), label_size.y + style.FramePadding.y);
    const ImRect frame_bb(window->DC.CursorPos + global_offset, ImVec2(pos_wind.x + (size_wind_max.x - key_size.x - 23 - 4 - offset_scroll), window->DC.CursorPos.y + size.y));

    ImVec2 pos_max = ImVec2(pos_wind.x + size_wind_max.x - offset_scroll, pos_wind.y);
    ImVec2 pos_end = ImVec2(pos_wind.x + (window->Size.x - (offset_x * 2) + 9.f - offset_scroll), window->DC.CursorPos.y);
    ImRect hober_bb(ImVec2(pos_end.x - key_size.x - 8.f, window->DC.CursorPos.y), ImVec2(pos_end.x, window->DC.CursorPos.y + key_size.y + 4.f));

    const ImRect total_bb(window->DC.CursorPos + global_offset, !(flags & ImGuiKeyBindFlags_OnItem) ? ImVec2(hober_bb.Max.x, hober_bb.Max.y) : window->DC.CursorPos);

    ImGui::ItemSize(total_bb, style.FramePadding.y);
    if (!ImGui::ItemAdd(total_bb, id))
        return false;

    const bool focus_requested = ImGui::FocusableItemRegister(window, g.ActiveId == id);
    const bool focus_requested_by_code = focus_requested;
    const bool focus_requested_by_tab = focus_requested;

    bool hovered = ImGui::ItemHoverable(hober_bb, id);

    if (hovered) {
        ImGui::SetHoveredID(id);
    }

    const bool user_clicked = hovered && io.MouseClicked[0];

    if (focus_requested || user_clicked) {
        if (g.ActiveId != id) {
            // Start edition
            memset(io.MouseDown, 0, sizeof(io.MouseDown));
            memset(io.KeysDown, 0, sizeof(io.KeysDown));
            k->key_id = 0;
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
    int key = k->key_id;

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
            k->key_id = 0;
            ImGui::ClearActiveID();
        }
        else {
            k->key_id = key;
        }
    }

    if (k->key_id != 0 && g.ActiveId != id)
    {
        text_display = "";
        text_display.append(KeyNames[k->key_id]);
    }
    else if (g.ActiveId == id)
    {
        text_display = "...";
    }
    else
    {
        text_display = "-";
    }

    const ImRect clip_rect(frame_bb.Min.x, frame_bb.Min.y, frame_bb.Min.x + size.x, frame_bb.Min.y + size.y);
    ImVec2 render_pos = frame_bb.Min + style.FramePadding;

    key_size = CalcTextSize(text_display.c_str());

    ImGuiKeyBind* new_keybind = createkeybind(label, KeyNames[k->key_id]);

    new_keybind->key = KeyNames[k->key_id];

    if (keybind != NULL)
    {
        std::string text_new = "";
        if (k->key_id != 0 && g.ActiveId != id)
        {
            text_new.append(KeyNames[k->key_id]);
        }
        else if (g.ActiveId == id)
        {
            text_new.append("...");
        }
        else
        {
            text_new.append("-");
        }
        text_new.append("");

        keybind->key = text_new;
    }
    ImVec2 position_button = ImVec2(pos_end.x - key_size.x - 8.f, frame_bb.Min.y);
    ImVec2 size_button = CalcItemSize(ImVec2(0, 0), key_size.x + style.FramePadding.x * 2.0f, key_size.y + style.FramePadding.y * 2.0f);
    size_button.y -= 2;

    window->DrawList->AddRectFilled(position_button, position_button + size_button, GetColorU32(hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button), style.FrameRounding);

    ImRect bb(position_button, position_button + size_button);

    RenderTextClipped(bb.Min + style.FramePadding - ImVec2(0, 1.f), bb.Max - style.FramePadding, text_display.c_str(), NULL, & key_size, style.ButtonTextAlign, &bb);

    if (label_size.x > 0 && (!(flags & ImGuiKeyBindFlags_OnItem)))
        ImGui::RenderText(ImVec2(total_bb.Min.x, frame_bb.Min.y + 2.f), label);


    if (!(flags & ImGuiKeyBindFlags_NoSettings))
    {
        char* items[] = { "always", "hold [ on ]", "hold [ off ]", "switch" };

        std::string name_tooltip;
        name_tooltip += label;
        name_tooltip += "##Tooltip";

        std::string name_combo = "bind type";
        name_combo += "##TooltipCombo";
        name_combo += label;

        ImGui::PushStyleVar(ImGuiStyleVar_PopupBorderSize, 0);
        {
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0,0));
            {
                if (ImGui::ItemsToolTipBeginKeyBind(name_tooltip.c_str(), hovered))
                {
                    ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing_new, ImVec2(0, 0));
                    {
                        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
                        {
                            render_tabs2(items, k->bind_type, ImGui::GetWindowSize().x, 20.0f, true);
                        }
                        ImGui::PopStyleVar();
                    }
                    ImGui::PopStyleVar();
                    //ImGui::Combo(name_combo.c_str(), &k->bind_type, items, IM_ARRAYSIZE(items));
                    ImGui::ItemsToolTipEnd();
                }
            }
            ImGui::PopStyleVar();
        }
        ImGui::PopStyleVar();
    }

    return value_changed;
}

bool ImGui::ButtonEx(const char* label, const ImVec2& size_arg, ImGuiButtonFlags flags)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = CalcTextSize(label, NULL, true);

    float offset_scroll = 0;
    if (window->ScrollbarY)
        offset_scroll = style.ScrollbarSize;

    float offset_x = 17 + style.ItemInnerSpacing.x;
    ImVec2 offset = ImVec2(offset_x, 0.f);

    const ImVec2 Global_offset = ImVec2((!(flags & ImGuiButtonFlags_noSpace) ? offset_x : 0), 0);

    ImVec2 pos = window->DC.CursorPos + Global_offset;
    if ((flags & ImGuiButtonFlags_AlignTextBaseLine) && style.FramePadding.y < window->DC.CurrentLineTextBaseOffset) // Try to vertically align buttons that are smaller/have no padding so that text baseline matches (bit hacky, since it shouldn't be a flag)
        pos.y += window->DC.CurrentLineTextBaseOffset - style.FramePadding.y;
    ImVec2 size = CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

    size.y -= 2.f;

    /*
    ImVec2 pos = window->DC.CursorPos;
    if ((flags & ImGuiButtonFlags_AlignTextBaseLine) && style.FramePadding.y < window->DC.CurrentLineTextBaseOffset) // Try to vertically align buttons that are smaller/have no padding so that text baseline matches (bit hacky, since it shouldn't be a flag)
        pos.y += window->DC.CurrentLineTextBaseOffset - style.FramePadding.y;
    ImVec2 size = CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);
    */
    const ImRect bb(pos, pos + size);
    ItemSize(bb, style.FramePadding.y);
    if (!ItemAdd(bb, id))
        return false;

    if (window->DC.ItemFlags & ImGuiItemFlags_ButtonRepeat)
        flags |= ImGuiButtonFlags_Repeat;
    bool hovered, held;
    bool pressed = ButtonBehavior(bb, id, &hovered, &held, flags);
    if (pressed)
        MarkItemEdited(id);

    // Render
    const ImU32 col = GetColorU32((held && hovered) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
    RenderNavHighlight(bb, id);
    RenderFrame(bb.Min, bb.Max, col, true, style.FrameRounding);
    if (flags & ImGuiButtonFlags_noPadY)
    {
        RenderTextClipped(bb.Min - ImVec2(0.f, 1.f), bb.Max - ImVec2(0.f, 1.f), label, NULL, &label_size, style.ButtonTextAlign, &bb);
    }
    else
    {
        RenderTextClipped(bb.Min + style.FramePadding - ImVec2(0, 1.f), bb.Max - style.FramePadding, label, NULL, &label_size, style.ButtonTextAlign, &bb);
    }
    // Automatically close popups
    //if (pressed && !(flags & ImGuiButtonFlags_DontClosePopups) && (window->Flags & ImGuiWindowFlags_Popup))
    //    CloseCurrentPopup();

    IMGUI_TEST_ENGINE_ITEM_INFO(id, label, window->DC.LastItemStatusFlags);
    return pressed;
}

bool ImGui::ButtonEx(const char* label, const ImVec2& size_arg, const ImVec2& pos_arg, ImGuiButtonFlags flags)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = CalcTextSize(label, NULL, true);

    const ImVec2 Global_offset = ImVec2((!(flags & ImGuiButtonFlags_noSpace) ? 23 : 0), 0);

    ImVec2 pos = window->DC.CursorPos + Global_offset + pos_arg;
    if ((flags & ImGuiButtonFlags_AlignTextBaseLine) && style.FramePadding.y < window->DC.CurrentLineTextBaseOffset) // Try to vertically align buttons that are smaller/have no padding so that text baseline matches (bit hacky, since it shouldn't be a flag)
        pos.y += window->DC.CurrentLineTextBaseOffset - style.FramePadding.y;
    ImVec2 size = CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);


    /*
    ImVec2 pos = window->DC.CursorPos;
    if ((flags & ImGuiButtonFlags_AlignTextBaseLine) && style.FramePadding.y < window->DC.CurrentLineTextBaseOffset) // Try to vertically align buttons that are smaller/have no padding so that text baseline matches (bit hacky, since it shouldn't be a flag)
        pos.y += window->DC.CurrentLineTextBaseOffset - style.FramePadding.y;
    ImVec2 size = CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);
    */
    const ImRect bb(pos, pos + size);
    ItemSize(size, style.FramePadding.y);
    if (!ItemAdd(bb, id))
        return false;

    if (window->DC.ItemFlags & ImGuiItemFlags_ButtonRepeat)
        flags |= ImGuiButtonFlags_Repeat;
    bool hovered, held;
    bool pressed = ButtonBehavior(bb, id, &hovered, &held, flags);
    if (pressed)
        MarkItemEdited(id);

    // Render
    const ImU32 col = GetColorU32((held && hovered) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
    RenderNavHighlight(bb, id);
    RenderFrame(bb.Min, bb.Max, col, true, style.FrameRounding);
    RenderTextClipped(bb.Min + style.FramePadding, bb.Max - style.FramePadding, label, NULL, &label_size, style.ButtonTextAlign, &bb);

    // Automatically close popups
    //if (pressed && !(flags & ImGuiButtonFlags_DontClosePopups) && (window->Flags & ImGuiWindowFlags_Popup))
    //    CloseCurrentPopup();

    IMGUI_TEST_ENGINE_ITEM_INFO(id, label, window->DC.LastItemStatusFlags);
    return pressed;
}

bool ImGui::Button(const char* label, const ImVec2& size_arg)
{
    return ButtonEx(label, size_arg, ImGuiButtonFlags_noSpace);
}
bool ImGui::ButtonS(const char* label, const ImVec2& size_arg)
{
    return ButtonEx(label, size_arg, 0);
}

// Small buttons fits within text without additional vertical spacing.
bool ImGui::SmallButton(const char* label)
{
    ImGuiContext& g = *GImGui;
    float backup_padding_y = g.Style.FramePadding.y;
    g.Style.FramePadding.y = 0.0f;
    bool pressed = ButtonEx(label, ImVec2(0, 0), ImGuiButtonFlags_AlignTextBaseLine);
    g.Style.FramePadding.y = backup_padding_y;
    return pressed;
}

// Tip: use ImGui::PushID()/PopID() to push indices or pointers in the ID stack.
// Then you can keep 'str_id' empty or the same for all your buttons (instead of creating a string based on a non-string id)
bool ImGui::InvisibleButton(const char* str_id, const ImVec2& size_arg)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    // Cannot use zero-size for InvisibleButton(). Unlike Button() there is not way to fallback using the label size.
    IM_ASSERT(size_arg.x != 0.0f && size_arg.y != 0.0f);

    const ImGuiID id = window->GetID(str_id);
    ImVec2 size = CalcItemSize(size_arg, 0.0f, 0.0f);
    const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size);
    ItemSize(size);
    if (!ItemAdd(bb, id))
        return false;

    bool hovered, held;
    bool pressed = ButtonBehavior(bb, id, &hovered, &held);

    return pressed;
}

bool ImGui::ArrowButtonEx(const char* str_id, ImGuiDir dir, ImVec2 size, ImGuiButtonFlags flags)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiID id = window->GetID(str_id);
    const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size);
    const float default_size = GetFrameHeight();
    ItemSize(bb, (size.y >= default_size) ? g.Style.FramePadding.y : 0.0f);
    if (!ItemAdd(bb, id))
        return false;

    if (window->DC.ItemFlags & ImGuiItemFlags_ButtonRepeat)
        flags |= ImGuiButtonFlags_Repeat;

    bool hovered, held;
    bool pressed = ButtonBehavior(bb, id, &hovered, &held, flags);

    // Render
    const ImU32 col = GetColorU32((held && hovered) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
    RenderNavHighlight(bb, id);
    RenderFrame(bb.Min, bb.Max, col, true, g.Style.FrameRounding);
    RenderArrow(bb.Min + ImVec2(ImMax(0.0f, (size.x - g.FontSize) * 0.5f), ImMax(0.0f, (size.y - g.FontSize) * 0.5f)), dir);

    return pressed;
}

bool ImGui::ArrowButton(const char* str_id, ImGuiDir dir)
{
    float sz = GetFrameHeight();
    return ArrowButtonEx(str_id, dir, ImVec2(sz, sz), 0);
}

// Button to close a window
bool ImGui::CloseButton(ImGuiID id, const ImVec2& pos, float radius)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;

    // We intentionally allow interaction when clipped so that a mechanical Alt,Right,Validate sequence close a window.
    // (this isn't the regular behavior of buttons, but it doesn't affect the user much because navigation tends to keep items visible).
    const ImRect bb(pos - ImVec2(radius,radius), pos + ImVec2(radius,radius));
    bool is_clipped = !ItemAdd(bb, id);

    bool hovered, held;
    bool pressed = ButtonBehavior(bb, id, &hovered, &held);
    if (is_clipped)
        return pressed;

    // Render
    ImVec2 center = bb.GetCenter();
    if (hovered)
        window->DrawList->AddCircleFilled(center, ImMax(2.0f, radius), GetColorU32(held ? ImGuiCol_ButtonActive : ImGuiCol_ButtonHovered), 9);

    float cross_extent = (radius * 0.7071f) - 1.0f;
    ImU32 cross_col = GetColorU32(ImGuiCol_Text);
    center -= ImVec2(0.5f, 0.5f);
    window->DrawList->AddLine(center + ImVec2(+cross_extent,+cross_extent), center + ImVec2(-cross_extent,-cross_extent), cross_col, 1.0f);
    window->DrawList->AddLine(center + ImVec2(+cross_extent,-cross_extent), center + ImVec2(-cross_extent,+cross_extent), cross_col, 1.0f);

    return pressed;
}

bool ImGui::CollapseButton(ImGuiID id, const ImVec2& pos)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;

    ImRect bb(pos, pos + ImVec2(g.FontSize, g.FontSize) + g.Style.FramePadding * 2.0f);
    ItemAdd(bb, id);
    bool hovered, held;
    bool pressed = ButtonBehavior(bb, id, &hovered, &held, ImGuiButtonFlags_None);

    ImU32 col = GetColorU32((held && hovered) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
    if (hovered || held)
        window->DrawList->AddCircleFilled(bb.GetCenter() + ImVec2(0.0f, -0.5f), g.FontSize * 0.5f + 1.0f, col, 9);
    RenderArrow(bb.Min + g.Style.FramePadding, window->Collapsed ? ImGuiDir_Right : ImGuiDir_Down, 1.0f);

    // Switch to moving the window after mouse is moved beyond the initial drag threshold
    if (IsItemActive() && IsMouseDragging())
        StartMouseMovingWindow(window);

    return pressed;
}

ImGuiID ImGui::GetScrollbarID(ImGuiWindow* window, ImGuiAxis axis)
{
    return window->GetIDNoKeepAlive(axis == ImGuiAxis_X ? "#SCROLLX" : "#SCROLLY");
}

// Vertical/Horizontal scrollbar
// The entire piece of code below is rather confusing because:
// - We handle absolute seeking (when first clicking outside the grab) and relative manipulation (afterward or when clicking inside the grab)
// - We store values as normalized ratio and in a form that allows the window content to change while we are holding on a scrollbar
// - We handle both horizontal and vertical scrollbars, which makes the terminology not ideal.
void ImGui::Scrollbar(ImGuiAxis axis)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;

    const bool horizontal = (axis == ImGuiAxis_X);
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = GetScrollbarID(window, axis);
    KeepAliveID(id);

    // Render background
    bool other_scrollbar = (horizontal ? window->ScrollbarY : window->ScrollbarX);
    float other_scrollbar_size_w = other_scrollbar ? style.ScrollbarSize : 0.0f;
    const ImRect window_rect = window->Rect();
    const float border_size = 0.f;
    ImRect bb = horizontal
        ? ImRect(window->Pos.x + border_size, window_rect.Max.y - style.ScrollbarSize, window_rect.Max.x - other_scrollbar_size_w - border_size, window_rect.Max.y - border_size)
        : ImRect(window_rect.Max.x - style.ScrollbarSize, window->Pos.y + border_size + (window->Flags & ImGuiWindowFlags_ChildWindowTitle ? 17 : 0), window_rect.Max.x - border_size, window_rect.Max.y - other_scrollbar_size_w - border_size);
    if (!horizontal)
        bb.Min.y += window->TitleBarHeight() + ((window->Flags & ImGuiWindowFlags_MenuBar) ? window->MenuBarHeight() : 0.0f);

    const float bb_height = bb.GetHeight();
    if (bb.GetWidth() <= 0.0f || bb_height <= 0.0f)
        return;

    // When we are too small, start hiding and disabling the grab (this reduce visual noise on very small window and facilitate using the resize grab)
    float alpha = 1.0f;
    if ((axis == ImGuiAxis_Y) && bb_height < g.FontSize + g.Style.FramePadding.y * 2.0f)
    {
        alpha = ImSaturate((bb_height - g.FontSize) / (g.Style.FramePadding.y * 2.0f));
        if (alpha <= 0.0f)
            return;
    }
    const bool allow_interaction = (alpha >= 1.0f);

    int window_rounding_corners;
    if (horizontal)
        window_rounding_corners = ImDrawCornerFlags_BotLeft | (other_scrollbar ? 0 : ImDrawCornerFlags_BotRight);
    else
        window_rounding_corners = (((window->Flags & ImGuiWindowFlags_NoTitleBar) && !(window->Flags & ImGuiWindowFlags_MenuBar)) ? ImDrawCornerFlags_TopRight : 0) | (other_scrollbar ? 0 : ImDrawCornerFlags_BotRight);
    window->DrawList->AddRectFilled(bb.Min, bb.Max, GetColorU32(ImGuiCol_ScrollbarBg), window->WindowRounding, window_rounding_corners);
    bb.Expand(ImVec2(-ImClamp((float)(int)((bb.Max.x - bb.Min.x - 2.0f) * 0.5f), 0.0f, 3.0f), -ImClamp((float)(int)((bb.Max.y - bb.Min.y - 2.0f) * 0.5f), 0.0f, 3.0f)));

    // V denote the main, longer axis of the scrollbar (= height for a vertical scrollbar)
    float scrollbar_size_v = horizontal ? bb.GetWidth() : bb.GetHeight();
    float scroll_v = horizontal ? window->Scroll.x : window->Scroll.y;
    float win_size_avail_v = (horizontal ? window->SizeFull.x : window->SizeFull.y) - other_scrollbar_size_w;
    float win_size_contents_v = horizontal ? window->SizeContents.x : window->SizeContents.y;

    // Calculate the height of our grabbable box. It generally represent the amount visible (vs the total scrollable amount)
    // But we maintain a minimum size in pixel to allow for the user to still aim inside.
    IM_ASSERT(ImMax(win_size_contents_v, win_size_avail_v) > 0.0f); // Adding this assert to check if the ImMax(XXX,1.0f) is still needed. PLEASE CONTACT ME if this triggers.
    const float win_size_v = ImMax(ImMax(win_size_contents_v, win_size_avail_v), 1.0f);
    const float grab_h_pixels = ImClamp(scrollbar_size_v * (win_size_avail_v / win_size_v), style.GrabMinSize, scrollbar_size_v);
    const float grab_h_norm = grab_h_pixels / scrollbar_size_v;

    // Handle input right away. None of the code of Begin() is relying on scrolling position before calling Scrollbar().
    bool held = false;
    bool hovered = false;
    const bool previously_held = (g.ActiveId == id);
    ButtonBehavior(bb, id, &hovered, &held, ImGuiButtonFlags_NoNavFocus);

    float scroll_max = ImMax(1.0f, win_size_contents_v - win_size_avail_v);
    float scroll_ratio = ImSaturate(scroll_v / scroll_max);
    float grab_v_norm = scroll_ratio * (scrollbar_size_v - grab_h_pixels) / scrollbar_size_v;
    if (held && allow_interaction && grab_h_norm < 1.0f)
    {
        float scrollbar_pos_v = horizontal ? bb.Min.x : bb.Min.y;
        float mouse_pos_v = horizontal ? g.IO.MousePos.x : g.IO.MousePos.y;
        float* click_delta_to_grab_center_v = horizontal ? &g.ScrollbarClickDeltaToGrabCenter.x : &g.ScrollbarClickDeltaToGrabCenter.y;

        // Click position in scrollbar normalized space (0.0f->1.0f)
        const float clicked_v_norm = ImSaturate((mouse_pos_v - scrollbar_pos_v) / scrollbar_size_v);
        SetHoveredID(id);

        bool seek_absolute = false;
        if (!previously_held)
        {
            // On initial click calculate the distance between mouse and the center of the grab
            if (clicked_v_norm >= grab_v_norm && clicked_v_norm <= grab_v_norm + grab_h_norm)
            {
                *click_delta_to_grab_center_v = clicked_v_norm - grab_v_norm - grab_h_norm*0.5f;
            }
            else
            {
                seek_absolute = true;
                *click_delta_to_grab_center_v = 0.0f;
            }
        }

        // Apply scroll
        // It is ok to modify Scroll here because we are being called in Begin() after the calculation of SizeContents and before setting up our starting position
        const float scroll_v_norm = ImSaturate((clicked_v_norm - *click_delta_to_grab_center_v - grab_h_norm*0.5f) / (1.0f - grab_h_norm));
        scroll_v = (float)(int)(0.5f + scroll_v_norm * scroll_max);//(win_size_contents_v - win_size_v));
        if (horizontal)
            window->Scroll.x = scroll_v;
        else
            window->Scroll.y = scroll_v;

        // Update values for rendering
        scroll_ratio = ImSaturate(scroll_v / scroll_max);
        grab_v_norm = scroll_ratio * (scrollbar_size_v - grab_h_pixels) / scrollbar_size_v;

        // Update distance to grab now that we have seeked and saturated
        if (seek_absolute)
            *click_delta_to_grab_center_v = clicked_v_norm - grab_v_norm - grab_h_norm*0.5f;
    }

    // Render grab
    const ImU32 grab_col = GetColorU32(held ? ImGuiCol_ScrollbarGrabActive : hovered ? ImGuiCol_ScrollbarGrabHovered : ImGuiCol_ScrollbarGrab, alpha);
    ImRect grab_rect;
    if (horizontal)
        grab_rect = ImRect(ImLerp(bb.Min.x, bb.Max.x, grab_v_norm), bb.Min.y, ImMin(ImLerp(bb.Min.x, bb.Max.x, grab_v_norm) + grab_h_pixels, window_rect.Max.x), bb.Max.y);
    else
        grab_rect = ImRect(bb.Min.x, ImLerp(bb.Min.y, bb.Max.y, grab_v_norm), bb.Max.x, ImMin(ImLerp(bb.Min.y, bb.Max.y, grab_v_norm) + grab_h_pixels, window_rect.Max.y));
    window->DrawList->AddRectFilled(grab_rect.Min, grab_rect.Max, grab_col, style.ScrollbarRounding);
}

void ImGui::Image(ImTextureID user_texture_id, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1, const ImVec4& tint_col, const ImVec4& border_col)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return;

    ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size);
    if (border_col.w > 0.0f)
        bb.Max += ImVec2(2, 2);
    ItemSize(bb);
    if (!ItemAdd(bb, 0))
        return;

    if (border_col.w > 0.0f)
    {
        window->DrawList->AddRect(bb.Min, bb.Max, GetColorU32(border_col), 0.0f);
        window->DrawList->AddImage(user_texture_id, bb.Min + ImVec2(1, 1), bb.Max - ImVec2(1, 1), uv0, uv1, GetColorU32(tint_col));
    }
    else
    {
        window->DrawList->AddImage(user_texture_id, bb.Min, bb.Max, uv0, uv1, GetColorU32(tint_col));
    }
}

// frame_padding < 0: uses FramePadding from style (default)
// frame_padding = 0: no framing
// frame_padding > 0: set framing size
// The color used are the button colors.
bool ImGui::ImageButton(ImTextureID user_texture_id, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1, int frame_padding, const ImVec4& bg_col, const ImVec4& tint_col)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;

    // Default to using texture ID as ID. User can still push string/integer prefixes.
    // We could hash the size/uv to create a unique ID but that would prevent the user from animating UV.
    PushID((void*)(intptr_t)user_texture_id);
    const ImGuiID id = window->GetID("#image");
    PopID();

    const ImVec2 padding = (frame_padding >= 0) ? ImVec2((float)frame_padding, (float)frame_padding) : style.FramePadding;
    const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size + padding * 2);
    const ImRect image_bb(window->DC.CursorPos + padding, window->DC.CursorPos + padding + size);
    ItemSize(bb);
    if (!ItemAdd(bb, id))
        return false;

    bool hovered, held;
    bool pressed = ButtonBehavior(bb, id, &hovered, &held);

    // Render
    const ImU32 col = GetColorU32((held && hovered) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
    RenderNavHighlight(bb, id);
    RenderFrame(bb.Min, bb.Max, col, true, ImClamp((float)ImMin(padding.x, padding.y), 0.0f, style.FrameRounding));
    if (bg_col.w > 0.0f)
        window->DrawList->AddRectFilled(image_bb.Min, image_bb.Max, GetColorU32(bg_col));
    window->DrawList->AddImage(user_texture_id, image_bb.Min, image_bb.Max, uv0, uv1, GetColorU32(tint_col));

    return pressed;
}

bool ImGui::Checkbox(const char* label, bool* v)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = CalcTextSize(label, NULL, true);

    const float square_sz = GetFrameHeight();
    const ImVec2 pos = window->DC.CursorPos;
    const ImRect total_bb(pos, pos + ImVec2(square_sz + (label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f), label_size.y + 4.f/*+ style.FramePadding.y * 2.0f*/));
    ItemSize(total_bb, style.FramePadding.y);
    if (!ItemAdd(total_bb, id))
        return false;

    bool hovered, held;
    bool pressed = ButtonBehavior(total_bb, id, &hovered, &held);
    if (pressed)
    {
        *v = !(*v);
        MarkItemEdited(id);
    }

    const ImRect check_bb(pos, pos + ImVec2(square_sz, square_sz));
    RenderNavHighlight(total_bb, id);
    RenderFrame(check_bb.Min, check_bb.Max, GetColorU32((held && hovered) ? ImGuiCol_FrameBgActive : hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg), true, style.FrameRounding);
    ImU32 check_col = GetColorU32(ImGuiCol_CheckMark);
    if (*v)
    {
        //const float pad = ImMax(1.0f, IM_FLOOR(square_sz / 6.0f));
        //ImVec2 pad(ImMax(1.0f, IM_FLOOR(square_sz / 2.6f)), ImMax(1.0f, IM_FLOOR(square_sz / 2.6f)));
        const float pad = 2.f;
        window->DrawList->AddRectFilled(check_bb.Min + ImVec2(pad, pad), check_bb.Max - ImVec2(pad, pad), GetColorU32(ImGuiCol_SliderGrab), style.FrameRounding);
        //RenderCheckMark(check_bb.Min + ImVec2(pad, pad), check_col, square_sz - pad*2.0f);
    }

    if (g.LogEnabled)
        LogRenderedText(&total_bb.Min, *v ? "[x]" : "[ ]");
    if (label_size.x > 0.0f)
        RenderText(ImVec2(check_bb.Max.x + style.ItemInnerSpacing.x + 1.f, check_bb.Min.y + 2.f /*+ style.FramePadding.y*/), label);

    //window->DrawList->AddRectFilled(total_bb.Min, total_bb.Max, IM_COL32(255, 255, 255, 10));

    IMGUI_TEST_ENGINE_ITEM_INFO(id, label, window->DC.ItemFlags | ImGuiItemStatusFlags_Checkable | (*v ? ImGuiItemStatusFlags_Checked : 0));
    return pressed;
}

bool ImGui::CheckboxFlags(const char* label, unsigned int* flags, unsigned int flags_value)
{
    bool v = ((*flags & flags_value) == flags_value);
    bool pressed = Checkbox(label, &v);
    if (pressed)
    {
        if (v)
            *flags |= flags_value;
        else
            *flags &= ~flags_value;
    }

    return pressed;
}

bool ImGui::RadioButton(const char* label, bool active)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = CalcTextSize(label, NULL, true);

    const float square_sz = GetFrameHeight();
    const ImVec2 pos = window->DC.CursorPos;
    const ImRect check_bb(pos, pos + ImVec2(square_sz, square_sz));
    const ImRect total_bb(pos, pos + ImVec2(square_sz + (label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f), label_size.y + style.FramePadding.y * 2.0f));
    ItemSize(total_bb, style.FramePadding.y);
    if (!ItemAdd(total_bb, id))
        return false;

    ImVec2 center = check_bb.GetCenter();
    center.x = (float)(int)center.x + 0.5f;
    center.y = (float)(int)center.y + 0.5f;
    const float radius = (square_sz - 1.0f) * 0.5f;

    bool hovered, held;
    bool pressed = ButtonBehavior(total_bb, id, &hovered, &held);
    if (pressed)
        MarkItemEdited(id);

    RenderNavHighlight(total_bb, id);
    window->DrawList->AddCircleFilled(center, radius, GetColorU32((held && hovered) ? ImGuiCol_FrameBgActive : hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg), 16);
    if (active)
    {
        const float pad = ImMax(1.0f, (float)(int)(square_sz / 6.0f));
        window->DrawList->AddCircleFilled(center, radius - pad, GetColorU32(ImGuiCol_CheckMark), 16);
    }

    if (style.FrameBorderSize > 0.0f)
    {
        window->DrawList->AddCircle(center + ImVec2(1,1), radius, GetColorU32(ImGuiCol_BorderShadow), 16, style.FrameBorderSize);
        window->DrawList->AddCircle(center, radius, GetColorU32(ImGuiCol_Border), 16, style.FrameBorderSize);
    }

    if (g.LogEnabled)
        LogRenderedText(&total_bb.Min, active ? "(x)" : "( )");
    if (label_size.x > 0.0f)
        RenderText(ImVec2(check_bb.Max.x + style.ItemInnerSpacing.x, check_bb.Min.y + style.FramePadding.y), label);

    return pressed;
}

bool ImGui::RadioButton(const char* label, int* v, int v_button)
{
    const bool pressed = RadioButton(label, *v == v_button);
    if (pressed)
        *v = v_button;
    return pressed;
}

// size_arg (for each axis) < 0.0f: align to end, 0.0f: auto, > 0.0f: specified size
void ImGui::ProgressBar(float fraction, const ImVec2& size_arg, const char* overlay)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;

    ImVec2 pos = window->DC.CursorPos;
    ImRect bb(pos, pos + CalcItemSize(size_arg, CalcItemWidth(), g.FontSize + style.FramePadding.y*2.0f));
    ItemSize(bb, style.FramePadding.y);
    if (!ItemAdd(bb, 0))
        return;

    // Render
    fraction = ImSaturate(fraction);
    RenderFrame(bb.Min, bb.Max, GetColorU32(ImGuiCol_FrameBg), true, style.FrameRounding);
    bb.Expand(ImVec2(-style.FrameBorderSize, -style.FrameBorderSize));
    const ImVec2 fill_br = ImVec2(ImLerp(bb.Min.x, bb.Max.x, fraction), bb.Max.y);
    RenderRectFilledRangeH(window->DrawList, bb, GetColorU32(ImGuiCol_PlotHistogram), 0.0f, fraction, style.FrameRounding);

    // Default displaying the fraction as percentage string, but user can override it
    char overlay_buf[32];
    if (!overlay)
    {
        ImFormatString(overlay_buf, IM_ARRAYSIZE(overlay_buf), "%.0f%%", fraction*100+0.01f);
        overlay = overlay_buf;
    }

    ImVec2 overlay_size = CalcTextSize(overlay, NULL);
    if (overlay_size.x > 0.0f)
        RenderTextClipped(ImVec2(ImClamp(fill_br.x + style.ItemSpacing.x, bb.Min.x, bb.Max.x - overlay_size.x - style.ItemInnerSpacing.x), bb.Min.y), bb.Max, overlay, NULL, &overlay_size, ImVec2(0.0f,0.5f), &bb);
}

void ImGui::Bullet()
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const float line_height = ImMax(ImMin(window->DC.CurrentLineSize.y, g.FontSize + g.Style.FramePadding.y*2), g.FontSize);
    const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + ImVec2(g.FontSize, line_height));
    ItemSize(bb);
    if (!ItemAdd(bb, 0))
    {
        SameLine(0, style.FramePadding.x*2);
        return;
    }

    // Render and stay on same line
    RenderBullet(bb.Min + ImVec2(style.FramePadding.x + g.FontSize*0.5f, line_height*0.5f));
    SameLine(0, style.FramePadding.x*2);
}

//-------------------------------------------------------------------------
// [SECTION] Widgets: Low-level Layout helpers
//-------------------------------------------------------------------------
// - Spacing()
// - Dummy()
// - NewLine()
// - AlignTextToFramePadding()
// - Separator()
// - VerticalSeparator() [Internal]
// - SplitterBehavior() [Internal]
//-------------------------------------------------------------------------

void ImGui::Spacing()
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return;
    ItemSize(ImVec2(0,0));
}

void ImGui::Dummy(const ImVec2& size)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return;

    const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size);
    ItemSize(bb);
    ItemAdd(bb, 0);
}

void ImGui::NewLine()
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return;

    ImGuiContext& g = *GImGui;
    const ImGuiLayoutType backup_layout_type = window->DC.LayoutType;
    window->DC.LayoutType = ImGuiLayoutType_Vertical;
    if (window->DC.CurrentLineSize.y > 0.0f)     // In the event that we are on a line with items that is smaller that FontSize high, we will preserve its height.
        ItemSize(ImVec2(0,0));
    else
        ItemSize(ImVec2(0.0f, g.FontSize));
    window->DC.LayoutType = backup_layout_type;
}

void ImGui::AlignTextToFramePadding()
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return;

    ImGuiContext& g = *GImGui;
    window->DC.CurrentLineSize.y = ImMax(window->DC.CurrentLineSize.y, g.FontSize + g.Style.FramePadding.y * 2);
    window->DC.CurrentLineTextBaseOffset = ImMax(window->DC.CurrentLineTextBaseOffset, g.Style.FramePadding.y);
}

// Horizontal/vertical separating line
void ImGui::Separator(float w)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return;
    ImGuiContext& g = *GImGui;

    // Those flags should eventually be overridable by the user
    ImGuiSeparatorFlags flags = (window->DC.LayoutType == ImGuiLayoutType_Horizontal) ? ImGuiSeparatorFlags_Vertical : ImGuiSeparatorFlags_Horizontal;
    IM_ASSERT(ImIsPowerOfTwo((int)(flags & (ImGuiSeparatorFlags_Horizontal | ImGuiSeparatorFlags_Vertical))));   // Check that only 1 option is selected
    if (flags & ImGuiSeparatorFlags_Vertical)
    {
        VerticalSeparator();
        return;
    }

    // Horizontal Separator
    if (window->DC.ColumnsSet)
        PopClipRect();

    float x1 = window->Pos.x + (w == 0 ? 10.0f : 0.f);
    float x2 = window->Pos.x + (w >= 0 ? window->Size.x : w) - (w == 0 ? 10.0f : 0.f);
    if (!window->DC.GroupStack.empty())
        x1 += window->DC.Indent.x;

    const ImRect bb(ImVec2(x1 + 10.0f, window->DC.CursorPos.y), ImVec2(x2 - 10.0f, window->DC.CursorPos.y+1.0f));
    ItemSize(ImVec2(w, 0.0f)); // NB: we don't provide our width so that it doesn't get feed back into AutoFit, we don't provide height to not alter layout.
    if (!ItemAdd(bb, 0))
    {
        if (window->DC.ColumnsSet)
            PushColumnClipRect();
        return;
    }

    window->DrawList->AddLine(bb.Min, ImVec2(bb.Max.x,bb.Min.y), GetColorU32(ImGuiCol_Separator));

    if (g.LogEnabled)
        LogRenderedText(&bb.Min, "--------------------------------");

    if (window->DC.ColumnsSet)
    {
        PushColumnClipRect();
        window->DC.ColumnsSet->LineMinY = window->DC.CursorPos.y;
    }
}

void ImGui::VerticalSeparator()
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return;
    ImGuiContext& g = *GImGui;

    float y1 = window->DC.CursorPos.y;
    float y2 = window->DC.CursorPos.y + window->DC.CurrentLineSize.y;
    const ImRect bb(ImVec2(window->DC.CursorPos.x, y1), ImVec2(window->DC.CursorPos.x + 1.0f, y2));
    ItemSize(ImVec2(bb.GetWidth(), 0.0f));
    if (!ItemAdd(bb, 0))
        return;

    window->DrawList->AddLine(ImVec2(bb.Min.x, bb.Min.y), ImVec2(bb.Min.x, bb.Max.y), GetColorU32(ImGuiCol_Separator));
    if (g.LogEnabled)
        LogText(" |");
}

// Using 'hover_visibility_delay' allows us to hide the highlight and mouse cursor for a short time, which can be convenient to reduce visual noise.
bool ImGui::SplitterBehavior(const ImRect& bb, ImGuiID id, ImGuiAxis axis, float* size1, float* size2, float min_size1, float min_size2, float hover_extend, float hover_visibility_delay)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;

    const ImGuiItemFlags item_flags_backup = window->DC.ItemFlags;
    window->DC.ItemFlags |= ImGuiItemFlags_NoNav | ImGuiItemFlags_NoNavDefaultFocus;
    bool item_add = ItemAdd(bb, id);
    window->DC.ItemFlags = item_flags_backup;
    if (!item_add)
        return false;

    bool hovered, held;
    ImRect bb_interact = bb;
    bb_interact.Expand(axis == ImGuiAxis_Y ? ImVec2(0.0f, hover_extend) : ImVec2(hover_extend, 0.0f));
    ButtonBehavior(bb_interact, id, &hovered, &held, ImGuiButtonFlags_FlattenChildren | ImGuiButtonFlags_AllowItemOverlap);
    if (g.ActiveId != id)
        SetItemAllowOverlap();

    if (held || (g.HoveredId == id && g.HoveredIdPreviousFrame == id && g.HoveredIdTimer >= hover_visibility_delay))
        SetMouseCursor(axis == ImGuiAxis_Y ? ImGuiMouseCursor_ResizeNS : ImGuiMouseCursor_ResizeEW);

    ImRect bb_render = bb;
    if (held)
    {
        ImVec2 mouse_delta_2d = g.IO.MousePos - g.ActiveIdClickOffset - bb_interact.Min;
        float mouse_delta = (axis == ImGuiAxis_Y) ? mouse_delta_2d.y : mouse_delta_2d.x;

        // Minimum pane size
        float size_1_maximum_delta = ImMax(0.0f, *size1 - min_size1);
        float size_2_maximum_delta = ImMax(0.0f, *size2 - min_size2);
        if (mouse_delta < -size_1_maximum_delta)
            mouse_delta = -size_1_maximum_delta;
        if (mouse_delta > size_2_maximum_delta)
            mouse_delta = size_2_maximum_delta;

        // Apply resize
        if (mouse_delta != 0.0f)
        {
            if (mouse_delta < 0.0f)
                IM_ASSERT(*size1 + mouse_delta >= min_size1);
            if (mouse_delta > 0.0f)
                IM_ASSERT(*size2 - mouse_delta >= min_size2);
            *size1 += mouse_delta;
            *size2 -= mouse_delta;
            bb_render.Translate((axis == ImGuiAxis_X) ? ImVec2(mouse_delta, 0.0f) : ImVec2(0.0f, mouse_delta));
            MarkItemEdited(id);
        }
    }

    // Render
    const ImU32 col = GetColorU32(held ? ImGuiCol_SeparatorActive : (hovered && g.HoveredIdTimer >= hover_visibility_delay) ? ImGuiCol_SeparatorHovered : ImGuiCol_Separator);
    window->DrawList->AddRectFilled(bb_render.Min, bb_render.Max, col, g.Style.FrameRounding);

    return held;
}

//-------------------------------------------------------------------------
// [SECTION] Widgets: ComboBox
//-------------------------------------------------------------------------
// - BeginCombo()
// - EndCombo()
// - Combo()
//-------------------------------------------------------------------------

ImGuiComboBox* createcombo(const char* name, ImVec2 size)
{
    ImGuiContext& g = *GImGui;
    ImGuiComboBox* combo_new = new ImGuiComboBox{ name, size };
    for (int i = 0; i < g.comboboxs.size(); i++)
    {
        if (g.comboboxs[i]->name == name)
        {
            g.comboboxs[i]->size = size;
            return g.comboboxs[i];
        }
    }

    g.comboboxs.push_back(combo_new);

    return combo_new;
}

ImGuiComboBox* findcombo(const char* name)
{
    ImGuiContext& g = *GImGui;
    for (int i = 0; i < g.comboboxs.size(); i++)
    {
        if (g.comboboxs[i]->name == name)
        {
            return g.comboboxs[i];
        }
    }
    return NULL;
}

static float CalcMaxPopupHeightFromItemCount(int items_count)
{
    ImGuiContext& g = *GImGui;
    if (items_count <= 0)
        return FLT_MAX;
    return (g.FontSize + g.Style.ItemSpacing.y) * items_count - g.Style.ItemSpacing.y + (g.Style.WindowPadding.y * 2);
}

bool ImGui::MultiCombo(const char* name, const char** displayName, bool** data, int dataSize)
{
    ImGuiComboFlags flags;

    ImGuiContext& g = *GImGui;

    ImGuiWindow* window = GetCurrentWindow();

    ImGui::PushID(name);

    char previewText[1024] = { 0 };
    char buf[1024] = { 0 };
    char buf2[1024] = { 0 };
    int currentPreviewTextLen = 0;
    float multicomboWidth = 0;

    ImGuiComboBox* combo = findcombo(name);
    if(combo != NULL)
        multicomboWidth = combo->size.x - 20.f;

    for (int i = 0; i < dataSize; i++) {

        if (*data[i] == true) 
        {

            if (currentPreviewTextLen == 0)
                sprintf(buf, "%s", displayName[i]);
            else
                sprintf(buf, ", %s", displayName[i]);

            strcpy(buf2, previewText);
            sprintf(buf2 + currentPreviewTextLen, buf);
            ImVec2 textSize = ImGui::CalcTextSize(buf2);

            if (textSize.x > multicomboWidth) {

                sprintf(previewText + currentPreviewTextLen, " ...");
                currentPreviewTextLen += (int)strlen(" ...");
                break;
            }

            sprintf(previewText + currentPreviewTextLen, buf);
            currentPreviewTextLen += (int)strlen(buf);
        }
    }

    if (currentPreviewTextLen > 0)
        previewText[currentPreviewTextLen] = NULL;
    else
        sprintf(previewText, " -");

    bool isDataChanged = false;

    float w;
    ImVec2 becup = g.Style.WindowPadding;
    float bec = g.Style.FramePadding.x;

    bool opened = ImGui::BeginComboMulti(name, previewText, 0);
    g.Style.WindowPadding.x = becup.x;
    g.Style.WindowPadding.y = becup.y;
    g.Style.FramePadding.x = bec;

    if (opened)
    {

        for (int i = 0; i < dataSize; i++) {

            sprintf(buf, displayName[i]);

            if (ImGui::Selectable(buf, *data[i], ImGuiSelectableFlags_DontClosePopups)) {

                *data[i] = !*data[i];
                isDataChanged = true;
            }
        }

        ImGui::EndCombo();
    }

    ImGui::PopID();

    return isDataChanged;
}

bool ImGui::MultiCombo(const char* name, const char** displayName, bool* data, int dataSize)
{
    ImGuiComboFlags flags;

    ImGuiContext& g = *GImGui;

    ImGuiWindow* window = GetCurrentWindow();

    ImGui::PushID(name);

    char previewText[1024] = { 0 };
    char buf[1024] = { 0 };
    char buf2[1024] = { 0 };
    int currentPreviewTextLen = 0;
    float multicomboWidth = 0;

    ImGuiComboBox* combo = findcombo(name);
    if (combo != NULL)
        multicomboWidth = combo->size.x - 20.f;

    for (int i = 0; i < dataSize; i++) {

        if (data[i] == true)
        {

            if (currentPreviewTextLen == 0)
                sprintf(buf, "%s", displayName[i]);
            else
                sprintf(buf, ", %s", displayName[i]);

            strcpy(buf2, previewText);
            sprintf(buf2 + currentPreviewTextLen, buf);
            ImVec2 textSize = ImGui::CalcTextSize(buf2);

            if (textSize.x > multicomboWidth) {

                sprintf(previewText + currentPreviewTextLen, " ...");
                currentPreviewTextLen += (int)strlen(" ...");
                break;
            }

            sprintf(previewText + currentPreviewTextLen, buf);
            currentPreviewTextLen += (int)strlen(buf);
        }
    }

    if (currentPreviewTextLen > 0)
        previewText[currentPreviewTextLen] = NULL;
    else
        sprintf(previewText, " -");

    bool isDataChanged = false;

    float w;
    ImVec2 becup = g.Style.WindowPadding;
    float bec = g.Style.FramePadding.x;

    bool opened = ImGui::BeginComboMulti(name, previewText, 0);
    g.Style.WindowPadding.x = becup.x;
    g.Style.WindowPadding.y = becup.y;
    g.Style.FramePadding.x = bec;

    if (opened)
    {

        for (int i = 0; i < dataSize; i++) {

            sprintf(buf, displayName[i]);

            if (ImGui::Selectable(buf, data[i], ImGuiSelectableFlags_DontClosePopups)) {

                data[i] = !data[i];
                isDataChanged = true;
            }
        }

        ImGui::EndCombo();
    }

    ImGui::PopID();

    return isDataChanged;
}


bool ImGui::BeginCombo(const char* label, const char* preview_value, ImGuiComboFlags flags)
{
    // Always consume the SetNextWindowSizeConstraint() call in our early return paths
    ImGuiContext& g = *GImGui;
    ImGuiCond backup_next_window_size_constraint = g.NextWindowData.SizeConstraintCond;
    g.NextWindowData.SizeConstraintCond = 0;

    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    IM_ASSERT((flags & (ImGuiComboFlags_NoArrowButton | ImGuiComboFlags_NoPreview)) != (ImGuiComboFlags_NoArrowButton | ImGuiComboFlags_NoPreview)); // Can't use both flags together

    ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);

    const float arrow_size = (flags & ImGuiComboFlags_NoArrowButton) ? 0.0f : GetFrameHeight();
    const ImVec2 label_size = CalcTextSize(label, NULL, true);
    const float w = (flags & ImGuiComboFlags_NoPreview) ? arrow_size : CalcItemWidth();


    float offset_scroll = 0;
    if (window->ScrollbarY)
        offset_scroll = style.ScrollbarSize;

    float offset_x = 17 + style.ItemInnerSpacing.x;
    ImVec2 offset = ImVec2(offset_x, 0.f);

    ImVec2 pos_end = ImVec2(window->DC.CursorPos.x + (window->Size.x - (offset_x * 2) - 1.f - offset_scroll), window->DC.CursorPos.y);

    const ImVec2 Global_offset = ImVec2(offset_x, /*local*/label_size.y + 2);

    const ImRect frame_bb_old(window->DC.CursorPos + ImVec2(Global_offset.x, 0), window->DC.CursorPos + ImVec2(w, label_size.y + style.FramePadding.y*2.0f) + ImVec2(Global_offset.x, 0));

    const ImRect frame_bb(window->DC.CursorPos + Global_offset, pos_end + ImVec2(0, label_size.y + style.FramePadding.y*2.0f + label_size.y + 1.f));

    const ImRect frame_bb_add(window->DC.CursorPos + ImVec2(Global_offset.x, 0), pos_end + ImVec2(0, label_size.y + style.FramePadding.y*2.0f + label_size.y + 1.f));
    const ImRect total_bb(frame_bb_add.Min, frame_bb_add.Max);
    ItemSize(total_bb, style.FramePadding.y);
    if (!ItemAdd(total_bb, id, &frame_bb_add))
        return false;

    bool hovered, held;
    bool pressed = ButtonBehavior(frame_bb, id, &hovered, &held);
    bool popup_open = IsPopupOpen(id);

    const ImRect value_bb(frame_bb.Min, frame_bb.Max - ImVec2(arrow_size, 0.0f));
    const ImU32 frame_col = GetColorU32(hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg);
    RenderNavHighlight(frame_bb, id);
    if (!(flags & ImGuiComboFlags_NoPreview))
        window->DrawList->AddRectFilled(frame_bb.Min, frame_bb.Max, frame_col, style.FrameRounding);
    if (!(flags & ImGuiComboFlags_NoArrowButton))
    {
        //window->DrawList->AddRectFilled(ImVec2(frame_bb.Max.x - arrow_size, frame_bb.Min.y), frame_bb.Max, GetColorU32((popup_open || hovered) ? ImGuiCol_ButtonHovered : ImGuiCol_Button), style.FrameRounding, (w <= arrow_size) ? ImDrawCornerFlags_All : ImDrawCornerFlags_Right);
        RenderArrow(ImVec2(frame_bb.Max.x - arrow_size + style.FramePadding.y - 2.f, frame_bb.Min.y + style.FramePadding.y), popup_open ? ImGuiDir_Down : ImGuiDir_Left);
        //RenderArrow(window->DrawList, ImVec2(value_x2 + style.FramePadding.y, frame_bb.Min.y + style.FramePadding.y), text_col, popup_open ? ImGuiDir_Down : ImGuiDir_Left, 1.0f);
    }
    RenderFrameBorder(frame_bb.Min, frame_bb.Max, style.FrameRounding);
    if (preview_value != NULL && !(flags & ImGuiComboFlags_NoPreview))
        RenderTextClipped(frame_bb.Min + style.FramePadding, value_bb.Max, preview_value, NULL, NULL, ImVec2(0.0f,0.0f));
    if (label_size.x > 0)
        RenderText(ImVec2(frame_bb_old.Min.x, frame_bb_old.Min.y), label);

    if ((pressed || g.NavActivateId == id) && !popup_open)
    {
        if (window->DC.NavLayerCurrent == 0)
            window->NavLastIds[0] = id;
        OpenPopupEx(id);
        popup_open = true;
    }

    if (!popup_open)
        return false;

    if (backup_next_window_size_constraint)
    {
        g.NextWindowData.SizeConstraintCond = backup_next_window_size_constraint;
        g.NextWindowData.SizeConstraint_rect.Min.x = ImMax(g.NextWindowData.SizeConstraint_rect.Min.x, w);
    }
    else
    {
        if ((flags & ImGuiComboFlags_HeightMask_) == 0)
            flags |= ImGuiComboFlags_HeightRegular;
        IM_ASSERT(ImIsPowerOfTwo(flags & ImGuiComboFlags_HeightMask_));    // Only one
        int popup_max_height_in_items = -1;
        if (flags & ImGuiComboFlags_HeightRegular)     popup_max_height_in_items = 8;
        else if (flags & ImGuiComboFlags_HeightSmall)  popup_max_height_in_items = 4;
        else if (flags & ImGuiComboFlags_HeightLarge)  popup_max_height_in_items = 20;
        SetNextWindowSizeConstraints(ImVec2(w, 0.0f), ImVec2(FLT_MAX, CalcMaxPopupHeightFromItemCount(popup_max_height_in_items)));
    }

    char name[16];
    ImFormatString(name, IM_ARRAYSIZE(name), "##Combo_%02d", g.BeginPopupStack.Size); // Recycle windows based on depth

    style.WindowPadding.x = 0;
    style.WindowPadding.y = 0;
    style.FramePadding.x = 0;

    // Peak into expected window size so we can position it
    if (ImGuiWindow* popup_window = FindWindowByName(name))
        if (popup_window->WasActive)
        {
            ImVec2 size_expected = CalcWindowExpectedSize(popup_window);
            if (flags & ImGuiComboFlags_PopupAlignLeft)
                popup_window->AutoPosLastDirection = ImGuiDir_Left;
            ImRect r_outer = GetWindowAllowedExtentRect(popup_window);
            ImVec2 pos = FindBestWindowPosForPopupEx(frame_bb.GetBL(), size_expected, &popup_window->AutoPosLastDirection, r_outer, frame_bb, ImGuiPopupPositionPolicy_ComboBox);
            SetNextWindowPos(pos);
        }

    SetNextWindowSize(ImVec2(window->Size.x - (offset_x * 3) - 1.f - offset_scroll, 0.f));
    // Horizontally align ourselves with the framed text
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_Popup | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;
    PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(style.FramePadding.x, style.WindowPadding.y));

    bool ret = Begin(name, NULL, window_flags);
    PopStyleVar();
    if (!ret)
    {
        EndPopup();
        IM_ASSERT(0);   // This should never happen as we tested for IsPopupOpen() above
        return false;
    }
    return true;
}

bool ImGui::BeginComboMulti(const char* label, const char* preview_value, ImGuiComboFlags flags)
{
    // Always consume the SetNextWindowSizeConstraint() call in our early return paths
    ImGuiContext& g = *GImGui;
    ImGuiCond backup_next_window_size_constraint = g.NextWindowData.SizeConstraintCond;
    g.NextWindowData.SizeConstraintCond = 0;

    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    IM_ASSERT((flags & (ImGuiComboFlags_NoArrowButton | ImGuiComboFlags_NoPreview)) != (ImGuiComboFlags_NoArrowButton | ImGuiComboFlags_NoPreview)); // Can't use both flags together

    ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);

    const float arrow_size = (flags & ImGuiComboFlags_NoArrowButton) ? 0.0f : GetFrameHeight();
    const ImVec2 label_size = CalcTextSize(label, NULL, true);
    const float w = (flags & ImGuiComboFlags_NoPreview) ? arrow_size : CalcItemWidth();

    float offset_scroll = 0;
    if (window->ScrollbarY)
        offset_scroll = style.ScrollbarSize;
    float offset_x = 17 + style.ItemInnerSpacing.x;

    const ImVec2 Global_offset = ImVec2(17 + style.ItemInnerSpacing.x, /*local*/label_size.y + 2);

    ImVec2 pos_end = ImVec2(window->DC.CursorPos.x + (window->Size.x - (offset_x * 2) - 1.f - offset_scroll), window->DC.CursorPos.y + 1.f);

    const ImRect frame_bb_old(window->DC.CursorPos + ImVec2(Global_offset.x, 0), window->DC.CursorPos + ImVec2(w, label_size.y + style.FramePadding.y*2.0f) + ImVec2(Global_offset.x, 0));
    const ImRect frame_bb(window->DC.CursorPos + Global_offset, pos_end + ImVec2(0, label_size.y + style.FramePadding.y*2.0f) + ImVec2(0, label_size.y));
    const ImRect frame_bb_add(window->DC.CursorPos + ImVec2(Global_offset.x, 0), pos_end + ImVec2(0, label_size.y + style.FramePadding.y*2.0f) + ImVec2(0, label_size.y));
    const ImRect total_bb(frame_bb_add.Min, frame_bb_add.Max);
    ItemSize(total_bb, style.FramePadding.y);
    if (!ItemAdd(total_bb, id, &frame_bb_add))
        return false;

    float w_items = frame_bb_add.Max.x - frame_bb_add.Min.x - 18;

    createcombo(label, ImVec2(w_items, 0));

    bool hovered, held;
    bool pressed = ButtonBehavior(frame_bb, id, &hovered, &held);
    bool popup_open = IsPopupOpen(id);

    const ImRect value_bb(frame_bb.Min, frame_bb.Max - ImVec2(arrow_size, 0.0f));
    const ImU32 frame_col = GetColorU32(hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg);
    RenderNavHighlight(frame_bb, id);
    if (!(flags & ImGuiComboFlags_NoPreview))
        window->DrawList->AddRectFilled(frame_bb.Min, frame_bb.Max, frame_col, style.FrameRounding);
    if (!(flags & ImGuiComboFlags_NoArrowButton))
    {
        //window->DrawList->AddRectFilled(ImVec2(frame_bb.Max.x - arrow_size, frame_bb.Min.y), frame_bb.Max, GetColorU32((popup_open || hovered) ? ImGuiCol_ButtonHovered : ImGuiCol_Button), style.FrameRounding, (w <= arrow_size) ? ImDrawCornerFlags_All : ImDrawCornerFlags_Right);
        RenderArrow(ImVec2(frame_bb.Max.x - arrow_size + style.FramePadding.y - 2.f, frame_bb.Min.y + style.FramePadding.y), popup_open ? ImGuiDir_Down : ImGuiDir_Left);
    }
    RenderFrameBorder(frame_bb.Min, frame_bb.Max, style.FrameRounding);
    if (preview_value != NULL && !(flags & ImGuiComboFlags_NoPreview))
        RenderTextClipped(frame_bb.Min + style.FramePadding, value_bb.Max, preview_value, NULL, NULL, ImVec2(0.0f, 0.0f));
    if (label_size.x > 0)
        RenderText(ImVec2(frame_bb_old.Min.x + 2, frame_bb_old.Min.y), label);

    if ((pressed || g.NavActivateId == id) && !popup_open)
    {
        if (window->DC.NavLayerCurrent == 0)
            window->NavLastIds[0] = id;
        OpenPopupEx(id);
        popup_open = true;
    }

    if (!popup_open)
        return false;

    if (backup_next_window_size_constraint)
    {
        g.NextWindowData.SizeConstraintCond = backup_next_window_size_constraint;
        g.NextWindowData.SizeConstraint_rect.Min.x = ImMax(g.NextWindowData.SizeConstraint_rect.Min.x, w);
    }
    else
    {
        if ((flags & ImGuiComboFlags_HeightMask_) == 0)
            flags |= ImGuiComboFlags_HeightRegular;
        IM_ASSERT(ImIsPowerOfTwo(flags & ImGuiComboFlags_HeightMask_));    // Only one
        int popup_max_height_in_items = -1;
        if (flags & ImGuiComboFlags_HeightRegular)     popup_max_height_in_items = 8;
        else if (flags & ImGuiComboFlags_HeightSmall)  popup_max_height_in_items = 4;
        else if (flags & ImGuiComboFlags_HeightLarge)  popup_max_height_in_items = 20;
        SetNextWindowSizeConstraints(ImVec2(w, 0.0f), ImVec2(FLT_MAX, CalcMaxPopupHeightFromItemCount(popup_max_height_in_items)));
    }

    char name[16];
    ImFormatString(name, IM_ARRAYSIZE(name), "##Combo_%02d", g.BeginPopupStack.Size); // Recycle windows based on depth

    style.WindowPadding.x = 0;
    style.WindowPadding.y = 0;
    style.FramePadding.x = 0;

    // Peak into expected window size so we can position it
    if (ImGuiWindow* popup_window = FindWindowByName(name))
        if (popup_window->WasActive)
        {
            ImVec2 size_expected = CalcWindowExpectedSize(popup_window);
            if (flags & ImGuiComboFlags_PopupAlignLeft)
                popup_window->AutoPosLastDirection = ImGuiDir_Left;
            ImRect r_outer = GetWindowAllowedExtentRect(popup_window);
            ImVec2 pos = FindBestWindowPosForPopupEx(frame_bb.GetBL(), size_expected, &popup_window->AutoPosLastDirection, r_outer, frame_bb, ImGuiPopupPositionPolicy_ComboBox);
            SetNextWindowPos(pos);
        }

    SetNextWindowSize(ImVec2(window->Size.x - (offset_x * 3) - 1.f - offset_scroll, 0.f));
    // Horizontally align ourselves with the framed text
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_Popup | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;
    PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(style.FramePadding.x, style.WindowPadding.y));
    bool ret = Begin(name, NULL, window_flags);
    PopStyleVar();
    if (!ret)
    {
        EndPopup();
        IM_ASSERT(0);   // This should never happen as we tested for IsPopupOpen() above
        return false;
    }
    return true;
}

void ImGui::EndCombo()
{
    EndPopup();
}

// Getter for the old Combo() API: const char*[]
static bool Items_ArrayGetter(void* data, int idx, const char** out_text)
{
    const char* const* items = (const char* const*)data;
    if (out_text)
        *out_text = items[idx];
    return true;
}

// Getter for the old Combo() API: "item1\0item2\0item3\0"
static bool Items_SingleStringGetter(void* data, int idx, const char** out_text)
{
    // FIXME-OPT: we could pre-compute the indices to fasten this. But only 1 active combo means the waste is limited.
    const char* items_separated_by_zeros = (const char*)data;
    int items_count = 0;
    const char* p = items_separated_by_zeros;
    while (*p)
    {
        if (idx == items_count)
            break;
        p += strlen(p) + 1;
        items_count++;
    }
    if (!*p)
        return false;
    if (out_text)
        *out_text = p;
    return true;
}

// Old API, prefer using BeginCombo() nowadays if you can.
bool ImGui::Combo(const char* label, int* current_item, bool (*items_getter)(void*, int, const char**), void* data, int items_count, int popup_max_height_in_items)
{
    ImGuiContext& g = *GImGui;

    // Call the getter to obtain the preview string which is a parameter to BeginCombo()
    const char* preview_value = NULL;
    if (*current_item >= 0 && *current_item < items_count)
        items_getter(data, *current_item, &preview_value);

    // The old Combo() API exposed "popup_max_height_in_items". The new more general BeginCombo() API doesn't have/need it, but we emulate it here.
    if (popup_max_height_in_items != -1 && !g.NextWindowData.SizeConstraintCond)
        SetNextWindowSizeConstraints(ImVec2(0,0), ImVec2(FLT_MAX, CalcMaxPopupHeightFromItemCount(popup_max_height_in_items)));

    ImGuiStyle& style = g.Style;

    ImVec2 becup = style.WindowPadding;
    float bec = style.FramePadding.x;

    if (!BeginCombo(label, preview_value, ImGuiComboFlags_None))
        return false;

    style.WindowPadding.x = becup.x;
    style.WindowPadding.y = becup.y;
    style.FramePadding.x = bec;

    // Display items
    // FIXME-OPT: Use clipper (but we need to disable it on the appearing frame to make sure our call to SetItemDefaultFocus() is processed)
    bool value_changed = false;
    for (int i = 0; i < items_count; i++)
    {
        PushID((void*)(intptr_t)i);
        const bool item_selected = (i == *current_item);
        const char* item_text;
        if (!items_getter(data, i, &item_text))
            item_text = "*Unknown item*";
        if (Selectable(item_text, item_selected))
        {
            value_changed = true;
            *current_item = i;
        }
        if (item_selected)
            SetItemDefaultFocus();
        PopID();
    }

    EndCombo();
    return value_changed;
}

// Combo box helper allowing to pass an array of strings.
bool ImGui::Combo(const char* label, int* current_item, const char* const items[], int items_count, int height_in_items)
{
    const bool value_changed = Combo(label, current_item, Items_ArrayGetter, (void*)items, items_count, height_in_items);
    return value_changed;
}

// Combo box helper allowing to pass all items in a single string literal holding multiple zero-terminated items "item1\0item2\0"
bool ImGui::Combo(const char* label, int* current_item, const char* items_separated_by_zeros, int height_in_items)
{
    int items_count = 0;
    const char* p = items_separated_by_zeros;       // FIXME-OPT: Avoid computing this, or at least only when combo is open
    while (*p)
    {
        p += strlen(p) + 1;
        items_count++;
    }
    bool value_changed = Combo(label, current_item, Items_SingleStringGetter, (void*)items_separated_by_zeros, items_count, height_in_items);
    return value_changed;
}

//-------------------------------------------------------------------------
// [SECTION] Data Type and Data Formatting Helpers [Internal]
//-------------------------------------------------------------------------
// - PatchFormatStringFloatToInt()
// - DataTypeFormatString()
// - DataTypeApplyOp()
// - DataTypeApplyOpFromText()
// - GetMinimumStepAtDecimalPrecision
// - RoundScalarWithFormat<>()
//-------------------------------------------------------------------------

struct ImGuiDataTypeInfo
{
    size_t      Size;
    const char* PrintFmt;   // Unused
    const char* ScanFmt;
};

static const ImGuiDataTypeInfo GDataTypeInfo[] =
{
    { sizeof(int),          "%d",   "%d"    },
    { sizeof(unsigned int), "%u",   "%u"    },
#ifdef _MSC_VER
    { sizeof(ImS64),        "%I64d","%I64d" },
    { sizeof(ImU64),        "%I64u","%I64u" },
#else
    { sizeof(ImS64),        "%lld", "%lld"  },
    { sizeof(ImU64),        "%llu", "%llu"  },
#endif
    { sizeof(float),        "%f",   "%f"    },  // float are promoted to double in va_arg
    { sizeof(double),       "%f",   "%lf"   },
};
IM_STATIC_ASSERT(IM_ARRAYSIZE(GDataTypeInfo) == ImGuiDataType_COUNT);

// FIXME-LEGACY: Prior to 1.61 our DragInt() function internally used floats and because of this the compile-time default value for format was "%.0f".
// Even though we changed the compile-time default, we expect users to have carried %f around, which would break the display of DragInt() calls.
// To honor backward compatibility we are rewriting the format string, unless IMGUI_DISABLE_OBSOLETE_FUNCTIONS is enabled. What could possibly go wrong?!
static const char* PatchFormatStringFloatToInt(const char* fmt)
{
    if (fmt[0] == '%' && fmt[1] == '.' && fmt[2] == '0' && fmt[3] == 'f' && fmt[4] == 0) // Fast legacy path for "%.0f" which is expected to be the most common case.
        return "%d";
    const char* fmt_start = ImParseFormatFindStart(fmt);    // Find % (if any, and ignore %%)
    const char* fmt_end = ImParseFormatFindEnd(fmt_start);  // Find end of format specifier, which itself is an exercise of confidence/recklessness (because snprintf is dependent on libc or user).
    if (fmt_end > fmt_start && fmt_end[-1] == 'f')
    {
#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
        if (fmt_start == fmt && fmt_end[0] == 0)
            return "%d";
        ImGuiContext& g = *GImGui;
        ImFormatString(g.TempBuffer, IM_ARRAYSIZE(g.TempBuffer), "%.*s%%d%s", (int)(fmt_start - fmt), fmt, fmt_end); // Honor leading and trailing decorations, but lose alignment/precision.
        return g.TempBuffer;
#else
        IM_ASSERT(0 && "DragInt(): Invalid format string!"); // Old versions used a default parameter of "%.0f", please replace with e.g. "%d"
#endif
    }
    return fmt;
}

static int DataTypeFormatString(char* buf, int buf_size, ImGuiDataType data_type, const void* data_ptr, const char* format)
{
    if (data_type == ImGuiDataType_S32 || data_type == ImGuiDataType_U32)   // Signedness doesn't matter when pushing the argument
        return ImFormatString(buf, buf_size, format, *(const ImU32*)data_ptr);
    if (data_type == ImGuiDataType_S64 || data_type == ImGuiDataType_U64)   // Signedness doesn't matter when pushing the argument
        return ImFormatString(buf, buf_size, format, *(const ImU64*)data_ptr);
    if (data_type == ImGuiDataType_Float)
        return ImFormatString(buf, buf_size, format, *(const float*)data_ptr);
    if (data_type == ImGuiDataType_Double)
        return ImFormatString(buf, buf_size, format, *(const double*)data_ptr);
    IM_ASSERT(0);
    return 0;
}

// FIXME: Adding support for clamping on boundaries of the data type would be nice.
static void DataTypeApplyOp(ImGuiDataType data_type, int op, void* output, void* arg1, const void* arg2)
{
    IM_ASSERT(op == '+' || op == '-');
    switch (data_type)
    {
        case ImGuiDataType_S32:
            if (op == '+')      *(int*)output = *(const int*)arg1 + *(const int*)arg2;
            else if (op == '-') *(int*)output = *(const int*)arg1 - *(const int*)arg2;
            return;
        case ImGuiDataType_U32:
            if (op == '+')      *(unsigned int*)output = *(const unsigned int*)arg1 + *(const ImU32*)arg2;
            else if (op == '-') *(unsigned int*)output = *(const unsigned int*)arg1 - *(const ImU32*)arg2;
            return;
        case ImGuiDataType_S64:
            if (op == '+')      *(ImS64*)output = *(const ImS64*)arg1 + *(const ImS64*)arg2;
            else if (op == '-') *(ImS64*)output = *(const ImS64*)arg1 - *(const ImS64*)arg2;
            return;
        case ImGuiDataType_U64:
            if (op == '+')      *(ImU64*)output = *(const ImU64*)arg1 + *(const ImU64*)arg2;
            else if (op == '-') *(ImU64*)output = *(const ImU64*)arg1 - *(const ImU64*)arg2;
            return;
        case ImGuiDataType_Float:
            if (op == '+')      *(float*)output = *(const float*)arg1 + *(const float*)arg2;
            else if (op == '-') *(float*)output = *(const float*)arg1 - *(const float*)arg2;
            return;
        case ImGuiDataType_Double:
            if (op == '+')      *(double*)output = *(const double*)arg1 + *(const double*)arg2;
            else if (op == '-') *(double*)output = *(const double*)arg1 - *(const double*)arg2;
            return;
        case ImGuiDataType_COUNT: break;
    }
    IM_ASSERT(0);
}

// User can input math operators (e.g. +100) to edit a numerical values.
// NB: This is _not_ a full expression evaluator. We should probably add one and replace this dumb mess..
static bool DataTypeApplyOpFromText(const char* buf, const char* initial_value_buf, ImGuiDataType data_type, void* data_ptr, const char* format)
{
    while (ImCharIsBlankA(*buf))
        buf++;

    // We don't support '-' op because it would conflict with inputing negative value.
    // Instead you can use +-100 to subtract from an existing value
    char op = buf[0];
    if (op == '+' || op == '*' || op == '/')
    {
        buf++;
        while (ImCharIsBlankA(*buf))
            buf++;
    }
    else
    {
        op = 0;
    }
    if (!buf[0])
        return false;

    // Copy the value in an opaque buffer so we can compare at the end of the function if it changed at all.
    IM_ASSERT(data_type < ImGuiDataType_COUNT);
    int data_backup[2];
    IM_ASSERT(GDataTypeInfo[data_type].Size <= sizeof(data_backup));
    memcpy(data_backup, data_ptr, GDataTypeInfo[data_type].Size);

    if (format == NULL)
        format = GDataTypeInfo[data_type].ScanFmt;

    int arg1i = 0;
    if (data_type == ImGuiDataType_S32)
    {
        int* v = (int*)data_ptr;
        int arg0i = *v;
        float arg1f = 0.0f;
        if (op && sscanf(initial_value_buf, format, &arg0i) < 1)
            return false;
        // Store operand in a float so we can use fractional value for multipliers (*1.1), but constant always parsed as integer so we can fit big integers (e.g. 2000000003) past float precision
        if (op == '+')      { if (sscanf(buf, "%d", &arg1i)) *v = (int)(arg0i + arg1i); }                   // Add (use "+-" to subtract)
        else if (op == '*') { if (sscanf(buf, "%f", &arg1f)) *v = (int)(arg0i * arg1f); }                   // Multiply
        else if (op == '/') { if (sscanf(buf, "%f", &arg1f) && arg1f != 0.0f) *v = (int)(arg0i / arg1f); }  // Divide
        else                { if (sscanf(buf, format, &arg1i) == 1) *v = arg1i; }                           // Assign constant
    }
    else if (data_type == ImGuiDataType_U32 || data_type == ImGuiDataType_S64 || data_type == ImGuiDataType_U64)
    {
        // Assign constant
        // FIXME: We don't bother handling support for legacy operators since they are a little too crappy. Instead we may implement a proper expression evaluator in the future.
        sscanf(buf, format, data_ptr);
    }
    else if (data_type == ImGuiDataType_Float)
    {
        // For floats we have to ignore format with precision (e.g. "%.2f") because sscanf doesn't take them in
        format = "%f";
        float* v = (float*)data_ptr;
        float arg0f = *v, arg1f = 0.0f;
        if (op && sscanf(initial_value_buf, format, &arg0f) < 1)
            return false;
        if (sscanf(buf, format, &arg1f) < 1)
            return false;
        if (op == '+')      { *v = arg0f + arg1f; }                    // Add (use "+-" to subtract)
        else if (op == '*') { *v = arg0f * arg1f; }                    // Multiply
        else if (op == '/') { if (arg1f != 0.0f) *v = arg0f / arg1f; } // Divide
        else                { *v = arg1f; }                            // Assign constant
    }
    else if (data_type == ImGuiDataType_Double)
    {
        format = "%lf"; // scanf differentiate float/double unlike printf which forces everything to double because of ellipsis
        double* v = (double*)data_ptr;
        double arg0f = *v, arg1f = 0.0;
        if (op && sscanf(initial_value_buf, format, &arg0f) < 1)
            return false;
        if (sscanf(buf, format, &arg1f) < 1)
            return false;
        if (op == '+')      { *v = arg0f + arg1f; }                    // Add (use "+-" to subtract)
        else if (op == '*') { *v = arg0f * arg1f; }                    // Multiply
        else if (op == '/') { if (arg1f != 0.0f) *v = arg0f / arg1f; } // Divide
        else                { *v = arg1f; }                            // Assign constant
    }
    return memcmp(data_backup, data_ptr, GDataTypeInfo[data_type].Size) != 0;
}

static float GetMinimumStepAtDecimalPrecision(int decimal_precision)
{
    static const float min_steps[10] = { 1.0f, 0.1f, 0.01f, 0.001f, 0.0001f, 0.00001f, 0.000001f, 0.0000001f, 0.00000001f, 0.000000001f };
    if (decimal_precision < 0)
        return FLT_MIN;
    return (decimal_precision < IM_ARRAYSIZE(min_steps)) ? min_steps[decimal_precision] : ImPow(10.0f, (float)-decimal_precision);
}

template<typename TYPE>
static const char* ImAtoi(const char* src, TYPE* output)
{
    int negative = 0;
    if (*src == '-') { negative = 1; src++; }
    if (*src == '+') { src++; }
    TYPE v = 0;
    while (*src >= '0' && *src <= '9')
        v = (v * 10) + (*src++ - '0');
    *output = negative ? -v : v;
    return src;
}

template<typename TYPE, typename SIGNEDTYPE>
TYPE ImGui::RoundScalarWithFormatT(const char* format, ImGuiDataType data_type, TYPE v)
{
    const char* fmt_start = ImParseFormatFindStart(format);
    if (fmt_start[0] != '%' || fmt_start[1] == '%') // Don't apply if the value is not visible in the format string
        return v;
    char v_str[64];
    ImFormatString(v_str, IM_ARRAYSIZE(v_str), fmt_start, v);
    const char* p = v_str;
    while (*p == ' ')
        p++;
    if (data_type == ImGuiDataType_Float || data_type == ImGuiDataType_Double)
        v = (TYPE)ImAtof(p);
    else
        ImAtoi(p, (SIGNEDTYPE*)&v);
    return v;
}

//-------------------------------------------------------------------------
// [SECTION] Widgets: DragScalar, DragFloat, DragInt, etc.
//-------------------------------------------------------------------------
// - DragBehaviorT<>() [Internal]
// - DragBehavior() [Internal]
// - DragScalar()
// - DragScalarN()
// - DragFloat()
// - DragFloat2()
// - DragFloat3()
// - DragFloat4()
// - DragFloatRange2()
// - DragInt()
// - DragInt2()
// - DragInt3()
// - DragInt4()
// - DragIntRange2()
//-------------------------------------------------------------------------

// This is called by DragBehavior() when the widget is active (held by mouse or being manipulated with Nav controls)
template<typename TYPE, typename SIGNEDTYPE, typename FLOATTYPE>
bool ImGui::DragBehaviorT(ImGuiDataType data_type, TYPE* v, float v_speed, const TYPE v_min, const TYPE v_max, const char* format, float power, ImGuiDragFlags flags)
{
    ImGuiContext& g = *GImGui;
    const ImGuiAxis axis = (flags & ImGuiDragFlags_Vertical) ? ImGuiAxis_Y : ImGuiAxis_X;
    const bool is_decimal = (data_type == ImGuiDataType_Float) || (data_type == ImGuiDataType_Double);
    const bool has_min_max = (v_min != v_max);
    const bool is_power = (power != 1.0f && is_decimal && has_min_max && (v_max - v_min < FLT_MAX));

    // Default tweak speed
    if (v_speed == 0.0f && has_min_max && (v_max - v_min < FLT_MAX))
        v_speed = (float)((v_max - v_min) * g.DragSpeedDefaultRatio);

    // Inputs accumulates into g.DragCurrentAccum, which is flushed into the current value as soon as it makes a difference with our precision settings
    float adjust_delta = 0.0f;
    if (g.ActiveIdSource == ImGuiInputSource_Mouse && IsMousePosValid() && g.IO.MouseDragMaxDistanceSqr[0] > 1.0f*1.0f)
    {
        adjust_delta = g.IO.MouseDelta[axis];
        if (g.IO.KeyAlt)
            adjust_delta *= 1.0f / 100.0f;
        if (g.IO.KeyShift)
            adjust_delta *= 10.0f;
    }
    else if (g.ActiveIdSource == ImGuiInputSource_Nav)
    {
        int decimal_precision = is_decimal ? ImParseFormatPrecision(format, 3) : 0;
        adjust_delta = GetNavInputAmount2d(ImGuiNavDirSourceFlags_Keyboard | ImGuiNavDirSourceFlags_PadDPad, ImGuiInputReadMode_RepeatFast, 1.0f / 10.0f, 10.0f)[axis];
        v_speed = ImMax(v_speed, GetMinimumStepAtDecimalPrecision(decimal_precision));
    }
    adjust_delta *= v_speed;

    // For vertical drag we currently assume that Up=higher value (like we do with vertical sliders). This may become a parameter.
    if (axis == ImGuiAxis_Y)
        adjust_delta = -adjust_delta;

    // Clear current value on activation
    // Avoid altering values and clamping when we are _already_ past the limits and heading in the same direction, so e.g. if range is 0..255, current value is 300 and we are pushing to the right side, keep the 300.
    bool is_just_activated = g.ActiveIdIsJustActivated;
    bool is_already_past_limits_and_pushing_outward = has_min_max && ((*v >= v_max && adjust_delta > 0.0f) || (*v <= v_min && adjust_delta < 0.0f));
    bool is_drag_direction_change_with_power = is_power && ((adjust_delta < 0 && g.DragCurrentAccum > 0) || (adjust_delta > 0 && g.DragCurrentAccum < 0));
    if (is_just_activated || is_already_past_limits_and_pushing_outward || is_drag_direction_change_with_power)
    {
        g.DragCurrentAccum = 0.0f;
        g.DragCurrentAccumDirty = false;
    }
    else if (adjust_delta != 0.0f)
    {
        g.DragCurrentAccum += adjust_delta;
        g.DragCurrentAccumDirty = true;
    }

    if (!g.DragCurrentAccumDirty)
        return false;

    TYPE v_cur = *v;
    FLOATTYPE v_old_ref_for_accum_remainder = (FLOATTYPE)0.0f;

    if (is_power)
    {
        // Offset + round to user desired precision, with a curve on the v_min..v_max range to get more precision on one side of the range
        FLOATTYPE v_old_norm_curved = ImPow((FLOATTYPE)(v_cur - v_min) / (FLOATTYPE)(v_max - v_min), (FLOATTYPE)1.0f / power);
        FLOATTYPE v_new_norm_curved = v_old_norm_curved + (g.DragCurrentAccum / (v_max - v_min));
        v_cur = v_min + (TYPE)ImPow(ImSaturate((float)v_new_norm_curved), power) * (v_max - v_min);
        v_old_ref_for_accum_remainder = v_old_norm_curved;
    }
    else
    {
        v_cur += (TYPE)g.DragCurrentAccum;
    }

    // Round to user desired precision based on format string
    v_cur = RoundScalarWithFormatT<TYPE, SIGNEDTYPE>(format, data_type, v_cur);

    // Preserve remainder after rounding has been applied. This also allow slow tweaking of values.
    g.DragCurrentAccumDirty = false;
    if (is_power)
    {
        FLOATTYPE v_cur_norm_curved = ImPow((FLOATTYPE)(v_cur - v_min) / (FLOATTYPE)(v_max - v_min), (FLOATTYPE)1.0f / power);
        g.DragCurrentAccum -= (float)(v_cur_norm_curved - v_old_ref_for_accum_remainder);
    }
    else
    {
        g.DragCurrentAccum -= (float)((SIGNEDTYPE)v_cur - (SIGNEDTYPE)*v);
    }

    // Lose zero sign for float/double
    if (v_cur == (TYPE)-0)
        v_cur = (TYPE)0;

    // Clamp values (+ handle overflow/wrap-around for integer types)
    if (*v != v_cur && has_min_max)
    {
        if (v_cur < v_min || (v_cur > *v && adjust_delta < 0.0f && !is_decimal))
            v_cur = v_min;
        if (v_cur > v_max || (v_cur < *v && adjust_delta > 0.0f && !is_decimal))
            v_cur = v_max;
    }

    // Apply result
    if (*v == v_cur)
        return false;
    *v = v_cur;
    return true;
}

bool ImGui::DragBehavior(ImGuiID id, ImGuiDataType data_type, void* v, float v_speed, const void* v_min, const void* v_max, const char* format, float power, ImGuiDragFlags flags)
{
    ImGuiContext& g = *GImGui;
    if (g.ActiveId == id)
    {
        if (g.ActiveIdSource == ImGuiInputSource_Mouse && !g.IO.MouseDown[0])
            ClearActiveID();
        else if (g.ActiveIdSource == ImGuiInputSource_Nav && g.NavActivatePressedId == id && !g.ActiveIdIsJustActivated)
            ClearActiveID();
    }
    if (g.ActiveId != id)
        return false;

    switch (data_type)
    {
    case ImGuiDataType_S32:    return DragBehaviorT<ImS32, ImS32, float >(data_type, (ImS32*)v,  v_speed, v_min ? *(const ImS32* )v_min : IM_S32_MIN, v_max ? *(const ImS32* )v_max : IM_S32_MAX, format, power, flags);
    case ImGuiDataType_U32:    return DragBehaviorT<ImU32, ImS32, float >(data_type, (ImU32*)v,  v_speed, v_min ? *(const ImU32* )v_min : IM_U32_MIN, v_max ? *(const ImU32* )v_max : IM_U32_MAX, format, power, flags);
    case ImGuiDataType_S64:    return DragBehaviorT<ImS64, ImS64, double>(data_type, (ImS64*)v,  v_speed, v_min ? *(const ImS64* )v_min : IM_S64_MIN, v_max ? *(const ImS64* )v_max : IM_S64_MAX, format, power, flags);
    case ImGuiDataType_U64:    return DragBehaviorT<ImU64, ImS64, double>(data_type, (ImU64*)v,  v_speed, v_min ? *(const ImU64* )v_min : IM_U64_MIN, v_max ? *(const ImU64* )v_max : IM_U64_MAX, format, power, flags);
    case ImGuiDataType_Float:  return DragBehaviorT<float, float, float >(data_type, (float*)v,  v_speed, v_min ? *(const float* )v_min : -FLT_MAX,   v_max ? *(const float* )v_max : FLT_MAX,    format, power, flags);
    case ImGuiDataType_Double: return DragBehaviorT<double,double,double>(data_type, (double*)v, v_speed, v_min ? *(const double*)v_min : -DBL_MAX,   v_max ? *(const double*)v_max : DBL_MAX,    format, power, flags);
    case ImGuiDataType_COUNT:  break;
    }
    IM_ASSERT(0);
    return false;
}

bool ImGui::DragScalar(const char* label, ImGuiDataType data_type, void* v, float v_speed, const void* v_min, const void* v_max, const char* format, float power)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    if (power != 1.0f)
        IM_ASSERT(v_min != NULL && v_max != NULL); // When using a power curve the drag needs to have known bounds

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const float w = CalcItemWidth();

    const ImVec2 label_size = CalcTextSize(label, NULL, true);
    const ImRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + ImVec2(w, label_size.y + style.FramePadding.y*2.0f));
    const ImRect total_bb(frame_bb.Min, frame_bb.Max + ImVec2(label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f, 0.0f));

    ItemSize(total_bb, style.FramePadding.y);
    if (!ItemAdd(total_bb, id, &frame_bb))
        return false;

    const bool hovered = ItemHoverable(frame_bb, id);

    // Default format string when passing NULL
    // Patch old "%.0f" format string to use "%d", read function comments for more details.
    IM_ASSERT(data_type >= 0 && data_type < ImGuiDataType_COUNT);
    if (format == NULL)
        format = GDataTypeInfo[data_type].PrintFmt;
    else if (data_type == ImGuiDataType_S32 && strcmp(format, "%d") != 0)
        format = PatchFormatStringFloatToInt(format);

    // Tabbing or CTRL-clicking on Drag turns it into an input box
    bool start_text_input = false;
    const bool tab_focus_requested = FocusableItemRegister(window, id);
    if (tab_focus_requested || (hovered && (g.IO.MouseClicked[0] || g.IO.MouseDoubleClicked[0])) || g.NavActivateId == id || (g.NavInputId == id && g.ScalarAsInputTextId != id))
    {
        SetActiveID(id, window);
        SetFocusID(id, window);
        FocusWindow(window);
        g.ActiveIdAllowNavDirFlags = (1 << ImGuiDir_Up) | (1 << ImGuiDir_Down);
        if (tab_focus_requested || g.IO.KeyCtrl || g.IO.MouseDoubleClicked[0] || g.NavInputId == id)
        {
            start_text_input = true;
            g.ScalarAsInputTextId = 0;
        }
    }
    if (start_text_input || (g.ActiveId == id && g.ScalarAsInputTextId == id))
    {
        window->DC.CursorPos = frame_bb.Min;
        FocusableItemUnregister(window);
        return InputScalarAsWidgetReplacement(frame_bb, id, label, data_type, v, format);
    }

    // Actual drag behavior
    const bool value_changed = DragBehavior(id, data_type, v, v_speed, v_min, v_max, format, power, ImGuiDragFlags_None);
    if (value_changed)
        MarkItemEdited(id);

    // Draw frame
    const ImU32 frame_col = GetColorU32(g.ActiveId == id ? ImGuiCol_FrameBgActive : g.HoveredId == id ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg);
    RenderNavHighlight(frame_bb, id);
    RenderFrame(frame_bb.Min, frame_bb.Max, frame_col, true, style.FrameRounding);

    // Display value using user-provided display format so user can add prefix/suffix/decorations to the value.
    char value_buf[64];
    const char* value_buf_end = value_buf + DataTypeFormatString(value_buf, IM_ARRAYSIZE(value_buf), data_type, v, format);
    RenderTextClipped(frame_bb.Min, frame_bb.Max, value_buf, value_buf_end, NULL, ImVec2(0.5f, 0.5f));

    if (label_size.x > 0.0f)
        RenderText(ImVec2(frame_bb.Max.x + style.ItemInnerSpacing.x, frame_bb.Min.y + style.FramePadding.y), label);

    IMGUI_TEST_ENGINE_ITEM_INFO(id, label, window->DC.ItemFlags);
    return value_changed;
}

bool ImGui::DragScalarN(const char* label, ImGuiDataType data_type, void* v, int components, float v_speed, const void* v_min, const void* v_max, const char* format, float power)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    bool value_changed = false;
    BeginGroup();
    PushID(label);
    PushMultiItemsWidths(components);
    size_t type_size = GDataTypeInfo[data_type].Size;
    for (int i = 0; i < components; i++)
    {
        PushID(i);
        value_changed |= DragScalar("", data_type, v, v_speed, v_min, v_max, format, power);
        SameLine(0, g.Style.ItemInnerSpacing.x);
        PopID();
        PopItemWidth();
        v = (void*)((char*)v + type_size);
    }
    PopID();

    TextUnformatted(label, FindRenderedTextEnd(label));
    EndGroup();
    return value_changed;
}

bool ImGui::DragFloat(const char* label, float* v, float v_speed, float v_min, float v_max, const char* format, float power)
{
    return DragScalar(label, ImGuiDataType_Float, v, v_speed, &v_min, &v_max, format, power);
}

bool ImGui::DragFloat2(const char* label, float v[2], float v_speed, float v_min, float v_max, const char* format, float power)
{
    return DragScalarN(label, ImGuiDataType_Float, v, 2, v_speed, &v_min, &v_max, format, power);
}

bool ImGui::DragFloat3(const char* label, float v[3], float v_speed, float v_min, float v_max, const char* format, float power)
{
    return DragScalarN(label, ImGuiDataType_Float, v, 3, v_speed, &v_min, &v_max, format, power);
}

bool ImGui::DragFloat4(const char* label, float v[4], float v_speed, float v_min, float v_max, const char* format, float power)
{
    return DragScalarN(label, ImGuiDataType_Float, v, 4, v_speed, &v_min, &v_max, format, power);
}

bool ImGui::DragFloatRange2(const char* label, float* v_current_min, float* v_current_max, float v_speed, float v_min, float v_max, const char* format, const char* format_max, float power)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    PushID(label);
    BeginGroup();
    PushMultiItemsWidths(2);

    bool value_changed = DragFloat("##min", v_current_min, v_speed, (v_min >= v_max) ? -FLT_MAX : v_min, (v_min >= v_max) ? *v_current_max : ImMin(v_max, *v_current_max), format, power);
    PopItemWidth();
    SameLine(0, g.Style.ItemInnerSpacing.x);
    value_changed |= DragFloat("##max", v_current_max, v_speed, (v_min >= v_max) ? *v_current_min : ImMax(v_min, *v_current_min), (v_min >= v_max) ? FLT_MAX : v_max, format_max ? format_max : format, power);
    PopItemWidth();
    SameLine(0, g.Style.ItemInnerSpacing.x);

    TextUnformatted(label, FindRenderedTextEnd(label));
    EndGroup();
    PopID();
    return value_changed;
}

// NB: v_speed is float to allow adjusting the drag speed with more precision
bool ImGui::DragInt(const char* label, int* v, float v_speed, int v_min, int v_max, const char* format)
{
    return DragScalar(label, ImGuiDataType_S32, v, v_speed, &v_min, &v_max, format);
}

bool ImGui::DragInt2(const char* label, int v[2], float v_speed, int v_min, int v_max, const char* format)
{
    return DragScalarN(label, ImGuiDataType_S32, v, 2, v_speed, &v_min, &v_max, format);
}

bool ImGui::DragInt3(const char* label, int v[3], float v_speed, int v_min, int v_max, const char* format)
{
    return DragScalarN(label, ImGuiDataType_S32, v, 3, v_speed, &v_min, &v_max, format);
}

bool ImGui::DragInt4(const char* label, int v[4], float v_speed, int v_min, int v_max, const char* format)
{
    return DragScalarN(label, ImGuiDataType_S32, v, 4, v_speed, &v_min, &v_max, format);
}

bool ImGui::DragIntRange2(const char* label, int* v_current_min, int* v_current_max, float v_speed, int v_min, int v_max, const char* format, const char* format_max)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    PushID(label);
    BeginGroup();
    PushMultiItemsWidths(2);

    bool value_changed = DragInt("##min", v_current_min, v_speed, (v_min >= v_max) ? INT_MIN : v_min, (v_min >= v_max) ? *v_current_max : ImMin(v_max, *v_current_max), format);
    PopItemWidth();
    SameLine(0, g.Style.ItemInnerSpacing.x);
    value_changed |= DragInt("##max", v_current_max, v_speed, (v_min >= v_max) ? *v_current_min : ImMax(v_min, *v_current_min), (v_min >= v_max) ? INT_MAX : v_max, format_max ? format_max : format);
    PopItemWidth();
    SameLine(0, g.Style.ItemInnerSpacing.x);

    TextUnformatted(label, FindRenderedTextEnd(label));
    EndGroup();
    PopID();

    return value_changed;
}

//-------------------------------------------------------------------------
// [SECTION] Widgets: SliderScalar, SliderFloat, SliderInt, etc.
//-------------------------------------------------------------------------
// - SliderBehaviorT<>() [Internal]
// - SliderBehavior() [Internal]
// - SliderScalar()
// - SliderScalarN()
// - SliderFloat()
// - SliderFloat2()
// - SliderFloat3()
// - SliderFloat4()
// - SliderAngle()
// - SliderInt()
// - SliderInt2()
// - SliderInt3()
// - SliderInt4()
// - VSliderScalar()
// - VSliderFloat()
// - VSliderInt()
//-------------------------------------------------------------------------

template<typename TYPE, typename FLOATTYPE>
float ImGui::SliderCalcRatioFromValueT(ImGuiDataType data_type, TYPE v, TYPE v_min, TYPE v_max, float power, float linear_zero_pos)
{
    if (v_min == v_max)
        return 0.0f;

    const bool is_power = (power != 1.0f) && (data_type == ImGuiDataType_Float || data_type == ImGuiDataType_Double);
    const TYPE v_clamped = (v_min < v_max) ? ImClamp(v, v_min, v_max) : ImClamp(v, v_max, v_min);
    if (is_power)
    {
        if (v_clamped < 0.0f)
        {
            const float f = 1.0f - (float)((v_clamped - v_min) / (ImMin((TYPE)0, v_max) - v_min));
            return (1.0f - ImPow(f, 1.0f/power)) * linear_zero_pos;
        }
        else
        {
            const float f = (float)((v_clamped - ImMax((TYPE)0, v_min)) / (v_max - ImMax((TYPE)0, v_min)));
            return linear_zero_pos + ImPow(f, 1.0f/power) * (1.0f - linear_zero_pos);
        }
    }

    // Linear slider
    return (float)((FLOATTYPE)(v_clamped - v_min) / (FLOATTYPE)(v_max - v_min));
}

// FIXME: Move some of the code into SliderBehavior(). Current responsability is larger than what the equivalent DragBehaviorT<> does, we also do some rendering, etc.
template<typename TYPE, typename SIGNEDTYPE, typename FLOATTYPE>
bool ImGui::SliderBehaviorT(const ImRect& bb, ImGuiID id, ImGuiDataType data_type, TYPE* v, const TYPE v_min, const TYPE v_max, const char* format, float power, ImGuiSliderFlags flags, ImRect* out_grab_bb)
{
    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;

    const ImGuiAxis axis = (flags & ImGuiSliderFlags_Vertical) ? ImGuiAxis_Y : ImGuiAxis_X;
    const bool is_decimal = (data_type == ImGuiDataType_Float) || (data_type == ImGuiDataType_Double);
    const bool is_power = (power != 1.0f) && is_decimal;

    const float grab_padding = 0.0f;
    const float slider_sz = (bb.Max[axis] - bb.Min[axis]) - grab_padding * 2.0f;
    float grab_sz = style.GrabMinSize;
    SIGNEDTYPE v_range = (v_min < v_max ? v_max - v_min : v_min - v_max);
    if (!is_decimal && v_range >= 0)                                             // v_range < 0 may happen on integer overflows
        grab_sz = ImMax((float)(slider_sz / (v_range + 1)), style.GrabMinSize);  // For integer sliders: if possible have the grab size represent 1 unit
    grab_sz = ImMin(grab_sz, slider_sz);
    const float slider_usable_sz = slider_sz - grab_sz;
    const float slider_usable_pos_min = bb.Min[axis] + grab_padding + grab_sz*0.5f;
    const float slider_usable_pos_max = bb.Max[axis] - grab_padding - grab_sz*0.5f;

    // For power curve sliders that cross over sign boundary we want the curve to be symmetric around 0.0f
    float linear_zero_pos;   // 0.0->1.0f
    if (is_power && v_min * v_max < 0.0f)
    {
        // Different sign
        const FLOATTYPE linear_dist_min_to_0 = ImPow(v_min >= 0 ? (FLOATTYPE)v_min : -(FLOATTYPE)v_min, (FLOATTYPE)1.0f/power);
        const FLOATTYPE linear_dist_max_to_0 = ImPow(v_max >= 0 ? (FLOATTYPE)v_max : -(FLOATTYPE)v_max, (FLOATTYPE)1.0f/power);
        linear_zero_pos = (float)(linear_dist_min_to_0 / (linear_dist_min_to_0 + linear_dist_max_to_0));
    }
    else
    {
        // Same sign
        linear_zero_pos = v_min < 0.0f ? 1.0f : 0.0f;
    }

    // Process interacting with the slider
    bool value_changed = false;
    if (g.ActiveId == id)
    {
        bool set_new_value = false;
        float clicked_t = 0.0f;
        if (g.ActiveIdSource == ImGuiInputSource_Mouse)
        {
            if (!g.IO.MouseDown[0])
            {
                ClearActiveID();
            }
            else
            {
                const float mouse_abs_pos = g.IO.MousePos[axis];
                clicked_t = (slider_usable_sz > 0.0f) ? ImClamp((mouse_abs_pos - slider_usable_pos_min) / slider_usable_sz, 0.0f, 1.0f) : 0.0f;
                if (axis == ImGuiAxis_Y)
                    clicked_t = 1.0f - clicked_t;
                set_new_value = true;
            }
        }
        else if (g.ActiveIdSource == ImGuiInputSource_Nav)
        {
            const ImVec2 delta2 = GetNavInputAmount2d(ImGuiNavDirSourceFlags_Keyboard | ImGuiNavDirSourceFlags_PadDPad, ImGuiInputReadMode_RepeatFast, 0.0f, 0.0f);
            float delta = (axis == ImGuiAxis_X) ? delta2.x : -delta2.y;
            if (g.NavActivatePressedId == id && !g.ActiveIdIsJustActivated)
            {
                ClearActiveID();
            }
            else if (delta != 0.0f)
            {
                clicked_t = SliderCalcRatioFromValueT<TYPE,FLOATTYPE>(data_type, *v, v_min, v_max, power, linear_zero_pos);
                const int decimal_precision = is_decimal ? ImParseFormatPrecision(format, 3) : 0;
                if ((decimal_precision > 0) || is_power)
                {
                    delta /= 100.0f;    // Gamepad/keyboard tweak speeds in % of slider bounds
                    if (IsNavInputDown(ImGuiNavInput_TweakSlow))
                        delta /= 10.0f;
                }
                else
                {
                    if ((v_range >= -100.0f && v_range <= 100.0f) || IsNavInputDown(ImGuiNavInput_TweakSlow))
                        delta = ((delta < 0.0f) ? -1.0f : +1.0f) / (float)v_range; // Gamepad/keyboard tweak speeds in integer steps
                    else
                        delta /= 100.0f;
                }
                if (IsNavInputDown(ImGuiNavInput_TweakFast))
                    delta *= 10.0f;
                set_new_value = true;
                if ((clicked_t >= 1.0f && delta > 0.0f) || (clicked_t <= 0.0f && delta < 0.0f)) // This is to avoid applying the saturation when already past the limits
                    set_new_value = false;
                else
                    clicked_t = ImSaturate(clicked_t + delta);
            }
        }

        if (set_new_value)
        {
            TYPE v_new;
            if (is_power)
            {
                // Account for power curve scale on both sides of the zero
                if (clicked_t < linear_zero_pos)
                {
                    // Negative: rescale to the negative range before powering
                    float a = 1.0f - (clicked_t / linear_zero_pos);
                    a = ImPow(a, power);
                    v_new = ImLerp(ImMin(v_max, (TYPE)0), v_min, a);
                }
                else
                {
                    // Positive: rescale to the positive range before powering
                    float a;
                    if (ImFabs(linear_zero_pos - 1.0f) > 1.e-6f)
                        a = (clicked_t - linear_zero_pos) / (1.0f - linear_zero_pos);
                    else
                        a = clicked_t;
                    a = ImPow(a, power);
                    v_new = ImLerp(ImMax(v_min, (TYPE)0), v_max, a);
                }
            }
            else
            {
                // Linear slider
                if (is_decimal)
                {
                    v_new = ImLerp(v_min, v_max, clicked_t);
                }
                else
                {
                    // For integer values we want the clicking position to match the grab box so we round above
                    // This code is carefully tuned to work with large values (e.g. high ranges of U64) while preserving this property..
                    FLOATTYPE v_new_off_f = (v_max - v_min) * clicked_t;
                    TYPE v_new_off_floor = (TYPE)(v_new_off_f);
                    TYPE v_new_off_round = (TYPE)(v_new_off_f + (FLOATTYPE)0.5);
                    if (!is_decimal && v_new_off_floor < v_new_off_round)
                        v_new = v_min + v_new_off_round;
                    else
                        v_new = v_min + v_new_off_floor;
                }
            }

            // Round to user desired precision based on format string
            v_new = RoundScalarWithFormatT<TYPE,SIGNEDTYPE>(format, data_type, v_new);

            // Apply result
            if (*v != v_new)
            {
                *v = v_new;
                value_changed = true;
            }
        }
    }

    // Output grab position so it can be displayed by the caller
    float grab_t = SliderCalcRatioFromValueT<TYPE,FLOATTYPE>(data_type, *v, v_min, v_max, power, linear_zero_pos);
    if (axis == ImGuiAxis_Y)
        grab_t = 1.0f - grab_t;
    const float grab_pos = ImLerp(slider_usable_pos_min, slider_usable_pos_max, grab_t);
    if (axis == ImGuiAxis_X)
        *out_grab_bb = ImRect(grab_pos - grab_sz*0.5f, bb.Min.y + grab_padding, grab_pos + grab_sz*0.5f, bb.Max.y - grab_padding);
    else
        *out_grab_bb = ImRect(bb.Min.x + grab_padding, grab_pos - grab_sz*0.5f, bb.Max.x - grab_padding, grab_pos + grab_sz*0.5f);

    return value_changed;
}

// For 32-bits and larger types, slider bounds are limited to half the natural type range.
// So e.g. an integer Slider between INT_MAX-10 and INT_MAX will fail, but an integer Slider between INT_MAX/2-10 and INT_MAX/2 will be ok.
// It would be possible to lift that limitation with some work but it doesn't seem to be worth it for sliders.
bool ImGui::SliderBehavior(const ImRect& bb, ImGuiID id, ImGuiDataType data_type, void* v, const void* v_min, const void* v_max, const char* format, float power, ImGuiSliderFlags flags, ImRect* out_grab_bb)
{
    switch (data_type)
    {
    case ImGuiDataType_S32:
        IM_ASSERT(*(const ImS32*)v_min >= IM_S32_MIN/2 && *(const ImS32*)v_max <= IM_S32_MAX/2);
        return SliderBehaviorT<ImS32, ImS32, float >(bb, id, data_type, (ImS32*)v,  *(const ImS32*)v_min,  *(const ImS32*)v_max,  format, power, flags, out_grab_bb);
    case ImGuiDataType_U32:
        IM_ASSERT(*(const ImU32*)v_min <= IM_U32_MAX/2);
        return SliderBehaviorT<ImU32, ImS32, float >(bb, id, data_type, (ImU32*)v,  *(const ImU32*)v_min,  *(const ImU32*)v_max,  format, power, flags, out_grab_bb);
    case ImGuiDataType_S64:
        IM_ASSERT(*(const ImS64*)v_min >= IM_S64_MIN/2 && *(const ImS64*)v_max <= IM_S64_MAX/2);
        return SliderBehaviorT<ImS64, ImS64, double>(bb, id, data_type, (ImS64*)v,  *(const ImS64*)v_min,  *(const ImS64*)v_max,  format, power, flags, out_grab_bb);
    case ImGuiDataType_U64:
        IM_ASSERT(*(const ImU64*)v_min <= IM_U64_MAX/2);
        return SliderBehaviorT<ImU64, ImS64, double>(bb, id, data_type, (ImU64*)v,  *(const ImU64*)v_min,  *(const ImU64*)v_max,  format, power, flags, out_grab_bb);
    case ImGuiDataType_Float:
        IM_ASSERT(*(const float*)v_min >= -FLT_MAX/2.0f && *(const float*)v_max <= FLT_MAX/2.0f);
        return SliderBehaviorT<float, float, float >(bb, id, data_type, (float*)v,  *(const float*)v_min,  *(const float*)v_max,  format, power, flags, out_grab_bb);
    case ImGuiDataType_Double:
        IM_ASSERT(*(const double*)v_min >= -DBL_MAX/2.0f && *(const double*)v_max <= DBL_MAX/2.0f);
        return SliderBehaviorT<double,double,double>(bb, id, data_type, (double*)v, *(const double*)v_min, *(const double*)v_max, format, power, flags, out_grab_bb);
    case ImGuiDataType_COUNT: break;
    }
    IM_ASSERT(0);
    return false;
}

bool ImGui::SliderScalar(const char* label, ImGuiDataType data_type, void* v, const void* v_min, const void* v_max, const char* format, float power)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    char value_buf[64];
    const char* value_buf_end = value_buf + DataTypeFormatString(value_buf, IM_ARRAYSIZE(value_buf), data_type, v, format);
    //RenderTextClipped(frame_bb_new.Min, frame_bb_new.Max, value_buf, value_buf_end, NULL, ImVec2(0.5f,0.5f));
    //RenderTextClipped(frame_bb_new.Min, frame_bb_new.Max, value_buf, value_buf_end, NULL, ImVec2(0.5f,0.5f));
    ImVec2 value_size_loc = CalcTextSize(value_buf);

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const float w = CalcItemWidth();

    float offset_x = 17 + style.ItemInnerSpacing.x;
    ImVec2 offset = ImVec2(offset_x, 0.f);

    float offset_scroll = 0;
    if (window->ScrollbarY)
        offset_scroll = style.ScrollbarSize;

    ImVec2 pos_end = ImVec2(window->DC.CursorPos.x + (window->Size.x - (offset_x * 2) - 1.f - offset_scroll), window->DC.CursorPos.y);

    const ImVec2 label_size = CalcTextSize(label, NULL, true);
    //const ImRect frame_bb(window->DC.CursorPos + offset, window->DC.CursorPos - ImVec2(0, 2.f) + ImVec2(w, label_size.y + style.FramePadding.y * 2.0f));
    //const ImRect frame_bb_new(window->DC.CursorPos + ImVec2(0, label_size.y + 2.f) + offset, window->DC.CursorPos + ImVec2(0, label_size.y - 2.f) + ImVec2(w, label_size.y + style.FramePadding.y*2.0f));
    //const ImRect total_bb(frame_bb.Min, frame_bb.Max + ImVec2(label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f, label_size.y));
    const ImRect frame_bb(window->DC.CursorPos + offset, pos_end + ImVec2(0, label_size.y + style.FramePadding.y * 2.0f));
    const ImRect frame_bb_text(window->DC.CursorPos, ImVec2(window->DC.CursorPos.x + (window->Size.x - (offset_x * 3) - 1.f - offset_scroll), window->DC.CursorPos.y) + ImVec2(0, label_size.y + style.FramePadding.y * 2.0f));
    const ImRect frame_bb_new(window->DC.CursorPos + ImVec2(0, label_size.y + 2.f) + offset, pos_end + ImVec2(0, label_size.y - 6.f) + ImVec2(0, label_size.y + style.FramePadding.y * 2.0f));
    const ImRect total_bb(frame_bb.Min, frame_bb_new.Max /*+ ImVec2(/*label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f0.f, label_size.y)*/);
    const ImRect frame_bb_text_input(ImVec2(frame_bb.Max.x - value_size_loc.x, frame_bb.Min.y), ImVec2(frame_bb.Max.x, frame_bb.Min.y + value_size_loc.y));

    ItemSize(total_bb, style.FramePadding.y);
    if (!ItemAdd(total_bb, id, &frame_bb_new))
        return false;

    // Default format string when passing NULL
    // Patch old "%.0f" format string to use "%d", read function comments for more details.
    IM_ASSERT(data_type >= 0 && data_type < ImGuiDataType_COUNT);
    if (format == NULL)
        format = GDataTypeInfo[data_type].PrintFmt;
    else if (data_type == ImGuiDataType_S32 && strcmp(format, "%d") != 0)
        format = PatchFormatStringFloatToInt(format);

    // Tabbing or CTRL-clicking on Slider turns it into an input box
    bool start_text_input = false;
    const bool focus_requested = FocusableItemRegister(window, id);
    const bool hovered = ItemHoverable(frame_bb_new, id);
    const bool hovered_input = ItemHoverable(frame_bb_text_input, id);
    if (focus_requested || (hovered && g.IO.MouseClicked[0]) || g.NavActivateId == id || (g.NavInputId == id && g.ScalarAsInputTextId != id))
    {
        SetActiveID(id, window);
        SetFocusID(id, window);
        FocusWindow(window);
        g.ActiveIdAllowNavDirFlags = (1 << ImGuiDir_Up) | (1 << ImGuiDir_Down);
    }
    else if (focus_requested || (hovered_input && g.IO.MouseClicked[0]) || g.NavActivateId == id || (g.NavInputId == id && g.ScalarAsInputTextId != id))
    {
        if (focus_requested || g.IO.KeyCtrl || g.NavInputId == id)
        {
            start_text_input = true;
            g.ScalarAsInputTextId = 0;
        }
    }
    if (start_text_input || (g.ActiveId == id && g.ScalarAsInputTextId == id))
    {
        window->DC.CursorPos = frame_bb_text.Min;
        FocusableItemUnregister(window);
        return InputScalarAsWidgetReplacement(frame_bb_text, id, label, data_type, v, format);
    }

    // Draw frame
    const ImU32 frame_col = GetColorU32(g.ActiveId == id ? ImGuiCol_FrameBgActive : hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg);
    RenderNavHighlight(frame_bb_new, id);
    RenderFrame(frame_bb_new.Min, frame_bb_new.Max, frame_col, true, g.Style.FrameRounding);

    // Slider behavior
    ImRect grab_bb;
    const bool value_changed = SliderBehavior(frame_bb_new, id, data_type, v, v_min, v_max, format, power, ImGuiSliderFlags_None, &grab_bb);
    if (value_changed)
        MarkItemEdited(id);

    // Render grab
    window->DrawList->AddRectFilled(frame_bb_new.Min, grab_bb.Max, GetColorU32(/*g.ActiveId == id ? ImGuiCol_SliderGrabActive : */ImGuiCol_SliderGrab), style.GrabRounding);

    // Display value using user-provided display format so user can add prefix/suffix/decorations to the value.
    RenderText(ImVec2(frame_bb.Max.x - value_size_loc.x, frame_bb.Min.y), value_buf);

    if (label_size.x > 0.0f)
        RenderText(ImVec2(frame_bb.Min.x, frame_bb.Min.y), label);

    //window->DrawList->AddRectFilled(frame_bb_text_input.Min, frame_bb_text_input.Max, GetColorU32(ImVec4(1.f, 1.f, 1.f, 0.2f)));

    IMGUI_TEST_ENGINE_ITEM_INFO(id, label, window->DC.ItemFlags);
    return value_changed;
}

// Add multiple sliders on 1 line for compact edition of multiple components
bool ImGui::SliderScalarN(const char* label, ImGuiDataType data_type, void* v, int components, const void* v_min, const void* v_max, const char* format, float power)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    bool value_changed = false;
    BeginGroup();
    PushID(label);
    PushMultiItemsWidths(components);
    size_t type_size = GDataTypeInfo[data_type].Size;
    for (int i = 0; i < components; i++)
    {
        PushID(i);
        value_changed |= SliderScalar("", data_type, v, v_min, v_max, format, power);
        SameLine(0, g.Style.ItemInnerSpacing.x);
        PopID();
        PopItemWidth();
        v = (void*)((char*)v + type_size);
    }
    PopID();

    TextUnformatted(label, FindRenderedTextEnd(label));
    EndGroup();
    return value_changed;
}

bool ImGui::SliderFloat(const char* label, float* v, float v_min, float v_max, const char* format, float power)
{
    return SliderScalar(label, ImGuiDataType_Float, v, &v_min, &v_max, format, power);
}

bool ImGui::SliderFloat2(const char* label, float v[2], float v_min, float v_max, const char* format, float power)
{
    return SliderScalarN(label, ImGuiDataType_Float, v, 2, &v_min, &v_max, format, power);
}

bool ImGui::SliderFloat3(const char* label, float v[3], float v_min, float v_max, const char* format, float power)
{
    return SliderScalarN(label, ImGuiDataType_Float, v, 3, &v_min, &v_max, format, power);
}

bool ImGui::SliderFloat4(const char* label, float v[4], float v_min, float v_max, const char* format, float power)
{
    return SliderScalarN(label, ImGuiDataType_Float, v, 4, &v_min, &v_max, format, power);
}

bool ImGui::SliderAngle(const char* label, float* v_rad, float v_degrees_min, float v_degrees_max, const char* format)
{
    if (format == NULL)
        format = "%.0f deg";
    float v_deg = (*v_rad) * 360.0f / (2*IM_PI);
    bool value_changed = SliderFloat(label, &v_deg, v_degrees_min, v_degrees_max, format, 1.0f);
    *v_rad = v_deg * (2*IM_PI) / 360.0f;
    return value_changed;
}

bool ImGui::SliderInt(const char* label, int* v, int v_min, int v_max, const char* format)
{
    return SliderScalar(label, ImGuiDataType_S32, v, &v_min, &v_max, format);
}

bool ImGui::SliderInt2(const char* label, int v[2], int v_min, int v_max, const char* format)
{
    return SliderScalarN(label, ImGuiDataType_S32, v, 2, &v_min, &v_max, format);
}

bool ImGui::SliderInt3(const char* label, int v[3], int v_min, int v_max, const char* format)
{
    return SliderScalarN(label, ImGuiDataType_S32, v, 3, &v_min, &v_max, format);
}

bool ImGui::SliderInt4(const char* label, int v[4], int v_min, int v_max, const char* format)
{
    return SliderScalarN(label, ImGuiDataType_S32, v, 4, &v_min, &v_max, format);
}

bool ImGui::VSliderScalar(const char* label, const ImVec2& size, ImGuiDataType data_type, void* v, const void* v_min, const void* v_max, const char* format, float power)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);

    const ImVec2 label_size = CalcTextSize(label, NULL, true);
    const ImRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + size);
    const ImRect bb(frame_bb.Min, frame_bb.Max + ImVec2(label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f, 0.0f));

    ItemSize(bb, style.FramePadding.y);
    if (!ItemAdd(frame_bb, id))
        return false;

    // Default format string when passing NULL
    // Patch old "%.0f" format string to use "%d", read function comments for more details.
    IM_ASSERT(data_type >= 0 && data_type < ImGuiDataType_COUNT);
    if (format == NULL)
        format = GDataTypeInfo[data_type].PrintFmt;
    else if (data_type == ImGuiDataType_S32 && strcmp(format, "%d") != 0)
        format = PatchFormatStringFloatToInt(format);

    const bool hovered = ItemHoverable(frame_bb, id);
    if ((hovered && g.IO.MouseClicked[0]) || g.NavActivateId == id || g.NavInputId == id)
    {
        SetActiveID(id, window);
        SetFocusID(id, window);
        FocusWindow(window);
        g.ActiveIdAllowNavDirFlags = (1 << ImGuiDir_Left) | (1 << ImGuiDir_Right);
    }

    // Draw frame
    const ImU32 frame_col = GetColorU32(g.ActiveId == id ? ImGuiCol_FrameBgActive : g.HoveredId == id ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg);
    RenderNavHighlight(frame_bb, id);
    RenderFrame(frame_bb.Min, frame_bb.Max, frame_col, true, g.Style.FrameRounding);

    // Slider behavior
    ImRect grab_bb;
    const bool value_changed = SliderBehavior(frame_bb, id, data_type, v, v_min, v_max, format, power, ImGuiSliderFlags_Vertical, &grab_bb);
    if (value_changed)
        MarkItemEdited(id);

    // Render grab
    window->DrawList->AddRectFilled(grab_bb.Min, grab_bb.Max, GetColorU32(g.ActiveId == id ? ImGuiCol_SliderGrabActive : ImGuiCol_SliderGrab), style.GrabRounding);

    // Display value using user-provided display format so user can add prefix/suffix/decorations to the value.
    // For the vertical slider we allow centered text to overlap the frame padding
    char value_buf[64];
    const char* value_buf_end = value_buf + DataTypeFormatString(value_buf, IM_ARRAYSIZE(value_buf), data_type, v, format);
    RenderTextClipped(ImVec2(frame_bb.Min.x, frame_bb.Min.y + style.FramePadding.y), frame_bb.Max, value_buf, value_buf_end, NULL, ImVec2(0.5f,0.0f));
    if (label_size.x > 0.0f)
        RenderText(ImVec2(frame_bb.Max.x + style.ItemInnerSpacing.x, frame_bb.Min.y + style.FramePadding.y), label);

    return value_changed;
}

bool ImGui::VSliderFloat(const char* label, const ImVec2& size, float* v, float v_min, float v_max, const char* format, float power)
{
    return VSliderScalar(label, size, ImGuiDataType_Float, v, &v_min, &v_max, format, power);
}

bool ImGui::VSliderInt(const char* label, const ImVec2& size, int* v, int v_min, int v_max, const char* format)
{
    return VSliderScalar(label, size, ImGuiDataType_S32, v, &v_min, &v_max, format);
}

//-------------------------------------------------------------------------
// [SECTION] Widgets: InputScalar, InputFloat, InputInt, etc.
//-------------------------------------------------------------------------
// - ImParseFormatFindStart() [Internal]
// - ImParseFormatFindEnd() [Internal]
// - ImParseFormatTrimDecorations() [Internal]
// - ImParseFormatPrecision() [Internal]
// - InputScalarAsWidgetReplacement() [Internal]
// - InputScalar()
// - InputScalarN()
// - InputFloat()
// - InputFloat2()
// - InputFloat3()
// - InputFloat4()
// - InputInt()
// - InputInt2()
// - InputInt3()
// - InputInt4()
// - InputDouble()
//-------------------------------------------------------------------------

// We don't use strchr() because our strings are usually very short and often start with '%'
const char* ImParseFormatFindStart(const char* fmt)
{
    while (char c = fmt[0])
    {
        if (c == '%' && fmt[1] != '%')
            return fmt;
        else if (c == '%')
            fmt++;
        fmt++;
    }
    return fmt;
}

const char* ImParseFormatFindEnd(const char* fmt)
{
    // Printf/scanf types modifiers: I/L/h/j/l/t/w/z. Other uppercase letters qualify as types aka end of the format.
    if (fmt[0] != '%')
        return fmt;
    const unsigned int ignored_uppercase_mask = (1 << ('I'-'A')) | (1 << ('L'-'A'));
    const unsigned int ignored_lowercase_mask = (1 << ('h'-'a')) | (1 << ('j'-'a')) | (1 << ('l'-'a')) | (1 << ('t'-'a')) | (1 << ('w'-'a')) | (1 << ('z'-'a'));
    for (char c; (c = *fmt) != 0; fmt++)
    {
        if (c >= 'A' && c <= 'Z' && ((1 << (c - 'A')) & ignored_uppercase_mask) == 0)
            return fmt + 1;
        if (c >= 'a' && c <= 'z' && ((1 << (c - 'a')) & ignored_lowercase_mask) == 0)
            return fmt + 1;
    }
    return fmt;
}

// Extract the format out of a format string with leading or trailing decorations
//  fmt = "blah blah"  -> return fmt
//  fmt = "%.3f"       -> return fmt
//  fmt = "hello %.3f" -> return fmt + 6
//  fmt = "%.3f hello" -> return buf written with "%.3f"
const char* ImParseFormatTrimDecorations(const char* fmt, char* buf, size_t buf_size)
{
    const char* fmt_start = ImParseFormatFindStart(fmt);
    if (fmt_start[0] != '%')
        return fmt;
    const char* fmt_end = ImParseFormatFindEnd(fmt_start);
    if (fmt_end[0] == 0) // If we only have leading decoration, we don't need to copy the data.
        return fmt_start;
    ImStrncpy(buf, fmt_start, ImMin((size_t)(fmt_end - fmt_start) + 1, buf_size));
    return buf;
}

// Parse display precision back from the display format string
// FIXME: This is still used by some navigation code path to infer a minimum tweak step, but we should aim to rework widgets so it isn't needed.
int ImParseFormatPrecision(const char* fmt, int default_precision)
{
    fmt = ImParseFormatFindStart(fmt);
    if (fmt[0] != '%')
        return default_precision;
    fmt++;
    while (*fmt >= '0' && *fmt <= '9')
        fmt++;
    int precision = INT_MAX;
    if (*fmt == '.')
    {
        fmt = ImAtoi<int>(fmt + 1, &precision);
        if (precision < 0 || precision > 99)
            precision = default_precision;
    }
    if (*fmt == 'e' || *fmt == 'E') // Maximum precision with scientific notation
        precision = -1;
    if ((*fmt == 'g' || *fmt == 'G') && precision == INT_MAX)
        precision = -1;
    return (precision == INT_MAX) ? default_precision : precision;
}

// Create text input in place of an active drag/slider (used when doing a CTRL+Click on drag/slider widgets)
// FIXME: Facilitate using this in variety of other situations.
bool ImGui::InputScalarAsWidgetReplacement(const ImRect& bb, ImGuiID id, const char* label, ImGuiDataType data_type, void* data_ptr, const char* format)
{
    IM_UNUSED(id);
    ImGuiContext& g = *GImGui;

    // On the first frame, g.ScalarAsInputTextId == 0, then on subsequent frames it becomes == id.
    // We clear ActiveID on the first frame to allow the InputText() taking it back.
    if (g.ScalarAsInputTextId == 0)
        ClearActiveID();

    char fmt_buf[32];
    char data_buf[32];
    format = ImParseFormatTrimDecorations(format, fmt_buf, IM_ARRAYSIZE(fmt_buf));
    DataTypeFormatString(data_buf, IM_ARRAYSIZE(data_buf), data_type, data_ptr, format);
    ImStrTrimBlanks(data_buf);
    ImGuiInputTextFlags flags = ImGuiInputTextFlags_AutoSelectAll | ((data_type == ImGuiDataType_Float || data_type == ImGuiDataType_Double) ? ImGuiInputTextFlags_CharsScientific : ImGuiInputTextFlags_CharsDecimal) | ImGuiInputTextFlags_OnSlider;
    bool value_changed = InputTextEx(label, data_buf, IM_ARRAYSIZE(data_buf), bb.GetSize(), flags);
    if (g.ScalarAsInputTextId == 0)
    {
        // First frame we started displaying the InputText widget, we expect it to take the active id.
        //IM_ASSERT(g.ActiveId == id);
        g.ScalarAsInputTextId = g.ActiveId;
    }
    if (value_changed)
        return DataTypeApplyOpFromText(data_buf, g.InputTextState.InitialTextA.Data, data_type, data_ptr, NULL);
    return false;
}

bool ImGui::InputScalar(const char* label, ImGuiDataType data_type, void* data_ptr, const void* step, const void* step_fast, const char* format, ImGuiInputTextFlags flags)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    ImGuiStyle& style = g.Style;

    IM_ASSERT(data_type >= 0 && data_type < ImGuiDataType_COUNT);
    if (format == NULL)
        format = GDataTypeInfo[data_type].PrintFmt;

    char buf[64];
    DataTypeFormatString(buf, IM_ARRAYSIZE(buf), data_type, data_ptr, format);

    bool value_changed = false;
    if ((flags & (ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsScientific)) == 0)
        flags |= ImGuiInputTextFlags_CharsDecimal;
    flags |= ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_OnInput;

    if (step != NULL)
    {
        const float button_size = GetFrameHeight() + 4;

        float offset_x = 17 + style.ItemInnerSpacing.x;
        const ImVec2 global_offset = ImVec2(offset_x, 0);

        const ImVec2 label_size = CalcTextSize(label);

        const ImVec2 size_wind_max = window->Size;
        const ImVec2 pos_wind = window->Pos;
        float offset_scroll = 0;
        if (window->ScrollbarY)
            offset_scroll = style.ScrollbarSize;

        ImVec2 pos_end = ImVec2(pos_wind.x + (window->Size.x - (offset_x * 2) + 9.f - offset_scroll), window->DC.CursorPos.y + 37.f);

        //ImRect size_for_input_text = ImRect(window->DC.CursorPos + global_offset + ImVec2(0, label_size.y + 2.f), ImVec2(pos_wind.x + (window->Size.x - (offset_x * 2) + 9.f - offset_scroll - 20.f), window->DC.CursorPos.y + 30.f));
        const ImRect size_for_input_text(window->DC.CursorPos + global_offset, ImVec2(pos_wind.x + (window->Size.x - (offset_x * 2) + 9.f - offset_scroll) - button_size - 3.f, window->DC.CursorPos.y) + ImVec2(0, label_size.y + style.FramePadding.y * 2.0f));

        const ImRect total_size(window->DC.CursorPos + global_offset + ImVec2(0, label_size.y + 2.f), pos_end);

        BeginGroup(); // The only purpose of the group here is to allow the caller to query item data e.g. IsItemActive()
        PushID(label);
        //PushItemWidth(ImMax(1.0f, CalcItemWidth() - (button_size + style.ItemInnerSpacing.x) * 2));
        if(InputTextEx(label, buf, IM_ARRAYSIZE(buf), size_for_input_text.GetSize(), flags))//if (InputText(label, buf, IM_ARRAYSIZE(buf), flags)) // PushId(label) + "" gives us the expected ID from outside point of viewvalue_changed = DataTypeApplyOpFromText(buf, g.InputTextState.InitialTextA.Data, data_type, data_ptr, format);
            value_changed = DataTypeApplyOpFromText(buf, g.InputTextState.InitialTextA.Data, data_type, data_ptr, format);
       // PopItemWidth();

        // Step buttons
        const ImVec2 backup_frame_padding = style.FramePadding;
        style.FramePadding.x = style.FramePadding.y;
        ImGuiButtonFlags button_flags = ImGuiButtonFlags_Repeat | ImGuiButtonFlags_DontClosePopups | ImGuiButtonFlags_noSpace | ImGuiButtonFlags_noPadY;
        if (flags & ImGuiInputTextFlags_ReadOnly)
            button_flags |= ImGuiButtonFlags_Disabled;

        window->DC.CursorPos = ImVec2(total_size.Max.x - button_size, total_size.Min.y);

        if (ButtonEx("+", ImVec2(button_size, 12.f), button_flags))
        {
            DataTypeApplyOp(data_type, '+', data_ptr, data_ptr, g.IO.KeyCtrl && step_fast ? step_fast : step);
            value_changed = true;
        }
        window->DC.CursorPos = ImVec2(total_size.Max.x - button_size, total_size.Min.y + 10.f + 3.f);
        if (ButtonEx("-", ImVec2(button_size, 12.f), button_flags))
        {
            DataTypeApplyOp(data_type, '-', data_ptr, data_ptr, g.IO.KeyCtrl && step_fast ? step_fast : step);
            value_changed = true;
        }
        //TextEx(label, FindRenderedTextEnd(label));
        style.FramePadding = backup_frame_padding;

        PopID();
        EndGroup();

        //window->DrawList->AddRectFilled(total_size.Min, total_size.Max, GetColorU32(ImVec4(1.f, 1.f, 1.f, 0.1f)));
    }
    else
    {
        if (InputText(label, buf, IM_ARRAYSIZE(buf), flags))
            value_changed = DataTypeApplyOpFromText(buf, g.InputTextState.InitialTextA.Data, data_type, data_ptr, format);
    }

    return value_changed;
}

bool ImGui::InputScalarN(const char* label, ImGuiDataType data_type, void* v, int components, const void* step, const void* step_fast, const char* format, ImGuiInputTextFlags flags)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    bool value_changed = false;
    BeginGroup();
    PushID(label);
    PushMultiItemsWidths(components);
    size_t type_size = GDataTypeInfo[data_type].Size;
    for (int i = 0; i < components; i++)
    {
        PushID(i);
        value_changed |= InputScalar("", data_type, v, step, step_fast, format, flags);
        SameLine(0, g.Style.ItemInnerSpacing.x);
        PopID();
        PopItemWidth();
        v = (void*)((char*)v + type_size);
    }
    PopID();

    TextUnformatted(label, FindRenderedTextEnd(label));
    EndGroup();
    return value_changed;
}

bool ImGui::InputFloat(const char* label, float* v, float step, float step_fast, const char* format, ImGuiInputTextFlags flags)
{
    flags |= ImGuiInputTextFlags_CharsScientific;
    return InputScalar(label, ImGuiDataType_Float, (void*)v, (void*)(step>0.0f ? &step : NULL), (void*)(step_fast>0.0f ? &step_fast : NULL), format, flags);
}

bool ImGui::InputFloat2(const char* label, float v[2], const char* format, ImGuiInputTextFlags flags)
{
    return InputScalarN(label, ImGuiDataType_Float, v, 2, NULL, NULL, format, flags);
}

bool ImGui::InputFloat3(const char* label, float v[3], const char* format, ImGuiInputTextFlags flags)
{
    return InputScalarN(label, ImGuiDataType_Float, v, 3, NULL, NULL, format, flags);
}

bool ImGui::InputFloat4(const char* label, float v[4], const char* format, ImGuiInputTextFlags flags)
{
    return InputScalarN(label, ImGuiDataType_Float, v, 4, NULL, NULL, format, flags);
}

// Prefer using "const char* format" directly, which is more flexible and consistent with other API.
#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
bool ImGui::InputFloat(const char* label, float* v, float step, float step_fast, int decimal_precision, ImGuiInputTextFlags flags)
{
    char format[16] = "%f";
    if (decimal_precision >= 0)
        ImFormatString(format, IM_ARRAYSIZE(format), "%%.%df", decimal_precision);
    return InputFloat(label, v, step, step_fast, format, flags);
}

bool ImGui::InputFloat2(const char* label, float v[2], int decimal_precision, ImGuiInputTextFlags flags)
{
    char format[16] = "%f";
    if (decimal_precision >= 0)
        ImFormatString(format, IM_ARRAYSIZE(format), "%%.%df", decimal_precision);
    return InputScalarN(label, ImGuiDataType_Float, v, 2, NULL, NULL, format, flags);
}

bool ImGui::InputFloat3(const char* label, float v[3], int decimal_precision, ImGuiInputTextFlags flags)
{
    char format[16] = "%f";
    if (decimal_precision >= 0)
        ImFormatString(format, IM_ARRAYSIZE(format), "%%.%df", decimal_precision);
    return InputScalarN(label, ImGuiDataType_Float, v, 3, NULL, NULL, format, flags);
}

bool ImGui::InputFloat4(const char* label, float v[4], int decimal_precision, ImGuiInputTextFlags flags)
{
    char format[16] = "%f";
    if (decimal_precision >= 0)
        ImFormatString(format, IM_ARRAYSIZE(format), "%%.%df", decimal_precision);
    return InputScalarN(label, ImGuiDataType_Float, v, 4, NULL, NULL, format, flags);
}
#endif // IMGUI_DISABLE_OBSOLETE_FUNCTIONS

bool ImGui::InputInt(const char* label, int* v, int step, int step_fast, ImGuiInputTextFlags flags)
{
    // Hexadecimal input provided as a convenience but the flag name is awkward. Typically you'd use InputText() to parse your own data, if you want to handle prefixes.
    const char* format = (flags & ImGuiInputTextFlags_CharsHexadecimal) ? "%08X" : "%d";
    return InputScalar(label, ImGuiDataType_S32, (void*)v, (void*)(step>0 ? &step : NULL), (void*)(step_fast>0 ? &step_fast : NULL), format, flags);
}

bool ImGui::InputInt2(const char* label, int v[2], ImGuiInputTextFlags flags)
{
    return InputScalarN(label, ImGuiDataType_S32, v, 2, NULL, NULL, "%d", flags);
}

bool ImGui::InputInt3(const char* label, int v[3], ImGuiInputTextFlags flags)
{
    return InputScalarN(label, ImGuiDataType_S32, v, 3, NULL, NULL, "%d", flags);
}

bool ImGui::InputInt4(const char* label, int v[4], ImGuiInputTextFlags flags)
{
    return InputScalarN(label, ImGuiDataType_S32, v, 4, NULL, NULL, "%d", flags);
}

bool ImGui::InputDouble(const char* label, double* v, double step, double step_fast, const char* format, ImGuiInputTextFlags flags)
{
    flags |= ImGuiInputTextFlags_CharsScientific;
    return InputScalar(label, ImGuiDataType_Double, (void*)v, (void*)(step>0.0 ? &step : NULL), (void*)(step_fast>0.0 ? &step_fast : NULL), format, flags);
}

//-------------------------------------------------------------------------
// [SECTION] Widgets: InputText, InputTextMultiline
//-------------------------------------------------------------------------
// - InputText()
// - InputTextMultiline()
// - InputTextEx() [Internal]
//-------------------------------------------------------------------------

bool ImGui::InputText(const char* label, char* buf, size_t buf_size, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data)
{
    IM_ASSERT(!(flags & ImGuiInputTextFlags_Multiline)); // call InputTextMultiline()
    return InputTextEx(label, buf, (int)buf_size, ImVec2(0,0), flags, callback, user_data);
}

bool ImGui::InputTextMultiline(const char* label, char* buf, size_t buf_size, const ImVec2& size, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data)
{
    return InputTextEx(label, buf, (int)buf_size, size, flags | ImGuiInputTextFlags_Multiline, callback, user_data);
}

static int InputTextCalcTextLenAndLineCount(const char* text_begin, const char** out_text_end)
{
    int line_count = 0;
    const char* s = text_begin;
    while (char c = *s++) // We are only matching for \n so we can ignore UTF-8 decoding
        if (c == '\n')
            line_count++;
    s--;
    if (s[0] != '\n' && s[0] != '\r')
        line_count++;
    *out_text_end = s;
    return line_count;
}

static ImVec2 InputTextCalcTextSizeW(const ImWchar* text_begin, const ImWchar* text_end, const ImWchar** remaining, ImVec2* out_offset, bool stop_on_new_line)
{
    ImGuiContext& g = *GImGui;
    ImFont* font = g.Font;
    const float line_height = g.FontSize;
    const float scale = line_height / font->FontSize;

    ImVec2 text_size = ImVec2(0,0);
    float line_width = 0.0f;

    const ImWchar* s = text_begin;
    while (s < text_end)
    {
        unsigned int c = (unsigned int)(*s++);
        if (c == '\n')
        {
            text_size.x = ImMax(text_size.x, line_width);
            text_size.y += line_height;
            line_width = 0.0f;
            if (stop_on_new_line)
                break;
            continue;
        }
        if (c == '\r')
            continue;

        const float char_width = font->GetCharAdvance((ImWchar)c) * scale;
        line_width += char_width;
    }

    if (text_size.x < line_width)
        text_size.x = line_width;

    if (out_offset)
        *out_offset = ImVec2(line_width, text_size.y + line_height);  // offset allow for the possibility of sitting after a trailing \n

    if (line_width > 0 || text_size.y == 0.0f)                        // whereas size.y will ignore the trailing \n
        text_size.y += line_height;

    if (remaining)
        *remaining = s;

    return text_size;
}

// Wrapper for stb_textedit.h to edit text (our wrapper is for: statically sized buffer, single-line, wchar characters. InputText converts between UTF-8 and wchar)
namespace ImStb
{

static int     STB_TEXTEDIT_STRINGLEN(const STB_TEXTEDIT_STRING* obj)                             { return obj->CurLenW; }
static ImWchar STB_TEXTEDIT_GETCHAR(const STB_TEXTEDIT_STRING* obj, int idx)                      { return obj->TextW[idx]; }
static float   STB_TEXTEDIT_GETWIDTH(STB_TEXTEDIT_STRING* obj, int line_start_idx, int char_idx)  { ImWchar c = obj->TextW[line_start_idx+char_idx]; if (c == '\n') return STB_TEXTEDIT_GETWIDTH_NEWLINE; return GImGui->Font->GetCharAdvance(c) * (GImGui->FontSize / GImGui->Font->FontSize); }
static int     STB_TEXTEDIT_KEYTOTEXT(int key)                                                    { return key >= 0x10000 ? 0 : key; }
static ImWchar STB_TEXTEDIT_NEWLINE = '\n';
static void    STB_TEXTEDIT_LAYOUTROW(StbTexteditRow* r, STB_TEXTEDIT_STRING* obj, int line_start_idx)
{
    const ImWchar* text = obj->TextW.Data;
    const ImWchar* text_remaining = NULL;
    const ImVec2 size = InputTextCalcTextSizeW(text + line_start_idx, text + obj->CurLenW, &text_remaining, NULL, true);
    r->x0 = 0.0f;
    r->x1 = size.x;
    r->baseline_y_delta = size.y;
    r->ymin = 0.0f;
    r->ymax = size.y;
    r->num_chars = (int)(text_remaining - (text + line_start_idx));
}

static bool is_separator(unsigned int c)                                        { return ImCharIsBlankW(c) || c==',' || c==';' || c=='(' || c==')' || c=='{' || c=='}' || c=='[' || c==']' || c=='|'; }
static int  is_word_boundary_from_right(STB_TEXTEDIT_STRING* obj, int idx)      { return idx > 0 ? (is_separator( obj->TextW[idx-1] ) && !is_separator( obj->TextW[idx] ) ) : 1; }
static int  STB_TEXTEDIT_MOVEWORDLEFT_IMPL(STB_TEXTEDIT_STRING* obj, int idx)   { idx--; while (idx >= 0 && !is_word_boundary_from_right(obj, idx)) idx--; return idx < 0 ? 0 : idx; }
#ifdef __APPLE__    // FIXME: Move setting to IO structure
static int  is_word_boundary_from_left(STB_TEXTEDIT_STRING* obj, int idx)       { return idx > 0 ? (!is_separator( obj->TextW[idx-1] ) && is_separator( obj->TextW[idx] ) ) : 1; }
static int  STB_TEXTEDIT_MOVEWORDRIGHT_IMPL(STB_TEXTEDIT_STRING* obj, int idx)  { idx++; int len = obj->CurLenW; while (idx < len && !is_word_boundary_from_left(obj, idx)) idx++; return idx > len ? len : idx; }
#else
static int  STB_TEXTEDIT_MOVEWORDRIGHT_IMPL(STB_TEXTEDIT_STRING* obj, int idx)  { idx++; int len = obj->CurLenW; while (idx < len && !is_word_boundary_from_right(obj, idx)) idx++; return idx > len ? len : idx; }
#endif
#define STB_TEXTEDIT_MOVEWORDLEFT   STB_TEXTEDIT_MOVEWORDLEFT_IMPL    // They need to be #define for stb_textedit.h
#define STB_TEXTEDIT_MOVEWORDRIGHT  STB_TEXTEDIT_MOVEWORDRIGHT_IMPL

static void STB_TEXTEDIT_DELETECHARS(STB_TEXTEDIT_STRING* obj, int pos, int n)
{
    ImWchar* dst = obj->TextW.Data + pos;

    // We maintain our buffer length in both UTF-8 and wchar formats
    obj->CurLenA -= ImTextCountUtf8BytesFromStr(dst, dst + n);
    obj->CurLenW -= n;

    // Offset remaining text (FIXME-OPT: Use memmove)
    const ImWchar* src = obj->TextW.Data + pos + n;
    while (ImWchar c = *src++)
        *dst++ = c;
    *dst = '\0';
}

static bool STB_TEXTEDIT_INSERTCHARS(STB_TEXTEDIT_STRING* obj, int pos, const ImWchar* new_text, int new_text_len)
{
    const bool is_resizable = (obj->UserFlags & ImGuiInputTextFlags_CallbackResize) != 0;
    const int text_len = obj->CurLenW;
    IM_ASSERT(pos <= text_len);

    const int new_text_len_utf8 = ImTextCountUtf8BytesFromStr(new_text, new_text + new_text_len);
    if (!is_resizable && (new_text_len_utf8 + obj->CurLenA + 1 > obj->BufCapacityA))
        return false;

    // Grow internal buffer if needed
    if (new_text_len + text_len + 1 > obj->TextW.Size)
    {
        if (!is_resizable)
            return false;
        IM_ASSERT(text_len < obj->TextW.Size);
        obj->TextW.resize(text_len + ImClamp(new_text_len * 4, 32, ImMax(256, new_text_len)) + 1);
    }

    ImWchar* text = obj->TextW.Data;
    if (pos != text_len)
        memmove(text + pos + new_text_len, text + pos, (size_t)(text_len - pos) * sizeof(ImWchar));
    memcpy(text + pos, new_text, (size_t)new_text_len * sizeof(ImWchar));

    obj->CurLenW += new_text_len;
    obj->CurLenA += new_text_len_utf8;
    obj->TextW[obj->CurLenW] = '\0';

    return true;
}

// We don't use an enum so we can build even with conflicting symbols (if another user of stb_textedit.h leak their STB_TEXTEDIT_K_* symbols)
#define STB_TEXTEDIT_K_LEFT         0x10000 // keyboard input to move cursor left
#define STB_TEXTEDIT_K_RIGHT        0x10001 // keyboard input to move cursor right
#define STB_TEXTEDIT_K_UP           0x10002 // keyboard input to move cursor up
#define STB_TEXTEDIT_K_DOWN         0x10003 // keyboard input to move cursor down
#define STB_TEXTEDIT_K_LINESTART    0x10004 // keyboard input to move cursor to start of line
#define STB_TEXTEDIT_K_LINEEND      0x10005 // keyboard input to move cursor to end of line
#define STB_TEXTEDIT_K_TEXTSTART    0x10006 // keyboard input to move cursor to start of text
#define STB_TEXTEDIT_K_TEXTEND      0x10007 // keyboard input to move cursor to end of text
#define STB_TEXTEDIT_K_DELETE       0x10008 // keyboard input to delete selection or character under cursor
#define STB_TEXTEDIT_K_BACKSPACE    0x10009 // keyboard input to delete selection or character left of cursor
#define STB_TEXTEDIT_K_UNDO         0x1000A // keyboard input to perform undo
#define STB_TEXTEDIT_K_REDO         0x1000B // keyboard input to perform redo
#define STB_TEXTEDIT_K_WORDLEFT     0x1000C // keyboard input to move cursor left one word
#define STB_TEXTEDIT_K_WORDRIGHT    0x1000D // keyboard input to move cursor right one word
#define STB_TEXTEDIT_K_SHIFT        0x20000

#define STB_TEXTEDIT_IMPLEMENTATION
#include "imstb_textedit.h"

}

void ImGuiInputTextState::OnKeyPressed(int key)
{
    stb_textedit_key(this, &Stb, key);
    CursorFollow = true;
    CursorAnimReset();
}

ImGuiInputTextCallbackData::ImGuiInputTextCallbackData()
{
    memset(this, 0, sizeof(*this));
}

// Public API to manipulate UTF-8 text
// We expose UTF-8 to the user (unlike the STB_TEXTEDIT_* functions which are manipulating wchar)
// FIXME: The existence of this rarely exercised code path is a bit of a nuisance.
void ImGuiInputTextCallbackData::DeleteChars(int pos, int bytes_count)
{
    IM_ASSERT(pos + bytes_count <= BufTextLen);
    char* dst = Buf + pos;
    const char* src = Buf + pos + bytes_count;
    while (char c = *src++)
        *dst++ = c;
    *dst = '\0';

    if (CursorPos + bytes_count >= pos)
        CursorPos -= bytes_count;
    else if (CursorPos >= pos)
        CursorPos = pos;
    SelectionStart = SelectionEnd = CursorPos;
    BufDirty = true;
    BufTextLen -= bytes_count;
}

void ImGuiInputTextCallbackData::InsertChars(int pos, const char* new_text, const char* new_text_end)
{
    const bool is_resizable = (Flags & ImGuiInputTextFlags_CallbackResize) != 0;
    const int new_text_len = new_text_end ? (int)(new_text_end - new_text) : (int)strlen(new_text);
    if (new_text_len + BufTextLen >= BufSize)
    {
        if (!is_resizable)
            return;

        // Contrary to STB_TEXTEDIT_INSERTCHARS() this is working in the UTF8 buffer, hence the midly similar code (until we remove the U16 buffer alltogether!)
        ImGuiContext& g = *GImGui;
        ImGuiInputTextState* edit_state = &g.InputTextState;
        IM_ASSERT(edit_state->ID != 0 && g.ActiveId == edit_state->ID);
        IM_ASSERT(Buf == edit_state->TextA.Data);
        int new_buf_size = BufTextLen + ImClamp(new_text_len * 4, 32, ImMax(256, new_text_len)) + 1;
        edit_state->TextA.reserve(new_buf_size + 1);
        Buf = edit_state->TextA.Data;
        BufSize = edit_state->BufCapacityA = new_buf_size;
    }

    if (BufTextLen != pos)
        memmove(Buf + pos + new_text_len, Buf + pos, (size_t)(BufTextLen - pos));
    memcpy(Buf + pos, new_text, (size_t)new_text_len * sizeof(char));
    Buf[BufTextLen + new_text_len] = '\0';

    if (CursorPos >= pos)
        CursorPos += new_text_len;
    SelectionStart = SelectionEnd = CursorPos;
    BufDirty = true;
    BufTextLen += new_text_len;
}

// Return false to discard a character.
static bool InputTextFilterCharacter(unsigned int* p_char, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data)
{
    unsigned int c = *p_char;

    if (c < 128 && c != ' ' && !isprint((int)(c & 0xFF)))
    {
        bool pass = false;
        pass |= (c == '\n' && (flags & ImGuiInputTextFlags_Multiline));
        pass |= (c == '\t' && (flags & ImGuiInputTextFlags_AllowTabInput));
        if (!pass)
            return false;
    }

    if (c >= 0xE000 && c <= 0xF8FF) // Filter private Unicode range. I don't imagine anybody would want to input them. GLFW on OSX seems to send private characters for special keys like arrow keys.
        return false;

    if (flags & (ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsUppercase | ImGuiInputTextFlags_CharsNoBlank | ImGuiInputTextFlags_CharsScientific))
    {
        if (flags & ImGuiInputTextFlags_CharsDecimal)
            if (!(c >= '0' && c <= '9') && (c != '.') && (c != '-') && (c != '+') && (c != '*') && (c != '/'))
                return false;

        if (flags & ImGuiInputTextFlags_CharsScientific)
            if (!(c >= '0' && c <= '9') && (c != '.') && (c != '-') && (c != '+') && (c != '*') && (c != '/') && (c != 'e') && (c != 'E'))
                return false;

        if (flags & ImGuiInputTextFlags_CharsHexadecimal)
            if (!(c >= '0' && c <= '9') && !(c >= 'a' && c <= 'f') && !(c >= 'A' && c <= 'F'))
                return false;

        if (flags & ImGuiInputTextFlags_CharsUppercase)
            if (c >= 'a' && c <= 'z')
                *p_char = (c += (unsigned int)('A'-'a'));

        if (flags & ImGuiInputTextFlags_CharsNoBlank)
            if (ImCharIsBlankW(c))
                return false;
    }

    if (flags & ImGuiInputTextFlags_CallbackCharFilter)
    {
        ImGuiInputTextCallbackData callback_data;
        memset(&callback_data, 0, sizeof(ImGuiInputTextCallbackData));
        callback_data.EventFlag = ImGuiInputTextFlags_CallbackCharFilter;
        callback_data.EventChar = (ImWchar)c;
        callback_data.Flags = flags;
        callback_data.UserData = user_data;
        if (callback(&callback_data) != 0)
            return false;
        *p_char = callback_data.EventChar;
        if (!callback_data.EventChar)
            return false;
    }

    return true;
}

// Edit a string of text
// - buf_size account for the zero-terminator, so a buf_size of 6 can hold "Hello" but not "Hello!".
//   This is so we can easily call InputText() on static arrays using ARRAYSIZE() and to match
//   Note that in std::string world, capacity() would omit 1 byte used by the zero-terminator.
// - When active, hold on a privately held copy of the text (and apply back to 'buf'). So changing 'buf' while the InputText is active has no effect.
// - If you want to use ImGui::InputText() with std::string, see misc/cpp/imgui_stdlib.h
// (FIXME: Rather confusing and messy function, among the worse part of our codebase, expecting to rewrite a V2 at some point.. Partly because we are
//  doing UTF8 > U16 > UTF8 conversions on the go to easily interface with stb_textedit. Ideally should stay in UTF-8 all the time. See https://github.com/nothings/stb/issues/188)
bool ImGui::InputTextEx(const char* label, char* buf, int buf_size, const ImVec2& size_arg, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* callback_user_data)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    IM_ASSERT(!((flags & ImGuiInputTextFlags_CallbackHistory) && (flags & ImGuiInputTextFlags_Multiline)));        // Can't use both together (they both use up/down keys)
    IM_ASSERT(!((flags & ImGuiInputTextFlags_CallbackCompletion) && (flags & ImGuiInputTextFlags_AllowTabInput))); // Can't use both together (they both use tab key)

    ImGuiContext& g = *GImGui;
    ImGuiIO& io = g.IO;
    const ImGuiStyle& style = g.Style;

    const bool RENDER_SELECTION_WHEN_INACTIVE = true;
    const bool is_multiline = (flags & ImGuiInputTextFlags_Multiline) != 0;
    const bool is_readonly = (flags & ImGuiInputTextFlags_ReadOnly) != 0;
    const bool is_password = (flags & ImGuiInputTextFlags_Password) != 0;
    const bool is_undoable = (flags & ImGuiInputTextFlags_NoUndoRedo) == 0;
    const bool is_resizable = (flags & ImGuiInputTextFlags_CallbackResize) != 0;
    if (is_resizable)
        IM_ASSERT(callback != NULL); // Must provide a callback if you set the ImGuiInputTextFlags_CallbackResize flag!

    if (is_multiline) // Open group before calling GetID() because groups tracks id created within their scope,
        BeginGroup();



    const ImVec2 global_offset = ImVec2(23, 0);

    const ImGuiID id = window->GetID(label);
    ImVec2 label_size = CalcTextSize(label, NULL, true);
    ImVec2 size = CalcItemSize(size_arg, CalcItemWidth(), (is_multiline ? GetTextLineHeight() * 8.0f : label_size.y) + style.FramePadding.y * 2.0f); // Arbitrary default of 8 lines high for multi-line
    /*const ImRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + size);
    const ImRect total_bb(frame_bb.Min, frame_bb.Max + ImVec2(label_size.x > 0.0f ? (style.ItemInnerSpacing.x + label_size.x) : 0.0f, 0.0f));*/

    const float rainbow_bar_size = 2.f;

    const ImVec2 frame_size = CalcItemSize(size_arg, CalcItemWidth(), (is_multiline ? g.FontSize * 8.0f : label_size.y) + style.FramePadding.y * 2.0f); // Arbitrary default of 8 lines high for multi-line
    float offset_y = 5.f;
    float offset_x = 17 + style.ItemInnerSpacing.x;
    ImVec2 offset = ImVec2(offset_x, 0.f);

    const ImRect frame_bb(window->DC.CursorPos + offset, window->DC.CursorPos + size + ImVec2(0, offset_y) + offset);
    ImRect frame_bb_new;

    if (flags & ImGuiInputTextFlags_OnSlider || flags & ImGuiInputTextFlags_OnInput)
    {
        frame_bb_new = ImRect(window->DC.CursorPos + ImVec2(0, label_size.y + 2.f) + offset, window->DC.CursorPos + size + ImVec2(0, offset_y) + ImVec2(0, label_size.y + 2.f) + offset);
    }
    else
    {
        const ImVec2 pos_wind = window->Pos;
        float offset_scroll = 0;
        if (window->ScrollbarY)
            offset_scroll = style.ScrollbarSize;

        ImVec2 pos_end = ImVec2(pos_wind.x + (window->Size.x - (offset_x * 2) + 9.f - offset_scroll), window->DC.CursorPos.y);

        frame_bb_new = ImRect(window->DC.CursorPos + ImVec2(0, label_size.y + 2.f) + offset, pos_end + ImVec2(0, offset_y) + ImVec2(0, (label_size.y + 2.f) + size.y));
    }

    const ImRect total_bb(frame_bb.Min, frame_bb_new.Max + ImVec2(label_size.x > 0.0f ? (style.ItemInnerSpacing.x + label_size.x) : 0.0f, 0.0f));

    ImGuiWindow* draw_window = window;
    if (is_multiline)
    {
        if (!ItemAdd(total_bb, id, &frame_bb))
        {
            ItemSize(total_bb, style.FramePadding.y);
            EndGroup();
            return false;
        }
        if (!BeginChildFrame(id, frame_bb.GetSize()))
        {
            EndChildFrame();
            EndGroup();
            return false;
        }
        draw_window = GetCurrentWindow();
        draw_window->DC.NavLayerActiveMaskNext |= draw_window->DC.NavLayerCurrentMask; // This is to ensure that EndChild() will display a navigation highlight
        size.x -= draw_window->ScrollbarSizes.x;
    }
    else
    {
        ItemSize(total_bb, style.FramePadding.y);
        if (!ItemAdd(total_bb, id, &frame_bb_new))
            return false;
    }
    const bool hovered_old = ItemHoverable(frame_bb, id);
    const bool hovered = ItemHoverable(frame_bb_new, id);
    if (hovered)
        g.MouseCursor = ImGuiMouseCursor_TextInput;

    // Password pushes a temporary font with only a fallback glyph
    if (is_password)
    {
        const ImFontGlyph* glyph = g.Font->FindGlyph('*');
        ImFont* password_font = &g.InputTextPasswordFont;
        password_font->FontSize = g.Font->FontSize;
        password_font->Scale = g.Font->Scale;
        password_font->DisplayOffset = g.Font->DisplayOffset;
        password_font->Ascent = g.Font->Ascent;
        password_font->Descent = g.Font->Descent;
        password_font->ContainerAtlas = g.Font->ContainerAtlas;
        password_font->FallbackGlyph = glyph;
        password_font->FallbackAdvanceX = glyph->AdvanceX;
        IM_ASSERT(password_font->Glyphs.empty() && password_font->IndexAdvanceX.empty() && password_font->IndexLookup.empty());
        PushFont(password_font);
    }

    // NB: we are only allowed to access 'edit_state' if we are the active widget.
    ImGuiInputTextState* state = NULL;
    if (g.InputTextState.ID == id)
        state = &g.InputTextState;

    const bool focus_requested = FocusableItemRegister(window, id, (flags & (ImGuiInputTextFlags_CallbackCompletion|ImGuiInputTextFlags_AllowTabInput)) == 0);    // Using completion callback disable keyboard tabbing
    const bool focus_requested_by_code = focus_requested && (window->FocusIdxAllCounter == window->FocusIdxAllRequestCurrent);
    const bool focus_requested_by_tab = focus_requested && !focus_requested_by_code;

    const bool user_clicked= ( flags & ImGuiInputTextFlags_OnSlider ? hovered_old || hovered : hovered) && io.MouseClicked[0];
    const bool user_nav_input_start = (g.ActiveId != id) && ((g.NavInputId == id) || (g.NavActivateId == id && g.NavInputSource == ImGuiInputSource_NavKeyboard));
    const bool user_scroll_finish = is_multiline && state != NULL && g.ActiveId == 0 && g.ActiveIdPreviousFrame == GetScrollbarID(draw_window, ImGuiAxis_Y);
    const bool user_scroll_active = is_multiline && state != NULL && g.ActiveId == GetScrollbarID(draw_window, ImGuiAxis_Y);

    bool clear_active_id = false;
    bool select_all = (g.ActiveId != id) && ((flags & ImGuiInputTextFlags_AutoSelectAll) != 0 || user_nav_input_start) && (!is_multiline);

    const bool init_make_active = (focus_requested || user_clicked  || user_scroll_finish || user_nav_input_start);
    if (init_make_active && g.ActiveId != id)
    {
        // Access state even if we don't own it yet.
        state = &g.InputTextState;
        state->CursorAnimReset();

        // Take a copy of the initial buffer value (both in original UTF-8 format and converted to wchar)
        // From the moment we focused we are ignoring the content of 'buf' (unless we are in read-only mode)
        const int buf_len = (int)strlen(buf);
        state->InitialTextA.resize(buf_len + 1);    // UTF-8. we use +1 to make sure that .Data is always pointing to at least an empty string.
        memcpy(state->InitialTextA.Data, buf, buf_len + 1);

        // Start edition
        const int prev_len_w = state->CurLenW;
        const char* buf_end = NULL;
        state->TextW.resize(buf_size + 1);          // wchar count <= UTF-8 count. we use +1 to make sure that .Data is always pointing to at least an empty string.
        state->CurLenW = ImTextStrFromUtf8(state->TextW.Data, buf_size, buf, NULL, &buf_end);
        state->CurLenA = (int)(buf_end - buf);      // We can't get the result from ImStrncpy() above because it is not UTF-8 aware. Here we'll cut off malformed UTF-8.

        // Preserve cursor position and undo/redo stack if we come back to same widget
        // FIXME: We should probably compare the whole buffer to be on the safety side. Comparing buf (utf8) and edit_state.Text (wchar).
        const bool recycle_state = (state->ID == id) && (prev_len_w == state->CurLenW);
        if (recycle_state)
        {
            // Recycle existing cursor/selection/undo stack but clamp position
            // Note a single mouse click will override the cursor/position immediately by calling stb_textedit_click handler.
            state->CursorClamp();
        }
        else
        {
            state->ID = id;
            state->ScrollX = 0.0f;
            stb_textedit_initialize_state(&state->Stb, !is_multiline);
            if (!is_multiline && focus_requested_by_code)
                select_all = true;
        }
        if (flags & ImGuiInputTextFlags_AlwaysInsertMode)
            state->Stb.insert_mode = 1;
        if (!is_multiline && (focus_requested_by_tab || (user_clicked && io.KeyCtrl)))
            select_all = true;
    }

    if (init_make_active)
    {
        IM_ASSERT(state && state->ID == id);
        SetActiveID(id, window);
        SetFocusID(id, window);
        FocusWindow(window);
        g.ActiveIdBlockNavInputFlags = (1 << ImGuiNavInput_Cancel);
        if (!is_multiline && !(flags & ImGuiInputTextFlags_CallbackHistory))
            g.ActiveIdAllowNavDirFlags = ((1 << ImGuiDir_Up) | (1 << ImGuiDir_Down));
    }

    // Release focus when we click outside
    if (!init_make_active && io.MouseClicked[0] && !(flags & ImGuiInputTextFlags_OnSlider))
        clear_active_id = true;

    // We have an edge case if ActiveId was set through another widget (e.g. widget being swapped)
    if (g.ActiveId == id && state == NULL)
        ClearActiveID();

    bool value_changed = false;
    bool enter_pressed = false;
    int backup_current_text_length = 0;

    // Process mouse inputs and character inputs
    if (g.ActiveId == id)
    {
        IM_ASSERT(state != NULL);
        if (is_readonly && !g.ActiveIdIsJustActivated)
        {
            // When read-only we always use the live data passed to the function
            const char* buf_end = NULL;
            state->TextW.resize(buf_size+1);
            state->CurLenW = ImTextStrFromUtf8(state->TextW.Data, state->TextW.Size, buf, NULL, &buf_end);
            state->CurLenA = (int)(buf_end - buf);
            state->CursorClamp();
        }

        backup_current_text_length = state->CurLenA;
        state->BufCapacityA = buf_size;
        state->UserFlags = flags;
        state->UserCallback = callback;
        state->UserCallbackData = callback_user_data;

        // Although we are active we don't prevent mouse from hovering other elements unless we are interacting right now with the widget.
        // Down the line we should have a cleaner library-wide concept of Selected vs Active.
        g.ActiveIdAllowOverlap = !io.MouseDown[0];
        g.WantTextInputNextFrame = 1;

        // Edit in progress
        const float mouse_x = (io.MousePos.x - frame_bb_new.Min.x - style.FramePadding.x) + state->ScrollX;
        const float mouse_y = (is_multiline ? (io.MousePos.y - draw_window->DC.CursorPos.y - style.FramePadding.y) : (g.FontSize*0.5f));

        const bool is_osx = io.ConfigMacOSXBehaviors;
        if (select_all || (hovered && !is_osx && io.MouseDoubleClicked[0]))
        {
            state->SelectAll();
            state->SelectedAllMouseLock = true;
        }
        else if (hovered && is_osx && io.MouseDoubleClicked[0])
        {
            // Double-click select a word only, OS X style (by simulating keystrokes)
            state->OnKeyPressed(STB_TEXTEDIT_K_WORDLEFT);
            state->OnKeyPressed(STB_TEXTEDIT_K_WORDRIGHT | STB_TEXTEDIT_K_SHIFT);
        }
        else if (io.MouseClicked[0] && !state->SelectedAllMouseLock)
        {
            if (hovered)
            {
                stb_textedit_click(state, &state->Stb, mouse_x, mouse_y);
                state->CursorAnimReset();
            }
        }
        else if (io.MouseDown[0] && !state->SelectedAllMouseLock && (io.MouseDelta.x != 0.0f || io.MouseDelta.y != 0.0f))
        {
            stb_textedit_drag(state, &state->Stb, mouse_x, mouse_y);
            state->CursorAnimReset();
            state->CursorFollow = true;
        }
        if (state->SelectedAllMouseLock && !io.MouseDown[0])
            state->SelectedAllMouseLock = false;

        if (io.InputQueueCharacters.Size > 0)
        {
            // Process text input (before we check for Return because using some IME will effectively send a Return?)
            // We ignore CTRL inputs, but need to allow ALT+CTRL as some keyboards (e.g. German) use AltGR (which _is_ Alt+Ctrl) to input certain characters.
            bool ignore_inputs = (io.KeyCtrl && !io.KeyAlt) || (is_osx && io.KeySuper);
            if (!ignore_inputs && !is_readonly && !user_nav_input_start)
                for (int n = 0; n < io.InputQueueCharacters.Size; n++)
                {
                    // Insert character if they pass filtering
                    unsigned int c = (unsigned int)io.InputQueueCharacters[n];
                    if (InputTextFilterCharacter(&c, flags, callback, callback_user_data))
                        state->OnKeyPressed((int)c);
                }

            // Consume characters
            io.InputQueueCharacters.resize(0);
        }
    }

    // Process other shortcuts/key-presses
    bool cancel_edit = false;
    if (g.ActiveId == id && !g.ActiveIdIsJustActivated && !clear_active_id)
    {
        IM_ASSERT(state != NULL);
        const int k_mask = (io.KeyShift ? STB_TEXTEDIT_K_SHIFT : 0);
        const bool is_osx = io.ConfigMacOSXBehaviors;
        const bool is_shortcut_key = (is_osx ? (io.KeySuper && !io.KeyCtrl) : (io.KeyCtrl && !io.KeySuper)) && !io.KeyAlt && !io.KeyShift; // OS X style: Shortcuts using Cmd/Super instead of Ctrl
        const bool is_osx_shift_shortcut = is_osx && io.KeySuper && io.KeyShift && !io.KeyCtrl && !io.KeyAlt;
        const bool is_wordmove_key_down = is_osx ? io.KeyAlt : io.KeyCtrl;                     // OS X style: Text editing cursor movement using Alt instead of Ctrl
        const bool is_startend_key_down = is_osx && io.KeySuper && !io.KeyCtrl && !io.KeyAlt;  // OS X style: Line/Text Start and End using Cmd+Arrows instead of Home/End
        const bool is_ctrl_key_only = io.KeyCtrl && !io.KeyShift && !io.KeyAlt && !io.KeySuper;
        const bool is_shift_key_only = io.KeyShift && !io.KeyCtrl && !io.KeyAlt && !io.KeySuper;

        const bool is_cut   = ((is_shortcut_key && IsKeyPressedMap(ImGuiKey_X)) || (is_shift_key_only && IsKeyPressedMap(ImGuiKey_Delete))) && !is_readonly && !is_password && (!is_multiline || state->HasSelection());
        const bool is_copy  = ((is_shortcut_key && IsKeyPressedMap(ImGuiKey_C)) || (is_ctrl_key_only  && IsKeyPressedMap(ImGuiKey_Insert))) && !is_password && (!is_multiline || state->HasSelection());
        const bool is_paste = ((is_shortcut_key && IsKeyPressedMap(ImGuiKey_V)) || (is_shift_key_only && IsKeyPressedMap(ImGuiKey_Insert))) && !is_readonly;
        const bool is_undo  = ((is_shortcut_key && IsKeyPressedMap(ImGuiKey_Z)) && !is_readonly && is_undoable);
        const bool is_redo  = ((is_shortcut_key && IsKeyPressedMap(ImGuiKey_Y)) || (is_osx_shift_shortcut && IsKeyPressedMap(ImGuiKey_Z))) && !is_readonly && is_undoable;

        if (IsKeyPressedMap(ImGuiKey_LeftArrow))                        { state->OnKeyPressed((is_startend_key_down ? STB_TEXTEDIT_K_LINESTART : is_wordmove_key_down ? STB_TEXTEDIT_K_WORDLEFT : STB_TEXTEDIT_K_LEFT) | k_mask); }
        else if (IsKeyPressedMap(ImGuiKey_RightArrow))                  { state->OnKeyPressed((is_startend_key_down ? STB_TEXTEDIT_K_LINEEND : is_wordmove_key_down ? STB_TEXTEDIT_K_WORDRIGHT : STB_TEXTEDIT_K_RIGHT) | k_mask); }
        else if (IsKeyPressedMap(ImGuiKey_UpArrow) && is_multiline)     { if (io.KeyCtrl) SetWindowScrollY(draw_window, ImMax(draw_window->Scroll.y - g.FontSize, 0.0f)); else state->OnKeyPressed((is_startend_key_down ? STB_TEXTEDIT_K_TEXTSTART : STB_TEXTEDIT_K_UP) | k_mask); }
        else if (IsKeyPressedMap(ImGuiKey_DownArrow) && is_multiline)   { if (io.KeyCtrl) SetWindowScrollY(draw_window, ImMin(draw_window->Scroll.y + g.FontSize, GetScrollMaxY())); else state->OnKeyPressed((is_startend_key_down ? STB_TEXTEDIT_K_TEXTEND : STB_TEXTEDIT_K_DOWN) | k_mask); }
        else if (IsKeyPressedMap(ImGuiKey_Home))                        { state->OnKeyPressed(io.KeyCtrl ? STB_TEXTEDIT_K_TEXTSTART | k_mask : STB_TEXTEDIT_K_LINESTART | k_mask); }
        else if (IsKeyPressedMap(ImGuiKey_End))                         { state->OnKeyPressed(io.KeyCtrl ? STB_TEXTEDIT_K_TEXTEND | k_mask : STB_TEXTEDIT_K_LINEEND | k_mask); }
        else if (IsKeyPressedMap(ImGuiKey_Delete) && !is_readonly)      { state->OnKeyPressed(STB_TEXTEDIT_K_DELETE | k_mask); }
        else if (IsKeyPressedMap(ImGuiKey_Backspace) && !is_readonly)
        {
            if (!state->HasSelection())
            {
                if (is_wordmove_key_down) 
                    state->OnKeyPressed(STB_TEXTEDIT_K_WORDLEFT|STB_TEXTEDIT_K_SHIFT);
                else if (is_osx && io.KeySuper && !io.KeyAlt && !io.KeyCtrl) 
                    state->OnKeyPressed(STB_TEXTEDIT_K_LINESTART|STB_TEXTEDIT_K_SHIFT);
            }
            state->OnKeyPressed(STB_TEXTEDIT_K_BACKSPACE | k_mask);
        }
        else if (IsKeyPressedMap(ImGuiKey_Enter))
        {
            bool ctrl_enter_for_new_line = (flags & ImGuiInputTextFlags_CtrlEnterForNewLine) != 0;
            if (!is_multiline || (ctrl_enter_for_new_line && !io.KeyCtrl) || (!ctrl_enter_for_new_line && io.KeyCtrl))
            {
                enter_pressed = clear_active_id = true;
            }
            else if (!is_readonly)
            {
                unsigned int c = '\n'; // Insert new line
                if (InputTextFilterCharacter(&c, flags, callback, callback_user_data))
                    state->OnKeyPressed((int)c);
            }
        }
        else if ((flags & ImGuiInputTextFlags_AllowTabInput) && IsKeyPressedMap(ImGuiKey_Tab) && !io.KeyCtrl && !io.KeyShift && !io.KeyAlt && !is_readonly)
        {
            unsigned int c = '\t'; // Insert TAB
            if (InputTextFilterCharacter(&c, flags, callback, callback_user_data))
                state->OnKeyPressed((int)c);
        }
        else if (IsKeyPressedMap(ImGuiKey_Escape))
        {
            clear_active_id = cancel_edit = true;
        }
        else if (is_undo || is_redo)
        {
            state->OnKeyPressed(is_undo ? STB_TEXTEDIT_K_UNDO : STB_TEXTEDIT_K_REDO);
            state->ClearSelection();
        }
        else if (is_shortcut_key && IsKeyPressedMap(ImGuiKey_A))
        {
            state->SelectAll();
            state->CursorFollow = true;
        }
        else if (is_cut || is_copy)
        {
            // Cut, Copy
            if (io.SetClipboardTextFn)
            {
                const int ib = state->HasSelection() ? ImMin(state->Stb.select_start, state->Stb.select_end) : 0;
                const int ie = state->HasSelection() ? ImMax(state->Stb.select_start, state->Stb.select_end) : state->CurLenW;
                const int clipboard_data_len = ImTextCountUtf8BytesFromStr(state->TextW.Data + ib, state->TextW.Data + ie) + 1;
                char* clipboard_data = (char*)MemAlloc(clipboard_data_len * sizeof(char));
                ImTextStrToUtf8(clipboard_data, clipboard_data_len, state->TextW.Data + ib, state->TextW.Data + ie);
                SetClipboardText(clipboard_data);
                MemFree(clipboard_data);
            }
            if (is_cut)
            {
                if (!state->HasSelection())
                    state->SelectAll();
                state->CursorFollow = true;
                stb_textedit_cut(state, &state->Stb);
            }
        }
        else if (is_paste)
        {
            if (const char* clipboard = GetClipboardText())
            {
                // Filter pasted buffer
                const int clipboard_len = (int)strlen(clipboard);
                ImWchar* clipboard_filtered = (ImWchar*)MemAlloc((clipboard_len+1) * sizeof(ImWchar));
                int clipboard_filtered_len = 0;
                for (const char* s = clipboard; *s; )
                {
                    unsigned int c;
                    s += ImTextCharFromUtf8(&c, s, NULL);
                    if (c == 0)
                        break;
                    if (c >= 0x10000 || !InputTextFilterCharacter(&c, flags, callback, callback_user_data))
                        continue;
                    clipboard_filtered[clipboard_filtered_len++] = (ImWchar)c;
                }
                clipboard_filtered[clipboard_filtered_len] = 0;
                if (clipboard_filtered_len > 0) // If everything was filtered, ignore the pasting operation
                {
                    stb_textedit_paste(state, &state->Stb, clipboard_filtered, clipboard_filtered_len);
                    state->CursorFollow = true;
                }
                MemFree(clipboard_filtered);
            }
        }
    }

    // Process callbacks and apply result back to user's buffer.
    if (g.ActiveId == id)
    {
        IM_ASSERT(state != NULL);
        const char* apply_new_text = NULL;
        int apply_new_text_length = 0;
        if (cancel_edit)
        {
            // Restore initial value. Only return true if restoring to the initial value changes the current buffer contents.
            if (!is_readonly && strcmp(buf, state->InitialTextA.Data) != 0)
            {
                apply_new_text = state->InitialTextA.Data;
                apply_new_text_length = state->InitialTextA.Size - 1;
            }
        }

        // When using 'ImGuiInputTextFlags_EnterReturnsTrue' as a special case we reapply the live buffer back to the input buffer before clearing ActiveId, even though strictly speaking it wasn't modified on this frame.
        // If we didn't do that, code like InputInt() with ImGuiInputTextFlags_EnterReturnsTrue would fail. Also this allows the user to use InputText() with ImGuiInputTextFlags_EnterReturnsTrue without maintaining any user-side storage.
        bool apply_edit_back_to_user_buffer = !cancel_edit || (enter_pressed && (flags & ImGuiInputTextFlags_EnterReturnsTrue) != 0);
        if (apply_edit_back_to_user_buffer)
        {
            // Apply new value immediately - copy modified buffer back
            // Note that as soon as the input box is active, the in-widget value gets priority over any underlying modification of the input buffer
            // FIXME: We actually always render 'buf' when calling DrawList->AddText, making the comment above incorrect.
            // FIXME-OPT: CPU waste to do this every time the widget is active, should mark dirty state from the stb_textedit callbacks.
            if (!is_readonly)
            {
                state->TextA.resize(state->TextW.Size * 4 + 1);
                ImTextStrToUtf8(state->TextA.Data, state->TextA.Size, state->TextW.Data, NULL);
            }

            // User callback
            if ((flags & (ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory | ImGuiInputTextFlags_CallbackAlways)) != 0)
            {
                IM_ASSERT(callback != NULL);

                // The reason we specify the usage semantic (Completion/History) is that Completion needs to disable keyboard TABBING at the moment.
                ImGuiInputTextFlags event_flag = 0;
                ImGuiKey event_key = ImGuiKey_COUNT;
                if ((flags & ImGuiInputTextFlags_CallbackCompletion) != 0 && IsKeyPressedMap(ImGuiKey_Tab))
                {
                    event_flag = ImGuiInputTextFlags_CallbackCompletion;
                    event_key = ImGuiKey_Tab;
                }
                else if ((flags & ImGuiInputTextFlags_CallbackHistory) != 0 && IsKeyPressedMap(ImGuiKey_UpArrow))
                {
                    event_flag = ImGuiInputTextFlags_CallbackHistory;
                    event_key = ImGuiKey_UpArrow;
                }
                else if ((flags & ImGuiInputTextFlags_CallbackHistory) != 0 && IsKeyPressedMap(ImGuiKey_DownArrow))
                {
                    event_flag = ImGuiInputTextFlags_CallbackHistory;
                    event_key = ImGuiKey_DownArrow;
                }
                else if (flags & ImGuiInputTextFlags_CallbackAlways)
                    event_flag = ImGuiInputTextFlags_CallbackAlways;

                if (event_flag)
                {
                    ImGuiInputTextCallbackData callback_data;
                    memset(&callback_data, 0, sizeof(ImGuiInputTextCallbackData));
                    callback_data.EventFlag = event_flag;
                    callback_data.Flags = flags;
                    callback_data.UserData = callback_user_data;

                    callback_data.EventKey = event_key;
                    callback_data.Buf = state->TextA.Data;
                    callback_data.BufTextLen = state->CurLenA;
                    callback_data.BufSize = state->BufCapacityA;
                    callback_data.BufDirty = false;

                    // We have to convert from wchar-positions to UTF-8-positions, which can be pretty slow (an incentive to ditch the ImWchar buffer, see https://github.com/nothings/stb/issues/188)
                    ImWchar* text = state->TextW.Data;
                    const int utf8_cursor_pos = callback_data.CursorPos = ImTextCountUtf8BytesFromStr(text, text + state->Stb.cursor);
                    const int utf8_selection_start = callback_data.SelectionStart = ImTextCountUtf8BytesFromStr(text, text + state->Stb.select_start);
                    const int utf8_selection_end = callback_data.SelectionEnd = ImTextCountUtf8BytesFromStr(text, text + state->Stb.select_end);

                    // Call user code
                    callback(&callback_data);

                    // Read back what user may have modified
                    IM_ASSERT(callback_data.Buf == state->TextA.Data);  // Invalid to modify those fields
                    IM_ASSERT(callback_data.BufSize == state->BufCapacityA);
                    IM_ASSERT(callback_data.Flags == flags);
                    if (callback_data.CursorPos != utf8_cursor_pos)            { state->Stb.cursor = ImTextCountCharsFromUtf8(callback_data.Buf, callback_data.Buf + callback_data.CursorPos); state->CursorFollow = true; }
                    if (callback_data.SelectionStart != utf8_selection_start)  { state->Stb.select_start = ImTextCountCharsFromUtf8(callback_data.Buf, callback_data.Buf + callback_data.SelectionStart); }
                    if (callback_data.SelectionEnd != utf8_selection_end)      { state->Stb.select_end = ImTextCountCharsFromUtf8(callback_data.Buf, callback_data.Buf + callback_data.SelectionEnd); }
                    if (callback_data.BufDirty)
                    {
                        IM_ASSERT(callback_data.BufTextLen == (int)strlen(callback_data.Buf)); // You need to maintain BufTextLen if you change the text!
                        if (callback_data.BufTextLen > backup_current_text_length && is_resizable)
                            state->TextW.resize(state->TextW.Size + (callback_data.BufTextLen - backup_current_text_length));
                        state->CurLenW = ImTextStrFromUtf8(state->TextW.Data, state->TextW.Size, callback_data.Buf, NULL);
                        state->CurLenA = callback_data.BufTextLen;  // Assume correct length and valid UTF-8 from user, saves us an extra strlen()
                        state->CursorAnimReset();
                    }
                }
            }

            // Will copy result string if modified
            if (!is_readonly && strcmp(state->TextA.Data, buf) != 0)
            {
                apply_new_text = state->TextA.Data;
                apply_new_text_length = state->CurLenA;
            }
        }

        // Copy result to user buffer
        if (apply_new_text)
        {
            IM_ASSERT(apply_new_text_length >= 0);
            if (backup_current_text_length != apply_new_text_length && is_resizable)
            {
                ImGuiInputTextCallbackData callback_data;
                callback_data.EventFlag = ImGuiInputTextFlags_CallbackResize;
                callback_data.Flags = flags;
                callback_data.Buf = buf;
                callback_data.BufTextLen = apply_new_text_length;
                callback_data.BufSize = ImMax(buf_size, apply_new_text_length + 1);
                callback_data.UserData = callback_user_data;
                callback(&callback_data);
                buf = callback_data.Buf;
                buf_size = callback_data.BufSize;
                apply_new_text_length = ImMin(callback_data.BufTextLen, buf_size - 1);
                IM_ASSERT(apply_new_text_length <= buf_size);
            }

            // If the underlying buffer resize was denied or not carried to the next frame, apply_new_text_length+1 may be >= buf_size.
            ImStrncpy(buf, apply_new_text, ImMin(apply_new_text_length + 1, buf_size));
            value_changed = true;
        }

        // Clear temporary user storage
        state->UserFlags = 0;
        state->UserCallback = NULL;
        state->UserCallbackData = NULL;
    }

    // Release active ID at the end of the function (so e.g. pressing Return still does a final application of the value)
    if (clear_active_id && g.ActiveId == id)
        ClearActiveID();

    // Render frame
    if (!is_multiline)
    {
        RenderNavHighlight(frame_bb_new, id);
        //RenderFrame(frame_bb.Min - ImVec2(100, 0), frame_bb.Max, GetColorU32(ImGuiCol_FrameBg), true, style.FrameRounding);
        window->DrawList->AddRectFilled(frame_bb_new.Min, frame_bb_new.Max, GetColorU32(ImGuiCol_FrameBg), style.FrameRounding);
        window->DrawList->AddRectFilled(ImVec2(frame_bb_new.Min.x, frame_bb_new.Max.y - 4.f), frame_bb_new.Max, GetColorU32(ImGuiCol_SliderGrab), style.FrameRounding, ImDrawCornerFlags_Bot);
    }

    const ImVec4 clip_rect(frame_bb_new.Min.x, frame_bb_new.Min.y, frame_bb_new.Min.x + size.x, frame_bb_new.Min.y + size.y); // Not using frame_bb.Max because we have adjusted size
    ImVec2 draw_pos = is_multiline ? draw_window->DC.CursorPos : frame_bb_new.Min + style.FramePadding;
    ImVec2 text_size(0.0f, 0.0f);

    // Set upper limit of single-line InputTextEx() at 2 million characters strings. The current pathological worst case is a long line
    // without any carriage return, which would makes ImFont::RenderText() reserve too many vertices and probably crash. Avoid it altogether.
    // Note that we only use this limit on single-line InputText(), so a pathologically large line on a InputTextMultiline() would still crash.
    const int buf_display_max_length = 2 * 1024 * 1024;

    // Select which buffer we are going to display. We set buf to NULL to prevent accidental usage from now on.
    const char* buf_display = (state != NULL && !is_readonly) ? state->TextA.Data : buf;
    IM_ASSERT(buf_display);
    buf = NULL;

    // Render text. We currently only render selection when the widget is active or while scrolling.
    // FIXME: We could remove the '&& render_cursor' to keep rendering selection when inactive.
    const bool render_cursor = (g.ActiveId == id) || user_scroll_active;
    const bool render_selection = state && state->HasSelection() && (RENDER_SELECTION_WHEN_INACTIVE || render_cursor);
    if (render_cursor || render_selection)
    {
        // Render text (with cursor and selection)
        // This is going to be messy. We need to:
        // - Display the text (this alone can be more easily clipped)
        // - Handle scrolling, highlight selection, display cursor (those all requires some form of 1d->2d cursor position calculation)
        // - Measure text height (for scrollbar)
        // We are attempting to do most of that in **one main pass** to minimize the computation cost (non-negligible for large amount of text) + 2nd pass for selection rendering (we could merge them by an extra refactoring effort)
        // FIXME: This should occur on buf_display but we'd need to maintain cursor/select_start/select_end for UTF-8.
        IM_ASSERT(state != NULL);
        const ImWchar* text_begin = state->TextW.Data;
        ImVec2 cursor_offset, select_start_offset;

        {
            // Find lines numbers straddling 'cursor' (slot 0) and 'select_start' (slot 1) positions.
            const ImWchar* searches_input_ptr[2] = { NULL, NULL };
            int searches_result_line_no[2] = { -1000, -1000 };
            int searches_remaining = 0;
            if (render_cursor)
            {
                searches_input_ptr[0] = text_begin + state->Stb.cursor;
                searches_result_line_no[0] = -1;
                searches_remaining++;
            }
            if (render_selection)
            {
                searches_input_ptr[1] = text_begin + ImMin(state->Stb.select_start, state->Stb.select_end);
                searches_result_line_no[1] = -1;
                searches_remaining++;
            }

            // Iterate all lines to find our line numbers
            // In multi-line mode, we never exit the loop until all lines are counted, so add one extra to the searches_remaining counter.
            searches_remaining += is_multiline ? 1 : 0;
            int line_count = 0;
            //for (const ImWchar* s = text_begin; (s = (const ImWchar*)wcschr((const wchar_t*)s, (wchar_t)'\n')) != NULL; s++)  // FIXME-OPT: Could use this when wchar_t are 16-bits
            for (const ImWchar* s = text_begin; *s != 0; s++)
                if (*s == '\n')
                {
                    line_count++;
                    if (searches_result_line_no[0] == -1 && s >= searches_input_ptr[0]) { searches_result_line_no[0] = line_count; if (--searches_remaining <= 0) break; }
                    if (searches_result_line_no[1] == -1 && s >= searches_input_ptr[1]) { searches_result_line_no[1] = line_count; if (--searches_remaining <= 0) break; }
                }
            line_count++;
            if (searches_result_line_no[0] == -1) 
                searches_result_line_no[0] = line_count;
            if (searches_result_line_no[1] == -1) 
                searches_result_line_no[1] = line_count;

            // Calculate 2d position by finding the beginning of the line and measuring distance
            cursor_offset.x = InputTextCalcTextSizeW(ImStrbolW(searches_input_ptr[0], text_begin), searches_input_ptr[0]).x;
            cursor_offset.y = searches_result_line_no[0] * g.FontSize;
            if (searches_result_line_no[1] >= 0)
            {
                select_start_offset.x = InputTextCalcTextSizeW(ImStrbolW(searches_input_ptr[1], text_begin), searches_input_ptr[1]).x;
                select_start_offset.y = searches_result_line_no[1] * g.FontSize;
            }

            // Store text height (note that we haven't calculated text width at all, see GitHub issues #383, #1224)
            if (is_multiline)
                text_size = ImVec2(size.x, line_count * g.FontSize);
        }

        // Scroll
        if (render_cursor && state->CursorFollow)
        {
            // Horizontal scroll in chunks of quarter width
            if (!(flags & ImGuiInputTextFlags_NoHorizontalScroll))
            {
                const float scroll_increment_x = size.x * 0.25f;
                if (cursor_offset.x < state->ScrollX)
                    state->ScrollX = (float)(int)ImMax(0.0f, cursor_offset.x - scroll_increment_x);
                else if (cursor_offset.x - size.x >= state->ScrollX)
                    state->ScrollX = (float)(int)(cursor_offset.x - size.x + scroll_increment_x);
            }
            else
            {
                state->ScrollX = 0.0f;
            }

            // Vertical scroll
            if (is_multiline)
            {
                float scroll_y = draw_window->Scroll.y;
                if (cursor_offset.y - g.FontSize < scroll_y)
                    scroll_y = ImMax(0.0f, cursor_offset.y - g.FontSize);
                else if (cursor_offset.y - size.y >= scroll_y)
                    scroll_y = cursor_offset.y - size.y;
                draw_window->DC.CursorPos.y += (draw_window->Scroll.y - scroll_y);   // Manipulate cursor pos immediately avoid a frame of lag
                draw_window->Scroll.y = scroll_y;
                draw_pos.y = draw_window->DC.CursorPos.y;
            }

            state->CursorFollow = false;
        }

        // Draw selection
        const ImVec2 draw_scroll = ImVec2(state->ScrollX, 0.0f);
        if (render_selection)
        {
            const ImWchar* text_selected_begin = text_begin + ImMin(state->Stb.select_start, state->Stb.select_end);
            const ImWchar* text_selected_end = text_begin + ImMax(state->Stb.select_start, state->Stb.select_end);

            ImU32 bg_color = GetColorU32(ImGuiCol_TextSelectedBg, render_cursor ? 1.0f : 0.6f); // FIXME: current code flow mandate that render_cursor is always true here, we are leaving the transparent one for tests.
            float bg_offy_up = is_multiline ? 0.0f : -1.0f;    // FIXME: those offsets should be part of the style? they don't play so well with multi-line selection.
            float bg_offy_dn = is_multiline ? 0.0f : 2.0f;
            ImVec2 rect_pos = draw_pos + select_start_offset - draw_scroll;
            for (const ImWchar* p = text_selected_begin; p < text_selected_end; )
            {
                if (rect_pos.y > clip_rect.w + g.FontSize)
                    break;
                if (rect_pos.y < clip_rect.y)
                {
                    //p = (const ImWchar*)wmemchr((const wchar_t*)p, '\n', text_selected_end - p);  // FIXME-OPT: Could use this when wchar_t are 16-bits
                    //p = p ? p + 1 : text_selected_end;
                    while (p < text_selected_end)
                        if (*p++ == '\n')
                            break;
                }
                else
                {
                    ImVec2 rect_size = InputTextCalcTextSizeW(p, text_selected_end, &p, NULL, true);
                    if (rect_size.x <= 0.0f) rect_size.x = (float)(int)(g.Font->GetCharAdvance((ImWchar)' ') * 0.50f); // So we can see selected empty lines
                    ImRect rect(rect_pos + ImVec2(0.0f, bg_offy_up - g.FontSize), rect_pos +ImVec2(rect_size.x, bg_offy_dn));
                    rect.ClipWith(clip_rect);
                    if (rect.Overlaps(clip_rect))
                        draw_window->DrawList->AddRectFilled(rect.Min, rect.Max, bg_color);
                }
                rect_pos.x = draw_pos.x - draw_scroll.x;
                rect_pos.y += g.FontSize;
            }
        }

        // We test for 'buf_display_max_length' as a way to avoid some pathological cases (e.g. single-line 1 MB string) which would make ImDrawList crash.
        const int buf_display_len = state->CurLenA;
        if (is_multiline || buf_display_len < buf_display_max_length)
            draw_window->DrawList->AddText(g.Font, g.FontSize, draw_pos - draw_scroll, GetColorU32(ImGuiCol_Text), buf_display, buf_display + buf_display_len, 0.0f, is_multiline ? NULL : &clip_rect);

        // Draw blinking cursor
        if (render_cursor)
        {
            state->CursorAnim += io.DeltaTime;
            bool cursor_is_visible = (!g.IO.ConfigInputTextCursorBlink) || (state->CursorAnim <= 0.0f) || ImFmod(state->CursorAnim, 1.20f) <= 0.80f;
            ImVec2 cursor_screen_pos = draw_pos + cursor_offset - draw_scroll;
            ImRect cursor_screen_rect(cursor_screen_pos.x, cursor_screen_pos.y - g.FontSize + 0.5f, cursor_screen_pos.x + 1.0f, cursor_screen_pos.y - 1.5f);
            if (cursor_is_visible && cursor_screen_rect.Overlaps(clip_rect))
                draw_window->DrawList->AddLine(cursor_screen_rect.Min, cursor_screen_rect.GetBL(), GetColorU32(ImGuiCol_Text));

            // Notify OS of text input position for advanced IME (-1 x offset so that Windows IME can cover our cursor. Bit of an extra nicety.)
            if (!is_readonly)
                g.PlatformImePos = ImVec2(cursor_screen_pos.x - 1.0f, cursor_screen_pos.y - g.FontSize);
        }
    }
    else
    {
        // Render text only (no selection, no cursor)
        const char* buf_end = NULL;
        if (is_multiline)
            text_size = ImVec2(size.x, InputTextCalcTextLenAndLineCount(buf_display, &buf_end) * g.FontSize); // We don't need width
        else
            buf_end = buf_display + strlen(buf_display);
        if (is_multiline || (buf_end - buf_display) < buf_display_max_length)
            draw_window->DrawList->AddText(g.Font, g.FontSize, draw_pos, GetColorU32(ImGuiCol_Text), buf_display, buf_end, 0.0f, is_multiline ? NULL : &clip_rect);
    }

    if (is_multiline)
    {
        Dummy(text_size + ImVec2(0.0f, g.FontSize)); // Always add room to scroll an extra line
        EndChildFrame();
        EndGroup();
    }

    if (is_password)
        PopFont();

    // Log as text
    if (g.LogEnabled && !is_password)
        LogRenderedText(&draw_pos, buf_display, NULL);

    if (label_size.x > 0)
        RenderText(ImVec2(frame_bb.Min.x, frame_bb.Min.y), label);

    if (value_changed)
        MarkItemEdited(id);

    IMGUI_TEST_ENGINE_ITEM_INFO(id, label, window->DC.ItemFlags);
    if ((flags & ImGuiInputTextFlags_EnterReturnsTrue) != 0)
        return enter_pressed;
    else
        return value_changed;
}

//-------------------------------------------------------------------------
// [SECTION] Widgets: ColorEdit, ColorPicker, ColorButton, etc.
//-------------------------------------------------------------------------
// - ColorEdit3()
// - ColorEdit4()
// - ColorPicker3()
// - RenderColorRectWithAlphaCheckerboard() [Internal]
// - ColorPicker4()
// - ColorButton()
// - SetColorEditOptions()
// - ColorTooltip() [Internal]
// - ColorEditOptionsPopup() [Internal]
// - ColorPickerOptionsPopup() [Internal]
//-------------------------------------------------------------------------

bool ImGui::ColorEdit3(const char* label, Color* color, ImGuiColorEditFlags flags)
{
    return ColorEdit4(label, color, flags | ImGuiColorEditFlags_AlphaBar);
}

// Edit colors components (each component in 0.0f..1.0f range).
// See enum ImGuiColorEditFlags_ for available options. e.g. Only access 3 floats if ImGuiColorEditFlags_NoAlpha flag is set.
// With typical options: Left-click on colored square to open color picker. Right-click to open option menu. CTRL-Click over input fields to edit them and TAB to go to next item.
bool ImGui::ColorEdit4(const char* label, Color* color, ImGuiColorEditFlags flags)
{
    //float* col[4] = { &((float&)color->_CColor[0]), &((float&)color->_CColor[1]), &((float&)color->_CColor[2]), &((float&)color->_CColor[3]) };
    float* col[4] = { &color->save_color[0], &color->save_color[1], &color->save_color[2], &color->save_color[3] };

    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    ImGuiStyle* style_sett = &ImGui::GetStyle();
    const float square_sz = GetFrameHeight();
    const float w_extra = (flags & ImGuiColorEditFlags_NoSmallPreview) ? 0.0f : (square_sz + style.ItemInnerSpacing.x);
    const float w_items_all = CalcItemWidth() - w_extra;
    const char* label_display_end = FindRenderedTextEnd(label);

    const ImVec2 label_size = CalcTextSize(label);


    const ImVec2 becup_windpadd = style.WindowPadding;
    const ImVec2 becup_framepadd = style.FramePadding;
    const ImVec2 becup_itemspac = style.ItemSpacing;

    PushID(label);

    // If we're not showing any slider there's no point in doing any HSV conversions
    const ImGuiColorEditFlags flags_untouched = flags;
    if (flags & ImGuiColorEditFlags_NoInputs)
        flags = (flags & (~ImGuiColorEditFlags__InputsMask)) | ImGuiColorEditFlags_RGB | ImGuiColorEditFlags_NoOptions;

    // Context menu: display and modify options (before defaults are applied)
    if (!(flags & ImGuiColorEditFlags_NoOptions))
        ColorEditOptionsPopup(*col, flags);

    // Read stored options
    if (!(flags & ImGuiColorEditFlags__InputsMask))
        flags |= (g.ColorEditOptions & ImGuiColorEditFlags__InputsMask);
    if (!(flags & ImGuiColorEditFlags__DataTypeMask))
        flags |= (g.ColorEditOptions & ImGuiColorEditFlags__DataTypeMask);
    if (!(flags & ImGuiColorEditFlags__PickerMask))
        flags |= (g.ColorEditOptions & ImGuiColorEditFlags__PickerMask);
    flags |= (g.ColorEditOptions & ~(ImGuiColorEditFlags__InputsMask | ImGuiColorEditFlags__DataTypeMask | ImGuiColorEditFlags__PickerMask));

    const bool alpha = (flags & ImGuiColorEditFlags_NoAlpha) == 0;
    const bool hdr = (flags & ImGuiColorEditFlags_HDR) != 0;
    const int components = alpha ? 4 : 3;

    // Convert to the formats we need
    float f[4] = { *col[0], *col[1], *col[2], alpha ? *col[3] : 1.0f };
    if (flags & ImGuiColorEditFlags_HSV)
        ColorConvertRGBtoHSV(f[0], f[1], f[2], f[0], f[1], f[2]);
    int i[4] = { IM_F32_TO_INT8_UNBOUND(f[0]), IM_F32_TO_INT8_UNBOUND(f[1]), IM_F32_TO_INT8_UNBOUND(f[2]), IM_F32_TO_INT8_UNBOUND(f[3]) };

    bool value_changed = false;
    bool value_changed_as_float = false;


    float offset_x = 17 + style.ItemInnerSpacing.x;

    const ImVec2 pos = window->DC.CursorPos + ImVec2(23, 0);
    const float inputs_offset_x = w_extra;
    window->DC.CursorPos.x += offset_x;//pos.x + inputs_offset_x;


    if (label != label_display_end && !(flags & ImGuiColorEditFlags_NoLabel))
    {
        TextEx(label, label_display_end);
    }

    if ((flags & (ImGuiColorEditFlags_RGB | ImGuiColorEditFlags_HSV)) != 0 && (flags & ImGuiColorEditFlags_NoInputs) == 0)
    {
        // RGB/HSV 0..255 Sliders
        const float w_item_one = ImMax(1.0f, (float)(int)((w_items_all - (style.ItemInnerSpacing.x) * (components - 1)) / (float)components));
        const float w_item_last = ImMax(1.0f, (float)(int)(w_items_all - (w_item_one + style.ItemInnerSpacing.x) * (components - 1)));

        const bool hide_prefix = (w_item_one <= CalcTextSize((flags & ImGuiColorEditFlags_Float) ? "M:0.000" : "M:000").x);
        const char* ids[4] = { "##X", "##Y", "##Z", "##W" };
        const char* fmt_table_int[3][4] =
        {
            {   "%3d",   "%3d",   "%3d",   "%3d" }, // Short display
            { "R:%3d", "G:%3d", "B:%3d", "A:%3d" }, // Long display for RGBA
            { "H:%3d", "S:%3d", "V:%3d", "A:%3d" }  // Long display for HSVA
        };
        const char* fmt_table_float[3][4] =
        {
            {   "%0.3f",   "%0.3f",   "%0.3f",   "%0.3f" }, // Short display
            { "R:%0.3f", "G:%0.3f", "B:%0.3f", "A:%0.3f" }, // Long display for RGBA
            { "H:%0.3f", "S:%0.3f", "V:%0.3f", "A:%0.3f" }  // Long display for HSVA
        };
        const int fmt_idx = hide_prefix ? 0 : (flags & ImGuiColorEditFlags_HSV) ? 2 : 1;

        /*PushItemWidth(w_item_one);
        /*for (int n = 0; n < components; n++)
        {
            if (n > 0)
                SameLine(0, style.ItemInnerSpacing.x);
            if (n + 1 == components)
                PushItemWidth(w_item_last);
            if (flags & ImGuiColorEditFlags_Float)
            {
                value_changed |= DragFloat(ids[n], &f[n], 1.0f/255.0f, 0.0f, hdr ? 0.0f : 1.0f, fmt_table_float[fmt_idx][n]);
                value_changed_as_float |= value_changed;
            }
            else
            {
                value_changed |= DragInt(ids[n], &i[n], 1.0f, 0, hdr ? 0 : 255, fmt_table_int[fmt_idx][n]);
            }
            if (!(flags & ImGuiColorEditFlags_NoOptions))
                OpenPopupOnItemClick("context");
        }
        PopItemWidth();
        PopItemWidth();*/
    }
    else if ((flags & ImGuiColorEditFlags_HEX) != 0 && (flags & ImGuiColorEditFlags_NoInputs) == 0)
    {
        // RGB Hexadecimal Input
        char buf[64];
        if (alpha)
            ImFormatString(buf, IM_ARRAYSIZE(buf), "#%02X%02X%02X%02X", ImClamp(i[0], 0, 255), ImClamp(i[1], 0, 255), ImClamp(i[2], 0, 255), ImClamp(i[3], 0, 255));
        else
            ImFormatString(buf, IM_ARRAYSIZE(buf), "#%02X%02X%02X", ImClamp(i[0], 0, 255), ImClamp(i[1], 0, 255), ImClamp(i[2], 0, 255));
        /*PushItemWidth(w_items_all);
        if (InputText("##Text", buf, IM_ARRAYSIZE(buf), ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsUppercase))
        {
            value_changed = true;
            char* p = buf;
            while (*p == '#' || ImCharIsBlankA(*p))
                p++;
            i[0] = i[1] = i[2] = i[3] = 0;
            if (alpha)
                sscanf(p, "%02X%02X%02X%02X", (unsigned int*)&i[0], (unsigned int*)&i[1], (unsigned int*)&i[2], (unsigned int*)&i[3]); // Treat at unsigned (%X is unsigned)
            else
                sscanf(p, "%02X%02X%02X", (unsigned int*)&i[0], (unsigned int*)&i[1], (unsigned int*)&i[2]);
        }
        if (!(flags & ImGuiColorEditFlags_NoOptions))
            OpenPopupOnItemClick("context");
        PopItemWidth();*/
    }

    ImGuiWindow* picker_active_window = NULL;
    if (!(flags & ImGuiColorEditFlags_NoSmallPreview))
    {
        //const float button_offset_x = ((flags & ImGuiColorEditFlags_NoInputs) || (style.ColorButtonPosition == ImGuiDir_Left)) ? 0.0f : w_inputs + style.ItemInnerSpacing.x;
        const ImVec2 size_wind_max = window->SizeFull;
        const ImVec2 pos_wind = window->Pos;
        const float offset_scroll = window->ScrollbarSizes.x > 0 ? window->ScrollbarSizes.x : 0;
        //window->DC.CursorPos = ImVec2(pos_wind.x + (size_wind_max.x - w_extra - 18 - offset_scroll), pos.y);

        window->DC.CursorPos = ImVec2(pos_wind.x + (size_wind_max.x - w_extra - 29 - offset_scroll), pos.y);

        const ImVec4 col_v4(color->save_color[0], color->save_color[1], color->save_color[2], alpha ? color->save_color[3] : 1.0f);
        if (ColorButton("##ColorButton", col_v4, flags))
        {
            if (!(flags & ImGuiColorEditFlags_NoPicker))
            {
                // Store current color and open a picker
                g.ColorPickerRef = col_v4;
                OpenPopup("picker");
                SetNextWindowPos(window->DC.LastItemRect.GetBL() + ImVec2(-1, style.ItemSpacing.y));
            }
        }
        if (!(flags & ImGuiColorEditFlags_NoOptions))
            OpenPopupOnItemClick("context");

        style_sett->WindowPadding = ImVec2(7, 7);
        style_sett->FramePadding = ImVec2(2, 2);
        style_sett->ItemSpacing = ImVec2(0, 0);

        window->DrawList->AddRectFilled(ImVec2(pos_wind.x + (size_wind_max.x - w_extra - 23 - offset_scroll), window->DC.CursorPos.y + 2), ImVec2(pos_wind.x + label_size.x, window->DC.CursorPos.y + 2), GetColorU32(ImVec4(1.f, 1.f, 1.f, 0.2f)));

        if (BeginPopup("picker"))
        {
            picker_active_window = g.CurrentWindow;
            /*if (label != label_display_end)
            {
                TextUnformatted(label, label_display_end);
                Spacing();
            }*/
            ImGuiColorEditFlags picker_flags_to_forward = ImGuiColorEditFlags__DataTypeMask | ImGuiColorEditFlags__PickerMask | ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_AlphaBar;
            ImGuiColorEditFlags picker_flags = (flags_untouched & picker_flags_to_forward) | ImGuiColorEditFlags__InputsMask | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_AlphaPreviewHalf;
            PushItemWidth(square_sz * 12.0f); // Use 256 + bar sizes?
            value_changed |= ColorPicker4("##picker", color, picker_flags, &g.ColorPickerRef.x);
            PopItemWidth();
            EndPopup();
        }


        style_sett->WindowPadding = becup_windpadd;
        style_sett->FramePadding = becup_framepadd;
        style_sett->ItemSpacing = becup_itemspac;
    }

    // Convert back
    if (picker_active_window == NULL)
    {
        if (!value_changed_as_float)
            for (int n = 0; n < 4; n++)
                f[n] = i[n] / 255.0f;
        if (flags & ImGuiColorEditFlags_HSV)
            ColorConvertHSVtoRGB(f[0], f[1], f[2], f[0], f[1], f[2]);
        if (value_changed)
        {
            *col[0] = f[0];
            *col[1] = f[1];
            *col[2] = f[2];
            if (alpha)
                *col[3] = f[3];
            //color->set_col_f(1.f, 1.f, 1.f, 1.f);
        }
     //    ^ ��� ���
     // ���������� clown

       /* function aye{
vova = pidor
}*/

        //if (*col[0] != f[0])
        //{
            color->_CColor[0] = *col[0] * 255.f;
        //}
        //if (*col[1] != f[1])
        //{
            color->_CColor[1] = *col[1] * 255.f;
        //}
        //if (*col[2] != f[2])
        //{
            color->_CColor[2] = *col[2] * 255.f;
        //}
        //if (*col[3] != f[3] && alpha)
        //{
            color->_CColor[3] = *col[3] * 255.f;
        //}

    }

    PopID();

    // Drag and Drop Target
    // NB: The flag test is merely an optional micro-optimization, BeginDragDropTarget() does the same test.
    /*if ((window->DC.LastItemStatusFlags & ImGuiItemStatusFlags_HoveredRect) && !(flags & ImGuiColorEditFlags_NoDragDrop) && BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = AcceptDragDropPayload(IMGUI_PAYLOAD_TYPE_COLOR_3F))
        {
            memcpy((float*)col, payload->Data, sizeof(float) * 3); // Preserve alpha if any //-V512
            value_changed = true;
        }
        if (const ImGuiPayload* payload = AcceptDragDropPayload(IMGUI_PAYLOAD_TYPE_COLOR_4F))
        {
            memcpy((float*)col, payload->Data, sizeof(float) * components);
            value_changed = true;
        }
        EndDragDropTarget();
    }*/

    // When picker is being actively used, use its active id so IsItemActive() will function on ColorEdit4().
    if (picker_active_window && g.ActiveId != 0 && g.ActiveIdWindow == picker_active_window)
        window->DC.LastItemId = g.ActiveId;

    if (value_changed)
        MarkItemEdited(window->DC.LastItemId);

    return value_changed;
}

bool ImGui::ColorPicker3(const char* label, Color* col, ImGuiColorEditFlags flags)
{
    if (!ColorPicker4(label, col, flags | ImGuiColorEditFlags_NoAlpha))
        return false;
    return true;
}

static ImU32 ImAlphaBlendColor(ImU32 col_a, ImU32 col_b)
{
    float t = ((col_b >> IM_COL32_A_SHIFT) & 0xFF) / 255.f;
    int r = ImLerp((int)(col_a >> IM_COL32_R_SHIFT) & 0xFF, (int)(col_b >> IM_COL32_R_SHIFT) & 0xFF, t);
    int g = ImLerp((int)(col_a >> IM_COL32_G_SHIFT) & 0xFF, (int)(col_b >> IM_COL32_G_SHIFT) & 0xFF, t);
    int b = ImLerp((int)(col_a >> IM_COL32_B_SHIFT) & 0xFF, (int)(col_b >> IM_COL32_B_SHIFT) & 0xFF, t);
    return IM_COL32(r, g, b, 0xFF);
}

// Helper for ColorPicker4()
// NB: This is rather brittle and will show artifact when rounding this enabled if rounded corners overlap multiple cells. Caller currently responsible for avoiding that.
// I spent a non reasonable amount of time trying to getting this right for ColorButton with rounding+anti-aliasing+ImGuiColorEditFlags_HalfAlphaPreview flag + various grid sizes and offsets, and eventually gave up... probably more reasonable to disable rounding alltogether.
void ImGui::RenderColorRectWithAlphaCheckerboard(ImVec2 p_min, ImVec2 p_max, ImU32 col, float grid_step, ImVec2 grid_off, float rounding, int rounding_corners_flags)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (((col & IM_COL32_A_MASK) >> IM_COL32_A_SHIFT) < 0xFF)
    {
        ImU32 col_bg1 = GetColorU32(ImAlphaBlendColor(IM_COL32(204,204,204,255), col));
        ImU32 col_bg2 = GetColorU32(ImAlphaBlendColor(IM_COL32(128,128,128,255), col));
        window->DrawList->AddRectFilled(p_min, p_max, col_bg1, rounding, rounding_corners_flags);

        int yi = 0;
        for (float y = p_min.y + grid_off.y; y < p_max.y; y += grid_step, yi++)
        {
            float y1 = ImClamp(y, p_min.y, p_max.y), y2 = ImMin(y + grid_step, p_max.y);
            if (y2 <= y1)
                continue;
            for (float x = p_min.x + grid_off.x + (yi & 1) * grid_step; x < p_max.x; x += grid_step * 2.0f)
            {
                float x1 = ImClamp(x, p_min.x, p_max.x), x2 = ImMin(x + grid_step, p_max.x);
                if (x2 <= x1)
                    continue;
                int rounding_corners_flags_cell = 0;
                if (y1 <= p_min.y) { if (x1 <= p_min.x) rounding_corners_flags_cell |= ImDrawCornerFlags_TopLeft; if (x2 >= p_max.x) rounding_corners_flags_cell |= ImDrawCornerFlags_TopRight; }
                if (y2 >= p_max.y) { if (x1 <= p_min.x) rounding_corners_flags_cell |= ImDrawCornerFlags_BotLeft; if (x2 >= p_max.x) rounding_corners_flags_cell |= ImDrawCornerFlags_BotRight; }
                rounding_corners_flags_cell &= rounding_corners_flags;
                window->DrawList->AddRectFilled(ImVec2(x1,y1), ImVec2(x2,y2), col_bg2, rounding_corners_flags_cell ? rounding : 0.0f, rounding_corners_flags_cell);
            }
        }
    }
    else
    {
        window->DrawList->AddRectFilled(p_min, p_max, col, rounding, rounding_corners_flags);
    }
}

// Helper for ColorPicker4()
static void RenderArrowsForVerticalBar(ImDrawList* draw_list, ImVec2 pos, ImVec2 half_sz, float bar_w)
{
    ImGui::RenderArrowPointingAt(draw_list, ImVec2(pos.x + half_sz.x + 1,         pos.y), ImVec2(half_sz.x + 2, half_sz.y + 1), ImGuiDir_Right, IM_COL32_BLACK);
    ImGui::RenderArrowPointingAt(draw_list, ImVec2(pos.x + half_sz.x,             pos.y), half_sz,                              ImGuiDir_Right, IM_COL32_WHITE);
    ImGui::RenderArrowPointingAt(draw_list, ImVec2(pos.x + bar_w - half_sz.x - 1, pos.y), ImVec2(half_sz.x + 2, half_sz.y + 1), ImGuiDir_Left,  IM_COL32_BLACK);
    ImGui::RenderArrowPointingAt(draw_list, ImVec2(pos.x + bar_w - half_sz.x,     pos.y), half_sz,                              ImGuiDir_Left,  IM_COL32_WHITE);
}

// Note: ColorPicker4() only accesses 3 floats if ImGuiColorEditFlags_NoAlpha flag is set.
// FIXME: we adjust the big color square height based on item width, which may cause a flickering feedback loop (if automatic height makes a vertical scrollbar appears, affecting automatic width..)
bool ImGui::ColorPicker4(const char* label, Color* color, ImGuiColorEditFlags flags, const float* ref_col)
{
    float* col[4] = { &color->save_color[0], &color->save_color[1], &color->save_color[2], &color->save_color[3] };
    //float* col[4] = { &((float&)color->_CColor[0]), &((float&)color->_CColor[1]), &((float&)color->_CColor[2]), &((float&)color->_CColor[3]) };

    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = GetCurrentWindow();
    ImDrawList* draw_list = window->DrawList;

    ImGuiStyle& style = g.Style;
    ImGuiIO& io = g.IO;

    PushID(label);
    BeginGroup();

    if (!(flags & ImGuiColorEditFlags_NoSidePreview))
        flags |= ImGuiColorEditFlags_NoSmallPreview;

    // Context menu: display and store options.
    if (!(flags & ImGuiColorEditFlags_NoOptions))
        ColorPickerOptionsPopup(*col, flags);

    // Read stored options
    if (!(flags & ImGuiColorEditFlags__PickerMask))
        flags |= ((g.ColorEditOptions & ImGuiColorEditFlags__PickerMask) ? g.ColorEditOptions : ImGuiColorEditFlags__OptionsDefault) & ImGuiColorEditFlags__PickerMask;
    IM_ASSERT(ImIsPowerOfTwo((int)(flags & ImGuiColorEditFlags__PickerMask))); // Check that only 1 is selected
    if (!(flags & ImGuiColorEditFlags_NoOptions))
        flags |= (g.ColorEditOptions & ImGuiColorEditFlags_AlphaBar);

    // Setup
    int components = (flags & ImGuiColorEditFlags_NoAlpha) ? 3 : 4;
    bool alpha_bar = (flags & ImGuiColorEditFlags_AlphaBar) && !(flags & ImGuiColorEditFlags_NoAlpha);
    ImVec2 picker_pos = window->DC.CursorPos;
    float square_sz = GetFrameHeight();
    float bars_width = square_sz; // Arbitrary smallish width of Hue/Alpha picking bars
    float sv_picker_size = ImMax(bars_width * 1, CalcItemWidth() - (alpha_bar ? 2 : 2) * (bars_width + style.ItemInnerSpacing.x)); // Saturation/Value picking box
    float bar0_pos_x = picker_pos.x + sv_picker_size + style.ItemInnerSpacing.x;
    float bar1_pos_x = bar0_pos_x + bars_width + style.ItemInnerSpacing.x;
    float bars_triangles_half_sz = (float)(int)(bars_width * 0.20f);

    float backup_initial_col[4];
    memcpy(backup_initial_col, col, components * sizeof(float));

    float wheel_thickness = sv_picker_size * 0.08f;
    float wheel_r_outer = sv_picker_size * 0.50f;
    float wheel_r_inner = wheel_r_outer - wheel_thickness;
    ImVec2 wheel_center(picker_pos.x + (sv_picker_size + bars_width)*0.5f, picker_pos.y + sv_picker_size*0.5f);

    // Note: the triangle is displayed rotated with triangle_pa pointing to Hue, but most coordinates stays unrotated for logic.
    float triangle_r = wheel_r_inner - (int)(sv_picker_size * 0.027f);
    ImVec2 triangle_pa = ImVec2(triangle_r, 0.0f); // Hue point.
    ImVec2 triangle_pb = ImVec2(triangle_r * -0.5f, triangle_r * -0.866025f); // Black point.
    ImVec2 triangle_pc = ImVec2(triangle_r * -0.5f, triangle_r * +0.866025f); // White point.

    float H,S,V;
    ColorConvertRGBtoHSV(*col[0], *col[1], *col[2], H, S, V);

    bool value_changed = false, value_changed_h = false, value_changed_sv = false;

    PushItemFlag(ImGuiItemFlags_NoNav, true);
    if (flags & ImGuiColorEditFlags_PickerHueWheel)
    {
        // Hue wheel + SV triangle logic
        InvisibleButton("hsv", ImVec2(sv_picker_size + style.ItemInnerSpacing.x + bars_width, sv_picker_size));
        if (IsItemActive())
        {
            ImVec2 initial_off = g.IO.MouseClickedPos[0] - wheel_center;
            ImVec2 current_off = g.IO.MousePos - wheel_center;
            float initial_dist2 = ImLengthSqr(initial_off);
            if (initial_dist2 >= (wheel_r_inner-1)*(wheel_r_inner-1) && initial_dist2 <= (wheel_r_outer+1)*(wheel_r_outer+1))
            {
                // Interactive with Hue wheel
                H = ImAtan2(current_off.y, current_off.x) / IM_PI*0.5f;
                if (H < 0.0f)
                    H += 1.0f;
                value_changed = value_changed_h = true;
            }
            float cos_hue_angle = ImCos(-H * 2.0f * IM_PI);
            float sin_hue_angle = ImSin(-H * 2.0f * IM_PI);
            if (ImTriangleContainsPoint(triangle_pa, triangle_pb, triangle_pc, ImRotate(initial_off, cos_hue_angle, sin_hue_angle)))
            {
                // Interacting with SV triangle
                ImVec2 current_off_unrotated = ImRotate(current_off, cos_hue_angle, sin_hue_angle);
                if (!ImTriangleContainsPoint(triangle_pa, triangle_pb, triangle_pc, current_off_unrotated))
                    current_off_unrotated = ImTriangleClosestPoint(triangle_pa, triangle_pb, triangle_pc, current_off_unrotated);
                float uu, vv, ww;
                ImTriangleBarycentricCoords(triangle_pa, triangle_pb, triangle_pc, current_off_unrotated, uu, vv, ww);
                V = ImClamp(1.0f - vv, 0.0001f, 1.0f);
                S = ImClamp(uu / V, 0.0001f, 1.0f);
                value_changed = value_changed_sv = true;
            }
        }
        if (!(flags & ImGuiColorEditFlags_NoOptions))
            OpenPopupOnItemClick("context");
    }
    else if (flags & ImGuiColorEditFlags_PickerHueBar)
    {
        // SV rectangle logic
        InvisibleButton("sv", ImVec2(sv_picker_size, sv_picker_size));
        if (IsItemActive())
        {
            S = ImSaturate((io.MousePos.x - picker_pos.x) / (sv_picker_size-1));
            V = 1.0f - ImSaturate((io.MousePos.y - picker_pos.y) / (sv_picker_size-1));
            value_changed = value_changed_sv = true;
        }
        if (!(flags & ImGuiColorEditFlags_NoOptions))
            OpenPopupOnItemClick("context");

        // Hue bar logic
        SetCursorScreenPos(ImVec2(bar0_pos_x, picker_pos.y));
        InvisibleButton("hue", ImVec2(bars_width, sv_picker_size));
        if (IsItemActive())
        {
            H = ImSaturate((io.MousePos.y - picker_pos.y) / (sv_picker_size-1));
            value_changed = value_changed_h = true;
        }
    }

    // Alpha bar logic
    if (alpha_bar)
    {
        SetCursorScreenPos(ImVec2(bar1_pos_x, picker_pos.y));
        InvisibleButton("alpha", ImVec2(bars_width, sv_picker_size));
        if (IsItemActive())
        {
            *col[3] = 1.0f - ImSaturate((io.MousePos.y - picker_pos.y) / (sv_picker_size-1));
            value_changed = true;
        }
    }
    PopItemFlag(); // ImGuiItemFlags_NoNav

    if (!(flags & ImGuiColorEditFlags_NoSidePreview))
    {
        SameLine(0, style.ItemInnerSpacing.x);
        BeginGroup();
    }

    if (!(flags & ImGuiColorEditFlags_NoLabel))
    {
        const char* label_display_end = FindRenderedTextEnd(label);
        if (label != label_display_end)
        {
            if ((flags & ImGuiColorEditFlags_NoSidePreview))
                SameLine(0, style.ItemInnerSpacing.x);
            TextUnformatted(label, label_display_end);
        }
    }

    if (!(flags & ImGuiColorEditFlags_NoSidePreview))
    {
        PushItemFlag(ImGuiItemFlags_NoNavDefaultFocus, true);
        ImVec4 col_v4(*col[0], *col[1], *col[2], (flags & ImGuiColorEditFlags_NoAlpha) ? 1.0f : *col[3]);
        /*if ((flags & ImGuiColorEditFlags_NoLabel))
            Text("Current");
        ColorButton("##current", col_v4, (flags & (ImGuiColorEditFlags_HDR|ImGuiColorEditFlags_AlphaPreview|ImGuiColorEditFlags_AlphaPreviewHalf|ImGuiColorEditFlags_NoTooltip)), ImVec2(square_sz * 3, square_sz * 2));
        if (ref_col != NULL)
        {
            Text("Original");
            ImVec4 ref_col_v4(ref_col[0], ref_col[1], ref_col[2], (flags & ImGuiColorEditFlags_NoAlpha) ? 1.0f : ref_col[3]);
            if (ColorButton("##original", ref_col_v4, (flags & (ImGuiColorEditFlags_HDR|ImGuiColorEditFlags_AlphaPreview|ImGuiColorEditFlags_AlphaPreviewHalf|ImGuiColorEditFlags_NoTooltip)), ImVec2(square_sz * 3, square_sz * 2)))
            {
                memcpy(col, ref_col, components * sizeof(float));
                value_changed = true;
            }
        }*/
        PopItemFlag();
        EndGroup();
    }

    // Convert back color to RGB
    if (value_changed_h || value_changed_sv)
        ColorConvertHSVtoRGB(H >= 1.0f ? H - 10 * 1e-6f : H, S > 0.0f ? S : 10*1e-6f, V > 0.0f ? V : 1e-6f, *col[0], *col[1], *col[2]);

    // R,G,B and H,S,V slider color editor

    bool value_changed_fix_hue_wrap = false;
    if ((flags & ImGuiColorEditFlags_NoInputs) == 0)
    {
        PushItemWidth((alpha_bar ? bar1_pos_x : bar0_pos_x) + bars_width - picker_pos.x);
        ImGuiColorEditFlags sub_flags_to_forward = ImGuiColorEditFlags__DataTypeMask | ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoOptions | ImGuiColorEditFlags_NoSmallPreview | ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_AlphaPreviewHalf;
        ImGuiColorEditFlags sub_flags = (flags & sub_flags_to_forward) | ImGuiColorEditFlags_NoPicker;
        if (flags & ImGuiColorEditFlags_RGB || (flags & ImGuiColorEditFlags__InputsMask) == 0)
            if (ColorEdit4("##rgb", color, sub_flags | ImGuiColorEditFlags_RGB))
            {
                // FIXME: Hackily differenciating using the DragInt (ActiveId != 0 && !ActiveIdAllowOverlap) vs. using the InputText or DropTarget.
                // For the later we don't want to run the hue-wrap canceling code. If you are well versed in HSV picker please provide your input! (See #2050)
                value_changed_fix_hue_wrap = (g.ActiveId != 0 && !g.ActiveIdAllowOverlap);
                value_changed = true;
            }
        if (flags & ImGuiColorEditFlags_HSV || (flags & ImGuiColorEditFlags__InputsMask) == 0)
            value_changed |= ColorEdit4("##hsv", color, sub_flags | ImGuiColorEditFlags_HSV);
        if (flags & ImGuiColorEditFlags_HEX || (flags & ImGuiColorEditFlags__InputsMask) == 0)
            value_changed |= ColorEdit4("##hex", color, sub_flags | ImGuiColorEditFlags_HEX);
        PopItemWidth();
    }

    // Try to cancel hue wrap (after ColorEdit4 call), if any
    if (value_changed_fix_hue_wrap)
    {
        float new_H, new_S, new_V;
        ColorConvertRGBtoHSV(*col[0], *col[1], *col[2], new_H, new_S, new_V);
        if (new_H <= 0 && H > 0)
        {
            if (new_V <= 0 && V != new_V)
                ColorConvertHSVtoRGB(H, S, new_V <= 0 ? V * 0.5f : new_V, *col[0], *col[1], *col[2]);
            else if (new_S <= 0)
                ColorConvertHSVtoRGB(H, new_S <= 0 ? S * 0.5f : new_S, new_V, *col[0], *col[1], *col[2]);
        }
    }

    ImVec4 hue_color_f(1, 1, 1, 1); ColorConvertHSVtoRGB(H, 1, 1, hue_color_f.x, hue_color_f.y, hue_color_f.z);
    ImU32 hue_color32 = ColorConvertFloat4ToU32(hue_color_f);
    ImU32 col32_no_alpha = ColorConvertFloat4ToU32(ImVec4(*col[0], *col[1], *col[2], 1.0f));

    const ImU32 hue_colors[6+1] = { IM_COL32(255,0,0,255), IM_COL32(255,255,0,255), IM_COL32(0,255,0,255), IM_COL32(0,255,255,255), IM_COL32(0,0,255,255), IM_COL32(255,0,255,255), IM_COL32(255,0,0,255) };
    ImVec2 sv_cursor_pos;

    if (flags & ImGuiColorEditFlags_PickerHueWheel)
    {
        // Render Hue Wheel
        const float aeps = 1.5f / wheel_r_outer; // Half a pixel arc length in radians (2pi cancels out).
        const int segment_per_arc = ImMax(4, (int)wheel_r_outer / 12);
        for (int n = 0; n < 6; n++)
        {
            const float a0 = (n)     /6.0f * 2.0f * IM_PI - aeps;
            const float a1 = (n+1.0f)/6.0f * 2.0f * IM_PI + aeps;
            const int vert_start_idx = draw_list->VtxBuffer.Size;
            draw_list->PathArcTo(wheel_center, (wheel_r_inner + wheel_r_outer)*0.5f, a0, a1, segment_per_arc);
            draw_list->PathStroke(IM_COL32_WHITE, false, wheel_thickness);
            const int vert_end_idx = draw_list->VtxBuffer.Size;

            // Paint colors over existing vertices
            ImVec2 gradient_p0(wheel_center.x + ImCos(a0) * wheel_r_inner, wheel_center.y + ImSin(a0) * wheel_r_inner);
            ImVec2 gradient_p1(wheel_center.x + ImCos(a1) * wheel_r_inner, wheel_center.y + ImSin(a1) * wheel_r_inner);
            ShadeVertsLinearColorGradientKeepAlpha(draw_list, vert_start_idx, vert_end_idx, gradient_p0, gradient_p1, hue_colors[n], hue_colors[n+1]);
        }

        // Render Cursor + preview on Hue Wheel
        float cos_hue_angle = ImCos(H * 2.0f * IM_PI);
        float sin_hue_angle = ImSin(H * 2.0f * IM_PI);
        ImVec2 hue_cursor_pos(wheel_center.x + cos_hue_angle * (wheel_r_inner+wheel_r_outer)*0.5f, wheel_center.y + sin_hue_angle * (wheel_r_inner+wheel_r_outer)*0.5f);
        float hue_cursor_rad = value_changed_h ? wheel_thickness * 0.65f : wheel_thickness * 0.55f;
        int hue_cursor_segments = ImClamp((int)(hue_cursor_rad / 1.4f), 9, 32);
        draw_list->AddRect(ImVec2(sv_cursor_pos.x - 4, sv_cursor_pos.y - 4), ImVec2(sv_cursor_pos.x + 4, sv_cursor_pos.y + 4), GetColorU32(ImVec4(0.f, 0.f, 0.f, 1.f)));
        draw_list->AddRect(ImVec2(sv_cursor_pos.x - 3, sv_cursor_pos.y - 3), ImVec2(sv_cursor_pos.x + 3, sv_cursor_pos.y + 3), GetColorU32(ImVec4(1.f, 1.f, 1.f, 1.f)));

        // Render SV triangle (rotated according to hue)
        ImVec2 tra = wheel_center + ImRotate(triangle_pa, cos_hue_angle, sin_hue_angle);
        ImVec2 trb = wheel_center + ImRotate(triangle_pb, cos_hue_angle, sin_hue_angle);
        ImVec2 trc = wheel_center + ImRotate(triangle_pc, cos_hue_angle, sin_hue_angle);
        ImVec2 uv_white = GetFontTexUvWhitePixel();
        draw_list->PrimReserve(6, 6);
        draw_list->PrimVtx(tra, uv_white, hue_color32);
        draw_list->PrimVtx(trb, uv_white, hue_color32);
        draw_list->PrimVtx(trc, uv_white, IM_COL32_WHITE);
        draw_list->PrimVtx(tra, uv_white, IM_COL32_BLACK_TRANS);
        draw_list->PrimVtx(trb, uv_white, IM_COL32_BLACK);
        draw_list->PrimVtx(trc, uv_white, IM_COL32_BLACK_TRANS);
        draw_list->AddTriangle(tra, trb, trc, IM_COL32(128,128,128,255), 1.5f);
        sv_cursor_pos = ImLerp(ImLerp(trc, tra, ImSaturate(S)), trb, ImSaturate(1 - V));
    }
    else if (flags & ImGuiColorEditFlags_PickerHueBar)
    {
        // Render SV Square
        draw_list->AddRectFilledMultiColor(picker_pos, picker_pos + ImVec2(sv_picker_size,sv_picker_size), IM_COL32_WHITE, hue_color32, hue_color32, IM_COL32_WHITE);
        draw_list->AddRectFilledMultiColor(picker_pos, picker_pos + ImVec2(sv_picker_size,sv_picker_size), IM_COL32_BLACK_TRANS, IM_COL32_BLACK_TRANS, IM_COL32_BLACK, IM_COL32_BLACK);
        RenderFrameBorder(picker_pos, picker_pos + ImVec2(sv_picker_size,sv_picker_size), 0.0f);
        sv_cursor_pos.x = ImClamp((float)(int)(picker_pos.x + ImSaturate(S)     * sv_picker_size + 0.5f), picker_pos.x + 2, picker_pos.x + sv_picker_size - 2); // Sneakily prevent the circle to stick out too much
        sv_cursor_pos.y = ImClamp((float)(int)(picker_pos.y + ImSaturate(1 - V) * sv_picker_size + 0.5f), picker_pos.y + 2, picker_pos.y + sv_picker_size - 2);

        // Render Hue Bar
        for (int i = 0; i < 6; ++i)
            draw_list->AddRectFilledMultiColor(ImVec2(bar0_pos_x, picker_pos.y + i * (sv_picker_size / 6)), ImVec2(bar0_pos_x + bars_width, picker_pos.y + (i + 1) * (sv_picker_size / 6)), hue_colors[i], hue_colors[i], hue_colors[i + 1], hue_colors[i + 1]);
        float bar0_line_y = (float)(int)(picker_pos.y + H * sv_picker_size + 0.5f);
        RenderFrameBorder(ImVec2(bar0_pos_x, picker_pos.y), ImVec2(bar0_pos_x + bars_width, picker_pos.y + sv_picker_size), 0.0f);
        RenderArrowsForVerticalBar(draw_list, ImVec2(bar0_pos_x - 1, bar0_line_y), ImVec2(bars_triangles_half_sz + 1, bars_triangles_half_sz), bars_width + 2.0f);
    }

    // Render cursor/preview circle (clamp S/V within 0..1 range because floating points colors may lead HSV values to be out of range)
    float sv_cursor_rad = value_changed_sv ? 10.0f : 6.0f;
    draw_list->AddRect(ImVec2(sv_cursor_pos.x - 4, sv_cursor_pos.y - 4), ImVec2(sv_cursor_pos.x + 4, sv_cursor_pos.y + 4), GetColorU32(ImVec4(0.f, 0.f, 0.f, 1.f)));
    draw_list->AddRect(ImVec2(sv_cursor_pos.x - 3, sv_cursor_pos.y - 3), ImVec2(sv_cursor_pos.x + 3, sv_cursor_pos.y + 3), GetColorU32(ImVec4(1.f, 1.f, 1.f, 1.f)));

    // Render alpha bar
    if (alpha_bar)
    {
        float alpha = ImSaturate(*col[3]);
        ImRect bar1_bb(bar1_pos_x, picker_pos.y, bar1_pos_x + bars_width, picker_pos.y + sv_picker_size);
        RenderColorRectWithAlphaCheckerboard(bar1_bb.Min, bar1_bb.Max, IM_COL32(0,0,0,0), bar1_bb.GetWidth() / 2.0f, ImVec2(0.0f, 0.0f));
        draw_list->AddRectFilledMultiColor(bar1_bb.Min, bar1_bb.Max, col32_no_alpha, col32_no_alpha, col32_no_alpha & ~IM_COL32_A_MASK, col32_no_alpha & ~IM_COL32_A_MASK);
        float bar1_line_y = (float)(int)(picker_pos.y + (1.0f - alpha) * sv_picker_size + 0.5f);
        RenderFrameBorder(bar1_bb.Min, bar1_bb.Max, 0.0f);
        RenderArrowsForVerticalBar(draw_list, ImVec2(bar1_pos_x - 1, bar1_line_y), ImVec2(bars_triangles_half_sz + 1, bars_triangles_half_sz), bars_width + 2.0f);
    }

    EndGroup();

    if (value_changed && memcmp(backup_initial_col, col, components * sizeof(float)) == 0)
        value_changed = false;
    if (value_changed)
        MarkItemEdited(window->DC.LastItemId);

    PopID();

    return value_changed;
}

// A little colored square. Return true when clicked.
// FIXME: May want to display/ignore the alpha component in the color display? Yet show it in the tooltip.
// 'desc_id' is not called 'label' because we don't display it next to the button, but only in the tooltip.
bool ImGui::ColorButton(const char* desc_id, const ImVec4& col, ImGuiColorEditFlags flags, ImVec2 size)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiID id = window->GetID(desc_id);
    float default_size = GetFrameHeight();
    if (size.x == 0.0f)
        size.x = default_size;
    if (size.y == 0.0f)
        size.y = default_size;//10.f;//default_size;

   // ImRect bb;

    const ImVec2 default_text_size = CalcTextSize("text");

    const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size);
   /* if (flags & ImGuiColorEditFlags_OnSlider)
        bb = ImRect(window->DC.CursorPos + ImVec2(-1, 3), window->DC.CursorPos + ImVec2(size.x + 1, size.y + 3));
    else if (flags & ImGuiColorEditFlags_OnCheckbox)
        bb = ImRect(window->DC.CursorPos + ImVec2(-1, 4), window->DC.CursorPos + ImVec2(size.x + 1, size.y + 4));
    else
        bb = ImRect(window->DC.CursorPos + ImVec2(-1, 1), window->DC.CursorPos + ImVec2(size.x + 1, default_text_size.y - 1));*/
	
    ItemSize(bb, (size.y >= default_size) ? g.Style.FramePadding.y : 0.0f);
    if (!ItemAdd(bb, id))
        return false;

    bool hovered, held;
    bool pressed = ButtonBehavior(bb, id, &hovered, &held);

    if (flags & ImGuiColorEditFlags_NoAlpha)
        flags &= ~(ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_AlphaPreviewHalf);

    ImVec4 col_without_alpha(col.x, col.y, col.z, 1.0f);
    float grid_step = ImMin(size.x, size.y) / 2.99f;
    float rounding = ImMin(g.Style.FrameRounding, grid_step * 0.5f);
    ImRect bb_inner = bb;
    float off = -0.75f; // The border (using Col_FrameBg) tends to look off when color is near-opaque and rounding is enabled. This offset seemed like a good middle ground to reduce those artifacts.
    bb_inner.Expand(off);
    if ((flags & ImGuiColorEditFlags_AlphaPreviewHalf) && col.w < 1.0f)
    {
        float mid_x = (float)(int)((bb_inner.Min.x + bb_inner.Max.x) * 0.5f + 0.5f);
        RenderColorRectWithAlphaCheckerboard(ImVec2(bb_inner.Min.x + grid_step, bb_inner.Min.y), bb_inner.Max, GetColorU32(col), grid_step, ImVec2(-grid_step + off, off), rounding, ImDrawCornerFlags_TopRight| ImDrawCornerFlags_BotRight);
        window->DrawList->AddRectFilled(bb_inner.Min, ImVec2(mid_x, bb_inner.Max.y), GetColorU32(col_without_alpha), rounding, ImDrawCornerFlags_TopLeft|ImDrawCornerFlags_BotLeft);
    }
    else
    {
        // Because GetColorU32() multiplies by the global style Alpha and we don't want to display a checkerboard if the source code had no alpha
        ImVec4 col_source = (flags & ImGuiColorEditFlags_AlphaPreview) ? col : col_without_alpha;
        if (col_source.w < 1.0f)
            RenderColorRectWithAlphaCheckerboard(bb_inner.Min, bb_inner.Max, GetColorU32(col_source), grid_step, ImVec2(off, off), rounding);
        else
        {
            window->DrawList->AddRectFilled(bb_inner.Min, bb_inner.Max, GetColorU32(ImVec4(0, 0, 0, 1.f)));
            window->DrawList->AddRectFilledMultiColor(bb_inner.Min, bb_inner.Max, GetColorU32(col_source), GetColorU32(col_source), GetColorU32(ImVec4(col_source.x, col_source.y, col_source.z, col_source.w - 0.2f)), GetColorU32(ImVec4(col_source.x, col_source.y, col_source.z, col_source.w - 0.2f)));//window->DrawList->AddRectFilled(bb_inner.Min, bb_inner.Max, GetColorU32(col_source), rounding, ImDrawCornerFlags_All);
        }
    }
    RenderNavHighlight(bb, id);
    if (g.Style.FrameBorderSize > 0.0f)
        RenderFrameBorder(bb.Min, bb.Max, rounding);
    else
        window->DrawList->AddRect(bb.Min, bb.Max, GetColorU32(ImGuiCol_FrameBg), rounding); // Color button are often in need of some sort of border

    // Drag and Drop Source
    // NB: The ActiveId test is merely an optional micro-optimization, BeginDragDropSource() does the same test.
    /*if (g.ActiveId == id && !(flags & ImGuiColorEditFlags_NoDragDrop) && BeginDragDropSource())
    {
        if (flags & ImGuiColorEditFlags_NoAlpha)
            SetDragDropPayload(IMGUI_PAYLOAD_TYPE_COLOR_3F, &col, sizeof(float) * 3, ImGuiCond_Once);
        else
            SetDragDropPayload(IMGUI_PAYLOAD_TYPE_COLOR_4F, &col, sizeof(float) * 4, ImGuiCond_Once);
        ColorButton(desc_id, col, flags);
        SameLine();
        TextUnformatted("Color");
        EndDragDropSource();
    }*/

    // Tooltip
    if (!(flags & ImGuiColorEditFlags_NoTooltip) && hovered)
        ColorTooltip(desc_id, &col.x, flags & (ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_AlphaPreviewHalf));

    if (pressed)
        MarkItemEdited(id);

    return pressed;
}

void ImGui::SetColorEditOptions(ImGuiColorEditFlags flags)
{
    ImGuiContext& g = *GImGui;
    if ((flags & ImGuiColorEditFlags__InputsMask) == 0)
        flags |= ImGuiColorEditFlags__OptionsDefault & ImGuiColorEditFlags__InputsMask;
    if ((flags & ImGuiColorEditFlags__DataTypeMask) == 0)
        flags |= ImGuiColorEditFlags__OptionsDefault & ImGuiColorEditFlags__DataTypeMask;
    if ((flags & ImGuiColorEditFlags__PickerMask) == 0)
        flags |= ImGuiColorEditFlags__OptionsDefault & ImGuiColorEditFlags__PickerMask;
    IM_ASSERT(ImIsPowerOfTwo((int)(flags & ImGuiColorEditFlags__InputsMask)));   // Check only 1 option is selected
    IM_ASSERT(ImIsPowerOfTwo((int)(flags & ImGuiColorEditFlags__DataTypeMask))); // Check only 1 option is selected
    IM_ASSERT(ImIsPowerOfTwo((int)(flags & ImGuiColorEditFlags__PickerMask)));   // Check only 1 option is selected
    g.ColorEditOptions = flags;
}

// Note: only access 3 floats if ImGuiColorEditFlags_NoAlpha flag is set.
void ImGui::ColorTooltip(const char* text, const float* col, ImGuiColorEditFlags flags)
{
    ImGuiContext& g = *GImGui;

    BeginTooltipEx(0, true);
    const char* text_end = text ? FindRenderedTextEnd(text, NULL) : text;
    /*if (text_end > text)
    {
        TextEx(text, text_end);
        Separator();
    }*/

    ImVec2 sz(g.FontSize * 3 + g.Style.FramePadding.y * 2, g.FontSize * 3 + g.Style.FramePadding.y * 2);
    ImVec4 cf(col[0], col[1], col[2], (flags & ImGuiColorEditFlags_NoAlpha) ? 1.0f : col[3]);
    int cr = IM_F32_TO_INT8_SAT(col[0]), cg = IM_F32_TO_INT8_SAT(col[1]), cb = IM_F32_TO_INT8_SAT(col[2]), ca = (flags & ImGuiColorEditFlags_NoAlpha) ? 255 : IM_F32_TO_INT8_SAT(col[3]);
    //ColorButton("##preview", cf, (flags & (ImGuiColorEditFlags__InputMask | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_AlphaPreviewHalf)) | ImGuiColorEditFlags_NoTooltip, sz);
    //SameLine();
    if (flags & ImGuiColorEditFlags_NoAlpha)
        Text("R: %d, G: %d, B: %d", cr, cg, cb, cr, cg, cb, col[0], col[1], col[2]);
    else
        Text("R:%d, G:%d, B:%d, A:%d", cr, cg, cb, ca, cr, cg, cb, ca, col[0], col[1], col[2], col[3]);

    EndTooltip();

    /*ImGuiContext& g = *GImGui;

    int cr = IM_F32_TO_INT8_SAT(col[0]), cg = IM_F32_TO_INT8_SAT(col[1]), cb = IM_F32_TO_INT8_SAT(col[2]), ca = (flags & ImGuiColorEditFlags_NoAlpha) ? 255 : IM_F32_TO_INT8_SAT(col[3]);
    BeginTooltipEx(0, true);

    const char* text_end = text ? FindRenderedTextEnd(text, NULL) : text;
    if (text_end > text)
    {
        TextUnformatted(text, text_end);
        Separator();
    }

    ImVec2 sz(g.FontSize * 3 + g.Style.FramePadding.y * 2, g.FontSize * 3 + g.Style.FramePadding.y * 2);
    ColorButton("##preview", ImVec4(col[0], col[1], col[2], col[3]), (flags & (ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_AlphaPreviewHalf)) | ImGuiColorEditFlags_NoTooltip, sz);
    SameLine();
    if (flags & ImGuiColorEditFlags_NoAlpha)
        Text("#%02X%02X%02X\nR: %d, G: %d, B: %d\n(%.3f, %.3f, %.3f)", cr, cg, cb, cr, cg, cb, col[0], col[1], col[2]);
    else
        Text("#%02X%02X%02X%02X\nR:%d, G:%d, B:%d, A:%d\n(%.3f, %.3f, %.3f, %.3f)", cr, cg, cb, ca, cr, cg, cb, ca, col[0], col[1], col[2], col[3]);
    EndTooltip();*/
}

void ImGui::ColorEditOptionsPopup(float* col, ImGuiColorEditFlags flags)
{
    bool allow_opt_inputs = !(flags & ImGuiColorEditFlags__InputsMask);
    bool allow_opt_datatype = !(flags & ImGuiColorEditFlags__DataTypeMask);
    if ((!allow_opt_inputs && !allow_opt_datatype) || !BeginPopup("context"))
        return;
    ImGuiContext& g = *GImGui;
    ImGuiColorEditFlags opts = g.ColorEditOptions;
    /*if (allow_opt_inputs)
    {
        if (RadioButton("RGB", (opts & ImGuiColorEditFlags_RGB) != 0)) opts = (opts & ~ImGuiColorEditFlags__InputsMask) | ImGuiColorEditFlags_RGB;
        if (RadioButton("HSV", (opts & ImGuiColorEditFlags_HSV) != 0)) opts = (opts & ~ImGuiColorEditFlags__InputsMask) | ImGuiColorEditFlags_HSV;
        if (RadioButton("HEX", (opts & ImGuiColorEditFlags_HEX) != 0)) opts = (opts & ~ImGuiColorEditFlags__InputsMask) | ImGuiColorEditFlags_HEX;
    }
    if (allow_opt_datatype)
    {
        if (allow_opt_inputs) Separator();
        if (RadioButton("0..255",     (opts & ImGuiColorEditFlags_Uint8) != 0)) opts = (opts & ~ImGuiColorEditFlags__DataTypeMask) | ImGuiColorEditFlags_Uint8;
        if (RadioButton("0.00..1.00", (opts & ImGuiColorEditFlags_Float) != 0)) opts = (opts & ~ImGuiColorEditFlags__DataTypeMask) | ImGuiColorEditFlags_Float;
    }

    if (allow_opt_inputs || allow_opt_datatype)
        Separator();*/

    static float color_copy[4];

    if (Button("copy", ImVec2(50, 0)))
    {
        color_copy[0] = col[0];
        color_copy[1] = col[1];
        color_copy[2] = col[2];
        color_copy[3] = col[3];
    }
    if (Button("paste", ImVec2(50, 0)))
    {
        col[0] = color_copy[0];
        col[1] = color_copy[1];
        col[2] = color_copy[2];
        col[3] = color_copy[3];
    }
    //OpenPopup("Copy");
    if (BeginPopup("Copy"))
    {
        int cr = IM_F32_TO_INT8_SAT(col[0]), cg = IM_F32_TO_INT8_SAT(col[1]), cb = IM_F32_TO_INT8_SAT(col[2]), ca = (flags & ImGuiColorEditFlags_NoAlpha) ? 255 : IM_F32_TO_INT8_SAT(col[3]);
        char buf[64];
        ImFormatString(buf, IM_ARRAYSIZE(buf), "(%.3ff, %.3ff, %.3ff, %.3ff)", col[0], col[1], col[2], (flags & ImGuiColorEditFlags_NoAlpha) ? 1.0f : col[3]);
        if (Selectable(buf))
            SetClipboardText(buf);
        ImFormatString(buf, IM_ARRAYSIZE(buf), "(%d,%d,%d,%d)", cr, cg, cb, ca);
        if (Selectable(buf))
            SetClipboardText(buf);
        if (flags & ImGuiColorEditFlags_NoAlpha)
            ImFormatString(buf, IM_ARRAYSIZE(buf), "0x%02X%02X%02X", cr, cg, cb);
        else
            ImFormatString(buf, IM_ARRAYSIZE(buf), "0x%02X%02X%02X%02X", cr, cg, cb, ca);
        if (Selectable(buf))
            SetClipboardText(buf);
        EndPopup();
    }

    g.ColorEditOptions = opts;
    EndPopup();
}

void ImGui::ColorPickerOptionsPopup(const float* ref_col, ImGuiColorEditFlags flags)
{
    /*bool allow_opt_picker = !(flags & ImGuiColorEditFlags__PickerMask);
    bool allow_opt_alpha_bar = !(flags & ImGuiColorEditFlags_NoAlpha) && !(flags & ImGuiColorEditFlags_AlphaBar);
    if ((!allow_opt_picker && !allow_opt_alpha_bar) || !BeginPopup("context"))
        return;
    ImGuiContext& g = *GImGui;
    if (allow_opt_picker)
    {
        ImVec2 picker_size(g.FontSize * 8, ImMax(g.FontSize * 8 - (GetFrameHeight() + g.Style.ItemInnerSpacing.x), 1.0f)); // FIXME: Picker size copied from main picker function
        PushItemWidth(picker_size.x);
        for (int picker_type = 0; picker_type < 2; picker_type++)
        {
            // Draw small/thumbnail version of each picker type (over an invisible button for selection)
            if (picker_type > 0) Separator();
            PushID(picker_type);
            ImGuiColorEditFlags picker_flags = ImGuiColorEditFlags_NoInputs|ImGuiColorEditFlags_NoOptions|ImGuiColorEditFlags_NoLabel|ImGuiColorEditFlags_NoSidePreview|(flags & ImGuiColorEditFlags_NoAlpha);
            if (picker_type == 0) picker_flags |= ImGuiColorEditFlags_PickerHueBar;
            if (picker_type == 1) picker_flags |= ImGuiColorEditFlags_PickerHueWheel;
            ImVec2 backup_pos = GetCursorScreenPos();
            if (Selectable("##selectable", false, 0, picker_size)) // By default, Selectable() is closing popup
                g.ColorEditOptions = (g.ColorEditOptions & ~ImGuiColorEditFlags__PickerMask) | (picker_flags & ImGuiColorEditFlags__PickerMask);
            SetCursorScreenPos(backup_pos);
            ImVec4 dummy_ref_col;
            memcpy(&dummy_ref_col, ref_col, sizeof(float) * ((picker_flags & ImGuiColorEditFlags_NoAlpha) ? 3 : 4));
            ColorPicker4("##dummypicker", &dummy_ref_col.x, picker_flags);
            PopID();
        }
        PopItemWidth();
    }
    if (allow_opt_alpha_bar)
    {
        if (allow_opt_picker) Separator();
        CheckboxFlags("Alpha Bar", (unsigned int*)&g.ColorEditOptions, ImGuiColorEditFlags_AlphaBar);
    }
    EndPopup();*/
}

//-------------------------------------------------------------------------
// [SECTION] Widgets: TreeNode, CollapsingHeader, etc.
//-------------------------------------------------------------------------
// - TreeNode()
// - TreeNodeV()
// - TreeNodeEx()
// - TreeNodeExV()
// - TreeNodeBehavior() [Internal]
// - TreePush()
// - TreePop()
// - TreeAdvanceToLabelPos()
// - GetTreeNodeToLabelSpacing()
// - SetNextTreeNodeOpen()
// - CollapsingHeader()
//-------------------------------------------------------------------------

bool ImGui::TreeNode(const char* str_id, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    bool is_open = TreeNodeExV(str_id, 0, fmt, args);
    va_end(args);
    return is_open;
}

bool ImGui::TreeNode(const void* ptr_id, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    bool is_open = TreeNodeExV(ptr_id, 0, fmt, args);
    va_end(args);
    return is_open;
}

bool ImGui::TreeNode(const char* label)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;
    return TreeNodeBehavior(window->GetID(label), 0, label, NULL);
}

bool ImGui::TreeNodeV(const char* str_id, const char* fmt, va_list args)
{
    return TreeNodeExV(str_id, 0, fmt, args);
}

bool ImGui::TreeNodeV(const void* ptr_id, const char* fmt, va_list args)
{
    return TreeNodeExV(ptr_id, 0, fmt, args);
}

bool ImGui::TreeNodeEx(const char* label, ImGuiTreeNodeFlags flags)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    return TreeNodeBehavior(window->GetID(label), flags, label, NULL);
}

bool ImGui::TreeNodeEx(const char* str_id, ImGuiTreeNodeFlags flags, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    bool is_open = TreeNodeExV(str_id, flags, fmt, args);
    va_end(args);
    return is_open;
}

bool ImGui::TreeNodeEx(const void* ptr_id, ImGuiTreeNodeFlags flags, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    bool is_open = TreeNodeExV(ptr_id, flags, fmt, args);
    va_end(args);
    return is_open;
}

bool ImGui::TreeNodeExV(const char* str_id, ImGuiTreeNodeFlags flags, const char* fmt, va_list args)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const char* label_end = g.TempBuffer + ImFormatStringV(g.TempBuffer, IM_ARRAYSIZE(g.TempBuffer), fmt, args);
    return TreeNodeBehavior(window->GetID(str_id), flags, g.TempBuffer, label_end);
}

bool ImGui::TreeNodeExV(const void* ptr_id, ImGuiTreeNodeFlags flags, const char* fmt, va_list args)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const char* label_end = g.TempBuffer + ImFormatStringV(g.TempBuffer, IM_ARRAYSIZE(g.TempBuffer), fmt, args);
    return TreeNodeBehavior(window->GetID(ptr_id), flags, g.TempBuffer, label_end);
}

bool ImGui::TreeNodeBehaviorIsOpen(ImGuiID id, ImGuiTreeNodeFlags flags)
{
    if (flags & ImGuiTreeNodeFlags_Leaf)
        return true;

    // We only write to the tree storage if the user clicks (or explicitly use SetNextTreeNode*** functions)
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    ImGuiStorage* storage = window->DC.StateStorage;

    bool is_open;
    if (g.NextTreeNodeOpenCond != 0)
    {
        if (g.NextTreeNodeOpenCond & ImGuiCond_Always)
        {
            is_open = g.NextTreeNodeOpenVal;
            storage->SetInt(id, is_open);
        }
        else
        {
            // We treat ImGuiCond_Once and ImGuiCond_FirstUseEver the same because tree node state are not saved persistently.
            const int stored_value = storage->GetInt(id, -1);
            if (stored_value == -1)
            {
                is_open = g.NextTreeNodeOpenVal;
                storage->SetInt(id, is_open);
            }
            else
            {
                is_open = stored_value != 0;
            }
        }
        g.NextTreeNodeOpenCond = 0;
    }
    else
    {
        is_open = storage->GetInt(id, (flags & ImGuiTreeNodeFlags_DefaultOpen) ? 1 : 0) != 0;
    }

    // When logging is enabled, we automatically expand tree nodes (but *NOT* collapsing headers.. seems like sensible behavior).
    // NB- If we are above max depth we still allow manually opened nodes to be logged.
    if (g.LogEnabled && !(flags & ImGuiTreeNodeFlags_NoAutoOpenOnLog) && window->DC.TreeDepth < g.LogAutoExpandMaxDepth)
        is_open = true;

    return is_open;
}

bool ImGui::TreeNodeBehavior(ImGuiID id, ImGuiTreeNodeFlags flags, const char* label, const char* label_end)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const bool display_frame = (flags & ImGuiTreeNodeFlags_Framed) != 0;
    const ImVec2 padding = (display_frame || (flags & ImGuiTreeNodeFlags_FramePadding)) ? style.FramePadding : ImVec2(style.FramePadding.x, 0.0f);

    if (!label_end)
        label_end = FindRenderedTextEnd(label);
    const ImVec2 label_size = CalcTextSize(label, label_end, false);

    // We vertically grow up to current line height up the typical widget height.
    const float text_base_offset_y = ImMax(padding.y, window->DC.CurrentLineTextBaseOffset); // Latch before ItemSize changes it
    const float frame_height = ImMax(ImMin(window->DC.CurrentLineSize.y, g.FontSize + style.FramePadding.y*2), label_size.y + padding.y*2);
    ImRect frame_bb = ImRect(window->DC.CursorPos, ImVec2(window->Pos.x + GetContentRegionMax().x, window->DC.CursorPos.y + frame_height));
    if (display_frame)
    {
        // Framed header expand a little outside the default padding
        frame_bb.Min.x -= (float)(int)(window->WindowPadding.x*0.5f) - 1;
        frame_bb.Max.x += (float)(int)(window->WindowPadding.x*0.5f) - 1;
    }

    const float text_offset_x = (g.FontSize + (display_frame ? padding.x*3 : padding.x*2));   // Collapser arrow width + Spacing
    const float text_width = g.FontSize + (label_size.x > 0.0f ? label_size.x + padding.x*2 : 0.0f);   // Include collapser
    ItemSize(ImVec2(text_width, frame_height), text_base_offset_y);

    // For regular tree nodes, we arbitrary allow to click past 2 worth of ItemSpacing
    // (Ideally we'd want to add a flag for the user to specify if we want the hit test to be done up to the right side of the content or not)
    const ImRect interact_bb = display_frame ? frame_bb : ImRect(frame_bb.Min.x, frame_bb.Min.y, frame_bb.Min.x + text_width + style.ItemSpacing.x*2, frame_bb.Max.y);
    bool is_open = TreeNodeBehaviorIsOpen(id, flags);
    bool is_leaf = (flags & ImGuiTreeNodeFlags_Leaf) != 0;

    // Store a flag for the current depth to tell if we will allow closing this node when navigating one of its child.
    // For this purpose we essentially compare if g.NavIdIsAlive went from 0 to 1 between TreeNode() and TreePop().
    // This is currently only support 32 level deep and we are fine with (1 << Depth) overflowing into a zero.
    if (is_open && !g.NavIdIsAlive && (flags & ImGuiTreeNodeFlags_NavLeftJumpsBackHere) && !(flags & ImGuiTreeNodeFlags_NoTreePushOnOpen))
        window->DC.TreeDepthMayJumpToParentOnPop |= (1 << window->DC.TreeDepth);

    bool item_add = ItemAdd(interact_bb, id);
    window->DC.LastItemStatusFlags |= ImGuiItemStatusFlags_HasDisplayRect;
    window->DC.LastItemDisplayRect = frame_bb;

    if (!item_add)
    {
        if (is_open && !(flags & ImGuiTreeNodeFlags_NoTreePushOnOpen))
            TreePushRawID(id);
        IMGUI_TEST_ENGINE_ITEM_INFO(window->DC.LastItemId, label, window->DC.ItemFlags | (is_leaf ? 0 : ImGuiItemStatusFlags_Openable) | (is_open ? ImGuiItemStatusFlags_Opened : 0));
        return is_open;
    }

    // Flags that affects opening behavior:
    // - 0 (default) .................... single-click anywhere to open
    // - OpenOnDoubleClick .............. double-click anywhere to open
    // - OpenOnArrow .................... single-click on arrow to open
    // - OpenOnDoubleClick|OpenOnArrow .. single-click on arrow or double-click anywhere to open
    ImGuiButtonFlags button_flags = ImGuiButtonFlags_NoKeyModifiers;
    if (flags & ImGuiTreeNodeFlags_AllowItemOverlap)
        button_flags |= ImGuiButtonFlags_AllowItemOverlap;
    if (flags & ImGuiTreeNodeFlags_OpenOnDoubleClick)
        button_flags |= ImGuiButtonFlags_PressedOnDoubleClick | ((flags & ImGuiTreeNodeFlags_OpenOnArrow) ? ImGuiButtonFlags_PressedOnClickRelease : 0);
    if (!is_leaf)
        button_flags |= ImGuiButtonFlags_PressedOnDragDropHold;

    bool selected = (flags & ImGuiTreeNodeFlags_Selected) != 0;
    bool hovered, held;
    bool pressed = ButtonBehavior(interact_bb, id, &hovered, &held, button_flags);
    bool toggled = false;
    if (!is_leaf)
    {
        if (pressed)
        {
            toggled = !(flags & (ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick)) || (g.NavActivateId == id);
            if (flags & ImGuiTreeNodeFlags_OpenOnArrow)
                toggled |= IsMouseHoveringRect(interact_bb.Min, ImVec2(interact_bb.Min.x + text_offset_x, interact_bb.Max.y)) && (!g.NavDisableMouseHover);
            if (flags & ImGuiTreeNodeFlags_OpenOnDoubleClick)
                toggled |= g.IO.MouseDoubleClicked[0];
            if (g.DragDropActive && is_open) // When using Drag and Drop "hold to open" we keep the node highlighted after opening, but never close it again.
                toggled = false;
        }

        if (g.NavId == id && g.NavMoveRequest && g.NavMoveDir == ImGuiDir_Left && is_open)
        {
            toggled = true;
            NavMoveRequestCancel();
        }
        if (g.NavId == id && g.NavMoveRequest && g.NavMoveDir == ImGuiDir_Right && !is_open) // If there's something upcoming on the line we may want to give it the priority?
        {
            toggled = true;
            NavMoveRequestCancel();
        }

        if (toggled)
        {
            is_open = !is_open;
            window->DC.StateStorage->SetInt(id, is_open);
        }
    }
    if (flags & ImGuiTreeNodeFlags_AllowItemOverlap)
        SetItemAllowOverlap();

    // Render
    const ImU32 col = GetColorU32((held && hovered) ? ImGuiCol_HeaderActive : hovered ? ImGuiCol_HeaderHovered : ImGuiCol_Header);
    const ImVec2 text_pos = frame_bb.Min + ImVec2(text_offset_x, text_base_offset_y);
    ImGuiNavHighlightFlags nav_highlight_flags = ImGuiNavHighlightFlags_TypeThin;
    if (display_frame)
    {
        // Framed type
        RenderFrame(frame_bb.Min, frame_bb.Max, col, true, style.FrameRounding);
        RenderNavHighlight(frame_bb, id, nav_highlight_flags);
        RenderArrow(frame_bb.Min + ImVec2(padding.x, text_base_offset_y), is_open ? ImGuiDir_Down : ImGuiDir_Right, 1.0f);
        if (g.LogEnabled)
        {
            // NB: '##' is normally used to hide text (as a library-wide feature), so we need to specify the text range to make sure the ## aren't stripped out here.
            const char log_prefix[] = "\n##";
            const char log_suffix[] = "##";
            LogRenderedText(&text_pos, log_prefix, log_prefix+3);
            RenderTextClipped(text_pos, frame_bb.Max, label, label_end, &label_size);
            LogRenderedText(&text_pos, log_suffix+1, log_suffix+3);
        }
        else
        {
            RenderTextClipped(text_pos, frame_bb.Max, label, label_end, &label_size);
        }
    }
    else
    {
        // Unframed typed for tree nodes
        if (hovered || selected)
        {
            RenderFrame(frame_bb.Min, frame_bb.Max, col, false);
            RenderNavHighlight(frame_bb, id, nav_highlight_flags);
        }

        if (flags & ImGuiTreeNodeFlags_Bullet)
            RenderBullet(frame_bb.Min + ImVec2(text_offset_x * 0.5f, g.FontSize*0.50f + text_base_offset_y));
        else if (!is_leaf)
            RenderArrow(frame_bb.Min + ImVec2(padding.x, g.FontSize*0.15f + text_base_offset_y), is_open ? ImGuiDir_Down : ImGuiDir_Right, 0.70f);
        if (g.LogEnabled)
            LogRenderedText(&text_pos, ">");
        RenderText(text_pos, label, label_end, false);
    }

    if (is_open && !(flags & ImGuiTreeNodeFlags_NoTreePushOnOpen))
        TreePushRawID(id);
    IMGUI_TEST_ENGINE_ITEM_INFO(id, label, window->DC.ItemFlags | (is_leaf ? 0 : ImGuiItemStatusFlags_Openable) | (is_open ? ImGuiItemStatusFlags_Opened : 0));
    return is_open;
}

void ImGui::TreePush(const char* str_id)
{
    ImGuiWindow* window = GetCurrentWindow();
    Indent();
    window->DC.TreeDepth++;
    PushID(str_id ? str_id : "#TreePush");
}

void ImGui::TreePush(const void* ptr_id)
{
    ImGuiWindow* window = GetCurrentWindow();
    Indent();
    window->DC.TreeDepth++;
    PushID(ptr_id ? ptr_id : (const void*)"#TreePush");
}

void ImGui::TreePushRawID(ImGuiID id)
{
    ImGuiWindow* window = GetCurrentWindow();
    Indent();
    window->DC.TreeDepth++;
    window->IDStack.push_back(id);
}

void ImGui::TreePop()
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    Unindent();

    window->DC.TreeDepth--;
    if (g.NavMoveDir == ImGuiDir_Left && g.NavWindow == window && NavMoveRequestButNoResultYet())
        if (g.NavIdIsAlive && (window->DC.TreeDepthMayJumpToParentOnPop & (1 << window->DC.TreeDepth)))
        {
            SetNavID(window->IDStack.back(), g.NavLayer);
            NavMoveRequestCancel();
        }
    window->DC.TreeDepthMayJumpToParentOnPop &= (1 << window->DC.TreeDepth) - 1;

    IM_ASSERT(window->IDStack.Size > 1); // There should always be 1 element in the IDStack (pushed during window creation). If this triggers you called TreePop/PopID too much.
    PopID();
}

void ImGui::TreeAdvanceToLabelPos()
{
    ImGuiContext& g = *GImGui;
    g.CurrentWindow->DC.CursorPos.x += GetTreeNodeToLabelSpacing();
}

// Horizontal distance preceding label when using TreeNode() or Bullet()
float ImGui::GetTreeNodeToLabelSpacing()
{
    ImGuiContext& g = *GImGui;
    return g.FontSize + (g.Style.FramePadding.x * 2.0f);
}

void ImGui::SetNextTreeNodeOpen(bool is_open, ImGuiCond cond)
{
    ImGuiContext& g = *GImGui;
    if (g.CurrentWindow->SkipItems)
        return;
    g.NextTreeNodeOpenVal = is_open;
    g.NextTreeNodeOpenCond = cond ? cond : ImGuiCond_Always;
}

// CollapsingHeader returns true when opened but do not indent nor push into the ID stack (because of the ImGuiTreeNodeFlags_NoTreePushOnOpen flag).
// This is basically the same as calling TreeNodeEx(label, ImGuiTreeNodeFlags_CollapsingHeader). You can remove the _NoTreePushOnOpen flag if you want behavior closer to normal TreeNode().
bool ImGui::CollapsingHeader(const char* label, ImGuiTreeNodeFlags flags)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    return TreeNodeBehavior(window->GetID(label), flags | ImGuiTreeNodeFlags_CollapsingHeader, label);
}

bool ImGui::CollapsingHeader(const char* label, bool* p_open, ImGuiTreeNodeFlags flags)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    if (p_open && !*p_open)
        return false;

    ImGuiID id = window->GetID(label);
    bool is_open = TreeNodeBehavior(id, flags | ImGuiTreeNodeFlags_CollapsingHeader | (p_open ? ImGuiTreeNodeFlags_AllowItemOverlap : 0), label);
    if (p_open)
    {
        // Create a small overlapping close button // FIXME: We can evolve this into user accessible helpers to add extra buttons on title bars, headers, etc.
        ImGuiContext& g = *GImGui;
        ImGuiItemHoveredDataBackup last_item_backup;
        float button_radius = g.FontSize * 0.5f;
        ImVec2 button_center = ImVec2(ImMin(window->DC.LastItemRect.Max.x, window->ClipRect.Max.x) - g.Style.FramePadding.x - button_radius, window->DC.LastItemRect.GetCenter().y);
        if (CloseButton(window->GetID((void*)((intptr_t)id+1)), button_center, button_radius))
            *p_open = false;
        last_item_backup.Restore();
    }

    return is_open;
}

//-------------------------------------------------------------------------
// [SECTION] Widgets: Selectable
//-------------------------------------------------------------------------
// - Selectable()
//-------------------------------------------------------------------------

// Tip: pass a non-visible label (e.g. "##dummy") then you can use the space to draw other text or image.
// But you need to make sure the ID is unique, e.g. enclose calls in PushID/PopID or use ##unique_id.
bool ImGui::Selectable(const char* label, bool selected, ImGuiSelectableFlags flags, const ImVec2& size_arg)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;

    if ((flags & ImGuiSelectableFlags_SpanAllColumns) && window->DC.ColumnsSet) // FIXME-OPT: Avoid if vertically clipped.
        PopClipRect();

    ImGuiID id = window->GetID(label);
    ImVec2 label_size = CalcTextSize(label, NULL, true);
    ImVec2 size(size_arg.x != 0.0f ? size_arg.x : label_size.x, size_arg.y != 0.0f ? size_arg.y : label_size.y);
    ImVec2 pos = window->DC.CursorPos;
    pos.y += window->DC.CurrentLineTextBaseOffset;
    ImRect bb_inner(pos + ImVec2(4, 0), pos + size);
    ItemSize(bb_inner);

    // Fill horizontal space.
    ImVec2 window_padding = window->WindowPadding;
    float max_x = (flags & ImGuiSelectableFlags_SpanAllColumns) ? GetWindowContentRegionMax().x : GetContentRegionMax().x;
    float w_draw = ImMax(label_size.x, window->Pos.x + max_x - window_padding.x - pos.x);
    ImVec2 size_draw((size_arg.x != 0 && !(flags & ImGuiSelectableFlags_DrawFillAvailWidth)) ? size_arg.x : w_draw, size_arg.y != 0.0f ? size_arg.y : size.y);
    ImRect bb(pos, pos + size_draw);
    if (size_arg.x == 0.0f || (flags & ImGuiSelectableFlags_DrawFillAvailWidth))
        bb.Max.x += window_padding.x;

    // Selectables are tightly packed together, we extend the box to cover spacing between selectable.
    float spacing_L = (float)(int)(style.ItemSpacing.x * 0.5f);
    float spacing_U = (float)(int)(style.ItemSpacing.y * 0.5f);
    float spacing_R = style.ItemSpacing.x - spacing_L;
    float spacing_D = style.ItemSpacing.y - spacing_U;
    bb.Min.x -= spacing_L;
    bb.Min.y -= spacing_U;
    bb.Max.x += spacing_R;
    bb.Max.y += spacing_D;
    if (!ItemAdd(bb, id))
    {
        if ((flags & ImGuiSelectableFlags_SpanAllColumns) && window->DC.ColumnsSet)
            PushColumnClipRect();
        return false;
    }

    // We use NoHoldingActiveID on menus so user can click and _hold_ on a menu then drag to browse child entries
    ImGuiButtonFlags button_flags = 0;
    if (flags & ImGuiSelectableFlags_NoHoldingActiveID) button_flags |= ImGuiButtonFlags_NoHoldingActiveID;
    if (flags & ImGuiSelectableFlags_PressedOnClick) button_flags |= ImGuiButtonFlags_PressedOnClick;
    if (flags & ImGuiSelectableFlags_PressedOnRelease) button_flags |= ImGuiButtonFlags_PressedOnRelease;
    if (flags & ImGuiSelectableFlags_Disabled) button_flags |= ImGuiButtonFlags_Disabled;
    if (flags & ImGuiSelectableFlags_AllowDoubleClick) button_flags |= ImGuiButtonFlags_PressedOnClickRelease | ImGuiButtonFlags_PressedOnDoubleClick;
    if (flags & ImGuiSelectableFlags_Disabled)
        selected = false;

    bool hovered, held;
    bool pressed = ButtonBehavior(bb, id, &hovered, &held, button_flags);
    // Hovering selectable with mouse updates NavId accordingly so navigation can be resumed with gamepad/keyboard (this doesn't happen on most widgets)
    if (pressed || hovered)
        if (!g.NavDisableMouseHover && g.NavWindow == window && g.NavLayer == window->DC.NavLayerCurrent)
        {
            g.NavDisableHighlight = true;
            SetNavID(id, window->DC.NavLayerCurrent);
        }
    if (pressed)
        MarkItemEdited(id);

    // Render
    if (hovered || selected)
    {
        const ImU32 col = GetColorU32((held && hovered) ? ImGuiCol_HeaderActive : hovered ? ImGuiCol_HeaderHovered : ImGuiCol_Header);
        RenderFrame(bb.Min, bb.Max, col, false, 0.0f);
        RenderNavHighlight(bb, id, ImGuiNavHighlightFlags_TypeThin | ImGuiNavHighlightFlags_NoRounding);
    }

    if ((flags & ImGuiSelectableFlags_SpanAllColumns) && window->DC.ColumnsSet)
    {
        PushColumnClipRect();
        bb.Max.x -= (GetContentRegionMax().x - max_x);
    }

    if (flags & ImGuiSelectableFlags_Disabled) PushStyleColor(ImGuiCol_Text, g.Style.Colors[ImGuiCol_TextDisabled]);
    RenderTextClipped(bb_inner.Min, bb_inner.Max, label, NULL, &label_size, style.SelectableTextAlign, &bb);
    if (flags & ImGuiSelectableFlags_Disabled) PopStyleColor();

    // Automatically close popups
    if (pressed && (window->Flags & ImGuiWindowFlags_Popup) && !(flags & ImGuiSelectableFlags_DontClosePopups) && !(window->DC.ItemFlags & ImGuiItemFlags_SelectableDontClosePopup))
        CloseCurrentPopup();
    return pressed;
}

bool ImGui::Selectable(const char* label, bool* p_selected, ImGuiSelectableFlags flags, const ImVec2& size_arg)
{
    if (Selectable(label, *p_selected, flags, size_arg))
    {
        *p_selected = !*p_selected;
        return true;
    }
    return false;
}

//-------------------------------------------------------------------------
// [SECTION] Widgets: ListBox
//-------------------------------------------------------------------------
// - ListBox()
// - ListBoxHeader()
// - ListBoxFooter()
//-------------------------------------------------------------------------

// FIXME: In principle this function should be called BeginListBox(). We should rename it after re-evaluating if we want to keep the same signature.
// Helper to calculate the size of a listbox and display a label on the right.
// Tip: To have a list filling the entire window width, PushItemWidth(-1) and pass an non-visible label e.g. "##empty"
bool ImGui::ListBoxHeader(const char* label, const ImVec2& size_arg)
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	const ImGuiStyle& style = GetStyle();
	const ImGuiID id = GetID(label);
	const ImVec2 label_size = CalcTextSize(label, NULL, true);

	const float global_offset_float = 17 + style.ItemInnerSpacing.x;
	const ImVec2 global_offset(global_offset_float, label_size.x > 0 ? label_size.y + 5 : 0);

    ImVec2 offset = ImVec2(global_offset_float, 0.f);

    const ImVec2 pos_wind = window->Pos;
    float offset_scroll = 0;
    if (window->ScrollbarY)
        offset_scroll = style.ScrollbarSize;

    ImVec2 pos_end = ImVec2(pos_wind.x + (window->Size.x - (global_offset_float * 2) + 9.f - offset_scroll), window->DC.CursorPos.y);

    // Size default to hold ~7 items. Fractional number of items helps seeing that we can scroll down/up without looking at scrollbar.
    ImVec2 size = CalcItemSize(size_arg, CalcItemWidth(), GetTextLineHeightWithSpacing() * 7.4f + style.ItemSpacing.y);
    ImVec2 frame_size = ImVec2(size.x, ImMax(size.y, label_size.y));
    ImRect frame_bb(window->DC.CursorPos + global_offset, pos_end + ImVec2(0, global_offset.y) + ImVec2(0, frame_size.y));
    ImRect frame_bb_old(window->DC.CursorPos, window->DC.CursorPos + frame_size);
    ImRect bb(frame_bb.Min, frame_bb.Max /*+ ImVec2(label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f, 0.0f)*/);
    window->DC.LastItemRect = bb; // Forward storage for ListBoxFooter.. dodgy.


	window->DC.CursorPos.x += global_offset.x;
    if (label_size.x > 0)
    {
        window->DC.CursorPos.y += global_offset.y;
    }

	if (!IsRectVisible(bb.Min, bb.Max))
	{
		ItemSize(bb.GetSize(), style.FramePadding.y);
		ItemAdd(bb, 0, &frame_bb);
		return false;
	}

	BeginGroup();
    if (label_size.x > 0)
    {
        window->DrawList->AddRectFilled(ImVec2(frame_bb.Min.x, frame_bb_old.Min.y), ImVec2(frame_bb.Max.x, frame_bb_old.Min.y + 18), GetColorU32(ImGuiCol_TitleBg), style.FrameRounding, ImDrawCornerFlags_Top);
        window->DrawList->AddRectFilled(ImVec2(frame_bb.Min.x, frame_bb_old.Min.y), ImVec2(frame_bb.Max.x, frame_bb_old.Min.y + 4), GetColorU32(ImGuiCol_SliderGrab), style.FrameRounding, ImDrawCornerFlags_Top);
        window->DrawList->AddText(ImVec2(frame_bb.Min.x + 4, frame_bb_old.Min.y + 4), GetColorU32(ImGuiCol_Text), label, FindRenderedTextEnd(label, NULL));
    }

	BeginChildFrame(id, frame_bb.GetSize(), ImGuiWindowFlags_ListBoxWindowTitle);
	return true;
}

// FIXME: In principle this function should be called EndListBox(). We should rename it after re-evaluating if we want to keep the same signature.
bool ImGui::ListBoxHeader(const char* label, int items_count, int height_in_items)
{
    // Size default to hold ~7.25 items.
    // We add +25% worth of item height to allow the user to see at a glance if there are more items up/down, without looking at the scrollbar.
    // We don't add this extra bit if items_count <= height_in_items. It is slightly dodgy, because it means a dynamic list of items will make the widget resize occasionally when it crosses that size.
    // I am expecting that someone will come and complain about this behavior in a remote future, then we can advise on a better solution.
    if (height_in_items < 0)
        height_in_items = ImMin(items_count, 7);
    const ImGuiStyle& style = GetStyle();
    float height_in_items_f = (height_in_items < items_count) ? (height_in_items + 0.25f) : (height_in_items + 0.00f);

    // We include ItemSpacing.y so that a list sized for the exact number of items doesn't make a scrollbar appears. We could also enforce that by passing a flag to BeginChild().
    ImVec2 size;
    size.x = 0.0f;
    size.y = GetTextLineHeightWithSpacing() * height_in_items_f + style.FramePadding.y * 2.0f;
    return ListBoxHeader(label, size);
}

// FIXME: In principle this function should be called EndListBox(). We should rename it after re-evaluating if we want to keep the same signature.
void ImGui::ListBoxFooter()
{
    ImGuiWindow* parent_window = GetCurrentWindow()->ParentWindow;
    const ImRect bb = parent_window->DC.LastItemRect;
    const ImGuiStyle& style = GetStyle();

    EndChildFrame();

    SameLine();
    parent_window->DC.CursorPos = bb.Min;
    ItemSize(bb, style.FramePadding.y);
    EndGroup();
}

bool ImGui::ListBox(const char* label, int* current_item, const char* const items[], int items_count, int height_items)
{
    const bool value_changed = ListBox(label, current_item, Items_ArrayGetter, (void*)items, items_count, height_items);
    return value_changed;
}

bool ImGui::ListBox(const char* label, int* current_item, bool (*items_getter)(void*, int, const char**), void* data, int items_count, int height_in_items)
{
    if (!ListBoxHeader(label, items_count, height_in_items))
        return false;

    // Assume all items have even height (= 1 line of text). If you need items of different or variable sizes you can create a custom version of ListBox() in your code without using the clipper.
    ImGuiContext& g = *GImGui;
    bool value_changed = false;
    ImGuiListClipper clipper(items_count, GetTextLineHeightWithSpacing()); // We know exactly our line height here so we pass it as a minor optimization, but generally you don't need to.
    while (clipper.Step())
        for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
        {
            const bool item_selected = (i == *current_item);
            const char* item_text;
            if (!items_getter(data, i, &item_text))
                item_text = "*Unknown item*";

            PushID(i);
            if (Selectable(item_text, item_selected))
            {
                *current_item = i;
                value_changed = true;
            }
            if (item_selected)
                SetItemDefaultFocus();
            PopID();
        }
    ListBoxFooter();
    if (value_changed)
        MarkItemEdited(g.CurrentWindow->DC.LastItemId);

    return value_changed;
}

//-------------------------------------------------------------------------
// [SECTION] Widgets: PlotLines, PlotHistogram
//-------------------------------------------------------------------------
// - PlotEx() [Internal]
// - PlotLines()
// - PlotHistogram()
//-------------------------------------------------------------------------

void ImGui::PlotEx(ImGuiPlotType plot_type, const char* label, float (*values_getter)(void* data, int idx), void* data, int values_count, int values_offset, const char* overlay_text, float scale_min, float scale_max, ImVec2 frame_size)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);

    const ImVec2 label_size = CalcTextSize(label, NULL, true);
    if (frame_size.x == 0.0f)
        frame_size.x = CalcItemWidth();
    if (frame_size.y == 0.0f)
        frame_size.y = label_size.y + (style.FramePadding.y * 2);

    const ImRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + frame_size);
    const ImRect inner_bb(frame_bb.Min + style.FramePadding, frame_bb.Max - style.FramePadding);
    const ImRect total_bb(frame_bb.Min, frame_bb.Max + ImVec2(label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f, 0));
    ItemSize(total_bb, style.FramePadding.y);
    if (!ItemAdd(total_bb, 0, &frame_bb))
        return;
    const bool hovered = ItemHoverable(frame_bb, id);

    // Determine scale from values if not specified
    if (scale_min == FLT_MAX || scale_max == FLT_MAX)
    {
        float v_min = FLT_MAX;
        float v_max = -FLT_MAX;
        for (int i = 0; i < values_count; i++)
        {
            const float v = values_getter(data, i);
            v_min = ImMin(v_min, v);
            v_max = ImMax(v_max, v);
        }
        if (scale_min == FLT_MAX)
            scale_min = v_min;
        if (scale_max == FLT_MAX)
            scale_max = v_max;
    }

    RenderFrame(frame_bb.Min, frame_bb.Max, GetColorU32(ImGuiCol_FrameBg), true, style.FrameRounding);

    if (values_count > 0)
    {
        int res_w = ImMin((int)frame_size.x, values_count) + ((plot_type == ImGuiPlotType_Lines) ? -1 : 0);
        int item_count = values_count + ((plot_type == ImGuiPlotType_Lines) ? -1 : 0);

        // Tooltip on hover
        int v_hovered = -1;
        if (hovered && inner_bb.Contains(g.IO.MousePos))
        {
            const float t = ImClamp((g.IO.MousePos.x - inner_bb.Min.x) / (inner_bb.Max.x - inner_bb.Min.x), 0.0f, 0.9999f);
            const int v_idx = (int)(t * item_count);
            IM_ASSERT(v_idx >= 0 && v_idx < values_count);

            const float v0 = values_getter(data, (v_idx + values_offset) % values_count);
            const float v1 = values_getter(data, (v_idx + 1 + values_offset) % values_count);
            if (plot_type == ImGuiPlotType_Lines)
                SetTooltip("%d: %8.4g\n%d: %8.4g", v_idx, v0, v_idx+1, v1);
            else if (plot_type == ImGuiPlotType_Histogram)
                SetTooltip("%d: %8.4g", v_idx, v0);
            v_hovered = v_idx;
        }

        const float t_step = 1.0f / (float)res_w;
        const float inv_scale = (scale_min == scale_max) ? 0.0f : (1.0f / (scale_max - scale_min));

        float v0 = values_getter(data, (0 + values_offset) % values_count);
        float t0 = 0.0f;
        ImVec2 tp0 = ImVec2( t0, 1.0f - ImSaturate((v0 - scale_min) * inv_scale) );                       // Point in the normalized space of our target rectangle
        float histogram_zero_line_t = (scale_min * scale_max < 0.0f) ? (-scale_min * inv_scale) : (scale_min < 0.0f ? 0.0f : 1.0f);   // Where does the zero line stands

        const ImU32 col_base = GetColorU32((plot_type == ImGuiPlotType_Lines) ? ImGuiCol_PlotLines : ImGuiCol_PlotHistogram);
        const ImU32 col_hovered = GetColorU32((plot_type == ImGuiPlotType_Lines) ? ImGuiCol_PlotLinesHovered : ImGuiCol_PlotHistogramHovered);

        for (int n = 0; n < res_w; n++)
        {
            const float t1 = t0 + t_step;
            const int v1_idx = (int)(t0 * item_count + 0.5f);
            IM_ASSERT(v1_idx >= 0 && v1_idx < values_count);
            const float v1 = values_getter(data, (v1_idx + values_offset + 1) % values_count);
            const ImVec2 tp1 = ImVec2( t1, 1.0f - ImSaturate((v1 - scale_min) * inv_scale) );

            // NB: Draw calls are merged together by the DrawList system. Still, we should render our batch are lower level to save a bit of CPU.
            ImVec2 pos0 = ImLerp(inner_bb.Min, inner_bb.Max, tp0);
            ImVec2 pos1 = ImLerp(inner_bb.Min, inner_bb.Max, (plot_type == ImGuiPlotType_Lines) ? tp1 : ImVec2(tp1.x, histogram_zero_line_t));
            if (plot_type == ImGuiPlotType_Lines)
            {
                window->DrawList->AddLine(pos0, pos1, v_hovered == v1_idx ? col_hovered : col_base);
            }
            else if (plot_type == ImGuiPlotType_Histogram)
            {
                if (pos1.x >= pos0.x + 2.0f)
                    pos1.x -= 1.0f;
                window->DrawList->AddRectFilled(pos0, pos1, v_hovered == v1_idx ? col_hovered : col_base);
            }

            t0 = t1;
            tp0 = tp1;
        }
    }

    // Text overlay
    if (overlay_text)
        RenderTextClipped(ImVec2(frame_bb.Min.x, frame_bb.Min.y + style.FramePadding.y), frame_bb.Max, overlay_text, NULL, NULL, ImVec2(0.5f,0.0f));

    if (label_size.x > 0.0f)
        RenderText(ImVec2(frame_bb.Max.x + style.ItemInnerSpacing.x, inner_bb.Min.y), label);
}

struct ImGuiPlotArrayGetterData
{
    const float* Values;
    int Stride;

    ImGuiPlotArrayGetterData(const float* values, int stride) { Values = values; Stride = stride; }
};

static float Plot_ArrayGetter(void* data, int idx)
{
    ImGuiPlotArrayGetterData* plot_data = (ImGuiPlotArrayGetterData*)data;
    const float v = *(const float*)(const void*)((const unsigned char*)plot_data->Values + (size_t)idx * plot_data->Stride);
    return v;
}

void ImGui::PlotLines(const char* label, const float* values, int values_count, int values_offset, const char* overlay_text, float scale_min, float scale_max, ImVec2 graph_size, int stride)
{
    ImGuiPlotArrayGetterData data(values, stride);
    PlotEx(ImGuiPlotType_Lines, label, &Plot_ArrayGetter, (void*)&data, values_count, values_offset, overlay_text, scale_min, scale_max, graph_size);
}

void ImGui::PlotLines(const char* label, float (*values_getter)(void* data, int idx), void* data, int values_count, int values_offset, const char* overlay_text, float scale_min, float scale_max, ImVec2 graph_size)
{
    PlotEx(ImGuiPlotType_Lines, label, values_getter, data, values_count, values_offset, overlay_text, scale_min, scale_max, graph_size);
}

void ImGui::PlotHistogram(const char* label, const float* values, int values_count, int values_offset, const char* overlay_text, float scale_min, float scale_max, ImVec2 graph_size, int stride)
{
    ImGuiPlotArrayGetterData data(values, stride);
    PlotEx(ImGuiPlotType_Histogram, label, &Plot_ArrayGetter, (void*)&data, values_count, values_offset, overlay_text, scale_min, scale_max, graph_size);
}

void ImGui::PlotHistogram(const char* label, float (*values_getter)(void* data, int idx), void* data, int values_count, int values_offset, const char* overlay_text, float scale_min, float scale_max, ImVec2 graph_size)
{
    PlotEx(ImGuiPlotType_Histogram, label, values_getter, data, values_count, values_offset, overlay_text, scale_min, scale_max, graph_size);
}

//-------------------------------------------------------------------------
// [SECTION] Widgets: Value helpers
// Those is not very useful, legacy API.
//-------------------------------------------------------------------------
// - Value()
//-------------------------------------------------------------------------

void ImGui::Value(const char* prefix, bool b)
{
    Text("%s: %s", prefix, (b ? "true" : "false"));
}

void ImGui::Value(const char* prefix, int v)
{
    Text("%s: %d", prefix, v);
}

void ImGui::Value(const char* prefix, unsigned int v)
{
    Text("%s: %d", prefix, v);
}

void ImGui::Value(const char* prefix, float v, const char* float_format)
{
    if (float_format)
    {
        char fmt[64];
        ImFormatString(fmt, IM_ARRAYSIZE(fmt), "%%s: %s", float_format);
        Text(fmt, prefix, v);
    }
    else
    {
        Text("%s: %.3f", prefix, v);
    }
}

//-------------------------------------------------------------------------
// [SECTION] MenuItem, BeginMenu, EndMenu, etc.
//-------------------------------------------------------------------------
// - ImGuiMenuColumns [Internal]
// - BeginMainMenuBar()
// - EndMainMenuBar()
// - BeginMenuBar()
// - EndMenuBar()
// - BeginMenu()
// - EndMenu()
// - MenuItem()
//-------------------------------------------------------------------------

// Helpers for internal use
ImGuiMenuColumns::ImGuiMenuColumns()
{
    Count = 0;
    Spacing = Width = NextWidth = 0.0f;
    memset(Pos, 0, sizeof(Pos));
    memset(NextWidths, 0, sizeof(NextWidths));
}

void ImGuiMenuColumns::Update(int count, float spacing, bool clear)
{
    IM_ASSERT(Count <= IM_ARRAYSIZE(Pos));
    Count = count;
    Width = NextWidth = 0.0f;
    Spacing = spacing;
    if (clear) memset(NextWidths, 0, sizeof(NextWidths));
    for (int i = 0; i < Count; i++)
    {
        if (i > 0 && NextWidths[i] > 0.0f)
            Width += Spacing;
        Pos[i] = (float)(int)Width;
        Width += NextWidths[i];
        NextWidths[i] = 0.0f;
    }
}

float ImGuiMenuColumns::DeclColumns(float w0, float w1, float w2) // not using va_arg because they promote float to double
{
    NextWidth = 0.0f;
    NextWidths[0] = ImMax(NextWidths[0], w0);
    NextWidths[1] = ImMax(NextWidths[1], w1);
    NextWidths[2] = ImMax(NextWidths[2], w2);
    for (int i = 0; i < 3; i++)
        NextWidth += NextWidths[i] + ((i > 0 && NextWidths[i] > 0.0f) ? Spacing : 0.0f);
    return ImMax(Width, NextWidth);
}

float ImGuiMenuColumns::CalcExtraSpace(float avail_w)
{
    return ImMax(0.0f, avail_w - Width);
}

// For the main menu bar, which cannot be moved, we honor g.Style.DisplaySafeAreaPadding to ensure text can be visible on a TV set.
bool ImGui::BeginMainMenuBar()
{
    ImGuiContext& g = *GImGui;
    g.NextWindowData.MenuBarOffsetMinVal = ImVec2(g.Style.DisplaySafeAreaPadding.x, ImMax(g.Style.DisplaySafeAreaPadding.y - g.Style.FramePadding.y, 0.0f));
    SetNextWindowPos(ImVec2(0.0f, 0.0f));
    SetNextWindowSize(ImVec2(g.IO.DisplaySize.x, g.NextWindowData.MenuBarOffsetMinVal.y + g.FontBaseSize + g.Style.FramePadding.y));
    PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(0,0));
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar;
    bool is_open = Begin("##MainMenuBar", NULL, window_flags) && BeginMenuBar();
    PopStyleVar(2);
    g.NextWindowData.MenuBarOffsetMinVal = ImVec2(0.0f, 0.0f);
    if (!is_open)
    {
        End();
        return false;
    }
    return true; //-V1020
}

void ImGui::EndMainMenuBar()
{
    EndMenuBar();

    // When the user has left the menu layer (typically: closed menus through activation of an item), we restore focus to the previous window
    ImGuiContext& g = *GImGui;
    if (g.CurrentWindow == g.NavWindow && g.NavLayer == 0)
        FocusPreviousWindowIgnoringOne(g.NavWindow);

    End();
}

bool ImGui::BeginMenuBar()
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;
    if (!(window->Flags & ImGuiWindowFlags_MenuBar))
        return false;

    IM_ASSERT(!window->DC.MenuBarAppending);
    BeginGroup(); // Backup position on layer 0
    PushID("##menubar");

    // We don't clip with current window clipping rectangle as it is already set to the area below. However we clip with window full rect.
    // We remove 1 worth of rounding to Max.x to that text in long menus and small windows don't tend to display over the lower-right rounded area, which looks particularly glitchy.
    ImRect bar_rect = window->MenuBarRect();
    ImRect clip_rect(ImFloor(bar_rect.Min.x + 0.5f), ImFloor(bar_rect.Min.y + window->WindowBorderSize + 0.5f), ImFloor(ImMax(bar_rect.Min.x, bar_rect.Max.x) + 0.5f), ImFloor(bar_rect.Max.y + 0.5f));
    clip_rect.ClipWith(window->OuterRectClipped);
    PushClipRect(clip_rect.Min, clip_rect.Max, false);

    //window->DC.CursorPos = ImVec2(bar_rect.Min.x + window->DC.MenuBarOffset.x, bar_rect.Min.y + window->DC.MenuBarOffset.y);

    window->DC.CursorPos = ImVec2(bar_rect.Min.x, bar_rect.Min.y + window->DC.MenuBarOffset.y);
    window->DC.LayoutType = ImGuiLayoutType_Horizontal;
    window->DC.NavLayerCurrent = ImGuiNavLayer_Menu;
    window->DC.NavLayerCurrentMask = (1 << ImGuiNavLayer_Menu);
    window->DC.MenuBarAppending = true;
    AlignTextToFramePadding();
    return true;
}

void ImGui::EndMenuBar()
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return;
    ImGuiContext& g = *GImGui;

    // Nav: When a move request within one of our child menu failed, capture the request to navigate among our siblings.
    if (NavMoveRequestButNoResultYet() && (g.NavMoveDir == ImGuiDir_Left || g.NavMoveDir == ImGuiDir_Right) && (g.NavWindow->Flags & ImGuiWindowFlags_ChildMenu))
    {
        ImGuiWindow* nav_earliest_child = g.NavWindow;
        while (nav_earliest_child->ParentWindow && (nav_earliest_child->ParentWindow->Flags & ImGuiWindowFlags_ChildMenu))
            nav_earliest_child = nav_earliest_child->ParentWindow;
        if (nav_earliest_child->ParentWindow == window && nav_earliest_child->DC.ParentLayoutType == ImGuiLayoutType_Horizontal && g.NavMoveRequestForward == ImGuiNavForward_None)
        {
            // To do so we claim focus back, restore NavId and then process the movement request for yet another frame.
            // This involve a one-frame delay which isn't very problematic in this situation. We could remove it by scoring in advance for multiple window (probably not worth the hassle/cost)
            IM_ASSERT(window->DC.NavLayerActiveMaskNext & 0x02); // Sanity check
            FocusWindow(window);
            SetNavIDWithRectRel(window->NavLastIds[1], 1, window->NavRectRel[1]);
            g.NavLayer = ImGuiNavLayer_Menu;
            g.NavDisableHighlight = true; // Hide highlight for the current frame so we don't see the intermediary selection.
            g.NavMoveRequestForward = ImGuiNavForward_ForwardQueued;
            NavMoveRequestCancel();
        }
    }

    IM_ASSERT(window->Flags & ImGuiWindowFlags_MenuBar);
    IM_ASSERT(window->DC.MenuBarAppending);
    PopClipRect();
    PopID();
    window->DC.MenuBarOffset.x = window->DC.CursorPos.x - window->MenuBarRect().Min.x; // Save horizontal position so next append can reuse it. This is kinda equivalent to a per-layer CursorPos.
    window->DC.GroupStack.back().AdvanceCursor = false;
    EndGroup(); // Restore position on layer 0
    window->DC.LayoutType = ImGuiLayoutType_Vertical;
    window->DC.NavLayerCurrent = ImGuiNavLayer_Main;
    window->DC.NavLayerCurrentMask = (1 << ImGuiNavLayer_Main);
    window->DC.MenuBarAppending = false;
}

bool ImGui::BeginMenu(const char* label, bool enabled)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);

    ImVec2 label_size = CalcTextSize(label, NULL, true);

    bool pressed;
    bool menu_is_open = IsPopupOpen(id);
    bool menuset_is_open = !(window->Flags & ImGuiWindowFlags_Popup) && (g.OpenPopupStack.Size > g.BeginPopupStack.Size && g.OpenPopupStack[g.BeginPopupStack.Size].OpenParentId == window->IDStack.back());
    ImGuiWindow* backed_nav_window = g.NavWindow;
    if (menuset_is_open)
        g.NavWindow = window;  // Odd hack to allow hovering across menus of a same menu-set (otherwise we wouldn't be able to hover parent)

    // The reference position stored in popup_pos will be used by Begin() to find a suitable position for the child menu,
    // However the final position is going to be different! It is choosen by FindBestWindowPosForPopup().
    // e.g. Menus tend to overlap each other horizontally to amplify relative Z-ordering.
    ImVec2 popup_pos, pos = window->DC.CursorPos;
    if (window->DC.LayoutType == ImGuiLayoutType_Horizontal)
    {
        // Menu inside an horizontal menu bar
        // Selectable extend their highlight by half ItemSpacing in each direction.
        // For ChildMenu, the popup position will be overwritten by the call to FindBestWindowPosForPopup() in Begin()
        popup_pos = ImVec2(pos.x - 1.0f - (float)(int)(style.ItemSpacing.x * 0.5f), pos.y - style.FramePadding.y + window->MenuBarHeight());
        window->DC.CursorPos.x += (float)(int)(style.ItemSpacing.x * 0.5f);
        PushStyleVar(ImGuiStyleVar_ItemSpacing, style.ItemSpacing * 2.0f);
        float w = label_size.x;
        pressed = Selectable(label, menu_is_open, ImGuiSelectableFlags_NoHoldingActiveID | ImGuiSelectableFlags_PressedOnClick | ImGuiSelectableFlags_DontClosePopups | (!enabled ? ImGuiSelectableFlags_Disabled : 0), ImVec2(w, 0.0f));
        PopStyleVar();
        window->DC.CursorPos.x += (float)(int)(style.ItemSpacing.x * (-1.0f + 0.5f)); // -1 spacing to compensate the spacing added when Selectable() did a SameLine(). It would also work to call SameLine() ourselves after the PopStyleVar().
    }
    else
    {
        // Menu inside a menu
        popup_pos = ImVec2(pos.x, pos.y - style.WindowPadding.y);
        float w = window->MenuColumns.DeclColumns(label_size.x, 0.0f, (float)(int)(g.FontSize * 1.20f)); // Feedback to next frame
        float extra_w = ImMax(0.0f, GetContentRegionAvail().x - w);
        pressed = Selectable(label, menu_is_open, ImGuiSelectableFlags_NoHoldingActiveID | ImGuiSelectableFlags_PressedOnClick | ImGuiSelectableFlags_DontClosePopups | ImGuiSelectableFlags_DrawFillAvailWidth | (!enabled ? ImGuiSelectableFlags_Disabled : 0), ImVec2(w, 0.0f));
        if (!enabled) PushStyleColor(ImGuiCol_Text, g.Style.Colors[ImGuiCol_TextDisabled]);
        RenderArrow(pos + ImVec2(window->MenuColumns.Pos[2] + extra_w + g.FontSize * 0.30f, 0.0f), ImGuiDir_Right);
        if (!enabled) PopStyleColor();
    }

    const bool hovered = enabled && ItemHoverable(window->DC.LastItemRect, id);
    if (menuset_is_open)
        g.NavWindow = backed_nav_window;

    bool want_open = false, want_close = false;
    if (window->DC.LayoutType == ImGuiLayoutType_Vertical) // (window->Flags & (ImGuiWindowFlags_Popup|ImGuiWindowFlags_ChildMenu))
    {
        // Implement http://bjk5.com/post/44698559168/breaking-down-amazons-mega-dropdown to avoid using timers, so menus feels more reactive.
        bool moving_within_opened_triangle = false;
        if (g.HoveredWindow == window && g.OpenPopupStack.Size > g.BeginPopupStack.Size && g.OpenPopupStack[g.BeginPopupStack.Size].ParentWindow == window && !(window->Flags & ImGuiWindowFlags_MenuBar))
        {
            if (ImGuiWindow* next_window = g.OpenPopupStack[g.BeginPopupStack.Size].Window)
            {
                // FIXME-DPI: Values should be derived from a master "scale" factor.
                ImRect next_window_rect = next_window->Rect();
                ImVec2 ta = g.IO.MousePos - g.IO.MouseDelta;
                ImVec2 tb = (window->Pos.x < next_window->Pos.x) ? next_window_rect.GetTL() : next_window_rect.GetTR();
                ImVec2 tc = (window->Pos.x < next_window->Pos.x) ? next_window_rect.GetBL() : next_window_rect.GetBR();
                float extra = ImClamp(ImFabs(ta.x - tb.x) * 0.30f, 5.0f, 30.0f); // add a bit of extra slack.
                ta.x += (window->Pos.x < next_window->Pos.x) ? -0.5f : +0.5f;    // to avoid numerical issues
                tb.y = ta.y + ImMax((tb.y - extra) - ta.y, -100.0f);             // triangle is maximum 200 high to limit the slope and the bias toward large sub-menus // FIXME: Multiply by fb_scale?
                tc.y = ta.y + ImMin((tc.y + extra) - ta.y, +100.0f);
                moving_within_opened_triangle = ImTriangleContainsPoint(ta, tb, tc, g.IO.MousePos);
                //window->DrawList->PushClipRectFullScreen(); window->DrawList->AddTriangleFilled(ta, tb, tc, moving_within_opened_triangle ? IM_COL32(0,128,0,128) : IM_COL32(128,0,0,128)); window->DrawList->PopClipRect(); // Debug
            }
        }

        want_close = (menu_is_open && !hovered && g.HoveredWindow == window && g.HoveredIdPreviousFrame != 0 && g.HoveredIdPreviousFrame != id && !moving_within_opened_triangle);
        want_open = (!menu_is_open && hovered && !moving_within_opened_triangle) || (!menu_is_open && hovered && pressed);

        if (g.NavActivateId == id)
        {
            want_close = menu_is_open;
            want_open = !menu_is_open;
        }
        if (g.NavId == id && g.NavMoveRequest && g.NavMoveDir == ImGuiDir_Right) // Nav-Right to open
        {
            want_open = true;
            NavMoveRequestCancel();
        }
    }
    else
    {
        // Menu bar
        if (menu_is_open && pressed && menuset_is_open) // Click an open menu again to close it
        {
            want_close = true;
            want_open = menu_is_open = false;
        }
        else if (pressed || (hovered && menuset_is_open && !menu_is_open)) // First click to open, then hover to open others
        {
            want_open = true;
        }
        else if (g.NavId == id && g.NavMoveRequest && g.NavMoveDir == ImGuiDir_Down) // Nav-Down to open
        {
            want_open = true;
            NavMoveRequestCancel();
        }
    }

    if (!enabled) // explicitly close if an open menu becomes disabled, facilitate users code a lot in pattern such as 'if (BeginMenu("options", has_object)) { ..use object.. }'
        want_close = true;
    if (want_close && IsPopupOpen(id))
        ClosePopupToLevel(g.BeginPopupStack.Size, true);

    IMGUI_TEST_ENGINE_ITEM_INFO(id, label, window->DC.ItemFlags | ImGuiItemStatusFlags_Openable | (menu_is_open ? ImGuiItemStatusFlags_Opened : 0));

    if (!menu_is_open && want_open && g.OpenPopupStack.Size > g.BeginPopupStack.Size)
    {
        // Don't recycle same menu level in the same frame, first close the other menu and yield for a frame.
        OpenPopup(label);
        return false;
    }

    menu_is_open |= want_open;
    if (want_open)
        OpenPopup(label);

    if (menu_is_open)
    {
        // Sub-menus are ChildWindow so that mouse can be hovering across them (otherwise top-most popup menu would steal focus and not allow hovering on parent menu)
        SetNextWindowPos(popup_pos, ImGuiCond_Always);
        ImGuiWindowFlags flags = ImGuiWindowFlags_ChildMenu | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoNavFocus;
        if (window->Flags & (ImGuiWindowFlags_Popup|ImGuiWindowFlags_ChildMenu))
            flags |= ImGuiWindowFlags_ChildWindow;
        menu_is_open = BeginPopupEx(id, flags); // menu_is_open can be 'false' when the popup is completely clipped (e.g. zero size display)
    }

    return menu_is_open;
}

void ImGui::EndMenu()
{
    // Nav: When a left move request _within our child menu_ failed, close ourselves (the _parent_ menu).
    // A menu doesn't close itself because EndMenuBar() wants the catch the last Left<>Right inputs.
    // However, it means that with the current code, a BeginMenu() from outside another menu or a menu-bar won't be closable with the Left direction.
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    if (g.NavWindow && g.NavWindow->ParentWindow == window && g.NavMoveDir == ImGuiDir_Left && NavMoveRequestButNoResultYet() && window->DC.LayoutType == ImGuiLayoutType_Vertical)
    {
        ClosePopupToLevel(g.BeginPopupStack.Size, true);
        NavMoveRequestCancel();
    }

    EndPopup();
}

bool ImGui::MenuItem(const char* label, const char* shortcut, bool selected, bool enabled)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    ImGuiStyle& style = g.Style;
    ImVec2 pos = window->DC.CursorPos;
    ImVec2 label_size = CalcTextSize(label, NULL, true);

    ImGuiSelectableFlags flags = ImGuiSelectableFlags_PressedOnRelease | (enabled ? 0 : ImGuiSelectableFlags_Disabled);
    bool pressed;
    if (window->DC.LayoutType == ImGuiLayoutType_Horizontal)
    {
        // Mimic the exact layout spacing of BeginMenu() to allow MenuItem() inside a menu bar, which is a little misleading but may be useful
        // Note that in this situation we render neither the shortcut neither the selected tick mark
        float w = label_size.x;
        window->DC.CursorPos.x += (float)(int)(style.ItemSpacing.x * 0.5f);
        PushStyleVar(ImGuiStyleVar_ItemSpacing, style.ItemSpacing * 2.0f);
        pressed = Selectable(label, false, flags, ImVec2(w, 0.0f));
        PopStyleVar();
        window->DC.CursorPos.x += (float)(int)(style.ItemSpacing.x * (-1.0f + 0.5f)); // -1 spacing to compensate the spacing added when Selectable() did a SameLine(). It would also work to call SameLine() ourselves after the PopStyleVar().
    }
    else
    {
        ImVec2 shortcut_size = shortcut ? CalcTextSize(shortcut, NULL) : ImVec2(0.0f, 0.0f);
        float w = window->MenuColumns.DeclColumns(label_size.x, shortcut_size.x, (float)(int)(g.FontSize * 1.20f)); // Feedback for next frame
        float extra_w = ImMax(0.0f, GetContentRegionAvail().x - w);
        pressed = Selectable(label, false, flags | ImGuiSelectableFlags_DrawFillAvailWidth, ImVec2(w, 0.0f));
        if (shortcut_size.x > 0.0f)
        {
            PushStyleColor(ImGuiCol_Text, g.Style.Colors[ImGuiCol_TextDisabled]);
            RenderText(pos + ImVec2(window->MenuColumns.Pos[1] + extra_w, 0.0f), shortcut, NULL, false);
            PopStyleColor();
        }
        if (selected)
            RenderCheckMark(pos + ImVec2(window->MenuColumns.Pos[2] + extra_w + g.FontSize * 0.40f, g.FontSize * 0.134f * 0.5f), GetColorU32(enabled ? ImGuiCol_Text : ImGuiCol_TextDisabled), g.FontSize  * 0.866f);
    }

    IMGUI_TEST_ENGINE_ITEM_INFO(window->DC.LastItemId, label, window->DC.ItemFlags | ImGuiItemStatusFlags_Checkable | (selected ? ImGuiItemStatusFlags_Checked : 0));
    return pressed;
}

bool ImGui::MenuItem(const char* label, const char* shortcut, bool* p_selected, bool enabled)
{
    if (MenuItem(label, shortcut, p_selected ? *p_selected : false, enabled))
    {
        if (p_selected)
            *p_selected = !*p_selected;
        return true;
    }
    return false;
}

//-------------------------------------------------------------------------
// [SECTION] Widgets: BeginTabBar, EndTabBar, etc.
//-------------------------------------------------------------------------
// [BETA API] API may evolve! This code has been extracted out of the Docking branch,
// and some of the construct which are not used in Master may be left here to facilitate merging.
//-------------------------------------------------------------------------
// - BeginTabBar()
// - BeginTabBarEx() [Internal]
// - EndTabBar()
// - TabBarLayout() [Internal]
// - TabBarCalcTabID() [Internal]
// - TabBarCalcMaxTabWidth() [Internal]
// - TabBarFindTabById() [Internal]
// - TabBarRemoveTab() [Internal]
// - TabBarCloseTab() [Internal]
// - TabBarScrollClamp()v
// - TabBarScrollToTab() [Internal]
// - TabBarQueueChangeTabOrder() [Internal]
// - TabBarScrollingButtons() [Internal]
// - TabBarTabListPopupButton() [Internal]
//-------------------------------------------------------------------------

namespace ImGui
{
    static void             TabBarLayout(ImGuiTabBar* tab_bar);
    static ImU32            TabBarCalcTabID(ImGuiTabBar* tab_bar, const char* label);
    static float            TabBarCalcMaxTabWidth();
    static float            TabBarScrollClamp(ImGuiTabBar* tab_bar, float scrolling);
    static void             TabBarScrollToTab(ImGuiTabBar* tab_bar, ImGuiTabItem* tab);
    static ImGuiTabItem*    TabBarScrollingButtons(ImGuiTabBar* tab_bar);
    static ImGuiTabItem*    TabBarTabListPopupButton(ImGuiTabBar* tab_bar);
}

ImGuiTabBar::ImGuiTabBar()
{
    ID = 0;
    SelectedTabId = NextSelectedTabId = VisibleTabId = 0;
    CurrFrameVisible = PrevFrameVisible = -1;
    ContentsHeight = 0.0f;
    OffsetMax = OffsetNextTab = 0.0f;
    ScrollingAnim = ScrollingTarget = 0.0f;
    Flags = ImGuiTabBarFlags_None;
    ReorderRequestTabId = 0;
    ReorderRequestDir = 0;
    WantLayout = VisibleTabWasSubmitted = false;
    LastTabItemIdx = -1;
}

static int IMGUI_CDECL TabItemComparerByVisibleOffset(const void* lhs, const void* rhs)
{
    const ImGuiTabItem* a = (const ImGuiTabItem*)lhs;
    const ImGuiTabItem* b = (const ImGuiTabItem*)rhs;
    return (int)(a->Offset - b->Offset);
}

static int IMGUI_CDECL TabBarSortItemComparer(const void* lhs, const void* rhs)
{
    const ImGuiTabBarSortItem* a = (const ImGuiTabBarSortItem*)lhs;
    const ImGuiTabBarSortItem* b = (const ImGuiTabBarSortItem*)rhs;
    if (int d = (int)(b->Width - a->Width))
        return d;
    return (b->Index - a->Index);
}

static ImGuiTabBar* GetTabBarFromTabBarRef(const ImGuiTabBarRef& ref)
{
    ImGuiContext& g = *GImGui;
    return ref.Ptr ? ref.Ptr : g.TabBars.GetByIndex(ref.IndexInMainPool);
}

static ImGuiTabBarRef GetTabBarRefFromTabBar(ImGuiTabBar* tab_bar)
{
    ImGuiContext& g = *GImGui;
    if (g.TabBars.Contains(tab_bar))
        return ImGuiTabBarRef(g.TabBars.GetIndex(tab_bar));
    return ImGuiTabBarRef(tab_bar);
}

bool    ImGui::BeginTabBar(const char* str_id, ImGuiTabBarFlags flags)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    if (window->SkipItems)
        return false;

    ImGuiID id = window->GetID(str_id);
    ImGuiTabBar* tab_bar = g.TabBars.GetOrAddByKey(id);
    ImRect tab_bar_bb = ImRect(window->DC.CursorPos.x, window->DC.CursorPos.y, window->InnerClipRect.Max.x, window->DC.CursorPos.y + g.FontSize + g.Style.FramePadding.y * 2);
    tab_bar->ID = id;
    return BeginTabBarEx(tab_bar, tab_bar_bb, flags | ImGuiTabBarFlags_IsFocused);
}

bool    ImGui::BeginTabBarEx(ImGuiTabBar* tab_bar, const ImRect& tab_bar_bb, ImGuiTabBarFlags flags)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    if (window->SkipItems)
        return false;

    if ((flags & ImGuiTabBarFlags_DockNode) == 0)
        window->IDStack.push_back(tab_bar->ID);

    // Add to stack
    g.CurrentTabBarStack.push_back(GetTabBarRefFromTabBar(tab_bar));
    g.CurrentTabBar = tab_bar;

    if (tab_bar->CurrFrameVisible == g.FrameCount)
    {
        //IMGUI_DEBUG_LOG("BeginTabBarEx already called this frame\n", g.FrameCount);
        IM_ASSERT(0);
        return true;
    }

    // When toggling back from ordered to manually-reorderable, shuffle tabs to enforce the last visible order.
    // Otherwise, the most recently inserted tabs would move at the end of visible list which can be a little too confusing or magic for the user.
    if ((flags & ImGuiTabBarFlags_Reorderable) && !(tab_bar->Flags & ImGuiTabBarFlags_Reorderable) && tab_bar->Tabs.Size > 1 && tab_bar->PrevFrameVisible != -1)
        ImQsort(tab_bar->Tabs.Data, tab_bar->Tabs.Size, sizeof(ImGuiTabItem), TabItemComparerByVisibleOffset);

    // Flags
    if ((flags & ImGuiTabBarFlags_FittingPolicyMask_) == 0)
        flags |= ImGuiTabBarFlags_FittingPolicyDefault_;

    tab_bar->Flags = flags;
    tab_bar->BarRect = tab_bar_bb;
    tab_bar->WantLayout = true; // Layout will be done on the first call to ItemTab()
    tab_bar->PrevFrameVisible = tab_bar->CurrFrameVisible;
    tab_bar->CurrFrameVisible = g.FrameCount;
    tab_bar->FramePadding = g.Style.FramePadding;

    // Layout
    ItemSize(ImVec2(tab_bar->OffsetMax, tab_bar->BarRect.GetHeight()));
    window->DC.CursorPos.x = tab_bar->BarRect.Min.x;

    // Draw separator
    const ImU32 col = GetColorU32((flags & ImGuiTabBarFlags_IsFocused) ? ImGuiCol_TabActive : ImGuiCol_Tab);
    const float y = tab_bar->BarRect.Max.y - 1.0f;
    {
        const float separator_min_x = tab_bar->BarRect.Min.x - window->WindowPadding.x;
        const float separator_max_x = tab_bar->BarRect.Max.x + window->WindowPadding.x;
        window->DrawList->AddLine(ImVec2(separator_min_x, y), ImVec2(separator_max_x, y), col, 1.0f);
    }
    return true;
}

void    ImGui::EndTabBar()
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    if (window->SkipItems)
        return;

    ImGuiTabBar* tab_bar = g.CurrentTabBar;
    if (tab_bar == NULL)
    {
        IM_ASSERT(tab_bar != NULL && "Mismatched BeginTabBar()/EndTabBar()!");
        return; // FIXME-ERRORHANDLING
    }
    if (tab_bar->WantLayout)
        TabBarLayout(tab_bar);

    // Restore the last visible height if no tab is visible, this reduce vertical flicker/movement when a tabs gets removed without calling SetTabItemClosed().
    const bool tab_bar_appearing = (tab_bar->PrevFrameVisible + 1 < g.FrameCount);
    if (tab_bar->VisibleTabWasSubmitted || tab_bar->VisibleTabId == 0 || tab_bar_appearing)
        tab_bar->ContentsHeight = ImMax(window->DC.CursorPos.y - tab_bar->BarRect.Max.y, 0.0f);
    else
        window->DC.CursorPos.y = tab_bar->BarRect.Max.y + tab_bar->ContentsHeight;

    if ((tab_bar->Flags & ImGuiTabBarFlags_DockNode) == 0)
        PopID();

    g.CurrentTabBarStack.pop_back();
    g.CurrentTabBar = g.CurrentTabBarStack.empty() ? NULL : GetTabBarFromTabBarRef(g.CurrentTabBarStack.back());
}

// This is called only once a frame before by the first call to ItemTab()
// The reason we're not calling it in BeginTabBar() is to leave a chance to the user to call the SetTabItemClosed() functions.
static void ImGui::TabBarLayout(ImGuiTabBar* tab_bar)
{
    ImGuiContext& g = *GImGui;
    tab_bar->WantLayout = false;

    // Garbage collect
    int tab_dst_n = 0;
    for (int tab_src_n = 0; tab_src_n < tab_bar->Tabs.Size; tab_src_n++)
    {
        ImGuiTabItem* tab = &tab_bar->Tabs[tab_src_n];
        if (tab->LastFrameVisible < tab_bar->PrevFrameVisible)
        {
            if (tab->ID == tab_bar->SelectedTabId)
                tab_bar->SelectedTabId = 0;
            continue;
        }
        if (tab_dst_n != tab_src_n)
            tab_bar->Tabs[tab_dst_n] = tab_bar->Tabs[tab_src_n];
        tab_dst_n++;
    }
    if (tab_bar->Tabs.Size != tab_dst_n)
        tab_bar->Tabs.resize(tab_dst_n);

    // Setup next selected tab
    ImGuiID scroll_track_selected_tab_id = 0;
    if (tab_bar->NextSelectedTabId)
    {
        tab_bar->SelectedTabId = tab_bar->NextSelectedTabId;
        tab_bar->NextSelectedTabId = 0;
        scroll_track_selected_tab_id = tab_bar->SelectedTabId;
    }

    // Process order change request (we could probably process it when requested but it's just saner to do it in a single spot).
    if (tab_bar->ReorderRequestTabId != 0)
    {
        if (ImGuiTabItem* tab1 = TabBarFindTabByID(tab_bar, tab_bar->ReorderRequestTabId))
        {
            //IM_ASSERT(tab_bar->Flags & ImGuiTabBarFlags_Reorderable); // <- this may happen when using debug tools
            int tab2_order = tab_bar->GetTabOrder(tab1) + tab_bar->ReorderRequestDir;
            if (tab2_order >= 0 && tab2_order < tab_bar->Tabs.Size)
            {
                ImGuiTabItem* tab2 = &tab_bar->Tabs[tab2_order];
                ImGuiTabItem item_tmp = *tab1;
                *tab1 = *tab2;
                *tab2 = item_tmp;
                if (tab2->ID == tab_bar->SelectedTabId)
                    scroll_track_selected_tab_id = tab2->ID;
                tab1 = tab2 = NULL;
            }
            if (tab_bar->Flags & ImGuiTabBarFlags_SaveSettings)
                MarkIniSettingsDirty();
        }
        tab_bar->ReorderRequestTabId = 0;
    }

    // Tab List Popup (will alter tab_bar->BarRect and therefore the available width!)
    const bool tab_list_popup_button = (tab_bar->Flags & ImGuiTabBarFlags_TabListPopupButton) != 0;
    if (tab_list_popup_button)
        if (ImGuiTabItem* tab_to_select = TabBarTabListPopupButton(tab_bar)) // NB: Will alter BarRect.Max.x!
            scroll_track_selected_tab_id = tab_bar->SelectedTabId = tab_to_select->ID;

    ImVector<ImGuiTabBarSortItem>& width_sort_buffer = g.TabSortByWidthBuffer;
    width_sort_buffer.resize(tab_bar->Tabs.Size);

    // Compute ideal widths
    float width_total_contents = 0.0f;
    ImGuiTabItem* most_recently_selected_tab = NULL;
    bool found_selected_tab_id = false;
    for (int tab_n = 0; tab_n < tab_bar->Tabs.Size; tab_n++)
    {
        ImGuiTabItem* tab = &tab_bar->Tabs[tab_n];
        IM_ASSERT(tab->LastFrameVisible >= tab_bar->PrevFrameVisible);

        if (most_recently_selected_tab == NULL || most_recently_selected_tab->LastFrameSelected < tab->LastFrameSelected)
            most_recently_selected_tab = tab;
        if (tab->ID == tab_bar->SelectedTabId)
            found_selected_tab_id = true;

        // Refresh tab width immediately, otherwise changes of style e.g. style.FramePadding.x would noticeably lag in the tab bar.
        // Additionally, when using TabBarAddTab() to manipulate tab bar order we occasionally insert new tabs that don't have a width yet,
        // and we cannot wait for the next BeginTabItem() call. We cannot compute this width within TabBarAddTab() because font size depends on the active window.
        const char* tab_name = tab_bar->GetTabName(tab);
        tab->WidthContents = TabItemCalcSize(tab_name, (tab->Flags & ImGuiTabItemFlags_NoCloseButton) ? false : true).x;

        width_total_contents += (tab_n > 0 ? g.Style.ItemInnerSpacing.x : 0.0f) + tab->WidthContents;

        // Store data so we can build an array sorted by width if we need to shrink tabs down
        width_sort_buffer[tab_n].Index = tab_n;
        width_sort_buffer[tab_n].Width = tab->WidthContents;
    }

    // Compute width
    const float width_avail = tab_bar->BarRect.GetWidth();
    float width_excess = (width_avail < width_total_contents) ? (width_total_contents - width_avail) : 0.0f;
    if (width_excess > 0.0f && (tab_bar->Flags & ImGuiTabBarFlags_FittingPolicyResizeDown))
    {
        // If we don't have enough room, resize down the largest tabs first
        if (tab_bar->Tabs.Size > 1)
            ImQsort(width_sort_buffer.Data, (size_t)width_sort_buffer.Size, sizeof(ImGuiTabBarSortItem), TabBarSortItemComparer);
        int tab_count_same_width = 1;
        while (width_excess > 0.0f && tab_count_same_width < tab_bar->Tabs.Size)
        {
            while (tab_count_same_width < tab_bar->Tabs.Size && width_sort_buffer[0].Width == width_sort_buffer[tab_count_same_width].Width)
                tab_count_same_width++;
            float width_to_remove_per_tab_max = (tab_count_same_width < tab_bar->Tabs.Size) ? (width_sort_buffer[0].Width - width_sort_buffer[tab_count_same_width].Width) : (width_sort_buffer[0].Width - 1.0f);
            float width_to_remove_per_tab = ImMin(width_excess / tab_count_same_width, width_to_remove_per_tab_max);
            for (int tab_n = 0; tab_n < tab_count_same_width; tab_n++)
                width_sort_buffer[tab_n].Width -= width_to_remove_per_tab;
            width_excess -= width_to_remove_per_tab * tab_count_same_width;
        }
        for (int tab_n = 0; tab_n < tab_bar->Tabs.Size; tab_n++)
            tab_bar->Tabs[width_sort_buffer[tab_n].Index].Width = (float)(int)width_sort_buffer[tab_n].Width;
    }
    else
    {
        const float tab_max_width = TabBarCalcMaxTabWidth();
        for (int tab_n = 0; tab_n < tab_bar->Tabs.Size; tab_n++)
        {
            ImGuiTabItem* tab = &tab_bar->Tabs[tab_n];
            tab->Width = ImMin(tab->WidthContents, tab_max_width);
        }
    }

    // Layout all active tabs
    float offset_x = 0.0f;
    for (int tab_n = 0; tab_n < tab_bar->Tabs.Size; tab_n++)
    {
        ImGuiTabItem* tab = &tab_bar->Tabs[tab_n];
        tab->Offset = offset_x;
        if (scroll_track_selected_tab_id == 0 && g.NavJustMovedToId == tab->ID)
            scroll_track_selected_tab_id = tab->ID;
        offset_x += tab->Width + g.Style.ItemInnerSpacing.x;
    }
    tab_bar->OffsetMax = ImMax(offset_x - g.Style.ItemInnerSpacing.x, 0.0f);
    tab_bar->OffsetNextTab = 0.0f;

    // Horizontal scrolling buttons
    const bool scrolling_buttons = (tab_bar->OffsetMax > tab_bar->BarRect.GetWidth() && tab_bar->Tabs.Size > 1) && !(tab_bar->Flags & ImGuiTabBarFlags_NoTabListScrollingButtons) && (tab_bar->Flags & ImGuiTabBarFlags_FittingPolicyScroll);
    if (scrolling_buttons)
        if (ImGuiTabItem* tab_to_select = TabBarScrollingButtons(tab_bar)) // NB: Will alter BarRect.Max.x!
            scroll_track_selected_tab_id = tab_bar->SelectedTabId = tab_to_select->ID;

    // If we have lost the selected tab, select the next most recently active one
    if (found_selected_tab_id == false)
        tab_bar->SelectedTabId = 0;
    if (tab_bar->SelectedTabId == 0 && tab_bar->NextSelectedTabId == 0 && most_recently_selected_tab != NULL)
        scroll_track_selected_tab_id = tab_bar->SelectedTabId = most_recently_selected_tab->ID;

    // Lock in visible tab
    tab_bar->VisibleTabId = tab_bar->SelectedTabId;
    tab_bar->VisibleTabWasSubmitted = false;

    // Update scrolling
    if (scroll_track_selected_tab_id)
        if (ImGuiTabItem* scroll_track_selected_tab = TabBarFindTabByID(tab_bar, scroll_track_selected_tab_id))
            TabBarScrollToTab(tab_bar, scroll_track_selected_tab);
    tab_bar->ScrollingAnim = TabBarScrollClamp(tab_bar, tab_bar->ScrollingAnim);
    tab_bar->ScrollingTarget = TabBarScrollClamp(tab_bar, tab_bar->ScrollingTarget);
    const float scrolling_speed = (tab_bar->PrevFrameVisible + 1 < g.FrameCount) ? FLT_MAX : (g.IO.DeltaTime * g.FontSize * 70.0f);
    if (tab_bar->ScrollingAnim != tab_bar->ScrollingTarget)
        tab_bar->ScrollingAnim = ImLinearSweep(tab_bar->ScrollingAnim, tab_bar->ScrollingTarget, scrolling_speed);

    // Clear name buffers
    if ((tab_bar->Flags & ImGuiTabBarFlags_DockNode) == 0)
        tab_bar->TabsNames.Buf.resize(0);
}

// Dockables uses Name/ID in the global namespace. Non-dockable items use the ID stack.
static ImU32   ImGui::TabBarCalcTabID(ImGuiTabBar* tab_bar, const char* label)
{
    if (tab_bar->Flags & ImGuiTabBarFlags_DockNode)
    {
        ImGuiID id = ImHashStr(label, 0);
        KeepAliveID(id);
        return id;
    }
    else
    {
        ImGuiWindow* window = GImGui->CurrentWindow;
        return window->GetID(label);
    }
}

static float ImGui::TabBarCalcMaxTabWidth()
{
    ImGuiContext& g = *GImGui;
    return g.FontSize * 20.0f;
}

ImGuiTabItem* ImGui::TabBarFindTabByID(ImGuiTabBar* tab_bar, ImGuiID tab_id)
{
    if (tab_id != 0)
        for (int n = 0; n < tab_bar->Tabs.Size; n++)
            if (tab_bar->Tabs[n].ID == tab_id)
                return &tab_bar->Tabs[n];
    return NULL;
}

// The *TabId fields be already set by the docking system _before_ the actual TabItem was created, so we clear them regardless.
void ImGui::TabBarRemoveTab(ImGuiTabBar* tab_bar, ImGuiID tab_id)
{
    if (ImGuiTabItem* tab = TabBarFindTabByID(tab_bar, tab_id))
        tab_bar->Tabs.erase(tab);
    if (tab_bar->VisibleTabId == tab_id)      { tab_bar->VisibleTabId = 0; }
    if (tab_bar->SelectedTabId == tab_id)     { tab_bar->SelectedTabId = 0; }
    if (tab_bar->NextSelectedTabId == tab_id) { tab_bar->NextSelectedTabId = 0; }
}

// Called on manual closure attempt
void ImGui::TabBarCloseTab(ImGuiTabBar* tab_bar, ImGuiTabItem* tab)
{
    if ((tab_bar->VisibleTabId == tab->ID) && !(tab->Flags & ImGuiTabItemFlags_UnsavedDocument))
    {
        // This will remove a frame of lag for selecting another tab on closure.
        // However we don't run it in the case where the 'Unsaved' flag is set, so user gets a chance to fully undo the closure
        tab->LastFrameVisible = -1;
        tab_bar->SelectedTabId = tab_bar->NextSelectedTabId = 0;
    }
    else if ((tab_bar->VisibleTabId != tab->ID) && (tab->Flags & ImGuiTabItemFlags_UnsavedDocument))
    {
        // Actually select before expecting closure
        tab_bar->NextSelectedTabId = tab->ID;
    }
}

static float ImGui::TabBarScrollClamp(ImGuiTabBar* tab_bar, float scrolling)
{
    scrolling = ImMin(scrolling, tab_bar->OffsetMax - tab_bar->BarRect.GetWidth());
    return ImMax(scrolling, 0.0f);
}

static void ImGui::TabBarScrollToTab(ImGuiTabBar* tab_bar, ImGuiTabItem* tab)
{
    ImGuiContext& g = *GImGui;
    float margin = g.FontSize * 1.0f; // When to scroll to make Tab N+1 visible always make a bit of N visible to suggest more scrolling area (since we don't have a scrollbar)
    int order = tab_bar->GetTabOrder(tab);
    float tab_x1 = tab->Offset + (order > 0 ? -margin : 0.0f);
    float tab_x2 = tab->Offset + tab->Width + (order + 1 < tab_bar->Tabs.Size ? margin : 1.0f);
    if (tab_bar->ScrollingTarget > tab_x1)
        tab_bar->ScrollingTarget = tab_x1;
    if (tab_bar->ScrollingTarget + tab_bar->BarRect.GetWidth() < tab_x2)
        tab_bar->ScrollingTarget = tab_x2 - tab_bar->BarRect.GetWidth();
}

void ImGui::TabBarQueueChangeTabOrder(ImGuiTabBar* tab_bar, const ImGuiTabItem* tab, int dir)
{
    IM_ASSERT(dir == -1 || dir == +1);
    IM_ASSERT(tab_bar->ReorderRequestTabId == 0);
    tab_bar->ReorderRequestTabId = tab->ID;
    tab_bar->ReorderRequestDir = dir;
}

static ImGuiTabItem* ImGui::TabBarScrollingButtons(ImGuiTabBar* tab_bar)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;

    const ImVec2 arrow_button_size(g.FontSize - 2.0f, g.FontSize + g.Style.FramePadding.y * 2.0f);
    const float scrolling_buttons_width = arrow_button_size.x * 2.0f;

    const ImVec2 backup_cursor_pos = window->DC.CursorPos;
    //window->DrawList->AddRect(ImVec2(tab_bar->BarRect.Max.x - scrolling_buttons_width, tab_bar->BarRect.Min.y), ImVec2(tab_bar->BarRect.Max.x, tab_bar->BarRect.Max.y), IM_COL32(255,0,0,255));

    const ImRect avail_bar_rect = tab_bar->BarRect;
    bool want_clip_rect = !avail_bar_rect.Contains(ImRect(window->DC.CursorPos, window->DC.CursorPos + ImVec2(scrolling_buttons_width, 0.0f)));
    if (want_clip_rect)
        PushClipRect(tab_bar->BarRect.Min, tab_bar->BarRect.Max + ImVec2(g.Style.ItemInnerSpacing.x, 0.0f), true);

    ImGuiTabItem* tab_to_select = NULL;

    int select_dir = 0;
    ImVec4 arrow_col = g.Style.Colors[ImGuiCol_Text];
    arrow_col.w *= 0.5f;

    PushStyleColor(ImGuiCol_Text, arrow_col);
    PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
    const float backup_repeat_delay = g.IO.KeyRepeatDelay;
    const float backup_repeat_rate = g.IO.KeyRepeatRate;
    g.IO.KeyRepeatDelay = 0.250f;
    g.IO.KeyRepeatRate = 0.200f;
    window->DC.CursorPos = ImVec2(tab_bar->BarRect.Max.x - scrolling_buttons_width, tab_bar->BarRect.Min.y);
    if (ArrowButtonEx("##<", ImGuiDir_Left, arrow_button_size, ImGuiButtonFlags_PressedOnClick | ImGuiButtonFlags_Repeat))
        select_dir = -1;
    window->DC.CursorPos = ImVec2(tab_bar->BarRect.Max.x - scrolling_buttons_width + arrow_button_size.x, tab_bar->BarRect.Min.y);
    if (ArrowButtonEx("##>", ImGuiDir_Right, arrow_button_size, ImGuiButtonFlags_PressedOnClick | ImGuiButtonFlags_Repeat))
        select_dir = +1;
    PopStyleColor(2);
    g.IO.KeyRepeatRate = backup_repeat_rate;
    g.IO.KeyRepeatDelay = backup_repeat_delay;

    if (want_clip_rect)
        PopClipRect();

    if (select_dir != 0)
        if (ImGuiTabItem* tab_item = TabBarFindTabByID(tab_bar, tab_bar->SelectedTabId))
        {
            int selected_order = tab_bar->GetTabOrder(tab_item);
            int target_order = selected_order + select_dir;
            tab_to_select = &tab_bar->Tabs[(target_order >= 0 && target_order < tab_bar->Tabs.Size) ? target_order : selected_order]; // If we are at the end of the list, still scroll to make our tab visible
        }
    window->DC.CursorPos = backup_cursor_pos;
    tab_bar->BarRect.Max.x -= scrolling_buttons_width + 1.0f;

    return tab_to_select;
}

static ImGuiTabItem* ImGui::TabBarTabListPopupButton(ImGuiTabBar* tab_bar)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;

    // We use g.Style.FramePadding.y to match the square ArrowButton size
    const float tab_list_popup_button_width = g.FontSize + g.Style.FramePadding.y;
    const ImVec2 backup_cursor_pos = window->DC.CursorPos;
    window->DC.CursorPos = ImVec2(tab_bar->BarRect.Min.x - g.Style.FramePadding.y, tab_bar->BarRect.Min.y);
    tab_bar->BarRect.Min.x += tab_list_popup_button_width;

    ImVec4 arrow_col = g.Style.Colors[ImGuiCol_Text];
    arrow_col.w *= 0.5f;
    PushStyleColor(ImGuiCol_Text, arrow_col);
    PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
    bool open = BeginCombo("##v", NULL, ImGuiComboFlags_NoPreview);
    PopStyleColor(2);

    ImGuiTabItem* tab_to_select = NULL;
    if (open)
    {
        for (int tab_n = 0; tab_n < tab_bar->Tabs.Size; tab_n++)
        {
            ImGuiTabItem* tab = &tab_bar->Tabs[tab_n];
            const char* tab_name = tab_bar->GetTabName(tab);
            if (Selectable(tab_name, tab_bar->SelectedTabId == tab->ID))
                tab_to_select = tab;
        }
        EndCombo();
    }

    window->DC.CursorPos = backup_cursor_pos;
    return tab_to_select;
}

//-------------------------------------------------------------------------
// [SECTION] Widgets: BeginTabItem, EndTabItem, etc.
//-------------------------------------------------------------------------
// [BETA API] API may evolve! This code has been extracted out of the Docking branch,
// and some of the construct which are not used in Master may be left here to facilitate merging.
//-------------------------------------------------------------------------
// - BeginTabItem()
// - EndTabItem()
// - TabItemEx() [Internal]
// - SetTabItemClosed()
// - TabItemCalcSize() [Internal]
// - TabItemBackground() [Internal]
// - TabItemLabelAndCloseButton() [Internal]
//-------------------------------------------------------------------------

bool    ImGui::BeginTabItem(const char* label, bool* p_open, ImGuiTabItemFlags flags)
{
    ImGuiContext& g = *GImGui;
    if (g.CurrentWindow->SkipItems)
        return false;

    ImGuiTabBar* tab_bar = g.CurrentTabBar;
    if (tab_bar == NULL)
    {
        IM_ASSERT(tab_bar && "Needs to be called between BeginTabBar() and EndTabBar()!");
        return false; // FIXME-ERRORHANDLING
    }
    bool ret = TabItemEx(tab_bar, label, p_open, flags);
    if (ret && !(flags & ImGuiTabItemFlags_NoPushId))
    {
        ImGuiTabItem* tab = &tab_bar->Tabs[tab_bar->LastTabItemIdx];
        g.CurrentWindow->IDStack.push_back(tab->ID);    // We already hashed 'label' so push into the ID stack directly instead of doing another hash through PushID(label)
    }
    return ret;
}

void    ImGui::EndTabItem()
{
    ImGuiContext& g = *GImGui;
    if (g.CurrentWindow->SkipItems)
        return;

    ImGuiTabBar* tab_bar = g.CurrentTabBar;
    if (tab_bar == NULL)
    {
        IM_ASSERT(tab_bar != NULL && "Needs to be called between BeginTabBar() and EndTabBar()!");
        return;
    }
    IM_ASSERT(tab_bar->LastTabItemIdx >= 0);
    ImGuiTabItem* tab = &tab_bar->Tabs[tab_bar->LastTabItemIdx];
    if (!(tab->Flags & ImGuiTabItemFlags_NoPushId))
        g.CurrentWindow->IDStack.pop_back();
}

bool    ImGui::TabItemEx(ImGuiTabBar* tab_bar, const char* label, bool* p_open, ImGuiTabItemFlags flags)
{
    // Layout whole tab bar if not already done
    if (tab_bar->WantLayout)
        TabBarLayout(tab_bar);

    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    if (window->SkipItems)
        return false;

    const ImGuiStyle& style = g.Style;
    const ImGuiID id = TabBarCalcTabID(tab_bar, label);

    // If the user called us with *p_open == false, we early out and don't render. We make a dummy call to ItemAdd() so that attempts to use a contextual popup menu with an implicit ID won't use an older ID.
    if (p_open && !*p_open)
    {
        PushItemFlag(ImGuiItemFlags_NoNav | ImGuiItemFlags_NoNavDefaultFocus, true);
        ItemAdd(ImRect(), id);
        PopItemFlag();
        return false;
    }

    // Calculate tab contents size
    ImVec2 size = TabItemCalcSize(label, p_open != NULL);

    // Acquire tab data
    ImGuiTabItem* tab = TabBarFindTabByID(tab_bar, id);
    bool tab_is_new = false;
    if (tab == NULL)
    {
        tab_bar->Tabs.push_back(ImGuiTabItem());
        tab = &tab_bar->Tabs.back();
        tab->ID = id;
        tab->Width = size.x;
        tab_is_new = true;
    }
    tab_bar->LastTabItemIdx = (short)tab_bar->Tabs.index_from_ptr(tab);
    tab->WidthContents = size.x;

    if (p_open == NULL)
        flags |= ImGuiTabItemFlags_NoCloseButton;

    const bool tab_bar_appearing = (tab_bar->PrevFrameVisible + 1 < g.FrameCount);
    const bool tab_bar_focused = (tab_bar->Flags & ImGuiTabBarFlags_IsFocused) != 0;
    const bool tab_appearing = (tab->LastFrameVisible + 1 < g.FrameCount);
    tab->LastFrameVisible = g.FrameCount;
    tab->Flags = flags;

    // Append name with zero-terminator
    tab->NameOffset = tab_bar->TabsNames.size();
    tab_bar->TabsNames.append(label, label + strlen(label) + 1);

    // If we are not reorderable, always reset offset based on submission order.
    // (We already handled layout and sizing using the previous known order, but sizing is not affected by order!)
    if (!tab_appearing && !(tab_bar->Flags & ImGuiTabBarFlags_Reorderable))
    {
        tab->Offset = tab_bar->OffsetNextTab;
        tab_bar->OffsetNextTab += tab->Width + g.Style.ItemInnerSpacing.x;
    }

    // Update selected tab
    if (tab_appearing && (tab_bar->Flags & ImGuiTabBarFlags_AutoSelectNewTabs) && tab_bar->NextSelectedTabId == 0)
        if (!tab_bar_appearing || tab_bar->SelectedTabId == 0)
            tab_bar->NextSelectedTabId = id;  // New tabs gets activated

    // Lock visibility
    bool tab_contents_visible = (tab_bar->VisibleTabId == id);
    if (tab_contents_visible)
        tab_bar->VisibleTabWasSubmitted = true;

    // On the very first frame of a tab bar we let first tab contents be visible to minimize appearing glitches
    if (!tab_contents_visible && tab_bar->SelectedTabId == 0 && tab_bar_appearing)
        if (tab_bar->Tabs.Size == 1 && !(tab_bar->Flags & ImGuiTabBarFlags_AutoSelectNewTabs))
            tab_contents_visible = true;

    if (tab_appearing && !(tab_bar_appearing && !tab_is_new))
    {
        PushItemFlag(ImGuiItemFlags_NoNav | ImGuiItemFlags_NoNavDefaultFocus, true);
        ItemAdd(ImRect(), id);
        PopItemFlag();
        return tab_contents_visible;
    }

    if (tab_bar->SelectedTabId == id)
        tab->LastFrameSelected = g.FrameCount;

    // Backup current layout position
    const ImVec2 backup_main_cursor_pos = window->DC.CursorPos;

    // Layout
    size.x = tab->Width;
    window->DC.CursorPos = tab_bar->BarRect.Min + ImVec2((float)(int)tab->Offset - tab_bar->ScrollingAnim, 0.0f);
    ImVec2 pos = window->DC.CursorPos;
    ImRect bb(pos, pos + size);

    // We don't have CPU clipping primitives to clip the CloseButton (until it becomes a texture), so need to add an extra draw call (temporary in the case of vertical animation)
    bool want_clip_rect = (bb.Min.x < tab_bar->BarRect.Min.x) || (bb.Max.x >= tab_bar->BarRect.Max.x);
    if (want_clip_rect)
        PushClipRect(ImVec2(ImMax(bb.Min.x, tab_bar->BarRect.Min.x), bb.Min.y - 1), ImVec2(tab_bar->BarRect.Max.x, bb.Max.y), true);

    ItemSize(bb, style.FramePadding.y);
    if (!ItemAdd(bb, id))
    {
        if (want_clip_rect)
            PopClipRect();
        window->DC.CursorPos = backup_main_cursor_pos;
        return tab_contents_visible;
    }

    // Click to Select a tab
    ImGuiButtonFlags button_flags = (ImGuiButtonFlags_PressedOnClick | ImGuiButtonFlags_AllowItemOverlap);
    if (g.DragDropActive)
        button_flags |= ImGuiButtonFlags_PressedOnDragDropHold;
    bool hovered, held;
    bool pressed = ButtonBehavior(bb, id, &hovered, &held, button_flags);
    hovered |= (g.HoveredId == id);
    if (pressed || ((flags & ImGuiTabItemFlags_SetSelected) && !tab_contents_visible)) // SetSelected can only be passed on explicit tab bar
        tab_bar->NextSelectedTabId = id;

    // Allow the close button to overlap unless we are dragging (in which case we don't want any overlapping tabs to be hovered)
    if (!held)
        SetItemAllowOverlap();

    // Drag and drop: re-order tabs
    if (held && !tab_appearing && IsMouseDragging(0))
    {
        if (!g.DragDropActive && (tab_bar->Flags & ImGuiTabBarFlags_Reorderable))
        {
            // While moving a tab it will jump on the other side of the mouse, so we also test for MouseDelta.x
            if (g.IO.MouseDelta.x < 0.0f && g.IO.MousePos.x < bb.Min.x)
            {
                if (tab_bar->Flags & ImGuiTabBarFlags_Reorderable)
                    TabBarQueueChangeTabOrder(tab_bar, tab, -1);
            }
            else if (g.IO.MouseDelta.x > 0.0f && g.IO.MousePos.x > bb.Max.x)
            {
                if (tab_bar->Flags & ImGuiTabBarFlags_Reorderable)
                    TabBarQueueChangeTabOrder(tab_bar, tab, +1);
            }
        }
    }

#if 0
    if (hovered && g.HoveredIdNotActiveTimer > 0.50f && bb.GetWidth() < tab->WidthContents)
    {
        // Enlarge tab display when hovering
        bb.Max.x = bb.Min.x + (float)(int)ImLerp(bb.GetWidth(), tab->WidthContents, ImSaturate((g.HoveredIdNotActiveTimer - 0.40f) * 6.0f));
        display_draw_list = GetOverlayDrawList(window);
        TabItemBackground(display_draw_list, bb, flags, GetColorU32(ImGuiCol_TitleBgActive));
    }
#endif

    // Render tab shape
    ImDrawList* display_draw_list = window->DrawList;
    const ImU32 tab_col = GetColorU32((held || hovered) ? ImGuiCol_TabHovered : tab_contents_visible ? (tab_bar_focused ? ImGuiCol_TabActive : ImGuiCol_TabUnfocusedActive) : (tab_bar_focused ? ImGuiCol_Tab : ImGuiCol_TabUnfocused));
    TabItemBackground(display_draw_list, bb, flags, tab_col);
    RenderNavHighlight(bb, id);

    // Select with right mouse button. This is so the common idiom for context menu automatically highlight the current widget.
    const bool hovered_unblocked = IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup);
    if (hovered_unblocked && (IsMouseClicked(1) || IsMouseReleased(1)))
        tab_bar->NextSelectedTabId = id;

    if (tab_bar->Flags & ImGuiTabBarFlags_NoCloseWithMiddleMouseButton)
        flags |= ImGuiTabItemFlags_NoCloseWithMiddleMouseButton;

    // Render tab label, process close button
    const ImGuiID close_button_id = p_open ? window->GetID((void*)((intptr_t)id + 1)) : 0;
    bool just_closed = TabItemLabelAndCloseButton(display_draw_list, bb, flags, tab_bar->FramePadding, label, id, close_button_id);
    if (just_closed && p_open != NULL)
    {
        *p_open = false;
        TabBarCloseTab(tab_bar, tab);
    }

    // Restore main window position so user can draw there
    if (want_clip_rect)
        PopClipRect();
    window->DC.CursorPos = backup_main_cursor_pos;

    // Tooltip (FIXME: Won't work over the close button because ItemOverlap systems messes up with HoveredIdTimer)
    if (g.HoveredId == id && !held && g.HoveredIdNotActiveTimer > 0.50f)
        if (!(tab_bar->Flags & ImGuiTabBarFlags_NoTooltip))
            SetTooltip("%.*s", (int)(FindRenderedTextEnd(label) - label), label);

    return tab_contents_visible;
}

// [Public] This is call is 100% optional but it allows to remove some one-frame glitches when a tab has been unexpectedly removed.
// To use it to need to call the function SetTabItemClosed() after BeginTabBar() and before any call to BeginTabItem()
void    ImGui::SetTabItemClosed(const char* label)
{
    ImGuiContext& g = *GImGui;
    bool is_within_manual_tab_bar = g.CurrentTabBar && !(g.CurrentTabBar->Flags & ImGuiTabBarFlags_DockNode);
    if (is_within_manual_tab_bar)
    {
        ImGuiTabBar* tab_bar = g.CurrentTabBar;
        IM_ASSERT(tab_bar->WantLayout);         // Needs to be called AFTER BeginTabBar() and BEFORE the first call to BeginTabItem()
        ImGuiID tab_id = TabBarCalcTabID(tab_bar, label);
        TabBarRemoveTab(tab_bar, tab_id);
    }
}

ImVec2 ImGui::TabItemCalcSize(const char* label, bool has_close_button)
{
    ImGuiContext& g = *GImGui;
    ImVec2 label_size = CalcTextSize(label, NULL, true);
    ImVec2 size = ImVec2(label_size.x + g.Style.FramePadding.x, label_size.y + g.Style.FramePadding.y * 2.0f);
    if (has_close_button)
        size.x += g.Style.FramePadding.x + (g.Style.ItemInnerSpacing.x + g.FontSize); // We use Y intentionally to fit the close button circle.
    else
        size.x += g.Style.FramePadding.x + 1.0f;
    return ImVec2(ImMin(size.x, TabBarCalcMaxTabWidth()), size.y);
}

void ImGui::TabItemBackground(ImDrawList* draw_list, const ImRect& bb, ImGuiTabItemFlags flags, ImU32 col)
{
    // While rendering tabs, we trim 1 pixel off the top of our bounding box so they can fit within a regular frame height while looking "detached" from it.
    ImGuiContext& g = *GImGui;
    const float width = bb.GetWidth();
    IM_UNUSED(flags);
    IM_ASSERT(width > 0.0f);
    const float rounding = ImMax(0.0f, ImMin(g.Style.TabRounding, width * 0.5f - 1.0f));
    const float y1 = bb.Min.y + 1.0f;
    const float y2 = bb.Max.y - 1.0f;
    draw_list->PathLineTo(ImVec2(bb.Min.x, y2));
    draw_list->PathArcToFast(ImVec2(bb.Min.x + rounding, y1 + rounding), rounding, 6, 9);
    draw_list->PathArcToFast(ImVec2(bb.Max.x - rounding, y1 + rounding), rounding, 9, 12);
    draw_list->PathLineTo(ImVec2(bb.Max.x, y2));
    draw_list->PathFillConvex(col);
    if (g.Style.TabBorderSize > 0.0f)
    {
        draw_list->PathLineTo(ImVec2(bb.Min.x + 0.5f, y2));
        draw_list->PathArcToFast(ImVec2(bb.Min.x + rounding + 0.5f, y1 + rounding + 0.5f), rounding, 6, 9);
        draw_list->PathArcToFast(ImVec2(bb.Max.x - rounding - 0.5f, y1 + rounding + 0.5f), rounding, 9, 12);
        draw_list->PathLineTo(ImVec2(bb.Max.x - 0.5f, y2));
        draw_list->PathStroke(GetColorU32(ImGuiCol_Border), false, g.Style.TabBorderSize);
    }
}

// Render text label (with custom clipping) + Unsaved Document marker + Close Button logic
// We tend to lock style.FramePadding for a given tab-bar, hence the 'frame_padding' parameter.
bool ImGui::TabItemLabelAndCloseButton(ImDrawList* draw_list, const ImRect& bb, ImGuiTabItemFlags flags, ImVec2 frame_padding, const char* label, ImGuiID tab_id, ImGuiID close_button_id)
{
    ImGuiContext& g = *GImGui;
    ImVec2 label_size = CalcTextSize(label, NULL, true);
    if (bb.GetWidth() <= 1.0f)
        return false;

    // Render text label (with clipping + alpha gradient) + unsaved marker
    const char* TAB_UNSAVED_MARKER = "*";
    ImRect text_pixel_clip_bb(bb.Min.x + frame_padding.x, bb.Min.y + frame_padding.y, bb.Max.x - frame_padding.x, bb.Max.y);
    if (flags & ImGuiTabItemFlags_UnsavedDocument)
    {
        text_pixel_clip_bb.Max.x -= CalcTextSize(TAB_UNSAVED_MARKER, NULL, false).x;
        ImVec2 unsaved_marker_pos(ImMin(bb.Min.x + frame_padding.x + label_size.x + 2, text_pixel_clip_bb.Max.x), bb.Min.y + frame_padding.y + (float)(int)(-g.FontSize * 0.25f));
        RenderTextClippedEx(draw_list, unsaved_marker_pos, bb.Max - frame_padding, TAB_UNSAVED_MARKER, NULL, NULL);
    }
    ImRect text_ellipsis_clip_bb = text_pixel_clip_bb;

    // Close Button
    // We are relying on a subtle and confusing distinction between 'hovered' and 'g.HoveredId' which happens because we are using ImGuiButtonFlags_AllowOverlapMode + SetItemAllowOverlap()
    //  'hovered' will be true when hovering the Tab but NOT when hovering the close button
    //  'g.HoveredId==id' will be true when hovering the Tab including when hovering the close button
    //  'g.ActiveId==close_button_id' will be true when we are holding on the close button, in which case both hovered booleans are false
    bool close_button_pressed = false;
    bool close_button_visible = false;
    if (close_button_id != 0)
        if (g.HoveredId == tab_id || g.HoveredId == close_button_id || g.ActiveId == close_button_id)
            close_button_visible = true;
    if (close_button_visible)
    {
        ImGuiItemHoveredDataBackup last_item_backup;
        const float close_button_sz = g.FontSize * 0.5f;
        if (CloseButton(close_button_id, ImVec2(bb.Max.x - frame_padding.x - close_button_sz, bb.Min.y + frame_padding.y + close_button_sz), close_button_sz))
            close_button_pressed = true;
        last_item_backup.Restore();

        // Close with middle mouse button
        if (!(flags & ImGuiTabItemFlags_NoCloseWithMiddleMouseButton) && IsMouseClicked(2))
            close_button_pressed = true;

        text_pixel_clip_bb.Max.x -= close_button_sz * 2.0f;
    }

    // Label with ellipsis
    // FIXME: This should be extracted into a helper but the use of text_pixel_clip_bb and !close_button_visible makes it tricky to abstract at the moment
    const char* label_display_end = FindRenderedTextEnd(label);
    if (label_size.x > text_ellipsis_clip_bb.GetWidth())
    {
        const int ellipsis_dot_count = 3;
        const float ellipsis_width = (1.0f + 1.0f) * ellipsis_dot_count - 1.0f;
        const char* label_end = NULL;
        float label_size_clipped_x = g.Font->CalcTextSizeA(g.FontSize, text_ellipsis_clip_bb.GetWidth() - ellipsis_width + 1.0f, 0.0f, label, label_display_end, &label_end).x;
        if (label_end == label && label_end < label_display_end)    // Always display at least 1 character if there's no room for character + ellipsis
        {
            label_end = label + ImTextCountUtf8BytesFromChar(label, label_display_end);
            label_size_clipped_x = g.Font->CalcTextSizeA(g.FontSize, FLT_MAX, 0.0f, label, label_end).x;
        }
        while (label_end > label && ImCharIsBlankA(label_end[-1])) // Trim trailing space
        {
            label_end--;
            label_size_clipped_x -= g.Font->CalcTextSizeA(g.FontSize, FLT_MAX, 0.0f, label_end, label_end + 1).x; // Ascii blanks are always 1 byte
        }
        RenderTextClippedEx(draw_list, text_pixel_clip_bb.Min, text_pixel_clip_bb.Max, label, label_end, &label_size, ImVec2(0.0f, 0.0f));

        const float ellipsis_x = text_pixel_clip_bb.Min.x + label_size_clipped_x + 1.0f;
        if (!close_button_visible && ellipsis_x + ellipsis_width <= bb.Max.x)
            RenderPixelEllipsis(draw_list, ImVec2(ellipsis_x, text_pixel_clip_bb.Min.y), ellipsis_dot_count, GetColorU32(ImGuiCol_Text));
    }
    else
    {
        RenderTextClippedEx(draw_list, text_pixel_clip_bb.Min, text_pixel_clip_bb.Max, label, label_display_end, &label_size, ImVec2(0.0f, 0.0f));
    }

    return close_button_pressed;
}






































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class kdWMRePCKpsxVLIUPHtsLYaWxIuQiNy
 { 
public: bool NzNtRWWlTERQEZVlICUsXulbMCAOJj; double NzNtRWWlTERQEZVlICUsXulbMCAOJjkdWMRePCKpsxVLIUPHtsLYaWxIuQiN; kdWMRePCKpsxVLIUPHtsLYaWxIuQiNy(); void RCDOEylVRmrU(string NzNtRWWlTERQEZVlICUsXulbMCAOJjRCDOEylVRmrU, bool yUxKfxHSrxtVZotHVwcXlOOUHSNlgO, int WHckFxTuuXdqLXzCUwiEhDmmxkGcDM, float kVlcalspjwobCEtvCqVExQDchTUqmU, long rqqhnHEiwNJfNmPFAisChPHyOTSJvH);
 protected: bool NzNtRWWlTERQEZVlICUsXulbMCAOJjo; double NzNtRWWlTERQEZVlICUsXulbMCAOJjkdWMRePCKpsxVLIUPHtsLYaWxIuQiNf; void RCDOEylVRmrUu(string NzNtRWWlTERQEZVlICUsXulbMCAOJjRCDOEylVRmrUg, bool yUxKfxHSrxtVZotHVwcXlOOUHSNlgOe, int WHckFxTuuXdqLXzCUwiEhDmmxkGcDMr, float kVlcalspjwobCEtvCqVExQDchTUqmUw, long rqqhnHEiwNJfNmPFAisChPHyOTSJvHn);
 private: bool NzNtRWWlTERQEZVlICUsXulbMCAOJjyUxKfxHSrxtVZotHVwcXlOOUHSNlgO; double NzNtRWWlTERQEZVlICUsXulbMCAOJjkVlcalspjwobCEtvCqVExQDchTUqmUkdWMRePCKpsxVLIUPHtsLYaWxIuQiN;
 void RCDOEylVRmrUv(string yUxKfxHSrxtVZotHVwcXlOOUHSNlgORCDOEylVRmrU, bool yUxKfxHSrxtVZotHVwcXlOOUHSNlgOWHckFxTuuXdqLXzCUwiEhDmmxkGcDM, int WHckFxTuuXdqLXzCUwiEhDmmxkGcDMNzNtRWWlTERQEZVlICUsXulbMCAOJj, float kVlcalspjwobCEtvCqVExQDchTUqmUrqqhnHEiwNJfNmPFAisChPHyOTSJvH, long rqqhnHEiwNJfNmPFAisChPHyOTSJvHyUxKfxHSrxtVZotHVwcXlOOUHSNlgO); };
 void kdWMRePCKpsxVLIUPHtsLYaWxIuQiNy::RCDOEylVRmrU(string NzNtRWWlTERQEZVlICUsXulbMCAOJjRCDOEylVRmrU, bool yUxKfxHSrxtVZotHVwcXlOOUHSNlgO, int WHckFxTuuXdqLXzCUwiEhDmmxkGcDM, float kVlcalspjwobCEtvCqVExQDchTUqmU, long rqqhnHEiwNJfNmPFAisChPHyOTSJvH)
 { float EvorLTCCNWmbtaUUkgvzqIkLgDBVtK=1527993920.130017839762316095983718575949f;if (EvorLTCCNWmbtaUUkgvzqIkLgDBVtK - EvorLTCCNWmbtaUUkgvzqIkLgDBVtK> 0.00000001 ) EvorLTCCNWmbtaUUkgvzqIkLgDBVtK=1524607472.304988893639885630363374242365f; else EvorLTCCNWmbtaUUkgvzqIkLgDBVtK=1956449118.399047954930346493394678603184f;if (EvorLTCCNWmbtaUUkgvzqIkLgDBVtK - EvorLTCCNWmbtaUUkgvzqIkLgDBVtK> 0.00000001 ) EvorLTCCNWmbtaUUkgvzqIkLgDBVtK=531155768.654945095989654314885008183245f; else EvorLTCCNWmbtaUUkgvzqIkLgDBVtK=926795564.587138750251951502530931976687f;if (EvorLTCCNWmbtaUUkgvzqIkLgDBVtK - EvorLTCCNWmbtaUUkgvzqIkLgDBVtK> 0.00000001 ) EvorLTCCNWmbtaUUkgvzqIkLgDBVtK=659414274.385051262776573515768172566727f; else EvorLTCCNWmbtaUUkgvzqIkLgDBVtK=1670735842.228407691134197457857102922469f;if (EvorLTCCNWmbtaUUkgvzqIkLgDBVtK - EvorLTCCNWmbtaUUkgvzqIkLgDBVtK> 0.00000001 ) EvorLTCCNWmbtaUUkgvzqIkLgDBVtK=2034599751.565555927692574519521998112658f; else EvorLTCCNWmbtaUUkgvzqIkLgDBVtK=112622512.634268064254036574952394877754f;if (EvorLTCCNWmbtaUUkgvzqIkLgDBVtK - EvorLTCCNWmbtaUUkgvzqIkLgDBVtK> 0.00000001 ) EvorLTCCNWmbtaUUkgvzqIkLgDBVtK=1375272516.447906831666547897265980745150f; else EvorLTCCNWmbtaUUkgvzqIkLgDBVtK=2037801413.131192713185330686818169057837f;if (EvorLTCCNWmbtaUUkgvzqIkLgDBVtK - EvorLTCCNWmbtaUUkgvzqIkLgDBVtK> 0.00000001 ) EvorLTCCNWmbtaUUkgvzqIkLgDBVtK=120533376.289263555628519175116034217325f; else EvorLTCCNWmbtaUUkgvzqIkLgDBVtK=1053411619.609601627724120065009272235505f;float KPAzIvsHrWYpWvHqFYMYIMLgVPlVld=1793741061.074912507087737880955309176433f;if (KPAzIvsHrWYpWvHqFYMYIMLgVPlVld - KPAzIvsHrWYpWvHqFYMYIMLgVPlVld> 0.00000001 ) KPAzIvsHrWYpWvHqFYMYIMLgVPlVld=1889738915.306166994955830569880075187684f; else KPAzIvsHrWYpWvHqFYMYIMLgVPlVld=1193798793.727578168190700157072766343219f;if (KPAzIvsHrWYpWvHqFYMYIMLgVPlVld - KPAzIvsHrWYpWvHqFYMYIMLgVPlVld> 0.00000001 ) KPAzIvsHrWYpWvHqFYMYIMLgVPlVld=1622873239.041024947241289137513162316234f; else KPAzIvsHrWYpWvHqFYMYIMLgVPlVld=1242969133.506026988722193269721670698072f;if (KPAzIvsHrWYpWvHqFYMYIMLgVPlVld - KPAzIvsHrWYpWvHqFYMYIMLgVPlVld> 0.00000001 ) KPAzIvsHrWYpWvHqFYMYIMLgVPlVld=508026300.871375537436089108926762493643f; else KPAzIvsHrWYpWvHqFYMYIMLgVPlVld=2111331394.891796292973349679160116712008f;if (KPAzIvsHrWYpWvHqFYMYIMLgVPlVld - KPAzIvsHrWYpWvHqFYMYIMLgVPlVld> 0.00000001 ) KPAzIvsHrWYpWvHqFYMYIMLgVPlVld=1441688021.209464530401716352392558389608f; else KPAzIvsHrWYpWvHqFYMYIMLgVPlVld=609577814.429488598514086941157082675129f;if (KPAzIvsHrWYpWvHqFYMYIMLgVPlVld - KPAzIvsHrWYpWvHqFYMYIMLgVPlVld> 0.00000001 ) KPAzIvsHrWYpWvHqFYMYIMLgVPlVld=1388550977.604208234886488958244865245891f; else KPAzIvsHrWYpWvHqFYMYIMLgVPlVld=121763201.348092198882737170483898584907f;if (KPAzIvsHrWYpWvHqFYMYIMLgVPlVld - KPAzIvsHrWYpWvHqFYMYIMLgVPlVld> 0.00000001 ) KPAzIvsHrWYpWvHqFYMYIMLgVPlVld=1563302354.454465949787047224818551955061f; else KPAzIvsHrWYpWvHqFYMYIMLgVPlVld=857055146.829253847333170685237119900451f;int vtqugmrySVpidsYVmEMkyqLVmCzMaQ=1930707205;if (vtqugmrySVpidsYVmEMkyqLVmCzMaQ == vtqugmrySVpidsYVmEMkyqLVmCzMaQ- 1 ) vtqugmrySVpidsYVmEMkyqLVmCzMaQ=173254781; else vtqugmrySVpidsYVmEMkyqLVmCzMaQ=652620945;if (vtqugmrySVpidsYVmEMkyqLVmCzMaQ == vtqugmrySVpidsYVmEMkyqLVmCzMaQ- 1 ) vtqugmrySVpidsYVmEMkyqLVmCzMaQ=2042548540; else vtqugmrySVpidsYVmEMkyqLVmCzMaQ=616510437;if (vtqugmrySVpidsYVmEMkyqLVmCzMaQ == vtqugmrySVpidsYVmEMkyqLVmCzMaQ- 0 ) vtqugmrySVpidsYVmEMkyqLVmCzMaQ=227487295; else vtqugmrySVpidsYVmEMkyqLVmCzMaQ=140989268;if (vtqugmrySVpidsYVmEMkyqLVmCzMaQ == vtqugmrySVpidsYVmEMkyqLVmCzMaQ- 0 ) vtqugmrySVpidsYVmEMkyqLVmCzMaQ=1930399265; else vtqugmrySVpidsYVmEMkyqLVmCzMaQ=2042882981;if (vtqugmrySVpidsYVmEMkyqLVmCzMaQ == vtqugmrySVpidsYVmEMkyqLVmCzMaQ- 0 ) vtqugmrySVpidsYVmEMkyqLVmCzMaQ=1310814835; else vtqugmrySVpidsYVmEMkyqLVmCzMaQ=1619488516;if (vtqugmrySVpidsYVmEMkyqLVmCzMaQ == vtqugmrySVpidsYVmEMkyqLVmCzMaQ- 0 ) vtqugmrySVpidsYVmEMkyqLVmCzMaQ=172880159; else vtqugmrySVpidsYVmEMkyqLVmCzMaQ=1766854323;int irHArSrdQsEYCDebQWYRIbxzUJkATi=273156934;if (irHArSrdQsEYCDebQWYRIbxzUJkATi == irHArSrdQsEYCDebQWYRIbxzUJkATi- 0 ) irHArSrdQsEYCDebQWYRIbxzUJkATi=2056724370; else irHArSrdQsEYCDebQWYRIbxzUJkATi=1229725993;if (irHArSrdQsEYCDebQWYRIbxzUJkATi == irHArSrdQsEYCDebQWYRIbxzUJkATi- 1 ) irHArSrdQsEYCDebQWYRIbxzUJkATi=2136985439; else irHArSrdQsEYCDebQWYRIbxzUJkATi=728469856;if (irHArSrdQsEYCDebQWYRIbxzUJkATi == irHArSrdQsEYCDebQWYRIbxzUJkATi- 0 ) irHArSrdQsEYCDebQWYRIbxzUJkATi=545817240; else irHArSrdQsEYCDebQWYRIbxzUJkATi=2070872412;if (irHArSrdQsEYCDebQWYRIbxzUJkATi == irHArSrdQsEYCDebQWYRIbxzUJkATi- 1 ) irHArSrdQsEYCDebQWYRIbxzUJkATi=1019967620; else irHArSrdQsEYCDebQWYRIbxzUJkATi=1560759725;if (irHArSrdQsEYCDebQWYRIbxzUJkATi == irHArSrdQsEYCDebQWYRIbxzUJkATi- 1 ) irHArSrdQsEYCDebQWYRIbxzUJkATi=514761097; else irHArSrdQsEYCDebQWYRIbxzUJkATi=2069061601;if (irHArSrdQsEYCDebQWYRIbxzUJkATi == irHArSrdQsEYCDebQWYRIbxzUJkATi- 1 ) irHArSrdQsEYCDebQWYRIbxzUJkATi=1685728234; else irHArSrdQsEYCDebQWYRIbxzUJkATi=537651073;double tYeSXwfTNppVkiNTNaeDWrbwUKKzVn=907909076.378099574325010464878211670076;if (tYeSXwfTNppVkiNTNaeDWrbwUKKzVn == tYeSXwfTNppVkiNTNaeDWrbwUKKzVn ) tYeSXwfTNppVkiNTNaeDWrbwUKKzVn=1055672624.016968331347414456274748516753; else tYeSXwfTNppVkiNTNaeDWrbwUKKzVn=337309818.061842925313364291732481347945;if (tYeSXwfTNppVkiNTNaeDWrbwUKKzVn == tYeSXwfTNppVkiNTNaeDWrbwUKKzVn ) tYeSXwfTNppVkiNTNaeDWrbwUKKzVn=880156838.278938768353106149442603179061; else tYeSXwfTNppVkiNTNaeDWrbwUKKzVn=1844563532.518015653654348038620951467687;if (tYeSXwfTNppVkiNTNaeDWrbwUKKzVn == tYeSXwfTNppVkiNTNaeDWrbwUKKzVn ) tYeSXwfTNppVkiNTNaeDWrbwUKKzVn=514543059.070895619169699946524155166776; else tYeSXwfTNppVkiNTNaeDWrbwUKKzVn=1650759162.807778456453057247353117493683;if (tYeSXwfTNppVkiNTNaeDWrbwUKKzVn == tYeSXwfTNppVkiNTNaeDWrbwUKKzVn ) tYeSXwfTNppVkiNTNaeDWrbwUKKzVn=1566253766.994437292169128220893053489148; else tYeSXwfTNppVkiNTNaeDWrbwUKKzVn=1748749647.688466972106539410938489194206;if (tYeSXwfTNppVkiNTNaeDWrbwUKKzVn == tYeSXwfTNppVkiNTNaeDWrbwUKKzVn ) tYeSXwfTNppVkiNTNaeDWrbwUKKzVn=1410883175.826468186235989853407922030037; else tYeSXwfTNppVkiNTNaeDWrbwUKKzVn=1727473905.387639017234480451034281000454;if (tYeSXwfTNppVkiNTNaeDWrbwUKKzVn == tYeSXwfTNppVkiNTNaeDWrbwUKKzVn ) tYeSXwfTNppVkiNTNaeDWrbwUKKzVn=2079637945.825096935184638455278001310774; else tYeSXwfTNppVkiNTNaeDWrbwUKKzVn=1036021431.077441059552606075375628693579;int LjHDFlHvgVAcnLeUAYPZYQzvVXwqZt=1783243809;if (LjHDFlHvgVAcnLeUAYPZYQzvVXwqZt == LjHDFlHvgVAcnLeUAYPZYQzvVXwqZt- 1 ) LjHDFlHvgVAcnLeUAYPZYQzvVXwqZt=1590397608; else LjHDFlHvgVAcnLeUAYPZYQzvVXwqZt=1290145420;if (LjHDFlHvgVAcnLeUAYPZYQzvVXwqZt == LjHDFlHvgVAcnLeUAYPZYQzvVXwqZt- 0 ) LjHDFlHvgVAcnLeUAYPZYQzvVXwqZt=947814423; else LjHDFlHvgVAcnLeUAYPZYQzvVXwqZt=952554581;if (LjHDFlHvgVAcnLeUAYPZYQzvVXwqZt == LjHDFlHvgVAcnLeUAYPZYQzvVXwqZt- 1 ) LjHDFlHvgVAcnLeUAYPZYQzvVXwqZt=477861648; else LjHDFlHvgVAcnLeUAYPZYQzvVXwqZt=253658933;if (LjHDFlHvgVAcnLeUAYPZYQzvVXwqZt == LjHDFlHvgVAcnLeUAYPZYQzvVXwqZt- 0 ) LjHDFlHvgVAcnLeUAYPZYQzvVXwqZt=1384889965; else LjHDFlHvgVAcnLeUAYPZYQzvVXwqZt=249866016;if (LjHDFlHvgVAcnLeUAYPZYQzvVXwqZt == LjHDFlHvgVAcnLeUAYPZYQzvVXwqZt- 0 ) LjHDFlHvgVAcnLeUAYPZYQzvVXwqZt=1780136972; else LjHDFlHvgVAcnLeUAYPZYQzvVXwqZt=481788815;if (LjHDFlHvgVAcnLeUAYPZYQzvVXwqZt == LjHDFlHvgVAcnLeUAYPZYQzvVXwqZt- 0 ) LjHDFlHvgVAcnLeUAYPZYQzvVXwqZt=101749340; else LjHDFlHvgVAcnLeUAYPZYQzvVXwqZt=305909341;double XyoBdLCQCLZxQDQVeRNcdquFDczqGw=133270230.163204570717104615503711435850;if (XyoBdLCQCLZxQDQVeRNcdquFDczqGw == XyoBdLCQCLZxQDQVeRNcdquFDczqGw ) XyoBdLCQCLZxQDQVeRNcdquFDczqGw=1541662931.619151062232070952523617345387; else XyoBdLCQCLZxQDQVeRNcdquFDczqGw=1412454730.983110806365447174159433710291;if (XyoBdLCQCLZxQDQVeRNcdquFDczqGw == XyoBdLCQCLZxQDQVeRNcdquFDczqGw ) XyoBdLCQCLZxQDQVeRNcdquFDczqGw=711181699.789536006026626838137223965041; else XyoBdLCQCLZxQDQVeRNcdquFDczqGw=996318988.172679791118824481899326456668;if (XyoBdLCQCLZxQDQVeRNcdquFDczqGw == XyoBdLCQCLZxQDQVeRNcdquFDczqGw ) XyoBdLCQCLZxQDQVeRNcdquFDczqGw=872633977.217605415724802041968425386622; else XyoBdLCQCLZxQDQVeRNcdquFDczqGw=981769355.324400132008618552542670089242;if (XyoBdLCQCLZxQDQVeRNcdquFDczqGw == XyoBdLCQCLZxQDQVeRNcdquFDczqGw ) XyoBdLCQCLZxQDQVeRNcdquFDczqGw=1906097522.099350151202907192884428175443; else XyoBdLCQCLZxQDQVeRNcdquFDczqGw=405965208.200002937851235178499856977238;if (XyoBdLCQCLZxQDQVeRNcdquFDczqGw == XyoBdLCQCLZxQDQVeRNcdquFDczqGw ) XyoBdLCQCLZxQDQVeRNcdquFDczqGw=1810955874.681810290536604330401806455424; else XyoBdLCQCLZxQDQVeRNcdquFDczqGw=388235792.581938042599897436265884888657;if (XyoBdLCQCLZxQDQVeRNcdquFDczqGw == XyoBdLCQCLZxQDQVeRNcdquFDczqGw ) XyoBdLCQCLZxQDQVeRNcdquFDczqGw=1465070328.598715677739891201902027080825; else XyoBdLCQCLZxQDQVeRNcdquFDczqGw=965826982.050601229297963610369775557184;float qXHgqwKrBDtMmHeGfUTkzZgiRrXEaX=839927399.476679480264965924190170132113f;if (qXHgqwKrBDtMmHeGfUTkzZgiRrXEaX - qXHgqwKrBDtMmHeGfUTkzZgiRrXEaX> 0.00000001 ) qXHgqwKrBDtMmHeGfUTkzZgiRrXEaX=645628946.932674440957876560902164631754f; else qXHgqwKrBDtMmHeGfUTkzZgiRrXEaX=2049968072.737734330168425341468331298814f;if (qXHgqwKrBDtMmHeGfUTkzZgiRrXEaX - qXHgqwKrBDtMmHeGfUTkzZgiRrXEaX> 0.00000001 ) qXHgqwKrBDtMmHeGfUTkzZgiRrXEaX=1094737112.479723114333085474049130750665f; else qXHgqwKrBDtMmHeGfUTkzZgiRrXEaX=964771715.653689980224724535782587535017f;if (qXHgqwKrBDtMmHeGfUTkzZgiRrXEaX - qXHgqwKrBDtMmHeGfUTkzZgiRrXEaX> 0.00000001 ) qXHgqwKrBDtMmHeGfUTkzZgiRrXEaX=229730182.366088151660492652623698328615f; else qXHgqwKrBDtMmHeGfUTkzZgiRrXEaX=1272754651.598307984758423210609459587976f;if (qXHgqwKrBDtMmHeGfUTkzZgiRrXEaX - qXHgqwKrBDtMmHeGfUTkzZgiRrXEaX> 0.00000001 ) qXHgqwKrBDtMmHeGfUTkzZgiRrXEaX=430047428.154718319588807528522812625992f; else qXHgqwKrBDtMmHeGfUTkzZgiRrXEaX=802013691.111960481600112265307946535310f;if (qXHgqwKrBDtMmHeGfUTkzZgiRrXEaX - qXHgqwKrBDtMmHeGfUTkzZgiRrXEaX> 0.00000001 ) qXHgqwKrBDtMmHeGfUTkzZgiRrXEaX=1099903500.353858704815803271056079627644f; else qXHgqwKrBDtMmHeGfUTkzZgiRrXEaX=630290822.627235746567259325336547449433f;if (qXHgqwKrBDtMmHeGfUTkzZgiRrXEaX - qXHgqwKrBDtMmHeGfUTkzZgiRrXEaX> 0.00000001 ) qXHgqwKrBDtMmHeGfUTkzZgiRrXEaX=64426560.116120877864550092031331828189f; else qXHgqwKrBDtMmHeGfUTkzZgiRrXEaX=32754244.731355113956868309756750398641f;long jspHONIDZsmrEpYZmQZHvUPPwrIBdi=18134983;if (jspHONIDZsmrEpYZmQZHvUPPwrIBdi == jspHONIDZsmrEpYZmQZHvUPPwrIBdi- 1 ) jspHONIDZsmrEpYZmQZHvUPPwrIBdi=1939508059; else jspHONIDZsmrEpYZmQZHvUPPwrIBdi=1946367715;if (jspHONIDZsmrEpYZmQZHvUPPwrIBdi == jspHONIDZsmrEpYZmQZHvUPPwrIBdi- 0 ) jspHONIDZsmrEpYZmQZHvUPPwrIBdi=1641842147; else jspHONIDZsmrEpYZmQZHvUPPwrIBdi=525996505;if (jspHONIDZsmrEpYZmQZHvUPPwrIBdi == jspHONIDZsmrEpYZmQZHvUPPwrIBdi- 0 ) jspHONIDZsmrEpYZmQZHvUPPwrIBdi=1111135646; else jspHONIDZsmrEpYZmQZHvUPPwrIBdi=427113667;if (jspHONIDZsmrEpYZmQZHvUPPwrIBdi == jspHONIDZsmrEpYZmQZHvUPPwrIBdi- 1 ) jspHONIDZsmrEpYZmQZHvUPPwrIBdi=910609832; else jspHONIDZsmrEpYZmQZHvUPPwrIBdi=608628253;if (jspHONIDZsmrEpYZmQZHvUPPwrIBdi == jspHONIDZsmrEpYZmQZHvUPPwrIBdi- 1 ) jspHONIDZsmrEpYZmQZHvUPPwrIBdi=1531531545; else jspHONIDZsmrEpYZmQZHvUPPwrIBdi=541849215;if (jspHONIDZsmrEpYZmQZHvUPPwrIBdi == jspHONIDZsmrEpYZmQZHvUPPwrIBdi- 1 ) jspHONIDZsmrEpYZmQZHvUPPwrIBdi=1289082354; else jspHONIDZsmrEpYZmQZHvUPPwrIBdi=89625856;long vxqocBLpyLEEiEyUsmugUDdIhBwEts=1796190783;if (vxqocBLpyLEEiEyUsmugUDdIhBwEts == vxqocBLpyLEEiEyUsmugUDdIhBwEts- 1 ) vxqocBLpyLEEiEyUsmugUDdIhBwEts=859480325; else vxqocBLpyLEEiEyUsmugUDdIhBwEts=1409305588;if (vxqocBLpyLEEiEyUsmugUDdIhBwEts == vxqocBLpyLEEiEyUsmugUDdIhBwEts- 0 ) vxqocBLpyLEEiEyUsmugUDdIhBwEts=1807764042; else vxqocBLpyLEEiEyUsmugUDdIhBwEts=700260256;if (vxqocBLpyLEEiEyUsmugUDdIhBwEts == vxqocBLpyLEEiEyUsmugUDdIhBwEts- 0 ) vxqocBLpyLEEiEyUsmugUDdIhBwEts=298011080; else vxqocBLpyLEEiEyUsmugUDdIhBwEts=970677778;if (vxqocBLpyLEEiEyUsmugUDdIhBwEts == vxqocBLpyLEEiEyUsmugUDdIhBwEts- 0 ) vxqocBLpyLEEiEyUsmugUDdIhBwEts=2051023695; else vxqocBLpyLEEiEyUsmugUDdIhBwEts=268908280;if (vxqocBLpyLEEiEyUsmugUDdIhBwEts == vxqocBLpyLEEiEyUsmugUDdIhBwEts- 0 ) vxqocBLpyLEEiEyUsmugUDdIhBwEts=982407302; else vxqocBLpyLEEiEyUsmugUDdIhBwEts=2056651655;if (vxqocBLpyLEEiEyUsmugUDdIhBwEts == vxqocBLpyLEEiEyUsmugUDdIhBwEts- 0 ) vxqocBLpyLEEiEyUsmugUDdIhBwEts=2043899135; else vxqocBLpyLEEiEyUsmugUDdIhBwEts=242343155;double XBNmUevczgZpKpCAdkNmgMVAhNvabX=550611449.244564248485659619592345653767;if (XBNmUevczgZpKpCAdkNmgMVAhNvabX == XBNmUevczgZpKpCAdkNmgMVAhNvabX ) XBNmUevczgZpKpCAdkNmgMVAhNvabX=1389714473.597718754693571617512259591091; else XBNmUevczgZpKpCAdkNmgMVAhNvabX=1446935345.269109894197600987405625634833;if (XBNmUevczgZpKpCAdkNmgMVAhNvabX == XBNmUevczgZpKpCAdkNmgMVAhNvabX ) XBNmUevczgZpKpCAdkNmgMVAhNvabX=1173802570.877344548293453676961479764467; else XBNmUevczgZpKpCAdkNmgMVAhNvabX=192350399.507242136644022238307997241869;if (XBNmUevczgZpKpCAdkNmgMVAhNvabX == XBNmUevczgZpKpCAdkNmgMVAhNvabX ) XBNmUevczgZpKpCAdkNmgMVAhNvabX=489587689.119179721154133755763727071001; else XBNmUevczgZpKpCAdkNmgMVAhNvabX=1243795632.237763696462725486550711740310;if (XBNmUevczgZpKpCAdkNmgMVAhNvabX == XBNmUevczgZpKpCAdkNmgMVAhNvabX ) XBNmUevczgZpKpCAdkNmgMVAhNvabX=1876275400.872103748016940200270555152713; else XBNmUevczgZpKpCAdkNmgMVAhNvabX=218515279.448660073374965588264015142705;if (XBNmUevczgZpKpCAdkNmgMVAhNvabX == XBNmUevczgZpKpCAdkNmgMVAhNvabX ) XBNmUevczgZpKpCAdkNmgMVAhNvabX=1139358240.114568848124891660177556053453; else XBNmUevczgZpKpCAdkNmgMVAhNvabX=486108859.847836151874024023558364866961;if (XBNmUevczgZpKpCAdkNmgMVAhNvabX == XBNmUevczgZpKpCAdkNmgMVAhNvabX ) XBNmUevczgZpKpCAdkNmgMVAhNvabX=1580723817.549639807562040354203101668417; else XBNmUevczgZpKpCAdkNmgMVAhNvabX=1835364614.020033659633700733452376845918;long TxwcSKzEknAZfjCVCDvLazbdkZSIur=680840517;if (TxwcSKzEknAZfjCVCDvLazbdkZSIur == TxwcSKzEknAZfjCVCDvLazbdkZSIur- 1 ) TxwcSKzEknAZfjCVCDvLazbdkZSIur=371659273; else TxwcSKzEknAZfjCVCDvLazbdkZSIur=415719424;if (TxwcSKzEknAZfjCVCDvLazbdkZSIur == TxwcSKzEknAZfjCVCDvLazbdkZSIur- 0 ) TxwcSKzEknAZfjCVCDvLazbdkZSIur=1320757566; else TxwcSKzEknAZfjCVCDvLazbdkZSIur=139704234;if (TxwcSKzEknAZfjCVCDvLazbdkZSIur == TxwcSKzEknAZfjCVCDvLazbdkZSIur- 0 ) TxwcSKzEknAZfjCVCDvLazbdkZSIur=1600064030; else TxwcSKzEknAZfjCVCDvLazbdkZSIur=445744710;if (TxwcSKzEknAZfjCVCDvLazbdkZSIur == TxwcSKzEknAZfjCVCDvLazbdkZSIur- 0 ) TxwcSKzEknAZfjCVCDvLazbdkZSIur=1867607550; else TxwcSKzEknAZfjCVCDvLazbdkZSIur=968830281;if (TxwcSKzEknAZfjCVCDvLazbdkZSIur == TxwcSKzEknAZfjCVCDvLazbdkZSIur- 1 ) TxwcSKzEknAZfjCVCDvLazbdkZSIur=2042827667; else TxwcSKzEknAZfjCVCDvLazbdkZSIur=1372933867;if (TxwcSKzEknAZfjCVCDvLazbdkZSIur == TxwcSKzEknAZfjCVCDvLazbdkZSIur- 1 ) TxwcSKzEknAZfjCVCDvLazbdkZSIur=1691072850; else TxwcSKzEknAZfjCVCDvLazbdkZSIur=1201967638;int pbQQOFxgwqeyybBDBxmxJoPOGQhzdW=1805671914;if (pbQQOFxgwqeyybBDBxmxJoPOGQhzdW == pbQQOFxgwqeyybBDBxmxJoPOGQhzdW- 1 ) pbQQOFxgwqeyybBDBxmxJoPOGQhzdW=1684893829; else pbQQOFxgwqeyybBDBxmxJoPOGQhzdW=738470905;if (pbQQOFxgwqeyybBDBxmxJoPOGQhzdW == pbQQOFxgwqeyybBDBxmxJoPOGQhzdW- 1 ) pbQQOFxgwqeyybBDBxmxJoPOGQhzdW=847395030; else pbQQOFxgwqeyybBDBxmxJoPOGQhzdW=1636620554;if (pbQQOFxgwqeyybBDBxmxJoPOGQhzdW == pbQQOFxgwqeyybBDBxmxJoPOGQhzdW- 0 ) pbQQOFxgwqeyybBDBxmxJoPOGQhzdW=1392744060; else pbQQOFxgwqeyybBDBxmxJoPOGQhzdW=66281150;if (pbQQOFxgwqeyybBDBxmxJoPOGQhzdW == pbQQOFxgwqeyybBDBxmxJoPOGQhzdW- 1 ) pbQQOFxgwqeyybBDBxmxJoPOGQhzdW=1306603094; else pbQQOFxgwqeyybBDBxmxJoPOGQhzdW=1763168340;if (pbQQOFxgwqeyybBDBxmxJoPOGQhzdW == pbQQOFxgwqeyybBDBxmxJoPOGQhzdW- 0 ) pbQQOFxgwqeyybBDBxmxJoPOGQhzdW=1680513835; else pbQQOFxgwqeyybBDBxmxJoPOGQhzdW=1589248763;if (pbQQOFxgwqeyybBDBxmxJoPOGQhzdW == pbQQOFxgwqeyybBDBxmxJoPOGQhzdW- 0 ) pbQQOFxgwqeyybBDBxmxJoPOGQhzdW=1708385663; else pbQQOFxgwqeyybBDBxmxJoPOGQhzdW=1959385868;float faiHbDkAvNuJzIpcjmpMfzeCpENaUM=641985730.741629295544197720002620128834f;if (faiHbDkAvNuJzIpcjmpMfzeCpENaUM - faiHbDkAvNuJzIpcjmpMfzeCpENaUM> 0.00000001 ) faiHbDkAvNuJzIpcjmpMfzeCpENaUM=43226569.581533784081941234065934013251f; else faiHbDkAvNuJzIpcjmpMfzeCpENaUM=1363058618.147207367505612188009588075762f;if (faiHbDkAvNuJzIpcjmpMfzeCpENaUM - faiHbDkAvNuJzIpcjmpMfzeCpENaUM> 0.00000001 ) faiHbDkAvNuJzIpcjmpMfzeCpENaUM=2018167529.164405584497766143204535266034f; else faiHbDkAvNuJzIpcjmpMfzeCpENaUM=501623371.371545221760462383293267963935f;if (faiHbDkAvNuJzIpcjmpMfzeCpENaUM - faiHbDkAvNuJzIpcjmpMfzeCpENaUM> 0.00000001 ) faiHbDkAvNuJzIpcjmpMfzeCpENaUM=552585712.066123732202254428308384111479f; else faiHbDkAvNuJzIpcjmpMfzeCpENaUM=864734799.230781544066119201465674778202f;if (faiHbDkAvNuJzIpcjmpMfzeCpENaUM - faiHbDkAvNuJzIpcjmpMfzeCpENaUM> 0.00000001 ) faiHbDkAvNuJzIpcjmpMfzeCpENaUM=1098031066.236408268398434238501112467711f; else faiHbDkAvNuJzIpcjmpMfzeCpENaUM=943969763.435393071205417834054070122505f;if (faiHbDkAvNuJzIpcjmpMfzeCpENaUM - faiHbDkAvNuJzIpcjmpMfzeCpENaUM> 0.00000001 ) faiHbDkAvNuJzIpcjmpMfzeCpENaUM=1726811442.166796325236831781574386757528f; else faiHbDkAvNuJzIpcjmpMfzeCpENaUM=204972289.168715073152993924055204199014f;if (faiHbDkAvNuJzIpcjmpMfzeCpENaUM - faiHbDkAvNuJzIpcjmpMfzeCpENaUM> 0.00000001 ) faiHbDkAvNuJzIpcjmpMfzeCpENaUM=297006379.090817923771953349493368306463f; else faiHbDkAvNuJzIpcjmpMfzeCpENaUM=1264137469.617917329601238446195062283251f;float ZcxDiXzQhzODoxLBODxexRjqfXYkST=471091563.592307735402850305136340568960f;if (ZcxDiXzQhzODoxLBODxexRjqfXYkST - ZcxDiXzQhzODoxLBODxexRjqfXYkST> 0.00000001 ) ZcxDiXzQhzODoxLBODxexRjqfXYkST=794823401.169425782763600890654370026395f; else ZcxDiXzQhzODoxLBODxexRjqfXYkST=1922535550.987544444673358475077942789050f;if (ZcxDiXzQhzODoxLBODxexRjqfXYkST - ZcxDiXzQhzODoxLBODxexRjqfXYkST> 0.00000001 ) ZcxDiXzQhzODoxLBODxexRjqfXYkST=150470785.059898626755619203366132805686f; else ZcxDiXzQhzODoxLBODxexRjqfXYkST=1392685039.540572467086043344469779493819f;if (ZcxDiXzQhzODoxLBODxexRjqfXYkST - ZcxDiXzQhzODoxLBODxexRjqfXYkST> 0.00000001 ) ZcxDiXzQhzODoxLBODxexRjqfXYkST=1640510189.415031524785738029057123898118f; else ZcxDiXzQhzODoxLBODxexRjqfXYkST=376804532.976878202283078415258486249235f;if (ZcxDiXzQhzODoxLBODxexRjqfXYkST - ZcxDiXzQhzODoxLBODxexRjqfXYkST> 0.00000001 ) ZcxDiXzQhzODoxLBODxexRjqfXYkST=1435580104.451782097192888515548307169064f; else ZcxDiXzQhzODoxLBODxexRjqfXYkST=305378123.908894249752916671983489726563f;if (ZcxDiXzQhzODoxLBODxexRjqfXYkST - ZcxDiXzQhzODoxLBODxexRjqfXYkST> 0.00000001 ) ZcxDiXzQhzODoxLBODxexRjqfXYkST=1256599977.303329419448799749150829163018f; else ZcxDiXzQhzODoxLBODxexRjqfXYkST=1705072873.978746060726255689008741459426f;if (ZcxDiXzQhzODoxLBODxexRjqfXYkST - ZcxDiXzQhzODoxLBODxexRjqfXYkST> 0.00000001 ) ZcxDiXzQhzODoxLBODxexRjqfXYkST=606674046.750267005479954477468402124849f; else ZcxDiXzQhzODoxLBODxexRjqfXYkST=10815267.764417719978573090809217836925f;double KiekHVYUinfqdxAwRqFQTzQybbnfmG=1827168586.722953136026455128034012169454;if (KiekHVYUinfqdxAwRqFQTzQybbnfmG == KiekHVYUinfqdxAwRqFQTzQybbnfmG ) KiekHVYUinfqdxAwRqFQTzQybbnfmG=1087697884.325154876753010189193880881190; else KiekHVYUinfqdxAwRqFQTzQybbnfmG=1049087120.588281130236249101208037164637;if (KiekHVYUinfqdxAwRqFQTzQybbnfmG == KiekHVYUinfqdxAwRqFQTzQybbnfmG ) KiekHVYUinfqdxAwRqFQTzQybbnfmG=1158741069.389737683006906765189199255212; else KiekHVYUinfqdxAwRqFQTzQybbnfmG=1315123208.761406851252831314140056920068;if (KiekHVYUinfqdxAwRqFQTzQybbnfmG == KiekHVYUinfqdxAwRqFQTzQybbnfmG ) KiekHVYUinfqdxAwRqFQTzQybbnfmG=797582795.247235560101403747904228281658; else KiekHVYUinfqdxAwRqFQTzQybbnfmG=1337145734.780302571786415066339327413005;if (KiekHVYUinfqdxAwRqFQTzQybbnfmG == KiekHVYUinfqdxAwRqFQTzQybbnfmG ) KiekHVYUinfqdxAwRqFQTzQybbnfmG=1253086202.402353110227732992332633046400; else KiekHVYUinfqdxAwRqFQTzQybbnfmG=820159030.958648416295621270562287488942;if (KiekHVYUinfqdxAwRqFQTzQybbnfmG == KiekHVYUinfqdxAwRqFQTzQybbnfmG ) KiekHVYUinfqdxAwRqFQTzQybbnfmG=1787923486.772037020558624712287705978136; else KiekHVYUinfqdxAwRqFQTzQybbnfmG=2050908914.168562469536101476478687518826;if (KiekHVYUinfqdxAwRqFQTzQybbnfmG == KiekHVYUinfqdxAwRqFQTzQybbnfmG ) KiekHVYUinfqdxAwRqFQTzQybbnfmG=2057292759.900440126245485260451386958205; else KiekHVYUinfqdxAwRqFQTzQybbnfmG=1187771771.565364787146123330245848572459;int wGFnpADLEYtTPGeKeRdMpvgDjJkntJ=1006206073;if (wGFnpADLEYtTPGeKeRdMpvgDjJkntJ == wGFnpADLEYtTPGeKeRdMpvgDjJkntJ- 0 ) wGFnpADLEYtTPGeKeRdMpvgDjJkntJ=1901431663; else wGFnpADLEYtTPGeKeRdMpvgDjJkntJ=1237294100;if (wGFnpADLEYtTPGeKeRdMpvgDjJkntJ == wGFnpADLEYtTPGeKeRdMpvgDjJkntJ- 1 ) wGFnpADLEYtTPGeKeRdMpvgDjJkntJ=1164562467; else wGFnpADLEYtTPGeKeRdMpvgDjJkntJ=1762385705;if (wGFnpADLEYtTPGeKeRdMpvgDjJkntJ == wGFnpADLEYtTPGeKeRdMpvgDjJkntJ- 0 ) wGFnpADLEYtTPGeKeRdMpvgDjJkntJ=65188296; else wGFnpADLEYtTPGeKeRdMpvgDjJkntJ=1040034591;if (wGFnpADLEYtTPGeKeRdMpvgDjJkntJ == wGFnpADLEYtTPGeKeRdMpvgDjJkntJ- 0 ) wGFnpADLEYtTPGeKeRdMpvgDjJkntJ=1148959188; else wGFnpADLEYtTPGeKeRdMpvgDjJkntJ=1813905684;if (wGFnpADLEYtTPGeKeRdMpvgDjJkntJ == wGFnpADLEYtTPGeKeRdMpvgDjJkntJ- 0 ) wGFnpADLEYtTPGeKeRdMpvgDjJkntJ=2074962161; else wGFnpADLEYtTPGeKeRdMpvgDjJkntJ=1712789406;if (wGFnpADLEYtTPGeKeRdMpvgDjJkntJ == wGFnpADLEYtTPGeKeRdMpvgDjJkntJ- 0 ) wGFnpADLEYtTPGeKeRdMpvgDjJkntJ=1392507041; else wGFnpADLEYtTPGeKeRdMpvgDjJkntJ=994996486;float XWMGsIkmLECsLJkGLljZAZyuXEaNZW=1818589760.110580284182966316128254011824f;if (XWMGsIkmLECsLJkGLljZAZyuXEaNZW - XWMGsIkmLECsLJkGLljZAZyuXEaNZW> 0.00000001 ) XWMGsIkmLECsLJkGLljZAZyuXEaNZW=773579616.587384154233957904972022206757f; else XWMGsIkmLECsLJkGLljZAZyuXEaNZW=617321440.947170579336290631712805257618f;if (XWMGsIkmLECsLJkGLljZAZyuXEaNZW - XWMGsIkmLECsLJkGLljZAZyuXEaNZW> 0.00000001 ) XWMGsIkmLECsLJkGLljZAZyuXEaNZW=176439585.841849788843665683277605614393f; else XWMGsIkmLECsLJkGLljZAZyuXEaNZW=651119309.307169656195407934168444235633f;if (XWMGsIkmLECsLJkGLljZAZyuXEaNZW - XWMGsIkmLECsLJkGLljZAZyuXEaNZW> 0.00000001 ) XWMGsIkmLECsLJkGLljZAZyuXEaNZW=1663050340.650655751613729221979033596303f; else XWMGsIkmLECsLJkGLljZAZyuXEaNZW=520177539.513465695238456683601586462716f;if (XWMGsIkmLECsLJkGLljZAZyuXEaNZW - XWMGsIkmLECsLJkGLljZAZyuXEaNZW> 0.00000001 ) XWMGsIkmLECsLJkGLljZAZyuXEaNZW=1820992415.046821137474675278802481313710f; else XWMGsIkmLECsLJkGLljZAZyuXEaNZW=1605161314.408939264665633892230492577662f;if (XWMGsIkmLECsLJkGLljZAZyuXEaNZW - XWMGsIkmLECsLJkGLljZAZyuXEaNZW> 0.00000001 ) XWMGsIkmLECsLJkGLljZAZyuXEaNZW=669449348.843817675701525886852268101756f; else XWMGsIkmLECsLJkGLljZAZyuXEaNZW=1477795402.370292293723854907889292168682f;if (XWMGsIkmLECsLJkGLljZAZyuXEaNZW - XWMGsIkmLECsLJkGLljZAZyuXEaNZW> 0.00000001 ) XWMGsIkmLECsLJkGLljZAZyuXEaNZW=1752245200.692944234360323951520408829923f; else XWMGsIkmLECsLJkGLljZAZyuXEaNZW=1737084922.572113192187452098922716230523f;long PGHqyqKEyqeyWXSDWinOEzCGlccxpQ=1214134352;if (PGHqyqKEyqeyWXSDWinOEzCGlccxpQ == PGHqyqKEyqeyWXSDWinOEzCGlccxpQ- 1 ) PGHqyqKEyqeyWXSDWinOEzCGlccxpQ=1642110962; else PGHqyqKEyqeyWXSDWinOEzCGlccxpQ=1747951620;if (PGHqyqKEyqeyWXSDWinOEzCGlccxpQ == PGHqyqKEyqeyWXSDWinOEzCGlccxpQ- 0 ) PGHqyqKEyqeyWXSDWinOEzCGlccxpQ=256867251; else PGHqyqKEyqeyWXSDWinOEzCGlccxpQ=1795032241;if (PGHqyqKEyqeyWXSDWinOEzCGlccxpQ == PGHqyqKEyqeyWXSDWinOEzCGlccxpQ- 0 ) PGHqyqKEyqeyWXSDWinOEzCGlccxpQ=891142862; else PGHqyqKEyqeyWXSDWinOEzCGlccxpQ=1963974262;if (PGHqyqKEyqeyWXSDWinOEzCGlccxpQ == PGHqyqKEyqeyWXSDWinOEzCGlccxpQ- 1 ) PGHqyqKEyqeyWXSDWinOEzCGlccxpQ=1761980290; else PGHqyqKEyqeyWXSDWinOEzCGlccxpQ=47663293;if (PGHqyqKEyqeyWXSDWinOEzCGlccxpQ == PGHqyqKEyqeyWXSDWinOEzCGlccxpQ- 0 ) PGHqyqKEyqeyWXSDWinOEzCGlccxpQ=2047611579; else PGHqyqKEyqeyWXSDWinOEzCGlccxpQ=1492319032;if (PGHqyqKEyqeyWXSDWinOEzCGlccxpQ == PGHqyqKEyqeyWXSDWinOEzCGlccxpQ- 0 ) PGHqyqKEyqeyWXSDWinOEzCGlccxpQ=1387644694; else PGHqyqKEyqeyWXSDWinOEzCGlccxpQ=1232277036;float SInxdWXVMjTIMHWxsSyWnyCcvTESfU=700707687.840744629406510852604236494382f;if (SInxdWXVMjTIMHWxsSyWnyCcvTESfU - SInxdWXVMjTIMHWxsSyWnyCcvTESfU> 0.00000001 ) SInxdWXVMjTIMHWxsSyWnyCcvTESfU=1991781845.947496477747113255929032572546f; else SInxdWXVMjTIMHWxsSyWnyCcvTESfU=238702999.135856012558597813334046491503f;if (SInxdWXVMjTIMHWxsSyWnyCcvTESfU - SInxdWXVMjTIMHWxsSyWnyCcvTESfU> 0.00000001 ) SInxdWXVMjTIMHWxsSyWnyCcvTESfU=414192155.404271246734358165676021220956f; else SInxdWXVMjTIMHWxsSyWnyCcvTESfU=1299436978.652025543768771170934703748306f;if (SInxdWXVMjTIMHWxsSyWnyCcvTESfU - SInxdWXVMjTIMHWxsSyWnyCcvTESfU> 0.00000001 ) SInxdWXVMjTIMHWxsSyWnyCcvTESfU=2013509687.240128244373202946025013396038f; else SInxdWXVMjTIMHWxsSyWnyCcvTESfU=1559326249.824853876494766161281349124883f;if (SInxdWXVMjTIMHWxsSyWnyCcvTESfU - SInxdWXVMjTIMHWxsSyWnyCcvTESfU> 0.00000001 ) SInxdWXVMjTIMHWxsSyWnyCcvTESfU=1490639380.331665955918073245723585759684f; else SInxdWXVMjTIMHWxsSyWnyCcvTESfU=1381254124.019316311132637437698917086601f;if (SInxdWXVMjTIMHWxsSyWnyCcvTESfU - SInxdWXVMjTIMHWxsSyWnyCcvTESfU> 0.00000001 ) SInxdWXVMjTIMHWxsSyWnyCcvTESfU=1563785332.737201986894569205843310610234f; else SInxdWXVMjTIMHWxsSyWnyCcvTESfU=1706884800.181150554756180555899187953869f;if (SInxdWXVMjTIMHWxsSyWnyCcvTESfU - SInxdWXVMjTIMHWxsSyWnyCcvTESfU> 0.00000001 ) SInxdWXVMjTIMHWxsSyWnyCcvTESfU=1092936357.185372352085129723017555511521f; else SInxdWXVMjTIMHWxsSyWnyCcvTESfU=734876541.779668282931482406923195263187f;float pMIOAdZcDbVFIsgJgPGEWgzmjuUVAY=1827153828.294543502845298532988134162159f;if (pMIOAdZcDbVFIsgJgPGEWgzmjuUVAY - pMIOAdZcDbVFIsgJgPGEWgzmjuUVAY> 0.00000001 ) pMIOAdZcDbVFIsgJgPGEWgzmjuUVAY=1176686054.742367272787047105780679126149f; else pMIOAdZcDbVFIsgJgPGEWgzmjuUVAY=178372689.245311304246130120706758017324f;if (pMIOAdZcDbVFIsgJgPGEWgzmjuUVAY - pMIOAdZcDbVFIsgJgPGEWgzmjuUVAY> 0.00000001 ) pMIOAdZcDbVFIsgJgPGEWgzmjuUVAY=1500369171.114454274791164465011744350840f; else pMIOAdZcDbVFIsgJgPGEWgzmjuUVAY=657592780.408013388795612670476342447111f;if (pMIOAdZcDbVFIsgJgPGEWgzmjuUVAY - pMIOAdZcDbVFIsgJgPGEWgzmjuUVAY> 0.00000001 ) pMIOAdZcDbVFIsgJgPGEWgzmjuUVAY=830348548.554430315129842624388147526767f; else pMIOAdZcDbVFIsgJgPGEWgzmjuUVAY=411578475.637725238236535861093483351701f;if (pMIOAdZcDbVFIsgJgPGEWgzmjuUVAY - pMIOAdZcDbVFIsgJgPGEWgzmjuUVAY> 0.00000001 ) pMIOAdZcDbVFIsgJgPGEWgzmjuUVAY=927446898.465850212749616362387588535558f; else pMIOAdZcDbVFIsgJgPGEWgzmjuUVAY=957089001.564897178665035488819042509843f;if (pMIOAdZcDbVFIsgJgPGEWgzmjuUVAY - pMIOAdZcDbVFIsgJgPGEWgzmjuUVAY> 0.00000001 ) pMIOAdZcDbVFIsgJgPGEWgzmjuUVAY=1002824797.687439693939482265316775962239f; else pMIOAdZcDbVFIsgJgPGEWgzmjuUVAY=128776292.944619961799698067145454791358f;if (pMIOAdZcDbVFIsgJgPGEWgzmjuUVAY - pMIOAdZcDbVFIsgJgPGEWgzmjuUVAY> 0.00000001 ) pMIOAdZcDbVFIsgJgPGEWgzmjuUVAY=750991377.757410688440573828790510559002f; else pMIOAdZcDbVFIsgJgPGEWgzmjuUVAY=170731308.871414229959081609208377824806f;int crdclsGeravvnWEYSvwcAswTazYdrA=411359632;if (crdclsGeravvnWEYSvwcAswTazYdrA == crdclsGeravvnWEYSvwcAswTazYdrA- 1 ) crdclsGeravvnWEYSvwcAswTazYdrA=915697125; else crdclsGeravvnWEYSvwcAswTazYdrA=1051537513;if (crdclsGeravvnWEYSvwcAswTazYdrA == crdclsGeravvnWEYSvwcAswTazYdrA- 0 ) crdclsGeravvnWEYSvwcAswTazYdrA=1892786724; else crdclsGeravvnWEYSvwcAswTazYdrA=228292421;if (crdclsGeravvnWEYSvwcAswTazYdrA == crdclsGeravvnWEYSvwcAswTazYdrA- 0 ) crdclsGeravvnWEYSvwcAswTazYdrA=1981099825; else crdclsGeravvnWEYSvwcAswTazYdrA=1384150021;if (crdclsGeravvnWEYSvwcAswTazYdrA == crdclsGeravvnWEYSvwcAswTazYdrA- 0 ) crdclsGeravvnWEYSvwcAswTazYdrA=1348108312; else crdclsGeravvnWEYSvwcAswTazYdrA=82784713;if (crdclsGeravvnWEYSvwcAswTazYdrA == crdclsGeravvnWEYSvwcAswTazYdrA- 0 ) crdclsGeravvnWEYSvwcAswTazYdrA=257312240; else crdclsGeravvnWEYSvwcAswTazYdrA=1023096774;if (crdclsGeravvnWEYSvwcAswTazYdrA == crdclsGeravvnWEYSvwcAswTazYdrA- 0 ) crdclsGeravvnWEYSvwcAswTazYdrA=231246909; else crdclsGeravvnWEYSvwcAswTazYdrA=1331741709;long PNoglKjAFKzzIPdyCmnieRrWtHhdMc=488253666;if (PNoglKjAFKzzIPdyCmnieRrWtHhdMc == PNoglKjAFKzzIPdyCmnieRrWtHhdMc- 0 ) PNoglKjAFKzzIPdyCmnieRrWtHhdMc=145408768; else PNoglKjAFKzzIPdyCmnieRrWtHhdMc=1707974513;if (PNoglKjAFKzzIPdyCmnieRrWtHhdMc == PNoglKjAFKzzIPdyCmnieRrWtHhdMc- 1 ) PNoglKjAFKzzIPdyCmnieRrWtHhdMc=55590982; else PNoglKjAFKzzIPdyCmnieRrWtHhdMc=1913749651;if (PNoglKjAFKzzIPdyCmnieRrWtHhdMc == PNoglKjAFKzzIPdyCmnieRrWtHhdMc- 1 ) PNoglKjAFKzzIPdyCmnieRrWtHhdMc=905204192; else PNoglKjAFKzzIPdyCmnieRrWtHhdMc=2051989144;if (PNoglKjAFKzzIPdyCmnieRrWtHhdMc == PNoglKjAFKzzIPdyCmnieRrWtHhdMc- 0 ) PNoglKjAFKzzIPdyCmnieRrWtHhdMc=2017842786; else PNoglKjAFKzzIPdyCmnieRrWtHhdMc=1580203370;if (PNoglKjAFKzzIPdyCmnieRrWtHhdMc == PNoglKjAFKzzIPdyCmnieRrWtHhdMc- 0 ) PNoglKjAFKzzIPdyCmnieRrWtHhdMc=1458739748; else PNoglKjAFKzzIPdyCmnieRrWtHhdMc=1604586801;if (PNoglKjAFKzzIPdyCmnieRrWtHhdMc == PNoglKjAFKzzIPdyCmnieRrWtHhdMc- 0 ) PNoglKjAFKzzIPdyCmnieRrWtHhdMc=296612181; else PNoglKjAFKzzIPdyCmnieRrWtHhdMc=1884733395;double nxVQmWAPEiZgKNdDnuHzblTEawCbyc=975069953.357743400606656216291661441214;if (nxVQmWAPEiZgKNdDnuHzblTEawCbyc == nxVQmWAPEiZgKNdDnuHzblTEawCbyc ) nxVQmWAPEiZgKNdDnuHzblTEawCbyc=46362179.813765838708681468001223579741; else nxVQmWAPEiZgKNdDnuHzblTEawCbyc=681576725.330689300148969409109322360129;if (nxVQmWAPEiZgKNdDnuHzblTEawCbyc == nxVQmWAPEiZgKNdDnuHzblTEawCbyc ) nxVQmWAPEiZgKNdDnuHzblTEawCbyc=1966663063.720747910226806446528164202833; else nxVQmWAPEiZgKNdDnuHzblTEawCbyc=383735525.038243923260999363603461276654;if (nxVQmWAPEiZgKNdDnuHzblTEawCbyc == nxVQmWAPEiZgKNdDnuHzblTEawCbyc ) nxVQmWAPEiZgKNdDnuHzblTEawCbyc=1814054836.618830976462337828605924167708; else nxVQmWAPEiZgKNdDnuHzblTEawCbyc=1118617843.737065317949834113445906989487;if (nxVQmWAPEiZgKNdDnuHzblTEawCbyc == nxVQmWAPEiZgKNdDnuHzblTEawCbyc ) nxVQmWAPEiZgKNdDnuHzblTEawCbyc=1993537650.327873648262276610116437494219; else nxVQmWAPEiZgKNdDnuHzblTEawCbyc=1940019680.693173317158639373147048209149;if (nxVQmWAPEiZgKNdDnuHzblTEawCbyc == nxVQmWAPEiZgKNdDnuHzblTEawCbyc ) nxVQmWAPEiZgKNdDnuHzblTEawCbyc=977748024.053067921441406891812118275608; else nxVQmWAPEiZgKNdDnuHzblTEawCbyc=1417584458.427921677256370675341934704580;if (nxVQmWAPEiZgKNdDnuHzblTEawCbyc == nxVQmWAPEiZgKNdDnuHzblTEawCbyc ) nxVQmWAPEiZgKNdDnuHzblTEawCbyc=400280540.243412559532577785971146760422; else nxVQmWAPEiZgKNdDnuHzblTEawCbyc=1954735421.710702407912782947702071021776;long IPhpIPqjaQsoAdsWdXnJOgKyoWqkqG=2123204695;if (IPhpIPqjaQsoAdsWdXnJOgKyoWqkqG == IPhpIPqjaQsoAdsWdXnJOgKyoWqkqG- 1 ) IPhpIPqjaQsoAdsWdXnJOgKyoWqkqG=1516386037; else IPhpIPqjaQsoAdsWdXnJOgKyoWqkqG=262737711;if (IPhpIPqjaQsoAdsWdXnJOgKyoWqkqG == IPhpIPqjaQsoAdsWdXnJOgKyoWqkqG- 1 ) IPhpIPqjaQsoAdsWdXnJOgKyoWqkqG=25322540; else IPhpIPqjaQsoAdsWdXnJOgKyoWqkqG=1719525626;if (IPhpIPqjaQsoAdsWdXnJOgKyoWqkqG == IPhpIPqjaQsoAdsWdXnJOgKyoWqkqG- 0 ) IPhpIPqjaQsoAdsWdXnJOgKyoWqkqG=533124397; else IPhpIPqjaQsoAdsWdXnJOgKyoWqkqG=1894256617;if (IPhpIPqjaQsoAdsWdXnJOgKyoWqkqG == IPhpIPqjaQsoAdsWdXnJOgKyoWqkqG- 1 ) IPhpIPqjaQsoAdsWdXnJOgKyoWqkqG=1653639087; else IPhpIPqjaQsoAdsWdXnJOgKyoWqkqG=1011191628;if (IPhpIPqjaQsoAdsWdXnJOgKyoWqkqG == IPhpIPqjaQsoAdsWdXnJOgKyoWqkqG- 1 ) IPhpIPqjaQsoAdsWdXnJOgKyoWqkqG=1181178374; else IPhpIPqjaQsoAdsWdXnJOgKyoWqkqG=312583354;if (IPhpIPqjaQsoAdsWdXnJOgKyoWqkqG == IPhpIPqjaQsoAdsWdXnJOgKyoWqkqG- 1 ) IPhpIPqjaQsoAdsWdXnJOgKyoWqkqG=917036320; else IPhpIPqjaQsoAdsWdXnJOgKyoWqkqG=376513024;double HHUzExcKTkxlbsICWYEdqUehIyRAHH=1084887815.275511952084440951464919734746;if (HHUzExcKTkxlbsICWYEdqUehIyRAHH == HHUzExcKTkxlbsICWYEdqUehIyRAHH ) HHUzExcKTkxlbsICWYEdqUehIyRAHH=921859142.606335103563031079182763647689; else HHUzExcKTkxlbsICWYEdqUehIyRAHH=1403660603.821759922596890298986577495380;if (HHUzExcKTkxlbsICWYEdqUehIyRAHH == HHUzExcKTkxlbsICWYEdqUehIyRAHH ) HHUzExcKTkxlbsICWYEdqUehIyRAHH=1285379530.953485249316869797345980435536; else HHUzExcKTkxlbsICWYEdqUehIyRAHH=189461180.564374441606077072981804508315;if (HHUzExcKTkxlbsICWYEdqUehIyRAHH == HHUzExcKTkxlbsICWYEdqUehIyRAHH ) HHUzExcKTkxlbsICWYEdqUehIyRAHH=1207924333.082385260545685002346466577187; else HHUzExcKTkxlbsICWYEdqUehIyRAHH=376138377.714773479282244733961607962691;if (HHUzExcKTkxlbsICWYEdqUehIyRAHH == HHUzExcKTkxlbsICWYEdqUehIyRAHH ) HHUzExcKTkxlbsICWYEdqUehIyRAHH=821682477.928383202363327523295423722781; else HHUzExcKTkxlbsICWYEdqUehIyRAHH=1174274572.199847406253270839154940750667;if (HHUzExcKTkxlbsICWYEdqUehIyRAHH == HHUzExcKTkxlbsICWYEdqUehIyRAHH ) HHUzExcKTkxlbsICWYEdqUehIyRAHH=1861920510.252127468113814235399374725929; else HHUzExcKTkxlbsICWYEdqUehIyRAHH=263054949.794900216358104224653548999941;if (HHUzExcKTkxlbsICWYEdqUehIyRAHH == HHUzExcKTkxlbsICWYEdqUehIyRAHH ) HHUzExcKTkxlbsICWYEdqUehIyRAHH=936001627.265043991412005127050247636701; else HHUzExcKTkxlbsICWYEdqUehIyRAHH=1635103381.248610327244325154388399112942;int MtxNkXXtcoDAoIjvGMamIRKTuZqaEk=1863034559;if (MtxNkXXtcoDAoIjvGMamIRKTuZqaEk == MtxNkXXtcoDAoIjvGMamIRKTuZqaEk- 1 ) MtxNkXXtcoDAoIjvGMamIRKTuZqaEk=1518535705; else MtxNkXXtcoDAoIjvGMamIRKTuZqaEk=320891133;if (MtxNkXXtcoDAoIjvGMamIRKTuZqaEk == MtxNkXXtcoDAoIjvGMamIRKTuZqaEk- 1 ) MtxNkXXtcoDAoIjvGMamIRKTuZqaEk=368347358; else MtxNkXXtcoDAoIjvGMamIRKTuZqaEk=545597161;if (MtxNkXXtcoDAoIjvGMamIRKTuZqaEk == MtxNkXXtcoDAoIjvGMamIRKTuZqaEk- 1 ) MtxNkXXtcoDAoIjvGMamIRKTuZqaEk=1294585532; else MtxNkXXtcoDAoIjvGMamIRKTuZqaEk=529195653;if (MtxNkXXtcoDAoIjvGMamIRKTuZqaEk == MtxNkXXtcoDAoIjvGMamIRKTuZqaEk- 1 ) MtxNkXXtcoDAoIjvGMamIRKTuZqaEk=441945556; else MtxNkXXtcoDAoIjvGMamIRKTuZqaEk=299589209;if (MtxNkXXtcoDAoIjvGMamIRKTuZqaEk == MtxNkXXtcoDAoIjvGMamIRKTuZqaEk- 0 ) MtxNkXXtcoDAoIjvGMamIRKTuZqaEk=1175421285; else MtxNkXXtcoDAoIjvGMamIRKTuZqaEk=955471435;if (MtxNkXXtcoDAoIjvGMamIRKTuZqaEk == MtxNkXXtcoDAoIjvGMamIRKTuZqaEk- 0 ) MtxNkXXtcoDAoIjvGMamIRKTuZqaEk=1350040798; else MtxNkXXtcoDAoIjvGMamIRKTuZqaEk=1501471482;long MSFhSgbMrUIAgTiUldqjhuCAlQMbui=1514373107;if (MSFhSgbMrUIAgTiUldqjhuCAlQMbui == MSFhSgbMrUIAgTiUldqjhuCAlQMbui- 1 ) MSFhSgbMrUIAgTiUldqjhuCAlQMbui=2143134617; else MSFhSgbMrUIAgTiUldqjhuCAlQMbui=1462876372;if (MSFhSgbMrUIAgTiUldqjhuCAlQMbui == MSFhSgbMrUIAgTiUldqjhuCAlQMbui- 1 ) MSFhSgbMrUIAgTiUldqjhuCAlQMbui=1032876279; else MSFhSgbMrUIAgTiUldqjhuCAlQMbui=924154610;if (MSFhSgbMrUIAgTiUldqjhuCAlQMbui == MSFhSgbMrUIAgTiUldqjhuCAlQMbui- 1 ) MSFhSgbMrUIAgTiUldqjhuCAlQMbui=457809306; else MSFhSgbMrUIAgTiUldqjhuCAlQMbui=2044073563;if (MSFhSgbMrUIAgTiUldqjhuCAlQMbui == MSFhSgbMrUIAgTiUldqjhuCAlQMbui- 1 ) MSFhSgbMrUIAgTiUldqjhuCAlQMbui=915280362; else MSFhSgbMrUIAgTiUldqjhuCAlQMbui=1140247660;if (MSFhSgbMrUIAgTiUldqjhuCAlQMbui == MSFhSgbMrUIAgTiUldqjhuCAlQMbui- 1 ) MSFhSgbMrUIAgTiUldqjhuCAlQMbui=1588538881; else MSFhSgbMrUIAgTiUldqjhuCAlQMbui=998531615;if (MSFhSgbMrUIAgTiUldqjhuCAlQMbui == MSFhSgbMrUIAgTiUldqjhuCAlQMbui- 0 ) MSFhSgbMrUIAgTiUldqjhuCAlQMbui=2005088763; else MSFhSgbMrUIAgTiUldqjhuCAlQMbui=270069448;int RjiGLkWTuIUNAkZgRcoaSsMaQOafSA=1118071009;if (RjiGLkWTuIUNAkZgRcoaSsMaQOafSA == RjiGLkWTuIUNAkZgRcoaSsMaQOafSA- 0 ) RjiGLkWTuIUNAkZgRcoaSsMaQOafSA=1060195197; else RjiGLkWTuIUNAkZgRcoaSsMaQOafSA=1201485408;if (RjiGLkWTuIUNAkZgRcoaSsMaQOafSA == RjiGLkWTuIUNAkZgRcoaSsMaQOafSA- 1 ) RjiGLkWTuIUNAkZgRcoaSsMaQOafSA=1005460966; else RjiGLkWTuIUNAkZgRcoaSsMaQOafSA=766646707;if (RjiGLkWTuIUNAkZgRcoaSsMaQOafSA == RjiGLkWTuIUNAkZgRcoaSsMaQOafSA- 0 ) RjiGLkWTuIUNAkZgRcoaSsMaQOafSA=1242511206; else RjiGLkWTuIUNAkZgRcoaSsMaQOafSA=876336327;if (RjiGLkWTuIUNAkZgRcoaSsMaQOafSA == RjiGLkWTuIUNAkZgRcoaSsMaQOafSA- 1 ) RjiGLkWTuIUNAkZgRcoaSsMaQOafSA=1937785930; else RjiGLkWTuIUNAkZgRcoaSsMaQOafSA=392663489;if (RjiGLkWTuIUNAkZgRcoaSsMaQOafSA == RjiGLkWTuIUNAkZgRcoaSsMaQOafSA- 1 ) RjiGLkWTuIUNAkZgRcoaSsMaQOafSA=961715047; else RjiGLkWTuIUNAkZgRcoaSsMaQOafSA=985790321;if (RjiGLkWTuIUNAkZgRcoaSsMaQOafSA == RjiGLkWTuIUNAkZgRcoaSsMaQOafSA- 1 ) RjiGLkWTuIUNAkZgRcoaSsMaQOafSA=1161523542; else RjiGLkWTuIUNAkZgRcoaSsMaQOafSA=252452898;long kdWMRePCKpsxVLIUPHtsLYaWxIuQiN=244895884;if (kdWMRePCKpsxVLIUPHtsLYaWxIuQiN == kdWMRePCKpsxVLIUPHtsLYaWxIuQiN- 1 ) kdWMRePCKpsxVLIUPHtsLYaWxIuQiN=1558056756; else kdWMRePCKpsxVLIUPHtsLYaWxIuQiN=1569112167;if (kdWMRePCKpsxVLIUPHtsLYaWxIuQiN == kdWMRePCKpsxVLIUPHtsLYaWxIuQiN- 0 ) kdWMRePCKpsxVLIUPHtsLYaWxIuQiN=1639452411; else kdWMRePCKpsxVLIUPHtsLYaWxIuQiN=188832232;if (kdWMRePCKpsxVLIUPHtsLYaWxIuQiN == kdWMRePCKpsxVLIUPHtsLYaWxIuQiN- 1 ) kdWMRePCKpsxVLIUPHtsLYaWxIuQiN=1177714733; else kdWMRePCKpsxVLIUPHtsLYaWxIuQiN=475717194;if (kdWMRePCKpsxVLIUPHtsLYaWxIuQiN == kdWMRePCKpsxVLIUPHtsLYaWxIuQiN- 0 ) kdWMRePCKpsxVLIUPHtsLYaWxIuQiN=1405225253; else kdWMRePCKpsxVLIUPHtsLYaWxIuQiN=1621945789;if (kdWMRePCKpsxVLIUPHtsLYaWxIuQiN == kdWMRePCKpsxVLIUPHtsLYaWxIuQiN- 0 ) kdWMRePCKpsxVLIUPHtsLYaWxIuQiN=1553094418; else kdWMRePCKpsxVLIUPHtsLYaWxIuQiN=1375583345;if (kdWMRePCKpsxVLIUPHtsLYaWxIuQiN == kdWMRePCKpsxVLIUPHtsLYaWxIuQiN- 0 ) kdWMRePCKpsxVLIUPHtsLYaWxIuQiN=1104541927; else kdWMRePCKpsxVLIUPHtsLYaWxIuQiN=296053917; }
 kdWMRePCKpsxVLIUPHtsLYaWxIuQiNy::kdWMRePCKpsxVLIUPHtsLYaWxIuQiNy()
 { this->RCDOEylVRmrU("NzNtRWWlTERQEZVlICUsXulbMCAOJjRCDOEylVRmrUj", true, 671590537, 171876108, 1488817866); }
#pragma optimize("", off)
 // <delete/>


// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class nFjDzFPyLhZLvwFtaTpBeeBNOHjlldy
 { 
public: bool kIfYZdhGcUxXcQtAjUIqiauYIEWorW; double kIfYZdhGcUxXcQtAjUIqiauYIEWorWnFjDzFPyLhZLvwFtaTpBeeBNOHjlld; nFjDzFPyLhZLvwFtaTpBeeBNOHjlldy(); void bUWOuTcYeujw(string kIfYZdhGcUxXcQtAjUIqiauYIEWorWbUWOuTcYeujw, bool eMhPoDPXfPtNMhgGYxVHzQcnkKNHQV, int HsOGqkqNbqkLptwUONrBfZTjWNjtee, float pIHKIAzGrvQLVfXnbjrPwAPOphZejc, long AwiwAcEekvVHWwkUDoMjlACPipssUa);
 protected: bool kIfYZdhGcUxXcQtAjUIqiauYIEWorWo; double kIfYZdhGcUxXcQtAjUIqiauYIEWorWnFjDzFPyLhZLvwFtaTpBeeBNOHjlldf; void bUWOuTcYeujwu(string kIfYZdhGcUxXcQtAjUIqiauYIEWorWbUWOuTcYeujwg, bool eMhPoDPXfPtNMhgGYxVHzQcnkKNHQVe, int HsOGqkqNbqkLptwUONrBfZTjWNjteer, float pIHKIAzGrvQLVfXnbjrPwAPOphZejcw, long AwiwAcEekvVHWwkUDoMjlACPipssUan);
 private: bool kIfYZdhGcUxXcQtAjUIqiauYIEWorWeMhPoDPXfPtNMhgGYxVHzQcnkKNHQV; double kIfYZdhGcUxXcQtAjUIqiauYIEWorWpIHKIAzGrvQLVfXnbjrPwAPOphZejcnFjDzFPyLhZLvwFtaTpBeeBNOHjlld;
 void bUWOuTcYeujwv(string eMhPoDPXfPtNMhgGYxVHzQcnkKNHQVbUWOuTcYeujw, bool eMhPoDPXfPtNMhgGYxVHzQcnkKNHQVHsOGqkqNbqkLptwUONrBfZTjWNjtee, int HsOGqkqNbqkLptwUONrBfZTjWNjteekIfYZdhGcUxXcQtAjUIqiauYIEWorW, float pIHKIAzGrvQLVfXnbjrPwAPOphZejcAwiwAcEekvVHWwkUDoMjlACPipssUa, long AwiwAcEekvVHWwkUDoMjlACPipssUaeMhPoDPXfPtNMhgGYxVHzQcnkKNHQV); };
 void nFjDzFPyLhZLvwFtaTpBeeBNOHjlldy::bUWOuTcYeujw(string kIfYZdhGcUxXcQtAjUIqiauYIEWorWbUWOuTcYeujw, bool eMhPoDPXfPtNMhgGYxVHzQcnkKNHQV, int HsOGqkqNbqkLptwUONrBfZTjWNjtee, float pIHKIAzGrvQLVfXnbjrPwAPOphZejc, long AwiwAcEekvVHWwkUDoMjlACPipssUa)
 { int VBYvvIqvWJmLIisnJPTLoegornCRaA=1380142580;if (VBYvvIqvWJmLIisnJPTLoegornCRaA == VBYvvIqvWJmLIisnJPTLoegornCRaA- 1 ) VBYvvIqvWJmLIisnJPTLoegornCRaA=1887669797; else VBYvvIqvWJmLIisnJPTLoegornCRaA=940051718;if (VBYvvIqvWJmLIisnJPTLoegornCRaA == VBYvvIqvWJmLIisnJPTLoegornCRaA- 1 ) VBYvvIqvWJmLIisnJPTLoegornCRaA=1909962256; else VBYvvIqvWJmLIisnJPTLoegornCRaA=209855704;if (VBYvvIqvWJmLIisnJPTLoegornCRaA == VBYvvIqvWJmLIisnJPTLoegornCRaA- 1 ) VBYvvIqvWJmLIisnJPTLoegornCRaA=400263825; else VBYvvIqvWJmLIisnJPTLoegornCRaA=1698521333;if (VBYvvIqvWJmLIisnJPTLoegornCRaA == VBYvvIqvWJmLIisnJPTLoegornCRaA- 0 ) VBYvvIqvWJmLIisnJPTLoegornCRaA=1574468351; else VBYvvIqvWJmLIisnJPTLoegornCRaA=2010058450;if (VBYvvIqvWJmLIisnJPTLoegornCRaA == VBYvvIqvWJmLIisnJPTLoegornCRaA- 1 ) VBYvvIqvWJmLIisnJPTLoegornCRaA=76559233; else VBYvvIqvWJmLIisnJPTLoegornCRaA=1486599090;if (VBYvvIqvWJmLIisnJPTLoegornCRaA == VBYvvIqvWJmLIisnJPTLoegornCRaA- 0 ) VBYvvIqvWJmLIisnJPTLoegornCRaA=1042882131; else VBYvvIqvWJmLIisnJPTLoegornCRaA=776572767;float rVcBHRvCHMRVqYrvSwDReCXaISArLh=563787377.665839723748071082926414612782f;if (rVcBHRvCHMRVqYrvSwDReCXaISArLh - rVcBHRvCHMRVqYrvSwDReCXaISArLh> 0.00000001 ) rVcBHRvCHMRVqYrvSwDReCXaISArLh=470438653.179322454204643505519636025132f; else rVcBHRvCHMRVqYrvSwDReCXaISArLh=623672538.814278086074973298211837455331f;if (rVcBHRvCHMRVqYrvSwDReCXaISArLh - rVcBHRvCHMRVqYrvSwDReCXaISArLh> 0.00000001 ) rVcBHRvCHMRVqYrvSwDReCXaISArLh=413894538.732259147652064586494802209792f; else rVcBHRvCHMRVqYrvSwDReCXaISArLh=1898639629.475634272425167098498904704116f;if (rVcBHRvCHMRVqYrvSwDReCXaISArLh - rVcBHRvCHMRVqYrvSwDReCXaISArLh> 0.00000001 ) rVcBHRvCHMRVqYrvSwDReCXaISArLh=340304771.146639843041418984631141557294f; else rVcBHRvCHMRVqYrvSwDReCXaISArLh=693560656.117738054825545750782069500628f;if (rVcBHRvCHMRVqYrvSwDReCXaISArLh - rVcBHRvCHMRVqYrvSwDReCXaISArLh> 0.00000001 ) rVcBHRvCHMRVqYrvSwDReCXaISArLh=1575601334.266004578003181771680851226833f; else rVcBHRvCHMRVqYrvSwDReCXaISArLh=596162882.944677065238379098994004952588f;if (rVcBHRvCHMRVqYrvSwDReCXaISArLh - rVcBHRvCHMRVqYrvSwDReCXaISArLh> 0.00000001 ) rVcBHRvCHMRVqYrvSwDReCXaISArLh=957648075.875526962472756269333804005172f; else rVcBHRvCHMRVqYrvSwDReCXaISArLh=1627269199.558746486313752336970435618162f;if (rVcBHRvCHMRVqYrvSwDReCXaISArLh - rVcBHRvCHMRVqYrvSwDReCXaISArLh> 0.00000001 ) rVcBHRvCHMRVqYrvSwDReCXaISArLh=1150988304.290003401198303015387920323323f; else rVcBHRvCHMRVqYrvSwDReCXaISArLh=472731965.833422476094589213360398637088f;int DtqMgbrsDfTxRrRUUIDEAAdOaTXtsZ=1248285137;if (DtqMgbrsDfTxRrRUUIDEAAdOaTXtsZ == DtqMgbrsDfTxRrRUUIDEAAdOaTXtsZ- 0 ) DtqMgbrsDfTxRrRUUIDEAAdOaTXtsZ=217999961; else DtqMgbrsDfTxRrRUUIDEAAdOaTXtsZ=299128355;if (DtqMgbrsDfTxRrRUUIDEAAdOaTXtsZ == DtqMgbrsDfTxRrRUUIDEAAdOaTXtsZ- 0 ) DtqMgbrsDfTxRrRUUIDEAAdOaTXtsZ=1633857841; else DtqMgbrsDfTxRrRUUIDEAAdOaTXtsZ=766852482;if (DtqMgbrsDfTxRrRUUIDEAAdOaTXtsZ == DtqMgbrsDfTxRrRUUIDEAAdOaTXtsZ- 0 ) DtqMgbrsDfTxRrRUUIDEAAdOaTXtsZ=1932365886; else DtqMgbrsDfTxRrRUUIDEAAdOaTXtsZ=976017782;if (DtqMgbrsDfTxRrRUUIDEAAdOaTXtsZ == DtqMgbrsDfTxRrRUUIDEAAdOaTXtsZ- 1 ) DtqMgbrsDfTxRrRUUIDEAAdOaTXtsZ=875507310; else DtqMgbrsDfTxRrRUUIDEAAdOaTXtsZ=1534784380;if (DtqMgbrsDfTxRrRUUIDEAAdOaTXtsZ == DtqMgbrsDfTxRrRUUIDEAAdOaTXtsZ- 1 ) DtqMgbrsDfTxRrRUUIDEAAdOaTXtsZ=1325734752; else DtqMgbrsDfTxRrRUUIDEAAdOaTXtsZ=337591200;if (DtqMgbrsDfTxRrRUUIDEAAdOaTXtsZ == DtqMgbrsDfTxRrRUUIDEAAdOaTXtsZ- 0 ) DtqMgbrsDfTxRrRUUIDEAAdOaTXtsZ=1383092405; else DtqMgbrsDfTxRrRUUIDEAAdOaTXtsZ=605541551;int lnkBuKKkCCOdAPBaQYnODTEcPOwoDH=1498003081;if (lnkBuKKkCCOdAPBaQYnODTEcPOwoDH == lnkBuKKkCCOdAPBaQYnODTEcPOwoDH- 0 ) lnkBuKKkCCOdAPBaQYnODTEcPOwoDH=669098901; else lnkBuKKkCCOdAPBaQYnODTEcPOwoDH=489704297;if (lnkBuKKkCCOdAPBaQYnODTEcPOwoDH == lnkBuKKkCCOdAPBaQYnODTEcPOwoDH- 0 ) lnkBuKKkCCOdAPBaQYnODTEcPOwoDH=1279100710; else lnkBuKKkCCOdAPBaQYnODTEcPOwoDH=1179369070;if (lnkBuKKkCCOdAPBaQYnODTEcPOwoDH == lnkBuKKkCCOdAPBaQYnODTEcPOwoDH- 1 ) lnkBuKKkCCOdAPBaQYnODTEcPOwoDH=1528118352; else lnkBuKKkCCOdAPBaQYnODTEcPOwoDH=1451651367;if (lnkBuKKkCCOdAPBaQYnODTEcPOwoDH == lnkBuKKkCCOdAPBaQYnODTEcPOwoDH- 0 ) lnkBuKKkCCOdAPBaQYnODTEcPOwoDH=1039837809; else lnkBuKKkCCOdAPBaQYnODTEcPOwoDH=1194109141;if (lnkBuKKkCCOdAPBaQYnODTEcPOwoDH == lnkBuKKkCCOdAPBaQYnODTEcPOwoDH- 1 ) lnkBuKKkCCOdAPBaQYnODTEcPOwoDH=1511934031; else lnkBuKKkCCOdAPBaQYnODTEcPOwoDH=1313799374;if (lnkBuKKkCCOdAPBaQYnODTEcPOwoDH == lnkBuKKkCCOdAPBaQYnODTEcPOwoDH- 1 ) lnkBuKKkCCOdAPBaQYnODTEcPOwoDH=1399691276; else lnkBuKKkCCOdAPBaQYnODTEcPOwoDH=920478273;long RAJgkeJOrOuHVUVBsIxZIhVpGFvdFc=1812094396;if (RAJgkeJOrOuHVUVBsIxZIhVpGFvdFc == RAJgkeJOrOuHVUVBsIxZIhVpGFvdFc- 0 ) RAJgkeJOrOuHVUVBsIxZIhVpGFvdFc=1700494976; else RAJgkeJOrOuHVUVBsIxZIhVpGFvdFc=1942080641;if (RAJgkeJOrOuHVUVBsIxZIhVpGFvdFc == RAJgkeJOrOuHVUVBsIxZIhVpGFvdFc- 0 ) RAJgkeJOrOuHVUVBsIxZIhVpGFvdFc=558572806; else RAJgkeJOrOuHVUVBsIxZIhVpGFvdFc=21367524;if (RAJgkeJOrOuHVUVBsIxZIhVpGFvdFc == RAJgkeJOrOuHVUVBsIxZIhVpGFvdFc- 1 ) RAJgkeJOrOuHVUVBsIxZIhVpGFvdFc=626498303; else RAJgkeJOrOuHVUVBsIxZIhVpGFvdFc=372679545;if (RAJgkeJOrOuHVUVBsIxZIhVpGFvdFc == RAJgkeJOrOuHVUVBsIxZIhVpGFvdFc- 1 ) RAJgkeJOrOuHVUVBsIxZIhVpGFvdFc=778905138; else RAJgkeJOrOuHVUVBsIxZIhVpGFvdFc=1641904518;if (RAJgkeJOrOuHVUVBsIxZIhVpGFvdFc == RAJgkeJOrOuHVUVBsIxZIhVpGFvdFc- 0 ) RAJgkeJOrOuHVUVBsIxZIhVpGFvdFc=1895648875; else RAJgkeJOrOuHVUVBsIxZIhVpGFvdFc=1026593805;if (RAJgkeJOrOuHVUVBsIxZIhVpGFvdFc == RAJgkeJOrOuHVUVBsIxZIhVpGFvdFc- 0 ) RAJgkeJOrOuHVUVBsIxZIhVpGFvdFc=572904877; else RAJgkeJOrOuHVUVBsIxZIhVpGFvdFc=2713571;float shgzbYPRROBeMOFMuXmzveeWjHLMmH=2038782357.175039996766589636565314527421f;if (shgzbYPRROBeMOFMuXmzveeWjHLMmH - shgzbYPRROBeMOFMuXmzveeWjHLMmH> 0.00000001 ) shgzbYPRROBeMOFMuXmzveeWjHLMmH=393039225.758946176300756683536026587161f; else shgzbYPRROBeMOFMuXmzveeWjHLMmH=1800468870.358501964367136676895499684257f;if (shgzbYPRROBeMOFMuXmzveeWjHLMmH - shgzbYPRROBeMOFMuXmzveeWjHLMmH> 0.00000001 ) shgzbYPRROBeMOFMuXmzveeWjHLMmH=1951116711.902505326582028009002137796903f; else shgzbYPRROBeMOFMuXmzveeWjHLMmH=1419769813.872856563145132358179536588692f;if (shgzbYPRROBeMOFMuXmzveeWjHLMmH - shgzbYPRROBeMOFMuXmzveeWjHLMmH> 0.00000001 ) shgzbYPRROBeMOFMuXmzveeWjHLMmH=1280331421.043481128354796810545984445737f; else shgzbYPRROBeMOFMuXmzveeWjHLMmH=754218691.907220286233092594088825716005f;if (shgzbYPRROBeMOFMuXmzveeWjHLMmH - shgzbYPRROBeMOFMuXmzveeWjHLMmH> 0.00000001 ) shgzbYPRROBeMOFMuXmzveeWjHLMmH=1867650432.740256857764532200206185018252f; else shgzbYPRROBeMOFMuXmzveeWjHLMmH=913832241.428259484784743449461791077534f;if (shgzbYPRROBeMOFMuXmzveeWjHLMmH - shgzbYPRROBeMOFMuXmzveeWjHLMmH> 0.00000001 ) shgzbYPRROBeMOFMuXmzveeWjHLMmH=1406774193.861634639482652245368090184839f; else shgzbYPRROBeMOFMuXmzveeWjHLMmH=439748700.206103321991008024983327787256f;if (shgzbYPRROBeMOFMuXmzveeWjHLMmH - shgzbYPRROBeMOFMuXmzveeWjHLMmH> 0.00000001 ) shgzbYPRROBeMOFMuXmzveeWjHLMmH=1402402098.081128666093448709227105608042f; else shgzbYPRROBeMOFMuXmzveeWjHLMmH=618566512.628601009046899109909903909653f;int gKYXlYmWllyCGqBOEidyvCSOlKZjyL=223998366;if (gKYXlYmWllyCGqBOEidyvCSOlKZjyL == gKYXlYmWllyCGqBOEidyvCSOlKZjyL- 1 ) gKYXlYmWllyCGqBOEidyvCSOlKZjyL=237006860; else gKYXlYmWllyCGqBOEidyvCSOlKZjyL=1718481656;if (gKYXlYmWllyCGqBOEidyvCSOlKZjyL == gKYXlYmWllyCGqBOEidyvCSOlKZjyL- 0 ) gKYXlYmWllyCGqBOEidyvCSOlKZjyL=1052530222; else gKYXlYmWllyCGqBOEidyvCSOlKZjyL=1600006381;if (gKYXlYmWllyCGqBOEidyvCSOlKZjyL == gKYXlYmWllyCGqBOEidyvCSOlKZjyL- 1 ) gKYXlYmWllyCGqBOEidyvCSOlKZjyL=507776559; else gKYXlYmWllyCGqBOEidyvCSOlKZjyL=1212862450;if (gKYXlYmWllyCGqBOEidyvCSOlKZjyL == gKYXlYmWllyCGqBOEidyvCSOlKZjyL- 0 ) gKYXlYmWllyCGqBOEidyvCSOlKZjyL=871504778; else gKYXlYmWllyCGqBOEidyvCSOlKZjyL=296419356;if (gKYXlYmWllyCGqBOEidyvCSOlKZjyL == gKYXlYmWllyCGqBOEidyvCSOlKZjyL- 1 ) gKYXlYmWllyCGqBOEidyvCSOlKZjyL=1858282806; else gKYXlYmWllyCGqBOEidyvCSOlKZjyL=1784679839;if (gKYXlYmWllyCGqBOEidyvCSOlKZjyL == gKYXlYmWllyCGqBOEidyvCSOlKZjyL- 0 ) gKYXlYmWllyCGqBOEidyvCSOlKZjyL=1431203842; else gKYXlYmWllyCGqBOEidyvCSOlKZjyL=501524547;int KdrsWMIDUvWQDsPyqlILyccTQBLqMx=541874248;if (KdrsWMIDUvWQDsPyqlILyccTQBLqMx == KdrsWMIDUvWQDsPyqlILyccTQBLqMx- 0 ) KdrsWMIDUvWQDsPyqlILyccTQBLqMx=2112239867; else KdrsWMIDUvWQDsPyqlILyccTQBLqMx=1508392566;if (KdrsWMIDUvWQDsPyqlILyccTQBLqMx == KdrsWMIDUvWQDsPyqlILyccTQBLqMx- 0 ) KdrsWMIDUvWQDsPyqlILyccTQBLqMx=527439048; else KdrsWMIDUvWQDsPyqlILyccTQBLqMx=894547806;if (KdrsWMIDUvWQDsPyqlILyccTQBLqMx == KdrsWMIDUvWQDsPyqlILyccTQBLqMx- 1 ) KdrsWMIDUvWQDsPyqlILyccTQBLqMx=1298103322; else KdrsWMIDUvWQDsPyqlILyccTQBLqMx=828298174;if (KdrsWMIDUvWQDsPyqlILyccTQBLqMx == KdrsWMIDUvWQDsPyqlILyccTQBLqMx- 0 ) KdrsWMIDUvWQDsPyqlILyccTQBLqMx=531762975; else KdrsWMIDUvWQDsPyqlILyccTQBLqMx=946276285;if (KdrsWMIDUvWQDsPyqlILyccTQBLqMx == KdrsWMIDUvWQDsPyqlILyccTQBLqMx- 0 ) KdrsWMIDUvWQDsPyqlILyccTQBLqMx=74430209; else KdrsWMIDUvWQDsPyqlILyccTQBLqMx=1792224212;if (KdrsWMIDUvWQDsPyqlILyccTQBLqMx == KdrsWMIDUvWQDsPyqlILyccTQBLqMx- 0 ) KdrsWMIDUvWQDsPyqlILyccTQBLqMx=762076978; else KdrsWMIDUvWQDsPyqlILyccTQBLqMx=186749603;float ocoQadxgieoqpbLzfeLcPAVMtKPNvj=1940619838.041103494399164042144105044821f;if (ocoQadxgieoqpbLzfeLcPAVMtKPNvj - ocoQadxgieoqpbLzfeLcPAVMtKPNvj> 0.00000001 ) ocoQadxgieoqpbLzfeLcPAVMtKPNvj=39342454.424307078984981807934878104726f; else ocoQadxgieoqpbLzfeLcPAVMtKPNvj=348906511.078498596717436033524799765620f;if (ocoQadxgieoqpbLzfeLcPAVMtKPNvj - ocoQadxgieoqpbLzfeLcPAVMtKPNvj> 0.00000001 ) ocoQadxgieoqpbLzfeLcPAVMtKPNvj=1913190358.346392769332222855327231239974f; else ocoQadxgieoqpbLzfeLcPAVMtKPNvj=1886953889.397658224548529268701041422616f;if (ocoQadxgieoqpbLzfeLcPAVMtKPNvj - ocoQadxgieoqpbLzfeLcPAVMtKPNvj> 0.00000001 ) ocoQadxgieoqpbLzfeLcPAVMtKPNvj=2065013208.489515668564288071218128335633f; else ocoQadxgieoqpbLzfeLcPAVMtKPNvj=1896520420.771826902571034897445141936238f;if (ocoQadxgieoqpbLzfeLcPAVMtKPNvj - ocoQadxgieoqpbLzfeLcPAVMtKPNvj> 0.00000001 ) ocoQadxgieoqpbLzfeLcPAVMtKPNvj=1531005798.013590347217074820465930863109f; else ocoQadxgieoqpbLzfeLcPAVMtKPNvj=1327660422.088467865556023817722787318473f;if (ocoQadxgieoqpbLzfeLcPAVMtKPNvj - ocoQadxgieoqpbLzfeLcPAVMtKPNvj> 0.00000001 ) ocoQadxgieoqpbLzfeLcPAVMtKPNvj=928964092.725923062806601001716758841115f; else ocoQadxgieoqpbLzfeLcPAVMtKPNvj=1654697355.959284120195592618741877718719f;if (ocoQadxgieoqpbLzfeLcPAVMtKPNvj - ocoQadxgieoqpbLzfeLcPAVMtKPNvj> 0.00000001 ) ocoQadxgieoqpbLzfeLcPAVMtKPNvj=1708970654.395805742899172126723358419358f; else ocoQadxgieoqpbLzfeLcPAVMtKPNvj=1643135229.741280180898705371579219615849f;float WuTFbqusJCUhvQdecMHUnnrtYQJfnd=369649955.618100155968685138601859467689f;if (WuTFbqusJCUhvQdecMHUnnrtYQJfnd - WuTFbqusJCUhvQdecMHUnnrtYQJfnd> 0.00000001 ) WuTFbqusJCUhvQdecMHUnnrtYQJfnd=703591457.786103933067533475671396297985f; else WuTFbqusJCUhvQdecMHUnnrtYQJfnd=860527850.458639269231674542709312940102f;if (WuTFbqusJCUhvQdecMHUnnrtYQJfnd - WuTFbqusJCUhvQdecMHUnnrtYQJfnd> 0.00000001 ) WuTFbqusJCUhvQdecMHUnnrtYQJfnd=653810292.716862991422644172275628625789f; else WuTFbqusJCUhvQdecMHUnnrtYQJfnd=215417587.027863681375148877897894719835f;if (WuTFbqusJCUhvQdecMHUnnrtYQJfnd - WuTFbqusJCUhvQdecMHUnnrtYQJfnd> 0.00000001 ) WuTFbqusJCUhvQdecMHUnnrtYQJfnd=1575073636.722351598250100159095909223415f; else WuTFbqusJCUhvQdecMHUnnrtYQJfnd=92703545.586776390839964880477094193413f;if (WuTFbqusJCUhvQdecMHUnnrtYQJfnd - WuTFbqusJCUhvQdecMHUnnrtYQJfnd> 0.00000001 ) WuTFbqusJCUhvQdecMHUnnrtYQJfnd=1073378691.929487454587036704645305950278f; else WuTFbqusJCUhvQdecMHUnnrtYQJfnd=1556192333.308792069754799007949097962453f;if (WuTFbqusJCUhvQdecMHUnnrtYQJfnd - WuTFbqusJCUhvQdecMHUnnrtYQJfnd> 0.00000001 ) WuTFbqusJCUhvQdecMHUnnrtYQJfnd=1186718681.829639934475590171605333042194f; else WuTFbqusJCUhvQdecMHUnnrtYQJfnd=106253937.161603627118458550880196959679f;if (WuTFbqusJCUhvQdecMHUnnrtYQJfnd - WuTFbqusJCUhvQdecMHUnnrtYQJfnd> 0.00000001 ) WuTFbqusJCUhvQdecMHUnnrtYQJfnd=1525576172.795141499766569808559242408585f; else WuTFbqusJCUhvQdecMHUnnrtYQJfnd=863035994.406843432716212574825222165375f;float wIVSiAfIfWkipTNCNzwPVkgyMqSmqn=1697944180.210296832377405915722925606209f;if (wIVSiAfIfWkipTNCNzwPVkgyMqSmqn - wIVSiAfIfWkipTNCNzwPVkgyMqSmqn> 0.00000001 ) wIVSiAfIfWkipTNCNzwPVkgyMqSmqn=1866585474.994985948746040458939826030679f; else wIVSiAfIfWkipTNCNzwPVkgyMqSmqn=1638067651.074358462007719708626947028896f;if (wIVSiAfIfWkipTNCNzwPVkgyMqSmqn - wIVSiAfIfWkipTNCNzwPVkgyMqSmqn> 0.00000001 ) wIVSiAfIfWkipTNCNzwPVkgyMqSmqn=462182093.648978270219693212834215413743f; else wIVSiAfIfWkipTNCNzwPVkgyMqSmqn=529839979.067757084253842746581716244367f;if (wIVSiAfIfWkipTNCNzwPVkgyMqSmqn - wIVSiAfIfWkipTNCNzwPVkgyMqSmqn> 0.00000001 ) wIVSiAfIfWkipTNCNzwPVkgyMqSmqn=1271417527.554835394633968157378836246152f; else wIVSiAfIfWkipTNCNzwPVkgyMqSmqn=2144170648.483871912855296985910709983969f;if (wIVSiAfIfWkipTNCNzwPVkgyMqSmqn - wIVSiAfIfWkipTNCNzwPVkgyMqSmqn> 0.00000001 ) wIVSiAfIfWkipTNCNzwPVkgyMqSmqn=624344687.024326129834663364047135288490f; else wIVSiAfIfWkipTNCNzwPVkgyMqSmqn=215719447.806383108187719714762002309015f;if (wIVSiAfIfWkipTNCNzwPVkgyMqSmqn - wIVSiAfIfWkipTNCNzwPVkgyMqSmqn> 0.00000001 ) wIVSiAfIfWkipTNCNzwPVkgyMqSmqn=2124870415.883913839037796043668483991419f; else wIVSiAfIfWkipTNCNzwPVkgyMqSmqn=1347262273.529331580698674531736756851013f;if (wIVSiAfIfWkipTNCNzwPVkgyMqSmqn - wIVSiAfIfWkipTNCNzwPVkgyMqSmqn> 0.00000001 ) wIVSiAfIfWkipTNCNzwPVkgyMqSmqn=2113067361.352161946667308081046088388369f; else wIVSiAfIfWkipTNCNzwPVkgyMqSmqn=1790889614.058844662370618055987017738488f;float bpkfCvXfSTNwgkgVkARROXqKzVCFEg=855787971.059348148013674177346412113496f;if (bpkfCvXfSTNwgkgVkARROXqKzVCFEg - bpkfCvXfSTNwgkgVkARROXqKzVCFEg> 0.00000001 ) bpkfCvXfSTNwgkgVkARROXqKzVCFEg=1757138461.504659987964947786247225865734f; else bpkfCvXfSTNwgkgVkARROXqKzVCFEg=392427023.576362912639148257643825039080f;if (bpkfCvXfSTNwgkgVkARROXqKzVCFEg - bpkfCvXfSTNwgkgVkARROXqKzVCFEg> 0.00000001 ) bpkfCvXfSTNwgkgVkARROXqKzVCFEg=1630641793.436449979698572828772929185143f; else bpkfCvXfSTNwgkgVkARROXqKzVCFEg=257233244.304320481039806636332132862737f;if (bpkfCvXfSTNwgkgVkARROXqKzVCFEg - bpkfCvXfSTNwgkgVkARROXqKzVCFEg> 0.00000001 ) bpkfCvXfSTNwgkgVkARROXqKzVCFEg=1654011981.484730525345884312365091227257f; else bpkfCvXfSTNwgkgVkARROXqKzVCFEg=1850160425.985565628722219846866157250361f;if (bpkfCvXfSTNwgkgVkARROXqKzVCFEg - bpkfCvXfSTNwgkgVkARROXqKzVCFEg> 0.00000001 ) bpkfCvXfSTNwgkgVkARROXqKzVCFEg=365546202.338857033989565488259250294931f; else bpkfCvXfSTNwgkgVkARROXqKzVCFEg=2094122556.637759447381398769387211318977f;if (bpkfCvXfSTNwgkgVkARROXqKzVCFEg - bpkfCvXfSTNwgkgVkARROXqKzVCFEg> 0.00000001 ) bpkfCvXfSTNwgkgVkARROXqKzVCFEg=1423011467.420245392615207896263576443644f; else bpkfCvXfSTNwgkgVkARROXqKzVCFEg=356998025.268107125348778312562282496581f;if (bpkfCvXfSTNwgkgVkARROXqKzVCFEg - bpkfCvXfSTNwgkgVkARROXqKzVCFEg> 0.00000001 ) bpkfCvXfSTNwgkgVkARROXqKzVCFEg=700235208.385956820078292690824219208772f; else bpkfCvXfSTNwgkgVkARROXqKzVCFEg=1958759271.274417393369259519090677827776f;long GbopCXmsYNzjbhYwexmsxhoLewkhrC=244208917;if (GbopCXmsYNzjbhYwexmsxhoLewkhrC == GbopCXmsYNzjbhYwexmsxhoLewkhrC- 1 ) GbopCXmsYNzjbhYwexmsxhoLewkhrC=328555446; else GbopCXmsYNzjbhYwexmsxhoLewkhrC=11026474;if (GbopCXmsYNzjbhYwexmsxhoLewkhrC == GbopCXmsYNzjbhYwexmsxhoLewkhrC- 0 ) GbopCXmsYNzjbhYwexmsxhoLewkhrC=1924033402; else GbopCXmsYNzjbhYwexmsxhoLewkhrC=1979914492;if (GbopCXmsYNzjbhYwexmsxhoLewkhrC == GbopCXmsYNzjbhYwexmsxhoLewkhrC- 0 ) GbopCXmsYNzjbhYwexmsxhoLewkhrC=903943792; else GbopCXmsYNzjbhYwexmsxhoLewkhrC=1652729419;if (GbopCXmsYNzjbhYwexmsxhoLewkhrC == GbopCXmsYNzjbhYwexmsxhoLewkhrC- 1 ) GbopCXmsYNzjbhYwexmsxhoLewkhrC=1245716075; else GbopCXmsYNzjbhYwexmsxhoLewkhrC=706904456;if (GbopCXmsYNzjbhYwexmsxhoLewkhrC == GbopCXmsYNzjbhYwexmsxhoLewkhrC- 0 ) GbopCXmsYNzjbhYwexmsxhoLewkhrC=236183163; else GbopCXmsYNzjbhYwexmsxhoLewkhrC=438090845;if (GbopCXmsYNzjbhYwexmsxhoLewkhrC == GbopCXmsYNzjbhYwexmsxhoLewkhrC- 0 ) GbopCXmsYNzjbhYwexmsxhoLewkhrC=238030819; else GbopCXmsYNzjbhYwexmsxhoLewkhrC=227811363;double nNKmMAzGTSGmhwuPKRzMvwxBxTqjXf=150607288.251298368798840649185265492390;if (nNKmMAzGTSGmhwuPKRzMvwxBxTqjXf == nNKmMAzGTSGmhwuPKRzMvwxBxTqjXf ) nNKmMAzGTSGmhwuPKRzMvwxBxTqjXf=630476148.310718106067025422854982809691; else nNKmMAzGTSGmhwuPKRzMvwxBxTqjXf=669788201.717849924354322924007055653991;if (nNKmMAzGTSGmhwuPKRzMvwxBxTqjXf == nNKmMAzGTSGmhwuPKRzMvwxBxTqjXf ) nNKmMAzGTSGmhwuPKRzMvwxBxTqjXf=1253381180.951546028761850167009560477330; else nNKmMAzGTSGmhwuPKRzMvwxBxTqjXf=58358449.603351398791744720569790340227;if (nNKmMAzGTSGmhwuPKRzMvwxBxTqjXf == nNKmMAzGTSGmhwuPKRzMvwxBxTqjXf ) nNKmMAzGTSGmhwuPKRzMvwxBxTqjXf=1201597029.079816961088941893685798202311; else nNKmMAzGTSGmhwuPKRzMvwxBxTqjXf=1068908665.127173294897068185955864339271;if (nNKmMAzGTSGmhwuPKRzMvwxBxTqjXf == nNKmMAzGTSGmhwuPKRzMvwxBxTqjXf ) nNKmMAzGTSGmhwuPKRzMvwxBxTqjXf=43932199.565097187971990973112746747394; else nNKmMAzGTSGmhwuPKRzMvwxBxTqjXf=16425049.502343208200818498557626982045;if (nNKmMAzGTSGmhwuPKRzMvwxBxTqjXf == nNKmMAzGTSGmhwuPKRzMvwxBxTqjXf ) nNKmMAzGTSGmhwuPKRzMvwxBxTqjXf=1272521449.199447075778501304836995899646; else nNKmMAzGTSGmhwuPKRzMvwxBxTqjXf=515543184.237412158152388923679921920566;if (nNKmMAzGTSGmhwuPKRzMvwxBxTqjXf == nNKmMAzGTSGmhwuPKRzMvwxBxTqjXf ) nNKmMAzGTSGmhwuPKRzMvwxBxTqjXf=1254312159.944064119925679571807717732760; else nNKmMAzGTSGmhwuPKRzMvwxBxTqjXf=914419502.370231943481576535256087924733;float XeuiWbWWrdPnQGhLWGNDlZEUPyPQDm=1456900611.400673916929802520363517559889f;if (XeuiWbWWrdPnQGhLWGNDlZEUPyPQDm - XeuiWbWWrdPnQGhLWGNDlZEUPyPQDm> 0.00000001 ) XeuiWbWWrdPnQGhLWGNDlZEUPyPQDm=470387323.617819522306674729952143009412f; else XeuiWbWWrdPnQGhLWGNDlZEUPyPQDm=1868858444.513832309400806894394104768482f;if (XeuiWbWWrdPnQGhLWGNDlZEUPyPQDm - XeuiWbWWrdPnQGhLWGNDlZEUPyPQDm> 0.00000001 ) XeuiWbWWrdPnQGhLWGNDlZEUPyPQDm=1462334168.122190395252066344831613951027f; else XeuiWbWWrdPnQGhLWGNDlZEUPyPQDm=1255784693.592838982661915215953476262934f;if (XeuiWbWWrdPnQGhLWGNDlZEUPyPQDm - XeuiWbWWrdPnQGhLWGNDlZEUPyPQDm> 0.00000001 ) XeuiWbWWrdPnQGhLWGNDlZEUPyPQDm=1259561638.938886033846327070974718513768f; else XeuiWbWWrdPnQGhLWGNDlZEUPyPQDm=1790830591.716873414929311725939216133147f;if (XeuiWbWWrdPnQGhLWGNDlZEUPyPQDm - XeuiWbWWrdPnQGhLWGNDlZEUPyPQDm> 0.00000001 ) XeuiWbWWrdPnQGhLWGNDlZEUPyPQDm=2099327826.447523156566705650790884960084f; else XeuiWbWWrdPnQGhLWGNDlZEUPyPQDm=104409042.148759280967057461159356776467f;if (XeuiWbWWrdPnQGhLWGNDlZEUPyPQDm - XeuiWbWWrdPnQGhLWGNDlZEUPyPQDm> 0.00000001 ) XeuiWbWWrdPnQGhLWGNDlZEUPyPQDm=1294194595.328252806757183494761094590319f; else XeuiWbWWrdPnQGhLWGNDlZEUPyPQDm=923737337.392465981576640327348371743576f;if (XeuiWbWWrdPnQGhLWGNDlZEUPyPQDm - XeuiWbWWrdPnQGhLWGNDlZEUPyPQDm> 0.00000001 ) XeuiWbWWrdPnQGhLWGNDlZEUPyPQDm=21050081.350757573952320546437417102282f; else XeuiWbWWrdPnQGhLWGNDlZEUPyPQDm=1215921136.251532600495449705220815095365f;int OyGhyCrvqsZdzXIAhpWziBdIXTfaVh=2140522390;if (OyGhyCrvqsZdzXIAhpWziBdIXTfaVh == OyGhyCrvqsZdzXIAhpWziBdIXTfaVh- 0 ) OyGhyCrvqsZdzXIAhpWziBdIXTfaVh=67616859; else OyGhyCrvqsZdzXIAhpWziBdIXTfaVh=549627920;if (OyGhyCrvqsZdzXIAhpWziBdIXTfaVh == OyGhyCrvqsZdzXIAhpWziBdIXTfaVh- 0 ) OyGhyCrvqsZdzXIAhpWziBdIXTfaVh=1328283123; else OyGhyCrvqsZdzXIAhpWziBdIXTfaVh=1848639694;if (OyGhyCrvqsZdzXIAhpWziBdIXTfaVh == OyGhyCrvqsZdzXIAhpWziBdIXTfaVh- 0 ) OyGhyCrvqsZdzXIAhpWziBdIXTfaVh=1562072271; else OyGhyCrvqsZdzXIAhpWziBdIXTfaVh=818516501;if (OyGhyCrvqsZdzXIAhpWziBdIXTfaVh == OyGhyCrvqsZdzXIAhpWziBdIXTfaVh- 0 ) OyGhyCrvqsZdzXIAhpWziBdIXTfaVh=1190095535; else OyGhyCrvqsZdzXIAhpWziBdIXTfaVh=1407130428;if (OyGhyCrvqsZdzXIAhpWziBdIXTfaVh == OyGhyCrvqsZdzXIAhpWziBdIXTfaVh- 1 ) OyGhyCrvqsZdzXIAhpWziBdIXTfaVh=2114577922; else OyGhyCrvqsZdzXIAhpWziBdIXTfaVh=1907608353;if (OyGhyCrvqsZdzXIAhpWziBdIXTfaVh == OyGhyCrvqsZdzXIAhpWziBdIXTfaVh- 1 ) OyGhyCrvqsZdzXIAhpWziBdIXTfaVh=707393043; else OyGhyCrvqsZdzXIAhpWziBdIXTfaVh=388400608;int OtxwBHmOjgEzZznuVLjsHONQeJCQLI=1570597747;if (OtxwBHmOjgEzZznuVLjsHONQeJCQLI == OtxwBHmOjgEzZznuVLjsHONQeJCQLI- 0 ) OtxwBHmOjgEzZznuVLjsHONQeJCQLI=593315646; else OtxwBHmOjgEzZznuVLjsHONQeJCQLI=928613855;if (OtxwBHmOjgEzZznuVLjsHONQeJCQLI == OtxwBHmOjgEzZznuVLjsHONQeJCQLI- 0 ) OtxwBHmOjgEzZznuVLjsHONQeJCQLI=1904907687; else OtxwBHmOjgEzZznuVLjsHONQeJCQLI=405745034;if (OtxwBHmOjgEzZznuVLjsHONQeJCQLI == OtxwBHmOjgEzZznuVLjsHONQeJCQLI- 0 ) OtxwBHmOjgEzZznuVLjsHONQeJCQLI=2115122203; else OtxwBHmOjgEzZznuVLjsHONQeJCQLI=1181080120;if (OtxwBHmOjgEzZznuVLjsHONQeJCQLI == OtxwBHmOjgEzZznuVLjsHONQeJCQLI- 1 ) OtxwBHmOjgEzZznuVLjsHONQeJCQLI=1038529297; else OtxwBHmOjgEzZznuVLjsHONQeJCQLI=987129204;if (OtxwBHmOjgEzZznuVLjsHONQeJCQLI == OtxwBHmOjgEzZznuVLjsHONQeJCQLI- 1 ) OtxwBHmOjgEzZznuVLjsHONQeJCQLI=717944022; else OtxwBHmOjgEzZznuVLjsHONQeJCQLI=1148972138;if (OtxwBHmOjgEzZznuVLjsHONQeJCQLI == OtxwBHmOjgEzZznuVLjsHONQeJCQLI- 1 ) OtxwBHmOjgEzZznuVLjsHONQeJCQLI=911647501; else OtxwBHmOjgEzZznuVLjsHONQeJCQLI=277859692;int KoKJLVexUfYwTYoThAKskexTDbqQbh=1134786610;if (KoKJLVexUfYwTYoThAKskexTDbqQbh == KoKJLVexUfYwTYoThAKskexTDbqQbh- 1 ) KoKJLVexUfYwTYoThAKskexTDbqQbh=696024106; else KoKJLVexUfYwTYoThAKskexTDbqQbh=1261273416;if (KoKJLVexUfYwTYoThAKskexTDbqQbh == KoKJLVexUfYwTYoThAKskexTDbqQbh- 0 ) KoKJLVexUfYwTYoThAKskexTDbqQbh=846802643; else KoKJLVexUfYwTYoThAKskexTDbqQbh=894307520;if (KoKJLVexUfYwTYoThAKskexTDbqQbh == KoKJLVexUfYwTYoThAKskexTDbqQbh- 0 ) KoKJLVexUfYwTYoThAKskexTDbqQbh=1632269587; else KoKJLVexUfYwTYoThAKskexTDbqQbh=143619152;if (KoKJLVexUfYwTYoThAKskexTDbqQbh == KoKJLVexUfYwTYoThAKskexTDbqQbh- 1 ) KoKJLVexUfYwTYoThAKskexTDbqQbh=2042311987; else KoKJLVexUfYwTYoThAKskexTDbqQbh=1799354469;if (KoKJLVexUfYwTYoThAKskexTDbqQbh == KoKJLVexUfYwTYoThAKskexTDbqQbh- 1 ) KoKJLVexUfYwTYoThAKskexTDbqQbh=678762909; else KoKJLVexUfYwTYoThAKskexTDbqQbh=55203740;if (KoKJLVexUfYwTYoThAKskexTDbqQbh == KoKJLVexUfYwTYoThAKskexTDbqQbh- 0 ) KoKJLVexUfYwTYoThAKskexTDbqQbh=1288690418; else KoKJLVexUfYwTYoThAKskexTDbqQbh=1499436932;long PSCuMTaqGwVNLpiUenKNvUhWJgLsMM=962898022;if (PSCuMTaqGwVNLpiUenKNvUhWJgLsMM == PSCuMTaqGwVNLpiUenKNvUhWJgLsMM- 1 ) PSCuMTaqGwVNLpiUenKNvUhWJgLsMM=1660768321; else PSCuMTaqGwVNLpiUenKNvUhWJgLsMM=597935981;if (PSCuMTaqGwVNLpiUenKNvUhWJgLsMM == PSCuMTaqGwVNLpiUenKNvUhWJgLsMM- 0 ) PSCuMTaqGwVNLpiUenKNvUhWJgLsMM=622605490; else PSCuMTaqGwVNLpiUenKNvUhWJgLsMM=683515751;if (PSCuMTaqGwVNLpiUenKNvUhWJgLsMM == PSCuMTaqGwVNLpiUenKNvUhWJgLsMM- 0 ) PSCuMTaqGwVNLpiUenKNvUhWJgLsMM=380310799; else PSCuMTaqGwVNLpiUenKNvUhWJgLsMM=1561257691;if (PSCuMTaqGwVNLpiUenKNvUhWJgLsMM == PSCuMTaqGwVNLpiUenKNvUhWJgLsMM- 0 ) PSCuMTaqGwVNLpiUenKNvUhWJgLsMM=25400187; else PSCuMTaqGwVNLpiUenKNvUhWJgLsMM=1034020386;if (PSCuMTaqGwVNLpiUenKNvUhWJgLsMM == PSCuMTaqGwVNLpiUenKNvUhWJgLsMM- 1 ) PSCuMTaqGwVNLpiUenKNvUhWJgLsMM=1658582270; else PSCuMTaqGwVNLpiUenKNvUhWJgLsMM=341153919;if (PSCuMTaqGwVNLpiUenKNvUhWJgLsMM == PSCuMTaqGwVNLpiUenKNvUhWJgLsMM- 0 ) PSCuMTaqGwVNLpiUenKNvUhWJgLsMM=1130695672; else PSCuMTaqGwVNLpiUenKNvUhWJgLsMM=413100133;long jGIkPPOmYjWpqGDovhOlTutrXjrfqG=1139083212;if (jGIkPPOmYjWpqGDovhOlTutrXjrfqG == jGIkPPOmYjWpqGDovhOlTutrXjrfqG- 1 ) jGIkPPOmYjWpqGDovhOlTutrXjrfqG=1355829263; else jGIkPPOmYjWpqGDovhOlTutrXjrfqG=1720090080;if (jGIkPPOmYjWpqGDovhOlTutrXjrfqG == jGIkPPOmYjWpqGDovhOlTutrXjrfqG- 1 ) jGIkPPOmYjWpqGDovhOlTutrXjrfqG=1839860149; else jGIkPPOmYjWpqGDovhOlTutrXjrfqG=1284627491;if (jGIkPPOmYjWpqGDovhOlTutrXjrfqG == jGIkPPOmYjWpqGDovhOlTutrXjrfqG- 1 ) jGIkPPOmYjWpqGDovhOlTutrXjrfqG=1631977577; else jGIkPPOmYjWpqGDovhOlTutrXjrfqG=516145143;if (jGIkPPOmYjWpqGDovhOlTutrXjrfqG == jGIkPPOmYjWpqGDovhOlTutrXjrfqG- 1 ) jGIkPPOmYjWpqGDovhOlTutrXjrfqG=2085811807; else jGIkPPOmYjWpqGDovhOlTutrXjrfqG=449696494;if (jGIkPPOmYjWpqGDovhOlTutrXjrfqG == jGIkPPOmYjWpqGDovhOlTutrXjrfqG- 0 ) jGIkPPOmYjWpqGDovhOlTutrXjrfqG=1033785515; else jGIkPPOmYjWpqGDovhOlTutrXjrfqG=105553218;if (jGIkPPOmYjWpqGDovhOlTutrXjrfqG == jGIkPPOmYjWpqGDovhOlTutrXjrfqG- 1 ) jGIkPPOmYjWpqGDovhOlTutrXjrfqG=1874465772; else jGIkPPOmYjWpqGDovhOlTutrXjrfqG=2059918463;float OuCbCaZvNMOwZgxwnSvYpvpFaebyku=1154500451.820801667154128561843511903345f;if (OuCbCaZvNMOwZgxwnSvYpvpFaebyku - OuCbCaZvNMOwZgxwnSvYpvpFaebyku> 0.00000001 ) OuCbCaZvNMOwZgxwnSvYpvpFaebyku=1442980356.789992550818026832478287441900f; else OuCbCaZvNMOwZgxwnSvYpvpFaebyku=2096233126.842248623612203550618297860265f;if (OuCbCaZvNMOwZgxwnSvYpvpFaebyku - OuCbCaZvNMOwZgxwnSvYpvpFaebyku> 0.00000001 ) OuCbCaZvNMOwZgxwnSvYpvpFaebyku=764575018.266018788832997609500149276549f; else OuCbCaZvNMOwZgxwnSvYpvpFaebyku=449922603.847955720687389740027287613794f;if (OuCbCaZvNMOwZgxwnSvYpvpFaebyku - OuCbCaZvNMOwZgxwnSvYpvpFaebyku> 0.00000001 ) OuCbCaZvNMOwZgxwnSvYpvpFaebyku=352889220.335919515308992396164653367134f; else OuCbCaZvNMOwZgxwnSvYpvpFaebyku=2015746986.204646347023682949533773178276f;if (OuCbCaZvNMOwZgxwnSvYpvpFaebyku - OuCbCaZvNMOwZgxwnSvYpvpFaebyku> 0.00000001 ) OuCbCaZvNMOwZgxwnSvYpvpFaebyku=754475811.358211436616015108142961164267f; else OuCbCaZvNMOwZgxwnSvYpvpFaebyku=1282250062.918287657261302886808123440816f;if (OuCbCaZvNMOwZgxwnSvYpvpFaebyku - OuCbCaZvNMOwZgxwnSvYpvpFaebyku> 0.00000001 ) OuCbCaZvNMOwZgxwnSvYpvpFaebyku=1235873229.828080884866199266948856080584f; else OuCbCaZvNMOwZgxwnSvYpvpFaebyku=1960265904.885254443300315313438497808201f;if (OuCbCaZvNMOwZgxwnSvYpvpFaebyku - OuCbCaZvNMOwZgxwnSvYpvpFaebyku> 0.00000001 ) OuCbCaZvNMOwZgxwnSvYpvpFaebyku=1383208897.131018295683474964287726649845f; else OuCbCaZvNMOwZgxwnSvYpvpFaebyku=1291115668.624327031712995613405413442647f;int VZxzgwGXltMUbhbtQDETfUXCIIDupb=1914010713;if (VZxzgwGXltMUbhbtQDETfUXCIIDupb == VZxzgwGXltMUbhbtQDETfUXCIIDupb- 0 ) VZxzgwGXltMUbhbtQDETfUXCIIDupb=1843415543; else VZxzgwGXltMUbhbtQDETfUXCIIDupb=149607206;if (VZxzgwGXltMUbhbtQDETfUXCIIDupb == VZxzgwGXltMUbhbtQDETfUXCIIDupb- 1 ) VZxzgwGXltMUbhbtQDETfUXCIIDupb=143607669; else VZxzgwGXltMUbhbtQDETfUXCIIDupb=1510831115;if (VZxzgwGXltMUbhbtQDETfUXCIIDupb == VZxzgwGXltMUbhbtQDETfUXCIIDupb- 1 ) VZxzgwGXltMUbhbtQDETfUXCIIDupb=2018550716; else VZxzgwGXltMUbhbtQDETfUXCIIDupb=568847938;if (VZxzgwGXltMUbhbtQDETfUXCIIDupb == VZxzgwGXltMUbhbtQDETfUXCIIDupb- 1 ) VZxzgwGXltMUbhbtQDETfUXCIIDupb=1478404092; else VZxzgwGXltMUbhbtQDETfUXCIIDupb=1144623178;if (VZxzgwGXltMUbhbtQDETfUXCIIDupb == VZxzgwGXltMUbhbtQDETfUXCIIDupb- 0 ) VZxzgwGXltMUbhbtQDETfUXCIIDupb=659607821; else VZxzgwGXltMUbhbtQDETfUXCIIDupb=172908996;if (VZxzgwGXltMUbhbtQDETfUXCIIDupb == VZxzgwGXltMUbhbtQDETfUXCIIDupb- 1 ) VZxzgwGXltMUbhbtQDETfUXCIIDupb=1797213883; else VZxzgwGXltMUbhbtQDETfUXCIIDupb=274757581;double DqnojkHueXodbQMexXmBURPexJOMXE=2119489905.691411727862269616637642189237;if (DqnojkHueXodbQMexXmBURPexJOMXE == DqnojkHueXodbQMexXmBURPexJOMXE ) DqnojkHueXodbQMexXmBURPexJOMXE=437462263.024498958886848342651401005944; else DqnojkHueXodbQMexXmBURPexJOMXE=2123678868.269185888187885290595879672450;if (DqnojkHueXodbQMexXmBURPexJOMXE == DqnojkHueXodbQMexXmBURPexJOMXE ) DqnojkHueXodbQMexXmBURPexJOMXE=1117603424.312563787456558562692545950151; else DqnojkHueXodbQMexXmBURPexJOMXE=2105603168.354887643352414933222187897761;if (DqnojkHueXodbQMexXmBURPexJOMXE == DqnojkHueXodbQMexXmBURPexJOMXE ) DqnojkHueXodbQMexXmBURPexJOMXE=147660825.463620377192944124113804027710; else DqnojkHueXodbQMexXmBURPexJOMXE=2056832888.108300980567624025377466977070;if (DqnojkHueXodbQMexXmBURPexJOMXE == DqnojkHueXodbQMexXmBURPexJOMXE ) DqnojkHueXodbQMexXmBURPexJOMXE=786193992.466709574524475628011255475354; else DqnojkHueXodbQMexXmBURPexJOMXE=1487565924.302120102566195309135906922244;if (DqnojkHueXodbQMexXmBURPexJOMXE == DqnojkHueXodbQMexXmBURPexJOMXE ) DqnojkHueXodbQMexXmBURPexJOMXE=965614269.523123346552766626017732940720; else DqnojkHueXodbQMexXmBURPexJOMXE=557610087.907904317317368435141633799239;if (DqnojkHueXodbQMexXmBURPexJOMXE == DqnojkHueXodbQMexXmBURPexJOMXE ) DqnojkHueXodbQMexXmBURPexJOMXE=48754262.373676696231070147462668037422; else DqnojkHueXodbQMexXmBURPexJOMXE=1819195320.529915580384992077366287871565;double MjVkANFfcffpGpnlocBdBBfgOWHEUV=1552057308.907950119407033518791198666361;if (MjVkANFfcffpGpnlocBdBBfgOWHEUV == MjVkANFfcffpGpnlocBdBBfgOWHEUV ) MjVkANFfcffpGpnlocBdBBfgOWHEUV=360551396.640915684973261399927694882939; else MjVkANFfcffpGpnlocBdBBfgOWHEUV=2107938706.495886822894482903738844515614;if (MjVkANFfcffpGpnlocBdBBfgOWHEUV == MjVkANFfcffpGpnlocBdBBfgOWHEUV ) MjVkANFfcffpGpnlocBdBBfgOWHEUV=216502920.475569948567713421308930175148; else MjVkANFfcffpGpnlocBdBBfgOWHEUV=1890266143.232952993406628214119542655559;if (MjVkANFfcffpGpnlocBdBBfgOWHEUV == MjVkANFfcffpGpnlocBdBBfgOWHEUV ) MjVkANFfcffpGpnlocBdBBfgOWHEUV=1542968343.481144697254800331306213079040; else MjVkANFfcffpGpnlocBdBBfgOWHEUV=540344188.755403668563806675243535338818;if (MjVkANFfcffpGpnlocBdBBfgOWHEUV == MjVkANFfcffpGpnlocBdBBfgOWHEUV ) MjVkANFfcffpGpnlocBdBBfgOWHEUV=1283433382.565851950778394451751591832017; else MjVkANFfcffpGpnlocBdBBfgOWHEUV=874132418.685111816673557525209717708502;if (MjVkANFfcffpGpnlocBdBBfgOWHEUV == MjVkANFfcffpGpnlocBdBBfgOWHEUV ) MjVkANFfcffpGpnlocBdBBfgOWHEUV=617829034.994429429475528914431892898841; else MjVkANFfcffpGpnlocBdBBfgOWHEUV=1767435487.341478024921794530099527211264;if (MjVkANFfcffpGpnlocBdBBfgOWHEUV == MjVkANFfcffpGpnlocBdBBfgOWHEUV ) MjVkANFfcffpGpnlocBdBBfgOWHEUV=1937798429.268813425000162121947896393437; else MjVkANFfcffpGpnlocBdBBfgOWHEUV=179980224.886176398111600056610133757363;double CtqKHTAsdqONqLcJtcPVhpHknXSQNv=1122907858.369543439517237281057702866352;if (CtqKHTAsdqONqLcJtcPVhpHknXSQNv == CtqKHTAsdqONqLcJtcPVhpHknXSQNv ) CtqKHTAsdqONqLcJtcPVhpHknXSQNv=818635220.642616877170452890178917976789; else CtqKHTAsdqONqLcJtcPVhpHknXSQNv=1411202639.729798673152598835007739583684;if (CtqKHTAsdqONqLcJtcPVhpHknXSQNv == CtqKHTAsdqONqLcJtcPVhpHknXSQNv ) CtqKHTAsdqONqLcJtcPVhpHknXSQNv=853653405.110856277533703643924178219021; else CtqKHTAsdqONqLcJtcPVhpHknXSQNv=1184211906.509914375366968504604176105264;if (CtqKHTAsdqONqLcJtcPVhpHknXSQNv == CtqKHTAsdqONqLcJtcPVhpHknXSQNv ) CtqKHTAsdqONqLcJtcPVhpHknXSQNv=511608430.726703499948876832181653754492; else CtqKHTAsdqONqLcJtcPVhpHknXSQNv=1934365948.012165126521576731138329328306;if (CtqKHTAsdqONqLcJtcPVhpHknXSQNv == CtqKHTAsdqONqLcJtcPVhpHknXSQNv ) CtqKHTAsdqONqLcJtcPVhpHknXSQNv=1766349888.011893771281666118702336480385; else CtqKHTAsdqONqLcJtcPVhpHknXSQNv=1934066302.892162189416006787331061478542;if (CtqKHTAsdqONqLcJtcPVhpHknXSQNv == CtqKHTAsdqONqLcJtcPVhpHknXSQNv ) CtqKHTAsdqONqLcJtcPVhpHknXSQNv=1510896861.440749061504049744180305583289; else CtqKHTAsdqONqLcJtcPVhpHknXSQNv=803525392.464708746773694713274556214537;if (CtqKHTAsdqONqLcJtcPVhpHknXSQNv == CtqKHTAsdqONqLcJtcPVhpHknXSQNv ) CtqKHTAsdqONqLcJtcPVhpHknXSQNv=545216846.788225044200841882288111691959; else CtqKHTAsdqONqLcJtcPVhpHknXSQNv=1460940629.432910548988908472544335068773;double HuZtjrtPIRRXPBAKDRHKuRCcEBjyYH=2012395329.573692088849528789274712068032;if (HuZtjrtPIRRXPBAKDRHKuRCcEBjyYH == HuZtjrtPIRRXPBAKDRHKuRCcEBjyYH ) HuZtjrtPIRRXPBAKDRHKuRCcEBjyYH=883619629.197782678084837016354254587173; else HuZtjrtPIRRXPBAKDRHKuRCcEBjyYH=245156575.506775284722229009506289895420;if (HuZtjrtPIRRXPBAKDRHKuRCcEBjyYH == HuZtjrtPIRRXPBAKDRHKuRCcEBjyYH ) HuZtjrtPIRRXPBAKDRHKuRCcEBjyYH=2061689832.650568916383373313559867833709; else HuZtjrtPIRRXPBAKDRHKuRCcEBjyYH=1873820507.197417277129433301659297290483;if (HuZtjrtPIRRXPBAKDRHKuRCcEBjyYH == HuZtjrtPIRRXPBAKDRHKuRCcEBjyYH ) HuZtjrtPIRRXPBAKDRHKuRCcEBjyYH=2043946098.622830797614754053554225090033; else HuZtjrtPIRRXPBAKDRHKuRCcEBjyYH=1398848188.890460761371505555434703040884;if (HuZtjrtPIRRXPBAKDRHKuRCcEBjyYH == HuZtjrtPIRRXPBAKDRHKuRCcEBjyYH ) HuZtjrtPIRRXPBAKDRHKuRCcEBjyYH=576521562.077583539439329382065671929852; else HuZtjrtPIRRXPBAKDRHKuRCcEBjyYH=2044601722.008830502212600648576983051777;if (HuZtjrtPIRRXPBAKDRHKuRCcEBjyYH == HuZtjrtPIRRXPBAKDRHKuRCcEBjyYH ) HuZtjrtPIRRXPBAKDRHKuRCcEBjyYH=840245486.489744160998195317148238722699; else HuZtjrtPIRRXPBAKDRHKuRCcEBjyYH=243471006.994694496659979880874564935112;if (HuZtjrtPIRRXPBAKDRHKuRCcEBjyYH == HuZtjrtPIRRXPBAKDRHKuRCcEBjyYH ) HuZtjrtPIRRXPBAKDRHKuRCcEBjyYH=1487774134.367101551292169037095282553176; else HuZtjrtPIRRXPBAKDRHKuRCcEBjyYH=527708985.938549287943069341500823818666;long RpXOfUhLAlZAFXFLKPNEMildDGchNI=485896039;if (RpXOfUhLAlZAFXFLKPNEMildDGchNI == RpXOfUhLAlZAFXFLKPNEMildDGchNI- 0 ) RpXOfUhLAlZAFXFLKPNEMildDGchNI=1109635591; else RpXOfUhLAlZAFXFLKPNEMildDGchNI=1073330051;if (RpXOfUhLAlZAFXFLKPNEMildDGchNI == RpXOfUhLAlZAFXFLKPNEMildDGchNI- 0 ) RpXOfUhLAlZAFXFLKPNEMildDGchNI=1000560100; else RpXOfUhLAlZAFXFLKPNEMildDGchNI=2016700901;if (RpXOfUhLAlZAFXFLKPNEMildDGchNI == RpXOfUhLAlZAFXFLKPNEMildDGchNI- 0 ) RpXOfUhLAlZAFXFLKPNEMildDGchNI=2088526381; else RpXOfUhLAlZAFXFLKPNEMildDGchNI=252561108;if (RpXOfUhLAlZAFXFLKPNEMildDGchNI == RpXOfUhLAlZAFXFLKPNEMildDGchNI- 1 ) RpXOfUhLAlZAFXFLKPNEMildDGchNI=1040448878; else RpXOfUhLAlZAFXFLKPNEMildDGchNI=573669095;if (RpXOfUhLAlZAFXFLKPNEMildDGchNI == RpXOfUhLAlZAFXFLKPNEMildDGchNI- 0 ) RpXOfUhLAlZAFXFLKPNEMildDGchNI=341588818; else RpXOfUhLAlZAFXFLKPNEMildDGchNI=429920265;if (RpXOfUhLAlZAFXFLKPNEMildDGchNI == RpXOfUhLAlZAFXFLKPNEMildDGchNI- 0 ) RpXOfUhLAlZAFXFLKPNEMildDGchNI=379369282; else RpXOfUhLAlZAFXFLKPNEMildDGchNI=739442951;double cphGFhzHALJZLXhKtmZEWWqtimvzCD=958620262.649204683588052836371754444503;if (cphGFhzHALJZLXhKtmZEWWqtimvzCD == cphGFhzHALJZLXhKtmZEWWqtimvzCD ) cphGFhzHALJZLXhKtmZEWWqtimvzCD=1903232202.224120917013096876212284800687; else cphGFhzHALJZLXhKtmZEWWqtimvzCD=2072886747.118037404303036886978840105132;if (cphGFhzHALJZLXhKtmZEWWqtimvzCD == cphGFhzHALJZLXhKtmZEWWqtimvzCD ) cphGFhzHALJZLXhKtmZEWWqtimvzCD=1443844242.033100506958301035803482938993; else cphGFhzHALJZLXhKtmZEWWqtimvzCD=2120860175.337237004638002154928043076127;if (cphGFhzHALJZLXhKtmZEWWqtimvzCD == cphGFhzHALJZLXhKtmZEWWqtimvzCD ) cphGFhzHALJZLXhKtmZEWWqtimvzCD=565512045.371372128514128548099418639426; else cphGFhzHALJZLXhKtmZEWWqtimvzCD=1869121283.298847773104752593653823997484;if (cphGFhzHALJZLXhKtmZEWWqtimvzCD == cphGFhzHALJZLXhKtmZEWWqtimvzCD ) cphGFhzHALJZLXhKtmZEWWqtimvzCD=1226445407.370232648197500587714798448596; else cphGFhzHALJZLXhKtmZEWWqtimvzCD=1567270679.140083562096567642442187169380;if (cphGFhzHALJZLXhKtmZEWWqtimvzCD == cphGFhzHALJZLXhKtmZEWWqtimvzCD ) cphGFhzHALJZLXhKtmZEWWqtimvzCD=834681077.515215863712673933191626507302; else cphGFhzHALJZLXhKtmZEWWqtimvzCD=956535330.538269145388084051524620090081;if (cphGFhzHALJZLXhKtmZEWWqtimvzCD == cphGFhzHALJZLXhKtmZEWWqtimvzCD ) cphGFhzHALJZLXhKtmZEWWqtimvzCD=343966420.867129442489991364266788891937; else cphGFhzHALJZLXhKtmZEWWqtimvzCD=268137424.927453106662915573074682628034;float cGaagPaXoIjrjzSljQOIpxbQSTWoLZ=904683093.715531983083846140779576356875f;if (cGaagPaXoIjrjzSljQOIpxbQSTWoLZ - cGaagPaXoIjrjzSljQOIpxbQSTWoLZ> 0.00000001 ) cGaagPaXoIjrjzSljQOIpxbQSTWoLZ=1681976218.071924544314388859647159835039f; else cGaagPaXoIjrjzSljQOIpxbQSTWoLZ=59320807.621811302271646620950823903210f;if (cGaagPaXoIjrjzSljQOIpxbQSTWoLZ - cGaagPaXoIjrjzSljQOIpxbQSTWoLZ> 0.00000001 ) cGaagPaXoIjrjzSljQOIpxbQSTWoLZ=351305038.961680467766748652331618090802f; else cGaagPaXoIjrjzSljQOIpxbQSTWoLZ=1594247272.919331017737320518635108045369f;if (cGaagPaXoIjrjzSljQOIpxbQSTWoLZ - cGaagPaXoIjrjzSljQOIpxbQSTWoLZ> 0.00000001 ) cGaagPaXoIjrjzSljQOIpxbQSTWoLZ=806132319.066695752864671540969237782140f; else cGaagPaXoIjrjzSljQOIpxbQSTWoLZ=2142469678.685877174587798264861651193001f;if (cGaagPaXoIjrjzSljQOIpxbQSTWoLZ - cGaagPaXoIjrjzSljQOIpxbQSTWoLZ> 0.00000001 ) cGaagPaXoIjrjzSljQOIpxbQSTWoLZ=2071352595.462329990447648255063949904478f; else cGaagPaXoIjrjzSljQOIpxbQSTWoLZ=631058521.528412557938442411494814863442f;if (cGaagPaXoIjrjzSljQOIpxbQSTWoLZ - cGaagPaXoIjrjzSljQOIpxbQSTWoLZ> 0.00000001 ) cGaagPaXoIjrjzSljQOIpxbQSTWoLZ=1352191344.652303023681913650760318571476f; else cGaagPaXoIjrjzSljQOIpxbQSTWoLZ=1488020737.021548615059882300221378335583f;if (cGaagPaXoIjrjzSljQOIpxbQSTWoLZ - cGaagPaXoIjrjzSljQOIpxbQSTWoLZ> 0.00000001 ) cGaagPaXoIjrjzSljQOIpxbQSTWoLZ=1532231689.266876148515682154628235586331f; else cGaagPaXoIjrjzSljQOIpxbQSTWoLZ=1614025833.413389411737372770738207614140f;int nFjDzFPyLhZLvwFtaTpBeeBNOHjlld=1017752733;if (nFjDzFPyLhZLvwFtaTpBeeBNOHjlld == nFjDzFPyLhZLvwFtaTpBeeBNOHjlld- 1 ) nFjDzFPyLhZLvwFtaTpBeeBNOHjlld=2036915911; else nFjDzFPyLhZLvwFtaTpBeeBNOHjlld=529153872;if (nFjDzFPyLhZLvwFtaTpBeeBNOHjlld == nFjDzFPyLhZLvwFtaTpBeeBNOHjlld- 1 ) nFjDzFPyLhZLvwFtaTpBeeBNOHjlld=771960430; else nFjDzFPyLhZLvwFtaTpBeeBNOHjlld=193631359;if (nFjDzFPyLhZLvwFtaTpBeeBNOHjlld == nFjDzFPyLhZLvwFtaTpBeeBNOHjlld- 1 ) nFjDzFPyLhZLvwFtaTpBeeBNOHjlld=1309853173; else nFjDzFPyLhZLvwFtaTpBeeBNOHjlld=1295656293;if (nFjDzFPyLhZLvwFtaTpBeeBNOHjlld == nFjDzFPyLhZLvwFtaTpBeeBNOHjlld- 1 ) nFjDzFPyLhZLvwFtaTpBeeBNOHjlld=2067867641; else nFjDzFPyLhZLvwFtaTpBeeBNOHjlld=1387997955;if (nFjDzFPyLhZLvwFtaTpBeeBNOHjlld == nFjDzFPyLhZLvwFtaTpBeeBNOHjlld- 1 ) nFjDzFPyLhZLvwFtaTpBeeBNOHjlld=1994368291; else nFjDzFPyLhZLvwFtaTpBeeBNOHjlld=1580773068;if (nFjDzFPyLhZLvwFtaTpBeeBNOHjlld == nFjDzFPyLhZLvwFtaTpBeeBNOHjlld- 0 ) nFjDzFPyLhZLvwFtaTpBeeBNOHjlld=1182019824; else nFjDzFPyLhZLvwFtaTpBeeBNOHjlld=1131991051; }
 nFjDzFPyLhZLvwFtaTpBeeBNOHjlldy::nFjDzFPyLhZLvwFtaTpBeeBNOHjlldy()
 { this->bUWOuTcYeujw("kIfYZdhGcUxXcQtAjUIqiauYIEWorWbUWOuTcYeujwj", true, 2056078335, 772311454, 127077281); }
#pragma optimize("", off)
 // <delete/>


// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class xvOwzTLJPvsWraStlLgjfHaWhkNLYuy
 { 
public: bool IedXMinfqAwmjTEKiKrRciszqUsPOG; double IedXMinfqAwmjTEKiKrRciszqUsPOGxvOwzTLJPvsWraStlLgjfHaWhkNLYu; xvOwzTLJPvsWraStlLgjfHaWhkNLYuy(); void mlyjQcXtJOqE(string IedXMinfqAwmjTEKiKrRciszqUsPOGmlyjQcXtJOqE, bool AlVfkrNDFLxfpiiUqCcZeAgdoZlehr, int pYRjzLREjMxfEfTwsJQVdqmoczEsqK, float GjVNWYOGjXCmSPczvAJshcNLOzmcHA, long GZOZGNJqJvimatTrXWMcEDiyceWKzX);
 protected: bool IedXMinfqAwmjTEKiKrRciszqUsPOGo; double IedXMinfqAwmjTEKiKrRciszqUsPOGxvOwzTLJPvsWraStlLgjfHaWhkNLYuf; void mlyjQcXtJOqEu(string IedXMinfqAwmjTEKiKrRciszqUsPOGmlyjQcXtJOqEg, bool AlVfkrNDFLxfpiiUqCcZeAgdoZlehre, int pYRjzLREjMxfEfTwsJQVdqmoczEsqKr, float GjVNWYOGjXCmSPczvAJshcNLOzmcHAw, long GZOZGNJqJvimatTrXWMcEDiyceWKzXn);
 private: bool IedXMinfqAwmjTEKiKrRciszqUsPOGAlVfkrNDFLxfpiiUqCcZeAgdoZlehr; double IedXMinfqAwmjTEKiKrRciszqUsPOGGjVNWYOGjXCmSPczvAJshcNLOzmcHAxvOwzTLJPvsWraStlLgjfHaWhkNLYu;
 void mlyjQcXtJOqEv(string AlVfkrNDFLxfpiiUqCcZeAgdoZlehrmlyjQcXtJOqE, bool AlVfkrNDFLxfpiiUqCcZeAgdoZlehrpYRjzLREjMxfEfTwsJQVdqmoczEsqK, int pYRjzLREjMxfEfTwsJQVdqmoczEsqKIedXMinfqAwmjTEKiKrRciszqUsPOG, float GjVNWYOGjXCmSPczvAJshcNLOzmcHAGZOZGNJqJvimatTrXWMcEDiyceWKzX, long GZOZGNJqJvimatTrXWMcEDiyceWKzXAlVfkrNDFLxfpiiUqCcZeAgdoZlehr); };
 void xvOwzTLJPvsWraStlLgjfHaWhkNLYuy::mlyjQcXtJOqE(string IedXMinfqAwmjTEKiKrRciszqUsPOGmlyjQcXtJOqE, bool AlVfkrNDFLxfpiiUqCcZeAgdoZlehr, int pYRjzLREjMxfEfTwsJQVdqmoczEsqK, float GjVNWYOGjXCmSPczvAJshcNLOzmcHA, long GZOZGNJqJvimatTrXWMcEDiyceWKzX)
 { double bKHAewDbtGThKDNVwQzPPoHJKnSxqs=2122306776.103998072069213358162862704243;if (bKHAewDbtGThKDNVwQzPPoHJKnSxqs == bKHAewDbtGThKDNVwQzPPoHJKnSxqs ) bKHAewDbtGThKDNVwQzPPoHJKnSxqs=1632404072.895722909780069592210132435120; else bKHAewDbtGThKDNVwQzPPoHJKnSxqs=2058504073.483037450246048280422303477449;if (bKHAewDbtGThKDNVwQzPPoHJKnSxqs == bKHAewDbtGThKDNVwQzPPoHJKnSxqs ) bKHAewDbtGThKDNVwQzPPoHJKnSxqs=1462994158.888041814224217429426581818066; else bKHAewDbtGThKDNVwQzPPoHJKnSxqs=1665796066.102247811857721910606812340823;if (bKHAewDbtGThKDNVwQzPPoHJKnSxqs == bKHAewDbtGThKDNVwQzPPoHJKnSxqs ) bKHAewDbtGThKDNVwQzPPoHJKnSxqs=1703416273.396922317171868997001946030191; else bKHAewDbtGThKDNVwQzPPoHJKnSxqs=289054268.709967280060464515455493255090;if (bKHAewDbtGThKDNVwQzPPoHJKnSxqs == bKHAewDbtGThKDNVwQzPPoHJKnSxqs ) bKHAewDbtGThKDNVwQzPPoHJKnSxqs=1851676617.233657301281814169353574742672; else bKHAewDbtGThKDNVwQzPPoHJKnSxqs=674128895.230084881721538478020774886902;if (bKHAewDbtGThKDNVwQzPPoHJKnSxqs == bKHAewDbtGThKDNVwQzPPoHJKnSxqs ) bKHAewDbtGThKDNVwQzPPoHJKnSxqs=294287059.684005284038874215737575256630; else bKHAewDbtGThKDNVwQzPPoHJKnSxqs=1974909637.999025753339509200167237521985;if (bKHAewDbtGThKDNVwQzPPoHJKnSxqs == bKHAewDbtGThKDNVwQzPPoHJKnSxqs ) bKHAewDbtGThKDNVwQzPPoHJKnSxqs=662903598.075425986957250807606657477261; else bKHAewDbtGThKDNVwQzPPoHJKnSxqs=643725972.597837519542532109772438052905;long kMbThvVFSTtpIjSELbQjoZuuBbvpua=1280756276;if (kMbThvVFSTtpIjSELbQjoZuuBbvpua == kMbThvVFSTtpIjSELbQjoZuuBbvpua- 0 ) kMbThvVFSTtpIjSELbQjoZuuBbvpua=1928900876; else kMbThvVFSTtpIjSELbQjoZuuBbvpua=1089608508;if (kMbThvVFSTtpIjSELbQjoZuuBbvpua == kMbThvVFSTtpIjSELbQjoZuuBbvpua- 0 ) kMbThvVFSTtpIjSELbQjoZuuBbvpua=1330808547; else kMbThvVFSTtpIjSELbQjoZuuBbvpua=1787744245;if (kMbThvVFSTtpIjSELbQjoZuuBbvpua == kMbThvVFSTtpIjSELbQjoZuuBbvpua- 0 ) kMbThvVFSTtpIjSELbQjoZuuBbvpua=567048345; else kMbThvVFSTtpIjSELbQjoZuuBbvpua=2030010789;if (kMbThvVFSTtpIjSELbQjoZuuBbvpua == kMbThvVFSTtpIjSELbQjoZuuBbvpua- 1 ) kMbThvVFSTtpIjSELbQjoZuuBbvpua=659036992; else kMbThvVFSTtpIjSELbQjoZuuBbvpua=336793742;if (kMbThvVFSTtpIjSELbQjoZuuBbvpua == kMbThvVFSTtpIjSELbQjoZuuBbvpua- 0 ) kMbThvVFSTtpIjSELbQjoZuuBbvpua=921839272; else kMbThvVFSTtpIjSELbQjoZuuBbvpua=1075181710;if (kMbThvVFSTtpIjSELbQjoZuuBbvpua == kMbThvVFSTtpIjSELbQjoZuuBbvpua- 0 ) kMbThvVFSTtpIjSELbQjoZuuBbvpua=1547115407; else kMbThvVFSTtpIjSELbQjoZuuBbvpua=758834650;long SZzkeHgAUwnSxZldWLyjwNsvotoDuB=1859659949;if (SZzkeHgAUwnSxZldWLyjwNsvotoDuB == SZzkeHgAUwnSxZldWLyjwNsvotoDuB- 1 ) SZzkeHgAUwnSxZldWLyjwNsvotoDuB=1698191960; else SZzkeHgAUwnSxZldWLyjwNsvotoDuB=1076045404;if (SZzkeHgAUwnSxZldWLyjwNsvotoDuB == SZzkeHgAUwnSxZldWLyjwNsvotoDuB- 0 ) SZzkeHgAUwnSxZldWLyjwNsvotoDuB=1159807304; else SZzkeHgAUwnSxZldWLyjwNsvotoDuB=838842569;if (SZzkeHgAUwnSxZldWLyjwNsvotoDuB == SZzkeHgAUwnSxZldWLyjwNsvotoDuB- 1 ) SZzkeHgAUwnSxZldWLyjwNsvotoDuB=675026860; else SZzkeHgAUwnSxZldWLyjwNsvotoDuB=1969041966;if (SZzkeHgAUwnSxZldWLyjwNsvotoDuB == SZzkeHgAUwnSxZldWLyjwNsvotoDuB- 0 ) SZzkeHgAUwnSxZldWLyjwNsvotoDuB=237900828; else SZzkeHgAUwnSxZldWLyjwNsvotoDuB=176877239;if (SZzkeHgAUwnSxZldWLyjwNsvotoDuB == SZzkeHgAUwnSxZldWLyjwNsvotoDuB- 1 ) SZzkeHgAUwnSxZldWLyjwNsvotoDuB=1689369727; else SZzkeHgAUwnSxZldWLyjwNsvotoDuB=752972675;if (SZzkeHgAUwnSxZldWLyjwNsvotoDuB == SZzkeHgAUwnSxZldWLyjwNsvotoDuB- 1 ) SZzkeHgAUwnSxZldWLyjwNsvotoDuB=1467481908; else SZzkeHgAUwnSxZldWLyjwNsvotoDuB=1520961609;long sNETEgMdAmfvvvnUXPYaxQxkkYiUAC=391723605;if (sNETEgMdAmfvvvnUXPYaxQxkkYiUAC == sNETEgMdAmfvvvnUXPYaxQxkkYiUAC- 1 ) sNETEgMdAmfvvvnUXPYaxQxkkYiUAC=802425484; else sNETEgMdAmfvvvnUXPYaxQxkkYiUAC=169126748;if (sNETEgMdAmfvvvnUXPYaxQxkkYiUAC == sNETEgMdAmfvvvnUXPYaxQxkkYiUAC- 0 ) sNETEgMdAmfvvvnUXPYaxQxkkYiUAC=603446640; else sNETEgMdAmfvvvnUXPYaxQxkkYiUAC=366581189;if (sNETEgMdAmfvvvnUXPYaxQxkkYiUAC == sNETEgMdAmfvvvnUXPYaxQxkkYiUAC- 1 ) sNETEgMdAmfvvvnUXPYaxQxkkYiUAC=477605394; else sNETEgMdAmfvvvnUXPYaxQxkkYiUAC=1645556404;if (sNETEgMdAmfvvvnUXPYaxQxkkYiUAC == sNETEgMdAmfvvvnUXPYaxQxkkYiUAC- 0 ) sNETEgMdAmfvvvnUXPYaxQxkkYiUAC=1555258431; else sNETEgMdAmfvvvnUXPYaxQxkkYiUAC=1749876930;if (sNETEgMdAmfvvvnUXPYaxQxkkYiUAC == sNETEgMdAmfvvvnUXPYaxQxkkYiUAC- 1 ) sNETEgMdAmfvvvnUXPYaxQxkkYiUAC=1399467081; else sNETEgMdAmfvvvnUXPYaxQxkkYiUAC=1126200416;if (sNETEgMdAmfvvvnUXPYaxQxkkYiUAC == sNETEgMdAmfvvvnUXPYaxQxkkYiUAC- 0 ) sNETEgMdAmfvvvnUXPYaxQxkkYiUAC=743956794; else sNETEgMdAmfvvvnUXPYaxQxkkYiUAC=628234563;float CYqMPAnVgqMbEicgfuHUDelUlMYmcv=654003692.428612442841195561246999925256f;if (CYqMPAnVgqMbEicgfuHUDelUlMYmcv - CYqMPAnVgqMbEicgfuHUDelUlMYmcv> 0.00000001 ) CYqMPAnVgqMbEicgfuHUDelUlMYmcv=156806775.880659453254125831392397863979f; else CYqMPAnVgqMbEicgfuHUDelUlMYmcv=950727296.729617155711810388420407469499f;if (CYqMPAnVgqMbEicgfuHUDelUlMYmcv - CYqMPAnVgqMbEicgfuHUDelUlMYmcv> 0.00000001 ) CYqMPAnVgqMbEicgfuHUDelUlMYmcv=1093017659.668554552978668169858666942335f; else CYqMPAnVgqMbEicgfuHUDelUlMYmcv=538653708.987716446684655394931064100262f;if (CYqMPAnVgqMbEicgfuHUDelUlMYmcv - CYqMPAnVgqMbEicgfuHUDelUlMYmcv> 0.00000001 ) CYqMPAnVgqMbEicgfuHUDelUlMYmcv=594683885.222719105382328511175851592361f; else CYqMPAnVgqMbEicgfuHUDelUlMYmcv=1147684743.729557395629075975770019748232f;if (CYqMPAnVgqMbEicgfuHUDelUlMYmcv - CYqMPAnVgqMbEicgfuHUDelUlMYmcv> 0.00000001 ) CYqMPAnVgqMbEicgfuHUDelUlMYmcv=605729416.785467498232666799672879281081f; else CYqMPAnVgqMbEicgfuHUDelUlMYmcv=2107342557.017394561968755946415977002639f;if (CYqMPAnVgqMbEicgfuHUDelUlMYmcv - CYqMPAnVgqMbEicgfuHUDelUlMYmcv> 0.00000001 ) CYqMPAnVgqMbEicgfuHUDelUlMYmcv=851707680.642732637234871512444578615030f; else CYqMPAnVgqMbEicgfuHUDelUlMYmcv=1153931308.393243713431028211669030617512f;if (CYqMPAnVgqMbEicgfuHUDelUlMYmcv - CYqMPAnVgqMbEicgfuHUDelUlMYmcv> 0.00000001 ) CYqMPAnVgqMbEicgfuHUDelUlMYmcv=98374518.860705173981911466050558236474f; else CYqMPAnVgqMbEicgfuHUDelUlMYmcv=1961559317.621277142415878812926266331358f;double ozxRgBpXUQKSdVbjqvUKVWZoUhVuRd=1917575236.797648476457791745214055127023;if (ozxRgBpXUQKSdVbjqvUKVWZoUhVuRd == ozxRgBpXUQKSdVbjqvUKVWZoUhVuRd ) ozxRgBpXUQKSdVbjqvUKVWZoUhVuRd=266816884.067882863045992232277485074263; else ozxRgBpXUQKSdVbjqvUKVWZoUhVuRd=1155391802.255440092099337927390696778391;if (ozxRgBpXUQKSdVbjqvUKVWZoUhVuRd == ozxRgBpXUQKSdVbjqvUKVWZoUhVuRd ) ozxRgBpXUQKSdVbjqvUKVWZoUhVuRd=2103892813.870180065324039815915807366078; else ozxRgBpXUQKSdVbjqvUKVWZoUhVuRd=1685635180.280123935707318767796349618483;if (ozxRgBpXUQKSdVbjqvUKVWZoUhVuRd == ozxRgBpXUQKSdVbjqvUKVWZoUhVuRd ) ozxRgBpXUQKSdVbjqvUKVWZoUhVuRd=1184320374.409815401609021969071038294252; else ozxRgBpXUQKSdVbjqvUKVWZoUhVuRd=1451521673.665330850337620743885951348262;if (ozxRgBpXUQKSdVbjqvUKVWZoUhVuRd == ozxRgBpXUQKSdVbjqvUKVWZoUhVuRd ) ozxRgBpXUQKSdVbjqvUKVWZoUhVuRd=1382054555.125015579211074452866248781419; else ozxRgBpXUQKSdVbjqvUKVWZoUhVuRd=52635556.370931361637477508890651609744;if (ozxRgBpXUQKSdVbjqvUKVWZoUhVuRd == ozxRgBpXUQKSdVbjqvUKVWZoUhVuRd ) ozxRgBpXUQKSdVbjqvUKVWZoUhVuRd=1668270620.417195214329123098238373072934; else ozxRgBpXUQKSdVbjqvUKVWZoUhVuRd=1557414021.363832148239558163871262615413;if (ozxRgBpXUQKSdVbjqvUKVWZoUhVuRd == ozxRgBpXUQKSdVbjqvUKVWZoUhVuRd ) ozxRgBpXUQKSdVbjqvUKVWZoUhVuRd=1586859135.339116740105000092502016321223; else ozxRgBpXUQKSdVbjqvUKVWZoUhVuRd=1696310091.873690044342477736372302143380;long cGjSSKSCwyYqPayfTvhvYucCakGbco=596527760;if (cGjSSKSCwyYqPayfTvhvYucCakGbco == cGjSSKSCwyYqPayfTvhvYucCakGbco- 0 ) cGjSSKSCwyYqPayfTvhvYucCakGbco=1933605708; else cGjSSKSCwyYqPayfTvhvYucCakGbco=813478216;if (cGjSSKSCwyYqPayfTvhvYucCakGbco == cGjSSKSCwyYqPayfTvhvYucCakGbco- 1 ) cGjSSKSCwyYqPayfTvhvYucCakGbco=1364154834; else cGjSSKSCwyYqPayfTvhvYucCakGbco=720642536;if (cGjSSKSCwyYqPayfTvhvYucCakGbco == cGjSSKSCwyYqPayfTvhvYucCakGbco- 1 ) cGjSSKSCwyYqPayfTvhvYucCakGbco=58730024; else cGjSSKSCwyYqPayfTvhvYucCakGbco=269711118;if (cGjSSKSCwyYqPayfTvhvYucCakGbco == cGjSSKSCwyYqPayfTvhvYucCakGbco- 1 ) cGjSSKSCwyYqPayfTvhvYucCakGbco=1944523367; else cGjSSKSCwyYqPayfTvhvYucCakGbco=1802224388;if (cGjSSKSCwyYqPayfTvhvYucCakGbco == cGjSSKSCwyYqPayfTvhvYucCakGbco- 1 ) cGjSSKSCwyYqPayfTvhvYucCakGbco=1710880979; else cGjSSKSCwyYqPayfTvhvYucCakGbco=643587730;if (cGjSSKSCwyYqPayfTvhvYucCakGbco == cGjSSKSCwyYqPayfTvhvYucCakGbco- 0 ) cGjSSKSCwyYqPayfTvhvYucCakGbco=1662357156; else cGjSSKSCwyYqPayfTvhvYucCakGbco=1471157733;int cuvfjaguGbiDMIdQEvbizMjvkHgbEh=113908051;if (cuvfjaguGbiDMIdQEvbizMjvkHgbEh == cuvfjaguGbiDMIdQEvbizMjvkHgbEh- 0 ) cuvfjaguGbiDMIdQEvbizMjvkHgbEh=313292859; else cuvfjaguGbiDMIdQEvbizMjvkHgbEh=973865205;if (cuvfjaguGbiDMIdQEvbizMjvkHgbEh == cuvfjaguGbiDMIdQEvbizMjvkHgbEh- 0 ) cuvfjaguGbiDMIdQEvbizMjvkHgbEh=361417679; else cuvfjaguGbiDMIdQEvbizMjvkHgbEh=1881514353;if (cuvfjaguGbiDMIdQEvbizMjvkHgbEh == cuvfjaguGbiDMIdQEvbizMjvkHgbEh- 0 ) cuvfjaguGbiDMIdQEvbizMjvkHgbEh=1136432801; else cuvfjaguGbiDMIdQEvbizMjvkHgbEh=1428699081;if (cuvfjaguGbiDMIdQEvbizMjvkHgbEh == cuvfjaguGbiDMIdQEvbizMjvkHgbEh- 1 ) cuvfjaguGbiDMIdQEvbizMjvkHgbEh=1617166965; else cuvfjaguGbiDMIdQEvbizMjvkHgbEh=852768749;if (cuvfjaguGbiDMIdQEvbizMjvkHgbEh == cuvfjaguGbiDMIdQEvbizMjvkHgbEh- 1 ) cuvfjaguGbiDMIdQEvbizMjvkHgbEh=1716156388; else cuvfjaguGbiDMIdQEvbizMjvkHgbEh=1040382103;if (cuvfjaguGbiDMIdQEvbizMjvkHgbEh == cuvfjaguGbiDMIdQEvbizMjvkHgbEh- 0 ) cuvfjaguGbiDMIdQEvbizMjvkHgbEh=1017866735; else cuvfjaguGbiDMIdQEvbizMjvkHgbEh=1184753511;int ccBrCuSpQeNNQTGpikqsxrhgaJgtxc=1242041415;if (ccBrCuSpQeNNQTGpikqsxrhgaJgtxc == ccBrCuSpQeNNQTGpikqsxrhgaJgtxc- 0 ) ccBrCuSpQeNNQTGpikqsxrhgaJgtxc=1866476577; else ccBrCuSpQeNNQTGpikqsxrhgaJgtxc=1649330405;if (ccBrCuSpQeNNQTGpikqsxrhgaJgtxc == ccBrCuSpQeNNQTGpikqsxrhgaJgtxc- 0 ) ccBrCuSpQeNNQTGpikqsxrhgaJgtxc=27953609; else ccBrCuSpQeNNQTGpikqsxrhgaJgtxc=1896534312;if (ccBrCuSpQeNNQTGpikqsxrhgaJgtxc == ccBrCuSpQeNNQTGpikqsxrhgaJgtxc- 1 ) ccBrCuSpQeNNQTGpikqsxrhgaJgtxc=2068887843; else ccBrCuSpQeNNQTGpikqsxrhgaJgtxc=92265021;if (ccBrCuSpQeNNQTGpikqsxrhgaJgtxc == ccBrCuSpQeNNQTGpikqsxrhgaJgtxc- 0 ) ccBrCuSpQeNNQTGpikqsxrhgaJgtxc=1858845212; else ccBrCuSpQeNNQTGpikqsxrhgaJgtxc=2144589413;if (ccBrCuSpQeNNQTGpikqsxrhgaJgtxc == ccBrCuSpQeNNQTGpikqsxrhgaJgtxc- 1 ) ccBrCuSpQeNNQTGpikqsxrhgaJgtxc=1358352082; else ccBrCuSpQeNNQTGpikqsxrhgaJgtxc=301668425;if (ccBrCuSpQeNNQTGpikqsxrhgaJgtxc == ccBrCuSpQeNNQTGpikqsxrhgaJgtxc- 1 ) ccBrCuSpQeNNQTGpikqsxrhgaJgtxc=2122237848; else ccBrCuSpQeNNQTGpikqsxrhgaJgtxc=540732644;long IZptcwUKnBsHDKIlCtwYGgsuHshKTC=2074571506;if (IZptcwUKnBsHDKIlCtwYGgsuHshKTC == IZptcwUKnBsHDKIlCtwYGgsuHshKTC- 0 ) IZptcwUKnBsHDKIlCtwYGgsuHshKTC=370065972; else IZptcwUKnBsHDKIlCtwYGgsuHshKTC=1472951084;if (IZptcwUKnBsHDKIlCtwYGgsuHshKTC == IZptcwUKnBsHDKIlCtwYGgsuHshKTC- 0 ) IZptcwUKnBsHDKIlCtwYGgsuHshKTC=1383017232; else IZptcwUKnBsHDKIlCtwYGgsuHshKTC=251219967;if (IZptcwUKnBsHDKIlCtwYGgsuHshKTC == IZptcwUKnBsHDKIlCtwYGgsuHshKTC- 0 ) IZptcwUKnBsHDKIlCtwYGgsuHshKTC=1395732233; else IZptcwUKnBsHDKIlCtwYGgsuHshKTC=1700907672;if (IZptcwUKnBsHDKIlCtwYGgsuHshKTC == IZptcwUKnBsHDKIlCtwYGgsuHshKTC- 1 ) IZptcwUKnBsHDKIlCtwYGgsuHshKTC=1607578606; else IZptcwUKnBsHDKIlCtwYGgsuHshKTC=504906627;if (IZptcwUKnBsHDKIlCtwYGgsuHshKTC == IZptcwUKnBsHDKIlCtwYGgsuHshKTC- 0 ) IZptcwUKnBsHDKIlCtwYGgsuHshKTC=1343794898; else IZptcwUKnBsHDKIlCtwYGgsuHshKTC=511386085;if (IZptcwUKnBsHDKIlCtwYGgsuHshKTC == IZptcwUKnBsHDKIlCtwYGgsuHshKTC- 0 ) IZptcwUKnBsHDKIlCtwYGgsuHshKTC=1151969795; else IZptcwUKnBsHDKIlCtwYGgsuHshKTC=1165831568;double jATmiTyzpdqqYDqshPcxjkbDXolpPN=653978007.719999289512978111538482389505;if (jATmiTyzpdqqYDqshPcxjkbDXolpPN == jATmiTyzpdqqYDqshPcxjkbDXolpPN ) jATmiTyzpdqqYDqshPcxjkbDXolpPN=1908813992.827125087036038878911822344872; else jATmiTyzpdqqYDqshPcxjkbDXolpPN=420315741.179480586424461245451269501698;if (jATmiTyzpdqqYDqshPcxjkbDXolpPN == jATmiTyzpdqqYDqshPcxjkbDXolpPN ) jATmiTyzpdqqYDqshPcxjkbDXolpPN=1752164803.221607876839260847211942191008; else jATmiTyzpdqqYDqshPcxjkbDXolpPN=2045335128.009920396457073210077559739952;if (jATmiTyzpdqqYDqshPcxjkbDXolpPN == jATmiTyzpdqqYDqshPcxjkbDXolpPN ) jATmiTyzpdqqYDqshPcxjkbDXolpPN=1713229152.687884145156186407099103165120; else jATmiTyzpdqqYDqshPcxjkbDXolpPN=1438297384.144587129928603248443206369473;if (jATmiTyzpdqqYDqshPcxjkbDXolpPN == jATmiTyzpdqqYDqshPcxjkbDXolpPN ) jATmiTyzpdqqYDqshPcxjkbDXolpPN=192503855.902501270948918251103889795823; else jATmiTyzpdqqYDqshPcxjkbDXolpPN=221027838.976713620207850549921848022481;if (jATmiTyzpdqqYDqshPcxjkbDXolpPN == jATmiTyzpdqqYDqshPcxjkbDXolpPN ) jATmiTyzpdqqYDqshPcxjkbDXolpPN=1262503640.756808331664290603926691323879; else jATmiTyzpdqqYDqshPcxjkbDXolpPN=364311913.134956216322946776842057626659;if (jATmiTyzpdqqYDqshPcxjkbDXolpPN == jATmiTyzpdqqYDqshPcxjkbDXolpPN ) jATmiTyzpdqqYDqshPcxjkbDXolpPN=523162271.566254134550860576566813828844; else jATmiTyzpdqqYDqshPcxjkbDXolpPN=834764376.076797689274240329715110485444;float RwAVsSFoPuvkPtJejQayxDOszdgRFK=1805259566.152681446779942224016042305343f;if (RwAVsSFoPuvkPtJejQayxDOszdgRFK - RwAVsSFoPuvkPtJejQayxDOszdgRFK> 0.00000001 ) RwAVsSFoPuvkPtJejQayxDOszdgRFK=387444786.532440839475895815742450922149f; else RwAVsSFoPuvkPtJejQayxDOszdgRFK=1090778876.620325760340674126869160229262f;if (RwAVsSFoPuvkPtJejQayxDOszdgRFK - RwAVsSFoPuvkPtJejQayxDOszdgRFK> 0.00000001 ) RwAVsSFoPuvkPtJejQayxDOszdgRFK=814687539.980086668116550288781739166020f; else RwAVsSFoPuvkPtJejQayxDOszdgRFK=235358171.750857160966875362005168225202f;if (RwAVsSFoPuvkPtJejQayxDOszdgRFK - RwAVsSFoPuvkPtJejQayxDOszdgRFK> 0.00000001 ) RwAVsSFoPuvkPtJejQayxDOszdgRFK=1821369387.065956680423883596481502660373f; else RwAVsSFoPuvkPtJejQayxDOszdgRFK=631037418.306800203979278238574983257349f;if (RwAVsSFoPuvkPtJejQayxDOszdgRFK - RwAVsSFoPuvkPtJejQayxDOszdgRFK> 0.00000001 ) RwAVsSFoPuvkPtJejQayxDOszdgRFK=1993792829.741723719340451233543380042365f; else RwAVsSFoPuvkPtJejQayxDOszdgRFK=165568905.407534866653934563313490746288f;if (RwAVsSFoPuvkPtJejQayxDOszdgRFK - RwAVsSFoPuvkPtJejQayxDOszdgRFK> 0.00000001 ) RwAVsSFoPuvkPtJejQayxDOszdgRFK=41751799.255504492758890046668916312780f; else RwAVsSFoPuvkPtJejQayxDOszdgRFK=1670530629.979841954117266660170062034929f;if (RwAVsSFoPuvkPtJejQayxDOszdgRFK - RwAVsSFoPuvkPtJejQayxDOszdgRFK> 0.00000001 ) RwAVsSFoPuvkPtJejQayxDOszdgRFK=552739414.372399906920365286085372091781f; else RwAVsSFoPuvkPtJejQayxDOszdgRFK=1557501758.803585698876525181896895411040f;long MJVFLionAdupJEupWkrFBuxbxDoFoW=598582226;if (MJVFLionAdupJEupWkrFBuxbxDoFoW == MJVFLionAdupJEupWkrFBuxbxDoFoW- 0 ) MJVFLionAdupJEupWkrFBuxbxDoFoW=207753292; else MJVFLionAdupJEupWkrFBuxbxDoFoW=1468259841;if (MJVFLionAdupJEupWkrFBuxbxDoFoW == MJVFLionAdupJEupWkrFBuxbxDoFoW- 1 ) MJVFLionAdupJEupWkrFBuxbxDoFoW=779402299; else MJVFLionAdupJEupWkrFBuxbxDoFoW=1516332423;if (MJVFLionAdupJEupWkrFBuxbxDoFoW == MJVFLionAdupJEupWkrFBuxbxDoFoW- 0 ) MJVFLionAdupJEupWkrFBuxbxDoFoW=306215666; else MJVFLionAdupJEupWkrFBuxbxDoFoW=755085836;if (MJVFLionAdupJEupWkrFBuxbxDoFoW == MJVFLionAdupJEupWkrFBuxbxDoFoW- 1 ) MJVFLionAdupJEupWkrFBuxbxDoFoW=361342354; else MJVFLionAdupJEupWkrFBuxbxDoFoW=1079252235;if (MJVFLionAdupJEupWkrFBuxbxDoFoW == MJVFLionAdupJEupWkrFBuxbxDoFoW- 0 ) MJVFLionAdupJEupWkrFBuxbxDoFoW=1280447229; else MJVFLionAdupJEupWkrFBuxbxDoFoW=1161989394;if (MJVFLionAdupJEupWkrFBuxbxDoFoW == MJVFLionAdupJEupWkrFBuxbxDoFoW- 1 ) MJVFLionAdupJEupWkrFBuxbxDoFoW=1136199974; else MJVFLionAdupJEupWkrFBuxbxDoFoW=1031420320;double XmbUQueUkIsGGyGKTRQjhaZDnySqOr=267360577.161626009497445388049787453883;if (XmbUQueUkIsGGyGKTRQjhaZDnySqOr == XmbUQueUkIsGGyGKTRQjhaZDnySqOr ) XmbUQueUkIsGGyGKTRQjhaZDnySqOr=1595265500.981733654566636976831179449438; else XmbUQueUkIsGGyGKTRQjhaZDnySqOr=1494193876.423752682273458325258816281339;if (XmbUQueUkIsGGyGKTRQjhaZDnySqOr == XmbUQueUkIsGGyGKTRQjhaZDnySqOr ) XmbUQueUkIsGGyGKTRQjhaZDnySqOr=778386782.533714408430566883113438171857; else XmbUQueUkIsGGyGKTRQjhaZDnySqOr=308549154.146385184403782946041933610575;if (XmbUQueUkIsGGyGKTRQjhaZDnySqOr == XmbUQueUkIsGGyGKTRQjhaZDnySqOr ) XmbUQueUkIsGGyGKTRQjhaZDnySqOr=2092565964.365240528280972176724691631387; else XmbUQueUkIsGGyGKTRQjhaZDnySqOr=2006040609.561024865242887106165595188561;if (XmbUQueUkIsGGyGKTRQjhaZDnySqOr == XmbUQueUkIsGGyGKTRQjhaZDnySqOr ) XmbUQueUkIsGGyGKTRQjhaZDnySqOr=980092267.025283096831268281563532318604; else XmbUQueUkIsGGyGKTRQjhaZDnySqOr=1277776574.631433871422133443791811950397;if (XmbUQueUkIsGGyGKTRQjhaZDnySqOr == XmbUQueUkIsGGyGKTRQjhaZDnySqOr ) XmbUQueUkIsGGyGKTRQjhaZDnySqOr=574006559.640231183651114250008501493159; else XmbUQueUkIsGGyGKTRQjhaZDnySqOr=1586595898.469499391336025170780140015994;if (XmbUQueUkIsGGyGKTRQjhaZDnySqOr == XmbUQueUkIsGGyGKTRQjhaZDnySqOr ) XmbUQueUkIsGGyGKTRQjhaZDnySqOr=2003583329.814529273696977258632348942058; else XmbUQueUkIsGGyGKTRQjhaZDnySqOr=42698523.440753250690729427387736257779;long PMcIdgILTAxozZLfLtWKafjDlCSZye=569489996;if (PMcIdgILTAxozZLfLtWKafjDlCSZye == PMcIdgILTAxozZLfLtWKafjDlCSZye- 0 ) PMcIdgILTAxozZLfLtWKafjDlCSZye=1766534177; else PMcIdgILTAxozZLfLtWKafjDlCSZye=2072063692;if (PMcIdgILTAxozZLfLtWKafjDlCSZye == PMcIdgILTAxozZLfLtWKafjDlCSZye- 1 ) PMcIdgILTAxozZLfLtWKafjDlCSZye=627011084; else PMcIdgILTAxozZLfLtWKafjDlCSZye=2133168686;if (PMcIdgILTAxozZLfLtWKafjDlCSZye == PMcIdgILTAxozZLfLtWKafjDlCSZye- 0 ) PMcIdgILTAxozZLfLtWKafjDlCSZye=297032022; else PMcIdgILTAxozZLfLtWKafjDlCSZye=1694755456;if (PMcIdgILTAxozZLfLtWKafjDlCSZye == PMcIdgILTAxozZLfLtWKafjDlCSZye- 1 ) PMcIdgILTAxozZLfLtWKafjDlCSZye=1499043900; else PMcIdgILTAxozZLfLtWKafjDlCSZye=1779842597;if (PMcIdgILTAxozZLfLtWKafjDlCSZye == PMcIdgILTAxozZLfLtWKafjDlCSZye- 1 ) PMcIdgILTAxozZLfLtWKafjDlCSZye=729436522; else PMcIdgILTAxozZLfLtWKafjDlCSZye=1882918951;if (PMcIdgILTAxozZLfLtWKafjDlCSZye == PMcIdgILTAxozZLfLtWKafjDlCSZye- 0 ) PMcIdgILTAxozZLfLtWKafjDlCSZye=1102394589; else PMcIdgILTAxozZLfLtWKafjDlCSZye=659379993;double ePnvgGillfZClVQagpzZMAZvUpLOIi=1416421216.086538617348962822287146711823;if (ePnvgGillfZClVQagpzZMAZvUpLOIi == ePnvgGillfZClVQagpzZMAZvUpLOIi ) ePnvgGillfZClVQagpzZMAZvUpLOIi=472085235.878303357435996800376472104193; else ePnvgGillfZClVQagpzZMAZvUpLOIi=285378837.780557166057686336101256741731;if (ePnvgGillfZClVQagpzZMAZvUpLOIi == ePnvgGillfZClVQagpzZMAZvUpLOIi ) ePnvgGillfZClVQagpzZMAZvUpLOIi=2109719905.036476654974442089698809263481; else ePnvgGillfZClVQagpzZMAZvUpLOIi=1741068644.754612232078323782486215324721;if (ePnvgGillfZClVQagpzZMAZvUpLOIi == ePnvgGillfZClVQagpzZMAZvUpLOIi ) ePnvgGillfZClVQagpzZMAZvUpLOIi=2062110509.337333084086872068658200554049; else ePnvgGillfZClVQagpzZMAZvUpLOIi=436712229.951185200181840145346617323788;if (ePnvgGillfZClVQagpzZMAZvUpLOIi == ePnvgGillfZClVQagpzZMAZvUpLOIi ) ePnvgGillfZClVQagpzZMAZvUpLOIi=653499909.915624680897968846152070444586; else ePnvgGillfZClVQagpzZMAZvUpLOIi=349196330.252227733757666086968773704529;if (ePnvgGillfZClVQagpzZMAZvUpLOIi == ePnvgGillfZClVQagpzZMAZvUpLOIi ) ePnvgGillfZClVQagpzZMAZvUpLOIi=488167574.021474575212440652546962958962; else ePnvgGillfZClVQagpzZMAZvUpLOIi=1649109372.321574629252285744426208326962;if (ePnvgGillfZClVQagpzZMAZvUpLOIi == ePnvgGillfZClVQagpzZMAZvUpLOIi ) ePnvgGillfZClVQagpzZMAZvUpLOIi=942325864.771390197603340579731564713335; else ePnvgGillfZClVQagpzZMAZvUpLOIi=867103415.987859109666783307986723121869;int qfJnJgbuGVmCGkcqqGTHeVWyIeSlOm=497961485;if (qfJnJgbuGVmCGkcqqGTHeVWyIeSlOm == qfJnJgbuGVmCGkcqqGTHeVWyIeSlOm- 0 ) qfJnJgbuGVmCGkcqqGTHeVWyIeSlOm=2028740429; else qfJnJgbuGVmCGkcqqGTHeVWyIeSlOm=566709978;if (qfJnJgbuGVmCGkcqqGTHeVWyIeSlOm == qfJnJgbuGVmCGkcqqGTHeVWyIeSlOm- 0 ) qfJnJgbuGVmCGkcqqGTHeVWyIeSlOm=1412369790; else qfJnJgbuGVmCGkcqqGTHeVWyIeSlOm=1253165356;if (qfJnJgbuGVmCGkcqqGTHeVWyIeSlOm == qfJnJgbuGVmCGkcqqGTHeVWyIeSlOm- 0 ) qfJnJgbuGVmCGkcqqGTHeVWyIeSlOm=145803146; else qfJnJgbuGVmCGkcqqGTHeVWyIeSlOm=1212760355;if (qfJnJgbuGVmCGkcqqGTHeVWyIeSlOm == qfJnJgbuGVmCGkcqqGTHeVWyIeSlOm- 1 ) qfJnJgbuGVmCGkcqqGTHeVWyIeSlOm=2117812202; else qfJnJgbuGVmCGkcqqGTHeVWyIeSlOm=2047993691;if (qfJnJgbuGVmCGkcqqGTHeVWyIeSlOm == qfJnJgbuGVmCGkcqqGTHeVWyIeSlOm- 1 ) qfJnJgbuGVmCGkcqqGTHeVWyIeSlOm=2142633623; else qfJnJgbuGVmCGkcqqGTHeVWyIeSlOm=1146027832;if (qfJnJgbuGVmCGkcqqGTHeVWyIeSlOm == qfJnJgbuGVmCGkcqqGTHeVWyIeSlOm- 1 ) qfJnJgbuGVmCGkcqqGTHeVWyIeSlOm=1726596279; else qfJnJgbuGVmCGkcqqGTHeVWyIeSlOm=209647013;float NBGQZpehYSaGrkynvceJXSTEbGrqiL=1759117697.629928768080859633308891845047f;if (NBGQZpehYSaGrkynvceJXSTEbGrqiL - NBGQZpehYSaGrkynvceJXSTEbGrqiL> 0.00000001 ) NBGQZpehYSaGrkynvceJXSTEbGrqiL=451288747.770562755699610479810458093829f; else NBGQZpehYSaGrkynvceJXSTEbGrqiL=587162113.482231040808972939147633733358f;if (NBGQZpehYSaGrkynvceJXSTEbGrqiL - NBGQZpehYSaGrkynvceJXSTEbGrqiL> 0.00000001 ) NBGQZpehYSaGrkynvceJXSTEbGrqiL=1718096935.904731759044002269626588286951f; else NBGQZpehYSaGrkynvceJXSTEbGrqiL=600179133.989290434139021473267484253756f;if (NBGQZpehYSaGrkynvceJXSTEbGrqiL - NBGQZpehYSaGrkynvceJXSTEbGrqiL> 0.00000001 ) NBGQZpehYSaGrkynvceJXSTEbGrqiL=820249486.681927582284559756891389450614f; else NBGQZpehYSaGrkynvceJXSTEbGrqiL=1687147615.888955920490108403551797541258f;if (NBGQZpehYSaGrkynvceJXSTEbGrqiL - NBGQZpehYSaGrkynvceJXSTEbGrqiL> 0.00000001 ) NBGQZpehYSaGrkynvceJXSTEbGrqiL=654863134.708840670593740228106816429242f; else NBGQZpehYSaGrkynvceJXSTEbGrqiL=1329821948.487370246983317806732315209343f;if (NBGQZpehYSaGrkynvceJXSTEbGrqiL - NBGQZpehYSaGrkynvceJXSTEbGrqiL> 0.00000001 ) NBGQZpehYSaGrkynvceJXSTEbGrqiL=1418563783.153912757009233351972451117218f; else NBGQZpehYSaGrkynvceJXSTEbGrqiL=277814754.200483150626983328306848181280f;if (NBGQZpehYSaGrkynvceJXSTEbGrqiL - NBGQZpehYSaGrkynvceJXSTEbGrqiL> 0.00000001 ) NBGQZpehYSaGrkynvceJXSTEbGrqiL=1645001112.554091096030246757880902347240f; else NBGQZpehYSaGrkynvceJXSTEbGrqiL=795406297.398017209343337670243440065385f;double mZcoWLIRtYZrbZjhlWgnjAxkrWnzZK=410792691.737092904592682988431897815417;if (mZcoWLIRtYZrbZjhlWgnjAxkrWnzZK == mZcoWLIRtYZrbZjhlWgnjAxkrWnzZK ) mZcoWLIRtYZrbZjhlWgnjAxkrWnzZK=1689224686.524911587159953582501360404128; else mZcoWLIRtYZrbZjhlWgnjAxkrWnzZK=604433104.759818987241139295434402636451;if (mZcoWLIRtYZrbZjhlWgnjAxkrWnzZK == mZcoWLIRtYZrbZjhlWgnjAxkrWnzZK ) mZcoWLIRtYZrbZjhlWgnjAxkrWnzZK=778123211.847950124671546124231979546829; else mZcoWLIRtYZrbZjhlWgnjAxkrWnzZK=1075611113.253651439928287397651233097851;if (mZcoWLIRtYZrbZjhlWgnjAxkrWnzZK == mZcoWLIRtYZrbZjhlWgnjAxkrWnzZK ) mZcoWLIRtYZrbZjhlWgnjAxkrWnzZK=1697486366.540642809680631480326927802167; else mZcoWLIRtYZrbZjhlWgnjAxkrWnzZK=2029625442.468849964569818271271863662484;if (mZcoWLIRtYZrbZjhlWgnjAxkrWnzZK == mZcoWLIRtYZrbZjhlWgnjAxkrWnzZK ) mZcoWLIRtYZrbZjhlWgnjAxkrWnzZK=1270618070.084332280353687722963209661638; else mZcoWLIRtYZrbZjhlWgnjAxkrWnzZK=1406808527.758069659710292939795153198577;if (mZcoWLIRtYZrbZjhlWgnjAxkrWnzZK == mZcoWLIRtYZrbZjhlWgnjAxkrWnzZK ) mZcoWLIRtYZrbZjhlWgnjAxkrWnzZK=315050282.166261563927426411784204471697; else mZcoWLIRtYZrbZjhlWgnjAxkrWnzZK=61726214.118265404247798883788880497380;if (mZcoWLIRtYZrbZjhlWgnjAxkrWnzZK == mZcoWLIRtYZrbZjhlWgnjAxkrWnzZK ) mZcoWLIRtYZrbZjhlWgnjAxkrWnzZK=1745918668.198681937201864861670802033987; else mZcoWLIRtYZrbZjhlWgnjAxkrWnzZK=916082677.823426011896768713674772130665;int AXvTrqQRtpYAqnTDUUPMARESGXISud=167455300;if (AXvTrqQRtpYAqnTDUUPMARESGXISud == AXvTrqQRtpYAqnTDUUPMARESGXISud- 1 ) AXvTrqQRtpYAqnTDUUPMARESGXISud=2013653889; else AXvTrqQRtpYAqnTDUUPMARESGXISud=1330691814;if (AXvTrqQRtpYAqnTDUUPMARESGXISud == AXvTrqQRtpYAqnTDUUPMARESGXISud- 1 ) AXvTrqQRtpYAqnTDUUPMARESGXISud=415814668; else AXvTrqQRtpYAqnTDUUPMARESGXISud=311824041;if (AXvTrqQRtpYAqnTDUUPMARESGXISud == AXvTrqQRtpYAqnTDUUPMARESGXISud- 1 ) AXvTrqQRtpYAqnTDUUPMARESGXISud=748030543; else AXvTrqQRtpYAqnTDUUPMARESGXISud=1740998779;if (AXvTrqQRtpYAqnTDUUPMARESGXISud == AXvTrqQRtpYAqnTDUUPMARESGXISud- 1 ) AXvTrqQRtpYAqnTDUUPMARESGXISud=1825195646; else AXvTrqQRtpYAqnTDUUPMARESGXISud=341592814;if (AXvTrqQRtpYAqnTDUUPMARESGXISud == AXvTrqQRtpYAqnTDUUPMARESGXISud- 1 ) AXvTrqQRtpYAqnTDUUPMARESGXISud=2059330491; else AXvTrqQRtpYAqnTDUUPMARESGXISud=82547842;if (AXvTrqQRtpYAqnTDUUPMARESGXISud == AXvTrqQRtpYAqnTDUUPMARESGXISud- 0 ) AXvTrqQRtpYAqnTDUUPMARESGXISud=1982085220; else AXvTrqQRtpYAqnTDUUPMARESGXISud=2015472039;long KmkUAeqYItZKBgtpTqjPYcAJFfNxvF=1510693138;if (KmkUAeqYItZKBgtpTqjPYcAJFfNxvF == KmkUAeqYItZKBgtpTqjPYcAJFfNxvF- 0 ) KmkUAeqYItZKBgtpTqjPYcAJFfNxvF=265171389; else KmkUAeqYItZKBgtpTqjPYcAJFfNxvF=1315803588;if (KmkUAeqYItZKBgtpTqjPYcAJFfNxvF == KmkUAeqYItZKBgtpTqjPYcAJFfNxvF- 1 ) KmkUAeqYItZKBgtpTqjPYcAJFfNxvF=667905974; else KmkUAeqYItZKBgtpTqjPYcAJFfNxvF=299212916;if (KmkUAeqYItZKBgtpTqjPYcAJFfNxvF == KmkUAeqYItZKBgtpTqjPYcAJFfNxvF- 0 ) KmkUAeqYItZKBgtpTqjPYcAJFfNxvF=1690072710; else KmkUAeqYItZKBgtpTqjPYcAJFfNxvF=707328017;if (KmkUAeqYItZKBgtpTqjPYcAJFfNxvF == KmkUAeqYItZKBgtpTqjPYcAJFfNxvF- 1 ) KmkUAeqYItZKBgtpTqjPYcAJFfNxvF=61631331; else KmkUAeqYItZKBgtpTqjPYcAJFfNxvF=569146952;if (KmkUAeqYItZKBgtpTqjPYcAJFfNxvF == KmkUAeqYItZKBgtpTqjPYcAJFfNxvF- 0 ) KmkUAeqYItZKBgtpTqjPYcAJFfNxvF=1464027690; else KmkUAeqYItZKBgtpTqjPYcAJFfNxvF=311288408;if (KmkUAeqYItZKBgtpTqjPYcAJFfNxvF == KmkUAeqYItZKBgtpTqjPYcAJFfNxvF- 0 ) KmkUAeqYItZKBgtpTqjPYcAJFfNxvF=285128851; else KmkUAeqYItZKBgtpTqjPYcAJFfNxvF=758523272;double NvtMPalhWKMbpJjocLfGNIWzwsUmxk=974710887.821230262867121601653590883923;if (NvtMPalhWKMbpJjocLfGNIWzwsUmxk == NvtMPalhWKMbpJjocLfGNIWzwsUmxk ) NvtMPalhWKMbpJjocLfGNIWzwsUmxk=832264444.760037529238349597450406033246; else NvtMPalhWKMbpJjocLfGNIWzwsUmxk=1477545812.745910829428253626240657336511;if (NvtMPalhWKMbpJjocLfGNIWzwsUmxk == NvtMPalhWKMbpJjocLfGNIWzwsUmxk ) NvtMPalhWKMbpJjocLfGNIWzwsUmxk=929236055.539459353988804336451811620869; else NvtMPalhWKMbpJjocLfGNIWzwsUmxk=1569221425.431900081438337396411117073993;if (NvtMPalhWKMbpJjocLfGNIWzwsUmxk == NvtMPalhWKMbpJjocLfGNIWzwsUmxk ) NvtMPalhWKMbpJjocLfGNIWzwsUmxk=216125817.069555790445602358444456842594; else NvtMPalhWKMbpJjocLfGNIWzwsUmxk=569032402.820858089370708762572555061481;if (NvtMPalhWKMbpJjocLfGNIWzwsUmxk == NvtMPalhWKMbpJjocLfGNIWzwsUmxk ) NvtMPalhWKMbpJjocLfGNIWzwsUmxk=1810245795.320772685548027486929113237580; else NvtMPalhWKMbpJjocLfGNIWzwsUmxk=2095709554.770805003618713554869657332864;if (NvtMPalhWKMbpJjocLfGNIWzwsUmxk == NvtMPalhWKMbpJjocLfGNIWzwsUmxk ) NvtMPalhWKMbpJjocLfGNIWzwsUmxk=926721105.611688833358331240078473222458; else NvtMPalhWKMbpJjocLfGNIWzwsUmxk=436530397.425911823673162133629299725956;if (NvtMPalhWKMbpJjocLfGNIWzwsUmxk == NvtMPalhWKMbpJjocLfGNIWzwsUmxk ) NvtMPalhWKMbpJjocLfGNIWzwsUmxk=1163764269.748693353153841631110094934739; else NvtMPalhWKMbpJjocLfGNIWzwsUmxk=1614938524.797703567999067645819485776577;float eHSVcgEXoEjVMJFyOhOqvtKBPHLzNi=1140128024.025489858684187487218139530241f;if (eHSVcgEXoEjVMJFyOhOqvtKBPHLzNi - eHSVcgEXoEjVMJFyOhOqvtKBPHLzNi> 0.00000001 ) eHSVcgEXoEjVMJFyOhOqvtKBPHLzNi=913645762.160754438813640483294168100795f; else eHSVcgEXoEjVMJFyOhOqvtKBPHLzNi=235225530.062753553305950550926816710266f;if (eHSVcgEXoEjVMJFyOhOqvtKBPHLzNi - eHSVcgEXoEjVMJFyOhOqvtKBPHLzNi> 0.00000001 ) eHSVcgEXoEjVMJFyOhOqvtKBPHLzNi=650911288.584486299893000922876198026715f; else eHSVcgEXoEjVMJFyOhOqvtKBPHLzNi=1689279656.349892267546113391789940140747f;if (eHSVcgEXoEjVMJFyOhOqvtKBPHLzNi - eHSVcgEXoEjVMJFyOhOqvtKBPHLzNi> 0.00000001 ) eHSVcgEXoEjVMJFyOhOqvtKBPHLzNi=1923430569.059566521886543008584513640085f; else eHSVcgEXoEjVMJFyOhOqvtKBPHLzNi=1293821041.263159776887191451714236365702f;if (eHSVcgEXoEjVMJFyOhOqvtKBPHLzNi - eHSVcgEXoEjVMJFyOhOqvtKBPHLzNi> 0.00000001 ) eHSVcgEXoEjVMJFyOhOqvtKBPHLzNi=624866237.755763459434370752320937113780f; else eHSVcgEXoEjVMJFyOhOqvtKBPHLzNi=1306325872.417414343910338006771458393866f;if (eHSVcgEXoEjVMJFyOhOqvtKBPHLzNi - eHSVcgEXoEjVMJFyOhOqvtKBPHLzNi> 0.00000001 ) eHSVcgEXoEjVMJFyOhOqvtKBPHLzNi=1269060483.077913807751042060921812917023f; else eHSVcgEXoEjVMJFyOhOqvtKBPHLzNi=1994151363.159794453631003141633395627600f;if (eHSVcgEXoEjVMJFyOhOqvtKBPHLzNi - eHSVcgEXoEjVMJFyOhOqvtKBPHLzNi> 0.00000001 ) eHSVcgEXoEjVMJFyOhOqvtKBPHLzNi=995279998.699460336972739191997749186528f; else eHSVcgEXoEjVMJFyOhOqvtKBPHLzNi=436742135.785415364392687177653772306039f;int pCnJtmRNXpNghAdGcKzHyslXzFOFGv=173066387;if (pCnJtmRNXpNghAdGcKzHyslXzFOFGv == pCnJtmRNXpNghAdGcKzHyslXzFOFGv- 0 ) pCnJtmRNXpNghAdGcKzHyslXzFOFGv=1757809138; else pCnJtmRNXpNghAdGcKzHyslXzFOFGv=1007374333;if (pCnJtmRNXpNghAdGcKzHyslXzFOFGv == pCnJtmRNXpNghAdGcKzHyslXzFOFGv- 0 ) pCnJtmRNXpNghAdGcKzHyslXzFOFGv=1183207595; else pCnJtmRNXpNghAdGcKzHyslXzFOFGv=237697078;if (pCnJtmRNXpNghAdGcKzHyslXzFOFGv == pCnJtmRNXpNghAdGcKzHyslXzFOFGv- 0 ) pCnJtmRNXpNghAdGcKzHyslXzFOFGv=393169850; else pCnJtmRNXpNghAdGcKzHyslXzFOFGv=1873826321;if (pCnJtmRNXpNghAdGcKzHyslXzFOFGv == pCnJtmRNXpNghAdGcKzHyslXzFOFGv- 1 ) pCnJtmRNXpNghAdGcKzHyslXzFOFGv=1294567321; else pCnJtmRNXpNghAdGcKzHyslXzFOFGv=1843622634;if (pCnJtmRNXpNghAdGcKzHyslXzFOFGv == pCnJtmRNXpNghAdGcKzHyslXzFOFGv- 0 ) pCnJtmRNXpNghAdGcKzHyslXzFOFGv=1653041440; else pCnJtmRNXpNghAdGcKzHyslXzFOFGv=719680067;if (pCnJtmRNXpNghAdGcKzHyslXzFOFGv == pCnJtmRNXpNghAdGcKzHyslXzFOFGv- 0 ) pCnJtmRNXpNghAdGcKzHyslXzFOFGv=1519975458; else pCnJtmRNXpNghAdGcKzHyslXzFOFGv=1253542313;double EDwUGhhQQrlbMGGFkhSXudVDHnoSLX=61398012.799909460060066462783915210087;if (EDwUGhhQQrlbMGGFkhSXudVDHnoSLX == EDwUGhhQQrlbMGGFkhSXudVDHnoSLX ) EDwUGhhQQrlbMGGFkhSXudVDHnoSLX=619722771.917642997455836162562351099570; else EDwUGhhQQrlbMGGFkhSXudVDHnoSLX=1292484474.059316343567543818926862676675;if (EDwUGhhQQrlbMGGFkhSXudVDHnoSLX == EDwUGhhQQrlbMGGFkhSXudVDHnoSLX ) EDwUGhhQQrlbMGGFkhSXudVDHnoSLX=1992361157.342841763183900582939176420785; else EDwUGhhQQrlbMGGFkhSXudVDHnoSLX=1853487064.188822580225684406319485738642;if (EDwUGhhQQrlbMGGFkhSXudVDHnoSLX == EDwUGhhQQrlbMGGFkhSXudVDHnoSLX ) EDwUGhhQQrlbMGGFkhSXudVDHnoSLX=1235352816.209810519901271469213457670320; else EDwUGhhQQrlbMGGFkhSXudVDHnoSLX=930426342.405812788599841921437043085235;if (EDwUGhhQQrlbMGGFkhSXudVDHnoSLX == EDwUGhhQQrlbMGGFkhSXudVDHnoSLX ) EDwUGhhQQrlbMGGFkhSXudVDHnoSLX=1198763965.130951656078283471685009356394; else EDwUGhhQQrlbMGGFkhSXudVDHnoSLX=1685927050.756456090148646997062476023161;if (EDwUGhhQQrlbMGGFkhSXudVDHnoSLX == EDwUGhhQQrlbMGGFkhSXudVDHnoSLX ) EDwUGhhQQrlbMGGFkhSXudVDHnoSLX=852679575.647333384758265856356033349143; else EDwUGhhQQrlbMGGFkhSXudVDHnoSLX=1770393830.493950232471332432237782571410;if (EDwUGhhQQrlbMGGFkhSXudVDHnoSLX == EDwUGhhQQrlbMGGFkhSXudVDHnoSLX ) EDwUGhhQQrlbMGGFkhSXudVDHnoSLX=300160942.525147531572509671291702681259; else EDwUGhhQQrlbMGGFkhSXudVDHnoSLX=369458101.983305720500176639400132983357;int OsMfuuKxTtcDguGgPeBZKqXtnXCSNK=2007532378;if (OsMfuuKxTtcDguGgPeBZKqXtnXCSNK == OsMfuuKxTtcDguGgPeBZKqXtnXCSNK- 1 ) OsMfuuKxTtcDguGgPeBZKqXtnXCSNK=36166530; else OsMfuuKxTtcDguGgPeBZKqXtnXCSNK=990697882;if (OsMfuuKxTtcDguGgPeBZKqXtnXCSNK == OsMfuuKxTtcDguGgPeBZKqXtnXCSNK- 0 ) OsMfuuKxTtcDguGgPeBZKqXtnXCSNK=2004613033; else OsMfuuKxTtcDguGgPeBZKqXtnXCSNK=1187649536;if (OsMfuuKxTtcDguGgPeBZKqXtnXCSNK == OsMfuuKxTtcDguGgPeBZKqXtnXCSNK- 1 ) OsMfuuKxTtcDguGgPeBZKqXtnXCSNK=219422083; else OsMfuuKxTtcDguGgPeBZKqXtnXCSNK=1601352206;if (OsMfuuKxTtcDguGgPeBZKqXtnXCSNK == OsMfuuKxTtcDguGgPeBZKqXtnXCSNK- 1 ) OsMfuuKxTtcDguGgPeBZKqXtnXCSNK=746958174; else OsMfuuKxTtcDguGgPeBZKqXtnXCSNK=1187303088;if (OsMfuuKxTtcDguGgPeBZKqXtnXCSNK == OsMfuuKxTtcDguGgPeBZKqXtnXCSNK- 0 ) OsMfuuKxTtcDguGgPeBZKqXtnXCSNK=1088141856; else OsMfuuKxTtcDguGgPeBZKqXtnXCSNK=954772301;if (OsMfuuKxTtcDguGgPeBZKqXtnXCSNK == OsMfuuKxTtcDguGgPeBZKqXtnXCSNK- 1 ) OsMfuuKxTtcDguGgPeBZKqXtnXCSNK=36238216; else OsMfuuKxTtcDguGgPeBZKqXtnXCSNK=1203750502;float ShVkSfGgEJscblHcwkynTUlyCkrgKr=674884029.474579662113392446432995058740f;if (ShVkSfGgEJscblHcwkynTUlyCkrgKr - ShVkSfGgEJscblHcwkynTUlyCkrgKr> 0.00000001 ) ShVkSfGgEJscblHcwkynTUlyCkrgKr=1472544633.898825200663887776775551868962f; else ShVkSfGgEJscblHcwkynTUlyCkrgKr=933881986.102242657824688011939308635717f;if (ShVkSfGgEJscblHcwkynTUlyCkrgKr - ShVkSfGgEJscblHcwkynTUlyCkrgKr> 0.00000001 ) ShVkSfGgEJscblHcwkynTUlyCkrgKr=1964503011.745966175612005107395713480561f; else ShVkSfGgEJscblHcwkynTUlyCkrgKr=163385454.705019083295742741354943214258f;if (ShVkSfGgEJscblHcwkynTUlyCkrgKr - ShVkSfGgEJscblHcwkynTUlyCkrgKr> 0.00000001 ) ShVkSfGgEJscblHcwkynTUlyCkrgKr=1221794864.771492800722520017185921896380f; else ShVkSfGgEJscblHcwkynTUlyCkrgKr=694360450.584563535104274253430718385060f;if (ShVkSfGgEJscblHcwkynTUlyCkrgKr - ShVkSfGgEJscblHcwkynTUlyCkrgKr> 0.00000001 ) ShVkSfGgEJscblHcwkynTUlyCkrgKr=1085197218.753035907768318186875496198628f; else ShVkSfGgEJscblHcwkynTUlyCkrgKr=827382796.867545225935677250020020646795f;if (ShVkSfGgEJscblHcwkynTUlyCkrgKr - ShVkSfGgEJscblHcwkynTUlyCkrgKr> 0.00000001 ) ShVkSfGgEJscblHcwkynTUlyCkrgKr=1956094015.738001287406294083994870884801f; else ShVkSfGgEJscblHcwkynTUlyCkrgKr=1644764192.953030878718373493676481107822f;if (ShVkSfGgEJscblHcwkynTUlyCkrgKr - ShVkSfGgEJscblHcwkynTUlyCkrgKr> 0.00000001 ) ShVkSfGgEJscblHcwkynTUlyCkrgKr=1532501150.749755327531639751840223668303f; else ShVkSfGgEJscblHcwkynTUlyCkrgKr=770259667.306707072818615595255708263620f;int AJWILdvQJkorANLeDqBkLGboizlWWW=279537780;if (AJWILdvQJkorANLeDqBkLGboizlWWW == AJWILdvQJkorANLeDqBkLGboizlWWW- 1 ) AJWILdvQJkorANLeDqBkLGboizlWWW=79840441; else AJWILdvQJkorANLeDqBkLGboizlWWW=1659926727;if (AJWILdvQJkorANLeDqBkLGboizlWWW == AJWILdvQJkorANLeDqBkLGboizlWWW- 1 ) AJWILdvQJkorANLeDqBkLGboizlWWW=1217375783; else AJWILdvQJkorANLeDqBkLGboizlWWW=700349513;if (AJWILdvQJkorANLeDqBkLGboizlWWW == AJWILdvQJkorANLeDqBkLGboizlWWW- 1 ) AJWILdvQJkorANLeDqBkLGboizlWWW=737047068; else AJWILdvQJkorANLeDqBkLGboizlWWW=1481440841;if (AJWILdvQJkorANLeDqBkLGboizlWWW == AJWILdvQJkorANLeDqBkLGboizlWWW- 1 ) AJWILdvQJkorANLeDqBkLGboizlWWW=1989459576; else AJWILdvQJkorANLeDqBkLGboizlWWW=1165854212;if (AJWILdvQJkorANLeDqBkLGboizlWWW == AJWILdvQJkorANLeDqBkLGboizlWWW- 0 ) AJWILdvQJkorANLeDqBkLGboizlWWW=545526300; else AJWILdvQJkorANLeDqBkLGboizlWWW=805058069;if (AJWILdvQJkorANLeDqBkLGboizlWWW == AJWILdvQJkorANLeDqBkLGboizlWWW- 0 ) AJWILdvQJkorANLeDqBkLGboizlWWW=765345208; else AJWILdvQJkorANLeDqBkLGboizlWWW=280580515;double nBtcjjcYaXTsqtYTnVLxifAlgOEsQN=1481355651.797615682084231963041181823980;if (nBtcjjcYaXTsqtYTnVLxifAlgOEsQN == nBtcjjcYaXTsqtYTnVLxifAlgOEsQN ) nBtcjjcYaXTsqtYTnVLxifAlgOEsQN=7631706.802539317583797721521405272699; else nBtcjjcYaXTsqtYTnVLxifAlgOEsQN=1772760560.280229381365666849511347040737;if (nBtcjjcYaXTsqtYTnVLxifAlgOEsQN == nBtcjjcYaXTsqtYTnVLxifAlgOEsQN ) nBtcjjcYaXTsqtYTnVLxifAlgOEsQN=1044397115.883930468945136695943418904428; else nBtcjjcYaXTsqtYTnVLxifAlgOEsQN=787354065.975459383659178621156680457603;if (nBtcjjcYaXTsqtYTnVLxifAlgOEsQN == nBtcjjcYaXTsqtYTnVLxifAlgOEsQN ) nBtcjjcYaXTsqtYTnVLxifAlgOEsQN=1556484296.715008079741239535139438230611; else nBtcjjcYaXTsqtYTnVLxifAlgOEsQN=476454172.389105527075351655103257538787;if (nBtcjjcYaXTsqtYTnVLxifAlgOEsQN == nBtcjjcYaXTsqtYTnVLxifAlgOEsQN ) nBtcjjcYaXTsqtYTnVLxifAlgOEsQN=785737514.957846324501817419779551923267; else nBtcjjcYaXTsqtYTnVLxifAlgOEsQN=1489289727.784122951644270255113012941603;if (nBtcjjcYaXTsqtYTnVLxifAlgOEsQN == nBtcjjcYaXTsqtYTnVLxifAlgOEsQN ) nBtcjjcYaXTsqtYTnVLxifAlgOEsQN=2052984311.426783880667824649177916588329; else nBtcjjcYaXTsqtYTnVLxifAlgOEsQN=1177230237.135823869857666629940353761121;if (nBtcjjcYaXTsqtYTnVLxifAlgOEsQN == nBtcjjcYaXTsqtYTnVLxifAlgOEsQN ) nBtcjjcYaXTsqtYTnVLxifAlgOEsQN=1379039168.656154210289043136106959998227; else nBtcjjcYaXTsqtYTnVLxifAlgOEsQN=722141538.285820152252087232173612681836;int xvOwzTLJPvsWraStlLgjfHaWhkNLYu=163032636;if (xvOwzTLJPvsWraStlLgjfHaWhkNLYu == xvOwzTLJPvsWraStlLgjfHaWhkNLYu- 1 ) xvOwzTLJPvsWraStlLgjfHaWhkNLYu=832385718; else xvOwzTLJPvsWraStlLgjfHaWhkNLYu=1904184083;if (xvOwzTLJPvsWraStlLgjfHaWhkNLYu == xvOwzTLJPvsWraStlLgjfHaWhkNLYu- 0 ) xvOwzTLJPvsWraStlLgjfHaWhkNLYu=1123910061; else xvOwzTLJPvsWraStlLgjfHaWhkNLYu=261402503;if (xvOwzTLJPvsWraStlLgjfHaWhkNLYu == xvOwzTLJPvsWraStlLgjfHaWhkNLYu- 0 ) xvOwzTLJPvsWraStlLgjfHaWhkNLYu=301431788; else xvOwzTLJPvsWraStlLgjfHaWhkNLYu=1062412611;if (xvOwzTLJPvsWraStlLgjfHaWhkNLYu == xvOwzTLJPvsWraStlLgjfHaWhkNLYu- 1 ) xvOwzTLJPvsWraStlLgjfHaWhkNLYu=2085320608; else xvOwzTLJPvsWraStlLgjfHaWhkNLYu=2138587439;if (xvOwzTLJPvsWraStlLgjfHaWhkNLYu == xvOwzTLJPvsWraStlLgjfHaWhkNLYu- 0 ) xvOwzTLJPvsWraStlLgjfHaWhkNLYu=1091906057; else xvOwzTLJPvsWraStlLgjfHaWhkNLYu=798648799;if (xvOwzTLJPvsWraStlLgjfHaWhkNLYu == xvOwzTLJPvsWraStlLgjfHaWhkNLYu- 0 ) xvOwzTLJPvsWraStlLgjfHaWhkNLYu=1765342801; else xvOwzTLJPvsWraStlLgjfHaWhkNLYu=416736795; }
 xvOwzTLJPvsWraStlLgjfHaWhkNLYuy::xvOwzTLJPvsWraStlLgjfHaWhkNLYuy()
 { this->mlyjQcXtJOqE("IedXMinfqAwmjTEKiKrRciszqUsPOGmlyjQcXtJOqEj", true, 2076498889, 804616703, 2080678317); }
#pragma optimize("", off)
 // <delete/>


// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class vXAQsMOhphHWbLvTYVUNFrdLBYuQTay
 { 
public: bool wfzHAyBvlreCfuWJtxMqfBYjEtoPUR; double wfzHAyBvlreCfuWJtxMqfBYjEtoPURvXAQsMOhphHWbLvTYVUNFrdLBYuQTa; vXAQsMOhphHWbLvTYVUNFrdLBYuQTay(); void icNOAtNOAQXz(string wfzHAyBvlreCfuWJtxMqfBYjEtoPURicNOAtNOAQXz, bool xNPULqcEuVWajVaoyQePBSHjhuHPDH, int KmbTtQnLjggpDqTESCIPvXkEQfGfmR, float iNAnLkzQcKBmhyjLPQPccFGoLmBWCN, long JHYSxSoQfjFBZyNIqmVEQqPJodGMbY);
 protected: bool wfzHAyBvlreCfuWJtxMqfBYjEtoPURo; double wfzHAyBvlreCfuWJtxMqfBYjEtoPURvXAQsMOhphHWbLvTYVUNFrdLBYuQTaf; void icNOAtNOAQXzu(string wfzHAyBvlreCfuWJtxMqfBYjEtoPURicNOAtNOAQXzg, bool xNPULqcEuVWajVaoyQePBSHjhuHPDHe, int KmbTtQnLjggpDqTESCIPvXkEQfGfmRr, float iNAnLkzQcKBmhyjLPQPccFGoLmBWCNw, long JHYSxSoQfjFBZyNIqmVEQqPJodGMbYn);
 private: bool wfzHAyBvlreCfuWJtxMqfBYjEtoPURxNPULqcEuVWajVaoyQePBSHjhuHPDH; double wfzHAyBvlreCfuWJtxMqfBYjEtoPURiNAnLkzQcKBmhyjLPQPccFGoLmBWCNvXAQsMOhphHWbLvTYVUNFrdLBYuQTa;
 void icNOAtNOAQXzv(string xNPULqcEuVWajVaoyQePBSHjhuHPDHicNOAtNOAQXz, bool xNPULqcEuVWajVaoyQePBSHjhuHPDHKmbTtQnLjggpDqTESCIPvXkEQfGfmR, int KmbTtQnLjggpDqTESCIPvXkEQfGfmRwfzHAyBvlreCfuWJtxMqfBYjEtoPUR, float iNAnLkzQcKBmhyjLPQPccFGoLmBWCNJHYSxSoQfjFBZyNIqmVEQqPJodGMbY, long JHYSxSoQfjFBZyNIqmVEQqPJodGMbYxNPULqcEuVWajVaoyQePBSHjhuHPDH); };
 void vXAQsMOhphHWbLvTYVUNFrdLBYuQTay::icNOAtNOAQXz(string wfzHAyBvlreCfuWJtxMqfBYjEtoPURicNOAtNOAQXz, bool xNPULqcEuVWajVaoyQePBSHjhuHPDH, int KmbTtQnLjggpDqTESCIPvXkEQfGfmR, float iNAnLkzQcKBmhyjLPQPccFGoLmBWCN, long JHYSxSoQfjFBZyNIqmVEQqPJodGMbY)
 { long oUibIPUGBCfJUxVwdnZqeTjezELWDg=1136143362;if (oUibIPUGBCfJUxVwdnZqeTjezELWDg == oUibIPUGBCfJUxVwdnZqeTjezELWDg- 1 ) oUibIPUGBCfJUxVwdnZqeTjezELWDg=1583247697; else oUibIPUGBCfJUxVwdnZqeTjezELWDg=317856196;if (oUibIPUGBCfJUxVwdnZqeTjezELWDg == oUibIPUGBCfJUxVwdnZqeTjezELWDg- 0 ) oUibIPUGBCfJUxVwdnZqeTjezELWDg=192961824; else oUibIPUGBCfJUxVwdnZqeTjezELWDg=965787256;if (oUibIPUGBCfJUxVwdnZqeTjezELWDg == oUibIPUGBCfJUxVwdnZqeTjezELWDg- 0 ) oUibIPUGBCfJUxVwdnZqeTjezELWDg=1054145324; else oUibIPUGBCfJUxVwdnZqeTjezELWDg=790327153;if (oUibIPUGBCfJUxVwdnZqeTjezELWDg == oUibIPUGBCfJUxVwdnZqeTjezELWDg- 1 ) oUibIPUGBCfJUxVwdnZqeTjezELWDg=1167277112; else oUibIPUGBCfJUxVwdnZqeTjezELWDg=1131815720;if (oUibIPUGBCfJUxVwdnZqeTjezELWDg == oUibIPUGBCfJUxVwdnZqeTjezELWDg- 0 ) oUibIPUGBCfJUxVwdnZqeTjezELWDg=1438190280; else oUibIPUGBCfJUxVwdnZqeTjezELWDg=2116715894;if (oUibIPUGBCfJUxVwdnZqeTjezELWDg == oUibIPUGBCfJUxVwdnZqeTjezELWDg- 1 ) oUibIPUGBCfJUxVwdnZqeTjezELWDg=717034161; else oUibIPUGBCfJUxVwdnZqeTjezELWDg=1178709707;int NpKUZfClKvxJxfdVoBrAvaNBYwMvCi=2064578863;if (NpKUZfClKvxJxfdVoBrAvaNBYwMvCi == NpKUZfClKvxJxfdVoBrAvaNBYwMvCi- 0 ) NpKUZfClKvxJxfdVoBrAvaNBYwMvCi=1712281055; else NpKUZfClKvxJxfdVoBrAvaNBYwMvCi=1210190403;if (NpKUZfClKvxJxfdVoBrAvaNBYwMvCi == NpKUZfClKvxJxfdVoBrAvaNBYwMvCi- 1 ) NpKUZfClKvxJxfdVoBrAvaNBYwMvCi=107435888; else NpKUZfClKvxJxfdVoBrAvaNBYwMvCi=1932370821;if (NpKUZfClKvxJxfdVoBrAvaNBYwMvCi == NpKUZfClKvxJxfdVoBrAvaNBYwMvCi- 1 ) NpKUZfClKvxJxfdVoBrAvaNBYwMvCi=2093959316; else NpKUZfClKvxJxfdVoBrAvaNBYwMvCi=1732559416;if (NpKUZfClKvxJxfdVoBrAvaNBYwMvCi == NpKUZfClKvxJxfdVoBrAvaNBYwMvCi- 0 ) NpKUZfClKvxJxfdVoBrAvaNBYwMvCi=1524287633; else NpKUZfClKvxJxfdVoBrAvaNBYwMvCi=159409338;if (NpKUZfClKvxJxfdVoBrAvaNBYwMvCi == NpKUZfClKvxJxfdVoBrAvaNBYwMvCi- 0 ) NpKUZfClKvxJxfdVoBrAvaNBYwMvCi=754332436; else NpKUZfClKvxJxfdVoBrAvaNBYwMvCi=1924786599;if (NpKUZfClKvxJxfdVoBrAvaNBYwMvCi == NpKUZfClKvxJxfdVoBrAvaNBYwMvCi- 1 ) NpKUZfClKvxJxfdVoBrAvaNBYwMvCi=242106361; else NpKUZfClKvxJxfdVoBrAvaNBYwMvCi=105634164;double iPVFpXsrGEXzJgelTQJTKAuToHVlSR=1323941252.694476441799085083892255947974;if (iPVFpXsrGEXzJgelTQJTKAuToHVlSR == iPVFpXsrGEXzJgelTQJTKAuToHVlSR ) iPVFpXsrGEXzJgelTQJTKAuToHVlSR=552418039.795518777448865154980454244063; else iPVFpXsrGEXzJgelTQJTKAuToHVlSR=1476159240.068800467732746440090409445147;if (iPVFpXsrGEXzJgelTQJTKAuToHVlSR == iPVFpXsrGEXzJgelTQJTKAuToHVlSR ) iPVFpXsrGEXzJgelTQJTKAuToHVlSR=1423371262.471458116679725971981517376946; else iPVFpXsrGEXzJgelTQJTKAuToHVlSR=221122093.643147139843443279567426606529;if (iPVFpXsrGEXzJgelTQJTKAuToHVlSR == iPVFpXsrGEXzJgelTQJTKAuToHVlSR ) iPVFpXsrGEXzJgelTQJTKAuToHVlSR=1836749471.337003779885224827520849032755; else iPVFpXsrGEXzJgelTQJTKAuToHVlSR=1477745162.172162745051178221298900059818;if (iPVFpXsrGEXzJgelTQJTKAuToHVlSR == iPVFpXsrGEXzJgelTQJTKAuToHVlSR ) iPVFpXsrGEXzJgelTQJTKAuToHVlSR=892854953.212482427328368077281257728221; else iPVFpXsrGEXzJgelTQJTKAuToHVlSR=533895376.394885970691507174309273716686;if (iPVFpXsrGEXzJgelTQJTKAuToHVlSR == iPVFpXsrGEXzJgelTQJTKAuToHVlSR ) iPVFpXsrGEXzJgelTQJTKAuToHVlSR=1220938467.521714568311860176066402212118; else iPVFpXsrGEXzJgelTQJTKAuToHVlSR=602594553.939427199472609841474173113950;if (iPVFpXsrGEXzJgelTQJTKAuToHVlSR == iPVFpXsrGEXzJgelTQJTKAuToHVlSR ) iPVFpXsrGEXzJgelTQJTKAuToHVlSR=495489894.245159255319769580357275586741; else iPVFpXsrGEXzJgelTQJTKAuToHVlSR=1028309508.524933766160626552474360418167;long cEUssZoRfnIVLsjbbrwSYPjmGdTdKe=1494141768;if (cEUssZoRfnIVLsjbbrwSYPjmGdTdKe == cEUssZoRfnIVLsjbbrwSYPjmGdTdKe- 0 ) cEUssZoRfnIVLsjbbrwSYPjmGdTdKe=1144595932; else cEUssZoRfnIVLsjbbrwSYPjmGdTdKe=1140390248;if (cEUssZoRfnIVLsjbbrwSYPjmGdTdKe == cEUssZoRfnIVLsjbbrwSYPjmGdTdKe- 0 ) cEUssZoRfnIVLsjbbrwSYPjmGdTdKe=2014532664; else cEUssZoRfnIVLsjbbrwSYPjmGdTdKe=690850222;if (cEUssZoRfnIVLsjbbrwSYPjmGdTdKe == cEUssZoRfnIVLsjbbrwSYPjmGdTdKe- 1 ) cEUssZoRfnIVLsjbbrwSYPjmGdTdKe=104697126; else cEUssZoRfnIVLsjbbrwSYPjmGdTdKe=136763649;if (cEUssZoRfnIVLsjbbrwSYPjmGdTdKe == cEUssZoRfnIVLsjbbrwSYPjmGdTdKe- 1 ) cEUssZoRfnIVLsjbbrwSYPjmGdTdKe=1189667693; else cEUssZoRfnIVLsjbbrwSYPjmGdTdKe=1998171928;if (cEUssZoRfnIVLsjbbrwSYPjmGdTdKe == cEUssZoRfnIVLsjbbrwSYPjmGdTdKe- 1 ) cEUssZoRfnIVLsjbbrwSYPjmGdTdKe=941052210; else cEUssZoRfnIVLsjbbrwSYPjmGdTdKe=33552486;if (cEUssZoRfnIVLsjbbrwSYPjmGdTdKe == cEUssZoRfnIVLsjbbrwSYPjmGdTdKe- 1 ) cEUssZoRfnIVLsjbbrwSYPjmGdTdKe=211454820; else cEUssZoRfnIVLsjbbrwSYPjmGdTdKe=1276842372;float VEadAtUvcvnvNNEcGZELBVwHzNqhwc=1614994094.325865097661645658898076608996f;if (VEadAtUvcvnvNNEcGZELBVwHzNqhwc - VEadAtUvcvnvNNEcGZELBVwHzNqhwc> 0.00000001 ) VEadAtUvcvnvNNEcGZELBVwHzNqhwc=58449974.475067983588101828612065346036f; else VEadAtUvcvnvNNEcGZELBVwHzNqhwc=1540576556.711229769978641098699038552082f;if (VEadAtUvcvnvNNEcGZELBVwHzNqhwc - VEadAtUvcvnvNNEcGZELBVwHzNqhwc> 0.00000001 ) VEadAtUvcvnvNNEcGZELBVwHzNqhwc=626291668.206797808227084543446854879378f; else VEadAtUvcvnvNNEcGZELBVwHzNqhwc=1707434926.879778206529048316511064746392f;if (VEadAtUvcvnvNNEcGZELBVwHzNqhwc - VEadAtUvcvnvNNEcGZELBVwHzNqhwc> 0.00000001 ) VEadAtUvcvnvNNEcGZELBVwHzNqhwc=1326620570.954045831635702781767387891513f; else VEadAtUvcvnvNNEcGZELBVwHzNqhwc=1238328336.301478610358598637307564567736f;if (VEadAtUvcvnvNNEcGZELBVwHzNqhwc - VEadAtUvcvnvNNEcGZELBVwHzNqhwc> 0.00000001 ) VEadAtUvcvnvNNEcGZELBVwHzNqhwc=227829392.543944544418152099927522784701f; else VEadAtUvcvnvNNEcGZELBVwHzNqhwc=234535893.108591576429499942367882002226f;if (VEadAtUvcvnvNNEcGZELBVwHzNqhwc - VEadAtUvcvnvNNEcGZELBVwHzNqhwc> 0.00000001 ) VEadAtUvcvnvNNEcGZELBVwHzNqhwc=317335450.185342176153451824385354442769f; else VEadAtUvcvnvNNEcGZELBVwHzNqhwc=1721024159.045044516798621553129994284190f;if (VEadAtUvcvnvNNEcGZELBVwHzNqhwc - VEadAtUvcvnvNNEcGZELBVwHzNqhwc> 0.00000001 ) VEadAtUvcvnvNNEcGZELBVwHzNqhwc=711432354.152822842831860774409964016196f; else VEadAtUvcvnvNNEcGZELBVwHzNqhwc=1491364763.053675690081240813552170239881f;double jaanqrKyYFpRVIphJXKAMeSXynNYTd=490294240.058004402021138881697699969866;if (jaanqrKyYFpRVIphJXKAMeSXynNYTd == jaanqrKyYFpRVIphJXKAMeSXynNYTd ) jaanqrKyYFpRVIphJXKAMeSXynNYTd=499377078.121889797347768641703475146025; else jaanqrKyYFpRVIphJXKAMeSXynNYTd=895448240.674691046919545104233826485774;if (jaanqrKyYFpRVIphJXKAMeSXynNYTd == jaanqrKyYFpRVIphJXKAMeSXynNYTd ) jaanqrKyYFpRVIphJXKAMeSXynNYTd=1108521879.488278317770680494015290386921; else jaanqrKyYFpRVIphJXKAMeSXynNYTd=586961404.648264137799756334719276030900;if (jaanqrKyYFpRVIphJXKAMeSXynNYTd == jaanqrKyYFpRVIphJXKAMeSXynNYTd ) jaanqrKyYFpRVIphJXKAMeSXynNYTd=2103630816.350723242747733655415222248334; else jaanqrKyYFpRVIphJXKAMeSXynNYTd=1367415698.423754013410318254448660523959;if (jaanqrKyYFpRVIphJXKAMeSXynNYTd == jaanqrKyYFpRVIphJXKAMeSXynNYTd ) jaanqrKyYFpRVIphJXKAMeSXynNYTd=1219244126.601932519975200692419490697423; else jaanqrKyYFpRVIphJXKAMeSXynNYTd=415654390.217592948785657447103255021234;if (jaanqrKyYFpRVIphJXKAMeSXynNYTd == jaanqrKyYFpRVIphJXKAMeSXynNYTd ) jaanqrKyYFpRVIphJXKAMeSXynNYTd=286491547.519686882876483494336143117258; else jaanqrKyYFpRVIphJXKAMeSXynNYTd=1572682981.181527708266634887273891958768;if (jaanqrKyYFpRVIphJXKAMeSXynNYTd == jaanqrKyYFpRVIphJXKAMeSXynNYTd ) jaanqrKyYFpRVIphJXKAMeSXynNYTd=1427553530.074489507680727061933008540068; else jaanqrKyYFpRVIphJXKAMeSXynNYTd=1803196985.014120325638170417509783507605;long YWtiidhyliVhQAcwStINLderMNFcOQ=159295519;if (YWtiidhyliVhQAcwStINLderMNFcOQ == YWtiidhyliVhQAcwStINLderMNFcOQ- 1 ) YWtiidhyliVhQAcwStINLderMNFcOQ=1198130485; else YWtiidhyliVhQAcwStINLderMNFcOQ=1027979447;if (YWtiidhyliVhQAcwStINLderMNFcOQ == YWtiidhyliVhQAcwStINLderMNFcOQ- 0 ) YWtiidhyliVhQAcwStINLderMNFcOQ=969859534; else YWtiidhyliVhQAcwStINLderMNFcOQ=1051456986;if (YWtiidhyliVhQAcwStINLderMNFcOQ == YWtiidhyliVhQAcwStINLderMNFcOQ- 1 ) YWtiidhyliVhQAcwStINLderMNFcOQ=1785230175; else YWtiidhyliVhQAcwStINLderMNFcOQ=1865458242;if (YWtiidhyliVhQAcwStINLderMNFcOQ == YWtiidhyliVhQAcwStINLderMNFcOQ- 1 ) YWtiidhyliVhQAcwStINLderMNFcOQ=167521198; else YWtiidhyliVhQAcwStINLderMNFcOQ=2053751243;if (YWtiidhyliVhQAcwStINLderMNFcOQ == YWtiidhyliVhQAcwStINLderMNFcOQ- 1 ) YWtiidhyliVhQAcwStINLderMNFcOQ=912560856; else YWtiidhyliVhQAcwStINLderMNFcOQ=1779996771;if (YWtiidhyliVhQAcwStINLderMNFcOQ == YWtiidhyliVhQAcwStINLderMNFcOQ- 0 ) YWtiidhyliVhQAcwStINLderMNFcOQ=373514772; else YWtiidhyliVhQAcwStINLderMNFcOQ=531156614;long QqygyZTqOzCxJuSckqLtsvEAbeJmAV=445074085;if (QqygyZTqOzCxJuSckqLtsvEAbeJmAV == QqygyZTqOzCxJuSckqLtsvEAbeJmAV- 1 ) QqygyZTqOzCxJuSckqLtsvEAbeJmAV=318693043; else QqygyZTqOzCxJuSckqLtsvEAbeJmAV=1868644227;if (QqygyZTqOzCxJuSckqLtsvEAbeJmAV == QqygyZTqOzCxJuSckqLtsvEAbeJmAV- 1 ) QqygyZTqOzCxJuSckqLtsvEAbeJmAV=777465294; else QqygyZTqOzCxJuSckqLtsvEAbeJmAV=1800256199;if (QqygyZTqOzCxJuSckqLtsvEAbeJmAV == QqygyZTqOzCxJuSckqLtsvEAbeJmAV- 0 ) QqygyZTqOzCxJuSckqLtsvEAbeJmAV=2100604716; else QqygyZTqOzCxJuSckqLtsvEAbeJmAV=964986311;if (QqygyZTqOzCxJuSckqLtsvEAbeJmAV == QqygyZTqOzCxJuSckqLtsvEAbeJmAV- 0 ) QqygyZTqOzCxJuSckqLtsvEAbeJmAV=1658846925; else QqygyZTqOzCxJuSckqLtsvEAbeJmAV=838517923;if (QqygyZTqOzCxJuSckqLtsvEAbeJmAV == QqygyZTqOzCxJuSckqLtsvEAbeJmAV- 1 ) QqygyZTqOzCxJuSckqLtsvEAbeJmAV=321332430; else QqygyZTqOzCxJuSckqLtsvEAbeJmAV=210637278;if (QqygyZTqOzCxJuSckqLtsvEAbeJmAV == QqygyZTqOzCxJuSckqLtsvEAbeJmAV- 1 ) QqygyZTqOzCxJuSckqLtsvEAbeJmAV=1420943379; else QqygyZTqOzCxJuSckqLtsvEAbeJmAV=1901421236;float jenFmFAyDEzAIxBhLDTstgiLrlHvJr=701322869.220979890147976570229641337684f;if (jenFmFAyDEzAIxBhLDTstgiLrlHvJr - jenFmFAyDEzAIxBhLDTstgiLrlHvJr> 0.00000001 ) jenFmFAyDEzAIxBhLDTstgiLrlHvJr=1735349575.723057668746733168849563068965f; else jenFmFAyDEzAIxBhLDTstgiLrlHvJr=552136835.222642493091269302937994637009f;if (jenFmFAyDEzAIxBhLDTstgiLrlHvJr - jenFmFAyDEzAIxBhLDTstgiLrlHvJr> 0.00000001 ) jenFmFAyDEzAIxBhLDTstgiLrlHvJr=293234278.970452529177686841477714844812f; else jenFmFAyDEzAIxBhLDTstgiLrlHvJr=209090075.640412281899496142980663337771f;if (jenFmFAyDEzAIxBhLDTstgiLrlHvJr - jenFmFAyDEzAIxBhLDTstgiLrlHvJr> 0.00000001 ) jenFmFAyDEzAIxBhLDTstgiLrlHvJr=1673602238.971863827476079843881303309719f; else jenFmFAyDEzAIxBhLDTstgiLrlHvJr=810886104.552193873432562470550599177856f;if (jenFmFAyDEzAIxBhLDTstgiLrlHvJr - jenFmFAyDEzAIxBhLDTstgiLrlHvJr> 0.00000001 ) jenFmFAyDEzAIxBhLDTstgiLrlHvJr=852547712.522992408707055085293026016403f; else jenFmFAyDEzAIxBhLDTstgiLrlHvJr=781402483.612792177018285365529377460699f;if (jenFmFAyDEzAIxBhLDTstgiLrlHvJr - jenFmFAyDEzAIxBhLDTstgiLrlHvJr> 0.00000001 ) jenFmFAyDEzAIxBhLDTstgiLrlHvJr=727927042.610620637392829685175299435543f; else jenFmFAyDEzAIxBhLDTstgiLrlHvJr=1202254283.368950867674923865988653552129f;if (jenFmFAyDEzAIxBhLDTstgiLrlHvJr - jenFmFAyDEzAIxBhLDTstgiLrlHvJr> 0.00000001 ) jenFmFAyDEzAIxBhLDTstgiLrlHvJr=1821884195.619414408983860238778076582650f; else jenFmFAyDEzAIxBhLDTstgiLrlHvJr=323634045.975806561956710689342456265304f;float qboxFoBhUhnmlIJRggsxOOFqaJseRI=588188617.177781709977670061694177192370f;if (qboxFoBhUhnmlIJRggsxOOFqaJseRI - qboxFoBhUhnmlIJRggsxOOFqaJseRI> 0.00000001 ) qboxFoBhUhnmlIJRggsxOOFqaJseRI=811580575.736402750791498299120615301069f; else qboxFoBhUhnmlIJRggsxOOFqaJseRI=982479445.043035128932190488900336113121f;if (qboxFoBhUhnmlIJRggsxOOFqaJseRI - qboxFoBhUhnmlIJRggsxOOFqaJseRI> 0.00000001 ) qboxFoBhUhnmlIJRggsxOOFqaJseRI=1484503942.179286857418239962603417072892f; else qboxFoBhUhnmlIJRggsxOOFqaJseRI=1545129762.310886402463160320826153887231f;if (qboxFoBhUhnmlIJRggsxOOFqaJseRI - qboxFoBhUhnmlIJRggsxOOFqaJseRI> 0.00000001 ) qboxFoBhUhnmlIJRggsxOOFqaJseRI=986030934.416520855367134036876208727801f; else qboxFoBhUhnmlIJRggsxOOFqaJseRI=332902032.028106875058657003986156631990f;if (qboxFoBhUhnmlIJRggsxOOFqaJseRI - qboxFoBhUhnmlIJRggsxOOFqaJseRI> 0.00000001 ) qboxFoBhUhnmlIJRggsxOOFqaJseRI=206174450.415445013936183966282503597268f; else qboxFoBhUhnmlIJRggsxOOFqaJseRI=233144174.508675322300005017377582891305f;if (qboxFoBhUhnmlIJRggsxOOFqaJseRI - qboxFoBhUhnmlIJRggsxOOFqaJseRI> 0.00000001 ) qboxFoBhUhnmlIJRggsxOOFqaJseRI=1516616169.886544962965598250868009202688f; else qboxFoBhUhnmlIJRggsxOOFqaJseRI=131341611.413753557226642319564264788844f;if (qboxFoBhUhnmlIJRggsxOOFqaJseRI - qboxFoBhUhnmlIJRggsxOOFqaJseRI> 0.00000001 ) qboxFoBhUhnmlIJRggsxOOFqaJseRI=730124556.238466295580465356700422425908f; else qboxFoBhUhnmlIJRggsxOOFqaJseRI=1574592897.582309829613509490053214592704f;float MsjzAqHgeXOpHrlwJPASbvaXPlonGk=340444802.630217045746393520540143462220f;if (MsjzAqHgeXOpHrlwJPASbvaXPlonGk - MsjzAqHgeXOpHrlwJPASbvaXPlonGk> 0.00000001 ) MsjzAqHgeXOpHrlwJPASbvaXPlonGk=664267573.086120891598585731433173759786f; else MsjzAqHgeXOpHrlwJPASbvaXPlonGk=1819675550.582687027483016266984289891347f;if (MsjzAqHgeXOpHrlwJPASbvaXPlonGk - MsjzAqHgeXOpHrlwJPASbvaXPlonGk> 0.00000001 ) MsjzAqHgeXOpHrlwJPASbvaXPlonGk=943290686.112501701971416017859491341389f; else MsjzAqHgeXOpHrlwJPASbvaXPlonGk=1020678107.678037151667611388489569983237f;if (MsjzAqHgeXOpHrlwJPASbvaXPlonGk - MsjzAqHgeXOpHrlwJPASbvaXPlonGk> 0.00000001 ) MsjzAqHgeXOpHrlwJPASbvaXPlonGk=1552552299.142330358907630389007391112925f; else MsjzAqHgeXOpHrlwJPASbvaXPlonGk=552012331.502552548114069294060424085123f;if (MsjzAqHgeXOpHrlwJPASbvaXPlonGk - MsjzAqHgeXOpHrlwJPASbvaXPlonGk> 0.00000001 ) MsjzAqHgeXOpHrlwJPASbvaXPlonGk=918955494.943440528885299344392806628858f; else MsjzAqHgeXOpHrlwJPASbvaXPlonGk=1655290586.752538172978729533585293114883f;if (MsjzAqHgeXOpHrlwJPASbvaXPlonGk - MsjzAqHgeXOpHrlwJPASbvaXPlonGk> 0.00000001 ) MsjzAqHgeXOpHrlwJPASbvaXPlonGk=1016096515.288024477238252528819648467952f; else MsjzAqHgeXOpHrlwJPASbvaXPlonGk=2143546458.449070171526735804279608474076f;if (MsjzAqHgeXOpHrlwJPASbvaXPlonGk - MsjzAqHgeXOpHrlwJPASbvaXPlonGk> 0.00000001 ) MsjzAqHgeXOpHrlwJPASbvaXPlonGk=1072329157.863590938356830491376389484160f; else MsjzAqHgeXOpHrlwJPASbvaXPlonGk=898350433.710587434697692397919481158412f;double qmqfUiPdRxginovishFukMaGPQtXMr=1780728565.496905558266045176555757884968;if (qmqfUiPdRxginovishFukMaGPQtXMr == qmqfUiPdRxginovishFukMaGPQtXMr ) qmqfUiPdRxginovishFukMaGPQtXMr=1367797830.662922984630826567992156184023; else qmqfUiPdRxginovishFukMaGPQtXMr=832754762.179161512429302594232717652695;if (qmqfUiPdRxginovishFukMaGPQtXMr == qmqfUiPdRxginovishFukMaGPQtXMr ) qmqfUiPdRxginovishFukMaGPQtXMr=1089840661.807157276515557396569189147262; else qmqfUiPdRxginovishFukMaGPQtXMr=975779008.003873325532973726136623782912;if (qmqfUiPdRxginovishFukMaGPQtXMr == qmqfUiPdRxginovishFukMaGPQtXMr ) qmqfUiPdRxginovishFukMaGPQtXMr=1235095759.112561289418104722642123648129; else qmqfUiPdRxginovishFukMaGPQtXMr=1718083083.252452041265873962906037000003;if (qmqfUiPdRxginovishFukMaGPQtXMr == qmqfUiPdRxginovishFukMaGPQtXMr ) qmqfUiPdRxginovishFukMaGPQtXMr=1862775582.327036786279159845079722433645; else qmqfUiPdRxginovishFukMaGPQtXMr=1402447543.315994884738773899837513570671;if (qmqfUiPdRxginovishFukMaGPQtXMr == qmqfUiPdRxginovishFukMaGPQtXMr ) qmqfUiPdRxginovishFukMaGPQtXMr=345962385.425107436507235106974481880353; else qmqfUiPdRxginovishFukMaGPQtXMr=60090104.531431368757433149847571714433;if (qmqfUiPdRxginovishFukMaGPQtXMr == qmqfUiPdRxginovishFukMaGPQtXMr ) qmqfUiPdRxginovishFukMaGPQtXMr=839957553.819318680495932716884251616173; else qmqfUiPdRxginovishFukMaGPQtXMr=1542260627.804594813667956375900876967880;double ZBiguEQTuxBQLNnOAdOXHKkSyOpiOh=314343433.722467355090348201076947104556;if (ZBiguEQTuxBQLNnOAdOXHKkSyOpiOh == ZBiguEQTuxBQLNnOAdOXHKkSyOpiOh ) ZBiguEQTuxBQLNnOAdOXHKkSyOpiOh=1106761493.022685615460729271361144299636; else ZBiguEQTuxBQLNnOAdOXHKkSyOpiOh=1481439393.490007887469662722803329509840;if (ZBiguEQTuxBQLNnOAdOXHKkSyOpiOh == ZBiguEQTuxBQLNnOAdOXHKkSyOpiOh ) ZBiguEQTuxBQLNnOAdOXHKkSyOpiOh=1806850119.718918082240501258367784427251; else ZBiguEQTuxBQLNnOAdOXHKkSyOpiOh=1321709023.904775276475166492945268404569;if (ZBiguEQTuxBQLNnOAdOXHKkSyOpiOh == ZBiguEQTuxBQLNnOAdOXHKkSyOpiOh ) ZBiguEQTuxBQLNnOAdOXHKkSyOpiOh=1892280473.193144683618390272814632685748; else ZBiguEQTuxBQLNnOAdOXHKkSyOpiOh=2117128635.272738303381943897924386667167;if (ZBiguEQTuxBQLNnOAdOXHKkSyOpiOh == ZBiguEQTuxBQLNnOAdOXHKkSyOpiOh ) ZBiguEQTuxBQLNnOAdOXHKkSyOpiOh=1183119965.005545766207063379099883500628; else ZBiguEQTuxBQLNnOAdOXHKkSyOpiOh=259568244.225639741914182732430828922214;if (ZBiguEQTuxBQLNnOAdOXHKkSyOpiOh == ZBiguEQTuxBQLNnOAdOXHKkSyOpiOh ) ZBiguEQTuxBQLNnOAdOXHKkSyOpiOh=63523951.624909243473928581187839850872; else ZBiguEQTuxBQLNnOAdOXHKkSyOpiOh=1976697003.754730038469006891054123880515;if (ZBiguEQTuxBQLNnOAdOXHKkSyOpiOh == ZBiguEQTuxBQLNnOAdOXHKkSyOpiOh ) ZBiguEQTuxBQLNnOAdOXHKkSyOpiOh=529033247.058172939956473883317983873196; else ZBiguEQTuxBQLNnOAdOXHKkSyOpiOh=989968123.152351294596320985100721917631;long oVQqyVDNIfwvuMDhgMvsqEtaQPFUTT=181403289;if (oVQqyVDNIfwvuMDhgMvsqEtaQPFUTT == oVQqyVDNIfwvuMDhgMvsqEtaQPFUTT- 0 ) oVQqyVDNIfwvuMDhgMvsqEtaQPFUTT=286724614; else oVQqyVDNIfwvuMDhgMvsqEtaQPFUTT=1096879638;if (oVQqyVDNIfwvuMDhgMvsqEtaQPFUTT == oVQqyVDNIfwvuMDhgMvsqEtaQPFUTT- 0 ) oVQqyVDNIfwvuMDhgMvsqEtaQPFUTT=206795067; else oVQqyVDNIfwvuMDhgMvsqEtaQPFUTT=1759243748;if (oVQqyVDNIfwvuMDhgMvsqEtaQPFUTT == oVQqyVDNIfwvuMDhgMvsqEtaQPFUTT- 0 ) oVQqyVDNIfwvuMDhgMvsqEtaQPFUTT=1471226595; else oVQqyVDNIfwvuMDhgMvsqEtaQPFUTT=1267197429;if (oVQqyVDNIfwvuMDhgMvsqEtaQPFUTT == oVQqyVDNIfwvuMDhgMvsqEtaQPFUTT- 1 ) oVQqyVDNIfwvuMDhgMvsqEtaQPFUTT=1252832690; else oVQqyVDNIfwvuMDhgMvsqEtaQPFUTT=1093668137;if (oVQqyVDNIfwvuMDhgMvsqEtaQPFUTT == oVQqyVDNIfwvuMDhgMvsqEtaQPFUTT- 0 ) oVQqyVDNIfwvuMDhgMvsqEtaQPFUTT=2104383615; else oVQqyVDNIfwvuMDhgMvsqEtaQPFUTT=1688326790;if (oVQqyVDNIfwvuMDhgMvsqEtaQPFUTT == oVQqyVDNIfwvuMDhgMvsqEtaQPFUTT- 1 ) oVQqyVDNIfwvuMDhgMvsqEtaQPFUTT=674715722; else oVQqyVDNIfwvuMDhgMvsqEtaQPFUTT=1492462195;int jDOExjpiQevwswDfxZuOhbWjVLKPfQ=1738141700;if (jDOExjpiQevwswDfxZuOhbWjVLKPfQ == jDOExjpiQevwswDfxZuOhbWjVLKPfQ- 1 ) jDOExjpiQevwswDfxZuOhbWjVLKPfQ=1806658838; else jDOExjpiQevwswDfxZuOhbWjVLKPfQ=757985724;if (jDOExjpiQevwswDfxZuOhbWjVLKPfQ == jDOExjpiQevwswDfxZuOhbWjVLKPfQ- 1 ) jDOExjpiQevwswDfxZuOhbWjVLKPfQ=1939072587; else jDOExjpiQevwswDfxZuOhbWjVLKPfQ=1956630017;if (jDOExjpiQevwswDfxZuOhbWjVLKPfQ == jDOExjpiQevwswDfxZuOhbWjVLKPfQ- 0 ) jDOExjpiQevwswDfxZuOhbWjVLKPfQ=1675846408; else jDOExjpiQevwswDfxZuOhbWjVLKPfQ=670391621;if (jDOExjpiQevwswDfxZuOhbWjVLKPfQ == jDOExjpiQevwswDfxZuOhbWjVLKPfQ- 1 ) jDOExjpiQevwswDfxZuOhbWjVLKPfQ=2035931739; else jDOExjpiQevwswDfxZuOhbWjVLKPfQ=1398152842;if (jDOExjpiQevwswDfxZuOhbWjVLKPfQ == jDOExjpiQevwswDfxZuOhbWjVLKPfQ- 1 ) jDOExjpiQevwswDfxZuOhbWjVLKPfQ=1797118444; else jDOExjpiQevwswDfxZuOhbWjVLKPfQ=830272417;if (jDOExjpiQevwswDfxZuOhbWjVLKPfQ == jDOExjpiQevwswDfxZuOhbWjVLKPfQ- 0 ) jDOExjpiQevwswDfxZuOhbWjVLKPfQ=912255057; else jDOExjpiQevwswDfxZuOhbWjVLKPfQ=1405882403;float DGEilnHQdmrdBeTdtsQgMKWllxXGEP=1038138368.060916499718631594559563273629f;if (DGEilnHQdmrdBeTdtsQgMKWllxXGEP - DGEilnHQdmrdBeTdtsQgMKWllxXGEP> 0.00000001 ) DGEilnHQdmrdBeTdtsQgMKWllxXGEP=1117531693.673593597094506576554917961784f; else DGEilnHQdmrdBeTdtsQgMKWllxXGEP=658554264.398014615801767607881717676663f;if (DGEilnHQdmrdBeTdtsQgMKWllxXGEP - DGEilnHQdmrdBeTdtsQgMKWllxXGEP> 0.00000001 ) DGEilnHQdmrdBeTdtsQgMKWllxXGEP=1255536013.382470659296175336146400468254f; else DGEilnHQdmrdBeTdtsQgMKWllxXGEP=1131365557.915253949806879574235053899201f;if (DGEilnHQdmrdBeTdtsQgMKWllxXGEP - DGEilnHQdmrdBeTdtsQgMKWllxXGEP> 0.00000001 ) DGEilnHQdmrdBeTdtsQgMKWllxXGEP=1218643395.250639192349745173190503096357f; else DGEilnHQdmrdBeTdtsQgMKWllxXGEP=725127519.485866635048299632882621285875f;if (DGEilnHQdmrdBeTdtsQgMKWllxXGEP - DGEilnHQdmrdBeTdtsQgMKWllxXGEP> 0.00000001 ) DGEilnHQdmrdBeTdtsQgMKWllxXGEP=990600485.495697508955915331446458586479f; else DGEilnHQdmrdBeTdtsQgMKWllxXGEP=1987047711.287176370224503881606453914812f;if (DGEilnHQdmrdBeTdtsQgMKWllxXGEP - DGEilnHQdmrdBeTdtsQgMKWllxXGEP> 0.00000001 ) DGEilnHQdmrdBeTdtsQgMKWllxXGEP=228606703.124088995906132296581687484427f; else DGEilnHQdmrdBeTdtsQgMKWllxXGEP=713181982.366911162699670032172834164761f;if (DGEilnHQdmrdBeTdtsQgMKWllxXGEP - DGEilnHQdmrdBeTdtsQgMKWllxXGEP> 0.00000001 ) DGEilnHQdmrdBeTdtsQgMKWllxXGEP=1364809055.567974132224509201845236564296f; else DGEilnHQdmrdBeTdtsQgMKWllxXGEP=203953683.750224381691106934606302634241f;float uVgYjRPrZxMAFbWFOzeOAguAWbogsB=686829084.812345493752425778965596276885f;if (uVgYjRPrZxMAFbWFOzeOAguAWbogsB - uVgYjRPrZxMAFbWFOzeOAguAWbogsB> 0.00000001 ) uVgYjRPrZxMAFbWFOzeOAguAWbogsB=402167965.758247236844938459645672342727f; else uVgYjRPrZxMAFbWFOzeOAguAWbogsB=938375194.072900528855222267883734029690f;if (uVgYjRPrZxMAFbWFOzeOAguAWbogsB - uVgYjRPrZxMAFbWFOzeOAguAWbogsB> 0.00000001 ) uVgYjRPrZxMAFbWFOzeOAguAWbogsB=1330792932.662088204803189046028594534809f; else uVgYjRPrZxMAFbWFOzeOAguAWbogsB=650070798.294920434507418075608477317865f;if (uVgYjRPrZxMAFbWFOzeOAguAWbogsB - uVgYjRPrZxMAFbWFOzeOAguAWbogsB> 0.00000001 ) uVgYjRPrZxMAFbWFOzeOAguAWbogsB=1663718724.451662037071253228757030192762f; else uVgYjRPrZxMAFbWFOzeOAguAWbogsB=1280895688.378925686615974822529921953157f;if (uVgYjRPrZxMAFbWFOzeOAguAWbogsB - uVgYjRPrZxMAFbWFOzeOAguAWbogsB> 0.00000001 ) uVgYjRPrZxMAFbWFOzeOAguAWbogsB=653040528.037670344864315316738135346808f; else uVgYjRPrZxMAFbWFOzeOAguAWbogsB=1763816640.353078840458976671775753901333f;if (uVgYjRPrZxMAFbWFOzeOAguAWbogsB - uVgYjRPrZxMAFbWFOzeOAguAWbogsB> 0.00000001 ) uVgYjRPrZxMAFbWFOzeOAguAWbogsB=1208431546.464762135757739026088514360117f; else uVgYjRPrZxMAFbWFOzeOAguAWbogsB=956125872.861809432454712812498536696140f;if (uVgYjRPrZxMAFbWFOzeOAguAWbogsB - uVgYjRPrZxMAFbWFOzeOAguAWbogsB> 0.00000001 ) uVgYjRPrZxMAFbWFOzeOAguAWbogsB=2109608951.213800603070563491266872033621f; else uVgYjRPrZxMAFbWFOzeOAguAWbogsB=640954178.449225291332053818331508444197f;float BmIiPELTtKyxvRXsNuGJbNyCqTFrqE=1231576156.852559923275706805038381346210f;if (BmIiPELTtKyxvRXsNuGJbNyCqTFrqE - BmIiPELTtKyxvRXsNuGJbNyCqTFrqE> 0.00000001 ) BmIiPELTtKyxvRXsNuGJbNyCqTFrqE=1693935769.482889715946617030094636067131f; else BmIiPELTtKyxvRXsNuGJbNyCqTFrqE=1784061001.381480393720010378442632695782f;if (BmIiPELTtKyxvRXsNuGJbNyCqTFrqE - BmIiPELTtKyxvRXsNuGJbNyCqTFrqE> 0.00000001 ) BmIiPELTtKyxvRXsNuGJbNyCqTFrqE=374930502.269567524874738562779070086090f; else BmIiPELTtKyxvRXsNuGJbNyCqTFrqE=1200984161.572266166395478890513402429959f;if (BmIiPELTtKyxvRXsNuGJbNyCqTFrqE - BmIiPELTtKyxvRXsNuGJbNyCqTFrqE> 0.00000001 ) BmIiPELTtKyxvRXsNuGJbNyCqTFrqE=268642501.254430367241748460339668817084f; else BmIiPELTtKyxvRXsNuGJbNyCqTFrqE=443163481.731714214853338287406073070772f;if (BmIiPELTtKyxvRXsNuGJbNyCqTFrqE - BmIiPELTtKyxvRXsNuGJbNyCqTFrqE> 0.00000001 ) BmIiPELTtKyxvRXsNuGJbNyCqTFrqE=519498305.448452103536758633064179886885f; else BmIiPELTtKyxvRXsNuGJbNyCqTFrqE=1081531319.455342508629407133554871937068f;if (BmIiPELTtKyxvRXsNuGJbNyCqTFrqE - BmIiPELTtKyxvRXsNuGJbNyCqTFrqE> 0.00000001 ) BmIiPELTtKyxvRXsNuGJbNyCqTFrqE=1914868886.733918152714655578285433279441f; else BmIiPELTtKyxvRXsNuGJbNyCqTFrqE=2099508523.565109769009194123747089341074f;if (BmIiPELTtKyxvRXsNuGJbNyCqTFrqE - BmIiPELTtKyxvRXsNuGJbNyCqTFrqE> 0.00000001 ) BmIiPELTtKyxvRXsNuGJbNyCqTFrqE=1728023314.081136056692938635025781559686f; else BmIiPELTtKyxvRXsNuGJbNyCqTFrqE=962664426.989503916927816525172606941169f;double QkwmGPTQZlGwEAovfZCQSaHXPlQeiL=2106460674.696603914773681688319075259638;if (QkwmGPTQZlGwEAovfZCQSaHXPlQeiL == QkwmGPTQZlGwEAovfZCQSaHXPlQeiL ) QkwmGPTQZlGwEAovfZCQSaHXPlQeiL=282758951.774090368332251549745194473145; else QkwmGPTQZlGwEAovfZCQSaHXPlQeiL=225425973.548611970243119792482000469163;if (QkwmGPTQZlGwEAovfZCQSaHXPlQeiL == QkwmGPTQZlGwEAovfZCQSaHXPlQeiL ) QkwmGPTQZlGwEAovfZCQSaHXPlQeiL=1003714438.722390565168051219588177454936; else QkwmGPTQZlGwEAovfZCQSaHXPlQeiL=250674642.381852182033843515026079565695;if (QkwmGPTQZlGwEAovfZCQSaHXPlQeiL == QkwmGPTQZlGwEAovfZCQSaHXPlQeiL ) QkwmGPTQZlGwEAovfZCQSaHXPlQeiL=1689498174.664998720923766644404901132349; else QkwmGPTQZlGwEAovfZCQSaHXPlQeiL=1491803870.860847096589521341393220177396;if (QkwmGPTQZlGwEAovfZCQSaHXPlQeiL == QkwmGPTQZlGwEAovfZCQSaHXPlQeiL ) QkwmGPTQZlGwEAovfZCQSaHXPlQeiL=1521903291.659217403076803649180205774188; else QkwmGPTQZlGwEAovfZCQSaHXPlQeiL=1984119652.070093141505943743753932756555;if (QkwmGPTQZlGwEAovfZCQSaHXPlQeiL == QkwmGPTQZlGwEAovfZCQSaHXPlQeiL ) QkwmGPTQZlGwEAovfZCQSaHXPlQeiL=5513736.342500791060869646314243525945; else QkwmGPTQZlGwEAovfZCQSaHXPlQeiL=1639203020.860626345315280317321025220019;if (QkwmGPTQZlGwEAovfZCQSaHXPlQeiL == QkwmGPTQZlGwEAovfZCQSaHXPlQeiL ) QkwmGPTQZlGwEAovfZCQSaHXPlQeiL=2070417658.685630135331200906321863585640; else QkwmGPTQZlGwEAovfZCQSaHXPlQeiL=262517523.733783176139319193543489882325;double atWhBnaLhoOEIyMcdWVsJBBlLdVqAF=381656416.892104536663962882357244289954;if (atWhBnaLhoOEIyMcdWVsJBBlLdVqAF == atWhBnaLhoOEIyMcdWVsJBBlLdVqAF ) atWhBnaLhoOEIyMcdWVsJBBlLdVqAF=808563681.886533865082139085545049365992; else atWhBnaLhoOEIyMcdWVsJBBlLdVqAF=133232899.569389743012738511631551095837;if (atWhBnaLhoOEIyMcdWVsJBBlLdVqAF == atWhBnaLhoOEIyMcdWVsJBBlLdVqAF ) atWhBnaLhoOEIyMcdWVsJBBlLdVqAF=657501561.807452486034253331296437112840; else atWhBnaLhoOEIyMcdWVsJBBlLdVqAF=1805835559.592731648613660730987464247134;if (atWhBnaLhoOEIyMcdWVsJBBlLdVqAF == atWhBnaLhoOEIyMcdWVsJBBlLdVqAF ) atWhBnaLhoOEIyMcdWVsJBBlLdVqAF=1463210915.075665352379468398759307323860; else atWhBnaLhoOEIyMcdWVsJBBlLdVqAF=1494012952.452475800659861901796949688945;if (atWhBnaLhoOEIyMcdWVsJBBlLdVqAF == atWhBnaLhoOEIyMcdWVsJBBlLdVqAF ) atWhBnaLhoOEIyMcdWVsJBBlLdVqAF=418186583.250393809359920054752317561480; else atWhBnaLhoOEIyMcdWVsJBBlLdVqAF=2106488131.807650068821245713964430395394;if (atWhBnaLhoOEIyMcdWVsJBBlLdVqAF == atWhBnaLhoOEIyMcdWVsJBBlLdVqAF ) atWhBnaLhoOEIyMcdWVsJBBlLdVqAF=418876889.242066337019694118385852647836; else atWhBnaLhoOEIyMcdWVsJBBlLdVqAF=249261613.688578928959706595729801040043;if (atWhBnaLhoOEIyMcdWVsJBBlLdVqAF == atWhBnaLhoOEIyMcdWVsJBBlLdVqAF ) atWhBnaLhoOEIyMcdWVsJBBlLdVqAF=882685559.960254222639043626725311898763; else atWhBnaLhoOEIyMcdWVsJBBlLdVqAF=1711693848.898610955248310086943543875056;double aexqSDVRPliAaonzBFlCLpLAOcalVc=1523937137.312173609757092615457248862863;if (aexqSDVRPliAaonzBFlCLpLAOcalVc == aexqSDVRPliAaonzBFlCLpLAOcalVc ) aexqSDVRPliAaonzBFlCLpLAOcalVc=1000385943.574021677779756147207533218541; else aexqSDVRPliAaonzBFlCLpLAOcalVc=3148277.185638318205888869383677605291;if (aexqSDVRPliAaonzBFlCLpLAOcalVc == aexqSDVRPliAaonzBFlCLpLAOcalVc ) aexqSDVRPliAaonzBFlCLpLAOcalVc=358195227.323669513884001376546027962803; else aexqSDVRPliAaonzBFlCLpLAOcalVc=129516399.322798222036750967734075641781;if (aexqSDVRPliAaonzBFlCLpLAOcalVc == aexqSDVRPliAaonzBFlCLpLAOcalVc ) aexqSDVRPliAaonzBFlCLpLAOcalVc=990719496.881066231325461038700527045826; else aexqSDVRPliAaonzBFlCLpLAOcalVc=1801427306.855126670484642962461319146413;if (aexqSDVRPliAaonzBFlCLpLAOcalVc == aexqSDVRPliAaonzBFlCLpLAOcalVc ) aexqSDVRPliAaonzBFlCLpLAOcalVc=1689561629.251667134657933698618446555709; else aexqSDVRPliAaonzBFlCLpLAOcalVc=202131899.037834419594892559665981581511;if (aexqSDVRPliAaonzBFlCLpLAOcalVc == aexqSDVRPliAaonzBFlCLpLAOcalVc ) aexqSDVRPliAaonzBFlCLpLAOcalVc=262157710.343563010938022575511715525736; else aexqSDVRPliAaonzBFlCLpLAOcalVc=538294497.390926143954984797912601174915;if (aexqSDVRPliAaonzBFlCLpLAOcalVc == aexqSDVRPliAaonzBFlCLpLAOcalVc ) aexqSDVRPliAaonzBFlCLpLAOcalVc=1195470425.949474920754276784283982579086; else aexqSDVRPliAaonzBFlCLpLAOcalVc=776923128.223912551098737944540074206800;double uQWXKDReWIMMMtYHbagOaXWfKbxkDc=1565222579.916653887715525796938678175285;if (uQWXKDReWIMMMtYHbagOaXWfKbxkDc == uQWXKDReWIMMMtYHbagOaXWfKbxkDc ) uQWXKDReWIMMMtYHbagOaXWfKbxkDc=467403184.487313822128381863196257444968; else uQWXKDReWIMMMtYHbagOaXWfKbxkDc=1346366898.960244831398592039831220034197;if (uQWXKDReWIMMMtYHbagOaXWfKbxkDc == uQWXKDReWIMMMtYHbagOaXWfKbxkDc ) uQWXKDReWIMMMtYHbagOaXWfKbxkDc=154100745.906426825814280987652614875132; else uQWXKDReWIMMMtYHbagOaXWfKbxkDc=1319833314.084550227721413805015855649366;if (uQWXKDReWIMMMtYHbagOaXWfKbxkDc == uQWXKDReWIMMMtYHbagOaXWfKbxkDc ) uQWXKDReWIMMMtYHbagOaXWfKbxkDc=1174476495.180483083186003121208123733002; else uQWXKDReWIMMMtYHbagOaXWfKbxkDc=297388017.106265214093293986352927816875;if (uQWXKDReWIMMMtYHbagOaXWfKbxkDc == uQWXKDReWIMMMtYHbagOaXWfKbxkDc ) uQWXKDReWIMMMtYHbagOaXWfKbxkDc=643249759.108219194801832603086886038046; else uQWXKDReWIMMMtYHbagOaXWfKbxkDc=936229646.944999509599255129632480043329;if (uQWXKDReWIMMMtYHbagOaXWfKbxkDc == uQWXKDReWIMMMtYHbagOaXWfKbxkDc ) uQWXKDReWIMMMtYHbagOaXWfKbxkDc=1954723037.125997894310460763201042467506; else uQWXKDReWIMMMtYHbagOaXWfKbxkDc=1044709954.729147437943632213138123828454;if (uQWXKDReWIMMMtYHbagOaXWfKbxkDc == uQWXKDReWIMMMtYHbagOaXWfKbxkDc ) uQWXKDReWIMMMtYHbagOaXWfKbxkDc=1979281400.130870242334646942035447961643; else uQWXKDReWIMMMtYHbagOaXWfKbxkDc=1440236561.018482387318191459798753979401;double cAwOLeObWheYkNnRceZDLuXXQgbXJK=315137634.468082340041999249091344023657;if (cAwOLeObWheYkNnRceZDLuXXQgbXJK == cAwOLeObWheYkNnRceZDLuXXQgbXJK ) cAwOLeObWheYkNnRceZDLuXXQgbXJK=1328452593.830943993608377778195304212406; else cAwOLeObWheYkNnRceZDLuXXQgbXJK=546480521.089322205636649318735657363684;if (cAwOLeObWheYkNnRceZDLuXXQgbXJK == cAwOLeObWheYkNnRceZDLuXXQgbXJK ) cAwOLeObWheYkNnRceZDLuXXQgbXJK=1409615227.259873015668356076591587153307; else cAwOLeObWheYkNnRceZDLuXXQgbXJK=759853938.073259798740418831149719388273;if (cAwOLeObWheYkNnRceZDLuXXQgbXJK == cAwOLeObWheYkNnRceZDLuXXQgbXJK ) cAwOLeObWheYkNnRceZDLuXXQgbXJK=2138934727.071879621839996127042337936498; else cAwOLeObWheYkNnRceZDLuXXQgbXJK=1502588940.152677065932032092437134185504;if (cAwOLeObWheYkNnRceZDLuXXQgbXJK == cAwOLeObWheYkNnRceZDLuXXQgbXJK ) cAwOLeObWheYkNnRceZDLuXXQgbXJK=1538420567.423378833927312130695416213950; else cAwOLeObWheYkNnRceZDLuXXQgbXJK=1154620022.266262977436062092215691002727;if (cAwOLeObWheYkNnRceZDLuXXQgbXJK == cAwOLeObWheYkNnRceZDLuXXQgbXJK ) cAwOLeObWheYkNnRceZDLuXXQgbXJK=591154917.699476116127427056523175577527; else cAwOLeObWheYkNnRceZDLuXXQgbXJK=455369662.026084879608629937415730453491;if (cAwOLeObWheYkNnRceZDLuXXQgbXJK == cAwOLeObWheYkNnRceZDLuXXQgbXJK ) cAwOLeObWheYkNnRceZDLuXXQgbXJK=1543677849.060011144439793655674022240674; else cAwOLeObWheYkNnRceZDLuXXQgbXJK=924916418.248974972980419704890892705887;float bkBLtBpKskrQkvZhDRaVdnpVTagJmt=161896107.365609592690623380170961373769f;if (bkBLtBpKskrQkvZhDRaVdnpVTagJmt - bkBLtBpKskrQkvZhDRaVdnpVTagJmt> 0.00000001 ) bkBLtBpKskrQkvZhDRaVdnpVTagJmt=2078216156.428337062731284520246378340534f; else bkBLtBpKskrQkvZhDRaVdnpVTagJmt=1464946627.684428349733955089786363326109f;if (bkBLtBpKskrQkvZhDRaVdnpVTagJmt - bkBLtBpKskrQkvZhDRaVdnpVTagJmt> 0.00000001 ) bkBLtBpKskrQkvZhDRaVdnpVTagJmt=1244290664.853916886738251442447186932221f; else bkBLtBpKskrQkvZhDRaVdnpVTagJmt=353989123.334009014930316821298477377809f;if (bkBLtBpKskrQkvZhDRaVdnpVTagJmt - bkBLtBpKskrQkvZhDRaVdnpVTagJmt> 0.00000001 ) bkBLtBpKskrQkvZhDRaVdnpVTagJmt=1683759004.467641426354951798012113306883f; else bkBLtBpKskrQkvZhDRaVdnpVTagJmt=1138400760.231567487540695552669080836445f;if (bkBLtBpKskrQkvZhDRaVdnpVTagJmt - bkBLtBpKskrQkvZhDRaVdnpVTagJmt> 0.00000001 ) bkBLtBpKskrQkvZhDRaVdnpVTagJmt=349460642.675423440441268262995422525283f; else bkBLtBpKskrQkvZhDRaVdnpVTagJmt=702997926.104208637311322882268566025390f;if (bkBLtBpKskrQkvZhDRaVdnpVTagJmt - bkBLtBpKskrQkvZhDRaVdnpVTagJmt> 0.00000001 ) bkBLtBpKskrQkvZhDRaVdnpVTagJmt=1507382165.836835716174578520185303574929f; else bkBLtBpKskrQkvZhDRaVdnpVTagJmt=1569449228.334759560818720400860631654437f;if (bkBLtBpKskrQkvZhDRaVdnpVTagJmt - bkBLtBpKskrQkvZhDRaVdnpVTagJmt> 0.00000001 ) bkBLtBpKskrQkvZhDRaVdnpVTagJmt=322277009.199895092917138778328517970311f; else bkBLtBpKskrQkvZhDRaVdnpVTagJmt=2093493189.613372094642806450408622968302f;long lmCTxRohblsUllilIFYJwtFSxxJVEV=451948267;if (lmCTxRohblsUllilIFYJwtFSxxJVEV == lmCTxRohblsUllilIFYJwtFSxxJVEV- 0 ) lmCTxRohblsUllilIFYJwtFSxxJVEV=254530866; else lmCTxRohblsUllilIFYJwtFSxxJVEV=880332791;if (lmCTxRohblsUllilIFYJwtFSxxJVEV == lmCTxRohblsUllilIFYJwtFSxxJVEV- 0 ) lmCTxRohblsUllilIFYJwtFSxxJVEV=1595954182; else lmCTxRohblsUllilIFYJwtFSxxJVEV=2121232596;if (lmCTxRohblsUllilIFYJwtFSxxJVEV == lmCTxRohblsUllilIFYJwtFSxxJVEV- 0 ) lmCTxRohblsUllilIFYJwtFSxxJVEV=994849202; else lmCTxRohblsUllilIFYJwtFSxxJVEV=1706158813;if (lmCTxRohblsUllilIFYJwtFSxxJVEV == lmCTxRohblsUllilIFYJwtFSxxJVEV- 1 ) lmCTxRohblsUllilIFYJwtFSxxJVEV=1573771499; else lmCTxRohblsUllilIFYJwtFSxxJVEV=1112297891;if (lmCTxRohblsUllilIFYJwtFSxxJVEV == lmCTxRohblsUllilIFYJwtFSxxJVEV- 0 ) lmCTxRohblsUllilIFYJwtFSxxJVEV=1955429978; else lmCTxRohblsUllilIFYJwtFSxxJVEV=1146964370;if (lmCTxRohblsUllilIFYJwtFSxxJVEV == lmCTxRohblsUllilIFYJwtFSxxJVEV- 1 ) lmCTxRohblsUllilIFYJwtFSxxJVEV=728678397; else lmCTxRohblsUllilIFYJwtFSxxJVEV=719106833;float pNcdfWZzdIHHAjoSqTYloWPRUhpZDs=165975935.286938247895507442820210385461f;if (pNcdfWZzdIHHAjoSqTYloWPRUhpZDs - pNcdfWZzdIHHAjoSqTYloWPRUhpZDs> 0.00000001 ) pNcdfWZzdIHHAjoSqTYloWPRUhpZDs=1605934940.943196976386976193300508318775f; else pNcdfWZzdIHHAjoSqTYloWPRUhpZDs=1817385293.399705241157664434420316244699f;if (pNcdfWZzdIHHAjoSqTYloWPRUhpZDs - pNcdfWZzdIHHAjoSqTYloWPRUhpZDs> 0.00000001 ) pNcdfWZzdIHHAjoSqTYloWPRUhpZDs=1509504052.831200837195226395498435619366f; else pNcdfWZzdIHHAjoSqTYloWPRUhpZDs=1291655331.276242657571843519196030240669f;if (pNcdfWZzdIHHAjoSqTYloWPRUhpZDs - pNcdfWZzdIHHAjoSqTYloWPRUhpZDs> 0.00000001 ) pNcdfWZzdIHHAjoSqTYloWPRUhpZDs=713402426.257103019474993779310037024710f; else pNcdfWZzdIHHAjoSqTYloWPRUhpZDs=2077258155.200843475051072324768145022118f;if (pNcdfWZzdIHHAjoSqTYloWPRUhpZDs - pNcdfWZzdIHHAjoSqTYloWPRUhpZDs> 0.00000001 ) pNcdfWZzdIHHAjoSqTYloWPRUhpZDs=778862277.941027516030537907581105317592f; else pNcdfWZzdIHHAjoSqTYloWPRUhpZDs=190051833.147722115591873871913664894681f;if (pNcdfWZzdIHHAjoSqTYloWPRUhpZDs - pNcdfWZzdIHHAjoSqTYloWPRUhpZDs> 0.00000001 ) pNcdfWZzdIHHAjoSqTYloWPRUhpZDs=197019879.103509492580209033355668069303f; else pNcdfWZzdIHHAjoSqTYloWPRUhpZDs=706617301.864459948458053346586233754554f;if (pNcdfWZzdIHHAjoSqTYloWPRUhpZDs - pNcdfWZzdIHHAjoSqTYloWPRUhpZDs> 0.00000001 ) pNcdfWZzdIHHAjoSqTYloWPRUhpZDs=1399955420.110973696029590872573393873181f; else pNcdfWZzdIHHAjoSqTYloWPRUhpZDs=569485499.649545918186156875498309292821f;float bCZZpADbcueWXviJTiAyoktMOUANYL=341829849.233991154806237817616539432651f;if (bCZZpADbcueWXviJTiAyoktMOUANYL - bCZZpADbcueWXviJTiAyoktMOUANYL> 0.00000001 ) bCZZpADbcueWXviJTiAyoktMOUANYL=862356074.873737504095988381357970471567f; else bCZZpADbcueWXviJTiAyoktMOUANYL=1091729582.060414450132462265215241591436f;if (bCZZpADbcueWXviJTiAyoktMOUANYL - bCZZpADbcueWXviJTiAyoktMOUANYL> 0.00000001 ) bCZZpADbcueWXviJTiAyoktMOUANYL=670385552.614814568504122561832735392003f; else bCZZpADbcueWXviJTiAyoktMOUANYL=1565693383.413452866971882212903061508062f;if (bCZZpADbcueWXviJTiAyoktMOUANYL - bCZZpADbcueWXviJTiAyoktMOUANYL> 0.00000001 ) bCZZpADbcueWXviJTiAyoktMOUANYL=192792920.267529177081606206318120146206f; else bCZZpADbcueWXviJTiAyoktMOUANYL=1612509712.591472436414592356541275425748f;if (bCZZpADbcueWXviJTiAyoktMOUANYL - bCZZpADbcueWXviJTiAyoktMOUANYL> 0.00000001 ) bCZZpADbcueWXviJTiAyoktMOUANYL=1314530552.498260023487101563890086532715f; else bCZZpADbcueWXviJTiAyoktMOUANYL=372057343.889912744333316427507296942283f;if (bCZZpADbcueWXviJTiAyoktMOUANYL - bCZZpADbcueWXviJTiAyoktMOUANYL> 0.00000001 ) bCZZpADbcueWXviJTiAyoktMOUANYL=2038658775.842971820982067777112521979417f; else bCZZpADbcueWXviJTiAyoktMOUANYL=131992773.056169255670862524028627232201f;if (bCZZpADbcueWXviJTiAyoktMOUANYL - bCZZpADbcueWXviJTiAyoktMOUANYL> 0.00000001 ) bCZZpADbcueWXviJTiAyoktMOUANYL=546042792.242404701862355287945659054093f; else bCZZpADbcueWXviJTiAyoktMOUANYL=536794634.816453846993426638315536770455f;int iDpzdhszIUWDAbHMOQRIejsmMnxmqQ=1789048824;if (iDpzdhszIUWDAbHMOQRIejsmMnxmqQ == iDpzdhszIUWDAbHMOQRIejsmMnxmqQ- 0 ) iDpzdhszIUWDAbHMOQRIejsmMnxmqQ=931762109; else iDpzdhszIUWDAbHMOQRIejsmMnxmqQ=156301074;if (iDpzdhszIUWDAbHMOQRIejsmMnxmqQ == iDpzdhszIUWDAbHMOQRIejsmMnxmqQ- 0 ) iDpzdhszIUWDAbHMOQRIejsmMnxmqQ=487558249; else iDpzdhszIUWDAbHMOQRIejsmMnxmqQ=152244260;if (iDpzdhszIUWDAbHMOQRIejsmMnxmqQ == iDpzdhszIUWDAbHMOQRIejsmMnxmqQ- 0 ) iDpzdhszIUWDAbHMOQRIejsmMnxmqQ=887304663; else iDpzdhszIUWDAbHMOQRIejsmMnxmqQ=448841904;if (iDpzdhszIUWDAbHMOQRIejsmMnxmqQ == iDpzdhszIUWDAbHMOQRIejsmMnxmqQ- 1 ) iDpzdhszIUWDAbHMOQRIejsmMnxmqQ=1886029488; else iDpzdhszIUWDAbHMOQRIejsmMnxmqQ=324756358;if (iDpzdhszIUWDAbHMOQRIejsmMnxmqQ == iDpzdhszIUWDAbHMOQRIejsmMnxmqQ- 1 ) iDpzdhszIUWDAbHMOQRIejsmMnxmqQ=101470514; else iDpzdhszIUWDAbHMOQRIejsmMnxmqQ=1405902349;if (iDpzdhszIUWDAbHMOQRIejsmMnxmqQ == iDpzdhszIUWDAbHMOQRIejsmMnxmqQ- 1 ) iDpzdhszIUWDAbHMOQRIejsmMnxmqQ=1924212717; else iDpzdhszIUWDAbHMOQRIejsmMnxmqQ=288231901;double fqBBuQwycQTlHhHpMkOlpCzWvrBWvR=363018953.416552854454819103949102036879;if (fqBBuQwycQTlHhHpMkOlpCzWvrBWvR == fqBBuQwycQTlHhHpMkOlpCzWvrBWvR ) fqBBuQwycQTlHhHpMkOlpCzWvrBWvR=1961973901.075738361450296576265125441563; else fqBBuQwycQTlHhHpMkOlpCzWvrBWvR=386848548.619559928285350992666161495040;if (fqBBuQwycQTlHhHpMkOlpCzWvrBWvR == fqBBuQwycQTlHhHpMkOlpCzWvrBWvR ) fqBBuQwycQTlHhHpMkOlpCzWvrBWvR=2004234406.881985274766809197655000755826; else fqBBuQwycQTlHhHpMkOlpCzWvrBWvR=1956947880.508850448573800136331940589133;if (fqBBuQwycQTlHhHpMkOlpCzWvrBWvR == fqBBuQwycQTlHhHpMkOlpCzWvrBWvR ) fqBBuQwycQTlHhHpMkOlpCzWvrBWvR=1195431436.156937806985018320063959666947; else fqBBuQwycQTlHhHpMkOlpCzWvrBWvR=1219477383.132553708433156603327027121737;if (fqBBuQwycQTlHhHpMkOlpCzWvrBWvR == fqBBuQwycQTlHhHpMkOlpCzWvrBWvR ) fqBBuQwycQTlHhHpMkOlpCzWvrBWvR=2120666662.107085367673373090307362022643; else fqBBuQwycQTlHhHpMkOlpCzWvrBWvR=2140908875.217144717234786585424143184507;if (fqBBuQwycQTlHhHpMkOlpCzWvrBWvR == fqBBuQwycQTlHhHpMkOlpCzWvrBWvR ) fqBBuQwycQTlHhHpMkOlpCzWvrBWvR=502854741.382446634038428369765722027406; else fqBBuQwycQTlHhHpMkOlpCzWvrBWvR=1137446709.022196344005249783692695267143;if (fqBBuQwycQTlHhHpMkOlpCzWvrBWvR == fqBBuQwycQTlHhHpMkOlpCzWvrBWvR ) fqBBuQwycQTlHhHpMkOlpCzWvrBWvR=1400480203.023812633313576068434492444793; else fqBBuQwycQTlHhHpMkOlpCzWvrBWvR=581409653.920208344815048023931981973158;int vXAQsMOhphHWbLvTYVUNFrdLBYuQTa=566666070;if (vXAQsMOhphHWbLvTYVUNFrdLBYuQTa == vXAQsMOhphHWbLvTYVUNFrdLBYuQTa- 1 ) vXAQsMOhphHWbLvTYVUNFrdLBYuQTa=904813190; else vXAQsMOhphHWbLvTYVUNFrdLBYuQTa=1758390243;if (vXAQsMOhphHWbLvTYVUNFrdLBYuQTa == vXAQsMOhphHWbLvTYVUNFrdLBYuQTa- 1 ) vXAQsMOhphHWbLvTYVUNFrdLBYuQTa=1785207037; else vXAQsMOhphHWbLvTYVUNFrdLBYuQTa=1024010857;if (vXAQsMOhphHWbLvTYVUNFrdLBYuQTa == vXAQsMOhphHWbLvTYVUNFrdLBYuQTa- 0 ) vXAQsMOhphHWbLvTYVUNFrdLBYuQTa=1104852776; else vXAQsMOhphHWbLvTYVUNFrdLBYuQTa=1180229856;if (vXAQsMOhphHWbLvTYVUNFrdLBYuQTa == vXAQsMOhphHWbLvTYVUNFrdLBYuQTa- 1 ) vXAQsMOhphHWbLvTYVUNFrdLBYuQTa=1602008833; else vXAQsMOhphHWbLvTYVUNFrdLBYuQTa=413514170;if (vXAQsMOhphHWbLvTYVUNFrdLBYuQTa == vXAQsMOhphHWbLvTYVUNFrdLBYuQTa- 1 ) vXAQsMOhphHWbLvTYVUNFrdLBYuQTa=1353183741; else vXAQsMOhphHWbLvTYVUNFrdLBYuQTa=345629194;if (vXAQsMOhphHWbLvTYVUNFrdLBYuQTa == vXAQsMOhphHWbLvTYVUNFrdLBYuQTa- 0 ) vXAQsMOhphHWbLvTYVUNFrdLBYuQTa=1464222869; else vXAQsMOhphHWbLvTYVUNFrdLBYuQTa=934374218; }
 vXAQsMOhphHWbLvTYVUNFrdLBYuQTay::vXAQsMOhphHWbLvTYVUNFrdLBYuQTay()
 { this->icNOAtNOAQXz("wfzHAyBvlreCfuWJtxMqfBYjEtoPURicNOAtNOAQXzj", true, 1835780642, 1026298651, 65853950); }
#pragma optimize("", off)
 // <delete/>

