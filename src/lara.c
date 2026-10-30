#include "game.h"

#define DAMAGE_START 140
#define STEPUP_HEIGHT ((STEP_L * 3) / 2)
#define FALLDOWN_A 34
#define FALLDOWN_F GF(FALLDOWN_A, 0)
#define DAMAGE_LENGTH 14
#define FASTFALL_FRICTION item->speed = (item->speed * 95) / 100
#define SLOPE_DIF 60
#define LARA_DEF_ADD_EDGE (5 * ONE_DEGREE)
#define CAM_A_HANG 0
#define CAM_E_HANG (-60 * ONE_DEGREE)
#define CLIMB_WIDTHL 80
#define CLIMB_WIDTHR 120
#define MAX_HEAD_ROTATION (44 * ONE_DEGREE)
#define HEAD_TURN (2 * ONE_DEGREE)
#define MIN_HEAD_TILT (-42 * ONE_DEGREE)
#define MAX_HEAD_TILT (22 * ONE_DEGREE)
#define BAD_JUMP_CEILING ((STEP_L * 3) / 4)
#define LANDFAR_A 24
#define LANDFAR_F GF(LANDFAR_A, 0)
#define FALLBACK_A 93
#define FALLBACK_F GF(FALLBACK_A, 0)
#define FASTFALL_A 32
#define FASTFALL_F GF(FASTFALL_A, 0)
#define LARA_EXTRA 12
#define PUZZLE_ITEM1 174

int LaraTestClimbPos(ITEM_INFO *item, int front, int right, int origin, int height, int *shift);
void GetLaraCollisionInfo(ITEM_INFO *item, COLL_INFO *coll);
void AnimateLara(ITEM_INFO *item);
int LaraTestHangJump(ITEM_INFO *item, COLL_INFO *coll);
int LaraHitCeiling(ITEM_INFO *item, COLL_INFO *coll);
int LaraDeflectEdge(ITEM_INFO *item, COLL_INFO *coll);
void LaraCollideStop(ITEM_INFO *item, COLL_INFO *coll);
int TestLaraSlide(ITEM_INFO *item, COLL_INFO *coll);
int LaraFallen(ITEM_INFO *item, COLL_INFO *coll);
int StartSyncedAudio(int num);
int Inv_AddItem(int itemNum);
void LaraDeflectEdgeJump(ITEM_INFO *item, COLL_INFO *coll);
void LaraSlideEdgeJump(ITEM_INFO *item, COLL_INFO *coll);
int LaraLandedBad(ITEM_INFO *item, COLL_INFO *coll);
void lara_default_col(ITEM_INFO *item, COLL_INFO *coll);
sint16 LaraFloorFront(ITEM_INFO *item, PHD_ANGLE ang, sint32 dist);

void LookUpDown(void)
{
    camera.type = LOOK_CAMERA;
    if (input & IN_FORWARD)
    {
        input -= IN_FORWARD;
        if (lara.head_x_rot > MIN_HEAD_TILT)
            lara.head_x_rot -= HEAD_TURN;
    }
    else if (input & IN_BACK)
    {
        input -= IN_BACK;
        if (lara.head_x_rot < MAX_HEAD_TILT)
            lara.head_x_rot += HEAD_TURN;
    }

    if (lara.gun_status != LG_HANDSBUSY)
        lara.torso_x_rot = lara.head_x_rot;
}

void LookLeftRight(void)
{
    camera.type = LOOK_CAMERA;
    if (input & IN_LEFT)
    {
        input -= IN_LEFT;
        if (lara.head_y_rot > -MAX_HEAD_ROTATION)
            lara.head_y_rot -= HEAD_TURN;
    }
    else if (input & IN_RIGHT)
    {
        input -= IN_RIGHT;
        if (lara.head_y_rot < MAX_HEAD_ROTATION)
            lara.head_y_rot += HEAD_TURN;
    }

    if (lara.gun_status != LG_HANDSBUSY && lara.skidoo == NO_ITEM)
        lara.torso_y_rot = lara.head_y_rot;
}

void ResetLook(void)
{
    if (camera.type == LOOK_CAMERA)
        return;

    if ((lara.head_x_rot > -HEAD_TURN) && (lara.head_x_rot < HEAD_TURN))
        lara.head_x_rot = 0;
    else
        lara.head_x_rot -= lara.head_x_rot / 8;
    lara.torso_x_rot = lara.head_x_rot;

    if ((lara.head_y_rot > -HEAD_TURN) && (lara.head_y_rot < HEAD_TURN))
        lara.head_y_rot = 0;
    else
        lara.head_y_rot -= lara.head_y_rot / 8;
    lara.torso_y_rot = lara.head_y_rot;
}

void lara_as_walk(ITEM_INFO *item, COLL_INFO *coll)
{
    if (item->hit_points <= 0)
    {
        item->goal_anim_state = AS_STOP;
        return;
    }
    if (input & IN_LEFT)
    {
        lara.turn_rate -= LARA_TURN_RATE;
        if (lara.turn_rate < -LARA_SLOW_TURN)
            lara.turn_rate = -LARA_SLOW_TURN;
    }
    else if (input & IN_RIGHT)
    {
        lara.turn_rate += LARA_TURN_RATE;
        if (lara.turn_rate > LARA_SLOW_TURN)
            lara.turn_rate = LARA_SLOW_TURN;
    }
    if (input & IN_FORWARD)
    {
        if (lara.water_status == LARA_WADE)
            item->goal_anim_state = AS_WADE;
        else
        {
            if (!(input & IN_SLOW))
                item->goal_anim_state = AS_RUN;
            else
                item->goal_anim_state = AS_WALK;
        }
    }
    else
        item->goal_anim_state = AS_STOP;
}

#define LARA_LEAN_UNDO ONE_DEGREE
#define LARA_LEAN_RATE ((ONE_DEGREE / 2) + LARA_LEAN_UNDO)
#define LARA_LEAN_MAX ((10 * ONE_DEGREE) + LARA_LEAN_UNDO)
#define ROLL_A 146
#define ROLL_F GF(ROLL_A, 2)

static int jump_ok = 1;

void lara_as_run(ITEM_INFO *item, COLL_INFO *coll)
{
    if (item->hit_points <= 0)
    {
        item->goal_anim_state = AS_DEATH;
        return;
    }

    if (input & IN_ROLL)
    {
        item->anim_number = ROLL_A;
        item->frame_number = ROLL_F;
        item->current_anim_state = AS_ROLL;
        item->goal_anim_state = AS_STOP;
        return;
    }

    if (input & IN_LEFT)
    {
        lara.turn_rate -= LARA_TURN_RATE;
        if (lara.turn_rate < -LARA_FAST_TURN)
            lara.turn_rate = -LARA_FAST_TURN;
        item->pos.z_rot -= LARA_LEAN_RATE;
        if (item->pos.z_rot < -LARA_LEAN_MAX)
            item->pos.z_rot = -LARA_LEAN_MAX;
    }
    else if (input & IN_RIGHT)
    {
        lara.turn_rate += LARA_TURN_RATE;
        if (lara.turn_rate > LARA_FAST_TURN)
            lara.turn_rate = LARA_FAST_TURN;
        item->pos.z_rot += LARA_LEAN_RATE;
        if (item->pos.z_rot > LARA_LEAN_MAX)
            item->pos.z_rot = LARA_LEAN_MAX;
    }

    if (item->anim_number == 6)
        jump_ok = 0;
    else if (item->anim_number == 0)
    {
        if (item->frame_number == 4)
            jump_ok = 1;
    }
    else
        jump_ok = 1;

    if ((input & IN_JUMP) && jump_ok && !item->gravity_status)
        item->goal_anim_state = AS_FORWARDJUMP;
    else if (input & IN_FORWARD)
    {
        if (lara.water_status == LARA_WADE)
            item->goal_anim_state = AS_WADE;
        else
        {
            if (input & IN_SLOW)
                item->goal_anim_state = AS_WALK;
            else
                item->goal_anim_state = AS_RUN;
        }
    }
    else
        item->goal_anim_state = AS_STOP;
}

