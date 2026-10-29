#ifndef LARA_H
#define LARA_H

#include "types.h"

#define ONE_DEGREE 182

enum lara_gun_states {
    LG_ARMLESS,
    LG_HANDSBUSY,
    LG_DRAW,
    LG_UNDRAW,
    LG_READY,
    LG_SPECIAL
};

typedef struct lara_info {
    sint16 item_number;
    sint16 gun_status;
} LARA_INFO;

extern LARA_INFO lara;

#endif
