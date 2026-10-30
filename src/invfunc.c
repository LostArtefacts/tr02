#include "game.h"
#include "invdata.h"

#define ROTATE_DURATION 24
#define CAMERA_YOFFSET -0x60

typedef struct imotion_info {
    sint16 count;
    sint16 status;
    sint16 status_target;
    sint16 radius_target;
    sint16 radius_rate;
    sint16 camera_ytarget;
    sint16 camera_yrate;
    sint16 camera_pitch_target;
    sint16 camera_pitch_rate;
    sint16 rotate_target;
    sint16 rotate_rate;
    PHD_ANGLE item_ptxrot_target;
    PHD_ANGLE item_ptxrot_rate;
    PHD_ANGLE item_xrot_target;
    PHD_ANGLE item_xrot_rate;
    sint32 item_ytrans_target;
    sint32 item_ytrans_rate;
    sint32 item_ztrans_target;
    sint32 item_ztrans_rate;
    sint32 misc;
} IMOTION_INFO;

typedef struct ring_info {
    INVENTORY_ITEM **list;
    sint16 type;
    sint16 radius;
    sint16 camera_pitch;
    sint16 rotating;
    sint16 rot_count;
    sint16 current_object;
    sint16 target_object;
    sint16 number_of_objects;
    sint16 angle_adder;
    sint16 rot_adder;
    sint16 rot_adderL;
    sint16 rot_adderR;
    PHD_3DPOS ringpos;
    PHD_3DPOS camera;
    PHD_VECTOR light;
    IMOTION_INFO *imo;
} RING_INFO;

extern sint16 inv_main_objects, inv_main_current;
extern sint16 inv_keys_objects, inv_keys_current;

void Inv_RemoveAllItems(void)
{
    inv_main_objects = 0;
    inv_main_current = 0;
    inv_keys_objects = 0;
    inv_keys_current = 0;
}

void Inv_RingGetView(RING_INFO *ring, PHD_3DPOS *viewer)
{
    PHD_ANGLE angles[2];

    phd_GetVectorAngles(0 - ring->camera.x_pos, CAMERA_YOFFSET - ring->camera.y_pos, ring->radius - ring->camera.z_pos, angles);
    viewer->x_pos = ring->camera.x_pos;
    viewer->y_pos = ring->camera.y_pos;
    viewer->z_pos = ring->camera.z_pos;
    viewer->x_rot = angles[1] + ring->camera_pitch;
    viewer->y_rot = angles[0];
    viewer->z_rot = 0;
}

extern sint32 ls_divider;

void Inv_RingLight(RING_INFO *ring)
{
    PHD_ANGLE angles[2];

    ls_divider = 0x6000;
    phd_GetVectorAngles(ring->light.x, ring->light.y, ring->light.z, angles);
    phd_RotateLight(angles[1], angles[0]);
}
