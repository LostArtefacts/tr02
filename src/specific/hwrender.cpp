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
extern "C" bool ColorKeyState;
extern "C" bool TexturesAlphaChannel;

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

bool HWR_Init()
{
    Zero(VertexBuffer);
    for (int nTexture = 0; nTexture < ArraySize(LanTextureHandle); ++nTexture)
        LanTextureHandle[nTexture] = -1;

    return true;
}
