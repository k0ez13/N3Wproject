#include <fstream>
#include "chams.h"
#include "../../hooks/hooks.h"
#include "../../options/options.h"
#include "../../helpers/input.h"
#include "../notify/notify.h"
#include "../../render/render.h"
#include "../../helpers/math.h"
#include "../legit_aimbot/backtrack.h"

//old chams
/*//int selected_entity;

//auto global_chams_settings = settings::visuals::chams::chams_items[selected_entity];
//auto global_chams_settings_ragdoll = settings::visuals::chams::chams_items_ragdoll[selected_entity];
//int selected_entity;
//auto global_chams_settings = settings::visuals::chams::chams_items[selected_entity];*/

namespace chams
{
	void init()
	{
		materialRegular = g_mat_system->find_material("debug/debugambientcube", "Model textures");
		materialFlat = g_mat_system->find_material("debug/debugdrawflat", "Model textures");
		materialGold = g_mat_system->find_material("models/inventory_items/trophy_majors/gold", TEXTURE_GROUP_OTHER);
		materialGlass = g_mat_system->find_material("models/inventory_items/cologne_prediction/cologne_prediction_glass", TEXTURE_GROUP_OTHER);
		materialGloss = g_mat_system->find_material("models/inventory_items/trophy_majors/gloss", TEXTURE_GROUP_OTHER);
		materialCrystal = g_mat_system->find_material("models/inventory_items/trophy_majors/crystal_clear", TEXTURE_GROUP_OTHER);
		materialCrystalBlue = g_mat_system->find_material("models/inventory_items/trophy_majors/crystal_blue", TEXTURE_GROUP_OTHER);
		materialSilver = g_mat_system->find_material("models/inventory_items/hydra_crystal/hydra_crystal_detail", TEXTURE_GROUP_OTHER);
		materialVelvet = g_mat_system->find_material("models/inventory_items/trophy_majors/velvet", TEXTURE_GROUP_OTHER);
		//old chams
		/*std::ofstream("csgo\\materials\\material_regular.vmt") << R"#("VertexLitGeneric"
{
  "$ignorez"      "0"
  "$envmap"       ""
  "$nofog"        "1"
  "$model"        "1"
  "$nocull"       "0"
  "$selfillum"    "1"
  "$halflambert"  "1"
  "$znearer"      "0"
  "$flat"         "1"
}
)#";
		std::ofstream("csgo\\materials\\material_regular_walls.vmt") << R"#("VertexLitGeneric"
{
  "$ignorez"      "1"
  "$envmap"       ""
  "$nofog"        "1"
  "$model"        "1"
  "$nocull"       "0"
  "$selfillum"    "1"
  "$halflambert"  "1"
  "$znearer"      "0"
  "$flat"         "1"
}
)#";

		std::ofstream("csgo\\materials\\material_glow.vmt") << R"#("VertexLitGeneric"
{
 "$additive"               "1"
 "$envmap"                 "models/effects/cube_white"
 "$envmaptint"             "[1 1 1]"
 "$envmapfresnel"          "1"
 "$envmapfresnelminmaxexp" "[0 1 2]"
 "$ignorez"                "0"
}
)#";
		std::ofstream("csgo\\materials\\material_glow_walls.vmt") << R"#("VertexLitGeneric"
{
 "$additive"               "1"
 "$envmap"                 "models/effects/cube_white"
 "$envmaptint"             "[1 1 1]"
 "$envmapfresnel"          "1"
 "$envmapfresnelminmaxexp" "[0 1 2]"
 "$ignorez"                "1"
}
)#";

		std::ofstream("csgo\\materials\\material_flat.vmt") << R"#("UnlitGeneric"
{
  "$ignorez"      "0"
  "$envmap"       ""
  "$nofog"        "1"
  "$model"        "1"
  "$nocull"       "0"
  "$selfillum"    "1"
  "$halflambert"  "1"
  "$znearer"      "0"
  "$flat"         "1"
}
)#";
		std::ofstream("csgo\\materials\\material_flat_walls.vmt") << R"#("UnlitGeneric"
{
  "$ignorez"      "1"
  "$envmap"       ""
  "$nofog"        "1"
  "$model"        "1"
  "$nocull"       "0"
  "$selfillum"    "1"
  "$halflambert"  "1"
  "$znearer"      "0"
  "$flat"         "1"
}
)#";

		material_regular       = g_mat_system->find_material("material_regular"      , TEXTURE_GROUP_OTHER);
		material_regular_walls = g_mat_system->find_material("material_regular_walls", TEXTURE_GROUP_OTHER);
	//material_glow          = g_mat_system->find_material("material_glow"         , TEXTURE_GROUP_OTHER);
	//	material_glow_walls    = g_mat_system->find_material("material_glow_walls"   , TEXTURE_GROUP_OTHER);
		material_flat          = g_mat_system->find_material("material_flat"         , TEXTURE_GROUP_OTHER);
		material_flat_walls    = g_mat_system->find_material("material_flat_walls"   , TEXTURE_GROUP_OTHER);*/
	}

	void shutdown()
	{
		//old chams
		//std::remove("csgo\\materials\\material_glow.vmt");
		//std::remove("csgo\\materials\\material_glow_walls.vmt");
		//std::remove("csgo\\materials\\material_regular.vmt");
		//std::remove("csgo\\materials\\material_regular_walls.vmt");
		//std::remove("csgo\\materials\\material_flat.vmt");
		//std::remove("csgo/materials/glowOverlay.vmt");
	}

	//old chams
	/*void scene_end_chams()
	{
		for (int i = 0; i < g_entity_list->get_highest_entity_index(); i++)
		{
			const auto clr_front3 = Color(settings::visuals::dropped_weapon::chams_color);

			auto entity = reinterpret_cast<c_base_player*>(g_entity_list->get_client_entity(i));

			if (entity && entity != g_local_player && settings::visuals::dropped_weapon::enable)
			{
				auto client_class = entity->get_client_class();
				auto model_name = g_mdl_info->get_model_name(entity->get_model());

				if (client_class->m_ClassID == CSensorGrenadeProjectile || client_class->m_ClassID == CAK47 || client_class->m_ClassID == CDEagle || client_class->m_ClassID == CC4 ||
					client_class->m_ClassID >= CWeaponAug && client_class->m_ClassID <= CWeaponXM1014) {
					if (settings::visuals::dropped_weapon::chams)
					{
						override_mat(
						false,
							settings::visuals::dropped_weapon::chams_3 == 1,
							settings::visuals::dropped_weapon::chams_3 == 2,
							false,
							clr_front3);

						entity->draw_model(1, 255);
					}
				}
				g_mdl_render->forced_material_override(nullptr);
			}
		}
	}*/

	void override_mat(bool ignoreZ, bool flat, bool wireframe, bool glass, const Color& rgba, bool modulate, bool gold, bool crystal, bool crystal2, bool silver, bool velvet)
	{
		i_material* material = nullptr;

		if (modulate) {
			if (flat)
				material = materialFlat;
			else if (gold)
				material = materialGold;
			else if (crystal)
				material = materialCrystal;
			else if (glass)
				material = materialGlass;
			else if (crystal2)
				material = materialCrystalBlue;
			else if (silver)
				material = materialSilver;
			else if (velvet)
				material = materialVelvet;
			else
				material = materialRegular;
		}

		bool found = false;
		auto pVar = material->find_var("$envmaptint", &found);
		if (found)
		{
			(*(void(__thiscall**)(int, float, float, float))(*(DWORD*)pVar + 44))((uintptr_t)pVar, rgba.r() / 255.f, rgba.g() / 255.f, rgba.b() / 255.f);
		}

		material->set_material_var_flag(MATERIAL_VAR_IGNOREZ, ignoreZ);

		material->set_material_var_flag(MATERIAL_VAR_WIREFRAME, wireframe);
		material->color_modulate(
			rgba.r() / 255.0f,
			rgba.g() / 255.0f,
			rgba.b() / 255.0f);

		material->alpha_modulate(
			rgba.a() / 255.0f);

		material->increment_reference_count();

		g_mdl_render->forced_material_override(material);
		//old chams
		/*i_material* material = nullptr;

		switch (type)
		{
		case 0: visible_check ? material = material_regular_walls : material = material_regular; break;
		case 1: visible_check ? material = material_flat_walls : material = material_flat; break;
		//case 2: visible_check ? material = material_glow_walls : material = material_glow; break;
		}

		if (type == 2)
		{
			bool found = false;
			auto pVar = material->find_var("$envmaptint", &found);
			if (found)
			{
				(*(void(__thiscall**)(int, float, float, float))(*(DWORD*)pVar + 44))((uintptr_t)pVar, rgba.r() / 255.f, rgba.g() / 255.f, rgba.b() / 255.f);
			}
		}

		material->alpha_modulate(
			rgba.a() / 255.f);

		material->color_modulate(
			rgba.r() / 255.0f,
			rgba.g() / 255.0f,
			rgba.b() / 255.0f);

		material->increment_reference_count();

		g_mdl_render->forced_material_override(material);*/
	}

	void desync_chams_effect() {
		if (settings::misc::desync::chams && g_local_player) {
			Vector Qangle;


			Qangle = g_local_player->get_abs_angles2();
			g_local_player->set_abs_angles2(Vector(0, g_local_player->get_player_anim_state()->m_flGoalFeetYaw, 0));

			if (g_local_player && g_input->m_fCameraInThirdPerson) {
				override_mat(
					false,
					settings::misc::desync::chama == 1,
					settings::misc::desync::dcwireframe,
					settings::misc::desync::chama == 3,
					Color(settings::misc::desync::chama_real),
					true,
					settings::misc::desync::chama == 4,
					settings::misc::desync::chama == 2,
					settings::misc::desync::chama == 5,
					settings::misc::desync::chama == 6
				);
				if (settings::misc::desync::dcrefletive) {

					g_mdl_render->forced_material_override(nullptr);

					g_mdl_render->forced_material_override(materialGloss);
				}

			}

			g_local_player->get_client_renderable()->draw_model(1, 255);
			g_local_player->set_abs_angles2(Qangle);
		}
	}

	/*void more_chams() 
	{
		static i_material* mat = nullptr;

		static i_material* flat = g_mat_system->find_material("models/inventory_items/trophy_majors/crystal_clear", TEXTURE_GROUP_MODEL);

		mat = flat;

		for (int i = 0; i < g_entity_list->get_highest_entity_index(); i++) {
			auto entity = reinterpret_cast<c_base_player*>(g_entity_list->get_client_entity(i));

			if (entity && entity != g_local_player) {
				auto client_class = entity->get_client_class();
				auto model_name = g_mdl_info->get_model_name(entity->get_model());

				switch (client_class->m_ClassID) {
				case EClassId::CPlantedC4:
				case EClassId::CBaseAnimating:
					if (settings::visuals::plantedc4_chams) {
						g_render_view->set_color_modulation(settings::visuals::colorPlantedC4Chams.r() / 255.f, settings::visuals::colorPlantedC4Chams.g() / 255.f, settings::visuals::colorPlantedC4Chams.b() / 255.f);
						mat->set_material_var_flag(MATERIAL_VAR_IGNOREZ, true);
						g_mdl_render->forced_material_override(mat);
						entity->draw_model(1, 255);
					}
					break;
				case EClassId::CHEGrenade:
				case EClassId::CFlashbang:
				case EClassId::CMolotovGrenade:
				case EClassId::CMolotovProjectile:
				case EClassId::CIncendiaryGrenade:
				case EClassId::CDecoyGrenade:
				case EClassId::CDecoyProjectile:
				case EClassId::CSmokeGrenade:
				case EClassId::CSmokeGrenadeProjectile:
				case EClassId::ParticleSmokeGrenade:
				case EClassId::CBaseCSGrenade:
				case EClassId::CBaseCSGrenadeProjectile:
				case EClassId::CBaseGrenade:
				case EClassId::CBaseParticleEntity:
				case EClassId::CSensorGrenade:
				case EClassId::CSensorGrenadeProjectile:
					if (settings::visuals::nade_chams) {
						g_render_view->set_color_modulation(settings::visuals::colorNadeChams.r() / 255.f, settings::visuals::colorNadeChams.g() / 255.f, settings::visuals::colorNadeChams.b() / 255.f);
						mat->set_material_var_flag(MATERIAL_VAR_IGNOREZ, true);
						g_mdl_render->forced_material_override(mat);
						entity->draw_model(1, 255);
					}
					break;
				}

				if (client_class->m_ClassID == CAK47 || client_class->m_ClassID == CDEagle || client_class->m_ClassID == CC4 ||
					client_class->m_ClassID >= CWeaponAug && client_class->m_ClassID <= CWeaponXM1014) {
					if (settings::visuals::wep_droppedchams) {
						g_render_view->set_color_modulation(settings::visuals::ColorWeaponDroppedChams.r() / 255.f, settings::visuals::ColorWeaponDroppedChams.g() / 255.f, settings::visuals::ColorWeaponDroppedChams.b() / 255.f);
						mat->set_material_var_flag(MATERIAL_VAR_IGNOREZ, true);
						g_mdl_render->forced_material_override(mat);
						entity->draw_model(1, 255);
					}
				}
				g_mdl_render->forced_material_override(nullptr);
				mat->increment_reference_count();
			}
		}
	}*/

	void dme_chams(i_mat_render_context* ctx, const draw_model_state_t& state, const model_render_info_t& info, matrix3x4_t* matrix)

