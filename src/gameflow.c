#include "game.h"

sint32 GF_DoFrontEndSequence(void)
{
    sint32 option;

    option = GF_InterpretSequence(GF_frontendSequence, IL_NORMAL, 1);
    if (option == EXITGAME)
        return 1;
    return 0;
}

sint32 GF_DoLevelSequence(int level, int type)
{
    sint16 *ptr;
    sint32 option;

    do
    {
        if (level > gameflow.num_levels - 1)
        {
            title_loaded = 0;
            return EXIT_TO_TITLE;
        }
        ptr = GF_level_sequence_list[level];
        option = GF_InterpretSequence(ptr, type, 0);
        level++;
        if (gameflow.singlelevel >= 0)
            return option;
    } while ((option & ~0xff) == LEVELCOMPLETE);
    return option;
}
