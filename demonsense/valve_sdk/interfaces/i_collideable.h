#pragma once

enum    solid_type_t;
class   i_handle_entity;
struct  ray_t;
struct  model_t;
class   c_game_trace;
typedef c_game_trace trace_t;
class   i_client_unknown;
class   matrix3x4_t;

class i_collideable
{
public:
    virtual i_handle_entity*     get_entity_handle() = 0;
    virtual const Vector&       obb_mins() const = 0;
    virtual const Vector&       obb_maxs() const = 0;
    virtual void                world_space_trigger_bounds(Vector *pVecWorldMins, Vector *pVecWorldMaxs) const = 0;
    virtual bool                test_collision(const ray_t &ray, unsigned int fContentsMask, trace_t& tr) = 0;
    virtual bool                test_hitboxes(const ray_t &ray, unsigned int fContentsMask, trace_t& tr) = 0;
    virtual int                 get_collision_model_index() = 0;
    virtual const model_t*      get_collision_model() = 0;
    virtual const Vector&       get_collision_origin() const = 0;
    virtual const QAngle&       get_collision_angles() const = 0;
    virtual const matrix3x4_t&  collision_to_world_transform() const = 0;
    virtual solid_type_t        get_solid() const = 0;
    virtual int                 get_solid_flags() const = 0;
    virtual i_client_unknown*    get_iclient_unknown() = 0;
    virtual int                 get_collision_group() const = 0;
    virtual void                world_space_surrounding_bounds(Vector *pVecMins, Vector *pVecMaxs) = 0;
    virtual bool                should_touch_trigger(int triggerSolidFlags) const = 0;
    virtual const matrix3x4_t*  get_root_parent_to_world_transform() const = 0;
};