#include "include.h"
#include "global.h"
#include "fmv.h"

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
} APP_SETTINGS;

extern "C" {
extern APP_SETTINGS SavedAppSettings;
extern int IsFmvPlaying;
extern bool IsGameToExit;
void S_CDStop(void);
void RenderFinish(bool reset);
char *GetFullPath(char *path);
void WinPlayFMV(char *path, bool playback);
void WinStopFMV(bool playback);
void FmvBackToGame(void);
}

int FMV_Play(char *szMovieName)
{
    char *path;

    if (!SavedAppSettings.DisableFMV) {
        IsFmvPlaying = 1;
        S_CDStop();
        ShowCursor(FALSE);
        RenderFinish(true);
        path = GetFullPath(szMovieName);
        WinPlayFMV(path, true);
        WinStopFMV(true);
        IsFmvPlaying = 0;
        if (!IsGameToExit)
            FmvBackToGame();
        ShowCursor(TRUE);
    }
    return IsGameToExit;
}
