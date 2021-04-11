#pragma once
#include <vector>
#include "../math/Vector4D.h"

#pragma region MASKS

#define   DISPSURF_FLAG_SURFACE           (1<<0)
#define   DISPSURF_FLAG_WALKABLE          (1<<1)
#define   DISPSURF_FLAG_BUILDABLE         (1<<2)
#define   DISPSURF_FLAG_SURFPROP1         (1<<3)
#define   DISPSURF_FLAG_SURFPROP2         (1<<4)

#define   CONTENTS_EMPTY                0

#define   CONTENTS_SOLID                0x1       
#define   CONTENTS_WINDOW               0x2
#define   CONTENTS_AUX                  0x4
#define   CONTENTS_GRATE                0x8
#define   CONTENTS_SLIME                0x10
#define   CONTENTS_WATER                0x20
#define   CONTENTS_BLOCKLOS             0x40 
#define   CONTENTS_OPAQUE               0x80 
#define   LAST_VISIBLE_CONTENTS         CONTENTS_OPAQUE

#define   ALL_VISIBLE_CONTENTS            (LAST_VISIBLE_CONTENTS | (LAST_VISIBLE_CONTENTS-1))

#define   CONTENTS_TESTFOGVOLUME        0x100
#define   CONTENTS_UNUSED               0x200     
#define   CONTENTS_BLOCKLIGHT           0x400
#define   CONTENTS_TEAM1                0x800 
#define   CONTENTS_TEAM2                0x1000 
#define   CONTENTS_IGNORE_NODRAW_OPAQUE 0x2000
#define   CONTENTS_MOVEABLE             0x4000
#define   CONTENTS_AREAPORTAL           0x8000
#define   CONTENTS_PLAYERCLIP           0x10000
#define   CONTENTS_MONSTERCLIP          0x20000
#define   CONTENTS_CURRENT_0            0x40000
#define   CONTENTS_CURRENT_90           0x80000
#define   CONTENTS_CURRENT_180          0x100000
#define   CONTENTS_CURRENT_270          0x200000
#define   CONTENTS_CURRENT_UP           0x400000
#define   CONTENTS_CURRENT_DOWN         0x800000

#define   CONTENTS_ORIGIN               0x1000000 

#define   CONTENTS_MONSTER              0x2000000 
#define   CONTENTS_DEBRIS               0x4000000
#define   CONTENTS_DETAIL               0x8000000 
#define   CONTENTS_TRANSLUCENT          0x10000000
#define   CONTENTS_LADDER               0x20000000
#define   CONTENTS_HITBOX               0x40000000

#define   SURF_LIGHT                    0x0001 
#define   SURF_SKY2D                    0x0002 
#define   SURF_SKY                      0x0004 
#define   SURF_WARP                     0x0008 
#define   SURF_TRANS                    0x0010
#define   SURF_NOPORTAL                 0x0020 
#define   SURF_TRIGGER                  0x0040 
#define   SURF_NODRAW                   0x0080 

#define   SURF_HINT                     0x0100 

#define   SURF_SKIP                     0x0200   
#define   SURF_NOLIGHT                  0x0400   
#define   SURF_BUMPLIGHT                0x0800   
#define   SURF_NOSHADOWS                0x1000   
#define   SURF_NODECALS                 0x2000   
#define   SURF_NOPAINT                  SURF_NODECALS
#define   SURF_NOCHOP                   0x4000   
#define   SURF_HITBOX                   0x8000   

