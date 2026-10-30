#include "global.h"
#include "../title.h"
#include "../invdata.h"
#include "../savegame.h"
#include "../gameflow.h"
#include "file.h"

extern "C" sint32 rand_1;
extern "C" sint32 rand_2;

extern "C" int SavedGames;
extern int save_counter;
extern "C" sint16 saved_levels[];

extern "C" uint32 CurrentLevel;
extern "C" int title_loaded;
extern "C" int level_complete;
extern "C" sint16 InventoryChosen;
extern "C" int InventoryExtraData[];
extern "C" SAVEGAME_INFO savegame;
extern "C" void ModifyStartInfo(int level_number);
extern "C" void InitialiseLevelFlags(void);
extern "C" int InitialiseLevel(int level_number, int type);
extern "C" sint32 GameLoop(int demo_mode);
extern "C" void S_FadeToBlack(void);
extern "C" void S_FadeInInventory(int fade);
extern "C" int S_LoadGame(void *data, int size, int slot);

extern "C" int OverlayStatus;
extern "C" int NoInputCounter;
extern "C" int GameMode;
extern "C" bool IsGameToExit;
extern "C" sint16 Option_Music_Volume;
extern "C" void InitialiseCamera(void);
extern "C" sint32 ControlPhase(sint32 nframes, int demo_mode);
extern "C" int DrawPhaseGame(void);
extern "C" void S_SoundStopAllSamples(void);
extern "C" void S_CDStop(void);
extern "C" void S_CDVolume(int volume);

extern "C" sint32 StartGame(int level_number, int type)
{
    sint32 result;

    if (type == IL_NORMAL || type == IL_SAVED || type == IL_DEMO)
        CurrentLevel = level_number;
    if (type != IL_SAVED)
        ModifyStartInfo(level_number);
    title_loaded = 0;
    if (type != IL_SAVED)
        InitialiseLevelFlags();
    if (!InitialiseLevel(level_number, type)) {
        CurrentLevel = 0;
        return EXITGAME;
    }
    result = GameLoop(0);
    if (result == EXIT_TO_TITLE || result == STARTDEMO)
        return result;
    if (result == EXITGAME) {
        CurrentLevel = 0;
        return EXITGAME;
    }
    if (level_complete) {
        if (gameflow.demoversion && gameflow.singlelevel)
            return EXIT_TO_TITLE;
        if (CurrentLevel == LV_GYM) {
            S_FadeToBlack();
            return EXIT_TO_TITLE;
        }
        S_FadeInInventory(1);
        return LEVELCOMPLETE | CurrentLevel;
    }
    S_FadeToBlack();
    if (!InventoryChosen)
        return EXIT_TO_TITLE;
    if (InventoryExtraData[0] == 0) {
        S_LoadGame(&savegame, sizeof(SAVEGAME_INFO), InventoryExtraData[1]);
        return STARTSAVEDGAME | InventoryExtraData[1];
    }
    if (InventoryExtraData[0] == 1) {
        if (gameflow.play_any_level)
            return STARTGAME | (InventoryExtraData[1] + 1);
        return STARTGAME | LV_FIRSTLEVEL;
    }
    return EXIT_TO_TITLE;
}

extern "C" sint32 GameLoop(int demo_mode)
{
    sint32 result;
    int nframes;

    OverlayStatus = 1;
    InitialiseCamera();
    NoInputCounter = 0;
    GameMode = demo_mode ? 2 : 1;
    result = ControlPhase(1, demo_mode);
    while (!result) {
        nframes = DrawPhaseGame();
        if (IsGameToExit)
            result = EXITGAME;
        else
            result = ControlPhase(nframes, demo_mode);
    }
    GameMode = 0;
    S_SoundStopAllSamples();
    S_CDStop();
    if (Option_Music_Volume)
        S_CDVolume(Option_Music_Volume * 25 + 5);
    return result;
}

sint32 LevelCompleteSequence(int level_number)
{
    return EXIT_TO_TITLE;
}

extern "C" sint32 input;
extern "C" sint32 inputDB;
extern "C" int IsResetFlag;
extern "C" void T_InitPrint(void);
extern "C" void T_DrawText(void);
extern "C" int S_UpdateInput(void);
extern "C" void S_InitialisePolyList(int clear);
extern "C" void S_CopyBufferToScreen(void);
extern "C" sint32 GetDebouncedInput(sint32 in);
extern "C" void ShowEndStatsText(void);
extern "C" void S_OutputPolyList(void);
extern "C" int S_DumpScreen(void);
extern "C" void S_DontDisplayPicture(void);

