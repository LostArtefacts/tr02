#include "game.h"
#include "box.h"
#include "specific/init.h"

#define SKIDOO_STUFF 39
#define SKIDOO_ANIM 10

#define SKID_SIT 0
#define SKID_GETON 1
#define SKID_LEFT 2
#define SKID_RIGHT 3
#define SKID_FALL 4
#define SKID_HIT 5
#define SKID_GETONL 6
#define SKID_GETOFFL 7
#define SKID_STILL 8
#define SKID_GETOFF 9
#define SKID_LETGO 10
#define SKID_DEATH 11
#define SKID_FALLOFF 12

#define SKIDOO_GETON_ANIM 1
#define SKIDOO_GETONL_ANIM 18
#define SKIDOO_HIT_LEFT 11
#define SKIDOO_HIT_RIGHT 12
#define SKIDOO_HIT_FRONT 13
#define SKIDOO_HIT_BACK 14
#define SKIDOO_GETOFF_DIST 330

#define SKIDOO_UNDO_TURN (ONE_DEGREE * 2)
#define SKIDOO_TURN (ONE_DEGREE / 2 + SKIDOO_UNDO_TURN)
#define SKIDOO_MAX_TURN (ONE_DEGREE * 6)
#define SKIDOO_MOMENTUM_TURN (ONE_DEGREE * 3)
#define SKIDOO_MAX_MOM_TURN (ONE_DEGREE * 150)
#define SKIDOO_FAST_SPEED 150
#define SKIDOO_MAX_SPEED 100
#define SKIDOO_SLOW_SPEED 50
#define SKIDOO_MIN_SPEED 15
#define SKIDOO_ACCELERATION 10
#define SKIDOO_BRAKE 5
#define SKIDOO_SLOWDOWN 2
#define SKIDOO_REVERSE -5
#define SKIDOO_MAX_BACK -30
#define SKIDOO_MAX_KICK -80
#define SKIDOO_SLIP 100
#define SKIDOO_SLIP_SIDE 50
#define SKIDOO_FRONT 550
#define SKIDOO_SIDE 260
#define SKIDOO_RADIUS 500
#define SKIDOO_SNOW 500
#define SKIDOO_MIN_BOUNCE (SKIDOO_MAX_SPEED / 2)

#define GRAVITY 6
#define BANDIT3 51
#define AVALANCHE 83
#define SNOW_SPRITE 90
#define TRAP_ACTIVATE 1
#define BIG_SLOPE 2

typedef struct {
    sint16 track_mesh;
    int skidoo_turn;
    int left_fallspeed, right_fallspeed;
    sint16 momentum_angle, extra_rotation;
    int pitch;
} SKIDOO_INFO;

int TestCollision(ITEM_INFO *item, ITEM_INFO *laraitem);
void CreateFlare(int thrown);

void InitialiseSkidoo(sint16 item_number)
{
    ITEM_INFO *skidoo;
    SKIDOO_INFO *skinfo;

    skidoo = &items[item_number];
    skinfo = (SKIDOO_INFO *)game_malloc(sizeof(SKIDOO_INFO), SKIDOO_STUFF);
    skidoo->data = skinfo;
    skinfo->skidoo_turn = 0;
    skinfo->left_fallspeed = skinfo->right_fallspeed = 0;
    skinfo->momentum_angle = skidoo->pos.y_rot;
    skinfo->extra_rotation = 0;
    skinfo->track_mesh = 0;
    skinfo->pitch = 0;
}

int SkidooCheckGeton(sint16 item_number, COLL_INFO *coll)
{
    int geton;
    sint16 rot, room_number;
    ITEM_INFO *skidoo;
    FLOOR_INFO *floor;

    if (!(input & IN_ACTION) || lara.gun_status != LG_ARMLESS || lara_item->gravity_status)
        return 0;

    skidoo = &items[item_number];
    rot = skidoo->pos.y_rot - lara_item->pos.y_rot;
    if (rot > 0x2000 && rot < 0x6000)
        geton = 1;
    else if (rot > -0x6000 && rot < -0x2000)
        geton = 2;
    else
        return 0;

    if (!TestBoundsCollide(skidoo, lara_item, coll->radius))
        return 0;

    if (!TestCollision(skidoo, lara_item))
        return 0;

    room_number = skidoo->room_number;
    floor = GetFloor(skidoo->pos.x_pos, skidoo->pos.y_pos, skidoo->pos.z_pos, &room_number);
    if (GetHeight(floor, skidoo->pos.x_pos, skidoo->pos.y_pos, skidoo->pos.z_pos) < -32000)
        return 0;

    return geton;
}

