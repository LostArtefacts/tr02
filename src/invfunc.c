#include <stdio.h>
#include <string.h>
#include "game.h"
#include "invdata.h"
#include "inventry.h"
#include "savegame.h"

#define ROTATE_DURATION 24
#define CAMERA_YOFFSET -0x60

typedef struct imotion_info {
    sint16 count;
    sint16 status;
    sint16 status_target;
    sint16 radius_target;
    sint16 radius_rate;
    sint16 camera_ytarget;
    sint16 camera_yrate;
    sint16 camera_pitch_target;
    sint16 camera_pitch_rate;
    sint16 rotate_target;
    sint16 rotate_rate;
    PHD_ANGLE item_ptxrot_target;
    PHD_ANGLE item_ptxrot_rate;
    PHD_ANGLE item_xrot_target;
    PHD_ANGLE item_xrot_rate;
    sint32 item_ytrans_target;
    sint32 item_ytrans_rate;
    sint32 item_ztrans_target;
    sint32 item_ztrans_rate;
    sint32 misc;
} IMOTION_INFO;

typedef struct ring_info {
    INVENTORY_ITEM **list;
    sint16 type;
    sint16 radius;
    sint16 camera_pitch;
    sint16 rotating;
    sint16 rot_count;
    sint16 current_object;
    sint16 target_object;
    sint16 number_of_objects;
    sint16 angle_adder;
    sint16 rot_adder;
    sint16 rot_adderL;
    sint16 rot_adderR;
    PHD_3DPOS ringpos;
    PHD_3DPOS camera;
    PHD_VECTOR light;
    IMOTION_INFO *imo;
} RING_INFO;

#define IT_NAME 0
#define IT_QTY 1
#define IT_NUMBEROF 2

extern TEXTSTRING *Inv_itemText[IT_NUMBEROF];

SG_COL S_COLOUR(int R, int G, int B);

void InitColours(void)
{
    inv_colours[C_BLACK] = S_COLOUR(0, 0, 0);
    inv_colours[C_GREY] = S_COLOUR(64, 64, 64);
    inv_colours[C_WHITE] = S_COLOUR(255, 255, 255);
    inv_colours[C_RED] = S_COLOUR(255, 0, 0);
    inv_colours[C_ORANGE] = S_COLOUR(255, 128, 0);
    inv_colours[C_YELLOW] = S_COLOUR(255, 255, 0);
    inv_colours[C_DARKGREEN] = S_COLOUR(0, 128, 0);
    inv_colours[C_GREEN] = S_COLOUR(0, 255, 0);
    inv_colours[C_CYAN] = S_COLOUR(0, 255, 255);
    inv_colours[C_BLUE] = S_COLOUR(0, 0, 255);
    inv_colours[C_MAGENTA] = S_COLOUR(255, 0, 255);
}

#define MAIN_RING 0
#define OPTION_RING 1
#define KEYS_RING 2

extern sint16 inv_main_objects, inv_main_current;
extern sint16 inv_keys_objects, inv_keys_current;

extern TEXTSTRING *Inv_tagText, *Inv_ringText;
extern TEXTSTRING *Inv_upArrow1, *Inv_upArrow2;
extern TEXTSTRING *Inv_downArrow1, *Inv_downArrow2;

void RingIsOpen(RING_INFO *ring)
{
    if (Inventory_Mode == INV_TITLE_MODE)
        return;

    if (!Inv_ringText)
    {
        switch (ring->type)
        {
        case MAIN_RING:
            Inv_ringText = T_Print(0, 26, 0, GF_GameStrings[GT_MAIN_HEADING]);
            break;

        case OPTION_RING:
            if (Inventory_Mode == INV_DEATH_MODE)
                Inv_ringText = T_Print(0, 26, 0, GF_GameStrings[GT_GAMEOVER_HEADING]);
            else
                Inv_ringText = T_Print(0, 26, 0, GF_GameStrings[GT_OPTION_HEADING]);
            break;

        case KEYS_RING:
            Inv_ringText = T_Print(0, 26, 0, GF_GameStrings[GT_KEYS_HEADING]);
            break;
        }
        T_CentreH(Inv_ringText, 1);
    }

    if (Inventory_Mode == INV_KEYS_MODE || Inventory_Mode == INV_DEATH_MODE)
        return;

    if (!Inv_upArrow1)
    {
        if (ring->type == OPTION_RING || (ring->type == MAIN_RING && inv_keys_objects))
        {
            Inv_upArrow1 = T_Print(20, 28, 0, "[");
            Inv_upArrow2 = T_Print(-20, 28, 0, "[");
            T_RightAlign(Inv_upArrow2, 1);
        }
    }

    if (!Inv_downArrow1)
    {
        if ((ring->type == MAIN_RING && !gameflow.lockout_optionring) || ring->type == KEYS_RING)
        {
            Inv_downArrow1 = T_Print(20, -15, 0, "]");
            T_BottomAlign(Inv_downArrow1, 1);
            Inv_downArrow2 = T_Print(-20, -15, 0, "]");
            T_BottomAlign(Inv_downArrow2, 1);
            T_RightAlign(Inv_downArrow2, 1);
        }
    }
}

void RingIsNotOpen(RING_INFO *ring)
{
    T_RemovePrint(Inv_tagText);
    Inv_tagText = NULL;
    T_RemovePrint(Inv_ringText);
    Inv_ringText = NULL;
    T_RemovePrint(Inv_upArrow1);
    Inv_upArrow1 = NULL;
    T_RemovePrint(Inv_upArrow2);
    Inv_upArrow2 = NULL;
    T_RemovePrint(Inv_downArrow1);
    Inv_downArrow1 = NULL;
    T_RemovePrint(Inv_downArrow2);
    Inv_downArrow2 = NULL;
}

#define PASSPORT_OPTION 120
#define SHOTGUN_OPTION 158
#define MAGNUM_OPTION 159
#define UZI_OPTION 160
#define HARPOON_OPTION 161
#define M16_OPTION 162
#define ROCKET_OPTION 163
#define SG_AMMO_OPTION 165
#define MAG_AMMO_OPTION 166
#define UZI_AMMO_OPTION 167
#define HARPOON_AMMO_OPTION 168
#define M16_AMMO_OPTION 169
#define ROCKET_AMMO_OPTION 170
#define MEDI_OPTION 171
#define BIGMEDI_OPTION 172
#define FLAREBOX_OPTION 173
#define PUZZLE_OPTION1 178
#define PUZZLE_OPTION2 179
#define PUZZLE_OPTION3 180
#define PUZZLE_OPTION4 181
#define KEY_OPTION1 197
#define KEY_OPTION2 198
#define KEY_OPTION3 199
#define KEY_OPTION4 200
#define PICKUP_OPTION1 207
#define PICKUP_OPTION2 208
#define SHOTGUN_AMMO_CLIP 6
#define NUM_SG_SHELLS 2

extern char **GF_Puzzle1Strings, **GF_Puzzle2Strings, **GF_Puzzle3Strings, **GF_Puzzle4Strings;
extern char **GF_Pickup1Strings, **GF_Pickup2Strings;
extern char **GF_Key1Strings, **GF_Key2Strings, **GF_Key3Strings, **GF_Key4Strings;
extern int HealthBarTimer;

int FlashIt(void);
void DrawHealthBar(int flash_state);
void MakeAmmoString(char *string);
int Inv_RequestItem(int itemNum);
__declspec(dllimport) int __cdecl wsprintfA(char *buf, const char *fmt, ...);

