#include "game.h"

sint32 DoDemoSequence(int level_number)
{
    static int demo_level = 0;

    if (level_number < 0 && !gameflow.num_demos)
        return EXIT_TO_TITLE;
    if (level_number < 0)
    {
        if (demo_level >= gameflow.num_demos)
            demo_level = 0;
        level_number = GF_valid_demos[demo_level];
        demo_level++;
    }
    else
        demo_level = level_number;
    return GF_DoLevelSequence(level_number, IL_DEMO);
}
