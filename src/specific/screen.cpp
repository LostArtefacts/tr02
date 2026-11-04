// cflags: /GX
#include "include.h"
#include "global.h"
#include <stdio.h>

struct DISPLAY_MODE {
    int width;
    int height;
    int bpp;
    int vga;
    DISPLAY_MODE() {}
    bool CompareVideoModes(DISPLAY_MODE *mode);
};

typedef struct DISPLAY_MODE_NODE {
    struct DISPLAY_MODE_NODE *next;
    struct DISPLAY_MODE_NODE *previous;
    DISPLAY_MODE body;
} DISPLAY_MODE_NODE;

typedef struct {
    DISPLAY_MODE_NODE *head;
    DISPLAY_MODE_NODE *tail;
    DWORD dwCount;
} DISPLAY_MODE_LIST;

#pragma pack(push, 1)
typedef struct {
    void *next;
    void *previous;
    BYTE adapterBody[0x3d2];
    DISPLAY_MODE_LIST hwDispModeList;
    DISPLAY_MODE_LIST swDispModeList;
} DISPLAY_ADAPTER_NODE;
#pragma pack(pop)

#ifdef TR2_EXE
extern "C" {
DISPLAY_MODE Var4d92c8;
}
#endif

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
HRESULT DDrawSurfaceRestoreLost(LPDIRECTDRAWSURFACE3 surface1, LPDIRECTDRAWSURFACE3 surface2, bool blank);
DWORD GetZBufferDepth(void);
}

extern "C" LPDIRECTDRAWSURFACE3 ThirdBufferSurface;

