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

#define GROUND_SHIFT (STEP_L)

extern int IsChunkyCamera;
extern sint32 PhdPersp;

int LOS(GAME_VECTOR *start, GAME_VECTOR *target);
void phd_LookAt(sint32 xsrc, sint32 ysrc, sint32 zsrc, sint32 xtar, sint32 ytar, sint32 ztar, sint16 roll);

void MoveCamera(GAME_VECTOR *ideal, int speed)
{
    sint32 height, ceiling, shake;
    FLOOR_INFO *floor;

    camera.pos.x += (ideal->x - camera.pos.x) / speed;
    camera.pos.z += (ideal->z - camera.pos.z) / speed;
    camera.pos.y += (ideal->y - camera.pos.y) / speed;
    camera.pos.room_number = ideal->room_number;

    IsChunkyCamera = 0;

    floor = GetFloor(camera.pos.x, camera.pos.y, camera.pos.z, &camera.pos.room_number);
    height = GetHeight(floor, camera.pos.x, camera.pos.y, camera.pos.z) - GROUND_SHIFT;

    if (camera.pos.y >= height && ideal->y >= height)
    {
        LOS(&camera.target, &camera.pos);
        floor = GetFloor(camera.pos.x, camera.pos.y, camera.pos.z, &camera.pos.room_number);
        height = GetHeight(floor, camera.pos.x, camera.pos.y, camera.pos.z) - GROUND_SHIFT;
    }

    ceiling = GetCeiling(floor, camera.pos.x, camera.pos.y, camera.pos.z) + GROUND_SHIFT;
    if (height < ceiling)
        height = ceiling = (height + ceiling) >> 1;

    if (camera.bounce)
    {
        if (camera.bounce > 0)
        {
            camera.pos.y += camera.bounce;
            camera.target.y += camera.bounce;
            camera.bounce = 0;
        }
        else
        {
            shake = (GetRandomControl() - 0x4000) * camera.bounce / 0x7fff;
            camera.pos.x += shake;
            camera.target.y += shake;
            shake = (GetRandomControl() - 0x4000) * camera.bounce / 0x7fff;
            camera.pos.y += shake;
            camera.target.y += shake;
            shake = (GetRandomControl() - 0x4000) * camera.bounce / 0x7fff;
            camera.pos.z += shake;
            camera.target.z += shake;
            camera.bounce += 5;
        }
    }

    if (camera.pos.y > height)
        camera.shift = height - camera.pos.y;
    else if (camera.pos.y < ceiling)
        camera.shift = ceiling - camera.pos.y;
    else
        camera.shift = 0;

    GetFloor(camera.pos.x, camera.pos.y + camera.shift, camera.pos.z, &camera.pos.room_number);

    phd_LookAt(camera.pos.x, camera.pos.y + camera.shift, camera.pos.z, camera.target.x, camera.target.y, camera.target.z, 0);

    if (camera.mike_at_lara)
    {
        camera.actual_angle = lara_item->pos.y_rot + lara.head_y_rot + lara.torso_y_rot;
        camera.mike_pos.x = lara_item->pos.x_pos;
        camera.mike_pos.y = lara_item->pos.y_pos;
        camera.mike_pos.z = lara_item->pos.z_pos;
    }
    else
    {
        camera.actual_angle = phd_atan(camera.target.z - camera.pos.z, camera.target.x - camera.pos.x);
        camera.mike_pos.x = camera.pos.x + ((PhdPersp * phd_sin(camera.actual_angle)) >> W2V_SHIFT);
        camera.mike_pos.z = camera.pos.z + ((PhdPersp * phd_cos(camera.actual_angle)) >> W2V_SHIFT);
        camera.mike_pos.y = camera.pos.y;
    }
}

