#pragma once

#include "sdk.h"
#include <array>
#include "../helpers/utils.h"

/*#define NETVAR(type, funcname, class_name, var_name) \
	NETVAR_OFFSET(type, funcname, class_name, var_name, 0) \

#define NETVAR_OFFSET(type, funcname, class_name, var_name, offset) \
auto funcname() -> std::add_lvalue_reference_t<type> \
{ \
	constexpr auto hash = fnv::hash_constexpr(class_name "->" var_name); \
	const auto addr = std::uintptr_t(this) + offset + NetvarSys::Get().Get<hash>(); \
	return *reinterpret_cast<std::add_pointer_t<type>>(addr); \
} \*/

#define NETVAR(type, name, table, netvar)                           \
    type& name##() const {                                          \
        static int _##name = netvar_sys::get_offset(table, netvar);     \
        return *(type*)((uintptr_t)this + _##name);                 \
    }

#define PNETVAR(type, name, table, netvar)                           \
    type* name##() const {                                          \
        static int _##name = netvar_sys::get_offset(table, netvar);     \
        return (type*)((uintptr_t)this + _##name);                 \
    } \


#define NETVAR_EX( type, name, table, netvar, extra )                         \
  type& name##() const {                                                      \
    static int _##name = netvar_sys::get_offset( table, netvar ) + extra; \
    return *(type*)( (uintptr_t)this + _##name );                             \
  }

#define NETPROP(name, table, netvar) static recv_prop* name() \
{ \
	static auto prop_ptr = netvar_sys::get_netvar_prop(table,netvar); \
	return prop_ptr; \
}

struct datamap_t;
class animation_layer;
class c_base_player_animstate;
class c_csgo_player_animstate;
class c_base_entity;




enum cs_weapon_type
{
	WEAPONTYPE_KNIFE = 0,
	WEAPONTYPE_PISTOL,
	WEAPONTYPE_SUBMACHINEGUN,
	WEAPONTYPE_RIFLE,
	WEAPONTYPE_SHOTGUN,
	WEAPONTYPE_SNIPER_RIFLE,
	WEAPONTYPE_MACHINEGUN,
	WEAPONTYPE_C4,
	WEAPONTYPE_PLACEHOLDER,
	WEAPONTYPE_GRENADE,
	WEAPONTYPE_UNKNOWN
};

class c_base_entity;

enum client_frame_staget
{
	FRAME_UNDEFINED = -1,			// (haven't run any frames yet)
	FRAME_START,

	// A network packet is being recieved
	FRAME_NET_UPDATE_START,
	// Data has been received and we're going to start calling PostDataUpdate
	FRAME_NET_UPDATE_POSTDATAUPDATE_START,
	// Data has been received and we've called PostDataUpdate on all data recipients
	FRAME_NET_UPDATE_POSTDATAUPDATE_END,
	// We've received all packets, we can now do interpolation, prediction, etc..
	FRAME_NET_UPDATE_END,

	// We're about to start rendering the scene
	FRAME_RENDER_START,
	// We've finished rendering the scene.
	FRAME_RENDER_END
};



// Created with ReClass.NET by KN4CK3R
class c_hud_texture
{
public:
	char szShortName[64];    //0x0000
	char szTextureFile[64];  //0x0040
	bool bRenderUsingFont;   //0x0080
	bool bPrecached;         //0x0081
	int8_t cCharacterInFont; //0x0082
	uint8_t pad_0083[1];     //0x0083
	uint32_t hFont;          //0x0084
	int32_t i_textureId;      //0x0088
	float afTexCoords[4];    //0x008C
	uint8_t pad_009C[16];    //0x009C
};

class c_econ_item_view
{
private:
	using str_32 = char[32];
public:
	NETVAR( int32_t, m_bInitialized, "DT_BaseAttributableItem", "m_bInitialized" );
	NETVAR( int16_t, m_iItemDefinitionIndex, "DT_BaseAttributableItem", "m_iItemDefinitionIndex" );
	NETVAR( int32_t, m_iEntityLevel, "DT_BaseAttributableItem", "m_iEntityLevel" );
	NETVAR( int32_t, m_iAccountID, "DT_BaseAttributableItem", "m_iAccountID" );
	NETVAR( int32_t, m_iItemIDLow, "DT_BaseAttributableItem", "m_iItemIDLow" );
	NETVAR( int32_t, m_iItemIDHigh, "DT_BaseAttributableItem", "m_iItemIDHigh" );
	NETVAR( int32_t, m_iEntityQuality, "DT_BaseAttributableItem", "m_iEntityQuality" );
	NETVAR( str_32, m_iCustomName, "DT_BaseAttributableItem", "m_szCustomName" );
};

