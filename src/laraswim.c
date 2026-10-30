#include "game.h"

#define UW_MAXSPEED (50 * 4)
#define WATER_FRICTION 6

void LaraSwimCollision(ITEM_INFO *item, COLL_INFO *coll);
void SwimTurn(ITEM_INFO *item);

#define LARA_LEAN_UNDO ONE_DEGREE
#define LARA_LEAN_RATE ((ONE_DEGREE / 2) + LARA_LEAN_UNDO)
#define LARA_LEAN_MAX ((10 * ONE_DEGREE) + LARA_LEAN_UNDO)
#define LARA_LEAN_MAX_UW (LARA_LEAN_MAX * 2)
#define UW_HITE 400
#define UW_RADIUS 300

extern void (*lara_control_routines[])(ITEM_INFO *item, COLL_INFO *coll);
extern void (*extra_control_routines[])(ITEM_INFO *item, COLL_INFO *coll);
extern void (*lara_collision_routines[])(ITEM_INFO *item, COLL_INFO *coll);
void LookLeftRight(void);
void ResetLook(void);
void LaraWaterCurrent(COLL_INFO *coll);
void AnimateLara(ITEM_INFO *item);
void LaraBaddieCollision(ITEM_INFO *laraitem, COLL_INFO *coll);
void UpdateLaraRoom(ITEM_INFO *item, int height);
void LaraGun(void);

void LaraUnderWater(ITEM_INFO *item, COLL_INFO *coll)
{
    coll->bad_pos = NO_BAD_POS;
    coll->bad_neg = -UW_HITE;
    coll->bad_ceiling = UW_HITE;
    coll->old.x = item->pos.x_pos;
    coll->old.y = item->pos.y_pos;
    coll->old.z = item->pos.z_pos;
    coll->radius = UW_RADIUS;
    coll->trigger = NULL;
    coll->slopes_are_walls = 0;
    coll->slopes_are_pits = 0;
    coll->lava_is_pit = 0;
    coll->enable_spaz = 0;
    coll->enable_baddie_push = 1;

    if ((input & IN_LOOK) && lara.look)
        LookLeftRight();
    else
        ResetLook();
    lara.look = 1;

    if (lara.extra_anim)
        (*extra_control_routines[item->current_anim_state])(item, coll);
    else
        (*lara_control_routines[item->current_anim_state])(item, coll);

    if (item->pos.z_rot >= -(2 * LARA_LEAN_UNDO) && item->pos.z_rot <= 2 * LARA_LEAN_UNDO)
        item->pos.z_rot = 0;
    else if (item->pos.z_rot < 0)
        item->pos.z_rot += 2 * LARA_LEAN_UNDO;
    else
        item->pos.z_rot -= 2 * LARA_LEAN_UNDO;

    if (item->pos.x_rot < -85 * ONE_DEGREE)
        item->pos.x_rot = -85 * ONE_DEGREE;
    else if (item->pos.x_rot > 85 * ONE_DEGREE)
        item->pos.x_rot = 85 * ONE_DEGREE;
    if (item->pos.z_rot < -LARA_LEAN_MAX_UW)
        item->pos.z_rot = -LARA_LEAN_MAX_UW;
    else if (item->pos.z_rot > LARA_LEAN_MAX_UW)
        item->pos.z_rot = LARA_LEAN_MAX_UW;

    if (lara.turn_rate >= -LARA_TURN_UNDO && lara.turn_rate <= LARA_TURN_UNDO)
        lara.turn_rate = 0;
    else if (lara.turn_rate < -LARA_TURN_UNDO)
        lara.turn_rate += LARA_TURN_UNDO;
    else
        lara.turn_rate -= LARA_TURN_UNDO;
    item->pos.y_rot += lara.turn_rate;

    if (lara.current_active && lara.water_status != LARA_CHEAT)
        LaraWaterCurrent(coll);

    AnimateLara(item);
    item->pos.y_pos -= (phd_sin(item->pos.x_rot) * item->fallspeed) >> (W2V_SHIFT + 2);
    item->pos.x_pos += (((phd_sin(item->pos.y_rot) * item->fallspeed) >> (W2V_SHIFT + 2)) * phd_cos(item->pos.x_rot)) >> W2V_SHIFT;
    item->pos.z_pos += (((phd_cos(item->pos.y_rot) * item->fallspeed) >> (W2V_SHIFT + 2)) * phd_cos(item->pos.x_rot)) >> W2V_SHIFT;

    if (lara.water_status != LARA_CHEAT && !lara.extra_anim)
        LaraBaddieCollision(item, coll);

    if (!lara.extra_anim)
        (*lara_collision_routines[item->current_anim_state])(item, coll);

    UpdateLaraRoom(item, 0);

    LaraGun();

    TestTriggers(coll->trigger, 0);
}