void lara_as_forwardjump(ITEM_INFO *item, COLL_INFO *coll)
{
    if (item->goal_anim_state == AS_SWANDIVE || item->goal_anim_state == AS_REACH)
        item->goal_anim_state = AS_FORWARDJUMP;
    if (item->goal_anim_state != AS_DEATH && item->goal_anim_state != AS_STOP && item->goal_anim_state != AS_RUN)
    {
        if ((input & IN_ACTION) && lara.gun_status == LG_ARMLESS)
            item->goal_anim_state = AS_REACH;
        if ((input & IN_ROLL) || (input & IN_BACK))
            item->goal_anim_state = AS_TWIST;
        if ((input & IN_SLOW) && lara.gun_status == LG_ARMLESS)
            item->goal_anim_state = AS_SWANDIVE;
        if (item->fallspeed > LARA_FASTFALL_SPEED)
            item->goal_anim_state = AS_FASTFALL;
    }
    if (input & IN_LEFT)
    {
        lara.turn_rate -= LARA_TURN_RATE;
        if (lara.turn_rate < -LARA_JUMP_TURN)
            lara.turn_rate = -LARA_JUMP_TURN;
    }
    else if (input & IN_RIGHT)
    {
        lara.turn_rate += LARA_TURN_RATE;
        if (lara.turn_rate > LARA_JUMP_TURN)
            lara.turn_rate = LARA_JUMP_TURN;
    }
}

void lara_as_fastback(ITEM_INFO *item, COLL_INFO *coll)
{
    item->goal_anim_state = AS_STOP;
    if (input & IN_LEFT)
    {
        lara.turn_rate -= LARA_TURN_RATE;
        if (lara.turn_rate < -LARA_MED_TURN)
            lara.turn_rate = -LARA_MED_TURN;
    }
    else if (input & IN_RIGHT)
    {
        lara.turn_rate += LARA_TURN_RATE;
        if (lara.turn_rate > LARA_MED_TURN)
            lara.turn_rate = LARA_MED_TURN;
    }
}

void lara_as_turn_r(ITEM_INFO *item, COLL_INFO *coll)
{
    if (item->hit_points <= 0)
    {
        item->goal_anim_state = AS_STOP;
        return;
    }

    lara.turn_rate += LARA_TURN_RATE;

    if (lara.gun_status == LG_READY)
        item->goal_anim_state = AS_FASTTURN;
    else if (lara.turn_rate > LARA_SLOW_TURN)
    {
        if (!(input & IN_SLOW))
            item->goal_anim_state = AS_FASTTURN;
        else
            lara.turn_rate = LARA_SLOW_TURN;
    }

    if (input & IN_FORWARD)
    {
        if (lara.water_status == LARA_WADE)
            item->goal_anim_state = AS_WADE;
        else
        {
            if (input & IN_SLOW)
                item->goal_anim_state = AS_WALK;
            else
                item->goal_anim_state = AS_RUN;
        }
    }
    else if (!(input & IN_RIGHT))
        item->goal_anim_state = AS_STOP;
}

void lara_as_turn_l(ITEM_INFO *item, COLL_INFO *coll)
{
    if (item->hit_points <= 0)
    {
        item->goal_anim_state = AS_STOP;
        return;
    }

    lara.turn_rate -= LARA_TURN_RATE;

    if (lara.gun_status == LG_READY)
        item->goal_anim_state = AS_FASTTURN;
    else if (lara.turn_rate < -LARA_SLOW_TURN)
    {
        if (!(input & IN_SLOW))
            item->goal_anim_state = AS_FASTTURN;
        else
            lara.turn_rate = -LARA_SLOW_TURN;
    }

    if (input & IN_FORWARD)
    {
        if (lara.water_status == LARA_WADE)
            item->goal_anim_state = AS_WADE;
        else
        {
            if (input & IN_SLOW)
                item->goal_anim_state = AS_WALK;
            else
                item->goal_anim_state = AS_RUN;
        }
    }
    else if (!(input & IN_LEFT))
        item->goal_anim_state = AS_STOP;
}

void lara_as_death(ITEM_INFO *item, COLL_INFO *coll)
{
    lara.look = 0;
    coll->enable_spaz = 0;
    coll->enable_baddie_push = 0;
}

void lara_as_fastfall(ITEM_INFO *item, COLL_INFO *coll)
{
    FASTFALL_FRICTION;
    if (item->fallspeed == DAMAGE_START + DAMAGE_LENGTH)
        SoundEffect(30, &item->pos, 0);
}

void lara_as_hang(ITEM_INFO *item, COLL_INFO *coll)
{
    if (input & IN_LOOK)
        LookUpDown();

    coll->enable_spaz = 0;
    coll->enable_baddie_push = 0;
    camera.target_angle = CAM_A_HANG;
    camera.target_elevation = CAM_E_HANG;

    if ((input & IN_LEFT) || (input & IN_STEPL))
        item->goal_anim_state = AS_HANGLEFT;
    else if ((input & IN_RIGHT) || (input & IN_STEPR))
        item->goal_anim_state = AS_HANGRIGHT;
}

void lara_as_reach(ITEM_INFO *item, COLL_INFO *coll)
{
    camera.target_angle = 85 * ONE_DEGREE;
    if (item->fallspeed > LARA_FASTFALL_SPEED)
        item->goal_anim_state = AS_FASTFALL;
}

void lara_as_splat(ITEM_INFO *item, COLL_INFO *coll)
{
    lara.look = 0;
}

void lara_as_compress(ITEM_INFO *item, COLL_INFO *coll)
{
    if (lara.water_status != LARA_WADE)
    {
        if ((input & IN_FORWARD) && LaraFloorFront(item, item->pos.y_rot, 256) >= -STEPUP_HEIGHT)
        {
            item->goal_anim_state = AS_FORWARDJUMP;
            lara.move_angle = item->pos.y_rot;
        }
        else if ((input & IN_LEFT) && LaraFloorFront(item, (sint16)(item->pos.y_rot - 16384), 256) >= -STEPUP_HEIGHT)
        {
            item->goal_anim_state = AS_LEFTJUMP;
            lara.move_angle = item->pos.y_rot - 16384;
        }
        else if ((input & IN_RIGHT) && LaraFloorFront(item, (sint16)(item->pos.y_rot + 16384), 256) >= -STEPUP_HEIGHT)
        {
            item->goal_anim_state = AS_RIGHTJUMP;
            lara.move_angle = item->pos.y_rot + 16384;
        }
        else if ((input & IN_BACK) && LaraFloorFront(item, (sint16)(item->pos.y_rot - 32768), 256) >= -STEPUP_HEIGHT)
        {
            item->goal_anim_state = AS_BACKJUMP;
            lara.move_angle = item->pos.y_rot - 32768;
        }
    }

    if (item->fallspeed > LARA_FASTFALL_SPEED)
        item->goal_anim_state = AS_FASTFALL;
}

void lara_as_back(ITEM_INFO *item, COLL_INFO *coll)
{
    if (item->hit_points <= 0)
    {
        item->goal_anim_state = AS_STOP;
        return;
    }
    if (!(input & IN_BACK) || (!(input & IN_SLOW) && lara.water_status != LARA_WADE))
        item->goal_anim_state = AS_STOP;
    else
        item->goal_anim_state = AS_BACK;
    if (input & IN_LEFT)
    {
        lara.turn_rate -= LARA_TURN_RATE;
        if (lara.turn_rate < -LARA_SLOW_TURN)
            lara.turn_rate = -LARA_SLOW_TURN;
    }
    else if (input & IN_RIGHT)
    {
        lara.turn_rate += LARA_TURN_RATE;
        if (lara.turn_rate > LARA_SLOW_TURN)
            lara.turn_rate = LARA_SLOW_TURN;
    }
}

void lara_as_null(ITEM_INFO *item, COLL_INFO *coll)
{
    coll->enable_spaz = 0;
    coll->enable_baddie_push = 0;
}

