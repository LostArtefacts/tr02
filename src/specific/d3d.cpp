#include "include.h"
#include "global.h"

extern "C" {
extern LPDIRECTDRAW2 DDraw;
extern LPDIRECT3D2 D3D;
bool D3DCreate(void);
void D3DRelease(void);
HRESULT WINAPI Enum3DDevicesCallback(GUID *lpGuid, LPTSTR lpDeviceDescription, LPTSTR lpDeviceName, LPD3DDEVICEDESC lpD3DHWDeviceDesc, LPD3DDEVICEDESC lpD3DHELDeviceDesc, LPVOID lpContext);
}

extern "C" void Enumerate3DDevices(void *adapter)
{
    if (D3DCreate())
    {
        D3D->EnumDevices(Enum3DDevicesCallback, adapter);
        D3DRelease();
    }
}

inline bool DX_TRY(HRESULT hr)
{
    if (SUCCEEDED(hr))
        return false;
    return true;
}

extern "C" bool D3DCreate(void)
{
    if (DX_TRY(DDraw->QueryInterface(IID_IDirect3D2, (LPVOID *)&D3D)))
        return false;
    return true;
}

extern "C" void D3DRelease(void)
{
    if (D3D)
    {
        D3D->Release();
        D3D = NULL;
    }
}

extern "C" bool D3DIsSupported(LPD3DDEVICEDESC desc)
{
    if (!(desc->dwFlags & D3DDD_COLORMODEL))
        return false;
    if (!(desc->dcmColorModel & D3DCOLOR_RGB))
        return false;
    if (!(desc->dwFlags & D3DDD_TRICAPS))
        return false;
    if (!(desc->dpcTriCaps.dwShadeCaps & D3DPSHADECAPS_COLORGOURAUDRGB))
        return false;
    if (!(desc->dpcTriCaps.dwTextureBlendCaps & D3DPTBLENDCAPS_MODULATE))
        return false;
    return true;
}

extern "C" {
extern LPDIRECT3DVIEWPORT2 D3DView;
extern LPDIRECT3DMATERIAL2 D3DMaterial;
}

extern "C" {
extern int GameVidWidth;
extern int GameVidHeight;
}

extern "C" bool D3DSetViewport(void)
{
    D3DVIEWPORT2 viewPort;

    memset(&viewPort, 0, sizeof(viewPort));
    viewPort.dwSize = sizeof(viewPort);
    viewPort.dwX = 0;
    viewPort.dwY = 0;
    viewPort.dwWidth = GameVidWidth;
    viewPort.dwHeight = GameVidHeight;
    viewPort.dvClipX = 0.0f;
    viewPort.dvClipY = 0.0f;
    viewPort.dvClipWidth = (float)GameVidWidth;
    viewPort.dvClipHeight = (float)GameVidHeight;
    viewPort.dvMinZ = 0.0f;
    viewPort.dvMaxZ = 1.0f;
    if (DX_TRY(D3DView->SetViewport2(&viewPort))) {
        D3DView->GetViewport2(&viewPort);
        return false;
    }
    if (DX_TRY(G_Direct3DDevice->SetCurrentViewport(D3DView)))
        return false;
    return true;
}

extern "C" void Direct3DRelease(void)
{
    if (D3DMaterial)
    {
        D3DMaterial->Release();
        D3DMaterial = NULL;
    }
    if (D3DView)
    {
        D3DView->Release();
        D3DView = NULL;
    }
    if (G_Direct3DDevice)
    {
        G_Direct3DDevice->Release();
        G_Direct3DDevice = NULL;
    }
    D3DRelease();
}

extern "C" BOOL Direct3DInit(void)
{
    return TRUE;
}
