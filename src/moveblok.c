#include "game.h"
#include "box.h"

void AlterFloorHeight(ITEM_INFO *item, int height);

void InitialiseMovingBlock(sint16 item_num)
{
    ITEM_INFO *item;

    item = &items[item_num];

    if (item->status != INVISIBLE)
        AlterFloorHeight(item, -1024);
}


void MovableBlock(sint16 item_number)
{
    ITEM_INFO *item;
    FLOOR_INFO *floor;
    int height;
    sint16 room_number;

    item = &items[item_number];

    if (item->flags & ONESHOT) {
        AlterFloorHeight(item, 1024);
        KillItem(item_number);
        return;
    }

    AnimateItem(item);

    room_number = item->room_number;
    floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
    height = GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);

    if (item->pos.y_pos < height)
        item->gravity_status = 1;
    else if (item->gravity_status) {
        item->gravity_status = 0;
        item->pos.y_pos = height;
        item->status = DEACTIVATED;
        floor_shake_effect(item);
        SoundEffect(70, &item->pos, 0);
    }

    if (item->room_number != room_number)
        ItemNewRoom(item_number, room_number);

    if (item->status == DEACTIVATED) {
        item->status = NOT_ACTIVE;
        RemoveActiveItem(item_number);
        AlterFloorHeight(item, -1024);

        room_number = item->room_number;
        floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
        GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);
        TestTriggers(trigger_index, 1);
    }
}

enum {
    NORTH,
    EAST,
    SOUTH,
    WEST
};

#define PUSH 2
#define PULL 3
#define PPREADY_A 120
#define PPREADY_F GF(PPREADY_A, 19)

extern sint16 MovingBlockBounds[];

int TestBlockPush(ITEM_INFO *item, int blokhite, uint16 quadrant);
int TestBlockPull(ITEM_INFO *item, int blokhite, uint16 quadrant);
void AnimateLara(ITEM_INFO *item);

void MovableBlockCollision(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll)
{
    uint16 quadrant;
    ITEM_INFO *item;
    FLOOR_INFO *floor;
    sint16 room_number;

    item = &items[item_num];
    if (!(input & IN_ACTION) || item->status == ACTIVE || laraitem->gravity_status
        || laraitem->pos.y_pos != item->pos.y_pos)
        return;

    quadrant = (uint16)(laraitem->pos.y_rot + 0x2000) / 0x4000;
    if (laraitem->current_anim_state == AS_STOP) {
        if (lara.gun_status != LG_ARMLESS)
            return;

        switch (quadrant) {
        case NORTH:
            item->pos.y_rot = 0;
            break;

        case EAST:
            item->pos.y_rot = 16384;
            break;

        case SOUTH:
            item->pos.y_rot = -32768;
            break;

        case WEST:
            item->pos.y_rot = -16384;
            break;
        }
        if (!TestLaraPosition(MovingBlockBounds, item, laraitem))
            return;

        room_number = laraitem->room_number;
        floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
        if (room_number != item->room_number)
            return;

        switch (quadrant) {
        case NORTH:
            laraitem->pos.z_pos &= -WALL_L;
            laraitem->pos.z_pos += WALL_L - LARA_RAD;
            break;

        case SOUTH:
            laraitem->pos.z_pos &= -WALL_L;
            laraitem->pos.z_pos += LARA_RAD;
            break;

        case EAST:
            laraitem->pos.x_pos &= -WALL_L;
            laraitem->pos.x_pos += WALL_L - LARA_RAD;
            break;

        case WEST:
            laraitem->pos.x_pos &= -WALL_L;
            laraitem->pos.x_pos += LARA_RAD;
            break;
        }
        laraitem->pos.y_rot = item->pos.y_rot;
        laraitem->goal_anim_state = AS_PPREADY;
        AnimateLara(laraitem);
        if (laraitem->current_anim_state == AS_PPREADY)
            lara.gun_status = LG_HANDSBUSY;
    } else if (laraitem->current_anim_state == AS_PPREADY) {
        if (laraitem->frame_number != PPREADY_F)
            return;
        if (!TestLaraPosition(MovingBlockBounds, item, laraitem))
            return;
        if (input & IN_FORWARD) {
            if (!TestBlockPush(item, 1024, quadrant))
                return;
            item->goal_anim_state = PUSH;
            laraitem->goal_anim_state = AS_PUSHBLOCK;
        } else if (input & IN_BACK) {
            if (!TestBlockPull(item, 1024, quadrant))
                return;
            item->goal_anim_state = PULL;
            laraitem->goal_anim_state = AS_PULLBLOCK;
        } else
            return;
        AddActiveItem(item_num);
        AlterFloorHeight(item, 1024);
        item->status = ACTIVE;
        AnimateItem(item);
        AnimateLara(laraitem);
    }
}

int TestBlockMovable(ITEM_INFO *item, int blokhite)
{
    sint16 room_number;
    FLOOR_INFO *floor;

    room_number = item->room_number;
    floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
    if (floor->floor == NO_HEIGHT / 256)
        return 1;
    if (floor->floor * 256 != item->pos.y_pos - blokhite)
        return 0;
    return 1;
}

void AlterFloorHeight(ITEM_INFO *item, int height)
{
    FLOOR_INFO *floor, *ceiling;
    sint16 room_num;

    room_num = item->room_number;
    floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_num);
    ceiling = GetFloor(item->pos.x_pos, item->pos.y_pos + height - WALL_L, item->pos.z_pos, &room_num);

    if (floor->floor == NO_HEIGHT / 256)
        floor->floor = ceiling->ceiling + height / 256;
    else
    {
        floor->floor += height / 256;
        if (floor->floor == ceiling->ceiling)
            floor->floor = NO_HEIGHT / 256;
    }

    if (boxes[floor->box].overlap_index & BLOCKABLE)
    {
        if (height < 0)
            boxes[floor->box].overlap_index |= BLOCKED;
        else
            boxes[floor->box].overlap_index &= (~BLOCKED);
    }
}

void DrawMovableBlock(ITEM_INFO *item)
{
    if (item->status != ACTIVE)
        DrawAnimatingItem(item);
    else
        DrawUnclippedItem(item);
}

void DrawUnclippedItem(ITEM_INFO *item)
{
    int left, right, top, bottom;

    left = phd_left;
    right = phd_right;
    top = phd_top;
    bottom = phd_bottom;
    phd_left = phd_top = 0;
    phd_bottom = phd_winymax;
    phd_right = phd_winxmax;
    DrawAnimatingItem(item);
    phd_left = left;
    phd_right = right;
    phd_top = top;
    phd_bottom = bottom;
}