void SkidmanPush(ITEM_INFO *item, ITEM_INFO *laraitem, int radius);

void SkidooBaddieCollision(ITEM_INFO *skidoo)
{
    int i, x, y, z;
    ITEM_INFO *item;
    OBJECT_INFO *object;
    sint16 roomies[12], numroom, *door;
    sint16 item_num;

    numroom = 1;
    roomies[0] = skidoo->room_number;

    door = room[skidoo->room_number].door;
    if (door)
    {
        for (i = (int)*(door++); i > 0; i--)
        {
            roomies[numroom++] = *(door);
            door += 16;
        }
    }

    for (i = 0; i < numroom; i++)
    {
        item_num = room[roomies[i]].item_number;
        while (item_num != NO_ITEM)
        {
            item = &items[item_num];
            if (item->collidable && item->status != INVISIBLE && item != lara_item && item != skidoo)
            {
                object = &objects[item->object_number];
                if (object->collision && (object->intelligent || item->object_number == AVALANCHE))
                {
                    x = skidoo->pos.x_pos - item->pos.x_pos;
                    y = skidoo->pos.y_pos - item->pos.y_pos;
                    z = skidoo->pos.z_pos - item->pos.z_pos;
                    if (x > -WALL_L * 2 && x < WALL_L * 2 && z > -WALL_L * 2 && z < WALL_L * 2 && y > -WALL_L * 2 && y < WALL_L * 2)
                    {
                        if (TestBoundsCollide(item, skidoo, SKIDOO_RADIUS))
                        {
                            if (item->object_number == BANDIT3)
                                SkidmanPush(item, skidoo, SKIDOO_RADIUS);
                            else if (item->object_number == AVALANCHE)
                            {
                                if (item->current_anim_state == TRAP_ACTIVATE)
                                {
                                    lara_item->hit_status = 1;
                                    lara_item->hit_points -= 100;
                                }
                            }
                            else
                            {
                                DoLotsOfBlood(item->pos.x_pos, skidoo->pos.y_pos - STEP_L, item->pos.z_pos, skidoo->speed, skidoo->pos.y_rot, item->room_number, 3);
                                item->hit_points = 0;
                            }
                        }
                    }
                }
            }
            item_num = item->next_item;
        }
    }
}

sint32 TestHeight(ITEM_INFO *item, sint32 z_off, sint32 x_off, PHD_VECTOR *pos)
{
    FLOOR_INFO *floor;
    sint32 s, c;
    sint16 room_number;

    pos->y = item->pos.y_pos - (z_off * phd_sin(item->pos.x_rot) >> W2V_SHIFT) + (x_off * phd_sin(item->pos.z_rot) >> W2V_SHIFT);

    c = phd_cos(item->pos.y_rot);
    s = phd_sin(item->pos.y_rot);

    pos->z = item->pos.z_pos + ((z_off * c - x_off * s) >> W2V_SHIFT);
    pos->x = item->pos.x_pos + ((z_off * s + x_off * c) >> W2V_SHIFT);

    room_number = item->room_number;
    floor = GetFloor(pos->x, pos->y, pos->z, &room_number);
    return GetHeight(floor, pos->x, pos->y, pos->z);
}