void SwimTurn(ITEM_INFO *item)
{
    if (input & IN_FORWARD)
        item->pos.x_rot -= 2 * ONE_DEGREE;
    else if (input & IN_BACK)
        item->pos.x_rot += 2 * ONE_DEGREE;

    if (input & IN_LEFT)
    {
        lara.turn_rate -= LARA_TURN_RATE;
        if (lara.turn_rate < -LARA_MED_TURN)
            lara.turn_rate = -LARA_MED_TURN;
        item->pos.z_rot -= LARA_LEAN_RATE * 2;
    }
    else if (input & IN_RIGHT)
    {
        lara.turn_rate += LARA_TURN_RATE;
        if (lara.turn_rate > LARA_MED_TURN)
            lara.turn_rate = LARA_MED_TURN;
        item->pos.z_rot += LARA_LEAN_RATE * 2;
    }
}

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

void lara_as_uwdeath(ITEM_INFO *item, COLL_INFO *coll)
{
    lara.look = 0;

    item->fallspeed -= 8;
    if (item->fallspeed <= 0)
        item->fallspeed = 0;

    if (item->pos.x_rot >= -2 * ONE_DEGREE && item->pos.x_rot <= 2 * ONE_DEGREE)
        item->pos.x_rot = 0;
    else if (item->pos.x_rot < 0)
        item->pos.x_rot += 2 * ONE_DEGREE;
    else
        item->pos.x_rot -= 2 * ONE_DEGREE;
}

void lara_as_waterroll(ITEM_INFO *item, COLL_INFO *coll)
{
    item->fallspeed = 0;
}

void lara_col_swim(ITEM_INFO *item, COLL_INFO *coll)
{
    LaraSwimCollision(item, coll);
}

void lara_col_uwdeath(ITEM_INFO *item, COLL_INFO *coll)
{
    int wh;

    item->hit_points = -1;
    lara.air = -1;
    lara.gun_status = LG_HANDSBUSY;
    wh = GetWaterHeight(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, item->room_number);
    if (wh != NO_HEIGHT && wh < item->pos.y_pos - 100)
        item->pos.y_pos -= 5;
    LaraSwimCollision(item, coll);
}


#define NO_ROOM 0xff
#define UNDERWATER 1
#define SWIM2QSTND_A 192
#define SWIM2QSTND_F GF(SWIM2QSTND_A, 0)

sint16 GetDoor(FLOOR_INFO *floor);

