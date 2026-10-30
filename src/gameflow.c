#include "game.h"

extern sint16 GF_SunsetEnabled;
extern INVENTORY_ITEM InvCompassOption, InvPistolOption, InvFlareOption, InvShotgunOption;
extern INVENTORY_ITEM InvMagnumOption, InvUziOption, InvHarpoonOption, InvM16Option;
extern INVENTORY_ITEM InvGrenadeOption, InvPistolAmmoOption, InvShotgunAmmoOption;
extern INVENTORY_ITEM InvMagnumAmmoOption, InvUziAmmoOption, InvHarpoonAmmoOption;
extern INVENTORY_ITEM InvM16AmmoOption, InvGrenadeAmmoOption, InvSmallMedipackOption;
extern INVENTORY_ITEM InvLargeMedipackOption, InvPickup1Option, InvPickup2Option;
extern INVENTORY_ITEM InvPuzzle1Option, InvPuzzle2Option, InvPuzzle3Option, InvPuzzle4Option;
extern INVENTORY_ITEM InvKey1Option, InvKey2Option, InvKey3Option, InvKey4Option;
extern INVENTORY_ITEM InvPassportOption, InvDetailOption, InvSoundOption, InvControlOption;
extern INVENTORY_ITEM InvPhotoOption;
extern REQUEST_INFO SaveGameRequester;
int S_LoadGameFlow(char *filename);
void SetRequesterHeading(REQUEST_INFO *req, char *text1, uint32 flags1, char *text2, uint32 flags2);

int GF_LoadScriptFile(char *fname)
{
    GF_SunsetEnabled = 0;
    if (!S_LoadGameFlow(fname))
        return 0;

    gameflow.stats_track = 41;

    InvCompassOption.itemText = GF_GameStrings[35];
    InvPistolOption.itemText = GF_GameStrings[36];
    InvFlareOption.itemText = GF_GameStrings[43];
    InvShotgunOption.itemText = GF_GameStrings[37];
    InvMagnumOption.itemText = GF_GameStrings[38];
    InvUziOption.itemText = GF_GameStrings[39];
    InvHarpoonOption.itemText = GF_GameStrings[40];
    InvM16Option.itemText = GF_GameStrings[41];
    InvGrenadeOption.itemText = GF_GameStrings[42];
    InvPistolAmmoOption.itemText = GF_GameStrings[44];
    InvShotgunAmmoOption.itemText = GF_GameStrings[45];
    InvMagnumAmmoOption.itemText = GF_GameStrings[46];
    InvUziAmmoOption.itemText = GF_GameStrings[47];
    InvHarpoonAmmoOption.itemText = GF_GameStrings[48];
    InvM16AmmoOption.itemText = GF_GameStrings[49];
    InvGrenadeAmmoOption.itemText = GF_GameStrings[50];
    InvSmallMedipackOption.itemText = GF_GameStrings[51];
    InvLargeMedipackOption.itemText = GF_GameStrings[52];
    InvPickup1Option.itemText = GF_GameStrings[53];
    InvPickup2Option.itemText = GF_GameStrings[53];
    InvPuzzle1Option.itemText = GF_GameStrings[54];
    InvPuzzle2Option.itemText = GF_GameStrings[54];
    InvPuzzle3Option.itemText = GF_GameStrings[54];
    InvPuzzle4Option.itemText = GF_GameStrings[54];
    InvKey1Option.itemText = GF_GameStrings[55];
    InvKey2Option.itemText = GF_GameStrings[55];
    InvKey3Option.itemText = GF_GameStrings[55];
    InvKey4Option.itemText = GF_GameStrings[55];
    InvPassportOption.itemText = GF_GameStrings[56];
    InvDetailOption.itemText = GF_PCStrings[PCSTR_DETAILLEVEL];
    InvSoundOption.itemText = GF_PCStrings[PCSTR_SOUND];
    InvControlOption.itemText = GF_PCStrings[PCSTR_CONTROLS];
    InvPhotoOption.itemText = GF_GameStrings[57];
    SetRequesterHeading(&Load_Game_Requester, GF_GameStrings[GT_SELECTLEVEL], R_CENTRE, NULL, 0);
    SetRequesterHeading(&SaveGameRequester, GF_GameStrings[GT_SELECTLEVEL], R_CENTRE, NULL, 0);
    return 1;
}

sint32 GF_DoFrontEndSequence(void)
{
    sint32 option;

    option = GF_InterpretSequence(GF_frontendSequence, IL_NORMAL, 1);
    if (option == EXITGAME)
        return 1;
    return 0;
}

sint32 GF_DoLevelSequence(int level, int type)
{
    sint16 *ptr;
    sint32 option;

    do
    {
        if (level > gameflow.num_levels - 1)
        {
            title_loaded = 0;
            return EXIT_TO_TITLE;
        }
        ptr = GF_level_sequence_list[level];
        option = GF_InterpretSequence(ptr, type, 0);
        level++;
        if (gameflow.singlelevel >= 0)
            return option;
    } while ((option & ~0xff) == LEVELCOMPLETE);
    return option;
}
