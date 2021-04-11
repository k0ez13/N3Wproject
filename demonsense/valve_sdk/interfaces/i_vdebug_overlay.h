#pragma once
#include "../math/Vector.h"
#include "../math/QAngle.h"
#include "../math/VMatrix.h"

class overlay_text_t;

class i_vdebug_overlay
{
public:
    virtual void            add_entity_text_overlay(int ent_index, int line_offset, float duration, int r, int g, int b, int a, const char* format, ...) = 0;
    virtual void            add_box_overlay(const Vector& origin, const Vector& mins, const Vector& max, QAngle const& orientation, int r, int g, int b, int a, float duration) = 0;
    virtual void            add_sphere_overlay(const Vector& vOrigin, float flRadius, int nTheta, int nPhi, int r, int g, int b, int a, float flDuration) = 0;
    virtual void            add_triangle_overlay(const Vector& p1, const Vector& p2, const Vector& p3, int r, int g, int b, int a, bool noDepthTest, float duration) = 0;
    virtual void            __unkn() = 0;
    virtual void            add_line_overlay(const Vector& origin, const Vector& dest, int r, int g, int b, bool noDepthTest, float duration) = 0;
    virtual void            add_text_overlay(const Vector& origin, float duration, const char* format, ...) = 0;
    virtual void            add_text_overlay(const Vector& origin, int line_offset, float duration, const char* format, ...) = 0;
    virtual void            add_screen_text_overlay(float flXPos, float flYPos, float flDuration, int r, int g, int b, int a, const char* text) = 0;
    virtual void            add_swept_box_overlay(const Vector& start, const Vector& end, const Vector& mins, const Vector& max, const QAngle& angles, int r, int g, int b, int a, float flDuration) = 0;
    virtual void            add_grid_overlay(const Vector& origin) = 0;
    virtual void            add_coord_frame_overlay(const matrix3x4_t& frame, float flScale, int vColorTable[3][3] = NULL) = 0;
    virtual int             screen_position(const Vector& point, Vector& screen) = 0;
    virtual int             screen_position(float flXPos, float flYPos, Vector& screen) = 0;
    virtual overlay_text_t* get_first(void) = 0;
    virtual overlay_text_t* get_next(overlay_text_t* current) = 0;
    virtual void            clear_dead_overlays(void) = 0;
    virtual void            clear_all_overlays() = 0;
    virtual void            add_text_verlay_rgb(const Vector& origin, int line_offset, float duration, float r, float g, float b, float alpha, const char* format, ...) = 0;
    virtual void            add_text_overlay_rgb(const Vector& origin, int line_offset, float duration, int r, int g, int b, int a, const char* format, ...) = 0;
    virtual void            add_line_overlay_alpha(const Vector& origin, const Vector& dest, int r, int g, int b, int a, bool noDepthTest, float duration) = 0;
    virtual void            add_box_overlay2(const Vector& origin, const Vector& mins, const Vector& max, QAngle const& orientation, const uint8_t* faceColor, const uint8_t* edgeColor, float duration) = 0;
    virtual void            purge_text_overlays() = 0;
    virtual void            draw_pill(const Vector& mins, const Vector& max, float& diameter, int r, int g, int b, int a, float duration) = 0;
    void                    add_capsule_overlay(Vector& mins, Vector& maxs, float pillradius, int r, int g, int b, int a, float duration)
    {
        return vfunc<void(__thiscall*)(void*, Vector&, Vector&, float&, int, int, int, int, float)>(this, 24)(this, mins, maxs, pillradius, r, g, b, a, duration);
    }

private:
    inline void add_text_overlay(const Vector& origin, int line_offset, float duration, int r, int g, int b, int a, const char* format, ...) {} /* catch improper use of bad interface. Needed because '0' duration can be resolved by compiler to NULL format string (i.e., compiles but calls wrong function) */
};