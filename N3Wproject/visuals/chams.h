#pragma once
#include "../../valve_sdk/csgostructs.h"


namespace chams
{



    //old chams
    void init();
    void shutdown();

    void more_chams();

    void scene_end_chams();
    
    void desync_chams_effect();

    void dme_chams(i_mat_render_context* ctx, const draw_model_state_t& state, const model_render_info_t& info, matrix3x4_t* matrix);

    void override_mat(bool ignoreZ, bool flat, bool wireframe, bool glass, const Color& rgba, bool modulate = true, bool gold = false, bool crystal = false, bool crystal2 = false, bool silver = false, bool velvet = false);

        
    //chams materials
    i_material* materialRegular = nullptr;
    i_material* materialRegularIgnoreZ = nullptr;
    i_material* materialFlatIgnoreZ = nullptr;
    i_material* materialFlat = nullptr;
    i_material* materialGold = nullptr;
    i_material* materialGlass = nullptr;
    i_material* materialGloss = nullptr;
    i_material* materialCrystal = nullptr;
    i_material* materialCrystalBlue = nullptr;
    i_material* materialSilver = nullptr;
    i_material* materialVelvet = nullptr;

    //old materials
   /* i_material* material_glow = nullptr;
    //i_material* material_glow_walls = nullptr;
    i_material* material_flat = nullptr;
    i_material* material_flat_walls = nullptr;*/
}
                                                                                                                         
