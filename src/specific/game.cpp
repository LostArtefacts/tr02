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