class c_base_entity : public i_client_entity
{
public:
	datamap_t * get_data_desc_map( ) {
		typedef datamap_t*( __thiscall *o_GetPredDescMap )( void* );
		return CallVFunction<o_GetPredDescMap>( this, 15 )( this );
	}

	datamap_t *get_pred_desc_map( ) {
		typedef datamap_t*( __thiscall *o_GetPredDescMap )( void* );
		return CallVFunction<o_GetPredDescMap>( this, 17 )( this );
	}
	static __forceinline c_base_entity* get_entity_by_index( int index ) {
		return static_cast< c_base_entity* >( g_entity_list->get_client_entity( index ) );
	}
	static __forceinline c_base_entity* get_entity_from_handle(c_base_handle h ) {
		return static_cast< c_base_entity* >( g_entity_list->get_client_entity_from_handle( h ) );
	
	
	}

	NETVAR(int32_t, m_nModelIndex, "DT_BaseEntity", "m_nModelIndex" );
	NETVAR(int32_t, m_iTeamNum, "DT_BaseEntity", "m_iTeamNum" );
	NETVAR(Vector, m_vecOrigin, "DT_BaseEntity", "m_vecOrigin" );
	NETVAR(Vector, m_vecAngles, "DT_BaseEntity", "m_vecAngles" );
	NETVAR(bool, m_bShouldGlow, "DT_DynamicProp", "m_bShouldGlow" );
	NETVAR(int32_t, m_nHitboxSet, "DT_BaseAnimating", "m_nHitboxSet");
	NETVAR(int32_t, m_lifeState, "DT_BasePlayer", "m_lifeState");
	NETVAR(c_handle<c_base_player>, m_hOwnerEntity, "DT_BaseEntity", "m_hOwnerEntity" );
	NETVAR(bool, m_bSpotted, "DT_BaseEntity", "m_bSpotted" );
	NETVAR(float_t, m_flC4Blow, "DT_PlantedC4", "m_flC4Blow" );
	NETVAR_EX(int, get_index, "DT_BaseEntity", "m_bIsAutoaimTarget", +0x4);

	void set_module_index(int index)
	{
		return CallVFunction<void(__thiscall*)(void*, int)>(this, 75)(this, index);
	}

	const matrix3x4_t& m_rgflCoordinateFrame( ) {
		static auto _m_rgflCoordinateFrame = netvar_sys::get_offset( "DT_BaseEntity", "m_CollisionGroup" ) - 0x30;
		return *( matrix3x4_t* )( ( uintptr_t )this + _m_rgflCoordinateFrame );
	}

	bool is_player();
	bool is_loot();
	bool is_weapon();
	bool is_planted_c4();
	bool is_defuse_kit();
	matrix3x4_t get_bone_matrix(int BoneID);
	int get_health();

	bool new_setup_bones(matrix3x4_t* matrix);

	Vector& get_abs_origin()
	{
		return CallVFunction<Vector & (__thiscall*)(void*)>(this, 10)(this);
	}

	void set_abs_original(Vector origin)
	{
		using SetAbsOriginFn = void(__thiscall*)(void*, const Vector & origin);
		static SetAbsOriginFn set_abs_origin;
		if (!set_abs_origin)
		{
			set_abs_origin = (SetAbsOriginFn)((DWORD)utils::pattern_scan(GetModuleHandleW(L"client.dll"), "55 8B EC 83 E4 F8 51 53 56 57 8B F1 E8"));
		}
		set_abs_origin(this, origin);
	}
};


class c_planted_c4
{
public:
	NETVAR( bool, m_bBombTicking, "DT_PlantedC4", "m_bBombTicking" );
	NETVAR( bool, m_bBombDefused, "DT_PlantedC4", "m_bBombDefused" );
	NETVAR( float, m_flC4Blow, "DT_PlantedC4", "m_flC4Blow" );
	NETVAR( float, m_flTimerLength, "DT_PlantedC4", "m_flTimerLength" );
	NETVAR( float, m_flDefuseLength, "DT_PlantedC4", "m_flDefuseLength" );
	NETVAR( float, m_flDefuseCountDown, "DT_PlantedC4", "m_flDefuseCountDown" );
	NETVAR( c_handle<c_base_player>, m_hBombDefuser, "DT_PlantedC4", "m_hBombDefuser" );
};

