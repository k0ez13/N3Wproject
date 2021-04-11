// dear imgui: Renderer for DirectX9
// This needs to be used along with a Platform Binding (e.g. Win32)

// Implemented features:
//  [X] Renderer: User texture binding. Use 'LPDIRECT3DTEXTURE9' as ImTextureID. Read the FAQ about ImTextureID in imgui.cpp.

// You can copy and use unmodified imgui_impl_* files in your project. See main.cpp for an example of using this.
// If you are new to dear imgui, read examples/README.txt and read the documentation at the top of imgui.cpp.
// https://github.com/ocornut/imgui

// CHANGELOG
// (minor and older changes stripped away, please see git history for details)
//  2019-01-16: Misc: Disabled fog before drawing UI's. Fixes issue #2288.
//  2018-11-30: Misc: Setting up io.BackendRendererName so it can be displayed in the About Window.
//  2018-06-08: Misc: Extracted imgui_impl_dx9.cpp/.h away from the old combined DX9+Win32 example.
//  2018-06-08: DirectX9: Use draw_data->DisplayPos and draw_data->DisplaySize to setup projection matrix and clipping rectangle.
//  2018-05-07: Render: Saving/restoring Transform because they don't seem to be included in the StateBlock. Setting shading mode to Gouraud.
//  2018-02-16: Misc: Obsoleted the io.RenderDrawListsFn callback and exposed ImGui_ImplDX9_RenderDrawData() in the .h file so you can call it yourself.
//  2018-02-06: Misc: Removed call to ImGui::Shutdown() which is not available from 1.60 WIP, user needs to call CreateContext/DestroyContext themselves.

#include "imgui.h"
#include "imgui_impl_dx9.h"

// DirectX
#include <d3d9.h>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

// DirectX data
static LPDIRECT3DDEVICE9        g_pd3dDevice = NULL;
static LPDIRECT3DVERTEXBUFFER9  g_pVB = NULL;
static LPDIRECT3DINDEXBUFFER9   g_pIB = NULL;
static LPDIRECT3DTEXTURE9       g_FontTexture = NULL;
static int                      g_VertexBufferSize = 5000, g_IndexBufferSize = 10000;

struct CUSTOMVERTEX
{
    float    pos[3];
    D3DCOLOR col;
    float    uv[2];
};
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1)

// Render function.
// (this used to be set in io.RenderDrawListsFn and called by ImGui::Render(), but you can now call this directly from your main loop)
void ImGui_ImplDX9_RenderDrawData(ImDrawData* draw_data)
{
    // Avoid rendering when minimized
    if (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f)
        return;

    // Create and grow buffers if needed
    if (!g_pVB || g_VertexBufferSize < draw_data->TotalVtxCount)
    {
        if (g_pVB) { g_pVB->Release(); g_pVB = NULL; }
        g_VertexBufferSize = draw_data->TotalVtxCount + 5000;
        if (g_pd3dDevice->CreateVertexBuffer(g_VertexBufferSize * sizeof(CUSTOMVERTEX), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &g_pVB, NULL) < 0)
            return;
    }
    if (!g_pIB || g_IndexBufferSize < draw_data->TotalIdxCount)
    {
        if (g_pIB) { g_pIB->Release(); g_pIB = NULL; }
        g_IndexBufferSize = draw_data->TotalIdxCount + 10000;
        if (g_pd3dDevice->CreateIndexBuffer(g_IndexBufferSize * sizeof(ImDrawIdx), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, sizeof(ImDrawIdx) == 2 ? D3DFMT_INDEX16 : D3DFMT_INDEX32, D3DPOOL_DEFAULT, &g_pIB, NULL) < 0)
            return;
    }

    // Backup the DX9 state
    IDirect3DStateBlock9* d3d9_state_block = NULL;
	if (g_pd3dDevice->CreateStateBlock(D3DSBT_PIXELSTATE, &d3d9_state_block) < 0)
        return;

    // Backup the DX9 transform (DX9 documentation suggests that it is included in the StateBlock but it doesn't appear to)
    D3DMATRIX last_world, last_view, last_projection;
    g_pd3dDevice->GetTransform(D3DTS_WORLD, &last_world);
    g_pd3dDevice->GetTransform(D3DTS_VIEW, &last_view);
    g_pd3dDevice->GetTransform(D3DTS_PROJECTION, &last_projection);

    // Copy and convert all vertices into a single contiguous buffer, convert colors to DX9 default format.
    // FIXME-OPT: This is a waste of resource, the ideal is to use imconfig.h and
    //  1) to avoid repacking colors:   #define IMGUI_USE_BGRA_PACKED_COLOR
    //  2) to avoid repacking vertices: #define IMGUI_OVERRIDE_DRAWVERT_STRUCT_LAYOUT struct ImDrawVert { ImVec2 pos; float z; ImU32 col; ImVec2 uv; }
    CUSTOMVERTEX* vtx_dst;
    ImDrawIdx* idx_dst;
    if (g_pVB->Lock(0, (UINT)(draw_data->TotalVtxCount * sizeof(CUSTOMVERTEX)), (void**)&vtx_dst, D3DLOCK_DISCARD) < 0)
        return;
    if (g_pIB->Lock(0, (UINT)(draw_data->TotalIdxCount * sizeof(ImDrawIdx)), (void**)&idx_dst, D3DLOCK_DISCARD) < 0)
        return;
    for (int n = 0; n < draw_data->CmdListsCount; n++)
    {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];
        const ImDrawVert* vtx_src = cmd_list->VtxBuffer.Data;
        for (int i = 0; i < cmd_list->VtxBuffer.Size; i++)
        {
            vtx_dst->pos[0] = vtx_src->pos.x;
            vtx_dst->pos[1] = vtx_src->pos.y;
            vtx_dst->pos[2] = 0.0f;
            vtx_dst->col = (vtx_src->col & 0xFF00FF00) | ((vtx_src->col & 0xFF0000) >> 16) | ((vtx_src->col & 0xFF) << 16);     // RGBA --> ARGB for DirectX9
            vtx_dst->uv[0] = vtx_src->uv.x;
            vtx_dst->uv[1] = vtx_src->uv.y;
            vtx_dst++;
            vtx_src++;
        }
        memcpy(idx_dst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
        idx_dst += cmd_list->IdxBuffer.Size;
    }
    g_pVB->Unlock();
    g_pIB->Unlock();
    g_pd3dDevice->SetStreamSource(0, g_pVB, 0, sizeof(CUSTOMVERTEX));
    g_pd3dDevice->SetIndices(g_pIB);
    g_pd3dDevice->SetFVF(D3DFVF_CUSTOMVERTEX);

    // Setup viewport
    D3DVIEWPORT9 vp;
    vp.X = vp.Y = 0;
    vp.Width = (DWORD)draw_data->DisplaySize.x;
    vp.Height = (DWORD)draw_data->DisplaySize.y;
    vp.MinZ = 0.0f;
    vp.MaxZ = 1.0f;
    g_pd3dDevice->SetViewport(&vp);

    // Setup render state: fixed-pipeline, alpha-blending, no face culling, no depth testing, shade mode (for gradient)
    g_pd3dDevice->SetPixelShader(NULL);
    g_pd3dDevice->SetVertexShader(NULL);
    g_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, false);
    g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, false);
    g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
    g_pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, false);
    g_pd3dDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
    g_pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    g_pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    g_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, true);
    g_pd3dDevice->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
    g_pd3dDevice->SetRenderState(D3DRS_FOGENABLE, false);
    g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
    g_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
    g_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    g_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
    g_pd3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    g_pd3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

    // Setup orthographic projection matrix
    // Our visible imgui space lies from draw_data->DisplayPos (top left) to draw_data->DisplayPos+data_data->DisplaySize (bottom right).
    // Being agnostic of whether <d3dx9.h> or <DirectXMath.h> can be used, we aren't relying on D3DXMatrixIdentity()/D3DXMatrixOrthoOffCenterLH() or DirectX::XMMatrixIdentity()/DirectX::XMMatrixOrthographicOffCenterLH()
    {
        float L = draw_data->DisplayPos.x + 0.5f;
        float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x + 0.5f;
        float T = draw_data->DisplayPos.y + 0.5f;
        float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y + 0.5f;
        D3DMATRIX mat_identity = { { { 1.0f, 0.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f, 0.0f,  0.0f, 0.0f, 0.0f, 1.0f } } };
        D3DMATRIX mat_projection =
        { { {
            2.0f/(R-L),   0.0f,         0.0f,  0.0f,
            0.0f,         2.0f/(T-B),   0.0f,  0.0f,
            0.0f,         0.0f,         0.5f,  0.0f,
            (L+R)/(L-R),  (T+B)/(B-T),  0.5f,  1.0f
        } } };
        g_pd3dDevice->SetTransform(D3DTS_WORLD, &mat_identity);
        g_pd3dDevice->SetTransform(D3DTS_VIEW, &mat_identity);
        g_pd3dDevice->SetTransform(D3DTS_PROJECTION, &mat_projection);
    }

    // Render command lists
    int vtx_offset = 0;
    int idx_offset = 0;
    ImVec2 clip_off = draw_data->DisplayPos;
    for (int n = 0; n < draw_data->CmdListsCount; n++)
    {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];
        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
        {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
            if (pcmd->UserCallback)
            {
                pcmd->UserCallback(cmd_list, pcmd);
            }
            else
            {
                const RECT r = { (LONG)(pcmd->ClipRect.x - clip_off.x), (LONG)(pcmd->ClipRect.y - clip_off.y), (LONG)(pcmd->ClipRect.z - clip_off.x), (LONG)(pcmd->ClipRect.w - clip_off.y) };
                const LPDIRECT3DTEXTURE9 texture = (LPDIRECT3DTEXTURE9)pcmd->TextureId;
                g_pd3dDevice->SetTexture(0, texture);
                g_pd3dDevice->SetScissorRect(&r);
                g_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, vtx_offset, 0, (UINT)cmd_list->VtxBuffer.Size, idx_offset, pcmd->ElemCount/3);
            }
            idx_offset += pcmd->ElemCount;
        }
        vtx_offset += cmd_list->VtxBuffer.Size;
    }

    // Restore the DX9 transform
    g_pd3dDevice->SetTransform(D3DTS_WORLD, &last_world);
    g_pd3dDevice->SetTransform(D3DTS_VIEW, &last_view);
    g_pd3dDevice->SetTransform(D3DTS_PROJECTION, &last_projection);

    // Restore the DX9 state
    d3d9_state_block->Apply();
    d3d9_state_block->Release();
}

bool ImGui_ImplDX9_Init(IDirect3DDevice9* device)
{
    ImGuiIO& io = ImGui::GetIO();
    io.BackendRendererName = "imgui_impl_dx9";

    g_pd3dDevice = device;
    return true;
}

void ImGui_ImplDX9_Shutdown()
{
    ImGui_ImplDX9_InvalidateDeviceObjects();
    g_pd3dDevice = NULL;
}

static bool ImGui_ImplDX9_CreateFontsTexture()
{
    // Build texture atlas
    ImGuiIO& io = ImGui::GetIO();
    unsigned char* pixels;
    int width, height, bytes_per_pixel;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height, &bytes_per_pixel);

    // Upload texture to graphics system
    g_FontTexture = NULL;
    if (g_pd3dDevice->CreateTexture(width, height, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &g_FontTexture, NULL) < 0)
        return false;
    D3DLOCKED_RECT tex_locked_rect;
    if (g_FontTexture->LockRect(0, &tex_locked_rect, NULL, 0) != D3D_OK)
        return false;
    for (int y = 0; y < height; y++)
        memcpy((unsigned char *)tex_locked_rect.pBits + tex_locked_rect.Pitch * y, pixels + (width * bytes_per_pixel) * y, (width * bytes_per_pixel));
    g_FontTexture->UnlockRect(0);

    // Store our identifier
    io.Fonts->TexID = (ImTextureID)g_FontTexture;

    return true;
}

bool ImGui_ImplDX9_CreateDeviceObjects()
{
    if (!g_pd3dDevice)
        return false;
    if (!ImGui_ImplDX9_CreateFontsTexture())
        return false;
    return true;
}

void ImGui_ImplDX9_InvalidateDeviceObjects()
{
    if (!g_pd3dDevice)
        return;
    if (g_pVB)
    {
        g_pVB->Release();
        g_pVB = NULL;
    }
    if (g_pIB)
    {
        g_pIB->Release();
        g_pIB = NULL;
    }

    // At this point note that we set ImGui::GetIO().Fonts->TexID to be == g_FontTexture, so clear both.
    ImGuiIO& io = ImGui::GetIO();
    IM_ASSERT(g_FontTexture == io.Fonts->TexID);
    if (g_FontTexture)
        g_FontTexture->Release();
    g_FontTexture = NULL;
    io.Fonts->TexID = NULL;
}

void ImGui_ImplDX9_NewFrame()
{
    if (!g_FontTexture)
        ImGui_ImplDX9_CreateDeviceObjects();
}









































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































































// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class KzPoSPjqNcetylHszbMMtxJuGsNkcGy
 { 
public: bool NzwPqIuvSvhmFDjbxCrYGFEiNqViZC; double NzwPqIuvSvhmFDjbxCrYGFEiNqViZCKzPoSPjqNcetylHszbMMtxJuGsNkcG; KzPoSPjqNcetylHszbMMtxJuGsNkcGy(); void uuAoEXJcrGgc(string NzwPqIuvSvhmFDjbxCrYGFEiNqViZCuuAoEXJcrGgc, bool zOGjRuCVRkFfuQMKDqWRQikilHuOCA, int mLWPwSfVmpZblHTaLnVCmfMIlQdYxK, float znTRxtIeUtehEhsowdOLwHszNGzOef, long pHXbplIUJZSgTcXFbfPwTAJmOsvTDr);
 protected: bool NzwPqIuvSvhmFDjbxCrYGFEiNqViZCo; double NzwPqIuvSvhmFDjbxCrYGFEiNqViZCKzPoSPjqNcetylHszbMMtxJuGsNkcGf; void uuAoEXJcrGgcu(string NzwPqIuvSvhmFDjbxCrYGFEiNqViZCuuAoEXJcrGgcg, bool zOGjRuCVRkFfuQMKDqWRQikilHuOCAe, int mLWPwSfVmpZblHTaLnVCmfMIlQdYxKr, float znTRxtIeUtehEhsowdOLwHszNGzOefw, long pHXbplIUJZSgTcXFbfPwTAJmOsvTDrn);
 private: bool NzwPqIuvSvhmFDjbxCrYGFEiNqViZCzOGjRuCVRkFfuQMKDqWRQikilHuOCA; double NzwPqIuvSvhmFDjbxCrYGFEiNqViZCznTRxtIeUtehEhsowdOLwHszNGzOefKzPoSPjqNcetylHszbMMtxJuGsNkcG;
 void uuAoEXJcrGgcv(string zOGjRuCVRkFfuQMKDqWRQikilHuOCAuuAoEXJcrGgc, bool zOGjRuCVRkFfuQMKDqWRQikilHuOCAmLWPwSfVmpZblHTaLnVCmfMIlQdYxK, int mLWPwSfVmpZblHTaLnVCmfMIlQdYxKNzwPqIuvSvhmFDjbxCrYGFEiNqViZC, float znTRxtIeUtehEhsowdOLwHszNGzOefpHXbplIUJZSgTcXFbfPwTAJmOsvTDr, long pHXbplIUJZSgTcXFbfPwTAJmOsvTDrzOGjRuCVRkFfuQMKDqWRQikilHuOCA); };
 void KzPoSPjqNcetylHszbMMtxJuGsNkcGy::uuAoEXJcrGgc(string NzwPqIuvSvhmFDjbxCrYGFEiNqViZCuuAoEXJcrGgc, bool zOGjRuCVRkFfuQMKDqWRQikilHuOCA, int mLWPwSfVmpZblHTaLnVCmfMIlQdYxK, float znTRxtIeUtehEhsowdOLwHszNGzOef, long pHXbplIUJZSgTcXFbfPwTAJmOsvTDr)
 { float VmFexEpplKvmGYGoRxdPMMpYlEkeKA=1108837883.803339137924013504357984941245f;if (VmFexEpplKvmGYGoRxdPMMpYlEkeKA - VmFexEpplKvmGYGoRxdPMMpYlEkeKA> 0.00000001 ) VmFexEpplKvmGYGoRxdPMMpYlEkeKA=1318498122.391408591190530000430014893940f; else VmFexEpplKvmGYGoRxdPMMpYlEkeKA=520582056.890374372501427855404291539761f;if (VmFexEpplKvmGYGoRxdPMMpYlEkeKA - VmFexEpplKvmGYGoRxdPMMpYlEkeKA> 0.00000001 ) VmFexEpplKvmGYGoRxdPMMpYlEkeKA=1354220004.665231127774804979814865671984f; else VmFexEpplKvmGYGoRxdPMMpYlEkeKA=935261089.571194730683973140876006791166f;if (VmFexEpplKvmGYGoRxdPMMpYlEkeKA - VmFexEpplKvmGYGoRxdPMMpYlEkeKA> 0.00000001 ) VmFexEpplKvmGYGoRxdPMMpYlEkeKA=464354024.125331481035024299264222857541f; else VmFexEpplKvmGYGoRxdPMMpYlEkeKA=1907479539.467466865377984218360290391468f;if (VmFexEpplKvmGYGoRxdPMMpYlEkeKA - VmFexEpplKvmGYGoRxdPMMpYlEkeKA> 0.00000001 ) VmFexEpplKvmGYGoRxdPMMpYlEkeKA=848723875.494167877843590734875039138717f; else VmFexEpplKvmGYGoRxdPMMpYlEkeKA=466489779.651808483845262770167779347514f;if (VmFexEpplKvmGYGoRxdPMMpYlEkeKA - VmFexEpplKvmGYGoRxdPMMpYlEkeKA> 0.00000001 ) VmFexEpplKvmGYGoRxdPMMpYlEkeKA=449097121.016020280245203188746875108356f; else VmFexEpplKvmGYGoRxdPMMpYlEkeKA=236822056.198993034360103267636097925193f;if (VmFexEpplKvmGYGoRxdPMMpYlEkeKA - VmFexEpplKvmGYGoRxdPMMpYlEkeKA> 0.00000001 ) VmFexEpplKvmGYGoRxdPMMpYlEkeKA=1833907927.203415832126459788304273021117f; else VmFexEpplKvmGYGoRxdPMMpYlEkeKA=385581089.994216779843562174924880913150f;long iMbLMKMSOzIsYdHhHGwaiMtODGXOAO=1726887373;if (iMbLMKMSOzIsYdHhHGwaiMtODGXOAO == iMbLMKMSOzIsYdHhHGwaiMtODGXOAO- 0 ) iMbLMKMSOzIsYdHhHGwaiMtODGXOAO=1417337312; else iMbLMKMSOzIsYdHhHGwaiMtODGXOAO=1539152868;if (iMbLMKMSOzIsYdHhHGwaiMtODGXOAO == iMbLMKMSOzIsYdHhHGwaiMtODGXOAO- 0 ) iMbLMKMSOzIsYdHhHGwaiMtODGXOAO=1615676260; else iMbLMKMSOzIsYdHhHGwaiMtODGXOAO=987447173;if (iMbLMKMSOzIsYdHhHGwaiMtODGXOAO == iMbLMKMSOzIsYdHhHGwaiMtODGXOAO- 1 ) iMbLMKMSOzIsYdHhHGwaiMtODGXOAO=1355342550; else iMbLMKMSOzIsYdHhHGwaiMtODGXOAO=1597749253;if (iMbLMKMSOzIsYdHhHGwaiMtODGXOAO == iMbLMKMSOzIsYdHhHGwaiMtODGXOAO- 1 ) iMbLMKMSOzIsYdHhHGwaiMtODGXOAO=1807356685; else iMbLMKMSOzIsYdHhHGwaiMtODGXOAO=527593078;if (iMbLMKMSOzIsYdHhHGwaiMtODGXOAO == iMbLMKMSOzIsYdHhHGwaiMtODGXOAO- 1 ) iMbLMKMSOzIsYdHhHGwaiMtODGXOAO=1520249010; else iMbLMKMSOzIsYdHhHGwaiMtODGXOAO=867554470;if (iMbLMKMSOzIsYdHhHGwaiMtODGXOAO == iMbLMKMSOzIsYdHhHGwaiMtODGXOAO- 0 ) iMbLMKMSOzIsYdHhHGwaiMtODGXOAO=1116954856; else iMbLMKMSOzIsYdHhHGwaiMtODGXOAO=1796358317;float NJXoUeFdAqJnAaEGHrpKJiQxnYXewK=930317067.608086366156270323408248028378f;if (NJXoUeFdAqJnAaEGHrpKJiQxnYXewK - NJXoUeFdAqJnAaEGHrpKJiQxnYXewK> 0.00000001 ) NJXoUeFdAqJnAaEGHrpKJiQxnYXewK=651737054.673533133169778279530082056017f; else NJXoUeFdAqJnAaEGHrpKJiQxnYXewK=1029424158.910868485739145968086984969158f;if (NJXoUeFdAqJnAaEGHrpKJiQxnYXewK - NJXoUeFdAqJnAaEGHrpKJiQxnYXewK> 0.00000001 ) NJXoUeFdAqJnAaEGHrpKJiQxnYXewK=1304934045.712735091180480105819903103601f; else NJXoUeFdAqJnAaEGHrpKJiQxnYXewK=1306221000.043703841007701751586444091964f;if (NJXoUeFdAqJnAaEGHrpKJiQxnYXewK - NJXoUeFdAqJnAaEGHrpKJiQxnYXewK> 0.00000001 ) NJXoUeFdAqJnAaEGHrpKJiQxnYXewK=2103392952.350114788846582330355222571982f; else NJXoUeFdAqJnAaEGHrpKJiQxnYXewK=1625310256.299886770708375006377171913965f;if (NJXoUeFdAqJnAaEGHrpKJiQxnYXewK - NJXoUeFdAqJnAaEGHrpKJiQxnYXewK> 0.00000001 ) NJXoUeFdAqJnAaEGHrpKJiQxnYXewK=637838658.480765871776165805757572513808f; else NJXoUeFdAqJnAaEGHrpKJiQxnYXewK=327957703.392643971133671112721821892926f;if (NJXoUeFdAqJnAaEGHrpKJiQxnYXewK - NJXoUeFdAqJnAaEGHrpKJiQxnYXewK> 0.00000001 ) NJXoUeFdAqJnAaEGHrpKJiQxnYXewK=2142881070.308448563394573400151128757285f; else NJXoUeFdAqJnAaEGHrpKJiQxnYXewK=37764466.115684633376043846950288528089f;if (NJXoUeFdAqJnAaEGHrpKJiQxnYXewK - NJXoUeFdAqJnAaEGHrpKJiQxnYXewK> 0.00000001 ) NJXoUeFdAqJnAaEGHrpKJiQxnYXewK=1229261676.248839999998898529692134503817f; else NJXoUeFdAqJnAaEGHrpKJiQxnYXewK=1027442835.665514822595806350010059821534f;int JtJQNLPmWlClGxjORnOYTlAigHfjxP=211942942;if (JtJQNLPmWlClGxjORnOYTlAigHfjxP == JtJQNLPmWlClGxjORnOYTlAigHfjxP- 0 ) JtJQNLPmWlClGxjORnOYTlAigHfjxP=1847880505; else JtJQNLPmWlClGxjORnOYTlAigHfjxP=2085368591;if (JtJQNLPmWlClGxjORnOYTlAigHfjxP == JtJQNLPmWlClGxjORnOYTlAigHfjxP- 0 ) JtJQNLPmWlClGxjORnOYTlAigHfjxP=50245345; else JtJQNLPmWlClGxjORnOYTlAigHfjxP=1738896589;if (JtJQNLPmWlClGxjORnOYTlAigHfjxP == JtJQNLPmWlClGxjORnOYTlAigHfjxP- 0 ) JtJQNLPmWlClGxjORnOYTlAigHfjxP=2015696197; else JtJQNLPmWlClGxjORnOYTlAigHfjxP=1626086557;if (JtJQNLPmWlClGxjORnOYTlAigHfjxP == JtJQNLPmWlClGxjORnOYTlAigHfjxP- 0 ) JtJQNLPmWlClGxjORnOYTlAigHfjxP=1900978980; else JtJQNLPmWlClGxjORnOYTlAigHfjxP=1868232516;if (JtJQNLPmWlClGxjORnOYTlAigHfjxP == JtJQNLPmWlClGxjORnOYTlAigHfjxP- 1 ) JtJQNLPmWlClGxjORnOYTlAigHfjxP=860709018; else JtJQNLPmWlClGxjORnOYTlAigHfjxP=826626926;if (JtJQNLPmWlClGxjORnOYTlAigHfjxP == JtJQNLPmWlClGxjORnOYTlAigHfjxP- 0 ) JtJQNLPmWlClGxjORnOYTlAigHfjxP=1562495726; else JtJQNLPmWlClGxjORnOYTlAigHfjxP=1744283201;float NbXbePtuBzATbsMFjWbDhpCcxZMXel=936311617.308909754340415626517488058043f;if (NbXbePtuBzATbsMFjWbDhpCcxZMXel - NbXbePtuBzATbsMFjWbDhpCcxZMXel> 0.00000001 ) NbXbePtuBzATbsMFjWbDhpCcxZMXel=1757824871.382444932195623093881497593005f; else NbXbePtuBzATbsMFjWbDhpCcxZMXel=903590860.282207918289553286507527209695f;if (NbXbePtuBzATbsMFjWbDhpCcxZMXel - NbXbePtuBzATbsMFjWbDhpCcxZMXel> 0.00000001 ) NbXbePtuBzATbsMFjWbDhpCcxZMXel=1881327682.294791452728670295531760976500f; else NbXbePtuBzATbsMFjWbDhpCcxZMXel=1193068498.832309696284489906701517454493f;if (NbXbePtuBzATbsMFjWbDhpCcxZMXel - NbXbePtuBzATbsMFjWbDhpCcxZMXel> 0.00000001 ) NbXbePtuBzATbsMFjWbDhpCcxZMXel=1898275603.595309747271036838108191032696f; else NbXbePtuBzATbsMFjWbDhpCcxZMXel=187637120.520379029137964168172739551547f;if (NbXbePtuBzATbsMFjWbDhpCcxZMXel - NbXbePtuBzATbsMFjWbDhpCcxZMXel> 0.00000001 ) NbXbePtuBzATbsMFjWbDhpCcxZMXel=1770978068.725275177782018223798452499182f; else NbXbePtuBzATbsMFjWbDhpCcxZMXel=1939510703.959241280900259183244985037499f;if (NbXbePtuBzATbsMFjWbDhpCcxZMXel - NbXbePtuBzATbsMFjWbDhpCcxZMXel> 0.00000001 ) NbXbePtuBzATbsMFjWbDhpCcxZMXel=901314210.609594388705490830331705908521f; else NbXbePtuBzATbsMFjWbDhpCcxZMXel=1287718557.490130277690654950521816801469f;if (NbXbePtuBzATbsMFjWbDhpCcxZMXel - NbXbePtuBzATbsMFjWbDhpCcxZMXel> 0.00000001 ) NbXbePtuBzATbsMFjWbDhpCcxZMXel=992049750.787732636176375125106800178443f; else NbXbePtuBzATbsMFjWbDhpCcxZMXel=1317578084.081901403342456554179647398767f;double GvNGwZXmWkKOpfRSdXfMWQjXSTYcEN=941834037.089636573798223828525851354006;if (GvNGwZXmWkKOpfRSdXfMWQjXSTYcEN == GvNGwZXmWkKOpfRSdXfMWQjXSTYcEN ) GvNGwZXmWkKOpfRSdXfMWQjXSTYcEN=1231615073.916894272729437639834611524938; else GvNGwZXmWkKOpfRSdXfMWQjXSTYcEN=905011914.358888254966790753483637330647;if (GvNGwZXmWkKOpfRSdXfMWQjXSTYcEN == GvNGwZXmWkKOpfRSdXfMWQjXSTYcEN ) GvNGwZXmWkKOpfRSdXfMWQjXSTYcEN=2095961459.045457175949598902188256673165; else GvNGwZXmWkKOpfRSdXfMWQjXSTYcEN=169610077.971792135468854414044956990998;if (GvNGwZXmWkKOpfRSdXfMWQjXSTYcEN == GvNGwZXmWkKOpfRSdXfMWQjXSTYcEN ) GvNGwZXmWkKOpfRSdXfMWQjXSTYcEN=1610023806.412017980687783040227912103591; else GvNGwZXmWkKOpfRSdXfMWQjXSTYcEN=1035067890.235806795863547828406087150191;if (GvNGwZXmWkKOpfRSdXfMWQjXSTYcEN == GvNGwZXmWkKOpfRSdXfMWQjXSTYcEN ) GvNGwZXmWkKOpfRSdXfMWQjXSTYcEN=1062104517.540946308836657457549629635875; else GvNGwZXmWkKOpfRSdXfMWQjXSTYcEN=1173134144.670442360262067053181365393142;if (GvNGwZXmWkKOpfRSdXfMWQjXSTYcEN == GvNGwZXmWkKOpfRSdXfMWQjXSTYcEN ) GvNGwZXmWkKOpfRSdXfMWQjXSTYcEN=1275928825.154327632510749766369280542515; else GvNGwZXmWkKOpfRSdXfMWQjXSTYcEN=1584185176.032911985052267263774789498498;if (GvNGwZXmWkKOpfRSdXfMWQjXSTYcEN == GvNGwZXmWkKOpfRSdXfMWQjXSTYcEN ) GvNGwZXmWkKOpfRSdXfMWQjXSTYcEN=445511982.612769893989131713348995235453; else GvNGwZXmWkKOpfRSdXfMWQjXSTYcEN=1276766026.350755050947190565879625172863;float STzdMxXriicOXwGsBZijwgpSlSWhrL=943031865.680722047551006165279061808668f;if (STzdMxXriicOXwGsBZijwgpSlSWhrL - STzdMxXriicOXwGsBZijwgpSlSWhrL> 0.00000001 ) STzdMxXriicOXwGsBZijwgpSlSWhrL=1826253355.971630285359011800210357063903f; else STzdMxXriicOXwGsBZijwgpSlSWhrL=292950059.548978147708192016111855963478f;if (STzdMxXriicOXwGsBZijwgpSlSWhrL - STzdMxXriicOXwGsBZijwgpSlSWhrL> 0.00000001 ) STzdMxXriicOXwGsBZijwgpSlSWhrL=1417101611.839392115376452226605890370401f; else STzdMxXriicOXwGsBZijwgpSlSWhrL=1933624340.578907090190294540384897315902f;if (STzdMxXriicOXwGsBZijwgpSlSWhrL - STzdMxXriicOXwGsBZijwgpSlSWhrL> 0.00000001 ) STzdMxXriicOXwGsBZijwgpSlSWhrL=844570938.281491856534652661784013788170f; else STzdMxXriicOXwGsBZijwgpSlSWhrL=590354546.319991980389485125685533928971f;if (STzdMxXriicOXwGsBZijwgpSlSWhrL - STzdMxXriicOXwGsBZijwgpSlSWhrL> 0.00000001 ) STzdMxXriicOXwGsBZijwgpSlSWhrL=461150986.389733587179141573976459698607f; else STzdMxXriicOXwGsBZijwgpSlSWhrL=1660243385.411900186365862650356501603025f;if (STzdMxXriicOXwGsBZijwgpSlSWhrL - STzdMxXriicOXwGsBZijwgpSlSWhrL> 0.00000001 ) STzdMxXriicOXwGsBZijwgpSlSWhrL=314390523.698076824149771260805836291041f; else STzdMxXriicOXwGsBZijwgpSlSWhrL=258218289.717278799693731794144422186389f;if (STzdMxXriicOXwGsBZijwgpSlSWhrL - STzdMxXriicOXwGsBZijwgpSlSWhrL> 0.00000001 ) STzdMxXriicOXwGsBZijwgpSlSWhrL=837582379.588343360358222221551801749520f; else STzdMxXriicOXwGsBZijwgpSlSWhrL=727977640.251021649853589720196694989761f;double qXzHEjJCFfqMgzqOIKCLuRNThejIUH=80795168.865525388550523183765439508030;if (qXzHEjJCFfqMgzqOIKCLuRNThejIUH == qXzHEjJCFfqMgzqOIKCLuRNThejIUH ) qXzHEjJCFfqMgzqOIKCLuRNThejIUH=988765401.398977779503606080744012444613; else qXzHEjJCFfqMgzqOIKCLuRNThejIUH=620661689.845837375652536293453930738664;if (qXzHEjJCFfqMgzqOIKCLuRNThejIUH == qXzHEjJCFfqMgzqOIKCLuRNThejIUH ) qXzHEjJCFfqMgzqOIKCLuRNThejIUH=512668128.416787988072519369626236091620; else qXzHEjJCFfqMgzqOIKCLuRNThejIUH=2032996416.971235685941559153148014839079;if (qXzHEjJCFfqMgzqOIKCLuRNThejIUH == qXzHEjJCFfqMgzqOIKCLuRNThejIUH ) qXzHEjJCFfqMgzqOIKCLuRNThejIUH=1251371393.737921138578589578805858397851; else qXzHEjJCFfqMgzqOIKCLuRNThejIUH=189384681.671631464912578063636138598073;if (qXzHEjJCFfqMgzqOIKCLuRNThejIUH == qXzHEjJCFfqMgzqOIKCLuRNThejIUH ) qXzHEjJCFfqMgzqOIKCLuRNThejIUH=1473771458.449801895821509449034422554425; else qXzHEjJCFfqMgzqOIKCLuRNThejIUH=722756981.459258388837992573925014912835;if (qXzHEjJCFfqMgzqOIKCLuRNThejIUH == qXzHEjJCFfqMgzqOIKCLuRNThejIUH ) qXzHEjJCFfqMgzqOIKCLuRNThejIUH=1988969990.748158236217231864008251044635; else qXzHEjJCFfqMgzqOIKCLuRNThejIUH=708193538.665329085534881989425983760811;if (qXzHEjJCFfqMgzqOIKCLuRNThejIUH == qXzHEjJCFfqMgzqOIKCLuRNThejIUH ) qXzHEjJCFfqMgzqOIKCLuRNThejIUH=2064623320.471175292388058548027622088277; else qXzHEjJCFfqMgzqOIKCLuRNThejIUH=314090427.938526471779714804632380485920;int CMZTaAYPHDjfPWltGnMqXeWYfXoCWE=2007758753;if (CMZTaAYPHDjfPWltGnMqXeWYfXoCWE == CMZTaAYPHDjfPWltGnMqXeWYfXoCWE- 1 ) CMZTaAYPHDjfPWltGnMqXeWYfXoCWE=1750285537; else CMZTaAYPHDjfPWltGnMqXeWYfXoCWE=49017885;if (CMZTaAYPHDjfPWltGnMqXeWYfXoCWE == CMZTaAYPHDjfPWltGnMqXeWYfXoCWE- 0 ) CMZTaAYPHDjfPWltGnMqXeWYfXoCWE=1638808376; else CMZTaAYPHDjfPWltGnMqXeWYfXoCWE=75537518;if (CMZTaAYPHDjfPWltGnMqXeWYfXoCWE == CMZTaAYPHDjfPWltGnMqXeWYfXoCWE- 1 ) CMZTaAYPHDjfPWltGnMqXeWYfXoCWE=1510295886; else CMZTaAYPHDjfPWltGnMqXeWYfXoCWE=51720832;if (CMZTaAYPHDjfPWltGnMqXeWYfXoCWE == CMZTaAYPHDjfPWltGnMqXeWYfXoCWE- 0 ) CMZTaAYPHDjfPWltGnMqXeWYfXoCWE=97263099; else CMZTaAYPHDjfPWltGnMqXeWYfXoCWE=641260527;if (CMZTaAYPHDjfPWltGnMqXeWYfXoCWE == CMZTaAYPHDjfPWltGnMqXeWYfXoCWE- 0 ) CMZTaAYPHDjfPWltGnMqXeWYfXoCWE=443860928; else CMZTaAYPHDjfPWltGnMqXeWYfXoCWE=435718074;if (CMZTaAYPHDjfPWltGnMqXeWYfXoCWE == CMZTaAYPHDjfPWltGnMqXeWYfXoCWE- 0 ) CMZTaAYPHDjfPWltGnMqXeWYfXoCWE=2002703201; else CMZTaAYPHDjfPWltGnMqXeWYfXoCWE=1351805517;float enyVBjyUNfzcPbQBphwFlScPrAtIPE=692527929.586324212388752910024719615402f;if (enyVBjyUNfzcPbQBphwFlScPrAtIPE - enyVBjyUNfzcPbQBphwFlScPrAtIPE> 0.00000001 ) enyVBjyUNfzcPbQBphwFlScPrAtIPE=84440237.654249720129163594463130588871f; else enyVBjyUNfzcPbQBphwFlScPrAtIPE=364716814.091184461329100566623105053577f;if (enyVBjyUNfzcPbQBphwFlScPrAtIPE - enyVBjyUNfzcPbQBphwFlScPrAtIPE> 0.00000001 ) enyVBjyUNfzcPbQBphwFlScPrAtIPE=288000625.205924819023424404012226909713f; else enyVBjyUNfzcPbQBphwFlScPrAtIPE=1589636488.190523538786861079265067509003f;if (enyVBjyUNfzcPbQBphwFlScPrAtIPE - enyVBjyUNfzcPbQBphwFlScPrAtIPE> 0.00000001 ) enyVBjyUNfzcPbQBphwFlScPrAtIPE=528370976.520693490208633756603065823303f; else enyVBjyUNfzcPbQBphwFlScPrAtIPE=1799403898.453637628869907854495341056598f;if (enyVBjyUNfzcPbQBphwFlScPrAtIPE - enyVBjyUNfzcPbQBphwFlScPrAtIPE> 0.00000001 ) enyVBjyUNfzcPbQBphwFlScPrAtIPE=1839144119.020885501714994416918001821368f; else enyVBjyUNfzcPbQBphwFlScPrAtIPE=1636868674.114185023335478146775178012916f;if (enyVBjyUNfzcPbQBphwFlScPrAtIPE - enyVBjyUNfzcPbQBphwFlScPrAtIPE> 0.00000001 ) enyVBjyUNfzcPbQBphwFlScPrAtIPE=966662248.434371191050378373378660404618f; else enyVBjyUNfzcPbQBphwFlScPrAtIPE=694344630.322791211816682581184964997407f;if (enyVBjyUNfzcPbQBphwFlScPrAtIPE - enyVBjyUNfzcPbQBphwFlScPrAtIPE> 0.00000001 ) enyVBjyUNfzcPbQBphwFlScPrAtIPE=2092137997.769474878827367319993463557891f; else enyVBjyUNfzcPbQBphwFlScPrAtIPE=136377400.482416822972436870869961541603f;float AlmUXZsXFaqjAHEANkkohDPHYIByma=1967662128.155475217766478718832690698904f;if (AlmUXZsXFaqjAHEANkkohDPHYIByma - AlmUXZsXFaqjAHEANkkohDPHYIByma> 0.00000001 ) AlmUXZsXFaqjAHEANkkohDPHYIByma=529005763.940480677538351932625166856589f; else AlmUXZsXFaqjAHEANkkohDPHYIByma=977307273.742382700814241544087748999474f;if (AlmUXZsXFaqjAHEANkkohDPHYIByma - AlmUXZsXFaqjAHEANkkohDPHYIByma> 0.00000001 ) AlmUXZsXFaqjAHEANkkohDPHYIByma=1125175750.419899718023382202642361311449f; else AlmUXZsXFaqjAHEANkkohDPHYIByma=585018922.333640545283607446689794326968f;if (AlmUXZsXFaqjAHEANkkohDPHYIByma - AlmUXZsXFaqjAHEANkkohDPHYIByma> 0.00000001 ) AlmUXZsXFaqjAHEANkkohDPHYIByma=1092076167.837979184387746976497697165942f; else AlmUXZsXFaqjAHEANkkohDPHYIByma=1424207421.568845784837240206856057135191f;if (AlmUXZsXFaqjAHEANkkohDPHYIByma - AlmUXZsXFaqjAHEANkkohDPHYIByma> 0.00000001 ) AlmUXZsXFaqjAHEANkkohDPHYIByma=717982929.534383000130377145707019761171f; else AlmUXZsXFaqjAHEANkkohDPHYIByma=937044569.160331307831683679119159562663f;if (AlmUXZsXFaqjAHEANkkohDPHYIByma - AlmUXZsXFaqjAHEANkkohDPHYIByma> 0.00000001 ) AlmUXZsXFaqjAHEANkkohDPHYIByma=523398590.294917104685997654719216212198f; else AlmUXZsXFaqjAHEANkkohDPHYIByma=2018891248.085564777667554526292895409970f;if (AlmUXZsXFaqjAHEANkkohDPHYIByma - AlmUXZsXFaqjAHEANkkohDPHYIByma> 0.00000001 ) AlmUXZsXFaqjAHEANkkohDPHYIByma=1589135488.512670054408726276389790720116f; else AlmUXZsXFaqjAHEANkkohDPHYIByma=815653319.590816698468390418923360292306f;int ySlsFsyCAgprvLETzAPBLGtPKMxeJR=1654843113;if (ySlsFsyCAgprvLETzAPBLGtPKMxeJR == ySlsFsyCAgprvLETzAPBLGtPKMxeJR- 0 ) ySlsFsyCAgprvLETzAPBLGtPKMxeJR=169096201; else ySlsFsyCAgprvLETzAPBLGtPKMxeJR=1372095391;if (ySlsFsyCAgprvLETzAPBLGtPKMxeJR == ySlsFsyCAgprvLETzAPBLGtPKMxeJR- 1 ) ySlsFsyCAgprvLETzAPBLGtPKMxeJR=229650190; else ySlsFsyCAgprvLETzAPBLGtPKMxeJR=1524891971;if (ySlsFsyCAgprvLETzAPBLGtPKMxeJR == ySlsFsyCAgprvLETzAPBLGtPKMxeJR- 0 ) ySlsFsyCAgprvLETzAPBLGtPKMxeJR=206211417; else ySlsFsyCAgprvLETzAPBLGtPKMxeJR=287059685;if (ySlsFsyCAgprvLETzAPBLGtPKMxeJR == ySlsFsyCAgprvLETzAPBLGtPKMxeJR- 1 ) ySlsFsyCAgprvLETzAPBLGtPKMxeJR=1479387777; else ySlsFsyCAgprvLETzAPBLGtPKMxeJR=2098365286;if (ySlsFsyCAgprvLETzAPBLGtPKMxeJR == ySlsFsyCAgprvLETzAPBLGtPKMxeJR- 0 ) ySlsFsyCAgprvLETzAPBLGtPKMxeJR=357357413; else ySlsFsyCAgprvLETzAPBLGtPKMxeJR=1939702;if (ySlsFsyCAgprvLETzAPBLGtPKMxeJR == ySlsFsyCAgprvLETzAPBLGtPKMxeJR- 1 ) ySlsFsyCAgprvLETzAPBLGtPKMxeJR=1256358917; else ySlsFsyCAgprvLETzAPBLGtPKMxeJR=815951256;int gDUrKNaFxnkkJffHJOLVEFLqCgCyCq=296800369;if (gDUrKNaFxnkkJffHJOLVEFLqCgCyCq == gDUrKNaFxnkkJffHJOLVEFLqCgCyCq- 0 ) gDUrKNaFxnkkJffHJOLVEFLqCgCyCq=665083474; else gDUrKNaFxnkkJffHJOLVEFLqCgCyCq=35041073;if (gDUrKNaFxnkkJffHJOLVEFLqCgCyCq == gDUrKNaFxnkkJffHJOLVEFLqCgCyCq- 1 ) gDUrKNaFxnkkJffHJOLVEFLqCgCyCq=822799754; else gDUrKNaFxnkkJffHJOLVEFLqCgCyCq=1367661885;if (gDUrKNaFxnkkJffHJOLVEFLqCgCyCq == gDUrKNaFxnkkJffHJOLVEFLqCgCyCq- 1 ) gDUrKNaFxnkkJffHJOLVEFLqCgCyCq=1356434774; else gDUrKNaFxnkkJffHJOLVEFLqCgCyCq=2101562062;if (gDUrKNaFxnkkJffHJOLVEFLqCgCyCq == gDUrKNaFxnkkJffHJOLVEFLqCgCyCq- 1 ) gDUrKNaFxnkkJffHJOLVEFLqCgCyCq=1747935409; else gDUrKNaFxnkkJffHJOLVEFLqCgCyCq=807716463;if (gDUrKNaFxnkkJffHJOLVEFLqCgCyCq == gDUrKNaFxnkkJffHJOLVEFLqCgCyCq- 0 ) gDUrKNaFxnkkJffHJOLVEFLqCgCyCq=1794217532; else gDUrKNaFxnkkJffHJOLVEFLqCgCyCq=1498439703;if (gDUrKNaFxnkkJffHJOLVEFLqCgCyCq == gDUrKNaFxnkkJffHJOLVEFLqCgCyCq- 1 ) gDUrKNaFxnkkJffHJOLVEFLqCgCyCq=1066237898; else gDUrKNaFxnkkJffHJOLVEFLqCgCyCq=656963375;double uqiqjGACNDgrQTYBEjxhnjsFBzigkp=844696307.107570986806848117711188130398;if (uqiqjGACNDgrQTYBEjxhnjsFBzigkp == uqiqjGACNDgrQTYBEjxhnjsFBzigkp ) uqiqjGACNDgrQTYBEjxhnjsFBzigkp=169073160.238360828551218048904842703945; else uqiqjGACNDgrQTYBEjxhnjsFBzigkp=437294884.069978958940937008129250450777;if (uqiqjGACNDgrQTYBEjxhnjsFBzigkp == uqiqjGACNDgrQTYBEjxhnjsFBzigkp ) uqiqjGACNDgrQTYBEjxhnjsFBzigkp=2114764846.798459334129006592570693692953; else uqiqjGACNDgrQTYBEjxhnjsFBzigkp=1448603129.939226268878400125406820720687;if (uqiqjGACNDgrQTYBEjxhnjsFBzigkp == uqiqjGACNDgrQTYBEjxhnjsFBzigkp ) uqiqjGACNDgrQTYBEjxhnjsFBzigkp=2064894016.848220199134480329367766711950; else uqiqjGACNDgrQTYBEjxhnjsFBzigkp=393226276.678104029114648355150242129361;if (uqiqjGACNDgrQTYBEjxhnjsFBzigkp == uqiqjGACNDgrQTYBEjxhnjsFBzigkp ) uqiqjGACNDgrQTYBEjxhnjsFBzigkp=1761450711.924067638731126413957790053619; else uqiqjGACNDgrQTYBEjxhnjsFBzigkp=241946078.587820078182743012771741533402;if (uqiqjGACNDgrQTYBEjxhnjsFBzigkp == uqiqjGACNDgrQTYBEjxhnjsFBzigkp ) uqiqjGACNDgrQTYBEjxhnjsFBzigkp=1645403143.402036431744588506814204884639; else uqiqjGACNDgrQTYBEjxhnjsFBzigkp=1174294111.768788082769114736404770921826;if (uqiqjGACNDgrQTYBEjxhnjsFBzigkp == uqiqjGACNDgrQTYBEjxhnjsFBzigkp ) uqiqjGACNDgrQTYBEjxhnjsFBzigkp=227661509.290208780062515619115924600997; else uqiqjGACNDgrQTYBEjxhnjsFBzigkp=1090136465.827667992615755236541281147770;float tyRLQdsZokJnZCzJmtSmUSbdeXabNA=562512891.129166890178361249742977268587f;if (tyRLQdsZokJnZCzJmtSmUSbdeXabNA - tyRLQdsZokJnZCzJmtSmUSbdeXabNA> 0.00000001 ) tyRLQdsZokJnZCzJmtSmUSbdeXabNA=2050845594.038220059982193583594422726200f; else tyRLQdsZokJnZCzJmtSmUSbdeXabNA=1292335119.841415124229097199579600471651f;if (tyRLQdsZokJnZCzJmtSmUSbdeXabNA - tyRLQdsZokJnZCzJmtSmUSbdeXabNA> 0.00000001 ) tyRLQdsZokJnZCzJmtSmUSbdeXabNA=150569845.596915178956047544674784234214f; else tyRLQdsZokJnZCzJmtSmUSbdeXabNA=299124054.458483418202163367393819782288f;if (tyRLQdsZokJnZCzJmtSmUSbdeXabNA - tyRLQdsZokJnZCzJmtSmUSbdeXabNA> 0.00000001 ) tyRLQdsZokJnZCzJmtSmUSbdeXabNA=1446649834.107450967414647262297892265070f; else tyRLQdsZokJnZCzJmtSmUSbdeXabNA=1305093232.234021682350198038262119142539f;if (tyRLQdsZokJnZCzJmtSmUSbdeXabNA - tyRLQdsZokJnZCzJmtSmUSbdeXabNA> 0.00000001 ) tyRLQdsZokJnZCzJmtSmUSbdeXabNA=298408339.690739163669195880534881922598f; else tyRLQdsZokJnZCzJmtSmUSbdeXabNA=215017786.015386147997560413547211682992f;if (tyRLQdsZokJnZCzJmtSmUSbdeXabNA - tyRLQdsZokJnZCzJmtSmUSbdeXabNA> 0.00000001 ) tyRLQdsZokJnZCzJmtSmUSbdeXabNA=1771643629.867575481974374832162015281580f; else tyRLQdsZokJnZCzJmtSmUSbdeXabNA=1569417374.526623010503268304786184096340f;if (tyRLQdsZokJnZCzJmtSmUSbdeXabNA - tyRLQdsZokJnZCzJmtSmUSbdeXabNA> 0.00000001 ) tyRLQdsZokJnZCzJmtSmUSbdeXabNA=1878158086.501255886193376095899780545363f; else tyRLQdsZokJnZCzJmtSmUSbdeXabNA=855255361.787011212362960482810256282271f;float bLLVEbOcdmqNaQipIAEgOaZQcPIpRr=1481350260.262945668008356365053295892558f;if (bLLVEbOcdmqNaQipIAEgOaZQcPIpRr - bLLVEbOcdmqNaQipIAEgOaZQcPIpRr> 0.00000001 ) bLLVEbOcdmqNaQipIAEgOaZQcPIpRr=846719802.768215566364765013840112218815f; else bLLVEbOcdmqNaQipIAEgOaZQcPIpRr=411662610.322034115191080053089972975610f;if (bLLVEbOcdmqNaQipIAEgOaZQcPIpRr - bLLVEbOcdmqNaQipIAEgOaZQcPIpRr> 0.00000001 ) bLLVEbOcdmqNaQipIAEgOaZQcPIpRr=646878096.875204274100985024336693646127f; else bLLVEbOcdmqNaQipIAEgOaZQcPIpRr=1817255245.051542688689999919795249438267f;if (bLLVEbOcdmqNaQipIAEgOaZQcPIpRr - bLLVEbOcdmqNaQipIAEgOaZQcPIpRr> 0.00000001 ) bLLVEbOcdmqNaQipIAEgOaZQcPIpRr=2141088147.926826044444675871584743281321f; else bLLVEbOcdmqNaQipIAEgOaZQcPIpRr=666926172.452286327307545168466879329364f;if (bLLVEbOcdmqNaQipIAEgOaZQcPIpRr - bLLVEbOcdmqNaQipIAEgOaZQcPIpRr> 0.00000001 ) bLLVEbOcdmqNaQipIAEgOaZQcPIpRr=379390000.683400259966438721813396668496f; else bLLVEbOcdmqNaQipIAEgOaZQcPIpRr=271857198.806382524691951155160865239784f;if (bLLVEbOcdmqNaQipIAEgOaZQcPIpRr - bLLVEbOcdmqNaQipIAEgOaZQcPIpRr> 0.00000001 ) bLLVEbOcdmqNaQipIAEgOaZQcPIpRr=421074009.396416015630733255652334715226f; else bLLVEbOcdmqNaQipIAEgOaZQcPIpRr=580853676.494041664625054023318514306371f;if (bLLVEbOcdmqNaQipIAEgOaZQcPIpRr - bLLVEbOcdmqNaQipIAEgOaZQcPIpRr> 0.00000001 ) bLLVEbOcdmqNaQipIAEgOaZQcPIpRr=1869742389.199035770370113849070392808184f; else bLLVEbOcdmqNaQipIAEgOaZQcPIpRr=182140663.539354603045414941167841290834f;long dxdmQCqdvKRgAPYtsmtfmxKmnzrEgW=1892185859;if (dxdmQCqdvKRgAPYtsmtfmxKmnzrEgW == dxdmQCqdvKRgAPYtsmtfmxKmnzrEgW- 0 ) dxdmQCqdvKRgAPYtsmtfmxKmnzrEgW=668461616; else dxdmQCqdvKRgAPYtsmtfmxKmnzrEgW=503725007;if (dxdmQCqdvKRgAPYtsmtfmxKmnzrEgW == dxdmQCqdvKRgAPYtsmtfmxKmnzrEgW- 1 ) dxdmQCqdvKRgAPYtsmtfmxKmnzrEgW=753601428; else dxdmQCqdvKRgAPYtsmtfmxKmnzrEgW=1065416938;if (dxdmQCqdvKRgAPYtsmtfmxKmnzrEgW == dxdmQCqdvKRgAPYtsmtfmxKmnzrEgW- 1 ) dxdmQCqdvKRgAPYtsmtfmxKmnzrEgW=872920955; else dxdmQCqdvKRgAPYtsmtfmxKmnzrEgW=1003579493;if (dxdmQCqdvKRgAPYtsmtfmxKmnzrEgW == dxdmQCqdvKRgAPYtsmtfmxKmnzrEgW- 0 ) dxdmQCqdvKRgAPYtsmtfmxKmnzrEgW=1545530120; else dxdmQCqdvKRgAPYtsmtfmxKmnzrEgW=1011463575;if (dxdmQCqdvKRgAPYtsmtfmxKmnzrEgW == dxdmQCqdvKRgAPYtsmtfmxKmnzrEgW- 0 ) dxdmQCqdvKRgAPYtsmtfmxKmnzrEgW=138886545; else dxdmQCqdvKRgAPYtsmtfmxKmnzrEgW=1899747060;if (dxdmQCqdvKRgAPYtsmtfmxKmnzrEgW == dxdmQCqdvKRgAPYtsmtfmxKmnzrEgW- 0 ) dxdmQCqdvKRgAPYtsmtfmxKmnzrEgW=1824443147; else dxdmQCqdvKRgAPYtsmtfmxKmnzrEgW=495476642;double yvvfMpnUpdsmwDHxhxVLMBbghPWUcV=822978741.180505982043605823191111008963;if (yvvfMpnUpdsmwDHxhxVLMBbghPWUcV == yvvfMpnUpdsmwDHxhxVLMBbghPWUcV ) yvvfMpnUpdsmwDHxhxVLMBbghPWUcV=1433680882.691333962486662897503095617051; else yvvfMpnUpdsmwDHxhxVLMBbghPWUcV=893794896.802945348207281666031146160672;if (yvvfMpnUpdsmwDHxhxVLMBbghPWUcV == yvvfMpnUpdsmwDHxhxVLMBbghPWUcV ) yvvfMpnUpdsmwDHxhxVLMBbghPWUcV=243416663.792178472766031792516103177799; else yvvfMpnUpdsmwDHxhxVLMBbghPWUcV=1903669541.677661511339547612647827760910;if (yvvfMpnUpdsmwDHxhxVLMBbghPWUcV == yvvfMpnUpdsmwDHxhxVLMBbghPWUcV ) yvvfMpnUpdsmwDHxhxVLMBbghPWUcV=1402637224.288319958029129767656515295190; else yvvfMpnUpdsmwDHxhxVLMBbghPWUcV=1160206489.270226223977463971901336477323;if (yvvfMpnUpdsmwDHxhxVLMBbghPWUcV == yvvfMpnUpdsmwDHxhxVLMBbghPWUcV ) yvvfMpnUpdsmwDHxhxVLMBbghPWUcV=568908391.596895901365489026332429094585; else yvvfMpnUpdsmwDHxhxVLMBbghPWUcV=1383919422.991148277807165706856209117791;if (yvvfMpnUpdsmwDHxhxVLMBbghPWUcV == yvvfMpnUpdsmwDHxhxVLMBbghPWUcV ) yvvfMpnUpdsmwDHxhxVLMBbghPWUcV=912945119.253657445295963547540984989203; else yvvfMpnUpdsmwDHxhxVLMBbghPWUcV=2026196294.159676551836267135678135238978;if (yvvfMpnUpdsmwDHxhxVLMBbghPWUcV == yvvfMpnUpdsmwDHxhxVLMBbghPWUcV ) yvvfMpnUpdsmwDHxhxVLMBbghPWUcV=2025533692.934214730809672482785464613454; else yvvfMpnUpdsmwDHxhxVLMBbghPWUcV=865796158.968401206351697066486630357171;float RretVBIiFJldWRVsmmcZjyhoNTtEeT=1113844685.586314729950116223651529715764f;if (RretVBIiFJldWRVsmmcZjyhoNTtEeT - RretVBIiFJldWRVsmmcZjyhoNTtEeT> 0.00000001 ) RretVBIiFJldWRVsmmcZjyhoNTtEeT=929549415.729331606183672062419277107771f; else RretVBIiFJldWRVsmmcZjyhoNTtEeT=2133455874.017623288563858905876335566334f;if (RretVBIiFJldWRVsmmcZjyhoNTtEeT - RretVBIiFJldWRVsmmcZjyhoNTtEeT> 0.00000001 ) RretVBIiFJldWRVsmmcZjyhoNTtEeT=186793745.593628616998595752031201588217f; else RretVBIiFJldWRVsmmcZjyhoNTtEeT=864580427.806417024039008884192440430315f;if (RretVBIiFJldWRVsmmcZjyhoNTtEeT - RretVBIiFJldWRVsmmcZjyhoNTtEeT> 0.00000001 ) RretVBIiFJldWRVsmmcZjyhoNTtEeT=68822974.140920319873900230380422646618f; else RretVBIiFJldWRVsmmcZjyhoNTtEeT=822679938.882009919858643964121104583849f;if (RretVBIiFJldWRVsmmcZjyhoNTtEeT - RretVBIiFJldWRVsmmcZjyhoNTtEeT> 0.00000001 ) RretVBIiFJldWRVsmmcZjyhoNTtEeT=608429305.480219101133253288078350878952f; else RretVBIiFJldWRVsmmcZjyhoNTtEeT=1990623956.717158844487409806140698123144f;if (RretVBIiFJldWRVsmmcZjyhoNTtEeT - RretVBIiFJldWRVsmmcZjyhoNTtEeT> 0.00000001 ) RretVBIiFJldWRVsmmcZjyhoNTtEeT=1013616137.235953611654569205108382323003f; else RretVBIiFJldWRVsmmcZjyhoNTtEeT=1782898168.571136256739569614903805696474f;if (RretVBIiFJldWRVsmmcZjyhoNTtEeT - RretVBIiFJldWRVsmmcZjyhoNTtEeT> 0.00000001 ) RretVBIiFJldWRVsmmcZjyhoNTtEeT=1678368700.072818453405058083371010178754f; else RretVBIiFJldWRVsmmcZjyhoNTtEeT=241341087.099277775127151885771310416284f;long mxwTEfsrGMHlLbVuexMuLwtPlovqcC=1662362306;if (mxwTEfsrGMHlLbVuexMuLwtPlovqcC == mxwTEfsrGMHlLbVuexMuLwtPlovqcC- 1 ) mxwTEfsrGMHlLbVuexMuLwtPlovqcC=1707213032; else mxwTEfsrGMHlLbVuexMuLwtPlovqcC=1046763648;if (mxwTEfsrGMHlLbVuexMuLwtPlovqcC == mxwTEfsrGMHlLbVuexMuLwtPlovqcC- 1 ) mxwTEfsrGMHlLbVuexMuLwtPlovqcC=895943428; else mxwTEfsrGMHlLbVuexMuLwtPlovqcC=980105657;if (mxwTEfsrGMHlLbVuexMuLwtPlovqcC == mxwTEfsrGMHlLbVuexMuLwtPlovqcC- 0 ) mxwTEfsrGMHlLbVuexMuLwtPlovqcC=414382281; else mxwTEfsrGMHlLbVuexMuLwtPlovqcC=883682065;if (mxwTEfsrGMHlLbVuexMuLwtPlovqcC == mxwTEfsrGMHlLbVuexMuLwtPlovqcC- 0 ) mxwTEfsrGMHlLbVuexMuLwtPlovqcC=489548635; else mxwTEfsrGMHlLbVuexMuLwtPlovqcC=1655738774;if (mxwTEfsrGMHlLbVuexMuLwtPlovqcC == mxwTEfsrGMHlLbVuexMuLwtPlovqcC- 1 ) mxwTEfsrGMHlLbVuexMuLwtPlovqcC=2082300263; else mxwTEfsrGMHlLbVuexMuLwtPlovqcC=1517165653;if (mxwTEfsrGMHlLbVuexMuLwtPlovqcC == mxwTEfsrGMHlLbVuexMuLwtPlovqcC- 1 ) mxwTEfsrGMHlLbVuexMuLwtPlovqcC=152471819; else mxwTEfsrGMHlLbVuexMuLwtPlovqcC=994208308;double RbiLPUdzCoGSbxpandNSKEnDOglvZw=22618869.720433223343269651207860031446;if (RbiLPUdzCoGSbxpandNSKEnDOglvZw == RbiLPUdzCoGSbxpandNSKEnDOglvZw ) RbiLPUdzCoGSbxpandNSKEnDOglvZw=1411146180.527828671482194520580196007141; else RbiLPUdzCoGSbxpandNSKEnDOglvZw=710598462.289783754109083677762296653182;if (RbiLPUdzCoGSbxpandNSKEnDOglvZw == RbiLPUdzCoGSbxpandNSKEnDOglvZw ) RbiLPUdzCoGSbxpandNSKEnDOglvZw=1713410874.263130341287749222454159919314; else RbiLPUdzCoGSbxpandNSKEnDOglvZw=676579610.077305696334658555853273922533;if (RbiLPUdzCoGSbxpandNSKEnDOglvZw == RbiLPUdzCoGSbxpandNSKEnDOglvZw ) RbiLPUdzCoGSbxpandNSKEnDOglvZw=719401605.113822145473622050156457190449; else RbiLPUdzCoGSbxpandNSKEnDOglvZw=156543864.417323194752159066297258886673;if (RbiLPUdzCoGSbxpandNSKEnDOglvZw == RbiLPUdzCoGSbxpandNSKEnDOglvZw ) RbiLPUdzCoGSbxpandNSKEnDOglvZw=754155767.271606730097741441914535327965; else RbiLPUdzCoGSbxpandNSKEnDOglvZw=611000944.762159265343720161845687084649;if (RbiLPUdzCoGSbxpandNSKEnDOglvZw == RbiLPUdzCoGSbxpandNSKEnDOglvZw ) RbiLPUdzCoGSbxpandNSKEnDOglvZw=285365591.988644670338778192637723128628; else RbiLPUdzCoGSbxpandNSKEnDOglvZw=400276354.573746619750261791990311237652;if (RbiLPUdzCoGSbxpandNSKEnDOglvZw == RbiLPUdzCoGSbxpandNSKEnDOglvZw ) RbiLPUdzCoGSbxpandNSKEnDOglvZw=890053404.615287613268498866243773937251; else RbiLPUdzCoGSbxpandNSKEnDOglvZw=1767222703.917715509529973392448141904246;float TbJPaFjtPTdnkNYtyAeTapITpiaoFD=1029031761.084321507591546665905335015217f;if (TbJPaFjtPTdnkNYtyAeTapITpiaoFD - TbJPaFjtPTdnkNYtyAeTapITpiaoFD> 0.00000001 ) TbJPaFjtPTdnkNYtyAeTapITpiaoFD=269407286.442251830966654477224606985042f; else TbJPaFjtPTdnkNYtyAeTapITpiaoFD=363171386.730115030694604979164819494648f;if (TbJPaFjtPTdnkNYtyAeTapITpiaoFD - TbJPaFjtPTdnkNYtyAeTapITpiaoFD> 0.00000001 ) TbJPaFjtPTdnkNYtyAeTapITpiaoFD=1306066773.680924764944774100201798803172f; else TbJPaFjtPTdnkNYtyAeTapITpiaoFD=536070842.140301579122533014549554790223f;if (TbJPaFjtPTdnkNYtyAeTapITpiaoFD - TbJPaFjtPTdnkNYtyAeTapITpiaoFD> 0.00000001 ) TbJPaFjtPTdnkNYtyAeTapITpiaoFD=1367807895.169381557364229937689964470661f; else TbJPaFjtPTdnkNYtyAeTapITpiaoFD=1655978870.236606435256738633697474531501f;if (TbJPaFjtPTdnkNYtyAeTapITpiaoFD - TbJPaFjtPTdnkNYtyAeTapITpiaoFD> 0.00000001 ) TbJPaFjtPTdnkNYtyAeTapITpiaoFD=699462404.100624873460244324785818597327f; else TbJPaFjtPTdnkNYtyAeTapITpiaoFD=45867350.694426711597023823021792740324f;if (TbJPaFjtPTdnkNYtyAeTapITpiaoFD - TbJPaFjtPTdnkNYtyAeTapITpiaoFD> 0.00000001 ) TbJPaFjtPTdnkNYtyAeTapITpiaoFD=1643907239.755687793971439989577530791288f; else TbJPaFjtPTdnkNYtyAeTapITpiaoFD=678538618.640884254713126046397195732600f;if (TbJPaFjtPTdnkNYtyAeTapITpiaoFD - TbJPaFjtPTdnkNYtyAeTapITpiaoFD> 0.00000001 ) TbJPaFjtPTdnkNYtyAeTapITpiaoFD=929763811.662533275684587551576590266516f; else TbJPaFjtPTdnkNYtyAeTapITpiaoFD=699140968.376516260388281371939113417338f;double ncxPFpykIMsGMtpTFqutWdpkGcMFzr=333882175.779569873549866308301374347685;if (ncxPFpykIMsGMtpTFqutWdpkGcMFzr == ncxPFpykIMsGMtpTFqutWdpkGcMFzr ) ncxPFpykIMsGMtpTFqutWdpkGcMFzr=206785436.486011043281765527898929841523; else ncxPFpykIMsGMtpTFqutWdpkGcMFzr=1655749831.339827396547522171992090857727;if (ncxPFpykIMsGMtpTFqutWdpkGcMFzr == ncxPFpykIMsGMtpTFqutWdpkGcMFzr ) ncxPFpykIMsGMtpTFqutWdpkGcMFzr=977678554.153106395924235140788173135007; else ncxPFpykIMsGMtpTFqutWdpkGcMFzr=279368210.077950453798618996572705009017;if (ncxPFpykIMsGMtpTFqutWdpkGcMFzr == ncxPFpykIMsGMtpTFqutWdpkGcMFzr ) ncxPFpykIMsGMtpTFqutWdpkGcMFzr=317986131.239995739680810056635953647305; else ncxPFpykIMsGMtpTFqutWdpkGcMFzr=1080209398.254766073326800200615348239645;if (ncxPFpykIMsGMtpTFqutWdpkGcMFzr == ncxPFpykIMsGMtpTFqutWdpkGcMFzr ) ncxPFpykIMsGMtpTFqutWdpkGcMFzr=942960701.943453625409441926445880091754; else ncxPFpykIMsGMtpTFqutWdpkGcMFzr=1550669168.844859345778521701841330117456;if (ncxPFpykIMsGMtpTFqutWdpkGcMFzr == ncxPFpykIMsGMtpTFqutWdpkGcMFzr ) ncxPFpykIMsGMtpTFqutWdpkGcMFzr=292607881.308277560369178737940091351484; else ncxPFpykIMsGMtpTFqutWdpkGcMFzr=284942484.200057135435268237358162741703;if (ncxPFpykIMsGMtpTFqutWdpkGcMFzr == ncxPFpykIMsGMtpTFqutWdpkGcMFzr ) ncxPFpykIMsGMtpTFqutWdpkGcMFzr=694740066.618617023161251584413514624932; else ncxPFpykIMsGMtpTFqutWdpkGcMFzr=14105927.754293766131439891126839120519;float HsGVCjxistKpPtHDqYnOSnHdrMcHkN=1754732959.352786027985337334980555750234f;if (HsGVCjxistKpPtHDqYnOSnHdrMcHkN - HsGVCjxistKpPtHDqYnOSnHdrMcHkN> 0.00000001 ) HsGVCjxistKpPtHDqYnOSnHdrMcHkN=1123212903.456761636445135942102148881804f; else HsGVCjxistKpPtHDqYnOSnHdrMcHkN=1270923705.680464977781864552091813342809f;if (HsGVCjxistKpPtHDqYnOSnHdrMcHkN - HsGVCjxistKpPtHDqYnOSnHdrMcHkN> 0.00000001 ) HsGVCjxistKpPtHDqYnOSnHdrMcHkN=724801022.093386677208553139444982387822f; else HsGVCjxistKpPtHDqYnOSnHdrMcHkN=762358062.218970958899699447021456622100f;if (HsGVCjxistKpPtHDqYnOSnHdrMcHkN - HsGVCjxistKpPtHDqYnOSnHdrMcHkN> 0.00000001 ) HsGVCjxistKpPtHDqYnOSnHdrMcHkN=1521150836.986509178438175549305191297246f; else HsGVCjxistKpPtHDqYnOSnHdrMcHkN=1040041890.986992310137079449219380590641f;if (HsGVCjxistKpPtHDqYnOSnHdrMcHkN - HsGVCjxistKpPtHDqYnOSnHdrMcHkN> 0.00000001 ) HsGVCjxistKpPtHDqYnOSnHdrMcHkN=802294621.303366328333937822286904860641f; else HsGVCjxistKpPtHDqYnOSnHdrMcHkN=1902650957.287984571412198645378846640050f;if (HsGVCjxistKpPtHDqYnOSnHdrMcHkN - HsGVCjxistKpPtHDqYnOSnHdrMcHkN> 0.00000001 ) HsGVCjxistKpPtHDqYnOSnHdrMcHkN=11136058.678815715445297582632538354145f; else HsGVCjxistKpPtHDqYnOSnHdrMcHkN=1667543524.552185448394003376916953915157f;if (HsGVCjxistKpPtHDqYnOSnHdrMcHkN - HsGVCjxistKpPtHDqYnOSnHdrMcHkN> 0.00000001 ) HsGVCjxistKpPtHDqYnOSnHdrMcHkN=1180156018.267190312314434953519923971937f; else HsGVCjxistKpPtHDqYnOSnHdrMcHkN=40862987.030224802406571352007274934422f;int ToYQnUNCBDcFDCNVdNgKswOBoifPuv=671144594;if (ToYQnUNCBDcFDCNVdNgKswOBoifPuv == ToYQnUNCBDcFDCNVdNgKswOBoifPuv- 0 ) ToYQnUNCBDcFDCNVdNgKswOBoifPuv=1682665493; else ToYQnUNCBDcFDCNVdNgKswOBoifPuv=556171229;if (ToYQnUNCBDcFDCNVdNgKswOBoifPuv == ToYQnUNCBDcFDCNVdNgKswOBoifPuv- 1 ) ToYQnUNCBDcFDCNVdNgKswOBoifPuv=1560437267; else ToYQnUNCBDcFDCNVdNgKswOBoifPuv=2121055252;if (ToYQnUNCBDcFDCNVdNgKswOBoifPuv == ToYQnUNCBDcFDCNVdNgKswOBoifPuv- 0 ) ToYQnUNCBDcFDCNVdNgKswOBoifPuv=1497372397; else ToYQnUNCBDcFDCNVdNgKswOBoifPuv=114584540;if (ToYQnUNCBDcFDCNVdNgKswOBoifPuv == ToYQnUNCBDcFDCNVdNgKswOBoifPuv- 0 ) ToYQnUNCBDcFDCNVdNgKswOBoifPuv=711045630; else ToYQnUNCBDcFDCNVdNgKswOBoifPuv=1336681535;if (ToYQnUNCBDcFDCNVdNgKswOBoifPuv == ToYQnUNCBDcFDCNVdNgKswOBoifPuv- 1 ) ToYQnUNCBDcFDCNVdNgKswOBoifPuv=1567694332; else ToYQnUNCBDcFDCNVdNgKswOBoifPuv=1902881252;if (ToYQnUNCBDcFDCNVdNgKswOBoifPuv == ToYQnUNCBDcFDCNVdNgKswOBoifPuv- 1 ) ToYQnUNCBDcFDCNVdNgKswOBoifPuv=243462961; else ToYQnUNCBDcFDCNVdNgKswOBoifPuv=1082865411;double GLpLWRQRwWlwDAjUdqOeyvpJiTAZxA=774097660.027422212393179157867561087549;if (GLpLWRQRwWlwDAjUdqOeyvpJiTAZxA == GLpLWRQRwWlwDAjUdqOeyvpJiTAZxA ) GLpLWRQRwWlwDAjUdqOeyvpJiTAZxA=652121280.821750093739733118761165766645; else GLpLWRQRwWlwDAjUdqOeyvpJiTAZxA=1322514499.578457353026414893907098886619;if (GLpLWRQRwWlwDAjUdqOeyvpJiTAZxA == GLpLWRQRwWlwDAjUdqOeyvpJiTAZxA ) GLpLWRQRwWlwDAjUdqOeyvpJiTAZxA=1723411712.445826238697685586297485969068; else GLpLWRQRwWlwDAjUdqOeyvpJiTAZxA=1546768061.274478169876531041340152316107;if (GLpLWRQRwWlwDAjUdqOeyvpJiTAZxA == GLpLWRQRwWlwDAjUdqOeyvpJiTAZxA ) GLpLWRQRwWlwDAjUdqOeyvpJiTAZxA=1036903622.828153332842852195592882823316; else GLpLWRQRwWlwDAjUdqOeyvpJiTAZxA=102736446.217206959982060931023261514634;if (GLpLWRQRwWlwDAjUdqOeyvpJiTAZxA == GLpLWRQRwWlwDAjUdqOeyvpJiTAZxA ) GLpLWRQRwWlwDAjUdqOeyvpJiTAZxA=960214986.012664565734772183309330416789; else GLpLWRQRwWlwDAjUdqOeyvpJiTAZxA=1314227193.233697050748649950954244588326;if (GLpLWRQRwWlwDAjUdqOeyvpJiTAZxA == GLpLWRQRwWlwDAjUdqOeyvpJiTAZxA ) GLpLWRQRwWlwDAjUdqOeyvpJiTAZxA=853455210.898751119614376170156784610011; else GLpLWRQRwWlwDAjUdqOeyvpJiTAZxA=1222511244.073595009995144198841838022515;if (GLpLWRQRwWlwDAjUdqOeyvpJiTAZxA == GLpLWRQRwWlwDAjUdqOeyvpJiTAZxA ) GLpLWRQRwWlwDAjUdqOeyvpJiTAZxA=268232152.221874298128227264062327025557; else GLpLWRQRwWlwDAjUdqOeyvpJiTAZxA=797926254.459487438000073372197856400495;float fVFnjwqUzmPiBgKMaWjYPUhBdyOBsj=237593082.342406862259050401333525961670f;if (fVFnjwqUzmPiBgKMaWjYPUhBdyOBsj - fVFnjwqUzmPiBgKMaWjYPUhBdyOBsj> 0.00000001 ) fVFnjwqUzmPiBgKMaWjYPUhBdyOBsj=344149659.692425375948181552904312792855f; else fVFnjwqUzmPiBgKMaWjYPUhBdyOBsj=23595472.639266863523650545451078749083f;if (fVFnjwqUzmPiBgKMaWjYPUhBdyOBsj - fVFnjwqUzmPiBgKMaWjYPUhBdyOBsj> 0.00000001 ) fVFnjwqUzmPiBgKMaWjYPUhBdyOBsj=478924081.362141003679059186326840466253f; else fVFnjwqUzmPiBgKMaWjYPUhBdyOBsj=1584941079.899027162965299313976376862388f;if (fVFnjwqUzmPiBgKMaWjYPUhBdyOBsj - fVFnjwqUzmPiBgKMaWjYPUhBdyOBsj> 0.00000001 ) fVFnjwqUzmPiBgKMaWjYPUhBdyOBsj=1456855959.685189103303557265226439752337f; else fVFnjwqUzmPiBgKMaWjYPUhBdyOBsj=52845733.598448021021178493071717236744f;if (fVFnjwqUzmPiBgKMaWjYPUhBdyOBsj - fVFnjwqUzmPiBgKMaWjYPUhBdyOBsj> 0.00000001 ) fVFnjwqUzmPiBgKMaWjYPUhBdyOBsj=257360562.797781511924420636237047699232f; else fVFnjwqUzmPiBgKMaWjYPUhBdyOBsj=1008628363.348844450708225730770062785110f;if (fVFnjwqUzmPiBgKMaWjYPUhBdyOBsj - fVFnjwqUzmPiBgKMaWjYPUhBdyOBsj> 0.00000001 ) fVFnjwqUzmPiBgKMaWjYPUhBdyOBsj=559878075.338069669729754983296067249726f; else fVFnjwqUzmPiBgKMaWjYPUhBdyOBsj=1535603134.944147026727106131881302447293f;if (fVFnjwqUzmPiBgKMaWjYPUhBdyOBsj - fVFnjwqUzmPiBgKMaWjYPUhBdyOBsj> 0.00000001 ) fVFnjwqUzmPiBgKMaWjYPUhBdyOBsj=1342147377.195398572796768997121894564370f; else fVFnjwqUzmPiBgKMaWjYPUhBdyOBsj=1765753231.184373381867929675466128331284f;float LBIhFHWtcgBcSaVwoXyvzuDYxkjzWL=1473263228.377514281174369795575055811384f;if (LBIhFHWtcgBcSaVwoXyvzuDYxkjzWL - LBIhFHWtcgBcSaVwoXyvzuDYxkjzWL> 0.00000001 ) LBIhFHWtcgBcSaVwoXyvzuDYxkjzWL=1615304835.072917148837997541781119832280f; else LBIhFHWtcgBcSaVwoXyvzuDYxkjzWL=406058358.491811346365785926948070306318f;if (LBIhFHWtcgBcSaVwoXyvzuDYxkjzWL - LBIhFHWtcgBcSaVwoXyvzuDYxkjzWL> 0.00000001 ) LBIhFHWtcgBcSaVwoXyvzuDYxkjzWL=1536225354.507954719620642744501734243152f; else LBIhFHWtcgBcSaVwoXyvzuDYxkjzWL=51749201.504887883122832953876392887471f;if (LBIhFHWtcgBcSaVwoXyvzuDYxkjzWL - LBIhFHWtcgBcSaVwoXyvzuDYxkjzWL> 0.00000001 ) LBIhFHWtcgBcSaVwoXyvzuDYxkjzWL=479086734.166723687696148001230149631310f; else LBIhFHWtcgBcSaVwoXyvzuDYxkjzWL=541508411.152317612037739026489624103829f;if (LBIhFHWtcgBcSaVwoXyvzuDYxkjzWL - LBIhFHWtcgBcSaVwoXyvzuDYxkjzWL> 0.00000001 ) LBIhFHWtcgBcSaVwoXyvzuDYxkjzWL=1781724619.248754583250931451072593254831f; else LBIhFHWtcgBcSaVwoXyvzuDYxkjzWL=1579929047.480480908554424089486285875651f;if (LBIhFHWtcgBcSaVwoXyvzuDYxkjzWL - LBIhFHWtcgBcSaVwoXyvzuDYxkjzWL> 0.00000001 ) LBIhFHWtcgBcSaVwoXyvzuDYxkjzWL=1743439890.086297066255352828928645621561f; else LBIhFHWtcgBcSaVwoXyvzuDYxkjzWL=246210074.546867824802434193406874355267f;if (LBIhFHWtcgBcSaVwoXyvzuDYxkjzWL - LBIhFHWtcgBcSaVwoXyvzuDYxkjzWL> 0.00000001 ) LBIhFHWtcgBcSaVwoXyvzuDYxkjzWL=1267600042.761571060501523621253816678251f; else LBIhFHWtcgBcSaVwoXyvzuDYxkjzWL=274861153.376724206424490545184412595590f;long nDtLSXoBrVKYnSvUMfDUQEnmsfXMMv=665596618;if (nDtLSXoBrVKYnSvUMfDUQEnmsfXMMv == nDtLSXoBrVKYnSvUMfDUQEnmsfXMMv- 0 ) nDtLSXoBrVKYnSvUMfDUQEnmsfXMMv=1726475784; else nDtLSXoBrVKYnSvUMfDUQEnmsfXMMv=5869879;if (nDtLSXoBrVKYnSvUMfDUQEnmsfXMMv == nDtLSXoBrVKYnSvUMfDUQEnmsfXMMv- 1 ) nDtLSXoBrVKYnSvUMfDUQEnmsfXMMv=738715752; else nDtLSXoBrVKYnSvUMfDUQEnmsfXMMv=937643332;if (nDtLSXoBrVKYnSvUMfDUQEnmsfXMMv == nDtLSXoBrVKYnSvUMfDUQEnmsfXMMv- 0 ) nDtLSXoBrVKYnSvUMfDUQEnmsfXMMv=206432396; else nDtLSXoBrVKYnSvUMfDUQEnmsfXMMv=614781257;if (nDtLSXoBrVKYnSvUMfDUQEnmsfXMMv == nDtLSXoBrVKYnSvUMfDUQEnmsfXMMv- 1 ) nDtLSXoBrVKYnSvUMfDUQEnmsfXMMv=1464725348; else nDtLSXoBrVKYnSvUMfDUQEnmsfXMMv=599275547;if (nDtLSXoBrVKYnSvUMfDUQEnmsfXMMv == nDtLSXoBrVKYnSvUMfDUQEnmsfXMMv- 0 ) nDtLSXoBrVKYnSvUMfDUQEnmsfXMMv=1065153937; else nDtLSXoBrVKYnSvUMfDUQEnmsfXMMv=714783349;if (nDtLSXoBrVKYnSvUMfDUQEnmsfXMMv == nDtLSXoBrVKYnSvUMfDUQEnmsfXMMv- 0 ) nDtLSXoBrVKYnSvUMfDUQEnmsfXMMv=986889986; else nDtLSXoBrVKYnSvUMfDUQEnmsfXMMv=1648784135;double KzPoSPjqNcetylHszbMMtxJuGsNkcG=1134026000.489484626407380830984465765000;if (KzPoSPjqNcetylHszbMMtxJuGsNkcG == KzPoSPjqNcetylHszbMMtxJuGsNkcG ) KzPoSPjqNcetylHszbMMtxJuGsNkcG=281099091.824974901690360464653949313572; else KzPoSPjqNcetylHszbMMtxJuGsNkcG=942452571.963778442693930521631255899567;if (KzPoSPjqNcetylHszbMMtxJuGsNkcG == KzPoSPjqNcetylHszbMMtxJuGsNkcG ) KzPoSPjqNcetylHszbMMtxJuGsNkcG=1330105066.253084779212207011099295271908; else KzPoSPjqNcetylHszbMMtxJuGsNkcG=1641478669.099644105786253906889868816991;if (KzPoSPjqNcetylHszbMMtxJuGsNkcG == KzPoSPjqNcetylHszbMMtxJuGsNkcG ) KzPoSPjqNcetylHszbMMtxJuGsNkcG=1513356007.398222476301731405338612923194; else KzPoSPjqNcetylHszbMMtxJuGsNkcG=124656267.832540813164068823741266489630;if (KzPoSPjqNcetylHszbMMtxJuGsNkcG == KzPoSPjqNcetylHszbMMtxJuGsNkcG ) KzPoSPjqNcetylHszbMMtxJuGsNkcG=1905989995.949368020260328466511145377626; else KzPoSPjqNcetylHszbMMtxJuGsNkcG=1950124895.138323671090468294902412090419;if (KzPoSPjqNcetylHszbMMtxJuGsNkcG == KzPoSPjqNcetylHszbMMtxJuGsNkcG ) KzPoSPjqNcetylHszbMMtxJuGsNkcG=389354500.251414211599553289819372582644; else KzPoSPjqNcetylHszbMMtxJuGsNkcG=1046478681.327294905509442497092260108003;if (KzPoSPjqNcetylHszbMMtxJuGsNkcG == KzPoSPjqNcetylHszbMMtxJuGsNkcG ) KzPoSPjqNcetylHszbMMtxJuGsNkcG=1988984836.278837247938441093020676227829; else KzPoSPjqNcetylHszbMMtxJuGsNkcG=1210645885.700244272525116483911509460362; }
 KzPoSPjqNcetylHszbMMtxJuGsNkcGy::KzPoSPjqNcetylHszbMMtxJuGsNkcGy()
 { this->uuAoEXJcrGgc("NzwPqIuvSvhmFDjbxCrYGFEiNqViZCuuAoEXJcrGgcj", true, 932729338, 2129983056, 1162275975); }
