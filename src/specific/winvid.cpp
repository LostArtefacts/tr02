#include "include.h"
#include "global.h"
#include "winmain.h"

#define AM_4_3 0
#define AM_16_9 1

extern "C" {
extern HWND HGameWindow;
extern bool IsGameWindowCreated;
extern bool IsGameFullScreen;
extern bool IsMinMaxInfoSpecial;
extern bool IsMinWindowSizeSet;
extern bool IsMaxWindowSizeSet;
extern int GameWindowWidth;
extern int GameWindowHeight;
extern int FullScreenWidth;
extern int FullScreenHeight;
extern int MinWindowClientWidth;
extern int MinWindowClientHeight;
extern int MinWindowWidth;
extern int MinWindowHeight;
extern int MaxWindowClientWidth;
extern int MaxWindowClientHeight;
extern int MaxWindowWidth;
extern int MaxWindowHeight;
}

extern "C" void GameWindowCalculateSizeFromClient(int *width, int *height)
{
    RECT rect;

    rect.top = 0;
    rect.left = 0;
    rect.right = *width;
    rect.bottom = *height;
    AdjustWindowRectEx(&rect, GetWindowLong(HGameWindow, GWL_STYLE), FALSE, GetWindowLong(HGameWindow, GWL_EXSTYLE));
    *width = rect.right - rect.left;
    *height = rect.bottom - rect.top;
}

extern "C" void GameWindowCalculateSizeFromClientByZero(int *width, int *height)
{
    RECT rect;

    rect.bottom = 0;
    rect.right = 0;
    rect.top = 0;
    rect.left = 0;
    AdjustWindowRectEx(&rect, GetWindowLong(HGameWindow, GWL_STYLE), FALSE, GetWindowLong(HGameWindow, GWL_EXSTYLE));
    *width += rect.left - rect.right;
    *height += rect.top - rect.bottom;
}

extern "C" void WinVidSetMinWindowSize(int width, int height)
{
    MinWindowClientWidth = width;
    MinWindowClientHeight = height;
    GameWindowCalculateSizeFromClient(&width, &height);
    MinWindowWidth = width;
    MinWindowHeight = height;
    IsMinWindowSizeSet = true;
}

extern "C" void WinVidClearMinWindowSize(void)
{
    IsMinWindowSizeSet = false;
}

extern "C" void WinVidSetMaxWindowSize(int width, int height)
{
    MaxWindowClientWidth = width;
    MaxWindowClientHeight = height;
    GameWindowCalculateSizeFromClient(&width, &height);
    MaxWindowWidth = width;
    MaxWindowHeight = height;
    IsMaxWindowSizeSet = true;
}

extern "C" void WinVidClearMaxWindowSize(void)
{
    IsMaxWindowSizeSet = false;
}

extern "C" int CalculateWindowWidth(int width, int height)
{
    switch (SavedAppSettings.AspectMode)
    {
    case AM_16_9:
        return 16 * height / 9;
    case AM_4_3:
        return 4 * height / 3;
    }
    return width;
}

extern "C" int CalculateWindowHeight(int width, int height)
{
    switch (SavedAppSettings.AspectMode)
    {
    case AM_16_9:
        return (9 * width) >> 4;
    case AM_4_3:
        return (3 * width) >> 2;
    }
    return height;
}

extern "C" bool WinVidGetMinMaxInfo(LPMINMAXINFO info)
{
    int width, height;

    if (!IsGameWindowCreated)
        return false;

    if (IsGameFullScreen)
    {
        info->ptMaxTrackSize.x = FullScreenWidth;
        info->ptMaxTrackSize.y = FullScreenHeight;
        info->ptMinTrackSize.x = FullScreenWidth;
        info->ptMinTrackSize.y = FullScreenHeight;
        info->ptMaxSize.x = FullScreenWidth;
        info->ptMaxSize.y = FullScreenHeight;
        return true;
    }

    if (IsMinWindowSizeSet)
    {
        info->ptMinTrackSize.x = MinWindowWidth;
        info->ptMinTrackSize.y = MinWindowHeight;
    }

    if (IsMinMaxInfoSpecial)
    {
        width = GameWindowWidth;
        height = GameWindowHeight;
        GameWindowCalculateSizeFromClient(&width, &height);
        info->ptMaxSize.x = width;
        info->ptMaxTrackSize.x = width;
        info->ptMaxSize.y = height;
        info->ptMaxTrackSize.y = height;
    }
    else if (IsMaxWindowSizeSet)
    {
        info->ptMaxTrackSize.x = MaxWindowWidth;
        info->ptMaxTrackSize.y = MaxWindowHeight;
        info->ptMaxSize.x = MaxWindowWidth;
        info->ptMaxSize.y = MaxWindowHeight;
    }

    return IsMinWindowSizeSet || IsMaxWindowSizeSet;
}