class c_base_attributable_item : public c_base_entity
{
public:
	NETVAR( uint64_t, m_OriginalOwnerXuid, "DT_BaseAttributableItem", "m_OriginalOwnerXuidLow" );
	NETVAR( int32_t, m_OriginalOwnerXuidLow, "DT_BaseAttributableItem", "m_OriginalOwnerXuidLow" );
	NETVAR( int32_t, m_OriginalOwnerXuidHigh, "DT_BaseAttributableItem", "m_OriginalOwnerXuidHigh" );
	NETVAR( int32_t, m_nFallbackStatTrak, "DT_BaseAttributableItem", "m_nFallbackStatTrak" );
	NETVAR( int32_t, m_nFallbackPaintKit, "DT_BaseAttributableItem", "m_nFallbackPaintKit" );
	NETVAR( int32_t, m_nFallbackSeed, "DT_BaseAttributableItem", "m_nFallbackSeed" );
	NETVAR( float_t, m_flFallbackWear, "DT_BaseAttributableItem", "m_flFallbackWear" );

	NETVAR( c_econ_item_view, m_Item2, "DT_BaseAttributableItem", "m_Item" );

	c_econ_item_view& m_Item( ) {
		// Cheating. It should be this + m_Item netvar but then the netvars inside c_econ_item_view wont work properly.
		// A real fix for this requires a rewrite of the netvar manager
		return *( c_econ_item_view* )this;
	}
	void set_glove_model_index( int modelIndex );

	void set_odel_index(const int index)
	{
		return CallVFunction<void(__thiscall*)(c_base_entity*, int)>(this, 75)(this, index);
	}

};

class c_base_weapon_world_model : public c_base_entity
{
public:
	NETVAR( int32_t, m_nModelIndex, "DT_BaseWeaponWorldModel", "m_nModelIndex" );
};

class c_base_combat_weapon : public c_base_attributable_item
{
public:
	NETVAR( float_t, m_flNextPrimaryAttack, "DT_BaseCombatWeapon", "m_flNextPrimaryAttack" );
	NETVAR( float_t, m_flNextSecondaryAttack, "DT_BaseCombatWeapon", "m_flNextSecondaryAttack" );
	NETVAR( int32_t, m_iClip1, "DT_BaseCombatWeapon", "m_iClip1" );
	NETVAR( int32_t, m_iClip2, "DT_BaseCombatWeapon", "m_iClip2" );
	NETVAR(bool, m_bStartedArming, "DT_WeaponC4", "m_bStartedArming");
	NETVAR( float_t, m_flRecoilIndex, "DT_WeaponCSBase", "m_flRecoilIndex" );
	NETVAR( int32_t, m_iViewModelIndex, "DT_BaseCombatWeapon", "m_iViewModelIndex" );
	NETVAR( int32_t, m_iWorldModelIndex, "DT_BaseCombatWeapon", "m_iWorldModelIndex" );
	NETVAR( int32_t, m_iWorldDroppedModelIndex, "DT_BaseCombatWeapon", "m_iWorldDroppedModelIndex" );
	NETVAR( bool, m_bPinPulled, "DT_BaseCSGrenade", "m_bPinPulled" );
	NETVAR(short, m_iItemDefinitionIndex, "CBaseAttributableItem", "m_iItemDefinitionIndex");
	NETVAR( float_t, m_fThrowTime, "DT_BaseCSGrenade", "m_fThrowTime" );
	NETVAR( float_t, m_flPostponeFireReadyTime, "DT_BaseCombatWeapon", "m_flPostponeFireReadyTime" );
	NETVAR( c_handle<c_base_weapon_world_model>, m_hWeaponWorldModel, "DT_BaseCombatWeapon", "m_hWeaponWorldModel" );

	NETVAR( int32_t, m_weaponMode, "DT_WeaponCSBase", "m_weaponMode" );
	NETVAR( int32_t, m_zoomLevel, "DT_WeaponCSBaseGun", "m_zoomLevel" );


	c_cs_weapon_info* get_cs_weapondata( );
	bool has_bullets( );
	char* get_gun_icon();
	char* GetGunIcon();
	float GetGunStringSize();
	bool can_fire( );
	bool is_grenade( );
	bool is_knife( );
	bool is_zeus( );
	bool IsAllWeapons();
	bool is_reloading( );
	bool is_rifle( );
	bool is_pistol( );
	bool is_sniper( );
	bool is_gun( );

	//float CurMaxReloadTime;

	float get_inaccuracy( );
	float get_spread( );
	void update_accuracy_penalty( );
	CUtlVector<i_ref_counted*>& m_CustomMaterials( );
	bool* m_b_custom_material_initialized( );
	int get_item_definition_index();

};

enum observer_mode {
	OBS_MODE_IN_EYE = 4,
	OBS_MODE_CHASE,
	OBS_MODE_ROAMING = 7
};

