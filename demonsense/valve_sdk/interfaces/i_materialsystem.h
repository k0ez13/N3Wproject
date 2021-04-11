#pragma once

#include "i_app_system.h"

#define DECLARE_POINTER_HANDLE(name) struct name##__ { int unused; }; typedef struct name##__ *name
#define MAXSTUDIOSKINS		32

// These are given to FindMaterial to reference the texture groups that Show up on the 
#define TEXTURE_GROUP_LIGHTMAP						        "Lightmaps"
#define TEXTURE_GROUP_WORLD							          "World textures"
#define TEXTURE_GROUP_MODEL							          "Model textures"
#define TEXTURE_GROUP_VGUI							          "VGUI textures"
#define TEXTURE_GROUP_PARTICLE						        "Particle textures"
#define TEXTURE_GROUP_DECAL							          "Decal textures"
#define TEXTURE_GROUP_SKYBOX						          "SkyBox textures"
#define TEXTURE_GROUP_CLIENT_EFFECTS				      "ClientEffect textures"
#define TEXTURE_GROUP_OTHER							          "Other textures"
#define TEXTURE_GROUP_PRECACHED						        "Precached"
#define TEXTURE_GROUP_CUBE_MAP						        "CubeMap textures"
#define TEXTURE_GROUP_RENDER_TARGET					      "RenderTargets"
#define TEXTURE_GROUP_UNACCOUNTED					        "Unaccounted textures"
//#define TEXTURE_GROUP_STATIC_VERTEX_BUFFER		  "Static Vertex"
#define TEXTURE_GROUP_STATIC_INDEX_BUFFER			    "Static Indices"
#define TEXTURE_GROUP_STATIC_VERTEX_BUFFER_DISP		"Displacement Verts"
#define TEXTURE_GROUP_STATIC_VERTEX_BUFFER_COLOR	"Lighting Verts"
#define TEXTURE_GROUP_STATIC_VERTEX_BUFFER_WORLD	"World Verts"
#define TEXTURE_GROUP_STATIC_VERTEX_BUFFER_MODELS	"Model Verts"
#define TEXTURE_GROUP_STATIC_VERTEX_BUFFER_OTHER	"Other Verts"
#define TEXTURE_GROUP_DYNAMIC_INDEX_BUFFER			  "Dynamic Indices"
#define TEXTURE_GROUP_DYNAMIC_VERTEX_BUFFER			  "Dynamic Verts"
#define TEXTURE_GROUP_DEPTH_BUFFER					      "DepthBuffer"
#define TEXTURE_GROUP_VIEW_MODEL					        "ViewModel"
#define TEXTURE_GROUP_PIXEL_SHADERS					      "Pixel Shaders"
#define TEXTURE_GROUP_VERTEX_SHADERS				      "Vertex Shaders"
#define TEXTURE_GROUP_RENDER_TARGET_SURFACE			  "RenderTarget Surfaces"
#define TEXTURE_GROUP_MORPH_TARGETS					      "Morph Targets"

//-----------------------------------------------------------------------------
// forward declarations
//-----------------------------------------------------------------------------
class i_material;
class i_mesh;
class i_vertex_buffer;
class i_index_buffer;
struct material_system_config_t;
class v_matrix;
class matrix3x4_t;
class i_exture;
struct material_system_hwid_t;
class key_values;
class i_shader;
class i_vertex_texture;
class i_morph;
class i_mat_render_context;
class i_call_aueue;
struct morph_weight_t;
class i_file_list;
struct vertex_stream_spec_t;
struct shader_stencil_state_t;
struct mesh_instance_data_t;
class i_client_material_system;
class c_paint_material;
class i_paint_map_data_manager;
class i_paint_map_texture_manager;
class gpu_memory_stats;
struct aspect_ratio_info_t;
struct cascaded_shadow_mapping_state_t;

class i_material_proxy_factory;
class i_texture;
class i_material_system_hardware_config;
class c_shadow_mgr;

