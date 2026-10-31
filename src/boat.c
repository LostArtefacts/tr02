#include "game.h"
#include "box.h"
#include "specific/init.h"

#define BOAT_GETON 0
#define BOAT_STILL 1
#define BOAT_MOVING 2
#define BOAT_JUMPR 3
#define BOAT_JUMPL 4
#define BOAT_HIT 5
#define BOAT_FALL 6
#define BOAT_NOTUSED 7
#define BOAT_DEATH 8

#define BOAT_GETONLW_ANIM 0
#define BOAT_GETONRW_ANIM 8
#define BOAT_GETONJ_ANIM 6
#define BOAT_GETON_START 1
#define BOAT_FALL_ANIM 15
#define BOAT_DEATH_ANIM 18

#define BOAT_UNDO_TURN (ONE_DEGREE / 4)
#define BOAT_TURN (ONE_DEGREE / 8)
#define BOAT_MAX_TURN (ONE_DEGREE * 4)
#define BOAT_MAX_SPEED 90
#define BOAT_SLOW_SPEED (BOAT_MAX_SPEED / 3)
#define BOAT_FAST_SPEED (BOAT_MAX_SPEED + 50)
#define BOAT_MIN_SPEED 20
#define BOAT_ACCELERATION 5
#define BOAT_BRAKE 5
#define BOAT_SLOWDOWN 1
#define BOAT_REVERSE -5
#define BOAT_MAX_BACK -20
#define BOAT_MAX_KICK -80
#define BOAT_SLIP 10
#define BOAT_SIDE_SLIP 30
#define BOAT_FRONT 750
#define BOAT_SIDE 300
#define BOAT_RADIUS 500
#define BOAT_SNOW 500
#define BOAT_MAX_HEIGHT (STEP_L)
#define BOAT_GETOFF_DIST (STEP_L * 2)
#define BOAT_WAKE 700
#define BOAT_SOUND_CEILING (WALL_L * 5)

typedef struct {
    int boat_turn;
    int left_fallspeed, right_fallspeed;
    sint16 tilt_angle, extra_rotation;
    int water, pitch;
} BOAT_INFO;

void InitialiseBoat(sint16 item_number)
{
    ITEM_INFO *boat;
    BOAT_INFO *binfo;

    boat = &items[item_number];
    binfo = (BOAT_INFO *)game_malloc(sizeof(BOAT_INFO), TEMP_ALLOC);
    boat->data = binfo;
    binfo->boat_turn = 0;
    binfo->left_fallspeed = binfo->right_fallspeed = 0;
    binfo->tilt_angle = 0;
    binfo->extra_rotation = 0;
    binfo->water = 0;
    binfo->pitch = 0;
}

#define MOTORBOAT 14
#define GONDOLA 24
#define GONDOLA_FLOATING 1
#define GONDOLA_CRASH 2
#define SQUARE(A) ((A) * (A))

sint32 TestWaterHeight(ITEM_INFO *item, sint32 z_off, sint32 x_off, PHD_VECTOR *pos)
{
    FLOOR_INFO *floor;
    sint16 room_number;
    sint32 c, height, s;

    pos->y = item->pos.y_pos - (z_off * phd_sin(item->pos.x_rot) >> W2V_SHIFT) + (x_off * phd_sin(item->pos.z_rot) >> W2V_SHIFT);
    c = phd_cos(item->pos.y_rot);
    s = phd_sin(item->pos.y_rot);
    pos->z = item->pos.z_pos + ((z_off * c - x_off * s) >> W2V_SHIFT);
    pos->x = item->pos.x_pos + ((z_off * s + x_off * c) >> W2V_SHIFT);
    room_number = item->room_number;
    GetFloor(pos->x, pos->y, pos->z, &room_number);
    height = GetWaterHeight(pos->x, pos->y, pos->z, room_number);
    if (height == NO_HEIGHT)
    {
        floor = GetFloor(pos->x, pos->y, pos->z, &room_number);
        height = GetHeight(floor, pos->x, pos->y, pos->z);
        if (height == NO_HEIGHT)
            return height;
    }
    return height - 5;
}

