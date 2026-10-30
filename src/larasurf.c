#include "game.h"

#define SURF_MAXSPEED (15 * 4)
#define STEPUP_HEIGHT ((STEP_L * 3) / 2)
#define SURFDIVE_A 119
#define DIVE_COUNT 10
#define SURFDIVE_F GF(SURFDIVE_A, 0)

void LaraSurfaceCollision(ITEM_INFO *item, COLL_INFO *coll);
int LaraTestWaterClimbOut(ITEM_INFO *item, COLL_INFO *coll);

void lara_as_surfswim(ITEM_INFO *item, COLL_INFO *coll)
{
    if (item->hit_points <= 0)
    {
        item->goal_anim_state = AS_UWDEATH;
        return;
    }
    lara.dive_count = 0;
    if (input & IN_LEFT)
        item->pos.y_rot -= LARA_SLOW_TURN;
    else if (input & IN_RIGHT)
        item->pos.y_rot += LARA_SLOW_TURN;
    if (!(input & IN_FORWARD))
        item->goal_anim_state = AS_SURFTREAD;
    if (input & IN_JUMP)
        item->goal_anim_state = AS_SURFTREAD;

    item->fallspeed += 2 * 4;
    if (item->fallspeed > SURF_MAXSPEED)
        item->fallspeed = SURF_MAXSPEED;
}

void lara_as_surfback(ITEM_INFO *item, COLL_INFO *coll)
{
    if (item->hit_points <= 0)
    {
        item->goal_anim_state = AS_UWDEATH;
        return;
    }
    lara.dive_count = 0;
    if (input & IN_LEFT)
        item->pos.y_rot -= LARA_SLOW_TURN / 2;
    else if (input & IN_RIGHT)
        item->pos.y_rot += LARA_SLOW_TURN / 2;
    if (!(input & IN_BACK))
        item->goal_anim_state = AS_SURFTREAD;

    item->fallspeed += 2 * 4;
    if (item->fallspeed > SURF_MAXSPEED)
        item->fallspeed = SURF_MAXSPEED;
}

void lara_as_surfleft(ITEM_INFO *item, COLL_INFO *coll)
{
    if (item->hit_points <= 0)
    {
        item->goal_anim_state = AS_UWDEATH;
        return;
    }
    lara.dive_count = 0;
    if (input & IN_LEFT)
        item->pos.y_rot -= LARA_SLOW_TURN / 2;
    else if (input & IN_RIGHT)
        item->pos.y_rot += LARA_SLOW_TURN / 2;
    if (!(input & IN_STEPL))
        item->goal_anim_state = AS_SURFTREAD;

    item->fallspeed += 2 * 4;
    if (item->fallspeed > SURF_MAXSPEED)
        item->fallspeed = SURF_MAXSPEED;
}

void lara_as_surfright(ITEM_INFO *item, COLL_INFO *coll)
{
    if (item->hit_points <= 0)
    {
        item->goal_anim_state = AS_UWDEATH;
        return;
    }
    lara.dive_count = 0;
    if (input & IN_LEFT)
        item->pos.y_rot -= LARA_SLOW_TURN / 2;
    else if (input & IN_RIGHT)
        item->pos.y_rot += LARA_SLOW_TURN / 2;
    if (!(input & IN_STEPR))
        item->goal_anim_state = AS_SURFTREAD;

    item->fallspeed += 2 * 4;
    if (item->fallspeed > SURF_MAXSPEED)
        item->fallspeed = SURF_MAXSPEED;
}

void lara_as_surftread(ITEM_INFO *item, COLL_INFO *coll)
{
    item->fallspeed -= 4;
    if (item->fallspeed < 0)
        item->fallspeed = 0;

    if (item->hit_points <= 0)
    {
        item->goal_anim_state = AS_UWDEATH;
        return;
    }

    if (input & IN_LOOK)
    {
        LookUpDown();
        return;
    }

    if (input & IN_LEFT)
        item->pos.y_rot -= LARA_SLOW_TURN;
    else if (input & IN_RIGHT)
        item->pos.y_rot += LARA_SLOW_TURN;
    if (input & IN_FORWARD)
        item->goal_anim_state = AS_SURFSWIM;
    else if (input & IN_BACK)
        item->goal_anim_state = AS_SURFBACK;
    if (input & IN_STEPL)
        item->goal_anim_state = AS_SURFLEFT;
    else if (input & IN_STEPR)
        item->goal_anim_state = AS_SURFRIGHT;

    if (input & IN_JUMP)
    {
        lara.dive_count++;
        if (lara.dive_count == DIVE_COUNT)
            item->goal_anim_state = AS_SWIM;
    }
    else
        lara.dive_count = 0;
}

void lara_col_surfswim(ITEM_INFO *item, COLL_INFO *coll)
{
    coll->bad_neg = -STEPUP_HEIGHT;
    lara.move_angle = item->pos.y_rot;
    LaraSurfaceCollision(item, coll);
    LaraTestWaterClimbOut(item, coll);
}

void lara_col_surfback(ITEM_INFO *item, COLL_INFO *coll)
{
    lara.move_angle = item->pos.y_rot - 32768;
    LaraSurfaceCollision(item, coll);
}

void lara_col_surfleft(ITEM_INFO *item, COLL_INFO *coll)
{
    lara.move_angle = item->pos.y_rot - 16384;
    LaraSurfaceCollision(item, coll);
}

void lara_col_surfright(ITEM_INFO *item, COLL_INFO *coll)
{
    lara.move_angle = item->pos.y_rot + 16384;
    LaraSurfaceCollision(item, coll);
}
