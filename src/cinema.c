#include "game.h"

void SetCutsceneTrack(int track)
{
    cutscene_track = track;
}

extern sint16 number_rooms;

void GetJointAbsPosition(ITEM_INFO *item, PHD_VECTOR *vec, int joint);
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

extern int original_room;

void DrawLara(ITEM_INFO *item);

void InitialisePlayer1(sint16 item_num)
{
    ITEM_INFO *item;
    OBJECT_INFO *obj;

    obj = &objects[LARA];
    obj->draw_routine = DrawLara;
    obj->control = LaraControlCinematic;
    AddActiveItem(item_num);
    item = &items[item_num];
    camera.pos.x = item->pos.x_pos;
    camera.pos.y = item->pos.y_pos;
    camera.pos.z = item->pos.z_pos;
    camera.target_angle = item->pos.y_rot = 0;
    original_room = camera.pos.room_number = item->room_number;
    item->dynamic_light = 0;
    item->goal_anim_state = 0;
    item->current_anim_state = 0;
    item->frame_number = 0;
    item->anim_number = 0;
    lara.hit_direction = -1;
}

