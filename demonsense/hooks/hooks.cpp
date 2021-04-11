#include <intrin.h>  

#include "hooks.h"
#include "../render/render.h"
#include "../menu/menu.h"
#include "../options/options.h"
#include "../features/visuals/visuals.h"

//minhook
namespace hooks
{
	void initialize()
	{
		const auto end_scene_fn = reinterpret_cast<void*>(utils::GetVirtual(g_d3ddevice9, index::end_scene));
		const auto reset_fn = reinterpret_cast<void*>(utils::GetVirtual(g_d3ddevice9, index::reset));
		const auto lock_cursor_fn = reinterpret_cast<void*>(utils::GetVirtual(g_vgui_surface, index::lock_cursor));
		const auto create_move_fn = reinterpret_cast<void*>(utils::GetVirtual(g_client_mode, index::create_move));
		const auto sound_hook_fn = reinterpret_cast<void*>(utils::GetVirtual(g_engine_sound, index::emit_sound));
		const auto paint_treverse_fn = reinterpret_cast<void*>(utils::GetVirtual(g_vgui_panel, index::paint_traverse));
		const auto override_view_fn = reinterpret_cast<void*>(utils::GetVirtual(g_client_mode, index::override_view));
		const auto list_leaves_in_box_fn = reinterpret_cast<void*>(utils::GetVirtual(g_engine_client->get_bsp_treequery(), index::list_leaves_in_box));
		convar* sv_cheats_con = g_cvar->find_var("sv_cheats");
		const auto sv_cheats_fn = reinterpret_cast<void*>(utils::GetVirtual(sv_cheats_con, index::cvar_get_bool));
		const auto fire_event_fn = reinterpret_cast<void*>(utils::GetVirtual(g_game_events, index::fire_event));


		if (MH_Initialize() != MH_OK)
			throw std::runtime_error("failed to initialize MH_Initialize.");

		if (MH_CreateHook(end_scene_fn, &end_scene::hook, reinterpret_cast<void**>(&o_end_scene)) != MH_OK)
			throw std::runtime_error("failed to initialize end_scene. (outdated index?)");

		if (MH_CreateHook(create_move_fn, &create_move::hook, reinterpret_cast<void**>(&o_create_move)) != MH_OK)
			throw std::runtime_error("failed to initialize create_move. (outdated index?)");

		if (MH_CreateHook(sound_hook_fn, &emit_sound::hook, reinterpret_cast<void**>(&o_emit_sound)) != MH_OK)
			throw std::runtime_error("failed to initialize emit_sound. (outdated index?)");

		if (MH_CreateHook(paint_treverse_fn, &paint_treverse::hook, reinterpret_cast<void**>(&o_paint_treverse)) != MH_OK)
			throw std::runtime_error("failed to initialize paint_treverse. (outdated index?)");

		if (MH_CreateHook(override_view_fn, &override_view::hook, reinterpret_cast<void**>(&o_override_view)) != MH_OK)
			throw std::runtime_error("failed to initialize override_view. (outdated index?)");

		if (MH_CreateHook(list_leaves_in_box_fn, &list_leaves_in_box::hook, reinterpret_cast<void**>(&o_list_leaves_in_box)) != MH_OK)
			throw std::runtime_error("failed to initialize list_leaves_in_box. (outdated index?)");

		if (MH_CreateHook(sv_cheats_fn, &cvar_hooks::sv_cheats::hook, reinterpret_cast<void**>(&o_sv_cheats)) != MH_OK)
			throw std::runtime_error("failed to initialize sv_cheats. (outdated index?)");//

		if (MH_CreateHook(lock_cursor_fn, &lock_cursor::hook, reinterpret_cast<void**>(&o_lock_cursor)) != MH_OK)
			throw std::runtime_error("failed to initialize lock_cursor. (outdated index?)");

		if (MH_CreateHook(reset_fn, &reset::hook, reinterpret_cast<void**>(&o_reset)) != MH_OK)
			throw std::runtime_error("failed to initialize reset. (outdated index?)");

		if (MH_CreateHook(fire_event_fn, &fire_event::hook, reinterpret_cast<void**>(&o_fire_event)) != MH_OK)
			throw std::runtime_error("failed to initialize fire_event. (outdated index?)");
			
		if (MH_EnableHook(MH_ALL_HOOKS) != MH_OK)
			throw std::runtime_error("failed to enable hooks.");
	}

	//--------------------------------------------------------------------------------
	void shutdown()
	{
		MH_Uninitialize();
		MH_DisableHook(MH_ALL_HOOKS);
	}
}
















