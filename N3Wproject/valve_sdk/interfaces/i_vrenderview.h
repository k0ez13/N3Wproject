#pragma once

#include "../math/QAngle.h"
#include "../math/Vector2D.h"
#include "../math/Vector4D.h"
#include "../math/VMatrix.h"

//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------
class c_viewsetup;
class c_engine_sprite;
class i_client_entity;
class i_material;
struct model_t;
class i_client_renderable;
class i_texture;


//-----------------------------------------------------------------------------
// Flags used by DrawWorldLists
//-----------------------------------------------------------------------------
enum
{
    DRAWWORLDLISTS_DRAW_STRICTLYABOVEWATER = 0x001,
    DRAWWORLDLISTS_DRAW_STRICTLYUNDERWATER = 0x002,
    DRAWWORLDLISTS_DRAW_INTERSECTSWATER = 0x004,
    DRAWWORLDLISTS_DRAW_WATERSURFACE = 0x008,
    DRAWWORLDLISTS_DRAW_SKYBOX = 0x010,
    DRAWWORLDLISTS_DRAW_CLIPSKYBOX = 0x020,
    DRAWWORLDLISTS_DRAW_SHADOWDEPTH = 0x040,
    DRAWWORLDLISTS_DRAW_REFRACTION = 0x080,
    DRAWWORLDLISTS_DRAW_REFLECTION = 0x100,
    DRAWWORLDLISTS_DRAW_WORLD_GEOMETRY = 0x200,
    DRAWWORLDLISTS_DRAW_DECALS_AND_OVERLAYS = 0x400,
};

enum
{
    MAT_SORT_GROUP_STRICTLY_ABOVEWATER = 0,
    MAT_SORT_GROUP_STRICTLY_UNDERWATER,
    MAT_SORT_GROUP_INTERSECTS_WATER_SURFACE,
    MAT_SORT_GROUP_WATERSURFACE,

    MAX_MAT_SORT_GROUPS
};

//-----------------------------------------------------------------------------
// Leaf index
//-----------------------------------------------------------------------------
typedef unsigned short eafindex_t;
enum
{
    INVALID_LEAF_INDEX = (eafindex_t)~0
};

struct world_list_leaf_data_t
{
    eafindex_t     leafIndex;    // 16 bits
    int16_t         waterData;
    uint16_t        firstTranslucentSurface;    // engine-internal list index
    uint16_t        translucentSurfaceCount;    // count of translucent surfaces+disps
};

struct world_list_info_t
{
    int                     m_ViewFogVolume;
    int                     m_LeafCount;
    bool                    m_bHasWater;
    world_list_leaf_data_t*    m_pLeafDataList;
};

class i_world_render_list /*: public i_ref_counted*/
{
};

//-----------------------------------------------------------------------------
// Describes the fog volume for a particular point
//-----------------------------------------------------------------------------
struct visible_fog_volume_info_t
{
    int            m_nVisibleFogVolume;
    int            m_nVisibleFogVolumeLeaf;
    bool        m_bEyeInFogVolume;
    float       m_flDistanceToWater;
    float       m_flWaterHeight;
    i_material*  m_pFogVolumeMaterial;
};

struct v_plane
{
    Vector        m_Normal;
    vec_t        m_Dist;
};
#define FRUSTUM_NUMPLANES    6
typedef v_plane Frustum[FRUSTUM_NUMPLANES];
//-----------------------------------------------------------------------------
// Vertex format for brush models
//-----------------------------------------------------------------------------
struct brush_vertex_t
{
    Vector        m_Pos;
    Vector        m_Normal;
    Vector        m_TangentS;
    Vector        m_TangentT;
    Vector2D    m_TexCoord;
    Vector2D    m_LightmapCoord;

private:
    brush_vertex_t(const brush_vertex_t& src);
};

//-----------------------------------------------------------------------------
// Visibility data for area portal culling
//-----------------------------------------------------------------------------
struct vis_override_data_t
{
    Vector        m_vecVisOrigin;                    // The point to to use as the viewpoint for area portal backface cull checks.
    float        m_fDistToAreaPortalTolerance;    // The distance from an area portal before using the full screen as the viewable portion.
};


//-----------------------------------------------------------------------------
// interface for asking about the Brush surfaces from the client DLL
//-----------------------------------------------------------------------------

class i_brush_surface
{
public:
    // Computes texture coordinates + lightmap coordinates given a world position
    virtual void compute_texture_coordinate(Vector const& worldPos, Vector2D& texCoord) = 0;
    virtual void compute_lightmap_coordinate(Vector const& worldPos, Vector2D& lightmapCoord) = 0;

    // Gets the vertex data for this surface
    virtual int  get_vertex_count() const = 0;
    virtual void get_vertex_data(brush_vertex_t* pVerts) = 0;

    // Gets at the material properties for this surface
    virtual i_material* get_material() = 0;
};


//-----------------------------------------------------------------------------
// interface for installing a new renderer for brush surfaces
//-----------------------------------------------------------------------------

class i_brush_renderer
{
public:
    // Draws the surface; returns true if decals should be rendered on this surface
    virtual bool render_brush_model_surface(i_client_entity* pBaseEntity, i_brush_surface* pBrushSurface) = 0;
};

#define MAX_VIS_LEAVES    32
#define MAX_AREA_STATE_BYTES        32
#define MAX_AREA_PORTAL_STATE_BYTES 24

