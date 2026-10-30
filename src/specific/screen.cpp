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
