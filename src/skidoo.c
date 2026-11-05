#include <ddraw.h>
#include <malloc.h>
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

void ObjectCollision(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *);
void undraw_flare_meshes(void);

void SkidooCollision(sint16 item_number, ITEM_INFO *litem, COLL_INFO *coll)
{
    int geton;
    ITEM_INFO *skidoo;

    if (litem->hit_points < 0 || lara.skidoo != NO_ITEM)
        return;

    geton = SkidooCheckGeton(item_number, coll);
    if (!geton)
    {
        ObjectCollision(item_number, litem, coll);
        return;
    }

    lara.skidoo = item_number;

    if (lara.gun_type == LG_FLARE)
    {
        CreateFlare(0);
        undraw_flare_meshes();
        lara.flare_control_left = 0;
        lara.request_gun_type = lara.gun_type = LG_ARMLESS;
    }

    if (geton == 1)
        litem->anim_number = objects[SKIDOO_ANIM].anim_index + SKIDOO_GETON_ANIM;
    else
        litem->anim_number = objects[SKIDOO_ANIM].anim_index + SKIDOO_GETONL_ANIM;
    litem->frame_number = anims[litem->anim_number].frame_base;
    litem->current_anim_state = SKID_GETON;
    lara.gun_status = LG_HANDSBUSY;

    skidoo = &items[item_number];
    litem->pos.y_rot = skidoo->pos.y_rot;
    litem->pos.x_pos = skidoo->pos.x_pos;
    litem->pos.y_pos = skidoo->pos.y_pos;
    litem->pos.z_pos = skidoo->pos.z_pos;
    skidoo->hit_points = 1;
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

#define SKIDOO 13
#define SKIDOO_DEAD_ANIM 15
#define PITCH_SHIFT 4

int SkidooControl(void)
{
    ITEM_INFO *skidoo;
    sint16 lara_anim_number;
    sint16 skidoo_anim_number;
    ITEM_INFO *rider;
    SKIDOO_INFO *skinfo;
    PHD_VECTOR fl, fr;
    sint32 hfl, hfr;
    FLOOR_INFO *floor;
    sint32 height, collide, drive;
    sint16 room_number, x_rot, z_rot, bandit_skidoo;
    int pitch, dead = 0;

    skidoo = &items[lara.skidoo];
    skinfo = (SKIDOO_INFO *)skidoo->data;
    collide = SkidooDynamics(skidoo);

    hfl = TestHeight(skidoo, SKIDOO_FRONT, -SKIDOO_SIDE, &fl);
    hfr = TestHeight(skidoo, SKIDOO_FRONT, SKIDOO_SIDE, &fr);

    room_number = skidoo->room_number;
    floor = GetFloor(skidoo->pos.x_pos, skidoo->pos.y_pos, skidoo->pos.z_pos, &room_number);
    height = GetHeight(floor, skidoo->pos.x_pos, skidoo->pos.y_pos, skidoo->pos.z_pos);
    TestTriggers(trigger_index, 0);

    if (lara_item->hit_points <= 0) {
        input &= ~(IN_LEFT | IN_RIGHT | IN_BACK | IN_FORWARD);
        dead = 1;
    } else if (lara_item->current_anim_state == SKID_LETGO) {
        dead = 1;
        collide = 0;
    }

    if (skidoo->flags & ONESHOT) {
        drive = 0;
        collide = 0;
    } else {
        switch (lara_item->current_anim_state) {
        case SKID_GETON:
        case SKID_GETOFF:
        case SKID_GETOFFL:
        case SKID_LETGO:
            drive = -1;
            collide = 0;
            break;

        default:
            drive = SkidooUserControl(skidoo, height, &pitch);
        }
    }

    bandit_skidoo = skinfo->track_mesh & 4;
    if (drive > 0) {
        skinfo->track_mesh = ((skinfo->track_mesh & 3) == 1) ? 2 : 1;

        skinfo->pitch += (pitch - skinfo->pitch) >> 2;
        SoundEffect(155, &skidoo->pos, PITCH_SHIFT + ((0x10000 - (SKIDOO_MAX_SPEED - skinfo->pitch) * 100) << 8));
    } else {
        skinfo->track_mesh = 0;
        if (!drive)
            SoundEffect(153, &skidoo->pos, 0);
        skinfo->pitch = 0;
    }
    skinfo->track_mesh |= bandit_skidoo;

    skidoo->floor = height;

    skinfo->left_fallspeed = DoDynamics(hfl, skinfo->left_fallspeed, &fl.y);
    skinfo->right_fallspeed = DoDynamics(hfr, skinfo->right_fallspeed, &fr.y);
    skidoo->fallspeed = DoDynamics(height, skidoo->fallspeed, &skidoo->pos.y_pos);

    height = fl.y + fr.y >> 1;
    x_rot = phd_atan(SKIDOO_FRONT, skidoo->pos.y_pos - height);
    z_rot = phd_atan(SKIDOO_SIDE, height - fl.y);

    skidoo->pos.x_rot += x_rot - skidoo->pos.x_rot >> 1;
    skidoo->pos.z_rot += z_rot - skidoo->pos.z_rot >> 1;

    if (skidoo->flags & ONESHOT) {
        if (room_number != skidoo->room_number)
            ItemNewRoom(lara.skidoo, room_number);

        if (skidoo->pos.y_pos == skidoo->floor)
            SkidooExplode(skidoo);
        return 0;
    }

    SkidooAnimation(skidoo, collide, dead);

    if (room_number != skidoo->room_number) {
        ItemNewRoom(lara.skidoo, room_number);
        ItemNewRoom(lara.item_number, room_number);
    }

    if (lara_item->current_anim_state != SKID_FALLOFF) {
        lara_item->pos.x_pos = skidoo->pos.x_pos;
        lara_item->pos.y_pos = skidoo->pos.y_pos;
        lara_item->pos.z_pos = skidoo->pos.z_pos;
        lara_item->pos.y_rot = skidoo->pos.y_rot;
        if (drive >= 0) {
            lara_item->pos.x_rot = skidoo->pos.x_rot;
            lara_item->pos.z_rot = skidoo->pos.z_rot;
        } else
            lara_item->pos.x_rot = lara_item->pos.z_rot = 0;
    } else
        lara_item->pos.x_rot = lara_item->pos.z_rot = 0;

    AnimateItem(lara_item);

    if (!dead && drive >= 0 && bandit_skidoo)
        SkidooGuns();

    if (!dead) {
        skidoo->anim_number = objects[SKIDOO].anim_index + (lara_item->anim_number - objects[SKIDOO_ANIM].anim_index);
        rider = lara_item;
        lara_anim_number = rider->anim_number;
        skidoo_anim_number = skidoo->anim_number;
        skidoo->frame_number = anims[skidoo_anim_number].frame_base - anims[lara_anim_number].frame_base + rider->frame_number;
    } else {
        skidoo->anim_number = objects[SKIDOO].anim_index + SKIDOO_DEAD_ANIM;
        skidoo->frame_number = anims[skidoo->anim_number].frame_base;
    }

    if (skidoo->speed && skidoo->floor == skidoo->pos.y_pos) {
        DoSnowEffect(skidoo);
        if (skidoo->speed < 50)
            DoSnowEffect(skidoo);
    }

    return SkidooCheckGetOff();
}

#define SKID_TRACK 91

int GetFrames(ITEM_INFO *, sint16 *[], int *);
int S_GetObjectBounds(sint16 *bptr);
void CalculateObjectLighting(ITEM_INFO *item, sint16 *frame);
void phd_RotYXZ(PHD_ANGLE ry, PHD_ANGLE rx, PHD_ANGLE rz);
int phd_TranslateRel(sint32 x, sint32 y, sint32 z);
void phd_RotYXZsuperpack(sint16 **pprot, int skip);
void InitInterpolate(int frac, int rate);
void phd_TranslateRel_ID(sint32 x, sint32 y, sint32 z, sint32 x2, sint32 y2, sint32 z2);
void phd_RotYXZsuperpack_I(sint16 **pprot1, sint16 **pprot2, int skip);
void phd_PutPolygons_I(sint16 *ptr, int clip);
void phd_PopMatrix_I(void);
void phd_PushMatrix_I(void);
void phd_TranslateRel_I(sint32 x, sint32 y, sint32 z);

void DrawSkidoo(ITEM_INFO *item)
{
    int clip, i, poppush;
    OBJECT_INFO *object;
    sint32 *bone;
    sint16 *rotation1, *rotation2;
    sint16 **meshpp, **track;
    sint16 *frmptr[2];
    int rate, frac;
    sint16 flags = 0;

    frac = GetFrames(item, frmptr, &rate);

    if (item->data)
        flags = *(sint16 *)item->data;

    if (flags & 4)
        object = &objects[BANDIT3];
    else
        object = &objects[item->object_number];

    phd_PushMatrix();
    phd_TranslateAbs(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);
    phd_RotYXZ(item->pos.y_rot, item->pos.x_rot, item->pos.z_rot);

    if ((clip = S_GetObjectBounds(frmptr[0]))) {
        CalculateObjectLighting(item, frmptr[0]);

        meshpp = &meshes[object->mesh_index];
        bone = bones + object->bone_index;

        track = NULL;
        if ((flags & 3) == 1)
            track = &meshes[objects[SKID_TRACK].mesh_index] + 1;
        else if ((flags & 3) == 2)
            track = &meshes[objects[SKID_TRACK].mesh_index] + 7;

        if (!frac) {
            rotation1 = frmptr[0] + 9;

            phd_TranslateRel((sint32)*(frmptr[0] + 6), (sint32)*(frmptr[0] + 7), (sint32)*(frmptr[0] + 8));
            phd_RotYXZsuperpack(&rotation1, 0);
            phd_PutPolygons(*meshpp, clip);

            for (i = object->nmeshes - 1, meshpp++; i > 0; i--, bone += 4, meshpp++) {
                poppush = *(bone);
                if (poppush & 1)
                    phd_PopMatrix();
                if (poppush & 2)
                    phd_PushMatrix();

                phd_TranslateRel(*(bone + 1), *(bone + 2), *(bone + 3));
                phd_RotYXZsuperpack(&rotation1, 0);

                if (track) {
                    phd_PutPolygons(*track, clip);
                    track = NULL;
                } else
                    phd_PutPolygons(*meshpp, clip);
            }
        } else {
            rotation1 = frmptr[0] + 9;
            rotation2 = frmptr[1] + 9;

            InitInterpolate(frac, rate);
            phd_TranslateRel_ID((sint32)*(frmptr[0] + 6), (sint32)*(frmptr[0] + 7), (sint32)*(frmptr[0] + 8), (sint32)*(frmptr[1] + 6), (sint32)*(frmptr[1] + 7), (sint32)*(frmptr[1] + 8));
            phd_RotYXZsuperpack_I(&rotation1, &rotation2, 0);
            phd_PutPolygons_I(*meshpp, clip);

            for (i = object->nmeshes - 1, meshpp++; i > 0; i--, bone += 4, meshpp++) {
                poppush = *(bone);
                if (poppush & 1)
                    phd_PopMatrix_I();
                if (poppush & 2)
                    phd_PushMatrix_I();

                phd_TranslateRel_I(*(bone + 1), *(bone + 2), *(bone + 3));
                phd_RotYXZsuperpack_I(&rotation1, &rotation2, 0);

                if (track) {
                    phd_PutPolygons_I(*track, clip);
                    track = NULL;
                } else
                    phd_PutPolygons_I(*meshpp, clip);
            }
        }
    }
    phd_PopMatrix();
}

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

sint16 *GetBestFrame(ITEM_INFO *item);

#define SKIDOO 13
#define SKIDMAN 52
#define PITCH_SHIFT 4
#define VIOLENT 1
#define DONT_TARGET (-16384)

#define SMAN_EMPTY 0
#define SMAN_WAIT 1
#define SMAN_MOVING 2
#define SMAN_STARTLEFT 3
#define SMAN_STARTRIGHT 4
#define SMAN_LEFT 5
#define SMAN_RIGHT 6
#define SMAN_DEATH 7

#define SMAN_SHOT_DAMAGE 10
#define SMAN_LARA_DAMAGE 50
#define SMAN_MIN_TURN (SKIDOO_MAX_TURN / 3)
#define SMAN_TARGET_ANGLE (ONE_DEGREE * 15)
#define SMAN_WAIT_RANGE SQUARE(WALL_L * 4)
#define SMAN_DEATH_ANIM 10
#define SQUARE(A) ((A) * (A))

typedef struct {
    sint16 zone_number, enemy_zone;
    sint32 distance, ahead, bite;
    sint16 angle, enemy_facing;
} AI_INFO;

int EnableBaddieAI(sint16 item_number, int always);
void DisableBaddieAI(sint16 item_number);
void CreatureAIInfo(ITEM_INFO *item, AI_INFO *info);
void CreatureMood(ITEM_INFO *, AI_INFO *, int);
sint16 CreatureTurn(ITEM_INFO *item, sint16 maximum_turn);
int CreatureAnimation(sint16 item_number, sint16 angle, sint16 tilt);
int ShotLara(ITEM_INFO *item, AI_INFO *info, BITE_INFO *gun, sint16 extra_rotation, int damage);

void SkidManControl(sint16 rider_number)
{
    ITEM_INFO *item, *rider;
    CREATURE_INFO *skidman;
    AI_INFO info;
    sint16 angle, item_number;
    int damage;

    rider = &items[rider_number];

    item_number = (int)rider->data;
    item = &items[item_number];

    if (!item->data) {
        EnableBaddieAI(item_number, 1);
        item->status = ACTIVE;
    }
    skidman = (CREATURE_INFO *)item->data;

    angle = 0;

    if (item->hit_points <= 0) {
        if (rider->current_anim_state != SMAN_DEATH) {
            rider->pos.x_pos = item->pos.x_pos;
            rider->pos.y_pos = item->pos.y_pos;
            rider->pos.z_pos = item->pos.z_pos;
            rider->pos.y_rot = item->pos.y_rot;
            rider->room_number = item->room_number;

            rider->anim_number = objects[SKIDMAN].anim_index + SMAN_DEATH_ANIM;
            rider->frame_number = anims[rider->anim_number].frame_base;
            rider->current_anim_state = SMAN_DEATH;

            if (lara.target == item)
                lara.target = NULL;
        } else
            AnimateItem(rider);

        if (item->current_anim_state == SMAN_MOVING || item->current_anim_state == SMAN_WAIT)
            item->goal_anim_state = SMAN_WAIT;
        else
            item->goal_anim_state = SMAN_MOVING;
    } else {
        CreatureAIInfo(item, &info);

        CreatureMood(item, &info, VIOLENT);

        angle = CreatureTurn(item, SKIDOO_MAX_TURN / 2);

        switch (item->current_anim_state) {
        case SMAN_WAIT:
            if (skidman->mood == BORED_MOOD)
                break;
            else if (ABS(info.angle) < SMAN_TARGET_ANGLE && info.distance < SMAN_WAIT_RANGE)
                break;
            item->goal_anim_state = SMAN_MOVING;
            break;

        case SMAN_MOVING:
            if (skidman->mood == BORED_MOOD)
                item->goal_anim_state = SMAN_WAIT;
            else if (ABS(info.angle) < SMAN_TARGET_ANGLE && info.distance < SMAN_WAIT_RANGE)
                item->goal_anim_state = SMAN_WAIT;
            else if (angle < -SMAN_MIN_TURN)
                item->goal_anim_state = SMAN_STARTLEFT;
            else if (angle > SMAN_MIN_TURN)
                item->goal_anim_state = SMAN_STARTRIGHT;
            break;

        case SMAN_STARTLEFT:
        case SMAN_LEFT:
            if (angle < -SMAN_MIN_TURN)
                item->goal_anim_state = SMAN_LEFT;
            else
                item->goal_anim_state = SMAN_MOVING;
            break;

        case SMAN_STARTRIGHT:
        case SMAN_RIGHT:
            if (angle < -SMAN_MIN_TURN)
                item->goal_anim_state = SMAN_LEFT;
            else
                item->goal_anim_state = SMAN_MOVING;
            break;
        }
    }

    if (rider->current_anim_state != SMAN_DEATH) {
        if (!skidman->flags && ABS(info.angle) < SMAN_TARGET_ANGLE && lara_item->hit_points > 0) {
            damage = (lara.skidoo != NO_ITEM) ? SMAN_SHOT_DAMAGE : SMAN_LARA_DAMAGE;

            if (ShotLara(item, &info, &left_gun, 0, damage) + ShotLara(item, &info, &right_gun, 0, damage))
                skidman->flags = 5;
        }

        if (skidman->flags) {
            SoundEffect(43, &item->pos, 0);
            skidman->flags--;
        }
    }

    if (item->current_anim_state == SMAN_WAIT) {
        SoundEffect(153, &item->pos, 0);
        skidman->head_rotation = 0;
    } else {
        skidman->head_rotation = (skidman->head_rotation == 1) ? 2 : 1;
        DoSnowEffect(item);
        SoundEffect(155, &item->pos, PITCH_SHIFT + ((0x10000 - (SKIDOO_MAX_SPEED - item->speed) * 100) << 8));
    }

    CreatureAnimation(item_number, angle, 0);

    if (rider->current_anim_state != SMAN_DEATH) {
        rider->pos.x_pos = item->pos.x_pos;
        rider->pos.y_pos = item->pos.y_pos;
        rider->pos.z_pos = item->pos.z_pos;
        rider->pos.y_rot = item->pos.y_rot;
        if (item->room_number != rider->room_number)
            ItemNewRoom(rider_number, item->room_number);

        rider->anim_number = objects[SKIDMAN].anim_index + (item->anim_number - objects[BANDIT3].anim_index);
        rider->frame_number = item->frame_number + (anims[rider->anim_number].frame_base - anims[item->anim_number].frame_base);
    } else if (rider->status == DEACTIVATED && item->speed == 0 && item->fallspeed == 0) {
        RemoveActiveItem(rider_number);
        rider->collidable = 0;
        rider->hit_points = DONT_TARGET;
        rider->flags |= ONESHOT;

        DisableBaddieAI(item_number);
        item->object_number = SKIDOO;
        item->status = DEACTIVATED;
        InitialiseSkidoo(item_number);
        *(sint16 *)item->data = 4;
    }
}

void SkidmanPush(ITEM_INFO *item, ITEM_INFO *laraitem, int radius)
{
    int x, z, rx, rz, c, s;
    int minx, maxx, minz, maxz;
    int l, r, t, b;
    OBJECT_INFO *object;
    sint16 *bounds;

    x = laraitem->pos.x_pos - item->pos.x_pos;
    z = laraitem->pos.z_pos - item->pos.z_pos;
    c = phd_cos(item->pos.y_rot);
    s = phd_sin(item->pos.y_rot);
    rx = (c * x - s * z) >> W2V_SHIFT;
    rz = (c * z + s * x) >> W2V_SHIFT;

    object = &objects[item->object_number];
    bounds = GetBestFrame(item);
    minx = *(bounds + 0) - radius;
    maxx = *(bounds + 1) + radius;
    minz = *(bounds + 4) - radius;
    maxz = *(bounds + 5) + radius;

    if (rx >= minx && rx <= maxx && rz >= minz && rz <= maxz) {
        l = rx - minx;
        r = maxx - rx;
        t = maxz - rz;
        b = rz - minz;

        if (l <= r && l <= t && l <= b)
            rx -= l;
        else if (r <= l && r <= t && r <= b)
            rx += r;
        else if (t <= l && t <= r && t <= b)
            rz += t;
        else
            rz -= b;

        laraitem->pos.x_pos = item->pos.x_pos + ((c * rx + s * rz) >> W2V_SHIFT);
        laraitem->pos.z_pos = item->pos.z_pos + ((c * rz - s * rx) >> W2V_SHIFT);
    }
}

void SkidmanCollision(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll)
{
    ITEM_INFO *item;

    item = &items[item_num];
    if (!TestBoundsCollide(item, laraitem, coll->radius))
        return;
    if (!TestCollision(item, laraitem))
        return;

    if (coll->enable_baddie_push) {
        if (item->speed > 0)
            ItemPushLara(item, laraitem, coll, coll->enable_spaz, 0);
        else
            ItemPushLara(item, laraitem, coll, 0, 0);
    }

    if (lara.skidoo == NO_ITEM && item->speed > 0) {
        laraitem->hit_status = 1;
        laraitem->hit_points -= 100;
    }
}

#ifdef TR2_EXE
BITE_INFO left_gun = { 219, -71, 550, 0 };
BITE_INFO right_gun = { -235, -71, 550, 0 };
ITEM_INFO *lara_item;
#endif