void lara_as_fastturn(ITEM_INFO *item, COLL_INFO *coll)
{
    if (item->hit_points <= 0)
    {
        item->goal_anim_state = AS_STOP;
        return;
    }
    if (lara.turn_rate < 0)
    {
        lara.turn_rate = -LARA_FAST_TURN;
        if (!(input & IN_LEFT))
            item->goal_anim_state = AS_STOP;
    }
    else
    {
        lara.turn_rate = LARA_FAST_TURN;
        if (!(input & IN_RIGHT))
            item->goal_anim_state = AS_STOP;
    }
}

void lara_as_stepright(ITEM_INFO *item, COLL_INFO *coll)
{
    lara.look = 0;
    if (item->hit_points <= 0)
    {
        item->goal_anim_state = AS_STOP;
        return;
    }
    if (!(input & IN_STEPR))
        item->goal_anim_state = AS_STOP;
    if (input & IN_LEFT)
    {
        lara.turn_rate -= LARA_TURN_RATE;
        if (lara.turn_rate < -LARA_SLOW_TURN)
            lara.turn_rate = -LARA_SLOW_TURN;
    }
    else if (input & IN_RIGHT)
    {
        lara.turn_rate += LARA_TURN_RATE;
        if (lara.turn_rate > LARA_SLOW_TURN)
            lara.turn_rate = LARA_SLOW_TURN;
    }
}

void lara_as_stepleft(ITEM_INFO *item, COLL_INFO *coll)
{
    lara.look = 0;
    if (item->hit_points <= 0)
    {
        item->goal_anim_state = AS_STOP;
        return;
    }
    if (!(input & IN_STEPL))
        item->goal_anim_state = AS_STOP;
    if (input & IN_LEFT)
    {
        lara.turn_rate -= LARA_TURN_RATE;
        if (lara.turn_rate < -LARA_SLOW_TURN)
            lara.turn_rate = -LARA_SLOW_TURN;
    }
    else if (input & IN_RIGHT)
    {
        lara.turn_rate += LARA_TURN_RATE;
        if (lara.turn_rate > LARA_SLOW_TURN)
            lara.turn_rate = LARA_SLOW_TURN;
    }
}

void lara_as_slide(ITEM_INFO *item, COLL_INFO *coll)
{
    camera.flags = NO_CHUNKY;
    camera.target_elevation = -45 * ONE_DEGREE;
    if ((input & IN_JUMP) && !(input & IN_BACK))
        item->goal_anim_state = AS_FORWARDJUMP;
}

void lara_as_backjump(ITEM_INFO *item, COLL_INFO *coll)
{
    camera.target_angle = ONE_DEGREE * 135;
    if (item->fallspeed > LARA_FASTFALL_SPEED)
        item->goal_anim_state = AS_FASTFALL;
    else if (item->goal_anim_state == AS_RUN)
        item->goal_anim_state = AS_STOP;
    else if ((input & IN_FORWARD || input & IN_ROLL) && item->goal_anim_state != AS_STOP)
        item->goal_anim_state = AS_TWIST;
}

void lara_as_rightjump(ITEM_INFO *item, COLL_INFO *coll)
{
    lara.look = 0;
    if (item->fallspeed > LARA_FASTFALL_SPEED)
        item->goal_anim_state = AS_FASTFALL;
    else if (input & IN_LEFT && item->goal_anim_state != AS_STOP)
        item->goal_anim_state = AS_TWIST;
}

void lara_as_leftjump(ITEM_INFO *item, COLL_INFO *coll)
{
    lara.look = 0;
    if (item->fallspeed > LARA_FASTFALL_SPEED)
        item->goal_anim_state = AS_FASTFALL;
    else if (input & IN_RIGHT && item->goal_anim_state != AS_STOP)
        item->goal_anim_state = AS_TWIST;
}

void lara_as_upjump(ITEM_INFO *item, COLL_INFO *coll)
{
    if (item->fallspeed > LARA_FASTFALL_SPEED)
        item->goal_anim_state = AS_FASTFALL;
}

void lara_as_fallback(ITEM_INFO *item, COLL_INFO *coll)
{
    if (item->fallspeed > LARA_FASTFALL_SPEED)
        item->goal_anim_state = AS_FASTFALL;
    if ((input & IN_ACTION) && lara.gun_status == LG_ARMLESS)
        item->goal_anim_state = AS_REACH;
}

void lara_as_hangleft(ITEM_INFO *item, COLL_INFO *coll)
{
    coll->enable_spaz = 0;
    coll->enable_baddie_push = 0;
    camera.target_angle = CAM_A_HANG;
    camera.target_elevation = CAM_E_HANG;
    if (!(input & IN_LEFT) && !(input & IN_STEPL))
        item->goal_anim_state = AS_HANG;
}

void lara_as_hangright(ITEM_INFO *item, COLL_INFO *coll)
{
    coll->enable_spaz = 0;
    coll->enable_baddie_push = 0;
    camera.target_angle = CAM_A_HANG;
    camera.target_elevation = CAM_E_HANG;
    if (!(input & IN_RIGHT) && !(input & IN_STEPR))
        item->goal_anim_state = AS_HANG;
}

void lara_as_slideback(ITEM_INFO *item, COLL_INFO *coll)
{
    if ((input & IN_JUMP) && !(input & IN_FORWARD))
        item->goal_anim_state = AS_BACKJUMP;
}

void lara_as_pushblock(ITEM_INFO *item, COLL_INFO *coll)
{
    lara.look = 0;
    coll->enable_spaz = 0;
    coll->enable_baddie_push = 0;
    camera.flags = FOLLOW_CENTRE;
    camera.target_angle = 35 * ONE_DEGREE;
    camera.target_elevation = -25 * ONE_DEGREE;
}

void lara_as_ppready(ITEM_INFO *item, COLL_INFO *coll)
{
    coll->enable_spaz = 0;
    coll->enable_baddie_push = 0;
    camera.target_angle = 75 * ONE_DEGREE;
    if (!(input & IN_ACTION))
        item->goal_anim_state = AS_STOP;
}

void lara_as_pickup(ITEM_INFO *item, COLL_INFO *coll)
{
    lara.look = 0;
    coll->enable_spaz = 0;
    coll->enable_baddie_push = 0;
    camera.target_angle = -130 * ONE_DEGREE;
    camera.target_elevation = -15 * ONE_DEGREE;
    camera.target_distance = WALL_L;
}

void lara_as_pickupflare(ITEM_INFO *item, COLL_INFO *coll)
{
    lara.look = 0;
    coll->enable_spaz = 0;
    coll->enable_baddie_push = 0;
    camera.target_angle = +130 * ONE_DEGREE;
    camera.target_elevation = -15 * ONE_DEGREE;
    camera.target_distance = WALL_L;

    if (item->frame_number == anims[item->anim_number].frame_end - 1)
        lara.gun_status = LG_ARMLESS;
}

void lara_as_switchon(ITEM_INFO *item, COLL_INFO *coll)
{
    lara.look = 0;
    coll->enable_spaz = 0;
    coll->enable_baddie_push = 0;
    camera.target_angle = 80 * ONE_DEGREE;
    camera.target_elevation = -25 * ONE_DEGREE;
    camera.target_distance = WALL_L;
    camera.speed = 6;
}

void lara_as_usekey(ITEM_INFO *item, COLL_INFO *coll)
{
    lara.look = 0;
    coll->enable_spaz = 0;
    coll->enable_baddie_push = 0;
    camera.target_angle = -80 * ONE_DEGREE;
    camera.target_elevation = -25 * ONE_DEGREE;
    camera.target_distance = WALL_L;
}

void lara_as_special(ITEM_INFO *item, COLL_INFO *coll)
{
    camera.flags = FOLLOW_CENTRE;
    camera.target_angle = 170 * ONE_DEGREE;
    camera.target_elevation = -25 * ONE_DEGREE;
}