class c_base_player : public c_base_entity
{
public:
	static __forceinline c_base_player* get_player_by_user_id( int id ) {
		return static_cast< c_base_player* >( get_entity_by_index( g_engine_client->get_player_for_user_id( id ) ) );
	}
	static __forceinline c_base_player* get_player_by_index( int i ) {
		return static_cast< c_base_player* >(get_entity_by_index( i ) );
	}

	NETVAR( observer_mode, m_observer_mode, "DT_BasePlayer", "m_iobserver_mode")
	NETVAR( bool, m_bIsDefusing, "DT_CSPlayer", "m_bIsDefusing" );
	NETVAR( bool, m_bGunGameImmunity, "DT_CSPlayer", "m_bGunGameImmunity" );
	NETVAR( int32_t, m_iShotsFired, "DT_CSPlayer", "m_iShotsFired" );
	NETVAR( QAngle, m_angEyeAngles, "DT_CSPlayer", "m_angEyeAngles[0]" );
	NETVAR( int, m_ArmorValue, "DT_CSPlayer", "m_ArmorValue" );
	NETVAR( bool, m_bHasHeavyArmor, "DT_CSPlayer", "m_bHasHeavyArmor" );
	NETVAR( bool, m_bHasHelmet, "DT_CSPlayer", "m_bHasHelmet" );
	NETVAR( int, ragdoll, "DT_CSPlayer", "m_hRagdoll");
	NETVAR( bool, m_bIsScoped, "DT_CSPlayer", "m_bIsScoped" );;
	NETVAR( float, m_flLowerBodyYawTarget, "DT_CSPlayer", "m_flLowerBodyYawTarget" );
	NETVAR( int32_t, m_iHealth, "DT_BasePlayer", "m_iHealth" );
	NETVAR( int32_t, m_lifeState, "DT_BasePlayer", "m_lifeState" );
	NETVAR( int32_t, m_fFlags, "DT_BasePlayer", "m_fFlags" );
	NETVAR(bool, m_bClientSideAnimation, "DT_BaseAnimating", "m_bClientSideAnimation");
	NETVAR( int32_t, m_nTickBase, "DT_BasePlayer", "m_nTickBase" );
	NETVAR( Vector, m_vecViewOffset, "DT_BasePlayer", "m_vecViewOffset[0]" );
	NETVAR( QAngle, m_viewPunchAngle, "DT_BasePlayer", "m_viewPunchAngle" );
	NETVAR( QAngle, m_aimPunchAngle, "DT_BasePlayer", "m_aimPunchAngle" );
	NETVAR( c_handle<c_base_view_model>, m_hViewModel, "DT_BasePlayer", "m_hViewModel[0]" );
	NETVAR(c_handle<c_base_player>, m_hGroundEntity, "DT_CSPlayer", "m_hGroundEntity");
	NETVAR( Vector, m_vecVelocity, "DT_BasePlayer", "m_vecVelocity[0]" );
	NETVAR( float, m_flMaxspeed, "DT_BasePlayer", "m_flMaxspeed" );
	NETVAR(c_handle<c_base_player>, m_hObserverTarget, "DT_BasePlayer", "m_hObserverTarget");
	NETVAR(int, ObserverTarget, "DT_BasePlayer", "m_hObserverTarget");
	NETVAR( float, m_flFlashMaxAlpha, "DT_CSPlayer", "m_flFlashMaxAlpha" );
	NETVAR( float, m_flNextAttack, "DT_BaseCombatCharacter", "m_flNextAttack" );
	NETVAR( c_handle<c_base_combat_weapon>, m_hActiveWeapon, "DT_BaseCombatCharacter", "m_hActiveWeapon" );
	NETVAR( int32_t, m_iAccount, "DT_CSPlayer", "m_iAccount" );
	NETVAR( float, m_flFlashDuration, "DT_CSPlayer", "m_flFlashDuration" );
	NETVAR( float, m_flSimulationTime, "DT_BaseEntity", "m_flSimulationTime" );
	NETVAR( float, m_flCycle, "DT_ServerAnimationData", "m_flCycle" );
	NETVAR( int, m_nSequence, "DT_BaseViewModel", "m_nSequence" );
	PNETVAR( char, m_szLastPlaceName, "DT_BasePlayer", "m_szLastPlaceName" );
	NETPROP( m_flLowerBodyYawTargetProp, "DT_CSPlayer", "m_flLowerBodyYawTarget" );
	NETVAR( int32_t, m_iFOV, "DT_BasePlayer", "m_iFOV" );
	NETVAR( int32_t, m_iDefaultFOV, "DT_BasePlayer", "m_iDefaultFOV" );
	NETVAR(Vector, m_skybox3d_origin, "DT_BasePlayer", "DT_LocalPlayerExclusive", "DT_Local", "m_skybox3d.origin");
	NETVAR(int, m_skybox3d_scale, "DT_BasePlayer", "DT_LocalPlayerExclusive", "DT_Local", "m_skybox3d.scale");

