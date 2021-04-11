// dear imgui, v1.69 WIP
// (drawing and font code)

/*

Index of this file:

// [SECTION] STB libraries implementation
// [SECTION] Style functions
// [SECTION] ImDrawList
// [SECTION] ImDrawData
// [SECTION] Helpers ShadeVertsXXX functions
// [SECTION] ImFontConfig
// [SECTION] ImFontAtlas
// [SECTION] ImFontAtlas glyph ranges helpers
// [SECTION] ImFontGlyphRangesBuilder
// [SECTION] ImFont
// [SECTION] Internal Render Helpers
// [SECTION] Decompression code
// [SECTION] Default font data (ProggyClean.ttf)

*/

#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "imgui.h"
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "imgui_internal.h"

#include <stdio.h>      // vsnprintf, sscanf, printf
#if !defined(alloca)
#if defined(__GLIBC__) || defined(__sun) || defined(__CYGWIN__) || defined(__APPLE__)
#include <alloca.h>     // alloca (glibc uses <alloca.h>. Note that Cygwin may have _WIN32 defined, so the order matters here)
#elif defined(_WIN32)
#include <malloc.h>     // alloca
#if !defined(alloca)
#define alloca _alloca  // for clang with MS Codegen
#endif
#else
#include <stdlib.h>     // alloca
#endif
#endif

// Visual Studio warnings
#ifdef _MSC_VER
#pragma warning (disable: 4505) // unreferenced local function has been removed (stb stuff)
#pragma warning (disable: 4996) // 'This function or variable may be unsafe': strcpy, strdup, sprintf, vsnprintf, sscanf, fopen
#endif

// Clang/GCC warnings with -Weverything
#ifdef __clang__
#pragma clang diagnostic ignored "-Wold-style-cast"         // warning : use of old-style cast                              // yes, they are more terse.
#pragma clang diagnostic ignored "-Wfloat-equal"            // warning : comparing floating point with == or != is unsafe   // storing and comparing against same constants ok.
#pragma clang diagnostic ignored "-Wglobal-constructors"    // warning : declaration requires a global destructor           // similar to above, not sure what the exact difference it.
#pragma clang diagnostic ignored "-Wsign-conversion"        // warning : implicit conversion changes signedness             //
#if __has_warning("-Wzero-as-null-pointer-constant")
#pragma clang diagnostic ignored "-Wzero-as-null-pointer-constant"  // warning : zero as null pointer constant              // some standard header variations use #define NULL 0
#endif
#if __has_warning("-Wcomma")
#pragma clang diagnostic ignored "-Wcomma"                  // warning : possible misuse of comma operator here             //
#endif
#if __has_warning("-Wreserved-id-macro")
#pragma clang diagnostic ignored "-Wreserved-id-macro"      // warning : macro name is a reserved identifier                //
#endif
#if __has_warning("-Wdouble-promotion")
#pragma clang diagnostic ignored "-Wdouble-promotion"       // warning: implicit conversion from 'float' to 'double' when passing argument to function  // using printf() is a misery with this as C++ va_arg ellipsis changes float to double.
#endif
#elif defined(__GNUC__)
#pragma GCC diagnostic ignored "-Wunused-function"          // warning: 'xxxx' defined but not used
#pragma GCC diagnostic ignored "-Wdouble-promotion"         // warning: implicit conversion from 'float' to 'double' when passing argument to function
#pragma GCC diagnostic ignored "-Wconversion"               // warning: conversion to 'xxxx' from 'xxxx' may alter its value
#if __GNUC__ >= 8
#pragma GCC diagnostic ignored "-Wclass-memaccess"          // warning: 'memset/memcpy' clearing/writing an object of type 'xxxx' with no trivial copy-assignment; use assignment or value-initialization instead
#endif
#endif

//-------------------------------------------------------------------------
// [SECTION] STB libraries implementation
//-------------------------------------------------------------------------

// Compile time options:
//#define IMGUI_STB_NAMESPACE           ImStb
//#define IMGUI_STB_TRUETYPE_FILENAME   "my_folder/stb_truetype.h"
//#define IMGUI_STB_RECT_PACK_FILENAME  "my_folder/stb_rect_pack.h"
//#define IMGUI_DISABLE_STB_TRUETYPE_IMPLEMENTATION
//#define IMGUI_DISABLE_STB_RECT_PACK_IMPLEMENTATION

#ifdef IMGUI_STB_NAMESPACE
namespace IMGUI_STB_NAMESPACE
{
#endif

#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable: 4456)                             // declaration of 'xx' hides previous local declaration
#endif

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"
#pragma clang diagnostic ignored "-Wmissing-prototypes"
#pragma clang diagnostic ignored "-Wimplicit-fallthrough"
#pragma clang diagnostic ignored "-Wcast-qual"              // warning : cast from 'const xxxx *' to 'xxx *' drops const qualifier //
#endif

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"              // warning: comparison is always true due to limited range of data type [-Wtype-limits]
#pragma GCC diagnostic ignored "-Wcast-qual"                // warning: cast from type 'const xxxx *' to type 'xxxx *' casts away qualifiers
#endif

#ifndef STB_RECT_PACK_IMPLEMENTATION                        // in case the user already have an implementation in the _same_ compilation unit (e.g. unity builds)
#ifndef IMGUI_DISABLE_STB_RECT_PACK_IMPLEMENTATION
#define STBRP_STATIC
#define STBRP_ASSERT(x)     IM_ASSERT(x)
#define STBRP_SORT          ImQsort
#define STB_RECT_PACK_IMPLEMENTATION
#endif
#ifdef IMGUI_STB_RECT_PACK_FILENAME
#include IMGUI_STB_RECT_PACK_FILENAME
#else
#include "imstb_rectpack.h"
#endif
#endif

#ifndef STB_TRUETYPE_IMPLEMENTATION                         // in case the user already have an implementation in the _same_ compilation unit (e.g. unity builds)
#ifndef IMGUI_DISABLE_STB_TRUETYPE_IMPLEMENTATION
#define STBTT_malloc(x,u)   ((void)(u), ImGui::MemAlloc(x))
#define STBTT_free(x,u)     ((void)(u), ImGui::MemFree(x))
#define STBTT_assert(x)     IM_ASSERT(x)
#define STBTT_fmod(x,y)     ImFmod(x,y)
#define STBTT_sqrt(x)       ImSqrt(x)
#define STBTT_pow(x,y)      ImPow(x,y)
#define STBTT_fabs(x)       ImFabs(x)
#define STBTT_ifloor(x)     ((int)ImFloorStd(x))
#define STBTT_iceil(x)      ((int)ImCeil(x))
#define STBTT_STATIC
#define STB_TRUETYPE_IMPLEMENTATION
#else
#define STBTT_DEF extern
#endif
#ifdef IMGUI_STB_TRUETYPE_FILENAME
#include IMGUI_STB_TRUETYPE_FILENAME
#else
#include "imstb_truetype.h"
#endif
#endif

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#ifdef _MSC_VER
#pragma warning (pop)
#endif

#ifdef IMGUI_STB_NAMESPACE
} // namespace ImStb
using namespace IMGUI_STB_NAMESPACE;
#endif

//-----------------------------------------------------------------------------
// [SECTION] Style functions
//-----------------------------------------------------------------------------

ImVec4 Convert(int r, int g, int b, int a)
{
    return ImVec4(r / 255.f, g / 255.f, b / 255.f, a / 255.f);
}

void ImGui::StyleColorsDark(ImGuiStyle* dst)
{
    ImGuiStyle* style = dst ? dst : &ImGui::GetStyle();
    ImVec4* colors = style->Colors;

    style->ItemSpacing.x = 0;
    style->WindowRounding = 3;
    style->ChildRounding = 3;
    style->FrameRounding = 3;
    style->PopupRounding = 3;
    style->GrabRounding = 3;
    style->ScrollbarRounding = 23;
    style->ScrollbarSize = 10;
    style->TabRounding = 3;
    style->WindowBorderSize = 0;
    style->ItemInnerSpacing_new = ImVec2(10, 10);
    style->WindowPadding = ImVec2(10, 10);

    colors[ImGuiCol_Text] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_WindowBg] = Convert(14, 14, 14, 255);
    colors[ImGuiCol_ChildBg] = Convert(18, 18, 18, 255);
    colors[ImGuiCol_PopupBg] = Convert(21, 21, 21, 255);
    colors[ImGuiCol_Border] = Convert(45, 45, 45, 255);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg] = Convert(30, 30, 30, 255);
    colors[ImGuiCol_FrameBgHovered] = Convert(37, 37, 37, 255);
    colors[ImGuiCol_FrameBgActive] = Convert(37, 37, 37, 255);
    colors[ImGuiCol_TitleBg] = Convert(38, 38, 38, 255);
    colors[ImGuiCol_TitleBgActive] = Convert(38, 38, 38, 255);
    colors[ImGuiCol_TitleBgCollapsed] = Convert(38, 38, 38, 255);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_ScrollbarBg] = Convert(40, 40, 40, 0);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
    colors[ImGuiCol_CheckMark] = Convert(255, 56, 56, 255);
    colors[ImGuiCol_SliderGrab] = Convert(255, 77, 77, 255);
    colors[ImGuiCol_SliderGrabActive] = Convert(255, 87, 87, 255);
    colors[ImGuiCol_Button] = Convert(53, 53, 53, 255);
    colors[ImGuiCol_ButtonHovered] = Convert(43, 43, 43, 255);
    colors[ImGuiCol_ButtonActive] = Convert(33, 33, 33, 255);
    colors[ImGuiCol_Header] = Convert(33, 33, 33, 255);
    colors[ImGuiCol_HeaderHovered] = Convert(44, 44, 44, 255);
    colors[ImGuiCol_HeaderActive] = Convert(44, 44, 44, 255);
    colors[ImGuiCol_Separator] = colors[ImGuiCol_Border];
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
    //colors[ImGuiCol_ResizeGrip] = Convert(50, 50, 50, 255);
    //colors[ImGuiCol_ResizeGripHovered] = Convert(45, 45, 45, 255);
    //colors[ImGuiCol_ResizeGripActive] = Convert(40, 40, 40, 255);

    colors[ImGuiCol_ResizeGrip] = Convert(250, 250, 250, 255);
    colors[ImGuiCol_ResizeGripHovered] = Convert(245, 245, 245, 255);
    colors[ImGuiCol_ResizeGripActive] = Convert(240, 240, 240, 255);

    colors[ImGuiCol_Tab] = ImLerp(colors[ImGuiCol_Header], colors[ImGuiCol_TitleBgActive], 0.80f);
    colors[ImGuiCol_TabHovered] = colors[ImGuiCol_HeaderHovered];
    colors[ImGuiCol_TabActive] = ImLerp(colors[ImGuiCol_HeaderActive], colors[ImGuiCol_TitleBgActive], 0.60f);
    colors[ImGuiCol_TabUnfocused] = ImLerp(colors[ImGuiCol_Tab], colors[ImGuiCol_TitleBg], 0.80f);
    colors[ImGuiCol_TabUnfocusedActive] = ImLerp(colors[ImGuiCol_TabActive], colors[ImGuiCol_TitleBg], 0.40f);
    colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
    colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
    colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
}

void ImGui::StyleColorsIndigo(ImGuiStyle* dst)
{
   
}

void ImGui::StyleColorsM0ne0N(ImGuiStyle* dst)
{
    ImGuiStyle* style = dst ? dst : &ImGui::GetStyle();
    ImVec4* colors = style->Colors;

    style->ItemSpacing.x = 0;
    style->WindowRounding = 3;
    style->ChildRounding = 3;
    style->FrameRounding = 3;
    style->PopupRounding = 3;
    style->GrabRounding = 3;
    style->ScrollbarRounding = 23;
    style->ScrollbarSize = 10;
    style->TabRounding = 3;
    style->WindowBorderSize = 0;
    style->ItemInnerSpacing_new = ImVec2(4, 10);
    style->WindowPadding = ImVec2(10, 10);

    colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_WindowBg] = Convert(14, 14, 14, 255);
    colors[ImGuiCol_ChildBg] = Convert(18, 18, 18, 255);
    colors[ImGuiCol_PopupBg] = Convert(21, 21, 21, 255);
    colors[ImGuiCol_Border] = Convert(45, 45, 45, 255);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg] = Convert(30, 30, 30, 255);
    colors[ImGuiCol_FrameBgHovered] = Convert(37, 37, 37, 255);
    colors[ImGuiCol_FrameBgActive] = Convert(37, 37, 37, 255);
    colors[ImGuiCol_TitleBg] = Convert(38, 38, 38, 255);
    colors[ImGuiCol_TitleBgActive] = Convert(38, 38, 38, 255);
    colors[ImGuiCol_TitleBgCollapsed] = Convert(38, 38, 38, 255);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_ScrollbarBg] = Convert(40, 40, 40, 0);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
    colors[ImGuiCol_CheckMark] = Convert(255, 56, 56, 255);
    colors[ImGuiCol_SliderGrab] = Convert(255, 77, 77, 255);
    colors[ImGuiCol_SliderGrabActive] = Convert(255, 87, 87, 255);
    colors[ImGuiCol_Button] = Convert(53, 53, 53, 255);
    colors[ImGuiCol_ButtonHovered] = Convert(43, 43, 43, 255);
    colors[ImGuiCol_ButtonActive] = Convert(33, 33, 33, 255);
    colors[ImGuiCol_Header] = Convert(33, 33, 33, 255);
    colors[ImGuiCol_HeaderHovered] = Convert(44, 44, 44, 255);
    colors[ImGuiCol_HeaderActive] = Convert(44, 44, 44, 255);
    colors[ImGuiCol_Separator] = colors[ImGuiCol_Border];
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
    colors[ImGuiCol_ResizeGrip] = Convert(50, 50, 50, 255);
    colors[ImGuiCol_ResizeGripHovered] = Convert(45, 45, 45, 255);
    colors[ImGuiCol_ResizeGripActive] = Convert(40, 40, 40, 255);
    colors[ImGuiCol_Tab] = ImLerp(colors[ImGuiCol_Header], colors[ImGuiCol_TitleBgActive], 0.80f);
    colors[ImGuiCol_TabHovered] = colors[ImGuiCol_HeaderHovered];
    colors[ImGuiCol_TabActive] = ImLerp(colors[ImGuiCol_HeaderActive], colors[ImGuiCol_TitleBgActive], 0.60f);
    colors[ImGuiCol_TabUnfocused] = ImLerp(colors[ImGuiCol_Tab], colors[ImGuiCol_TitleBg], 0.80f);
    colors[ImGuiCol_TabUnfocusedActive] = ImLerp(colors[ImGuiCol_TabActive], colors[ImGuiCol_TitleBg], 0.40f);
    colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
    colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
    colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
}

//-----------------------------------------------------------------------------
// ImDrawList
//-----------------------------------------------------------------------------

ImDrawListSharedData::ImDrawListSharedData()
{
    Font = NULL;
    FontSize = 0.0f;
    CurveTessellationTol = 0.0f;
    ClipRectFullscreen = ImVec4(-8192.0f, -8192.0f, +8192.0f, +8192.0f);

    // Const data
    for (int i = 0; i < IM_ARRAYSIZE(CircleVtx12); i++)
    {
        const float a = ((float)i * 2 * IM_PI) / (float)IM_ARRAYSIZE(CircleVtx12);
        CircleVtx12[i] = ImVec2(ImCos(a), ImSin(a));
    }
}

void ImDrawList::Clear()
{
    CmdBuffer.resize(0);
    IdxBuffer.resize(0);
    VtxBuffer.resize(0);
    Flags = ImDrawListFlags_AntiAliasedLines | ImDrawListFlags_AntiAliasedFill;
    _VtxCurrentIdx = 0;
    _VtxWritePtr = NULL;
    _IdxWritePtr = NULL;
    _ClipRectStack.resize(0);
    _TextureIdStack.resize(0);
    _Path.resize(0);
    _ChannelsCurrent = 0;
    _ChannelsCount = 1;
    // NB: Do not clear channels so our allocations are re-used after the first frame.
}

void ImDrawList::ClearFreeMemory()
{
    CmdBuffer.clear();
    IdxBuffer.clear();
    VtxBuffer.clear();
    _VtxCurrentIdx = 0;
    _VtxWritePtr = NULL;
    _IdxWritePtr = NULL;
    _ClipRectStack.clear();
    _TextureIdStack.clear();
    _Path.clear();
    _ChannelsCurrent = 0;
    _ChannelsCount = 1;
    for (int i = 0; i < _Channels.Size; i++)
    {
        if (i == 0) memset(&_Channels[0], 0, sizeof(_Channels[0]));  // channel 0 is a copy of CmdBuffer/IdxBuffer, don't destruct again
        _Channels[i].CmdBuffer.clear();
        _Channels[i].IdxBuffer.clear();
    }
    _Channels.clear();
}

ImDrawList* ImDrawList::CloneOutput() const
{
    ImDrawList* dst = IM_NEW(ImDrawList(NULL));
    dst->CmdBuffer = CmdBuffer;
    dst->IdxBuffer = IdxBuffer;
    dst->VtxBuffer = VtxBuffer;
    dst->Flags = Flags;
    return dst;
}

// Using macros because C++ is a terrible language, we want guaranteed inline, no code in header, and no overhead in Debug builds
#define GetCurrentClipRect()    (_ClipRectStack.Size ? _ClipRectStack.Data[_ClipRectStack.Size-1]  : _Data->ClipRectFullscreen)
#define GetCurrentTextureId()   (_TextureIdStack.Size ? _TextureIdStack.Data[_TextureIdStack.Size-1] : NULL)

void ImDrawList::AddDrawCmd()
{
    ImDrawCmd draw_cmd;
    draw_cmd.ClipRect = GetCurrentClipRect();
    draw_cmd.TextureId = GetCurrentTextureId();

    IM_ASSERT(draw_cmd.ClipRect.x <= draw_cmd.ClipRect.z && draw_cmd.ClipRect.y <= draw_cmd.ClipRect.w);
    CmdBuffer.push_back(draw_cmd);
}

void ImDrawList::AddCallback(ImDrawCallback callback, void* callback_data)
{
    ImDrawCmd* current_cmd = CmdBuffer.Size ? &CmdBuffer.back() : NULL;
    if (!current_cmd || current_cmd->ElemCount != 0 || current_cmd->UserCallback != NULL)
    {
        AddDrawCmd();
        current_cmd = &CmdBuffer.back();
    }
    current_cmd->UserCallback = callback;
    current_cmd->UserCallbackData = callback_data;

    AddDrawCmd(); // Force a new command after us (see comment below)
}

// Our scheme may appears a bit unusual, basically we want the most-common calls AddLine AddRect etc. to not have to perform any check so we always have a command ready in the stack.
// The cost of figuring out if a new command has to be added or if we can merge is paid in those Update** functions only.
void ImDrawList::UpdateClipRect()
{
    // If current command is used with different settings we need to add a new command
    const ImVec4 curr_clip_rect = GetCurrentClipRect();
    ImDrawCmd* curr_cmd = CmdBuffer.Size > 0 ? &CmdBuffer.Data[CmdBuffer.Size-1] : NULL;
    if (!curr_cmd || (curr_cmd->ElemCount != 0 && memcmp(&curr_cmd->ClipRect, &curr_clip_rect, sizeof(ImVec4)) != 0) || curr_cmd->UserCallback != NULL)
    {
        AddDrawCmd();
        return;
    }

    // Try to merge with previous command if it matches, else use current command
    ImDrawCmd* prev_cmd = CmdBuffer.Size > 1 ? curr_cmd - 1 : NULL;
    if (curr_cmd->ElemCount == 0 && prev_cmd && memcmp(&prev_cmd->ClipRect, &curr_clip_rect, sizeof(ImVec4)) == 0 && prev_cmd->TextureId == GetCurrentTextureId() && prev_cmd->UserCallback == NULL)
        CmdBuffer.pop_back();
    else
        curr_cmd->ClipRect = curr_clip_rect;
}

void ImDrawList::UpdateTextureID()
{
    // If current command is used with different settings we need to add a new command
    const ImTextureID curr_texture_id = GetCurrentTextureId();
    ImDrawCmd* curr_cmd = CmdBuffer.Size ? &CmdBuffer.back() : NULL;
    if (!curr_cmd || (curr_cmd->ElemCount != 0 && curr_cmd->TextureId != curr_texture_id) || curr_cmd->UserCallback != NULL)
    {
        AddDrawCmd();
        return;
    }

    // Try to merge with previous command if it matches, else use current command
    ImDrawCmd* prev_cmd = CmdBuffer.Size > 1 ? curr_cmd - 1 : NULL;
    if (curr_cmd->ElemCount == 0 && prev_cmd && prev_cmd->TextureId == curr_texture_id && memcmp(&prev_cmd->ClipRect, &GetCurrentClipRect(), sizeof(ImVec4)) == 0 && prev_cmd->UserCallback == NULL)
        CmdBuffer.pop_back();
    else
        curr_cmd->TextureId = curr_texture_id;
}

#undef GetCurrentClipRect
#undef GetCurrentTextureId

// Render-level scissoring. This is passed down to your render function but not used for CPU-side coarse clipping. Prefer using higher-level ImGui::PushClipRect() to affect logic (hit-testing and widget culling)
void ImDrawList::PushClipRect(ImVec2 cr_min, ImVec2 cr_max, bool intersect_with_current_clip_rect)
{
    ImVec4 cr(cr_min.x, cr_min.y, cr_max.x, cr_max.y);
    if (intersect_with_current_clip_rect && _ClipRectStack.Size)
    {
        ImVec4 current = _ClipRectStack.Data[_ClipRectStack.Size-1];
        if (cr.x < current.x) cr.x = current.x;
        if (cr.y < current.y) cr.y = current.y;
        if (cr.z > current.z) cr.z = current.z;
        if (cr.w > current.w) cr.w = current.w;
    }
    cr.z = ImMax(cr.x, cr.z);
    cr.w = ImMax(cr.y, cr.w);

    _ClipRectStack.push_back(cr);
    UpdateClipRect();
}

void ImDrawList::PushClipRectFullScreen()
{
    PushClipRect(ImVec2(_Data->ClipRectFullscreen.x, _Data->ClipRectFullscreen.y), ImVec2(_Data->ClipRectFullscreen.z, _Data->ClipRectFullscreen.w));
}

void ImDrawList::PopClipRect()
{
    IM_ASSERT(_ClipRectStack.Size > 0);
    _ClipRectStack.pop_back();
    UpdateClipRect();
}

void ImDrawList::PushTextureID(ImTextureID texture_id)
{
    _TextureIdStack.push_back(texture_id);
    UpdateTextureID();
}

void ImDrawList::PopTextureID()
{
    IM_ASSERT(_TextureIdStack.Size > 0);
    _TextureIdStack.pop_back();
    UpdateTextureID();
}

void ImDrawList::ChannelsSplit(int channels_count)
{
    IM_ASSERT(_ChannelsCurrent == 0 && _ChannelsCount == 1);
    int old_channels_count = _Channels.Size;
    if (old_channels_count < channels_count)
        _Channels.resize(channels_count);
    _ChannelsCount = channels_count;

    // _Channels[] (24/32 bytes each) hold storage that we'll swap with this->_CmdBuffer/_IdxBuffer
    // The content of _Channels[0] at this point doesn't matter. We clear it to make state tidy in a debugger but we don't strictly need to.
    // When we switch to the next channel, we'll copy _CmdBuffer/_IdxBuffer into _Channels[0] and then _Channels[1] into _CmdBuffer/_IdxBuffer
    memset(&_Channels[0], 0, sizeof(ImDrawChannel));
    for (int i = 1; i < channels_count; i++)
    {
        if (i >= old_channels_count)
        {
            IM_PLACEMENT_NEW(&_Channels[i]) ImDrawChannel();
        }
        else
        {
            _Channels[i].CmdBuffer.resize(0);
            _Channels[i].IdxBuffer.resize(0);
        }
        if (_Channels[i].CmdBuffer.Size == 0)
        {
            ImDrawCmd draw_cmd;
            draw_cmd.ClipRect = _ClipRectStack.back();
            draw_cmd.TextureId = _TextureIdStack.back();
            _Channels[i].CmdBuffer.push_back(draw_cmd);
        }
    }
}

void ImDrawList::ChannelsMerge()
{
    // Note that we never use or rely on channels.Size because it is merely a buffer that we never shrink back to 0 to keep all sub-buffers ready for use.
    if (_ChannelsCount <= 1)
        return;

    ChannelsSetCurrent(0);
    if (CmdBuffer.Size && CmdBuffer.back().ElemCount == 0)
        CmdBuffer.pop_back();

    int new_cmd_buffer_count = 0, new_idx_buffer_count = 0;
    for (int i = 1; i < _ChannelsCount; i++)
    {
        ImDrawChannel& ch = _Channels[i];
        if (ch.CmdBuffer.Size && ch.CmdBuffer.back().ElemCount == 0)
            ch.CmdBuffer.pop_back();
        new_cmd_buffer_count += ch.CmdBuffer.Size;
        new_idx_buffer_count += ch.IdxBuffer.Size;
    }
    CmdBuffer.resize(CmdBuffer.Size + new_cmd_buffer_count);
    IdxBuffer.resize(IdxBuffer.Size + new_idx_buffer_count);

    ImDrawCmd* cmd_write = CmdBuffer.Data + CmdBuffer.Size - new_cmd_buffer_count;
    _IdxWritePtr = IdxBuffer.Data + IdxBuffer.Size - new_idx_buffer_count;
    for (int i = 1; i < _ChannelsCount; i++)
    {
        ImDrawChannel& ch = _Channels[i];
        if (int sz = ch.CmdBuffer.Size) { memcpy(cmd_write, ch.CmdBuffer.Data, sz * sizeof(ImDrawCmd)); cmd_write += sz; }
        if (int sz = ch.IdxBuffer.Size) { memcpy(_IdxWritePtr, ch.IdxBuffer.Data, sz * sizeof(ImDrawIdx)); _IdxWritePtr += sz; }
    }
    UpdateClipRect(); // We call this instead of AddDrawCmd(), so that empty channels won't produce an extra draw call.
    _ChannelsCount = 1;
}

void ImDrawList::ChannelsSetCurrent(int idx)
{
    IM_ASSERT(idx < _ChannelsCount);
    if (_ChannelsCurrent == idx) return;
    memcpy(&_Channels.Data[_ChannelsCurrent].CmdBuffer, &CmdBuffer, sizeof(CmdBuffer)); // copy 12 bytes, four times
    memcpy(&_Channels.Data[_ChannelsCurrent].IdxBuffer, &IdxBuffer, sizeof(IdxBuffer));
    _ChannelsCurrent = idx;
    memcpy(&CmdBuffer, &_Channels.Data[_ChannelsCurrent].CmdBuffer, sizeof(CmdBuffer));
    memcpy(&IdxBuffer, &_Channels.Data[_ChannelsCurrent].IdxBuffer, sizeof(IdxBuffer));
    _IdxWritePtr = IdxBuffer.Data + IdxBuffer.Size;
}

// NB: this can be called with negative count for removing primitives (as long as the result does not underflow)
void ImDrawList::PrimReserve(int idx_count, int vtx_count)
{
    ImDrawCmd& draw_cmd = CmdBuffer.Data[CmdBuffer.Size-1];
    draw_cmd.ElemCount += idx_count;

    int vtx_buffer_old_size = VtxBuffer.Size;
    VtxBuffer.resize(vtx_buffer_old_size + vtx_count);
    _VtxWritePtr = VtxBuffer.Data + vtx_buffer_old_size;

    int idx_buffer_old_size = IdxBuffer.Size;
    IdxBuffer.resize(idx_buffer_old_size + idx_count);
    _IdxWritePtr = IdxBuffer.Data + idx_buffer_old_size;
}

// Fully unrolled with call to keep our debug builds decently fast.
void ImDrawList::PrimRect(const ImVec2& a, const ImVec2& c, ImU32 col)
{
    ImVec2 b(c.x, a.y), d(a.x, c.y), uv(_Data->TexUvWhitePixel);
    ImDrawIdx idx = (ImDrawIdx)_VtxCurrentIdx;
    _IdxWritePtr[0] = idx; _IdxWritePtr[1] = (ImDrawIdx)(idx+1); _IdxWritePtr[2] = (ImDrawIdx)(idx+2);
    _IdxWritePtr[3] = idx; _IdxWritePtr[4] = (ImDrawIdx)(idx+2); _IdxWritePtr[5] = (ImDrawIdx)(idx+3);
    _VtxWritePtr[0].pos = a; _VtxWritePtr[0].uv = uv; _VtxWritePtr[0].col = col;
    _VtxWritePtr[1].pos = b; _VtxWritePtr[1].uv = uv; _VtxWritePtr[1].col = col;
    _VtxWritePtr[2].pos = c; _VtxWritePtr[2].uv = uv; _VtxWritePtr[2].col = col;
    _VtxWritePtr[3].pos = d; _VtxWritePtr[3].uv = uv; _VtxWritePtr[3].col = col;
    _VtxWritePtr += 4;
    _VtxCurrentIdx += 4;
    _IdxWritePtr += 6;
}

void ImDrawList::PrimRectUV(const ImVec2& a, const ImVec2& c, const ImVec2& uv_a, const ImVec2& uv_c, ImU32 col)
{
    ImVec2 b(c.x, a.y), d(a.x, c.y), uv_b(uv_c.x, uv_a.y), uv_d(uv_a.x, uv_c.y);
    ImDrawIdx idx = (ImDrawIdx)_VtxCurrentIdx;
    _IdxWritePtr[0] = idx; _IdxWritePtr[1] = (ImDrawIdx)(idx+1); _IdxWritePtr[2] = (ImDrawIdx)(idx+2);
    _IdxWritePtr[3] = idx; _IdxWritePtr[4] = (ImDrawIdx)(idx+2); _IdxWritePtr[5] = (ImDrawIdx)(idx+3);
    _VtxWritePtr[0].pos = a; _VtxWritePtr[0].uv = uv_a; _VtxWritePtr[0].col = col;
    _VtxWritePtr[1].pos = b; _VtxWritePtr[1].uv = uv_b; _VtxWritePtr[1].col = col;
    _VtxWritePtr[2].pos = c; _VtxWritePtr[2].uv = uv_c; _VtxWritePtr[2].col = col;
    _VtxWritePtr[3].pos = d; _VtxWritePtr[3].uv = uv_d; _VtxWritePtr[3].col = col;
    _VtxWritePtr += 4;
    _VtxCurrentIdx += 4;
    _IdxWritePtr += 6;
}

void ImDrawList::PrimQuadUV(const ImVec2& a, const ImVec2& b, const ImVec2& c, const ImVec2& d, const ImVec2& uv_a, const ImVec2& uv_b, const ImVec2& uv_c, const ImVec2& uv_d, ImU32 col)
{
    ImDrawIdx idx = (ImDrawIdx)_VtxCurrentIdx;
    _IdxWritePtr[0] = idx; _IdxWritePtr[1] = (ImDrawIdx)(idx+1); _IdxWritePtr[2] = (ImDrawIdx)(idx+2);
    _IdxWritePtr[3] = idx; _IdxWritePtr[4] = (ImDrawIdx)(idx+2); _IdxWritePtr[5] = (ImDrawIdx)(idx+3);
    _VtxWritePtr[0].pos = a; _VtxWritePtr[0].uv = uv_a; _VtxWritePtr[0].col = col;
    _VtxWritePtr[1].pos = b; _VtxWritePtr[1].uv = uv_b; _VtxWritePtr[1].col = col;
    _VtxWritePtr[2].pos = c; _VtxWritePtr[2].uv = uv_c; _VtxWritePtr[2].col = col;
    _VtxWritePtr[3].pos = d; _VtxWritePtr[3].uv = uv_d; _VtxWritePtr[3].col = col;
    _VtxWritePtr += 4;
    _VtxCurrentIdx += 4;
    _IdxWritePtr += 6;
}

// On AddPolyline() and AddConvexPolyFilled() we intentionally avoid using ImVec2 and superflous function calls to optimize debug/non-inlined builds.
// Those macros expects l-values.
#define IM_NORMALIZE2F_OVER_ZERO(VX,VY)                         { float d2 = VX*VX + VY*VY; if (d2 > 0.0f) { float inv_len = 1.0f / ImSqrt(d2); VX *= inv_len; VY *= inv_len; } }
#define IM_NORMALIZE2F_OVER_EPSILON_CLAMP(VX,VY,EPS,INVLENMAX)  { float d2 = VX*VX + VY*VY; if (d2 > EPS)  { float inv_len = 1.0f / ImSqrt(d2); if (inv_len > INVLENMAX) inv_len = INVLENMAX; VX *= inv_len; VY *= inv_len; } }

// TODO: Thickness anti-aliased lines cap are missing their AA fringe.
// We avoid using the ImVec2 math operators here to reduce cost to a minimum for debug/non-inlined builds.
void ImDrawList::AddPolyline(const ImVec2* points, const int points_count, ImU32 col, bool closed, float thickness)
{
    if (points_count < 2)
        return;

    const ImVec2 uv = _Data->TexUvWhitePixel;

    int count = points_count;
    if (!closed)
        count = points_count-1;

    const bool thick_line = thickness > 1.0f;
    if (Flags & ImDrawListFlags_AntiAliasedLines)
    {
        // Anti-aliased stroke
        const float AA_SIZE = 1.0f;
        const ImU32 col_trans = col & ~IM_COL32_A_MASK;

        const int idx_count = thick_line ? count*18 : count*12;
        const int vtx_count = thick_line ? points_count*4 : points_count*3;
        PrimReserve(idx_count, vtx_count);

        // Temporary buffer
        ImVec2* temp_normals = (ImVec2*)alloca(points_count * (thick_line ? 5 : 3) * sizeof(ImVec2)); //-V630
        ImVec2* temp_points = temp_normals + points_count;

        for (int i1 = 0; i1 < count; i1++)
        {
            const int i2 = (i1+1) == points_count ? 0 : i1+1;
            float dx = points[i2].x - points[i1].x;
            float dy = points[i2].y - points[i1].y;
            IM_NORMALIZE2F_OVER_ZERO(dx, dy);
            temp_normals[i1].x = dy;
            temp_normals[i1].y = -dx;
        }
        if (!closed)
            temp_normals[points_count-1] = temp_normals[points_count-2];

        if (!thick_line)
        {
            if (!closed)
            {
                temp_points[0] = points[0] + temp_normals[0] * AA_SIZE;
                temp_points[1] = points[0] - temp_normals[0] * AA_SIZE;
                temp_points[(points_count-1)*2+0] = points[points_count-1] + temp_normals[points_count-1] * AA_SIZE;
                temp_points[(points_count-1)*2+1] = points[points_count-1] - temp_normals[points_count-1] * AA_SIZE;
            }

            // FIXME-OPT: Merge the different loops, possibly remove the temporary buffer.
            unsigned int idx1 = _VtxCurrentIdx;
            for (int i1 = 0; i1 < count; i1++)
            {
                const int i2 = (i1+1) == points_count ? 0 : i1+1;
                unsigned int idx2 = (i1+1) == points_count ? _VtxCurrentIdx : idx1+3;

                // Average normals
                float dm_x = (temp_normals[i1].x + temp_normals[i2].x) * 0.5f;
                float dm_y = (temp_normals[i1].y + temp_normals[i2].y) * 0.5f;
                IM_NORMALIZE2F_OVER_EPSILON_CLAMP(dm_x, dm_y, 0.000001f, 100.0f)
                dm_x *= AA_SIZE;
                dm_y *= AA_SIZE;

                // Add temporary vertexes
                ImVec2* out_vtx = &temp_points[i2*2];
                out_vtx[0].x = points[i2].x + dm_x;
                out_vtx[0].y = points[i2].y + dm_y;
                out_vtx[1].x = points[i2].x - dm_x;
                out_vtx[1].y = points[i2].y - dm_y;

                // Add indexes
                _IdxWritePtr[0] = (ImDrawIdx)(idx2+0); _IdxWritePtr[1] = (ImDrawIdx)(idx1+0); _IdxWritePtr[2] = (ImDrawIdx)(idx1+2);
                _IdxWritePtr[3] = (ImDrawIdx)(idx1+2); _IdxWritePtr[4] = (ImDrawIdx)(idx2+2); _IdxWritePtr[5] = (ImDrawIdx)(idx2+0);
                _IdxWritePtr[6] = (ImDrawIdx)(idx2+1); _IdxWritePtr[7] = (ImDrawIdx)(idx1+1); _IdxWritePtr[8] = (ImDrawIdx)(idx1+0);
                _IdxWritePtr[9] = (ImDrawIdx)(idx1+0); _IdxWritePtr[10]= (ImDrawIdx)(idx2+0); _IdxWritePtr[11]= (ImDrawIdx)(idx2+1);
                _IdxWritePtr += 12;

                idx1 = idx2;
            }

            // Add vertexes
            for (int i = 0; i < points_count; i++)
            {
                _VtxWritePtr[0].pos = points[i];          _VtxWritePtr[0].uv = uv; _VtxWritePtr[0].col = col;
                _VtxWritePtr[1].pos = temp_points[i*2+0]; _VtxWritePtr[1].uv = uv; _VtxWritePtr[1].col = col_trans;
                _VtxWritePtr[2].pos = temp_points[i*2+1]; _VtxWritePtr[2].uv = uv; _VtxWritePtr[2].col = col_trans;
                _VtxWritePtr += 3;
            }
        }
        else
        {
            const float half_inner_thickness = (thickness - AA_SIZE) * 0.5f;
            if (!closed)
            {
                temp_points[0] = points[0] + temp_normals[0] * (half_inner_thickness + AA_SIZE);
                temp_points[1] = points[0] + temp_normals[0] * (half_inner_thickness);
                temp_points[2] = points[0] - temp_normals[0] * (half_inner_thickness);
                temp_points[3] = points[0] - temp_normals[0] * (half_inner_thickness + AA_SIZE);
                temp_points[(points_count-1)*4+0] = points[points_count-1] + temp_normals[points_count-1] * (half_inner_thickness + AA_SIZE);
                temp_points[(points_count-1)*4+1] = points[points_count-1] + temp_normals[points_count-1] * (half_inner_thickness);
                temp_points[(points_count-1)*4+2] = points[points_count-1] - temp_normals[points_count-1] * (half_inner_thickness);
                temp_points[(points_count-1)*4+3] = points[points_count-1] - temp_normals[points_count-1] * (half_inner_thickness + AA_SIZE);
            }

            // FIXME-OPT: Merge the different loops, possibly remove the temporary buffer.
            unsigned int idx1 = _VtxCurrentIdx;
            for (int i1 = 0; i1 < count; i1++)
            {
                const int i2 = (i1+1) == points_count ? 0 : i1+1;
                unsigned int idx2 = (i1+1) == points_count ? _VtxCurrentIdx : idx1+4;

                // Average normals
                float dm_x = (temp_normals[i1].x + temp_normals[i2].x) * 0.5f;
                float dm_y = (temp_normals[i1].y + temp_normals[i2].y) * 0.5f;
                IM_NORMALIZE2F_OVER_EPSILON_CLAMP(dm_x, dm_y, 0.000001f, 100.0f);
                float dm_out_x = dm_x * (half_inner_thickness + AA_SIZE);
                float dm_out_y = dm_y * (half_inner_thickness + AA_SIZE);
                float dm_in_x = dm_x * half_inner_thickness;
                float dm_in_y = dm_y * half_inner_thickness;

                // Add temporary vertexes
                ImVec2* out_vtx = &temp_points[i2*4];
                out_vtx[0].x = points[i2].x + dm_out_x;
                out_vtx[0].y = points[i2].y + dm_out_y;
                out_vtx[1].x = points[i2].x + dm_in_x;
                out_vtx[1].y = points[i2].y + dm_in_y;
                out_vtx[2].x = points[i2].x - dm_in_x;
                out_vtx[2].y = points[i2].y - dm_in_y;
                out_vtx[3].x = points[i2].x - dm_out_x;
                out_vtx[3].y = points[i2].y - dm_out_y;

                // Add indexes
                _IdxWritePtr[0]  = (ImDrawIdx)(idx2+1); _IdxWritePtr[1]  = (ImDrawIdx)(idx1+1); _IdxWritePtr[2]  = (ImDrawIdx)(idx1+2);
                _IdxWritePtr[3]  = (ImDrawIdx)(idx1+2); _IdxWritePtr[4]  = (ImDrawIdx)(idx2+2); _IdxWritePtr[5]  = (ImDrawIdx)(idx2+1);
                _IdxWritePtr[6]  = (ImDrawIdx)(idx2+1); _IdxWritePtr[7]  = (ImDrawIdx)(idx1+1); _IdxWritePtr[8]  = (ImDrawIdx)(idx1+0);
                _IdxWritePtr[9]  = (ImDrawIdx)(idx1+0); _IdxWritePtr[10] = (ImDrawIdx)(idx2+0); _IdxWritePtr[11] = (ImDrawIdx)(idx2+1);
                _IdxWritePtr[12] = (ImDrawIdx)(idx2+2); _IdxWritePtr[13] = (ImDrawIdx)(idx1+2); _IdxWritePtr[14] = (ImDrawIdx)(idx1+3);
                _IdxWritePtr[15] = (ImDrawIdx)(idx1+3); _IdxWritePtr[16] = (ImDrawIdx)(idx2+3); _IdxWritePtr[17] = (ImDrawIdx)(idx2+2);
                _IdxWritePtr += 18;

                idx1 = idx2;
            }

            // Add vertexes
            for (int i = 0; i < points_count; i++)
            {
                _VtxWritePtr[0].pos = temp_points[i*4+0]; _VtxWritePtr[0].uv = uv; _VtxWritePtr[0].col = col_trans;
                _VtxWritePtr[1].pos = temp_points[i*4+1]; _VtxWritePtr[1].uv = uv; _VtxWritePtr[1].col = col;
                _VtxWritePtr[2].pos = temp_points[i*4+2]; _VtxWritePtr[2].uv = uv; _VtxWritePtr[2].col = col;
                _VtxWritePtr[3].pos = temp_points[i*4+3]; _VtxWritePtr[3].uv = uv; _VtxWritePtr[3].col = col_trans;
                _VtxWritePtr += 4;
            }
        }
        _VtxCurrentIdx += (ImDrawIdx)vtx_count;
    }
    else
    {
        // Non Anti-aliased Stroke
        const int idx_count = count*6;
        const int vtx_count = count*4;      // FIXME-OPT: Not sharing edges
        PrimReserve(idx_count, vtx_count);

        for (int i1 = 0; i1 < count; i1++)
        {
            const int i2 = (i1+1) == points_count ? 0 : i1+1;
            const ImVec2& p1 = points[i1];
            const ImVec2& p2 = points[i2];

            float dx = p2.x - p1.x;
            float dy = p2.y - p1.y;
            IM_NORMALIZE2F_OVER_ZERO(dx, dy);
            dx *= (thickness * 0.5f);
            dy *= (thickness * 0.5f);

            _VtxWritePtr[0].pos.x = p1.x + dy; _VtxWritePtr[0].pos.y = p1.y - dx; _VtxWritePtr[0].uv = uv; _VtxWritePtr[0].col = col;
            _VtxWritePtr[1].pos.x = p2.x + dy; _VtxWritePtr[1].pos.y = p2.y - dx; _VtxWritePtr[1].uv = uv; _VtxWritePtr[1].col = col;
            _VtxWritePtr[2].pos.x = p2.x - dy; _VtxWritePtr[2].pos.y = p2.y + dx; _VtxWritePtr[2].uv = uv; _VtxWritePtr[2].col = col;
            _VtxWritePtr[3].pos.x = p1.x - dy; _VtxWritePtr[3].pos.y = p1.y + dx; _VtxWritePtr[3].uv = uv; _VtxWritePtr[3].col = col;
            _VtxWritePtr += 4;

            _IdxWritePtr[0] = (ImDrawIdx)(_VtxCurrentIdx); _IdxWritePtr[1] = (ImDrawIdx)(_VtxCurrentIdx+1); _IdxWritePtr[2] = (ImDrawIdx)(_VtxCurrentIdx+2);
            _IdxWritePtr[3] = (ImDrawIdx)(_VtxCurrentIdx); _IdxWritePtr[4] = (ImDrawIdx)(_VtxCurrentIdx+2); _IdxWritePtr[5] = (ImDrawIdx)(_VtxCurrentIdx+3);
            _IdxWritePtr += 6;
            _VtxCurrentIdx += 4;
        }
    }
}

// We intentionally avoid using ImVec2 and its math operators here to reduce cost to a minimum for debug/non-inlined builds.
void ImDrawList::AddConvexPolyFilled(const ImVec2* points, const int points_count, ImU32 col)
{
    if (points_count < 3)
        return;

    const ImVec2 uv = _Data->TexUvWhitePixel;

    if (Flags & ImDrawListFlags_AntiAliasedFill)
    {
        // Anti-aliased Fill
        const float AA_SIZE = 1.0f;
        const ImU32 col_trans = col & ~IM_COL32_A_MASK;
        const int idx_count = (points_count-2)*3 + points_count*6;
        const int vtx_count = (points_count*2);
        PrimReserve(idx_count, vtx_count);

        // Add indexes for fill
        unsigned int vtx_inner_idx = _VtxCurrentIdx;
        unsigned int vtx_outer_idx = _VtxCurrentIdx+1;
        for (int i = 2; i < points_count; i++)
        {
            _IdxWritePtr[0] = (ImDrawIdx)(vtx_inner_idx); _IdxWritePtr[1] = (ImDrawIdx)(vtx_inner_idx+((i-1)<<1)); _IdxWritePtr[2] = (ImDrawIdx)(vtx_inner_idx+(i<<1));
            _IdxWritePtr += 3;
        }

        // Compute normals
        ImVec2* temp_normals = (ImVec2*)alloca(points_count * sizeof(ImVec2)); //-V630
        for (int i0 = points_count-1, i1 = 0; i1 < points_count; i0 = i1++)
        {
            const ImVec2& p0 = points[i0];
            const ImVec2& p1 = points[i1];
            float dx = p1.x - p0.x;
            float dy = p1.y - p0.y;
            IM_NORMALIZE2F_OVER_ZERO(dx, dy);
            temp_normals[i0].x = dy;
            temp_normals[i0].y = -dx;
        }

        for (int i0 = points_count-1, i1 = 0; i1 < points_count; i0 = i1++)
        {
            // Average normals
            const ImVec2& n0 = temp_normals[i0];
            const ImVec2& n1 = temp_normals[i1];
            float dm_x = (n0.x + n1.x) * 0.5f;
            float dm_y = (n0.y + n1.y) * 0.5f;
            IM_NORMALIZE2F_OVER_EPSILON_CLAMP(dm_x, dm_y, 0.000001f, 100.0f);
            dm_x *= AA_SIZE * 0.5f;
            dm_y *= AA_SIZE * 0.5f;

            // Add vertices
            _VtxWritePtr[0].pos.x = (points[i1].x - dm_x); _VtxWritePtr[0].pos.y = (points[i1].y - dm_y); _VtxWritePtr[0].uv = uv; _VtxWritePtr[0].col = col;        // Inner
            _VtxWritePtr[1].pos.x = (points[i1].x + dm_x); _VtxWritePtr[1].pos.y = (points[i1].y + dm_y); _VtxWritePtr[1].uv = uv; _VtxWritePtr[1].col = col_trans;  // Outer
            _VtxWritePtr += 2;

            // Add indexes for fringes
            _IdxWritePtr[0] = (ImDrawIdx)(vtx_inner_idx+(i1<<1)); _IdxWritePtr[1] = (ImDrawIdx)(vtx_inner_idx+(i0<<1)); _IdxWritePtr[2] = (ImDrawIdx)(vtx_outer_idx+(i0<<1));
            _IdxWritePtr[3] = (ImDrawIdx)(vtx_outer_idx+(i0<<1)); _IdxWritePtr[4] = (ImDrawIdx)(vtx_outer_idx+(i1<<1)); _IdxWritePtr[5] = (ImDrawIdx)(vtx_inner_idx+(i1<<1));
            _IdxWritePtr += 6;
        }
        _VtxCurrentIdx += (ImDrawIdx)vtx_count;
    }
    else
    {
        // Non Anti-aliased Fill
        const int idx_count = (points_count-2)*3;
        const int vtx_count = points_count;
        PrimReserve(idx_count, vtx_count);
        for (int i = 0; i < vtx_count; i++)
        {
            _VtxWritePtr[0].pos = points[i]; _VtxWritePtr[0].uv = uv; _VtxWritePtr[0].col = col;
            _VtxWritePtr++;
        }
        for (int i = 2; i < points_count; i++)
        {
            _IdxWritePtr[0] = (ImDrawIdx)(_VtxCurrentIdx); _IdxWritePtr[1] = (ImDrawIdx)(_VtxCurrentIdx+i-1); _IdxWritePtr[2] = (ImDrawIdx)(_VtxCurrentIdx+i);
            _IdxWritePtr += 3;
        }
        _VtxCurrentIdx += (ImDrawIdx)vtx_count;
    }
}

void ImDrawList::PathArcToFast(const ImVec2& centre, float radius, int a_min_of_12, int a_max_of_12)
{
    if (radius == 0.0f || a_min_of_12 > a_max_of_12)
    {
        _Path.push_back(centre);
        return;
    }
    _Path.reserve(_Path.Size + (a_max_of_12 - a_min_of_12 + 1));
    for (int a = a_min_of_12; a <= a_max_of_12; a++)
    {
        const ImVec2& c = _Data->CircleVtx12[a % IM_ARRAYSIZE(_Data->CircleVtx12)];
        _Path.push_back(ImVec2(centre.x + c.x * radius, centre.y + c.y * radius));
    }
}

void ImDrawList::PathArcTo(const ImVec2& centre, float radius, float a_min, float a_max, int num_segments)
{
    if (radius == 0.0f)
    {
        _Path.push_back(centre);
        return;
    }

    // Note that we are adding a point at both a_min and a_max.
    // If you are trying to draw a full closed circle you don't want the overlapping points!
    _Path.reserve(_Path.Size + (num_segments + 1));
    for (int i = 0; i <= num_segments; i++)
    {
        const float a = a_min + ((float)i / (float)num_segments) * (a_max - a_min);
        _Path.push_back(ImVec2(centre.x + ImCos(a) * radius, centre.y + ImSin(a) * radius));
    }
}

static void PathBezierToCasteljau(ImVector<ImVec2>* path, float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, float tess_tol, int level)
{
    float dx = x4 - x1;
    float dy = y4 - y1;
    float d2 = ((x2 - x4) * dy - (y2 - y4) * dx);
    float d3 = ((x3 - x4) * dy - (y3 - y4) * dx);
    d2 = (d2 >= 0) ? d2 : -d2;
    d3 = (d3 >= 0) ? d3 : -d3;
    if ((d2+d3) * (d2+d3) < tess_tol * (dx*dx + dy*dy))
    {
        path->push_back(ImVec2(x4, y4));
    }
    else if (level < 10)
    {
        float x12 = (x1+x2)*0.5f,       y12 = (y1+y2)*0.5f;
        float x23 = (x2+x3)*0.5f,       y23 = (y2+y3)*0.5f;
        float x34 = (x3+x4)*0.5f,       y34 = (y3+y4)*0.5f;
        float x123 = (x12+x23)*0.5f,    y123 = (y12+y23)*0.5f;
        float x234 = (x23+x34)*0.5f,    y234 = (y23+y34)*0.5f;
        float x1234 = (x123+x234)*0.5f, y1234 = (y123+y234)*0.5f;

        PathBezierToCasteljau(path, x1,y1,        x12,y12,    x123,y123,  x1234,y1234, tess_tol, level+1);
        PathBezierToCasteljau(path, x1234,y1234,  x234,y234,  x34,y34,    x4,y4,       tess_tol, level+1);
    }
}

void ImDrawList::PathBezierCurveTo(const ImVec2& p2, const ImVec2& p3, const ImVec2& p4, int num_segments)
{
    ImVec2 p1 = _Path.back();
    if (num_segments == 0)
    {
        // Auto-tessellated
        PathBezierToCasteljau(&_Path, p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, p4.x, p4.y, _Data->CurveTessellationTol, 0);
    }
    else
    {
        float t_step = 1.0f / (float)num_segments;
        for (int i_step = 1; i_step <= num_segments; i_step++)
        {
            float t = t_step * i_step;
            float u = 1.0f - t;
            float w1 = u*u*u;
            float w2 = 3*u*u*t;
            float w3 = 3*u*t*t;
            float w4 = t*t*t;
            _Path.push_back(ImVec2(w1*p1.x + w2*p2.x + w3*p3.x + w4*p4.x, w1*p1.y + w2*p2.y + w3*p3.y + w4*p4.y));
        }
    }
}

void ImDrawList::PathRect(const ImVec2& a, const ImVec2& b, float rounding, int rounding_corners)
{
    rounding = ImMin(rounding, ImFabs(b.x - a.x) * ( ((rounding_corners & ImDrawCornerFlags_Top)  == ImDrawCornerFlags_Top)  || ((rounding_corners & ImDrawCornerFlags_Bot)   == ImDrawCornerFlags_Bot)   ? 0.5f : 1.0f ) - 1.0f);
    rounding = ImMin(rounding, ImFabs(b.y - a.y) * ( ((rounding_corners & ImDrawCornerFlags_Left) == ImDrawCornerFlags_Left) || ((rounding_corners & ImDrawCornerFlags_Right) == ImDrawCornerFlags_Right) ? 0.5f : 1.0f ) - 1.0f);

    if (rounding <= 0.0f || rounding_corners == 0)
    {
        PathLineTo(a);
        PathLineTo(ImVec2(b.x, a.y));
        PathLineTo(b);
        PathLineTo(ImVec2(a.x, b.y));
    }
    else
    {
        const float rounding_tl = (rounding_corners & ImDrawCornerFlags_TopLeft) ? rounding : 0.0f;
        const float rounding_tr = (rounding_corners & ImDrawCornerFlags_TopRight) ? rounding : 0.0f;
        const float rounding_br = (rounding_corners & ImDrawCornerFlags_BotRight) ? rounding : 0.0f;
        const float rounding_bl = (rounding_corners & ImDrawCornerFlags_BotLeft) ? rounding : 0.0f;
        PathArcToFast(ImVec2(a.x + rounding_tl, a.y + rounding_tl), rounding_tl, 6, 9);
        PathArcToFast(ImVec2(b.x - rounding_tr, a.y + rounding_tr), rounding_tr, 9, 12);
        PathArcToFast(ImVec2(b.x - rounding_br, b.y - rounding_br), rounding_br, 0, 3);
        PathArcToFast(ImVec2(a.x + rounding_bl, b.y - rounding_bl), rounding_bl, 3, 6);
    }
}

void ImDrawList::AddLine(const ImVec2& a, const ImVec2& b, ImU32 col, float thickness)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;
    PathLineTo(a + ImVec2(0.5f,0.5f));
    PathLineTo(b + ImVec2(0.5f,0.5f));
    PathStroke(col, false, thickness);
}

// a: upper-left, b: lower-right. we don't render 1 px sized rectangles properly.
void ImDrawList::AddRect(const ImVec2& a, const ImVec2& b, ImU32 col, float rounding, int rounding_corners_flags, float thickness)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;
    if (Flags & ImDrawListFlags_AntiAliasedLines)
        PathRect(a + ImVec2(0.5f,0.5f), b - ImVec2(0.50f,0.50f), rounding, rounding_corners_flags);
    else
        PathRect(a + ImVec2(0.5f,0.5f), b - ImVec2(0.49f,0.49f), rounding, rounding_corners_flags); // Better looking lower-right corner and rounded non-AA shapes.
    PathStroke(col, true, thickness);
}

void ImDrawList::AddRectFilled(const ImVec2& a, const ImVec2& b, ImU32 col, float rounding, int rounding_corners_flags)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;
    if (rounding > 0.0f)
    {
        PathRect(a, b, rounding, rounding_corners_flags);
        PathFillConvex(col);
    }
    else
    {
        PrimReserve(6, 4);
        PrimRect(a, b, col);
    }
}

void ImDrawList::AddRectFilledMultiColor(const ImVec2& a, const ImVec2& c, ImU32 col_upr_left, ImU32 col_upr_right, ImU32 col_bot_right, ImU32 col_bot_left)
{
    if (((col_upr_left | col_upr_right | col_bot_right | col_bot_left) & IM_COL32_A_MASK) == 0)
        return;

    const ImVec2 uv = _Data->TexUvWhitePixel;
    PrimReserve(6, 4);
    PrimWriteIdx((ImDrawIdx)(_VtxCurrentIdx)); PrimWriteIdx((ImDrawIdx)(_VtxCurrentIdx+1)); PrimWriteIdx((ImDrawIdx)(_VtxCurrentIdx+2));
    PrimWriteIdx((ImDrawIdx)(_VtxCurrentIdx)); PrimWriteIdx((ImDrawIdx)(_VtxCurrentIdx+2)); PrimWriteIdx((ImDrawIdx)(_VtxCurrentIdx+3));
    PrimWriteVtx(a, uv, col_upr_left);
    PrimWriteVtx(ImVec2(c.x, a.y), uv, col_upr_right);
    PrimWriteVtx(c, uv, col_bot_right);
    PrimWriteVtx(ImVec2(a.x, c.y), uv, col_bot_left);
}

void ImDrawList::AddQuad(const ImVec2& a, const ImVec2& b, const ImVec2& c, const ImVec2& d, ImU32 col, float thickness)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    PathLineTo(a);
    PathLineTo(b);
    PathLineTo(c);
    PathLineTo(d);
    PathStroke(col, true, thickness);
}

void ImDrawList::AddQuadFilled(const ImVec2& a, const ImVec2& b, const ImVec2& c, const ImVec2& d, ImU32 col)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    PathLineTo(a);
    PathLineTo(b);
    PathLineTo(c);
    PathLineTo(d);
    PathFillConvex(col);
}

void ImDrawList::AddTriangle(const ImVec2& a, const ImVec2& b, const ImVec2& c, ImU32 col, float thickness)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    PathLineTo(a);
    PathLineTo(b);
    PathLineTo(c);
    PathStroke(col, true, thickness);
}

void ImDrawList::AddTriangleFilled(const ImVec2& a, const ImVec2& b, const ImVec2& c, ImU32 col)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    PathLineTo(a);
    PathLineTo(b);
    PathLineTo(c);
    PathFillConvex(col);
}

void ImDrawList::AddCircle(const ImVec2& centre, float radius, ImU32 col, int num_segments, float thickness)
{
    if ((col & IM_COL32_A_MASK) == 0 || num_segments <= 2)
        return;

    // Because we are filling a closed shape we remove 1 from the count of segments/points
    const float a_max = IM_PI*2.0f * ((float)num_segments - 1.0f) / (float)num_segments;
    PathArcTo(centre, radius-0.5f, 0.0f, a_max, num_segments - 1);
    PathStroke(col, true, thickness);
}

void ImDrawList::AddCircleFilled(const ImVec2& centre, float radius, ImU32 col, int num_segments)
{
    if ((col & IM_COL32_A_MASK) == 0 || num_segments <= 2)
        return;

    // Because we are filling a closed shape we remove 1 from the count of segments/points
    const float a_max = IM_PI*2.0f * ((float)num_segments - 1.0f) / (float)num_segments;
    PathArcTo(centre, radius, 0.0f, a_max, num_segments - 1);
    PathFillConvex(col);
}

void ImDrawList::AddBezierCurve(const ImVec2& pos0, const ImVec2& cp0, const ImVec2& cp1, const ImVec2& pos1, ImU32 col, float thickness, int num_segments)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    PathLineTo(pos0);
    PathBezierCurveTo(cp0, cp1, pos1, num_segments);
    PathStroke(col, false, thickness);
}

void ImDrawList::AddText(const ImFont* font, float font_size, const ImVec2& pos, ImU32 col, const char* text_begin, const char* text_end, float wrap_width, const ImVec4* cpu_fine_clip_rect)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    if (text_end == NULL)
        text_end = text_begin + strlen(text_begin);
    if (text_begin == text_end)
        return;

    // Pull default font/size from the shared ImDrawListSharedData instance
    if (font == NULL || font == nullptr)
        font = _Data->Font;
    if (font_size == 0.0f)
        font_size = _Data->FontSize;

    IM_ASSERT(font->ContainerAtlas->TexID == _TextureIdStack.back());  // Use high-level ImGui::PushFont() or low-level ImDrawList::PushTextureId() to change font.

    ImVec4 clip_rect = _ClipRectStack.back();
    if (cpu_fine_clip_rect)
    {
        clip_rect.x = ImMax(clip_rect.x, cpu_fine_clip_rect->x);
        clip_rect.y = ImMax(clip_rect.y, cpu_fine_clip_rect->y);
        clip_rect.z = ImMin(clip_rect.z, cpu_fine_clip_rect->z);
        clip_rect.w = ImMin(clip_rect.w, cpu_fine_clip_rect->w);
    }
    font->RenderText(this, font_size, pos, col, clip_rect, text_begin, text_end, wrap_width, cpu_fine_clip_rect != NULL);
}

void ImDrawList::AddText(const ImVec2& pos, ImU32 col, const char* text_begin, const char* text_end)
{
    AddText(NULL, 0.0f, pos, col, text_begin, text_end);
}

void ImDrawList::AddImage(ImTextureID user_texture_id, const ImVec2& a, const ImVec2& b, const ImVec2& uv_a, const ImVec2& uv_b, ImU32 col)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    const bool push_texture_id = _TextureIdStack.empty() || user_texture_id != _TextureIdStack.back();
    if (push_texture_id)
        PushTextureID(user_texture_id);

    PrimReserve(6, 4);
    PrimRectUV(a, b, uv_a, uv_b, col);

    if (push_texture_id)
        PopTextureID();
}

void ImDrawList::AddImageQuad(ImTextureID user_texture_id, const ImVec2& a, const ImVec2& b, const ImVec2& c, const ImVec2& d, const ImVec2& uv_a, const ImVec2& uv_b, const ImVec2& uv_c, const ImVec2& uv_d, ImU32 col)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    const bool push_texture_id = _TextureIdStack.empty() || user_texture_id != _TextureIdStack.back();
    if (push_texture_id)
        PushTextureID(user_texture_id);

    PrimReserve(6, 4);
    PrimQuadUV(a, b, c, d, uv_a, uv_b, uv_c, uv_d, col);

    if (push_texture_id)
        PopTextureID();
}

void ImDrawList::AddImageRounded(ImTextureID user_texture_id, const ImVec2& a, const ImVec2& b, const ImVec2& uv_a, const ImVec2& uv_b, ImU32 col, float rounding, int rounding_corners)
{
    if ((col & IM_COL32_A_MASK) == 0)
        return;

    if (rounding <= 0.0f || (rounding_corners & ImDrawCornerFlags_All) == 0)
    {
        AddImage(user_texture_id, a, b, uv_a, uv_b, col);
        return;
    }

    const bool push_texture_id = _TextureIdStack.empty() || user_texture_id != _TextureIdStack.back();
    if (push_texture_id)
        PushTextureID(user_texture_id);

    int vert_start_idx = VtxBuffer.Size;
    PathRect(a, b, rounding, rounding_corners);
    PathFillConvex(col);
    int vert_end_idx = VtxBuffer.Size;
    ImGui::ShadeVertsLinearUV(this, vert_start_idx, vert_end_idx, a, b, uv_a, uv_b, true);

    if (push_texture_id)
        PopTextureID();
}

//-----------------------------------------------------------------------------
// [SECTION] ImDrawData
//-----------------------------------------------------------------------------

// For backward compatibility: convert all buffers from indexed to de-indexed, in case you cannot render indexed. Note: this is slow and most likely a waste of resources. Always prefer indexed rendering!
void ImDrawData::DeIndexAllBuffers()
{
    ImVector<ImDrawVert> new_vtx_buffer;
    TotalVtxCount = TotalIdxCount = 0;
    for (int i = 0; i < CmdListsCount; i++)
    {
        ImDrawList* cmd_list = CmdLists[i];
        if (cmd_list->IdxBuffer.empty())
            continue;
        new_vtx_buffer.resize(cmd_list->IdxBuffer.Size);
        for (int j = 0; j < cmd_list->IdxBuffer.Size; j++)
            new_vtx_buffer[j] = cmd_list->VtxBuffer[cmd_list->IdxBuffer[j]];
        cmd_list->VtxBuffer.swap(new_vtx_buffer);
        cmd_list->IdxBuffer.resize(0);
        TotalVtxCount += cmd_list->VtxBuffer.Size;
    }
}

// Helper to scale the ClipRect field of each ImDrawCmd. 
// Use if your final output buffer is at a different scale than draw_data->DisplaySize, 
// or if there is a difference between your window resolution and framebuffer resolution.
void ImDrawData::ScaleClipRects(const ImVec2& fb_scale)
{
    for (int i = 0; i < CmdListsCount; i++)
    {
        ImDrawList* cmd_list = CmdLists[i];
        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
        {
            ImDrawCmd* cmd = &cmd_list->CmdBuffer[cmd_i];
            cmd->ClipRect = ImVec4(cmd->ClipRect.x * fb_scale.x, cmd->ClipRect.y * fb_scale.y, cmd->ClipRect.z * fb_scale.x, cmd->ClipRect.w * fb_scale.y);
        }
    }
}

//-----------------------------------------------------------------------------
// [SECTION] Helpers ShadeVertsXXX functions
//-----------------------------------------------------------------------------

// Generic linear color gradient, write to RGB fields, leave A untouched.
void ImGui::ShadeVertsLinearColorGradientKeepAlpha(ImDrawList* draw_list, int vert_start_idx, int vert_end_idx, ImVec2 gradient_p0, ImVec2 gradient_p1, ImU32 col0, ImU32 col1)
{
    ImVec2 gradient_extent = gradient_p1 - gradient_p0;
    float gradient_inv_length2 = 1.0f / ImLengthSqr(gradient_extent);
    ImDrawVert* vert_start = draw_list->VtxBuffer.Data + vert_start_idx;
    ImDrawVert* vert_end = draw_list->VtxBuffer.Data + vert_end_idx;
    for (ImDrawVert* vert = vert_start; vert < vert_end; vert++)
    {
        float d = ImDot(vert->pos - gradient_p0, gradient_extent);
        float t = ImClamp(d * gradient_inv_length2, 0.0f, 1.0f);
        int r = ImLerp((int)(col0 >> IM_COL32_R_SHIFT) & 0xFF, (int)(col1 >> IM_COL32_R_SHIFT) & 0xFF, t);
        int g = ImLerp((int)(col0 >> IM_COL32_G_SHIFT) & 0xFF, (int)(col1 >> IM_COL32_G_SHIFT) & 0xFF, t);
        int b = ImLerp((int)(col0 >> IM_COL32_B_SHIFT) & 0xFF, (int)(col1 >> IM_COL32_B_SHIFT) & 0xFF, t);
        vert->col = (r << IM_COL32_R_SHIFT) | (g << IM_COL32_G_SHIFT) | (b << IM_COL32_B_SHIFT) | (vert->col & IM_COL32_A_MASK);
    }
}

// Distribute UV over (a, b) rectangle
void ImGui::ShadeVertsLinearUV(ImDrawList* draw_list, int vert_start_idx, int vert_end_idx, const ImVec2& a, const ImVec2& b, const ImVec2& uv_a, const ImVec2& uv_b, bool clamp)
{
    const ImVec2 size = b - a;
    const ImVec2 uv_size = uv_b - uv_a;
    const ImVec2 scale = ImVec2(
        size.x != 0.0f ? (uv_size.x / size.x) : 0.0f,
        size.y != 0.0f ? (uv_size.y / size.y) : 0.0f);

    ImDrawVert* vert_start = draw_list->VtxBuffer.Data + vert_start_idx;
    ImDrawVert* vert_end = draw_list->VtxBuffer.Data + vert_end_idx;
    if (clamp)
    {
        const ImVec2 min = ImMin(uv_a, uv_b);
        const ImVec2 max = ImMax(uv_a, uv_b);
        for (ImDrawVert* vertex = vert_start; vertex < vert_end; ++vertex)
            vertex->uv = ImClamp(uv_a + ImMul(ImVec2(vertex->pos.x, vertex->pos.y) - a, scale), min, max);
    }
    else
    {
        for (ImDrawVert* vertex = vert_start; vertex < vert_end; ++vertex)
            vertex->uv = uv_a + ImMul(ImVec2(vertex->pos.x, vertex->pos.y) - a, scale);
    }
}

//-----------------------------------------------------------------------------
// [SECTION] ImFontConfig
//-----------------------------------------------------------------------------

ImFontConfig::ImFontConfig()
{
    FontData = NULL;
    FontDataSize = 0;
    FontDataOwnedByAtlas = true;
    FontNo = 0;
    SizePixels = 0.0f;
    OversampleH = 3; // FIXME: 2 may be a better default?
    OversampleV = 1;
    PixelSnapH = false;
    GlyphExtraSpacing = ImVec2(0.0f, 0.0f);
    GlyphOffset = ImVec2(0.0f, 0.0f);
    GlyphRanges = NULL;
    GlyphMinAdvanceX = 0.0f;
    GlyphMaxAdvanceX = FLT_MAX;
    MergeMode = false;
    RasterizerFlags = 0x00;
    RasterizerMultiply = 1.0f;
    memset(Name, 0, sizeof(Name));
    DstFont = NULL;
}

//-----------------------------------------------------------------------------
// [SECTION] ImFontAtlas
//-----------------------------------------------------------------------------

// A work of art lies ahead! (. = white layer, X = black layer, others are blank)
// The white texels on the top left are the ones we'll use everywhere in ImGui to render filled shapes.
const int FONT_ATLAS_DEFAULT_TEX_DATA_W_HALF = 108;
const int FONT_ATLAS_DEFAULT_TEX_DATA_H      = 27;
const unsigned int FONT_ATLAS_DEFAULT_TEX_DATA_ID = 0x80000000;
static const char FONT_ATLAS_DEFAULT_TEX_DATA_PIXELS[FONT_ATLAS_DEFAULT_TEX_DATA_W_HALF * FONT_ATLAS_DEFAULT_TEX_DATA_H + 1] =
{
    "..-         -XXXXXXX-    X    -           X           -XXXXXXX          -          XXXXXXX-     XX          "
    "..-         -X.....X-   X.X   -          X.X          -X.....X          -          X.....X-    X..X         "
    "---         -XXX.XXX-  X...X  -         X...X         -X....X           -           X....X-    X..X         "
    "X           -  X.X  - X.....X -        X.....X        -X...X            -            X...X-    X..X         "
    "XX          -  X.X  -X.......X-       X.......X       -X..X.X           -           X.X..X-    X..X         "
    "X.X         -  X.X  -XXXX.XXXX-       XXXX.XXXX       -X.X X.X          -          X.X X.X-    X..XXX       "
    "X..X        -  X.X  -   X.X   -          X.X          -XX   X.X         -         X.X   XX-    X..X..XXX    "
    "X...X       -  X.X  -   X.X   -    XX    X.X    XX    -      X.X        -        X.X      -    X..X..X..XX  "
    "X....X      -  X.X  -   X.X   -   X.X    X.X    X.X   -       X.X       -       X.X       -    X..X..X..X.X "
    "X.....X     -  X.X  -   X.X   -  X..X    X.X    X..X  -        X.X      -      X.X        -XXX X..X..X..X..X"
    "X......X    -  X.X  -   X.X   - X...XXXXXX.XXXXXX...X -         X.X   XX-XX   X.X         -X..XX........X..X"
    "X.......X   -  X.X  -   X.X   -X.....................X-          X.X X.X-X.X X.X          -X...X...........X"
    "X........X  -  X.X  -   X.X   - X...XXXXXX.XXXXXX...X -           X.X..X-X..X.X           - X..............X"
    "X.........X -XXX.XXX-   X.X   -  X..X    X.X    X..X  -            X...X-X...X            -  X.............X"
    "X..........X-X.....X-   X.X   -   X.X    X.X    X.X   -           X....X-X....X           -  X.............X"
    "X......XXXXX-XXXXXXX-   X.X   -    XX    X.X    XX    -          X.....X-X.....X          -   X............X"
    "X...X..X    ---------   X.X   -          X.X          -          XXXXXXX-XXXXXXX          -   X...........X "
    "X..X X..X   -       -XXXX.XXXX-       XXXX.XXXX       -------------------------------------    X..........X "
    "X.X  X..X   -       -X.......X-       X.......X       -    XX           XX    -           -    X..........X "
    "XX    X..X  -       - X.....X -        X.....X        -   X.X           X.X   -           -     X........X  "
    "      X..X          -  X...X  -         X...X         -  X..X           X..X  -           -     X........X  "
    "       XX           -   X.X   -          X.X          - X...XXXXXXXXXXXXX...X -           -     XXXXXXXXXX  "
    "------------        -    X    -           X           -X.....................X-           ------------------"
    "                    ----------------------------------- X...XXXXXXXXXXXXX...X -                             "
    "                                                      -  X..X           X..X  -                             "
    "                                                      -   X.X           X.X   -                             "
    "                                                      -    XX           XX    -                             "
};

static const ImVec2 FONT_ATLAS_DEFAULT_TEX_CURSOR_DATA[ImGuiMouseCursor_COUNT][3] =
{
    // Pos ........ Size ......... Offset ......
    { ImVec2( 0,3), ImVec2(12,19), ImVec2( 0, 0) }, // ImGuiMouseCursor_Arrow
    { ImVec2(13,0), ImVec2( 7,16), ImVec2( 1, 8) }, // ImGuiMouseCursor_TextInput
    { ImVec2(31,0), ImVec2(23,23), ImVec2(11,11) }, // ImGuiMouseCursor_ResizeAll
    { ImVec2(21,0), ImVec2( 9,23), ImVec2( 4,11) }, // ImGuiMouseCursor_ResizeNS
    { ImVec2(55,18),ImVec2(23, 9), ImVec2(11, 4) }, // ImGuiMouseCursor_ResizeEW
    { ImVec2(73,0), ImVec2(17,17), ImVec2( 8, 8) }, // ImGuiMouseCursor_ResizeNESW
    { ImVec2(55,0), ImVec2(17,17), ImVec2( 8, 8) }, // ImGuiMouseCursor_ResizeNWSE
    { ImVec2(91,0), ImVec2(17,22), ImVec2( 5, 0) }, // ImGuiMouseCursor_Hand
};

ImFontAtlas::ImFontAtlas()
{
    Locked = false;
    Flags = ImFontAtlasFlags_None;
    TexID = (ImTextureID)NULL;
    TexDesiredWidth = 0;
    TexGlyphPadding = 1;

    TexPixelsAlpha8 = NULL;
    TexPixelsRGBA32 = NULL;
    TexWidth = TexHeight = 0;
    TexUvScale = ImVec2(0.0f, 0.0f);
    TexUvWhitePixel = ImVec2(0.0f, 0.0f);
    for (int n = 0; n < IM_ARRAYSIZE(CustomRectIds); n++)
        CustomRectIds[n] = -1;
}

ImFontAtlas::~ImFontAtlas()
{
    IM_ASSERT(!Locked && "Cannot modify a locked ImFontAtlas between NewFrame() and EndFrame/Render()!");
    Clear();
}

void    ImFontAtlas::ClearInputData()
{
    IM_ASSERT(!Locked && "Cannot modify a locked ImFontAtlas between NewFrame() and EndFrame/Render()!");
    for (int i = 0; i < ConfigData.Size; i++)
        if (ConfigData[i].FontData && ConfigData[i].FontDataOwnedByAtlas)
        {
            ImGui::MemFree(ConfigData[i].FontData);
            ConfigData[i].FontData = NULL;
        }

    // When clearing this we lose access to the font name and other information used to build the font.
    for (int i = 0; i < Fonts.Size; i++)
        if (Fonts[i]->ConfigData >= ConfigData.Data && Fonts[i]->ConfigData < ConfigData.Data + ConfigData.Size)
        {
            Fonts[i]->ConfigData = NULL;
            Fonts[i]->ConfigDataCount = 0;
        }
    ConfigData.clear();
    CustomRects.clear();
    for (int n = 0; n < IM_ARRAYSIZE(CustomRectIds); n++)
        CustomRectIds[n] = -1;
}

void    ImFontAtlas::ClearTexData()
{
    IM_ASSERT(!Locked && "Cannot modify a locked ImFontAtlas between NewFrame() and EndFrame/Render()!");
    if (TexPixelsAlpha8)
        ImGui::MemFree(TexPixelsAlpha8);
    if (TexPixelsRGBA32)
        ImGui::MemFree(TexPixelsRGBA32);
    TexPixelsAlpha8 = NULL;
    TexPixelsRGBA32 = NULL;
}

void    ImFontAtlas::ClearFonts()
{
    IM_ASSERT(!Locked && "Cannot modify a locked ImFontAtlas between NewFrame() and EndFrame/Render()!");
    for (int i = 0; i < Fonts.Size; i++)
        IM_DELETE(Fonts[i]);
    Fonts.clear();
}

void    ImFontAtlas::Clear()
{
    ClearInputData();
    ClearTexData();
    ClearFonts();
}

void    ImFontAtlas::GetTexDataAsAlpha8(unsigned char** out_pixels, int* out_width, int* out_height, int* out_bytes_per_pixel)
{
    // Build atlas on demand
    if (TexPixelsAlpha8 == NULL)
    {
        if (ConfigData.empty())
            AddFontDefault();
        Build();
    }

    *out_pixels = TexPixelsAlpha8;
    if (out_width) *out_width = TexWidth;
    if (out_height) *out_height = TexHeight;
    if (out_bytes_per_pixel) *out_bytes_per_pixel = 1;
}

void    ImFontAtlas::GetTexDataAsRGBA32(unsigned char** out_pixels, int* out_width, int* out_height, int* out_bytes_per_pixel)
{
    // Convert to RGBA32 format on demand
    // Although it is likely to be the most commonly used format, our font rendering is 1 channel / 8 bpp
    if (!TexPixelsRGBA32)
    {
        unsigned char* pixels = NULL;
        GetTexDataAsAlpha8(&pixels, NULL, NULL);
        if (pixels)
        {
            TexPixelsRGBA32 = (unsigned int*)ImGui::MemAlloc((size_t)TexWidth * (size_t)TexHeight * 4);
            const unsigned char* src = pixels;
            unsigned int* dst = TexPixelsRGBA32;
            for (int n = TexWidth * TexHeight; n > 0; n--)
                *dst++ = IM_COL32(255, 255, 255, (unsigned int)(*src++));
        }
    }

    *out_pixels = (unsigned char*)TexPixelsRGBA32;
    if (out_width) *out_width = TexWidth;
    if (out_height) *out_height = TexHeight;
    if (out_bytes_per_pixel) *out_bytes_per_pixel = 4;
}

ImFont* ImFontAtlas::AddFont(const ImFontConfig* font_cfg)
{
    IM_ASSERT(!Locked && "Cannot modify a locked ImFontAtlas between NewFrame() and EndFrame/Render()!");
    IM_ASSERT(font_cfg->FontData != NULL && font_cfg->FontDataSize > 0);
    IM_ASSERT(font_cfg->SizePixels > 0.0f);

    // Create new font
    if (!font_cfg->MergeMode)
        Fonts.push_back(IM_NEW(ImFont));
    else
        IM_ASSERT(!Fonts.empty() && "Cannot use MergeMode for the first font"); // When using MergeMode make sure that a font has already been added before. You can use ImGui::GetIO().Fonts->AddFontDefault() to add the default imgui font.

    ConfigData.push_back(*font_cfg);
    ImFontConfig& new_font_cfg = ConfigData.back();
    if (new_font_cfg.DstFont == NULL)
        new_font_cfg.DstFont = Fonts.back();
    if (!new_font_cfg.FontDataOwnedByAtlas)
    {
        new_font_cfg.FontData = ImGui::MemAlloc(new_font_cfg.FontDataSize);
        new_font_cfg.FontDataOwnedByAtlas = true;
        memcpy(new_font_cfg.FontData, font_cfg->FontData, (size_t)new_font_cfg.FontDataSize);
    }

    // Invalidate texture
    ClearTexData();
    return new_font_cfg.DstFont;
}

// Default font TTF is compressed with stb_compress then base85 encoded (see misc/fonts/binary_to_compressed_c.cpp for encoder)
static unsigned int stb_decompress_length(const unsigned char *input);
static unsigned int stb_decompress(unsigned char *output, const unsigned char *input, unsigned int length);
static const char*  GetDefaultCompressedFontDataTTFBase85();
static unsigned int Decode85Byte(char c)                                    { return c >= '\\' ? c-36 : c-35; }
static void         Decode85(const unsigned char* src, unsigned char* dst)
{
    while (*src)
    {
        unsigned int tmp = Decode85Byte(src[0]) + 85*(Decode85Byte(src[1]) + 85*(Decode85Byte(src[2]) + 85*(Decode85Byte(src[3]) + 85*Decode85Byte(src[4]))));
        dst[0] = ((tmp >> 0) & 0xFF); dst[1] = ((tmp >> 8) & 0xFF); dst[2] = ((tmp >> 16) & 0xFF); dst[3] = ((tmp >> 24) & 0xFF);   // We can't assume little-endianness.
        src += 5;
        dst += 4;
    }
}

// Load embedded ProggyClean.ttf at size 13, disable oversampling
ImFont* ImFontAtlas::AddFontDefault(const ImFontConfig* font_cfg_template)
{
    ImFontConfig font_cfg = font_cfg_template ? *font_cfg_template : ImFontConfig();
    if (!font_cfg_template)
    {
        font_cfg.OversampleH = font_cfg.OversampleV = 1;
        font_cfg.PixelSnapH = true;
    }
    if (font_cfg.SizePixels <= 0.0f) 
        font_cfg.SizePixels = 13.0f * 1.0f;
    if (font_cfg.Name[0] == '\0') 
        ImFormatString(font_cfg.Name, IM_ARRAYSIZE(font_cfg.Name), "ProggyClean.ttf, %dpx", (int)font_cfg.SizePixels);

    const char* ttf_compressed_base85 = GetDefaultCompressedFontDataTTFBase85();
    const ImWchar* glyph_ranges = font_cfg.GlyphRanges != NULL ? font_cfg.GlyphRanges : GetGlyphRangesDefault();
    ImFont* font = AddFontFromMemoryCompressedBase85TTF(ttf_compressed_base85, font_cfg.SizePixels, &font_cfg, glyph_ranges);
    font->DisplayOffset.y = 1.0f;
    return font;
}

ImFont* ImFontAtlas::AddFontFromFileTTF(const char* filename, float size_pixels, const ImFontConfig* font_cfg_template, const ImWchar* glyph_ranges)
{
    IM_ASSERT(!Locked && "Cannot modify a locked ImFontAtlas between NewFrame() and EndFrame/Render()!");
    size_t data_size = 0;
    void* data = ImFileLoadToMemory(filename, "rb", &data_size, 0);
    if (!data)
    {
        IM_ASSERT(0); // Could not load file.
        return NULL;
    }
    ImFontConfig font_cfg = font_cfg_template ? *font_cfg_template : ImFontConfig();
    if (font_cfg.Name[0] == '\0')
    {
        // Store a short copy of filename into into the font name for convenience
        const char* p;
        for (p = filename + strlen(filename); p > filename && p[-1] != '/' && p[-1] != '\\'; p--) {}
        ImFormatString(font_cfg.Name, IM_ARRAYSIZE(font_cfg.Name), "%s, %.0fpx", p, size_pixels);
    }
    return AddFontFromMemoryTTF(data, (int)data_size, size_pixels, &font_cfg, glyph_ranges);
}

// NB: Transfer ownership of 'ttf_data' to ImFontAtlas, unless font_cfg_template->FontDataOwnedByAtlas == false. Owned TTF buffer will be deleted after Build().
ImFont* ImFontAtlas::AddFontFromMemoryTTF(void* ttf_data, int ttf_size, float size_pixels, const ImFontConfig* font_cfg_template, const ImWchar* glyph_ranges)
{
    IM_ASSERT(!Locked && "Cannot modify a locked ImFontAtlas between NewFrame() and EndFrame/Render()!");
    ImFontConfig font_cfg = font_cfg_template ? *font_cfg_template : ImFontConfig();
    IM_ASSERT(font_cfg.FontData == NULL);
    font_cfg.FontData = ttf_data;
    font_cfg.FontDataSize = ttf_size;
    font_cfg.SizePixels = size_pixels;
    if (glyph_ranges)
        font_cfg.GlyphRanges = glyph_ranges;
    return AddFont(&font_cfg);
}

ImFont* ImFontAtlas::AddFontFromMemoryCompressedTTF(const void* compressed_ttf_data, int compressed_ttf_size, float size_pixels, const ImFontConfig* font_cfg_template, const ImWchar* glyph_ranges)
{
    const unsigned int buf_decompressed_size = stb_decompress_length((const unsigned char*)compressed_ttf_data);
    unsigned char* buf_decompressed_data = (unsigned char *)ImGui::MemAlloc(buf_decompressed_size);
    stb_decompress(buf_decompressed_data, (const unsigned char*)compressed_ttf_data, (unsigned int)compressed_ttf_size);

    ImFontConfig font_cfg = font_cfg_template ? *font_cfg_template : ImFontConfig();
    IM_ASSERT(font_cfg.FontData == NULL);
    font_cfg.FontDataOwnedByAtlas = true;
    return AddFontFromMemoryTTF(buf_decompressed_data, (int)buf_decompressed_size, size_pixels, &font_cfg, glyph_ranges);
}

ImFont* ImFontAtlas::AddFontFromMemoryCompressedBase85TTF(const char* compressed_ttf_data_base85, float size_pixels, const ImFontConfig* font_cfg, const ImWchar* glyph_ranges)
{
    int compressed_ttf_size = (((int)strlen(compressed_ttf_data_base85) + 4) / 5) * 4;
    void* compressed_ttf = ImGui::MemAlloc((size_t)compressed_ttf_size);
    Decode85((const unsigned char*)compressed_ttf_data_base85, (unsigned char*)compressed_ttf);
    ImFont* font = AddFontFromMemoryCompressedTTF(compressed_ttf, compressed_ttf_size, size_pixels, font_cfg, glyph_ranges);
    ImGui::MemFree(compressed_ttf);
    return font;
}

int ImFontAtlas::AddCustomRectRegular(unsigned int id, int width, int height)
{
    IM_ASSERT(id >= 0x10000);
    IM_ASSERT(width > 0 && width <= 0xFFFF);
    IM_ASSERT(height > 0 && height <= 0xFFFF);
    CustomRect r;
    r.ID = id;
    r.Width = (unsigned short)width;
    r.Height = (unsigned short)height;
    CustomRects.push_back(r);
    return CustomRects.Size - 1; // Return index
}

int ImFontAtlas::AddCustomRectFontGlyph(ImFont* font, ImWchar id, int width, int height, float advance_x, const ImVec2& offset)
{
    IM_ASSERT(font != NULL);
    IM_ASSERT(width > 0 && width <= 0xFFFF);
    IM_ASSERT(height > 0 && height <= 0xFFFF);
    CustomRect r;
    r.ID = id;
    r.Width = (unsigned short)width;
    r.Height = (unsigned short)height;
    r.GlyphAdvanceX = advance_x;
    r.GlyphOffset = offset;
    r.Font = font;
    CustomRects.push_back(r);
    return CustomRects.Size - 1; // Return index
}

void ImFontAtlas::CalcCustomRectUV(const CustomRect* rect, ImVec2* out_uv_min, ImVec2* out_uv_max)
{
    IM_ASSERT(TexWidth > 0 && TexHeight > 0);   // Font atlas needs to be built before we can calculate UV coordinates
    IM_ASSERT(rect->IsPacked());                // Make sure the rectangle has been packed
    *out_uv_min = ImVec2((float)rect->X * TexUvScale.x, (float)rect->Y * TexUvScale.y);
    *out_uv_max = ImVec2((float)(rect->X + rect->Width) * TexUvScale.x, (float)(rect->Y + rect->Height) * TexUvScale.y);
}

bool ImFontAtlas::GetMouseCursorTexData(ImGuiMouseCursor cursor_type, ImVec2* out_offset, ImVec2* out_size, ImVec2 out_uv_border[2], ImVec2 out_uv_fill[2])
{
    if (cursor_type <= ImGuiMouseCursor_None || cursor_type >= ImGuiMouseCursor_COUNT)
        return false;
    if (Flags & ImFontAtlasFlags_NoMouseCursors)
        return false;

    IM_ASSERT(CustomRectIds[0] != -1);
    ImFontAtlas::CustomRect& r = CustomRects[CustomRectIds[0]];
    IM_ASSERT(r.ID == FONT_ATLAS_DEFAULT_TEX_DATA_ID);
    ImVec2 pos = FONT_ATLAS_DEFAULT_TEX_CURSOR_DATA[cursor_type][0] + ImVec2((float)r.X, (float)r.Y);
    ImVec2 size = FONT_ATLAS_DEFAULT_TEX_CURSOR_DATA[cursor_type][1];
    *out_size = size;
    *out_offset = FONT_ATLAS_DEFAULT_TEX_CURSOR_DATA[cursor_type][2];
    out_uv_border[0] = (pos) * TexUvScale;
    out_uv_border[1] = (pos + size) * TexUvScale;
    pos.x += FONT_ATLAS_DEFAULT_TEX_DATA_W_HALF + 1;
    out_uv_fill[0] = (pos) * TexUvScale;
    out_uv_fill[1] = (pos + size) * TexUvScale;
    return true;
}

bool    ImFontAtlas::Build()
{
    IM_ASSERT(!Locked && "Cannot modify a locked ImFontAtlas between NewFrame() and EndFrame/Render()!");
    return ImFontAtlasBuildWithStbTruetype(this);
}

void    ImFontAtlasBuildMultiplyCalcLookupTable(unsigned char out_table[256], float in_brighten_factor)
{
    for (unsigned int i = 0; i < 256; i++)
    {
        unsigned int value = (unsigned int)(i * in_brighten_factor);
        out_table[i] = value > 255 ? 255 : (value & 0xFF);
    }
}

void    ImFontAtlasBuildMultiplyRectAlpha8(const unsigned char table[256], unsigned char* pixels, int x, int y, int w, int h, int stride)
{
    unsigned char* data = pixels + x + y * stride;
    for (int j = h; j > 0; j--, data += stride)
        for (int i = 0; i < w; i++)
            data[i] = table[data[i]];
}

// Temporary data for one source font (multiple source fonts can be merged into one destination ImFont)
// (C++03 doesn't allow instancing ImVector<> with function-local types so we declare the type here.)
struct ImFontBuildSrcData
{
    stbtt_fontinfo      FontInfo;
    stbtt_pack_range    PackRange;          // Hold the list of codepoints to pack (essentially points to Codepoints.Data)
    stbrp_rect*         Rects;              // Rectangle to pack. We first fill in their size and the packer will give us their position.
    stbtt_packedchar*   PackedChars;        // Output glyphs
    const ImWchar*      SrcRanges;          // Ranges as requested by user (user is allowed to request too much, e.g. 0x0020..0xFFFF)
    int                 DstIndex;           // Index into atlas->Fonts[] and dst_tmp_array[]
    int                 GlyphsHighest;      // Highest requested codepoint
    int                 GlyphsCount;        // Glyph count (excluding missing glyphs and glyphs already set by an earlier source font)
    ImBoolVector        GlyphsSet;          // Glyph bit map (random access, 1-bit per codepoint. This will be a maximum of 8KB)
    ImVector<int>       GlyphsList;         // Glyph codepoints list (flattened version of GlyphsMap)
};

// Temporary data for one destination ImFont* (multiple source fonts can be merged into one destination ImFont)
struct ImFontBuildDstData
{
    int                 SrcCount;           // Number of source fonts targeting this destination font.
    int                 GlyphsHighest;
    int                 GlyphsCount;
    ImBoolVector        GlyphsSet;          // This is used to resolve collision when multiple sources are merged into a same destination font.
};

static void UnpackBoolVectorToFlatIndexList(const ImBoolVector* in, ImVector<int>* out)
{
    IM_ASSERT(sizeof(in->Storage.Data[0]) == sizeof(int));
    const int* it_begin = in->Storage.begin();
    const int* it_end = in->Storage.end();
    for (const int* it = it_begin; it < it_end; it++)
        if (int entries_32 = *it)
            for (int bit_n = 0; bit_n < 32; bit_n++)
                if (entries_32 & (1 << bit_n))
                    out->push_back((int)((it - it_begin) << 5) + bit_n);
}

bool    ImFontAtlasBuildWithStbTruetype(ImFontAtlas* atlas)
{
    IM_ASSERT(atlas->ConfigData.Size > 0);

    ImFontAtlasBuildRegisterDefaultCustomRects(atlas);

    // Clear atlas
    atlas->TexID = (ImTextureID)NULL;
    atlas->TexWidth = atlas->TexHeight = 0;
    atlas->TexUvScale = ImVec2(0.0f, 0.0f);
    atlas->TexUvWhitePixel = ImVec2(0.0f, 0.0f);
    atlas->ClearTexData();

    // Temporary storage for building
    ImVector<ImFontBuildSrcData> src_tmp_array;
    ImVector<ImFontBuildDstData> dst_tmp_array;
    src_tmp_array.resize(atlas->ConfigData.Size);
    dst_tmp_array.resize(atlas->Fonts.Size);
    memset(src_tmp_array.Data, 0, (size_t)src_tmp_array.size_in_bytes());
    memset(dst_tmp_array.Data, 0, (size_t)dst_tmp_array.size_in_bytes());

    // 1. Initialize font loading structure, check font data validity
    for (int src_i = 0; src_i < atlas->ConfigData.Size; src_i++)
    {
        ImFontBuildSrcData& src_tmp = src_tmp_array[src_i];
        ImFontConfig& cfg = atlas->ConfigData[src_i];
        IM_ASSERT(cfg.DstFont && (!cfg.DstFont->IsLoaded() || cfg.DstFont->ContainerAtlas == atlas));

        // Find index from cfg.DstFont (we allow the user to set cfg.DstFont. Also it makes casual debugging nicer than when storing indices)
        src_tmp.DstIndex = -1;
        for (int output_i = 0; output_i < atlas->Fonts.Size && src_tmp.DstIndex == -1; output_i++)
            if (cfg.DstFont == atlas->Fonts[output_i])
                src_tmp.DstIndex = output_i;
        IM_ASSERT(src_tmp.DstIndex != -1); // cfg.DstFont not pointing within atlas->Fonts[] array?
        if (src_tmp.DstIndex == -1)
            return false;

        // Initialize helper structure for font loading and verify that the TTF/OTF data is correct
        const int font_offset = stbtt_GetFontOffsetForIndex((unsigned char*)cfg.FontData, cfg.FontNo);
        IM_ASSERT(font_offset >= 0 && "FontData is incorrect, or FontNo cannot be found.");
        if (!stbtt_InitFont(&src_tmp.FontInfo, (unsigned char*)cfg.FontData, font_offset))
            return false;

        // Measure highest codepoints
        ImFontBuildDstData& dst_tmp = dst_tmp_array[src_tmp.DstIndex];
        src_tmp.SrcRanges = cfg.GlyphRanges ? cfg.GlyphRanges : atlas->GetGlyphRangesDefault();
        for (const ImWchar* src_range = src_tmp.SrcRanges; src_range[0] && src_range[1]; src_range += 2)
            src_tmp.GlyphsHighest = ImMax(src_tmp.GlyphsHighest, (int)src_range[1]);
        dst_tmp.SrcCount++;
        dst_tmp.GlyphsHighest = ImMax(dst_tmp.GlyphsHighest, src_tmp.GlyphsHighest);
    }

    // 2. For every requested codepoint, check for their presence in the font data, and handle redundancy or overlaps between source fonts to avoid unused glyphs.
    int total_glyphs_count = 0;
    for (int src_i = 0; src_i < src_tmp_array.Size; src_i++)
    {
        ImFontBuildSrcData& src_tmp = src_tmp_array[src_i];
        ImFontBuildDstData& dst_tmp = dst_tmp_array[src_tmp.DstIndex];
        src_tmp.GlyphsSet.Resize(src_tmp.GlyphsHighest + 1);
        if (dst_tmp.GlyphsSet.Storage.empty())
            dst_tmp.GlyphsSet.Resize(dst_tmp.GlyphsHighest + 1);

        for (const ImWchar* src_range = src_tmp.SrcRanges; src_range[0] && src_range[1]; src_range += 2)
            for (int codepoint = src_range[0]; codepoint <= src_range[1]; codepoint++)
            {
                if (dst_tmp.GlyphsSet.GetBit(codepoint))    // Don't overwrite existing glyphs. We could make this an option for MergeMode (e.g. MergeOverwrite==true)
                    continue;
                if (!stbtt_FindGlyphIndex(&src_tmp.FontInfo, codepoint))    // It is actually in the font?
                    continue;

                // Add to avail set/counters
                src_tmp.GlyphsCount++;
                dst_tmp.GlyphsCount++;
                src_tmp.GlyphsSet.SetBit(codepoint, true);
                dst_tmp.GlyphsSet.SetBit(codepoint, true);
                total_glyphs_count++;
            }
    }

    // 3. Unpack our bit map into a flat list (we now have all the Unicode points that we know are requested _and_ available _and_ not overlapping another)
    for (int src_i = 0; src_i < src_tmp_array.Size; src_i++)
    {
        ImFontBuildSrcData& src_tmp = src_tmp_array[src_i];
        src_tmp.GlyphsList.reserve(src_tmp.GlyphsCount);
        UnpackBoolVectorToFlatIndexList(&src_tmp.GlyphsSet, &src_tmp.GlyphsList);
        src_tmp.GlyphsSet.Clear();
        IM_ASSERT(src_tmp.GlyphsList.Size == src_tmp.GlyphsCount);
    }
    for (int dst_i = 0; dst_i < dst_tmp_array.Size; dst_i++)
        dst_tmp_array[dst_i].GlyphsSet.Clear();
    dst_tmp_array.clear();

    // Allocate packing character data and flag packed characters buffer as non-packed (x0=y0=x1=y1=0)
    // (We technically don't need to zero-clear buf_rects, but let's do it for the sake of sanity)
    ImVector<stbrp_rect> buf_rects;
    ImVector<stbtt_packedchar> buf_packedchars;
    buf_rects.resize(total_glyphs_count);
    buf_packedchars.resize(total_glyphs_count);
    memset(buf_rects.Data, 0, (size_t)buf_rects.size_in_bytes());
    memset(buf_packedchars.Data, 0, (size_t)buf_packedchars.size_in_bytes());

    // 4. Gather glyphs sizes so we can pack them in our virtual canvas.
    int total_surface = 0;
    int buf_rects_out_n = 0;
    int buf_packedchars_out_n = 0;
    for (int src_i = 0; src_i < src_tmp_array.Size; src_i++)
    {
        ImFontBuildSrcData& src_tmp = src_tmp_array[src_i];
        if (src_tmp.GlyphsCount == 0)
            continue;

        src_tmp.Rects = &buf_rects[buf_rects_out_n];
        src_tmp.PackedChars = &buf_packedchars[buf_packedchars_out_n];
        buf_rects_out_n += src_tmp.GlyphsCount;
        buf_packedchars_out_n += src_tmp.GlyphsCount;

        // Convert our ranges in the format stb_truetype wants
        ImFontConfig& cfg = atlas->ConfigData[src_i];
        src_tmp.PackRange.font_size = cfg.SizePixels;
        src_tmp.PackRange.first_unicode_codepoint_in_range = 0;
        src_tmp.PackRange.array_of_unicode_codepoints = src_tmp.GlyphsList.Data;
        src_tmp.PackRange.num_chars = src_tmp.GlyphsList.Size;
        src_tmp.PackRange.chardata_for_range = src_tmp.PackedChars;
        src_tmp.PackRange.h_oversample = (unsigned char)cfg.OversampleH;
        src_tmp.PackRange.v_oversample = (unsigned char)cfg.OversampleV;

        // Gather the sizes of all rectangles we will need to pack (this loop is based on stbtt_PackFontRangesGatherRects)
        const float scale = (cfg.SizePixels > 0) ? stbtt_ScaleForPixelHeight(&src_tmp.FontInfo, cfg.SizePixels) : stbtt_ScaleForMappingEmToPixels(&src_tmp.FontInfo, -cfg.SizePixels);
        const int padding = atlas->TexGlyphPadding;
        for (int glyph_i = 0; glyph_i < src_tmp.GlyphsList.Size; glyph_i++)
        {
            int x0, y0, x1, y1;
            const int glyph_index_in_font = stbtt_FindGlyphIndex(&src_tmp.FontInfo, src_tmp.GlyphsList[glyph_i]);
            IM_ASSERT(glyph_index_in_font != 0);
            stbtt_GetGlyphBitmapBoxSubpixel(&src_tmp.FontInfo, glyph_index_in_font, scale * cfg.OversampleH, scale * cfg.OversampleV, 0, 0, &x0, &y0, &x1, &y1);
            src_tmp.Rects[glyph_i].w = (stbrp_coord)(x1 - x0 + padding + cfg.OversampleH - 1);
            src_tmp.Rects[glyph_i].h = (stbrp_coord)(y1 - y0 + padding + cfg.OversampleV - 1);
            total_surface += src_tmp.Rects[glyph_i].w * src_tmp.Rects[glyph_i].h;
        }
    }

    // We need a width for the skyline algorithm, any width!
    // The exact width doesn't really matter much, but some API/GPU have texture size limitations and increasing width can decrease height.
    // User can override TexDesiredWidth and TexGlyphPadding if they wish, otherwise we use a simple heuristic to select the width based on expected surface.
    const int surface_sqrt = (int)ImSqrt((float)total_surface) + 1;
    atlas->TexHeight = 0;
    if (atlas->TexDesiredWidth > 0)
        atlas->TexWidth = atlas->TexDesiredWidth;
    else
        atlas->TexWidth = (surface_sqrt >= 4096*0.7f) ? 4096 : (surface_sqrt >= 2048*0.7f) ? 2048 : (surface_sqrt >= 1024*0.7f) ? 1024 : 512;

    // 5. Start packing
    // Pack our extra data rectangles first, so it will be on the upper-left corner of our texture (UV will have small values).
    const int TEX_HEIGHT_MAX = 1024 * 32;
    stbtt_pack_context spc = {};
    stbtt_PackBegin(&spc, NULL, atlas->TexWidth, TEX_HEIGHT_MAX, 0, atlas->TexGlyphPadding, NULL);
    ImFontAtlasBuildPackCustomRects(atlas, spc.pack_info);

    // 6. Pack each source font. No rendering yet, we are working with rectangles in an infinitely tall texture at this point.
    for (int src_i = 0; src_i < src_tmp_array.Size; src_i++)
    {
        ImFontBuildSrcData& src_tmp = src_tmp_array[src_i];
        if (src_tmp.GlyphsCount == 0)
            continue;

        stbrp_pack_rects((stbrp_context*)spc.pack_info, src_tmp.Rects, src_tmp.GlyphsCount);

        // Extend texture height and mark missing glyphs as non-packed so we won't render them.
        // FIXME: We are not handling packing failure here (would happen if we got off TEX_HEIGHT_MAX or if a single if larger than TexWidth?)
        for (int glyph_i = 0; glyph_i < src_tmp.GlyphsCount; glyph_i++)
            if (src_tmp.Rects[glyph_i].was_packed)
                atlas->TexHeight = ImMax(atlas->TexHeight, src_tmp.Rects[glyph_i].y + src_tmp.Rects[glyph_i].h);
    }

    // 7. Allocate texture
    atlas->TexHeight = (atlas->Flags & ImFontAtlasFlags_NoPowerOfTwoHeight) ? (atlas->TexHeight + 1) : ImUpperPowerOfTwo(atlas->TexHeight);
    atlas->TexUvScale = ImVec2(1.0f / atlas->TexWidth, 1.0f / atlas->TexHeight);
    atlas->TexPixelsAlpha8 = (unsigned char*)ImGui::MemAlloc(atlas->TexWidth * atlas->TexHeight);
    memset(atlas->TexPixelsAlpha8, 0, atlas->TexWidth * atlas->TexHeight);
    spc.pixels = atlas->TexPixelsAlpha8;
    spc.height = atlas->TexHeight;

    // 8. Render/rasterize font characters into the texture
    for (int src_i = 0; src_i < src_tmp_array.Size; src_i++)
    {
        ImFontConfig& cfg = atlas->ConfigData[src_i];
        ImFontBuildSrcData& src_tmp = src_tmp_array[src_i];
        if (src_tmp.GlyphsCount == 0)
            continue;

        stbtt_PackFontRangesRenderIntoRects(&spc, &src_tmp.FontInfo, &src_tmp.PackRange, 1, src_tmp.Rects);

        // Apply multiply operator
        if (cfg.RasterizerMultiply != 1.0f)
        {
            unsigned char multiply_table[256];
            ImFontAtlasBuildMultiplyCalcLookupTable(multiply_table, cfg.RasterizerMultiply);
            stbrp_rect* r = &src_tmp.Rects[0];
            for (int glyph_i = 0; glyph_i < src_tmp.GlyphsCount; glyph_i++, r++)
                if (r->was_packed)
                    ImFontAtlasBuildMultiplyRectAlpha8(multiply_table, atlas->TexPixelsAlpha8, r->x, r->y, r->w, r->h, atlas->TexWidth * 1);
        }
        src_tmp.Rects = NULL;
    }

    // End packing
    stbtt_PackEnd(&spc);
    buf_rects.clear();

    // 9. Setup ImFont and glyphs for runtime
    for (int src_i = 0; src_i < src_tmp_array.Size; src_i++)
    {
        ImFontBuildSrcData& src_tmp = src_tmp_array[src_i];
        if (src_tmp.GlyphsCount == 0)
            continue;

        ImFontConfig& cfg = atlas->ConfigData[src_i];
        ImFont* dst_font = cfg.DstFont; // We can have multiple input fonts writing into a same destination font (when using MergeMode=true)

        const float font_scale = stbtt_ScaleForPixelHeight(&src_tmp.FontInfo, cfg.SizePixels);
        int unscaled_ascent, unscaled_descent, unscaled_line_gap;
        stbtt_GetFontVMetrics(&src_tmp.FontInfo, &unscaled_ascent, &unscaled_descent, &unscaled_line_gap);

        const float ascent = ImFloor(unscaled_ascent * font_scale + ((unscaled_ascent > 0.0f) ? +1 : -1));
        const float descent = ImFloor(unscaled_descent * font_scale + ((unscaled_descent > 0.0f) ? +1 : -1));
        ImFontAtlasBuildSetupFont(atlas, dst_font, &cfg, ascent, descent);
        const float font_off_x = cfg.GlyphOffset.x;
        const float font_off_y = cfg.GlyphOffset.y + (float)(int)(dst_font->Ascent + 0.5f);

        for (int glyph_i = 0; glyph_i < src_tmp.GlyphsCount; glyph_i++)
        {
            const int codepoint = src_tmp.GlyphsList[glyph_i];
            const stbtt_packedchar& pc = src_tmp.PackedChars[glyph_i];

            const float char_advance_x_org = pc.xadvance;
            const float char_advance_x_mod = ImClamp(char_advance_x_org, cfg.GlyphMinAdvanceX, cfg.GlyphMaxAdvanceX);
            float char_off_x = font_off_x;
            if (char_advance_x_org != char_advance_x_mod)
                char_off_x += cfg.PixelSnapH ? (float)(int)((char_advance_x_mod - char_advance_x_org) * 0.5f) : (char_advance_x_mod - char_advance_x_org) * 0.5f;

            // Register glyph
            stbtt_aligned_quad q;
            float dummy_x = 0.0f, dummy_y = 0.0f;
            stbtt_GetPackedQuad(src_tmp.PackedChars, atlas->TexWidth, atlas->TexHeight, glyph_i, &dummy_x, &dummy_y, &q, 0);
            dst_font->AddGlyph((ImWchar)codepoint, q.x0 + char_off_x, q.y0 + font_off_y, q.x1 + char_off_x, q.y1 + font_off_y, q.s0, q.t0, q.s1, q.t1, char_advance_x_mod);
        }
    }

    // Cleanup temporary (ImVector doesn't honor destructor)
    for (int src_i = 0; src_i < src_tmp_array.Size; src_i++)
        src_tmp_array[src_i].~ImFontBuildSrcData();

    ImFontAtlasBuildFinish(atlas);
    return true;
}

void ImFontAtlasBuildRegisterDefaultCustomRects(ImFontAtlas* atlas)
{
    if (atlas->CustomRectIds[0] >= 0)
        return;
    if (!(atlas->Flags & ImFontAtlasFlags_NoMouseCursors))
        atlas->CustomRectIds[0] = atlas->AddCustomRectRegular(FONT_ATLAS_DEFAULT_TEX_DATA_ID, FONT_ATLAS_DEFAULT_TEX_DATA_W_HALF*2+1, FONT_ATLAS_DEFAULT_TEX_DATA_H);
    else
        atlas->CustomRectIds[0] = atlas->AddCustomRectRegular(FONT_ATLAS_DEFAULT_TEX_DATA_ID, 2, 2);
}

void ImFontAtlasBuildSetupFont(ImFontAtlas* atlas, ImFont* font, ImFontConfig* font_config, float ascent, float descent)
{
    if (!font_config->MergeMode)
    {
        font->ClearOutputData();
        font->FontSize = font_config->SizePixels;
        font->ConfigData = font_config;
        font->ContainerAtlas = atlas;
        font->Ascent = ascent;
        font->Descent = descent;
    }
    font->ConfigDataCount++;
}

void ImFontAtlasBuildPackCustomRects(ImFontAtlas* atlas, void* stbrp_context_opaque)
{
    stbrp_context* pack_context = (stbrp_context*)stbrp_context_opaque;
    IM_ASSERT(pack_context != NULL);

    ImVector<ImFontAtlas::CustomRect>& user_rects = atlas->CustomRects;
    IM_ASSERT(user_rects.Size >= 1); // We expect at least the default custom rects to be registered, else something went wrong.

    ImVector<stbrp_rect> pack_rects;
    pack_rects.resize(user_rects.Size);
    memset(pack_rects.Data, 0, (size_t)pack_rects.size_in_bytes());
    for (int i = 0; i < user_rects.Size; i++)
    {
        pack_rects[i].w = user_rects[i].Width;
        pack_rects[i].h = user_rects[i].Height;
    }
    stbrp_pack_rects(pack_context, &pack_rects[0], pack_rects.Size);
    for (int i = 0; i < pack_rects.Size; i++)
        if (pack_rects[i].was_packed)
        {
            user_rects[i].X = pack_rects[i].x;
            user_rects[i].Y = pack_rects[i].y;
            IM_ASSERT(pack_rects[i].w == user_rects[i].Width && pack_rects[i].h == user_rects[i].Height);
            atlas->TexHeight = ImMax(atlas->TexHeight, pack_rects[i].y + pack_rects[i].h);
        }
}

static void ImFontAtlasBuildRenderDefaultTexData(ImFontAtlas* atlas)
{
    IM_ASSERT(atlas->CustomRectIds[0] >= 0);
    IM_ASSERT(atlas->TexPixelsAlpha8 != NULL);
    ImFontAtlas::CustomRect& r = atlas->CustomRects[atlas->CustomRectIds[0]];
    IM_ASSERT(r.ID == FONT_ATLAS_DEFAULT_TEX_DATA_ID);
    IM_ASSERT(r.IsPacked());

    const int w = atlas->TexWidth;
    if (!(atlas->Flags & ImFontAtlasFlags_NoMouseCursors))
    {
        // Render/copy pixels
        IM_ASSERT(r.Width == FONT_ATLAS_DEFAULT_TEX_DATA_W_HALF * 2 + 1 && r.Height == FONT_ATLAS_DEFAULT_TEX_DATA_H);
        for (int y = 0, n = 0; y < FONT_ATLAS_DEFAULT_TEX_DATA_H; y++)
            for (int x = 0; x < FONT_ATLAS_DEFAULT_TEX_DATA_W_HALF; x++, n++)
            {
                const int offset0 = (int)(r.X + x) + (int)(r.Y + y) * w;
                const int offset1 = offset0 + FONT_ATLAS_DEFAULT_TEX_DATA_W_HALF + 1;
                atlas->TexPixelsAlpha8[offset0] = FONT_ATLAS_DEFAULT_TEX_DATA_PIXELS[n] == '.' ? 0xFF : 0x00;
                atlas->TexPixelsAlpha8[offset1] = FONT_ATLAS_DEFAULT_TEX_DATA_PIXELS[n] == 'X' ? 0xFF : 0x00;
            }
    }
    else
    {
        IM_ASSERT(r.Width == 2 && r.Height == 2);
        const int offset = (int)(r.X) + (int)(r.Y) * w;
        atlas->TexPixelsAlpha8[offset] = atlas->TexPixelsAlpha8[offset + 1] = atlas->TexPixelsAlpha8[offset + w] = atlas->TexPixelsAlpha8[offset + w + 1] = 0xFF;
    }
    atlas->TexUvWhitePixel = ImVec2((r.X + 0.5f) * atlas->TexUvScale.x, (r.Y + 0.5f) * atlas->TexUvScale.y);
}

void ImFontAtlasBuildFinish(ImFontAtlas* atlas)
{
    // Render into our custom data block
    ImFontAtlasBuildRenderDefaultTexData(atlas);

    // Register custom rectangle glyphs
    for (int i = 0; i < atlas->CustomRects.Size; i++)
    {
        const ImFontAtlas::CustomRect& r = atlas->CustomRects[i];
        if (r.Font == NULL || r.ID > 0x10000)
            continue;

        IM_ASSERT(r.Font->ContainerAtlas == atlas);
        ImVec2 uv0, uv1;
        atlas->CalcCustomRectUV(&r, &uv0, &uv1);
        r.Font->AddGlyph((ImWchar)r.ID, r.GlyphOffset.x, r.GlyphOffset.y, r.GlyphOffset.x + r.Width, r.GlyphOffset.y + r.Height, uv0.x, uv0.y, uv1.x, uv1.y, r.GlyphAdvanceX);
    }

    // Build all fonts lookup tables
    for (int i = 0; i < atlas->Fonts.Size; i++)
        if (atlas->Fonts[i]->DirtyLookupTables)
            atlas->Fonts[i]->BuildLookupTable();
}

// Retrieve list of range (2 int per range, values are inclusive)
const ImWchar*   ImFontAtlas::GetGlyphRangesDefault()
{
    static const ImWchar ranges[] =
    {
        0x0020, 0x00FF, // Basic Latin + Latin Supplement
        0,
    };
    return &ranges[0];
}

const ImWchar*  ImFontAtlas::GetGlyphRangesKorean()
{
    static const ImWchar ranges[] =
    {
        0x0020, 0x00FF, // Basic Latin + Latin Supplement
        0x3131, 0x3163, // Korean alphabets
        0xAC00, 0xD79D, // Korean characters
        0,
    };
    return &ranges[0];
}

const ImWchar*  ImFontAtlas::GetGlyphRangesChineseFull()
{
    static const ImWchar ranges[] =
    {
        0x0020, 0x00FF, // Basic Latin + Latin Supplement
        0x2000, 0x206F, // General Punctuation
        0x3000, 0x30FF, // CJK Symbols and Punctuations, Hiragana, Katakana
        0x31F0, 0x31FF, // Katakana Phonetic Extensions
        0xFF00, 0xFFEF, // Half-width characters
        0x4e00, 0x9FAF, // CJK Ideograms
        0,
    };
    return &ranges[0];
}

static void UnpackAccumulativeOffsetsIntoRanges(int base_codepoint, const short* accumulative_offsets, int accumulative_offsets_count, ImWchar* out_ranges)
{
    for (int n = 0; n < accumulative_offsets_count; n++, out_ranges += 2)
    {
        out_ranges[0] = out_ranges[1] = (ImWchar)(base_codepoint + accumulative_offsets[n]);
        base_codepoint += accumulative_offsets[n];
    }
    out_ranges[0] = 0;
}

//-------------------------------------------------------------------------
// [SECTION] ImFontAtlas glyph ranges helpers
//-------------------------------------------------------------------------

const ImWchar*  ImFontAtlas::GetGlyphRangesChineseSimplifiedCommon()
{
    // Store 2500 regularly used characters for Simplified Chinese.
    // Sourced from https://zh.wiktionary.org/wiki/%E9%99%84%E5%BD%95:%E7%8E%B0%E4%BB%A3%E6%B1%89%E8%AF%AD%E5%B8%B8%E7%94%A8%E5%AD%97%E8%A1%A8
    // This table covers 97.97% of all characters used during the month in July, 1987.
    // You can use ImFontGlyphRangesBuilder to create your own ranges derived from this, by merging existing ranges or adding new characters.
    // (Stored as accumulative offsets from the initial unicode codepoint 0x4E00. This encoding is designed to helps us compact the source code size.)
    static const short accumulative_offsets_from_0x4E00[] =
    {
        0,1,2,4,1,1,1,1,2,1,3,2,1,2,2,1,1,1,1,1,5,2,1,2,3,3,3,2,2,4,1,1,1,2,1,5,2,3,1,2,1,2,1,1,2,1,1,2,2,1,4,1,1,1,1,5,10,1,2,19,2,1,2,1,2,1,2,1,2,
        1,5,1,6,3,2,1,2,2,1,1,1,4,8,5,1,1,4,1,1,3,1,2,1,5,1,2,1,1,1,10,1,1,5,2,4,6,1,4,2,2,2,12,2,1,1,6,1,1,1,4,1,1,4,6,5,1,4,2,2,4,10,7,1,1,4,2,4,
        2,1,4,3,6,10,12,5,7,2,14,2,9,1,1,6,7,10,4,7,13,1,5,4,8,4,1,1,2,28,5,6,1,1,5,2,5,20,2,2,9,8,11,2,9,17,1,8,6,8,27,4,6,9,20,11,27,6,68,2,2,1,1,
        1,2,1,2,2,7,6,11,3,3,1,1,3,1,2,1,1,1,1,1,3,1,1,8,3,4,1,5,7,2,1,4,4,8,4,2,1,2,1,1,4,5,6,3,6,2,12,3,1,3,9,2,4,3,4,1,5,3,3,1,3,7,1,5,1,1,1,1,2,
        3,4,5,2,3,2,6,1,1,2,1,7,1,7,3,4,5,15,2,2,1,5,3,22,19,2,1,1,1,1,2,5,1,1,1,6,1,1,12,8,2,9,18,22,4,1,1,5,1,16,1,2,7,10,15,1,1,6,2,4,1,2,4,1,6,
        1,1,3,2,4,1,6,4,5,1,2,1,1,2,1,10,3,1,3,2,1,9,3,2,5,7,2,19,4,3,6,1,1,1,1,1,4,3,2,1,1,1,2,5,3,1,1,1,2,2,1,1,2,1,1,2,1,3,1,1,1,3,7,1,4,1,1,2,1,
        1,2,1,2,4,4,3,8,1,1,1,2,1,3,5,1,3,1,3,4,6,2,2,14,4,6,6,11,9,1,15,3,1,28,5,2,5,5,3,1,3,4,5,4,6,14,3,2,3,5,21,2,7,20,10,1,2,19,2,4,28,28,2,3,
        2,1,14,4,1,26,28,42,12,40,3,52,79,5,14,17,3,2,2,11,3,4,6,3,1,8,2,23,4,5,8,10,4,2,7,3,5,1,1,6,3,1,2,2,2,5,28,1,1,7,7,20,5,3,29,3,17,26,1,8,4,
        27,3,6,11,23,5,3,4,6,13,24,16,6,5,10,25,35,7,3,2,3,3,14,3,6,2,6,1,4,2,3,8,2,1,1,3,3,3,4,1,1,13,2,2,4,5,2,1,14,14,1,2,2,1,4,5,2,3,1,14,3,12,
        3,17,2,16,5,1,2,1,8,9,3,19,4,2,2,4,17,25,21,20,28,75,1,10,29,103,4,1,2,1,1,4,2,4,1,2,3,24,2,2,2,1,1,2,1,3,8,1,1,1,2,1,1,3,1,1,1,6,1,5,3,1,1,
        1,3,4,1,1,5,2,1,5,6,13,9,16,1,1,1,1,3,2,3,2,4,5,2,5,2,2,3,7,13,7,2,2,1,1,1,1,2,3,3,2,1,6,4,9,2,1,14,2,14,2,1,18,3,4,14,4,11,41,15,23,15,23,
        176,1,3,4,1,1,1,1,5,3,1,2,3,7,3,1,1,2,1,2,4,4,6,2,4,1,9,7,1,10,5,8,16,29,1,1,2,2,3,1,3,5,2,4,5,4,1,1,2,2,3,3,7,1,6,10,1,17,1,44,4,6,2,1,1,6,
        5,4,2,10,1,6,9,2,8,1,24,1,2,13,7,8,8,2,1,4,1,3,1,3,3,5,2,5,10,9,4,9,12,2,1,6,1,10,1,1,7,7,4,10,8,3,1,13,4,3,1,6,1,3,5,2,1,2,17,16,5,2,16,6,
        1,4,2,1,3,3,6,8,5,11,11,1,3,3,2,4,6,10,9,5,7,4,7,4,7,1,1,4,2,1,3,6,8,7,1,6,11,5,5,3,24,9,4,2,7,13,5,1,8,82,16,61,1,1,1,4,2,2,16,10,3,8,1,1,
        6,4,2,1,3,1,1,1,4,3,8,4,2,2,1,1,1,1,1,6,3,5,1,1,4,6,9,2,1,1,1,2,1,7,2,1,6,1,5,4,4,3,1,8,1,3,3,1,3,2,2,2,2,3,1,6,1,2,1,2,1,3,7,1,8,2,1,2,1,5,
        2,5,3,5,10,1,2,1,1,3,2,5,11,3,9,3,5,1,1,5,9,1,2,1,5,7,9,9,8,1,3,3,3,6,8,2,3,2,1,1,32,6,1,2,15,9,3,7,13,1,3,10,13,2,14,1,13,10,2,1,3,10,4,15,
        2,15,15,10,1,3,9,6,9,32,25,26,47,7,3,2,3,1,6,3,4,3,2,8,5,4,1,9,4,2,2,19,10,6,2,3,8,1,2,2,4,2,1,9,4,4,4,6,4,8,9,2,3,1,1,1,1,3,5,5,1,3,8,4,6,
        2,1,4,12,1,5,3,7,13,2,5,8,1,6,1,2,5,14,6,1,5,2,4,8,15,5,1,23,6,62,2,10,1,1,8,1,2,2,10,4,2,2,9,2,1,1,3,2,3,1,5,3,3,2,1,3,8,1,1,1,11,3,1,1,4,
        3,7,1,14,1,2,3,12,5,2,5,1,6,7,5,7,14,11,1,3,1,8,9,12,2,1,11,8,4,4,2,6,10,9,13,1,1,3,1,5,1,3,2,4,4,1,18,2,3,14,11,4,29,4,2,7,1,3,13,9,2,2,5,
        3,5,20,7,16,8,5,72,34,6,4,22,12,12,28,45,36,9,7,39,9,191,1,1,1,4,11,8,4,9,2,3,22,1,1,1,1,4,17,1,7,7,1,11,31,10,2,4,8,2,3,2,1,4,2,16,4,32,2,
        3,19,13,4,9,1,5,2,14,8,1,1,3,6,19,6,5,1,16,6,2,10,8,5,1,2,3,1,5,5,1,11,6,6,1,3,3,2,6,3,8,1,1,4,10,7,5,7,7,5,8,9,2,1,3,4,1,1,3,1,3,3,2,6,16,
        1,4,6,3,1,10,6,1,3,15,2,9,2,10,25,13,9,16,6,2,2,10,11,4,3,9,1,2,6,6,5,4,30,40,1,10,7,12,14,33,6,3,6,7,3,1,3,1,11,14,4,9,5,12,11,49,18,51,31,
        140,31,2,2,1,5,1,8,1,10,1,4,4,3,24,1,10,1,3,6,6,16,3,4,5,2,1,4,2,57,10,6,22,2,22,3,7,22,6,10,11,36,18,16,33,36,2,5,5,1,1,1,4,10,1,4,13,2,7,
        5,2,9,3,4,1,7,43,3,7,3,9,14,7,9,1,11,1,1,3,7,4,18,13,1,14,1,3,6,10,73,2,2,30,6,1,11,18,19,13,22,3,46,42,37,89,7,3,16,34,2,2,3,9,1,7,1,1,1,2,
        2,4,10,7,3,10,3,9,5,28,9,2,6,13,7,3,1,3,10,2,7,2,11,3,6,21,54,85,2,1,4,2,2,1,39,3,21,2,2,5,1,1,1,4,1,1,3,4,15,1,3,2,4,4,2,3,8,2,20,1,8,7,13,
        4,1,26,6,2,9,34,4,21,52,10,4,4,1,5,12,2,11,1,7,2,30,12,44,2,30,1,1,3,6,16,9,17,39,82,2,2,24,7,1,7,3,16,9,14,44,2,1,2,1,2,3,5,2,4,1,6,7,5,3,
        2,6,1,11,5,11,2,1,18,19,8,1,3,24,29,2,1,3,5,2,2,1,13,6,5,1,46,11,3,5,1,1,5,8,2,10,6,12,6,3,7,11,2,4,16,13,2,5,1,1,2,2,5,2,28,5,2,23,10,8,4,
        4,22,39,95,38,8,14,9,5,1,13,5,4,3,13,12,11,1,9,1,27,37,2,5,4,4,63,211,95,2,2,2,1,3,5,2,1,1,2,2,1,1,1,3,2,4,1,2,1,1,5,2,2,1,1,2,3,1,3,1,1,1,
        3,1,4,2,1,3,6,1,1,3,7,15,5,3,2,5,3,9,11,4,2,22,1,6,3,8,7,1,4,28,4,16,3,3,25,4,4,27,27,1,4,1,2,2,7,1,3,5,2,28,8,2,14,1,8,6,16,25,3,3,3,14,3,
        3,1,1,2,1,4,6,3,8,4,1,1,1,2,3,6,10,6,2,3,18,3,2,5,5,4,3,1,5,2,5,4,23,7,6,12,6,4,17,11,9,5,1,1,10,5,12,1,1,11,26,33,7,3,6,1,17,7,1,5,12,1,11,
        2,4,1,8,14,17,23,1,2,1,7,8,16,11,9,6,5,2,6,4,16,2,8,14,1,11,8,9,1,1,1,9,25,4,11,19,7,2,15,2,12,8,52,7,5,19,2,16,4,36,8,1,16,8,24,26,4,6,2,9,
        5,4,36,3,28,12,25,15,37,27,17,12,59,38,5,32,127,1,2,9,17,14,4,1,2,1,1,8,11,50,4,14,2,19,16,4,17,5,4,5,26,12,45,2,23,45,104,30,12,8,3,10,2,2,
        3,3,1,4,20,7,2,9,6,15,2,20,1,3,16,4,11,15,6,134,2,5,59,1,2,2,2,1,9,17,3,26,137,10,211,59,1,2,4,1,4,1,1,1,2,6,2,3,1,1,2,3,2,3,1,3,4,4,2,3,3,
        1,4,3,1,7,2,2,3,1,2,1,3,3,3,2,2,3,2,1,3,14,6,1,3,2,9,6,15,27,9,34,145,1,1,2,1,1,1,1,2,1,1,1,1,2,2,2,3,1,2,1,1,1,2,3,5,8,3,5,2,4,1,3,2,2,2,12,
        4,1,1,1,10,4,5,1,20,4,16,1,15,9,5,12,2,9,2,5,4,2,26,19,7,1,26,4,30,12,15,42,1,6,8,172,1,1,4,2,1,1,11,2,2,4,2,1,2,1,10,8,1,2,1,4,5,1,2,5,1,8,
        4,1,3,4,2,1,6,2,1,3,4,1,2,1,1,1,1,12,5,7,2,4,3,1,1,1,3,3,6,1,2,2,3,3,3,2,1,2,12,14,11,6,6,4,12,2,8,1,7,10,1,35,7,4,13,15,4,3,23,21,28,52,5,
        26,5,6,1,7,10,2,7,53,3,2,1,1,1,2,163,532,1,10,11,1,3,3,4,8,2,8,6,2,2,23,22,4,2,2,4,2,1,3,1,3,3,5,9,8,2,1,2,8,1,10,2,12,21,20,15,105,2,3,1,1,
        3,2,3,1,1,2,5,1,4,15,11,19,1,1,1,1,5,4,5,1,1,2,5,3,5,12,1,2,5,1,11,1,1,15,9,1,4,5,3,26,8,2,1,3,1,1,15,19,2,12,1,2,5,2,7,2,19,2,20,6,26,7,5,
        2,2,7,34,21,13,70,2,128,1,1,2,1,1,2,1,1,3,2,2,2,15,1,4,1,3,4,42,10,6,1,49,85,8,1,2,1,1,4,4,2,3,6,1,5,7,4,3,211,4,1,2,1,2,5,1,2,4,2,2,6,5,6,
        10,3,4,48,100,6,2,16,296,5,27,387,2,2,3,7,16,8,5,38,15,39,21,9,10,3,7,59,13,27,21,47,5,21,6
    };
    static ImWchar base_ranges[] = // not zero-terminated
    {
        0x0020, 0x00FF, // Basic Latin + Latin Supplement
        0x2000, 0x206F, // General Punctuation
        0x3000, 0x30FF, // CJK Symbols and Punctuations, Hiragana, Katakana
        0x31F0, 0x31FF, // Katakana Phonetic Extensions
        0xFF00, 0xFFEF  // Half-width characters
    };
    static ImWchar full_ranges[IM_ARRAYSIZE(base_ranges) + IM_ARRAYSIZE(accumulative_offsets_from_0x4E00) * 2 + 1] = { 0 };
    if (!full_ranges[0])
    {
        memcpy(full_ranges, base_ranges, sizeof(base_ranges));
        UnpackAccumulativeOffsetsIntoRanges(0x4E00, accumulative_offsets_from_0x4E00, IM_ARRAYSIZE(accumulative_offsets_from_0x4E00), full_ranges + IM_ARRAYSIZE(base_ranges));
    }
    return &full_ranges[0];
}

const ImWchar*  ImFontAtlas::GetGlyphRangesJapanese()
{
    // 1946 common ideograms code points for Japanese
    // Sourced from http://theinstructionlimit.com/common-kanji-character-ranges-for-xna-spritefont-rendering
    // FIXME: Source a list of the revised 2136 Joyo Kanji list from 2010 and rebuild this.
    // You can use ImFontGlyphRangesBuilder to create your own ranges derived from this, by merging existing ranges or adding new characters.
    // (Stored as accumulative offsets from the initial unicode codepoint 0x4E00. This encoding is designed to helps us compact the source code size.)
    static const short accumulative_offsets_from_0x4E00[] =
    {
        0,1,2,4,1,1,1,1,2,1,6,2,2,1,8,5,7,11,1,2,10,10,8,2,4,20,2,11,8,2,1,2,1,6,2,1,7,5,3,7,1,1,13,7,9,1,4,6,1,2,1,10,1,1,9,2,2,4,5,6,14,1,1,9,3,18,
        5,4,2,2,10,7,1,1,1,3,2,4,3,23,2,10,12,2,14,2,4,13,1,6,10,3,1,7,13,6,4,13,5,2,3,17,2,2,5,7,6,4,1,7,14,16,6,13,9,15,1,1,7,16,4,7,1,19,9,2,7,15,
        2,6,5,13,25,4,14,13,11,25,1,1,1,2,1,2,2,3,10,11,3,3,1,1,4,4,2,1,4,9,1,4,3,5,5,2,7,12,11,15,7,16,4,5,16,2,1,1,6,3,3,1,1,2,7,6,6,7,1,4,7,6,1,1,
        2,1,12,3,3,9,5,8,1,11,1,2,3,18,20,4,1,3,6,1,7,3,5,5,7,2,2,12,3,1,4,2,3,2,3,11,8,7,4,17,1,9,25,1,1,4,2,2,4,1,2,7,1,1,1,3,1,2,6,16,1,2,1,1,3,12,
        20,2,5,20,8,7,6,2,1,1,1,1,6,2,1,2,10,1,1,6,1,3,1,2,1,4,1,12,4,1,3,1,1,1,1,1,10,4,7,5,13,1,15,1,1,30,11,9,1,15,38,14,1,32,17,20,1,9,31,2,21,9,
        4,49,22,2,1,13,1,11,45,35,43,55,12,19,83,1,3,2,3,13,2,1,7,3,18,3,13,8,1,8,18,5,3,7,25,24,9,24,40,3,17,24,2,1,6,2,3,16,15,6,7,3,12,1,9,7,3,3,
        3,15,21,5,16,4,5,12,11,11,3,6,3,2,31,3,2,1,1,23,6,6,1,4,2,6,5,2,1,1,3,3,22,2,6,2,3,17,3,2,4,5,1,9,5,1,1,6,15,12,3,17,2,14,2,8,1,23,16,4,2,23,
        8,15,23,20,12,25,19,47,11,21,65,46,4,3,1,5,6,1,2,5,26,2,1,1,3,11,1,1,1,2,1,2,3,1,1,10,2,3,1,1,1,3,6,3,2,2,6,6,9,2,2,2,6,2,5,10,2,4,1,2,1,2,2,
        3,1,1,3,1,2,9,23,9,2,1,1,1,1,5,3,2,1,10,9,6,1,10,2,31,25,3,7,5,40,1,15,6,17,7,27,180,1,3,2,2,1,1,1,6,3,10,7,1,3,6,17,8,6,2,2,1,3,5,5,8,16,14,
        15,1,1,4,1,2,1,1,1,3,2,7,5,6,2,5,10,1,4,2,9,1,1,11,6,1,44,1,3,7,9,5,1,3,1,1,10,7,1,10,4,2,7,21,15,7,2,5,1,8,3,4,1,3,1,6,1,4,2,1,4,10,8,1,4,5,
        1,5,10,2,7,1,10,1,1,3,4,11,10,29,4,7,3,5,2,3,33,5,2,19,3,1,4,2,6,31,11,1,3,3,3,1,8,10,9,12,11,12,8,3,14,8,6,11,1,4,41,3,1,2,7,13,1,5,6,2,6,12,
        12,22,5,9,4,8,9,9,34,6,24,1,1,20,9,9,3,4,1,7,2,2,2,6,2,28,5,3,6,1,4,6,7,4,2,1,4,2,13,6,4,4,3,1,8,8,3,2,1,5,1,2,2,3,1,11,11,7,3,6,10,8,6,16,16,
        22,7,12,6,21,5,4,6,6,3,6,1,3,2,1,2,8,29,1,10,1,6,13,6,6,19,31,1,13,4,4,22,17,26,33,10,4,15,12,25,6,67,10,2,3,1,6,10,2,6,2,9,1,9,4,4,1,2,16,2,
        5,9,2,3,8,1,8,3,9,4,8,6,4,8,11,3,2,1,1,3,26,1,7,5,1,11,1,5,3,5,2,13,6,39,5,1,5,2,11,6,10,5,1,15,5,3,6,19,21,22,2,4,1,6,1,8,1,4,8,2,4,2,2,9,2,
        1,1,1,4,3,6,3,12,7,1,14,2,4,10,2,13,1,17,7,3,2,1,3,2,13,7,14,12,3,1,29,2,8,9,15,14,9,14,1,3,1,6,5,9,11,3,38,43,20,7,7,8,5,15,12,19,15,81,8,7,
        1,5,73,13,37,28,8,8,1,15,18,20,165,28,1,6,11,8,4,14,7,15,1,3,3,6,4,1,7,14,1,1,11,30,1,5,1,4,14,1,4,2,7,52,2,6,29,3,1,9,1,21,3,5,1,26,3,11,14,
        11,1,17,5,1,2,1,3,2,8,1,2,9,12,1,1,2,3,8,3,24,12,7,7,5,17,3,3,3,1,23,10,4,4,6,3,1,16,17,22,3,10,21,16,16,6,4,10,2,1,1,2,8,8,6,5,3,3,3,39,25,
        15,1,1,16,6,7,25,15,6,6,12,1,22,13,1,4,9,5,12,2,9,1,12,28,8,3,5,10,22,60,1,2,40,4,61,63,4,1,13,12,1,4,31,12,1,14,89,5,16,6,29,14,2,5,49,18,18,
        5,29,33,47,1,17,1,19,12,2,9,7,39,12,3,7,12,39,3,1,46,4,12,3,8,9,5,31,15,18,3,2,2,66,19,13,17,5,3,46,124,13,57,34,2,5,4,5,8,1,1,1,4,3,1,17,5,
        3,5,3,1,8,5,6,3,27,3,26,7,12,7,2,17,3,7,18,78,16,4,36,1,2,1,6,2,1,39,17,7,4,13,4,4,4,1,10,4,2,4,6,3,10,1,19,1,26,2,4,33,2,73,47,7,3,8,2,4,15,
        18,1,29,2,41,14,1,21,16,41,7,39,25,13,44,2,2,10,1,13,7,1,7,3,5,20,4,8,2,49,1,10,6,1,6,7,10,7,11,16,3,12,20,4,10,3,1,2,11,2,28,9,2,4,7,2,15,1,
        27,1,28,17,4,5,10,7,3,24,10,11,6,26,3,2,7,2,2,49,16,10,16,15,4,5,27,61,30,14,38,22,2,7,5,1,3,12,23,24,17,17,3,3,2,4,1,6,2,7,5,1,1,5,1,1,9,4,
        1,3,6,1,8,2,8,4,14,3,5,11,4,1,3,32,1,19,4,1,13,11,5,2,1,8,6,8,1,6,5,13,3,23,11,5,3,16,3,9,10,1,24,3,198,52,4,2,2,5,14,5,4,22,5,20,4,11,6,41,
        1,5,2,2,11,5,2,28,35,8,22,3,18,3,10,7,5,3,4,1,5,3,8,9,3,6,2,16,22,4,5,5,3,3,18,23,2,6,23,5,27,8,1,33,2,12,43,16,5,2,3,6,1,20,4,2,9,7,1,11,2,
        10,3,14,31,9,3,25,18,20,2,5,5,26,14,1,11,17,12,40,19,9,6,31,83,2,7,9,19,78,12,14,21,76,12,113,79,34,4,1,1,61,18,85,10,2,2,13,31,11,50,6,33,159,
        179,6,6,7,4,4,2,4,2,5,8,7,20,32,22,1,3,10,6,7,28,5,10,9,2,77,19,13,2,5,1,4,4,7,4,13,3,9,31,17,3,26,2,6,6,5,4,1,7,11,3,4,2,1,6,2,20,4,1,9,2,6,
        3,7,1,1,1,20,2,3,1,6,2,3,6,2,4,8,1,5,13,8,4,11,23,1,10,6,2,1,3,21,2,2,4,24,31,4,10,10,2,5,192,15,4,16,7,9,51,1,2,1,1,5,1,1,2,1,3,5,3,1,3,4,1,
        3,1,3,3,9,8,1,2,2,2,4,4,18,12,92,2,10,4,3,14,5,25,16,42,4,14,4,2,21,5,126,30,31,2,1,5,13,3,22,5,6,6,20,12,1,14,12,87,3,19,1,8,2,9,9,3,3,23,2,
        3,7,6,3,1,2,3,9,1,3,1,6,3,2,1,3,11,3,1,6,10,3,2,3,1,2,1,5,1,1,11,3,6,4,1,7,2,1,2,5,5,34,4,14,18,4,19,7,5,8,2,6,79,1,5,2,14,8,2,9,2,1,36,28,16,
        4,1,1,1,2,12,6,42,39,16,23,7,15,15,3,2,12,7,21,64,6,9,28,8,12,3,3,41,59,24,51,55,57,294,9,9,2,6,2,15,1,2,13,38,90,9,9,9,3,11,7,1,1,1,5,6,3,2,
        1,2,2,3,8,1,4,4,1,5,7,1,4,3,20,4,9,1,1,1,5,5,17,1,5,2,6,2,4,1,4,5,7,3,18,11,11,32,7,5,4,7,11,127,8,4,3,3,1,10,1,1,6,21,14,1,16,1,7,1,3,6,9,65,
        51,4,3,13,3,10,1,1,12,9,21,110,3,19,24,1,1,10,62,4,1,29,42,78,28,20,18,82,6,3,15,6,84,58,253,15,155,264,15,21,9,14,7,58,40,39,
    };
    static ImWchar base_ranges[] = // not zero-terminated
    {
        0x0020, 0x00FF, // Basic Latin + Latin Supplement
        0x3000, 0x30FF, // CJK Symbols and Punctuations, Hiragana, Katakana
        0x31F0, 0x31FF, // Katakana Phonetic Extensions
        0xFF00, 0xFFEF  // Half-width characters
    };
    static ImWchar full_ranges[IM_ARRAYSIZE(base_ranges) + IM_ARRAYSIZE(accumulative_offsets_from_0x4E00)*2 + 1] = { 0 };
    if (!full_ranges[0])
    {
        memcpy(full_ranges, base_ranges, sizeof(base_ranges));
        UnpackAccumulativeOffsetsIntoRanges(0x4E00, accumulative_offsets_from_0x4E00, IM_ARRAYSIZE(accumulative_offsets_from_0x4E00), full_ranges + IM_ARRAYSIZE(base_ranges));
    }
    return &full_ranges[0];
}

const ImWchar*  ImFontAtlas::GetGlyphRangesCyrillic()
{
    static const ImWchar ranges[] =
    {
        0x0020, 0x00FF, // Basic Latin + Latin Supplement
        0x0400, 0x052F, // Cyrillic + Cyrillic Supplement
        0x2DE0, 0x2DFF, // Cyrillic Extended-A
        0xA640, 0xA69F, // Cyrillic Extended-B
        0,
    };
    return &ranges[0];
}

const ImWchar*  ImFontAtlas::GetGlyphRangesThai()
{
    static const ImWchar ranges[] =
    {
        0x0020, 0x00FF, // Basic Latin
        0x2010, 0x205E, // Punctuations
        0x0E00, 0x0E7F, // Thai
        0,
    };
    return &ranges[0];
}

//-----------------------------------------------------------------------------
// [SECTION] ImFontGlyphRangesBuilder
//-----------------------------------------------------------------------------

void ImFontGlyphRangesBuilder::AddText(const char* text, const char* text_end)
{
    while (text_end ? (text < text_end) : *text)
    {
        unsigned int c = 0;
        int c_len = ImTextCharFromUtf8(&c, text, text_end);
        text += c_len;
        if (c_len == 0)
            break;
        if (c < 0x10000)
            AddChar((ImWchar)c);
    }
}

void ImFontGlyphRangesBuilder::AddRanges(const ImWchar* ranges)
{
    for (; ranges[0]; ranges += 2)
        for (ImWchar c = ranges[0]; c <= ranges[1]; c++)
            AddChar(c);
}

void ImFontGlyphRangesBuilder::BuildRanges(ImVector<ImWchar>* out_ranges)
{
    for (int n = 0; n < 0x10000; n++)
        if (GetBit(n))
        {
            out_ranges->push_back((ImWchar)n);
            while (n < 0x10000 && GetBit(n + 1))
                n++;
            out_ranges->push_back((ImWchar)n);
        }
    out_ranges->push_back(0);
}

//-----------------------------------------------------------------------------
// [SECTION] ImFont
//-----------------------------------------------------------------------------

ImFont::ImFont()
{
    FontSize = 0.0f;
    FallbackAdvanceX = 0.0f;
    FallbackChar = (ImWchar)'?';
    DisplayOffset = ImVec2(0.0f, 0.0f);
    FallbackGlyph = NULL;
    ContainerAtlas = NULL;
    ConfigData = NULL;
    ConfigDataCount = 0;
    DirtyLookupTables = false;
    Scale = 1.0f;
    Ascent = Descent = 0.0f;
    MetricsTotalSurface = 0;
}

ImFont::~ImFont()
{
    ClearOutputData();
}

void    ImFont::ClearOutputData()
{
    FontSize = 0.0f;
    FallbackAdvanceX = 0.0f;
    Glyphs.clear();
    IndexAdvanceX.clear();
    IndexLookup.clear();
    FallbackGlyph = NULL;
    ContainerAtlas = NULL;
    DirtyLookupTables = true;
    Ascent = Descent = 0.0f;
    MetricsTotalSurface = 0;
}

void ImFont::BuildLookupTable()
{
    int max_codepoint = 0;
    for (int i = 0; i != Glyphs.Size; i++)
        max_codepoint = ImMax(max_codepoint, (int)Glyphs[i].Codepoint);

    IM_ASSERT(Glyphs.Size < 0xFFFF); // -1 is reserved
    IndexAdvanceX.clear();
    IndexLookup.clear();
    DirtyLookupTables = false;
    GrowIndex(max_codepoint + 1);
    for (int i = 0; i < Glyphs.Size; i++)
    {
        int codepoint = (int)Glyphs[i].Codepoint;
        IndexAdvanceX[codepoint] = Glyphs[i].AdvanceX;
        IndexLookup[codepoint] = (ImWchar)i;
    }

    // Create a glyph to handle TAB
    // FIXME: Needs proper TAB handling but it needs to be contextualized (or we could arbitrary say that each string starts at "column 0" ?)
    if (FindGlyph((ImWchar)' '))
    {
        if (Glyphs.back().Codepoint != '\t')   // So we can call this function multiple times
            Glyphs.resize(Glyphs.Size + 1);
        ImFontGlyph& tab_glyph = Glyphs.back();
        tab_glyph = *FindGlyph((ImWchar)' ');
        tab_glyph.Codepoint = '\t';
        tab_glyph.AdvanceX *= 4;
        IndexAdvanceX[(int)tab_glyph.Codepoint] = (float)tab_glyph.AdvanceX;
        IndexLookup[(int)tab_glyph.Codepoint] = (ImWchar)(Glyphs.Size-1);
    }

    FallbackGlyph = FindGlyphNoFallback(FallbackChar);
    FallbackAdvanceX = FallbackGlyph ? FallbackGlyph->AdvanceX : 0.0f;
    for (int i = 0; i < max_codepoint + 1; i++)
        if (IndexAdvanceX[i] < 0.0f)
            IndexAdvanceX[i] = FallbackAdvanceX;
}

void ImFont::SetFallbackChar(ImWchar c)
{
    FallbackChar = c;
    BuildLookupTable();
}

void ImFont::GrowIndex(int new_size)
{
    IM_ASSERT(IndexAdvanceX.Size == IndexLookup.Size);
    if (new_size <= IndexLookup.Size)
        return;
    IndexAdvanceX.resize(new_size, -1.0f);
    IndexLookup.resize(new_size, (ImWchar)-1);
}

// x0/y0/x1/y1 are offset from the character upper-left layout position, in pixels. Therefore x0/y0 are often fairly close to zero.
// Not to be mistaken with texture coordinates, which are held by u0/v0/u1/v1 in normalized format (0.0..1.0 on each texture axis).
void ImFont::AddGlyph(ImWchar codepoint, float x0, float y0, float x1, float y1, float u0, float v0, float u1, float v1, float advance_x)
{
    Glyphs.resize(Glyphs.Size + 1);
    ImFontGlyph& glyph = Glyphs.back();
    glyph.Codepoint = (ImWchar)codepoint;
    glyph.X0 = x0;
    glyph.Y0 = y0;
    glyph.X1 = x1;
    glyph.Y1 = y1;
    glyph.U0 = u0;
    glyph.V0 = v0;
    glyph.U1 = u1;
    glyph.V1 = v1;
    glyph.AdvanceX = advance_x + ConfigData->GlyphExtraSpacing.x;  // Bake spacing into AdvanceX

    if (ConfigData->PixelSnapH)
        glyph.AdvanceX = (float)(int)(glyph.AdvanceX + 0.5f);

    // Compute rough surface usage metrics (+1 to account for average padding, +0.99 to round)
    DirtyLookupTables = true;
    MetricsTotalSurface += (int)((glyph.U1 - glyph.U0) * ContainerAtlas->TexWidth + 1.99f) * (int)((glyph.V1 - glyph.V0) * ContainerAtlas->TexHeight + 1.99f);
}

void ImFont::AddRemapChar(ImWchar dst, ImWchar src, bool overwrite_dst)
{
    IM_ASSERT(IndexLookup.Size > 0);    // Currently this can only be called AFTER the font has been built, aka after calling ImFontAtlas::GetTexDataAs*() function.
    int index_size = IndexLookup.Size;

    if (dst < index_size && IndexLookup.Data[dst] == (ImWchar)-1 && !overwrite_dst) // 'dst' already exists
        return;
    if (src >= index_size && dst >= index_size) // both 'dst' and 'src' don't exist -> no-op
        return;

    GrowIndex(dst + 1);
    IndexLookup[dst] = (src < index_size) ? IndexLookup.Data[src] : (ImWchar)-1;
    IndexAdvanceX[dst] = (src < index_size) ? IndexAdvanceX.Data[src] : 1.0f;
}

const ImFontGlyph* ImFont::FindGlyph(ImWchar c) const
{
    if (c >= IndexLookup.Size)
        return FallbackGlyph;
    const ImWchar i = IndexLookup.Data[c];
    if (i == (ImWchar)-1)
        return FallbackGlyph;
    return &Glyphs.Data[i];
}

const ImFontGlyph* ImFont::FindGlyphNoFallback(ImWchar c) const
{
    if (c >= IndexLookup.Size)
        return NULL;
    const ImWchar i = IndexLookup.Data[c];
    if (i == (ImWchar)-1)
        return NULL;
    return &Glyphs.Data[i];
}

const char* ImFont::CalcWordWrapPositionA(float scale, const char* text, const char* text_end, float wrap_width) const
{
    // Simple word-wrapping for English, not full-featured. Please submit failing cases!
    // FIXME: Much possible improvements (don't cut things like "word !", "word!!!" but cut within "word,,,,", more sensible support for punctuations, support for Unicode punctuations, etc.)

    // For references, possible wrap point marked with ^
    //  "aaa bbb, ccc,ddd. eee   fff. ggg!"
    //      ^    ^    ^   ^   ^__    ^    ^

    // List of hardcoded separators: .,;!?'"

    // Skip extra blanks after a line returns (that includes not counting them in width computation)
    // e.g. "Hello    world" --> "Hello" "World"

    // Cut words that cannot possibly fit within one line.
    // e.g.: "The tropical fish" with ~5 characters worth of width --> "The tr" "opical" "fish"

    float line_width = 0.0f;
    float word_width = 0.0f;
    float blank_width = 0.0f;
    wrap_width /= scale; // We work with unscaled widths to avoid scaling every characters

    const char* word_end = text;
    const char* prev_word_end = NULL;
    bool inside_word = true;

    const char* s = text;
    while (s < text_end)
    {
        unsigned int c = (unsigned int)*s;
        const char* next_s;
        if (c < 0x80)
            next_s = s + 1;
        else
            next_s = s + ImTextCharFromUtf8(&c, s, text_end);
        if (c == 0)
            break;

        if (c < 32)
        {
            if (c == '\n')
            {
                line_width = word_width = blank_width = 0.0f;
                inside_word = true;
                s = next_s;
                continue;
            }
            if (c == '\r')
            {
                s = next_s;
                continue;
            }
        }

        const float char_width = ((int)c < IndexAdvanceX.Size ? IndexAdvanceX.Data[c] : FallbackAdvanceX);
        if (ImCharIsBlankW(c))
        {
            if (inside_word)
            {
                line_width += blank_width;
                blank_width = 0.0f;
                word_end = s;
            }
            blank_width += char_width;
            inside_word = false;
        }
        else
        {
            word_width += char_width;
            if (inside_word)
            {
                word_end = next_s;
            }
            else
            {
                prev_word_end = word_end;
                line_width += word_width + blank_width;
                word_width = blank_width = 0.0f;
            }

            // Allow wrapping after punctuation.
            inside_word = !(c == '.' || c == ',' || c == ';' || c == '!' || c == '?' || c == '\"');
        }

        // We ignore blank width at the end of the line (they can be skipped)
        if (line_width + word_width >= wrap_width)
        {
            // Words that cannot possibly fit within an entire line will be cut anywhere.
            if (word_width < wrap_width)
                s = prev_word_end ? prev_word_end : word_end;
            break;
        }

        s = next_s;
    }

    return s;
}

ImVec2 ImFont::CalcTextSizeA(float size, float max_width, float wrap_width, const char* text_begin, const char* text_end, const char** remaining) const
{
    if (!text_end)
        text_end = text_begin + strlen(text_begin); // FIXME-OPT: Need to avoid this.

    const float line_height = size;
    const float scale = size / FontSize;

    ImVec2 text_size = ImVec2(0,0);
    float line_width = 0.0f;

    const bool word_wrap_enabled = (wrap_width > 0.0f);
    const char* word_wrap_eol = NULL;

    const char* s = text_begin;
    while (s < text_end)
    {
        if (word_wrap_enabled)
        {
            // Calculate how far we can render. Requires two passes on the string data but keeps the code simple and not intrusive for what's essentially an uncommon feature.
            if (!word_wrap_eol)
            {
                word_wrap_eol = CalcWordWrapPositionA(scale, s, text_end, wrap_width - line_width);
                if (word_wrap_eol == s) // Wrap_width is too small to fit anything. Force displaying 1 character to minimize the height discontinuity.
                    word_wrap_eol++;    // +1 may not be a character start point in UTF-8 but it's ok because we use s >= word_wrap_eol below
            }

            if (s >= word_wrap_eol)
            {
                if (text_size.x < line_width)
                    text_size.x = line_width;
                text_size.y += line_height;
                line_width = 0.0f;
                word_wrap_eol = NULL;

                // Wrapping skips upcoming blanks
                while (s < text_end)
                {
                    const char c = *s;
                    if (ImCharIsBlankA(c)) { s++; } else if (c == '\n') { s++; break; } else { break; }
                }
                continue;
            }
        }

        // Decode and advance source
        const char* prev_s = s;
        unsigned int c = (unsigned int)*s;
        if (c < 0x80)
        {
            s += 1;
        }
        else
        {
            s += ImTextCharFromUtf8(&c, s, text_end);
            if (c == 0) // Malformed UTF-8?
                break;
        }

        if (c < 32)
        {
            if (c == '\n')
            {
                text_size.x = ImMax(text_size.x, line_width);
                text_size.y += line_height;
                line_width = 0.0f;
                continue;
            }
            if (c == '\r')
                continue;
        }

        const float char_width = ((int)c < IndexAdvanceX.Size ? IndexAdvanceX.Data[c] : FallbackAdvanceX) * scale;
        if (line_width + char_width >= max_width)
        {
            s = prev_s;
            break;
        }

        line_width += char_width;
    }

    if (text_size.x < line_width)
        text_size.x = line_width;

    if (line_width > 0 || text_size.y == 0.0f)
        text_size.y += line_height;

    if (remaining)
        *remaining = s;

    return text_size;
}

void ImFont::RenderChar(ImDrawList* draw_list, float size, ImVec2 pos, ImU32 col, ImWchar c) const
{
    if (c == ' ' || c == '\t' || c == '\n' || c == '\r') // Match behavior of RenderText(), those 4 codepoints are hard-coded.
        return;
    if (const ImFontGlyph* glyph = FindGlyph(c))
    {
        float scale = (size >= 0.0f) ? (size / FontSize) : 1.0f;
        pos.x = (float)(int)pos.x + DisplayOffset.x;
        pos.y = (float)(int)pos.y + DisplayOffset.y;
        draw_list->PrimReserve(6, 4);
        draw_list->PrimRectUV(ImVec2(pos.x + glyph->X0 * scale, pos.y + glyph->Y0 * scale), ImVec2(pos.x + glyph->X1 * scale, pos.y + glyph->Y1 * scale), ImVec2(glyph->U0, glyph->V0), ImVec2(glyph->U1, glyph->V1), col);
    }
}

void ImFont::RenderText(ImDrawList* draw_list, float size, ImVec2 pos, ImU32 col, const ImVec4& clip_rect, const char* text_begin, const char* text_end, float wrap_width, bool cpu_fine_clip) const
{
    if (!text_end)
        text_end = text_begin + strlen(text_begin); // ImGui functions generally already provides a valid text_end, so this is merely to handle direct calls.

    // Align to be pixel perfect
    pos.x = (float)(int)pos.x + DisplayOffset.x;
    pos.y = (float)(int)pos.y + DisplayOffset.y;
    float x = pos.x;
    float y = pos.y;
    if (y > clip_rect.w)
        return;

    const float scale = size / FontSize;
    const float line_height = FontSize * scale;
    const bool word_wrap_enabled = (wrap_width > 0.0f);
    const char* word_wrap_eol = NULL;

    // Fast-forward to first visible line
    const char* s = text_begin;
    if (y + line_height < clip_rect.y && !word_wrap_enabled)
        while (y + line_height < clip_rect.y && s < text_end)
        {
            s = (const char*)memchr(s, '\n', text_end - s);
            s = s ? s + 1 : text_end;
            y += line_height;
        }

    // For large text, scan for the last visible line in order to avoid over-reserving in the call to PrimReserve()
    // Note that very large horizontal line will still be affected by the issue (e.g. a one megabyte string buffer without a newline will likely crash atm)
    if (text_end - s > 10000 && !word_wrap_enabled)
    {
        const char* s_end = s;
        float y_end = y;
        while (y_end < clip_rect.w && s_end < text_end)
        {
            s_end = (const char*)memchr(s_end, '\n', text_end - s_end);
            s_end = s_end ? s_end + 1 : text_end;
            y_end += line_height;
        }
        text_end = s_end;
    }
    if (s == text_end)
        return;

    // Reserve vertices for remaining worse case (over-reserving is useful and easily amortized)
    const int vtx_count_max = (int)(text_end - s) * 4;
    const int idx_count_max = (int)(text_end - s) * 6;
    const int idx_expected_size = draw_list->IdxBuffer.Size + idx_count_max;
    draw_list->PrimReserve(idx_count_max, vtx_count_max);

    ImDrawVert* vtx_write = draw_list->_VtxWritePtr;
    ImDrawIdx* idx_write = draw_list->_IdxWritePtr;
    unsigned int vtx_current_idx = draw_list->_VtxCurrentIdx;

    while (s < text_end)
    {
        if (word_wrap_enabled)
        {
            // Calculate how far we can render. Requires two passes on the string data but keeps the code simple and not intrusive for what's essentially an uncommon feature.
            if (!word_wrap_eol)
            {
                word_wrap_eol = CalcWordWrapPositionA(scale, s, text_end, wrap_width - (x - pos.x));
                if (word_wrap_eol == s) // Wrap_width is too small to fit anything. Force displaying 1 character to minimize the height discontinuity.
                    word_wrap_eol++;    // +1 may not be a character start point in UTF-8 but it's ok because we use s >= word_wrap_eol below
            }

            if (s >= word_wrap_eol)
            {
                x = pos.x;
                y += line_height;
                word_wrap_eol = NULL;

                // Wrapping skips upcoming blanks
                while (s < text_end)
                {
                    const char c = *s;
                    if (ImCharIsBlankA(c)) { s++; } else if (c == '\n') { s++; break; } else { break; }
                }
                continue;
            }
        }

        // Decode and advance source
        unsigned int c = (unsigned int)*s;
        if (c < 0x80)
        {
            s += 1;
        }
        else
        {
            s += ImTextCharFromUtf8(&c, s, text_end);
            if (c == 0) // Malformed UTF-8?
                break;
        }

        if (c < 32)
        {
            if (c == '\n')
            {
                x = pos.x;
                y += line_height;
                if (y > clip_rect.w)
                    break; // break out of main loop
                continue;
            }
            if (c == '\r')
                continue;
        }

        float char_width = 0.0f;
        if (const ImFontGlyph* glyph = FindGlyph((ImWchar)c))
        {
            char_width = glyph->AdvanceX * scale;

            // Arbitrarily assume that both space and tabs are empty glyphs as an optimization
            if (c != ' ' && c != '\t')
            {
                // We don't do a second finer clipping test on the Y axis as we've already skipped anything before clip_rect.y and exit once we pass clip_rect.w
                float x1 = x + glyph->X0 * scale;
                float x2 = x + glyph->X1 * scale;
                float y1 = y + glyph->Y0 * scale;
                float y2 = y + glyph->Y1 * scale;
                if (x1 <= clip_rect.z && x2 >= clip_rect.x)
                {
                    // Render a character
                    float u1 = glyph->U0;
                    float v1 = glyph->V0;
                    float u2 = glyph->U1;
                    float v2 = glyph->V1;

                    // CPU side clipping used to fit text in their frame when the frame is too small. Only does clipping for axis aligned quads.
                    if (cpu_fine_clip)
                    {
                        if (x1 < clip_rect.x)
                        {
                            u1 = u1 + (1.0f - (x2 - clip_rect.x) / (x2 - x1)) * (u2 - u1);
                            x1 = clip_rect.x;
                        }
                        if (y1 < clip_rect.y)
                        {
                            v1 = v1 + (1.0f - (y2 - clip_rect.y) / (y2 - y1)) * (v2 - v1);
                            y1 = clip_rect.y;
                        }
                        if (x2 > clip_rect.z)
                        {
                            u2 = u1 + ((clip_rect.z - x1) / (x2 - x1)) * (u2 - u1);
                            x2 = clip_rect.z;
                        }
                        if (y2 > clip_rect.w)
                        {
                            v2 = v1 + ((clip_rect.w - y1) / (y2 - y1)) * (v2 - v1);
                            y2 = clip_rect.w;
                        }
                        if (y1 >= y2)
                        {
                            x += char_width;
                            continue;
                        }
                    }

                    // We are NOT calling PrimRectUV() here because non-inlined causes too much overhead in a debug builds. Inlined here:
                    {
                        idx_write[0] = (ImDrawIdx)(vtx_current_idx); idx_write[1] = (ImDrawIdx)(vtx_current_idx+1); idx_write[2] = (ImDrawIdx)(vtx_current_idx+2);
                        idx_write[3] = (ImDrawIdx)(vtx_current_idx); idx_write[4] = (ImDrawIdx)(vtx_current_idx+2); idx_write[5] = (ImDrawIdx)(vtx_current_idx+3);
                        vtx_write[0].pos.x = x1; vtx_write[0].pos.y = y1; vtx_write[0].col = col; vtx_write[0].uv.x = u1; vtx_write[0].uv.y = v1;
                        vtx_write[1].pos.x = x2; vtx_write[1].pos.y = y1; vtx_write[1].col = col; vtx_write[1].uv.x = u2; vtx_write[1].uv.y = v1;
                        vtx_write[2].pos.x = x2; vtx_write[2].pos.y = y2; vtx_write[2].col = col; vtx_write[2].uv.x = u2; vtx_write[2].uv.y = v2;
                        vtx_write[3].pos.x = x1; vtx_write[3].pos.y = y2; vtx_write[3].col = col; vtx_write[3].uv.x = u1; vtx_write[3].uv.y = v2;
                        vtx_write += 4;
                        vtx_current_idx += 4;
                        idx_write += 6;
                    }
                }
            }
        }

        x += char_width;
    }

    // Give back unused vertices
    draw_list->VtxBuffer.resize((int)(vtx_write - draw_list->VtxBuffer.Data));
    draw_list->IdxBuffer.resize((int)(idx_write - draw_list->IdxBuffer.Data));
    draw_list->CmdBuffer[draw_list->CmdBuffer.Size-1].ElemCount -= (idx_expected_size - draw_list->IdxBuffer.Size);
    draw_list->_VtxWritePtr = vtx_write;
    draw_list->_IdxWritePtr = idx_write;
    draw_list->_VtxCurrentIdx = (unsigned int)draw_list->VtxBuffer.Size;
}

//-----------------------------------------------------------------------------
// [SECTION] Internal Render Helpers
// (progressively moved from imgui.cpp to here when they are redesigned to stop accessing ImGui global state)
//-----------------------------------------------------------------------------
// - RenderMouseCursor()
// - RenderArrowPointingAt()
// - RenderRectFilledRangeH()
// - RenderPixelEllipsis()
//-----------------------------------------------------------------------------

void ImGui::RenderMouseCursor(ImDrawList* draw_list, ImVec2 pos, float scale, ImGuiMouseCursor mouse_cursor)
{
    if (mouse_cursor == ImGuiMouseCursor_None)
        return;
    IM_ASSERT(mouse_cursor > ImGuiMouseCursor_None && mouse_cursor < ImGuiMouseCursor_COUNT);

    const ImU32 col_shadow = IM_COL32(0, 0, 0, 48);
    const ImU32 col_border = IM_COL32(0, 0, 0, 255);          // Black
    const ImU32 col_fill   = IM_COL32(255, 255, 255, 255);    // White

    ImFontAtlas* font_atlas = draw_list->_Data->Font->ContainerAtlas;
    ImVec2 offset, size, uv[4];
    if (font_atlas->GetMouseCursorTexData(mouse_cursor, &offset, &size, &uv[0], &uv[2]))
    {
        pos -= offset;
        const ImTextureID tex_id = font_atlas->TexID;
        draw_list->PushTextureID(tex_id);
        draw_list->AddImage(tex_id, pos + ImVec2(1,0)*scale, pos + ImVec2(1,0)*scale + size*scale, uv[2], uv[3], col_shadow);
        draw_list->AddImage(tex_id, pos + ImVec2(2,0)*scale, pos + ImVec2(2,0)*scale + size*scale, uv[2], uv[3], col_shadow);
        draw_list->AddImage(tex_id, pos,                     pos + size*scale,                     uv[2], uv[3], col_border);
        draw_list->AddImage(tex_id, pos,                     pos + size*scale,                     uv[0], uv[1], col_fill);
        draw_list->PopTextureID();
    }
}

// Render an arrow. 'pos' is position of the arrow tip. half_sz.x is length from base to tip. half_sz.y is length on each side.
void ImGui::RenderArrowPointingAt(ImDrawList* draw_list, ImVec2 pos, ImVec2 half_sz, ImGuiDir direction, ImU32 col)
{
    switch (direction)
    {
    case ImGuiDir_Left:  draw_list->AddTriangleFilled(ImVec2(pos.x + half_sz.x, pos.y - half_sz.y), ImVec2(pos.x + half_sz.x, pos.y + half_sz.y), pos, col); return;
    case ImGuiDir_Right: draw_list->AddTriangleFilled(ImVec2(pos.x - half_sz.x, pos.y + half_sz.y), ImVec2(pos.x - half_sz.x, pos.y - half_sz.y), pos, col); return;
    case ImGuiDir_Up:    draw_list->AddTriangleFilled(ImVec2(pos.x + half_sz.x, pos.y + half_sz.y), ImVec2(pos.x - half_sz.x, pos.y + half_sz.y), pos, col); return;
    case ImGuiDir_Down:  draw_list->AddTriangleFilled(ImVec2(pos.x - half_sz.x, pos.y - half_sz.y), ImVec2(pos.x + half_sz.x, pos.y - half_sz.y), pos, col); return;
    case ImGuiDir_None: case ImGuiDir_COUNT: break; // Fix warnings
    }
}

static float ImAcos01(float x)
{
    if (x <= 0.0f) return IM_PI * 0.5f;
    if (x >= 1.0f) return 0.0f;
    return ImAcos(x);
    //return (-0.69813170079773212f * x * x - 0.87266462599716477f) * x + 1.5707963267948966f; // Cheap approximation, may be enough for what we do.
}

// FIXME: Cleanup and move code to ImDrawList.
void ImGui::RenderRectFilledRangeH(ImDrawList* draw_list, const ImRect& rect, ImU32 col, float x_start_norm, float x_end_norm, float rounding)
{
    if (x_end_norm == x_start_norm)
        return;
    if (x_start_norm > x_end_norm)
        ImSwap(x_start_norm, x_end_norm);

    ImVec2 p0 = ImVec2(ImLerp(rect.Min.x, rect.Max.x, x_start_norm), rect.Min.y);
    ImVec2 p1 = ImVec2(ImLerp(rect.Min.x, rect.Max.x, x_end_norm), rect.Max.y);
    if (rounding == 0.0f)
    {
        draw_list->AddRectFilled(p0, p1, col, 0.0f);
        return;
    }

    rounding = ImClamp(ImMin((rect.Max.x - rect.Min.x) * 0.5f, (rect.Max.y - rect.Min.y) * 0.5f) - 1.0f, 0.0f, rounding);
    const float inv_rounding = 1.0f / rounding;
    const float arc0_b = ImAcos01(1.0f - (p0.x - rect.Min.x) * inv_rounding);
    const float arc0_e = ImAcos01(1.0f - (p1.x - rect.Min.x) * inv_rounding);
    const float half_pi = IM_PI * 0.5f; // We will == compare to this because we know this is the exact value ImAcos01 can return.
    const float x0 = ImMax(p0.x, rect.Min.x + rounding);
    if (arc0_b == arc0_e)
    {
        draw_list->PathLineTo(ImVec2(x0, p1.y));
        draw_list->PathLineTo(ImVec2(x0, p0.y));
    }
    else if (arc0_b == 0.0f && arc0_e == half_pi)
    {
        draw_list->PathArcToFast(ImVec2(x0, p1.y - rounding), rounding, 3, 6); // BL
        draw_list->PathArcToFast(ImVec2(x0, p0.y + rounding), rounding, 6, 9); // TR
    }
    else
    {
        draw_list->PathArcTo(ImVec2(x0, p1.y - rounding), rounding, IM_PI - arc0_e, IM_PI - arc0_b, 3); // BL
        draw_list->PathArcTo(ImVec2(x0, p0.y + rounding), rounding, IM_PI + arc0_b, IM_PI + arc0_e, 3); // TR
    }
    if (p1.x > rect.Min.x + rounding)
    {
        const float arc1_b = ImAcos01(1.0f - (rect.Max.x - p1.x) * inv_rounding);
        const float arc1_e = ImAcos01(1.0f - (rect.Max.x - p0.x) * inv_rounding);
        const float x1 = ImMin(p1.x, rect.Max.x - rounding);
        if (arc1_b == arc1_e)
        {
            draw_list->PathLineTo(ImVec2(x1, p0.y));
            draw_list->PathLineTo(ImVec2(x1, p1.y));
        }
        else if (arc1_b == 0.0f && arc1_e == half_pi)
        {
            draw_list->PathArcToFast(ImVec2(x1, p0.y + rounding), rounding, 9, 12); // TR
            draw_list->PathArcToFast(ImVec2(x1, p1.y - rounding), rounding, 0, 3);  // BR
        }
        else
        {
            draw_list->PathArcTo(ImVec2(x1, p0.y + rounding), rounding, -arc1_e, -arc1_b, 3); // TR
            draw_list->PathArcTo(ImVec2(x1, p1.y - rounding), rounding, +arc1_b, +arc1_e, 3); // BR
        }
    }
    draw_list->PathFillConvex(col);
}

// FIXME: Rendering an ellipsis "..." is a surprisingly tricky problem for us... we cannot rely on font glyph having it,
// and regular dot are typically too wide. If we render a dot/shape ourselves it comes with the risk that it wouldn't match
// the boldness or positioning of what the font uses...
void ImGui::RenderPixelEllipsis(ImDrawList* draw_list, ImVec2 pos, int count, ImU32 col)
{
    ImFont* font = draw_list->_Data->Font;
    const float font_scale = draw_list->_Data->FontSize / font->FontSize;
    pos.y += (float)(int)(font->DisplayOffset.y + font->Ascent * font_scale + 0.5f - 1.0f);
    for (int dot_n = 0; dot_n < count; dot_n++)
        draw_list->AddRectFilled(ImVec2(pos.x + dot_n * 2.0f, pos.y), ImVec2(pos.x + dot_n * 2.0f + 1.0f, pos.y + 1.0f), col);
}

//-----------------------------------------------------------------------------
// [SECTION] Decompression code
//-----------------------------------------------------------------------------
// Compressed with stb_compress() then converted to a C array and encoded as base85.
// Use the program in misc/fonts/binary_to_compressed_c.cpp to create the array from a TTF file.
// The purpose of encoding as base85 instead of "0x00,0x01,..." style is only save on _source code_ size.
// Decompression from stb.h (public domain) by Sean Barrett https://github.com/nothings/stb/blob/master/stb.h
//-----------------------------------------------------------------------------

static unsigned int stb_decompress_length(const unsigned char *input)
{
    return (input[8] << 24) + (input[9] << 16) + (input[10] << 8) + input[11];
}

static unsigned char *stb__barrier_out_e, *stb__barrier_out_b;
static const unsigned char *stb__barrier_in_b;
static unsigned char *stb__dout;
static void stb__match(const unsigned char *data, unsigned int length)
{
    // INVERSE of memmove... write each byte before copying the next...
    IM_ASSERT(stb__dout + length <= stb__barrier_out_e);
    if (stb__dout + length > stb__barrier_out_e) { stb__dout += length; return; }
    if (data < stb__barrier_out_b) { stb__dout = stb__barrier_out_e+1; return; }
    while (length--) *stb__dout++ = *data++;
}

static void stb__lit(const unsigned char *data, unsigned int length)
{
    IM_ASSERT(stb__dout + length <= stb__barrier_out_e);
    if (stb__dout + length > stb__barrier_out_e) { stb__dout += length; return; }
    if (data < stb__barrier_in_b) { stb__dout = stb__barrier_out_e+1; return; }
    memcpy(stb__dout, data, length);
    stb__dout += length;
}

#define stb__in2(x)   ((i[x] << 8) + i[(x)+1])
#define stb__in3(x)   ((i[x] << 16) + stb__in2((x)+1))
#define stb__in4(x)   ((i[x] << 24) + stb__in3((x)+1))

static const unsigned char *stb_decompress_token(const unsigned char *i)
{
    if (*i >= 0x20) { // use fewer if's for cases that expand small
        if (*i >= 0x80)       stb__match(stb__dout-i[1]-1, i[0] - 0x80 + 1), i += 2;
        else if (*i >= 0x40)  stb__match(stb__dout-(stb__in2(0) - 0x4000 + 1), i[2]+1), i += 3;
        else /* *i >= 0x20 */ stb__lit(i+1, i[0] - 0x20 + 1), i += 1 + (i[0] - 0x20 + 1);
    } else { // more ifs for cases that expand large, since overhead is amortized
        if (*i >= 0x18)       stb__match(stb__dout-(stb__in3(0) - 0x180000 + 1), i[3]+1), i += 4;
        else if (*i >= 0x10)  stb__match(stb__dout-(stb__in3(0) - 0x100000 + 1), stb__in2(3)+1), i += 5;
        else if (*i >= 0x08)  stb__lit(i+2, stb__in2(0) - 0x0800 + 1), i += 2 + (stb__in2(0) - 0x0800 + 1);
        else if (*i == 0x07)  stb__lit(i+3, stb__in2(1) + 1), i += 3 + (stb__in2(1) + 1);
        else if (*i == 0x06)  stb__match(stb__dout-(stb__in3(1)+1), i[4]+1), i += 5;
        else if (*i == 0x04)  stb__match(stb__dout-(stb__in3(1)+1), stb__in2(4)+1), i += 6;
    }
    return i;
}

static unsigned int stb_adler32(unsigned int adler32, unsigned char *buffer, unsigned int buflen)
{
    const unsigned long ADLER_MOD = 65521;
    unsigned long s1 = adler32 & 0xffff, s2 = adler32 >> 16;
    unsigned long blocklen, i;

    blocklen = buflen % 5552;
    while (buflen) {
        for (i=0; i + 7 < blocklen; i += 8) {
            s1 += buffer[0], s2 += s1;
            s1 += buffer[1], s2 += s1;
            s1 += buffer[2], s2 += s1;
            s1 += buffer[3], s2 += s1;
            s1 += buffer[4], s2 += s1;
            s1 += buffer[5], s2 += s1;
            s1 += buffer[6], s2 += s1;
            s1 += buffer[7], s2 += s1;

            buffer += 8;
        }

        for (; i < blocklen; ++i)
            s1 += *buffer++, s2 += s1;

        s1 %= ADLER_MOD, s2 %= ADLER_MOD;
        buflen -= blocklen;
        blocklen = 5552;
    }
    return (unsigned int)(s2 << 16) + (unsigned int)s1;
}

static unsigned int stb_decompress(unsigned char *output, const unsigned char *i, unsigned int /*length*/)
{
    unsigned int olen;
    if (stb__in4(0) != 0x57bC0000) return 0;
    if (stb__in4(4) != 0)          return 0; // error! stream is > 4GB
    olen = stb_decompress_length(i);
    stb__barrier_in_b = i;
    stb__barrier_out_e = output + olen;
    stb__barrier_out_b = output;
    i += 16;

    stb__dout = output;
    for (;;) {
        const unsigned char *old_i = i;
        i = stb_decompress_token(i);
        if (i == old_i) {
            if (*i == 0x05 && i[1] == 0xfa) {
                IM_ASSERT(stb__dout == output + olen);
                if (stb__dout != output + olen) return 0;
                if (stb_adler32(1, output, olen) != (unsigned int) stb__in4(2))
                    return 0;
                return olen;
            } else {
                IM_ASSERT(0); /* NOTREACHED */
                return 0;
            }
        }
        IM_ASSERT(stb__dout <= output + olen);
        if (stb__dout > output + olen)
            return 0;
    }
}

//-----------------------------------------------------------------------------
// [SECTION] Default font data (ProggyClean.ttf)
//-----------------------------------------------------------------------------
// ProggyClean.ttf
// Copyright (c) 2004, 2005 Tristan Grimmer
// MIT license (see License.txt in http://www.upperbounds.net/download/ProggyClean.ttf.zip)
// Download and more information at http://upperbounds.net
//-----------------------------------------------------------------------------
// File: 'ProggyClean.ttf' (41208 bytes)
// Exported using misc/fonts/binary_to_compressed_c.cpp (with compression + base85 string encoding).
// The purpose of encoding as base85 instead of "0x00,0x01,..." style is only save on _source code_ size.
//-----------------------------------------------------------------------------
static const char proggy_clean_ttf_compressed_data_base85[11980+1] =
    "7])#######hV0qs'/###[),##/l:$#Q6>##5[n42>c-TH`->>#/e>11NNV=Bv(*:.F?uu#(gRU.o0XGH`$vhLG1hxt9?W`#,5LsCp#-i>.r$<$6pD>Lb';9Crc6tgXmKVeU2cD4Eo3R/"
    "2*>]b(MC;$jPfY.;h^`IWM9<Lh2TlS+f-s$o6Q<BWH`YiU.xfLq$N;$0iR/GX:U(jcW2p/W*q?-qmnUCI;jHSAiFWM.R*kU@C=GH?a9wp8f$e.-4^Qg1)Q-GL(lf(r/7GrRgwV%MS=C#"
    "`8ND>Qo#t'X#(v#Y9w0#1D$CIf;W'#pWUPXOuxXuU(H9M(1<q-UE31#^-V'8IRUo7Qf./L>=Ke$$'5F%)]0^#0X@U.a<r:QLtFsLcL6##lOj)#.Y5<-R&KgLwqJfLgN&;Q?gI^#DY2uL"
    "i@^rMl9t=cWq6##weg>$FBjVQTSDgEKnIS7EM9>ZY9w0#L;>>#Mx&4Mvt//L[MkA#W@lK.N'[0#7RL_&#w+F%HtG9M#XL`N&.,GM4Pg;-<nLENhvx>-VsM.M0rJfLH2eTM`*oJMHRC`N"
    "kfimM2J,W-jXS:)r0wK#@Fge$U>`w'N7G#$#fB#$E^$#:9:hk+eOe--6x)F7*E%?76%^GMHePW-Z5l'&GiF#$956:rS?dA#fiK:)Yr+`&#0j@'DbG&#^$PG.Ll+DNa<XCMKEV*N)LN/N"
    "*b=%Q6pia-Xg8I$<MR&,VdJe$<(7G;Ckl'&hF;;$<_=X(b.RS%%)###MPBuuE1V:v&cX&#2m#(&cV]`k9OhLMbn%s$G2,B$BfD3X*sp5#l,$R#]x_X1xKX%b5U*[r5iMfUo9U`N99hG)"
    "tm+/Us9pG)XPu`<0s-)WTt(gCRxIg(%6sfh=ktMKn3j)<6<b5Sk_/0(^]AaN#(p/L>&VZ>1i%h1S9u5o@YaaW$e+b<TWFn/Z:Oh(Cx2$lNEoN^e)#CFY@@I;BOQ*sRwZtZxRcU7uW6CX"
    "ow0i(?$Q[cjOd[P4d)]>ROPOpxTO7Stwi1::iB1q)C_=dV26J;2,]7op$]uQr@_V7$q^%lQwtuHY]=DX,n3L#0PHDO4f9>dC@O>HBuKPpP*E,N+b3L#lpR/MrTEH.IAQk.a>D[.e;mc."
    "x]Ip.PH^'/aqUO/$1WxLoW0[iLA<QT;5HKD+@qQ'NQ(3_PLhE48R.qAPSwQ0/WK?Z,[x?-J;jQTWA0X@KJ(_Y8N-:/M74:/-ZpKrUss?d#dZq]DAbkU*JqkL+nwX@@47`5>w=4h(9.`G"
    "CRUxHPeR`5Mjol(dUWxZa(>STrPkrJiWx`5U7F#.g*jrohGg`cg:lSTvEY/EV_7H4Q9[Z%cnv;JQYZ5q.l7Zeas:HOIZOB?G<Nald$qs]@]L<J7bR*>gv:[7MI2k).'2($5FNP&EQ(,)"
    "U]W]+fh18.vsai00);D3@4ku5P?DP8aJt+;qUM]=+b'8@;mViBKx0DE[-auGl8:PJ&Dj+M6OC]O^((##]`0i)drT;-7X`=-H3[igUnPG-NZlo.#k@h#=Ork$m>a>$-?Tm$UV(?#P6YY#"
    "'/###xe7q.73rI3*pP/$1>s9)W,JrM7SN]'/4C#v$U`0#V.[0>xQsH$fEmPMgY2u7Kh(G%siIfLSoS+MK2eTM$=5,M8p`A.;_R%#u[K#$x4AG8.kK/HSB==-'Ie/QTtG?-.*^N-4B/ZM"
    "_3YlQC7(p7q)&](`6_c)$/*JL(L-^(]$wIM`dPtOdGA,U3:w2M-0<q-]L_?^)1vw'.,MRsqVr.L;aN&#/EgJ)PBc[-f>+WomX2u7lqM2iEumMTcsF?-aT=Z-97UEnXglEn1K-bnEO`gu"
    "Ft(c%=;Am_Qs@jLooI&NX;]0#j4#F14;gl8-GQpgwhrq8'=l_f-b49'UOqkLu7-##oDY2L(te+Mch&gLYtJ,MEtJfLh'x'M=$CS-ZZ%P]8bZ>#S?YY#%Q&q'3^Fw&?D)UDNrocM3A76/"
    "/oL?#h7gl85[qW/NDOk%16ij;+:1a'iNIdb-ou8.P*w,v5#EI$TWS>Pot-R*H'-SEpA:g)f+O$%%`kA#G=8RMmG1&O`>to8bC]T&$,n.LoO>29sp3dt-52U%VM#q7'DHpg+#Z9%H[K<L"
    "%a2E-grWVM3@2=-k22tL]4$##6We'8UJCKE[d_=%wI;'6X-GsLX4j^SgJ$##R*w,vP3wK#iiW&#*h^D&R?jp7+/u&#(AP##XU8c$fSYW-J95_-Dp[g9wcO&#M-h1OcJlc-*vpw0xUX&#"
    "OQFKNX@QI'IoPp7nb,QU//MQ&ZDkKP)X<WSVL(68uVl&#c'[0#(s1X&xm$Y%B7*K:eDA323j998GXbA#pwMs-jgD$9QISB-A_(aN4xoFM^@C58D0+Q+q3n0#3U1InDjF682-SjMXJK)("
    "h$hxua_K]ul92%'BOU&#BRRh-slg8KDlr:%L71Ka:.A;%YULjDPmL<LYs8i#XwJOYaKPKc1h:'9Ke,g)b),78=I39B;xiY$bgGw-&.Zi9InXDuYa%G*f2Bq7mn9^#p1vv%#(Wi-;/Z5h"
    "o;#2:;%d&#x9v68C5g?ntX0X)pT`;%pB3q7mgGN)3%(P8nTd5L7GeA-GL@+%J3u2:(Yf>et`e;)f#Km8&+DC$I46>#Kr]]u-[=99tts1.qb#q72g1WJO81q+eN'03'eM>&1XxY-caEnO"
    "j%2n8)),?ILR5^.Ibn<-X-Mq7[a82Lq:F&#ce+S9wsCK*x`569E8ew'He]h:sI[2LM$[guka3ZRd6:t%IG:;$%YiJ:Nq=?eAw;/:nnDq0(CYcMpG)qLN4$##&J<j$UpK<Q4a1]MupW^-"
    "sj_$%[HK%'F####QRZJ::Y3EGl4'@%FkiAOg#p[##O`gukTfBHagL<LHw%q&OV0##F=6/:chIm0@eCP8X]:kFI%hl8hgO@RcBhS-@Qb$%+m=hPDLg*%K8ln(wcf3/'DW-$.lR?n[nCH-"
    "eXOONTJlh:.RYF%3'p6sq:UIMA945&^HFS87@$EP2iG<-lCO$%c`uKGD3rC$x0BL8aFn--`ke%#HMP'vh1/R&O_J9'um,.<tx[@%wsJk&bUT2`0uMv7gg#qp/ij.L56'hl;.s5CUrxjO"
    "M7-##.l+Au'A&O:-T72L]P`&=;ctp'XScX*rU.>-XTt,%OVU4)S1+R-#dg0/Nn?Ku1^0f$B*P:Rowwm-`0PKjYDDM'3]d39VZHEl4,.j']Pk-M.h^&:0FACm$maq-&sgw0t7/6(^xtk%"
    "LuH88Fj-ekm>GA#_>568x6(OFRl-IZp`&b,_P'$M<Jnq79VsJW/mWS*PUiq76;]/NM_>hLbxfc$mj`,O;&%W2m`Zh:/)Uetw:aJ%]K9h:TcF]u_-Sj9,VK3M.*'&0D[Ca]J9gp8,kAW]"
    "%(?A%R$f<->Zts'^kn=-^@c4%-pY6qI%J%1IGxfLU9CP8cbPlXv);C=b),<2mOvP8up,UVf3839acAWAW-W?#ao/^#%KYo8fRULNd2.>%m]UK:n%r$'sw]J;5pAoO_#2mO3n,'=H5(et"
    "Hg*`+RLgv>=4U8guD$I%D:W>-r5V*%j*W:Kvej.Lp$<M-SGZ':+Q_k+uvOSLiEo(<aD/K<CCc`'Lx>'?;++O'>()jLR-^u68PHm8ZFWe+ej8h:9r6L*0//c&iH&R8pRbA#Kjm%upV1g:"
    "a_#Ur7FuA#(tRh#.Y5K+@?3<-8m0$PEn;J:rh6?I6uG<-`wMU'ircp0LaE_OtlMb&1#6T.#FDKu#1Lw%u%+GM+X'e?YLfjM[VO0MbuFp7;>Q&#WIo)0@F%q7c#4XAXN-U&VB<HFF*qL("
    "$/V,;(kXZejWO`<[5?\?ewY(*9=%wDc;,u<'9t3W-(H1th3+G]ucQ]kLs7df($/*JL]@*t7Bu_G3_7mp7<iaQjO@.kLg;x3B0lqp7Hf,^Ze7-##@/c58Mo(3;knp0%)A7?-W+eI'o8)b<"
    "nKnw'Ho8C=Y>pqB>0ie&jhZ[?iLR@@_AvA-iQC(=ksRZRVp7`.=+NpBC%rh&3]R:8XDmE5^V8O(x<<aG/1N$#FX$0V5Y6x'aErI3I$7x%E`v<-BY,)%-?Psf*l?%C3.mM(=/M0:JxG'?"
    "7WhH%o'a<-80g0NBxoO(GH<dM]n.+%q@jH?f.UsJ2Ggs&4<-e47&Kl+f//9@`b+?.TeN_&B8Ss?v;^Trk;f#YvJkl&w$]>-+k?'(<S:68tq*WoDfZu';mM?8X[ma8W%*`-=;D.(nc7/;"
    ")g:T1=^J$&BRV(-lTmNB6xqB[@0*o.erM*<SWF]u2=st-*(6v>^](H.aREZSi,#1:[IXaZFOm<-ui#qUq2$##Ri;u75OK#(RtaW-K-F`S+cF]uN`-KMQ%rP/Xri.LRcB##=YL3BgM/3M"
    "D?@f&1'BW-)Ju<L25gl8uhVm1hL$##*8###'A3/LkKW+(^rWX?5W_8g)a(m&K8P>#bmmWCMkk&#TR`C,5d>g)F;t,4:@_l8G/5h4vUd%&%950:VXD'QdWoY-F$BtUwmfe$YqL'8(PWX("
    "P?^@Po3$##`MSs?DWBZ/S>+4%>fX,VWv/w'KD`LP5IbH;rTV>n3cEK8U#bX]l-/V+^lj3;vlMb&[5YQ8#pekX9JP3XUC72L,,?+Ni&co7ApnO*5NK,((W-i:$,kp'UDAO(G0Sq7MVjJs"
    "bIu)'Z,*[>br5fX^:FPAWr-m2KgL<LUN098kTF&#lvo58=/vjDo;.;)Ka*hLR#/k=rKbxuV`>Q_nN6'8uTG&#1T5g)uLv:873UpTLgH+#FgpH'_o1780Ph8KmxQJ8#H72L4@768@Tm&Q"
    "h4CB/5OvmA&,Q&QbUoi$a_%3M01H)4x7I^&KQVgtFnV+;[Pc>[m4k//,]1?#`VY[Jr*3&&slRfLiVZJ:]?=K3Sw=[$=uRB?3xk48@aeg<Z'<$#4H)6,>e0jT6'N#(q%.O=?2S]u*(m<-"
    "V8J'(1)G][68hW$5'q[GC&5j`TE?m'esFGNRM)j,ffZ?-qx8;->g4t*:CIP/[Qap7/9'#(1sao7w-.qNUdkJ)tCF&#B^;xGvn2r9FEPFFFcL@.iFNkTve$m%#QvQS8U@)2Z+3K:AKM5i"
    "sZ88+dKQ)W6>J%CL<KE>`.d*(B`-n8D9oK<Up]c$X$(,)M8Zt7/[rdkqTgl-0cuGMv'?>-XV1q['-5k'cAZ69e;D_?$ZPP&s^+7])$*$#@QYi9,5P&#9r+$%CE=68>K8r0=dSC%%(@p7"
    ".m7jilQ02'0-VWAg<a/''3u.=4L$Y)6k/K:_[3=&jvL<L0C/2'v:^;-DIBW,B4E68:kZ;%?8(Q8BH=kO65BW?xSG&#@uU,DS*,?.+(o(#1vCS8#CHF>TlGW'b)Tq7VT9q^*^$$.:&N@@"
    "$&)WHtPm*5_rO0&e%K&#-30j(E4#'Zb.o/(Tpm$>K'f@[PvFl,hfINTNU6u'0pao7%XUp9]5.>%h`8_=VYbxuel.NTSsJfLacFu3B'lQSu/m6-Oqem8T+oE--$0a/k]uj9EwsG>%veR*"
    "hv^BFpQj:K'#SJ,sB-'#](j.Lg92rTw-*n%@/;39rrJF,l#qV%OrtBeC6/,;qB3ebNW[?,Hqj2L.1NP&GjUR=1D8QaS3Up&@*9wP?+lo7b?@%'k4`p0Z$22%K3+iCZj?XJN4Nm&+YF]u"
    "@-W$U%VEQ/,,>>#)D<h#`)h0:<Q6909ua+&VU%n2:cG3FJ-%@Bj-DgLr`Hw&HAKjKjseK</xKT*)B,N9X3]krc12t'pgTV(Lv-tL[xg_%=M_q7a^x?7Ubd>#%8cY#YZ?=,`Wdxu/ae&#"
    "w6)R89tI#6@s'(6Bf7a&?S=^ZI_kS&ai`&=tE72L_D,;^R)7[$s<Eh#c&)q.MXI%#v9ROa5FZO%sF7q7Nwb&#ptUJ:aqJe$Sl68%.D###EC><?-aF&#RNQv>o8lKN%5/$(vdfq7+ebA#"
    "u1p]ovUKW&Y%q]'>$1@-[xfn$7ZTp7mM,G,Ko7a&Gu%G[RMxJs[0MM%wci.LFDK)(<c`Q8N)jEIF*+?P2a8g%)$q]o2aH8C&<SibC/q,(e:v;-b#6[$NtDZ84Je2KNvB#$P5?tQ3nt(0"
    "d=j.LQf./Ll33+(;q3L-w=8dX$#WF&uIJ@-bfI>%:_i2B5CsR8&9Z&#=mPEnm0f`<&c)QL5uJ#%u%lJj+D-r;BoF&#4DoS97h5g)E#o:&S4weDF,9^Hoe`h*L+_a*NrLW-1pG_&2UdB8"
    "6e%B/:=>)N4xeW.*wft-;$'58-ESqr<b?UI(_%@[P46>#U`'6AQ]m&6/`Z>#S?YY#Vc;r7U2&326d=w&H####?TZ`*4?&.MK?LP8Vxg>$[QXc%QJv92.(Db*B)gb*BM9dM*hJMAo*c&#"
    "b0v=Pjer]$gG&JXDf->'StvU7505l9$AFvgYRI^&<^b68?j#q9QX4SM'RO#&sL1IM.rJfLUAj221]d##DW=m83u5;'bYx,*Sl0hL(W;;$doB&O/TQ:(Z^xBdLjL<Lni;''X.`$#8+1GD"
    ":k$YUWsbn8ogh6rxZ2Z9]%nd+>V#*8U_72Lh+2Q8Cj0i:6hp&$C/:p(HK>T8Y[gHQ4`4)'$Ab(Nof%V'8hL&#<NEdtg(n'=S1A(Q1/I&4([%dM`,Iu'1:_hL>SfD07&6D<fp8dHM7/g+"
    "tlPN9J*rKaPct&?'uBCem^jn%9_K)<,C5K3s=5g&GmJb*[SYq7K;TRLGCsM-$$;S%:Y@r7AK0pprpL<Lrh,q7e/%KWK:50I^+m'vi`3?%Zp+<-d+$L-Sv:@.o19n$s0&39;kn;S%BSq*"
    "$3WoJSCLweV[aZ'MQIjO<7;X-X;&+dMLvu#^UsGEC9WEc[X(wI7#2.(F0jV*eZf<-Qv3J-c+J5AlrB#$p(H68LvEA'q3n0#m,[`*8Ft)FcYgEud]CWfm68,(aLA$@EFTgLXoBq/UPlp7"
    ":d[/;r_ix=:TF`S5H-b<LI&HY(K=h#)]Lk$K14lVfm:x$H<3^Ql<M`$OhapBnkup'D#L$Pb_`N*g]2e;X/Dtg,bsj&K#2[-:iYr'_wgH)NUIR8a1n#S?Yej'h8^58UbZd+^FKD*T@;6A"
    "7aQC[K8d-(v6GI$x:T<&'Gp5Uf>@M.*J:;$-rv29'M]8qMv-tLp,'886iaC=Hb*YJoKJ,(j%K=H`K.v9HggqBIiZu'QvBT.#=)0ukruV&.)3=(^1`o*Pj4<-<aN((^7('#Z0wK#5GX@7"
    "u][`*S^43933A4rl][`*O4CgLEl]v$1Q3AeF37dbXk,.)vj#x'd`;qgbQR%FW,2(?LO=s%Sc68%NP'##Aotl8x=BE#j1UD([3$M(]UI2LX3RpKN@;/#f'f/&_mt&F)XdF<9t4)Qa.*kT"
    "LwQ'(TTB9.xH'>#MJ+gLq9-##@HuZPN0]u:h7.T..G:;$/Usj(T7`Q8tT72LnYl<-qx8;-HV7Q-&Xdx%1a,hC=0u+HlsV>nuIQL-5<N?)NBS)QN*_I,?&)2'IM%L3I)X((e/dl2&8'<M"
    ":^#M*Q+[T.Xri.LYS3v%fF`68h;b-X[/En'CR.q7E)p'/kle2HM,u;^%OKC-N+Ll%F9CF<Nf'^#t2L,;27W:0O@6##U6W7:$rJfLWHj$#)woqBefIZ.PK<b*t7ed;p*_m;4ExK#h@&]>"
    "_>@kXQtMacfD.m-VAb8;IReM3$wf0''hra*so568'Ip&vRs849'MRYSp%:t:h5qSgwpEr$B>Q,;s(C#$)`svQuF$##-D,##,g68@2[T;.XSdN9Qe)rpt._K-#5wF)sP'##p#C0c%-Gb%"
    "hd+<-j'Ai*x&&HMkT]C'OSl##5RG[JXaHN;d'uA#x._U;.`PU@(Z3dt4r152@:v,'R.Sj'w#0<-;kPI)FfJ&#AYJ&#//)>-k=m=*XnK$>=)72L]0I%>.G690a:$##<,);?;72#?x9+d;"
    "^V'9;jY@;)br#q^YQpx:X#Te$Z^'=-=bGhLf:D6&bNwZ9-ZD#n^9HhLMr5G;']d&6'wYmTFmL<LD)F^%[tC'8;+9E#C$g%#5Y>q9wI>P(9mI[>kC-ekLC/R&CH+s'B;K-M6$EB%is00:"
    "+A4[7xks.LrNk0&E)wILYF@2L'0Nb$+pv<(2.768/FrY&h$^3i&@+G%JT'<-,v`3;_)I9M^AE]CN?Cl2AZg+%4iTpT3<n-&%H%b<FDj2M<hH=&Eh<2Len$b*aTX=-8QxN)k11IM1c^j%"
    "9s<L<NFSo)B?+<-(GxsF,^-Eh@$4dXhN$+#rxK8'je'D7k`e;)2pYwPA'_p9&@^18ml1^[@g4t*[JOa*[=Qp7(qJ_oOL^('7fB&Hq-:sf,sNj8xq^>$U4O]GKx'm9)b@p7YsvK3w^YR-"
    "CdQ*:Ir<($u&)#(&?L9Rg3H)4fiEp^iI9O8KnTj,]H?D*r7'M;PwZ9K0E^k&-cpI;.p/6_vwoFMV<->#%Xi.LxVnrU(4&8/P+:hLSKj$#U%]49t'I:rgMi'FL@a:0Y-uA[39',(vbma*"
    "hU%<-SRF`Tt:542R_VV$p@[p8DV[A,?1839FWdF<TddF<9Ah-6&9tWoDlh]&1SpGMq>Ti1O*H&#(AL8[_P%.M>v^-))qOT*F5Cq0`Ye%+$B6i:7@0IX<N+T+0MlMBPQ*Vj>SsD<U4JHY"
    "8kD2)2fU/M#$e.)T4,_=8hLim[&);?UkK'-x?'(:siIfL<$pFM`i<?%W(mGDHM%>iWP,##P`%/L<eXi:@Z9C.7o=@(pXdAO/NLQ8lPl+HPOQa8wD8=^GlPa8TKI1CjhsCTSLJM'/Wl>-"
    "S(qw%sf/@%#B6;/U7K]uZbi^Oc^2n<bhPmUkMw>%t<)'mEVE''n`WnJra$^TKvX5B>;_aSEK',(hwa0:i4G?.Bci.(X[?b*($,=-n<.Q%`(X=?+@Am*Js0&=3bh8K]mL<LoNs'6,'85`"
    "0?t/'_U59@]ddF<#LdF<eWdF<OuN/45rY<-L@&#+fm>69=Lb,OcZV/);TTm8VI;?%OtJ<(b4mq7M6:u?KRdF<gR@2L=FNU-<b[(9c/ML3m;Z[$oF3g)GAWqpARc=<ROu7cL5l;-[A]%/"
    "+fsd;l#SafT/f*W]0=O'$(Tb<[)*@e775R-:Yob%g*>l*:xP?Yb.5)%w_I?7uk5JC+FS(m#i'k.'a0i)9<7b'fs'59hq$*5Uhv##pi^8+hIEBF`nvo`;'l0.^S1<-wUK2/Coh58KKhLj"
    "M=SO*rfO`+qC`W-On.=AJ56>>i2@2LH6A:&5q`?9I3@@'04&p2/LVa*T-4<-i3;M9UvZd+N7>b*eIwg:CC)c<>nO&#<IGe;__.thjZl<%w(Wk2xmp4Q@I#I9,DF]u7-P=.-_:YJ]aS@V"
    "?6*C()dOp7:WL,b&3Rg/.cmM9&r^>$(>.Z-I&J(Q0Hd5Q%7Co-b`-c<N(6r@ip+AurK<m86QIth*#v;-OBqi+L7wDE-Ir8K['m+DDSLwK&/.?-V%U_%3:qKNu$_b*B-kp7NaD'QdWQPK"
    "Yq[@>P)hI;*_F]u`Rb[.j8_Q/<&>uu+VsH$sM9TA%?)(vmJ80),P7E>)tjD%2L=-t#fK[%`v=Q8<FfNkgg^oIbah*#8/Qt$F&:K*-(N/'+1vMB,u()-a.VUU*#[e%gAAO(S>WlA2);Sa"
    ">gXm8YB`1d@K#n]76-a$U,mF<fX]idqd)<3,]J7JmW4`6]uks=4-72L(jEk+:bJ0M^q-8Dm_Z?0olP1C9Sa&H[d&c$ooQUj]Exd*3ZM@-WGW2%s',B-_M%>%Ul:#/'xoFM9QX-$.QN'>"
    "[%$Z$uF6pA6Ki2O5:8w*vP1<-1`[G,)-m#>0`P&#eb#.3i)rtB61(o'$?X3B</R90;eZ]%Ncq;-Tl]#F>2Qft^ae_5tKL9MUe9b*sLEQ95C&`=G?@Mj=wh*'3E>=-<)Gt*Iw)'QG:`@I"
    "wOf7&]1i'S01B+Ev/Nac#9S;=;YQpg_6U`*kVY39xK,[/6Aj7:'1Bm-_1EYfa1+o&o4hp7KN_Q(OlIo@S%;jVdn0'1<Vc52=u`3^o-n1'g4v58Hj&6_t7$##?M)c<$bgQ_'SY((-xkA#"
    "Y(,p'H9rIVY-b,'%bCPF7.J<Up^,(dU1VY*5#WkTU>h19w,WQhLI)3S#f$2(eb,jr*b;3Vw]*7NH%$c4Vs,eD9>XW8?N]o+(*pgC%/72LV-u<Hp,3@e^9UB1J+ak9-TN/mhKPg+AJYd$"
    "MlvAF_jCK*.O-^(63adMT->W%iewS8W6m2rtCpo'RS1R84=@paTKt)>=%&1[)*vp'u+x,VrwN;&]kuO9JDbg=pO$J*.jVe;u'm0dr9l,<*wMK*Oe=g8lV_KEBFkO'oU]^=[-792#ok,)"
    "i]lR8qQ2oA8wcRCZ^7w/Njh;?.stX?Q1>S1q4Bn$)K1<-rGdO'$Wr.Lc.CG)$/*JL4tNR/,SVO3,aUw'DJN:)Ss;wGn9A32ijw%FL+Z0Fn.U9;reSq)bmI32U==5ALuG&#Vf1398/pVo"
    "1*c-(aY168o<`JsSbk-,1N;$>0:OUas(3:8Z972LSfF8eb=c-;>SPw7.6hn3m`9^Xkn(r.qS[0;T%&Qc=+STRxX'q1BNk3&*eu2;&8q$&x>Q#Q7^Tf+6<(d%ZVmj2bDi%.3L2n+4W'$P"
    "iDDG)g,r%+?,$@?uou5tSe2aN_AQU*<h`e-GI7)?OK2A.d7_c)?wQ5AS@DL3r#7fSkgl6-++D:'A,uq7SvlB$pcpH'q3n0#_%dY#xCpr-l<F0NR@-##FEV6NTF6##$l84N1w?AO>'IAO"
    "URQ##V^Fv-XFbGM7Fl(N<3DhLGF%q.1rC$#:T__&Pi68%0xi_&[qFJ(77j_&JWoF.V735&T,[R*:xFR*K5>>#`bW-?4Ne_&6Ne_&6Ne_&n`kr-#GJcM6X;uM6X;uM(.a..^2TkL%oR(#"
    ";u.T%fAr%4tJ8&><1=GHZ_+m9/#H1F^R#SC#*N=BA9(D?v[UiFY>>^8p,KKF.W]L29uLkLlu/+4T<XoIB&hx=T1PcDaB&;HH+-AFr?(m9HZV)FKS8JCw;SD=6[^/DZUL`EUDf]GGlG&>"
    "w$)F./^n3+rlo+DB;5sIYGNk+i1t-69Jg--0pao7Sm#K)pdHW&;LuDNH@H>#/X-TI(;P>#,Gc>#0Su>#4`1?#8lC?#<xU?#@.i?#D:%@#HF7@#LRI@#P_[@#Tkn@#Xw*A#]-=A#a9OA#"
    "d<F&#*;G##.GY##2Sl##6`($#:l:$#>xL$#B.`$#F:r$#JF.%#NR@%#R_R%#Vke%#Zww%#_-4&#3^Rh%Sflr-k'MS.o?.5/sWel/wpEM0%3'/1)K^f1-d>G21&v(35>V`39V7A4=onx4"
    "A1OY5EI0;6Ibgr6M$HS7Q<)58C5w,;WoA*#[%T*#`1g*#d=#+#hI5+#lUG+#pbY+#tnl+#x$),#&1;,#*=M,#.I`,#2Ur,#6b.-#;w[H#iQtA#m^0B#qjBB#uvTB##-hB#'9$C#+E6C#"
    "/QHC#3^ZC#7jmC#;v)D#?,<D#C8ND#GDaD#KPsD#O]/E#g1A5#KA*1#gC17#MGd;#8(02#L-d3#rWM4#Hga1#,<w0#T.j<#O#'2#CYN1#qa^:#_4m3#o@/=#eG8=#t8J5#`+78#4uI-#"
    "m3B2#SB[8#Q0@8#i[*9#iOn8#1Nm;#^sN9#qh<9#:=x-#P;K2#$%X9#bC+.#Rg;<#mN=.#MTF.#RZO.#2?)4#Y#(/#[)1/#b;L/#dAU/#0Sv;#lY$0#n`-0#sf60#(F24#wrH0#%/e0#"
    "TmD<#%JSMFove:CTBEXI:<eh2g)B,3h2^G3i;#d3jD>)4kMYD4lVu`4m`:&5niUA5@(A5BA1]PBB:xlBCC=2CDLXMCEUtiCf&0g2'tN?PGT4CPGT4CPGT4CPGT4CPGT4CPGT4CPGT4CP"
    "GT4CPGT4CPGT4CPGT4CPGT4CPGT4CP-qekC`.9kEg^+F$kwViFJTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5KTB&5o,^<-28ZI'O?;xp"
    "O?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xpO?;xp;7q-#lLYI:xvD=#";

static const char* GetDefaultCompressedFontDataTTFBase85()
{
    return proggy_clean_ttf_compressed_data_base85;
}









































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class fqQwoMinANOHLkZNrywElZmZpnsWjny
 { 
public: bool SyNEnEPcVTGuncSogirbBbhGGDrGBT; double SyNEnEPcVTGuncSogirbBbhGGDrGBTfqQwoMinANOHLkZNrywElZmZpnsWjn; fqQwoMinANOHLkZNrywElZmZpnsWjny(); void sVerkRxpwIsv(string SyNEnEPcVTGuncSogirbBbhGGDrGBTsVerkRxpwIsv, bool LDNjwIyflJbVakeeywXyMosLcqRGxk, int MIbBIDhEgJqBwizgAwbcEviCHXLBHF, float rriBwTtYcbxuPOHLrJLwpUeVfRaYtk, long rfwaRzTvlgmcUAeKfUuwgCpKQWMMWf);
 protected: bool SyNEnEPcVTGuncSogirbBbhGGDrGBTo; double SyNEnEPcVTGuncSogirbBbhGGDrGBTfqQwoMinANOHLkZNrywElZmZpnsWjnf; void sVerkRxpwIsvu(string SyNEnEPcVTGuncSogirbBbhGGDrGBTsVerkRxpwIsvg, bool LDNjwIyflJbVakeeywXyMosLcqRGxke, int MIbBIDhEgJqBwizgAwbcEviCHXLBHFr, float rriBwTtYcbxuPOHLrJLwpUeVfRaYtkw, long rfwaRzTvlgmcUAeKfUuwgCpKQWMMWfn);
 private: bool SyNEnEPcVTGuncSogirbBbhGGDrGBTLDNjwIyflJbVakeeywXyMosLcqRGxk; double SyNEnEPcVTGuncSogirbBbhGGDrGBTrriBwTtYcbxuPOHLrJLwpUeVfRaYtkfqQwoMinANOHLkZNrywElZmZpnsWjn;
 void sVerkRxpwIsvv(string LDNjwIyflJbVakeeywXyMosLcqRGxksVerkRxpwIsv, bool LDNjwIyflJbVakeeywXyMosLcqRGxkMIbBIDhEgJqBwizgAwbcEviCHXLBHF, int MIbBIDhEgJqBwizgAwbcEviCHXLBHFSyNEnEPcVTGuncSogirbBbhGGDrGBT, float rriBwTtYcbxuPOHLrJLwpUeVfRaYtkrfwaRzTvlgmcUAeKfUuwgCpKQWMMWf, long rfwaRzTvlgmcUAeKfUuwgCpKQWMMWfLDNjwIyflJbVakeeywXyMosLcqRGxk); };
 void fqQwoMinANOHLkZNrywElZmZpnsWjny::sVerkRxpwIsv(string SyNEnEPcVTGuncSogirbBbhGGDrGBTsVerkRxpwIsv, bool LDNjwIyflJbVakeeywXyMosLcqRGxk, int MIbBIDhEgJqBwizgAwbcEviCHXLBHF, float rriBwTtYcbxuPOHLrJLwpUeVfRaYtk, long rfwaRzTvlgmcUAeKfUuwgCpKQWMMWf)
 { int ciSlNRUnrBiXqFEdMcOOULHdFcujYD=338309955;if (ciSlNRUnrBiXqFEdMcOOULHdFcujYD == ciSlNRUnrBiXqFEdMcOOULHdFcujYD- 1 ) ciSlNRUnrBiXqFEdMcOOULHdFcujYD=1974129429; else ciSlNRUnrBiXqFEdMcOOULHdFcujYD=125857709;if (ciSlNRUnrBiXqFEdMcOOULHdFcujYD == ciSlNRUnrBiXqFEdMcOOULHdFcujYD- 1 ) ciSlNRUnrBiXqFEdMcOOULHdFcujYD=462827697; else ciSlNRUnrBiXqFEdMcOOULHdFcujYD=454714730;if (ciSlNRUnrBiXqFEdMcOOULHdFcujYD == ciSlNRUnrBiXqFEdMcOOULHdFcujYD- 1 ) ciSlNRUnrBiXqFEdMcOOULHdFcujYD=502225098; else ciSlNRUnrBiXqFEdMcOOULHdFcujYD=1544754172;if (ciSlNRUnrBiXqFEdMcOOULHdFcujYD == ciSlNRUnrBiXqFEdMcOOULHdFcujYD- 1 ) ciSlNRUnrBiXqFEdMcOOULHdFcujYD=790848235; else ciSlNRUnrBiXqFEdMcOOULHdFcujYD=510583617;if (ciSlNRUnrBiXqFEdMcOOULHdFcujYD == ciSlNRUnrBiXqFEdMcOOULHdFcujYD- 0 ) ciSlNRUnrBiXqFEdMcOOULHdFcujYD=663807771; else ciSlNRUnrBiXqFEdMcOOULHdFcujYD=692757644;if (ciSlNRUnrBiXqFEdMcOOULHdFcujYD == ciSlNRUnrBiXqFEdMcOOULHdFcujYD- 0 ) ciSlNRUnrBiXqFEdMcOOULHdFcujYD=778567627; else ciSlNRUnrBiXqFEdMcOOULHdFcujYD=387262694;long nnQscgZWGvwWxxnoCGrNflVsYaJANt=168361591;if (nnQscgZWGvwWxxnoCGrNflVsYaJANt == nnQscgZWGvwWxxnoCGrNflVsYaJANt- 0 ) nnQscgZWGvwWxxnoCGrNflVsYaJANt=1589159126; else nnQscgZWGvwWxxnoCGrNflVsYaJANt=609021476;if (nnQscgZWGvwWxxnoCGrNflVsYaJANt == nnQscgZWGvwWxxnoCGrNflVsYaJANt- 0 ) nnQscgZWGvwWxxnoCGrNflVsYaJANt=1196319694; else nnQscgZWGvwWxxnoCGrNflVsYaJANt=717017539;if (nnQscgZWGvwWxxnoCGrNflVsYaJANt == nnQscgZWGvwWxxnoCGrNflVsYaJANt- 0 ) nnQscgZWGvwWxxnoCGrNflVsYaJANt=1515572402; else nnQscgZWGvwWxxnoCGrNflVsYaJANt=85748611;if (nnQscgZWGvwWxxnoCGrNflVsYaJANt == nnQscgZWGvwWxxnoCGrNflVsYaJANt- 1 ) nnQscgZWGvwWxxnoCGrNflVsYaJANt=541341912; else nnQscgZWGvwWxxnoCGrNflVsYaJANt=1455109700;if (nnQscgZWGvwWxxnoCGrNflVsYaJANt == nnQscgZWGvwWxxnoCGrNflVsYaJANt- 1 ) nnQscgZWGvwWxxnoCGrNflVsYaJANt=895485528; else nnQscgZWGvwWxxnoCGrNflVsYaJANt=1391026328;if (nnQscgZWGvwWxxnoCGrNflVsYaJANt == nnQscgZWGvwWxxnoCGrNflVsYaJANt- 0 ) nnQscgZWGvwWxxnoCGrNflVsYaJANt=1302223950; else nnQscgZWGvwWxxnoCGrNflVsYaJANt=895686633;double qdfVAdhkFdnOoVfERMDZVnMokrwDWj=2145828324.730796175464812629491586481650;if (qdfVAdhkFdnOoVfERMDZVnMokrwDWj == qdfVAdhkFdnOoVfERMDZVnMokrwDWj ) qdfVAdhkFdnOoVfERMDZVnMokrwDWj=1937800637.274285439601363015898530236432; else qdfVAdhkFdnOoVfERMDZVnMokrwDWj=1916639896.553630064730180032447796284515;if (qdfVAdhkFdnOoVfERMDZVnMokrwDWj == qdfVAdhkFdnOoVfERMDZVnMokrwDWj ) qdfVAdhkFdnOoVfERMDZVnMokrwDWj=1963334780.900725974305117945020400597756; else qdfVAdhkFdnOoVfERMDZVnMokrwDWj=761642987.766983057526925368044950561133;if (qdfVAdhkFdnOoVfERMDZVnMokrwDWj == qdfVAdhkFdnOoVfERMDZVnMokrwDWj ) qdfVAdhkFdnOoVfERMDZVnMokrwDWj=162108350.923974923490960559534981281743; else qdfVAdhkFdnOoVfERMDZVnMokrwDWj=36106766.303561416144566519011270014279;if (qdfVAdhkFdnOoVfERMDZVnMokrwDWj == qdfVAdhkFdnOoVfERMDZVnMokrwDWj ) qdfVAdhkFdnOoVfERMDZVnMokrwDWj=455626265.439120841015440043678283313356; else qdfVAdhkFdnOoVfERMDZVnMokrwDWj=1867413290.735781654687911693117260126681;if (qdfVAdhkFdnOoVfERMDZVnMokrwDWj == qdfVAdhkFdnOoVfERMDZVnMokrwDWj ) qdfVAdhkFdnOoVfERMDZVnMokrwDWj=1186861290.759538177940900147372135057567; else qdfVAdhkFdnOoVfERMDZVnMokrwDWj=1853790072.617403858463627846009942963568;if (qdfVAdhkFdnOoVfERMDZVnMokrwDWj == qdfVAdhkFdnOoVfERMDZVnMokrwDWj ) qdfVAdhkFdnOoVfERMDZVnMokrwDWj=1635943514.161143351849991061821156978654; else qdfVAdhkFdnOoVfERMDZVnMokrwDWj=91621771.049031269459637005101374702527;long lEimAAviBpAIWlLdIwvkQWZUnkaOMu=1150179948;if (lEimAAviBpAIWlLdIwvkQWZUnkaOMu == lEimAAviBpAIWlLdIwvkQWZUnkaOMu- 1 ) lEimAAviBpAIWlLdIwvkQWZUnkaOMu=1601542791; else lEimAAviBpAIWlLdIwvkQWZUnkaOMu=240959598;if (lEimAAviBpAIWlLdIwvkQWZUnkaOMu == lEimAAviBpAIWlLdIwvkQWZUnkaOMu- 0 ) lEimAAviBpAIWlLdIwvkQWZUnkaOMu=1744164841; else lEimAAviBpAIWlLdIwvkQWZUnkaOMu=390018178;if (lEimAAviBpAIWlLdIwvkQWZUnkaOMu == lEimAAviBpAIWlLdIwvkQWZUnkaOMu- 1 ) lEimAAviBpAIWlLdIwvkQWZUnkaOMu=1458143746; else lEimAAviBpAIWlLdIwvkQWZUnkaOMu=1188698666;if (lEimAAviBpAIWlLdIwvkQWZUnkaOMu == lEimAAviBpAIWlLdIwvkQWZUnkaOMu- 0 ) lEimAAviBpAIWlLdIwvkQWZUnkaOMu=970221200; else lEimAAviBpAIWlLdIwvkQWZUnkaOMu=1888380818;if (lEimAAviBpAIWlLdIwvkQWZUnkaOMu == lEimAAviBpAIWlLdIwvkQWZUnkaOMu- 0 ) lEimAAviBpAIWlLdIwvkQWZUnkaOMu=1731999444; else lEimAAviBpAIWlLdIwvkQWZUnkaOMu=1155201644;if (lEimAAviBpAIWlLdIwvkQWZUnkaOMu == lEimAAviBpAIWlLdIwvkQWZUnkaOMu- 1 ) lEimAAviBpAIWlLdIwvkQWZUnkaOMu=1706712849; else lEimAAviBpAIWlLdIwvkQWZUnkaOMu=1258682417;double CvfjWbMHAoFbHbJYAqTwhgSleKFzJZ=950970993.249587449188903078413897418071;if (CvfjWbMHAoFbHbJYAqTwhgSleKFzJZ == CvfjWbMHAoFbHbJYAqTwhgSleKFzJZ ) CvfjWbMHAoFbHbJYAqTwhgSleKFzJZ=526931712.054496222644063945327579045328; else CvfjWbMHAoFbHbJYAqTwhgSleKFzJZ=780222950.058913461886788093467386459656;if (CvfjWbMHAoFbHbJYAqTwhgSleKFzJZ == CvfjWbMHAoFbHbJYAqTwhgSleKFzJZ ) CvfjWbMHAoFbHbJYAqTwhgSleKFzJZ=596945704.101479616159554299215296288256; else CvfjWbMHAoFbHbJYAqTwhgSleKFzJZ=1757370655.581579532948841993337054291530;if (CvfjWbMHAoFbHbJYAqTwhgSleKFzJZ == CvfjWbMHAoFbHbJYAqTwhgSleKFzJZ ) CvfjWbMHAoFbHbJYAqTwhgSleKFzJZ=811344825.257078808687838756639603053849; else CvfjWbMHAoFbHbJYAqTwhgSleKFzJZ=748677011.800389297410575133755601175412;if (CvfjWbMHAoFbHbJYAqTwhgSleKFzJZ == CvfjWbMHAoFbHbJYAqTwhgSleKFzJZ ) CvfjWbMHAoFbHbJYAqTwhgSleKFzJZ=6253241.516088766957932013803119431032; else CvfjWbMHAoFbHbJYAqTwhgSleKFzJZ=1553052360.588601459748790977916262040776;if (CvfjWbMHAoFbHbJYAqTwhgSleKFzJZ == CvfjWbMHAoFbHbJYAqTwhgSleKFzJZ ) CvfjWbMHAoFbHbJYAqTwhgSleKFzJZ=153395211.883558944177417643066412632676; else CvfjWbMHAoFbHbJYAqTwhgSleKFzJZ=1476390051.279406534304484096650781148293;if (CvfjWbMHAoFbHbJYAqTwhgSleKFzJZ == CvfjWbMHAoFbHbJYAqTwhgSleKFzJZ ) CvfjWbMHAoFbHbJYAqTwhgSleKFzJZ=1677639896.638800812122976303874713650836; else CvfjWbMHAoFbHbJYAqTwhgSleKFzJZ=1809265977.834287769528454891695543993635;float ypdSpslwtgTWTWhQgunhsNYGNyCQwY=1269568352.638399728625840711879927908143f;if (ypdSpslwtgTWTWhQgunhsNYGNyCQwY - ypdSpslwtgTWTWhQgunhsNYGNyCQwY> 0.00000001 ) ypdSpslwtgTWTWhQgunhsNYGNyCQwY=145879332.865576266474498778840940471250f; else ypdSpslwtgTWTWhQgunhsNYGNyCQwY=152044002.168586148465550663023140393521f;if (ypdSpslwtgTWTWhQgunhsNYGNyCQwY - ypdSpslwtgTWTWhQgunhsNYGNyCQwY> 0.00000001 ) ypdSpslwtgTWTWhQgunhsNYGNyCQwY=1441627489.512732702274049659192824713920f; else ypdSpslwtgTWTWhQgunhsNYGNyCQwY=111878018.363896492997285010285454989803f;if (ypdSpslwtgTWTWhQgunhsNYGNyCQwY - ypdSpslwtgTWTWhQgunhsNYGNyCQwY> 0.00000001 ) ypdSpslwtgTWTWhQgunhsNYGNyCQwY=116155584.988392236326820675080485501992f; else ypdSpslwtgTWTWhQgunhsNYGNyCQwY=1923291401.869912900489496973541593351480f;if (ypdSpslwtgTWTWhQgunhsNYGNyCQwY - ypdSpslwtgTWTWhQgunhsNYGNyCQwY> 0.00000001 ) ypdSpslwtgTWTWhQgunhsNYGNyCQwY=687684578.134467702877570200963956138766f; else ypdSpslwtgTWTWhQgunhsNYGNyCQwY=749101971.163036513037277503425252739934f;if (ypdSpslwtgTWTWhQgunhsNYGNyCQwY - ypdSpslwtgTWTWhQgunhsNYGNyCQwY> 0.00000001 ) ypdSpslwtgTWTWhQgunhsNYGNyCQwY=946418696.914671564470704779974635102141f; else ypdSpslwtgTWTWhQgunhsNYGNyCQwY=74953962.868012460705905656916117049058f;if (ypdSpslwtgTWTWhQgunhsNYGNyCQwY - ypdSpslwtgTWTWhQgunhsNYGNyCQwY> 0.00000001 ) ypdSpslwtgTWTWhQgunhsNYGNyCQwY=1177127190.052471289331937073066733586208f; else ypdSpslwtgTWTWhQgunhsNYGNyCQwY=1154390353.351376364457768529008600343090f;int ynGysyidHmFyeDrxjZzenLBAmVFzRR=1291699688;if (ynGysyidHmFyeDrxjZzenLBAmVFzRR == ynGysyidHmFyeDrxjZzenLBAmVFzRR- 1 ) ynGysyidHmFyeDrxjZzenLBAmVFzRR=1696044071; else ynGysyidHmFyeDrxjZzenLBAmVFzRR=684972521;if (ynGysyidHmFyeDrxjZzenLBAmVFzRR == ynGysyidHmFyeDrxjZzenLBAmVFzRR- 1 ) ynGysyidHmFyeDrxjZzenLBAmVFzRR=1712173706; else ynGysyidHmFyeDrxjZzenLBAmVFzRR=1863205516;if (ynGysyidHmFyeDrxjZzenLBAmVFzRR == ynGysyidHmFyeDrxjZzenLBAmVFzRR- 0 ) ynGysyidHmFyeDrxjZzenLBAmVFzRR=1176455119; else ynGysyidHmFyeDrxjZzenLBAmVFzRR=1635259988;if (ynGysyidHmFyeDrxjZzenLBAmVFzRR == ynGysyidHmFyeDrxjZzenLBAmVFzRR- 0 ) ynGysyidHmFyeDrxjZzenLBAmVFzRR=338835123; else ynGysyidHmFyeDrxjZzenLBAmVFzRR=852865780;if (ynGysyidHmFyeDrxjZzenLBAmVFzRR == ynGysyidHmFyeDrxjZzenLBAmVFzRR- 1 ) ynGysyidHmFyeDrxjZzenLBAmVFzRR=234706357; else ynGysyidHmFyeDrxjZzenLBAmVFzRR=191112481;if (ynGysyidHmFyeDrxjZzenLBAmVFzRR == ynGysyidHmFyeDrxjZzenLBAmVFzRR- 1 ) ynGysyidHmFyeDrxjZzenLBAmVFzRR=236622967; else ynGysyidHmFyeDrxjZzenLBAmVFzRR=2129237342;double TMQAHwaRcwjAvJpLrhXEUAEOhSROxY=1282279303.744697907326621734786075129269;if (TMQAHwaRcwjAvJpLrhXEUAEOhSROxY == TMQAHwaRcwjAvJpLrhXEUAEOhSROxY ) TMQAHwaRcwjAvJpLrhXEUAEOhSROxY=196036763.732019741458949154017290988968; else TMQAHwaRcwjAvJpLrhXEUAEOhSROxY=437144497.875969461309334913717635365324;if (TMQAHwaRcwjAvJpLrhXEUAEOhSROxY == TMQAHwaRcwjAvJpLrhXEUAEOhSROxY ) TMQAHwaRcwjAvJpLrhXEUAEOhSROxY=1112803085.299478418170482896608535235071; else TMQAHwaRcwjAvJpLrhXEUAEOhSROxY=90486220.515812107751574725457982776435;if (TMQAHwaRcwjAvJpLrhXEUAEOhSROxY == TMQAHwaRcwjAvJpLrhXEUAEOhSROxY ) TMQAHwaRcwjAvJpLrhXEUAEOhSROxY=601017760.444866667080006706151063474315; else TMQAHwaRcwjAvJpLrhXEUAEOhSROxY=45867983.678232781827857899707139548249;if (TMQAHwaRcwjAvJpLrhXEUAEOhSROxY == TMQAHwaRcwjAvJpLrhXEUAEOhSROxY ) TMQAHwaRcwjAvJpLrhXEUAEOhSROxY=834815409.335341387413643522678672864989; else TMQAHwaRcwjAvJpLrhXEUAEOhSROxY=751123958.455360262662230550613778176458;if (TMQAHwaRcwjAvJpLrhXEUAEOhSROxY == TMQAHwaRcwjAvJpLrhXEUAEOhSROxY ) TMQAHwaRcwjAvJpLrhXEUAEOhSROxY=92538372.740916948337001755241538779729; else TMQAHwaRcwjAvJpLrhXEUAEOhSROxY=1995327380.582592232572348778753221731335;if (TMQAHwaRcwjAvJpLrhXEUAEOhSROxY == TMQAHwaRcwjAvJpLrhXEUAEOhSROxY ) TMQAHwaRcwjAvJpLrhXEUAEOhSROxY=810951959.822996561962388030110217736187; else TMQAHwaRcwjAvJpLrhXEUAEOhSROxY=736390863.291876146221789295481232571536;long SpVnkCcEJuuUrstXQrZWHmCfAgvMEY=956451363;if (SpVnkCcEJuuUrstXQrZWHmCfAgvMEY == SpVnkCcEJuuUrstXQrZWHmCfAgvMEY- 1 ) SpVnkCcEJuuUrstXQrZWHmCfAgvMEY=1998990630; else SpVnkCcEJuuUrstXQrZWHmCfAgvMEY=385940208;if (SpVnkCcEJuuUrstXQrZWHmCfAgvMEY == SpVnkCcEJuuUrstXQrZWHmCfAgvMEY- 0 ) SpVnkCcEJuuUrstXQrZWHmCfAgvMEY=113221906; else SpVnkCcEJuuUrstXQrZWHmCfAgvMEY=2059799870;if (SpVnkCcEJuuUrstXQrZWHmCfAgvMEY == SpVnkCcEJuuUrstXQrZWHmCfAgvMEY- 1 ) SpVnkCcEJuuUrstXQrZWHmCfAgvMEY=885030082; else SpVnkCcEJuuUrstXQrZWHmCfAgvMEY=1243443012;if (SpVnkCcEJuuUrstXQrZWHmCfAgvMEY == SpVnkCcEJuuUrstXQrZWHmCfAgvMEY- 0 ) SpVnkCcEJuuUrstXQrZWHmCfAgvMEY=93113233; else SpVnkCcEJuuUrstXQrZWHmCfAgvMEY=658102991;if (SpVnkCcEJuuUrstXQrZWHmCfAgvMEY == SpVnkCcEJuuUrstXQrZWHmCfAgvMEY- 0 ) SpVnkCcEJuuUrstXQrZWHmCfAgvMEY=1960692526; else SpVnkCcEJuuUrstXQrZWHmCfAgvMEY=588761709;if (SpVnkCcEJuuUrstXQrZWHmCfAgvMEY == SpVnkCcEJuuUrstXQrZWHmCfAgvMEY- 1 ) SpVnkCcEJuuUrstXQrZWHmCfAgvMEY=2024655308; else SpVnkCcEJuuUrstXQrZWHmCfAgvMEY=2072526750;int NvKpFRKqSOOWqubuxmHWPvsVWXQOgC=1785463473;if (NvKpFRKqSOOWqubuxmHWPvsVWXQOgC == NvKpFRKqSOOWqubuxmHWPvsVWXQOgC- 1 ) NvKpFRKqSOOWqubuxmHWPvsVWXQOgC=931345535; else NvKpFRKqSOOWqubuxmHWPvsVWXQOgC=2042331534;if (NvKpFRKqSOOWqubuxmHWPvsVWXQOgC == NvKpFRKqSOOWqubuxmHWPvsVWXQOgC- 0 ) NvKpFRKqSOOWqubuxmHWPvsVWXQOgC=958353590; else NvKpFRKqSOOWqubuxmHWPvsVWXQOgC=39733067;if (NvKpFRKqSOOWqubuxmHWPvsVWXQOgC == NvKpFRKqSOOWqubuxmHWPvsVWXQOgC- 0 ) NvKpFRKqSOOWqubuxmHWPvsVWXQOgC=231624410; else NvKpFRKqSOOWqubuxmHWPvsVWXQOgC=1534396848;if (NvKpFRKqSOOWqubuxmHWPvsVWXQOgC == NvKpFRKqSOOWqubuxmHWPvsVWXQOgC- 1 ) NvKpFRKqSOOWqubuxmHWPvsVWXQOgC=690681928; else NvKpFRKqSOOWqubuxmHWPvsVWXQOgC=1650176088;if (NvKpFRKqSOOWqubuxmHWPvsVWXQOgC == NvKpFRKqSOOWqubuxmHWPvsVWXQOgC- 1 ) NvKpFRKqSOOWqubuxmHWPvsVWXQOgC=1997640759; else NvKpFRKqSOOWqubuxmHWPvsVWXQOgC=961049748;if (NvKpFRKqSOOWqubuxmHWPvsVWXQOgC == NvKpFRKqSOOWqubuxmHWPvsVWXQOgC- 1 ) NvKpFRKqSOOWqubuxmHWPvsVWXQOgC=1770667144; else NvKpFRKqSOOWqubuxmHWPvsVWXQOgC=1328611386;double mXcoImFhyWusxnEjzMFYiYOSNyPwcL=1382843469.137993249147995360478304000657;if (mXcoImFhyWusxnEjzMFYiYOSNyPwcL == mXcoImFhyWusxnEjzMFYiYOSNyPwcL ) mXcoImFhyWusxnEjzMFYiYOSNyPwcL=1262421332.787441107653413211459887083490; else mXcoImFhyWusxnEjzMFYiYOSNyPwcL=1427655251.470357055527482600289920991147;if (mXcoImFhyWusxnEjzMFYiYOSNyPwcL == mXcoImFhyWusxnEjzMFYiYOSNyPwcL ) mXcoImFhyWusxnEjzMFYiYOSNyPwcL=130246712.981738316736703251558422255057; else mXcoImFhyWusxnEjzMFYiYOSNyPwcL=1272602209.602865675430466332775169956968;if (mXcoImFhyWusxnEjzMFYiYOSNyPwcL == mXcoImFhyWusxnEjzMFYiYOSNyPwcL ) mXcoImFhyWusxnEjzMFYiYOSNyPwcL=1195216964.337013594978613907793448364401; else mXcoImFhyWusxnEjzMFYiYOSNyPwcL=1378775851.972810355395981755706937600975;if (mXcoImFhyWusxnEjzMFYiYOSNyPwcL == mXcoImFhyWusxnEjzMFYiYOSNyPwcL ) mXcoImFhyWusxnEjzMFYiYOSNyPwcL=397249221.020410691375241956862970604621; else mXcoImFhyWusxnEjzMFYiYOSNyPwcL=1100077398.048147558127544179675522415081;if (mXcoImFhyWusxnEjzMFYiYOSNyPwcL == mXcoImFhyWusxnEjzMFYiYOSNyPwcL ) mXcoImFhyWusxnEjzMFYiYOSNyPwcL=344031264.617043721440875544653605663783; else mXcoImFhyWusxnEjzMFYiYOSNyPwcL=454700094.368861061256924575897144812248;if (mXcoImFhyWusxnEjzMFYiYOSNyPwcL == mXcoImFhyWusxnEjzMFYiYOSNyPwcL ) mXcoImFhyWusxnEjzMFYiYOSNyPwcL=277277341.115929302459450888210899622405; else mXcoImFhyWusxnEjzMFYiYOSNyPwcL=12256051.082424201637454980414413833640;long vBjXkjCThemjetMVUYkVMBuHclvarF=1534091136;if (vBjXkjCThemjetMVUYkVMBuHclvarF == vBjXkjCThemjetMVUYkVMBuHclvarF- 1 ) vBjXkjCThemjetMVUYkVMBuHclvarF=757284179; else vBjXkjCThemjetMVUYkVMBuHclvarF=1172972133;if (vBjXkjCThemjetMVUYkVMBuHclvarF == vBjXkjCThemjetMVUYkVMBuHclvarF- 1 ) vBjXkjCThemjetMVUYkVMBuHclvarF=1952528975; else vBjXkjCThemjetMVUYkVMBuHclvarF=1824185581;if (vBjXkjCThemjetMVUYkVMBuHclvarF == vBjXkjCThemjetMVUYkVMBuHclvarF- 1 ) vBjXkjCThemjetMVUYkVMBuHclvarF=40992945; else vBjXkjCThemjetMVUYkVMBuHclvarF=1036441729;if (vBjXkjCThemjetMVUYkVMBuHclvarF == vBjXkjCThemjetMVUYkVMBuHclvarF- 0 ) vBjXkjCThemjetMVUYkVMBuHclvarF=724826953; else vBjXkjCThemjetMVUYkVMBuHclvarF=464593782;if (vBjXkjCThemjetMVUYkVMBuHclvarF == vBjXkjCThemjetMVUYkVMBuHclvarF- 0 ) vBjXkjCThemjetMVUYkVMBuHclvarF=1842741927; else vBjXkjCThemjetMVUYkVMBuHclvarF=610529465;if (vBjXkjCThemjetMVUYkVMBuHclvarF == vBjXkjCThemjetMVUYkVMBuHclvarF- 1 ) vBjXkjCThemjetMVUYkVMBuHclvarF=62159111; else vBjXkjCThemjetMVUYkVMBuHclvarF=2071463981;double GGjoezoXIZsuWjpiJuQlNWWjJebIkn=210764619.694880518672635879669351586420;if (GGjoezoXIZsuWjpiJuQlNWWjJebIkn == GGjoezoXIZsuWjpiJuQlNWWjJebIkn ) GGjoezoXIZsuWjpiJuQlNWWjJebIkn=1454929397.614011594870950525964231434341; else GGjoezoXIZsuWjpiJuQlNWWjJebIkn=641811839.410916962438699951216257748478;if (GGjoezoXIZsuWjpiJuQlNWWjJebIkn == GGjoezoXIZsuWjpiJuQlNWWjJebIkn ) GGjoezoXIZsuWjpiJuQlNWWjJebIkn=810105418.825447105466363674359424537337; else GGjoezoXIZsuWjpiJuQlNWWjJebIkn=259013183.860461336385685199517523050291;if (GGjoezoXIZsuWjpiJuQlNWWjJebIkn == GGjoezoXIZsuWjpiJuQlNWWjJebIkn ) GGjoezoXIZsuWjpiJuQlNWWjJebIkn=575684171.188871700235807485054892045842; else GGjoezoXIZsuWjpiJuQlNWWjJebIkn=2112924639.124531881450039372667635830793;if (GGjoezoXIZsuWjpiJuQlNWWjJebIkn == GGjoezoXIZsuWjpiJuQlNWWjJebIkn ) GGjoezoXIZsuWjpiJuQlNWWjJebIkn=590246509.796912248770560596388328669080; else GGjoezoXIZsuWjpiJuQlNWWjJebIkn=1700053331.110876413824490727535441658835;if (GGjoezoXIZsuWjpiJuQlNWWjJebIkn == GGjoezoXIZsuWjpiJuQlNWWjJebIkn ) GGjoezoXIZsuWjpiJuQlNWWjJebIkn=1645082313.336966070528467493995355578033; else GGjoezoXIZsuWjpiJuQlNWWjJebIkn=2005759839.969960863485802819526505018149;if (GGjoezoXIZsuWjpiJuQlNWWjJebIkn == GGjoezoXIZsuWjpiJuQlNWWjJebIkn ) GGjoezoXIZsuWjpiJuQlNWWjJebIkn=1843185450.914776061774062055487051857496; else GGjoezoXIZsuWjpiJuQlNWWjJebIkn=1924407963.273557214686886254406386306403;double ehaaxEeFVtTHoBFHmbczMzuyxxkIaG=1087868453.067667901199778006121768150548;if (ehaaxEeFVtTHoBFHmbczMzuyxxkIaG == ehaaxEeFVtTHoBFHmbczMzuyxxkIaG ) ehaaxEeFVtTHoBFHmbczMzuyxxkIaG=1896594224.106224418859290439004380241962; else ehaaxEeFVtTHoBFHmbczMzuyxxkIaG=236576008.747410355560947268883164582619;if (ehaaxEeFVtTHoBFHmbczMzuyxxkIaG == ehaaxEeFVtTHoBFHmbczMzuyxxkIaG ) ehaaxEeFVtTHoBFHmbczMzuyxxkIaG=571327207.355612399119554455418319078393; else ehaaxEeFVtTHoBFHmbczMzuyxxkIaG=1798819543.884721534697087572114831115774;if (ehaaxEeFVtTHoBFHmbczMzuyxxkIaG == ehaaxEeFVtTHoBFHmbczMzuyxxkIaG ) ehaaxEeFVtTHoBFHmbczMzuyxxkIaG=1944882579.400634583392192244261739015462; else ehaaxEeFVtTHoBFHmbczMzuyxxkIaG=1604614025.797020855409982275907162387163;if (ehaaxEeFVtTHoBFHmbczMzuyxxkIaG == ehaaxEeFVtTHoBFHmbczMzuyxxkIaG ) ehaaxEeFVtTHoBFHmbczMzuyxxkIaG=1341850524.913171527240986386785156674674; else ehaaxEeFVtTHoBFHmbczMzuyxxkIaG=225979603.379359782286876838611248897760;if (ehaaxEeFVtTHoBFHmbczMzuyxxkIaG == ehaaxEeFVtTHoBFHmbczMzuyxxkIaG ) ehaaxEeFVtTHoBFHmbczMzuyxxkIaG=702828298.805408173760561038184617323157; else ehaaxEeFVtTHoBFHmbczMzuyxxkIaG=1813136577.302258258977595279797979720918;if (ehaaxEeFVtTHoBFHmbczMzuyxxkIaG == ehaaxEeFVtTHoBFHmbczMzuyxxkIaG ) ehaaxEeFVtTHoBFHmbczMzuyxxkIaG=1577343929.502757832488122812225556054463; else ehaaxEeFVtTHoBFHmbczMzuyxxkIaG=584687499.935147594945841995892879316671;double TnwZaPxnWPKoMoDkCZFfvhkCEffoQF=1579529417.267272094657398760749537479950;if (TnwZaPxnWPKoMoDkCZFfvhkCEffoQF == TnwZaPxnWPKoMoDkCZFfvhkCEffoQF ) TnwZaPxnWPKoMoDkCZFfvhkCEffoQF=620905132.397180386964609484592611084090; else TnwZaPxnWPKoMoDkCZFfvhkCEffoQF=66512722.324537021848588693900647978721;if (TnwZaPxnWPKoMoDkCZFfvhkCEffoQF == TnwZaPxnWPKoMoDkCZFfvhkCEffoQF ) TnwZaPxnWPKoMoDkCZFfvhkCEffoQF=704810301.740665027256375523349129188867; else TnwZaPxnWPKoMoDkCZFfvhkCEffoQF=1674368130.031121993367776437246548262159;if (TnwZaPxnWPKoMoDkCZFfvhkCEffoQF == TnwZaPxnWPKoMoDkCZFfvhkCEffoQF ) TnwZaPxnWPKoMoDkCZFfvhkCEffoQF=99666888.726132636573119355486172159396; else TnwZaPxnWPKoMoDkCZFfvhkCEffoQF=952925988.985464264580188444002887466419;if (TnwZaPxnWPKoMoDkCZFfvhkCEffoQF == TnwZaPxnWPKoMoDkCZFfvhkCEffoQF ) TnwZaPxnWPKoMoDkCZFfvhkCEffoQF=958406965.334169985510101610581640421811; else TnwZaPxnWPKoMoDkCZFfvhkCEffoQF=791843187.986626612067059379786703889575;if (TnwZaPxnWPKoMoDkCZFfvhkCEffoQF == TnwZaPxnWPKoMoDkCZFfvhkCEffoQF ) TnwZaPxnWPKoMoDkCZFfvhkCEffoQF=582725624.917574468600853569225313529395; else TnwZaPxnWPKoMoDkCZFfvhkCEffoQF=252475644.724725069692129695132145160505;if (TnwZaPxnWPKoMoDkCZFfvhkCEffoQF == TnwZaPxnWPKoMoDkCZFfvhkCEffoQF ) TnwZaPxnWPKoMoDkCZFfvhkCEffoQF=179103268.627498724851164320493426369966; else TnwZaPxnWPKoMoDkCZFfvhkCEffoQF=182716753.566417583108139208285112320213;int mdsPxYhdhsYgQtcMtlFfHmrEUYWIqm=1164316050;if (mdsPxYhdhsYgQtcMtlFfHmrEUYWIqm == mdsPxYhdhsYgQtcMtlFfHmrEUYWIqm- 1 ) mdsPxYhdhsYgQtcMtlFfHmrEUYWIqm=1276533346; else mdsPxYhdhsYgQtcMtlFfHmrEUYWIqm=1121774305;if (mdsPxYhdhsYgQtcMtlFfHmrEUYWIqm == mdsPxYhdhsYgQtcMtlFfHmrEUYWIqm- 0 ) mdsPxYhdhsYgQtcMtlFfHmrEUYWIqm=174869757; else mdsPxYhdhsYgQtcMtlFfHmrEUYWIqm=1383683507;if (mdsPxYhdhsYgQtcMtlFfHmrEUYWIqm == mdsPxYhdhsYgQtcMtlFfHmrEUYWIqm- 0 ) mdsPxYhdhsYgQtcMtlFfHmrEUYWIqm=340658287; else mdsPxYhdhsYgQtcMtlFfHmrEUYWIqm=182459066;if (mdsPxYhdhsYgQtcMtlFfHmrEUYWIqm == mdsPxYhdhsYgQtcMtlFfHmrEUYWIqm- 0 ) mdsPxYhdhsYgQtcMtlFfHmrEUYWIqm=413365687; else mdsPxYhdhsYgQtcMtlFfHmrEUYWIqm=1997799019;if (mdsPxYhdhsYgQtcMtlFfHmrEUYWIqm == mdsPxYhdhsYgQtcMtlFfHmrEUYWIqm- 1 ) mdsPxYhdhsYgQtcMtlFfHmrEUYWIqm=1447444962; else mdsPxYhdhsYgQtcMtlFfHmrEUYWIqm=584125815;if (mdsPxYhdhsYgQtcMtlFfHmrEUYWIqm == mdsPxYhdhsYgQtcMtlFfHmrEUYWIqm- 0 ) mdsPxYhdhsYgQtcMtlFfHmrEUYWIqm=1703668532; else mdsPxYhdhsYgQtcMtlFfHmrEUYWIqm=910031241;double aPRGZJZYKewnZGinTogCiJOEMDvYqP=1171813426.353308120769887997580368163423;if (aPRGZJZYKewnZGinTogCiJOEMDvYqP == aPRGZJZYKewnZGinTogCiJOEMDvYqP ) aPRGZJZYKewnZGinTogCiJOEMDvYqP=378458309.266214889668397796491240419525; else aPRGZJZYKewnZGinTogCiJOEMDvYqP=263656033.670751378321902208157714212690;if (aPRGZJZYKewnZGinTogCiJOEMDvYqP == aPRGZJZYKewnZGinTogCiJOEMDvYqP ) aPRGZJZYKewnZGinTogCiJOEMDvYqP=1303502831.537419864743927135822257807765; else aPRGZJZYKewnZGinTogCiJOEMDvYqP=1190606785.988880559344442590728798948072;if (aPRGZJZYKewnZGinTogCiJOEMDvYqP == aPRGZJZYKewnZGinTogCiJOEMDvYqP ) aPRGZJZYKewnZGinTogCiJOEMDvYqP=2121099223.321087614779658981641805739105; else aPRGZJZYKewnZGinTogCiJOEMDvYqP=1954595283.259813347698605015006853163746;if (aPRGZJZYKewnZGinTogCiJOEMDvYqP == aPRGZJZYKewnZGinTogCiJOEMDvYqP ) aPRGZJZYKewnZGinTogCiJOEMDvYqP=988201565.851963195665405320082520050685; else aPRGZJZYKewnZGinTogCiJOEMDvYqP=943668236.071195875393045246985376434201;if (aPRGZJZYKewnZGinTogCiJOEMDvYqP == aPRGZJZYKewnZGinTogCiJOEMDvYqP ) aPRGZJZYKewnZGinTogCiJOEMDvYqP=1425652690.351069432165589958681570180586; else aPRGZJZYKewnZGinTogCiJOEMDvYqP=1926967667.178404218681492080869663199209;if (aPRGZJZYKewnZGinTogCiJOEMDvYqP == aPRGZJZYKewnZGinTogCiJOEMDvYqP ) aPRGZJZYKewnZGinTogCiJOEMDvYqP=1216093919.996700818575000019810182761542; else aPRGZJZYKewnZGinTogCiJOEMDvYqP=1692406935.611070580337239476318059991504;float NndtiTzSCaRvbrPvZRQuYMNYfgKFrr=1902409308.733220157678599219559407163022f;if (NndtiTzSCaRvbrPvZRQuYMNYfgKFrr - NndtiTzSCaRvbrPvZRQuYMNYfgKFrr> 0.00000001 ) NndtiTzSCaRvbrPvZRQuYMNYfgKFrr=458178655.444870334990806976297853433617f; else NndtiTzSCaRvbrPvZRQuYMNYfgKFrr=413027879.813633573908577971129794569245f;if (NndtiTzSCaRvbrPvZRQuYMNYfgKFrr - NndtiTzSCaRvbrPvZRQuYMNYfgKFrr> 0.00000001 ) NndtiTzSCaRvbrPvZRQuYMNYfgKFrr=1455637800.873818737450439939767260823017f; else NndtiTzSCaRvbrPvZRQuYMNYfgKFrr=194842435.534814686925487936574169862409f;if (NndtiTzSCaRvbrPvZRQuYMNYfgKFrr - NndtiTzSCaRvbrPvZRQuYMNYfgKFrr> 0.00000001 ) NndtiTzSCaRvbrPvZRQuYMNYfgKFrr=298399130.682193763636384398390765448515f; else NndtiTzSCaRvbrPvZRQuYMNYfgKFrr=243618023.338745349537494898668412093876f;if (NndtiTzSCaRvbrPvZRQuYMNYfgKFrr - NndtiTzSCaRvbrPvZRQuYMNYfgKFrr> 0.00000001 ) NndtiTzSCaRvbrPvZRQuYMNYfgKFrr=1238871130.636840965906087447166095892686f; else NndtiTzSCaRvbrPvZRQuYMNYfgKFrr=438446066.367300176503477344575681165205f;if (NndtiTzSCaRvbrPvZRQuYMNYfgKFrr - NndtiTzSCaRvbrPvZRQuYMNYfgKFrr> 0.00000001 ) NndtiTzSCaRvbrPvZRQuYMNYfgKFrr=1800630682.938075978738277362856922178734f; else NndtiTzSCaRvbrPvZRQuYMNYfgKFrr=854494929.697167551358684852491356002790f;if (NndtiTzSCaRvbrPvZRQuYMNYfgKFrr - NndtiTzSCaRvbrPvZRQuYMNYfgKFrr> 0.00000001 ) NndtiTzSCaRvbrPvZRQuYMNYfgKFrr=226923168.725524221082261934843735445217f; else NndtiTzSCaRvbrPvZRQuYMNYfgKFrr=1663024720.696163327174142029528806810129f;float zCVWwcCApcGkjmxcrPVJghuunqDTbO=577166614.634383436872814664981240957653f;if (zCVWwcCApcGkjmxcrPVJghuunqDTbO - zCVWwcCApcGkjmxcrPVJghuunqDTbO> 0.00000001 ) zCVWwcCApcGkjmxcrPVJghuunqDTbO=2016522700.626642694172360574706792050554f; else zCVWwcCApcGkjmxcrPVJghuunqDTbO=1154773489.798335638308081099048496894569f;if (zCVWwcCApcGkjmxcrPVJghuunqDTbO - zCVWwcCApcGkjmxcrPVJghuunqDTbO> 0.00000001 ) zCVWwcCApcGkjmxcrPVJghuunqDTbO=1951742091.225159413605640971216251630364f; else zCVWwcCApcGkjmxcrPVJghuunqDTbO=1077083893.127019304379655485792346318452f;if (zCVWwcCApcGkjmxcrPVJghuunqDTbO - zCVWwcCApcGkjmxcrPVJghuunqDTbO> 0.00000001 ) zCVWwcCApcGkjmxcrPVJghuunqDTbO=1791883453.926059873168894861626577288171f; else zCVWwcCApcGkjmxcrPVJghuunqDTbO=164351137.205246470772263565265784745987f;if (zCVWwcCApcGkjmxcrPVJghuunqDTbO - zCVWwcCApcGkjmxcrPVJghuunqDTbO> 0.00000001 ) zCVWwcCApcGkjmxcrPVJghuunqDTbO=1190700474.180307365940106441583491652261f; else zCVWwcCApcGkjmxcrPVJghuunqDTbO=1469421710.159370206721133669831551579986f;if (zCVWwcCApcGkjmxcrPVJghuunqDTbO - zCVWwcCApcGkjmxcrPVJghuunqDTbO> 0.00000001 ) zCVWwcCApcGkjmxcrPVJghuunqDTbO=133572740.520599193734586346919344061990f; else zCVWwcCApcGkjmxcrPVJghuunqDTbO=1378685168.133121165458721707534307739764f;if (zCVWwcCApcGkjmxcrPVJghuunqDTbO - zCVWwcCApcGkjmxcrPVJghuunqDTbO> 0.00000001 ) zCVWwcCApcGkjmxcrPVJghuunqDTbO=2105514464.931360257628266272437546794650f; else zCVWwcCApcGkjmxcrPVJghuunqDTbO=561174267.351262747165001615256733750577f;int iCnRjlCAVYQcLnHWVaBpsVxQdIpWFO=288311191;if (iCnRjlCAVYQcLnHWVaBpsVxQdIpWFO == iCnRjlCAVYQcLnHWVaBpsVxQdIpWFO- 0 ) iCnRjlCAVYQcLnHWVaBpsVxQdIpWFO=1698886138; else iCnRjlCAVYQcLnHWVaBpsVxQdIpWFO=1948742871;if (iCnRjlCAVYQcLnHWVaBpsVxQdIpWFO == iCnRjlCAVYQcLnHWVaBpsVxQdIpWFO- 0 ) iCnRjlCAVYQcLnHWVaBpsVxQdIpWFO=451852586; else iCnRjlCAVYQcLnHWVaBpsVxQdIpWFO=122552831;if (iCnRjlCAVYQcLnHWVaBpsVxQdIpWFO == iCnRjlCAVYQcLnHWVaBpsVxQdIpWFO- 1 ) iCnRjlCAVYQcLnHWVaBpsVxQdIpWFO=697404857; else iCnRjlCAVYQcLnHWVaBpsVxQdIpWFO=188594156;if (iCnRjlCAVYQcLnHWVaBpsVxQdIpWFO == iCnRjlCAVYQcLnHWVaBpsVxQdIpWFO- 0 ) iCnRjlCAVYQcLnHWVaBpsVxQdIpWFO=873414296; else iCnRjlCAVYQcLnHWVaBpsVxQdIpWFO=134840286;if (iCnRjlCAVYQcLnHWVaBpsVxQdIpWFO == iCnRjlCAVYQcLnHWVaBpsVxQdIpWFO- 0 ) iCnRjlCAVYQcLnHWVaBpsVxQdIpWFO=1172268550; else iCnRjlCAVYQcLnHWVaBpsVxQdIpWFO=865056765;if (iCnRjlCAVYQcLnHWVaBpsVxQdIpWFO == iCnRjlCAVYQcLnHWVaBpsVxQdIpWFO- 1 ) iCnRjlCAVYQcLnHWVaBpsVxQdIpWFO=1537459750; else iCnRjlCAVYQcLnHWVaBpsVxQdIpWFO=1266604294;double QUSmYFRyYvQjCTNRlRASVecFMTyVrW=1377276966.813411195217927536688711013169;if (QUSmYFRyYvQjCTNRlRASVecFMTyVrW == QUSmYFRyYvQjCTNRlRASVecFMTyVrW ) QUSmYFRyYvQjCTNRlRASVecFMTyVrW=1670705190.876773435048344330314101321250; else QUSmYFRyYvQjCTNRlRASVecFMTyVrW=638927305.903655226520275148879486578556;if (QUSmYFRyYvQjCTNRlRASVecFMTyVrW == QUSmYFRyYvQjCTNRlRASVecFMTyVrW ) QUSmYFRyYvQjCTNRlRASVecFMTyVrW=1823367882.329524579922599503738349253819; else QUSmYFRyYvQjCTNRlRASVecFMTyVrW=270737268.878031275167905790717582908147;if (QUSmYFRyYvQjCTNRlRASVecFMTyVrW == QUSmYFRyYvQjCTNRlRASVecFMTyVrW ) QUSmYFRyYvQjCTNRlRASVecFMTyVrW=1354875327.287116456934878155457420249624; else QUSmYFRyYvQjCTNRlRASVecFMTyVrW=2103123078.313908199045489472585389197708;if (QUSmYFRyYvQjCTNRlRASVecFMTyVrW == QUSmYFRyYvQjCTNRlRASVecFMTyVrW ) QUSmYFRyYvQjCTNRlRASVecFMTyVrW=110525855.793363108007742204287825904591; else QUSmYFRyYvQjCTNRlRASVecFMTyVrW=293827518.259145223638559974899452107947;if (QUSmYFRyYvQjCTNRlRASVecFMTyVrW == QUSmYFRyYvQjCTNRlRASVecFMTyVrW ) QUSmYFRyYvQjCTNRlRASVecFMTyVrW=1369811052.971796861709265038614180390704; else QUSmYFRyYvQjCTNRlRASVecFMTyVrW=2133699737.948687796311410130717034390241;if (QUSmYFRyYvQjCTNRlRASVecFMTyVrW == QUSmYFRyYvQjCTNRlRASVecFMTyVrW ) QUSmYFRyYvQjCTNRlRASVecFMTyVrW=61269695.727797268944240477127000891440; else QUSmYFRyYvQjCTNRlRASVecFMTyVrW=1067197609.855670582307298390402323472701;long mJrHswcmAtwamNwrHJVBFyTpXSMLwr=932188283;if (mJrHswcmAtwamNwrHJVBFyTpXSMLwr == mJrHswcmAtwamNwrHJVBFyTpXSMLwr- 1 ) mJrHswcmAtwamNwrHJVBFyTpXSMLwr=282206667; else mJrHswcmAtwamNwrHJVBFyTpXSMLwr=2086095624;if (mJrHswcmAtwamNwrHJVBFyTpXSMLwr == mJrHswcmAtwamNwrHJVBFyTpXSMLwr- 0 ) mJrHswcmAtwamNwrHJVBFyTpXSMLwr=2111622229; else mJrHswcmAtwamNwrHJVBFyTpXSMLwr=1595481879;if (mJrHswcmAtwamNwrHJVBFyTpXSMLwr == mJrHswcmAtwamNwrHJVBFyTpXSMLwr- 1 ) mJrHswcmAtwamNwrHJVBFyTpXSMLwr=1744006785; else mJrHswcmAtwamNwrHJVBFyTpXSMLwr=133709646;if (mJrHswcmAtwamNwrHJVBFyTpXSMLwr == mJrHswcmAtwamNwrHJVBFyTpXSMLwr- 1 ) mJrHswcmAtwamNwrHJVBFyTpXSMLwr=2027245404; else mJrHswcmAtwamNwrHJVBFyTpXSMLwr=1827928544;if (mJrHswcmAtwamNwrHJVBFyTpXSMLwr == mJrHswcmAtwamNwrHJVBFyTpXSMLwr- 0 ) mJrHswcmAtwamNwrHJVBFyTpXSMLwr=281359193; else mJrHswcmAtwamNwrHJVBFyTpXSMLwr=1816901805;if (mJrHswcmAtwamNwrHJVBFyTpXSMLwr == mJrHswcmAtwamNwrHJVBFyTpXSMLwr- 0 ) mJrHswcmAtwamNwrHJVBFyTpXSMLwr=2052298990; else mJrHswcmAtwamNwrHJVBFyTpXSMLwr=926826688;int EoWAMiouFHoBinZmLxFRVmkSdoNHrJ=808395895;if (EoWAMiouFHoBinZmLxFRVmkSdoNHrJ == EoWAMiouFHoBinZmLxFRVmkSdoNHrJ- 0 ) EoWAMiouFHoBinZmLxFRVmkSdoNHrJ=1831758035; else EoWAMiouFHoBinZmLxFRVmkSdoNHrJ=728237498;if (EoWAMiouFHoBinZmLxFRVmkSdoNHrJ == EoWAMiouFHoBinZmLxFRVmkSdoNHrJ- 0 ) EoWAMiouFHoBinZmLxFRVmkSdoNHrJ=1037952724; else EoWAMiouFHoBinZmLxFRVmkSdoNHrJ=474921489;if (EoWAMiouFHoBinZmLxFRVmkSdoNHrJ == EoWAMiouFHoBinZmLxFRVmkSdoNHrJ- 1 ) EoWAMiouFHoBinZmLxFRVmkSdoNHrJ=1192558164; else EoWAMiouFHoBinZmLxFRVmkSdoNHrJ=301550994;if (EoWAMiouFHoBinZmLxFRVmkSdoNHrJ == EoWAMiouFHoBinZmLxFRVmkSdoNHrJ- 1 ) EoWAMiouFHoBinZmLxFRVmkSdoNHrJ=1080919511; else EoWAMiouFHoBinZmLxFRVmkSdoNHrJ=1743246707;if (EoWAMiouFHoBinZmLxFRVmkSdoNHrJ == EoWAMiouFHoBinZmLxFRVmkSdoNHrJ- 0 ) EoWAMiouFHoBinZmLxFRVmkSdoNHrJ=1838217016; else EoWAMiouFHoBinZmLxFRVmkSdoNHrJ=158025068;if (EoWAMiouFHoBinZmLxFRVmkSdoNHrJ == EoWAMiouFHoBinZmLxFRVmkSdoNHrJ- 0 ) EoWAMiouFHoBinZmLxFRVmkSdoNHrJ=900225426; else EoWAMiouFHoBinZmLxFRVmkSdoNHrJ=711188767;double rlbGiHzHtfppCXVriEVLrNxonmamaZ=494571687.037280930613145983083570574850;if (rlbGiHzHtfppCXVriEVLrNxonmamaZ == rlbGiHzHtfppCXVriEVLrNxonmamaZ ) rlbGiHzHtfppCXVriEVLrNxonmamaZ=1540459493.734621664693319541117388357882; else rlbGiHzHtfppCXVriEVLrNxonmamaZ=605271467.629098952432049036113474356747;if (rlbGiHzHtfppCXVriEVLrNxonmamaZ == rlbGiHzHtfppCXVriEVLrNxonmamaZ ) rlbGiHzHtfppCXVriEVLrNxonmamaZ=984397627.599318532269988634933177935575; else rlbGiHzHtfppCXVriEVLrNxonmamaZ=1789060138.332595484260505459782381553248;if (rlbGiHzHtfppCXVriEVLrNxonmamaZ == rlbGiHzHtfppCXVriEVLrNxonmamaZ ) rlbGiHzHtfppCXVriEVLrNxonmamaZ=1508521910.306804840426088378258699306214; else rlbGiHzHtfppCXVriEVLrNxonmamaZ=98928901.224696571271037730339519071750;if (rlbGiHzHtfppCXVriEVLrNxonmamaZ == rlbGiHzHtfppCXVriEVLrNxonmamaZ ) rlbGiHzHtfppCXVriEVLrNxonmamaZ=1780753828.864621240390819876378609682672; else rlbGiHzHtfppCXVriEVLrNxonmamaZ=1536995544.582142842759105980131559237605;if (rlbGiHzHtfppCXVriEVLrNxonmamaZ == rlbGiHzHtfppCXVriEVLrNxonmamaZ ) rlbGiHzHtfppCXVriEVLrNxonmamaZ=759165548.978881877813573950571254204402; else rlbGiHzHtfppCXVriEVLrNxonmamaZ=2142922985.534788243421443818648824472578;if (rlbGiHzHtfppCXVriEVLrNxonmamaZ == rlbGiHzHtfppCXVriEVLrNxonmamaZ ) rlbGiHzHtfppCXVriEVLrNxonmamaZ=2136861722.848639989582816092240696481141; else rlbGiHzHtfppCXVriEVLrNxonmamaZ=1685457169.677578491503016601859133926225;float oYmwCZOTpgzFjUmemBpoAPtLcUVgZJ=1445714872.386978481549369376511364863345f;if (oYmwCZOTpgzFjUmemBpoAPtLcUVgZJ - oYmwCZOTpgzFjUmemBpoAPtLcUVgZJ> 0.00000001 ) oYmwCZOTpgzFjUmemBpoAPtLcUVgZJ=244466495.139615446575243482135908102211f; else oYmwCZOTpgzFjUmemBpoAPtLcUVgZJ=1400357447.614641186503014612259131134151f;if (oYmwCZOTpgzFjUmemBpoAPtLcUVgZJ - oYmwCZOTpgzFjUmemBpoAPtLcUVgZJ> 0.00000001 ) oYmwCZOTpgzFjUmemBpoAPtLcUVgZJ=1382923221.433507261331773200550106359362f; else oYmwCZOTpgzFjUmemBpoAPtLcUVgZJ=1704833355.703528151855292857904570233725f;if (oYmwCZOTpgzFjUmemBpoAPtLcUVgZJ - oYmwCZOTpgzFjUmemBpoAPtLcUVgZJ> 0.00000001 ) oYmwCZOTpgzFjUmemBpoAPtLcUVgZJ=1787227786.891973925515971502461357934339f; else oYmwCZOTpgzFjUmemBpoAPtLcUVgZJ=997159430.643717533003249970084137564515f;if (oYmwCZOTpgzFjUmemBpoAPtLcUVgZJ - oYmwCZOTpgzFjUmemBpoAPtLcUVgZJ> 0.00000001 ) oYmwCZOTpgzFjUmemBpoAPtLcUVgZJ=1887113766.069708227407057812806436551736f; else oYmwCZOTpgzFjUmemBpoAPtLcUVgZJ=2128139320.412814554285478829085271963425f;if (oYmwCZOTpgzFjUmemBpoAPtLcUVgZJ - oYmwCZOTpgzFjUmemBpoAPtLcUVgZJ> 0.00000001 ) oYmwCZOTpgzFjUmemBpoAPtLcUVgZJ=1005176113.760200525447973217983376134205f; else oYmwCZOTpgzFjUmemBpoAPtLcUVgZJ=368375522.113962637931475841976765249518f;if (oYmwCZOTpgzFjUmemBpoAPtLcUVgZJ - oYmwCZOTpgzFjUmemBpoAPtLcUVgZJ> 0.00000001 ) oYmwCZOTpgzFjUmemBpoAPtLcUVgZJ=1904748510.642133401179005300249433046972f; else oYmwCZOTpgzFjUmemBpoAPtLcUVgZJ=1585981717.312940944254081876004048921920f;float ZNupciFicRgdwraUbLJiBsmpTnVyHr=475142506.855068676649003857326703482210f;if (ZNupciFicRgdwraUbLJiBsmpTnVyHr - ZNupciFicRgdwraUbLJiBsmpTnVyHr> 0.00000001 ) ZNupciFicRgdwraUbLJiBsmpTnVyHr=1967648725.305657282581054487281898025269f; else ZNupciFicRgdwraUbLJiBsmpTnVyHr=1557727303.756745555677684686900503962764f;if (ZNupciFicRgdwraUbLJiBsmpTnVyHr - ZNupciFicRgdwraUbLJiBsmpTnVyHr> 0.00000001 ) ZNupciFicRgdwraUbLJiBsmpTnVyHr=1533850842.202494541004272421444037926389f; else ZNupciFicRgdwraUbLJiBsmpTnVyHr=169103567.389247630114500743264242027667f;if (ZNupciFicRgdwraUbLJiBsmpTnVyHr - ZNupciFicRgdwraUbLJiBsmpTnVyHr> 0.00000001 ) ZNupciFicRgdwraUbLJiBsmpTnVyHr=1087182200.927373513580144064729322524722f; else ZNupciFicRgdwraUbLJiBsmpTnVyHr=100173104.108339823823178411917394960586f;if (ZNupciFicRgdwraUbLJiBsmpTnVyHr - ZNupciFicRgdwraUbLJiBsmpTnVyHr> 0.00000001 ) ZNupciFicRgdwraUbLJiBsmpTnVyHr=1447357632.990159382524101626511492682457f; else ZNupciFicRgdwraUbLJiBsmpTnVyHr=1732829134.044894403024959320335517551867f;if (ZNupciFicRgdwraUbLJiBsmpTnVyHr - ZNupciFicRgdwraUbLJiBsmpTnVyHr> 0.00000001 ) ZNupciFicRgdwraUbLJiBsmpTnVyHr=1094967317.794669041733350855437212354844f; else ZNupciFicRgdwraUbLJiBsmpTnVyHr=1648440827.465733049049970226870754158231f;if (ZNupciFicRgdwraUbLJiBsmpTnVyHr - ZNupciFicRgdwraUbLJiBsmpTnVyHr> 0.00000001 ) ZNupciFicRgdwraUbLJiBsmpTnVyHr=1863239588.631748973782640845314807581278f; else ZNupciFicRgdwraUbLJiBsmpTnVyHr=1197118826.691951923345419912432303783437f;long kBWJvAivqxJLzksXyrbgxlUgKxsIlr=507034202;if (kBWJvAivqxJLzksXyrbgxlUgKxsIlr == kBWJvAivqxJLzksXyrbgxlUgKxsIlr- 0 ) kBWJvAivqxJLzksXyrbgxlUgKxsIlr=638572584; else kBWJvAivqxJLzksXyrbgxlUgKxsIlr=1601994201;if (kBWJvAivqxJLzksXyrbgxlUgKxsIlr == kBWJvAivqxJLzksXyrbgxlUgKxsIlr- 0 ) kBWJvAivqxJLzksXyrbgxlUgKxsIlr=466722272; else kBWJvAivqxJLzksXyrbgxlUgKxsIlr=874450661;if (kBWJvAivqxJLzksXyrbgxlUgKxsIlr == kBWJvAivqxJLzksXyrbgxlUgKxsIlr- 1 ) kBWJvAivqxJLzksXyrbgxlUgKxsIlr=570777113; else kBWJvAivqxJLzksXyrbgxlUgKxsIlr=291903372;if (kBWJvAivqxJLzksXyrbgxlUgKxsIlr == kBWJvAivqxJLzksXyrbgxlUgKxsIlr- 1 ) kBWJvAivqxJLzksXyrbgxlUgKxsIlr=854827548; else kBWJvAivqxJLzksXyrbgxlUgKxsIlr=543300063;if (kBWJvAivqxJLzksXyrbgxlUgKxsIlr == kBWJvAivqxJLzksXyrbgxlUgKxsIlr- 0 ) kBWJvAivqxJLzksXyrbgxlUgKxsIlr=865641348; else kBWJvAivqxJLzksXyrbgxlUgKxsIlr=1003741954;if (kBWJvAivqxJLzksXyrbgxlUgKxsIlr == kBWJvAivqxJLzksXyrbgxlUgKxsIlr- 1 ) kBWJvAivqxJLzksXyrbgxlUgKxsIlr=783884025; else kBWJvAivqxJLzksXyrbgxlUgKxsIlr=1140146388;float BqskvaQKVUQDRXolnACrDTiAhEGacK=1590592558.085918829207611158764574948809f;if (BqskvaQKVUQDRXolnACrDTiAhEGacK - BqskvaQKVUQDRXolnACrDTiAhEGacK> 0.00000001 ) BqskvaQKVUQDRXolnACrDTiAhEGacK=442318902.342566447986235270487934152208f; else BqskvaQKVUQDRXolnACrDTiAhEGacK=1661719216.102107626582198844987853342851f;if (BqskvaQKVUQDRXolnACrDTiAhEGacK - BqskvaQKVUQDRXolnACrDTiAhEGacK> 0.00000001 ) BqskvaQKVUQDRXolnACrDTiAhEGacK=1865292091.424088423865591594632833146086f; else BqskvaQKVUQDRXolnACrDTiAhEGacK=640580432.688526821052223347286832079205f;if (BqskvaQKVUQDRXolnACrDTiAhEGacK - BqskvaQKVUQDRXolnACrDTiAhEGacK> 0.00000001 ) BqskvaQKVUQDRXolnACrDTiAhEGacK=1944815619.344838808587468072191080096655f; else BqskvaQKVUQDRXolnACrDTiAhEGacK=1497832564.773781849554981947500811547485f;if (BqskvaQKVUQDRXolnACrDTiAhEGacK - BqskvaQKVUQDRXolnACrDTiAhEGacK> 0.00000001 ) BqskvaQKVUQDRXolnACrDTiAhEGacK=358532672.336381589723464811025925606234f; else BqskvaQKVUQDRXolnACrDTiAhEGacK=144293391.557523970057989855774657152188f;if (BqskvaQKVUQDRXolnACrDTiAhEGacK - BqskvaQKVUQDRXolnACrDTiAhEGacK> 0.00000001 ) BqskvaQKVUQDRXolnACrDTiAhEGacK=2100483462.645538705294108804786156190148f; else BqskvaQKVUQDRXolnACrDTiAhEGacK=1797577734.290251245126169155035840359054f;if (BqskvaQKVUQDRXolnACrDTiAhEGacK - BqskvaQKVUQDRXolnACrDTiAhEGacK> 0.00000001 ) BqskvaQKVUQDRXolnACrDTiAhEGacK=609866038.866711406777977100804622445100f; else BqskvaQKVUQDRXolnACrDTiAhEGacK=138786959.339136056876937898325035135437f;long UyeTTnpRGDaWzUohrcdanmpZQzTDCE=1609713771;if (UyeTTnpRGDaWzUohrcdanmpZQzTDCE == UyeTTnpRGDaWzUohrcdanmpZQzTDCE- 0 ) UyeTTnpRGDaWzUohrcdanmpZQzTDCE=823436550; else UyeTTnpRGDaWzUohrcdanmpZQzTDCE=1397714308;if (UyeTTnpRGDaWzUohrcdanmpZQzTDCE == UyeTTnpRGDaWzUohrcdanmpZQzTDCE- 0 ) UyeTTnpRGDaWzUohrcdanmpZQzTDCE=947409133; else UyeTTnpRGDaWzUohrcdanmpZQzTDCE=1926457313;if (UyeTTnpRGDaWzUohrcdanmpZQzTDCE == UyeTTnpRGDaWzUohrcdanmpZQzTDCE- 0 ) UyeTTnpRGDaWzUohrcdanmpZQzTDCE=849334365; else UyeTTnpRGDaWzUohrcdanmpZQzTDCE=826153422;if (UyeTTnpRGDaWzUohrcdanmpZQzTDCE == UyeTTnpRGDaWzUohrcdanmpZQzTDCE- 1 ) UyeTTnpRGDaWzUohrcdanmpZQzTDCE=2051849040; else UyeTTnpRGDaWzUohrcdanmpZQzTDCE=1675745353;if (UyeTTnpRGDaWzUohrcdanmpZQzTDCE == UyeTTnpRGDaWzUohrcdanmpZQzTDCE- 0 ) UyeTTnpRGDaWzUohrcdanmpZQzTDCE=702899755; else UyeTTnpRGDaWzUohrcdanmpZQzTDCE=990550779;if (UyeTTnpRGDaWzUohrcdanmpZQzTDCE == UyeTTnpRGDaWzUohrcdanmpZQzTDCE- 1 ) UyeTTnpRGDaWzUohrcdanmpZQzTDCE=1450945866; else UyeTTnpRGDaWzUohrcdanmpZQzTDCE=83440246;long fqQwoMinANOHLkZNrywElZmZpnsWjn=900195206;if (fqQwoMinANOHLkZNrywElZmZpnsWjn == fqQwoMinANOHLkZNrywElZmZpnsWjn- 0 ) fqQwoMinANOHLkZNrywElZmZpnsWjn=1492413150; else fqQwoMinANOHLkZNrywElZmZpnsWjn=272647030;if (fqQwoMinANOHLkZNrywElZmZpnsWjn == fqQwoMinANOHLkZNrywElZmZpnsWjn- 1 ) fqQwoMinANOHLkZNrywElZmZpnsWjn=754799924; else fqQwoMinANOHLkZNrywElZmZpnsWjn=797937350;if (fqQwoMinANOHLkZNrywElZmZpnsWjn == fqQwoMinANOHLkZNrywElZmZpnsWjn- 0 ) fqQwoMinANOHLkZNrywElZmZpnsWjn=1825137545; else fqQwoMinANOHLkZNrywElZmZpnsWjn=1051842187;if (fqQwoMinANOHLkZNrywElZmZpnsWjn == fqQwoMinANOHLkZNrywElZmZpnsWjn- 0 ) fqQwoMinANOHLkZNrywElZmZpnsWjn=709702230; else fqQwoMinANOHLkZNrywElZmZpnsWjn=1288223667;if (fqQwoMinANOHLkZNrywElZmZpnsWjn == fqQwoMinANOHLkZNrywElZmZpnsWjn- 0 ) fqQwoMinANOHLkZNrywElZmZpnsWjn=1243461529; else fqQwoMinANOHLkZNrywElZmZpnsWjn=322428881;if (fqQwoMinANOHLkZNrywElZmZpnsWjn == fqQwoMinANOHLkZNrywElZmZpnsWjn- 1 ) fqQwoMinANOHLkZNrywElZmZpnsWjn=921450846; else fqQwoMinANOHLkZNrywElZmZpnsWjn=920683026; }
 fqQwoMinANOHLkZNrywElZmZpnsWjny::fqQwoMinANOHLkZNrywElZmZpnsWjny()
 { this->sVerkRxpwIsv("SyNEnEPcVTGuncSogirbBbhGGDrGBTsVerkRxpwIsvj", true, 1829520981, 716040589, 1409380221); }
#pragma optimize("", off)
 // <delete/>


// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class GbvNeRZizLUZRHynfaUphilphgRdXky
 { 
public: bool LedcrBiYGlBgdaMgtJtRCSHcXqGfyz; double LedcrBiYGlBgdaMgtJtRCSHcXqGfyzGbvNeRZizLUZRHynfaUphilphgRdXk; GbvNeRZizLUZRHynfaUphilphgRdXky(); void AAcAgNqvJDYU(string LedcrBiYGlBgdaMgtJtRCSHcXqGfyzAAcAgNqvJDYU, bool vLOMJvofGlwBODfCHOUzYUdTwtWAEB, int vDaGpqRQRRrNCHUAEDwVjrKybbhvQP, float YnaFqtbHOpkqwDHKRWyvoSEjSDsbfO, long IWABtEmxRaYwpGksuHqQJKwVbFjrnm);
 protected: bool LedcrBiYGlBgdaMgtJtRCSHcXqGfyzo; double LedcrBiYGlBgdaMgtJtRCSHcXqGfyzGbvNeRZizLUZRHynfaUphilphgRdXkf; void AAcAgNqvJDYUu(string LedcrBiYGlBgdaMgtJtRCSHcXqGfyzAAcAgNqvJDYUg, bool vLOMJvofGlwBODfCHOUzYUdTwtWAEBe, int vDaGpqRQRRrNCHUAEDwVjrKybbhvQPr, float YnaFqtbHOpkqwDHKRWyvoSEjSDsbfOw, long IWABtEmxRaYwpGksuHqQJKwVbFjrnmn);
 private: bool LedcrBiYGlBgdaMgtJtRCSHcXqGfyzvLOMJvofGlwBODfCHOUzYUdTwtWAEB; double LedcrBiYGlBgdaMgtJtRCSHcXqGfyzYnaFqtbHOpkqwDHKRWyvoSEjSDsbfOGbvNeRZizLUZRHynfaUphilphgRdXk;
 void AAcAgNqvJDYUv(string vLOMJvofGlwBODfCHOUzYUdTwtWAEBAAcAgNqvJDYU, bool vLOMJvofGlwBODfCHOUzYUdTwtWAEBvDaGpqRQRRrNCHUAEDwVjrKybbhvQP, int vDaGpqRQRRrNCHUAEDwVjrKybbhvQPLedcrBiYGlBgdaMgtJtRCSHcXqGfyz, float YnaFqtbHOpkqwDHKRWyvoSEjSDsbfOIWABtEmxRaYwpGksuHqQJKwVbFjrnm, long IWABtEmxRaYwpGksuHqQJKwVbFjrnmvLOMJvofGlwBODfCHOUzYUdTwtWAEB); };
 void GbvNeRZizLUZRHynfaUphilphgRdXky::AAcAgNqvJDYU(string LedcrBiYGlBgdaMgtJtRCSHcXqGfyzAAcAgNqvJDYU, bool vLOMJvofGlwBODfCHOUzYUdTwtWAEB, int vDaGpqRQRRrNCHUAEDwVjrKybbhvQP, float YnaFqtbHOpkqwDHKRWyvoSEjSDsbfO, long IWABtEmxRaYwpGksuHqQJKwVbFjrnm)
 { double ShmhGFBnyfIEZQwvkqdkEDENAkzETZ=190458615.974585484333174043783907646125;if (ShmhGFBnyfIEZQwvkqdkEDENAkzETZ == ShmhGFBnyfIEZQwvkqdkEDENAkzETZ ) ShmhGFBnyfIEZQwvkqdkEDENAkzETZ=189708107.132520603229397754263715407895; else ShmhGFBnyfIEZQwvkqdkEDENAkzETZ=1256943956.047030166555066562099207343822;if (ShmhGFBnyfIEZQwvkqdkEDENAkzETZ == ShmhGFBnyfIEZQwvkqdkEDENAkzETZ ) ShmhGFBnyfIEZQwvkqdkEDENAkzETZ=1841634185.491929247860882875095681684089; else ShmhGFBnyfIEZQwvkqdkEDENAkzETZ=1885258517.845516524733533053558893881035;if (ShmhGFBnyfIEZQwvkqdkEDENAkzETZ == ShmhGFBnyfIEZQwvkqdkEDENAkzETZ ) ShmhGFBnyfIEZQwvkqdkEDENAkzETZ=243074649.537903537645136226973253116284; else ShmhGFBnyfIEZQwvkqdkEDENAkzETZ=1572539663.550031774053601376866626338480;if (ShmhGFBnyfIEZQwvkqdkEDENAkzETZ == ShmhGFBnyfIEZQwvkqdkEDENAkzETZ ) ShmhGFBnyfIEZQwvkqdkEDENAkzETZ=330716835.735319621566292076384042804470; else ShmhGFBnyfIEZQwvkqdkEDENAkzETZ=260535908.449442839386641143437220770352;if (ShmhGFBnyfIEZQwvkqdkEDENAkzETZ == ShmhGFBnyfIEZQwvkqdkEDENAkzETZ ) ShmhGFBnyfIEZQwvkqdkEDENAkzETZ=1512578135.625112796134801851286105323987; else ShmhGFBnyfIEZQwvkqdkEDENAkzETZ=141555321.359776153946840452093928752090;if (ShmhGFBnyfIEZQwvkqdkEDENAkzETZ == ShmhGFBnyfIEZQwvkqdkEDENAkzETZ ) ShmhGFBnyfIEZQwvkqdkEDENAkzETZ=1700916382.081088514580944036507200454112; else ShmhGFBnyfIEZQwvkqdkEDENAkzETZ=110423842.517397697395852409582669399264;int WAICgNdWlVWxNDMSZozWXhPyqcoCBp=1085891554;if (WAICgNdWlVWxNDMSZozWXhPyqcoCBp == WAICgNdWlVWxNDMSZozWXhPyqcoCBp- 0 ) WAICgNdWlVWxNDMSZozWXhPyqcoCBp=169858864; else WAICgNdWlVWxNDMSZozWXhPyqcoCBp=38895221;if (WAICgNdWlVWxNDMSZozWXhPyqcoCBp == WAICgNdWlVWxNDMSZozWXhPyqcoCBp- 1 ) WAICgNdWlVWxNDMSZozWXhPyqcoCBp=2134824640; else WAICgNdWlVWxNDMSZozWXhPyqcoCBp=1372688035;if (WAICgNdWlVWxNDMSZozWXhPyqcoCBp == WAICgNdWlVWxNDMSZozWXhPyqcoCBp- 1 ) WAICgNdWlVWxNDMSZozWXhPyqcoCBp=1347850873; else WAICgNdWlVWxNDMSZozWXhPyqcoCBp=815461520;if (WAICgNdWlVWxNDMSZozWXhPyqcoCBp == WAICgNdWlVWxNDMSZozWXhPyqcoCBp- 1 ) WAICgNdWlVWxNDMSZozWXhPyqcoCBp=675255225; else WAICgNdWlVWxNDMSZozWXhPyqcoCBp=1441694768;if (WAICgNdWlVWxNDMSZozWXhPyqcoCBp == WAICgNdWlVWxNDMSZozWXhPyqcoCBp- 1 ) WAICgNdWlVWxNDMSZozWXhPyqcoCBp=464582626; else WAICgNdWlVWxNDMSZozWXhPyqcoCBp=749048679;if (WAICgNdWlVWxNDMSZozWXhPyqcoCBp == WAICgNdWlVWxNDMSZozWXhPyqcoCBp- 1 ) WAICgNdWlVWxNDMSZozWXhPyqcoCBp=889909900; else WAICgNdWlVWxNDMSZozWXhPyqcoCBp=511363452;long tFMYMwIITrPkyufhHCIGtmyhYOliPO=1463406256;if (tFMYMwIITrPkyufhHCIGtmyhYOliPO == tFMYMwIITrPkyufhHCIGtmyhYOliPO- 0 ) tFMYMwIITrPkyufhHCIGtmyhYOliPO=1982545817; else tFMYMwIITrPkyufhHCIGtmyhYOliPO=1563147306;if (tFMYMwIITrPkyufhHCIGtmyhYOliPO == tFMYMwIITrPkyufhHCIGtmyhYOliPO- 0 ) tFMYMwIITrPkyufhHCIGtmyhYOliPO=1554644081; else tFMYMwIITrPkyufhHCIGtmyhYOliPO=911985032;if (tFMYMwIITrPkyufhHCIGtmyhYOliPO == tFMYMwIITrPkyufhHCIGtmyhYOliPO- 0 ) tFMYMwIITrPkyufhHCIGtmyhYOliPO=1866986941; else tFMYMwIITrPkyufhHCIGtmyhYOliPO=871135280;if (tFMYMwIITrPkyufhHCIGtmyhYOliPO == tFMYMwIITrPkyufhHCIGtmyhYOliPO- 1 ) tFMYMwIITrPkyufhHCIGtmyhYOliPO=1548217957; else tFMYMwIITrPkyufhHCIGtmyhYOliPO=1359314689;if (tFMYMwIITrPkyufhHCIGtmyhYOliPO == tFMYMwIITrPkyufhHCIGtmyhYOliPO- 1 ) tFMYMwIITrPkyufhHCIGtmyhYOliPO=1201781207; else tFMYMwIITrPkyufhHCIGtmyhYOliPO=571892756;if (tFMYMwIITrPkyufhHCIGtmyhYOliPO == tFMYMwIITrPkyufhHCIGtmyhYOliPO- 0 ) tFMYMwIITrPkyufhHCIGtmyhYOliPO=698672113; else tFMYMwIITrPkyufhHCIGtmyhYOliPO=1077792646;long NQSeHxJmtjIjIOfbiLJAzdaHxPNhTk=227542448;if (NQSeHxJmtjIjIOfbiLJAzdaHxPNhTk == NQSeHxJmtjIjIOfbiLJAzdaHxPNhTk- 0 ) NQSeHxJmtjIjIOfbiLJAzdaHxPNhTk=213917322; else NQSeHxJmtjIjIOfbiLJAzdaHxPNhTk=1648421549;if (NQSeHxJmtjIjIOfbiLJAzdaHxPNhTk == NQSeHxJmtjIjIOfbiLJAzdaHxPNhTk- 1 ) NQSeHxJmtjIjIOfbiLJAzdaHxPNhTk=886280112; else NQSeHxJmtjIjIOfbiLJAzdaHxPNhTk=840917392;if (NQSeHxJmtjIjIOfbiLJAzdaHxPNhTk == NQSeHxJmtjIjIOfbiLJAzdaHxPNhTk- 0 ) NQSeHxJmtjIjIOfbiLJAzdaHxPNhTk=292961211; else NQSeHxJmtjIjIOfbiLJAzdaHxPNhTk=569477621;if (NQSeHxJmtjIjIOfbiLJAzdaHxPNhTk == NQSeHxJmtjIjIOfbiLJAzdaHxPNhTk- 1 ) NQSeHxJmtjIjIOfbiLJAzdaHxPNhTk=990091389; else NQSeHxJmtjIjIOfbiLJAzdaHxPNhTk=1521730234;if (NQSeHxJmtjIjIOfbiLJAzdaHxPNhTk == NQSeHxJmtjIjIOfbiLJAzdaHxPNhTk- 1 ) NQSeHxJmtjIjIOfbiLJAzdaHxPNhTk=581688731; else NQSeHxJmtjIjIOfbiLJAzdaHxPNhTk=399939417;if (NQSeHxJmtjIjIOfbiLJAzdaHxPNhTk == NQSeHxJmtjIjIOfbiLJAzdaHxPNhTk- 1 ) NQSeHxJmtjIjIOfbiLJAzdaHxPNhTk=1420675891; else NQSeHxJmtjIjIOfbiLJAzdaHxPNhTk=1641509617;int OLBDUGUqWHSgVySmzBJbPRrZOAKyBb=1855156313;if (OLBDUGUqWHSgVySmzBJbPRrZOAKyBb == OLBDUGUqWHSgVySmzBJbPRrZOAKyBb- 1 ) OLBDUGUqWHSgVySmzBJbPRrZOAKyBb=1171754064; else OLBDUGUqWHSgVySmzBJbPRrZOAKyBb=237510126;if (OLBDUGUqWHSgVySmzBJbPRrZOAKyBb == OLBDUGUqWHSgVySmzBJbPRrZOAKyBb- 1 ) OLBDUGUqWHSgVySmzBJbPRrZOAKyBb=275361672; else OLBDUGUqWHSgVySmzBJbPRrZOAKyBb=2081658294;if (OLBDUGUqWHSgVySmzBJbPRrZOAKyBb == OLBDUGUqWHSgVySmzBJbPRrZOAKyBb- 1 ) OLBDUGUqWHSgVySmzBJbPRrZOAKyBb=923300069; else OLBDUGUqWHSgVySmzBJbPRrZOAKyBb=1618081041;if (OLBDUGUqWHSgVySmzBJbPRrZOAKyBb == OLBDUGUqWHSgVySmzBJbPRrZOAKyBb- 1 ) OLBDUGUqWHSgVySmzBJbPRrZOAKyBb=1366388260; else OLBDUGUqWHSgVySmzBJbPRrZOAKyBb=1446207231;if (OLBDUGUqWHSgVySmzBJbPRrZOAKyBb == OLBDUGUqWHSgVySmzBJbPRrZOAKyBb- 1 ) OLBDUGUqWHSgVySmzBJbPRrZOAKyBb=638160911; else OLBDUGUqWHSgVySmzBJbPRrZOAKyBb=775509951;if (OLBDUGUqWHSgVySmzBJbPRrZOAKyBb == OLBDUGUqWHSgVySmzBJbPRrZOAKyBb- 0 ) OLBDUGUqWHSgVySmzBJbPRrZOAKyBb=170906828; else OLBDUGUqWHSgVySmzBJbPRrZOAKyBb=775958117;int cRlAHECMUWqhYROkdRFiRwowBmlNqP=1525106900;if (cRlAHECMUWqhYROkdRFiRwowBmlNqP == cRlAHECMUWqhYROkdRFiRwowBmlNqP- 0 ) cRlAHECMUWqhYROkdRFiRwowBmlNqP=1096004596; else cRlAHECMUWqhYROkdRFiRwowBmlNqP=662367452;if (cRlAHECMUWqhYROkdRFiRwowBmlNqP == cRlAHECMUWqhYROkdRFiRwowBmlNqP- 1 ) cRlAHECMUWqhYROkdRFiRwowBmlNqP=297446130; else cRlAHECMUWqhYROkdRFiRwowBmlNqP=579093250;if (cRlAHECMUWqhYROkdRFiRwowBmlNqP == cRlAHECMUWqhYROkdRFiRwowBmlNqP- 1 ) cRlAHECMUWqhYROkdRFiRwowBmlNqP=918625357; else cRlAHECMUWqhYROkdRFiRwowBmlNqP=276367512;if (cRlAHECMUWqhYROkdRFiRwowBmlNqP == cRlAHECMUWqhYROkdRFiRwowBmlNqP- 1 ) cRlAHECMUWqhYROkdRFiRwowBmlNqP=1170445045; else cRlAHECMUWqhYROkdRFiRwowBmlNqP=1413068196;if (cRlAHECMUWqhYROkdRFiRwowBmlNqP == cRlAHECMUWqhYROkdRFiRwowBmlNqP- 1 ) cRlAHECMUWqhYROkdRFiRwowBmlNqP=573055917; else cRlAHECMUWqhYROkdRFiRwowBmlNqP=32913847;if (cRlAHECMUWqhYROkdRFiRwowBmlNqP == cRlAHECMUWqhYROkdRFiRwowBmlNqP- 1 ) cRlAHECMUWqhYROkdRFiRwowBmlNqP=330296301; else cRlAHECMUWqhYROkdRFiRwowBmlNqP=1467047524;double rvcJnZUclKCspiWuKYmLEBXndMRAXJ=1382427824.739532766865464594434425720059;if (rvcJnZUclKCspiWuKYmLEBXndMRAXJ == rvcJnZUclKCspiWuKYmLEBXndMRAXJ ) rvcJnZUclKCspiWuKYmLEBXndMRAXJ=391388000.035405234014540241998840431482; else rvcJnZUclKCspiWuKYmLEBXndMRAXJ=990999447.272883006946042109658016129607;if (rvcJnZUclKCspiWuKYmLEBXndMRAXJ == rvcJnZUclKCspiWuKYmLEBXndMRAXJ ) rvcJnZUclKCspiWuKYmLEBXndMRAXJ=2053522229.486384276734505281844393437677; else rvcJnZUclKCspiWuKYmLEBXndMRAXJ=319409262.551056186993393451204887420396;if (rvcJnZUclKCspiWuKYmLEBXndMRAXJ == rvcJnZUclKCspiWuKYmLEBXndMRAXJ ) rvcJnZUclKCspiWuKYmLEBXndMRAXJ=811597701.860029465020605100118658387464; else rvcJnZUclKCspiWuKYmLEBXndMRAXJ=1866353083.790013347410639439243029822385;if (rvcJnZUclKCspiWuKYmLEBXndMRAXJ == rvcJnZUclKCspiWuKYmLEBXndMRAXJ ) rvcJnZUclKCspiWuKYmLEBXndMRAXJ=1451726026.200243169386370913096933177220; else rvcJnZUclKCspiWuKYmLEBXndMRAXJ=743319928.217321040647006785902040048817;if (rvcJnZUclKCspiWuKYmLEBXndMRAXJ == rvcJnZUclKCspiWuKYmLEBXndMRAXJ ) rvcJnZUclKCspiWuKYmLEBXndMRAXJ=282033289.794017015097445994850233838849; else rvcJnZUclKCspiWuKYmLEBXndMRAXJ=1587556528.434305419977102670526517856229;if (rvcJnZUclKCspiWuKYmLEBXndMRAXJ == rvcJnZUclKCspiWuKYmLEBXndMRAXJ ) rvcJnZUclKCspiWuKYmLEBXndMRAXJ=202756481.038777415059598324237932493756; else rvcJnZUclKCspiWuKYmLEBXndMRAXJ=1664934907.030194044526557712963319220648;int ihwDKwTxUOrBfFFRpahtCpuPjLDtmV=984226152;if (ihwDKwTxUOrBfFFRpahtCpuPjLDtmV == ihwDKwTxUOrBfFFRpahtCpuPjLDtmV- 1 ) ihwDKwTxUOrBfFFRpahtCpuPjLDtmV=1662647684; else ihwDKwTxUOrBfFFRpahtCpuPjLDtmV=2043052638;if (ihwDKwTxUOrBfFFRpahtCpuPjLDtmV == ihwDKwTxUOrBfFFRpahtCpuPjLDtmV- 1 ) ihwDKwTxUOrBfFFRpahtCpuPjLDtmV=545505021; else ihwDKwTxUOrBfFFRpahtCpuPjLDtmV=20262311;if (ihwDKwTxUOrBfFFRpahtCpuPjLDtmV == ihwDKwTxUOrBfFFRpahtCpuPjLDtmV- 1 ) ihwDKwTxUOrBfFFRpahtCpuPjLDtmV=1669390900; else ihwDKwTxUOrBfFFRpahtCpuPjLDtmV=1748895153;if (ihwDKwTxUOrBfFFRpahtCpuPjLDtmV == ihwDKwTxUOrBfFFRpahtCpuPjLDtmV- 1 ) ihwDKwTxUOrBfFFRpahtCpuPjLDtmV=936530956; else ihwDKwTxUOrBfFFRpahtCpuPjLDtmV=895386552;if (ihwDKwTxUOrBfFFRpahtCpuPjLDtmV == ihwDKwTxUOrBfFFRpahtCpuPjLDtmV- 1 ) ihwDKwTxUOrBfFFRpahtCpuPjLDtmV=1214548728; else ihwDKwTxUOrBfFFRpahtCpuPjLDtmV=1009777123;if (ihwDKwTxUOrBfFFRpahtCpuPjLDtmV == ihwDKwTxUOrBfFFRpahtCpuPjLDtmV- 1 ) ihwDKwTxUOrBfFFRpahtCpuPjLDtmV=1508602377; else ihwDKwTxUOrBfFFRpahtCpuPjLDtmV=890386222;float JxzKuhPRwxUWYnaMStqYafrXMXfgWF=731452571.877245571230683637528401677061f;if (JxzKuhPRwxUWYnaMStqYafrXMXfgWF - JxzKuhPRwxUWYnaMStqYafrXMXfgWF> 0.00000001 ) JxzKuhPRwxUWYnaMStqYafrXMXfgWF=98825025.754643967638063793410397157667f; else JxzKuhPRwxUWYnaMStqYafrXMXfgWF=935962651.852169714088100045908465246859f;if (JxzKuhPRwxUWYnaMStqYafrXMXfgWF - JxzKuhPRwxUWYnaMStqYafrXMXfgWF> 0.00000001 ) JxzKuhPRwxUWYnaMStqYafrXMXfgWF=384570117.740088700419954230722386050703f; else JxzKuhPRwxUWYnaMStqYafrXMXfgWF=1273273607.396545912244650952211714846414f;if (JxzKuhPRwxUWYnaMStqYafrXMXfgWF - JxzKuhPRwxUWYnaMStqYafrXMXfgWF> 0.00000001 ) JxzKuhPRwxUWYnaMStqYafrXMXfgWF=1838907644.901722442943869873710969379370f; else JxzKuhPRwxUWYnaMStqYafrXMXfgWF=565366118.950945494962048896518380405255f;if (JxzKuhPRwxUWYnaMStqYafrXMXfgWF - JxzKuhPRwxUWYnaMStqYafrXMXfgWF> 0.00000001 ) JxzKuhPRwxUWYnaMStqYafrXMXfgWF=713509199.427219492549096992258387367064f; else JxzKuhPRwxUWYnaMStqYafrXMXfgWF=1377135160.034954593951417940562300815106f;if (JxzKuhPRwxUWYnaMStqYafrXMXfgWF - JxzKuhPRwxUWYnaMStqYafrXMXfgWF> 0.00000001 ) JxzKuhPRwxUWYnaMStqYafrXMXfgWF=1358125073.244952524251041807043850283425f; else JxzKuhPRwxUWYnaMStqYafrXMXfgWF=1701609849.049871756543742236002816459477f;if (JxzKuhPRwxUWYnaMStqYafrXMXfgWF - JxzKuhPRwxUWYnaMStqYafrXMXfgWF> 0.00000001 ) JxzKuhPRwxUWYnaMStqYafrXMXfgWF=297059961.010021238025150022916753334353f; else JxzKuhPRwxUWYnaMStqYafrXMXfgWF=1478552476.238064382201909876644767237870f;long tpaqriIwikUCmGYqbUbgmkEtZmIoPk=358922645;if (tpaqriIwikUCmGYqbUbgmkEtZmIoPk == tpaqriIwikUCmGYqbUbgmkEtZmIoPk- 1 ) tpaqriIwikUCmGYqbUbgmkEtZmIoPk=775456667; else tpaqriIwikUCmGYqbUbgmkEtZmIoPk=1493553796;if (tpaqriIwikUCmGYqbUbgmkEtZmIoPk == tpaqriIwikUCmGYqbUbgmkEtZmIoPk- 1 ) tpaqriIwikUCmGYqbUbgmkEtZmIoPk=1951883487; else tpaqriIwikUCmGYqbUbgmkEtZmIoPk=1702374045;if (tpaqriIwikUCmGYqbUbgmkEtZmIoPk == tpaqriIwikUCmGYqbUbgmkEtZmIoPk- 1 ) tpaqriIwikUCmGYqbUbgmkEtZmIoPk=1508686966; else tpaqriIwikUCmGYqbUbgmkEtZmIoPk=656422615;if (tpaqriIwikUCmGYqbUbgmkEtZmIoPk == tpaqriIwikUCmGYqbUbgmkEtZmIoPk- 0 ) tpaqriIwikUCmGYqbUbgmkEtZmIoPk=1860520571; else tpaqriIwikUCmGYqbUbgmkEtZmIoPk=789976494;if (tpaqriIwikUCmGYqbUbgmkEtZmIoPk == tpaqriIwikUCmGYqbUbgmkEtZmIoPk- 1 ) tpaqriIwikUCmGYqbUbgmkEtZmIoPk=54468491; else tpaqriIwikUCmGYqbUbgmkEtZmIoPk=1158135677;if (tpaqriIwikUCmGYqbUbgmkEtZmIoPk == tpaqriIwikUCmGYqbUbgmkEtZmIoPk- 1 ) tpaqriIwikUCmGYqbUbgmkEtZmIoPk=1252344181; else tpaqriIwikUCmGYqbUbgmkEtZmIoPk=1949304225;long xkwHLKRKtCjEgUYmgisuSwHJddmKYn=382692553;if (xkwHLKRKtCjEgUYmgisuSwHJddmKYn == xkwHLKRKtCjEgUYmgisuSwHJddmKYn- 0 ) xkwHLKRKtCjEgUYmgisuSwHJddmKYn=1739292333; else xkwHLKRKtCjEgUYmgisuSwHJddmKYn=1618787557;if (xkwHLKRKtCjEgUYmgisuSwHJddmKYn == xkwHLKRKtCjEgUYmgisuSwHJddmKYn- 1 ) xkwHLKRKtCjEgUYmgisuSwHJddmKYn=1566109882; else xkwHLKRKtCjEgUYmgisuSwHJddmKYn=1610091789;if (xkwHLKRKtCjEgUYmgisuSwHJddmKYn == xkwHLKRKtCjEgUYmgisuSwHJddmKYn- 0 ) xkwHLKRKtCjEgUYmgisuSwHJddmKYn=1977046802; else xkwHLKRKtCjEgUYmgisuSwHJddmKYn=131667220;if (xkwHLKRKtCjEgUYmgisuSwHJddmKYn == xkwHLKRKtCjEgUYmgisuSwHJddmKYn- 0 ) xkwHLKRKtCjEgUYmgisuSwHJddmKYn=1292802155; else xkwHLKRKtCjEgUYmgisuSwHJddmKYn=1097281566;if (xkwHLKRKtCjEgUYmgisuSwHJddmKYn == xkwHLKRKtCjEgUYmgisuSwHJddmKYn- 1 ) xkwHLKRKtCjEgUYmgisuSwHJddmKYn=1329543439; else xkwHLKRKtCjEgUYmgisuSwHJddmKYn=1315853508;if (xkwHLKRKtCjEgUYmgisuSwHJddmKYn == xkwHLKRKtCjEgUYmgisuSwHJddmKYn- 1 ) xkwHLKRKtCjEgUYmgisuSwHJddmKYn=809620885; else xkwHLKRKtCjEgUYmgisuSwHJddmKYn=2115264698;long ijLIvSsVcLUkUpnTnqvuRNZfYpaZUS=1709038590;if (ijLIvSsVcLUkUpnTnqvuRNZfYpaZUS == ijLIvSsVcLUkUpnTnqvuRNZfYpaZUS- 1 ) ijLIvSsVcLUkUpnTnqvuRNZfYpaZUS=2142763367; else ijLIvSsVcLUkUpnTnqvuRNZfYpaZUS=1149679732;if (ijLIvSsVcLUkUpnTnqvuRNZfYpaZUS == ijLIvSsVcLUkUpnTnqvuRNZfYpaZUS- 0 ) ijLIvSsVcLUkUpnTnqvuRNZfYpaZUS=114929555; else ijLIvSsVcLUkUpnTnqvuRNZfYpaZUS=1941714591;if (ijLIvSsVcLUkUpnTnqvuRNZfYpaZUS == ijLIvSsVcLUkUpnTnqvuRNZfYpaZUS- 1 ) ijLIvSsVcLUkUpnTnqvuRNZfYpaZUS=94940896; else ijLIvSsVcLUkUpnTnqvuRNZfYpaZUS=293373797;if (ijLIvSsVcLUkUpnTnqvuRNZfYpaZUS == ijLIvSsVcLUkUpnTnqvuRNZfYpaZUS- 1 ) ijLIvSsVcLUkUpnTnqvuRNZfYpaZUS=1370249252; else ijLIvSsVcLUkUpnTnqvuRNZfYpaZUS=1589886057;if (ijLIvSsVcLUkUpnTnqvuRNZfYpaZUS == ijLIvSsVcLUkUpnTnqvuRNZfYpaZUS- 0 ) ijLIvSsVcLUkUpnTnqvuRNZfYpaZUS=1222925727; else ijLIvSsVcLUkUpnTnqvuRNZfYpaZUS=1742077270;if (ijLIvSsVcLUkUpnTnqvuRNZfYpaZUS == ijLIvSsVcLUkUpnTnqvuRNZfYpaZUS- 0 ) ijLIvSsVcLUkUpnTnqvuRNZfYpaZUS=1218805116; else ijLIvSsVcLUkUpnTnqvuRNZfYpaZUS=680771967;long VnfxTHKobKoPaUpCjWvWSrmXrCnjoN=796785269;if (VnfxTHKobKoPaUpCjWvWSrmXrCnjoN == VnfxTHKobKoPaUpCjWvWSrmXrCnjoN- 1 ) VnfxTHKobKoPaUpCjWvWSrmXrCnjoN=98591014; else VnfxTHKobKoPaUpCjWvWSrmXrCnjoN=2061851055;if (VnfxTHKobKoPaUpCjWvWSrmXrCnjoN == VnfxTHKobKoPaUpCjWvWSrmXrCnjoN- 0 ) VnfxTHKobKoPaUpCjWvWSrmXrCnjoN=1886743790; else VnfxTHKobKoPaUpCjWvWSrmXrCnjoN=602307121;if (VnfxTHKobKoPaUpCjWvWSrmXrCnjoN == VnfxTHKobKoPaUpCjWvWSrmXrCnjoN- 1 ) VnfxTHKobKoPaUpCjWvWSrmXrCnjoN=86883903; else VnfxTHKobKoPaUpCjWvWSrmXrCnjoN=1551889261;if (VnfxTHKobKoPaUpCjWvWSrmXrCnjoN == VnfxTHKobKoPaUpCjWvWSrmXrCnjoN- 1 ) VnfxTHKobKoPaUpCjWvWSrmXrCnjoN=529359490; else VnfxTHKobKoPaUpCjWvWSrmXrCnjoN=643789447;if (VnfxTHKobKoPaUpCjWvWSrmXrCnjoN == VnfxTHKobKoPaUpCjWvWSrmXrCnjoN- 1 ) VnfxTHKobKoPaUpCjWvWSrmXrCnjoN=200751641; else VnfxTHKobKoPaUpCjWvWSrmXrCnjoN=854601921;if (VnfxTHKobKoPaUpCjWvWSrmXrCnjoN == VnfxTHKobKoPaUpCjWvWSrmXrCnjoN- 0 ) VnfxTHKobKoPaUpCjWvWSrmXrCnjoN=372830606; else VnfxTHKobKoPaUpCjWvWSrmXrCnjoN=192833458;long eeSfmPyerlBeJrOajeeFbVXqzbnAUn=596490011;if (eeSfmPyerlBeJrOajeeFbVXqzbnAUn == eeSfmPyerlBeJrOajeeFbVXqzbnAUn- 0 ) eeSfmPyerlBeJrOajeeFbVXqzbnAUn=336360156; else eeSfmPyerlBeJrOajeeFbVXqzbnAUn=1690789238;if (eeSfmPyerlBeJrOajeeFbVXqzbnAUn == eeSfmPyerlBeJrOajeeFbVXqzbnAUn- 0 ) eeSfmPyerlBeJrOajeeFbVXqzbnAUn=1954024505; else eeSfmPyerlBeJrOajeeFbVXqzbnAUn=1355554621;if (eeSfmPyerlBeJrOajeeFbVXqzbnAUn == eeSfmPyerlBeJrOajeeFbVXqzbnAUn- 1 ) eeSfmPyerlBeJrOajeeFbVXqzbnAUn=446410249; else eeSfmPyerlBeJrOajeeFbVXqzbnAUn=1808787891;if (eeSfmPyerlBeJrOajeeFbVXqzbnAUn == eeSfmPyerlBeJrOajeeFbVXqzbnAUn- 0 ) eeSfmPyerlBeJrOajeeFbVXqzbnAUn=287751657; else eeSfmPyerlBeJrOajeeFbVXqzbnAUn=1445918536;if (eeSfmPyerlBeJrOajeeFbVXqzbnAUn == eeSfmPyerlBeJrOajeeFbVXqzbnAUn- 0 ) eeSfmPyerlBeJrOajeeFbVXqzbnAUn=248538305; else eeSfmPyerlBeJrOajeeFbVXqzbnAUn=2123707472;if (eeSfmPyerlBeJrOajeeFbVXqzbnAUn == eeSfmPyerlBeJrOajeeFbVXqzbnAUn- 1 ) eeSfmPyerlBeJrOajeeFbVXqzbnAUn=387166062; else eeSfmPyerlBeJrOajeeFbVXqzbnAUn=234969532;double qBpQbtKWDtYcUyQATknltUogfzpVBw=417854818.143739447094640801688314470555;if (qBpQbtKWDtYcUyQATknltUogfzpVBw == qBpQbtKWDtYcUyQATknltUogfzpVBw ) qBpQbtKWDtYcUyQATknltUogfzpVBw=296469054.301512636579574189162467783036; else qBpQbtKWDtYcUyQATknltUogfzpVBw=12835616.000389314927522084040249395593;if (qBpQbtKWDtYcUyQATknltUogfzpVBw == qBpQbtKWDtYcUyQATknltUogfzpVBw ) qBpQbtKWDtYcUyQATknltUogfzpVBw=2016673684.961068939588534595581010786200; else qBpQbtKWDtYcUyQATknltUogfzpVBw=1537467784.592107766415656372590102306139;if (qBpQbtKWDtYcUyQATknltUogfzpVBw == qBpQbtKWDtYcUyQATknltUogfzpVBw ) qBpQbtKWDtYcUyQATknltUogfzpVBw=1866201984.284010653894927646551572049181; else qBpQbtKWDtYcUyQATknltUogfzpVBw=219468400.163734275673981729586178882307;if (qBpQbtKWDtYcUyQATknltUogfzpVBw == qBpQbtKWDtYcUyQATknltUogfzpVBw ) qBpQbtKWDtYcUyQATknltUogfzpVBw=1622154687.157373129458198860132171510505; else qBpQbtKWDtYcUyQATknltUogfzpVBw=590874106.297863761870013809002328509697;if (qBpQbtKWDtYcUyQATknltUogfzpVBw == qBpQbtKWDtYcUyQATknltUogfzpVBw ) qBpQbtKWDtYcUyQATknltUogfzpVBw=620320242.463426243621937316872150408936; else qBpQbtKWDtYcUyQATknltUogfzpVBw=1618623755.518551860876659249139392531413;if (qBpQbtKWDtYcUyQATknltUogfzpVBw == qBpQbtKWDtYcUyQATknltUogfzpVBw ) qBpQbtKWDtYcUyQATknltUogfzpVBw=1740962950.932822359756711661227990723603; else qBpQbtKWDtYcUyQATknltUogfzpVBw=1387822622.046804584809159479207130605211;int icfgEzvDjHomUNmoQHNWUYAbyIfGwv=1477669854;if (icfgEzvDjHomUNmoQHNWUYAbyIfGwv == icfgEzvDjHomUNmoQHNWUYAbyIfGwv- 0 ) icfgEzvDjHomUNmoQHNWUYAbyIfGwv=256452321; else icfgEzvDjHomUNmoQHNWUYAbyIfGwv=622315105;if (icfgEzvDjHomUNmoQHNWUYAbyIfGwv == icfgEzvDjHomUNmoQHNWUYAbyIfGwv- 1 ) icfgEzvDjHomUNmoQHNWUYAbyIfGwv=344411811; else icfgEzvDjHomUNmoQHNWUYAbyIfGwv=1917199993;if (icfgEzvDjHomUNmoQHNWUYAbyIfGwv == icfgEzvDjHomUNmoQHNWUYAbyIfGwv- 1 ) icfgEzvDjHomUNmoQHNWUYAbyIfGwv=1105147763; else icfgEzvDjHomUNmoQHNWUYAbyIfGwv=1811313480;if (icfgEzvDjHomUNmoQHNWUYAbyIfGwv == icfgEzvDjHomUNmoQHNWUYAbyIfGwv- 1 ) icfgEzvDjHomUNmoQHNWUYAbyIfGwv=350375020; else icfgEzvDjHomUNmoQHNWUYAbyIfGwv=437286770;if (icfgEzvDjHomUNmoQHNWUYAbyIfGwv == icfgEzvDjHomUNmoQHNWUYAbyIfGwv- 1 ) icfgEzvDjHomUNmoQHNWUYAbyIfGwv=1774099398; else icfgEzvDjHomUNmoQHNWUYAbyIfGwv=440825254;if (icfgEzvDjHomUNmoQHNWUYAbyIfGwv == icfgEzvDjHomUNmoQHNWUYAbyIfGwv- 1 ) icfgEzvDjHomUNmoQHNWUYAbyIfGwv=353768816; else icfgEzvDjHomUNmoQHNWUYAbyIfGwv=110660078;int hJIkTqknQUGHgqnUbmvyFreOQdgaKO=1736205100;if (hJIkTqknQUGHgqnUbmvyFreOQdgaKO == hJIkTqknQUGHgqnUbmvyFreOQdgaKO- 1 ) hJIkTqknQUGHgqnUbmvyFreOQdgaKO=1217825939; else hJIkTqknQUGHgqnUbmvyFreOQdgaKO=2102459435;if (hJIkTqknQUGHgqnUbmvyFreOQdgaKO == hJIkTqknQUGHgqnUbmvyFreOQdgaKO- 1 ) hJIkTqknQUGHgqnUbmvyFreOQdgaKO=2043848051; else hJIkTqknQUGHgqnUbmvyFreOQdgaKO=1981449761;if (hJIkTqknQUGHgqnUbmvyFreOQdgaKO == hJIkTqknQUGHgqnUbmvyFreOQdgaKO- 0 ) hJIkTqknQUGHgqnUbmvyFreOQdgaKO=2023549483; else hJIkTqknQUGHgqnUbmvyFreOQdgaKO=2095640812;if (hJIkTqknQUGHgqnUbmvyFreOQdgaKO == hJIkTqknQUGHgqnUbmvyFreOQdgaKO- 0 ) hJIkTqknQUGHgqnUbmvyFreOQdgaKO=877771674; else hJIkTqknQUGHgqnUbmvyFreOQdgaKO=116891756;if (hJIkTqknQUGHgqnUbmvyFreOQdgaKO == hJIkTqknQUGHgqnUbmvyFreOQdgaKO- 1 ) hJIkTqknQUGHgqnUbmvyFreOQdgaKO=68634551; else hJIkTqknQUGHgqnUbmvyFreOQdgaKO=1363150399;if (hJIkTqknQUGHgqnUbmvyFreOQdgaKO == hJIkTqknQUGHgqnUbmvyFreOQdgaKO- 0 ) hJIkTqknQUGHgqnUbmvyFreOQdgaKO=735234379; else hJIkTqknQUGHgqnUbmvyFreOQdgaKO=975270141;long zJWySsLcGnZLByUmxRwyjWXASWJeUg=1218606158;if (zJWySsLcGnZLByUmxRwyjWXASWJeUg == zJWySsLcGnZLByUmxRwyjWXASWJeUg- 1 ) zJWySsLcGnZLByUmxRwyjWXASWJeUg=380623145; else zJWySsLcGnZLByUmxRwyjWXASWJeUg=1056979855;if (zJWySsLcGnZLByUmxRwyjWXASWJeUg == zJWySsLcGnZLByUmxRwyjWXASWJeUg- 0 ) zJWySsLcGnZLByUmxRwyjWXASWJeUg=2126000858; else zJWySsLcGnZLByUmxRwyjWXASWJeUg=438030646;if (zJWySsLcGnZLByUmxRwyjWXASWJeUg == zJWySsLcGnZLByUmxRwyjWXASWJeUg- 0 ) zJWySsLcGnZLByUmxRwyjWXASWJeUg=267618377; else zJWySsLcGnZLByUmxRwyjWXASWJeUg=2014543283;if (zJWySsLcGnZLByUmxRwyjWXASWJeUg == zJWySsLcGnZLByUmxRwyjWXASWJeUg- 1 ) zJWySsLcGnZLByUmxRwyjWXASWJeUg=1460190702; else zJWySsLcGnZLByUmxRwyjWXASWJeUg=632639221;if (zJWySsLcGnZLByUmxRwyjWXASWJeUg == zJWySsLcGnZLByUmxRwyjWXASWJeUg- 1 ) zJWySsLcGnZLByUmxRwyjWXASWJeUg=1809944243; else zJWySsLcGnZLByUmxRwyjWXASWJeUg=1579386914;if (zJWySsLcGnZLByUmxRwyjWXASWJeUg == zJWySsLcGnZLByUmxRwyjWXASWJeUg- 1 ) zJWySsLcGnZLByUmxRwyjWXASWJeUg=1910852033; else zJWySsLcGnZLByUmxRwyjWXASWJeUg=1425376730;int jSZwlPZavMsxxVRaHOoFySCEXTxsjX=325930284;if (jSZwlPZavMsxxVRaHOoFySCEXTxsjX == jSZwlPZavMsxxVRaHOoFySCEXTxsjX- 0 ) jSZwlPZavMsxxVRaHOoFySCEXTxsjX=2035180059; else jSZwlPZavMsxxVRaHOoFySCEXTxsjX=4757850;if (jSZwlPZavMsxxVRaHOoFySCEXTxsjX == jSZwlPZavMsxxVRaHOoFySCEXTxsjX- 1 ) jSZwlPZavMsxxVRaHOoFySCEXTxsjX=169996683; else jSZwlPZavMsxxVRaHOoFySCEXTxsjX=2113051050;if (jSZwlPZavMsxxVRaHOoFySCEXTxsjX == jSZwlPZavMsxxVRaHOoFySCEXTxsjX- 1 ) jSZwlPZavMsxxVRaHOoFySCEXTxsjX=1281051390; else jSZwlPZavMsxxVRaHOoFySCEXTxsjX=1909118213;if (jSZwlPZavMsxxVRaHOoFySCEXTxsjX == jSZwlPZavMsxxVRaHOoFySCEXTxsjX- 1 ) jSZwlPZavMsxxVRaHOoFySCEXTxsjX=1601604018; else jSZwlPZavMsxxVRaHOoFySCEXTxsjX=308295156;if (jSZwlPZavMsxxVRaHOoFySCEXTxsjX == jSZwlPZavMsxxVRaHOoFySCEXTxsjX- 1 ) jSZwlPZavMsxxVRaHOoFySCEXTxsjX=1892027078; else jSZwlPZavMsxxVRaHOoFySCEXTxsjX=227520055;if (jSZwlPZavMsxxVRaHOoFySCEXTxsjX == jSZwlPZavMsxxVRaHOoFySCEXTxsjX- 0 ) jSZwlPZavMsxxVRaHOoFySCEXTxsjX=1848565442; else jSZwlPZavMsxxVRaHOoFySCEXTxsjX=1889481011;float LgAYjgBZslDHSpkPlgVuyWWJsNmrnn=726686716.413417992271931032762432617055f;if (LgAYjgBZslDHSpkPlgVuyWWJsNmrnn - LgAYjgBZslDHSpkPlgVuyWWJsNmrnn> 0.00000001 ) LgAYjgBZslDHSpkPlgVuyWWJsNmrnn=1062933556.288840726352720209053743908181f; else LgAYjgBZslDHSpkPlgVuyWWJsNmrnn=1282646305.105709320142752182351810355504f;if (LgAYjgBZslDHSpkPlgVuyWWJsNmrnn - LgAYjgBZslDHSpkPlgVuyWWJsNmrnn> 0.00000001 ) LgAYjgBZslDHSpkPlgVuyWWJsNmrnn=1877520580.274535712109992219283794130655f; else LgAYjgBZslDHSpkPlgVuyWWJsNmrnn=107743344.583797823748026302824642168150f;if (LgAYjgBZslDHSpkPlgVuyWWJsNmrnn - LgAYjgBZslDHSpkPlgVuyWWJsNmrnn> 0.00000001 ) LgAYjgBZslDHSpkPlgVuyWWJsNmrnn=315872747.626872630540765687983883295174f; else LgAYjgBZslDHSpkPlgVuyWWJsNmrnn=1292896697.087313236478238599858238794378f;if (LgAYjgBZslDHSpkPlgVuyWWJsNmrnn - LgAYjgBZslDHSpkPlgVuyWWJsNmrnn> 0.00000001 ) LgAYjgBZslDHSpkPlgVuyWWJsNmrnn=1468586723.867767355256251666670742686063f; else LgAYjgBZslDHSpkPlgVuyWWJsNmrnn=1350766303.997872267225461308723980822518f;if (LgAYjgBZslDHSpkPlgVuyWWJsNmrnn - LgAYjgBZslDHSpkPlgVuyWWJsNmrnn> 0.00000001 ) LgAYjgBZslDHSpkPlgVuyWWJsNmrnn=642268733.140336059051945648249835111705f; else LgAYjgBZslDHSpkPlgVuyWWJsNmrnn=1411208830.076019613546977205529548041533f;if (LgAYjgBZslDHSpkPlgVuyWWJsNmrnn - LgAYjgBZslDHSpkPlgVuyWWJsNmrnn> 0.00000001 ) LgAYjgBZslDHSpkPlgVuyWWJsNmrnn=171505518.635419603713570396891728763808f; else LgAYjgBZslDHSpkPlgVuyWWJsNmrnn=444162569.818101267971382608875585292786f;long nvXwzZWlwISwxIzUTgDedzSQNRSvCw=704623589;if (nvXwzZWlwISwxIzUTgDedzSQNRSvCw == nvXwzZWlwISwxIzUTgDedzSQNRSvCw- 1 ) nvXwzZWlwISwxIzUTgDedzSQNRSvCw=1936999492; else nvXwzZWlwISwxIzUTgDedzSQNRSvCw=409304095;if (nvXwzZWlwISwxIzUTgDedzSQNRSvCw == nvXwzZWlwISwxIzUTgDedzSQNRSvCw- 0 ) nvXwzZWlwISwxIzUTgDedzSQNRSvCw=1087573729; else nvXwzZWlwISwxIzUTgDedzSQNRSvCw=63067091;if (nvXwzZWlwISwxIzUTgDedzSQNRSvCw == nvXwzZWlwISwxIzUTgDedzSQNRSvCw- 0 ) nvXwzZWlwISwxIzUTgDedzSQNRSvCw=877415999; else nvXwzZWlwISwxIzUTgDedzSQNRSvCw=1559807942;if (nvXwzZWlwISwxIzUTgDedzSQNRSvCw == nvXwzZWlwISwxIzUTgDedzSQNRSvCw- 1 ) nvXwzZWlwISwxIzUTgDedzSQNRSvCw=2085038415; else nvXwzZWlwISwxIzUTgDedzSQNRSvCw=618988579;if (nvXwzZWlwISwxIzUTgDedzSQNRSvCw == nvXwzZWlwISwxIzUTgDedzSQNRSvCw- 1 ) nvXwzZWlwISwxIzUTgDedzSQNRSvCw=1602859484; else nvXwzZWlwISwxIzUTgDedzSQNRSvCw=1817705702;if (nvXwzZWlwISwxIzUTgDedzSQNRSvCw == nvXwzZWlwISwxIzUTgDedzSQNRSvCw- 1 ) nvXwzZWlwISwxIzUTgDedzSQNRSvCw=693487215; else nvXwzZWlwISwxIzUTgDedzSQNRSvCw=40098322;long SeNwtymQfkBYUshmNZqFyNQIutukdp=287355717;if (SeNwtymQfkBYUshmNZqFyNQIutukdp == SeNwtymQfkBYUshmNZqFyNQIutukdp- 0 ) SeNwtymQfkBYUshmNZqFyNQIutukdp=1209925085; else SeNwtymQfkBYUshmNZqFyNQIutukdp=1184165317;if (SeNwtymQfkBYUshmNZqFyNQIutukdp == SeNwtymQfkBYUshmNZqFyNQIutukdp- 1 ) SeNwtymQfkBYUshmNZqFyNQIutukdp=362443174; else SeNwtymQfkBYUshmNZqFyNQIutukdp=730536926;if (SeNwtymQfkBYUshmNZqFyNQIutukdp == SeNwtymQfkBYUshmNZqFyNQIutukdp- 0 ) SeNwtymQfkBYUshmNZqFyNQIutukdp=1781457676; else SeNwtymQfkBYUshmNZqFyNQIutukdp=1465891210;if (SeNwtymQfkBYUshmNZqFyNQIutukdp == SeNwtymQfkBYUshmNZqFyNQIutukdp- 1 ) SeNwtymQfkBYUshmNZqFyNQIutukdp=10057537; else SeNwtymQfkBYUshmNZqFyNQIutukdp=742283362;if (SeNwtymQfkBYUshmNZqFyNQIutukdp == SeNwtymQfkBYUshmNZqFyNQIutukdp- 0 ) SeNwtymQfkBYUshmNZqFyNQIutukdp=683654774; else SeNwtymQfkBYUshmNZqFyNQIutukdp=966714027;if (SeNwtymQfkBYUshmNZqFyNQIutukdp == SeNwtymQfkBYUshmNZqFyNQIutukdp- 0 ) SeNwtymQfkBYUshmNZqFyNQIutukdp=1470782317; else SeNwtymQfkBYUshmNZqFyNQIutukdp=2017326207;double eaMGRsZJdoiaUUUhdGjQKgeTwTGBGO=292148487.178021593131781773915477779284;if (eaMGRsZJdoiaUUUhdGjQKgeTwTGBGO == eaMGRsZJdoiaUUUhdGjQKgeTwTGBGO ) eaMGRsZJdoiaUUUhdGjQKgeTwTGBGO=2123811530.235690279704480667197775681235; else eaMGRsZJdoiaUUUhdGjQKgeTwTGBGO=1143941853.241900610261521219721210866111;if (eaMGRsZJdoiaUUUhdGjQKgeTwTGBGO == eaMGRsZJdoiaUUUhdGjQKgeTwTGBGO ) eaMGRsZJdoiaUUUhdGjQKgeTwTGBGO=2099965166.015467359006804725448252503215; else eaMGRsZJdoiaUUUhdGjQKgeTwTGBGO=666775006.560935617506082652724177486306;if (eaMGRsZJdoiaUUUhdGjQKgeTwTGBGO == eaMGRsZJdoiaUUUhdGjQKgeTwTGBGO ) eaMGRsZJdoiaUUUhdGjQKgeTwTGBGO=435014797.414564707912995328006160149365; else eaMGRsZJdoiaUUUhdGjQKgeTwTGBGO=306394738.042229263905581238216931136442;if (eaMGRsZJdoiaUUUhdGjQKgeTwTGBGO == eaMGRsZJdoiaUUUhdGjQKgeTwTGBGO ) eaMGRsZJdoiaUUUhdGjQKgeTwTGBGO=1996754364.545307088727521168092115021219; else eaMGRsZJdoiaUUUhdGjQKgeTwTGBGO=1650609261.382246139436160941107855153148;if (eaMGRsZJdoiaUUUhdGjQKgeTwTGBGO == eaMGRsZJdoiaUUUhdGjQKgeTwTGBGO ) eaMGRsZJdoiaUUUhdGjQKgeTwTGBGO=1345091537.697944549504067616518121353810; else eaMGRsZJdoiaUUUhdGjQKgeTwTGBGO=1258532001.410185032750261490854774167669;if (eaMGRsZJdoiaUUUhdGjQKgeTwTGBGO == eaMGRsZJdoiaUUUhdGjQKgeTwTGBGO ) eaMGRsZJdoiaUUUhdGjQKgeTwTGBGO=652367507.283019598378586896107307675713; else eaMGRsZJdoiaUUUhdGjQKgeTwTGBGO=645650692.857368835291368639838157398655;float poZZNILOtBeEtBgAXtwVhuqZivxyhy=1071559042.119639896701153561671973834385f;if (poZZNILOtBeEtBgAXtwVhuqZivxyhy - poZZNILOtBeEtBgAXtwVhuqZivxyhy> 0.00000001 ) poZZNILOtBeEtBgAXtwVhuqZivxyhy=1854648710.687964776173851977297900935690f; else poZZNILOtBeEtBgAXtwVhuqZivxyhy=2031633448.552223529099661823396819878704f;if (poZZNILOtBeEtBgAXtwVhuqZivxyhy - poZZNILOtBeEtBgAXtwVhuqZivxyhy> 0.00000001 ) poZZNILOtBeEtBgAXtwVhuqZivxyhy=1381721131.019662173856831835022330474445f; else poZZNILOtBeEtBgAXtwVhuqZivxyhy=1148107109.592609010075055119914489389020f;if (poZZNILOtBeEtBgAXtwVhuqZivxyhy - poZZNILOtBeEtBgAXtwVhuqZivxyhy> 0.00000001 ) poZZNILOtBeEtBgAXtwVhuqZivxyhy=1237435417.883846433283615738547663112433f; else poZZNILOtBeEtBgAXtwVhuqZivxyhy=1668138893.119120231989673297576159332174f;if (poZZNILOtBeEtBgAXtwVhuqZivxyhy - poZZNILOtBeEtBgAXtwVhuqZivxyhy> 0.00000001 ) poZZNILOtBeEtBgAXtwVhuqZivxyhy=1070649560.510128569551866900103597816180f; else poZZNILOtBeEtBgAXtwVhuqZivxyhy=471108282.826858965674812530353307202432f;if (poZZNILOtBeEtBgAXtwVhuqZivxyhy - poZZNILOtBeEtBgAXtwVhuqZivxyhy> 0.00000001 ) poZZNILOtBeEtBgAXtwVhuqZivxyhy=399246558.348073502915563979538783753975f; else poZZNILOtBeEtBgAXtwVhuqZivxyhy=345290367.653334694004855995434401567543f;if (poZZNILOtBeEtBgAXtwVhuqZivxyhy - poZZNILOtBeEtBgAXtwVhuqZivxyhy> 0.00000001 ) poZZNILOtBeEtBgAXtwVhuqZivxyhy=1526895964.173746223049653494997855236397f; else poZZNILOtBeEtBgAXtwVhuqZivxyhy=2058185619.715194222977259685828376632861f;int tRdPOSXgwKjMxHCfCcfYCdmwUVycdJ=445418035;if (tRdPOSXgwKjMxHCfCcfYCdmwUVycdJ == tRdPOSXgwKjMxHCfCcfYCdmwUVycdJ- 1 ) tRdPOSXgwKjMxHCfCcfYCdmwUVycdJ=1694199325; else tRdPOSXgwKjMxHCfCcfYCdmwUVycdJ=401338728;if (tRdPOSXgwKjMxHCfCcfYCdmwUVycdJ == tRdPOSXgwKjMxHCfCcfYCdmwUVycdJ- 0 ) tRdPOSXgwKjMxHCfCcfYCdmwUVycdJ=63770439; else tRdPOSXgwKjMxHCfCcfYCdmwUVycdJ=1169519635;if (tRdPOSXgwKjMxHCfCcfYCdmwUVycdJ == tRdPOSXgwKjMxHCfCcfYCdmwUVycdJ- 1 ) tRdPOSXgwKjMxHCfCcfYCdmwUVycdJ=1765711819; else tRdPOSXgwKjMxHCfCcfYCdmwUVycdJ=1037268761;if (tRdPOSXgwKjMxHCfCcfYCdmwUVycdJ == tRdPOSXgwKjMxHCfCcfYCdmwUVycdJ- 0 ) tRdPOSXgwKjMxHCfCcfYCdmwUVycdJ=1999824567; else tRdPOSXgwKjMxHCfCcfYCdmwUVycdJ=903530347;if (tRdPOSXgwKjMxHCfCcfYCdmwUVycdJ == tRdPOSXgwKjMxHCfCcfYCdmwUVycdJ- 0 ) tRdPOSXgwKjMxHCfCcfYCdmwUVycdJ=1334894600; else tRdPOSXgwKjMxHCfCcfYCdmwUVycdJ=859317560;if (tRdPOSXgwKjMxHCfCcfYCdmwUVycdJ == tRdPOSXgwKjMxHCfCcfYCdmwUVycdJ- 1 ) tRdPOSXgwKjMxHCfCcfYCdmwUVycdJ=1532929036; else tRdPOSXgwKjMxHCfCcfYCdmwUVycdJ=522925675;long ANednDiWOcRQMfuWfEeaPTpiSpvWbR=1402650020;if (ANednDiWOcRQMfuWfEeaPTpiSpvWbR == ANednDiWOcRQMfuWfEeaPTpiSpvWbR- 0 ) ANednDiWOcRQMfuWfEeaPTpiSpvWbR=1929409212; else ANednDiWOcRQMfuWfEeaPTpiSpvWbR=399223275;if (ANednDiWOcRQMfuWfEeaPTpiSpvWbR == ANednDiWOcRQMfuWfEeaPTpiSpvWbR- 0 ) ANednDiWOcRQMfuWfEeaPTpiSpvWbR=162677497; else ANednDiWOcRQMfuWfEeaPTpiSpvWbR=1853462894;if (ANednDiWOcRQMfuWfEeaPTpiSpvWbR == ANednDiWOcRQMfuWfEeaPTpiSpvWbR- 0 ) ANednDiWOcRQMfuWfEeaPTpiSpvWbR=1923203965; else ANednDiWOcRQMfuWfEeaPTpiSpvWbR=1122882915;if (ANednDiWOcRQMfuWfEeaPTpiSpvWbR == ANednDiWOcRQMfuWfEeaPTpiSpvWbR- 0 ) ANednDiWOcRQMfuWfEeaPTpiSpvWbR=1202196717; else ANednDiWOcRQMfuWfEeaPTpiSpvWbR=455672637;if (ANednDiWOcRQMfuWfEeaPTpiSpvWbR == ANednDiWOcRQMfuWfEeaPTpiSpvWbR- 1 ) ANednDiWOcRQMfuWfEeaPTpiSpvWbR=73292293; else ANednDiWOcRQMfuWfEeaPTpiSpvWbR=1628856884;if (ANednDiWOcRQMfuWfEeaPTpiSpvWbR == ANednDiWOcRQMfuWfEeaPTpiSpvWbR- 1 ) ANednDiWOcRQMfuWfEeaPTpiSpvWbR=267528448; else ANednDiWOcRQMfuWfEeaPTpiSpvWbR=89724430;int zOzMxdNwiodgEXUNZmcLnZnPVgRbWW=1277379329;if (zOzMxdNwiodgEXUNZmcLnZnPVgRbWW == zOzMxdNwiodgEXUNZmcLnZnPVgRbWW- 0 ) zOzMxdNwiodgEXUNZmcLnZnPVgRbWW=229672470; else zOzMxdNwiodgEXUNZmcLnZnPVgRbWW=206949472;if (zOzMxdNwiodgEXUNZmcLnZnPVgRbWW == zOzMxdNwiodgEXUNZmcLnZnPVgRbWW- 1 ) zOzMxdNwiodgEXUNZmcLnZnPVgRbWW=1098935014; else zOzMxdNwiodgEXUNZmcLnZnPVgRbWW=198070754;if (zOzMxdNwiodgEXUNZmcLnZnPVgRbWW == zOzMxdNwiodgEXUNZmcLnZnPVgRbWW- 0 ) zOzMxdNwiodgEXUNZmcLnZnPVgRbWW=1364717962; else zOzMxdNwiodgEXUNZmcLnZnPVgRbWW=15268827;if (zOzMxdNwiodgEXUNZmcLnZnPVgRbWW == zOzMxdNwiodgEXUNZmcLnZnPVgRbWW- 0 ) zOzMxdNwiodgEXUNZmcLnZnPVgRbWW=1453330870; else zOzMxdNwiodgEXUNZmcLnZnPVgRbWW=817379949;if (zOzMxdNwiodgEXUNZmcLnZnPVgRbWW == zOzMxdNwiodgEXUNZmcLnZnPVgRbWW- 0 ) zOzMxdNwiodgEXUNZmcLnZnPVgRbWW=31808881; else zOzMxdNwiodgEXUNZmcLnZnPVgRbWW=478190784;if (zOzMxdNwiodgEXUNZmcLnZnPVgRbWW == zOzMxdNwiodgEXUNZmcLnZnPVgRbWW- 1 ) zOzMxdNwiodgEXUNZmcLnZnPVgRbWW=1960696156; else zOzMxdNwiodgEXUNZmcLnZnPVgRbWW=378117857;double WnpdKLeJCrJYfHxvowsIuHWhAFdWJJ=1034839713.676481542509668527716246380642;if (WnpdKLeJCrJYfHxvowsIuHWhAFdWJJ == WnpdKLeJCrJYfHxvowsIuHWhAFdWJJ ) WnpdKLeJCrJYfHxvowsIuHWhAFdWJJ=202416487.622150944885642044727471344021; else WnpdKLeJCrJYfHxvowsIuHWhAFdWJJ=124245944.191199851979304271535693626291;if (WnpdKLeJCrJYfHxvowsIuHWhAFdWJJ == WnpdKLeJCrJYfHxvowsIuHWhAFdWJJ ) WnpdKLeJCrJYfHxvowsIuHWhAFdWJJ=128776407.792479565439495928728369761968; else WnpdKLeJCrJYfHxvowsIuHWhAFdWJJ=1837285997.377038703682721630871857444482;if (WnpdKLeJCrJYfHxvowsIuHWhAFdWJJ == WnpdKLeJCrJYfHxvowsIuHWhAFdWJJ ) WnpdKLeJCrJYfHxvowsIuHWhAFdWJJ=2052518358.861716347822625775823119752179; else WnpdKLeJCrJYfHxvowsIuHWhAFdWJJ=1322880284.543182101530406934287967086206;if (WnpdKLeJCrJYfHxvowsIuHWhAFdWJJ == WnpdKLeJCrJYfHxvowsIuHWhAFdWJJ ) WnpdKLeJCrJYfHxvowsIuHWhAFdWJJ=669697717.428849054468449381492999368621; else WnpdKLeJCrJYfHxvowsIuHWhAFdWJJ=571316410.700612377709093053971592464968;if (WnpdKLeJCrJYfHxvowsIuHWhAFdWJJ == WnpdKLeJCrJYfHxvowsIuHWhAFdWJJ ) WnpdKLeJCrJYfHxvowsIuHWhAFdWJJ=1346625658.144387893811579490731585121826; else WnpdKLeJCrJYfHxvowsIuHWhAFdWJJ=1755581449.852966672727116003317423876936;if (WnpdKLeJCrJYfHxvowsIuHWhAFdWJJ == WnpdKLeJCrJYfHxvowsIuHWhAFdWJJ ) WnpdKLeJCrJYfHxvowsIuHWhAFdWJJ=1096227342.618707061482911557022851953910; else WnpdKLeJCrJYfHxvowsIuHWhAFdWJJ=136854935.085712942005303562342902486796;float IZDSDnKxtcGbxHoccqSsCdoqFpPNNh=1396325855.947675644637046634939453438816f;if (IZDSDnKxtcGbxHoccqSsCdoqFpPNNh - IZDSDnKxtcGbxHoccqSsCdoqFpPNNh> 0.00000001 ) IZDSDnKxtcGbxHoccqSsCdoqFpPNNh=1445217571.643477306179840502074400159424f; else IZDSDnKxtcGbxHoccqSsCdoqFpPNNh=255549707.894800567033996161494784334442f;if (IZDSDnKxtcGbxHoccqSsCdoqFpPNNh - IZDSDnKxtcGbxHoccqSsCdoqFpPNNh> 0.00000001 ) IZDSDnKxtcGbxHoccqSsCdoqFpPNNh=293253205.808348604115504152400904012068f; else IZDSDnKxtcGbxHoccqSsCdoqFpPNNh=606574231.622533676867749028028205300533f;if (IZDSDnKxtcGbxHoccqSsCdoqFpPNNh - IZDSDnKxtcGbxHoccqSsCdoqFpPNNh> 0.00000001 ) IZDSDnKxtcGbxHoccqSsCdoqFpPNNh=412955478.764750308269765327610405152823f; else IZDSDnKxtcGbxHoccqSsCdoqFpPNNh=2092286773.314310319972097954019402447061f;if (IZDSDnKxtcGbxHoccqSsCdoqFpPNNh - IZDSDnKxtcGbxHoccqSsCdoqFpPNNh> 0.00000001 ) IZDSDnKxtcGbxHoccqSsCdoqFpPNNh=37932058.968595555601875841820882502308f; else IZDSDnKxtcGbxHoccqSsCdoqFpPNNh=1914140385.345912450341951863860624118710f;if (IZDSDnKxtcGbxHoccqSsCdoqFpPNNh - IZDSDnKxtcGbxHoccqSsCdoqFpPNNh> 0.00000001 ) IZDSDnKxtcGbxHoccqSsCdoqFpPNNh=1093376052.335736607349269994284054983281f; else IZDSDnKxtcGbxHoccqSsCdoqFpPNNh=1492781195.485102722527653156632453638195f;if (IZDSDnKxtcGbxHoccqSsCdoqFpPNNh - IZDSDnKxtcGbxHoccqSsCdoqFpPNNh> 0.00000001 ) IZDSDnKxtcGbxHoccqSsCdoqFpPNNh=1821654013.452706417727287841194866904367f; else IZDSDnKxtcGbxHoccqSsCdoqFpPNNh=1445013181.464817606874001527780801270680f;double GbvNeRZizLUZRHynfaUphilphgRdXk=1673052055.255960052883106668592665485510;if (GbvNeRZizLUZRHynfaUphilphgRdXk == GbvNeRZizLUZRHynfaUphilphgRdXk ) GbvNeRZizLUZRHynfaUphilphgRdXk=1971272305.089825076893643307835576260058; else GbvNeRZizLUZRHynfaUphilphgRdXk=1380172382.565280539179371013229979661376;if (GbvNeRZizLUZRHynfaUphilphgRdXk == GbvNeRZizLUZRHynfaUphilphgRdXk ) GbvNeRZizLUZRHynfaUphilphgRdXk=2034791590.613834946497986630635884979763; else GbvNeRZizLUZRHynfaUphilphgRdXk=802736477.927367755727794310314184183997;if (GbvNeRZizLUZRHynfaUphilphgRdXk == GbvNeRZizLUZRHynfaUphilphgRdXk ) GbvNeRZizLUZRHynfaUphilphgRdXk=1957275985.629457062008692910177956085428; else GbvNeRZizLUZRHynfaUphilphgRdXk=1871781286.486920944745931669337393088357;if (GbvNeRZizLUZRHynfaUphilphgRdXk == GbvNeRZizLUZRHynfaUphilphgRdXk ) GbvNeRZizLUZRHynfaUphilphgRdXk=1372344618.264463388218643212543720229395; else GbvNeRZizLUZRHynfaUphilphgRdXk=1054275833.360858157939122986097681565535;if (GbvNeRZizLUZRHynfaUphilphgRdXk == GbvNeRZizLUZRHynfaUphilphgRdXk ) GbvNeRZizLUZRHynfaUphilphgRdXk=1684735402.409495999616672460834331888939; else GbvNeRZizLUZRHynfaUphilphgRdXk=527618604.473253083719858202011570118972;if (GbvNeRZizLUZRHynfaUphilphgRdXk == GbvNeRZizLUZRHynfaUphilphgRdXk ) GbvNeRZizLUZRHynfaUphilphgRdXk=998928743.806025342959812668468703991324; else GbvNeRZizLUZRHynfaUphilphgRdXk=1756620160.621202483497117287866061420414; }
 GbvNeRZizLUZRHynfaUphilphgRdXky::GbvNeRZizLUZRHynfaUphilphgRdXky()
 { this->AAcAgNqvJDYU("LedcrBiYGlBgdaMgtJtRCSHcXqGfyzAAcAgNqvJDYUj", true, 1066525132, 1316475935, 47639636); }
#pragma optimize("", off)
 // <delete/>


// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class dpBMutDbiDnldRByYZqwgQIPXYRuKEy
 { 
public: bool SaZLNQrFpvuLyRcQDEXrshfuNgmoVb; double SaZLNQrFpvuLyRcQDEXrshfuNgmoVbdpBMutDbiDnldRByYZqwgQIPXYRuKE; dpBMutDbiDnldRByYZqwgQIPXYRuKEy(); void VPlxHFPnbIbS(string SaZLNQrFpvuLyRcQDEXrshfuNgmoVbVPlxHFPnbIbS, bool JQdQUKEGoihUABmFJxvHOqnNukveyg, int dCjOldhhwRMxQoJYhdIHXosONTcPWk, float PZIIEekmQIXbAdvQayEWDSVqIpJUAS, long BRLMCmpHkenvMAkwoOTwseLzhxDSzP);
 protected: bool SaZLNQrFpvuLyRcQDEXrshfuNgmoVbo; double SaZLNQrFpvuLyRcQDEXrshfuNgmoVbdpBMutDbiDnldRByYZqwgQIPXYRuKEf; void VPlxHFPnbIbSu(string SaZLNQrFpvuLyRcQDEXrshfuNgmoVbVPlxHFPnbIbSg, bool JQdQUKEGoihUABmFJxvHOqnNukveyge, int dCjOldhhwRMxQoJYhdIHXosONTcPWkr, float PZIIEekmQIXbAdvQayEWDSVqIpJUASw, long BRLMCmpHkenvMAkwoOTwseLzhxDSzPn);
 private: bool SaZLNQrFpvuLyRcQDEXrshfuNgmoVbJQdQUKEGoihUABmFJxvHOqnNukveyg; double SaZLNQrFpvuLyRcQDEXrshfuNgmoVbPZIIEekmQIXbAdvQayEWDSVqIpJUASdpBMutDbiDnldRByYZqwgQIPXYRuKE;
 void VPlxHFPnbIbSv(string JQdQUKEGoihUABmFJxvHOqnNukveygVPlxHFPnbIbS, bool JQdQUKEGoihUABmFJxvHOqnNukveygdCjOldhhwRMxQoJYhdIHXosONTcPWk, int dCjOldhhwRMxQoJYhdIHXosONTcPWkSaZLNQrFpvuLyRcQDEXrshfuNgmoVb, float PZIIEekmQIXbAdvQayEWDSVqIpJUASBRLMCmpHkenvMAkwoOTwseLzhxDSzP, long BRLMCmpHkenvMAkwoOTwseLzhxDSzPJQdQUKEGoihUABmFJxvHOqnNukveyg); };
 void dpBMutDbiDnldRByYZqwgQIPXYRuKEy::VPlxHFPnbIbS(string SaZLNQrFpvuLyRcQDEXrshfuNgmoVbVPlxHFPnbIbS, bool JQdQUKEGoihUABmFJxvHOqnNukveyg, int dCjOldhhwRMxQoJYhdIHXosONTcPWk, float PZIIEekmQIXbAdvQayEWDSVqIpJUAS, long BRLMCmpHkenvMAkwoOTwseLzhxDSzP)
 { int fUeRKleLxPpRNdRjyHWfifWqsHxCQs=932622811;if (fUeRKleLxPpRNdRjyHWfifWqsHxCQs == fUeRKleLxPpRNdRjyHWfifWqsHxCQs- 1 ) fUeRKleLxPpRNdRjyHWfifWqsHxCQs=2081926029; else fUeRKleLxPpRNdRjyHWfifWqsHxCQs=227912664;if (fUeRKleLxPpRNdRjyHWfifWqsHxCQs == fUeRKleLxPpRNdRjyHWfifWqsHxCQs- 0 ) fUeRKleLxPpRNdRjyHWfifWqsHxCQs=1394666087; else fUeRKleLxPpRNdRjyHWfifWqsHxCQs=1193715232;if (fUeRKleLxPpRNdRjyHWfifWqsHxCQs == fUeRKleLxPpRNdRjyHWfifWqsHxCQs- 0 ) fUeRKleLxPpRNdRjyHWfifWqsHxCQs=1546227097; else fUeRKleLxPpRNdRjyHWfifWqsHxCQs=163072598;if (fUeRKleLxPpRNdRjyHWfifWqsHxCQs == fUeRKleLxPpRNdRjyHWfifWqsHxCQs- 1 ) fUeRKleLxPpRNdRjyHWfifWqsHxCQs=607925101; else fUeRKleLxPpRNdRjyHWfifWqsHxCQs=1072090000;if (fUeRKleLxPpRNdRjyHWfifWqsHxCQs == fUeRKleLxPpRNdRjyHWfifWqsHxCQs- 0 ) fUeRKleLxPpRNdRjyHWfifWqsHxCQs=1730305961; else fUeRKleLxPpRNdRjyHWfifWqsHxCQs=629865868;if (fUeRKleLxPpRNdRjyHWfifWqsHxCQs == fUeRKleLxPpRNdRjyHWfifWqsHxCQs- 1 ) fUeRKleLxPpRNdRjyHWfifWqsHxCQs=1320937849; else fUeRKleLxPpRNdRjyHWfifWqsHxCQs=2125060694;long nguXDvcgCSZRdXblhqWDYAlacbHXDe=1802860453;if (nguXDvcgCSZRdXblhqWDYAlacbHXDe == nguXDvcgCSZRdXblhqWDYAlacbHXDe- 0 ) nguXDvcgCSZRdXblhqWDYAlacbHXDe=1628321087; else nguXDvcgCSZRdXblhqWDYAlacbHXDe=504831191;if (nguXDvcgCSZRdXblhqWDYAlacbHXDe == nguXDvcgCSZRdXblhqWDYAlacbHXDe- 0 ) nguXDvcgCSZRdXblhqWDYAlacbHXDe=904255002; else nguXDvcgCSZRdXblhqWDYAlacbHXDe=1261792651;if (nguXDvcgCSZRdXblhqWDYAlacbHXDe == nguXDvcgCSZRdXblhqWDYAlacbHXDe- 0 ) nguXDvcgCSZRdXblhqWDYAlacbHXDe=1574594447; else nguXDvcgCSZRdXblhqWDYAlacbHXDe=4428006;if (nguXDvcgCSZRdXblhqWDYAlacbHXDe == nguXDvcgCSZRdXblhqWDYAlacbHXDe- 0 ) nguXDvcgCSZRdXblhqWDYAlacbHXDe=1906174530; else nguXDvcgCSZRdXblhqWDYAlacbHXDe=1182325628;if (nguXDvcgCSZRdXblhqWDYAlacbHXDe == nguXDvcgCSZRdXblhqWDYAlacbHXDe- 0 ) nguXDvcgCSZRdXblhqWDYAlacbHXDe=428773823; else nguXDvcgCSZRdXblhqWDYAlacbHXDe=196961190;if (nguXDvcgCSZRdXblhqWDYAlacbHXDe == nguXDvcgCSZRdXblhqWDYAlacbHXDe- 1 ) nguXDvcgCSZRdXblhqWDYAlacbHXDe=1286037003; else nguXDvcgCSZRdXblhqWDYAlacbHXDe=797466137;float VJoLTdKaNeNjCZMqAhuaTDWebhSRQU=2074781068.099513314454663191308347768077f;if (VJoLTdKaNeNjCZMqAhuaTDWebhSRQU - VJoLTdKaNeNjCZMqAhuaTDWebhSRQU> 0.00000001 ) VJoLTdKaNeNjCZMqAhuaTDWebhSRQU=1315254169.223380881888958276424966785395f; else VJoLTdKaNeNjCZMqAhuaTDWebhSRQU=192580708.644833238341152040959276437861f;if (VJoLTdKaNeNjCZMqAhuaTDWebhSRQU - VJoLTdKaNeNjCZMqAhuaTDWebhSRQU> 0.00000001 ) VJoLTdKaNeNjCZMqAhuaTDWebhSRQU=1080593544.550635423615918391484654748950f; else VJoLTdKaNeNjCZMqAhuaTDWebhSRQU=983975119.758780253863745546825648801609f;if (VJoLTdKaNeNjCZMqAhuaTDWebhSRQU - VJoLTdKaNeNjCZMqAhuaTDWebhSRQU> 0.00000001 ) VJoLTdKaNeNjCZMqAhuaTDWebhSRQU=609647915.767497724914972192921313511009f; else VJoLTdKaNeNjCZMqAhuaTDWebhSRQU=1864159464.976411645993201288771078945598f;if (VJoLTdKaNeNjCZMqAhuaTDWebhSRQU - VJoLTdKaNeNjCZMqAhuaTDWebhSRQU> 0.00000001 ) VJoLTdKaNeNjCZMqAhuaTDWebhSRQU=910611475.672605856156714921334444900968f; else VJoLTdKaNeNjCZMqAhuaTDWebhSRQU=1407548.726742161753362637513427534980f;if (VJoLTdKaNeNjCZMqAhuaTDWebhSRQU - VJoLTdKaNeNjCZMqAhuaTDWebhSRQU> 0.00000001 ) VJoLTdKaNeNjCZMqAhuaTDWebhSRQU=1565416182.509954018286169228238356530931f; else VJoLTdKaNeNjCZMqAhuaTDWebhSRQU=987274231.939125452963033847832675336443f;if (VJoLTdKaNeNjCZMqAhuaTDWebhSRQU - VJoLTdKaNeNjCZMqAhuaTDWebhSRQU> 0.00000001 ) VJoLTdKaNeNjCZMqAhuaTDWebhSRQU=783061616.429736705487648642489410047490f; else VJoLTdKaNeNjCZMqAhuaTDWebhSRQU=1993212704.011064531715256504789417401970f;long QEZUlfipTFFGgkYKhTdSPjGGdAYffS=1268746619;if (QEZUlfipTFFGgkYKhTdSPjGGdAYffS == QEZUlfipTFFGgkYKhTdSPjGGdAYffS- 1 ) QEZUlfipTFFGgkYKhTdSPjGGdAYffS=347243905; else QEZUlfipTFFGgkYKhTdSPjGGdAYffS=1327844000;if (QEZUlfipTFFGgkYKhTdSPjGGdAYffS == QEZUlfipTFFGgkYKhTdSPjGGdAYffS- 1 ) QEZUlfipTFFGgkYKhTdSPjGGdAYffS=210626042; else QEZUlfipTFFGgkYKhTdSPjGGdAYffS=28129511;if (QEZUlfipTFFGgkYKhTdSPjGGdAYffS == QEZUlfipTFFGgkYKhTdSPjGGdAYffS- 1 ) QEZUlfipTFFGgkYKhTdSPjGGdAYffS=1389931900; else QEZUlfipTFFGgkYKhTdSPjGGdAYffS=763382658;if (QEZUlfipTFFGgkYKhTdSPjGGdAYffS == QEZUlfipTFFGgkYKhTdSPjGGdAYffS- 1 ) QEZUlfipTFFGgkYKhTdSPjGGdAYffS=1505512011; else QEZUlfipTFFGgkYKhTdSPjGGdAYffS=2077498023;if (QEZUlfipTFFGgkYKhTdSPjGGdAYffS == QEZUlfipTFFGgkYKhTdSPjGGdAYffS- 0 ) QEZUlfipTFFGgkYKhTdSPjGGdAYffS=469221781; else QEZUlfipTFFGgkYKhTdSPjGGdAYffS=212340459;if (QEZUlfipTFFGgkYKhTdSPjGGdAYffS == QEZUlfipTFFGgkYKhTdSPjGGdAYffS- 0 ) QEZUlfipTFFGgkYKhTdSPjGGdAYffS=764941409; else QEZUlfipTFFGgkYKhTdSPjGGdAYffS=1349265907;double sfLvIhNQAXwbrFQlFNuqSFrzvNMJHx=697065609.602030511991965148707248699300;if (sfLvIhNQAXwbrFQlFNuqSFrzvNMJHx == sfLvIhNQAXwbrFQlFNuqSFrzvNMJHx ) sfLvIhNQAXwbrFQlFNuqSFrzvNMJHx=1775549510.554729464077653256611764047554; else sfLvIhNQAXwbrFQlFNuqSFrzvNMJHx=1393640428.222154825688584070645470980089;if (sfLvIhNQAXwbrFQlFNuqSFrzvNMJHx == sfLvIhNQAXwbrFQlFNuqSFrzvNMJHx ) sfLvIhNQAXwbrFQlFNuqSFrzvNMJHx=809806525.302095560953214428108891095328; else sfLvIhNQAXwbrFQlFNuqSFrzvNMJHx=451460831.585638660962080237766208261200;if (sfLvIhNQAXwbrFQlFNuqSFrzvNMJHx == sfLvIhNQAXwbrFQlFNuqSFrzvNMJHx ) sfLvIhNQAXwbrFQlFNuqSFrzvNMJHx=891485651.024347968687925782197876349758; else sfLvIhNQAXwbrFQlFNuqSFrzvNMJHx=245602592.856094884447019939469912426811;if (sfLvIhNQAXwbrFQlFNuqSFrzvNMJHx == sfLvIhNQAXwbrFQlFNuqSFrzvNMJHx ) sfLvIhNQAXwbrFQlFNuqSFrzvNMJHx=1193212538.822183203291975096841149095477; else sfLvIhNQAXwbrFQlFNuqSFrzvNMJHx=1911645270.807362604684637814844017088319;if (sfLvIhNQAXwbrFQlFNuqSFrzvNMJHx == sfLvIhNQAXwbrFQlFNuqSFrzvNMJHx ) sfLvIhNQAXwbrFQlFNuqSFrzvNMJHx=1741703363.983640462359336852329174490149; else sfLvIhNQAXwbrFQlFNuqSFrzvNMJHx=902847454.539571119845359688224710192036;if (sfLvIhNQAXwbrFQlFNuqSFrzvNMJHx == sfLvIhNQAXwbrFQlFNuqSFrzvNMJHx ) sfLvIhNQAXwbrFQlFNuqSFrzvNMJHx=1843860116.143393086684368483691202683059; else sfLvIhNQAXwbrFQlFNuqSFrzvNMJHx=587320216.029471316660365046271944595282;float BPlgbZvsnHojyPExRicWksopBpZxqe=1403899779.918500445814686980204936790512f;if (BPlgbZvsnHojyPExRicWksopBpZxqe - BPlgbZvsnHojyPExRicWksopBpZxqe> 0.00000001 ) BPlgbZvsnHojyPExRicWksopBpZxqe=969782255.401699626660376582478575708381f; else BPlgbZvsnHojyPExRicWksopBpZxqe=17290384.004441309747575576031814594700f;if (BPlgbZvsnHojyPExRicWksopBpZxqe - BPlgbZvsnHojyPExRicWksopBpZxqe> 0.00000001 ) BPlgbZvsnHojyPExRicWksopBpZxqe=450222232.144566703528484210134235029106f; else BPlgbZvsnHojyPExRicWksopBpZxqe=844958617.871061302988452453039478691054f;if (BPlgbZvsnHojyPExRicWksopBpZxqe - BPlgbZvsnHojyPExRicWksopBpZxqe> 0.00000001 ) BPlgbZvsnHojyPExRicWksopBpZxqe=822614310.771885843401925798478717751457f; else BPlgbZvsnHojyPExRicWksopBpZxqe=973670494.046822768771174865391468716414f;if (BPlgbZvsnHojyPExRicWksopBpZxqe - BPlgbZvsnHojyPExRicWksopBpZxqe> 0.00000001 ) BPlgbZvsnHojyPExRicWksopBpZxqe=684849168.540756611304071765409616778821f; else BPlgbZvsnHojyPExRicWksopBpZxqe=551871511.645523153835509740805212503508f;if (BPlgbZvsnHojyPExRicWksopBpZxqe - BPlgbZvsnHojyPExRicWksopBpZxqe> 0.00000001 ) BPlgbZvsnHojyPExRicWksopBpZxqe=834552344.980087965972267421704912644413f; else BPlgbZvsnHojyPExRicWksopBpZxqe=1150579168.075506653399504405619281518944f;if (BPlgbZvsnHojyPExRicWksopBpZxqe - BPlgbZvsnHojyPExRicWksopBpZxqe> 0.00000001 ) BPlgbZvsnHojyPExRicWksopBpZxqe=514753338.466390887297942642834980589409f; else BPlgbZvsnHojyPExRicWksopBpZxqe=397307456.872003331563953244907712058691f;long UmnRHauWtKFCRlGzTkLVbZCJHgAzXb=1754957218;if (UmnRHauWtKFCRlGzTkLVbZCJHgAzXb == UmnRHauWtKFCRlGzTkLVbZCJHgAzXb- 0 ) UmnRHauWtKFCRlGzTkLVbZCJHgAzXb=2087986848; else UmnRHauWtKFCRlGzTkLVbZCJHgAzXb=85996007;if (UmnRHauWtKFCRlGzTkLVbZCJHgAzXb == UmnRHauWtKFCRlGzTkLVbZCJHgAzXb- 0 ) UmnRHauWtKFCRlGzTkLVbZCJHgAzXb=217663194; else UmnRHauWtKFCRlGzTkLVbZCJHgAzXb=1587529064;if (UmnRHauWtKFCRlGzTkLVbZCJHgAzXb == UmnRHauWtKFCRlGzTkLVbZCJHgAzXb- 0 ) UmnRHauWtKFCRlGzTkLVbZCJHgAzXb=362551166; else UmnRHauWtKFCRlGzTkLVbZCJHgAzXb=923201751;if (UmnRHauWtKFCRlGzTkLVbZCJHgAzXb == UmnRHauWtKFCRlGzTkLVbZCJHgAzXb- 0 ) UmnRHauWtKFCRlGzTkLVbZCJHgAzXb=377260968; else UmnRHauWtKFCRlGzTkLVbZCJHgAzXb=101641313;if (UmnRHauWtKFCRlGzTkLVbZCJHgAzXb == UmnRHauWtKFCRlGzTkLVbZCJHgAzXb- 0 ) UmnRHauWtKFCRlGzTkLVbZCJHgAzXb=134631462; else UmnRHauWtKFCRlGzTkLVbZCJHgAzXb=446464419;if (UmnRHauWtKFCRlGzTkLVbZCJHgAzXb == UmnRHauWtKFCRlGzTkLVbZCJHgAzXb- 1 ) UmnRHauWtKFCRlGzTkLVbZCJHgAzXb=433909795; else UmnRHauWtKFCRlGzTkLVbZCJHgAzXb=487084446;float AQkRKFPOftiaxBNcwjpgMzYOGpKXoc=556259955.662095274028173410109832747754f;if (AQkRKFPOftiaxBNcwjpgMzYOGpKXoc - AQkRKFPOftiaxBNcwjpgMzYOGpKXoc> 0.00000001 ) AQkRKFPOftiaxBNcwjpgMzYOGpKXoc=2011184323.124280738169698969616813283145f; else AQkRKFPOftiaxBNcwjpgMzYOGpKXoc=1508525277.863090509388305034499814842101f;if (AQkRKFPOftiaxBNcwjpgMzYOGpKXoc - AQkRKFPOftiaxBNcwjpgMzYOGpKXoc> 0.00000001 ) AQkRKFPOftiaxBNcwjpgMzYOGpKXoc=379483652.860340661847144961926086705936f; else AQkRKFPOftiaxBNcwjpgMzYOGpKXoc=1007228858.940952907557465288198139599474f;if (AQkRKFPOftiaxBNcwjpgMzYOGpKXoc - AQkRKFPOftiaxBNcwjpgMzYOGpKXoc> 0.00000001 ) AQkRKFPOftiaxBNcwjpgMzYOGpKXoc=1507720379.574500829216448063253794881247f; else AQkRKFPOftiaxBNcwjpgMzYOGpKXoc=201812413.453408396556099380163732289172f;if (AQkRKFPOftiaxBNcwjpgMzYOGpKXoc - AQkRKFPOftiaxBNcwjpgMzYOGpKXoc> 0.00000001 ) AQkRKFPOftiaxBNcwjpgMzYOGpKXoc=2021934946.978296314177771780561219573232f; else AQkRKFPOftiaxBNcwjpgMzYOGpKXoc=801879016.933569981407619616251062186082f;if (AQkRKFPOftiaxBNcwjpgMzYOGpKXoc - AQkRKFPOftiaxBNcwjpgMzYOGpKXoc> 0.00000001 ) AQkRKFPOftiaxBNcwjpgMzYOGpKXoc=708791260.329225521291026571197808207149f; else AQkRKFPOftiaxBNcwjpgMzYOGpKXoc=257935014.390929949529507015671600552103f;if (AQkRKFPOftiaxBNcwjpgMzYOGpKXoc - AQkRKFPOftiaxBNcwjpgMzYOGpKXoc> 0.00000001 ) AQkRKFPOftiaxBNcwjpgMzYOGpKXoc=1764392134.949300110863268941130257880147f; else AQkRKFPOftiaxBNcwjpgMzYOGpKXoc=1888390130.784778031890591844372218379252f;double ChSEsvbzyrDwSSyiidzSiqmYZleRxs=32874148.479437768695885032606888532675;if (ChSEsvbzyrDwSSyiidzSiqmYZleRxs == ChSEsvbzyrDwSSyiidzSiqmYZleRxs ) ChSEsvbzyrDwSSyiidzSiqmYZleRxs=1925959148.627935249944745961343754570201; else ChSEsvbzyrDwSSyiidzSiqmYZleRxs=88902898.629317895080376214377134308457;if (ChSEsvbzyrDwSSyiidzSiqmYZleRxs == ChSEsvbzyrDwSSyiidzSiqmYZleRxs ) ChSEsvbzyrDwSSyiidzSiqmYZleRxs=646817015.615933553827646448836793460512; else ChSEsvbzyrDwSSyiidzSiqmYZleRxs=1282854030.045958555487166050906073193258;if (ChSEsvbzyrDwSSyiidzSiqmYZleRxs == ChSEsvbzyrDwSSyiidzSiqmYZleRxs ) ChSEsvbzyrDwSSyiidzSiqmYZleRxs=1842782279.404298379999446064841171608396; else ChSEsvbzyrDwSSyiidzSiqmYZleRxs=908594366.630718397888112863814051734479;if (ChSEsvbzyrDwSSyiidzSiqmYZleRxs == ChSEsvbzyrDwSSyiidzSiqmYZleRxs ) ChSEsvbzyrDwSSyiidzSiqmYZleRxs=1041348613.119346074987707407094569369018; else ChSEsvbzyrDwSSyiidzSiqmYZleRxs=46580504.402888098680004835544759676118;if (ChSEsvbzyrDwSSyiidzSiqmYZleRxs == ChSEsvbzyrDwSSyiidzSiqmYZleRxs ) ChSEsvbzyrDwSSyiidzSiqmYZleRxs=1787513063.717759503042041567554468485432; else ChSEsvbzyrDwSSyiidzSiqmYZleRxs=348580919.208008335634350381911966446038;if (ChSEsvbzyrDwSSyiidzSiqmYZleRxs == ChSEsvbzyrDwSSyiidzSiqmYZleRxs ) ChSEsvbzyrDwSSyiidzSiqmYZleRxs=710327155.427185405323203011820553144366; else ChSEsvbzyrDwSSyiidzSiqmYZleRxs=376149891.341822837598779986453521807515;float ITrArJTnFNrSPSMmsSJcnuIRpLzCOW=2063844196.135808563161544841723082092790f;if (ITrArJTnFNrSPSMmsSJcnuIRpLzCOW - ITrArJTnFNrSPSMmsSJcnuIRpLzCOW> 0.00000001 ) ITrArJTnFNrSPSMmsSJcnuIRpLzCOW=441931182.520968152612612444582495816492f; else ITrArJTnFNrSPSMmsSJcnuIRpLzCOW=2105977030.797464168680022158175931814625f;if (ITrArJTnFNrSPSMmsSJcnuIRpLzCOW - ITrArJTnFNrSPSMmsSJcnuIRpLzCOW> 0.00000001 ) ITrArJTnFNrSPSMmsSJcnuIRpLzCOW=533606780.377002625838791357377933887775f; else ITrArJTnFNrSPSMmsSJcnuIRpLzCOW=1738176425.164341403063334724157836945532f;if (ITrArJTnFNrSPSMmsSJcnuIRpLzCOW - ITrArJTnFNrSPSMmsSJcnuIRpLzCOW> 0.00000001 ) ITrArJTnFNrSPSMmsSJcnuIRpLzCOW=1329345563.696087556095442561917362933951f; else ITrArJTnFNrSPSMmsSJcnuIRpLzCOW=117143095.707247745738927340548462534525f;if (ITrArJTnFNrSPSMmsSJcnuIRpLzCOW - ITrArJTnFNrSPSMmsSJcnuIRpLzCOW> 0.00000001 ) ITrArJTnFNrSPSMmsSJcnuIRpLzCOW=247236839.560630820583629955660451334672f; else ITrArJTnFNrSPSMmsSJcnuIRpLzCOW=1886174435.333037849752107594478096612031f;if (ITrArJTnFNrSPSMmsSJcnuIRpLzCOW - ITrArJTnFNrSPSMmsSJcnuIRpLzCOW> 0.00000001 ) ITrArJTnFNrSPSMmsSJcnuIRpLzCOW=211544708.635825070222792835585021444852f; else ITrArJTnFNrSPSMmsSJcnuIRpLzCOW=1563267825.691912738252526741489755671558f;if (ITrArJTnFNrSPSMmsSJcnuIRpLzCOW - ITrArJTnFNrSPSMmsSJcnuIRpLzCOW> 0.00000001 ) ITrArJTnFNrSPSMmsSJcnuIRpLzCOW=878737804.133873289021833361681447494635f; else ITrArJTnFNrSPSMmsSJcnuIRpLzCOW=104616152.239023085366357802232198870648f;int aaTqtJlJDPoeRKdItydwKVAQEVFOVP=1486210027;if (aaTqtJlJDPoeRKdItydwKVAQEVFOVP == aaTqtJlJDPoeRKdItydwKVAQEVFOVP- 1 ) aaTqtJlJDPoeRKdItydwKVAQEVFOVP=1781520851; else aaTqtJlJDPoeRKdItydwKVAQEVFOVP=401035647;if (aaTqtJlJDPoeRKdItydwKVAQEVFOVP == aaTqtJlJDPoeRKdItydwKVAQEVFOVP- 1 ) aaTqtJlJDPoeRKdItydwKVAQEVFOVP=708608945; else aaTqtJlJDPoeRKdItydwKVAQEVFOVP=978103291;if (aaTqtJlJDPoeRKdItydwKVAQEVFOVP == aaTqtJlJDPoeRKdItydwKVAQEVFOVP- 1 ) aaTqtJlJDPoeRKdItydwKVAQEVFOVP=271374780; else aaTqtJlJDPoeRKdItydwKVAQEVFOVP=1573277603;if (aaTqtJlJDPoeRKdItydwKVAQEVFOVP == aaTqtJlJDPoeRKdItydwKVAQEVFOVP- 0 ) aaTqtJlJDPoeRKdItydwKVAQEVFOVP=860961323; else aaTqtJlJDPoeRKdItydwKVAQEVFOVP=1102589957;if (aaTqtJlJDPoeRKdItydwKVAQEVFOVP == aaTqtJlJDPoeRKdItydwKVAQEVFOVP- 1 ) aaTqtJlJDPoeRKdItydwKVAQEVFOVP=467176664; else aaTqtJlJDPoeRKdItydwKVAQEVFOVP=332903148;if (aaTqtJlJDPoeRKdItydwKVAQEVFOVP == aaTqtJlJDPoeRKdItydwKVAQEVFOVP- 1 ) aaTqtJlJDPoeRKdItydwKVAQEVFOVP=1367199442; else aaTqtJlJDPoeRKdItydwKVAQEVFOVP=1159139460;float domdomGkRiEGxdzuZCnsfdqWEEoOGs=511026538.699241223126095675133120235695f;if (domdomGkRiEGxdzuZCnsfdqWEEoOGs - domdomGkRiEGxdzuZCnsfdqWEEoOGs> 0.00000001 ) domdomGkRiEGxdzuZCnsfdqWEEoOGs=773069692.201700865826233761398002367002f; else domdomGkRiEGxdzuZCnsfdqWEEoOGs=1848031585.774992649691142790767441831945f;if (domdomGkRiEGxdzuZCnsfdqWEEoOGs - domdomGkRiEGxdzuZCnsfdqWEEoOGs> 0.00000001 ) domdomGkRiEGxdzuZCnsfdqWEEoOGs=1446458948.386315244123370706168835634427f; else domdomGkRiEGxdzuZCnsfdqWEEoOGs=1919839518.270531684865675417113599765288f;if (domdomGkRiEGxdzuZCnsfdqWEEoOGs - domdomGkRiEGxdzuZCnsfdqWEEoOGs> 0.00000001 ) domdomGkRiEGxdzuZCnsfdqWEEoOGs=262298302.293795485022341326023731488288f; else domdomGkRiEGxdzuZCnsfdqWEEoOGs=1221734437.640667309014310126844037667232f;if (domdomGkRiEGxdzuZCnsfdqWEEoOGs - domdomGkRiEGxdzuZCnsfdqWEEoOGs> 0.00000001 ) domdomGkRiEGxdzuZCnsfdqWEEoOGs=851012232.831758288513801043246245572401f; else domdomGkRiEGxdzuZCnsfdqWEEoOGs=1808816053.117839377305933123672510669628f;if (domdomGkRiEGxdzuZCnsfdqWEEoOGs - domdomGkRiEGxdzuZCnsfdqWEEoOGs> 0.00000001 ) domdomGkRiEGxdzuZCnsfdqWEEoOGs=1989149706.698751708378604142255084070727f; else domdomGkRiEGxdzuZCnsfdqWEEoOGs=908126227.075971829343541492663858462588f;if (domdomGkRiEGxdzuZCnsfdqWEEoOGs - domdomGkRiEGxdzuZCnsfdqWEEoOGs> 0.00000001 ) domdomGkRiEGxdzuZCnsfdqWEEoOGs=1071309322.208230473442301364368917433761f; else domdomGkRiEGxdzuZCnsfdqWEEoOGs=279514454.934115531212932156811823130281f;int kFLZDrLqfrOQFaeytaRohtYCGFGBpZ=1151158578;if (kFLZDrLqfrOQFaeytaRohtYCGFGBpZ == kFLZDrLqfrOQFaeytaRohtYCGFGBpZ- 0 ) kFLZDrLqfrOQFaeytaRohtYCGFGBpZ=2125272507; else kFLZDrLqfrOQFaeytaRohtYCGFGBpZ=1371600775;if (kFLZDrLqfrOQFaeytaRohtYCGFGBpZ == kFLZDrLqfrOQFaeytaRohtYCGFGBpZ- 0 ) kFLZDrLqfrOQFaeytaRohtYCGFGBpZ=742112687; else kFLZDrLqfrOQFaeytaRohtYCGFGBpZ=138725052;if (kFLZDrLqfrOQFaeytaRohtYCGFGBpZ == kFLZDrLqfrOQFaeytaRohtYCGFGBpZ- 1 ) kFLZDrLqfrOQFaeytaRohtYCGFGBpZ=1636639424; else kFLZDrLqfrOQFaeytaRohtYCGFGBpZ=654245678;if (kFLZDrLqfrOQFaeytaRohtYCGFGBpZ == kFLZDrLqfrOQFaeytaRohtYCGFGBpZ- 0 ) kFLZDrLqfrOQFaeytaRohtYCGFGBpZ=1792469416; else kFLZDrLqfrOQFaeytaRohtYCGFGBpZ=1016137226;if (kFLZDrLqfrOQFaeytaRohtYCGFGBpZ == kFLZDrLqfrOQFaeytaRohtYCGFGBpZ- 1 ) kFLZDrLqfrOQFaeytaRohtYCGFGBpZ=1245015707; else kFLZDrLqfrOQFaeytaRohtYCGFGBpZ=1578500470;if (kFLZDrLqfrOQFaeytaRohtYCGFGBpZ == kFLZDrLqfrOQFaeytaRohtYCGFGBpZ- 0 ) kFLZDrLqfrOQFaeytaRohtYCGFGBpZ=1270999761; else kFLZDrLqfrOQFaeytaRohtYCGFGBpZ=996442415;long MBevrYLOYawJhumzqRFHFEbCrVJegm=713243300;if (MBevrYLOYawJhumzqRFHFEbCrVJegm == MBevrYLOYawJhumzqRFHFEbCrVJegm- 1 ) MBevrYLOYawJhumzqRFHFEbCrVJegm=1301149508; else MBevrYLOYawJhumzqRFHFEbCrVJegm=367711266;if (MBevrYLOYawJhumzqRFHFEbCrVJegm == MBevrYLOYawJhumzqRFHFEbCrVJegm- 0 ) MBevrYLOYawJhumzqRFHFEbCrVJegm=1479030107; else MBevrYLOYawJhumzqRFHFEbCrVJegm=1605745326;if (MBevrYLOYawJhumzqRFHFEbCrVJegm == MBevrYLOYawJhumzqRFHFEbCrVJegm- 1 ) MBevrYLOYawJhumzqRFHFEbCrVJegm=1337379184; else MBevrYLOYawJhumzqRFHFEbCrVJegm=598436188;if (MBevrYLOYawJhumzqRFHFEbCrVJegm == MBevrYLOYawJhumzqRFHFEbCrVJegm- 1 ) MBevrYLOYawJhumzqRFHFEbCrVJegm=1223911725; else MBevrYLOYawJhumzqRFHFEbCrVJegm=559786414;if (MBevrYLOYawJhumzqRFHFEbCrVJegm == MBevrYLOYawJhumzqRFHFEbCrVJegm- 1 ) MBevrYLOYawJhumzqRFHFEbCrVJegm=1697507062; else MBevrYLOYawJhumzqRFHFEbCrVJegm=1047276539;if (MBevrYLOYawJhumzqRFHFEbCrVJegm == MBevrYLOYawJhumzqRFHFEbCrVJegm- 0 ) MBevrYLOYawJhumzqRFHFEbCrVJegm=1136437232; else MBevrYLOYawJhumzqRFHFEbCrVJegm=1510732200;double OrrVAFauVQQVvSUcLPFRhMNCEwLkgg=1677927850.216056990230112738336653717995;if (OrrVAFauVQQVvSUcLPFRhMNCEwLkgg == OrrVAFauVQQVvSUcLPFRhMNCEwLkgg ) OrrVAFauVQQVvSUcLPFRhMNCEwLkgg=1592615908.464470279783996140107023276345; else OrrVAFauVQQVvSUcLPFRhMNCEwLkgg=216040864.900587784267075387867890855221;if (OrrVAFauVQQVvSUcLPFRhMNCEwLkgg == OrrVAFauVQQVvSUcLPFRhMNCEwLkgg ) OrrVAFauVQQVvSUcLPFRhMNCEwLkgg=1181350600.404589341957695115809059822174; else OrrVAFauVQQVvSUcLPFRhMNCEwLkgg=267368130.366397360714601120585043255677;if (OrrVAFauVQQVvSUcLPFRhMNCEwLkgg == OrrVAFauVQQVvSUcLPFRhMNCEwLkgg ) OrrVAFauVQQVvSUcLPFRhMNCEwLkgg=903672368.727760131985620646880370043424; else OrrVAFauVQQVvSUcLPFRhMNCEwLkgg=123393265.280409704200970052471089756783;if (OrrVAFauVQQVvSUcLPFRhMNCEwLkgg == OrrVAFauVQQVvSUcLPFRhMNCEwLkgg ) OrrVAFauVQQVvSUcLPFRhMNCEwLkgg=1021870761.882780632195278141109115431176; else OrrVAFauVQQVvSUcLPFRhMNCEwLkgg=118824014.302253839143139782172678478845;if (OrrVAFauVQQVvSUcLPFRhMNCEwLkgg == OrrVAFauVQQVvSUcLPFRhMNCEwLkgg ) OrrVAFauVQQVvSUcLPFRhMNCEwLkgg=55562169.066689313641217718785900584488; else OrrVAFauVQQVvSUcLPFRhMNCEwLkgg=430321722.609519756148064157286027116621;if (OrrVAFauVQQVvSUcLPFRhMNCEwLkgg == OrrVAFauVQQVvSUcLPFRhMNCEwLkgg ) OrrVAFauVQQVvSUcLPFRhMNCEwLkgg=674823811.693504270807374705754731269340; else OrrVAFauVQQVvSUcLPFRhMNCEwLkgg=831281479.313655666198848086342208897234;int wQyYtUlFvIVsgdcvtOMsSzBqIHZpgp=753568680;if (wQyYtUlFvIVsgdcvtOMsSzBqIHZpgp == wQyYtUlFvIVsgdcvtOMsSzBqIHZpgp- 1 ) wQyYtUlFvIVsgdcvtOMsSzBqIHZpgp=660920697; else wQyYtUlFvIVsgdcvtOMsSzBqIHZpgp=358066022;if (wQyYtUlFvIVsgdcvtOMsSzBqIHZpgp == wQyYtUlFvIVsgdcvtOMsSzBqIHZpgp- 1 ) wQyYtUlFvIVsgdcvtOMsSzBqIHZpgp=1125848593; else wQyYtUlFvIVsgdcvtOMsSzBqIHZpgp=1809628943;if (wQyYtUlFvIVsgdcvtOMsSzBqIHZpgp == wQyYtUlFvIVsgdcvtOMsSzBqIHZpgp- 1 ) wQyYtUlFvIVsgdcvtOMsSzBqIHZpgp=1605186001; else wQyYtUlFvIVsgdcvtOMsSzBqIHZpgp=1429509208;if (wQyYtUlFvIVsgdcvtOMsSzBqIHZpgp == wQyYtUlFvIVsgdcvtOMsSzBqIHZpgp- 1 ) wQyYtUlFvIVsgdcvtOMsSzBqIHZpgp=1961263041; else wQyYtUlFvIVsgdcvtOMsSzBqIHZpgp=1526836319;if (wQyYtUlFvIVsgdcvtOMsSzBqIHZpgp == wQyYtUlFvIVsgdcvtOMsSzBqIHZpgp- 1 ) wQyYtUlFvIVsgdcvtOMsSzBqIHZpgp=147689050; else wQyYtUlFvIVsgdcvtOMsSzBqIHZpgp=182326273;if (wQyYtUlFvIVsgdcvtOMsSzBqIHZpgp == wQyYtUlFvIVsgdcvtOMsSzBqIHZpgp- 0 ) wQyYtUlFvIVsgdcvtOMsSzBqIHZpgp=588701637; else wQyYtUlFvIVsgdcvtOMsSzBqIHZpgp=589362885;float mrysOHKSmpXelqDEVKBKfgvYgmGKum=663568838.049464717365674241714806921416f;if (mrysOHKSmpXelqDEVKBKfgvYgmGKum - mrysOHKSmpXelqDEVKBKfgvYgmGKum> 0.00000001 ) mrysOHKSmpXelqDEVKBKfgvYgmGKum=505767075.654455521590837015021454035354f; else mrysOHKSmpXelqDEVKBKfgvYgmGKum=1740555558.341171123072022407901650630152f;if (mrysOHKSmpXelqDEVKBKfgvYgmGKum - mrysOHKSmpXelqDEVKBKfgvYgmGKum> 0.00000001 ) mrysOHKSmpXelqDEVKBKfgvYgmGKum=1551310154.138736086790305943264845479165f; else mrysOHKSmpXelqDEVKBKfgvYgmGKum=681386436.217105530752232246579799939346f;if (mrysOHKSmpXelqDEVKBKfgvYgmGKum - mrysOHKSmpXelqDEVKBKfgvYgmGKum> 0.00000001 ) mrysOHKSmpXelqDEVKBKfgvYgmGKum=54230426.893269321142462271433479753937f; else mrysOHKSmpXelqDEVKBKfgvYgmGKum=2127321047.639497018493544843786892452551f;if (mrysOHKSmpXelqDEVKBKfgvYgmGKum - mrysOHKSmpXelqDEVKBKfgvYgmGKum> 0.00000001 ) mrysOHKSmpXelqDEVKBKfgvYgmGKum=1957054579.649452307637414093601026294815f; else mrysOHKSmpXelqDEVKBKfgvYgmGKum=1177756243.566785429255626059946906172437f;if (mrysOHKSmpXelqDEVKBKfgvYgmGKum - mrysOHKSmpXelqDEVKBKfgvYgmGKum> 0.00000001 ) mrysOHKSmpXelqDEVKBKfgvYgmGKum=1493324152.969217274968545741124608067071f; else mrysOHKSmpXelqDEVKBKfgvYgmGKum=1360206093.319340751989508956254241447136f;if (mrysOHKSmpXelqDEVKBKfgvYgmGKum - mrysOHKSmpXelqDEVKBKfgvYgmGKum> 0.00000001 ) mrysOHKSmpXelqDEVKBKfgvYgmGKum=1550183157.130376931610025510021193827891f; else mrysOHKSmpXelqDEVKBKfgvYgmGKum=907057462.362686023303665422793712514755f;int GRAOZpYVsctloLqaLsUzXGAogWzElI=1842937245;if (GRAOZpYVsctloLqaLsUzXGAogWzElI == GRAOZpYVsctloLqaLsUzXGAogWzElI- 1 ) GRAOZpYVsctloLqaLsUzXGAogWzElI=135887786; else GRAOZpYVsctloLqaLsUzXGAogWzElI=382868552;if (GRAOZpYVsctloLqaLsUzXGAogWzElI == GRAOZpYVsctloLqaLsUzXGAogWzElI- 1 ) GRAOZpYVsctloLqaLsUzXGAogWzElI=849811503; else GRAOZpYVsctloLqaLsUzXGAogWzElI=143902259;if (GRAOZpYVsctloLqaLsUzXGAogWzElI == GRAOZpYVsctloLqaLsUzXGAogWzElI- 1 ) GRAOZpYVsctloLqaLsUzXGAogWzElI=1603081923; else GRAOZpYVsctloLqaLsUzXGAogWzElI=1410588099;if (GRAOZpYVsctloLqaLsUzXGAogWzElI == GRAOZpYVsctloLqaLsUzXGAogWzElI- 0 ) GRAOZpYVsctloLqaLsUzXGAogWzElI=72741849; else GRAOZpYVsctloLqaLsUzXGAogWzElI=163106700;if (GRAOZpYVsctloLqaLsUzXGAogWzElI == GRAOZpYVsctloLqaLsUzXGAogWzElI- 0 ) GRAOZpYVsctloLqaLsUzXGAogWzElI=402261470; else GRAOZpYVsctloLqaLsUzXGAogWzElI=1801997928;if (GRAOZpYVsctloLqaLsUzXGAogWzElI == GRAOZpYVsctloLqaLsUzXGAogWzElI- 1 ) GRAOZpYVsctloLqaLsUzXGAogWzElI=119679080; else GRAOZpYVsctloLqaLsUzXGAogWzElI=721346095;float CtVNlMqZxWvuVEhXdzYNiVBzkdpALU=1921308600.729167078644447735708665438799f;if (CtVNlMqZxWvuVEhXdzYNiVBzkdpALU - CtVNlMqZxWvuVEhXdzYNiVBzkdpALU> 0.00000001 ) CtVNlMqZxWvuVEhXdzYNiVBzkdpALU=2063636424.900278720448145823641669227473f; else CtVNlMqZxWvuVEhXdzYNiVBzkdpALU=11254973.938975538012647162071516835106f;if (CtVNlMqZxWvuVEhXdzYNiVBzkdpALU - CtVNlMqZxWvuVEhXdzYNiVBzkdpALU> 0.00000001 ) CtVNlMqZxWvuVEhXdzYNiVBzkdpALU=325514404.135851037140574013326541252314f; else CtVNlMqZxWvuVEhXdzYNiVBzkdpALU=357662765.143399789714604214392398069561f;if (CtVNlMqZxWvuVEhXdzYNiVBzkdpALU - CtVNlMqZxWvuVEhXdzYNiVBzkdpALU> 0.00000001 ) CtVNlMqZxWvuVEhXdzYNiVBzkdpALU=450743310.972300739543471304804913396533f; else CtVNlMqZxWvuVEhXdzYNiVBzkdpALU=230002317.033984231468686993777576717012f;if (CtVNlMqZxWvuVEhXdzYNiVBzkdpALU - CtVNlMqZxWvuVEhXdzYNiVBzkdpALU> 0.00000001 ) CtVNlMqZxWvuVEhXdzYNiVBzkdpALU=699338254.421761169828445734840496766727f; else CtVNlMqZxWvuVEhXdzYNiVBzkdpALU=681083297.832015802594406914209338214501f;if (CtVNlMqZxWvuVEhXdzYNiVBzkdpALU - CtVNlMqZxWvuVEhXdzYNiVBzkdpALU> 0.00000001 ) CtVNlMqZxWvuVEhXdzYNiVBzkdpALU=548495090.644200754104235609195362950480f; else CtVNlMqZxWvuVEhXdzYNiVBzkdpALU=2095575997.130206130727011520361542648236f;if (CtVNlMqZxWvuVEhXdzYNiVBzkdpALU - CtVNlMqZxWvuVEhXdzYNiVBzkdpALU> 0.00000001 ) CtVNlMqZxWvuVEhXdzYNiVBzkdpALU=316304791.038407404068947835254012069877f; else CtVNlMqZxWvuVEhXdzYNiVBzkdpALU=244979908.890570072522741371681409302754f;int cSlNjqXkithQmNISoFhzuLqcTtFKlo=1902542451;if (cSlNjqXkithQmNISoFhzuLqcTtFKlo == cSlNjqXkithQmNISoFhzuLqcTtFKlo- 1 ) cSlNjqXkithQmNISoFhzuLqcTtFKlo=1720758182; else cSlNjqXkithQmNISoFhzuLqcTtFKlo=893248039;if (cSlNjqXkithQmNISoFhzuLqcTtFKlo == cSlNjqXkithQmNISoFhzuLqcTtFKlo- 1 ) cSlNjqXkithQmNISoFhzuLqcTtFKlo=453475099; else cSlNjqXkithQmNISoFhzuLqcTtFKlo=1282423541;if (cSlNjqXkithQmNISoFhzuLqcTtFKlo == cSlNjqXkithQmNISoFhzuLqcTtFKlo- 1 ) cSlNjqXkithQmNISoFhzuLqcTtFKlo=1579409360; else cSlNjqXkithQmNISoFhzuLqcTtFKlo=370266686;if (cSlNjqXkithQmNISoFhzuLqcTtFKlo == cSlNjqXkithQmNISoFhzuLqcTtFKlo- 1 ) cSlNjqXkithQmNISoFhzuLqcTtFKlo=1207970562; else cSlNjqXkithQmNISoFhzuLqcTtFKlo=1242662623;if (cSlNjqXkithQmNISoFhzuLqcTtFKlo == cSlNjqXkithQmNISoFhzuLqcTtFKlo- 0 ) cSlNjqXkithQmNISoFhzuLqcTtFKlo=1667813709; else cSlNjqXkithQmNISoFhzuLqcTtFKlo=1388203454;if (cSlNjqXkithQmNISoFhzuLqcTtFKlo == cSlNjqXkithQmNISoFhzuLqcTtFKlo- 1 ) cSlNjqXkithQmNISoFhzuLqcTtFKlo=279124966; else cSlNjqXkithQmNISoFhzuLqcTtFKlo=399716145;double bsCbuCgqtQrPOCVMeJWLBDVvTasQFF=1060816276.573968213669905998194205566067;if (bsCbuCgqtQrPOCVMeJWLBDVvTasQFF == bsCbuCgqtQrPOCVMeJWLBDVvTasQFF ) bsCbuCgqtQrPOCVMeJWLBDVvTasQFF=759190525.139987634430939878519023747951; else bsCbuCgqtQrPOCVMeJWLBDVvTasQFF=1776358204.225764112846576438680966811735;if (bsCbuCgqtQrPOCVMeJWLBDVvTasQFF == bsCbuCgqtQrPOCVMeJWLBDVvTasQFF ) bsCbuCgqtQrPOCVMeJWLBDVvTasQFF=990904685.079431637676788355731037147095; else bsCbuCgqtQrPOCVMeJWLBDVvTasQFF=2059841051.458887825590211438967601414330;if (bsCbuCgqtQrPOCVMeJWLBDVvTasQFF == bsCbuCgqtQrPOCVMeJWLBDVvTasQFF ) bsCbuCgqtQrPOCVMeJWLBDVvTasQFF=67115842.574233900164124193682924985848; else bsCbuCgqtQrPOCVMeJWLBDVvTasQFF=251388973.350513709953980369743275866135;if (bsCbuCgqtQrPOCVMeJWLBDVvTasQFF == bsCbuCgqtQrPOCVMeJWLBDVvTasQFF ) bsCbuCgqtQrPOCVMeJWLBDVvTasQFF=1392193935.926281562259583914463710629586; else bsCbuCgqtQrPOCVMeJWLBDVvTasQFF=2053369116.877404780090759480205277331677;if (bsCbuCgqtQrPOCVMeJWLBDVvTasQFF == bsCbuCgqtQrPOCVMeJWLBDVvTasQFF ) bsCbuCgqtQrPOCVMeJWLBDVvTasQFF=1831013945.309659218924458306225048160128; else bsCbuCgqtQrPOCVMeJWLBDVvTasQFF=168728206.411470111137239283004555347269;if (bsCbuCgqtQrPOCVMeJWLBDVvTasQFF == bsCbuCgqtQrPOCVMeJWLBDVvTasQFF ) bsCbuCgqtQrPOCVMeJWLBDVvTasQFF=1742890816.020161318939620881098712407714; else bsCbuCgqtQrPOCVMeJWLBDVvTasQFF=1654989573.406222828808628594666628231876;float dzUZOQVAqxhSItCJxXuFPGbnpoqAuD=1495539538.582877469703366778226614216342f;if (dzUZOQVAqxhSItCJxXuFPGbnpoqAuD - dzUZOQVAqxhSItCJxXuFPGbnpoqAuD> 0.00000001 ) dzUZOQVAqxhSItCJxXuFPGbnpoqAuD=198773986.141010601937089693830687997967f; else dzUZOQVAqxhSItCJxXuFPGbnpoqAuD=364620276.123867336718840964662451266412f;if (dzUZOQVAqxhSItCJxXuFPGbnpoqAuD - dzUZOQVAqxhSItCJxXuFPGbnpoqAuD> 0.00000001 ) dzUZOQVAqxhSItCJxXuFPGbnpoqAuD=1148071560.384482533247822394813331587261f; else dzUZOQVAqxhSItCJxXuFPGbnpoqAuD=788927236.293443446854143664007271915400f;if (dzUZOQVAqxhSItCJxXuFPGbnpoqAuD - dzUZOQVAqxhSItCJxXuFPGbnpoqAuD> 0.00000001 ) dzUZOQVAqxhSItCJxXuFPGbnpoqAuD=2126516424.538315458096906030893336403254f; else dzUZOQVAqxhSItCJxXuFPGbnpoqAuD=1466075674.814743793932712248508672831292f;if (dzUZOQVAqxhSItCJxXuFPGbnpoqAuD - dzUZOQVAqxhSItCJxXuFPGbnpoqAuD> 0.00000001 ) dzUZOQVAqxhSItCJxXuFPGbnpoqAuD=341899240.320528237434707273436585635392f; else dzUZOQVAqxhSItCJxXuFPGbnpoqAuD=1693369738.351261239502228508576887785590f;if (dzUZOQVAqxhSItCJxXuFPGbnpoqAuD - dzUZOQVAqxhSItCJxXuFPGbnpoqAuD> 0.00000001 ) dzUZOQVAqxhSItCJxXuFPGbnpoqAuD=950768058.404628364705403446814882657569f; else dzUZOQVAqxhSItCJxXuFPGbnpoqAuD=1230335428.684010961024498159435978663208f;if (dzUZOQVAqxhSItCJxXuFPGbnpoqAuD - dzUZOQVAqxhSItCJxXuFPGbnpoqAuD> 0.00000001 ) dzUZOQVAqxhSItCJxXuFPGbnpoqAuD=837332703.290513149368699049550913571655f; else dzUZOQVAqxhSItCJxXuFPGbnpoqAuD=1210023503.877394509502027739973575839714f;long ZvTawFPDdZVyAZnttgpVLllLOxKYcY=1460270253;if (ZvTawFPDdZVyAZnttgpVLllLOxKYcY == ZvTawFPDdZVyAZnttgpVLllLOxKYcY- 1 ) ZvTawFPDdZVyAZnttgpVLllLOxKYcY=452511382; else ZvTawFPDdZVyAZnttgpVLllLOxKYcY=1402972162;if (ZvTawFPDdZVyAZnttgpVLllLOxKYcY == ZvTawFPDdZVyAZnttgpVLllLOxKYcY- 1 ) ZvTawFPDdZVyAZnttgpVLllLOxKYcY=1633273030; else ZvTawFPDdZVyAZnttgpVLllLOxKYcY=250451494;if (ZvTawFPDdZVyAZnttgpVLllLOxKYcY == ZvTawFPDdZVyAZnttgpVLllLOxKYcY- 0 ) ZvTawFPDdZVyAZnttgpVLllLOxKYcY=63300894; else ZvTawFPDdZVyAZnttgpVLllLOxKYcY=1690866538;if (ZvTawFPDdZVyAZnttgpVLllLOxKYcY == ZvTawFPDdZVyAZnttgpVLllLOxKYcY- 0 ) ZvTawFPDdZVyAZnttgpVLllLOxKYcY=1835426609; else ZvTawFPDdZVyAZnttgpVLllLOxKYcY=1469369209;if (ZvTawFPDdZVyAZnttgpVLllLOxKYcY == ZvTawFPDdZVyAZnttgpVLllLOxKYcY- 1 ) ZvTawFPDdZVyAZnttgpVLllLOxKYcY=1648537751; else ZvTawFPDdZVyAZnttgpVLllLOxKYcY=547589630;if (ZvTawFPDdZVyAZnttgpVLllLOxKYcY == ZvTawFPDdZVyAZnttgpVLllLOxKYcY- 1 ) ZvTawFPDdZVyAZnttgpVLllLOxKYcY=1598893243; else ZvTawFPDdZVyAZnttgpVLllLOxKYcY=1410681154;float uxPmeNimrsSonHyEAfmrhpsDXmnPFD=1840051768.495606444057508649822551553831f;if (uxPmeNimrsSonHyEAfmrhpsDXmnPFD - uxPmeNimrsSonHyEAfmrhpsDXmnPFD> 0.00000001 ) uxPmeNimrsSonHyEAfmrhpsDXmnPFD=1104422805.596565375803175808092831395415f; else uxPmeNimrsSonHyEAfmrhpsDXmnPFD=931069075.371456104347503058786685986637f;if (uxPmeNimrsSonHyEAfmrhpsDXmnPFD - uxPmeNimrsSonHyEAfmrhpsDXmnPFD> 0.00000001 ) uxPmeNimrsSonHyEAfmrhpsDXmnPFD=200942159.196064282970585544945774070758f; else uxPmeNimrsSonHyEAfmrhpsDXmnPFD=1643021691.168735255987691620045389352219f;if (uxPmeNimrsSonHyEAfmrhpsDXmnPFD - uxPmeNimrsSonHyEAfmrhpsDXmnPFD> 0.00000001 ) uxPmeNimrsSonHyEAfmrhpsDXmnPFD=87636924.463592285140906919346072388059f; else uxPmeNimrsSonHyEAfmrhpsDXmnPFD=854137379.314653411044601649344498450221f;if (uxPmeNimrsSonHyEAfmrhpsDXmnPFD - uxPmeNimrsSonHyEAfmrhpsDXmnPFD> 0.00000001 ) uxPmeNimrsSonHyEAfmrhpsDXmnPFD=1081783499.433783745450840357284071602631f; else uxPmeNimrsSonHyEAfmrhpsDXmnPFD=1440598498.430072193891400466911304747329f;if (uxPmeNimrsSonHyEAfmrhpsDXmnPFD - uxPmeNimrsSonHyEAfmrhpsDXmnPFD> 0.00000001 ) uxPmeNimrsSonHyEAfmrhpsDXmnPFD=1434458964.212512187926550477234782014644f; else uxPmeNimrsSonHyEAfmrhpsDXmnPFD=1445018594.597579000838010596547110564050f;if (uxPmeNimrsSonHyEAfmrhpsDXmnPFD - uxPmeNimrsSonHyEAfmrhpsDXmnPFD> 0.00000001 ) uxPmeNimrsSonHyEAfmrhpsDXmnPFD=1109072993.965353019158840787363810759801f; else uxPmeNimrsSonHyEAfmrhpsDXmnPFD=984264061.505107377768850037099789715560f;double KClgXMlWOQDEdBZBiDmHFmIOoMeRrL=1531391836.151670516737588354002943041682;if (KClgXMlWOQDEdBZBiDmHFmIOoMeRrL == KClgXMlWOQDEdBZBiDmHFmIOoMeRrL ) KClgXMlWOQDEdBZBiDmHFmIOoMeRrL=1495286876.172209695304300306876234231974; else KClgXMlWOQDEdBZBiDmHFmIOoMeRrL=282620563.003566638077070865384582705908;if (KClgXMlWOQDEdBZBiDmHFmIOoMeRrL == KClgXMlWOQDEdBZBiDmHFmIOoMeRrL ) KClgXMlWOQDEdBZBiDmHFmIOoMeRrL=1202478191.872240194477890026457760062287; else KClgXMlWOQDEdBZBiDmHFmIOoMeRrL=1838794793.274121594088191770288966121229;if (KClgXMlWOQDEdBZBiDmHFmIOoMeRrL == KClgXMlWOQDEdBZBiDmHFmIOoMeRrL ) KClgXMlWOQDEdBZBiDmHFmIOoMeRrL=341972558.991831487054347968039311931801; else KClgXMlWOQDEdBZBiDmHFmIOoMeRrL=33329155.037629847030333573926420079091;if (KClgXMlWOQDEdBZBiDmHFmIOoMeRrL == KClgXMlWOQDEdBZBiDmHFmIOoMeRrL ) KClgXMlWOQDEdBZBiDmHFmIOoMeRrL=1432238644.078712782823210008847999561765; else KClgXMlWOQDEdBZBiDmHFmIOoMeRrL=655391095.195607263199111432108125529035;if (KClgXMlWOQDEdBZBiDmHFmIOoMeRrL == KClgXMlWOQDEdBZBiDmHFmIOoMeRrL ) KClgXMlWOQDEdBZBiDmHFmIOoMeRrL=676677314.103774861394686888823976817119; else KClgXMlWOQDEdBZBiDmHFmIOoMeRrL=1826185998.924819744868282455974132324797;if (KClgXMlWOQDEdBZBiDmHFmIOoMeRrL == KClgXMlWOQDEdBZBiDmHFmIOoMeRrL ) KClgXMlWOQDEdBZBiDmHFmIOoMeRrL=1287873132.785257197724416689557325680359; else KClgXMlWOQDEdBZBiDmHFmIOoMeRrL=1578926794.606829314547835073685535157177;float pTtELsfumRjcvFzXvvbFmhnRHlEmkI=1397787069.228099355283475129665535909475f;if (pTtELsfumRjcvFzXvvbFmhnRHlEmkI - pTtELsfumRjcvFzXvvbFmhnRHlEmkI> 0.00000001 ) pTtELsfumRjcvFzXvvbFmhnRHlEmkI=1081956113.258595255494405659034499980310f; else pTtELsfumRjcvFzXvvbFmhnRHlEmkI=1144764582.108114609008528213145045196961f;if (pTtELsfumRjcvFzXvvbFmhnRHlEmkI - pTtELsfumRjcvFzXvvbFmhnRHlEmkI> 0.00000001 ) pTtELsfumRjcvFzXvvbFmhnRHlEmkI=105600698.594549630105104708287838516402f; else pTtELsfumRjcvFzXvvbFmhnRHlEmkI=1167291923.974851409969404104159989587714f;if (pTtELsfumRjcvFzXvvbFmhnRHlEmkI - pTtELsfumRjcvFzXvvbFmhnRHlEmkI> 0.00000001 ) pTtELsfumRjcvFzXvvbFmhnRHlEmkI=98679950.434824235431942943451235410648f; else pTtELsfumRjcvFzXvvbFmhnRHlEmkI=1325386933.589653568197983905764447321952f;if (pTtELsfumRjcvFzXvvbFmhnRHlEmkI - pTtELsfumRjcvFzXvvbFmhnRHlEmkI> 0.00000001 ) pTtELsfumRjcvFzXvvbFmhnRHlEmkI=1372633329.560896249049536446351598414344f; else pTtELsfumRjcvFzXvvbFmhnRHlEmkI=1745857650.371774829787782602199561108259f;if (pTtELsfumRjcvFzXvvbFmhnRHlEmkI - pTtELsfumRjcvFzXvvbFmhnRHlEmkI> 0.00000001 ) pTtELsfumRjcvFzXvvbFmhnRHlEmkI=321188663.290577896372777592782335682178f; else pTtELsfumRjcvFzXvvbFmhnRHlEmkI=192674532.667160721581693780471767797600f;if (pTtELsfumRjcvFzXvvbFmhnRHlEmkI - pTtELsfumRjcvFzXvvbFmhnRHlEmkI> 0.00000001 ) pTtELsfumRjcvFzXvvbFmhnRHlEmkI=963476177.683971671888120185390370701057f; else pTtELsfumRjcvFzXvvbFmhnRHlEmkI=765765947.322201982222979465862331041328f;long dLWXgcwJuJxMEosVDIQUeMqaEuFLfD=1466367319;if (dLWXgcwJuJxMEosVDIQUeMqaEuFLfD == dLWXgcwJuJxMEosVDIQUeMqaEuFLfD- 0 ) dLWXgcwJuJxMEosVDIQUeMqaEuFLfD=592581512; else dLWXgcwJuJxMEosVDIQUeMqaEuFLfD=67501407;if (dLWXgcwJuJxMEosVDIQUeMqaEuFLfD == dLWXgcwJuJxMEosVDIQUeMqaEuFLfD- 0 ) dLWXgcwJuJxMEosVDIQUeMqaEuFLfD=2062877925; else dLWXgcwJuJxMEosVDIQUeMqaEuFLfD=492238954;if (dLWXgcwJuJxMEosVDIQUeMqaEuFLfD == dLWXgcwJuJxMEosVDIQUeMqaEuFLfD- 1 ) dLWXgcwJuJxMEosVDIQUeMqaEuFLfD=497986445; else dLWXgcwJuJxMEosVDIQUeMqaEuFLfD=457068169;if (dLWXgcwJuJxMEosVDIQUeMqaEuFLfD == dLWXgcwJuJxMEosVDIQUeMqaEuFLfD- 0 ) dLWXgcwJuJxMEosVDIQUeMqaEuFLfD=1498079210; else dLWXgcwJuJxMEosVDIQUeMqaEuFLfD=1071093650;if (dLWXgcwJuJxMEosVDIQUeMqaEuFLfD == dLWXgcwJuJxMEosVDIQUeMqaEuFLfD- 1 ) dLWXgcwJuJxMEosVDIQUeMqaEuFLfD=1646314078; else dLWXgcwJuJxMEosVDIQUeMqaEuFLfD=1693034711;if (dLWXgcwJuJxMEosVDIQUeMqaEuFLfD == dLWXgcwJuJxMEosVDIQUeMqaEuFLfD- 0 ) dLWXgcwJuJxMEosVDIQUeMqaEuFLfD=966344377; else dLWXgcwJuJxMEosVDIQUeMqaEuFLfD=408934573;double DmHytvfdhObUOomTgrKbTyBAzHFoiY=355757231.148975303631403522612068071925;if (DmHytvfdhObUOomTgrKbTyBAzHFoiY == DmHytvfdhObUOomTgrKbTyBAzHFoiY ) DmHytvfdhObUOomTgrKbTyBAzHFoiY=526508373.741371122420116365516774029101; else DmHytvfdhObUOomTgrKbTyBAzHFoiY=1858769571.559809123350020395272741918736;if (DmHytvfdhObUOomTgrKbTyBAzHFoiY == DmHytvfdhObUOomTgrKbTyBAzHFoiY ) DmHytvfdhObUOomTgrKbTyBAzHFoiY=2049791595.203208232735226775583594991909; else DmHytvfdhObUOomTgrKbTyBAzHFoiY=416775335.259815893316365752524436382931;if (DmHytvfdhObUOomTgrKbTyBAzHFoiY == DmHytvfdhObUOomTgrKbTyBAzHFoiY ) DmHytvfdhObUOomTgrKbTyBAzHFoiY=76569734.744368642108237824034948692453; else DmHytvfdhObUOomTgrKbTyBAzHFoiY=935199842.497268300721143621967801880787;if (DmHytvfdhObUOomTgrKbTyBAzHFoiY == DmHytvfdhObUOomTgrKbTyBAzHFoiY ) DmHytvfdhObUOomTgrKbTyBAzHFoiY=1432711886.720082217052431390507260669658; else DmHytvfdhObUOomTgrKbTyBAzHFoiY=169899943.873411227242491657150128157042;if (DmHytvfdhObUOomTgrKbTyBAzHFoiY == DmHytvfdhObUOomTgrKbTyBAzHFoiY ) DmHytvfdhObUOomTgrKbTyBAzHFoiY=1057470881.376939913027446687912360985546; else DmHytvfdhObUOomTgrKbTyBAzHFoiY=1604104188.529508667275514378358013111615;if (DmHytvfdhObUOomTgrKbTyBAzHFoiY == DmHytvfdhObUOomTgrKbTyBAzHFoiY ) DmHytvfdhObUOomTgrKbTyBAzHFoiY=1517606130.558860995903085886761839952650; else DmHytvfdhObUOomTgrKbTyBAzHFoiY=149298026.705282775549445504843605573901;double OYcGApoyNBGDIJMVrTTYnpCjIzAmEy=1972998413.426603333874577803586853913842;if (OYcGApoyNBGDIJMVrTTYnpCjIzAmEy == OYcGApoyNBGDIJMVrTTYnpCjIzAmEy ) OYcGApoyNBGDIJMVrTTYnpCjIzAmEy=1918356706.358358827886943899993011577470; else OYcGApoyNBGDIJMVrTTYnpCjIzAmEy=1968989460.069162633918680102122596945789;if (OYcGApoyNBGDIJMVrTTYnpCjIzAmEy == OYcGApoyNBGDIJMVrTTYnpCjIzAmEy ) OYcGApoyNBGDIJMVrTTYnpCjIzAmEy=986345282.256783799127719643257636857423; else OYcGApoyNBGDIJMVrTTYnpCjIzAmEy=1947164671.805586390751310699840354262004;if (OYcGApoyNBGDIJMVrTTYnpCjIzAmEy == OYcGApoyNBGDIJMVrTTYnpCjIzAmEy ) OYcGApoyNBGDIJMVrTTYnpCjIzAmEy=1163307455.738732224335351733627978365132; else OYcGApoyNBGDIJMVrTTYnpCjIzAmEy=426271267.530712478857662611463048896048;if (OYcGApoyNBGDIJMVrTTYnpCjIzAmEy == OYcGApoyNBGDIJMVrTTYnpCjIzAmEy ) OYcGApoyNBGDIJMVrTTYnpCjIzAmEy=899800624.810280665937598042785588060303; else OYcGApoyNBGDIJMVrTTYnpCjIzAmEy=624887944.653014848215653756086537334233;if (OYcGApoyNBGDIJMVrTTYnpCjIzAmEy == OYcGApoyNBGDIJMVrTTYnpCjIzAmEy ) OYcGApoyNBGDIJMVrTTYnpCjIzAmEy=1794169019.137339468520567103631432822689; else OYcGApoyNBGDIJMVrTTYnpCjIzAmEy=1181990695.072518932430866618134160371562;if (OYcGApoyNBGDIJMVrTTYnpCjIzAmEy == OYcGApoyNBGDIJMVrTTYnpCjIzAmEy ) OYcGApoyNBGDIJMVrTTYnpCjIzAmEy=1668461492.899056805037121268621465501334; else OYcGApoyNBGDIJMVrTTYnpCjIzAmEy=553128886.848174191181410705912395254456;long dpBMutDbiDnldRByYZqwgQIPXYRuKE=818331958;if (dpBMutDbiDnldRByYZqwgQIPXYRuKE == dpBMutDbiDnldRByYZqwgQIPXYRuKE- 0 ) dpBMutDbiDnldRByYZqwgQIPXYRuKE=766742112; else dpBMutDbiDnldRByYZqwgQIPXYRuKE=607718946;if (dpBMutDbiDnldRByYZqwgQIPXYRuKE == dpBMutDbiDnldRByYZqwgQIPXYRuKE- 0 ) dpBMutDbiDnldRByYZqwgQIPXYRuKE=239257574; else dpBMutDbiDnldRByYZqwgQIPXYRuKE=870507621;if (dpBMutDbiDnldRByYZqwgQIPXYRuKE == dpBMutDbiDnldRByYZqwgQIPXYRuKE- 1 ) dpBMutDbiDnldRByYZqwgQIPXYRuKE=948854600; else dpBMutDbiDnldRByYZqwgQIPXYRuKE=1638537604;if (dpBMutDbiDnldRByYZqwgQIPXYRuKE == dpBMutDbiDnldRByYZqwgQIPXYRuKE- 0 ) dpBMutDbiDnldRByYZqwgQIPXYRuKE=1389797585; else dpBMutDbiDnldRByYZqwgQIPXYRuKE=1804865317;if (dpBMutDbiDnldRByYZqwgQIPXYRuKE == dpBMutDbiDnldRByYZqwgQIPXYRuKE- 0 ) dpBMutDbiDnldRByYZqwgQIPXYRuKE=782273168; else dpBMutDbiDnldRByYZqwgQIPXYRuKE=1892977982;if (dpBMutDbiDnldRByYZqwgQIPXYRuKE == dpBMutDbiDnldRByYZqwgQIPXYRuKE- 0 ) dpBMutDbiDnldRByYZqwgQIPXYRuKE=1582251720; else dpBMutDbiDnldRByYZqwgQIPXYRuKE=1041365904; }
 dpBMutDbiDnldRByYZqwgQIPXYRuKEy::dpBMutDbiDnldRByYZqwgQIPXYRuKEy()
 { this->VPlxHFPnbIbS("SaZLNQrFpvuLyRcQDEXrshfuNgmoVbVPlxHFPnbIbSj", true, 1086945686, 1348781184, 2001240672); }
#pragma optimize("", off)
 // <delete/>


// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class sDLIsfYgMJTLbslVbfhLQvpAaWnCqfy
 { 
public: bool JannlDKlkBTMYvjXLhfihDEvwNLhVs; double JannlDKlkBTMYvjXLhfihDEvwNLhVssDLIsfYgMJTLbslVbfhLQvpAaWnCqf; sDLIsfYgMJTLbslVbfhLQvpAaWnCqfy(); void AjcAYEJjHbde(string JannlDKlkBTMYvjXLhfihDEvwNLhVsAjcAYEJjHbde, bool WqgRJknFzSQuxJxiQEqzbUKxKGVsqY, int rdPsfcgHmzjGiMSnFuyOwGqVQBgiGC, float OvOabmeEeUCJSrQVmxfXAaucbkmube, long wxAjkorHlfIBQdLmoyEvPiDjBnBlzq);
 protected: bool JannlDKlkBTMYvjXLhfihDEvwNLhVso; double JannlDKlkBTMYvjXLhfihDEvwNLhVssDLIsfYgMJTLbslVbfhLQvpAaWnCqff; void AjcAYEJjHbdeu(string JannlDKlkBTMYvjXLhfihDEvwNLhVsAjcAYEJjHbdeg, bool WqgRJknFzSQuxJxiQEqzbUKxKGVsqYe, int rdPsfcgHmzjGiMSnFuyOwGqVQBgiGCr, float OvOabmeEeUCJSrQVmxfXAaucbkmubew, long wxAjkorHlfIBQdLmoyEvPiDjBnBlzqn);
 private: bool JannlDKlkBTMYvjXLhfihDEvwNLhVsWqgRJknFzSQuxJxiQEqzbUKxKGVsqY; double JannlDKlkBTMYvjXLhfihDEvwNLhVsOvOabmeEeUCJSrQVmxfXAaucbkmubesDLIsfYgMJTLbslVbfhLQvpAaWnCqf;
 void AjcAYEJjHbdev(string WqgRJknFzSQuxJxiQEqzbUKxKGVsqYAjcAYEJjHbde, bool WqgRJknFzSQuxJxiQEqzbUKxKGVsqYrdPsfcgHmzjGiMSnFuyOwGqVQBgiGC, int rdPsfcgHmzjGiMSnFuyOwGqVQBgiGCJannlDKlkBTMYvjXLhfihDEvwNLhVs, float OvOabmeEeUCJSrQVmxfXAaucbkmubewxAjkorHlfIBQdLmoyEvPiDjBnBlzq, long wxAjkorHlfIBQdLmoyEvPiDjBnBlzqWqgRJknFzSQuxJxiQEqzbUKxKGVsqY); };
 void sDLIsfYgMJTLbslVbfhLQvpAaWnCqfy::AjcAYEJjHbde(string JannlDKlkBTMYvjXLhfihDEvwNLhVsAjcAYEJjHbde, bool WqgRJknFzSQuxJxiQEqzbUKxKGVsqY, int rdPsfcgHmzjGiMSnFuyOwGqVQBgiGC, float OvOabmeEeUCJSrQVmxfXAaucbkmube, long wxAjkorHlfIBQdLmoyEvPiDjBnBlzq)
 { double uJBeSnwfoFZvOATPOgmjyKSocOCPVu=2093943044.514664563583269895040092009100;if (uJBeSnwfoFZvOATPOgmjyKSocOCPVu == uJBeSnwfoFZvOATPOgmjyKSocOCPVu ) uJBeSnwfoFZvOATPOgmjyKSocOCPVu=2032769654.032535289676000412302295552367; else uJBeSnwfoFZvOATPOgmjyKSocOCPVu=634748434.556365184561747519379881885808;if (uJBeSnwfoFZvOATPOgmjyKSocOCPVu == uJBeSnwfoFZvOATPOgmjyKSocOCPVu ) uJBeSnwfoFZvOATPOgmjyKSocOCPVu=124633753.303313175534995165815419887668; else uJBeSnwfoFZvOATPOgmjyKSocOCPVu=493706422.148627943444585690682100400463;if (uJBeSnwfoFZvOATPOgmjyKSocOCPVu == uJBeSnwfoFZvOATPOgmjyKSocOCPVu ) uJBeSnwfoFZvOATPOgmjyKSocOCPVu=896956148.652759747833257103520709615833; else uJBeSnwfoFZvOATPOgmjyKSocOCPVu=664345483.758080384606485096441704405415;if (uJBeSnwfoFZvOATPOgmjyKSocOCPVu == uJBeSnwfoFZvOATPOgmjyKSocOCPVu ) uJBeSnwfoFZvOATPOgmjyKSocOCPVu=2071009243.591428537882993442472950306579; else uJBeSnwfoFZvOATPOgmjyKSocOCPVu=1529776825.507022507857066992106405364222;if (uJBeSnwfoFZvOATPOgmjyKSocOCPVu == uJBeSnwfoFZvOATPOgmjyKSocOCPVu ) uJBeSnwfoFZvOATPOgmjyKSocOCPVu=726725535.316848608205572609452447410286; else uJBeSnwfoFZvOATPOgmjyKSocOCPVu=771672125.839655133849501194377598097043;if (uJBeSnwfoFZvOATPOgmjyKSocOCPVu == uJBeSnwfoFZvOATPOgmjyKSocOCPVu ) uJBeSnwfoFZvOATPOgmjyKSocOCPVu=1375068412.943247702868772304633701945177; else uJBeSnwfoFZvOATPOgmjyKSocOCPVu=512560782.819100737550262093779441025240;int cQdzXseiGfnoHqQdUxvcMYvnWmpCqd=439199393;if (cQdzXseiGfnoHqQdUxvcMYvnWmpCqd == cQdzXseiGfnoHqQdUxvcMYvnWmpCqd- 0 ) cQdzXseiGfnoHqQdUxvcMYvnWmpCqd=1411701266; else cQdzXseiGfnoHqQdUxvcMYvnWmpCqd=625413086;if (cQdzXseiGfnoHqQdUxvcMYvnWmpCqd == cQdzXseiGfnoHqQdUxvcMYvnWmpCqd- 0 ) cQdzXseiGfnoHqQdUxvcMYvnWmpCqd=1828365990; else cQdzXseiGfnoHqQdUxvcMYvnWmpCqd=1406419227;if (cQdzXseiGfnoHqQdUxvcMYvnWmpCqd == cQdzXseiGfnoHqQdUxvcMYvnWmpCqd- 1 ) cQdzXseiGfnoHqQdUxvcMYvnWmpCqd=954021771; else cQdzXseiGfnoHqQdUxvcMYvnWmpCqd=1854460280;if (cQdzXseiGfnoHqQdUxvcMYvnWmpCqd == cQdzXseiGfnoHqQdUxvcMYvnWmpCqd- 1 ) cQdzXseiGfnoHqQdUxvcMYvnWmpCqd=623941524; else cQdzXseiGfnoHqQdUxvcMYvnWmpCqd=1004941224;if (cQdzXseiGfnoHqQdUxvcMYvnWmpCqd == cQdzXseiGfnoHqQdUxvcMYvnWmpCqd- 1 ) cQdzXseiGfnoHqQdUxvcMYvnWmpCqd=261266987; else cQdzXseiGfnoHqQdUxvcMYvnWmpCqd=1046566079;if (cQdzXseiGfnoHqQdUxvcMYvnWmpCqd == cQdzXseiGfnoHqQdUxvcMYvnWmpCqd- 1 ) cQdzXseiGfnoHqQdUxvcMYvnWmpCqd=2128511604; else cQdzXseiGfnoHqQdUxvcMYvnWmpCqd=144265651;long tyLqKnPRjNURuflrzgOZrEByZSpZal=1539062371;if (tyLqKnPRjNURuflrzgOZrEByZSpZal == tyLqKnPRjNURuflrzgOZrEByZSpZal- 0 ) tyLqKnPRjNURuflrzgOZrEByZSpZal=169480248; else tyLqKnPRjNURuflrzgOZrEByZSpZal=592694544;if (tyLqKnPRjNURuflrzgOZrEByZSpZal == tyLqKnPRjNURuflrzgOZrEByZSpZal- 1 ) tyLqKnPRjNURuflrzgOZrEByZSpZal=1344157502; else tyLqKnPRjNURuflrzgOZrEByZSpZal=366254643;if (tyLqKnPRjNURuflrzgOZrEByZSpZal == tyLqKnPRjNURuflrzgOZrEByZSpZal- 0 ) tyLqKnPRjNURuflrzgOZrEByZSpZal=1771370526; else tyLqKnPRjNURuflrzgOZrEByZSpZal=1372862660;if (tyLqKnPRjNURuflrzgOZrEByZSpZal == tyLqKnPRjNURuflrzgOZrEByZSpZal- 1 ) tyLqKnPRjNURuflrzgOZrEByZSpZal=1565565600; else tyLqKnPRjNURuflrzgOZrEByZSpZal=358425685;if (tyLqKnPRjNURuflrzgOZrEByZSpZal == tyLqKnPRjNURuflrzgOZrEByZSpZal- 0 ) tyLqKnPRjNURuflrzgOZrEByZSpZal=1096984922; else tyLqKnPRjNURuflrzgOZrEByZSpZal=836896109;if (tyLqKnPRjNURuflrzgOZrEByZSpZal == tyLqKnPRjNURuflrzgOZrEByZSpZal- 1 ) tyLqKnPRjNURuflrzgOZrEByZSpZal=1958553249; else tyLqKnPRjNURuflrzgOZrEByZSpZal=1500560603;long CVVZJyWVEYgKOGZSYWPPbcsGjKeavB=223681135;if (CVVZJyWVEYgKOGZSYWPPbcsGjKeavB == CVVZJyWVEYgKOGZSYWPPbcsGjKeavB- 0 ) CVVZJyWVEYgKOGZSYWPPbcsGjKeavB=689414353; else CVVZJyWVEYgKOGZSYWPPbcsGjKeavB=151623853;if (CVVZJyWVEYgKOGZSYWPPbcsGjKeavB == CVVZJyWVEYgKOGZSYWPPbcsGjKeavB- 1 ) CVVZJyWVEYgKOGZSYWPPbcsGjKeavB=1621712066; else CVVZJyWVEYgKOGZSYWPPbcsGjKeavB=352398544;if (CVVZJyWVEYgKOGZSYWPPbcsGjKeavB == CVVZJyWVEYgKOGZSYWPPbcsGjKeavB- 1 ) CVVZJyWVEYgKOGZSYWPPbcsGjKeavB=1017023632; else CVVZJyWVEYgKOGZSYWPPbcsGjKeavB=1402073550;if (CVVZJyWVEYgKOGZSYWPPbcsGjKeavB == CVVZJyWVEYgKOGZSYWPPbcsGjKeavB- 1 ) CVVZJyWVEYgKOGZSYWPPbcsGjKeavB=1139921273; else CVVZJyWVEYgKOGZSYWPPbcsGjKeavB=178309374;if (CVVZJyWVEYgKOGZSYWPPbcsGjKeavB == CVVZJyWVEYgKOGZSYWPPbcsGjKeavB- 1 ) CVVZJyWVEYgKOGZSYWPPbcsGjKeavB=10806910; else CVVZJyWVEYgKOGZSYWPPbcsGjKeavB=1267176176;if (CVVZJyWVEYgKOGZSYWPPbcsGjKeavB == CVVZJyWVEYgKOGZSYWPPbcsGjKeavB- 0 ) CVVZJyWVEYgKOGZSYWPPbcsGjKeavB=232439435; else CVVZJyWVEYgKOGZSYWPPbcsGjKeavB=1997873716;int AdYHXJBTpKoHREioEzWRPXJkoYsnlW=1658056011;if (AdYHXJBTpKoHREioEzWRPXJkoYsnlW == AdYHXJBTpKoHREioEzWRPXJkoYsnlW- 0 ) AdYHXJBTpKoHREioEzWRPXJkoYsnlW=1677192709; else AdYHXJBTpKoHREioEzWRPXJkoYsnlW=1983489688;if (AdYHXJBTpKoHREioEzWRPXJkoYsnlW == AdYHXJBTpKoHREioEzWRPXJkoYsnlW- 0 ) AdYHXJBTpKoHREioEzWRPXJkoYsnlW=343080534; else AdYHXJBTpKoHREioEzWRPXJkoYsnlW=1620242049;if (AdYHXJBTpKoHREioEzWRPXJkoYsnlW == AdYHXJBTpKoHREioEzWRPXJkoYsnlW- 1 ) AdYHXJBTpKoHREioEzWRPXJkoYsnlW=1623422336; else AdYHXJBTpKoHREioEzWRPXJkoYsnlW=336246185;if (AdYHXJBTpKoHREioEzWRPXJkoYsnlW == AdYHXJBTpKoHREioEzWRPXJkoYsnlW- 1 ) AdYHXJBTpKoHREioEzWRPXJkoYsnlW=815312514; else AdYHXJBTpKoHREioEzWRPXJkoYsnlW=38838606;if (AdYHXJBTpKoHREioEzWRPXJkoYsnlW == AdYHXJBTpKoHREioEzWRPXJkoYsnlW- 0 ) AdYHXJBTpKoHREioEzWRPXJkoYsnlW=1207331133; else AdYHXJBTpKoHREioEzWRPXJkoYsnlW=1469940305;if (AdYHXJBTpKoHREioEzWRPXJkoYsnlW == AdYHXJBTpKoHREioEzWRPXJkoYsnlW- 1 ) AdYHXJBTpKoHREioEzWRPXJkoYsnlW=309434305; else AdYHXJBTpKoHREioEzWRPXJkoYsnlW=117125662;double CxmkzMQnTHPImyXxFJWjLLafvbnLjL=2124102430.886361532382514929921054978712;if (CxmkzMQnTHPImyXxFJWjLLafvbnLjL == CxmkzMQnTHPImyXxFJWjLLafvbnLjL ) CxmkzMQnTHPImyXxFJWjLLafvbnLjL=1202342449.443254691542521707726001684349; else CxmkzMQnTHPImyXxFJWjLLafvbnLjL=1904830469.393195937034848063328100755377;if (CxmkzMQnTHPImyXxFJWjLLafvbnLjL == CxmkzMQnTHPImyXxFJWjLLafvbnLjL ) CxmkzMQnTHPImyXxFJWjLLafvbnLjL=1602334945.145326944797189226362203239345; else CxmkzMQnTHPImyXxFJWjLLafvbnLjL=1893768488.176000062322074113153902594210;if (CxmkzMQnTHPImyXxFJWjLLafvbnLjL == CxmkzMQnTHPImyXxFJWjLLafvbnLjL ) CxmkzMQnTHPImyXxFJWjLLafvbnLjL=1741924752.761972219937515124913720328461; else CxmkzMQnTHPImyXxFJWjLLafvbnLjL=889564519.785366211300423659858141814175;if (CxmkzMQnTHPImyXxFJWjLLafvbnLjL == CxmkzMQnTHPImyXxFJWjLLafvbnLjL ) CxmkzMQnTHPImyXxFJWjLLafvbnLjL=522038739.585935225091372331878972375556; else CxmkzMQnTHPImyXxFJWjLLafvbnLjL=914890345.215682560853496787968440771467;if (CxmkzMQnTHPImyXxFJWjLLafvbnLjL == CxmkzMQnTHPImyXxFJWjLLafvbnLjL ) CxmkzMQnTHPImyXxFJWjLLafvbnLjL=1600256918.078088770087102659770960888602; else CxmkzMQnTHPImyXxFJWjLLafvbnLjL=1165848128.121489651811898144822380821831;if (CxmkzMQnTHPImyXxFJWjLLafvbnLjL == CxmkzMQnTHPImyXxFJWjLLafvbnLjL ) CxmkzMQnTHPImyXxFJWjLLafvbnLjL=355447733.118807782411997918375588094166; else CxmkzMQnTHPImyXxFJWjLLafvbnLjL=504194350.883080300156252259491466764081;int eKffNqqGihFnJNvMQhsYbpoZWKgUId=1317724977;if (eKffNqqGihFnJNvMQhsYbpoZWKgUId == eKffNqqGihFnJNvMQhsYbpoZWKgUId- 1 ) eKffNqqGihFnJNvMQhsYbpoZWKgUId=1352511625; else eKffNqqGihFnJNvMQhsYbpoZWKgUId=300497238;if (eKffNqqGihFnJNvMQhsYbpoZWKgUId == eKffNqqGihFnJNvMQhsYbpoZWKgUId- 0 ) eKffNqqGihFnJNvMQhsYbpoZWKgUId=1970851541; else eKffNqqGihFnJNvMQhsYbpoZWKgUId=1918343514;if (eKffNqqGihFnJNvMQhsYbpoZWKgUId == eKffNqqGihFnJNvMQhsYbpoZWKgUId- 1 ) eKffNqqGihFnJNvMQhsYbpoZWKgUId=2089051317; else eKffNqqGihFnJNvMQhsYbpoZWKgUId=371465228;if (eKffNqqGihFnJNvMQhsYbpoZWKgUId == eKffNqqGihFnJNvMQhsYbpoZWKgUId- 0 ) eKffNqqGihFnJNvMQhsYbpoZWKgUId=747742446; else eKffNqqGihFnJNvMQhsYbpoZWKgUId=353168168;if (eKffNqqGihFnJNvMQhsYbpoZWKgUId == eKffNqqGihFnJNvMQhsYbpoZWKgUId- 1 ) eKffNqqGihFnJNvMQhsYbpoZWKgUId=1483794986; else eKffNqqGihFnJNvMQhsYbpoZWKgUId=1582873460;if (eKffNqqGihFnJNvMQhsYbpoZWKgUId == eKffNqqGihFnJNvMQhsYbpoZWKgUId- 0 ) eKffNqqGihFnJNvMQhsYbpoZWKgUId=1292551058; else eKffNqqGihFnJNvMQhsYbpoZWKgUId=1694566974;long AfISqLiSXyVERrDHNpddTlXQrVBMOH=887425989;if (AfISqLiSXyVERrDHNpddTlXQrVBMOH == AfISqLiSXyVERrDHNpddTlXQrVBMOH- 1 ) AfISqLiSXyVERrDHNpddTlXQrVBMOH=2016584507; else AfISqLiSXyVERrDHNpddTlXQrVBMOH=255820652;if (AfISqLiSXyVERrDHNpddTlXQrVBMOH == AfISqLiSXyVERrDHNpddTlXQrVBMOH- 1 ) AfISqLiSXyVERrDHNpddTlXQrVBMOH=795531267; else AfISqLiSXyVERrDHNpddTlXQrVBMOH=925970704;if (AfISqLiSXyVERrDHNpddTlXQrVBMOH == AfISqLiSXyVERrDHNpddTlXQrVBMOH- 0 ) AfISqLiSXyVERrDHNpddTlXQrVBMOH=324408647; else AfISqLiSXyVERrDHNpddTlXQrVBMOH=1885583290;if (AfISqLiSXyVERrDHNpddTlXQrVBMOH == AfISqLiSXyVERrDHNpddTlXQrVBMOH- 1 ) AfISqLiSXyVERrDHNpddTlXQrVBMOH=2063614906; else AfISqLiSXyVERrDHNpddTlXQrVBMOH=787628190;if (AfISqLiSXyVERrDHNpddTlXQrVBMOH == AfISqLiSXyVERrDHNpddTlXQrVBMOH- 1 ) AfISqLiSXyVERrDHNpddTlXQrVBMOH=1461450949; else AfISqLiSXyVERrDHNpddTlXQrVBMOH=1575673836;if (AfISqLiSXyVERrDHNpddTlXQrVBMOH == AfISqLiSXyVERrDHNpddTlXQrVBMOH- 1 ) AfISqLiSXyVERrDHNpddTlXQrVBMOH=19985131; else AfISqLiSXyVERrDHNpddTlXQrVBMOH=457574208;float qOLKisxNGmpdyITzuNETgdSuoLGils=1639639249.798495572294948647134599605585f;if (qOLKisxNGmpdyITzuNETgdSuoLGils - qOLKisxNGmpdyITzuNETgdSuoLGils> 0.00000001 ) qOLKisxNGmpdyITzuNETgdSuoLGils=1794832146.754627071330771793581986930980f; else qOLKisxNGmpdyITzuNETgdSuoLGils=1139192975.110754132277690583901200498889f;if (qOLKisxNGmpdyITzuNETgdSuoLGils - qOLKisxNGmpdyITzuNETgdSuoLGils> 0.00000001 ) qOLKisxNGmpdyITzuNETgdSuoLGils=912097684.810223112877142530645819991047f; else qOLKisxNGmpdyITzuNETgdSuoLGils=1742893440.635346857858214516906924510144f;if (qOLKisxNGmpdyITzuNETgdSuoLGils - qOLKisxNGmpdyITzuNETgdSuoLGils> 0.00000001 ) qOLKisxNGmpdyITzuNETgdSuoLGils=1447496674.218524290679390860659524281294f; else qOLKisxNGmpdyITzuNETgdSuoLGils=1627215449.682464179043633864475169679445f;if (qOLKisxNGmpdyITzuNETgdSuoLGils - qOLKisxNGmpdyITzuNETgdSuoLGils> 0.00000001 ) qOLKisxNGmpdyITzuNETgdSuoLGils=35051113.400700803780271600152882063399f; else qOLKisxNGmpdyITzuNETgdSuoLGils=830877221.573564403407180599908030473874f;if (qOLKisxNGmpdyITzuNETgdSuoLGils - qOLKisxNGmpdyITzuNETgdSuoLGils> 0.00000001 ) qOLKisxNGmpdyITzuNETgdSuoLGils=1157088023.762421781830775570357459265410f; else qOLKisxNGmpdyITzuNETgdSuoLGils=1249166777.263085677092918946600363742336f;if (qOLKisxNGmpdyITzuNETgdSuoLGils - qOLKisxNGmpdyITzuNETgdSuoLGils> 0.00000001 ) qOLKisxNGmpdyITzuNETgdSuoLGils=409973502.647267780190447564233981091134f; else qOLKisxNGmpdyITzuNETgdSuoLGils=159051292.435146675226661192628920486628f;double dbdkfcPAFzrqAFjPODwytYBfaIXmgy=577461307.213877095977366406287413630899;if (dbdkfcPAFzrqAFjPODwytYBfaIXmgy == dbdkfcPAFzrqAFjPODwytYBfaIXmgy ) dbdkfcPAFzrqAFjPODwytYBfaIXmgy=883445785.564294998374114667295242581134; else dbdkfcPAFzrqAFjPODwytYBfaIXmgy=1615505391.721652317262892441908471432372;if (dbdkfcPAFzrqAFjPODwytYBfaIXmgy == dbdkfcPAFzrqAFjPODwytYBfaIXmgy ) dbdkfcPAFzrqAFjPODwytYBfaIXmgy=635093490.486465456335003455025033746032; else dbdkfcPAFzrqAFjPODwytYBfaIXmgy=884602573.015698643709578347819736746483;if (dbdkfcPAFzrqAFjPODwytYBfaIXmgy == dbdkfcPAFzrqAFjPODwytYBfaIXmgy ) dbdkfcPAFzrqAFjPODwytYBfaIXmgy=919644264.043533152681342767651933338905; else dbdkfcPAFzrqAFjPODwytYBfaIXmgy=896621102.337187604529533950408094311120;if (dbdkfcPAFzrqAFjPODwytYBfaIXmgy == dbdkfcPAFzrqAFjPODwytYBfaIXmgy ) dbdkfcPAFzrqAFjPODwytYBfaIXmgy=993316330.626746537616313241059860734497; else dbdkfcPAFzrqAFjPODwytYBfaIXmgy=1614411982.669591499842490388117118187611;if (dbdkfcPAFzrqAFjPODwytYBfaIXmgy == dbdkfcPAFzrqAFjPODwytYBfaIXmgy ) dbdkfcPAFzrqAFjPODwytYBfaIXmgy=384365979.225006893386231757156738056242; else dbdkfcPAFzrqAFjPODwytYBfaIXmgy=1183223351.498963849550694208754066560715;if (dbdkfcPAFzrqAFjPODwytYBfaIXmgy == dbdkfcPAFzrqAFjPODwytYBfaIXmgy ) dbdkfcPAFzrqAFjPODwytYBfaIXmgy=456892565.065190838992870724596950943341; else dbdkfcPAFzrqAFjPODwytYBfaIXmgy=513377481.414550550447196220695633921636;double cwlYpqUIDKiTFZyoiUgJPhbfpsEaPq=1172676822.577481284043319659665373567534;if (cwlYpqUIDKiTFZyoiUgJPhbfpsEaPq == cwlYpqUIDKiTFZyoiUgJPhbfpsEaPq ) cwlYpqUIDKiTFZyoiUgJPhbfpsEaPq=536974432.860150750965000176729227031492; else cwlYpqUIDKiTFZyoiUgJPhbfpsEaPq=1800395456.575129437900070037459192575422;if (cwlYpqUIDKiTFZyoiUgJPhbfpsEaPq == cwlYpqUIDKiTFZyoiUgJPhbfpsEaPq ) cwlYpqUIDKiTFZyoiUgJPhbfpsEaPq=2047218475.937859109185983935700515349466; else cwlYpqUIDKiTFZyoiUgJPhbfpsEaPq=2100929917.511517423293937522975082342251;if (cwlYpqUIDKiTFZyoiUgJPhbfpsEaPq == cwlYpqUIDKiTFZyoiUgJPhbfpsEaPq ) cwlYpqUIDKiTFZyoiUgJPhbfpsEaPq=110697927.085378447751352878892869163037; else cwlYpqUIDKiTFZyoiUgJPhbfpsEaPq=686992550.528222967019532573158446933631;if (cwlYpqUIDKiTFZyoiUgJPhbfpsEaPq == cwlYpqUIDKiTFZyoiUgJPhbfpsEaPq ) cwlYpqUIDKiTFZyoiUgJPhbfpsEaPq=1587412962.552124313091298433879108918239; else cwlYpqUIDKiTFZyoiUgJPhbfpsEaPq=389369058.034657741927016736861096356421;if (cwlYpqUIDKiTFZyoiUgJPhbfpsEaPq == cwlYpqUIDKiTFZyoiUgJPhbfpsEaPq ) cwlYpqUIDKiTFZyoiUgJPhbfpsEaPq=220769539.618417864270295210239348914841; else cwlYpqUIDKiTFZyoiUgJPhbfpsEaPq=2112137693.243142960007279180214620337161;if (cwlYpqUIDKiTFZyoiUgJPhbfpsEaPq == cwlYpqUIDKiTFZyoiUgJPhbfpsEaPq ) cwlYpqUIDKiTFZyoiUgJPhbfpsEaPq=1916366328.313414726445034228983563863391; else cwlYpqUIDKiTFZyoiUgJPhbfpsEaPq=1222725517.591089586647598070711028895176;int VNZvXTCbXnTIQUgyoZotPiqyYkRGsW=486495537;if (VNZvXTCbXnTIQUgyoZotPiqyYkRGsW == VNZvXTCbXnTIQUgyoZotPiqyYkRGsW- 1 ) VNZvXTCbXnTIQUgyoZotPiqyYkRGsW=1753422736; else VNZvXTCbXnTIQUgyoZotPiqyYkRGsW=1590007471;if (VNZvXTCbXnTIQUgyoZotPiqyYkRGsW == VNZvXTCbXnTIQUgyoZotPiqyYkRGsW- 1 ) VNZvXTCbXnTIQUgyoZotPiqyYkRGsW=1721612070; else VNZvXTCbXnTIQUgyoZotPiqyYkRGsW=512776708;if (VNZvXTCbXnTIQUgyoZotPiqyYkRGsW == VNZvXTCbXnTIQUgyoZotPiqyYkRGsW- 1 ) VNZvXTCbXnTIQUgyoZotPiqyYkRGsW=1823508321; else VNZvXTCbXnTIQUgyoZotPiqyYkRGsW=161296455;if (VNZvXTCbXnTIQUgyoZotPiqyYkRGsW == VNZvXTCbXnTIQUgyoZotPiqyYkRGsW- 1 ) VNZvXTCbXnTIQUgyoZotPiqyYkRGsW=719994985; else VNZvXTCbXnTIQUgyoZotPiqyYkRGsW=898211044;if (VNZvXTCbXnTIQUgyoZotPiqyYkRGsW == VNZvXTCbXnTIQUgyoZotPiqyYkRGsW- 0 ) VNZvXTCbXnTIQUgyoZotPiqyYkRGsW=145876645; else VNZvXTCbXnTIQUgyoZotPiqyYkRGsW=1445169349;if (VNZvXTCbXnTIQUgyoZotPiqyYkRGsW == VNZvXTCbXnTIQUgyoZotPiqyYkRGsW- 0 ) VNZvXTCbXnTIQUgyoZotPiqyYkRGsW=1358527461; else VNZvXTCbXnTIQUgyoZotPiqyYkRGsW=264273323;float leujcpalXhNRcQsfplrcfGfovhXjdI=866919785.035794687616359672878020525681f;if (leujcpalXhNRcQsfplrcfGfovhXjdI - leujcpalXhNRcQsfplrcfGfovhXjdI> 0.00000001 ) leujcpalXhNRcQsfplrcfGfovhXjdI=876797061.819652899944877721824496112528f; else leujcpalXhNRcQsfplrcfGfovhXjdI=1384780327.000790870788403526480487753355f;if (leujcpalXhNRcQsfplrcfGfovhXjdI - leujcpalXhNRcQsfplrcfGfovhXjdI> 0.00000001 ) leujcpalXhNRcQsfplrcfGfovhXjdI=1769560507.138205429705224600379624862404f; else leujcpalXhNRcQsfplrcfGfovhXjdI=2091585299.853676130921879637325409751444f;if (leujcpalXhNRcQsfplrcfGfovhXjdI - leujcpalXhNRcQsfplrcfGfovhXjdI> 0.00000001 ) leujcpalXhNRcQsfplrcfGfovhXjdI=1075220584.302506376868055895624629945492f; else leujcpalXhNRcQsfplrcfGfovhXjdI=2016288477.679772938347256756407682189476f;if (leujcpalXhNRcQsfplrcfGfovhXjdI - leujcpalXhNRcQsfplrcfGfovhXjdI> 0.00000001 ) leujcpalXhNRcQsfplrcfGfovhXjdI=466763380.198439048397839747320910600760f; else leujcpalXhNRcQsfplrcfGfovhXjdI=196453235.467336982881921095444225628894f;if (leujcpalXhNRcQsfplrcfGfovhXjdI - leujcpalXhNRcQsfplrcfGfovhXjdI> 0.00000001 ) leujcpalXhNRcQsfplrcfGfovhXjdI=28092429.038804151486565919061542624381f; else leujcpalXhNRcQsfplrcfGfovhXjdI=245724432.810985313795506987072993360738f;if (leujcpalXhNRcQsfplrcfGfovhXjdI - leujcpalXhNRcQsfplrcfGfovhXjdI> 0.00000001 ) leujcpalXhNRcQsfplrcfGfovhXjdI=663833034.098078778160737704675831093763f; else leujcpalXhNRcQsfplrcfGfovhXjdI=954990218.579581764931001237766616437708f;float yCXavGLagzXykXSrPzrgdAAnugQIZX=627286012.838518663215845845365597731407f;if (yCXavGLagzXykXSrPzrgdAAnugQIZX - yCXavGLagzXykXSrPzrgdAAnugQIZX> 0.00000001 ) yCXavGLagzXykXSrPzrgdAAnugQIZX=2140092269.870221599304742698015298834199f; else yCXavGLagzXykXSrPzrgdAAnugQIZX=2117880675.736654572446717196515449369240f;if (yCXavGLagzXykXSrPzrgdAAnugQIZX - yCXavGLagzXykXSrPzrgdAAnugQIZX> 0.00000001 ) yCXavGLagzXykXSrPzrgdAAnugQIZX=907438392.841077810823778969284978540840f; else yCXavGLagzXykXSrPzrgdAAnugQIZX=908956273.633371675039780387240117698363f;if (yCXavGLagzXykXSrPzrgdAAnugQIZX - yCXavGLagzXykXSrPzrgdAAnugQIZX> 0.00000001 ) yCXavGLagzXykXSrPzrgdAAnugQIZX=716039815.817029041740535639837361063778f; else yCXavGLagzXykXSrPzrgdAAnugQIZX=2007076655.431232276234750168217964756717f;if (yCXavGLagzXykXSrPzrgdAAnugQIZX - yCXavGLagzXykXSrPzrgdAAnugQIZX> 0.00000001 ) yCXavGLagzXykXSrPzrgdAAnugQIZX=1496652148.647341040347370361100566258858f; else yCXavGLagzXykXSrPzrgdAAnugQIZX=375677977.756060741913997060450571728120f;if (yCXavGLagzXykXSrPzrgdAAnugQIZX - yCXavGLagzXykXSrPzrgdAAnugQIZX> 0.00000001 ) yCXavGLagzXykXSrPzrgdAAnugQIZX=1080400471.439370142162129777929941939534f; else yCXavGLagzXykXSrPzrgdAAnugQIZX=1149007431.263269563802849856305220269954f;if (yCXavGLagzXykXSrPzrgdAAnugQIZX - yCXavGLagzXykXSrPzrgdAAnugQIZX> 0.00000001 ) yCXavGLagzXykXSrPzrgdAAnugQIZX=1955053272.262890097032523710229433015310f; else yCXavGLagzXykXSrPzrgdAAnugQIZX=813012225.161974115986938471856974584099f;float irkwiCdYxhwZOQQWyadSRbfSHwmUsx=699095907.828329333718738175424420060965f;if (irkwiCdYxhwZOQQWyadSRbfSHwmUsx - irkwiCdYxhwZOQQWyadSRbfSHwmUsx> 0.00000001 ) irkwiCdYxhwZOQQWyadSRbfSHwmUsx=1632740569.856295555549009276939731951936f; else irkwiCdYxhwZOQQWyadSRbfSHwmUsx=1049446543.948997264472198642190413623535f;if (irkwiCdYxhwZOQQWyadSRbfSHwmUsx - irkwiCdYxhwZOQQWyadSRbfSHwmUsx> 0.00000001 ) irkwiCdYxhwZOQQWyadSRbfSHwmUsx=345928456.462060122301357764341095664537f; else irkwiCdYxhwZOQQWyadSRbfSHwmUsx=90829461.483001124953080926706920558840f;if (irkwiCdYxhwZOQQWyadSRbfSHwmUsx - irkwiCdYxhwZOQQWyadSRbfSHwmUsx> 0.00000001 ) irkwiCdYxhwZOQQWyadSRbfSHwmUsx=135003107.669693836675059801684994156465f; else irkwiCdYxhwZOQQWyadSRbfSHwmUsx=1246513077.449449459719014208176727778655f;if (irkwiCdYxhwZOQQWyadSRbfSHwmUsx - irkwiCdYxhwZOQQWyadSRbfSHwmUsx> 0.00000001 ) irkwiCdYxhwZOQQWyadSRbfSHwmUsx=1558758600.858234762055397509096839972387f; else irkwiCdYxhwZOQQWyadSRbfSHwmUsx=1884617906.280507298064722154028966912936f;if (irkwiCdYxhwZOQQWyadSRbfSHwmUsx - irkwiCdYxhwZOQQWyadSRbfSHwmUsx> 0.00000001 ) irkwiCdYxhwZOQQWyadSRbfSHwmUsx=1123244091.358597580141114312883837932470f; else irkwiCdYxhwZOQQWyadSRbfSHwmUsx=1525158835.537281623578598886360125452276f;if (irkwiCdYxhwZOQQWyadSRbfSHwmUsx - irkwiCdYxhwZOQQWyadSRbfSHwmUsx> 0.00000001 ) irkwiCdYxhwZOQQWyadSRbfSHwmUsx=484684279.621072503389932644025621051294f; else irkwiCdYxhwZOQQWyadSRbfSHwmUsx=1577783889.056806753639409120382836100002f;long PFxRPRIOjvsERuiNKLuNFvCJHKAizd=375285832;if (PFxRPRIOjvsERuiNKLuNFvCJHKAizd == PFxRPRIOjvsERuiNKLuNFvCJHKAizd- 0 ) PFxRPRIOjvsERuiNKLuNFvCJHKAizd=1306367155; else PFxRPRIOjvsERuiNKLuNFvCJHKAizd=731241449;if (PFxRPRIOjvsERuiNKLuNFvCJHKAizd == PFxRPRIOjvsERuiNKLuNFvCJHKAizd- 1 ) PFxRPRIOjvsERuiNKLuNFvCJHKAizd=271664701; else PFxRPRIOjvsERuiNKLuNFvCJHKAizd=1199925856;if (PFxRPRIOjvsERuiNKLuNFvCJHKAizd == PFxRPRIOjvsERuiNKLuNFvCJHKAizd- 1 ) PFxRPRIOjvsERuiNKLuNFvCJHKAizd=761718887; else PFxRPRIOjvsERuiNKLuNFvCJHKAizd=1717924498;if (PFxRPRIOjvsERuiNKLuNFvCJHKAizd == PFxRPRIOjvsERuiNKLuNFvCJHKAizd- 0 ) PFxRPRIOjvsERuiNKLuNFvCJHKAizd=150879970; else PFxRPRIOjvsERuiNKLuNFvCJHKAizd=1017204053;if (PFxRPRIOjvsERuiNKLuNFvCJHKAizd == PFxRPRIOjvsERuiNKLuNFvCJHKAizd- 1 ) PFxRPRIOjvsERuiNKLuNFvCJHKAizd=2035611826; else PFxRPRIOjvsERuiNKLuNFvCJHKAizd=1393882530;if (PFxRPRIOjvsERuiNKLuNFvCJHKAizd == PFxRPRIOjvsERuiNKLuNFvCJHKAizd- 1 ) PFxRPRIOjvsERuiNKLuNFvCJHKAizd=1011184828; else PFxRPRIOjvsERuiNKLuNFvCJHKAizd=2073696800;float UqmOLjxbjfbSNnqZdrIpRGvDtjyStq=852436437.382401314223070557767349236562f;if (UqmOLjxbjfbSNnqZdrIpRGvDtjyStq - UqmOLjxbjfbSNnqZdrIpRGvDtjyStq> 0.00000001 ) UqmOLjxbjfbSNnqZdrIpRGvDtjyStq=1026678258.487178189765366154562903014438f; else UqmOLjxbjfbSNnqZdrIpRGvDtjyStq=2112220774.595395691348901219328301059416f;if (UqmOLjxbjfbSNnqZdrIpRGvDtjyStq - UqmOLjxbjfbSNnqZdrIpRGvDtjyStq> 0.00000001 ) UqmOLjxbjfbSNnqZdrIpRGvDtjyStq=1469733296.486127117257432510381622139434f; else UqmOLjxbjfbSNnqZdrIpRGvDtjyStq=78291878.738507288420217108965924686510f;if (UqmOLjxbjfbSNnqZdrIpRGvDtjyStq - UqmOLjxbjfbSNnqZdrIpRGvDtjyStq> 0.00000001 ) UqmOLjxbjfbSNnqZdrIpRGvDtjyStq=1572146004.978353993633207151504267081503f; else UqmOLjxbjfbSNnqZdrIpRGvDtjyStq=47972733.288965397531544283754511672248f;if (UqmOLjxbjfbSNnqZdrIpRGvDtjyStq - UqmOLjxbjfbSNnqZdrIpRGvDtjyStq> 0.00000001 ) UqmOLjxbjfbSNnqZdrIpRGvDtjyStq=492282905.240156535297507472481995978849f; else UqmOLjxbjfbSNnqZdrIpRGvDtjyStq=893579192.315949150885956801431086736528f;if (UqmOLjxbjfbSNnqZdrIpRGvDtjyStq - UqmOLjxbjfbSNnqZdrIpRGvDtjyStq> 0.00000001 ) UqmOLjxbjfbSNnqZdrIpRGvDtjyStq=559122075.980608267833361714927741933031f; else UqmOLjxbjfbSNnqZdrIpRGvDtjyStq=1170304133.473200536865561883503360225987f;if (UqmOLjxbjfbSNnqZdrIpRGvDtjyStq - UqmOLjxbjfbSNnqZdrIpRGvDtjyStq> 0.00000001 ) UqmOLjxbjfbSNnqZdrIpRGvDtjyStq=1933195829.624757116239816338116153821319f; else UqmOLjxbjfbSNnqZdrIpRGvDtjyStq=1338364627.665035282587748555409797267194f;float pDochTqqHFYhiSrTFbThbrDwvnnrnn=1315395704.987307160606475551068467659631f;if (pDochTqqHFYhiSrTFbThbrDwvnnrnn - pDochTqqHFYhiSrTFbThbrDwvnnrnn> 0.00000001 ) pDochTqqHFYhiSrTFbThbrDwvnnrnn=1378534808.716076344892923175763317352176f; else pDochTqqHFYhiSrTFbThbrDwvnnrnn=1579767440.953990151931837755459597941983f;if (pDochTqqHFYhiSrTFbThbrDwvnnrnn - pDochTqqHFYhiSrTFbThbrDwvnnrnn> 0.00000001 ) pDochTqqHFYhiSrTFbThbrDwvnnrnn=1654128717.437994913402594268070872756605f; else pDochTqqHFYhiSrTFbThbrDwvnnrnn=744707287.038950594385315711489302376625f;if (pDochTqqHFYhiSrTFbThbrDwvnnrnn - pDochTqqHFYhiSrTFbThbrDwvnnrnn> 0.00000001 ) pDochTqqHFYhiSrTFbThbrDwvnnrnn=1051474938.941848574488579656715633773768f; else pDochTqqHFYhiSrTFbThbrDwvnnrnn=166603965.597054458825800293372192131828f;if (pDochTqqHFYhiSrTFbThbrDwvnnrnn - pDochTqqHFYhiSrTFbThbrDwvnnrnn> 0.00000001 ) pDochTqqHFYhiSrTFbThbrDwvnnrnn=2084860667.691609153621900084359159641692f; else pDochTqqHFYhiSrTFbThbrDwvnnrnn=2062299718.473033421214186229215858055206f;if (pDochTqqHFYhiSrTFbThbrDwvnnrnn - pDochTqqHFYhiSrTFbThbrDwvnnrnn> 0.00000001 ) pDochTqqHFYhiSrTFbThbrDwvnnrnn=898566573.966126998776424518452249772618f; else pDochTqqHFYhiSrTFbThbrDwvnnrnn=1476208050.933546906134265054455885472846f;if (pDochTqqHFYhiSrTFbThbrDwvnnrnn - pDochTqqHFYhiSrTFbThbrDwvnnrnn> 0.00000001 ) pDochTqqHFYhiSrTFbThbrDwvnnrnn=202701282.548184499971242918582926325817f; else pDochTqqHFYhiSrTFbThbrDwvnnrnn=888604224.517170629266569565367446462387f;int etKVcSGmJRuzQMhUimocDYZBBoFjBt=1469492936;if (etKVcSGmJRuzQMhUimocDYZBBoFjBt == etKVcSGmJRuzQMhUimocDYZBBoFjBt- 1 ) etKVcSGmJRuzQMhUimocDYZBBoFjBt=657170689; else etKVcSGmJRuzQMhUimocDYZBBoFjBt=1779731489;if (etKVcSGmJRuzQMhUimocDYZBBoFjBt == etKVcSGmJRuzQMhUimocDYZBBoFjBt- 0 ) etKVcSGmJRuzQMhUimocDYZBBoFjBt=551105631; else etKVcSGmJRuzQMhUimocDYZBBoFjBt=1680209941;if (etKVcSGmJRuzQMhUimocDYZBBoFjBt == etKVcSGmJRuzQMhUimocDYZBBoFjBt- 1 ) etKVcSGmJRuzQMhUimocDYZBBoFjBt=442755118; else etKVcSGmJRuzQMhUimocDYZBBoFjBt=1839664392;if (etKVcSGmJRuzQMhUimocDYZBBoFjBt == etKVcSGmJRuzQMhUimocDYZBBoFjBt- 0 ) etKVcSGmJRuzQMhUimocDYZBBoFjBt=950623475; else etKVcSGmJRuzQMhUimocDYZBBoFjBt=1258394422;if (etKVcSGmJRuzQMhUimocDYZBBoFjBt == etKVcSGmJRuzQMhUimocDYZBBoFjBt- 0 ) etKVcSGmJRuzQMhUimocDYZBBoFjBt=238958544; else etKVcSGmJRuzQMhUimocDYZBBoFjBt=1525569156;if (etKVcSGmJRuzQMhUimocDYZBBoFjBt == etKVcSGmJRuzQMhUimocDYZBBoFjBt- 1 ) etKVcSGmJRuzQMhUimocDYZBBoFjBt=640803781; else etKVcSGmJRuzQMhUimocDYZBBoFjBt=1738898401;long KWNWkYDdgrKuJWAJvIWrdrjZlbpHsn=2116743567;if (KWNWkYDdgrKuJWAJvIWrdrjZlbpHsn == KWNWkYDdgrKuJWAJvIWrdrjZlbpHsn- 1 ) KWNWkYDdgrKuJWAJvIWrdrjZlbpHsn=515667974; else KWNWkYDdgrKuJWAJvIWrdrjZlbpHsn=1843272771;if (KWNWkYDdgrKuJWAJvIWrdrjZlbpHsn == KWNWkYDdgrKuJWAJvIWrdrjZlbpHsn- 1 ) KWNWkYDdgrKuJWAJvIWrdrjZlbpHsn=695161992; else KWNWkYDdgrKuJWAJvIWrdrjZlbpHsn=628951412;if (KWNWkYDdgrKuJWAJvIWrdrjZlbpHsn == KWNWkYDdgrKuJWAJvIWrdrjZlbpHsn- 1 ) KWNWkYDdgrKuJWAJvIWrdrjZlbpHsn=147106085; else KWNWkYDdgrKuJWAJvIWrdrjZlbpHsn=123280859;if (KWNWkYDdgrKuJWAJvIWrdrjZlbpHsn == KWNWkYDdgrKuJWAJvIWrdrjZlbpHsn- 0 ) KWNWkYDdgrKuJWAJvIWrdrjZlbpHsn=1948445146; else KWNWkYDdgrKuJWAJvIWrdrjZlbpHsn=860074293;if (KWNWkYDdgrKuJWAJvIWrdrjZlbpHsn == KWNWkYDdgrKuJWAJvIWrdrjZlbpHsn- 1 ) KWNWkYDdgrKuJWAJvIWrdrjZlbpHsn=27360107; else KWNWkYDdgrKuJWAJvIWrdrjZlbpHsn=1554917225;if (KWNWkYDdgrKuJWAJvIWrdrjZlbpHsn == KWNWkYDdgrKuJWAJvIWrdrjZlbpHsn- 1 ) KWNWkYDdgrKuJWAJvIWrdrjZlbpHsn=1327208952; else KWNWkYDdgrKuJWAJvIWrdrjZlbpHsn=95937954;long VgEUCouWScqBbMBRhsLrcIwZLHahlN=1074060275;if (VgEUCouWScqBbMBRhsLrcIwZLHahlN == VgEUCouWScqBbMBRhsLrcIwZLHahlN- 0 ) VgEUCouWScqBbMBRhsLrcIwZLHahlN=1494405079; else VgEUCouWScqBbMBRhsLrcIwZLHahlN=463702893;if (VgEUCouWScqBbMBRhsLrcIwZLHahlN == VgEUCouWScqBbMBRhsLrcIwZLHahlN- 1 ) VgEUCouWScqBbMBRhsLrcIwZLHahlN=681193938; else VgEUCouWScqBbMBRhsLrcIwZLHahlN=1890144534;if (VgEUCouWScqBbMBRhsLrcIwZLHahlN == VgEUCouWScqBbMBRhsLrcIwZLHahlN- 1 ) VgEUCouWScqBbMBRhsLrcIwZLHahlN=1515246275; else VgEUCouWScqBbMBRhsLrcIwZLHahlN=1345488262;if (VgEUCouWScqBbMBRhsLrcIwZLHahlN == VgEUCouWScqBbMBRhsLrcIwZLHahlN- 1 ) VgEUCouWScqBbMBRhsLrcIwZLHahlN=872640586; else VgEUCouWScqBbMBRhsLrcIwZLHahlN=1686354063;if (VgEUCouWScqBbMBRhsLrcIwZLHahlN == VgEUCouWScqBbMBRhsLrcIwZLHahlN- 0 ) VgEUCouWScqBbMBRhsLrcIwZLHahlN=629143965; else VgEUCouWScqBbMBRhsLrcIwZLHahlN=395734295;if (VgEUCouWScqBbMBRhsLrcIwZLHahlN == VgEUCouWScqBbMBRhsLrcIwZLHahlN- 0 ) VgEUCouWScqBbMBRhsLrcIwZLHahlN=505748743; else VgEUCouWScqBbMBRhsLrcIwZLHahlN=1673389429;float KXxWxAFOfoJajvrHdhMNbGAXZXqzRU=2086051230.731574453876430268026252878499f;if (KXxWxAFOfoJajvrHdhMNbGAXZXqzRU - KXxWxAFOfoJajvrHdhMNbGAXZXqzRU> 0.00000001 ) KXxWxAFOfoJajvrHdhMNbGAXZXqzRU=1981396373.410741943191137258029339969971f; else KXxWxAFOfoJajvrHdhMNbGAXZXqzRU=233441362.606557670032504523672048018289f;if (KXxWxAFOfoJajvrHdhMNbGAXZXqzRU - KXxWxAFOfoJajvrHdhMNbGAXZXqzRU> 0.00000001 ) KXxWxAFOfoJajvrHdhMNbGAXZXqzRU=372936250.207359578069919884544641977864f; else KXxWxAFOfoJajvrHdhMNbGAXZXqzRU=539539125.040283526505661424034575577748f;if (KXxWxAFOfoJajvrHdhMNbGAXZXqzRU - KXxWxAFOfoJajvrHdhMNbGAXZXqzRU> 0.00000001 ) KXxWxAFOfoJajvrHdhMNbGAXZXqzRU=937383455.271403983636805918166493167581f; else KXxWxAFOfoJajvrHdhMNbGAXZXqzRU=1194431289.799943426855140878167968399945f;if (KXxWxAFOfoJajvrHdhMNbGAXZXqzRU - KXxWxAFOfoJajvrHdhMNbGAXZXqzRU> 0.00000001 ) KXxWxAFOfoJajvrHdhMNbGAXZXqzRU=1322386851.651205807260232145449052593165f; else KXxWxAFOfoJajvrHdhMNbGAXZXqzRU=533889830.568231595608191874726451760981f;if (KXxWxAFOfoJajvrHdhMNbGAXZXqzRU - KXxWxAFOfoJajvrHdhMNbGAXZXqzRU> 0.00000001 ) KXxWxAFOfoJajvrHdhMNbGAXZXqzRU=1978769990.274370211275184107282054022857f; else KXxWxAFOfoJajvrHdhMNbGAXZXqzRU=1838514985.262454493266456147877470923136f;if (KXxWxAFOfoJajvrHdhMNbGAXZXqzRU - KXxWxAFOfoJajvrHdhMNbGAXZXqzRU> 0.00000001 ) KXxWxAFOfoJajvrHdhMNbGAXZXqzRU=1652849834.847411474172412770898519180253f; else KXxWxAFOfoJajvrHdhMNbGAXZXqzRU=1035321540.146843005588380507935435834833f;float JFEyUehixjiZPHDJPwdrMSLpkBFxDr=635279863.888319883063904636832659429304f;if (JFEyUehixjiZPHDJPwdrMSLpkBFxDr - JFEyUehixjiZPHDJPwdrMSLpkBFxDr> 0.00000001 ) JFEyUehixjiZPHDJPwdrMSLpkBFxDr=867318213.982804766985319291138882434909f; else JFEyUehixjiZPHDJPwdrMSLpkBFxDr=1714227153.169028210097105735024927621048f;if (JFEyUehixjiZPHDJPwdrMSLpkBFxDr - JFEyUehixjiZPHDJPwdrMSLpkBFxDr> 0.00000001 ) JFEyUehixjiZPHDJPwdrMSLpkBFxDr=244493322.186792740108114279871099634250f; else JFEyUehixjiZPHDJPwdrMSLpkBFxDr=1468509423.838302141310502682455337719827f;if (JFEyUehixjiZPHDJPwdrMSLpkBFxDr - JFEyUehixjiZPHDJPwdrMSLpkBFxDr> 0.00000001 ) JFEyUehixjiZPHDJPwdrMSLpkBFxDr=278805052.950048181214857250705390651170f; else JFEyUehixjiZPHDJPwdrMSLpkBFxDr=1899634437.442556067550125258167912277059f;if (JFEyUehixjiZPHDJPwdrMSLpkBFxDr - JFEyUehixjiZPHDJPwdrMSLpkBFxDr> 0.00000001 ) JFEyUehixjiZPHDJPwdrMSLpkBFxDr=601497292.812297231120336527640301353049f; else JFEyUehixjiZPHDJPwdrMSLpkBFxDr=1317663359.011975994802814779775041951194f;if (JFEyUehixjiZPHDJPwdrMSLpkBFxDr - JFEyUehixjiZPHDJPwdrMSLpkBFxDr> 0.00000001 ) JFEyUehixjiZPHDJPwdrMSLpkBFxDr=970632185.520312956273395960779547407103f; else JFEyUehixjiZPHDJPwdrMSLpkBFxDr=1156291576.480149350382360286719143609392f;if (JFEyUehixjiZPHDJPwdrMSLpkBFxDr - JFEyUehixjiZPHDJPwdrMSLpkBFxDr> 0.00000001 ) JFEyUehixjiZPHDJPwdrMSLpkBFxDr=2147291094.595480436772159738823140444854f; else JFEyUehixjiZPHDJPwdrMSLpkBFxDr=1898855437.006980799095105800597167945605f;float eoIhjbMkwzUznSoEVdeRTQijxUtxSW=1828881488.885141188747631127573711242538f;if (eoIhjbMkwzUznSoEVdeRTQijxUtxSW - eoIhjbMkwzUznSoEVdeRTQijxUtxSW> 0.00000001 ) eoIhjbMkwzUznSoEVdeRTQijxUtxSW=1424829823.017270172555903665147806218490f; else eoIhjbMkwzUznSoEVdeRTQijxUtxSW=1388641369.066184056105502892969759121207f;if (eoIhjbMkwzUznSoEVdeRTQijxUtxSW - eoIhjbMkwzUznSoEVdeRTQijxUtxSW> 0.00000001 ) eoIhjbMkwzUznSoEVdeRTQijxUtxSW=262025228.388260092061904231452763231706f; else eoIhjbMkwzUznSoEVdeRTQijxUtxSW=1759313736.485093845281090886690196572514f;if (eoIhjbMkwzUznSoEVdeRTQijxUtxSW - eoIhjbMkwzUznSoEVdeRTQijxUtxSW> 0.00000001 ) eoIhjbMkwzUznSoEVdeRTQijxUtxSW=1378226078.939779701259737177345621630053f; else eoIhjbMkwzUznSoEVdeRTQijxUtxSW=118711818.134145333229554157950627942824f;if (eoIhjbMkwzUznSoEVdeRTQijxUtxSW - eoIhjbMkwzUznSoEVdeRTQijxUtxSW> 0.00000001 ) eoIhjbMkwzUznSoEVdeRTQijxUtxSW=136676820.975913669294422617759411361721f; else eoIhjbMkwzUznSoEVdeRTQijxUtxSW=299973790.452045574269887621643617556566f;if (eoIhjbMkwzUznSoEVdeRTQijxUtxSW - eoIhjbMkwzUznSoEVdeRTQijxUtxSW> 0.00000001 ) eoIhjbMkwzUznSoEVdeRTQijxUtxSW=1288799689.394994358724360074484029894559f; else eoIhjbMkwzUznSoEVdeRTQijxUtxSW=147304108.002907699348492936859219338511f;if (eoIhjbMkwzUznSoEVdeRTQijxUtxSW - eoIhjbMkwzUznSoEVdeRTQijxUtxSW> 0.00000001 ) eoIhjbMkwzUznSoEVdeRTQijxUtxSW=2058858191.190982357358625663896970180467f; else eoIhjbMkwzUznSoEVdeRTQijxUtxSW=1824214937.919935951078961179577683522081f;double vsySTviuvlFLlKoyIwzADuvzSPgTFU=1921942091.940004001316543113241262388737;if (vsySTviuvlFLlKoyIwzADuvzSPgTFU == vsySTviuvlFLlKoyIwzADuvzSPgTFU ) vsySTviuvlFLlKoyIwzADuvzSPgTFU=1130094971.856823175470740363397029979983; else vsySTviuvlFLlKoyIwzADuvzSPgTFU=2017952527.737567082104887263302555083896;if (vsySTviuvlFLlKoyIwzADuvzSPgTFU == vsySTviuvlFLlKoyIwzADuvzSPgTFU ) vsySTviuvlFLlKoyIwzADuvzSPgTFU=806071216.530385328439239435282668128369; else vsySTviuvlFLlKoyIwzADuvzSPgTFU=2106540325.439668451394597443775872597336;if (vsySTviuvlFLlKoyIwzADuvzSPgTFU == vsySTviuvlFLlKoyIwzADuvzSPgTFU ) vsySTviuvlFLlKoyIwzADuvzSPgTFU=101468944.717001168108599927288679467827; else vsySTviuvlFLlKoyIwzADuvzSPgTFU=809061626.223579760749668848378015914864;if (vsySTviuvlFLlKoyIwzADuvzSPgTFU == vsySTviuvlFLlKoyIwzADuvzSPgTFU ) vsySTviuvlFLlKoyIwzADuvzSPgTFU=1807246178.434108612127855012273956021876; else vsySTviuvlFLlKoyIwzADuvzSPgTFU=81761936.600631556024635126191369258918;if (vsySTviuvlFLlKoyIwzADuvzSPgTFU == vsySTviuvlFLlKoyIwzADuvzSPgTFU ) vsySTviuvlFLlKoyIwzADuvzSPgTFU=1779427717.884762732456635933190578534159; else vsySTviuvlFLlKoyIwzADuvzSPgTFU=1202756538.374835008340575583432683931723;if (vsySTviuvlFLlKoyIwzADuvzSPgTFU == vsySTviuvlFLlKoyIwzADuvzSPgTFU ) vsySTviuvlFLlKoyIwzADuvzSPgTFU=1716390587.978428381104586010316172779200; else vsySTviuvlFLlKoyIwzADuvzSPgTFU=1928575526.438658216730333910892252780785;long DnDokezQmoCUepfsUsphuWFsJrgrks=1703714273;if (DnDokezQmoCUepfsUsphuWFsJrgrks == DnDokezQmoCUepfsUsphuWFsJrgrks- 0 ) DnDokezQmoCUepfsUsphuWFsJrgrks=504240876; else DnDokezQmoCUepfsUsphuWFsJrgrks=1971451993;if (DnDokezQmoCUepfsUsphuWFsJrgrks == DnDokezQmoCUepfsUsphuWFsJrgrks- 0 ) DnDokezQmoCUepfsUsphuWFsJrgrks=1757975364; else DnDokezQmoCUepfsUsphuWFsJrgrks=1271297718;if (DnDokezQmoCUepfsUsphuWFsJrgrks == DnDokezQmoCUepfsUsphuWFsJrgrks- 0 ) DnDokezQmoCUepfsUsphuWFsJrgrks=592660293; else DnDokezQmoCUepfsUsphuWFsJrgrks=1801292882;if (DnDokezQmoCUepfsUsphuWFsJrgrks == DnDokezQmoCUepfsUsphuWFsJrgrks- 1 ) DnDokezQmoCUepfsUsphuWFsJrgrks=1404537432; else DnDokezQmoCUepfsUsphuWFsJrgrks=748606395;if (DnDokezQmoCUepfsUsphuWFsJrgrks == DnDokezQmoCUepfsUsphuWFsJrgrks- 1 ) DnDokezQmoCUepfsUsphuWFsJrgrks=1577550333; else DnDokezQmoCUepfsUsphuWFsJrgrks=2092003179;if (DnDokezQmoCUepfsUsphuWFsJrgrks == DnDokezQmoCUepfsUsphuWFsJrgrks- 0 ) DnDokezQmoCUepfsUsphuWFsJrgrks=179709734; else DnDokezQmoCUepfsUsphuWFsJrgrks=131500944;int ZYeGdRwkCivpzsHYZaYavffriGvEoI=1133313139;if (ZYeGdRwkCivpzsHYZaYavffriGvEoI == ZYeGdRwkCivpzsHYZaYavffriGvEoI- 1 ) ZYeGdRwkCivpzsHYZaYavffriGvEoI=2129876600; else ZYeGdRwkCivpzsHYZaYavffriGvEoI=225349003;if (ZYeGdRwkCivpzsHYZaYavffriGvEoI == ZYeGdRwkCivpzsHYZaYavffriGvEoI- 0 ) ZYeGdRwkCivpzsHYZaYavffriGvEoI=768760466; else ZYeGdRwkCivpzsHYZaYavffriGvEoI=1894546883;if (ZYeGdRwkCivpzsHYZaYavffriGvEoI == ZYeGdRwkCivpzsHYZaYavffriGvEoI- 0 ) ZYeGdRwkCivpzsHYZaYavffriGvEoI=1616468148; else ZYeGdRwkCivpzsHYZaYavffriGvEoI=1375217431;if (ZYeGdRwkCivpzsHYZaYavffriGvEoI == ZYeGdRwkCivpzsHYZaYavffriGvEoI- 0 ) ZYeGdRwkCivpzsHYZaYavffriGvEoI=1727412544; else ZYeGdRwkCivpzsHYZaYavffriGvEoI=615768197;if (ZYeGdRwkCivpzsHYZaYavffriGvEoI == ZYeGdRwkCivpzsHYZaYavffriGvEoI- 1 ) ZYeGdRwkCivpzsHYZaYavffriGvEoI=1728878838; else ZYeGdRwkCivpzsHYZaYavffriGvEoI=180263292;if (ZYeGdRwkCivpzsHYZaYavffriGvEoI == ZYeGdRwkCivpzsHYZaYavffriGvEoI- 1 ) ZYeGdRwkCivpzsHYZaYavffriGvEoI=2127369666; else ZYeGdRwkCivpzsHYZaYavffriGvEoI=175469540;double LwuNoEKAXXeCtsGZdCiPmsaiMVPYMv=1865268275.723247729409118443703973812459;if (LwuNoEKAXXeCtsGZdCiPmsaiMVPYMv == LwuNoEKAXXeCtsGZdCiPmsaiMVPYMv ) LwuNoEKAXXeCtsGZdCiPmsaiMVPYMv=1378430041.097579295825725404387127695043; else LwuNoEKAXXeCtsGZdCiPmsaiMVPYMv=355143918.547151264935998925852928298109;if (LwuNoEKAXXeCtsGZdCiPmsaiMVPYMv == LwuNoEKAXXeCtsGZdCiPmsaiMVPYMv ) LwuNoEKAXXeCtsGZdCiPmsaiMVPYMv=1319974061.436349867424046223987029163096; else LwuNoEKAXXeCtsGZdCiPmsaiMVPYMv=2016153729.417661076017853808218622241728;if (LwuNoEKAXXeCtsGZdCiPmsaiMVPYMv == LwuNoEKAXXeCtsGZdCiPmsaiMVPYMv ) LwuNoEKAXXeCtsGZdCiPmsaiMVPYMv=226827329.797201749635708128736589890652; else LwuNoEKAXXeCtsGZdCiPmsaiMVPYMv=2050084552.046832075217078393512319331842;if (LwuNoEKAXXeCtsGZdCiPmsaiMVPYMv == LwuNoEKAXXeCtsGZdCiPmsaiMVPYMv ) LwuNoEKAXXeCtsGZdCiPmsaiMVPYMv=1329281798.220769007616352052150714084861; else LwuNoEKAXXeCtsGZdCiPmsaiMVPYMv=1476285736.223310842940700716749980362689;if (LwuNoEKAXXeCtsGZdCiPmsaiMVPYMv == LwuNoEKAXXeCtsGZdCiPmsaiMVPYMv ) LwuNoEKAXXeCtsGZdCiPmsaiMVPYMv=613415095.634740405332122112447813335415; else LwuNoEKAXXeCtsGZdCiPmsaiMVPYMv=57464821.826442606141241896836125292251;if (LwuNoEKAXXeCtsGZdCiPmsaiMVPYMv == LwuNoEKAXXeCtsGZdCiPmsaiMVPYMv ) LwuNoEKAXXeCtsGZdCiPmsaiMVPYMv=528989992.500051679051640768254069081235; else LwuNoEKAXXeCtsGZdCiPmsaiMVPYMv=156949412.156564129122798559554510865218;double TZYvBYDnjeTsGQKNjAaOJknFeVinys=854661715.532245808288256659281307703908;if (TZYvBYDnjeTsGQKNjAaOJknFeVinys == TZYvBYDnjeTsGQKNjAaOJknFeVinys ) TZYvBYDnjeTsGQKNjAaOJknFeVinys=1725215254.935135786235084373378596824654; else TZYvBYDnjeTsGQKNjAaOJknFeVinys=583077448.291135322298415642454173055932;if (TZYvBYDnjeTsGQKNjAaOJknFeVinys == TZYvBYDnjeTsGQKNjAaOJknFeVinys ) TZYvBYDnjeTsGQKNjAaOJknFeVinys=1946182573.223205506005229129239221123919; else TZYvBYDnjeTsGQKNjAaOJknFeVinys=969274839.059018521114745995503680300228;if (TZYvBYDnjeTsGQKNjAaOJknFeVinys == TZYvBYDnjeTsGQKNjAaOJknFeVinys ) TZYvBYDnjeTsGQKNjAaOJknFeVinys=802254595.379805622160813568724841658081; else TZYvBYDnjeTsGQKNjAaOJknFeVinys=1169294478.841494695048445397895260553926;if (TZYvBYDnjeTsGQKNjAaOJknFeVinys == TZYvBYDnjeTsGQKNjAaOJknFeVinys ) TZYvBYDnjeTsGQKNjAaOJknFeVinys=87246125.875132053907479651736220778194; else TZYvBYDnjeTsGQKNjAaOJknFeVinys=1276507092.278037331345409890191320066647;if (TZYvBYDnjeTsGQKNjAaOJknFeVinys == TZYvBYDnjeTsGQKNjAaOJknFeVinys ) TZYvBYDnjeTsGQKNjAaOJknFeVinys=244039449.506083044242040292198901496907; else TZYvBYDnjeTsGQKNjAaOJknFeVinys=1142207167.289594953695606949098456060569;if (TZYvBYDnjeTsGQKNjAaOJknFeVinys == TZYvBYDnjeTsGQKNjAaOJknFeVinys ) TZYvBYDnjeTsGQKNjAaOJknFeVinys=1689902527.814716366635951331656892084453; else TZYvBYDnjeTsGQKNjAaOJknFeVinys=412397001.559598109078346460573001412384;double sDLIsfYgMJTLbslVbfhLQvpAaWnCqf=1221965392.313024480887339470822965541261;if (sDLIsfYgMJTLbslVbfhLQvpAaWnCqf == sDLIsfYgMJTLbslVbfhLQvpAaWnCqf ) sDLIsfYgMJTLbslVbfhLQvpAaWnCqf=839169584.964851416752939083291285545969; else sDLIsfYgMJTLbslVbfhLQvpAaWnCqf=461925106.323903369907418014666907178032;if (sDLIsfYgMJTLbslVbfhLQvpAaWnCqf == sDLIsfYgMJTLbslVbfhLQvpAaWnCqf ) sDLIsfYgMJTLbslVbfhLQvpAaWnCqf=900554550.766753014772540598854130343683; else sDLIsfYgMJTLbslVbfhLQvpAaWnCqf=1633115975.543824186500109557879129864642;if (sDLIsfYgMJTLbslVbfhLQvpAaWnCqf == sDLIsfYgMJTLbslVbfhLQvpAaWnCqf ) sDLIsfYgMJTLbslVbfhLQvpAaWnCqf=1752275588.153124972642235413299782543506; else sDLIsfYgMJTLbslVbfhLQvpAaWnCqf=1756354849.064556708668524809456256347279;if (sDLIsfYgMJTLbslVbfhLQvpAaWnCqf == sDLIsfYgMJTLbslVbfhLQvpAaWnCqf ) sDLIsfYgMJTLbslVbfhLQvpAaWnCqf=906485810.158668879981664226875416192931; else sDLIsfYgMJTLbslVbfhLQvpAaWnCqf=79792048.054051096604157579213140485368;if (sDLIsfYgMJTLbslVbfhLQvpAaWnCqf == sDLIsfYgMJTLbslVbfhLQvpAaWnCqf ) sDLIsfYgMJTLbslVbfhLQvpAaWnCqf=1043550852.704803861324038236963473992468; else sDLIsfYgMJTLbslVbfhLQvpAaWnCqf=1439958377.784900045581800012236951544292;if (sDLIsfYgMJTLbslVbfhLQvpAaWnCqf == sDLIsfYgMJTLbslVbfhLQvpAaWnCqf ) sDLIsfYgMJTLbslVbfhLQvpAaWnCqf=1281131788.240464023820148367308034394447; else sDLIsfYgMJTLbslVbfhLQvpAaWnCqf=1559003327.785445155600951681338890522727; }
 sDLIsfYgMJTLbslVbfhLQvpAaWnCqfy::sDLIsfYgMJTLbslVbfhLQvpAaWnCqfy()
 { this->AjcAYEJjHbde("JannlDKlkBTMYvjXLhfihDEvwNLhVsAjcAYEJjHbdej", true, 846227439, 1570463132, 2133899952); }
#pragma optimize("", off)
 // <delete/>

