#pragma once

#include <intrin.h>
#include "../features/notify/notify.h"
#include "../valve_sdk/csgostructs.h"
#include <d3d9.h>
#pragma intrinsic(_ReturnAddress)  
#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_win32.h"
#include "../imgui/imgui_impl_dx9.h"
#include "../menu/menu.h"
#include "../render/render.h"
#include "../options/globals.h"
#include "../features/visuals/visuals.h"


#include "../features/visuals/grenade_prediction.h"
#include "../features/misc/misc.h"
#include "../features/misc/engine_prediction.h"
#include "../features/legit_aimbot/aimbot.h"
#include "../features/legit_aimbot/backtrack.h"
#include "../helpers/input.h"
#include "../minhook/minhook.h"

namespace index
{
    constexpr auto is_playing_demo = 82;
    constexpr auto send_datagram = 46;
    constexpr auto retrieve_message = 2;
    constexpr auto emit_sound = 5;
    constexpr auto end_scene = 42;
    constexpr auto reset = 16;
    constexpr auto paint_traverse = 41;
    constexpr auto create_move = 24;
    constexpr auto play_sound = 82;
    constexpr auto draw_model_execute = 21;
    constexpr auto do_post_screen_space_effects = 44;
    constexpr auto cvar_get_bool = 13;
    constexpr auto override_view = 18;
    constexpr auto fire_event = 9;
    constexpr auto lock_cursor = 67;
    constexpr auto list_leaves_in_box = 6;
    constexpr auto frame_stage_notify = 37;
    constexpr auto is_connected = 27;
    constexpr auto sendnetmsg = 40;
    constexpr auto read_packet = 17;
    constexpr auto hud_update = 11;
    constexpr auto client_command = 7;
    constexpr auto is_hltv = 93;
    constexpr auto suppress_lists = 16;
    constexpr auto scene_end_i = 9;
    constexpr auto read_packet_entities = 64;
}

//minhook
namespace hooks
{
    void initialize();
    void shutdown();

    namespace create_move
    {
        using fn = bool(__thiscall*)(i_client_mode*, float, c_usercmd*);
        bool __stdcall hook(float input_sample_frametime, c_usercmd* cmd);
    }
    inline create_move::fn o_create_move = nullptr; //

    namespace end_scene
    {
        using fn = long(__stdcall*)(IDirect3DDevice9*);
        long __stdcall hook(IDirect3DDevice9* device);
    }
    inline end_scene::fn o_end_scene = nullptr; //

    namespace reset
    {
        using fn = long(__stdcall*)(IDirect3DDevice9*, D3DPRESENT_PARAMETERS*);
        long __stdcall hook(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* pPresentationParameters);
    }
    inline reset::fn o_reset = nullptr; //

    namespace paint_treverse
    {
        using fn = void(__thiscall*)(i_panel*, vgui::VPANEL, bool, bool);
        void __stdcall hook(vgui::VPANEL panel, bool forceRepaint, bool allowForce);
    }
    inline paint_treverse::fn o_paint_treverse = nullptr; //

    namespace fire_event
    {
        using fn = bool(__thiscall*)(i_gameeventmanager2*, i_gameevent* pEvent);
        bool __stdcall hook(i_gameevent* pEvent);
    }
    inline fire_event::fn o_fire_event = nullptr; //

    namespace lock_cursor
    {
        using fn = void(__thiscall*)(i_surface*);
        void __stdcall hook();
    }
    inline lock_cursor::fn o_lock_cursor = nullptr; //

    namespace override_view
    {
        using fn = void(__thiscall*)(i_client_mode*, c_viewsetup*);
        void __stdcall hook(c_viewsetup* vsView);
    }
    inline override_view::fn o_override_view = nullptr;

    namespace emit_sound
    {
        using fn = void(__thiscall*)(void*, i_recipient_filter&, int, int, const char*, unsigned int, const char*, float, int, float, int, int, const Vector*, const Vector*, void*, bool, float, int, int);
        void __stdcall hook(i_recipient_filter& filter, int iEntIndex, int iChannel, const char* pSoundEntry, unsigned int nSoundEntryHash, const char* pSample, float flVolume, int nSeed, float flAttenuation, int iFlags, int iPitch, const Vector* pOrigin, const Vector* pDirection, void* pUtlVecOrigins, bool bUpdatePositions, float soundtime, int speakerentity, int unk);
    }
    inline emit_sound::fn o_emit_sound = nullptr; //

    namespace list_leaves_in_box
    {
        using fn = int(__thiscall*)(void*, const Vector&, const Vector&, unsigned short*, int);
        int __fastcall hook(void* bsp, void* edx, Vector& mins, Vector& maxs, unsigned short* pList, int listMax);
    }
    inline list_leaves_in_box::fn o_list_leaves_in_box = nullptr;

    namespace cvar_hooks
    {
        namespace sv_cheats
        {
            using fn = bool(__thiscall*)(PVOID);
            bool __fastcall hook(PVOID pConVar, void* edx);
        }
    }
    inline cvar_hooks::sv_cheats::fn o_sv_cheats = nullptr;

 }








