#include "game.h"
#include "box.h"

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

#define SQUARE(A) ((A) * (A))
#define MIN_SQUARE SQUARE(WALL_L / 3)

void ShiftCamera(sint32 *x, sint32 *y, sint32 *h, sint32 target_x, sint32 target_y, sint32 target_h,
    sint32 left, sint32 top, sint32 right, sint32 bottom)
{
    sint32 shift, TL_square, BL_square, TR_square;

    TL_square = SQUARE(target_x - left) + SQUARE(target_y - top);
    BL_square = SQUARE(target_x - left) + SQUARE(target_y - bottom);
    TR_square = SQUARE(target_x - right) + SQUARE(target_y - top);

    if (camera.target_square < TL_square)
    {
        *x = left;
        shift = camera.target_square - SQUARE(target_x - left);
        if (shift < 0)
            return;
        shift = phd_sqrt(shift);
        *y = target_y + ((top < bottom) ? -shift : shift);
    }
    else if (TL_square > MIN_SQUARE)
    {
        *x = left;
        *y = top;
    }
    else if (camera.target_square < BL_square)
    {
        *x = left;
        shift = camera.target_square - SQUARE(target_x - left);
        if (shift < 0)
            return;
        shift = phd_sqrt(shift);
        *y = target_y + ((top < bottom) ? shift : -shift);
    }
    else if ((camera.target_square << 1) < TR_square)
    {
        shift = (camera.target_square << 1) - SQUARE(target_y - top);
        if (shift < 0)
            return;
        shift = phd_sqrt(shift);
        *x = target_x + ((left < right) ? shift : -shift);
        *y = top;
    }
    else if (BL_square > TR_square)
    {
        *x = left;
        *y = bottom;
    }
    else
    {
        *x = right;
        *y = top;
    }
}

#define MAX_ELEVATION (ONE_DEGREE * 85)

void MoveCamera(GAME_VECTOR *ideal, int speed);
void SmartShift(GAME_VECTOR *ideal, void (*shift)(sint32 *x, sint32 *y, sint32 *h, sint32 target_x, sint32 target_y, sint32 target_h, sint32 left, sint32 top, sint32 right, sint32 bottom));

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

#define COMBAT_DISTANCE (WALL_L * 5 / 2)

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