	NETVAR(int, m_nSurvivalTeam, "DT_CSPlayer", "m_nSurvivalTeam");

	int get_fov( ) {
		if( m_iFOV( ) != 0 )
			return m_iFOV( );
		return m_iDefaultFOV( );
	}

	//NETVAR(int, m_iAccount, "DT_CSPlayer", "m_iAccount");

	bool in_dangerzone()
	{
		static auto game_type = g_cvar->find_var("game_type");
		return game_type->get_int() == 6;
	}

	float& m_flOldSimulationTime() {

		static int name = netvar_sys::get_offset("DT_BaseEntity", "m_flSimulationTime");
		return *(float*)((uintptr_t)this + (name + 4));
	}

	Vector& get_abs_velocity()
	{
		static unsigned int _m_flMaxspeed = utils::find_in_data_map(get_pred_desc_map(), "m_vecAbsVelocity");
		return *(Vector*)((uintptr_t)this + _m_flMaxspeed);
	}

	bool is_enemy()
	{
		if (in_dangerzone())
		{
			return this->m_nSurvivalTeam() != g_local_player->m_nSurvivalTeam() || g_local_player->m_nSurvivalTeam() == -1;
		}
		else
		{
			return this->m_iTeamNum() != g_local_player->m_iTeamNum();
		}
	}

	void set_module_index(int index)
	{
		return CallVFunction<void(__thiscall*)(void*, int)>(this, 75)(this, index);
	}

	void set_abs_angles2(Vector angle)
	{
		using SetAbsAnglesFn = void(__thiscall*)(void*, const Vector & angle);
		static SetAbsAnglesFn set_abs_angles;

		if (!set_abs_angles)
			set_abs_angles = (SetAbsAnglesFn)(utils::pattern_scan(GetModuleHandleW(L"client.dll"), "55 8B EC 83 E4 F8 83 EC 64 53 56 57 8B F1 E8"));

		set_abs_angles(this, angle);
	}

	void set_abs_angles(const QAngle& angles)
	{
		using SetAbsAnglesFn = void(__thiscall*)(void*, const QAngle &angles);
		static SetAbsAnglesFn set_abs_angles = (SetAbsAnglesFn)utils::pattern_scan(GetModuleHandleW(L"client.dll"), "55 8B EC 83 E4 F8 83 EC 64 53 56 57 8B F1 E8");

		set_abs_angles(this, angles);
	}

	void set_abs_original(Vector origin)
	{
		using SetAbsOriginFn = void(__thiscall*)(void*, const Vector &origin);
		static SetAbsOriginFn SetAbsOrigin;
		if (!SetAbsOrigin)
		{
			SetAbsOrigin = (SetAbsOriginFn)((DWORD)utils::pattern_scan(GetModuleHandleW(L"client.dll"), "55 8B EC 83 E4 F8 51 53 56 57 8B F1 E8"));
		}
		SetAbsOrigin(this, origin);
	}

	NETVAR( QAngle, m_angAbsAngles, "DT_BaseEntity", "m_angAbsAngles" );
	NETVAR( Vector, m_angAbsOrigin, "DT_BaseEntity", "m_angAbsOrigin" );
	NETVAR( float, m_flDuckSpeed, "DT_BaseEntity", "m_flDuckSpeed" );
	NETVAR( float, m_flDuckAmount, "DT_BasePlayer", "m_flDuckAmount" );
	matrix3x4_t get_bone_matrix(int BoneID);
	std::array<float, 24> m_flPoseParameter( ) const {
		static int _m_flPoseParameter = netvar_sys::get_offset( "DT_BaseAnimating", "m_flPoseParameter" );
		return *( std::array<float, 24>* )( ( uintptr_t )this + _m_flPoseParameter );
	}


	PNETVAR( c_handle<c_base_combat_weapon>, m_hMyWeapons, "DT_BaseCombatCharacter", "m_hMyWeapons" );
	PNETVAR( c_handle<c_base_attributable_item>, m_hMyWearables, "DT_BaseCombatCharacter", "m_hMyWearables" );

	c_usercmd*& m_pCurrentCommand( );

	int& get_eflags()
	{
		static unsigned int _m_flMaxspeed = utils::find_in_data_map(get_pred_desc_map(), "m_iEFlags"); 
		return *(int*)((uintptr_t)this + _m_flMaxspeed);
	}