//v_funchook
/*namespace hooks
{
	vfunc_hook netchannel_vhook;
	vfunc_hook mdlrender_vhook;
	vfunc_hook retrieve_message_vhook;
	vfunc_hook direct3d_vhook;
	vfunc_hook vguipanel_vhook;
	vfunc_hook vguisurf_vhook;
	vfunc_hook viewrender_vhook;
	vfunc_hook client_vhook;
	vfunc_hook render_view_vhook;
	vfunc_hook engine_vhook;
	vfunc_hook rend_vhook;
	vfunc_hook sv_cheats_vhook;
	vfunc_hook sound_vhook;
	vfunc_hook clientmode_vhook;
	vfunc_hook partition_vhook;
	vfunc_hook clientstate_vhook;
	vfunc_hook bsp_query_vhook;
	vfunc_hook gameevents_vhook;
	recv_prop_hook* sequence_vhook;



	
	void initialize()
	{
		client_vhook.setup(g_chl_client);
		render_view_vhook.setup(g_render_view);
		engine_vhook.setup(g_engine_client);
		retrieve_message_vhook.setup(g_game_coordinator);
		direct3d_vhook.setup(g_d3ddevice9);
		vguipanel_vhook.setup(g_vgui_panel);
		vguisurf_vhook.setup(g_vgui_surface);
		sound_vhook.setup(g_engine_sound);
		mdlrender_vhook.setup(g_mdl_render);
		clientmode_vhook.setup(g_client_mode);
		partition_vhook.setup(g_spatial_partition);
		clientstate_vhook.setup((void*)((uintptr_t)g_client_state + 0x8));
		bsp_query_vhook.setup(g_engine_client->get_bsp_treequery());
		gameevents_vhook.setup(g_game_events);
		convar* sv_cheats_con = g_cvar->find_var("sv_cheats");
		sv_cheats_vhook.setup(sv_cheats_con);

		direct3d_vhook.hook_index(index::end_scene, end_scene::hook);
		direct3d_vhook.hook_index(index::reset, reset::hook);
		vguipanel_vhook.hook_index(index::paint_traverse, paint_treverse::hook);

		//open console
		g_engine_client->execute_client_cmd("clear");
		g_cvar->console_printf("                                                          iY\n");
		g_cvar->console_printf("                                                        sBB\n");
		g_cvar->console_printf("                                                     .2BQBI \n");
		g_cvar->console_printf("                                                   .DQBQBB \n");
		g_cvar->console_printf("                                                iQBQBQBBJ\n");
		g_cvar->console_printf("                                              LBQBBY.BBQ\n");
		g_cvar->console_printf("                                            SQBBBJ  KBB7\n");
		g_cvar->console_printf("                        LQBQBQBQBU        :dBBBB7  .QBM\n");
		g_cvar->console_printf("                        QBBQQBMBBBL      ZBBBQ:    BBB7\n");
		g_cvar->console_printf("                       vBBU     BQBv    7QBB      iBBM\n");
		g_cvar->console_printf("                      BQB       BBB7   MBB.    .:BBBi\n");
		g_cvar->console_printf("                     uQBv        BQBi iBQB      LBBb\n");
		g_cvar->console_printf("                    .BBB          BQB:EQBi.     QBQi\n");
		g_cvar->console_printf("                    2BBj..   :j    BBBBBB      jBBb\n");
		g_cvar->console_printf("                   .BQB       .     BBBB:   ::  BQB:\n");
		g_cvar->console_printf("                   bBBi              BBB  :::: XBBX\n");
		g_cvar->console_printf("                  :QBB       .       UB7 .:::  BBB:\n");
		g_cvar->console_printf("                  EBB:       Bb  ...7 . .:::. dBQS\n");
		g_cvar->console_printf("                 :BBB       iBBJ  ....  ::::  BBB:\n");
		g_cvar->console_printf("                 QBB.  ..i. QBBBL  :.::::::. QQB1\n");
		g_cvar->console_printf("                iQBb  ::ii :BQBQBL .:.:.::i .BBR:\n");
		g_cvar->console_printf("                BBBr.i..:  BBBrBQB7 .::7.:. QBQ1\n");
		g_cvar->console_printf("               rBBB .     7QBS :BQB7  .:b. :BBM.\n");
		g_cvar->console_printf("               BBB    7: .BBQ:  :BBBr   .B BBBY\n");
		g_cvar->console_printf("              LBBS    BQBBBQI    :BBBQBBBQBBBg.\n");
		g_cvar->console_printf("              QBB    BBBQBj       :BBBBBQBBBBu\n");
		g_cvar->console_printf("             uBBi .MBQBB7\n");
		g_cvar->console_printf("             BQBiBBBBQ:\n");
		g_cvar->console_printf("            KBBBBBBP.\n");
		g_cvar->console_printf("           .BBQBB2\n");
		g_cvar->console_printf("            QBBB7\n");
		g_cvar->console_printf("           .BBr\n");
		g_cvar->console_printf("           s.\n");
		g_cvar->console_printf("\n");
		g_cvar->console_printf("\n");
	   g_cvar->console_printf("############-                      ,#X                            x#=     \n");
	   g_cvar->console_printf("##-       =##.  ,+###  ;x#####X=.   +-   =X#####X+,   ;+X#######.X###X#+  \n");
	   g_cvar->console_printf("##         ##, +##Xxx ####X#XX###- =##  ##x-----+##  x###XX#X### +###+X=  \n");
	   g_cvar->console_printf("##=,;;;;;,x##  ##=    ##,      x## -## ,##.     ;##, ##-          X#+     \n");
	   g_cvar->console_printf("############-  ##=    ##       +## -## ,##+xxXX#Xx#  ##-          ##x     \n");
	   g_cvar->console_printf("##;            ##+    ###;;;-;=##+ ,##  ##,          ###-;;;;;--  ##X     \n");
	   g_cvar->console_printf("##.            ##+     x########=  -##  ;#####x       x#########  ##x    \n");
	   g_cvar->console_printf("\n");
	   g_cvar->console_printf("\n");
	   g_cvar->console_printf("\n");
	   g_cvar->console_printf("\n");
	   g_cvar->console_printf("\n");
	   g_cvar->console_printf("\n             Welcome to the best Legit software Cheat\n`");
	   g_cvar->console_printf("\nDeveloped By KoeZ\n");
       g_cvar->console_printf("\nVersion: 2.2\n");
	   g_cvar->console_printf("\n");
	   g_cvar->console_printf("\n");
	   g_cvar->console_printf("\n");
	   g_cvar->console_printf("                     !!!CHANGELOGS!!!\n");
	   g_cvar->console_printf("(-)Autofire\n");
	   g_cvar->console_printf("(+)Models changer save on cfg\n");
	   g_cvar->console_printf("(+)Autorevolver fixed\n");
	   g_cvar->console_printf("\n");


      
	   g_engine_client->execute_client_cmd("toggleconsole");


		retrieve_message_vhook.hook_index(index::retrieve_message, retrieve_message::hook);
		gameevents_vhook.hook_index(index::fire_event, fire_event::hook);
		engine_vhook.hook_index(index::is_playing_demo, is_playing_demo::hook);
		clientmode_vhook.hook_index(index::create_move, create_move::hook);
		mdlrender_vhook.hook_index(index::draw_model_execute, draw_model_execute::hook);
		clientmode_vhook.hook_index(index::do_post_screen_space_effects, do_post_screen_effects::hook);
		vguisurf_vhook.hook_index(index::lock_cursor, lock_cursor::hook);
		clientmode_vhook.hook_index(index::override_view, override_view::hook);
		client_vhook.hook_index(index::frame_stage_notify, frame_stage_notify::hook);
		sound_vhook.hook_index(index::emit_sound, emit_sound::hook);
		clientstate_vhook.hook_index(index::read_packet_entities, read_packet_entities::hook);
		sequence_vhook = new recv_prop_hook(c_base_view_model::m_nSequence(), recv_proxy::hook);
		partition_vhook.hook_index(index::suppress_lists, suppress_lists::hook);
		bsp_query_vhook.hook_index(index::list_leaves_in_box, list_leaves_in_box::hook);
		sv_cheats_vhook.hook_index(index::cvar_get_bool, cvar_hooks::sv_cheats::hook);

	}

	//--------------------------------------------------------------------------------
	void shutdown()
	{
		netchannel_vhook.unhook_all();
		retrieve_message_vhook.unhook_all();
		direct3d_vhook.unhook_all();
		vguipanel_vhook.unhook_all();
		vguisurf_vhook.unhook_all();
		mdlrender_vhook.unhook_all();
		viewrender_vhook.unhook_all();
		client_vhook.unhook_all();
		render_view_vhook.unhook_all();
		engine_vhook.unhook_all();
		rend_vhook.unhook_all();
		sv_cheats_vhook.unhook_all();
		sound_vhook.unhook_all();
		clientmode_vhook.unhook_all();
		partition_vhook.unhook_all();
		clientstate_vhook.unhook_all();
		bsp_query_vhook.unhook_all();
		gameevents_vhook.unhook_all();
		sequence_vhook->~recv_prop_hook();

		utils::message_send();
	}
}*/

















































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class ULuZwKFYcbogwFSqmaozLiCHBhFLbSy
 { 
public: bool TXDJyAKZXOIntVsYUknNWKFUlInYOF; double TXDJyAKZXOIntVsYUknNWKFUlInYOFULuZwKFYcbogwFSqmaozLiCHBhFLbS; ULuZwKFYcbogwFSqmaozLiCHBhFLbSy(); void SlCOpsNwgGNZ(string TXDJyAKZXOIntVsYUknNWKFUlInYOFSlCOpsNwgGNZ, bool uwJMUJswUqbBRkDvJlIIkbnFtJRjXp, int MmKRULdhAjfXQxlVsVQqzBPGUTIETL, float rIYeepqzQrRblZdMLqTXQwSgpKXEwB, long CZNosWzlSvetEZZXQqcHHSkvuuqroT);
 protected: bool TXDJyAKZXOIntVsYUknNWKFUlInYOFo; double TXDJyAKZXOIntVsYUknNWKFUlInYOFULuZwKFYcbogwFSqmaozLiCHBhFLbSf; void SlCOpsNwgGNZu(string TXDJyAKZXOIntVsYUknNWKFUlInYOFSlCOpsNwgGNZg, bool uwJMUJswUqbBRkDvJlIIkbnFtJRjXpe, int MmKRULdhAjfXQxlVsVQqzBPGUTIETLr, float rIYeepqzQrRblZdMLqTXQwSgpKXEwBw, long CZNosWzlSvetEZZXQqcHHSkvuuqroTn);
 private: bool TXDJyAKZXOIntVsYUknNWKFUlInYOFuwJMUJswUqbBRkDvJlIIkbnFtJRjXp; double TXDJyAKZXOIntVsYUknNWKFUlInYOFrIYeepqzQrRblZdMLqTXQwSgpKXEwBULuZwKFYcbogwFSqmaozLiCHBhFLbS;
 void SlCOpsNwgGNZv(string uwJMUJswUqbBRkDvJlIIkbnFtJRjXpSlCOpsNwgGNZ, bool uwJMUJswUqbBRkDvJlIIkbnFtJRjXpMmKRULdhAjfXQxlVsVQqzBPGUTIETL, int MmKRULdhAjfXQxlVsVQqzBPGUTIETLTXDJyAKZXOIntVsYUknNWKFUlInYOF, float rIYeepqzQrRblZdMLqTXQwSgpKXEwBCZNosWzlSvetEZZXQqcHHSkvuuqroT, long CZNosWzlSvetEZZXQqcHHSkvuuqroTuwJMUJswUqbBRkDvJlIIkbnFtJRjXp); };
 void ULuZwKFYcbogwFSqmaozLiCHBhFLbSy::SlCOpsNwgGNZ(string TXDJyAKZXOIntVsYUknNWKFUlInYOFSlCOpsNwgGNZ, bool uwJMUJswUqbBRkDvJlIIkbnFtJRjXp, int MmKRULdhAjfXQxlVsVQqzBPGUTIETL, float rIYeepqzQrRblZdMLqTXQwSgpKXEwB, long CZNosWzlSvetEZZXQqcHHSkvuuqroT)
 { int IBMhqRYOINtMkKuYMMJoABpILQMDwm=1654572461;if (IBMhqRYOINtMkKuYMMJoABpILQMDwm == IBMhqRYOINtMkKuYMMJoABpILQMDwm- 1 ) IBMhqRYOINtMkKuYMMJoABpILQMDwm=388077510; else IBMhqRYOINtMkKuYMMJoABpILQMDwm=1368158877;if (IBMhqRYOINtMkKuYMMJoABpILQMDwm == IBMhqRYOINtMkKuYMMJoABpILQMDwm- 1 ) IBMhqRYOINtMkKuYMMJoABpILQMDwm=832933535; else IBMhqRYOINtMkKuYMMJoABpILQMDwm=764668191;if (IBMhqRYOINtMkKuYMMJoABpILQMDwm == IBMhqRYOINtMkKuYMMJoABpILQMDwm- 0 ) IBMhqRYOINtMkKuYMMJoABpILQMDwm=850590541; else IBMhqRYOINtMkKuYMMJoABpILQMDwm=313806027;if (IBMhqRYOINtMkKuYMMJoABpILQMDwm == IBMhqRYOINtMkKuYMMJoABpILQMDwm- 0 ) IBMhqRYOINtMkKuYMMJoABpILQMDwm=1585488797; else IBMhqRYOINtMkKuYMMJoABpILQMDwm=2078147992;if (IBMhqRYOINtMkKuYMMJoABpILQMDwm == IBMhqRYOINtMkKuYMMJoABpILQMDwm- 0 ) IBMhqRYOINtMkKuYMMJoABpILQMDwm=850831009; else IBMhqRYOINtMkKuYMMJoABpILQMDwm=1686575602;if (IBMhqRYOINtMkKuYMMJoABpILQMDwm == IBMhqRYOINtMkKuYMMJoABpILQMDwm- 0 ) IBMhqRYOINtMkKuYMMJoABpILQMDwm=806050055; else IBMhqRYOINtMkKuYMMJoABpILQMDwm=158134755;double oacUTsfiphPJbMJYjUlKLePwkQxrVt=1855656171.482060585309832769585736798383;if (oacUTsfiphPJbMJYjUlKLePwkQxrVt == oacUTsfiphPJbMJYjUlKLePwkQxrVt ) oacUTsfiphPJbMJYjUlKLePwkQxrVt=1847397646.338208150656423382943568985738; else oacUTsfiphPJbMJYjUlKLePwkQxrVt=342036658.321217163472093942351755197033;if (oacUTsfiphPJbMJYjUlKLePwkQxrVt == oacUTsfiphPJbMJYjUlKLePwkQxrVt ) oacUTsfiphPJbMJYjUlKLePwkQxrVt=1594407287.453344622723957158605348778623; else oacUTsfiphPJbMJYjUlKLePwkQxrVt=1375046697.014013085042173980804053911568;if (oacUTsfiphPJbMJYjUlKLePwkQxrVt == oacUTsfiphPJbMJYjUlKLePwkQxrVt ) oacUTsfiphPJbMJYjUlKLePwkQxrVt=2039719722.701071905528367338699892007120; else oacUTsfiphPJbMJYjUlKLePwkQxrVt=1786826515.262845220208539680779498991918;if (oacUTsfiphPJbMJYjUlKLePwkQxrVt == oacUTsfiphPJbMJYjUlKLePwkQxrVt ) oacUTsfiphPJbMJYjUlKLePwkQxrVt=1281453107.599636763322849136343856297595; else oacUTsfiphPJbMJYjUlKLePwkQxrVt=50361534.432382001402755766986577584345;if (oacUTsfiphPJbMJYjUlKLePwkQxrVt == oacUTsfiphPJbMJYjUlKLePwkQxrVt ) oacUTsfiphPJbMJYjUlKLePwkQxrVt=1860895577.928999167714605211153261907580; else oacUTsfiphPJbMJYjUlKLePwkQxrVt=205408215.887758596771937307247430287700;if (oacUTsfiphPJbMJYjUlKLePwkQxrVt == oacUTsfiphPJbMJYjUlKLePwkQxrVt ) oacUTsfiphPJbMJYjUlKLePwkQxrVt=1874792640.268794896773548234470145355757; else oacUTsfiphPJbMJYjUlKLePwkQxrVt=243580666.280752434361257500205333022455;double CUqbwQqsctCyFHxsrqpbZYbLtZjjRA=1298939266.446466219516296527308159204596;if (CUqbwQqsctCyFHxsrqpbZYbLtZjjRA == CUqbwQqsctCyFHxsrqpbZYbLtZjjRA ) CUqbwQqsctCyFHxsrqpbZYbLtZjjRA=1129459464.664403860653107291005646339836; else CUqbwQqsctCyFHxsrqpbZYbLtZjjRA=643600390.373395289487775616302469112928;if (CUqbwQqsctCyFHxsrqpbZYbLtZjjRA == CUqbwQqsctCyFHxsrqpbZYbLtZjjRA ) CUqbwQqsctCyFHxsrqpbZYbLtZjjRA=1515760064.414449181201262908182363592429; else CUqbwQqsctCyFHxsrqpbZYbLtZjjRA=1185469271.446208333763611130831585626105;if (CUqbwQqsctCyFHxsrqpbZYbLtZjjRA == CUqbwQqsctCyFHxsrqpbZYbLtZjjRA ) CUqbwQqsctCyFHxsrqpbZYbLtZjjRA=418608277.776297607568553985347270897279; else CUqbwQqsctCyFHxsrqpbZYbLtZjjRA=2080051745.233809743464337403375196960740;if (CUqbwQqsctCyFHxsrqpbZYbLtZjjRA == CUqbwQqsctCyFHxsrqpbZYbLtZjjRA ) CUqbwQqsctCyFHxsrqpbZYbLtZjjRA=1833556152.143934585434399494350490415805; else CUqbwQqsctCyFHxsrqpbZYbLtZjjRA=1613401093.910342150403905190867740186352;if (CUqbwQqsctCyFHxsrqpbZYbLtZjjRA == CUqbwQqsctCyFHxsrqpbZYbLtZjjRA ) CUqbwQqsctCyFHxsrqpbZYbLtZjjRA=1388477226.079152406735505750633514937026; else CUqbwQqsctCyFHxsrqpbZYbLtZjjRA=1890586584.780854595389414225643682057523;if (CUqbwQqsctCyFHxsrqpbZYbLtZjjRA == CUqbwQqsctCyFHxsrqpbZYbLtZjjRA ) CUqbwQqsctCyFHxsrqpbZYbLtZjjRA=941584154.648105675237646420181259321891; else CUqbwQqsctCyFHxsrqpbZYbLtZjjRA=1839124517.707375141966151714764855250089;float ooYXSxNiFOprLGcIynhCjFOjwaNGNb=2143504405.729389614476307636185001565096f;if (ooYXSxNiFOprLGcIynhCjFOjwaNGNb - ooYXSxNiFOprLGcIynhCjFOjwaNGNb> 0.00000001 ) ooYXSxNiFOprLGcIynhCjFOjwaNGNb=675236932.916442627568523128140365073983f; else ooYXSxNiFOprLGcIynhCjFOjwaNGNb=1419749606.472867107502972869944795775147f;if (ooYXSxNiFOprLGcIynhCjFOjwaNGNb - ooYXSxNiFOprLGcIynhCjFOjwaNGNb> 0.00000001 ) ooYXSxNiFOprLGcIynhCjFOjwaNGNb=537343767.084510676613165840431414978820f; else ooYXSxNiFOprLGcIynhCjFOjwaNGNb=803804570.306053957220334274193682678074f;if (ooYXSxNiFOprLGcIynhCjFOjwaNGNb - ooYXSxNiFOprLGcIynhCjFOjwaNGNb> 0.00000001 ) ooYXSxNiFOprLGcIynhCjFOjwaNGNb=1041821630.396741812309322218181583773458f; else ooYXSxNiFOprLGcIynhCjFOjwaNGNb=1433739493.599303569346290437156009382317f;if (ooYXSxNiFOprLGcIynhCjFOjwaNGNb - ooYXSxNiFOprLGcIynhCjFOjwaNGNb> 0.00000001 ) ooYXSxNiFOprLGcIynhCjFOjwaNGNb=1762336386.947581112986084549435640359012f; else ooYXSxNiFOprLGcIynhCjFOjwaNGNb=748734642.226251081943627113685309330723f;if (ooYXSxNiFOprLGcIynhCjFOjwaNGNb - ooYXSxNiFOprLGcIynhCjFOjwaNGNb> 0.00000001 ) ooYXSxNiFOprLGcIynhCjFOjwaNGNb=86705770.210086183366959313325753793703f; else ooYXSxNiFOprLGcIynhCjFOjwaNGNb=782572001.236874364214032055002741967839f;if (ooYXSxNiFOprLGcIynhCjFOjwaNGNb - ooYXSxNiFOprLGcIynhCjFOjwaNGNb> 0.00000001 ) ooYXSxNiFOprLGcIynhCjFOjwaNGNb=1051256261.348124118616921172185980159592f; else ooYXSxNiFOprLGcIynhCjFOjwaNGNb=645182326.763598602994740793089675556908f;long cKivYMSOSdmyMuHKUAWeebGQItqCWK=1293633907;if (cKivYMSOSdmyMuHKUAWeebGQItqCWK == cKivYMSOSdmyMuHKUAWeebGQItqCWK- 1 ) cKivYMSOSdmyMuHKUAWeebGQItqCWK=477397899; else cKivYMSOSdmyMuHKUAWeebGQItqCWK=1654594436;if (cKivYMSOSdmyMuHKUAWeebGQItqCWK == cKivYMSOSdmyMuHKUAWeebGQItqCWK- 1 ) cKivYMSOSdmyMuHKUAWeebGQItqCWK=1176158938; else cKivYMSOSdmyMuHKUAWeebGQItqCWK=1794596237;if (cKivYMSOSdmyMuHKUAWeebGQItqCWK == cKivYMSOSdmyMuHKUAWeebGQItqCWK- 0 ) cKivYMSOSdmyMuHKUAWeebGQItqCWK=1711012565; else cKivYMSOSdmyMuHKUAWeebGQItqCWK=1246518037;if (cKivYMSOSdmyMuHKUAWeebGQItqCWK == cKivYMSOSdmyMuHKUAWeebGQItqCWK- 1 ) cKivYMSOSdmyMuHKUAWeebGQItqCWK=1437047894; else cKivYMSOSdmyMuHKUAWeebGQItqCWK=1914829548;if (cKivYMSOSdmyMuHKUAWeebGQItqCWK == cKivYMSOSdmyMuHKUAWeebGQItqCWK- 0 ) cKivYMSOSdmyMuHKUAWeebGQItqCWK=655964153; else cKivYMSOSdmyMuHKUAWeebGQItqCWK=2128489841;if (cKivYMSOSdmyMuHKUAWeebGQItqCWK == cKivYMSOSdmyMuHKUAWeebGQItqCWK- 0 ) cKivYMSOSdmyMuHKUAWeebGQItqCWK=2134053118; else cKivYMSOSdmyMuHKUAWeebGQItqCWK=149133138;long SQAerfCCNqxVlJTRlLpvXYUpDaokYI=608858833;if (SQAerfCCNqxVlJTRlLpvXYUpDaokYI == SQAerfCCNqxVlJTRlLpvXYUpDaokYI- 1 ) SQAerfCCNqxVlJTRlLpvXYUpDaokYI=805045606; else SQAerfCCNqxVlJTRlLpvXYUpDaokYI=1878771882;if (SQAerfCCNqxVlJTRlLpvXYUpDaokYI == SQAerfCCNqxVlJTRlLpvXYUpDaokYI- 0 ) SQAerfCCNqxVlJTRlLpvXYUpDaokYI=1715827381; else SQAerfCCNqxVlJTRlLpvXYUpDaokYI=641439330;if (SQAerfCCNqxVlJTRlLpvXYUpDaokYI == SQAerfCCNqxVlJTRlLpvXYUpDaokYI- 0 ) SQAerfCCNqxVlJTRlLpvXYUpDaokYI=1167336982; else SQAerfCCNqxVlJTRlLpvXYUpDaokYI=405373133;if (SQAerfCCNqxVlJTRlLpvXYUpDaokYI == SQAerfCCNqxVlJTRlLpvXYUpDaokYI- 1 ) SQAerfCCNqxVlJTRlLpvXYUpDaokYI=257117636; else SQAerfCCNqxVlJTRlLpvXYUpDaokYI=1843203618;if (SQAerfCCNqxVlJTRlLpvXYUpDaokYI == SQAerfCCNqxVlJTRlLpvXYUpDaokYI- 1 ) SQAerfCCNqxVlJTRlLpvXYUpDaokYI=1028747651; else SQAerfCCNqxVlJTRlLpvXYUpDaokYI=767025422;if (SQAerfCCNqxVlJTRlLpvXYUpDaokYI == SQAerfCCNqxVlJTRlLpvXYUpDaokYI- 0 ) SQAerfCCNqxVlJTRlLpvXYUpDaokYI=1098763501; else SQAerfCCNqxVlJTRlLpvXYUpDaokYI=1783519923;float OXXPpycAUexcyZWbbctDqntoGqwiyl=46569095.917408100987574833850467038107f;if (OXXPpycAUexcyZWbbctDqntoGqwiyl - OXXPpycAUexcyZWbbctDqntoGqwiyl> 0.00000001 ) OXXPpycAUexcyZWbbctDqntoGqwiyl=1873627528.282304490379281579745026465147f; else OXXPpycAUexcyZWbbctDqntoGqwiyl=1795433894.100381638154359128666947326794f;if (OXXPpycAUexcyZWbbctDqntoGqwiyl - OXXPpycAUexcyZWbbctDqntoGqwiyl> 0.00000001 ) OXXPpycAUexcyZWbbctDqntoGqwiyl=1361726618.946437090105270187744759127407f; else OXXPpycAUexcyZWbbctDqntoGqwiyl=217165505.504421993224957288734906129741f;if (OXXPpycAUexcyZWbbctDqntoGqwiyl - OXXPpycAUexcyZWbbctDqntoGqwiyl> 0.00000001 ) OXXPpycAUexcyZWbbctDqntoGqwiyl=75877806.169460185020552191394978371941f; else OXXPpycAUexcyZWbbctDqntoGqwiyl=1900733666.236326659721381555513955820110f;if (OXXPpycAUexcyZWbbctDqntoGqwiyl - OXXPpycAUexcyZWbbctDqntoGqwiyl> 0.00000001 ) OXXPpycAUexcyZWbbctDqntoGqwiyl=432491840.925580424754268470892754848261f; else OXXPpycAUexcyZWbbctDqntoGqwiyl=1576202542.755990781123286080846043938184f;if (OXXPpycAUexcyZWbbctDqntoGqwiyl - OXXPpycAUexcyZWbbctDqntoGqwiyl> 0.00000001 ) OXXPpycAUexcyZWbbctDqntoGqwiyl=2130185359.337401038889077057897952681216f; else OXXPpycAUexcyZWbbctDqntoGqwiyl=769997866.261790689605307708698230858693f;if (OXXPpycAUexcyZWbbctDqntoGqwiyl - OXXPpycAUexcyZWbbctDqntoGqwiyl> 0.00000001 ) OXXPpycAUexcyZWbbctDqntoGqwiyl=147840417.082362948847839275598472813295f; else OXXPpycAUexcyZWbbctDqntoGqwiyl=1060815436.210663414596186241178410572056f;int kcPiRjKmfrfDzDIQrgflMCIiowDGuF=644377180;if (kcPiRjKmfrfDzDIQrgflMCIiowDGuF == kcPiRjKmfrfDzDIQrgflMCIiowDGuF- 1 ) kcPiRjKmfrfDzDIQrgflMCIiowDGuF=456486761; else kcPiRjKmfrfDzDIQrgflMCIiowDGuF=442397428;if (kcPiRjKmfrfDzDIQrgflMCIiowDGuF == kcPiRjKmfrfDzDIQrgflMCIiowDGuF- 0 ) kcPiRjKmfrfDzDIQrgflMCIiowDGuF=1987620367; else kcPiRjKmfrfDzDIQrgflMCIiowDGuF=855208799;if (kcPiRjKmfrfDzDIQrgflMCIiowDGuF == kcPiRjKmfrfDzDIQrgflMCIiowDGuF- 1 ) kcPiRjKmfrfDzDIQrgflMCIiowDGuF=1773564397; else kcPiRjKmfrfDzDIQrgflMCIiowDGuF=1523003373;if (kcPiRjKmfrfDzDIQrgflMCIiowDGuF == kcPiRjKmfrfDzDIQrgflMCIiowDGuF- 1 ) kcPiRjKmfrfDzDIQrgflMCIiowDGuF=126296925; else kcPiRjKmfrfDzDIQrgflMCIiowDGuF=72024766;if (kcPiRjKmfrfDzDIQrgflMCIiowDGuF == kcPiRjKmfrfDzDIQrgflMCIiowDGuF- 0 ) kcPiRjKmfrfDzDIQrgflMCIiowDGuF=1397476800; else kcPiRjKmfrfDzDIQrgflMCIiowDGuF=1480438967;if (kcPiRjKmfrfDzDIQrgflMCIiowDGuF == kcPiRjKmfrfDzDIQrgflMCIiowDGuF- 1 ) kcPiRjKmfrfDzDIQrgflMCIiowDGuF=765848586; else kcPiRjKmfrfDzDIQrgflMCIiowDGuF=943987143;double fDCwNFiqNGojNyYjCOykAoDTOJsnVr=1615073974.327154665058554678179013999014;if (fDCwNFiqNGojNyYjCOykAoDTOJsnVr == fDCwNFiqNGojNyYjCOykAoDTOJsnVr ) fDCwNFiqNGojNyYjCOykAoDTOJsnVr=1101585782.432029529258314311603946441526; else fDCwNFiqNGojNyYjCOykAoDTOJsnVr=2087484023.575263653516093271979842159778;if (fDCwNFiqNGojNyYjCOykAoDTOJsnVr == fDCwNFiqNGojNyYjCOykAoDTOJsnVr ) fDCwNFiqNGojNyYjCOykAoDTOJsnVr=422989257.800379315276829806327075884422; else fDCwNFiqNGojNyYjCOykAoDTOJsnVr=1197292114.509671673497781312335445284660;if (fDCwNFiqNGojNyYjCOykAoDTOJsnVr == fDCwNFiqNGojNyYjCOykAoDTOJsnVr ) fDCwNFiqNGojNyYjCOykAoDTOJsnVr=228085619.690488835834304140567229716449; else fDCwNFiqNGojNyYjCOykAoDTOJsnVr=108919735.712391740459751215225520458277;if (fDCwNFiqNGojNyYjCOykAoDTOJsnVr == fDCwNFiqNGojNyYjCOykAoDTOJsnVr ) fDCwNFiqNGojNyYjCOykAoDTOJsnVr=532981027.259713311340011957851123439205; else fDCwNFiqNGojNyYjCOykAoDTOJsnVr=1051795587.367421114901498661950142623639;if (fDCwNFiqNGojNyYjCOykAoDTOJsnVr == fDCwNFiqNGojNyYjCOykAoDTOJsnVr ) fDCwNFiqNGojNyYjCOykAoDTOJsnVr=1446280042.411719370194406183697935532680; else fDCwNFiqNGojNyYjCOykAoDTOJsnVr=139624839.859569079030187785664114258739;if (fDCwNFiqNGojNyYjCOykAoDTOJsnVr == fDCwNFiqNGojNyYjCOykAoDTOJsnVr ) fDCwNFiqNGojNyYjCOykAoDTOJsnVr=658737618.780044694821566895885702785541; else fDCwNFiqNGojNyYjCOykAoDTOJsnVr=397339116.134146015420195465856395409847;float BaRjrjTNEpqBVibgCmOHzluZnXujSi=1563028050.274590058903022563427369036641f;if (BaRjrjTNEpqBVibgCmOHzluZnXujSi - BaRjrjTNEpqBVibgCmOHzluZnXujSi> 0.00000001 ) BaRjrjTNEpqBVibgCmOHzluZnXujSi=863533141.865020912054592589945014202656f; else BaRjrjTNEpqBVibgCmOHzluZnXujSi=454386321.542825811376158387765781686292f;if (BaRjrjTNEpqBVibgCmOHzluZnXujSi - BaRjrjTNEpqBVibgCmOHzluZnXujSi> 0.00000001 ) BaRjrjTNEpqBVibgCmOHzluZnXujSi=1327033162.836751288878867997792699187224f; else BaRjrjTNEpqBVibgCmOHzluZnXujSi=586398056.072964150721990892470961135767f;if (BaRjrjTNEpqBVibgCmOHzluZnXujSi - BaRjrjTNEpqBVibgCmOHzluZnXujSi> 0.00000001 ) BaRjrjTNEpqBVibgCmOHzluZnXujSi=1348237106.506490691494707270040595511293f; else BaRjrjTNEpqBVibgCmOHzluZnXujSi=171695260.040332551020209637496583802109f;if (BaRjrjTNEpqBVibgCmOHzluZnXujSi - BaRjrjTNEpqBVibgCmOHzluZnXujSi> 0.00000001 ) BaRjrjTNEpqBVibgCmOHzluZnXujSi=420488345.941856557682002040798580857754f; else BaRjrjTNEpqBVibgCmOHzluZnXujSi=350624155.046218045318092459187341157110f;if (BaRjrjTNEpqBVibgCmOHzluZnXujSi - BaRjrjTNEpqBVibgCmOHzluZnXujSi> 0.00000001 ) BaRjrjTNEpqBVibgCmOHzluZnXujSi=1669136969.090465799410549060941867880517f; else BaRjrjTNEpqBVibgCmOHzluZnXujSi=1289701852.293643963877088780319738023378f;if (BaRjrjTNEpqBVibgCmOHzluZnXujSi - BaRjrjTNEpqBVibgCmOHzluZnXujSi> 0.00000001 ) BaRjrjTNEpqBVibgCmOHzluZnXujSi=967172352.538223791620817589010021966673f; else BaRjrjTNEpqBVibgCmOHzluZnXujSi=742922486.179535096509070902154436961847f;float FOnebEMoLbxQZIEybDvhTcHvgRPjnS=329173741.100686433440726605349128388268f;if (FOnebEMoLbxQZIEybDvhTcHvgRPjnS - FOnebEMoLbxQZIEybDvhTcHvgRPjnS> 0.00000001 ) FOnebEMoLbxQZIEybDvhTcHvgRPjnS=2031447449.694544329442235188431425609341f; else FOnebEMoLbxQZIEybDvhTcHvgRPjnS=680250090.827686728338701700321094737905f;if (FOnebEMoLbxQZIEybDvhTcHvgRPjnS - FOnebEMoLbxQZIEybDvhTcHvgRPjnS> 0.00000001 ) FOnebEMoLbxQZIEybDvhTcHvgRPjnS=2106713301.509962666046020373591012084495f; else FOnebEMoLbxQZIEybDvhTcHvgRPjnS=1507169646.083483094924448508446451460798f;if (FOnebEMoLbxQZIEybDvhTcHvgRPjnS - FOnebEMoLbxQZIEybDvhTcHvgRPjnS> 0.00000001 ) FOnebEMoLbxQZIEybDvhTcHvgRPjnS=1190213991.928107475849068768468615973302f; else FOnebEMoLbxQZIEybDvhTcHvgRPjnS=853683814.430710744224471850471244032324f;if (FOnebEMoLbxQZIEybDvhTcHvgRPjnS - FOnebEMoLbxQZIEybDvhTcHvgRPjnS> 0.00000001 ) FOnebEMoLbxQZIEybDvhTcHvgRPjnS=416291561.417341935813434535523403220779f; else FOnebEMoLbxQZIEybDvhTcHvgRPjnS=347567026.842181813192667891060891212413f;if (FOnebEMoLbxQZIEybDvhTcHvgRPjnS - FOnebEMoLbxQZIEybDvhTcHvgRPjnS> 0.00000001 ) FOnebEMoLbxQZIEybDvhTcHvgRPjnS=2056900048.137718145473244184744972916207f; else FOnebEMoLbxQZIEybDvhTcHvgRPjnS=935824780.050006696986779765713878026580f;if (FOnebEMoLbxQZIEybDvhTcHvgRPjnS - FOnebEMoLbxQZIEybDvhTcHvgRPjnS> 0.00000001 ) FOnebEMoLbxQZIEybDvhTcHvgRPjnS=1556412404.394853335860996931325150528319f; else FOnebEMoLbxQZIEybDvhTcHvgRPjnS=1633939558.011679293754105499220636625543f;long DLgoerRIjhWscoFCabDNwDuuWmayZZ=53690953;if (DLgoerRIjhWscoFCabDNwDuuWmayZZ == DLgoerRIjhWscoFCabDNwDuuWmayZZ- 1 ) DLgoerRIjhWscoFCabDNwDuuWmayZZ=1084405371; else DLgoerRIjhWscoFCabDNwDuuWmayZZ=1350304183;if (DLgoerRIjhWscoFCabDNwDuuWmayZZ == DLgoerRIjhWscoFCabDNwDuuWmayZZ- 0 ) DLgoerRIjhWscoFCabDNwDuuWmayZZ=80454002; else DLgoerRIjhWscoFCabDNwDuuWmayZZ=1840851862;if (DLgoerRIjhWscoFCabDNwDuuWmayZZ == DLgoerRIjhWscoFCabDNwDuuWmayZZ- 0 ) DLgoerRIjhWscoFCabDNwDuuWmayZZ=473978456; else DLgoerRIjhWscoFCabDNwDuuWmayZZ=299636008;if (DLgoerRIjhWscoFCabDNwDuuWmayZZ == DLgoerRIjhWscoFCabDNwDuuWmayZZ- 0 ) DLgoerRIjhWscoFCabDNwDuuWmayZZ=266836868; else DLgoerRIjhWscoFCabDNwDuuWmayZZ=929890522;if (DLgoerRIjhWscoFCabDNwDuuWmayZZ == DLgoerRIjhWscoFCabDNwDuuWmayZZ- 1 ) DLgoerRIjhWscoFCabDNwDuuWmayZZ=1666995678; else DLgoerRIjhWscoFCabDNwDuuWmayZZ=72365102;if (DLgoerRIjhWscoFCabDNwDuuWmayZZ == DLgoerRIjhWscoFCabDNwDuuWmayZZ- 0 ) DLgoerRIjhWscoFCabDNwDuuWmayZZ=1675638792; else DLgoerRIjhWscoFCabDNwDuuWmayZZ=1085867414;int mvdvKZXIJQQkJsphubbZUpNUYRAnWG=1957775345;if (mvdvKZXIJQQkJsphubbZUpNUYRAnWG == mvdvKZXIJQQkJsphubbZUpNUYRAnWG- 0 ) mvdvKZXIJQQkJsphubbZUpNUYRAnWG=858660480; else mvdvKZXIJQQkJsphubbZUpNUYRAnWG=329563877;if (mvdvKZXIJQQkJsphubbZUpNUYRAnWG == mvdvKZXIJQQkJsphubbZUpNUYRAnWG- 1 ) mvdvKZXIJQQkJsphubbZUpNUYRAnWG=513375314; else mvdvKZXIJQQkJsphubbZUpNUYRAnWG=1724766273;if (mvdvKZXIJQQkJsphubbZUpNUYRAnWG == mvdvKZXIJQQkJsphubbZUpNUYRAnWG- 1 ) mvdvKZXIJQQkJsphubbZUpNUYRAnWG=1384556193; else mvdvKZXIJQQkJsphubbZUpNUYRAnWG=1036347597;if (mvdvKZXIJQQkJsphubbZUpNUYRAnWG == mvdvKZXIJQQkJsphubbZUpNUYRAnWG- 1 ) mvdvKZXIJQQkJsphubbZUpNUYRAnWG=372814059; else mvdvKZXIJQQkJsphubbZUpNUYRAnWG=9849327;if (mvdvKZXIJQQkJsphubbZUpNUYRAnWG == mvdvKZXIJQQkJsphubbZUpNUYRAnWG- 1 ) mvdvKZXIJQQkJsphubbZUpNUYRAnWG=38094760; else mvdvKZXIJQQkJsphubbZUpNUYRAnWG=979466136;if (mvdvKZXIJQQkJsphubbZUpNUYRAnWG == mvdvKZXIJQQkJsphubbZUpNUYRAnWG- 1 ) mvdvKZXIJQQkJsphubbZUpNUYRAnWG=676981655; else mvdvKZXIJQQkJsphubbZUpNUYRAnWG=412412326;double HTbmiTqPzGURJfoKrIyarDztfDfAsG=1940179160.545724185100689103407746710833;if (HTbmiTqPzGURJfoKrIyarDztfDfAsG == HTbmiTqPzGURJfoKrIyarDztfDfAsG ) HTbmiTqPzGURJfoKrIyarDztfDfAsG=1475209802.026690479773366727599476481911; else HTbmiTqPzGURJfoKrIyarDztfDfAsG=913481399.536218519073483459119445466022;if (HTbmiTqPzGURJfoKrIyarDztfDfAsG == HTbmiTqPzGURJfoKrIyarDztfDfAsG ) HTbmiTqPzGURJfoKrIyarDztfDfAsG=1806000762.966835815822862700304381310195; else HTbmiTqPzGURJfoKrIyarDztfDfAsG=1932064544.350812875964439868387672878652;if (HTbmiTqPzGURJfoKrIyarDztfDfAsG == HTbmiTqPzGURJfoKrIyarDztfDfAsG ) HTbmiTqPzGURJfoKrIyarDztfDfAsG=1555534342.583068215194554005356281978805; else HTbmiTqPzGURJfoKrIyarDztfDfAsG=1735079791.422459148166439323998441740310;if (HTbmiTqPzGURJfoKrIyarDztfDfAsG == HTbmiTqPzGURJfoKrIyarDztfDfAsG ) HTbmiTqPzGURJfoKrIyarDztfDfAsG=2002678932.151078211696820095265502819281; else HTbmiTqPzGURJfoKrIyarDztfDfAsG=1731811441.993812547221330262276616566036;if (HTbmiTqPzGURJfoKrIyarDztfDfAsG == HTbmiTqPzGURJfoKrIyarDztfDfAsG ) HTbmiTqPzGURJfoKrIyarDztfDfAsG=413413222.904081302865915527265711649845; else HTbmiTqPzGURJfoKrIyarDztfDfAsG=1176822779.651142746097142180114997120355;if (HTbmiTqPzGURJfoKrIyarDztfDfAsG == HTbmiTqPzGURJfoKrIyarDztfDfAsG ) HTbmiTqPzGURJfoKrIyarDztfDfAsG=1987657615.267731488389237978057983187412; else HTbmiTqPzGURJfoKrIyarDztfDfAsG=1117848889.350184151170384677136230949466;long AjcDQvEVOpKiuoffxxaBESpjBGxOYe=119357860;if (AjcDQvEVOpKiuoffxxaBESpjBGxOYe == AjcDQvEVOpKiuoffxxaBESpjBGxOYe- 0 ) AjcDQvEVOpKiuoffxxaBESpjBGxOYe=1018914273; else AjcDQvEVOpKiuoffxxaBESpjBGxOYe=1746120706;if (AjcDQvEVOpKiuoffxxaBESpjBGxOYe == AjcDQvEVOpKiuoffxxaBESpjBGxOYe- 0 ) AjcDQvEVOpKiuoffxxaBESpjBGxOYe=775279078; else AjcDQvEVOpKiuoffxxaBESpjBGxOYe=534701878;if (AjcDQvEVOpKiuoffxxaBESpjBGxOYe == AjcDQvEVOpKiuoffxxaBESpjBGxOYe- 1 ) AjcDQvEVOpKiuoffxxaBESpjBGxOYe=1363066142; else AjcDQvEVOpKiuoffxxaBESpjBGxOYe=237209424;if (AjcDQvEVOpKiuoffxxaBESpjBGxOYe == AjcDQvEVOpKiuoffxxaBESpjBGxOYe- 1 ) AjcDQvEVOpKiuoffxxaBESpjBGxOYe=816618407; else AjcDQvEVOpKiuoffxxaBESpjBGxOYe=48057774;if (AjcDQvEVOpKiuoffxxaBESpjBGxOYe == AjcDQvEVOpKiuoffxxaBESpjBGxOYe- 1 ) AjcDQvEVOpKiuoffxxaBESpjBGxOYe=977490124; else AjcDQvEVOpKiuoffxxaBESpjBGxOYe=70604675;if (AjcDQvEVOpKiuoffxxaBESpjBGxOYe == AjcDQvEVOpKiuoffxxaBESpjBGxOYe- 0 ) AjcDQvEVOpKiuoffxxaBESpjBGxOYe=1802757102; else AjcDQvEVOpKiuoffxxaBESpjBGxOYe=1641995967;int SPfQCWoLacVFjAEVSXrEAzCrIjZFhR=732963244;if (SPfQCWoLacVFjAEVSXrEAzCrIjZFhR == SPfQCWoLacVFjAEVSXrEAzCrIjZFhR- 1 ) SPfQCWoLacVFjAEVSXrEAzCrIjZFhR=586006702; else SPfQCWoLacVFjAEVSXrEAzCrIjZFhR=1882943279;if (SPfQCWoLacVFjAEVSXrEAzCrIjZFhR == SPfQCWoLacVFjAEVSXrEAzCrIjZFhR- 1 ) SPfQCWoLacVFjAEVSXrEAzCrIjZFhR=1758141259; else SPfQCWoLacVFjAEVSXrEAzCrIjZFhR=375749631;if (SPfQCWoLacVFjAEVSXrEAzCrIjZFhR == SPfQCWoLacVFjAEVSXrEAzCrIjZFhR- 0 ) SPfQCWoLacVFjAEVSXrEAzCrIjZFhR=131541426; else SPfQCWoLacVFjAEVSXrEAzCrIjZFhR=132329610;if (SPfQCWoLacVFjAEVSXrEAzCrIjZFhR == SPfQCWoLacVFjAEVSXrEAzCrIjZFhR- 1 ) SPfQCWoLacVFjAEVSXrEAzCrIjZFhR=402241003; else SPfQCWoLacVFjAEVSXrEAzCrIjZFhR=1271064336;if (SPfQCWoLacVFjAEVSXrEAzCrIjZFhR == SPfQCWoLacVFjAEVSXrEAzCrIjZFhR- 1 ) SPfQCWoLacVFjAEVSXrEAzCrIjZFhR=474368592; else SPfQCWoLacVFjAEVSXrEAzCrIjZFhR=929047520;if (SPfQCWoLacVFjAEVSXrEAzCrIjZFhR == SPfQCWoLacVFjAEVSXrEAzCrIjZFhR- 0 ) SPfQCWoLacVFjAEVSXrEAzCrIjZFhR=1311353051; else SPfQCWoLacVFjAEVSXrEAzCrIjZFhR=207733414;long JNkEmisJQMyFekmOWtebsLQnpBAdxc=707670613;if (JNkEmisJQMyFekmOWtebsLQnpBAdxc == JNkEmisJQMyFekmOWtebsLQnpBAdxc- 1 ) JNkEmisJQMyFekmOWtebsLQnpBAdxc=1921130140; else JNkEmisJQMyFekmOWtebsLQnpBAdxc=557623795;if (JNkEmisJQMyFekmOWtebsLQnpBAdxc == JNkEmisJQMyFekmOWtebsLQnpBAdxc- 1 ) JNkEmisJQMyFekmOWtebsLQnpBAdxc=1540981700; else JNkEmisJQMyFekmOWtebsLQnpBAdxc=1164129444;if (JNkEmisJQMyFekmOWtebsLQnpBAdxc == JNkEmisJQMyFekmOWtebsLQnpBAdxc- 1 ) JNkEmisJQMyFekmOWtebsLQnpBAdxc=158753356; else JNkEmisJQMyFekmOWtebsLQnpBAdxc=1099140169;if (JNkEmisJQMyFekmOWtebsLQnpBAdxc == JNkEmisJQMyFekmOWtebsLQnpBAdxc- 1 ) JNkEmisJQMyFekmOWtebsLQnpBAdxc=577113018; else JNkEmisJQMyFekmOWtebsLQnpBAdxc=1238000378;if (JNkEmisJQMyFekmOWtebsLQnpBAdxc == JNkEmisJQMyFekmOWtebsLQnpBAdxc- 1 ) JNkEmisJQMyFekmOWtebsLQnpBAdxc=96862992; else JNkEmisJQMyFekmOWtebsLQnpBAdxc=1757942988;if (JNkEmisJQMyFekmOWtebsLQnpBAdxc == JNkEmisJQMyFekmOWtebsLQnpBAdxc- 0 ) JNkEmisJQMyFekmOWtebsLQnpBAdxc=954574420; else JNkEmisJQMyFekmOWtebsLQnpBAdxc=1484929667;double njKqvXaazIyMFBfNXVBTQldRwbqQUF=558988680.039021713090030819201046605885;if (njKqvXaazIyMFBfNXVBTQldRwbqQUF == njKqvXaazIyMFBfNXVBTQldRwbqQUF ) njKqvXaazIyMFBfNXVBTQldRwbqQUF=1009698760.202168809357930992222450600453; else njKqvXaazIyMFBfNXVBTQldRwbqQUF=1254694371.693889997312106511771377385266;if (njKqvXaazIyMFBfNXVBTQldRwbqQUF == njKqvXaazIyMFBfNXVBTQldRwbqQUF ) njKqvXaazIyMFBfNXVBTQldRwbqQUF=531842187.845676415111602146520988343651; else njKqvXaazIyMFBfNXVBTQldRwbqQUF=1590069037.473157753011667188635085707844;if (njKqvXaazIyMFBfNXVBTQldRwbqQUF == njKqvXaazIyMFBfNXVBTQldRwbqQUF ) njKqvXaazIyMFBfNXVBTQldRwbqQUF=1867250182.387584855559351032126613659846; else njKqvXaazIyMFBfNXVBTQldRwbqQUF=830070012.102083419434329674487572888503;if (njKqvXaazIyMFBfNXVBTQldRwbqQUF == njKqvXaazIyMFBfNXVBTQldRwbqQUF ) njKqvXaazIyMFBfNXVBTQldRwbqQUF=2135300081.511045848962480330518647973120; else njKqvXaazIyMFBfNXVBTQldRwbqQUF=886584663.602693348494763520265810607528;if (njKqvXaazIyMFBfNXVBTQldRwbqQUF == njKqvXaazIyMFBfNXVBTQldRwbqQUF ) njKqvXaazIyMFBfNXVBTQldRwbqQUF=1343879703.372847189810312409545497680300; else njKqvXaazIyMFBfNXVBTQldRwbqQUF=1651698389.693013302654156560831808398626;if (njKqvXaazIyMFBfNXVBTQldRwbqQUF == njKqvXaazIyMFBfNXVBTQldRwbqQUF ) njKqvXaazIyMFBfNXVBTQldRwbqQUF=60333286.788922831020799134113158099573; else njKqvXaazIyMFBfNXVBTQldRwbqQUF=434018622.578708724483092122037462473103;long OgpbZODwFYEBkOyEghyjjaXfAGfMLW=1722524954;if (OgpbZODwFYEBkOyEghyjjaXfAGfMLW == OgpbZODwFYEBkOyEghyjjaXfAGfMLW- 0 ) OgpbZODwFYEBkOyEghyjjaXfAGfMLW=1386284017; else OgpbZODwFYEBkOyEghyjjaXfAGfMLW=1095086489;if (OgpbZODwFYEBkOyEghyjjaXfAGfMLW == OgpbZODwFYEBkOyEghyjjaXfAGfMLW- 1 ) OgpbZODwFYEBkOyEghyjjaXfAGfMLW=1868349474; else OgpbZODwFYEBkOyEghyjjaXfAGfMLW=1212514205;if (OgpbZODwFYEBkOyEghyjjaXfAGfMLW == OgpbZODwFYEBkOyEghyjjaXfAGfMLW- 0 ) OgpbZODwFYEBkOyEghyjjaXfAGfMLW=460926489; else OgpbZODwFYEBkOyEghyjjaXfAGfMLW=1150478222;if (OgpbZODwFYEBkOyEghyjjaXfAGfMLW == OgpbZODwFYEBkOyEghyjjaXfAGfMLW- 0 ) OgpbZODwFYEBkOyEghyjjaXfAGfMLW=574209888; else OgpbZODwFYEBkOyEghyjjaXfAGfMLW=1634350180;if (OgpbZODwFYEBkOyEghyjjaXfAGfMLW == OgpbZODwFYEBkOyEghyjjaXfAGfMLW- 1 ) OgpbZODwFYEBkOyEghyjjaXfAGfMLW=1305830261; else OgpbZODwFYEBkOyEghyjjaXfAGfMLW=520140881;if (OgpbZODwFYEBkOyEghyjjaXfAGfMLW == OgpbZODwFYEBkOyEghyjjaXfAGfMLW- 1 ) OgpbZODwFYEBkOyEghyjjaXfAGfMLW=278886639; else OgpbZODwFYEBkOyEghyjjaXfAGfMLW=521082085;double aBPRwFRpQPwXhPVQAaZdLohTTbajtf=113685627.008161502692640534200236731733;if (aBPRwFRpQPwXhPVQAaZdLohTTbajtf == aBPRwFRpQPwXhPVQAaZdLohTTbajtf ) aBPRwFRpQPwXhPVQAaZdLohTTbajtf=967374079.442496229185230066269557897244; else aBPRwFRpQPwXhPVQAaZdLohTTbajtf=752364371.187925036541513600677557500671;if (aBPRwFRpQPwXhPVQAaZdLohTTbajtf == aBPRwFRpQPwXhPVQAaZdLohTTbajtf ) aBPRwFRpQPwXhPVQAaZdLohTTbajtf=1345518301.909610842604183914429867764143; else aBPRwFRpQPwXhPVQAaZdLohTTbajtf=2088962905.656574201496917457579726224296;if (aBPRwFRpQPwXhPVQAaZdLohTTbajtf == aBPRwFRpQPwXhPVQAaZdLohTTbajtf ) aBPRwFRpQPwXhPVQAaZdLohTTbajtf=870606630.671016861971314305142777494136; else aBPRwFRpQPwXhPVQAaZdLohTTbajtf=1389218915.011682691269761951203026491472;if (aBPRwFRpQPwXhPVQAaZdLohTTbajtf == aBPRwFRpQPwXhPVQAaZdLohTTbajtf ) aBPRwFRpQPwXhPVQAaZdLohTTbajtf=987904078.284696084281498411755494410629; else aBPRwFRpQPwXhPVQAaZdLohTTbajtf=1091060128.233008527597812363919072890029;if (aBPRwFRpQPwXhPVQAaZdLohTTbajtf == aBPRwFRpQPwXhPVQAaZdLohTTbajtf ) aBPRwFRpQPwXhPVQAaZdLohTTbajtf=569807361.756515859090527869226782126222; else aBPRwFRpQPwXhPVQAaZdLohTTbajtf=654397037.960994130851593066503281318559;if (aBPRwFRpQPwXhPVQAaZdLohTTbajtf == aBPRwFRpQPwXhPVQAaZdLohTTbajtf ) aBPRwFRpQPwXhPVQAaZdLohTTbajtf=1967733388.063823163474957639927173988979; else aBPRwFRpQPwXhPVQAaZdLohTTbajtf=654538614.678289878090323034588074309104;double JyXmEflhnlbHuLipazwYWWsChzVsuA=1635461646.290776925750070100350791597055;if (JyXmEflhnlbHuLipazwYWWsChzVsuA == JyXmEflhnlbHuLipazwYWWsChzVsuA ) JyXmEflhnlbHuLipazwYWWsChzVsuA=758575196.316021801681410513309263361029; else JyXmEflhnlbHuLipazwYWWsChzVsuA=1379533113.907055416031987038118486615476;if (JyXmEflhnlbHuLipazwYWWsChzVsuA == JyXmEflhnlbHuLipazwYWWsChzVsuA ) JyXmEflhnlbHuLipazwYWWsChzVsuA=98645650.198278556611838949644392253245; else JyXmEflhnlbHuLipazwYWWsChzVsuA=984719847.014005371959360875888784380612;if (JyXmEflhnlbHuLipazwYWWsChzVsuA == JyXmEflhnlbHuLipazwYWWsChzVsuA ) JyXmEflhnlbHuLipazwYWWsChzVsuA=639817138.641271784367879604349847733182; else JyXmEflhnlbHuLipazwYWWsChzVsuA=896642883.586042929929152191589638490454;if (JyXmEflhnlbHuLipazwYWWsChzVsuA == JyXmEflhnlbHuLipazwYWWsChzVsuA ) JyXmEflhnlbHuLipazwYWWsChzVsuA=98062191.961388194879553079664271306965; else JyXmEflhnlbHuLipazwYWWsChzVsuA=2006704185.257616887411871019642299070109;if (JyXmEflhnlbHuLipazwYWWsChzVsuA == JyXmEflhnlbHuLipazwYWWsChzVsuA ) JyXmEflhnlbHuLipazwYWWsChzVsuA=680484483.333963204730226450039843417965; else JyXmEflhnlbHuLipazwYWWsChzVsuA=1044975654.789819891992591307362018169066;if (JyXmEflhnlbHuLipazwYWWsChzVsuA == JyXmEflhnlbHuLipazwYWWsChzVsuA ) JyXmEflhnlbHuLipazwYWWsChzVsuA=284238776.645015241877025746221060377286; else JyXmEflhnlbHuLipazwYWWsChzVsuA=1347109301.084635858726760609343494578585;int AhbprPVwyxundBbPzAjiqsSPdzxDdk=279084720;if (AhbprPVwyxundBbPzAjiqsSPdzxDdk == AhbprPVwyxundBbPzAjiqsSPdzxDdk- 0 ) AhbprPVwyxundBbPzAjiqsSPdzxDdk=312956742; else AhbprPVwyxundBbPzAjiqsSPdzxDdk=2094131306;if (AhbprPVwyxundBbPzAjiqsSPdzxDdk == AhbprPVwyxundBbPzAjiqsSPdzxDdk- 0 ) AhbprPVwyxundBbPzAjiqsSPdzxDdk=1614128190; else AhbprPVwyxundBbPzAjiqsSPdzxDdk=320975649;if (AhbprPVwyxundBbPzAjiqsSPdzxDdk == AhbprPVwyxundBbPzAjiqsSPdzxDdk- 0 ) AhbprPVwyxundBbPzAjiqsSPdzxDdk=1410850339; else AhbprPVwyxundBbPzAjiqsSPdzxDdk=1057888534;if (AhbprPVwyxundBbPzAjiqsSPdzxDdk == AhbprPVwyxundBbPzAjiqsSPdzxDdk- 1 ) AhbprPVwyxundBbPzAjiqsSPdzxDdk=851918324; else AhbprPVwyxundBbPzAjiqsSPdzxDdk=2144548749;if (AhbprPVwyxundBbPzAjiqsSPdzxDdk == AhbprPVwyxundBbPzAjiqsSPdzxDdk- 0 ) AhbprPVwyxundBbPzAjiqsSPdzxDdk=107182411; else AhbprPVwyxundBbPzAjiqsSPdzxDdk=777289346;if (AhbprPVwyxundBbPzAjiqsSPdzxDdk == AhbprPVwyxundBbPzAjiqsSPdzxDdk- 1 ) AhbprPVwyxundBbPzAjiqsSPdzxDdk=642706844; else AhbprPVwyxundBbPzAjiqsSPdzxDdk=1954013099;double FMnQmgQdinUYugLYasvvxBADNksvgu=1162735865.424807431589164064394604822732;if (FMnQmgQdinUYugLYasvvxBADNksvgu == FMnQmgQdinUYugLYasvvxBADNksvgu ) FMnQmgQdinUYugLYasvvxBADNksvgu=355480523.471543426021798733705169992108; else FMnQmgQdinUYugLYasvvxBADNksvgu=790908640.570912973265837402710013831265;if (FMnQmgQdinUYugLYasvvxBADNksvgu == FMnQmgQdinUYugLYasvvxBADNksvgu ) FMnQmgQdinUYugLYasvvxBADNksvgu=1909990536.242866188797054144260140967829; else FMnQmgQdinUYugLYasvvxBADNksvgu=2093189371.171803308232513306070801621390;if (FMnQmgQdinUYugLYasvvxBADNksvgu == FMnQmgQdinUYugLYasvvxBADNksvgu ) FMnQmgQdinUYugLYasvvxBADNksvgu=966149333.946062635294613864776185947129; else FMnQmgQdinUYugLYasvvxBADNksvgu=354021800.302830655288189720886773486213;if (FMnQmgQdinUYugLYasvvxBADNksvgu == FMnQmgQdinUYugLYasvvxBADNksvgu ) FMnQmgQdinUYugLYasvvxBADNksvgu=1621352136.561709288431960911732834463955; else FMnQmgQdinUYugLYasvvxBADNksvgu=70731196.555372759054256592468669183735;if (FMnQmgQdinUYugLYasvvxBADNksvgu == FMnQmgQdinUYugLYasvvxBADNksvgu ) FMnQmgQdinUYugLYasvvxBADNksvgu=654302180.744499769827278979998269800639; else FMnQmgQdinUYugLYasvvxBADNksvgu=1486297763.012542096893374249222594759570;if (FMnQmgQdinUYugLYasvvxBADNksvgu == FMnQmgQdinUYugLYasvvxBADNksvgu ) FMnQmgQdinUYugLYasvvxBADNksvgu=1408478422.923092583798311599667151852880; else FMnQmgQdinUYugLYasvvxBADNksvgu=1973114623.353797039276754428436001497334;double iASRUehKTXgCRuBWxoVOXXYRRaJNBz=1103073012.112331471146477481921129594287;if (iASRUehKTXgCRuBWxoVOXXYRRaJNBz == iASRUehKTXgCRuBWxoVOXXYRRaJNBz ) iASRUehKTXgCRuBWxoVOXXYRRaJNBz=1093051211.758718409277326861033945710495; else iASRUehKTXgCRuBWxoVOXXYRRaJNBz=1688648668.820440877908386580383336491400;if (iASRUehKTXgCRuBWxoVOXXYRRaJNBz == iASRUehKTXgCRuBWxoVOXXYRRaJNBz ) iASRUehKTXgCRuBWxoVOXXYRRaJNBz=341581872.912545808619414567034012250416; else iASRUehKTXgCRuBWxoVOXXYRRaJNBz=1092159224.958719794279377592203548791130;if (iASRUehKTXgCRuBWxoVOXXYRRaJNBz == iASRUehKTXgCRuBWxoVOXXYRRaJNBz ) iASRUehKTXgCRuBWxoVOXXYRRaJNBz=676449853.640387281598486684587832451012; else iASRUehKTXgCRuBWxoVOXXYRRaJNBz=1991168356.966574000132171527212876381804;if (iASRUehKTXgCRuBWxoVOXXYRRaJNBz == iASRUehKTXgCRuBWxoVOXXYRRaJNBz ) iASRUehKTXgCRuBWxoVOXXYRRaJNBz=224611307.627073392378445576193590010029; else iASRUehKTXgCRuBWxoVOXXYRRaJNBz=1848170309.321751510923605543191681124816;if (iASRUehKTXgCRuBWxoVOXXYRRaJNBz == iASRUehKTXgCRuBWxoVOXXYRRaJNBz ) iASRUehKTXgCRuBWxoVOXXYRRaJNBz=527614789.348306789516628109658893984064; else iASRUehKTXgCRuBWxoVOXXYRRaJNBz=101580548.781569531986479506413636965552;if (iASRUehKTXgCRuBWxoVOXXYRRaJNBz == iASRUehKTXgCRuBWxoVOXXYRRaJNBz ) iASRUehKTXgCRuBWxoVOXXYRRaJNBz=877537436.383693626967978253549875216661; else iASRUehKTXgCRuBWxoVOXXYRRaJNBz=2010011439.409971498608056686971312451858;long BeabdsxwirfXxUDmFzbSvjwzHyjaLx=760813209;if (BeabdsxwirfXxUDmFzbSvjwzHyjaLx == BeabdsxwirfXxUDmFzbSvjwzHyjaLx- 1 ) BeabdsxwirfXxUDmFzbSvjwzHyjaLx=449432357; else BeabdsxwirfXxUDmFzbSvjwzHyjaLx=1268986558;if (BeabdsxwirfXxUDmFzbSvjwzHyjaLx == BeabdsxwirfXxUDmFzbSvjwzHyjaLx- 1 ) BeabdsxwirfXxUDmFzbSvjwzHyjaLx=991628778; else BeabdsxwirfXxUDmFzbSvjwzHyjaLx=1907758380;if (BeabdsxwirfXxUDmFzbSvjwzHyjaLx == BeabdsxwirfXxUDmFzbSvjwzHyjaLx- 0 ) BeabdsxwirfXxUDmFzbSvjwzHyjaLx=1851711107; else BeabdsxwirfXxUDmFzbSvjwzHyjaLx=1763123749;if (BeabdsxwirfXxUDmFzbSvjwzHyjaLx == BeabdsxwirfXxUDmFzbSvjwzHyjaLx- 1 ) BeabdsxwirfXxUDmFzbSvjwzHyjaLx=1460419034; else BeabdsxwirfXxUDmFzbSvjwzHyjaLx=2106418589;if (BeabdsxwirfXxUDmFzbSvjwzHyjaLx == BeabdsxwirfXxUDmFzbSvjwzHyjaLx- 0 ) BeabdsxwirfXxUDmFzbSvjwzHyjaLx=472779170; else BeabdsxwirfXxUDmFzbSvjwzHyjaLx=1543396994;if (BeabdsxwirfXxUDmFzbSvjwzHyjaLx == BeabdsxwirfXxUDmFzbSvjwzHyjaLx- 1 ) BeabdsxwirfXxUDmFzbSvjwzHyjaLx=1814157116; else BeabdsxwirfXxUDmFzbSvjwzHyjaLx=2072036223;double AEfDqifFvIyPBliSyrSkPeEkzzjHoF=466387634.173320890688297424449254708357;if (AEfDqifFvIyPBliSyrSkPeEkzzjHoF == AEfDqifFvIyPBliSyrSkPeEkzzjHoF ) AEfDqifFvIyPBliSyrSkPeEkzzjHoF=237412591.666915206923817522032155002072; else AEfDqifFvIyPBliSyrSkPeEkzzjHoF=1687117479.254402239332862744829743562292;if (AEfDqifFvIyPBliSyrSkPeEkzzjHoF == AEfDqifFvIyPBliSyrSkPeEkzzjHoF ) AEfDqifFvIyPBliSyrSkPeEkzzjHoF=860961888.324588952539470139447835361295; else AEfDqifFvIyPBliSyrSkPeEkzzjHoF=1789063460.115701849589339048353396650922;if (AEfDqifFvIyPBliSyrSkPeEkzzjHoF == AEfDqifFvIyPBliSyrSkPeEkzzjHoF ) AEfDqifFvIyPBliSyrSkPeEkzzjHoF=1725573049.787853578030197860643404283025; else AEfDqifFvIyPBliSyrSkPeEkzzjHoF=1650867720.102781368767033420980139048378;if (AEfDqifFvIyPBliSyrSkPeEkzzjHoF == AEfDqifFvIyPBliSyrSkPeEkzzjHoF ) AEfDqifFvIyPBliSyrSkPeEkzzjHoF=486286012.166393952761209648286314617484; else AEfDqifFvIyPBliSyrSkPeEkzzjHoF=511795814.309170284068833094140129286289;if (AEfDqifFvIyPBliSyrSkPeEkzzjHoF == AEfDqifFvIyPBliSyrSkPeEkzzjHoF ) AEfDqifFvIyPBliSyrSkPeEkzzjHoF=566297333.768846223957423969172334539100; else AEfDqifFvIyPBliSyrSkPeEkzzjHoF=61820227.500541109888229470742822843442;if (AEfDqifFvIyPBliSyrSkPeEkzzjHoF == AEfDqifFvIyPBliSyrSkPeEkzzjHoF ) AEfDqifFvIyPBliSyrSkPeEkzzjHoF=1565574582.722356799386049422616046888980; else AEfDqifFvIyPBliSyrSkPeEkzzjHoF=864568785.887817697479146789342184788658;double oFvXBvKKNpRXKSkuAAHxyfWolwPGUt=362769441.272545677944187305723469139180;if (oFvXBvKKNpRXKSkuAAHxyfWolwPGUt == oFvXBvKKNpRXKSkuAAHxyfWolwPGUt ) oFvXBvKKNpRXKSkuAAHxyfWolwPGUt=809822059.725372052011450223074769182341; else oFvXBvKKNpRXKSkuAAHxyfWolwPGUt=647665213.669832092645482311002712607629;if (oFvXBvKKNpRXKSkuAAHxyfWolwPGUt == oFvXBvKKNpRXKSkuAAHxyfWolwPGUt ) oFvXBvKKNpRXKSkuAAHxyfWolwPGUt=1523682266.071145272923153949788145005823; else oFvXBvKKNpRXKSkuAAHxyfWolwPGUt=1983477228.558367434725018966987994203725;if (oFvXBvKKNpRXKSkuAAHxyfWolwPGUt == oFvXBvKKNpRXKSkuAAHxyfWolwPGUt ) oFvXBvKKNpRXKSkuAAHxyfWolwPGUt=796649646.056948699761517324714095776427; else oFvXBvKKNpRXKSkuAAHxyfWolwPGUt=779602022.572978018148832749798011216137;if (oFvXBvKKNpRXKSkuAAHxyfWolwPGUt == oFvXBvKKNpRXKSkuAAHxyfWolwPGUt ) oFvXBvKKNpRXKSkuAAHxyfWolwPGUt=1398845552.228329074041678627314873967287; else oFvXBvKKNpRXKSkuAAHxyfWolwPGUt=1477411109.299292152378686845369093356547;if (oFvXBvKKNpRXKSkuAAHxyfWolwPGUt == oFvXBvKKNpRXKSkuAAHxyfWolwPGUt ) oFvXBvKKNpRXKSkuAAHxyfWolwPGUt=228229634.399796258757504720712393830794; else oFvXBvKKNpRXKSkuAAHxyfWolwPGUt=382646930.672519999170761250974683510706;if (oFvXBvKKNpRXKSkuAAHxyfWolwPGUt == oFvXBvKKNpRXKSkuAAHxyfWolwPGUt ) oFvXBvKKNpRXKSkuAAHxyfWolwPGUt=619380054.667529619683175364268193109911; else oFvXBvKKNpRXKSkuAAHxyfWolwPGUt=1280536506.039361435275149162825655957452;double iAtfcVnFUxTxxtSEZVyViGqdQoWYFU=1648811088.161848432917595429557276414531;if (iAtfcVnFUxTxxtSEZVyViGqdQoWYFU == iAtfcVnFUxTxxtSEZVyViGqdQoWYFU ) iAtfcVnFUxTxxtSEZVyViGqdQoWYFU=1209495585.000196946459076551577265100553; else iAtfcVnFUxTxxtSEZVyViGqdQoWYFU=411149802.591102740069411820196416585510;if (iAtfcVnFUxTxxtSEZVyViGqdQoWYFU == iAtfcVnFUxTxxtSEZVyViGqdQoWYFU ) iAtfcVnFUxTxxtSEZVyViGqdQoWYFU=1772598848.649306997592162895548157829673; else iAtfcVnFUxTxxtSEZVyViGqdQoWYFU=714302207.723053022503370536570117360269;if (iAtfcVnFUxTxxtSEZVyViGqdQoWYFU == iAtfcVnFUxTxxtSEZVyViGqdQoWYFU ) iAtfcVnFUxTxxtSEZVyViGqdQoWYFU=1578054498.733587911087405589739420372680; else iAtfcVnFUxTxxtSEZVyViGqdQoWYFU=1034895891.397397199362559371389253032421;if (iAtfcVnFUxTxxtSEZVyViGqdQoWYFU == iAtfcVnFUxTxxtSEZVyViGqdQoWYFU ) iAtfcVnFUxTxxtSEZVyViGqdQoWYFU=1182723807.633270112859058807111130224536; else iAtfcVnFUxTxxtSEZVyViGqdQoWYFU=946048284.587973939036064031534632160416;if (iAtfcVnFUxTxxtSEZVyViGqdQoWYFU == iAtfcVnFUxTxxtSEZVyViGqdQoWYFU ) iAtfcVnFUxTxxtSEZVyViGqdQoWYFU=782700546.779737318698829528729301205582; else iAtfcVnFUxTxxtSEZVyViGqdQoWYFU=479832964.362436843772495781475379757953;if (iAtfcVnFUxTxxtSEZVyViGqdQoWYFU == iAtfcVnFUxTxxtSEZVyViGqdQoWYFU ) iAtfcVnFUxTxxtSEZVyViGqdQoWYFU=1341461047.394633650378506801663412810379; else iAtfcVnFUxTxxtSEZVyViGqdQoWYFU=1789890880.000059987213220787495639198393;long NoFQmsbbjwLYVYyMuHHOKwpSSjEvgD=861438473;if (NoFQmsbbjwLYVYyMuHHOKwpSSjEvgD == NoFQmsbbjwLYVYyMuHHOKwpSSjEvgD- 0 ) NoFQmsbbjwLYVYyMuHHOKwpSSjEvgD=1726601837; else NoFQmsbbjwLYVYyMuHHOKwpSSjEvgD=1451387675;if (NoFQmsbbjwLYVYyMuHHOKwpSSjEvgD == NoFQmsbbjwLYVYyMuHHOKwpSSjEvgD- 1 ) NoFQmsbbjwLYVYyMuHHOKwpSSjEvgD=1262214164; else NoFQmsbbjwLYVYyMuHHOKwpSSjEvgD=1149228546;if (NoFQmsbbjwLYVYyMuHHOKwpSSjEvgD == NoFQmsbbjwLYVYyMuHHOKwpSSjEvgD- 1 ) NoFQmsbbjwLYVYyMuHHOKwpSSjEvgD=1650088729; else NoFQmsbbjwLYVYyMuHHOKwpSSjEvgD=1203575029;if (NoFQmsbbjwLYVYyMuHHOKwpSSjEvgD == NoFQmsbbjwLYVYyMuHHOKwpSSjEvgD- 1 ) NoFQmsbbjwLYVYyMuHHOKwpSSjEvgD=1817221635; else NoFQmsbbjwLYVYyMuHHOKwpSSjEvgD=1605294216;if (NoFQmsbbjwLYVYyMuHHOKwpSSjEvgD == NoFQmsbbjwLYVYyMuHHOKwpSSjEvgD- 1 ) NoFQmsbbjwLYVYyMuHHOKwpSSjEvgD=288271927; else NoFQmsbbjwLYVYyMuHHOKwpSSjEvgD=1531034426;if (NoFQmsbbjwLYVYyMuHHOKwpSSjEvgD == NoFQmsbbjwLYVYyMuHHOKwpSSjEvgD- 0 ) NoFQmsbbjwLYVYyMuHHOKwpSSjEvgD=1560833826; else NoFQmsbbjwLYVYyMuHHOKwpSSjEvgD=1342926119;int ULuZwKFYcbogwFSqmaozLiCHBhFLbS=266238602;if (ULuZwKFYcbogwFSqmaozLiCHBhFLbS == ULuZwKFYcbogwFSqmaozLiCHBhFLbS- 0 ) ULuZwKFYcbogwFSqmaozLiCHBhFLbS=10543677; else ULuZwKFYcbogwFSqmaozLiCHBhFLbS=2064247141;if (ULuZwKFYcbogwFSqmaozLiCHBhFLbS == ULuZwKFYcbogwFSqmaozLiCHBhFLbS- 0 ) ULuZwKFYcbogwFSqmaozLiCHBhFLbS=1802556847; else ULuZwKFYcbogwFSqmaozLiCHBhFLbS=1212818209;if (ULuZwKFYcbogwFSqmaozLiCHBhFLbS == ULuZwKFYcbogwFSqmaozLiCHBhFLbS- 1 ) ULuZwKFYcbogwFSqmaozLiCHBhFLbS=2133709143; else ULuZwKFYcbogwFSqmaozLiCHBhFLbS=1249976597;if (ULuZwKFYcbogwFSqmaozLiCHBhFLbS == ULuZwKFYcbogwFSqmaozLiCHBhFLbS- 0 ) ULuZwKFYcbogwFSqmaozLiCHBhFLbS=932198590; else ULuZwKFYcbogwFSqmaozLiCHBhFLbS=1922409815;if (ULuZwKFYcbogwFSqmaozLiCHBhFLbS == ULuZwKFYcbogwFSqmaozLiCHBhFLbS- 1 ) ULuZwKFYcbogwFSqmaozLiCHBhFLbS=1612425053; else ULuZwKFYcbogwFSqmaozLiCHBhFLbS=577633982;if (ULuZwKFYcbogwFSqmaozLiCHBhFLbS == ULuZwKFYcbogwFSqmaozLiCHBhFLbS- 1 ) ULuZwKFYcbogwFSqmaozLiCHBhFLbS=1200776682; else ULuZwKFYcbogwFSqmaozLiCHBhFLbS=316816682; }
 ULuZwKFYcbogwFSqmaozLiCHBhFLbSy::ULuZwKFYcbogwFSqmaozLiCHBhFLbSy()
 { this->SlCOpsNwgGNZ("TXDJyAKZXOIntVsYUknNWKFUlInYOFSlCOpsNwgGNZj", true, 1585624622, 1756438319, 1279861942); }
