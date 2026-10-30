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
