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

#define PASSPORT_OPTION 120
#define MAP_OPTION 121
#define PFRONT 1
#define PSPINE 2
#define PINFRONT 4
#define PPAGE2 8
#define PBACK 16
#define PINBACK 32
#define PPAGE1 64
#define PASS_MESH (PFRONT | PSPINE | PBACK)

void SelectMeshes(INVENTORY_ITEM *inv_item)
{
    int frame;

    if (inv_item->object_number == PASSPORT_OPTION) {
        frame = inv_item->current_frame;
        if (frame < 4)
            inv_item->drawn_meshes = PASS_MESH | PINFRONT;
        else if (frame <= 16)
            inv_item->drawn_meshes = PASS_MESH | PINFRONT | PPAGE1;
        else if (frame < 19)
            inv_item->drawn_meshes = PASS_MESH | PINFRONT | PPAGE1 | PPAGE2;
        else if (frame == 19)
            inv_item->drawn_meshes = PASS_MESH | PPAGE1 | PPAGE2;
        else if (frame < 24)
            inv_item->drawn_meshes = PASS_MESH | PPAGE1 | PPAGE2 | PINBACK;
        else if (frame < 29)
            inv_item->drawn_meshes = PASS_MESH | PPAGE2 | PINBACK;
        else if (frame == 29)
            inv_item->drawn_meshes = PASS_MESH;
    } else if (inv_item->object_number == MAP_OPTION) {
        frame = inv_item->current_frame;
        if (frame && frame < 18)
            inv_item->drawn_meshes = 0xffffffff;
        else
            inv_item->drawn_meshes = inv_item->which_meshes;
    } else
        inv_item->drawn_meshes = 0xffffffff;
}

sint32 AnimateInventoryItem(INVENTORY_ITEM *inv_item)
{
    if (inv_item->current_frame != inv_item->goal_frame) {
        if (inv_item->anim_count)
            inv_item->anim_count--;
        else {
            inv_item->anim_count = inv_item->anim_speed;
            inv_item->current_frame += inv_item->anim_direction;
            if (inv_item->current_frame >= inv_item->frames_total)
                inv_item->current_frame = 0;
            else if (inv_item->current_frame < 0)
                inv_item->current_frame = inv_item->frames_total - 1;
        }
        SelectMeshes(inv_item);
        return 1;
    }
    SelectMeshes(inv_item);
    return 0;
}

#include "savegame.h"

#define ALPHABET 255
#define SHAPE_SPRITE 1
#define SHAPE_LINE 2
#define SHAPE_BOX 3
#define SHAPE_FBOX 4

typedef struct {
    sint16 mesh_number;
    uint16 flags;
    sint16 x_minp, x_maxp, y_minp, y_maxp, z_minp, z_maxp;
    sint16 x_minc, x_maxc, y_minc, y_maxc, z_minc, z_maxc;
} STATIC_INFO;

extern STATIC_INFO StaticObjects[];
extern sint32 PhdPersp;

void phd_RotYXZ(PHD_ANGLE ry, PHD_ANGLE rx, PHD_ANGLE rz);
void phd_RotZ(PHD_ANGLE rz);
void phd_RotYXZsuperpack(sint16 **pprot, int skip);
int S_GetObjectBounds(sint16 *bptr);
void S_DrawSprite(uint32 dwFlags, sint32 nX, sint32 nY, sint32 nZ, sint16 nSprite, sint16 nShade, sint16 nScale);
void S_DrawScreenSprite(sint32 sx, sint32 sy, sint32 z, sint32 scaleH, sint32 scaleV, sint16 sprnum, sint16 shade, uint16 flags);
void S_DrawScreenLine(sint32 sx, sint32 sy, sint32 z, sint32 w, sint32 h, sint32 col, uint16 *grdptr, uint16 flags);
void S_DrawScreenBox(sint32 sx, sint32 sy, sint32 z, sint32 w, sint32 h, sint32 col, uint16 *grdptr, uint16 flags);
void S_DrawScreenFBox(sint32 sx, sint32 sy, sint32 z, sint32 w, sint32 h, sint32 col, uint16 *grdptr, uint16 flags);