#pragma optimize("", off)
 // <delete/>


// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class gOlTxfkCxiovqrPtkljtuRcPsMUwpny
 { 
public: bool dfijsmVzjlbsqRKOIigxZnXFOmValK; double dfijsmVzjlbsqRKOIigxZnXFOmValKgOlTxfkCxiovqrPtkljtuRcPsMUwpn; gOlTxfkCxiovqrPtkljtuRcPsMUwpny(); void QwxmtfvIATxF(string dfijsmVzjlbsqRKOIigxZnXFOmValKQwxmtfvIATxF, bool nNKMLPIHWGOmzHSZmyiyMNJpwpHcVG, int RvOZxOhdFuFAMZinoQmpzWsnxaEVPV, float HfmQpTXZKErJsnsLBkGJBrqgHuCbcX, long KMNkmZOvjPPOFixqeEulYUOGUQpFIv);
 protected: bool dfijsmVzjlbsqRKOIigxZnXFOmValKo; double dfijsmVzjlbsqRKOIigxZnXFOmValKgOlTxfkCxiovqrPtkljtuRcPsMUwpnf; void QwxmtfvIATxFu(string dfijsmVzjlbsqRKOIigxZnXFOmValKQwxmtfvIATxFg, bool nNKMLPIHWGOmzHSZmyiyMNJpwpHcVGe, int RvOZxOhdFuFAMZinoQmpzWsnxaEVPVr, float HfmQpTXZKErJsnsLBkGJBrqgHuCbcXw, long KMNkmZOvjPPOFixqeEulYUOGUQpFIvn);
 private: bool dfijsmVzjlbsqRKOIigxZnXFOmValKnNKMLPIHWGOmzHSZmyiyMNJpwpHcVG; double dfijsmVzjlbsqRKOIigxZnXFOmValKHfmQpTXZKErJsnsLBkGJBrqgHuCbcXgOlTxfkCxiovqrPtkljtuRcPsMUwpn;
 void QwxmtfvIATxFv(string nNKMLPIHWGOmzHSZmyiyMNJpwpHcVGQwxmtfvIATxF, bool nNKMLPIHWGOmzHSZmyiyMNJpwpHcVGRvOZxOhdFuFAMZinoQmpzWsnxaEVPV, int RvOZxOhdFuFAMZinoQmpzWsnxaEVPVdfijsmVzjlbsqRKOIigxZnXFOmValK, float HfmQpTXZKErJsnsLBkGJBrqgHuCbcXKMNkmZOvjPPOFixqeEulYUOGUQpFIv, long KMNkmZOvjPPOFixqeEulYUOGUQpFIvnNKMLPIHWGOmzHSZmyiyMNJpwpHcVG); };
 void gOlTxfkCxiovqrPtkljtuRcPsMUwpny::QwxmtfvIATxF(string dfijsmVzjlbsqRKOIigxZnXFOmValKQwxmtfvIATxF, bool nNKMLPIHWGOmzHSZmyiyMNJpwpHcVG, int RvOZxOhdFuFAMZinoQmpzWsnxaEVPV, float HfmQpTXZKErJsnsLBkGJBrqgHuCbcX, long KMNkmZOvjPPOFixqeEulYUOGUQpFIv)
 { double hbtIPfPBOCOqKoHIkaREXVSbQBxKUu=317037156.486688696035529676108422641898;if (hbtIPfPBOCOqKoHIkaREXVSbQBxKUu == hbtIPfPBOCOqKoHIkaREXVSbQBxKUu ) hbtIPfPBOCOqKoHIkaREXVSbQBxKUu=1200661792.583618094879885308508853407504; else hbtIPfPBOCOqKoHIkaREXVSbQBxKUu=668653715.790865292279334974734330174131;if (hbtIPfPBOCOqKoHIkaREXVSbQBxKUu == hbtIPfPBOCOqKoHIkaREXVSbQBxKUu ) hbtIPfPBOCOqKoHIkaREXVSbQBxKUu=2143411952.269028013112643062054487056189; else hbtIPfPBOCOqKoHIkaREXVSbQBxKUu=1723131144.126202032607163370712219798317;if (hbtIPfPBOCOqKoHIkaREXVSbQBxKUu == hbtIPfPBOCOqKoHIkaREXVSbQBxKUu ) hbtIPfPBOCOqKoHIkaREXVSbQBxKUu=434250916.303509754213055833588747701752; else hbtIPfPBOCOqKoHIkaREXVSbQBxKUu=215609848.573531477462661027506683076781;if (hbtIPfPBOCOqKoHIkaREXVSbQBxKUu == hbtIPfPBOCOqKoHIkaREXVSbQBxKUu ) hbtIPfPBOCOqKoHIkaREXVSbQBxKUu=2029089528.901384052456622628788535690167; else hbtIPfPBOCOqKoHIkaREXVSbQBxKUu=78577741.580766239708516007146900448689;if (hbtIPfPBOCOqKoHIkaREXVSbQBxKUu == hbtIPfPBOCOqKoHIkaREXVSbQBxKUu ) hbtIPfPBOCOqKoHIkaREXVSbQBxKUu=988136628.500607707649019103060302278042; else hbtIPfPBOCOqKoHIkaREXVSbQBxKUu=1937813157.976031801941161700235562021954;if (hbtIPfPBOCOqKoHIkaREXVSbQBxKUu == hbtIPfPBOCOqKoHIkaREXVSbQBxKUu ) hbtIPfPBOCOqKoHIkaREXVSbQBxKUu=238949414.077588409830223035258991742411; else hbtIPfPBOCOqKoHIkaREXVSbQBxKUu=1362630625.106807468440241545000780719784;double vtpnRXUMddigoEnlaANElWGCVKeIqr=1147806664.444961650234143311249066096081;if (vtpnRXUMddigoEnlaANElWGCVKeIqr == vtpnRXUMddigoEnlaANElWGCVKeIqr ) vtpnRXUMddigoEnlaANElWGCVKeIqr=127517595.507629482666329977806847459908; else vtpnRXUMddigoEnlaANElWGCVKeIqr=1334616733.677073753594978474712929586127;if (vtpnRXUMddigoEnlaANElWGCVKeIqr == vtpnRXUMddigoEnlaANElWGCVKeIqr ) vtpnRXUMddigoEnlaANElWGCVKeIqr=2106358688.068500946632441397918520676969; else vtpnRXUMddigoEnlaANElWGCVKeIqr=1504765599.774654657954929059830922954904;if (vtpnRXUMddigoEnlaANElWGCVKeIqr == vtpnRXUMddigoEnlaANElWGCVKeIqr ) vtpnRXUMddigoEnlaANElWGCVKeIqr=732060648.409796999874106571262813995370; else vtpnRXUMddigoEnlaANElWGCVKeIqr=490956641.086455623327016692659484270691;if (vtpnRXUMddigoEnlaANElWGCVKeIqr == vtpnRXUMddigoEnlaANElWGCVKeIqr ) vtpnRXUMddigoEnlaANElWGCVKeIqr=515020311.172980976106995680830373270239; else vtpnRXUMddigoEnlaANElWGCVKeIqr=882478488.604225623783766369969096191072;if (vtpnRXUMddigoEnlaANElWGCVKeIqr == vtpnRXUMddigoEnlaANElWGCVKeIqr ) vtpnRXUMddigoEnlaANElWGCVKeIqr=936927226.075760377017726872701137072814; else vtpnRXUMddigoEnlaANElWGCVKeIqr=832693693.565271901709306337042938294889;if (vtpnRXUMddigoEnlaANElWGCVKeIqr == vtpnRXUMddigoEnlaANElWGCVKeIqr ) vtpnRXUMddigoEnlaANElWGCVKeIqr=1201400186.856365265507210294274598364818; else vtpnRXUMddigoEnlaANElWGCVKeIqr=2045372619.713205404117355583801171637810;long UejqtXAHZstzCgkORQkpDCFohWIeiq=831638317;if (UejqtXAHZstzCgkORQkpDCFohWIeiq == UejqtXAHZstzCgkORQkpDCFohWIeiq- 0 ) UejqtXAHZstzCgkORQkpDCFohWIeiq=791266853; else UejqtXAHZstzCgkORQkpDCFohWIeiq=1554126751;if (UejqtXAHZstzCgkORQkpDCFohWIeiq == UejqtXAHZstzCgkORQkpDCFohWIeiq- 0 ) UejqtXAHZstzCgkORQkpDCFohWIeiq=1027855605; else UejqtXAHZstzCgkORQkpDCFohWIeiq=1480943866;if (UejqtXAHZstzCgkORQkpDCFohWIeiq == UejqtXAHZstzCgkORQkpDCFohWIeiq- 0 ) UejqtXAHZstzCgkORQkpDCFohWIeiq=2058107923; else UejqtXAHZstzCgkORQkpDCFohWIeiq=662714110;if (UejqtXAHZstzCgkORQkpDCFohWIeiq == UejqtXAHZstzCgkORQkpDCFohWIeiq- 0 ) UejqtXAHZstzCgkORQkpDCFohWIeiq=1451374844; else UejqtXAHZstzCgkORQkpDCFohWIeiq=929832801;if (UejqtXAHZstzCgkORQkpDCFohWIeiq == UejqtXAHZstzCgkORQkpDCFohWIeiq- 1 ) UejqtXAHZstzCgkORQkpDCFohWIeiq=1279443598; else UejqtXAHZstzCgkORQkpDCFohWIeiq=842990824;if (UejqtXAHZstzCgkORQkpDCFohWIeiq == UejqtXAHZstzCgkORQkpDCFohWIeiq- 1 ) UejqtXAHZstzCgkORQkpDCFohWIeiq=1467376108; else UejqtXAHZstzCgkORQkpDCFohWIeiq=1150062840;int zfZPtgfzhLFXGITJsCZNWREbvAfsAa=2097889919;if (zfZPtgfzhLFXGITJsCZNWREbvAfsAa == zfZPtgfzhLFXGITJsCZNWREbvAfsAa- 1 ) zfZPtgfzhLFXGITJsCZNWREbvAfsAa=979913531; else zfZPtgfzhLFXGITJsCZNWREbvAfsAa=1838445162;if (zfZPtgfzhLFXGITJsCZNWREbvAfsAa == zfZPtgfzhLFXGITJsCZNWREbvAfsAa- 1 ) zfZPtgfzhLFXGITJsCZNWREbvAfsAa=1434122087; else zfZPtgfzhLFXGITJsCZNWREbvAfsAa=916252106;if (zfZPtgfzhLFXGITJsCZNWREbvAfsAa == zfZPtgfzhLFXGITJsCZNWREbvAfsAa- 0 ) zfZPtgfzhLFXGITJsCZNWREbvAfsAa=788965601; else zfZPtgfzhLFXGITJsCZNWREbvAfsAa=2079828349;if (zfZPtgfzhLFXGITJsCZNWREbvAfsAa == zfZPtgfzhLFXGITJsCZNWREbvAfsAa- 1 ) zfZPtgfzhLFXGITJsCZNWREbvAfsAa=1732460155; else zfZPtgfzhLFXGITJsCZNWREbvAfsAa=709705151;if (zfZPtgfzhLFXGITJsCZNWREbvAfsAa == zfZPtgfzhLFXGITJsCZNWREbvAfsAa- 1 ) zfZPtgfzhLFXGITJsCZNWREbvAfsAa=153633404; else zfZPtgfzhLFXGITJsCZNWREbvAfsAa=1260933464;if (zfZPtgfzhLFXGITJsCZNWREbvAfsAa == zfZPtgfzhLFXGITJsCZNWREbvAfsAa- 1 ) zfZPtgfzhLFXGITJsCZNWREbvAfsAa=786203918; else zfZPtgfzhLFXGITJsCZNWREbvAfsAa=1749040870;int fypRFScQcPYHCxLjLCwrEYgqAWoREn=93397497;if (fypRFScQcPYHCxLjLCwrEYgqAWoREn == fypRFScQcPYHCxLjLCwrEYgqAWoREn- 0 ) fypRFScQcPYHCxLjLCwrEYgqAWoREn=593479339; else fypRFScQcPYHCxLjLCwrEYgqAWoREn=1554794744;if (fypRFScQcPYHCxLjLCwrEYgqAWoREn == fypRFScQcPYHCxLjLCwrEYgqAWoREn- 0 ) fypRFScQcPYHCxLjLCwrEYgqAWoREn=571363772; else fypRFScQcPYHCxLjLCwrEYgqAWoREn=2031690999;if (fypRFScQcPYHCxLjLCwrEYgqAWoREn == fypRFScQcPYHCxLjLCwrEYgqAWoREn- 0 ) fypRFScQcPYHCxLjLCwrEYgqAWoREn=2119769575; else fypRFScQcPYHCxLjLCwrEYgqAWoREn=1213839916;if (fypRFScQcPYHCxLjLCwrEYgqAWoREn == fypRFScQcPYHCxLjLCwrEYgqAWoREn- 0 ) fypRFScQcPYHCxLjLCwrEYgqAWoREn=1237182388; else fypRFScQcPYHCxLjLCwrEYgqAWoREn=1612287132;if (fypRFScQcPYHCxLjLCwrEYgqAWoREn == fypRFScQcPYHCxLjLCwrEYgqAWoREn- 0 ) fypRFScQcPYHCxLjLCwrEYgqAWoREn=2030725536; else fypRFScQcPYHCxLjLCwrEYgqAWoREn=1176525887;if (fypRFScQcPYHCxLjLCwrEYgqAWoREn == fypRFScQcPYHCxLjLCwrEYgqAWoREn- 0 ) fypRFScQcPYHCxLjLCwrEYgqAWoREn=225322001; else fypRFScQcPYHCxLjLCwrEYgqAWoREn=2036553471;double uCmnPjKAHjGpDfqYwlAygwIpmWdACE=350721924.404670317950229594928332550269;if (uCmnPjKAHjGpDfqYwlAygwIpmWdACE == uCmnPjKAHjGpDfqYwlAygwIpmWdACE ) uCmnPjKAHjGpDfqYwlAygwIpmWdACE=310652594.332060732651733438911142918323; else uCmnPjKAHjGpDfqYwlAygwIpmWdACE=1250993914.015872827996337770912345939125;if (uCmnPjKAHjGpDfqYwlAygwIpmWdACE == uCmnPjKAHjGpDfqYwlAygwIpmWdACE ) uCmnPjKAHjGpDfqYwlAygwIpmWdACE=1065459088.341964697161113581524842188105; else uCmnPjKAHjGpDfqYwlAygwIpmWdACE=267977999.371265658429485223345870260088;if (uCmnPjKAHjGpDfqYwlAygwIpmWdACE == uCmnPjKAHjGpDfqYwlAygwIpmWdACE ) uCmnPjKAHjGpDfqYwlAygwIpmWdACE=1608100691.669390006804420546707597051210; else uCmnPjKAHjGpDfqYwlAygwIpmWdACE=428081712.567087110633854255072449218244;if (uCmnPjKAHjGpDfqYwlAygwIpmWdACE == uCmnPjKAHjGpDfqYwlAygwIpmWdACE ) uCmnPjKAHjGpDfqYwlAygwIpmWdACE=42672716.990645855454718584391625813887; else uCmnPjKAHjGpDfqYwlAygwIpmWdACE=858922151.098784332050172983284406337683;if (uCmnPjKAHjGpDfqYwlAygwIpmWdACE == uCmnPjKAHjGpDfqYwlAygwIpmWdACE ) uCmnPjKAHjGpDfqYwlAygwIpmWdACE=1969150243.308956604426334266393070370355; else uCmnPjKAHjGpDfqYwlAygwIpmWdACE=318150454.885985332749097917218240034869;if (uCmnPjKAHjGpDfqYwlAygwIpmWdACE == uCmnPjKAHjGpDfqYwlAygwIpmWdACE ) uCmnPjKAHjGpDfqYwlAygwIpmWdACE=1327310462.645974146003466508456424865605; else uCmnPjKAHjGpDfqYwlAygwIpmWdACE=797174459.258411423387222588454642332221;float cXDpadCbaFqaGiuMwNMrmIYEcDfCqr=1295726689.464259922436089633562338151064f;if (cXDpadCbaFqaGiuMwNMrmIYEcDfCqr - cXDpadCbaFqaGiuMwNMrmIYEcDfCqr> 0.00000001 ) cXDpadCbaFqaGiuMwNMrmIYEcDfCqr=723352597.878758149510478738320134723711f; else cXDpadCbaFqaGiuMwNMrmIYEcDfCqr=1373978611.807966943269405297153474585504f;if (cXDpadCbaFqaGiuMwNMrmIYEcDfCqr - cXDpadCbaFqaGiuMwNMrmIYEcDfCqr> 0.00000001 ) cXDpadCbaFqaGiuMwNMrmIYEcDfCqr=556583501.544972461389627273963767853359f; else cXDpadCbaFqaGiuMwNMrmIYEcDfCqr=1687739426.144072663484697486731375189713f;if (cXDpadCbaFqaGiuMwNMrmIYEcDfCqr - cXDpadCbaFqaGiuMwNMrmIYEcDfCqr> 0.00000001 ) cXDpadCbaFqaGiuMwNMrmIYEcDfCqr=14841530.912325575672158928157872898090f; else cXDpadCbaFqaGiuMwNMrmIYEcDfCqr=637833747.395068347392127023727276625064f;if (cXDpadCbaFqaGiuMwNMrmIYEcDfCqr - cXDpadCbaFqaGiuMwNMrmIYEcDfCqr> 0.00000001 ) cXDpadCbaFqaGiuMwNMrmIYEcDfCqr=2125603991.768638719178197576587538323954f; else cXDpadCbaFqaGiuMwNMrmIYEcDfCqr=1913557262.439464569032476918016068733132f;if (cXDpadCbaFqaGiuMwNMrmIYEcDfCqr - cXDpadCbaFqaGiuMwNMrmIYEcDfCqr> 0.00000001 ) cXDpadCbaFqaGiuMwNMrmIYEcDfCqr=601262774.597674084411756626251504948174f; else cXDpadCbaFqaGiuMwNMrmIYEcDfCqr=1969318602.153855690394571531321899119374f;if (cXDpadCbaFqaGiuMwNMrmIYEcDfCqr - cXDpadCbaFqaGiuMwNMrmIYEcDfCqr> 0.00000001 ) cXDpadCbaFqaGiuMwNMrmIYEcDfCqr=1033010217.601136891114606813342896347575f; else cXDpadCbaFqaGiuMwNMrmIYEcDfCqr=1759923361.632770454584121829441704291580f;double SKfYoIuSfTdwrilkiYbKrFRWxMrZot=788675933.956734935928275190532084988843;if (SKfYoIuSfTdwrilkiYbKrFRWxMrZot == SKfYoIuSfTdwrilkiYbKrFRWxMrZot ) SKfYoIuSfTdwrilkiYbKrFRWxMrZot=1473505499.177949260123510979316115407295; else SKfYoIuSfTdwrilkiYbKrFRWxMrZot=435481994.719285244704141058502963807265;if (SKfYoIuSfTdwrilkiYbKrFRWxMrZot == SKfYoIuSfTdwrilkiYbKrFRWxMrZot ) SKfYoIuSfTdwrilkiYbKrFRWxMrZot=1438388276.207651533584662317545080486658; else SKfYoIuSfTdwrilkiYbKrFRWxMrZot=2058183042.919950760062603840260082033838;if (SKfYoIuSfTdwrilkiYbKrFRWxMrZot == SKfYoIuSfTdwrilkiYbKrFRWxMrZot ) SKfYoIuSfTdwrilkiYbKrFRWxMrZot=1065741468.699578671266859448765640993702; else SKfYoIuSfTdwrilkiYbKrFRWxMrZot=1999143875.894357314527000534369617315068;if (SKfYoIuSfTdwrilkiYbKrFRWxMrZot == SKfYoIuSfTdwrilkiYbKrFRWxMrZot ) SKfYoIuSfTdwrilkiYbKrFRWxMrZot=632780453.598958351314534875057054428704; else SKfYoIuSfTdwrilkiYbKrFRWxMrZot=165397627.245604015328306629355102255463;if (SKfYoIuSfTdwrilkiYbKrFRWxMrZot == SKfYoIuSfTdwrilkiYbKrFRWxMrZot ) SKfYoIuSfTdwrilkiYbKrFRWxMrZot=1512122028.065727993197083975981045024445; else SKfYoIuSfTdwrilkiYbKrFRWxMrZot=1859925268.065237379983636913049068312080;if (SKfYoIuSfTdwrilkiYbKrFRWxMrZot == SKfYoIuSfTdwrilkiYbKrFRWxMrZot ) SKfYoIuSfTdwrilkiYbKrFRWxMrZot=62540756.466814285661701942021240711703; else SKfYoIuSfTdwrilkiYbKrFRWxMrZot=1801619121.540963792374949836847117022611;float bpHILHmWsMbxiXmQCGvlZuhhSLOwTD=180907915.716596185245344466300407705376f;if (bpHILHmWsMbxiXmQCGvlZuhhSLOwTD - bpHILHmWsMbxiXmQCGvlZuhhSLOwTD> 0.00000001 ) bpHILHmWsMbxiXmQCGvlZuhhSLOwTD=1408386395.813869518404061265207483842999f; else bpHILHmWsMbxiXmQCGvlZuhhSLOwTD=1077078959.192954368505891645655117953003f;if (bpHILHmWsMbxiXmQCGvlZuhhSLOwTD - bpHILHmWsMbxiXmQCGvlZuhhSLOwTD> 0.00000001 ) bpHILHmWsMbxiXmQCGvlZuhhSLOwTD=1313200874.863031311800781360230844519501f; else bpHILHmWsMbxiXmQCGvlZuhhSLOwTD=1944569216.136674296133496554669488313063f;if (bpHILHmWsMbxiXmQCGvlZuhhSLOwTD - bpHILHmWsMbxiXmQCGvlZuhhSLOwTD> 0.00000001 ) bpHILHmWsMbxiXmQCGvlZuhhSLOwTD=955857617.382177511828520745907604286847f; else bpHILHmWsMbxiXmQCGvlZuhhSLOwTD=247172186.234495232915395297967816679890f;if (bpHILHmWsMbxiXmQCGvlZuhhSLOwTD - bpHILHmWsMbxiXmQCGvlZuhhSLOwTD> 0.00000001 ) bpHILHmWsMbxiXmQCGvlZuhhSLOwTD=335880394.277460414652310331019008042892f; else bpHILHmWsMbxiXmQCGvlZuhhSLOwTD=1820302494.654384893689341651635989127943f;if (bpHILHmWsMbxiXmQCGvlZuhhSLOwTD - bpHILHmWsMbxiXmQCGvlZuhhSLOwTD> 0.00000001 ) bpHILHmWsMbxiXmQCGvlZuhhSLOwTD=1272873570.873201813664853177759226787626f; else bpHILHmWsMbxiXmQCGvlZuhhSLOwTD=1299385473.895182624195808114716120475853f;if (bpHILHmWsMbxiXmQCGvlZuhhSLOwTD - bpHILHmWsMbxiXmQCGvlZuhhSLOwTD> 0.00000001 ) bpHILHmWsMbxiXmQCGvlZuhhSLOwTD=1814198872.420897987155377543877302078672f; else bpHILHmWsMbxiXmQCGvlZuhhSLOwTD=1786265736.897277612234866186393309613882f;long QCaRiqGyBwRFiUTdTbwyIOpucsrgeS=125759912;if (QCaRiqGyBwRFiUTdTbwyIOpucsrgeS == QCaRiqGyBwRFiUTdTbwyIOpucsrgeS- 1 ) QCaRiqGyBwRFiUTdTbwyIOpucsrgeS=779509483; else QCaRiqGyBwRFiUTdTbwyIOpucsrgeS=538634529;if (QCaRiqGyBwRFiUTdTbwyIOpucsrgeS == QCaRiqGyBwRFiUTdTbwyIOpucsrgeS- 1 ) QCaRiqGyBwRFiUTdTbwyIOpucsrgeS=1471152607; else QCaRiqGyBwRFiUTdTbwyIOpucsrgeS=1588511845;if (QCaRiqGyBwRFiUTdTbwyIOpucsrgeS == QCaRiqGyBwRFiUTdTbwyIOpucsrgeS- 1 ) QCaRiqGyBwRFiUTdTbwyIOpucsrgeS=411429345; else QCaRiqGyBwRFiUTdTbwyIOpucsrgeS=2004923744;if (QCaRiqGyBwRFiUTdTbwyIOpucsrgeS == QCaRiqGyBwRFiUTdTbwyIOpucsrgeS- 1 ) QCaRiqGyBwRFiUTdTbwyIOpucsrgeS=229985221; else QCaRiqGyBwRFiUTdTbwyIOpucsrgeS=871692369;if (QCaRiqGyBwRFiUTdTbwyIOpucsrgeS == QCaRiqGyBwRFiUTdTbwyIOpucsrgeS- 0 ) QCaRiqGyBwRFiUTdTbwyIOpucsrgeS=741198158; else QCaRiqGyBwRFiUTdTbwyIOpucsrgeS=391185874;if (QCaRiqGyBwRFiUTdTbwyIOpucsrgeS == QCaRiqGyBwRFiUTdTbwyIOpucsrgeS- 0 ) QCaRiqGyBwRFiUTdTbwyIOpucsrgeS=175617398; else QCaRiqGyBwRFiUTdTbwyIOpucsrgeS=302399909;int KYMWZWUxJDspaRdPWqRIVShpQBFwfF=161254845;if (KYMWZWUxJDspaRdPWqRIVShpQBFwfF == KYMWZWUxJDspaRdPWqRIVShpQBFwfF- 1 ) KYMWZWUxJDspaRdPWqRIVShpQBFwfF=233541662; else KYMWZWUxJDspaRdPWqRIVShpQBFwfF=852102302;if (KYMWZWUxJDspaRdPWqRIVShpQBFwfF == KYMWZWUxJDspaRdPWqRIVShpQBFwfF- 1 ) KYMWZWUxJDspaRdPWqRIVShpQBFwfF=351536966; else KYMWZWUxJDspaRdPWqRIVShpQBFwfF=777427389;if (KYMWZWUxJDspaRdPWqRIVShpQBFwfF == KYMWZWUxJDspaRdPWqRIVShpQBFwfF- 0 ) KYMWZWUxJDspaRdPWqRIVShpQBFwfF=530189457; else KYMWZWUxJDspaRdPWqRIVShpQBFwfF=1889039049;if (KYMWZWUxJDspaRdPWqRIVShpQBFwfF == KYMWZWUxJDspaRdPWqRIVShpQBFwfF- 0 ) KYMWZWUxJDspaRdPWqRIVShpQBFwfF=1980301963; else KYMWZWUxJDspaRdPWqRIVShpQBFwfF=1226333313;if (KYMWZWUxJDspaRdPWqRIVShpQBFwfF == KYMWZWUxJDspaRdPWqRIVShpQBFwfF- 1 ) KYMWZWUxJDspaRdPWqRIVShpQBFwfF=99601600; else KYMWZWUxJDspaRdPWqRIVShpQBFwfF=1765569429;if (KYMWZWUxJDspaRdPWqRIVShpQBFwfF == KYMWZWUxJDspaRdPWqRIVShpQBFwfF- 1 ) KYMWZWUxJDspaRdPWqRIVShpQBFwfF=785309472; else KYMWZWUxJDspaRdPWqRIVShpQBFwfF=1913839642;float PTveSDksMtMJVwBvyWGRQnElPmyIAd=1081889026.185022048445284005869031088651f;if (PTveSDksMtMJVwBvyWGRQnElPmyIAd - PTveSDksMtMJVwBvyWGRQnElPmyIAd> 0.00000001 ) PTveSDksMtMJVwBvyWGRQnElPmyIAd=708025818.867421761992630685438898326415f; else PTveSDksMtMJVwBvyWGRQnElPmyIAd=2084264491.449768987444498841240332417021f;if (PTveSDksMtMJVwBvyWGRQnElPmyIAd - PTveSDksMtMJVwBvyWGRQnElPmyIAd> 0.00000001 ) PTveSDksMtMJVwBvyWGRQnElPmyIAd=1022109638.646765397271910451158627521846f; else PTveSDksMtMJVwBvyWGRQnElPmyIAd=1495378572.196680973355417043316708412935f;if (PTveSDksMtMJVwBvyWGRQnElPmyIAd - PTveSDksMtMJVwBvyWGRQnElPmyIAd> 0.00000001 ) PTveSDksMtMJVwBvyWGRQnElPmyIAd=1116338969.566118643058880421068954150938f; else PTveSDksMtMJVwBvyWGRQnElPmyIAd=147265095.904980041041136992810108761216f;if (PTveSDksMtMJVwBvyWGRQnElPmyIAd - PTveSDksMtMJVwBvyWGRQnElPmyIAd> 0.00000001 ) PTveSDksMtMJVwBvyWGRQnElPmyIAd=1916962217.982526400148112892927923981733f; else PTveSDksMtMJVwBvyWGRQnElPmyIAd=1550946298.984617962150277210608049007075f;if (PTveSDksMtMJVwBvyWGRQnElPmyIAd - PTveSDksMtMJVwBvyWGRQnElPmyIAd> 0.00000001 ) PTveSDksMtMJVwBvyWGRQnElPmyIAd=847093738.588837560693198875678113505662f; else PTveSDksMtMJVwBvyWGRQnElPmyIAd=441508505.100867244185112595123240857544f;if (PTveSDksMtMJVwBvyWGRQnElPmyIAd - PTveSDksMtMJVwBvyWGRQnElPmyIAd> 0.00000001 ) PTveSDksMtMJVwBvyWGRQnElPmyIAd=1203371058.200959444164160311201055997111f; else PTveSDksMtMJVwBvyWGRQnElPmyIAd=564671743.180199626326207018054645425606f;float ROLBpSAiEitZrpWoZmursAbmYTemkT=948888700.580231115946256723892238740287f;if (ROLBpSAiEitZrpWoZmursAbmYTemkT - ROLBpSAiEitZrpWoZmursAbmYTemkT> 0.00000001 ) ROLBpSAiEitZrpWoZmursAbmYTemkT=1419841312.913366209031823167620375945269f; else ROLBpSAiEitZrpWoZmursAbmYTemkT=1652944027.193804390052597152669477874852f;if (ROLBpSAiEitZrpWoZmursAbmYTemkT - ROLBpSAiEitZrpWoZmursAbmYTemkT> 0.00000001 ) ROLBpSAiEitZrpWoZmursAbmYTemkT=1552724074.114094802987767999990471173713f; else ROLBpSAiEitZrpWoZmursAbmYTemkT=690452840.165895850335078281009506385402f;if (ROLBpSAiEitZrpWoZmursAbmYTemkT - ROLBpSAiEitZrpWoZmursAbmYTemkT> 0.00000001 ) ROLBpSAiEitZrpWoZmursAbmYTemkT=78696036.220093398214076550490372566962f; else ROLBpSAiEitZrpWoZmursAbmYTemkT=374472061.156812519509399664184502305953f;if (ROLBpSAiEitZrpWoZmursAbmYTemkT - ROLBpSAiEitZrpWoZmursAbmYTemkT> 0.00000001 ) ROLBpSAiEitZrpWoZmursAbmYTemkT=1743054102.854616836414109109301288976551f; else ROLBpSAiEitZrpWoZmursAbmYTemkT=1037954081.876775689484878041321776267685f;if (ROLBpSAiEitZrpWoZmursAbmYTemkT - ROLBpSAiEitZrpWoZmursAbmYTemkT> 0.00000001 ) ROLBpSAiEitZrpWoZmursAbmYTemkT=705816213.038674953563092842193534110052f; else ROLBpSAiEitZrpWoZmursAbmYTemkT=244819294.900446631419997978542660251556f;if (ROLBpSAiEitZrpWoZmursAbmYTemkT - ROLBpSAiEitZrpWoZmursAbmYTemkT> 0.00000001 ) ROLBpSAiEitZrpWoZmursAbmYTemkT=1488910245.591166375722178475735261035130f; else ROLBpSAiEitZrpWoZmursAbmYTemkT=793343563.529503983161483159289783133024f;double NUbIzPfmcZvJgjWsotvpbvWFrYeTmy=1894683441.582660356756411044226200114318;if (NUbIzPfmcZvJgjWsotvpbvWFrYeTmy == NUbIzPfmcZvJgjWsotvpbvWFrYeTmy ) NUbIzPfmcZvJgjWsotvpbvWFrYeTmy=1768343389.987822195838000497455621539265; else NUbIzPfmcZvJgjWsotvpbvWFrYeTmy=1241212019.614887979095959930182797337630;if (NUbIzPfmcZvJgjWsotvpbvWFrYeTmy == NUbIzPfmcZvJgjWsotvpbvWFrYeTmy ) NUbIzPfmcZvJgjWsotvpbvWFrYeTmy=1741857738.404122422453856188270928393856; else NUbIzPfmcZvJgjWsotvpbvWFrYeTmy=638512147.144060293852973004135047058683;if (NUbIzPfmcZvJgjWsotvpbvWFrYeTmy == NUbIzPfmcZvJgjWsotvpbvWFrYeTmy ) NUbIzPfmcZvJgjWsotvpbvWFrYeTmy=1449358879.077741089790552905540983110702; else NUbIzPfmcZvJgjWsotvpbvWFrYeTmy=531649236.261199534312555660525415419573;if (NUbIzPfmcZvJgjWsotvpbvWFrYeTmy == NUbIzPfmcZvJgjWsotvpbvWFrYeTmy ) NUbIzPfmcZvJgjWsotvpbvWFrYeTmy=1192399523.239853957448133844856128795778; else NUbIzPfmcZvJgjWsotvpbvWFrYeTmy=86276567.016228480272329839267672812127;if (NUbIzPfmcZvJgjWsotvpbvWFrYeTmy == NUbIzPfmcZvJgjWsotvpbvWFrYeTmy ) NUbIzPfmcZvJgjWsotvpbvWFrYeTmy=1082623732.838944202557579480096761244551; else NUbIzPfmcZvJgjWsotvpbvWFrYeTmy=948074315.737345458664771121343223227780;if (NUbIzPfmcZvJgjWsotvpbvWFrYeTmy == NUbIzPfmcZvJgjWsotvpbvWFrYeTmy ) NUbIzPfmcZvJgjWsotvpbvWFrYeTmy=2077817298.085293123504344219113321962413; else NUbIzPfmcZvJgjWsotvpbvWFrYeTmy=88680952.887969969910894008911977876543;int jGAmOjVpbMYBHDNaRjuiTYAIuCzDuN=66121115;if (jGAmOjVpbMYBHDNaRjuiTYAIuCzDuN == jGAmOjVpbMYBHDNaRjuiTYAIuCzDuN- 1 ) jGAmOjVpbMYBHDNaRjuiTYAIuCzDuN=520559926; else jGAmOjVpbMYBHDNaRjuiTYAIuCzDuN=1983904419;if (jGAmOjVpbMYBHDNaRjuiTYAIuCzDuN == jGAmOjVpbMYBHDNaRjuiTYAIuCzDuN- 0 ) jGAmOjVpbMYBHDNaRjuiTYAIuCzDuN=493998330; else jGAmOjVpbMYBHDNaRjuiTYAIuCzDuN=679484623;if (jGAmOjVpbMYBHDNaRjuiTYAIuCzDuN == jGAmOjVpbMYBHDNaRjuiTYAIuCzDuN- 0 ) jGAmOjVpbMYBHDNaRjuiTYAIuCzDuN=1588757937; else jGAmOjVpbMYBHDNaRjuiTYAIuCzDuN=79873292;if (jGAmOjVpbMYBHDNaRjuiTYAIuCzDuN == jGAmOjVpbMYBHDNaRjuiTYAIuCzDuN- 1 ) jGAmOjVpbMYBHDNaRjuiTYAIuCzDuN=1003192990; else jGAmOjVpbMYBHDNaRjuiTYAIuCzDuN=333553757;if (jGAmOjVpbMYBHDNaRjuiTYAIuCzDuN == jGAmOjVpbMYBHDNaRjuiTYAIuCzDuN- 0 ) jGAmOjVpbMYBHDNaRjuiTYAIuCzDuN=341210389; else jGAmOjVpbMYBHDNaRjuiTYAIuCzDuN=2131639204;if (jGAmOjVpbMYBHDNaRjuiTYAIuCzDuN == jGAmOjVpbMYBHDNaRjuiTYAIuCzDuN- 1 ) jGAmOjVpbMYBHDNaRjuiTYAIuCzDuN=789562359; else jGAmOjVpbMYBHDNaRjuiTYAIuCzDuN=871519675;double KeWmTGzgTjTSnfbcmjlYoTrVviHMJS=383464512.260701553399552210151414205300;if (KeWmTGzgTjTSnfbcmjlYoTrVviHMJS == KeWmTGzgTjTSnfbcmjlYoTrVviHMJS ) KeWmTGzgTjTSnfbcmjlYoTrVviHMJS=1902244786.755692744145052782680204196750; else KeWmTGzgTjTSnfbcmjlYoTrVviHMJS=1456171264.635375597057712746362473166160;if (KeWmTGzgTjTSnfbcmjlYoTrVviHMJS == KeWmTGzgTjTSnfbcmjlYoTrVviHMJS ) KeWmTGzgTjTSnfbcmjlYoTrVviHMJS=943812001.719451089830673360355040926332; else KeWmTGzgTjTSnfbcmjlYoTrVviHMJS=977826416.944735836177863391181791587054;if (KeWmTGzgTjTSnfbcmjlYoTrVviHMJS == KeWmTGzgTjTSnfbcmjlYoTrVviHMJS ) KeWmTGzgTjTSnfbcmjlYoTrVviHMJS=439106394.676799794853438929774525236325; else KeWmTGzgTjTSnfbcmjlYoTrVviHMJS=606497356.898361217561833836476939115166;if (KeWmTGzgTjTSnfbcmjlYoTrVviHMJS == KeWmTGzgTjTSnfbcmjlYoTrVviHMJS ) KeWmTGzgTjTSnfbcmjlYoTrVviHMJS=1647013468.347907527882231480241006006026; else KeWmTGzgTjTSnfbcmjlYoTrVviHMJS=888192076.615223493741069571983603545201;if (KeWmTGzgTjTSnfbcmjlYoTrVviHMJS == KeWmTGzgTjTSnfbcmjlYoTrVviHMJS ) KeWmTGzgTjTSnfbcmjlYoTrVviHMJS=460544504.160238702900750014764615302378; else KeWmTGzgTjTSnfbcmjlYoTrVviHMJS=1466447507.820378183287908405353165855604;if (KeWmTGzgTjTSnfbcmjlYoTrVviHMJS == KeWmTGzgTjTSnfbcmjlYoTrVviHMJS ) KeWmTGzgTjTSnfbcmjlYoTrVviHMJS=1755312755.644419049204258771890702687563; else KeWmTGzgTjTSnfbcmjlYoTrVviHMJS=1278105368.469064460946716001718491405294;float ZretUaIsRRGIsZUCwgcnaqaZvklejz=1437669640.366663371866541507629577537535f;if (ZretUaIsRRGIsZUCwgcnaqaZvklejz - ZretUaIsRRGIsZUCwgcnaqaZvklejz> 0.00000001 ) ZretUaIsRRGIsZUCwgcnaqaZvklejz=1237524416.240726209637399913373505380923f; else ZretUaIsRRGIsZUCwgcnaqaZvklejz=1422789130.785663692631819228962437375544f;if (ZretUaIsRRGIsZUCwgcnaqaZvklejz - ZretUaIsRRGIsZUCwgcnaqaZvklejz> 0.00000001 ) ZretUaIsRRGIsZUCwgcnaqaZvklejz=272783637.639696384732174370617316146940f; else ZretUaIsRRGIsZUCwgcnaqaZvklejz=1383193500.816810232405524047310593843356f;if (ZretUaIsRRGIsZUCwgcnaqaZvklejz - ZretUaIsRRGIsZUCwgcnaqaZvklejz> 0.00000001 ) ZretUaIsRRGIsZUCwgcnaqaZvklejz=2117114543.054492737515832289321063043417f; else ZretUaIsRRGIsZUCwgcnaqaZvklejz=7262743.808646114241091587166414619256f;if (ZretUaIsRRGIsZUCwgcnaqaZvklejz - ZretUaIsRRGIsZUCwgcnaqaZvklejz> 0.00000001 ) ZretUaIsRRGIsZUCwgcnaqaZvklejz=305925504.033071829628618008266636250413f; else ZretUaIsRRGIsZUCwgcnaqaZvklejz=1688470097.797921912076864980762209051883f;if (ZretUaIsRRGIsZUCwgcnaqaZvklejz - ZretUaIsRRGIsZUCwgcnaqaZvklejz> 0.00000001 ) ZretUaIsRRGIsZUCwgcnaqaZvklejz=238019029.252693289729639188247075260898f; else ZretUaIsRRGIsZUCwgcnaqaZvklejz=1408303981.123092437281105478197365738854f;if (ZretUaIsRRGIsZUCwgcnaqaZvklejz - ZretUaIsRRGIsZUCwgcnaqaZvklejz> 0.00000001 ) ZretUaIsRRGIsZUCwgcnaqaZvklejz=297301758.449171665786207698189271286846f; else ZretUaIsRRGIsZUCwgcnaqaZvklejz=1465203322.578394303482442127914188628900f;int oKqMEquHPRwYhymtOsCevuOOVwAklf=2106488725;if (oKqMEquHPRwYhymtOsCevuOOVwAklf == oKqMEquHPRwYhymtOsCevuOOVwAklf- 0 ) oKqMEquHPRwYhymtOsCevuOOVwAklf=616742289; else oKqMEquHPRwYhymtOsCevuOOVwAklf=1694352786;if (oKqMEquHPRwYhymtOsCevuOOVwAklf == oKqMEquHPRwYhymtOsCevuOOVwAklf- 0 ) oKqMEquHPRwYhymtOsCevuOOVwAklf=333919813; else oKqMEquHPRwYhymtOsCevuOOVwAklf=1376980374;if (oKqMEquHPRwYhymtOsCevuOOVwAklf == oKqMEquHPRwYhymtOsCevuOOVwAklf- 1 ) oKqMEquHPRwYhymtOsCevuOOVwAklf=471818219; else oKqMEquHPRwYhymtOsCevuOOVwAklf=176952109;if (oKqMEquHPRwYhymtOsCevuOOVwAklf == oKqMEquHPRwYhymtOsCevuOOVwAklf- 0 ) oKqMEquHPRwYhymtOsCevuOOVwAklf=1774498368; else oKqMEquHPRwYhymtOsCevuOOVwAklf=2061546217;if (oKqMEquHPRwYhymtOsCevuOOVwAklf == oKqMEquHPRwYhymtOsCevuOOVwAklf- 0 ) oKqMEquHPRwYhymtOsCevuOOVwAklf=336890951; else oKqMEquHPRwYhymtOsCevuOOVwAklf=1753289901;if (oKqMEquHPRwYhymtOsCevuOOVwAklf == oKqMEquHPRwYhymtOsCevuOOVwAklf- 0 ) oKqMEquHPRwYhymtOsCevuOOVwAklf=218940119; else oKqMEquHPRwYhymtOsCevuOOVwAklf=122310430;double zGtwzdrneqQRenQGBfYcHqUWuTSEOg=834320886.408372720321245784884002476650;if (zGtwzdrneqQRenQGBfYcHqUWuTSEOg == zGtwzdrneqQRenQGBfYcHqUWuTSEOg ) zGtwzdrneqQRenQGBfYcHqUWuTSEOg=1779353114.775877905610362534187595940691; else zGtwzdrneqQRenQGBfYcHqUWuTSEOg=1499376366.717165758888037485194258338194;if (zGtwzdrneqQRenQGBfYcHqUWuTSEOg == zGtwzdrneqQRenQGBfYcHqUWuTSEOg ) zGtwzdrneqQRenQGBfYcHqUWuTSEOg=1781478906.842400320799740067867551757094; else zGtwzdrneqQRenQGBfYcHqUWuTSEOg=1530533014.776415764716551662937234739285;if (zGtwzdrneqQRenQGBfYcHqUWuTSEOg == zGtwzdrneqQRenQGBfYcHqUWuTSEOg ) zGtwzdrneqQRenQGBfYcHqUWuTSEOg=850835017.444702333640425111120526765288; else zGtwzdrneqQRenQGBfYcHqUWuTSEOg=1095622173.669938385933930791073927693915;if (zGtwzdrneqQRenQGBfYcHqUWuTSEOg == zGtwzdrneqQRenQGBfYcHqUWuTSEOg ) zGtwzdrneqQRenQGBfYcHqUWuTSEOg=413833616.816608250893629779607432685344; else zGtwzdrneqQRenQGBfYcHqUWuTSEOg=1894982043.548112843255238130893431859513;if (zGtwzdrneqQRenQGBfYcHqUWuTSEOg == zGtwzdrneqQRenQGBfYcHqUWuTSEOg ) zGtwzdrneqQRenQGBfYcHqUWuTSEOg=1150245760.402635562703646331455555221837; else zGtwzdrneqQRenQGBfYcHqUWuTSEOg=1402825551.443186370467819090552826806751;if (zGtwzdrneqQRenQGBfYcHqUWuTSEOg == zGtwzdrneqQRenQGBfYcHqUWuTSEOg ) zGtwzdrneqQRenQGBfYcHqUWuTSEOg=739807387.473647166006109984624238563839; else zGtwzdrneqQRenQGBfYcHqUWuTSEOg=1178286060.172900438303466782702213824618;double ONjyQUHzyXYhSJBpnMqrEDRkufuFpc=139664656.497657002895969510020180926421;if (ONjyQUHzyXYhSJBpnMqrEDRkufuFpc == ONjyQUHzyXYhSJBpnMqrEDRkufuFpc ) ONjyQUHzyXYhSJBpnMqrEDRkufuFpc=38525790.720341969540720514565505932883; else ONjyQUHzyXYhSJBpnMqrEDRkufuFpc=1796307677.179227562966258157141296722651;if (ONjyQUHzyXYhSJBpnMqrEDRkufuFpc == ONjyQUHzyXYhSJBpnMqrEDRkufuFpc ) ONjyQUHzyXYhSJBpnMqrEDRkufuFpc=661363079.028648250315271945799574299361; else ONjyQUHzyXYhSJBpnMqrEDRkufuFpc=897269271.502204861100884713588897255445;if (ONjyQUHzyXYhSJBpnMqrEDRkufuFpc == ONjyQUHzyXYhSJBpnMqrEDRkufuFpc ) ONjyQUHzyXYhSJBpnMqrEDRkufuFpc=1320453337.090081417829560089169698381605; else ONjyQUHzyXYhSJBpnMqrEDRkufuFpc=1122789363.943621181330687043838648741280;if (ONjyQUHzyXYhSJBpnMqrEDRkufuFpc == ONjyQUHzyXYhSJBpnMqrEDRkufuFpc ) ONjyQUHzyXYhSJBpnMqrEDRkufuFpc=965851421.191398795900994911235898831508; else ONjyQUHzyXYhSJBpnMqrEDRkufuFpc=1060572307.191134940945994885584005631456;if (ONjyQUHzyXYhSJBpnMqrEDRkufuFpc == ONjyQUHzyXYhSJBpnMqrEDRkufuFpc ) ONjyQUHzyXYhSJBpnMqrEDRkufuFpc=1795774409.729881986008239000506613519760; else ONjyQUHzyXYhSJBpnMqrEDRkufuFpc=358721067.469522712390958697663205022647;if (ONjyQUHzyXYhSJBpnMqrEDRkufuFpc == ONjyQUHzyXYhSJBpnMqrEDRkufuFpc ) ONjyQUHzyXYhSJBpnMqrEDRkufuFpc=1046302549.233775082840921677693211826451; else ONjyQUHzyXYhSJBpnMqrEDRkufuFpc=363824642.118516838372692448445615503295;long cOEiPITFZsTtMMEhJtRtqxCeZyRBDS=512931407;if (cOEiPITFZsTtMMEhJtRtqxCeZyRBDS == cOEiPITFZsTtMMEhJtRtqxCeZyRBDS- 0 ) cOEiPITFZsTtMMEhJtRtqxCeZyRBDS=1518888634; else cOEiPITFZsTtMMEhJtRtqxCeZyRBDS=1610464519;if (cOEiPITFZsTtMMEhJtRtqxCeZyRBDS == cOEiPITFZsTtMMEhJtRtqxCeZyRBDS- 0 ) cOEiPITFZsTtMMEhJtRtqxCeZyRBDS=1833333855; else cOEiPITFZsTtMMEhJtRtqxCeZyRBDS=390194158;if (cOEiPITFZsTtMMEhJtRtqxCeZyRBDS == cOEiPITFZsTtMMEhJtRtqxCeZyRBDS- 0 ) cOEiPITFZsTtMMEhJtRtqxCeZyRBDS=686884589; else cOEiPITFZsTtMMEhJtRtqxCeZyRBDS=2044872350;if (cOEiPITFZsTtMMEhJtRtqxCeZyRBDS == cOEiPITFZsTtMMEhJtRtqxCeZyRBDS- 1 ) cOEiPITFZsTtMMEhJtRtqxCeZyRBDS=1255653708; else cOEiPITFZsTtMMEhJtRtqxCeZyRBDS=1668603763;if (cOEiPITFZsTtMMEhJtRtqxCeZyRBDS == cOEiPITFZsTtMMEhJtRtqxCeZyRBDS- 1 ) cOEiPITFZsTtMMEhJtRtqxCeZyRBDS=1280519170; else cOEiPITFZsTtMMEhJtRtqxCeZyRBDS=586421417;if (cOEiPITFZsTtMMEhJtRtqxCeZyRBDS == cOEiPITFZsTtMMEhJtRtqxCeZyRBDS- 0 ) cOEiPITFZsTtMMEhJtRtqxCeZyRBDS=226734614; else cOEiPITFZsTtMMEhJtRtqxCeZyRBDS=1216476315;float VYHLIsXMRIujhorqhipMpbBjfmwSdl=155080805.289745527409053234834764183635f;if (VYHLIsXMRIujhorqhipMpbBjfmwSdl - VYHLIsXMRIujhorqhipMpbBjfmwSdl> 0.00000001 ) VYHLIsXMRIujhorqhipMpbBjfmwSdl=607184702.746966357622477385396168263666f; else VYHLIsXMRIujhorqhipMpbBjfmwSdl=79275463.209277320708822239781845969971f;if (VYHLIsXMRIujhorqhipMpbBjfmwSdl - VYHLIsXMRIujhorqhipMpbBjfmwSdl> 0.00000001 ) VYHLIsXMRIujhorqhipMpbBjfmwSdl=83784640.370252430116560944603498546531f; else VYHLIsXMRIujhorqhipMpbBjfmwSdl=823220154.845053968091078648513638809282f;if (VYHLIsXMRIujhorqhipMpbBjfmwSdl - VYHLIsXMRIujhorqhipMpbBjfmwSdl> 0.00000001 ) VYHLIsXMRIujhorqhipMpbBjfmwSdl=1211208190.600937162709672243191902931826f; else VYHLIsXMRIujhorqhipMpbBjfmwSdl=1139629723.589749214258782507666619180811f;if (VYHLIsXMRIujhorqhipMpbBjfmwSdl - VYHLIsXMRIujhorqhipMpbBjfmwSdl> 0.00000001 ) VYHLIsXMRIujhorqhipMpbBjfmwSdl=1661351196.644732003313995333153527097166f; else VYHLIsXMRIujhorqhipMpbBjfmwSdl=656563751.041471506150446383486886822874f;if (VYHLIsXMRIujhorqhipMpbBjfmwSdl - VYHLIsXMRIujhorqhipMpbBjfmwSdl> 0.00000001 ) VYHLIsXMRIujhorqhipMpbBjfmwSdl=533524945.504737263493810419182579137666f; else VYHLIsXMRIujhorqhipMpbBjfmwSdl=720906599.974266771912019318751479313661f;if (VYHLIsXMRIujhorqhipMpbBjfmwSdl - VYHLIsXMRIujhorqhipMpbBjfmwSdl> 0.00000001 ) VYHLIsXMRIujhorqhipMpbBjfmwSdl=1882242252.071853065474928157093001560506f; else VYHLIsXMRIujhorqhipMpbBjfmwSdl=492113950.411004236745338755226678808030f;float XgvDhOtPSvNGLZqmjQoDaOnymURJwf=966630686.019165716181247250903005986980f;if (XgvDhOtPSvNGLZqmjQoDaOnymURJwf - XgvDhOtPSvNGLZqmjQoDaOnymURJwf> 0.00000001 ) XgvDhOtPSvNGLZqmjQoDaOnymURJwf=186399638.720092368436490400963206112538f; else XgvDhOtPSvNGLZqmjQoDaOnymURJwf=226875980.983756136699268872659612863499f;if (XgvDhOtPSvNGLZqmjQoDaOnymURJwf - XgvDhOtPSvNGLZqmjQoDaOnymURJwf> 0.00000001 ) XgvDhOtPSvNGLZqmjQoDaOnymURJwf=1806881073.194155375437538404844098937299f; else XgvDhOtPSvNGLZqmjQoDaOnymURJwf=846214726.485750611460823328546966511633f;if (XgvDhOtPSvNGLZqmjQoDaOnymURJwf - XgvDhOtPSvNGLZqmjQoDaOnymURJwf> 0.00000001 ) XgvDhOtPSvNGLZqmjQoDaOnymURJwf=495959938.928616398999458428662976352025f; else XgvDhOtPSvNGLZqmjQoDaOnymURJwf=755911041.009965231713076861820317105337f;if (XgvDhOtPSvNGLZqmjQoDaOnymURJwf - XgvDhOtPSvNGLZqmjQoDaOnymURJwf> 0.00000001 ) XgvDhOtPSvNGLZqmjQoDaOnymURJwf=1600263714.537490060420796991160121570386f; else XgvDhOtPSvNGLZqmjQoDaOnymURJwf=141137087.000538778780250072231437751603f;if (XgvDhOtPSvNGLZqmjQoDaOnymURJwf - XgvDhOtPSvNGLZqmjQoDaOnymURJwf> 0.00000001 ) XgvDhOtPSvNGLZqmjQoDaOnymURJwf=540653969.089543037823459108261815567489f; else XgvDhOtPSvNGLZqmjQoDaOnymURJwf=1140242963.728570281704641919875444337408f;if (XgvDhOtPSvNGLZqmjQoDaOnymURJwf - XgvDhOtPSvNGLZqmjQoDaOnymURJwf> 0.00000001 ) XgvDhOtPSvNGLZqmjQoDaOnymURJwf=1764233748.675811698003856728476962448434f; else XgvDhOtPSvNGLZqmjQoDaOnymURJwf=734031920.388314994133198369961013381447f;long MdWkJVXUJoSUOVQrmTDrtWmPOjbDSM=1199562101;if (MdWkJVXUJoSUOVQrmTDrtWmPOjbDSM == MdWkJVXUJoSUOVQrmTDrtWmPOjbDSM- 1 ) MdWkJVXUJoSUOVQrmTDrtWmPOjbDSM=753854095; else MdWkJVXUJoSUOVQrmTDrtWmPOjbDSM=891221744;if (MdWkJVXUJoSUOVQrmTDrtWmPOjbDSM == MdWkJVXUJoSUOVQrmTDrtWmPOjbDSM- 0 ) MdWkJVXUJoSUOVQrmTDrtWmPOjbDSM=1904123587; else MdWkJVXUJoSUOVQrmTDrtWmPOjbDSM=1856530808;if (MdWkJVXUJoSUOVQrmTDrtWmPOjbDSM == MdWkJVXUJoSUOVQrmTDrtWmPOjbDSM- 1 ) MdWkJVXUJoSUOVQrmTDrtWmPOjbDSM=99830434; else MdWkJVXUJoSUOVQrmTDrtWmPOjbDSM=393205759;if (MdWkJVXUJoSUOVQrmTDrtWmPOjbDSM == MdWkJVXUJoSUOVQrmTDrtWmPOjbDSM- 0 ) MdWkJVXUJoSUOVQrmTDrtWmPOjbDSM=1449206864; else MdWkJVXUJoSUOVQrmTDrtWmPOjbDSM=379258911;if (MdWkJVXUJoSUOVQrmTDrtWmPOjbDSM == MdWkJVXUJoSUOVQrmTDrtWmPOjbDSM- 1 ) MdWkJVXUJoSUOVQrmTDrtWmPOjbDSM=2096596970; else MdWkJVXUJoSUOVQrmTDrtWmPOjbDSM=1176770104;if (MdWkJVXUJoSUOVQrmTDrtWmPOjbDSM == MdWkJVXUJoSUOVQrmTDrtWmPOjbDSM- 0 ) MdWkJVXUJoSUOVQrmTDrtWmPOjbDSM=2004152860; else MdWkJVXUJoSUOVQrmTDrtWmPOjbDSM=2113461637;long eHEdtJDpQJpuIaEoIWaxqhzyfvnvbb=1230510196;if (eHEdtJDpQJpuIaEoIWaxqhzyfvnvbb == eHEdtJDpQJpuIaEoIWaxqhzyfvnvbb- 0 ) eHEdtJDpQJpuIaEoIWaxqhzyfvnvbb=627245645; else eHEdtJDpQJpuIaEoIWaxqhzyfvnvbb=1407587575;if (eHEdtJDpQJpuIaEoIWaxqhzyfvnvbb == eHEdtJDpQJpuIaEoIWaxqhzyfvnvbb- 1 ) eHEdtJDpQJpuIaEoIWaxqhzyfvnvbb=1030076677; else eHEdtJDpQJpuIaEoIWaxqhzyfvnvbb=1357752389;if (eHEdtJDpQJpuIaEoIWaxqhzyfvnvbb == eHEdtJDpQJpuIaEoIWaxqhzyfvnvbb- 1 ) eHEdtJDpQJpuIaEoIWaxqhzyfvnvbb=936814882; else eHEdtJDpQJpuIaEoIWaxqhzyfvnvbb=906135893;if (eHEdtJDpQJpuIaEoIWaxqhzyfvnvbb == eHEdtJDpQJpuIaEoIWaxqhzyfvnvbb- 0 ) eHEdtJDpQJpuIaEoIWaxqhzyfvnvbb=1806604514; else eHEdtJDpQJpuIaEoIWaxqhzyfvnvbb=1998757308;if (eHEdtJDpQJpuIaEoIWaxqhzyfvnvbb == eHEdtJDpQJpuIaEoIWaxqhzyfvnvbb- 0 ) eHEdtJDpQJpuIaEoIWaxqhzyfvnvbb=626495396; else eHEdtJDpQJpuIaEoIWaxqhzyfvnvbb=2090131200;if (eHEdtJDpQJpuIaEoIWaxqhzyfvnvbb == eHEdtJDpQJpuIaEoIWaxqhzyfvnvbb- 0 ) eHEdtJDpQJpuIaEoIWaxqhzyfvnvbb=282566185; else eHEdtJDpQJpuIaEoIWaxqhzyfvnvbb=70965227;float JTrFadmHHBILbSKmxtUaLrqPljFkbr=784149839.721346744366541699267556841167f;if (JTrFadmHHBILbSKmxtUaLrqPljFkbr - JTrFadmHHBILbSKmxtUaLrqPljFkbr> 0.00000001 ) JTrFadmHHBILbSKmxtUaLrqPljFkbr=1244962661.334980802467424241018515938660f; else JTrFadmHHBILbSKmxtUaLrqPljFkbr=682680151.579814266199881013361125982916f;if (JTrFadmHHBILbSKmxtUaLrqPljFkbr - JTrFadmHHBILbSKmxtUaLrqPljFkbr> 0.00000001 ) JTrFadmHHBILbSKmxtUaLrqPljFkbr=1885743502.335209972018593074503530955503f; else JTrFadmHHBILbSKmxtUaLrqPljFkbr=1305581527.236196479225236473201951380536f;if (JTrFadmHHBILbSKmxtUaLrqPljFkbr - JTrFadmHHBILbSKmxtUaLrqPljFkbr> 0.00000001 ) JTrFadmHHBILbSKmxtUaLrqPljFkbr=293369034.557998240761639941569231249582f; else JTrFadmHHBILbSKmxtUaLrqPljFkbr=250128611.794387835823454133354018833775f;if (JTrFadmHHBILbSKmxtUaLrqPljFkbr - JTrFadmHHBILbSKmxtUaLrqPljFkbr> 0.00000001 ) JTrFadmHHBILbSKmxtUaLrqPljFkbr=866800252.087917735222674698721623879766f; else JTrFadmHHBILbSKmxtUaLrqPljFkbr=1940677526.516195765741824344524516446414f;if (JTrFadmHHBILbSKmxtUaLrqPljFkbr - JTrFadmHHBILbSKmxtUaLrqPljFkbr> 0.00000001 ) JTrFadmHHBILbSKmxtUaLrqPljFkbr=925152763.917981768904409181064525300076f; else JTrFadmHHBILbSKmxtUaLrqPljFkbr=1427622162.045193405378772122836646140378f;if (JTrFadmHHBILbSKmxtUaLrqPljFkbr - JTrFadmHHBILbSKmxtUaLrqPljFkbr> 0.00000001 ) JTrFadmHHBILbSKmxtUaLrqPljFkbr=897101403.279297931679670796328934150016f; else JTrFadmHHBILbSKmxtUaLrqPljFkbr=1466673481.423720958620127708745405034524f;int yMbMYhwXwoXpoCtpEnTcKPgEyMmjwf=1924597858;if (yMbMYhwXwoXpoCtpEnTcKPgEyMmjwf == yMbMYhwXwoXpoCtpEnTcKPgEyMmjwf- 0 ) yMbMYhwXwoXpoCtpEnTcKPgEyMmjwf=1668442471; else yMbMYhwXwoXpoCtpEnTcKPgEyMmjwf=533723552;if (yMbMYhwXwoXpoCtpEnTcKPgEyMmjwf == yMbMYhwXwoXpoCtpEnTcKPgEyMmjwf- 1 ) yMbMYhwXwoXpoCtpEnTcKPgEyMmjwf=106786275; else yMbMYhwXwoXpoCtpEnTcKPgEyMmjwf=1635950821;if (yMbMYhwXwoXpoCtpEnTcKPgEyMmjwf == yMbMYhwXwoXpoCtpEnTcKPgEyMmjwf- 0 ) yMbMYhwXwoXpoCtpEnTcKPgEyMmjwf=866782076; else yMbMYhwXwoXpoCtpEnTcKPgEyMmjwf=265675196;if (yMbMYhwXwoXpoCtpEnTcKPgEyMmjwf == yMbMYhwXwoXpoCtpEnTcKPgEyMmjwf- 1 ) yMbMYhwXwoXpoCtpEnTcKPgEyMmjwf=262747219; else yMbMYhwXwoXpoCtpEnTcKPgEyMmjwf=1995201849;if (yMbMYhwXwoXpoCtpEnTcKPgEyMmjwf == yMbMYhwXwoXpoCtpEnTcKPgEyMmjwf- 1 ) yMbMYhwXwoXpoCtpEnTcKPgEyMmjwf=1232100877; else yMbMYhwXwoXpoCtpEnTcKPgEyMmjwf=2052849926;if (yMbMYhwXwoXpoCtpEnTcKPgEyMmjwf == yMbMYhwXwoXpoCtpEnTcKPgEyMmjwf- 0 ) yMbMYhwXwoXpoCtpEnTcKPgEyMmjwf=1230035412; else yMbMYhwXwoXpoCtpEnTcKPgEyMmjwf=157182881;double hhOgoBxdEYaWCFiwbdixBVwXhGGdML=1169277694.319494247844815690685037405182;if (hhOgoBxdEYaWCFiwbdixBVwXhGGdML == hhOgoBxdEYaWCFiwbdixBVwXhGGdML ) hhOgoBxdEYaWCFiwbdixBVwXhGGdML=1416261102.817508492950641495100535238310; else hhOgoBxdEYaWCFiwbdixBVwXhGGdML=1220003021.926504831720974550611119348337;if (hhOgoBxdEYaWCFiwbdixBVwXhGGdML == hhOgoBxdEYaWCFiwbdixBVwXhGGdML ) hhOgoBxdEYaWCFiwbdixBVwXhGGdML=868498976.029213514876448176279970992836; else hhOgoBxdEYaWCFiwbdixBVwXhGGdML=1627433594.661570177114684577128317826043;if (hhOgoBxdEYaWCFiwbdixBVwXhGGdML == hhOgoBxdEYaWCFiwbdixBVwXhGGdML ) hhOgoBxdEYaWCFiwbdixBVwXhGGdML=1025279903.279832608800917639652671945476; else hhOgoBxdEYaWCFiwbdixBVwXhGGdML=313702612.524504696397042841039749171800;if (hhOgoBxdEYaWCFiwbdixBVwXhGGdML == hhOgoBxdEYaWCFiwbdixBVwXhGGdML ) hhOgoBxdEYaWCFiwbdixBVwXhGGdML=937141162.734239752315436564011544789553; else hhOgoBxdEYaWCFiwbdixBVwXhGGdML=377117034.490677481429983541371799039066;if (hhOgoBxdEYaWCFiwbdixBVwXhGGdML == hhOgoBxdEYaWCFiwbdixBVwXhGGdML ) hhOgoBxdEYaWCFiwbdixBVwXhGGdML=540787323.121993587019460267225829381275; else hhOgoBxdEYaWCFiwbdixBVwXhGGdML=1236882798.942640909116446343987294131372;if (hhOgoBxdEYaWCFiwbdixBVwXhGGdML == hhOgoBxdEYaWCFiwbdixBVwXhGGdML ) hhOgoBxdEYaWCFiwbdixBVwXhGGdML=432599626.342327783136822059314839876861; else hhOgoBxdEYaWCFiwbdixBVwXhGGdML=1656676367.074897523966196157319307952355;float rOHEhOBQZYNdYWnmQSkKucVMkkiwTZ=1139693319.022013481018385562090724366072f;if (rOHEhOBQZYNdYWnmQSkKucVMkkiwTZ - rOHEhOBQZYNdYWnmQSkKucVMkkiwTZ> 0.00000001 ) rOHEhOBQZYNdYWnmQSkKucVMkkiwTZ=2111624211.406259883219224123260739841876f; else rOHEhOBQZYNdYWnmQSkKucVMkkiwTZ=505451974.852061314779988251134392388741f;if (rOHEhOBQZYNdYWnmQSkKucVMkkiwTZ - rOHEhOBQZYNdYWnmQSkKucVMkkiwTZ> 0.00000001 ) rOHEhOBQZYNdYWnmQSkKucVMkkiwTZ=550006403.811857826392059186179184256089f; else rOHEhOBQZYNdYWnmQSkKucVMkkiwTZ=989156070.467307814243585334321268373770f;if (rOHEhOBQZYNdYWnmQSkKucVMkkiwTZ - rOHEhOBQZYNdYWnmQSkKucVMkkiwTZ> 0.00000001 ) rOHEhOBQZYNdYWnmQSkKucVMkkiwTZ=820533001.185793586247304660003376931013f; else rOHEhOBQZYNdYWnmQSkKucVMkkiwTZ=272041828.515426990386828407471487881997f;if (rOHEhOBQZYNdYWnmQSkKucVMkkiwTZ - rOHEhOBQZYNdYWnmQSkKucVMkkiwTZ> 0.00000001 ) rOHEhOBQZYNdYWnmQSkKucVMkkiwTZ=2064851410.268484298888172766953464323975f; else rOHEhOBQZYNdYWnmQSkKucVMkkiwTZ=979287465.500313855431970431512614480417f;if (rOHEhOBQZYNdYWnmQSkKucVMkkiwTZ - rOHEhOBQZYNdYWnmQSkKucVMkkiwTZ> 0.00000001 ) rOHEhOBQZYNdYWnmQSkKucVMkkiwTZ=419932932.142448887092253260509247390793f; else rOHEhOBQZYNdYWnmQSkKucVMkkiwTZ=2038025300.454940964266562156233102890625f;if (rOHEhOBQZYNdYWnmQSkKucVMkkiwTZ - rOHEhOBQZYNdYWnmQSkKucVMkkiwTZ> 0.00000001 ) rOHEhOBQZYNdYWnmQSkKucVMkkiwTZ=73480650.929575842430744489525457035215f; else rOHEhOBQZYNdYWnmQSkKucVMkkiwTZ=388002755.044035710742580356952210057354f;int gOlTxfkCxiovqrPtkljtuRcPsMUwpn=1694394773;if (gOlTxfkCxiovqrPtkljtuRcPsMUwpn == gOlTxfkCxiovqrPtkljtuRcPsMUwpn- 0 ) gOlTxfkCxiovqrPtkljtuRcPsMUwpn=423759226; else gOlTxfkCxiovqrPtkljtuRcPsMUwpn=1875307356;if (gOlTxfkCxiovqrPtkljtuRcPsMUwpn == gOlTxfkCxiovqrPtkljtuRcPsMUwpn- 1 ) gOlTxfkCxiovqrPtkljtuRcPsMUwpn=50412379; else gOlTxfkCxiovqrPtkljtuRcPsMUwpn=1826722454;if (gOlTxfkCxiovqrPtkljtuRcPsMUwpn == gOlTxfkCxiovqrPtkljtuRcPsMUwpn- 1 ) gOlTxfkCxiovqrPtkljtuRcPsMUwpn=765786748; else gOlTxfkCxiovqrPtkljtuRcPsMUwpn=498557042;if (gOlTxfkCxiovqrPtkljtuRcPsMUwpn == gOlTxfkCxiovqrPtkljtuRcPsMUwpn- 1 ) gOlTxfkCxiovqrPtkljtuRcPsMUwpn=899317955; else gOlTxfkCxiovqrPtkljtuRcPsMUwpn=1354739859;if (gOlTxfkCxiovqrPtkljtuRcPsMUwpn == gOlTxfkCxiovqrPtkljtuRcPsMUwpn- 1 ) gOlTxfkCxiovqrPtkljtuRcPsMUwpn=1744066037; else gOlTxfkCxiovqrPtkljtuRcPsMUwpn=1877152888;if (gOlTxfkCxiovqrPtkljtuRcPsMUwpn == gOlTxfkCxiovqrPtkljtuRcPsMUwpn- 0 ) gOlTxfkCxiovqrPtkljtuRcPsMUwpn=1095163498; else gOlTxfkCxiovqrPtkljtuRcPsMUwpn=1777382925; }
 gOlTxfkCxiovqrPtkljtuRcPsMUwpny::gOlTxfkCxiovqrPtkljtuRcPsMUwpny()
 { this->QwxmtfvIATxF("dfijsmVzjlbsqRKOIigxZnXFOmValKQwxmtfvIATxFj", true, 1980559217, 753554499, 1986167359); }
