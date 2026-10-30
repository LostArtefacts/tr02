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

extern "C" void GameWindowCalculateSizeFromClient(int *width, int *height);

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

