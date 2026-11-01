#include "global.h"
#include "../3dglodef.h"
#include "../invdata.h"
#include "input.h"
#include <stdio.h>
#include <dinput.h>
#include "../savegame.h"
#include "../title.h"
#include "../room.h"
#include "../sound.h"

extern "C" sint16 Option_SoundFX_Volume;
extern "C" sint16 Option_Music_Volume;
extern "C" int detail_level;
extern "C" void S_CDVolume(int volume);

extern TEXTSTRING *dtext[5];
extern TEXTSTRING *ctext[];
extern TEXTSTRING *btext[];

#define NUM_CONTROLS 14
#define PCSTR_DEFAULTKEYS 19

extern "C" int iconfig;
extern "C" char *kname[];
extern "C" char BadKeyString[];
extern "C" TEXTSTRING *ControlTextInfo[2];
extern "C" void T_FlashText(TEXTSTRING *textString, sint16 b, sint16 rate);

#define DWIDTH 160

#define PASSPORT_OPTION 120
#define MAP_OPTION 121
#define DETAIL_OPTION 153
#define SOUND_OPTION 154
#define CONTROL_OPTION 155
#define GAMMA_OPTION 156

extern "C" void do_passport_option(INVENTORY_ITEM *inv_item);
extern "C" void do_gamma_option(INVENTORY_ITEM *inv_item);
extern "C" void do_control_option(INVENTORY_ITEM *inv_item);

extern "C" void do_inventory_options(INVENTORY_ITEM *inv_item)
{
    switch (inv_item->object_number)
    {
    case MAP_OPTION:
        do_compass_option(inv_item);
        break;
    case 157:
    case 158:
    case 159:
    case 160:
    case 161:
    case 162:
    case 163:
        inputDB |= SELECT_ITEM;
        break;
    case 164:
    case 165:
    case 166:
    case 167:
    case 168:
    case 169:
    case 170:
        break;
    case 171:
    case 172:
    case 178:
    case 179:
    case 180:
    case 181:
    case 197:
    case 198:
    case 199:
    case 200:
    case 207:
    case 208:
        inputDB |= SELECT_ITEM;
        break;
    case PASSPORT_OPTION:
        do_passport_option(inv_item);
        break;
    case GAMMA_OPTION:
        do_gamma_option(inv_item);
        break;
    case DETAIL_OPTION:
        do_detail_option(inv_item);
        break;
    case SOUND_OPTION:
        do_sound_option(inv_item);
        break;
    case CONTROL_OPTION:
        do_control_option(inv_item);
        break;
    default:
        if (inputDB & (SELECT_ITEM | DESELECT_ITEM))
        {
            inv_item->goal_frame = 0;
            inv_item->anim_direction = -1;
        }
        break;
    }
}

#define IT_NAME 0

extern "C" TEXTSTRING *Inv_itemText[];
extern "C" void T_BottomAlign(TEXTSTRING *textString, sint16 ok);
extern "C" TEXTSTRING *Inv_levelText;
extern "C" TEXTSTRING *Inv_tagText;
extern "C" TEXTSTRING *Inv_ringText;
extern "C" TEXTSTRING *PassportTextInfo;
extern "C" sint32 passport_mode;
extern "C" sint32 InventoryExtraData[];
extern "C" int SavedGames;
extern "C" REQUEST_INFO SaveGameRequester;
extern "C" void SetRequesterHeading(REQUEST_INFO *req, char *text1, uint32 flags1, char *text2, uint32 flags2);
extern "C" void Remove_Requester(REQUEST_INFO *req);
extern "C" sint32 Display_Requester(REQUEST_INFO *req, int des, int backgrounds);

