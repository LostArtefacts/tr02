#include "game.h"
#include "box.h"

#define ESCAPE_DIST (WALL_L * 5)

int SearchLOT(LOT_INFO *LOT, int expansion);

void InitialiseCreature(sint16 item_number)
{
    ITEM_INFO *item;

    item = &items[item_number];

    item->pos.y_rot += (GetRandomControl() - 0x4000) >> 1;
    item->collidable = 1;
    item->data = NULL;
}

int UpdateLOT(LOT_INFO *LOT, int expansion)
{
    BOX_NODE *expand;

    if (LOT->required_box != NO_BOX && LOT->required_box != LOT->target_box)
    {
        LOT->target_box = LOT->required_box;

        expand = &LOT->node[LOT->target_box];
        if (expand->next_expansion == NO_BOX && LOT->tail != LOT->target_box)
        {
            expand->next_expansion = LOT->head;

            if (LOT->head == NO_BOX)
                LOT->tail = LOT->target_box;

            LOT->head = LOT->target_box;
        }

        expand->search_number = ++LOT->search_number;
        expand->exit_box = NO_BOX;
    }

    return SearchLOT(LOT, expansion);
}

void TargetBox(LOT_INFO *LOT, sint16 box_number)
{
    BOX_INFO *box;

    box_number &= BOX_NUMBER;

    box = &boxes[box_number];

    LOT->target.z = (((uint32)box->left << WALL_SHIFT)) + GetRandomControl() * (((uint32)box->right - (uint32)box->left - 1) >> (15 - WALL_SHIFT)) + WALL_L / 2;
    LOT->target.x = ((uint32)box->top << WALL_SHIFT) + GetRandomControl() * (((uint32)box->bottom - (uint32)box->top - 1) >> (15 - WALL_SHIFT)) + WALL_L / 2;
    LOT->required_box = box_number;

    if (LOT->fly == NO_FLYING)
        LOT->target.y = box->height;
    else
        LOT->target.y = box->height - STEP_L * 3 / 2;
}

int EscapeBox(ITEM_INFO *item, ITEM_INFO *enemy, sint16 box_number)
{
    BOX_INFO *box;
    sint32 x, z;

    box = &boxes[box_number];
    z = (((uint32)box->left + (uint32)box->right) << (WALL_SHIFT - 1)) - enemy->pos.z_pos;
    x = (((uint32)box->top + (uint32)box->bottom) << (WALL_SHIFT - 1)) - enemy->pos.x_pos;

    if (x > -ESCAPE_DIST && x < ESCAPE_DIST && z > -ESCAPE_DIST && z < ESCAPE_DIST)
        return 0;

    if (((z > 0) ^ (item->pos.z_pos > enemy->pos.z_pos)) && ((x > 0) ^ (item->pos.x_pos > enemy->pos.x_pos)))
        return 0;

    return 1;
}

void CreatureTilt(ITEM_INFO *item, sint16 angle)
{
    angle = (angle << 2) - item->pos.z_rot;
    if (angle < -MAX_TILT)
        angle = -MAX_TILT;
    else if (angle > MAX_TILT)
        angle = MAX_TILT;

    item->pos.z_rot += angle;
}

void CreatureUnderwater(ITEM_INFO *item, sint32 depth)
{
    sint32 water_level;

    water_level = GetWaterHeight(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, item->room_number);
    if (item->pos.y_pos < water_level + depth)
    {
        item->pos.y_pos = water_level + depth;
        if (item->pos.x_rot > 2 * ONE_DEGREE)
            item->pos.x_rot -= 2 * ONE_DEGREE;
        else if (item->pos.x_rot > 0)
            item->pos.x_rot = 0;
    }
}

sint16 CreatureEffect(ITEM_INFO *item, BITE_INFO *bite, sint16 (*generate)(sint32 x, sint32 y, sint32 z, sint16 speed, sint16 yrot, sint16 room_number))
{
    PHD_VECTOR pos;

    pos.x = bite->x;
    pos.y = bite->y;
    pos.z = bite->z;
    GetJointAbsPosition(item, &pos, bite->mesh_num);
    return ((*generate)(pos.x, pos.y, pos.z, item->speed, item->pos.y_rot, item->room_number));
}