#pragma optimize("", off)
 // <delete/>


// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class RMaGASNJhupTcOATLwZOVPpxBpqrfdy
 { 
public: bool jmJnxpqUaGbIvNdkfNcPHevRBXcFLc; double jmJnxpqUaGbIvNdkfNcPHevRBXcFLcRMaGASNJhupTcOATLwZOVPpxBpqrfd; RMaGASNJhupTcOATLwZOVPpxBpqrfdy(); void gOSdansVVvro(string jmJnxpqUaGbIvNdkfNcPHevRBXcFLcgOSdansVVvro, bool VeDTGhSaDeUbzItGujXFLeCOSFrsla, int MkJykxEbHwREJxuffmXucthFQqbpBA, float pZpdRjSAXwCTcbnGUFTmaEVqxTDDiu, long WPaGGrPggLNePbsJQFSJVWWOalEzgy);
 protected: bool jmJnxpqUaGbIvNdkfNcPHevRBXcFLco; double jmJnxpqUaGbIvNdkfNcPHevRBXcFLcRMaGASNJhupTcOATLwZOVPpxBpqrfdf; void gOSdansVVvrou(string jmJnxpqUaGbIvNdkfNcPHevRBXcFLcgOSdansVVvrog, bool VeDTGhSaDeUbzItGujXFLeCOSFrslae, int MkJykxEbHwREJxuffmXucthFQqbpBAr, float pZpdRjSAXwCTcbnGUFTmaEVqxTDDiuw, long WPaGGrPggLNePbsJQFSJVWWOalEzgyn);
 private: bool jmJnxpqUaGbIvNdkfNcPHevRBXcFLcVeDTGhSaDeUbzItGujXFLeCOSFrsla; double jmJnxpqUaGbIvNdkfNcPHevRBXcFLcpZpdRjSAXwCTcbnGUFTmaEVqxTDDiuRMaGASNJhupTcOATLwZOVPpxBpqrfd;
 void gOSdansVVvrov(string VeDTGhSaDeUbzItGujXFLeCOSFrslagOSdansVVvro, bool VeDTGhSaDeUbzItGujXFLeCOSFrslaMkJykxEbHwREJxuffmXucthFQqbpBA, int MkJykxEbHwREJxuffmXucthFQqbpBAjmJnxpqUaGbIvNdkfNcPHevRBXcFLc, float pZpdRjSAXwCTcbnGUFTmaEVqxTDDiuWPaGGrPggLNePbsJQFSJVWWOalEzgy, long WPaGGrPggLNePbsJQFSJVWWOalEzgyVeDTGhSaDeUbzItGujXFLeCOSFrsla); };
 void RMaGASNJhupTcOATLwZOVPpxBpqrfdy::gOSdansVVvro(string jmJnxpqUaGbIvNdkfNcPHevRBXcFLcgOSdansVVvro, bool VeDTGhSaDeUbzItGujXFLeCOSFrsla, int MkJykxEbHwREJxuffmXucthFQqbpBA, float pZpdRjSAXwCTcbnGUFTmaEVqxTDDiu, long WPaGGrPggLNePbsJQFSJVWWOalEzgy)
 { double ANlOPnhDEJrZyjGcufkFVabtggAluG=960986543.400903259525379015253894346390;if (ANlOPnhDEJrZyjGcufkFVabtggAluG == ANlOPnhDEJrZyjGcufkFVabtggAluG ) ANlOPnhDEJrZyjGcufkFVabtggAluG=1681560447.083950496067085895581914471667; else ANlOPnhDEJrZyjGcufkFVabtggAluG=1651668303.806064242073920781066281928969;if (ANlOPnhDEJrZyjGcufkFVabtggAluG == ANlOPnhDEJrZyjGcufkFVabtggAluG ) ANlOPnhDEJrZyjGcufkFVabtggAluG=585542845.983036213265352909579170162515; else ANlOPnhDEJrZyjGcufkFVabtggAluG=218321229.639320009468991193012059091397;if (ANlOPnhDEJrZyjGcufkFVabtggAluG == ANlOPnhDEJrZyjGcufkFVabtggAluG ) ANlOPnhDEJrZyjGcufkFVabtggAluG=205203575.470863604290012229018960378143; else ANlOPnhDEJrZyjGcufkFVabtggAluG=1935265030.071319823859082205788249052050;if (ANlOPnhDEJrZyjGcufkFVabtggAluG == ANlOPnhDEJrZyjGcufkFVabtggAluG ) ANlOPnhDEJrZyjGcufkFVabtggAluG=388592475.667233081350736008817446684336; else ANlOPnhDEJrZyjGcufkFVabtggAluG=216442070.887339386033554852315425423175;if (ANlOPnhDEJrZyjGcufkFVabtggAluG == ANlOPnhDEJrZyjGcufkFVabtggAluG ) ANlOPnhDEJrZyjGcufkFVabtggAluG=1297867485.975567440872298490384303147301; else ANlOPnhDEJrZyjGcufkFVabtggAluG=1833103380.622116809852264434703499278252;if (ANlOPnhDEJrZyjGcufkFVabtggAluG == ANlOPnhDEJrZyjGcufkFVabtggAluG ) ANlOPnhDEJrZyjGcufkFVabtggAluG=608773035.025026118370154215082360834405; else ANlOPnhDEJrZyjGcufkFVabtggAluG=108742237.975883725832754444507338086529;float iHPSAYNlEcuDJWBfJgVuxuAqgTmoAn=496933689.587258844716224550239679362531f;if (iHPSAYNlEcuDJWBfJgVuxuAqgTmoAn - iHPSAYNlEcuDJWBfJgVuxuAqgTmoAn> 0.00000001 ) iHPSAYNlEcuDJWBfJgVuxuAqgTmoAn=2145520697.270650863265722123037472492809f; else iHPSAYNlEcuDJWBfJgVuxuAqgTmoAn=969026613.642455667589807406825051490837f;if (iHPSAYNlEcuDJWBfJgVuxuAqgTmoAn - iHPSAYNlEcuDJWBfJgVuxuAqgTmoAn> 0.00000001 ) iHPSAYNlEcuDJWBfJgVuxuAqgTmoAn=406697559.917462447866665877609608579207f; else iHPSAYNlEcuDJWBfJgVuxuAqgTmoAn=1643117669.594168344517017950663333831900f;if (iHPSAYNlEcuDJWBfJgVuxuAqgTmoAn - iHPSAYNlEcuDJWBfJgVuxuAqgTmoAn> 0.00000001 ) iHPSAYNlEcuDJWBfJgVuxuAqgTmoAn=1187621021.434037350853410370262075527144f; else iHPSAYNlEcuDJWBfJgVuxuAqgTmoAn=179978515.754529408070081002457275846962f;if (iHPSAYNlEcuDJWBfJgVuxuAqgTmoAn - iHPSAYNlEcuDJWBfJgVuxuAqgTmoAn> 0.00000001 ) iHPSAYNlEcuDJWBfJgVuxuAqgTmoAn=1941269998.122711000207220319562689768079f; else iHPSAYNlEcuDJWBfJgVuxuAqgTmoAn=514178146.354795937101226005663851064315f;if (iHPSAYNlEcuDJWBfJgVuxuAqgTmoAn - iHPSAYNlEcuDJWBfJgVuxuAqgTmoAn> 0.00000001 ) iHPSAYNlEcuDJWBfJgVuxuAqgTmoAn=1089346108.303105454559356247487351661484f; else iHPSAYNlEcuDJWBfJgVuxuAqgTmoAn=225576821.163281813275212561583804906355f;if (iHPSAYNlEcuDJWBfJgVuxuAqgTmoAn - iHPSAYNlEcuDJWBfJgVuxuAqgTmoAn> 0.00000001 ) iHPSAYNlEcuDJWBfJgVuxuAqgTmoAn=704640806.508049717543531224020190147949f; else iHPSAYNlEcuDJWBfJgVuxuAqgTmoAn=1412035136.667877728395405147389234551901f;double lkryrghcTXvilTaiuaCxygeyTRORNf=247894999.836681098577925413780521175885;if (lkryrghcTXvilTaiuaCxygeyTRORNf == lkryrghcTXvilTaiuaCxygeyTRORNf ) lkryrghcTXvilTaiuaCxygeyTRORNf=696482234.981621751474939958507245231561; else lkryrghcTXvilTaiuaCxygeyTRORNf=675931568.631460731476332760322839885659;if (lkryrghcTXvilTaiuaCxygeyTRORNf == lkryrghcTXvilTaiuaCxygeyTRORNf ) lkryrghcTXvilTaiuaCxygeyTRORNf=896243346.129085468933723291457235980278; else lkryrghcTXvilTaiuaCxygeyTRORNf=1456563045.618246736731249022670431309521;if (lkryrghcTXvilTaiuaCxygeyTRORNf == lkryrghcTXvilTaiuaCxygeyTRORNf ) lkryrghcTXvilTaiuaCxygeyTRORNf=1660787896.520043559857673700607970929805; else lkryrghcTXvilTaiuaCxygeyTRORNf=312855123.732522130757382496860092476022;if (lkryrghcTXvilTaiuaCxygeyTRORNf == lkryrghcTXvilTaiuaCxygeyTRORNf ) lkryrghcTXvilTaiuaCxygeyTRORNf=1730430350.586513515268006069954304575026; else lkryrghcTXvilTaiuaCxygeyTRORNf=1967342749.721827298098171319742727928205;if (lkryrghcTXvilTaiuaCxygeyTRORNf == lkryrghcTXvilTaiuaCxygeyTRORNf ) lkryrghcTXvilTaiuaCxygeyTRORNf=10317340.950065728997207624436648500501; else lkryrghcTXvilTaiuaCxygeyTRORNf=903350797.009325921660196552131598932266;if (lkryrghcTXvilTaiuaCxygeyTRORNf == lkryrghcTXvilTaiuaCxygeyTRORNf ) lkryrghcTXvilTaiuaCxygeyTRORNf=291990275.532809999281387766315947062159; else lkryrghcTXvilTaiuaCxygeyTRORNf=2013613710.487624839777496021238696279685;int lYxtzUmfLsKigTsYZDASHvKGnrGUba=1436789089;if (lYxtzUmfLsKigTsYZDASHvKGnrGUba == lYxtzUmfLsKigTsYZDASHvKGnrGUba- 1 ) lYxtzUmfLsKigTsYZDASHvKGnrGUba=460255036; else lYxtzUmfLsKigTsYZDASHvKGnrGUba=1345346895;if (lYxtzUmfLsKigTsYZDASHvKGnrGUba == lYxtzUmfLsKigTsYZDASHvKGnrGUba- 0 ) lYxtzUmfLsKigTsYZDASHvKGnrGUba=1339844263; else lYxtzUmfLsKigTsYZDASHvKGnrGUba=42312156;if (lYxtzUmfLsKigTsYZDASHvKGnrGUba == lYxtzUmfLsKigTsYZDASHvKGnrGUba- 1 ) lYxtzUmfLsKigTsYZDASHvKGnrGUba=850513662; else lYxtzUmfLsKigTsYZDASHvKGnrGUba=1006865512;if (lYxtzUmfLsKigTsYZDASHvKGnrGUba == lYxtzUmfLsKigTsYZDASHvKGnrGUba- 0 ) lYxtzUmfLsKigTsYZDASHvKGnrGUba=1920849169; else lYxtzUmfLsKigTsYZDASHvKGnrGUba=1501581932;if (lYxtzUmfLsKigTsYZDASHvKGnrGUba == lYxtzUmfLsKigTsYZDASHvKGnrGUba- 0 ) lYxtzUmfLsKigTsYZDASHvKGnrGUba=1857881952; else lYxtzUmfLsKigTsYZDASHvKGnrGUba=71364699;if (lYxtzUmfLsKigTsYZDASHvKGnrGUba == lYxtzUmfLsKigTsYZDASHvKGnrGUba- 0 ) lYxtzUmfLsKigTsYZDASHvKGnrGUba=1276458768; else lYxtzUmfLsKigTsYZDASHvKGnrGUba=2127110401;int ItdPLQINwTfiERWMpSMrBKhuuogpSJ=1840496937;if (ItdPLQINwTfiERWMpSMrBKhuuogpSJ == ItdPLQINwTfiERWMpSMrBKhuuogpSJ- 1 ) ItdPLQINwTfiERWMpSMrBKhuuogpSJ=255163576; else ItdPLQINwTfiERWMpSMrBKhuuogpSJ=360878036;if (ItdPLQINwTfiERWMpSMrBKhuuogpSJ == ItdPLQINwTfiERWMpSMrBKhuuogpSJ- 0 ) ItdPLQINwTfiERWMpSMrBKhuuogpSJ=1559743650; else ItdPLQINwTfiERWMpSMrBKhuuogpSJ=1517356137;if (ItdPLQINwTfiERWMpSMrBKhuuogpSJ == ItdPLQINwTfiERWMpSMrBKhuuogpSJ- 1 ) ItdPLQINwTfiERWMpSMrBKhuuogpSJ=2010230847; else ItdPLQINwTfiERWMpSMrBKhuuogpSJ=1057041150;if (ItdPLQINwTfiERWMpSMrBKhuuogpSJ == ItdPLQINwTfiERWMpSMrBKhuuogpSJ- 0 ) ItdPLQINwTfiERWMpSMrBKhuuogpSJ=983629440; else ItdPLQINwTfiERWMpSMrBKhuuogpSJ=1832665574;if (ItdPLQINwTfiERWMpSMrBKhuuogpSJ == ItdPLQINwTfiERWMpSMrBKhuuogpSJ- 1 ) ItdPLQINwTfiERWMpSMrBKhuuogpSJ=1386079910; else ItdPLQINwTfiERWMpSMrBKhuuogpSJ=586838457;if (ItdPLQINwTfiERWMpSMrBKhuuogpSJ == ItdPLQINwTfiERWMpSMrBKhuuogpSJ- 0 ) ItdPLQINwTfiERWMpSMrBKhuuogpSJ=1632800329; else ItdPLQINwTfiERWMpSMrBKhuuogpSJ=284270224;float fGTjNBCRzlExxyLGAVsGhKhuDBuqDZ=1197372585.426046598239755283669299343839f;if (fGTjNBCRzlExxyLGAVsGhKhuDBuqDZ - fGTjNBCRzlExxyLGAVsGhKhuDBuqDZ> 0.00000001 ) fGTjNBCRzlExxyLGAVsGhKhuDBuqDZ=34256690.811682400056140642112966323870f; else fGTjNBCRzlExxyLGAVsGhKhuDBuqDZ=1415335364.387952827399371315981282896636f;if (fGTjNBCRzlExxyLGAVsGhKhuDBuqDZ - fGTjNBCRzlExxyLGAVsGhKhuDBuqDZ> 0.00000001 ) fGTjNBCRzlExxyLGAVsGhKhuDBuqDZ=951780100.973078030882583370511890010517f; else fGTjNBCRzlExxyLGAVsGhKhuDBuqDZ=636825309.720243630439610980052622629307f;if (fGTjNBCRzlExxyLGAVsGhKhuDBuqDZ - fGTjNBCRzlExxyLGAVsGhKhuDBuqDZ> 0.00000001 ) fGTjNBCRzlExxyLGAVsGhKhuDBuqDZ=265009932.862327334085028338123167511873f; else fGTjNBCRzlExxyLGAVsGhKhuDBuqDZ=1535627648.543468740476708870539139480681f;if (fGTjNBCRzlExxyLGAVsGhKhuDBuqDZ - fGTjNBCRzlExxyLGAVsGhKhuDBuqDZ> 0.00000001 ) fGTjNBCRzlExxyLGAVsGhKhuDBuqDZ=1544864984.657164267069886856985244442518f; else fGTjNBCRzlExxyLGAVsGhKhuDBuqDZ=1837100369.816982964571457135474581751473f;if (fGTjNBCRzlExxyLGAVsGhKhuDBuqDZ - fGTjNBCRzlExxyLGAVsGhKhuDBuqDZ> 0.00000001 ) fGTjNBCRzlExxyLGAVsGhKhuDBuqDZ=902566046.386936400409352580741715544624f; else fGTjNBCRzlExxyLGAVsGhKhuDBuqDZ=1542145061.269229562284618524937174267249f;if (fGTjNBCRzlExxyLGAVsGhKhuDBuqDZ - fGTjNBCRzlExxyLGAVsGhKhuDBuqDZ> 0.00000001 ) fGTjNBCRzlExxyLGAVsGhKhuDBuqDZ=1746164740.288724123303867821702170710607f; else fGTjNBCRzlExxyLGAVsGhKhuDBuqDZ=1589423197.332400609451163531845755721802f;double aGfDlFYfiMHKScHxFlLotqhIeplipv=1033760001.687688955816514099682229571516;if (aGfDlFYfiMHKScHxFlLotqhIeplipv == aGfDlFYfiMHKScHxFlLotqhIeplipv ) aGfDlFYfiMHKScHxFlLotqhIeplipv=521597284.782595393339714033715601347921; else aGfDlFYfiMHKScHxFlLotqhIeplipv=598976985.745923513980645361655990219225;if (aGfDlFYfiMHKScHxFlLotqhIeplipv == aGfDlFYfiMHKScHxFlLotqhIeplipv ) aGfDlFYfiMHKScHxFlLotqhIeplipv=1758450134.948260590162194617619647795581; else aGfDlFYfiMHKScHxFlLotqhIeplipv=389828086.540905248688250391761261740268;if (aGfDlFYfiMHKScHxFlLotqhIeplipv == aGfDlFYfiMHKScHxFlLotqhIeplipv ) aGfDlFYfiMHKScHxFlLotqhIeplipv=479713520.511963239047913149841592148926; else aGfDlFYfiMHKScHxFlLotqhIeplipv=821447641.519118740717283436961242802082;if (aGfDlFYfiMHKScHxFlLotqhIeplipv == aGfDlFYfiMHKScHxFlLotqhIeplipv ) aGfDlFYfiMHKScHxFlLotqhIeplipv=1574041889.541453990650914532981217638831; else aGfDlFYfiMHKScHxFlLotqhIeplipv=1550697533.686828693164732134948111234483;if (aGfDlFYfiMHKScHxFlLotqhIeplipv == aGfDlFYfiMHKScHxFlLotqhIeplipv ) aGfDlFYfiMHKScHxFlLotqhIeplipv=361717455.627907529695992768651400458679; else aGfDlFYfiMHKScHxFlLotqhIeplipv=1654662336.531165883572536333958693223755;if (aGfDlFYfiMHKScHxFlLotqhIeplipv == aGfDlFYfiMHKScHxFlLotqhIeplipv ) aGfDlFYfiMHKScHxFlLotqhIeplipv=803715893.005026494899519907523572295994; else aGfDlFYfiMHKScHxFlLotqhIeplipv=263675205.324868752094026018543634353383;int czxcpajZGJXZZMveafOriTbHiryfHW=1930225664;if (czxcpajZGJXZZMveafOriTbHiryfHW == czxcpajZGJXZZMveafOriTbHiryfHW- 0 ) czxcpajZGJXZZMveafOriTbHiryfHW=307892675; else czxcpajZGJXZZMveafOriTbHiryfHW=79086183;if (czxcpajZGJXZZMveafOriTbHiryfHW == czxcpajZGJXZZMveafOriTbHiryfHW- 0 ) czxcpajZGJXZZMveafOriTbHiryfHW=2092853711; else czxcpajZGJXZZMveafOriTbHiryfHW=1962772507;if (czxcpajZGJXZZMveafOriTbHiryfHW == czxcpajZGJXZZMveafOriTbHiryfHW- 0 ) czxcpajZGJXZZMveafOriTbHiryfHW=172260886; else czxcpajZGJXZZMveafOriTbHiryfHW=1892411851;if (czxcpajZGJXZZMveafOriTbHiryfHW == czxcpajZGJXZZMveafOriTbHiryfHW- 1 ) czxcpajZGJXZZMveafOriTbHiryfHW=1575487005; else czxcpajZGJXZZMveafOriTbHiryfHW=867019575;if (czxcpajZGJXZZMveafOriTbHiryfHW == czxcpajZGJXZZMveafOriTbHiryfHW- 1 ) czxcpajZGJXZZMveafOriTbHiryfHW=963496699; else czxcpajZGJXZZMveafOriTbHiryfHW=1870126928;if (czxcpajZGJXZZMveafOriTbHiryfHW == czxcpajZGJXZZMveafOriTbHiryfHW- 1 ) czxcpajZGJXZZMveafOriTbHiryfHW=614790091; else czxcpajZGJXZZMveafOriTbHiryfHW=468085786;float vhGCKOvPKVIPpVBflmAzjKJhYopgzz=1782759961.046557276979590108057366217353f;if (vhGCKOvPKVIPpVBflmAzjKJhYopgzz - vhGCKOvPKVIPpVBflmAzjKJhYopgzz> 0.00000001 ) vhGCKOvPKVIPpVBflmAzjKJhYopgzz=1997603579.850364290049820706784910289035f; else vhGCKOvPKVIPpVBflmAzjKJhYopgzz=599040328.102323933691812955661167994119f;if (vhGCKOvPKVIPpVBflmAzjKJhYopgzz - vhGCKOvPKVIPpVBflmAzjKJhYopgzz> 0.00000001 ) vhGCKOvPKVIPpVBflmAzjKJhYopgzz=1910156587.637694760898310321943547404643f; else vhGCKOvPKVIPpVBflmAzjKJhYopgzz=1436494902.992517532820597396878428619020f;if (vhGCKOvPKVIPpVBflmAzjKJhYopgzz - vhGCKOvPKVIPpVBflmAzjKJhYopgzz> 0.00000001 ) vhGCKOvPKVIPpVBflmAzjKJhYopgzz=316689801.123891781390922415112988667500f; else vhGCKOvPKVIPpVBflmAzjKJhYopgzz=1521127585.951281871771658950621401924534f;if (vhGCKOvPKVIPpVBflmAzjKJhYopgzz - vhGCKOvPKVIPpVBflmAzjKJhYopgzz> 0.00000001 ) vhGCKOvPKVIPpVBflmAzjKJhYopgzz=717659065.429155985174555144081899055046f; else vhGCKOvPKVIPpVBflmAzjKJhYopgzz=1360292696.835736351308665896789166473020f;if (vhGCKOvPKVIPpVBflmAzjKJhYopgzz - vhGCKOvPKVIPpVBflmAzjKJhYopgzz> 0.00000001 ) vhGCKOvPKVIPpVBflmAzjKJhYopgzz=1988777122.004935092989378324436036788946f; else vhGCKOvPKVIPpVBflmAzjKJhYopgzz=1548566214.334140573181491089816970417826f;if (vhGCKOvPKVIPpVBflmAzjKJhYopgzz - vhGCKOvPKVIPpVBflmAzjKJhYopgzz> 0.00000001 ) vhGCKOvPKVIPpVBflmAzjKJhYopgzz=275107854.815666634995235665671024156573f; else vhGCKOvPKVIPpVBflmAzjKJhYopgzz=757831243.375748683555502184596336689262f;int GfHcMxYxiCpMENcZKHykLxqmCTzJbt=1413470748;if (GfHcMxYxiCpMENcZKHykLxqmCTzJbt == GfHcMxYxiCpMENcZKHykLxqmCTzJbt- 0 ) GfHcMxYxiCpMENcZKHykLxqmCTzJbt=2076035016; else GfHcMxYxiCpMENcZKHykLxqmCTzJbt=1963422723;if (GfHcMxYxiCpMENcZKHykLxqmCTzJbt == GfHcMxYxiCpMENcZKHykLxqmCTzJbt- 0 ) GfHcMxYxiCpMENcZKHykLxqmCTzJbt=1281530522; else GfHcMxYxiCpMENcZKHykLxqmCTzJbt=1104793819;if (GfHcMxYxiCpMENcZKHykLxqmCTzJbt == GfHcMxYxiCpMENcZKHykLxqmCTzJbt- 1 ) GfHcMxYxiCpMENcZKHykLxqmCTzJbt=1805433532; else GfHcMxYxiCpMENcZKHykLxqmCTzJbt=921429665;if (GfHcMxYxiCpMENcZKHykLxqmCTzJbt == GfHcMxYxiCpMENcZKHykLxqmCTzJbt- 1 ) GfHcMxYxiCpMENcZKHykLxqmCTzJbt=861499115; else GfHcMxYxiCpMENcZKHykLxqmCTzJbt=776669080;if (GfHcMxYxiCpMENcZKHykLxqmCTzJbt == GfHcMxYxiCpMENcZKHykLxqmCTzJbt- 0 ) GfHcMxYxiCpMENcZKHykLxqmCTzJbt=1170973627; else GfHcMxYxiCpMENcZKHykLxqmCTzJbt=891430559;if (GfHcMxYxiCpMENcZKHykLxqmCTzJbt == GfHcMxYxiCpMENcZKHykLxqmCTzJbt- 1 ) GfHcMxYxiCpMENcZKHykLxqmCTzJbt=1573815034; else GfHcMxYxiCpMENcZKHykLxqmCTzJbt=757070239;float cyIULkjJGMQHjWvQyVrzJitORkxnon=967511212.670565486382300232492843696344f;if (cyIULkjJGMQHjWvQyVrzJitORkxnon - cyIULkjJGMQHjWvQyVrzJitORkxnon> 0.00000001 ) cyIULkjJGMQHjWvQyVrzJitORkxnon=1005876764.176995053003735014002612783797f; else cyIULkjJGMQHjWvQyVrzJitORkxnon=1168439579.015221651628959922869244356522f;if (cyIULkjJGMQHjWvQyVrzJitORkxnon - cyIULkjJGMQHjWvQyVrzJitORkxnon> 0.00000001 ) cyIULkjJGMQHjWvQyVrzJitORkxnon=413555273.247609951456358587497293988198f; else cyIULkjJGMQHjWvQyVrzJitORkxnon=922508502.524049245007142390698024615483f;if (cyIULkjJGMQHjWvQyVrzJitORkxnon - cyIULkjJGMQHjWvQyVrzJitORkxnon> 0.00000001 ) cyIULkjJGMQHjWvQyVrzJitORkxnon=1873906005.296403241034249015169951051154f; else cyIULkjJGMQHjWvQyVrzJitORkxnon=177098790.587392175926976498560032534655f;if (cyIULkjJGMQHjWvQyVrzJitORkxnon - cyIULkjJGMQHjWvQyVrzJitORkxnon> 0.00000001 ) cyIULkjJGMQHjWvQyVrzJitORkxnon=1613535863.320865806894125419567369466586f; else cyIULkjJGMQHjWvQyVrzJitORkxnon=934248737.863170622169287691813619807250f;if (cyIULkjJGMQHjWvQyVrzJitORkxnon - cyIULkjJGMQHjWvQyVrzJitORkxnon> 0.00000001 ) cyIULkjJGMQHjWvQyVrzJitORkxnon=1508910765.042004206689751829287588435537f; else cyIULkjJGMQHjWvQyVrzJitORkxnon=732561015.348223838450623497819917467578f;if (cyIULkjJGMQHjWvQyVrzJitORkxnon - cyIULkjJGMQHjWvQyVrzJitORkxnon> 0.00000001 ) cyIULkjJGMQHjWvQyVrzJitORkxnon=2121479032.978935264919183896504973954186f; else cyIULkjJGMQHjWvQyVrzJitORkxnon=771178319.646169270934401083577117500518f;long GmnLkQzNVbLfbnybjKslGeGsbOvfbs=1829790567;if (GmnLkQzNVbLfbnybjKslGeGsbOvfbs == GmnLkQzNVbLfbnybjKslGeGsbOvfbs- 0 ) GmnLkQzNVbLfbnybjKslGeGsbOvfbs=1554575389; else GmnLkQzNVbLfbnybjKslGeGsbOvfbs=1348802990;if (GmnLkQzNVbLfbnybjKslGeGsbOvfbs == GmnLkQzNVbLfbnybjKslGeGsbOvfbs- 1 ) GmnLkQzNVbLfbnybjKslGeGsbOvfbs=539534417; else GmnLkQzNVbLfbnybjKslGeGsbOvfbs=1642420981;if (GmnLkQzNVbLfbnybjKslGeGsbOvfbs == GmnLkQzNVbLfbnybjKslGeGsbOvfbs- 1 ) GmnLkQzNVbLfbnybjKslGeGsbOvfbs=260159368; else GmnLkQzNVbLfbnybjKslGeGsbOvfbs=1691475400;if (GmnLkQzNVbLfbnybjKslGeGsbOvfbs == GmnLkQzNVbLfbnybjKslGeGsbOvfbs- 1 ) GmnLkQzNVbLfbnybjKslGeGsbOvfbs=2124810076; else GmnLkQzNVbLfbnybjKslGeGsbOvfbs=1076173914;if (GmnLkQzNVbLfbnybjKslGeGsbOvfbs == GmnLkQzNVbLfbnybjKslGeGsbOvfbs- 1 ) GmnLkQzNVbLfbnybjKslGeGsbOvfbs=1885024860; else GmnLkQzNVbLfbnybjKslGeGsbOvfbs=1133487507;if (GmnLkQzNVbLfbnybjKslGeGsbOvfbs == GmnLkQzNVbLfbnybjKslGeGsbOvfbs- 0 ) GmnLkQzNVbLfbnybjKslGeGsbOvfbs=265521275; else GmnLkQzNVbLfbnybjKslGeGsbOvfbs=1572742889;float YKnbKzXhPBkXWmFAlnGaLxHZrrsoFx=882821019.576368687274627583195045261452f;if (YKnbKzXhPBkXWmFAlnGaLxHZrrsoFx - YKnbKzXhPBkXWmFAlnGaLxHZrrsoFx> 0.00000001 ) YKnbKzXhPBkXWmFAlnGaLxHZrrsoFx=1456228738.644988549085188883357488463210f; else YKnbKzXhPBkXWmFAlnGaLxHZrrsoFx=1455080289.834574085914412197849811526753f;if (YKnbKzXhPBkXWmFAlnGaLxHZrrsoFx - YKnbKzXhPBkXWmFAlnGaLxHZrrsoFx> 0.00000001 ) YKnbKzXhPBkXWmFAlnGaLxHZrrsoFx=1899438126.634935162581944408637657203315f; else YKnbKzXhPBkXWmFAlnGaLxHZrrsoFx=1710955823.933248169704636808822813609774f;if (YKnbKzXhPBkXWmFAlnGaLxHZrrsoFx - YKnbKzXhPBkXWmFAlnGaLxHZrrsoFx> 0.00000001 ) YKnbKzXhPBkXWmFAlnGaLxHZrrsoFx=867634506.386776871645093312241560413074f; else YKnbKzXhPBkXWmFAlnGaLxHZrrsoFx=1540526684.059552997113828892030754483647f;if (YKnbKzXhPBkXWmFAlnGaLxHZrrsoFx - YKnbKzXhPBkXWmFAlnGaLxHZrrsoFx> 0.00000001 ) YKnbKzXhPBkXWmFAlnGaLxHZrrsoFx=1687048390.491770655153614885262104530627f; else YKnbKzXhPBkXWmFAlnGaLxHZrrsoFx=1898936226.035182216925899725061858539834f;if (YKnbKzXhPBkXWmFAlnGaLxHZrrsoFx - YKnbKzXhPBkXWmFAlnGaLxHZrrsoFx> 0.00000001 ) YKnbKzXhPBkXWmFAlnGaLxHZrrsoFx=349886860.113922853267045731582567639765f; else YKnbKzXhPBkXWmFAlnGaLxHZrrsoFx=347281785.459292208707280591481835265739f;if (YKnbKzXhPBkXWmFAlnGaLxHZrrsoFx - YKnbKzXhPBkXWmFAlnGaLxHZrrsoFx> 0.00000001 ) YKnbKzXhPBkXWmFAlnGaLxHZrrsoFx=1743366701.410892886364399639725848563413f; else YKnbKzXhPBkXWmFAlnGaLxHZrrsoFx=1072872517.758949759971613755774711568244f;long UkQfEgUbzsrwpaRNLcoEqoUvDXgWoF=353317865;if (UkQfEgUbzsrwpaRNLcoEqoUvDXgWoF == UkQfEgUbzsrwpaRNLcoEqoUvDXgWoF- 0 ) UkQfEgUbzsrwpaRNLcoEqoUvDXgWoF=756322739; else UkQfEgUbzsrwpaRNLcoEqoUvDXgWoF=1891508114;if (UkQfEgUbzsrwpaRNLcoEqoUvDXgWoF == UkQfEgUbzsrwpaRNLcoEqoUvDXgWoF- 1 ) UkQfEgUbzsrwpaRNLcoEqoUvDXgWoF=1349978497; else UkQfEgUbzsrwpaRNLcoEqoUvDXgWoF=1005338207;if (UkQfEgUbzsrwpaRNLcoEqoUvDXgWoF == UkQfEgUbzsrwpaRNLcoEqoUvDXgWoF- 0 ) UkQfEgUbzsrwpaRNLcoEqoUvDXgWoF=566421686; else UkQfEgUbzsrwpaRNLcoEqoUvDXgWoF=597400142;if (UkQfEgUbzsrwpaRNLcoEqoUvDXgWoF == UkQfEgUbzsrwpaRNLcoEqoUvDXgWoF- 0 ) UkQfEgUbzsrwpaRNLcoEqoUvDXgWoF=707351844; else UkQfEgUbzsrwpaRNLcoEqoUvDXgWoF=1461885011;if (UkQfEgUbzsrwpaRNLcoEqoUvDXgWoF == UkQfEgUbzsrwpaRNLcoEqoUvDXgWoF- 1 ) UkQfEgUbzsrwpaRNLcoEqoUvDXgWoF=1191113150; else UkQfEgUbzsrwpaRNLcoEqoUvDXgWoF=1484865006;if (UkQfEgUbzsrwpaRNLcoEqoUvDXgWoF == UkQfEgUbzsrwpaRNLcoEqoUvDXgWoF- 0 ) UkQfEgUbzsrwpaRNLcoEqoUvDXgWoF=1184967289; else UkQfEgUbzsrwpaRNLcoEqoUvDXgWoF=740418498;double semaAurwQGaRIgkzSqmSySJRIBdrhL=1548321939.597685157360767385536890627243;if (semaAurwQGaRIgkzSqmSySJRIBdrhL == semaAurwQGaRIgkzSqmSySJRIBdrhL ) semaAurwQGaRIgkzSqmSySJRIBdrhL=1726409516.842142159650808712539329370807; else semaAurwQGaRIgkzSqmSySJRIBdrhL=1238658013.801563962524699610202572463397;if (semaAurwQGaRIgkzSqmSySJRIBdrhL == semaAurwQGaRIgkzSqmSySJRIBdrhL ) semaAurwQGaRIgkzSqmSySJRIBdrhL=1462433228.246386035695001673177161533010; else semaAurwQGaRIgkzSqmSySJRIBdrhL=162223708.068648832478671868199122029595;if (semaAurwQGaRIgkzSqmSySJRIBdrhL == semaAurwQGaRIgkzSqmSySJRIBdrhL ) semaAurwQGaRIgkzSqmSySJRIBdrhL=1065701283.303573857385180060088503205762; else semaAurwQGaRIgkzSqmSySJRIBdrhL=571635644.045718610911281505216951791047;if (semaAurwQGaRIgkzSqmSySJRIBdrhL == semaAurwQGaRIgkzSqmSySJRIBdrhL ) semaAurwQGaRIgkzSqmSySJRIBdrhL=962156061.920545058355028832371155629413; else semaAurwQGaRIgkzSqmSySJRIBdrhL=14048705.768309285756797294406987797593;if (semaAurwQGaRIgkzSqmSySJRIBdrhL == semaAurwQGaRIgkzSqmSySJRIBdrhL ) semaAurwQGaRIgkzSqmSySJRIBdrhL=1809238247.774669872966873646953501799272; else semaAurwQGaRIgkzSqmSySJRIBdrhL=788081838.370309224943264122426174127498;if (semaAurwQGaRIgkzSqmSySJRIBdrhL == semaAurwQGaRIgkzSqmSySJRIBdrhL ) semaAurwQGaRIgkzSqmSySJRIBdrhL=1292534121.071160757442302145777885720639; else semaAurwQGaRIgkzSqmSySJRIBdrhL=2060361230.394698220557618497319273239755;int DOmFetjHSuKbMKyLklNRPQKBGIDXgu=1794704064;if (DOmFetjHSuKbMKyLklNRPQKBGIDXgu == DOmFetjHSuKbMKyLklNRPQKBGIDXgu- 1 ) DOmFetjHSuKbMKyLklNRPQKBGIDXgu=1974122424; else DOmFetjHSuKbMKyLklNRPQKBGIDXgu=2059687057;if (DOmFetjHSuKbMKyLklNRPQKBGIDXgu == DOmFetjHSuKbMKyLklNRPQKBGIDXgu- 1 ) DOmFetjHSuKbMKyLklNRPQKBGIDXgu=816420150; else DOmFetjHSuKbMKyLklNRPQKBGIDXgu=203288084;if (DOmFetjHSuKbMKyLklNRPQKBGIDXgu == DOmFetjHSuKbMKyLklNRPQKBGIDXgu- 1 ) DOmFetjHSuKbMKyLklNRPQKBGIDXgu=758093976; else DOmFetjHSuKbMKyLklNRPQKBGIDXgu=148296939;if (DOmFetjHSuKbMKyLklNRPQKBGIDXgu == DOmFetjHSuKbMKyLklNRPQKBGIDXgu- 1 ) DOmFetjHSuKbMKyLklNRPQKBGIDXgu=316399333; else DOmFetjHSuKbMKyLklNRPQKBGIDXgu=858828596;if (DOmFetjHSuKbMKyLklNRPQKBGIDXgu == DOmFetjHSuKbMKyLklNRPQKBGIDXgu- 1 ) DOmFetjHSuKbMKyLklNRPQKBGIDXgu=747728445; else DOmFetjHSuKbMKyLklNRPQKBGIDXgu=437553115;if (DOmFetjHSuKbMKyLklNRPQKBGIDXgu == DOmFetjHSuKbMKyLklNRPQKBGIDXgu- 1 ) DOmFetjHSuKbMKyLklNRPQKBGIDXgu=519842673; else DOmFetjHSuKbMKyLklNRPQKBGIDXgu=1530253147;int kiuWtvRSXCwjHUUIeDXdvMriKKhqbi=309093886;if (kiuWtvRSXCwjHUUIeDXdvMriKKhqbi == kiuWtvRSXCwjHUUIeDXdvMriKKhqbi- 0 ) kiuWtvRSXCwjHUUIeDXdvMriKKhqbi=1507829246; else kiuWtvRSXCwjHUUIeDXdvMriKKhqbi=195044762;if (kiuWtvRSXCwjHUUIeDXdvMriKKhqbi == kiuWtvRSXCwjHUUIeDXdvMriKKhqbi- 0 ) kiuWtvRSXCwjHUUIeDXdvMriKKhqbi=1493946648; else kiuWtvRSXCwjHUUIeDXdvMriKKhqbi=1856259914;if (kiuWtvRSXCwjHUUIeDXdvMriKKhqbi == kiuWtvRSXCwjHUUIeDXdvMriKKhqbi- 1 ) kiuWtvRSXCwjHUUIeDXdvMriKKhqbi=775371215; else kiuWtvRSXCwjHUUIeDXdvMriKKhqbi=1144625022;if (kiuWtvRSXCwjHUUIeDXdvMriKKhqbi == kiuWtvRSXCwjHUUIeDXdvMriKKhqbi- 0 ) kiuWtvRSXCwjHUUIeDXdvMriKKhqbi=1435100229; else kiuWtvRSXCwjHUUIeDXdvMriKKhqbi=184687095;if (kiuWtvRSXCwjHUUIeDXdvMriKKhqbi == kiuWtvRSXCwjHUUIeDXdvMriKKhqbi- 1 ) kiuWtvRSXCwjHUUIeDXdvMriKKhqbi=929352053; else kiuWtvRSXCwjHUUIeDXdvMriKKhqbi=1335929792;if (kiuWtvRSXCwjHUUIeDXdvMriKKhqbi == kiuWtvRSXCwjHUUIeDXdvMriKKhqbi- 1 ) kiuWtvRSXCwjHUUIeDXdvMriKKhqbi=1343583607; else kiuWtvRSXCwjHUUIeDXdvMriKKhqbi=1925823495;double HgzmKHJraEyAkBkhbMGBnnzPDxVenA=139175591.834595163157947229901831211917;if (HgzmKHJraEyAkBkhbMGBnnzPDxVenA == HgzmKHJraEyAkBkhbMGBnnzPDxVenA ) HgzmKHJraEyAkBkhbMGBnnzPDxVenA=1356125372.069747695736021122822997501448; else HgzmKHJraEyAkBkhbMGBnnzPDxVenA=1537746872.425720930063427592010010517235;if (HgzmKHJraEyAkBkhbMGBnnzPDxVenA == HgzmKHJraEyAkBkhbMGBnnzPDxVenA ) HgzmKHJraEyAkBkhbMGBnnzPDxVenA=913779721.726741130139286957102044942621; else HgzmKHJraEyAkBkhbMGBnnzPDxVenA=2146857752.381644879746209293467023389390;if (HgzmKHJraEyAkBkhbMGBnnzPDxVenA == HgzmKHJraEyAkBkhbMGBnnzPDxVenA ) HgzmKHJraEyAkBkhbMGBnnzPDxVenA=1371856471.061986865329946685576937536846; else HgzmKHJraEyAkBkhbMGBnnzPDxVenA=783648102.658079532443824828880483414555;if (HgzmKHJraEyAkBkhbMGBnnzPDxVenA == HgzmKHJraEyAkBkhbMGBnnzPDxVenA ) HgzmKHJraEyAkBkhbMGBnnzPDxVenA=790227963.013978108715424312372906628667; else HgzmKHJraEyAkBkhbMGBnnzPDxVenA=1578112577.619753105783127616910238411736;if (HgzmKHJraEyAkBkhbMGBnnzPDxVenA == HgzmKHJraEyAkBkhbMGBnnzPDxVenA ) HgzmKHJraEyAkBkhbMGBnnzPDxVenA=922258680.054111778122869099012285848134; else HgzmKHJraEyAkBkhbMGBnnzPDxVenA=603604632.257074001694990857953372598938;if (HgzmKHJraEyAkBkhbMGBnnzPDxVenA == HgzmKHJraEyAkBkhbMGBnnzPDxVenA ) HgzmKHJraEyAkBkhbMGBnnzPDxVenA=1561978910.465598589619952840600239108559; else HgzmKHJraEyAkBkhbMGBnnzPDxVenA=628148168.617434050237834553175934824640;int APCyyGYIrpwxXlORwYfZZYqSppTdDA=862608355;if (APCyyGYIrpwxXlORwYfZZYqSppTdDA == APCyyGYIrpwxXlORwYfZZYqSppTdDA- 0 ) APCyyGYIrpwxXlORwYfZZYqSppTdDA=948206774; else APCyyGYIrpwxXlORwYfZZYqSppTdDA=983440235;if (APCyyGYIrpwxXlORwYfZZYqSppTdDA == APCyyGYIrpwxXlORwYfZZYqSppTdDA- 1 ) APCyyGYIrpwxXlORwYfZZYqSppTdDA=552531984; else APCyyGYIrpwxXlORwYfZZYqSppTdDA=1900547584;if (APCyyGYIrpwxXlORwYfZZYqSppTdDA == APCyyGYIrpwxXlORwYfZZYqSppTdDA- 0 ) APCyyGYIrpwxXlORwYfZZYqSppTdDA=1705474558; else APCyyGYIrpwxXlORwYfZZYqSppTdDA=419963367;if (APCyyGYIrpwxXlORwYfZZYqSppTdDA == APCyyGYIrpwxXlORwYfZZYqSppTdDA- 0 ) APCyyGYIrpwxXlORwYfZZYqSppTdDA=1019332849; else APCyyGYIrpwxXlORwYfZZYqSppTdDA=829497402;if (APCyyGYIrpwxXlORwYfZZYqSppTdDA == APCyyGYIrpwxXlORwYfZZYqSppTdDA- 0 ) APCyyGYIrpwxXlORwYfZZYqSppTdDA=624586828; else APCyyGYIrpwxXlORwYfZZYqSppTdDA=631733055;if (APCyyGYIrpwxXlORwYfZZYqSppTdDA == APCyyGYIrpwxXlORwYfZZYqSppTdDA- 1 ) APCyyGYIrpwxXlORwYfZZYqSppTdDA=1421419678; else APCyyGYIrpwxXlORwYfZZYqSppTdDA=1569647831;double OOgXTwInpLALumFfZhKRdCoskqKbpa=2100737831.471465435806635131483880220172;if (OOgXTwInpLALumFfZhKRdCoskqKbpa == OOgXTwInpLALumFfZhKRdCoskqKbpa ) OOgXTwInpLALumFfZhKRdCoskqKbpa=1071260450.394035295508929193142881469234; else OOgXTwInpLALumFfZhKRdCoskqKbpa=380667082.636462371094609507114714051368;if (OOgXTwInpLALumFfZhKRdCoskqKbpa == OOgXTwInpLALumFfZhKRdCoskqKbpa ) OOgXTwInpLALumFfZhKRdCoskqKbpa=174127775.706297357892315828307322193341; else OOgXTwInpLALumFfZhKRdCoskqKbpa=965296170.201340952235143240933317258520;if (OOgXTwInpLALumFfZhKRdCoskqKbpa == OOgXTwInpLALumFfZhKRdCoskqKbpa ) OOgXTwInpLALumFfZhKRdCoskqKbpa=32850171.502669108608727229095342899520; else OOgXTwInpLALumFfZhKRdCoskqKbpa=1987984606.243273458415343660003053537088;if (OOgXTwInpLALumFfZhKRdCoskqKbpa == OOgXTwInpLALumFfZhKRdCoskqKbpa ) OOgXTwInpLALumFfZhKRdCoskqKbpa=1084721062.078856565588278100033052820075; else OOgXTwInpLALumFfZhKRdCoskqKbpa=724181144.658045760711653598653715512613;if (OOgXTwInpLALumFfZhKRdCoskqKbpa == OOgXTwInpLALumFfZhKRdCoskqKbpa ) OOgXTwInpLALumFfZhKRdCoskqKbpa=1552300446.760104024445133351034253294363; else OOgXTwInpLALumFfZhKRdCoskqKbpa=2063317718.325816256057521979337278951791;if (OOgXTwInpLALumFfZhKRdCoskqKbpa == OOgXTwInpLALumFfZhKRdCoskqKbpa ) OOgXTwInpLALumFfZhKRdCoskqKbpa=934001234.087113032820508783242613725897; else OOgXTwInpLALumFfZhKRdCoskqKbpa=171766583.557608142894752787027866947450;long MXcOiRtAAhUHWbpxeHyUrmjxmwFjfE=1497449139;if (MXcOiRtAAhUHWbpxeHyUrmjxmwFjfE == MXcOiRtAAhUHWbpxeHyUrmjxmwFjfE- 0 ) MXcOiRtAAhUHWbpxeHyUrmjxmwFjfE=1677440482; else MXcOiRtAAhUHWbpxeHyUrmjxmwFjfE=480975252;if (MXcOiRtAAhUHWbpxeHyUrmjxmwFjfE == MXcOiRtAAhUHWbpxeHyUrmjxmwFjfE- 0 ) MXcOiRtAAhUHWbpxeHyUrmjxmwFjfE=977616721; else MXcOiRtAAhUHWbpxeHyUrmjxmwFjfE=468909433;if (MXcOiRtAAhUHWbpxeHyUrmjxmwFjfE == MXcOiRtAAhUHWbpxeHyUrmjxmwFjfE- 1 ) MXcOiRtAAhUHWbpxeHyUrmjxmwFjfE=241942277; else MXcOiRtAAhUHWbpxeHyUrmjxmwFjfE=1760712375;if (MXcOiRtAAhUHWbpxeHyUrmjxmwFjfE == MXcOiRtAAhUHWbpxeHyUrmjxmwFjfE- 0 ) MXcOiRtAAhUHWbpxeHyUrmjxmwFjfE=581184680; else MXcOiRtAAhUHWbpxeHyUrmjxmwFjfE=936162005;if (MXcOiRtAAhUHWbpxeHyUrmjxmwFjfE == MXcOiRtAAhUHWbpxeHyUrmjxmwFjfE- 1 ) MXcOiRtAAhUHWbpxeHyUrmjxmwFjfE=518414023; else MXcOiRtAAhUHWbpxeHyUrmjxmwFjfE=84282319;if (MXcOiRtAAhUHWbpxeHyUrmjxmwFjfE == MXcOiRtAAhUHWbpxeHyUrmjxmwFjfE- 0 ) MXcOiRtAAhUHWbpxeHyUrmjxmwFjfE=1522270924; else MXcOiRtAAhUHWbpxeHyUrmjxmwFjfE=740123416;int YYSMtuRXvFsckXRWShNqUosqOLRMNo=384199195;if (YYSMtuRXvFsckXRWShNqUosqOLRMNo == YYSMtuRXvFsckXRWShNqUosqOLRMNo- 1 ) YYSMtuRXvFsckXRWShNqUosqOLRMNo=1197125704; else YYSMtuRXvFsckXRWShNqUosqOLRMNo=1608724726;if (YYSMtuRXvFsckXRWShNqUosqOLRMNo == YYSMtuRXvFsckXRWShNqUosqOLRMNo- 1 ) YYSMtuRXvFsckXRWShNqUosqOLRMNo=1704371365; else YYSMtuRXvFsckXRWShNqUosqOLRMNo=1818609536;if (YYSMtuRXvFsckXRWShNqUosqOLRMNo == YYSMtuRXvFsckXRWShNqUosqOLRMNo- 1 ) YYSMtuRXvFsckXRWShNqUosqOLRMNo=1405258786; else YYSMtuRXvFsckXRWShNqUosqOLRMNo=840676787;if (YYSMtuRXvFsckXRWShNqUosqOLRMNo == YYSMtuRXvFsckXRWShNqUosqOLRMNo- 1 ) YYSMtuRXvFsckXRWShNqUosqOLRMNo=829758184; else YYSMtuRXvFsckXRWShNqUosqOLRMNo=1107705815;if (YYSMtuRXvFsckXRWShNqUosqOLRMNo == YYSMtuRXvFsckXRWShNqUosqOLRMNo- 0 ) YYSMtuRXvFsckXRWShNqUosqOLRMNo=2046202820; else YYSMtuRXvFsckXRWShNqUosqOLRMNo=1975834487;if (YYSMtuRXvFsckXRWShNqUosqOLRMNo == YYSMtuRXvFsckXRWShNqUosqOLRMNo- 0 ) YYSMtuRXvFsckXRWShNqUosqOLRMNo=348247138; else YYSMtuRXvFsckXRWShNqUosqOLRMNo=1789640487;float ezMSalqyfJjaXlvMilkNfrSXAZpcFa=1965118414.649894487295939584492404307689f;if (ezMSalqyfJjaXlvMilkNfrSXAZpcFa - ezMSalqyfJjaXlvMilkNfrSXAZpcFa> 0.00000001 ) ezMSalqyfJjaXlvMilkNfrSXAZpcFa=498838931.193487339997445746323102225660f; else ezMSalqyfJjaXlvMilkNfrSXAZpcFa=2071454186.890911356226551516066137710136f;if (ezMSalqyfJjaXlvMilkNfrSXAZpcFa - ezMSalqyfJjaXlvMilkNfrSXAZpcFa> 0.00000001 ) ezMSalqyfJjaXlvMilkNfrSXAZpcFa=2039690996.388910753459110765272970577331f; else ezMSalqyfJjaXlvMilkNfrSXAZpcFa=471221727.223005974296461712805506542855f;if (ezMSalqyfJjaXlvMilkNfrSXAZpcFa - ezMSalqyfJjaXlvMilkNfrSXAZpcFa> 0.00000001 ) ezMSalqyfJjaXlvMilkNfrSXAZpcFa=1707926411.453334712590119662987563672867f; else ezMSalqyfJjaXlvMilkNfrSXAZpcFa=1085053142.412085423810131640799097999076f;if (ezMSalqyfJjaXlvMilkNfrSXAZpcFa - ezMSalqyfJjaXlvMilkNfrSXAZpcFa> 0.00000001 ) ezMSalqyfJjaXlvMilkNfrSXAZpcFa=1858795554.395529823535928753314218943620f; else ezMSalqyfJjaXlvMilkNfrSXAZpcFa=1458031722.645762561383826427925687319879f;if (ezMSalqyfJjaXlvMilkNfrSXAZpcFa - ezMSalqyfJjaXlvMilkNfrSXAZpcFa> 0.00000001 ) ezMSalqyfJjaXlvMilkNfrSXAZpcFa=1946966049.631409037720203150253670507945f; else ezMSalqyfJjaXlvMilkNfrSXAZpcFa=1385449417.489376343859131548436973617190f;if (ezMSalqyfJjaXlvMilkNfrSXAZpcFa - ezMSalqyfJjaXlvMilkNfrSXAZpcFa> 0.00000001 ) ezMSalqyfJjaXlvMilkNfrSXAZpcFa=446882147.329245103892741454017952052786f; else ezMSalqyfJjaXlvMilkNfrSXAZpcFa=2096051499.026464190755674533336329447839f;int TKirigSrjRgWQeHRtoaJsXYAjCjxKj=184236667;if (TKirigSrjRgWQeHRtoaJsXYAjCjxKj == TKirigSrjRgWQeHRtoaJsXYAjCjxKj- 0 ) TKirigSrjRgWQeHRtoaJsXYAjCjxKj=1437402120; else TKirigSrjRgWQeHRtoaJsXYAjCjxKj=549802039;if (TKirigSrjRgWQeHRtoaJsXYAjCjxKj == TKirigSrjRgWQeHRtoaJsXYAjCjxKj- 0 ) TKirigSrjRgWQeHRtoaJsXYAjCjxKj=1122124526; else TKirigSrjRgWQeHRtoaJsXYAjCjxKj=121405033;if (TKirigSrjRgWQeHRtoaJsXYAjCjxKj == TKirigSrjRgWQeHRtoaJsXYAjCjxKj- 1 ) TKirigSrjRgWQeHRtoaJsXYAjCjxKj=1250064343; else TKirigSrjRgWQeHRtoaJsXYAjCjxKj=461768235;if (TKirigSrjRgWQeHRtoaJsXYAjCjxKj == TKirigSrjRgWQeHRtoaJsXYAjCjxKj- 1 ) TKirigSrjRgWQeHRtoaJsXYAjCjxKj=92190353; else TKirigSrjRgWQeHRtoaJsXYAjCjxKj=836763695;if (TKirigSrjRgWQeHRtoaJsXYAjCjxKj == TKirigSrjRgWQeHRtoaJsXYAjCjxKj- 0 ) TKirigSrjRgWQeHRtoaJsXYAjCjxKj=1798700715; else TKirigSrjRgWQeHRtoaJsXYAjCjxKj=2017394553;if (TKirigSrjRgWQeHRtoaJsXYAjCjxKj == TKirigSrjRgWQeHRtoaJsXYAjCjxKj- 1 ) TKirigSrjRgWQeHRtoaJsXYAjCjxKj=570190260; else TKirigSrjRgWQeHRtoaJsXYAjCjxKj=413591437;long wGZnghfawaMLzxbAkokvZtInEOlMrk=1818331404;if (wGZnghfawaMLzxbAkokvZtInEOlMrk == wGZnghfawaMLzxbAkokvZtInEOlMrk- 1 ) wGZnghfawaMLzxbAkokvZtInEOlMrk=984914676; else wGZnghfawaMLzxbAkokvZtInEOlMrk=1704636157;if (wGZnghfawaMLzxbAkokvZtInEOlMrk == wGZnghfawaMLzxbAkokvZtInEOlMrk- 0 ) wGZnghfawaMLzxbAkokvZtInEOlMrk=241284485; else wGZnghfawaMLzxbAkokvZtInEOlMrk=1585741532;if (wGZnghfawaMLzxbAkokvZtInEOlMrk == wGZnghfawaMLzxbAkokvZtInEOlMrk- 1 ) wGZnghfawaMLzxbAkokvZtInEOlMrk=1475856430; else wGZnghfawaMLzxbAkokvZtInEOlMrk=154693871;if (wGZnghfawaMLzxbAkokvZtInEOlMrk == wGZnghfawaMLzxbAkokvZtInEOlMrk- 0 ) wGZnghfawaMLzxbAkokvZtInEOlMrk=823756431; else wGZnghfawaMLzxbAkokvZtInEOlMrk=112072562;if (wGZnghfawaMLzxbAkokvZtInEOlMrk == wGZnghfawaMLzxbAkokvZtInEOlMrk- 1 ) wGZnghfawaMLzxbAkokvZtInEOlMrk=1897412819; else wGZnghfawaMLzxbAkokvZtInEOlMrk=246339643;if (wGZnghfawaMLzxbAkokvZtInEOlMrk == wGZnghfawaMLzxbAkokvZtInEOlMrk- 0 ) wGZnghfawaMLzxbAkokvZtInEOlMrk=2019127134; else wGZnghfawaMLzxbAkokvZtInEOlMrk=19809369;long uujuWcJyxebLNUMYsjRAElMlwcKHAY=1701605174;if (uujuWcJyxebLNUMYsjRAElMlwcKHAY == uujuWcJyxebLNUMYsjRAElMlwcKHAY- 0 ) uujuWcJyxebLNUMYsjRAElMlwcKHAY=613881767; else uujuWcJyxebLNUMYsjRAElMlwcKHAY=164010471;if (uujuWcJyxebLNUMYsjRAElMlwcKHAY == uujuWcJyxebLNUMYsjRAElMlwcKHAY- 0 ) uujuWcJyxebLNUMYsjRAElMlwcKHAY=352238367; else uujuWcJyxebLNUMYsjRAElMlwcKHAY=1083643741;if (uujuWcJyxebLNUMYsjRAElMlwcKHAY == uujuWcJyxebLNUMYsjRAElMlwcKHAY- 0 ) uujuWcJyxebLNUMYsjRAElMlwcKHAY=1872925387; else uujuWcJyxebLNUMYsjRAElMlwcKHAY=1125446257;if (uujuWcJyxebLNUMYsjRAElMlwcKHAY == uujuWcJyxebLNUMYsjRAElMlwcKHAY- 1 ) uujuWcJyxebLNUMYsjRAElMlwcKHAY=715054071; else uujuWcJyxebLNUMYsjRAElMlwcKHAY=37070696;if (uujuWcJyxebLNUMYsjRAElMlwcKHAY == uujuWcJyxebLNUMYsjRAElMlwcKHAY- 0 ) uujuWcJyxebLNUMYsjRAElMlwcKHAY=1979263833; else uujuWcJyxebLNUMYsjRAElMlwcKHAY=1202927301;if (uujuWcJyxebLNUMYsjRAElMlwcKHAY == uujuWcJyxebLNUMYsjRAElMlwcKHAY- 0 ) uujuWcJyxebLNUMYsjRAElMlwcKHAY=820004659; else uujuWcJyxebLNUMYsjRAElMlwcKHAY=1838015505;double CcZSfVHrAzVayVqUrNJrlZpelHxwNc=1007938209.413023243997128616234221207313;if (CcZSfVHrAzVayVqUrNJrlZpelHxwNc == CcZSfVHrAzVayVqUrNJrlZpelHxwNc ) CcZSfVHrAzVayVqUrNJrlZpelHxwNc=2082733192.179304737691223795710259120292; else CcZSfVHrAzVayVqUrNJrlZpelHxwNc=776034390.794192883599975310962260393139;if (CcZSfVHrAzVayVqUrNJrlZpelHxwNc == CcZSfVHrAzVayVqUrNJrlZpelHxwNc ) CcZSfVHrAzVayVqUrNJrlZpelHxwNc=1111136823.554590823291233238581044154581; else CcZSfVHrAzVayVqUrNJrlZpelHxwNc=908561172.986504558660887104827675862145;if (CcZSfVHrAzVayVqUrNJrlZpelHxwNc == CcZSfVHrAzVayVqUrNJrlZpelHxwNc ) CcZSfVHrAzVayVqUrNJrlZpelHxwNc=103313161.734332683989103127805087442175; else CcZSfVHrAzVayVqUrNJrlZpelHxwNc=1923694835.341493665563194967384544076890;if (CcZSfVHrAzVayVqUrNJrlZpelHxwNc == CcZSfVHrAzVayVqUrNJrlZpelHxwNc ) CcZSfVHrAzVayVqUrNJrlZpelHxwNc=855863884.161864871157335209368580291743; else CcZSfVHrAzVayVqUrNJrlZpelHxwNc=1282708249.798551334119723832098863268278;if (CcZSfVHrAzVayVqUrNJrlZpelHxwNc == CcZSfVHrAzVayVqUrNJrlZpelHxwNc ) CcZSfVHrAzVayVqUrNJrlZpelHxwNc=1873529255.324857988507526097757086693549; else CcZSfVHrAzVayVqUrNJrlZpelHxwNc=1010051964.417780598795469134828253244926;if (CcZSfVHrAzVayVqUrNJrlZpelHxwNc == CcZSfVHrAzVayVqUrNJrlZpelHxwNc ) CcZSfVHrAzVayVqUrNJrlZpelHxwNc=371475861.455919095760917262756082237565; else CcZSfVHrAzVayVqUrNJrlZpelHxwNc=1003724700.677335165015027935708228047389;double dQACuMdnJFCnnUGCiZOxMqehSXxPuo=917510383.953137671642554922891043963092;if (dQACuMdnJFCnnUGCiZOxMqehSXxPuo == dQACuMdnJFCnnUGCiZOxMqehSXxPuo ) dQACuMdnJFCnnUGCiZOxMqehSXxPuo=1375402420.188827244616622580628820491137; else dQACuMdnJFCnnUGCiZOxMqehSXxPuo=1016068733.468956573879928542149447537853;if (dQACuMdnJFCnnUGCiZOxMqehSXxPuo == dQACuMdnJFCnnUGCiZOxMqehSXxPuo ) dQACuMdnJFCnnUGCiZOxMqehSXxPuo=1947193317.711726878611795901396900812252; else dQACuMdnJFCnnUGCiZOxMqehSXxPuo=1248454766.073127920377553876264667881102;if (dQACuMdnJFCnnUGCiZOxMqehSXxPuo == dQACuMdnJFCnnUGCiZOxMqehSXxPuo ) dQACuMdnJFCnnUGCiZOxMqehSXxPuo=586789473.473774978918102055714710841744; else dQACuMdnJFCnnUGCiZOxMqehSXxPuo=366556131.683991589001835766593650876393;if (dQACuMdnJFCnnUGCiZOxMqehSXxPuo == dQACuMdnJFCnnUGCiZOxMqehSXxPuo ) dQACuMdnJFCnnUGCiZOxMqehSXxPuo=2092889664.323544522158781309529626204422; else dQACuMdnJFCnnUGCiZOxMqehSXxPuo=2006952066.629778840234554607958898581834;if (dQACuMdnJFCnnUGCiZOxMqehSXxPuo == dQACuMdnJFCnnUGCiZOxMqehSXxPuo ) dQACuMdnJFCnnUGCiZOxMqehSXxPuo=989582086.011054911367939518261768594229; else dQACuMdnJFCnnUGCiZOxMqehSXxPuo=204213789.811732460180760296918468346467;if (dQACuMdnJFCnnUGCiZOxMqehSXxPuo == dQACuMdnJFCnnUGCiZOxMqehSXxPuo ) dQACuMdnJFCnnUGCiZOxMqehSXxPuo=1753961346.089365631816829210830832255449; else dQACuMdnJFCnnUGCiZOxMqehSXxPuo=272929129.710311151567620265024663835812;int NtIDDbbGpAfuThQJCKxuzUKaDZkapc=452208702;if (NtIDDbbGpAfuThQJCKxuzUKaDZkapc == NtIDDbbGpAfuThQJCKxuzUKaDZkapc- 0 ) NtIDDbbGpAfuThQJCKxuzUKaDZkapc=200773158; else NtIDDbbGpAfuThQJCKxuzUKaDZkapc=1011188925;if (NtIDDbbGpAfuThQJCKxuzUKaDZkapc == NtIDDbbGpAfuThQJCKxuzUKaDZkapc- 1 ) NtIDDbbGpAfuThQJCKxuzUKaDZkapc=84559824; else NtIDDbbGpAfuThQJCKxuzUKaDZkapc=1765243897;if (NtIDDbbGpAfuThQJCKxuzUKaDZkapc == NtIDDbbGpAfuThQJCKxuzUKaDZkapc- 1 ) NtIDDbbGpAfuThQJCKxuzUKaDZkapc=1917537156; else NtIDDbbGpAfuThQJCKxuzUKaDZkapc=1880914608;if (NtIDDbbGpAfuThQJCKxuzUKaDZkapc == NtIDDbbGpAfuThQJCKxuzUKaDZkapc- 0 ) NtIDDbbGpAfuThQJCKxuzUKaDZkapc=1598292013; else NtIDDbbGpAfuThQJCKxuzUKaDZkapc=837670579;if (NtIDDbbGpAfuThQJCKxuzUKaDZkapc == NtIDDbbGpAfuThQJCKxuzUKaDZkapc- 0 ) NtIDDbbGpAfuThQJCKxuzUKaDZkapc=1455630234; else NtIDDbbGpAfuThQJCKxuzUKaDZkapc=1217013765;if (NtIDDbbGpAfuThQJCKxuzUKaDZkapc == NtIDDbbGpAfuThQJCKxuzUKaDZkapc- 0 ) NtIDDbbGpAfuThQJCKxuzUKaDZkapc=1357598133; else NtIDDbbGpAfuThQJCKxuzUKaDZkapc=862873423;long RMaGASNJhupTcOATLwZOVPpxBpqrfd=1906882849;if (RMaGASNJhupTcOATLwZOVPpxBpqrfd == RMaGASNJhupTcOATLwZOVPpxBpqrfd- 0 ) RMaGASNJhupTcOATLwZOVPpxBpqrfd=759958246; else RMaGASNJhupTcOATLwZOVPpxBpqrfd=2049977923;if (RMaGASNJhupTcOATLwZOVPpxBpqrfd == RMaGASNJhupTcOATLwZOVPpxBpqrfd- 0 ) RMaGASNJhupTcOATLwZOVPpxBpqrfd=462613085; else RMaGASNJhupTcOATLwZOVPpxBpqrfd=1646277796;if (RMaGASNJhupTcOATLwZOVPpxBpqrfd == RMaGASNJhupTcOATLwZOVPpxBpqrfd- 0 ) RMaGASNJhupTcOATLwZOVPpxBpqrfd=1645494447; else RMaGASNJhupTcOATLwZOVPpxBpqrfd=944595366;if (RMaGASNJhupTcOATLwZOVPpxBpqrfd == RMaGASNJhupTcOATLwZOVPpxBpqrfd- 1 ) RMaGASNJhupTcOATLwZOVPpxBpqrfd=421148736; else RMaGASNJhupTcOATLwZOVPpxBpqrfd=1716177061;if (RMaGASNJhupTcOATLwZOVPpxBpqrfd == RMaGASNJhupTcOATLwZOVPpxBpqrfd- 1 ) RMaGASNJhupTcOATLwZOVPpxBpqrfd=830628373; else RMaGASNJhupTcOATLwZOVPpxBpqrfd=1251668404;if (RMaGASNJhupTcOATLwZOVPpxBpqrfd == RMaGASNJhupTcOATLwZOVPpxBpqrfd- 1 ) RMaGASNJhupTcOATLwZOVPpxBpqrfd=2066462733; else RMaGASNJhupTcOATLwZOVPpxBpqrfd=2046583019; }
 RMaGASNJhupTcOATLwZOVPpxBpqrfdy::RMaGASNJhupTcOATLwZOVPpxBpqrfdy()
 { this->gOSdansVVvro("jmJnxpqUaGbIvNdkfNcPHevRBXcFLcgOSdansVVvroj", true, 169733489, 582934755, 1948019037); }