// -----------------------------------------------------
// spatial content masks - used for spatial queries (traceline,etc.)
// -----------------------------------------------------
#define   MASK_ALL                      (0xFFFFFFFF)
#define   MASK_SOLID                    (CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_WINDOW|CONTENTS_MONSTER|CONTENTS_GRATE)
#define   MASK_PLAYERSOLID              (CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_PLAYERCLIP|CONTENTS_WINDOW|CONTENTS_MONSTER|CONTENTS_GRATE)
#define   MASK_NPCSOLID                 (CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_MONSTERCLIP|CONTENTS_WINDOW|CONTENTS_MONSTER|CONTENTS_GRATE)
#define   MASK_NPCFLUID                 (CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_MONSTERCLIP|CONTENTS_WINDOW|CONTENTS_MONSTER)
#define   MASK_WATER                    (CONTENTS_WATER|CONTENTS_MOVEABLE|CONTENTS_SLIME)
#define   MASK_OPAQUE                   (CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_OPAQUE)
#define   MASK_OPAQUE_AND_NPCS          (MASK_OPAQUE|CONTENTS_MONSTER)
#define   MASK_BLOCKLOS                 (CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_BLOCKLOS)
#define   MASK_BLOCKLOS_AND_NPCS        (MASK_BLOCKLOS|CONTENTS_MONSTER)
#define   MASK_VISIBLE                  (MASK_OPAQUE|CONTENTS_IGNORE_NODRAW_OPAQUE)
#define   MASK_VISIBLE_AND_NPCS         (MASK_OPAQUE_AND_NPCS|CONTENTS_IGNORE_NODRAW_OPAQUE)
#define   MASK_SHOT                     (CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_MONSTER|CONTENTS_WINDOW|CONTENTS_DEBRIS|CONTENTS_HITBOX)
#define   MASK_SHOT_BRUSHONLY           (CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_WINDOW|CONTENTS_DEBRIS)
#define   MASK_SHOT_HULL                (CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_MONSTER|CONTENTS_WINDOW|CONTENTS_DEBRIS|CONTENTS_GRATE)
#define   MASK_SHOT_PORTAL              (CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_WINDOW|CONTENTS_MONSTER)
#define   MASK_SOLID_BRUSHONLY          (CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_WINDOW|CONTENTS_GRATE)
#define   MASK_PLAYERSOLID_BRUSHONLY    (CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_WINDOW|CONTENTS_PLAYERCLIP|CONTENTS_GRATE)
#define   MASK_NPCSOLID_BRUSHONLY       (CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_WINDOW|CONTENTS_MONSTERCLIP|CONTENTS_GRATE)
#define   MASK_NPCWORLDSTATIC           (CONTENTS_SOLID|CONTENTS_WINDOW|CONTENTS_MONSTERCLIP|CONTENTS_GRATE)
#define   MASK_NPCWORLDSTATIC_FLUID     (CONTENTS_SOLID|CONTENTS_WINDOW|CONTENTS_MONSTERCLIP)
#define   MASK_SPLITAREAPORTAL          (CONTENTS_WATER|CONTENTS_SLIME)
#define   MASK_CURRENT                  (CONTENTS_CURRENT_0|CONTENTS_CURRENT_90|CONTENTS_CURRENT_180|CONTENTS_CURRENT_270|CONTENTS_CURRENT_UP|CONTENTS_CURRENT_DOWN)
#define   MASK_DEADSOLID                (CONTENTS_SOLID|CONTENTS_PLAYERCLIP|CONTENTS_WINDOW|CONTENTS_GRATE)
#pragma endregion

class i_handle_entity;
struct ray_t;
class c_game_trace;
typedef c_game_trace trace_t;
class i_collideable;
class i_tracelist_data;
class c_physcollide;
struct cplane_t;
struct virtualmeshlist_t;

enum class trace_type
{
    TRACE_EVERYTHING = 0,
    TRACE_WORLD_ONLY,
    TRACE_ENTITIES_ONLY,
    TRACE_EVERYTHING_FILTER_PROPS,
};

class i_tracefilter
{
public:
    virtual bool should_hit_entity(i_handle_entity*pEntity, int contentsMask) = 0;
    virtual trace_type get_trace_type() const = 0;
};


//-----------------------------------------------------------------------------
// Classes are expected to inherit these + implement the ShouldHitEntity method
//-----------------------------------------------------------------------------

