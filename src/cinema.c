#include "game.h"

void SetCutsceneTrack(int track)
{
    cutscene_track = track;
}

extern sint16 number_rooms;

#define CINE_ADJUST 6
#define END_ACTION 2

extern sint16 CineLevelID;
extern sint16 CineTargetAngle;
extern int CineCurrentFrame;
extern sint16 cine_frame;
extern sint16 Option_Music_Volume;

int InitialiseLevel(int level_number, int type);
void InitCinematicRooms(void);
void InitialisePlayer1(sint16 item_num);
int StartSyncedAudio(int num);
void S_CDVolume(int volume);
void S_CDStop(void);
void DrawPhaseCinematic(void);
void S_FadeToBlack(void);
void S_ClearScreen(void);
sint32 DoCinematic(sint32 nframes);

#define OUTSIDE 8

void InitCinematicRooms(void)
{
    int i;

    for (i = 0; i < number_rooms; i++) {
        if (room[i].flipped_room >= 0)
            room[room[i].flipped_room].bound_active = 1;
        room[i].flags |= OUTSIDE;
    }

    number_draw_rooms = 0;
    for (i = 0; i < number_rooms; i++) {
        if (!room[i].bound_active)
            draw_rooms[number_draw_rooms++] = i;
    }
}

int GetCinematicRoom(sint32 x, sint32 y, sint32 z);
void phd_LookAt(sint32 xsrc, sint32 ysrc, sint32 zsrc, sint32 xtar, sint32 ytar, sint32 ztar, sint16 roll);

extern int CineTickRate;
extern int number_dynamics;
extern sint16 next_item_active;

int S_UpdateInput(void);
int S_CDGetLoc(void);
void HairControl(int in_cutscene);

void CalculateCinematicCamera(void);

int GetCinematicRoom(sint32 x, sint32 y, sint32 z)
{
    int i;

    for (i = 0; i < number_rooms; i++) {
        if ((x >= room[i].x + WALL_L && x < room[i].x + (room[i].y_size - 1) * WALL_L)
            && (y >= room[i].maxceiling && y <= room[i].minfloor)
            && (z >= room[i].z + WALL_L && z < room[i].z + (room[i].x_size - 1) * WALL_L)) {
            return i;
        }
    }
    return -1;
}

void GetJointAbsPosition(ITEM_INFO *item, PHD_VECTOR *vec, int joint);
void AnimateLara(ITEM_INFO *item);

void AddDynamicLight(sint32 x, sint32 y, sint32 z, sint32 intensity, sint32 falloff);

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

void InitialiseGenPlayer(sint16 item_num)
{
    AddActiveItem(item_num);
    items[item_num].pos.y_rot = 0;
    items[item_num].dynamic_light = 0;
}

extern PHD_3DPOS cinematic_pos;
extern sint32 PhdPersp;