void do_passport_option(INVENTORY_ITEM *inv_item)
{
    sint32 frame;
    sint16 page;
    sint32 select;

    T_RemovePrint(Inv_itemText[IT_NAME]);
    Inv_itemText[IT_NAME] = NULL;

    frame = inv_item->open_frame;
    frame = inv_item->goal_frame - frame;
    page = frame / 5;
    if (frame % 5)
        page = -1;

    if (Inventory_Mode == INV_LOAD_MODE || Inventory_Mode == INV_SAVE_MODE || gameflow.loadsave_disabled)
        inputDB &= ~(IN_LEFT | IN_RIGHT);

    switch (page)
    {
    case 0:
        if (gameflow.loadsave_disabled)
        {
            inputDB = IN_RIGHT;
            break;
        }
        if (passport_mode == 1)
        {
            SetPCRequesterSize(&Load_Game_Requester, 10, -32);
            select = Display_Requester(&Load_Game_Requester, 1, 1);
            if (select)
            {
                if (select > 0)
                    InventoryExtraData[1] = select - 1;
                passport_mode = 0;
            }
            else if (inputDB & IN_RIGHT)
            {
                Remove_Requester(&Load_Game_Requester);
                passport_mode = 0;
            }
            else
                input = inputDB = 0;
        }
        else if (passport_mode == 0)
        {
            if (!SavedGames || Inventory_Mode == INV_SAVE_MODE)
                inputDB = IN_RIGHT;
            else
            {
                if (!PassportTextInfo)
                {
                    PassportTextInfo = T_Print(0, -16, 0, GF_GameStrings[GT_LOADGAME]);
                    T_BottomAlign(PassportTextInfo, 1);
                    T_CentreH(PassportTextInfo, 1);
                }
                T_RemovePrint(Inv_levelText);
                Inv_levelText = NULL;
                T_RemovePrint(Inv_tagText);
                Inv_tagText = NULL;
                T_RemovePrint(Inv_ringText);
                Inv_ringText = NULL;
                T_RemovePrint(Inv_itemText[IT_NAME]);
                Inv_itemText[IT_NAME] = NULL;
                GetSavedGamesList(&Load_Game_Requester);
                SetRequesterHeading(&Load_Game_Requester, GF_GameStrings[GT_LOADGAME], R_CENTRE, NULL, 0);
                passport_mode = 1;
                input = 0;
                inputDB = 0;
            }
        }
        break;
    case 1:
        if (gameflow.loadsave_disabled)
        {
            inputDB = IN_RIGHT;
            break;
        }
        if (passport_mode == 1 || passport_mode == 2)
        {
            if (passport_mode == 1)
            {
                SetPCRequesterSize(&Load_Game_Requester, 10, -32);
                select = Display_Requester(&Load_Game_Requester, 1, 1);
            }
            else
            {
                SetPCRequesterSize(&SaveGameRequester, 10, -32);
                select = Display_Requester(&SaveGameRequester, 1, 1);
            }
            if (select)
            {
                if (select > 0)
                    InventoryExtraData[1] = select - 1;
                passport_mode = 0;
            }
            else if (inputDB & (IN_LEFT | IN_RIGHT))
            {
                if (passport_mode == 1)
                    Remove_Requester(&Load_Game_Requester);
                else
                    Remove_Requester(&SaveGameRequester);
                passport_mode = 0;
            }
            else
                input = inputDB = 0;
        }
        else if (passport_mode == 0)
        {
            if (Inventory_Mode == INV_DEATH_MODE)
            {
                if (inv_item->anim_direction == -1)
                    inputDB = IN_LEFT;
                else
                    inputDB = IN_RIGHT;
            }
            else
            {
                if (!PassportTextInfo)
                {
                    if (Inventory_Mode == INV_TITLE_MODE || CurrentLevel == LV_GYM)
                        PassportTextInfo = T_Print(0, -16, 0, GF_GameStrings[GT_STARTGAME]);
                    else
                        PassportTextInfo = T_Print(0, -16, 0, GF_GameStrings[GT_SAVEGAME]);
                    T_BottomAlign(PassportTextInfo, 1);
                    T_CentreH(PassportTextInfo, 1);
                }
                if (Inventory_Mode == INV_TITLE_MODE || CurrentLevel == LV_GYM)
                {
                    if (gameflow.play_any_level)
                    {
                        T_RemovePrint(Inv_levelText);
                        Inv_levelText = NULL;
                        T_RemovePrint(Inv_tagText);
                        Inv_tagText = NULL;
                        T_RemovePrint(Inv_itemText[IT_NAME]);
                        Inv_itemText[IT_NAME] = NULL;
                        Init_Requester(&SaveGameRequester);
                        GetValidLevelsList(&SaveGameRequester);
                        SetRequesterHeading(&SaveGameRequester, GF_GameStrings[GT_SELECTLEVEL], R_CENTRE, NULL, 0);
                        passport_mode = 2;
                        input = 0;
                        inputDB = 0;
                    }
                    else if (inputDB & SELECT_ITEM)
                        InventoryExtraData[1] = LV_FIRSTLEVEL;
                }
                else
                {
                    T_RemovePrint(Inv_levelText);
                    Inv_levelText = NULL;
                    T_RemovePrint(Inv_tagText);
                    Inv_tagText = NULL;
                    T_RemovePrint(Inv_ringText);
                    Inv_ringText = NULL;
                    T_RemovePrint(Inv_itemText[IT_NAME]);
                    Inv_itemText[IT_NAME] = NULL;
                    GetSavedGamesList(&Load_Game_Requester);
                    SetRequesterHeading(&Load_Game_Requester, GF_GameStrings[GT_SAVEGAME], R_CENTRE, NULL, 0);
                    passport_mode = 1;
                    input = 0;
                    inputDB = 0;
                }
            }
        }
        break;
    case 2:
        if (!PassportTextInfo)
        {
            if (Inventory_Mode == INV_TITLE_MODE)
                PassportTextInfo = T_Print(0, -16, 0, GF_GameStrings[GT_EXITGAME]);
            else if (!gameflow.demoversion)
                PassportTextInfo = T_Print(0, -16, 0, GF_GameStrings[GT_EXIT2TITLE]);
            else
                PassportTextInfo = T_Print(0, -16, 0, GF_GameStrings[GT_EXITDEMO]);
            T_BottomAlign(PassportTextInfo, 1);
            T_CentreH(PassportTextInfo, 1);
        }
        break;
    }

    if (inputDB & IN_LEFT)
    {
        if (Inventory_Mode != INV_DEATH_MODE || SavedGames)
        {
            inv_item->anim_direction = -1;
            inv_item->goal_frame -= 5;
            if (!SavedGames)
            {
                if (inv_item->goal_frame < inv_item->open_frame + 5)
                    inv_item->goal_frame = inv_item->open_frame + 5;
                else
                {
                    T_RemovePrint(PassportTextInfo);
                    PassportTextInfo = NULL;
                }
            }
            else
            {
                if (inv_item->goal_frame < inv_item->open_frame)
                    inv_item->goal_frame = inv_item->open_frame;
                else
                {
                    SoundEffect(115, NULL, SFX_ALWAYS);
                    T_RemovePrint(PassportTextInfo);
                    PassportTextInfo = NULL;
                }
            }
            input = 0;
            inputDB = 0;
        }
    }

    if (inputDB & IN_RIGHT)
    {
        inv_item->anim_direction = 1;
        inv_item->goal_frame += 5;
        if (inv_item->goal_frame > inv_item->frames_total - 6)
            inv_item->goal_frame = inv_item->frames_total - 6;
        else
        {
            SoundEffect(115, NULL, SFX_ALWAYS);
            T_RemovePrint(PassportTextInfo);
            PassportTextInfo = NULL;
        }
        input = 0;
        inputDB = 0;
    }

    if (inputDB & DESELECT_ITEM)
    {
        if (Inventory_Mode == INV_DEATH_MODE)
        {
            input = 0;
            inputDB = 0;
        }
        else
        {
            if (page == 2)
            {
                inv_item->goal_frame = inv_item->frames_total - 1;
                inv_item->anim_direction = 1;
            }
            else
            {
                inv_item->goal_frame = 0;
                inv_item->anim_direction = -1;
            }
            T_RemovePrint(PassportTextInfo);
            PassportTextInfo = NULL;
        }
    }

    if (inputDB & SELECT_ITEM)
    {
        InventoryExtraData[0] = page;
        if (page == 2)
        {
            inv_item->goal_frame = inv_item->frames_total - 1;
            inv_item->anim_direction = 1;
        }
        else
        {
            inv_item->goal_frame = 0;
            inv_item->anim_direction = -1;
        }
        T_RemovePrint(PassportTextInfo);
        PassportTextInfo = NULL;
    }
}

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

