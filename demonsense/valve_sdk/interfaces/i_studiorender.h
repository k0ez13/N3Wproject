#pragma once
class light_desc_t;
class lightpos_t;

enum light_type_t {
  MATERIAL_LIGHT_DISABLE = 0,
  MATERIAL_LIGHT_POINT,
  MATERIAL_LIGHT_DIRECTIONAL,
  MATERIAL_LIGHT_SPOT,
};

class light_desc_t {
public:
  light_type_t m_Type;                 //0x0000
  Vector m_Color;                     //0x0004
  Vector m_Position;                  //0x0010
  Vector m_Direction;                 //0x001C
  float m_Range;                      //0x0028
  float m_Falloff;                    //0x002C
  float m_Attenuation0;               //0x0030
  float m_Attenuation1;               //0x0034
  float m_Attenuation2;               //0x0038
  float m_Theta;                      //0x003C
  float m_Phi;                        //0x0040
  float m_ThetaDot;                   //0x0044
  float m_PhiDot;                     //0x0048
  float m_OneOverThetaDotMinusPhiDot; //0x004C
  __int32 m_Flags;                    //0x0050
  float m_RangeSquared;               //0x0054

}; //Size=0x0058

class lightpos_t {
public:
  Vector delta;  //0x0000
  float falloff; //0x000C
  float dot;     //0x0010

}; //Size=0x0014

struct material_lighting_state_t {
  Vector m_vecAmbientCube[6]; // ambient, and lights that aren't in locallight[]
  Vector m_vecLightingOrigin; // The position from which lighting state was computed
  int m_nLocalLightCount;
  light_desc_t m_pLocalLightDesc[4];
};

struct draw_model_info_t {
  studiohdr_t* m_pStudioHdr;
  studiohwdata_t* m_pHardwareData;
  studio_decal_handle_t m_Decals;
  int m_Skin;
  int m_Body;
  int m_HitboxSet;
  i_client_renderable* m_pClientEntity;
  int m_Lod;
  color_mesh_info_t* m_pColorMeshes;
  bool m_bStaticLighting;
  material_lighting_state_t m_LightingState;
};

class i_studiorender {
public:
  void* vtable;
  float fEyeShiftX;                      //0x0004
  float fEyeShiftY;                      //0x0008
  float fEyeShiftZ;                      //0x000C
  float fEyeSize;                        //0x0010
  float fEyeGlintPixelWidthLODThreshold; //0x0014
  __int32 maxDecalsPerModel;             //0x0018
  __int32 drawEntities;                  //0x001C
  __int32 skin;                          //0x0020
  __int32 fullbright;                    //0x0024
  bool bEyeMove : 1;                     // look around
  bool bSoftwareSkin : 1;
  bool bNoHardware : 1;
  bool bNoSoftware : 1;
  bool bTeeth : 1;
  bool bEyes : 1;
  bool bFlex : 1;
  bool bWireframe : 1;
  bool bDrawNormals : 1;
  bool bDrawTangentFrame : 1;
  bool bDrawZBufferedWireframe : 1;
  bool bSoftwareLighting : 1;
  bool bShowEnvCubemapOnly : 1;
  bool bWireframeDecals : 1;
  int m_nReserved[5];
  Vector m_ViewTarget;           //0x0040
  Vector m_ViewOrigin;           //0x004C
  Vector m_ViewRight;            //0x0058
  Vector m_ViewUp;               //0x0064
  Vector m_ViewPlaneNormal;      //0x0070
  Vector4D m_LightBoxColors[6];  //0x00CC
  light_desc_t m_LocalLights[4];  //0x01E4
  __int32 m_NumLocalLights;      //0x023C
  float m_ColorMod[3];           //0x0248
  float m_AlphaMod;              //0x024C
  i_material* m_pForcedMaterial;  //0x0250
  override_type m_nForcedMaterialType; //0x0254
  char pad_0x0258[0xC];          //0x0258
  __int32 unkhandle1;            //0x0264
  __int32 unkhandle2;            //0x0268
  __int32 unkhandle3;            //0x026C
  __int32 unkhandle4;            //0x0270
  char pad_0x0274[0x4];          //0x0274
  lightpos_t m_pLightPos[16];    //0x0278

  void set_color_modulation( float const* pColor ) {
	 typedef void( __thiscall * OrigFn )( void*, float const* );
	 CallVFunction< OrigFn >( this, 27 )( this, pColor );
  }

  void forced_material_override( i_material* material, override_type noverride_type = override_type::Normal ) {
	 m_pForcedMaterial = material;
	 m_nForcedMaterialType = noverride_type;
  }
};

static_assert( FIELD_OFFSET(i_studiorender, m_pForcedMaterial ) == 0x250, "Wrong m_pForcedMaterial offset" );