void RingNotActive(INVENTORY_ITEM *inv_item)
{
    int tempInt;
    char tempText[64];

    if (!Inv_itemText[IT_NAME])
    {
        switch (inv_item->object_number)
        {
        case PUZZLE_OPTION1:
            Inv_itemText[IT_NAME] = T_Print(0, -16, 0, GF_Puzzle1Strings[CurrentLevel]);
            break;

        case PUZZLE_OPTION2:
            Inv_itemText[IT_NAME] = T_Print(0, -16, 0, GF_Puzzle2Strings[CurrentLevel]);
            break;

        case PUZZLE_OPTION3:
            Inv_itemText[IT_NAME] = T_Print(0, -16, 0, GF_Puzzle3Strings[CurrentLevel]);
            break;

        case PUZZLE_OPTION4:
            Inv_itemText[IT_NAME] = T_Print(0, -16, 0, GF_Puzzle4Strings[CurrentLevel]);
            break;

        case PICKUP_OPTION1:
            Inv_itemText[IT_NAME] = T_Print(0, -16, 0, GF_Pickup1Strings[CurrentLevel]);
            break;

        case PICKUP_OPTION2:
            Inv_itemText[IT_NAME] = T_Print(0, -16, 0, GF_Pickup2Strings[CurrentLevel]);
            break;

        case KEY_OPTION1:
            Inv_itemText[IT_NAME] = T_Print(0, -16, 0, GF_Key1Strings[CurrentLevel]);
            break;

        case KEY_OPTION2:
            Inv_itemText[IT_NAME] = T_Print(0, -16, 0, GF_Key2Strings[CurrentLevel]);
            break;

        case KEY_OPTION3:
            Inv_itemText[IT_NAME] = T_Print(0, -16, 0, GF_Key3Strings[CurrentLevel]);
            break;

        case KEY_OPTION4:
            Inv_itemText[IT_NAME] = T_Print(0, -16, 0, GF_Key4Strings[CurrentLevel]);
            break;

        case PASSPORT_OPTION:
            break;

        default:
            Inv_itemText[IT_NAME] = T_Print(0, -16, 0, inv_item->itemText);
            break;
        }

        if (Inv_itemText[IT_NAME])
        {
            T_BottomAlign(Inv_itemText[IT_NAME], 1);
            T_CentreH(Inv_itemText[IT_NAME], 1);
        }
    }

    tempInt = Inv_RequestItem(inv_item->object_number);

    switch (inv_item->object_number)
    {
    case SHOTGUN_OPTION:
        if (!Inv_itemText[IT_QTY] && !savegame.bonus_flag)
            wsprintfA(tempText, "%5d", lara.shotgun.ammo / SHOTGUN_AMMO_CLIP);
        else
            return;
        break;

    case MAGNUM_OPTION:
        if (!Inv_itemText[IT_QTY] && !savegame.bonus_flag)
            wsprintfA(tempText, "%5d", lara.magnums.ammo);
        else
            return;
        break;

    case UZI_OPTION:
        if (!Inv_itemText[IT_QTY] && !savegame.bonus_flag)
            wsprintfA(tempText, "%5d", lara.uzis.ammo);
        else
            return;
        break;

    case HARPOON_OPTION:
        if (!Inv_itemText[IT_QTY] && !savegame.bonus_flag)
            wsprintfA(tempText, "%5d", lara.harpoon.ammo);
        else
            return;
        break;

    case M16_OPTION:
        if (!Inv_itemText[IT_QTY] && !savegame.bonus_flag)
            wsprintfA(tempText, "%5d", lara.m16.ammo);
        else
            return;
        break;

    case ROCKET_OPTION:
        if (!Inv_itemText[IT_QTY] && !savegame.bonus_flag)
            wsprintfA(tempText, "%5d", lara.grenade.ammo);
        else
            return;
        break;

    case SG_AMMO_OPTION:
        if (!Inv_itemText[IT_QTY])
            wsprintfA(tempText, "%d", tempInt * NUM_SG_SHELLS);
        else
            return;
        break;

    case MAG_AMMO_OPTION:
    case UZI_AMMO_OPTION:
    case M16_AMMO_OPTION:
    case HARPOON_AMMO_OPTION:
        if (!Inv_itemText[IT_QTY] && !savegame.bonus_flag)
            wsprintfA(tempText, "%d", tempInt * 2);
        else
            return;
        break;

    case ROCKET_AMMO_OPTION:
    case FLAREBOX_OPTION:
        if (!Inv_itemText[IT_QTY] && !savegame.bonus_flag)
            wsprintfA(tempText, "%d", tempInt);
        else
            return;
        break;

    case MEDI_OPTION:
    case BIGMEDI_OPTION:
        HealthBarTimer = 40;
        DrawHealthBar(FlashIt());
    case PUZZLE_OPTION1:
    case PUZZLE_OPTION2:
    case PUZZLE_OPTION3:
    case PUZZLE_OPTION4:
    case PICKUP_OPTION1:
    case PICKUP_OPTION2:
    case KEY_OPTION1:
    case KEY_OPTION2:
    case KEY_OPTION3:
    case KEY_OPTION4:
        if (!Inv_itemText[IT_QTY] && tempInt > 1)
            wsprintfA(tempText, "%d", tempInt);
        else
            return;
        break;

    default:
        return;
    }

    if (!Inv_itemText[IT_QTY])
    {
        MakeAmmoString(tempText);
        Inv_itemText[IT_QTY] = T_Print(64, -56, 0, tempText);
        T_BottomAlign(Inv_itemText[IT_QTY], 1);
        T_CentreH(Inv_itemText[IT_QTY], 1);
    }
}

void RingActive(void)
{
    T_RemovePrint(Inv_itemText[IT_NAME]);
    Inv_itemText[IT_NAME] = NULL;
    T_RemovePrint(Inv_itemText[IT_QTY]);
    Inv_itemText[IT_QTY] = NULL;
}

extern INVENTORY_ITEM *inv_main_list[];
extern INVENTORY_ITEM *inv_keys_list[];
extern sint16 inv_main_qtys[];
extern sint16 inv_keys_qtys[];

#define MAP_OPTION 121
#define MAP_CLOSED 134
#define GUN_ITEM 135
#define SHOTGUN_ITEM 136
#define MAGNUM_ITEM 137
#define UZI_ITEM 138
#define HARPOON_ITEM 139
#define M16_ITEM 140
#define ROCKET_GUN_ITEM 141
#define SG_AMMO_ITEM 143
#define MAG_AMMO_ITEM 144
#define UZI_AMMO_ITEM 145
#define HARPOON_AMMO_ITEM 146
#define M16_AMMO_ITEM 147
#define ROCKET_AMMO_ITEM 148
#define MEDI_ITEM 149
#define BIGMEDI_ITEM 150
#define FLAREBOX_ITEM 151
#define FLARE_ITEM 152
#define GUN_OPTION 157
#define PUZZLE_ITEM1 174
#define PUZZLE_ITEM2 175
#define PUZZLE_ITEM3 176
#define PUZZLE_ITEM4 177
#define SECRET_ITEM1 190
#define SECRET_ITEM2 191
#define SECRET_ITEM3 192
#define KEY_ITEM1 193
#define KEY_ITEM2 194
#define KEY_ITEM3 195
#define KEY_ITEM4 196
#define PICKUP_ITEM1 205
#define PICKUP_ITEM2 206
#define FLARE_AMMO_BOX 6

extern INVENTORY_ITEM InvCompassOption, InvPistolOption, InvFlareOption, InvShotgunOption, InvMagnumOption, InvUziOption;
extern INVENTORY_ITEM InvHarpoonOption, InvM16Option, InvGrenadeOption, InvShotgunAmmoOption, InvMagnumAmmoOption, InvUziAmmoOption;
extern INVENTORY_ITEM InvHarpoonAmmoOption, InvM16AmmoOption, InvGrenadeAmmoOption, InvSmallMedipackOption, InvLargeMedipackOption;
extern INVENTORY_ITEM InvPickup1Option, InvPickup2Option, InvPuzzle1Option, InvPuzzle2Option, InvPuzzle3Option, InvPuzzle4Option;
extern INVENTORY_ITEM InvKey1Option, InvKey2Option, InvKey3Option, InvKey4Option;

void Inv_InsertItem(INVENTORY_ITEM *inv_item);
int Inv_GetItemOption(int itemNum);
int Inv_RemoveItem(int itemNum);
int GlobalItemReplace(int in_objnum, int out_objnum);