void lara_as_swandive(ITEM_INFO *item, COLL_INFO *coll)
{
    coll->enable_spaz = 0;
    coll->enable_baddie_push = 1;
    if (item->fallspeed > LARA_FASTFALL_SPEED && item->goal_anim_state != AS_DIVE)
        item->goal_anim_state = AS_FASTDIVE;
}

void lara_as_fastdive(ITEM_INFO *item, COLL_INFO *coll)
{
    if ((input & IN_ROLL) && item->goal_anim_state == AS_FASTDIVE)
        item->goal_anim_state = AS_TWIST;
    coll->enable_spaz = 0;
    coll->enable_baddie_push = 1;
    FASTFALL_FRICTION;
}

void lara_as_waterout(ITEM_INFO *item, COLL_INFO *coll)
{
    coll->enable_spaz = 0;
    coll->enable_baddie_push = 0;
    camera.flags = FOLLOW_CENTRE;
}

void lara_as_deathslide(ITEM_INFO *item, COLL_INFO *coll)
{
    sint16 room_number;
    FLOOR_INFO *floor;

    camera.target_angle = ONE_DEGREE * 70;

    room_number = item->room_number;
    floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
    GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);
    coll->trigger = trigger_index;

    if (!(input & IN_ACTION))
    {
        item->goal_anim_state = AS_FORWARDJUMP;
        AnimateLara(item);
        lara_item->gravity_status = 1;
        lara_item->speed = 100;
        lara_item->fallspeed = 40;
        lara.move_angle = item->pos.y_rot;
    }
}

void extra_as_breath(ITEM_INFO *item, COLL_INFO *coll)
{
    item->anim_number = BREATH_A;
    item->frame_number = BREATH_F;
    item->current_anim_state = item->goal_anim_state = AS_STOP;
    lara.gun_status = LG_ARMLESS;
    camera.type = CHASE_CAMERA;
    AlterFOV(GAME_FOV * ONE_DEGREE);
    lara.extra_anim = 0;
}

void extra_as_yetikill(ITEM_INFO *item, COLL_INFO *coll)
{
    camera.target_angle = ONE_DEGREE * 160;
    camera.target_distance = 3 * WALL_L;
    lara.hit_direction = -1;

    if (item->frame_number < anims[item->anim_number].frame_end - 30)
        lara.death_count = 1;
}

void extra_as_sharkkill(ITEM_INFO *item, COLL_INFO *coll)
{
    int wh;

    camera.target_angle = ONE_DEGREE * 160;
    camera.target_distance = 3 * WALL_L;
    lara.hit_direction = -1;

    if (item->frame_number == anims[item->anim_number].frame_end)
    {
        wh = GetWaterHeight(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, item->room_number);
        if (wh != NO_HEIGHT && wh < item->pos.y_pos - 100)
            item->pos.y_pos -= 5;
    }

    if (item->frame_number < anims[item->anim_number].frame_end - 30)
        lara.death_count = 1;
}

void extra_as_airlock(ITEM_INFO *item, COLL_INFO *coll)
{
    camera.target_angle = 80 * ONE_DEGREE;
    camera.target_elevation = -25 * ONE_DEGREE;
}

void extra_as_gongbong(ITEM_INFO *item, COLL_INFO *coll)
{
    camera.target_angle = -25 * ONE_DEGREE;
    camera.target_elevation = -20 * ONE_DEGREE;
    camera.target_distance = WALL_L * 3;
}

void extra_as_dinokill(ITEM_INFO *item, COLL_INFO *coll)
{
    camera.flags = FOLLOW_CENTRE;
    camera.target_angle = 170 * ONE_DEGREE;
    camera.target_elevation = -25 * ONE_DEGREE;
    lara.hit_direction = -1;
    if (item->frame_number < anims[item->anim_number].frame_end - 30)
        lara.death_count = 1;
}


#define PUZZLE_ITEM2 175

void extra_as_pulldagger(ITEM_INFO *item, COLL_INFO *coll)
{
    sint16 room_number;
    int frame;
    FLOOR_INFO *floor;

    frame = item->frame_number - anims[item->anim_number].frame_base;
    if (frame == 1)
        StartSyncedAudio(28);
    else if (frame == 180)
    {
        lara.mesh_ptrs[HAND_R] = meshes[objects[LARA].mesh_index + HAND_R];
        Inv_AddItem(PUZZLE_ITEM2);
    }

    if (item->frame_number == anims[item->anim_number].frame_end)
    {
        item->pos.y_rot += 0x4000;
        room_number = item->room_number;
        floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
        GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);
        TestTriggers(trigger_index, 1);
    }
}

void extra_as_startanim(ITEM_INFO *item, COLL_INFO *coll)
{
    sint16 room_number;
    FLOOR_INFO *floor;

    room_number = item->room_number;
    floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
    GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);
    TestTriggers(trigger_index, 0);
}

void extra_as_starthouse(ITEM_INFO *item, COLL_INFO *coll)
{
    int frame;

    frame = item->frame_number - anims[item->anim_number].frame_base;
    if (frame == 1)
    {
        StartSyncedAudio(63);
        lara.mesh_ptrs[HAND_R] = meshes[objects[LARA_EXTRA].mesh_index + HAND_R];
        lara.mesh_ptrs[HIPS] = meshes[objects[LARA_EXTRA].mesh_index + HIPS];
    }
    else if (frame == 401)
    {
        lara.mesh_ptrs[HAND_R] = meshes[objects[LARA].mesh_index + HAND_R];
        lara.mesh_ptrs[HIPS] = meshes[objects[LARA].mesh_index + HIPS];
        Inv_AddItem(PUZZLE_ITEM1);
    }
    else if (item->frame_number == anims[item->anim_number].frame_end)
    {
        camera.type = CHASE_CAMERA;
        AlterFOV(GAME_FOV * ONE_DEGREE);
    }
}


void extra_as_finalanim(ITEM_INFO *item, COLL_INFO *coll)
{
    int frame;

    item->hit_points = 1000;
    frame = item->frame_number - anims[item->anim_number].frame_base;
    if (frame == 1)
    {
        lara.back_gun = 0;
        lara.mesh_ptrs[HAND_R] = meshes[objects[LARA].mesh_index + HAND_R];
        lara.mesh_ptrs[HEAD] = meshes[objects[LARA].mesh_index + HEAD];
        lara.mesh_ptrs[HIPS] = meshes[objects[LARA_EXTRA].mesh_index + HIPS];
        StartSyncedAudio(27);
    }
    else if (frame == 316)
        lara.mesh_ptrs[HAND_R] = meshes[objects[SHOTGUN].mesh_index + HAND_R];
    else if (item->frame_number == anims[item->anim_number].frame_end - 1)
        level_complete = 1;
}

int LaraFallen(ITEM_INFO *item, COLL_INFO *coll)
{
    if (coll->mid_floor > STEPUP_HEIGHT && lara.water_status != LARA_WADE)
    {
        item->anim_number = FALLDOWN_A;
        item->frame_number = FALLDOWN_F;
        item->current_anim_state = AS_FORWARDJUMP;
        item->goal_anim_state = AS_FORWARDJUMP;
        item->fallspeed = 0;
        item->gravity_status = 1;
        return 1;
    }
    return 0;
}


void LaraCollideStop(ITEM_INFO *item, COLL_INFO *coll)
{
    switch (coll->old_anim_state)
    {
    case AS_STOP:
    case AS_TURN_L:
    case AS_TURN_R:
    case AS_FASTTURN:
        item->current_anim_state = coll->old_anim_state;
        item->anim_number = coll->old_anim_number;
        item->frame_number = coll->old_frame_number;
        if (input & IN_LEFT)
            item->goal_anim_state = AS_TURN_L;
        else if (input & IN_RIGHT)
            item->goal_anim_state = AS_TURN_R;
        else
            item->goal_anim_state = AS_STOP;
        AnimateLara(item);
        break;
    default:
        item->anim_number = STOP_A;
        item->frame_number = STOP_F;
    }
}

