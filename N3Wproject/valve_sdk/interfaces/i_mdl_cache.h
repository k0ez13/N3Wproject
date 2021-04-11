#pragma once

#include "i_app_system.h"
#include "../misc/studio.h"

class studiohdr_t;
struct studiohwdata_t;
struct vcollide_t;
struct virtualmodel_t;
struct vertexFileHeader_t;

enum mdl_cachedata_type_t
{
    // Callbacks to Get called when data is loaded or unloaded for these:
    MDLCACHE_STUDIOHDR = 0,
    MDLCACHE_STUDIOHWDATA,
    MDLCACHE_VCOLLIDE,

    // Callbacks NOT called when data is loaded or unloaded for these:
    MDLCACHE_ANIMBLOCK,
    MDLCACHE_VIRTUALMODEL,
    MDLCACHE_VERTEXES,
    MDLCACHE_DECODEDANIMBLOCK
};

enum mdl_cacheflush_t
{
    MDLCACHE_FLUSH_STUDIOHDR = 0x01,
    MDLCACHE_FLUSH_STUDIOHWDATA = 0x02,
    MDLCACHE_FLUSH_VCOLLIDE = 0x04,
    MDLCACHE_FLUSH_ANIMBLOCK = 0x08,
    MDLCACHE_FLUSH_VIRTUALMODEL = 0x10,
    MDLCACHE_FLUSH_AUTOPLAY = 0x20,
    MDLCACHE_FLUSH_VERTEXES = 0x40,

    MDLCACHE_FLUSH_IGNORELOCK = 0x80000000,
    MDLCACHE_FLUSH_ALL = 0xFFFFFFFF
};

class i_mdl_cachenotify
{
public:
    virtual void on_data_loaded(mdl_cachedata_type_t type, mdl_handle_t handle) = 0;
    virtual void on_data_unloaded(mdl_cachedata_type_t type, mdl_handle_t handle) = 0;
};

class i_mdl_cache : public i_app_system
{
public:
    virtual void                set_cache_notify(i_mdl_cachenotify*pNotify) = 0;
    virtual mdl_handle_t         find_mdl(const char *pMDLRelativePath) = 0;
    virtual int                 sdd_ref(mdl_handle_t handle) = 0;
    virtual int                 release(mdl_handle_t handle) = 0;
    virtual int                 get_ref(mdl_handle_t handle) = 0;
    virtual studiohdr_t*        set_studio_hdr(mdl_handle_t handle) = 0;
    virtual studiohwdata_t*     get_hardware_data(mdl_handle_t handle) = 0;
    virtual vcollide_t*         get_vcollide(mdl_handle_t handle) = 0;
    virtual unsigned char*      get_anim_block(mdl_handle_t handle, int nBlock) = 0;
    virtual virtualmodel_t*     get_virtual_model(mdl_handle_t handle) = 0;
    virtual int                 get_autoplay_list(mdl_handle_t handle, unsigned short **pOut) = 0;
    virtual vertexFileHeader_t* get_vertex_data(mdl_handle_t handle) = 0;
    virtual void                touch_all_data(mdl_handle_t handle) = 0;
    virtual void                set_user_data(mdl_handle_t handle, void* pData) = 0;
    virtual void*               get_user_data(mdl_handle_t handle) = 0;
    virtual bool                is_error_model(mdl_handle_t handle) = 0;
    virtual void                flush(mdl_cacheflush_t nFlushFlags = MDLCACHE_FLUSH_ALL) = 0;
    virtual void                flush(mdl_handle_t handle, int nFlushFlags = MDLCACHE_FLUSH_ALL) = 0;
    virtual const char*         get_model_name(mdl_handle_t handle) = 0;
    virtual virtualmodel_t*     get_virtual_model_fast(const studiohdr_t *pStudioHdr, mdl_handle_t handle) = 0;
    virtual void                begin_lock() = 0;
    virtual void                end_lock() = 0;
    virtual int*                get_frame_unlock_counter_ptr_old() = 0;
    virtual void                finish_pending_loads() = 0;
    virtual vcollide_t*         get_vcollide_ex(mdl_handle_t handle, bool synchronousLoad = true) = 0;
    virtual bool                get_vcollide_size(mdl_handle_t handle, int *pVCollideSize) = 0;
    virtual bool                get_async_load(mdl_cachedata_type_t type) = 0;
    virtual bool                set_async_load(mdl_cachedata_type_t type, bool bAsync) = 0;
    virtual void                begin_map_load() = 0;
    virtual void                end_map_load() = 0;
    virtual void                mark_as_loaded(mdl_handle_t handle) = 0;
    virtual void                init_preload_data(bool rebuild) = 0;
    virtual void                shutdown_preload_data() = 0;
    virtual bool                is_data_loaded(mdl_handle_t handle, mdl_cachedata_type_t type) = 0;
    virtual int*                get_frame_unlock_counter_ptr(mdl_cachedata_type_t type) = 0;
    virtual studiohdr_t*        lock_studio_hdr(mdl_handle_t handle) = 0;
    virtual void                unlock_studio_hdr(mdl_handle_t handle) = 0;
    virtual bool                preload_model(mdl_handle_t handle) = 0;
    virtual void                reset_error_model_status(mdl_handle_t handle) = 0;
    virtual void                mark_frame() = 0;
    virtual void                begin_coarse_lock() = 0;
    virtual void                end_coarse_lock() = 0;
    virtual void                reload_vcollide(mdl_handle_t handle) = 0;
};