void do_sound_option(INVENTORY_ITEM *inv_item)
{
    int i;
    char string[20];

    if (!sa_text[0])
    {
        if (Option_Music_Volume > 10)
            Option_Music_Volume = 10;
        wsprintf(string, "| %2d", Option_Music_Volume);
        sa_text[0] = T_Print(0, 0, 0, string);
        T_AddBackground(sa_text[0], SAWIDTH - 12, 0, 0, 0, 8, C_BLACK, NULL, 0);
        T_AddOutline(sa_text[0], 1, C_ORANGE, NULL, 0);
        if (Option_SoundFX_Volume > 10)
            Option_SoundFX_Volume = 10;
        wsprintf(string, "} %2d", Option_SoundFX_Volume);
        sa_text[1] = T_Print(0, 25, 0, string);
        sa_text[2] = T_Print(0, -32, 0, " ");
        T_AddBackground(sa_text[2], SAWIDTH, 85, 0, 0, 48, C_BLACK, NULL, 0);
        T_AddOutline(sa_text[2], 1, C_BLUE, NULL, 0);
        sa_text[3] = T_Print(0, -30, 0, GF_PCStrings[PCSTR_SETVOLUME]);
        T_AddBackground(sa_text[3], SAWIDTH - 4, 0, 0, 0, 8, C_BLACK, NULL, 0);
        T_AddOutline(sa_text[3], 1, C_BLUE, NULL, 0);
        for (i = 0; i < 4; i++)
        {
            T_CentreH(sa_text[i], 1);
            T_CentreV(sa_text[i], 1);
        }
    }
    if ((inputDB & PREV_HBAR) && item_data > 0)
    {
        T_RemoveOutline(sa_text[item_data]);
        T_RemoveBackground(sa_text[item_data]);
        item_data--;
        T_AddBackground(sa_text[item_data], SAWIDTH - 12, 0, 0, 0, 8, C_BLACK, NULL, 0);
        T_AddOutline(sa_text[item_data], 1, C_ORANGE, NULL, 0);
    }
    if ((inputDB & NEXT_HBAR) && item_data < 1)
    {
        T_RemoveOutline(sa_text[item_data]);
        T_RemoveBackground(sa_text[item_data]);
        item_data++;
        T_AddBackground(sa_text[item_data], SAWIDTH - 12, 0, 0, 0, 8, C_BLACK, NULL, 0);
        T_AddOutline(sa_text[item_data], 1, C_ORANGE, NULL, 0);
    }
    if (!item_data)
    {
        if ((input & DEC_HBAR) && Option_Music_Volume > 0)
        {
            idelay = 1;
            idcount = 10;
            Option_Music_Volume--;
            wsprintf(string, "| %2d", Option_Music_Volume);
            T_ChangeText(sa_text[0], string);
            if (Option_Music_Volume)
                S_CDVolume(Option_Music_Volume * 25 + 5);
            else
                S_CDVolume(0);
            SoundEffect(115, NULL, SFX_ALWAYS);
        }
        else if ((input & INC_HBAR) && Option_Music_Volume < 10)
        {
            idelay = 1;
            idcount = 10;
            Option_Music_Volume++;
            wsprintf(string, "| %2d", Option_Music_Volume);
            T_ChangeText(sa_text[0], string);
            if (Option_Music_Volume)
                S_CDVolume(Option_Music_Volume * 25 + 5);
            else
                S_CDVolume(0);
            SoundEffect(115, NULL, SFX_ALWAYS);
        }
    }
    else
    {
        if ((input & DEC_HBAR) && Option_SoundFX_Volume > 0)
        {
            idelay = 1;
            idcount = 10;
            Option_SoundFX_Volume--;
            wsprintf(string, "} %2d", Option_SoundFX_Volume);
            T_ChangeText(sa_text[1], string);
            if (Option_SoundFX_Volume)
                S_SoundSetMasterVolume(Option_SoundFX_Volume * 6 + 4);
            else
                S_SoundSetMasterVolume(0);
            SoundEffect(115, NULL, SFX_ALWAYS);
        }
        else if ((input & INC_HBAR) && Option_SoundFX_Volume < 10)
        {
            idelay = 1;
            idcount = 10;
            Option_SoundFX_Volume++;
            wsprintf(string, "} %2d", Option_SoundFX_Volume);
            T_ChangeText(sa_text[1], string);
            if (Option_SoundFX_Volume)
                S_SoundSetMasterVolume(Option_SoundFX_Volume * 6 + 4);
            else
                S_SoundSetMasterVolume(0);
            SoundEffect(115, NULL, SFX_ALWAYS);
        }
    }
    is_sound_sfxlevel[6].param1 = Option_SoundFX_Volume;
    is_sound_sfxlevellow[6].param1 = Option_SoundFX_Volume;
    is_sound_muslevel[6].param1 = Option_Music_Volume;
    is_sound_muslevellow[6].param1 = Option_Music_Volume;
    if (inputDB & (DESELECT_ITEM | SELECT_ITEM))
    {
        T_RemovePrint(sa_text[0]);
        sa_text[0] = NULL;
        T_RemovePrint(sa_text[1]);
        sa_text[1] = NULL;
        T_RemovePrint(sa_text[2]);
        sa_text[2] = NULL;
        T_RemovePrint(sa_text[3]);
        sa_text[3] = NULL;
    }
}

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

