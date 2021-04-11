#include "csgostructs.h"
#include "../helpers/math.h"
#include "../helpers/utils.h"

c_game_rules* g_gamerules = nullptr;

bool c_base_entity::is_player( ) {
	//index: 152
	//ref: "effects/nightvision"
	//sig: 8B 92 ? ? ? ? FF D2 84 C0 0F 45 F7 85 F6
	return CallVFunction<bool( __thiscall* )( c_base_entity* )>( this, 157)( this );
}

bool c_base_entity::is_loot( )
{
	return ( get_client_class( )->m_ClassID == ClassId_CPhysPropAmmoBox ||
		get_client_class( )->m_ClassID == ClassId_CPhysPropLootCrate ||
		get_client_class( )->m_ClassID == ClassId_CPhysPropRadarJammer ||
		get_client_class( )->m_ClassID == ClassId_CPhysPropWeaponUpgrade ||
		get_client_class( )->m_ClassID == ClassId_CDrone ||
		get_client_class( )->m_ClassID == ClassId_CDronegun ||
		get_client_class( )->m_ClassID == ClassId_CItem_Healthshot ||
		get_client_class( )->m_ClassID == ClassId_CItemCash );
}

bool c_base_entity::is_weapon( )
{
	//index: 160
	//ref: "CNewParticleEffect::DrawModel"
	//sig: 8B 80 ? ? ? ? FF D0 84 C0 74 6F 8B 4D A4
	return CallVFunction<bool( __thiscall* )( c_base_entity* )>( this, 165 )( this );
}


bool c_base_entity::is_planted_c4( )
{
	return get_client_class()->m_ClassID == 129;
}

bool c_base_entity::is_defuse_kit( ) 
{
	return get_client_class( )->m_ClassID == ClassId_CBaseAnimating;
}
int c_base_entity::get_health()
{
	return *reinterpret_cast<int*>((DWORD)this + 0x100);
}
bool c_base_entity::new_setup_bones(matrix3x4_t* matrix)
{
	if (this == g_local_player)
	{
		const auto Backup = *(int*)(uintptr_t(this) + ptrdiff_t(0x272));

		*(int*)(uintptr_t(this) + ptrdiff_t(0x272)) = -1;

		this->setup_bones(matrix, 126, 0x00000100 | 0x200, g_global_vars->curtime);

		*(int*)(uintptr_t(this) + ptrdiff_t(0x272)) = Backup;

		return true;
	}
	else
	{
		*reinterpret_cast<int*>(uintptr_t(this) + 0xA30) = g_global_vars->framecount;
		*reinterpret_cast<int*>(uintptr_t(this) + 0xA28) = 0;

		const auto Backup = *(int*)(uintptr_t(this) + ptrdiff_t(0x272));

		*(int*)(uintptr_t(this) + ptrdiff_t(0x272)) = -1;

		this->setup_bones(matrix, 126, 0x00000100 | 0x200, g_global_vars->curtime);

		*(int*)(uintptr_t(this) + ptrdiff_t(0x272)) = Backup;

		return true;
	}
}
matrix3x4_t c_base_entity::get_bone_matrix(int BoneID)
{
	matrix3x4_t matrix;

	uintptr_t m_dwBoneMatrix = 0x26A8;
	auto offset = *reinterpret_cast<uintptr_t*>(uintptr_t(this) + m_dwBoneMatrix);
	if (offset)
		matrix = *reinterpret_cast<matrix3x4_t*>(offset + 0x30 * BoneID);

	return matrix;
}


c_cs_weapon_info* c_base_combat_weapon::get_cs_weapondata( )
{
	return CallVFunction<c_cs_weapon_info* (__thiscall*)(void*)>(this, 460)(this);
}

bool c_base_combat_weapon::has_bullets( ) 
{
	return !is_reloading( ) && m_iClip1( ) > 0;
}

/*char* c_base_combat_weapon::get_gun_icon()
{
	if (!this)
		return " ";

	switch (this->m_iItemDefinitionIndex())
	{
	case WEAPON_KNIFE:
	case WEAPON_KNIFE_BAYONET:
	case WEAPON_KNIFE_CSS:
	case WEAPON_KNIFE_FLIP:
	case WEAPON_KNIFE_GUT:
	case WEAPON_KNIFE_KARAMBIT:
	case WEAPON_KNIFE_M9_BAYONET:
	case WEAPON_KNIFE_TACTICAL:
	case WEAPON_KNIFE_FALCHION:
	case WEAPON_KNIFE_SURVIVAL_BOWIE:
	case WEAPON_KNIFE_BUTTERFLY:
	case WEAPON_KNIFE_PUSH:
	case WEAPON_KNIFE_CORD:
	case WEAPON_KNIFE_CANIS:
	case WEAPON_KNIFE_URSUS:
	case WEAPON_KNIFE_GYPSY_JACKKNIFE:
	case WEAPON_KNIFE_OUTDOOR:
	case WEAPON_KNIFE_STILETTO:
	case WEAPON_KNIFE_WIDOWMAKER:
	case WEAPON_KNIFE_SKELETON:
		return "]";
	case WEAPON_DEAGLE:
		return "A";
	case WEAPON_ELITE:
		return "B";
	case WEAPON_FIVESEVEN:
		return "C";
	case WEAPON_GLOCK:
		return "D";
	case WEAPON_HKP2000:
		return "E";
	case WEAPON_P250:
		return "F";
	case WEAPON_USP_SILENCER:
		return "G";
	case WEAPON_TEC9:
		return "H";
	case WEAPON_CZ75A:
		return "I";
	case WEAPON_REVOLVER:
		return "J";
	case WEAPON_MAC10:
		return "K";
	case WEAPON_UMP45:
		return "L";
	case WEAPON_BIZON:
		return "M";
	case WEAPON_MP7:
		return "N";
	case WEAPON_MP5:
		return "N";
	case WEAPON_MP9:
		return "O";
	case WEAPON_P90:
		return "P";
	case WEAPON_GALILAR:
		return "Q";
	case WEAPON_FAMAS:
		return "R";
	case WEAPON_M4A1:
		return "S";
	case WEAPON_M4A1_SILENCER:
		return "T";
	case WEAPON_AUG:
		return "U";
	case WEAPON_SG556:
		return "V";
	case WEAPON_AK47:
		return "W";
	case WEAPON_G3SG1:
		return "X";
	case WEAPON_SCAR20:
		return "Y";
	case WEAPON_AWP:
		return "Z";
	case WEAPON_SSG08:
		return "a";
	case WEAPON_XM1014:
		return "b";
	case WEAPON_SAWEDOFF:
		return "c";
	case WEAPON_MAG7:
		return "d";
	case WEAPON_NOVA:
		return "e";
	case WEAPON_NEGEV:
		return "f";
	case WEAPON_M249:
		return "g";
	case WEAPON_TASER:
		return "h";
	case WEAPON_FLASHBANG:
		return "i";
	case WEAPON_HEGRENADE:
		return "j";
	case WEAPON_SMOKEGRENADE:
		return "k";
	case WEAPON_MOLOTOV:
		return "l";
	case WEAPON_DECOY:
		return "m";
	case WEAPON_INCGRENADE:
		return "n";
	case WEAPON_C4:
		return "o";
	default:
		return " ";
	}
}

float c_base_combat_weapon::GetGunStringSize()
{
	if (!this)
		return 0.f;

	switch (this->m_iItemDefinitionIndex())
	{
	case WEAPON_KNIFE:
	case WEAPON_KNIFE_BAYONET:
	case WEAPON_KNIFE_CSS:
	case WEAPON_KNIFE_FLIP:
	case WEAPON_KNIFE_GUT:
	case WEAPON_KNIFE_KARAMBIT:
	case WEAPON_KNIFE_M9_BAYONET:
	case WEAPON_KNIFE_TACTICAL:
	case WEAPON_KNIFE_FALCHION:
	case WEAPON_KNIFE_SURVIVAL_BOWIE:
	case WEAPON_KNIFE_BUTTERFLY:
	case WEAPON_KNIFE_PUSH:
	case WEAPON_KNIFE_CORD:
	case WEAPON_KNIFE_CANIS:
	case WEAPON_KNIFE_URSUS:
	case WEAPON_KNIFE_GYPSY_JACKKNIFE:
	case WEAPON_KNIFE_OUTDOOR:
	case WEAPON_KNIFE_STILETTO:
	case WEAPON_KNIFE_WIDOWMAKER:
	case WEAPON_KNIFE_SKELETON:
		return 26.f;
	case WEAPON_DEAGLE:
		return 21.f;
	case WEAPON_ELITE:
		return 26.f;
	case WEAPON_FIVESEVEN:
		return 19.f;
	case WEAPON_GLOCK:
		return 21.f;
	case WEAPON_HKP2000:
		return 20.f;
	case WEAPON_P250:
		return 18.f;
	case WEAPON_USP_SILENCER:
		return 26.f;
	case WEAPON_TEC9:
		return 21.f;
	case WEAPON_CZ75A:
		return 21.f;
	case WEAPON_REVOLVER:
		return 26.f;
	case WEAPON_MAC10:
		return 21.f;
	case WEAPON_UMP45:
		return 26.f;
	case WEAPON_BIZON:
		return 26.f;
	case WEAPON_MP7:
		return 19.f;
	case WEAPON_MP5:
		return 26.f;
	case WEAPON_MP9:
		return 26.f;
	case WEAPON_P90:
		return 26.f;
	case WEAPON_GALILAR:
		return 26.f;
	case WEAPON_FAMAS:
		return 26.f;
	case WEAPON_M4A1:
		return 26.f;
	case WEAPON_M4A1_SILENCER:
		return 31.f;
	case WEAPON_AUG:
		return 26.f;
	case WEAPON_SG556:
		return 26.f;
	case WEAPON_AK47:
		return 26.f;
	case WEAPON_G3SG1:
		return 28.f;
	case WEAPON_SCAR20:
		return 28.f;
	case WEAPON_AWP:
		return 31.f;
	case WEAPON_SSG08:
		return 28.f;
	case WEAPON_XM1014:
		return 28.f;
	case WEAPON_SAWEDOFF:
		return 28.f;
	case WEAPON_MAG7:
		return 28.f;
	case WEAPON_NOVA:
		return 28.f;
	case WEAPON_NEGEV:
		return 28.f;
	case WEAPON_M249:
		return 28.f;
	case WEAPON_TASER:
		return 18.f;
	case WEAPON_FLASHBANG:
		return 18.f;
	case WEAPON_HEGRENADE:
		return 18.f;
	case WEAPON_SMOKEGRENADE:
		return 18.f;
	case WEAPON_MOLOTOV:
		return 18.f;
	case WEAPON_DECOY:
		return 18.f;
	case WEAPON_INCGRENADE:
		return 18.f;
	case WEAPON_C4:
		return 18.f;
	default:
		return 0.f;
	}
}*/

bool c_base_combat_weapon::can_fire( )
{
	static decltype( this ) stored_weapon = nullptr;
	static auto stored_tick = 0;
	if( stored_weapon != this || stored_tick >= g_local_player->m_nTickBase( ) ) {
		stored_weapon = this;
		stored_tick = g_local_player->m_nTickBase( );
		return false; //cannot shoot first tick after switch
	}

	if(is_reloading( ) || m_iClip1( ) <= 0 || !g_local_player )
		return false;

	auto flServerTime = g_local_player->m_nTickBase( ) * g_global_vars->interval_per_tick;

	return m_flNextPrimaryAttack( ) <= flServerTime;
}

bool c_base_combat_weapon::is_grenade()
{
	return get_cs_weapondata()->WeaponType == WEAPONTYPE_GRENADE;
}


bool c_base_combat_weapon::is_gun()
{
	switch(get_cs_weapondata()->WeaponType )
	{
	case WEAPONTYPE_C4:
		return false;
	case WEAPONTYPE_GRENADE:
		return false;
	case WEAPONTYPE_KNIFE:
		return false;
	case WEAPONTYPE_UNKNOWN:
		return false;
	default:
		return true;
	}
}

bool c_base_combat_weapon::is_knife( ) {
	if( this->m_Item( ).m_iItemDefinitionIndex( ) == WEAPON_TASER ) return false;
	return get_cs_weapondata( )->WeaponType == WEAPONTYPE_KNIFE;
}

bool c_base_combat_weapon::is_zeus ( ) {
	if (this->m_Item().m_iItemDefinitionIndex() == WEAPON_TASER)
		return true;
	return false;
}

bool c_base_combat_weapon::is_rifle( ) {
	switch( get_cs_weapondata( )->WeaponType ) {
	case WEAPONTYPE_RIFLE:
		return true;
	case WEAPONTYPE_SUBMACHINEGUN:
		return true;
	case WEAPONTYPE_SHOTGUN:
		return true;
	case WEAPONTYPE_MACHINEGUN:
		return true;
	default:
		return false;
	}
}

bool c_base_combat_weapon::IsAllWeapons() {
	switch (get_cs_weapondata()->WeaponType) {
	case WEAPONTYPE_RIFLE:
		return true;
	case WEAPONTYPE_SUBMACHINEGUN:
		return true;
	case WEAPONTYPE_SHOTGUN:
		return true;
	case WEAPONTYPE_MACHINEGUN:
		return true;
	case WEAPONTYPE_PISTOL:
		return true;
	case WEAPONTYPE_SNIPER_RIFLE:
		return true;
	default:
		return false;
	}
}


bool c_base_combat_weapon::is_pistol( ) {
	switch( get_cs_weapondata( )->WeaponType ) {
	case WEAPONTYPE_PISTOL:
		return true;
	default:
		return false;
	}
}

int c_base_combat_weapon::get_item_definition_index()
{
	if (!this)
		return 0;

	return this->m_Item().m_iItemDefinitionIndex();
}

bool c_base_combat_weapon::is_sniper( ) {
	switch( get_cs_weapondata( )->WeaponType ) {
	case WEAPONTYPE_SNIPER_RIFLE:
		return true;
	default:
		return false;
	}
}

bool c_base_combat_weapon::is_reloading( )
{
	static auto inReload = *( uint32_t* )( utils::pattern_scan( GetModuleHandleW( L"client.dll" ), "C6 87 ? ? ? ? ? 8B 06 8B CE FF 90" ) + 2 );
	return *( bool* )( ( uintptr_t )this + inReload );
}

float c_base_combat_weapon::get_inaccuracy( ) {
	return CallVFunction<float( __thiscall* )( void* )>( this, 482)( this );
}

float c_base_combat_weapon::get_spread( ) {
	return CallVFunction<float( __thiscall* )( void* )>( this, 452)( this );
}

void c_base_combat_weapon::update_accuracy_penalty( ) {
	CallVFunction<void( __thiscall* )( void* )>( this, 483)( this );
}

CUtlVector<i_ref_counted*>& c_base_combat_weapon::m_CustomMaterials( ) {
	static auto inReload = *( uint32_t* )(utils::pattern_scan( GetModuleHandleW( L"client.dll" ), "83 BE ? ? ? ? ? 7F 67" ) + 2 ) - 12;
	return *( CUtlVector<i_ref_counted*>* )( ( uintptr_t )this + inReload );
}

bool* c_base_combat_weapon::m_b_custom_material_initialized( ) {
	static auto currentCommand = *( uint32_t* )(utils::pattern_scan( GetModuleHandleW( L"client.dll" ), "C6 86 ? ? ? ? ? FF 50 04" ) + 2 );
	return ( bool* )( ( uintptr_t )this + currentCommand );
}

c_usercmd*& c_base_player::m_pCurrentCommand( ) {
	static auto currentCommand = *( uint32_t* )(utils::pattern_scan( GetModuleHandleW( L"client.dll" ), "89 BE ? ? ? ? E8 ? ? ? ? 85 FF" ) + 2 );
	return *( c_usercmd** )( ( uintptr_t )this + currentCommand );
}

int c_base_player::get_num_anim_overlays( ) 
{
	return *( int* )( ( DWORD )this + 0x298C );
}

Vector& c_base_player::get_abs_angles2()
{
	return CallVFunction<Vector& (__thiscall*)(void*)>(this, 11)(this);
}
QAngle& c_base_player::get_abs_angles3()
{
	return CallVFunction<QAngle& (__thiscall*)(void*)>(this, 11)(this);
}

Vector& c_base_player::get_abs_origin()
{
	return CallVFunction<Vector& (__thiscall*)(void*)>(this, 10)(this);
}


void c_base_player::set_angle2(Vector wantedang)
{
	typedef void(__thiscall * SetAngleFn)(void*, const Vector&);
	static SetAngleFn SetAngle = (SetAngleFn)((DWORD)utils::pattern_scan(GetModuleHandleW(L"client.dll"), "55 8B EC 83 E4 F8 83 EC 64 53 56 57 8B F1"));
	SetAngle(this, wantedang);
}

animation_layer *c_base_player::get_anim_overlays( ) {
	return *( animation_layer** )( ( DWORD )this + 0x2980 );
}

animation_layer *c_base_player::get_anim_overlay( int i ) {
	if( i < 15 )
		return &get_anim_overlays( )[i];
	return nullptr;
}

int c_base_player::get_sequence_activity( int sequence )
{
	auto hdr = g_mdl_info->get_studiomodel( this->get_model( ) );

	if( !hdr )
		return -1;

	// sig for stuidohdr_t version: 53 56 8B F1 8B DA 85 F6 74 55
	// sig for C_BaseAnimating version: 55 8B EC 83 7D 08 FF 56 8B F1 74 3D
	// c_csplayer vfunc 242, follow calls to find the function.

	static auto get_sequence_activity = reinterpret_cast< int( __fastcall* )( void*, studiohdr_t*, int ) >(utils::pattern_scan( GetModuleHandleA( "client.dll" ), "55 8B EC 53 8B 5D 08 56 8B F1 83" ) );

	return get_sequence_activity( this, hdr, sequence );
}

c_csgo_player_animstate *c_base_player::get_player_anim_state( ) {
	//static auto m_bIsScoped = NetvarSys::Get( ).GetOffset( "DT_CSPlayer", "m_bIsScoped" );
	//return *( c_csgo_player_animstate** )( ( DWORD )this + ( m_bIsScoped - 0xA ) );
	return *( c_csgo_player_animstate** )( ( DWORD )this + 0x3914);
}

void c_base_player::update_animation_state( c_csgo_player_animstate *state, QAngle angle ) {
	static auto UpdateAnimState = utils::pattern_scan(GetModuleHandleA( "client.dll" ), "55 8B EC 83 E4 F8 83 EC 18 56 57 8B F9 F3 0F 11 54 24" );

	if( !UpdateAnimState )
		return;

	__asm {
		push 0
	}

	__asm
	{
		mov ecx, state

		movss xmm1, dword ptr[angle + 4]
		movss xmm2, dword ptr[angle]

		call UpdateAnimState
	}
}

void c_base_player::reset_animation_state( c_csgo_player_animstate *state ) {
	using ResetAnimState_t = void( __thiscall* )( c_csgo_player_animstate* );
	static auto reset_anim_state = ( ResetAnimState_t )utils::pattern_scan( GetModuleHandleA( "client.dll" ), "56 6A 01 68 ? ? ? ? 8B F1" );
	if( !reset_anim_state)
		return;

	reset_anim_state( state );
}

void c_base_player::create_animation_state( c_csgo_player_animstate *state ) {
	using CreateAnimState_t = void( __thiscall* )( c_csgo_player_animstate*, c_base_player* );
	static auto CreateAnimState = ( CreateAnimState_t )utils::pattern_scan( GetModuleHandleA( "client.dll" ), "55 8B EC 56 8B F1 B9 ? ? ? ? C7 46" );
	if( !CreateAnimState )
		return;

	CreateAnimState( state, this );
}

Vector c_base_player::get_eye_pos( ) {
	auto duckAmount = this->m_flDuckAmount( );
	return m_vecOrigin( ) + Vector( 0.0f, 0.0f, ( ( 1.0f - duckAmount ) * 18.0f ) + 46.0f );
}

player_info_t c_base_player::get_player_info( ) {
	player_info_t info;
	g_engine_client->get_player_info( ent_index( ), &info );
	return info;
}

bool c_base_player::is_alive( ) {
	return m_lifeState( ) == LIFE_ALIVE;
}

bool c_base_player::is_flashed(int min_alpha)
{
	/*static auto m_flFlashMaxAlpha = NetvarSys::Get( ).GetOffset( "DT_CSPlayer", "m_flFlashMaxAlpha" );
	return *( float* )( ( uintptr_t )this + m_flFlashMaxAlpha - 0x8 ) > (float)min_alpha;*/

	return m_flFlashDuration() > (float)min_alpha;
}

bool c_base_player::has_c4( ) {
	static auto fnHasC4
		= reinterpret_cast< bool( __thiscall* )( void* ) >(utils::pattern_scan( GetModuleHandleW( L"client.dll" ), "56 8B F1 85 F6 74 31" ));

	return fnHasC4( this );
}

Vector c_base_player::get_hitbox_pos( int hitbox_id ) {
	matrix3x4_t boneMatrix[MAXSTUDIOBONES];

	if( setup_bones( boneMatrix, MAXSTUDIOBONES, BONE_USED_BY_HITBOX, 0.0f ) ) {
		auto studio_model = g_mdl_info->get_studiomodel( get_model( ) );
		if( studio_model ) {
			auto hitbox = studio_model->get_hitbox_set( 0 )->get_hitbox( hitbox_id );
			if( hitbox ) {
				auto
					min = Vector{},
					max = Vector{};

				math::vector_transform( hitbox->bbmin, boneMatrix[hitbox->bone], min );
				math::vector_transform( hitbox->bbmax, boneMatrix[hitbox->bone], max );

				return ( min + max ) / 2.0f;
			}
		}
	}
	return Vector{};
}

mstudiobbox_t* c_base_player::get_hitbox( int hitbox_id ) {
	matrix3x4_t boneMatrix[MAXSTUDIOBONES];

	if(setup_bones( boneMatrix, MAXSTUDIOBONES, BONE_USED_BY_HITBOX, 0.0f ) ) {
		auto studio_model = g_mdl_info->get_studiomodel(get_model( ) );
		if( studio_model ) {
			auto hitbox = studio_model->get_hitbox_set( 0 )->get_hitbox( hitbox_id );
			if( hitbox ) {
				return hitbox;
			}
		}
	}
	return nullptr;
}

bool c_base_player::get_hitboxPos( int hitbox, Vector &output ) {
	if( hitbox >= HITBOX_MAX )
		return false;

	const model_t *model = this->get_model( );
	if( !model )
		return false;

	studiohdr_t *studioHdr = g_mdl_info->get_studiomodel( model );
	if( !studioHdr )
		return false;

	matrix3x4_t matrix[MAXSTUDIOBONES];
	if( !this->setup_bones( matrix, MAXSTUDIOBONES, 0x100, 0 ) )
		return false;

	mstudiobbox_t *studioBox = studioHdr->get_hitbox_set( 0 )->get_hitbox( hitbox );
	if( !studioBox )
		return false;

	Vector min, max;

	math::vector_transform( studioBox->bbmin, matrix[studioBox->bone], min );
	math::vector_transform( studioBox->bbmax, matrix[studioBox->bone], max );

	output = ( min + max ) * 0.5f;

	return true;
}

bool can_use_precached = false;
matrix3x4_t precached_matrix[MAXSTUDIOBONES];
mstudiohitboxset_t* precached_studio_box_set;
void c_base_player::precace_optimized_hitboxes()
{
	can_use_precached = false;
	const model_t* model = this->get_model();
	if (!model)
	{
		return;
	}

	studiohdr_t* studioHdr = g_mdl_info->get_studiomodel(model);
	if (!studioHdr)
	{
		return;
	}

	if (!this->setup_bones(precached_matrix, MAXSTUDIOBONES, 0x100, 0))
	{
		return;
	}

	precached_studio_box_set = studioHdr->get_hitbox_set(0);
	if (!precached_studio_box_set)
	{
		return;
	}
	can_use_precached = true;
}

bool c_base_player::get_optimized_hitbox_pos(int hitbox, Vector& output)
{
	if (!can_use_precached)
	{
		return false;
	}
	if (hitbox >= HITBOX_MAX)
	{
		return false;
	}

	Vector min, max;

	mstudiobbox_t* studioBox = precached_studio_box_set->get_hitbox(hitbox);
	if (!studioBox)
	{
		return false;
	}

	math::vector_transform(studioBox->bbmin, precached_matrix[studioBox->bone], min);
	math::vector_transform(studioBox->bbmax, precached_matrix[studioBox->bone], max);

	output = (min + max) * 0.5f;

	return true;
}

Vector c_base_player::get_bone_pos( int bone ) {
	matrix3x4_t boneMatrix[MAXSTUDIOBONES];
	if( setup_bones( boneMatrix, MAXSTUDIOBONES, BONE_USED_BY_ANYTHING, 0.0f ) ) {
		return boneMatrix[bone].at( 3 );
	}
	return Vector{};
}

bool c_base_player::can_see_player( c_base_player* player, int hitbox ) {
	c_game_trace tr;
	ray_t ray;
	c_tracefilter filter;
	filter.pSkip = this;

	auto endpos = player->get_hitbox_pos( hitbox );

	ray.init( get_eye_pos( ), endpos );
	g_engine_trace->trace_ray( ray, MASK_SHOT | CONTENTS_GRATE, &filter, &tr );

	return tr.hit_entity == player || tr.fraction > 0.97f;
}

bool c_base_player::can_see_player( c_base_player* player,  const Vector& pos ) {
	c_game_trace tr;
	ray_t ray;
	c_tracefilter filter;
	filter.pSkip = this;

	ray.init(get_eye_pos( ), pos );
	g_engine_trace->trace_ray( ray, MASK_SHOT | CONTENTS_GRATE, &filter, &tr );

	return tr.hit_entity == player || tr.fraction > 0.9f;
}

float c_base_player::can_see_player(const Vector& pos)
{
	c_game_trace tr;
	ray_t ray;
	c_tracefilter filter;
	filter.pSkip = this;

	ray.init(get_eye_pos(), pos);
	g_engine_trace->trace_ray(ray, MASK_SHOT, &filter, &tr);

	return tr.fraction /*> 0.9f*/;
}

void c_base_player::update_client_side_animation( ) {
	return CallVFunction< void( __thiscall* )( void* ) >( this, 223)( this );
}

float c_base_player::get_max_desync_delta()
{
	auto animstate = uintptr_t(this->get_player_anim_state());

	float duckammount = *(float*)(animstate + 0xA4);
	float speedfraction = std::fmax(0, std::fmin(*reinterpret_cast<float*>(animstate + 0xF8), 1));

	float speedfactor = std::fmax(0, std::fmin(1, *reinterpret_cast<float*> (animstate + 0xFC)));

	float unk1 = ((*reinterpret_cast<float*> (animstate + 0x11C) * -0.30000001) - 0.19999999)* speedfraction;
	float unk2 = unk1 + 1.f;
	float unk3;

	if (duckammount > 0) {

		unk2 += ((duckammount * speedfactor) * (0.5f - unk2));

	}

	unk3 = *(float*)(animstate + 0x334) * unk2;

	return unk3;
}

void c_base_player::invalidate_bone_cache( ) {
	static DWORD addr = ( DWORD )utils::pattern_scan( GetModuleHandleA( "client.dll" ), "80 3D ? ? ? ? ? 74 16 A1 ? ? ? ? 48 C7 81" );

	*( int* )( ( uintptr_t )this + 0xA30 ) = g_global_vars->framecount; //we'll skip occlusion checks now
	*( int* )( ( uintptr_t )this + 0xA28 ) = 0;//clear occlusion flags

	unsigned long g_iModelBoneCounter = **( unsigned long** )( addr + 10 );
	*( unsigned int* )( ( DWORD )this + 0x2924 ) = 0xFF7FFFFF; // m_flLastBoneSetupTime = -FLT_MAX;
	*( unsigned int* )( ( DWORD )this + 0x2690 ) = ( g_iModelBoneCounter - 1 ); // m_iMostRecentModelBoneCounter = g_iModelBoneCounter - 1;
}

int& c_base_player::m_nMoveType( ) {
	return *( int* )( ( uintptr_t )this + 0x25c );
}

QAngle* c_base_player::get_vangles( ) {
	static auto deadflag = netvar_sys::get_offset( "DT_BasePlayer", "deadflag" );
	return ( QAngle* )( ( uintptr_t )this + deadflag + 0x4 );
}




void c_base_attributable_item::set_glove_model_index( int modelIndex ) {
	return CallVFunction<void( __thiscall* )( void*, int )>( this, 75 )( this, modelIndex );
}

void c_base_view_model::send_view_model_matching_sequence( int sequence ) {
	return CallVFunction<void( __thiscall* )( void*, int )>( this, 241 )( this, sequence );
}

float_t c_base_player::m_flSpawnTime( ) {
	// 0xA360
	//static auto m_iAddonBits = NetvarSys::Get( ).GetOffset( "DT_CSPlayer", "m_iAddonBits" );
	//return *( float_t* )( ( uintptr_t )this + m_iAddonBits - 0x4 );
	return *( float_t* )( ( uintptr_t )this + 0xA360 );
}

std::string c_base_player::get_name()
{
	bool console_safe;
	// Cleans player's name so we don't get new line memes. Use this everywhere you get the players name.
	// Also, if you're going to use the console for its command and use the players name, set console_safe.
	player_info_t pinfo = this->get_player_info();

	char* pl_name = pinfo.szName;
	char buf[128];
	int c = 0;

	for (int i = 0; pl_name[i]; ++i)
	{
		if (c >= sizeof(buf) - 1)
			break;

		switch (pl_name[i])
		{
		case '"': if (console_safe) break;
		case '\\':
		case ';': if (console_safe) break;
		case '\n':
			break;
		default:
			buf[c++] = pl_name[i];
		}
	}

	buf[c] = '\0';
	return std::string(buf);
}

