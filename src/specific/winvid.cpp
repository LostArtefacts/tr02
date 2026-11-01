#include "include.h"
#pragma inline_depth(3)
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

extern "C" void WinVidResizeGameWindow(HWND hWnd, int edge, LPRECT rect)
{
    bool isShiftPressed;
    int width;
    int height;
    int newHeight;
    int newWidth;

    if (IsGameFullScreen) {
        rect->left = 0;
        rect->top = 0;
        rect->right = FullScreenWidth;
        rect->bottom = FullScreenHeight;
        return;
    }
    isShiftPressed = GetAsyncKeyState(VK_SHIFT) < 0;
    width = rect->right - rect->left;
    height = rect->bottom - rect->top;
    GameWindowCalculateSizeFromClientByZero(&width, &height);
    if (edge == WMSZ_TOP || edge == WMSZ_BOTTOM) {
        newHeight = height;
        if (isShiftPressed)
            newHeight &= ~0x1F;
        newWidth = CalculateWindowWidth(width, newHeight);
    } else {
        newWidth = width;
        if (isShiftPressed)
            newWidth &= ~0x1F;
        newHeight = CalculateWindowHeight(newWidth, height);
    }
    if (IsMinWindowSizeSet) {
        if (newHeight < MinWindowClientHeight)
            newHeight = MinWindowClientHeight;
        if (newWidth < MinWindowClientWidth)
            newWidth = MinWindowClientWidth;
    }
    if (IsMaxWindowSizeSet) {
        if (newHeight > MaxWindowClientHeight)
            newHeight = MaxWindowClientHeight;
        if (newWidth > MaxWindowClientWidth)
            newWidth = MaxWindowClientWidth;
    }
    GameWindowCalculateSizeFromClient(&newWidth, &newHeight);
    switch (edge) {
    case WMSZ_TOPLEFT:
        rect->left = rect->right - newWidth;
        rect->top = rect->bottom - newHeight;
        break;
    case WMSZ_RIGHT:
    case WMSZ_BOTTOM:
    case WMSZ_BOTTOMRIGHT:
        rect->right = rect->left + newWidth;
        rect->bottom = rect->top + newHeight;
        break;
    case WMSZ_LEFT:
    case WMSZ_BOTTOMLEFT:
        rect->bottom = rect->top + newHeight;
        rect->left = rect->right - newWidth;
        break;
    case WMSZ_TOP:
    case WMSZ_TOPRIGHT:
        rect->top = rect->bottom - newHeight;
        rect->right = rect->left + newWidth;
        break;
    }
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
bool HideDDrawGameWindow(void);
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
extern int GameWindow_X;
extern int GameWindow_Y;
void WinVidShowGameWindow(int nCmdShow);
void WinVidHideGameWindow(void);
}

