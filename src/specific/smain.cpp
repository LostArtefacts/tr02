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

#define EXIT_TO_TITLE 0x400
#define EXIT_TO_OPTION 0x700
#define STARTSAVEDGAME 0x100
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

extern "C" int TitleSequence(void)
{
    T_InitPrint();
    TempVideoAdjust(1, 1.0);
    NoInputCounter = 0;
    if (!title_loaded)
    {
        if (!InitialiseLevel(0, 0))
            return EXIT_TO_OPTION;
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
        return EXIT_TO_TITLE;
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
    return EXIT_TO_OPTION;
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
