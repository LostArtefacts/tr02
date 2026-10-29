#include "global.h"
#include "init.h"

void ShutdownGame(void)
{
    if (malloc_buffer)
        GlobalFree((HGLOBAL)malloc_buffer);
}