enum compiled_vtf_flags
{
    TEXTUREFLAGS_POINTSAMPLE = 0x00000001,
    TEXTUREFLAGS_TRILINEAR = 0x00000002,
    TEXTUREFLAGS_CLAMPS = 0x00000004,
    TEXTUREFLAGS_CLAMPT = 0x00000008,
    TEXTUREFLAGS_ANISOTROPIC = 0x00000010,
    TEXTUREFLAGS_HINT_DXT5 = 0x00000020,
    TEXTUREFLAGS_PWL_CORRECTED = 0x00000040,
    TEXTUREFLAGS_NORMAL = 0x00000080,
    TEXTUREFLAGS_NOMIP = 0x00000100,
    TEXTUREFLAGS_NOLOD = 0x00000200,
    TEXTUREFLAGS_ALL_MIPS = 0x00000400,
    TEXTUREFLAGS_PROCEDURAL = 0x00000800,
    TEXTUREFLAGS_ONEBITALPHA = 0x00001000,
    TEXTUREFLAGS_EIGHTBITALPHA = 0x00002000,
    TEXTUREFLAGS_ENVMAP = 0x00004000,
    TEXTUREFLAGS_RENDERTARGET = 0x00008000,
    TEXTUREFLAGS_DEPTHRENDERTARGET = 0x00010000,
    TEXTUREFLAGS_NODEBUGOVERRIDE = 0x00020000,
    TEXTUREFLAGS_SINGLECOPY = 0x00040000,
    TEXTUREFLAGS_PRE_SRGB = 0x00080000,
    TEXTUREFLAGS_UNUSED_00100000 = 0x00100000,
    TEXTUREFLAGS_UNUSED_00200000 = 0x00200000,
    TEXTUREFLAGS_UNUSED_00400000 = 0x00400000,
    TEXTUREFLAGS_NODEPTHBUFFER = 0x00800000,
    TEXTUREFLAGS_UNUSED_01000000 = 0x01000000,
    TEXTUREFLAGS_CLAMPU = 0x02000000,
    TEXTUREFLAGS_VERTEXTEXTURE = 0x04000000,
    TEXTUREFLAGS_SSBUMP = 0x08000000,
    TEXTUREFLAGS_UNUSED_10000000 = 0x10000000,
    TEXTUREFLAGS_BORDER = 0x20000000,
    TEXTUREFLAGS_UNUSED_40000000 = 0x40000000,
    TEXTUREFLAGS_UNUSED_80000000 = 0x80000000
};

enum standard_lightmap_t
{
    MATERIAL_SYSTEM_LIGHTMAP_PAGE_WHITE = -1,
    MATERIAL_SYSTEM_LIGHTMAP_PAGE_WHITE_BUMP = -2,
    MATERIAL_SYSTEM_LIGHTMAP_PAGE_USER_DEFINED = -3
};


struct material_system_sort_info_t
{
    i_material	*material;
    int			lightmapPageID;
};

enum material_thread_mode_t
{
    MATERIAL_SINGLE_THREADED,
    MATERIAL_QUEUED_SINGLE_THREADED,
    MATERIAL_QUEUED_THREADED
};

enum material_context_type_t
{
    MATERIAL_HARDWARE_CONTEXT,
    MATERIAL_QUEUED_CONTEXT,
    MATERIAL_NULL_CONTEXT
};

enum
{
    MATERIAL_ADAPTER_NAME_LENGTH = 512
};

struct material_texture_info_t
{
    int iExcludeInformation;
};

struct application_performance_counters_info_t
{
    float msMain;
    float msMST;
    float msGPU;
    float msFlip;
    float msTotal;
};

struct application_instant_counters_info_t
{
    uint32_t m_nCpuActivityMask;
    uint32_t m_nDeferredWordsAllocated;
};
struct material_adapter_info_t
{
    char m_pDriverName[MATERIAL_ADAPTER_NAME_LENGTH];
    unsigned int m_VendorID;
    unsigned int m_DeviceID;
    unsigned int m_SubSysID;
    unsigned int m_Revision;
    int m_nDXSupportLevel;			// This is the *preferred* dx support level
    int m_nMinDXSupportLevel;
    int m_nMaxDXSupportLevel;
    unsigned int m_nDriverVersionHigh;
    unsigned int m_nDriverVersionLow;
};

struct material_video_mode_t
{
    int m_Width;			// if width and height are 0 and you select 
    int m_Height;			// windowed mode, it'll use the window size
    image_format m_Format;	// use image_formats (ignored for windowed mode)
    int m_RefreshRate;		// 0 == default (ignored for windowed mode)
};
enum hdr_type_t
{
    HDR_TYPE_NONE,
    HDR_TYPE_INTEGER,
    HDR_TYPE_FLOAT,
};

enum restore_change_flags_t
{
    MATERIAL_RESTORE_VERTEX_FORMAT_CHANGED = 0x1,
    MATERIAL_RESTORE_RELEASE_MANAGED_RESOURCES = 0x2,
};

enum render_target_size_mode_t
{
    RT_SIZE_NO_CHANGE = 0,
    RT_SIZE_DEFAULT = 1,
    RT_SIZE_PICMIP = 2,
    RT_SIZE_HDR = 3,
    RT_SIZE_FULL_FRAME_BUFFER = 4,
    RT_SIZE_OFFSCREEN = 5,
    RT_SIZE_FULL_FRAME_BUFFER_ROUNDED_UP = 6
};

