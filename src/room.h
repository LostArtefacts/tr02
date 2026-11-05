#ifndef ROOM_H
#define ROOM_H

#include "types.h"

#define WALL_L 1024
#define NO_HEIGHT (-0x7f00)
#define WALL_SHIFT 10
#define STEP_L (WALL_L / 4)

#define SFX_ALWAYS 2

typedef struct {
    uint16 index;
    sint16 box;
    unsigned char pit_room;
    signed char floor;
    unsigned char sky_room;
    signed char ceiling;
} FLOOR_INFO;

typedef struct room_info {
    sint16 *data;
    sint16 *door;
    FLOOR_INFO *floor;
    void *light;
    void *mesh;
    sint32 x, y, z;
    sint32 minfloor, maxceiling;
    sint16 x_size, y_size;
    sint16 ambient, ambient2;
    sint16 lighting;
    sint16 num_lights;
    sint16 num_meshes;
    sint16 left, right, top, bottom;
    sint16 bound_active;
    sint16 test_left, test_right, test_top, test_bottom;
    sint16 item_number;
    sint16 fx_number;
    sint16 flipped_room;
    uint16 flags;
} ROOM_INFO;

extern ROOM_INFO *room;

#endif
