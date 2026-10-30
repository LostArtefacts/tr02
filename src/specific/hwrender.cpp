#include "include.h"
#include "global.h"
#include "hwrender.h"
#include "winmain.h"

#define MAX_TLVERTICES 8192

extern "C" D3DTLVERTEX VertexBuffer[MAX_TLVERTICES];
extern "C" int LanTextureHandle[32];
extern "C" bool ZEnableState;
extern "C" bool ZWriteEnableState;
extern "C" LPDIRECTDRAWSURFACE ZBufferSurface;
extern "C" DWORD HWR_PageHandles[32];
extern "C" DWORD GetTexturePageHandle(int nHandle);
extern "C" void SafeFreeTexturePage(int nHandle);
extern "C" void SafeFreePalette(int nHandle);
extern "C" int PaletteIndex;
extern "C" int CreateTexturePalette(uint8 *pPalette);
extern "C" int AddTexturePage8(int nWidth, int nHeight, uint8 *pTexture, int nPalette);
extern "C" int AddTexturePage16(int nWidth, int nHeight, uint8 *pTexture);
extern "C" bool ColorKeyState;
extern "C" bool TexturesAlphaChannel;
extern "C" D3DTEXTUREHANDLE CurrentTexSource;

void HWR_ResetColorKey()
{
    ColorKeyState = false;
    if (TexturesAlphaChannel)
        G_Direct3DDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, false);
    else
        G_Direct3DDevice->SetRenderState(D3DRENDERSTATE_COLORKEYENABLE, false);
}

void HWR_ResetZBuffer()
{
    ZWriteEnableState = false;
    ZEnableState = false;
    if (SavedAppSettings.ZBuffer)
    {
        if (ZBufferSurface)
            G_Direct3DDevice->SetRenderState(D3DRENDERSTATE_ZFUNC, D3DCMP_ALWAYS);
        else
            G_Direct3DDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, false);
        G_Direct3DDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, false);
    }
}

void HWR_TexSource(D3DTEXTUREHANDLE tSource)
{
    if (tSource != CurrentTexSource)
    {
        G_Direct3DDevice->SetRenderState(D3DRENDERSTATE_TEXTUREHANDLE, tSource);
        CurrentTexSource = tSource;
    }
}

void HWR_EnableColorKey(bool tEnable)
{
    if (tEnable != ColorKeyState)
    {
        if (TexturesAlphaChannel)
            G_Direct3DDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, tEnable);
        else
            G_Direct3DDevice->SetRenderState(D3DRENDERSTATE_COLORKEYENABLE, tEnable);
        ColorKeyState = tEnable;
    }
}

void HWR_EnableZBuffer(bool tWrite, bool tCompare)
{
    if (SavedAppSettings.ZBuffer)
    {
        if (tWrite != ZWriteEnableState)
        {
            G_Direct3DDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, tWrite);
            ZWriteEnableState = tWrite;
        }
        if (tCompare != ZEnableState)
        {
            if (ZBufferSurface)
                G_Direct3DDevice->SetRenderState(D3DRENDERSTATE_ZFUNC, tCompare ? D3DCMP_LESSEQUAL : D3DCMP_ALWAYS);
            else
                G_Direct3DDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, tCompare);
            ZEnableState = tCompare;
        }
    }
}

void HWR_BeginScene()
{
    HWR_GetAllTextureHandles();
    DX_DoFlipWait();
    G_Direct3DDevice->BeginScene();
}

void HWR_LoadTexturePages(int nPages, uint8 *pImage, uint8 *pPalette)
{
    HWR_FreeTexturePages();

    int nPage, nHandle;
    if (pPalette)
        PaletteIndex = CreateTexturePalette(pPalette);

    for (nPage = 0; nPage < nPages; ++nPage)
    {
        if (pPalette)
        {
            nHandle = AddTexturePage8(256, 256, pImage, PaletteIndex);
            pImage += 0x10000;
        }
        else
        {
            nHandle = AddTexturePage16(256, 256, pImage);
            pImage += 0x20000;
        }

        if (nHandle >= 0)
            LanTextureHandle[nPage] = nHandle;
        else
            LanTextureHandle[nPage] = -1;
    }

    HWR_GetAllTextureHandles();
}

void HWR_FreeTexturePages()
{
    for (int nPage = 0; nPage < ArraySize(LanTextureHandle); ++nPage)
    {
        int nHandle = LanTextureHandle[nPage];
        if (nHandle >= 0)
        {
            SafeFreeTexturePage(nHandle);
            LanTextureHandle[nPage] = -1;
        }
        HWR_PageHandles[nPage] = 0;
    }
    if (PaletteIndex >= 0)
        SafeFreePalette(PaletteIndex);
}

void HWR_GetAllTextureHandles()
{
    for (int nTPage = 0; nTPage < ArraySize(LanTextureHandle); ++nTPage)
    {
        int nHandle = LanTextureHandle[nTPage];
        if (nHandle >= 0)
            HWR_PageHandles[nTPage] = GetTexturePageHandle(nHandle);
        else
            HWR_PageHandles[nTPage] = 0;
    }
}

bool HWR_Init()
{
    Zero(VertexBuffer);
    for (int nTexture = 0; nTexture < ArraySize(LanTextureHandle); ++nTexture)
        LanTextureHandle[nTexture] = -1;

    return true;
}
