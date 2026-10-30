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

sint32 LevelCompleteSequence(int level_number)
{
    return EXIT_TO_TITLE;
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