int DoShift(ITEM_INFO *skidoo, PHD_VECTOR *pos, PHD_VECTOR *old)
{
    int x, z, x_old, z_old, shift_x, shift_z;

    x = pos->x >> WALL_SHIFT;
    z = pos->z >> WALL_SHIFT;

    x_old = old->x >> WALL_SHIFT;
    z_old = old->z >> WALL_SHIFT;

    shift_x = pos->x & (WALL_L - 1);
    shift_z = pos->z & (WALL_L - 1);

    if (x == x_old)
    {
        if (z == z_old)
        {
            skidoo->pos.z_pos += (old->z - pos->z);
            skidoo->pos.x_pos += (old->x - pos->x);
        }
        else if (z > z_old)
        {
            skidoo->pos.z_pos -= shift_z + 1;
            return (pos->x - skidoo->pos.x_pos);
        }
        else
        {
            skidoo->pos.z_pos += WALL_L - shift_z;
            return (skidoo->pos.x_pos - pos->x);
        }
    }
    else if (z == z_old)
    {
        if (x > x_old)
        {
            skidoo->pos.x_pos -= shift_x + 1;
            return (skidoo->pos.z_pos - pos->z);
        }
        else
        {
            skidoo->pos.x_pos += WALL_L - shift_x;
            return (pos->z - skidoo->pos.z_pos);
        }
    }
    else
    {
        sint16 room_number;
        FLOOR_INFO *floor;
        sint32 height;

        x = z = 0;

        room_number = skidoo->room_number;
        floor = GetFloor(old->x, pos->y, pos->z, &room_number);
        height = GetHeight(floor, old->x, pos->y, pos->z);
        if (height < old->y - STEP_L)
        {
            if (pos->z > old->z)
                z = -shift_z - 1;
            else
                z = WALL_L - shift_z;
        }

        room_number = skidoo->room_number;
        floor = GetFloor(pos->x, pos->y, old->z, &room_number);
        height = GetHeight(floor, pos->x, pos->y, old->z);
        if (height < old->y - STEP_L)
        {
            if (pos->x > old->x)
                x = -shift_x - 1;
            else
                x = WALL_L - shift_x;
        }

        if (x && z)
        {
            skidoo->pos.z_pos += z;
            skidoo->pos.x_pos += x;
        }
        else if (z)
        {
            skidoo->pos.z_pos += z;
            if (z > 0)
                return (skidoo->pos.x_pos - pos->x);
            else
                return (pos->x - skidoo->pos.x_pos);
        }
        else if (x)
        {
            skidoo->pos.x_pos += x;
            if (x > 0)
                return (pos->z - skidoo->pos.z_pos);
            else
                return (skidoo->pos.z_pos - pos->z);
        }
        else
        {
            skidoo->pos.z_pos += (old->z - pos->z);
            skidoo->pos.x_pos += (old->x - pos->x);
        }
    }

    return 0;
}

int DoDynamics(int height, int fallspeed, int *y)
{
    int kick;

    if (height > *y)
    {
        *y += fallspeed;
        if (*y > height - SKIDOO_MIN_BOUNCE)
        {
            *y = height;
            fallspeed = 0;
        }
        else
            fallspeed += GRAVITY;
    }
    else
    {
        kick = height - *y << 2;
        if (kick < SKIDOO_MAX_KICK)
            kick = SKIDOO_MAX_KICK;
        fallspeed += (kick - fallspeed >> 3);
        if (*y > height)
            *y = height;
    }

    return fallspeed;
}

int GetCollisionAnim(ITEM_INFO *skidoo, PHD_VECTOR *moved)
{
    sint32 c, s, front, side;

    moved->x = skidoo->pos.x_pos - moved->x;
    moved->z = skidoo->pos.z_pos - moved->z;

    if (moved->x || moved->z)
    {
        c = phd_cos(skidoo->pos.y_rot);
        s = phd_sin(skidoo->pos.y_rot);
        front = (moved->z * c + moved->x * s) >> W2V_SHIFT;
        side = (-moved->z * s + moved->x * c) >> W2V_SHIFT;
        if (ABS(front) > ABS(side))
        {
            if (front > 0)
                return SKIDOO_HIT_BACK;
            else
                return SKIDOO_HIT_FRONT;
        }
        else
        {
            if (side > 0)
                return SKIDOO_HIT_LEFT;
            else
                return SKIDOO_HIT_RIGHT;
        }
    }

    return 0;
}

void S_CalculateLight(sint32 x, sint32 y, sint32 z, sint16 room_number);

extern sint32 LsAdder;

void DoSnowEffect(ITEM_INFO *skidoo)
{
    sint32 c, s, x;
    sint16 fx_number;
    FX_INFO *fx;

    fx_number = CreateEffect(skidoo->room_number);
    if (fx_number != NO_ITEM)
    {
        s = phd_sin(skidoo->pos.y_rot);
        c = phd_cos(skidoo->pos.y_rot);
        x = (GetRandomDraw() - 0x4000) * SKIDOO_SIDE >> 14;

        fx = &effects[fx_number];
        fx->pos.x_pos = skidoo->pos.x_pos - (SKIDOO_SNOW * s + x * c >> W2V_SHIFT);
        fx->pos.y_pos = skidoo->pos.y_pos + (SKIDOO_SNOW * phd_sin(skidoo->pos.x_rot) >> W2V_SHIFT);
        fx->pos.z_pos = skidoo->pos.z_pos - (SKIDOO_SNOW * c - x * s >> W2V_SHIFT);
        fx->room_number = skidoo->room_number;
        fx->frame_number = 0;
        fx->object_number = SNOW_SPRITE;
        fx->speed = 0;
        if (skidoo->speed < 64)
            fx->fallspeed = GetRandomDraw() * (ABS(skidoo->speed) - 64) >> 15;
        else
            fx->fallspeed = 0;

        *(phd_mxptr + M23) = 0;
        S_CalculateLight(fx->pos.x_pos, fx->pos.y_pos, fx->pos.z_pos, fx->room_number);
        fx->shade = LsAdder - 0x200;
        if (fx->shade < 0)
            fx->shade = 0;
    }
}

