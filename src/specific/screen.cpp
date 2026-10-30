#include "include.h"
#include "global.h"

typedef struct {
    LPTSTR lpString;
    bool isPresented;
} STRING_FLAGGED;

typedef struct {
    LPGUID lpAdapterGuid;
    GUID adapterGuid;
    STRING_FLAGGED driverDescription;
    STRING_FLAGGED driverName;
    DDCAPS driverCaps;
    DDCAPS helCaps;
    GUID deviceGuid;
    D3DDEVICEDESC D3DHWDeviceDesc;
} DISPLAY_ADAPTER;

extern "C" DISPLAY_ADAPTER CurrentDisplayAdapter;

#pragma pack(push, 1)
typedef struct {
    void *PreferredDisplayAdapter;
    void *PreferredSoundAdapter;
    void *PreferredJoystick;
    void *VideoMode;
    int RenderMode;
    int WindowWidth;
    int WindowHeight;
    int AspectMode;
    bool PerspectiveCorrect;
    bool Dither;
    bool ZBuffer;
    bool BilinearFiltering;
    bool TripleBuffering;
    bool FullScreen;
    bool SoundEnabled;
    bool LaraMic;
    bool JoystickEnabled;
    bool Disable16BitTextures;
    bool DontSortPrimitives;
    bool FlipBroken;
    bool DisableFMV;
    int TexelAdjustMode;
    int NearestAdjustment;
    int LinearAdjustment;
} APP_SETTINGS;
#pragma pack(pop)

extern "C" {
extern APP_SETTINGS SavedAppSettings;
extern LPDIRECTDRAWSURFACE3 PrimaryBufferSurface;
}

enum InitResult {
    ERR_OK,
    ERR_PreferredAdapterNotFound,
    ERR_CantCreateWindow,
    ERR_CantCreateDirectDraw,
    ERR_CantInitRenderer,
    ERR_CantCreateDirectInput,
    ERR_CantCreateKeyboardDevice,
    ERR_CantSetKBCooperativeLevel,
    ERR_CantSetKBDataFormat,
    ERR_CantAcquireKeyboard,
    ERR_CantSetDSCooperativeLevel,
    ERR_DD_SetExclusiveMode,
    ERR_DD_ClearExclusiveMode,
    ERR_SetDisplayMode,
    ERR_CreateScreenBuffers,
    ERR_GetBackBuffer,
    ERR_CreatePalette,
    ERR_SetPalette,
    ERR_CreatePrimarySurface,
    ERR_CreateBackBuffer,
    ERR_CreateClipper,
    ERR_SetClipperHWnd,
    ERR_SetClipper,
    ERR_CreateZBuffer,
    ERR_AttachZBuffer,
    ERR_CreateRenderBuffer,
    ERR_CreatePictureBuffer,
    ERR_D3D_Create,
    ERR_CreateDevice,
    ERR_CreateViewport,
    ERR_AddViewport,
    ERR_SetViewport2,
    ERR_SetCurrentViewport,
    ERR_ClearRenderBuffer,
    ERR_UpdateRenderInfo,
    ERR_GetThirdBuffer,
    ERR_GoFullScreen,
    ERR_GoWindowed,
    ERR_WrongBitDepth,
    ERR_GetPixelFormat,
};

inline bool DX_TRY(HRESULT hr)
{
    if (SUCCEEDED(hr))
        return false;
    return true;
}

#define RM_Software 1
#define RM_Hardware 2

extern "C" {
extern HWND HGameWindow;
extern bool GameVid_IsVga;
extern int GameVidBufWidth;
extern int GameVidBufHeight;
extern LPDIRECTDRAW2 DDraw;
extern LPDIRECTDRAWCLIPPER DDrawClipper;
extern LPDIRECTDRAWSURFACE3 BackBufferSurface;
extern LPDIRECTDRAWSURFACE3 PictureBufferSurface;
extern LPDIRECTDRAWSURFACE3 RenderBufferSurface;
extern LPDIRECTDRAWSURFACE3 ZBufferSurface;
extern LPDIRECTDRAWPALETTE DDrawPalette;
extern PALETTEENTRY WinVidPalette[256];
extern bool GameVid_IsWindowedVga;
HRESULT DDrawSurfaceCreate(LPDDSURFACEDESC dsp, LPDIRECTDRAWSURFACE3 *surface);
bool WinVidClearBuffer(LPDIRECTDRAWSURFACE3 surface, LPRECT rect, DWORD fillColor);
DWORD GetZBufferDepth(void);
}

