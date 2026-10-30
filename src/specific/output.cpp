#include "global.h"
#include "output.h"
#include "../inventry.h"
#include "winmain.h"
#include "display.h"
#include "time.h"
#include "../gameflow.h"

extern "C" void Func444560(void);
extern "C" void HWR_InitState(void);
extern "C" void ClearBuffers(DWORD dwClrFlags, DWORD dwColour);

#define ANIMATED_TEXTURE 10

int S_DumpScreen(void)
{
    int nframes;

    nframes = Sync();
    while (nframes < 2)
    {
        while (!Sync())
            ;
        nframes++;
    }

    ScreenPartialDump();

    return nframes;
}

void S_ClearScreen(void)
{
    ScreenClear();
}

void S_InitialiseScreen(int type)
{
    if (type >= 0)
    {
        if (type != IL_TITLE)
            TempVideoRemove();
        FadeToPal(30, game_palette);
    }
    else
        FadeToPal(0, game_palette);

    if (SavedAppSettings.RenderMode != RM_Software)
        HWR_InitState();
}

void S_InsertBackPolygon(sint32 min_x, sint32 min_y, sint32 max_x, sint32 max_y, int colour)
{
    min_x += phd_winxmin;
    max_x += phd_winxmin;
    min_y += phd_winymin;
    max_y += phd_winymin;
    InsertFlatRect(min_x, min_y, max_x, max_y, phd_zfar + 1, inv_colours[C_BLACK]);
}

void AnimateTextures(int nframes)
{
    int i, j;
    sint16 *ptr;
    PHDTEXTURESTRUCT temp;
    static int frame_comp = 0;

    frame_comp += nframes;
    while (frame_comp > ANIMATED_TEXTURE)
    {
        ptr = aranges;
        for (i = (int)*(ptr++); i > 0; i--, ptr++)
        {
            j = (int)*(ptr++);
            temp = phdtextinfo[*(ptr)];
            for (; j > 0; j--, ptr++)
                phdtextinfo[*(ptr)] = phdtextinfo[*(ptr + 1)];
            phdtextinfo[*(ptr)] = temp;
        }
        frame_comp -= ANIMATED_TEXTURE;
    }
}

void S_SetupBelowWater(int underwater)
{
    if (underwater ^ wet)
    {
        if (underwater)
            FadeToPal(1, water_palette);
        else
            FadeToPal(1, game_palette);
        wet = underwater;
    }
    water_effect = !underwater;
    shade_effect = 1;
    bBlueEffect = true;
}

void S_SetupAboveWater(int underwater)
{
    water_effect = underwater;
    shade_effect = 0;
    bBlueEffect = bool(underwater);
}

void S_DontDisplayPicture(void)
{
    if (SavedAppSettings.RenderMode == RM_Hardware)
        Func444560();
}

void ScreenDump(void)
{
    DX_UpdateFrame(true, 0);
}

void ScreenPartialDump(void)
{
    DX_UpdateFrame(true, &GrcDumpWindow);
}

void ScreenClear(bool tWindow)
{
    DWORD dwFlags = SavedAppSettings.RenderMode == RM_Software ? 0x10 : 2;
    if (tWindow)
        dwFlags |= 0x100;
    ClearBuffers(dwFlags, 0);
}
