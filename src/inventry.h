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

extern sint32 Inventory_Mode;

#ifdef __cplusplus
}
#endif

#endif