int SkidooDynamics(ITEM_INFO *skidoo)
{
    SKIDOO_INFO *skinfo;
    PHD_VECTOR moved, fl, fr, br, bl;
    PHD_VECTOR old, fl_old, fr_old, bl_old, br_old;
    sint32 hfl, hfr, hbr, hbl;
    sint32 hfr_old, hfl_old, hbr_old, hbl_old;
    FLOOR_INFO *floor;
    sint32 height, slip, collide;
    sint16 room_number, rot;
    int newspeed;

    skinfo = (SKIDOO_INFO *)skidoo->data;

    hfl_old = TestHeight(skidoo, SKIDOO_FRONT, -SKIDOO_SIDE, &fl_old);
    hfr_old = TestHeight(skidoo, SKIDOO_FRONT, SKIDOO_SIDE, &fr_old);
    hbl_old = TestHeight(skidoo, -SKIDOO_FRONT, -SKIDOO_SIDE, &bl_old);
    hbr_old = TestHeight(skidoo, -SKIDOO_FRONT, SKIDOO_SIDE, &br_old);

    old.x = skidoo->pos.x_pos;
    old.y = skidoo->pos.y_pos;
    old.z = skidoo->pos.z_pos;

    if (bl_old.y > hbl_old)
        bl_old.y = hbl_old;
    if (br_old.y > hbr_old)
        br_old.y = hbr_old;
    if (fl_old.y > hfl_old)
        fl_old.y = hfl_old;
    if (fr_old.y > hfr_old)
        fr_old.y = hfr_old;

    if (skidoo->pos.y_pos > skidoo->floor - STEP_L)
    {
        if (skinfo->skidoo_turn < -SKIDOO_UNDO_TURN)
            skinfo->skidoo_turn += SKIDOO_UNDO_TURN;
        else if (skinfo->skidoo_turn > SKIDOO_UNDO_TURN)
            skinfo->skidoo_turn -= SKIDOO_UNDO_TURN;
        else
            skinfo->skidoo_turn = 0;
        skidoo->pos.y_rot += skinfo->skidoo_turn + skinfo->extra_rotation;

        rot = skidoo->pos.y_rot - skinfo->momentum_angle;
        if (rot < -SKIDOO_MOMENTUM_TURN)
        {
            if (rot < -SKIDOO_MAX_MOM_TURN)
            {
                rot = -SKIDOO_MAX_MOM_TURN;
                skinfo->momentum_angle = skidoo->pos.y_rot - rot;
            }
            else
                skinfo->momentum_angle -= SKIDOO_MOMENTUM_TURN;
        }
        else if (rot > SKIDOO_MOMENTUM_TURN)
        {
            if (rot > SKIDOO_MAX_MOM_TURN)
            {
                rot = SKIDOO_MAX_MOM_TURN;
                skinfo->momentum_angle = skidoo->pos.y_rot - rot;
            }
            else
                skinfo->momentum_angle += SKIDOO_MOMENTUM_TURN;
        }
        else
            skinfo->momentum_angle = skidoo->pos.y_rot;
    }
    else
        skidoo->pos.y_rot += skinfo->skidoo_turn + skinfo->extra_rotation;

    skidoo->pos.z_pos += skidoo->speed * phd_cos(skinfo->momentum_angle) >> W2V_SHIFT;
    skidoo->pos.x_pos += skidoo->speed * phd_sin(skinfo->momentum_angle) >> W2V_SHIFT;

    slip = SKIDOO_SLIP * phd_sin(skidoo->pos.x_rot) >> W2V_SHIFT;
    if (ABS(slip) > SKIDOO_SLIP / 2)
    {
        skidoo->pos.z_pos -= slip * phd_cos(skidoo->pos.y_rot) >> W2V_SHIFT;
        skidoo->pos.x_pos -= slip * phd_sin(skidoo->pos.y_rot) >> W2V_SHIFT;
    }

    slip = SKIDOO_SLIP_SIDE * phd_sin(skidoo->pos.z_rot) >> W2V_SHIFT;
    if (ABS(slip) > SKIDOO_SLIP_SIDE / 2)
    {
        skidoo->pos.z_pos -= slip * phd_sin(skidoo->pos.y_rot) >> W2V_SHIFT;
        skidoo->pos.x_pos += slip * phd_cos(skidoo->pos.y_rot) >> W2V_SHIFT;
    }

    moved.x = skidoo->pos.x_pos;
    moved.z = skidoo->pos.z_pos;

    if (!(skidoo->flags & ONESHOT))
        SkidooBaddieCollision(skidoo);

    rot = 0;

    hbl = TestHeight(skidoo, -SKIDOO_FRONT, -SKIDOO_SIDE, &bl);
    if (hbl < bl_old.y - STEP_L)
        rot = DoShift(skidoo, &bl, &bl_old);

    hbr = TestHeight(skidoo, -SKIDOO_FRONT, SKIDOO_SIDE, &br);
    if (hbr < br_old.y - STEP_L)
        rot += DoShift(skidoo, &br, &br_old);

    hfl = TestHeight(skidoo, SKIDOO_FRONT, -SKIDOO_SIDE, &fl);
    if (hfl < fl_old.y - STEP_L)
        rot += DoShift(skidoo, &fl, &fl_old);

    hfr = TestHeight(skidoo, SKIDOO_FRONT, SKIDOO_SIDE, &fr);
    if (hfr < fr_old.y - STEP_L)
        rot += DoShift(skidoo, &fr, &fr_old);

    room_number = skidoo->room_number;
    floor = GetFloor(skidoo->pos.x_pos, skidoo->pos.y_pos, skidoo->pos.z_pos, &room_number);
    height = GetHeight(floor, skidoo->pos.x_pos, skidoo->pos.y_pos, skidoo->pos.z_pos);
    if (height < skidoo->pos.y_pos - STEP_L)
        DoShift(skidoo, (PHD_VECTOR *)&skidoo->pos, &old);

    skinfo->extra_rotation = rot;

    collide = GetCollisionAnim(skidoo, &moved);
    if (collide)
    {
        newspeed = (skidoo->pos.z_pos - old.z) * phd_cos(skinfo->momentum_angle) + (skidoo->pos.x_pos - old.x) * phd_sin(skinfo->momentum_angle) >> W2V_SHIFT;

        if (&items[lara.skidoo] == skidoo && skidoo->speed > SKIDOO_MAX_SPEED + SKIDOO_ACCELERATION && newspeed < skidoo->speed - 10)
        {
            lara_item->hit_points -= (skidoo->speed - newspeed) >> 1;
            lara_item->hit_status = 1;
        }

        if (skidoo->speed > 0 && newspeed < skidoo->speed)
            skidoo->speed = (newspeed < 0) ? 0 : newspeed;
        else if (skidoo->speed < 0 && newspeed > skidoo->speed)
            skidoo->speed = (newspeed > 0) ? 0 : newspeed;

        if (skidoo->speed < SKIDOO_MAX_BACK)
            skidoo->speed = SKIDOO_MAX_BACK;
    }

    return collide;
}