extern "C" {
extern char ClassName[];
extern char WindowName[];
extern int MessageLoopCounter;
extern bool IsMessageLoopClosed;
extern bool IsGameToExit;
extern bool IsGameWindowActive;
extern bool IsGameWindowMinimized;
extern bool IsGameWindowShow;
extern bool IsGameWindowUpdating;
extern int AppResultCode;
extern int StopInventory;
}

extern "C" HWND WinVidFindGameWindow(void)
{
    return FindWindow(ClassName, WindowName);
}

extern "C" bool WinVidSpinMessageLoop(bool needWait)
{
    MSG msg;

    ++MessageLoopCounter;
    if (IsMessageLoopClosed)
    {
        --MessageLoopCounter;
        return false;
    }

    if (needWait)
        WaitMessage();

    for (;;)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
            {
                IsMessageLoopClosed = true;
                AppResultCode = msg.wParam;
                IsGameToExit = true;
                --MessageLoopCounter;
                StopInventory = 1;
                return false;
            }
        }
        else
        {
            if (IsGameWindowActive && !IsGameWindowMinimized)
                break;
            WaitMessage();
        }
    }

    --MessageLoopCounter;
    return true;
}

extern "C" void WinVidShowGameWindow(int nCmdShow)
{
    if (nCmdShow != SW_SHOW || !IsGameWindowShow)
    {
        IsGameWindowUpdating = true;
        ShowWindow(HGameWindow, nCmdShow);
        UpdateWindow(HGameWindow);
        IsGameWindowUpdating = false;
        IsGameWindowShow = true;
    }
}

extern "C" void WinVidHideGameWindow(void)
{
    if (IsGameWindowShow)
    {
        IsGameWindowUpdating = true;
        ShowWindow(HGameWindow, SW_HIDE);
        UpdateWindow(HGameWindow);
        IsGameWindowUpdating = false;
        IsGameWindowShow = false;
    }
}

extern "C" void WinVidSetGameWindowSize(int width, int height)
{
    GameWindowCalculateSizeFromClient(&width, &height);
    SetWindowPos(HGameWindow, NULL, 0, 0, width, height, SWP_NOCOPYBITS | SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE);
}

extern "C" {
extern bool IsDDrawGameWindowShow;
extern LPDIRECTDRAW DDrawInterface;
extern LPDIRECTDRAW2 DDraw;
void HideDDrawGameWindow(void);
}

extern "C" void DDrawRelease(void)
{
    if (DDraw)
    {
        DDraw->Release();
        DDraw = NULL;
    }
    if (DDrawInterface)
    {
        DDrawInterface->Release();
        DDrawInterface = NULL;
    }
}

inline bool DX_TRY(HRESULT hr)
{
    if (SUCCEEDED(hr))
        return false;
    return true;
}

extern "C" {
extern int LockedBufferCount;
bool WinVidClearBuffer(LPDIRECTDRAWSURFACE3 surface, LPRECT rect, DWORD fillColor);
}

extern "C" HRESULT DDrawSurfaceCreate(LPDDSURFACEDESC dsp, LPDIRECTDRAWSURFACE3 *surface)
{
    LPDIRECTDRAWSURFACE subSurface;
    HRESULT rc;

    rc = DDraw->CreateSurface(dsp, &subSurface, NULL);
    if (DX_TRY(rc))
        return rc;
    rc = subSurface->QueryInterface(IID_IDirectDrawSurface3, (LPVOID *)surface);
    subSurface->Release();
    return rc;
}

extern "C" HRESULT DDrawSurfaceRestoreLost(LPDIRECTDRAWSURFACE3 surface1, LPDIRECTDRAWSURFACE3 surface2, bool blank)
{
    HRESULT rc;

    rc = surface1->IsLost();
    if (rc == DDERR_SURFACELOST)
    {
        rc = (surface2 ? surface2 : surface1)->Restore();
        if (blank && SUCCEEDED(rc))
            WinVidClearBuffer(surface1, NULL, 0);
    }
    return rc;
}