	void invalidate_bone_cache( );
	int get_num_anim_overlays( );
	Vector& get_abs_angles2();
	QAngle& get_abs_angles3();
	Vector& get_abs_origin();
	void set_angle2(Vector wantedang);
	animation_layer *get_anim_overlays( );
	animation_layer *get_anim_overlay( int i );
	int get_sequence_activity( int sequence );
	c_csgo_player_animstate *get_player_anim_state( );

	static void update_animation_state( c_csgo_player_animstate *state, QAngle angle );
	static void reset_animation_state( c_csgo_player_animstate *state );
	void create_animation_state( c_csgo_player_animstate *state );

	float_t &m_surfaceFriction( ) {
		static unsigned int _m_surfaceFriction = utils::find_in_data_map( get_pred_desc_map( ), "m_surfaceFriction" );
		return *( float_t* )( ( uintptr_t )this + _m_surfaceFriction );
	}
	Vector &m_vecBaseVelocity( ) {
		static unsigned int _m_vecBaseVelocity = utils::find_in_data_map(get_pred_desc_map( ), "m_vecBaseVelocity" );
		return *( Vector* )( ( uintptr_t )this + _m_vecBaseVelocity );
	}

	float_t &m_flMaxspeed( ) {
		static unsigned int _m_flMaxspeed = utils::find_in_data_map(get_pred_desc_map( ), "m_flMaxspeed" );
		return *( float_t* )( ( uintptr_t )this + _m_flMaxspeed );
	}


	Vector        get_eye_pos( );
	player_info_t get_player_info( );
	bool          is_alive( );
	bool		  is_flashed(int min_alpha);
	bool          has_c4( );
	Vector        get_hitbox_pos( int hitbox_id );

	void		  precace_optimized_hitboxes();
	bool get_optimized_hitbox_pos(int hitbox, Vector& output);

	mstudiobbox_t * get_hitbox( int hitbox_id );
	bool          get_hitboxPos( int hitbox, Vector &output );
	Vector        get_bone_pos( int bone );
	bool          can_see_player( c_base_player* player, int hitbox );
	bool          can_see_player( c_base_player* player, const Vector& pos );
	float         can_see_player(const Vector& pos);
	void update_client_side_animation( );
	float get_max_desync_delta();

	int& m_nMoveType( );


	QAngle * get_vangles( );

	float_t m_flSpawnTime( );
	std::string get_name();
	bool is_not_target( );

};

class c_player_resource
{
private:
	using int_65 = int[65];
public:
	//NETVAR(char[MAX_PLAYERS][16], GetClanTag, "CCSPlayerResource", "m_szClan");
	NETPROP( iteam, "DT_CSPlayerResource", "m_iTeam");
	NETVAR(int_65, iping, "DT_CSPlayerResource", "m_iPing");
};

class c_cs_player_resource : public c_player_resource
{
private:
	using int_65 = int[65];
	using char6516 = char[65][16];
	using str_32 = char[32];
	using unsigned65 = unsigned[65];

public:
	NETVAR(int_65, irank, "DT_CSPlayerResource", "m_iCompetitiveRanking");
	NETVAR(int_65, iwind, "DT_CSPlayerResource", "m_iCompetitiveWins");
	NETVAR(int_65, iteam_color, "DT_CSPlayerResource", "m_iCompTeammateColor");
	NETVAR(char6516, szclantag, "DT_CSPlayerResource", "m_szClan");
	NETVAR(unsigned65, ncoin, "DT_CSPlayerResource", "m_nActiveCoinRank");
	NETVAR(unsigned65, nmusik_kit, "DT_CSPlayerResource", "m_nMusicID");
};

class c_base_view_model : public c_base_entity
{
public:
	NETVAR( int32_t, m_nModelIndex, "DT_BaseViewModel", "m_nModelIndex" );
	NETVAR( int32_t, m_nViewModelIndex, "DT_BaseViewModel", "m_nViewModelIndex" );
	NETVAR( c_handle<c_base_combat_weapon>, m_hWeapon, "DT_BaseViewModel", "m_hWeapon" );
	NETVAR(c_handle<c_base_player>, m_hOwner, "DT_BaseViewModel", "m_hOwner" );
	NETPROP( m_nSequence, "DT_BaseViewModel", "m_nSequence" );
	void send_view_model_matching_sequence( int sequence );
};

