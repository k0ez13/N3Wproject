#pragma once

#include "../math/Vector.h"
#include "../math/QAngle.h"
#include "../math/VMatrix.h"

#define SIGNONSTATE_NONE		0	// no state yet, about to connect
#define SIGNONSTATE_CHALLENGE	1	// client challenging server, all OOB packets
#define SIGNONSTATE_CONNECTED	2	// client is connected to server, netchans ready
#define SIGNONSTATE_NEW			3	// just got serverinfo and string tables
#define SIGNONSTATE_PRESPAWN	4	// received signon buffers
#define SIGNONSTATE_SPAWN		5	// ready to receive entity packets
#define SIGNONSTATE_FULL		6	// we are fully connected, first non-delta packet received
#define SIGNONSTATE_CHANGELEVEL	7	// server is changing level, please wait

typedef struct input_context_handle_t__ *input_context_handle_t;
struct client_textmessage_t;
struct model_t;
class surf_info;
class i_material;
class c_sentence;
class c_audio_source;
class audio_state_t;
class i_spatial_query;
class i_materialsystem;
class c_phys_collide;
class i_achievement_mgr;
#define FLOW_OUTGOING	0		
#define FLOW_INCOMING	1
#define MAX_FLOWS		2		// in & out

class i_net_channel_info
{
public:

	enum {
		GENERIC = 0,	// must be first and is default group
		LOCALPLAYER,	// bytes for local player entity update
		OTHERPLAYERS,	// bytes for other players update
		ENTITIES,		// all other entity bytes
		SOUNDS,			// game sounds
		EVENTS,			// event messages
		USERMESSAGES,	// user messages
		ENTMESSAGES,	// entity messages
		VOICE,			// voice data
		STRINGTABLE,	// a stringtable update
		MOVE,			// client move cmds
		STRINGCMD,		// string command
		SIGNON,			// various signondata
		TOTAL,			// must be last and is not a real group
	};

	virtual const char  *get_name(void) const = 0;	// get channel name
	virtual const char  *get_address(void) const = 0; // get channel IP address as string
	virtual float		get_time(void) const = 0;	// current net time
	virtual float		get_time_connected(void) const = 0;	// get connection time in seconds
	virtual int			get_buffer_size(void) const = 0;	// netchannel packet history size
	virtual int			get_data_rate(void) const = 0; // send data rate in byte/sec

	virtual bool		is_loopback(void) const = 0;	// true if loopback channel
	virtual bool		is_timingOut(void) const = 0;	// true if timing out
	virtual bool		is_playback(void) const = 0;	// true if demo playback

	virtual float		get_latency(int flow) const = 0;	 // current latency (RTT), more accurate but jittering
	virtual float		get_avg_latency(int flow) const = 0; // average packet latency in seconds
	virtual float		get_avg_loss(int flow) const = 0;	 // avg packet loss[0..1]
	virtual float		get_avg_choke(int flow) const = 0;	 // avg packet choke[0..1]
	virtual float		get_avg_data(int flow) const = 0;	 // data flow in bytes/sec
	virtual float		get_avg_packets(int flow) const = 0; // avg packets/sec
	virtual int		    get_total_data(int flow) const = 0;	 // total flow in/out in bytes
	virtual int			get_sequence_nr(int flow) const = 0;	// last send seq number
	virtual bool		is_valid_packet(int flow, int frame_number) const = 0; // true if packet was not lost/dropped/chocked/flushed
	virtual float		get_packet_time(int flow, int frame_number) const = 0; // time when packet was send
	virtual int			get_packet_bytes(int flow, int frame_number, int group) const = 0; // group size of this packet
	virtual bool		get_stream_progress(int flow, int *received, int *total) const = 0;  // TCP progress if transmitting
	virtual float		get_time_since_last_received(void) const = 0;	// get time since last recieved packet in seconds
	virtual	float		get_command_interpolation_amount(int flow, int frame_number) const = 0;
	virtual void		get_packet_response_latency(int flow, int frame_number, int *pnLatencyMsecs, int *pnChoke) const = 0;
	virtual void		get_remote_framerate(float *pflFrameTime, float *pflFrameTimeStdDeviation) const = 0;

