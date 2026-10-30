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
