#pragma once

#include "../math/QAngle.h"
#include "../math/VMatrix.h"

enum preview_image_ret_val_t
{
    MATERIAL_PREVIEW_IMAGE_BAD = 0,
    MATERIAL_PREVIEW_IMAGE_OK,
    MATERIAL_NO_PREVIEW_IMAGE,
};

enum material_var_flags_t
{
    MATERIAL_VAR_DEBUG = (1 << 0),
    MATERIAL_VAR_NO_DEBUG_OVERRIDE = (1 << 1),
    MATERIAL_VAR_NO_DRAW = (1 << 2),
    MATERIAL_VAR_USE_IN_FILLRATE_MODE = (1 << 3),
    MATERIAL_VAR_VERTEXCOLOR = (1 << 4),
    MATERIAL_VAR_VERTEXALPHA = (1 << 5),
    MATERIAL_VAR_SELFILLUM = (1 << 6),
    MATERIAL_VAR_ADDITIVE = (1 << 7),
    MATERIAL_VAR_ALPHATEST = (1 << 8),
    //MATERIAL_VAR_UNUSED = (1 << 9),
    MATERIAL_VAR_ZNEARER = (1 << 10),
    MATERIAL_VAR_MODEL = (1 << 11),
    MATERIAL_VAR_FLAT = (1 << 12),
    MATERIAL_VAR_NOCULL = (1 << 13),
    MATERIAL_VAR_NOFOG = (1 << 14),
    MATERIAL_VAR_IGNOREZ = (1 << 15),
    MATERIAL_VAR_DECAL = (1 << 16),
    MATERIAL_VAR_ENVMAPSPHERE = (1 << 17), // OBSOLETE
    MATERIAL_VAR_UNUSED = (1 << 18), // UNUSED
    MATERIAL_VAR_ENVMAPCAMERASPACE = (1 << 19), // OBSOLETE
    MATERIAL_VAR_BASEALPHAENVMAPMASK = (1 << 20),
    MATERIAL_VAR_TRANSLUCENT = (1 << 21),
    MATERIAL_VAR_NORMALMAPALPHAENVMAPMASK = (1 << 22),
    MATERIAL_VAR_NEEDS_SOFTWARE_SKINNING = (1 << 23), // OBSOLETE
    MATERIAL_VAR_OPAQUETEXTURE = (1 << 24),
    MATERIAL_VAR_ENVMAPMODE = (1 << 25), // OBSOLETE
    MATERIAL_VAR_SUPPRESS_DECALS = (1 << 26),
    MATERIAL_VAR_HALFLAMBERT = (1 << 27),
    MATERIAL_VAR_WIREFRAME = (1 << 28),
    MATERIAL_VAR_ALLOWALPHATOCOVERAGE = (1 << 29),
    MATERIAL_VAR_ALPHA_MODIFIED_BY_PROXY = (1 << 30),
    MATERIAL_VAR_VERTEXFOG = (1 << 31),
};

typedef unsigned short model_instance_handle_t;
typedef void* light_cache_handle_t;
typedef void* studio_decal_handle_t;
typedef int image_format;
typedef int vertex_format_t;
typedef int material_property_types_t;

class i_client_renderable;
class i_material;
class c_studio_hdr;
class i_mat_render_context;
class data_cache_handle_t;
class i_texture;
class i_materialvar;
class key_values;
struct model_t;
struct mstudioanimdesc_t;
struct mstudioseqdesc_t;
struct ray_t;
struct draw_model_info_t;
struct studiohwdata_t;
struct material_lighting_state_t;
struct color_mesh_info_t;

struct draw_model_state_t
{
    studiohdr_t*            m_pStudioHdr;
    studiohwdata_t*         m_pStudioHWData;
    i_client_renderable*      m_pRenderable;
    const matrix3x4_t*      m_pModelToWorld;
    studio_decal_handle_t     m_decals;
    int                     m_drawFlags;
    int                     m_lod;
};

struct static_prop_render_info_t
{
    const matrix3x4_t*      pModelToWorld;
    const model_t*          pModel;
    i_client_renderable*      pRenderable;
    Vector*                 pLightingOrigin;
    short                   skin;
    model_instance_handle_t   instance;
};

struct model_render_info_t
{
    Vector                  origin;
    QAngle                  angles;
	char                    pad[4];
    i_client_renderable*      pRenderable;
    const model_t*          pModel;
    const matrix3x4_t*      pModelToWorld;
    const matrix3x4_t*      pLightingOffset;
    const Vector*           pLightingOrigin;
    int                     flags;
    int                     entity_index;
    int                     skin;
    int                     body;
    int                     hitboxset;
    model_instance_handle_t   instance;