sint32 GetWaterDepth(sint32 x, sint32 y, sint32 z, sint16 room_number)
{
    int wh;
    ROOM_INFO *r;
    FLOOR_INFO *floor;
    sint16 data;
    int x_floor, y_floor;

    r = &room[room_number];
    do
    {
        x_floor = (z - r->z) >> WALL_SHIFT;
        y_floor = (x - r->x) >> WALL_SHIFT;

        if (x_floor <= 0)
        {
            x_floor = 0;
            if (y_floor < 1)
                y_floor = 1;
            else if (y_floor > r->y_size - 2)
                y_floor = r->y_size - 2;
        }
        else if (x_floor >= r->x_size - 1)
        {
            x_floor = r->x_size - 1;
            if (y_floor < 1)
                y_floor = 1;
            else if (y_floor > r->y_size - 2)
                y_floor = r->y_size - 2;
        }
        else if (y_floor < 0)
            y_floor = 0;
        else if (y_floor >= r->y_size)
            y_floor = r->y_size - 1;

        floor = &r->floor[x_floor + y_floor * r->x_size];
        data = GetDoor(floor);
        if (data != NO_ROOM)
        {
            room_number = data;
            r = &room[data];
        }
    }
    while (data != NO_ROOM);

    if (r->flags & UNDERWATER)
    {
        while (floor->sky_room != NO_ROOM)
        {
            r = &room[floor->sky_room];

            if (!(r->flags & UNDERWATER))
            {
                wh = floor->ceiling << 8;
                floor = GetFloor(x, y, z, &room_number);
                return GetHeight(floor, x, y, z) - wh;
            }
            floor = &r->floor[((z - r->z) >> WALL_SHIFT) + ((x - r->x) >> WALL_SHIFT) * r->x_size];
        }
        return 0x7fff;
    }
    else
    {
        while (floor->pit_room != NO_ROOM)
        {
            r = &room[floor->pit_room];

            if (r->flags & UNDERWATER)
            {
                wh = floor->floor << 8;
                floor = GetFloor(x, y, z, &room_number);
                return GetHeight(floor, x, y, z) - wh;
            }
            floor = &r->floor[((z - r->z) >> WALL_SHIFT) + ((x - r->x) >> WALL_SHIFT) * r->x_size];
        }
        return NO_HEIGHT;
    }
}

void LaraTestWaterDepth(ITEM_INFO *item, COLL_INFO *coll)
{
    int wd;
    FLOOR_INFO *floor;
    sint16 room_number;

    room_number = item->room_number;
    floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);

    wd = GetWaterDepth(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, room_number);
    if (wd == NO_HEIGHT)
    {
        item->pos.x_pos = coll->old.x;
        item->pos.y_pos = coll->old.y;
        item->pos.z_pos = coll->old.z;
        item->fallspeed = 0;
    }
    else if (wd <= STEP_L * 2)
    {
        item->anim_number = SWIM2QSTND_A;
        item->frame_number = SWIM2QSTND_F;
        item->current_anim_state = AS_WATEROUT;
        item->goal_anim_state = AS_STOP;
        item->pos.x_rot = item->pos.z_rot = 0;
        item->gravity_status = 0;
        item->speed = 0;
        item->fallspeed = 0;
        lara.water_status = LARA_WADE;

        item->pos.y_pos = GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);
    }
}

#define UW_WALLDEFLECT (2 * ONE_DEGREE)

void GetCollisionInfo(COLL_INFO *coll, sint32 xpos, sint32 ypos, sint32 zpos, sint16 room_number, sint32 objheight);

void LaraSwimCollision(ITEM_INFO *item, COLL_INFO *coll)
{
    int height;

    if (item->pos.x_rot >= -16384 && item->pos.x_rot <= 16384)
        coll->facing = lara.move_angle = item->pos.y_rot;
    else
        coll->facing = lara.move_angle = item->pos.y_rot - 32768;

    height = phd_sin(item->pos.x_rot) * LARA_HITE >> W2V_SHIFT;
    if (height < 0)
        height = -height;
    if (height < 200)
        height = 200;
    coll->bad_neg = -height;
    GetCollisionInfo(coll, item->pos.x_pos, item->pos.y_pos + height / 2, item->pos.z_pos, item->room_number, height);
    ShiftItem(item, coll);
    if (coll->coll_type == COLL_FRONT) {
        if (item->pos.x_rot > 35 * ONE_DEGREE)
            item->pos.x_rot += UW_WALLDEFLECT;
        else if (item->pos.x_rot < -35 * ONE_DEGREE)
            item->pos.x_rot -= UW_WALLDEFLECT;
        else
            item->fallspeed = 0;
    } else if (coll->coll_type == COLL_TOP) {
        if (item->pos.x_rot >= -45 * ONE_DEGREE)
            item->pos.x_rot -= UW_WALLDEFLECT;
    } else if (coll->coll_type == COLL_TOPFRONT)
        item->fallspeed = 0;
    else if (coll->coll_type == COLL_LEFT)
        item->pos.y_rot += 5 * ONE_DEGREE;
    else if (coll->coll_type == COLL_RIGHT)
        item->pos.y_rot -= 5 * ONE_DEGREE;
    else if (coll->coll_type == COLL_CLAMP) {
        item->pos.x_pos = coll->old.x;
        item->pos.y_pos = coll->old.y;
        item->pos.z_pos = coll->old.z;
        item->fallspeed = 0;
        return;
    }

    if (coll->mid_floor < 0) {
        item->pos.y_pos += coll->mid_floor;
        item->pos.x_rot += UW_WALLDEFLECT;
    }

    if (lara.water_status != LARA_CHEAT && !lara.extra_anim)
        LaraTestWaterDepth(item, coll);
}

