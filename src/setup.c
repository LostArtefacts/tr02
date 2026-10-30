#include "game.h"
#include "savegame.h"

#define NUMBER_OBJECTS 265
#define DEFAULT_RADIUS 10
#define DONT_TARGET (-16384)
#define NO_ROOM 0xff

void InitialiseLevelFlags(void)
{
    savegame.secrets = 0;
    savegame.timer = 0;
    savegame.kills = 0;
    savegame.distance_travelled = 0;
    savegame.ammo_used = savegame.ammo_hit = 0;
    savegame.health_used = 0;
}
