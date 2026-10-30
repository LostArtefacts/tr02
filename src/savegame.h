#ifndef SAVEGAME_H
#define SAVEGAME_H

#ifdef __cplusplus
extern "C" {
#endif

#include "gameflow.h"

#define MAX_SAVEGAME_BUFFER (6 * 1024 + 128)

typedef struct start_info {
    uint16 pistol_ammo, magnum_ammo, uzi_ammo, shotgun_ammo, m16_ammo, rocket_ammo, harpoon_ammo;
    unsigned char num_medis, num_big_medis, num_scions, num_flares;
    char gun_status, gun_type;
    uint16 available:1;
    uint16 got_pistols:1;
    uint16 got_magnums:1;
    uint16 got_uzis:1;
    uint16 got_shotgun:1;
    uint16 got_m16:1;
    uint16 got_rocket:1;
    uint16 got_harpoon:1;
    uint16 secrets;
    uint32 timer;
    uint32 ammo_used, ammo_hit;
    uint32 distance_travelled;
    uint16 kills;
    unsigned char secrets_found;
    unsigned char health_used;
} START_INFO;

typedef struct savegame_info {
    START_INFO start[MAXIMUM_LEVELS];
    uint32 timer;
    uint32 ammo_used, ammo_hit;
    uint32 distance_travelled;
    uint16 kills;
    unsigned char secrets;
    unsigned char health_used;
    sint16 current_level;
    unsigned char bonus_flag;
    unsigned char num_pickup1, num_pickup2;
    unsigned char num_puzzle1, num_puzzle2, num_puzzle3, num_puzzle4;
    unsigned char num_key1, num_key2, num_key3, num_key4;
    signed char checksum;
    unsigned char mid_level_save:1;
    char buffer[MAX_SAVEGAME_BUFFER];
} SAVEGAME_INFO;

extern SAVEGAME_INFO savegame;
extern int number_cameras;

#ifdef __cplusplus
}
#endif

#endif
