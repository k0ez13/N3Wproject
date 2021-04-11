#pragma once

#include "UtlVector.h"
#include "../interfaces/i_client_entity.h"

class GlowObjectDefinition_t
{
public:
    GlowObjectDefinition_t() { memset(this, 0, sizeof(*this)); }

    class i_client_entity* m_pEntity;    //0x0000
    union
    {
        Vector m_vGlowColor;           //0x0004
        struct
        {
            float   m_flRed;           //0x0004
            float   m_flGreen;         //0x0008
            float   m_flBlue;          //0x000C
        };
    };
    float   m_flAlpha;                 //0x0010
    uint8_t pad_0014[4];               //0x0014
    float   m_flSomeFloat;             //0x0018
    uint8_t pad_001C[4];               //0x001C
    float   m_flAnotherFloat;          //0x0020
    bool    m_bRenderWhenOccluded;     //0x0024
    bool    m_bRenderWhenUnoccluded;   //0x0025
    bool    m_bFullBloomRender;        //0x0026
    uint8_t pad_0027[5];               //0x0027
    int32_t m_nGlowStyle;              //0x002C
    int32_t m_nSplitScreenSlot;        //0x0030
    int32_t m_nNextFreeSlot;           //0x0034

    bool is_unused() const { return m_nNextFreeSlot != GlowObjectDefinition_t::ENTRY_IN_USE; }

    static const int END_OF_FREE_LIST = -1;
    static const int ENTRY_IN_USE = -2;
}; //Size: 0x0038 (56)

class c_glow_object_manager
{
public:
    int register_glow_object(i_client_entity *pEntity, const Vector &vGlowColor, float flGlowAlpha, bool bRenderWhenOccluded, bool bRenderWhenUnoccluded, int nSplitScreenSlot)
    {
        int nIndex;
        if(m_nFirstFreeSlot == GlowObjectDefinition_t::END_OF_FREE_LIST) {
            nIndex = m_GlowObjectDefinitions.AddToTail();
        } else {
            nIndex = m_nFirstFreeSlot;
            m_nFirstFreeSlot = m_GlowObjectDefinitions[nIndex].m_nNextFreeSlot;
        }

        m_GlowObjectDefinitions[nIndex].m_pEntity = pEntity;
        m_GlowObjectDefinitions[nIndex].m_vGlowColor = vGlowColor;
        m_GlowObjectDefinitions[nIndex].m_flAlpha = flGlowAlpha;
        m_GlowObjectDefinitions[nIndex].m_bRenderWhenOccluded = bRenderWhenOccluded;
        m_GlowObjectDefinitions[nIndex].m_bRenderWhenUnoccluded = bRenderWhenUnoccluded;
        m_GlowObjectDefinitions[nIndex].m_nSplitScreenSlot = nSplitScreenSlot;
        m_GlowObjectDefinitions[nIndex].m_nNextFreeSlot = GlowObjectDefinition_t::ENTRY_IN_USE;

        return nIndex;
    }

    void unregister_glow_object(int nGlowObjectHandle)
    {
        m_GlowObjectDefinitions[nGlowObjectHandle].m_nNextFreeSlot = m_nFirstFreeSlot;
        m_GlowObjectDefinitions[nGlowObjectHandle].m_pEntity = NULL;
        m_nFirstFreeSlot = nGlowObjectHandle;
    }

    void set_entity(int nGlowObjectHandle, i_client_entity *pEntity)
    {
        m_GlowObjectDefinitions[nGlowObjectHandle].m_pEntity = pEntity;
    }

    void set_color(int nGlowObjectHandle, const Vector &vGlowColor)
    {
        m_GlowObjectDefinitions[nGlowObjectHandle].m_vGlowColor = vGlowColor;
    }

    void set_alpha(int nGlowObjectHandle, float flAlpha)
    {
        m_GlowObjectDefinitions[nGlowObjectHandle].m_flAlpha = flAlpha;
    }

    void set_render_flags(int nGlowObjectHandle, bool bRenderWhenOccluded, bool bRenderWhenUnoccluded)
    {
        m_GlowObjectDefinitions[nGlowObjectHandle].m_bRenderWhenOccluded = bRenderWhenOccluded;
        m_GlowObjectDefinitions[nGlowObjectHandle].m_bRenderWhenUnoccluded = bRenderWhenUnoccluded;
    }

    bool is_reendering_when_occluded(int nGlowObjectHandle) const
    {
        return m_GlowObjectDefinitions[nGlowObjectHandle].m_bRenderWhenOccluded;
    }

    bool is_rendering_when_unoccluded(int nGlowObjectHandle) const
    {
        return m_GlowObjectDefinitions[nGlowObjectHandle].m_bRenderWhenUnoccluded;
    }

    bool has_glow_effect(i_client_entity *pEntity) const
    {
        for(int i = 0; i < m_GlowObjectDefinitions.Count(); ++i) {
            if(!m_GlowObjectDefinitions[i].is_unused() && m_GlowObjectDefinitions[i].m_pEntity == pEntity) {
                return true;
            }
        }

        return false;
    }


    CUtlVector<GlowObjectDefinition_t> m_GlowObjectDefinitions; //0x0000
    int m_nFirstFreeSlot;                                       //0x000C
};