int Inv_AddItem(int itemNum)
{
    int n, m, found = 0;
    int itemNumOption;
    INVENTORY_ITEM *inv_item;

    itemNumOption = Inv_GetItemOption(itemNum);
    for (n = 0; n < inv_main_objects; n++)
    {
        inv_item = inv_main_list[n];
        if (inv_item->object_number == itemNumOption)
        {
            found = 1;
            break;
        }
    }
    for (m = 0; m < inv_keys_objects; m++)
    {
        inv_item = inv_keys_list[m];
        if (inv_item->object_number == itemNumOption)
        {
            found = 2;
            break;
        }
    }

    if (found == 1)
    {
        if (itemNum == FLAREBOX_ITEM)
            inv_main_qtys[n] += FLARE_AMMO_BOX;
        else
            inv_main_qtys[n]++;
        return 1;
    }
    else if (found == 2)
    {
        inv_keys_qtys[m]++;
        return 1;
    }
    else
    {
        switch (itemNum)
        {
        case MAP_CLOSED:
        case MAP_OPTION:
            Inv_InsertItem(&InvCompassOption);
            return 1;

        case GUN_ITEM:
        case GUN_OPTION:
            Inv_InsertItem(&InvPistolOption);
            if (lara.last_gun_type == LG_UNARMED)
                lara.last_gun_type = LG_PISTOLS;
            return 1;

        case SHOTGUN_ITEM:
        case SHOTGUN_OPTION:
            if ((found = Inv_RequestItem(SG_AMMO_ITEM)))
            {
                for (n = 0; n < found; n++)
                {
                    Inv_RemoveItem(SG_AMMO_ITEM);
                    lara.shotgun.ammo += 12;
                }
            }
            lara.shotgun.ammo += 12;
            Inv_InsertItem(&InvShotgunOption);
            if (lara.last_gun_type == LG_UNARMED)
                lara.last_gun_type = LG_SHOTGUN;
            if (!lara.back_gun)
                lara.back_gun = SHOTGUN;
            GlobalItemReplace(SHOTGUN_ITEM, SG_AMMO_ITEM);
            break;

        case MAGNUM_ITEM:
        case MAGNUM_OPTION:
            if ((found = Inv_RequestItem(MAG_AMMO_ITEM)))
            {
                for (n = 0; n < found; n++)
                {
                    Inv_RemoveItem(MAG_AMMO_ITEM);
                    lara.magnums.ammo += 40;
                }
            }
            GlobalItemReplace(MAGNUM_ITEM, MAG_AMMO_ITEM);
            lara.magnums.ammo += 40;
            Inv_InsertItem(&InvMagnumOption);
            break;

        case UZI_ITEM:
        case UZI_OPTION:
            if ((found = Inv_RequestItem(UZI_AMMO_ITEM)))
            {
                for (n = 0; n < found; n++)
                {
                    Inv_RemoveItem(UZI_AMMO_ITEM);
                    lara.uzis.ammo += 80;
                }
            }
            GlobalItemReplace(UZI_ITEM, UZI_AMMO_ITEM);
            lara.uzis.ammo += 80;
            Inv_InsertItem(&InvUziOption);
            break;

        case HARPOON_ITEM:
        case HARPOON_OPTION:
            if ((found = Inv_RequestItem(HARPOON_AMMO_ITEM)))
            {
                for (n = 0; n < found; n++)
                {
                    Inv_RemoveItem(HARPOON_AMMO_ITEM);
                    lara.harpoon.ammo += 3;
                }
            }
            GlobalItemReplace(HARPOON_ITEM, HARPOON_AMMO_ITEM);
            lara.harpoon.ammo += 3;
            Inv_InsertItem(&InvHarpoonOption);
            break;

        case M16_ITEM:
        case M16_OPTION:
            if ((found = Inv_RequestItem(M16_AMMO_ITEM)))
            {
                for (n = 0; n < found; n++)
                {
                    Inv_RemoveItem(M16_AMMO_ITEM);
                    lara.m16.ammo += 40;
                }
            }
            GlobalItemReplace(M16_ITEM, M16_AMMO_ITEM);
            lara.m16.ammo += 40;
            Inv_InsertItem(&InvM16Option);
            break;

        case ROCKET_GUN_ITEM:
        case ROCKET_OPTION:
            if ((found = Inv_RequestItem(ROCKET_AMMO_ITEM)))
            {
                for (n = 0; n < found; n++)
                {
                    Inv_RemoveItem(ROCKET_AMMO_ITEM);
                    lara.grenade.ammo += 2;
                }
            }
            GlobalItemReplace(ROCKET_GUN_ITEM, ROCKET_AMMO_ITEM);
            lara.grenade.ammo += 2;
            Inv_InsertItem(&InvGrenadeOption);
            break;

        case SG_AMMO_ITEM:
        case SG_AMMO_OPTION:
            if (Inv_RequestItem(SHOTGUN_ITEM))
                lara.shotgun.ammo += 12;
            else
                Inv_InsertItem(&InvShotgunAmmoOption);
            break;

        case MAG_AMMO_ITEM:
        case MAG_AMMO_OPTION:
            if (Inv_RequestItem(MAGNUM_ITEM))
                lara.magnums.ammo += 40;
            else
                Inv_InsertItem(&InvMagnumAmmoOption);
            break;

        case UZI_AMMO_ITEM:
        case UZI_AMMO_OPTION:
            if (Inv_RequestItem(UZI_ITEM))
                lara.uzis.ammo += 80;
            else
                Inv_InsertItem(&InvUziAmmoOption);
            break;

        case HARPOON_AMMO_ITEM:
        case HARPOON_AMMO_OPTION:
            if (Inv_RequestItem(HARPOON_ITEM))
                lara.harpoon.ammo += 3;
            else
                Inv_InsertItem(&InvHarpoonAmmoOption);
            break;

        case M16_AMMO_ITEM:
        case M16_AMMO_OPTION:
            if (Inv_RequestItem(M16_ITEM))
                lara.m16.ammo += 40;
            else
                Inv_InsertItem(&InvM16AmmoOption);
            break;

        case ROCKET_AMMO_ITEM:
        case ROCKET_AMMO_OPTION:
            if (Inv_RequestItem(ROCKET_GUN_ITEM))
                lara.grenade.ammo += 2;
            else
                Inv_InsertItem(&InvGrenadeAmmoOption);
            break;

        case FLAREBOX_ITEM:
        case FLAREBOX_OPTION:
            Inv_InsertItem(&InvFlareOption);
            for (n = 0; n < FLARE_AMMO_BOX - 1; n++)
                Inv_AddItem(FLARE_ITEM);
            return 1;

        case FLARE_ITEM:
            Inv_InsertItem(&InvFlareOption);
            return 1;

        case MEDI_ITEM:
        case MEDI_OPTION:
            Inv_InsertItem(&InvSmallMedipackOption);
            return 1;

        case BIGMEDI_ITEM:
        case BIGMEDI_OPTION:
            Inv_InsertItem(&InvLargeMedipackOption);
            return 1;

        case SECRET_ITEM1:
            savegame.secrets |= 1;
            return 1;

        case SECRET_ITEM2:
            savegame.secrets |= 2;
            return 1;

        case SECRET_ITEM3:
            savegame.secrets |= 4;
            return 1;

        case PICKUP_ITEM1:
        case PICKUP_OPTION1:
            Inv_InsertItem(&InvPickup1Option);
            return 1;

        case PICKUP_ITEM2:
        case PICKUP_OPTION2:
            Inv_InsertItem(&InvPickup2Option);
            return 1;

        case PUZZLE_ITEM1:
        case PUZZLE_OPTION1:
            Inv_InsertItem(&InvPuzzle1Option);
            return 1;

        case PUZZLE_ITEM2:
        case PUZZLE_OPTION2:
            Inv_InsertItem(&InvPuzzle2Option);
            return 1;

        case PUZZLE_ITEM3:
        case PUZZLE_OPTION3:
            Inv_InsertItem(&InvPuzzle3Option);
            return 1;

        case PUZZLE_ITEM4:
        case PUZZLE_OPTION4:
            Inv_InsertItem(&InvPuzzle4Option);
            return 1;

        case KEY_ITEM1:
        case KEY_OPTION1:
            Inv_InsertItem(&InvKey1Option);
            return 1;

        case KEY_ITEM2:
        case KEY_OPTION2:
            Inv_InsertItem(&InvKey2Option);
            return 1;

        case KEY_ITEM3:
        case KEY_OPTION3:
            Inv_InsertItem(&InvKey3Option);
            return 1;

        case KEY_ITEM4:
        case KEY_OPTION4:
            Inv_InsertItem(&InvKey4Option);
            return 1;

        default:
            return 0;
        }
    }
    return 0;
}

