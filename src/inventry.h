#ifndef INVENTRY_H
#define INVENTRY_H

#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"

enum inv_modes {
    INV_GAME_MODE = 0,
    INV_TITLE_MODE,
    INV_KEYS_MODE,
    INV_SAVE_MODE,
    INV_LOAD_MODE,
    INV_DEATH_MODE
};

typedef uint16 SG_COL;

enum inventory_colours {
    C_BLACK = 0,
    C_GREY,
    C_WHITE,
    C_RED,
    C_ORANGE,
    C_YELLOW,
    C_GREEN1,
    C_GREEN2,
    C_GREEN3,
    C_GREEN4,
    C_GREEN5,
    C_GREEN6,
    C_DARKGREEN,
    C_GREEN,
    C_CYAN,
    C_BLUE,
    C_MAGENTA,
    C_NUMBER_COLOURS
};

extern sint32 Inventory_Mode;
extern SG_COL inv_colours[];

#ifdef __cplusplus
}
#endif

#endif