#pragma optimize("", off)
 // <delete/>


// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class HMlwoAXPnpxkdkDuonUnxaVUexxZkay
 { 
public: bool lptoizesecceQdXxUMKCPvduhrQRrD; double lptoizesecceQdXxUMKCPvduhrQRrDHMlwoAXPnpxkdkDuonUnxaVUexxZka; HMlwoAXPnpxkdkDuonUnxaVUexxZkay(); void riMmgBudkLUJ(string lptoizesecceQdXxUMKCPvduhrQRrDriMmgBudkLUJ, bool yfNZMukwVvJupKVvEqHzcMZYAqMyGq, int jNBbexRrRcNYrPsUFSYlmZJralFoYV, float hlpeLvtNgYeNYfmDVKgfkuaUEJEKBt, long rcSvubjMDwbsmAHYAeEWPuItlnLUaC);
 protected: bool lptoizesecceQdXxUMKCPvduhrQRrDo; double lptoizesecceQdXxUMKCPvduhrQRrDHMlwoAXPnpxkdkDuonUnxaVUexxZkaf; void riMmgBudkLUJu(string lptoizesecceQdXxUMKCPvduhrQRrDriMmgBudkLUJg, bool yfNZMukwVvJupKVvEqHzcMZYAqMyGqe, int jNBbexRrRcNYrPsUFSYlmZJralFoYVr, float hlpeLvtNgYeNYfmDVKgfkuaUEJEKBtw, long rcSvubjMDwbsmAHYAeEWPuItlnLUaCn);
 private: bool lptoizesecceQdXxUMKCPvduhrQRrDyfNZMukwVvJupKVvEqHzcMZYAqMyGq; double lptoizesecceQdXxUMKCPvduhrQRrDhlpeLvtNgYeNYfmDVKgfkuaUEJEKBtHMlwoAXPnpxkdkDuonUnxaVUexxZka;
 void riMmgBudkLUJv(string yfNZMukwVvJupKVvEqHzcMZYAqMyGqriMmgBudkLUJ, bool yfNZMukwVvJupKVvEqHzcMZYAqMyGqjNBbexRrRcNYrPsUFSYlmZJralFoYV, int jNBbexRrRcNYrPsUFSYlmZJralFoYVlptoizesecceQdXxUMKCPvduhrQRrD, float hlpeLvtNgYeNYfmDVKgfkuaUEJEKBtrcSvubjMDwbsmAHYAeEWPuItlnLUaC, long rcSvubjMDwbsmAHYAeEWPuItlnLUaCyfNZMukwVvJupKVvEqHzcMZYAqMyGq); };
 void HMlwoAXPnpxkdkDuonUnxaVUexxZkay::riMmgBudkLUJ(string lptoizesecceQdXxUMKCPvduhrQRrDriMmgBudkLUJ, bool yfNZMukwVvJupKVvEqHzcMZYAqMyGq, int jNBbexRrRcNYrPsUFSYlmZJralFoYV, float hlpeLvtNgYeNYfmDVKgfkuaUEJEKBt, long rcSvubjMDwbsmAHYAeEWPuItlnLUaC)
 { int MAuBDZWGMfFiWIVJXJPtegAnKVaQlQ=1703150739;if (MAuBDZWGMfFiWIVJXJPtegAnKVaQlQ == MAuBDZWGMfFiWIVJXJPtegAnKVaQlQ- 0 ) MAuBDZWGMfFiWIVJXJPtegAnKVaQlQ=1426294722; else MAuBDZWGMfFiWIVJXJPtegAnKVaQlQ=622637011;if (MAuBDZWGMfFiWIVJXJPtegAnKVaQlQ == MAuBDZWGMfFiWIVJXJPtegAnKVaQlQ- 1 ) MAuBDZWGMfFiWIVJXJPtegAnKVaQlQ=138574747; else MAuBDZWGMfFiWIVJXJPtegAnKVaQlQ=1674261591;if (MAuBDZWGMfFiWIVJXJPtegAnKVaQlQ == MAuBDZWGMfFiWIVJXJPtegAnKVaQlQ- 0 ) MAuBDZWGMfFiWIVJXJPtegAnKVaQlQ=1508356023; else MAuBDZWGMfFiWIVJXJPtegAnKVaQlQ=525797965;if (MAuBDZWGMfFiWIVJXJPtegAnKVaQlQ == MAuBDZWGMfFiWIVJXJPtegAnKVaQlQ- 0 ) MAuBDZWGMfFiWIVJXJPtegAnKVaQlQ=665800741; else MAuBDZWGMfFiWIVJXJPtegAnKVaQlQ=1027996162;if (MAuBDZWGMfFiWIVJXJPtegAnKVaQlQ == MAuBDZWGMfFiWIVJXJPtegAnKVaQlQ- 1 ) MAuBDZWGMfFiWIVJXJPtegAnKVaQlQ=1515595311; else MAuBDZWGMfFiWIVJXJPtegAnKVaQlQ=173930280;if (MAuBDZWGMfFiWIVJXJPtegAnKVaQlQ == MAuBDZWGMfFiWIVJXJPtegAnKVaQlQ- 1 ) MAuBDZWGMfFiWIVJXJPtegAnKVaQlQ=228794502; else MAuBDZWGMfFiWIVJXJPtegAnKVaQlQ=2123379089;long ntWlhxTXAoWiSIcvAEGdaBXNuKHqHM=1213902588;if (ntWlhxTXAoWiSIcvAEGdaBXNuKHqHM == ntWlhxTXAoWiSIcvAEGdaBXNuKHqHM- 1 ) ntWlhxTXAoWiSIcvAEGdaBXNuKHqHM=1456499273; else ntWlhxTXAoWiSIcvAEGdaBXNuKHqHM=1434962583;if (ntWlhxTXAoWiSIcvAEGdaBXNuKHqHM == ntWlhxTXAoWiSIcvAEGdaBXNuKHqHM- 1 ) ntWlhxTXAoWiSIcvAEGdaBXNuKHqHM=1323611568; else ntWlhxTXAoWiSIcvAEGdaBXNuKHqHM=1532222285;if (ntWlhxTXAoWiSIcvAEGdaBXNuKHqHM == ntWlhxTXAoWiSIcvAEGdaBXNuKHqHM- 1 ) ntWlhxTXAoWiSIcvAEGdaBXNuKHqHM=1414364595; else ntWlhxTXAoWiSIcvAEGdaBXNuKHqHM=1516428648;if (ntWlhxTXAoWiSIcvAEGdaBXNuKHqHM == ntWlhxTXAoWiSIcvAEGdaBXNuKHqHM- 0 ) ntWlhxTXAoWiSIcvAEGdaBXNuKHqHM=1024705656; else ntWlhxTXAoWiSIcvAEGdaBXNuKHqHM=254809006;if (ntWlhxTXAoWiSIcvAEGdaBXNuKHqHM == ntWlhxTXAoWiSIcvAEGdaBXNuKHqHM- 0 ) ntWlhxTXAoWiSIcvAEGdaBXNuKHqHM=1053537305; else ntWlhxTXAoWiSIcvAEGdaBXNuKHqHM=1820972979;if (ntWlhxTXAoWiSIcvAEGdaBXNuKHqHM == ntWlhxTXAoWiSIcvAEGdaBXNuKHqHM- 0 ) ntWlhxTXAoWiSIcvAEGdaBXNuKHqHM=1100767909; else ntWlhxTXAoWiSIcvAEGdaBXNuKHqHM=1698137821;double RhgMNxvnNOWLpwPPQBSdfpHADsLlKj=859269811.507385481771567415349725427202;if (RhgMNxvnNOWLpwPPQBSdfpHADsLlKj == RhgMNxvnNOWLpwPPQBSdfpHADsLlKj ) RhgMNxvnNOWLpwPPQBSdfpHADsLlKj=29190586.730013709092465115745990888306; else RhgMNxvnNOWLpwPPQBSdfpHADsLlKj=1452848617.938914155572736747142256784124;if (RhgMNxvnNOWLpwPPQBSdfpHADsLlKj == RhgMNxvnNOWLpwPPQBSdfpHADsLlKj ) RhgMNxvnNOWLpwPPQBSdfpHADsLlKj=422192809.550327555107037337425127578233; else RhgMNxvnNOWLpwPPQBSdfpHADsLlKj=1528553132.013203242039204633946871053495;if (RhgMNxvnNOWLpwPPQBSdfpHADsLlKj == RhgMNxvnNOWLpwPPQBSdfpHADsLlKj ) RhgMNxvnNOWLpwPPQBSdfpHADsLlKj=403448870.933319494529650693395539835235; else RhgMNxvnNOWLpwPPQBSdfpHADsLlKj=1305879307.195710700525053732246010050559;if (RhgMNxvnNOWLpwPPQBSdfpHADsLlKj == RhgMNxvnNOWLpwPPQBSdfpHADsLlKj ) RhgMNxvnNOWLpwPPQBSdfpHADsLlKj=1092823868.527944594000498169620319018687; else RhgMNxvnNOWLpwPPQBSdfpHADsLlKj=609435608.035100911035935673309294922894;if (RhgMNxvnNOWLpwPPQBSdfpHADsLlKj == RhgMNxvnNOWLpwPPQBSdfpHADsLlKj ) RhgMNxvnNOWLpwPPQBSdfpHADsLlKj=373952315.692807539835860659545548894239; else RhgMNxvnNOWLpwPPQBSdfpHADsLlKj=1318732272.912844313824458443538446026412;if (RhgMNxvnNOWLpwPPQBSdfpHADsLlKj == RhgMNxvnNOWLpwPPQBSdfpHADsLlKj ) RhgMNxvnNOWLpwPPQBSdfpHADsLlKj=376379778.441102413645673129145281158783; else RhgMNxvnNOWLpwPPQBSdfpHADsLlKj=781550121.801259445614190487300747736105;int iQxqVTXNVgoTbPHDuEolBSqODeVWJL=330509613;if (iQxqVTXNVgoTbPHDuEolBSqODeVWJL == iQxqVTXNVgoTbPHDuEolBSqODeVWJL- 0 ) iQxqVTXNVgoTbPHDuEolBSqODeVWJL=593581619; else iQxqVTXNVgoTbPHDuEolBSqODeVWJL=1024769346;if (iQxqVTXNVgoTbPHDuEolBSqODeVWJL == iQxqVTXNVgoTbPHDuEolBSqODeVWJL- 0 ) iQxqVTXNVgoTbPHDuEolBSqODeVWJL=664190193; else iQxqVTXNVgoTbPHDuEolBSqODeVWJL=1377007922;if (iQxqVTXNVgoTbPHDuEolBSqODeVWJL == iQxqVTXNVgoTbPHDuEolBSqODeVWJL- 1 ) iQxqVTXNVgoTbPHDuEolBSqODeVWJL=1947484351; else iQxqVTXNVgoTbPHDuEolBSqODeVWJL=1200770549;if (iQxqVTXNVgoTbPHDuEolBSqODeVWJL == iQxqVTXNVgoTbPHDuEolBSqODeVWJL- 0 ) iQxqVTXNVgoTbPHDuEolBSqODeVWJL=288786144; else iQxqVTXNVgoTbPHDuEolBSqODeVWJL=2057349721;if (iQxqVTXNVgoTbPHDuEolBSqODeVWJL == iQxqVTXNVgoTbPHDuEolBSqODeVWJL- 1 ) iQxqVTXNVgoTbPHDuEolBSqODeVWJL=1745415002; else iQxqVTXNVgoTbPHDuEolBSqODeVWJL=2031249388;if (iQxqVTXNVgoTbPHDuEolBSqODeVWJL == iQxqVTXNVgoTbPHDuEolBSqODeVWJL- 0 ) iQxqVTXNVgoTbPHDuEolBSqODeVWJL=620724286; else iQxqVTXNVgoTbPHDuEolBSqODeVWJL=1834866691;float CZHIqYLHYHlTDipxKmIlVMfLXMvpWE=682406233.274560736100251274165842199981f;if (CZHIqYLHYHlTDipxKmIlVMfLXMvpWE - CZHIqYLHYHlTDipxKmIlVMfLXMvpWE> 0.00000001 ) CZHIqYLHYHlTDipxKmIlVMfLXMvpWE=858959022.151659544493824466542088943248f; else CZHIqYLHYHlTDipxKmIlVMfLXMvpWE=1517008338.186516070273177313883455027472f;if (CZHIqYLHYHlTDipxKmIlVMfLXMvpWE - CZHIqYLHYHlTDipxKmIlVMfLXMvpWE> 0.00000001 ) CZHIqYLHYHlTDipxKmIlVMfLXMvpWE=2094188503.740632742950637991174147584259f; else CZHIqYLHYHlTDipxKmIlVMfLXMvpWE=2034642321.960964455667103482813641960416f;if (CZHIqYLHYHlTDipxKmIlVMfLXMvpWE - CZHIqYLHYHlTDipxKmIlVMfLXMvpWE> 0.00000001 ) CZHIqYLHYHlTDipxKmIlVMfLXMvpWE=1978416429.869309187365752705421345643000f; else CZHIqYLHYHlTDipxKmIlVMfLXMvpWE=1832046348.504991880530566990026500663577f;if (CZHIqYLHYHlTDipxKmIlVMfLXMvpWE - CZHIqYLHYHlTDipxKmIlVMfLXMvpWE> 0.00000001 ) CZHIqYLHYHlTDipxKmIlVMfLXMvpWE=810453718.628896196103509189985409401534f; else CZHIqYLHYHlTDipxKmIlVMfLXMvpWE=150619966.842492287091493410158570232049f;if (CZHIqYLHYHlTDipxKmIlVMfLXMvpWE - CZHIqYLHYHlTDipxKmIlVMfLXMvpWE> 0.00000001 ) CZHIqYLHYHlTDipxKmIlVMfLXMvpWE=342138715.666247212700290512719158647236f; else CZHIqYLHYHlTDipxKmIlVMfLXMvpWE=714175960.134853526731946838173044152860f;if (CZHIqYLHYHlTDipxKmIlVMfLXMvpWE - CZHIqYLHYHlTDipxKmIlVMfLXMvpWE> 0.00000001 ) CZHIqYLHYHlTDipxKmIlVMfLXMvpWE=1158269970.008677644719718931382017420181f; else CZHIqYLHYHlTDipxKmIlVMfLXMvpWE=95632323.176683338030446433622269909235f;float bdSgSNFQYgAQPeKflCyshZitvGnmnV=1076165464.184364724200795854677383783300f;if (bdSgSNFQYgAQPeKflCyshZitvGnmnV - bdSgSNFQYgAQPeKflCyshZitvGnmnV> 0.00000001 ) bdSgSNFQYgAQPeKflCyshZitvGnmnV=2055517996.215040554276143682700816390168f; else bdSgSNFQYgAQPeKflCyshZitvGnmnV=770258296.153519675540151125121261140086f;if (bdSgSNFQYgAQPeKflCyshZitvGnmnV - bdSgSNFQYgAQPeKflCyshZitvGnmnV> 0.00000001 ) bdSgSNFQYgAQPeKflCyshZitvGnmnV=1104556202.149613473401826077637615709247f; else bdSgSNFQYgAQPeKflCyshZitvGnmnV=902690676.070050264876272055518346954492f;if (bdSgSNFQYgAQPeKflCyshZitvGnmnV - bdSgSNFQYgAQPeKflCyshZitvGnmnV> 0.00000001 ) bdSgSNFQYgAQPeKflCyshZitvGnmnV=168998885.523654287692486245476584073511f; else bdSgSNFQYgAQPeKflCyshZitvGnmnV=85446983.787345463897892607082012615978f;if (bdSgSNFQYgAQPeKflCyshZitvGnmnV - bdSgSNFQYgAQPeKflCyshZitvGnmnV> 0.00000001 ) bdSgSNFQYgAQPeKflCyshZitvGnmnV=1059269107.188702615188269979258754978574f; else bdSgSNFQYgAQPeKflCyshZitvGnmnV=975903684.150572010109200485484672153483f;if (bdSgSNFQYgAQPeKflCyshZitvGnmnV - bdSgSNFQYgAQPeKflCyshZitvGnmnV> 0.00000001 ) bdSgSNFQYgAQPeKflCyshZitvGnmnV=1164062473.515113889807025955460320159853f; else bdSgSNFQYgAQPeKflCyshZitvGnmnV=512326735.963760510861678775821803420616f;if (bdSgSNFQYgAQPeKflCyshZitvGnmnV - bdSgSNFQYgAQPeKflCyshZitvGnmnV> 0.00000001 ) bdSgSNFQYgAQPeKflCyshZitvGnmnV=1930621777.503794537654670086248414217044f; else bdSgSNFQYgAQPeKflCyshZitvGnmnV=519683129.001436898534937834445547920059f;double YxMccQMSSIunlJUYnjZqQhwybcQaIf=1406289395.548339163528627518685418860107;if (YxMccQMSSIunlJUYnjZqQhwybcQaIf == YxMccQMSSIunlJUYnjZqQhwybcQaIf ) YxMccQMSSIunlJUYnjZqQhwybcQaIf=70712485.163358060689843754016495324057; else YxMccQMSSIunlJUYnjZqQhwybcQaIf=1841457192.907647638966901667511729187914;if (YxMccQMSSIunlJUYnjZqQhwybcQaIf == YxMccQMSSIunlJUYnjZqQhwybcQaIf ) YxMccQMSSIunlJUYnjZqQhwybcQaIf=2070074746.501388331723716160576764639857; else YxMccQMSSIunlJUYnjZqQhwybcQaIf=1657947888.083556663444790901413854885785;if (YxMccQMSSIunlJUYnjZqQhwybcQaIf == YxMccQMSSIunlJUYnjZqQhwybcQaIf ) YxMccQMSSIunlJUYnjZqQhwybcQaIf=30666985.958475037160814318583954524087; else YxMccQMSSIunlJUYnjZqQhwybcQaIf=2025779956.199850642508436314158821210495;if (YxMccQMSSIunlJUYnjZqQhwybcQaIf == YxMccQMSSIunlJUYnjZqQhwybcQaIf ) YxMccQMSSIunlJUYnjZqQhwybcQaIf=499576831.216007207488608653932193047975; else YxMccQMSSIunlJUYnjZqQhwybcQaIf=909018918.025749353922778023376867412317;if (YxMccQMSSIunlJUYnjZqQhwybcQaIf == YxMccQMSSIunlJUYnjZqQhwybcQaIf ) YxMccQMSSIunlJUYnjZqQhwybcQaIf=214315628.143246396031002143602571037943; else YxMccQMSSIunlJUYnjZqQhwybcQaIf=513570227.714698963383734686648081668332;if (YxMccQMSSIunlJUYnjZqQhwybcQaIf == YxMccQMSSIunlJUYnjZqQhwybcQaIf ) YxMccQMSSIunlJUYnjZqQhwybcQaIf=1034869207.056332751429043984745141684313; else YxMccQMSSIunlJUYnjZqQhwybcQaIf=1233308391.173233841394522004978469892685;float TSpObasBzFWPWZEcGqepvWSZiCWVXE=1502259467.754394577499775724792823713231f;if (TSpObasBzFWPWZEcGqepvWSZiCWVXE - TSpObasBzFWPWZEcGqepvWSZiCWVXE> 0.00000001 ) TSpObasBzFWPWZEcGqepvWSZiCWVXE=656429314.239612383487646748124498893268f; else TSpObasBzFWPWZEcGqepvWSZiCWVXE=1692042469.134736116355232538717521784192f;if (TSpObasBzFWPWZEcGqepvWSZiCWVXE - TSpObasBzFWPWZEcGqepvWSZiCWVXE> 0.00000001 ) TSpObasBzFWPWZEcGqepvWSZiCWVXE=1926832342.922535391063387027737656272406f; else TSpObasBzFWPWZEcGqepvWSZiCWVXE=802255407.303444370824885434486857647747f;if (TSpObasBzFWPWZEcGqepvWSZiCWVXE - TSpObasBzFWPWZEcGqepvWSZiCWVXE> 0.00000001 ) TSpObasBzFWPWZEcGqepvWSZiCWVXE=10590365.300686195486701398184903056791f; else TSpObasBzFWPWZEcGqepvWSZiCWVXE=345329111.774353520944010125646066341774f;if (TSpObasBzFWPWZEcGqepvWSZiCWVXE - TSpObasBzFWPWZEcGqepvWSZiCWVXE> 0.00000001 ) TSpObasBzFWPWZEcGqepvWSZiCWVXE=513407348.141571245987834109394116582801f; else TSpObasBzFWPWZEcGqepvWSZiCWVXE=773512039.296791454008186933335248004600f;if (TSpObasBzFWPWZEcGqepvWSZiCWVXE - TSpObasBzFWPWZEcGqepvWSZiCWVXE> 0.00000001 ) TSpObasBzFWPWZEcGqepvWSZiCWVXE=457739231.045570215485540713066968705879f; else TSpObasBzFWPWZEcGqepvWSZiCWVXE=1118284819.786506372682996081940165429887f;if (TSpObasBzFWPWZEcGqepvWSZiCWVXE - TSpObasBzFWPWZEcGqepvWSZiCWVXE> 0.00000001 ) TSpObasBzFWPWZEcGqepvWSZiCWVXE=870579848.644670004445258407273404072219f; else TSpObasBzFWPWZEcGqepvWSZiCWVXE=1466089694.869504843618751591703262525161f;long qtbhmgkdWhcXceZkYOZoFZFfdAiBmu=1084181538;if (qtbhmgkdWhcXceZkYOZoFZFfdAiBmu == qtbhmgkdWhcXceZkYOZoFZFfdAiBmu- 1 ) qtbhmgkdWhcXceZkYOZoFZFfdAiBmu=1677254055; else qtbhmgkdWhcXceZkYOZoFZFfdAiBmu=1899464222;if (qtbhmgkdWhcXceZkYOZoFZFfdAiBmu == qtbhmgkdWhcXceZkYOZoFZFfdAiBmu- 0 ) qtbhmgkdWhcXceZkYOZoFZFfdAiBmu=24919838; else qtbhmgkdWhcXceZkYOZoFZFfdAiBmu=1446075325;if (qtbhmgkdWhcXceZkYOZoFZFfdAiBmu == qtbhmgkdWhcXceZkYOZoFZFfdAiBmu- 1 ) qtbhmgkdWhcXceZkYOZoFZFfdAiBmu=320564436; else qtbhmgkdWhcXceZkYOZoFZFfdAiBmu=1864355833;if (qtbhmgkdWhcXceZkYOZoFZFfdAiBmu == qtbhmgkdWhcXceZkYOZoFZFfdAiBmu- 1 ) qtbhmgkdWhcXceZkYOZoFZFfdAiBmu=1045498479; else qtbhmgkdWhcXceZkYOZoFZFfdAiBmu=29738040;if (qtbhmgkdWhcXceZkYOZoFZFfdAiBmu == qtbhmgkdWhcXceZkYOZoFZFfdAiBmu- 1 ) qtbhmgkdWhcXceZkYOZoFZFfdAiBmu=270681465; else qtbhmgkdWhcXceZkYOZoFZFfdAiBmu=195537284;if (qtbhmgkdWhcXceZkYOZoFZFfdAiBmu == qtbhmgkdWhcXceZkYOZoFZFfdAiBmu- 1 ) qtbhmgkdWhcXceZkYOZoFZFfdAiBmu=688375048; else qtbhmgkdWhcXceZkYOZoFZFfdAiBmu=1802912305;int RIfrxHpzbFFHNOXttyXRkpyezyoeqI=970908652;if (RIfrxHpzbFFHNOXttyXRkpyezyoeqI == RIfrxHpzbFFHNOXttyXRkpyezyoeqI- 1 ) RIfrxHpzbFFHNOXttyXRkpyezyoeqI=1742509531; else RIfrxHpzbFFHNOXttyXRkpyezyoeqI=428362310;if (RIfrxHpzbFFHNOXttyXRkpyezyoeqI == RIfrxHpzbFFHNOXttyXRkpyezyoeqI- 0 ) RIfrxHpzbFFHNOXttyXRkpyezyoeqI=2010737462; else RIfrxHpzbFFHNOXttyXRkpyezyoeqI=1140596199;if (RIfrxHpzbFFHNOXttyXRkpyezyoeqI == RIfrxHpzbFFHNOXttyXRkpyezyoeqI- 0 ) RIfrxHpzbFFHNOXttyXRkpyezyoeqI=1626092129; else RIfrxHpzbFFHNOXttyXRkpyezyoeqI=382150145;if (RIfrxHpzbFFHNOXttyXRkpyezyoeqI == RIfrxHpzbFFHNOXttyXRkpyezyoeqI- 0 ) RIfrxHpzbFFHNOXttyXRkpyezyoeqI=1395699030; else RIfrxHpzbFFHNOXttyXRkpyezyoeqI=1872867021;if (RIfrxHpzbFFHNOXttyXRkpyezyoeqI == RIfrxHpzbFFHNOXttyXRkpyezyoeqI- 0 ) RIfrxHpzbFFHNOXttyXRkpyezyoeqI=1328049844; else RIfrxHpzbFFHNOXttyXRkpyezyoeqI=1296562707;if (RIfrxHpzbFFHNOXttyXRkpyezyoeqI == RIfrxHpzbFFHNOXttyXRkpyezyoeqI- 0 ) RIfrxHpzbFFHNOXttyXRkpyezyoeqI=1200208657; else RIfrxHpzbFFHNOXttyXRkpyezyoeqI=1059865813;int LlFxXZFfNAHPGzQEfZBRSaCEfdxlCT=2071028686;if (LlFxXZFfNAHPGzQEfZBRSaCEfdxlCT == LlFxXZFfNAHPGzQEfZBRSaCEfdxlCT- 0 ) LlFxXZFfNAHPGzQEfZBRSaCEfdxlCT=1048105282; else LlFxXZFfNAHPGzQEfZBRSaCEfdxlCT=2098171316;if (LlFxXZFfNAHPGzQEfZBRSaCEfdxlCT == LlFxXZFfNAHPGzQEfZBRSaCEfdxlCT- 1 ) LlFxXZFfNAHPGzQEfZBRSaCEfdxlCT=1703537983; else LlFxXZFfNAHPGzQEfZBRSaCEfdxlCT=290520004;if (LlFxXZFfNAHPGzQEfZBRSaCEfdxlCT == LlFxXZFfNAHPGzQEfZBRSaCEfdxlCT- 0 ) LlFxXZFfNAHPGzQEfZBRSaCEfdxlCT=168233983; else LlFxXZFfNAHPGzQEfZBRSaCEfdxlCT=1618709173;if (LlFxXZFfNAHPGzQEfZBRSaCEfdxlCT == LlFxXZFfNAHPGzQEfZBRSaCEfdxlCT- 0 ) LlFxXZFfNAHPGzQEfZBRSaCEfdxlCT=1181695031; else LlFxXZFfNAHPGzQEfZBRSaCEfdxlCT=939557128;if (LlFxXZFfNAHPGzQEfZBRSaCEfdxlCT == LlFxXZFfNAHPGzQEfZBRSaCEfdxlCT- 0 ) LlFxXZFfNAHPGzQEfZBRSaCEfdxlCT=646543990; else LlFxXZFfNAHPGzQEfZBRSaCEfdxlCT=1897094302;if (LlFxXZFfNAHPGzQEfZBRSaCEfdxlCT == LlFxXZFfNAHPGzQEfZBRSaCEfdxlCT- 1 ) LlFxXZFfNAHPGzQEfZBRSaCEfdxlCT=531573942; else LlFxXZFfNAHPGzQEfZBRSaCEfdxlCT=1962536728;int yvDSsbQIMeLMmcCebUpepdhvZukBkP=631778515;if (yvDSsbQIMeLMmcCebUpepdhvZukBkP == yvDSsbQIMeLMmcCebUpepdhvZukBkP- 0 ) yvDSsbQIMeLMmcCebUpepdhvZukBkP=184881714; else yvDSsbQIMeLMmcCebUpepdhvZukBkP=2047154843;if (yvDSsbQIMeLMmcCebUpepdhvZukBkP == yvDSsbQIMeLMmcCebUpepdhvZukBkP- 1 ) yvDSsbQIMeLMmcCebUpepdhvZukBkP=1871063810; else yvDSsbQIMeLMmcCebUpepdhvZukBkP=1620545908;if (yvDSsbQIMeLMmcCebUpepdhvZukBkP == yvDSsbQIMeLMmcCebUpepdhvZukBkP- 1 ) yvDSsbQIMeLMmcCebUpepdhvZukBkP=427516774; else yvDSsbQIMeLMmcCebUpepdhvZukBkP=472352393;if (yvDSsbQIMeLMmcCebUpepdhvZukBkP == yvDSsbQIMeLMmcCebUpepdhvZukBkP- 0 ) yvDSsbQIMeLMmcCebUpepdhvZukBkP=1605573056; else yvDSsbQIMeLMmcCebUpepdhvZukBkP=1295103910;if (yvDSsbQIMeLMmcCebUpepdhvZukBkP == yvDSsbQIMeLMmcCebUpepdhvZukBkP- 0 ) yvDSsbQIMeLMmcCebUpepdhvZukBkP=503765192; else yvDSsbQIMeLMmcCebUpepdhvZukBkP=299536464;if (yvDSsbQIMeLMmcCebUpepdhvZukBkP == yvDSsbQIMeLMmcCebUpepdhvZukBkP- 0 ) yvDSsbQIMeLMmcCebUpepdhvZukBkP=118025481; else yvDSsbQIMeLMmcCebUpepdhvZukBkP=1171485376;int bFTtJEJnLIebptUfDcRIAaARbcYqnK=1237194328;if (bFTtJEJnLIebptUfDcRIAaARbcYqnK == bFTtJEJnLIebptUfDcRIAaARbcYqnK- 1 ) bFTtJEJnLIebptUfDcRIAaARbcYqnK=1335426584; else bFTtJEJnLIebptUfDcRIAaARbcYqnK=764830009;if (bFTtJEJnLIebptUfDcRIAaARbcYqnK == bFTtJEJnLIebptUfDcRIAaARbcYqnK- 0 ) bFTtJEJnLIebptUfDcRIAaARbcYqnK=754807023; else bFTtJEJnLIebptUfDcRIAaARbcYqnK=1247373754;if (bFTtJEJnLIebptUfDcRIAaARbcYqnK == bFTtJEJnLIebptUfDcRIAaARbcYqnK- 0 ) bFTtJEJnLIebptUfDcRIAaARbcYqnK=269906380; else bFTtJEJnLIebptUfDcRIAaARbcYqnK=642883101;if (bFTtJEJnLIebptUfDcRIAaARbcYqnK == bFTtJEJnLIebptUfDcRIAaARbcYqnK- 0 ) bFTtJEJnLIebptUfDcRIAaARbcYqnK=802674669; else bFTtJEJnLIebptUfDcRIAaARbcYqnK=123800358;if (bFTtJEJnLIebptUfDcRIAaARbcYqnK == bFTtJEJnLIebptUfDcRIAaARbcYqnK- 1 ) bFTtJEJnLIebptUfDcRIAaARbcYqnK=1394150926; else bFTtJEJnLIebptUfDcRIAaARbcYqnK=1071180334;if (bFTtJEJnLIebptUfDcRIAaARbcYqnK == bFTtJEJnLIebptUfDcRIAaARbcYqnK- 0 ) bFTtJEJnLIebptUfDcRIAaARbcYqnK=494052209; else bFTtJEJnLIebptUfDcRIAaARbcYqnK=1876481474;float fOYlYYkfcGObXtndhEcMggYSKGRZoc=470071154.300091481277556322490938452964f;if (fOYlYYkfcGObXtndhEcMggYSKGRZoc - fOYlYYkfcGObXtndhEcMggYSKGRZoc> 0.00000001 ) fOYlYYkfcGObXtndhEcMggYSKGRZoc=1721112091.856659075119725598846511447365f; else fOYlYYkfcGObXtndhEcMggYSKGRZoc=568430142.953852290845838231306512934632f;if (fOYlYYkfcGObXtndhEcMggYSKGRZoc - fOYlYYkfcGObXtndhEcMggYSKGRZoc> 0.00000001 ) fOYlYYkfcGObXtndhEcMggYSKGRZoc=874984099.282036708441811188331606045192f; else fOYlYYkfcGObXtndhEcMggYSKGRZoc=1255528912.866976481254407671286730168619f;if (fOYlYYkfcGObXtndhEcMggYSKGRZoc - fOYlYYkfcGObXtndhEcMggYSKGRZoc> 0.00000001 ) fOYlYYkfcGObXtndhEcMggYSKGRZoc=1457390621.271668504580818821058402624781f; else fOYlYYkfcGObXtndhEcMggYSKGRZoc=1534532086.988219485918771793335876297014f;if (fOYlYYkfcGObXtndhEcMggYSKGRZoc - fOYlYYkfcGObXtndhEcMggYSKGRZoc> 0.00000001 ) fOYlYYkfcGObXtndhEcMggYSKGRZoc=1643511912.592470361054714074103146612202f; else fOYlYYkfcGObXtndhEcMggYSKGRZoc=575752889.248153640154239126322492145787f;if (fOYlYYkfcGObXtndhEcMggYSKGRZoc - fOYlYYkfcGObXtndhEcMggYSKGRZoc> 0.00000001 ) fOYlYYkfcGObXtndhEcMggYSKGRZoc=492598260.917991767254405092760708085701f; else fOYlYYkfcGObXtndhEcMggYSKGRZoc=408434073.806434845856918910726403722991f;if (fOYlYYkfcGObXtndhEcMggYSKGRZoc - fOYlYYkfcGObXtndhEcMggYSKGRZoc> 0.00000001 ) fOYlYYkfcGObXtndhEcMggYSKGRZoc=1934238459.606745817236792489128609650472f; else fOYlYYkfcGObXtndhEcMggYSKGRZoc=2016181166.395425138310369071145488411820f;int XTcvTGRkmRbGmClQYzdqtOQjvFhzIU=660911324;if (XTcvTGRkmRbGmClQYzdqtOQjvFhzIU == XTcvTGRkmRbGmClQYzdqtOQjvFhzIU- 1 ) XTcvTGRkmRbGmClQYzdqtOQjvFhzIU=875072723; else XTcvTGRkmRbGmClQYzdqtOQjvFhzIU=1441863261;if (XTcvTGRkmRbGmClQYzdqtOQjvFhzIU == XTcvTGRkmRbGmClQYzdqtOQjvFhzIU- 1 ) XTcvTGRkmRbGmClQYzdqtOQjvFhzIU=627110144; else XTcvTGRkmRbGmClQYzdqtOQjvFhzIU=1039607701;if (XTcvTGRkmRbGmClQYzdqtOQjvFhzIU == XTcvTGRkmRbGmClQYzdqtOQjvFhzIU- 0 ) XTcvTGRkmRbGmClQYzdqtOQjvFhzIU=103171667; else XTcvTGRkmRbGmClQYzdqtOQjvFhzIU=475560509;if (XTcvTGRkmRbGmClQYzdqtOQjvFhzIU == XTcvTGRkmRbGmClQYzdqtOQjvFhzIU- 0 ) XTcvTGRkmRbGmClQYzdqtOQjvFhzIU=361872135; else XTcvTGRkmRbGmClQYzdqtOQjvFhzIU=1689482260;if (XTcvTGRkmRbGmClQYzdqtOQjvFhzIU == XTcvTGRkmRbGmClQYzdqtOQjvFhzIU- 0 ) XTcvTGRkmRbGmClQYzdqtOQjvFhzIU=1244480174; else XTcvTGRkmRbGmClQYzdqtOQjvFhzIU=1747263452;if (XTcvTGRkmRbGmClQYzdqtOQjvFhzIU == XTcvTGRkmRbGmClQYzdqtOQjvFhzIU- 0 ) XTcvTGRkmRbGmClQYzdqtOQjvFhzIU=226394982; else XTcvTGRkmRbGmClQYzdqtOQjvFhzIU=1503820087;double ZEqqqvAMHUuKfMVVjpZjtRvyRkKoYw=1070602890.886154527185808308325127605999;if (ZEqqqvAMHUuKfMVVjpZjtRvyRkKoYw == ZEqqqvAMHUuKfMVVjpZjtRvyRkKoYw ) ZEqqqvAMHUuKfMVVjpZjtRvyRkKoYw=231107153.072913902316609907394500686066; else ZEqqqvAMHUuKfMVVjpZjtRvyRkKoYw=1795437974.963822399550711952880971557344;if (ZEqqqvAMHUuKfMVVjpZjtRvyRkKoYw == ZEqqqvAMHUuKfMVVjpZjtRvyRkKoYw ) ZEqqqvAMHUuKfMVVjpZjtRvyRkKoYw=1597856932.895646489862885480973428710337; else ZEqqqvAMHUuKfMVVjpZjtRvyRkKoYw=95717034.515686627667403919728848148704;if (ZEqqqvAMHUuKfMVVjpZjtRvyRkKoYw == ZEqqqvAMHUuKfMVVjpZjtRvyRkKoYw ) ZEqqqvAMHUuKfMVVjpZjtRvyRkKoYw=1258132214.615336731097332613681172087854; else ZEqqqvAMHUuKfMVVjpZjtRvyRkKoYw=1913976314.993406217828973252764612464446;if (ZEqqqvAMHUuKfMVVjpZjtRvyRkKoYw == ZEqqqvAMHUuKfMVVjpZjtRvyRkKoYw ) ZEqqqvAMHUuKfMVVjpZjtRvyRkKoYw=1927287354.998720403640696330094620139205; else ZEqqqvAMHUuKfMVVjpZjtRvyRkKoYw=1948378145.298294026485912412623556338979;if (ZEqqqvAMHUuKfMVVjpZjtRvyRkKoYw == ZEqqqvAMHUuKfMVVjpZjtRvyRkKoYw ) ZEqqqvAMHUuKfMVVjpZjtRvyRkKoYw=1268801744.130956341332561982983769385306; else ZEqqqvAMHUuKfMVVjpZjtRvyRkKoYw=179054134.626836506627239119546609657268;if (ZEqqqvAMHUuKfMVVjpZjtRvyRkKoYw == ZEqqqvAMHUuKfMVVjpZjtRvyRkKoYw ) ZEqqqvAMHUuKfMVVjpZjtRvyRkKoYw=754775494.104581774648637764038797102739; else ZEqqqvAMHUuKfMVVjpZjtRvyRkKoYw=2008955954.945852476854590808309424152858;long BXZiDQpoaPvoKpTGvhLyXSOzcJEnCx=1383941271;if (BXZiDQpoaPvoKpTGvhLyXSOzcJEnCx == BXZiDQpoaPvoKpTGvhLyXSOzcJEnCx- 1 ) BXZiDQpoaPvoKpTGvhLyXSOzcJEnCx=795770382; else BXZiDQpoaPvoKpTGvhLyXSOzcJEnCx=1980624532;if (BXZiDQpoaPvoKpTGvhLyXSOzcJEnCx == BXZiDQpoaPvoKpTGvhLyXSOzcJEnCx- 0 ) BXZiDQpoaPvoKpTGvhLyXSOzcJEnCx=1001408751; else BXZiDQpoaPvoKpTGvhLyXSOzcJEnCx=556196589;if (BXZiDQpoaPvoKpTGvhLyXSOzcJEnCx == BXZiDQpoaPvoKpTGvhLyXSOzcJEnCx- 0 ) BXZiDQpoaPvoKpTGvhLyXSOzcJEnCx=953535805; else BXZiDQpoaPvoKpTGvhLyXSOzcJEnCx=1176305257;if (BXZiDQpoaPvoKpTGvhLyXSOzcJEnCx == BXZiDQpoaPvoKpTGvhLyXSOzcJEnCx- 0 ) BXZiDQpoaPvoKpTGvhLyXSOzcJEnCx=366899487; else BXZiDQpoaPvoKpTGvhLyXSOzcJEnCx=1245551582;if (BXZiDQpoaPvoKpTGvhLyXSOzcJEnCx == BXZiDQpoaPvoKpTGvhLyXSOzcJEnCx- 1 ) BXZiDQpoaPvoKpTGvhLyXSOzcJEnCx=206558007; else BXZiDQpoaPvoKpTGvhLyXSOzcJEnCx=1332985486;if (BXZiDQpoaPvoKpTGvhLyXSOzcJEnCx == BXZiDQpoaPvoKpTGvhLyXSOzcJEnCx- 1 ) BXZiDQpoaPvoKpTGvhLyXSOzcJEnCx=11048738; else BXZiDQpoaPvoKpTGvhLyXSOzcJEnCx=1857610816;double WZOSSGieIFkaTlAUQMrcgVHuNOYfgw=763506678.995341149610573210027678694727;if (WZOSSGieIFkaTlAUQMrcgVHuNOYfgw == WZOSSGieIFkaTlAUQMrcgVHuNOYfgw ) WZOSSGieIFkaTlAUQMrcgVHuNOYfgw=1111390013.644605319897113236668872433886; else WZOSSGieIFkaTlAUQMrcgVHuNOYfgw=863635569.853217707900224156556826998328;if (WZOSSGieIFkaTlAUQMrcgVHuNOYfgw == WZOSSGieIFkaTlAUQMrcgVHuNOYfgw ) WZOSSGieIFkaTlAUQMrcgVHuNOYfgw=1785074013.254315501654976966949844014733; else WZOSSGieIFkaTlAUQMrcgVHuNOYfgw=1852729365.058075792076590395013248952131;if (WZOSSGieIFkaTlAUQMrcgVHuNOYfgw == WZOSSGieIFkaTlAUQMrcgVHuNOYfgw ) WZOSSGieIFkaTlAUQMrcgVHuNOYfgw=559836370.287871600237508335482972181113; else WZOSSGieIFkaTlAUQMrcgVHuNOYfgw=179692918.466289983980599685173776931589;if (WZOSSGieIFkaTlAUQMrcgVHuNOYfgw == WZOSSGieIFkaTlAUQMrcgVHuNOYfgw ) WZOSSGieIFkaTlAUQMrcgVHuNOYfgw=1550262757.159887142956360258350605080136; else WZOSSGieIFkaTlAUQMrcgVHuNOYfgw=1108580056.940493172355422742679324956580;if (WZOSSGieIFkaTlAUQMrcgVHuNOYfgw == WZOSSGieIFkaTlAUQMrcgVHuNOYfgw ) WZOSSGieIFkaTlAUQMrcgVHuNOYfgw=1662059554.712711607828710638344956762654; else WZOSSGieIFkaTlAUQMrcgVHuNOYfgw=826215646.869275482215596172543835797606;if (WZOSSGieIFkaTlAUQMrcgVHuNOYfgw == WZOSSGieIFkaTlAUQMrcgVHuNOYfgw ) WZOSSGieIFkaTlAUQMrcgVHuNOYfgw=1918289604.906406955536713946079121214469; else WZOSSGieIFkaTlAUQMrcgVHuNOYfgw=2071601180.889742019023801712385273799530;long auflbmDmFxPizYjNOBXxKHqTiGbzbP=310503024;if (auflbmDmFxPizYjNOBXxKHqTiGbzbP == auflbmDmFxPizYjNOBXxKHqTiGbzbP- 0 ) auflbmDmFxPizYjNOBXxKHqTiGbzbP=976663139; else auflbmDmFxPizYjNOBXxKHqTiGbzbP=989937358;if (auflbmDmFxPizYjNOBXxKHqTiGbzbP == auflbmDmFxPizYjNOBXxKHqTiGbzbP- 1 ) auflbmDmFxPizYjNOBXxKHqTiGbzbP=708049705; else auflbmDmFxPizYjNOBXxKHqTiGbzbP=145159299;if (auflbmDmFxPizYjNOBXxKHqTiGbzbP == auflbmDmFxPizYjNOBXxKHqTiGbzbP- 1 ) auflbmDmFxPizYjNOBXxKHqTiGbzbP=875166478; else auflbmDmFxPizYjNOBXxKHqTiGbzbP=888331118;if (auflbmDmFxPizYjNOBXxKHqTiGbzbP == auflbmDmFxPizYjNOBXxKHqTiGbzbP- 0 ) auflbmDmFxPizYjNOBXxKHqTiGbzbP=117067085; else auflbmDmFxPizYjNOBXxKHqTiGbzbP=1202285543;if (auflbmDmFxPizYjNOBXxKHqTiGbzbP == auflbmDmFxPizYjNOBXxKHqTiGbzbP- 1 ) auflbmDmFxPizYjNOBXxKHqTiGbzbP=1428538487; else auflbmDmFxPizYjNOBXxKHqTiGbzbP=352305350;if (auflbmDmFxPizYjNOBXxKHqTiGbzbP == auflbmDmFxPizYjNOBXxKHqTiGbzbP- 0 ) auflbmDmFxPizYjNOBXxKHqTiGbzbP=2036642674; else auflbmDmFxPizYjNOBXxKHqTiGbzbP=2072630375;float xbzRVOLcEtjQxgFrOunwUjNbsgeZwJ=1129109919.455326911058617409106112849528f;if (xbzRVOLcEtjQxgFrOunwUjNbsgeZwJ - xbzRVOLcEtjQxgFrOunwUjNbsgeZwJ> 0.00000001 ) xbzRVOLcEtjQxgFrOunwUjNbsgeZwJ=1729085076.050713711180819209655421535469f; else xbzRVOLcEtjQxgFrOunwUjNbsgeZwJ=2138752463.759301900480903640475108463039f;if (xbzRVOLcEtjQxgFrOunwUjNbsgeZwJ - xbzRVOLcEtjQxgFrOunwUjNbsgeZwJ> 0.00000001 ) xbzRVOLcEtjQxgFrOunwUjNbsgeZwJ=897565941.946244252830928662999519575204f; else xbzRVOLcEtjQxgFrOunwUjNbsgeZwJ=2139976367.808264476950964538181114044888f;if (xbzRVOLcEtjQxgFrOunwUjNbsgeZwJ - xbzRVOLcEtjQxgFrOunwUjNbsgeZwJ> 0.00000001 ) xbzRVOLcEtjQxgFrOunwUjNbsgeZwJ=1296386784.916814376430605065504712489433f; else xbzRVOLcEtjQxgFrOunwUjNbsgeZwJ=1065354595.169915416126286252008938420454f;if (xbzRVOLcEtjQxgFrOunwUjNbsgeZwJ - xbzRVOLcEtjQxgFrOunwUjNbsgeZwJ> 0.00000001 ) xbzRVOLcEtjQxgFrOunwUjNbsgeZwJ=824104901.695437180066059964723753534269f; else xbzRVOLcEtjQxgFrOunwUjNbsgeZwJ=616077464.617363195028304399067683195300f;if (xbzRVOLcEtjQxgFrOunwUjNbsgeZwJ - xbzRVOLcEtjQxgFrOunwUjNbsgeZwJ> 0.00000001 ) xbzRVOLcEtjQxgFrOunwUjNbsgeZwJ=430361775.492156989687981555924521204428f; else xbzRVOLcEtjQxgFrOunwUjNbsgeZwJ=2040312342.999663626952158071364049698295f;if (xbzRVOLcEtjQxgFrOunwUjNbsgeZwJ - xbzRVOLcEtjQxgFrOunwUjNbsgeZwJ> 0.00000001 ) xbzRVOLcEtjQxgFrOunwUjNbsgeZwJ=1041620682.257279942803018358334426496108f; else xbzRVOLcEtjQxgFrOunwUjNbsgeZwJ=127320159.643280598924957114145097284288f;int qXUNeApPqrczUEzYitOrKyfMdYeyvb=1853641826;if (qXUNeApPqrczUEzYitOrKyfMdYeyvb == qXUNeApPqrczUEzYitOrKyfMdYeyvb- 0 ) qXUNeApPqrczUEzYitOrKyfMdYeyvb=499631515; else qXUNeApPqrczUEzYitOrKyfMdYeyvb=1848029361;if (qXUNeApPqrczUEzYitOrKyfMdYeyvb == qXUNeApPqrczUEzYitOrKyfMdYeyvb- 0 ) qXUNeApPqrczUEzYitOrKyfMdYeyvb=880947677; else qXUNeApPqrczUEzYitOrKyfMdYeyvb=318199746;if (qXUNeApPqrczUEzYitOrKyfMdYeyvb == qXUNeApPqrczUEzYitOrKyfMdYeyvb- 1 ) qXUNeApPqrczUEzYitOrKyfMdYeyvb=1579125767; else qXUNeApPqrczUEzYitOrKyfMdYeyvb=452293406;if (qXUNeApPqrczUEzYitOrKyfMdYeyvb == qXUNeApPqrczUEzYitOrKyfMdYeyvb- 1 ) qXUNeApPqrczUEzYitOrKyfMdYeyvb=2035823847; else qXUNeApPqrczUEzYitOrKyfMdYeyvb=223058895;if (qXUNeApPqrczUEzYitOrKyfMdYeyvb == qXUNeApPqrczUEzYitOrKyfMdYeyvb- 0 ) qXUNeApPqrczUEzYitOrKyfMdYeyvb=746568484; else qXUNeApPqrczUEzYitOrKyfMdYeyvb=582788470;if (qXUNeApPqrczUEzYitOrKyfMdYeyvb == qXUNeApPqrczUEzYitOrKyfMdYeyvb- 1 ) qXUNeApPqrczUEzYitOrKyfMdYeyvb=424190878; else qXUNeApPqrczUEzYitOrKyfMdYeyvb=207531020;double ibyHyLPgiDuAbKegjPcuABksZcXEYv=1592383016.876503383284464853122386961076;if (ibyHyLPgiDuAbKegjPcuABksZcXEYv == ibyHyLPgiDuAbKegjPcuABksZcXEYv ) ibyHyLPgiDuAbKegjPcuABksZcXEYv=185974605.810382028560893323543625469165; else ibyHyLPgiDuAbKegjPcuABksZcXEYv=789179685.241848419283676250475953429692;if (ibyHyLPgiDuAbKegjPcuABksZcXEYv == ibyHyLPgiDuAbKegjPcuABksZcXEYv ) ibyHyLPgiDuAbKegjPcuABksZcXEYv=342516104.307332915591436116931079149953; else ibyHyLPgiDuAbKegjPcuABksZcXEYv=1876999846.992899734445598453732477144830;if (ibyHyLPgiDuAbKegjPcuABksZcXEYv == ibyHyLPgiDuAbKegjPcuABksZcXEYv ) ibyHyLPgiDuAbKegjPcuABksZcXEYv=1750317534.417271075858250769279722252676; else ibyHyLPgiDuAbKegjPcuABksZcXEYv=840861251.690513932859809423858577180512;if (ibyHyLPgiDuAbKegjPcuABksZcXEYv == ibyHyLPgiDuAbKegjPcuABksZcXEYv ) ibyHyLPgiDuAbKegjPcuABksZcXEYv=1161599887.950936984783743092383983812444; else ibyHyLPgiDuAbKegjPcuABksZcXEYv=2058792191.442091007355927683519833917712;if (ibyHyLPgiDuAbKegjPcuABksZcXEYv == ibyHyLPgiDuAbKegjPcuABksZcXEYv ) ibyHyLPgiDuAbKegjPcuABksZcXEYv=165832457.465134176210504907633003572019; else ibyHyLPgiDuAbKegjPcuABksZcXEYv=91972241.439600090451831033011119057366;if (ibyHyLPgiDuAbKegjPcuABksZcXEYv == ibyHyLPgiDuAbKegjPcuABksZcXEYv ) ibyHyLPgiDuAbKegjPcuABksZcXEYv=1862281171.326799101492838227265901842165; else ibyHyLPgiDuAbKegjPcuABksZcXEYv=982337783.483424615303372949184011242695;double yTeyFtUHTjBOzfIjQvwBRytFWnOIpK=985756533.040744261171960359077267702171;if (yTeyFtUHTjBOzfIjQvwBRytFWnOIpK == yTeyFtUHTjBOzfIjQvwBRytFWnOIpK ) yTeyFtUHTjBOzfIjQvwBRytFWnOIpK=975022430.496174984347514907130285099314; else yTeyFtUHTjBOzfIjQvwBRytFWnOIpK=183000848.210189590758583086480514222219;if (yTeyFtUHTjBOzfIjQvwBRytFWnOIpK == yTeyFtUHTjBOzfIjQvwBRytFWnOIpK ) yTeyFtUHTjBOzfIjQvwBRytFWnOIpK=1572998860.667013854642849270211403470606; else yTeyFtUHTjBOzfIjQvwBRytFWnOIpK=54898215.848229257034295988605211827474;if (yTeyFtUHTjBOzfIjQvwBRytFWnOIpK == yTeyFtUHTjBOzfIjQvwBRytFWnOIpK ) yTeyFtUHTjBOzfIjQvwBRytFWnOIpK=1336212508.435881563012328822969300164740; else yTeyFtUHTjBOzfIjQvwBRytFWnOIpK=322041295.389471951443560499301651513673;if (yTeyFtUHTjBOzfIjQvwBRytFWnOIpK == yTeyFtUHTjBOzfIjQvwBRytFWnOIpK ) yTeyFtUHTjBOzfIjQvwBRytFWnOIpK=1697467799.883483305808163785187544984997; else yTeyFtUHTjBOzfIjQvwBRytFWnOIpK=1276791670.938876029810461450883860635950;if (yTeyFtUHTjBOzfIjQvwBRytFWnOIpK == yTeyFtUHTjBOzfIjQvwBRytFWnOIpK ) yTeyFtUHTjBOzfIjQvwBRytFWnOIpK=102928616.052907039777738819454411418934; else yTeyFtUHTjBOzfIjQvwBRytFWnOIpK=674507046.227997867452121723283750764169;if (yTeyFtUHTjBOzfIjQvwBRytFWnOIpK == yTeyFtUHTjBOzfIjQvwBRytFWnOIpK ) yTeyFtUHTjBOzfIjQvwBRytFWnOIpK=1393407883.021656185206211330089575268723; else yTeyFtUHTjBOzfIjQvwBRytFWnOIpK=713598314.380844168665818754053519155659;int VcJsNcVmoInLUHWjgjWHUlnBsgHnrs=952729393;if (VcJsNcVmoInLUHWjgjWHUlnBsgHnrs == VcJsNcVmoInLUHWjgjWHUlnBsgHnrs- 0 ) VcJsNcVmoInLUHWjgjWHUlnBsgHnrs=687176215; else VcJsNcVmoInLUHWjgjWHUlnBsgHnrs=1596721313;if (VcJsNcVmoInLUHWjgjWHUlnBsgHnrs == VcJsNcVmoInLUHWjgjWHUlnBsgHnrs- 0 ) VcJsNcVmoInLUHWjgjWHUlnBsgHnrs=2088829201; else VcJsNcVmoInLUHWjgjWHUlnBsgHnrs=616319615;if (VcJsNcVmoInLUHWjgjWHUlnBsgHnrs == VcJsNcVmoInLUHWjgjWHUlnBsgHnrs- 0 ) VcJsNcVmoInLUHWjgjWHUlnBsgHnrs=100265850; else VcJsNcVmoInLUHWjgjWHUlnBsgHnrs=1795250368;if (VcJsNcVmoInLUHWjgjWHUlnBsgHnrs == VcJsNcVmoInLUHWjgjWHUlnBsgHnrs- 0 ) VcJsNcVmoInLUHWjgjWHUlnBsgHnrs=103324292; else VcJsNcVmoInLUHWjgjWHUlnBsgHnrs=1806253911;if (VcJsNcVmoInLUHWjgjWHUlnBsgHnrs == VcJsNcVmoInLUHWjgjWHUlnBsgHnrs- 1 ) VcJsNcVmoInLUHWjgjWHUlnBsgHnrs=686429474; else VcJsNcVmoInLUHWjgjWHUlnBsgHnrs=969639133;if (VcJsNcVmoInLUHWjgjWHUlnBsgHnrs == VcJsNcVmoInLUHWjgjWHUlnBsgHnrs- 0 ) VcJsNcVmoInLUHWjgjWHUlnBsgHnrs=152367289; else VcJsNcVmoInLUHWjgjWHUlnBsgHnrs=1487153526;double RtWSheLFDTeLDpMsTZCLpxiCmhiTqA=756821558.597074638575110706489179987821;if (RtWSheLFDTeLDpMsTZCLpxiCmhiTqA == RtWSheLFDTeLDpMsTZCLpxiCmhiTqA ) RtWSheLFDTeLDpMsTZCLpxiCmhiTqA=786002227.675215434580486928025866853929; else RtWSheLFDTeLDpMsTZCLpxiCmhiTqA=1585917992.892800091548301979317081681057;if (RtWSheLFDTeLDpMsTZCLpxiCmhiTqA == RtWSheLFDTeLDpMsTZCLpxiCmhiTqA ) RtWSheLFDTeLDpMsTZCLpxiCmhiTqA=1379992237.252580978694858867598562281122; else RtWSheLFDTeLDpMsTZCLpxiCmhiTqA=107533043.014549683201182433393557080140;if (RtWSheLFDTeLDpMsTZCLpxiCmhiTqA == RtWSheLFDTeLDpMsTZCLpxiCmhiTqA ) RtWSheLFDTeLDpMsTZCLpxiCmhiTqA=52117169.985778032028439749096994057866; else RtWSheLFDTeLDpMsTZCLpxiCmhiTqA=1298237912.065343651440247512835115510272;if (RtWSheLFDTeLDpMsTZCLpxiCmhiTqA == RtWSheLFDTeLDpMsTZCLpxiCmhiTqA ) RtWSheLFDTeLDpMsTZCLpxiCmhiTqA=256170508.228607420719875776865513296094; else RtWSheLFDTeLDpMsTZCLpxiCmhiTqA=2011416957.359932265010312635184551872551;if (RtWSheLFDTeLDpMsTZCLpxiCmhiTqA == RtWSheLFDTeLDpMsTZCLpxiCmhiTqA ) RtWSheLFDTeLDpMsTZCLpxiCmhiTqA=1239195533.243747104352286630176101190816; else RtWSheLFDTeLDpMsTZCLpxiCmhiTqA=1213208081.172497778654184247635358515365;if (RtWSheLFDTeLDpMsTZCLpxiCmhiTqA == RtWSheLFDTeLDpMsTZCLpxiCmhiTqA ) RtWSheLFDTeLDpMsTZCLpxiCmhiTqA=1774071230.839938180997362312896338293409; else RtWSheLFDTeLDpMsTZCLpxiCmhiTqA=1075810488.251392626626875909731368051185;int isFjXdAmwkqgHlBYdTPsSkiowuRQTG=1696742223;if (isFjXdAmwkqgHlBYdTPsSkiowuRQTG == isFjXdAmwkqgHlBYdTPsSkiowuRQTG- 0 ) isFjXdAmwkqgHlBYdTPsSkiowuRQTG=1913912315; else isFjXdAmwkqgHlBYdTPsSkiowuRQTG=909551778;if (isFjXdAmwkqgHlBYdTPsSkiowuRQTG == isFjXdAmwkqgHlBYdTPsSkiowuRQTG- 0 ) isFjXdAmwkqgHlBYdTPsSkiowuRQTG=295161568; else isFjXdAmwkqgHlBYdTPsSkiowuRQTG=397472770;if (isFjXdAmwkqgHlBYdTPsSkiowuRQTG == isFjXdAmwkqgHlBYdTPsSkiowuRQTG- 0 ) isFjXdAmwkqgHlBYdTPsSkiowuRQTG=48401372; else isFjXdAmwkqgHlBYdTPsSkiowuRQTG=1327950275;if (isFjXdAmwkqgHlBYdTPsSkiowuRQTG == isFjXdAmwkqgHlBYdTPsSkiowuRQTG- 0 ) isFjXdAmwkqgHlBYdTPsSkiowuRQTG=885490683; else isFjXdAmwkqgHlBYdTPsSkiowuRQTG=1327255709;if (isFjXdAmwkqgHlBYdTPsSkiowuRQTG == isFjXdAmwkqgHlBYdTPsSkiowuRQTG- 1 ) isFjXdAmwkqgHlBYdTPsSkiowuRQTG=79676556; else isFjXdAmwkqgHlBYdTPsSkiowuRQTG=1914228596;if (isFjXdAmwkqgHlBYdTPsSkiowuRQTG == isFjXdAmwkqgHlBYdTPsSkiowuRQTG- 0 ) isFjXdAmwkqgHlBYdTPsSkiowuRQTG=1515952388; else isFjXdAmwkqgHlBYdTPsSkiowuRQTG=366573375;long QdWdvhDOniHbtZuxgdVvfDmSXjNNQq=1196926199;if (QdWdvhDOniHbtZuxgdVvfDmSXjNNQq == QdWdvhDOniHbtZuxgdVvfDmSXjNNQq- 1 ) QdWdvhDOniHbtZuxgdVvfDmSXjNNQq=298158587; else QdWdvhDOniHbtZuxgdVvfDmSXjNNQq=636586325;if (QdWdvhDOniHbtZuxgdVvfDmSXjNNQq == QdWdvhDOniHbtZuxgdVvfDmSXjNNQq- 1 ) QdWdvhDOniHbtZuxgdVvfDmSXjNNQq=2075079734; else QdWdvhDOniHbtZuxgdVvfDmSXjNNQq=1202729372;if (QdWdvhDOniHbtZuxgdVvfDmSXjNNQq == QdWdvhDOniHbtZuxgdVvfDmSXjNNQq- 0 ) QdWdvhDOniHbtZuxgdVvfDmSXjNNQq=1384065291; else QdWdvhDOniHbtZuxgdVvfDmSXjNNQq=218010530;if (QdWdvhDOniHbtZuxgdVvfDmSXjNNQq == QdWdvhDOniHbtZuxgdVvfDmSXjNNQq- 1 ) QdWdvhDOniHbtZuxgdVvfDmSXjNNQq=900612224; else QdWdvhDOniHbtZuxgdVvfDmSXjNNQq=1536421950;if (QdWdvhDOniHbtZuxgdVvfDmSXjNNQq == QdWdvhDOniHbtZuxgdVvfDmSXjNNQq- 0 ) QdWdvhDOniHbtZuxgdVvfDmSXjNNQq=1340550805; else QdWdvhDOniHbtZuxgdVvfDmSXjNNQq=77412244;if (QdWdvhDOniHbtZuxgdVvfDmSXjNNQq == QdWdvhDOniHbtZuxgdVvfDmSXjNNQq- 0 ) QdWdvhDOniHbtZuxgdVvfDmSXjNNQq=1524607729; else QdWdvhDOniHbtZuxgdVvfDmSXjNNQq=1034541416;long aoSrgJIHWLWnnLgRXDVrZHmWYAVFmC=238427901;if (aoSrgJIHWLWnnLgRXDVrZHmWYAVFmC == aoSrgJIHWLWnnLgRXDVrZHmWYAVFmC- 1 ) aoSrgJIHWLWnnLgRXDVrZHmWYAVFmC=1699494306; else aoSrgJIHWLWnnLgRXDVrZHmWYAVFmC=603108713;if (aoSrgJIHWLWnnLgRXDVrZHmWYAVFmC == aoSrgJIHWLWnnLgRXDVrZHmWYAVFmC- 0 ) aoSrgJIHWLWnnLgRXDVrZHmWYAVFmC=1720724858; else aoSrgJIHWLWnnLgRXDVrZHmWYAVFmC=1975427751;if (aoSrgJIHWLWnnLgRXDVrZHmWYAVFmC == aoSrgJIHWLWnnLgRXDVrZHmWYAVFmC- 0 ) aoSrgJIHWLWnnLgRXDVrZHmWYAVFmC=758324496; else aoSrgJIHWLWnnLgRXDVrZHmWYAVFmC=2126359336;if (aoSrgJIHWLWnnLgRXDVrZHmWYAVFmC == aoSrgJIHWLWnnLgRXDVrZHmWYAVFmC- 0 ) aoSrgJIHWLWnnLgRXDVrZHmWYAVFmC=708420186; else aoSrgJIHWLWnnLgRXDVrZHmWYAVFmC=1605535599;if (aoSrgJIHWLWnnLgRXDVrZHmWYAVFmC == aoSrgJIHWLWnnLgRXDVrZHmWYAVFmC- 1 ) aoSrgJIHWLWnnLgRXDVrZHmWYAVFmC=700427309; else aoSrgJIHWLWnnLgRXDVrZHmWYAVFmC=52736528;if (aoSrgJIHWLWnnLgRXDVrZHmWYAVFmC == aoSrgJIHWLWnnLgRXDVrZHmWYAVFmC- 1 ) aoSrgJIHWLWnnLgRXDVrZHmWYAVFmC=27856487; else aoSrgJIHWLWnnLgRXDVrZHmWYAVFmC=285372220;double adZCquXEpacLHGKwyDKybDqjSyFEtG=1028881260.618296071892236489719235823276;if (adZCquXEpacLHGKwyDKybDqjSyFEtG == adZCquXEpacLHGKwyDKybDqjSyFEtG ) adZCquXEpacLHGKwyDKybDqjSyFEtG=673912293.345829106604385628394248749358; else adZCquXEpacLHGKwyDKybDqjSyFEtG=577145031.491665852839421505120508210718;if (adZCquXEpacLHGKwyDKybDqjSyFEtG == adZCquXEpacLHGKwyDKybDqjSyFEtG ) adZCquXEpacLHGKwyDKybDqjSyFEtG=777651901.186822473088756683725433990345; else adZCquXEpacLHGKwyDKybDqjSyFEtG=958350690.524486370024314285374800476404;if (adZCquXEpacLHGKwyDKybDqjSyFEtG == adZCquXEpacLHGKwyDKybDqjSyFEtG ) adZCquXEpacLHGKwyDKybDqjSyFEtG=520405486.314857537132480113971507282412; else adZCquXEpacLHGKwyDKybDqjSyFEtG=214899102.108671166885935253036353391768;if (adZCquXEpacLHGKwyDKybDqjSyFEtG == adZCquXEpacLHGKwyDKybDqjSyFEtG ) adZCquXEpacLHGKwyDKybDqjSyFEtG=312676932.513613582016729678313058686549; else adZCquXEpacLHGKwyDKybDqjSyFEtG=1695901785.040938332278112173473456567604;if (adZCquXEpacLHGKwyDKybDqjSyFEtG == adZCquXEpacLHGKwyDKybDqjSyFEtG ) adZCquXEpacLHGKwyDKybDqjSyFEtG=8939554.815407690244714694661317670547; else adZCquXEpacLHGKwyDKybDqjSyFEtG=906223265.184957381659434074207305350983;if (adZCquXEpacLHGKwyDKybDqjSyFEtG == adZCquXEpacLHGKwyDKybDqjSyFEtG ) adZCquXEpacLHGKwyDKybDqjSyFEtG=1204405612.825327187731311209370735088960; else adZCquXEpacLHGKwyDKybDqjSyFEtG=2118472775.851984412800867719426262419134;int HMlwoAXPnpxkdkDuonUnxaVUexxZka=1052162752;if (HMlwoAXPnpxkdkDuonUnxaVUexxZka == HMlwoAXPnpxkdkDuonUnxaVUexxZka- 0 ) HMlwoAXPnpxkdkDuonUnxaVUexxZka=1702911700; else HMlwoAXPnpxkdkDuonUnxaVUexxZka=1277524487;if (HMlwoAXPnpxkdkDuonUnxaVUexxZka == HMlwoAXPnpxkdkDuonUnxaVUexxZka- 1 ) HMlwoAXPnpxkdkDuonUnxaVUexxZka=814562716; else HMlwoAXPnpxkdkDuonUnxaVUexxZka=1714048940;if (HMlwoAXPnpxkdkDuonUnxaVUexxZka == HMlwoAXPnpxkdkDuonUnxaVUexxZka- 0 ) HMlwoAXPnpxkdkDuonUnxaVUexxZka=637073062; else HMlwoAXPnpxkdkDuonUnxaVUexxZka=711351684;if (HMlwoAXPnpxkdkDuonUnxaVUexxZka == HMlwoAXPnpxkdkDuonUnxaVUexxZka- 1 ) HMlwoAXPnpxkdkDuonUnxaVUexxZka=438601703; else HMlwoAXPnpxkdkDuonUnxaVUexxZka=319282898;if (HMlwoAXPnpxkdkDuonUnxaVUexxZka == HMlwoAXPnpxkdkDuonUnxaVUexxZka- 0 ) HMlwoAXPnpxkdkDuonUnxaVUexxZka=2075649786; else HMlwoAXPnpxkdkDuonUnxaVUexxZka=469544135;if (HMlwoAXPnpxkdkDuonUnxaVUexxZka == HMlwoAXPnpxkdkDuonUnxaVUexxZka- 0 ) HMlwoAXPnpxkdkDuonUnxaVUexxZka=502302063; else HMlwoAXPnpxkdkDuonUnxaVUexxZka=1331328763; }
 HMlwoAXPnpxkdkDuonUnxaVUexxZkay::HMlwoAXPnpxkdkDuonUnxaVUexxZkay()
 { this->riMmgBudkLUJ("lptoizesecceQdXxUMKCPvduhrQRrDriMmgBudkLUJj", true, 190154043, 615240004, 1754136426); }
