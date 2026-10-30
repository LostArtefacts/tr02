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

#define MENU_BACKGROUND 223

void phd_GetVectorAngles(sint32 x, sint32 y, sint32 z, PHD_ANGLE *dest);
void phd_GenerateW2V(PHD_3DPOS *viewpos);
void phd_RotateLight(sint16 pitch, sint16 yaw);
void phd_RotYXZsuperpack(sint16 **pprot, int skip);
void S_InsertInvBgnd(sint16 *objptr);
extern sint32 ls_divider;

static int bz = 12288;

void DoInventoryBackground(void)
{
    PHD_ANGLE angles[2];
    sint16 *frame;
    PHD_3DPOS viewer;

    S_CopyBufferToScreen();
    if (!objects[MENU_BACKGROUND].loaded)
        return;

    phd_GetVectorAngles(0, 0x1000, 0, angles);
    viewer.x_pos = 0;
    viewer.y_pos = -0x200;
    viewer.z_pos = 0;
    viewer.x_rot = angles[1];
    viewer.y_rot = angles[0];
    viewer.z_rot = 0;
    phd_GenerateW2V(&viewer);

    ls_divider = 0x6000;
    phd_GetVectorAngles(-0x600, 0x100, 0x400, angles);
    phd_RotateLight(angles[1], angles[0]);

    phd_PushMatrix();
    phd_TranslateAbs(0, bz, 0);
    phd_RotYXZ(0, 0x4000, (sint16)0x8000);
    frame = anims[objects[MENU_BACKGROUND].anim_index].frame_ptr + 9;
    phd_RotYXZsuperpack(&frame, 0);
    phd_RotYXZ((sint16)0x8000, 0, 0);
    S_InsertInvBgnd(meshes[objects[MENU_BACKGROUND].mesh_index]);
    phd_PopMatrix();
}