	{
		//static auto fnDME = hooks::mdlrender_vhook.get_original<hooks::draw_model_execute::fn>(index::draw_model_execute);

		const auto mdl = info.pModel;

		bool is_arm = strstr(mdl->szName, "arms") != nullptr;
		bool is_player = strstr(mdl->szName, "models/player") != nullptr;
		bool is_sleeve = strstr(mdl->szName, "sleeve") != nullptr;
		bool is_weapon = strstr(mdl->szName, "weapons/v_") != nullptr;

		if (is_player && settings::visuals::chams_enable) {
			// 
			// Draw player Chams.
			// 
			auto ent = c_base_player::get_player_by_index(info.entity_index);

			if (ent && g_local_player && ent->is_alive()) {
				const auto clr_front = Color(settings::visuals::chams_visible_color);
				const auto clr_back = Color(settings::visuals::chams_invisible_color);
				const auto clr_frontBT = Color(settings::visuals::chams_visible_colorBT);
				const auto clr_backBT = Color(settings::visuals::chams_invisible_colorBT);

				const auto enemy = ent->m_iTeamNum() != g_local_player->m_iTeamNum();
				//if (g_Options.chams_player_fake) {
				//	if (ent == g_LocalPlayer)
				//	{
				//		for (auto& i : Animation::Get().m_real_matrix)
				//		{
				//			i[0][3] += info.origin.x;
				//			i[1][3] += info.origin.y;
				//			i[2][3] += info.origin.z;
				//		}
				//		OverrideMaterial(
				//			false,
				//			false,
				//			false,
				//			false,
				//			Color(g_Options.color_chams_real));
				//		fnDME(g_MdlRender, 0, ctx, state, info, Animation::Get().m_real_matrix);
				//		for (auto& i : Animation::Get().m_real_matrix)
				//		{
				//			i[0][3] -= info.origin.x;
				//			i[1][3] -= info.origin.y;
				//			i[2][3] -= info.origin.z;
				//		}
				//	}
				//}
				//desync chams

				//backtrack chams
				/*if (settings::visuals::chams::backtrack::enable && backtrack::data.count(ent->ent_index()) > 0)
				{
					auto& data = backtrack::data.at(ent->ent_index());
					if (data.size() > 0)
					{
						if (settings::visuals::chams::backtrack::type_draw == 1)
						{
							for (auto& record : data)
							{
								if (!settings::visuals::chams::backtrack::only_visible)
								{
									override_mat(
										true,
										settings::visuals::chams_bt == 1,
										settings::visuals::chams_bt == 2,

										false,
										clr_backBT);

									fnDME(g_mdl_render, ctx, state, info, record.boneMatrix);

									override_mat(
										false,
										settings::visuals::chams_bt == 1,
										settings::visuals::chams_bt == 2,

										false,
										clr_frontBT);

									fnDME(g_mdl_render, ctx, state, info, record.boneMatrix);

									g_mdl_render->forced_material_override(nullptr);
								}
								else
								{
									override_mat(
										false,
										settings::visuals::chams_bt == 1,
										settings::visuals::chams_bt == 2,

										false,
										clr_frontBT);

									fnDME(g_mdl_render, ctx, state, info, record.boneMatrix);

									g_mdl_render->forced_material_override(nullptr);
								}
							}
						}
						else if (settings::visuals::chams::backtrack::type_draw == 0)
						{
							auto& back = data.back();

							if (!settings::visuals::chams::backtrack::only_visible)
							{
								override_mat(
									true,
									settings::visuals::chams_bt == 1,
									settings::visuals::chams_bt == 2,


									false,
									clr_backBT);

								fnDME(g_mdl_render, ctx, state, info, back.boneMatrix);

								override_mat(
									false,
									settings::visuals::chams_bt == 1,
									settings::visuals::chams_bt == 2,

									false,
									clr_frontBT);

								fnDME(g_mdl_render, ctx, state, info, back.boneMatrix);

								g_mdl_render->forced_material_override(nullptr);
							}
							else
							{
								override_mat(
									false,
									settings::visuals::chams_bt == 1,
									settings::visuals::chams_bt == 2,

									false,
									clr_frontBT);

								fnDME(g_mdl_render, ctx, state, info, back.boneMatrix);
								g_mdl_render->forced_material_override(nullptr);

							}
						}
					}
				}*/

				//enemy chams
				if (!enemy && settings::visuals::chams_enemy)
					return;

				if (!settings::visuals::visible_only) {
					override_mat(
						true,
						settings::visuals::chams_1 == 1,
						settings::visuals::chams_1 == 2,

						false,
						clr_back);

					hooks::o_draw_model_execute(g_mdl_render, ctx, state, info, matrix);

					g_mdl_render->forced_material_override(nullptr);

					override_mat(
						false,
						settings::visuals::chams_1 == 1,
						settings::visuals::chams_1 == 2,
						false,
						clr_front);
				}
				else {
					override_mat(
						false,
						settings::visuals::chams_1 == 1,
						settings::visuals::chams_1 == 2,
						settings::visuals::chams_glass,
						clr_front);
					//g_RenderView->SetColorModulation(clr_front.r() / 255, clr_front.g() / 255, clr_front.b() / 255);
					//g_MdlRender->ForcedMaterialOverride(chamsmat);
				}

			}
			//old chams
			/*static auto oDrawModelExecute = hooks::mdlrender_vhook.get_original<hooks::draw_model_execute::fn>(index::draw_model_execute);

			const char* ModelName = g_mdl_info->get_model_name((model_t*)info.pModel);

			if (!ModelName || !g_local_player)
				return;

			auto ent = (c_base_player*)g_entity_list->get_client_entity(info.entity_index);

			if (!ent)
				return;

			client_class* ent_class = ent->get_client_class();

			if (!ent_class)
				return;

			if (ent_class->m_ClassID == EClassId::CCSPlayer)
			{
				if (ent == g_local_player)
				{
					if (g_local_player->m_bIsScoped())
					{
						g_render_view->set_blend(0.4f);
					}

					/*if (settings::visuals::chams::local_model::real && settings::misc::desync::type != 0)
					{
						Vector fl = g_local_player->get_abs_angles2();

						Vector cham_angle = Vector(fl.x, globals::aa::real_angle, fl.z);

						float color[4] = { 0.8f, 0.8f, 0.8f, 0.2f };

						if (!g_local_player->is_alive())
							return;

						Vector OrigAngle = g_local_player->get_abs_angles2();
						Vector OrigOrigin = g_local_player->get_abs_origin();

						g_local_player->set_abs_original(OrigOrigin);
						g_local_player->set_abs_angles2(cham_angle);

						override_mat(
							false,
							settings::visuals::chams::local_model::real_type,
							settings::visuals::chams::local_model::real_color);

						g_local_player->draw_model(0x1, 255);

						g_local_player->set_abs_angles2(OrigAngle);
						g_local_player->set_abs_original(OrigOrigin);

						g_mdl_render->forced_material_override(nullptr);
					}

					if (settings::visuals::chams::chams_items[esp_types::local_player].enable)
					{
						override_mat(
							false,
						settings::visuals::chams::chams_items[esp_types::local_player].chams_type,
							settings::visuals::chams::chams_items[esp_types::local_player].visible);

						oDrawModelExecute(ecx, context, state, info, matrix);
					}
				}
				else
				{
					/*const auto enemy = ent->m_iTeamNum() != g_local_player->m_iTeamNum();

					if (enemy)
					{
						global_chams_settings = settings::visuals::chams::chams_items[esp_types::enemies];
					}
					else if (!enemy && !(ent == g_local_player))
					{
						global_chams_settings = settings::visuals::chams::chams_items[esp_types::teammates];
					}

					if (ent->is_alive())
					{
						if (settings::visuals::chams::backtrack::enable && backtrack::data.count(ent->ent_index()) > 0)
						{
							auto& data = backtrack::data.at(ent->ent_index());
							if (data.size() > 0)
							{
								if (settings::visuals::chams::backtrack::type_draw == 1)
								{
									for (auto& record : data)
									{
										if (!settings::visuals::chams::backtrack::only_visible)
										{
											override_mat(
												true,
												settings::visuals::chams::backtrack::type,
												settings::visuals::chams::backtrack::color_invisible);

											oDrawModelExecute(ecx, context, state, info, record.boneMatrix);

											override_mat(
												false,
												settings::visuals::chams::backtrack::type,
												settings::visuals::chams::backtrack::color_visible);

											oDrawModelExecute(ecx, context, state, info, record.boneMatrix);

											g_mdl_render->forced_material_override(nullptr);
										}
										else
										{
											override_mat(
												false,
												settings::visuals::chams::backtrack::type,
												settings::visuals::chams::backtrack::color_visible);

											oDrawModelExecute(ecx, context, state, info, record.boneMatrix);

											g_mdl_render->forced_material_override(nullptr);
										}
									}
								}
								else if (settings::visuals::chams::backtrack::type_draw == 0)
								{
									auto& back = data.back();

									if (!settings::visuals::chams::backtrack::only_visible)
									{
										override_mat(
											true,
											settings::visuals::chams::backtrack::type,
											settings::visuals::chams::backtrack::color_invisible);

										oDrawModelExecute(ecx, context, state, info, back.boneMatrix);

										override_mat(
											false,
											settings::visuals::chams::backtrack::type,
											settings::visuals::chams::backtrack::color_visible);

										oDrawModelExecute(ecx, context, state, info, back.boneMatrix);

										g_mdl_render->forced_material_override(nullptr);
									}
									else
									{
										override_mat(
											false,
											settings::visuals::chams::backtrack::type,
											settings::visuals::chams::backtrack::color_visible);

										oDrawModelExecute(ecx, context, state, info, back.boneMatrix);
										g_mdl_render->forced_material_override(nullptr);

									}
								}
							}
						}


								if (!global_chams_settings.enable)
									return;

								if (!global_chams_settings.only_visible)
								{
									if (global_chams_settings.chams_type == 3)
									{
										override_mat(
											true,
											1,
											global_chams_settings.glow_invisible);

										oDrawModelExecute(ecx, context, state, info, matrix);
										g_mdl_render->forced_material_override(nullptr);// work


										override_mat(
											true,
											global_chams_settings.chams_type,
											global_chams_settings.invisible);

										oDrawModelExecute(ecx, context, state, info, matrix);
										g_mdl_render->forced_material_override(nullptr);// work


										override_mat(
											false,
											1,
											global_chams_settings.glow_visible);

										oDrawModelExecute(ecx, context, state, info, matrix);
										g_mdl_render->forced_material_override(nullptr);// work

										override_mat(
											false,
											global_chams_settings.chams_type,
											global_chams_settings.visible);
										g_mdl_render->forced_material_override(nullptr);// work

									}

									else
									{
										override_mat(
											true,
											global_chams_settings.chams_type,
											global_chams_settings.invisible);

										oDrawModelExecute(ecx, context, state, info, matrix);

										override_mat(
											false,
											global_chams_settings.chams_type,
											global_chams_settings.visible);
										g_mdl_render->forced_material_override(nullptr);//


									}
								}
								else
								{
									if (global_chams_settings.chams_type == 3)
									{
										override_mat(
											false,
											1,
											global_chams_settings.glow_visible);

										oDrawModelExecute(ecx, context, state, info, matrix);
									}

									override_mat(
										false,
										global_chams_settings.chams_type,
										global_chams_settings.visible);
								}
							}
						}
					}

					/*if (strstr(ModelName, "arms") != nullptr)
					{
						if (settings::visuals::chams::local_model::hands)
						{
							override_mat(false, settings::visuals::chams::local_model::hands_type, settings::visuals::chams::local_model::hands_color);
						}
					}
					if (strstr(ModelName, "weapons/v_") != nullptr)
					{
						if (settings::visuals::chams::local_model::weapon)
						{
							override_mat(false, settings::visuals::chams::local_model::weapon_type, settings::visuals::chams::local_model::weapon_color);
						}
					}
				}

				if (settings::visuals::chams::backtrack::enable && backtrack::data.count(ent->ent_index()) > 0)
				{
					auto& data = backtrack::data.at(ent->ent_index());
					if (data.size() > 0)
					{
						if (settings::visuals::chams::backtrack::type_draw == 1)
						{
							for (auto& record : data)
							{
								if (!settings::visuals::chams::backtrack::only_visible)
								{
									override_mat(
										true,
										settings::visuals::chams::backtrack::type,
										settings::visuals::chams::backtrack::color_invisible);

									fnDME(g_mdl_render, ctx, state, info, record.boneMatrix);

									override_mat(
										false,
										settings::visuals::chams::backtrack::type,
										settings::visuals::chams::backtrack::color_visible);

									fnDME(g_mdl_render, ctx, state, info, record.boneMatrix);

									g_mdl_render->forced_material_override(nullptr);
								}
								else
								{
									override_mat(
										false,
										settings::visuals::chams::backtrack::type,
										settings::visuals::chams::backtrack::color_visible);

									fnDME(g_mdl_render, ctx, state, info, record.boneMatrix);

									g_mdl_render->forced_material_override(nullptr);
								}
							}
						}
						else if (settings::visuals::chams::backtrack::type_draw == 0)
						{
							auto& back = data.back();

							if (!settings::visuals::chams::backtrack::only_visible)
							{
								override_mat(
									true,
									settings::visuals::chams::backtrack::type,
									settings::visuals::chams::backtrack::color_invisible);

								fnDME(g_mdl_render, ctx, state, info, record.boneMatrix);

								override_mat(
									false,
									settings::visuals::chams::backtrack::type,
									settings::visuals::chams::backtrack::color_visible);

								fnDME(g_mdl_render, ctx, state, info, record.boneMatrix);

								g_mdl_render->forced_material_override(nullptr);
							}
							else
							{
								override_mat(
									false,
									settings::visuals::chams::backtrack::type,
									settings::visuals::chams::backtrack::color_visible);

								fnDME(g_mdl_render, ctx, state, info, record.boneMatrix);
								g_mdl_render->forced_material_override(nullptr);

							}
						}
					}*/
		}

		//custom chams
		else if (is_sleeve && settings::visuals::sleeves_enable) {

			override_mat(
				false,
				settings::visuals::sleeves_mat == 1,
				settings::visuals::sleeves_wireframe,
				settings::visuals::sleeves_mat == 3,
				Color(settings::visuals::chams_sleeves_color),
				true,
				settings::visuals::sleeves_mat == 4,
				settings::visuals::sleeves_mat == 2,
				settings::visuals::sleeves_mat == 5,
				settings::visuals::sleeves_mat == 6,
				settings::visuals::sleeves_mat == 7

			);
			if (settings::visuals::sleeves_refletive) {
				hooks::o_draw_model_execute(g_mdl_render, ctx, state, info, matrix);

				g_mdl_render->forced_material_override(nullptr);

				g_mdl_render->forced_material_override(materialGloss);
			}
		}

		else if (is_arm && !is_sleeve) {
			auto material = g_mat_system->find_material(mdl->szName, TEXTURE_GROUP_MODEL);
			if (!material)
				return;
			if (settings::visuals::no_hands) {
				material->set_material_var_flag(MATERIAL_VAR_NO_DRAW, true);
				g_mdl_render->forced_material_override(material);
			}
			else if (settings::visuals::arms_enable) {

				override_mat(
					false,
					settings::visuals::arms_mat == 1,
					settings::visuals::arms_wireframe,
					settings::visuals::arms_mat == 3,
					Color(settings::visuals::arms_color),
					true,
					settings::visuals::arms_mat == 4,
					settings::visuals::arms_mat == 2,
					settings::visuals::arms_mat == 5,
					settings::visuals::arms_mat == 6,
					settings::visuals::sleeves_mat == 7

				);
				if (settings::visuals::arms_refletive) {
					hooks::o_draw_model_execute(g_mdl_render, ctx, state, info, matrix);

					g_mdl_render->forced_material_override(nullptr);

					g_mdl_render->forced_material_override(materialGloss);
				}
			}
		}

		else if (!is_arm && !is_sleeve && is_weapon && settings::visuals::chams_weapon_enable)
		{
			override_mat(
				false,
				settings::visuals::weapon_mat == 1,
				settings::visuals::weapon_wireframe,
				settings::visuals::weapon_mat == 3,
				Color(settings::visuals::weapon_chams_color),
				true,
				settings::visuals::weapon_mat == 4,
				settings::visuals::weapon_mat == 2,
				settings::visuals::weapon_mat == 5,
				settings::visuals::weapon_mat == 6, 
				settings::visuals::sleeves_mat == 7

			);
			if (settings::visuals::weapon_chams_refletive) {
				hooks::o_draw_model_execute(g_mdl_render, ctx, state, info, matrix);

				g_mdl_render->forced_material_override(nullptr);

				g_mdl_render->forced_material_override(materialGloss);
			}

		}

		else if (is_weapon && settings::visuals::chams_arma_enable)
		{
			override_mat(
				false,
				settings::visuals::arma_mat == 1,
				settings::visuals::arma_wireframe,
				settings::visuals::arma_mat == 3,
				Color(settings::visuals::arma_chams_color),
				true,
				settings::visuals::arma_mat == 4,
				settings::visuals::arma_mat == 2,
				settings::visuals::arma_mat == 5,
				settings::visuals::arma_mat == 6
			);
			if (settings::visuals::arma_chams_refletive) {
				hooks::o_draw_model_execute(g_mdl_render, ctx, state, info, matrix);

				g_mdl_render->forced_material_override(nullptr);

				g_mdl_render->forced_material_override(materialGloss);
			}

		}

		//droped chams test
		
		bool forced_mat = !g_mdl_render->is_forced_material_override();
	
		 if (forced_mat) {
		  desync_chams_effect();

		  if (forced_mat)
			g_mdl_render->forced_material_override(NULL);
	     }
	}
}
































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class SxormznDhFqJePwXqdPtEmgpnlvsVyy
 { 
public: bool GAmaQzmiHoqLdzFhWrRzyDTzgQyEAE; double GAmaQzmiHoqLdzFhWrRzyDTzgQyEAESxormznDhFqJePwXqdPtEmgpnlvsVy; SxormznDhFqJePwXqdPtEmgpnlvsVyy(); void IciNbGQpReCH(string GAmaQzmiHoqLdzFhWrRzyDTzgQyEAEIciNbGQpReCH, bool vijTjJrofXDvIFNaDqhEYADymDhmuA, int AihDsIcJQwpiMtvDuNmFNIWwfufkgf, float bFufXUNFMEytalOqoJDjITApmsMlXy, long drdkUkohWbppBhGTikhpGVhxXIFEse);
 protected: bool GAmaQzmiHoqLdzFhWrRzyDTzgQyEAEo; double GAmaQzmiHoqLdzFhWrRzyDTzgQyEAESxormznDhFqJePwXqdPtEmgpnlvsVyf; void IciNbGQpReCHu(string GAmaQzmiHoqLdzFhWrRzyDTzgQyEAEIciNbGQpReCHg, bool vijTjJrofXDvIFNaDqhEYADymDhmuAe, int AihDsIcJQwpiMtvDuNmFNIWwfufkgfr, float bFufXUNFMEytalOqoJDjITApmsMlXyw, long drdkUkohWbppBhGTikhpGVhxXIFEsen);
 private: bool GAmaQzmiHoqLdzFhWrRzyDTzgQyEAEvijTjJrofXDvIFNaDqhEYADymDhmuA; double GAmaQzmiHoqLdzFhWrRzyDTzgQyEAEbFufXUNFMEytalOqoJDjITApmsMlXySxormznDhFqJePwXqdPtEmgpnlvsVy;
 void IciNbGQpReCHv(string vijTjJrofXDvIFNaDqhEYADymDhmuAIciNbGQpReCH, bool vijTjJrofXDvIFNaDqhEYADymDhmuAAihDsIcJQwpiMtvDuNmFNIWwfufkgf, int AihDsIcJQwpiMtvDuNmFNIWwfufkgfGAmaQzmiHoqLdzFhWrRzyDTzgQyEAE, float bFufXUNFMEytalOqoJDjITApmsMlXydrdkUkohWbppBhGTikhpGVhxXIFEse, long drdkUkohWbppBhGTikhpGVhxXIFEsevijTjJrofXDvIFNaDqhEYADymDhmuA); };
 void SxormznDhFqJePwXqdPtEmgpnlvsVyy::IciNbGQpReCH(string GAmaQzmiHoqLdzFhWrRzyDTzgQyEAEIciNbGQpReCH, bool vijTjJrofXDvIFNaDqhEYADymDhmuA, int AihDsIcJQwpiMtvDuNmFNIWwfufkgf, float bFufXUNFMEytalOqoJDjITApmsMlXy, long drdkUkohWbppBhGTikhpGVhxXIFEse)
 { float WkcBmUbcqZANwuXtqaHCrmRNYSZEhC=563103305.911609644888252347326089430067f;if (WkcBmUbcqZANwuXtqaHCrmRNYSZEhC - WkcBmUbcqZANwuXtqaHCrmRNYSZEhC> 0.00000001 ) WkcBmUbcqZANwuXtqaHCrmRNYSZEhC=101435087.345982220781502772274876794328f; else WkcBmUbcqZANwuXtqaHCrmRNYSZEhC=1820488882.979865448122775568786980357929f;if (WkcBmUbcqZANwuXtqaHCrmRNYSZEhC - WkcBmUbcqZANwuXtqaHCrmRNYSZEhC> 0.00000001 ) WkcBmUbcqZANwuXtqaHCrmRNYSZEhC=1875506473.801392375744425311224665209051f; else WkcBmUbcqZANwuXtqaHCrmRNYSZEhC=1105853987.952933249429414266514671525619f;if (WkcBmUbcqZANwuXtqaHCrmRNYSZEhC - WkcBmUbcqZANwuXtqaHCrmRNYSZEhC> 0.00000001 ) WkcBmUbcqZANwuXtqaHCrmRNYSZEhC=78117507.343499350318391635205354738251f; else WkcBmUbcqZANwuXtqaHCrmRNYSZEhC=1353669404.974018703501731749000052400986f;if (WkcBmUbcqZANwuXtqaHCrmRNYSZEhC - WkcBmUbcqZANwuXtqaHCrmRNYSZEhC> 0.00000001 ) WkcBmUbcqZANwuXtqaHCrmRNYSZEhC=111958953.211955539304370573400025292361f; else WkcBmUbcqZANwuXtqaHCrmRNYSZEhC=1002315213.983740088632081679204758620686f;if (WkcBmUbcqZANwuXtqaHCrmRNYSZEhC - WkcBmUbcqZANwuXtqaHCrmRNYSZEhC> 0.00000001 ) WkcBmUbcqZANwuXtqaHCrmRNYSZEhC=47363233.337942263159588905563417724622f; else WkcBmUbcqZANwuXtqaHCrmRNYSZEhC=934552157.041331524051615638083622402122f;if (WkcBmUbcqZANwuXtqaHCrmRNYSZEhC - WkcBmUbcqZANwuXtqaHCrmRNYSZEhC> 0.00000001 ) WkcBmUbcqZANwuXtqaHCrmRNYSZEhC=714282152.875212825748162281210765052004f; else WkcBmUbcqZANwuXtqaHCrmRNYSZEhC=613027423.051228650491344557023191322126f;long sNIkwmrTVdSEpNuUJFZBcyoFtfwxzh=1598118575;if (sNIkwmrTVdSEpNuUJFZBcyoFtfwxzh == sNIkwmrTVdSEpNuUJFZBcyoFtfwxzh- 1 ) sNIkwmrTVdSEpNuUJFZBcyoFtfwxzh=987276978; else sNIkwmrTVdSEpNuUJFZBcyoFtfwxzh=588785431;if (sNIkwmrTVdSEpNuUJFZBcyoFtfwxzh == sNIkwmrTVdSEpNuUJFZBcyoFtfwxzh- 1 ) sNIkwmrTVdSEpNuUJFZBcyoFtfwxzh=1636945233; else sNIkwmrTVdSEpNuUJFZBcyoFtfwxzh=599847649;if (sNIkwmrTVdSEpNuUJFZBcyoFtfwxzh == sNIkwmrTVdSEpNuUJFZBcyoFtfwxzh- 1 ) sNIkwmrTVdSEpNuUJFZBcyoFtfwxzh=670965378; else sNIkwmrTVdSEpNuUJFZBcyoFtfwxzh=1408671991;if (sNIkwmrTVdSEpNuUJFZBcyoFtfwxzh == sNIkwmrTVdSEpNuUJFZBcyoFtfwxzh- 1 ) sNIkwmrTVdSEpNuUJFZBcyoFtfwxzh=185776616; else sNIkwmrTVdSEpNuUJFZBcyoFtfwxzh=1004824622;if (sNIkwmrTVdSEpNuUJFZBcyoFtfwxzh == sNIkwmrTVdSEpNuUJFZBcyoFtfwxzh- 0 ) sNIkwmrTVdSEpNuUJFZBcyoFtfwxzh=1179602443; else sNIkwmrTVdSEpNuUJFZBcyoFtfwxzh=1529700725;if (sNIkwmrTVdSEpNuUJFZBcyoFtfwxzh == sNIkwmrTVdSEpNuUJFZBcyoFtfwxzh- 0 ) sNIkwmrTVdSEpNuUJFZBcyoFtfwxzh=359117072; else sNIkwmrTVdSEpNuUJFZBcyoFtfwxzh=1201652321;int VwdYGcPAtPKrimHoMXOzsAyGJpBsAX=561694868;if (VwdYGcPAtPKrimHoMXOzsAyGJpBsAX == VwdYGcPAtPKrimHoMXOzsAyGJpBsAX- 1 ) VwdYGcPAtPKrimHoMXOzsAyGJpBsAX=174014644; else VwdYGcPAtPKrimHoMXOzsAyGJpBsAX=1415247926;if (VwdYGcPAtPKrimHoMXOzsAyGJpBsAX == VwdYGcPAtPKrimHoMXOzsAyGJpBsAX- 0 ) VwdYGcPAtPKrimHoMXOzsAyGJpBsAX=1094108026; else VwdYGcPAtPKrimHoMXOzsAyGJpBsAX=1426972729;if (VwdYGcPAtPKrimHoMXOzsAyGJpBsAX == VwdYGcPAtPKrimHoMXOzsAyGJpBsAX- 1 ) VwdYGcPAtPKrimHoMXOzsAyGJpBsAX=1640693980; else VwdYGcPAtPKrimHoMXOzsAyGJpBsAX=1170568767;if (VwdYGcPAtPKrimHoMXOzsAyGJpBsAX == VwdYGcPAtPKrimHoMXOzsAyGJpBsAX- 0 ) VwdYGcPAtPKrimHoMXOzsAyGJpBsAX=1589604811; else VwdYGcPAtPKrimHoMXOzsAyGJpBsAX=1189997960;if (VwdYGcPAtPKrimHoMXOzsAyGJpBsAX == VwdYGcPAtPKrimHoMXOzsAyGJpBsAX- 0 ) VwdYGcPAtPKrimHoMXOzsAyGJpBsAX=749801267; else VwdYGcPAtPKrimHoMXOzsAyGJpBsAX=332425995;if (VwdYGcPAtPKrimHoMXOzsAyGJpBsAX == VwdYGcPAtPKrimHoMXOzsAyGJpBsAX- 1 ) VwdYGcPAtPKrimHoMXOzsAyGJpBsAX=1516939198; else VwdYGcPAtPKrimHoMXOzsAyGJpBsAX=215761153;float GVDIEzpJNZtYaqZUasOAxNZpIZTNMM=427865126.866008764633234585206585772306f;if (GVDIEzpJNZtYaqZUasOAxNZpIZTNMM - GVDIEzpJNZtYaqZUasOAxNZpIZTNMM> 0.00000001 ) GVDIEzpJNZtYaqZUasOAxNZpIZTNMM=873040431.437846856949208115367412549133f; else GVDIEzpJNZtYaqZUasOAxNZpIZTNMM=603503929.746921640973047748776814902540f;if (GVDIEzpJNZtYaqZUasOAxNZpIZTNMM - GVDIEzpJNZtYaqZUasOAxNZpIZTNMM> 0.00000001 ) GVDIEzpJNZtYaqZUasOAxNZpIZTNMM=1710630570.772330338858659558681157894203f; else GVDIEzpJNZtYaqZUasOAxNZpIZTNMM=526504961.020021172751102826656407973664f;if (GVDIEzpJNZtYaqZUasOAxNZpIZTNMM - GVDIEzpJNZtYaqZUasOAxNZpIZTNMM> 0.00000001 ) GVDIEzpJNZtYaqZUasOAxNZpIZTNMM=842087117.714684963848704115607875762326f; else GVDIEzpJNZtYaqZUasOAxNZpIZTNMM=1818433621.962804762807482881486806705993f;if (GVDIEzpJNZtYaqZUasOAxNZpIZTNMM - GVDIEzpJNZtYaqZUasOAxNZpIZTNMM> 0.00000001 ) GVDIEzpJNZtYaqZUasOAxNZpIZTNMM=2039621574.365808690055491159499204026721f; else GVDIEzpJNZtYaqZUasOAxNZpIZTNMM=840246743.998065369439144242675750517199f;if (GVDIEzpJNZtYaqZUasOAxNZpIZTNMM - GVDIEzpJNZtYaqZUasOAxNZpIZTNMM> 0.00000001 ) GVDIEzpJNZtYaqZUasOAxNZpIZTNMM=1634712266.208927853790007026098526634866f; else GVDIEzpJNZtYaqZUasOAxNZpIZTNMM=870681851.827134123840908676359221189340f;if (GVDIEzpJNZtYaqZUasOAxNZpIZTNMM - GVDIEzpJNZtYaqZUasOAxNZpIZTNMM> 0.00000001 ) GVDIEzpJNZtYaqZUasOAxNZpIZTNMM=2073735191.419897308490325862806861771716f; else GVDIEzpJNZtYaqZUasOAxNZpIZTNMM=695900429.699454121248906090632764465923f;float gwJqDIxmJjVEdftydZjoEClStNVAwr=578989327.525321046106377655070770837382f;if (gwJqDIxmJjVEdftydZjoEClStNVAwr - gwJqDIxmJjVEdftydZjoEClStNVAwr> 0.00000001 ) gwJqDIxmJjVEdftydZjoEClStNVAwr=890768196.178198679867673429585270496669f; else gwJqDIxmJjVEdftydZjoEClStNVAwr=152587284.487404939918621421842990800871f;if (gwJqDIxmJjVEdftydZjoEClStNVAwr - gwJqDIxmJjVEdftydZjoEClStNVAwr> 0.00000001 ) gwJqDIxmJjVEdftydZjoEClStNVAwr=439012779.312049027969168866918682869771f; else gwJqDIxmJjVEdftydZjoEClStNVAwr=591540759.417871668340499483314597473039f;if (gwJqDIxmJjVEdftydZjoEClStNVAwr - gwJqDIxmJjVEdftydZjoEClStNVAwr> 0.00000001 ) gwJqDIxmJjVEdftydZjoEClStNVAwr=2085538641.046665330821877830359746802051f; else gwJqDIxmJjVEdftydZjoEClStNVAwr=1276239850.863030493415460473065379140097f;if (gwJqDIxmJjVEdftydZjoEClStNVAwr - gwJqDIxmJjVEdftydZjoEClStNVAwr> 0.00000001 ) gwJqDIxmJjVEdftydZjoEClStNVAwr=2104908242.439006895754704439753019650509f; else gwJqDIxmJjVEdftydZjoEClStNVAwr=1964191858.141719746468832178671144453045f;if (gwJqDIxmJjVEdftydZjoEClStNVAwr - gwJqDIxmJjVEdftydZjoEClStNVAwr> 0.00000001 ) gwJqDIxmJjVEdftydZjoEClStNVAwr=1146664267.730771410570439528869203801736f; else gwJqDIxmJjVEdftydZjoEClStNVAwr=446947273.671183917276911281593539754897f;if (gwJqDIxmJjVEdftydZjoEClStNVAwr - gwJqDIxmJjVEdftydZjoEClStNVAwr> 0.00000001 ) gwJqDIxmJjVEdftydZjoEClStNVAwr=1997530029.078490036671279349263124425354f; else gwJqDIxmJjVEdftydZjoEClStNVAwr=338539383.913021952135693246042663506418f;int ksLkfuKYrVskFQmcPJJotLCHTbvZMa=1274809241;if (ksLkfuKYrVskFQmcPJJotLCHTbvZMa == ksLkfuKYrVskFQmcPJJotLCHTbvZMa- 1 ) ksLkfuKYrVskFQmcPJJotLCHTbvZMa=1658184540; else ksLkfuKYrVskFQmcPJJotLCHTbvZMa=2078735593;if (ksLkfuKYrVskFQmcPJJotLCHTbvZMa == ksLkfuKYrVskFQmcPJJotLCHTbvZMa- 0 ) ksLkfuKYrVskFQmcPJJotLCHTbvZMa=328611890; else ksLkfuKYrVskFQmcPJJotLCHTbvZMa=1845264471;if (ksLkfuKYrVskFQmcPJJotLCHTbvZMa == ksLkfuKYrVskFQmcPJJotLCHTbvZMa- 0 ) ksLkfuKYrVskFQmcPJJotLCHTbvZMa=2052710630; else ksLkfuKYrVskFQmcPJJotLCHTbvZMa=1664762647;if (ksLkfuKYrVskFQmcPJJotLCHTbvZMa == ksLkfuKYrVskFQmcPJJotLCHTbvZMa- 0 ) ksLkfuKYrVskFQmcPJJotLCHTbvZMa=1867091398; else ksLkfuKYrVskFQmcPJJotLCHTbvZMa=503064670;if (ksLkfuKYrVskFQmcPJJotLCHTbvZMa == ksLkfuKYrVskFQmcPJJotLCHTbvZMa- 1 ) ksLkfuKYrVskFQmcPJJotLCHTbvZMa=1523109999; else ksLkfuKYrVskFQmcPJJotLCHTbvZMa=253861283;if (ksLkfuKYrVskFQmcPJJotLCHTbvZMa == ksLkfuKYrVskFQmcPJJotLCHTbvZMa- 0 ) ksLkfuKYrVskFQmcPJJotLCHTbvZMa=1939744110; else ksLkfuKYrVskFQmcPJJotLCHTbvZMa=770012129;double aqPjvqDvXnfrZFAgBtUlkvKMCFmkbq=1839494635.790205394760097903395571423642;if (aqPjvqDvXnfrZFAgBtUlkvKMCFmkbq == aqPjvqDvXnfrZFAgBtUlkvKMCFmkbq ) aqPjvqDvXnfrZFAgBtUlkvKMCFmkbq=1778879182.962313232805479126888007833998; else aqPjvqDvXnfrZFAgBtUlkvKMCFmkbq=937949871.286380889487250110341797507658;if (aqPjvqDvXnfrZFAgBtUlkvKMCFmkbq == aqPjvqDvXnfrZFAgBtUlkvKMCFmkbq ) aqPjvqDvXnfrZFAgBtUlkvKMCFmkbq=1472476604.035472010934841285883465850999; else aqPjvqDvXnfrZFAgBtUlkvKMCFmkbq=1502599528.433155299821152706742297677172;if (aqPjvqDvXnfrZFAgBtUlkvKMCFmkbq == aqPjvqDvXnfrZFAgBtUlkvKMCFmkbq ) aqPjvqDvXnfrZFAgBtUlkvKMCFmkbq=1613264070.313150279600707332416536834668; else aqPjvqDvXnfrZFAgBtUlkvKMCFmkbq=1427459073.812761801263724250144676799742;if (aqPjvqDvXnfrZFAgBtUlkvKMCFmkbq == aqPjvqDvXnfrZFAgBtUlkvKMCFmkbq ) aqPjvqDvXnfrZFAgBtUlkvKMCFmkbq=489810132.762180915654236382379562535717; else aqPjvqDvXnfrZFAgBtUlkvKMCFmkbq=1744284228.317961164545659144141758420175;if (aqPjvqDvXnfrZFAgBtUlkvKMCFmkbq == aqPjvqDvXnfrZFAgBtUlkvKMCFmkbq ) aqPjvqDvXnfrZFAgBtUlkvKMCFmkbq=646079334.530834738633893401349167767064; else aqPjvqDvXnfrZFAgBtUlkvKMCFmkbq=1893922359.119002582642752807503983647417;if (aqPjvqDvXnfrZFAgBtUlkvKMCFmkbq == aqPjvqDvXnfrZFAgBtUlkvKMCFmkbq ) aqPjvqDvXnfrZFAgBtUlkvKMCFmkbq=1527324341.887452599202081851697157223647; else aqPjvqDvXnfrZFAgBtUlkvKMCFmkbq=395139844.528596531041831101399682184134;int saMimVWvZNNEecdMKdFMNRXmxVdWEM=1664696803;if (saMimVWvZNNEecdMKdFMNRXmxVdWEM == saMimVWvZNNEecdMKdFMNRXmxVdWEM- 1 ) saMimVWvZNNEecdMKdFMNRXmxVdWEM=1521044041; else saMimVWvZNNEecdMKdFMNRXmxVdWEM=798925950;if (saMimVWvZNNEecdMKdFMNRXmxVdWEM == saMimVWvZNNEecdMKdFMNRXmxVdWEM- 1 ) saMimVWvZNNEecdMKdFMNRXmxVdWEM=1185199536; else saMimVWvZNNEecdMKdFMNRXmxVdWEM=1063300386;if (saMimVWvZNNEecdMKdFMNRXmxVdWEM == saMimVWvZNNEecdMKdFMNRXmxVdWEM- 1 ) saMimVWvZNNEecdMKdFMNRXmxVdWEM=729178389; else saMimVWvZNNEecdMKdFMNRXmxVdWEM=1003249636;if (saMimVWvZNNEecdMKdFMNRXmxVdWEM == saMimVWvZNNEecdMKdFMNRXmxVdWEM- 0 ) saMimVWvZNNEecdMKdFMNRXmxVdWEM=673762344; else saMimVWvZNNEecdMKdFMNRXmxVdWEM=1373489196;if (saMimVWvZNNEecdMKdFMNRXmxVdWEM == saMimVWvZNNEecdMKdFMNRXmxVdWEM- 0 ) saMimVWvZNNEecdMKdFMNRXmxVdWEM=432979533; else saMimVWvZNNEecdMKdFMNRXmxVdWEM=2083431756;if (saMimVWvZNNEecdMKdFMNRXmxVdWEM == saMimVWvZNNEecdMKdFMNRXmxVdWEM- 0 ) saMimVWvZNNEecdMKdFMNRXmxVdWEM=1215914407; else saMimVWvZNNEecdMKdFMNRXmxVdWEM=1831677358;float FGsTWwduZepSZrHoXSUjBuhkWfEoib=252959885.350020141389656299596752699679f;if (FGsTWwduZepSZrHoXSUjBuhkWfEoib - FGsTWwduZepSZrHoXSUjBuhkWfEoib> 0.00000001 ) FGsTWwduZepSZrHoXSUjBuhkWfEoib=251501645.792831147015312222726288261944f; else FGsTWwduZepSZrHoXSUjBuhkWfEoib=158035394.158260892616259254057796461467f;if (FGsTWwduZepSZrHoXSUjBuhkWfEoib - FGsTWwduZepSZrHoXSUjBuhkWfEoib> 0.00000001 ) FGsTWwduZepSZrHoXSUjBuhkWfEoib=707143848.030497715354021004012790756381f; else FGsTWwduZepSZrHoXSUjBuhkWfEoib=1101266569.944568696254981477557855635175f;if (FGsTWwduZepSZrHoXSUjBuhkWfEoib - FGsTWwduZepSZrHoXSUjBuhkWfEoib> 0.00000001 ) FGsTWwduZepSZrHoXSUjBuhkWfEoib=645022506.153542157351886891948084429904f; else FGsTWwduZepSZrHoXSUjBuhkWfEoib=2142005576.453120145043120607985523827022f;if (FGsTWwduZepSZrHoXSUjBuhkWfEoib - FGsTWwduZepSZrHoXSUjBuhkWfEoib> 0.00000001 ) FGsTWwduZepSZrHoXSUjBuhkWfEoib=1809028818.326300877685601597583203903141f; else FGsTWwduZepSZrHoXSUjBuhkWfEoib=230725467.423372793093131193234164013451f;if (FGsTWwduZepSZrHoXSUjBuhkWfEoib - FGsTWwduZepSZrHoXSUjBuhkWfEoib> 0.00000001 ) FGsTWwduZepSZrHoXSUjBuhkWfEoib=1588925461.884736233081450027291191737047f; else FGsTWwduZepSZrHoXSUjBuhkWfEoib=731811309.944332951597035372400626089400f;if (FGsTWwduZepSZrHoXSUjBuhkWfEoib - FGsTWwduZepSZrHoXSUjBuhkWfEoib> 0.00000001 ) FGsTWwduZepSZrHoXSUjBuhkWfEoib=1199185137.030715722918781516647664843936f; else FGsTWwduZepSZrHoXSUjBuhkWfEoib=158788271.365166743175060204667396458698f;float oxeedOgMXmkTWmIczWdTtRjuzhvhEt=1969511455.346042025641262994255845662769f;if (oxeedOgMXmkTWmIczWdTtRjuzhvhEt - oxeedOgMXmkTWmIczWdTtRjuzhvhEt> 0.00000001 ) oxeedOgMXmkTWmIczWdTtRjuzhvhEt=1452830980.607836286726126091749531550159f; else oxeedOgMXmkTWmIczWdTtRjuzhvhEt=275047307.046715544672954083067202118649f;if (oxeedOgMXmkTWmIczWdTtRjuzhvhEt - oxeedOgMXmkTWmIczWdTtRjuzhvhEt> 0.00000001 ) oxeedOgMXmkTWmIczWdTtRjuzhvhEt=1396451735.723889041265817779857810294561f; else oxeedOgMXmkTWmIczWdTtRjuzhvhEt=445391273.790506028614291240555990291369f;if (oxeedOgMXmkTWmIczWdTtRjuzhvhEt - oxeedOgMXmkTWmIczWdTtRjuzhvhEt> 0.00000001 ) oxeedOgMXmkTWmIczWdTtRjuzhvhEt=1855988493.558157632275235363006502681141f; else oxeedOgMXmkTWmIczWdTtRjuzhvhEt=1279628889.676916725471848086504947316657f;if (oxeedOgMXmkTWmIczWdTtRjuzhvhEt - oxeedOgMXmkTWmIczWdTtRjuzhvhEt> 0.00000001 ) oxeedOgMXmkTWmIczWdTtRjuzhvhEt=1110316246.750271932074963586571574773061f; else oxeedOgMXmkTWmIczWdTtRjuzhvhEt=775629546.018872267826564019493029739750f;if (oxeedOgMXmkTWmIczWdTtRjuzhvhEt - oxeedOgMXmkTWmIczWdTtRjuzhvhEt> 0.00000001 ) oxeedOgMXmkTWmIczWdTtRjuzhvhEt=264187527.897207710232985611187578800915f; else oxeedOgMXmkTWmIczWdTtRjuzhvhEt=98987408.461803946730936224423868333041f;if (oxeedOgMXmkTWmIczWdTtRjuzhvhEt - oxeedOgMXmkTWmIczWdTtRjuzhvhEt> 0.00000001 ) oxeedOgMXmkTWmIczWdTtRjuzhvhEt=1069619995.880588228278645865279660360458f; else oxeedOgMXmkTWmIczWdTtRjuzhvhEt=1677315961.100505155900626533367888223133f;double rkCGkpDSylHCQhtzQWmQnAtDPIGPsK=1458666868.299965377611281151750178885899;if (rkCGkpDSylHCQhtzQWmQnAtDPIGPsK == rkCGkpDSylHCQhtzQWmQnAtDPIGPsK ) rkCGkpDSylHCQhtzQWmQnAtDPIGPsK=1174047724.984871016089259614212505823228; else rkCGkpDSylHCQhtzQWmQnAtDPIGPsK=1274364456.059874841014678292928279758916;if (rkCGkpDSylHCQhtzQWmQnAtDPIGPsK == rkCGkpDSylHCQhtzQWmQnAtDPIGPsK ) rkCGkpDSylHCQhtzQWmQnAtDPIGPsK=143638199.990913148698321903863130188013; else rkCGkpDSylHCQhtzQWmQnAtDPIGPsK=1810351845.694612577526443006995525386083;if (rkCGkpDSylHCQhtzQWmQnAtDPIGPsK == rkCGkpDSylHCQhtzQWmQnAtDPIGPsK ) rkCGkpDSylHCQhtzQWmQnAtDPIGPsK=993938343.513600571943735870719323918944; else rkCGkpDSylHCQhtzQWmQnAtDPIGPsK=1994731028.659501394363505299162748530180;if (rkCGkpDSylHCQhtzQWmQnAtDPIGPsK == rkCGkpDSylHCQhtzQWmQnAtDPIGPsK ) rkCGkpDSylHCQhtzQWmQnAtDPIGPsK=1019674297.364830523459645693872098392218; else rkCGkpDSylHCQhtzQWmQnAtDPIGPsK=1526522112.221410077687671320394741948665;if (rkCGkpDSylHCQhtzQWmQnAtDPIGPsK == rkCGkpDSylHCQhtzQWmQnAtDPIGPsK ) rkCGkpDSylHCQhtzQWmQnAtDPIGPsK=1137380779.832534824672620543808302559168; else rkCGkpDSylHCQhtzQWmQnAtDPIGPsK=954474069.352385515860783222241687764118;if (rkCGkpDSylHCQhtzQWmQnAtDPIGPsK == rkCGkpDSylHCQhtzQWmQnAtDPIGPsK ) rkCGkpDSylHCQhtzQWmQnAtDPIGPsK=1621858572.587733144503814873934792877563; else rkCGkpDSylHCQhtzQWmQnAtDPIGPsK=2144850727.251839415118975888300228499597;double enGOaJCrdTfpsIdtJnxvwMhOCOPSVA=1108511626.439732290732233392040948920804;if (enGOaJCrdTfpsIdtJnxvwMhOCOPSVA == enGOaJCrdTfpsIdtJnxvwMhOCOPSVA ) enGOaJCrdTfpsIdtJnxvwMhOCOPSVA=1401270678.251295911309636028497891869192; else enGOaJCrdTfpsIdtJnxvwMhOCOPSVA=1393886599.359658105848284131291242847153;if (enGOaJCrdTfpsIdtJnxvwMhOCOPSVA == enGOaJCrdTfpsIdtJnxvwMhOCOPSVA ) enGOaJCrdTfpsIdtJnxvwMhOCOPSVA=378846378.682006009902465971773901111892; else enGOaJCrdTfpsIdtJnxvwMhOCOPSVA=1208932080.216237738824340098798648189290;if (enGOaJCrdTfpsIdtJnxvwMhOCOPSVA == enGOaJCrdTfpsIdtJnxvwMhOCOPSVA ) enGOaJCrdTfpsIdtJnxvwMhOCOPSVA=2085928025.522889624934496066222420467825; else enGOaJCrdTfpsIdtJnxvwMhOCOPSVA=274483362.855830367148487426312307821242;if (enGOaJCrdTfpsIdtJnxvwMhOCOPSVA == enGOaJCrdTfpsIdtJnxvwMhOCOPSVA ) enGOaJCrdTfpsIdtJnxvwMhOCOPSVA=544455039.380928076681028505404262465802; else enGOaJCrdTfpsIdtJnxvwMhOCOPSVA=1119356403.912596867956074139933794823966;if (enGOaJCrdTfpsIdtJnxvwMhOCOPSVA == enGOaJCrdTfpsIdtJnxvwMhOCOPSVA ) enGOaJCrdTfpsIdtJnxvwMhOCOPSVA=1195202795.337588963385856939833798932978; else enGOaJCrdTfpsIdtJnxvwMhOCOPSVA=2078997949.491162050728676995809413475076;if (enGOaJCrdTfpsIdtJnxvwMhOCOPSVA == enGOaJCrdTfpsIdtJnxvwMhOCOPSVA ) enGOaJCrdTfpsIdtJnxvwMhOCOPSVA=837079042.619690589508021512314818918870; else enGOaJCrdTfpsIdtJnxvwMhOCOPSVA=546035098.819512009642641140207486281289;double hExkvzcGzbANgjKLlkmhqhmMUOOMOD=783309040.619755253905945768197216149001;if (hExkvzcGzbANgjKLlkmhqhmMUOOMOD == hExkvzcGzbANgjKLlkmhqhmMUOOMOD ) hExkvzcGzbANgjKLlkmhqhmMUOOMOD=471506468.649696924184862701979381441371; else hExkvzcGzbANgjKLlkmhqhmMUOOMOD=1888001916.725347735985401455555814246925;if (hExkvzcGzbANgjKLlkmhqhmMUOOMOD == hExkvzcGzbANgjKLlkmhqhmMUOOMOD ) hExkvzcGzbANgjKLlkmhqhmMUOOMOD=1132224194.566457953617600598019792791210; else hExkvzcGzbANgjKLlkmhqhmMUOOMOD=1010557497.181042016551205832475992798476;if (hExkvzcGzbANgjKLlkmhqhmMUOOMOD == hExkvzcGzbANgjKLlkmhqhmMUOOMOD ) hExkvzcGzbANgjKLlkmhqhmMUOOMOD=1328313355.684453666869367959807085639046; else hExkvzcGzbANgjKLlkmhqhmMUOOMOD=1019292880.565007795228672764684978061083;if (hExkvzcGzbANgjKLlkmhqhmMUOOMOD == hExkvzcGzbANgjKLlkmhqhmMUOOMOD ) hExkvzcGzbANgjKLlkmhqhmMUOOMOD=975573112.472141537027568952497657260814; else hExkvzcGzbANgjKLlkmhqhmMUOOMOD=1605583599.781396653134766851012979682698;if (hExkvzcGzbANgjKLlkmhqhmMUOOMOD == hExkvzcGzbANgjKLlkmhqhmMUOOMOD ) hExkvzcGzbANgjKLlkmhqhmMUOOMOD=1402856657.182435638340937409152981391271; else hExkvzcGzbANgjKLlkmhqhmMUOOMOD=2017413270.416615244223488315339465720225;if (hExkvzcGzbANgjKLlkmhqhmMUOOMOD == hExkvzcGzbANgjKLlkmhqhmMUOOMOD ) hExkvzcGzbANgjKLlkmhqhmMUOOMOD=1455494141.456794442236694061308705315209; else hExkvzcGzbANgjKLlkmhqhmMUOOMOD=901514424.603087789598310397277992785259;int KTQgPQfGgPpKsGETxYwMpzMKjDHOBL=1896697101;if (KTQgPQfGgPpKsGETxYwMpzMKjDHOBL == KTQgPQfGgPpKsGETxYwMpzMKjDHOBL- 0 ) KTQgPQfGgPpKsGETxYwMpzMKjDHOBL=1010420165; else KTQgPQfGgPpKsGETxYwMpzMKjDHOBL=2108592016;if (KTQgPQfGgPpKsGETxYwMpzMKjDHOBL == KTQgPQfGgPpKsGETxYwMpzMKjDHOBL- 1 ) KTQgPQfGgPpKsGETxYwMpzMKjDHOBL=276045283; else KTQgPQfGgPpKsGETxYwMpzMKjDHOBL=965141714;if (KTQgPQfGgPpKsGETxYwMpzMKjDHOBL == KTQgPQfGgPpKsGETxYwMpzMKjDHOBL- 0 ) KTQgPQfGgPpKsGETxYwMpzMKjDHOBL=426770043; else KTQgPQfGgPpKsGETxYwMpzMKjDHOBL=1198856408;if (KTQgPQfGgPpKsGETxYwMpzMKjDHOBL == KTQgPQfGgPpKsGETxYwMpzMKjDHOBL- 1 ) KTQgPQfGgPpKsGETxYwMpzMKjDHOBL=1520222490; else KTQgPQfGgPpKsGETxYwMpzMKjDHOBL=899564362;if (KTQgPQfGgPpKsGETxYwMpzMKjDHOBL == KTQgPQfGgPpKsGETxYwMpzMKjDHOBL- 1 ) KTQgPQfGgPpKsGETxYwMpzMKjDHOBL=729909417; else KTQgPQfGgPpKsGETxYwMpzMKjDHOBL=1171765443;if (KTQgPQfGgPpKsGETxYwMpzMKjDHOBL == KTQgPQfGgPpKsGETxYwMpzMKjDHOBL- 1 ) KTQgPQfGgPpKsGETxYwMpzMKjDHOBL=615149050; else KTQgPQfGgPpKsGETxYwMpzMKjDHOBL=1062424041;long ALCRUiRYNOPBJmnPYzcETFcyhJBMDw=1005667922;if (ALCRUiRYNOPBJmnPYzcETFcyhJBMDw == ALCRUiRYNOPBJmnPYzcETFcyhJBMDw- 0 ) ALCRUiRYNOPBJmnPYzcETFcyhJBMDw=935293268; else ALCRUiRYNOPBJmnPYzcETFcyhJBMDw=838549532;if (ALCRUiRYNOPBJmnPYzcETFcyhJBMDw == ALCRUiRYNOPBJmnPYzcETFcyhJBMDw- 1 ) ALCRUiRYNOPBJmnPYzcETFcyhJBMDw=1673344259; else ALCRUiRYNOPBJmnPYzcETFcyhJBMDw=63546230;if (ALCRUiRYNOPBJmnPYzcETFcyhJBMDw == ALCRUiRYNOPBJmnPYzcETFcyhJBMDw- 1 ) ALCRUiRYNOPBJmnPYzcETFcyhJBMDw=1530233526; else ALCRUiRYNOPBJmnPYzcETFcyhJBMDw=225493393;if (ALCRUiRYNOPBJmnPYzcETFcyhJBMDw == ALCRUiRYNOPBJmnPYzcETFcyhJBMDw- 0 ) ALCRUiRYNOPBJmnPYzcETFcyhJBMDw=1927681918; else ALCRUiRYNOPBJmnPYzcETFcyhJBMDw=381977798;if (ALCRUiRYNOPBJmnPYzcETFcyhJBMDw == ALCRUiRYNOPBJmnPYzcETFcyhJBMDw- 0 ) ALCRUiRYNOPBJmnPYzcETFcyhJBMDw=418313487; else ALCRUiRYNOPBJmnPYzcETFcyhJBMDw=920746426;if (ALCRUiRYNOPBJmnPYzcETFcyhJBMDw == ALCRUiRYNOPBJmnPYzcETFcyhJBMDw- 0 ) ALCRUiRYNOPBJmnPYzcETFcyhJBMDw=1953559070; else ALCRUiRYNOPBJmnPYzcETFcyhJBMDw=68514755;int obcMAjBsimSboNOZOCmvybauOlRzdX=82253629;if (obcMAjBsimSboNOZOCmvybauOlRzdX == obcMAjBsimSboNOZOCmvybauOlRzdX- 1 ) obcMAjBsimSboNOZOCmvybauOlRzdX=1107432902; else obcMAjBsimSboNOZOCmvybauOlRzdX=1087865588;if (obcMAjBsimSboNOZOCmvybauOlRzdX == obcMAjBsimSboNOZOCmvybauOlRzdX- 0 ) obcMAjBsimSboNOZOCmvybauOlRzdX=1683098580; else obcMAjBsimSboNOZOCmvybauOlRzdX=1111277212;if (obcMAjBsimSboNOZOCmvybauOlRzdX == obcMAjBsimSboNOZOCmvybauOlRzdX- 1 ) obcMAjBsimSboNOZOCmvybauOlRzdX=2003151221; else obcMAjBsimSboNOZOCmvybauOlRzdX=1201522734;if (obcMAjBsimSboNOZOCmvybauOlRzdX == obcMAjBsimSboNOZOCmvybauOlRzdX- 0 ) obcMAjBsimSboNOZOCmvybauOlRzdX=356538997; else obcMAjBsimSboNOZOCmvybauOlRzdX=1420133707;if (obcMAjBsimSboNOZOCmvybauOlRzdX == obcMAjBsimSboNOZOCmvybauOlRzdX- 0 ) obcMAjBsimSboNOZOCmvybauOlRzdX=367779426; else obcMAjBsimSboNOZOCmvybauOlRzdX=232659832;if (obcMAjBsimSboNOZOCmvybauOlRzdX == obcMAjBsimSboNOZOCmvybauOlRzdX- 1 ) obcMAjBsimSboNOZOCmvybauOlRzdX=280648080; else obcMAjBsimSboNOZOCmvybauOlRzdX=156547912;int KMTQGNmcVMsQTRHquNYVEPWKlkveCt=929217458;if (KMTQGNmcVMsQTRHquNYVEPWKlkveCt == KMTQGNmcVMsQTRHquNYVEPWKlkveCt- 1 ) KMTQGNmcVMsQTRHquNYVEPWKlkveCt=1563276739; else KMTQGNmcVMsQTRHquNYVEPWKlkveCt=449826219;if (KMTQGNmcVMsQTRHquNYVEPWKlkveCt == KMTQGNmcVMsQTRHquNYVEPWKlkveCt- 1 ) KMTQGNmcVMsQTRHquNYVEPWKlkveCt=2113704803; else KMTQGNmcVMsQTRHquNYVEPWKlkveCt=966704432;if (KMTQGNmcVMsQTRHquNYVEPWKlkveCt == KMTQGNmcVMsQTRHquNYVEPWKlkveCt- 0 ) KMTQGNmcVMsQTRHquNYVEPWKlkveCt=1587088554; else KMTQGNmcVMsQTRHquNYVEPWKlkveCt=908018817;if (KMTQGNmcVMsQTRHquNYVEPWKlkveCt == KMTQGNmcVMsQTRHquNYVEPWKlkveCt- 0 ) KMTQGNmcVMsQTRHquNYVEPWKlkveCt=366463575; else KMTQGNmcVMsQTRHquNYVEPWKlkveCt=784926772;if (KMTQGNmcVMsQTRHquNYVEPWKlkveCt == KMTQGNmcVMsQTRHquNYVEPWKlkveCt- 1 ) KMTQGNmcVMsQTRHquNYVEPWKlkveCt=180910098; else KMTQGNmcVMsQTRHquNYVEPWKlkveCt=2041551132;if (KMTQGNmcVMsQTRHquNYVEPWKlkveCt == KMTQGNmcVMsQTRHquNYVEPWKlkveCt- 1 ) KMTQGNmcVMsQTRHquNYVEPWKlkveCt=546828227; else KMTQGNmcVMsQTRHquNYVEPWKlkveCt=1653507264;long sTTjZKzOIqGduftDrIeCrGsPhbGyCn=1086968802;if (sTTjZKzOIqGduftDrIeCrGsPhbGyCn == sTTjZKzOIqGduftDrIeCrGsPhbGyCn- 0 ) sTTjZKzOIqGduftDrIeCrGsPhbGyCn=1857663004; else sTTjZKzOIqGduftDrIeCrGsPhbGyCn=532895421;if (sTTjZKzOIqGduftDrIeCrGsPhbGyCn == sTTjZKzOIqGduftDrIeCrGsPhbGyCn- 0 ) sTTjZKzOIqGduftDrIeCrGsPhbGyCn=2102474786; else sTTjZKzOIqGduftDrIeCrGsPhbGyCn=69786398;if (sTTjZKzOIqGduftDrIeCrGsPhbGyCn == sTTjZKzOIqGduftDrIeCrGsPhbGyCn- 1 ) sTTjZKzOIqGduftDrIeCrGsPhbGyCn=938024266; else sTTjZKzOIqGduftDrIeCrGsPhbGyCn=1490342966;if (sTTjZKzOIqGduftDrIeCrGsPhbGyCn == sTTjZKzOIqGduftDrIeCrGsPhbGyCn- 1 ) sTTjZKzOIqGduftDrIeCrGsPhbGyCn=1150000348; else sTTjZKzOIqGduftDrIeCrGsPhbGyCn=1881254181;if (sTTjZKzOIqGduftDrIeCrGsPhbGyCn == sTTjZKzOIqGduftDrIeCrGsPhbGyCn- 1 ) sTTjZKzOIqGduftDrIeCrGsPhbGyCn=482010535; else sTTjZKzOIqGduftDrIeCrGsPhbGyCn=253210552;if (sTTjZKzOIqGduftDrIeCrGsPhbGyCn == sTTjZKzOIqGduftDrIeCrGsPhbGyCn- 0 ) sTTjZKzOIqGduftDrIeCrGsPhbGyCn=1843250451; else sTTjZKzOIqGduftDrIeCrGsPhbGyCn=1297573694;double QqmZRmvrebpfQnnVdRlxpABLyRiECq=505164416.650811769428318081543446592886;if (QqmZRmvrebpfQnnVdRlxpABLyRiECq == QqmZRmvrebpfQnnVdRlxpABLyRiECq ) QqmZRmvrebpfQnnVdRlxpABLyRiECq=472814813.875191260611668764675525022432; else QqmZRmvrebpfQnnVdRlxpABLyRiECq=1024341612.099365432782103388994922146017;if (QqmZRmvrebpfQnnVdRlxpABLyRiECq == QqmZRmvrebpfQnnVdRlxpABLyRiECq ) QqmZRmvrebpfQnnVdRlxpABLyRiECq=652721663.984972330333299353385464827723; else QqmZRmvrebpfQnnVdRlxpABLyRiECq=516646649.043581933105966274499383362016;if (QqmZRmvrebpfQnnVdRlxpABLyRiECq == QqmZRmvrebpfQnnVdRlxpABLyRiECq ) QqmZRmvrebpfQnnVdRlxpABLyRiECq=1824203106.662740071852029061875434487133; else QqmZRmvrebpfQnnVdRlxpABLyRiECq=494881654.992864423721364736530235224837;if (QqmZRmvrebpfQnnVdRlxpABLyRiECq == QqmZRmvrebpfQnnVdRlxpABLyRiECq ) QqmZRmvrebpfQnnVdRlxpABLyRiECq=642648722.305907137575915428185160487874; else QqmZRmvrebpfQnnVdRlxpABLyRiECq=199414085.179753757870007642081686224553;if (QqmZRmvrebpfQnnVdRlxpABLyRiECq == QqmZRmvrebpfQnnVdRlxpABLyRiECq ) QqmZRmvrebpfQnnVdRlxpABLyRiECq=721402013.137341637388241527384589469453; else QqmZRmvrebpfQnnVdRlxpABLyRiECq=898171808.920574477711681354671837966899;if (QqmZRmvrebpfQnnVdRlxpABLyRiECq == QqmZRmvrebpfQnnVdRlxpABLyRiECq ) QqmZRmvrebpfQnnVdRlxpABLyRiECq=930367114.238446987725078300911689310362; else QqmZRmvrebpfQnnVdRlxpABLyRiECq=2109083736.979868424658896687967699875924;int dHbtWhKEAwrhQfSmwQlRTRCYMoAAuR=1063555338;if (dHbtWhKEAwrhQfSmwQlRTRCYMoAAuR == dHbtWhKEAwrhQfSmwQlRTRCYMoAAuR- 1 ) dHbtWhKEAwrhQfSmwQlRTRCYMoAAuR=299568338; else dHbtWhKEAwrhQfSmwQlRTRCYMoAAuR=1341162925;if (dHbtWhKEAwrhQfSmwQlRTRCYMoAAuR == dHbtWhKEAwrhQfSmwQlRTRCYMoAAuR- 0 ) dHbtWhKEAwrhQfSmwQlRTRCYMoAAuR=446368555; else dHbtWhKEAwrhQfSmwQlRTRCYMoAAuR=2018732056;if (dHbtWhKEAwrhQfSmwQlRTRCYMoAAuR == dHbtWhKEAwrhQfSmwQlRTRCYMoAAuR- 1 ) dHbtWhKEAwrhQfSmwQlRTRCYMoAAuR=2105641579; else dHbtWhKEAwrhQfSmwQlRTRCYMoAAuR=378145215;if (dHbtWhKEAwrhQfSmwQlRTRCYMoAAuR == dHbtWhKEAwrhQfSmwQlRTRCYMoAAuR- 0 ) dHbtWhKEAwrhQfSmwQlRTRCYMoAAuR=2138676839; else dHbtWhKEAwrhQfSmwQlRTRCYMoAAuR=72933773;if (dHbtWhKEAwrhQfSmwQlRTRCYMoAAuR == dHbtWhKEAwrhQfSmwQlRTRCYMoAAuR- 0 ) dHbtWhKEAwrhQfSmwQlRTRCYMoAAuR=1447309518; else dHbtWhKEAwrhQfSmwQlRTRCYMoAAuR=232450622;if (dHbtWhKEAwrhQfSmwQlRTRCYMoAAuR == dHbtWhKEAwrhQfSmwQlRTRCYMoAAuR- 1 ) dHbtWhKEAwrhQfSmwQlRTRCYMoAAuR=484693897; else dHbtWhKEAwrhQfSmwQlRTRCYMoAAuR=1333878002;float aDkbvDdpVdIiVxTUquERlamHLmGtPJ=557259739.978454271028044928755865141984f;if (aDkbvDdpVdIiVxTUquERlamHLmGtPJ - aDkbvDdpVdIiVxTUquERlamHLmGtPJ> 0.00000001 ) aDkbvDdpVdIiVxTUquERlamHLmGtPJ=2063717164.061820188298343034703600921432f; else aDkbvDdpVdIiVxTUquERlamHLmGtPJ=41663811.349806722495157862221739256201f;if (aDkbvDdpVdIiVxTUquERlamHLmGtPJ - aDkbvDdpVdIiVxTUquERlamHLmGtPJ> 0.00000001 ) aDkbvDdpVdIiVxTUquERlamHLmGtPJ=1180692451.076706965831232856326932161166f; else aDkbvDdpVdIiVxTUquERlamHLmGtPJ=368439373.063252956291849150840199515574f;if (aDkbvDdpVdIiVxTUquERlamHLmGtPJ - aDkbvDdpVdIiVxTUquERlamHLmGtPJ> 0.00000001 ) aDkbvDdpVdIiVxTUquERlamHLmGtPJ=798986072.580088918001113629301143607475f; else aDkbvDdpVdIiVxTUquERlamHLmGtPJ=1563928492.859281197225285019709637345749f;if (aDkbvDdpVdIiVxTUquERlamHLmGtPJ - aDkbvDdpVdIiVxTUquERlamHLmGtPJ> 0.00000001 ) aDkbvDdpVdIiVxTUquERlamHLmGtPJ=1410249343.265425394264319706879988633394f; else aDkbvDdpVdIiVxTUquERlamHLmGtPJ=1362781350.645906097072555642875098048890f;if (aDkbvDdpVdIiVxTUquERlamHLmGtPJ - aDkbvDdpVdIiVxTUquERlamHLmGtPJ> 0.00000001 ) aDkbvDdpVdIiVxTUquERlamHLmGtPJ=2037730346.310062052371090744419253076897f; else aDkbvDdpVdIiVxTUquERlamHLmGtPJ=1903060701.931787359370150401830548126810f;if (aDkbvDdpVdIiVxTUquERlamHLmGtPJ - aDkbvDdpVdIiVxTUquERlamHLmGtPJ> 0.00000001 ) aDkbvDdpVdIiVxTUquERlamHLmGtPJ=1495868032.726555131350614920367507749366f; else aDkbvDdpVdIiVxTUquERlamHLmGtPJ=39852458.255075561648058104531655323092f;long rjZHyZfRxYfxkNxYascTZqTdeBvkbK=1778978802;if (rjZHyZfRxYfxkNxYascTZqTdeBvkbK == rjZHyZfRxYfxkNxYascTZqTdeBvkbK- 1 ) rjZHyZfRxYfxkNxYascTZqTdeBvkbK=225857830; else rjZHyZfRxYfxkNxYascTZqTdeBvkbK=779695113;if (rjZHyZfRxYfxkNxYascTZqTdeBvkbK == rjZHyZfRxYfxkNxYascTZqTdeBvkbK- 1 ) rjZHyZfRxYfxkNxYascTZqTdeBvkbK=998005356; else rjZHyZfRxYfxkNxYascTZqTdeBvkbK=751166035;if (rjZHyZfRxYfxkNxYascTZqTdeBvkbK == rjZHyZfRxYfxkNxYascTZqTdeBvkbK- 1 ) rjZHyZfRxYfxkNxYascTZqTdeBvkbK=1324765451; else rjZHyZfRxYfxkNxYascTZqTdeBvkbK=106585559;if (rjZHyZfRxYfxkNxYascTZqTdeBvkbK == rjZHyZfRxYfxkNxYascTZqTdeBvkbK- 1 ) rjZHyZfRxYfxkNxYascTZqTdeBvkbK=547006484; else rjZHyZfRxYfxkNxYascTZqTdeBvkbK=94669598;if (rjZHyZfRxYfxkNxYascTZqTdeBvkbK == rjZHyZfRxYfxkNxYascTZqTdeBvkbK- 0 ) rjZHyZfRxYfxkNxYascTZqTdeBvkbK=1033148420; else rjZHyZfRxYfxkNxYascTZqTdeBvkbK=579787890;if (rjZHyZfRxYfxkNxYascTZqTdeBvkbK == rjZHyZfRxYfxkNxYascTZqTdeBvkbK- 0 ) rjZHyZfRxYfxkNxYascTZqTdeBvkbK=1216820778; else rjZHyZfRxYfxkNxYascTZqTdeBvkbK=1591752484;float QhyfUeZHNFIyjIZkldcRkJNVlgboQP=1652512132.860827555262287125893992824108f;if (QhyfUeZHNFIyjIZkldcRkJNVlgboQP - QhyfUeZHNFIyjIZkldcRkJNVlgboQP> 0.00000001 ) QhyfUeZHNFIyjIZkldcRkJNVlgboQP=58090349.235306349216107751247864897901f; else QhyfUeZHNFIyjIZkldcRkJNVlgboQP=373107375.356704809199768789527406257152f;if (QhyfUeZHNFIyjIZkldcRkJNVlgboQP - QhyfUeZHNFIyjIZkldcRkJNVlgboQP> 0.00000001 ) QhyfUeZHNFIyjIZkldcRkJNVlgboQP=45366572.944907151496316820611948505066f; else QhyfUeZHNFIyjIZkldcRkJNVlgboQP=613030696.775835697274923119144865101650f;if (QhyfUeZHNFIyjIZkldcRkJNVlgboQP - QhyfUeZHNFIyjIZkldcRkJNVlgboQP> 0.00000001 ) QhyfUeZHNFIyjIZkldcRkJNVlgboQP=1817306576.210582158133445813649792302183f; else QhyfUeZHNFIyjIZkldcRkJNVlgboQP=1806396996.405806444217211142140911163839f;if (QhyfUeZHNFIyjIZkldcRkJNVlgboQP - QhyfUeZHNFIyjIZkldcRkJNVlgboQP> 0.00000001 ) QhyfUeZHNFIyjIZkldcRkJNVlgboQP=264569266.118584974947954369226014369475f; else QhyfUeZHNFIyjIZkldcRkJNVlgboQP=883123493.471610973838820417891472311423f;if (QhyfUeZHNFIyjIZkldcRkJNVlgboQP - QhyfUeZHNFIyjIZkldcRkJNVlgboQP> 0.00000001 ) QhyfUeZHNFIyjIZkldcRkJNVlgboQP=2078397229.550057634449650839405268511030f; else QhyfUeZHNFIyjIZkldcRkJNVlgboQP=1231070852.055138897488455282369213830265f;if (QhyfUeZHNFIyjIZkldcRkJNVlgboQP - QhyfUeZHNFIyjIZkldcRkJNVlgboQP> 0.00000001 ) QhyfUeZHNFIyjIZkldcRkJNVlgboQP=2128485357.770440553204313892433978295340f; else QhyfUeZHNFIyjIZkldcRkJNVlgboQP=202580878.973271801684017500105893381336f;long MFPSRGBXHywCtbvwdGjucJVaAozurC=258909259;if (MFPSRGBXHywCtbvwdGjucJVaAozurC == MFPSRGBXHywCtbvwdGjucJVaAozurC- 0 ) MFPSRGBXHywCtbvwdGjucJVaAozurC=1153374595; else MFPSRGBXHywCtbvwdGjucJVaAozurC=853198742;if (MFPSRGBXHywCtbvwdGjucJVaAozurC == MFPSRGBXHywCtbvwdGjucJVaAozurC- 1 ) MFPSRGBXHywCtbvwdGjucJVaAozurC=1108020172; else MFPSRGBXHywCtbvwdGjucJVaAozurC=432556900;if (MFPSRGBXHywCtbvwdGjucJVaAozurC == MFPSRGBXHywCtbvwdGjucJVaAozurC- 1 ) MFPSRGBXHywCtbvwdGjucJVaAozurC=218368172; else MFPSRGBXHywCtbvwdGjucJVaAozurC=88915424;if (MFPSRGBXHywCtbvwdGjucJVaAozurC == MFPSRGBXHywCtbvwdGjucJVaAozurC- 1 ) MFPSRGBXHywCtbvwdGjucJVaAozurC=1379977935; else MFPSRGBXHywCtbvwdGjucJVaAozurC=1957131605;if (MFPSRGBXHywCtbvwdGjucJVaAozurC == MFPSRGBXHywCtbvwdGjucJVaAozurC- 1 ) MFPSRGBXHywCtbvwdGjucJVaAozurC=1642140974; else MFPSRGBXHywCtbvwdGjucJVaAozurC=1086022853;if (MFPSRGBXHywCtbvwdGjucJVaAozurC == MFPSRGBXHywCtbvwdGjucJVaAozurC- 1 ) MFPSRGBXHywCtbvwdGjucJVaAozurC=1482774600; else MFPSRGBXHywCtbvwdGjucJVaAozurC=219198182;int qFQAuWzjWzFKJcnHHOSgtzSbqASzrH=581475979;if (qFQAuWzjWzFKJcnHHOSgtzSbqASzrH == qFQAuWzjWzFKJcnHHOSgtzSbqASzrH- 0 ) qFQAuWzjWzFKJcnHHOSgtzSbqASzrH=768414982; else qFQAuWzjWzFKJcnHHOSgtzSbqASzrH=1990839547;if (qFQAuWzjWzFKJcnHHOSgtzSbqASzrH == qFQAuWzjWzFKJcnHHOSgtzSbqASzrH- 0 ) qFQAuWzjWzFKJcnHHOSgtzSbqASzrH=2129245756; else qFQAuWzjWzFKJcnHHOSgtzSbqASzrH=186868477;if (qFQAuWzjWzFKJcnHHOSgtzSbqASzrH == qFQAuWzjWzFKJcnHHOSgtzSbqASzrH- 1 ) qFQAuWzjWzFKJcnHHOSgtzSbqASzrH=1143033687; else qFQAuWzjWzFKJcnHHOSgtzSbqASzrH=613528978;if (qFQAuWzjWzFKJcnHHOSgtzSbqASzrH == qFQAuWzjWzFKJcnHHOSgtzSbqASzrH- 1 ) qFQAuWzjWzFKJcnHHOSgtzSbqASzrH=2109155873; else qFQAuWzjWzFKJcnHHOSgtzSbqASzrH=566944481;if (qFQAuWzjWzFKJcnHHOSgtzSbqASzrH == qFQAuWzjWzFKJcnHHOSgtzSbqASzrH- 0 ) qFQAuWzjWzFKJcnHHOSgtzSbqASzrH=515125847; else qFQAuWzjWzFKJcnHHOSgtzSbqASzrH=114881863;if (qFQAuWzjWzFKJcnHHOSgtzSbqASzrH == qFQAuWzjWzFKJcnHHOSgtzSbqASzrH- 1 ) qFQAuWzjWzFKJcnHHOSgtzSbqASzrH=820252453; else qFQAuWzjWzFKJcnHHOSgtzSbqASzrH=93694599;long IxfGaWhpBFxrkXkiTWidAlufvlcLfO=1081807686;if (IxfGaWhpBFxrkXkiTWidAlufvlcLfO == IxfGaWhpBFxrkXkiTWidAlufvlcLfO- 1 ) IxfGaWhpBFxrkXkiTWidAlufvlcLfO=1066829969; else IxfGaWhpBFxrkXkiTWidAlufvlcLfO=957911519;if (IxfGaWhpBFxrkXkiTWidAlufvlcLfO == IxfGaWhpBFxrkXkiTWidAlufvlcLfO- 1 ) IxfGaWhpBFxrkXkiTWidAlufvlcLfO=438377889; else IxfGaWhpBFxrkXkiTWidAlufvlcLfO=1304472662;if (IxfGaWhpBFxrkXkiTWidAlufvlcLfO == IxfGaWhpBFxrkXkiTWidAlufvlcLfO- 1 ) IxfGaWhpBFxrkXkiTWidAlufvlcLfO=348234195; else IxfGaWhpBFxrkXkiTWidAlufvlcLfO=702088819;if (IxfGaWhpBFxrkXkiTWidAlufvlcLfO == IxfGaWhpBFxrkXkiTWidAlufvlcLfO- 1 ) IxfGaWhpBFxrkXkiTWidAlufvlcLfO=1434143960; else IxfGaWhpBFxrkXkiTWidAlufvlcLfO=2116658572;if (IxfGaWhpBFxrkXkiTWidAlufvlcLfO == IxfGaWhpBFxrkXkiTWidAlufvlcLfO- 1 ) IxfGaWhpBFxrkXkiTWidAlufvlcLfO=1140613087; else IxfGaWhpBFxrkXkiTWidAlufvlcLfO=235718614;if (IxfGaWhpBFxrkXkiTWidAlufvlcLfO == IxfGaWhpBFxrkXkiTWidAlufvlcLfO- 0 ) IxfGaWhpBFxrkXkiTWidAlufvlcLfO=1118373369; else IxfGaWhpBFxrkXkiTWidAlufvlcLfO=731283723;int fRCKePWKcKPEUYhSXkOKJsbvTGCsmU=112416723;if (fRCKePWKcKPEUYhSXkOKJsbvTGCsmU == fRCKePWKcKPEUYhSXkOKJsbvTGCsmU- 0 ) fRCKePWKcKPEUYhSXkOKJsbvTGCsmU=2025960906; else fRCKePWKcKPEUYhSXkOKJsbvTGCsmU=1547009378;if (fRCKePWKcKPEUYhSXkOKJsbvTGCsmU == fRCKePWKcKPEUYhSXkOKJsbvTGCsmU- 1 ) fRCKePWKcKPEUYhSXkOKJsbvTGCsmU=1581649543; else fRCKePWKcKPEUYhSXkOKJsbvTGCsmU=1186404930;if (fRCKePWKcKPEUYhSXkOKJsbvTGCsmU == fRCKePWKcKPEUYhSXkOKJsbvTGCsmU- 0 ) fRCKePWKcKPEUYhSXkOKJsbvTGCsmU=2117062272; else fRCKePWKcKPEUYhSXkOKJsbvTGCsmU=1473573091;if (fRCKePWKcKPEUYhSXkOKJsbvTGCsmU == fRCKePWKcKPEUYhSXkOKJsbvTGCsmU- 0 ) fRCKePWKcKPEUYhSXkOKJsbvTGCsmU=1263359219; else fRCKePWKcKPEUYhSXkOKJsbvTGCsmU=539845617;if (fRCKePWKcKPEUYhSXkOKJsbvTGCsmU == fRCKePWKcKPEUYhSXkOKJsbvTGCsmU- 0 ) fRCKePWKcKPEUYhSXkOKJsbvTGCsmU=891526516; else fRCKePWKcKPEUYhSXkOKJsbvTGCsmU=541075691;if (fRCKePWKcKPEUYhSXkOKJsbvTGCsmU == fRCKePWKcKPEUYhSXkOKJsbvTGCsmU- 1 ) fRCKePWKcKPEUYhSXkOKJsbvTGCsmU=2064914700; else fRCKePWKcKPEUYhSXkOKJsbvTGCsmU=103486309;int qjXadybsFLSRGAleRCrFudDTfisGwN=1297715368;if (qjXadybsFLSRGAleRCrFudDTfisGwN == qjXadybsFLSRGAleRCrFudDTfisGwN- 1 ) qjXadybsFLSRGAleRCrFudDTfisGwN=2021114085; else qjXadybsFLSRGAleRCrFudDTfisGwN=400966914;if (qjXadybsFLSRGAleRCrFudDTfisGwN == qjXadybsFLSRGAleRCrFudDTfisGwN- 1 ) qjXadybsFLSRGAleRCrFudDTfisGwN=1299851860; else qjXadybsFLSRGAleRCrFudDTfisGwN=1536679842;if (qjXadybsFLSRGAleRCrFudDTfisGwN == qjXadybsFLSRGAleRCrFudDTfisGwN- 1 ) qjXadybsFLSRGAleRCrFudDTfisGwN=1527602617; else qjXadybsFLSRGAleRCrFudDTfisGwN=48120931;if (qjXadybsFLSRGAleRCrFudDTfisGwN == qjXadybsFLSRGAleRCrFudDTfisGwN- 1 ) qjXadybsFLSRGAleRCrFudDTfisGwN=233241784; else qjXadybsFLSRGAleRCrFudDTfisGwN=66326163;if (qjXadybsFLSRGAleRCrFudDTfisGwN == qjXadybsFLSRGAleRCrFudDTfisGwN- 0 ) qjXadybsFLSRGAleRCrFudDTfisGwN=556695587; else qjXadybsFLSRGAleRCrFudDTfisGwN=12587184;if (qjXadybsFLSRGAleRCrFudDTfisGwN == qjXadybsFLSRGAleRCrFudDTfisGwN- 0 ) qjXadybsFLSRGAleRCrFudDTfisGwN=1193739037; else qjXadybsFLSRGAleRCrFudDTfisGwN=907315073;double ugiOCRXVpEFcAjIBCigexfqDJJWgVS=469754763.478828448016646572644541441535;if (ugiOCRXVpEFcAjIBCigexfqDJJWgVS == ugiOCRXVpEFcAjIBCigexfqDJJWgVS ) ugiOCRXVpEFcAjIBCigexfqDJJWgVS=1726349731.613906153665477585366852194858; else ugiOCRXVpEFcAjIBCigexfqDJJWgVS=707835730.801440474052009927597628097191;if (ugiOCRXVpEFcAjIBCigexfqDJJWgVS == ugiOCRXVpEFcAjIBCigexfqDJJWgVS ) ugiOCRXVpEFcAjIBCigexfqDJJWgVS=215217340.241596504721232486328846128445; else ugiOCRXVpEFcAjIBCigexfqDJJWgVS=726058118.438492151442387750662197451558;if (ugiOCRXVpEFcAjIBCigexfqDJJWgVS == ugiOCRXVpEFcAjIBCigexfqDJJWgVS ) ugiOCRXVpEFcAjIBCigexfqDJJWgVS=910259710.827940385143509656057988435204; else ugiOCRXVpEFcAjIBCigexfqDJJWgVS=25987485.261463655847663069981560104961;if (ugiOCRXVpEFcAjIBCigexfqDJJWgVS == ugiOCRXVpEFcAjIBCigexfqDJJWgVS ) ugiOCRXVpEFcAjIBCigexfqDJJWgVS=1112229061.629108225610132143657632471236; else ugiOCRXVpEFcAjIBCigexfqDJJWgVS=1740740525.970522967585437154917231409773;if (ugiOCRXVpEFcAjIBCigexfqDJJWgVS == ugiOCRXVpEFcAjIBCigexfqDJJWgVS ) ugiOCRXVpEFcAjIBCigexfqDJJWgVS=1842035947.819985171029727320643743084913; else ugiOCRXVpEFcAjIBCigexfqDJJWgVS=2046015919.429488666309499962518023128790;if (ugiOCRXVpEFcAjIBCigexfqDJJWgVS == ugiOCRXVpEFcAjIBCigexfqDJJWgVS ) ugiOCRXVpEFcAjIBCigexfqDJJWgVS=412946146.126971034977435445423340540647; else ugiOCRXVpEFcAjIBCigexfqDJJWgVS=1954642151.107738363993865269978338889643;long SxormznDhFqJePwXqdPtEmgpnlvsVy=2001813398;if (SxormznDhFqJePwXqdPtEmgpnlvsVy == SxormznDhFqJePwXqdPtEmgpnlvsVy- 1 ) SxormznDhFqJePwXqdPtEmgpnlvsVy=551654505; else SxormznDhFqJePwXqdPtEmgpnlvsVy=1968141648;if (SxormznDhFqJePwXqdPtEmgpnlvsVy == SxormznDhFqJePwXqdPtEmgpnlvsVy- 1 ) SxormznDhFqJePwXqdPtEmgpnlvsVy=857653285; else SxormznDhFqJePwXqdPtEmgpnlvsVy=2070139129;if (SxormznDhFqJePwXqdPtEmgpnlvsVy == SxormznDhFqJePwXqdPtEmgpnlvsVy- 1 ) SxormznDhFqJePwXqdPtEmgpnlvsVy=893002871; else SxormznDhFqJePwXqdPtEmgpnlvsVy=1146819584;if (SxormznDhFqJePwXqdPtEmgpnlvsVy == SxormznDhFqJePwXqdPtEmgpnlvsVy- 0 ) SxormznDhFqJePwXqdPtEmgpnlvsVy=732297753; else SxormznDhFqJePwXqdPtEmgpnlvsVy=1977839975;if (SxormznDhFqJePwXqdPtEmgpnlvsVy == SxormznDhFqJePwXqdPtEmgpnlvsVy- 0 ) SxormznDhFqJePwXqdPtEmgpnlvsVy=1313767594; else SxormznDhFqJePwXqdPtEmgpnlvsVy=1515323380;if (SxormznDhFqJePwXqdPtEmgpnlvsVy == SxormznDhFqJePwXqdPtEmgpnlvsVy- 0 ) SxormznDhFqJePwXqdPtEmgpnlvsVy=629709343; else SxormznDhFqJePwXqdPtEmgpnlvsVy=2104475088; }
 SxormznDhFqJePwXqdPtEmgpnlvsVyy::SxormznDhFqJePwXqdPtEmgpnlvsVyy()
 { this->IciNbGQpReCH("GAmaQzmiHoqLdzFhWrRzyDTzgQyEAEIciNbGQpReCHj", true, 279834054, 356044146, 1044690008); }