	virtual float		get_timeout_seconds() const = 0;
};

class i_sp_shared_memory;
class c_gamestats_data;
class key_values;
class c_steam_api_context;
struct frustum_t;

typedef void(*pfnDemoCustomDataCallback)(uint8_t *pData, size_t iSize);


typedef struct player_info_s
{
	__int64         unknown;            //0x0000 
	union
	{
		__int64       steamID64;          //0x0008 - SteamID64
		struct
		{
			__int32     xuid_low;
			__int32     xuid_high;
		};
	};

	char            szName[128];        //0x0010 - Player Name
	int             userId;             //0x0090 - Unique Server Identifier
	char            szSteamID[20];      //0x0094 - STEAM_X:Y:Z
	char            pad_0x00A8[0x10];   //0x00A8
	unsigned long   iSteamID;           //0x00B8 - SteamID 
	char            szFriendsName[128];
	bool            fakeplayer;
	bool            ishltv;
	unsigned int    customfiles[4];
	unsigned char   filesdownloaded;
} player_info_t;

#include "../math/Vector2D.h"

class i_map_overview // : public IBaseInterface
{
public:
	virtual	~i_map_overview(void) {};

	virtual	void set_visible(bool state) = 0;	// set map panel visible
	virtual void set_bounds(int x, int y, int wide, int tall) = 0; // set pos & size
	virtual void set_zoom(float zoom) = 0; // set zoom
	virtual void set_time(float time) = 0; // set game time
	virtual void set_angle(float angle) = 0; // set map orientation
	virtual void set_follow_angle(bool state) = 0; // if true, map rotates with spectators view
	virtual void set_center(Vector2D &mappos) = 0; // set map pos in center of panel
	virtual void set_player_positions(int index, const Vector& position, const QAngle& angle) = 0; // update player position
	virtual Vector2D world_to_map(Vector &worldpos) = 0; // convert 3d world to 2d map pos

	virtual bool  is_visible(void) = 0;	// true if MapOverview is visible
	virtual void  get_bounds(int& x, int& y, int& wide, int& tall) = 0; // get current pos & size
	virtual float get_zoom(void) = 0;

	// deatils properties
	virtual	void show_player_names(bool state) = 0;	// show player names under icons
	virtual	void show_tracers(bool state) = 0;	// show shooting traces as lines
	virtual	void show_explosions(bool state) = 0;	// show, smoke, flash & HE grenades
	virtual	void show_health(bool state) = 0;		// show player health under icon
	virtual	void show_hurts(bool state) = 0;	// show player icon flashing if player is hurt
	virtual	void show_tracks(float seconds) = 0; // show player trails for n seconds
};