void Inv_InsertItem(INVENTORY_ITEM *inv_item)
{
    int n, m;
    INVENTORY_ITEM *inv_place;

    if (inv_item->inv_pos < 100)
    {
        for (n = 0; n < inv_main_objects; n++)
        {
            inv_place = inv_main_list[n];
            if (inv_place->inv_pos > inv_item->inv_pos)
                break;
        }

        if (n == inv_main_objects)
        {
            inv_main_list[inv_main_objects] = inv_item;
            inv_main_qtys[inv_main_objects] = 1;
            inv_main_objects++;
        }
        else
        {
            for (m = inv_main_objects; m > (n - 1); m--)
            {
                inv_main_list[m + 1] = inv_main_list[m];
                inv_main_qtys[m + 1] = inv_main_qtys[m];
            }
            inv_main_list[n] = inv_item;
            inv_main_qtys[n] = 1;
            inv_main_objects++;
        }
    }
    else
    {
        for (n = 0; n < inv_keys_objects; n++)
        {
            inv_place = inv_keys_list[n];
            if (inv_place->inv_pos > inv_item->inv_pos)
                break;
        }

        if (n == inv_keys_objects)
        {
            inv_keys_list[inv_keys_objects] = inv_item;
            inv_keys_qtys[inv_keys_objects] = 1;
            inv_keys_objects++;
        }
        else
        {
            for (m = inv_keys_objects; m > (n - 1); m--)
            {
                inv_keys_list[m + 1] = inv_keys_list[m];
                inv_keys_qtys[m + 1] = inv_keys_qtys[m];
            }
            inv_keys_list[n] = inv_item;
            inv_keys_qtys[n] = 1;
            inv_keys_objects++;
        }
    }
}

int Inv_GetItemOption(int itemNum);

int Inv_RequestItem(int itemNum)
{
    int n;
    int itemNumOption;
    INVENTORY_ITEM *inv_item;

    itemNumOption = Inv_GetItemOption(itemNum);
    for (n = 0; n < inv_main_objects; n++)
    {
        inv_item = inv_main_list[n];
        if (inv_item->object_number == itemNumOption)
            return inv_main_qtys[n];
    }
    for (n = 0; n < inv_keys_objects; n++)
    {
        inv_item = inv_keys_list[n];
        if (inv_item->object_number == itemNumOption)
            return inv_keys_qtys[n];
    }
    return 0;
}

void Inv_RemoveAllItems(void)
{
    inv_main_objects = 0;
    inv_main_current = 0;
    inv_keys_objects = 0;
    inv_keys_current = 0;
}

int Inv_RemoveItem(int itemNum)
{
    int n, m;
    int itemNumOption;
    INVENTORY_ITEM *inv_item;

    itemNumOption = Inv_GetItemOption(itemNum);

    for (n = 0; n < inv_main_objects; n++)
    {
        inv_item = inv_main_list[n];
        if (inv_item->object_number == itemNumOption)
        {
            inv_main_qtys[n]--;
            if (inv_main_qtys[n] <= 0)
            {
                inv_main_objects--;
                for (m = n; m < inv_main_objects; m++)
                {
                    inv_main_list[m] = inv_main_list[m + 1];
                    inv_main_qtys[m] = inv_main_qtys[m + 1];
                }
            }
            return 1;
        }
    }

    for (n = 0; n < inv_keys_objects; n++)
    {
        inv_item = inv_keys_list[n];
        if (inv_item->object_number == itemNumOption)
        {
            inv_keys_qtys[n]--;
            if (inv_keys_qtys[n] <= 0)
            {
                inv_keys_objects--;
                for (m = n; m < inv_keys_objects; m++)
                {
                    inv_keys_list[m] = inv_keys_list[m + 1];
                    inv_keys_qtys[m] = inv_keys_qtys[m + 1];
                }
            }
            return 1;
        }
    }

    return 0;
}

int Inv_GetItemOption(int itemNum)
{
    switch (itemNum)
    {
    case GUN_ITEM:
    case GUN_OPTION:
        return GUN_OPTION;

    case FLARE_ITEM:
    case FLAREBOX_ITEM:
    case FLAREBOX_OPTION:
        return FLAREBOX_OPTION;

    case SHOTGUN_ITEM:
    case SHOTGUN_OPTION:
        return SHOTGUN_OPTION;

    case MAGNUM_ITEM:
    case MAGNUM_OPTION:
        return MAGNUM_OPTION;

    case UZI_ITEM:
    case UZI_OPTION:
        return UZI_OPTION;

    case M16_ITEM:
    case M16_OPTION:
        return M16_OPTION;

    case ROCKET_GUN_ITEM:
    case ROCKET_OPTION:
        return ROCKET_OPTION;

    case HARPOON_ITEM:
    case HARPOON_OPTION:
        return HARPOON_OPTION;

    case SG_AMMO_ITEM:
    case SG_AMMO_OPTION:
        return SG_AMMO_OPTION;

    case MAG_AMMO_ITEM:
    case MAG_AMMO_OPTION:
        return MAG_AMMO_OPTION;

    case HARPOON_AMMO_ITEM:
    case HARPOON_AMMO_OPTION:
        return HARPOON_AMMO_OPTION;

    case M16_AMMO_ITEM:
    case M16_AMMO_OPTION:
        return M16_AMMO_OPTION;

    case ROCKET_AMMO_ITEM:
    case ROCKET_AMMO_OPTION:
        return ROCKET_AMMO_OPTION;

    case UZI_AMMO_ITEM:
    case UZI_AMMO_OPTION:
        return UZI_AMMO_OPTION;

    case MEDI_ITEM:
    case MEDI_OPTION:
        return MEDI_OPTION;

    case BIGMEDI_ITEM:
    case BIGMEDI_OPTION:
        return BIGMEDI_OPTION;

    case PUZZLE_ITEM1:
    case PUZZLE_OPTION1:
        return PUZZLE_OPTION1;

    case PUZZLE_ITEM2:
    case PUZZLE_OPTION2:
        return PUZZLE_OPTION2;

    case PUZZLE_ITEM3:
    case PUZZLE_OPTION3:
        return PUZZLE_OPTION3;

    case PUZZLE_ITEM4:
    case PUZZLE_OPTION4:
        return PUZZLE_OPTION4;

    case PICKUP_ITEM1:
    case PICKUP_OPTION1:
        return PICKUP_OPTION1;

    case PICKUP_ITEM2:
    case PICKUP_OPTION2:
        return PICKUP_OPTION2;

    case KEY_ITEM1:
    case KEY_OPTION1:
        return KEY_OPTION1;

    case KEY_ITEM2:
    case KEY_OPTION2:
        return KEY_OPTION2;

    case KEY_ITEM3:
    case KEY_OPTION3:
        return KEY_OPTION3;

    case KEY_ITEM4:
    case KEY_OPTION4:
        return KEY_OPTION4;
    }

    return -1;
}

void RemoveInventoryText(void)
{
    int n;

    for (n = 0; n < IT_NUMBEROF; n++)
    {
        T_RemovePrint(Inv_itemText[n]);
        Inv_itemText[n] = NULL;
    }
}

#define OPEN_FRAMES 32
#define OPEN_ROTATION -0x8000
#define RING_RADIUS 0x2b0
#define CAMERA_HEIGHT -0x100
#define CAMERA_STARTHEIGHT -0x600
#define RNG_OPENING 0
#define RNG_OPEN 1

void Inv_RingMotionInit(RING_INFO *ring, sint16 frames, sint16 status, sint16 status_target);
void Inv_RingMotionRadius(RING_INFO *ring, sint16 target);
void Inv_RingMotionRotation(RING_INFO *ring, sint16 rotation, sint16 target);
void Inv_RingMotionCameraPos(RING_INFO *ring, sint16 target);

void Inv_RingInit(RING_INFO *ring, sint16 type, INVENTORY_ITEM **list, sint16 qty, sint16 current, IMOTION_INFO *imo)
{
    ring->type = type;
    ring->radius = 0;
    ring->list = list;
    ring->number_of_objects = qty;
    ring->current_object = current;
    ring->angle_adder = 0x10000 / qty;

    if (Inventory_Mode != INV_TITLE_MODE)
        ring->camera_pitch = 0;
    else
        ring->camera_pitch = 0x400;

    ring->rotating = 0;
    ring->rot_count = 0;
    ring->target_object = 0;
    ring->rot_adder = 0;
    ring->rot_adderL = 0;
    ring->rot_adderR = 0;

    ring->imo = imo;

    ring->camera.x_pos = 0;
    ring->camera.y_pos = CAMERA_STARTHEIGHT;
    ring->camera.z_pos = 0x380;
    ring->camera.x_rot = 0;
    ring->camera.y_rot = 0;
    ring->camera.z_rot = 0;

    Inv_RingMotionInit(ring, OPEN_FRAMES, RNG_OPENING, RNG_OPEN);
    Inv_RingMotionRadius(ring, RING_RADIUS);
    Inv_RingMotionCameraPos(ring, CAMERA_HEIGHT);
    Inv_RingMotionRotation(ring, OPEN_ROTATION, (sint16)(0xc000 - (ring->current_object * ring->angle_adder)));

    ring->ringpos.x_pos = 0;
    ring->ringpos.y_pos = 0;
    ring->ringpos.z_pos = 0;
    ring->ringpos.x_rot = 0;
    ring->ringpos.y_rot = imo->rotate_target - OPEN_ROTATION;
    ring->ringpos.z_rot = 0;

    ring->light.x = -0x600;
    ring->light.y = 0x100;
    ring->light.z = 0x400;
}