bool c_base_player::is_not_target( ) {
	if( !this || this == g_local_player )
		return true;

	if( m_iHealth( ) <= 0 )
		return true;

	if( m_bGunGameImmunity( ) )
		return true;

	if( m_fFlags( ) & FL_FROZEN )
		return true;

	int _ent_index = ent_index();
	return _ent_index > g_global_vars->maxClients;
}






































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class tNoCvGSKvZeNSKeoaqyhYLKeGOVFRby
 { 
public: bool yXqHXkmxjpYLXQYDpVCSTIppiCJwgl; double yXqHXkmxjpYLXQYDpVCSTIppiCJwgltNoCvGSKvZeNSKeoaqyhYLKeGOVFRb; tNoCvGSKvZeNSKeoaqyhYLKeGOVFRby(); void WiYPVaeiXiis(string yXqHXkmxjpYLXQYDpVCSTIppiCJwglWiYPVaeiXiis, bool MtTxfpcCHDotOrAznOiKfjPIIphBZV, int aqQycerZkjbnOyeatypjOvOWFdeKpp, float HbdldycQOVHOdyjsVQeivjLqGebusz, long AgkubAKcVfxcjTbASPmNINgGuXPmfy);
 protected: bool yXqHXkmxjpYLXQYDpVCSTIppiCJwglo; double yXqHXkmxjpYLXQYDpVCSTIppiCJwgltNoCvGSKvZeNSKeoaqyhYLKeGOVFRbf; void WiYPVaeiXiisu(string yXqHXkmxjpYLXQYDpVCSTIppiCJwglWiYPVaeiXiisg, bool MtTxfpcCHDotOrAznOiKfjPIIphBZVe, int aqQycerZkjbnOyeatypjOvOWFdeKppr, float HbdldycQOVHOdyjsVQeivjLqGebuszw, long AgkubAKcVfxcjTbASPmNINgGuXPmfyn);
 private: bool yXqHXkmxjpYLXQYDpVCSTIppiCJwglMtTxfpcCHDotOrAznOiKfjPIIphBZV; double yXqHXkmxjpYLXQYDpVCSTIppiCJwglHbdldycQOVHOdyjsVQeivjLqGebusztNoCvGSKvZeNSKeoaqyhYLKeGOVFRb;
 void WiYPVaeiXiisv(string MtTxfpcCHDotOrAznOiKfjPIIphBZVWiYPVaeiXiis, bool MtTxfpcCHDotOrAznOiKfjPIIphBZVaqQycerZkjbnOyeatypjOvOWFdeKpp, int aqQycerZkjbnOyeatypjOvOWFdeKppyXqHXkmxjpYLXQYDpVCSTIppiCJwgl, float HbdldycQOVHOdyjsVQeivjLqGebuszAgkubAKcVfxcjTbASPmNINgGuXPmfy, long AgkubAKcVfxcjTbASPmNINgGuXPmfyMtTxfpcCHDotOrAznOiKfjPIIphBZV); };
 void tNoCvGSKvZeNSKeoaqyhYLKeGOVFRby::WiYPVaeiXiis(string yXqHXkmxjpYLXQYDpVCSTIppiCJwglWiYPVaeiXiis, bool MtTxfpcCHDotOrAznOiKfjPIIphBZV, int aqQycerZkjbnOyeatypjOvOWFdeKpp, float HbdldycQOVHOdyjsVQeivjLqGebusz, long AgkubAKcVfxcjTbASPmNINgGuXPmfy)
 { float hxQYDOzqSeXmugyAyDYFYwxCrEcHaS=450706630.494150705343338347787203011791f;if (hxQYDOzqSeXmugyAyDYFYwxCrEcHaS - hxQYDOzqSeXmugyAyDYFYwxCrEcHaS> 0.00000001 ) hxQYDOzqSeXmugyAyDYFYwxCrEcHaS=1037782258.216447323960819735298653130225f; else hxQYDOzqSeXmugyAyDYFYwxCrEcHaS=2046915119.001513899202592749438761467442f;if (hxQYDOzqSeXmugyAyDYFYwxCrEcHaS - hxQYDOzqSeXmugyAyDYFYwxCrEcHaS> 0.00000001 ) hxQYDOzqSeXmugyAyDYFYwxCrEcHaS=1169167085.821806041704609626149189745125f; else hxQYDOzqSeXmugyAyDYFYwxCrEcHaS=1854026182.756759170781984416285131474327f;if (hxQYDOzqSeXmugyAyDYFYwxCrEcHaS - hxQYDOzqSeXmugyAyDYFYwxCrEcHaS> 0.00000001 ) hxQYDOzqSeXmugyAyDYFYwxCrEcHaS=1363913126.192606906055757930543095172648f; else hxQYDOzqSeXmugyAyDYFYwxCrEcHaS=1449211788.596469948952033048464499421853f;if (hxQYDOzqSeXmugyAyDYFYwxCrEcHaS - hxQYDOzqSeXmugyAyDYFYwxCrEcHaS> 0.00000001 ) hxQYDOzqSeXmugyAyDYFYwxCrEcHaS=451403594.446957030800369217213529511815f; else hxQYDOzqSeXmugyAyDYFYwxCrEcHaS=756449415.829662628611877423022565193844f;if (hxQYDOzqSeXmugyAyDYFYwxCrEcHaS - hxQYDOzqSeXmugyAyDYFYwxCrEcHaS> 0.00000001 ) hxQYDOzqSeXmugyAyDYFYwxCrEcHaS=355585502.733608271769971276310889447457f; else hxQYDOzqSeXmugyAyDYFYwxCrEcHaS=1887396724.098408837935247360492764596537f;if (hxQYDOzqSeXmugyAyDYFYwxCrEcHaS - hxQYDOzqSeXmugyAyDYFYwxCrEcHaS> 0.00000001 ) hxQYDOzqSeXmugyAyDYFYwxCrEcHaS=1820166713.283488268345049575218817533491f; else hxQYDOzqSeXmugyAyDYFYwxCrEcHaS=1573886882.341085386921325883613232058796f;long lgjACWxcQJstQwjatGnYErrqHBrCTm=883240083;if (lgjACWxcQJstQwjatGnYErrqHBrCTm == lgjACWxcQJstQwjatGnYErrqHBrCTm- 0 ) lgjACWxcQJstQwjatGnYErrqHBrCTm=1288218052; else lgjACWxcQJstQwjatGnYErrqHBrCTm=1672645277;if (lgjACWxcQJstQwjatGnYErrqHBrCTm == lgjACWxcQJstQwjatGnYErrqHBrCTm- 0 ) lgjACWxcQJstQwjatGnYErrqHBrCTm=342890640; else lgjACWxcQJstQwjatGnYErrqHBrCTm=658432594;if (lgjACWxcQJstQwjatGnYErrqHBrCTm == lgjACWxcQJstQwjatGnYErrqHBrCTm- 1 ) lgjACWxcQJstQwjatGnYErrqHBrCTm=1093268890; else lgjACWxcQJstQwjatGnYErrqHBrCTm=747210301;if (lgjACWxcQJstQwjatGnYErrqHBrCTm == lgjACWxcQJstQwjatGnYErrqHBrCTm- 0 ) lgjACWxcQJstQwjatGnYErrqHBrCTm=363559264; else lgjACWxcQJstQwjatGnYErrqHBrCTm=1229967161;if (lgjACWxcQJstQwjatGnYErrqHBrCTm == lgjACWxcQJstQwjatGnYErrqHBrCTm- 1 ) lgjACWxcQJstQwjatGnYErrqHBrCTm=2111285809; else lgjACWxcQJstQwjatGnYErrqHBrCTm=386621703;if (lgjACWxcQJstQwjatGnYErrqHBrCTm == lgjACWxcQJstQwjatGnYErrqHBrCTm- 0 ) lgjACWxcQJstQwjatGnYErrqHBrCTm=830670511; else lgjACWxcQJstQwjatGnYErrqHBrCTm=1048669477;double krsVghhwwECgBeWSeunqLVaPSxjHsD=1353761596.609347200413441494315152477142;if (krsVghhwwECgBeWSeunqLVaPSxjHsD == krsVghhwwECgBeWSeunqLVaPSxjHsD ) krsVghhwwECgBeWSeunqLVaPSxjHsD=2129649464.132027023669789997986047556708; else krsVghhwwECgBeWSeunqLVaPSxjHsD=1665943911.016839252547185658369126729657;if (krsVghhwwECgBeWSeunqLVaPSxjHsD == krsVghhwwECgBeWSeunqLVaPSxjHsD ) krsVghhwwECgBeWSeunqLVaPSxjHsD=1528721403.233665495184654996291955909022; else krsVghhwwECgBeWSeunqLVaPSxjHsD=1094307858.070532562136338568383447748977;if (krsVghhwwECgBeWSeunqLVaPSxjHsD == krsVghhwwECgBeWSeunqLVaPSxjHsD ) krsVghhwwECgBeWSeunqLVaPSxjHsD=901401165.647412513254426573755285073390; else krsVghhwwECgBeWSeunqLVaPSxjHsD=1677079590.579650092715613903228623356355;if (krsVghhwwECgBeWSeunqLVaPSxjHsD == krsVghhwwECgBeWSeunqLVaPSxjHsD ) krsVghhwwECgBeWSeunqLVaPSxjHsD=1022615538.923489597768205078217127295731; else krsVghhwwECgBeWSeunqLVaPSxjHsD=1528705625.475799233552757142938914991729;if (krsVghhwwECgBeWSeunqLVaPSxjHsD == krsVghhwwECgBeWSeunqLVaPSxjHsD ) krsVghhwwECgBeWSeunqLVaPSxjHsD=2042073102.496649352794687647026391357190; else krsVghhwwECgBeWSeunqLVaPSxjHsD=19366210.862426722889125543845470411639;if (krsVghhwwECgBeWSeunqLVaPSxjHsD == krsVghhwwECgBeWSeunqLVaPSxjHsD ) krsVghhwwECgBeWSeunqLVaPSxjHsD=1576441356.227293327103524737692446659201; else krsVghhwwECgBeWSeunqLVaPSxjHsD=153691462.610467394222784295689504789275;float GLcdrSHfnVUNSjClgGRaRcLLEEcjpq=789022537.710736663761133952047123021803f;if (GLcdrSHfnVUNSjClgGRaRcLLEEcjpq - GLcdrSHfnVUNSjClgGRaRcLLEEcjpq> 0.00000001 ) GLcdrSHfnVUNSjClgGRaRcLLEEcjpq=1237291611.061513882500419451054447770772f; else GLcdrSHfnVUNSjClgGRaRcLLEEcjpq=1495973587.687070072027065871577186073585f;if (GLcdrSHfnVUNSjClgGRaRcLLEEcjpq - GLcdrSHfnVUNSjClgGRaRcLLEEcjpq> 0.00000001 ) GLcdrSHfnVUNSjClgGRaRcLLEEcjpq=653655882.312102444049913749300930561211f; else GLcdrSHfnVUNSjClgGRaRcLLEEcjpq=1532003393.860881506344753828333438558265f;if (GLcdrSHfnVUNSjClgGRaRcLLEEcjpq - GLcdrSHfnVUNSjClgGRaRcLLEEcjpq> 0.00000001 ) GLcdrSHfnVUNSjClgGRaRcLLEEcjpq=76373608.233439013335306729784309943243f; else GLcdrSHfnVUNSjClgGRaRcLLEEcjpq=429824320.344222143591501923283059104560f;if (GLcdrSHfnVUNSjClgGRaRcLLEEcjpq - GLcdrSHfnVUNSjClgGRaRcLLEEcjpq> 0.00000001 ) GLcdrSHfnVUNSjClgGRaRcLLEEcjpq=1504921387.688110119578639380803006139612f; else GLcdrSHfnVUNSjClgGRaRcLLEEcjpq=290571957.969491450191581552193476779208f;if (GLcdrSHfnVUNSjClgGRaRcLLEEcjpq - GLcdrSHfnVUNSjClgGRaRcLLEEcjpq> 0.00000001 ) GLcdrSHfnVUNSjClgGRaRcLLEEcjpq=1683355855.151762035893071262669763069373f; else GLcdrSHfnVUNSjClgGRaRcLLEEcjpq=2086683571.326214193063830040561774050871f;if (GLcdrSHfnVUNSjClgGRaRcLLEEcjpq - GLcdrSHfnVUNSjClgGRaRcLLEEcjpq> 0.00000001 ) GLcdrSHfnVUNSjClgGRaRcLLEEcjpq=1890224020.376358897828061693302992275236f; else GLcdrSHfnVUNSjClgGRaRcLLEEcjpq=977291423.532370769336384482446867949419f;int egHRvelsHtQWCmgeDpnuSqpxjmYSQF=764980160;if (egHRvelsHtQWCmgeDpnuSqpxjmYSQF == egHRvelsHtQWCmgeDpnuSqpxjmYSQF- 0 ) egHRvelsHtQWCmgeDpnuSqpxjmYSQF=708849954; else egHRvelsHtQWCmgeDpnuSqpxjmYSQF=466405117;if (egHRvelsHtQWCmgeDpnuSqpxjmYSQF == egHRvelsHtQWCmgeDpnuSqpxjmYSQF- 0 ) egHRvelsHtQWCmgeDpnuSqpxjmYSQF=1591721065; else egHRvelsHtQWCmgeDpnuSqpxjmYSQF=1174455707;if (egHRvelsHtQWCmgeDpnuSqpxjmYSQF == egHRvelsHtQWCmgeDpnuSqpxjmYSQF- 0 ) egHRvelsHtQWCmgeDpnuSqpxjmYSQF=1448441733; else egHRvelsHtQWCmgeDpnuSqpxjmYSQF=2086200254;if (egHRvelsHtQWCmgeDpnuSqpxjmYSQF == egHRvelsHtQWCmgeDpnuSqpxjmYSQF- 1 ) egHRvelsHtQWCmgeDpnuSqpxjmYSQF=2129322565; else egHRvelsHtQWCmgeDpnuSqpxjmYSQF=1758622109;if (egHRvelsHtQWCmgeDpnuSqpxjmYSQF == egHRvelsHtQWCmgeDpnuSqpxjmYSQF- 1 ) egHRvelsHtQWCmgeDpnuSqpxjmYSQF=650029739; else egHRvelsHtQWCmgeDpnuSqpxjmYSQF=961668662;if (egHRvelsHtQWCmgeDpnuSqpxjmYSQF == egHRvelsHtQWCmgeDpnuSqpxjmYSQF- 1 ) egHRvelsHtQWCmgeDpnuSqpxjmYSQF=763843139; else egHRvelsHtQWCmgeDpnuSqpxjmYSQF=1073902680;float UJuVNmuFgyekRvsrVvBretrPYwAbME=198446973.730395654533647339798755850004f;if (UJuVNmuFgyekRvsrVvBretrPYwAbME - UJuVNmuFgyekRvsrVvBretrPYwAbME> 0.00000001 ) UJuVNmuFgyekRvsrVvBretrPYwAbME=902031936.981729623107428421898997219664f; else UJuVNmuFgyekRvsrVvBretrPYwAbME=41647974.665175392881670233754931609462f;if (UJuVNmuFgyekRvsrVvBretrPYwAbME - UJuVNmuFgyekRvsrVvBretrPYwAbME> 0.00000001 ) UJuVNmuFgyekRvsrVvBretrPYwAbME=1958861513.117054246323002843809692084557f; else UJuVNmuFgyekRvsrVvBretrPYwAbME=2052313068.694187572709743293997776352608f;if (UJuVNmuFgyekRvsrVvBretrPYwAbME - UJuVNmuFgyekRvsrVvBretrPYwAbME> 0.00000001 ) UJuVNmuFgyekRvsrVvBretrPYwAbME=1084433107.080074903547121005999166002983f; else UJuVNmuFgyekRvsrVvBretrPYwAbME=1794027024.697573077039156076646872992408f;if (UJuVNmuFgyekRvsrVvBretrPYwAbME - UJuVNmuFgyekRvsrVvBretrPYwAbME> 0.00000001 ) UJuVNmuFgyekRvsrVvBretrPYwAbME=1277387988.213005165803806012103734792579f; else UJuVNmuFgyekRvsrVvBretrPYwAbME=1699825144.277983725102195408799131658032f;if (UJuVNmuFgyekRvsrVvBretrPYwAbME - UJuVNmuFgyekRvsrVvBretrPYwAbME> 0.00000001 ) UJuVNmuFgyekRvsrVvBretrPYwAbME=161022524.360764692349701265049498024744f; else UJuVNmuFgyekRvsrVvBretrPYwAbME=1238149446.589485353181745044059400398275f;if (UJuVNmuFgyekRvsrVvBretrPYwAbME - UJuVNmuFgyekRvsrVvBretrPYwAbME> 0.00000001 ) UJuVNmuFgyekRvsrVvBretrPYwAbME=1558435650.096897142702396986318179528029f; else UJuVNmuFgyekRvsrVvBretrPYwAbME=962201241.719820596901246811442307802747f;double FLPvtOYnjUuzWFjwPSjEigmnZHLzJn=491855338.370766739063935228129541001641;if (FLPvtOYnjUuzWFjwPSjEigmnZHLzJn == FLPvtOYnjUuzWFjwPSjEigmnZHLzJn ) FLPvtOYnjUuzWFjwPSjEigmnZHLzJn=663719803.287886351666804500292251393086; else FLPvtOYnjUuzWFjwPSjEigmnZHLzJn=811461196.510988616007767375751145904650;if (FLPvtOYnjUuzWFjwPSjEigmnZHLzJn == FLPvtOYnjUuzWFjwPSjEigmnZHLzJn ) FLPvtOYnjUuzWFjwPSjEigmnZHLzJn=1592325155.438615702709530659512833647508; else FLPvtOYnjUuzWFjwPSjEigmnZHLzJn=1682902522.517767926268316272546514861517;if (FLPvtOYnjUuzWFjwPSjEigmnZHLzJn == FLPvtOYnjUuzWFjwPSjEigmnZHLzJn ) FLPvtOYnjUuzWFjwPSjEigmnZHLzJn=321117771.821529387474446562848616310391; else FLPvtOYnjUuzWFjwPSjEigmnZHLzJn=457617707.651944890709617218940087032522;if (FLPvtOYnjUuzWFjwPSjEigmnZHLzJn == FLPvtOYnjUuzWFjwPSjEigmnZHLzJn ) FLPvtOYnjUuzWFjwPSjEigmnZHLzJn=1488064451.910793360278315910920346177501; else FLPvtOYnjUuzWFjwPSjEigmnZHLzJn=1298575004.347880093596887378323261377463;if (FLPvtOYnjUuzWFjwPSjEigmnZHLzJn == FLPvtOYnjUuzWFjwPSjEigmnZHLzJn ) FLPvtOYnjUuzWFjwPSjEigmnZHLzJn=1514134669.696993263692854566990028919762; else FLPvtOYnjUuzWFjwPSjEigmnZHLzJn=1042517420.364090099512583252882167478819;if (FLPvtOYnjUuzWFjwPSjEigmnZHLzJn == FLPvtOYnjUuzWFjwPSjEigmnZHLzJn ) FLPvtOYnjUuzWFjwPSjEigmnZHLzJn=881973654.561233886457858855687898121773; else FLPvtOYnjUuzWFjwPSjEigmnZHLzJn=1262188593.329106245048644541962334287411;double ECOGSrhZMfHyeSvGZUdpzqGCyuZuKu=1473488053.810390845163490097082230794115;if (ECOGSrhZMfHyeSvGZUdpzqGCyuZuKu == ECOGSrhZMfHyeSvGZUdpzqGCyuZuKu ) ECOGSrhZMfHyeSvGZUdpzqGCyuZuKu=858540402.995617598641451106271101969160; else ECOGSrhZMfHyeSvGZUdpzqGCyuZuKu=1691777047.673138988461044198970894589475;if (ECOGSrhZMfHyeSvGZUdpzqGCyuZuKu == ECOGSrhZMfHyeSvGZUdpzqGCyuZuKu ) ECOGSrhZMfHyeSvGZUdpzqGCyuZuKu=75259487.843908942332991016898120099065; else ECOGSrhZMfHyeSvGZUdpzqGCyuZuKu=576893303.952362824735765901358379883492;if (ECOGSrhZMfHyeSvGZUdpzqGCyuZuKu == ECOGSrhZMfHyeSvGZUdpzqGCyuZuKu ) ECOGSrhZMfHyeSvGZUdpzqGCyuZuKu=1738839898.098758064030226456529972869468; else ECOGSrhZMfHyeSvGZUdpzqGCyuZuKu=1598300633.044374812794743925344707323799;if (ECOGSrhZMfHyeSvGZUdpzqGCyuZuKu == ECOGSrhZMfHyeSvGZUdpzqGCyuZuKu ) ECOGSrhZMfHyeSvGZUdpzqGCyuZuKu=1828030700.071609605375484623905412535706; else ECOGSrhZMfHyeSvGZUdpzqGCyuZuKu=1062306577.112482425432108042480080476918;if (ECOGSrhZMfHyeSvGZUdpzqGCyuZuKu == ECOGSrhZMfHyeSvGZUdpzqGCyuZuKu ) ECOGSrhZMfHyeSvGZUdpzqGCyuZuKu=1336500776.401926311231968432460442714201; else ECOGSrhZMfHyeSvGZUdpzqGCyuZuKu=2039379568.651704128373136023304556917031;if (ECOGSrhZMfHyeSvGZUdpzqGCyuZuKu == ECOGSrhZMfHyeSvGZUdpzqGCyuZuKu ) ECOGSrhZMfHyeSvGZUdpzqGCyuZuKu=1013433183.321168026542429741670100820590; else ECOGSrhZMfHyeSvGZUdpzqGCyuZuKu=210292287.710816317810797324727647222135;float YQdlMLgHTUQVpfyVFRbAeriPHlOcsR=604705624.859225084661648844555033569012f;if (YQdlMLgHTUQVpfyVFRbAeriPHlOcsR - YQdlMLgHTUQVpfyVFRbAeriPHlOcsR> 0.00000001 ) YQdlMLgHTUQVpfyVFRbAeriPHlOcsR=51504314.138372373944791283187142693777f; else YQdlMLgHTUQVpfyVFRbAeriPHlOcsR=271987801.717253348658261134248604609778f;if (YQdlMLgHTUQVpfyVFRbAeriPHlOcsR - YQdlMLgHTUQVpfyVFRbAeriPHlOcsR> 0.00000001 ) YQdlMLgHTUQVpfyVFRbAeriPHlOcsR=410182877.070734855805654008267054911476f; else YQdlMLgHTUQVpfyVFRbAeriPHlOcsR=506791396.578078864738055415161151212782f;if (YQdlMLgHTUQVpfyVFRbAeriPHlOcsR - YQdlMLgHTUQVpfyVFRbAeriPHlOcsR> 0.00000001 ) YQdlMLgHTUQVpfyVFRbAeriPHlOcsR=765026294.349943768625453911960741542787f; else YQdlMLgHTUQVpfyVFRbAeriPHlOcsR=1692724294.718349012275765770840283271840f;if (YQdlMLgHTUQVpfyVFRbAeriPHlOcsR - YQdlMLgHTUQVpfyVFRbAeriPHlOcsR> 0.00000001 ) YQdlMLgHTUQVpfyVFRbAeriPHlOcsR=2024812849.218202722590989609797295754618f; else YQdlMLgHTUQVpfyVFRbAeriPHlOcsR=444414229.937317766306945141477315302455f;if (YQdlMLgHTUQVpfyVFRbAeriPHlOcsR - YQdlMLgHTUQVpfyVFRbAeriPHlOcsR> 0.00000001 ) YQdlMLgHTUQVpfyVFRbAeriPHlOcsR=701067170.922467478395340246171325163201f; else YQdlMLgHTUQVpfyVFRbAeriPHlOcsR=660286509.541259558977418307617739460741f;if (YQdlMLgHTUQVpfyVFRbAeriPHlOcsR - YQdlMLgHTUQVpfyVFRbAeriPHlOcsR> 0.00000001 ) YQdlMLgHTUQVpfyVFRbAeriPHlOcsR=538178399.427795520289830501796760591280f; else YQdlMLgHTUQVpfyVFRbAeriPHlOcsR=41915687.982856073499853991006016586830f;float MUxOBmznWYPnRAkvIjgyzOoUvleSjv=1877487464.495445340717490829314632650445f;if (MUxOBmznWYPnRAkvIjgyzOoUvleSjv - MUxOBmznWYPnRAkvIjgyzOoUvleSjv> 0.00000001 ) MUxOBmznWYPnRAkvIjgyzOoUvleSjv=118346434.136982105815349236123726797480f; else MUxOBmznWYPnRAkvIjgyzOoUvleSjv=84947597.500669162873460939352533342020f;if (MUxOBmznWYPnRAkvIjgyzOoUvleSjv - MUxOBmznWYPnRAkvIjgyzOoUvleSjv> 0.00000001 ) MUxOBmznWYPnRAkvIjgyzOoUvleSjv=103660839.672632679118535046696914817743f; else MUxOBmznWYPnRAkvIjgyzOoUvleSjv=242562170.824231194937134501831644486982f;if (MUxOBmznWYPnRAkvIjgyzOoUvleSjv - MUxOBmznWYPnRAkvIjgyzOoUvleSjv> 0.00000001 ) MUxOBmznWYPnRAkvIjgyzOoUvleSjv=2117548275.593640430505386989132702228086f; else MUxOBmznWYPnRAkvIjgyzOoUvleSjv=333271045.531245459418144747960796190252f;if (MUxOBmznWYPnRAkvIjgyzOoUvleSjv - MUxOBmznWYPnRAkvIjgyzOoUvleSjv> 0.00000001 ) MUxOBmznWYPnRAkvIjgyzOoUvleSjv=900499087.355507415164173567969113785625f; else MUxOBmznWYPnRAkvIjgyzOoUvleSjv=1212902817.520817374733280672374440424383f;if (MUxOBmznWYPnRAkvIjgyzOoUvleSjv - MUxOBmznWYPnRAkvIjgyzOoUvleSjv> 0.00000001 ) MUxOBmznWYPnRAkvIjgyzOoUvleSjv=1130914143.389064536729323603321336229150f; else MUxOBmznWYPnRAkvIjgyzOoUvleSjv=530018578.310256522725775512783107205395f;if (MUxOBmznWYPnRAkvIjgyzOoUvleSjv - MUxOBmznWYPnRAkvIjgyzOoUvleSjv> 0.00000001 ) MUxOBmznWYPnRAkvIjgyzOoUvleSjv=346401746.549742028828271777272747721305f; else MUxOBmznWYPnRAkvIjgyzOoUvleSjv=429221850.124828613359406603654922380203f;int kHQkBVRbOHhTsLMPXEJLqkbGKxGZRn=347013345;if (kHQkBVRbOHhTsLMPXEJLqkbGKxGZRn == kHQkBVRbOHhTsLMPXEJLqkbGKxGZRn- 1 ) kHQkBVRbOHhTsLMPXEJLqkbGKxGZRn=1218234528; else kHQkBVRbOHhTsLMPXEJLqkbGKxGZRn=277268030;if (kHQkBVRbOHhTsLMPXEJLqkbGKxGZRn == kHQkBVRbOHhTsLMPXEJLqkbGKxGZRn- 1 ) kHQkBVRbOHhTsLMPXEJLqkbGKxGZRn=1210684279; else kHQkBVRbOHhTsLMPXEJLqkbGKxGZRn=1541477027;if (kHQkBVRbOHhTsLMPXEJLqkbGKxGZRn == kHQkBVRbOHhTsLMPXEJLqkbGKxGZRn- 0 ) kHQkBVRbOHhTsLMPXEJLqkbGKxGZRn=20835830; else kHQkBVRbOHhTsLMPXEJLqkbGKxGZRn=613011616;if (kHQkBVRbOHhTsLMPXEJLqkbGKxGZRn == kHQkBVRbOHhTsLMPXEJLqkbGKxGZRn- 0 ) kHQkBVRbOHhTsLMPXEJLqkbGKxGZRn=708461759; else kHQkBVRbOHhTsLMPXEJLqkbGKxGZRn=1313299755;if (kHQkBVRbOHhTsLMPXEJLqkbGKxGZRn == kHQkBVRbOHhTsLMPXEJLqkbGKxGZRn- 0 ) kHQkBVRbOHhTsLMPXEJLqkbGKxGZRn=1814447845; else kHQkBVRbOHhTsLMPXEJLqkbGKxGZRn=1778328905;if (kHQkBVRbOHhTsLMPXEJLqkbGKxGZRn == kHQkBVRbOHhTsLMPXEJLqkbGKxGZRn- 0 ) kHQkBVRbOHhTsLMPXEJLqkbGKxGZRn=2023309780; else kHQkBVRbOHhTsLMPXEJLqkbGKxGZRn=1078553389;double DlueNsTBueShKcmWfnCLsrEUMbnIXz=1321301381.559843557379858863383612991480;if (DlueNsTBueShKcmWfnCLsrEUMbnIXz == DlueNsTBueShKcmWfnCLsrEUMbnIXz ) DlueNsTBueShKcmWfnCLsrEUMbnIXz=5535605.444715443835776279464642404706; else DlueNsTBueShKcmWfnCLsrEUMbnIXz=1283429366.217803514452065895682213681342;if (DlueNsTBueShKcmWfnCLsrEUMbnIXz == DlueNsTBueShKcmWfnCLsrEUMbnIXz ) DlueNsTBueShKcmWfnCLsrEUMbnIXz=91945853.275762998477612369761022411532; else DlueNsTBueShKcmWfnCLsrEUMbnIXz=442817007.238328830301732670142368125446;if (DlueNsTBueShKcmWfnCLsrEUMbnIXz == DlueNsTBueShKcmWfnCLsrEUMbnIXz ) DlueNsTBueShKcmWfnCLsrEUMbnIXz=1063460485.247624939463947040194249108121; else DlueNsTBueShKcmWfnCLsrEUMbnIXz=1729204369.440426411874289942119536201862;if (DlueNsTBueShKcmWfnCLsrEUMbnIXz == DlueNsTBueShKcmWfnCLsrEUMbnIXz ) DlueNsTBueShKcmWfnCLsrEUMbnIXz=634640996.791912463273932017126601292522; else DlueNsTBueShKcmWfnCLsrEUMbnIXz=1865716916.565929338869095771430584719183;if (DlueNsTBueShKcmWfnCLsrEUMbnIXz == DlueNsTBueShKcmWfnCLsrEUMbnIXz ) DlueNsTBueShKcmWfnCLsrEUMbnIXz=1518972361.094171812478037861653210319131; else DlueNsTBueShKcmWfnCLsrEUMbnIXz=1344763707.036687503596916265896953099587;if (DlueNsTBueShKcmWfnCLsrEUMbnIXz == DlueNsTBueShKcmWfnCLsrEUMbnIXz ) DlueNsTBueShKcmWfnCLsrEUMbnIXz=1523360900.378155356146927731526445519369; else DlueNsTBueShKcmWfnCLsrEUMbnIXz=235007716.803722996292075103747817103474;int AtlMTQLAYsVYPNWWkwYFdiKIBwgdOu=1570778653;if (AtlMTQLAYsVYPNWWkwYFdiKIBwgdOu == AtlMTQLAYsVYPNWWkwYFdiKIBwgdOu- 1 ) AtlMTQLAYsVYPNWWkwYFdiKIBwgdOu=2036959756; else AtlMTQLAYsVYPNWWkwYFdiKIBwgdOu=191165054;if (AtlMTQLAYsVYPNWWkwYFdiKIBwgdOu == AtlMTQLAYsVYPNWWkwYFdiKIBwgdOu- 1 ) AtlMTQLAYsVYPNWWkwYFdiKIBwgdOu=971164806; else AtlMTQLAYsVYPNWWkwYFdiKIBwgdOu=634785340;if (AtlMTQLAYsVYPNWWkwYFdiKIBwgdOu == AtlMTQLAYsVYPNWWkwYFdiKIBwgdOu- 1 ) AtlMTQLAYsVYPNWWkwYFdiKIBwgdOu=951998763; else AtlMTQLAYsVYPNWWkwYFdiKIBwgdOu=492366936;if (AtlMTQLAYsVYPNWWkwYFdiKIBwgdOu == AtlMTQLAYsVYPNWWkwYFdiKIBwgdOu- 0 ) AtlMTQLAYsVYPNWWkwYFdiKIBwgdOu=1856651634; else AtlMTQLAYsVYPNWWkwYFdiKIBwgdOu=1652818465;if (AtlMTQLAYsVYPNWWkwYFdiKIBwgdOu == AtlMTQLAYsVYPNWWkwYFdiKIBwgdOu- 0 ) AtlMTQLAYsVYPNWWkwYFdiKIBwgdOu=1523969485; else AtlMTQLAYsVYPNWWkwYFdiKIBwgdOu=937844731;if (AtlMTQLAYsVYPNWWkwYFdiKIBwgdOu == AtlMTQLAYsVYPNWWkwYFdiKIBwgdOu- 1 ) AtlMTQLAYsVYPNWWkwYFdiKIBwgdOu=575597972; else AtlMTQLAYsVYPNWWkwYFdiKIBwgdOu=339219370;float NGtYXMLzhYgIuhursNsiGRrvUrTUbH=1492282777.304260262063869890762332244185f;if (NGtYXMLzhYgIuhursNsiGRrvUrTUbH - NGtYXMLzhYgIuhursNsiGRrvUrTUbH> 0.00000001 ) NGtYXMLzhYgIuhursNsiGRrvUrTUbH=379765371.645177071181476413938659808312f; else NGtYXMLzhYgIuhursNsiGRrvUrTUbH=1172584012.629752979282078196694236700647f;if (NGtYXMLzhYgIuhursNsiGRrvUrTUbH - NGtYXMLzhYgIuhursNsiGRrvUrTUbH> 0.00000001 ) NGtYXMLzhYgIuhursNsiGRrvUrTUbH=423686245.087957383464475768272282950415f; else NGtYXMLzhYgIuhursNsiGRrvUrTUbH=308238805.333331612292424086114257680511f;if (NGtYXMLzhYgIuhursNsiGRrvUrTUbH - NGtYXMLzhYgIuhursNsiGRrvUrTUbH> 0.00000001 ) NGtYXMLzhYgIuhursNsiGRrvUrTUbH=1185826311.708823124426709096997430119776f; else NGtYXMLzhYgIuhursNsiGRrvUrTUbH=327993393.211259561107270965969572547882f;if (NGtYXMLzhYgIuhursNsiGRrvUrTUbH - NGtYXMLzhYgIuhursNsiGRrvUrTUbH> 0.00000001 ) NGtYXMLzhYgIuhursNsiGRrvUrTUbH=1431036507.455931556064689037135582875982f; else NGtYXMLzhYgIuhursNsiGRrvUrTUbH=1636513806.378985334952091599351186807427f;if (NGtYXMLzhYgIuhursNsiGRrvUrTUbH - NGtYXMLzhYgIuhursNsiGRrvUrTUbH> 0.00000001 ) NGtYXMLzhYgIuhursNsiGRrvUrTUbH=1790110681.768489671480328350138663230328f; else NGtYXMLzhYgIuhursNsiGRrvUrTUbH=1492451010.992829571293898186856739910536f;if (NGtYXMLzhYgIuhursNsiGRrvUrTUbH - NGtYXMLzhYgIuhursNsiGRrvUrTUbH> 0.00000001 ) NGtYXMLzhYgIuhursNsiGRrvUrTUbH=22504666.045460860999005698615468226491f; else NGtYXMLzhYgIuhursNsiGRrvUrTUbH=823555770.004526854842796783443393178824f;double UiuyvXWqwpTTojQkjMZWOzdqiEJXZl=218856846.037137384936614820417791322885;if (UiuyvXWqwpTTojQkjMZWOzdqiEJXZl == UiuyvXWqwpTTojQkjMZWOzdqiEJXZl ) UiuyvXWqwpTTojQkjMZWOzdqiEJXZl=778099200.155431120043647434436303634681; else UiuyvXWqwpTTojQkjMZWOzdqiEJXZl=452531127.651089229380048526409946986290;if (UiuyvXWqwpTTojQkjMZWOzdqiEJXZl == UiuyvXWqwpTTojQkjMZWOzdqiEJXZl ) UiuyvXWqwpTTojQkjMZWOzdqiEJXZl=1189077280.855949377922526646622938381511; else UiuyvXWqwpTTojQkjMZWOzdqiEJXZl=868957180.404963620935968843045077893777;if (UiuyvXWqwpTTojQkjMZWOzdqiEJXZl == UiuyvXWqwpTTojQkjMZWOzdqiEJXZl ) UiuyvXWqwpTTojQkjMZWOzdqiEJXZl=814950207.887663966313780707154958744718; else UiuyvXWqwpTTojQkjMZWOzdqiEJXZl=1662951514.095415930608883349820672403003;if (UiuyvXWqwpTTojQkjMZWOzdqiEJXZl == UiuyvXWqwpTTojQkjMZWOzdqiEJXZl ) UiuyvXWqwpTTojQkjMZWOzdqiEJXZl=369302618.026932837587856718551590853312; else UiuyvXWqwpTTojQkjMZWOzdqiEJXZl=1660652316.304878305151244382789556253619;if (UiuyvXWqwpTTojQkjMZWOzdqiEJXZl == UiuyvXWqwpTTojQkjMZWOzdqiEJXZl ) UiuyvXWqwpTTojQkjMZWOzdqiEJXZl=1574261379.053628418842440046767174396863; else UiuyvXWqwpTTojQkjMZWOzdqiEJXZl=586611035.570621862608404821732805399044;if (UiuyvXWqwpTTojQkjMZWOzdqiEJXZl == UiuyvXWqwpTTojQkjMZWOzdqiEJXZl ) UiuyvXWqwpTTojQkjMZWOzdqiEJXZl=1066331169.366637383038417978068623188543; else UiuyvXWqwpTTojQkjMZWOzdqiEJXZl=125615754.283944397754881550942219785356;double qUzoXaYlOitEOERlWOjNreAEAlUMev=1697026663.425644997523357634730936339290;if (qUzoXaYlOitEOERlWOjNreAEAlUMev == qUzoXaYlOitEOERlWOjNreAEAlUMev ) qUzoXaYlOitEOERlWOjNreAEAlUMev=1191983124.969470402414767506640880960941; else qUzoXaYlOitEOERlWOjNreAEAlUMev=31078123.571876598834233728994375018962;if (qUzoXaYlOitEOERlWOjNreAEAlUMev == qUzoXaYlOitEOERlWOjNreAEAlUMev ) qUzoXaYlOitEOERlWOjNreAEAlUMev=2002725992.057194409211892665835316731687; else qUzoXaYlOitEOERlWOjNreAEAlUMev=173738536.874788825943988458345314785247;if (qUzoXaYlOitEOERlWOjNreAEAlUMev == qUzoXaYlOitEOERlWOjNreAEAlUMev ) qUzoXaYlOitEOERlWOjNreAEAlUMev=1171904754.483345821595109244848344256101; else qUzoXaYlOitEOERlWOjNreAEAlUMev=691990900.378857174464896032554241925910;if (qUzoXaYlOitEOERlWOjNreAEAlUMev == qUzoXaYlOitEOERlWOjNreAEAlUMev ) qUzoXaYlOitEOERlWOjNreAEAlUMev=384952342.982122836227049565833465723893; else qUzoXaYlOitEOERlWOjNreAEAlUMev=1708966363.327569378430734721357019763741;if (qUzoXaYlOitEOERlWOjNreAEAlUMev == qUzoXaYlOitEOERlWOjNreAEAlUMev ) qUzoXaYlOitEOERlWOjNreAEAlUMev=907612194.914301168129750383200264631981; else qUzoXaYlOitEOERlWOjNreAEAlUMev=1482134647.773853644377334937958055186077;if (qUzoXaYlOitEOERlWOjNreAEAlUMev == qUzoXaYlOitEOERlWOjNreAEAlUMev ) qUzoXaYlOitEOERlWOjNreAEAlUMev=992158306.038846700875004494180504373978; else qUzoXaYlOitEOERlWOjNreAEAlUMev=1607031400.767744959934818305189309863568;int nPGCEnSfAXUZNejktcyklowwZiouqC=1050515442;if (nPGCEnSfAXUZNejktcyklowwZiouqC == nPGCEnSfAXUZNejktcyklowwZiouqC- 0 ) nPGCEnSfAXUZNejktcyklowwZiouqC=970867524; else nPGCEnSfAXUZNejktcyklowwZiouqC=356741126;if (nPGCEnSfAXUZNejktcyklowwZiouqC == nPGCEnSfAXUZNejktcyklowwZiouqC- 1 ) nPGCEnSfAXUZNejktcyklowwZiouqC=1708603817; else nPGCEnSfAXUZNejktcyklowwZiouqC=4913785;if (nPGCEnSfAXUZNejktcyklowwZiouqC == nPGCEnSfAXUZNejktcyklowwZiouqC- 1 ) nPGCEnSfAXUZNejktcyklowwZiouqC=780352065; else nPGCEnSfAXUZNejktcyklowwZiouqC=1431307050;if (nPGCEnSfAXUZNejktcyklowwZiouqC == nPGCEnSfAXUZNejktcyklowwZiouqC- 0 ) nPGCEnSfAXUZNejktcyklowwZiouqC=677332570; else nPGCEnSfAXUZNejktcyklowwZiouqC=864297504;if (nPGCEnSfAXUZNejktcyklowwZiouqC == nPGCEnSfAXUZNejktcyklowwZiouqC- 0 ) nPGCEnSfAXUZNejktcyklowwZiouqC=803281394; else nPGCEnSfAXUZNejktcyklowwZiouqC=910517576;if (nPGCEnSfAXUZNejktcyklowwZiouqC == nPGCEnSfAXUZNejktcyklowwZiouqC- 0 ) nPGCEnSfAXUZNejktcyklowwZiouqC=881461073; else nPGCEnSfAXUZNejktcyklowwZiouqC=599215276;int haPZlVtVKXpHBQFcdNeihUqtHYDKcV=1494689055;if (haPZlVtVKXpHBQFcdNeihUqtHYDKcV == haPZlVtVKXpHBQFcdNeihUqtHYDKcV- 0 ) haPZlVtVKXpHBQFcdNeihUqtHYDKcV=84179006; else haPZlVtVKXpHBQFcdNeihUqtHYDKcV=472961650;if (haPZlVtVKXpHBQFcdNeihUqtHYDKcV == haPZlVtVKXpHBQFcdNeihUqtHYDKcV- 0 ) haPZlVtVKXpHBQFcdNeihUqtHYDKcV=1779056293; else haPZlVtVKXpHBQFcdNeihUqtHYDKcV=1206056240;if (haPZlVtVKXpHBQFcdNeihUqtHYDKcV == haPZlVtVKXpHBQFcdNeihUqtHYDKcV- 1 ) haPZlVtVKXpHBQFcdNeihUqtHYDKcV=618211698; else haPZlVtVKXpHBQFcdNeihUqtHYDKcV=1940722318;if (haPZlVtVKXpHBQFcdNeihUqtHYDKcV == haPZlVtVKXpHBQFcdNeihUqtHYDKcV- 0 ) haPZlVtVKXpHBQFcdNeihUqtHYDKcV=1194435739; else haPZlVtVKXpHBQFcdNeihUqtHYDKcV=86108300;if (haPZlVtVKXpHBQFcdNeihUqtHYDKcV == haPZlVtVKXpHBQFcdNeihUqtHYDKcV- 0 ) haPZlVtVKXpHBQFcdNeihUqtHYDKcV=67578785; else haPZlVtVKXpHBQFcdNeihUqtHYDKcV=1627594564;if (haPZlVtVKXpHBQFcdNeihUqtHYDKcV == haPZlVtVKXpHBQFcdNeihUqtHYDKcV- 1 ) haPZlVtVKXpHBQFcdNeihUqtHYDKcV=2108828633; else haPZlVtVKXpHBQFcdNeihUqtHYDKcV=1480299207;long AvAADIoUcUlgEYbVxmowHRxrQMzylu=541165515;if (AvAADIoUcUlgEYbVxmowHRxrQMzylu == AvAADIoUcUlgEYbVxmowHRxrQMzylu- 0 ) AvAADIoUcUlgEYbVxmowHRxrQMzylu=170926933; else AvAADIoUcUlgEYbVxmowHRxrQMzylu=15815727;if (AvAADIoUcUlgEYbVxmowHRxrQMzylu == AvAADIoUcUlgEYbVxmowHRxrQMzylu- 0 ) AvAADIoUcUlgEYbVxmowHRxrQMzylu=1302231877; else AvAADIoUcUlgEYbVxmowHRxrQMzylu=796865271;if (AvAADIoUcUlgEYbVxmowHRxrQMzylu == AvAADIoUcUlgEYbVxmowHRxrQMzylu- 1 ) AvAADIoUcUlgEYbVxmowHRxrQMzylu=734301456; else AvAADIoUcUlgEYbVxmowHRxrQMzylu=1403358219;if (AvAADIoUcUlgEYbVxmowHRxrQMzylu == AvAADIoUcUlgEYbVxmowHRxrQMzylu- 1 ) AvAADIoUcUlgEYbVxmowHRxrQMzylu=916674598; else AvAADIoUcUlgEYbVxmowHRxrQMzylu=1908159721;if (AvAADIoUcUlgEYbVxmowHRxrQMzylu == AvAADIoUcUlgEYbVxmowHRxrQMzylu- 1 ) AvAADIoUcUlgEYbVxmowHRxrQMzylu=1501229200; else AvAADIoUcUlgEYbVxmowHRxrQMzylu=1138428488;if (AvAADIoUcUlgEYbVxmowHRxrQMzylu == AvAADIoUcUlgEYbVxmowHRxrQMzylu- 0 ) AvAADIoUcUlgEYbVxmowHRxrQMzylu=1517940789; else AvAADIoUcUlgEYbVxmowHRxrQMzylu=261387178;int AoaeeumsowkpKfGgjkiCuikqESEuMe=1749675088;if (AoaeeumsowkpKfGgjkiCuikqESEuMe == AoaeeumsowkpKfGgjkiCuikqESEuMe- 0 ) AoaeeumsowkpKfGgjkiCuikqESEuMe=999227238; else AoaeeumsowkpKfGgjkiCuikqESEuMe=1644952898;if (AoaeeumsowkpKfGgjkiCuikqESEuMe == AoaeeumsowkpKfGgjkiCuikqESEuMe- 0 ) AoaeeumsowkpKfGgjkiCuikqESEuMe=1522852394; else AoaeeumsowkpKfGgjkiCuikqESEuMe=2144384267;if (AoaeeumsowkpKfGgjkiCuikqESEuMe == AoaeeumsowkpKfGgjkiCuikqESEuMe- 1 ) AoaeeumsowkpKfGgjkiCuikqESEuMe=1401523218; else AoaeeumsowkpKfGgjkiCuikqESEuMe=1357111509;if (AoaeeumsowkpKfGgjkiCuikqESEuMe == AoaeeumsowkpKfGgjkiCuikqESEuMe- 0 ) AoaeeumsowkpKfGgjkiCuikqESEuMe=432303744; else AoaeeumsowkpKfGgjkiCuikqESEuMe=1177628853;if (AoaeeumsowkpKfGgjkiCuikqESEuMe == AoaeeumsowkpKfGgjkiCuikqESEuMe- 1 ) AoaeeumsowkpKfGgjkiCuikqESEuMe=1309789034; else AoaeeumsowkpKfGgjkiCuikqESEuMe=1622495517;if (AoaeeumsowkpKfGgjkiCuikqESEuMe == AoaeeumsowkpKfGgjkiCuikqESEuMe- 1 ) AoaeeumsowkpKfGgjkiCuikqESEuMe=2084818647; else AoaeeumsowkpKfGgjkiCuikqESEuMe=1300241148;float HJuzzAPdYICSCilylrMfCIaLDFWDmR=2041010176.189532953855381590202999033837f;if (HJuzzAPdYICSCilylrMfCIaLDFWDmR - HJuzzAPdYICSCilylrMfCIaLDFWDmR> 0.00000001 ) HJuzzAPdYICSCilylrMfCIaLDFWDmR=1867211177.203099459494388001574410920150f; else HJuzzAPdYICSCilylrMfCIaLDFWDmR=340295558.912675249215192861114336490418f;if (HJuzzAPdYICSCilylrMfCIaLDFWDmR - HJuzzAPdYICSCilylrMfCIaLDFWDmR> 0.00000001 ) HJuzzAPdYICSCilylrMfCIaLDFWDmR=428288343.041958382486337032827914779738f; else HJuzzAPdYICSCilylrMfCIaLDFWDmR=1393330144.694237425568083800046869114738f;if (HJuzzAPdYICSCilylrMfCIaLDFWDmR - HJuzzAPdYICSCilylrMfCIaLDFWDmR> 0.00000001 ) HJuzzAPdYICSCilylrMfCIaLDFWDmR=3188876.075480321071847904297979283512f; else HJuzzAPdYICSCilylrMfCIaLDFWDmR=1833525785.480860460469008513322064110621f;if (HJuzzAPdYICSCilylrMfCIaLDFWDmR - HJuzzAPdYICSCilylrMfCIaLDFWDmR> 0.00000001 ) HJuzzAPdYICSCilylrMfCIaLDFWDmR=760387599.781969219764484998287130142675f; else HJuzzAPdYICSCilylrMfCIaLDFWDmR=828304434.185088924941566969132355466292f;if (HJuzzAPdYICSCilylrMfCIaLDFWDmR - HJuzzAPdYICSCilylrMfCIaLDFWDmR> 0.00000001 ) HJuzzAPdYICSCilylrMfCIaLDFWDmR=1703770699.874479050608444777429455383062f; else HJuzzAPdYICSCilylrMfCIaLDFWDmR=2018380219.283227810802953502179087442919f;if (HJuzzAPdYICSCilylrMfCIaLDFWDmR - HJuzzAPdYICSCilylrMfCIaLDFWDmR> 0.00000001 ) HJuzzAPdYICSCilylrMfCIaLDFWDmR=1852310687.609853622807873761399209430699f; else HJuzzAPdYICSCilylrMfCIaLDFWDmR=1627266857.590120311008048322578273051052f;int vQRMCgjYOWazLvFHfGBFoJGBSMlDjw=281841719;if (vQRMCgjYOWazLvFHfGBFoJGBSMlDjw == vQRMCgjYOWazLvFHfGBFoJGBSMlDjw- 1 ) vQRMCgjYOWazLvFHfGBFoJGBSMlDjw=1327774072; else vQRMCgjYOWazLvFHfGBFoJGBSMlDjw=359153545;if (vQRMCgjYOWazLvFHfGBFoJGBSMlDjw == vQRMCgjYOWazLvFHfGBFoJGBSMlDjw- 0 ) vQRMCgjYOWazLvFHfGBFoJGBSMlDjw=481071969; else vQRMCgjYOWazLvFHfGBFoJGBSMlDjw=1173323957;if (vQRMCgjYOWazLvFHfGBFoJGBSMlDjw == vQRMCgjYOWazLvFHfGBFoJGBSMlDjw- 1 ) vQRMCgjYOWazLvFHfGBFoJGBSMlDjw=1534386118; else vQRMCgjYOWazLvFHfGBFoJGBSMlDjw=1193889426;if (vQRMCgjYOWazLvFHfGBFoJGBSMlDjw == vQRMCgjYOWazLvFHfGBFoJGBSMlDjw- 1 ) vQRMCgjYOWazLvFHfGBFoJGBSMlDjw=1287125944; else vQRMCgjYOWazLvFHfGBFoJGBSMlDjw=961299071;if (vQRMCgjYOWazLvFHfGBFoJGBSMlDjw == vQRMCgjYOWazLvFHfGBFoJGBSMlDjw- 1 ) vQRMCgjYOWazLvFHfGBFoJGBSMlDjw=1730995630; else vQRMCgjYOWazLvFHfGBFoJGBSMlDjw=124603024;if (vQRMCgjYOWazLvFHfGBFoJGBSMlDjw == vQRMCgjYOWazLvFHfGBFoJGBSMlDjw- 0 ) vQRMCgjYOWazLvFHfGBFoJGBSMlDjw=1634559884; else vQRMCgjYOWazLvFHfGBFoJGBSMlDjw=1259289586;float QmvuhDilhhArmXPlWBYcDPEOeURsIn=156712190.310789108749499092335400862956f;if (QmvuhDilhhArmXPlWBYcDPEOeURsIn - QmvuhDilhhArmXPlWBYcDPEOeURsIn> 0.00000001 ) QmvuhDilhhArmXPlWBYcDPEOeURsIn=944924192.092333578378182733986382691002f; else QmvuhDilhhArmXPlWBYcDPEOeURsIn=1624414260.726386717653407843823008081084f;if (QmvuhDilhhArmXPlWBYcDPEOeURsIn - QmvuhDilhhArmXPlWBYcDPEOeURsIn> 0.00000001 ) QmvuhDilhhArmXPlWBYcDPEOeURsIn=541659648.357775280703643498596829973940f; else QmvuhDilhhArmXPlWBYcDPEOeURsIn=1617717916.574431684366312057569459863980f;if (QmvuhDilhhArmXPlWBYcDPEOeURsIn - QmvuhDilhhArmXPlWBYcDPEOeURsIn> 0.00000001 ) QmvuhDilhhArmXPlWBYcDPEOeURsIn=1504932370.345133031281533151793648122568f; else QmvuhDilhhArmXPlWBYcDPEOeURsIn=1053973995.870402139013648903682230871802f;if (QmvuhDilhhArmXPlWBYcDPEOeURsIn - QmvuhDilhhArmXPlWBYcDPEOeURsIn> 0.00000001 ) QmvuhDilhhArmXPlWBYcDPEOeURsIn=1746486212.167352113102379101032098765174f; else QmvuhDilhhArmXPlWBYcDPEOeURsIn=1313185100.234795649789068006177197017979f;if (QmvuhDilhhArmXPlWBYcDPEOeURsIn - QmvuhDilhhArmXPlWBYcDPEOeURsIn> 0.00000001 ) QmvuhDilhhArmXPlWBYcDPEOeURsIn=884565299.714008869457236414187000725889f; else QmvuhDilhhArmXPlWBYcDPEOeURsIn=694547960.888017506124883196271490371549f;if (QmvuhDilhhArmXPlWBYcDPEOeURsIn - QmvuhDilhhArmXPlWBYcDPEOeURsIn> 0.00000001 ) QmvuhDilhhArmXPlWBYcDPEOeURsIn=440613568.319068022991806333863971859984f; else QmvuhDilhhArmXPlWBYcDPEOeURsIn=1530626646.717892682528538169405879655832f;long FWqKpXAKToaGPWcWTOmtDWnvtTpLUk=376740473;if (FWqKpXAKToaGPWcWTOmtDWnvtTpLUk == FWqKpXAKToaGPWcWTOmtDWnvtTpLUk- 0 ) FWqKpXAKToaGPWcWTOmtDWnvtTpLUk=1346917044; else FWqKpXAKToaGPWcWTOmtDWnvtTpLUk=1802976928;if (FWqKpXAKToaGPWcWTOmtDWnvtTpLUk == FWqKpXAKToaGPWcWTOmtDWnvtTpLUk- 1 ) FWqKpXAKToaGPWcWTOmtDWnvtTpLUk=2119950723; else FWqKpXAKToaGPWcWTOmtDWnvtTpLUk=1110808519;if (FWqKpXAKToaGPWcWTOmtDWnvtTpLUk == FWqKpXAKToaGPWcWTOmtDWnvtTpLUk- 1 ) FWqKpXAKToaGPWcWTOmtDWnvtTpLUk=863445041; else FWqKpXAKToaGPWcWTOmtDWnvtTpLUk=1167663986;if (FWqKpXAKToaGPWcWTOmtDWnvtTpLUk == FWqKpXAKToaGPWcWTOmtDWnvtTpLUk- 1 ) FWqKpXAKToaGPWcWTOmtDWnvtTpLUk=506624058; else FWqKpXAKToaGPWcWTOmtDWnvtTpLUk=673321751;if (FWqKpXAKToaGPWcWTOmtDWnvtTpLUk == FWqKpXAKToaGPWcWTOmtDWnvtTpLUk- 0 ) FWqKpXAKToaGPWcWTOmtDWnvtTpLUk=1200653261; else FWqKpXAKToaGPWcWTOmtDWnvtTpLUk=1614472919;if (FWqKpXAKToaGPWcWTOmtDWnvtTpLUk == FWqKpXAKToaGPWcWTOmtDWnvtTpLUk- 1 ) FWqKpXAKToaGPWcWTOmtDWnvtTpLUk=1809818161; else FWqKpXAKToaGPWcWTOmtDWnvtTpLUk=2026069499;long lwGysLWIddqDvreEvQIjOkiEaKcjSN=2087337249;if (lwGysLWIddqDvreEvQIjOkiEaKcjSN == lwGysLWIddqDvreEvQIjOkiEaKcjSN- 1 ) lwGysLWIddqDvreEvQIjOkiEaKcjSN=1580182562; else lwGysLWIddqDvreEvQIjOkiEaKcjSN=1695598497;if (lwGysLWIddqDvreEvQIjOkiEaKcjSN == lwGysLWIddqDvreEvQIjOkiEaKcjSN- 0 ) lwGysLWIddqDvreEvQIjOkiEaKcjSN=682342665; else lwGysLWIddqDvreEvQIjOkiEaKcjSN=945850916;if (lwGysLWIddqDvreEvQIjOkiEaKcjSN == lwGysLWIddqDvreEvQIjOkiEaKcjSN- 1 ) lwGysLWIddqDvreEvQIjOkiEaKcjSN=391388913; else lwGysLWIddqDvreEvQIjOkiEaKcjSN=805344204;if (lwGysLWIddqDvreEvQIjOkiEaKcjSN == lwGysLWIddqDvreEvQIjOkiEaKcjSN- 0 ) lwGysLWIddqDvreEvQIjOkiEaKcjSN=924392996; else lwGysLWIddqDvreEvQIjOkiEaKcjSN=273800077;if (lwGysLWIddqDvreEvQIjOkiEaKcjSN == lwGysLWIddqDvreEvQIjOkiEaKcjSN- 1 ) lwGysLWIddqDvreEvQIjOkiEaKcjSN=760150980; else lwGysLWIddqDvreEvQIjOkiEaKcjSN=1315370516;if (lwGysLWIddqDvreEvQIjOkiEaKcjSN == lwGysLWIddqDvreEvQIjOkiEaKcjSN- 1 ) lwGysLWIddqDvreEvQIjOkiEaKcjSN=288758658; else lwGysLWIddqDvreEvQIjOkiEaKcjSN=839838158;float HeflYhkKQWKUGbPkLLsRKLrQYtlDKq=778475096.794505391285109998531459639291f;if (HeflYhkKQWKUGbPkLLsRKLrQYtlDKq - HeflYhkKQWKUGbPkLLsRKLrQYtlDKq> 0.00000001 ) HeflYhkKQWKUGbPkLLsRKLrQYtlDKq=1517239347.716059621254005985861797444598f; else HeflYhkKQWKUGbPkLLsRKLrQYtlDKq=1257819411.610058926137749845586454012961f;if (HeflYhkKQWKUGbPkLLsRKLrQYtlDKq - HeflYhkKQWKUGbPkLLsRKLrQYtlDKq> 0.00000001 ) HeflYhkKQWKUGbPkLLsRKLrQYtlDKq=2059856189.524465416717073506970902237604f; else HeflYhkKQWKUGbPkLLsRKLrQYtlDKq=1810529938.606572348544087058262710262324f;if (HeflYhkKQWKUGbPkLLsRKLrQYtlDKq - HeflYhkKQWKUGbPkLLsRKLrQYtlDKq> 0.00000001 ) HeflYhkKQWKUGbPkLLsRKLrQYtlDKq=1791450021.881221151127853602539055309641f; else HeflYhkKQWKUGbPkLLsRKLrQYtlDKq=1474872785.012577487790725697866090694602f;if (HeflYhkKQWKUGbPkLLsRKLrQYtlDKq - HeflYhkKQWKUGbPkLLsRKLrQYtlDKq> 0.00000001 ) HeflYhkKQWKUGbPkLLsRKLrQYtlDKq=1440750796.262011539733585933081398503308f; else HeflYhkKQWKUGbPkLLsRKLrQYtlDKq=1924743853.389653871416914461577162207561f;if (HeflYhkKQWKUGbPkLLsRKLrQYtlDKq - HeflYhkKQWKUGbPkLLsRKLrQYtlDKq> 0.00000001 ) HeflYhkKQWKUGbPkLLsRKLrQYtlDKq=1117790202.924145504226377558425283232756f; else HeflYhkKQWKUGbPkLLsRKLrQYtlDKq=2015821544.742059388906846190575014784202f;if (HeflYhkKQWKUGbPkLLsRKLrQYtlDKq - HeflYhkKQWKUGbPkLLsRKLrQYtlDKq> 0.00000001 ) HeflYhkKQWKUGbPkLLsRKLrQYtlDKq=417064655.386868532837062045724328283848f; else HeflYhkKQWKUGbPkLLsRKLrQYtlDKq=2037943098.619762018866874806274959816879f;float jtVOKrRwBwTlwcpXaURpkekVhfCwbe=1383467286.691638117694785967800001678259f;if (jtVOKrRwBwTlwcpXaURpkekVhfCwbe - jtVOKrRwBwTlwcpXaURpkekVhfCwbe> 0.00000001 ) jtVOKrRwBwTlwcpXaURpkekVhfCwbe=1332266745.553355316083217865408017070016f; else jtVOKrRwBwTlwcpXaURpkekVhfCwbe=500759966.106925253176292889592485164739f;if (jtVOKrRwBwTlwcpXaURpkekVhfCwbe - jtVOKrRwBwTlwcpXaURpkekVhfCwbe> 0.00000001 ) jtVOKrRwBwTlwcpXaURpkekVhfCwbe=2097927731.640098740261244382539152572287f; else jtVOKrRwBwTlwcpXaURpkekVhfCwbe=2104169619.153553620525043224198330059962f;if (jtVOKrRwBwTlwcpXaURpkekVhfCwbe - jtVOKrRwBwTlwcpXaURpkekVhfCwbe> 0.00000001 ) jtVOKrRwBwTlwcpXaURpkekVhfCwbe=270177869.756312980176202522046810093350f; else jtVOKrRwBwTlwcpXaURpkekVhfCwbe=1956480055.587897073245502213954892795642f;if (jtVOKrRwBwTlwcpXaURpkekVhfCwbe - jtVOKrRwBwTlwcpXaURpkekVhfCwbe> 0.00000001 ) jtVOKrRwBwTlwcpXaURpkekVhfCwbe=2132835207.963070942665991131889140779520f; else jtVOKrRwBwTlwcpXaURpkekVhfCwbe=541572840.243138263337140991276487492441f;if (jtVOKrRwBwTlwcpXaURpkekVhfCwbe - jtVOKrRwBwTlwcpXaURpkekVhfCwbe> 0.00000001 ) jtVOKrRwBwTlwcpXaURpkekVhfCwbe=878583932.153342396308865201189846596784f; else jtVOKrRwBwTlwcpXaURpkekVhfCwbe=1846150646.693329590871697963220236241886f;if (jtVOKrRwBwTlwcpXaURpkekVhfCwbe - jtVOKrRwBwTlwcpXaURpkekVhfCwbe> 0.00000001 ) jtVOKrRwBwTlwcpXaURpkekVhfCwbe=350657539.762372655782131935973466556729f; else jtVOKrRwBwTlwcpXaURpkekVhfCwbe=1695558172.871522933531965811641792441782f;int vTeqGEgGxwAMVQGXyDhXaEooJmsFqD=1444153963;if (vTeqGEgGxwAMVQGXyDhXaEooJmsFqD == vTeqGEgGxwAMVQGXyDhXaEooJmsFqD- 0 ) vTeqGEgGxwAMVQGXyDhXaEooJmsFqD=157974670; else vTeqGEgGxwAMVQGXyDhXaEooJmsFqD=1031343065;if (vTeqGEgGxwAMVQGXyDhXaEooJmsFqD == vTeqGEgGxwAMVQGXyDhXaEooJmsFqD- 1 ) vTeqGEgGxwAMVQGXyDhXaEooJmsFqD=508830152; else vTeqGEgGxwAMVQGXyDhXaEooJmsFqD=1088630137;if (vTeqGEgGxwAMVQGXyDhXaEooJmsFqD == vTeqGEgGxwAMVQGXyDhXaEooJmsFqD- 1 ) vTeqGEgGxwAMVQGXyDhXaEooJmsFqD=1736209118; else vTeqGEgGxwAMVQGXyDhXaEooJmsFqD=1846718571;if (vTeqGEgGxwAMVQGXyDhXaEooJmsFqD == vTeqGEgGxwAMVQGXyDhXaEooJmsFqD- 0 ) vTeqGEgGxwAMVQGXyDhXaEooJmsFqD=295887228; else vTeqGEgGxwAMVQGXyDhXaEooJmsFqD=105309777;if (vTeqGEgGxwAMVQGXyDhXaEooJmsFqD == vTeqGEgGxwAMVQGXyDhXaEooJmsFqD- 0 ) vTeqGEgGxwAMVQGXyDhXaEooJmsFqD=254847701; else vTeqGEgGxwAMVQGXyDhXaEooJmsFqD=905082459;if (vTeqGEgGxwAMVQGXyDhXaEooJmsFqD == vTeqGEgGxwAMVQGXyDhXaEooJmsFqD- 1 ) vTeqGEgGxwAMVQGXyDhXaEooJmsFqD=1975544361; else vTeqGEgGxwAMVQGXyDhXaEooJmsFqD=523051016;double TdFGFmKKlMmOKKajyeejfhvfPbwhCE=1039734267.646816644184078130219623212629;if (TdFGFmKKlMmOKKajyeejfhvfPbwhCE == TdFGFmKKlMmOKKajyeejfhvfPbwhCE ) TdFGFmKKlMmOKKajyeejfhvfPbwhCE=201151317.485449205165662601133133102371; else TdFGFmKKlMmOKKajyeejfhvfPbwhCE=1052775019.413671963993516517161259645245;if (TdFGFmKKlMmOKKajyeejfhvfPbwhCE == TdFGFmKKlMmOKKajyeejfhvfPbwhCE ) TdFGFmKKlMmOKKajyeejfhvfPbwhCE=1655055060.804585319284835072860621037404; else TdFGFmKKlMmOKKajyeejfhvfPbwhCE=252515892.673929004120808843578458529366;if (TdFGFmKKlMmOKKajyeejfhvfPbwhCE == TdFGFmKKlMmOKKajyeejfhvfPbwhCE ) TdFGFmKKlMmOKKajyeejfhvfPbwhCE=1953538861.992672201607975093429404732541; else TdFGFmKKlMmOKKajyeejfhvfPbwhCE=1499812277.185933934764297158454694750895;if (TdFGFmKKlMmOKKajyeejfhvfPbwhCE == TdFGFmKKlMmOKKajyeejfhvfPbwhCE ) TdFGFmKKlMmOKKajyeejfhvfPbwhCE=508297227.076163315226086521868520759128; else TdFGFmKKlMmOKKajyeejfhvfPbwhCE=1708242939.414970924495434872823914866741;if (TdFGFmKKlMmOKKajyeejfhvfPbwhCE == TdFGFmKKlMmOKKajyeejfhvfPbwhCE ) TdFGFmKKlMmOKKajyeejfhvfPbwhCE=1272467851.429781211771847320699830551122; else TdFGFmKKlMmOKKajyeejfhvfPbwhCE=1518283349.539418775367126328854715269934;if (TdFGFmKKlMmOKKajyeejfhvfPbwhCE == TdFGFmKKlMmOKKajyeejfhvfPbwhCE ) TdFGFmKKlMmOKKajyeejfhvfPbwhCE=931946006.129035353076544372534347882425; else TdFGFmKKlMmOKKajyeejfhvfPbwhCE=2092783022.985225829118589927209850199225;long tNoCvGSKvZeNSKeoaqyhYLKeGOVFRb=1451004302;if (tNoCvGSKvZeNSKeoaqyhYLKeGOVFRb == tNoCvGSKvZeNSKeoaqyhYLKeGOVFRb- 1 ) tNoCvGSKvZeNSKeoaqyhYLKeGOVFRb=2095775651; else tNoCvGSKvZeNSKeoaqyhYLKeGOVFRb=1120394339;if (tNoCvGSKvZeNSKeoaqyhYLKeGOVFRb == tNoCvGSKvZeNSKeoaqyhYLKeGOVFRb- 1 ) tNoCvGSKvZeNSKeoaqyhYLKeGOVFRb=1879968428; else tNoCvGSKvZeNSKeoaqyhYLKeGOVFRb=360296416;if (tNoCvGSKvZeNSKeoaqyhYLKeGOVFRb == tNoCvGSKvZeNSKeoaqyhYLKeGOVFRb- 1 ) tNoCvGSKvZeNSKeoaqyhYLKeGOVFRb=1359070208; else tNoCvGSKvZeNSKeoaqyhYLKeGOVFRb=25589062;if (tNoCvGSKvZeNSKeoaqyhYLKeGOVFRb == tNoCvGSKvZeNSKeoaqyhYLKeGOVFRb- 0 ) tNoCvGSKvZeNSKeoaqyhYLKeGOVFRb=1794741815; else tNoCvGSKvZeNSKeoaqyhYLKeGOVFRb=1633031821;if (tNoCvGSKvZeNSKeoaqyhYLKeGOVFRb == tNoCvGSKvZeNSKeoaqyhYLKeGOVFRb- 0 ) tNoCvGSKvZeNSKeoaqyhYLKeGOVFRb=204872738; else tNoCvGSKvZeNSKeoaqyhYLKeGOVFRb=1992617954;if (tNoCvGSKvZeNSKeoaqyhYLKeGOVFRb == tNoCvGSKvZeNSKeoaqyhYLKeGOVFRb- 1 ) tNoCvGSKvZeNSKeoaqyhYLKeGOVFRb=1849321918; else tNoCvGSKvZeNSKeoaqyhYLKeGOVFRb=1512579057; }
 tNoCvGSKvZeNSKeoaqyhYLKeGOVFRby::tNoCvGSKvZeNSKeoaqyhYLKeGOVFRby()
 { this->WiYPVaeiXiis("yXqHXkmxjpYLXQYDpVCSTIppiCJwglWiYPVaeiXiisj", true, 2128419341, 1609784191, 153293291); }