    model_render_info_t()
    {
        pModelToWorld = NULL;
        pLightingOffset = NULL;
        pLightingOrigin = NULL;
    }
};

enum class override_type {
    Normal = 0,
    BuildShadows,
    DepthWrite,
    CustomMaterial, // weapon skins
    SsaoDepthWrite
};

struct lighting_query_t
{
    Vector                  m_LightingOrigin;
    model_instance_handle_t   m_InstanceHandle;
    bool                    m_bAmbientBoost;
};

struct static_lighting_query_t : public lighting_query_t
{
    i_client_renderable*        m_pRenderable;
};

class i_material
{
public:
    virtual const char*             get_name() const = 0;
    virtual const char*             get_texture_group_name() const = 0;
    virtual preview_image_ret_val_t get_preview_image_properties(int *width, int *height, image_format *image_format, bool* isTranslucent) const = 0;
    virtual preview_image_ret_val_t get_preview_image(unsigned char *data, int width, int height, image_format image_format) const = 0;
    virtual int                     get_mapping_width() = 0;
    virtual int                     get_mapping_height() = 0;
    virtual int                     get_num_animation_frames() = 0;
    virtual bool                    in_material_page(void) = 0;
    virtual    void                 get_material_offset(float *pOffset) = 0;
    virtual void                    get_material_scale(float *pScale) = 0;
    virtual i_material*             get_material_page(void) = 0;
    virtual i_materialvar*          find_var(const char *varName, bool *found, bool complain = true) = 0;
    virtual void                    increment_reference_count(void) = 0;
    virtual void                    decrement_reference_count(void) = 0;
    void                            add_ref() { increment_reference_count(); }
    void                            release() { decrement_reference_count(); }
    virtual int                     get_enumeration_id(void) const = 0;
    virtual void                    Get_lowres_color_sample(float s, float t, float *color) const = 0;
    virtual void                    recompute_state_snapshots() = 0;
    virtual bool                    is_translucent() = 0;
    virtual bool                    is_alpha_tested() = 0;
    virtual bool                    is_vertex_lit() = 0;
    virtual vertex_format_t         get_vertex_format() const = 0;
    virtual bool                    gas_proxy(void) const = 0;
    virtual bool                    uses_env_cubemap(void) = 0;
    virtual bool                    needs_tangent_space(void) = 0;
    virtual bool                    needs_power_of_two_frame_buffer_texture(bool bCheckSpecificToThisFrame = true) = 0;
    virtual bool                    needs_full_frame_buffer_texture(bool bCheckSpecificToThisFrame = true) = 0;
    virtual bool                    needs_software_skinning(void) = 0;
    virtual void                    alpha_modulate(float alpha) = 0;
    virtual void                    color_modulate(float r, float g, float b) = 0;
    virtual void                    set_material_var_flag(material_var_flags_t flag, bool on) = 0;
    virtual bool                    get_material_var_flag(material_var_flags_t flag) const = 0;
    virtual void                    get_reflectivity(Vector& reflect) = 0;
    virtual bool                    get_property_flag(material_property_types_t type) = 0;
    virtual bool                    is_two_sided() = 0;
    virtual void                    set_shader(const char *pShaderName) = 0;
    virtual int                     get_num_passes(void) = 0;
    virtual int                     get_texture_memory_bytes(void) = 0;
    virtual void                    refresh() = 0;
    virtual bool                    needs_lightmap_blend_alpha(void) = 0;
    virtual bool                    needs_software_lighting(void) = 0;
    virtual int                     shader_param_count() const = 0;
    virtual i_materialvar**         get_shader_params(void) = 0;
    virtual bool                    is_error_material() const = 0;
    virtual void                    unused() = 0;
    virtual float                   get_alpha_modulation() = 0;
    virtual void                    get_color_modulation(float *r, float *g, float *b) = 0;
    virtual bool                    is_translucent_under_modulation(float fAlphaModulation = 1.0f) const = 0;
    virtual i_materialvar*          find_var_fast(char const *pVarName, unsigned int *pToken) = 0;
    virtual void                    set_shader_and_params(key_values *pkey_values) = 0;
    virtual const char*             get_shader_name() const = 0;
    virtual void                    delete_if_unreferenced() = 0;
    virtual bool                    is_sprite_card() = 0;
    virtual void                    call_bind_proxy(void *proxyData) = 0;
    virtual void                    refresh_preserving_material_vars() = 0;
    virtual bool                    was_reloaded_from_whitelist() = 0;
    virtual bool                    set_temp_excluded(bool bSet, int nExcludedDimensionLimit) = 0;
    virtual int                     get_reference_count() const = 0;
};