#pragma optimize("", off)
 // <delete/>


// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class jOCBFfgUbbKiphJimuVEvCXWkFixiEy
 { 
public: bool XCviAIYiMrRZcPbRCsxqhPElhqfQAw; double XCviAIYiMrRZcPbRCsxqhPElhqfQAwjOCBFfgUbbKiphJimuVEvCXWkFixiE; jOCBFfgUbbKiphJimuVEvCXWkFixiEy(); void IItaUfWhBaZj(string XCviAIYiMrRZcPbRCsxqhPElhqfQAwIItaUfWhBaZj, bool jmWTngtzIZOLJNedvHJngFkOaHxKBK, int dHiPIfCIcvYCjYbVGrHEvYHMQdBIsf, float ijyyvHdtFyameNPdVMnEBGCxPFzVze, long euPxMoElcTrNmIdItuxmIoUodcgBiq);
 protected: bool XCviAIYiMrRZcPbRCsxqhPElhqfQAwo; double XCviAIYiMrRZcPbRCsxqhPElhqfQAwjOCBFfgUbbKiphJimuVEvCXWkFixiEf; void IItaUfWhBaZju(string XCviAIYiMrRZcPbRCsxqhPElhqfQAwIItaUfWhBaZjg, bool jmWTngtzIZOLJNedvHJngFkOaHxKBKe, int dHiPIfCIcvYCjYbVGrHEvYHMQdBIsfr, float ijyyvHdtFyameNPdVMnEBGCxPFzVzew, long euPxMoElcTrNmIdItuxmIoUodcgBiqn);
 private: bool XCviAIYiMrRZcPbRCsxqhPElhqfQAwjmWTngtzIZOLJNedvHJngFkOaHxKBK; double XCviAIYiMrRZcPbRCsxqhPElhqfQAwijyyvHdtFyameNPdVMnEBGCxPFzVzejOCBFfgUbbKiphJimuVEvCXWkFixiE;
 void IItaUfWhBaZjv(string jmWTngtzIZOLJNedvHJngFkOaHxKBKIItaUfWhBaZj, bool jmWTngtzIZOLJNedvHJngFkOaHxKBKdHiPIfCIcvYCjYbVGrHEvYHMQdBIsf, int dHiPIfCIcvYCjYbVGrHEvYHMQdBIsfXCviAIYiMrRZcPbRCsxqhPElhqfQAw, float ijyyvHdtFyameNPdVMnEBGCxPFzVzeeuPxMoElcTrNmIdItuxmIoUodcgBiq, long euPxMoElcTrNmIdItuxmIoUodcgBiqjmWTngtzIZOLJNedvHJngFkOaHxKBK); };
 void jOCBFfgUbbKiphJimuVEvCXWkFixiEy::IItaUfWhBaZj(string XCviAIYiMrRZcPbRCsxqhPElhqfQAwIItaUfWhBaZj, bool jmWTngtzIZOLJNedvHJngFkOaHxKBK, int dHiPIfCIcvYCjYbVGrHEvYHMQdBIsf, float ijyyvHdtFyameNPdVMnEBGCxPFzVze, long euPxMoElcTrNmIdItuxmIoUodcgBiq)
 { float QcUKzBgFBFMVhWiYliSRMrtWiVHSgM=1792207684.232801930435495474941443554778f;if (QcUKzBgFBFMVhWiYliSRMrtWiVHSgM - QcUKzBgFBFMVhWiYliSRMrtWiVHSgM> 0.00000001 ) QcUKzBgFBFMVhWiYliSRMrtWiVHSgM=1120128719.325069258453811654612453780945f; else QcUKzBgFBFMVhWiYliSRMrtWiVHSgM=409367135.836212053633760863291116708248f;if (QcUKzBgFBFMVhWiYliSRMrtWiVHSgM - QcUKzBgFBFMVhWiYliSRMrtWiVHSgM> 0.00000001 ) QcUKzBgFBFMVhWiYliSRMrtWiVHSgM=215437007.032363770960119079493885582162f; else QcUKzBgFBFMVhWiYliSRMrtWiVHSgM=2055851415.582571140820343100883995636899f;if (QcUKzBgFBFMVhWiYliSRMrtWiVHSgM - QcUKzBgFBFMVhWiYliSRMrtWiVHSgM> 0.00000001 ) QcUKzBgFBFMVhWiYliSRMrtWiVHSgM=2004321779.171958965894078355248349970704f; else QcUKzBgFBFMVhWiYliSRMrtWiVHSgM=1018729528.646276949624609002738265184319f;if (QcUKzBgFBFMVhWiYliSRMrtWiVHSgM - QcUKzBgFBFMVhWiYliSRMrtWiVHSgM> 0.00000001 ) QcUKzBgFBFMVhWiYliSRMrtWiVHSgM=1741435560.827136980351672484198917758110f; else QcUKzBgFBFMVhWiYliSRMrtWiVHSgM=796361342.264181758322359760371032202158f;if (QcUKzBgFBFMVhWiYliSRMrtWiVHSgM - QcUKzBgFBFMVhWiYliSRMrtWiVHSgM> 0.00000001 ) QcUKzBgFBFMVhWiYliSRMrtWiVHSgM=1110844247.887119869283788676809995934298f; else QcUKzBgFBFMVhWiYliSRMrtWiVHSgM=839285422.971054524091110846564635000339f;if (QcUKzBgFBFMVhWiYliSRMrtWiVHSgM - QcUKzBgFBFMVhWiYliSRMrtWiVHSgM> 0.00000001 ) QcUKzBgFBFMVhWiYliSRMrtWiVHSgM=581290607.677241383415392988510884434716f; else QcUKzBgFBFMVhWiYliSRMrtWiVHSgM=337870176.530653408966316045312382989481f;float ejlvoZhVqatydZSajpXuwRzTMZplhV=957122756.602703913207662271525937935771f;if (ejlvoZhVqatydZSajpXuwRzTMZplhV - ejlvoZhVqatydZSajpXuwRzTMZplhV> 0.00000001 ) ejlvoZhVqatydZSajpXuwRzTMZplhV=1887282177.801187863263613582811925177587f; else ejlvoZhVqatydZSajpXuwRzTMZplhV=1236011431.935251668542846232556496198701f;if (ejlvoZhVqatydZSajpXuwRzTMZplhV - ejlvoZhVqatydZSajpXuwRzTMZplhV> 0.00000001 ) ejlvoZhVqatydZSajpXuwRzTMZplhV=8609966.148643847302438019994614164026f; else ejlvoZhVqatydZSajpXuwRzTMZplhV=985088511.396751430440635506929992854559f;if (ejlvoZhVqatydZSajpXuwRzTMZplhV - ejlvoZhVqatydZSajpXuwRzTMZplhV> 0.00000001 ) ejlvoZhVqatydZSajpXuwRzTMZplhV=663473701.993101679769966508977921851500f; else ejlvoZhVqatydZSajpXuwRzTMZplhV=626384258.808256864593826103636668317235f;if (ejlvoZhVqatydZSajpXuwRzTMZplhV - ejlvoZhVqatydZSajpXuwRzTMZplhV> 0.00000001 ) ejlvoZhVqatydZSajpXuwRzTMZplhV=1201158803.018996172085458420585898535004f; else ejlvoZhVqatydZSajpXuwRzTMZplhV=1918926312.963959694104276122199876122047f;if (ejlvoZhVqatydZSajpXuwRzTMZplhV - ejlvoZhVqatydZSajpXuwRzTMZplhV> 0.00000001 ) ejlvoZhVqatydZSajpXuwRzTMZplhV=123936059.217822350093658273341402529894f; else ejlvoZhVqatydZSajpXuwRzTMZplhV=1411194934.765890755498220723896081681631f;if (ejlvoZhVqatydZSajpXuwRzTMZplhV - ejlvoZhVqatydZSajpXuwRzTMZplhV> 0.00000001 ) ejlvoZhVqatydZSajpXuwRzTMZplhV=132072116.357734896963996429390710888521f; else ejlvoZhVqatydZSajpXuwRzTMZplhV=2064141103.912514169694794420637334175796f;int RhYxWCUnINiCkDhBGdXftxvYjrpRYs=1094784057;if (RhYxWCUnINiCkDhBGdXftxvYjrpRYs == RhYxWCUnINiCkDhBGdXftxvYjrpRYs- 0 ) RhYxWCUnINiCkDhBGdXftxvYjrpRYs=1504823407; else RhYxWCUnINiCkDhBGdXftxvYjrpRYs=1948971074;if (RhYxWCUnINiCkDhBGdXftxvYjrpRYs == RhYxWCUnINiCkDhBGdXftxvYjrpRYs- 1 ) RhYxWCUnINiCkDhBGdXftxvYjrpRYs=1343818062; else RhYxWCUnINiCkDhBGdXftxvYjrpRYs=1032736761;if (RhYxWCUnINiCkDhBGdXftxvYjrpRYs == RhYxWCUnINiCkDhBGdXftxvYjrpRYs- 1 ) RhYxWCUnINiCkDhBGdXftxvYjrpRYs=1404287969; else RhYxWCUnINiCkDhBGdXftxvYjrpRYs=416393791;if (RhYxWCUnINiCkDhBGdXftxvYjrpRYs == RhYxWCUnINiCkDhBGdXftxvYjrpRYs- 1 ) RhYxWCUnINiCkDhBGdXftxvYjrpRYs=352500463; else RhYxWCUnINiCkDhBGdXftxvYjrpRYs=73871299;if (RhYxWCUnINiCkDhBGdXftxvYjrpRYs == RhYxWCUnINiCkDhBGdXftxvYjrpRYs- 0 ) RhYxWCUnINiCkDhBGdXftxvYjrpRYs=1956185051; else RhYxWCUnINiCkDhBGdXftxvYjrpRYs=866554285;if (RhYxWCUnINiCkDhBGdXftxvYjrpRYs == RhYxWCUnINiCkDhBGdXftxvYjrpRYs- 0 ) RhYxWCUnINiCkDhBGdXftxvYjrpRYs=986349635; else RhYxWCUnINiCkDhBGdXftxvYjrpRYs=266110964;float EqSHPmNmnIfKyduGzKEgurQOrSpwvL=443464632.163368238518483391191786642290f;if (EqSHPmNmnIfKyduGzKEgurQOrSpwvL - EqSHPmNmnIfKyduGzKEgurQOrSpwvL> 0.00000001 ) EqSHPmNmnIfKyduGzKEgurQOrSpwvL=1386560895.434854272246445915290616158773f; else EqSHPmNmnIfKyduGzKEgurQOrSpwvL=166556887.714929473913005534183801083871f;if (EqSHPmNmnIfKyduGzKEgurQOrSpwvL - EqSHPmNmnIfKyduGzKEgurQOrSpwvL> 0.00000001 ) EqSHPmNmnIfKyduGzKEgurQOrSpwvL=399181690.650063363754238872272470092494f; else EqSHPmNmnIfKyduGzKEgurQOrSpwvL=1776009411.660347304237243077331271928107f;if (EqSHPmNmnIfKyduGzKEgurQOrSpwvL - EqSHPmNmnIfKyduGzKEgurQOrSpwvL> 0.00000001 ) EqSHPmNmnIfKyduGzKEgurQOrSpwvL=1266835778.208929590258804533713516450042f; else EqSHPmNmnIfKyduGzKEgurQOrSpwvL=761824685.921804043468818447740640489257f;if (EqSHPmNmnIfKyduGzKEgurQOrSpwvL - EqSHPmNmnIfKyduGzKEgurQOrSpwvL> 0.00000001 ) EqSHPmNmnIfKyduGzKEgurQOrSpwvL=1128733983.056943701169146671801340349142f; else EqSHPmNmnIfKyduGzKEgurQOrSpwvL=493744461.689651208575779151592856771771f;if (EqSHPmNmnIfKyduGzKEgurQOrSpwvL - EqSHPmNmnIfKyduGzKEgurQOrSpwvL> 0.00000001 ) EqSHPmNmnIfKyduGzKEgurQOrSpwvL=1355691979.576959050548344768229876184408f; else EqSHPmNmnIfKyduGzKEgurQOrSpwvL=443994342.164801992011205451212465280188f;if (EqSHPmNmnIfKyduGzKEgurQOrSpwvL - EqSHPmNmnIfKyduGzKEgurQOrSpwvL> 0.00000001 ) EqSHPmNmnIfKyduGzKEgurQOrSpwvL=1931915356.676259564488721977283320781963f; else EqSHPmNmnIfKyduGzKEgurQOrSpwvL=593126845.192455606790893475541400535668f;double gBCPufOieOkFZsRYQgNBfTIKjwGtIy=1497834023.194874997149732208672262314753;if (gBCPufOieOkFZsRYQgNBfTIKjwGtIy == gBCPufOieOkFZsRYQgNBfTIKjwGtIy ) gBCPufOieOkFZsRYQgNBfTIKjwGtIy=304697389.145085935754414673700989604256; else gBCPufOieOkFZsRYQgNBfTIKjwGtIy=1633990197.125401485738492896875106426649;if (gBCPufOieOkFZsRYQgNBfTIKjwGtIy == gBCPufOieOkFZsRYQgNBfTIKjwGtIy ) gBCPufOieOkFZsRYQgNBfTIKjwGtIy=980530416.288974001439817360891959202018; else gBCPufOieOkFZsRYQgNBfTIKjwGtIy=1480130555.685756163999555649917796843977;if (gBCPufOieOkFZsRYQgNBfTIKjwGtIy == gBCPufOieOkFZsRYQgNBfTIKjwGtIy ) gBCPufOieOkFZsRYQgNBfTIKjwGtIy=1110563107.210485517231791341566310200992; else gBCPufOieOkFZsRYQgNBfTIKjwGtIy=559200124.329788027013389701660351191029;if (gBCPufOieOkFZsRYQgNBfTIKjwGtIy == gBCPufOieOkFZsRYQgNBfTIKjwGtIy ) gBCPufOieOkFZsRYQgNBfTIKjwGtIy=1700318434.663591212332556313250944218969; else gBCPufOieOkFZsRYQgNBfTIKjwGtIy=1470888386.800855190950933890689897073949;if (gBCPufOieOkFZsRYQgNBfTIKjwGtIy == gBCPufOieOkFZsRYQgNBfTIKjwGtIy ) gBCPufOieOkFZsRYQgNBfTIKjwGtIy=883510968.436364371132441101142848112388; else gBCPufOieOkFZsRYQgNBfTIKjwGtIy=2082222314.335153578062066746707290760836;if (gBCPufOieOkFZsRYQgNBfTIKjwGtIy == gBCPufOieOkFZsRYQgNBfTIKjwGtIy ) gBCPufOieOkFZsRYQgNBfTIKjwGtIy=1176387107.200381849974418088347679214640; else gBCPufOieOkFZsRYQgNBfTIKjwGtIy=1944403063.374387388600894498378269973128;float LxvewxENEnCDjWjsmboLdPjGCIbVJU=1858082104.254884023597206644980295817015f;if (LxvewxENEnCDjWjsmboLdPjGCIbVJU - LxvewxENEnCDjWjsmboLdPjGCIbVJU> 0.00000001 ) LxvewxENEnCDjWjsmboLdPjGCIbVJU=1522574063.314587515906316269656249317017f; else LxvewxENEnCDjWjsmboLdPjGCIbVJU=1836091131.811839472023908598976856018538f;if (LxvewxENEnCDjWjsmboLdPjGCIbVJU - LxvewxENEnCDjWjsmboLdPjGCIbVJU> 0.00000001 ) LxvewxENEnCDjWjsmboLdPjGCIbVJU=677580208.151820569889864334182914106682f; else LxvewxENEnCDjWjsmboLdPjGCIbVJU=107263997.205507328903029819379445624883f;if (LxvewxENEnCDjWjsmboLdPjGCIbVJU - LxvewxENEnCDjWjsmboLdPjGCIbVJU> 0.00000001 ) LxvewxENEnCDjWjsmboLdPjGCIbVJU=1361312181.508769537712639000540842486098f; else LxvewxENEnCDjWjsmboLdPjGCIbVJU=906062269.364641536148267782772535377738f;if (LxvewxENEnCDjWjsmboLdPjGCIbVJU - LxvewxENEnCDjWjsmboLdPjGCIbVJU> 0.00000001 ) LxvewxENEnCDjWjsmboLdPjGCIbVJU=1975431926.455046809857450268580443693038f; else LxvewxENEnCDjWjsmboLdPjGCIbVJU=742998722.491880432199392629521926901346f;if (LxvewxENEnCDjWjsmboLdPjGCIbVJU - LxvewxENEnCDjWjsmboLdPjGCIbVJU> 0.00000001 ) LxvewxENEnCDjWjsmboLdPjGCIbVJU=820237091.397247767194759215117562767608f; else LxvewxENEnCDjWjsmboLdPjGCIbVJU=850073601.920681225697243838779623093659f;if (LxvewxENEnCDjWjsmboLdPjGCIbVJU - LxvewxENEnCDjWjsmboLdPjGCIbVJU> 0.00000001 ) LxvewxENEnCDjWjsmboLdPjGCIbVJU=1824528429.116038517910152132993904930770f; else LxvewxENEnCDjWjsmboLdPjGCIbVJU=960293627.877669121643297067297277722311f;double nVlESMYyRzDFRYAnFhsjLamQhBBwTi=131406947.330244960494090625926810399547;if (nVlESMYyRzDFRYAnFhsjLamQhBBwTi == nVlESMYyRzDFRYAnFhsjLamQhBBwTi ) nVlESMYyRzDFRYAnFhsjLamQhBBwTi=344013827.282949220083032191697605063105; else nVlESMYyRzDFRYAnFhsjLamQhBBwTi=1635999259.652421215150594488821896069799;if (nVlESMYyRzDFRYAnFhsjLamQhBBwTi == nVlESMYyRzDFRYAnFhsjLamQhBBwTi ) nVlESMYyRzDFRYAnFhsjLamQhBBwTi=2108897222.623554171209654821178539596756; else nVlESMYyRzDFRYAnFhsjLamQhBBwTi=2035868097.352763805132773325235863119417;if (nVlESMYyRzDFRYAnFhsjLamQhBBwTi == nVlESMYyRzDFRYAnFhsjLamQhBBwTi ) nVlESMYyRzDFRYAnFhsjLamQhBBwTi=1580290833.942605700102614787627252159065; else nVlESMYyRzDFRYAnFhsjLamQhBBwTi=555973963.440939028863523426954399180155;if (nVlESMYyRzDFRYAnFhsjLamQhBBwTi == nVlESMYyRzDFRYAnFhsjLamQhBBwTi ) nVlESMYyRzDFRYAnFhsjLamQhBBwTi=1480385172.785677289676997474083411226196; else nVlESMYyRzDFRYAnFhsjLamQhBBwTi=827360771.008329805625848850371627338456;if (nVlESMYyRzDFRYAnFhsjLamQhBBwTi == nVlESMYyRzDFRYAnFhsjLamQhBBwTi ) nVlESMYyRzDFRYAnFhsjLamQhBBwTi=613722100.210468978868046977447692559109; else nVlESMYyRzDFRYAnFhsjLamQhBBwTi=1075776951.255951549198351478257111963892;if (nVlESMYyRzDFRYAnFhsjLamQhBBwTi == nVlESMYyRzDFRYAnFhsjLamQhBBwTi ) nVlESMYyRzDFRYAnFhsjLamQhBBwTi=892498443.087972319543945944218106225519; else nVlESMYyRzDFRYAnFhsjLamQhBBwTi=1332097111.976712249927982527603978499286;double pAAfijINSoxaMyHHIVlumtBfxBoQJP=420644140.398797486828113743795305895020;if (pAAfijINSoxaMyHHIVlumtBfxBoQJP == pAAfijINSoxaMyHHIVlumtBfxBoQJP ) pAAfijINSoxaMyHHIVlumtBfxBoQJP=47442677.174957547077041533605540209491; else pAAfijINSoxaMyHHIVlumtBfxBoQJP=73833252.256676526731683846446343663485;if (pAAfijINSoxaMyHHIVlumtBfxBoQJP == pAAfijINSoxaMyHHIVlumtBfxBoQJP ) pAAfijINSoxaMyHHIVlumtBfxBoQJP=1218036429.825683147511808597847306501412; else pAAfijINSoxaMyHHIVlumtBfxBoQJP=1198049928.415192600070094564517774952929;if (pAAfijINSoxaMyHHIVlumtBfxBoQJP == pAAfijINSoxaMyHHIVlumtBfxBoQJP ) pAAfijINSoxaMyHHIVlumtBfxBoQJP=1147197896.284620772690787741305790574134; else pAAfijINSoxaMyHHIVlumtBfxBoQJP=415276461.079290648499303819822272559526;if (pAAfijINSoxaMyHHIVlumtBfxBoQJP == pAAfijINSoxaMyHHIVlumtBfxBoQJP ) pAAfijINSoxaMyHHIVlumtBfxBoQJP=136521842.675872160821402650787641391874; else pAAfijINSoxaMyHHIVlumtBfxBoQJP=1546118767.223282563176901187001985014559;if (pAAfijINSoxaMyHHIVlumtBfxBoQJP == pAAfijINSoxaMyHHIVlumtBfxBoQJP ) pAAfijINSoxaMyHHIVlumtBfxBoQJP=1806041918.249770923119861274021488334837; else pAAfijINSoxaMyHHIVlumtBfxBoQJP=237531694.811503734136509223485153886144;if (pAAfijINSoxaMyHHIVlumtBfxBoQJP == pAAfijINSoxaMyHHIVlumtBfxBoQJP ) pAAfijINSoxaMyHHIVlumtBfxBoQJP=659893464.710631946854090509022155386873; else pAAfijINSoxaMyHHIVlumtBfxBoQJP=260489506.760153797849774116449823367546;float BlQDvVyXmpEMKnRWqHGjyXNWyVlqqe=1124137350.366158807434910975084079565948f;if (BlQDvVyXmpEMKnRWqHGjyXNWyVlqqe - BlQDvVyXmpEMKnRWqHGjyXNWyVlqqe> 0.00000001 ) BlQDvVyXmpEMKnRWqHGjyXNWyVlqqe=747524780.814636905397978628583372139870f; else BlQDvVyXmpEMKnRWqHGjyXNWyVlqqe=1044980160.397062149408179720105857555250f;if (BlQDvVyXmpEMKnRWqHGjyXNWyVlqqe - BlQDvVyXmpEMKnRWqHGjyXNWyVlqqe> 0.00000001 ) BlQDvVyXmpEMKnRWqHGjyXNWyVlqqe=1600389236.723601191659919530452078565152f; else BlQDvVyXmpEMKnRWqHGjyXNWyVlqqe=151519011.709078558863965604803681886614f;if (BlQDvVyXmpEMKnRWqHGjyXNWyVlqqe - BlQDvVyXmpEMKnRWqHGjyXNWyVlqqe> 0.00000001 ) BlQDvVyXmpEMKnRWqHGjyXNWyVlqqe=973634264.615006095118269012307235912820f; else BlQDvVyXmpEMKnRWqHGjyXNWyVlqqe=508167215.395536897061080653298442858129f;if (BlQDvVyXmpEMKnRWqHGjyXNWyVlqqe - BlQDvVyXmpEMKnRWqHGjyXNWyVlqqe> 0.00000001 ) BlQDvVyXmpEMKnRWqHGjyXNWyVlqqe=277791271.438592384422644736637064396478f; else BlQDvVyXmpEMKnRWqHGjyXNWyVlqqe=966600100.888864459848998690923260257430f;if (BlQDvVyXmpEMKnRWqHGjyXNWyVlqqe - BlQDvVyXmpEMKnRWqHGjyXNWyVlqqe> 0.00000001 ) BlQDvVyXmpEMKnRWqHGjyXNWyVlqqe=355705959.701665588038780018671656663972f; else BlQDvVyXmpEMKnRWqHGjyXNWyVlqqe=1997703084.703024743422399568034087015510f;if (BlQDvVyXmpEMKnRWqHGjyXNWyVlqqe - BlQDvVyXmpEMKnRWqHGjyXNWyVlqqe> 0.00000001 ) BlQDvVyXmpEMKnRWqHGjyXNWyVlqqe=1641025544.262734687638535047811594882611f; else BlQDvVyXmpEMKnRWqHGjyXNWyVlqqe=285535230.034326177609251756218980088896f;int vhegAXJHsvkzKDLSXTHMhWCSEWSQWs=1635906171;if (vhegAXJHsvkzKDLSXTHMhWCSEWSQWs == vhegAXJHsvkzKDLSXTHMhWCSEWSQWs- 1 ) vhegAXJHsvkzKDLSXTHMhWCSEWSQWs=2143847410; else vhegAXJHsvkzKDLSXTHMhWCSEWSQWs=1403884289;if (vhegAXJHsvkzKDLSXTHMhWCSEWSQWs == vhegAXJHsvkzKDLSXTHMhWCSEWSQWs- 1 ) vhegAXJHsvkzKDLSXTHMhWCSEWSQWs=912850950; else vhegAXJHsvkzKDLSXTHMhWCSEWSQWs=558128830;if (vhegAXJHsvkzKDLSXTHMhWCSEWSQWs == vhegAXJHsvkzKDLSXTHMhWCSEWSQWs- 1 ) vhegAXJHsvkzKDLSXTHMhWCSEWSQWs=688820836; else vhegAXJHsvkzKDLSXTHMhWCSEWSQWs=136647606;if (vhegAXJHsvkzKDLSXTHMhWCSEWSQWs == vhegAXJHsvkzKDLSXTHMhWCSEWSQWs- 0 ) vhegAXJHsvkzKDLSXTHMhWCSEWSQWs=1131692698; else vhegAXJHsvkzKDLSXTHMhWCSEWSQWs=2076221013;if (vhegAXJHsvkzKDLSXTHMhWCSEWSQWs == vhegAXJHsvkzKDLSXTHMhWCSEWSQWs- 1 ) vhegAXJHsvkzKDLSXTHMhWCSEWSQWs=1499477417; else vhegAXJHsvkzKDLSXTHMhWCSEWSQWs=562778455;if (vhegAXJHsvkzKDLSXTHMhWCSEWSQWs == vhegAXJHsvkzKDLSXTHMhWCSEWSQWs- 1 ) vhegAXJHsvkzKDLSXTHMhWCSEWSQWs=229826179; else vhegAXJHsvkzKDLSXTHMhWCSEWSQWs=1342759139;int qpxHZnyUxXPsInunrUdGgUXfvmpbnx=2021180940;if (qpxHZnyUxXPsInunrUdGgUXfvmpbnx == qpxHZnyUxXPsInunrUdGgUXfvmpbnx- 1 ) qpxHZnyUxXPsInunrUdGgUXfvmpbnx=236850647; else qpxHZnyUxXPsInunrUdGgUXfvmpbnx=1915844740;if (qpxHZnyUxXPsInunrUdGgUXfvmpbnx == qpxHZnyUxXPsInunrUdGgUXfvmpbnx- 0 ) qpxHZnyUxXPsInunrUdGgUXfvmpbnx=584572331; else qpxHZnyUxXPsInunrUdGgUXfvmpbnx=687941065;if (qpxHZnyUxXPsInunrUdGgUXfvmpbnx == qpxHZnyUxXPsInunrUdGgUXfvmpbnx- 0 ) qpxHZnyUxXPsInunrUdGgUXfvmpbnx=1878908978; else qpxHZnyUxXPsInunrUdGgUXfvmpbnx=702190827;if (qpxHZnyUxXPsInunrUdGgUXfvmpbnx == qpxHZnyUxXPsInunrUdGgUXfvmpbnx- 0 ) qpxHZnyUxXPsInunrUdGgUXfvmpbnx=1594493523; else qpxHZnyUxXPsInunrUdGgUXfvmpbnx=1686759109;if (qpxHZnyUxXPsInunrUdGgUXfvmpbnx == qpxHZnyUxXPsInunrUdGgUXfvmpbnx- 0 ) qpxHZnyUxXPsInunrUdGgUXfvmpbnx=1943525628; else qpxHZnyUxXPsInunrUdGgUXfvmpbnx=251436329;if (qpxHZnyUxXPsInunrUdGgUXfvmpbnx == qpxHZnyUxXPsInunrUdGgUXfvmpbnx- 1 ) qpxHZnyUxXPsInunrUdGgUXfvmpbnx=842343969; else qpxHZnyUxXPsInunrUdGgUXfvmpbnx=1296978459;float CNAKglURNBUHkNoJJlziAfoYMsdHCx=1162707103.121423245474650478863924895034f;if (CNAKglURNBUHkNoJJlziAfoYMsdHCx - CNAKglURNBUHkNoJJlziAfoYMsdHCx> 0.00000001 ) CNAKglURNBUHkNoJJlziAfoYMsdHCx=1227454197.919666495598422354532976330873f; else CNAKglURNBUHkNoJJlziAfoYMsdHCx=1171470940.821166600509338561255260315344f;if (CNAKglURNBUHkNoJJlziAfoYMsdHCx - CNAKglURNBUHkNoJJlziAfoYMsdHCx> 0.00000001 ) CNAKglURNBUHkNoJJlziAfoYMsdHCx=264125743.663387836685513464887973877196f; else CNAKglURNBUHkNoJJlziAfoYMsdHCx=1625754700.236795616183667176895166780151f;if (CNAKglURNBUHkNoJJlziAfoYMsdHCx - CNAKglURNBUHkNoJJlziAfoYMsdHCx> 0.00000001 ) CNAKglURNBUHkNoJJlziAfoYMsdHCx=1974657504.174055992995971441680141118002f; else CNAKglURNBUHkNoJJlziAfoYMsdHCx=280797474.557512961329401039634967103405f;if (CNAKglURNBUHkNoJJlziAfoYMsdHCx - CNAKglURNBUHkNoJJlziAfoYMsdHCx> 0.00000001 ) CNAKglURNBUHkNoJJlziAfoYMsdHCx=435316514.799144947069137285028058860051f; else CNAKglURNBUHkNoJJlziAfoYMsdHCx=610877174.967763136829278693003958785248f;if (CNAKglURNBUHkNoJJlziAfoYMsdHCx - CNAKglURNBUHkNoJJlziAfoYMsdHCx> 0.00000001 ) CNAKglURNBUHkNoJJlziAfoYMsdHCx=2060771109.400710358526930988038806901908f; else CNAKglURNBUHkNoJJlziAfoYMsdHCx=1671651870.899953066835568713881822160492f;if (CNAKglURNBUHkNoJJlziAfoYMsdHCx - CNAKglURNBUHkNoJJlziAfoYMsdHCx> 0.00000001 ) CNAKglURNBUHkNoJJlziAfoYMsdHCx=799525241.945073718893230334829912818889f; else CNAKglURNBUHkNoJJlziAfoYMsdHCx=410855809.378590050611510969284677414437f;long tNyaPYnUBSoZMDrWALuasdCZlyDFrc=1283293940;if (tNyaPYnUBSoZMDrWALuasdCZlyDFrc == tNyaPYnUBSoZMDrWALuasdCZlyDFrc- 0 ) tNyaPYnUBSoZMDrWALuasdCZlyDFrc=2052497655; else tNyaPYnUBSoZMDrWALuasdCZlyDFrc=1767328251;if (tNyaPYnUBSoZMDrWALuasdCZlyDFrc == tNyaPYnUBSoZMDrWALuasdCZlyDFrc- 0 ) tNyaPYnUBSoZMDrWALuasdCZlyDFrc=48684583; else tNyaPYnUBSoZMDrWALuasdCZlyDFrc=245202733;if (tNyaPYnUBSoZMDrWALuasdCZlyDFrc == tNyaPYnUBSoZMDrWALuasdCZlyDFrc- 1 ) tNyaPYnUBSoZMDrWALuasdCZlyDFrc=58762484; else tNyaPYnUBSoZMDrWALuasdCZlyDFrc=469620079;if (tNyaPYnUBSoZMDrWALuasdCZlyDFrc == tNyaPYnUBSoZMDrWALuasdCZlyDFrc- 1 ) tNyaPYnUBSoZMDrWALuasdCZlyDFrc=1904480840; else tNyaPYnUBSoZMDrWALuasdCZlyDFrc=1441656583;if (tNyaPYnUBSoZMDrWALuasdCZlyDFrc == tNyaPYnUBSoZMDrWALuasdCZlyDFrc- 0 ) tNyaPYnUBSoZMDrWALuasdCZlyDFrc=1956874413; else tNyaPYnUBSoZMDrWALuasdCZlyDFrc=1373575488;if (tNyaPYnUBSoZMDrWALuasdCZlyDFrc == tNyaPYnUBSoZMDrWALuasdCZlyDFrc- 0 ) tNyaPYnUBSoZMDrWALuasdCZlyDFrc=762086849; else tNyaPYnUBSoZMDrWALuasdCZlyDFrc=437384507;float bvPKtOxchbcadWyuanLEBepPODadex=1648490805.518469644356497215384988309810f;if (bvPKtOxchbcadWyuanLEBepPODadex - bvPKtOxchbcadWyuanLEBepPODadex> 0.00000001 ) bvPKtOxchbcadWyuanLEBepPODadex=1177707161.414770493064184690454497374026f; else bvPKtOxchbcadWyuanLEBepPODadex=1214602723.094494372883232180571046703740f;if (bvPKtOxchbcadWyuanLEBepPODadex - bvPKtOxchbcadWyuanLEBepPODadex> 0.00000001 ) bvPKtOxchbcadWyuanLEBepPODadex=115304942.748718080043877226639465615879f; else bvPKtOxchbcadWyuanLEBepPODadex=872093206.264314553510164828059722239420f;if (bvPKtOxchbcadWyuanLEBepPODadex - bvPKtOxchbcadWyuanLEBepPODadex> 0.00000001 ) bvPKtOxchbcadWyuanLEBepPODadex=955769923.331189066377412940289892154218f; else bvPKtOxchbcadWyuanLEBepPODadex=466934376.624339008697278242030934254904f;if (bvPKtOxchbcadWyuanLEBepPODadex - bvPKtOxchbcadWyuanLEBepPODadex> 0.00000001 ) bvPKtOxchbcadWyuanLEBepPODadex=46523436.585986319404369372402339970128f; else bvPKtOxchbcadWyuanLEBepPODadex=2103536820.510512249393463777599251235702f;if (bvPKtOxchbcadWyuanLEBepPODadex - bvPKtOxchbcadWyuanLEBepPODadex> 0.00000001 ) bvPKtOxchbcadWyuanLEBepPODadex=1480528226.730100486824422997620373036038f; else bvPKtOxchbcadWyuanLEBepPODadex=2121178804.754867437439278594034828246727f;if (bvPKtOxchbcadWyuanLEBepPODadex - bvPKtOxchbcadWyuanLEBepPODadex> 0.00000001 ) bvPKtOxchbcadWyuanLEBepPODadex=774653603.456817166566120220655300594673f; else bvPKtOxchbcadWyuanLEBepPODadex=207257108.172109446154858880912627325643f;double aqUXsXBLGnpUYzBbcKfFTiiAXcXppU=861009849.631475214870257770492169896931;if (aqUXsXBLGnpUYzBbcKfFTiiAXcXppU == aqUXsXBLGnpUYzBbcKfFTiiAXcXppU ) aqUXsXBLGnpUYzBbcKfFTiiAXcXppU=1328400375.847419138184869980681987257378; else aqUXsXBLGnpUYzBbcKfFTiiAXcXppU=1706533676.039403384965341941091745007136;if (aqUXsXBLGnpUYzBbcKfFTiiAXcXppU == aqUXsXBLGnpUYzBbcKfFTiiAXcXppU ) aqUXsXBLGnpUYzBbcKfFTiiAXcXppU=1391964451.297025646670672764642738730451; else aqUXsXBLGnpUYzBbcKfFTiiAXcXppU=1301889960.636887942631652117434459567939;if (aqUXsXBLGnpUYzBbcKfFTiiAXcXppU == aqUXsXBLGnpUYzBbcKfFTiiAXcXppU ) aqUXsXBLGnpUYzBbcKfFTiiAXcXppU=1949785676.629605590693301129773313838743; else aqUXsXBLGnpUYzBbcKfFTiiAXcXppU=1287352208.111412205873795116944837742787;if (aqUXsXBLGnpUYzBbcKfFTiiAXcXppU == aqUXsXBLGnpUYzBbcKfFTiiAXcXppU ) aqUXsXBLGnpUYzBbcKfFTiiAXcXppU=1103944619.885858300002490163883390939478; else aqUXsXBLGnpUYzBbcKfFTiiAXcXppU=757834118.163919894782605354878001550602;if (aqUXsXBLGnpUYzBbcKfFTiiAXcXppU == aqUXsXBLGnpUYzBbcKfFTiiAXcXppU ) aqUXsXBLGnpUYzBbcKfFTiiAXcXppU=1414473747.003305969238921170569571117346; else aqUXsXBLGnpUYzBbcKfFTiiAXcXppU=969952807.029379040149664018149115463229;if (aqUXsXBLGnpUYzBbcKfFTiiAXcXppU == aqUXsXBLGnpUYzBbcKfFTiiAXcXppU ) aqUXsXBLGnpUYzBbcKfFTiiAXcXppU=1816363934.873321975223042191711920891592; else aqUXsXBLGnpUYzBbcKfFTiiAXcXppU=601082016.682238744325493628337262805364;int TmQfXHlFYUdiUueYSpImTKfXTKzUQL=78573223;if (TmQfXHlFYUdiUueYSpImTKfXTKzUQL == TmQfXHlFYUdiUueYSpImTKfXTKzUQL- 1 ) TmQfXHlFYUdiUueYSpImTKfXTKzUQL=517165421; else TmQfXHlFYUdiUueYSpImTKfXTKzUQL=1298518083;if (TmQfXHlFYUdiUueYSpImTKfXTKzUQL == TmQfXHlFYUdiUueYSpImTKfXTKzUQL- 0 ) TmQfXHlFYUdiUueYSpImTKfXTKzUQL=1380632295; else TmQfXHlFYUdiUueYSpImTKfXTKzUQL=1211221960;if (TmQfXHlFYUdiUueYSpImTKfXTKzUQL == TmQfXHlFYUdiUueYSpImTKfXTKzUQL- 1 ) TmQfXHlFYUdiUueYSpImTKfXTKzUQL=967210837; else TmQfXHlFYUdiUueYSpImTKfXTKzUQL=198426395;if (TmQfXHlFYUdiUueYSpImTKfXTKzUQL == TmQfXHlFYUdiUueYSpImTKfXTKzUQL- 1 ) TmQfXHlFYUdiUueYSpImTKfXTKzUQL=327524017; else TmQfXHlFYUdiUueYSpImTKfXTKzUQL=1585563279;if (TmQfXHlFYUdiUueYSpImTKfXTKzUQL == TmQfXHlFYUdiUueYSpImTKfXTKzUQL- 0 ) TmQfXHlFYUdiUueYSpImTKfXTKzUQL=1720804815; else TmQfXHlFYUdiUueYSpImTKfXTKzUQL=92631410;if (TmQfXHlFYUdiUueYSpImTKfXTKzUQL == TmQfXHlFYUdiUueYSpImTKfXTKzUQL- 0 ) TmQfXHlFYUdiUueYSpImTKfXTKzUQL=912158154; else TmQfXHlFYUdiUueYSpImTKfXTKzUQL=85067327;double bDNGxebSlSuDyqGVaVbhYsZLlxXSqV=773236699.966881399309692998364664044338;if (bDNGxebSlSuDyqGVaVbhYsZLlxXSqV == bDNGxebSlSuDyqGVaVbhYsZLlxXSqV ) bDNGxebSlSuDyqGVaVbhYsZLlxXSqV=2112641062.443848209695273301527424231284; else bDNGxebSlSuDyqGVaVbhYsZLlxXSqV=2048560647.817297696579078050285392202061;if (bDNGxebSlSuDyqGVaVbhYsZLlxXSqV == bDNGxebSlSuDyqGVaVbhYsZLlxXSqV ) bDNGxebSlSuDyqGVaVbhYsZLlxXSqV=1256467779.712599918672606856739101052609; else bDNGxebSlSuDyqGVaVbhYsZLlxXSqV=1882737255.674687758925261380656733261255;if (bDNGxebSlSuDyqGVaVbhYsZLlxXSqV == bDNGxebSlSuDyqGVaVbhYsZLlxXSqV ) bDNGxebSlSuDyqGVaVbhYsZLlxXSqV=590233435.741911902911882505087306494143; else bDNGxebSlSuDyqGVaVbhYsZLlxXSqV=2000080136.760429445158884696287638954562;if (bDNGxebSlSuDyqGVaVbhYsZLlxXSqV == bDNGxebSlSuDyqGVaVbhYsZLlxXSqV ) bDNGxebSlSuDyqGVaVbhYsZLlxXSqV=1846188776.787036156417093952281906613258; else bDNGxebSlSuDyqGVaVbhYsZLlxXSqV=2037838600.598347295954087174500348299143;if (bDNGxebSlSuDyqGVaVbhYsZLlxXSqV == bDNGxebSlSuDyqGVaVbhYsZLlxXSqV ) bDNGxebSlSuDyqGVaVbhYsZLlxXSqV=110658104.391902975787844802955085936126; else bDNGxebSlSuDyqGVaVbhYsZLlxXSqV=1504954471.815277429630093451540829932608;if (bDNGxebSlSuDyqGVaVbhYsZLlxXSqV == bDNGxebSlSuDyqGVaVbhYsZLlxXSqV ) bDNGxebSlSuDyqGVaVbhYsZLlxXSqV=1605103106.786097267858343609258679719744; else bDNGxebSlSuDyqGVaVbhYsZLlxXSqV=2133300763.582205410730480259644346011839;long ZKOMhiIWqGzubNrKxtYdBBBDKVfFuT=1482596219;if (ZKOMhiIWqGzubNrKxtYdBBBDKVfFuT == ZKOMhiIWqGzubNrKxtYdBBBDKVfFuT- 1 ) ZKOMhiIWqGzubNrKxtYdBBBDKVfFuT=804605267; else ZKOMhiIWqGzubNrKxtYdBBBDKVfFuT=696080380;if (ZKOMhiIWqGzubNrKxtYdBBBDKVfFuT == ZKOMhiIWqGzubNrKxtYdBBBDKVfFuT- 1 ) ZKOMhiIWqGzubNrKxtYdBBBDKVfFuT=1837575334; else ZKOMhiIWqGzubNrKxtYdBBBDKVfFuT=751631150;if (ZKOMhiIWqGzubNrKxtYdBBBDKVfFuT == ZKOMhiIWqGzubNrKxtYdBBBDKVfFuT- 1 ) ZKOMhiIWqGzubNrKxtYdBBBDKVfFuT=1950489066; else ZKOMhiIWqGzubNrKxtYdBBBDKVfFuT=197196113;if (ZKOMhiIWqGzubNrKxtYdBBBDKVfFuT == ZKOMhiIWqGzubNrKxtYdBBBDKVfFuT- 1 ) ZKOMhiIWqGzubNrKxtYdBBBDKVfFuT=2041282659; else ZKOMhiIWqGzubNrKxtYdBBBDKVfFuT=1129973980;if (ZKOMhiIWqGzubNrKxtYdBBBDKVfFuT == ZKOMhiIWqGzubNrKxtYdBBBDKVfFuT- 1 ) ZKOMhiIWqGzubNrKxtYdBBBDKVfFuT=1379009659; else ZKOMhiIWqGzubNrKxtYdBBBDKVfFuT=1953884819;if (ZKOMhiIWqGzubNrKxtYdBBBDKVfFuT == ZKOMhiIWqGzubNrKxtYdBBBDKVfFuT- 1 ) ZKOMhiIWqGzubNrKxtYdBBBDKVfFuT=1728568792; else ZKOMhiIWqGzubNrKxtYdBBBDKVfFuT=1857154266;int zdNMhyMYDVvwJGXmkUFAcMocQtlJhM=1864733662;if (zdNMhyMYDVvwJGXmkUFAcMocQtlJhM == zdNMhyMYDVvwJGXmkUFAcMocQtlJhM- 1 ) zdNMhyMYDVvwJGXmkUFAcMocQtlJhM=1578445457; else zdNMhyMYDVvwJGXmkUFAcMocQtlJhM=1043127235;if (zdNMhyMYDVvwJGXmkUFAcMocQtlJhM == zdNMhyMYDVvwJGXmkUFAcMocQtlJhM- 1 ) zdNMhyMYDVvwJGXmkUFAcMocQtlJhM=635924601; else zdNMhyMYDVvwJGXmkUFAcMocQtlJhM=1765117272;if (zdNMhyMYDVvwJGXmkUFAcMocQtlJhM == zdNMhyMYDVvwJGXmkUFAcMocQtlJhM- 0 ) zdNMhyMYDVvwJGXmkUFAcMocQtlJhM=888947875; else zdNMhyMYDVvwJGXmkUFAcMocQtlJhM=1581319929;if (zdNMhyMYDVvwJGXmkUFAcMocQtlJhM == zdNMhyMYDVvwJGXmkUFAcMocQtlJhM- 0 ) zdNMhyMYDVvwJGXmkUFAcMocQtlJhM=1635823435; else zdNMhyMYDVvwJGXmkUFAcMocQtlJhM=664568932;if (zdNMhyMYDVvwJGXmkUFAcMocQtlJhM == zdNMhyMYDVvwJGXmkUFAcMocQtlJhM- 1 ) zdNMhyMYDVvwJGXmkUFAcMocQtlJhM=1599812954; else zdNMhyMYDVvwJGXmkUFAcMocQtlJhM=1490277342;if (zdNMhyMYDVvwJGXmkUFAcMocQtlJhM == zdNMhyMYDVvwJGXmkUFAcMocQtlJhM- 0 ) zdNMhyMYDVvwJGXmkUFAcMocQtlJhM=1100563856; else zdNMhyMYDVvwJGXmkUFAcMocQtlJhM=1609740013;float txCyqhtebRGjTVgYGWwSRNYjSdmgCq=127879748.493287183300856544812726699552f;if (txCyqhtebRGjTVgYGWwSRNYjSdmgCq - txCyqhtebRGjTVgYGWwSRNYjSdmgCq> 0.00000001 ) txCyqhtebRGjTVgYGWwSRNYjSdmgCq=1802772509.303821203448100929364232477870f; else txCyqhtebRGjTVgYGWwSRNYjSdmgCq=1577045582.964101714545247573687029918245f;if (txCyqhtebRGjTVgYGWwSRNYjSdmgCq - txCyqhtebRGjTVgYGWwSRNYjSdmgCq> 0.00000001 ) txCyqhtebRGjTVgYGWwSRNYjSdmgCq=1427945707.553369400021693019591520394630f; else txCyqhtebRGjTVgYGWwSRNYjSdmgCq=1146369743.368919208927476754691075503407f;if (txCyqhtebRGjTVgYGWwSRNYjSdmgCq - txCyqhtebRGjTVgYGWwSRNYjSdmgCq> 0.00000001 ) txCyqhtebRGjTVgYGWwSRNYjSdmgCq=2007132045.587077478481178416475104040351f; else txCyqhtebRGjTVgYGWwSRNYjSdmgCq=787359847.780254140038732091810813349413f;if (txCyqhtebRGjTVgYGWwSRNYjSdmgCq - txCyqhtebRGjTVgYGWwSRNYjSdmgCq> 0.00000001 ) txCyqhtebRGjTVgYGWwSRNYjSdmgCq=970231280.822781202448655747316065208791f; else txCyqhtebRGjTVgYGWwSRNYjSdmgCq=1915444949.673832716180957227960211887859f;if (txCyqhtebRGjTVgYGWwSRNYjSdmgCq - txCyqhtebRGjTVgYGWwSRNYjSdmgCq> 0.00000001 ) txCyqhtebRGjTVgYGWwSRNYjSdmgCq=7277988.451297592314223711818294147305f; else txCyqhtebRGjTVgYGWwSRNYjSdmgCq=126493799.606884987174575709927605126715f;if (txCyqhtebRGjTVgYGWwSRNYjSdmgCq - txCyqhtebRGjTVgYGWwSRNYjSdmgCq> 0.00000001 ) txCyqhtebRGjTVgYGWwSRNYjSdmgCq=503727596.139882855984584473054849810276f; else txCyqhtebRGjTVgYGWwSRNYjSdmgCq=783832263.827215679496514413186487139007f;long fpqCdkQkvozReeqQSYXAjFtDVXDFnd=1239264459;if (fpqCdkQkvozReeqQSYXAjFtDVXDFnd == fpqCdkQkvozReeqQSYXAjFtDVXDFnd- 0 ) fpqCdkQkvozReeqQSYXAjFtDVXDFnd=442086829; else fpqCdkQkvozReeqQSYXAjFtDVXDFnd=1887853091;if (fpqCdkQkvozReeqQSYXAjFtDVXDFnd == fpqCdkQkvozReeqQSYXAjFtDVXDFnd- 1 ) fpqCdkQkvozReeqQSYXAjFtDVXDFnd=554855306; else fpqCdkQkvozReeqQSYXAjFtDVXDFnd=1902410501;if (fpqCdkQkvozReeqQSYXAjFtDVXDFnd == fpqCdkQkvozReeqQSYXAjFtDVXDFnd- 0 ) fpqCdkQkvozReeqQSYXAjFtDVXDFnd=957000466; else fpqCdkQkvozReeqQSYXAjFtDVXDFnd=819708923;if (fpqCdkQkvozReeqQSYXAjFtDVXDFnd == fpqCdkQkvozReeqQSYXAjFtDVXDFnd- 1 ) fpqCdkQkvozReeqQSYXAjFtDVXDFnd=593648344; else fpqCdkQkvozReeqQSYXAjFtDVXDFnd=1370768985;if (fpqCdkQkvozReeqQSYXAjFtDVXDFnd == fpqCdkQkvozReeqQSYXAjFtDVXDFnd- 1 ) fpqCdkQkvozReeqQSYXAjFtDVXDFnd=1207740592; else fpqCdkQkvozReeqQSYXAjFtDVXDFnd=1173006402;if (fpqCdkQkvozReeqQSYXAjFtDVXDFnd == fpqCdkQkvozReeqQSYXAjFtDVXDFnd- 0 ) fpqCdkQkvozReeqQSYXAjFtDVXDFnd=1299301843; else fpqCdkQkvozReeqQSYXAjFtDVXDFnd=460211724;float vrReGatGWUmSoWUErQtqrMgYhBqbdu=1037302758.926669051880102616290861969799f;if (vrReGatGWUmSoWUErQtqrMgYhBqbdu - vrReGatGWUmSoWUErQtqrMgYhBqbdu> 0.00000001 ) vrReGatGWUmSoWUErQtqrMgYhBqbdu=1166375629.550444874791901054689366927570f; else vrReGatGWUmSoWUErQtqrMgYhBqbdu=1600689044.136234186995251096192521139463f;if (vrReGatGWUmSoWUErQtqrMgYhBqbdu - vrReGatGWUmSoWUErQtqrMgYhBqbdu> 0.00000001 ) vrReGatGWUmSoWUErQtqrMgYhBqbdu=54381757.740576578897970512603159862621f; else vrReGatGWUmSoWUErQtqrMgYhBqbdu=945632119.683325589553931575262786179271f;if (vrReGatGWUmSoWUErQtqrMgYhBqbdu - vrReGatGWUmSoWUErQtqrMgYhBqbdu> 0.00000001 ) vrReGatGWUmSoWUErQtqrMgYhBqbdu=1738415232.506732566604316122830756990735f; else vrReGatGWUmSoWUErQtqrMgYhBqbdu=2063981546.642915731901077957115744186085f;if (vrReGatGWUmSoWUErQtqrMgYhBqbdu - vrReGatGWUmSoWUErQtqrMgYhBqbdu> 0.00000001 ) vrReGatGWUmSoWUErQtqrMgYhBqbdu=2005085264.294107351140294089014363190883f; else vrReGatGWUmSoWUErQtqrMgYhBqbdu=791085610.975782797171707665121817194111f;if (vrReGatGWUmSoWUErQtqrMgYhBqbdu - vrReGatGWUmSoWUErQtqrMgYhBqbdu> 0.00000001 ) vrReGatGWUmSoWUErQtqrMgYhBqbdu=72895955.758837662239295476664586217869f; else vrReGatGWUmSoWUErQtqrMgYhBqbdu=867963299.969742498101490273373866728262f;if (vrReGatGWUmSoWUErQtqrMgYhBqbdu - vrReGatGWUmSoWUErQtqrMgYhBqbdu> 0.00000001 ) vrReGatGWUmSoWUErQtqrMgYhBqbdu=1757839284.180447156418966329715560621302f; else vrReGatGWUmSoWUErQtqrMgYhBqbdu=762454076.523603216422707690151966872083f;float paSstWNgNGHQPtMlgOENHMbGegltQC=1610778444.743130513948817893941107061207f;if (paSstWNgNGHQPtMlgOENHMbGegltQC - paSstWNgNGHQPtMlgOENHMbGegltQC> 0.00000001 ) paSstWNgNGHQPtMlgOENHMbGegltQC=1975116443.195159014770051368425562575628f; else paSstWNgNGHQPtMlgOENHMbGegltQC=2008783044.031686477898928215680498374478f;if (paSstWNgNGHQPtMlgOENHMbGegltQC - paSstWNgNGHQPtMlgOENHMbGegltQC> 0.00000001 ) paSstWNgNGHQPtMlgOENHMbGegltQC=1167653184.540784948699164764536014875341f; else paSstWNgNGHQPtMlgOENHMbGegltQC=1000437492.211946925437809133562194353602f;if (paSstWNgNGHQPtMlgOENHMbGegltQC - paSstWNgNGHQPtMlgOENHMbGegltQC> 0.00000001 ) paSstWNgNGHQPtMlgOENHMbGegltQC=1934335242.970759933984011931674398012869f; else paSstWNgNGHQPtMlgOENHMbGegltQC=1032582336.343467715131179829453213869374f;if (paSstWNgNGHQPtMlgOENHMbGegltQC - paSstWNgNGHQPtMlgOENHMbGegltQC> 0.00000001 ) paSstWNgNGHQPtMlgOENHMbGegltQC=1318362929.717575165522654433020442614160f; else paSstWNgNGHQPtMlgOENHMbGegltQC=983063586.854252560576403176944736547596f;if (paSstWNgNGHQPtMlgOENHMbGegltQC - paSstWNgNGHQPtMlgOENHMbGegltQC> 0.00000001 ) paSstWNgNGHQPtMlgOENHMbGegltQC=983397238.110577951307366405055823723971f; else paSstWNgNGHQPtMlgOENHMbGegltQC=57176722.508547604732575747015364230885f;if (paSstWNgNGHQPtMlgOENHMbGegltQC - paSstWNgNGHQPtMlgOENHMbGegltQC> 0.00000001 ) paSstWNgNGHQPtMlgOENHMbGegltQC=2086112798.409080993090243345908696735531f; else paSstWNgNGHQPtMlgOENHMbGegltQC=834125643.792934450586513513290509781345f;float YdMLzsMDuzsdTKKJoPNJKPSCMsRMWL=1723218989.721293591384026423055476361992f;if (YdMLzsMDuzsdTKKJoPNJKPSCMsRMWL - YdMLzsMDuzsdTKKJoPNJKPSCMsRMWL> 0.00000001 ) YdMLzsMDuzsdTKKJoPNJKPSCMsRMWL=1884810402.440102879726369107215645236503f; else YdMLzsMDuzsdTKKJoPNJKPSCMsRMWL=1613908485.370994502576443021726488890598f;if (YdMLzsMDuzsdTKKJoPNJKPSCMsRMWL - YdMLzsMDuzsdTKKJoPNJKPSCMsRMWL> 0.00000001 ) YdMLzsMDuzsdTKKJoPNJKPSCMsRMWL=1764940281.738756657571585921231598799211f; else YdMLzsMDuzsdTKKJoPNJKPSCMsRMWL=818305947.402585989848436600407036725006f;if (YdMLzsMDuzsdTKKJoPNJKPSCMsRMWL - YdMLzsMDuzsdTKKJoPNJKPSCMsRMWL> 0.00000001 ) YdMLzsMDuzsdTKKJoPNJKPSCMsRMWL=2082136400.744718010005250426320092557687f; else YdMLzsMDuzsdTKKJoPNJKPSCMsRMWL=717012427.098410572870040399741989686237f;if (YdMLzsMDuzsdTKKJoPNJKPSCMsRMWL - YdMLzsMDuzsdTKKJoPNJKPSCMsRMWL> 0.00000001 ) YdMLzsMDuzsdTKKJoPNJKPSCMsRMWL=1648332874.243412785095556105336905622094f; else YdMLzsMDuzsdTKKJoPNJKPSCMsRMWL=525588930.535573774962575753358101245899f;if (YdMLzsMDuzsdTKKJoPNJKPSCMsRMWL - YdMLzsMDuzsdTKKJoPNJKPSCMsRMWL> 0.00000001 ) YdMLzsMDuzsdTKKJoPNJKPSCMsRMWL=2030251474.070167009344157543008984744544f; else YdMLzsMDuzsdTKKJoPNJKPSCMsRMWL=1911253343.909699242975512415480399110753f;if (YdMLzsMDuzsdTKKJoPNJKPSCMsRMWL - YdMLzsMDuzsdTKKJoPNJKPSCMsRMWL> 0.00000001 ) YdMLzsMDuzsdTKKJoPNJKPSCMsRMWL=1829514546.466555666831123348234917561058f; else YdMLzsMDuzsdTKKJoPNJKPSCMsRMWL=89037167.060062916795975928797352056019f;double gGqqmGFfGVTIcWsxUKdWQPgElEmCBe=355749420.823960773071437643627039723845;if (gGqqmGFfGVTIcWsxUKdWQPgElEmCBe == gGqqmGFfGVTIcWsxUKdWQPgElEmCBe ) gGqqmGFfGVTIcWsxUKdWQPgElEmCBe=779948814.686952015046963528314663180524; else gGqqmGFfGVTIcWsxUKdWQPgElEmCBe=1836007046.893959559673868227621563102397;if (gGqqmGFfGVTIcWsxUKdWQPgElEmCBe == gGqqmGFfGVTIcWsxUKdWQPgElEmCBe ) gGqqmGFfGVTIcWsxUKdWQPgElEmCBe=632578928.359236302507085291697301434095; else gGqqmGFfGVTIcWsxUKdWQPgElEmCBe=1382816507.737242573202612559902280106980;if (gGqqmGFfGVTIcWsxUKdWQPgElEmCBe == gGqqmGFfGVTIcWsxUKdWQPgElEmCBe ) gGqqmGFfGVTIcWsxUKdWQPgElEmCBe=1411373109.332788094382840689099620455814; else gGqqmGFfGVTIcWsxUKdWQPgElEmCBe=1536213199.779307194466562045063790272243;if (gGqqmGFfGVTIcWsxUKdWQPgElEmCBe == gGqqmGFfGVTIcWsxUKdWQPgElEmCBe ) gGqqmGFfGVTIcWsxUKdWQPgElEmCBe=1250451163.428072464459663326842186208023; else gGqqmGFfGVTIcWsxUKdWQPgElEmCBe=133793293.606114950179171424796641245387;if (gGqqmGFfGVTIcWsxUKdWQPgElEmCBe == gGqqmGFfGVTIcWsxUKdWQPgElEmCBe ) gGqqmGFfGVTIcWsxUKdWQPgElEmCBe=282326115.454186466057677302599433706697; else gGqqmGFfGVTIcWsxUKdWQPgElEmCBe=1218801818.077483609607985018404480912249;if (gGqqmGFfGVTIcWsxUKdWQPgElEmCBe == gGqqmGFfGVTIcWsxUKdWQPgElEmCBe ) gGqqmGFfGVTIcWsxUKdWQPgElEmCBe=2109718528.197918130481309256880820469867; else gGqqmGFfGVTIcWsxUKdWQPgElEmCBe=1681238510.256152017808802419176747037865;float PjPPRXMWkJYuDrHBMlxaEgtFZEwFMb=1710360046.130675259663835870753119421536f;if (PjPPRXMWkJYuDrHBMlxaEgtFZEwFMb - PjPPRXMWkJYuDrHBMlxaEgtFZEwFMb> 0.00000001 ) PjPPRXMWkJYuDrHBMlxaEgtFZEwFMb=196634254.185675000874456353661339402135f; else PjPPRXMWkJYuDrHBMlxaEgtFZEwFMb=34620295.573469926098108087418808585628f;if (PjPPRXMWkJYuDrHBMlxaEgtFZEwFMb - PjPPRXMWkJYuDrHBMlxaEgtFZEwFMb> 0.00000001 ) PjPPRXMWkJYuDrHBMlxaEgtFZEwFMb=1025127321.397531210853768423153683877598f; else PjPPRXMWkJYuDrHBMlxaEgtFZEwFMb=1611167495.440970178887877525116257582576f;if (PjPPRXMWkJYuDrHBMlxaEgtFZEwFMb - PjPPRXMWkJYuDrHBMlxaEgtFZEwFMb> 0.00000001 ) PjPPRXMWkJYuDrHBMlxaEgtFZEwFMb=1234534538.572241399846648791736359229450f; else PjPPRXMWkJYuDrHBMlxaEgtFZEwFMb=1722235288.960242563634637171080195022413f;if (PjPPRXMWkJYuDrHBMlxaEgtFZEwFMb - PjPPRXMWkJYuDrHBMlxaEgtFZEwFMb> 0.00000001 ) PjPPRXMWkJYuDrHBMlxaEgtFZEwFMb=1676125691.304908641633921373226847188632f; else PjPPRXMWkJYuDrHBMlxaEgtFZEwFMb=1258104016.116133074053156738575039856892f;if (PjPPRXMWkJYuDrHBMlxaEgtFZEwFMb - PjPPRXMWkJYuDrHBMlxaEgtFZEwFMb> 0.00000001 ) PjPPRXMWkJYuDrHBMlxaEgtFZEwFMb=360450170.112062789841037380146026729748f; else PjPPRXMWkJYuDrHBMlxaEgtFZEwFMb=642064254.729295210515073511358785610635f;if (PjPPRXMWkJYuDrHBMlxaEgtFZEwFMb - PjPPRXMWkJYuDrHBMlxaEgtFZEwFMb> 0.00000001 ) PjPPRXMWkJYuDrHBMlxaEgtFZEwFMb=1117669665.405112384317164215331859015468f; else PjPPRXMWkJYuDrHBMlxaEgtFZEwFMb=23081899.578621096223715272150631466711f;int tKflUrZWSHNrIBqHWTqTSPEpzTkKga=1152202970;if (tKflUrZWSHNrIBqHWTqTSPEpzTkKga == tKflUrZWSHNrIBqHWTqTSPEpzTkKga- 1 ) tKflUrZWSHNrIBqHWTqTSPEpzTkKga=1911483717; else tKflUrZWSHNrIBqHWTqTSPEpzTkKga=1730363378;if (tKflUrZWSHNrIBqHWTqTSPEpzTkKga == tKflUrZWSHNrIBqHWTqTSPEpzTkKga- 0 ) tKflUrZWSHNrIBqHWTqTSPEpzTkKga=54176829; else tKflUrZWSHNrIBqHWTqTSPEpzTkKga=1947018252;if (tKflUrZWSHNrIBqHWTqTSPEpzTkKga == tKflUrZWSHNrIBqHWTqTSPEpzTkKga- 1 ) tKflUrZWSHNrIBqHWTqTSPEpzTkKga=2024924275; else tKflUrZWSHNrIBqHWTqTSPEpzTkKga=1435996185;if (tKflUrZWSHNrIBqHWTqTSPEpzTkKga == tKflUrZWSHNrIBqHWTqTSPEpzTkKga- 0 ) tKflUrZWSHNrIBqHWTqTSPEpzTkKga=311845880; else tKflUrZWSHNrIBqHWTqTSPEpzTkKga=348597203;if (tKflUrZWSHNrIBqHWTqTSPEpzTkKga == tKflUrZWSHNrIBqHWTqTSPEpzTkKga- 0 ) tKflUrZWSHNrIBqHWTqTSPEpzTkKga=363457322; else tKflUrZWSHNrIBqHWTqTSPEpzTkKga=1631146988;if (tKflUrZWSHNrIBqHWTqTSPEpzTkKga == tKflUrZWSHNrIBqHWTqTSPEpzTkKga- 0 ) tKflUrZWSHNrIBqHWTqTSPEpzTkKga=535979832; else tKflUrZWSHNrIBqHWTqTSPEpzTkKga=863334582;double YhiXbpoWbAEOBiHAlrGaFrPdeKXByr=859291853.150808243007661903163874890997;if (YhiXbpoWbAEOBiHAlrGaFrPdeKXByr == YhiXbpoWbAEOBiHAlrGaFrPdeKXByr ) YhiXbpoWbAEOBiHAlrGaFrPdeKXByr=608225737.991339053812144976774382938712; else YhiXbpoWbAEOBiHAlrGaFrPdeKXByr=119154500.935611734395036183092423471040;if (YhiXbpoWbAEOBiHAlrGaFrPdeKXByr == YhiXbpoWbAEOBiHAlrGaFrPdeKXByr ) YhiXbpoWbAEOBiHAlrGaFrPdeKXByr=2039886560.145198754334359046786673051335; else YhiXbpoWbAEOBiHAlrGaFrPdeKXByr=1174732991.070425194843086828544636468324;if (YhiXbpoWbAEOBiHAlrGaFrPdeKXByr == YhiXbpoWbAEOBiHAlrGaFrPdeKXByr ) YhiXbpoWbAEOBiHAlrGaFrPdeKXByr=953550594.753036593495205470734490255507; else YhiXbpoWbAEOBiHAlrGaFrPdeKXByr=829492804.898010105143615954210860147499;if (YhiXbpoWbAEOBiHAlrGaFrPdeKXByr == YhiXbpoWbAEOBiHAlrGaFrPdeKXByr ) YhiXbpoWbAEOBiHAlrGaFrPdeKXByr=1268698529.210761729647579188530230703907; else YhiXbpoWbAEOBiHAlrGaFrPdeKXByr=1205197173.382693496533577340117096918107;if (YhiXbpoWbAEOBiHAlrGaFrPdeKXByr == YhiXbpoWbAEOBiHAlrGaFrPdeKXByr ) YhiXbpoWbAEOBiHAlrGaFrPdeKXByr=159881355.983337608679311877357390139679; else YhiXbpoWbAEOBiHAlrGaFrPdeKXByr=1521958559.640065124184354889624186070320;if (YhiXbpoWbAEOBiHAlrGaFrPdeKXByr == YhiXbpoWbAEOBiHAlrGaFrPdeKXByr ) YhiXbpoWbAEOBiHAlrGaFrPdeKXByr=1022366337.913014045675328314636005556500; else YhiXbpoWbAEOBiHAlrGaFrPdeKXByr=769308855.581538245195416987877680491956;int bScDtYuwAJjydZIdxwdkvgPQwCEBSi=1200484000;if (bScDtYuwAJjydZIdxwdkvgPQwCEBSi == bScDtYuwAJjydZIdxwdkvgPQwCEBSi- 0 ) bScDtYuwAJjydZIdxwdkvgPQwCEBSi=1445091518; else bScDtYuwAJjydZIdxwdkvgPQwCEBSi=957515558;if (bScDtYuwAJjydZIdxwdkvgPQwCEBSi == bScDtYuwAJjydZIdxwdkvgPQwCEBSi- 1 ) bScDtYuwAJjydZIdxwdkvgPQwCEBSi=1917238440; else bScDtYuwAJjydZIdxwdkvgPQwCEBSi=394989017;if (bScDtYuwAJjydZIdxwdkvgPQwCEBSi == bScDtYuwAJjydZIdxwdkvgPQwCEBSi- 1 ) bScDtYuwAJjydZIdxwdkvgPQwCEBSi=1116782792; else bScDtYuwAJjydZIdxwdkvgPQwCEBSi=1503493001;if (bScDtYuwAJjydZIdxwdkvgPQwCEBSi == bScDtYuwAJjydZIdxwdkvgPQwCEBSi- 1 ) bScDtYuwAJjydZIdxwdkvgPQwCEBSi=1832919418; else bScDtYuwAJjydZIdxwdkvgPQwCEBSi=908121716;if (bScDtYuwAJjydZIdxwdkvgPQwCEBSi == bScDtYuwAJjydZIdxwdkvgPQwCEBSi- 1 ) bScDtYuwAJjydZIdxwdkvgPQwCEBSi=1870258062; else bScDtYuwAJjydZIdxwdkvgPQwCEBSi=676530118;if (bScDtYuwAJjydZIdxwdkvgPQwCEBSi == bScDtYuwAJjydZIdxwdkvgPQwCEBSi- 0 ) bScDtYuwAJjydZIdxwdkvgPQwCEBSi=1247710173; else bScDtYuwAJjydZIdxwdkvgPQwCEBSi=1750871197;float jOCBFfgUbbKiphJimuVEvCXWkFixiE=393355806.743849423253585647211837438083f;if (jOCBFfgUbbKiphJimuVEvCXWkFixiE - jOCBFfgUbbKiphJimuVEvCXWkFixiE> 0.00000001 ) jOCBFfgUbbKiphJimuVEvCXWkFixiE=94344072.953511682876618716203445702652f; else jOCBFfgUbbKiphJimuVEvCXWkFixiE=258377812.255131008516655622082299061992f;if (jOCBFfgUbbKiphJimuVEvCXWkFixiE - jOCBFfgUbbKiphJimuVEvCXWkFixiE> 0.00000001 ) jOCBFfgUbbKiphJimuVEvCXWkFixiE=1562339809.904173269772979790129167106617f; else jOCBFfgUbbKiphJimuVEvCXWkFixiE=1231396937.021771369246431033048298394807f;if (jOCBFfgUbbKiphJimuVEvCXWkFixiE - jOCBFfgUbbKiphJimuVEvCXWkFixiE> 0.00000001 ) jOCBFfgUbbKiphJimuVEvCXWkFixiE=1336922849.866286131953298373717478094448f; else jOCBFfgUbbKiphJimuVEvCXWkFixiE=746460956.766259126691319500112119049929f;if (jOCBFfgUbbKiphJimuVEvCXWkFixiE - jOCBFfgUbbKiphJimuVEvCXWkFixiE> 0.00000001 ) jOCBFfgUbbKiphJimuVEvCXWkFixiE=198652376.280320805035813726959544665721f; else jOCBFfgUbbKiphJimuVEvCXWkFixiE=1081990913.021842109080628741743407085776f;if (jOCBFfgUbbKiphJimuVEvCXWkFixiE - jOCBFfgUbbKiphJimuVEvCXWkFixiE> 0.00000001 ) jOCBFfgUbbKiphJimuVEvCXWkFixiE=461664849.619401517172708093603409972487f; else jOCBFfgUbbKiphJimuVEvCXWkFixiE=996463303.853447222724477171019318421064f;if (jOCBFfgUbbKiphJimuVEvCXWkFixiE - jOCBFfgUbbKiphJimuVEvCXWkFixiE> 0.00000001 ) jOCBFfgUbbKiphJimuVEvCXWkFixiE=1787136897.023142046188108004281257954503f; else jOCBFfgUbbKiphJimuVEvCXWkFixiE=502965716.627881143623693222152888498739f; }
 jOCBFfgUbbKiphJimuVEvCXWkFixiEy::jOCBFfgUbbKiphJimuVEvCXWkFixiEy()
 { this->IItaUfWhBaZj("XCviAIYiMrRZcPbRCsxqhPElhqfQAwIItaUfWhBaZjj", true, 413629848, 1690020672, 2077537316); }