void lara_col_stop(ITEM_INFO *item, COLL_INFO *coll)
{
    item->gravity_status = item->fallspeed = 0;
    lara.move_angle = item->pos.y_rot;
    coll->bad_pos = STEPUP_HEIGHT;
    coll->bad_neg = -STEPUP_HEIGHT;
    coll->bad_ceiling = 0;
    coll->slopes_are_pits = 1;
    coll->slopes_are_walls = 1;
    GetLaraCollisionInfo(item, coll);
    if (LaraHitCeiling(item, coll))
        return;
    if (LaraFallen(item, coll))
        return;
    if (TestLaraSlide(item, coll))
        return;
    ShiftItem(item, coll);
    item->pos.y_pos += coll->mid_floor;
}

void lara_col_forwardjump(ITEM_INFO *item, COLL_INFO *coll)
{
    if (item->speed >= 0)
        lara.move_angle = item->pos.y_rot;
    else
        lara.move_angle = item->pos.y_rot - 0x8000;

    coll->bad_pos = NO_BAD_POS;
    coll->bad_neg = -STEPUP_HEIGHT;
    coll->bad_ceiling = BAD_JUMP_CEILING;
    GetLaraCollisionInfo(item, coll);
    LaraDeflectEdgeJump(item, coll);

    if (item->speed < 0)
        lara.move_angle = item->pos.y_rot;

    if (coll->mid_floor <= 0 && item->fallspeed > 0)
    {
        if (LaraLandedBad(item, coll))
            item->goal_anim_state = AS_DEATH;
        else
        {
            if (lara.water_status == LARA_WADE)
            {
                item->goal_anim_state = AS_STOP;
            }
            else
            {
                if (input & IN_FORWARD && !(input & IN_SLOW))
                    item->goal_anim_state = AS_RUN;
                else
                    item->goal_anim_state = AS_STOP;
            }
        }
        item->fallspeed = 0;
        item->pos.y_pos += coll->mid_floor;
        item->gravity_status = 0;
        item->speed = 0;
        AnimateLara(item);
    }
}

void lara_col_fastback(ITEM_INFO *item, COLL_INFO *coll)
{
    item->gravity_status = item->fallspeed = 0;
    lara.move_angle = item->pos.y_rot - 32768;
    coll->bad_pos = NO_BAD_POS;
    coll->bad_neg = -STEPUP_HEIGHT;
    coll->bad_ceiling = 0;
    coll->slopes_are_pits = 1;
    coll->slopes_are_walls = 1;
    GetLaraCollisionInfo(item, coll);

    if (LaraHitCeiling(item, coll))
        return;

    if (coll->mid_floor > 200)
    {
        item->anim_number = FALLBACK_A;
        item->frame_number = FALLBACK_F;
        item->current_anim_state = AS_FALLBACK;
        item->goal_anim_state = AS_FALLBACK;
        item->fallspeed = 0;
        item->gravity_status = 1;
        return;
    }

    if (LaraDeflectEdge(item, coll))
        LaraCollideStop(item, coll);

    item->pos.y_pos += coll->mid_floor;
}

void lara_col_turn_r(ITEM_INFO *item, COLL_INFO *coll)
{
    item->gravity_status = item->fallspeed = 0;
    lara.move_angle = item->pos.y_rot;
    coll->bad_pos = STEPUP_HEIGHT;
    coll->bad_neg = -STEPUP_HEIGHT;
    coll->bad_ceiling = 0;
    coll->slopes_are_pits = 1;
    coll->slopes_are_walls = 1;
    GetLaraCollisionInfo(item, coll);

    if (coll->mid_floor > 100)
    {
        item->anim_number = FALLDOWN_A;
        item->frame_number = FALLDOWN_F;
        item->current_anim_state = AS_FORWARDJUMP;
        item->goal_anim_state = AS_FORWARDJUMP;
        item->fallspeed = 0;
        item->gravity_status = 1;
        return;
    }

    if (TestLaraSlide(item, coll))
        return;

    item->pos.y_pos += coll->mid_floor;
}

void lara_col_turn_l(ITEM_INFO *item, COLL_INFO *coll)
{
    lara_col_turn_r(item, coll);
}

void lara_col_death(ITEM_INFO *item, COLL_INFO *coll)
{
    StopSoundEffect(30);
    lara.move_angle = item->pos.y_rot;
    coll->bad_pos = STEPUP_HEIGHT;
    coll->bad_neg = -STEPUP_HEIGHT;
    coll->bad_ceiling = 0;
    coll->radius = LARA_RAD * 4;
    GetLaraCollisionInfo(item, coll);
    ShiftItem(item, coll);
    item->pos.y_pos += coll->mid_floor;
    item->hit_points = -1;
    lara.air = -1;
}

void lara_col_fastfall(ITEM_INFO *item, COLL_INFO *coll)
{
    item->gravity_status = 1;
    coll->bad_pos = NO_BAD_POS;
    coll->bad_neg = -STEPUP_HEIGHT;
    coll->bad_ceiling = BAD_JUMP_CEILING;
    GetLaraCollisionInfo(item, coll);
    LaraSlideEdgeJump(item, coll);
    if (coll->mid_floor <= 0)
    {
        if (LaraLandedBad(item, coll))
            item->goal_anim_state = AS_DEATH;
        else
        {
            item->goal_anim_state = AS_STOP;
            item->current_anim_state = AS_STOP;
            item->frame_number = LANDFAR_F;
            item->anim_number = LANDFAR_A;
        }
        StopSoundEffect(30);
        item->pos.y_pos += coll->mid_floor;
        item->gravity_status = 0;
        item->fallspeed = 0;
    }
}

void lara_col_reach(ITEM_INFO *item, COLL_INFO *coll)
{
    item->gravity_status = 1;
    lara.move_angle = item->pos.y_rot;
    coll->bad_pos = NO_BAD_POS;
    coll->bad_neg = 0;

    coll->bad_ceiling = BAD_JUMP_CEILING;

    GetLaraCollisionInfo(item, coll);

    if (LaraTestHangJump(item, coll))
        return;

    LaraSlideEdgeJump(item, coll);

    if (item->fallspeed > 0 && coll->mid_floor <= 0)
    {
        if (LaraLandedBad(item, coll))
            item->goal_anim_state = AS_DEATH;
        else
            item->goal_anim_state = AS_STOP;
        item->fallspeed = 0;
        item->pos.y_pos += coll->mid_floor;
        item->gravity_status = 0;
    }
}

void lara_col_splat(ITEM_INFO *item, COLL_INFO *coll)
{
    lara.move_angle = item->pos.y_rot;
    coll->bad_pos = STEPUP_HEIGHT;
    coll->bad_neg = -STEPUP_HEIGHT;
    coll->bad_ceiling = 0;
    coll->slopes_are_pits = 1;
    coll->slopes_are_walls = 1;
    GetLaraCollisionInfo(item, coll);
    ShiftItem(item, coll);

    if (coll->mid_floor > -STEP_L && coll->mid_floor < STEP_L)
        item->pos.y_pos += coll->mid_floor;
}

void lara_col_compress(ITEM_INFO *item, COLL_INFO *coll)
{
    item->gravity_status = item->fallspeed = 0;
    coll->bad_pos = NO_BAD_POS;
    coll->bad_neg = NO_HEIGHT;
    coll->bad_ceiling = 0;
    GetLaraCollisionInfo(item, coll);
    if (coll->mid_ceiling > -100)
    {
        item->goal_anim_state = AS_STOP;
        item->current_anim_state = AS_STOP;
        item->anim_number = STOP_A;
        item->frame_number = STOP_F;
        item->speed = 0;
        item->fallspeed = 0;
        item->gravity_status = 0;
        item->pos.x_pos = coll->old.x;
        item->pos.y_pos = coll->old.y;
        item->pos.z_pos = coll->old.z;
    }

    if (coll->mid_floor > -STEP_L && coll->mid_floor < STEP_L)
        item->pos.y_pos += coll->mid_floor;
}

void lara_col_fastturn(ITEM_INFO *item, COLL_INFO *coll)
{
    lara_col_stop(item, coll);
}