class i_vmodelrender
{
    std::byte pad_0[0x250];
    i_material* materialOverride;
    std::byte pad_1[0xC];
    override_type override_type_t;

public:
    virtual int                     draw_model(int flags, i_client_renderable*pRenderable, model_instance_handle_t instance, int entity_index, const model_t *model, Vector const& origin, QAngle const& angles, int skin, int body, int hitboxset, const matrix3x4_t *modelToWorld = NULL, const matrix3x4_t *pLightingOffset = NULL) = 0;
    virtual void                    forced_material_override(i_material *newMaterial, override_type noverride_type = override_type::Normal, int nOverrides = 0) = 0;

    bool is_forced_override()
    {
        if (!materialOverride)
            return override_type_t == override_type::DepthWrite || override_type_t == override_type::SsaoDepthWrite; // see CStudioRenderContext::IsForcedMaterialOverride

        return strstr(materialOverride->get_name(), "dev/glow");
    }
    virtual bool                    is_forced_material_override(void) = 0;
    virtual void                    set_view_target(const c_studio_hdr *pStudioHdr, int nBodyIndex, const Vector& target) = 0;
    virtual model_instance_handle_t create_instance(i_client_renderable*pRenderable, light_cache_handle_t *pCache = NULL) = 0;
    virtual void                    destroy_instance(model_instance_handle_t handle) = 0;
    virtual void                    set_static_lighting(model_instance_handle_t handle, light_cache_handle_t* pHandle) = 0;
    virtual light_cache_handle_t    get_static_lighting(model_instance_handle_t handle) = 0;
    virtual bool                    change_instance(model_instance_handle_t handle, i_client_renderable*pRenderable) = 0;
    virtual void                    add_decal(model_instance_handle_t handle, ray_t const& ray, Vector const& decalUp, int decalIndex, int body, bool noPokeThru, int maxLODToDecal) = 0;
    virtual void                    remove_all_decals(model_instance_handle_t handle) = 0;
    virtual bool                    model_has_decals(model_instance_handle_t handle) = 0;
    virtual void                    remove_all_decals_from_all_models() = 0;
    virtual matrix3x4_t*            draw_model_shadow_setup(i_client_renderable*pRenderable, int body, int skin, draw_model_info_t *pInfo, matrix3x4_t *pCustomBoneToWorld = NULL) = 0;
    virtual void                    draw_model_shadow(i_client_renderable*pRenderable, const draw_model_info_t &info, matrix3x4_t *pCustomBoneToWorld = NULL) = 0;
    virtual bool                    recompute_static_lighting(model_instance_handle_t handle) = 0;
    virtual void                    release_all_static_prop_colorData(void) = 0;
    virtual void                    restore_all_static_prop_color_data(void) = 0;
    virtual int                     draw_model_ex(model_render_info_t &pInfo) = 0;
    virtual int                     draw_model_ex_static_prop(model_render_info_t &pInfo) = 0;
    virtual bool                    draw_model_setup(model_render_info_t &pInfo, draw_model_state_t *pState, matrix3x4_t **ppBoneToWorldOut) = 0;
    virtual void                    draw_model_execute(i_mat_render_context* ctx, const draw_model_state_t& state, const model_render_info_t& pInfo, matrix3x4_t* pCustomBoneToWorld = NULL) = 0;
    virtual void                    setup_lighting(const Vector &vecCenter) = 0;
    virtual int                     draw_static_prop_array_fast(static_prop_render_info_t *pProps, int count, bool bShadowDepth) = 0;
    virtual void                    suppress_engine_lighting(bool bSuppress) = 0;
    virtual void                    setup_color_meshes(int nTotalVerts) = 0;
    virtual void                    setup_lighting_ex(const Vector &vecCenter, model_instance_handle_t handle) = 0;
    virtual bool                    get_brightest_shadowing_light_source(const Vector &vecCenter, Vector& lightPos, Vector& lightBrightness, bool bAllowNonTaggedLights) = 0;
    virtual void                    compute_lighting_state(int nCount, const lighting_query_t *pQuery, material_lighting_state_t *pState, i_texture **ppEnvCubemapTexture) = 0;
    virtual void                    get_model_decal_handles(studio_decal_handle_t *pDecals, int nDecalStride, int nCount, const model_instance_handle_t *pHandles) = 0;
    virtual void                    compute_static_lighting_state(int nCount, const static_lighting_query_t *pQuery, material_lighting_state_t *pState, material_lighting_state_t *pDecalState, color_mesh_info_t **ppStaticLighting, i_texture **ppEnvCubemapTexture, data_cache_handle_t *pColorMeshHandles) = 0;
    virtual void                    cleanup_static_lighting_state(int nCount, data_cache_handle_t* pColorMeshHandles) = 0;
};