int SkidooUserControl(ITEM_INFO *skidoo, sint32 height, int *pitch)
{
    int drive = 0, max_speed;
    SKIDOO_INFO *skinfo;

    skinfo = (SKIDOO_INFO *)skidoo->data;

    if (skidoo->pos.y_pos >= height - STEP_L)
    {
        *pitch = skidoo->speed + (height - skidoo->pos.y_pos);

        if (skidoo->speed == 0 && (input & IN_LOOK))
            LookUpDown();

        if (((input & IN_LEFT) && !(input & IN_BACK)) || ((input & IN_RIGHT) && (input & IN_BACK)))
        {
            skinfo->skidoo_turn -= SKIDOO_TURN;
            if (skinfo->skidoo_turn < -SKIDOO_MAX_TURN)
                skinfo->skidoo_turn = -SKIDOO_MAX_TURN;
        }

        if (((input & IN_RIGHT) && !(input & IN_BACK)) || ((input & IN_LEFT) && (input & IN_BACK)))
        {
            skinfo->skidoo_turn += SKIDOO_TURN;
            if (skinfo->skidoo_turn > SKIDOO_MAX_TURN)
                skinfo->skidoo_turn = SKIDOO_MAX_TURN;
        }

        if (input & IN_BACK)
        {
            if (skidoo->speed > 0)
                skidoo->speed -= SKIDOO_BRAKE;
            else
            {
                if (skidoo->speed > SKIDOO_MAX_BACK)
                    skidoo->speed += SKIDOO_REVERSE;
                drive = 1;
            }
        }
        else if (input & IN_FORWARD)
        {
            if ((input & IN_ACTION) && !(skinfo->track_mesh & 4))
                max_speed = SKIDOO_FAST_SPEED;
            else if (input & IN_SLOW)
                max_speed = SKIDOO_SLOW_SPEED;
            else
                max_speed = SKIDOO_MAX_SPEED;

            if (skidoo->speed < max_speed)
                skidoo->speed += SKIDOO_ACCELERATION / 2 + SKIDOO_ACCELERATION * skidoo->speed / (2 * max_speed);
            else if (skidoo->speed > max_speed + SKIDOO_SLOWDOWN)
                skidoo->speed -= SKIDOO_SLOWDOWN;
            drive = 1;
        }
        else if (skidoo->speed >= 0 && skidoo->speed < SKIDOO_MIN_SPEED && (input & (IN_LEFT | IN_RIGHT)))
        {
            skidoo->speed = SKIDOO_MIN_SPEED;
            drive = 1;
        }
        else if (skidoo->speed > SKIDOO_SLOWDOWN)
        {
            skidoo->speed -= SKIDOO_SLOWDOWN;
            if ((GetRandomDraw() & 0x7f) < skidoo->speed)
                drive = 1;
        }
        else
            skidoo->speed = 0;
    }
    else if (input & (IN_FORWARD | IN_BACK))
    {
        drive = 1;
        *pitch = skinfo->pitch + 50;
    }

    return drive;
}