// This is the one most normal traces will inherit from
class c_tracefilter : public i_tracefilter
{
public:
    bool should_hit_entity(i_handle_entity* pEntityHandle, int /*contentsMask*/)
    {
        client_class* pEntCC = ((i_client_entity*)pEntityHandle)->get_client_class();
        if (pEntCC && ignors.size() > 0)
        {
            for (int i = 0; i < ignors.size(); i++)
            {
                if (ignors[i] != "")
                {
                    std::string pName = pEntCC->m_pNetworkName;
                    if (pName.find(ignors[i]))
                    {
                        return false;
                    }
                }
            }
        }

        return !(pEntityHandle == pSkip);
    }
    virtual trace_type get_trace_type() const
    {
        return trace_type::TRACE_EVERYTHING;
    }

    void set_ignore_class(char* Class)
    {
        ignors.push_back(Class);
    }

    std::vector<char*> ignors;

    void* pSkip;
};

class c_tracefilter_skipentity : public i_tracefilter
{
public:
    c_tracefilter_skipentity(i_handle_entity* pEntityHandle)
    {
        pSkip = pEntityHandle;
    }

    bool should_hit_entity(i_handle_entity* pEntityHandle, int /*contentsMask*/)
    {
        return !(pEntityHandle == pSkip);
    }
    virtual trace_type get_trace_type() const
    {
        return trace_type::TRACE_EVERYTHING;
    }
    void* pSkip;
};

class c_tracefilter_entities_only : public i_tracefilter
{
public:
    bool should_hit_entity(i_handle_entity* pEntityHandle, int /*contentsMask*/)
    {
        return true;
    }
    virtual trace_type get_trace_type() const
    {
        return trace_type::TRACE_ENTITIES_ONLY;
    }
};


//-----------------------------------------------------------------------------
// Classes need not inherit from these
//-----------------------------------------------------------------------------
class c_tracefilter_world_only : public i_tracefilter
{
public:
    bool should_hit_entity(i_handle_entity* /*pServerEntity*/, int /*contentsMask*/)
    {
        return false;
    }
    virtual trace_type get_trace_type() const
    {
        return trace_type::TRACE_WORLD_ONLY;
    }
};

class c_tracefilter_world_and_props_only : public i_tracefilter
{
public:
    bool should_hit_entity(i_handle_entity* /*pServerEntity*/, int /*contentsMask*/)
    {
        return false;
    }
    virtual trace_type get_trace_type() const
    {
        return trace_type::TRACE_EVERYTHING;
    }
};

class c_tracefilter_players_only_skip_one : public i_tracefilter
{
public:
    c_tracefilter_players_only_skip_one(i_client_entity* ent)
    {
        pEnt = ent;
    }
    bool should_hit_entity(i_handle_entity* pEntityHandle, int /*contentsMask*/)
    {
        return pEntityHandle != pEnt && ((i_client_entity*)pEntityHandle)->get_client_class()->m_ClassID == ClassId_CCSPlayer;
    }
    virtual trace_type get_trace_type() const
    {
        return trace_type::TRACE_ENTITIES_ONLY;
    }

private:
    i_client_entity* pEnt;
};

class c_tracefilter_skip_two_entities : public i_tracefilter
{
public:
    c_tracefilter_skip_two_entities(i_client_entity* ent1, i_client_entity* ent2)
    {
        pEnt1 = ent1;
        pEnt2 = ent2;
    }
    bool should_hit_entity(i_handle_entity* pEntityHandle, int /*contentsMask*/)
    {
        return !(pEntityHandle == pEnt1 || pEntityHandle == pEnt2);
    }
    virtual trace_type get_trace_type() const
    {
        return trace_type::TRACE_EVERYTHING;
    }

private:
    i_client_entity* pEnt1;
    i_client_entity* pEnt2;
};

