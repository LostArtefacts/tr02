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
