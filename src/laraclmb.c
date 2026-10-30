#include "game.h"

#define CAM_A_CLIMBL (-30 * ONE_DEGREE)
#define CAM_E_CLIMBL (-15 * ONE_DEGREE)
#define CAM_A_CLIMBR (30 * ONE_DEGREE)
#define CAM_E_CLIMBR (-15 * ONE_DEGREE)
#define CAM_E_CLIMBU (30 * ONE_DEGREE)
#define CAM_E_CLIMBD (-45 * ONE_DEGREE)
#define CLIMB_HITE (STEP_L * 2)
#define CLIMB_WIDTHL 80
#define CLIMB_WIDTHR 120

int LaraCheckForLetGo(ITEM_INFO *item, COLL_INFO *coll);
int LaraTestClimbPos(ITEM_INFO *item, int front, int right, int origin, int height, int *shift);
void LaraDoClimbLeftRight(ITEM_INFO *item, COLL_INFO *coll, int result, int shift);

void lara_as_climbleft(ITEM_INFO *item, COLL_INFO *coll)
{
    coll->enable_spaz = 0;
    coll->enable_baddie_push = 0;
    camera.target_angle = CAM_A_CLIMBL;
    camera.target_elevation = CAM_E_CLIMBL;
    if (!(input & IN_LEFT) && !(input & IN_STEPL))
        item->goal_anim_state = AS_CLIMBSTNC;
}

void lara_as_climbright(ITEM_INFO *item, COLL_INFO *coll)
{
    coll->enable_spaz = 0;
    coll->enable_baddie_push = 0;
    camera.target_angle = CAM_A_CLIMBR;
    camera.target_elevation = CAM_E_CLIMBR;
    if (!(input & IN_RIGHT) && !(input & IN_STEPR))
        item->goal_anim_state = AS_CLIMBSTNC;
}
