#ifndef SPHERE_H
#define SPHERE_H

#include "items.h"

typedef struct {
    sint32 x;
    sint32 y;
    sint32 z;
    sint32 r;
} SPHERE;

int TestCollision(ITEM_INFO *item, ITEM_INFO *laraitem);
int GetSpheres(ITEM_INFO *item, SPHERE *ptr, int WorldSpace);

#endif
