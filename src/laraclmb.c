#include "game.h"

#define CAM_A_CLIMBL (-30 * ONE_DEGREE)
#define CAM_E_CLIMBL (-15 * ONE_DEGREE)
#define CAM_A_CLIMBR (30 * ONE_DEGREE)
#define CAM_E_CLIMBR (-15 * ONE_DEGREE)
#define CAM_E_CLIMBS (-20 * ONE_DEGREE)
#define CAM_E_CLIMBU (30 * ONE_DEGREE)
#define CAM_E_CLIMBD (-45 * ONE_DEGREE)
#define CLIMB_HITE (STEP_L * 2)
#define CLIMB_WIDTHL 80
#define CLIMB_WIDTHR 120

int LaraCheckForLetGo(ITEM_INFO *item, COLL_INFO *coll);
int LaraTestClimbPos(ITEM_INFO *item, int front, int right, int origin, int height, int *shift);
void LaraDoClimbLeftRight(ITEM_INFO *item, COLL_INFO *coll, int result, int shift);

void lara_as_climbleft(ITEM_INFO *item, COLL_INFO *coll)
{
    coll->enable_spaz = 0;
    coll->enable_baddie_push = 0;
    camera.target_angle = CAM_A_CLIMBL;
    camera.target_elevation = CAM_E_CLIMBL;
    if (!(input & IN_LEFT) && !(input & IN_STEPL))
        item->goal_anim_state = AS_CLIMBSTNC;
}

void lara_as_climbright(ITEM_INFO *item, COLL_INFO *coll)
{
    coll->enable_spaz = 0;
    coll->enable_baddie_push = 0;
    camera.target_angle = CAM_A_CLIMBR;
    camera.target_elevation = CAM_E_CLIMBR;
    if (!(input & IN_RIGHT) && !(input & IN_STEPR))
        item->goal_anim_state = AS_CLIMBSTNC;
}

void lara_as_climbstnc(ITEM_INFO *item, COLL_INFO *coll)
{
    coll->enable_spaz = 0;
    coll->enable_baddie_push = 0;
    camera.target_elevation = CAM_E_CLIMBS;

    if (input & IN_LOOK)
        LookUpDown();

    if (input & IN_LEFT || input & IN_STEPL)
        item->goal_anim_state = AS_CLIMBLEFT;
    else if (input & IN_RIGHT || input & IN_STEPR)
        item->goal_anim_state = AS_CLIMBRIGHT;
    else if (input & IN_JUMP)
    {
        item->goal_anim_state = AS_BACKJUMP;
        lara.move_angle = item->pos.y_rot - 32768;
        lara.gun_status = LG_ARMLESS;
    }
}

void lara_as_climbing(ITEM_INFO *item, COLL_INFO *coll)
{
    coll->enable_spaz = 0;
    coll->enable_baddie_push = 0;
    camera.target_elevation = CAM_E_CLIMBU;
}

void lara_as_climbend(ITEM_INFO *item, COLL_INFO *coll)
{
    coll->enable_spaz = 0;
    coll->enable_baddie_push = 0;
    camera.flags = FOLLOW_CENTRE;
    camera.target_angle = -45 * ONE_DEGREE;
}

void lara_as_climbdown(ITEM_INFO *item, COLL_INFO *coll)
{
    coll->enable_spaz = 0;
    coll->enable_baddie_push = 0;
    camera.target_elevation = CAM_E_CLIMBD;
}

void lara_col_climbleft(ITEM_INFO *item, COLL_INFO *coll)
{
    int result, shift;

    if (LaraCheckForLetGo(item, coll))
        return;

    lara.move_angle = item->pos.y_rot - 16384;

    result = LaraTestClimbPos(item, coll->radius, -(coll->radius + CLIMB_WIDTHL), -CLIMB_HITE, CLIMB_HITE, &shift);

    LaraDoClimbLeftRight(item, coll, result, shift);
}

