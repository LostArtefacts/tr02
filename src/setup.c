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

void BaddyObjects(void);
void TrapObjects(void);
void ObjectObjects(void);

void InitialiseObjects(void)
{
    int i;
    OBJECT_INFO *obj;

    for (i = 0, obj = objects; i < NUMBER_OBJECTS; i++, obj++)
    {
        obj->initialise = NULL;
        obj->collision = NULL;
        obj->control = NULL;
        obj->draw_routine = DrawAnimatingItem;
        obj->floor = obj->ceiling = NULL;
        obj->pivot_length = 0;
        obj->radius = DEFAULT_RADIUS;
        obj->shadow_size = 0;
        obj->hit_points = DONT_TARGET;
        obj->intelligent = obj->water_creature = 0;
        obj->save_position = obj->save_hitpoints = obj->save_flags = obj->save_anim = 0;
    }

    BaddyObjects();
    TrapObjects();
    ObjectObjects();

    InitialiseHair();
}

void PickUpCollision(sint16 item_num, ITEM_INFO *laraitem, COLL_INFO *coll);
void RemoveDrawnItem(sint16 item_num);
extern int level_items;

void GetCarriedItems(void)
{
    int i;
    sint16 pickup_number;
    ITEM_INFO *item, *pickup;

    for (i = 0, item = items; i < level_items; i++, item++)
    {
        if (objects[item->object_number].intelligent)
        {
            item->carried_item = NO_ITEM;

            pickup_number = room[item->room_number].item_number;
            do
            {
                pickup = &items[pickup_number];
                if (pickup->pos.x_pos == item->pos.x_pos && pickup->pos.y_pos == item->pos.y_pos
                    && pickup->pos.z_pos == item->pos.z_pos && objects[pickup->object_number].collision == PickUpCollision)
                {
                    pickup->carried_item = item->carried_item;
                    item->carried_item = pickup_number;
                    RemoveDrawnItem(pickup_number);
                    pickup->room_number = NO_ROOM;
                }

                pickup_number = pickup->next_item;
            }
            while (pickup_number != NO_ITEM);
        }
    }
}