extern "C" bool ShowDDrawGameWindow(bool active)
{
    RECT rect;
    DWORD flags;
    HRESULT rc;

    if (!HGameWindow || !DDraw)
        return false;
    if (!IsDDrawGameWindowShow)
    {
        GetWindowRect(HGameWindow, &rect);
        GameWindow_X = rect.left;
        GameWindow_Y = rect.top;
        if (active)
            WinVidShowGameWindow(SW_SHOW);
        IsGameWindowUpdating = true;
        flags = DDSCL_FULLSCREEN | DDSCL_ALLOWREBOOT | DDSCL_EXCLUSIVE | DDSCL_ALLOWMODEX;
        if (!active)
            flags = DDSCL_FULLSCREEN | DDSCL_ALLOWREBOOT | DDSCL_NOWINDOWCHANGES | DDSCL_EXCLUSIVE | DDSCL_ALLOWMODEX;
        rc = DDraw->SetCooperativeLevel(HGameWindow, flags);
        IsGameWindowUpdating = false;
        if (DX_TRY(rc))
            return false;
        IsDDrawGameWindowShow = true;
    }
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

extern "C" bool WinVidCopyBitmapToBuffer(LPDIRECTDRAWSURFACE3 surface, BYTE *bitmap)
{
    DDSURFACEDESC desc;
    BYTE *src;
    BYTE *dst;
    DWORD width;
    DWORD i;

    if (!(bool)FAILED(WinVidBufferLock(surface, &desc, DDLOCK_WRITEONLY | DDLOCK_WAIT)))
    {
        src = bitmap;
        dst = (BYTE *)desc.lpSurface;
        width = desc.dwWidth;
        for (i = desc.dwHeight; i != 0; i--)
        {
            memcpy(dst, src, width);
            src += width;
            dst += desc.lPitch;
        }
        WinVidBufferUnlock(surface, &desc);
        return true;
    }
    return false;
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

extern "C" bool HideDDrawGameWindow(void)
{
    if (!HGameWindow || !DDraw)
        return false;
    if (IsDDrawGameWindowShow)
    {
        WinVidHideGameWindow();
        IsGameWindowUpdating = true;
        if (DX_TRY(DDraw->SetCooperativeLevel(HGameWindow, DDSCL_NORMAL)))
        {
            IsGameWindowUpdating = false;
            return false;
        }
        IsDDrawGameWindowShow = false;
        SetWindowPos(HGameWindow, NULL, GameWindow_X, GameWindow_Y, 0, 0, SWP_NOCOPYBITS | SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
        IsGameWindowUpdating = false;
    }
    return true;
}

struct STRING_FLAGGED {
    LPTSTR lpString;
    bool isPresented;
    STRING_FLAGGED(DWORD size)
    {
        lpString = new char[size];
        *lpString = 0;
        isPresented = true;
    }
    ~STRING_FLAGGED();
    STRING_FLAGGED &operator=(const STRING_FLAGGED &src)
    {
        if (this != &src)
        {
            if (!isPresented)
                lpString = new char[strlen(src.lpString) + 1];
            strcpy(lpString, src.lpString);
        }
        return *this;
    }
};

inline STRING_FLAGGED::~STRING_FLAGGED()
{
    if (isPresented)
        delete[] lpString;
}

struct DISPLAY_MODE {
    int width;
    int height;
    int bpp;
    int vga;
    bool CompareVideoModes(DISPLAY_MODE *mode);
};

struct DISPLAY_MODE_NODE {
    DISPLAY_MODE_NODE *next;
    DISPLAY_MODE_NODE *previous;
    DISPLAY_MODE body;
    DISPLAY_MODE_NODE(DISPLAY_MODE_NODE *prevNode, DISPLAY_MODE_NODE *nextNode)
    {
        next = nextNode;
        previous = prevNode;
    }
};

struct DISPLAY_MODE_LIST {
    DISPLAY_MODE_NODE *head;
    DISPLAY_MODE_NODE *tail;
    DWORD dwCount;
    DISPLAY_MODE_LIST();
    ~DISPLAY_MODE_LIST()
    {
        DisplayModeListDelete();
    }
    void DisplayModeListDelete(void);
    DISPLAY_MODE *InsertDisplayModeInListHead(void);
    DISPLAY_MODE *InsertDisplayModeInListTail(void);
    DISPLAY_MODE_LIST &operator=(const DISPLAY_MODE_LIST &src)
    {
        DISPLAY_MODE_NODE *node;

        if (this != &src)
        {
            DisplayModeListDelete();
            for (node = src.head; node; node = node->next)
                *InsertDisplayModeInListTail() = node->body;
        }
        return *this;
    }
};

struct DISPLAY_ADAPTER_INFO {
    LPGUID lpAdapterGuid;
    GUID adapterGuid;
    STRING_FLAGGED driverDescription;
    STRING_FLAGGED driverName;
    DDCAPS driverCaps;
    DDCAPS helCaps;
    GUID deviceGuid;
    D3DDEVICEDESC D3DHWDeviceDesc;
    DISPLAY_ADAPTER_INFO();
};

struct DISPLAY_ADAPTER : DISPLAY_ADAPTER_INFO {
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
};

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
bool ShowDDrawGameWindow(bool active);
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

extern "C" bool WinVidGetDisplayModes(void)
{
    DISPLAY_ADAPTER_NODE *node;
    DISPLAY_ADAPTER *adapter;

    node = DisplayAdapterList.head;
    if (node)
    {
        do
        {
            adapter = &node->body;
            DDrawCreate(adapter->lpAdapterGuid);
            ShowDDrawGameWindow(false);
            DDraw->EnumDisplayModes(DDEDM_STANDARDVGAMODES, NULL, adapter, EnumDisplayModesCallback);
            HideDDrawGameWindow();
            DDrawRelease();
        } while (node && (node = node->next));
    }
    return true;
}

extern "C" void WinVidExitMessage(void)
{
    if (HGameWindow && IsWindow(HGameWindow))
    {
        PostMessage(HGameWindow, WM_CLOSE, 0, 0);
        while (WinVidSpinMessageLoop(false))
            ;
        HGameWindow = NULL;
    }
}

extern "C" void WinVidFinish(void)
{
    if (IsDDrawGameWindowShow)
        HideDDrawGameWindow();
    DDrawRelease();
}

extern "C" {
bool WinVidRegisterGameWindowClass(void);
bool WinVidCreateGameWindow(void);
bool WinVidGetDisplayAdapters(void);
BOOL WINAPI EnumDisplayAdaptersCallback(GUID *lpGUID, LPTSTR lpDriverDescription, LPTSTR lpDriverName, LPVOID lpContext);
}

extern "C" LRESULT CALLBACK WinVidGameWindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

extern "C" bool WinVidRegisterGameWindowClass(void)
{
    WNDCLASSEXA wndClass;

    memset(&wndClass, 0, sizeof(wndClass));
    wndClass.style = 0;
    wndClass.cbSize = sizeof(wndClass);
    wndClass.lpfnWndProc = WinVidGameWindowProc;
    wndClass.hInstance = GhInstance;
    wndClass.hIcon = LoadIconA(GhInstance, MAKEINTRESOURCE(100));
    wndClass.hCursor = LoadCursorA(NULL, IDC_ARROW);
    wndClass.lpszClassName = ClassName;
    return RegisterClassExA(&wndClass) ? true : false;
}

extern "C" {
extern BOOL IsFmvPlaying;
extern int GameWindowPositionX;
extern int GameWindowPositionY;
extern bool IsGameWindowMaximized;
extern bool IsGameWindowChanging;
extern bool WinVidNeedToResetBuffers;
extern LPDIRECTDRAWSURFACE3 RenderBufferSurface;
extern LPDIRECTDRAWSURFACE3 BackBufferSurface;
extern LPDIRECTDRAWSURFACE3 PrimaryBufferSurface;
extern LPDIRECTDRAWPALETTE DDrawPalette;
void UpdateGameResolution(void);
bool WinVidCheckGameWindowPalette(HWND hWnd);
}

extern "C" LRESULT CALLBACK WinVidGameWindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    HDC hdc;
    HBRUSH hBrush;
    int width, height;
    PAINTSTRUCT paint;

    if (IsFmvPlaying) {
        switch (Msg) {
        case WM_DESTROY:
            IsGameWindowCreated = false;
            HGameWindow = NULL;
            PostQuitMessage(0);
            break;
        case WM_MOVE:
            GameWindowPositionX = (int)(short)LOWORD(lParam);
            GameWindowPositionY = (int)(short)HIWORD(lParam);
            break;
        case WM_ACTIVATEAPP:
            IsGameWindowActive = (wParam != 0);
            break;
        case WM_SYSCOMMAND:
            if (wParam == SC_KEYMENU)
                return 0;
            break;
        }
        return DefWindowProcA(hWnd, Msg, wParam, lParam);
    }

    switch (Msg) {
    case WM_CREATE:
        IsGameWindowCreated = true;
        break;

    case WM_MOVE:
        GameWindowPositionX = (int)(short)LOWORD(lParam);
        GameWindowPositionY = (int)(short)HIWORD(lParam);
        break;

    case WM_SIZE:
        switch (wParam) {
        case SIZE_RESTORED:
            IsGameWindowMaximized = false;
            IsGameWindowMinimized = false;
            break;
        case SIZE_MAXIMIZED:
            IsGameWindowMaximized = true;
            IsGameWindowMinimized = false;
            break;
        case SIZE_MINIMIZED:
            IsGameWindowMinimized = true;
        default:
            goto done;
        }
        if (IsGameFullScreen)
            break;
        width = (int)(short)LOWORD(lParam);
        height = (int)(short)HIWORD(lParam);
        if (width == GameWindowWidth && height == GameWindowHeight)
            break;
        GameWindowWidth = width;
        GameWindowHeight = height;
        if (IsGameWindowUpdating)
            break;
        UpdateGameResolution();
        break;

    case WM_PAINT:
        hdc = BeginPaint(hWnd, &paint);
        if (IsGameFullScreen || !PrimaryBufferSurface || !(SavedAppSettings.RenderMode == RM_Software ? RenderBufferSurface : BackBufferSurface)) {
            hBrush = (HBRUSH)GetStockObject(BLACK_BRUSH);
            FillRect(hdc, &paint.rcPaint, hBrush);
        } else {
            if (SavedAppSettings.RenderMode == RM_Software && !WinVidCheckGameWindowPalette(hWnd) && RenderBufferSurface)
                WinVidClearBuffer(RenderBufferSurface, NULL, 0);
            DX_UpdateFrame(false, NULL);
        }
        EndPaint(hWnd, &paint);
        return 0;

    case WM_ACTIVATE:
        if (LOWORD(wParam) && DDrawPalette && PrimaryBufferSurface)
            PrimaryBufferSurface->SetPalette(DDrawPalette);
        break;

    case WM_DESTROY:
        IsGameWindowCreated = false;
        HGameWindow = NULL;
        PostQuitMessage(0);
        break;

    case WM_ERASEBKGND:
        return 1;

    case WM_ACTIVATEAPP:
        if (wParam && !IsGameWindowActive && SavedAppSettings.RenderMode == RM_Hardware && IsGameFullScreen)
            WinVidNeedToResetBuffers = true;
        IsGameWindowActive = (wParam != 0);
        break;

    case WM_SETCURSOR:
        if (IsGameFullScreen) {
            SetCursor(NULL);
            return 1;
        }
        break;

    case WM_GETMINMAXINFO:
        if (WinVidGetMinMaxInfo((LPMINMAXINFO)lParam))
            return 0;
        break;

    case WM_NCPAINT:
    case WM_NCLBUTTONDOWN:
    case WM_NCLBUTTONDBLCLK:
    case WM_NCRBUTTONDOWN:
    case WM_NCRBUTTONDBLCLK:
    case WM_NCMBUTTONDOWN:
    case WM_NCMBUTTONDBLCLK:
        if (IsGameFullScreen)
            return 0;
        break;

    case WM_SYSCOMMAND:
        if (wParam == SC_KEYMENU)
            return 0;
        break;

    case WM_MOVING:
        if (IsGameFullScreen || IsGameWindowMaximized) {
            GetWindowRect(hWnd, (LPRECT)lParam);
            return 1;
        }
        break;

    case WM_SIZING:
        WinVidResizeGameWindow(hWnd, wParam, (LPRECT)lParam);
        break;

    case WM_ENTERSIZEMOVE:
        IsGameWindowChanging = true;
        break;

    case WM_EXITSIZEMOVE:
        IsGameWindowChanging = false;
        break;

    case WM_PALETTECHANGED:
        if (hWnd != (HWND)wParam && !IsGameFullScreen && DDrawPalette)
            InvalidateRect(hWnd, NULL, FALSE);
        break;
    }
done:
    return DefWindowProcA(hWnd, Msg, wParam, lParam);
}

typedef struct {
    uint8 red;
    uint8 green;
    uint8 blue;
} RGB888;

extern "C" bool WinVidCheckGameWindowPalette(HWND hWnd)
{
    HDC hdc;
    PALETTEENTRY sysPalette[256];
    RGB888 bufPalette[256];
    uint8 *ptr;
    PALETTEENTRY *src;
    int i;

    hdc = GetDC(hWnd);
    if (!hdc)
        return false;
    GetSystemPaletteEntries(hdc, 0, 256, sysPalette);
    ReleaseDC(hWnd, hdc);
    ptr = (uint8 *)bufPalette;
    src = sysPalette;
    for (i = 0; i < 256; i++) {
        *ptr++ = src->peRed;
        *ptr++ = src->peGreen;
        *ptr++ = src->peBlue;
        src++;
    }
    bufPalette[8].blue = bufPalette[9].blue = bufPalette[246].blue = 0;
    bufPalette[8].green = bufPalette[9].green = bufPalette[246].green = 0;
    bufPalette[8].red = bufPalette[9].red = bufPalette[246].red = 0;
    return !memcmp(bufPalette, game_palette, sizeof(bufPalette));
}

extern "C" {
extern bool IsGameWindowMaximized;
extern bool IsGameWindowChanging;
}

extern "C" bool WinVidCreateGameWindow(void)
{
    RECT rect;

    IsGameWindowActive = true;
    IsGameWindowShow = true;
    IsDDrawGameWindowShow = false;
    IsMessageLoopClosed = false;
    IsGameWindowUpdating = false;
    IsGameWindowMinimized = false;
    IsGameWindowMaximized = false;
    IsGameWindowCreated = false;
    IsGameFullScreen = false;
    IsMinMaxInfoSpecial = false;
    IsGameWindowChanging = false;
    WinVidClearMinWindowSize();
    WinVidClearMaxWindowSize();
    HGameWindow = CreateWindowExA(WS_EX_APPWINDOW, ClassName, WindowName, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, GhInstance, NULL);
    if (!HGameWindow)
        return false;
    GetWindowRect(HGameWindow, &rect);
    GameWindow_X = rect.left;
    GameWindow_Y = rect.top;
    WinVidHideGameWindow();
    return true;
}

extern "C" bool WinVidInit(void)
{
    AppResultCode = 0;
    return WinVidRegisterGameWindowClass() && WinVidCreateGameWindow() && WinVidGetDisplayAdapters() && DisplayAdapterList.dwCount && WinVidGetDisplayModes();
}

extern "C" bool EnumerateDisplayAdapters(DISPLAY_ADAPTER_LIST *displayAdapterList);
extern "C" DISPLAY_ADAPTER_NODE *PrimaryDisplayAdapter;

#pragma inline_depth(4)
extern "C" bool WinVidGetDisplayAdapters(void)
{
    DISPLAY_ADAPTER_NODE *node;
    DISPLAY_ADAPTER_NODE *nextNode;

    for (node = DisplayAdapterList.head; node; node = nextNode)
    {
        nextNode = node->next;
        delete node;
    }
    DisplayAdapterList.head = NULL;
    DisplayAdapterList.tail = NULL;
    DisplayAdapterList.dwCount = 0;
    if (!EnumerateDisplayAdapters(&DisplayAdapterList))
        return false;
    for (node = DisplayAdapterList.head; node; node = node->next)
    {
        if (!node->body.lpAdapterGuid)
        {
            PrimaryDisplayAdapter = node;
            return true;
        }
        if (!node)
            break;
    }
    return false;
}

#pragma inline_depth(3)
extern "C" bool EnumerateDisplayAdapters(DISPLAY_ADAPTER_LIST *displayAdapterList)
{
    if (DX_TRY(DirectDrawEnumerate(EnumDisplayAdaptersCallback, displayAdapterList)))
        return false;
    return true;
}

extern "C" void Enumerate3DDevices(DISPLAY_ADAPTER *adapter);

extern "C" BOOL WINAPI EnumDisplayAdaptersCallback(GUID *lpGUID, LPTSTR lpDriverDescription, LPTSTR lpDriverName, LPVOID lpContext)
{
    DDCAPS driverCaps;
    DDCAPS helCaps;
    DISPLAY_ADAPTER_NODE *node;
    DISPLAY_ADAPTER_LIST *list;
    DISPLAY_ADAPTER *adapter;

    list = (DISPLAY_ADAPTER_LIST *)lpContext;
    if (DDrawCreate(lpGUID)) {
        memset(&driverCaps, 0, sizeof(driverCaps));
        memset(&helCaps, 0, sizeof(helCaps));
        driverCaps.dwSize = sizeof(DDCAPS);
        helCaps.dwSize = sizeof(DDCAPS);
        if (!DX_TRY(DDraw->GetCaps(&driverCaps, &helCaps))) {
            node = new DISPLAY_ADAPTER_NODE;
            if (node) {
                node->next = NULL;
                node->previous = list->tail;
            }
            if (list->tail)
                list->tail->next = node;
            else
                list->head = node;
            list->dwCount++;
            list->tail = node;
            adapter = &node->body;
            if (lpGUID) {
                adapter->adapterGuid = *lpGUID;
                adapter->lpAdapterGuid = &adapter->adapterGuid;
            } else {
                memset(&adapter->adapterGuid, 0, sizeof(GUID));
                adapter->lpAdapterGuid = NULL;
            }
            strcpy(adapter->driverDescription.lpString, lpDriverDescription);
            strcpy(adapter->driverName.lpString, lpDriverName);
            adapter->driverCaps = driverCaps;
            adapter->helCaps = helCaps;
            adapter->swWindowedSupported = false;
            adapter->hwWindowedSupported = false;
            adapter->isVgaMode1Presented = false;
            adapter->isVgaMode2Presented = false;
            adapter->screenWidth = 0;
            adapter->hwRenderSupported = false;
            Enumerate3DDevices(adapter);
        }
        DDrawRelease();
    }
    return TRUE;
}

extern "C" DISPLAY_ADAPTER_NODE *PrimaryDisplayAdapter;

extern "C" DISPLAY_ADAPTER_NODE *WinVidGetDisplayAdapter(GUID *lpGuid)
{
    DISPLAY_ADAPTER_NODE *node;
    DISPLAY_ADAPTER_NODE *result;

    result = NULL;
    for (node = DisplayAdapterList.head; node; node = node->next)
    {
        if (lpGuid && !memcmp(&node->body.adapterGuid, lpGuid, sizeof(GUID)))
        {
            result = node;
            break;
        }
        if (!node)
            break;
    }
    if (!result)
    {
        result = PrimaryDisplayAdapter;
        if (!result)
            result = NULL;
    }
    return result;
}

extern "C" bool WinVidGetDisplayMode(DISPLAY_MODE *dispMode)
{
    DDSURFACEDESC dsp;

    memset(&dsp, 0, sizeof(dsp));
    dsp.dwSize = sizeof(dsp);
    if (DX_TRY(DDraw->GetDisplayMode(&dsp)))
        return false;
    if (!(dsp.dwFlags & DDSD_WIDTH) || !(dsp.dwFlags & DDSD_HEIGHT) || !(dsp.dwFlags & DDSD_PIXELFORMAT) || !(dsp.ddpfPixelFormat.dwFlags & DDPF_RGB))
        return false;
    dispMode->width = dsp.dwWidth;
    dispMode->height = dsp.dwHeight;
    dispMode->bpp = dsp.ddpfPixelFormat.dwRGBBitCount;
    if (dsp.ddpfPixelFormat.dwFlags & DDPF_PALETTEINDEXED8)
        dispMode->vga = 1;
    else
        dispMode->vga = 0;
    return true;
}

extern "C" {
extern int FullScreenBPP;
extern int FullScreenVGA;
}

extern "C" bool WinVidGoFullScreen(DISPLAY_MODE *dispMode)
{
    DWORD flags;

    *(DISPLAY_MODE *)&FullScreenWidth = *dispMode;
    if (!ShowDDrawGameWindow(true))
        return false;
    flags = 0;
    if (dispMode->vga == 3)
        flags = DDSDM_STANDARDVGAMODE;
    IsGameWindowUpdating = true;
    if (DX_TRY(DDraw->SetDisplayMode(dispMode->width, dispMode->height, dispMode->bpp, 0, flags)))
        return false;
    IsGameWindowUpdating = false;
    IsGameFullScreen = true;
    return true;
}

extern "C" bool WinVidGoWindowed(int width, int height, DISPLAY_MODE *dispMode)
{
    int maxWidth;
    int maxHeight;
    RECT rect;

    if (!HideDDrawGameWindow())
        return false;
    if (!WinVidGetDisplayMode(dispMode))
        return false;
    maxWidth = dispMode->width;
    maxHeight = CalculateWindowHeight(dispMode->width, dispMode->height);
    if (maxHeight > dispMode->height)
    {
        maxHeight = dispMode->height;
        maxWidth = CalculateWindowWidth(dispMode->width, maxHeight);
    }
    WinVidSetMaxWindowSize(maxWidth, maxHeight);
    if (width > maxWidth || height > maxHeight)
    {
        width = maxWidth;
        height = maxHeight;
    }
    IsGameFullScreen = false;
    IsGameWindowUpdating = true;
    WinVidSetGameWindowSize(width, height);
    IsGameWindowUpdating = false;
    GetClientRect(HGameWindow, &rect);
    MapWindowPoints(HGameWindow, NULL, (LPPOINT)&rect, 2);
    if ((rect.left > 0 || rect.right < dispMode->width) && (rect.top > 0 || rect.bottom < dispMode->height))
        WinVidShowGameWindow(SW_SHOW);
    else
        WinVidShowGameWindow(SW_MAXIMIZE);
    dispMode->width = width;
    dispMode->height = height;
    return true;
}

extern "C" DWORD GetRenderBitDepth(DWORD dwRGBBitCount);

extern "C" void WinVidSetDisplayAdapter(DISPLAY_ADAPTER *dispAdapter)
{
    DISPLAY_MODE dispMode;
    DWORD bitDepth;
    int width;
    bool rc;

    dispAdapter->swWindowedSupported = false;
    dispAdapter->hwWindowedSupported = false;
    dispAdapter->screenWidth = 0;
    if (dispAdapter->lpAdapterGuid)
        return;
    if (!DDrawCreate(NULL))
        return;
    rc = WinVidGetDisplayMode(&dispMode);
    DDrawRelease();
    if (!rc)
        return;
    dispAdapter->swWindowedSupported = dispMode.vga == 1;
    bitDepth = GetRenderBitDepth(dispMode.bpp);
    dispAdapter->hwWindowedSupported = dispAdapter->hwRenderSupported && (dispAdapter->D3DHWDeviceDesc.dwFlags & D3DDD_DEVICERENDERBITDEPTH) && (bitDepth & dispAdapter->D3DHWDeviceDesc.dwDeviceRenderBitDepth);
    width = dispMode.width & ~0x1F;
    if (width * 3 >> 2 > dispMode.height)
        width = (dispMode.height * 4 / 3) & ~0x1F;
    dispAdapter->screenWidth = width;
}

bool DISPLAY_MODE::CompareVideoModes(DISPLAY_MODE *mode)
{
    int square1 = width * height;
    int square2 = mode->width * mode->height;

    if (square1 < square2)
        return true;
    if (square1 == square2) {
        if (bpp < mode->bpp)
            return true;
        if (bpp == mode->bpp) {
            if (vga < mode->vga)
                return true;
        }
    }
    return false;
}

void COLOR_BIT_MASKS::WinVidGetColorBitMasks(LPDDPIXELFORMAT pixelFormat)
{
    dwRBitMask = pixelFormat->dwRBitMask;
    dwGBitMask = pixelFormat->dwGBitMask;
    dwBBitMask = pixelFormat->dwBBitMask;
    dwRGBAlphaBitMask = pixelFormat->dwRGBAlphaBitMask;
    BitMaskGetNumberOfBits(dwRBitMask, &dwRBitDepth, &dwRBitOffset);
    BitMaskGetNumberOfBits(dwGBitMask, &dwGBitDepth, &dwGBitOffset);
    BitMaskGetNumberOfBits(dwBBitMask, &dwBBitDepth, &dwBBitOffset);
    BitMaskGetNumberOfBits(dwRGBAlphaBitMask, &dwRGBAlphaBitDepth, &dwRGBAlphaBitOffset);
}

DISPLAY_ADAPTER_INFO::DISPLAY_ADAPTER_INFO() : driverDescription(256), driverName(256)
{
}

DISPLAY_MODE_LIST::DISPLAY_MODE_LIST()
{
    head = NULL;
    tail = NULL;
    dwCount = 0;
}

inline void DISPLAY_MODE_LIST::DisplayModeListDelete(void)
{
    DISPLAY_MODE_NODE *node;
    DISPLAY_MODE_NODE *nextNode;

    for (node = head; node; node = nextNode) {
        nextNode = node->next;
        delete node;
    }
    head = NULL;
    tail = NULL;
    dwCount = 0;
}

DISPLAY_MODE *DISPLAY_MODE_LIST::InsertDisplayModeInListHead(void)
{
    DISPLAY_MODE_NODE *node = new DISPLAY_MODE_NODE(NULL, head);

    if (head)
        head->previous = node;
    else
        tail = node;
    head = node;
    dwCount++;
    return &node->body;
}

inline DISPLAY_MODE *DISPLAY_MODE_LIST::InsertDisplayModeInListTail(void)
{
    DISPLAY_MODE_NODE *node = new DISPLAY_MODE_NODE(tail, NULL);

    if (tail)
        tail->next = node;
    else
        head = node;
    tail = node;
    dwCount++;
    return &node->body;
}

enum WinVidStartResult {
    ERR_CantInitRenderer = 3
};

extern "C" DISPLAY_ADAPTER CurrentDisplayAdapter;

extern "C" void WinVidStart(void)
{
    DISPLAY_ADAPTER *preferred;

    if (!SavedAppSettings.PreferredDisplayAdapter)
        throw ERR_CantInitRenderer;
    preferred = &((DISPLAY_ADAPTER_NODE *)SavedAppSettings.PreferredDisplayAdapter)->body;
    CurrentDisplayAdapter = *preferred;
    if (!DDrawCreate(CurrentDisplayAdapter.lpAdapterGuid))
        throw ERR_CantInitRenderer;
}