#pragma optimize("", off)
 // <delete/>


// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class LUNbQLqUBGykGZQNvIMMTXHPjbogHLy
 { 
public: bool FMiepLMxjpkdyaxezzOEioqunMxspW; double FMiepLMxjpkdyaxezzOEioqunMxspWLUNbQLqUBGykGZQNvIMMTXHPjbogHL; LUNbQLqUBGykGZQNvIMMTXHPjbogHLy(); void BmjnICJCnTkj(string FMiepLMxjpkdyaxezzOEioqunMxspWBmjnICJCnTkj, bool JNqStXYubCjLdWjuIplwpgsGGQVorc, int NUINgXnfhZGEYMVWKUKsDIGsgcUjMc, float CjDypOCQqMGdILqlIgljGBdyYENQgD, long DETFXWBZXGUudymaAuBvUnxcVAFFZk);
 protected: bool FMiepLMxjpkdyaxezzOEioqunMxspWo; double FMiepLMxjpkdyaxezzOEioqunMxspWLUNbQLqUBGykGZQNvIMMTXHPjbogHLf; void BmjnICJCnTkju(string FMiepLMxjpkdyaxezzOEioqunMxspWBmjnICJCnTkjg, bool JNqStXYubCjLdWjuIplwpgsGGQVorce, int NUINgXnfhZGEYMVWKUKsDIGsgcUjMcr, float CjDypOCQqMGdILqlIgljGBdyYENQgDw, long DETFXWBZXGUudymaAuBvUnxcVAFFZkn);
 private: bool FMiepLMxjpkdyaxezzOEioqunMxspWJNqStXYubCjLdWjuIplwpgsGGQVorc; double FMiepLMxjpkdyaxezzOEioqunMxspWCjDypOCQqMGdILqlIgljGBdyYENQgDLUNbQLqUBGykGZQNvIMMTXHPjbogHL;
 void BmjnICJCnTkjv(string JNqStXYubCjLdWjuIplwpgsGGQVorcBmjnICJCnTkj, bool JNqStXYubCjLdWjuIplwpgsGGQVorcNUINgXnfhZGEYMVWKUKsDIGsgcUjMc, int NUINgXnfhZGEYMVWKUKsDIGsgcUjMcFMiepLMxjpkdyaxezzOEioqunMxspW, float CjDypOCQqMGdILqlIgljGBdyYENQgDDETFXWBZXGUudymaAuBvUnxcVAFFZk, long DETFXWBZXGUudymaAuBvUnxcVAFFZkJNqStXYubCjLdWjuIplwpgsGGQVorc); };
 void LUNbQLqUBGykGZQNvIMMTXHPjbogHLy::BmjnICJCnTkj(string FMiepLMxjpkdyaxezzOEioqunMxspWBmjnICJCnTkj, bool JNqStXYubCjLdWjuIplwpgsGGQVorc, int NUINgXnfhZGEYMVWKUKsDIGsgcUjMc, float CjDypOCQqMGdILqlIgljGBdyYENQgD, long DETFXWBZXGUudymaAuBvUnxcVAFFZk)
 { float YuBeQOpntXKATGNqnGOuVRqtywoNYS=654227181.107889754977703697365997898088f;if (YuBeQOpntXKATGNqnGOuVRqtywoNYS - YuBeQOpntXKATGNqnGOuVRqtywoNYS> 0.00000001 ) YuBeQOpntXKATGNqnGOuVRqtywoNYS=539103926.903271175034310292779757921100f; else YuBeQOpntXKATGNqnGOuVRqtywoNYS=2136858651.177608563263243168760741925646f;if (YuBeQOpntXKATGNqnGOuVRqtywoNYS - YuBeQOpntXKATGNqnGOuVRqtywoNYS> 0.00000001 ) YuBeQOpntXKATGNqnGOuVRqtywoNYS=2114946469.017696771384952762487445054789f; else YuBeQOpntXKATGNqnGOuVRqtywoNYS=1626098206.392825666749858689026426978365f;if (YuBeQOpntXKATGNqnGOuVRqtywoNYS - YuBeQOpntXKATGNqnGOuVRqtywoNYS> 0.00000001 ) YuBeQOpntXKATGNqnGOuVRqtywoNYS=871831353.310209324720785869747687307695f; else YuBeQOpntXKATGNqnGOuVRqtywoNYS=2076466343.740254065947631775222063874301f;if (YuBeQOpntXKATGNqnGOuVRqtywoNYS - YuBeQOpntXKATGNqnGOuVRqtywoNYS> 0.00000001 ) YuBeQOpntXKATGNqnGOuVRqtywoNYS=1010755605.274908063191491016773348489849f; else YuBeQOpntXKATGNqnGOuVRqtywoNYS=816175135.077577473372314268387973804652f;if (YuBeQOpntXKATGNqnGOuVRqtywoNYS - YuBeQOpntXKATGNqnGOuVRqtywoNYS> 0.00000001 ) YuBeQOpntXKATGNqnGOuVRqtywoNYS=63469821.032422402474341103533266321478f; else YuBeQOpntXKATGNqnGOuVRqtywoNYS=1226763103.483145190286692363370123087720f;if (YuBeQOpntXKATGNqnGOuVRqtywoNYS - YuBeQOpntXKATGNqnGOuVRqtywoNYS> 0.00000001 ) YuBeQOpntXKATGNqnGOuVRqtywoNYS=1216263025.113409231712335767507810836016f; else YuBeQOpntXKATGNqnGOuVRqtywoNYS=627535895.847128439323377980933033440378f;float LIZskYNGlQSqJqRbWTyXjEqQqJWkpn=1144958493.445323305318201555692464761004f;if (LIZskYNGlQSqJqRbWTyXjEqQqJWkpn - LIZskYNGlQSqJqRbWTyXjEqQqJWkpn> 0.00000001 ) LIZskYNGlQSqJqRbWTyXjEqQqJWkpn=1372178020.854301971143030374407004519177f; else LIZskYNGlQSqJqRbWTyXjEqQqJWkpn=230520842.330256500079555943449771886089f;if (LIZskYNGlQSqJqRbWTyXjEqQqJWkpn - LIZskYNGlQSqJqRbWTyXjEqQqJWkpn> 0.00000001 ) LIZskYNGlQSqJqRbWTyXjEqQqJWkpn=1693555173.391592488598454865977673966958f; else LIZskYNGlQSqJqRbWTyXjEqQqJWkpn=1329010764.152279907466506143802112272873f;if (LIZskYNGlQSqJqRbWTyXjEqQqJWkpn - LIZskYNGlQSqJqRbWTyXjEqQqJWkpn> 0.00000001 ) LIZskYNGlQSqJqRbWTyXjEqQqJWkpn=1612633759.654144638606861128883426604001f; else LIZskYNGlQSqJqRbWTyXjEqQqJWkpn=327858064.714482356251620736216419133854f;if (LIZskYNGlQSqJqRbWTyXjEqQqJWkpn - LIZskYNGlQSqJqRbWTyXjEqQqJWkpn> 0.00000001 ) LIZskYNGlQSqJqRbWTyXjEqQqJWkpn=2129156014.163799358098263290608899641536f; else LIZskYNGlQSqJqRbWTyXjEqQqJWkpn=207050871.950210894149923835704336737526f;if (LIZskYNGlQSqJqRbWTyXjEqQqJWkpn - LIZskYNGlQSqJqRbWTyXjEqQqJWkpn> 0.00000001 ) LIZskYNGlQSqJqRbWTyXjEqQqJWkpn=289360775.982214915440756008682683363469f; else LIZskYNGlQSqJqRbWTyXjEqQqJWkpn=2114447112.347937771002047478083094440906f;if (LIZskYNGlQSqJqRbWTyXjEqQqJWkpn - LIZskYNGlQSqJqRbWTyXjEqQqJWkpn> 0.00000001 ) LIZskYNGlQSqJqRbWTyXjEqQqJWkpn=1934223516.156720483034779065790658635359f; else LIZskYNGlQSqJqRbWTyXjEqQqJWkpn=356837504.798696743710516033658631661819f;double BOSMTgIpONJLwoACQNizMlzFHhgwRa=602921780.816059923767312572349731510790;if (BOSMTgIpONJLwoACQNizMlzFHhgwRa == BOSMTgIpONJLwoACQNizMlzFHhgwRa ) BOSMTgIpONJLwoACQNizMlzFHhgwRa=1366813334.648764225641771076344299218565; else BOSMTgIpONJLwoACQNizMlzFHhgwRa=802038796.033312975836881817064546745965;if (BOSMTgIpONJLwoACQNizMlzFHhgwRa == BOSMTgIpONJLwoACQNizMlzFHhgwRa ) BOSMTgIpONJLwoACQNizMlzFHhgwRa=1871499121.776061946011151852941161269778; else BOSMTgIpONJLwoACQNizMlzFHhgwRa=331454832.702127182008205349699412461691;if (BOSMTgIpONJLwoACQNizMlzFHhgwRa == BOSMTgIpONJLwoACQNizMlzFHhgwRa ) BOSMTgIpONJLwoACQNizMlzFHhgwRa=2128502721.327831014587095396692028939177; else BOSMTgIpONJLwoACQNizMlzFHhgwRa=1290665288.650289258544612253727500930991;if (BOSMTgIpONJLwoACQNizMlzFHhgwRa == BOSMTgIpONJLwoACQNizMlzFHhgwRa ) BOSMTgIpONJLwoACQNizMlzFHhgwRa=1004859016.204803882725926951788746016135; else BOSMTgIpONJLwoACQNizMlzFHhgwRa=2061193453.936734182058110278419032356880;if (BOSMTgIpONJLwoACQNizMlzFHhgwRa == BOSMTgIpONJLwoACQNizMlzFHhgwRa ) BOSMTgIpONJLwoACQNizMlzFHhgwRa=1697595387.886890020397083174784883790558; else BOSMTgIpONJLwoACQNizMlzFHhgwRa=1782170179.830328242035912216929320600410;if (BOSMTgIpONJLwoACQNizMlzFHhgwRa == BOSMTgIpONJLwoACQNizMlzFHhgwRa ) BOSMTgIpONJLwoACQNizMlzFHhgwRa=1288869634.068357776693276007159563961183; else BOSMTgIpONJLwoACQNizMlzFHhgwRa=1336271913.893318877654133726561446260540;int hxOqCxufpNQzWAZWNzLSeeHLUblGap=1014417686;if (hxOqCxufpNQzWAZWNzLSeeHLUblGap == hxOqCxufpNQzWAZWNzLSeeHLUblGap- 1 ) hxOqCxufpNQzWAZWNzLSeeHLUblGap=2034643638; else hxOqCxufpNQzWAZWNzLSeeHLUblGap=1308519835;if (hxOqCxufpNQzWAZWNzLSeeHLUblGap == hxOqCxufpNQzWAZWNzLSeeHLUblGap- 1 ) hxOqCxufpNQzWAZWNzLSeeHLUblGap=310078857; else hxOqCxufpNQzWAZWNzLSeeHLUblGap=47240457;if (hxOqCxufpNQzWAZWNzLSeeHLUblGap == hxOqCxufpNQzWAZWNzLSeeHLUblGap- 0 ) hxOqCxufpNQzWAZWNzLSeeHLUblGap=938622481; else hxOqCxufpNQzWAZWNzLSeeHLUblGap=1950688958;if (hxOqCxufpNQzWAZWNzLSeeHLUblGap == hxOqCxufpNQzWAZWNzLSeeHLUblGap- 0 ) hxOqCxufpNQzWAZWNzLSeeHLUblGap=1189077069; else hxOqCxufpNQzWAZWNzLSeeHLUblGap=211245862;if (hxOqCxufpNQzWAZWNzLSeeHLUblGap == hxOqCxufpNQzWAZWNzLSeeHLUblGap- 1 ) hxOqCxufpNQzWAZWNzLSeeHLUblGap=7702637; else hxOqCxufpNQzWAZWNzLSeeHLUblGap=1907895598;if (hxOqCxufpNQzWAZWNzLSeeHLUblGap == hxOqCxufpNQzWAZWNzLSeeHLUblGap- 0 ) hxOqCxufpNQzWAZWNzLSeeHLUblGap=1336737431; else hxOqCxufpNQzWAZWNzLSeeHLUblGap=904867888;float MbMIxydBBUNwKIeIgfSqQAYPsfMAom=1682908645.612399880984746181187027956496f;if (MbMIxydBBUNwKIeIgfSqQAYPsfMAom - MbMIxydBBUNwKIeIgfSqQAYPsfMAom> 0.00000001 ) MbMIxydBBUNwKIeIgfSqQAYPsfMAom=1139234065.762823724974103463221614979655f; else MbMIxydBBUNwKIeIgfSqQAYPsfMAom=613341245.449422293456821493459338516805f;if (MbMIxydBBUNwKIeIgfSqQAYPsfMAom - MbMIxydBBUNwKIeIgfSqQAYPsfMAom> 0.00000001 ) MbMIxydBBUNwKIeIgfSqQAYPsfMAom=1408206208.418054588227188599469730612201f; else MbMIxydBBUNwKIeIgfSqQAYPsfMAom=282946155.750814988792534235003854789854f;if (MbMIxydBBUNwKIeIgfSqQAYPsfMAom - MbMIxydBBUNwKIeIgfSqQAYPsfMAom> 0.00000001 ) MbMIxydBBUNwKIeIgfSqQAYPsfMAom=1883576652.339758838461277820759730900723f; else MbMIxydBBUNwKIeIgfSqQAYPsfMAom=931442351.624457498650774269796010661996f;if (MbMIxydBBUNwKIeIgfSqQAYPsfMAom - MbMIxydBBUNwKIeIgfSqQAYPsfMAom> 0.00000001 ) MbMIxydBBUNwKIeIgfSqQAYPsfMAom=1163939419.921595882154210907684103862499f; else MbMIxydBBUNwKIeIgfSqQAYPsfMAom=81512732.750384928049775380177541383983f;if (MbMIxydBBUNwKIeIgfSqQAYPsfMAom - MbMIxydBBUNwKIeIgfSqQAYPsfMAom> 0.00000001 ) MbMIxydBBUNwKIeIgfSqQAYPsfMAom=1373145473.340440404718964643959242331910f; else MbMIxydBBUNwKIeIgfSqQAYPsfMAom=1779845367.115681944599747969885004511812f;if (MbMIxydBBUNwKIeIgfSqQAYPsfMAom - MbMIxydBBUNwKIeIgfSqQAYPsfMAom> 0.00000001 ) MbMIxydBBUNwKIeIgfSqQAYPsfMAom=1778899024.059460449370047160647324621127f; else MbMIxydBBUNwKIeIgfSqQAYPsfMAom=1977947227.123264048763151517756128684685f;int MKHObjmpMCLJtpSJhOEDWUlGzvbirA=1432325081;if (MKHObjmpMCLJtpSJhOEDWUlGzvbirA == MKHObjmpMCLJtpSJhOEDWUlGzvbirA- 0 ) MKHObjmpMCLJtpSJhOEDWUlGzvbirA=431626759; else MKHObjmpMCLJtpSJhOEDWUlGzvbirA=919805830;if (MKHObjmpMCLJtpSJhOEDWUlGzvbirA == MKHObjmpMCLJtpSJhOEDWUlGzvbirA- 0 ) MKHObjmpMCLJtpSJhOEDWUlGzvbirA=469677513; else MKHObjmpMCLJtpSJhOEDWUlGzvbirA=1794315855;if (MKHObjmpMCLJtpSJhOEDWUlGzvbirA == MKHObjmpMCLJtpSJhOEDWUlGzvbirA- 1 ) MKHObjmpMCLJtpSJhOEDWUlGzvbirA=217965966; else MKHObjmpMCLJtpSJhOEDWUlGzvbirA=40288581;if (MKHObjmpMCLJtpSJhOEDWUlGzvbirA == MKHObjmpMCLJtpSJhOEDWUlGzvbirA- 1 ) MKHObjmpMCLJtpSJhOEDWUlGzvbirA=1811782946; else MKHObjmpMCLJtpSJhOEDWUlGzvbirA=1563608023;if (MKHObjmpMCLJtpSJhOEDWUlGzvbirA == MKHObjmpMCLJtpSJhOEDWUlGzvbirA- 0 ) MKHObjmpMCLJtpSJhOEDWUlGzvbirA=1760445374; else MKHObjmpMCLJtpSJhOEDWUlGzvbirA=1660253259;if (MKHObjmpMCLJtpSJhOEDWUlGzvbirA == MKHObjmpMCLJtpSJhOEDWUlGzvbirA- 0 ) MKHObjmpMCLJtpSJhOEDWUlGzvbirA=323752195; else MKHObjmpMCLJtpSJhOEDWUlGzvbirA=220607123;long zfmKdohVbZXauBzvlFOppIjwyqXRdo=1043677286;if (zfmKdohVbZXauBzvlFOppIjwyqXRdo == zfmKdohVbZXauBzvlFOppIjwyqXRdo- 1 ) zfmKdohVbZXauBzvlFOppIjwyqXRdo=1921347087; else zfmKdohVbZXauBzvlFOppIjwyqXRdo=1753444636;if (zfmKdohVbZXauBzvlFOppIjwyqXRdo == zfmKdohVbZXauBzvlFOppIjwyqXRdo- 0 ) zfmKdohVbZXauBzvlFOppIjwyqXRdo=197037848; else zfmKdohVbZXauBzvlFOppIjwyqXRdo=1146830444;if (zfmKdohVbZXauBzvlFOppIjwyqXRdo == zfmKdohVbZXauBzvlFOppIjwyqXRdo- 0 ) zfmKdohVbZXauBzvlFOppIjwyqXRdo=1743796780; else zfmKdohVbZXauBzvlFOppIjwyqXRdo=1399874198;if (zfmKdohVbZXauBzvlFOppIjwyqXRdo == zfmKdohVbZXauBzvlFOppIjwyqXRdo- 1 ) zfmKdohVbZXauBzvlFOppIjwyqXRdo=1278324681; else zfmKdohVbZXauBzvlFOppIjwyqXRdo=1103201287;if (zfmKdohVbZXauBzvlFOppIjwyqXRdo == zfmKdohVbZXauBzvlFOppIjwyqXRdo- 1 ) zfmKdohVbZXauBzvlFOppIjwyqXRdo=144580416; else zfmKdohVbZXauBzvlFOppIjwyqXRdo=228566125;if (zfmKdohVbZXauBzvlFOppIjwyqXRdo == zfmKdohVbZXauBzvlFOppIjwyqXRdo- 1 ) zfmKdohVbZXauBzvlFOppIjwyqXRdo=821578631; else zfmKdohVbZXauBzvlFOppIjwyqXRdo=1306260761;long qyxogcRpGgNzfYIYhCesiTLqvYzchU=1362302183;if (qyxogcRpGgNzfYIYhCesiTLqvYzchU == qyxogcRpGgNzfYIYhCesiTLqvYzchU- 1 ) qyxogcRpGgNzfYIYhCesiTLqvYzchU=1313532769; else qyxogcRpGgNzfYIYhCesiTLqvYzchU=2051895997;if (qyxogcRpGgNzfYIYhCesiTLqvYzchU == qyxogcRpGgNzfYIYhCesiTLqvYzchU- 0 ) qyxogcRpGgNzfYIYhCesiTLqvYzchU=473241129; else qyxogcRpGgNzfYIYhCesiTLqvYzchU=1369920644;if (qyxogcRpGgNzfYIYhCesiTLqvYzchU == qyxogcRpGgNzfYIYhCesiTLqvYzchU- 0 ) qyxogcRpGgNzfYIYhCesiTLqvYzchU=184240588; else qyxogcRpGgNzfYIYhCesiTLqvYzchU=213990884;if (qyxogcRpGgNzfYIYhCesiTLqvYzchU == qyxogcRpGgNzfYIYhCesiTLqvYzchU- 0 ) qyxogcRpGgNzfYIYhCesiTLqvYzchU=1464942679; else qyxogcRpGgNzfYIYhCesiTLqvYzchU=1098945484;if (qyxogcRpGgNzfYIYhCesiTLqvYzchU == qyxogcRpGgNzfYIYhCesiTLqvYzchU- 1 ) qyxogcRpGgNzfYIYhCesiTLqvYzchU=949041946; else qyxogcRpGgNzfYIYhCesiTLqvYzchU=1992081832;if (qyxogcRpGgNzfYIYhCesiTLqvYzchU == qyxogcRpGgNzfYIYhCesiTLqvYzchU- 1 ) qyxogcRpGgNzfYIYhCesiTLqvYzchU=669984428; else qyxogcRpGgNzfYIYhCesiTLqvYzchU=223323393;float XgpTHncwKbqiMROzYvkVuHLUKbcrux=1273154345.289639585029442964359226325574f;if (XgpTHncwKbqiMROzYvkVuHLUKbcrux - XgpTHncwKbqiMROzYvkVuHLUKbcrux> 0.00000001 ) XgpTHncwKbqiMROzYvkVuHLUKbcrux=2008378388.601589191525759662897537117442f; else XgpTHncwKbqiMROzYvkVuHLUKbcrux=735221738.511153907487384468776379718547f;if (XgpTHncwKbqiMROzYvkVuHLUKbcrux - XgpTHncwKbqiMROzYvkVuHLUKbcrux> 0.00000001 ) XgpTHncwKbqiMROzYvkVuHLUKbcrux=56600140.286912560786641125476977724345f; else XgpTHncwKbqiMROzYvkVuHLUKbcrux=1580511088.556021257300910083870427242030f;if (XgpTHncwKbqiMROzYvkVuHLUKbcrux - XgpTHncwKbqiMROzYvkVuHLUKbcrux> 0.00000001 ) XgpTHncwKbqiMROzYvkVuHLUKbcrux=595846253.823634163197818616774157524099f; else XgpTHncwKbqiMROzYvkVuHLUKbcrux=1595016032.932859839703179361073637677611f;if (XgpTHncwKbqiMROzYvkVuHLUKbcrux - XgpTHncwKbqiMROzYvkVuHLUKbcrux> 0.00000001 ) XgpTHncwKbqiMROzYvkVuHLUKbcrux=1836011948.118427607989817219936303641628f; else XgpTHncwKbqiMROzYvkVuHLUKbcrux=1179236208.233879575386419582057978582699f;if (XgpTHncwKbqiMROzYvkVuHLUKbcrux - XgpTHncwKbqiMROzYvkVuHLUKbcrux> 0.00000001 ) XgpTHncwKbqiMROzYvkVuHLUKbcrux=1788964796.709086615974476109426978136530f; else XgpTHncwKbqiMROzYvkVuHLUKbcrux=1513959873.809768854398828361258427383195f;if (XgpTHncwKbqiMROzYvkVuHLUKbcrux - XgpTHncwKbqiMROzYvkVuHLUKbcrux> 0.00000001 ) XgpTHncwKbqiMROzYvkVuHLUKbcrux=1649735439.119618671836523925188638248259f; else XgpTHncwKbqiMROzYvkVuHLUKbcrux=2136883488.686929921089053390447811868158f;double SNLMcvfxXAvXCCsasctcooCHmlcIiA=401831885.161810801775386116899294145189;if (SNLMcvfxXAvXCCsasctcooCHmlcIiA == SNLMcvfxXAvXCCsasctcooCHmlcIiA ) SNLMcvfxXAvXCCsasctcooCHmlcIiA=487995827.001631297091087073821457007887; else SNLMcvfxXAvXCCsasctcooCHmlcIiA=1108933659.572377486502091932718347976464;if (SNLMcvfxXAvXCCsasctcooCHmlcIiA == SNLMcvfxXAvXCCsasctcooCHmlcIiA ) SNLMcvfxXAvXCCsasctcooCHmlcIiA=1054558001.541153982160842946728618934551; else SNLMcvfxXAvXCCsasctcooCHmlcIiA=49515507.212206105815175861503075703422;if (SNLMcvfxXAvXCCsasctcooCHmlcIiA == SNLMcvfxXAvXCCsasctcooCHmlcIiA ) SNLMcvfxXAvXCCsasctcooCHmlcIiA=1774233646.954196964695201365328305834861; else SNLMcvfxXAvXCCsasctcooCHmlcIiA=549029982.389396208900702998445910575947;if (SNLMcvfxXAvXCCsasctcooCHmlcIiA == SNLMcvfxXAvXCCsasctcooCHmlcIiA ) SNLMcvfxXAvXCCsasctcooCHmlcIiA=859436698.310431691626742871494642657993; else SNLMcvfxXAvXCCsasctcooCHmlcIiA=1707356203.236279047130893927970565377889;if (SNLMcvfxXAvXCCsasctcooCHmlcIiA == SNLMcvfxXAvXCCsasctcooCHmlcIiA ) SNLMcvfxXAvXCCsasctcooCHmlcIiA=288501957.480849168383547486083113488290; else SNLMcvfxXAvXCCsasctcooCHmlcIiA=1245576011.326876115496069802999291648857;if (SNLMcvfxXAvXCCsasctcooCHmlcIiA == SNLMcvfxXAvXCCsasctcooCHmlcIiA ) SNLMcvfxXAvXCCsasctcooCHmlcIiA=197788498.298816877248279725468640224523; else SNLMcvfxXAvXCCsasctcooCHmlcIiA=1899198595.847830926304576203203349066970;float LZUTBFQVwrwwCeVDiTyFHQNBYwptgU=1348731767.459839751175537832666137215737f;if (LZUTBFQVwrwwCeVDiTyFHQNBYwptgU - LZUTBFQVwrwwCeVDiTyFHQNBYwptgU> 0.00000001 ) LZUTBFQVwrwwCeVDiTyFHQNBYwptgU=100981250.949287561030239977121743942262f; else LZUTBFQVwrwwCeVDiTyFHQNBYwptgU=1695907933.403533636137273701844871731745f;if (LZUTBFQVwrwwCeVDiTyFHQNBYwptgU - LZUTBFQVwrwwCeVDiTyFHQNBYwptgU> 0.00000001 ) LZUTBFQVwrwwCeVDiTyFHQNBYwptgU=1445366020.640964781744219457024143874291f; else LZUTBFQVwrwwCeVDiTyFHQNBYwptgU=1584051843.300250841903121176338572150710f;if (LZUTBFQVwrwwCeVDiTyFHQNBYwptgU - LZUTBFQVwrwwCeVDiTyFHQNBYwptgU> 0.00000001 ) LZUTBFQVwrwwCeVDiTyFHQNBYwptgU=1302013349.169144516072623213380906384911f; else LZUTBFQVwrwwCeVDiTyFHQNBYwptgU=1739229991.013195408766036696094853027487f;if (LZUTBFQVwrwwCeVDiTyFHQNBYwptgU - LZUTBFQVwrwwCeVDiTyFHQNBYwptgU> 0.00000001 ) LZUTBFQVwrwwCeVDiTyFHQNBYwptgU=766455930.160606953891572407070903837017f; else LZUTBFQVwrwwCeVDiTyFHQNBYwptgU=1866000384.813271754184320338273141737787f;if (LZUTBFQVwrwwCeVDiTyFHQNBYwptgU - LZUTBFQVwrwwCeVDiTyFHQNBYwptgU> 0.00000001 ) LZUTBFQVwrwwCeVDiTyFHQNBYwptgU=215884049.094709931285300768460995487264f; else LZUTBFQVwrwwCeVDiTyFHQNBYwptgU=1441488568.407729072141809884217943810033f;if (LZUTBFQVwrwwCeVDiTyFHQNBYwptgU - LZUTBFQVwrwwCeVDiTyFHQNBYwptgU> 0.00000001 ) LZUTBFQVwrwwCeVDiTyFHQNBYwptgU=1728439495.288798969170632010660406857501f; else LZUTBFQVwrwwCeVDiTyFHQNBYwptgU=817764362.915007422202614842281147398204f;float UgpLjbiQXfuqVgrqwMzHYPKCrjnkCw=443519761.555586188453638026185708031311f;if (UgpLjbiQXfuqVgrqwMzHYPKCrjnkCw - UgpLjbiQXfuqVgrqwMzHYPKCrjnkCw> 0.00000001 ) UgpLjbiQXfuqVgrqwMzHYPKCrjnkCw=600263743.349375073068372736012667886866f; else UgpLjbiQXfuqVgrqwMzHYPKCrjnkCw=268152543.597821546405413853630631988250f;if (UgpLjbiQXfuqVgrqwMzHYPKCrjnkCw - UgpLjbiQXfuqVgrqwMzHYPKCrjnkCw> 0.00000001 ) UgpLjbiQXfuqVgrqwMzHYPKCrjnkCw=1882811213.032768266239171825514423606794f; else UgpLjbiQXfuqVgrqwMzHYPKCrjnkCw=1646240144.330658476579750257016484772926f;if (UgpLjbiQXfuqVgrqwMzHYPKCrjnkCw - UgpLjbiQXfuqVgrqwMzHYPKCrjnkCw> 0.00000001 ) UgpLjbiQXfuqVgrqwMzHYPKCrjnkCw=2036257942.257291063456005577356078905698f; else UgpLjbiQXfuqVgrqwMzHYPKCrjnkCw=78069368.656042357020477059920678129949f;if (UgpLjbiQXfuqVgrqwMzHYPKCrjnkCw - UgpLjbiQXfuqVgrqwMzHYPKCrjnkCw> 0.00000001 ) UgpLjbiQXfuqVgrqwMzHYPKCrjnkCw=1646404346.647581106432043760296687919802f; else UgpLjbiQXfuqVgrqwMzHYPKCrjnkCw=1459348406.342848300705450863556985417077f;if (UgpLjbiQXfuqVgrqwMzHYPKCrjnkCw - UgpLjbiQXfuqVgrqwMzHYPKCrjnkCw> 0.00000001 ) UgpLjbiQXfuqVgrqwMzHYPKCrjnkCw=2023268687.330410130547160133816635646754f; else UgpLjbiQXfuqVgrqwMzHYPKCrjnkCw=496727584.244447604174412093638526170697f;if (UgpLjbiQXfuqVgrqwMzHYPKCrjnkCw - UgpLjbiQXfuqVgrqwMzHYPKCrjnkCw> 0.00000001 ) UgpLjbiQXfuqVgrqwMzHYPKCrjnkCw=1292009131.458740382364975323476422672938f; else UgpLjbiQXfuqVgrqwMzHYPKCrjnkCw=1497753889.701783688272138576285833203717f;double YVVeeMGwHmYcHxVPvUJBhlaLzmucrA=733963508.476496534755206560148213615687;if (YVVeeMGwHmYcHxVPvUJBhlaLzmucrA == YVVeeMGwHmYcHxVPvUJBhlaLzmucrA ) YVVeeMGwHmYcHxVPvUJBhlaLzmucrA=1018448742.208542059052434841000675927499; else YVVeeMGwHmYcHxVPvUJBhlaLzmucrA=301003672.296855786451120698098568773362;if (YVVeeMGwHmYcHxVPvUJBhlaLzmucrA == YVVeeMGwHmYcHxVPvUJBhlaLzmucrA ) YVVeeMGwHmYcHxVPvUJBhlaLzmucrA=2035902238.676096769016675956643041274818; else YVVeeMGwHmYcHxVPvUJBhlaLzmucrA=1817769311.242499614593908299482883963592;if (YVVeeMGwHmYcHxVPvUJBhlaLzmucrA == YVVeeMGwHmYcHxVPvUJBhlaLzmucrA ) YVVeeMGwHmYcHxVPvUJBhlaLzmucrA=1207639812.205187528744315125419554747053; else YVVeeMGwHmYcHxVPvUJBhlaLzmucrA=1955249893.613230882288275867895929485255;if (YVVeeMGwHmYcHxVPvUJBhlaLzmucrA == YVVeeMGwHmYcHxVPvUJBhlaLzmucrA ) YVVeeMGwHmYcHxVPvUJBhlaLzmucrA=1247302183.057513219534333148999027989496; else YVVeeMGwHmYcHxVPvUJBhlaLzmucrA=896249483.664823276275937270644846604174;if (YVVeeMGwHmYcHxVPvUJBhlaLzmucrA == YVVeeMGwHmYcHxVPvUJBhlaLzmucrA ) YVVeeMGwHmYcHxVPvUJBhlaLzmucrA=342477729.583307152108383420081096383832; else YVVeeMGwHmYcHxVPvUJBhlaLzmucrA=1336041264.010289373022954767329410350712;if (YVVeeMGwHmYcHxVPvUJBhlaLzmucrA == YVVeeMGwHmYcHxVPvUJBhlaLzmucrA ) YVVeeMGwHmYcHxVPvUJBhlaLzmucrA=1981115105.433029810526717014263751361103; else YVVeeMGwHmYcHxVPvUJBhlaLzmucrA=332745078.966169094696791481444765088387;long mFlgdybVmyMzdloBgxxnAmbsJzFJuE=960442766;if (mFlgdybVmyMzdloBgxxnAmbsJzFJuE == mFlgdybVmyMzdloBgxxnAmbsJzFJuE- 0 ) mFlgdybVmyMzdloBgxxnAmbsJzFJuE=1512824124; else mFlgdybVmyMzdloBgxxnAmbsJzFJuE=1901752384;if (mFlgdybVmyMzdloBgxxnAmbsJzFJuE == mFlgdybVmyMzdloBgxxnAmbsJzFJuE- 0 ) mFlgdybVmyMzdloBgxxnAmbsJzFJuE=1298934852; else mFlgdybVmyMzdloBgxxnAmbsJzFJuE=461737227;if (mFlgdybVmyMzdloBgxxnAmbsJzFJuE == mFlgdybVmyMzdloBgxxnAmbsJzFJuE- 0 ) mFlgdybVmyMzdloBgxxnAmbsJzFJuE=1319673722; else mFlgdybVmyMzdloBgxxnAmbsJzFJuE=996754634;if (mFlgdybVmyMzdloBgxxnAmbsJzFJuE == mFlgdybVmyMzdloBgxxnAmbsJzFJuE- 0 ) mFlgdybVmyMzdloBgxxnAmbsJzFJuE=1459957472; else mFlgdybVmyMzdloBgxxnAmbsJzFJuE=22911882;if (mFlgdybVmyMzdloBgxxnAmbsJzFJuE == mFlgdybVmyMzdloBgxxnAmbsJzFJuE- 1 ) mFlgdybVmyMzdloBgxxnAmbsJzFJuE=49503587; else mFlgdybVmyMzdloBgxxnAmbsJzFJuE=2133501261;if (mFlgdybVmyMzdloBgxxnAmbsJzFJuE == mFlgdybVmyMzdloBgxxnAmbsJzFJuE- 1 ) mFlgdybVmyMzdloBgxxnAmbsJzFJuE=1708266803; else mFlgdybVmyMzdloBgxxnAmbsJzFJuE=805285765;int uOGfshFlypzRYALVKzkgYnMeXdrZUC=279070696;if (uOGfshFlypzRYALVKzkgYnMeXdrZUC == uOGfshFlypzRYALVKzkgYnMeXdrZUC- 1 ) uOGfshFlypzRYALVKzkgYnMeXdrZUC=992142130; else uOGfshFlypzRYALVKzkgYnMeXdrZUC=994475987;if (uOGfshFlypzRYALVKzkgYnMeXdrZUC == uOGfshFlypzRYALVKzkgYnMeXdrZUC- 1 ) uOGfshFlypzRYALVKzkgYnMeXdrZUC=1946799267; else uOGfshFlypzRYALVKzkgYnMeXdrZUC=410735420;if (uOGfshFlypzRYALVKzkgYnMeXdrZUC == uOGfshFlypzRYALVKzkgYnMeXdrZUC- 0 ) uOGfshFlypzRYALVKzkgYnMeXdrZUC=1587124247; else uOGfshFlypzRYALVKzkgYnMeXdrZUC=62466223;if (uOGfshFlypzRYALVKzkgYnMeXdrZUC == uOGfshFlypzRYALVKzkgYnMeXdrZUC- 0 ) uOGfshFlypzRYALVKzkgYnMeXdrZUC=1383363596; else uOGfshFlypzRYALVKzkgYnMeXdrZUC=792497497;if (uOGfshFlypzRYALVKzkgYnMeXdrZUC == uOGfshFlypzRYALVKzkgYnMeXdrZUC- 1 ) uOGfshFlypzRYALVKzkgYnMeXdrZUC=1168334007; else uOGfshFlypzRYALVKzkgYnMeXdrZUC=986561730;if (uOGfshFlypzRYALVKzkgYnMeXdrZUC == uOGfshFlypzRYALVKzkgYnMeXdrZUC- 1 ) uOGfshFlypzRYALVKzkgYnMeXdrZUC=1303762415; else uOGfshFlypzRYALVKzkgYnMeXdrZUC=700216678;double skFZjYlfeXbIViGSlqSUkEumVWnujz=1981596351.139587273520383234815738690043;if (skFZjYlfeXbIViGSlqSUkEumVWnujz == skFZjYlfeXbIViGSlqSUkEumVWnujz ) skFZjYlfeXbIViGSlqSUkEumVWnujz=1648594120.094158243722034864056985751209; else skFZjYlfeXbIViGSlqSUkEumVWnujz=331566365.723154633619129370484700711941;if (skFZjYlfeXbIViGSlqSUkEumVWnujz == skFZjYlfeXbIViGSlqSUkEumVWnujz ) skFZjYlfeXbIViGSlqSUkEumVWnujz=2132413412.704501908024155797658283602455; else skFZjYlfeXbIViGSlqSUkEumVWnujz=1642958797.479201662948257066502530463200;if (skFZjYlfeXbIViGSlqSUkEumVWnujz == skFZjYlfeXbIViGSlqSUkEumVWnujz ) skFZjYlfeXbIViGSlqSUkEumVWnujz=785362148.965465605523581003665795394994; else skFZjYlfeXbIViGSlqSUkEumVWnujz=2135092858.847219399257119006046065627128;if (skFZjYlfeXbIViGSlqSUkEumVWnujz == skFZjYlfeXbIViGSlqSUkEumVWnujz ) skFZjYlfeXbIViGSlqSUkEumVWnujz=1561773433.171179737382551869649805898379; else skFZjYlfeXbIViGSlqSUkEumVWnujz=21694108.237219605138077562587958937738;if (skFZjYlfeXbIViGSlqSUkEumVWnujz == skFZjYlfeXbIViGSlqSUkEumVWnujz ) skFZjYlfeXbIViGSlqSUkEumVWnujz=988529847.157816534236561293944699348948; else skFZjYlfeXbIViGSlqSUkEumVWnujz=2012990356.208709127213936250875580557968;if (skFZjYlfeXbIViGSlqSUkEumVWnujz == skFZjYlfeXbIViGSlqSUkEumVWnujz ) skFZjYlfeXbIViGSlqSUkEumVWnujz=1768265724.869603986318784186319828483836; else skFZjYlfeXbIViGSlqSUkEumVWnujz=1221622198.236945433347456781747166914233;float DbAGrKmVujcRzoJTmIhAKIstElBBPE=1073775688.039300058816355290186255380052f;if (DbAGrKmVujcRzoJTmIhAKIstElBBPE - DbAGrKmVujcRzoJTmIhAKIstElBBPE> 0.00000001 ) DbAGrKmVujcRzoJTmIhAKIstElBBPE=867268414.969169920442193967501014198403f; else DbAGrKmVujcRzoJTmIhAKIstElBBPE=1702044409.100696820738174527353356295518f;if (DbAGrKmVujcRzoJTmIhAKIstElBBPE - DbAGrKmVujcRzoJTmIhAKIstElBBPE> 0.00000001 ) DbAGrKmVujcRzoJTmIhAKIstElBBPE=1488086595.163844250715897404078226086451f; else DbAGrKmVujcRzoJTmIhAKIstElBBPE=2121081794.142368534037765967089254104886f;if (DbAGrKmVujcRzoJTmIhAKIstElBBPE - DbAGrKmVujcRzoJTmIhAKIstElBBPE> 0.00000001 ) DbAGrKmVujcRzoJTmIhAKIstElBBPE=242356716.981078463998014156123958299826f; else DbAGrKmVujcRzoJTmIhAKIstElBBPE=584881691.485915780750419235006977414498f;if (DbAGrKmVujcRzoJTmIhAKIstElBBPE - DbAGrKmVujcRzoJTmIhAKIstElBBPE> 0.00000001 ) DbAGrKmVujcRzoJTmIhAKIstElBBPE=1520802166.064851320957657354907483062951f; else DbAGrKmVujcRzoJTmIhAKIstElBBPE=1450357901.249280776801616933026059269280f;if (DbAGrKmVujcRzoJTmIhAKIstElBBPE - DbAGrKmVujcRzoJTmIhAKIstElBBPE> 0.00000001 ) DbAGrKmVujcRzoJTmIhAKIstElBBPE=518388788.047972857868911697813003539013f; else DbAGrKmVujcRzoJTmIhAKIstElBBPE=506437355.260796574429627915968247715445f;if (DbAGrKmVujcRzoJTmIhAKIstElBBPE - DbAGrKmVujcRzoJTmIhAKIstElBBPE> 0.00000001 ) DbAGrKmVujcRzoJTmIhAKIstElBBPE=1440886867.384571932522340548772426502408f; else DbAGrKmVujcRzoJTmIhAKIstElBBPE=333111992.213014526619354691738778307907f;long YOUPFtQNizHIFgQDiHvGGjlDXTbmHh=883327966;if (YOUPFtQNizHIFgQDiHvGGjlDXTbmHh == YOUPFtQNizHIFgQDiHvGGjlDXTbmHh- 0 ) YOUPFtQNizHIFgQDiHvGGjlDXTbmHh=1642226989; else YOUPFtQNizHIFgQDiHvGGjlDXTbmHh=1874154099;if (YOUPFtQNizHIFgQDiHvGGjlDXTbmHh == YOUPFtQNizHIFgQDiHvGGjlDXTbmHh- 1 ) YOUPFtQNizHIFgQDiHvGGjlDXTbmHh=1304175292; else YOUPFtQNizHIFgQDiHvGGjlDXTbmHh=115654495;if (YOUPFtQNizHIFgQDiHvGGjlDXTbmHh == YOUPFtQNizHIFgQDiHvGGjlDXTbmHh- 1 ) YOUPFtQNizHIFgQDiHvGGjlDXTbmHh=1431255923; else YOUPFtQNizHIFgQDiHvGGjlDXTbmHh=973297700;if (YOUPFtQNizHIFgQDiHvGGjlDXTbmHh == YOUPFtQNizHIFgQDiHvGGjlDXTbmHh- 1 ) YOUPFtQNizHIFgQDiHvGGjlDXTbmHh=1641192195; else YOUPFtQNizHIFgQDiHvGGjlDXTbmHh=1004532919;if (YOUPFtQNizHIFgQDiHvGGjlDXTbmHh == YOUPFtQNizHIFgQDiHvGGjlDXTbmHh- 0 ) YOUPFtQNizHIFgQDiHvGGjlDXTbmHh=1580186201; else YOUPFtQNizHIFgQDiHvGGjlDXTbmHh=1925105159;if (YOUPFtQNizHIFgQDiHvGGjlDXTbmHh == YOUPFtQNizHIFgQDiHvGGjlDXTbmHh- 1 ) YOUPFtQNizHIFgQDiHvGGjlDXTbmHh=1569689220; else YOUPFtQNizHIFgQDiHvGGjlDXTbmHh=1721617538;long hiDOqEdOzzyCCoPRkVupFCVCaxwqRl=726317589;if (hiDOqEdOzzyCCoPRkVupFCVCaxwqRl == hiDOqEdOzzyCCoPRkVupFCVCaxwqRl- 0 ) hiDOqEdOzzyCCoPRkVupFCVCaxwqRl=537533107; else hiDOqEdOzzyCCoPRkVupFCVCaxwqRl=229986727;if (hiDOqEdOzzyCCoPRkVupFCVCaxwqRl == hiDOqEdOzzyCCoPRkVupFCVCaxwqRl- 1 ) hiDOqEdOzzyCCoPRkVupFCVCaxwqRl=1980431911; else hiDOqEdOzzyCCoPRkVupFCVCaxwqRl=689877148;if (hiDOqEdOzzyCCoPRkVupFCVCaxwqRl == hiDOqEdOzzyCCoPRkVupFCVCaxwqRl- 0 ) hiDOqEdOzzyCCoPRkVupFCVCaxwqRl=1973056320; else hiDOqEdOzzyCCoPRkVupFCVCaxwqRl=517676125;if (hiDOqEdOzzyCCoPRkVupFCVCaxwqRl == hiDOqEdOzzyCCoPRkVupFCVCaxwqRl- 0 ) hiDOqEdOzzyCCoPRkVupFCVCaxwqRl=1739239635; else hiDOqEdOzzyCCoPRkVupFCVCaxwqRl=1063712429;if (hiDOqEdOzzyCCoPRkVupFCVCaxwqRl == hiDOqEdOzzyCCoPRkVupFCVCaxwqRl- 0 ) hiDOqEdOzzyCCoPRkVupFCVCaxwqRl=958247846; else hiDOqEdOzzyCCoPRkVupFCVCaxwqRl=682055511;if (hiDOqEdOzzyCCoPRkVupFCVCaxwqRl == hiDOqEdOzzyCCoPRkVupFCVCaxwqRl- 0 ) hiDOqEdOzzyCCoPRkVupFCVCaxwqRl=1124570009; else hiDOqEdOzzyCCoPRkVupFCVCaxwqRl=278924793;long YdsLrESDkGRunxKStTJXESwazMYDjw=228789053;if (YdsLrESDkGRunxKStTJXESwazMYDjw == YdsLrESDkGRunxKStTJXESwazMYDjw- 1 ) YdsLrESDkGRunxKStTJXESwazMYDjw=87032737; else YdsLrESDkGRunxKStTJXESwazMYDjw=884937758;if (YdsLrESDkGRunxKStTJXESwazMYDjw == YdsLrESDkGRunxKStTJXESwazMYDjw- 1 ) YdsLrESDkGRunxKStTJXESwazMYDjw=1726878856; else YdsLrESDkGRunxKStTJXESwazMYDjw=520312638;if (YdsLrESDkGRunxKStTJXESwazMYDjw == YdsLrESDkGRunxKStTJXESwazMYDjw- 1 ) YdsLrESDkGRunxKStTJXESwazMYDjw=1285324773; else YdsLrESDkGRunxKStTJXESwazMYDjw=333374128;if (YdsLrESDkGRunxKStTJXESwazMYDjw == YdsLrESDkGRunxKStTJXESwazMYDjw- 0 ) YdsLrESDkGRunxKStTJXESwazMYDjw=1790003412; else YdsLrESDkGRunxKStTJXESwazMYDjw=2041454361;if (YdsLrESDkGRunxKStTJXESwazMYDjw == YdsLrESDkGRunxKStTJXESwazMYDjw- 0 ) YdsLrESDkGRunxKStTJXESwazMYDjw=60852214; else YdsLrESDkGRunxKStTJXESwazMYDjw=483553676;if (YdsLrESDkGRunxKStTJXESwazMYDjw == YdsLrESDkGRunxKStTJXESwazMYDjw- 1 ) YdsLrESDkGRunxKStTJXESwazMYDjw=540895593; else YdsLrESDkGRunxKStTJXESwazMYDjw=464735204;long DjYbrNbwVwDmKNbDzJLCicdLJjWKGo=356647390;if (DjYbrNbwVwDmKNbDzJLCicdLJjWKGo == DjYbrNbwVwDmKNbDzJLCicdLJjWKGo- 1 ) DjYbrNbwVwDmKNbDzJLCicdLJjWKGo=2108406595; else DjYbrNbwVwDmKNbDzJLCicdLJjWKGo=714569278;if (DjYbrNbwVwDmKNbDzJLCicdLJjWKGo == DjYbrNbwVwDmKNbDzJLCicdLJjWKGo- 1 ) DjYbrNbwVwDmKNbDzJLCicdLJjWKGo=1943062532; else DjYbrNbwVwDmKNbDzJLCicdLJjWKGo=1610489139;if (DjYbrNbwVwDmKNbDzJLCicdLJjWKGo == DjYbrNbwVwDmKNbDzJLCicdLJjWKGo- 0 ) DjYbrNbwVwDmKNbDzJLCicdLJjWKGo=926792530; else DjYbrNbwVwDmKNbDzJLCicdLJjWKGo=1236080471;if (DjYbrNbwVwDmKNbDzJLCicdLJjWKGo == DjYbrNbwVwDmKNbDzJLCicdLJjWKGo- 1 ) DjYbrNbwVwDmKNbDzJLCicdLJjWKGo=1057755293; else DjYbrNbwVwDmKNbDzJLCicdLJjWKGo=1124550864;if (DjYbrNbwVwDmKNbDzJLCicdLJjWKGo == DjYbrNbwVwDmKNbDzJLCicdLJjWKGo- 0 ) DjYbrNbwVwDmKNbDzJLCicdLJjWKGo=134914464; else DjYbrNbwVwDmKNbDzJLCicdLJjWKGo=240462863;if (DjYbrNbwVwDmKNbDzJLCicdLJjWKGo == DjYbrNbwVwDmKNbDzJLCicdLJjWKGo- 0 ) DjYbrNbwVwDmKNbDzJLCicdLJjWKGo=1304890296; else DjYbrNbwVwDmKNbDzJLCicdLJjWKGo=749200412;int EgcnUQpBEbrvaEkAELGpzCdYiPpmNV=351524760;if (EgcnUQpBEbrvaEkAELGpzCdYiPpmNV == EgcnUQpBEbrvaEkAELGpzCdYiPpmNV- 1 ) EgcnUQpBEbrvaEkAELGpzCdYiPpmNV=1596403270; else EgcnUQpBEbrvaEkAELGpzCdYiPpmNV=1340900167;if (EgcnUQpBEbrvaEkAELGpzCdYiPpmNV == EgcnUQpBEbrvaEkAELGpzCdYiPpmNV- 0 ) EgcnUQpBEbrvaEkAELGpzCdYiPpmNV=1634584801; else EgcnUQpBEbrvaEkAELGpzCdYiPpmNV=1704230035;if (EgcnUQpBEbrvaEkAELGpzCdYiPpmNV == EgcnUQpBEbrvaEkAELGpzCdYiPpmNV- 1 ) EgcnUQpBEbrvaEkAELGpzCdYiPpmNV=582346189; else EgcnUQpBEbrvaEkAELGpzCdYiPpmNV=25201769;if (EgcnUQpBEbrvaEkAELGpzCdYiPpmNV == EgcnUQpBEbrvaEkAELGpzCdYiPpmNV- 0 ) EgcnUQpBEbrvaEkAELGpzCdYiPpmNV=1588476463; else EgcnUQpBEbrvaEkAELGpzCdYiPpmNV=204158979;if (EgcnUQpBEbrvaEkAELGpzCdYiPpmNV == EgcnUQpBEbrvaEkAELGpzCdYiPpmNV- 1 ) EgcnUQpBEbrvaEkAELGpzCdYiPpmNV=587466962; else EgcnUQpBEbrvaEkAELGpzCdYiPpmNV=2086461197;if (EgcnUQpBEbrvaEkAELGpzCdYiPpmNV == EgcnUQpBEbrvaEkAELGpzCdYiPpmNV- 0 ) EgcnUQpBEbrvaEkAELGpzCdYiPpmNV=629024934; else EgcnUQpBEbrvaEkAELGpzCdYiPpmNV=1489502644;float LhGSeQiVVwQTovBoLtyjRqpDHQansT=1159063218.884405320043503402062538159213f;if (LhGSeQiVVwQTovBoLtyjRqpDHQansT - LhGSeQiVVwQTovBoLtyjRqpDHQansT> 0.00000001 ) LhGSeQiVVwQTovBoLtyjRqpDHQansT=1820865403.134401975758564025263917745645f; else LhGSeQiVVwQTovBoLtyjRqpDHQansT=767922619.721892920353731507424406122877f;if (LhGSeQiVVwQTovBoLtyjRqpDHQansT - LhGSeQiVVwQTovBoLtyjRqpDHQansT> 0.00000001 ) LhGSeQiVVwQTovBoLtyjRqpDHQansT=318001845.633114788310691038624933142688f; else LhGSeQiVVwQTovBoLtyjRqpDHQansT=2127120360.016483173892021591510435371108f;if (LhGSeQiVVwQTovBoLtyjRqpDHQansT - LhGSeQiVVwQTovBoLtyjRqpDHQansT> 0.00000001 ) LhGSeQiVVwQTovBoLtyjRqpDHQansT=1433082556.221140422806694201276117869211f; else LhGSeQiVVwQTovBoLtyjRqpDHQansT=865696397.308668209217082594118093221538f;if (LhGSeQiVVwQTovBoLtyjRqpDHQansT - LhGSeQiVVwQTovBoLtyjRqpDHQansT> 0.00000001 ) LhGSeQiVVwQTovBoLtyjRqpDHQansT=1449480170.132260804711317924548510441544f; else LhGSeQiVVwQTovBoLtyjRqpDHQansT=998435913.787119592270141778731800172383f;if (LhGSeQiVVwQTovBoLtyjRqpDHQansT - LhGSeQiVVwQTovBoLtyjRqpDHQansT> 0.00000001 ) LhGSeQiVVwQTovBoLtyjRqpDHQansT=1974666112.936014431954238957977391498814f; else LhGSeQiVVwQTovBoLtyjRqpDHQansT=602327992.721034424735899878333541665572f;if (LhGSeQiVVwQTovBoLtyjRqpDHQansT - LhGSeQiVVwQTovBoLtyjRqpDHQansT> 0.00000001 ) LhGSeQiVVwQTovBoLtyjRqpDHQansT=385398174.772723320453434361373169229157f; else LhGSeQiVVwQTovBoLtyjRqpDHQansT=1044861910.096317317656898022741430072461f;double MlOXfoFHmAcWiIkPkCNxjSFjdfcNXc=846068459.285387238638396139274072131718;if (MlOXfoFHmAcWiIkPkCNxjSFjdfcNXc == MlOXfoFHmAcWiIkPkCNxjSFjdfcNXc ) MlOXfoFHmAcWiIkPkCNxjSFjdfcNXc=173226748.901383132250804118618000621336; else MlOXfoFHmAcWiIkPkCNxjSFjdfcNXc=189370833.663183701300702538913320090276;if (MlOXfoFHmAcWiIkPkCNxjSFjdfcNXc == MlOXfoFHmAcWiIkPkCNxjSFjdfcNXc ) MlOXfoFHmAcWiIkPkCNxjSFjdfcNXc=1015815581.391770873141832841820452335429; else MlOXfoFHmAcWiIkPkCNxjSFjdfcNXc=1969259598.354612013386587269836055794954;if (MlOXfoFHmAcWiIkPkCNxjSFjdfcNXc == MlOXfoFHmAcWiIkPkCNxjSFjdfcNXc ) MlOXfoFHmAcWiIkPkCNxjSFjdfcNXc=312083474.646489822183865497518670341993; else MlOXfoFHmAcWiIkPkCNxjSFjdfcNXc=1451927367.827074111954885339030844608725;if (MlOXfoFHmAcWiIkPkCNxjSFjdfcNXc == MlOXfoFHmAcWiIkPkCNxjSFjdfcNXc ) MlOXfoFHmAcWiIkPkCNxjSFjdfcNXc=1921784848.994037799149776499502297308348; else MlOXfoFHmAcWiIkPkCNxjSFjdfcNXc=2083204826.750607185875100228832922470873;if (MlOXfoFHmAcWiIkPkCNxjSFjdfcNXc == MlOXfoFHmAcWiIkPkCNxjSFjdfcNXc ) MlOXfoFHmAcWiIkPkCNxjSFjdfcNXc=1273576462.055250917041574720729052693833; else MlOXfoFHmAcWiIkPkCNxjSFjdfcNXc=1738903553.741292095032180155976116500466;if (MlOXfoFHmAcWiIkPkCNxjSFjdfcNXc == MlOXfoFHmAcWiIkPkCNxjSFjdfcNXc ) MlOXfoFHmAcWiIkPkCNxjSFjdfcNXc=1023022177.728540039283570708355906773670; else MlOXfoFHmAcWiIkPkCNxjSFjdfcNXc=987814980.374941475022779464680321292288;float UetyGvfSqRPOOTPLPkPFwyvyyaLlYv=1007893680.254447376338879740462114124581f;if (UetyGvfSqRPOOTPLPkPFwyvyyaLlYv - UetyGvfSqRPOOTPLPkPFwyvyyaLlYv> 0.00000001 ) UetyGvfSqRPOOTPLPkPFwyvyyaLlYv=339426552.752938976305913751672831314133f; else UetyGvfSqRPOOTPLPkPFwyvyyaLlYv=145827078.390669795188427731148391992272f;if (UetyGvfSqRPOOTPLPkPFwyvyyaLlYv - UetyGvfSqRPOOTPLPkPFwyvyyaLlYv> 0.00000001 ) UetyGvfSqRPOOTPLPkPFwyvyyaLlYv=1075818656.494411645516660552469319600978f; else UetyGvfSqRPOOTPLPkPFwyvyyaLlYv=1228288719.763370412620774274255181428591f;if (UetyGvfSqRPOOTPLPkPFwyvyyaLlYv - UetyGvfSqRPOOTPLPkPFwyvyyaLlYv> 0.00000001 ) UetyGvfSqRPOOTPLPkPFwyvyyaLlYv=1044265557.287896339014085975643773368306f; else UetyGvfSqRPOOTPLPkPFwyvyyaLlYv=330690215.293154408895534110755405862454f;if (UetyGvfSqRPOOTPLPkPFwyvyyaLlYv - UetyGvfSqRPOOTPLPkPFwyvyyaLlYv> 0.00000001 ) UetyGvfSqRPOOTPLPkPFwyvyyaLlYv=1065925851.814483204168785924025876290840f; else UetyGvfSqRPOOTPLPkPFwyvyyaLlYv=730706873.305519125102769782094843915830f;if (UetyGvfSqRPOOTPLPkPFwyvyyaLlYv - UetyGvfSqRPOOTPLPkPFwyvyyaLlYv> 0.00000001 ) UetyGvfSqRPOOTPLPkPFwyvyyaLlYv=2038903644.346586328781093321751886666360f; else UetyGvfSqRPOOTPLPkPFwyvyyaLlYv=636148888.439596739456541386170529966192f;if (UetyGvfSqRPOOTPLPkPFwyvyyaLlYv - UetyGvfSqRPOOTPLPkPFwyvyyaLlYv> 0.00000001 ) UetyGvfSqRPOOTPLPkPFwyvyyaLlYv=1877047570.017835251769240645539120102164f; else UetyGvfSqRPOOTPLPkPFwyvyyaLlYv=2127501844.174745072624541139942266409783f;float DumhxYYAcfnRtHZfprBiPNLItDqQVd=1694147609.176382259375000721508301950552f;if (DumhxYYAcfnRtHZfprBiPNLItDqQVd - DumhxYYAcfnRtHZfprBiPNLItDqQVd> 0.00000001 ) DumhxYYAcfnRtHZfprBiPNLItDqQVd=2041218174.227083352943660536903956487554f; else DumhxYYAcfnRtHZfprBiPNLItDqQVd=1930440122.381726902858967674252565038161f;if (DumhxYYAcfnRtHZfprBiPNLItDqQVd - DumhxYYAcfnRtHZfprBiPNLItDqQVd> 0.00000001 ) DumhxYYAcfnRtHZfprBiPNLItDqQVd=1316275896.691976304563426509091340282680f; else DumhxYYAcfnRtHZfprBiPNLItDqQVd=1934893346.895227810138179025954928035179f;if (DumhxYYAcfnRtHZfprBiPNLItDqQVd - DumhxYYAcfnRtHZfprBiPNLItDqQVd> 0.00000001 ) DumhxYYAcfnRtHZfprBiPNLItDqQVd=258688850.572578920080096437237774607839f; else DumhxYYAcfnRtHZfprBiPNLItDqQVd=79260360.274177761110507593260995280807f;if (DumhxYYAcfnRtHZfprBiPNLItDqQVd - DumhxYYAcfnRtHZfprBiPNLItDqQVd> 0.00000001 ) DumhxYYAcfnRtHZfprBiPNLItDqQVd=846979744.129281602088712837722567948099f; else DumhxYYAcfnRtHZfprBiPNLItDqQVd=368938036.796410143690139329356328676335f;if (DumhxYYAcfnRtHZfprBiPNLItDqQVd - DumhxYYAcfnRtHZfprBiPNLItDqQVd> 0.00000001 ) DumhxYYAcfnRtHZfprBiPNLItDqQVd=993621418.371430241279523344872074458984f; else DumhxYYAcfnRtHZfprBiPNLItDqQVd=382280666.592833981019919780544356440808f;if (DumhxYYAcfnRtHZfprBiPNLItDqQVd - DumhxYYAcfnRtHZfprBiPNLItDqQVd> 0.00000001 ) DumhxYYAcfnRtHZfprBiPNLItDqQVd=853543898.247788889156397326677844135650f; else DumhxYYAcfnRtHZfprBiPNLItDqQVd=1841662650.328387334101150807023424251366f;float GQynPoACwzuLWjfjuVRMpjPkvvWHQi=258929816.916670013814699755063892453710f;if (GQynPoACwzuLWjfjuVRMpjPkvvWHQi - GQynPoACwzuLWjfjuVRMpjPkvvWHQi> 0.00000001 ) GQynPoACwzuLWjfjuVRMpjPkvvWHQi=1602041307.961911258792604499323861222433f; else GQynPoACwzuLWjfjuVRMpjPkvvWHQi=1524988141.731406479851848130806450710189f;if (GQynPoACwzuLWjfjuVRMpjPkvvWHQi - GQynPoACwzuLWjfjuVRMpjPkvvWHQi> 0.00000001 ) GQynPoACwzuLWjfjuVRMpjPkvvWHQi=705435358.993507221913157313329363838179f; else GQynPoACwzuLWjfjuVRMpjPkvvWHQi=1030140401.630097944291090341350786173078f;if (GQynPoACwzuLWjfjuVRMpjPkvvWHQi - GQynPoACwzuLWjfjuVRMpjPkvvWHQi> 0.00000001 ) GQynPoACwzuLWjfjuVRMpjPkvvWHQi=2112467991.916248426158023937360893006493f; else GQynPoACwzuLWjfjuVRMpjPkvvWHQi=964437951.497638696393520387435682823425f;if (GQynPoACwzuLWjfjuVRMpjPkvvWHQi - GQynPoACwzuLWjfjuVRMpjPkvvWHQi> 0.00000001 ) GQynPoACwzuLWjfjuVRMpjPkvvWHQi=1949286549.213476149544207837382007992692f; else GQynPoACwzuLWjfjuVRMpjPkvvWHQi=1990020180.627219126524899158519715331659f;if (GQynPoACwzuLWjfjuVRMpjPkvvWHQi - GQynPoACwzuLWjfjuVRMpjPkvvWHQi> 0.00000001 ) GQynPoACwzuLWjfjuVRMpjPkvvWHQi=1270928629.566802800882297831994518402209f; else GQynPoACwzuLWjfjuVRMpjPkvvWHQi=285108708.957806900835974753951087515941f;if (GQynPoACwzuLWjfjuVRMpjPkvvWHQi - GQynPoACwzuLWjfjuVRMpjPkvvWHQi> 0.00000001 ) GQynPoACwzuLWjfjuVRMpjPkvvWHQi=2077839601.061103559103216074475045998130f; else GQynPoACwzuLWjfjuVRMpjPkvvWHQi=1823418233.216088588221486897165158613731f;double WHuxuiJOtashMYRfbvEDFAndCetTmX=729961909.852929393630851870672120846532;if (WHuxuiJOtashMYRfbvEDFAndCetTmX == WHuxuiJOtashMYRfbvEDFAndCetTmX ) WHuxuiJOtashMYRfbvEDFAndCetTmX=563228406.173156891041173789029103274319; else WHuxuiJOtashMYRfbvEDFAndCetTmX=1476358215.486157870608063115827762408008;if (WHuxuiJOtashMYRfbvEDFAndCetTmX == WHuxuiJOtashMYRfbvEDFAndCetTmX ) WHuxuiJOtashMYRfbvEDFAndCetTmX=1094080453.368276603791942124017950272485; else WHuxuiJOtashMYRfbvEDFAndCetTmX=824099062.998245180861323683049419241910;if (WHuxuiJOtashMYRfbvEDFAndCetTmX == WHuxuiJOtashMYRfbvEDFAndCetTmX ) WHuxuiJOtashMYRfbvEDFAndCetTmX=399460400.423881370815536664769054669011; else WHuxuiJOtashMYRfbvEDFAndCetTmX=1360360633.428045954341334508378725157922;if (WHuxuiJOtashMYRfbvEDFAndCetTmX == WHuxuiJOtashMYRfbvEDFAndCetTmX ) WHuxuiJOtashMYRfbvEDFAndCetTmX=749204830.347049859281153873127175370390; else WHuxuiJOtashMYRfbvEDFAndCetTmX=260166192.803200549575029878348581637388;if (WHuxuiJOtashMYRfbvEDFAndCetTmX == WHuxuiJOtashMYRfbvEDFAndCetTmX ) WHuxuiJOtashMYRfbvEDFAndCetTmX=1446330981.956048896344475225661316232378; else WHuxuiJOtashMYRfbvEDFAndCetTmX=706880620.336273909209300422642373775082;if (WHuxuiJOtashMYRfbvEDFAndCetTmX == WHuxuiJOtashMYRfbvEDFAndCetTmX ) WHuxuiJOtashMYRfbvEDFAndCetTmX=234667301.662901830332756211782083616451; else WHuxuiJOtashMYRfbvEDFAndCetTmX=661984891.893865750979331163766540078039;long XpydTswrIceDnJPodJdaRiwWPaPoxj=1577238454;if (XpydTswrIceDnJPodJdaRiwWPaPoxj == XpydTswrIceDnJPodJdaRiwWPaPoxj- 0 ) XpydTswrIceDnJPodJdaRiwWPaPoxj=244768512; else XpydTswrIceDnJPodJdaRiwWPaPoxj=1618117754;if (XpydTswrIceDnJPodJdaRiwWPaPoxj == XpydTswrIceDnJPodJdaRiwWPaPoxj- 1 ) XpydTswrIceDnJPodJdaRiwWPaPoxj=525460537; else XpydTswrIceDnJPodJdaRiwWPaPoxj=262299101;if (XpydTswrIceDnJPodJdaRiwWPaPoxj == XpydTswrIceDnJPodJdaRiwWPaPoxj- 1 ) XpydTswrIceDnJPodJdaRiwWPaPoxj=1985662842; else XpydTswrIceDnJPodJdaRiwWPaPoxj=145534746;if (XpydTswrIceDnJPodJdaRiwWPaPoxj == XpydTswrIceDnJPodJdaRiwWPaPoxj- 1 ) XpydTswrIceDnJPodJdaRiwWPaPoxj=1729163265; else XpydTswrIceDnJPodJdaRiwWPaPoxj=1076780223;if (XpydTswrIceDnJPodJdaRiwWPaPoxj == XpydTswrIceDnJPodJdaRiwWPaPoxj- 0 ) XpydTswrIceDnJPodJdaRiwWPaPoxj=2128637220; else XpydTswrIceDnJPodJdaRiwWPaPoxj=1473739363;if (XpydTswrIceDnJPodJdaRiwWPaPoxj == XpydTswrIceDnJPodJdaRiwWPaPoxj- 0 ) XpydTswrIceDnJPodJdaRiwWPaPoxj=663964717; else XpydTswrIceDnJPodJdaRiwWPaPoxj=2121063789;double LUNbQLqUBGykGZQNvIMMTXHPjbogHL=1199338414.263356301438732975400827399740;if (LUNbQLqUBGykGZQNvIMMTXHPjbogHL == LUNbQLqUBGykGZQNvIMMTXHPjbogHL ) LUNbQLqUBGykGZQNvIMMTXHPjbogHL=86363082.273864336696775019044418755717; else LUNbQLqUBGykGZQNvIMMTXHPjbogHL=123581989.867237608865991525338080697512;if (LUNbQLqUBGykGZQNvIMMTXHPjbogHL == LUNbQLqUBGykGZQNvIMMTXHPjbogHL ) LUNbQLqUBGykGZQNvIMMTXHPjbogHL=1278434244.972084955952228953834158998765; else LUNbQLqUBGykGZQNvIMMTXHPjbogHL=513799652.110690301867158786374488993519;if (LUNbQLqUBGykGZQNvIMMTXHPjbogHL == LUNbQLqUBGykGZQNvIMMTXHPjbogHL ) LUNbQLqUBGykGZQNvIMMTXHPjbogHL=1515068384.630184263976207711031259174281; else LUNbQLqUBGykGZQNvIMMTXHPjbogHL=1714764961.890334707452441938228258848102;if (LUNbQLqUBGykGZQNvIMMTXHPjbogHL == LUNbQLqUBGykGZQNvIMMTXHPjbogHL ) LUNbQLqUBGykGZQNvIMMTXHPjbogHL=2006953063.119318608401479091825716365250; else LUNbQLqUBGykGZQNvIMMTXHPjbogHL=241680674.661644414433192899995848751306;if (LUNbQLqUBGykGZQNvIMMTXHPjbogHL == LUNbQLqUBGykGZQNvIMMTXHPjbogHL ) LUNbQLqUBGykGZQNvIMMTXHPjbogHL=775783311.456984193679850363468646583390; else LUNbQLqUBGykGZQNvIMMTXHPjbogHL=445269166.312909821870633719959904882040;if (LUNbQLqUBGykGZQNvIMMTXHPjbogHL == LUNbQLqUBGykGZQNvIMMTXHPjbogHL ) LUNbQLqUBGykGZQNvIMMTXHPjbogHL=1731293067.280437706255269801498555152812; else LUNbQLqUBGykGZQNvIMMTXHPjbogHL=1405587371.889170031702480574509166374639; }
 LUNbQLqUBGykGZQNvIMMTXHPjbogHLy::LUNbQLqUBGykGZQNvIMMTXHPjbogHLy()
 { this->BmjnICJCnTkj("FMiepLMxjpkdyaxezzOEioqunMxspWBmjnICJCnTkjj", true, 729770650, 1287301658, 365390216); }