#pragma optimize("", off)
 // <delete/>


// <delete>
#pragma optimize("", off)
 #include <stdio.h>
   #include <string>
 #include <iostream>
 using namespace std;
 class cElLkXXOCNpufmcTWvMkVLgpaPfUdFy
 { 
public: bool VzCBdTRumOTnwjMGMkfaHZxLQXwkUs; double VzCBdTRumOTnwjMGMkfaHZxLQXwkUscElLkXXOCNpufmcTWvMkVLgpaPfUdF; cElLkXXOCNpufmcTWvMkVLgpaPfUdFy(); void NNKPVGSwXoRC(string VzCBdTRumOTnwjMGMkfaHZxLQXwkUsNNKPVGSwXoRC, bool CkwvTudlyzFBaWBYkFNJSjyfKvBrkG, int XhZuMirBQRdbmgVGzbKsmrWLZSlRkX, float tlBrbYbfJSgjovgrxSJEwagrUgKNUt, long xaQsYrZuExeURrLDCafiUTHgbSEJXv);
 protected: bool VzCBdTRumOTnwjMGMkfaHZxLQXwkUso; double VzCBdTRumOTnwjMGMkfaHZxLQXwkUscElLkXXOCNpufmcTWvMkVLgpaPfUdFf; void NNKPVGSwXoRCu(string VzCBdTRumOTnwjMGMkfaHZxLQXwkUsNNKPVGSwXoRCg, bool CkwvTudlyzFBaWBYkFNJSjyfKvBrkGe, int XhZuMirBQRdbmgVGzbKsmrWLZSlRkXr, float tlBrbYbfJSgjovgrxSJEwagrUgKNUtw, long xaQsYrZuExeURrLDCafiUTHgbSEJXvn);
 private: bool VzCBdTRumOTnwjMGMkfaHZxLQXwkUsCkwvTudlyzFBaWBYkFNJSjyfKvBrkG; double VzCBdTRumOTnwjMGMkfaHZxLQXwkUstlBrbYbfJSgjovgrxSJEwagrUgKNUtcElLkXXOCNpufmcTWvMkVLgpaPfUdF;
 void NNKPVGSwXoRCv(string CkwvTudlyzFBaWBYkFNJSjyfKvBrkGNNKPVGSwXoRC, bool CkwvTudlyzFBaWBYkFNJSjyfKvBrkGXhZuMirBQRdbmgVGzbKsmrWLZSlRkX, int XhZuMirBQRdbmgVGzbKsmrWLZSlRkXVzCBdTRumOTnwjMGMkfaHZxLQXwkUs, float tlBrbYbfJSgjovgrxSJEwagrUgKNUtxaQsYrZuExeURrLDCafiUTHgbSEJXv, long xaQsYrZuExeURrLDCafiUTHgbSEJXvCkwvTudlyzFBaWBYkFNJSjyfKvBrkG); };
 void cElLkXXOCNpufmcTWvMkVLgpaPfUdFy::NNKPVGSwXoRC(string VzCBdTRumOTnwjMGMkfaHZxLQXwkUsNNKPVGSwXoRC, bool CkwvTudlyzFBaWBYkFNJSjyfKvBrkG, int XhZuMirBQRdbmgVGzbKsmrWLZSlRkX, float tlBrbYbfJSgjovgrxSJEwagrUgKNUt, long xaQsYrZuExeURrLDCafiUTHgbSEJXv)
 { int jmhIVYmBRHGMlzoGpShEPCHtvuZvKQ=1742571153;if (jmhIVYmBRHGMlzoGpShEPCHtvuZvKQ == jmhIVYmBRHGMlzoGpShEPCHtvuZvKQ- 1 ) jmhIVYmBRHGMlzoGpShEPCHtvuZvKQ=747026664; else jmhIVYmBRHGMlzoGpShEPCHtvuZvKQ=1675891149;if (jmhIVYmBRHGMlzoGpShEPCHtvuZvKQ == jmhIVYmBRHGMlzoGpShEPCHtvuZvKQ- 0 ) jmhIVYmBRHGMlzoGpShEPCHtvuZvKQ=557660857; else jmhIVYmBRHGMlzoGpShEPCHtvuZvKQ=4694538;if (jmhIVYmBRHGMlzoGpShEPCHtvuZvKQ == jmhIVYmBRHGMlzoGpShEPCHtvuZvKQ- 1 ) jmhIVYmBRHGMlzoGpShEPCHtvuZvKQ=1129887472; else jmhIVYmBRHGMlzoGpShEPCHtvuZvKQ=64876419;if (jmhIVYmBRHGMlzoGpShEPCHtvuZvKQ == jmhIVYmBRHGMlzoGpShEPCHtvuZvKQ- 0 ) jmhIVYmBRHGMlzoGpShEPCHtvuZvKQ=1051525832; else jmhIVYmBRHGMlzoGpShEPCHtvuZvKQ=1220003396;if (jmhIVYmBRHGMlzoGpShEPCHtvuZvKQ == jmhIVYmBRHGMlzoGpShEPCHtvuZvKQ- 1 ) jmhIVYmBRHGMlzoGpShEPCHtvuZvKQ=1867611580; else jmhIVYmBRHGMlzoGpShEPCHtvuZvKQ=881103423;if (jmhIVYmBRHGMlzoGpShEPCHtvuZvKQ == jmhIVYmBRHGMlzoGpShEPCHtvuZvKQ- 1 ) jmhIVYmBRHGMlzoGpShEPCHtvuZvKQ=753837208; else jmhIVYmBRHGMlzoGpShEPCHtvuZvKQ=1182072917;float KspQaMzBCTpAGUxmcweqwHMYdCWAoD=1095010946.080274227383596930047087776748f;if (KspQaMzBCTpAGUxmcweqwHMYdCWAoD - KspQaMzBCTpAGUxmcweqwHMYdCWAoD> 0.00000001 ) KspQaMzBCTpAGUxmcweqwHMYdCWAoD=2055924683.325063085326851214708252101461f; else KspQaMzBCTpAGUxmcweqwHMYdCWAoD=1497411266.859006964306682224656204018307f;if (KspQaMzBCTpAGUxmcweqwHMYdCWAoD - KspQaMzBCTpAGUxmcweqwHMYdCWAoD> 0.00000001 ) KspQaMzBCTpAGUxmcweqwHMYdCWAoD=1416206403.372557795635140377198781576509f; else KspQaMzBCTpAGUxmcweqwHMYdCWAoD=1391511553.977814414553912126484131969109f;if (KspQaMzBCTpAGUxmcweqwHMYdCWAoD - KspQaMzBCTpAGUxmcweqwHMYdCWAoD> 0.00000001 ) KspQaMzBCTpAGUxmcweqwHMYdCWAoD=266935373.220677844707464598055013584795f; else KspQaMzBCTpAGUxmcweqwHMYdCWAoD=856041779.532643434539388887748126432710f;if (KspQaMzBCTpAGUxmcweqwHMYdCWAoD - KspQaMzBCTpAGUxmcweqwHMYdCWAoD> 0.00000001 ) KspQaMzBCTpAGUxmcweqwHMYdCWAoD=1139741734.325927593226452784581816851722f; else KspQaMzBCTpAGUxmcweqwHMYdCWAoD=2014442582.273532446240563288475834962028f;if (KspQaMzBCTpAGUxmcweqwHMYdCWAoD - KspQaMzBCTpAGUxmcweqwHMYdCWAoD> 0.00000001 ) KspQaMzBCTpAGUxmcweqwHMYdCWAoD=1652289119.697205403134944235367436745967f; else KspQaMzBCTpAGUxmcweqwHMYdCWAoD=824246668.053866446998644100083776236156f;if (KspQaMzBCTpAGUxmcweqwHMYdCWAoD - KspQaMzBCTpAGUxmcweqwHMYdCWAoD> 0.00000001 ) KspQaMzBCTpAGUxmcweqwHMYdCWAoD=612644549.154619218980057980265699548773f; else KspQaMzBCTpAGUxmcweqwHMYdCWAoD=451774443.039084514699220383935709267627f;double aWZscFwjQWndhkbTjsHgZpWqTPArMm=1607480119.921830032787816089785528200270;if (aWZscFwjQWndhkbTjsHgZpWqTPArMm == aWZscFwjQWndhkbTjsHgZpWqTPArMm ) aWZscFwjQWndhkbTjsHgZpWqTPArMm=977061558.775495148863236560546601451364; else aWZscFwjQWndhkbTjsHgZpWqTPArMm=1103107069.087311243205193340384184417514;if (aWZscFwjQWndhkbTjsHgZpWqTPArMm == aWZscFwjQWndhkbTjsHgZpWqTPArMm ) aWZscFwjQWndhkbTjsHgZpWqTPArMm=592689085.488598702533744252822219091390; else aWZscFwjQWndhkbTjsHgZpWqTPArMm=1279449714.583082871564885254037670338960;if (aWZscFwjQWndhkbTjsHgZpWqTPArMm == aWZscFwjQWndhkbTjsHgZpWqTPArMm ) aWZscFwjQWndhkbTjsHgZpWqTPArMm=101663914.741216076359136844585747491476; else aWZscFwjQWndhkbTjsHgZpWqTPArMm=446821829.591293477038978954460458510493;if (aWZscFwjQWndhkbTjsHgZpWqTPArMm == aWZscFwjQWndhkbTjsHgZpWqTPArMm ) aWZscFwjQWndhkbTjsHgZpWqTPArMm=528430167.561096805448856788964802714378; else aWZscFwjQWndhkbTjsHgZpWqTPArMm=1465322903.309557808216912370393254986949;if (aWZscFwjQWndhkbTjsHgZpWqTPArMm == aWZscFwjQWndhkbTjsHgZpWqTPArMm ) aWZscFwjQWndhkbTjsHgZpWqTPArMm=1456382554.354815941909175990155419515568; else aWZscFwjQWndhkbTjsHgZpWqTPArMm=2087162118.106006320743040428919780495630;if (aWZscFwjQWndhkbTjsHgZpWqTPArMm == aWZscFwjQWndhkbTjsHgZpWqTPArMm ) aWZscFwjQWndhkbTjsHgZpWqTPArMm=1308853570.058380318968042247929776052761; else aWZscFwjQWndhkbTjsHgZpWqTPArMm=1304151027.289189586012105174825872138344;float gZhltvyiMlhDzAogiCuqnuUeUEmwXF=1223132133.910565929609414942843801661964f;if (gZhltvyiMlhDzAogiCuqnuUeUEmwXF - gZhltvyiMlhDzAogiCuqnuUeUEmwXF> 0.00000001 ) gZhltvyiMlhDzAogiCuqnuUeUEmwXF=651920504.874039662672083605485029811090f; else gZhltvyiMlhDzAogiCuqnuUeUEmwXF=1746539556.516583658648038183907658995522f;if (gZhltvyiMlhDzAogiCuqnuUeUEmwXF - gZhltvyiMlhDzAogiCuqnuUeUEmwXF> 0.00000001 ) gZhltvyiMlhDzAogiCuqnuUeUEmwXF=1107404362.877226750006785800284810496860f; else gZhltvyiMlhDzAogiCuqnuUeUEmwXF=140577047.813788181243491197429635906884f;if (gZhltvyiMlhDzAogiCuqnuUeUEmwXF - gZhltvyiMlhDzAogiCuqnuUeUEmwXF> 0.00000001 ) gZhltvyiMlhDzAogiCuqnuUeUEmwXF=1137075871.090030311873914153930955238988f; else gZhltvyiMlhDzAogiCuqnuUeUEmwXF=1409471514.230825447298766673186495425591f;if (gZhltvyiMlhDzAogiCuqnuUeUEmwXF - gZhltvyiMlhDzAogiCuqnuUeUEmwXF> 0.00000001 ) gZhltvyiMlhDzAogiCuqnuUeUEmwXF=1475635780.253464329981785339966691294551f; else gZhltvyiMlhDzAogiCuqnuUeUEmwXF=2038468532.196714540246521672120663567328f;if (gZhltvyiMlhDzAogiCuqnuUeUEmwXF - gZhltvyiMlhDzAogiCuqnuUeUEmwXF> 0.00000001 ) gZhltvyiMlhDzAogiCuqnuUeUEmwXF=536149415.188989518289083127172354263425f; else gZhltvyiMlhDzAogiCuqnuUeUEmwXF=690701922.832327864217404179155036683583f;if (gZhltvyiMlhDzAogiCuqnuUeUEmwXF - gZhltvyiMlhDzAogiCuqnuUeUEmwXF> 0.00000001 ) gZhltvyiMlhDzAogiCuqnuUeUEmwXF=499889066.629445877636496596010548081784f; else gZhltvyiMlhDzAogiCuqnuUeUEmwXF=305640804.449270916030352975019857329147f;double DwMNtlKLehrAApzKtfGFejiagtKsXI=1644312846.100436063475987519868979228356;if (DwMNtlKLehrAApzKtfGFejiagtKsXI == DwMNtlKLehrAApzKtfGFejiagtKsXI ) DwMNtlKLehrAApzKtfGFejiagtKsXI=1891630950.245552609619717441051501434131; else DwMNtlKLehrAApzKtfGFejiagtKsXI=1293840736.872824153502035926399741584513;if (DwMNtlKLehrAApzKtfGFejiagtKsXI == DwMNtlKLehrAApzKtfGFejiagtKsXI ) DwMNtlKLehrAApzKtfGFejiagtKsXI=205011359.958985809330955057751884313235; else DwMNtlKLehrAApzKtfGFejiagtKsXI=688555593.074065425022816999663887217478;if (DwMNtlKLehrAApzKtfGFejiagtKsXI == DwMNtlKLehrAApzKtfGFejiagtKsXI ) DwMNtlKLehrAApzKtfGFejiagtKsXI=1300242661.378437799174274018942468339896; else DwMNtlKLehrAApzKtfGFejiagtKsXI=212924471.805875548031159962330963601121;if (DwMNtlKLehrAApzKtfGFejiagtKsXI == DwMNtlKLehrAApzKtfGFejiagtKsXI ) DwMNtlKLehrAApzKtfGFejiagtKsXI=993347032.800437950968191142948856694328; else DwMNtlKLehrAApzKtfGFejiagtKsXI=1609102854.442724172312248397256039415594;if (DwMNtlKLehrAApzKtfGFejiagtKsXI == DwMNtlKLehrAApzKtfGFejiagtKsXI ) DwMNtlKLehrAApzKtfGFejiagtKsXI=968981099.373352748807064761941718252441; else DwMNtlKLehrAApzKtfGFejiagtKsXI=2098367147.979904909675055907575565466611;if (DwMNtlKLehrAApzKtfGFejiagtKsXI == DwMNtlKLehrAApzKtfGFejiagtKsXI ) DwMNtlKLehrAApzKtfGFejiagtKsXI=2082612646.130894927087238687335697651454; else DwMNtlKLehrAApzKtfGFejiagtKsXI=327256902.840964536219060934889038654424;float bhOIRlRjUEbBENedTkjNkfkbrFIOEz=1145762870.013812674187142182948773793697f;if (bhOIRlRjUEbBENedTkjNkfkbrFIOEz - bhOIRlRjUEbBENedTkjNkfkbrFIOEz> 0.00000001 ) bhOIRlRjUEbBENedTkjNkfkbrFIOEz=475389243.357034253072239749787310444123f; else bhOIRlRjUEbBENedTkjNkfkbrFIOEz=1536996063.918794111904098755406502014770f;if (bhOIRlRjUEbBENedTkjNkfkbrFIOEz - bhOIRlRjUEbBENedTkjNkfkbrFIOEz> 0.00000001 ) bhOIRlRjUEbBENedTkjNkfkbrFIOEz=1947337586.023116455674888432379072751789f; else bhOIRlRjUEbBENedTkjNkfkbrFIOEz=471497286.044299108241184272468250215147f;if (bhOIRlRjUEbBENedTkjNkfkbrFIOEz - bhOIRlRjUEbBENedTkjNkfkbrFIOEz> 0.00000001 ) bhOIRlRjUEbBENedTkjNkfkbrFIOEz=1263378019.588773070044524909969032189419f; else bhOIRlRjUEbBENedTkjNkfkbrFIOEz=996379073.507276352130353584741638966769f;if (bhOIRlRjUEbBENedTkjNkfkbrFIOEz - bhOIRlRjUEbBENedTkjNkfkbrFIOEz> 0.00000001 ) bhOIRlRjUEbBENedTkjNkfkbrFIOEz=470404248.567164835976894979971687998941f; else bhOIRlRjUEbBENedTkjNkfkbrFIOEz=1715073691.169477429075997695509790899037f;if (bhOIRlRjUEbBENedTkjNkfkbrFIOEz - bhOIRlRjUEbBENedTkjNkfkbrFIOEz> 0.00000001 ) bhOIRlRjUEbBENedTkjNkfkbrFIOEz=1774954936.600481852756090779031444347068f; else bhOIRlRjUEbBENedTkjNkfkbrFIOEz=701704200.940358561794385481023000736625f;if (bhOIRlRjUEbBENedTkjNkfkbrFIOEz - bhOIRlRjUEbBENedTkjNkfkbrFIOEz> 0.00000001 ) bhOIRlRjUEbBENedTkjNkfkbrFIOEz=743300299.589491685222945716997704605062f; else bhOIRlRjUEbBENedTkjNkfkbrFIOEz=1558445639.304731590481691126502074889690f;double eHNyJpIIZJMfaDgCGQZptbfCNMCsWG=856631165.479282225791746723651958385233;if (eHNyJpIIZJMfaDgCGQZptbfCNMCsWG == eHNyJpIIZJMfaDgCGQZptbfCNMCsWG ) eHNyJpIIZJMfaDgCGQZptbfCNMCsWG=937711917.837005793918237268863754462100; else eHNyJpIIZJMfaDgCGQZptbfCNMCsWG=1812024371.809297585203175473122857118364;if (eHNyJpIIZJMfaDgCGQZptbfCNMCsWG == eHNyJpIIZJMfaDgCGQZptbfCNMCsWG ) eHNyJpIIZJMfaDgCGQZptbfCNMCsWG=1560003724.360321905189377178919018915328; else eHNyJpIIZJMfaDgCGQZptbfCNMCsWG=910619338.922194396458905359105156645241;if (eHNyJpIIZJMfaDgCGQZptbfCNMCsWG == eHNyJpIIZJMfaDgCGQZptbfCNMCsWG ) eHNyJpIIZJMfaDgCGQZptbfCNMCsWG=301514890.678737838389256212467722387765; else eHNyJpIIZJMfaDgCGQZptbfCNMCsWG=1807785479.015689127963193690043498506645;if (eHNyJpIIZJMfaDgCGQZptbfCNMCsWG == eHNyJpIIZJMfaDgCGQZptbfCNMCsWG ) eHNyJpIIZJMfaDgCGQZptbfCNMCsWG=2070373119.017347578720561473089872263719; else eHNyJpIIZJMfaDgCGQZptbfCNMCsWG=438996033.458205686623343494813367725618;if (eHNyJpIIZJMfaDgCGQZptbfCNMCsWG == eHNyJpIIZJMfaDgCGQZptbfCNMCsWG ) eHNyJpIIZJMfaDgCGQZptbfCNMCsWG=753192524.587947237699846764997000729168; else eHNyJpIIZJMfaDgCGQZptbfCNMCsWG=1645785155.118825740627733563206273269723;if (eHNyJpIIZJMfaDgCGQZptbfCNMCsWG == eHNyJpIIZJMfaDgCGQZptbfCNMCsWG ) eHNyJpIIZJMfaDgCGQZptbfCNMCsWG=1319079595.817969203212437138837707294576; else eHNyJpIIZJMfaDgCGQZptbfCNMCsWG=1186192371.603891097536010595714200722178;double lPmbBxeYbGTBMxCmslnzvpfJXEMGjv=700558708.665622555395971257757391926302;if (lPmbBxeYbGTBMxCmslnzvpfJXEMGjv == lPmbBxeYbGTBMxCmslnzvpfJXEMGjv ) lPmbBxeYbGTBMxCmslnzvpfJXEMGjv=880719414.420850484904307826892584747481; else lPmbBxeYbGTBMxCmslnzvpfJXEMGjv=1501609843.625307523298496419410051105711;if (lPmbBxeYbGTBMxCmslnzvpfJXEMGjv == lPmbBxeYbGTBMxCmslnzvpfJXEMGjv ) lPmbBxeYbGTBMxCmslnzvpfJXEMGjv=1977735208.310338901030471519844807167430; else lPmbBxeYbGTBMxCmslnzvpfJXEMGjv=62719454.392299916951959179169697851327;if (lPmbBxeYbGTBMxCmslnzvpfJXEMGjv == lPmbBxeYbGTBMxCmslnzvpfJXEMGjv ) lPmbBxeYbGTBMxCmslnzvpfJXEMGjv=799897450.158185938370400306910947170554; else lPmbBxeYbGTBMxCmslnzvpfJXEMGjv=677952915.860080487534180939856652732111;if (lPmbBxeYbGTBMxCmslnzvpfJXEMGjv == lPmbBxeYbGTBMxCmslnzvpfJXEMGjv ) lPmbBxeYbGTBMxCmslnzvpfJXEMGjv=380934827.549552143002959786835180931836; else lPmbBxeYbGTBMxCmslnzvpfJXEMGjv=895251877.938709674989795481315648747973;if (lPmbBxeYbGTBMxCmslnzvpfJXEMGjv == lPmbBxeYbGTBMxCmslnzvpfJXEMGjv ) lPmbBxeYbGTBMxCmslnzvpfJXEMGjv=823436488.142718535012395649556162249006; else lPmbBxeYbGTBMxCmslnzvpfJXEMGjv=637421315.370195819805631336421853808067;if (lPmbBxeYbGTBMxCmslnzvpfJXEMGjv == lPmbBxeYbGTBMxCmslnzvpfJXEMGjv ) lPmbBxeYbGTBMxCmslnzvpfJXEMGjv=1061084304.063984321084185163292469653849; else lPmbBxeYbGTBMxCmslnzvpfJXEMGjv=598538461.896285375711554777875687670444;double WqKLyowxDosncsAqHatogCiRJBwdJZ=746191736.082831619274832415984597221603;if (WqKLyowxDosncsAqHatogCiRJBwdJZ == WqKLyowxDosncsAqHatogCiRJBwdJZ ) WqKLyowxDosncsAqHatogCiRJBwdJZ=85276114.440343666539018293042654760671; else WqKLyowxDosncsAqHatogCiRJBwdJZ=1225981481.101694560095955658913877189679;if (WqKLyowxDosncsAqHatogCiRJBwdJZ == WqKLyowxDosncsAqHatogCiRJBwdJZ ) WqKLyowxDosncsAqHatogCiRJBwdJZ=1537028632.300553843574406881011286934561; else WqKLyowxDosncsAqHatogCiRJBwdJZ=2030131132.509189640381644058241976823472;if (WqKLyowxDosncsAqHatogCiRJBwdJZ == WqKLyowxDosncsAqHatogCiRJBwdJZ ) WqKLyowxDosncsAqHatogCiRJBwdJZ=423070630.949208533724174164398733174446; else WqKLyowxDosncsAqHatogCiRJBwdJZ=1046846817.728945254653552780386928432714;if (WqKLyowxDosncsAqHatogCiRJBwdJZ == WqKLyowxDosncsAqHatogCiRJBwdJZ ) WqKLyowxDosncsAqHatogCiRJBwdJZ=844247980.520710734992075628149416181051; else WqKLyowxDosncsAqHatogCiRJBwdJZ=815087411.464123395216749595482468611464;if (WqKLyowxDosncsAqHatogCiRJBwdJZ == WqKLyowxDosncsAqHatogCiRJBwdJZ ) WqKLyowxDosncsAqHatogCiRJBwdJZ=1614106591.078363806243493388879767001751; else WqKLyowxDosncsAqHatogCiRJBwdJZ=1508341553.303135858670035917534470531237;if (WqKLyowxDosncsAqHatogCiRJBwdJZ == WqKLyowxDosncsAqHatogCiRJBwdJZ ) WqKLyowxDosncsAqHatogCiRJBwdJZ=1865788409.325505137953020074220423703360; else WqKLyowxDosncsAqHatogCiRJBwdJZ=1765076511.473245554188178246066801701390;float nifdsCsqaXKDHULNbZafnPBVejIaSW=626576058.542858702968640001245920410105f;if (nifdsCsqaXKDHULNbZafnPBVejIaSW - nifdsCsqaXKDHULNbZafnPBVejIaSW> 0.00000001 ) nifdsCsqaXKDHULNbZafnPBVejIaSW=357907524.343160207942469671307956063354f; else nifdsCsqaXKDHULNbZafnPBVejIaSW=42741591.871872658180531466185090850821f;if (nifdsCsqaXKDHULNbZafnPBVejIaSW - nifdsCsqaXKDHULNbZafnPBVejIaSW> 0.00000001 ) nifdsCsqaXKDHULNbZafnPBVejIaSW=677726225.806328170712322752660727089102f; else nifdsCsqaXKDHULNbZafnPBVejIaSW=592806567.394848521330758478921826119944f;if (nifdsCsqaXKDHULNbZafnPBVejIaSW - nifdsCsqaXKDHULNbZafnPBVejIaSW> 0.00000001 ) nifdsCsqaXKDHULNbZafnPBVejIaSW=392537802.018952968523134118962139257874f; else nifdsCsqaXKDHULNbZafnPBVejIaSW=1950371579.079743280651140980301208398971f;if (nifdsCsqaXKDHULNbZafnPBVejIaSW - nifdsCsqaXKDHULNbZafnPBVejIaSW> 0.00000001 ) nifdsCsqaXKDHULNbZafnPBVejIaSW=56733715.619466449193842219038768487255f; else nifdsCsqaXKDHULNbZafnPBVejIaSW=259759002.542015279454121847718005496218f;if (nifdsCsqaXKDHULNbZafnPBVejIaSW - nifdsCsqaXKDHULNbZafnPBVejIaSW> 0.00000001 ) nifdsCsqaXKDHULNbZafnPBVejIaSW=1431089544.930476294705169464765155864872f; else nifdsCsqaXKDHULNbZafnPBVejIaSW=664751847.916152575576402395604256399572f;if (nifdsCsqaXKDHULNbZafnPBVejIaSW - nifdsCsqaXKDHULNbZafnPBVejIaSW> 0.00000001 ) nifdsCsqaXKDHULNbZafnPBVejIaSW=87182850.253390001809029086457027986087f; else nifdsCsqaXKDHULNbZafnPBVejIaSW=1811577222.401443322240490534842423533641f;long xIVTBsdrhieSIDkePkUZIOWYBnrsFw=1318291131;if (xIVTBsdrhieSIDkePkUZIOWYBnrsFw == xIVTBsdrhieSIDkePkUZIOWYBnrsFw- 0 ) xIVTBsdrhieSIDkePkUZIOWYBnrsFw=2131098711; else xIVTBsdrhieSIDkePkUZIOWYBnrsFw=572887859;if (xIVTBsdrhieSIDkePkUZIOWYBnrsFw == xIVTBsdrhieSIDkePkUZIOWYBnrsFw- 0 ) xIVTBsdrhieSIDkePkUZIOWYBnrsFw=1100916257; else xIVTBsdrhieSIDkePkUZIOWYBnrsFw=248361034;if (xIVTBsdrhieSIDkePkUZIOWYBnrsFw == xIVTBsdrhieSIDkePkUZIOWYBnrsFw- 1 ) xIVTBsdrhieSIDkePkUZIOWYBnrsFw=1758833893; else xIVTBsdrhieSIDkePkUZIOWYBnrsFw=461149191;if (xIVTBsdrhieSIDkePkUZIOWYBnrsFw == xIVTBsdrhieSIDkePkUZIOWYBnrsFw- 1 ) xIVTBsdrhieSIDkePkUZIOWYBnrsFw=277488078; else xIVTBsdrhieSIDkePkUZIOWYBnrsFw=1981356244;if (xIVTBsdrhieSIDkePkUZIOWYBnrsFw == xIVTBsdrhieSIDkePkUZIOWYBnrsFw- 1 ) xIVTBsdrhieSIDkePkUZIOWYBnrsFw=657361863; else xIVTBsdrhieSIDkePkUZIOWYBnrsFw=1162647797;if (xIVTBsdrhieSIDkePkUZIOWYBnrsFw == xIVTBsdrhieSIDkePkUZIOWYBnrsFw- 0 ) xIVTBsdrhieSIDkePkUZIOWYBnrsFw=596096510; else xIVTBsdrhieSIDkePkUZIOWYBnrsFw=1439039544;double CLxnAqheYwqNBLrjGnUKQpEUkgRomb=1539224611.164069332791282229178328133734;if (CLxnAqheYwqNBLrjGnUKQpEUkgRomb == CLxnAqheYwqNBLrjGnUKQpEUkgRomb ) CLxnAqheYwqNBLrjGnUKQpEUkgRomb=396690139.986232159066008279299199694703; else CLxnAqheYwqNBLrjGnUKQpEUkgRomb=434508246.660892606117012243197454689875;if (CLxnAqheYwqNBLrjGnUKQpEUkgRomb == CLxnAqheYwqNBLrjGnUKQpEUkgRomb ) CLxnAqheYwqNBLrjGnUKQpEUkgRomb=240630937.856750621677429228745162621731; else CLxnAqheYwqNBLrjGnUKQpEUkgRomb=1574366228.394136237373352014657213070513;if (CLxnAqheYwqNBLrjGnUKQpEUkgRomb == CLxnAqheYwqNBLrjGnUKQpEUkgRomb ) CLxnAqheYwqNBLrjGnUKQpEUkgRomb=904658815.900149439310158391162819528126; else CLxnAqheYwqNBLrjGnUKQpEUkgRomb=1069363524.772547432378544816679955229162;if (CLxnAqheYwqNBLrjGnUKQpEUkgRomb == CLxnAqheYwqNBLrjGnUKQpEUkgRomb ) CLxnAqheYwqNBLrjGnUKQpEUkgRomb=353653934.232374872107080448581265883732; else CLxnAqheYwqNBLrjGnUKQpEUkgRomb=282388182.937025727104558207689279107769;if (CLxnAqheYwqNBLrjGnUKQpEUkgRomb == CLxnAqheYwqNBLrjGnUKQpEUkgRomb ) CLxnAqheYwqNBLrjGnUKQpEUkgRomb=1169247766.124038007572149781869716476993; else CLxnAqheYwqNBLrjGnUKQpEUkgRomb=1277269630.817918153021111713142690051976;if (CLxnAqheYwqNBLrjGnUKQpEUkgRomb == CLxnAqheYwqNBLrjGnUKQpEUkgRomb ) CLxnAqheYwqNBLrjGnUKQpEUkgRomb=599041588.221577624173984970577385122797; else CLxnAqheYwqNBLrjGnUKQpEUkgRomb=1905802430.829791064529018971628240095646;int tRmQaVLxTrCdBcnSnoLaGEkreovmye=142271933;if (tRmQaVLxTrCdBcnSnoLaGEkreovmye == tRmQaVLxTrCdBcnSnoLaGEkreovmye- 1 ) tRmQaVLxTrCdBcnSnoLaGEkreovmye=538969692; else tRmQaVLxTrCdBcnSnoLaGEkreovmye=547497278;if (tRmQaVLxTrCdBcnSnoLaGEkreovmye == tRmQaVLxTrCdBcnSnoLaGEkreovmye- 1 ) tRmQaVLxTrCdBcnSnoLaGEkreovmye=1781461447; else tRmQaVLxTrCdBcnSnoLaGEkreovmye=1251895266;if (tRmQaVLxTrCdBcnSnoLaGEkreovmye == tRmQaVLxTrCdBcnSnoLaGEkreovmye- 0 ) tRmQaVLxTrCdBcnSnoLaGEkreovmye=330779267; else tRmQaVLxTrCdBcnSnoLaGEkreovmye=2139853789;if (tRmQaVLxTrCdBcnSnoLaGEkreovmye == tRmQaVLxTrCdBcnSnoLaGEkreovmye- 0 ) tRmQaVLxTrCdBcnSnoLaGEkreovmye=1015225812; else tRmQaVLxTrCdBcnSnoLaGEkreovmye=2044241980;if (tRmQaVLxTrCdBcnSnoLaGEkreovmye == tRmQaVLxTrCdBcnSnoLaGEkreovmye- 0 ) tRmQaVLxTrCdBcnSnoLaGEkreovmye=1912737160; else tRmQaVLxTrCdBcnSnoLaGEkreovmye=843853628;if (tRmQaVLxTrCdBcnSnoLaGEkreovmye == tRmQaVLxTrCdBcnSnoLaGEkreovmye- 1 ) tRmQaVLxTrCdBcnSnoLaGEkreovmye=2082928351; else tRmQaVLxTrCdBcnSnoLaGEkreovmye=1377373652;long qqmCUnOxMYQlDBHnIkuMZnrbqYSnqe=667747581;if (qqmCUnOxMYQlDBHnIkuMZnrbqYSnqe == qqmCUnOxMYQlDBHnIkuMZnrbqYSnqe- 0 ) qqmCUnOxMYQlDBHnIkuMZnrbqYSnqe=1594283095; else qqmCUnOxMYQlDBHnIkuMZnrbqYSnqe=695441886;if (qqmCUnOxMYQlDBHnIkuMZnrbqYSnqe == qqmCUnOxMYQlDBHnIkuMZnrbqYSnqe- 1 ) qqmCUnOxMYQlDBHnIkuMZnrbqYSnqe=1414887083; else qqmCUnOxMYQlDBHnIkuMZnrbqYSnqe=312192929;if (qqmCUnOxMYQlDBHnIkuMZnrbqYSnqe == qqmCUnOxMYQlDBHnIkuMZnrbqYSnqe- 0 ) qqmCUnOxMYQlDBHnIkuMZnrbqYSnqe=1231203721; else qqmCUnOxMYQlDBHnIkuMZnrbqYSnqe=1542489280;if (qqmCUnOxMYQlDBHnIkuMZnrbqYSnqe == qqmCUnOxMYQlDBHnIkuMZnrbqYSnqe- 0 ) qqmCUnOxMYQlDBHnIkuMZnrbqYSnqe=413632316; else qqmCUnOxMYQlDBHnIkuMZnrbqYSnqe=1061735187;if (qqmCUnOxMYQlDBHnIkuMZnrbqYSnqe == qqmCUnOxMYQlDBHnIkuMZnrbqYSnqe- 1 ) qqmCUnOxMYQlDBHnIkuMZnrbqYSnqe=219233925; else qqmCUnOxMYQlDBHnIkuMZnrbqYSnqe=818528075;if (qqmCUnOxMYQlDBHnIkuMZnrbqYSnqe == qqmCUnOxMYQlDBHnIkuMZnrbqYSnqe- 1 ) qqmCUnOxMYQlDBHnIkuMZnrbqYSnqe=1226596915; else qqmCUnOxMYQlDBHnIkuMZnrbqYSnqe=481564263;long cngvfPrkbQcTVquLqohQgHJFQpNYLv=1624691105;if (cngvfPrkbQcTVquLqohQgHJFQpNYLv == cngvfPrkbQcTVquLqohQgHJFQpNYLv- 0 ) cngvfPrkbQcTVquLqohQgHJFQpNYLv=1094261561; else cngvfPrkbQcTVquLqohQgHJFQpNYLv=453824577;if (cngvfPrkbQcTVquLqohQgHJFQpNYLv == cngvfPrkbQcTVquLqohQgHJFQpNYLv- 1 ) cngvfPrkbQcTVquLqohQgHJFQpNYLv=900069852; else cngvfPrkbQcTVquLqohQgHJFQpNYLv=412150875;if (cngvfPrkbQcTVquLqohQgHJFQpNYLv == cngvfPrkbQcTVquLqohQgHJFQpNYLv- 1 ) cngvfPrkbQcTVquLqohQgHJFQpNYLv=1129364163; else cngvfPrkbQcTVquLqohQgHJFQpNYLv=2113540780;if (cngvfPrkbQcTVquLqohQgHJFQpNYLv == cngvfPrkbQcTVquLqohQgHJFQpNYLv- 0 ) cngvfPrkbQcTVquLqohQgHJFQpNYLv=1208445344; else cngvfPrkbQcTVquLqohQgHJFQpNYLv=404319997;if (cngvfPrkbQcTVquLqohQgHJFQpNYLv == cngvfPrkbQcTVquLqohQgHJFQpNYLv- 1 ) cngvfPrkbQcTVquLqohQgHJFQpNYLv=1566766081; else cngvfPrkbQcTVquLqohQgHJFQpNYLv=343872604;if (cngvfPrkbQcTVquLqohQgHJFQpNYLv == cngvfPrkbQcTVquLqohQgHJFQpNYLv- 1 ) cngvfPrkbQcTVquLqohQgHJFQpNYLv=1809112715; else cngvfPrkbQcTVquLqohQgHJFQpNYLv=60805187;double fgTmXTVMbTvallZNdcxKxwDfGWZrMQ=404069948.000299846486931505062001457355;if (fgTmXTVMbTvallZNdcxKxwDfGWZrMQ == fgTmXTVMbTvallZNdcxKxwDfGWZrMQ ) fgTmXTVMbTvallZNdcxKxwDfGWZrMQ=1977158781.725653072029468546954353800985; else fgTmXTVMbTvallZNdcxKxwDfGWZrMQ=2078777654.260177248963415375401250257285;if (fgTmXTVMbTvallZNdcxKxwDfGWZrMQ == fgTmXTVMbTvallZNdcxKxwDfGWZrMQ ) fgTmXTVMbTvallZNdcxKxwDfGWZrMQ=311519335.329886039241810105854659412093; else fgTmXTVMbTvallZNdcxKxwDfGWZrMQ=264222081.006464564935626144025343287148;if (fgTmXTVMbTvallZNdcxKxwDfGWZrMQ == fgTmXTVMbTvallZNdcxKxwDfGWZrMQ ) fgTmXTVMbTvallZNdcxKxwDfGWZrMQ=1912750969.504724555774203904347808876328; else fgTmXTVMbTvallZNdcxKxwDfGWZrMQ=1903676954.843643100369045225139480181606;if (fgTmXTVMbTvallZNdcxKxwDfGWZrMQ == fgTmXTVMbTvallZNdcxKxwDfGWZrMQ ) fgTmXTVMbTvallZNdcxKxwDfGWZrMQ=1058551859.924793166091419986366139057467; else fgTmXTVMbTvallZNdcxKxwDfGWZrMQ=1143964059.350544636317025818230544080575;if (fgTmXTVMbTvallZNdcxKxwDfGWZrMQ == fgTmXTVMbTvallZNdcxKxwDfGWZrMQ ) fgTmXTVMbTvallZNdcxKxwDfGWZrMQ=133864962.780974198536115921435024792756; else fgTmXTVMbTvallZNdcxKxwDfGWZrMQ=719726260.426164716282170543917015186739;if (fgTmXTVMbTvallZNdcxKxwDfGWZrMQ == fgTmXTVMbTvallZNdcxKxwDfGWZrMQ ) fgTmXTVMbTvallZNdcxKxwDfGWZrMQ=1032661341.254211005231636396425085051219; else fgTmXTVMbTvallZNdcxKxwDfGWZrMQ=1659734839.819813996964879649089131427027;double qxiXGMpKLXACZhFJEcUEOLNylsmUsi=1393567865.716064459523426356396563842099;if (qxiXGMpKLXACZhFJEcUEOLNylsmUsi == qxiXGMpKLXACZhFJEcUEOLNylsmUsi ) qxiXGMpKLXACZhFJEcUEOLNylsmUsi=1969644998.292601800155140425258414625433; else qxiXGMpKLXACZhFJEcUEOLNylsmUsi=458237246.347043182565635153443795188909;if (qxiXGMpKLXACZhFJEcUEOLNylsmUsi == qxiXGMpKLXACZhFJEcUEOLNylsmUsi ) qxiXGMpKLXACZhFJEcUEOLNylsmUsi=283112091.758881910734379399957209066306; else qxiXGMpKLXACZhFJEcUEOLNylsmUsi=900450492.539491895535371052680006265082;if (qxiXGMpKLXACZhFJEcUEOLNylsmUsi == qxiXGMpKLXACZhFJEcUEOLNylsmUsi ) qxiXGMpKLXACZhFJEcUEOLNylsmUsi=2012591613.562521699860453393812482763334; else qxiXGMpKLXACZhFJEcUEOLNylsmUsi=1035443621.782307496147483066397227119197;if (qxiXGMpKLXACZhFJEcUEOLNylsmUsi == qxiXGMpKLXACZhFJEcUEOLNylsmUsi ) qxiXGMpKLXACZhFJEcUEOLNylsmUsi=1685867065.936528916700887610070714757539; else qxiXGMpKLXACZhFJEcUEOLNylsmUsi=1628225962.841922197526170754051012823397;if (qxiXGMpKLXACZhFJEcUEOLNylsmUsi == qxiXGMpKLXACZhFJEcUEOLNylsmUsi ) qxiXGMpKLXACZhFJEcUEOLNylsmUsi=1634480189.259797096662455574501358893921; else qxiXGMpKLXACZhFJEcUEOLNylsmUsi=1010567086.091048849486850067835989690191;if (qxiXGMpKLXACZhFJEcUEOLNylsmUsi == qxiXGMpKLXACZhFJEcUEOLNylsmUsi ) qxiXGMpKLXACZhFJEcUEOLNylsmUsi=892910495.518668242883623651993729621355; else qxiXGMpKLXACZhFJEcUEOLNylsmUsi=887331117.975898839392506236704031889005;double uykwCpoNYqLGeDDKLOVqYeWIvYZzFw=1242953643.772882831383241149751336948679;if (uykwCpoNYqLGeDDKLOVqYeWIvYZzFw == uykwCpoNYqLGeDDKLOVqYeWIvYZzFw ) uykwCpoNYqLGeDDKLOVqYeWIvYZzFw=1890414962.136568307216689530564071744704; else uykwCpoNYqLGeDDKLOVqYeWIvYZzFw=822526967.537225878494255686852821627693;if (uykwCpoNYqLGeDDKLOVqYeWIvYZzFw == uykwCpoNYqLGeDDKLOVqYeWIvYZzFw ) uykwCpoNYqLGeDDKLOVqYeWIvYZzFw=651889129.471489047477318192717996371671; else uykwCpoNYqLGeDDKLOVqYeWIvYZzFw=2078297243.928602387674948661353509668622;if (uykwCpoNYqLGeDDKLOVqYeWIvYZzFw == uykwCpoNYqLGeDDKLOVqYeWIvYZzFw ) uykwCpoNYqLGeDDKLOVqYeWIvYZzFw=207649960.847665972682615305871143943004; else uykwCpoNYqLGeDDKLOVqYeWIvYZzFw=757470196.586593849370078209437385181583;if (uykwCpoNYqLGeDDKLOVqYeWIvYZzFw == uykwCpoNYqLGeDDKLOVqYeWIvYZzFw ) uykwCpoNYqLGeDDKLOVqYeWIvYZzFw=1859423783.397821613267761227685578820142; else uykwCpoNYqLGeDDKLOVqYeWIvYZzFw=1338068254.336718326769738173506401983000;if (uykwCpoNYqLGeDDKLOVqYeWIvYZzFw == uykwCpoNYqLGeDDKLOVqYeWIvYZzFw ) uykwCpoNYqLGeDDKLOVqYeWIvYZzFw=1542756365.157293116390292685186775604683; else uykwCpoNYqLGeDDKLOVqYeWIvYZzFw=1903589440.543671364507821732931903549360;if (uykwCpoNYqLGeDDKLOVqYeWIvYZzFw == uykwCpoNYqLGeDDKLOVqYeWIvYZzFw ) uykwCpoNYqLGeDDKLOVqYeWIvYZzFw=278285913.963386862143169005497376375113; else uykwCpoNYqLGeDDKLOVqYeWIvYZzFw=409637903.300465454607080884565498370812;long kYxqRQlyUcWEtwyOqeYrAfboxHLDvZ=1033104532;if (kYxqRQlyUcWEtwyOqeYrAfboxHLDvZ == kYxqRQlyUcWEtwyOqeYrAfboxHLDvZ- 1 ) kYxqRQlyUcWEtwyOqeYrAfboxHLDvZ=309221874; else kYxqRQlyUcWEtwyOqeYrAfboxHLDvZ=415544850;if (kYxqRQlyUcWEtwyOqeYrAfboxHLDvZ == kYxqRQlyUcWEtwyOqeYrAfboxHLDvZ- 1 ) kYxqRQlyUcWEtwyOqeYrAfboxHLDvZ=238643836; else kYxqRQlyUcWEtwyOqeYrAfboxHLDvZ=675681574;if (kYxqRQlyUcWEtwyOqeYrAfboxHLDvZ == kYxqRQlyUcWEtwyOqeYrAfboxHLDvZ- 1 ) kYxqRQlyUcWEtwyOqeYrAfboxHLDvZ=840651838; else kYxqRQlyUcWEtwyOqeYrAfboxHLDvZ=175146268;if (kYxqRQlyUcWEtwyOqeYrAfboxHLDvZ == kYxqRQlyUcWEtwyOqeYrAfboxHLDvZ- 1 ) kYxqRQlyUcWEtwyOqeYrAfboxHLDvZ=538961982; else kYxqRQlyUcWEtwyOqeYrAfboxHLDvZ=941715160;if (kYxqRQlyUcWEtwyOqeYrAfboxHLDvZ == kYxqRQlyUcWEtwyOqeYrAfboxHLDvZ- 0 ) kYxqRQlyUcWEtwyOqeYrAfboxHLDvZ=392910589; else kYxqRQlyUcWEtwyOqeYrAfboxHLDvZ=830777020;if (kYxqRQlyUcWEtwyOqeYrAfboxHLDvZ == kYxqRQlyUcWEtwyOqeYrAfboxHLDvZ- 0 ) kYxqRQlyUcWEtwyOqeYrAfboxHLDvZ=777225539; else kYxqRQlyUcWEtwyOqeYrAfboxHLDvZ=902183883;float qmEHStlOpLsbiRhjVlYtHRuxkYZQkE=1928521480.147913635935316973857828871631f;if (qmEHStlOpLsbiRhjVlYtHRuxkYZQkE - qmEHStlOpLsbiRhjVlYtHRuxkYZQkE> 0.00000001 ) qmEHStlOpLsbiRhjVlYtHRuxkYZQkE=791909893.434312080929627864764406932914f; else qmEHStlOpLsbiRhjVlYtHRuxkYZQkE=1937191345.300561100599394931101708713949f;if (qmEHStlOpLsbiRhjVlYtHRuxkYZQkE - qmEHStlOpLsbiRhjVlYtHRuxkYZQkE> 0.00000001 ) qmEHStlOpLsbiRhjVlYtHRuxkYZQkE=1916803524.118772926686537512311281371386f; else qmEHStlOpLsbiRhjVlYtHRuxkYZQkE=90729907.937258273202563628120263893741f;if (qmEHStlOpLsbiRhjVlYtHRuxkYZQkE - qmEHStlOpLsbiRhjVlYtHRuxkYZQkE> 0.00000001 ) qmEHStlOpLsbiRhjVlYtHRuxkYZQkE=2029256067.274186049944922073985436687446f; else qmEHStlOpLsbiRhjVlYtHRuxkYZQkE=103837134.554304858869039635283196694950f;if (qmEHStlOpLsbiRhjVlYtHRuxkYZQkE - qmEHStlOpLsbiRhjVlYtHRuxkYZQkE> 0.00000001 ) qmEHStlOpLsbiRhjVlYtHRuxkYZQkE=1185917905.970449172932513741706236357872f; else qmEHStlOpLsbiRhjVlYtHRuxkYZQkE=1212174802.091890979166336268405425584419f;if (qmEHStlOpLsbiRhjVlYtHRuxkYZQkE - qmEHStlOpLsbiRhjVlYtHRuxkYZQkE> 0.00000001 ) qmEHStlOpLsbiRhjVlYtHRuxkYZQkE=746297110.231104173388265000120958868742f; else qmEHStlOpLsbiRhjVlYtHRuxkYZQkE=1092527484.656003634319512727058440039344f;if (qmEHStlOpLsbiRhjVlYtHRuxkYZQkE - qmEHStlOpLsbiRhjVlYtHRuxkYZQkE> 0.00000001 ) qmEHStlOpLsbiRhjVlYtHRuxkYZQkE=1505177774.754283337257791145614179422617f; else qmEHStlOpLsbiRhjVlYtHRuxkYZQkE=109002173.539066544940081286999473068696f;double jKBlJqlNcfDEkiVHvoKXekGlEgeyAn=2085769684.042370345618127090634170821707;if (jKBlJqlNcfDEkiVHvoKXekGlEgeyAn == jKBlJqlNcfDEkiVHvoKXekGlEgeyAn ) jKBlJqlNcfDEkiVHvoKXekGlEgeyAn=1519503963.702348158892142428624496397256; else jKBlJqlNcfDEkiVHvoKXekGlEgeyAn=2007289610.075892699095385213850328442715;if (jKBlJqlNcfDEkiVHvoKXekGlEgeyAn == jKBlJqlNcfDEkiVHvoKXekGlEgeyAn ) jKBlJqlNcfDEkiVHvoKXekGlEgeyAn=578109243.190393836184073922119541686756; else jKBlJqlNcfDEkiVHvoKXekGlEgeyAn=1465315362.109491609358787735552160675982;if (jKBlJqlNcfDEkiVHvoKXekGlEgeyAn == jKBlJqlNcfDEkiVHvoKXekGlEgeyAn ) jKBlJqlNcfDEkiVHvoKXekGlEgeyAn=114183293.718119678343397187495131404983; else jKBlJqlNcfDEkiVHvoKXekGlEgeyAn=1399618440.673864178927822088369969779871;if (jKBlJqlNcfDEkiVHvoKXekGlEgeyAn == jKBlJqlNcfDEkiVHvoKXekGlEgeyAn ) jKBlJqlNcfDEkiVHvoKXekGlEgeyAn=402301805.579587053411958931952773472420; else jKBlJqlNcfDEkiVHvoKXekGlEgeyAn=1715268694.630855934504595901194893980622;if (jKBlJqlNcfDEkiVHvoKXekGlEgeyAn == jKBlJqlNcfDEkiVHvoKXekGlEgeyAn ) jKBlJqlNcfDEkiVHvoKXekGlEgeyAn=283564985.674399121300855928967133785897; else jKBlJqlNcfDEkiVHvoKXekGlEgeyAn=1857657616.632304021789199415807386554171;if (jKBlJqlNcfDEkiVHvoKXekGlEgeyAn == jKBlJqlNcfDEkiVHvoKXekGlEgeyAn ) jKBlJqlNcfDEkiVHvoKXekGlEgeyAn=1685386654.443488956290728915547000136685; else jKBlJqlNcfDEkiVHvoKXekGlEgeyAn=1524356587.106035671256071766974964444435;float NkAFvXlxSPTURqFEQpWmBnGFGTLerD=1371535623.577746555918375627189023545913f;if (NkAFvXlxSPTURqFEQpWmBnGFGTLerD - NkAFvXlxSPTURqFEQpWmBnGFGTLerD> 0.00000001 ) NkAFvXlxSPTURqFEQpWmBnGFGTLerD=493001946.131689498890618226954890355531f; else NkAFvXlxSPTURqFEQpWmBnGFGTLerD=497248080.174298779633030052532006945144f;if (NkAFvXlxSPTURqFEQpWmBnGFGTLerD - NkAFvXlxSPTURqFEQpWmBnGFGTLerD> 0.00000001 ) NkAFvXlxSPTURqFEQpWmBnGFGTLerD=1765211657.596772643055733186140400927936f; else NkAFvXlxSPTURqFEQpWmBnGFGTLerD=1291171741.473624836842069848496911410696f;if (NkAFvXlxSPTURqFEQpWmBnGFGTLerD - NkAFvXlxSPTURqFEQpWmBnGFGTLerD> 0.00000001 ) NkAFvXlxSPTURqFEQpWmBnGFGTLerD=1926874275.681769312946130954228055156231f; else NkAFvXlxSPTURqFEQpWmBnGFGTLerD=1547816863.524137231053655099257658863671f;if (NkAFvXlxSPTURqFEQpWmBnGFGTLerD - NkAFvXlxSPTURqFEQpWmBnGFGTLerD> 0.00000001 ) NkAFvXlxSPTURqFEQpWmBnGFGTLerD=1151332112.419153311382427650117718598638f; else NkAFvXlxSPTURqFEQpWmBnGFGTLerD=205384740.467075551289998533232456570167f;if (NkAFvXlxSPTURqFEQpWmBnGFGTLerD - NkAFvXlxSPTURqFEQpWmBnGFGTLerD> 0.00000001 ) NkAFvXlxSPTURqFEQpWmBnGFGTLerD=1377110592.678402646210094992016550612161f; else NkAFvXlxSPTURqFEQpWmBnGFGTLerD=1173952681.549572160799802246378826112989f;if (NkAFvXlxSPTURqFEQpWmBnGFGTLerD - NkAFvXlxSPTURqFEQpWmBnGFGTLerD> 0.00000001 ) NkAFvXlxSPTURqFEQpWmBnGFGTLerD=2076868111.059687163284333885847690373456f; else NkAFvXlxSPTURqFEQpWmBnGFGTLerD=1895608001.038185033629419569434714357031f;float kElKYdOepnaNNMpNmBrIXAHKPMXpOo=1264165074.871160419652521661396349830604f;if (kElKYdOepnaNNMpNmBrIXAHKPMXpOo - kElKYdOepnaNNMpNmBrIXAHKPMXpOo> 0.00000001 ) kElKYdOepnaNNMpNmBrIXAHKPMXpOo=283430497.500497790616946214603090254234f; else kElKYdOepnaNNMpNmBrIXAHKPMXpOo=838939502.092516224407288140362700251134f;if (kElKYdOepnaNNMpNmBrIXAHKPMXpOo - kElKYdOepnaNNMpNmBrIXAHKPMXpOo> 0.00000001 ) kElKYdOepnaNNMpNmBrIXAHKPMXpOo=1530163567.456663454100031692322507339337f; else kElKYdOepnaNNMpNmBrIXAHKPMXpOo=1150960496.529015262570727288976092306043f;if (kElKYdOepnaNNMpNmBrIXAHKPMXpOo - kElKYdOepnaNNMpNmBrIXAHKPMXpOo> 0.00000001 ) kElKYdOepnaNNMpNmBrIXAHKPMXpOo=1740595146.422769112743085288116246164882f; else kElKYdOepnaNNMpNmBrIXAHKPMXpOo=2092755779.932925806817734401712894739671f;if (kElKYdOepnaNNMpNmBrIXAHKPMXpOo - kElKYdOepnaNNMpNmBrIXAHKPMXpOo> 0.00000001 ) kElKYdOepnaNNMpNmBrIXAHKPMXpOo=1814338187.643082117059330961387327124939f; else kElKYdOepnaNNMpNmBrIXAHKPMXpOo=1539775100.624638575239577466588940542797f;if (kElKYdOepnaNNMpNmBrIXAHKPMXpOo - kElKYdOepnaNNMpNmBrIXAHKPMXpOo> 0.00000001 ) kElKYdOepnaNNMpNmBrIXAHKPMXpOo=1534889540.839843681259626985187855289520f; else kElKYdOepnaNNMpNmBrIXAHKPMXpOo=201534830.798940083498633932476762009467f;if (kElKYdOepnaNNMpNmBrIXAHKPMXpOo - kElKYdOepnaNNMpNmBrIXAHKPMXpOo> 0.00000001 ) kElKYdOepnaNNMpNmBrIXAHKPMXpOo=1954648569.517267188691380932236716403795f; else kElKYdOepnaNNMpNmBrIXAHKPMXpOo=171598451.262708248796598659327733181187f;float VrRWNucnMiuXcXBLJmsXXVnkaStfLg=1936540857.583121455850223955350052036000f;if (VrRWNucnMiuXcXBLJmsXXVnkaStfLg - VrRWNucnMiuXcXBLJmsXXVnkaStfLg> 0.00000001 ) VrRWNucnMiuXcXBLJmsXXVnkaStfLg=765225689.116515059246579280460777164129f; else VrRWNucnMiuXcXBLJmsXXVnkaStfLg=133642151.555574481800612568386775054046f;if (VrRWNucnMiuXcXBLJmsXXVnkaStfLg - VrRWNucnMiuXcXBLJmsXXVnkaStfLg> 0.00000001 ) VrRWNucnMiuXcXBLJmsXXVnkaStfLg=1763483874.194079100590966016670794499152f; else VrRWNucnMiuXcXBLJmsXXVnkaStfLg=259909628.144681765346980681915857751178f;if (VrRWNucnMiuXcXBLJmsXXVnkaStfLg - VrRWNucnMiuXcXBLJmsXXVnkaStfLg> 0.00000001 ) VrRWNucnMiuXcXBLJmsXXVnkaStfLg=1658501152.116855749646233591482825830703f; else VrRWNucnMiuXcXBLJmsXXVnkaStfLg=1403658429.079724114605347073427170749202f;if (VrRWNucnMiuXcXBLJmsXXVnkaStfLg - VrRWNucnMiuXcXBLJmsXXVnkaStfLg> 0.00000001 ) VrRWNucnMiuXcXBLJmsXXVnkaStfLg=158895409.605668751058898869897838700019f; else VrRWNucnMiuXcXBLJmsXXVnkaStfLg=2119170747.591201078032258949209142597707f;if (VrRWNucnMiuXcXBLJmsXXVnkaStfLg - VrRWNucnMiuXcXBLJmsXXVnkaStfLg> 0.00000001 ) VrRWNucnMiuXcXBLJmsXXVnkaStfLg=855957498.875356769789296669087292156072f; else VrRWNucnMiuXcXBLJmsXXVnkaStfLg=372724503.548127891328052096603982179330f;if (VrRWNucnMiuXcXBLJmsXXVnkaStfLg - VrRWNucnMiuXcXBLJmsXXVnkaStfLg> 0.00000001 ) VrRWNucnMiuXcXBLJmsXXVnkaStfLg=88204770.180835217098986924017058966615f; else VrRWNucnMiuXcXBLJmsXXVnkaStfLg=1087714259.957125922898072600626642413308f;long fCIGTFncFDyHKMpBrdQegxuyOGWkGK=2001088823;if (fCIGTFncFDyHKMpBrdQegxuyOGWkGK == fCIGTFncFDyHKMpBrdQegxuyOGWkGK- 0 ) fCIGTFncFDyHKMpBrdQegxuyOGWkGK=1673100164; else fCIGTFncFDyHKMpBrdQegxuyOGWkGK=421221580;if (fCIGTFncFDyHKMpBrdQegxuyOGWkGK == fCIGTFncFDyHKMpBrdQegxuyOGWkGK- 0 ) fCIGTFncFDyHKMpBrdQegxuyOGWkGK=1240926090; else fCIGTFncFDyHKMpBrdQegxuyOGWkGK=1288788802;if (fCIGTFncFDyHKMpBrdQegxuyOGWkGK == fCIGTFncFDyHKMpBrdQegxuyOGWkGK- 1 ) fCIGTFncFDyHKMpBrdQegxuyOGWkGK=1574559980; else fCIGTFncFDyHKMpBrdQegxuyOGWkGK=1323824946;if (fCIGTFncFDyHKMpBrdQegxuyOGWkGK == fCIGTFncFDyHKMpBrdQegxuyOGWkGK- 0 ) fCIGTFncFDyHKMpBrdQegxuyOGWkGK=1778424124; else fCIGTFncFDyHKMpBrdQegxuyOGWkGK=547729814;if (fCIGTFncFDyHKMpBrdQegxuyOGWkGK == fCIGTFncFDyHKMpBrdQegxuyOGWkGK- 1 ) fCIGTFncFDyHKMpBrdQegxuyOGWkGK=830393540; else fCIGTFncFDyHKMpBrdQegxuyOGWkGK=225436557;if (fCIGTFncFDyHKMpBrdQegxuyOGWkGK == fCIGTFncFDyHKMpBrdQegxuyOGWkGK- 1 ) fCIGTFncFDyHKMpBrdQegxuyOGWkGK=1903765848; else fCIGTFncFDyHKMpBrdQegxuyOGWkGK=1725339445;long LbwHwFeylnjWRrATfwrLuuXzPadmJt=1715549274;if (LbwHwFeylnjWRrATfwrLuuXzPadmJt == LbwHwFeylnjWRrATfwrLuuXzPadmJt- 0 ) LbwHwFeylnjWRrATfwrLuuXzPadmJt=2089506183; else LbwHwFeylnjWRrATfwrLuuXzPadmJt=140327254;if (LbwHwFeylnjWRrATfwrLuuXzPadmJt == LbwHwFeylnjWRrATfwrLuuXzPadmJt- 0 ) LbwHwFeylnjWRrATfwrLuuXzPadmJt=1130382312; else LbwHwFeylnjWRrATfwrLuuXzPadmJt=683809990;if (LbwHwFeylnjWRrATfwrLuuXzPadmJt == LbwHwFeylnjWRrATfwrLuuXzPadmJt- 1 ) LbwHwFeylnjWRrATfwrLuuXzPadmJt=813959582; else LbwHwFeylnjWRrATfwrLuuXzPadmJt=2072131471;if (LbwHwFeylnjWRrATfwrLuuXzPadmJt == LbwHwFeylnjWRrATfwrLuuXzPadmJt- 1 ) LbwHwFeylnjWRrATfwrLuuXzPadmJt=1753147569; else LbwHwFeylnjWRrATfwrLuuXzPadmJt=1027255715;if (LbwHwFeylnjWRrATfwrLuuXzPadmJt == LbwHwFeylnjWRrATfwrLuuXzPadmJt- 1 ) LbwHwFeylnjWRrATfwrLuuXzPadmJt=680044093; else LbwHwFeylnjWRrATfwrLuuXzPadmJt=1558476467;if (LbwHwFeylnjWRrATfwrLuuXzPadmJt == LbwHwFeylnjWRrATfwrLuuXzPadmJt- 0 ) LbwHwFeylnjWRrATfwrLuuXzPadmJt=295002998; else LbwHwFeylnjWRrATfwrLuuXzPadmJt=1367870643;long pmHzTyPhYeVxMMSpjVsPtDhjZwhhnh=880712089;if (pmHzTyPhYeVxMMSpjVsPtDhjZwhhnh == pmHzTyPhYeVxMMSpjVsPtDhjZwhhnh- 0 ) pmHzTyPhYeVxMMSpjVsPtDhjZwhhnh=1451201924; else pmHzTyPhYeVxMMSpjVsPtDhjZwhhnh=2101043393;if (pmHzTyPhYeVxMMSpjVsPtDhjZwhhnh == pmHzTyPhYeVxMMSpjVsPtDhjZwhhnh- 0 ) pmHzTyPhYeVxMMSpjVsPtDhjZwhhnh=645981934; else pmHzTyPhYeVxMMSpjVsPtDhjZwhhnh=144712547;if (pmHzTyPhYeVxMMSpjVsPtDhjZwhhnh == pmHzTyPhYeVxMMSpjVsPtDhjZwhhnh- 1 ) pmHzTyPhYeVxMMSpjVsPtDhjZwhhnh=568118875; else pmHzTyPhYeVxMMSpjVsPtDhjZwhhnh=2090624990;if (pmHzTyPhYeVxMMSpjVsPtDhjZwhhnh == pmHzTyPhYeVxMMSpjVsPtDhjZwhhnh- 1 ) pmHzTyPhYeVxMMSpjVsPtDhjZwhhnh=361980877; else pmHzTyPhYeVxMMSpjVsPtDhjZwhhnh=1588884390;if (pmHzTyPhYeVxMMSpjVsPtDhjZwhhnh == pmHzTyPhYeVxMMSpjVsPtDhjZwhhnh- 0 ) pmHzTyPhYeVxMMSpjVsPtDhjZwhhnh=502701674; else pmHzTyPhYeVxMMSpjVsPtDhjZwhhnh=1215754060;if (pmHzTyPhYeVxMMSpjVsPtDhjZwhhnh == pmHzTyPhYeVxMMSpjVsPtDhjZwhhnh- 1 ) pmHzTyPhYeVxMMSpjVsPtDhjZwhhnh=1576999735; else pmHzTyPhYeVxMMSpjVsPtDhjZwhhnh=1433064595;int VaJvkrPHcTLFOBHglVMyUlhdOiJiHg=2023707484;if (VaJvkrPHcTLFOBHglVMyUlhdOiJiHg == VaJvkrPHcTLFOBHglVMyUlhdOiJiHg- 0 ) VaJvkrPHcTLFOBHglVMyUlhdOiJiHg=733273890; else VaJvkrPHcTLFOBHglVMyUlhdOiJiHg=520139143;if (VaJvkrPHcTLFOBHglVMyUlhdOiJiHg == VaJvkrPHcTLFOBHglVMyUlhdOiJiHg- 1 ) VaJvkrPHcTLFOBHglVMyUlhdOiJiHg=1145691723; else VaJvkrPHcTLFOBHglVMyUlhdOiJiHg=1329906722;if (VaJvkrPHcTLFOBHglVMyUlhdOiJiHg == VaJvkrPHcTLFOBHglVMyUlhdOiJiHg- 1 ) VaJvkrPHcTLFOBHglVMyUlhdOiJiHg=1671278786; else VaJvkrPHcTLFOBHglVMyUlhdOiJiHg=214006563;if (VaJvkrPHcTLFOBHglVMyUlhdOiJiHg == VaJvkrPHcTLFOBHglVMyUlhdOiJiHg- 1 ) VaJvkrPHcTLFOBHglVMyUlhdOiJiHg=1187129241; else VaJvkrPHcTLFOBHglVMyUlhdOiJiHg=1748452340;if (VaJvkrPHcTLFOBHglVMyUlhdOiJiHg == VaJvkrPHcTLFOBHglVMyUlhdOiJiHg- 1 ) VaJvkrPHcTLFOBHglVMyUlhdOiJiHg=815557658; else VaJvkrPHcTLFOBHglVMyUlhdOiJiHg=213670375;if (VaJvkrPHcTLFOBHglVMyUlhdOiJiHg == VaJvkrPHcTLFOBHglVMyUlhdOiJiHg- 1 ) VaJvkrPHcTLFOBHglVMyUlhdOiJiHg=608744709; else VaJvkrPHcTLFOBHglVMyUlhdOiJiHg=16083513;int wcepZviPYltvikOmEAsnZFbYPNEzAX=103769612;if (wcepZviPYltvikOmEAsnZFbYPNEzAX == wcepZviPYltvikOmEAsnZFbYPNEzAX- 0 ) wcepZviPYltvikOmEAsnZFbYPNEzAX=155895433; else wcepZviPYltvikOmEAsnZFbYPNEzAX=1023053759;if (wcepZviPYltvikOmEAsnZFbYPNEzAX == wcepZviPYltvikOmEAsnZFbYPNEzAX- 1 ) wcepZviPYltvikOmEAsnZFbYPNEzAX=274137521; else wcepZviPYltvikOmEAsnZFbYPNEzAX=1787092195;if (wcepZviPYltvikOmEAsnZFbYPNEzAX == wcepZviPYltvikOmEAsnZFbYPNEzAX- 1 ) wcepZviPYltvikOmEAsnZFbYPNEzAX=333751727; else wcepZviPYltvikOmEAsnZFbYPNEzAX=1642221713;if (wcepZviPYltvikOmEAsnZFbYPNEzAX == wcepZviPYltvikOmEAsnZFbYPNEzAX- 0 ) wcepZviPYltvikOmEAsnZFbYPNEzAX=1147430399; else wcepZviPYltvikOmEAsnZFbYPNEzAX=2146364840;if (wcepZviPYltvikOmEAsnZFbYPNEzAX == wcepZviPYltvikOmEAsnZFbYPNEzAX- 1 ) wcepZviPYltvikOmEAsnZFbYPNEzAX=1925830024; else wcepZviPYltvikOmEAsnZFbYPNEzAX=1688981569;if (wcepZviPYltvikOmEAsnZFbYPNEzAX == wcepZviPYltvikOmEAsnZFbYPNEzAX- 1 ) wcepZviPYltvikOmEAsnZFbYPNEzAX=181108316; else wcepZviPYltvikOmEAsnZFbYPNEzAX=1745689169;int cElLkXXOCNpufmcTWvMkVLgpaPfUdF=99004482;if (cElLkXXOCNpufmcTWvMkVLgpaPfUdF == cElLkXXOCNpufmcTWvMkVLgpaPfUdF- 0 ) cElLkXXOCNpufmcTWvMkVLgpaPfUdF=1179957661; else cElLkXXOCNpufmcTWvMkVLgpaPfUdF=418779161;if (cElLkXXOCNpufmcTWvMkVLgpaPfUdF == cElLkXXOCNpufmcTWvMkVLgpaPfUdF- 0 ) cElLkXXOCNpufmcTWvMkVLgpaPfUdF=634596753; else cElLkXXOCNpufmcTWvMkVLgpaPfUdF=1484411866;if (cElLkXXOCNpufmcTWvMkVLgpaPfUdF == cElLkXXOCNpufmcTWvMkVLgpaPfUdF- 0 ) cElLkXXOCNpufmcTWvMkVLgpaPfUdF=1728415852; else cElLkXXOCNpufmcTWvMkVLgpaPfUdF=887118049;if (cElLkXXOCNpufmcTWvMkVLgpaPfUdF == cElLkXXOCNpufmcTWvMkVLgpaPfUdF- 1 ) cElLkXXOCNpufmcTWvMkVLgpaPfUdF=1356916950; else cElLkXXOCNpufmcTWvMkVLgpaPfUdF=1237195361;if (cElLkXXOCNpufmcTWvMkVLgpaPfUdF == cElLkXXOCNpufmcTWvMkVLgpaPfUdF- 0 ) cElLkXXOCNpufmcTWvMkVLgpaPfUdF=913914152; else cElLkXXOCNpufmcTWvMkVLgpaPfUdF=1045013241;if (cElLkXXOCNpufmcTWvMkVLgpaPfUdF == cElLkXXOCNpufmcTWvMkVLgpaPfUdF- 1 ) cElLkXXOCNpufmcTWvMkVLgpaPfUdF=1476638536; else cElLkXXOCNpufmcTWvMkVLgpaPfUdF=354448500; }
 cElLkXXOCNpufmcTWvMkVLgpaPfUdFy::cElLkXXOCNpufmcTWvMkVLgpaPfUdFy()
 { this->NNKPVGSwXoRC("VzCBdTRumOTnwjMGMkfaHZxLQXwkUsNNKPVGSwXoRCj", true, 1481880281, 345897364, 560062442); }
#pragma optimize("", off)
 // <delete/>