void Inv_RingGetView(RING_INFO *ring, PHD_3DPOS *viewer)
{
    PHD_ANGLE angles[2];

    phd_GetVectorAngles(0 - ring->camera.x_pos, CAMERA_YOFFSET - ring->camera.y_pos, ring->radius - ring->camera.z_pos, angles);
    viewer->x_pos = ring->camera.x_pos;
    viewer->y_pos = ring->camera.y_pos;
    viewer->z_pos = ring->camera.z_pos;
    viewer->x_rot = angles[1] + ring->camera_pitch;
    viewer->y_rot = angles[0];
    viewer->z_rot = 0;
}

extern sint32 ls_divider;

void Inv_RingLight(RING_INFO *ring)
{
    PHD_ANGLE angles[2];

    ls_divider = 0x6000;
    phd_GetVectorAngles(ring->light.x, ring->light.y, ring->light.z, angles);
    phd_RotateLight(angles[1], angles[0]);
}

void Inv_RingCalcAdders(RING_INFO *ring, sint16 rotation_duration)
{
    ring->angle_adder = 0x10000 / ring->number_of_objects;
    ring->rot_adderL = ring->angle_adder / rotation_duration;
    ring->rot_adderR = -ring->rot_adderL;
}

void Inv_RingDoMotions(RING_INFO *ring)
{
    INVENTORY_ITEM *inv_item;
    IMOTION_INFO *imo;

    imo = ring->imo;
    if (imo->count)
    {
        ring->radius += imo->radius_rate;
        ring->camera.y_pos += imo->camera_yrate;
        ring->ringpos.y_rot += imo->rotate_rate;
        ring->camera_pitch += imo->camera_pitch_rate;

        inv_item = *(ring->list + ring->current_object);
        inv_item->pt_xrot += imo->item_ptxrot_rate;
        inv_item->x_rot += imo->item_xrot_rate;
        inv_item->ytrans += imo->item_ytrans_rate;
        inv_item->ztrans += imo->item_ztrans_rate;

        imo->count--;
        if (!imo->count)
        {
            imo->status = imo->status_target;
            if (imo->radius_rate)
            {
                imo->radius_rate = 0;
                ring->radius = imo->radius_target;
            }
            if (imo->camera_yrate)
            {
                imo->camera_yrate = 0;
                ring->camera.y_pos = imo->camera_ytarget;
            }
            if (imo->rotate_rate)
            {
                imo->rotate_rate = 0;
                ring->ringpos.y_rot = imo->rotate_target;
            }
            if (imo->item_ptxrot_rate)
            {
                imo->item_ptxrot_rate = 0;
                inv_item->pt_xrot = imo->item_ptxrot_target;
            }
            if (imo->item_xrot_rate)
            {
                imo->item_xrot_rate = 0;
                inv_item->x_rot = imo->item_xrot_target;
            }
            if (imo->item_ytrans_rate)
            {
                imo->item_ytrans_rate = 0;
                inv_item->ytrans = imo->item_ytrans_target;
            }
            if (imo->item_ztrans_rate)
            {
                imo->item_ztrans_rate = 0;
                inv_item->ztrans = imo->item_ztrans_target;
            }
            if (imo->camera_pitch_rate)
            {
                imo->camera_pitch_rate = 0;
                ring->camera_pitch = imo->camera_pitch_target;
            }
        }
    }

    if (ring->rotating)
    {
        ring->ringpos.y_rot += ring->rot_adder;
        ring->rot_count--;
        if (ring->rot_count == 0)
        {
            ring->current_object = ring->target_object;
            ring->ringpos.y_rot = 0xc000 - (ring->current_object * ring->angle_adder);
            ring->rotating = 0;
        }
    }
}

void Inv_RingRotateLeft(RING_INFO *ring)
{
    ring->rotating = 1;
    ring->target_object = ring->current_object - 1;
    if (ring->target_object < 0)
        ring->target_object = ring->number_of_objects - 1;
    ring->rot_count = ROTATE_DURATION;
    ring->rot_adder = ring->rot_adderL;
}

void Inv_RingRotateRight(RING_INFO *ring)
{
    ring->rotating = 1;
    ring->target_object = ring->current_object + 1;
    if (ring->target_object >= ring->number_of_objects)
        ring->target_object = 0;
    ring->rot_count = ROTATE_DURATION;
    ring->rot_adder = ring->rot_adderR;
}

void Inv_RingMotionInit(RING_INFO *ring, sint16 frames, sint16 status, sint16 status_target)
{
    IMOTION_INFO *imo;

    imo = ring->imo;
    imo->count = frames;
    imo->status = status;
    imo->status_target = status_target;
    imo->radius_target = 0;
    imo->radius_rate = 0;
    imo->camera_ytarget = 0;
    imo->camera_yrate = 0;
    imo->camera_pitch_target = 0;
    imo->camera_pitch_rate = 0;
    imo->rotate_target = 0;
    imo->rotate_rate = 0;
    imo->item_ptxrot_target = 0;
    imo->item_ptxrot_rate = 0;
    imo->item_xrot_target = 0;
    imo->item_xrot_rate = 0;
    imo->item_ytrans_target = 0;
    imo->item_ytrans_rate = 0;
    imo->item_ztrans_target = 0;
    imo->item_ztrans_rate = 0;
    imo->misc = 0;
}

void Inv_RingMotionSetup(RING_INFO *ring, sint16 status, sint16 status_target, sint16 frames)
{
    IMOTION_INFO *imo;

    imo = ring->imo;
    imo->count = frames;
    imo->status = status;
    imo->status_target = status_target;
    imo->radius_rate = 0;
    imo->camera_yrate = 0;
}

void Inv_RingMotionRadius(RING_INFO *ring, sint16 target)
{
    IMOTION_INFO *imo;

    imo = ring->imo;
    imo->radius_target = target;
    imo->radius_rate = (target - ring->radius) / imo->count;
}

void Inv_RingMotionRotation(RING_INFO *ring, sint16 rotation, sint16 target)
{
    IMOTION_INFO *imo;

    imo = ring->imo;
    imo->rotate_rate = rotation / imo->count;
    imo->rotate_target = target;
}

void Inv_RingMotionCameraPos(RING_INFO *ring, sint16 target)
{
    IMOTION_INFO *imo;

    imo = ring->imo;
    imo->camera_ytarget = target;
    imo->camera_yrate = (target - ring->camera.y_pos) / imo->count;
}

void Inv_RingMotionCameraPitch(RING_INFO *ring, sint16 target)
{
    IMOTION_INFO *imo;

    imo = ring->imo;
    imo->camera_pitch_target = target;
    imo->camera_pitch_rate = target / imo->count;
}

void Inv_RingMotionItemSelect(RING_INFO *ring, INVENTORY_ITEM *inv_item)
{
    IMOTION_INFO *imo;

    imo = ring->imo;
    imo->item_ptxrot_target = inv_item->pt_xrot_sel;
    imo->item_ptxrot_rate = inv_item->pt_xrot_sel / imo->count;
    imo->item_xrot_target = inv_item->x_rot_sel;
    imo->item_xrot_rate = (inv_item->x_rot_sel - inv_item->x_rot_nosel) / imo->count;
    imo->item_ytrans_target = inv_item->ytrans_sel;
    imo->item_ytrans_rate = inv_item->ytrans_sel / imo->count;
    imo->item_ztrans_target = inv_item->ztrans_sel;
    imo->item_ztrans_rate = inv_item->ztrans_sel / imo->count;
}

void Inv_RingMotionItemDeselect(RING_INFO *ring, INVENTORY_ITEM *inv_item)
{
    IMOTION_INFO *imo;

    imo = ring->imo;
    imo->item_ptxrot_target = 0;
    imo->item_ptxrot_rate = 0 - inv_item->pt_xrot_sel / imo->count;
    imo->item_xrot_target = inv_item->x_rot_nosel;
    imo->item_xrot_rate = (inv_item->x_rot_nosel - inv_item->x_rot_sel) / imo->count;
    imo->item_ytrans_target = 0;
    imo->item_ytrans_rate = 0 - inv_item->ytrans_sel / imo->count;
    imo->item_ztrans_target = 0;
    imo->item_ztrans_rate = 0 - inv_item->ztrans_sel / imo->count;
}

