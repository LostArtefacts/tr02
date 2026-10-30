#include "game.h"

#define NO_CAMERA (-1)

void CalculateCamera(void);

void InitialiseCamera(void)
{
    camera.target.x = lara_item->pos.x_pos;
    camera.target.y = camera.shift = lara_item->pos.y_pos - WALL_L;
    camera.target.z = lara_item->pos.z_pos;
    camera.target.room_number = lara_item->room_number;

    camera.pos.x = camera.target.x;
    camera.pos.y = camera.target.y;
    camera.pos.z = camera.target.z - 100;
    camera.pos.room_number = camera.target.room_number;

    camera.target_distance = WALL_L * 3 / 2;
    camera.item = NULL;

    camera.number_frames = 1;
    if (!lara.extra_anim)
        camera.type = CHASE_CAMERA;
    camera.speed = 1;
    camera.flags = 0;

    camera.bounce = 0;

    camera.number = NO_CAMERA;
    camera.fixed_camera = 0;
    AlterFOV(GAME_FOV * ONE_DEGREE);

    CalculateCamera();
}

#define MAX_ELEVATION (ONE_DEGREE * 85)
#define SQUARE(A) ((A) * (A))

void MoveCamera(GAME_VECTOR *ideal, int speed);
void ShiftCamera(GAME_VECTOR *ideal);
void SmartShift(GAME_VECTOR *ideal, void (*shift)(GAME_VECTOR *ideal));

void ChaseCamera(ITEM_INFO *item)
{
    GAME_VECTOR ideal;
    sint32 distance;
    sint16 angle;

    camera.target_elevation += item->pos.x_rot;
    if (camera.target_elevation > MAX_ELEVATION)
        camera.target_elevation = MAX_ELEVATION;
    else if (camera.target_elevation < -MAX_ELEVATION)
        camera.target_elevation = -MAX_ELEVATION;

    distance = camera.target_distance * phd_cos(camera.target_elevation) >> W2V_SHIFT;
    ideal.y = camera.target.y + (camera.target_distance * phd_sin(camera.target_elevation) >> W2V_SHIFT);

    camera.target_square = SQUARE(distance);

    angle = item->pos.y_rot + camera.target_angle;
    ideal.x = camera.target.x - (distance * phd_sin(angle) >> W2V_SHIFT);
    ideal.z = camera.target.z - (distance * phd_cos(angle) >> W2V_SHIFT);
    ideal.room_number = camera.pos.room_number;

    SmartShift(&ideal, ShiftCamera);

    MoveCamera(&ideal, camera.speed);
}

void ShiftClamp(GAME_VECTOR *pos, int clamp);
int LOS(GAME_VECTOR *start, GAME_VECTOR *target);

void FixedCamera(void)
{
    GAME_VECTOR ideal;
    OBJECT_VECTOR *fixed;

    fixed = &camera.fixed[camera.number];
    ideal.x = fixed->x;
    ideal.y = fixed->y;
    ideal.z = fixed->z;
    ideal.room_number = fixed->data;

    if (!LOS(&camera.target, &ideal))
        ShiftClamp(&ideal, STEP_L);

    camera.fixed_camera = 1;

    MoveCamera(&ideal, camera.speed);

    if (camera.timer)
    {
        camera.timer--;
        if (!camera.timer)
            camera.timer = -1;
    }
}
