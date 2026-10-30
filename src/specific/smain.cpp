#include "global.h"
#include "../gameflow.h"
#include "../savegame.h"

extern sint16 layout[2][14];

extern "C" {
extern sint16 Option_Music_Volume;
extern sint16 Option_SoundFX_Volume;
extern int detail_level;
extern double game_sizer;
BOOL OpenGameRegistryKey(LPCTSTR key);
LONG CloseGameRegistryKey(void);
LONG SetRegistryDwordValue(LPCTSTR lpValueName, DWORD value);
LONG SetRegistryFloatValue(LPCTSTR lpValueName, double value);
LONG SetRegistryBinaryValue(LPCTSTR lpValueName, LPBYTE value, DWORD valueSize);
bool GetRegistryDwordValue(LPCTSTR lpValueName, DWORD *pValue, DWORD defaultValue);
bool GetRegistryFloatValue(LPCTSTR lpValueName, double *value, double defaultValue);
bool GetRegistryBinaryValue(LPCTSTR lpValueName, LPBYTE value, DWORD valueSize, LPBYTE defaultValue);
void S_SoundSetMasterVolume(int volume);
void S_CDVolume(int volume);
}

#define STARTGAME 0
#define STARTSAVEDGAME 0x100
#define STARTCINE 0x200
#define STARTDEMO 0x400
#define EXIT_TO_TITLE 0x500
#define LEVELCOMPLETE 0x600
#define EXITGAME 0x700
#define EXIT_TO_OPTION 0x800
#define PASSPORT_OPTION 120
#define PHOTO_OPTION 122

extern "C" {
extern int NoInputCounter;
extern int title_loaded;
extern int IsResetFlag;
extern sint16 InventoryChosen;
extern int InventoryExtraData[];
extern SAVEGAME_INFO savegame;
void T_InitPrint(void);
void TempVideoAdjust(int hires, double sizer);
int InitialiseLevel(int level_number, int type);
void S_DisplayPicture(char *file, int unused);
void S_CDPlay(sint16 track, int mode);
int Display_Inventory(int inventory_mode);
void S_FadeToBlack(void);
void S_DontDisplayPicture(void);
void S_CDStop(void);
void Inv_RemoveAllItems(void);
int S_LoadGame(void *data, int size, int slot);
void InitialiseStartInfo(void);
}

extern "C" {
extern int HiRes;
extern int GameMainReset;
extern double screen_sizer;
extern char *malloc_buffer;
extern char StringToShow[128];
extern bool IsVidModeLock;
extern uint8 game_palette[];
extern bool IsGameToExit;
extern bool GF_StartGame;
int S_InitialiseSystem(void);
int GF_LoadScriptFile(char *fname);
void S_ExitSystem(char *message);
void SOUND_Init(void);
void S_FrontEndCheck(void *data, int size);
void S_LoadSettings(void);
void S_SaveSettings(void);
int S_UpdateInput(void);
void S_InitialisePolyList(BOOL clear_back_buffer);
void S_CopyBufferToScreen(void);
void S_OutputPolyList(void);
int S_DumpScreen(void);
void FadeToPal(int nframes, uint8 *pal);
void S_Wait(int nframes, int input);
sint32 LevelCompleteSequence(int level_number);
sint32 DoDemoSequence(int level_number);
void StartCinematic(int level_number);
void ShutdownGame(void);
int TitleSequence(void);
}

