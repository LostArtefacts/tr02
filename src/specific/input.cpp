#include "global.h"
#include <dinput.h>
#include "input.h"
#include <ddraw.h>
#include "../camera.h"
#include "../gameflow.h"
#include "../input.h"

extern "C" {
#include "../lara.h"
}

int Key(int number)
{
    sint16 key;

    key = layout[1][number];
    if (key >= 256)
    {
        if ((1 << (key & 0xff)) & joy_fire)
            return 1;
    }
    else
    {
        if (rawkey(key))
            return 1;
        if (key == DIK_RCONTROL)
            return rawkey(DIK_LCONTROL);
        if (key == DIK_LCONTROL)
            return rawkey(DIK_RCONTROL);
        if (key == DIK_RSHIFT)
            return rawkey(DIK_LSHIFT);
        if (key == DIK_LSHIFT)
            return rawkey(DIK_RSHIFT);
        if (key == DIK_RMENU)
            return rawkey(DIK_LMENU);
        if (key == DIK_LMENU)
            return rawkey(DIK_RMENU);
    }
    if (conflict[number])
        return 0;
    key = layout[0][number];
    if (rawkey(key))
        return 1;
    if (key == DIK_RCONTROL)
        return rawkey(DIK_LCONTROL);
    if (key == DIK_LCONTROL)
        return rawkey(DIK_RCONTROL);
    if (key == DIK_RSHIFT)
        return rawkey(DIK_LSHIFT);
    if (key == DIK_LSHIFT)
        return rawkey(DIK_RSHIFT);
    if (key == DIK_RMENU)
        return rawkey(DIK_LMENU);
    if (key == DIK_LMENU)
        return rawkey(DIK_RMENU);
    return 0;
}

#define GUN_OPTION 157
#define SHOTGUN_OPTION 158
#define MAGNUM_OPTION 159
#define UZI_OPTION 160
#define HARPOON_OPTION 161
#define M16_OPTION 162
#define GRENADE_OPTION 163
#define MEDI_OPTION 171
#define BIGMEDI_OPTION 172
#define FLAREBOX_OPTION 173

#define RM_Software 1
#define RM_Hardware 2

#define JOY_SENSITIVITY 8