void DoBoatShift(int boat_number)
{
    ITEM_INFO *item, *boat;
    int item_number, distance, x, z, dx, dz;

    boat = &items[boat_number];
    item_number = room[boat->room_number].item_number;
    while (item_number != NO_ITEM) {
        item = &items[item_number];
        if (item->object_number == MOTORBOAT && item_number != boat_number && lara.skidoo != item_number) {
            x = item->pos.x_pos - boat->pos.x_pos;
            z = item->pos.z_pos - boat->pos.z_pos;
            distance = x * x + z * z;
            if (distance < SQUARE(BOAT_RADIUS * 2)) {
                boat->pos.x_pos = item->pos.x_pos - x * SQUARE(BOAT_RADIUS * 2) / distance;
                boat->pos.z_pos = item->pos.z_pos - z * SQUARE(BOAT_RADIUS * 2) / distance;
            }
            return;
        }

        if (item->object_number == GONDOLA && item->current_anim_state == GONDOLA_FLOATING) {
            x = item->pos.x_pos - (512 * phd_sin(item->pos.y_rot) >> W2V_SHIFT);
            z = item->pos.z_pos - (512 * phd_cos(item->pos.y_rot) >> W2V_SHIFT);
            dx = x - boat->pos.x_pos;
            dz = z - boat->pos.z_pos;
            distance = dx * dx + dz * dz;
            if (distance < SQUARE(BOAT_RADIUS * 2)) {
                if (boat->speed < BOAT_MAX_SPEED - 10) {
                    boat->pos.x_pos = x - dx * SQUARE(BOAT_RADIUS * 2) / distance;
                    boat->pos.z_pos = z - dz * SQUARE(BOAT_RADIUS * 2) / distance;
                } else if (item->pos.y_pos - boat->pos.y_pos < WALL_L * 2) {
                    SoundEffect(337, &item->pos, 0);
                    item->goal_anim_state = GONDOLA_CRASH;
                }
            }
        }

        item_number = item->next_item;
    }
}

#define WATER_SPRITE 89

void S_CalculateLight(sint32 x, sint32 y, sint32 z, sint16 room_number);

extern sint32 LsAdder;

void DoWakeEffect(ITEM_INFO *boat)
{
    int i;
    sint32 c, s, w, h;
    sint16 fx_number, frame;
    FX_INFO *fx;

    *(phd_mxptr + M23) = 0;
    S_CalculateLight(boat->pos.x_pos, boat->pos.y_pos, boat->pos.z_pos, boat->room_number);

    frame = GetRandomDraw() * objects[WATER_SPRITE].nmeshes >> 15;
    for (i = 0; i < 3; i++) {
        fx_number = CreateEffect(boat->room_number);
        if (fx_number != NO_ITEM) {
            fx = &effects[fx_number];
            c = phd_cos(boat->pos.y_rot);
            s = phd_sin(boat->pos.y_rot);
            h = BOAT_WAKE;
            w = (1 - i) * BOAT_SIDE;
            fx->pos.x_pos = boat->pos.x_pos + ((-(c * w) - (h * s)) >> W2V_SHIFT);
            fx->pos.y_pos = boat->pos.y_pos;
            fx->pos.z_pos = boat->pos.z_pos + (((s * w) - (h * c)) >> W2V_SHIFT);
            fx->pos.y_rot = boat->pos.y_rot - (1 - i) * 0x4000;
            fx->room_number = boat->room_number;
            fx->frame_number = frame;
            fx->counter = 20;
            fx->object_number = WATER_SPRITE;
            fx->speed = boat->speed >> 2;
            if (boat->speed < 64)
                fx->fallspeed = GetRandomDraw() * (ABS(boat->speed) - 64) >> 15;
            else
                fx->fallspeed = 0;
            fx->shade = LsAdder - 0x300;
            if (fx->shade < 0)
                fx->shade = 0;
        }
    }
}

#define GRAVITY 6

int DoBoatDynamics(int height, int fallspeed, int *y)
{
    if (height > *y) {
        *y += fallspeed;
        if (*y > height) {
            *y = height;
            fallspeed = 0;
        } else
            fallspeed += GRAVITY;
    } else {
        fallspeed += ((height - *y - fallspeed) >> 3);
        if (fallspeed < -20)
            fallspeed = -20;
        if (*y > height)
            *y = height;
    }
    return fallspeed;
}

int BoatUserControl(ITEM_INFO *boat)
{
    int no_turn = 1, max_speed;
    BOAT_INFO *binfo;

    binfo = (BOAT_INFO *)boat->data;

    if (boat->pos.y_pos >= binfo->water - STEP_L / 2 && binfo->water != NO_HEIGHT) {
        if (!(input & IN_JUMP) && !(input & IN_LOOK)) {
            if (((input & IN_LEFT) && !(input & IN_BACK)) || ((input & IN_RIGHT) && (input & IN_BACK))) {
                if (binfo->boat_turn > 0)
                    binfo->boat_turn -= BOAT_UNDO_TURN;
                else {
                    binfo->boat_turn -= BOAT_TURN;
                    if (binfo->boat_turn < -BOAT_MAX_TURN)
                        binfo->boat_turn = -BOAT_MAX_TURN;
                }
                no_turn = 0;
            } else if (((input & IN_RIGHT) && !(input & IN_BACK)) || ((input & IN_LEFT) && (input & IN_BACK))) {
                if (binfo->boat_turn < 0)
                    binfo->boat_turn += BOAT_UNDO_TURN;
                else {
                    binfo->boat_turn += BOAT_TURN;
                    if (binfo->boat_turn > BOAT_MAX_TURN)
                        binfo->boat_turn = BOAT_MAX_TURN;
                }
                no_turn = 0;
            }

            if (input & IN_BACK) {
                if (boat->speed > 0)
                    boat->speed -= BOAT_BRAKE;
                else if (boat->speed > BOAT_MAX_BACK)
                    boat->speed += BOAT_REVERSE;
            } else if (input & IN_FORWARD) {
                if (input & IN_ACTION)
                    max_speed = BOAT_FAST_SPEED;
                else
                    max_speed = (input & IN_SLOW) ? BOAT_SLOW_SPEED : BOAT_MAX_SPEED;

                if (boat->speed < max_speed)
                    boat->speed += BOAT_ACCELERATION / 2 + BOAT_ACCELERATION * boat->speed / (2 * max_speed);
                else if (boat->speed > max_speed + BOAT_SLOWDOWN)
                    boat->speed -= BOAT_SLOWDOWN;
            } else if (boat->speed >= 0 && boat->speed < BOAT_MIN_SPEED && (input & (IN_LEFT | IN_RIGHT)))
                boat->speed = BOAT_MIN_SPEED;
            else if (boat->speed > BOAT_SLOWDOWN)
                boat->speed -= BOAT_SLOWDOWN;
            else
                boat->speed = 0;
        } else if ((input & IN_LOOK) && boat->speed == 0)
            LookUpDown();
    }

    return no_turn;
}

