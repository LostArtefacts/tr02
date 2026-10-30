#include "game.h"

extern sint16 inv_main_objects, inv_main_current;
extern sint16 inv_keys_objects, inv_keys_current;

void Inv_RemoveAllItems(void)
{
    inv_main_objects = 0;
    inv_main_current = 0;
    inv_keys_objects = 0;
    inv_keys_current = 0;
}
