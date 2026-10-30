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

#define SKIDOO_MAX_SPEED 100
#define SKIDOO_MAX_KICK -80
#define SKIDOO_RADIUS 500
#define SKIDOO_MIN_BOUNCE (SKIDOO_MAX_SPEED / 2)

#define GRAVITY 6
#define SKIDMAN 51
#define AVALANCHE 83
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