class c_tracefilter_hit_all : public c_tracefilter
{
public:
    virtual bool should_hit_entity(i_handle_entity* /*pServerEntity*/, int /*contentsMask*/)
    {
        return true;
    }
};


enum class debug_trace_counter_behavior_t
{
    kTRACE_COUNTER_SET = 0,
    kTRACE_COUNTER_INC,
};

//-----------------------------------------------------------------------------
// Enumeration interface for EnumerateLinkEntities
//-----------------------------------------------------------------------------
class i_entity_enumerator
{
public:
    // This gets called with each handle
    virtual bool enum_entity(i_handle_entity* pHandleEntity) = 0;
};


struct vrush_side_info_t
{
    Vector4D plane;               // The plane of the brush side
    unsigned short bevel;    // Bevel plane?
    unsigned short thin;     // Thin?
};

class c_phys_collide;

struct vcollide_t
{
    unsigned short solidCount : 15;
    unsigned short isPacked : 1;
    unsigned short descSize;
    // VPhysicsSolids
    c_physcollide   **solids;
    char           *pkey_values;
    void           *pUserData;
};

struct cmodel_t
{
    Vector         mins, maxs;
    Vector         origin;        // for sounds or lights
    int            headnode;
    vcollide_t     vcollisionData;
};

struct csurface_t
{
    const char     *name;
    short          surfaceProps;
    unsigned short flags;         // BUGBUG: These are declared per surface, not per material, but this database is per-material now
};

//-----------------------------------------------------------------------------
// A ray...
//-----------------------------------------------------------------------------
struct ray_t
{
    VectorAligned  m_Start;  // starting point, centered within the extents
    VectorAligned  m_Delta;  // direction + length of the ray
    VectorAligned  m_StartOffset; // Add this to m_Start to Get the actual ray start
    VectorAligned  m_Extents;     // Describes an axis aligned box extruded along a ray
    const matrix3x4_t *m_pWorldAxisTransform;
    bool m_IsRay;  // are the extents zero?
    bool m_IsSwept;     // is delta != 0?

    ray_t() : m_pWorldAxisTransform(NULL) {}

    void init(Vector const& start, Vector const& end)
    {
        m_Delta = end - start;

        m_IsSwept = (m_Delta.LengthSqr() != 0);

        m_Extents.Init();

        m_pWorldAxisTransform = NULL;
        m_IsRay = true;

        // Offset m_Start to be in the center of the box...
        m_StartOffset.Init();
        m_Start = start;
    }

    void init(Vector const& start, Vector const& end, Vector const& mins, Vector const& maxs)
    {
        m_Delta = end - start;

        m_pWorldAxisTransform = NULL;
        m_IsSwept = (m_Delta.LengthSqr() != 0);

        m_Extents = maxs - mins;
        m_Extents *= 0.5f;
        m_IsRay = (m_Extents.LengthSqr() < 1e-6);

        // Offset m_Start to be in the center of the box...
        m_StartOffset = maxs + mins;
        m_StartOffset *= 0.5f;
        m_Start = start + m_StartOffset;
        m_StartOffset *= -1.0f;
    }
    Vector inv_delta() const
    {
        Vector vecInvDelta;
        for(int iAxis = 0; iAxis < 3; ++iAxis) {
            if(m_Delta[iAxis] != 0.0f) {
                vecInvDelta[iAxis] = 1.0f / m_Delta[iAxis];
            } else {
                vecInvDelta[iAxis] = FLT_MAX;
            }
        }
        return vecInvDelta;
    }

private:
};

class c_base_trace
{
public:
    bool is_disp_surface(void)           { return ((dispFlags & DISPSURF_FLAG_SURFACE) != 0); }
    bool is_disp_surface_walkable(void)  { return ((dispFlags & DISPSURF_FLAG_WALKABLE) != 0); }
    bool is_disp_surface_buildable(void) { return ((dispFlags & DISPSURF_FLAG_BUILDABLE) != 0); }
    bool is_disp_surface_prop1(void)     { return ((dispFlags & DISPSURF_FLAG_SURFPROP1) != 0); }
    bool is_disp_surface_prop2(void)     { return ((dispFlags & DISPSURF_FLAG_SURFPROP2) != 0); }

public:

