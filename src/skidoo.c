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

#define SKIDOO_UNDO_TURN (ONE_DEGREE * 2)
#define SKIDOO_MOMENTUM_TURN (ONE_DEGREE * 3)
#define SKIDOO_MAX_MOM_TURN (ONE_DEGREE * 150)
#define SKIDOO_MAX_SPEED 100
#define SKIDOO_ACCELERATION 10
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
#define SKIDMAN 51
#define AVALANCHE 83
#define SNOW_SPRITE 90
#define TRAP_ACTIVATE 1

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
                            if (item->object_number == SKIDMAN)
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