#pragma optimize("", off)
 // <delete/>


// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class vjOFaCKyHhRroCLwyjAnVcpoFHuUOzy
 { 
public: bool JjNgENBSomzSuzCFHRrOfPdYStaATH; double JjNgENBSomzSuzCFHRrOfPdYStaATHvjOFaCKyHhRroCLwyjAnVcpoFHuUOz; vjOFaCKyHhRroCLwyjAnVcpoFHuUOzy(); void slTwFLlUQcYb(string JjNgENBSomzSuzCFHRrOfPdYStaATHslTwFLlUQcYb, bool qkHIHIaVGjxRMAWKuoCBnCXlttFpNL, int mBygwOKmruelLHQODcBIzmxRUnMWPO, float ClIvwwSfEHzIxIWBrPUfpdoEMAEgMI, long CnXcrtdQLUvXXtJqoovSwxCkYZwDiw);
 protected: bool JjNgENBSomzSuzCFHRrOfPdYStaATHo; double JjNgENBSomzSuzCFHRrOfPdYStaATHvjOFaCKyHhRroCLwyjAnVcpoFHuUOzf; void slTwFLlUQcYbu(string JjNgENBSomzSuzCFHRrOfPdYStaATHslTwFLlUQcYbg, bool qkHIHIaVGjxRMAWKuoCBnCXlttFpNLe, int mBygwOKmruelLHQODcBIzmxRUnMWPOr, float ClIvwwSfEHzIxIWBrPUfpdoEMAEgMIw, long CnXcrtdQLUvXXtJqoovSwxCkYZwDiwn);
 private: bool JjNgENBSomzSuzCFHRrOfPdYStaATHqkHIHIaVGjxRMAWKuoCBnCXlttFpNL; double JjNgENBSomzSuzCFHRrOfPdYStaATHClIvwwSfEHzIxIWBrPUfpdoEMAEgMIvjOFaCKyHhRroCLwyjAnVcpoFHuUOz;
 void slTwFLlUQcYbv(string qkHIHIaVGjxRMAWKuoCBnCXlttFpNLslTwFLlUQcYb, bool qkHIHIaVGjxRMAWKuoCBnCXlttFpNLmBygwOKmruelLHQODcBIzmxRUnMWPO, int mBygwOKmruelLHQODcBIzmxRUnMWPOJjNgENBSomzSuzCFHRrOfPdYStaATH, float ClIvwwSfEHzIxIWBrPUfpdoEMAEgMICnXcrtdQLUvXXtJqoovSwxCkYZwDiw, long CnXcrtdQLUvXXtJqoovSwxCkYZwDiwqkHIHIaVGjxRMAWKuoCBnCXlttFpNL); };
 void vjOFaCKyHhRroCLwyjAnVcpoFHuUOzy::slTwFLlUQcYb(string JjNgENBSomzSuzCFHRrOfPdYStaATHslTwFLlUQcYb, bool qkHIHIaVGjxRMAWKuoCBnCXlttFpNL, int mBygwOKmruelLHQODcBIzmxRUnMWPO, float ClIvwwSfEHzIxIWBrPUfpdoEMAEgMI, long CnXcrtdQLUvXXtJqoovSwxCkYZwDiw)
 { double DyTxXeHlijgYYgJPBWGFTeAzcfSNkG=386888233.657371189674147427899901633239;if (DyTxXeHlijgYYgJPBWGFTeAzcfSNkG == DyTxXeHlijgYYgJPBWGFTeAzcfSNkG ) DyTxXeHlijgYYgJPBWGFTeAzcfSNkG=864862994.435708277830793464078240185784; else DyTxXeHlijgYYgJPBWGFTeAzcfSNkG=1527819490.996124955025006922828937230673;if (DyTxXeHlijgYYgJPBWGFTeAzcfSNkG == DyTxXeHlijgYYgJPBWGFTeAzcfSNkG ) DyTxXeHlijgYYgJPBWGFTeAzcfSNkG=1915952556.360305577061005324427496706000; else DyTxXeHlijgYYgJPBWGFTeAzcfSNkG=1364308130.449277670256597804291625708471;if (DyTxXeHlijgYYgJPBWGFTeAzcfSNkG == DyTxXeHlijgYYgJPBWGFTeAzcfSNkG ) DyTxXeHlijgYYgJPBWGFTeAzcfSNkG=1159990580.957863283976438290001381082567; else DyTxXeHlijgYYgJPBWGFTeAzcfSNkG=1756746110.031040984462143630862848168788;if (DyTxXeHlijgYYgJPBWGFTeAzcfSNkG == DyTxXeHlijgYYgJPBWGFTeAzcfSNkG ) DyTxXeHlijgYYgJPBWGFTeAzcfSNkG=2018643826.183794559832774935956371620308; else DyTxXeHlijgYYgJPBWGFTeAzcfSNkG=1607915434.330638045919103128424384291218;if (DyTxXeHlijgYYgJPBWGFTeAzcfSNkG == DyTxXeHlijgYYgJPBWGFTeAzcfSNkG ) DyTxXeHlijgYYgJPBWGFTeAzcfSNkG=1328572073.525261214709004828961309405754; else DyTxXeHlijgYYgJPBWGFTeAzcfSNkG=1327595969.784849494201483549048446833797;if (DyTxXeHlijgYYgJPBWGFTeAzcfSNkG == DyTxXeHlijgYYgJPBWGFTeAzcfSNkG ) DyTxXeHlijgYYgJPBWGFTeAzcfSNkG=201312074.757002637653476093797771745801; else DyTxXeHlijgYYgJPBWGFTeAzcfSNkG=205023381.781855558102250507085010119747;double qNaulKfKGbWYeyaVKDIRmnJihiHnVh=1674091655.855263306076956159247969262144;if (qNaulKfKGbWYeyaVKDIRmnJihiHnVh == qNaulKfKGbWYeyaVKDIRmnJihiHnVh ) qNaulKfKGbWYeyaVKDIRmnJihiHnVh=1198260753.150750664684827966134416900600; else qNaulKfKGbWYeyaVKDIRmnJihiHnVh=1701947401.126202731769677414341701185485;if (qNaulKfKGbWYeyaVKDIRmnJihiHnVh == qNaulKfKGbWYeyaVKDIRmnJihiHnVh ) qNaulKfKGbWYeyaVKDIRmnJihiHnVh=925523975.107456862523558595059646052523; else qNaulKfKGbWYeyaVKDIRmnJihiHnVh=874193127.835776323166283826460053657151;if (qNaulKfKGbWYeyaVKDIRmnJihiHnVh == qNaulKfKGbWYeyaVKDIRmnJihiHnVh ) qNaulKfKGbWYeyaVKDIRmnJihiHnVh=890217275.445079998327390446433726488312; else qNaulKfKGbWYeyaVKDIRmnJihiHnVh=1962834391.928283231403915387431182882970;if (qNaulKfKGbWYeyaVKDIRmnJihiHnVh == qNaulKfKGbWYeyaVKDIRmnJihiHnVh ) qNaulKfKGbWYeyaVKDIRmnJihiHnVh=284594461.279641011732513082211299195641; else qNaulKfKGbWYeyaVKDIRmnJihiHnVh=1659557172.236234538447680078090186908066;if (qNaulKfKGbWYeyaVKDIRmnJihiHnVh == qNaulKfKGbWYeyaVKDIRmnJihiHnVh ) qNaulKfKGbWYeyaVKDIRmnJihiHnVh=88127256.747160638056488472922776125043; else qNaulKfKGbWYeyaVKDIRmnJihiHnVh=859107445.657280153144004471954724408214;if (qNaulKfKGbWYeyaVKDIRmnJihiHnVh == qNaulKfKGbWYeyaVKDIRmnJihiHnVh ) qNaulKfKGbWYeyaVKDIRmnJihiHnVh=528199219.670640920942098988285529004499; else qNaulKfKGbWYeyaVKDIRmnJihiHnVh=202760141.395958456455112284662104915175;int cREeYoESBXRIpMAVFmmJznQlLxEwWO=1706158869;if (cREeYoESBXRIpMAVFmmJznQlLxEwWO == cREeYoESBXRIpMAVFmmJznQlLxEwWO- 0 ) cREeYoESBXRIpMAVFmmJznQlLxEwWO=837531759; else cREeYoESBXRIpMAVFmmJznQlLxEwWO=578404476;if (cREeYoESBXRIpMAVFmmJznQlLxEwWO == cREeYoESBXRIpMAVFmmJznQlLxEwWO- 0 ) cREeYoESBXRIpMAVFmmJznQlLxEwWO=869767525; else cREeYoESBXRIpMAVFmmJznQlLxEwWO=1104726848;if (cREeYoESBXRIpMAVFmmJznQlLxEwWO == cREeYoESBXRIpMAVFmmJznQlLxEwWO- 0 ) cREeYoESBXRIpMAVFmmJznQlLxEwWO=146948943; else cREeYoESBXRIpMAVFmmJznQlLxEwWO=1409417975;if (cREeYoESBXRIpMAVFmmJznQlLxEwWO == cREeYoESBXRIpMAVFmmJznQlLxEwWO- 1 ) cREeYoESBXRIpMAVFmmJznQlLxEwWO=1862377628; else cREeYoESBXRIpMAVFmmJznQlLxEwWO=863447805;if (cREeYoESBXRIpMAVFmmJznQlLxEwWO == cREeYoESBXRIpMAVFmmJznQlLxEwWO- 0 ) cREeYoESBXRIpMAVFmmJznQlLxEwWO=172336379; else cREeYoESBXRIpMAVFmmJznQlLxEwWO=1281935760;if (cREeYoESBXRIpMAVFmmJznQlLxEwWO == cREeYoESBXRIpMAVFmmJznQlLxEwWO- 1 ) cREeYoESBXRIpMAVFmmJznQlLxEwWO=1070739138; else cREeYoESBXRIpMAVFmmJznQlLxEwWO=1181531022;long qIoZfWSOFhILwjblrPFkdsGJJlFOfN=1484668803;if (qIoZfWSOFhILwjblrPFkdsGJJlFOfN == qIoZfWSOFhILwjblrPFkdsGJJlFOfN- 1 ) qIoZfWSOFhILwjblrPFkdsGJJlFOfN=1519887478; else qIoZfWSOFhILwjblrPFkdsGJJlFOfN=1993462985;if (qIoZfWSOFhILwjblrPFkdsGJJlFOfN == qIoZfWSOFhILwjblrPFkdsGJJlFOfN- 1 ) qIoZfWSOFhILwjblrPFkdsGJJlFOfN=1871011267; else qIoZfWSOFhILwjblrPFkdsGJJlFOfN=963221530;if (qIoZfWSOFhILwjblrPFkdsGJJlFOfN == qIoZfWSOFhILwjblrPFkdsGJJlFOfN- 1 ) qIoZfWSOFhILwjblrPFkdsGJJlFOfN=216322820; else qIoZfWSOFhILwjblrPFkdsGJJlFOfN=955729722;if (qIoZfWSOFhILwjblrPFkdsGJJlFOfN == qIoZfWSOFhILwjblrPFkdsGJJlFOfN- 0 ) qIoZfWSOFhILwjblrPFkdsGJJlFOfN=1644154605; else qIoZfWSOFhILwjblrPFkdsGJJlFOfN=1049512250;if (qIoZfWSOFhILwjblrPFkdsGJJlFOfN == qIoZfWSOFhILwjblrPFkdsGJJlFOfN- 1 ) qIoZfWSOFhILwjblrPFkdsGJJlFOfN=1243225029; else qIoZfWSOFhILwjblrPFkdsGJJlFOfN=256395384;if (qIoZfWSOFhILwjblrPFkdsGJJlFOfN == qIoZfWSOFhILwjblrPFkdsGJJlFOfN- 1 ) qIoZfWSOFhILwjblrPFkdsGJJlFOfN=1276180874; else qIoZfWSOFhILwjblrPFkdsGJJlFOfN=300883135;double LbWKgMxoHRLQmioOdgHXxqDENFiVHY=339743319.197062457463766395648618610487;if (LbWKgMxoHRLQmioOdgHXxqDENFiVHY == LbWKgMxoHRLQmioOdgHXxqDENFiVHY ) LbWKgMxoHRLQmioOdgHXxqDENFiVHY=908492835.640819296016655468208666659945; else LbWKgMxoHRLQmioOdgHXxqDENFiVHY=642636852.318193495711073216436229909427;if (LbWKgMxoHRLQmioOdgHXxqDENFiVHY == LbWKgMxoHRLQmioOdgHXxqDENFiVHY ) LbWKgMxoHRLQmioOdgHXxqDENFiVHY=1514975269.045170956317359879454462019020; else LbWKgMxoHRLQmioOdgHXxqDENFiVHY=1997416739.517978690347023908124008479327;if (LbWKgMxoHRLQmioOdgHXxqDENFiVHY == LbWKgMxoHRLQmioOdgHXxqDENFiVHY ) LbWKgMxoHRLQmioOdgHXxqDENFiVHY=1078748689.203790058128011844022291651060; else LbWKgMxoHRLQmioOdgHXxqDENFiVHY=1334205322.038425434395001600864541611314;if (LbWKgMxoHRLQmioOdgHXxqDENFiVHY == LbWKgMxoHRLQmioOdgHXxqDENFiVHY ) LbWKgMxoHRLQmioOdgHXxqDENFiVHY=1527142712.010919179741173720016130760882; else LbWKgMxoHRLQmioOdgHXxqDENFiVHY=1936326425.522139520364213348899131909126;if (LbWKgMxoHRLQmioOdgHXxqDENFiVHY == LbWKgMxoHRLQmioOdgHXxqDENFiVHY ) LbWKgMxoHRLQmioOdgHXxqDENFiVHY=1987053420.049823015431086202752309349224; else LbWKgMxoHRLQmioOdgHXxqDENFiVHY=62076170.040443640143447826670858484059;if (LbWKgMxoHRLQmioOdgHXxqDENFiVHY == LbWKgMxoHRLQmioOdgHXxqDENFiVHY ) LbWKgMxoHRLQmioOdgHXxqDENFiVHY=701856748.499159175376261006353466430317; else LbWKgMxoHRLQmioOdgHXxqDENFiVHY=1755765162.969186530876131569759981703482;float sbrWEkHseZjcZomONXwlEfhpzntXmM=1736874983.522988213909012822468917761604f;if (sbrWEkHseZjcZomONXwlEfhpzntXmM - sbrWEkHseZjcZomONXwlEfhpzntXmM> 0.00000001 ) sbrWEkHseZjcZomONXwlEfhpzntXmM=1396351722.095634464837861814132779471623f; else sbrWEkHseZjcZomONXwlEfhpzntXmM=1191014063.762901549946963126605767332737f;if (sbrWEkHseZjcZomONXwlEfhpzntXmM - sbrWEkHseZjcZomONXwlEfhpzntXmM> 0.00000001 ) sbrWEkHseZjcZomONXwlEfhpzntXmM=830356310.394228697047620682645613810677f; else sbrWEkHseZjcZomONXwlEfhpzntXmM=373129364.082375385653967924776120289252f;if (sbrWEkHseZjcZomONXwlEfhpzntXmM - sbrWEkHseZjcZomONXwlEfhpzntXmM> 0.00000001 ) sbrWEkHseZjcZomONXwlEfhpzntXmM=1265301134.813116546541709275799790297319f; else sbrWEkHseZjcZomONXwlEfhpzntXmM=1603365251.724484095148934542102456323492f;if (sbrWEkHseZjcZomONXwlEfhpzntXmM - sbrWEkHseZjcZomONXwlEfhpzntXmM> 0.00000001 ) sbrWEkHseZjcZomONXwlEfhpzntXmM=1489836049.033783843528175297786521837827f; else sbrWEkHseZjcZomONXwlEfhpzntXmM=2029285684.270312625270068449124682751206f;if (sbrWEkHseZjcZomONXwlEfhpzntXmM - sbrWEkHseZjcZomONXwlEfhpzntXmM> 0.00000001 ) sbrWEkHseZjcZomONXwlEfhpzntXmM=1081733518.713646986477361667350870470976f; else sbrWEkHseZjcZomONXwlEfhpzntXmM=1967738922.143957843357485743110505794337f;if (sbrWEkHseZjcZomONXwlEfhpzntXmM - sbrWEkHseZjcZomONXwlEfhpzntXmM> 0.00000001 ) sbrWEkHseZjcZomONXwlEfhpzntXmM=2008985466.611044541593510350673886774837f; else sbrWEkHseZjcZomONXwlEfhpzntXmM=2038037206.351685645230273810645487593547f;float kDVKRItOZhFdQUPdESsfZLvIumUGvJ=503936341.149454004074116695366154610866f;if (kDVKRItOZhFdQUPdESsfZLvIumUGvJ - kDVKRItOZhFdQUPdESsfZLvIumUGvJ> 0.00000001 ) kDVKRItOZhFdQUPdESsfZLvIumUGvJ=2040612675.667656339618975940684157510684f; else kDVKRItOZhFdQUPdESsfZLvIumUGvJ=730995819.434842767046686938078029252115f;if (kDVKRItOZhFdQUPdESsfZLvIumUGvJ - kDVKRItOZhFdQUPdESsfZLvIumUGvJ> 0.00000001 ) kDVKRItOZhFdQUPdESsfZLvIumUGvJ=273038187.686638414439464968179261642660f; else kDVKRItOZhFdQUPdESsfZLvIumUGvJ=1156504252.075357015952575076540505742609f;if (kDVKRItOZhFdQUPdESsfZLvIumUGvJ - kDVKRItOZhFdQUPdESsfZLvIumUGvJ> 0.00000001 ) kDVKRItOZhFdQUPdESsfZLvIumUGvJ=1131244298.795547422806333780678613067161f; else kDVKRItOZhFdQUPdESsfZLvIumUGvJ=1760306278.551626713976585427207203219259f;if (kDVKRItOZhFdQUPdESsfZLvIumUGvJ - kDVKRItOZhFdQUPdESsfZLvIumUGvJ> 0.00000001 ) kDVKRItOZhFdQUPdESsfZLvIumUGvJ=405920114.036235571708881223965097435952f; else kDVKRItOZhFdQUPdESsfZLvIumUGvJ=185682156.856404145005760053210649504981f;if (kDVKRItOZhFdQUPdESsfZLvIumUGvJ - kDVKRItOZhFdQUPdESsfZLvIumUGvJ> 0.00000001 ) kDVKRItOZhFdQUPdESsfZLvIumUGvJ=466320273.063095249719907044758375332800f; else kDVKRItOZhFdQUPdESsfZLvIumUGvJ=2082168489.127505378582450274357123389987f;if (kDVKRItOZhFdQUPdESsfZLvIumUGvJ - kDVKRItOZhFdQUPdESsfZLvIumUGvJ> 0.00000001 ) kDVKRItOZhFdQUPdESsfZLvIumUGvJ=1123651757.444079351388637792526965311423f; else kDVKRItOZhFdQUPdESsfZLvIumUGvJ=154246650.649124859103418050669847159976f;double qNokSrZCPrWSWKRXbmUgErfVpBMzjc=2140161590.096709588714866787809840901248;if (qNokSrZCPrWSWKRXbmUgErfVpBMzjc == qNokSrZCPrWSWKRXbmUgErfVpBMzjc ) qNokSrZCPrWSWKRXbmUgErfVpBMzjc=395979316.522046659298135944687292726691; else qNokSrZCPrWSWKRXbmUgErfVpBMzjc=1686789538.934030292113872181617867243132;if (qNokSrZCPrWSWKRXbmUgErfVpBMzjc == qNokSrZCPrWSWKRXbmUgErfVpBMzjc ) qNokSrZCPrWSWKRXbmUgErfVpBMzjc=1052015060.007947411675267443175618427899; else qNokSrZCPrWSWKRXbmUgErfVpBMzjc=37532828.727112604588115260180107750052;if (qNokSrZCPrWSWKRXbmUgErfVpBMzjc == qNokSrZCPrWSWKRXbmUgErfVpBMzjc ) qNokSrZCPrWSWKRXbmUgErfVpBMzjc=985527375.062206926685069851436881062047; else qNokSrZCPrWSWKRXbmUgErfVpBMzjc=1015677368.375262691935550625800963739234;if (qNokSrZCPrWSWKRXbmUgErfVpBMzjc == qNokSrZCPrWSWKRXbmUgErfVpBMzjc ) qNokSrZCPrWSWKRXbmUgErfVpBMzjc=1221925832.498032080827050233155834741364; else qNokSrZCPrWSWKRXbmUgErfVpBMzjc=1452611231.357855762964518402450719077430;if (qNokSrZCPrWSWKRXbmUgErfVpBMzjc == qNokSrZCPrWSWKRXbmUgErfVpBMzjc ) qNokSrZCPrWSWKRXbmUgErfVpBMzjc=1300284450.183758020668139133734002574999; else qNokSrZCPrWSWKRXbmUgErfVpBMzjc=1633173232.833389296048280369005580718128;if (qNokSrZCPrWSWKRXbmUgErfVpBMzjc == qNokSrZCPrWSWKRXbmUgErfVpBMzjc ) qNokSrZCPrWSWKRXbmUgErfVpBMzjc=915683221.524283595194770692321599445377; else qNokSrZCPrWSWKRXbmUgErfVpBMzjc=1258493414.117962501788502182113790704694;double LKnAllqGqGvDeXRDwnRdSMdWTKiGbp=425558927.739683695309859558267447216995;if (LKnAllqGqGvDeXRDwnRdSMdWTKiGbp == LKnAllqGqGvDeXRDwnRdSMdWTKiGbp ) LKnAllqGqGvDeXRDwnRdSMdWTKiGbp=427175256.423134442933714731598218049348; else LKnAllqGqGvDeXRDwnRdSMdWTKiGbp=197920407.780008773511734584368541822610;if (LKnAllqGqGvDeXRDwnRdSMdWTKiGbp == LKnAllqGqGvDeXRDwnRdSMdWTKiGbp ) LKnAllqGqGvDeXRDwnRdSMdWTKiGbp=1862636134.521526372606908868466618024938; else LKnAllqGqGvDeXRDwnRdSMdWTKiGbp=161099434.726400122822277375407425401360;if (LKnAllqGqGvDeXRDwnRdSMdWTKiGbp == LKnAllqGqGvDeXRDwnRdSMdWTKiGbp ) LKnAllqGqGvDeXRDwnRdSMdWTKiGbp=977508899.245508150884775554286063241101; else LKnAllqGqGvDeXRDwnRdSMdWTKiGbp=851395463.987278888923563386106829538237;if (LKnAllqGqGvDeXRDwnRdSMdWTKiGbp == LKnAllqGqGvDeXRDwnRdSMdWTKiGbp ) LKnAllqGqGvDeXRDwnRdSMdWTKiGbp=605630685.806900860787736351006600627974; else LKnAllqGqGvDeXRDwnRdSMdWTKiGbp=1783529091.165123766452653919329724120648;if (LKnAllqGqGvDeXRDwnRdSMdWTKiGbp == LKnAllqGqGvDeXRDwnRdSMdWTKiGbp ) LKnAllqGqGvDeXRDwnRdSMdWTKiGbp=785093949.400772262379675516781865645210; else LKnAllqGqGvDeXRDwnRdSMdWTKiGbp=644674154.313906398276806026962496874614;if (LKnAllqGqGvDeXRDwnRdSMdWTKiGbp == LKnAllqGqGvDeXRDwnRdSMdWTKiGbp ) LKnAllqGqGvDeXRDwnRdSMdWTKiGbp=2054292738.816206678032962606962035802823; else LKnAllqGqGvDeXRDwnRdSMdWTKiGbp=1330616292.288556161145804164527976299151;double tdWoZQUGcbVcpJhGODmTrMxgQCOMfl=1193344075.380663411247966859461796563401;if (tdWoZQUGcbVcpJhGODmTrMxgQCOMfl == tdWoZQUGcbVcpJhGODmTrMxgQCOMfl ) tdWoZQUGcbVcpJhGODmTrMxgQCOMfl=1810321925.244343730303113293227120479421; else tdWoZQUGcbVcpJhGODmTrMxgQCOMfl=2016307523.001000362422016869448138523507;if (tdWoZQUGcbVcpJhGODmTrMxgQCOMfl == tdWoZQUGcbVcpJhGODmTrMxgQCOMfl ) tdWoZQUGcbVcpJhGODmTrMxgQCOMfl=1642057890.432040222393191512462118308368; else tdWoZQUGcbVcpJhGODmTrMxgQCOMfl=593931210.897600018764726614801115642574;if (tdWoZQUGcbVcpJhGODmTrMxgQCOMfl == tdWoZQUGcbVcpJhGODmTrMxgQCOMfl ) tdWoZQUGcbVcpJhGODmTrMxgQCOMfl=509479433.207460669160570840370566778207; else tdWoZQUGcbVcpJhGODmTrMxgQCOMfl=1744851733.864467846605584274680320574179;if (tdWoZQUGcbVcpJhGODmTrMxgQCOMfl == tdWoZQUGcbVcpJhGODmTrMxgQCOMfl ) tdWoZQUGcbVcpJhGODmTrMxgQCOMfl=1665892613.356341612301488518360473854509; else tdWoZQUGcbVcpJhGODmTrMxgQCOMfl=1024935307.388746035466338005317691998469;if (tdWoZQUGcbVcpJhGODmTrMxgQCOMfl == tdWoZQUGcbVcpJhGODmTrMxgQCOMfl ) tdWoZQUGcbVcpJhGODmTrMxgQCOMfl=1656553634.722303475358491425586029380795; else tdWoZQUGcbVcpJhGODmTrMxgQCOMfl=967910603.878684503673016930058498370313;if (tdWoZQUGcbVcpJhGODmTrMxgQCOMfl == tdWoZQUGcbVcpJhGODmTrMxgQCOMfl ) tdWoZQUGcbVcpJhGODmTrMxgQCOMfl=2003703449.379059181802292302445074858042; else tdWoZQUGcbVcpJhGODmTrMxgQCOMfl=1645554713.420140517885036621617908841620;double EZuTqLEbIrwdgbEAVpIPnVPbdLJPAy=977214767.289027540149170493624272687505;if (EZuTqLEbIrwdgbEAVpIPnVPbdLJPAy == EZuTqLEbIrwdgbEAVpIPnVPbdLJPAy ) EZuTqLEbIrwdgbEAVpIPnVPbdLJPAy=279079165.144148371704192475500320712845; else EZuTqLEbIrwdgbEAVpIPnVPbdLJPAy=698092830.005510514313302118021211592554;if (EZuTqLEbIrwdgbEAVpIPnVPbdLJPAy == EZuTqLEbIrwdgbEAVpIPnVPbdLJPAy ) EZuTqLEbIrwdgbEAVpIPnVPbdLJPAy=1874555041.663770079397239803860482490503; else EZuTqLEbIrwdgbEAVpIPnVPbdLJPAy=55952567.916176064325084419289079428138;if (EZuTqLEbIrwdgbEAVpIPnVPbdLJPAy == EZuTqLEbIrwdgbEAVpIPnVPbdLJPAy ) EZuTqLEbIrwdgbEAVpIPnVPbdLJPAy=173236956.157649226395965651919133594776; else EZuTqLEbIrwdgbEAVpIPnVPbdLJPAy=2143801210.993627944639390412498833003425;if (EZuTqLEbIrwdgbEAVpIPnVPbdLJPAy == EZuTqLEbIrwdgbEAVpIPnVPbdLJPAy ) EZuTqLEbIrwdgbEAVpIPnVPbdLJPAy=1162652691.555150244972749094419338817173; else EZuTqLEbIrwdgbEAVpIPnVPbdLJPAy=1692067500.552918535038862629512912722735;if (EZuTqLEbIrwdgbEAVpIPnVPbdLJPAy == EZuTqLEbIrwdgbEAVpIPnVPbdLJPAy ) EZuTqLEbIrwdgbEAVpIPnVPbdLJPAy=1081158853.428253083166048168730876164139; else EZuTqLEbIrwdgbEAVpIPnVPbdLJPAy=1415969616.780072162974645607467511566219;if (EZuTqLEbIrwdgbEAVpIPnVPbdLJPAy == EZuTqLEbIrwdgbEAVpIPnVPbdLJPAy ) EZuTqLEbIrwdgbEAVpIPnVPbdLJPAy=1399922526.096458608710686623166787337062; else EZuTqLEbIrwdgbEAVpIPnVPbdLJPAy=340853221.605309930811076794604316431923;float fWhGEYRokhzRUbdpGDcSTWcoQvxwnl=2112178698.701653016116269774993791152675f;if (fWhGEYRokhzRUbdpGDcSTWcoQvxwnl - fWhGEYRokhzRUbdpGDcSTWcoQvxwnl> 0.00000001 ) fWhGEYRokhzRUbdpGDcSTWcoQvxwnl=2005244169.686878587832769179484493912160f; else fWhGEYRokhzRUbdpGDcSTWcoQvxwnl=1869822793.255154040130816103308320129407f;if (fWhGEYRokhzRUbdpGDcSTWcoQvxwnl - fWhGEYRokhzRUbdpGDcSTWcoQvxwnl> 0.00000001 ) fWhGEYRokhzRUbdpGDcSTWcoQvxwnl=1595655136.753779862624045578781086393513f; else fWhGEYRokhzRUbdpGDcSTWcoQvxwnl=1603879627.565459725835289549576282777848f;if (fWhGEYRokhzRUbdpGDcSTWcoQvxwnl - fWhGEYRokhzRUbdpGDcSTWcoQvxwnl> 0.00000001 ) fWhGEYRokhzRUbdpGDcSTWcoQvxwnl=2142014910.636916070785496691786515749378f; else fWhGEYRokhzRUbdpGDcSTWcoQvxwnl=1209158114.208730037467099289718617323128f;if (fWhGEYRokhzRUbdpGDcSTWcoQvxwnl - fWhGEYRokhzRUbdpGDcSTWcoQvxwnl> 0.00000001 ) fWhGEYRokhzRUbdpGDcSTWcoQvxwnl=2063563141.770258887777542052666177160141f; else fWhGEYRokhzRUbdpGDcSTWcoQvxwnl=829807170.791080794240204228547266261319f;if (fWhGEYRokhzRUbdpGDcSTWcoQvxwnl - fWhGEYRokhzRUbdpGDcSTWcoQvxwnl> 0.00000001 ) fWhGEYRokhzRUbdpGDcSTWcoQvxwnl=679511441.974619667337852797495271564682f; else fWhGEYRokhzRUbdpGDcSTWcoQvxwnl=837700827.048538770677231041567403581325f;if (fWhGEYRokhzRUbdpGDcSTWcoQvxwnl - fWhGEYRokhzRUbdpGDcSTWcoQvxwnl> 0.00000001 ) fWhGEYRokhzRUbdpGDcSTWcoQvxwnl=652029447.518549043503883804052323564606f; else fWhGEYRokhzRUbdpGDcSTWcoQvxwnl=9598296.845517979311907463188747430618f;int VQllcqOicuQamVJVGfDzIaOnCDpItm=1637667249;if (VQllcqOicuQamVJVGfDzIaOnCDpItm == VQllcqOicuQamVJVGfDzIaOnCDpItm- 0 ) VQllcqOicuQamVJVGfDzIaOnCDpItm=1931695501; else VQllcqOicuQamVJVGfDzIaOnCDpItm=1077077971;if (VQllcqOicuQamVJVGfDzIaOnCDpItm == VQllcqOicuQamVJVGfDzIaOnCDpItm- 1 ) VQllcqOicuQamVJVGfDzIaOnCDpItm=1051537127; else VQllcqOicuQamVJVGfDzIaOnCDpItm=1929104311;if (VQllcqOicuQamVJVGfDzIaOnCDpItm == VQllcqOicuQamVJVGfDzIaOnCDpItm- 1 ) VQllcqOicuQamVJVGfDzIaOnCDpItm=1608518005; else VQllcqOicuQamVJVGfDzIaOnCDpItm=1719460143;if (VQllcqOicuQamVJVGfDzIaOnCDpItm == VQllcqOicuQamVJVGfDzIaOnCDpItm- 1 ) VQllcqOicuQamVJVGfDzIaOnCDpItm=1020107119; else VQllcqOicuQamVJVGfDzIaOnCDpItm=1814004362;if (VQllcqOicuQamVJVGfDzIaOnCDpItm == VQllcqOicuQamVJVGfDzIaOnCDpItm- 1 ) VQllcqOicuQamVJVGfDzIaOnCDpItm=853654832; else VQllcqOicuQamVJVGfDzIaOnCDpItm=2097474037;if (VQllcqOicuQamVJVGfDzIaOnCDpItm == VQllcqOicuQamVJVGfDzIaOnCDpItm- 1 ) VQllcqOicuQamVJVGfDzIaOnCDpItm=1660256004; else VQllcqOicuQamVJVGfDzIaOnCDpItm=1240993464;double kdJWlUqEyFpIPAyEuiGEsONHOnDHLX=1765244094.182103736898176531662345685909;if (kdJWlUqEyFpIPAyEuiGEsONHOnDHLX == kdJWlUqEyFpIPAyEuiGEsONHOnDHLX ) kdJWlUqEyFpIPAyEuiGEsONHOnDHLX=2142496513.549394880423490915341108385863; else kdJWlUqEyFpIPAyEuiGEsONHOnDHLX=2039008398.994845088376135905688066845112;if (kdJWlUqEyFpIPAyEuiGEsONHOnDHLX == kdJWlUqEyFpIPAyEuiGEsONHOnDHLX ) kdJWlUqEyFpIPAyEuiGEsONHOnDHLX=1787794191.912754075503717608449028693800; else kdJWlUqEyFpIPAyEuiGEsONHOnDHLX=1122283911.197747458194119536024888309427;if (kdJWlUqEyFpIPAyEuiGEsONHOnDHLX == kdJWlUqEyFpIPAyEuiGEsONHOnDHLX ) kdJWlUqEyFpIPAyEuiGEsONHOnDHLX=1846738858.237652618036035502027789641045; else kdJWlUqEyFpIPAyEuiGEsONHOnDHLX=1404066320.904297857893800985390850055902;if (kdJWlUqEyFpIPAyEuiGEsONHOnDHLX == kdJWlUqEyFpIPAyEuiGEsONHOnDHLX ) kdJWlUqEyFpIPAyEuiGEsONHOnDHLX=982683504.177423394985533409986686516425; else kdJWlUqEyFpIPAyEuiGEsONHOnDHLX=1217404698.296673758869141532876702650192;if (kdJWlUqEyFpIPAyEuiGEsONHOnDHLX == kdJWlUqEyFpIPAyEuiGEsONHOnDHLX ) kdJWlUqEyFpIPAyEuiGEsONHOnDHLX=782013336.898089907175122864609565969503; else kdJWlUqEyFpIPAyEuiGEsONHOnDHLX=1044747871.466717127263680269080660434413;if (kdJWlUqEyFpIPAyEuiGEsONHOnDHLX == kdJWlUqEyFpIPAyEuiGEsONHOnDHLX ) kdJWlUqEyFpIPAyEuiGEsONHOnDHLX=1523924773.050572951574844609580052380748; else kdJWlUqEyFpIPAyEuiGEsONHOnDHLX=1483019776.481522321931952683684593300829;int LGTEtRSHLMpsecwKADkBmFcQxvmJKK=2121082881;if (LGTEtRSHLMpsecwKADkBmFcQxvmJKK == LGTEtRSHLMpsecwKADkBmFcQxvmJKK- 1 ) LGTEtRSHLMpsecwKADkBmFcQxvmJKK=477063582; else LGTEtRSHLMpsecwKADkBmFcQxvmJKK=1909738924;if (LGTEtRSHLMpsecwKADkBmFcQxvmJKK == LGTEtRSHLMpsecwKADkBmFcQxvmJKK- 1 ) LGTEtRSHLMpsecwKADkBmFcQxvmJKK=556641367; else LGTEtRSHLMpsecwKADkBmFcQxvmJKK=31790306;if (LGTEtRSHLMpsecwKADkBmFcQxvmJKK == LGTEtRSHLMpsecwKADkBmFcQxvmJKK- 0 ) LGTEtRSHLMpsecwKADkBmFcQxvmJKK=987256060; else LGTEtRSHLMpsecwKADkBmFcQxvmJKK=1191277073;if (LGTEtRSHLMpsecwKADkBmFcQxvmJKK == LGTEtRSHLMpsecwKADkBmFcQxvmJKK- 0 ) LGTEtRSHLMpsecwKADkBmFcQxvmJKK=503660693; else LGTEtRSHLMpsecwKADkBmFcQxvmJKK=285784026;if (LGTEtRSHLMpsecwKADkBmFcQxvmJKK == LGTEtRSHLMpsecwKADkBmFcQxvmJKK- 0 ) LGTEtRSHLMpsecwKADkBmFcQxvmJKK=849715674; else LGTEtRSHLMpsecwKADkBmFcQxvmJKK=1929134421;if (LGTEtRSHLMpsecwKADkBmFcQxvmJKK == LGTEtRSHLMpsecwKADkBmFcQxvmJKK- 0 ) LGTEtRSHLMpsecwKADkBmFcQxvmJKK=750224795; else LGTEtRSHLMpsecwKADkBmFcQxvmJKK=44540873;int ZjKzNWXngnxnVmSDtPiQxqwKbZcWqj=1501955696;if (ZjKzNWXngnxnVmSDtPiQxqwKbZcWqj == ZjKzNWXngnxnVmSDtPiQxqwKbZcWqj- 0 ) ZjKzNWXngnxnVmSDtPiQxqwKbZcWqj=921633797; else ZjKzNWXngnxnVmSDtPiQxqwKbZcWqj=1034269000;if (ZjKzNWXngnxnVmSDtPiQxqwKbZcWqj == ZjKzNWXngnxnVmSDtPiQxqwKbZcWqj- 1 ) ZjKzNWXngnxnVmSDtPiQxqwKbZcWqj=14585430; else ZjKzNWXngnxnVmSDtPiQxqwKbZcWqj=1103650910;if (ZjKzNWXngnxnVmSDtPiQxqwKbZcWqj == ZjKzNWXngnxnVmSDtPiQxqwKbZcWqj- 0 ) ZjKzNWXngnxnVmSDtPiQxqwKbZcWqj=1467249075; else ZjKzNWXngnxnVmSDtPiQxqwKbZcWqj=1964105770;if (ZjKzNWXngnxnVmSDtPiQxqwKbZcWqj == ZjKzNWXngnxnVmSDtPiQxqwKbZcWqj- 1 ) ZjKzNWXngnxnVmSDtPiQxqwKbZcWqj=1938412038; else ZjKzNWXngnxnVmSDtPiQxqwKbZcWqj=527629181;if (ZjKzNWXngnxnVmSDtPiQxqwKbZcWqj == ZjKzNWXngnxnVmSDtPiQxqwKbZcWqj- 1 ) ZjKzNWXngnxnVmSDtPiQxqwKbZcWqj=94394467; else ZjKzNWXngnxnVmSDtPiQxqwKbZcWqj=1981616076;if (ZjKzNWXngnxnVmSDtPiQxqwKbZcWqj == ZjKzNWXngnxnVmSDtPiQxqwKbZcWqj- 0 ) ZjKzNWXngnxnVmSDtPiQxqwKbZcWqj=1147090975; else ZjKzNWXngnxnVmSDtPiQxqwKbZcWqj=563770134;int qUvNxvbBNGTkoOAIFOnBfUBhBXvJIR=1848084084;if (qUvNxvbBNGTkoOAIFOnBfUBhBXvJIR == qUvNxvbBNGTkoOAIFOnBfUBhBXvJIR- 0 ) qUvNxvbBNGTkoOAIFOnBfUBhBXvJIR=1400582198; else qUvNxvbBNGTkoOAIFOnBfUBhBXvJIR=1686656770;if (qUvNxvbBNGTkoOAIFOnBfUBhBXvJIR == qUvNxvbBNGTkoOAIFOnBfUBhBXvJIR- 0 ) qUvNxvbBNGTkoOAIFOnBfUBhBXvJIR=763929882; else qUvNxvbBNGTkoOAIFOnBfUBhBXvJIR=582673930;if (qUvNxvbBNGTkoOAIFOnBfUBhBXvJIR == qUvNxvbBNGTkoOAIFOnBfUBhBXvJIR- 0 ) qUvNxvbBNGTkoOAIFOnBfUBhBXvJIR=768398025; else qUvNxvbBNGTkoOAIFOnBfUBhBXvJIR=2031760371;if (qUvNxvbBNGTkoOAIFOnBfUBhBXvJIR == qUvNxvbBNGTkoOAIFOnBfUBhBXvJIR- 0 ) qUvNxvbBNGTkoOAIFOnBfUBhBXvJIR=777988034; else qUvNxvbBNGTkoOAIFOnBfUBhBXvJIR=951219440;if (qUvNxvbBNGTkoOAIFOnBfUBhBXvJIR == qUvNxvbBNGTkoOAIFOnBfUBhBXvJIR- 1 ) qUvNxvbBNGTkoOAIFOnBfUBhBXvJIR=1535347705; else qUvNxvbBNGTkoOAIFOnBfUBhBXvJIR=1502010165;if (qUvNxvbBNGTkoOAIFOnBfUBhBXvJIR == qUvNxvbBNGTkoOAIFOnBfUBhBXvJIR- 0 ) qUvNxvbBNGTkoOAIFOnBfUBhBXvJIR=272568237; else qUvNxvbBNGTkoOAIFOnBfUBhBXvJIR=2065088084;int QQKYwBRUrmiFafptHpfxpTVRtUdMmV=2106927306;if (QQKYwBRUrmiFafptHpfxpTVRtUdMmV == QQKYwBRUrmiFafptHpfxpTVRtUdMmV- 0 ) QQKYwBRUrmiFafptHpfxpTVRtUdMmV=559869908; else QQKYwBRUrmiFafptHpfxpTVRtUdMmV=21969077;if (QQKYwBRUrmiFafptHpfxpTVRtUdMmV == QQKYwBRUrmiFafptHpfxpTVRtUdMmV- 1 ) QQKYwBRUrmiFafptHpfxpTVRtUdMmV=561385979; else QQKYwBRUrmiFafptHpfxpTVRtUdMmV=457502763;if (QQKYwBRUrmiFafptHpfxpTVRtUdMmV == QQKYwBRUrmiFafptHpfxpTVRtUdMmV- 1 ) QQKYwBRUrmiFafptHpfxpTVRtUdMmV=1138468965; else QQKYwBRUrmiFafptHpfxpTVRtUdMmV=1740724576;if (QQKYwBRUrmiFafptHpfxpTVRtUdMmV == QQKYwBRUrmiFafptHpfxpTVRtUdMmV- 1 ) QQKYwBRUrmiFafptHpfxpTVRtUdMmV=653833806; else QQKYwBRUrmiFafptHpfxpTVRtUdMmV=660441459;if (QQKYwBRUrmiFafptHpfxpTVRtUdMmV == QQKYwBRUrmiFafptHpfxpTVRtUdMmV- 1 ) QQKYwBRUrmiFafptHpfxpTVRtUdMmV=2118810533; else QQKYwBRUrmiFafptHpfxpTVRtUdMmV=29012186;if (QQKYwBRUrmiFafptHpfxpTVRtUdMmV == QQKYwBRUrmiFafptHpfxpTVRtUdMmV- 1 ) QQKYwBRUrmiFafptHpfxpTVRtUdMmV=2084879486; else QQKYwBRUrmiFafptHpfxpTVRtUdMmV=1153123631;double VhsGTYpNusViCGCpaCoWOeBXvWKWVB=1312628331.067266224166539151517117643976;if (VhsGTYpNusViCGCpaCoWOeBXvWKWVB == VhsGTYpNusViCGCpaCoWOeBXvWKWVB ) VhsGTYpNusViCGCpaCoWOeBXvWKWVB=1606901822.781773810273456658240442006073; else VhsGTYpNusViCGCpaCoWOeBXvWKWVB=1049624358.435948586634235576200584745087;if (VhsGTYpNusViCGCpaCoWOeBXvWKWVB == VhsGTYpNusViCGCpaCoWOeBXvWKWVB ) VhsGTYpNusViCGCpaCoWOeBXvWKWVB=791442322.884794861827261621854098496121; else VhsGTYpNusViCGCpaCoWOeBXvWKWVB=9728987.461986155272659332422523490740;if (VhsGTYpNusViCGCpaCoWOeBXvWKWVB == VhsGTYpNusViCGCpaCoWOeBXvWKWVB ) VhsGTYpNusViCGCpaCoWOeBXvWKWVB=58639795.984156749568779255927471904950; else VhsGTYpNusViCGCpaCoWOeBXvWKWVB=2049687680.742640950360496486535683451425;if (VhsGTYpNusViCGCpaCoWOeBXvWKWVB == VhsGTYpNusViCGCpaCoWOeBXvWKWVB ) VhsGTYpNusViCGCpaCoWOeBXvWKWVB=733557671.390992724940566719552842527939; else VhsGTYpNusViCGCpaCoWOeBXvWKWVB=1037357073.954618151587360612774397455858;if (VhsGTYpNusViCGCpaCoWOeBXvWKWVB == VhsGTYpNusViCGCpaCoWOeBXvWKWVB ) VhsGTYpNusViCGCpaCoWOeBXvWKWVB=256280966.589509706976818669301527577726; else VhsGTYpNusViCGCpaCoWOeBXvWKWVB=1210849637.331300896292790859090765817304;if (VhsGTYpNusViCGCpaCoWOeBXvWKWVB == VhsGTYpNusViCGCpaCoWOeBXvWKWVB ) VhsGTYpNusViCGCpaCoWOeBXvWKWVB=1715786852.218075661887411994772974778660; else VhsGTYpNusViCGCpaCoWOeBXvWKWVB=2112722557.459209473639821335492473244541;double xCITQqIDJaMDGWOEZPSQOJioDqgzhd=1303735483.361984391186655608929808617901;if (xCITQqIDJaMDGWOEZPSQOJioDqgzhd == xCITQqIDJaMDGWOEZPSQOJioDqgzhd ) xCITQqIDJaMDGWOEZPSQOJioDqgzhd=313113488.782227566478617386217770467348; else xCITQqIDJaMDGWOEZPSQOJioDqgzhd=1187647316.850141543825355229233279512397;if (xCITQqIDJaMDGWOEZPSQOJioDqgzhd == xCITQqIDJaMDGWOEZPSQOJioDqgzhd ) xCITQqIDJaMDGWOEZPSQOJioDqgzhd=3900226.122973858795350242534921578173; else xCITQqIDJaMDGWOEZPSQOJioDqgzhd=173566293.926780349347165803037086331241;if (xCITQqIDJaMDGWOEZPSQOJioDqgzhd == xCITQqIDJaMDGWOEZPSQOJioDqgzhd ) xCITQqIDJaMDGWOEZPSQOJioDqgzhd=1123185011.579587067001420886778625704046; else xCITQqIDJaMDGWOEZPSQOJioDqgzhd=2012213483.020898979894614767766557406499;if (xCITQqIDJaMDGWOEZPSQOJioDqgzhd == xCITQqIDJaMDGWOEZPSQOJioDqgzhd ) xCITQqIDJaMDGWOEZPSQOJioDqgzhd=709615119.403208164301899401114849157359; else xCITQqIDJaMDGWOEZPSQOJioDqgzhd=1807341269.259059152661357586900538567525;if (xCITQqIDJaMDGWOEZPSQOJioDqgzhd == xCITQqIDJaMDGWOEZPSQOJioDqgzhd ) xCITQqIDJaMDGWOEZPSQOJioDqgzhd=1032822964.181515385046065460268156972169; else xCITQqIDJaMDGWOEZPSQOJioDqgzhd=103488423.363758084231296558421836814674;if (xCITQqIDJaMDGWOEZPSQOJioDqgzhd == xCITQqIDJaMDGWOEZPSQOJioDqgzhd ) xCITQqIDJaMDGWOEZPSQOJioDqgzhd=611347044.423688341748614706152484954705; else xCITQqIDJaMDGWOEZPSQOJioDqgzhd=739385839.552191962922878333311863205973;double gVcJxhZLOuUZJMBhFcfPDVtWpvvuti=1595457146.262867911510495337612902986664;if (gVcJxhZLOuUZJMBhFcfPDVtWpvvuti == gVcJxhZLOuUZJMBhFcfPDVtWpvvuti ) gVcJxhZLOuUZJMBhFcfPDVtWpvvuti=1411761509.804991437606686186761597937199; else gVcJxhZLOuUZJMBhFcfPDVtWpvvuti=1107423553.146882750400112742790101431716;if (gVcJxhZLOuUZJMBhFcfPDVtWpvvuti == gVcJxhZLOuUZJMBhFcfPDVtWpvvuti ) gVcJxhZLOuUZJMBhFcfPDVtWpvvuti=458186262.544857081443736149630541914655; else gVcJxhZLOuUZJMBhFcfPDVtWpvvuti=1751700814.215497636676235757530228710405;if (gVcJxhZLOuUZJMBhFcfPDVtWpvvuti == gVcJxhZLOuUZJMBhFcfPDVtWpvvuti ) gVcJxhZLOuUZJMBhFcfPDVtWpvvuti=146700309.354634935051416441726722600879; else gVcJxhZLOuUZJMBhFcfPDVtWpvvuti=1658773601.734132361192358981247079973214;if (gVcJxhZLOuUZJMBhFcfPDVtWpvvuti == gVcJxhZLOuUZJMBhFcfPDVtWpvvuti ) gVcJxhZLOuUZJMBhFcfPDVtWpvvuti=2048287511.516642839528173558472964277537; else gVcJxhZLOuUZJMBhFcfPDVtWpvvuti=657665875.016869671726382000752684276247;if (gVcJxhZLOuUZJMBhFcfPDVtWpvvuti == gVcJxhZLOuUZJMBhFcfPDVtWpvvuti ) gVcJxhZLOuUZJMBhFcfPDVtWpvvuti=1435895053.631361231598785768338799544370; else gVcJxhZLOuUZJMBhFcfPDVtWpvvuti=1671512553.353529513287026500951802095282;if (gVcJxhZLOuUZJMBhFcfPDVtWpvvuti == gVcJxhZLOuUZJMBhFcfPDVtWpvvuti ) gVcJxhZLOuUZJMBhFcfPDVtWpvvuti=201221797.807125537726616187314442065253; else gVcJxhZLOuUZJMBhFcfPDVtWpvvuti=2075102975.533149091208433539691808856526;int PmSGuPWlmFueDDjzGyaSbfgKmRYQxM=98002932;if (PmSGuPWlmFueDDjzGyaSbfgKmRYQxM == PmSGuPWlmFueDDjzGyaSbfgKmRYQxM- 1 ) PmSGuPWlmFueDDjzGyaSbfgKmRYQxM=155224530; else PmSGuPWlmFueDDjzGyaSbfgKmRYQxM=781144003;if (PmSGuPWlmFueDDjzGyaSbfgKmRYQxM == PmSGuPWlmFueDDjzGyaSbfgKmRYQxM- 0 ) PmSGuPWlmFueDDjzGyaSbfgKmRYQxM=840010143; else PmSGuPWlmFueDDjzGyaSbfgKmRYQxM=1004022429;if (PmSGuPWlmFueDDjzGyaSbfgKmRYQxM == PmSGuPWlmFueDDjzGyaSbfgKmRYQxM- 0 ) PmSGuPWlmFueDDjzGyaSbfgKmRYQxM=2083473980; else PmSGuPWlmFueDDjzGyaSbfgKmRYQxM=2064166010;if (PmSGuPWlmFueDDjzGyaSbfgKmRYQxM == PmSGuPWlmFueDDjzGyaSbfgKmRYQxM- 1 ) PmSGuPWlmFueDDjzGyaSbfgKmRYQxM=189443320; else PmSGuPWlmFueDDjzGyaSbfgKmRYQxM=1742171986;if (PmSGuPWlmFueDDjzGyaSbfgKmRYQxM == PmSGuPWlmFueDDjzGyaSbfgKmRYQxM- 0 ) PmSGuPWlmFueDDjzGyaSbfgKmRYQxM=340009239; else PmSGuPWlmFueDDjzGyaSbfgKmRYQxM=1131584700;if (PmSGuPWlmFueDDjzGyaSbfgKmRYQxM == PmSGuPWlmFueDDjzGyaSbfgKmRYQxM- 0 ) PmSGuPWlmFueDDjzGyaSbfgKmRYQxM=1124389670; else PmSGuPWlmFueDDjzGyaSbfgKmRYQxM=2102635019;double IZgGggJPzbUGcPgusYXbfKBMsDqDIU=631416563.840965310359775977474715566884;if (IZgGggJPzbUGcPgusYXbfKBMsDqDIU == IZgGggJPzbUGcPgusYXbfKBMsDqDIU ) IZgGggJPzbUGcPgusYXbfKBMsDqDIU=303816295.476375173184622400696668814550; else IZgGggJPzbUGcPgusYXbfKBMsDqDIU=120329706.954698440181449110326720992957;if (IZgGggJPzbUGcPgusYXbfKBMsDqDIU == IZgGggJPzbUGcPgusYXbfKBMsDqDIU ) IZgGggJPzbUGcPgusYXbfKBMsDqDIU=700961048.263324013507061523971433433570; else IZgGggJPzbUGcPgusYXbfKBMsDqDIU=584113980.769402912611792077948008684090;if (IZgGggJPzbUGcPgusYXbfKBMsDqDIU == IZgGggJPzbUGcPgusYXbfKBMsDqDIU ) IZgGggJPzbUGcPgusYXbfKBMsDqDIU=1562621339.164485867971137085256763386182; else IZgGggJPzbUGcPgusYXbfKBMsDqDIU=269570489.992366831722008798270095440121;if (IZgGggJPzbUGcPgusYXbfKBMsDqDIU == IZgGggJPzbUGcPgusYXbfKBMsDqDIU ) IZgGggJPzbUGcPgusYXbfKBMsDqDIU=1157035174.088508988135454576669892301803; else IZgGggJPzbUGcPgusYXbfKBMsDqDIU=801823534.672085460092125664107467329731;if (IZgGggJPzbUGcPgusYXbfKBMsDqDIU == IZgGggJPzbUGcPgusYXbfKBMsDqDIU ) IZgGggJPzbUGcPgusYXbfKBMsDqDIU=1286843452.819557458022777788875781631686; else IZgGggJPzbUGcPgusYXbfKBMsDqDIU=1493717998.773028467831729680065196614803;if (IZgGggJPzbUGcPgusYXbfKBMsDqDIU == IZgGggJPzbUGcPgusYXbfKBMsDqDIU ) IZgGggJPzbUGcPgusYXbfKBMsDqDIU=885154887.687345975043105406315263168841; else IZgGggJPzbUGcPgusYXbfKBMsDqDIU=1599156105.466535558571602300131860852528;double HOQdqihmBwcppTTfeDtbsgIAfygipM=344228068.755243383380492538885422726629;if (HOQdqihmBwcppTTfeDtbsgIAfygipM == HOQdqihmBwcppTTfeDtbsgIAfygipM ) HOQdqihmBwcppTTfeDtbsgIAfygipM=1134584497.669996191878346584128690033363; else HOQdqihmBwcppTTfeDtbsgIAfygipM=513344112.786572160744811829724361217097;if (HOQdqihmBwcppTTfeDtbsgIAfygipM == HOQdqihmBwcppTTfeDtbsgIAfygipM ) HOQdqihmBwcppTTfeDtbsgIAfygipM=584161309.394810297452448498931564953637; else HOQdqihmBwcppTTfeDtbsgIAfygipM=1313220529.518356101263338392382916693007;if (HOQdqihmBwcppTTfeDtbsgIAfygipM == HOQdqihmBwcppTTfeDtbsgIAfygipM ) HOQdqihmBwcppTTfeDtbsgIAfygipM=932337907.776094510339877924776235670287; else HOQdqihmBwcppTTfeDtbsgIAfygipM=2050494560.321460734498316425087043164153;if (HOQdqihmBwcppTTfeDtbsgIAfygipM == HOQdqihmBwcppTTfeDtbsgIAfygipM ) HOQdqihmBwcppTTfeDtbsgIAfygipM=1659466813.534302842729149892035630039840; else HOQdqihmBwcppTTfeDtbsgIAfygipM=1495079146.769106501568336800537223890447;if (HOQdqihmBwcppTTfeDtbsgIAfygipM == HOQdqihmBwcppTTfeDtbsgIAfygipM ) HOQdqihmBwcppTTfeDtbsgIAfygipM=917980233.411217105340267840534941455731; else HOQdqihmBwcppTTfeDtbsgIAfygipM=863497923.617426722080630817616101467097;if (HOQdqihmBwcppTTfeDtbsgIAfygipM == HOQdqihmBwcppTTfeDtbsgIAfygipM ) HOQdqihmBwcppTTfeDtbsgIAfygipM=1411691575.257474484222865532274913661617; else HOQdqihmBwcppTTfeDtbsgIAfygipM=1162599256.974684336744340434704692344581;double qhoTWrLrIHvdbLCdjRKSkuHPNpHRRw=1441723221.412331088687016645056859434334;if (qhoTWrLrIHvdbLCdjRKSkuHPNpHRRw == qhoTWrLrIHvdbLCdjRKSkuHPNpHRRw ) qhoTWrLrIHvdbLCdjRKSkuHPNpHRRw=581036365.761887118994137000559451266390; else qhoTWrLrIHvdbLCdjRKSkuHPNpHRRw=1717288881.876492633964717272344588541063;if (qhoTWrLrIHvdbLCdjRKSkuHPNpHRRw == qhoTWrLrIHvdbLCdjRKSkuHPNpHRRw ) qhoTWrLrIHvdbLCdjRKSkuHPNpHRRw=1771286680.192822984557770674510515739996; else qhoTWrLrIHvdbLCdjRKSkuHPNpHRRw=2052091665.267188390530980782807766378729;if (qhoTWrLrIHvdbLCdjRKSkuHPNpHRRw == qhoTWrLrIHvdbLCdjRKSkuHPNpHRRw ) qhoTWrLrIHvdbLCdjRKSkuHPNpHRRw=2135117495.310949179338518063616819951281; else qhoTWrLrIHvdbLCdjRKSkuHPNpHRRw=532273593.068223105161367035832621361109;if (qhoTWrLrIHvdbLCdjRKSkuHPNpHRRw == qhoTWrLrIHvdbLCdjRKSkuHPNpHRRw ) qhoTWrLrIHvdbLCdjRKSkuHPNpHRRw=682865240.900948039607955555203645044806; else qhoTWrLrIHvdbLCdjRKSkuHPNpHRRw=2033137688.200637253001624577782601746590;if (qhoTWrLrIHvdbLCdjRKSkuHPNpHRRw == qhoTWrLrIHvdbLCdjRKSkuHPNpHRRw ) qhoTWrLrIHvdbLCdjRKSkuHPNpHRRw=1771592476.434441743940331202258722571090; else qhoTWrLrIHvdbLCdjRKSkuHPNpHRRw=38186609.177467839394595348154544382298;if (qhoTWrLrIHvdbLCdjRKSkuHPNpHRRw == qhoTWrLrIHvdbLCdjRKSkuHPNpHRRw ) qhoTWrLrIHvdbLCdjRKSkuHPNpHRRw=1864662624.175487307458406434734305674319; else qhoTWrLrIHvdbLCdjRKSkuHPNpHRRw=589755982.664123011905180715895957587081;double AqpmvQMmRXJMtQuJLYxdyaNUnFnhUc=1705497095.594154071301231188601318068669;if (AqpmvQMmRXJMtQuJLYxdyaNUnFnhUc == AqpmvQMmRXJMtQuJLYxdyaNUnFnhUc ) AqpmvQMmRXJMtQuJLYxdyaNUnFnhUc=1496664802.920811415413583370520547828720; else AqpmvQMmRXJMtQuJLYxdyaNUnFnhUc=780161602.752358243077330893579914948310;if (AqpmvQMmRXJMtQuJLYxdyaNUnFnhUc == AqpmvQMmRXJMtQuJLYxdyaNUnFnhUc ) AqpmvQMmRXJMtQuJLYxdyaNUnFnhUc=968050522.380586224524755431803625185851; else AqpmvQMmRXJMtQuJLYxdyaNUnFnhUc=924996524.193470886577916626062761787550;if (AqpmvQMmRXJMtQuJLYxdyaNUnFnhUc == AqpmvQMmRXJMtQuJLYxdyaNUnFnhUc ) AqpmvQMmRXJMtQuJLYxdyaNUnFnhUc=1557494170.158353148665644671067989044370; else AqpmvQMmRXJMtQuJLYxdyaNUnFnhUc=1924739306.334742966458850822711669337942;if (AqpmvQMmRXJMtQuJLYxdyaNUnFnhUc == AqpmvQMmRXJMtQuJLYxdyaNUnFnhUc ) AqpmvQMmRXJMtQuJLYxdyaNUnFnhUc=1846562303.998933190282671014032267088552; else AqpmvQMmRXJMtQuJLYxdyaNUnFnhUc=400805382.503557003874364010932402084206;if (AqpmvQMmRXJMtQuJLYxdyaNUnFnhUc == AqpmvQMmRXJMtQuJLYxdyaNUnFnhUc ) AqpmvQMmRXJMtQuJLYxdyaNUnFnhUc=608346540.575988912402137901307962111268; else AqpmvQMmRXJMtQuJLYxdyaNUnFnhUc=1353365549.491831948710864048518390971397;if (AqpmvQMmRXJMtQuJLYxdyaNUnFnhUc == AqpmvQMmRXJMtQuJLYxdyaNUnFnhUc ) AqpmvQMmRXJMtQuJLYxdyaNUnFnhUc=1813617394.786512241616379811918376010196; else AqpmvQMmRXJMtQuJLYxdyaNUnFnhUc=699123416.065204545280800213117058941061;int EZYfWFFjbRmghNrdmclylApwBHkOcP=1341190960;if (EZYfWFFjbRmghNrdmclylApwBHkOcP == EZYfWFFjbRmghNrdmclylApwBHkOcP- 0 ) EZYfWFFjbRmghNrdmclylApwBHkOcP=126909112; else EZYfWFFjbRmghNrdmclylApwBHkOcP=1590915313;if (EZYfWFFjbRmghNrdmclylApwBHkOcP == EZYfWFFjbRmghNrdmclylApwBHkOcP- 1 ) EZYfWFFjbRmghNrdmclylApwBHkOcP=1018119740; else EZYfWFFjbRmghNrdmclylApwBHkOcP=93702805;if (EZYfWFFjbRmghNrdmclylApwBHkOcP == EZYfWFFjbRmghNrdmclylApwBHkOcP- 1 ) EZYfWFFjbRmghNrdmclylApwBHkOcP=1158192758; else EZYfWFFjbRmghNrdmclylApwBHkOcP=1877795527;if (EZYfWFFjbRmghNrdmclylApwBHkOcP == EZYfWFFjbRmghNrdmclylApwBHkOcP- 1 ) EZYfWFFjbRmghNrdmclylApwBHkOcP=356594220; else EZYfWFFjbRmghNrdmclylApwBHkOcP=602310904;if (EZYfWFFjbRmghNrdmclylApwBHkOcP == EZYfWFFjbRmghNrdmclylApwBHkOcP- 0 ) EZYfWFFjbRmghNrdmclylApwBHkOcP=1977962519; else EZYfWFFjbRmghNrdmclylApwBHkOcP=698507268;if (EZYfWFFjbRmghNrdmclylApwBHkOcP == EZYfWFFjbRmghNrdmclylApwBHkOcP- 1 ) EZYfWFFjbRmghNrdmclylApwBHkOcP=1689111700; else EZYfWFFjbRmghNrdmclylApwBHkOcP=894151298;long puaYRjuuZeBkNcaLKkppQzBIwCNpLa=180209371;if (puaYRjuuZeBkNcaLKkppQzBIwCNpLa == puaYRjuuZeBkNcaLKkppQzBIwCNpLa- 0 ) puaYRjuuZeBkNcaLKkppQzBIwCNpLa=932317623; else puaYRjuuZeBkNcaLKkppQzBIwCNpLa=1853678127;if (puaYRjuuZeBkNcaLKkppQzBIwCNpLa == puaYRjuuZeBkNcaLKkppQzBIwCNpLa- 1 ) puaYRjuuZeBkNcaLKkppQzBIwCNpLa=1813418101; else puaYRjuuZeBkNcaLKkppQzBIwCNpLa=1901705976;if (puaYRjuuZeBkNcaLKkppQzBIwCNpLa == puaYRjuuZeBkNcaLKkppQzBIwCNpLa- 1 ) puaYRjuuZeBkNcaLKkppQzBIwCNpLa=1125085617; else puaYRjuuZeBkNcaLKkppQzBIwCNpLa=441812362;if (puaYRjuuZeBkNcaLKkppQzBIwCNpLa == puaYRjuuZeBkNcaLKkppQzBIwCNpLa- 1 ) puaYRjuuZeBkNcaLKkppQzBIwCNpLa=2031712698; else puaYRjuuZeBkNcaLKkppQzBIwCNpLa=803780706;if (puaYRjuuZeBkNcaLKkppQzBIwCNpLa == puaYRjuuZeBkNcaLKkppQzBIwCNpLa- 1 ) puaYRjuuZeBkNcaLKkppQzBIwCNpLa=2018210225; else puaYRjuuZeBkNcaLKkppQzBIwCNpLa=1370481298;if (puaYRjuuZeBkNcaLKkppQzBIwCNpLa == puaYRjuuZeBkNcaLKkppQzBIwCNpLa- 0 ) puaYRjuuZeBkNcaLKkppQzBIwCNpLa=1443745125; else puaYRjuuZeBkNcaLKkppQzBIwCNpLa=781751946;int COZRlXlzXhhBIMFkxUBUGWUMbSZHKc=1777156558;if (COZRlXlzXhhBIMFkxUBUGWUMbSZHKc == COZRlXlzXhhBIMFkxUBUGWUMbSZHKc- 1 ) COZRlXlzXhhBIMFkxUBUGWUMbSZHKc=1918230653; else COZRlXlzXhhBIMFkxUBUGWUMbSZHKc=523471664;if (COZRlXlzXhhBIMFkxUBUGWUMbSZHKc == COZRlXlzXhhBIMFkxUBUGWUMbSZHKc- 1 ) COZRlXlzXhhBIMFkxUBUGWUMbSZHKc=462846870; else COZRlXlzXhhBIMFkxUBUGWUMbSZHKc=1735579457;if (COZRlXlzXhhBIMFkxUBUGWUMbSZHKc == COZRlXlzXhhBIMFkxUBUGWUMbSZHKc- 0 ) COZRlXlzXhhBIMFkxUBUGWUMbSZHKc=1867134769; else COZRlXlzXhhBIMFkxUBUGWUMbSZHKc=1984961142;if (COZRlXlzXhhBIMFkxUBUGWUMbSZHKc == COZRlXlzXhhBIMFkxUBUGWUMbSZHKc- 0 ) COZRlXlzXhhBIMFkxUBUGWUMbSZHKc=547304337; else COZRlXlzXhhBIMFkxUBUGWUMbSZHKc=1766352922;if (COZRlXlzXhhBIMFkxUBUGWUMbSZHKc == COZRlXlzXhhBIMFkxUBUGWUMbSZHKc- 0 ) COZRlXlzXhhBIMFkxUBUGWUMbSZHKc=423567382; else COZRlXlzXhhBIMFkxUBUGWUMbSZHKc=365739618;if (COZRlXlzXhhBIMFkxUBUGWUMbSZHKc == COZRlXlzXhhBIMFkxUBUGWUMbSZHKc- 0 ) COZRlXlzXhhBIMFkxUBUGWUMbSZHKc=1094517652; else COZRlXlzXhhBIMFkxUBUGWUMbSZHKc=858986902;int vjOFaCKyHhRroCLwyjAnVcpoFHuUOz=1686119356;if (vjOFaCKyHhRroCLwyjAnVcpoFHuUOz == vjOFaCKyHhRroCLwyjAnVcpoFHuUOz- 0 ) vjOFaCKyHhRroCLwyjAnVcpoFHuUOz=1037297526; else vjOFaCKyHhRroCLwyjAnVcpoFHuUOz=1633408023;if (vjOFaCKyHhRroCLwyjAnVcpoFHuUOz == vjOFaCKyHhRroCLwyjAnVcpoFHuUOz- 0 ) vjOFaCKyHhRroCLwyjAnVcpoFHuUOz=1914289440; else vjOFaCKyHhRroCLwyjAnVcpoFHuUOz=1299168081;if (vjOFaCKyHhRroCLwyjAnVcpoFHuUOz == vjOFaCKyHhRroCLwyjAnVcpoFHuUOz- 1 ) vjOFaCKyHhRroCLwyjAnVcpoFHuUOz=328501464; else vjOFaCKyHhRroCLwyjAnVcpoFHuUOz=513217274;if (vjOFaCKyHhRroCLwyjAnVcpoFHuUOz == vjOFaCKyHhRroCLwyjAnVcpoFHuUOz- 0 ) vjOFaCKyHhRroCLwyjAnVcpoFHuUOz=216105343; else vjOFaCKyHhRroCLwyjAnVcpoFHuUOz=1832580397;if (vjOFaCKyHhRroCLwyjAnVcpoFHuUOz == vjOFaCKyHhRroCLwyjAnVcpoFHuUOz- 0 ) vjOFaCKyHhRroCLwyjAnVcpoFHuUOz=1706686262; else vjOFaCKyHhRroCLwyjAnVcpoFHuUOz=214339034;if (vjOFaCKyHhRroCLwyjAnVcpoFHuUOz == vjOFaCKyHhRroCLwyjAnVcpoFHuUOz- 1 ) vjOFaCKyHhRroCLwyjAnVcpoFHuUOz=222976227; else vjOFaCKyHhRroCLwyjAnVcpoFHuUOz=1935195107; }
 vjOFaCKyHhRroCLwyjAnVcpoFHuUOzy::vjOFaCKyHhRroCLwyjAnVcpoFHuUOzy()
 { this->slTwFLlUQcYb("JjNgENBSomzSuzCFHRrOfPdYStaATHslTwFLlUQcYbj", true, 434050402, 1722325921, 1883654705); }