void Init_Requester(REQUEST_INFO *req)
{
    int n;

    req->items = 0;
    req->heading1text = NULL;
    req->heading2text = NULL;
    req->heading1_flags = 0;
    req->heading2_flags = 0;
    req->backgroundtext = NULL;
    req->background_flags = R_USE;
    req->moreuptext = NULL;
    req->moreup_flags = R_USE;
    req->moredowntext = NULL;
    req->moredown_flags = R_USE;
    for (n = 0; n < MAX_REQLINES; n++)
    {
        req->texts1[n] = NULL;
        req->texts1_flags[n] = 0;
        req->texts2[n] = NULL;
        req->texts2_flags[n] = 0;
    }
    req->itemtexts1_flags = RequesterFlags1;
    req->itemtexts2_flags = RequesterFlags2;
    req->original_render_width = GetRenderWidth();
    req->original_render_height = GetRenderHeight();
}

void Remove_Requester(REQUEST_INFO *req)
{
    int n;

    T_RemovePrint(req->heading1text);
    req->heading1text = NULL;
    T_RemovePrint(req->heading2text);
    req->heading2text = NULL;
    T_RemovePrint(req->backgroundtext);
    req->backgroundtext = NULL;
    T_RemovePrint(req->moreuptext);
    req->moreuptext = NULL;
    T_RemovePrint(req->moredowntext);
    req->moredowntext = NULL;
    for (n = 0; n < MAX_REQLINES; n++)
    {
        T_RemovePrint(req->texts1[n]);
        req->texts1[n] = NULL;
        T_RemovePrint(req->texts2[n]);
        req->texts2[n] = NULL;
    }
}

void ReqItemCentreAlign(REQUEST_INFO *req, TEXTSTRING *txt)
{
    if (txt)
    {
        txt->xpos = req->xpos;
        txt->bgndOffX = 0;
    }
}

uint32 GetTextScaleH(uint32 scaleH);
sint32 T_GetTextWidth(TEXTSTRING *textString);

void ReqItemLeftAlign(REQUEST_INFO *req, TEXTSTRING *txt)
{
    int x;
    uint32 scaleH = GetTextScaleH(txt->scaleH);

    if (txt)
    {
        x = (((req->pixwidth * scaleH) >> 16) / 2 - T_GetTextWidth(txt) / 2) - ((8 * scaleH) >> 16);
        txt->xpos = req->xpos - x;
        txt->bgndOffX = x;
    }
}

void ReqItemRightAlign(REQUEST_INFO *req, TEXTSTRING *txt)
{
    int x;
    uint32 scaleH = GetTextScaleH(txt->scaleH);

    if (txt)
    {
        x = (((req->pixwidth * scaleH) >> 16) / 2 - T_GetTextWidth(txt) / 2) - ((8 * scaleH) >> 16);
        txt->xpos = req->xpos + x;
        txt->bgndOffX = -x;
    }
}

extern SG_COL req_bgnd_gour1[];
extern SG_COL req_bgnd_gour2[];
extern SG_COL req_main_gour1[];
extern SG_COL req_main_gour2[];
extern SG_COL req_sel_gour2[];
extern SG_COL req_unsel_gour1[];
extern TEXTSTRING *PassportTextInfo;
extern sint32 input;

sint32 Display_Requester(REQUEST_INFO *req, int des, int backgrounds)
{
    int n;
    int linesheight = req->vis_lines * req->line_height + 10;
    int lineoneoff = req->ypos - linesheight;
    int lineqty = req->vis_lines;
    int nWidth = GetRenderWidth();
    int nHeight = GetRenderHeight();

    if (nWidth != req->original_render_width || nHeight != req->original_render_height)
    {
        Remove_Requester(req);
        req->original_render_width = nWidth;
        req->original_render_height = nHeight;
    }

    req->itemtexts1_flags = RequesterFlags1;
    req->itemtexts2_flags = RequesterFlags2;

    if (req->items < req->vis_lines)
        lineqty = req->items;

    if (req->heading1_flags & R_USE)
    {
        if (!req->heading1text)
        {
            req->heading1text = T_Print(req->xpos, lineoneoff - req->line_height - 10, req->zpos, req->heading1_str);
            T_CentreH(req->heading1text, 1);
            T_BottomAlign(req->heading1text, 1);
            if (backgrounds)
            {
                T_AddBackground(req->heading1text, req->pixwidth - 4, 0, 0, 0, 8, C_BLACK, req_main_gour1, 2);
                T_AddOutline(req->heading1text, 1, C_ORANGE, req_main_gour2, 0);
            }
        }
        if (req->heading1_flags & R_LEFTALIGN)
            ReqItemLeftAlign(req, req->heading1text);
        if (req->heading1_flags & R_RIGHTALIGN)
            ReqItemRightAlign(req, req->heading1text);
    }

    if (req->heading2_flags & R_USE)
    {
        if (!req->heading2text)
        {
            req->heading2text = T_Print(req->xpos, lineoneoff - req->line_height - 10, req->zpos, req->heading2_str);
            T_CentreH(req->heading2text, 1);
            T_BottomAlign(req->heading2text, 1);
            if (backgrounds)
            {
                T_AddBackground(req->heading2text, req->pixwidth - 4, 0, 0, 0, 8, C_BLACK, req_main_gour1, 2);
                T_AddOutline(req->heading2text, 1, C_ORANGE, req_main_gour2, 0);
            }
        }
        if (req->heading2_flags & R_LEFTALIGN)
            ReqItemLeftAlign(req, req->heading2text);
        if (req->heading2_flags & R_RIGHTALIGN)
            ReqItemRightAlign(req, req->heading2text);
    }

    if (backgrounds && !req->backgroundtext && (req->background_flags & R_USE))
    {
        req->backgroundtext = T_Print(req->xpos, lineoneoff - req->line_height - 12, 0, " ");
        T_CentreH(req->backgroundtext, 1);
        T_BottomAlign(req->backgroundtext, 1);
        T_AddBackground(req->backgroundtext, req->pixwidth, linesheight + req->line_height + 12, 0, 0, 48, C_BLACK, req_bgnd_gour1, 1);
        T_AddOutline(req->backgroundtext, 1, C_BLUE, req_bgnd_gour2, 0);
    }

    if (req->line_offset)
    {
        if (!req->moreuptext && (req->moreup_flags & R_USE))
        {
            T_CentreH(req->moreuptext, 1);
            T_BottomAlign(req->moreuptext, 1);
        }
    }
    else
    {
        T_RemovePrint(req->moreuptext);
        req->moreuptext = NULL;
    }

    if (req->items > req->line_offset + req->vis_lines)
    {
        if (!req->moredowntext && (req->moredown_flags & R_USE))
        {
            T_CentreH(req->moredowntext, 1);
            T_BottomAlign(req->moredowntext, 1);
        }
    }
    else
    {
        T_RemovePrint(req->moredowntext);
        req->moredowntext = NULL;
    }

    for (n = 0; n < lineqty; n++)
    {
        if (req->itemtexts1_flags[n + req->line_offset] & R_USE)
        {
            if (!req->texts1[n])
            {
                req->texts1[n] = T_Print(0, lineoneoff + n * req->line_height, 0, req->itemtexts1 + (n + req->line_offset) * req->itemtextlen);
                T_CentreH(req->texts1[n], 1);
                T_BottomAlign(req->texts1[n], 1);
            }
            if (req->noselector || n + req->line_offset != req->selected)
            {
                T_RemoveBackground(req->texts1[n]);
                T_RemoveOutline(req->texts1[n]);
            }
            else
            {
                T_AddBackground(req->texts1[n], req->pixwidth - 12, 0, 0, 0, 16, C_BLACK, req_unsel_gour1, 1);
                T_AddOutline(req->texts1[n], 1, C_ORANGE, req_sel_gour2, 0);
            }
            if (req->itemtexts1_flags[n + req->line_offset] & R_LEFTALIGN)
                ReqItemLeftAlign(req, req->texts1[n]);
            else if (req->itemtexts1_flags[n + req->line_offset] & R_RIGHTALIGN)
                ReqItemRightAlign(req, req->texts1[n]);
            else
                ReqItemCentreAlign(req, req->texts1[n]);
        }
        else
        {
            T_RemovePrint(req->texts1[n]);
            T_RemoveBackground(req->texts1[n]);
            T_RemoveOutline(req->texts1[n]);
            req->texts1[n] = NULL;
        }

        if (req->itemtexts2_flags[n + req->line_offset] & R_USE)
        {
            if (!req->texts2[n])
            {
                req->texts2[n] = T_Print(0, lineoneoff + n * req->line_height, 0, req->itemtexts2 + (n + req->line_offset) * req->itemtextlen);
                T_CentreH(req->texts2[n], 1);
                T_BottomAlign(req->texts2[n], 1);
            }
            if (req->itemtexts2_flags[n + req->line_offset] & R_LEFTALIGN)
                ReqItemLeftAlign(req, req->texts2[n]);
            else if (req->itemtexts2_flags[n + req->line_offset] & R_RIGHTALIGN)
                ReqItemRightAlign(req, req->texts2[n]);
            else
                ReqItemCentreAlign(req, req->texts2[n]);
        }
        else
        {
            T_RemovePrint(req->texts2[n]);
            T_RemoveBackground(req->texts2[n]);
            T_RemoveOutline(req->texts2[n]);
            req->texts2[n] = NULL;
        }
    }

    if (req->line_offset != req->line_oldoffset)
    {
        for (n = 0; n < lineqty; n++)
        {
            if (req->texts1[n] && (req->itemtexts1_flags[n + req->line_offset] & R_USE))
                T_ChangeText(req->texts1[n], req->itemtexts1 + (n + req->line_offset) * req->itemtextlen);
            if (req->itemtexts1_flags[n + req->line_offset] & R_LEFTALIGN)
                ReqItemLeftAlign(req, req->texts1[n]);
            else if (req->itemtexts1_flags[n + req->line_offset] & R_RIGHTALIGN)
                ReqItemRightAlign(req, req->texts1[n]);
            else
                ReqItemCentreAlign(req, req->texts1[n]);
            if (req->texts2[n] && (req->itemtexts2_flags[n + req->line_offset] & R_USE))
                T_ChangeText(req->texts2[n], req->itemtexts2 + (n + req->line_offset) * req->itemtextlen);
            if (req->itemtexts2_flags[n + req->line_offset] & R_LEFTALIGN)
                ReqItemLeftAlign(req, req->texts2[n]);
            else if (req->itemtexts2_flags[n + req->line_offset] & R_RIGHTALIGN)
                ReqItemRightAlign(req, req->texts2[n]);
            else
                ReqItemCentreAlign(req, req->texts2[n]);
        }
    }

    if (inputDB & IN_BACK)
    {
        if (req->noselector)
        {
            req->line_oldoffset = req->line_offset;
            if (req->line_offset + req->vis_lines < req->items)
                req->line_offset++;
        }
        else
        {
            if (req->selected < req->items - 1)
                req->selected++;
            req->line_oldoffset = req->line_offset;
            if (req->selected > req->line_offset + req->vis_lines - 1)
                req->line_offset++;
        }
        return 0;
    }
    else if (inputDB & IN_FORWARD)
    {
        if (req->noselector)
        {
            req->line_oldoffset = req->line_offset;
            if (req->line_offset > 0)
                req->line_offset--;
        }
        else
        {
            if (req->selected > 0)
                req->selected--;
            req->line_oldoffset = req->line_offset;
            if (req->selected < req->line_offset)
                req->line_offset--;
        }
        return 0;
    }
    else if (inputDB & SELECT_ITEM)
    {
        if (!strncmp(req->texts1[req->selected - req->line_offset]->string, GF_PCStrings[15], 12) && !strcmp(PassportTextInfo->string, GF_GameStrings[4]))
        {
            input = 0;
            return 0;
        }
        Remove_Requester(req);
        return req->selected + 1;
    }
    else if ((inputDB & DESELECT_ITEM) && des == 1)
    {
        Remove_Requester(req);
        return -1;
    }
    return 0;
}