void lara_col_stepright(ITEM_INFO *item, COLL_INFO *coll)
{
    if (item->current_anim_state == AS_STEPRIGHT)
        lara.move_angle = item->pos.y_rot + 16384;
    else
        lara.move_angle = item->pos.y_rot - 16384;

    item->gravity_status = item->fallspeed = 0;
    if (lara.water_status == LARA_WADE)
        coll->bad_pos = NO_BAD_POS;
    else
        coll->bad_pos = STEP_L / 2;
    coll->bad_neg = -STEP_L / 2;
    coll->bad_ceiling = 0;
    coll->slopes_are_pits = 1;
    coll->slopes_are_walls = 1;
    GetLaraCollisionInfo(item, coll);
    if (LaraHitCeiling(item, coll))
        return;
    if (LaraDeflectEdge(item, coll))
        LaraCollideStop(item, coll);
    if (LaraFallen(item, coll))
        return;
    if (TestLaraSlide(item, coll))
        return;
    item->pos.y_pos += coll->mid_floor;
}

void lara_col_stepleft(ITEM_INFO *item, COLL_INFO *coll)
{
    lara_col_stepright(item, coll);
}

void lara_col_slide(ITEM_INFO *item, COLL_INFO *coll)
{
    lara.move_angle = item->pos.y_rot;
    lara_slide_slope(item, coll);
}

void lara_col_backjump(ITEM_INFO *item, COLL_INFO *coll)
{
    lara.move_angle = item->pos.y_rot - 32768;
    lara_col_jumper(item, coll);
}

void lara_col_rightjump(ITEM_INFO *item, COLL_INFO *coll)
{
    lara.move_angle = item->pos.y_rot + 16384;
    lara_col_jumper(item, coll);
}

void lara_col_leftjump(ITEM_INFO *item, COLL_INFO *coll)
{
    lara.move_angle = item->pos.y_rot - 16384;
    lara_col_jumper(item, coll);
}

void lara_col_fallback(ITEM_INFO *item, COLL_INFO *coll)
{
    lara.move_angle = item->pos.y_rot - 32768;
    coll->bad_pos = NO_BAD_POS;
    coll->bad_neg = -STEPUP_HEIGHT;
    coll->bad_ceiling = BAD_JUMP_CEILING;
    GetLaraCollisionInfo(item, coll);
    LaraDeflectEdgeJump(item, coll);
    if (coll->mid_floor <= 0 && item->fallspeed > 0)
    {
        if (LaraLandedBad(item, coll))
            item->goal_anim_state = AS_DEATH;
        else
            item->goal_anim_state = AS_STOP;
        item->fallspeed = 0;
        item->pos.y_pos += coll->mid_floor;
        item->gravity_status = 0;
    }
}

void lara_col_hangleft(ITEM_INFO *item, COLL_INFO *coll)
{
    lara.move_angle = item->pos.y_rot - 16384;
    LaraHangTest(item, coll);
    lara.move_angle = item->pos.y_rot - 16384;
}

void lara_col_hangright(ITEM_INFO *item, COLL_INFO *coll)
{
    lara.move_angle = item->pos.y_rot + 16384;
    LaraHangTest(item, coll);
    lara.move_angle = item->pos.y_rot + 16384;
}

void lara_col_slideback(ITEM_INFO *item, COLL_INFO *coll)
{
    lara.move_angle = item->pos.y_rot - 32768;
    lara_slide_slope(item, coll);
}

void lara_col_roll(ITEM_INFO *item, COLL_INFO *coll)
{
    item->gravity_status = item->fallspeed = 0;
    lara.move_angle = item->pos.y_rot;
    coll->bad_pos = NO_BAD_POS;
    coll->bad_neg = -STEPUP_HEIGHT;
    coll->bad_ceiling = 0;
    coll->slopes_are_walls = 1;
    GetLaraCollisionInfo(item, coll);
    if (LaraHitCeiling(item, coll))
        return;
    if (TestLaraSlide(item, coll))
        return;
    if (LaraFallen(item, coll))
        return;
    if (coll->mid_floor < coll->bad_neg)
    {
        item->pos.x_pos = coll->old.x;
        item->pos.y_pos = coll->old.y;
        item->pos.z_pos = coll->old.z;
        return;
    }
    ShiftItem(item, coll);
    item->pos.y_pos += coll->mid_floor;
}

void lara_col_roll2(ITEM_INFO *item, COLL_INFO *coll)
{
    item->gravity_status = item->fallspeed = 0;
    lara.move_angle = item->pos.y_rot - 32768;
    coll->bad_pos = NO_BAD_POS;
    coll->bad_neg = -STEPUP_HEIGHT;
    coll->bad_ceiling = 0;
    coll->slopes_are_walls = 1;
    GetLaraCollisionInfo(item, coll);
    if (LaraHitCeiling(item, coll))
        return;
    if (TestLaraSlide(item, coll))
        return;
    if (coll->mid_floor > 200)
    {
        item->anim_number = FALLBACK_A;
        item->frame_number = FALLBACK_F;
        item->current_anim_state = AS_FALLBACK;
        item->goal_anim_state = AS_FALLBACK;
        item->fallspeed = 0;
        item->gravity_status = 1;
        return;
    }
    if (coll->mid_floor < coll->bad_neg)
    {
        item->pos.x_pos = coll->old.x;
        item->pos.y_pos = coll->old.y;
        item->pos.z_pos = coll->old.z;
        return;
    }
    ShiftItem(item, coll);
    item->pos.y_pos += coll->mid_floor;
}

void lara_col_special(ITEM_INFO *item, COLL_INFO *coll)
{
    lara_default_col(item, coll);
}

void lara_col_swandive(ITEM_INFO *item, COLL_INFO *coll)
{
    lara.move_angle = item->pos.y_rot;
    coll->bad_pos = NO_BAD_POS;
    coll->bad_neg = -STEPUP_HEIGHT;
    coll->bad_ceiling = BAD_JUMP_CEILING;
    GetLaraCollisionInfo(item, coll);
    LaraDeflectEdgeJump(item, coll);
    if (coll->mid_floor <= 0 && item->fallspeed > 0)
    {
        item->goal_anim_state = AS_STOP;
        item->fallspeed = 0;
        item->pos.y_pos += coll->mid_floor;
        item->gravity_status = 0;
    }
}

void lara_col_fastdive(ITEM_INFO *item, COLL_INFO *coll)
{
    lara.move_angle = item->pos.y_rot;
    coll->bad_pos = NO_BAD_POS;
    coll->bad_neg = -STEPUP_HEIGHT;
    coll->bad_ceiling = BAD_JUMP_CEILING;
    GetLaraCollisionInfo(item, coll);
    LaraDeflectEdgeJump(item, coll);
    if (coll->mid_floor <= 0 && item->fallspeed > 0)
    {
        if (item->fallspeed > 133)
            item->goal_anim_state = AS_DEATH;
        else
            item->goal_anim_state = AS_STOP;
        item->fallspeed = 0;
        item->pos.y_pos += coll->mid_floor;
        item->gravity_status = 0;
    }
}

void lara_default_col(ITEM_INFO *item, COLL_INFO *coll)
{
    lara.move_angle = item->pos.y_rot;
    coll->bad_pos = STEPUP_HEIGHT;
    coll->bad_neg = -STEPUP_HEIGHT;
    coll->bad_ceiling = 0;
    coll->slopes_are_pits = 1;
    coll->slopes_are_walls = 1;
    GetLaraCollisionInfo(item, coll);
}

