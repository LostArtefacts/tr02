#include "game.h"

void finish_level_effect(ITEM_INFO *item)
{
    level_complete = 1;
}

void lara_hands_free(ITEM_INFO *item)
{
    lara.gun_status = LG_ARMLESS;
}
