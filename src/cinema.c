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

sint32 StartCinematic(int level_number)
{
    sint32 nframes, cine_over;
    int saved_sound_active;

    title_loaded = 0;
    CineLevelID = level_number;
    S_FadeToBlack();
    if (!InitialiseLevel(level_number, IL_CUTSCENE))
        return END_ACTION;

    InitCinematicRooms();
    InitialisePlayer1(lara.item_number);
    camera.target_angle = CineTargetAngle;
    saved_sound_active = sound_active;
    sound_active = 0;
    cine_frame = 0;
    S_ClearScreen();
    if (!StartSyncedAudio(cutscene_track))
        return 1;
    S_CDVolume(255);
    CineCurrentFrame = 0;

    do {
        DrawPhaseCinematic();
        nframes = CineCurrentFrame - cine_frame * 2 + CINE_ADJUST + 2;
        if (nframes < 2)
            nframes = 2;
        cine_over = DoCinematic(nframes);
    } while (!cine_over);

    if (Option_Music_Volume)
        S_CDVolume(Option_Music_Volume * 25 + 5);
    else
        S_CDVolume(0);
    S_CDStop();
    sound_active = saved_sound_active;
    S_SoundStopAllSamples();
    level_complete = 1;
    return cine_over;
}

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

sint32 DoCinematic(sint32 nframes)
{
    sint16 item_num, nex;
    static int framecount = 0;

    framecount += nframes * CineTickRate;
    for (; framecount >= 0; framecount -= 65536) {
        if (S_UpdateInput())
            return 3;
        if (input & IN_ACTION)
            return 1;
        if (input & IN_OPTION)
            return 2;

        number_dynamics = 0;
        item_num = next_item_active;
        while (item_num != NO_ITEM) {
            nex = items[item_num].next_active;
            if (objects[items[item_num].object_number].control)
                (*objects[items[item_num].object_number].control)(item_num);
            item_num = nex;
        }

        item_num = next_fx_active;
        while (item_num != NO_ITEM) {
            nex = effects[item_num].next_active;
            if (objects[effects[item_num].object_number].control)
                (*objects[effects[item_num].object_number].control)(item_num);
            item_num = nex;
        }

        HairControl(1);
        CalculateCinematicCamera();
        cine_frame++;
        if (cine_frame >= num_cine_frames)
            return 1;
    }
    CineCurrentFrame = (S_CDGetLoc() * 4) / 5;
    return 0;
}

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

void ControlCinematicPlayer(sint16 item_num)
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

    if (item->dynamic_light && item->status != INVISIBLE) {
        pos.x = 0;
        pos.y = 0;
        pos.z = 0;
        GetJointAbsPosition(item, &pos, 0);
        AddDynamicLight(pos.x, pos.y, pos.z, 12, 11);
    }
    AnimateItem(item);
}

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

void InGameCinematicCamera(void)
{
    sint32 c, s;
    sint32 tx, ty, tz;
    sint32 cx, cy, cz;
    sint16 roll, fov;
    sint16 *ptr;

    cine_frame++;
    if (cine_frame >= num_cine_frames)
        cine_frame = num_cine_frames - 1;

    ptr = cine + (int)(8 * cine_frame);
    tx = (sint32)*(ptr + 0);
    ty = (sint32)*(ptr + 1);
    tz = (sint32)*(ptr + 2);
    cx = (sint32)*(ptr + 3);
    cy = (sint32)*(ptr + 4);
    cz = (sint32)*(ptr + 5);
    fov = *(ptr + 6);
    roll = *(ptr + 7);
    c = phd_cos(cinematic_pos.y_rot);
    s = phd_sin(cinematic_pos.y_rot);
    camera.target.x = cinematic_pos.x_pos + ((tx * c + tz * s) >> W2V_SHIFT);
    camera.target.y = cinematic_pos.y_pos + ty;
    camera.target.z = cinematic_pos.z_pos + ((tz * c - tx * s) >> W2V_SHIFT);
    camera.pos.x = cinematic_pos.x_pos + ((cx * c + cz * s) >> W2V_SHIFT);
    camera.pos.y = cinematic_pos.y_pos + cy;
    camera.pos.z = cinematic_pos.z_pos + ((cz * c - cx * s) >> W2V_SHIFT);
    AlterFOV(fov);
    phd_LookAt(camera.pos.x, camera.pos.y, camera.pos.z, camera.target.x, camera.target.y, camera.target.z, roll);
    GetFloor(camera.pos.x, camera.pos.y, camera.pos.z, &camera.pos.room_number);

    if (camera.mike_at_lara) {
        camera.actual_angle = lara_item->pos.y_rot + lara.head_y_rot + lara.torso_y_rot;
        camera.mike_pos.x = lara_item->pos.x_pos;
        camera.mike_pos.y = lara_item->pos.y_pos;
        camera.mike_pos.z = lara_item->pos.z_pos;
    } else {
        camera.actual_angle = phd_atan(camera.target.z - camera.pos.z, camera.target.x - camera.pos.x);
        camera.mike_pos.x = camera.pos.x + ((PhdPersp * phd_sin(camera.actual_angle)) >> W2V_SHIFT);
        camera.mike_pos.z = camera.pos.z + ((PhdPersp * phd_cos(camera.actual_angle)) >> W2V_SHIFT);
        camera.mike_pos.y = camera.pos.y;
    }
}
