#include "game.h"
#include <windows.h>

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


#define CLIMBING_A 161
#define CLIMBDOWN_A 168
#define CLIMBUP_LNK1 29
#define CLIMBUP_LNK2 58
#define CLIMBDN_LNK1 29
#define CLIMBDN_LNK2 58

int LaraTestClimbUpPos(ITEM_INFO *item, int front, int right, int *shift, int *ledge);
#define CLIMBSTNC_A 164
#define CLIMBSTNC_F GF(CLIMBSTNC_A, 0)

void AnimateLara(ITEM_INFO *item);

void lara_col_climbstnc(ITEM_INFO *item, COLL_INFO *coll)
{
    int shift_r, ledge_r, shift_l, result_l, result_r, ledge_l;

    if (LaraCheckForLetGo(item, coll))
        return;
    if (item->anim_number != CLIMBSTNC_A)
        return;

    if (input & IN_FORWARD)
    {
        if (item->goal_anim_state == AS_NULL)
            return;

        item->goal_anim_state = AS_CLIMBSTNC;
        result_r = LaraTestClimbUpPos(item, coll->radius, coll->radius + CLIMB_WIDTHR, &shift_r, &ledge_r);
        result_l = LaraTestClimbUpPos(item, coll->radius, -(coll->radius + CLIMB_WIDTHL), &shift_l, &ledge_l);
        if (!result_r || !result_l)
            return;

        if (result_r < 0 || result_l < 0)
        {
            if (ABS(ledge_l - ledge_r) > 120)
                return;
            item->pos.y_pos += (ledge_l + ledge_r) / 2 - STEP_L;
            item->goal_anim_state = AS_NULL;
            return;
        }

        if (shift_r)
        {
            if (shift_l)
            {
                if ((shift_r < 0) ^ (shift_l < 0))
                    return;
                if (shift_r < 0 && shift_r < shift_l)
                    shift_l = shift_r;
                else if (shift_r > 0 && shift_r > shift_l)
                    shift_l = shift_r;
            }
            else
                shift_l = shift_r;
        }

        item->goal_anim_state = AS_CLIMBING;
        item->pos.y_pos += shift_l;
    }
    else if (input & IN_BACK)
    {
        if (item->goal_anim_state == AS_HANG)
            return;

        item->goal_anim_state = AS_CLIMBSTNC;
        item->pos.y_pos += STEP_L;
        result_r = LaraTestClimbPos(item, coll->radius, coll->radius + CLIMB_WIDTHR, -CLIMB_HITE, CLIMB_HITE, &shift_r);
        result_l = LaraTestClimbPos(item, coll->radius, -(coll->radius + CLIMB_WIDTHL), -CLIMB_HITE, CLIMB_HITE, &shift_l);
        item->pos.y_pos -= STEP_L;
        if (!result_r || !result_l)
            return;

        if (shift_r && shift_l)
        {
            if ((shift_r < 0) ^ (shift_l < 0))
                return;
            if (shift_r < 0 && shift_r < shift_l)
                shift_l = shift_r;
            else if (shift_r > 0 && shift_r > shift_l)
                shift_l = shift_r;
        }

        if (result_r == 1 && result_l == 1)
        {
            item->goal_anim_state = AS_CLIMBDOWN;
            item->pos.y_pos += shift_l;
        }
        else
            item->goal_anim_state = AS_HANG;
    }
}