void SetRequesterHeading(REQUEST_INFO *req, char *text1, uint32 flags1, char *text2, uint32 flags2)
{
    T_RemovePrint(req->heading1text);
    req->heading1text = NULL;
    T_RemovePrint(req->heading2text);
    req->heading2text = NULL;

    if (text1)
    {
        sprintf(req->heading1_str, text1);
        req->heading1_flags = R_USE | flags1;
    }
    else
    {
        sprintf(req->heading1_str, "u");
        req->heading1_flags = 0;
    }

    if (text2)
    {
        sprintf(req->heading2_str, text2);
        req->heading2_flags = R_USE | flags2;
    }
    else
    {
        sprintf(req->heading2_str, "u");
        req->heading2_flags = 0;
    }
}

void RemoveAllReqItems(REQUEST_INFO *req)
{
    req->items = 0;
    req->line_offset = 0;
    req->selected = 0;
}

void ChangeRequesterItem(REQUEST_INFO *req, int item, char *text1, uint32 flags1, char *text2, uint32 flags2)
{
    T_RemovePrint(req->texts1[item]);
    req->texts1[item] = NULL;
    T_RemovePrint(req->texts2[item]);
    req->texts2[item] = NULL;

    if (text1)
    {
        sprintf(req->itemtexts1 + (item * req->itemtextlen), text1);
        req->itemtexts1_flags[item] = R_USE | flags1;
    }
    else
        req->itemtexts1_flags[item] = 0;

    if (text2)
    {
        sprintf(req->itemtexts2 + (item * req->itemtextlen), text2);
        req->itemtexts2_flags[item] = R_USE | flags2;
    }
    else
        req->itemtexts2_flags[item] = 0;
}

void AddRequesterItem(REQUEST_INFO *req, char *text1, uint32 flags1, char *text2, uint32 flags2)
{
    req->itemtexts1_flags = RequesterFlags1;
    req->itemtexts2_flags = RequesterFlags2;

    if (text1)
    {
        sprintf(req->itemtexts1 + (req->items * req->itemtextlen), text1);
        req->itemtexts1_flags[req->items] = R_USE | flags1;
    }
    else
        req->itemtexts1_flags[req->items] = 0;

    if (text2)
    {
        sprintf(req->itemtexts2 + (req->items * req->itemtextlen), text2);
        req->itemtexts2_flags[req->items] = R_USE | flags2;
    }
    else
        req->itemtexts2_flags[req->items] = 0;

    req->items++;
}

#define TEXT_HEIGHT 11

void SetPCRequesterSize(REQUEST_INFO *req, int maxlines, int ypos)
{
    int height = GetRenderHeight();
    int lines = (height / 2) / (TEXT_HEIGHT + 7);

    if (lines > maxlines)
        lines = maxlines;
    req->ypos = ypos;
    req->vis_lines = lines;
}

#define NUM_ASS_TIMES 10

extern uint32 best_assault_times[NUM_ASS_TIMES];
extern uint32 best_assault_finish[NUM_ASS_TIMES];
extern uint32 finishes;

int AddAssaultTime(uint32 time)
{
    int i, j, insert = 0;

    for (i = 0; i < NUM_ASS_TIMES; i++)
    {
        if (best_assault_times[i] == 0 || time < best_assault_times[i])
        {
            insert = 1;
            break;
        }
    }

    if (insert)
    {
        for (j = NUM_ASS_TIMES - 1; j > i; j--)
        {
            best_assault_times[j] = best_assault_times[j - 1];
            best_assault_finish[j] = best_assault_finish[j - 1];
        }
        best_assault_times[i] = time;
        best_assault_finish[i] = finishes + 1;
        finishes++;
        return 1;
    }
    return 0;
}

extern REQUEST_INFO StatsRequester;
extern char SaveGameStrings1[][50];
extern char SaveGameStrings2[][50];

sint32 Display_Requester(REQUEST_INFO *req, int des, int backgrounds);

static int gym_mode;

void ShowGymStatsText(char *time_str, int type)
{
    int select, m, s, h, i;
    char str[32], str2[32];

    if (gym_mode == 1)
    {
        if ((select = Display_Requester(&StatsRequester, 1, 1)))
            gym_mode = 0;
        else
            input = inputDB = 0;
    }
    else
    {
        StatsRequester.noselector = 1;
        SetPCRequesterSize(&StatsRequester, 7, -32);
        StatsRequester.line_height = 18;
        StatsRequester.items = 0;
        StatsRequester.selected = 0;
        StatsRequester.line_offset = 0;
        StatsRequester.line_oldoffset = 0;
        StatsRequester.pixwidth = 304;
        StatsRequester.xpos = 0;
        StatsRequester.zpos = 0;
        StatsRequester.itemtexts1 = &SaveGameStrings1[0][0];
        StatsRequester.itemtexts2 = &SaveGameStrings2[0][0];
        StatsRequester.itemtextlen = 50;
        Init_Requester(&StatsRequester);
        SetRequesterHeading(&StatsRequester, GF_GameStrings[71], R_CENTRE, NULL, 0);
        for (i = 0; i < NUM_ASS_TIMES; i++)
        {
            if (best_assault_times[i] == 0)
                break;
            sprintf(str, "%2d: %s %d", i + 1, GF_GameStrings[70], (int)best_assault_finish[i]);
            s = best_assault_times[i] / 30;
            h = (best_assault_times[i] % 30) / 3;
            m = s / 60;
            s = s % 60;
            sprintf(str2, "%02d:%02d.%-2d", m, s, h);
            AddRequesterItem(&StatsRequester, str, R_LEFTALIGN, str2, R_RIGHTALIGN);
        }
        if (i == 0)
            AddRequesterItem(&StatsRequester, GF_GameStrings[72], R_CENTRE, NULL, 0);
        gym_mode = 1;
    }
}

