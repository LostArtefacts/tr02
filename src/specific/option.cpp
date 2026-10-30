#include "global.h"
#include "../3dglodef.h"
#include "../invdata.h"
#include "input.h"
#include <stdio.h>
#include "../savegame.h"
#include "../title.h"
#include "../room.h"
#include "../sound.h"

extern "C" sint16 Option_SoundFX_Volume;
extern "C" sint16 Option_Music_Volume;
extern "C" int detail_level;
extern "C" void S_CDVolume(int volume);

extern TEXTSTRING *dtext[5];

#define DWIDTH 160

void do_detail_option(INVENTORY_ITEM *inv_item)
{
    int i;

    if (dtext[0] == NULL)
    {
        dtext[2] = T_Print(0, 0, 0, GF_GameStrings[GT_HIGH_DETAIL]);
        dtext[1] = T_Print(0, 25, 0, GF_GameStrings[GT_MEDIUM_DETAIL]);
        dtext[0] = T_Print(0, 50, 0, GF_GameStrings[GT_LOW_DETAIL]);
        dtext[3] = T_Print(0, -32, 0, " ");
        dtext[4] = T_Print(0, -30, 0, GF_GameStrings[GT_DETAIL]);
        T_AddBackground(dtext[4], DWIDTH - 4, 0, 0, 0, 8, C_BLACK, NULL, 0);
        T_AddOutline(dtext[4], 1, C_ORANGE, NULL, 0);
        T_AddBackground(dtext[detail_level], DWIDTH - 12, 0, 0, 0, 8, C_BLACK, NULL, 0);
        T_AddOutline(dtext[detail_level], 1, C_ORANGE, NULL, 0);
        T_AddBackground(dtext[3], DWIDTH, 107, 0, 0, 16, C_BLACK, NULL, 0);
        T_AddOutline(dtext[3], 1, C_BLUE, NULL, 0);
        for (i = 0; i < 5; i++)
        {
            T_CentreH(dtext[i], 1);
            T_CentreV(dtext[i], 1);
        }
    }
    if ((inputDB & IN_BACK) && detail_level > 0)
    {
        T_RemoveOutline(dtext[detail_level]);
        T_RemoveBackground(dtext[detail_level]);
        detail_level--;
        T_AddOutline(dtext[detail_level], 1, C_ORANGE, NULL, 0);
        T_AddBackground(dtext[detail_level], DWIDTH - 12, 0, 0, 0, 8, C_BLACK, NULL, 0);
    }
    if ((inputDB & IN_FORWARD) && detail_level < 2)
    {
        T_RemoveOutline(dtext[detail_level]);
        T_RemoveBackground(dtext[detail_level]);
        detail_level++;
        T_AddOutline(dtext[detail_level], 1, C_ORANGE, NULL, 0);
        T_AddBackground(dtext[detail_level], DWIDTH - 12, 0, 0, 0, 8, C_BLACK, NULL, 0);
    }
    switch (detail_level)
    {
    case 0:
        perspective_distance = 0 * 1024 * W2V_SCALE;
        break;
    case 1:
        perspective_distance = 3 * 1024 * W2V_SCALE;
        break;
    case 2:
        perspective_distance = 20 * 1024 * W2V_SCALE;
    }
    if (inputDB & (SELECT_ITEM | DESELECT_ITEM))
    {
        for (i = 0; i < 5; i++)
        {
            T_RemovePrint(dtext[i]);
            dtext[i] = NULL;
        }
    }
}

extern TEXTSTRING *sa_text[4];

#define SAWIDTH 140

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