extern "C" int GameStats(int level_number)
{
    int n;

    savegame.start[CurrentLevel].timer = savegame.timer;
    savegame.start[CurrentLevel].ammo_used = savegame.ammo_used;
    savegame.start[CurrentLevel].ammo_hit = savegame.ammo_hit;
    savegame.start[CurrentLevel].distance_travelled = savegame.distance_travelled;
    savegame.start[CurrentLevel].kills = savegame.kills;
    savegame.start[CurrentLevel].secrets_found = savegame.secrets;
    savegame.start[CurrentLevel].health_used = savegame.health_used;
    T_InitPrint();
    while (input & IN_SELECT)
        S_UpdateInput();
    for (;;) {
        S_InitialisePolyList(0);
        S_CopyBufferToScreen();
        S_UpdateInput();
        if (IsResetFlag)
            input = IN_SELECT;
        inputDB = GetDebouncedInput(input);
        ShowEndStatsText();
        T_DrawText();
        S_OutputPolyList();
        S_DumpScreen();
        if (input & IN_SELECT)
            break;
    }
    savegame.bonus_flag = 1;
    for (n = LV_FIRSTLEVEL; n <= gameflow.num_levels; n++)
        ModifyStartInfo(n);
    savegame.current_level = LV_FIRSTLEVEL;
    S_DontDisplayPicture();
    return 0;
}

sint32 GetRandomControl(void)
{
    rand_1 = rand_1 * 0x41c64e6d + 0x3039;
    return (rand_1 >> 10) & 0x7fff;
}

void SeedRandomControl(sint32 seed)
{
    rand_1 = seed;
}

sint32 GetRandomDraw(void)
{
    rand_2 = rand_2 * 0x41c64e6d + 0x3039;
    return (rand_2 >> 10) & 0x7fff;
}

void SeedRandomDraw(sint32 seed)
{
    rand_2 = seed;
}

void GetValidLevelsList(REQUEST_INFO *req)
{
    int i;

    RemoveAllReqItems(req);
    for (i = 1; i < (gameflow.num_levels); i++)
    {
        AddRequesterItem(req, GF_Level_Names[i], R_CENTRE, NULL, 0);
    }
}

void GetSavedGamesList(REQUEST_INFO *req)
{
    SetPCRequesterSize(req, 10, -32);
    if (req->selected >= req->vis_lines)
        req->line_offset = req->selected - req->vis_lines + 1;
    memcpy(RequesterFlags1, SaveGameReqFlags1, MAXIMUM_LEVELS * sizeof(uint32));
    memcpy(RequesterFlags2, SaveGameReqFlags2, MAXIMUM_LEVELS * sizeof(uint32));
}

int S_FrontEndCheck(void *data, int size)
{
    int i;
    REQUEST_INFO *req;
    int count;
    HANDLE file;
    char filename[80], count_str[16];
    DWORD read;

    req = &Load_Game_Requester;
    Init_Requester(req);
    SavedGames = 0;
    for (i = 0; i < 16; i++)
    {
        wsprintf(filename, "savegame.%d", i);
        file = CreateFile(filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (file != INVALID_HANDLE_VALUE)
        {
            ReadFile(file, filename, 75, &read, NULL);
            ReadFile(file, &count, sizeof(sint32), &read, NULL);
            CloseHandle(file);
            wsprintf(count_str, "%d", count);
            AddRequesterItem(req, filename, R_LEFTALIGN, count_str, R_RIGHTALIGN);
            if (count > save_counter)
            {
                save_counter = count;
                req->selected = i;
            }
            saved_levels[i] = 1;
            SavedGames++;
        }
        else
        {
            AddRequesterItem(req, GF_PCStrings[PCSTR_SAVESLOT], R_CENTRE, 0, 0);
            saved_levels[i] = 0;
        }
    }
    save_counter++;
    memcpy(SaveGameReqFlags1, RequesterFlags1, MAXIMUM_LEVELS * sizeof(uint32));
    memcpy(SaveGameReqFlags2, RequesterFlags2, MAXIMUM_LEVELS * sizeof(uint32));
    return 1;
}

int S_SaveGame(void *data, int size, int slot)
{
    HANDLE file;
    char filename[80], count_str[16];
    DWORD write;

    wsprintf(filename, "savegame.%d", slot);
    file = CreateFile(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file != INVALID_HANDLE_VALUE)
    {
        wsprintf(filename, "%s", GF_Level_Names[savegame.current_level]);
        WriteFile(file, filename, 75, &write, NULL);
        WriteFile(file, &save_counter, sizeof(sint32), &write, NULL);
        WriteFile(file, data, size, &write, NULL);
        CloseHandle(file);
        wsprintf(count_str, "%d", save_counter);
        ChangeRequesterItem(&Load_Game_Requester, slot, filename, R_LEFTALIGN, count_str, R_RIGHTALIGN);
        saved_levels[slot] = 1;
        save_counter++;
        SavedGames++;
        return 1;
    }
    return 0;
}

int S_LoadGame(void *data, int size, int slot)
{
    HANDLE file;
    char filename[80];
    sint32 count;
    DWORD read;

    wsprintf(filename, "savegame.%d", slot);
    file = CreateFile(filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file != INVALID_HANDLE_VALUE)
    {
        ReadFile(file, filename, 75, &read, NULL);
        ReadFile(file, &count, sizeof(sint32), &read, NULL);
        ReadFile(file, data, size, &read, NULL);
        CloseHandle(file);
        return 1;
    }
    return 0;
}
