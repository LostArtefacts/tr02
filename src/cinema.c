#include "game.h"

void SetCutsceneTrack(int track)
{
    cutscene_track = track;
}

void GetJointAbsPosition(ITEM_INFO *item, PHD_VECTOR *vec, int joint);
int GetCinematicRoom(sint32 x, sint32 y, sint32 z);
void AnimateLara(ITEM_INFO *item);

void LaraControlCinematic(sint16 item_num)
{
    ITEM_INFO *item;
    PHD_VECTOR pos;
    sint16 room_number;

    item = &items[item_num];
    item->pos.y_rot = camera.target_angle;
    item->pos.x_pos = camera.pos.x;
    item->pos.y_pos = camera.pos.y;
    item->pos.z_pos = camera.pos.z;

    pos.x = 0;
    pos.y = 0;
    pos.z = 0;
    GetJointAbsPosition(item, &pos, 0);
    room_number = GetCinematicRoom(pos.x, pos.y, pos.z);

    if (room_number != -1 && item->room_number != room_number)
        ItemNewRoom(item_num, room_number);

    AnimateLara(item);
}