void ClipCamera(sint32 *x, sint32 *y, sint32 *h, sint32 target_x, sint32 target_y, sint32 target_h,
    sint32 left, sint32 top, sint32 right, sint32 bottom)
{
    if ((right > left) ^ (target_x < left))
    {
        *y = target_y + (*y - target_y) * (left - target_x) / (*x - target_x);
        *h = target_h + (*h - target_h) * (left - target_x) / (*x - target_x);
        *x = left;
    }
    if ((bottom > top && target_y > top && *y < top) || (bottom < top && target_y < top && *y > top))
    {
        *x = target_x + (*x - target_x) * (top - target_y) / (*y - target_y);
        *h = target_h + (*h - target_h) * (top - target_y) / (*y - target_y);
        *y = top;
    }
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

FLOOR_INFO *GoodPosition(sint32 x, sint32 y, sint32 z, sint16 room_number)
{
    FLOOR_INFO *floor;

    floor = GetFloor(x, y, z, &room_number);
    if (y <= GetHeight(floor, x, y, z) && y >= GetCeiling(floor, x, y, z))
        return floor;
    return NULL;
}

#define MAX_ELEVATION (ONE_DEGREE * 85)

void MoveCamera(GAME_VECTOR *ideal, int speed);
void SmartShift(GAME_VECTOR *ideal, void (*shift)(sint32 *x, sint32 *y, sint32 *h, sint32 target_x, sint32 target_y, sint32 target_h, sint32 left, sint32 top, sint32 right, sint32 bottom));

void SmartShift(GAME_VECTOR *ideal, void (*shift)(sint32 *x, sint32 *y, sint32 *h, sint32 target_x, sint32 target_y, sint32 target_h, sint32 left, sint32 top, sint32 right, sint32 bottom))
{
    int noclip, preferA;
    ROOM_INFO *r;
    sint16 item_box, camera_box;
    BOX_INFO *box;
    int x_floor, y_floor;
    sint32 left, right, top, bottom, test, edge;
    FLOOR_INFO *good_left, *good_right, *good_top, *good_bottom;
    GAME_VECTOR A, B;

    LOS(&camera.target, ideal);

    r = &room[camera.target.room_number];
    x_floor = (camera.target.z - r->z) >> WALL_SHIFT;
    y_floor = (camera.target.x - r->x) >> WALL_SHIFT;
    item_box = r->floor[x_floor + y_floor * r->x_size].box;

    box = &boxes[item_box];

    left = (sint32)box->left << WALL_SHIFT;
    right = ((sint32)box->right << WALL_SHIFT) - 1;
    top = (sint32)box->top << WALL_SHIFT;
    bottom = ((sint32)box->bottom << WALL_SHIFT) - 1;

    r = &room[ideal->room_number];
    x_floor = (ideal->z - r->z) >> WALL_SHIFT;
    y_floor = (ideal->x - r->x) >> WALL_SHIFT;
    camera_box = r->floor[x_floor + y_floor * r->x_size].box;

    if (camera_box != NO_BOX && (ideal->z < left || ideal->z > right || ideal->x < top || ideal->x > bottom))
    {
        box = &boxes[camera_box];
        left = (sint32)box->left << WALL_SHIFT;
        right = ((sint32)box->right << WALL_SHIFT) - 1;
        top = (sint32)box->top << WALL_SHIFT;
        bottom = ((sint32)box->bottom << WALL_SHIFT) - 1;
    }

    test = (ideal->z - WALL_L) | (WALL_L - 1);
    good_left = GoodPosition(ideal->x, ideal->y, test, ideal->room_number);
    if (good_left)
    {
        camera_box = good_left->box;
        edge = (sint32)boxes[camera_box].left << WALL_SHIFT;
        if (camera_box != NO_ITEM && edge < left)
            left = edge;
    }
    else
        left = test;

    test = (ideal->z + WALL_L) & ~(WALL_L - 1);
    good_right = GoodPosition(ideal->x, ideal->y, test, ideal->room_number);
    if (good_right)
    {
        camera_box = good_right->box;
        edge = ((sint32)boxes[camera_box].right << WALL_SHIFT) - 1;
        if (camera_box != NO_ITEM && edge > right)
            right = edge;
    }
    else
        right = test;

    test = (ideal->x - WALL_L) | (WALL_L - 1);
    good_top = GoodPosition(test, ideal->y, ideal->z, ideal->room_number);
    if (good_top)
    {
        camera_box = good_top->box;
        edge = (sint32)boxes[camera_box].top << WALL_SHIFT;
        if (camera_box != NO_ITEM && edge < top)
            top = edge;
    }
    else
        top = test;

    test = (ideal->x + WALL_L) & ~(WALL_L - 1);
    good_bottom = GoodPosition(test, ideal->y, ideal->z, ideal->room_number);
    if (good_bottom)
    {
        camera_box = good_bottom->box;
        edge = ((sint32)boxes[camera_box].bottom << WALL_SHIFT) - 1;
        if (camera_box != NO_ITEM && edge > bottom)
            bottom = edge;
    }
    else
        bottom = test;

    left += STEP_L;
    right -= STEP_L;
    top += STEP_L;
    bottom -= STEP_L;

    A.x = B.x = ideal->x;
    A.y = B.y = ideal->y;
    A.z = B.z = ideal->z;
    A.room_number = B.room_number = ideal->room_number;

    noclip = 1;
    if (ABS(ideal->z - camera.target.z) > ABS(ideal->x - camera.target.x))
    {
        if (ideal->z < left && !good_left)
        {
            noclip = 0;
            preferA = (camera.pos.x < camera.target.x);
            (*shift)(&A.z, &A.x, &A.y, camera.target.z, camera.target.x, camera.target.y, left, top, right, bottom);
            (*shift)(&B.z, &B.x, &B.y, camera.target.z, camera.target.x, camera.target.y, left, bottom, right, top);
        }
        else if (ideal->z > right && !good_right)
        {
            noclip = 0;
            preferA = (camera.pos.x < camera.target.x);
            (*shift)(&A.z, &A.x, &A.y, camera.target.z, camera.target.x, camera.target.y, right, top, left, bottom);
            (*shift)(&B.z, &B.x, &B.y, camera.target.z, camera.target.x, camera.target.y, right, bottom, left, top);
        }

        if (noclip)
        {
            if (ideal->x < top && !good_top)
            {
                noclip = 0;
                preferA = (ideal->z < camera.target.z);
                (*shift)(&A.x, &A.z, &A.y, camera.target.x, camera.target.z, camera.target.y, top, left, bottom, right);
                (*shift)(&B.x, &B.z, &B.y, camera.target.x, camera.target.z, camera.target.y, top, right, bottom, left);
            }
            else if (ideal->x > bottom && !good_bottom)
            {
                noclip = 0;
                preferA = (ideal->z < camera.target.z);
                (*shift)(&A.x, &A.z, &A.y, camera.target.x, camera.target.z, camera.target.y, bottom, left, top, right);
                (*shift)(&B.x, &B.z, &B.y, camera.target.x, camera.target.z, camera.target.y, bottom, right, top, left);
            }
        }
    }
    else
    {
        if (ideal->x < top && !good_top)
        {
            noclip = 0;
            preferA = (camera.pos.z < camera.target.z);
            (*shift)(&A.x, &A.z, &A.y, camera.target.x, camera.target.z, camera.target.y, top, left, bottom, right);
            (*shift)(&B.x, &B.z, &B.y, camera.target.x, camera.target.z, camera.target.y, top, right, bottom, left);
        }
        else if (ideal->x > bottom && !good_bottom)
        {
            noclip = 0;
            preferA = (camera.pos.z < camera.target.z);
            (*shift)(&A.x, &A.z, &A.y, camera.target.x, camera.target.z, camera.target.y, bottom, left, top, right);
            (*shift)(&B.x, &B.z, &B.y, camera.target.x, camera.target.z, camera.target.y, bottom, right, top, left);
        }

        if (noclip)
        {
            if (ideal->z < left && !good_left)
            {
                noclip = 0;
                preferA = (ideal->x < camera.target.x);
                (*shift)(&A.z, &A.x, &A.y, camera.target.z, camera.target.x, camera.target.y, left, top, right, bottom);
                (*shift)(&B.z, &B.x, &B.y, camera.target.z, camera.target.x, camera.target.y, left, bottom, right, top);
            }
            else if (ideal->z > right && !good_right)
            {
                noclip = 0;
                preferA = (ideal->x < camera.target.x);
                (*shift)(&A.z, &A.x, &A.y, camera.target.z, camera.target.x, camera.target.y, right, top, left, bottom);
                (*shift)(&B.z, &B.x, &B.y, camera.target.z, camera.target.x, camera.target.y, right, bottom, left, top);
            }
        }
    }

    if (noclip)
        return;

    if (preferA && !LOS(&camera.target, &A))
        preferA = 0;
    else if (!preferA && !LOS(&camera.target, &B))
        preferA = 1;

    if (preferA)
    {
        ideal->x = A.x;
        ideal->y = A.y;
        ideal->z = A.z;
    }
    else
    {
        ideal->x = B.x;
        ideal->y = B.y;
        ideal->z = B.z;
    }

    GetFloor(ideal->x, ideal->y, ideal->z, &ideal->room_number);
}

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

sint32 ShiftClamp(GAME_VECTOR *pos, sint32 clamp)
{
    sint32 height, ceiling, shift, x, y, z;
    sint32 left, right, top, bottom;
    FLOOR_INFO *floor;
    BOX_INFO *box;

    x = pos->x;
    y = pos->y;
    z = pos->z;

    floor = GetFloor(x, y, z, &pos->room_number);

    box = &boxes[floor->box];
    left = ((sint32)box->left << WALL_SHIFT) + clamp;
    right = ((sint32)box->right << WALL_SHIFT) - clamp - 1;
    if (z < left && !GoodPosition(x, y, z - clamp, pos->room_number))
        pos->z = left;
    else if (z > right && !GoodPosition(x, y, z + clamp, pos->room_number))
        pos->z = right;

    top = ((sint32)box->top << WALL_SHIFT) + clamp;
    bottom = ((sint32)box->bottom << WALL_SHIFT) - clamp - 1;
    if (x < top && !GoodPosition(x - clamp, y, z, pos->room_number))
        pos->x = top;
    else if (x > bottom && !GoodPosition(x + clamp, y, z, pos->room_number))
        pos->x = bottom;

    height = GetHeight(floor, x, y, z) - clamp;
    ceiling = GetCeiling(floor, x, y, z) + clamp;

    if (height < ceiling)
        height = ceiling = (height + ceiling) >> 1;

    if (y > height)
        shift = height - y;
    else if (pos->y < ceiling)
        shift = ceiling - y;
    else
        shift = 0;

    return shift;
}

#define COMBAT_DISTANCE (WALL_L * 5 / 2)

void CombatCamera(ITEM_INFO *item)
{
    sint32 distance, water_height;
    GAME_VECTOR ideal;

    camera.target.z = item->pos.z_pos;
    camera.target.x = item->pos.x_pos;

    if (lara.target)
    {
        camera.target_angle = item->pos.y_rot + lara.target_angles[0];
        camera.target_elevation = item->pos.x_rot + lara.target_angles[1];
    }
    else
    {
        camera.target_angle = item->pos.y_rot + lara.head_y_rot + lara.torso_y_rot;
        camera.target_elevation = item->pos.x_rot + lara.head_x_rot + lara.torso_x_rot;
    }
    camera.target_distance = COMBAT_DISTANCE;

    distance = camera.target_distance * phd_cos(camera.target_elevation) >> W2V_SHIFT;

    ideal.x = camera.target.x - (distance * phd_sin(camera.target_angle) >> W2V_SHIFT);
    ideal.y = camera.target.y + (camera.target_distance * phd_sin(camera.target_elevation) >> W2V_SHIFT);
    ideal.z = camera.target.z - (distance * phd_cos(camera.target_angle) >> W2V_SHIFT);
    ideal.room_number = camera.pos.room_number;

    if (lara.water_status == LARA_UNDERWATER)
    {
        water_height = lara_item->pos.y_pos + lara.water_surface_dist;
        if (camera.target.y > water_height && water_height > ideal.y)
        {
            ideal.z = camera.target.z + (ideal.z - camera.target.z) * (water_height - camera.target.y) / (ideal.y - camera.target.y);
            ideal.x = camera.target.x + (ideal.x - camera.target.x) * (water_height - camera.target.y) / (ideal.y - camera.target.y);
            ideal.y = water_height;
        }
    }

    SmartShift(&ideal, ShiftCamera);

    MoveCamera(&ideal, camera.speed);
}

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

#define UNDERWATER 1
#define CHASE_SPEED 10
#define LOOK_SPEED 4
#define COMBAT_SPEED 8
#define CAM_MAX_HEAD_ROTATION (50 * ONE_DEGREE)
#define CAM_MAX_HEAD_TILT (85 * ONE_DEGREE)
#define CAM_MIN_HEAD_TILT (-85 * ONE_DEGREE)
#define CAM_HEAD_TURN (4 * ONE_DEGREE)

extern sint16 Option_Music_Volume;
void S_CDVolume(int volume);
void InGameCinematicCamera(void);
void LookCamera(ITEM_INFO *item);

void CalculateCamera(void)
{
    FLOOR_INFO *floor;
    ITEM_INFO *item;
    sint32 shift, fixed_camera, y;
    OBJECT_INFO *object;
    sint16 *bounds, angle, tilt, change;

    if (room[camera.pos.room_number].flags & UNDERWATER)
    {
        SoundEffect(60, NULL, SFX_ALWAYS);
        if (!camera.underwater)
        {
            S_CDVolume(0);
            camera.underwater = 1;
        }
    }
    else
    {
        if (camera.underwater)
        {
            if (Option_Music_Volume)
                S_CDVolume(Option_Music_Volume * 25 + 5);
            camera.underwater = 0;
        }
    }

    if (camera.type == CINEMATIC_CAMERA)
    {
        InGameCinematicCamera();
        return;
    }

    if (camera.flags != NO_CHUNKY)
        IsChunkyCamera = 1;

    fixed_camera = (camera.item && (camera.type == FIXED_CAMERA || camera.type == HEAVY_CAMERA));
    item = fixed_camera ? camera.item : lara_item;

    object = &objects[item->object_number];
    bounds = GetBoundsAccurate(item);

    y = item->pos.y_pos;
    if (!fixed_camera)
        y += bounds[3] + ((bounds[2] - bounds[3]) * 3 >> 2);
    else
        y += (bounds[2] + bounds[3]) / 2;

    if (camera.item && !fixed_camera)
    {
        shift = phd_sqrt(SQUARE(camera.item->pos.z_pos - item->pos.z_pos) + SQUARE(camera.item->pos.x_pos - item->pos.x_pos));
        angle = phd_atan(camera.item->pos.z_pos - item->pos.z_pos, camera.item->pos.x_pos - item->pos.x_pos) - item->pos.y_rot;
        bounds = GetBoundsAccurate(camera.item);
        tilt = phd_atan(shift, y - (camera.item->pos.y_pos + (bounds[2] + bounds[3]) / 2));
        angle >>= 1;
        tilt >>= 1;
        if (angle > -CAM_MAX_HEAD_ROTATION && angle < CAM_MAX_HEAD_ROTATION && tilt > CAM_MIN_HEAD_TILT && tilt < CAM_MAX_HEAD_TILT)
        {
            change = angle - lara.head_y_rot;
            if (change > CAM_HEAD_TURN)
                lara.head_y_rot += CAM_HEAD_TURN;
            else if (change < -CAM_HEAD_TURN)
                lara.head_y_rot -= CAM_HEAD_TURN;
            else
                lara.head_y_rot += change;
            lara.torso_y_rot = lara.head_y_rot;

            change = tilt - lara.head_x_rot;
            if (change > CAM_HEAD_TURN)
                lara.head_x_rot += CAM_HEAD_TURN;
            else if (change < -CAM_HEAD_TURN)
                lara.head_x_rot -= CAM_HEAD_TURN;
            else
                lara.head_x_rot += change;
            lara.torso_x_rot = lara.head_x_rot;

            camera.type = LOOK_CAMERA;

            camera.item->looked_at = 1;
        }
    }

    if (camera.type == LOOK_CAMERA || camera.type == COMBAT_CAMERA)
    {
        y -= STEP_L;
        camera.target.room_number = item->room_number;

        if (camera.fixed_camera)
        {
            camera.target.y = y;
            camera.speed = 1;
        }
        else
        {
            camera.target.y += ((y - camera.target.y) >> 2);
            camera.speed = (camera.type == LOOK_CAMERA) ? LOOK_SPEED : COMBAT_SPEED;
        }

        camera.fixed_camera = 0;

        if (camera.type == LOOK_CAMERA)
            LookCamera(item);
        else
            CombatCamera(item);
    }
    else
    {
        camera.target.x = item->pos.x_pos;
        camera.target.z = item->pos.z_pos;

        if (camera.flags == FOLLOW_CENTRE)
        {
            shift = (bounds[4] + bounds[5]) / 2;
            camera.target.z += phd_cos(item->pos.y_rot) * shift >> W2V_SHIFT;
            camera.target.x += phd_sin(item->pos.y_rot) * shift >> W2V_SHIFT;
        }
        camera.target.room_number = item->room_number;

        if (camera.fixed_camera ^ fixed_camera)
        {
            camera.target.y = y;
            camera.fixed_camera = 1;
            camera.speed = 1;
        }
        else
        {
            camera.target.y += (y - camera.target.y) / 4;
            camera.fixed_camera = 0;
        }

        floor = GetFloor(camera.target.x, camera.target.y, camera.target.z, &camera.target.room_number);

        if (camera.target.y > GetHeight(floor, camera.target.x, camera.target.y, camera.target.z))
            IsChunkyCamera = 0;

        if (camera.type == CHASE_CAMERA || camera.flags == CHASE_OBJECT)
            ChaseCamera(item);
        else
            FixedCamera();
    }

    camera.last = camera.number;
    camera.fixed_camera = fixed_camera;

    if (camera.type != HEAVY_CAMERA || camera.timer == -1)
    {
        camera.type = CHASE_CAMERA;
        camera.speed = CHASE_SPEED;
        camera.number = NO_CAMERA;
        camera.last_item = camera.item;
        camera.item = NULL;
        camera.target_angle = camera.target_elevation = 0;
        camera.target_distance = WALL_L * 3 / 2;
        camera.flags = 0;
    }

    IsChunkyCamera = 0;
}