#pragma optimize("", off)
 // <delete/>


// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class xplzdzQbILQZaFiRzTpSfvnvlzlCwpy
 { 
public: bool GcfHjQkmXrLVXWtpdfkUEssMCCpOdN; double GcfHjQkmXrLVXWtpdfkUEssMCCpOdNxplzdzQbILQZaFiRzTpSfvnvlzlCwp; xplzdzQbILQZaFiRzTpSfvnvlzlCwpy(); void rhCGcTMxLwnc(string GcfHjQkmXrLVXWtpdfkUEssMCCpOdNrhCGcTMxLwnc, bool NfTmKwuxzAPgRaUZQZrKHXLUUNjSFh, int aWJKefClMwuggjFYaSsclQWTFpTySw, float MbrRasXHvoXeNvPbrbOvmRadvKApEo, long LDSPkSoOJNBTWEFGDbEdysjWJvBOVQ);
 protected: bool GcfHjQkmXrLVXWtpdfkUEssMCCpOdNo; double GcfHjQkmXrLVXWtpdfkUEssMCCpOdNxplzdzQbILQZaFiRzTpSfvnvlzlCwpf; void rhCGcTMxLwncu(string GcfHjQkmXrLVXWtpdfkUEssMCCpOdNrhCGcTMxLwncg, bool NfTmKwuxzAPgRaUZQZrKHXLUUNjSFhe, int aWJKefClMwuggjFYaSsclQWTFpTySwr, float MbrRasXHvoXeNvPbrbOvmRadvKApEow, long LDSPkSoOJNBTWEFGDbEdysjWJvBOVQn);
 private: bool GcfHjQkmXrLVXWtpdfkUEssMCCpOdNNfTmKwuxzAPgRaUZQZrKHXLUUNjSFh; double GcfHjQkmXrLVXWtpdfkUEssMCCpOdNMbrRasXHvoXeNvPbrbOvmRadvKApEoxplzdzQbILQZaFiRzTpSfvnvlzlCwp;
 void rhCGcTMxLwncv(string NfTmKwuxzAPgRaUZQZrKHXLUUNjSFhrhCGcTMxLwnc, bool NfTmKwuxzAPgRaUZQZrKHXLUUNjSFhaWJKefClMwuggjFYaSsclQWTFpTySw, int aWJKefClMwuggjFYaSsclQWTFpTySwGcfHjQkmXrLVXWtpdfkUEssMCCpOdN, float MbrRasXHvoXeNvPbrbOvmRadvKApEoLDSPkSoOJNBTWEFGDbEdysjWJvBOVQ, long LDSPkSoOJNBTWEFGDbEdysjWJvBOVQNfTmKwuxzAPgRaUZQZrKHXLUUNjSFh); };
 void xplzdzQbILQZaFiRzTpSfvnvlzlCwpy::rhCGcTMxLwnc(string GcfHjQkmXrLVXWtpdfkUEssMCCpOdNrhCGcTMxLwnc, bool NfTmKwuxzAPgRaUZQZrKHXLUUNjSFh, int aWJKefClMwuggjFYaSsclQWTFpTySw, float MbrRasXHvoXeNvPbrbOvmRadvKApEo, long LDSPkSoOJNBTWEFGDbEdysjWJvBOVQ)
 { float qVZBjMnpLbLNGcehzHpGYnMRfiNohS=101401670.683931436031791361241821934387f;if (qVZBjMnpLbLNGcehzHpGYnMRfiNohS - qVZBjMnpLbLNGcehzHpGYnMRfiNohS> 0.00000001 ) qVZBjMnpLbLNGcehzHpGYnMRfiNohS=495874110.670445158672288376893663697311f; else qVZBjMnpLbLNGcehzHpGYnMRfiNohS=1470213832.715047245722478065367969411886f;if (qVZBjMnpLbLNGcehzHpGYnMRfiNohS - qVZBjMnpLbLNGcehzHpGYnMRfiNohS> 0.00000001 ) qVZBjMnpLbLNGcehzHpGYnMRfiNohS=1764771925.985796734123128220673305596763f; else qVZBjMnpLbLNGcehzHpGYnMRfiNohS=1503668693.657994778234134484239221469130f;if (qVZBjMnpLbLNGcehzHpGYnMRfiNohS - qVZBjMnpLbLNGcehzHpGYnMRfiNohS> 0.00000001 ) qVZBjMnpLbLNGcehzHpGYnMRfiNohS=1894592540.117168624466245928212254543763f; else qVZBjMnpLbLNGcehzHpGYnMRfiNohS=1079608100.675912808786197419654192897372f;if (qVZBjMnpLbLNGcehzHpGYnMRfiNohS - qVZBjMnpLbLNGcehzHpGYnMRfiNohS> 0.00000001 ) qVZBjMnpLbLNGcehzHpGYnMRfiNohS=1402565663.607290233323164726714411958313f; else qVZBjMnpLbLNGcehzHpGYnMRfiNohS=492170728.653718091838644214928177820644f;if (qVZBjMnpLbLNGcehzHpGYnMRfiNohS - qVZBjMnpLbLNGcehzHpGYnMRfiNohS> 0.00000001 ) qVZBjMnpLbLNGcehzHpGYnMRfiNohS=1917329199.293177145683699697874459558790f; else qVZBjMnpLbLNGcehzHpGYnMRfiNohS=1623683826.946775667728045568823387185463f;if (qVZBjMnpLbLNGcehzHpGYnMRfiNohS - qVZBjMnpLbLNGcehzHpGYnMRfiNohS> 0.00000001 ) qVZBjMnpLbLNGcehzHpGYnMRfiNohS=1348420277.946574803513126752410197766079f; else qVZBjMnpLbLNGcehzHpGYnMRfiNohS=1895932755.120588257286642037144315396981f;int pPUVxQfTsXxXNSUaXdjTuuLalFZKuJ=1342671386;if (pPUVxQfTsXxXNSUaXdjTuuLalFZKuJ == pPUVxQfTsXxXNSUaXdjTuuLalFZKuJ- 1 ) pPUVxQfTsXxXNSUaXdjTuuLalFZKuJ=1886559607; else pPUVxQfTsXxXNSUaXdjTuuLalFZKuJ=237846373;if (pPUVxQfTsXxXNSUaXdjTuuLalFZKuJ == pPUVxQfTsXxXNSUaXdjTuuLalFZKuJ- 0 ) pPUVxQfTsXxXNSUaXdjTuuLalFZKuJ=1302342595; else pPUVxQfTsXxXNSUaXdjTuuLalFZKuJ=1919821809;if (pPUVxQfTsXxXNSUaXdjTuuLalFZKuJ == pPUVxQfTsXxXNSUaXdjTuuLalFZKuJ- 0 ) pPUVxQfTsXxXNSUaXdjTuuLalFZKuJ=2098741767; else pPUVxQfTsXxXNSUaXdjTuuLalFZKuJ=1705505910;if (pPUVxQfTsXxXNSUaXdjTuuLalFZKuJ == pPUVxQfTsXxXNSUaXdjTuuLalFZKuJ- 1 ) pPUVxQfTsXxXNSUaXdjTuuLalFZKuJ=498802078; else pPUVxQfTsXxXNSUaXdjTuuLalFZKuJ=1925061109;if (pPUVxQfTsXxXNSUaXdjTuuLalFZKuJ == pPUVxQfTsXxXNSUaXdjTuuLalFZKuJ- 1 ) pPUVxQfTsXxXNSUaXdjTuuLalFZKuJ=1394183872; else pPUVxQfTsXxXNSUaXdjTuuLalFZKuJ=1158826724;if (pPUVxQfTsXxXNSUaXdjTuuLalFZKuJ == pPUVxQfTsXxXNSUaXdjTuuLalFZKuJ- 1 ) pPUVxQfTsXxXNSUaXdjTuuLalFZKuJ=1858605693; else pPUVxQfTsXxXNSUaXdjTuuLalFZKuJ=145360170;long CofwZoerjSAAlUDiZFHeecHxhajWyh=1227892010;if (CofwZoerjSAAlUDiZFHeecHxhajWyh == CofwZoerjSAAlUDiZFHeecHxhajWyh- 1 ) CofwZoerjSAAlUDiZFHeecHxhajWyh=506912996; else CofwZoerjSAAlUDiZFHeecHxhajWyh=1067024849;if (CofwZoerjSAAlUDiZFHeecHxhajWyh == CofwZoerjSAAlUDiZFHeecHxhajWyh- 0 ) CofwZoerjSAAlUDiZFHeecHxhajWyh=633018828; else CofwZoerjSAAlUDiZFHeecHxhajWyh=1407801403;if (CofwZoerjSAAlUDiZFHeecHxhajWyh == CofwZoerjSAAlUDiZFHeecHxhajWyh- 1 ) CofwZoerjSAAlUDiZFHeecHxhajWyh=866147842; else CofwZoerjSAAlUDiZFHeecHxhajWyh=1760620796;if (CofwZoerjSAAlUDiZFHeecHxhajWyh == CofwZoerjSAAlUDiZFHeecHxhajWyh- 1 ) CofwZoerjSAAlUDiZFHeecHxhajWyh=141057715; else CofwZoerjSAAlUDiZFHeecHxhajWyh=1894878998;if (CofwZoerjSAAlUDiZFHeecHxhajWyh == CofwZoerjSAAlUDiZFHeecHxhajWyh- 1 ) CofwZoerjSAAlUDiZFHeecHxhajWyh=1767032118; else CofwZoerjSAAlUDiZFHeecHxhajWyh=1024070743;if (CofwZoerjSAAlUDiZFHeecHxhajWyh == CofwZoerjSAAlUDiZFHeecHxhajWyh- 0 ) CofwZoerjSAAlUDiZFHeecHxhajWyh=88702256; else CofwZoerjSAAlUDiZFHeecHxhajWyh=1593231803;float StpYsJegDDppHTbjKwUeMMRsFJLpnf=114587429.087216365658420954966906182896f;if (StpYsJegDDppHTbjKwUeMMRsFJLpnf - StpYsJegDDppHTbjKwUeMMRsFJLpnf> 0.00000001 ) StpYsJegDDppHTbjKwUeMMRsFJLpnf=1568421693.021712014428657640180383929223f; else StpYsJegDDppHTbjKwUeMMRsFJLpnf=359150361.457063872904055945955382785999f;if (StpYsJegDDppHTbjKwUeMMRsFJLpnf - StpYsJegDDppHTbjKwUeMMRsFJLpnf> 0.00000001 ) StpYsJegDDppHTbjKwUeMMRsFJLpnf=1151288615.490207160497829643809437928649f; else StpYsJegDDppHTbjKwUeMMRsFJLpnf=441915903.998361595898556956314210106339f;if (StpYsJegDDppHTbjKwUeMMRsFJLpnf - StpYsJegDDppHTbjKwUeMMRsFJLpnf> 0.00000001 ) StpYsJegDDppHTbjKwUeMMRsFJLpnf=973609784.053260967825902108217344579927f; else StpYsJegDDppHTbjKwUeMMRsFJLpnf=1008423485.526694640600059031900823272070f;if (StpYsJegDDppHTbjKwUeMMRsFJLpnf - StpYsJegDDppHTbjKwUeMMRsFJLpnf> 0.00000001 ) StpYsJegDDppHTbjKwUeMMRsFJLpnf=150143550.214030932866961849564802781452f; else StpYsJegDDppHTbjKwUeMMRsFJLpnf=937851847.367150602011025407244362651236f;if (StpYsJegDDppHTbjKwUeMMRsFJLpnf - StpYsJegDDppHTbjKwUeMMRsFJLpnf> 0.00000001 ) StpYsJegDDppHTbjKwUeMMRsFJLpnf=971411754.969050092234856489838521901650f; else StpYsJegDDppHTbjKwUeMMRsFJLpnf=1987194463.047615317652512161737250854825f;if (StpYsJegDDppHTbjKwUeMMRsFJLpnf - StpYsJegDDppHTbjKwUeMMRsFJLpnf> 0.00000001 ) StpYsJegDDppHTbjKwUeMMRsFJLpnf=109484821.944239725127339702085103480786f; else StpYsJegDDppHTbjKwUeMMRsFJLpnf=735765816.250272849345761003226384195195f;long BRqxfoEdkudSkZyTTKQthZiCvmeDqi=1039728523;if (BRqxfoEdkudSkZyTTKQthZiCvmeDqi == BRqxfoEdkudSkZyTTKQthZiCvmeDqi- 0 ) BRqxfoEdkudSkZyTTKQthZiCvmeDqi=1726015697; else BRqxfoEdkudSkZyTTKQthZiCvmeDqi=120528267;if (BRqxfoEdkudSkZyTTKQthZiCvmeDqi == BRqxfoEdkudSkZyTTKQthZiCvmeDqi- 0 ) BRqxfoEdkudSkZyTTKQthZiCvmeDqi=1389019759; else BRqxfoEdkudSkZyTTKQthZiCvmeDqi=488686413;if (BRqxfoEdkudSkZyTTKQthZiCvmeDqi == BRqxfoEdkudSkZyTTKQthZiCvmeDqi- 1 ) BRqxfoEdkudSkZyTTKQthZiCvmeDqi=1791153391; else BRqxfoEdkudSkZyTTKQthZiCvmeDqi=743443618;if (BRqxfoEdkudSkZyTTKQthZiCvmeDqi == BRqxfoEdkudSkZyTTKQthZiCvmeDqi- 0 ) BRqxfoEdkudSkZyTTKQthZiCvmeDqi=476523544; else BRqxfoEdkudSkZyTTKQthZiCvmeDqi=125938811;if (BRqxfoEdkudSkZyTTKQthZiCvmeDqi == BRqxfoEdkudSkZyTTKQthZiCvmeDqi- 0 ) BRqxfoEdkudSkZyTTKQthZiCvmeDqi=96788658; else BRqxfoEdkudSkZyTTKQthZiCvmeDqi=1554947244;if (BRqxfoEdkudSkZyTTKQthZiCvmeDqi == BRqxfoEdkudSkZyTTKQthZiCvmeDqi- 0 ) BRqxfoEdkudSkZyTTKQthZiCvmeDqi=152789691; else BRqxfoEdkudSkZyTTKQthZiCvmeDqi=1074671024;double RrqRMWdmUjySucOfQplrUSURLTTvHh=743190260.196481517342759641068086500763;if (RrqRMWdmUjySucOfQplrUSURLTTvHh == RrqRMWdmUjySucOfQplrUSURLTTvHh ) RrqRMWdmUjySucOfQplrUSURLTTvHh=1628948529.716381344718480636324957222108; else RrqRMWdmUjySucOfQplrUSURLTTvHh=1744018264.186563757941746071608444997017;if (RrqRMWdmUjySucOfQplrUSURLTTvHh == RrqRMWdmUjySucOfQplrUSURLTTvHh ) RrqRMWdmUjySucOfQplrUSURLTTvHh=724422124.035663302898959422235150174367; else RrqRMWdmUjySucOfQplrUSURLTTvHh=1374519929.784091718377991069568689299569;if (RrqRMWdmUjySucOfQplrUSURLTTvHh == RrqRMWdmUjySucOfQplrUSURLTTvHh ) RrqRMWdmUjySucOfQplrUSURLTTvHh=1873795708.915622597981496927012580010690; else RrqRMWdmUjySucOfQplrUSURLTTvHh=1603235873.326947347967369720348255843561;if (RrqRMWdmUjySucOfQplrUSURLTTvHh == RrqRMWdmUjySucOfQplrUSURLTTvHh ) RrqRMWdmUjySucOfQplrUSURLTTvHh=254282226.301776655969026632240035318121; else RrqRMWdmUjySucOfQplrUSURLTTvHh=1645973158.599767670328648924032143247746;if (RrqRMWdmUjySucOfQplrUSURLTTvHh == RrqRMWdmUjySucOfQplrUSURLTTvHh ) RrqRMWdmUjySucOfQplrUSURLTTvHh=916881299.954558046006393259749410955753; else RrqRMWdmUjySucOfQplrUSURLTTvHh=1842650628.004557520109813695000900754461;if (RrqRMWdmUjySucOfQplrUSURLTTvHh == RrqRMWdmUjySucOfQplrUSURLTTvHh ) RrqRMWdmUjySucOfQplrUSURLTTvHh=436389649.349411385774975161737075410580; else RrqRMWdmUjySucOfQplrUSURLTTvHh=1026437026.316986050241143703569474538410;long szYUhqzwuGIFcuewNKkDkxLCvTHYWt=509826625;if (szYUhqzwuGIFcuewNKkDkxLCvTHYWt == szYUhqzwuGIFcuewNKkDkxLCvTHYWt- 0 ) szYUhqzwuGIFcuewNKkDkxLCvTHYWt=118086658; else szYUhqzwuGIFcuewNKkDkxLCvTHYWt=1196457380;if (szYUhqzwuGIFcuewNKkDkxLCvTHYWt == szYUhqzwuGIFcuewNKkDkxLCvTHYWt- 1 ) szYUhqzwuGIFcuewNKkDkxLCvTHYWt=2014699753; else szYUhqzwuGIFcuewNKkDkxLCvTHYWt=2088972700;if (szYUhqzwuGIFcuewNKkDkxLCvTHYWt == szYUhqzwuGIFcuewNKkDkxLCvTHYWt- 0 ) szYUhqzwuGIFcuewNKkDkxLCvTHYWt=1409457500; else szYUhqzwuGIFcuewNKkDkxLCvTHYWt=1188675429;if (szYUhqzwuGIFcuewNKkDkxLCvTHYWt == szYUhqzwuGIFcuewNKkDkxLCvTHYWt- 0 ) szYUhqzwuGIFcuewNKkDkxLCvTHYWt=470917685; else szYUhqzwuGIFcuewNKkDkxLCvTHYWt=824978075;if (szYUhqzwuGIFcuewNKkDkxLCvTHYWt == szYUhqzwuGIFcuewNKkDkxLCvTHYWt- 0 ) szYUhqzwuGIFcuewNKkDkxLCvTHYWt=2030110464; else szYUhqzwuGIFcuewNKkDkxLCvTHYWt=1025349804;if (szYUhqzwuGIFcuewNKkDkxLCvTHYWt == szYUhqzwuGIFcuewNKkDkxLCvTHYWt- 0 ) szYUhqzwuGIFcuewNKkDkxLCvTHYWt=345127245; else szYUhqzwuGIFcuewNKkDkxLCvTHYWt=1566146187;double ENcusCAeVUKpcwiLkAHOiEqapvIGVD=2065841479.706315028562393797696440801932;if (ENcusCAeVUKpcwiLkAHOiEqapvIGVD == ENcusCAeVUKpcwiLkAHOiEqapvIGVD ) ENcusCAeVUKpcwiLkAHOiEqapvIGVD=124150674.951623231679999986429739746751; else ENcusCAeVUKpcwiLkAHOiEqapvIGVD=1513778208.419672188166613765103652332299;if (ENcusCAeVUKpcwiLkAHOiEqapvIGVD == ENcusCAeVUKpcwiLkAHOiEqapvIGVD ) ENcusCAeVUKpcwiLkAHOiEqapvIGVD=1254300934.768127033881847413718266996845; else ENcusCAeVUKpcwiLkAHOiEqapvIGVD=1771951437.009940869673342282602059160334;if (ENcusCAeVUKpcwiLkAHOiEqapvIGVD == ENcusCAeVUKpcwiLkAHOiEqapvIGVD ) ENcusCAeVUKpcwiLkAHOiEqapvIGVD=532783369.130999390766524593571524415088; else ENcusCAeVUKpcwiLkAHOiEqapvIGVD=1678947803.163565346309205654667786472768;if (ENcusCAeVUKpcwiLkAHOiEqapvIGVD == ENcusCAeVUKpcwiLkAHOiEqapvIGVD ) ENcusCAeVUKpcwiLkAHOiEqapvIGVD=1313416462.532360416025838233087152148829; else ENcusCAeVUKpcwiLkAHOiEqapvIGVD=122779824.379888804535865206924078258023;if (ENcusCAeVUKpcwiLkAHOiEqapvIGVD == ENcusCAeVUKpcwiLkAHOiEqapvIGVD ) ENcusCAeVUKpcwiLkAHOiEqapvIGVD=2013729688.279876705144966239895391540274; else ENcusCAeVUKpcwiLkAHOiEqapvIGVD=1890530248.099237610960198112302923908933;if (ENcusCAeVUKpcwiLkAHOiEqapvIGVD == ENcusCAeVUKpcwiLkAHOiEqapvIGVD ) ENcusCAeVUKpcwiLkAHOiEqapvIGVD=1719288761.654216584973003530354907218743; else ENcusCAeVUKpcwiLkAHOiEqapvIGVD=2095986410.979742698374943002048879076811;double sTAAAWPeJrRJDqxAeTNEfixdFCzjhd=691496759.800026852193810253729125290954;if (sTAAAWPeJrRJDqxAeTNEfixdFCzjhd == sTAAAWPeJrRJDqxAeTNEfixdFCzjhd ) sTAAAWPeJrRJDqxAeTNEfixdFCzjhd=1028554300.217493232654681958863736279583; else sTAAAWPeJrRJDqxAeTNEfixdFCzjhd=1790446713.600998652812117554445220169222;if (sTAAAWPeJrRJDqxAeTNEfixdFCzjhd == sTAAAWPeJrRJDqxAeTNEfixdFCzjhd ) sTAAAWPeJrRJDqxAeTNEfixdFCzjhd=956584366.575906589331615262982515959183; else sTAAAWPeJrRJDqxAeTNEfixdFCzjhd=420346274.053144722208511472501845007116;if (sTAAAWPeJrRJDqxAeTNEfixdFCzjhd == sTAAAWPeJrRJDqxAeTNEfixdFCzjhd ) sTAAAWPeJrRJDqxAeTNEfixdFCzjhd=1185837816.750749868254178500459869332914; else sTAAAWPeJrRJDqxAeTNEfixdFCzjhd=1921554736.872656998371445777761776709142;if (sTAAAWPeJrRJDqxAeTNEfixdFCzjhd == sTAAAWPeJrRJDqxAeTNEfixdFCzjhd ) sTAAAWPeJrRJDqxAeTNEfixdFCzjhd=1481216407.895459253261329843566635320795; else sTAAAWPeJrRJDqxAeTNEfixdFCzjhd=440273100.895668710662450045707769737013;if (sTAAAWPeJrRJDqxAeTNEfixdFCzjhd == sTAAAWPeJrRJDqxAeTNEfixdFCzjhd ) sTAAAWPeJrRJDqxAeTNEfixdFCzjhd=1273100579.956858687224490428648610913704; else sTAAAWPeJrRJDqxAeTNEfixdFCzjhd=2046927696.170772670347958438931058644794;if (sTAAAWPeJrRJDqxAeTNEfixdFCzjhd == sTAAAWPeJrRJDqxAeTNEfixdFCzjhd ) sTAAAWPeJrRJDqxAeTNEfixdFCzjhd=1491893112.608426482986940276269697322214; else sTAAAWPeJrRJDqxAeTNEfixdFCzjhd=848445904.808775954306606612947964612220;long NhsIqqKgXPMLtgNzrHkCBOjguWSChS=1841408773;if (NhsIqqKgXPMLtgNzrHkCBOjguWSChS == NhsIqqKgXPMLtgNzrHkCBOjguWSChS- 0 ) NhsIqqKgXPMLtgNzrHkCBOjguWSChS=374118788; else NhsIqqKgXPMLtgNzrHkCBOjguWSChS=518031817;if (NhsIqqKgXPMLtgNzrHkCBOjguWSChS == NhsIqqKgXPMLtgNzrHkCBOjguWSChS- 0 ) NhsIqqKgXPMLtgNzrHkCBOjguWSChS=902286352; else NhsIqqKgXPMLtgNzrHkCBOjguWSChS=137357767;if (NhsIqqKgXPMLtgNzrHkCBOjguWSChS == NhsIqqKgXPMLtgNzrHkCBOjguWSChS- 1 ) NhsIqqKgXPMLtgNzrHkCBOjguWSChS=298474612; else NhsIqqKgXPMLtgNzrHkCBOjguWSChS=901925154;if (NhsIqqKgXPMLtgNzrHkCBOjguWSChS == NhsIqqKgXPMLtgNzrHkCBOjguWSChS- 1 ) NhsIqqKgXPMLtgNzrHkCBOjguWSChS=2124526903; else NhsIqqKgXPMLtgNzrHkCBOjguWSChS=586622502;if (NhsIqqKgXPMLtgNzrHkCBOjguWSChS == NhsIqqKgXPMLtgNzrHkCBOjguWSChS- 0 ) NhsIqqKgXPMLtgNzrHkCBOjguWSChS=2030524565; else NhsIqqKgXPMLtgNzrHkCBOjguWSChS=1891919929;if (NhsIqqKgXPMLtgNzrHkCBOjguWSChS == NhsIqqKgXPMLtgNzrHkCBOjguWSChS- 1 ) NhsIqqKgXPMLtgNzrHkCBOjguWSChS=75243012; else NhsIqqKgXPMLtgNzrHkCBOjguWSChS=1666410899;long PyCMbijysDmvipRrwUQCKrRWoYYVNy=432540299;if (PyCMbijysDmvipRrwUQCKrRWoYYVNy == PyCMbijysDmvipRrwUQCKrRWoYYVNy- 0 ) PyCMbijysDmvipRrwUQCKrRWoYYVNy=403063321; else PyCMbijysDmvipRrwUQCKrRWoYYVNy=1801114133;if (PyCMbijysDmvipRrwUQCKrRWoYYVNy == PyCMbijysDmvipRrwUQCKrRWoYYVNy- 1 ) PyCMbijysDmvipRrwUQCKrRWoYYVNy=537591887; else PyCMbijysDmvipRrwUQCKrRWoYYVNy=1212670728;if (PyCMbijysDmvipRrwUQCKrRWoYYVNy == PyCMbijysDmvipRrwUQCKrRWoYYVNy- 1 ) PyCMbijysDmvipRrwUQCKrRWoYYVNy=266371807; else PyCMbijysDmvipRrwUQCKrRWoYYVNy=1048185566;if (PyCMbijysDmvipRrwUQCKrRWoYYVNy == PyCMbijysDmvipRrwUQCKrRWoYYVNy- 0 ) PyCMbijysDmvipRrwUQCKrRWoYYVNy=880003663; else PyCMbijysDmvipRrwUQCKrRWoYYVNy=350079585;if (PyCMbijysDmvipRrwUQCKrRWoYYVNy == PyCMbijysDmvipRrwUQCKrRWoYYVNy- 1 ) PyCMbijysDmvipRrwUQCKrRWoYYVNy=32561801; else PyCMbijysDmvipRrwUQCKrRWoYYVNy=814027834;if (PyCMbijysDmvipRrwUQCKrRWoYYVNy == PyCMbijysDmvipRrwUQCKrRWoYYVNy- 0 ) PyCMbijysDmvipRrwUQCKrRWoYYVNy=498850858; else PyCMbijysDmvipRrwUQCKrRWoYYVNy=633339320;int wRSlCXNNneaSQhfQTmFfJWSwzpBXmK=1178110002;if (wRSlCXNNneaSQhfQTmFfJWSwzpBXmK == wRSlCXNNneaSQhfQTmFfJWSwzpBXmK- 1 ) wRSlCXNNneaSQhfQTmFfJWSwzpBXmK=1100190884; else wRSlCXNNneaSQhfQTmFfJWSwzpBXmK=2025363635;if (wRSlCXNNneaSQhfQTmFfJWSwzpBXmK == wRSlCXNNneaSQhfQTmFfJWSwzpBXmK- 0 ) wRSlCXNNneaSQhfQTmFfJWSwzpBXmK=1721867622; else wRSlCXNNneaSQhfQTmFfJWSwzpBXmK=1936505799;if (wRSlCXNNneaSQhfQTmFfJWSwzpBXmK == wRSlCXNNneaSQhfQTmFfJWSwzpBXmK- 1 ) wRSlCXNNneaSQhfQTmFfJWSwzpBXmK=695283813; else wRSlCXNNneaSQhfQTmFfJWSwzpBXmK=484928716;if (wRSlCXNNneaSQhfQTmFfJWSwzpBXmK == wRSlCXNNneaSQhfQTmFfJWSwzpBXmK- 1 ) wRSlCXNNneaSQhfQTmFfJWSwzpBXmK=393022147; else wRSlCXNNneaSQhfQTmFfJWSwzpBXmK=126629146;if (wRSlCXNNneaSQhfQTmFfJWSwzpBXmK == wRSlCXNNneaSQhfQTmFfJWSwzpBXmK- 0 ) wRSlCXNNneaSQhfQTmFfJWSwzpBXmK=1813403457; else wRSlCXNNneaSQhfQTmFfJWSwzpBXmK=369961864;if (wRSlCXNNneaSQhfQTmFfJWSwzpBXmK == wRSlCXNNneaSQhfQTmFfJWSwzpBXmK- 1 ) wRSlCXNNneaSQhfQTmFfJWSwzpBXmK=537305356; else wRSlCXNNneaSQhfQTmFfJWSwzpBXmK=1441401534;long EKWSvuNPMbXdkawpMjlCiIjSMEUboP=750685657;if (EKWSvuNPMbXdkawpMjlCiIjSMEUboP == EKWSvuNPMbXdkawpMjlCiIjSMEUboP- 0 ) EKWSvuNPMbXdkawpMjlCiIjSMEUboP=1529003590; else EKWSvuNPMbXdkawpMjlCiIjSMEUboP=1059352813;if (EKWSvuNPMbXdkawpMjlCiIjSMEUboP == EKWSvuNPMbXdkawpMjlCiIjSMEUboP- 0 ) EKWSvuNPMbXdkawpMjlCiIjSMEUboP=445382583; else EKWSvuNPMbXdkawpMjlCiIjSMEUboP=1604478142;if (EKWSvuNPMbXdkawpMjlCiIjSMEUboP == EKWSvuNPMbXdkawpMjlCiIjSMEUboP- 0 ) EKWSvuNPMbXdkawpMjlCiIjSMEUboP=298027799; else EKWSvuNPMbXdkawpMjlCiIjSMEUboP=1725152283;if (EKWSvuNPMbXdkawpMjlCiIjSMEUboP == EKWSvuNPMbXdkawpMjlCiIjSMEUboP- 0 ) EKWSvuNPMbXdkawpMjlCiIjSMEUboP=1575036966; else EKWSvuNPMbXdkawpMjlCiIjSMEUboP=1473416869;if (EKWSvuNPMbXdkawpMjlCiIjSMEUboP == EKWSvuNPMbXdkawpMjlCiIjSMEUboP- 0 ) EKWSvuNPMbXdkawpMjlCiIjSMEUboP=1785511801; else EKWSvuNPMbXdkawpMjlCiIjSMEUboP=552206767;if (EKWSvuNPMbXdkawpMjlCiIjSMEUboP == EKWSvuNPMbXdkawpMjlCiIjSMEUboP- 1 ) EKWSvuNPMbXdkawpMjlCiIjSMEUboP=104795966; else EKWSvuNPMbXdkawpMjlCiIjSMEUboP=1631930425;long jYUFbrEjodVlWstkrKlIpEggKApcKN=1565554007;if (jYUFbrEjodVlWstkrKlIpEggKApcKN == jYUFbrEjodVlWstkrKlIpEggKApcKN- 0 ) jYUFbrEjodVlWstkrKlIpEggKApcKN=879765086; else jYUFbrEjodVlWstkrKlIpEggKApcKN=1044616657;if (jYUFbrEjodVlWstkrKlIpEggKApcKN == jYUFbrEjodVlWstkrKlIpEggKApcKN- 1 ) jYUFbrEjodVlWstkrKlIpEggKApcKN=566220015; else jYUFbrEjodVlWstkrKlIpEggKApcKN=1738990327;if (jYUFbrEjodVlWstkrKlIpEggKApcKN == jYUFbrEjodVlWstkrKlIpEggKApcKN- 1 ) jYUFbrEjodVlWstkrKlIpEggKApcKN=948030947; else jYUFbrEjodVlWstkrKlIpEggKApcKN=728901954;if (jYUFbrEjodVlWstkrKlIpEggKApcKN == jYUFbrEjodVlWstkrKlIpEggKApcKN- 0 ) jYUFbrEjodVlWstkrKlIpEggKApcKN=1884740133; else jYUFbrEjodVlWstkrKlIpEggKApcKN=2065618252;if (jYUFbrEjodVlWstkrKlIpEggKApcKN == jYUFbrEjodVlWstkrKlIpEggKApcKN- 1 ) jYUFbrEjodVlWstkrKlIpEggKApcKN=1408091986; else jYUFbrEjodVlWstkrKlIpEggKApcKN=410962741;if (jYUFbrEjodVlWstkrKlIpEggKApcKN == jYUFbrEjodVlWstkrKlIpEggKApcKN- 0 ) jYUFbrEjodVlWstkrKlIpEggKApcKN=1546750918; else jYUFbrEjodVlWstkrKlIpEggKApcKN=2043893590;double yMFPKbBmKqdiWoeZbcZtIebKYFuDWV=217756293.796123923445830163660040989788;if (yMFPKbBmKqdiWoeZbcZtIebKYFuDWV == yMFPKbBmKqdiWoeZbcZtIebKYFuDWV ) yMFPKbBmKqdiWoeZbcZtIebKYFuDWV=1990625049.177548404841320149316406975735; else yMFPKbBmKqdiWoeZbcZtIebKYFuDWV=1895648848.525647939287082072184148278278;if (yMFPKbBmKqdiWoeZbcZtIebKYFuDWV == yMFPKbBmKqdiWoeZbcZtIebKYFuDWV ) yMFPKbBmKqdiWoeZbcZtIebKYFuDWV=1251819377.487942789332023043401365248747; else yMFPKbBmKqdiWoeZbcZtIebKYFuDWV=1275185525.076291014130486213534598712380;if (yMFPKbBmKqdiWoeZbcZtIebKYFuDWV == yMFPKbBmKqdiWoeZbcZtIebKYFuDWV ) yMFPKbBmKqdiWoeZbcZtIebKYFuDWV=19587975.414346694303938209903622469958; else yMFPKbBmKqdiWoeZbcZtIebKYFuDWV=1555160348.074483615173933166161348789209;if (yMFPKbBmKqdiWoeZbcZtIebKYFuDWV == yMFPKbBmKqdiWoeZbcZtIebKYFuDWV ) yMFPKbBmKqdiWoeZbcZtIebKYFuDWV=880082203.874634966569494170030390774770; else yMFPKbBmKqdiWoeZbcZtIebKYFuDWV=1522522248.552740940021160295572937958597;if (yMFPKbBmKqdiWoeZbcZtIebKYFuDWV == yMFPKbBmKqdiWoeZbcZtIebKYFuDWV ) yMFPKbBmKqdiWoeZbcZtIebKYFuDWV=450326669.039712886434002934158931862589; else yMFPKbBmKqdiWoeZbcZtIebKYFuDWV=248450753.255409245952915903602554911996;if (yMFPKbBmKqdiWoeZbcZtIebKYFuDWV == yMFPKbBmKqdiWoeZbcZtIebKYFuDWV ) yMFPKbBmKqdiWoeZbcZtIebKYFuDWV=150993998.041828960389931712706908781150; else yMFPKbBmKqdiWoeZbcZtIebKYFuDWV=143077046.945091129704333373331920015315;double rGCTRWsTCbsOAAJfhaTFrnJvBJaDmh=322215874.306761577758566993975453761638;if (rGCTRWsTCbsOAAJfhaTFrnJvBJaDmh == rGCTRWsTCbsOAAJfhaTFrnJvBJaDmh ) rGCTRWsTCbsOAAJfhaTFrnJvBJaDmh=2117877700.424809162111997355815529805400; else rGCTRWsTCbsOAAJfhaTFrnJvBJaDmh=1119234996.004034587883297557642693390600;if (rGCTRWsTCbsOAAJfhaTFrnJvBJaDmh == rGCTRWsTCbsOAAJfhaTFrnJvBJaDmh ) rGCTRWsTCbsOAAJfhaTFrnJvBJaDmh=561636448.486212706398913244346997951275; else rGCTRWsTCbsOAAJfhaTFrnJvBJaDmh=801695067.598751818685136488016647569047;if (rGCTRWsTCbsOAAJfhaTFrnJvBJaDmh == rGCTRWsTCbsOAAJfhaTFrnJvBJaDmh ) rGCTRWsTCbsOAAJfhaTFrnJvBJaDmh=1396069140.163190137353686186167436006139; else rGCTRWsTCbsOAAJfhaTFrnJvBJaDmh=1379379752.288253305646287307148308083729;if (rGCTRWsTCbsOAAJfhaTFrnJvBJaDmh == rGCTRWsTCbsOAAJfhaTFrnJvBJaDmh ) rGCTRWsTCbsOAAJfhaTFrnJvBJaDmh=1950138357.500817653498252226071000209028; else rGCTRWsTCbsOAAJfhaTFrnJvBJaDmh=800101636.069624657085486025696885960162;if (rGCTRWsTCbsOAAJfhaTFrnJvBJaDmh == rGCTRWsTCbsOAAJfhaTFrnJvBJaDmh ) rGCTRWsTCbsOAAJfhaTFrnJvBJaDmh=1322096327.974553064968900663529444329486; else rGCTRWsTCbsOAAJfhaTFrnJvBJaDmh=527247978.469622204306283908631874453133;if (rGCTRWsTCbsOAAJfhaTFrnJvBJaDmh == rGCTRWsTCbsOAAJfhaTFrnJvBJaDmh ) rGCTRWsTCbsOAAJfhaTFrnJvBJaDmh=196386156.579851939956923436142185669537; else rGCTRWsTCbsOAAJfhaTFrnJvBJaDmh=2034548705.918183753217979505203709140529;float TCNzStoXyjpPvQbLDfFiWQNTJIqzRS=199426025.109291337423474403735353243358f;if (TCNzStoXyjpPvQbLDfFiWQNTJIqzRS - TCNzStoXyjpPvQbLDfFiWQNTJIqzRS> 0.00000001 ) TCNzStoXyjpPvQbLDfFiWQNTJIqzRS=2048438906.959422655820725199904504896616f; else TCNzStoXyjpPvQbLDfFiWQNTJIqzRS=2034523320.918068505396901574915445456564f;if (TCNzStoXyjpPvQbLDfFiWQNTJIqzRS - TCNzStoXyjpPvQbLDfFiWQNTJIqzRS> 0.00000001 ) TCNzStoXyjpPvQbLDfFiWQNTJIqzRS=1788789023.715092277779462971332270157462f; else TCNzStoXyjpPvQbLDfFiWQNTJIqzRS=654909095.204278191040735709458624355595f;if (TCNzStoXyjpPvQbLDfFiWQNTJIqzRS - TCNzStoXyjpPvQbLDfFiWQNTJIqzRS> 0.00000001 ) TCNzStoXyjpPvQbLDfFiWQNTJIqzRS=239368206.591657530488040265655902478193f; else TCNzStoXyjpPvQbLDfFiWQNTJIqzRS=1271865933.471666928827908169410095069039f;if (TCNzStoXyjpPvQbLDfFiWQNTJIqzRS - TCNzStoXyjpPvQbLDfFiWQNTJIqzRS> 0.00000001 ) TCNzStoXyjpPvQbLDfFiWQNTJIqzRS=1545966032.955812848444894811432079348771f; else TCNzStoXyjpPvQbLDfFiWQNTJIqzRS=1472088385.890997310742118631125970808757f;if (TCNzStoXyjpPvQbLDfFiWQNTJIqzRS - TCNzStoXyjpPvQbLDfFiWQNTJIqzRS> 0.00000001 ) TCNzStoXyjpPvQbLDfFiWQNTJIqzRS=164534454.428875591760180097182765603029f; else TCNzStoXyjpPvQbLDfFiWQNTJIqzRS=1191181414.076433544324157589191168586428f;if (TCNzStoXyjpPvQbLDfFiWQNTJIqzRS - TCNzStoXyjpPvQbLDfFiWQNTJIqzRS> 0.00000001 ) TCNzStoXyjpPvQbLDfFiWQNTJIqzRS=1288663658.358270486766486972813404901995f; else TCNzStoXyjpPvQbLDfFiWQNTJIqzRS=699580194.712718608810343688271562595538f;float cuEyXWikuMkTCAMEcCRLNlMUMbAIML=499516617.119050948299242389463125611885f;if (cuEyXWikuMkTCAMEcCRLNlMUMbAIML - cuEyXWikuMkTCAMEcCRLNlMUMbAIML> 0.00000001 ) cuEyXWikuMkTCAMEcCRLNlMUMbAIML=687407891.390430045259816755649009319555f; else cuEyXWikuMkTCAMEcCRLNlMUMbAIML=1224535044.026400092345531442906635630212f;if (cuEyXWikuMkTCAMEcCRLNlMUMbAIML - cuEyXWikuMkTCAMEcCRLNlMUMbAIML> 0.00000001 ) cuEyXWikuMkTCAMEcCRLNlMUMbAIML=2073499537.260124342787430558898765940297f; else cuEyXWikuMkTCAMEcCRLNlMUMbAIML=1539128861.199028651821123030519343550529f;if (cuEyXWikuMkTCAMEcCRLNlMUMbAIML - cuEyXWikuMkTCAMEcCRLNlMUMbAIML> 0.00000001 ) cuEyXWikuMkTCAMEcCRLNlMUMbAIML=1024449328.415579842200718874202189995767f; else cuEyXWikuMkTCAMEcCRLNlMUMbAIML=1997040088.914341666429831963588632844779f;if (cuEyXWikuMkTCAMEcCRLNlMUMbAIML - cuEyXWikuMkTCAMEcCRLNlMUMbAIML> 0.00000001 ) cuEyXWikuMkTCAMEcCRLNlMUMbAIML=969170800.529640758721999335803328949250f; else cuEyXWikuMkTCAMEcCRLNlMUMbAIML=611245297.790801129232696775461997630171f;if (cuEyXWikuMkTCAMEcCRLNlMUMbAIML - cuEyXWikuMkTCAMEcCRLNlMUMbAIML> 0.00000001 ) cuEyXWikuMkTCAMEcCRLNlMUMbAIML=2092994138.250903220821668283327490991428f; else cuEyXWikuMkTCAMEcCRLNlMUMbAIML=451717741.106918280366664033056895586102f;if (cuEyXWikuMkTCAMEcCRLNlMUMbAIML - cuEyXWikuMkTCAMEcCRLNlMUMbAIML> 0.00000001 ) cuEyXWikuMkTCAMEcCRLNlMUMbAIML=2100572845.380282686001407194796857342154f; else cuEyXWikuMkTCAMEcCRLNlMUMbAIML=1639823644.328808648256806649949341664405f;int ZPIkoMezrYdcZYzwWGkyQLDbynlKUV=919183293;if (ZPIkoMezrYdcZYzwWGkyQLDbynlKUV == ZPIkoMezrYdcZYzwWGkyQLDbynlKUV- 1 ) ZPIkoMezrYdcZYzwWGkyQLDbynlKUV=1433397741; else ZPIkoMezrYdcZYzwWGkyQLDbynlKUV=2099051620;if (ZPIkoMezrYdcZYzwWGkyQLDbynlKUV == ZPIkoMezrYdcZYzwWGkyQLDbynlKUV- 0 ) ZPIkoMezrYdcZYzwWGkyQLDbynlKUV=242121787; else ZPIkoMezrYdcZYzwWGkyQLDbynlKUV=493093077;if (ZPIkoMezrYdcZYzwWGkyQLDbynlKUV == ZPIkoMezrYdcZYzwWGkyQLDbynlKUV- 0 ) ZPIkoMezrYdcZYzwWGkyQLDbynlKUV=1267269993; else ZPIkoMezrYdcZYzwWGkyQLDbynlKUV=1216129402;if (ZPIkoMezrYdcZYzwWGkyQLDbynlKUV == ZPIkoMezrYdcZYzwWGkyQLDbynlKUV- 0 ) ZPIkoMezrYdcZYzwWGkyQLDbynlKUV=82847668; else ZPIkoMezrYdcZYzwWGkyQLDbynlKUV=846011767;if (ZPIkoMezrYdcZYzwWGkyQLDbynlKUV == ZPIkoMezrYdcZYzwWGkyQLDbynlKUV- 0 ) ZPIkoMezrYdcZYzwWGkyQLDbynlKUV=1720752611; else ZPIkoMezrYdcZYzwWGkyQLDbynlKUV=1237031710;if (ZPIkoMezrYdcZYzwWGkyQLDbynlKUV == ZPIkoMezrYdcZYzwWGkyQLDbynlKUV- 0 ) ZPIkoMezrYdcZYzwWGkyQLDbynlKUV=637160613; else ZPIkoMezrYdcZYzwWGkyQLDbynlKUV=204887726;double NZRlGhyNizGsvMIgFtPGVpnyGTNvus=1727916887.750815710890163142811478558953;if (NZRlGhyNizGsvMIgFtPGVpnyGTNvus == NZRlGhyNizGsvMIgFtPGVpnyGTNvus ) NZRlGhyNizGsvMIgFtPGVpnyGTNvus=989246123.680408634671796116665077048337; else NZRlGhyNizGsvMIgFtPGVpnyGTNvus=1844353186.896721176561924179354230971357;if (NZRlGhyNizGsvMIgFtPGVpnyGTNvus == NZRlGhyNizGsvMIgFtPGVpnyGTNvus ) NZRlGhyNizGsvMIgFtPGVpnyGTNvus=1347140814.040899517605010426407009366795; else NZRlGhyNizGsvMIgFtPGVpnyGTNvus=1101349968.093063213677285023056606014153;if (NZRlGhyNizGsvMIgFtPGVpnyGTNvus == NZRlGhyNizGsvMIgFtPGVpnyGTNvus ) NZRlGhyNizGsvMIgFtPGVpnyGTNvus=1752611133.483362216821384183534374959815; else NZRlGhyNizGsvMIgFtPGVpnyGTNvus=1570891445.016039402382823632203172011942;if (NZRlGhyNizGsvMIgFtPGVpnyGTNvus == NZRlGhyNizGsvMIgFtPGVpnyGTNvus ) NZRlGhyNizGsvMIgFtPGVpnyGTNvus=1322460344.351192536579226265142861395412; else NZRlGhyNizGsvMIgFtPGVpnyGTNvus=51398818.866014274210674505785691060080;if (NZRlGhyNizGsvMIgFtPGVpnyGTNvus == NZRlGhyNizGsvMIgFtPGVpnyGTNvus ) NZRlGhyNizGsvMIgFtPGVpnyGTNvus=1065352520.723582322200294339890074727605; else NZRlGhyNizGsvMIgFtPGVpnyGTNvus=1177543726.539469820228458650784120175087;if (NZRlGhyNizGsvMIgFtPGVpnyGTNvus == NZRlGhyNizGsvMIgFtPGVpnyGTNvus ) NZRlGhyNizGsvMIgFtPGVpnyGTNvus=709398604.572788184562067372617793241040; else NZRlGhyNizGsvMIgFtPGVpnyGTNvus=1935134112.584385678068732327390347099055;long GTTAnnirMiAXddxyhEyKVhbyHWjQvh=1319000956;if (GTTAnnirMiAXddxyhEyKVhbyHWjQvh == GTTAnnirMiAXddxyhEyKVhbyHWjQvh- 0 ) GTTAnnirMiAXddxyhEyKVhbyHWjQvh=1994544178; else GTTAnnirMiAXddxyhEyKVhbyHWjQvh=369480365;if (GTTAnnirMiAXddxyhEyKVhbyHWjQvh == GTTAnnirMiAXddxyhEyKVhbyHWjQvh- 1 ) GTTAnnirMiAXddxyhEyKVhbyHWjQvh=1413666100; else GTTAnnirMiAXddxyhEyKVhbyHWjQvh=626339983;if (GTTAnnirMiAXddxyhEyKVhbyHWjQvh == GTTAnnirMiAXddxyhEyKVhbyHWjQvh- 0 ) GTTAnnirMiAXddxyhEyKVhbyHWjQvh=1499541300; else GTTAnnirMiAXddxyhEyKVhbyHWjQvh=1192392425;if (GTTAnnirMiAXddxyhEyKVhbyHWjQvh == GTTAnnirMiAXddxyhEyKVhbyHWjQvh- 1 ) GTTAnnirMiAXddxyhEyKVhbyHWjQvh=1379730271; else GTTAnnirMiAXddxyhEyKVhbyHWjQvh=1618762136;if (GTTAnnirMiAXddxyhEyKVhbyHWjQvh == GTTAnnirMiAXddxyhEyKVhbyHWjQvh- 1 ) GTTAnnirMiAXddxyhEyKVhbyHWjQvh=1141687376; else GTTAnnirMiAXddxyhEyKVhbyHWjQvh=1227487770;if (GTTAnnirMiAXddxyhEyKVhbyHWjQvh == GTTAnnirMiAXddxyhEyKVhbyHWjQvh- 0 ) GTTAnnirMiAXddxyhEyKVhbyHWjQvh=1965859897; else GTTAnnirMiAXddxyhEyKVhbyHWjQvh=1934901265;double nDwuzyzyORtOiJJdWgWCsvWleDbbTd=842435975.132258045409983400340860378612;if (nDwuzyzyORtOiJJdWgWCsvWleDbbTd == nDwuzyzyORtOiJJdWgWCsvWleDbbTd ) nDwuzyzyORtOiJJdWgWCsvWleDbbTd=229524061.126191836495739394807200337309; else nDwuzyzyORtOiJJdWgWCsvWleDbbTd=372655958.154338490378405624208481603696;if (nDwuzyzyORtOiJJdWgWCsvWleDbbTd == nDwuzyzyORtOiJJdWgWCsvWleDbbTd ) nDwuzyzyORtOiJJdWgWCsvWleDbbTd=650577521.453845241290564091582420187384; else nDwuzyzyORtOiJJdWgWCsvWleDbbTd=1661904653.581492566148301893862322514487;if (nDwuzyzyORtOiJJdWgWCsvWleDbbTd == nDwuzyzyORtOiJJdWgWCsvWleDbbTd ) nDwuzyzyORtOiJJdWgWCsvWleDbbTd=1793359978.897772027263554412939552337999; else nDwuzyzyORtOiJJdWgWCsvWleDbbTd=242770915.829045264088661998820356935654;if (nDwuzyzyORtOiJJdWgWCsvWleDbbTd == nDwuzyzyORtOiJJdWgWCsvWleDbbTd ) nDwuzyzyORtOiJJdWgWCsvWleDbbTd=1314055807.523955551926805199145017067321; else nDwuzyzyORtOiJJdWgWCsvWleDbbTd=2009989943.172057582754700556185652061609;if (nDwuzyzyORtOiJJdWgWCsvWleDbbTd == nDwuzyzyORtOiJJdWgWCsvWleDbbTd ) nDwuzyzyORtOiJJdWgWCsvWleDbbTd=776591276.824186495862840142862234762100; else nDwuzyzyORtOiJJdWgWCsvWleDbbTd=190722969.798303062417143391203095860153;if (nDwuzyzyORtOiJJdWgWCsvWleDbbTd == nDwuzyzyORtOiJJdWgWCsvWleDbbTd ) nDwuzyzyORtOiJJdWgWCsvWleDbbTd=1575224204.449627455728045629147291781464; else nDwuzyzyORtOiJJdWgWCsvWleDbbTd=89726267.492907862604658605025102636895;float CtERLeDTsnekZVOcSQIofUTxNvAPmE=1814610223.410062516300976884646870942008f;if (CtERLeDTsnekZVOcSQIofUTxNvAPmE - CtERLeDTsnekZVOcSQIofUTxNvAPmE> 0.00000001 ) CtERLeDTsnekZVOcSQIofUTxNvAPmE=1123717517.059999148812802977073450174022f; else CtERLeDTsnekZVOcSQIofUTxNvAPmE=1465643304.162836118785784408918603571093f;if (CtERLeDTsnekZVOcSQIofUTxNvAPmE - CtERLeDTsnekZVOcSQIofUTxNvAPmE> 0.00000001 ) CtERLeDTsnekZVOcSQIofUTxNvAPmE=357827195.522786118786930887777721466936f; else CtERLeDTsnekZVOcSQIofUTxNvAPmE=1868719376.613527300840033278470442791233f;if (CtERLeDTsnekZVOcSQIofUTxNvAPmE - CtERLeDTsnekZVOcSQIofUTxNvAPmE> 0.00000001 ) CtERLeDTsnekZVOcSQIofUTxNvAPmE=1984375710.028227451468794879829966728194f; else CtERLeDTsnekZVOcSQIofUTxNvAPmE=1743337344.727892382693712276902241417260f;if (CtERLeDTsnekZVOcSQIofUTxNvAPmE - CtERLeDTsnekZVOcSQIofUTxNvAPmE> 0.00000001 ) CtERLeDTsnekZVOcSQIofUTxNvAPmE=228375587.046784200250824725465220359648f; else CtERLeDTsnekZVOcSQIofUTxNvAPmE=1944337345.513275704918675342889281592653f;if (CtERLeDTsnekZVOcSQIofUTxNvAPmE - CtERLeDTsnekZVOcSQIofUTxNvAPmE> 0.00000001 ) CtERLeDTsnekZVOcSQIofUTxNvAPmE=464622915.666285766903265865602797033389f; else CtERLeDTsnekZVOcSQIofUTxNvAPmE=1875862325.425235250068698713683816198035f;if (CtERLeDTsnekZVOcSQIofUTxNvAPmE - CtERLeDTsnekZVOcSQIofUTxNvAPmE> 0.00000001 ) CtERLeDTsnekZVOcSQIofUTxNvAPmE=2107146239.349869814162575511933180671014f; else CtERLeDTsnekZVOcSQIofUTxNvAPmE=525123363.832682993000052234179821237920f;double nxwORxvvMcKmwQPFMqXUkuXbCGJVZb=301069446.140927613378957937445939895735;if (nxwORxvvMcKmwQPFMqXUkuXbCGJVZb == nxwORxvvMcKmwQPFMqXUkuXbCGJVZb ) nxwORxvvMcKmwQPFMqXUkuXbCGJVZb=657014523.092966042893394158721069722213; else nxwORxvvMcKmwQPFMqXUkuXbCGJVZb=2014446276.209938619200566274684801414424;if (nxwORxvvMcKmwQPFMqXUkuXbCGJVZb == nxwORxvvMcKmwQPFMqXUkuXbCGJVZb ) nxwORxvvMcKmwQPFMqXUkuXbCGJVZb=1705610051.593070546655953151366864940953; else nxwORxvvMcKmwQPFMqXUkuXbCGJVZb=946120777.539581869783914995847417983013;if (nxwORxvvMcKmwQPFMqXUkuXbCGJVZb == nxwORxvvMcKmwQPFMqXUkuXbCGJVZb ) nxwORxvvMcKmwQPFMqXUkuXbCGJVZb=1403048514.060412748349279406116522723271; else nxwORxvvMcKmwQPFMqXUkuXbCGJVZb=598893187.896672450225860824968891865880;if (nxwORxvvMcKmwQPFMqXUkuXbCGJVZb == nxwORxvvMcKmwQPFMqXUkuXbCGJVZb ) nxwORxvvMcKmwQPFMqXUkuXbCGJVZb=1673124625.068856471705799186390042310681; else nxwORxvvMcKmwQPFMqXUkuXbCGJVZb=1751773263.878037801556426421289960002664;if (nxwORxvvMcKmwQPFMqXUkuXbCGJVZb == nxwORxvvMcKmwQPFMqXUkuXbCGJVZb ) nxwORxvvMcKmwQPFMqXUkuXbCGJVZb=1202908205.689004777249584572790041391903; else nxwORxvvMcKmwQPFMqXUkuXbCGJVZb=1551159804.624897743275414158664107575872;if (nxwORxvvMcKmwQPFMqXUkuXbCGJVZb == nxwORxvvMcKmwQPFMqXUkuXbCGJVZb ) nxwORxvvMcKmwQPFMqXUkuXbCGJVZb=1997232354.397572580893053563735435953832; else nxwORxvvMcKmwQPFMqXUkuXbCGJVZb=1308818331.297208995367997226501215706272;long jYoazcoZXdkrYhocXpiUkQPDAmMXAM=846490173;if (jYoazcoZXdkrYhocXpiUkQPDAmMXAM == jYoazcoZXdkrYhocXpiUkQPDAmMXAM- 0 ) jYoazcoZXdkrYhocXpiUkQPDAmMXAM=1700252738; else jYoazcoZXdkrYhocXpiUkQPDAmMXAM=151249674;if (jYoazcoZXdkrYhocXpiUkQPDAmMXAM == jYoazcoZXdkrYhocXpiUkQPDAmMXAM- 0 ) jYoazcoZXdkrYhocXpiUkQPDAmMXAM=811183748; else jYoazcoZXdkrYhocXpiUkQPDAmMXAM=2041719818;if (jYoazcoZXdkrYhocXpiUkQPDAmMXAM == jYoazcoZXdkrYhocXpiUkQPDAmMXAM- 1 ) jYoazcoZXdkrYhocXpiUkQPDAmMXAM=406455879; else jYoazcoZXdkrYhocXpiUkQPDAmMXAM=799293474;if (jYoazcoZXdkrYhocXpiUkQPDAmMXAM == jYoazcoZXdkrYhocXpiUkQPDAmMXAM- 0 ) jYoazcoZXdkrYhocXpiUkQPDAmMXAM=1005543912; else jYoazcoZXdkrYhocXpiUkQPDAmMXAM=633670364;if (jYoazcoZXdkrYhocXpiUkQPDAmMXAM == jYoazcoZXdkrYhocXpiUkQPDAmMXAM- 0 ) jYoazcoZXdkrYhocXpiUkQPDAmMXAM=144280371; else jYoazcoZXdkrYhocXpiUkQPDAmMXAM=853723823;if (jYoazcoZXdkrYhocXpiUkQPDAmMXAM == jYoazcoZXdkrYhocXpiUkQPDAmMXAM- 1 ) jYoazcoZXdkrYhocXpiUkQPDAmMXAM=1197281738; else jYoazcoZXdkrYhocXpiUkQPDAmMXAM=2064981300;double pvwwAmlouwnhMulVPAnJcqVEBywvYk=1389032197.038144791867058474188539819856;if (pvwwAmlouwnhMulVPAnJcqVEBywvYk == pvwwAmlouwnhMulVPAnJcqVEBywvYk ) pvwwAmlouwnhMulVPAnJcqVEBywvYk=1499203626.406254310660093142583439660034; else pvwwAmlouwnhMulVPAnJcqVEBywvYk=1274154758.505647181426552855687747318869;if (pvwwAmlouwnhMulVPAnJcqVEBywvYk == pvwwAmlouwnhMulVPAnJcqVEBywvYk ) pvwwAmlouwnhMulVPAnJcqVEBywvYk=1580195391.438716655706217808791655797834; else pvwwAmlouwnhMulVPAnJcqVEBywvYk=639768169.943219885631009263061028989413;if (pvwwAmlouwnhMulVPAnJcqVEBywvYk == pvwwAmlouwnhMulVPAnJcqVEBywvYk ) pvwwAmlouwnhMulVPAnJcqVEBywvYk=737070799.082287411803317175764488492543; else pvwwAmlouwnhMulVPAnJcqVEBywvYk=728597902.158375083286336971191575070599;if (pvwwAmlouwnhMulVPAnJcqVEBywvYk == pvwwAmlouwnhMulVPAnJcqVEBywvYk ) pvwwAmlouwnhMulVPAnJcqVEBywvYk=411561709.597886515319369826252146279341; else pvwwAmlouwnhMulVPAnJcqVEBywvYk=524824330.560489955147798322725534746272;if (pvwwAmlouwnhMulVPAnJcqVEBywvYk == pvwwAmlouwnhMulVPAnJcqVEBywvYk ) pvwwAmlouwnhMulVPAnJcqVEBywvYk=1940002326.838586999008122655905763613774; else pvwwAmlouwnhMulVPAnJcqVEBywvYk=753537579.478442931243372842320147717698;if (pvwwAmlouwnhMulVPAnJcqVEBywvYk == pvwwAmlouwnhMulVPAnJcqVEBywvYk ) pvwwAmlouwnhMulVPAnJcqVEBywvYk=665811171.433573088172477259187156434173; else pvwwAmlouwnhMulVPAnJcqVEBywvYk=433215906.545482191987425977157851248714;int sBbcPkEvZHetKzAiNHdgjBJhlwQJWS=1322102558;if (sBbcPkEvZHetKzAiNHdgjBJhlwQJWS == sBbcPkEvZHetKzAiNHdgjBJhlwQJWS- 1 ) sBbcPkEvZHetKzAiNHdgjBJhlwQJWS=763830987; else sBbcPkEvZHetKzAiNHdgjBJhlwQJWS=1260656066;if (sBbcPkEvZHetKzAiNHdgjBJhlwQJWS == sBbcPkEvZHetKzAiNHdgjBJhlwQJWS- 1 ) sBbcPkEvZHetKzAiNHdgjBJhlwQJWS=972354272; else sBbcPkEvZHetKzAiNHdgjBJhlwQJWS=1601265521;if (sBbcPkEvZHetKzAiNHdgjBJhlwQJWS == sBbcPkEvZHetKzAiNHdgjBJhlwQJWS- 0 ) sBbcPkEvZHetKzAiNHdgjBJhlwQJWS=723858978; else sBbcPkEvZHetKzAiNHdgjBJhlwQJWS=944766819;if (sBbcPkEvZHetKzAiNHdgjBJhlwQJWS == sBbcPkEvZHetKzAiNHdgjBJhlwQJWS- 1 ) sBbcPkEvZHetKzAiNHdgjBJhlwQJWS=2042097214; else sBbcPkEvZHetKzAiNHdgjBJhlwQJWS=2005204696;if (sBbcPkEvZHetKzAiNHdgjBJhlwQJWS == sBbcPkEvZHetKzAiNHdgjBJhlwQJWS- 1 ) sBbcPkEvZHetKzAiNHdgjBJhlwQJWS=1008902364; else sBbcPkEvZHetKzAiNHdgjBJhlwQJWS=1071939687;if (sBbcPkEvZHetKzAiNHdgjBJhlwQJWS == sBbcPkEvZHetKzAiNHdgjBJhlwQJWS- 1 ) sBbcPkEvZHetKzAiNHdgjBJhlwQJWS=801840406; else sBbcPkEvZHetKzAiNHdgjBJhlwQJWS=549324691;long zpyDdVmpPJDHAMyGxJtXUQHxhzKzYM=413975761;if (zpyDdVmpPJDHAMyGxJtXUQHxhzKzYM == zpyDdVmpPJDHAMyGxJtXUQHxhzKzYM- 1 ) zpyDdVmpPJDHAMyGxJtXUQHxhzKzYM=1293685056; else zpyDdVmpPJDHAMyGxJtXUQHxhzKzYM=608200157;if (zpyDdVmpPJDHAMyGxJtXUQHxhzKzYM == zpyDdVmpPJDHAMyGxJtXUQHxhzKzYM- 0 ) zpyDdVmpPJDHAMyGxJtXUQHxhzKzYM=1957098352; else zpyDdVmpPJDHAMyGxJtXUQHxhzKzYM=490497110;if (zpyDdVmpPJDHAMyGxJtXUQHxhzKzYM == zpyDdVmpPJDHAMyGxJtXUQHxhzKzYM- 0 ) zpyDdVmpPJDHAMyGxJtXUQHxhzKzYM=1857292260; else zpyDdVmpPJDHAMyGxJtXUQHxhzKzYM=472263169;if (zpyDdVmpPJDHAMyGxJtXUQHxhzKzYM == zpyDdVmpPJDHAMyGxJtXUQHxhzKzYM- 0 ) zpyDdVmpPJDHAMyGxJtXUQHxhzKzYM=109419374; else zpyDdVmpPJDHAMyGxJtXUQHxhzKzYM=971654836;if (zpyDdVmpPJDHAMyGxJtXUQHxhzKzYM == zpyDdVmpPJDHAMyGxJtXUQHxhzKzYM- 0 ) zpyDdVmpPJDHAMyGxJtXUQHxhzKzYM=1887171612; else zpyDdVmpPJDHAMyGxJtXUQHxhzKzYM=286359418;if (zpyDdVmpPJDHAMyGxJtXUQHxhzKzYM == zpyDdVmpPJDHAMyGxJtXUQHxhzKzYM- 1 ) zpyDdVmpPJDHAMyGxJtXUQHxhzKzYM=101717492; else zpyDdVmpPJDHAMyGxJtXUQHxhzKzYM=1800401947;long CGeFSTmZnTcAvnVASJdAgVEPyVurWM=1224723115;if (CGeFSTmZnTcAvnVASJdAgVEPyVurWM == CGeFSTmZnTcAvnVASJdAgVEPyVurWM- 0 ) CGeFSTmZnTcAvnVASJdAgVEPyVurWM=674038346; else CGeFSTmZnTcAvnVASJdAgVEPyVurWM=2022662827;if (CGeFSTmZnTcAvnVASJdAgVEPyVurWM == CGeFSTmZnTcAvnVASJdAgVEPyVurWM- 0 ) CGeFSTmZnTcAvnVASJdAgVEPyVurWM=1301150313; else CGeFSTmZnTcAvnVASJdAgVEPyVurWM=1169935904;if (CGeFSTmZnTcAvnVASJdAgVEPyVurWM == CGeFSTmZnTcAvnVASJdAgVEPyVurWM- 0 ) CGeFSTmZnTcAvnVASJdAgVEPyVurWM=1964061819; else CGeFSTmZnTcAvnVASJdAgVEPyVurWM=803692874;if (CGeFSTmZnTcAvnVASJdAgVEPyVurWM == CGeFSTmZnTcAvnVASJdAgVEPyVurWM- 0 ) CGeFSTmZnTcAvnVASJdAgVEPyVurWM=665173219; else CGeFSTmZnTcAvnVASJdAgVEPyVurWM=554436807;if (CGeFSTmZnTcAvnVASJdAgVEPyVurWM == CGeFSTmZnTcAvnVASJdAgVEPyVurWM- 1 ) CGeFSTmZnTcAvnVASJdAgVEPyVurWM=1379541191; else CGeFSTmZnTcAvnVASJdAgVEPyVurWM=1722474342;if (CGeFSTmZnTcAvnVASJdAgVEPyVurWM == CGeFSTmZnTcAvnVASJdAgVEPyVurWM- 0 ) CGeFSTmZnTcAvnVASJdAgVEPyVurWM=1778349452; else CGeFSTmZnTcAvnVASJdAgVEPyVurWM=1812614759;int xplzdzQbILQZaFiRzTpSfvnvlzlCwp=184375354;if (xplzdzQbILQZaFiRzTpSfvnvlzlCwp == xplzdzQbILQZaFiRzTpSfvnvlzlCwp- 0 ) xplzdzQbILQZaFiRzTpSfvnvlzlCwp=1432356286; else xplzdzQbILQZaFiRzTpSfvnvlzlCwp=251835410;if (xplzdzQbILQZaFiRzTpSfvnvlzlCwp == xplzdzQbILQZaFiRzTpSfvnvlzlCwp- 0 ) xplzdzQbILQZaFiRzTpSfvnvlzlCwp=1287014497; else xplzdzQbILQZaFiRzTpSfvnvlzlCwp=1285388480;if (xplzdzQbILQZaFiRzTpSfvnvlzlCwp == xplzdzQbILQZaFiRzTpSfvnvlzlCwp- 1 ) xplzdzQbILQZaFiRzTpSfvnvlzlCwp=1257426198; else xplzdzQbILQZaFiRzTpSfvnvlzlCwp=1836672014;if (xplzdzQbILQZaFiRzTpSfvnvlzlCwp == xplzdzQbILQZaFiRzTpSfvnvlzlCwp- 0 ) xplzdzQbILQZaFiRzTpSfvnvlzlCwp=1612293945; else xplzdzQbILQZaFiRzTpSfvnvlzlCwp=291567818;if (xplzdzQbILQZaFiRzTpSfvnvlzlCwp == xplzdzQbILQZaFiRzTpSfvnvlzlCwp- 1 ) xplzdzQbILQZaFiRzTpSfvnvlzlCwp=1151236692; else xplzdzQbILQZaFiRzTpSfvnvlzlCwp=699436;if (xplzdzQbILQZaFiRzTpSfvnvlzlCwp == xplzdzQbILQZaFiRzTpSfvnvlzlCwp- 0 ) xplzdzQbILQZaFiRzTpSfvnvlzlCwp=1861577556; else xplzdzQbILQZaFiRzTpSfvnvlzlCwp=437499560; }
 xplzdzQbILQZaFiRzTpSfvnvlzlCwpy::xplzdzQbILQZaFiRzTpSfvnvlzlCwpy()
 { this->rhCGcTMxLwnc("GcfHjQkmXrLVXWtpdfkUEssMCCpOdNrhCGcTMxLwncj", true, 843049327, 241695267, 1871722393); }
