#include "game.h"

#define UW_MAXSPEED (50 * 4)
#define WATER_FRICTION 6

void LaraSwimCollision(ITEM_INFO *item, COLL_INFO *coll);
void SwimTurn(ITEM_INFO *item);

void lara_as_swim(ITEM_INFO *item, COLL_INFO *coll)
{
    if (item->hit_points <= 0)
    {
        item->goal_anim_state = AS_UWDEATH;
        return;
    }
    if (input & IN_ROLL)
    {
        item->current_anim_state = AS_WATERROLL;
        item->anim_number = WATERROLL_A;
        item->frame_number = WATERROLL_F;
        return;
    }
    SwimTurn(item);
    item->fallspeed += 2 * 4;
    if (item->fallspeed > UW_MAXSPEED)
        item->fallspeed = UW_MAXSPEED;
    if (!(input & IN_JUMP))
        item->goal_anim_state = AS_GLIDE;
}

void lara_as_glide(ITEM_INFO *item, COLL_INFO *coll)
{
    if (item->hit_points <= 0)
    {
        item->goal_anim_state = AS_UWDEATH;
        return;
    }
    if (input & IN_ROLL)
    {
        item->current_anim_state = AS_WATERROLL;
        item->anim_number = WATERROLL_A;
        item->frame_number = WATERROLL_F;
        return;
    }
    SwimTurn(item);
    if (input & IN_JUMP)
        item->goal_anim_state = AS_SWIM;
    item->fallspeed -= WATER_FRICTION;
    if (item->fallspeed < 0)
        item->fallspeed = 0;
    if (item->fallspeed <= (UW_MAXSPEED * 2) / 3)
        item->goal_anim_state = AS_TREAD;
}

void lara_as_tread(ITEM_INFO *item, COLL_INFO *coll)
{
    if (item->hit_points <= 0)
    {
        item->goal_anim_state = AS_UWDEATH;
        return;
    }
    if (input & IN_ROLL)
    {
        item->current_anim_state = AS_WATERROLL;
        item->anim_number = WATERROLL_A;
        item->frame_number = WATERROLL_F;
        return;
    }
    if (input & IN_LOOK)
        LookUpDown();
    SwimTurn(item);
    if (input & IN_JUMP)
        item->goal_anim_state = AS_SWIM;
    item->fallspeed -= WATER_FRICTION;
    if (item->fallspeed < 0)
        item->fallspeed = 0;
    if (lara.gun_status == LG_HANDSBUSY)
        lara.gun_status = LG_ARMLESS;
}

void lara_as_dive(ITEM_INFO *item, COLL_INFO *coll)
{
    if (input & IN_FORWARD)
        item->pos.x_rot -= ONE_DEGREE;
}

void lara_as_waterroll(ITEM_INFO *item, COLL_INFO *coll)
{
    item->fallspeed = 0;
}

void lara_col_swim(ITEM_INFO *item, COLL_INFO *coll)
{
    LaraSwimCollision(item, coll);
}
