#include "../3dglodef.h"
#include "../invdata.h"
#include "input.h"
#include <stdio.h>
#include "../savegame.h"
#include "../title.h"
#include "../room.h"
#include "../sound.h"

void do_compass_option(INVENTORY_ITEM *inv_item)
{
    int h = 0, m = 0, s = 0;
    char time_str[32];

    s = savegame.timer / 30;
    h = s / 3600;
    m = (s / 60) % 60;
    s = s % 60;
    sprintf(time_str, "%02d:%02d:%02d", h, m, s);
    if (CurrentLevel == LV_GYM)
        ShowGymStatsText(time_str, 1);
    else
        ShowStatsText(time_str, 1);
    if (inputDB & (DESELECT_ITEM | SELECT_ITEM))
    {
        inv_item->goal_frame = inv_item->frames_total - 1;
        inv_item->anim_direction = 1;
    }
    SoundEffect(113, NULL, SFX_ALWAYS);
}

void DefaultConflict(void)
{
    int i, j;
    sint16 key;

    for (i = 0; i < 13; i++)
    {
        key = layout[0][i];
        conflict[i] = 0;
        for (j = 0; j < 13; j++)
        {
            if (key == layout[1][j])
            {
                conflict[i] = 1;
                break;
            }
        }
    }
}

int GetRenderHeight(void)
{
    return phd_winheight;
}

int GetRenderWidth(void)
{
    return phd_winwidth;
}
