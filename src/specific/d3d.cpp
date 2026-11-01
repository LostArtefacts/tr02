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

extern "C" bool D3DIsSupported(LPD3DDEVICEDESC desc, bool hardware)
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

typedef struct {
    LPTSTR lpString;
    bool isPresented;
} STRING_FLAGGED;

typedef struct {
    int width;
    int height;
    int bpp;
    int vga;
} DISPLAY_MODE;

typedef struct {
    void *head;
    void *tail;
    DWORD dwCount;
} DISPLAY_MODE_LIST;

typedef struct {
    LPGUID lpAdapterGuid;
    GUID adapterGuid;
    STRING_FLAGGED driverDescription;
    STRING_FLAGGED driverName;
    DDCAPS driverCaps;
    DDCAPS helCaps;
    GUID deviceGuid;
    D3DDEVICEDESC D3DHWDeviceDesc;
    DISPLAY_MODE_LIST hwDispModeList;
    DISPLAY_MODE_LIST swDispModeList;
    DISPLAY_MODE vgaMode1;
    DISPLAY_MODE vgaMode2;
    DWORD screenWidth;
    bool hwRenderSupported;
    bool swWindowedSupported;
    bool hwWindowedSupported;
    bool isVgaMode1Presented;
    bool isVgaMode2Presented;
    bool perspectiveCorrectSupported;
    bool ditherSupported;
    bool zBufferSupported;
    bool linearFilterSupported;
    bool shadeRestricted;
} DISPLAY_ADAPTER;

extern "C" HRESULT WINAPI Enum3DDevicesCallback(GUID *lpGuid, LPTSTR lpDeviceDescription, LPTSTR lpDeviceName, LPD3DDEVICEDESC lpD3DHWDeviceDesc, LPD3DDEVICEDESC lpD3DHELDeviceDesc, LPVOID lpContext)
{
    DISPLAY_ADAPTER *adapter = (DISPLAY_ADAPTER *)lpContext;

    if (lpD3DHWDeviceDesc && D3DIsSupported(lpD3DHWDeviceDesc, true)) {
        adapter->hwRenderSupported = true;
        adapter->deviceGuid = *lpGuid;
        adapter->D3DHWDeviceDesc = *lpD3DHWDeviceDesc;
        adapter->perspectiveCorrectSupported = (lpD3DHWDeviceDesc->dpcTriCaps.dwTextureCaps & D3DPTEXTURECAPS_PERSPECTIVE) ? true : false;
        adapter->ditherSupported = (lpD3DHWDeviceDesc->dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_DITHER) ? true : false;
        adapter->zBufferSupported = lpD3DHWDeviceDesc->dwDeviceZBufferBitDepth != 0;
        adapter->linearFilterSupported = (lpD3DHWDeviceDesc->dpcTriCaps.dwTextureFilterCaps & D3DPTFILTERCAPS_LINEAR) ? true : false;
        adapter->shadeRestricted = !(lpD3DHWDeviceDesc->dpcTriCaps.dwShadeCaps & (D3DPSHADECAPS_ALPHAGOURAUDBLEND | D3DPSHADECAPS_ALPHAFLATBLEND));
    }
    return D3DENUMRET_OK;
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

enum InitResult {
    ERR_D3D_Create = 27,
    ERR_CreateDevice,
    ERR_CreateViewport,
    ERR_AddViewport,
    ERR_SetViewport2,
    ERR_SetCurrentViewport,
};

extern "C" void D3DDeviceCreate(LPDIRECTDRAWSURFACE3 lpBackBuffer)
{
    D3DMATERIAL matData;
    D3DMATERIALHANDLE matHandle;

    if (!D3DCreate())
        throw ERR_D3D_Create;
    if (DX_TRY(D3D->CreateDevice(IID_IDirect3DHALDevice, (LPDIRECTDRAWSURFACE)lpBackBuffer, &G_Direct3DDevice)))
        throw ERR_CreateDevice;
    if (DX_TRY(D3D->CreateViewport(&D3DView, NULL)))
        throw ERR_CreateViewport;
    if (DX_TRY(G_Direct3DDevice->AddViewport(D3DView)))
        throw ERR_AddViewport;
    if (!D3DSetViewport())
        throw ERR_SetViewport2;
    if (DX_TRY(D3D->CreateMaterial(&D3DMaterial, NULL)))
        throw ERR_CreateViewport;
    memset(&matData, 0, sizeof(matData));
    matData.dwSize = sizeof(matData);
    if (DX_TRY(D3DMaterial->SetMaterial(&matData)))
        throw ERR_CreateViewport;
    if (DX_TRY(D3DMaterial->GetHandle(G_Direct3DDevice, &matHandle)))
        throw ERR_CreateViewport;
    if (DX_TRY(D3DView->SetBackground(matHandle)))
        throw ERR_CreateViewport;
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

#ifdef TR2_EXE
LPDIRECT3D2 D3D;
LPDIRECT3DVIEWPORT2 D3DView;
LPDIRECT3DMATERIAL2 D3DMaterial;
#endif