void lara_col_climbing(ITEM_INFO *item, COLL_INFO *coll)
{
    int right, left, right_shift, left_shift, right_ledge, left_ledge;
    int step, frame;

    if (LaraCheckForLetGo(item, coll) || item->anim_number != CLIMBING_A)
        return;
    frame = item->frame_number - anims[CLIMBING_A].frame_base;
    if (!frame)
        step = 0;
    else if (frame == 28 || frame == 29)
        step = -STEP_L;
    else if (frame == 57)
        step = -STEP_L * 2;
    else
        return;
    item->pos.y_pos += step - STEP_L;
    right = LaraTestClimbUpPos(item, coll->radius, coll->radius + 120, &right_shift, &right_ledge);
    left = LaraTestClimbUpPos(item, coll->radius, -80 - coll->radius, &left_shift, &left_ledge);
    item->pos.y_pos += STEP_L;
    if (right && left && (input & IN_FORWARD))
    {
        if (right < 0 || left < 0)
        {
            item->goal_anim_state = AS_CLIMBSTNC;
            AnimateLara(item);
            if (ABS(left_ledge - right_ledge) <= 120)
            {
                item->goal_anim_state = AS_NULL;
                item->pos.y_pos += (right_ledge + left_ledge) / 2 - STEP_L;
            }
        }
        else
        {
            item->goal_anim_state = AS_CLIMBING;
            item->pos.y_pos -= step;
        }
    }
    else
    {
        item->goal_anim_state = AS_CLIMBSTNC;
        if (step)
            AnimateLara(item);
    }
}

