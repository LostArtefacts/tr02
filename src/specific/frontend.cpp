#include "../inventry.h"
#include "display.h"
#include "fmv.h"
#include "frontend.h"

void S_FinishInventory(void)
{
    if (Inventory_Mode != INV_TITLE_MODE)
        TempVideoRemove();
}