void lara_col_climbright(ITEM_INFO *item, COLL_INFO *coll)
{
    int result, shift;

    if (LaraCheckForLetGo(item, coll))
        return;

    lara.move_angle = item->pos.y_rot + 16384;

    result = LaraTestClimbPos(item, coll->radius, (coll->radius + CLIMB_WIDTHR), -CLIMB_HITE, CLIMB_HITE, &shift);

    LaraDoClimbLeftRight(item, coll, result, shift);
}


#define FALLDOWN_A 34

int LaraCheckForLetGo(ITEM_INFO *item, COLL_INFO *coll)
{
    sint16 room_number;
    FLOOR_INFO *floor;

    item->gravity_status = item->fallspeed = 0;

    room_number = item->room_number;
    floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
    GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);
    coll->trigger = trigger_index;

    if (!(input & IN_ACTION) || item->hit_points <= 0)
    {
        item->goal_anim_state = AS_FORWARDJUMP;
        item->current_anim_state = AS_FORWARDJUMP;
        item->anim_number = FALLDOWN_A;
        item->frame_number = anims[FALLDOWN_A].frame_base;
        item->gravity_status = 1;
        item->speed = 2;
        item->fallspeed = 1;
        lara.gun_status = LG_ARMLESS;
        return 1;
    }
    return 0;
}


enum direction {
    NORTH,
    EAST,
    SOUTH,
    WEST
};

int LaraTestClimb(int x, int y, int z, int xfront, int zfront, int item_height, sint16 item_room, int *shift);

int LaraTestClimbPos(ITEM_INFO *item, int front, int right, int origin, int height, int *shift)
{
    int angle, x, z;
    int xfront = 0, zfront = 0;

    angle = (uint16)(item->pos.y_rot + 0x2000) / 0x4000;
    switch (angle)
    {
    case NORTH:
        z = item->pos.z_pos + front;
        x = item->pos.x_pos + right;
        zfront = 2;
        break;
    case EAST:
        x = item->pos.x_pos + front;
        z = item->pos.z_pos - right;
        xfront = 2;
        break;
    case SOUTH:
        z = item->pos.z_pos - front;
        x = item->pos.x_pos - right;
        zfront = -2;
        break;
    case WEST:
        x = item->pos.x_pos - front;
        z = item->pos.z_pos + right;
        xfront = -2;
        break;
    }

    return LaraTestClimb(x, item->pos.y_pos + origin, z, xfront, zfront, height, item->room_number, shift);
}


#define CLIMBSTNC_A 164
#define CLIMBSTNC_F GF(CLIMBSTNC_A, 0)

void AnimateLara(ITEM_INFO *item);

void LaraDoClimbLeftRight(ITEM_INFO *item, COLL_INFO *coll, int result, int shift)
{
    if (result == 1)
    {
        if (input & IN_LEFT)
            item->goal_anim_state = AS_CLIMBLEFT;
        else if (input & IN_RIGHT)
            item->goal_anim_state = AS_CLIMBRIGHT;
        else
            item->goal_anim_state = AS_CLIMBSTNC;
        item->pos.y_pos += shift;
    }
    else if (!result)
    {
        item->pos.x_pos = coll->old.x;
        item->pos.z_pos = coll->old.z;
        item->goal_anim_state = AS_CLIMBSTNC;
        item->current_anim_state = AS_CLIMBSTNC;
        if (coll->old_anim_state != AS_CLIMBSTNC)
        {
            item->frame_number = CLIMBSTNC_F;
            item->anim_number = CLIMBSTNC_A;
        }
        else
        {
            item->frame_number = coll->old_frame_number;
            item->anim_number = coll->old_anim_number;
            AnimateLara(item);
        }
    }
    else
    {
        item->goal_anim_state = AS_HANG;
        do
            AnimateItem(item);
        while (item->current_anim_state != AS_HANG);
        item->pos.x_pos = coll->old.x;
        item->pos.z_pos = coll->old.z;
    }
}