void DrawInventoryItem(INVENTORY_ITEM *inv_item)
{
    int hours, minutes, seconds, totsec;
    int clip, i, poppush;
    sint16 mesh, mesh_num;
    OBJECT_INFO *object;
    sint32 *bone;
    sint16 *rotation;
    sint16 *frame;
    INVENTORY_SPRITE *isprite, **isprite_list;
    sint32 xv, yv, zv, zp;
    sint32 sx, sy;

    if (inv_item->object_number == MAP_OPTION)
    {
        totsec = savegame.timer / 30;
        seconds = totsec % 60;
        minutes = ((totsec % 3600) * -91) / 5;
        hours = ((totsec / 12) * -91) / 5;
        seconds *= -1092;
    }
    else
        hours = minutes = seconds = totsec = 0;

    phd_TranslateRel(0, inv_item->ytrans, inv_item->ztrans);
    phd_RotYXZ(inv_item->y_rot, inv_item->x_rot, 0);

    object = &objects[inv_item->object_number];
    if (!object->loaded)
        return;

    if (object->nmeshes < 0)
    {
        mesh = object->mesh_index;
        S_DrawSprite(0, 0, 0, 0, mesh, 0, 0);
        return;
    }

    if ((isprite_list = inv_item->sprlist))
    {
        zv = *(phd_mxptr + M23);
        xv = *(phd_mxptr + M03);
        yv = *(phd_mxptr + M13);
        zp = zv / PhdPersp;
        sx = (sint32)(xv / zp + phd_centerx);
        sy = (sint32)(yv / zp + phd_centery);
        while ((isprite = *isprite_list++))
        {
            if (zv < phd_znear || zv > phd_zfar)
                break;
            while (isprite->shape)
            {
                switch (isprite->shape)
                {
                case SHAPE_SPRITE:
                    S_DrawScreenSprite(sx + isprite->x, sy + isprite->y, isprite->z, isprite->param1, isprite->param2, (sint16)(StaticObjects[ALPHABET].mesh_number + isprite->sprnum), 16 * 256, 0);
                    break;
                case SHAPE_LINE:
                    S_DrawScreenLine(sx + isprite->x, sy + isprite->y, isprite->z, isprite->param1, isprite->param2, isprite->sprnum, isprite->grdptr, 0);
                    break;
                case SHAPE_BOX:
                    S_DrawScreenBox(sx + isprite->x, sy + isprite->y, isprite->z, isprite->param1, isprite->param2, isprite->sprnum, isprite->grdptr, 0);
                    break;
                case SHAPE_FBOX:
                    S_DrawScreenFBox(sx + isprite->x, sy + isprite->y, isprite->z, isprite->param1, isprite->param2, isprite->sprnum, isprite->grdptr, 0);
                    break;
                default:
                    break;
                }
                isprite++;
            }
        }
    }

    frame = object->frame_base + inv_item->current_frame * (anims[object->anim_index].interpolation >> 8);

    phd_PushMatrix();
    if ((clip = S_GetObjectBounds(frame)))
    {
        phd_TranslateRel((sint32)*(frame + 6), (sint32)*(frame + 7), (sint32)*(frame + 8));
        rotation = frame + 9;
        phd_RotYXZsuperpack(&rotation, 0);

        mesh = object->mesh_index;
        bone = bones + object->bone_index;

        mesh_num = 1;
        if (mesh_num & inv_item->drawn_meshes)
            phd_PutPolygons(meshes[mesh], clip);

        for (i = object->nmeshes - 1; i > 0; i--)
        {
            mesh++;
            mesh_num <<= 1;

            poppush = *(bone++);
            if (poppush & 1)
                phd_PopMatrix();
            if (poppush & 2)
                phd_PushMatrix();

            phd_TranslateRel(*(bone), *(bone + 1), *(bone + 2));
            phd_RotYXZsuperpack(&rotation, 0);

            if (inv_item->object_number == MAP_OPTION)
            {
                if (i == 2)
                {
                    phd_RotZ((PHD_ANGLE)seconds);
                    inv_item->misc_data[1] = inv_item->misc_data[0];
                    inv_item->misc_data[0] = seconds;
                }
                if (i == 3)
                    phd_RotZ((PHD_ANGLE)minutes);
                if (i == 4)
                    phd_RotZ((PHD_ANGLE)hours);
            }

            if (mesh_num & inv_item->drawn_meshes)
                phd_PutPolygons(meshes[mesh], clip);
            bone += 3;
        }
    }
    phd_PopMatrix();
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