class animation_layer
{
public:
	char  pad_0000[20];
	// These should also be present in the padding, don't see the use for it though
	//float	m_flLayerAnimtime;
	//float	m_flLayerFadeOuttime;
	uint32_t m_nOrder; //0x0014
	uint32_t m_nSequence; //0x0018
	float_t m_flPrevCycle; //0x001C
	float_t m_flWeight; //0x0020
	float_t m_flWeightDeltaRate; //0x0024
	float_t m_flPlaybackRate; //0x0028
	float_t m_flCycle; //0x002C
	void* m_pOwner; //0x0030 // player's thisptr
	char  pad_0038[4]; //0x0034
}; //Size: 0x0038

class c_csgo_player_animstate
{
public:
	/*void* pThis;
	char pad2[91];
	void* pBaseEntity; //0x60
	void* pActiveWeapon; //0x64
	void* pLastActiveWeapon; //0x68
	float m_flLastClientSideAnimationUpdateTime; //0x6C
	int m_iLastClientSideAnimationUpdateFramecount; //0x70
	float m_flEyePitch; //0x74
	float m_flEyeYaw; //0x78
	float m_flPitch; //0x7C
	float m_flGoalFeetYaw; //0x80
	float m_flSpeedFactor; //0x00FC
	float m_flDuckAmount; //0x00A4
	float m_flSpeedFraction; //0x00F8
	float m_flLandingRatio; //0x011C
	float m_flCurrentFeetYaw; //0x84
	float m_flCurrentTorsoYaw; //0x88
	float m_flUnknownVelocityLean; //0x8C //changes when moving/jumping/hitting ground
	float m_flLeanAmount; //0x90
	char pad4[4]; //NaN
	float m_flFeetCycle; //0x98 0 to 1
	float m_flFeetYawRate; //0x9C 0 to 1
	float m_fUnknown2;
	float m_fDuckAmount; //0xA4
	float m_fLandingDuckAdditiveSomething; //0xA8
	float m_fUnknown3; //0xAC
	Vector m_vOrigin; //0xB0, 0xB4, 0xB8
	Vector m_vLastOrigin; //0xBC, 0xC0, 0xC4
	float m_vVelocityX; //0xC8
	float m_vVelocityY; //0xCC
	char pad5[4];
	float m_flUnknownFloat1; //0xD4 Affected by movement and direction
	char pad6[8];
	float m_flUnknownFloat2; //0xE0 //from -1 to 1 when moving and affected by direction
	float m_flUnknownFloat3; //0xE4 //from -1 to 1 when moving and affected by direction
	float m_unknown; //0xE8
	float speed_2d; //0xEC
	float flUpVelocity; //0xF0
	float m_flSpeedNormalized; //0xF4 //from 0 to 1
	float m_flFeetSpeedForwardsOrSideWays; //0xF8 //from 0 to 2. something  is 1 when walking, 2.something when running, 0.653 when crouch walking
	float m_flFeetSpeedUnknownForwardOrSideways; //0xFC //from 0 to 3. something
	float m_flTimeSinceStartedMoving; //0x100
	float m_flTimeSinceStoppedMoving; //0x104
	unsigned char m_bOnGround; //0x108
	unsigned char m_bInHitGroundAnimation; //0x109
	char pad7[10];
	float m_flLastOriginZ; //0x114
	float m_flMinBodyYawDegrees; //0x0330
	float m_flMaxBodyYawDegrees; //0x0334
	float m_flHeadHeightOrOffsetFromHittingGroundAnimation; //0x118 from 0 to 1, is 1 when standing
	float m_flStopToFullRunningFraction; //0x11C from 0 to 1, doesnt change when walking or crouching, only running
	char pad8[4]; //NaN
	float m_flUnknownFraction; //0x124 affected while jumping and running, or when just jumping, 0 to 1
	char pad9[4]; //NaN
	float m_flUnknown3;
	char pad10[528];*/
	void* pThis;
	char pad2[91];
	void* pBaseEntity; //0x60
	void* pActiveWeapon; //0x64
	void* pLastActiveWeapon; //0x68
	float m_flLastClientSideAnimationUpdateTime; //0x6C
	int m_iLastClientSideAnimationUpdateFramecount; //0x70
	float m_flEyePitch; //0x74
	float m_flEyeYaw; //0x78
	float m_flPitch; //0x7C
	float m_flGoalFeetYaw; //0x80
	float m_flCurrentFeetYaw; //0x84
	float m_flCurrentTorsoYaw; //0x88
	float m_flUnknownVelocityLean; //0x8C //changes when moving/jumping/hitting ground
	float m_flLeanAmount; //0x90
	char pad4[4]; //NaN
	float m_flFeetCycle; //0x98 0 to 1
	float m_flFeetYawRate; //0x9C 0 to 1
	float m_fUnknown2;
	float m_fDuckAmount; //0xA4
	float m_fLandingDuckAdditiveSomething; //0xA8
	float m_fUnknown3; //0xAC
	Vector m_vOrigin; //0xB0, 0xB4, 0xB8
	Vector m_vLastOrigin; //0xBC, 0xC0, 0xC4
	float m_vVelocityX; //0xC8
	float m_vVelocityY; //0xCC
	char pad5[4];
	float m_flUnknownFloat1; //0xD4 Affected by movement and direction
	char pad6[8];
	float m_flUnknownFloat2; //0xE0 //from -1 to 1 when moving and affected by direction
	float m_flUnknownFloat3; //0xE4 //from -1 to 1 when moving and affected by direction
	float m_unknown; //0xE8
	float speed_2d; //0xEC
	float flUpVelocity; //0xF0
	float m_flSpeedNormalized; //0xF4 //from 0 to 1
	float m_flFeetSpeedForwardsOrSideWays; //0xF8 //from 0 to 2. something  is 1 when walking, 2.something when running, 0.653 when crouch walking
	float m_flFeetSpeedUnknownForwardOrSideways; //0xFC //from 0 to 3. something
	float m_flTimeSinceStartedMoving; //0x100
	float m_flTimeSinceStoppedMoving; //0x104
	unsigned char m_bOnGround; //0x108
	unsigned char m_bInHitGroundAnimation; //0x109
	char pad7[10];
	float m_flLastOriginZ; //0x114
	float m_flHeadHeightOrOffsetFromHittingGroundAnimation; //0x118 from 0 to 1, is 1 when standing
	float m_flStopToFullRunningFraction; //0x11C from 0 to 1, doesnt change when walking or crouching, only running
	char pad8[4]; //NaN
	float m_flUnknownFraction; //0x124 affected while jumping and running, or when just jumping, 0 to 1
	char pad9[4]; //NaN
	float m_flUnknown3;
	char pad10[528];
}; //Size=0x344