void lara_col_jumper(ITEM_INFO *item, COLL_INFO *coll)
{
    coll->bad_pos = NO_BAD_POS;
    coll->bad_neg = -STEPUP_HEIGHT;
    coll->bad_ceiling = BAD_JUMP_CEILING;
    GetLaraCollisionInfo(item, coll);
    LaraDeflectEdgeJump(item, coll);
    if (item->fallspeed > 0 && coll->mid_floor <= 0)
    {
        if (LaraLandedBad(item, coll))
            item->goal_anim_state = AS_DEATH;
        else
            item->goal_anim_state = AS_STOP;
        item->fallspeed = 0;
        item->pos.y_pos += coll->mid_floor;
        item->gravity_status = 0;
    }

    if (ABS(coll->mid_ceiling - coll->mid_floor) < LARA_HITE)
    {
        item->goal_anim_state = AS_FASTFALL;
        item->current_anim_state = AS_FASTFALL;
        item->frame_number = FASTFALL_F + 1;
        item->anim_number = FASTFALL_A;
        item->speed /= 4;
        lara.move_angle -= 0x8000;
        if (item->fallspeed <= 0)
            item->fallspeed = 1;
    }
}

void lara_col_kick(ITEM_INFO *item, COLL_INFO *coll)
{
}

void lara_slide_slope(ITEM_INFO *item, COLL_INFO *coll)
{
    coll->bad_pos = NO_BAD_POS;
    coll->bad_neg = -512;
    coll->bad_ceiling = 0;
    GetLaraCollisionInfo(item, coll);
    if (LaraHitCeiling(item, coll))
        return;

    LaraDeflectEdge(item, coll);

    if (coll->mid_floor > 200)
    {
        if (item->current_anim_state == AS_SLIDE)
        {
            item->anim_number = FALLDOWN_A;
            item->frame_number = FALLDOWN_F;
            item->current_anim_state = AS_FORWARDJUMP;
            item->goal_anim_state = AS_FORWARDJUMP;
        }
        else
        {
            item->anim_number = FALLBACK_A;
            item->frame_number = FALLBACK_F;
            item->current_anim_state = AS_FALLBACK;
            item->goal_anim_state = AS_FALLBACK;
        }
        item->fallspeed = 0;
        item->gravity_status = 1;
        return;
    }

    TestLaraSlide(item, coll);
    item->pos.y_pos += coll->mid_floor;
    if (ABS(coll->tilt_x) <= 2 && ABS(coll->tilt_z) <= 2)
        item->goal_anim_state = AS_STOP;
}

int LaraHitCeiling(ITEM_INFO *item, COLL_INFO *coll)
{
    if (coll->coll_type == COLL_TOP || coll->coll_type == COLL_CLAMP)
    {
        item->pos.x_pos = coll->old.x;
        item->pos.y_pos = coll->old.y;
        item->pos.z_pos = coll->old.z;
        item->goal_anim_state = AS_STOP;
        item->current_anim_state = AS_STOP;
        item->frame_number = STOP_F;
        item->anim_number = STOP_A;
        item->speed = 0;
        item->fallspeed = 0;
        item->gravity_status = 0;
        return 1;
    }
    return 0;
}

int LaraDeflectEdge(ITEM_INFO *item, COLL_INFO *coll)
{
    if (coll->coll_type == COLL_FRONT || coll->coll_type == COLL_TOPFRONT)
    {
        ShiftItem(item, coll);

        item->goal_anim_state = AS_STOP;
        item->current_anim_state = AS_STOP;
        item->speed = 0;
        item->gravity_status = 0;
        return 1;
    }
    else if (coll->coll_type == COLL_LEFT)
    {
        ShiftItem(item, coll);
        item->pos.y_rot += LARA_DEF_ADD_EDGE;
    }
    else if (coll->coll_type == COLL_RIGHT)
    {
        ShiftItem(item, coll);
        item->pos.y_rot -= LARA_DEF_ADD_EDGE;
    }
    return 0;
}

#define LAND_A 31
#define LAND_F GF(LAND_A, 0)
#define FASTSPLAT_A 32
#define FASTSPLAT_F GF(FASTSPLAT_A, 1)

void LaraDeflectEdgeJump(ITEM_INFO *item, COLL_INFO *coll)
{
    ShiftItem(item, coll);
    switch (coll->coll_type)
    {
    case COLL_FRONT:
    case COLL_TOPFRONT:
        if (lara.climb_status && item->speed == 2)
            return;

        if (coll->mid_floor > 512)
        {
            item->goal_anim_state = AS_FASTFALL;
            item->current_anim_state = AS_FASTFALL;
            item->frame_number = FASTSPLAT_F;
            item->anim_number = FASTSPLAT_A;
        }
        else if (coll->mid_floor <= 128)
        {
            item->goal_anim_state = AS_LAND;
            item->current_anim_state = AS_LAND;
            item->frame_number = LAND_F;
            item->anim_number = LAND_A;
        }
        item->speed /= 4;
        lara.move_angle += -0x8000;
        if (item->fallspeed <= 0)
            item->fallspeed = 1;
        break;

    case COLL_LEFT:
        item->pos.y_rot += LARA_DEF_ADD_EDGE;
        break;

    case COLL_RIGHT:
        item->pos.y_rot -= LARA_DEF_ADD_EDGE;
        break;

    case COLL_TOP:
        if (item->fallspeed <= 0)
            item->fallspeed = 1;
        break;

    case COLL_CLAMP:
        item->pos.z_pos -= (phd_cos(coll->facing) * 100) >> W2V_SHIFT;
        item->pos.x_pos -= (phd_sin(coll->facing) * 100) >> W2V_SHIFT;
        item->speed = 0;
        coll->mid_floor = 0;
        if (item->fallspeed <= 0)
            item->fallspeed = 16;
        break;
    }
}

void LaraSlideEdgeJump(ITEM_INFO *item, COLL_INFO *coll)
{
    ShiftItem(item, coll);
    switch (coll->coll_type)
    {
    case COLL_LEFT:
        item->pos.y_rot += LARA_DEF_ADD_EDGE;
        break;

    case COLL_RIGHT:
        item->pos.y_rot -= LARA_DEF_ADD_EDGE;
        break;

    case COLL_TOP:
    case COLL_TOPFRONT:
        if (item->fallspeed <= 0)
            item->fallspeed = 1;
        break;

    case COLL_CLAMP:
        item->pos.z_pos -= (phd_cos(coll->facing) * 100) >> W2V_SHIFT;
        item->pos.x_pos -= (phd_sin(coll->facing) * 100) >> W2V_SHIFT;
        item->speed = 0;
        coll->mid_floor = 0;
        if (item->fallspeed <= 0)
            item->fallspeed = 16;
        break;

    default:
        break;
    }
}

int TestWall(ITEM_INFO *item, sint32 front, sint32 right, sint32 down)
{
    int x, y, z;
    int h, c;
    FLOOR_INFO *floor;
    sint16 room_num;
    PHD_ANGLE angle;

    x = item->pos.x_pos;
    y = item->pos.y_pos + down;
    z = item->pos.z_pos;
    room_num = item->room_number;

    angle = (uint16)(item->pos.y_rot + 0x2000) / 0x4000;
    switch (angle)
    {
    case 0:
        x -= right;
        break;

    case 3:
        z += right;
        break;

    case 2:
        x += right;
        break;

    case 1:
        z -= right;
        break;
    }

    floor = GetFloor(x, y, z, &room_num);

    switch (angle)
    {
    case 0:
        z += front;
        break;

    case 3:
        x -= front;
        break;

    case 2:
        z -= front;
        break;

    case 1:
        x += front;
        break;
    }

    floor = GetFloor(x, y, z, &room_num);

    h = GetHeight(floor, x, y, z);
    c = GetCeiling(floor, x, y, z);
    if (h != NO_HEIGHT)
    {
        if ((h - y) > 0 && (c - y) < 0)
            return 0;
        return 2;
    }
    return 1;
}

int LaraTestHangOnClimbWall(ITEM_INFO *item, COLL_INFO *coll)
{
    int shift, result;
    sint16 *bounds;

    if (!lara.climb_status || item->fallspeed < 0)
        return 0;

    switch ((uint16)(item->pos.y_rot + 0x2000) / 0x4000)
    {
    case 0:
    case 2:
        item->pos.z_pos += coll->shift.z;
        break;
    case 1:
    case 3:
        item->pos.x_pos += coll->shift.x;
        break;
    }

    bounds = GetBoundsAccurate(item);

    if (!LaraTestClimbPos(item, coll->radius, coll->radius, bounds[2], bounds[3] - bounds[2], &shift)
        || !LaraTestClimbPos(item, coll->radius, -coll->radius, bounds[2], bounds[3] - bounds[2], &shift))
        return 0;

    result = LaraTestClimbPos(item, coll->radius, 0, bounds[2], bounds[3] - bounds[2], &shift);
    if (!result)
        return 0;
    else if (result == 1)
        return 1;

    item->pos.y_pos += shift;

    return 1;
}