void lara_col_climbdown(ITEM_INFO *item, COLL_INFO *coll)
{
    int result_r, result_l, shift_r, shift_l;
    int yshift, frame;

    if (LaraCheckForLetGo(item, coll))
        return;

    if (item->anim_number == CLIMBDOWN_A)
    {
        frame = item->frame_number - anims[CLIMBDOWN_A].frame_base;
        if (frame == 0)
            yshift = 0;
        else if (frame == CLIMBDN_LNK1 - 1 || frame == CLIMBDN_LNK1)
            yshift = STEP_L;
        else if (frame == CLIMBDN_LNK2 - 1)
            yshift = STEP_L * 2;
        else
            return;
    }
    else
        return;

    item->pos.y_pos += yshift + STEP_L;
    result_r = LaraTestClimbPos(item, coll->radius, coll->radius + CLIMB_WIDTHR, -CLIMB_HITE, CLIMB_HITE, &shift_r);
    result_l = LaraTestClimbPos(item, coll->radius, -(coll->radius + CLIMB_WIDTHL), -CLIMB_HITE, CLIMB_HITE, &shift_l);
    item->pos.y_pos -= STEP_L;

    if (!result_r || !result_l || !(input & IN_BACK))
    {
        item->goal_anim_state = AS_CLIMBSTNC;
        if (yshift)
            AnimateLara(item);
        return;
    }

    if (shift_r && shift_l)
    {
        if ((shift_r < 0) ^ (shift_l < 0))
        {
            item->goal_anim_state = AS_CLIMBSTNC;
            AnimateLara(item);
            return;
        }
        if (shift_r < 0 && shift_r < shift_l)
            shift_l = shift_r;
        else if (shift_r > 0 && shift_r > shift_l)
            shift_l = shift_r;
    }

    if (result_r == -1 || result_l == -1)
    {
        item->current_anim_state = AS_CLIMBSTNC;
        item->frame_number = CLIMBSTNC_F;
        item->anim_number = CLIMBSTNC_A;
        item->goal_anim_state = AS_HANG;
        AnimateLara(item);
        return;
    }

    item->goal_anim_state = AS_CLIMBDOWN;
    item->pos.y_pos -= yshift;
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

#define CLIMB_SHIFT 70
#define CLIMB_HANG 900

sint32 GetCeiling(FLOOR_INFO *floor, sint32 x, sint32 y, sint32 z);

int LaraTestClimb(int x, int y, int z, int xfront, int zfront, int item_height, sint16 item_room, int *shift)
{
    sint16 room_number;
    int height, ceiling;
    int hang = 1;
    FLOOR_INFO *floor;

    *shift = 0;

    if (!lara.climb_status)
        return 0;

    room_number = item_room;
    floor = GetFloor(x, y - STEP_L / 2, z, &room_number);
    height = GetHeight(floor, x, y, z);
    if (height == NO_HEIGHT)
        return 0;

    height -= (y + item_height + (STEP_L / 2));
    if (height < -CLIMB_SHIFT)
        return 0;
    else if (height < 0)
        *shift = height;

    ceiling = GetCeiling(floor, x, y, z) - y;
    if (ceiling > CLIMB_SHIFT)
        return 0;
    else if (ceiling > 0)
    {
        if (*shift)
            return 0;
        *shift = ceiling;
    }

    if (item_height + height < CLIMB_HANG)
        hang = 0;

    floor = GetFloor(x + xfront, y, z + zfront, &room_number);
    height = GetHeight(floor, x + xfront, y, z + zfront);
    if (height != NO_HEIGHT)
        height -= y;

    if (height > CLIMB_SHIFT)
    {
        ceiling = GetCeiling(floor, x + xfront, y, z + zfront) - y;
        if (ceiling >= CLIMB_HITE)
            return 1;
        else if (ceiling > CLIMB_HITE - CLIMB_SHIFT)
        {
            if (*shift > 0)
            {
                if (hang)
                    return -1;
                else
                    return 0;
            }
            *shift = ceiling - CLIMB_HITE;
            return 1;
        }
        else if (ceiling > 0)
        {
            if (hang)
                return -1;
            else
                return 0;
        }
        else if (ceiling > -CLIMB_SHIFT)
        {
            if (hang && *shift <= 0)
            {
                if (*shift > ceiling)
                    *shift = ceiling;
                return -1;
            }
            else
                return 0;
        }
        else
            return 0;
    }
    else if (height > 0)
    {
        if (*shift < 0)
            return 0;
        if (height > *shift)
            *shift = height;
    }

    room_number = item_room;
    floor = GetFloor(x, y + item_height, z, &room_number);
    floor = GetFloor(x + xfront, y + item_height, z + zfront, &room_number);
    ceiling = GetCeiling(floor, x + xfront, y + item_height, z + zfront);
    if (ceiling == NO_HEIGHT)
        return 1;

    ceiling -= y;
    if (ceiling <= height)
        return 1;

    if (ceiling >= CLIMB_HITE)
        return 1;
    else if (ceiling > CLIMB_HITE - CLIMB_SHIFT)
    {
        if (*shift > 0)
        {
            if (hang)
                return -1;
            else
                return 0;
        }
        *shift = ceiling - CLIMB_HITE;
        return 1;
    }
    else
    {
        if (hang)
            return -1;
        else
            return 0;
    }
}

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

int LaraTestClimbUpPos(ITEM_INFO *item, int front, int right, int *shift, int *ledge)
{
    int angle, x, y, z;
    int xfront = 0, zfront = 0;
    int height, ceiling;
    FLOOR_INFO *floor;
    sint16 room_number;

    y = item->pos.y_pos - CLIMB_HITE - STEP_L;
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

    *shift = 0;
    room_number = item->room_number;
    floor = GetFloor(x, y, z, &room_number);
    ceiling = GetCeiling(floor, x, y, z) - (y - STEP_L);
    if (ceiling > CLIMB_SHIFT)
        return 0;
    else if (ceiling > 0)
        *shift = ceiling;

    floor = GetFloor(x + xfront, y, z + zfront, &room_number);
    height = GetHeight(floor, x + xfront, y, z + zfront);
    if (height == NO_HEIGHT)
    {
        *ledge = NO_HEIGHT;
        return 1;
    }

    height -= y;
    *ledge = height;
    if (height > STEP_L / 2)
    {
        ceiling = GetCeiling(floor, x + xfront, y, z + zfront) - y;
        if (ceiling >= CLIMB_HITE)
            return 1;
        else if (height - ceiling > LARA_HITE)
        {
            *shift = height;
            return -1;
        }
        else
            return 0;
    }
    else if (height > 0)
    {
        if (height > *shift)
            *shift = height;
    }

    room_number = item->room_number;
    floor = GetFloor(x, y + CLIMB_HITE, z, &room_number);
    floor = GetFloor(x + xfront, y + CLIMB_HITE, z + zfront, &room_number);
    ceiling = GetCeiling(floor, x + xfront, y + CLIMB_HITE, z + zfront) - y;
    if (ceiling <= height)
        return 1;
    if (ceiling >= CLIMB_HITE)
        return 1;
    else
        return 0;
}