#pragma optimize("", off)
 // <delete/>


// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class TbhDBRvzKeZrpzwuyXiKgtScEIiMHyy
 { 
public: bool nacSGuUfwaMGjvlqjYUoXtyyaIEzti; double nacSGuUfwaMGjvlqjYUoXtyyaIEztiTbhDBRvzKeZrpzwuyXiKgtScEIiMHy; TbhDBRvzKeZrpzwuyXiKgtScEIiMHyy(); void TutyjzdEUujR(string nacSGuUfwaMGjvlqjYUoXtyyaIEztiTutyjzdEUujR, bool jrMRlRNgqAdrmjTmjhTRznmJpubdsy, int pVmNqUuRJTdChUKfGOenYXgbiwFCcr, float pjosKIFYtyrSXznxGRzKnMzEZRpKfu, long PspSetOpJBaiVfvFAgnlpnCRDckjSs);
 protected: bool nacSGuUfwaMGjvlqjYUoXtyyaIEztio; double nacSGuUfwaMGjvlqjYUoXtyyaIEztiTbhDBRvzKeZrpzwuyXiKgtScEIiMHyf; void TutyjzdEUujRu(string nacSGuUfwaMGjvlqjYUoXtyyaIEztiTutyjzdEUujRg, bool jrMRlRNgqAdrmjTmjhTRznmJpubdsye, int pVmNqUuRJTdChUKfGOenYXgbiwFCcrr, float pjosKIFYtyrSXznxGRzKnMzEZRpKfuw, long PspSetOpJBaiVfvFAgnlpnCRDckjSsn);
 private: bool nacSGuUfwaMGjvlqjYUoXtyyaIEztijrMRlRNgqAdrmjTmjhTRznmJpubdsy; double nacSGuUfwaMGjvlqjYUoXtyyaIEztipjosKIFYtyrSXznxGRzKnMzEZRpKfuTbhDBRvzKeZrpzwuyXiKgtScEIiMHy;
 void TutyjzdEUujRv(string jrMRlRNgqAdrmjTmjhTRznmJpubdsyTutyjzdEUujR, bool jrMRlRNgqAdrmjTmjhTRznmJpubdsypVmNqUuRJTdChUKfGOenYXgbiwFCcr, int pVmNqUuRJTdChUKfGOenYXgbiwFCcrnacSGuUfwaMGjvlqjYUoXtyyaIEzti, float pjosKIFYtyrSXznxGRzKnMzEZRpKfuPspSetOpJBaiVfvFAgnlpnCRDckjSs, long PspSetOpJBaiVfvFAgnlpnCRDckjSsjrMRlRNgqAdrmjTmjhTRznmJpubdsy); };
 void TbhDBRvzKeZrpzwuyXiKgtScEIiMHyy::TutyjzdEUujR(string nacSGuUfwaMGjvlqjYUoXtyyaIEztiTutyjzdEUujR, bool jrMRlRNgqAdrmjTmjhTRznmJpubdsy, int pVmNqUuRJTdChUKfGOenYXgbiwFCcr, float pjosKIFYtyrSXznxGRzKnMzEZRpKfu, long PspSetOpJBaiVfvFAgnlpnCRDckjSs)
 { int FHtbIkHBvOiwqwYwWiicUpyAjUJJTV=1548208466;if (FHtbIkHBvOiwqwYwWiicUpyAjUJJTV == FHtbIkHBvOiwqwYwWiicUpyAjUJJTV- 1 ) FHtbIkHBvOiwqwYwWiicUpyAjUJJTV=815706619; else FHtbIkHBvOiwqwYwWiicUpyAjUJJTV=1934655260;if (FHtbIkHBvOiwqwYwWiicUpyAjUJJTV == FHtbIkHBvOiwqwYwWiicUpyAjUJJTV- 0 ) FHtbIkHBvOiwqwYwWiicUpyAjUJJTV=645920222; else FHtbIkHBvOiwqwYwWiicUpyAjUJJTV=664299320;if (FHtbIkHBvOiwqwYwWiicUpyAjUJJTV == FHtbIkHBvOiwqwYwWiicUpyAjUJJTV- 0 ) FHtbIkHBvOiwqwYwWiicUpyAjUJJTV=510719631; else FHtbIkHBvOiwqwYwWiicUpyAjUJJTV=110535348;if (FHtbIkHBvOiwqwYwWiicUpyAjUJJTV == FHtbIkHBvOiwqwYwWiicUpyAjUJJTV- 1 ) FHtbIkHBvOiwqwYwWiicUpyAjUJJTV=1334244321; else FHtbIkHBvOiwqwYwWiicUpyAjUJJTV=2065602259;if (FHtbIkHBvOiwqwYwWiicUpyAjUJJTV == FHtbIkHBvOiwqwYwWiicUpyAjUJJTV- 0 ) FHtbIkHBvOiwqwYwWiicUpyAjUJJTV=324991647; else FHtbIkHBvOiwqwYwWiicUpyAjUJJTV=1469402226;if (FHtbIkHBvOiwqwYwWiicUpyAjUJJTV == FHtbIkHBvOiwqwYwWiicUpyAjUJJTV- 0 ) FHtbIkHBvOiwqwYwWiicUpyAjUJJTV=255442637; else FHtbIkHBvOiwqwYwWiicUpyAjUJJTV=740007116;int OABXsVtjKbnvPkIDurDUadDwRmUpmK=310430595;if (OABXsVtjKbnvPkIDurDUadDwRmUpmK == OABXsVtjKbnvPkIDurDUadDwRmUpmK- 1 ) OABXsVtjKbnvPkIDurDUadDwRmUpmK=981640932; else OABXsVtjKbnvPkIDurDUadDwRmUpmK=1822529296;if (OABXsVtjKbnvPkIDurDUadDwRmUpmK == OABXsVtjKbnvPkIDurDUadDwRmUpmK- 0 ) OABXsVtjKbnvPkIDurDUadDwRmUpmK=1849634963; else OABXsVtjKbnvPkIDurDUadDwRmUpmK=1018819703;if (OABXsVtjKbnvPkIDurDUadDwRmUpmK == OABXsVtjKbnvPkIDurDUadDwRmUpmK- 0 ) OABXsVtjKbnvPkIDurDUadDwRmUpmK=269644599; else OABXsVtjKbnvPkIDurDUadDwRmUpmK=1665383018;if (OABXsVtjKbnvPkIDurDUadDwRmUpmK == OABXsVtjKbnvPkIDurDUadDwRmUpmK- 0 ) OABXsVtjKbnvPkIDurDUadDwRmUpmK=1149845102; else OABXsVtjKbnvPkIDurDUadDwRmUpmK=1482172768;if (OABXsVtjKbnvPkIDurDUadDwRmUpmK == OABXsVtjKbnvPkIDurDUadDwRmUpmK- 0 ) OABXsVtjKbnvPkIDurDUadDwRmUpmK=2068104067; else OABXsVtjKbnvPkIDurDUadDwRmUpmK=1708712334;if (OABXsVtjKbnvPkIDurDUadDwRmUpmK == OABXsVtjKbnvPkIDurDUadDwRmUpmK- 1 ) OABXsVtjKbnvPkIDurDUadDwRmUpmK=1370673820; else OABXsVtjKbnvPkIDurDUadDwRmUpmK=1697043302;double oxydgZFmKCisxryCStnACXwVjqUfvq=1170440172.042377369429842009007018785475;if (oxydgZFmKCisxryCStnACXwVjqUfvq == oxydgZFmKCisxryCStnACXwVjqUfvq ) oxydgZFmKCisxryCStnACXwVjqUfvq=1839241485.443738106660480295250534914469; else oxydgZFmKCisxryCStnACXwVjqUfvq=978518312.209409711590345773869133502993;if (oxydgZFmKCisxryCStnACXwVjqUfvq == oxydgZFmKCisxryCStnACXwVjqUfvq ) oxydgZFmKCisxryCStnACXwVjqUfvq=1133331483.585757208268486129035161027791; else oxydgZFmKCisxryCStnACXwVjqUfvq=487006372.883902670957571285758407842573;if (oxydgZFmKCisxryCStnACXwVjqUfvq == oxydgZFmKCisxryCStnACXwVjqUfvq ) oxydgZFmKCisxryCStnACXwVjqUfvq=1308671554.061858658466928970195305358617; else oxydgZFmKCisxryCStnACXwVjqUfvq=918121171.120890682363839359008249987651;if (oxydgZFmKCisxryCStnACXwVjqUfvq == oxydgZFmKCisxryCStnACXwVjqUfvq ) oxydgZFmKCisxryCStnACXwVjqUfvq=369848106.115145623939983257710071008292; else oxydgZFmKCisxryCStnACXwVjqUfvq=1220465942.403226970574618947683005214424;if (oxydgZFmKCisxryCStnACXwVjqUfvq == oxydgZFmKCisxryCStnACXwVjqUfvq ) oxydgZFmKCisxryCStnACXwVjqUfvq=1851388766.853163676849734646416272041902; else oxydgZFmKCisxryCStnACXwVjqUfvq=1131557638.355773156537194438267812878222;if (oxydgZFmKCisxryCStnACXwVjqUfvq == oxydgZFmKCisxryCStnACXwVjqUfvq ) oxydgZFmKCisxryCStnACXwVjqUfvq=98747124.388550416150198019861385554491; else oxydgZFmKCisxryCStnACXwVjqUfvq=688878921.604536064513221883496056201411;long AMdZefgfMAdaQdqEeRkkIAxXfZmPMY=439603319;if (AMdZefgfMAdaQdqEeRkkIAxXfZmPMY == AMdZefgfMAdaQdqEeRkkIAxXfZmPMY- 0 ) AMdZefgfMAdaQdqEeRkkIAxXfZmPMY=1862057926; else AMdZefgfMAdaQdqEeRkkIAxXfZmPMY=817242838;if (AMdZefgfMAdaQdqEeRkkIAxXfZmPMY == AMdZefgfMAdaQdqEeRkkIAxXfZmPMY- 0 ) AMdZefgfMAdaQdqEeRkkIAxXfZmPMY=1134613644; else AMdZefgfMAdaQdqEeRkkIAxXfZmPMY=1287490563;if (AMdZefgfMAdaQdqEeRkkIAxXfZmPMY == AMdZefgfMAdaQdqEeRkkIAxXfZmPMY- 1 ) AMdZefgfMAdaQdqEeRkkIAxXfZmPMY=1990898199; else AMdZefgfMAdaQdqEeRkkIAxXfZmPMY=1594420614;if (AMdZefgfMAdaQdqEeRkkIAxXfZmPMY == AMdZefgfMAdaQdqEeRkkIAxXfZmPMY- 0 ) AMdZefgfMAdaQdqEeRkkIAxXfZmPMY=1278563867; else AMdZefgfMAdaQdqEeRkkIAxXfZmPMY=1297807248;if (AMdZefgfMAdaQdqEeRkkIAxXfZmPMY == AMdZefgfMAdaQdqEeRkkIAxXfZmPMY- 0 ) AMdZefgfMAdaQdqEeRkkIAxXfZmPMY=784810158; else AMdZefgfMAdaQdqEeRkkIAxXfZmPMY=1311231101;if (AMdZefgfMAdaQdqEeRkkIAxXfZmPMY == AMdZefgfMAdaQdqEeRkkIAxXfZmPMY- 0 ) AMdZefgfMAdaQdqEeRkkIAxXfZmPMY=743678900; else AMdZefgfMAdaQdqEeRkkIAxXfZmPMY=949490944;int DmTmOpCRUmrbMnVKvKnaiYkeBWYpFW=1300733721;if (DmTmOpCRUmrbMnVKvKnaiYkeBWYpFW == DmTmOpCRUmrbMnVKvKnaiYkeBWYpFW- 1 ) DmTmOpCRUmrbMnVKvKnaiYkeBWYpFW=810136034; else DmTmOpCRUmrbMnVKvKnaiYkeBWYpFW=1232486112;if (DmTmOpCRUmrbMnVKvKnaiYkeBWYpFW == DmTmOpCRUmrbMnVKvKnaiYkeBWYpFW- 0 ) DmTmOpCRUmrbMnVKvKnaiYkeBWYpFW=1048249278; else DmTmOpCRUmrbMnVKvKnaiYkeBWYpFW=1018714310;if (DmTmOpCRUmrbMnVKvKnaiYkeBWYpFW == DmTmOpCRUmrbMnVKvKnaiYkeBWYpFW- 0 ) DmTmOpCRUmrbMnVKvKnaiYkeBWYpFW=1810685374; else DmTmOpCRUmrbMnVKvKnaiYkeBWYpFW=1424848915;if (DmTmOpCRUmrbMnVKvKnaiYkeBWYpFW == DmTmOpCRUmrbMnVKvKnaiYkeBWYpFW- 0 ) DmTmOpCRUmrbMnVKvKnaiYkeBWYpFW=1149242688; else DmTmOpCRUmrbMnVKvKnaiYkeBWYpFW=63519761;if (DmTmOpCRUmrbMnVKvKnaiYkeBWYpFW == DmTmOpCRUmrbMnVKvKnaiYkeBWYpFW- 0 ) DmTmOpCRUmrbMnVKvKnaiYkeBWYpFW=1452681190; else DmTmOpCRUmrbMnVKvKnaiYkeBWYpFW=629169021;if (DmTmOpCRUmrbMnVKvKnaiYkeBWYpFW == DmTmOpCRUmrbMnVKvKnaiYkeBWYpFW- 0 ) DmTmOpCRUmrbMnVKvKnaiYkeBWYpFW=1314914584; else DmTmOpCRUmrbMnVKvKnaiYkeBWYpFW=1285570608;double zwhuGrrMAWGyiaxwfTsYgjNQjleqqj=309593987.270283816652139021423111894278;if (zwhuGrrMAWGyiaxwfTsYgjNQjleqqj == zwhuGrrMAWGyiaxwfTsYgjNQjleqqj ) zwhuGrrMAWGyiaxwfTsYgjNQjleqqj=1628911916.532812801482855479459119627950; else zwhuGrrMAWGyiaxwfTsYgjNQjleqqj=931070501.075813032722104237742582052430;if (zwhuGrrMAWGyiaxwfTsYgjNQjleqqj == zwhuGrrMAWGyiaxwfTsYgjNQjleqqj ) zwhuGrrMAWGyiaxwfTsYgjNQjleqqj=1982469023.505252714255774711559122850497; else zwhuGrrMAWGyiaxwfTsYgjNQjleqqj=1421939235.171222896353350882935198259440;if (zwhuGrrMAWGyiaxwfTsYgjNQjleqqj == zwhuGrrMAWGyiaxwfTsYgjNQjleqqj ) zwhuGrrMAWGyiaxwfTsYgjNQjleqqj=37127929.342818455202620687317856209318; else zwhuGrrMAWGyiaxwfTsYgjNQjleqqj=1519259276.446612164244783367967214459662;if (zwhuGrrMAWGyiaxwfTsYgjNQjleqqj == zwhuGrrMAWGyiaxwfTsYgjNQjleqqj ) zwhuGrrMAWGyiaxwfTsYgjNQjleqqj=1327025620.915887712853417428495345234227; else zwhuGrrMAWGyiaxwfTsYgjNQjleqqj=244820871.350334901775621591713238778075;if (zwhuGrrMAWGyiaxwfTsYgjNQjleqqj == zwhuGrrMAWGyiaxwfTsYgjNQjleqqj ) zwhuGrrMAWGyiaxwfTsYgjNQjleqqj=1847438092.359550318453873083575462423377; else zwhuGrrMAWGyiaxwfTsYgjNQjleqqj=1983007882.603642717128346146570560100713;if (zwhuGrrMAWGyiaxwfTsYgjNQjleqqj == zwhuGrrMAWGyiaxwfTsYgjNQjleqqj ) zwhuGrrMAWGyiaxwfTsYgjNQjleqqj=1849679861.814286034147646712749881724464; else zwhuGrrMAWGyiaxwfTsYgjNQjleqqj=2144924100.310442416012354393449808513457;float fiwqPmJxXQCejOiYPlVLGzLZVDJwRs=66704100.475185715644845469622864527617f;if (fiwqPmJxXQCejOiYPlVLGzLZVDJwRs - fiwqPmJxXQCejOiYPlVLGzLZVDJwRs> 0.00000001 ) fiwqPmJxXQCejOiYPlVLGzLZVDJwRs=1305137452.395626189303993601944582062166f; else fiwqPmJxXQCejOiYPlVLGzLZVDJwRs=945497050.921936932949033907103687391476f;if (fiwqPmJxXQCejOiYPlVLGzLZVDJwRs - fiwqPmJxXQCejOiYPlVLGzLZVDJwRs> 0.00000001 ) fiwqPmJxXQCejOiYPlVLGzLZVDJwRs=2026226534.402349206877901655017517434575f; else fiwqPmJxXQCejOiYPlVLGzLZVDJwRs=1487318702.557754770480233688674757000602f;if (fiwqPmJxXQCejOiYPlVLGzLZVDJwRs - fiwqPmJxXQCejOiYPlVLGzLZVDJwRs> 0.00000001 ) fiwqPmJxXQCejOiYPlVLGzLZVDJwRs=710260802.584242135708663738486790181176f; else fiwqPmJxXQCejOiYPlVLGzLZVDJwRs=1208569755.517800710165498629437223799417f;if (fiwqPmJxXQCejOiYPlVLGzLZVDJwRs - fiwqPmJxXQCejOiYPlVLGzLZVDJwRs> 0.00000001 ) fiwqPmJxXQCejOiYPlVLGzLZVDJwRs=776401592.106929186403833131267284940522f; else fiwqPmJxXQCejOiYPlVLGzLZVDJwRs=437209011.231406957054413449166935103209f;if (fiwqPmJxXQCejOiYPlVLGzLZVDJwRs - fiwqPmJxXQCejOiYPlVLGzLZVDJwRs> 0.00000001 ) fiwqPmJxXQCejOiYPlVLGzLZVDJwRs=1815483797.077528353728431398160156631641f; else fiwqPmJxXQCejOiYPlVLGzLZVDJwRs=1071093883.375919899629644860581625092190f;if (fiwqPmJxXQCejOiYPlVLGzLZVDJwRs - fiwqPmJxXQCejOiYPlVLGzLZVDJwRs> 0.00000001 ) fiwqPmJxXQCejOiYPlVLGzLZVDJwRs=1982293020.772356353260964151288126076002f; else fiwqPmJxXQCejOiYPlVLGzLZVDJwRs=1361729178.771095840951833276295191276738f;float PbHctwzivQCJROjTsPtBAsDidnwmNs=323843977.956180655850349962704016199252f;if (PbHctwzivQCJROjTsPtBAsDidnwmNs - PbHctwzivQCJROjTsPtBAsDidnwmNs> 0.00000001 ) PbHctwzivQCJROjTsPtBAsDidnwmNs=401379500.473590596395438231458272304109f; else PbHctwzivQCJROjTsPtBAsDidnwmNs=434084913.241449786047641979806990430230f;if (PbHctwzivQCJROjTsPtBAsDidnwmNs - PbHctwzivQCJROjTsPtBAsDidnwmNs> 0.00000001 ) PbHctwzivQCJROjTsPtBAsDidnwmNs=1468062675.750825509955652163244631446202f; else PbHctwzivQCJROjTsPtBAsDidnwmNs=2103758321.066639893657095134847709722959f;if (PbHctwzivQCJROjTsPtBAsDidnwmNs - PbHctwzivQCJROjTsPtBAsDidnwmNs> 0.00000001 ) PbHctwzivQCJROjTsPtBAsDidnwmNs=1949699290.041800902338170765731246537892f; else PbHctwzivQCJROjTsPtBAsDidnwmNs=551964598.895572620633361974006832880548f;if (PbHctwzivQCJROjTsPtBAsDidnwmNs - PbHctwzivQCJROjTsPtBAsDidnwmNs> 0.00000001 ) PbHctwzivQCJROjTsPtBAsDidnwmNs=1263605792.422064470825521809567399707379f; else PbHctwzivQCJROjTsPtBAsDidnwmNs=1438360405.218321240860429232219219114346f;if (PbHctwzivQCJROjTsPtBAsDidnwmNs - PbHctwzivQCJROjTsPtBAsDidnwmNs> 0.00000001 ) PbHctwzivQCJROjTsPtBAsDidnwmNs=2052944139.820161730077981798168517111022f; else PbHctwzivQCJROjTsPtBAsDidnwmNs=803428407.890504136851335333952739931416f;if (PbHctwzivQCJROjTsPtBAsDidnwmNs - PbHctwzivQCJROjTsPtBAsDidnwmNs> 0.00000001 ) PbHctwzivQCJROjTsPtBAsDidnwmNs=1318759865.172838749777231839398928027277f; else PbHctwzivQCJROjTsPtBAsDidnwmNs=1975161139.373189051860529651821322835481f;long YlHRyQEWLpoqaOsTztWiiTbhBYHQeR=2032324028;if (YlHRyQEWLpoqaOsTztWiiTbhBYHQeR == YlHRyQEWLpoqaOsTztWiiTbhBYHQeR- 1 ) YlHRyQEWLpoqaOsTztWiiTbhBYHQeR=296048254; else YlHRyQEWLpoqaOsTztWiiTbhBYHQeR=1248210484;if (YlHRyQEWLpoqaOsTztWiiTbhBYHQeR == YlHRyQEWLpoqaOsTztWiiTbhBYHQeR- 1 ) YlHRyQEWLpoqaOsTztWiiTbhBYHQeR=2127916803; else YlHRyQEWLpoqaOsTztWiiTbhBYHQeR=621138844;if (YlHRyQEWLpoqaOsTztWiiTbhBYHQeR == YlHRyQEWLpoqaOsTztWiiTbhBYHQeR- 1 ) YlHRyQEWLpoqaOsTztWiiTbhBYHQeR=582223294; else YlHRyQEWLpoqaOsTztWiiTbhBYHQeR=1570016546;if (YlHRyQEWLpoqaOsTztWiiTbhBYHQeR == YlHRyQEWLpoqaOsTztWiiTbhBYHQeR- 0 ) YlHRyQEWLpoqaOsTztWiiTbhBYHQeR=1746816832; else YlHRyQEWLpoqaOsTztWiiTbhBYHQeR=420342161;if (YlHRyQEWLpoqaOsTztWiiTbhBYHQeR == YlHRyQEWLpoqaOsTztWiiTbhBYHQeR- 0 ) YlHRyQEWLpoqaOsTztWiiTbhBYHQeR=154668909; else YlHRyQEWLpoqaOsTztWiiTbhBYHQeR=1545260012;if (YlHRyQEWLpoqaOsTztWiiTbhBYHQeR == YlHRyQEWLpoqaOsTztWiiTbhBYHQeR- 0 ) YlHRyQEWLpoqaOsTztWiiTbhBYHQeR=1753939085; else YlHRyQEWLpoqaOsTztWiiTbhBYHQeR=1113517693;long LtFAoEcADjKiGBmBnDvEjYtquTeUyy=1854444833;if (LtFAoEcADjKiGBmBnDvEjYtquTeUyy == LtFAoEcADjKiGBmBnDvEjYtquTeUyy- 1 ) LtFAoEcADjKiGBmBnDvEjYtquTeUyy=104352881; else LtFAoEcADjKiGBmBnDvEjYtquTeUyy=1525835884;if (LtFAoEcADjKiGBmBnDvEjYtquTeUyy == LtFAoEcADjKiGBmBnDvEjYtquTeUyy- 0 ) LtFAoEcADjKiGBmBnDvEjYtquTeUyy=1743544600; else LtFAoEcADjKiGBmBnDvEjYtquTeUyy=1887841005;if (LtFAoEcADjKiGBmBnDvEjYtquTeUyy == LtFAoEcADjKiGBmBnDvEjYtquTeUyy- 0 ) LtFAoEcADjKiGBmBnDvEjYtquTeUyy=99778134; else LtFAoEcADjKiGBmBnDvEjYtquTeUyy=376846093;if (LtFAoEcADjKiGBmBnDvEjYtquTeUyy == LtFAoEcADjKiGBmBnDvEjYtquTeUyy- 0 ) LtFAoEcADjKiGBmBnDvEjYtquTeUyy=264488457; else LtFAoEcADjKiGBmBnDvEjYtquTeUyy=753172854;if (LtFAoEcADjKiGBmBnDvEjYtquTeUyy == LtFAoEcADjKiGBmBnDvEjYtquTeUyy- 1 ) LtFAoEcADjKiGBmBnDvEjYtquTeUyy=1829374905; else LtFAoEcADjKiGBmBnDvEjYtquTeUyy=587866129;if (LtFAoEcADjKiGBmBnDvEjYtquTeUyy == LtFAoEcADjKiGBmBnDvEjYtquTeUyy- 0 ) LtFAoEcADjKiGBmBnDvEjYtquTeUyy=1581858210; else LtFAoEcADjKiGBmBnDvEjYtquTeUyy=2054316042;double aoEsIMNgTufYuzUEsnDuaymkZxUdEs=663681562.316918905505613512073290566463;if (aoEsIMNgTufYuzUEsnDuaymkZxUdEs == aoEsIMNgTufYuzUEsnDuaymkZxUdEs ) aoEsIMNgTufYuzUEsnDuaymkZxUdEs=1182016393.231907927166721231392400822646; else aoEsIMNgTufYuzUEsnDuaymkZxUdEs=2097452639.364337572660591858044517284129;if (aoEsIMNgTufYuzUEsnDuaymkZxUdEs == aoEsIMNgTufYuzUEsnDuaymkZxUdEs ) aoEsIMNgTufYuzUEsnDuaymkZxUdEs=1065680924.538291259486074302007571285816; else aoEsIMNgTufYuzUEsnDuaymkZxUdEs=1178779193.346413241061966854409312207711;if (aoEsIMNgTufYuzUEsnDuaymkZxUdEs == aoEsIMNgTufYuzUEsnDuaymkZxUdEs ) aoEsIMNgTufYuzUEsnDuaymkZxUdEs=12560103.654324177116214671643242814521; else aoEsIMNgTufYuzUEsnDuaymkZxUdEs=1257516157.764042765289849321874627852261;if (aoEsIMNgTufYuzUEsnDuaymkZxUdEs == aoEsIMNgTufYuzUEsnDuaymkZxUdEs ) aoEsIMNgTufYuzUEsnDuaymkZxUdEs=1889104330.176645327391746540360375825729; else aoEsIMNgTufYuzUEsnDuaymkZxUdEs=978846601.161422421500458408142201280193;if (aoEsIMNgTufYuzUEsnDuaymkZxUdEs == aoEsIMNgTufYuzUEsnDuaymkZxUdEs ) aoEsIMNgTufYuzUEsnDuaymkZxUdEs=834751728.170934033522096002742769896135; else aoEsIMNgTufYuzUEsnDuaymkZxUdEs=1047720514.234844063547672031529855141214;if (aoEsIMNgTufYuzUEsnDuaymkZxUdEs == aoEsIMNgTufYuzUEsnDuaymkZxUdEs ) aoEsIMNgTufYuzUEsnDuaymkZxUdEs=1949089412.801930935205931520797661759970; else aoEsIMNgTufYuzUEsnDuaymkZxUdEs=404439278.495074784310642492563253734039;long zcapFAvbhqWbVojyKtMbhWDnMVJQYA=2087647697;if (zcapFAvbhqWbVojyKtMbhWDnMVJQYA == zcapFAvbhqWbVojyKtMbhWDnMVJQYA- 0 ) zcapFAvbhqWbVojyKtMbhWDnMVJQYA=838113566; else zcapFAvbhqWbVojyKtMbhWDnMVJQYA=1611798679;if (zcapFAvbhqWbVojyKtMbhWDnMVJQYA == zcapFAvbhqWbVojyKtMbhWDnMVJQYA- 0 ) zcapFAvbhqWbVojyKtMbhWDnMVJQYA=1870808258; else zcapFAvbhqWbVojyKtMbhWDnMVJQYA=196816817;if (zcapFAvbhqWbVojyKtMbhWDnMVJQYA == zcapFAvbhqWbVojyKtMbhWDnMVJQYA- 1 ) zcapFAvbhqWbVojyKtMbhWDnMVJQYA=1555741282; else zcapFAvbhqWbVojyKtMbhWDnMVJQYA=148720132;if (zcapFAvbhqWbVojyKtMbhWDnMVJQYA == zcapFAvbhqWbVojyKtMbhWDnMVJQYA- 1 ) zcapFAvbhqWbVojyKtMbhWDnMVJQYA=1932545894; else zcapFAvbhqWbVojyKtMbhWDnMVJQYA=2066685808;if (zcapFAvbhqWbVojyKtMbhWDnMVJQYA == zcapFAvbhqWbVojyKtMbhWDnMVJQYA- 1 ) zcapFAvbhqWbVojyKtMbhWDnMVJQYA=983722027; else zcapFAvbhqWbVojyKtMbhWDnMVJQYA=1374743949;if (zcapFAvbhqWbVojyKtMbhWDnMVJQYA == zcapFAvbhqWbVojyKtMbhWDnMVJQYA- 0 ) zcapFAvbhqWbVojyKtMbhWDnMVJQYA=939247586; else zcapFAvbhqWbVojyKtMbhWDnMVJQYA=2141840812;float LmNQOgrnUhkLzsbTapeRFLLjdkPjIP=1353428456.928777543112847457818444545862f;if (LmNQOgrnUhkLzsbTapeRFLLjdkPjIP - LmNQOgrnUhkLzsbTapeRFLLjdkPjIP> 0.00000001 ) LmNQOgrnUhkLzsbTapeRFLLjdkPjIP=683220055.011732037622803451046503359523f; else LmNQOgrnUhkLzsbTapeRFLLjdkPjIP=1090257523.289221211542120664220582762859f;if (LmNQOgrnUhkLzsbTapeRFLLjdkPjIP - LmNQOgrnUhkLzsbTapeRFLLjdkPjIP> 0.00000001 ) LmNQOgrnUhkLzsbTapeRFLLjdkPjIP=2078984947.362455379335151549489541470856f; else LmNQOgrnUhkLzsbTapeRFLLjdkPjIP=1734480911.047229087540509554133358083108f;if (LmNQOgrnUhkLzsbTapeRFLLjdkPjIP - LmNQOgrnUhkLzsbTapeRFLLjdkPjIP> 0.00000001 ) LmNQOgrnUhkLzsbTapeRFLLjdkPjIP=1047099165.652430893256294015292673470024f; else LmNQOgrnUhkLzsbTapeRFLLjdkPjIP=934019295.708909827087209726418802609007f;if (LmNQOgrnUhkLzsbTapeRFLLjdkPjIP - LmNQOgrnUhkLzsbTapeRFLLjdkPjIP> 0.00000001 ) LmNQOgrnUhkLzsbTapeRFLLjdkPjIP=1841884730.087966930629821658585472790730f; else LmNQOgrnUhkLzsbTapeRFLLjdkPjIP=994320371.192011045489789019106153271637f;if (LmNQOgrnUhkLzsbTapeRFLLjdkPjIP - LmNQOgrnUhkLzsbTapeRFLLjdkPjIP> 0.00000001 ) LmNQOgrnUhkLzsbTapeRFLLjdkPjIP=1784215201.630063792172420780389263505994f; else LmNQOgrnUhkLzsbTapeRFLLjdkPjIP=764697999.041886972732714584469245957767f;if (LmNQOgrnUhkLzsbTapeRFLLjdkPjIP - LmNQOgrnUhkLzsbTapeRFLLjdkPjIP> 0.00000001 ) LmNQOgrnUhkLzsbTapeRFLLjdkPjIP=1053089277.990813164481975756003164691704f; else LmNQOgrnUhkLzsbTapeRFLLjdkPjIP=1199541267.022685290152118838120186493733f;long VjyUexQsDXehvdGBYcmdRfmXtWeGwG=1679286806;if (VjyUexQsDXehvdGBYcmdRfmXtWeGwG == VjyUexQsDXehvdGBYcmdRfmXtWeGwG- 1 ) VjyUexQsDXehvdGBYcmdRfmXtWeGwG=833955627; else VjyUexQsDXehvdGBYcmdRfmXtWeGwG=1641694160;if (VjyUexQsDXehvdGBYcmdRfmXtWeGwG == VjyUexQsDXehvdGBYcmdRfmXtWeGwG- 0 ) VjyUexQsDXehvdGBYcmdRfmXtWeGwG=1216202476; else VjyUexQsDXehvdGBYcmdRfmXtWeGwG=425494858;if (VjyUexQsDXehvdGBYcmdRfmXtWeGwG == VjyUexQsDXehvdGBYcmdRfmXtWeGwG- 0 ) VjyUexQsDXehvdGBYcmdRfmXtWeGwG=1225399489; else VjyUexQsDXehvdGBYcmdRfmXtWeGwG=665223140;if (VjyUexQsDXehvdGBYcmdRfmXtWeGwG == VjyUexQsDXehvdGBYcmdRfmXtWeGwG- 1 ) VjyUexQsDXehvdGBYcmdRfmXtWeGwG=1255423927; else VjyUexQsDXehvdGBYcmdRfmXtWeGwG=1033296261;if (VjyUexQsDXehvdGBYcmdRfmXtWeGwG == VjyUexQsDXehvdGBYcmdRfmXtWeGwG- 1 ) VjyUexQsDXehvdGBYcmdRfmXtWeGwG=164906745; else VjyUexQsDXehvdGBYcmdRfmXtWeGwG=1146478763;if (VjyUexQsDXehvdGBYcmdRfmXtWeGwG == VjyUexQsDXehvdGBYcmdRfmXtWeGwG- 1 ) VjyUexQsDXehvdGBYcmdRfmXtWeGwG=195057166; else VjyUexQsDXehvdGBYcmdRfmXtWeGwG=785299801;long ubxoFShAIRgrkuCbWNoWDfXgSWOYLn=1142250938;if (ubxoFShAIRgrkuCbWNoWDfXgSWOYLn == ubxoFShAIRgrkuCbWNoWDfXgSWOYLn- 0 ) ubxoFShAIRgrkuCbWNoWDfXgSWOYLn=517188243; else ubxoFShAIRgrkuCbWNoWDfXgSWOYLn=595660956;if (ubxoFShAIRgrkuCbWNoWDfXgSWOYLn == ubxoFShAIRgrkuCbWNoWDfXgSWOYLn- 1 ) ubxoFShAIRgrkuCbWNoWDfXgSWOYLn=1868702870; else ubxoFShAIRgrkuCbWNoWDfXgSWOYLn=2002735284;if (ubxoFShAIRgrkuCbWNoWDfXgSWOYLn == ubxoFShAIRgrkuCbWNoWDfXgSWOYLn- 1 ) ubxoFShAIRgrkuCbWNoWDfXgSWOYLn=218586799; else ubxoFShAIRgrkuCbWNoWDfXgSWOYLn=166913238;if (ubxoFShAIRgrkuCbWNoWDfXgSWOYLn == ubxoFShAIRgrkuCbWNoWDfXgSWOYLn- 0 ) ubxoFShAIRgrkuCbWNoWDfXgSWOYLn=1040548532; else ubxoFShAIRgrkuCbWNoWDfXgSWOYLn=2051577918;if (ubxoFShAIRgrkuCbWNoWDfXgSWOYLn == ubxoFShAIRgrkuCbWNoWDfXgSWOYLn- 0 ) ubxoFShAIRgrkuCbWNoWDfXgSWOYLn=1917397596; else ubxoFShAIRgrkuCbWNoWDfXgSWOYLn=876487887;if (ubxoFShAIRgrkuCbWNoWDfXgSWOYLn == ubxoFShAIRgrkuCbWNoWDfXgSWOYLn- 1 ) ubxoFShAIRgrkuCbWNoWDfXgSWOYLn=560085263; else ubxoFShAIRgrkuCbWNoWDfXgSWOYLn=791043283;int EqmHOOHAlRrWkgDcCEWvPrdVXFrvZq=1123672848;if (EqmHOOHAlRrWkgDcCEWvPrdVXFrvZq == EqmHOOHAlRrWkgDcCEWvPrdVXFrvZq- 0 ) EqmHOOHAlRrWkgDcCEWvPrdVXFrvZq=1567080255; else EqmHOOHAlRrWkgDcCEWvPrdVXFrvZq=1407444427;if (EqmHOOHAlRrWkgDcCEWvPrdVXFrvZq == EqmHOOHAlRrWkgDcCEWvPrdVXFrvZq- 0 ) EqmHOOHAlRrWkgDcCEWvPrdVXFrvZq=1307885185; else EqmHOOHAlRrWkgDcCEWvPrdVXFrvZq=493947823;if (EqmHOOHAlRrWkgDcCEWvPrdVXFrvZq == EqmHOOHAlRrWkgDcCEWvPrdVXFrvZq- 0 ) EqmHOOHAlRrWkgDcCEWvPrdVXFrvZq=623781961; else EqmHOOHAlRrWkgDcCEWvPrdVXFrvZq=105037413;if (EqmHOOHAlRrWkgDcCEWvPrdVXFrvZq == EqmHOOHAlRrWkgDcCEWvPrdVXFrvZq- 0 ) EqmHOOHAlRrWkgDcCEWvPrdVXFrvZq=128028967; else EqmHOOHAlRrWkgDcCEWvPrdVXFrvZq=17996915;if (EqmHOOHAlRrWkgDcCEWvPrdVXFrvZq == EqmHOOHAlRrWkgDcCEWvPrdVXFrvZq- 1 ) EqmHOOHAlRrWkgDcCEWvPrdVXFrvZq=1982317243; else EqmHOOHAlRrWkgDcCEWvPrdVXFrvZq=1045688686;if (EqmHOOHAlRrWkgDcCEWvPrdVXFrvZq == EqmHOOHAlRrWkgDcCEWvPrdVXFrvZq- 1 ) EqmHOOHAlRrWkgDcCEWvPrdVXFrvZq=1569574166; else EqmHOOHAlRrWkgDcCEWvPrdVXFrvZq=2048104049;float toyHIrFMJLMAeHzrMlKUkhNSIVrWkf=2036951683.372764322189283346273237766233f;if (toyHIrFMJLMAeHzrMlKUkhNSIVrWkf - toyHIrFMJLMAeHzrMlKUkhNSIVrWkf> 0.00000001 ) toyHIrFMJLMAeHzrMlKUkhNSIVrWkf=1921493381.996682539862750050431264025510f; else toyHIrFMJLMAeHzrMlKUkhNSIVrWkf=2058321986.929939371192841168872781284267f;if (toyHIrFMJLMAeHzrMlKUkhNSIVrWkf - toyHIrFMJLMAeHzrMlKUkhNSIVrWkf> 0.00000001 ) toyHIrFMJLMAeHzrMlKUkhNSIVrWkf=682353024.803005483188044053407774834876f; else toyHIrFMJLMAeHzrMlKUkhNSIVrWkf=2127063019.612539985688670485723297024928f;if (toyHIrFMJLMAeHzrMlKUkhNSIVrWkf - toyHIrFMJLMAeHzrMlKUkhNSIVrWkf> 0.00000001 ) toyHIrFMJLMAeHzrMlKUkhNSIVrWkf=138829956.261867752627882921546015353552f; else toyHIrFMJLMAeHzrMlKUkhNSIVrWkf=2099895704.616853792483321642521349225652f;if (toyHIrFMJLMAeHzrMlKUkhNSIVrWkf - toyHIrFMJLMAeHzrMlKUkhNSIVrWkf> 0.00000001 ) toyHIrFMJLMAeHzrMlKUkhNSIVrWkf=1460700007.421001213023027370471104237863f; else toyHIrFMJLMAeHzrMlKUkhNSIVrWkf=667042389.814006232153960842396638133898f;if (toyHIrFMJLMAeHzrMlKUkhNSIVrWkf - toyHIrFMJLMAeHzrMlKUkhNSIVrWkf> 0.00000001 ) toyHIrFMJLMAeHzrMlKUkhNSIVrWkf=601145628.556021903124843935449567839797f; else toyHIrFMJLMAeHzrMlKUkhNSIVrWkf=1312108205.455561795095028289531955909030f;if (toyHIrFMJLMAeHzrMlKUkhNSIVrWkf - toyHIrFMJLMAeHzrMlKUkhNSIVrWkf> 0.00000001 ) toyHIrFMJLMAeHzrMlKUkhNSIVrWkf=655580909.232687038324056564965127661067f; else toyHIrFMJLMAeHzrMlKUkhNSIVrWkf=348911602.287687444236131891045527521513f;double wjHDXiZjuHHAkPwAteolBBKJigxBfb=1579385765.615207932053154358087655307011;if (wjHDXiZjuHHAkPwAteolBBKJigxBfb == wjHDXiZjuHHAkPwAteolBBKJigxBfb ) wjHDXiZjuHHAkPwAteolBBKJigxBfb=1802516930.588473917538347964423406907000; else wjHDXiZjuHHAkPwAteolBBKJigxBfb=1218867965.193158579154256041955479928367;if (wjHDXiZjuHHAkPwAteolBBKJigxBfb == wjHDXiZjuHHAkPwAteolBBKJigxBfb ) wjHDXiZjuHHAkPwAteolBBKJigxBfb=1365703193.792854529049538935096626573424; else wjHDXiZjuHHAkPwAteolBBKJigxBfb=1058307791.032257241352301794219437218738;if (wjHDXiZjuHHAkPwAteolBBKJigxBfb == wjHDXiZjuHHAkPwAteolBBKJigxBfb ) wjHDXiZjuHHAkPwAteolBBKJigxBfb=586861980.211502748279585432599932670514; else wjHDXiZjuHHAkPwAteolBBKJigxBfb=496740442.962324281005614096388624039004;if (wjHDXiZjuHHAkPwAteolBBKJigxBfb == wjHDXiZjuHHAkPwAteolBBKJigxBfb ) wjHDXiZjuHHAkPwAteolBBKJigxBfb=518468977.234532727136269433868433735849; else wjHDXiZjuHHAkPwAteolBBKJigxBfb=412150830.628788228338025356661635680190;if (wjHDXiZjuHHAkPwAteolBBKJigxBfb == wjHDXiZjuHHAkPwAteolBBKJigxBfb ) wjHDXiZjuHHAkPwAteolBBKJigxBfb=467631989.898852858231563051949939910420; else wjHDXiZjuHHAkPwAteolBBKJigxBfb=1850705955.814964037351953027364035197095;if (wjHDXiZjuHHAkPwAteolBBKJigxBfb == wjHDXiZjuHHAkPwAteolBBKJigxBfb ) wjHDXiZjuHHAkPwAteolBBKJigxBfb=20418041.354843771913460909600326798087; else wjHDXiZjuHHAkPwAteolBBKJigxBfb=1320381760.814310711135746267639850604826;double lsWGzuJcwGwpXgVinAcQGWZlxdRgCf=860812667.240621974993706001267400054429;if (lsWGzuJcwGwpXgVinAcQGWZlxdRgCf == lsWGzuJcwGwpXgVinAcQGWZlxdRgCf ) lsWGzuJcwGwpXgVinAcQGWZlxdRgCf=200436087.787981297526282496065467684077; else lsWGzuJcwGwpXgVinAcQGWZlxdRgCf=670617227.856622683591485222866857442140;if (lsWGzuJcwGwpXgVinAcQGWZlxdRgCf == lsWGzuJcwGwpXgVinAcQGWZlxdRgCf ) lsWGzuJcwGwpXgVinAcQGWZlxdRgCf=1017033549.823177177191468409347231101929; else lsWGzuJcwGwpXgVinAcQGWZlxdRgCf=1332276163.165487941926688650004380470217;if (lsWGzuJcwGwpXgVinAcQGWZlxdRgCf == lsWGzuJcwGwpXgVinAcQGWZlxdRgCf ) lsWGzuJcwGwpXgVinAcQGWZlxdRgCf=50651603.625814708167853165466203688501; else lsWGzuJcwGwpXgVinAcQGWZlxdRgCf=1511866108.873431298595295764831595242323;if (lsWGzuJcwGwpXgVinAcQGWZlxdRgCf == lsWGzuJcwGwpXgVinAcQGWZlxdRgCf ) lsWGzuJcwGwpXgVinAcQGWZlxdRgCf=984842892.621839442308897111903588049939; else lsWGzuJcwGwpXgVinAcQGWZlxdRgCf=1614668198.907597547632253470823143125489;if (lsWGzuJcwGwpXgVinAcQGWZlxdRgCf == lsWGzuJcwGwpXgVinAcQGWZlxdRgCf ) lsWGzuJcwGwpXgVinAcQGWZlxdRgCf=2094228067.062341141068537233568911786558; else lsWGzuJcwGwpXgVinAcQGWZlxdRgCf=640842796.326867092344096672699292226176;if (lsWGzuJcwGwpXgVinAcQGWZlxdRgCf == lsWGzuJcwGwpXgVinAcQGWZlxdRgCf ) lsWGzuJcwGwpXgVinAcQGWZlxdRgCf=2040285842.679385277125233503512496913565; else lsWGzuJcwGwpXgVinAcQGWZlxdRgCf=1459157403.292677571250668517082467538457;float lEItMLgcezSJSeKqSpVCcasSrPcEWw=1517936599.128172080425503401401406165511f;if (lEItMLgcezSJSeKqSpVCcasSrPcEWw - lEItMLgcezSJSeKqSpVCcasSrPcEWw> 0.00000001 ) lEItMLgcezSJSeKqSpVCcasSrPcEWw=1255506927.995002477295849726954055883201f; else lEItMLgcezSJSeKqSpVCcasSrPcEWw=2137672048.767063060114521666745977894122f;if (lEItMLgcezSJSeKqSpVCcasSrPcEWw - lEItMLgcezSJSeKqSpVCcasSrPcEWw> 0.00000001 ) lEItMLgcezSJSeKqSpVCcasSrPcEWw=245587119.027654898786685053422766699137f; else lEItMLgcezSJSeKqSpVCcasSrPcEWw=1667577811.904919708788817949385280147207f;if (lEItMLgcezSJSeKqSpVCcasSrPcEWw - lEItMLgcezSJSeKqSpVCcasSrPcEWw> 0.00000001 ) lEItMLgcezSJSeKqSpVCcasSrPcEWw=1838365383.687288287646674496976095383498f; else lEItMLgcezSJSeKqSpVCcasSrPcEWw=1765227656.350221274076275971675144169200f;if (lEItMLgcezSJSeKqSpVCcasSrPcEWw - lEItMLgcezSJSeKqSpVCcasSrPcEWw> 0.00000001 ) lEItMLgcezSJSeKqSpVCcasSrPcEWw=1450089703.680049756332300082142292422716f; else lEItMLgcezSJSeKqSpVCcasSrPcEWw=1424752939.318449734732635394429726940872f;if (lEItMLgcezSJSeKqSpVCcasSrPcEWw - lEItMLgcezSJSeKqSpVCcasSrPcEWw> 0.00000001 ) lEItMLgcezSJSeKqSpVCcasSrPcEWw=1539853009.714012596245952076814830869361f; else lEItMLgcezSJSeKqSpVCcasSrPcEWw=270202194.997302863390839488497516898535f;if (lEItMLgcezSJSeKqSpVCcasSrPcEWw - lEItMLgcezSJSeKqSpVCcasSrPcEWw> 0.00000001 ) lEItMLgcezSJSeKqSpVCcasSrPcEWw=1659431030.334635975022362511148297449151f; else lEItMLgcezSJSeKqSpVCcasSrPcEWw=435607648.543807631462347535549740724739f;double lLTcsyadbtpDmpisZAtIThGLovODpO=1608701145.166344237527382845819761181172;if (lLTcsyadbtpDmpisZAtIThGLovODpO == lLTcsyadbtpDmpisZAtIThGLovODpO ) lLTcsyadbtpDmpisZAtIThGLovODpO=2146976063.096211447284970717197850751527; else lLTcsyadbtpDmpisZAtIThGLovODpO=1942251889.416653218240874599247419635667;if (lLTcsyadbtpDmpisZAtIThGLovODpO == lLTcsyadbtpDmpisZAtIThGLovODpO ) lLTcsyadbtpDmpisZAtIThGLovODpO=148475515.192088833860005709727349010813; else lLTcsyadbtpDmpisZAtIThGLovODpO=1582004297.966215899786758842494605196408;if (lLTcsyadbtpDmpisZAtIThGLovODpO == lLTcsyadbtpDmpisZAtIThGLovODpO ) lLTcsyadbtpDmpisZAtIThGLovODpO=1594830742.803191599001004260137952936113; else lLTcsyadbtpDmpisZAtIThGLovODpO=605389243.049209366836440371193648196127;if (lLTcsyadbtpDmpisZAtIThGLovODpO == lLTcsyadbtpDmpisZAtIThGLovODpO ) lLTcsyadbtpDmpisZAtIThGLovODpO=1528734162.648616391691540261377603141111; else lLTcsyadbtpDmpisZAtIThGLovODpO=290650822.325930884518061220904696114657;if (lLTcsyadbtpDmpisZAtIThGLovODpO == lLTcsyadbtpDmpisZAtIThGLovODpO ) lLTcsyadbtpDmpisZAtIThGLovODpO=234025073.528038762148897965293232884993; else lLTcsyadbtpDmpisZAtIThGLovODpO=1898518642.633709113466996680025047540726;if (lLTcsyadbtpDmpisZAtIThGLovODpO == lLTcsyadbtpDmpisZAtIThGLovODpO ) lLTcsyadbtpDmpisZAtIThGLovODpO=1111563371.488052529895393735630407879727; else lLTcsyadbtpDmpisZAtIThGLovODpO=2093502831.049809904517124175481610618139;double rvLtWhaxXoPDGGPNrPkvTNUeTStHtC=688514624.528717033712138782302606589066;if (rvLtWhaxXoPDGGPNrPkvTNUeTStHtC == rvLtWhaxXoPDGGPNrPkvTNUeTStHtC ) rvLtWhaxXoPDGGPNrPkvTNUeTStHtC=1937846917.435694767701503856870084793333; else rvLtWhaxXoPDGGPNrPkvTNUeTStHtC=649965089.808739865513671211374320755190;if (rvLtWhaxXoPDGGPNrPkvTNUeTStHtC == rvLtWhaxXoPDGGPNrPkvTNUeTStHtC ) rvLtWhaxXoPDGGPNrPkvTNUeTStHtC=64874833.253490397510246752469306058909; else rvLtWhaxXoPDGGPNrPkvTNUeTStHtC=754634318.477647028193487429411484205854;if (rvLtWhaxXoPDGGPNrPkvTNUeTStHtC == rvLtWhaxXoPDGGPNrPkvTNUeTStHtC ) rvLtWhaxXoPDGGPNrPkvTNUeTStHtC=894341011.350924361083437693064950900355; else rvLtWhaxXoPDGGPNrPkvTNUeTStHtC=1792521625.327494456327853815861290141013;if (rvLtWhaxXoPDGGPNrPkvTNUeTStHtC == rvLtWhaxXoPDGGPNrPkvTNUeTStHtC ) rvLtWhaxXoPDGGPNrPkvTNUeTStHtC=1169930931.512786605435785821710569414835; else rvLtWhaxXoPDGGPNrPkvTNUeTStHtC=582692078.444481527821153652543040405337;if (rvLtWhaxXoPDGGPNrPkvTNUeTStHtC == rvLtWhaxXoPDGGPNrPkvTNUeTStHtC ) rvLtWhaxXoPDGGPNrPkvTNUeTStHtC=1368011171.297100140431400952786330351518; else rvLtWhaxXoPDGGPNrPkvTNUeTStHtC=1739764257.729163405462222364156476107886;if (rvLtWhaxXoPDGGPNrPkvTNUeTStHtC == rvLtWhaxXoPDGGPNrPkvTNUeTStHtC ) rvLtWhaxXoPDGGPNrPkvTNUeTStHtC=1939906801.074381245765843512819746336406; else rvLtWhaxXoPDGGPNrPkvTNUeTStHtC=1927933056.763940805112049174379608743815;double sUsyyNqxOlIklzjjzlGkLCRjJeWMDk=1953909820.118337581274800375060501837956;if (sUsyyNqxOlIklzjjzlGkLCRjJeWMDk == sUsyyNqxOlIklzjjzlGkLCRjJeWMDk ) sUsyyNqxOlIklzjjzlGkLCRjJeWMDk=718623126.483003459663073325576428372760; else sUsyyNqxOlIklzjjzlGkLCRjJeWMDk=431584697.064590180792651864952358006674;if (sUsyyNqxOlIklzjjzlGkLCRjJeWMDk == sUsyyNqxOlIklzjjzlGkLCRjJeWMDk ) sUsyyNqxOlIklzjjzlGkLCRjJeWMDk=1459664987.279960959424588488761991872769; else sUsyyNqxOlIklzjjzlGkLCRjJeWMDk=1802171909.452762480292563997063991339819;if (sUsyyNqxOlIklzjjzlGkLCRjJeWMDk == sUsyyNqxOlIklzjjzlGkLCRjJeWMDk ) sUsyyNqxOlIklzjjzlGkLCRjJeWMDk=1778125497.839007282711968044809570667144; else sUsyyNqxOlIklzjjzlGkLCRjJeWMDk=478338388.467147356771646304449612137055;if (sUsyyNqxOlIklzjjzlGkLCRjJeWMDk == sUsyyNqxOlIklzjjzlGkLCRjJeWMDk ) sUsyyNqxOlIklzjjzlGkLCRjJeWMDk=2070589504.125936482179823803546470586612; else sUsyyNqxOlIklzjjzlGkLCRjJeWMDk=650117684.131166771003247089933461985556;if (sUsyyNqxOlIklzjjzlGkLCRjJeWMDk == sUsyyNqxOlIklzjjzlGkLCRjJeWMDk ) sUsyyNqxOlIklzjjzlGkLCRjJeWMDk=608937886.187114976713355424451778988129; else sUsyyNqxOlIklzjjzlGkLCRjJeWMDk=2102419944.133602623321333112666220641024;if (sUsyyNqxOlIklzjjzlGkLCRjJeWMDk == sUsyyNqxOlIklzjjzlGkLCRjJeWMDk ) sUsyyNqxOlIklzjjzlGkLCRjJeWMDk=1433552738.785879185481213055697757302417; else sUsyyNqxOlIklzjjzlGkLCRjJeWMDk=2087330388.170974656414739611760743714968;long klqNFBMXvUvuHtSwJpktDgXiVPmKfC=333057788;if (klqNFBMXvUvuHtSwJpktDgXiVPmKfC == klqNFBMXvUvuHtSwJpktDgXiVPmKfC- 1 ) klqNFBMXvUvuHtSwJpktDgXiVPmKfC=1454991515; else klqNFBMXvUvuHtSwJpktDgXiVPmKfC=970916406;if (klqNFBMXvUvuHtSwJpktDgXiVPmKfC == klqNFBMXvUvuHtSwJpktDgXiVPmKfC- 1 ) klqNFBMXvUvuHtSwJpktDgXiVPmKfC=645244378; else klqNFBMXvUvuHtSwJpktDgXiVPmKfC=1429512574;if (klqNFBMXvUvuHtSwJpktDgXiVPmKfC == klqNFBMXvUvuHtSwJpktDgXiVPmKfC- 1 ) klqNFBMXvUvuHtSwJpktDgXiVPmKfC=75443414; else klqNFBMXvUvuHtSwJpktDgXiVPmKfC=1315068999;if (klqNFBMXvUvuHtSwJpktDgXiVPmKfC == klqNFBMXvUvuHtSwJpktDgXiVPmKfC- 1 ) klqNFBMXvUvuHtSwJpktDgXiVPmKfC=714360134; else klqNFBMXvUvuHtSwJpktDgXiVPmKfC=354454438;if (klqNFBMXvUvuHtSwJpktDgXiVPmKfC == klqNFBMXvUvuHtSwJpktDgXiVPmKfC- 1 ) klqNFBMXvUvuHtSwJpktDgXiVPmKfC=772320958; else klqNFBMXvUvuHtSwJpktDgXiVPmKfC=1713267084;if (klqNFBMXvUvuHtSwJpktDgXiVPmKfC == klqNFBMXvUvuHtSwJpktDgXiVPmKfC- 0 ) klqNFBMXvUvuHtSwJpktDgXiVPmKfC=213993126; else klqNFBMXvUvuHtSwJpktDgXiVPmKfC=2002550132;int GFputkXvxCeWLxXwuuoAInqnGSjGAh=1832273476;if (GFputkXvxCeWLxXwuuoAInqnGSjGAh == GFputkXvxCeWLxXwuuoAInqnGSjGAh- 1 ) GFputkXvxCeWLxXwuuoAInqnGSjGAh=215844460; else GFputkXvxCeWLxXwuuoAInqnGSjGAh=1305137198;if (GFputkXvxCeWLxXwuuoAInqnGSjGAh == GFputkXvxCeWLxXwuuoAInqnGSjGAh- 0 ) GFputkXvxCeWLxXwuuoAInqnGSjGAh=1374879705; else GFputkXvxCeWLxXwuuoAInqnGSjGAh=172353550;if (GFputkXvxCeWLxXwuuoAInqnGSjGAh == GFputkXvxCeWLxXwuuoAInqnGSjGAh- 0 ) GFputkXvxCeWLxXwuuoAInqnGSjGAh=1894613881; else GFputkXvxCeWLxXwuuoAInqnGSjGAh=1308006064;if (GFputkXvxCeWLxXwuuoAInqnGSjGAh == GFputkXvxCeWLxXwuuoAInqnGSjGAh- 1 ) GFputkXvxCeWLxXwuuoAInqnGSjGAh=1057872774; else GFputkXvxCeWLxXwuuoAInqnGSjGAh=1459508529;if (GFputkXvxCeWLxXwuuoAInqnGSjGAh == GFputkXvxCeWLxXwuuoAInqnGSjGAh- 1 ) GFputkXvxCeWLxXwuuoAInqnGSjGAh=726859232; else GFputkXvxCeWLxXwuuoAInqnGSjGAh=1562240796;if (GFputkXvxCeWLxXwuuoAInqnGSjGAh == GFputkXvxCeWLxXwuuoAInqnGSjGAh- 0 ) GFputkXvxCeWLxXwuuoAInqnGSjGAh=145696432; else GFputkXvxCeWLxXwuuoAInqnGSjGAh=939404714;long kPzuzVtjFDXFWJWhsEfMOXDywVSigs=2011424299;if (kPzuzVtjFDXFWJWhsEfMOXDywVSigs == kPzuzVtjFDXFWJWhsEfMOXDywVSigs- 1 ) kPzuzVtjFDXFWJWhsEfMOXDywVSigs=918949565; else kPzuzVtjFDXFWJWhsEfMOXDywVSigs=1606849013;if (kPzuzVtjFDXFWJWhsEfMOXDywVSigs == kPzuzVtjFDXFWJWhsEfMOXDywVSigs- 1 ) kPzuzVtjFDXFWJWhsEfMOXDywVSigs=472941541; else kPzuzVtjFDXFWJWhsEfMOXDywVSigs=1029002319;if (kPzuzVtjFDXFWJWhsEfMOXDywVSigs == kPzuzVtjFDXFWJWhsEfMOXDywVSigs- 0 ) kPzuzVtjFDXFWJWhsEfMOXDywVSigs=2051474513; else kPzuzVtjFDXFWJWhsEfMOXDywVSigs=253161608;if (kPzuzVtjFDXFWJWhsEfMOXDywVSigs == kPzuzVtjFDXFWJWhsEfMOXDywVSigs- 0 ) kPzuzVtjFDXFWJWhsEfMOXDywVSigs=1878466406; else kPzuzVtjFDXFWJWhsEfMOXDywVSigs=1551037774;if (kPzuzVtjFDXFWJWhsEfMOXDywVSigs == kPzuzVtjFDXFWJWhsEfMOXDywVSigs- 0 ) kPzuzVtjFDXFWJWhsEfMOXDywVSigs=1864708210; else kPzuzVtjFDXFWJWhsEfMOXDywVSigs=1105210549;if (kPzuzVtjFDXFWJWhsEfMOXDywVSigs == kPzuzVtjFDXFWJWhsEfMOXDywVSigs- 1 ) kPzuzVtjFDXFWJWhsEfMOXDywVSigs=1029850951; else kPzuzVtjFDXFWJWhsEfMOXDywVSigs=64858413;double XARvpFACXodJoPyPqlyCBUxtNumOPX=1008136780.032182601892391991192706714139;if (XARvpFACXodJoPyPqlyCBUxtNumOPX == XARvpFACXodJoPyPqlyCBUxtNumOPX ) XARvpFACXodJoPyPqlyCBUxtNumOPX=1664204200.859058014308980641792122634747; else XARvpFACXodJoPyPqlyCBUxtNumOPX=1748762909.857315445308790482628455773752;if (XARvpFACXodJoPyPqlyCBUxtNumOPX == XARvpFACXodJoPyPqlyCBUxtNumOPX ) XARvpFACXodJoPyPqlyCBUxtNumOPX=1871485928.538755178630649554226213051364; else XARvpFACXodJoPyPqlyCBUxtNumOPX=1496010734.270497114542410882633063733571;if (XARvpFACXodJoPyPqlyCBUxtNumOPX == XARvpFACXodJoPyPqlyCBUxtNumOPX ) XARvpFACXodJoPyPqlyCBUxtNumOPX=129190814.592546135244179480338973121777; else XARvpFACXodJoPyPqlyCBUxtNumOPX=648461142.564415108485749009535435463942;if (XARvpFACXodJoPyPqlyCBUxtNumOPX == XARvpFACXodJoPyPqlyCBUxtNumOPX ) XARvpFACXodJoPyPqlyCBUxtNumOPX=585927554.547889187449396333523334324905; else XARvpFACXodJoPyPqlyCBUxtNumOPX=146985451.327694906193220614014052032367;if (XARvpFACXodJoPyPqlyCBUxtNumOPX == XARvpFACXodJoPyPqlyCBUxtNumOPX ) XARvpFACXodJoPyPqlyCBUxtNumOPX=2060527279.528312381244743122571109248567; else XARvpFACXodJoPyPqlyCBUxtNumOPX=1333219496.265068601944487131002199653918;if (XARvpFACXodJoPyPqlyCBUxtNumOPX == XARvpFACXodJoPyPqlyCBUxtNumOPX ) XARvpFACXodJoPyPqlyCBUxtNumOPX=702653342.482419089662163882311857628641; else XARvpFACXodJoPyPqlyCBUxtNumOPX=660686265.679456208926244003698271773699;long SSnBrNlCjhoQQwZaFMcGYybTUEsgsP=1689720415;if (SSnBrNlCjhoQQwZaFMcGYybTUEsgsP == SSnBrNlCjhoQQwZaFMcGYybTUEsgsP- 1 ) SSnBrNlCjhoQQwZaFMcGYybTUEsgsP=1784239291; else SSnBrNlCjhoQQwZaFMcGYybTUEsgsP=350052474;if (SSnBrNlCjhoQQwZaFMcGYybTUEsgsP == SSnBrNlCjhoQQwZaFMcGYybTUEsgsP- 0 ) SSnBrNlCjhoQQwZaFMcGYybTUEsgsP=1083600567; else SSnBrNlCjhoQQwZaFMcGYybTUEsgsP=1353600723;if (SSnBrNlCjhoQQwZaFMcGYybTUEsgsP == SSnBrNlCjhoQQwZaFMcGYybTUEsgsP- 1 ) SSnBrNlCjhoQQwZaFMcGYybTUEsgsP=1275343212; else SSnBrNlCjhoQQwZaFMcGYybTUEsgsP=1556697072;if (SSnBrNlCjhoQQwZaFMcGYybTUEsgsP == SSnBrNlCjhoQQwZaFMcGYybTUEsgsP- 0 ) SSnBrNlCjhoQQwZaFMcGYybTUEsgsP=1928282610; else SSnBrNlCjhoQQwZaFMcGYybTUEsgsP=2110166499;if (SSnBrNlCjhoQQwZaFMcGYybTUEsgsP == SSnBrNlCjhoQQwZaFMcGYybTUEsgsP- 0 ) SSnBrNlCjhoQQwZaFMcGYybTUEsgsP=1574154439; else SSnBrNlCjhoQQwZaFMcGYybTUEsgsP=1971325578;if (SSnBrNlCjhoQQwZaFMcGYybTUEsgsP == SSnBrNlCjhoQQwZaFMcGYybTUEsgsP- 1 ) SSnBrNlCjhoQQwZaFMcGYybTUEsgsP=455128987; else SSnBrNlCjhoQQwZaFMcGYybTUEsgsP=789403332;float fTqsyMifJzJTZxNEnIzXSUEsiBmRTN=658819860.294325192760829676126270279763f;if (fTqsyMifJzJTZxNEnIzXSUEsiBmRTN - fTqsyMifJzJTZxNEnIzXSUEsiBmRTN> 0.00000001 ) fTqsyMifJzJTZxNEnIzXSUEsiBmRTN=1725089201.368034846765461484136183302515f; else fTqsyMifJzJTZxNEnIzXSUEsiBmRTN=1285043299.644951289684047191090674739683f;if (fTqsyMifJzJTZxNEnIzXSUEsiBmRTN - fTqsyMifJzJTZxNEnIzXSUEsiBmRTN> 0.00000001 ) fTqsyMifJzJTZxNEnIzXSUEsiBmRTN=1422684161.243222252120555314622263508123f; else fTqsyMifJzJTZxNEnIzXSUEsiBmRTN=757689625.371655193604097112942516219172f;if (fTqsyMifJzJTZxNEnIzXSUEsiBmRTN - fTqsyMifJzJTZxNEnIzXSUEsiBmRTN> 0.00000001 ) fTqsyMifJzJTZxNEnIzXSUEsiBmRTN=1506081909.414096991033213927650628187655f; else fTqsyMifJzJTZxNEnIzXSUEsiBmRTN=580500706.702229111223292941449253280691f;if (fTqsyMifJzJTZxNEnIzXSUEsiBmRTN - fTqsyMifJzJTZxNEnIzXSUEsiBmRTN> 0.00000001 ) fTqsyMifJzJTZxNEnIzXSUEsiBmRTN=1882233485.045743109491653854735622656004f; else fTqsyMifJzJTZxNEnIzXSUEsiBmRTN=270488423.053129856994206871054697642319f;if (fTqsyMifJzJTZxNEnIzXSUEsiBmRTN - fTqsyMifJzJTZxNEnIzXSUEsiBmRTN> 0.00000001 ) fTqsyMifJzJTZxNEnIzXSUEsiBmRTN=1020921459.811544953501885252799615146769f; else fTqsyMifJzJTZxNEnIzXSUEsiBmRTN=325956090.765728487202866482250073131640f;if (fTqsyMifJzJTZxNEnIzXSUEsiBmRTN - fTqsyMifJzJTZxNEnIzXSUEsiBmRTN> 0.00000001 ) fTqsyMifJzJTZxNEnIzXSUEsiBmRTN=1115958687.830595627887650886086551079270f; else fTqsyMifJzJTZxNEnIzXSUEsiBmRTN=718255017.784758237287657353435677548962f;float TbhDBRvzKeZrpzwuyXiKgtScEIiMHy=2089752790.208294244259169531265477850060f;if (TbhDBRvzKeZrpzwuyXiKgtScEIiMHy - TbhDBRvzKeZrpzwuyXiKgtScEIiMHy> 0.00000001 ) TbhDBRvzKeZrpzwuyXiKgtScEIiMHy=1109724998.391561708460635092228481993144f; else TbhDBRvzKeZrpzwuyXiKgtScEIiMHy=1487614183.539506970894491595286885292068f;if (TbhDBRvzKeZrpzwuyXiKgtScEIiMHy - TbhDBRvzKeZrpzwuyXiKgtScEIiMHy> 0.00000001 ) TbhDBRvzKeZrpzwuyXiKgtScEIiMHy=428102769.867493213923353519151906875441f; else TbhDBRvzKeZrpzwuyXiKgtScEIiMHy=2061776435.501203800838337463278302807262f;if (TbhDBRvzKeZrpzwuyXiKgtScEIiMHy - TbhDBRvzKeZrpzwuyXiKgtScEIiMHy> 0.00000001 ) TbhDBRvzKeZrpzwuyXiKgtScEIiMHy=1131922452.888253316182932477952237320050f; else TbhDBRvzKeZrpzwuyXiKgtScEIiMHy=631034519.764906019525820758054032176294f;if (TbhDBRvzKeZrpzwuyXiKgtScEIiMHy - TbhDBRvzKeZrpzwuyXiKgtScEIiMHy> 0.00000001 ) TbhDBRvzKeZrpzwuyXiKgtScEIiMHy=1880277215.337642813747954752877300774394f; else TbhDBRvzKeZrpzwuyXiKgtScEIiMHy=107507128.092529938804531829843543704707f;if (TbhDBRvzKeZrpzwuyXiKgtScEIiMHy - TbhDBRvzKeZrpzwuyXiKgtScEIiMHy> 0.00000001 ) TbhDBRvzKeZrpzwuyXiKgtScEIiMHy=1967963946.963976553692417487006656265982f; else TbhDBRvzKeZrpzwuyXiKgtScEIiMHy=1908803076.534204277638636124643780232871f;if (TbhDBRvzKeZrpzwuyXiKgtScEIiMHy - TbhDBRvzKeZrpzwuyXiKgtScEIiMHy> 0.00000001 ) TbhDBRvzKeZrpzwuyXiKgtScEIiMHy=2069339942.177116921422147802025234096407f; else TbhDBRvzKeZrpzwuyXiKgtScEIiMHy=305348883.350667153020333274444086794323f; }
 TbhDBRvzKeZrpzwuyXiKgtScEIiMHyy::TbhDBRvzKeZrpzwuyXiKgtScEIiMHyy()
 { this->TutyjzdEUujR("nacSGuUfwaMGjvlqjYUoXtyyaIEztiTutyjzdEUujRj", true, 193332155, 1944007869, 2016313985); }
#pragma optimize("", off)
 // <delete/>

