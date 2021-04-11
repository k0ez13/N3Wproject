#pragma once

#define NOMINMAX
#include <Windows.h>

#include "misc/vfunc.h"

#include "misc/Enums.h"
#include "math/VMatrix.h"
#include "math/QAngle.h"
#include "math/Vector.h"
#include "misc/Studio.h"

#include "interfaces/steam.h"
#include "interfaces/i_basefilesystem.h"
#include "interfaces/i_app_system.h"
#include "interfaces/i_vieweenderbeams.h"
#include "interfaces/i_base_clientdll.h"
#include "interfaces/i_client_entity.h"
#include "interfaces/i_client_entity_list.h"
#include "interfaces/i_client_mode.h"
#include "interfaces/i_convar.h"
#include "interfaces/i_cvar.h"
#include "interfaces/i_engine_trace.h"
#include "interfaces/i_vengine_client.h"
#include "interfaces/i_vdebug_overlay.h"
#include "interfaces/i_surface.h"
#include "interfaces/c_input.h"
#include "interfaces/i_vmodel_info_client.h"
#include "interfaces/i_vmodelrender.h"
#include "interfaces/i_vrenderview.h"
#include "interfaces/i_gameeventmanager.h"
#include "interfaces/i_materialSystem.h"
#include "interfaces/i_move_helper.h"
#include "interfaces/i_mdl_cache.h"
#include "interfaces/i_prediction.h"
#include "interfaces/i_panel.h"
#include "interfaces/i_enginesound.h"
#include "interfaces/i_viewrender.h"
#include "interfaces/c_clientstate.h"
#include "interfaces/i_physics.h"
#include "interfaces/i_inputsystem.h"
#include "interfaces/i_ref_counted.h"
#include "interfaces/i_memalloc.h"
#include "interfaces/i_localize.h"
#include "interfaces/i_studiorender.h"
#include "misc/convar.h"
#include "misc/c_usercmd.h"
#include "misc/glow_outline_effect.h"
#include "misc/datamap.h"
#include "netvars.h"

struct IDirect3DDevice9;

namespace interfaces
{
  void initialize( );
  void dump( );
}

extern i_vengine_client*            g_engine_client;
extern i_base_clientdll*            g_chl_client;
extern i_client_entity_list*        g_entity_list;
extern c_global_varsbase*           g_global_vars;
extern i_engine_trace*              g_engine_trace;
extern i_cvar*                      g_cvar;
extern i_panel*                     g_vgui_panel;
extern i_client_mode*               g_client_mode;
extern i_vdebug_overlay*            g_debug_overlay;
extern i_surface*                   g_vgui_surface;
extern c_input*                     g_input;
extern i_vmodelinfo_client*         g_mdl_info;
extern i_vmodelrender*              g_mdl_render;
extern i_vrenderview*               g_render_view;
extern i_materialsystem*            g_mat_system;
extern i_gameeventmanager2*         g_game_events;
extern i_move_helper*               g_move_helper;
extern i_mdl_cache*                 g_mdl_cache;
extern i_prediction*                g_prediction;
extern c_gamemovement*              g_game_movement;
extern i_enginesound*               g_engine_sound;
extern i_vieweenderbeams*           g_beam;
extern c_glow_object_manager*       g_glow_obj_manager;
extern i_viewrender*                g_view_render;
extern IDirect3DDevice9*            g_d3ddevice9;
extern c_clientstate*               g_client_state;
extern i_physics_surface_props*     g_phys_surface;
extern i_localize*				    g_localize;
extern i_inputsystem*               g_input_system;
extern i_memalloc*			    	g_mem_alloc;
extern i_basefilesystem*            g_base_file_system;
extern uintptr_t*					g_spatial_partition;
extern i_studiorender*				g_studio_render;
extern i_steam_game_coordinator*    g_game_coordinator;
extern i_steam_user*                g_steam_user;
extern i_steam_http*                g_steam_http;
extern i_steam_client*              g_steam_client;
extern i_steam_friends*             g_steam_friends;


extern void( __cdecl* RandomSeed )( uint32_t seed );
extern int( __cdecl* RandomInt )( int min, int max );
extern float( __cdecl* RandomFloat )( float min, float max );

template<typename... Args>
void ConMsg( const char* pMsg, Args... args ) {
  static auto import = ( void( *)( const char*, ... ) )GetProcAddress( GetModuleHandleW( L"tier0.dll" ), "?ConMsg@@YAXPBDZZ" );
  return import( pMsg, args... );
}
template<typename... Args>
void ConColorMsg( const Color& clr, const char* pMsg, Args... args ) {
  static auto import = ( void( *)( const Color&, const char*, ... ) )GetProcAddress( GetModuleHandleW( L"tier0.dll" ), "?ConColorMsg@@YAXABVColor@@PBDZZ" );
  return import( clr, pMsg, args... );
}

#include "misc/e_handle.h"

class c_localplayer
{
  friend bool operator==( const c_localplayer& lhs, void* rhs );
public:
    c_localplayer( ) : m_local( nullptr ) { }

  operator bool( ) const { return *m_local != nullptr; }
  operator c_base_player*( ) const { return *m_local; }

  c_base_player* operator->( ) { return *m_local; }

private:
  c_base_player** m_local;
};

extern c_localplayer g_local_player;
class c_cs_player_resource;
extern c_cs_player_resource** g_player_resource;