extern "C" void CreateScreenBuffers(void)
{
    DDSCAPS caps;
    DDSURFACEDESC dsp;

    memset(&dsp, 0, sizeof(dsp));
    dsp.dwSize = sizeof(dsp);
    dsp.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT;
    dsp.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX;
    if (SavedAppSettings.RenderMode != RM_Software)
        dsp.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_FLIP | DDSCAPS_COMPLEX | DDSCAPS_3DDEVICE;
    dsp.dwBackBufferCount = SavedAppSettings.TripleBuffering + 1;
    if (DX_TRY(DDrawSurfaceCreate(&dsp, &PrimaryBufferSurface)))
        throw ERR_CreateScreenBuffers;
    WinVidClearBuffer(PrimaryBufferSurface, NULL, 0);
    caps.dwCaps = DDSCAPS_BACKBUFFER;
    if (DX_TRY(PrimaryBufferSurface->GetAttachedSurface(&caps, &BackBufferSurface)))
        throw ERR_GetBackBuffer;
    WinVidClearBuffer(BackBufferSurface, NULL, 0);
    if (SavedAppSettings.TripleBuffering) {
        caps.dwCaps = DDSCAPS_FLIP;
        if (DX_TRY(BackBufferSurface->GetAttachedSurface(&caps, &ThirdBufferSurface)))
            throw ERR_GetThirdBuffer;
        WinVidClearBuffer(ThirdBufferSurface, NULL, 0);
    }
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

extern "C" {
extern sint16 phd_winxmin;
extern sint16 phd_winymin;
extern int phd_winwidth;
extern int phd_winheight;
extern int GameVidWidth;
extern int GameVidHeight;
extern int GameWindowPositionX;
extern int GameWindowWidth;
extern int GameWindowPositionY;
extern int GameWindowHeight;
extern LPDIRECT3DVIEWPORT2 D3DView;
}

extern "C" void ClearBuffers(DWORD flags, DWORD fillColor)
{
    RECT rect;
    DWORD d3dFlags;
    D3DRECT d3dRect;

    if (flags & 0x100) {
        rect.left = phd_winxmin;
        rect.top = phd_winymin;
        rect.right = phd_winxmin + phd_winwidth;
        rect.bottom = phd_winymin + phd_winheight;
    } else {
        rect.top = 0;
        rect.left = 0;
        rect.right = GameVidWidth;
        rect.bottom = GameVidHeight;
    }
    if (SavedAppSettings.RenderMode == RM_Hardware) {
        d3dFlags = 0;
        if (flags & 2)
            d3dFlags |= D3DCLEAR_TARGET;
        if (flags & 8)
            d3dFlags |= D3DCLEAR_ZBUFFER;
        if (d3dFlags) {
            d3dRect.x1 = rect.left;
            d3dRect.x2 = rect.right;
            d3dRect.y1 = rect.top;
            d3dRect.y2 = rect.bottom;
            D3DView->Clear(1, &d3dRect, d3dFlags);
        }
    } else if (flags & 2) {
        WinVidClearBuffer(BackBufferSurface, &rect, fillColor);
    }
    if (flags & 1)
        WinVidClearBuffer(PrimaryBufferSurface, &rect, fillColor);
    if (flags & 4)
        WinVidClearBuffer(ThirdBufferSurface, &rect, fillColor);
    if (flags & 0x10)
        WinVidClearBuffer(RenderBufferSurface, &rect, fillColor);
    if (flags & 0x20) {
        rect.top = 0;
        rect.left = 0;
        rect.right = 640;
        rect.bottom = 480;
        WinVidClearBuffer(PictureBufferSurface, &rect, fillColor);
    }
    if (flags & 0x40) {
        rect.left = GameWindowPositionX;
        rect.top = GameWindowPositionY;
        rect.right = GameWindowPositionX + GameWindowWidth;
        rect.bottom = GameWindowPositionY + GameWindowHeight;
        WinVidClearBuffer(PrimaryBufferSurface, &rect, fillColor);
    }
}

extern "C" {
extern int GameWindowWidth, GameWindowHeight;
extern int GameVidWidth, GameVidHeight;
}

extern "C" {
extern bool NeedToReloadTextures;
void Direct3DRelease(void);
void CleanupTextures(void);
}
void HWR_FreeTexturePages(void);

extern "C" {
extern bool IsGameToExit;
void S_ExitSystem(const char *message);
bool ApplySettings(APP_SETTINGS *newSettings);
}
void HWR_GetAllTextureHandles(void);

extern "C" bool RestoreLostBuffers(void)
{
    bool rebuild;

    if (!PrimaryBufferSurface)
        S_ExitSystem("Oops... no front buffer");
    rebuild = DX_TRY(DDrawSurfaceRestoreLost(PrimaryBufferSurface, NULL, SavedAppSettings.FullScreen));
    if ((SavedAppSettings.FullScreen || SavedAppSettings.RenderMode == RM_Hardware) && (rebuild || DX_TRY(DDrawSurfaceRestoreLost(BackBufferSurface, PrimaryBufferSurface, true))))
        rebuild = true;
    if (SavedAppSettings.TripleBuffering && (rebuild || DX_TRY(DDrawSurfaceRestoreLost(ThirdBufferSurface, PrimaryBufferSurface, true))))
        rebuild = true;
    if (SavedAppSettings.RenderMode == RM_Software && (rebuild || DX_TRY(DDrawSurfaceRestoreLost(RenderBufferSurface, NULL, false))))
        rebuild = true;
    if (ZBufferSurface && (rebuild || DX_TRY(DDrawSurfaceRestoreLost(ZBufferSurface, NULL, false))))
        rebuild = true;
    if (PictureBufferSurface && (rebuild || DX_TRY(DDrawSurfaceRestoreLost(PictureBufferSurface, NULL, false))))
        rebuild = true;
    if (rebuild && !IsGameToExit) {
        ApplySettings(&SavedAppSettings);
        if (SavedAppSettings.RenderMode == RM_Hardware)
            HWR_GetAllTextureHandles();
    }
    return rebuild;
}

extern "C" {
extern RECT GameVidRect;
extern int GameWindowPositionX;
extern int GameWindowPositionY;
bool WinVidSpinMessageLoop(bool needWait);
}

extern "C" bool DX_UpdateFrame(bool needRunMessageLoop, LPRECT rect)
{
    LPDIRECTDRAWSURFACE3 surface;
    RECT dstRect;

    if (!rect)
        rect = &GameVidRect;
    RestoreLostBuffers();
    if (SavedAppSettings.FullScreen) {
        if (SavedAppSettings.RenderMode == RM_Software)
            BackBufferSurface->Blt(rect, RenderBufferSurface, rect, DDBLT_WAIT, NULL);
        PrimaryBufferSurface->Flip(NULL, DDFLIP_WAIT);
    } else {
        dstRect.left = GameWindowPositionX + rect->left;
        dstRect.top = GameWindowPositionY + rect->top;
        dstRect.right = GameWindowPositionX + rect->right;
        dstRect.bottom = GameWindowPositionY + rect->bottom;
        surface = (SavedAppSettings.RenderMode == RM_Software) ? RenderBufferSurface : BackBufferSurface;
        PrimaryBufferSurface->Blt(&dstRect, surface, rect, DDBLT_WAIT, NULL);
    }
    if (needRunMessageLoop)
        return WinVidSpinMessageLoop(false);
    return true;
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

struct COLOR_BIT_MASKS {
    DWORD dwRBitMask;
    DWORD dwGBitMask;
    DWORD dwBBitMask;
    DWORD dwRGBAlphaBitMask;
    DWORD dwRBitDepth;
    DWORD dwGBitDepth;
    DWORD dwBBitDepth;
    DWORD dwRGBAlphaBitDepth;
    DWORD dwRBitOffset;
    DWORD dwGBitOffset;
    DWORD dwBBitOffset;
    DWORD dwRGBAlphaBitOffset;
    void WinVidGetColorBitMasks(LPDDPIXELFORMAT pixelFormat);
};

typedef struct {
    DDPIXELFORMAT pixelFormat;
    COLOR_BIT_MASKS colorBitMasks;
    DWORD bpp;
} TEXTURE_FORMAT;

enum {
    VGA_NoVga,
    VGA_256Color,
    VGA_ModeX,
    VGA_Standard,
};

extern "C" {
extern COLOR_BIT_MASKS ColorBitMasks;
extern TEXTURE_FORMAT TextureFormat;
extern RECT GameVidRect;
extern RECT GameVidBufRect;
extern int GameVidWidth, GameVidHeight;
extern int GameVidBPP;
extern bool GameVid_IsFullscreenVga;
extern bool IsWindowedVGA;
extern bool Is16bitTextures;
extern bool NeedToReloadTextures;
extern sint16 DumpWidth;
extern sint16 DumpHeight;
extern LPDIRECTDRAWSURFACE3 BackBufferSurface;
bool WinVidGoFullScreen(DISPLAY_MODE *dispMode);
bool WinVidGoWindowed(int width, int height, DISPLAY_MODE *dispMode);
void WinVidSetMinWindowSize(int width, int height);
int CalculateWindowWidth(int width, int height);
int CalculateWindowHeight(int width, int height);
void CreateScreenBuffers(void);
void CreatePrimarySurface(void);
void CreateBackBuffer(void);
void CreateClipper(void);
void D3DDeviceCreate(LPDIRECTDRAWSURFACE3 lpBackBuffer);
void EnumerateTextureFormats(void);
int S_ReloadLevelGraphics(int tPalDPQ, int tTPages);
bool ReloadTextures(bool reset);
}
void HWR_GetAllTextureHandles(void);

void RenderStart(bool isReset)
{
    int minWidth;
    int minHeight;
    DISPLAY_MODE dispMode;
    bool is16bitTextures;
    bool isVgaChanged;
    bool is16bitChanged;
    DDPIXELFORMAT pixelFormat;

    if (isReset)
        NeedToReloadTextures = false;
    if (SavedAppSettings.FullScreen) {
        if (!SavedAppSettings.VideoMode)
            throw ERR_GoFullScreen;
        dispMode = ((DISPLAY_MODE_NODE *)SavedAppSettings.VideoMode)->body;
        if (!WinVidGoFullScreen(&dispMode))
            throw ERR_GoFullScreen;
        CreateScreenBuffers();
        GameVidWidth = dispMode.width;
        GameVidHeight = dispMode.height;
        GameVidBPP = dispMode.bpp;
        GameVidBufWidth = dispMode.width;
        GameVidBufHeight = dispMode.height;
        GameVid_IsVga = dispMode.vga != VGA_NoVga;
        GameVid_IsWindowedVga = false;
        GameVid_IsFullscreenVga = dispMode.vga == VGA_Standard;
    } else {
        minWidth = 320;
        minHeight = CalculateWindowHeight(320, 200);
        if (minHeight < 200) {
            minWidth = CalculateWindowWidth(320, 200);
            minHeight = 200;
        }
        WinVidSetMinWindowSize(minWidth, minHeight);
        if (!WinVidGoWindowed(SavedAppSettings.WindowWidth, SavedAppSettings.WindowHeight, &dispMode))
            throw ERR_GoWindowed;
        GameVidWidth = dispMode.width;
        GameVidHeight = dispMode.height;
        GameVidBPP = dispMode.bpp;
        GameVidBufWidth = (dispMode.width + 31) & ~31;
        GameVidBufHeight = (dispMode.height + 31) & ~31;
        GameVid_IsVga = dispMode.vga != VGA_NoVga;
        GameVid_IsWindowedVga = dispMode.vga != VGA_NoVga;
        GameVid_IsFullscreenVga = false;
        CreatePrimarySurface();
        if (SavedAppSettings.RenderMode != RM_Software)
            CreateBackBuffer();
        CreateClipper();
    }
    memset(&pixelFormat, 0, sizeof(pixelFormat));
    pixelFormat.dwSize = sizeof(pixelFormat);
    if (FAILED(PrimaryBufferSurface->GetPixelFormat(&pixelFormat)))
        throw ERR_GetPixelFormat;
    ColorBitMasks.WinVidGetColorBitMasks(&pixelFormat);
    if (GameVid_IsVga)
        CreateWindowPalette();
    if (SavedAppSettings.RenderMode != RM_Software && SavedAppSettings.ZBuffer)
        CreateZBuffer();
    if (SavedAppSettings.RenderMode == RM_Software) {
        CreateRenderBuffer();
        if (!PictureBufferSurface)
            CreatePictureBuffer();
    } else {
        D3DDeviceCreate(BackBufferSurface);
        EnumerateTextureFormats();
    }
    if (NeedToReloadTextures) {
        is16bitTextures = (int)TextureFormat.bpp >= 16;
        isVgaChanged = GameVid_IsWindowedVga != IsWindowedVGA;
        is16bitChanged = is16bitTextures != Is16bitTextures;
        if (isVgaChanged || is16bitChanged) {
            S_ReloadLevelGraphics(isVgaChanged, is16bitChanged);
            IsWindowedVGA = GameVid_IsWindowedVga;
            Is16bitTextures = is16bitTextures;
        } else if (SavedAppSettings.RenderMode == RM_Hardware) {
            ReloadTextures(true);
            HWR_GetAllTextureHandles();
        }
    } else {
        IsWindowedVGA = GameVid_IsWindowedVga;
        Is16bitTextures = (int)TextureFormat.bpp >= 16;
    }
    GameVidBufRect.left = 0;
    GameVidBufRect.top = 0;
    GameVidBufRect.right = GameVidBufWidth;
    GameVidBufRect.bottom = GameVidBufHeight;
    GameVidRect.left = 0;
    GameVidRect.top = 0;
    GameVidRect.right = GameVidWidth;
    GameVidRect.bottom = GameVidHeight;
    DumpWidth = GameVidWidth;
    DumpHeight = GameVidHeight;
    setup_screen_size();
    NeedToReloadTextures = true;
}

void RenderFinish(bool needToClearTextures)
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

extern "C" {
extern APP_SETTINGS SavedAppSettings;
extern DISPLAY_ADAPTER_NODE *PrimaryDisplayAdapter;
}

void RenderStart(bool reset);
void RenderFinish(bool reset);
void S_ExitSystem(char *message);

extern "C" {
extern int GameVidWidth;
extern int GameVidHeight;
extern int GameVidBPP;
void S_InitialiseScreen(int type);
void DisplayModeInfo(char *modeString);
}

extern "C" bool ApplySettings(APP_SETTINGS *newSettings)
{
    DISPLAY_MODE_LIST *modeList;
    DISPLAY_MODE_NODE *mode;
    DISPLAY_MODE targetMode;
    char modeString[64];
    APP_SETTINGS oldSettings;

    RenderFinish(false);
    oldSettings = SavedAppSettings;
    if (newSettings != &SavedAppSettings) {
        SavedAppSettings = *newSettings;
    }
    *(bool *)&newSettings = true;
    try {
        RenderStart(false);
    } catch (InitResult error) {
        *(bool *)&newSettings = false;
        RenderFinish(false);
        SavedAppSettings = oldSettings;
        try {
            RenderStart(false);
        } catch (InitResult error) {
            RenderFinish(false);
            SavedAppSettings.TripleBuffering = false;
            SavedAppSettings.PreferredDisplayAdapter = PrimaryDisplayAdapter;
            SavedAppSettings.FullScreen = true;
            SavedAppSettings.RenderMode = 1;
            targetMode.width = 640;
            targetMode.height = 480;
            targetMode.bpp = 0;
            targetMode.vga = 0;
            modeList = &PrimaryDisplayAdapter->swDispModeList;
            mode = modeList->head;
            if (mode) {
                while (mode->body.CompareVideoModes(&targetMode)) {
                    mode = mode->next;
                    if (!mode) {
                        mode = modeList->tail;
                        break;
                    }
                }
            }
            SavedAppSettings.VideoMode = mode;
            try {
                RenderStart(false);
            } catch (InitResult error) {
                S_ExitSystem("Can't reinitialise renderer");
            }
        }
    }
    S_InitialiseScreen(-1);
    if (SavedAppSettings.RenderMode != oldSettings.RenderMode) {
        S_ReloadLevelGraphics(1, 1);
    } else if (SavedAppSettings.RenderMode == 1 && SavedAppSettings.FullScreen != oldSettings.FullScreen) {
        S_ReloadLevelGraphics(1, 0);
    }
    if (SavedAppSettings.FullScreen) {
        sprintf(modeString, "%dx%dx%d", GameVidWidth, GameVidHeight, GameVidBPP);
    } else {
        sprintf(modeString, "%dx%d", GameVidWidth, GameVidHeight);
    }
    DisplayModeInfo(modeString);
    return *(bool *)&newSettings;
}

extern "C" void FmvBackToGame(void)
{
    DISPLAY_MODE_LIST *modeList;
    DISPLAY_MODE_NODE *mode;
    DISPLAY_MODE targetMode;

    try {
        RenderStart(true);
    } catch (...) {
        RenderFinish(true);
        SavedAppSettings.TripleBuffering = false;
        SavedAppSettings.PreferredDisplayAdapter = PrimaryDisplayAdapter;
        SavedAppSettings.FullScreen = true;
        SavedAppSettings.RenderMode = 1;
        targetMode.width = 640;
        targetMode.height = 480;
        targetMode.bpp = 0;
        targetMode.vga = 0;
        modeList = &PrimaryDisplayAdapter->swDispModeList;
        mode = modeList->head;
        if (mode) {
            while (mode->body.CompareVideoModes(&targetMode)) {
                mode = mode->next;
                if (!mode) {
                    mode = modeList->tail;
                    break;
                }
            }
        }
        SavedAppSettings.VideoMode = mode;
        try {
            RenderStart(true);
        } catch (...) {
            S_ExitSystem("Can't reinitialise renderer");
        }
    }
}

extern "C" {
extern int GameVidBufWidth;
extern int GameVidBufHeight;
extern RECT GameVidRect;
extern bool WinVidNeedToResetBuffers;
bool WinVidGoWindowed(int width, int height, DISPLAY_MODE *dispMode);
void D3DSetViewport(void);
void setup_screen_size(void);
void HWR_InitState(void);
void S_AdjustTexelCoordinates(void);
}

extern "C" bool GameApplySettings(APP_SETTINGS *newSettings)
{
    bool needInitRenderState;
    bool needRestart;
    bool needAdjustTexel;
    bool needRebuildBuffers;
    bool result;
    DISPLAY_MODE dispMode;
    int dx;
    int dy;

    needRestart = false;
    needInitRenderState = false;
    needAdjustTexel = false;
    needRebuildBuffers = false;
    result = true;
    if (newSettings->PreferredDisplayAdapter != SavedAppSettings.PreferredDisplayAdapter
        || newSettings->PreferredSoundAdapter != SavedAppSettings.PreferredSoundAdapter
        || newSettings->PreferredJoystick != SavedAppSettings.PreferredJoystick) {
        return false;
    }
    if (newSettings->RenderMode != SavedAppSettings.RenderMode
        || newSettings->VideoMode != SavedAppSettings.VideoMode
        || newSettings->FullScreen != SavedAppSettings.FullScreen
        || newSettings->ZBuffer != SavedAppSettings.ZBuffer
        || newSettings->TripleBuffering != SavedAppSettings.TripleBuffering) {
        needRestart = true;
    }
    if (newSettings->PerspectiveCorrect != SavedAppSettings.PerspectiveCorrect
        || newSettings->Dither != SavedAppSettings.Dither
        || newSettings->BilinearFiltering != SavedAppSettings.BilinearFiltering) {
        needInitRenderState = true;
    }
    if (newSettings->BilinearFiltering != SavedAppSettings.BilinearFiltering
        || newSettings->RenderMode != SavedAppSettings.RenderMode) {
        needAdjustTexel = true;
    }
    if (!needRestart && !newSettings->FullScreen) {
        if (newSettings->WindowWidth != SavedAppSettings.WindowWidth
            || newSettings->WindowHeight != SavedAppSettings.WindowHeight) {
            if (!WinVidGoWindowed(newSettings->WindowWidth, newSettings->WindowHeight, &dispMode)) {
                return false;
            }
            newSettings->WindowWidth = dispMode.width;
            newSettings->WindowHeight = dispMode.height;
            if (dispMode.width != SavedAppSettings.WindowWidth
                || dispMode.height != SavedAppSettings.WindowHeight) {
                dx = dispMode.width - GameVidBufWidth;
                dy = dispMode.height - GameVidBufHeight;
                if (dx > 0 || dx < -64 || dy > 0 || dy < -64) {
                    needRebuildBuffers = true;
                } else {
                    SavedAppSettings.WindowWidth = dispMode.width;
                    SavedAppSettings.WindowHeight = newSettings->WindowHeight;
                    GameVidWidth = newSettings->WindowWidth;
                    GameVidHeight = newSettings->WindowHeight;
                    GameVidRect.right = newSettings->WindowWidth;
                    GameVidRect.bottom = newSettings->WindowHeight;
                    if (SavedAppSettings.RenderMode == 2) {
                        D3DSetViewport();
                    }
                    setup_screen_size();
                    WinVidNeedToResetBuffers = false;
                }
            }
        }
    }
    if (needRestart) {
        result = ApplySettings(newSettings);
        needInitRenderState = false;
        needRebuildBuffers = false;
    }
    if (needInitRenderState) {
        SavedAppSettings.PerspectiveCorrect = newSettings->PerspectiveCorrect;
        SavedAppSettings.Dither = newSettings->Dither;
        SavedAppSettings.BilinearFiltering = newSettings->BilinearFiltering;
        if (SavedAppSettings.RenderMode == 2) {
            HWR_InitState();
        }
    }
    if (needAdjustTexel) {
        S_AdjustTexelCoordinates();
    }
    if (needRebuildBuffers) {
        ClearBuffers(0x40, 0);
        result = ApplySettings(newSettings);
    }
    return result;
}

extern "C" void UpdateGameResolution(void)
{
    APP_SETTINGS newSettings;
    char modeString[64];

    newSettings = SavedAppSettings;
    newSettings.WindowWidth = GameWindowWidth;
    newSettings.WindowHeight = GameWindowHeight;
    GameApplySettings(&newSettings);
    sprintf(modeString, "%dx%d", GameVidWidth, GameVidHeight);
    DisplayModeInfo(modeString);
}

#ifdef TR2_EXE
int GameWindowHeight;
int GameWindowPositionX;
int GameWindowPositionY;
LPDIRECTDRAWSURFACE3 RenderBufferSurface;
LPDIRECTDRAWCLIPPER DDrawClipper;
PALETTEENTRY WinVidPalette[256];
LPDIRECTDRAWSURFACE3 ThirdBufferSurface;
LPDIRECTDRAWSURFACE3 PictureBufferSurface;
LPDIRECTDRAWSURFACE3 ZBufferSurface;
LPDIRECTDRAWPALETTE DDrawPalette;
LPDIRECTDRAWSURFACE3 PrimaryBufferSurface;
COLOR_BIT_MASKS ColorBitMasks;
RECT GameVidBufRect;
RECT GameVidRect;
int GameVidWidth;
int GameVidHeight;
int GameVidBPP;
int GameVidBufWidth;
int GameVidBufHeight;
bool GameVid_IsVga;
bool GameVid_IsWindowedVga;
bool GameVid_IsFullscreenVga;
bool IsWindowedVGA;
bool Is16bitTextures;
bool NeedToReloadTextures;
LPDIRECTDRAWSURFACE3 BackBufferSurface;
#endif