extern "C" bool WinVidClearBuffer(LPDIRECTDRAWSURFACE3 surface, LPRECT rect, DWORD fillColor)
{
    DDBLTFX blt;

    memset(&blt, 0, sizeof(blt));
    blt.dwFillColor = fillColor;
    blt.dwSize = sizeof(blt);
    if (DX_TRY(surface->Blt(rect, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &blt)))
        return false;
    return true;
}

extern "C" HRESULT WinVidBufferLock(LPDIRECTDRAWSURFACE3 surface, LPDDSURFACEDESC desc, DWORD flags)
{
    HRESULT rc;

    memset(desc, 0, sizeof(DDSURFACEDESC));
    desc->dwSize = sizeof(DDSURFACEDESC);
    rc = surface->Lock(NULL, desc, flags, NULL);
    if (!DX_TRY(rc))
        LockedBufferCount++;
    return rc;
}

extern "C" HRESULT WinVidBufferUnlock(LPDIRECTDRAWSURFACE3 surface, LPDDSURFACEDESC desc)
{
    HRESULT rc;

    rc = surface->Unlock(desc->lpSurface);
    if (!DX_TRY(rc))
        LockedBufferCount--;
    return rc;
}

extern "C" DWORD GetRenderBitDepth(DWORD dwRGBBitCount)
{
    switch (dwRGBBitCount)
    {
    case 1:
        return DDBD_1;
    case 2:
        return DDBD_2;
    case 4:
        return DDBD_4;
    case 8:
        return DDBD_8;
    case 16:
        return DDBD_16;
    case 24:
        return DDBD_24;
    case 32:
        return DDBD_32;
    }
    return 0;
}

typedef struct {
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
} COLOR_BIT_MASKS;

extern "C" void BitMaskGetNumberOfBits(DWORD bitMask, DWORD *bitDepth, DWORD *bitOffset)
{
    DWORD i;

    if (bitMask)
    {
        for (i = 0; !(bitMask & 1); i++)
            bitMask >>= 1;
        *bitOffset = i;
        for (i = 0; bitMask; i++)
            bitMask >>= 1;
        *bitDepth = i;
    }
    else
    {
        *bitOffset = 0;
        *bitDepth = 0;
    }
}

extern "C" DWORD CalculateCompatibleColor(COLOR_BIT_MASKS *mask, int red, int green, int blue, int alpha)
{
    return (red >> (8 - mask->dwRBitDepth) << mask->dwRBitOffset) | (green >> (8 - mask->dwGBitDepth) << mask->dwGBitOffset) | (blue >> (8 - mask->dwBBitDepth) << mask->dwBBitOffset) | (alpha >> (8 - mask->dwRGBAlphaBitDepth) << mask->dwRGBAlphaBitOffset);
}

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

typedef struct DISPLAY_ADAPTER_NODE {
    struct DISPLAY_ADAPTER_NODE *next;
    struct DISPLAY_ADAPTER_NODE *previous;
    DISPLAY_ADAPTER body;
} DISPLAY_ADAPTER_NODE;

typedef struct {
    DISPLAY_ADAPTER_NODE *head;
    DISPLAY_ADAPTER_NODE *tail;
    DWORD dwCount;
} DISPLAY_ADAPTER_LIST;

extern "C" {
extern DISPLAY_ADAPTER_LIST DisplayAdapterList;
void ShowDDrawGameWindow(bool active);
HRESULT WINAPI EnumDisplayModesCallback(LPDDSURFACEDESC lpDDSurfaceDesc, LPVOID lpContext);
}

extern "C" bool DDrawCreate(LPGUID lpGUID)
{
    if (DX_TRY(DirectDrawCreate(lpGUID, &DDrawInterface, NULL)))
        return false;
    if (DX_TRY(DDrawInterface->QueryInterface(IID_IDirectDraw2, (LPVOID *)&DDraw)))
        return false;
    DDraw->SetCooperativeLevel(HGameWindow, DDSCL_NORMAL);
    return true;
}

extern "C" void WinVidFinish(void)
{
    if (IsDDrawGameWindowShow)
        HideDDrawGameWindow();
    DDrawRelease();
}