struct DISPLAY_MODE {
    int width;
    int height;
    int bpp;
    int vga;
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

typedef struct {
    BYTE info[0x3d2];
    DISPLAY_MODE_LIST hwDispModeList;
    DISPLAY_MODE_LIST swDispModeList;
} DISPLAY_ADAPTER;

#pragma pack(push, 1)
typedef struct {
    void *PreferredDisplayAdapter;
    void *PreferredSoundAdapter;
    void *PreferredJoystick;
    DISPLAY_MODE_NODE *VideoMode;
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
extern DISPLAY_ADAPTER CurrentDisplayAdapter;
extern LPDIRECTDRAWSURFACE PrimaryBufferSurface;
extern int IsFmvPlaying;
extern bool IsGameToExit;
extern bool IsVidModeLock;
extern sint16 IsInventoryActive;
extern int GameVidWidth;
extern int GameVidHeight;
extern int GameVidBPP;
extern int perspective_distance;
bool WinVidSpinMessageLoop(bool needWait);
void WinInReadKeyboard(LPVOID lpInputData);
DWORD WinInReadJoystick(int *x, int *y);
int Inv_RequestItem(int itemNum);
void UseItem(sint16 object_num);
void ScreenShot(LPDIRECTDRAWSURFACE screen);
bool GameApplySettings(APP_SETTINGS *newSettings);
int CalculateWindowWidth(int width, int height);
void IncreaseScreenSize(void);
void DecreaseScreenSize(void);
}

extern int joy_x;
extern int joy_y;

inline bool IsModeListEmpty(DISPLAY_MODE_LIST *list)
{
    return list->dwCount == 0;
}

int S_UpdateInput()
{
    sint32 linput;
    bool tShift;
    bool tApply;
    int width;
    int height;
    DISPLAY_MODE_NODE *mode;
    DISPLAY_MODE_LIST *modeList;
    DISPLAY_MODE targetMode;
    APP_SETTINGS newSettings;
    static int medi_used = 0;
    static bool tDebounceF3 = false;
    static bool tDebounceF4 = false;
    static bool tDebounceF7 = false;
    static bool tDebounceF8 = false;
    static bool tDebounceF11 = false;

    WinVidSpinMessageLoop(false);
    WinInReadKeyboard(keymap);
    joy_fire = WinInReadJoystick(&joy_x, &joy_y);
    linput = 0;
    if (joy_x < -JOY_SENSITIVITY)
        linput |= IN_LEFT;
    else if (joy_x > JOY_SENSITIVITY)
        linput |= IN_RIGHT;
    if (joy_y > JOY_SENSITIVITY)
        linput |= IN_BACK;
    else if (joy_y < -JOY_SENSITIVITY)
        linput |= IN_FORWARD;
    if (Key(0))
        linput |= IN_FORWARD;
    if (Key(1))
        linput |= IN_BACK;
    if (Key(2))
        linput |= IN_LEFT;
    if (Key(3))
        linput |= IN_RIGHT;
    if (Key(4))
        linput |= IN_STEPL;
    if (Key(5))
        linput |= IN_STEPR;
    if (Key(6))
        linput |= IN_SLOW;
    if (Key(7))
        linput |= IN_JUMP;
    if (Key(8))
        linput |= IN_ACTION;
    if (Key(9))
        linput |= IN_DRAW;
    if (Key(10))
        linput |= IN_F;
    if (Key(11))
        linput |= IN_LOOK;
    if (Key(12))
        linput |= IN_ROLL;
    if (Key(13) && camera.type != CINEMATIC_CAMERA)
        linput |= IN_OPTION;
    if ((linput & IN_FORWARD) && (linput & IN_BACK))
        linput |= IN_ROLL;
    if (rawkey(DIK_RETURN) || (linput & IN_ACTION))
        linput |= IN_SELECT;
    if (rawkey(DIK_ESCAPE))
        linput |= IN_DESELECT;
    if ((linput & (IN_LEFT | IN_RIGHT)) == (IN_LEFT | IN_RIGHT))
        linput -= IN_LEFT | IN_RIGHT;
    if (IsFmvPlaying)
    {
        input = linput;
        return IsGameToExit;
    }
    if (rawkey(DIK_1) && Inv_RequestItem(GUN_OPTION))
        lara.request_gun_type = LG_PISTOLS;
    else if (rawkey(DIK_2) && Inv_RequestItem(SHOTGUN_OPTION))
        lara.request_gun_type = LG_SHOTGUN;
    else if (rawkey(DIK_3) && Inv_RequestItem(MAGNUM_OPTION))
        lara.request_gun_type = LG_MAGNUMS;
    else if (rawkey(DIK_4) && Inv_RequestItem(UZI_OPTION))
        lara.request_gun_type = LG_UZIS;
    else if (rawkey(DIK_5) && Inv_RequestItem(HARPOON_OPTION))
        lara.request_gun_type = LG_HARPOON;
    else if (rawkey(DIK_6) && Inv_RequestItem(M16_OPTION))
        lara.request_gun_type = LG_M16;
    else if (rawkey(DIK_7) && Inv_RequestItem(GRENADE_OPTION))
        lara.request_gun_type = LG_GRENADE;
    else if (rawkey(DIK_0) && Inv_RequestItem(FLAREBOX_OPTION))
        lara.request_gun_type = LG_FLARE;
    if (rawkey(DIK_8) && Inv_RequestItem(MEDI_OPTION))
    {
        if (!medi_used)
        {
            UseItem(MEDI_OPTION);
            medi_used = 15;
        }
    }
    else if (rawkey(DIK_9) && Inv_RequestItem(BIGMEDI_OPTION))
    {
        if (!medi_used)
        {
            UseItem(BIGMEDI_OPTION);
            medi_used = 15;
        }
    }
    else if (medi_used)
        medi_used--;
    if (rawkey(DIK_S))
        ScreenShot(PrimaryBufferSurface);
    tShift = rawkey(DIK_LSHIFT) || rawkey(DIK_RSHIFT);
    if (SavedAppSettings.RenderMode != RM_Software)
    {
        if (rawkey(DIK_F7))
        {
            if (!tDebounceF7)
            {
                tDebounceF7 = true;
                newSettings = SavedAppSettings;
                if (tShift)
                {
                    if (SavedAppSettings.FullScreen)
                    {
                        newSettings.TripleBuffering = !SavedAppSettings.TripleBuffering;
                        GameApplySettings(&newSettings);
                    }
                }
                else
                {
                    newSettings.ZBuffer = !SavedAppSettings.ZBuffer;
                    GameApplySettings(&newSettings);
                }
            }
        }
        else
            tDebounceF7 = false;
        if (rawkey(DIK_F8))
        {
            if (!tDebounceF8)
            {
                tDebounceF8 = true;
                newSettings = SavedAppSettings;
                if (tShift)
                    newSettings.PerspectiveCorrect = !SavedAppSettings.PerspectiveCorrect;
                else
                    newSettings.BilinearFiltering = !SavedAppSettings.BilinearFiltering;
                GameApplySettings(&newSettings);
            }
        }
        else
            tDebounceF8 = false;
        if (rawkey(DIK_F11))
        {
            if (!tDebounceF11)
            {
                tDebounceF11 = true;
                newSettings = SavedAppSettings;
                newSettings.Dither = !SavedAppSettings.Dither;
                GameApplySettings(&newSettings);
            }
        }
        else
            tDebounceF11 = false;
    }
    else
    {
        if (rawkey(DIK_F7))
        {
            if (!tDebounceF7)
            {
                tDebounceF7 = true;
                if (tShift)
                {
                    if (SavedAppSettings.FullScreen)
                    {
                        newSettings = SavedAppSettings;
                        newSettings.TripleBuffering = !SavedAppSettings.TripleBuffering;
                        GameApplySettings(&newSettings);
                    }
                }
                else
                {
                    SavedAppSettings.PerspectiveCorrect = !SavedAppSettings.PerspectiveCorrect;
                    perspective_distance = SavedAppSettings.PerspectiveCorrect ? 0x6000000 : 0x3000000;
                }
            }
        }
        else
            tDebounceF7 = false;
    }
    if (!IsVidModeLock && rawkey(DIK_F12))
    {
        newSettings = SavedAppSettings;
        tApply = false;
        if (tShift)
        {
            if (!IsInventoryActive)
            {
                if (SavedAppSettings.RenderMode == RM_Software)
                {
                    modeList = &CurrentDisplayAdapter.hwDispModeList;
                    if (!IsModeListEmpty(modeList))
                    {
                        newSettings.RenderMode = RM_Hardware;
                        targetMode.width = GameVidWidth;
                        targetMode.height = GameVidHeight;
                        targetMode.bpp = 16;
                        targetMode.vga = 0;
                        mode = modeList->head;
                        if (mode)
                        {
                            while (mode->body.CompareVideoModes(&targetMode))
                            {
                                mode = mode->next;
                                if (!mode)
                                {
                                    mode = modeList->tail;
                                    break;
                                }
                            }
                        }
                        newSettings.VideoMode = mode;
                        newSettings.FullScreen = true;
                        tApply = true;
                    }
                }
                else
                {
                    modeList = &CurrentDisplayAdapter.swDispModeList;
                    if (!IsModeListEmpty(modeList))
                    {
                        newSettings.RenderMode = RM_Software;
                        targetMode.width = GameVidWidth;
                        targetMode.height = GameVidHeight;
                        targetMode.bpp = 8;
                        targetMode.vga = 0;
                        mode = modeList->head;
                        if (mode)
                        {
                            while (mode->body.CompareVideoModes(&targetMode))
                            {
                                mode = mode->next;
                                if (!mode)
                                {
                                    mode = modeList->tail;
                                    break;
                                }
                            }
                        }
                        newSettings.VideoMode = mode;
                        newSettings.FullScreen = true;
                        tApply = true;
                    }
                }
            }
        }
        else
        {
            newSettings.FullScreen = !SavedAppSettings.FullScreen;
            if (newSettings.FullScreen)
            {
                if (SavedAppSettings.RenderMode == RM_Hardware)
                    modeList = &CurrentDisplayAdapter.hwDispModeList;
                else
                    modeList = &CurrentDisplayAdapter.swDispModeList;
                if (!IsModeListEmpty(modeList))
                {
                    targetMode.width = GameVidWidth;
                    targetMode.height = GameVidHeight;
                    targetMode.bpp = GameVidBPP;
                    targetMode.vga = 0;
                    mode = modeList->head;
                    if (mode)
                    {
                        while (mode->body.CompareVideoModes(&targetMode))
                        {
                            mode = mode->next;
                            if (!mode)
                            {
                                mode = modeList->tail;
                                break;
                            }
                        }
                    }
                    newSettings.VideoMode = mode;
                    tApply = true;
                }
            }
            else
            {
                width = phd_winwidth;
                if (width < 320)
                    width = 320;
                height = phd_winheight;
                if (height < 240)
                    height = 240;
                newSettings.WindowHeight = height;
                newSettings.WindowWidth = CalculateWindowWidth(width, height);
                newSettings.TripleBuffering = false;
                if (GameApplySettings(&newSettings))
                {
                    screen_sizer = 1.0;
                    game_sizer = 1.0;
                    setup_screen_size();
                }
            }
        }
        if (tApply)
            GameApplySettings(&newSettings);
    }
    if (!mode_lock && camera.type != CINEMATIC_CAMERA && !gameflow.screensizing_disabled)
    {
        if (rawkey(DIK_F1) && SavedAppSettings.FullScreen)
        {
            mode = SavedAppSettings.VideoMode;
            if (SavedAppSettings.RenderMode == RM_Hardware)
            {
                for (;;)
                {
                    if (mode)
                        mode = mode->previous;
                    if (!mode)
                        break;
                    if (tShift)
                    {
                        if (mode->body.width == SavedAppSettings.VideoMode->body.width && mode->body.height == SavedAppSettings.VideoMode->body.height && mode->body.vga == SavedAppSettings.VideoMode->body.vga && mode->body.bpp < SavedAppSettings.VideoMode->body.bpp)
                            break;
                    }
                    else if (mode->body.vga == SavedAppSettings.VideoMode->body.vga && mode->body.bpp == SavedAppSettings.VideoMode->body.bpp)
                        break;
                }
            }
            else if (mode)
                mode = mode->previous;
            if (mode)
            {
                newSettings = SavedAppSettings;
                newSettings.VideoMode = mode;
                GameApplySettings(&newSettings);
            }
        }
        if (rawkey(DIK_F2) && SavedAppSettings.FullScreen)
        {
            mode = SavedAppSettings.VideoMode;
            if (SavedAppSettings.RenderMode == RM_Hardware)
            {
                for (;;)
                {
                    if (mode)
                        mode = mode->next;
                    if (!mode)
                        break;
                    if (tShift)
                    {
                        if (mode->body.width == SavedAppSettings.VideoMode->body.width && mode->body.height == SavedAppSettings.VideoMode->body.height && mode->body.vga == SavedAppSettings.VideoMode->body.vga && mode->body.bpp > SavedAppSettings.VideoMode->body.bpp)
                            break;
                    }
                    else if (mode->body.vga == SavedAppSettings.VideoMode->body.vga && mode->body.bpp == SavedAppSettings.VideoMode->body.bpp)
                        break;
                }
            }
            else if (mode)
                mode = mode->next;
            if (mode)
            {
                newSettings = SavedAppSettings;
                newSettings.VideoMode = mode;
                GameApplySettings(&newSettings);
            }
        }
        if (rawkey(DIK_F3))
        {
            if (!tDebounceF3)
            {
                tDebounceF3 = true;
                if (SavedAppSettings.FullScreen)
                    DecreaseScreenSize();
            }
        }
        else
            tDebounceF3 = false;
        if (rawkey(DIK_F4))
        {
            if (!tDebounceF4)
            {
                tDebounceF4 = true;
                if (SavedAppSettings.FullScreen)
                    IncreaseScreenSize();
            }
        }
        else
            tDebounceF4 = false;
    }
    if (!gameflow.loadsave_disabled)
    {
        if (rawkey(DIK_F5))
            linput |= IN_SAVE;
        else if (rawkey(DIK_F6))
            linput |= IN_LOAD;
    }
    input = linput;
    return IsGameToExit;
}

#ifdef TR2_EXE
unsigned char keymap[256];
int joy_x;
int joy_y;
#endif