#define BOAT_ANIM 11

void BoatAnimation(ITEM_INFO *boat, int collide)
{
    BOAT_INFO *binfo;

    binfo = (BOAT_INFO *)boat->data;

    if (lara_item->hit_points <= 0) {
        if (lara_item->current_anim_state != BOAT_DEATH) {
            lara_item->anim_number = objects[BOAT_ANIM].anim_index + BOAT_DEATH_ANIM;
            lara_item->frame_number = anims[lara_item->anim_number].frame_base;
            lara_item->current_anim_state = lara_item->goal_anim_state = BOAT_DEATH;
        }
    } else if (boat->pos.y_pos < binfo->water - STEP_L / 2 && boat->fallspeed > 0) {
        if (lara_item->current_anim_state != BOAT_FALL) {
            lara_item->anim_number = objects[BOAT_ANIM].anim_index + BOAT_FALL_ANIM;
            lara_item->frame_number = anims[lara_item->anim_number].frame_base;
            lara_item->current_anim_state = lara_item->goal_anim_state = BOAT_FALL;
        }
    } else if (collide) {
        if (lara_item->current_anim_state != BOAT_HIT) {
            lara_item->anim_number = (sint16)(objects[BOAT_ANIM].anim_index + collide);
            lara_item->frame_number = anims[lara_item->anim_number].frame_base;
            lara_item->current_anim_state = lara_item->goal_anim_state = BOAT_HIT;
        }
    } else {
        switch (lara_item->current_anim_state) {
        case BOAT_STILL:
            if (input & IN_JUMP) {
                if (input & IN_RIGHT)
                    lara_item->goal_anim_state = BOAT_JUMPR;
                else if (input & IN_LEFT)
                    lara_item->goal_anim_state = BOAT_JUMPL;
            }
            if (boat->speed > 0)
                lara_item->goal_anim_state = BOAT_MOVING;
            break;

        case BOAT_MOVING:
            if (input & IN_JUMP) {
                if (input & IN_RIGHT)
                    lara_item->goal_anim_state = BOAT_JUMPR;
                else if (input & IN_LEFT)
                    lara_item->goal_anim_state = BOAT_JUMPL;
            } else if (boat->speed <= 0)
                lara_item->goal_anim_state = BOAT_STILL;
            break;

        case BOAT_FALL:
            lara_item->goal_anim_state = BOAT_MOVING;
            break;
        }
    }
}

int ExplodingDeath(sint16 item_number, sint32 mesh_bits, sint16 damage);

#define GONDOLA_FLOATING 1
#define GONDOLA_CRASH 2
#define GONDOLA_SINK 3
#define GONDOLA_LAND 4
#define GONDOLA_SINK_SPEED 50

void GondolaControl(sint16 item_number)
{
    ITEM_INFO *item;
    sint16 room_number;
    FLOOR_INFO *floor;

    item = &items[item_number];
    switch (item->current_anim_state) {
    case GONDOLA_FLOATING:
        if (item->goal_anim_state == GONDOLA_CRASH) {
            item->mesh_bits = 0xff;
            ExplodingDeath(item_number, 240, 0);
        }
        break;

    case GONDOLA_SINK:
        item->pos.y_pos += GONDOLA_SINK_SPEED;
        room_number = item->room_number;
        floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
        item->floor = GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);
        if (item->pos.y_pos >= item->floor) {
            item->goal_anim_state = GONDOLA_LAND;
            item->pos.y_pos = item->floor;
        }
        break;
    }

    AnimateItem(item);
    if (item->status == DEACTIVATED)
        RemoveActiveItem(item_number);
}
