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
char *GetFullPath(char *path);
void WinPlayFMV(char *path, bool playback);
void WinStopFMV(bool playback);
void FmvBackToGame(void);
}
void RenderFinish(bool reset);

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

extern "C" {
extern LPDIRECTDRAW DDraw;
extern HWND HGameWindow;
extern void *MovieContext;
extern void *FmvContext;
extern void *FmvSoundContext;
extern sint32 input;
int S_UpdateInput(void);
int Player_PassInDirectDrawObject(LPDIRECTDRAW ddraw);
int Player_InitMovie(void **movie, int a, int b, char *path, int bufSize);
int Movie_GetFormat(void *movie);
int Movie_GetXSize(void *movie);
int Movie_GetYSize(void *movie);
int Player_InitVideo(void **fmv, void *movie, int width, int height, int x, int y, int a, int b, int w, int h, int c, int d, int e);
int Player_InitPlaybackMode(HWND hwnd, void *fmv, int a, int b);
int Player_BlankScreen(RECT rect);
int Player_InitSoundSystem(HWND hwnd);
int Player_GetDSErrorCode(void);
int Movie_GetSoundPrecision(void *movie);
int Movie_GetSoundRate(void *movie);
int Movie_GetSoundChannels(void *movie);
int Player_InitSound(void **sound, int bufSize, int format, bool uncompressed, int blockSize, int channels, int rate, int precision, int a);
int Movie_SetSyncAdjust(void *movie, void *sound, int a);
int Player_InitMoviePlayback(void *movie, void *fmv, void *sound);
int Player_StartTimer(void *movie);
int Movie_GetCurrentFrame(void *movie);
int Movie_GetTotalFrames(void *movie);
int Player_PlayFrame(void *movie, void *fmv, void *sound, int a, RECT *rect, int b, int c, int d);
int Player_StopTimer(void *movie);
int Player_ShutDownSound(void **sound);
int Player_ShutDownVideo(void **fmv);
int Player_ShutDownMovie(void **movie);
int Player_ShutDownSoundSystem(void);
int Player_ReturnPlaybackMode(void);
}

void WinPlayFMV(char *path, bool playback)
{
    RECT rect;
    int result;

    rect.left = 0;
    rect.top = 0;
    rect.right = 640;
    rect.bottom = 480;

    if (Player_PassInDirectDrawObject(DDraw) || Player_InitMovie(&MovieContext, 0, 0, path, 0x200000) || Movie_GetFormat(MovieContext) != 130)
        return;

    if (Player_InitVideo(&FmvContext, MovieContext, Movie_GetXSize(MovieContext), Movie_GetYSize(MovieContext), 0, 240 - Movie_GetYSize(MovieContext), 0, 0, 640, 480, 0, 1, 13))
        return;
    if (playback && Player_InitPlaybackMode(HGameWindow, FmvContext, 1, 0))
        return;

    Player_BlankScreen(rect);

    if (Player_InitSoundSystem(HGameWindow) || Player_GetDSErrorCode() < 0)
        return;

    if (Player_InitSound(&FmvSoundContext, 0x4000, Movie_GetSoundPrecision(MovieContext) != 4 ? 1 : 4, Movie_GetSoundPrecision(MovieContext) != 4, 0x1000, Movie_GetSoundChannels(MovieContext), Movie_GetSoundRate(MovieContext), Movie_GetSoundPrecision(MovieContext), 2))
        return;

    Movie_SetSyncAdjust(MovieContext, FmvSoundContext, 4);

    result = Player_InitMoviePlayback(MovieContext, FmvContext, FmvSoundContext);
    if (result)
        return;

    S_UpdateInput();
    Player_StartTimer(MovieContext);
    Player_BlankScreen(rect);
    S_UpdateInput();

    while (Movie_GetCurrentFrame(MovieContext) < Movie_GetTotalFrames(MovieContext) && !result)
    {
        result = Player_PlayFrame(MovieContext, FmvContext, FmvSoundContext, 0, &rect, 0, 0, 0);
        if (S_UpdateInput() || (input & 0x100))
            break;
    }
}

void WinStopFMV(bool playback)
{
    Player_StopTimer(MovieContext);
    Player_ShutDownSound(&FmvSoundContext);
    Player_ShutDownVideo(&FmvContext);
    Player_ShutDownMovie(&MovieContext);
    Player_ShutDownSoundSystem();
    if (playback)
        Player_ReturnPlaybackMode();
}

int FMV_PlayIntro(char *szMovie1, char *szMovie2)
{
    char *path;

    if (!SavedAppSettings.DisableFMV) {
        IsFmvPlaying = 1;
        ShowCursor(FALSE);
        RenderFinish(true);
        path = GetFullPath(szMovie1);
        WinPlayFMV(path, true);
        WinStopFMV(true);
        path = GetFullPath(szMovie2);
        WinPlayFMV(path, true);
        WinStopFMV(true);
        IsFmvPlaying = 0;
        if (!IsGameToExit)
            FmvBackToGame();
        ShowCursor(TRUE);
    }
    return IsGameToExit;
}

#ifdef TR2_EXE
int IsFmvPlaying;
void *MovieContext;
void *FmvContext;
void *FmvSoundContext;
#endif