class i_vrenderview
{
    enum
    {
        VIEW_SETUP_VIS_EX_RETURN_FLAGS_USES_RADIAL_VIS = 0x00000001
    };
public:
    virtual void                draw_brush_model(i_client_entity *baseentity, model_t *model, const Vector& origin, const QAngle& angles, bool sort) = 0;
    virtual void                draw_identity_brush_model(i_world_render_list *pList, model_t *model) = 0;
    virtual void                touch_light(struct dlight_t *light) = 0;
    virtual void                draw_3d_debug_overlays(void) = 0;
    virtual void                set_blend(float blend) = 0;
    virtual float               get_blend(void) = 0;
    virtual void                set_color_modulation(float const* blend) = 0;
    void                        set_color_modulation(float r, float g, float b)
    {
        float clr[3] = { r, g, b };
        set_color_modulation(clr);
    }
    virtual void                get_color_modulation(float* blend) = 0;
    virtual void                scene_begin(void) = 0;
    virtual void                scene_end(void) = 0;
    virtual void                get_visible_fog_volume(const Vector& eyePoint, visible_fog_volume_info_t *pInfo) = 0;
    virtual i_world_render_list*   create_world_List() = 0;
    virtual void                build_world_lists(i_world_render_list *pList, world_list_info_t* pInfo, int iForceFViewLeaf, const vis_override_data_t* pVisData = NULL, bool bShadowDepth = false, float *pReflectionWaterHeight = NULL) = 0;
    virtual void                draw_world_lists(i_world_render_list *pList, unsigned long flags, float waterZAdjust) = 0;
    virtual int                 get_num_indices_for_world_lists(i_world_render_list *pList, unsigned long nFlags) = 0;
    virtual void                draw_top_view(bool enable) = 0;
    virtual void                top_view_bounds(Vector2D const& mins, Vector2D const& maxs) = 0;
    virtual void                draw_lights(void) = 0;
    virtual void                draw_mask_entities(void) = 0;
    virtual void                draw_translucent_surfaces(i_world_render_list *pList, int *pSortList, int sortCount, unsigned long flags) = 0;
    virtual void                draw_line_file(void) = 0;
    virtual void                draw_lightmaps(i_world_render_list *pList, int pageId) = 0;
    virtual void                view_setup_vis(bool novis, int numorigins, const Vector origin[]) = 0;
    virtual bool                are_any_leaves_visible(int *leafList, int nLeaves) = 0;
    virtual    void             vgui_paint(void) = 0;
    virtual void                view_draw_fade(uint8_t *color, i_material *pMaterial) = 0;
    virtual void                old_set_projection_matrix(float fov, float zNear, float zFar) = 0;
    virtual unsigned long       get_light_at_point(Vector& pos) = 0;
    virtual int                 get_view_entity(void) = 0;
    virtual bool                is_view_entity(int ent_index) = 0;
    virtual float               get_field_of_view(void) = 0;
    virtual unsigned char**     get_area_bits(void) = 0;
    virtual void                set_fog_volume_state(int nVisibleFogVolume, bool bUseHeightFog) = 0;
    virtual void                install_brush_surface_renderer(i_brush_renderer* pBrushRenderer) = 0;
    virtual void                draw_brush_model_shadow(i_client_renderable*pRenderable) = 0;
    virtual    bool             leaf_contains_translucent_surfaces(i_world_render_list *pList, int sortIndex, unsigned long flags) = 0;
    virtual bool                does_box_intersect_water_volume(const Vector &mins, const Vector &maxs, int leafWaterDataID) = 0;
    virtual void                set_area_state(unsigned char chAreaBits[MAX_AREA_STATE_BYTES], unsigned char chAreaPortalBits[MAX_AREA_PORTAL_STATE_BYTES]) = 0;
    virtual void                vgui_paint(int mode) = 0;
    virtual void                push_3d_view(const c_viewsetup &view, int nFlags, i_texture* pRenderTarget, Frustum frustumPlanes) = 0;
    virtual void                push_2d_view(const c_viewsetup &view, int nFlags, i_texture* pRenderTarget, Frustum frustumPlanes) = 0;
    virtual void                pop_view(Frustum frustumPlanes) = 0;
    virtual void                set_main_view(const Vector &vecOrigin, const QAngle &angles) = 0;
    virtual void                view_setup_vis_ex(bool novis, int numorigins, const Vector origin[], unsigned int &returnFlags) = 0;
    virtual void                override_view_frustum(Frustum custom) = 0;
    virtual void                draw_brush_model_shadow_depth(i_client_entity *baseentity, model_t *model, const Vector& origin, const QAngle& angles, bool bSort) = 0;
    virtual void                update_brush_model_lightmap(model_t *model, i_client_renderable*pRenderable) = 0;
    virtual void                begin_update_lightmaps(void) = 0;
    virtual void                end_update_lightmaps(void) = 0;
    virtual void                old_set_off_center_projection_matrix(float fov, float zNear, float zFar, float flAspectRatio, float flBottom, float flTop, float flLeft, float flRight) = 0;
    virtual void                old_set_projection_matrix_ortho(float left, float top, float right, float bottom, float zNear, float zFar) = 0;
    virtual void                push_3d_view(const c_viewsetup &view, int nFlags, i_texture* pRenderTarget, Frustum frustumPlanes, i_texture* pDepthTexture) = 0;
    virtual void                get_matrices_for_view(const c_viewsetup &view, VMatrix *pWorldToView, VMatrix *pViewToProjection, VMatrix *pWorldToProjection, VMatrix *pWorldToPixels) = 0;
};