extern int height_type;
sint32 GetCeiling(FLOOR_INFO *floor, sint32 x, sint32 y, sint32 z);

int SkidooCheckGetOffOK(int direction)
{
    int x, y, z, height, ceiling;
    sint16 room_number, angle;
    ITEM_INFO *skidoo;
    FLOOR_INFO *floor;

    skidoo = &items[lara.skidoo];

    if (direction == SKID_GETOFFL)
        angle = skidoo->pos.y_rot + 0x4000;
    else
        angle = skidoo->pos.y_rot - 0x4000;

    x = skidoo->pos.x_pos - (SKIDOO_GETOFF_DIST * phd_sin(angle) >> W2V_SHIFT);
    y = skidoo->pos.y_pos;
    z = skidoo->pos.z_pos - (SKIDOO_GETOFF_DIST * phd_cos(angle) >> W2V_SHIFT);

    room_number = skidoo->room_number;
    floor = GetFloor(x, y, z, &room_number);
    height = GetHeight(floor, x, y, z);

    if (height_type == BIG_SLOPE || height == NO_HEIGHT)
        return 0;

    if (ABS(height - skidoo->pos.y_pos) > WALL_L / 2)
        return 0;

    ceiling = GetCeiling(floor, x, y, z);
    if (ceiling - skidoo->pos.y_pos > -LARA_HITE || height - ceiling < LARA_HITE)
        return 0;

    return 1;
}

#define SKIDOO_FALL_ANIM 8
#define DAMAGE_START 140
#define DAMAGE_LENGTH 14

extern sint16 cd_flags[];
void S_CDPlay(sint16 track, int mode);

