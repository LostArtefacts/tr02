// cflags: /GX
#include "include.h"
#include "global.h"
#include <stdio.h>

struct DISPLAY_MODE {
    int width;
    int height;
    int bpp;
    int vga;
    bool CompareVideoModes(DISPLAY_MODE *mode);
};

#pragma pack(push, 1)
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

typedef struct {
    void *next;
    void *previous;
    BYTE adapterBody[0x3d2];
    DISPLAY_MODE_LIST hwDispModeList;
    DISPLAY_MODE_LIST swDispModeList;
} DISPLAY_ADAPTER_NODE;

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
extern DISPLAY_ADAPTER_NODE *PrimaryDisplayAdapter;
extern char RendererReinitFailed[];
}

void RenderStart(bool reset);
void RenderFinish(bool reset);
void S_ExitSystem(char *message);

extern "C" {
extern int GameVidWidth;
extern int GameVidHeight;
extern int GameVidBPP;
extern char ModeInfoFormat[];
extern char Var4664d8[];
void S_InitialiseScreen(int type);
void S_ReloadLevelGraphics(int reloadPalettes, int reloadTexPages);
void DisplayModeInfo(char *modeString);
}

enum InitResult {
    ERR_OK,
};

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
                S_ExitSystem(RendererReinitFailed);
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
        sprintf(modeString, Var4664d8, GameVidWidth, GameVidHeight, GameVidBPP);
    } else {
        sprintf(modeString, ModeInfoFormat, GameVidWidth, GameVidHeight);
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
            S_ExitSystem(RendererReinitFailed);
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
void ClearBuffers(unsigned int flags, unsigned int fillColor);
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