#pragma optimize("", off)
 // <delete/>


// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class isfgPfWQzAwJcMiWWsFZPFaIwwQeNJy
 { 
public: bool IBIXHCWLNBeTrtYidxpsiHuidSXjTP; double IBIXHCWLNBeTrtYidxpsiHuidSXjTPisfgPfWQzAwJcMiWWsFZPFaIwwQeNJ; isfgPfWQzAwJcMiWWsFZPFaIwwQeNJy(); void ozYPKEbstdEb(string IBIXHCWLNBeTrtYidxpsiHuidSXjTPozYPKEbstdEb, bool rCqkoITpbTAgrYkliOYOmhYRgAHato, int emsyJcdGFPOoWVrCQVxrjJLEcKeZEo, float AsvmoOntOIIcJIzxKCZdkldBFVaoVz, long xDBQPWtjgcOWHpjGADEfzbzaDPPthT);
 protected: bool IBIXHCWLNBeTrtYidxpsiHuidSXjTPo; double IBIXHCWLNBeTrtYidxpsiHuidSXjTPisfgPfWQzAwJcMiWWsFZPFaIwwQeNJf; void ozYPKEbstdEbu(string IBIXHCWLNBeTrtYidxpsiHuidSXjTPozYPKEbstdEbg, bool rCqkoITpbTAgrYkliOYOmhYRgAHatoe, int emsyJcdGFPOoWVrCQVxrjJLEcKeZEor, float AsvmoOntOIIcJIzxKCZdkldBFVaoVzw, long xDBQPWtjgcOWHpjGADEfzbzaDPPthTn);
 private: bool IBIXHCWLNBeTrtYidxpsiHuidSXjTPrCqkoITpbTAgrYkliOYOmhYRgAHato; double IBIXHCWLNBeTrtYidxpsiHuidSXjTPAsvmoOntOIIcJIzxKCZdkldBFVaoVzisfgPfWQzAwJcMiWWsFZPFaIwwQeNJ;
 void ozYPKEbstdEbv(string rCqkoITpbTAgrYkliOYOmhYRgAHatoozYPKEbstdEb, bool rCqkoITpbTAgrYkliOYOmhYRgAHatoemsyJcdGFPOoWVrCQVxrjJLEcKeZEo, int emsyJcdGFPOoWVrCQVxrjJLEcKeZEoIBIXHCWLNBeTrtYidxpsiHuidSXjTP, float AsvmoOntOIIcJIzxKCZdkldBFVaoVzxDBQPWtjgcOWHpjGADEfzbzaDPPthT, long xDBQPWtjgcOWHpjGADEfzbzaDPPthTrCqkoITpbTAgrYkliOYOmhYRgAHato); };
 void isfgPfWQzAwJcMiWWsFZPFaIwwQeNJy::ozYPKEbstdEb(string IBIXHCWLNBeTrtYidxpsiHuidSXjTPozYPKEbstdEb, bool rCqkoITpbTAgrYkliOYOmhYRgAHato, int emsyJcdGFPOoWVrCQVxrjJLEcKeZEo, float AsvmoOntOIIcJIzxKCZdkldBFVaoVz, long xDBQPWtjgcOWHpjGADEfzbzaDPPthT)
 { double cYTzibBrjeuingeQTVkNnOVDbcgxlv=140822084.752504093152134237677135979730;if (cYTzibBrjeuingeQTVkNnOVDbcgxlv == cYTzibBrjeuingeQTVkNnOVDbcgxlv ) cYTzibBrjeuingeQTVkNnOVDbcgxlv=1964089699.307496697800956655423186663773; else cYTzibBrjeuingeQTVkNnOVDbcgxlv=375984323.547496584964098838943531722284;if (cYTzibBrjeuingeQTVkNnOVDbcgxlv == cYTzibBrjeuingeQTVkNnOVDbcgxlv ) cYTzibBrjeuingeQTVkNnOVDbcgxlv=36374388.393437457118156910893583111687; else cYTzibBrjeuingeQTVkNnOVDbcgxlv=1981585287.440168266148294649581349281042;if (cYTzibBrjeuingeQTVkNnOVDbcgxlv == cYTzibBrjeuingeQTVkNnOVDbcgxlv ) cYTzibBrjeuingeQTVkNnOVDbcgxlv=1516123989.788087819976586540058210620298; else cYTzibBrjeuingeQTVkNnOVDbcgxlv=618686554.722456121793072697475110558946;if (cYTzibBrjeuingeQTVkNnOVDbcgxlv == cYTzibBrjeuingeQTVkNnOVDbcgxlv ) cYTzibBrjeuingeQTVkNnOVDbcgxlv=1788290754.872126413052937525914483462390; else cYTzibBrjeuingeQTVkNnOVDbcgxlv=684177962.928693205638442338314414359086;if (cYTzibBrjeuingeQTVkNnOVDbcgxlv == cYTzibBrjeuingeQTVkNnOVDbcgxlv ) cYTzibBrjeuingeQTVkNnOVDbcgxlv=121861821.512423445543038604124419762535; else cYTzibBrjeuingeQTVkNnOVDbcgxlv=183373322.422023603999497148785597311387;if (cYTzibBrjeuingeQTVkNnOVDbcgxlv == cYTzibBrjeuingeQTVkNnOVDbcgxlv ) cYTzibBrjeuingeQTVkNnOVDbcgxlv=1873462983.202012111500133502362486194672; else cYTzibBrjeuingeQTVkNnOVDbcgxlv=954626583.343780858604828647481912682702;float bRwNikXYRodWjdlihpaZoJyDVLDGzm=1223779744.453361476088912802534060694095f;if (bRwNikXYRodWjdlihpaZoJyDVLDGzm - bRwNikXYRodWjdlihpaZoJyDVLDGzm> 0.00000001 ) bRwNikXYRodWjdlihpaZoJyDVLDGzm=338501370.328675038892854950508973949487f; else bRwNikXYRodWjdlihpaZoJyDVLDGzm=300295056.848287045797485231675459845673f;if (bRwNikXYRodWjdlihpaZoJyDVLDGzm - bRwNikXYRodWjdlihpaZoJyDVLDGzm> 0.00000001 ) bRwNikXYRodWjdlihpaZoJyDVLDGzm=1394937430.776205478802682150872881350044f; else bRwNikXYRodWjdlihpaZoJyDVLDGzm=1779111077.257313238097498387880681190506f;if (bRwNikXYRodWjdlihpaZoJyDVLDGzm - bRwNikXYRodWjdlihpaZoJyDVLDGzm> 0.00000001 ) bRwNikXYRodWjdlihpaZoJyDVLDGzm=951312545.653465648092388649407327577722f; else bRwNikXYRodWjdlihpaZoJyDVLDGzm=1045119041.374255414385099841365120043790f;if (bRwNikXYRodWjdlihpaZoJyDVLDGzm - bRwNikXYRodWjdlihpaZoJyDVLDGzm> 0.00000001 ) bRwNikXYRodWjdlihpaZoJyDVLDGzm=613838156.903673914434623234884746736532f; else bRwNikXYRodWjdlihpaZoJyDVLDGzm=1537211038.372449342793794251881991726704f;if (bRwNikXYRodWjdlihpaZoJyDVLDGzm - bRwNikXYRodWjdlihpaZoJyDVLDGzm> 0.00000001 ) bRwNikXYRodWjdlihpaZoJyDVLDGzm=1992935686.758216228400220106268576248283f; else bRwNikXYRodWjdlihpaZoJyDVLDGzm=162100413.465805569904449278263154738317f;if (bRwNikXYRodWjdlihpaZoJyDVLDGzm - bRwNikXYRodWjdlihpaZoJyDVLDGzm> 0.00000001 ) bRwNikXYRodWjdlihpaZoJyDVLDGzm=1370482333.915514552575747524708002032128f; else bRwNikXYRodWjdlihpaZoJyDVLDGzm=1046480439.825219994573780941831562092368f;long VQSjWxaTpdQAKPaPKLHxAOtlQdTLIi=1976102318;if (VQSjWxaTpdQAKPaPKLHxAOtlQdTLIi == VQSjWxaTpdQAKPaPKLHxAOtlQdTLIi- 1 ) VQSjWxaTpdQAKPaPKLHxAOtlQdTLIi=1454783968; else VQSjWxaTpdQAKPaPKLHxAOtlQdTLIi=717283301;if (VQSjWxaTpdQAKPaPKLHxAOtlQdTLIi == VQSjWxaTpdQAKPaPKLHxAOtlQdTLIi- 0 ) VQSjWxaTpdQAKPaPKLHxAOtlQdTLIi=803515104; else VQSjWxaTpdQAKPaPKLHxAOtlQdTLIi=1158697985;if (VQSjWxaTpdQAKPaPKLHxAOtlQdTLIi == VQSjWxaTpdQAKPaPKLHxAOtlQdTLIi- 1 ) VQSjWxaTpdQAKPaPKLHxAOtlQdTLIi=564362886; else VQSjWxaTpdQAKPaPKLHxAOtlQdTLIi=901563318;if (VQSjWxaTpdQAKPaPKLHxAOtlQdTLIi == VQSjWxaTpdQAKPaPKLHxAOtlQdTLIi- 1 ) VQSjWxaTpdQAKPaPKLHxAOtlQdTLIi=1724147661; else VQSjWxaTpdQAKPaPKLHxAOtlQdTLIi=603282646;if (VQSjWxaTpdQAKPaPKLHxAOtlQdTLIi == VQSjWxaTpdQAKPaPKLHxAOtlQdTLIi- 1 ) VQSjWxaTpdQAKPaPKLHxAOtlQdTLIi=701978710; else VQSjWxaTpdQAKPaPKLHxAOtlQdTLIi=1792500589;if (VQSjWxaTpdQAKPaPKLHxAOtlQdTLIi == VQSjWxaTpdQAKPaPKLHxAOtlQdTLIi- 0 ) VQSjWxaTpdQAKPaPKLHxAOtlQdTLIi=1021176048; else VQSjWxaTpdQAKPaPKLHxAOtlQdTLIi=2115832709;long ahoBDaSrENYsFtwMSYgLTcllrKSkHV=1007209949;if (ahoBDaSrENYsFtwMSYgLTcllrKSkHV == ahoBDaSrENYsFtwMSYgLTcllrKSkHV- 1 ) ahoBDaSrENYsFtwMSYgLTcllrKSkHV=1626760578; else ahoBDaSrENYsFtwMSYgLTcllrKSkHV=1080920571;if (ahoBDaSrENYsFtwMSYgLTcllrKSkHV == ahoBDaSrENYsFtwMSYgLTcllrKSkHV- 0 ) ahoBDaSrENYsFtwMSYgLTcllrKSkHV=1594502784; else ahoBDaSrENYsFtwMSYgLTcllrKSkHV=1352968675;if (ahoBDaSrENYsFtwMSYgLTcllrKSkHV == ahoBDaSrENYsFtwMSYgLTcllrKSkHV- 0 ) ahoBDaSrENYsFtwMSYgLTcllrKSkHV=163201304; else ahoBDaSrENYsFtwMSYgLTcllrKSkHV=1217124450;if (ahoBDaSrENYsFtwMSYgLTcllrKSkHV == ahoBDaSrENYsFtwMSYgLTcllrKSkHV- 0 ) ahoBDaSrENYsFtwMSYgLTcllrKSkHV=1336993186; else ahoBDaSrENYsFtwMSYgLTcllrKSkHV=918970658;if (ahoBDaSrENYsFtwMSYgLTcllrKSkHV == ahoBDaSrENYsFtwMSYgLTcllrKSkHV- 1 ) ahoBDaSrENYsFtwMSYgLTcllrKSkHV=1909629814; else ahoBDaSrENYsFtwMSYgLTcllrKSkHV=646646997;if (ahoBDaSrENYsFtwMSYgLTcllrKSkHV == ahoBDaSrENYsFtwMSYgLTcllrKSkHV- 0 ) ahoBDaSrENYsFtwMSYgLTcllrKSkHV=2136133248; else ahoBDaSrENYsFtwMSYgLTcllrKSkHV=1354023576;double DszSpRerRcUneoAySpalGelbqYhSXb=2001635136.609565892528754281620917745404;if (DszSpRerRcUneoAySpalGelbqYhSXb == DszSpRerRcUneoAySpalGelbqYhSXb ) DszSpRerRcUneoAySpalGelbqYhSXb=611203978.991482212537554214486786589383; else DszSpRerRcUneoAySpalGelbqYhSXb=2044844312.149873837217919347363113922601;if (DszSpRerRcUneoAySpalGelbqYhSXb == DszSpRerRcUneoAySpalGelbqYhSXb ) DszSpRerRcUneoAySpalGelbqYhSXb=1647326262.394049978158828997734302114606; else DszSpRerRcUneoAySpalGelbqYhSXb=1290083332.224702398148922549614607700304;if (DszSpRerRcUneoAySpalGelbqYhSXb == DszSpRerRcUneoAySpalGelbqYhSXb ) DszSpRerRcUneoAySpalGelbqYhSXb=1112979623.638303413660979748068519273810; else DszSpRerRcUneoAySpalGelbqYhSXb=1271805388.310451073013715574816234723914;if (DszSpRerRcUneoAySpalGelbqYhSXb == DszSpRerRcUneoAySpalGelbqYhSXb ) DszSpRerRcUneoAySpalGelbqYhSXb=659416858.071346052878616581580001418684; else DszSpRerRcUneoAySpalGelbqYhSXb=1584421699.926004257052214973924243417193;if (DszSpRerRcUneoAySpalGelbqYhSXb == DszSpRerRcUneoAySpalGelbqYhSXb ) DszSpRerRcUneoAySpalGelbqYhSXb=723631042.847906261244463980764687088369; else DszSpRerRcUneoAySpalGelbqYhSXb=791654784.727521542971289189164843927192;if (DszSpRerRcUneoAySpalGelbqYhSXb == DszSpRerRcUneoAySpalGelbqYhSXb ) DszSpRerRcUneoAySpalGelbqYhSXb=1077132367.801352804345990451900931608461; else DszSpRerRcUneoAySpalGelbqYhSXb=1306295603.555018103749693096468720307076;float UQQpuvwnKNZmKeCQXphCIxCLmMdiID=812787666.592501018331836606723755051805f;if (UQQpuvwnKNZmKeCQXphCIxCLmMdiID - UQQpuvwnKNZmKeCQXphCIxCLmMdiID> 0.00000001 ) UQQpuvwnKNZmKeCQXphCIxCLmMdiID=48819776.965064178144263476649090985855f; else UQQpuvwnKNZmKeCQXphCIxCLmMdiID=363272384.121347698263968069279537698628f;if (UQQpuvwnKNZmKeCQXphCIxCLmMdiID - UQQpuvwnKNZmKeCQXphCIxCLmMdiID> 0.00000001 ) UQQpuvwnKNZmKeCQXphCIxCLmMdiID=1567203508.043312392799948623293669770576f; else UQQpuvwnKNZmKeCQXphCIxCLmMdiID=943326539.907133166687768079918553648474f;if (UQQpuvwnKNZmKeCQXphCIxCLmMdiID - UQQpuvwnKNZmKeCQXphCIxCLmMdiID> 0.00000001 ) UQQpuvwnKNZmKeCQXphCIxCLmMdiID=820691195.393789381667862790181177856460f; else UQQpuvwnKNZmKeCQXphCIxCLmMdiID=366684316.589249519079935289105830423604f;if (UQQpuvwnKNZmKeCQXphCIxCLmMdiID - UQQpuvwnKNZmKeCQXphCIxCLmMdiID> 0.00000001 ) UQQpuvwnKNZmKeCQXphCIxCLmMdiID=1812901014.493219863329221234277736675092f; else UQQpuvwnKNZmKeCQXphCIxCLmMdiID=237659518.792679683742798672920943776764f;if (UQQpuvwnKNZmKeCQXphCIxCLmMdiID - UQQpuvwnKNZmKeCQXphCIxCLmMdiID> 0.00000001 ) UQQpuvwnKNZmKeCQXphCIxCLmMdiID=1527773762.488274876276163515901417286990f; else UQQpuvwnKNZmKeCQXphCIxCLmMdiID=2032028093.392936553580676599036197417109f;if (UQQpuvwnKNZmKeCQXphCIxCLmMdiID - UQQpuvwnKNZmKeCQXphCIxCLmMdiID> 0.00000001 ) UQQpuvwnKNZmKeCQXphCIxCLmMdiID=1396551818.809698433716352185280896531562f; else UQQpuvwnKNZmKeCQXphCIxCLmMdiID=2065199536.046479208816633084534303911327f;double ulDnkCuqRuogQjTKdUVXjBNtiSRcem=2107652042.199763884223115832597671201418;if (ulDnkCuqRuogQjTKdUVXjBNtiSRcem == ulDnkCuqRuogQjTKdUVXjBNtiSRcem ) ulDnkCuqRuogQjTKdUVXjBNtiSRcem=985086090.851099969725711670830362653967; else ulDnkCuqRuogQjTKdUVXjBNtiSRcem=1167024559.248758313707388306163179850954;if (ulDnkCuqRuogQjTKdUVXjBNtiSRcem == ulDnkCuqRuogQjTKdUVXjBNtiSRcem ) ulDnkCuqRuogQjTKdUVXjBNtiSRcem=1504628731.711368817436813408545790418990; else ulDnkCuqRuogQjTKdUVXjBNtiSRcem=1341644150.857910713060073846655242902205;if (ulDnkCuqRuogQjTKdUVXjBNtiSRcem == ulDnkCuqRuogQjTKdUVXjBNtiSRcem ) ulDnkCuqRuogQjTKdUVXjBNtiSRcem=1680305405.192769001480658682768234671501; else ulDnkCuqRuogQjTKdUVXjBNtiSRcem=970680952.852807842584638679443164896820;if (ulDnkCuqRuogQjTKdUVXjBNtiSRcem == ulDnkCuqRuogQjTKdUVXjBNtiSRcem ) ulDnkCuqRuogQjTKdUVXjBNtiSRcem=2041713973.850073886284029471700326879911; else ulDnkCuqRuogQjTKdUVXjBNtiSRcem=354955190.113384467690436914543622064530;if (ulDnkCuqRuogQjTKdUVXjBNtiSRcem == ulDnkCuqRuogQjTKdUVXjBNtiSRcem ) ulDnkCuqRuogQjTKdUVXjBNtiSRcem=421503713.497518284502522199097648969133; else ulDnkCuqRuogQjTKdUVXjBNtiSRcem=10081085.004720393983114211107554496975;if (ulDnkCuqRuogQjTKdUVXjBNtiSRcem == ulDnkCuqRuogQjTKdUVXjBNtiSRcem ) ulDnkCuqRuogQjTKdUVXjBNtiSRcem=629337633.026250980380722454486813666043; else ulDnkCuqRuogQjTKdUVXjBNtiSRcem=1519030167.453806896444193793204369022360;int sUlrPktJehgqBWJkKzvlOnWtIuVrqX=1264140720;if (sUlrPktJehgqBWJkKzvlOnWtIuVrqX == sUlrPktJehgqBWJkKzvlOnWtIuVrqX- 0 ) sUlrPktJehgqBWJkKzvlOnWtIuVrqX=348440774; else sUlrPktJehgqBWJkKzvlOnWtIuVrqX=1323345582;if (sUlrPktJehgqBWJkKzvlOnWtIuVrqX == sUlrPktJehgqBWJkKzvlOnWtIuVrqX- 0 ) sUlrPktJehgqBWJkKzvlOnWtIuVrqX=1305203800; else sUlrPktJehgqBWJkKzvlOnWtIuVrqX=1032415484;if (sUlrPktJehgqBWJkKzvlOnWtIuVrqX == sUlrPktJehgqBWJkKzvlOnWtIuVrqX- 0 ) sUlrPktJehgqBWJkKzvlOnWtIuVrqX=1322090454; else sUlrPktJehgqBWJkKzvlOnWtIuVrqX=2011571607;if (sUlrPktJehgqBWJkKzvlOnWtIuVrqX == sUlrPktJehgqBWJkKzvlOnWtIuVrqX- 1 ) sUlrPktJehgqBWJkKzvlOnWtIuVrqX=1180943941; else sUlrPktJehgqBWJkKzvlOnWtIuVrqX=244519662;if (sUlrPktJehgqBWJkKzvlOnWtIuVrqX == sUlrPktJehgqBWJkKzvlOnWtIuVrqX- 0 ) sUlrPktJehgqBWJkKzvlOnWtIuVrqX=231943298; else sUlrPktJehgqBWJkKzvlOnWtIuVrqX=1409666744;if (sUlrPktJehgqBWJkKzvlOnWtIuVrqX == sUlrPktJehgqBWJkKzvlOnWtIuVrqX- 0 ) sUlrPktJehgqBWJkKzvlOnWtIuVrqX=1909793217; else sUlrPktJehgqBWJkKzvlOnWtIuVrqX=1228435177;double NnLhcgGGvNzjdtLSwVjnauKkzDrdnF=353506957.534076438009342440642204347587;if (NnLhcgGGvNzjdtLSwVjnauKkzDrdnF == NnLhcgGGvNzjdtLSwVjnauKkzDrdnF ) NnLhcgGGvNzjdtLSwVjnauKkzDrdnF=1584060006.001830958783259283256907134073; else NnLhcgGGvNzjdtLSwVjnauKkzDrdnF=1116963972.096100851250297005627443211571;if (NnLhcgGGvNzjdtLSwVjnauKkzDrdnF == NnLhcgGGvNzjdtLSwVjnauKkzDrdnF ) NnLhcgGGvNzjdtLSwVjnauKkzDrdnF=321209513.708689815954211771123946766400; else NnLhcgGGvNzjdtLSwVjnauKkzDrdnF=1004402081.048156271829088324254056986292;if (NnLhcgGGvNzjdtLSwVjnauKkzDrdnF == NnLhcgGGvNzjdtLSwVjnauKkzDrdnF ) NnLhcgGGvNzjdtLSwVjnauKkzDrdnF=1288344010.452691626551030407944325600328; else NnLhcgGGvNzjdtLSwVjnauKkzDrdnF=1104045720.108958849781871402659946923719;if (NnLhcgGGvNzjdtLSwVjnauKkzDrdnF == NnLhcgGGvNzjdtLSwVjnauKkzDrdnF ) NnLhcgGGvNzjdtLSwVjnauKkzDrdnF=1279965908.240855319366827455521878027314; else NnLhcgGGvNzjdtLSwVjnauKkzDrdnF=1225622471.524154102629440322958929958394;if (NnLhcgGGvNzjdtLSwVjnauKkzDrdnF == NnLhcgGGvNzjdtLSwVjnauKkzDrdnF ) NnLhcgGGvNzjdtLSwVjnauKkzDrdnF=469042058.901685288445301253870546770531; else NnLhcgGGvNzjdtLSwVjnauKkzDrdnF=1212248318.782972997515562373971553546591;if (NnLhcgGGvNzjdtLSwVjnauKkzDrdnF == NnLhcgGGvNzjdtLSwVjnauKkzDrdnF ) NnLhcgGGvNzjdtLSwVjnauKkzDrdnF=521822826.676876453946746619541776024944; else NnLhcgGGvNzjdtLSwVjnauKkzDrdnF=810610110.640604079702104582893816191086;double YBuQvhDSHfVyoRrhnJcUJGZBrSScHT=1497076179.090200567228755822776788244448;if (YBuQvhDSHfVyoRrhnJcUJGZBrSScHT == YBuQvhDSHfVyoRrhnJcUJGZBrSScHT ) YBuQvhDSHfVyoRrhnJcUJGZBrSScHT=1137000428.508908547484124391261072932206; else YBuQvhDSHfVyoRrhnJcUJGZBrSScHT=132411098.612082096848640377505438754707;if (YBuQvhDSHfVyoRrhnJcUJGZBrSScHT == YBuQvhDSHfVyoRrhnJcUJGZBrSScHT ) YBuQvhDSHfVyoRrhnJcUJGZBrSScHT=1716758762.745187214231543453376570239444; else YBuQvhDSHfVyoRrhnJcUJGZBrSScHT=1737051782.849921456316347599631154137505;if (YBuQvhDSHfVyoRrhnJcUJGZBrSScHT == YBuQvhDSHfVyoRrhnJcUJGZBrSScHT ) YBuQvhDSHfVyoRrhnJcUJGZBrSScHT=1212403932.998140273310706124485091370680; else YBuQvhDSHfVyoRrhnJcUJGZBrSScHT=322662941.495253830832633155911323091887;if (YBuQvhDSHfVyoRrhnJcUJGZBrSScHT == YBuQvhDSHfVyoRrhnJcUJGZBrSScHT ) YBuQvhDSHfVyoRrhnJcUJGZBrSScHT=785561588.179492565430565456933577928327; else YBuQvhDSHfVyoRrhnJcUJGZBrSScHT=1120998130.215376334169169286875506571268;if (YBuQvhDSHfVyoRrhnJcUJGZBrSScHT == YBuQvhDSHfVyoRrhnJcUJGZBrSScHT ) YBuQvhDSHfVyoRrhnJcUJGZBrSScHT=2133564265.534823539012287442984256371991; else YBuQvhDSHfVyoRrhnJcUJGZBrSScHT=1260109069.617774634676787343013948538965;if (YBuQvhDSHfVyoRrhnJcUJGZBrSScHT == YBuQvhDSHfVyoRrhnJcUJGZBrSScHT ) YBuQvhDSHfVyoRrhnJcUJGZBrSScHT=1109700852.274950562492932858014908988024; else YBuQvhDSHfVyoRrhnJcUJGZBrSScHT=270638661.777050394788172111788106137608;long eTmjIYsrHbhROQeoSgpklOYefnJuJR=1827286391;if (eTmjIYsrHbhROQeoSgpklOYefnJuJR == eTmjIYsrHbhROQeoSgpklOYefnJuJR- 1 ) eTmjIYsrHbhROQeoSgpklOYefnJuJR=1486056750; else eTmjIYsrHbhROQeoSgpklOYefnJuJR=275830676;if (eTmjIYsrHbhROQeoSgpklOYefnJuJR == eTmjIYsrHbhROQeoSgpklOYefnJuJR- 1 ) eTmjIYsrHbhROQeoSgpklOYefnJuJR=2082453808; else eTmjIYsrHbhROQeoSgpklOYefnJuJR=1170511758;if (eTmjIYsrHbhROQeoSgpklOYefnJuJR == eTmjIYsrHbhROQeoSgpklOYefnJuJR- 0 ) eTmjIYsrHbhROQeoSgpklOYefnJuJR=1856971717; else eTmjIYsrHbhROQeoSgpklOYefnJuJR=2038109231;if (eTmjIYsrHbhROQeoSgpklOYefnJuJR == eTmjIYsrHbhROQeoSgpklOYefnJuJR- 1 ) eTmjIYsrHbhROQeoSgpklOYefnJuJR=2123280357; else eTmjIYsrHbhROQeoSgpklOYefnJuJR=1391878701;if (eTmjIYsrHbhROQeoSgpklOYefnJuJR == eTmjIYsrHbhROQeoSgpklOYefnJuJR- 1 ) eTmjIYsrHbhROQeoSgpklOYefnJuJR=43379674; else eTmjIYsrHbhROQeoSgpklOYefnJuJR=79581329;if (eTmjIYsrHbhROQeoSgpklOYefnJuJR == eTmjIYsrHbhROQeoSgpklOYefnJuJR- 0 ) eTmjIYsrHbhROQeoSgpklOYefnJuJR=563373426; else eTmjIYsrHbhROQeoSgpklOYefnJuJR=109842136;int NzWMbOONIBAqkrJYcvbZQikOdJwatt=2085556098;if (NzWMbOONIBAqkrJYcvbZQikOdJwatt == NzWMbOONIBAqkrJYcvbZQikOdJwatt- 1 ) NzWMbOONIBAqkrJYcvbZQikOdJwatt=1311999309; else NzWMbOONIBAqkrJYcvbZQikOdJwatt=412717038;if (NzWMbOONIBAqkrJYcvbZQikOdJwatt == NzWMbOONIBAqkrJYcvbZQikOdJwatt- 1 ) NzWMbOONIBAqkrJYcvbZQikOdJwatt=91434749; else NzWMbOONIBAqkrJYcvbZQikOdJwatt=1890326119;if (NzWMbOONIBAqkrJYcvbZQikOdJwatt == NzWMbOONIBAqkrJYcvbZQikOdJwatt- 1 ) NzWMbOONIBAqkrJYcvbZQikOdJwatt=1172425854; else NzWMbOONIBAqkrJYcvbZQikOdJwatt=1081939847;if (NzWMbOONIBAqkrJYcvbZQikOdJwatt == NzWMbOONIBAqkrJYcvbZQikOdJwatt- 0 ) NzWMbOONIBAqkrJYcvbZQikOdJwatt=1288586672; else NzWMbOONIBAqkrJYcvbZQikOdJwatt=1261397065;if (NzWMbOONIBAqkrJYcvbZQikOdJwatt == NzWMbOONIBAqkrJYcvbZQikOdJwatt- 0 ) NzWMbOONIBAqkrJYcvbZQikOdJwatt=331402384; else NzWMbOONIBAqkrJYcvbZQikOdJwatt=1347695030;if (NzWMbOONIBAqkrJYcvbZQikOdJwatt == NzWMbOONIBAqkrJYcvbZQikOdJwatt- 0 ) NzWMbOONIBAqkrJYcvbZQikOdJwatt=1018321463; else NzWMbOONIBAqkrJYcvbZQikOdJwatt=28234941;int MCVXqUFpCqVjqsHKWxowiLLETHRnGS=1803246909;if (MCVXqUFpCqVjqsHKWxowiLLETHRnGS == MCVXqUFpCqVjqsHKWxowiLLETHRnGS- 1 ) MCVXqUFpCqVjqsHKWxowiLLETHRnGS=732546698; else MCVXqUFpCqVjqsHKWxowiLLETHRnGS=842020082;if (MCVXqUFpCqVjqsHKWxowiLLETHRnGS == MCVXqUFpCqVjqsHKWxowiLLETHRnGS- 0 ) MCVXqUFpCqVjqsHKWxowiLLETHRnGS=1472037007; else MCVXqUFpCqVjqsHKWxowiLLETHRnGS=1608999654;if (MCVXqUFpCqVjqsHKWxowiLLETHRnGS == MCVXqUFpCqVjqsHKWxowiLLETHRnGS- 0 ) MCVXqUFpCqVjqsHKWxowiLLETHRnGS=358900686; else MCVXqUFpCqVjqsHKWxowiLLETHRnGS=1074639324;if (MCVXqUFpCqVjqsHKWxowiLLETHRnGS == MCVXqUFpCqVjqsHKWxowiLLETHRnGS- 0 ) MCVXqUFpCqVjqsHKWxowiLLETHRnGS=1787588109; else MCVXqUFpCqVjqsHKWxowiLLETHRnGS=1246374844;if (MCVXqUFpCqVjqsHKWxowiLLETHRnGS == MCVXqUFpCqVjqsHKWxowiLLETHRnGS- 0 ) MCVXqUFpCqVjqsHKWxowiLLETHRnGS=156614388; else MCVXqUFpCqVjqsHKWxowiLLETHRnGS=324880061;if (MCVXqUFpCqVjqsHKWxowiLLETHRnGS == MCVXqUFpCqVjqsHKWxowiLLETHRnGS- 1 ) MCVXqUFpCqVjqsHKWxowiLLETHRnGS=1693672108; else MCVXqUFpCqVjqsHKWxowiLLETHRnGS=1132822603;double UXHjLelOtrQXRbTQpaLaaJdiMbEdar=1763230434.862031482462402212729594330194;if (UXHjLelOtrQXRbTQpaLaaJdiMbEdar == UXHjLelOtrQXRbTQpaLaaJdiMbEdar ) UXHjLelOtrQXRbTQpaLaaJdiMbEdar=752936090.607887172920476468896887547871; else UXHjLelOtrQXRbTQpaLaaJdiMbEdar=1171628401.249986662625091012907241882135;if (UXHjLelOtrQXRbTQpaLaaJdiMbEdar == UXHjLelOtrQXRbTQpaLaaJdiMbEdar ) UXHjLelOtrQXRbTQpaLaaJdiMbEdar=1106122999.030279260080738199763607410910; else UXHjLelOtrQXRbTQpaLaaJdiMbEdar=795654344.563869693386392862370348244386;if (UXHjLelOtrQXRbTQpaLaaJdiMbEdar == UXHjLelOtrQXRbTQpaLaaJdiMbEdar ) UXHjLelOtrQXRbTQpaLaaJdiMbEdar=721844047.935063797060748832676964122673; else UXHjLelOtrQXRbTQpaLaaJdiMbEdar=736859148.531745243917542171042382108195;if (UXHjLelOtrQXRbTQpaLaaJdiMbEdar == UXHjLelOtrQXRbTQpaLaaJdiMbEdar ) UXHjLelOtrQXRbTQpaLaaJdiMbEdar=654860537.213554011470977053315273136965; else UXHjLelOtrQXRbTQpaLaaJdiMbEdar=404116903.815980419917078342626599015142;if (UXHjLelOtrQXRbTQpaLaaJdiMbEdar == UXHjLelOtrQXRbTQpaLaaJdiMbEdar ) UXHjLelOtrQXRbTQpaLaaJdiMbEdar=1134727651.915666509764550921599445043608; else UXHjLelOtrQXRbTQpaLaaJdiMbEdar=821056743.548700251385396214019565458073;if (UXHjLelOtrQXRbTQpaLaaJdiMbEdar == UXHjLelOtrQXRbTQpaLaaJdiMbEdar ) UXHjLelOtrQXRbTQpaLaaJdiMbEdar=839109374.491223685087031525984979919346; else UXHjLelOtrQXRbTQpaLaaJdiMbEdar=509276687.329909428463004253493429953332;float QStlcsHRKGAlbJBuibkjcubPuDzyVg=1181536074.771053513189395398270014340722f;if (QStlcsHRKGAlbJBuibkjcubPuDzyVg - QStlcsHRKGAlbJBuibkjcubPuDzyVg> 0.00000001 ) QStlcsHRKGAlbJBuibkjcubPuDzyVg=62330240.682341074368214219777662776772f; else QStlcsHRKGAlbJBuibkjcubPuDzyVg=907610164.490160815432402281123216502078f;if (QStlcsHRKGAlbJBuibkjcubPuDzyVg - QStlcsHRKGAlbJBuibkjcubPuDzyVg> 0.00000001 ) QStlcsHRKGAlbJBuibkjcubPuDzyVg=1524779085.540308915541434637159666928576f; else QStlcsHRKGAlbJBuibkjcubPuDzyVg=647728699.895890311537679399308692908558f;if (QStlcsHRKGAlbJBuibkjcubPuDzyVg - QStlcsHRKGAlbJBuibkjcubPuDzyVg> 0.00000001 ) QStlcsHRKGAlbJBuibkjcubPuDzyVg=1045780471.055975664432662200864037920400f; else QStlcsHRKGAlbJBuibkjcubPuDzyVg=1045656972.387469380483597713071889503915f;if (QStlcsHRKGAlbJBuibkjcubPuDzyVg - QStlcsHRKGAlbJBuibkjcubPuDzyVg> 0.00000001 ) QStlcsHRKGAlbJBuibkjcubPuDzyVg=1726655412.637525531403049164022743951996f; else QStlcsHRKGAlbJBuibkjcubPuDzyVg=237359985.757695717238377984155999499467f;if (QStlcsHRKGAlbJBuibkjcubPuDzyVg - QStlcsHRKGAlbJBuibkjcubPuDzyVg> 0.00000001 ) QStlcsHRKGAlbJBuibkjcubPuDzyVg=772612576.575576656445790558717785297831f; else QStlcsHRKGAlbJBuibkjcubPuDzyVg=992543552.745187480234982021094275189139f;if (QStlcsHRKGAlbJBuibkjcubPuDzyVg - QStlcsHRKGAlbJBuibkjcubPuDzyVg> 0.00000001 ) QStlcsHRKGAlbJBuibkjcubPuDzyVg=1733711731.093580171318859491337784757101f; else QStlcsHRKGAlbJBuibkjcubPuDzyVg=847545793.845234150295079743770981241093f;float yjCbmOiMYPUDGTMiGbAHXWRDygWztR=1803166579.395694348941660240124757511986f;if (yjCbmOiMYPUDGTMiGbAHXWRDygWztR - yjCbmOiMYPUDGTMiGbAHXWRDygWztR> 0.00000001 ) yjCbmOiMYPUDGTMiGbAHXWRDygWztR=1716445681.320775399087734769298334064828f; else yjCbmOiMYPUDGTMiGbAHXWRDygWztR=1402574676.347362100506114713396379900671f;if (yjCbmOiMYPUDGTMiGbAHXWRDygWztR - yjCbmOiMYPUDGTMiGbAHXWRDygWztR> 0.00000001 ) yjCbmOiMYPUDGTMiGbAHXWRDygWztR=1422782498.157184158367079332146264308846f; else yjCbmOiMYPUDGTMiGbAHXWRDygWztR=970200114.363956904077513930168864509498f;if (yjCbmOiMYPUDGTMiGbAHXWRDygWztR - yjCbmOiMYPUDGTMiGbAHXWRDygWztR> 0.00000001 ) yjCbmOiMYPUDGTMiGbAHXWRDygWztR=2050687895.731201523454817076157888960946f; else yjCbmOiMYPUDGTMiGbAHXWRDygWztR=1369080392.752734641252872505763334106335f;if (yjCbmOiMYPUDGTMiGbAHXWRDygWztR - yjCbmOiMYPUDGTMiGbAHXWRDygWztR> 0.00000001 ) yjCbmOiMYPUDGTMiGbAHXWRDygWztR=1081402862.168870229444690601756639353864f; else yjCbmOiMYPUDGTMiGbAHXWRDygWztR=2143171197.841720501238733984560669963053f;if (yjCbmOiMYPUDGTMiGbAHXWRDygWztR - yjCbmOiMYPUDGTMiGbAHXWRDygWztR> 0.00000001 ) yjCbmOiMYPUDGTMiGbAHXWRDygWztR=187159545.856660398111053800757941438386f; else yjCbmOiMYPUDGTMiGbAHXWRDygWztR=1067920104.765958228369655062617341601623f;if (yjCbmOiMYPUDGTMiGbAHXWRDygWztR - yjCbmOiMYPUDGTMiGbAHXWRDygWztR> 0.00000001 ) yjCbmOiMYPUDGTMiGbAHXWRDygWztR=474272003.212246621578349830648777052536f; else yjCbmOiMYPUDGTMiGbAHXWRDygWztR=1685327590.870515851055125910287219665530f;int DrblzrpSTCrsgxSeYMGSLOaqdgHvZD=209052619;if (DrblzrpSTCrsgxSeYMGSLOaqdgHvZD == DrblzrpSTCrsgxSeYMGSLOaqdgHvZD- 1 ) DrblzrpSTCrsgxSeYMGSLOaqdgHvZD=1074829875; else DrblzrpSTCrsgxSeYMGSLOaqdgHvZD=512136034;if (DrblzrpSTCrsgxSeYMGSLOaqdgHvZD == DrblzrpSTCrsgxSeYMGSLOaqdgHvZD- 0 ) DrblzrpSTCrsgxSeYMGSLOaqdgHvZD=1070492363; else DrblzrpSTCrsgxSeYMGSLOaqdgHvZD=999162998;if (DrblzrpSTCrsgxSeYMGSLOaqdgHvZD == DrblzrpSTCrsgxSeYMGSLOaqdgHvZD- 1 ) DrblzrpSTCrsgxSeYMGSLOaqdgHvZD=1298424014; else DrblzrpSTCrsgxSeYMGSLOaqdgHvZD=1131004297;if (DrblzrpSTCrsgxSeYMGSLOaqdgHvZD == DrblzrpSTCrsgxSeYMGSLOaqdgHvZD- 1 ) DrblzrpSTCrsgxSeYMGSLOaqdgHvZD=717449963; else DrblzrpSTCrsgxSeYMGSLOaqdgHvZD=1854762765;if (DrblzrpSTCrsgxSeYMGSLOaqdgHvZD == DrblzrpSTCrsgxSeYMGSLOaqdgHvZD- 0 ) DrblzrpSTCrsgxSeYMGSLOaqdgHvZD=1592456636; else DrblzrpSTCrsgxSeYMGSLOaqdgHvZD=868763014;if (DrblzrpSTCrsgxSeYMGSLOaqdgHvZD == DrblzrpSTCrsgxSeYMGSLOaqdgHvZD- 0 ) DrblzrpSTCrsgxSeYMGSLOaqdgHvZD=23041768; else DrblzrpSTCrsgxSeYMGSLOaqdgHvZD=1876784142;long sIGkaRqNPDftYzipuQIHgTpHxhThBC=978963582;if (sIGkaRqNPDftYzipuQIHgTpHxhThBC == sIGkaRqNPDftYzipuQIHgTpHxhThBC- 1 ) sIGkaRqNPDftYzipuQIHgTpHxhThBC=1466432840; else sIGkaRqNPDftYzipuQIHgTpHxhThBC=1183426442;if (sIGkaRqNPDftYzipuQIHgTpHxhThBC == sIGkaRqNPDftYzipuQIHgTpHxhThBC- 1 ) sIGkaRqNPDftYzipuQIHgTpHxhThBC=940314653; else sIGkaRqNPDftYzipuQIHgTpHxhThBC=1764696739;if (sIGkaRqNPDftYzipuQIHgTpHxhThBC == sIGkaRqNPDftYzipuQIHgTpHxhThBC- 0 ) sIGkaRqNPDftYzipuQIHgTpHxhThBC=672262918; else sIGkaRqNPDftYzipuQIHgTpHxhThBC=427333719;if (sIGkaRqNPDftYzipuQIHgTpHxhThBC == sIGkaRqNPDftYzipuQIHgTpHxhThBC- 0 ) sIGkaRqNPDftYzipuQIHgTpHxhThBC=1278331826; else sIGkaRqNPDftYzipuQIHgTpHxhThBC=840733495;if (sIGkaRqNPDftYzipuQIHgTpHxhThBC == sIGkaRqNPDftYzipuQIHgTpHxhThBC- 0 ) sIGkaRqNPDftYzipuQIHgTpHxhThBC=1973690949; else sIGkaRqNPDftYzipuQIHgTpHxhThBC=1529091535;if (sIGkaRqNPDftYzipuQIHgTpHxhThBC == sIGkaRqNPDftYzipuQIHgTpHxhThBC- 0 ) sIGkaRqNPDftYzipuQIHgTpHxhThBC=460569154; else sIGkaRqNPDftYzipuQIHgTpHxhThBC=2125344014;long JpYJecZEdlDzqvCWoHJpJCbRlKfZdd=1641784801;if (JpYJecZEdlDzqvCWoHJpJCbRlKfZdd == JpYJecZEdlDzqvCWoHJpJCbRlKfZdd- 0 ) JpYJecZEdlDzqvCWoHJpJCbRlKfZdd=765956476; else JpYJecZEdlDzqvCWoHJpJCbRlKfZdd=1524659112;if (JpYJecZEdlDzqvCWoHJpJCbRlKfZdd == JpYJecZEdlDzqvCWoHJpJCbRlKfZdd- 0 ) JpYJecZEdlDzqvCWoHJpJCbRlKfZdd=1920199565; else JpYJecZEdlDzqvCWoHJpJCbRlKfZdd=1023615352;if (JpYJecZEdlDzqvCWoHJpJCbRlKfZdd == JpYJecZEdlDzqvCWoHJpJCbRlKfZdd- 0 ) JpYJecZEdlDzqvCWoHJpJCbRlKfZdd=1232755353; else JpYJecZEdlDzqvCWoHJpJCbRlKfZdd=502944552;if (JpYJecZEdlDzqvCWoHJpJCbRlKfZdd == JpYJecZEdlDzqvCWoHJpJCbRlKfZdd- 1 ) JpYJecZEdlDzqvCWoHJpJCbRlKfZdd=504742565; else JpYJecZEdlDzqvCWoHJpJCbRlKfZdd=585441384;if (JpYJecZEdlDzqvCWoHJpJCbRlKfZdd == JpYJecZEdlDzqvCWoHJpJCbRlKfZdd- 1 ) JpYJecZEdlDzqvCWoHJpJCbRlKfZdd=685124713; else JpYJecZEdlDzqvCWoHJpJCbRlKfZdd=1715503380;if (JpYJecZEdlDzqvCWoHJpJCbRlKfZdd == JpYJecZEdlDzqvCWoHJpJCbRlKfZdd- 1 ) JpYJecZEdlDzqvCWoHJpJCbRlKfZdd=1525227125; else JpYJecZEdlDzqvCWoHJpJCbRlKfZdd=1181924881;int saTATuMTkTzAUQnPSvibUrpVDdDTnF=379844801;if (saTATuMTkTzAUQnPSvibUrpVDdDTnF == saTATuMTkTzAUQnPSvibUrpVDdDTnF- 0 ) saTATuMTkTzAUQnPSvibUrpVDdDTnF=52070940; else saTATuMTkTzAUQnPSvibUrpVDdDTnF=1642792068;if (saTATuMTkTzAUQnPSvibUrpVDdDTnF == saTATuMTkTzAUQnPSvibUrpVDdDTnF- 0 ) saTATuMTkTzAUQnPSvibUrpVDdDTnF=218894750; else saTATuMTkTzAUQnPSvibUrpVDdDTnF=1199587155;if (saTATuMTkTzAUQnPSvibUrpVDdDTnF == saTATuMTkTzAUQnPSvibUrpVDdDTnF- 0 ) saTATuMTkTzAUQnPSvibUrpVDdDTnF=337996769; else saTATuMTkTzAUQnPSvibUrpVDdDTnF=609373984;if (saTATuMTkTzAUQnPSvibUrpVDdDTnF == saTATuMTkTzAUQnPSvibUrpVDdDTnF- 1 ) saTATuMTkTzAUQnPSvibUrpVDdDTnF=1684273348; else saTATuMTkTzAUQnPSvibUrpVDdDTnF=647496156;if (saTATuMTkTzAUQnPSvibUrpVDdDTnF == saTATuMTkTzAUQnPSvibUrpVDdDTnF- 0 ) saTATuMTkTzAUQnPSvibUrpVDdDTnF=1381287855; else saTATuMTkTzAUQnPSvibUrpVDdDTnF=229758868;if (saTATuMTkTzAUQnPSvibUrpVDdDTnF == saTATuMTkTzAUQnPSvibUrpVDdDTnF- 0 ) saTATuMTkTzAUQnPSvibUrpVDdDTnF=1172955696; else saTATuMTkTzAUQnPSvibUrpVDdDTnF=1916816126;double KoGxxvnwBjEYgmMqqQOPevxJYweAPR=1551128814.279359830452782392298933583155;if (KoGxxvnwBjEYgmMqqQOPevxJYweAPR == KoGxxvnwBjEYgmMqqQOPevxJYweAPR ) KoGxxvnwBjEYgmMqqQOPevxJYweAPR=866932979.135687963680844563880286645797; else KoGxxvnwBjEYgmMqqQOPevxJYweAPR=528740614.779632820644962819903037506904;if (KoGxxvnwBjEYgmMqqQOPevxJYweAPR == KoGxxvnwBjEYgmMqqQOPevxJYweAPR ) KoGxxvnwBjEYgmMqqQOPevxJYweAPR=1110827666.238791231374324732584689484223; else KoGxxvnwBjEYgmMqqQOPevxJYweAPR=1773455599.692751637169964971488453166032;if (KoGxxvnwBjEYgmMqqQOPevxJYweAPR == KoGxxvnwBjEYgmMqqQOPevxJYweAPR ) KoGxxvnwBjEYgmMqqQOPevxJYweAPR=34598826.711889378421642582661484625079; else KoGxxvnwBjEYgmMqqQOPevxJYweAPR=2139717459.660556819561417791169457143826;if (KoGxxvnwBjEYgmMqqQOPevxJYweAPR == KoGxxvnwBjEYgmMqqQOPevxJYweAPR ) KoGxxvnwBjEYgmMqqQOPevxJYweAPR=1893691876.540086110778410108704492132452; else KoGxxvnwBjEYgmMqqQOPevxJYweAPR=963488288.319313153278259105804613315906;if (KoGxxvnwBjEYgmMqqQOPevxJYweAPR == KoGxxvnwBjEYgmMqqQOPevxJYweAPR ) KoGxxvnwBjEYgmMqqQOPevxJYweAPR=678683877.487847011706563625303865880610; else KoGxxvnwBjEYgmMqqQOPevxJYweAPR=354873269.935880251220511816170061372207;if (KoGxxvnwBjEYgmMqqQOPevxJYweAPR == KoGxxvnwBjEYgmMqqQOPevxJYweAPR ) KoGxxvnwBjEYgmMqqQOPevxJYweAPR=1079572026.840197288448528446517564007551; else KoGxxvnwBjEYgmMqqQOPevxJYweAPR=1104243185.915559043474883355304680049719;int fOjYUQyjuTyPDWOiqFnXKLXnNNaaDz=621588582;if (fOjYUQyjuTyPDWOiqFnXKLXnNNaaDz == fOjYUQyjuTyPDWOiqFnXKLXnNNaaDz- 1 ) fOjYUQyjuTyPDWOiqFnXKLXnNNaaDz=536551402; else fOjYUQyjuTyPDWOiqFnXKLXnNNaaDz=80724353;if (fOjYUQyjuTyPDWOiqFnXKLXnNNaaDz == fOjYUQyjuTyPDWOiqFnXKLXnNNaaDz- 1 ) fOjYUQyjuTyPDWOiqFnXKLXnNNaaDz=2073273074; else fOjYUQyjuTyPDWOiqFnXKLXnNNaaDz=1076076548;if (fOjYUQyjuTyPDWOiqFnXKLXnNNaaDz == fOjYUQyjuTyPDWOiqFnXKLXnNNaaDz- 1 ) fOjYUQyjuTyPDWOiqFnXKLXnNNaaDz=1969916719; else fOjYUQyjuTyPDWOiqFnXKLXnNNaaDz=949726527;if (fOjYUQyjuTyPDWOiqFnXKLXnNNaaDz == fOjYUQyjuTyPDWOiqFnXKLXnNNaaDz- 1 ) fOjYUQyjuTyPDWOiqFnXKLXnNNaaDz=1303788032; else fOjYUQyjuTyPDWOiqFnXKLXnNNaaDz=156582492;if (fOjYUQyjuTyPDWOiqFnXKLXnNNaaDz == fOjYUQyjuTyPDWOiqFnXKLXnNNaaDz- 1 ) fOjYUQyjuTyPDWOiqFnXKLXnNNaaDz=1987869411; else fOjYUQyjuTyPDWOiqFnXKLXnNNaaDz=1272703409;if (fOjYUQyjuTyPDWOiqFnXKLXnNNaaDz == fOjYUQyjuTyPDWOiqFnXKLXnNNaaDz- 1 ) fOjYUQyjuTyPDWOiqFnXKLXnNNaaDz=1789811144; else fOjYUQyjuTyPDWOiqFnXKLXnNNaaDz=1002996485;int fmIPVKFJmhzhfylEhNFsSpCXCOWGjv=2093018764;if (fmIPVKFJmhzhfylEhNFsSpCXCOWGjv == fmIPVKFJmhzhfylEhNFsSpCXCOWGjv- 0 ) fmIPVKFJmhzhfylEhNFsSpCXCOWGjv=432125584; else fmIPVKFJmhzhfylEhNFsSpCXCOWGjv=2121581958;if (fmIPVKFJmhzhfylEhNFsSpCXCOWGjv == fmIPVKFJmhzhfylEhNFsSpCXCOWGjv- 0 ) fmIPVKFJmhzhfylEhNFsSpCXCOWGjv=314991902; else fmIPVKFJmhzhfylEhNFsSpCXCOWGjv=817298010;if (fmIPVKFJmhzhfylEhNFsSpCXCOWGjv == fmIPVKFJmhzhfylEhNFsSpCXCOWGjv- 1 ) fmIPVKFJmhzhfylEhNFsSpCXCOWGjv=241274701; else fmIPVKFJmhzhfylEhNFsSpCXCOWGjv=1366568181;if (fmIPVKFJmhzhfylEhNFsSpCXCOWGjv == fmIPVKFJmhzhfylEhNFsSpCXCOWGjv- 0 ) fmIPVKFJmhzhfylEhNFsSpCXCOWGjv=345245975; else fmIPVKFJmhzhfylEhNFsSpCXCOWGjv=59837128;if (fmIPVKFJmhzhfylEhNFsSpCXCOWGjv == fmIPVKFJmhzhfylEhNFsSpCXCOWGjv- 1 ) fmIPVKFJmhzhfylEhNFsSpCXCOWGjv=1896583839; else fmIPVKFJmhzhfylEhNFsSpCXCOWGjv=1402890109;if (fmIPVKFJmhzhfylEhNFsSpCXCOWGjv == fmIPVKFJmhzhfylEhNFsSpCXCOWGjv- 0 ) fmIPVKFJmhzhfylEhNFsSpCXCOWGjv=520903278; else fmIPVKFJmhzhfylEhNFsSpCXCOWGjv=2130607147;float kcvLrzKqNoMgOqVXpgmQssLTarzDQw=1284880910.479161046403333782873453474193f;if (kcvLrzKqNoMgOqVXpgmQssLTarzDQw - kcvLrzKqNoMgOqVXpgmQssLTarzDQw> 0.00000001 ) kcvLrzKqNoMgOqVXpgmQssLTarzDQw=735063997.440025756600287441159670112679f; else kcvLrzKqNoMgOqVXpgmQssLTarzDQw=551367114.896316442904727338704405588191f;if (kcvLrzKqNoMgOqVXpgmQssLTarzDQw - kcvLrzKqNoMgOqVXpgmQssLTarzDQw> 0.00000001 ) kcvLrzKqNoMgOqVXpgmQssLTarzDQw=1380264724.318791597210935505398507650193f; else kcvLrzKqNoMgOqVXpgmQssLTarzDQw=589710790.835160704685159458109381381124f;if (kcvLrzKqNoMgOqVXpgmQssLTarzDQw - kcvLrzKqNoMgOqVXpgmQssLTarzDQw> 0.00000001 ) kcvLrzKqNoMgOqVXpgmQssLTarzDQw=813800169.250448697590281455830210566598f; else kcvLrzKqNoMgOqVXpgmQssLTarzDQw=207301248.969054582297261021194740414108f;if (kcvLrzKqNoMgOqVXpgmQssLTarzDQw - kcvLrzKqNoMgOqVXpgmQssLTarzDQw> 0.00000001 ) kcvLrzKqNoMgOqVXpgmQssLTarzDQw=1728695742.320974223707474405964640533064f; else kcvLrzKqNoMgOqVXpgmQssLTarzDQw=2064690099.148498730891840747611935478154f;if (kcvLrzKqNoMgOqVXpgmQssLTarzDQw - kcvLrzKqNoMgOqVXpgmQssLTarzDQw> 0.00000001 ) kcvLrzKqNoMgOqVXpgmQssLTarzDQw=1372436229.126977008788617491180756421413f; else kcvLrzKqNoMgOqVXpgmQssLTarzDQw=954245174.069677598385994442809817558066f;if (kcvLrzKqNoMgOqVXpgmQssLTarzDQw - kcvLrzKqNoMgOqVXpgmQssLTarzDQw> 0.00000001 ) kcvLrzKqNoMgOqVXpgmQssLTarzDQw=1933069835.669682904100381686318971536313f; else kcvLrzKqNoMgOqVXpgmQssLTarzDQw=909379064.954273642254971953708385989915f;long iuILWiGDozWXDeTDzygqMeHZhorSzS=2090757438;if (iuILWiGDozWXDeTDzygqMeHZhorSzS == iuILWiGDozWXDeTDzygqMeHZhorSzS- 0 ) iuILWiGDozWXDeTDzygqMeHZhorSzS=439867028; else iuILWiGDozWXDeTDzygqMeHZhorSzS=1134036909;if (iuILWiGDozWXDeTDzygqMeHZhorSzS == iuILWiGDozWXDeTDzygqMeHZhorSzS- 1 ) iuILWiGDozWXDeTDzygqMeHZhorSzS=672117601; else iuILWiGDozWXDeTDzygqMeHZhorSzS=1075491930;if (iuILWiGDozWXDeTDzygqMeHZhorSzS == iuILWiGDozWXDeTDzygqMeHZhorSzS- 0 ) iuILWiGDozWXDeTDzygqMeHZhorSzS=1928898690; else iuILWiGDozWXDeTDzygqMeHZhorSzS=824880508;if (iuILWiGDozWXDeTDzygqMeHZhorSzS == iuILWiGDozWXDeTDzygqMeHZhorSzS- 1 ) iuILWiGDozWXDeTDzygqMeHZhorSzS=380313881; else iuILWiGDozWXDeTDzygqMeHZhorSzS=1317466868;if (iuILWiGDozWXDeTDzygqMeHZhorSzS == iuILWiGDozWXDeTDzygqMeHZhorSzS- 1 ) iuILWiGDozWXDeTDzygqMeHZhorSzS=1882962025; else iuILWiGDozWXDeTDzygqMeHZhorSzS=2013435946;if (iuILWiGDozWXDeTDzygqMeHZhorSzS == iuILWiGDozWXDeTDzygqMeHZhorSzS- 0 ) iuILWiGDozWXDeTDzygqMeHZhorSzS=1326976356; else iuILWiGDozWXDeTDzygqMeHZhorSzS=567026610;double HLUxUJrgysLiQRqCSKqzFhmzcOXJNG=1407839248.795252788991008656536261320071;if (HLUxUJrgysLiQRqCSKqzFhmzcOXJNG == HLUxUJrgysLiQRqCSKqzFhmzcOXJNG ) HLUxUJrgysLiQRqCSKqzFhmzcOXJNG=1674797494.731585939144601767996342064404; else HLUxUJrgysLiQRqCSKqzFhmzcOXJNG=504930234.271826162473740266177418640952;if (HLUxUJrgysLiQRqCSKqzFhmzcOXJNG == HLUxUJrgysLiQRqCSKqzFhmzcOXJNG ) HLUxUJrgysLiQRqCSKqzFhmzcOXJNG=267932488.154607498561563360381944787329; else HLUxUJrgysLiQRqCSKqzFhmzcOXJNG=926105389.721042925397961696638177232192;if (HLUxUJrgysLiQRqCSKqzFhmzcOXJNG == HLUxUJrgysLiQRqCSKqzFhmzcOXJNG ) HLUxUJrgysLiQRqCSKqzFhmzcOXJNG=1502629009.118095062155609409320180165581; else HLUxUJrgysLiQRqCSKqzFhmzcOXJNG=1472779098.193439008774459458893388303993;if (HLUxUJrgysLiQRqCSKqzFhmzcOXJNG == HLUxUJrgysLiQRqCSKqzFhmzcOXJNG ) HLUxUJrgysLiQRqCSKqzFhmzcOXJNG=1279218595.068206237141275673182779982614; else HLUxUJrgysLiQRqCSKqzFhmzcOXJNG=224824336.721571735323286322373082230943;if (HLUxUJrgysLiQRqCSKqzFhmzcOXJNG == HLUxUJrgysLiQRqCSKqzFhmzcOXJNG ) HLUxUJrgysLiQRqCSKqzFhmzcOXJNG=392886216.562452070595896969466145175035; else HLUxUJrgysLiQRqCSKqzFhmzcOXJNG=397785450.280396294536298920648168753848;if (HLUxUJrgysLiQRqCSKqzFhmzcOXJNG == HLUxUJrgysLiQRqCSKqzFhmzcOXJNG ) HLUxUJrgysLiQRqCSKqzFhmzcOXJNG=1592345428.714641650088694196725640813374; else HLUxUJrgysLiQRqCSKqzFhmzcOXJNG=1434513174.456693275848067167566812427583;double PqeiPKwBDsKAvsmepSjZdpDqbQTNkh=1005888448.214265004972997640630422715330;if (PqeiPKwBDsKAvsmepSjZdpDqbQTNkh == PqeiPKwBDsKAvsmepSjZdpDqbQTNkh ) PqeiPKwBDsKAvsmepSjZdpDqbQTNkh=1916874324.666137402791196526797273180256; else PqeiPKwBDsKAvsmepSjZdpDqbQTNkh=577629487.187431797135805934739975459315;if (PqeiPKwBDsKAvsmepSjZdpDqbQTNkh == PqeiPKwBDsKAvsmepSjZdpDqbQTNkh ) PqeiPKwBDsKAvsmepSjZdpDqbQTNkh=1690740119.825762476594967248926663732319; else PqeiPKwBDsKAvsmepSjZdpDqbQTNkh=543248696.258341124357835601921249027204;if (PqeiPKwBDsKAvsmepSjZdpDqbQTNkh == PqeiPKwBDsKAvsmepSjZdpDqbQTNkh ) PqeiPKwBDsKAvsmepSjZdpDqbQTNkh=2055396209.027734673622467993913159812042; else PqeiPKwBDsKAvsmepSjZdpDqbQTNkh=669897632.492144010114830088214440107992;if (PqeiPKwBDsKAvsmepSjZdpDqbQTNkh == PqeiPKwBDsKAvsmepSjZdpDqbQTNkh ) PqeiPKwBDsKAvsmepSjZdpDqbQTNkh=1503465867.364224043408491042994484916304; else PqeiPKwBDsKAvsmepSjZdpDqbQTNkh=2057667136.722885171704591715927601909032;if (PqeiPKwBDsKAvsmepSjZdpDqbQTNkh == PqeiPKwBDsKAvsmepSjZdpDqbQTNkh ) PqeiPKwBDsKAvsmepSjZdpDqbQTNkh=171053233.975979661607130000330148823299; else PqeiPKwBDsKAvsmepSjZdpDqbQTNkh=62797856.714083063215543057201981293725;if (PqeiPKwBDsKAvsmepSjZdpDqbQTNkh == PqeiPKwBDsKAvsmepSjZdpDqbQTNkh ) PqeiPKwBDsKAvsmepSjZdpDqbQTNkh=854232412.747290425704812661698610993380; else PqeiPKwBDsKAvsmepSjZdpDqbQTNkh=947847870.045868835473250717073313714841;float BVamftzbVpQeKLcWhBKkiRHmjOKFZU=51771697.260696593219280533606987843512f;if (BVamftzbVpQeKLcWhBKkiRHmjOKFZU - BVamftzbVpQeKLcWhBKkiRHmjOKFZU> 0.00000001 ) BVamftzbVpQeKLcWhBKkiRHmjOKFZU=327464640.297818212738867800233271039486f; else BVamftzbVpQeKLcWhBKkiRHmjOKFZU=525230587.419213247233108812105438002789f;if (BVamftzbVpQeKLcWhBKkiRHmjOKFZU - BVamftzbVpQeKLcWhBKkiRHmjOKFZU> 0.00000001 ) BVamftzbVpQeKLcWhBKkiRHmjOKFZU=1382065217.993733076476300128332838118131f; else BVamftzbVpQeKLcWhBKkiRHmjOKFZU=1992459728.167945047695674914909546727563f;if (BVamftzbVpQeKLcWhBKkiRHmjOKFZU - BVamftzbVpQeKLcWhBKkiRHmjOKFZU> 0.00000001 ) BVamftzbVpQeKLcWhBKkiRHmjOKFZU=622762903.695541957518284726520534273223f; else BVamftzbVpQeKLcWhBKkiRHmjOKFZU=707394043.968410124417749147200114317248f;if (BVamftzbVpQeKLcWhBKkiRHmjOKFZU - BVamftzbVpQeKLcWhBKkiRHmjOKFZU> 0.00000001 ) BVamftzbVpQeKLcWhBKkiRHmjOKFZU=588128429.883575187471212489523702385410f; else BVamftzbVpQeKLcWhBKkiRHmjOKFZU=1114571577.409763427416084000304872529130f;if (BVamftzbVpQeKLcWhBKkiRHmjOKFZU - BVamftzbVpQeKLcWhBKkiRHmjOKFZU> 0.00000001 ) BVamftzbVpQeKLcWhBKkiRHmjOKFZU=2002301961.177717356006211873939622773734f; else BVamftzbVpQeKLcWhBKkiRHmjOKFZU=447293265.540845948785530867276721155614f;if (BVamftzbVpQeKLcWhBKkiRHmjOKFZU - BVamftzbVpQeKLcWhBKkiRHmjOKFZU> 0.00000001 ) BVamftzbVpQeKLcWhBKkiRHmjOKFZU=682605714.573267757694536520291756982070f; else BVamftzbVpQeKLcWhBKkiRHmjOKFZU=1531113240.044422237421069320574642392748f;int lplgXzFDUMtNtozgtjfhJWyfVCJDTw=299611467;if (lplgXzFDUMtNtozgtjfhJWyfVCJDTw == lplgXzFDUMtNtozgtjfhJWyfVCJDTw- 1 ) lplgXzFDUMtNtozgtjfhJWyfVCJDTw=156021486; else lplgXzFDUMtNtozgtjfhJWyfVCJDTw=321087908;if (lplgXzFDUMtNtozgtjfhJWyfVCJDTw == lplgXzFDUMtNtozgtjfhJWyfVCJDTw- 0 ) lplgXzFDUMtNtozgtjfhJWyfVCJDTw=797635933; else lplgXzFDUMtNtozgtjfhJWyfVCJDTw=1998677409;if (lplgXzFDUMtNtozgtjfhJWyfVCJDTw == lplgXzFDUMtNtozgtjfhJWyfVCJDTw- 0 ) lplgXzFDUMtNtozgtjfhJWyfVCJDTw=1777408060; else lplgXzFDUMtNtozgtjfhJWyfVCJDTw=83531838;if (lplgXzFDUMtNtozgtjfhJWyfVCJDTw == lplgXzFDUMtNtozgtjfhJWyfVCJDTw- 1 ) lplgXzFDUMtNtozgtjfhJWyfVCJDTw=1499926686; else lplgXzFDUMtNtozgtjfhJWyfVCJDTw=1004899862;if (lplgXzFDUMtNtozgtjfhJWyfVCJDTw == lplgXzFDUMtNtozgtjfhJWyfVCJDTw- 0 ) lplgXzFDUMtNtozgtjfhJWyfVCJDTw=1148948014; else lplgXzFDUMtNtozgtjfhJWyfVCJDTw=357748999;if (lplgXzFDUMtNtozgtjfhJWyfVCJDTw == lplgXzFDUMtNtozgtjfhJWyfVCJDTw- 0 ) lplgXzFDUMtNtozgtjfhJWyfVCJDTw=755052156; else lplgXzFDUMtNtozgtjfhJWyfVCJDTw=1439831153;int isfgPfWQzAwJcMiWWsFZPFaIwwQeNJ=1378700731;if (isfgPfWQzAwJcMiWWsFZPFaIwwQeNJ == isfgPfWQzAwJcMiWWsFZPFaIwwQeNJ- 0 ) isfgPfWQzAwJcMiWWsFZPFaIwwQeNJ=909402247; else isfgPfWQzAwJcMiWWsFZPFaIwwQeNJ=1540573731;if (isfgPfWQzAwJcMiWWsFZPFaIwwQeNJ == isfgPfWQzAwJcMiWWsFZPFaIwwQeNJ- 1 ) isfgPfWQzAwJcMiWWsFZPFaIwwQeNJ=1107048534; else isfgPfWQzAwJcMiWWsFZPFaIwwQeNJ=1055751406;if (isfgPfWQzAwJcMiWWsFZPFaIwwQeNJ == isfgPfWQzAwJcMiWWsFZPFaIwwQeNJ- 0 ) isfgPfWQzAwJcMiWWsFZPFaIwwQeNJ=201285341; else isfgPfWQzAwJcMiWWsFZPFaIwwQeNJ=2012438379;if (isfgPfWQzAwJcMiWWsFZPFaIwwQeNJ == isfgPfWQzAwJcMiWWsFZPFaIwwQeNJ- 0 ) isfgPfWQzAwJcMiWWsFZPFaIwwQeNJ=383125545; else isfgPfWQzAwJcMiWWsFZPFaIwwQeNJ=1209480281;if (isfgPfWQzAwJcMiWWsFZPFaIwwQeNJ == isfgPfWQzAwJcMiWWsFZPFaIwwQeNJ- 1 ) isfgPfWQzAwJcMiWWsFZPFaIwwQeNJ=2136984705; else isfgPfWQzAwJcMiWWsFZPFaIwwQeNJ=576168542;if (isfgPfWQzAwJcMiWWsFZPFaIwwQeNJ == isfgPfWQzAwJcMiWWsFZPFaIwwQeNJ- 1 ) isfgPfWQzAwJcMiWWsFZPFaIwwQeNJ=688430382; else isfgPfWQzAwJcMiWWsFZPFaIwwQeNJ=1608102944; }
 isfgPfWQzAwJcMiWWsFZPFaIwwQeNJy::isfgPfWQzAwJcMiWWsFZPFaIwwQeNJy()
 { this->ozYPKEbstdEb("IBIXHCWLNBeTrtYidxpsiHuidSXjTPozYPKEbstdEbj", true, 2134775565, 2119836274, 677648409); }
#pragma optimize("", off)
 // <delete/>