void SkidooAnimation(ITEM_INFO *skidoo, int collide, int dead)
{
    sint16 cd;
    SKIDOO_INFO *skinfo;

    skinfo = (SKIDOO_INFO *)skidoo->data;

    if (skidoo->pos.y_pos != skidoo->floor && skidoo->fallspeed > 0 && lara_item->current_anim_state != SKID_FALL && !dead) {
        lara_item->anim_number = objects[SKIDOO_ANIM].anim_index + SKIDOO_FALL_ANIM;
        lara_item->frame_number = anims[lara_item->anim_number].frame_base;
        lara_item->current_anim_state = lara_item->goal_anim_state = SKID_FALL;
    } else if (collide && !dead && lara_item->current_anim_state != SKID_FALL) {
        if (lara_item->current_anim_state != SKID_HIT) {
            if (collide == SKIDOO_HIT_FRONT)
                SoundEffect(202, &skidoo->pos, 0);
            else
                SoundEffect(201, &skidoo->pos, 0);
            lara_item->anim_number = (sint16)(objects[SKIDOO_ANIM].anim_index + collide);
            lara_item->frame_number = anims[lara_item->anim_number].frame_base;
            lara_item->current_anim_state = lara_item->goal_anim_state = SKID_HIT;
        }
    } else {
        switch (lara_item->current_anim_state) {
        case SKID_STILL:
            cd = (skinfo->track_mesh & 4) ? 53 : 52;
            if (!(cd_flags[cd] & ONESHOT)) {
                S_CDPlay(cd, 0);
                cd_flags[cd] |= ONESHOT;
            }

            if (dead) {
                lara_item->goal_anim_state = SKID_DEATH;
                break;
            }

            lara_item->goal_anim_state = SKID_STILL;

            if (input & IN_JUMP) {
                if ((input & IN_RIGHT) && SkidooCheckGetOffOK(SKID_GETOFF)) {
                    lara_item->goal_anim_state = SKID_GETOFF;
                    skidoo->speed = 0;
                } else if ((input & IN_LEFT) && SkidooCheckGetOffOK(SKID_GETOFFL)) {
                    lara_item->goal_anim_state = SKID_GETOFFL;
                    skidoo->speed = 0;
                }
            } else if (input & IN_LEFT)
                lara_item->goal_anim_state = SKID_LEFT;
            else if (input & IN_RIGHT)
                lara_item->goal_anim_state = SKID_RIGHT;
            else if (input & (IN_FORWARD | IN_BACK))
                lara_item->goal_anim_state = SKID_SIT;
            break;

        case SKID_SIT:
            if (skidoo->speed == 0)
                lara_item->goal_anim_state = SKID_STILL;

            if (dead)
                lara_item->goal_anim_state = SKID_FALLOFF;
            else if (input & IN_LEFT)
                lara_item->goal_anim_state = SKID_LEFT;
            else if (input & IN_RIGHT)
                lara_item->goal_anim_state = SKID_RIGHT;
            break;

        case SKID_LEFT:
            if (!(input & IN_LEFT))
                lara_item->goal_anim_state = SKID_SIT;
            break;

        case SKID_RIGHT:
            if (!(input & IN_RIGHT))
                lara_item->goal_anim_state = SKID_SIT;
            break;

        case SKID_FALL:
            if (skidoo->fallspeed <= 0 || skinfo->left_fallspeed <= 0 || skinfo->right_fallspeed <= 0) {
                SoundEffect(203, &skidoo->pos, 0);
                lara_item->goal_anim_state = SKID_SIT;
            } else if (skidoo->fallspeed > DAMAGE_START + DAMAGE_LENGTH)
                lara_item->goal_anim_state = SKID_LETGO;
            break;
        }
    }
}

#define EXPLOSION1 229

int ExplodingDeath(sint16 item_number, sint32 mesh_bits, sint16 damage);

void SkidooExplode(ITEM_INFO *skidoo)
{
    int fx_number;
    FX_INFO *fx;

    fx_number = CreateEffect(skidoo->room_number);
    if (fx_number != NO_ITEM) {
        fx = &effects[fx_number];
        fx->pos.x_pos = skidoo->pos.x_pos;
        fx->pos.y_pos = skidoo->pos.y_pos;
        fx->pos.z_pos = skidoo->pos.z_pos;
        fx->speed = 0;
        fx->frame_number = 0;
        fx->counter = 0;
        fx->object_number = EXPLOSION1;
    }
    ExplodingDeath(lara.skidoo, 0xfffffffc, 0);
    SoundEffect(105, NULL, 0);
    lara.skidoo = NO_ITEM;
}

#define SKID_FASTFALL_A 23

int SkidooCheckGetOff(void)
{
    ITEM_INFO *skidoo;

    skidoo = &items[lara.skidoo];

    if ((lara_item->current_anim_state == SKID_GETOFF || lara_item->current_anim_state == SKID_GETOFFL) && lara_item->frame_number == anims[lara_item->anim_number].frame_end) {
        if (lara_item->current_anim_state == SKID_GETOFFL)
            lara_item->pos.y_rot += 0x4000;
        else
            lara_item->pos.y_rot -= 0x4000;

        lara_item->anim_number = STOP_A;
        lara_item->frame_number = STOP_F;
        lara_item->current_anim_state = lara_item->goal_anim_state = AS_STOP;
        lara_item->pos.x_pos -= SKIDOO_GETOFF_DIST * phd_sin(lara_item->pos.y_rot) >> W2V_SHIFT;
        lara_item->pos.z_pos -= SKIDOO_GETOFF_DIST * phd_cos(lara_item->pos.y_rot) >> W2V_SHIFT;
        lara_item->pos.x_rot = lara_item->pos.z_rot = 0;
        lara.skidoo = NO_ITEM;
        lara.gun_status = LG_ARMLESS;
    } else if (lara_item->current_anim_state == SKID_LETGO && (skidoo->pos.y_pos == skidoo->floor || lara_item->frame_number == anims[lara_item->anim_number].frame_end)) {
        lara_item->anim_number = SKID_FASTFALL_A;
        lara_item->frame_number = GF(SKID_FASTFALL_A, 0);
        lara_item->current_anim_state = AS_FASTFALL;

        if (skidoo->pos.y_pos == skidoo->floor) {
            lara_item->goal_anim_state = AS_DEATH;
            lara_item->fallspeed = DAMAGE_START + DAMAGE_LENGTH;
            lara_item->speed = 0;
            SkidooExplode(skidoo);
        } else {
            lara_item->goal_anim_state = AS_FASTFALL;
            lara_item->pos.y_pos -= 200;
            lara_item->fallspeed = skidoo->fallspeed;
            lara_item->speed = skidoo->speed;
            SoundEffect(30, &lara_item->pos, 0);
        }

        lara_item->pos.x_rot = lara_item->pos.z_rot = 0;
        lara_item->gravity_status = 1;
        lara.gun_status = LG_ARMLESS;
        lara.move_angle = skidoo->pos.y_rot;
        skidoo->flags |= ONESHOT;
        skidoo->collidable = 0;

        return 0;
    }

    return 1;
}

