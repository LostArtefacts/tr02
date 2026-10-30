#include "game.h"

#define DEMOCOUNT_MAX (30 * 60 * 5)

extern int democount;
extern uint32 *demoptr;

sint32 DoDemoSequence(int level_number)
{
    static int demo_level = 0;

    if (level_number < 0 && !gameflow.num_demos)
        return EXIT_TO_TITLE;
    if (level_number < 0)
    {
        if (demo_level >= gameflow.num_demos)
            demo_level = 0;
        level_number = GF_valid_demos[demo_level];
        demo_level++;
    }
    else
        demo_level = level_number;
    return GF_DoLevelSequence(level_number, IL_DEMO);
}

void LoadLaraDemoPos(void)
{
    sint16 room_number;
    FLOOR_INFO *floor;
    ITEM_INFO *item;

    item = lara_item;
    item->pos.x_pos = (sint32)*(demoptr + 0);
    item->pos.y_pos = (sint32)*(demoptr + 1);
    item->pos.z_pos = (sint32)*(demoptr + 2);
    item->pos.x_rot = (sint16)*(demoptr + 3);
    item->pos.y_rot = (sint16)*(demoptr + 4);
    item->pos.z_rot = (sint16)*(demoptr + 5);
    room_number = (sint16)*(demoptr + 6);
    if (item->room_number != room_number)
        ItemNewRoom(lara.item_number, room_number);
    floor = GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
    lara_item->floor = GetHeight(floor, item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);

    lara.last_gun_type = (sint16)*(demoptr + 7);

    democount += 8;
}

void GetDemoInput(void)
{
    if (democount < DEMOCOUNT_MAX)
        input = *(demoptr + democount);
    else
        input = -1;
    if (input != -1)
        democount++;
}