#include "box.h"

int CalculateTarget(PHD_VECTOR *target, ITEM_INFO *item, LOT_INFO *LOT);

void LaraWaterCurrent(COLL_INFO *coll)
{
    PHD_VECTOR target;
    ROOM_INFO *r;
    FLOOR_INFO *floor;
    ITEM_INFO *item;

    item = lara_item;
    r = &room[lara_item->room_number];
    floor = &r->floor[((item->pos.z_pos - r->z) >> WALL_SHIFT) + ((item->pos.x_pos - r->x) >> WALL_SHIFT) * r->x_size];
    item->box_number = floor->box;

    if (!lara.creature)
    {
        lara.current_active = 0;
        return;
    }

    if (CalculateTarget(&target, item, &lara.creature->LOT) == 0)
        return;

    target.x -= item->pos.x_pos;
    if (target.x > lara.current_active)
        item->pos.x_pos += lara.current_active;
    else if (target.x < -lara.current_active)
        item->pos.x_pos -= lara.current_active;
    else
        item->pos.x_pos += target.x;

    target.z -= item->pos.z_pos;
    if (target.z > lara.current_active)
        item->pos.z_pos += lara.current_active;
    else if (target.z < -lara.current_active)
        item->pos.z_pos -= lara.current_active;
    else
        item->pos.z_pos += target.z;

    target.y -= item->pos.y_pos;
    if (target.y > lara.current_active)
        item->pos.y_pos += lara.current_active;
    else if (target.y < -lara.current_active)
        item->pos.y_pos -= lara.current_active;
    else
        item->pos.y_pos += target.y;

    lara.current_active = 0;

    coll->facing = phd_atan(item->pos.z_pos - coll->old.z, item->pos.x_pos - coll->old.x);
    GetCollisionInfo(coll, item->pos.x_pos, item->pos.y_pos + 200, item->pos.z_pos, item->room_number, 400);
    if (coll->coll_type == COLL_FRONT)
    {
        if (item->pos.x_rot > 35 * ONE_DEGREE)
            item->pos.x_rot += UW_WALLDEFLECT;
        else if (item->pos.x_rot < -35 * ONE_DEGREE)
            item->pos.x_rot -= UW_WALLDEFLECT;
        else
            item->fallspeed = 0;
    }
    else if (coll->coll_type == COLL_TOP)
        item->pos.x_rot -= UW_WALLDEFLECT;
    else if (coll->coll_type == COLL_TOPFRONT)
        item->fallspeed = 0;
    else if (coll->coll_type == COLL_LEFT)
        item->pos.y_rot += 5 * ONE_DEGREE;
    else if (coll->coll_type == COLL_RIGHT)
        item->pos.y_rot -= 5 * ONE_DEGREE;

    if (coll->mid_floor < 0)
    {
        item->pos.y_pos += coll->mid_floor;
        item->pos.x_rot += UW_WALLDEFLECT;
    }
    ShiftItem(item, coll);
    coll->old.x = item->pos.x_pos;
    coll->old.y = item->pos.y_pos;
    coll->old.z = item->pos.z_pos;
}
