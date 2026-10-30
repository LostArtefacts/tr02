#include "game.h"

#include "inventry.h"
#include "specific/display.h"

#define TITLE_RING_OBJECTS 4
#define OPTION_RING_OBJECTS 3

void S_SetupAboveWater(int underwater);

extern int HiRes;
extern sint16 IsInventoryActive;
extern sint16 InventoryChosen;
extern int InventoryExtraData[];
extern sint16 inv_main_objects, inv_main_current;
extern sint16 InvOptionObjectsCount, InvOptionCurrent;
extern INVENTORY_ITEM *inv_main_list[];
extern INVENTORY_ITEM *InvOptionList[];
extern int GymInvOpenEnabled;

void Construct_Inventory(void)
{
    int n;
    INVENTORY_ITEM *inv_item;

    S_SetupAboveWater(0);
    if (Inventory_Mode != INV_TITLE_MODE)
        TempVideoAdjust(HiRes, 1.0);

    phd_left = 0;
    phd_top = 0;
    phd_right = phd_winxmax;
    phd_bottom = phd_winymax;

    IsInventoryActive = 1;
    InventoryChosen = 0;
    for (n = 0; n < 8; n++)
        InventoryExtraData[n] = 0;

    if (Inventory_Mode == INV_TITLE_MODE)
    {
        if (gameflow.gym_enabled)
            InvOptionObjectsCount = TITLE_RING_OBJECTS;
        else
            InvOptionObjectsCount = TITLE_RING_OBJECTS - 1;
    }
    else
        InvOptionObjectsCount = OPTION_RING_OBJECTS;

    for (n = 0; n < inv_main_objects; n++)
    {
        inv_item = inv_main_list[n];
        inv_item->drawn_meshes = inv_item->which_meshes;
        inv_item->current_frame = 0;
        inv_item->goal_frame = inv_item->current_frame;
        inv_item->anim_count = 0;
        inv_item->y_rot = 0;
    }

    for (n = 0; n < InvOptionObjectsCount; n++)
    {
        inv_item = InvOptionList[n];
        inv_item->current_frame = 0;
        inv_item->goal_frame = inv_item->current_frame;
        inv_item->anim_count = 0;
        inv_item->y_rot = 0;
    }

    inv_main_current = 0;
    if (GymInvOpenEnabled && Inventory_Mode == INV_TITLE_MODE && !gameflow.loadsave_disabled && gameflow.gym_enabled)
        InvOptionCurrent = 3;
    else
    {
        InvOptionCurrent = 0;
        GymInvOpenEnabled = 0;
    }
    item_data = 0;
}

void DoInventoryPicture(void)
{
    S_CopyBufferToScreen();
}