    // these members are aligned!!
    Vector         startpos;            // start position
    Vector         endpos;              // final position
    cplane_t       plane;               // surface normal at impact

    float          fraction;            // time completed, 1.0 = didn't hit anything

    int            contents;            // contents on other side of surface hit
    unsigned short dispFlags;           // displacement flags for marking surfaces with data

    bool           allsolid;            // if true, plane is not valid
    bool           startsolid;          // if true, the initial point was in a solid area

    c_base_trace() {}

};

class c_game_trace : public c_base_trace
{
public:
    bool did_hit_world() const;
    bool did_hit_non_world_entity() const;
    int  get_entity_index() const;
    bool did_hit() const;
    bool is_visible() const;

public:

    float               fractionleftsolid;  // time we left a solid, only valid if we started in solid
    csurface_t          surface;            // surface hit (impact surface)
    int                 hitgroup;           // 0 == generic, non-zero is specific body part
    short               physicsbone;        // physics bone hit by trace in studio
    unsigned short      worldSurfaceIndex;  // Index of the msurface2_t, if applicable
    i_client_entity*      hit_entity;
    int                 hitbox;                       // box hit by trace in studio

    c_game_trace() {}

    // No copy constructors allowed
    c_game_trace(const c_game_trace& other) :
        fractionleftsolid(other.fractionleftsolid),
        surface(other.surface),
        hitgroup(other.hitgroup),
        physicsbone(other.physicsbone),
        worldSurfaceIndex(other.worldSurfaceIndex),
        hit_entity(other.hit_entity),
        hitbox(other.hitbox)
    {
        startpos = other.startpos;
        endpos = other.endpos;
        plane = other.plane;
        fraction = other.fraction;
        contents = other.contents;
        dispFlags = other.dispFlags;
        allsolid = other.allsolid;
        startsolid = other.startsolid;
    }

    c_game_trace& operator=(const c_game_trace& other)
    {
        startpos = other.startpos;
        endpos = other.endpos;
        plane = other.plane;
        fraction = other.fraction;
        contents = other.contents;
        dispFlags = other.dispFlags;
        allsolid = other.allsolid;
        startsolid = other.startsolid;
        fractionleftsolid = other.fractionleftsolid;
        surface = other.surface;
        hitgroup = other.hitgroup;
        physicsbone = other.physicsbone;
        worldSurfaceIndex = other.worldSurfaceIndex;
        hit_entity = other.hit_entity;
        hitbox = other.hitbox;
        return *this;
    }
};

bool c_game_trace::did_hit() const
{
    return fraction < 1 || allsolid || startsolid;
}

bool c_game_trace::is_visible() const
{
    return fraction > 0.97f;
}

class i_engine_trace
{
public:
    virtual int   get_point_contents(const Vector &vecAbsPosition, int contentsMask = MASK_ALL, i_handle_entity** ppEntity = nullptr) = 0;
    virtual int   get_point_contents_world_only(const Vector &vecAbsPosition, int contentsMask = MASK_ALL) = 0;
    virtual int   get_point_contents_collideable(i_collideable *pCollide, const Vector &vecAbsPosition) = 0;
    virtual void  clip_ray_to_entity(const ray_t &ray, unsigned int fMask, i_handle_entity*pEnt, c_game_trace *pTrace) = 0;
    virtual void  clip_ray_to_collideable(const ray_t &ray, unsigned int fMask, i_collideable*pCollide, c_game_trace*pTrace) = 0;
    virtual void  trace_ray(const ray_t &ray, unsigned int fMask, i_tracefilter*pTraceFilter, c_game_trace*pTrace) = 0;
};