extern "C" void FlashConflicts(void)
{
    int i, j;
    sint16 key;

    for (i = 0; i < NUM_CONTROLS; i++)
    {
        key = layout[iconfig][i];
        T_FlashText(btext[i], 0, 0);
        for (j = 0; j < NUM_CONTROLS; j++)
        {
            if (i == j)
                continue;
            if (key == layout[iconfig][j])
            {
                T_FlashText(btext[i], 1, 20);
                break;
            }
        }
    }
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

extern "C" int KeyCursor;
extern "C" void T_SetScale(TEXTSTRING *textString, sint32 scaleH, sint32 scaleV);
extern "C" int select_key;
extern "C" void S_ShowControls(void);
extern "C" void S_ChangeCtrlText(void);
void S_RemoveCtrlText(void);

#define JOY_KEY 256

void do_control_option(INVENTORY_ITEM *inv_item)
{
    sint16 key;

    if (!ControlTextInfo[0])
    {
        if (iconfig)
            ControlTextInfo[0] = T_Print(0, -50, 0, GF_PCStrings[PCSTR_USERKEYS]);
        else
            ControlTextInfo[0] = T_Print(0, -50, 0, GF_PCStrings[PCSTR_DEFAULTKEYS]);
        T_CentreH(ControlTextInfo[0], 1);
        T_CentreV(ControlTextInfo[0], 1);
        S_ShowControls();
        KeyCursor = -1;
        T_AddBackground(ControlTextInfo[0], 0, 0, 0, 0, 48, C_BLACK, NULL, 0);
        T_AddOutline(ControlTextInfo[0], 1, C_BLUE, NULL, 0);
    }

    switch (select_key)
    {
    case 0:
        if (inputDB & (IN_RIGHT | IN_LEFT))
        {
            if (KeyCursor == -1)
            {
                iconfig = !iconfig;
                S_ChangeCtrlText();
                FlashConflicts();
            }
            else
            {
                ctext[KeyCursor]->zpos = 16;
                T_RemoveBackground(ctext[KeyCursor]);
                T_RemoveOutline(ctext[KeyCursor]);
                if (KeyCursor <= 6)
                    KeyCursor += 7;
                else if (KeyCursor == 14)
                    KeyCursor = 7;
                else
                    KeyCursor -= 7;
                ctext[KeyCursor]->zpos = 0;
                T_AddBackground(ctext[KeyCursor], 0, 0, 0, 0, 0, C_BLACK, NULL, 0);
                T_AddOutline(ctext[KeyCursor], 1, C_BLUE, NULL, 0);
            }
        }
        else if ((inputDB & IN_DESELECT) || ((inputDB & IN_SELECT) && KeyCursor == -1))
        {
            T_RemovePrint(ControlTextInfo[0]);
            ControlTextInfo[0] = NULL;
            T_RemovePrint(ControlTextInfo[1]);
            ControlTextInfo[1] = NULL;
            S_RemoveCtrlText();
            DefaultConflict();
            return;
        }

        if (iconfig)
        {
            if (inputDB & IN_SELECT)
            {
                select_key = 1;
                ctext[KeyCursor]->zpos = 16;
                T_RemoveBackground(ctext[KeyCursor]);
                T_RemoveOutline(ctext[KeyCursor]);
                btext[KeyCursor]->zpos = 0;
                T_AddBackground(btext[KeyCursor], 0, 0, 0, 0, 0, C_BLACK, NULL, 0);
                T_AddOutline(btext[KeyCursor], 1, C_BLUE, NULL, 0);
            }
            else if (inputDB & IN_FORWARD)
            {
                if (KeyCursor == -1)
                {
                    T_RemoveBackground(ControlTextInfo[0]);
                    T_RemoveOutline(ControlTextInfo[0]);
                }
                else
                {
                    ctext[KeyCursor]->zpos = 16;
                    T_RemoveBackground(ctext[KeyCursor]);
                    T_RemoveOutline(ctext[KeyCursor]);
                }
                KeyCursor--;
                if (KeyCursor < -1)
                    KeyCursor = 13;
                if (KeyCursor == -1)
                {
                    T_AddBackground(ControlTextInfo[0], 0, 0, 0, 0, 0, C_BLACK, NULL, 0);
                    T_AddOutline(ControlTextInfo[0], 1, C_BLUE, NULL, 0);
                }
                else
                {
                    ctext[KeyCursor]->zpos = 0;
                    T_AddBackground(ctext[KeyCursor], 0, 0, 0, 0, 0, C_BLACK, NULL, 0);
                    T_AddOutline(ctext[KeyCursor], 1, C_BLUE, NULL, 0);
                }
            }
            else if (inputDB & IN_BACK)
            {
                if (KeyCursor == -1)
                {
                    T_RemoveBackground(ControlTextInfo[0]);
                    T_RemoveOutline(ControlTextInfo[0]);
                }
                else
                {
                    ctext[KeyCursor]->zpos = 16;
                    T_RemoveBackground(ctext[KeyCursor]);
                    T_RemoveOutline(ctext[KeyCursor]);
                }
                KeyCursor++;
                if (KeyCursor > 13)
                    KeyCursor = -1;
                if (KeyCursor == -1)
                {
                    T_AddBackground(ControlTextInfo[0], 0, 0, 0, 0, 0, C_BLACK, NULL, 0);
                    T_AddOutline(ControlTextInfo[0], 1, C_BLUE, NULL, 0);
                }
                else
                {
                    ctext[KeyCursor]->zpos = 0;
                    T_AddBackground(ctext[KeyCursor], 0, 0, 0, 0, 0, C_BLACK, NULL, 0);
                    T_AddOutline(ctext[KeyCursor], 1, C_BLUE, NULL, 0);
                }
            }
        }
        break;
    case 1:
        if (!(inputDB & IN_SELECT))
            select_key = 2;
        break;
    case 2:
        if (joy_fire)
        {
            for (key = 0; key < 32; key++)
                if ((1 << key) & joy_fire)
                    break;
            if (key == 32)
                key = 0;
            else
                key += JOY_KEY;
        }
        else
        {
            for (key = 0; key < 256; key++)
                if (rawkey(key))
                    break;
            if (key == 256)
                key = 0;
        }
        if (key && kname[key])
        {
            if (key == DIK_RETURN || key == DIK_LEFT || key == DIK_RIGHT || key == DIK_UP || key == DIK_DOWN)
                break;
            if (key != DIK_ESCAPE)
            {
                layout[iconfig][KeyCursor] = key;
                T_ChangeText(btext[KeyCursor], kname[key]);
            }
            btext[KeyCursor]->zpos = 16;
            T_RemoveBackground(btext[KeyCursor]);
            T_RemoveOutline(btext[KeyCursor]);
            ctext[KeyCursor]->zpos = 0;
            T_AddBackground(ctext[KeyCursor], 0, 0, 0, 0, 0, C_BLACK, NULL, 0);
            T_AddOutline(ctext[KeyCursor], 1, C_BLUE, NULL, 0);
            select_key = 3;
            FlashConflicts();
        }
        break;
    case 3:
        if (layout[iconfig][KeyCursor] & JOY_KEY)
        {
            if (!((1 << layout[iconfig][KeyCursor]) & joy_fire))
                select_key = 0;
        }
        else if (!rawkey(layout[iconfig][KeyCursor]))
        {
            select_key = 0;
            if (layout[iconfig][KeyCursor] == DIK_LCONTROL)
                layout[iconfig][KeyCursor] = DIK_RCONTROL;
            if (layout[iconfig][KeyCursor] == DIK_LSHIFT)
                layout[iconfig][KeyCursor] = DIK_RSHIFT;
            if (layout[iconfig][KeyCursor] == DIK_LALT)
                layout[iconfig][KeyCursor] = DIK_RALT;
            FlashConflicts();
        }
        break;
    }
    input = 0;
    inputDB = 0;
}

extern "C" void S_ShowControls(void)
{
    int i;
    int centre;
    sint16 vpos;
    sint16 *key;

    centre = GetRenderWidth() / 2;
    if (centre > 320)
        centre = 320;
    if (!btext[0])
    {
        key = layout[iconfig];
        if (centre < 320)
            vpos = centre - 140;
        else
            vpos = centre - 200;
        btext[0] = T_Print(vpos, -25, 16, kname[key[0]]);
        btext[1] = T_Print(vpos, -10, 16, kname[key[1]]);
        btext[2] = T_Print(vpos, 5, 16, kname[key[2]]);
        btext[3] = T_Print(vpos, 20, 16, kname[key[3]]);
        btext[4] = T_Print(vpos, 35, 16, kname[key[4]]);
        btext[5] = T_Print(vpos, 50, 16, kname[key[5]]);
        btext[6] = T_Print(vpos, 65, 16, kname[key[6]]);
        vpos = centre + 10;
        btext[7] = T_Print(vpos, -25, 16, kname[key[7]]);
        btext[8] = T_Print(vpos, -10, 16, kname[key[8]]);
        btext[9] = T_Print(vpos, 5, 16, kname[key[9]]);
        btext[10] = T_Print(vpos, 20, 16, kname[key[10]]);
        btext[11] = T_Print(vpos, 35, 16, kname[key[11]]);
        btext[12] = T_Print(vpos, 50, 16, kname[key[12]]);
        btext[13] = T_Print(vpos, 65, 16, kname[key[13]]);
        for (i = 0; i < NUM_CONTROLS; i++)
            T_CentreV(btext[i], 1);
        KeyCursor = 0;
    }
    if (!ctext[0])
    {
        if (centre < 320)
            vpos = centre - 70;
        else
            vpos = centre - 130;
        ctext[0] = T_Print(vpos, -25, 16, GF_GameStrings[19]);
        ctext[1] = T_Print(vpos, -10, 16, GF_GameStrings[22]);
        ctext[2] = T_Print(vpos, 5, 16, GF_GameStrings[20]);
        ctext[3] = T_Print(vpos, 20, 16, GF_GameStrings[21]);
        ctext[4] = T_Print(vpos, 35, 16, GF_GameStrings[23]);
        ctext[5] = T_Print(vpos, 50, 16, GF_GameStrings[25]);
        ctext[6] = T_Print(vpos, 65, 16, GF_GameStrings[17]);
        vpos = centre + 90;
        ctext[7] = T_Print(vpos, -25, 16, GF_GameStrings[28]);
        ctext[8] = T_Print(vpos, -10, 16, GF_GameStrings[29]);
        ctext[9] = T_Print(vpos, 5, 16, GF_GameStrings[30]);
        ctext[10] = T_Print(vpos, 20, 16, GF_GameStrings[33]);
        ctext[11] = T_Print(vpos, 35, 16, GF_GameStrings[27]);
        ctext[12] = T_Print(vpos, 50, 16, GF_GameStrings[18]);
        ctext[13] = T_Print(vpos, 65, 16, GF_GameStrings[32]);
        for (i = 0; i < NUM_CONTROLS; i++)
            T_CentreV(ctext[i], 1);
    }
    ControlTextInfo[1] = T_Print(0, -55, 0, " ");
    T_CentreV(ControlTextInfo[1], 1);
    T_CentreH(ControlTextInfo[1], 1);
    T_AddOutline(ControlTextInfo[1], 1, 15, NULL, 0);
    if (centre < 320)
    {
        for (i = 0; i < NUM_CONTROLS; i++)
        {
            T_SetScale(btext[i], 0x8000, 0x10000);
            T_SetScale(ctext[i], 0x8000, 0x10000);
        }
        T_AddBackground(ControlTextInfo[1], 300, 140, 0, 0, 48, 0, NULL, 0);
    }
    else
        T_AddBackground(ControlTextInfo[1], 420, 150, 0, 0, 48, 0, NULL, 0);
}

extern "C" void S_ChangeCtrlText(void)
{
    int i;
    char str[40];

    if (iconfig)
    {
        wsprintf(str, GF_PCStrings[PCSTR_USERKEYS], iconfig);
        T_ChangeText(ControlTextInfo[0], str);
    }
    else
        T_ChangeText(ControlTextInfo[0], GF_PCStrings[PCSTR_DEFAULTKEYS]);
    for (i = 0; i < NUM_CONTROLS; i++)
    {
        if (kname[layout[iconfig][i]])
            T_ChangeText(btext[i], kname[layout[iconfig][i]]);
        else
            T_ChangeText(btext[i], BadKeyString);
    }
}

void S_RemoveCtrlText(void)
{
    int i;

    for (i = 0; i < NUM_CONTROLS; i++)
    {
        T_RemovePrint(ctext[i]);
        ctext[i] = NULL;
        T_RemovePrint(btext[i]);
        btext[i] = NULL;
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

#ifdef TR2_EXE
extern "C" void do_gamma_option(INVENTORY_ITEM *inv_item)
{
}
#endif

#ifdef TR2_EXE
extern "C" char SaveGameStrings1[][50];
extern "C" char SaveGameStrings2[][50];
SG_COL Var4645dc[2] = {
    0x1010, 0x1600,
};
SG_COL Var4645e0[4] = {
    0x1a00, 0x1e00, 0, 0,
};
SG_COL Var4645e8[8] = {
    0x1600, 0x1800, 0x1a00, 0x1800, 0x2000, 0x1010, 0x1010, 0x2000,
};
SG_COL Var4645f8[4] = {
    0x2000, 0x1010, 0x1010, 0x2000,
};
SG_COL Var464600[8] = {
    0x2000, 0x1010, 0x1010, 0x2000, 0x2000, 0x1010, 0x1010, 0x2000,
};
SG_COL Var464610[2] = {
    0x1210, 0x1800,
};
SG_COL Var464614[2] = {
    0x1c00, 0x2000,
};
SG_COL Var464618[4] = {
    0x1800, 0x1a00, 0x1c00, 0x1a00,
};
SG_COL Var464620[4] = {
    0x2000, 0x1800, 0x1800, 0x2000,
};
SG_COL Var464628[4] = {
    0x2000, 0x1800, 0x1800, 0x2000,
};
char Var520d60[MAX_REQLINES][50];
char Var521220[MAX_REQLINES][50];
INVENTORY_SPRITE is_sound_muslevel[9] = {
    { 2, -66, -80, 32, 132, 0, Var4645dc, 1 },
    { 2, -66, -65, 32, 132, 0, Var4645e0, 1 },
    { 2, -66, -80, 32, 0, 16, Var4645dc, 1 },
    { 2, 66, -80, 32, 0, 16, Var4645e0, 1 },
    { 3, -65, -79, 32, 131, 14, Var4645e8, 1 },
    { 3, -64, -78, 32, 129, 12, Var4645e8, 1 },
    { 4, -63, -77, 32, 126, 10, Var4645f8, 3 },
    { 4, -63, -77, 33, 127, 10, NULL, 0 },
    { 0, 0, 0, 0, 0, 0, NULL, 0 },
};
INVENTORY_SPRITE is_sound_sfxlevel[9] = {
    { 2, -66, -56, 32, 132, 0, Var4645dc, 1 },
    { 2, -66, -41, 32, 132, 0, Var4645e0, 1 },
    { 2, -66, -56, 32, 0, 16, Var4645dc, 1 },
    { 2, 66, -56, 32, 0, 16, Var4645e0, 1 },
    { 3, -65, -55, 32, 131, 14, Var4645e8, 1 },
    { 3, -64, -54, 32, 129, 12, Var4645e8, 1 },
    { 4, -63, -53, 32, 126, 10, Var464600, 15 },
    { 4, -63, -53, 33, 127, 10, NULL, 0 },
    { 0, 0, 0, 0, 0, 0, NULL, 0 },
};
INVENTORY_SPRITE is_sound_muslevellow[9] = {
    { 2, -66, -80, 32, 132, 0, Var464610, 1 },
    { 2, -66, -65, 32, 132, 0, Var464614, 1 },
    { 2, -66, -80, 32, 0, 16, Var464610, 1 },
    { 2, 66, -80, 32, 0, 16, Var464614, 1 },
    { 3, -65, -79, 32, 131, 14, Var464618, 1 },
    { 3, -64, -78, 32, 129, 12, Var464618, 1 },
    { 4, -63, -77, 32, 126, 10, Var464620, 3 },
    { 4, -63, -77, 33, 127, 10, NULL, 0 },
    { 0, 0, 0, 0, 0, 0, NULL, 0 },
};
INVENTORY_SPRITE is_sound_sfxlevellow[9] = {
    { 2, -66, -56, 32, 132, 0, Var464610, 1 },
    { 2, -66, -41, 32, 132, 0, Var464614, 1 },
    { 2, -66, -56, 32, 0, 16, Var464610, 1 },
    { 2, 66, -56, 32, 0, 16, Var464614, 1 },
    { 3, -65, -55, 32, 131, 14, Var464618, 1 },
    { 3, -64, -54, 32, 129, 12, Var464618, 1 },
    { 4, -63, -53, 32, 126, 10, Var464628, 15 },
    { 4, -63, -53, 33, 127, 10, NULL, 0 },
    { 0, 0, 0, 0, 0, 0, NULL, 0 },
};
REQUEST_INFO Load_Game_Requester = {
    0, 1, 0, 5, 0, 0, 0x128, 18, 0, -32, 0, 50, Var521220[0], Var520d60[0], NULL, NULL, 0,
    0, 0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, NULL, NULL,
    NULL, NULL, NULL,
    {
        NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
        NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    },
    {
        NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
        NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    },
    "", "", 0, 0,
};
REQUEST_INFO SaveGameRequester = {
    0, 1, 0, 5, 0, 0, 0x110, 18, 0, -32, 0, 50, SaveGameStrings1[0], SaveGameStrings2[0],
    NULL, NULL, 0, 0, 0, 0, 0,
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, NULL, NULL,
    NULL, NULL, NULL,
    {
        NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
        NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    },
    {
        NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
        NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    },
    "", "", 0, 0,
};
sint16 Option_SoundFX_Volume = 165;
sint16 Option_Music_Volume = 255;
sint16 layout[2][14] = {
    { 200, 208, 203, 205, 211, 209, 54, 184, 157, 57, 53, 82, 207, 1 },
    { 72, 80, 75, 77, 71, 73, 79, 78, 156, 81, 74, 82, 76, 83 },
};
char *kname[272] = {
    NULL,
    "ESC",
    "1",
    "2",
    "3",
    "4",
    "5",
    "6",
    "7",
    "8",
    "9",
    "0",
    "-",
    "+",
    "BKSP",
    "TAB",
    "Q",
    "W",
    "E",
    "R",
    "T",
    "Y",
    "U",
    "I",
    "O",
    "P",
    "<",
    ">",
    "RET",
    "CTRL",
    "A",
    "S",
    "D",
    "F",
    "G",
    "H",
    "J",
    "K",
    "L",
    ";",
    "'",
    "`",
    "SHIFT",
    "#",
    "Z",
    "X",
    "C",
    "V",
    "B",
    "N",
    "M",
    ",",
    ".",
    "/",
    "SHIFT",
    "PADx",
    "ALT",
    "SPACE",
    "CAPS",
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    "NMLK",
    NULL,
    "PAD7",
    "PAD8",
    "PAD9",
    "PAD-",
    "PAD4",
    "PAD5",
    "PAD6",
    "PAD+",
    "PAD1",
    "PAD2",
    "PAD3",
    "PAD0",
    "PAD.",
    NULL,
    NULL,
    "\\",
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    "ENTER",
    "CTRL",
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    "SHIFT",
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    "PAD/",
    NULL,
    NULL,
    "ALT",
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    "HOME",
    "UP",
    "PGUP",
    NULL,
    "LEFT",
    NULL,
    "RIGHT",
    NULL,
    "END",
    "DOWN",
    "PGDN",
    "INS",
    "DEL",
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    "JOY1",
    "JOY2",
    "JOY3",
    "JOY4",
    "JOY5",
    "JOY6",
    "JOY7",
    "JOY8",
    "JOY9",
    "JOY10",
    "JOY11",
    "JOY12",
    "JOY13",
    "JOY14",
    "JOY15",
    "JOY16",
};
int detail_level = 1;
char BadKeyString[] = "BAD";
TEXTSTRING *Inv_levelText;
TEXTSTRING *Inv_ringText;
sint32 inputDB;
sint32 idelay;
sint32 idcount;
sint16 item_data;
int SavedGames;
int conflict[14];
int joy_fire;
int KeyCursor;
int select_key;
int iconfig;
TEXTSTRING *ctext[14];
TEXTSTRING *btext[14];
TEXTSTRING *PassportTextInfo;
sint32 passport_mode;
TEXTSTRING *dtext[5];
TEXTSTRING *sa_text[4];
TEXTSTRING *ControlTextInfo[2];
#endif