extern sint16 GF_NumSecrets;

static int stats_mode;

void ShowStatsText(char *time_str, int type)
{
    int select, numsecrets, distance, len = 0;
    char str[32];

    if (stats_mode == 1)
    {
        ChangeRequesterItem(&StatsRequester, 0, GF_GameStrings[59], R_LEFTALIGN, time_str, R_RIGHTALIGN);
        if ((select = Display_Requester(&StatsRequester, type, 1)))
            stats_mode = 0;
        else
            input = inputDB = 0;
    }
    else
    {
        StatsRequester.noselector = 1;
        SetPCRequesterSize(&StatsRequester, 7, -32);
        StatsRequester.line_height = 18;
        StatsRequester.items = 0;
        StatsRequester.selected = 0;
        StatsRequester.line_offset = 0;
        StatsRequester.line_oldoffset = 0;
        StatsRequester.pixwidth = 304;
        StatsRequester.xpos = 0;
        StatsRequester.zpos = 0;
        StatsRequester.itemtexts1 = &SaveGameStrings1[0][0];
        StatsRequester.itemtexts2 = &SaveGameStrings2[0][0];
        StatsRequester.itemtextlen = 50;
        Init_Requester(&StatsRequester);
        SetRequesterHeading(&StatsRequester, GF_Level_Names[CurrentLevel], R_CENTRE, NULL, 0);
        AddRequesterItem(&StatsRequester, GF_GameStrings[59], R_LEFTALIGN, time_str, R_RIGHTALIGN);

        if (GF_NumSecrets)
        {
            numsecrets = (savegame.secrets & 1) + ((savegame.secrets & 2) >> 1) + ((savegame.secrets & 4) >> 2);
            if (numsecrets)
            {
                if (savegame.secrets & 1)
                    str[len++] = 127;
                else
                {
                    str[len++] = ' ';
                    str[len++] = ' ';
                    str[len++] = ' ';
                }
                if (savegame.secrets & 2)
                    str[len++] = (char)128;
                else
                {
                    str[len++] = ' ';
                    str[len++] = ' ';
                    str[len++] = ' ';
                }
                if (savegame.secrets & 4)
                    str[len++] = (char)129;
                else
                {
                    str[len++] = ' ';
                    str[len++] = ' ';
                    str[len++] = ' ';
                }
                str[len] = 0;
            }
            else
                sprintf(str, GF_GameStrings[69]);
            AddRequesterItem(&StatsRequester, GF_GameStrings[60], R_LEFTALIGN, str, R_RIGHTALIGN);
        }

        sprintf(str, "%d", savegame.kills);
        AddRequesterItem(&StatsRequester, GF_GameStrings[62], R_LEFTALIGN, str, R_RIGHTALIGN);
        sprintf(str, "%d", (int)savegame.ammo_used);
        AddRequesterItem(&StatsRequester, GF_GameStrings[63], R_LEFTALIGN, str, R_RIGHTALIGN);
        sprintf(str, "%d", (int)savegame.ammo_hit);
        AddRequesterItem(&StatsRequester, GF_GameStrings[64], R_LEFTALIGN, str, R_RIGHTALIGN);
        if (savegame.health_used & 1)
            sprintf(str, "%d.5", savegame.health_used >> 1);
        else
            sprintf(str, "%d.0", savegame.health_used >> 1);
        AddRequesterItem(&StatsRequester, GF_GameStrings[67], R_LEFTALIGN, str, R_RIGHTALIGN);

        distance = savegame.distance_travelled / 445;
        if (distance >= 1000)
            sprintf(str, "%d.%02dkm", distance / 1000, distance % 100);
        else
            sprintf(str, "%dm", distance);
        AddRequesterItem(&StatsRequester, GF_GameStrings[66], R_LEFTALIGN, str, R_RIGHTALIGN);
        stats_mode = 1;
    }
}


static int end_stats_mode;

void ShowEndStatsText(void)
{
    int select, numsecrets, distance, i, num;
    char str[32];
    int h = 0, m = 0, s = 0;
    int totallevels = gameflow.num_levels - gameflow.num_demos;

    if (end_stats_mode == 1)
    {
        if ((select = Display_Requester(&StatsRequester, 0, 1)))
            end_stats_mode = 0;
        else
            input = inputDB = 0;
    }
    else
    {
        StatsRequester.noselector = 1;
        SetPCRequesterSize(&StatsRequester, 7, -32);
        StatsRequester.line_height = 18;
        StatsRequester.items = 0;
        StatsRequester.selected = 0;
        StatsRequester.line_offset = 0;
        StatsRequester.line_oldoffset = 0;
        StatsRequester.pixwidth = 304;
        StatsRequester.xpos = 0;
        StatsRequester.zpos = 0;
        StatsRequester.itemtexts1 = &SaveGameStrings1[0][0];
        StatsRequester.itemtexts2 = &SaveGameStrings2[0][0];
        StatsRequester.itemtextlen = 50;
        Init_Requester(&StatsRequester);
        SetRequesterHeading(&StatsRequester, GF_GameStrings[75], R_CENTRE, NULL, 0);

        s = 0;
        for (i = 1; i < totallevels; i++)
            s += savegame.start[i].timer;
        s = s / 30;
        h = s / 3600;
        m = (s / 60) % 60;
        s = s % 60;
        sprintf(str, "%02d:%02d:%02d", h, m, s);
        AddRequesterItem(&StatsRequester, GF_GameStrings[59], R_LEFTALIGN, str, R_RIGHTALIGN);

        numsecrets = num = 0;
        for (i = 1; i < totallevels - 2; i++)
        {
            numsecrets += (savegame.start[i].secrets_found & 1) + ((savegame.start[i].secrets_found & 2) >> 1) + ((savegame.start[i].secrets_found & 4) >> 2);
            num += 3;
        }
        sprintf(str, "%d %s %d", numsecrets, GF_GameStrings[76], num);
        AddRequesterItem(&StatsRequester, GF_GameStrings[60], R_LEFTALIGN, str, R_RIGHTALIGN);

        num = 0;
        for (i = 1; i < totallevels; i++)
            num += savegame.start[i].kills;
        sprintf(str, "%d", num);
        AddRequesterItem(&StatsRequester, GF_GameStrings[62], R_LEFTALIGN, str, R_RIGHTALIGN);

        num = 0;
        for (i = 1; i < totallevels; i++)
            num += savegame.start[i].ammo_used;
        sprintf(str, "%d", num);
        AddRequesterItem(&StatsRequester, GF_GameStrings[63], R_LEFTALIGN, str, R_RIGHTALIGN);

        num = 0;
        for (i = 1; i < totallevels; i++)
            num += savegame.start[i].ammo_hit;
        sprintf(str, "%d", num);
        AddRequesterItem(&StatsRequester, GF_GameStrings[64], R_LEFTALIGN, str, R_RIGHTALIGN);

        num = 0;
        for (i = 1; i < totallevels; i++)
            num += savegame.start[i].health_used;
        if (num & 1)
            sprintf(str, "%d.5", num >> 1);
        else
            sprintf(str, "%d.0", num >> 1);
        AddRequesterItem(&StatsRequester, GF_GameStrings[67], R_LEFTALIGN, str, R_RIGHTALIGN);

        num = 0;
        for (i = 1; i < totallevels; i++)
            num += savegame.start[i].distance_travelled;
        distance = num / 445;
        if (distance >= 1000)
            sprintf(str, "%d.%02dkm", distance / 1000, distance % 100);
        else
            sprintf(str, "%dm", distance);
        AddRequesterItem(&StatsRequester, GF_GameStrings[66], R_LEFTALIGN, str, R_RIGHTALIGN);
        end_stats_mode = 1;
    }
}