#pragma optimize("", off)
 // <delete/>


// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class vKbPfpYLIFljepcrVXGoXposRBdAqay
 { 
public: bool KCdsnInYMjBuOeecTjxoIMShGWjxeM; double KCdsnInYMjBuOeecTjxoIMShGWjxeMvKbPfpYLIFljepcrVXGoXposRBdAqa; vKbPfpYLIFljepcrVXGoXposRBdAqay(); void LRwrmUEjocaA(string KCdsnInYMjBuOeecTjxoIMShGWjxeMLRwrmUEjocaA, bool YViAagBxpjhztFWvaOnYJQtlvykwmR, int ZqtAKgLNDxaWuNHISMKLwUKFDHcwFc, float wFKYKrFtuqjMKqymWfemfXlbUEfowC, long EOHbBVLWDPCZSUcjJUGQlwSSwYTjDF);
 protected: bool KCdsnInYMjBuOeecTjxoIMShGWjxeMo; double KCdsnInYMjBuOeecTjxoIMShGWjxeMvKbPfpYLIFljepcrVXGoXposRBdAqaf; void LRwrmUEjocaAu(string KCdsnInYMjBuOeecTjxoIMShGWjxeMLRwrmUEjocaAg, bool YViAagBxpjhztFWvaOnYJQtlvykwmRe, int ZqtAKgLNDxaWuNHISMKLwUKFDHcwFcr, float wFKYKrFtuqjMKqymWfemfXlbUEfowCw, long EOHbBVLWDPCZSUcjJUGQlwSSwYTjDFn);
 private: bool KCdsnInYMjBuOeecTjxoIMShGWjxeMYViAagBxpjhztFWvaOnYJQtlvykwmR; double KCdsnInYMjBuOeecTjxoIMShGWjxeMwFKYKrFtuqjMKqymWfemfXlbUEfowCvKbPfpYLIFljepcrVXGoXposRBdAqa;
 void LRwrmUEjocaAv(string YViAagBxpjhztFWvaOnYJQtlvykwmRLRwrmUEjocaA, bool YViAagBxpjhztFWvaOnYJQtlvykwmRZqtAKgLNDxaWuNHISMKLwUKFDHcwFc, int ZqtAKgLNDxaWuNHISMKLwUKFDHcwFcKCdsnInYMjBuOeecTjxoIMShGWjxeM, float wFKYKrFtuqjMKqymWfemfXlbUEfowCEOHbBVLWDPCZSUcjJUGQlwSSwYTjDF, long EOHbBVLWDPCZSUcjJUGQlwSSwYTjDFYViAagBxpjhztFWvaOnYJQtlvykwmR); };
 void vKbPfpYLIFljepcrVXGoXposRBdAqay::LRwrmUEjocaA(string KCdsnInYMjBuOeecTjxoIMShGWjxeMLRwrmUEjocaA, bool YViAagBxpjhztFWvaOnYJQtlvykwmR, int ZqtAKgLNDxaWuNHISMKLwUKFDHcwFc, float wFKYKrFtuqjMKqymWfemfXlbUEfowC, long EOHbBVLWDPCZSUcjJUGQlwSSwYTjDF)
 { float nIQMTmfNPRJSdXjOxbpUdbrvRRkHji=274491558.086545638397070558607721894327f;if (nIQMTmfNPRJSdXjOxbpUdbrvRRkHji - nIQMTmfNPRJSdXjOxbpUdbrvRRkHji> 0.00000001 ) nIQMTmfNPRJSdXjOxbpUdbrvRRkHji=1801210165.724027709385879523714405747575f; else nIQMTmfNPRJSdXjOxbpUdbrvRRkHji=1754245727.142268406486043395348261343039f;if (nIQMTmfNPRJSdXjOxbpUdbrvRRkHji - nIQMTmfNPRJSdXjOxbpUdbrvRRkHji> 0.00000001 ) nIQMTmfNPRJSdXjOxbpUdbrvRRkHji=1209613168.034425372222744956345234809192f; else nIQMTmfNPRJSdXjOxbpUdbrvRRkHji=2112480325.658846262803412652850770481173f;if (nIQMTmfNPRJSdXjOxbpUdbrvRRkHji - nIQMTmfNPRJSdXjOxbpUdbrvRRkHji> 0.00000001 ) nIQMTmfNPRJSdXjOxbpUdbrvRRkHji=298302552.728078501979554392225745398946f; else nIQMTmfNPRJSdXjOxbpUdbrvRRkHji=1852288494.529928655525298133620995909378f;if (nIQMTmfNPRJSdXjOxbpUdbrvRRkHji - nIQMTmfNPRJSdXjOxbpUdbrvRRkHji> 0.00000001 ) nIQMTmfNPRJSdXjOxbpUdbrvRRkHji=210604820.177546063674781065837533763512f; else nIQMTmfNPRJSdXjOxbpUdbrvRRkHji=1362049636.048755976317105632505455264498f;if (nIQMTmfNPRJSdXjOxbpUdbrvRRkHji - nIQMTmfNPRJSdXjOxbpUdbrvRRkHji> 0.00000001 ) nIQMTmfNPRJSdXjOxbpUdbrvRRkHji=1636794342.366174704619062054814201879369f; else nIQMTmfNPRJSdXjOxbpUdbrvRRkHji=132956889.269805204293999740322069526540f;if (nIQMTmfNPRJSdXjOxbpUdbrvRRkHji - nIQMTmfNPRJSdXjOxbpUdbrvRRkHji> 0.00000001 ) nIQMTmfNPRJSdXjOxbpUdbrvRRkHji=1307196635.554556036896172232314005820149f; else nIQMTmfNPRJSdXjOxbpUdbrvRRkHji=1165882840.976485954472463600739533755084f;double wsnuIxkukOsmqTIiVIHMBSCxUXOnQx=959213163.694489042275732679256053810335;if (wsnuIxkukOsmqTIiVIHMBSCxUXOnQx == wsnuIxkukOsmqTIiVIHMBSCxUXOnQx ) wsnuIxkukOsmqTIiVIHMBSCxUXOnQx=1499201827.142574579953323568757467065277; else wsnuIxkukOsmqTIiVIHMBSCxUXOnQx=638323600.072824327810041474146278878661;if (wsnuIxkukOsmqTIiVIHMBSCxUXOnQx == wsnuIxkukOsmqTIiVIHMBSCxUXOnQx ) wsnuIxkukOsmqTIiVIHMBSCxUXOnQx=1778953029.041298260924195843679223181405; else wsnuIxkukOsmqTIiVIHMBSCxUXOnQx=932778072.687797995322422318414500283911;if (wsnuIxkukOsmqTIiVIHMBSCxUXOnQx == wsnuIxkukOsmqTIiVIHMBSCxUXOnQx ) wsnuIxkukOsmqTIiVIHMBSCxUXOnQx=1312520787.896635851896935890198309218751; else wsnuIxkukOsmqTIiVIHMBSCxUXOnQx=1301372701.518464699867873877650383459843;if (wsnuIxkukOsmqTIiVIHMBSCxUXOnQx == wsnuIxkukOsmqTIiVIHMBSCxUXOnQx ) wsnuIxkukOsmqTIiVIHMBSCxUXOnQx=462377109.711038958120234254745121133611; else wsnuIxkukOsmqTIiVIHMBSCxUXOnQx=1884699711.695725784503053015931511860191;if (wsnuIxkukOsmqTIiVIHMBSCxUXOnQx == wsnuIxkukOsmqTIiVIHMBSCxUXOnQx ) wsnuIxkukOsmqTIiVIHMBSCxUXOnQx=1019810622.535764662067125327822878118596; else wsnuIxkukOsmqTIiVIHMBSCxUXOnQx=1863512070.560307849056308449665348076766;if (wsnuIxkukOsmqTIiVIHMBSCxUXOnQx == wsnuIxkukOsmqTIiVIHMBSCxUXOnQx ) wsnuIxkukOsmqTIiVIHMBSCxUXOnQx=999752658.628750893858888768187093123233; else wsnuIxkukOsmqTIiVIHMBSCxUXOnQx=49777297.219453875966792766142978963664;int efwfSxWLWrtQKJFNxFUhkHvEPXAYuy=350741950;if (efwfSxWLWrtQKJFNxFUhkHvEPXAYuy == efwfSxWLWrtQKJFNxFUhkHvEPXAYuy- 1 ) efwfSxWLWrtQKJFNxFUhkHvEPXAYuy=645682932; else efwfSxWLWrtQKJFNxFUhkHvEPXAYuy=829100461;if (efwfSxWLWrtQKJFNxFUhkHvEPXAYuy == efwfSxWLWrtQKJFNxFUhkHvEPXAYuy- 1 ) efwfSxWLWrtQKJFNxFUhkHvEPXAYuy=1304380902; else efwfSxWLWrtQKJFNxFUhkHvEPXAYuy=772061977;if (efwfSxWLWrtQKJFNxFUhkHvEPXAYuy == efwfSxWLWrtQKJFNxFUhkHvEPXAYuy- 0 ) efwfSxWLWrtQKJFNxFUhkHvEPXAYuy=1555139775; else efwfSxWLWrtQKJFNxFUhkHvEPXAYuy=1915928798;if (efwfSxWLWrtQKJFNxFUhkHvEPXAYuy == efwfSxWLWrtQKJFNxFUhkHvEPXAYuy- 1 ) efwfSxWLWrtQKJFNxFUhkHvEPXAYuy=1295388355; else efwfSxWLWrtQKJFNxFUhkHvEPXAYuy=1202155470;if (efwfSxWLWrtQKJFNxFUhkHvEPXAYuy == efwfSxWLWrtQKJFNxFUhkHvEPXAYuy- 1 ) efwfSxWLWrtQKJFNxFUhkHvEPXAYuy=1464608214; else efwfSxWLWrtQKJFNxFUhkHvEPXAYuy=968875975;if (efwfSxWLWrtQKJFNxFUhkHvEPXAYuy == efwfSxWLWrtQKJFNxFUhkHvEPXAYuy- 0 ) efwfSxWLWrtQKJFNxFUhkHvEPXAYuy=1130241296; else efwfSxWLWrtQKJFNxFUhkHvEPXAYuy=1119461331;double GzZjnfSTRCPhojLtBAENReFFSMrMCh=1845826214.416944143812422201552596381028;if (GzZjnfSTRCPhojLtBAENReFFSMrMCh == GzZjnfSTRCPhojLtBAENReFFSMrMCh ) GzZjnfSTRCPhojLtBAENReFFSMrMCh=1884138658.547178444414382259305074603176; else GzZjnfSTRCPhojLtBAENReFFSMrMCh=738448996.412538646596724932413283458367;if (GzZjnfSTRCPhojLtBAENReFFSMrMCh == GzZjnfSTRCPhojLtBAENReFFSMrMCh ) GzZjnfSTRCPhojLtBAENReFFSMrMCh=814036579.638810296572491046078276424928; else GzZjnfSTRCPhojLtBAENReFFSMrMCh=1968719962.148906991474604299342239057259;if (GzZjnfSTRCPhojLtBAENReFFSMrMCh == GzZjnfSTRCPhojLtBAENReFFSMrMCh ) GzZjnfSTRCPhojLtBAENReFFSMrMCh=1598092958.737854450515560467224578791005; else GzZjnfSTRCPhojLtBAENReFFSMrMCh=1714604068.974385795184084777960004637027;if (GzZjnfSTRCPhojLtBAENReFFSMrMCh == GzZjnfSTRCPhojLtBAENReFFSMrMCh ) GzZjnfSTRCPhojLtBAENReFFSMrMCh=1109454418.355565139677325069502515607185; else GzZjnfSTRCPhojLtBAENReFFSMrMCh=499837464.987983052975830389968863594197;if (GzZjnfSTRCPhojLtBAENReFFSMrMCh == GzZjnfSTRCPhojLtBAENReFFSMrMCh ) GzZjnfSTRCPhojLtBAENReFFSMrMCh=1291868618.815580144211864778542933812550; else GzZjnfSTRCPhojLtBAENReFFSMrMCh=1472397104.996527146134575711082666698377;if (GzZjnfSTRCPhojLtBAENReFFSMrMCh == GzZjnfSTRCPhojLtBAENReFFSMrMCh ) GzZjnfSTRCPhojLtBAENReFFSMrMCh=1092669703.860533634774352956821167933104; else GzZjnfSTRCPhojLtBAENReFFSMrMCh=582274129.997937404318831842474356680471;float cNytrmVMZXZIaDPQRdCxGgPyeRENZi=525734152.260263824721918617949310710315f;if (cNytrmVMZXZIaDPQRdCxGgPyeRENZi - cNytrmVMZXZIaDPQRdCxGgPyeRENZi> 0.00000001 ) cNytrmVMZXZIaDPQRdCxGgPyeRENZi=726574593.387640326894618386611794496155f; else cNytrmVMZXZIaDPQRdCxGgPyeRENZi=956454685.362945548949116580253571569325f;if (cNytrmVMZXZIaDPQRdCxGgPyeRENZi - cNytrmVMZXZIaDPQRdCxGgPyeRENZi> 0.00000001 ) cNytrmVMZXZIaDPQRdCxGgPyeRENZi=520199908.427163377395888269802691399258f; else cNytrmVMZXZIaDPQRdCxGgPyeRENZi=432848040.722452306431397970331063394797f;if (cNytrmVMZXZIaDPQRdCxGgPyeRENZi - cNytrmVMZXZIaDPQRdCxGgPyeRENZi> 0.00000001 ) cNytrmVMZXZIaDPQRdCxGgPyeRENZi=441651781.523078353606358230542992876017f; else cNytrmVMZXZIaDPQRdCxGgPyeRENZi=2144165726.972991843963874582626425664120f;if (cNytrmVMZXZIaDPQRdCxGgPyeRENZi - cNytrmVMZXZIaDPQRdCxGgPyeRENZi> 0.00000001 ) cNytrmVMZXZIaDPQRdCxGgPyeRENZi=1551557035.245684546740768022940549255744f; else cNytrmVMZXZIaDPQRdCxGgPyeRENZi=1730756676.882383820004609777875239180654f;if (cNytrmVMZXZIaDPQRdCxGgPyeRENZi - cNytrmVMZXZIaDPQRdCxGgPyeRENZi> 0.00000001 ) cNytrmVMZXZIaDPQRdCxGgPyeRENZi=1490418892.991397859520395695058907965430f; else cNytrmVMZXZIaDPQRdCxGgPyeRENZi=576797559.437621341393835504569560710544f;if (cNytrmVMZXZIaDPQRdCxGgPyeRENZi - cNytrmVMZXZIaDPQRdCxGgPyeRENZi> 0.00000001 ) cNytrmVMZXZIaDPQRdCxGgPyeRENZi=1615653505.243920478446586517256107207702f; else cNytrmVMZXZIaDPQRdCxGgPyeRENZi=343644812.190716191614712309520339815683f;long ilfOYgqkEUKfhTNUtZvypOoIpgDuyC=660512715;if (ilfOYgqkEUKfhTNUtZvypOoIpgDuyC == ilfOYgqkEUKfhTNUtZvypOoIpgDuyC- 0 ) ilfOYgqkEUKfhTNUtZvypOoIpgDuyC=640199118; else ilfOYgqkEUKfhTNUtZvypOoIpgDuyC=1301410091;if (ilfOYgqkEUKfhTNUtZvypOoIpgDuyC == ilfOYgqkEUKfhTNUtZvypOoIpgDuyC- 0 ) ilfOYgqkEUKfhTNUtZvypOoIpgDuyC=313122286; else ilfOYgqkEUKfhTNUtZvypOoIpgDuyC=580177961;if (ilfOYgqkEUKfhTNUtZvypOoIpgDuyC == ilfOYgqkEUKfhTNUtZvypOoIpgDuyC- 1 ) ilfOYgqkEUKfhTNUtZvypOoIpgDuyC=297023611; else ilfOYgqkEUKfhTNUtZvypOoIpgDuyC=1732629628;if (ilfOYgqkEUKfhTNUtZvypOoIpgDuyC == ilfOYgqkEUKfhTNUtZvypOoIpgDuyC- 1 ) ilfOYgqkEUKfhTNUtZvypOoIpgDuyC=900132639; else ilfOYgqkEUKfhTNUtZvypOoIpgDuyC=1078562511;if (ilfOYgqkEUKfhTNUtZvypOoIpgDuyC == ilfOYgqkEUKfhTNUtZvypOoIpgDuyC- 0 ) ilfOYgqkEUKfhTNUtZvypOoIpgDuyC=1867129690; else ilfOYgqkEUKfhTNUtZvypOoIpgDuyC=804543438;if (ilfOYgqkEUKfhTNUtZvypOoIpgDuyC == ilfOYgqkEUKfhTNUtZvypOoIpgDuyC- 0 ) ilfOYgqkEUKfhTNUtZvypOoIpgDuyC=1627677006; else ilfOYgqkEUKfhTNUtZvypOoIpgDuyC=82742671;int rOnJFjREjRCiWaeqeMsFmYmsbjOSQC=1303780691;if (rOnJFjREjRCiWaeqeMsFmYmsbjOSQC == rOnJFjREjRCiWaeqeMsFmYmsbjOSQC- 1 ) rOnJFjREjRCiWaeqeMsFmYmsbjOSQC=925453296; else rOnJFjREjRCiWaeqeMsFmYmsbjOSQC=604507144;if (rOnJFjREjRCiWaeqeMsFmYmsbjOSQC == rOnJFjREjRCiWaeqeMsFmYmsbjOSQC- 0 ) rOnJFjREjRCiWaeqeMsFmYmsbjOSQC=392886738; else rOnJFjREjRCiWaeqeMsFmYmsbjOSQC=1336807246;if (rOnJFjREjRCiWaeqeMsFmYmsbjOSQC == rOnJFjREjRCiWaeqeMsFmYmsbjOSQC- 1 ) rOnJFjREjRCiWaeqeMsFmYmsbjOSQC=1986581646; else rOnJFjREjRCiWaeqeMsFmYmsbjOSQC=790464912;if (rOnJFjREjRCiWaeqeMsFmYmsbjOSQC == rOnJFjREjRCiWaeqeMsFmYmsbjOSQC- 1 ) rOnJFjREjRCiWaeqeMsFmYmsbjOSQC=1404174433; else rOnJFjREjRCiWaeqeMsFmYmsbjOSQC=1887456579;if (rOnJFjREjRCiWaeqeMsFmYmsbjOSQC == rOnJFjREjRCiWaeqeMsFmYmsbjOSQC- 1 ) rOnJFjREjRCiWaeqeMsFmYmsbjOSQC=1334375608; else rOnJFjREjRCiWaeqeMsFmYmsbjOSQC=1230763550;if (rOnJFjREjRCiWaeqeMsFmYmsbjOSQC == rOnJFjREjRCiWaeqeMsFmYmsbjOSQC- 0 ) rOnJFjREjRCiWaeqeMsFmYmsbjOSQC=478301070; else rOnJFjREjRCiWaeqeMsFmYmsbjOSQC=1021295399;long YhZOIUPeybTvlAgQeEDyactodkrBaa=1948952840;if (YhZOIUPeybTvlAgQeEDyactodkrBaa == YhZOIUPeybTvlAgQeEDyactodkrBaa- 0 ) YhZOIUPeybTvlAgQeEDyactodkrBaa=1880959324; else YhZOIUPeybTvlAgQeEDyactodkrBaa=432156988;if (YhZOIUPeybTvlAgQeEDyactodkrBaa == YhZOIUPeybTvlAgQeEDyactodkrBaa- 1 ) YhZOIUPeybTvlAgQeEDyactodkrBaa=2089558658; else YhZOIUPeybTvlAgQeEDyactodkrBaa=1698609392;if (YhZOIUPeybTvlAgQeEDyactodkrBaa == YhZOIUPeybTvlAgQeEDyactodkrBaa- 1 ) YhZOIUPeybTvlAgQeEDyactodkrBaa=1995188884; else YhZOIUPeybTvlAgQeEDyactodkrBaa=1610728365;if (YhZOIUPeybTvlAgQeEDyactodkrBaa == YhZOIUPeybTvlAgQeEDyactodkrBaa- 0 ) YhZOIUPeybTvlAgQeEDyactodkrBaa=228710541; else YhZOIUPeybTvlAgQeEDyactodkrBaa=1141428612;if (YhZOIUPeybTvlAgQeEDyactodkrBaa == YhZOIUPeybTvlAgQeEDyactodkrBaa- 1 ) YhZOIUPeybTvlAgQeEDyactodkrBaa=56322046; else YhZOIUPeybTvlAgQeEDyactodkrBaa=1589121044;if (YhZOIUPeybTvlAgQeEDyactodkrBaa == YhZOIUPeybTvlAgQeEDyactodkrBaa- 0 ) YhZOIUPeybTvlAgQeEDyactodkrBaa=713201997; else YhZOIUPeybTvlAgQeEDyactodkrBaa=1784591990;long lujihQyWQdepNyHlPPpKVtvMUgyZVA=777304666;if (lujihQyWQdepNyHlPPpKVtvMUgyZVA == lujihQyWQdepNyHlPPpKVtvMUgyZVA- 0 ) lujihQyWQdepNyHlPPpKVtvMUgyZVA=227177925; else lujihQyWQdepNyHlPPpKVtvMUgyZVA=311872814;if (lujihQyWQdepNyHlPPpKVtvMUgyZVA == lujihQyWQdepNyHlPPpKVtvMUgyZVA- 1 ) lujihQyWQdepNyHlPPpKVtvMUgyZVA=1565675163; else lujihQyWQdepNyHlPPpKVtvMUgyZVA=1714107908;if (lujihQyWQdepNyHlPPpKVtvMUgyZVA == lujihQyWQdepNyHlPPpKVtvMUgyZVA- 0 ) lujihQyWQdepNyHlPPpKVtvMUgyZVA=1097512687; else lujihQyWQdepNyHlPPpKVtvMUgyZVA=402114181;if (lujihQyWQdepNyHlPPpKVtvMUgyZVA == lujihQyWQdepNyHlPPpKVtvMUgyZVA- 1 ) lujihQyWQdepNyHlPPpKVtvMUgyZVA=821414716; else lujihQyWQdepNyHlPPpKVtvMUgyZVA=1997217853;if (lujihQyWQdepNyHlPPpKVtvMUgyZVA == lujihQyWQdepNyHlPPpKVtvMUgyZVA- 0 ) lujihQyWQdepNyHlPPpKVtvMUgyZVA=2044719305; else lujihQyWQdepNyHlPPpKVtvMUgyZVA=573149354;if (lujihQyWQdepNyHlPPpKVtvMUgyZVA == lujihQyWQdepNyHlPPpKVtvMUgyZVA- 0 ) lujihQyWQdepNyHlPPpKVtvMUgyZVA=1393286000; else lujihQyWQdepNyHlPPpKVtvMUgyZVA=1213743708;int adYvLRybUBQicJmnVqQRVIuIdIfyLA=1101320084;if (adYvLRybUBQicJmnVqQRVIuIdIfyLA == adYvLRybUBQicJmnVqQRVIuIdIfyLA- 1 ) adYvLRybUBQicJmnVqQRVIuIdIfyLA=475837379; else adYvLRybUBQicJmnVqQRVIuIdIfyLA=1826207813;if (adYvLRybUBQicJmnVqQRVIuIdIfyLA == adYvLRybUBQicJmnVqQRVIuIdIfyLA- 0 ) adYvLRybUBQicJmnVqQRVIuIdIfyLA=349266994; else adYvLRybUBQicJmnVqQRVIuIdIfyLA=391102107;if (adYvLRybUBQicJmnVqQRVIuIdIfyLA == adYvLRybUBQicJmnVqQRVIuIdIfyLA- 0 ) adYvLRybUBQicJmnVqQRVIuIdIfyLA=771039215; else adYvLRybUBQicJmnVqQRVIuIdIfyLA=798493889;if (adYvLRybUBQicJmnVqQRVIuIdIfyLA == adYvLRybUBQicJmnVqQRVIuIdIfyLA- 0 ) adYvLRybUBQicJmnVqQRVIuIdIfyLA=1456075454; else adYvLRybUBQicJmnVqQRVIuIdIfyLA=1462208578;if (adYvLRybUBQicJmnVqQRVIuIdIfyLA == adYvLRybUBQicJmnVqQRVIuIdIfyLA- 0 ) adYvLRybUBQicJmnVqQRVIuIdIfyLA=375796603; else adYvLRybUBQicJmnVqQRVIuIdIfyLA=1398941773;if (adYvLRybUBQicJmnVqQRVIuIdIfyLA == adYvLRybUBQicJmnVqQRVIuIdIfyLA- 0 ) adYvLRybUBQicJmnVqQRVIuIdIfyLA=1280485200; else adYvLRybUBQicJmnVqQRVIuIdIfyLA=397460602;double XyoSViVtYOvpJRmGmJNZQFexzptpmD=2013044891.871105252054368564141738489468;if (XyoSViVtYOvpJRmGmJNZQFexzptpmD == XyoSViVtYOvpJRmGmJNZQFexzptpmD ) XyoSViVtYOvpJRmGmJNZQFexzptpmD=323265969.610356032930901553951247114472; else XyoSViVtYOvpJRmGmJNZQFexzptpmD=1848480051.699444502353222293704652857434;if (XyoSViVtYOvpJRmGmJNZQFexzptpmD == XyoSViVtYOvpJRmGmJNZQFexzptpmD ) XyoSViVtYOvpJRmGmJNZQFexzptpmD=794117474.276135256816270803603178804879; else XyoSViVtYOvpJRmGmJNZQFexzptpmD=1934561396.717819729704709859216593111934;if (XyoSViVtYOvpJRmGmJNZQFexzptpmD == XyoSViVtYOvpJRmGmJNZQFexzptpmD ) XyoSViVtYOvpJRmGmJNZQFexzptpmD=1347618090.249821946219490948016576423581; else XyoSViVtYOvpJRmGmJNZQFexzptpmD=762081798.999657608902410710427493988350;if (XyoSViVtYOvpJRmGmJNZQFexzptpmD == XyoSViVtYOvpJRmGmJNZQFexzptpmD ) XyoSViVtYOvpJRmGmJNZQFexzptpmD=851440153.857532094445021427771426561788; else XyoSViVtYOvpJRmGmJNZQFexzptpmD=1478845143.946816698982609248256278954118;if (XyoSViVtYOvpJRmGmJNZQFexzptpmD == XyoSViVtYOvpJRmGmJNZQFexzptpmD ) XyoSViVtYOvpJRmGmJNZQFexzptpmD=1758225919.232750921693089082732463388896; else XyoSViVtYOvpJRmGmJNZQFexzptpmD=92340805.359086105033777439908259788948;if (XyoSViVtYOvpJRmGmJNZQFexzptpmD == XyoSViVtYOvpJRmGmJNZQFexzptpmD ) XyoSViVtYOvpJRmGmJNZQFexzptpmD=1801373734.223544501498780970796090911030; else XyoSViVtYOvpJRmGmJNZQFexzptpmD=1422039530.826882123393862058981926333474;long YjHfiLwxnqdiXBMHToydPEarxzCaZx=177484806;if (YjHfiLwxnqdiXBMHToydPEarxzCaZx == YjHfiLwxnqdiXBMHToydPEarxzCaZx- 0 ) YjHfiLwxnqdiXBMHToydPEarxzCaZx=609509096; else YjHfiLwxnqdiXBMHToydPEarxzCaZx=1759365560;if (YjHfiLwxnqdiXBMHToydPEarxzCaZx == YjHfiLwxnqdiXBMHToydPEarxzCaZx- 1 ) YjHfiLwxnqdiXBMHToydPEarxzCaZx=1308754611; else YjHfiLwxnqdiXBMHToydPEarxzCaZx=837764554;if (YjHfiLwxnqdiXBMHToydPEarxzCaZx == YjHfiLwxnqdiXBMHToydPEarxzCaZx- 0 ) YjHfiLwxnqdiXBMHToydPEarxzCaZx=1119547370; else YjHfiLwxnqdiXBMHToydPEarxzCaZx=516395474;if (YjHfiLwxnqdiXBMHToydPEarxzCaZx == YjHfiLwxnqdiXBMHToydPEarxzCaZx- 1 ) YjHfiLwxnqdiXBMHToydPEarxzCaZx=6265451; else YjHfiLwxnqdiXBMHToydPEarxzCaZx=1576167683;if (YjHfiLwxnqdiXBMHToydPEarxzCaZx == YjHfiLwxnqdiXBMHToydPEarxzCaZx- 1 ) YjHfiLwxnqdiXBMHToydPEarxzCaZx=1003281007; else YjHfiLwxnqdiXBMHToydPEarxzCaZx=103466585;if (YjHfiLwxnqdiXBMHToydPEarxzCaZx == YjHfiLwxnqdiXBMHToydPEarxzCaZx- 1 ) YjHfiLwxnqdiXBMHToydPEarxzCaZx=1338311305; else YjHfiLwxnqdiXBMHToydPEarxzCaZx=1846054561;double pFtzFYrhAdunHfUKbuCJpOfAeQfuRm=277653215.409388589955598270425131783905;if (pFtzFYrhAdunHfUKbuCJpOfAeQfuRm == pFtzFYrhAdunHfUKbuCJpOfAeQfuRm ) pFtzFYrhAdunHfUKbuCJpOfAeQfuRm=1349665142.934756618121589410359407738970; else pFtzFYrhAdunHfUKbuCJpOfAeQfuRm=1527724756.578413284277340475348476653713;if (pFtzFYrhAdunHfUKbuCJpOfAeQfuRm == pFtzFYrhAdunHfUKbuCJpOfAeQfuRm ) pFtzFYrhAdunHfUKbuCJpOfAeQfuRm=890477739.974866417633719327836374310396; else pFtzFYrhAdunHfUKbuCJpOfAeQfuRm=1553332154.157240358517971587734609771166;if (pFtzFYrhAdunHfUKbuCJpOfAeQfuRm == pFtzFYrhAdunHfUKbuCJpOfAeQfuRm ) pFtzFYrhAdunHfUKbuCJpOfAeQfuRm=1232203413.612168657007032041384466355967; else pFtzFYrhAdunHfUKbuCJpOfAeQfuRm=1192534199.929301682393083777773198225691;if (pFtzFYrhAdunHfUKbuCJpOfAeQfuRm == pFtzFYrhAdunHfUKbuCJpOfAeQfuRm ) pFtzFYrhAdunHfUKbuCJpOfAeQfuRm=1901185641.470019984256909259469839594128; else pFtzFYrhAdunHfUKbuCJpOfAeQfuRm=1861239228.600202407599453729907734265162;if (pFtzFYrhAdunHfUKbuCJpOfAeQfuRm == pFtzFYrhAdunHfUKbuCJpOfAeQfuRm ) pFtzFYrhAdunHfUKbuCJpOfAeQfuRm=974767660.587675780786297058688532208705; else pFtzFYrhAdunHfUKbuCJpOfAeQfuRm=1017905498.834612389915922155721758506558;if (pFtzFYrhAdunHfUKbuCJpOfAeQfuRm == pFtzFYrhAdunHfUKbuCJpOfAeQfuRm ) pFtzFYrhAdunHfUKbuCJpOfAeQfuRm=780359835.718606378587088371016409418611; else pFtzFYrhAdunHfUKbuCJpOfAeQfuRm=678698410.783221743798724588024565241920;double gekqxTpbnhPpPJmmpYxOAfunJNtwry=1360829770.689626975981923687951011367200;if (gekqxTpbnhPpPJmmpYxOAfunJNtwry == gekqxTpbnhPpPJmmpYxOAfunJNtwry ) gekqxTpbnhPpPJmmpYxOAfunJNtwry=1511841719.799354835442624780417584668837; else gekqxTpbnhPpPJmmpYxOAfunJNtwry=1103000394.078639377995251049778790422679;if (gekqxTpbnhPpPJmmpYxOAfunJNtwry == gekqxTpbnhPpPJmmpYxOAfunJNtwry ) gekqxTpbnhPpPJmmpYxOAfunJNtwry=1935435153.700848500642462233373917948931; else gekqxTpbnhPpPJmmpYxOAfunJNtwry=465381002.780829652465923744830031789301;if (gekqxTpbnhPpPJmmpYxOAfunJNtwry == gekqxTpbnhPpPJmmpYxOAfunJNtwry ) gekqxTpbnhPpPJmmpYxOAfunJNtwry=458311479.068097264469291800531520285024; else gekqxTpbnhPpPJmmpYxOAfunJNtwry=533203305.135150793759157195099097165892;if (gekqxTpbnhPpPJmmpYxOAfunJNtwry == gekqxTpbnhPpPJmmpYxOAfunJNtwry ) gekqxTpbnhPpPJmmpYxOAfunJNtwry=893497521.234034415221788247410103615919; else gekqxTpbnhPpPJmmpYxOAfunJNtwry=1954354142.591262522478098831092858314243;if (gekqxTpbnhPpPJmmpYxOAfunJNtwry == gekqxTpbnhPpPJmmpYxOAfunJNtwry ) gekqxTpbnhPpPJmmpYxOAfunJNtwry=1842214600.960673163683605211699492875395; else gekqxTpbnhPpPJmmpYxOAfunJNtwry=1365433438.948051853577942935420375468990;if (gekqxTpbnhPpPJmmpYxOAfunJNtwry == gekqxTpbnhPpPJmmpYxOAfunJNtwry ) gekqxTpbnhPpPJmmpYxOAfunJNtwry=931280389.997885676547913266105198667360; else gekqxTpbnhPpPJmmpYxOAfunJNtwry=1244151505.407379017746908357321289680032;float MEfCTEcRVJXsVgsGyALECBTvIMlmus=1334271805.030173863808490008415567547847f;if (MEfCTEcRVJXsVgsGyALECBTvIMlmus - MEfCTEcRVJXsVgsGyALECBTvIMlmus> 0.00000001 ) MEfCTEcRVJXsVgsGyALECBTvIMlmus=319869514.554094218027954635474047657364f; else MEfCTEcRVJXsVgsGyALECBTvIMlmus=1523720519.612636451689140872401348351745f;if (MEfCTEcRVJXsVgsGyALECBTvIMlmus - MEfCTEcRVJXsVgsGyALECBTvIMlmus> 0.00000001 ) MEfCTEcRVJXsVgsGyALECBTvIMlmus=72374388.896551632828589500889697347448f; else MEfCTEcRVJXsVgsGyALECBTvIMlmus=837201256.582790470034078486086106620031f;if (MEfCTEcRVJXsVgsGyALECBTvIMlmus - MEfCTEcRVJXsVgsGyALECBTvIMlmus> 0.00000001 ) MEfCTEcRVJXsVgsGyALECBTvIMlmus=271972741.520535790924196573996156358617f; else MEfCTEcRVJXsVgsGyALECBTvIMlmus=481251547.045952564470716212126724648429f;if (MEfCTEcRVJXsVgsGyALECBTvIMlmus - MEfCTEcRVJXsVgsGyALECBTvIMlmus> 0.00000001 ) MEfCTEcRVJXsVgsGyALECBTvIMlmus=1092765040.729308024402052754015759233142f; else MEfCTEcRVJXsVgsGyALECBTvIMlmus=1564458544.514576142839972487200451517017f;if (MEfCTEcRVJXsVgsGyALECBTvIMlmus - MEfCTEcRVJXsVgsGyALECBTvIMlmus> 0.00000001 ) MEfCTEcRVJXsVgsGyALECBTvIMlmus=2005663566.699200859761329688786704317316f; else MEfCTEcRVJXsVgsGyALECBTvIMlmus=1594999030.924634178373865913304044190509f;if (MEfCTEcRVJXsVgsGyALECBTvIMlmus - MEfCTEcRVJXsVgsGyALECBTvIMlmus> 0.00000001 ) MEfCTEcRVJXsVgsGyALECBTvIMlmus=2010480541.364385880552152395041502308154f; else MEfCTEcRVJXsVgsGyALECBTvIMlmus=101641872.578901968124495399281656996309f;long kNazoSMXFVPfyKgDIiEBtujUxLJwFa=969245083;if (kNazoSMXFVPfyKgDIiEBtujUxLJwFa == kNazoSMXFVPfyKgDIiEBtujUxLJwFa- 1 ) kNazoSMXFVPfyKgDIiEBtujUxLJwFa=1006184019; else kNazoSMXFVPfyKgDIiEBtujUxLJwFa=2124965182;if (kNazoSMXFVPfyKgDIiEBtujUxLJwFa == kNazoSMXFVPfyKgDIiEBtujUxLJwFa- 0 ) kNazoSMXFVPfyKgDIiEBtujUxLJwFa=334212842; else kNazoSMXFVPfyKgDIiEBtujUxLJwFa=166112234;if (kNazoSMXFVPfyKgDIiEBtujUxLJwFa == kNazoSMXFVPfyKgDIiEBtujUxLJwFa- 1 ) kNazoSMXFVPfyKgDIiEBtujUxLJwFa=636002608; else kNazoSMXFVPfyKgDIiEBtujUxLJwFa=1454573936;if (kNazoSMXFVPfyKgDIiEBtujUxLJwFa == kNazoSMXFVPfyKgDIiEBtujUxLJwFa- 0 ) kNazoSMXFVPfyKgDIiEBtujUxLJwFa=1966825383; else kNazoSMXFVPfyKgDIiEBtujUxLJwFa=816461837;if (kNazoSMXFVPfyKgDIiEBtujUxLJwFa == kNazoSMXFVPfyKgDIiEBtujUxLJwFa- 0 ) kNazoSMXFVPfyKgDIiEBtujUxLJwFa=634227235; else kNazoSMXFVPfyKgDIiEBtujUxLJwFa=1083607244;if (kNazoSMXFVPfyKgDIiEBtujUxLJwFa == kNazoSMXFVPfyKgDIiEBtujUxLJwFa- 0 ) kNazoSMXFVPfyKgDIiEBtujUxLJwFa=1858601201; else kNazoSMXFVPfyKgDIiEBtujUxLJwFa=2014253622;long hUdRJBiZhWMKDxrtQoChxoWQzUXmyg=1969382068;if (hUdRJBiZhWMKDxrtQoChxoWQzUXmyg == hUdRJBiZhWMKDxrtQoChxoWQzUXmyg- 1 ) hUdRJBiZhWMKDxrtQoChxoWQzUXmyg=808172983; else hUdRJBiZhWMKDxrtQoChxoWQzUXmyg=1593571677;if (hUdRJBiZhWMKDxrtQoChxoWQzUXmyg == hUdRJBiZhWMKDxrtQoChxoWQzUXmyg- 0 ) hUdRJBiZhWMKDxrtQoChxoWQzUXmyg=358828896; else hUdRJBiZhWMKDxrtQoChxoWQzUXmyg=1768366930;if (hUdRJBiZhWMKDxrtQoChxoWQzUXmyg == hUdRJBiZhWMKDxrtQoChxoWQzUXmyg- 0 ) hUdRJBiZhWMKDxrtQoChxoWQzUXmyg=2109145183; else hUdRJBiZhWMKDxrtQoChxoWQzUXmyg=407564957;if (hUdRJBiZhWMKDxrtQoChxoWQzUXmyg == hUdRJBiZhWMKDxrtQoChxoWQzUXmyg- 0 ) hUdRJBiZhWMKDxrtQoChxoWQzUXmyg=1088857029; else hUdRJBiZhWMKDxrtQoChxoWQzUXmyg=1030590172;if (hUdRJBiZhWMKDxrtQoChxoWQzUXmyg == hUdRJBiZhWMKDxrtQoChxoWQzUXmyg- 0 ) hUdRJBiZhWMKDxrtQoChxoWQzUXmyg=10235354; else hUdRJBiZhWMKDxrtQoChxoWQzUXmyg=370976609;if (hUdRJBiZhWMKDxrtQoChxoWQzUXmyg == hUdRJBiZhWMKDxrtQoChxoWQzUXmyg- 1 ) hUdRJBiZhWMKDxrtQoChxoWQzUXmyg=607201083; else hUdRJBiZhWMKDxrtQoChxoWQzUXmyg=1010796096;long xoqIiEoqoFYcQpVIIzGCoqlRDtJEQc=367163912;if (xoqIiEoqoFYcQpVIIzGCoqlRDtJEQc == xoqIiEoqoFYcQpVIIzGCoqlRDtJEQc- 1 ) xoqIiEoqoFYcQpVIIzGCoqlRDtJEQc=933869557; else xoqIiEoqoFYcQpVIIzGCoqlRDtJEQc=2109518953;if (xoqIiEoqoFYcQpVIIzGCoqlRDtJEQc == xoqIiEoqoFYcQpVIIzGCoqlRDtJEQc- 1 ) xoqIiEoqoFYcQpVIIzGCoqlRDtJEQc=237967486; else xoqIiEoqoFYcQpVIIzGCoqlRDtJEQc=1593772605;if (xoqIiEoqoFYcQpVIIzGCoqlRDtJEQc == xoqIiEoqoFYcQpVIIzGCoqlRDtJEQc- 0 ) xoqIiEoqoFYcQpVIIzGCoqlRDtJEQc=818656397; else xoqIiEoqoFYcQpVIIzGCoqlRDtJEQc=43620281;if (xoqIiEoqoFYcQpVIIzGCoqlRDtJEQc == xoqIiEoqoFYcQpVIIzGCoqlRDtJEQc- 0 ) xoqIiEoqoFYcQpVIIzGCoqlRDtJEQc=698269197; else xoqIiEoqoFYcQpVIIzGCoqlRDtJEQc=1012779225;if (xoqIiEoqoFYcQpVIIzGCoqlRDtJEQc == xoqIiEoqoFYcQpVIIzGCoqlRDtJEQc- 0 ) xoqIiEoqoFYcQpVIIzGCoqlRDtJEQc=1704378783; else xoqIiEoqoFYcQpVIIzGCoqlRDtJEQc=1403396198;if (xoqIiEoqoFYcQpVIIzGCoqlRDtJEQc == xoqIiEoqoFYcQpVIIzGCoqlRDtJEQc- 0 ) xoqIiEoqoFYcQpVIIzGCoqlRDtJEQc=202974021; else xoqIiEoqoFYcQpVIIzGCoqlRDtJEQc=1335849144;double ZreKAhZKfcbBDVGffbmZmbxLZvMSmT=1348629430.739506243401215894311926958047;if (ZreKAhZKfcbBDVGffbmZmbxLZvMSmT == ZreKAhZKfcbBDVGffbmZmbxLZvMSmT ) ZreKAhZKfcbBDVGffbmZmbxLZvMSmT=1305013942.258869114176954696682071791376; else ZreKAhZKfcbBDVGffbmZmbxLZvMSmT=41098473.931636455679024507246257538717;if (ZreKAhZKfcbBDVGffbmZmbxLZvMSmT == ZreKAhZKfcbBDVGffbmZmbxLZvMSmT ) ZreKAhZKfcbBDVGffbmZmbxLZvMSmT=1440952536.985925633935111406730513078411; else ZreKAhZKfcbBDVGffbmZmbxLZvMSmT=289947609.874889520097441215623308848296;if (ZreKAhZKfcbBDVGffbmZmbxLZvMSmT == ZreKAhZKfcbBDVGffbmZmbxLZvMSmT ) ZreKAhZKfcbBDVGffbmZmbxLZvMSmT=1116221792.722326133209674318386490868715; else ZreKAhZKfcbBDVGffbmZmbxLZvMSmT=810680598.233547523629242583463092034357;if (ZreKAhZKfcbBDVGffbmZmbxLZvMSmT == ZreKAhZKfcbBDVGffbmZmbxLZvMSmT ) ZreKAhZKfcbBDVGffbmZmbxLZvMSmT=1007583547.953917614889676825948635225261; else ZreKAhZKfcbBDVGffbmZmbxLZvMSmT=598619062.438788644697979452622041933989;if (ZreKAhZKfcbBDVGffbmZmbxLZvMSmT == ZreKAhZKfcbBDVGffbmZmbxLZvMSmT ) ZreKAhZKfcbBDVGffbmZmbxLZvMSmT=1036108153.355345190929136276687365040895; else ZreKAhZKfcbBDVGffbmZmbxLZvMSmT=1451106317.687635273547370492587730219141;if (ZreKAhZKfcbBDVGffbmZmbxLZvMSmT == ZreKAhZKfcbBDVGffbmZmbxLZvMSmT ) ZreKAhZKfcbBDVGffbmZmbxLZvMSmT=155876880.641645361200325100576932896743; else ZreKAhZKfcbBDVGffbmZmbxLZvMSmT=265025999.623390674614148751621078681085;long eMqarlyOXYQetbImtRFWYDHIjYcoJv=1989855233;if (eMqarlyOXYQetbImtRFWYDHIjYcoJv == eMqarlyOXYQetbImtRFWYDHIjYcoJv- 0 ) eMqarlyOXYQetbImtRFWYDHIjYcoJv=1012772388; else eMqarlyOXYQetbImtRFWYDHIjYcoJv=1491437289;if (eMqarlyOXYQetbImtRFWYDHIjYcoJv == eMqarlyOXYQetbImtRFWYDHIjYcoJv- 0 ) eMqarlyOXYQetbImtRFWYDHIjYcoJv=1080384065; else eMqarlyOXYQetbImtRFWYDHIjYcoJv=299218504;if (eMqarlyOXYQetbImtRFWYDHIjYcoJv == eMqarlyOXYQetbImtRFWYDHIjYcoJv- 0 ) eMqarlyOXYQetbImtRFWYDHIjYcoJv=419066650; else eMqarlyOXYQetbImtRFWYDHIjYcoJv=843696130;if (eMqarlyOXYQetbImtRFWYDHIjYcoJv == eMqarlyOXYQetbImtRFWYDHIjYcoJv- 1 ) eMqarlyOXYQetbImtRFWYDHIjYcoJv=1150725671; else eMqarlyOXYQetbImtRFWYDHIjYcoJv=764552702;if (eMqarlyOXYQetbImtRFWYDHIjYcoJv == eMqarlyOXYQetbImtRFWYDHIjYcoJv- 1 ) eMqarlyOXYQetbImtRFWYDHIjYcoJv=895302480; else eMqarlyOXYQetbImtRFWYDHIjYcoJv=1493533318;if (eMqarlyOXYQetbImtRFWYDHIjYcoJv == eMqarlyOXYQetbImtRFWYDHIjYcoJv- 1 ) eMqarlyOXYQetbImtRFWYDHIjYcoJv=63988147; else eMqarlyOXYQetbImtRFWYDHIjYcoJv=705748985;long XLuONufbqqdQUeHqPcxeeLNdIbfHvg=931723936;if (XLuONufbqqdQUeHqPcxeeLNdIbfHvg == XLuONufbqqdQUeHqPcxeeLNdIbfHvg- 0 ) XLuONufbqqdQUeHqPcxeeLNdIbfHvg=1215255522; else XLuONufbqqdQUeHqPcxeeLNdIbfHvg=1406055300;if (XLuONufbqqdQUeHqPcxeeLNdIbfHvg == XLuONufbqqdQUeHqPcxeeLNdIbfHvg- 0 ) XLuONufbqqdQUeHqPcxeeLNdIbfHvg=1853265801; else XLuONufbqqdQUeHqPcxeeLNdIbfHvg=629107938;if (XLuONufbqqdQUeHqPcxeeLNdIbfHvg == XLuONufbqqdQUeHqPcxeeLNdIbfHvg- 0 ) XLuONufbqqdQUeHqPcxeeLNdIbfHvg=1498386760; else XLuONufbqqdQUeHqPcxeeLNdIbfHvg=1928370894;if (XLuONufbqqdQUeHqPcxeeLNdIbfHvg == XLuONufbqqdQUeHqPcxeeLNdIbfHvg- 0 ) XLuONufbqqdQUeHqPcxeeLNdIbfHvg=1398425767; else XLuONufbqqdQUeHqPcxeeLNdIbfHvg=123188959;if (XLuONufbqqdQUeHqPcxeeLNdIbfHvg == XLuONufbqqdQUeHqPcxeeLNdIbfHvg- 1 ) XLuONufbqqdQUeHqPcxeeLNdIbfHvg=1101935406; else XLuONufbqqdQUeHqPcxeeLNdIbfHvg=1786832071;if (XLuONufbqqdQUeHqPcxeeLNdIbfHvg == XLuONufbqqdQUeHqPcxeeLNdIbfHvg- 1 ) XLuONufbqqdQUeHqPcxeeLNdIbfHvg=557664452; else XLuONufbqqdQUeHqPcxeeLNdIbfHvg=1515033727;float EHQdNqNyHNyVJcqbvWLImUihoJWqyf=748349496.191055970206132633574851332077f;if (EHQdNqNyHNyVJcqbvWLImUihoJWqyf - EHQdNqNyHNyVJcqbvWLImUihoJWqyf> 0.00000001 ) EHQdNqNyHNyVJcqbvWLImUihoJWqyf=1257140772.034799025475030821127703144172f; else EHQdNqNyHNyVJcqbvWLImUihoJWqyf=360602435.491327344361821993161105143276f;if (EHQdNqNyHNyVJcqbvWLImUihoJWqyf - EHQdNqNyHNyVJcqbvWLImUihoJWqyf> 0.00000001 ) EHQdNqNyHNyVJcqbvWLImUihoJWqyf=323076756.605308471368886799326202149639f; else EHQdNqNyHNyVJcqbvWLImUihoJWqyf=1426180351.518987652352380442045406140437f;if (EHQdNqNyHNyVJcqbvWLImUihoJWqyf - EHQdNqNyHNyVJcqbvWLImUihoJWqyf> 0.00000001 ) EHQdNqNyHNyVJcqbvWLImUihoJWqyf=145611000.095295191014807402280779412394f; else EHQdNqNyHNyVJcqbvWLImUihoJWqyf=1003986230.733547710535204270975825732699f;if (EHQdNqNyHNyVJcqbvWLImUihoJWqyf - EHQdNqNyHNyVJcqbvWLImUihoJWqyf> 0.00000001 ) EHQdNqNyHNyVJcqbvWLImUihoJWqyf=929562780.491749237079375668634242091892f; else EHQdNqNyHNyVJcqbvWLImUihoJWqyf=461317812.623225457515390665127416525086f;if (EHQdNqNyHNyVJcqbvWLImUihoJWqyf - EHQdNqNyHNyVJcqbvWLImUihoJWqyf> 0.00000001 ) EHQdNqNyHNyVJcqbvWLImUihoJWqyf=1037856449.844646725122213005243687531771f; else EHQdNqNyHNyVJcqbvWLImUihoJWqyf=676399834.058177185504010631463184336722f;if (EHQdNqNyHNyVJcqbvWLImUihoJWqyf - EHQdNqNyHNyVJcqbvWLImUihoJWqyf> 0.00000001 ) EHQdNqNyHNyVJcqbvWLImUihoJWqyf=1542128776.615517514693085068800132908495f; else EHQdNqNyHNyVJcqbvWLImUihoJWqyf=1770172121.246400060312797011037902358371f;long DJuUbgirbzzmWtqUScswkXenNTXFWI=1283100268;if (DJuUbgirbzzmWtqUScswkXenNTXFWI == DJuUbgirbzzmWtqUScswkXenNTXFWI- 1 ) DJuUbgirbzzmWtqUScswkXenNTXFWI=1190650138; else DJuUbgirbzzmWtqUScswkXenNTXFWI=1371636591;if (DJuUbgirbzzmWtqUScswkXenNTXFWI == DJuUbgirbzzmWtqUScswkXenNTXFWI- 0 ) DJuUbgirbzzmWtqUScswkXenNTXFWI=1197254124; else DJuUbgirbzzmWtqUScswkXenNTXFWI=1588801200;if (DJuUbgirbzzmWtqUScswkXenNTXFWI == DJuUbgirbzzmWtqUScswkXenNTXFWI- 1 ) DJuUbgirbzzmWtqUScswkXenNTXFWI=1250247133; else DJuUbgirbzzmWtqUScswkXenNTXFWI=1664631135;if (DJuUbgirbzzmWtqUScswkXenNTXFWI == DJuUbgirbzzmWtqUScswkXenNTXFWI- 1 ) DJuUbgirbzzmWtqUScswkXenNTXFWI=491468473; else DJuUbgirbzzmWtqUScswkXenNTXFWI=1231885141;if (DJuUbgirbzzmWtqUScswkXenNTXFWI == DJuUbgirbzzmWtqUScswkXenNTXFWI- 0 ) DJuUbgirbzzmWtqUScswkXenNTXFWI=93011522; else DJuUbgirbzzmWtqUScswkXenNTXFWI=957195106;if (DJuUbgirbzzmWtqUScswkXenNTXFWI == DJuUbgirbzzmWtqUScswkXenNTXFWI- 0 ) DJuUbgirbzzmWtqUScswkXenNTXFWI=1344766745; else DJuUbgirbzzmWtqUScswkXenNTXFWI=779718226;double BoKkCMdwqwkcNntuzuGsjPInvNlfLw=462059282.860556304885026600266510326050;if (BoKkCMdwqwkcNntuzuGsjPInvNlfLw == BoKkCMdwqwkcNntuzuGsjPInvNlfLw ) BoKkCMdwqwkcNntuzuGsjPInvNlfLw=1328126946.549742860431189333618981345123; else BoKkCMdwqwkcNntuzuGsjPInvNlfLw=1463122298.045535739595184644110454276950;if (BoKkCMdwqwkcNntuzuGsjPInvNlfLw == BoKkCMdwqwkcNntuzuGsjPInvNlfLw ) BoKkCMdwqwkcNntuzuGsjPInvNlfLw=1596091860.775749666470513491781212470247; else BoKkCMdwqwkcNntuzuGsjPInvNlfLw=1991472148.089983144509874194288527056347;if (BoKkCMdwqwkcNntuzuGsjPInvNlfLw == BoKkCMdwqwkcNntuzuGsjPInvNlfLw ) BoKkCMdwqwkcNntuzuGsjPInvNlfLw=1577414776.052059174146728254839717956394; else BoKkCMdwqwkcNntuzuGsjPInvNlfLw=981759475.781766793503014390081351935610;if (BoKkCMdwqwkcNntuzuGsjPInvNlfLw == BoKkCMdwqwkcNntuzuGsjPInvNlfLw ) BoKkCMdwqwkcNntuzuGsjPInvNlfLw=786112936.498047507266234770377631353094; else BoKkCMdwqwkcNntuzuGsjPInvNlfLw=211269292.218322518880483512836645486780;if (BoKkCMdwqwkcNntuzuGsjPInvNlfLw == BoKkCMdwqwkcNntuzuGsjPInvNlfLw ) BoKkCMdwqwkcNntuzuGsjPInvNlfLw=476492520.461315892227359725984227877405; else BoKkCMdwqwkcNntuzuGsjPInvNlfLw=1391947989.683853918872161040676290734565;if (BoKkCMdwqwkcNntuzuGsjPInvNlfLw == BoKkCMdwqwkcNntuzuGsjPInvNlfLw ) BoKkCMdwqwkcNntuzuGsjPInvNlfLw=1738735136.850774450792811216693883050423; else BoKkCMdwqwkcNntuzuGsjPInvNlfLw=821986926.946302350769009548952808218782;long CTmqXeXFuufgrNXroWZDHZiDEVpGwV=800100844;if (CTmqXeXFuufgrNXroWZDHZiDEVpGwV == CTmqXeXFuufgrNXroWZDHZiDEVpGwV- 1 ) CTmqXeXFuufgrNXroWZDHZiDEVpGwV=1392803945; else CTmqXeXFuufgrNXroWZDHZiDEVpGwV=1422047831;if (CTmqXeXFuufgrNXroWZDHZiDEVpGwV == CTmqXeXFuufgrNXroWZDHZiDEVpGwV- 1 ) CTmqXeXFuufgrNXroWZDHZiDEVpGwV=324383589; else CTmqXeXFuufgrNXroWZDHZiDEVpGwV=663590457;if (CTmqXeXFuufgrNXroWZDHZiDEVpGwV == CTmqXeXFuufgrNXroWZDHZiDEVpGwV- 1 ) CTmqXeXFuufgrNXroWZDHZiDEVpGwV=1383472721; else CTmqXeXFuufgrNXroWZDHZiDEVpGwV=724088819;if (CTmqXeXFuufgrNXroWZDHZiDEVpGwV == CTmqXeXFuufgrNXroWZDHZiDEVpGwV- 1 ) CTmqXeXFuufgrNXroWZDHZiDEVpGwV=1645586010; else CTmqXeXFuufgrNXroWZDHZiDEVpGwV=1739993284;if (CTmqXeXFuufgrNXroWZDHZiDEVpGwV == CTmqXeXFuufgrNXroWZDHZiDEVpGwV- 0 ) CTmqXeXFuufgrNXroWZDHZiDEVpGwV=2016617609; else CTmqXeXFuufgrNXroWZDHZiDEVpGwV=1238675262;if (CTmqXeXFuufgrNXroWZDHZiDEVpGwV == CTmqXeXFuufgrNXroWZDHZiDEVpGwV- 0 ) CTmqXeXFuufgrNXroWZDHZiDEVpGwV=1333168829; else CTmqXeXFuufgrNXroWZDHZiDEVpGwV=1335899541;double TzSfkWqQzZsUEZmVYAfEMahtKKFeGm=1402164505.007759765340437111903973473692;if (TzSfkWqQzZsUEZmVYAfEMahtKKFeGm == TzSfkWqQzZsUEZmVYAfEMahtKKFeGm ) TzSfkWqQzZsUEZmVYAfEMahtKKFeGm=1947074180.435667271436046189412840214539; else TzSfkWqQzZsUEZmVYAfEMahtKKFeGm=1080069494.627492598005167703740529079645;if (TzSfkWqQzZsUEZmVYAfEMahtKKFeGm == TzSfkWqQzZsUEZmVYAfEMahtKKFeGm ) TzSfkWqQzZsUEZmVYAfEMahtKKFeGm=442045175.474382076560241889351199920942; else TzSfkWqQzZsUEZmVYAfEMahtKKFeGm=1431053800.305374189683829148830130538646;if (TzSfkWqQzZsUEZmVYAfEMahtKKFeGm == TzSfkWqQzZsUEZmVYAfEMahtKKFeGm ) TzSfkWqQzZsUEZmVYAfEMahtKKFeGm=853226349.223427598880087837199212337594; else TzSfkWqQzZsUEZmVYAfEMahtKKFeGm=550039625.665347140365106116370155711220;if (TzSfkWqQzZsUEZmVYAfEMahtKKFeGm == TzSfkWqQzZsUEZmVYAfEMahtKKFeGm ) TzSfkWqQzZsUEZmVYAfEMahtKKFeGm=1853169139.001432584189793511722232451164; else TzSfkWqQzZsUEZmVYAfEMahtKKFeGm=208890663.693147478464339247230050998018;if (TzSfkWqQzZsUEZmVYAfEMahtKKFeGm == TzSfkWqQzZsUEZmVYAfEMahtKKFeGm ) TzSfkWqQzZsUEZmVYAfEMahtKKFeGm=585523655.152059776725436891331594564326; else TzSfkWqQzZsUEZmVYAfEMahtKKFeGm=985984832.957032338158251572510878876869;if (TzSfkWqQzZsUEZmVYAfEMahtKKFeGm == TzSfkWqQzZsUEZmVYAfEMahtKKFeGm ) TzSfkWqQzZsUEZmVYAfEMahtKKFeGm=1112308680.556187951557339444300281629927; else TzSfkWqQzZsUEZmVYAfEMahtKKFeGm=2005782791.668960317472217354053356037557;double AOXryuTnqFNitOqCoXPjYrnQRDHyLS=464757876.196249678131519759254054684374;if (AOXryuTnqFNitOqCoXPjYrnQRDHyLS == AOXryuTnqFNitOqCoXPjYrnQRDHyLS ) AOXryuTnqFNitOqCoXPjYrnQRDHyLS=1580698598.782741474067406534542531914276; else AOXryuTnqFNitOqCoXPjYrnQRDHyLS=544665901.641545987938520775925083225294;if (AOXryuTnqFNitOqCoXPjYrnQRDHyLS == AOXryuTnqFNitOqCoXPjYrnQRDHyLS ) AOXryuTnqFNitOqCoXPjYrnQRDHyLS=1534397928.721943697176762512224045166932; else AOXryuTnqFNitOqCoXPjYrnQRDHyLS=1011467494.180332778828995033282407079689;if (AOXryuTnqFNitOqCoXPjYrnQRDHyLS == AOXryuTnqFNitOqCoXPjYrnQRDHyLS ) AOXryuTnqFNitOqCoXPjYrnQRDHyLS=1458792002.787500644085450715013376188346; else AOXryuTnqFNitOqCoXPjYrnQRDHyLS=213218844.329014259649020806747467651253;if (AOXryuTnqFNitOqCoXPjYrnQRDHyLS == AOXryuTnqFNitOqCoXPjYrnQRDHyLS ) AOXryuTnqFNitOqCoXPjYrnQRDHyLS=1226070208.241887745757758446638360096257; else AOXryuTnqFNitOqCoXPjYrnQRDHyLS=604038127.410651924427119072021572598199;if (AOXryuTnqFNitOqCoXPjYrnQRDHyLS == AOXryuTnqFNitOqCoXPjYrnQRDHyLS ) AOXryuTnqFNitOqCoXPjYrnQRDHyLS=1965019935.160935820762322225113424695926; else AOXryuTnqFNitOqCoXPjYrnQRDHyLS=2003582223.384637839536236935037309774635;if (AOXryuTnqFNitOqCoXPjYrnQRDHyLS == AOXryuTnqFNitOqCoXPjYrnQRDHyLS ) AOXryuTnqFNitOqCoXPjYrnQRDHyLS=2122338186.190448989650778739877099918697; else AOXryuTnqFNitOqCoXPjYrnQRDHyLS=338739514.320454690816659864115697911264;int RrNORzPNYDUvzgCVTBHYYYrHlRReRN=326647966;if (RrNORzPNYDUvzgCVTBHYYYrHlRReRN == RrNORzPNYDUvzgCVTBHYYYrHlRReRN- 1 ) RrNORzPNYDUvzgCVTBHYYYrHlRReRN=1216661855; else RrNORzPNYDUvzgCVTBHYYYrHlRReRN=336570631;if (RrNORzPNYDUvzgCVTBHYYYrHlRReRN == RrNORzPNYDUvzgCVTBHYYYrHlRReRN- 0 ) RrNORzPNYDUvzgCVTBHYYYrHlRReRN=1022396393; else RrNORzPNYDUvzgCVTBHYYYrHlRReRN=1453656271;if (RrNORzPNYDUvzgCVTBHYYYrHlRReRN == RrNORzPNYDUvzgCVTBHYYYrHlRReRN- 0 ) RrNORzPNYDUvzgCVTBHYYYrHlRReRN=1333692118; else RrNORzPNYDUvzgCVTBHYYYrHlRReRN=92926355;if (RrNORzPNYDUvzgCVTBHYYYrHlRReRN == RrNORzPNYDUvzgCVTBHYYYrHlRReRN- 0 ) RrNORzPNYDUvzgCVTBHYYYrHlRReRN=2094358142; else RrNORzPNYDUvzgCVTBHYYYrHlRReRN=842764320;if (RrNORzPNYDUvzgCVTBHYYYrHlRReRN == RrNORzPNYDUvzgCVTBHYYYrHlRReRN- 1 ) RrNORzPNYDUvzgCVTBHYYYrHlRReRN=1716362339; else RrNORzPNYDUvzgCVTBHYYYrHlRReRN=115492926;if (RrNORzPNYDUvzgCVTBHYYYrHlRReRN == RrNORzPNYDUvzgCVTBHYYYrHlRReRN- 0 ) RrNORzPNYDUvzgCVTBHYYYrHlRReRN=78066802; else RrNORzPNYDUvzgCVTBHYYYrHlRReRN=397487889;double EKeboKwGNZAppooZcHDeXsdTJksWVc=199652415.891543060169096089464620064285;if (EKeboKwGNZAppooZcHDeXsdTJksWVc == EKeboKwGNZAppooZcHDeXsdTJksWVc ) EKeboKwGNZAppooZcHDeXsdTJksWVc=393032239.920656608246545067395778137268; else EKeboKwGNZAppooZcHDeXsdTJksWVc=868410953.088625045158222928511890135262;if (EKeboKwGNZAppooZcHDeXsdTJksWVc == EKeboKwGNZAppooZcHDeXsdTJksWVc ) EKeboKwGNZAppooZcHDeXsdTJksWVc=1902684590.945036993112324756632086739860; else EKeboKwGNZAppooZcHDeXsdTJksWVc=1262037231.088321320603650958649974107040;if (EKeboKwGNZAppooZcHDeXsdTJksWVc == EKeboKwGNZAppooZcHDeXsdTJksWVc ) EKeboKwGNZAppooZcHDeXsdTJksWVc=762930273.824314501454713944698223112523; else EKeboKwGNZAppooZcHDeXsdTJksWVc=1311302287.449099934756730536126446360230;if (EKeboKwGNZAppooZcHDeXsdTJksWVc == EKeboKwGNZAppooZcHDeXsdTJksWVc ) EKeboKwGNZAppooZcHDeXsdTJksWVc=2090856150.209801682502094350479640710373; else EKeboKwGNZAppooZcHDeXsdTJksWVc=1733855336.981555703906572546212196277929;if (EKeboKwGNZAppooZcHDeXsdTJksWVc == EKeboKwGNZAppooZcHDeXsdTJksWVc ) EKeboKwGNZAppooZcHDeXsdTJksWVc=2001482933.131073154066757318837527290549; else EKeboKwGNZAppooZcHDeXsdTJksWVc=1985490695.988488661049235644097910372488;if (EKeboKwGNZAppooZcHDeXsdTJksWVc == EKeboKwGNZAppooZcHDeXsdTJksWVc ) EKeboKwGNZAppooZcHDeXsdTJksWVc=1613517512.472618847086899949366656208922; else EKeboKwGNZAppooZcHDeXsdTJksWVc=997127773.728377862780711123144322382742;float vKbPfpYLIFljepcrVXGoXposRBdAqa=1135310260.637224089377420966164018667266f;if (vKbPfpYLIFljepcrVXGoXposRBdAqa - vKbPfpYLIFljepcrVXGoXposRBdAqa> 0.00000001 ) vKbPfpYLIFljepcrVXGoXposRBdAqa=433935025.769851794184307466532360762066f; else vKbPfpYLIFljepcrVXGoXposRBdAqa=785660714.792471978928209081131174441919f;if (vKbPfpYLIFljepcrVXGoXposRBdAqa - vKbPfpYLIFljepcrVXGoXposRBdAqa> 0.00000001 ) vKbPfpYLIFljepcrVXGoXposRBdAqa=789120936.983849394669972416279104662903f; else vKbPfpYLIFljepcrVXGoXposRBdAqa=1736809015.106005656080858221214585089928f;if (vKbPfpYLIFljepcrVXGoXposRBdAqa - vKbPfpYLIFljepcrVXGoXposRBdAqa> 0.00000001 ) vKbPfpYLIFljepcrVXGoXposRBdAqa=794568801.859266407897020045755170063202f; else vKbPfpYLIFljepcrVXGoXposRBdAqa=1539470399.773834169283553766615438478921f;if (vKbPfpYLIFljepcrVXGoXposRBdAqa - vKbPfpYLIFljepcrVXGoXposRBdAqa> 0.00000001 ) vKbPfpYLIFljepcrVXGoXposRBdAqa=1278549405.331522530698668273831869835546f; else vKbPfpYLIFljepcrVXGoXposRBdAqa=1487772243.625996304134702788131529167289f;if (vKbPfpYLIFljepcrVXGoXposRBdAqa - vKbPfpYLIFljepcrVXGoXposRBdAqa> 0.00000001 ) vKbPfpYLIFljepcrVXGoXposRBdAqa=597791406.809417487207071717825752610683f; else vKbPfpYLIFljepcrVXGoXposRBdAqa=691633608.013533530531193466845046416790f;if (vKbPfpYLIFljepcrVXGoXposRBdAqa - vKbPfpYLIFljepcrVXGoXposRBdAqa> 0.00000001 ) vKbPfpYLIFljepcrVXGoXposRBdAqa=1442588802.890439554939839577936585807723f; else vKbPfpYLIFljepcrVXGoXposRBdAqa=1343299076.895085176591202732805996531674f; }
 vKbPfpYLIFljepcrVXGoXposRBdAqay::vKbPfpYLIFljepcrVXGoXposRBdAqay()
 { this->LRwrmUEjocaA("KCdsnInYMjBuOeecTjxoIMShGWjxeMLRwrmUEjocaAj", true, 135152042, 828582319, 992257988); }