int LaraTestClimbStance(ITEM_INFO *item, COLL_INFO *coll)
{
    int result_r, result_l, shift_r, shift_l;

    result_r = LaraTestClimbPos(item, coll->radius, coll->radius + CLIMB_WIDTHR, -700, STEP_L * 2, &shift_r);
    if (result_r != 1)
        return 0;

    result_l = LaraTestClimbPos(item, coll->radius, -(coll->radius + CLIMB_WIDTHL), -700, STEP_L * 2, &shift_l);
    if (result_l != 1)
        return 0;

    if (shift_r)
    {
        if (shift_l)
        {
            if ((shift_l < 0) ^ (shift_r < 0))
                return 0;
            if (shift_r < 0 && shift_l < shift_r)
                shift_r = shift_l;
            else if (shift_r > 0 && shift_l > shift_r)
                shift_r = shift_l;
        }

        item->pos.y_pos += shift_r;
    }
    else if (shift_l)
        item->pos.y_pos += shift_l;

    return 1;
}

int LaraTestEdgeCatch(ITEM_INFO *item, COLL_INFO *coll, sint32 *edge)
{
    int hdif;
    sint16 *bounds;

    bounds = GetBoundsAccurate(item);
    hdif = coll->front_floor - bounds[2];

    if ((hdif < 0 && hdif + item->fallspeed < 0) || (hdif > 0 && hdif + item->fallspeed > 0))
    {
        hdif = item->pos.y_pos + bounds[2];

        if ((hdif >> (WALL_SHIFT - 2)) != ((hdif + item->fallspeed) >> (WALL_SHIFT - 2)))
        {
            if (item->fallspeed > 0)
                *edge = (hdif + item->fallspeed) & ~(STEP_L - 1);
            else
                *edge = hdif & ~(STEP_L - 1);

            return -1;
        }
        return 0;
    }

    if (ABS(coll->left_floor - coll->right_floor) >= SLOPE_DIF)
        return 0;

    return 1;
}


#define HANG_ANGLE (35 * ONE_DEGREE)
#define STARTHANG_A 96
#define STARTHANG_F GF(STARTHANG_A, 12)

int LaraTestHangJumpUp(ITEM_INFO *item, COLL_INFO *coll)
{
    sint32 edge;
    int edge_catch;
    sint16 *bounds;
    PHD_ANGLE angle;

    if (coll->coll_type != COLL_FRONT || !(input & IN_ACTION) || lara.gun_status != LG_ARMLESS || coll->hit_static
        || coll->mid_ceiling > -STEPUP_HEIGHT)
        return 0;

    if (!(edge_catch = LaraTestEdgeCatch(item, coll, &edge)))
        return 0;

    if (edge_catch < 0 && !LaraTestHangOnClimbWall(item, coll))
        return 0;

    angle = item->pos.y_rot;

    if (angle >= 0 - HANG_ANGLE && angle <= 0 + HANG_ANGLE)
        angle = 0;
    else if (angle >= 16384 - HANG_ANGLE && angle <= 16384 + HANG_ANGLE)
        angle = 16384;
    else if (angle >= 32767 - HANG_ANGLE || angle <= -32767 + HANG_ANGLE)
        angle = -32768;
    else if (angle >= -16384 - HANG_ANGLE && angle <= -16384 + HANG_ANGLE)
        angle = -16384;
    if (angle & 0x3fff)
        return 0;

    item->goal_anim_state = AS_HANG;
    item->current_anim_state = AS_HANG;
    item->anim_number = STARTHANG_A;
    item->frame_number = STARTHANG_F;
    bounds = GetBoundsAccurate(item);
    if (edge_catch > 0)
        item->pos.y_pos += coll->front_floor - bounds[2];
    else
        item->pos.y_pos = edge - bounds[2];

    item->pos.x_pos += coll->shift.x;
    item->pos.z_pos += coll->shift.z;
    item->pos.y_rot = angle;
    item->gravity_status = 0;
    item->speed = 0;
    item->fallspeed = 0;
    lara.gun_status = LG_HANDSBUSY;
    return 1;
}


#define SLIDE_A 70
#define SLIDE_F GF(SLIDE_A, 0)
#define SLIDEBACK_A 104
#define SLIDEBACK_F GF(SLIDEBACK_A, 0)

static PHD_ANGLE old_ang = 1;

int TestLaraSlide(ITEM_INFO *item, COLL_INFO *coll)
{
    PHD_ANGLE adif, ang;

    if (ABS(coll->tilt_x) <= 2 && ABS(coll->tilt_z) <= 2)
        return 0;

    ang = 0;
    if (coll->tilt_x > 2)
        ang = -16384;
    else if (coll->tilt_x < -2)
        ang = 16384;
    if (coll->tilt_z > 2 && coll->tilt_z > ABS(coll->tilt_x))
        ang = -32768;
    else if (coll->tilt_z < -2 && -coll->tilt_z > ABS(coll->tilt_x))
        ang = 0;

    adif = ang - item->pos.y_rot;
    ShiftItem(item, coll);
    if (adif >= -16384 && adif <= 16384)
    {
        if (item->current_anim_state != AS_SLIDE || old_ang != ang)
        {
            item->anim_number = SLIDE_A;
            item->frame_number = SLIDE_F;
            item->goal_anim_state = AS_SLIDE;
            item->current_anim_state = AS_SLIDE;
            item->pos.y_rot = ang;
            lara.move_angle = ang;
            old_ang = ang;
        }
    }
    else
    {
        if (item->current_anim_state != AS_SLIDEBACK || old_ang != ang)
        {
            item->anim_number = SLIDEBACK_A;
            item->frame_number = SLIDEBACK_F;
            item->goal_anim_state = AS_SLIDEBACK;
            item->current_anim_state = AS_SLIDEBACK;
            item->pos.y_rot = ang - 32768;
            lara.move_angle = ang;
            old_ang = ang;
        }
    }
    return 1;
}

sint16 LaraFloorFront(ITEM_INFO *item, PHD_ANGLE ang, sint32 dist)
{
    sint16 room;
    FLOOR_INFO *floor;
    sint32 x, y, z, height;

    x = item->pos.x_pos + ((phd_sin(ang) * dist) >> W2V_SHIFT);
    y = item->pos.y_pos - LARA_HITE;
    z = item->pos.z_pos + ((phd_cos(ang) * dist) >> W2V_SHIFT);
    room = item->room_number;

    floor = GetFloor(x, y, z, &room);
    height = GetHeight(floor, x, y, z);

    if (height != NO_HEIGHT)
        height -= item->pos.y_pos;

    return (sint16)height;
}


int LaraLandedBad(ITEM_INFO *item, COLL_INFO *coll)
{
    int landspeed, oy;
    FLOOR_INFO *floor;
    sint16 room_number;

    room_number = item->room_number;
    floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
    oy = item->pos.y_pos;
    item->floor = item->pos.y_pos = GetHeight(floor, item->pos.x_pos, item->pos.y_pos - LARA_HITE, item->pos.z_pos);
    TestTriggers(trigger_index, 0);
    item->pos.y_pos = oy;

    landspeed = item->fallspeed - DAMAGE_START;
    if (landspeed <= 0)
        return 0;
    else if (landspeed > DAMAGE_LENGTH)
        item->hit_points = -1;
    else
        item->hit_points -= (landspeed * landspeed * 1000) / (DAMAGE_LENGTH * DAMAGE_LENGTH);
    if (item->hit_points <= 0)
        return 1;
    return 0;
}

