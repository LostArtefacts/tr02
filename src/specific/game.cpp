#include "global.h"
#include "../title.h"

extern "C" sint32 rand_1;
extern "C" sint32 rand_2;

sint32 LevelCompleteSequence(int level_number)
{
    return EXIT_TO_TITLE;
}

void SeedRandomControl(sint32 seed)
{
    rand_1 = seed;
}

void SeedRandomDraw(sint32 seed)
{
    rand_2 = seed;
}