extern "C" void CreatePrimarySurface(void)
{
    DDSURFACEDESC dsp;

    if ((GameVid_IsVga && SavedAppSettings.RenderMode == RM_Hardware) || (!GameVid_IsVga && SavedAppSettings.RenderMode == RM_Software))
        throw ERR_WrongBitDepth;
    memset(&dsp, 0, sizeof(dsp));
    dsp.dwSize = sizeof(dsp);
    dsp.dwFlags = DDSD_CAPS;
    dsp.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
    if (DX_TRY(DDrawSurfaceCreate(&dsp, &PrimaryBufferSurface)))
        throw ERR_CreatePrimarySurface;
}

extern "C" void CreateBackBuffer(void)
{
    DDSURFACEDESC dsp;

    memset(&dsp, 0, sizeof(dsp));
    dsp.dwSize = sizeof(dsp);
    dsp.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
    dsp.dwWidth = GameVidBufWidth;
    dsp.dwHeight = GameVidBufHeight;
    dsp.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_3DDEVICE;
    switch (SavedAppSettings.RenderMode) {
    case RM_Hardware:
        dsp.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_VIDEOMEMORY | DDSCAPS_3DDEVICE;
        break;
    }
    if (DX_TRY(DDrawSurfaceCreate(&dsp, &BackBufferSurface)))
        throw ERR_CreateBackBuffer;
    WinVidClearBuffer(BackBufferSurface, NULL, 0);
}

extern "C" void CreateClipper(void)
{
    if (DX_TRY(DDraw->CreateClipper(0, &DDrawClipper, NULL)))
        throw ERR_CreateClipper;
    if (DX_TRY(DDrawClipper->SetHWnd(0, HGameWindow)))
        throw ERR_SetClipperHWnd;
    if (DX_TRY(PrimaryBufferSurface->SetClipper(DDrawClipper)))
        throw ERR_SetClipper;
}

extern "C" void CreateWindowPalette(void)
{
    DWORD flags = DDPCAPS_8BIT;
    int i;

    memset(WinVidPalette, 0, sizeof(WinVidPalette));
    if (GameVid_IsWindowedVga) {
        for (i = 0; i < 10; i++) {
            WinVidPalette[i].peFlags = WinVidPalette[i + 246].peFlags = PC_EXPLICIT;
            WinVidPalette[i].peRed = WinVidPalette[i + 246].peRed = i;
        }
        for (i = 10; i < 246; i++)
            WinVidPalette[i].peFlags = PC_NOCOLLAPSE | PC_RESERVED;
    } else {
        for (i = 0; i < 256; i++)
            WinVidPalette[i].peFlags = PC_RESERVED;
        flags = DDPCAPS_8BIT | DDPCAPS_ALLOW256;
    }
    if (DX_TRY(DDraw->CreatePalette(flags, WinVidPalette, &DDrawPalette, NULL)))
        throw ERR_CreatePalette;
    if (DX_TRY(PrimaryBufferSurface->SetPalette(DDrawPalette)))
        throw ERR_SetPalette;
}

extern "C" void CreateZBuffer(void)
{
    DDSURFACEDESC dsp;

    if (!(CurrentDisplayAdapter.D3DHWDeviceDesc.dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_ZBUFFERLESSHSR)) {
        memset(&dsp, 0, sizeof(dsp));
        dsp.dwSize = sizeof(dsp);
        dsp.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_ZBUFFERBITDEPTH;
        dsp.ddsCaps.dwCaps = DDSCAPS_ZBUFFER | DDSCAPS_VIDEOMEMORY;
        dsp.dwWidth = GameVidBufWidth;
        dsp.dwHeight = GameVidBufHeight;
        dsp.dwZBufferBitDepth = GetZBufferDepth();
        if (DX_TRY(DDrawSurfaceCreate(&dsp, &ZBufferSurface)))
            throw ERR_CreateZBuffer;
        if (DX_TRY(BackBufferSurface->AddAttachedSurface(ZBufferSurface)))
            throw ERR_AttachZBuffer;
    }
}

