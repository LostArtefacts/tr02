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

#endif