#define LG_SKIDOO 9

typedef struct {
    PHD_ANGLE lock_angles[4];
    PHD_ANGLE left_angles[4];
    PHD_ANGLE right_angles[4];
    PHD_ANGLE aim_speed;
    PHD_ANGLE shot_accuracy;
    int gun_height;
    int damage;
    int target_dist;
    sint16 recoil_frame;
    sint16 flash_time;
    sint16 sample_num;
} WEAPON_INFO;

extern WEAPON_INFO weapons[];
extern BITE_INFO left_gun;
extern BITE_INFO right_gun;

void LaraGetNewTarget(WEAPON_INFO *winfo);
void AimWeapon(WEAPON_INFO *winfo, LARA_ARM *arm);
int FireWeapon(int weapon_type, ITEM_INFO *target, ITEM_INFO *src, PHD_ANGLE *angles);
void AddDynamicLight(sint32 x, sint32 y, sint32 z, sint32 intensity, sint32 falloff);
sint16 GunShot(sint32 x, sint32 y, sint32 z, sint16 speed, PHD_ANGLE yrot, sint16 room_number);

void SkidooGuns(void)
{
    WEAPON_INFO *winfo;
    PHD_ANGLE angles[2];
    ITEM_INFO *skidoo;

    winfo = &weapons[LG_SKIDOO];

    LaraGetNewTarget(winfo);

    AimWeapon(winfo, &lara.right_arm);

    if (input & IN_ACTION) {
        angles[0] = lara.right_arm.y_rot + lara_item->pos.y_rot;
        angles[1] = lara.right_arm.x_rot;
        if (FireWeapon(LG_SKIDOO, lara.target, lara_item, angles)) {
            lara.right_arm.flash_gun = winfo->flash_time;
            SoundEffect(winfo->sample_num, &lara_item->pos, 0);
            AddDynamicLight(lara_item->pos.x_pos + (phd_sin(lara_item->pos.y_rot) >> (W2V_SHIFT - 10)), lara_item->pos.y_pos - WALL_L / 2, lara_item->pos.z_pos + (phd_cos(lara_item->pos.y_rot) >> (W2V_SHIFT - 10)), 12, 11);

            skidoo = &items[lara.skidoo];
            CreatureEffect(skidoo, &left_gun, GunShot);
            CreatureEffect(skidoo, &right_gun, GunShot);
        }
    }
}

#define NOT_VISIBLE ONESHOT

extern int level_items;

sint16 CreateItem(void);
void InitialiseItem(sint16 item_num);
void S_ExitSystem(char *message);

void InitialiseSkidman(sint16 item_number)
{
    sint16 skidoo_item;
    ITEM_INFO *item, *skidoo;

    skidoo_item = CreateItem();
    if (skidoo_item != NO_ITEM) {
        item = &items[item_number];
        skidoo = &items[skidoo_item];
        skidoo->object_number = BANDIT3;
        skidoo->pos.x_pos = item->pos.x_pos;
        skidoo->pos.y_pos = item->pos.y_pos;
        skidoo->pos.z_pos = item->pos.z_pos;
        skidoo->pos.y_rot = item->pos.y_rot;
        skidoo->room_number = item->room_number;
        skidoo->flags = NOT_VISIBLE;
        skidoo->shade = -1;

        InitialiseItem(skidoo_item);

        item->data = (void *)skidoo_item;

        level_items++;
    } else
        S_ExitSystem("FATAL: Cannot create SKIDOO for SKIDMAN");
}