extern "C" DWORD GetZBufferDepth(void)
{
    DWORD bitDepth = CurrentDisplayAdapter.D3DHWDeviceDesc.dwDeviceZBufferBitDepth;

    if (bitDepth & DDBD_16)
        return 16;
    if (bitDepth & DDBD_24)
        return 24;
    if (bitDepth & DDBD_32)
        return 32;
    return 8;
}

extern "C" void CreateRenderBuffer(void)
{
    DDSURFACEDESC dsp;

    memset(&dsp, 0, sizeof(dsp));
    dsp.dwSize = sizeof(dsp);
    dsp.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
    dsp.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
    dsp.dwWidth = GameVidBufWidth;
    dsp.dwHeight = GameVidBufHeight;
    if (DX_TRY(DDrawSurfaceCreate(&dsp, &RenderBufferSurface)))
        throw ERR_CreateRenderBuffer;
    if (!WinVidClearBuffer(RenderBufferSurface, NULL, 0))
        throw ERR_ClearRenderBuffer;
}

extern "C" void CreatePictureBuffer(void)
{
    DDSURFACEDESC dsp;

    memset(&dsp, 0, sizeof(dsp));
    dsp.dwSize = sizeof(dsp);
    dsp.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
    dsp.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
    dsp.dwWidth = 640;
    dsp.dwHeight = 480;
    if (DX_TRY(DDrawSurfaceCreate(&dsp, &PictureBufferSurface)))
        throw ERR_CreatePictureBuffer;
}

extern "C" void DX_DoFlipWait(void)
{
    if (SavedAppSettings.FlipBroken && SavedAppSettings.FullScreen)
    {
        while (PrimaryBufferSurface->GetFlipStatus(DDGFS_ISFLIPDONE) == DDERR_WASSTILLDRAWING)
            ;
    }
}

extern "C" bool RenderInit(void)
{
    return true;
}

extern "C" {
extern LPCTSTR ErrorStringTable[];
}

extern "C" LPCTSTR DecodeErrorMessage(DWORD errorCode)
{
    return ErrorStringTable[errorCode];
}

#include <stdio.h>

extern "C" {
extern int GameWindowWidth, GameWindowHeight;
extern int GameVidWidth, GameVidHeight;
extern char ModeInfoFormat[];
void DisplayModeInfo(char *string);
void GameApplySettings(APP_SETTINGS *newSettings);
}

extern "C" void UpdateGameResolution(void)
{
    APP_SETTINGS newSettings;
    char modeString[64];

    newSettings = SavedAppSettings;
    newSettings.WindowWidth = GameWindowWidth;
    newSettings.WindowHeight = GameWindowHeight;
    GameApplySettings(&newSettings);
    sprintf(modeString, ModeInfoFormat, GameVidWidth, GameVidHeight);
    DisplayModeInfo(modeString);
}

extern "C" {
extern LPDIRECTDRAWSURFACE3 ThirdBufferSurface;
extern bool NeedToReloadTextures;
void Direct3DRelease(void);
void CleanupTextures(void);
}
void HWR_FreeTexturePages(void);

extern "C" void RenderFinish(bool needToClearTextures)
{
    if (SavedAppSettings.RenderMode == RM_Hardware) {
        if (needToClearTextures) {
            HWR_FreeTexturePages();
            CleanupTextures();
        }
        Direct3DRelease();
        if (ZBufferSurface) {
            ZBufferSurface->Release();
            ZBufferSurface = NULL;
        }
    } else {
        if (needToClearTextures && PictureBufferSurface) {
            PictureBufferSurface->Release();
            PictureBufferSurface = NULL;
        }
        if (RenderBufferSurface) {
            RenderBufferSurface->Release();
            RenderBufferSurface = NULL;
        }
    }
    if (DDrawPalette) {
        DDrawPalette->Release();
        DDrawPalette = NULL;
    }
    if (DDrawClipper) {
        DDrawClipper->Release();
        DDrawClipper = NULL;
    }
    if (ThirdBufferSurface) {
        ThirdBufferSurface->Release();
        ThirdBufferSurface = NULL;
    }
    if (BackBufferSurface) {
        BackBufferSurface->Release();
        BackBufferSurface = NULL;
    }
    if (PrimaryBufferSurface) {
        PrimaryBufferSurface->Release();
        PrimaryBufferSurface = NULL;
    }
    if (needToClearTextures)
        NeedToReloadTextures = false;
}