class i_vengine_client
{
public:
	virtual int                   get_intersecting_surfaces(const model_t *model, const Vector &vCenter, const float radius, const bool bOnlyVisibleSurfaces, surf_info *pInfos, const int nMaxInfos) = 0;
	virtual Vector                get_light_for_point(const Vector &pos, bool bClamp) = 0;
	virtual i_material*            trace_line_material_and_lighting(const Vector &start, const Vector &end, Vector &diffuseLightColor, Vector& baseColor) = 0;
	virtual const char*           parse_file(const char *data, char *token, int maxlen) = 0;
	virtual bool                  CopyFile(const char *source, const char *destination) = 0;
	virtual void                  get_screen_size(int& width, int& height) = 0;
	virtual void                  server_cmd(const char *szCmdString, bool bReliable = true) = 0;
	virtual void                  client_cmd(const char *szCmdString) = 0;
	virtual bool                  get_player_info(int ent_num, player_info_t *pinfo) = 0;
	virtual int                   get_player_for_user_id(int userID) = 0;
	virtual client_textmessage_t* text_message_get(const char *pName) = 0; // 10
	virtual bool                  con_is_visible(void) = 0;
	virtual int                   get_local_player(void) = 0;
	virtual const model_t*        load_model(const char *pName, bool bProp = false) = 0;
	virtual float                 get_last_time_stamp(void) = 0;
	virtual c_sentence*            get_sentence(c_audio_source *pAudioSource) = 0; // 15
	virtual float                 get_sentence_length(c_audio_source *pAudioSource) = 0;
	virtual bool                  is_streaming(c_audio_source *pAudioSource) const = 0;
	virtual void                  get_view_angles(QAngle* va) = 0;
	virtual void                  set_view_angles(QAngle* va) = 0;
	virtual int                   get_max_clients(void) = 0; // 20
	virtual const char*           key_lookup_binding(const char *pBinding) = 0;
	virtual const char*           key_binding_for_key(int &code) = 0;
	virtual void                  key_set_binding(int, char const*) = 0;
	virtual void                  start_key_trap_mode(void) = 0;
	virtual bool                  check_done_key_trapping(int &code) = 0;
	virtual bool                  is_ingame(void) = 0;
	virtual bool                  is_connected(void) = 0;
	virtual bool                  is_drawing_loading_image(void) = 0;
	virtual void                  hide_loading_plaque(void) = 0;
	virtual void                  con_nprintf(int pos, const char *fmt, ...) = 0; // 30
	virtual void                  con_nxprintf(const struct con_nprint_s *info, const char *fmt, ...) = 0;
	virtual int                   is_box_visible(const Vector& mins, const Vector& maxs) = 0;
	virtual int                   ss_Box_inview_cluster(const Vector& mins, const Vector& maxs) = 0;
	virtual bool                  cull_box(const Vector& mins, const Vector& maxs) = 0;
	virtual void                  sound_extra_update(void) = 0;
	virtual const char*           get_game_directory(void) = 0;
	virtual const VMatrix&        world_to_screen_matrix() = 0;
	virtual const VMatrix&        world_to_view_matrix() = 0;
	virtual int                   game_lump_version(int lumpId) const = 0;
	virtual int                   game_lump_size(int lumpId) const = 0; // 40
	virtual bool                  load_game_lump(int lumpId, void* pBuffer, int size) = 0;
	virtual int                   level_leaf_count() const = 0;
	virtual i_spatial_query*        get_bsp_treequery() = 0;
	virtual void                  linear_to_gamma(float* linear, float* gamma) = 0;
	virtual float                 light_style_value(int style) = 0; // 45
	virtual void                  compute_dynamic_lighting(const Vector& pt, const Vector* pNormal, Vector& color) = 0;
	virtual void                  get_ambient_light_color(Vector& color) = 0;
	virtual int                   get_dx_support_level() = 0;
	virtual bool                  supports_hdr() = 0;
	virtual void                  mat_stub(i_materialsystem*pMatSys) = 0; // 50
	virtual void                  get_chapter_name(char *pchBuff, int iMaxLength) = 0;
	virtual char const*           get_level_name(void) = 0;
	virtual char const*           get_level_name_short(void) = 0;
	virtual char const*           get_map_group_name(void) = 0;
	virtual struct i_voice_tweak_s* get_voice_tweak_api(void) = 0;
	virtual void                  set_voice_caster_id(unsigned int someint) = 0; // 56
	virtual void                  engine_stats_begin_frame(void) = 0;
	virtual void                  engine_stats_end_frame(void) = 0;
	virtual void                  fire_events() = 0;
	virtual int                   get_leaves_area(unsigned short *pLeaves, int nLeaves) = 0;
	virtual bool                  does_box_touch_area_frustum(const Vector &mins, const Vector &maxs, int iArea) = 0; // 60
	virtual int                   get_frustum_list(frustum_t **pList, int listMax) = 0;
	virtual bool                  should_use_area_frustum(int i) = 0;
	virtual void                  set_audio_state(const audio_state_t& state) = 0;
	virtual int                   sentence_group_pick(int groupIndex, char *name, int nameBufLen) = 0;
	virtual int                   sentence_group_pick_sequential(int groupIndex, char *name, int nameBufLen, int sentenceIndex, int reset) = 0;
	virtual int                   sentence_index_from_name(const char *pSentenceName) = 0;
	virtual const char*           sentence_name_from_index(int sentenceIndex) = 0;
	virtual int                   sentence_group_index_from_name(const char *pGroupName) = 0;
	virtual const char*           sentence_group_name_from_index(int groupIndex) = 0;
	virtual float                 sentence_length(int sentenceIndex) = 0;
	virtual void                  compute_lighting(const Vector& pt, const Vector* pNormal, bool bClamp, Vector& color, Vector *pBoxColors = NULL) = 0;
	virtual void                  activate_occluder(int nOccluderIndex, bool bActive) = 0;
	virtual bool                  is_occluded(const Vector &vecAbsMins, const Vector &vecAbsMaxs) = 0; // 74
	virtual int                   get_occlusion_viewId(void) = 0;
	virtual void*                 save_alloc_memory(size_t num, size_t size) = 0;
	virtual void                  save_free_memory(void *pSaveMem) = 0;
	virtual i_net_channel_info*      get_net_channel_info(void) = 0;
	virtual void                  debug_draw_phys_collide(const c_phys_collide *pCollide, i_material *pMaterial, const matrix3x4_t& transform, const uint8_t* color) = 0; //79
	virtual void                  check_point(const char *pName) = 0; // 80
	virtual void                  draw_portals() = 0;
	virtual bool                  is_playing_demo(void) = 0;
	virtual bool                  is_recording_demo(void) = 0;
	virtual bool                  is_playing_time_demo(void) = 0;
	virtual int                   get_demo_recording_tick(void) = 0;
	virtual int                   get_demo_playback_tick(void) = 0;
	virtual int                   get_demo_playback_start_tick(void) = 0;
	virtual float                 get_demo_playback_time_scale(void) = 0;
	virtual int                   get_demo_playback_total_ticks(void) = 0;
	virtual bool                  is_paused(void) = 0; // 90
	virtual float                 get_timescale(void) const = 0;
	virtual bool                  is_taking_screenshot(void) = 0;
	virtual bool                  is_hltv(void) = 0;
	virtual bool                  is_level_main_menu_background(void) = 0;
	virtual void                  get_main_menu_background_name(char *dest, int destlen) = 0;
	virtual void                  set_occlusion_parameters(const int /*OcclusionParams_t*/ &params) = 0; // 96
	virtual void                  get_ui_language(char *dest, int destlen) = 0;
	virtual int                   is_skybox_visible_from_point(const Vector &vecPoint) = 0;
	virtual const char*           get_map_entities_string() = 0;
	virtual bool                  is_ineditmode(void) = 0; // 100
	virtual float                 get_screen_aspect_ratio(int viewportWidth, int viewportHeight) = 0;
	virtual bool                  removed_steam_refresh_login(const char *password, bool isSecure) = 0;
	virtual bool                  remobed_SteamProcessCall(bool & finished) = 0;
	virtual unsigned int          get_engine_build_number() = 0; // engines build
	virtual const char *          get_product_version_string() = 0; // mods version number (steam.inf)
	virtual void                  grab_pre_color_corrected_frame(int x, int y, int width, int height) = 0;
	virtual bool                  is_hammer_running() const = 0;
	virtual void                  execute_client_cmd(const char *szCmdString) = 0; //108
	virtual bool                  map_has_hdr_lighting(void) = 0;
	virtual bool                  map_has_light_map_alpha_data(void) = 0;
	virtual int                   get_app_id() = 0;
	virtual Vector                get_light_for_point_fast(const Vector &pos, bool bClamp) = 0;
	virtual void                  client_cmd_unrestricted(char  const*, int, bool) = 0;
	virtual void                  client_cmd_unrestricted(const char *szCmdString) = 0; // 114
	virtual void                  set_restrict_server_commands(bool bRestrict) = 0;
	virtual void                  set_restrict_client_commands(bool bRestrict) = 0;
	virtual void                  set_overlay_bind_proxy(int iOverlayID, void *pBindProxy) = 0;
	virtual bool                  copy_frame_buffer_to_material(const char *pMaterialName) = 0;
	virtual void                  read_configuration(const int iController, const bool readDefault) = 0;
	virtual void                  set_achievement_mgr(i_achievement_mgr *pAchievementMgr) = 0;
	virtual i_achievement_mgr*      get_achievement_mgr() = 0;
	virtual bool                  map_load_failed(void) = 0;
	virtual void                  set_map_load_failed(bool bState) = 0;
	virtual bool                  is_low_violence() = 0;
	virtual const char*           get_most_recent_save_game(void) = 0;
	virtual void                  set_most_recent_save_game(const char *lpszFilename) = 0;
	virtual void                  start_xbox_exiting_process() = 0;
	virtual bool                  is_save_in_progress() = 0;
	virtual bool                  is_auto_save_dangerous_in_progress(void) = 0;
	virtual unsigned int          on_storage_device_attached(int iController) = 0;
	virtual void                  on_storage_device_detached(int iController) = 0;
	virtual const char*           get_save_dir_name(void) = 0;
	virtual void                  write_screenshot(const char *pFilename) = 0;
	virtual void                  reset_demo_interpolation(void) = 0;
	virtual int                   get_active_split_screen_player_slot() = 0;
	virtual int                   set_active_split_screen_player_slot(int slot) = 0;
	virtual bool                  set_local_player_is_resolvable(char const* pchContext, int nLine, bool bResolvable) = 0;
	virtual bool                  is_local_player_resolvable() = 0;
	virtual int                   get_split_screen_player(int nSlot) = 0;
	virtual bool                  is_split_screen_active() = 0;
	virtual bool                  is_valid_split_screen_slot(int nSlot) = 0;
	virtual int                   first_valid_split_screen_slot() = 0; // -1 == invalid
	virtual int                   next_valid_split_screen_slot(int nPreviousSlot) = 0; // -1 == invalid
	virtual i_sp_shared_memory*      get_single_player_shared_memory_space(const char *szName, int ent_num = (1 << 11)) = 0;
	virtual void                  compute_lighting_cube(const Vector& pt, bool bClamp, Vector *pBoxColors) = 0;
	virtual void                  register_demo_custom_data_callback(const char* szCallbackSaveID, pfnDemoCustomDataCallback pCallback) = 0;
	virtual void                  record_demo_custom_data(pfnDemoCustomDataCallback pCallback, const void *pData, size_t iDataLength) = 0;
	virtual void                  set_pitch_csale(float flPitchScale) = 0;
	virtual float                 get_pitch_scale(void) = 0;
	virtual bool                  load_filmmaker() = 0;
	virtual void                  unload_filmmaker() = 0;
	virtual void                  set_leaf_flag(int nLeafIndex, int nFlagBits) = 0;
	virtual void                  recalculate_bsp_leaf_flags(void) = 0;
	virtual bool                  dsp_get_current_das_room_new(void) = 0;
	virtual bool                  dsp_get_current_das_room_changed(void) = 0;
	virtual bool                  dsp_get_current_das_room_sky_above(void) = 0;
	virtual float                 dsp_get_current_das_room_sky_percent(void) = 0;
	virtual void                  set_mix_group_of_current_mixer(const char *szgroupname, const char *szparam, float val, int setMixerType) = 0;
	virtual int                   get_mix_layer_index(const char *szmixlayername) = 0;
	virtual void                  set_mix_layer_level(int index, float level) = 0;
	virtual int                   get_mix_group_index(char  const* groupname) = 0;
	virtual void                  set_mix_layer_trigger_factor(int i1, int i2, float fl) = 0;
	virtual void                  set_mix_layer_trigger_factor(char  const* char1, char  const* char2, float fl) = 0;
	virtual bool                  is_creating_reslist() = 0;
	virtual bool                  is_creating_xbox_reslist() = 0;
	virtual void                  set_timescale(float flTimescale) = 0;
	virtual void                  set_gamestats_data(c_gamestats_data *pGamestatsData) = 0;
	virtual c_gamestats_data*       get_gamestats_data() = 0;
	virtual void                  get_mouse_delta(int &dx, int &dy, bool b) = 0; // unknown
	virtual   const char*         key_lookup_binding_ex(const char *pBinding, int iUserId = -1, int iStartCount = 0, int iAllowJoystick = -1) = 0;
	virtual int                   ley_code_for_binding(char  const*, int, int, int) = 0;
	virtual void                  update_d_and_e_lights(void) = 0;
	virtual int                   get_bug_submission_count() const = 0;
	virtual void                  clear_bug_submission_count() = 0;
	virtual bool                  does_level_contain_water() const = 0;
	virtual float                 get_server_simulation_frame_time() const = 0;
	virtual void                  solid_moved(class i_client_entity *pSolidEnt, class ICollideable *pSolidCollide, const Vector* pPrevAbsOrigin, bool accurateBboxTriggerChecks) = 0;
	virtual void                  trigger_moved(class i_client_entity *pTriggerEnt, bool accurateBboxTriggerChecks) = 0;
	virtual void                  compute_leaves_connected(const Vector &vecOrigin, int nCount, const int *pLeafIndices, bool *pIsConnected) = 0;
	virtual bool                  is_in_commentary_mode(void) = 0;
	virtual void                  set_blur_fade(float amount) = 0;
	virtual bool                  is_transitioning_to_load() = 0;
	virtual void                  search_paths_changed_after_install() = 0;
	virtual void                  configure_system_level(int nCPULevel, int nGPULevel) = 0;
	virtual void                  set_connection_password(char const *pchCurrentPW) = 0;
	virtual c_steam_api_context*     get_steam_api_context() = 0;
	virtual void                  submit_stat_record(char const *szMapName, unsigned int uiBlobVersion, unsigned int uiBlobSize, const void *pvBlob) = 0;
	virtual void                  server_cmd_key_values(key_values *pkey_values) = 0; // 203
	virtual void                  sphere_paint_surface(const model_t* model, const Vector& location, unsigned char chr, float fl1, float fl2) = 0;
	virtual bool                  has_paintmap(void) = 0;
	virtual void                  enable_paintmap_render() = 0;
	//virtual void                trace_paint_surface( const model_t *model, const Vector& position, float radius, CUtlVector<Color>& surfColors ) = 0;
	virtual void                  sphere_trace_paint_surface(const model_t* model, const Vector& position, const Vector &vec2, float radius, /*CUtlVector<unsigned char, CUtlMemory<unsigned char, int>>*/ int& utilVecShit) = 0;
	virtual void                  remove_all_paint() = 0;
	virtual void                  paint_all_surfaces(unsigned char uchr) = 0;
	virtual void                  remove_paint(const model_t* model) = 0;
	virtual bool                  is_active_app() = 0;
	virtual bool                  is_client_local_to_active_server() = 0;
	virtual void                  tick_progress_bar() = 0;
	virtual input_context_handle_t  get_input_context(int /*EngineInputContextId_t*/ id) = 0;
	virtual void                  get_startup_image(char* filename, int size) = 0;
	virtual bool                  is_using_local_network_backdoor(void) = 0;
	virtual void                  save_game(const char*, bool, char*, int, char*, int) = 0;
	virtual void                  get_generic_memory_stats(/* GenericMemoryStat_t */ void **) = 0;
	virtual bool                  game_has_shutdown_and_flushed_memory(void) = 0;
	virtual int                   get_last_acknowledged_Command(void) = 0;
	virtual void                  finish_container_writes(int i) = 0;
	virtual void                  finish_async_Save(void) = 0;
	virtual int                   get_server_tick(void) = 0;
	virtual const char*           get_mod_directory(void) = 0;
	virtual bool                  audio_language_changed(void) = 0;
	virtual bool                  is_auto_save_in_progress(void) = 0;
	virtual void                  start_loading_screen_for_command(const char* command) = 0;
	virtual void                  start_loading_screen_for_key_values(key_values* values) = 0;
	virtual void                  sos_set_opvar_float(const char*, float) = 0;
	virtual void                  sos_get_opvar_float(const char*, float &) = 0;
	virtual bool                  is_subscribed_map(const char*, bool) = 0;
	virtual bool                  is_featured_map(const char*, bool) = 0;
	virtual void                  get_demo_playback_parameters(void) = 0;
	virtual int                   get_client_version(void) = 0;
	virtual bool                  is_demo_skipping(void) = 0;
	virtual void                  set_demo_important_event_data(const key_values* values) = 0;
	virtual void                  clear_events(void) = 0;
	virtual int                   get_safe_zone_xmin(void) = 0;
	virtual bool                  is_voice_recording(void) = 0;
	virtual void                  force_voice_record_on(void) = 0;
	virtual bool                  is_replay(void) = 0;
};