class dt_cs_player_resource
{
public:
	PNETVAR( int32_t, m_nActiveCoinRank, "DT_CSPlayerResource", "m_nActiveCoinRank" );
	PNETVAR( int32_t, m_nMusicID, "DT_CSPlayerResource", "m_nMusicID" );
	PNETVAR( int32_t, m_nPersonaDataPublicLevel, "DT_CSPlayerResource", "m_nPersonaDataPublicLevel" );
	PNETVAR( int32_t, m_nPersonaDataPublicCommendsLeader, "DT_CSPlayerResource", "m_nPersonaDataPublicCommendsLeader" );
	PNETVAR( int32_t, m_nPersonaDataPublicCommendsTeacher, "DT_CSPlayerResource", "m_nPersonaDataPublicCommendsTeacher" );
	PNETVAR( int32_t, m_nPersonaDataPublicCommendsFriendly, "DT_CSPlayerResource", "m_nPersonaDataPublicCommendsFriendly" );
	PNETVAR( int32_t, m_iCompetitiveRanking, "DT_CSPlayerResource", "m_iCompetitiveRanking" );
	PNETVAR( int32_t, m_iCompetitiveWins, "DT_CSPlayerResource", "m_iCompetitiveWins" );
	PNETVAR( int32_t, m_iPlayerVIP, "DT_CSPlayerResource", "m_iPlayerVIP" );
	PNETVAR( int32_t, m_iMVPs, "DT_CSPlayerResource", "m_iMVPs" );
	PNETVAR( int32_t, m_iScore, "DT_CSPlayerResource", "m_iScore" );
};

class c_game_rules {
public:
	NETVAR( int, m_iMatchStats_PlayersAlive_T, "DT_CSGameRulesProxy", "m_iMatchStats_PlayersAlive_T" );
	NETVAR( int, m_iMatchStats_PlayersAlive_CT, "DT_CSGameRulesProxy", "m_iMatchStats_PlayersAlive_CT" );
	NETVAR( int, m_iRoundTime, "DT_CSGameRulesProxy", "m_iRoundTime" );
	NETVAR( bool, m_bFreezePeriod, "DT_CSGameRulesProxy", "m_bBombDropped" );
	NETVAR( bool, m_bIsValveDS, "DT_CSGameRulesProxy", "m_bIsValveDS" );
	NETVAR( bool, m_bBombDropped, "DT_CSGameRulesProxy", "m_bBombDropped" );
	NETVAR( bool, m_bBombPlanted, "DT_CSGameRulesProxy", "m_bBombPlanted" );
	NETVAR( float, m_flSurvivalStartTime, "DT_CSGameRulesProxy", "m_flSurvivalStartTime" );
}; extern c_game_rules* g_gamerules;