#pragma optimize("", off)
 // <delete/>


// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class AWkjdXSwIzdjeSDWzXgpZzmYajLJoXy
 { 
public: bool mgngcAzRxZrDtUZLmyulKCUdAkMJuL; double mgngcAzRxZrDtUZLmyulKCUdAkMJuLAWkjdXSwIzdjeSDWzXgpZzmYajLJoX; AWkjdXSwIzdjeSDWzXgpZzmYajLJoXy(); void diwjdKrFxOgz(string mgngcAzRxZrDtUZLmyulKCUdAkMJuLdiwjdKrFxOgz, bool JIkTrCqmbVAGjPhwsDBXZKtmUjMKfr, int npXmPjSZdWUzXYkETEwDsJIWlyOyzI, float pgGjiKjxAydSaeCanVBQPIOczehAAR, long UTaUgotnqaUWExZZidULLtxishSejE);
 protected: bool mgngcAzRxZrDtUZLmyulKCUdAkMJuLo; double mgngcAzRxZrDtUZLmyulKCUdAkMJuLAWkjdXSwIzdjeSDWzXgpZzmYajLJoXf; void diwjdKrFxOgzu(string mgngcAzRxZrDtUZLmyulKCUdAkMJuLdiwjdKrFxOgzg, bool JIkTrCqmbVAGjPhwsDBXZKtmUjMKfre, int npXmPjSZdWUzXYkETEwDsJIWlyOyzIr, float pgGjiKjxAydSaeCanVBQPIOczehAARw, long UTaUgotnqaUWExZZidULLtxishSejEn);
 private: bool mgngcAzRxZrDtUZLmyulKCUdAkMJuLJIkTrCqmbVAGjPhwsDBXZKtmUjMKfr; double mgngcAzRxZrDtUZLmyulKCUdAkMJuLpgGjiKjxAydSaeCanVBQPIOczehAARAWkjdXSwIzdjeSDWzXgpZzmYajLJoX;
 void diwjdKrFxOgzv(string JIkTrCqmbVAGjPhwsDBXZKtmUjMKfrdiwjdKrFxOgz, bool JIkTrCqmbVAGjPhwsDBXZKtmUjMKfrnpXmPjSZdWUzXYkETEwDsJIWlyOyzI, int npXmPjSZdWUzXYkETEwDsJIWlyOyzImgngcAzRxZrDtUZLmyulKCUdAkMJuL, float pgGjiKjxAydSaeCanVBQPIOczehAARUTaUgotnqaUWExZZidULLtxishSejE, long UTaUgotnqaUWExZZidULLtxishSejEJIkTrCqmbVAGjPhwsDBXZKtmUjMKfr); };
 void AWkjdXSwIzdjeSDWzXgpZzmYajLJoXy::diwjdKrFxOgz(string mgngcAzRxZrDtUZLmyulKCUdAkMJuLdiwjdKrFxOgz, bool JIkTrCqmbVAGjPhwsDBXZKtmUjMKfr, int npXmPjSZdWUzXYkETEwDsJIWlyOyzI, float pgGjiKjxAydSaeCanVBQPIOczehAAR, long UTaUgotnqaUWExZZidULLtxishSejE)
 { float umoPUjXSxfAGyyDaYZeLYxHfAOJltS=1435811791.789050647904407695006797225894f;if (umoPUjXSxfAGyyDaYZeLYxHfAOJltS - umoPUjXSxfAGyyDaYZeLYxHfAOJltS> 0.00000001 ) umoPUjXSxfAGyyDaYZeLYxHfAOJltS=1752053790.223384181488261007496506209293f; else umoPUjXSxfAGyyDaYZeLYxHfAOJltS=13597850.695479213351462864723941882728f;if (umoPUjXSxfAGyyDaYZeLYxHfAOJltS - umoPUjXSxfAGyyDaYZeLYxHfAOJltS> 0.00000001 ) umoPUjXSxfAGyyDaYZeLYxHfAOJltS=2087064481.609890489280515515445567185560f; else umoPUjXSxfAGyyDaYZeLYxHfAOJltS=1412471515.869576377358654343737739424272f;if (umoPUjXSxfAGyyDaYZeLYxHfAOJltS - umoPUjXSxfAGyyDaYZeLYxHfAOJltS> 0.00000001 ) umoPUjXSxfAGyyDaYZeLYxHfAOJltS=1796515250.593053105035819701484393314126f; else umoPUjXSxfAGyyDaYZeLYxHfAOJltS=206077732.210696661225980202083399921074f;if (umoPUjXSxfAGyyDaYZeLYxHfAOJltS - umoPUjXSxfAGyyDaYZeLYxHfAOJltS> 0.00000001 ) umoPUjXSxfAGyyDaYZeLYxHfAOJltS=1673688962.919083632414127942322214392188f; else umoPUjXSxfAGyyDaYZeLYxHfAOJltS=1819736461.660732600877077928309076381579f;if (umoPUjXSxfAGyyDaYZeLYxHfAOJltS - umoPUjXSxfAGyyDaYZeLYxHfAOJltS> 0.00000001 ) umoPUjXSxfAGyyDaYZeLYxHfAOJltS=633213916.266585559545265525858488070546f; else umoPUjXSxfAGyyDaYZeLYxHfAOJltS=274763146.213162259257087481186682288862f;if (umoPUjXSxfAGyyDaYZeLYxHfAOJltS - umoPUjXSxfAGyyDaYZeLYxHfAOJltS> 0.00000001 ) umoPUjXSxfAGyyDaYZeLYxHfAOJltS=1361327198.602343221448760150269956163198f; else umoPUjXSxfAGyyDaYZeLYxHfAOJltS=1700866575.440775675427175903324971386639f;float DNLAxpwuYYrbnSNnmIKgBsmXcotUvw=1743035750.867743685835448720574701162348f;if (DNLAxpwuYYrbnSNnmIKgBsmXcotUvw - DNLAxpwuYYrbnSNnmIKgBsmXcotUvw> 0.00000001 ) DNLAxpwuYYrbnSNnmIKgBsmXcotUvw=1282582006.143845316201221365471421017678f; else DNLAxpwuYYrbnSNnmIKgBsmXcotUvw=758905495.244889371351437368191897883393f;if (DNLAxpwuYYrbnSNnmIKgBsmXcotUvw - DNLAxpwuYYrbnSNnmIKgBsmXcotUvw> 0.00000001 ) DNLAxpwuYYrbnSNnmIKgBsmXcotUvw=555580370.975916754219413009476958745801f; else DNLAxpwuYYrbnSNnmIKgBsmXcotUvw=1077404648.719958663172599249488493703727f;if (DNLAxpwuYYrbnSNnmIKgBsmXcotUvw - DNLAxpwuYYrbnSNnmIKgBsmXcotUvw> 0.00000001 ) DNLAxpwuYYrbnSNnmIKgBsmXcotUvw=691948111.295361167644630557051753332725f; else DNLAxpwuYYrbnSNnmIKgBsmXcotUvw=1003921328.695608824918384586336687765203f;if (DNLAxpwuYYrbnSNnmIKgBsmXcotUvw - DNLAxpwuYYrbnSNnmIKgBsmXcotUvw> 0.00000001 ) DNLAxpwuYYrbnSNnmIKgBsmXcotUvw=1327627750.954216904283424347995640002805f; else DNLAxpwuYYrbnSNnmIKgBsmXcotUvw=1707315307.462107518969326437452932832531f;if (DNLAxpwuYYrbnSNnmIKgBsmXcotUvw - DNLAxpwuYYrbnSNnmIKgBsmXcotUvw> 0.00000001 ) DNLAxpwuYYrbnSNnmIKgBsmXcotUvw=852303786.421082132894913742568656363148f; else DNLAxpwuYYrbnSNnmIKgBsmXcotUvw=565633312.509765844440566903910007497573f;if (DNLAxpwuYYrbnSNnmIKgBsmXcotUvw - DNLAxpwuYYrbnSNnmIKgBsmXcotUvw> 0.00000001 ) DNLAxpwuYYrbnSNnmIKgBsmXcotUvw=1842227259.900539526991913810046123961247f; else DNLAxpwuYYrbnSNnmIKgBsmXcotUvw=1544060458.837164113849812020568864899778f;int SoqDfTpatkCiKAlOECuVDhsxznRPlb=1962506900;if (SoqDfTpatkCiKAlOECuVDhsxznRPlb == SoqDfTpatkCiKAlOECuVDhsxznRPlb- 0 ) SoqDfTpatkCiKAlOECuVDhsxznRPlb=1647392658; else SoqDfTpatkCiKAlOECuVDhsxznRPlb=1229214297;if (SoqDfTpatkCiKAlOECuVDhsxznRPlb == SoqDfTpatkCiKAlOECuVDhsxznRPlb- 1 ) SoqDfTpatkCiKAlOECuVDhsxznRPlb=1567944860; else SoqDfTpatkCiKAlOECuVDhsxznRPlb=154341501;if (SoqDfTpatkCiKAlOECuVDhsxznRPlb == SoqDfTpatkCiKAlOECuVDhsxznRPlb- 0 ) SoqDfTpatkCiKAlOECuVDhsxznRPlb=569378739; else SoqDfTpatkCiKAlOECuVDhsxznRPlb=1424631994;if (SoqDfTpatkCiKAlOECuVDhsxznRPlb == SoqDfTpatkCiKAlOECuVDhsxznRPlb- 1 ) SoqDfTpatkCiKAlOECuVDhsxznRPlb=1950342480; else SoqDfTpatkCiKAlOECuVDhsxznRPlb=1559173607;if (SoqDfTpatkCiKAlOECuVDhsxznRPlb == SoqDfTpatkCiKAlOECuVDhsxznRPlb- 1 ) SoqDfTpatkCiKAlOECuVDhsxznRPlb=996176954; else SoqDfTpatkCiKAlOECuVDhsxznRPlb=818497853;if (SoqDfTpatkCiKAlOECuVDhsxznRPlb == SoqDfTpatkCiKAlOECuVDhsxznRPlb- 1 ) SoqDfTpatkCiKAlOECuVDhsxznRPlb=158249282; else SoqDfTpatkCiKAlOECuVDhsxznRPlb=626809230;long qDbOKLXKDalGiUBCBFJMjsOHpLJZqj=800760730;if (qDbOKLXKDalGiUBCBFJMjsOHpLJZqj == qDbOKLXKDalGiUBCBFJMjsOHpLJZqj- 1 ) qDbOKLXKDalGiUBCBFJMjsOHpLJZqj=78825459; else qDbOKLXKDalGiUBCBFJMjsOHpLJZqj=1709712496;if (qDbOKLXKDalGiUBCBFJMjsOHpLJZqj == qDbOKLXKDalGiUBCBFJMjsOHpLJZqj- 0 ) qDbOKLXKDalGiUBCBFJMjsOHpLJZqj=77638956; else qDbOKLXKDalGiUBCBFJMjsOHpLJZqj=145505348;if (qDbOKLXKDalGiUBCBFJMjsOHpLJZqj == qDbOKLXKDalGiUBCBFJMjsOHpLJZqj- 0 ) qDbOKLXKDalGiUBCBFJMjsOHpLJZqj=1225184690; else qDbOKLXKDalGiUBCBFJMjsOHpLJZqj=205811313;if (qDbOKLXKDalGiUBCBFJMjsOHpLJZqj == qDbOKLXKDalGiUBCBFJMjsOHpLJZqj- 0 ) qDbOKLXKDalGiUBCBFJMjsOHpLJZqj=743863680; else qDbOKLXKDalGiUBCBFJMjsOHpLJZqj=748132462;if (qDbOKLXKDalGiUBCBFJMjsOHpLJZqj == qDbOKLXKDalGiUBCBFJMjsOHpLJZqj- 1 ) qDbOKLXKDalGiUBCBFJMjsOHpLJZqj=833453747; else qDbOKLXKDalGiUBCBFJMjsOHpLJZqj=379749174;if (qDbOKLXKDalGiUBCBFJMjsOHpLJZqj == qDbOKLXKDalGiUBCBFJMjsOHpLJZqj- 0 ) qDbOKLXKDalGiUBCBFJMjsOHpLJZqj=560167729; else qDbOKLXKDalGiUBCBFJMjsOHpLJZqj=1230881938;float gPdtswzeaNmVXQGWNlVUGbbqaKGXPT=1486724554.943701265347167259785520638903f;if (gPdtswzeaNmVXQGWNlVUGbbqaKGXPT - gPdtswzeaNmVXQGWNlVUGbbqaKGXPT> 0.00000001 ) gPdtswzeaNmVXQGWNlVUGbbqaKGXPT=628217792.330805533510335695295160352428f; else gPdtswzeaNmVXQGWNlVUGbbqaKGXPT=1546303945.437201746283738539515381838156f;if (gPdtswzeaNmVXQGWNlVUGbbqaKGXPT - gPdtswzeaNmVXQGWNlVUGbbqaKGXPT> 0.00000001 ) gPdtswzeaNmVXQGWNlVUGbbqaKGXPT=53473917.321528413495002354383492316658f; else gPdtswzeaNmVXQGWNlVUGbbqaKGXPT=1601629258.479096221012612149968498404032f;if (gPdtswzeaNmVXQGWNlVUGbbqaKGXPT - gPdtswzeaNmVXQGWNlVUGbbqaKGXPT> 0.00000001 ) gPdtswzeaNmVXQGWNlVUGbbqaKGXPT=1173588466.397613601714438188038779018260f; else gPdtswzeaNmVXQGWNlVUGbbqaKGXPT=87325672.330083511054835517043600562861f;if (gPdtswzeaNmVXQGWNlVUGbbqaKGXPT - gPdtswzeaNmVXQGWNlVUGbbqaKGXPT> 0.00000001 ) gPdtswzeaNmVXQGWNlVUGbbqaKGXPT=1173657011.545249545668818404560223883063f; else gPdtswzeaNmVXQGWNlVUGbbqaKGXPT=2005433659.185152070617358590317941143257f;if (gPdtswzeaNmVXQGWNlVUGbbqaKGXPT - gPdtswzeaNmVXQGWNlVUGbbqaKGXPT> 0.00000001 ) gPdtswzeaNmVXQGWNlVUGbbqaKGXPT=956046662.759269476753596498630485724412f; else gPdtswzeaNmVXQGWNlVUGbbqaKGXPT=1143890410.093406810022466455685562738564f;if (gPdtswzeaNmVXQGWNlVUGbbqaKGXPT - gPdtswzeaNmVXQGWNlVUGbbqaKGXPT> 0.00000001 ) gPdtswzeaNmVXQGWNlVUGbbqaKGXPT=81227694.507587119069971802038084753178f; else gPdtswzeaNmVXQGWNlVUGbbqaKGXPT=2020933905.845653609044762670203763662384f;int vXHNLVMFrJARzwYOMJEqKzjXFrKDxD=1380715366;if (vXHNLVMFrJARzwYOMJEqKzjXFrKDxD == vXHNLVMFrJARzwYOMJEqKzjXFrKDxD- 1 ) vXHNLVMFrJARzwYOMJEqKzjXFrKDxD=872759312; else vXHNLVMFrJARzwYOMJEqKzjXFrKDxD=1041466529;if (vXHNLVMFrJARzwYOMJEqKzjXFrKDxD == vXHNLVMFrJARzwYOMJEqKzjXFrKDxD- 1 ) vXHNLVMFrJARzwYOMJEqKzjXFrKDxD=1465234999; else vXHNLVMFrJARzwYOMJEqKzjXFrKDxD=1628987832;if (vXHNLVMFrJARzwYOMJEqKzjXFrKDxD == vXHNLVMFrJARzwYOMJEqKzjXFrKDxD- 0 ) vXHNLVMFrJARzwYOMJEqKzjXFrKDxD=1216334053; else vXHNLVMFrJARzwYOMJEqKzjXFrKDxD=1648523653;if (vXHNLVMFrJARzwYOMJEqKzjXFrKDxD == vXHNLVMFrJARzwYOMJEqKzjXFrKDxD- 0 ) vXHNLVMFrJARzwYOMJEqKzjXFrKDxD=737322210; else vXHNLVMFrJARzwYOMJEqKzjXFrKDxD=1441581345;if (vXHNLVMFrJARzwYOMJEqKzjXFrKDxD == vXHNLVMFrJARzwYOMJEqKzjXFrKDxD- 0 ) vXHNLVMFrJARzwYOMJEqKzjXFrKDxD=485350617; else vXHNLVMFrJARzwYOMJEqKzjXFrKDxD=819812398;if (vXHNLVMFrJARzwYOMJEqKzjXFrKDxD == vXHNLVMFrJARzwYOMJEqKzjXFrKDxD- 1 ) vXHNLVMFrJARzwYOMJEqKzjXFrKDxD=1468371401; else vXHNLVMFrJARzwYOMJEqKzjXFrKDxD=189629565;float SamiSWEQnAxeDimJmYbCczUWEDxTdG=866548450.182352035341557596197508166261f;if (SamiSWEQnAxeDimJmYbCczUWEDxTdG - SamiSWEQnAxeDimJmYbCczUWEDxTdG> 0.00000001 ) SamiSWEQnAxeDimJmYbCczUWEDxTdG=189978073.521102765423033693481499168530f; else SamiSWEQnAxeDimJmYbCczUWEDxTdG=819008375.798592145584178457558399014315f;if (SamiSWEQnAxeDimJmYbCczUWEDxTdG - SamiSWEQnAxeDimJmYbCczUWEDxTdG> 0.00000001 ) SamiSWEQnAxeDimJmYbCczUWEDxTdG=2146075085.433684665165990842271208340958f; else SamiSWEQnAxeDimJmYbCczUWEDxTdG=1667621696.984025150490250351938224434544f;if (SamiSWEQnAxeDimJmYbCczUWEDxTdG - SamiSWEQnAxeDimJmYbCczUWEDxTdG> 0.00000001 ) SamiSWEQnAxeDimJmYbCczUWEDxTdG=1565598150.571828197579389205800003177170f; else SamiSWEQnAxeDimJmYbCczUWEDxTdG=238728389.262602954326456904623984458875f;if (SamiSWEQnAxeDimJmYbCczUWEDxTdG - SamiSWEQnAxeDimJmYbCczUWEDxTdG> 0.00000001 ) SamiSWEQnAxeDimJmYbCczUWEDxTdG=1774655911.113831878735269566828465749925f; else SamiSWEQnAxeDimJmYbCczUWEDxTdG=2138983434.528931362523699972609216886920f;if (SamiSWEQnAxeDimJmYbCczUWEDxTdG - SamiSWEQnAxeDimJmYbCczUWEDxTdG> 0.00000001 ) SamiSWEQnAxeDimJmYbCczUWEDxTdG=536055485.305432743831530587698743631316f; else SamiSWEQnAxeDimJmYbCczUWEDxTdG=219688944.644233837824087988730944367280f;if (SamiSWEQnAxeDimJmYbCczUWEDxTdG - SamiSWEQnAxeDimJmYbCczUWEDxTdG> 0.00000001 ) SamiSWEQnAxeDimJmYbCczUWEDxTdG=1336942333.253839731785197586786469671107f; else SamiSWEQnAxeDimJmYbCczUWEDxTdG=81294280.766768125935806686798160499537f;long eRTzGvWMvThDpaHefOAsYSGtgzcOmu=132635227;if (eRTzGvWMvThDpaHefOAsYSGtgzcOmu == eRTzGvWMvThDpaHefOAsYSGtgzcOmu- 1 ) eRTzGvWMvThDpaHefOAsYSGtgzcOmu=1886359508; else eRTzGvWMvThDpaHefOAsYSGtgzcOmu=1326936010;if (eRTzGvWMvThDpaHefOAsYSGtgzcOmu == eRTzGvWMvThDpaHefOAsYSGtgzcOmu- 0 ) eRTzGvWMvThDpaHefOAsYSGtgzcOmu=358122626; else eRTzGvWMvThDpaHefOAsYSGtgzcOmu=1617351238;if (eRTzGvWMvThDpaHefOAsYSGtgzcOmu == eRTzGvWMvThDpaHefOAsYSGtgzcOmu- 0 ) eRTzGvWMvThDpaHefOAsYSGtgzcOmu=811877152; else eRTzGvWMvThDpaHefOAsYSGtgzcOmu=1147015595;if (eRTzGvWMvThDpaHefOAsYSGtgzcOmu == eRTzGvWMvThDpaHefOAsYSGtgzcOmu- 0 ) eRTzGvWMvThDpaHefOAsYSGtgzcOmu=270390501; else eRTzGvWMvThDpaHefOAsYSGtgzcOmu=1127177786;if (eRTzGvWMvThDpaHefOAsYSGtgzcOmu == eRTzGvWMvThDpaHefOAsYSGtgzcOmu- 1 ) eRTzGvWMvThDpaHefOAsYSGtgzcOmu=808981735; else eRTzGvWMvThDpaHefOAsYSGtgzcOmu=759376219;if (eRTzGvWMvThDpaHefOAsYSGtgzcOmu == eRTzGvWMvThDpaHefOAsYSGtgzcOmu- 1 ) eRTzGvWMvThDpaHefOAsYSGtgzcOmu=1116278641; else eRTzGvWMvThDpaHefOAsYSGtgzcOmu=353776068;int siJjGpQEyxWjRLPiYenMXkTdCcXews=236586120;if (siJjGpQEyxWjRLPiYenMXkTdCcXews == siJjGpQEyxWjRLPiYenMXkTdCcXews- 0 ) siJjGpQEyxWjRLPiYenMXkTdCcXews=96050923; else siJjGpQEyxWjRLPiYenMXkTdCcXews=1362162891;if (siJjGpQEyxWjRLPiYenMXkTdCcXews == siJjGpQEyxWjRLPiYenMXkTdCcXews- 1 ) siJjGpQEyxWjRLPiYenMXkTdCcXews=1830955832; else siJjGpQEyxWjRLPiYenMXkTdCcXews=26663671;if (siJjGpQEyxWjRLPiYenMXkTdCcXews == siJjGpQEyxWjRLPiYenMXkTdCcXews- 0 ) siJjGpQEyxWjRLPiYenMXkTdCcXews=702227082; else siJjGpQEyxWjRLPiYenMXkTdCcXews=1120735264;if (siJjGpQEyxWjRLPiYenMXkTdCcXews == siJjGpQEyxWjRLPiYenMXkTdCcXews- 1 ) siJjGpQEyxWjRLPiYenMXkTdCcXews=1962600863; else siJjGpQEyxWjRLPiYenMXkTdCcXews=634030923;if (siJjGpQEyxWjRLPiYenMXkTdCcXews == siJjGpQEyxWjRLPiYenMXkTdCcXews- 0 ) siJjGpQEyxWjRLPiYenMXkTdCcXews=1414294265; else siJjGpQEyxWjRLPiYenMXkTdCcXews=1473735212;if (siJjGpQEyxWjRLPiYenMXkTdCcXews == siJjGpQEyxWjRLPiYenMXkTdCcXews- 1 ) siJjGpQEyxWjRLPiYenMXkTdCcXews=1092932347; else siJjGpQEyxWjRLPiYenMXkTdCcXews=996645109;int GaRVeBeFircIAcxVFyHjQuuOrWPdam=1762420842;if (GaRVeBeFircIAcxVFyHjQuuOrWPdam == GaRVeBeFircIAcxVFyHjQuuOrWPdam- 0 ) GaRVeBeFircIAcxVFyHjQuuOrWPdam=917351982; else GaRVeBeFircIAcxVFyHjQuuOrWPdam=1335736174;if (GaRVeBeFircIAcxVFyHjQuuOrWPdam == GaRVeBeFircIAcxVFyHjQuuOrWPdam- 0 ) GaRVeBeFircIAcxVFyHjQuuOrWPdam=450753704; else GaRVeBeFircIAcxVFyHjQuuOrWPdam=1685011902;if (GaRVeBeFircIAcxVFyHjQuuOrWPdam == GaRVeBeFircIAcxVFyHjQuuOrWPdam- 0 ) GaRVeBeFircIAcxVFyHjQuuOrWPdam=361337916; else GaRVeBeFircIAcxVFyHjQuuOrWPdam=1577971896;if (GaRVeBeFircIAcxVFyHjQuuOrWPdam == GaRVeBeFircIAcxVFyHjQuuOrWPdam- 1 ) GaRVeBeFircIAcxVFyHjQuuOrWPdam=54671298; else GaRVeBeFircIAcxVFyHjQuuOrWPdam=1190446125;if (GaRVeBeFircIAcxVFyHjQuuOrWPdam == GaRVeBeFircIAcxVFyHjQuuOrWPdam- 1 ) GaRVeBeFircIAcxVFyHjQuuOrWPdam=548617874; else GaRVeBeFircIAcxVFyHjQuuOrWPdam=1018897299;if (GaRVeBeFircIAcxVFyHjQuuOrWPdam == GaRVeBeFircIAcxVFyHjQuuOrWPdam- 1 ) GaRVeBeFircIAcxVFyHjQuuOrWPdam=858639961; else GaRVeBeFircIAcxVFyHjQuuOrWPdam=806221931;float lfPGZRnqXPlxCKgnRvzlXkVFINdQLW=1699511686.649369423149303300904084508944f;if (lfPGZRnqXPlxCKgnRvzlXkVFINdQLW - lfPGZRnqXPlxCKgnRvzlXkVFINdQLW> 0.00000001 ) lfPGZRnqXPlxCKgnRvzlXkVFINdQLW=1226203197.355312062472805122955202267596f; else lfPGZRnqXPlxCKgnRvzlXkVFINdQLW=1100356213.169099500134963228048600916325f;if (lfPGZRnqXPlxCKgnRvzlXkVFINdQLW - lfPGZRnqXPlxCKgnRvzlXkVFINdQLW> 0.00000001 ) lfPGZRnqXPlxCKgnRvzlXkVFINdQLW=2132727004.568632492673392144005055966018f; else lfPGZRnqXPlxCKgnRvzlXkVFINdQLW=909904375.428668734184648532888242161487f;if (lfPGZRnqXPlxCKgnRvzlXkVFINdQLW - lfPGZRnqXPlxCKgnRvzlXkVFINdQLW> 0.00000001 ) lfPGZRnqXPlxCKgnRvzlXkVFINdQLW=1186941237.758526583451103142743523039751f; else lfPGZRnqXPlxCKgnRvzlXkVFINdQLW=2023280392.845754612316054715002075423550f;if (lfPGZRnqXPlxCKgnRvzlXkVFINdQLW - lfPGZRnqXPlxCKgnRvzlXkVFINdQLW> 0.00000001 ) lfPGZRnqXPlxCKgnRvzlXkVFINdQLW=1577891792.793372755964053768043576987133f; else lfPGZRnqXPlxCKgnRvzlXkVFINdQLW=765624244.217363478316024903276662732749f;if (lfPGZRnqXPlxCKgnRvzlXkVFINdQLW - lfPGZRnqXPlxCKgnRvzlXkVFINdQLW> 0.00000001 ) lfPGZRnqXPlxCKgnRvzlXkVFINdQLW=1511818794.562276889149084756309506866563f; else lfPGZRnqXPlxCKgnRvzlXkVFINdQLW=1871575350.274986585923414869919655117651f;if (lfPGZRnqXPlxCKgnRvzlXkVFINdQLW - lfPGZRnqXPlxCKgnRvzlXkVFINdQLW> 0.00000001 ) lfPGZRnqXPlxCKgnRvzlXkVFINdQLW=203056973.132707610837148619311776563998f; else lfPGZRnqXPlxCKgnRvzlXkVFINdQLW=1485625587.094490282726919252760845146724f;long kNLuTPkZqWTigsZqgkMFEIPpHZvQbT=152953805;if (kNLuTPkZqWTigsZqgkMFEIPpHZvQbT == kNLuTPkZqWTigsZqgkMFEIPpHZvQbT- 0 ) kNLuTPkZqWTigsZqgkMFEIPpHZvQbT=1589862140; else kNLuTPkZqWTigsZqgkMFEIPpHZvQbT=1501341446;if (kNLuTPkZqWTigsZqgkMFEIPpHZvQbT == kNLuTPkZqWTigsZqgkMFEIPpHZvQbT- 1 ) kNLuTPkZqWTigsZqgkMFEIPpHZvQbT=1583907733; else kNLuTPkZqWTigsZqgkMFEIPpHZvQbT=1578185391;if (kNLuTPkZqWTigsZqgkMFEIPpHZvQbT == kNLuTPkZqWTigsZqgkMFEIPpHZvQbT- 1 ) kNLuTPkZqWTigsZqgkMFEIPpHZvQbT=533273742; else kNLuTPkZqWTigsZqgkMFEIPpHZvQbT=1603441139;if (kNLuTPkZqWTigsZqgkMFEIPpHZvQbT == kNLuTPkZqWTigsZqgkMFEIPpHZvQbT- 0 ) kNLuTPkZqWTigsZqgkMFEIPpHZvQbT=2022731851; else kNLuTPkZqWTigsZqgkMFEIPpHZvQbT=665562674;if (kNLuTPkZqWTigsZqgkMFEIPpHZvQbT == kNLuTPkZqWTigsZqgkMFEIPpHZvQbT- 1 ) kNLuTPkZqWTigsZqgkMFEIPpHZvQbT=1307491593; else kNLuTPkZqWTigsZqgkMFEIPpHZvQbT=640509707;if (kNLuTPkZqWTigsZqgkMFEIPpHZvQbT == kNLuTPkZqWTigsZqgkMFEIPpHZvQbT- 1 ) kNLuTPkZqWTigsZqgkMFEIPpHZvQbT=1625529444; else kNLuTPkZqWTigsZqgkMFEIPpHZvQbT=1830813430;float rWpvWXVSYGSgsHetoQrzzccBcdWDVV=2140898069.943246569917576108816645226042f;if (rWpvWXVSYGSgsHetoQrzzccBcdWDVV - rWpvWXVSYGSgsHetoQrzzccBcdWDVV> 0.00000001 ) rWpvWXVSYGSgsHetoQrzzccBcdWDVV=101189696.619140988239800514847750541903f; else rWpvWXVSYGSgsHetoQrzzccBcdWDVV=1540904308.152830953733169894571230607061f;if (rWpvWXVSYGSgsHetoQrzzccBcdWDVV - rWpvWXVSYGSgsHetoQrzzccBcdWDVV> 0.00000001 ) rWpvWXVSYGSgsHetoQrzzccBcdWDVV=1917925559.008518717077222469285743303324f; else rWpvWXVSYGSgsHetoQrzzccBcdWDVV=1358708754.499373565118761501924898637532f;if (rWpvWXVSYGSgsHetoQrzzccBcdWDVV - rWpvWXVSYGSgsHetoQrzzccBcdWDVV> 0.00000001 ) rWpvWXVSYGSgsHetoQrzzccBcdWDVV=670784573.559478895001418809380340161184f; else rWpvWXVSYGSgsHetoQrzzccBcdWDVV=407093351.704467949372603448194835231041f;if (rWpvWXVSYGSgsHetoQrzzccBcdWDVV - rWpvWXVSYGSgsHetoQrzzccBcdWDVV> 0.00000001 ) rWpvWXVSYGSgsHetoQrzzccBcdWDVV=575479605.872240815197011985882400198934f; else rWpvWXVSYGSgsHetoQrzzccBcdWDVV=1041555237.601961157464310882536627948215f;if (rWpvWXVSYGSgsHetoQrzzccBcdWDVV - rWpvWXVSYGSgsHetoQrzzccBcdWDVV> 0.00000001 ) rWpvWXVSYGSgsHetoQrzzccBcdWDVV=1905328029.208259626193942910032640819412f; else rWpvWXVSYGSgsHetoQrzzccBcdWDVV=1832613107.394327716227745389504838376855f;if (rWpvWXVSYGSgsHetoQrzzccBcdWDVV - rWpvWXVSYGSgsHetoQrzzccBcdWDVV> 0.00000001 ) rWpvWXVSYGSgsHetoQrzzccBcdWDVV=173193108.121480072530582601653091967606f; else rWpvWXVSYGSgsHetoQrzzccBcdWDVV=637246213.297120636542718445777530313163f;long UhhPVRSgJAjYHIGTsOaIKvPtjejrHi=1274872482;if (UhhPVRSgJAjYHIGTsOaIKvPtjejrHi == UhhPVRSgJAjYHIGTsOaIKvPtjejrHi- 0 ) UhhPVRSgJAjYHIGTsOaIKvPtjejrHi=203300833; else UhhPVRSgJAjYHIGTsOaIKvPtjejrHi=705686156;if (UhhPVRSgJAjYHIGTsOaIKvPtjejrHi == UhhPVRSgJAjYHIGTsOaIKvPtjejrHi- 1 ) UhhPVRSgJAjYHIGTsOaIKvPtjejrHi=1363843438; else UhhPVRSgJAjYHIGTsOaIKvPtjejrHi=1916075596;if (UhhPVRSgJAjYHIGTsOaIKvPtjejrHi == UhhPVRSgJAjYHIGTsOaIKvPtjejrHi- 1 ) UhhPVRSgJAjYHIGTsOaIKvPtjejrHi=1984455757; else UhhPVRSgJAjYHIGTsOaIKvPtjejrHi=1941843772;if (UhhPVRSgJAjYHIGTsOaIKvPtjejrHi == UhhPVRSgJAjYHIGTsOaIKvPtjejrHi- 1 ) UhhPVRSgJAjYHIGTsOaIKvPtjejrHi=1166237944; else UhhPVRSgJAjYHIGTsOaIKvPtjejrHi=1770245705;if (UhhPVRSgJAjYHIGTsOaIKvPtjejrHi == UhhPVRSgJAjYHIGTsOaIKvPtjejrHi- 0 ) UhhPVRSgJAjYHIGTsOaIKvPtjejrHi=1225108009; else UhhPVRSgJAjYHIGTsOaIKvPtjejrHi=1467164330;if (UhhPVRSgJAjYHIGTsOaIKvPtjejrHi == UhhPVRSgJAjYHIGTsOaIKvPtjejrHi- 0 ) UhhPVRSgJAjYHIGTsOaIKvPtjejrHi=1749896429; else UhhPVRSgJAjYHIGTsOaIKvPtjejrHi=546431530;int HJDYBRfLoXXyXvHbPeUAeSvPIADdrK=355439862;if (HJDYBRfLoXXyXvHbPeUAeSvPIADdrK == HJDYBRfLoXXyXvHbPeUAeSvPIADdrK- 0 ) HJDYBRfLoXXyXvHbPeUAeSvPIADdrK=359994175; else HJDYBRfLoXXyXvHbPeUAeSvPIADdrK=209642551;if (HJDYBRfLoXXyXvHbPeUAeSvPIADdrK == HJDYBRfLoXXyXvHbPeUAeSvPIADdrK- 0 ) HJDYBRfLoXXyXvHbPeUAeSvPIADdrK=1384435891; else HJDYBRfLoXXyXvHbPeUAeSvPIADdrK=660662587;if (HJDYBRfLoXXyXvHbPeUAeSvPIADdrK == HJDYBRfLoXXyXvHbPeUAeSvPIADdrK- 1 ) HJDYBRfLoXXyXvHbPeUAeSvPIADdrK=1650787127; else HJDYBRfLoXXyXvHbPeUAeSvPIADdrK=1604371359;if (HJDYBRfLoXXyXvHbPeUAeSvPIADdrK == HJDYBRfLoXXyXvHbPeUAeSvPIADdrK- 1 ) HJDYBRfLoXXyXvHbPeUAeSvPIADdrK=1629652879; else HJDYBRfLoXXyXvHbPeUAeSvPIADdrK=1182768789;if (HJDYBRfLoXXyXvHbPeUAeSvPIADdrK == HJDYBRfLoXXyXvHbPeUAeSvPIADdrK- 1 ) HJDYBRfLoXXyXvHbPeUAeSvPIADdrK=925861841; else HJDYBRfLoXXyXvHbPeUAeSvPIADdrK=542352496;if (HJDYBRfLoXXyXvHbPeUAeSvPIADdrK == HJDYBRfLoXXyXvHbPeUAeSvPIADdrK- 1 ) HJDYBRfLoXXyXvHbPeUAeSvPIADdrK=1820341009; else HJDYBRfLoXXyXvHbPeUAeSvPIADdrK=848144282;int uoFcExknOCajGEegVHUtQZzkEsToIi=590962235;if (uoFcExknOCajGEegVHUtQZzkEsToIi == uoFcExknOCajGEegVHUtQZzkEsToIi- 1 ) uoFcExknOCajGEegVHUtQZzkEsToIi=1651630477; else uoFcExknOCajGEegVHUtQZzkEsToIi=350656962;if (uoFcExknOCajGEegVHUtQZzkEsToIi == uoFcExknOCajGEegVHUtQZzkEsToIi- 0 ) uoFcExknOCajGEegVHUtQZzkEsToIi=1627512597; else uoFcExknOCajGEegVHUtQZzkEsToIi=1703892794;if (uoFcExknOCajGEegVHUtQZzkEsToIi == uoFcExknOCajGEegVHUtQZzkEsToIi- 0 ) uoFcExknOCajGEegVHUtQZzkEsToIi=1940019141; else uoFcExknOCajGEegVHUtQZzkEsToIi=1742989226;if (uoFcExknOCajGEegVHUtQZzkEsToIi == uoFcExknOCajGEegVHUtQZzkEsToIi- 1 ) uoFcExknOCajGEegVHUtQZzkEsToIi=156442312; else uoFcExknOCajGEegVHUtQZzkEsToIi=306829571;if (uoFcExknOCajGEegVHUtQZzkEsToIi == uoFcExknOCajGEegVHUtQZzkEsToIi- 0 ) uoFcExknOCajGEegVHUtQZzkEsToIi=374666364; else uoFcExknOCajGEegVHUtQZzkEsToIi=147679854;if (uoFcExknOCajGEegVHUtQZzkEsToIi == uoFcExknOCajGEegVHUtQZzkEsToIi- 0 ) uoFcExknOCajGEegVHUtQZzkEsToIi=133600745; else uoFcExknOCajGEegVHUtQZzkEsToIi=1351103890;double FhCyesuLjNCrJVxqNGADFWBnUvLreh=10766020.434684043080054303852515235278;if (FhCyesuLjNCrJVxqNGADFWBnUvLreh == FhCyesuLjNCrJVxqNGADFWBnUvLreh ) FhCyesuLjNCrJVxqNGADFWBnUvLreh=1329084166.005889640568313807926381992955; else FhCyesuLjNCrJVxqNGADFWBnUvLreh=1965236893.987519527687430421343945382509;if (FhCyesuLjNCrJVxqNGADFWBnUvLreh == FhCyesuLjNCrJVxqNGADFWBnUvLreh ) FhCyesuLjNCrJVxqNGADFWBnUvLreh=277252038.779568190439045095158272215324; else FhCyesuLjNCrJVxqNGADFWBnUvLreh=1165272372.703891897243191590049845261846;if (FhCyesuLjNCrJVxqNGADFWBnUvLreh == FhCyesuLjNCrJVxqNGADFWBnUvLreh ) FhCyesuLjNCrJVxqNGADFWBnUvLreh=1479577114.498774736521802628183539506873; else FhCyesuLjNCrJVxqNGADFWBnUvLreh=475700290.006286652896553905465387306038;if (FhCyesuLjNCrJVxqNGADFWBnUvLreh == FhCyesuLjNCrJVxqNGADFWBnUvLreh ) FhCyesuLjNCrJVxqNGADFWBnUvLreh=1771569002.896362562608639718660251973009; else FhCyesuLjNCrJVxqNGADFWBnUvLreh=746413121.073511120145387876879847864649;if (FhCyesuLjNCrJVxqNGADFWBnUvLreh == FhCyesuLjNCrJVxqNGADFWBnUvLreh ) FhCyesuLjNCrJVxqNGADFWBnUvLreh=1223516924.067374622979862698521734024676; else FhCyesuLjNCrJVxqNGADFWBnUvLreh=181074649.759219801326490118537622933445;if (FhCyesuLjNCrJVxqNGADFWBnUvLreh == FhCyesuLjNCrJVxqNGADFWBnUvLreh ) FhCyesuLjNCrJVxqNGADFWBnUvLreh=990213755.005733176489720534402724923748; else FhCyesuLjNCrJVxqNGADFWBnUvLreh=1442103261.657733187815316891240005348291;double RNkmNMBrkSangsnTEoBNMZzpjYLUuc=1987106018.960636988981809925681798654645;if (RNkmNMBrkSangsnTEoBNMZzpjYLUuc == RNkmNMBrkSangsnTEoBNMZzpjYLUuc ) RNkmNMBrkSangsnTEoBNMZzpjYLUuc=29032932.940057199854585707877735828308; else RNkmNMBrkSangsnTEoBNMZzpjYLUuc=1158934194.344414295383683721395898831623;if (RNkmNMBrkSangsnTEoBNMZzpjYLUuc == RNkmNMBrkSangsnTEoBNMZzpjYLUuc ) RNkmNMBrkSangsnTEoBNMZzpjYLUuc=1042284700.999262118152259516211802355679; else RNkmNMBrkSangsnTEoBNMZzpjYLUuc=47093986.478250453272425464942641329891;if (RNkmNMBrkSangsnTEoBNMZzpjYLUuc == RNkmNMBrkSangsnTEoBNMZzpjYLUuc ) RNkmNMBrkSangsnTEoBNMZzpjYLUuc=267049412.755988501016305949790283793703; else RNkmNMBrkSangsnTEoBNMZzpjYLUuc=947119794.167252754706403634648110268330;if (RNkmNMBrkSangsnTEoBNMZzpjYLUuc == RNkmNMBrkSangsnTEoBNMZzpjYLUuc ) RNkmNMBrkSangsnTEoBNMZzpjYLUuc=562904368.681906016430510498026016505719; else RNkmNMBrkSangsnTEoBNMZzpjYLUuc=764488596.453522551872932971188031477640;if (RNkmNMBrkSangsnTEoBNMZzpjYLUuc == RNkmNMBrkSangsnTEoBNMZzpjYLUuc ) RNkmNMBrkSangsnTEoBNMZzpjYLUuc=53200239.140809700804826519564574242226; else RNkmNMBrkSangsnTEoBNMZzpjYLUuc=1077606320.602183223337139531606829768767;if (RNkmNMBrkSangsnTEoBNMZzpjYLUuc == RNkmNMBrkSangsnTEoBNMZzpjYLUuc ) RNkmNMBrkSangsnTEoBNMZzpjYLUuc=285996223.869530275330724754672162089452; else RNkmNMBrkSangsnTEoBNMZzpjYLUuc=1503107273.666767961936748620502410570855;float tNOjhVCeissRlWfZJVUgvtXAJcgHxm=896813766.182242861529806313649255920390f;if (tNOjhVCeissRlWfZJVUgvtXAJcgHxm - tNOjhVCeissRlWfZJVUgvtXAJcgHxm> 0.00000001 ) tNOjhVCeissRlWfZJVUgvtXAJcgHxm=2046031854.529806065364602419845737393832f; else tNOjhVCeissRlWfZJVUgvtXAJcgHxm=1809574989.257210001135849575362165689361f;if (tNOjhVCeissRlWfZJVUgvtXAJcgHxm - tNOjhVCeissRlWfZJVUgvtXAJcgHxm> 0.00000001 ) tNOjhVCeissRlWfZJVUgvtXAJcgHxm=1666543763.324411850242982000893857120273f; else tNOjhVCeissRlWfZJVUgvtXAJcgHxm=1612494785.184191253983623038479058957830f;if (tNOjhVCeissRlWfZJVUgvtXAJcgHxm - tNOjhVCeissRlWfZJVUgvtXAJcgHxm> 0.00000001 ) tNOjhVCeissRlWfZJVUgvtXAJcgHxm=1108233600.168817889195701134510281423734f; else tNOjhVCeissRlWfZJVUgvtXAJcgHxm=272859026.542483581625048435792432198850f;if (tNOjhVCeissRlWfZJVUgvtXAJcgHxm - tNOjhVCeissRlWfZJVUgvtXAJcgHxm> 0.00000001 ) tNOjhVCeissRlWfZJVUgvtXAJcgHxm=1258868768.416675443966390535309914211139f; else tNOjhVCeissRlWfZJVUgvtXAJcgHxm=1175930187.573221048419383715367465161246f;if (tNOjhVCeissRlWfZJVUgvtXAJcgHxm - tNOjhVCeissRlWfZJVUgvtXAJcgHxm> 0.00000001 ) tNOjhVCeissRlWfZJVUgvtXAJcgHxm=726571607.441459318969790164260788322927f; else tNOjhVCeissRlWfZJVUgvtXAJcgHxm=881099476.181318994441851877610818676423f;if (tNOjhVCeissRlWfZJVUgvtXAJcgHxm - tNOjhVCeissRlWfZJVUgvtXAJcgHxm> 0.00000001 ) tNOjhVCeissRlWfZJVUgvtXAJcgHxm=480375870.850188959228183003765532747474f; else tNOjhVCeissRlWfZJVUgvtXAJcgHxm=1758944492.258179277393949374677386833845f;int anZBmzAFQsRgRhPhsQZhQnDitkHMTi=56572702;if (anZBmzAFQsRgRhPhsQZhQnDitkHMTi == anZBmzAFQsRgRhPhsQZhQnDitkHMTi- 1 ) anZBmzAFQsRgRhPhsQZhQnDitkHMTi=1955165827; else anZBmzAFQsRgRhPhsQZhQnDitkHMTi=293978374;if (anZBmzAFQsRgRhPhsQZhQnDitkHMTi == anZBmzAFQsRgRhPhsQZhQnDitkHMTi- 1 ) anZBmzAFQsRgRhPhsQZhQnDitkHMTi=1322070958; else anZBmzAFQsRgRhPhsQZhQnDitkHMTi=1793230022;if (anZBmzAFQsRgRhPhsQZhQnDitkHMTi == anZBmzAFQsRgRhPhsQZhQnDitkHMTi- 1 ) anZBmzAFQsRgRhPhsQZhQnDitkHMTi=1134247022; else anZBmzAFQsRgRhPhsQZhQnDitkHMTi=596710303;if (anZBmzAFQsRgRhPhsQZhQnDitkHMTi == anZBmzAFQsRgRhPhsQZhQnDitkHMTi- 0 ) anZBmzAFQsRgRhPhsQZhQnDitkHMTi=1891200255; else anZBmzAFQsRgRhPhsQZhQnDitkHMTi=381964372;if (anZBmzAFQsRgRhPhsQZhQnDitkHMTi == anZBmzAFQsRgRhPhsQZhQnDitkHMTi- 1 ) anZBmzAFQsRgRhPhsQZhQnDitkHMTi=1402332525; else anZBmzAFQsRgRhPhsQZhQnDitkHMTi=1660247089;if (anZBmzAFQsRgRhPhsQZhQnDitkHMTi == anZBmzAFQsRgRhPhsQZhQnDitkHMTi- 0 ) anZBmzAFQsRgRhPhsQZhQnDitkHMTi=1112072133; else anZBmzAFQsRgRhPhsQZhQnDitkHMTi=401970794;double DZHbbuBOchDDlqlTEbbHZoDbEhWdzU=944967935.283222494235082574206863653075;if (DZHbbuBOchDDlqlTEbbHZoDbEhWdzU == DZHbbuBOchDDlqlTEbbHZoDbEhWdzU ) DZHbbuBOchDDlqlTEbbHZoDbEhWdzU=1950470076.024346357050061828503734853077; else DZHbbuBOchDDlqlTEbbHZoDbEhWdzU=93399989.052022284623825691284568230150;if (DZHbbuBOchDDlqlTEbbHZoDbEhWdzU == DZHbbuBOchDDlqlTEbbHZoDbEhWdzU ) DZHbbuBOchDDlqlTEbbHZoDbEhWdzU=1543555054.438011804074508101936389450589; else DZHbbuBOchDDlqlTEbbHZoDbEhWdzU=459411421.323420592589732016024923525067;if (DZHbbuBOchDDlqlTEbbHZoDbEhWdzU == DZHbbuBOchDDlqlTEbbHZoDbEhWdzU ) DZHbbuBOchDDlqlTEbbHZoDbEhWdzU=799033546.133586219972055766854467640000; else DZHbbuBOchDDlqlTEbbHZoDbEhWdzU=874986536.563645735680826308996022378704;if (DZHbbuBOchDDlqlTEbbHZoDbEhWdzU == DZHbbuBOchDDlqlTEbbHZoDbEhWdzU ) DZHbbuBOchDDlqlTEbbHZoDbEhWdzU=878872418.622951951004237574135321565237; else DZHbbuBOchDDlqlTEbbHZoDbEhWdzU=1903657553.713117908555709380591567041140;if (DZHbbuBOchDDlqlTEbbHZoDbEhWdzU == DZHbbuBOchDDlqlTEbbHZoDbEhWdzU ) DZHbbuBOchDDlqlTEbbHZoDbEhWdzU=2047549073.782430112167779063328575636618; else DZHbbuBOchDDlqlTEbbHZoDbEhWdzU=2013838160.686987447027853589908992232880;if (DZHbbuBOchDDlqlTEbbHZoDbEhWdzU == DZHbbuBOchDDlqlTEbbHZoDbEhWdzU ) DZHbbuBOchDDlqlTEbbHZoDbEhWdzU=1468006026.450558190273631182225782080476; else DZHbbuBOchDDlqlTEbbHZoDbEhWdzU=1533433583.676768841540974636225067578957;double kLWfBjtiPQWGkrdpDoWbVLXDfFwLuA=1338861188.217480791375414114960861065237;if (kLWfBjtiPQWGkrdpDoWbVLXDfFwLuA == kLWfBjtiPQWGkrdpDoWbVLXDfFwLuA ) kLWfBjtiPQWGkrdpDoWbVLXDfFwLuA=892279512.571922926344608670870875346752; else kLWfBjtiPQWGkrdpDoWbVLXDfFwLuA=229423521.666480560329963636340547881668;if (kLWfBjtiPQWGkrdpDoWbVLXDfFwLuA == kLWfBjtiPQWGkrdpDoWbVLXDfFwLuA ) kLWfBjtiPQWGkrdpDoWbVLXDfFwLuA=1695425093.738488923921165101281041212070; else kLWfBjtiPQWGkrdpDoWbVLXDfFwLuA=1176792240.990269259582453959279059880727;if (kLWfBjtiPQWGkrdpDoWbVLXDfFwLuA == kLWfBjtiPQWGkrdpDoWbVLXDfFwLuA ) kLWfBjtiPQWGkrdpDoWbVLXDfFwLuA=1103961678.706313291605371060947381295258; else kLWfBjtiPQWGkrdpDoWbVLXDfFwLuA=732341845.053399624086977488883199142612;if (kLWfBjtiPQWGkrdpDoWbVLXDfFwLuA == kLWfBjtiPQWGkrdpDoWbVLXDfFwLuA ) kLWfBjtiPQWGkrdpDoWbVLXDfFwLuA=1910050391.905376319355696152482847606425; else kLWfBjtiPQWGkrdpDoWbVLXDfFwLuA=1449321551.701329160227964951409133016877;if (kLWfBjtiPQWGkrdpDoWbVLXDfFwLuA == kLWfBjtiPQWGkrdpDoWbVLXDfFwLuA ) kLWfBjtiPQWGkrdpDoWbVLXDfFwLuA=2065858381.099575437800237599485613668831; else kLWfBjtiPQWGkrdpDoWbVLXDfFwLuA=1284579391.340324950952470000763849348791;if (kLWfBjtiPQWGkrdpDoWbVLXDfFwLuA == kLWfBjtiPQWGkrdpDoWbVLXDfFwLuA ) kLWfBjtiPQWGkrdpDoWbVLXDfFwLuA=210162260.761657091475964434366928960119; else kLWfBjtiPQWGkrdpDoWbVLXDfFwLuA=1595470158.626929283411030899692553595380;double toKsWqARYIZJsbatqrEjFeIkFoPVRT=458109878.789113132677786215562794827190;if (toKsWqARYIZJsbatqrEjFeIkFoPVRT == toKsWqARYIZJsbatqrEjFeIkFoPVRT ) toKsWqARYIZJsbatqrEjFeIkFoPVRT=1605456969.436063721648145505637507400539; else toKsWqARYIZJsbatqrEjFeIkFoPVRT=1682891582.705225173350503575932256394977;if (toKsWqARYIZJsbatqrEjFeIkFoPVRT == toKsWqARYIZJsbatqrEjFeIkFoPVRT ) toKsWqARYIZJsbatqrEjFeIkFoPVRT=1955958063.439204492144338811503561755903; else toKsWqARYIZJsbatqrEjFeIkFoPVRT=659375482.231703130928499992969458497855;if (toKsWqARYIZJsbatqrEjFeIkFoPVRT == toKsWqARYIZJsbatqrEjFeIkFoPVRT ) toKsWqARYIZJsbatqrEjFeIkFoPVRT=1465751291.950902374678179628397415598431; else toKsWqARYIZJsbatqrEjFeIkFoPVRT=1873399034.748507850762949139088967965652;if (toKsWqARYIZJsbatqrEjFeIkFoPVRT == toKsWqARYIZJsbatqrEjFeIkFoPVRT ) toKsWqARYIZJsbatqrEjFeIkFoPVRT=1405022803.404494067287963558756056388175; else toKsWqARYIZJsbatqrEjFeIkFoPVRT=1080179291.331837965517280210551700646912;if (toKsWqARYIZJsbatqrEjFeIkFoPVRT == toKsWqARYIZJsbatqrEjFeIkFoPVRT ) toKsWqARYIZJsbatqrEjFeIkFoPVRT=1562589603.690296714660424987727972463139; else toKsWqARYIZJsbatqrEjFeIkFoPVRT=1565897052.105393819461799248776930583884;if (toKsWqARYIZJsbatqrEjFeIkFoPVRT == toKsWqARYIZJsbatqrEjFeIkFoPVRT ) toKsWqARYIZJsbatqrEjFeIkFoPVRT=1893164596.369567513228836738433546328692; else toKsWqARYIZJsbatqrEjFeIkFoPVRT=1267892509.013610324374666920495546192324;double tMEHHaKIfVEWGfwxpCXEnNzlZgmaNq=450889002.215769712454807839793657117483;if (tMEHHaKIfVEWGfwxpCXEnNzlZgmaNq == tMEHHaKIfVEWGfwxpCXEnNzlZgmaNq ) tMEHHaKIfVEWGfwxpCXEnNzlZgmaNq=1648533964.359349165599377746465226766876; else tMEHHaKIfVEWGfwxpCXEnNzlZgmaNq=1920694592.344911667987520539147102923368;if (tMEHHaKIfVEWGfwxpCXEnNzlZgmaNq == tMEHHaKIfVEWGfwxpCXEnNzlZgmaNq ) tMEHHaKIfVEWGfwxpCXEnNzlZgmaNq=1657174929.430936656619069981734773024310; else tMEHHaKIfVEWGfwxpCXEnNzlZgmaNq=2107764193.830055760409453825304374577701;if (tMEHHaKIfVEWGfwxpCXEnNzlZgmaNq == tMEHHaKIfVEWGfwxpCXEnNzlZgmaNq ) tMEHHaKIfVEWGfwxpCXEnNzlZgmaNq=720520283.334799012523133543021901656758; else tMEHHaKIfVEWGfwxpCXEnNzlZgmaNq=246333914.906917587592499234114589144414;if (tMEHHaKIfVEWGfwxpCXEnNzlZgmaNq == tMEHHaKIfVEWGfwxpCXEnNzlZgmaNq ) tMEHHaKIfVEWGfwxpCXEnNzlZgmaNq=1988489904.346472723707663962343365543897; else tMEHHaKIfVEWGfwxpCXEnNzlZgmaNq=1218128231.427326602751251793628327118293;if (tMEHHaKIfVEWGfwxpCXEnNzlZgmaNq == tMEHHaKIfVEWGfwxpCXEnNzlZgmaNq ) tMEHHaKIfVEWGfwxpCXEnNzlZgmaNq=330833245.004648034846069312486138911132; else tMEHHaKIfVEWGfwxpCXEnNzlZgmaNq=94233503.952086861386999336890781897673;if (tMEHHaKIfVEWGfwxpCXEnNzlZgmaNq == tMEHHaKIfVEWGfwxpCXEnNzlZgmaNq ) tMEHHaKIfVEWGfwxpCXEnNzlZgmaNq=541036687.801399518365121024079535228139; else tMEHHaKIfVEWGfwxpCXEnNzlZgmaNq=1661937802.020968670083954666187370548728;double VqvUTdLgMVbYzvegmFfhIyXJgXvWJC=1190651099.280750986700122859702219850863;if (VqvUTdLgMVbYzvegmFfhIyXJgXvWJC == VqvUTdLgMVbYzvegmFfhIyXJgXvWJC ) VqvUTdLgMVbYzvegmFfhIyXJgXvWJC=1027612040.072937223516743941882065318100; else VqvUTdLgMVbYzvegmFfhIyXJgXvWJC=1009896148.113474939674739668602918211863;if (VqvUTdLgMVbYzvegmFfhIyXJgXvWJC == VqvUTdLgMVbYzvegmFfhIyXJgXvWJC ) VqvUTdLgMVbYzvegmFfhIyXJgXvWJC=2075460261.012340447001231834675050921527; else VqvUTdLgMVbYzvegmFfhIyXJgXvWJC=931335989.328776249933270188748839807343;if (VqvUTdLgMVbYzvegmFfhIyXJgXvWJC == VqvUTdLgMVbYzvegmFfhIyXJgXvWJC ) VqvUTdLgMVbYzvegmFfhIyXJgXvWJC=1142969107.992179466679674298195489960998; else VqvUTdLgMVbYzvegmFfhIyXJgXvWJC=1499821290.001615211272227936571936861332;if (VqvUTdLgMVbYzvegmFfhIyXJgXvWJC == VqvUTdLgMVbYzvegmFfhIyXJgXvWJC ) VqvUTdLgMVbYzvegmFfhIyXJgXvWJC=2020593544.057967907018892326204443718595; else VqvUTdLgMVbYzvegmFfhIyXJgXvWJC=1166364125.409032646014066324455513872536;if (VqvUTdLgMVbYzvegmFfhIyXJgXvWJC == VqvUTdLgMVbYzvegmFfhIyXJgXvWJC ) VqvUTdLgMVbYzvegmFfhIyXJgXvWJC=971884365.051272097089030049749414099627; else VqvUTdLgMVbYzvegmFfhIyXJgXvWJC=615245802.111117380544858759493489864838;if (VqvUTdLgMVbYzvegmFfhIyXJgXvWJC == VqvUTdLgMVbYzvegmFfhIyXJgXvWJC ) VqvUTdLgMVbYzvegmFfhIyXJgXvWJC=1761686284.348107802486021677744124483335; else VqvUTdLgMVbYzvegmFfhIyXJgXvWJC=1685548273.356230775106711746920729432573;float ZjlBOBnzBvzLfYtRFOiSkTFbgSlpyp=1708091709.715624113005233875963471191911f;if (ZjlBOBnzBvzLfYtRFOiSkTFbgSlpyp - ZjlBOBnzBvzLfYtRFOiSkTFbgSlpyp> 0.00000001 ) ZjlBOBnzBvzLfYtRFOiSkTFbgSlpyp=1369358943.616688762403310918672599035453f; else ZjlBOBnzBvzLfYtRFOiSkTFbgSlpyp=1906756905.330249966612367126239435022875f;if (ZjlBOBnzBvzLfYtRFOiSkTFbgSlpyp - ZjlBOBnzBvzLfYtRFOiSkTFbgSlpyp> 0.00000001 ) ZjlBOBnzBvzLfYtRFOiSkTFbgSlpyp=2094419841.332917417105316134202933591533f; else ZjlBOBnzBvzLfYtRFOiSkTFbgSlpyp=1535059595.810238045081023522090978341052f;if (ZjlBOBnzBvzLfYtRFOiSkTFbgSlpyp - ZjlBOBnzBvzLfYtRFOiSkTFbgSlpyp> 0.00000001 ) ZjlBOBnzBvzLfYtRFOiSkTFbgSlpyp=1347206692.799540628819923102008964495299f; else ZjlBOBnzBvzLfYtRFOiSkTFbgSlpyp=1025945574.819449358491529115535006583525f;if (ZjlBOBnzBvzLfYtRFOiSkTFbgSlpyp - ZjlBOBnzBvzLfYtRFOiSkTFbgSlpyp> 0.00000001 ) ZjlBOBnzBvzLfYtRFOiSkTFbgSlpyp=1885073242.543911069495471855639187582406f; else ZjlBOBnzBvzLfYtRFOiSkTFbgSlpyp=1359123055.784406861825625074574800747754f;if (ZjlBOBnzBvzLfYtRFOiSkTFbgSlpyp - ZjlBOBnzBvzLfYtRFOiSkTFbgSlpyp> 0.00000001 ) ZjlBOBnzBvzLfYtRFOiSkTFbgSlpyp=1841885325.077471217610582456352803209576f; else ZjlBOBnzBvzLfYtRFOiSkTFbgSlpyp=737829832.911895935487797532839150228428f;if (ZjlBOBnzBvzLfYtRFOiSkTFbgSlpyp - ZjlBOBnzBvzLfYtRFOiSkTFbgSlpyp> 0.00000001 ) ZjlBOBnzBvzLfYtRFOiSkTFbgSlpyp=328542237.284280989763298621687775787168f; else ZjlBOBnzBvzLfYtRFOiSkTFbgSlpyp=1371517788.851721362876427606361785262950f;int oIGrFNXhkOfutFCSWnFrCpzlGhcajr=131703696;if (oIGrFNXhkOfutFCSWnFrCpzlGhcajr == oIGrFNXhkOfutFCSWnFrCpzlGhcajr- 1 ) oIGrFNXhkOfutFCSWnFrCpzlGhcajr=970510039; else oIGrFNXhkOfutFCSWnFrCpzlGhcajr=702513497;if (oIGrFNXhkOfutFCSWnFrCpzlGhcajr == oIGrFNXhkOfutFCSWnFrCpzlGhcajr- 0 ) oIGrFNXhkOfutFCSWnFrCpzlGhcajr=240280469; else oIGrFNXhkOfutFCSWnFrCpzlGhcajr=266291776;if (oIGrFNXhkOfutFCSWnFrCpzlGhcajr == oIGrFNXhkOfutFCSWnFrCpzlGhcajr- 1 ) oIGrFNXhkOfutFCSWnFrCpzlGhcajr=429790058; else oIGrFNXhkOfutFCSWnFrCpzlGhcajr=1131368106;if (oIGrFNXhkOfutFCSWnFrCpzlGhcajr == oIGrFNXhkOfutFCSWnFrCpzlGhcajr- 1 ) oIGrFNXhkOfutFCSWnFrCpzlGhcajr=1455403542; else oIGrFNXhkOfutFCSWnFrCpzlGhcajr=148712674;if (oIGrFNXhkOfutFCSWnFrCpzlGhcajr == oIGrFNXhkOfutFCSWnFrCpzlGhcajr- 0 ) oIGrFNXhkOfutFCSWnFrCpzlGhcajr=2047584695; else oIGrFNXhkOfutFCSWnFrCpzlGhcajr=490810804;if (oIGrFNXhkOfutFCSWnFrCpzlGhcajr == oIGrFNXhkOfutFCSWnFrCpzlGhcajr- 0 ) oIGrFNXhkOfutFCSWnFrCpzlGhcajr=1135879828; else oIGrFNXhkOfutFCSWnFrCpzlGhcajr=105274481;double VAJveadivdPAZLywHRNVeCWRWJaExW=1836159010.380609631386607859245321489478;if (VAJveadivdPAZLywHRNVeCWRWJaExW == VAJveadivdPAZLywHRNVeCWRWJaExW ) VAJveadivdPAZLywHRNVeCWRWJaExW=2068583523.606599952318571405960625817922; else VAJveadivdPAZLywHRNVeCWRWJaExW=980428625.026478022901704896109561818289;if (VAJveadivdPAZLywHRNVeCWRWJaExW == VAJveadivdPAZLywHRNVeCWRWJaExW ) VAJveadivdPAZLywHRNVeCWRWJaExW=292578859.313130210359417993202283467577; else VAJveadivdPAZLywHRNVeCWRWJaExW=905551018.184360977469695016593508723078;if (VAJveadivdPAZLywHRNVeCWRWJaExW == VAJveadivdPAZLywHRNVeCWRWJaExW ) VAJveadivdPAZLywHRNVeCWRWJaExW=1483949713.385428615812685469946218568675; else VAJveadivdPAZLywHRNVeCWRWJaExW=1207811065.790028438840179193446524228806;if (VAJveadivdPAZLywHRNVeCWRWJaExW == VAJveadivdPAZLywHRNVeCWRWJaExW ) VAJveadivdPAZLywHRNVeCWRWJaExW=1990928054.015022277703211614948623928780; else VAJveadivdPAZLywHRNVeCWRWJaExW=1666466.174294625803684441372496949320;if (VAJveadivdPAZLywHRNVeCWRWJaExW == VAJveadivdPAZLywHRNVeCWRWJaExW ) VAJveadivdPAZLywHRNVeCWRWJaExW=1272306553.459764913710914504969742978835; else VAJveadivdPAZLywHRNVeCWRWJaExW=716337206.540645492153977369354546281899;if (VAJveadivdPAZLywHRNVeCWRWJaExW == VAJveadivdPAZLywHRNVeCWRWJaExW ) VAJveadivdPAZLywHRNVeCWRWJaExW=1236934311.628928047922292698662069110547; else VAJveadivdPAZLywHRNVeCWRWJaExW=405139275.570464515932243336560433876082;long YzCYZGtPpFqOgPBqLbpkYBqlEUkevo=1228799364;if (YzCYZGtPpFqOgPBqLbpkYBqlEUkevo == YzCYZGtPpFqOgPBqLbpkYBqlEUkevo- 0 ) YzCYZGtPpFqOgPBqLbpkYBqlEUkevo=199890787; else YzCYZGtPpFqOgPBqLbpkYBqlEUkevo=1629982588;if (YzCYZGtPpFqOgPBqLbpkYBqlEUkevo == YzCYZGtPpFqOgPBqLbpkYBqlEUkevo- 0 ) YzCYZGtPpFqOgPBqLbpkYBqlEUkevo=715038234; else YzCYZGtPpFqOgPBqLbpkYBqlEUkevo=284147399;if (YzCYZGtPpFqOgPBqLbpkYBqlEUkevo == YzCYZGtPpFqOgPBqLbpkYBqlEUkevo- 1 ) YzCYZGtPpFqOgPBqLbpkYBqlEUkevo=401877413; else YzCYZGtPpFqOgPBqLbpkYBqlEUkevo=2054325498;if (YzCYZGtPpFqOgPBqLbpkYBqlEUkevo == YzCYZGtPpFqOgPBqLbpkYBqlEUkevo- 1 ) YzCYZGtPpFqOgPBqLbpkYBqlEUkevo=1278301651; else YzCYZGtPpFqOgPBqLbpkYBqlEUkevo=237990837;if (YzCYZGtPpFqOgPBqLbpkYBqlEUkevo == YzCYZGtPpFqOgPBqLbpkYBqlEUkevo- 1 ) YzCYZGtPpFqOgPBqLbpkYBqlEUkevo=451353363; else YzCYZGtPpFqOgPBqLbpkYBqlEUkevo=1945707167;if (YzCYZGtPpFqOgPBqLbpkYBqlEUkevo == YzCYZGtPpFqOgPBqLbpkYBqlEUkevo- 0 ) YzCYZGtPpFqOgPBqLbpkYBqlEUkevo=1634958547; else YzCYZGtPpFqOgPBqLbpkYBqlEUkevo=856395888;long AWkjdXSwIzdjeSDWzXgpZzmYajLJoX=1538943694;if (AWkjdXSwIzdjeSDWzXgpZzmYajLJoX == AWkjdXSwIzdjeSDWzXgpZzmYajLJoX- 1 ) AWkjdXSwIzdjeSDWzXgpZzmYajLJoX=506362497; else AWkjdXSwIzdjeSDWzXgpZzmYajLJoX=639866874;if (AWkjdXSwIzdjeSDWzXgpZzmYajLJoX == AWkjdXSwIzdjeSDWzXgpZzmYajLJoX- 1 ) AWkjdXSwIzdjeSDWzXgpZzmYajLJoX=1450417912; else AWkjdXSwIzdjeSDWzXgpZzmYajLJoX=351933722;if (AWkjdXSwIzdjeSDWzXgpZzmYajLJoX == AWkjdXSwIzdjeSDWzXgpZzmYajLJoX- 1 ) AWkjdXSwIzdjeSDWzXgpZzmYajLJoX=1597989789; else AWkjdXSwIzdjeSDWzXgpZzmYajLJoX=1657287644;if (AWkjdXSwIzdjeSDWzXgpZzmYajLJoX == AWkjdXSwIzdjeSDWzXgpZzmYajLJoX- 1 ) AWkjdXSwIzdjeSDWzXgpZzmYajLJoX=795237630; else AWkjdXSwIzdjeSDWzXgpZzmYajLJoX=1910182621;if (AWkjdXSwIzdjeSDWzXgpZzmYajLJoX == AWkjdXSwIzdjeSDWzXgpZzmYajLJoX- 0 ) AWkjdXSwIzdjeSDWzXgpZzmYajLJoX=859069090; else AWkjdXSwIzdjeSDWzXgpZzmYajLJoX=238614003;if (AWkjdXSwIzdjeSDWzXgpZzmYajLJoX == AWkjdXSwIzdjeSDWzXgpZzmYajLJoX- 0 ) AWkjdXSwIzdjeSDWzXgpZzmYajLJoX=1141468870; else AWkjdXSwIzdjeSDWzXgpZzmYajLJoX=1860936499; }
 AWkjdXSwIzdjeSDWzXgpZzmYajLJoXy::AWkjdXSwIzdjeSDWzXgpZzmYajLJoXy()
 { this->diwjdKrFxOgz("mgngcAzRxZrDtUZLmyulKCUdAkMJuLdiwjdKrFxOgzj", true, 2041917442, 1050264267, 1124917268); }
#pragma optimize("", off)
 // <delete/>