extern "C" BOOL GameMain(void)
{
    int option;
    int param;
    int loop;
    int frontend;
    int level;

    HiRes = 0;
    GameMainReset = 0;
    screen_sizer = 1.0;
    game_sizer = 1.0;
    if (!S_InitialiseSystem())
        return FALSE;
    if (!GF_LoadScriptFile("data\\tombPC.DAT"))
        S_ExitSystem("GameMain: could not load script file");
    SOUND_Init();
    InitialiseStartInfo();
    S_FrontEndCheck(&savegame, sizeof(SAVEGAME_INFO));
    S_LoadSettings();
    HiRes = -1;
    malloc_buffer = (char *)GlobalAlloc(GMEM_FIXED, 0x380000);
    if (!malloc_buffer)
    {
        lstrcpy(StringToShow, "GameMain: could not allocate malloc_buffer");
        return FALSE;
    }
    HiRes = 0;
    TempVideoAdjust(1, 1.0);
    S_UpdateInput();
    IsVidModeLock = true;
    S_DisplayPicture("data\\legal.pcx", 0);
    S_InitialisePolyList(0);
    S_CopyBufferToScreen();
    S_OutputPolyList();
    S_DumpScreen();
    FadeToPal(30, game_palette);
    S_Wait(180, 1);
    S_FadeToBlack();
    S_DontDisplayPicture();
    IsVidModeLock = false;
    frontend = GF_DoFrontEndSequence();
    if (IsGameToExit)
        return TRUE;
    if (frontend == 1)
    {
        lstrcpy(StringToShow, "GameMain: failed in GF_DoFrontEndSequence()");
        return FALSE;
    }
    S_FadeToBlack();
    title_loaded = 0;
    option = gameflow.firstOption;
    loop = 1;
    while (loop)
    {
        param = option & 0xff;
        option &= ~0xff;
        switch (option)
        {
        case STARTGAME:
            if (gameflow.singlelevel >= 0)
                option = GF_DoLevelSequence(gameflow.singlelevel, IL_NORMAL);
            else
            {
                if (param > gameflow.num_levels)
                {
                    wsprintf(StringToShow, "GameMain: STARTGAME with invalid level number (%d)", param);
                    return FALSE;
                }
                option = GF_DoLevelSequence(param, IL_NORMAL);
            }
            break;

        case STARTSAVEDGAME:
            S_LoadGame(&savegame, sizeof(SAVEGAME_INFO), param);
            level = savegame.current_level;
            if (level > gameflow.num_levels)
            {
                wsprintf(StringToShow, "GameMain: STARTSAVEDGAME with invalid level number (%d)", level);
                return FALSE;
            }
            option = GF_DoLevelSequence(level, IL_SAVED);
            break;

        case STARTCINE:
            StartCinematic(param);
            option = EXIT_TO_TITLE;
            break;

        case STARTDEMO:
            option = DoDemoSequence(-1);
            break;

        case LEVELCOMPLETE:
            option = LevelCompleteSequence(param);
            break;

        case EXIT_TO_TITLE:
        case EXIT_TO_OPTION:
            if (!gameflow.title_disabled)
            {
                option = TitleSequence();
                GF_StartGame = true;
            }
            else
            {
                option = gameflow.title_replace;
                if (option < 0 || option == EXIT_TO_TITLE)
                {
                    lstrcpy(StringToShow, "GameMain Failed: Title disabled & no replacement");
                    return FALSE;
                }
            }
            break;

        case EXITGAME:
            loop = 0;
            break;

        default:
            loop = 0;
            break;
        }
    }
    S_SaveSettings();
    ShutdownGame();
    return TRUE;
}

extern "C" int TitleSequence(void)
{
    T_InitPrint();
    TempVideoAdjust(1, 1.0);
    NoInputCounter = 0;
    if (!title_loaded)
    {
        if (!InitialiseLevel(0, 0))
            return EXITGAME;
        title_loaded = 1;
    }
    S_DisplayPicture("data\\title.pcx", 1);
    if (gameflow.title_track)
        S_CDPlay(gameflow.title_track, 1);
    Display_Inventory(1);
    S_FadeToBlack();
    S_DontDisplayPicture();
    S_CDStop();
    if (IsResetFlag)
    {
        IsResetFlag = 0;
        return STARTDEMO;
    }
    if (InventoryChosen == PHOTO_OPTION)
        return 0;
    if (InventoryChosen == PASSPORT_OPTION)
    {
        if (InventoryExtraData[0] == 0)
        {
            Inv_RemoveAllItems();
            S_LoadGame(&savegame, sizeof(SAVEGAME_INFO), InventoryExtraData[1]);
            return InventoryExtraData[1] | STARTSAVEDGAME;
        }
        if (InventoryExtraData[0] == 1)
        {
            InitialiseStartInfo();
            if (gameflow.play_any_level)
                return InventoryExtraData[1] + 1;
            return 1;
        }
    }
    return EXITGAME;
}

extern "C" void S_LoadSettings(void)
{
    DWORD value;

    OpenGameRegistryKey("Game");
    GetRegistryDwordValue("MusicVolume", &value, 165);
    Option_Music_Volume = (sint16)value;
    GetRegistryDwordValue("SoundFXVolume", &value, 10);
    Option_SoundFX_Volume = (sint16)value;
    GetRegistryDwordValue("DetailLevel", (DWORD *)&detail_level, 1);
    GetRegistryFloatValue("Sizer", &game_sizer, 1.0);
    GetRegistryBinaryValue("Layout", (LPBYTE)layout[1], sizeof(layout[1]), NULL);
    CloseGameRegistryKey();
    S_SoundSetMasterVolume(6 * Option_SoundFX_Volume + 4);
    if (Option_Music_Volume)
        S_CDVolume(25 * Option_Music_Volume + 5);
    else
        S_CDVolume(0);
}