enum material_render_target_depth_t
{
    MATERIAL_RT_DEPTH_SHARED = 0x0,
    MATERIAL_RT_DEPTH_SEPARATE = 0x1,
    MATERIAL_RT_DEPTH_NONE = 0x2,
    MATERIAL_RT_DEPTH_ONLY = 0x3,
};

typedef void(*material_buffer_release_func_t)(int nChangeFlags);	// see RestoreChangeFlags_t
typedef void(*material_buffer_restore_func_t)(int nChangeFlags);	// see RestoreChangeFlags_t
typedef void(*mode_change_callback_func_t)(void);
typedef void(*end_frame_cleanup_func_t)(void);
typedef bool(*end_frame_prior_to_next_context_func_t)(void);
typedef void(*on_level_shutdown_func_t)(void *pUserData);

typedef unsigned short material_handle_t;
DECLARE_POINTER_HANDLE(material_lock_t);

class i_materialsystem : public i_app_system
{
public:

    virtual create_interface_fn               init(char const* pShaderAPIDLL, i_material_proxy_factory *pMaterialProxyFactory, create_interface_fn fileSystemFactory, create_interface_fn cvarFactory = NULL) = 0;
    virtual void                            set_shader_api(char const *pShaderAPIDLL) = 0;
    virtual void                            set_adapter(int nAdapter, int nFlags) = 0;
    virtual void                            mod_init() = 0;
    virtual void                            mod_shutdown() = 0;
    virtual void                            set_thread_mode(material_thread_mode_t mode, int nServiceThread = -1) = 0;
    virtual material_thread_mode_t          get_thread_mode() = 0;
    virtual void                            execute_queued() = 0;
    virtual void                            on_debug_event(const char *pEvent) = 0;
    virtual i_material_system_hardware_config*  get_hardware_config(const char *pVersion, int *returnCode) = 0;
    virtual void                            __unknown() = 0;
    virtual bool                            update_config(bool bForceUpdate) = 0; //20
    virtual bool                            override_config(const material_system_config_t &config, bool bForceUpdate) = 0;
    virtual const material_system_config_t&  get_current_config_for_video_card() const = 0;
    virtual bool                            get_recommended_configuration_info(int nDXLevel, key_values * pkey_values) = 0;
    virtual int                             get_display_adapter_count() const = 0;
    virtual int                             get_current_adapter() const = 0;
    virtual void                            get_display_adapter_info(int adapter, material_adapter_info_t& info) const = 0;
    virtual int                             get_mode_count(int adapter) const = 0;
    virtual void                            get_mode_info(int adapter, int mode, material_video_mode_t& info) const = 0;
    virtual void                            add_mode_change_callBack(mode_change_callback_func_t func) = 0;
    virtual void                            get_display_mode(material_video_mode_t& mode) const = 0; //30
    virtual bool                            set_mode(void* hwnd, const material_system_config_t &config) = 0;
    virtual bool                            supports_msaa_mode(int nMSAAMode) = 0;
    virtual const material_system_hwid_t&     get_video_card_identifier(void) const = 0;
    virtual void                            spew_driver_info() const = 0;
    virtual void                            get_back_buffer_dimensions(int &width, int &height) const = 0;
    virtual image_format                     get_back_buffer_format() const = 0;
    virtual const aspect_ratio_info_t&        get_aspect_ratio_info() const = 0;
    virtual bool                            supports_hdr_mode(hdr_type_t nHDRModede) = 0;
    virtual bool                            add_view(void* hwnd) = 0;
    virtual void                            remove_view(void* hwnd) = 0; //40
    virtual void                            set_view(void* hwnd) = 0;
    virtual void                            begin_frame(float frameTime) = 0;
    virtual void                            end_frame() = 0;
    virtual void                            flush(bool flushHardware = false) = 0;
    virtual unsigned int                    get_current_frame_count() = 0;
    virtual void                            swap_buffers() = 0;
    virtual void                            evict_managed_resources() = 0;
    virtual void                            release_resources(void) = 0;
    virtual void                            reacquire_resources(void) = 0;
    virtual void                            add_release_func(material_buffer_release_func_t func) = 0; //50
    virtual void                            remove_release_func(material_buffer_release_func_t func) = 0;
    virtual void                            add_restore_func(material_buffer_restore_func_t func) = 0;
    virtual void                            remove_restore_func(material_buffer_restore_func_t func) = 0;
    virtual void                            add_end_frame_cleanup_func(end_frame_cleanup_func_t func) = 0;
    virtual void                            remove_end_frame_cleanup_func(end_frame_cleanup_func_t func) = 0;
    virtual void                            on_level_shutdown() = 0;
    virtual bool                            add_on_level_shutdown_func(on_level_shutdown_func_t func, void *pUserData) = 0;
    virtual bool                            remove_on_level_shutdown_func(on_level_shutdown_func_t func, void *pUserData) = 0;
    virtual void                            on_level_loading_complete() = 0;
    virtual void                            reset_temp_hw_memory(bool bExitingLevel = false) = 0; //60
    virtual void                            handle_device_lost() = 0;
    virtual int                             shader_count() const = 0;
    virtual int                             get_shaders(int nFirstShader, int nMaxCount, i_shader **ppShaderList) const = 0;
    virtual int                             shader_flag_count() const = 0;
    virtual const char*                     shader_flag_name(int nIndex) const = 0;
    virtual void                            get_shader_fallback(const char *pShaderName, char *pFallbackShader, int nFallbackLength) = 0;
    virtual i_material_proxy_factory*       get_material_proxy_factory() = 0;
    virtual void                            set_material_proxy_factory(i_material_proxy_factory* pFactory) = 0;
    virtual void                            enable_editor_materials() = 0;
    virtual void                            enable_gbuffers() = 0; //70
    virtual void                            set_in_stubmode(bool bInStubMode) = 0;
    virtual void                            debug_print_used_materials(const char *pSearchSubString, bool bVerbose) = 0;
    virtual void                            debug_print_used_textures(void) = 0;
    virtual void                            toggle_suppress_material(char const* pMaterialName) = 0;
    virtual void                            toggle_debug_material(char const* pMaterialName) = 0;
    virtual bool                            using_fast_clipping(void) = 0;
    virtual int                             stencil_buffer_bits(void) = 0; //number of bits per pixel in the stencil buffer
    virtual void                            uncache_all_materials() = 0;
    virtual void                            uncache_unused_materials(bool bRecomputeStateSnapshots = false) = 0;
    virtual void                            cache_used_materials() = 0; //80
    virtual void                            reload_textures() = 0;
    virtual void                            reload_materials(const char *pSubString = NULL) = 0;
    virtual i_material*                      create_material(const char *pMaterialName, key_values *pVMTkey_values) = 0;
    virtual i_material*                      find_material(char const* pMaterialName, const char *pTextureGroupName, bool complain = true, const char *pComplainPrefix = NULL) = 0;
    virtual material_handle_t                first_material() const = 0;
    virtual material_handle_t                next_material(material_handle_t h) const = 0;
    virtual material_handle_t                invalid_material() const = 0;
    virtual i_material*                      get_material(material_handle_t h) const = 0;
    virtual int                             get_num_materials() const = 0;
    virtual i_texture*                       find_texture(char const* pTextureName, const char *pTextureGroupName, bool complain = true) = 0;
    virtual bool                            is_texture_loaded(char const* pTextureName) const = 0;
    virtual i_texture*                       create_procedural_texture(const char	*pTextureName, const char *pTextureGroupName, int w, int h, image_format fmt, int nFlags) = 0;
    virtual void                            begin_render_target_allocation() = 0;
    virtual void                            end_render_target_allocation() = 0; // Simulate an Alt-Tab in here, which causes a release/restore of all resources
    virtual i_texture*                       create_render_target_texture(int w, int h, render_target_size_mode_t sizeMode, image_format	format, material_render_target_depth_t depth = MATERIAL_RT_DEPTH_SHARED) = 0;
    virtual i_texture*                       create_named_render_target_texture_ex(const char *pRTName, int w, int h, render_target_size_mode_t sizeMode, image_format format, material_render_target_depth_t depth = MATERIAL_RT_DEPTH_SHARED, unsigned int textureFlags = TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT, unsigned int renderTargetFlags = 0) = 0;
    virtual i_texture*                       create_named_render_target_texture(const char *pRTName, int w, int h, render_target_size_mode_t sizeMode, image_format format, material_render_target_depth_t depth = MATERIAL_RT_DEPTH_SHARED, bool bClampTexCoords = true, bool bAutoMipMap = false) = 0;
    virtual i_texture*                       create_named_render_target_texture_ex2(const char *pRTName, int w, int h, render_target_size_mode_t sizeMode, image_format format, material_render_target_depth_t depth = MATERIAL_RT_DEPTH_SHARED, unsigned int textureFlags = TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT, unsigned int renderTargetFlags = 0) = 0;
};
