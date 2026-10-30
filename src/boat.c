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
