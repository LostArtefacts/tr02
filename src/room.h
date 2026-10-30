#ifndef ROOM_H
#define ROOM_H

#include "types.h"

#define WALL_L 1024
#define WALL_SHIFT 10
#define STEP_L (WALL_L / 4)

#define SFX_ALWAYS 2

typedef struct {
    uint16 index;
    uint16 fx:4;
    uint16 box:11;
    uint16 stopper:1;
    unsigned char pit_room;
    signed char floor;
    unsigned char sky_room;
    signed char ceiling;
} FLOOR_INFO;

#endif
