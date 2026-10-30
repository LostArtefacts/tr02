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

enum {
    GFE_PICTURE,
    GFE_LIST_START,
    GFE_LIST_END,
    GFE_PLAYFMV,
    GFE_STARTLEVEL,
    GFE_CUTSCENE,
    GFE_LEVCOMPLETE,
    GFE_DEMOPLAY,
    GFE_JUMPTO_SEQ,
    GFE_END_SEQ,
    GFE_SETTRACK,
    GFE_SUNSET,
    GFE_LOADINGPIC,
    GFE_DEADLY_WATER,
    GFE_REMOVE_WEAPONS,
    GFE_GAMECOMPLETE,
    GFE_CUTANGLE,
    GFE_NOFLOOR,
    GFE_ADD2INV,
    GFE_STARTANIM,
    GFE_NUMSECRETS,
    GFE_KILL2COMPLETE,
    GFE_REMOVE_AMMO
};

#define ADDINV_NUMBEROF 27

extern sint16 GF_NoFloor;
extern sint16 GF_DeadlyWater;
extern sint32 GF_LaraStartAnim;
extern char GF_Kill2Complete;
extern sint16 GF_RemoveAmmo;
extern sint16 GF_RemoveWeapons;
extern char GF_StartGame;
extern sint16 GF_NumSecrets;
extern sint16 GF_CDtracks[];
extern sint16 CineTargetAngle;
extern uint8 GF_Add2InvItems[];
extern uint8 GF_SecretInvItems[];
extern char **GF_PictureFilesStringTable;
extern char **GF_FmvFilesStringTable;
extern char **GF_cutscenefilenames;
sint32 StartGame(int level, int type);
sint32 StartDemo(int level_number);
sint32 StartCinematic(int level_number);
void SetCutsceneTrack(int track);
int LevelStats(int level);
int GameStats(int level);
void DisplayCredits(void);
void ShowThis(char *name, int time);
int S_IntroFMV(char *movie1, char *movie2);
int S_PlayFMV(char *fname, int sequence);

sint32 GF_InterpretSequence(sint16 *ptr, int type, int seq_type)
{
    sint32 ret_val;
    int i;
    sint32 option = EXIT_TO_TITLE;
    sint16 level;
    sint16 ntracks;
    char string[80];

    ntracks = 0;
    GF_CDtracks[0] = 2;
    GF_LaraStartAnim = GF_SunsetEnabled = GF_DeadlyWater = GF_NoFloor = 0;
    GF_RemoveWeapons = GF_RemoveAmmo = GF_Kill2Complete = 0;
    CineTargetAngle = 0x4000;
    GF_NumSecrets = 3;
    for (i = 0; i < ADDINV_NUMBEROF; i++)
        GF_Add2InvItems[i] = 0;
    for (i = 0; i < ADDINV_NUMBEROF; i++)
        GF_SecretInvItems[i] = 0;
    while (*ptr != GFE_END_SEQ) {
        switch (*ptr) {
        case GFE_STARTLEVEL:
            level = ptr[1];
            if (level <= gameflow.num_levels) {
                if (type == IL_STORY_SO_FAR) {
                    ptr += 2;
                    continue;
                }
                if (type == IL_STORY_SO_FAR_MIDLEVEL)
                    return EXIT_TO_TITLE;
                option = StartGame(level, type);
                GF_StartGame = 0;
                if (type == IL_SAVED)
                    type = IL_NORMAL;
                if ((option & ~0xff) != LEVELCOMPLETE)
                    return option;
            } else {
                sprintf(string, "INVALID LEVEL %d", level);
                option = EXIT_TO_TITLE;
            }
            ptr += 2;
            break;

        case GFE_DEMOPLAY:
            if (type != IL_SAVED && type != IL_STORY_SO_FAR && type != IL_STORY_SO_FAR_MIDLEVEL)
                return StartDemo(ptr[1]);
            ptr += 2;
            break;

        case GFE_CUTANGLE:
            if (type != IL_SAVED)
                CineTargetAngle = ptr[1];
            ptr += 2;
            break;

        case GFE_CUTSCENE:
            if (type != IL_SAVED) {
                sprintf(string, "CUTSCENE %d %s", ptr[1], GF_cutscenefilenames[ptr[1]]);
                level = (sint16)CurrentLevel;
                ret_val = StartCinematic(ptr[1]);
                CurrentLevel = level;
                if (ret_val == 2 && (type == IL_STORY_SO_FAR || type == IL_STORY_SO_FAR_MIDLEVEL))
                    return EXIT_TO_TITLE;
                if (ret_val == 3)
                    return EXITGAME;
            }
            ptr += 2;
            break;

        case GFE_PLAYFMV:
            if (type != IL_SAVED) {
                if (ptr[2] == GFE_PLAYFMV) {
                    if (S_IntroFMV(GF_FmvFilesStringTable[ptr[1]], GF_FmvFilesStringTable[ptr[3]]))
                        return EXITGAME;
                    ptr += 2;
                } else if (S_PlayFMV(GF_FmvFilesStringTable[ptr[1]], ptr[1])) {
                    return EXITGAME;
                }
            }
            ptr += 2;
            break;

        case GFE_LEVCOMPLETE:
            if (type == IL_STORY_SO_FAR || type == IL_STORY_SO_FAR_MIDLEVEL) {
                ptr++;
                continue;
            }
            if (LevelStats(CurrentLevel))
                return EXIT_TO_TITLE;
            option = STARTGAME | (CurrentLevel + 1);
            ptr++;
            break;

        case GFE_GAMECOMPLETE:
            DisplayCredits();
            if (GameStats(CurrentLevel))
                return EXIT_TO_TITLE;
            option = EXIT_TO_TITLE;
            ptr++;
            break;

        case GFE_PICTURE:
            if (type != IL_SAVED) {
                sprintf(string, "PICTURE %s", GF_PictureFilesStringTable[ptr[1]]);
                ShowThis(string, 150);
            }
            ptr += 2;
            break;

        case GFE_JUMPTO_SEQ:
            sprintf(string, "JUMPSEQ %d", ptr[1]);
            ptr += 2;
            break;

        case GFE_LIST_START:
            ptr++;
            break;

        case GFE_LIST_END:
            ptr++;
            break;

        case GFE_SETTRACK:
            GF_CDtracks[ntracks] = ptr[1];
            SetCutsceneTrack(ptr[1]);
            ntracks++;
            ptr += 2;
            break;

        case GFE_SUNSET:
            if (type == IL_STORY_SO_FAR || type == IL_STORY_SO_FAR_MIDLEVEL) {
                ptr++;
                continue;
            }
            GF_SunsetEnabled = 1;
            ptr++;
            break;

        case GFE_DEADLY_WATER:
            if (type == IL_STORY_SO_FAR || type == IL_STORY_SO_FAR_MIDLEVEL) {
                ptr++;
                continue;
            }
            GF_DeadlyWater = 1;
            ptr++;
            break;

        case GFE_NOFLOOR:
            if (type == IL_STORY_SO_FAR || type == IL_STORY_SO_FAR_MIDLEVEL) {
                ptr += 2;
                continue;
            }
            GF_NoFloor = ptr[1];
            ptr += 2;
            break;

        case GFE_STARTANIM:
            if (type == IL_STORY_SO_FAR || type == IL_STORY_SO_FAR_MIDLEVEL) {
                ptr += 2;
                continue;
            }
            GF_LaraStartAnim = ptr[1];
            ptr += 2;
            break;

        case GFE_NUMSECRETS:
            if (type == IL_STORY_SO_FAR || type == IL_STORY_SO_FAR_MIDLEVEL) {
                ptr += 2;
                continue;
            }
            GF_NumSecrets = ptr[1];
            ptr += 2;
            break;

        case GFE_ADD2INV:
            if (type == IL_STORY_SO_FAR || type == IL_STORY_SO_FAR_MIDLEVEL) {
                ptr += 2;
                continue;
            }
            if (ptr[1] < 1000) {
                GF_SecretInvItems[ptr[1]]++;
            } else if (type != IL_SAVED) {
                GF_Add2InvItems[ptr[1] - 1000]++;
            }
            ptr += 2;
            break;

        case GFE_REMOVE_WEAPONS:
            if (type == IL_STORY_SO_FAR || type == IL_STORY_SO_FAR_MIDLEVEL) {
                ptr++;
                continue;
            }
            if (type != IL_SAVED)
                GF_RemoveWeapons = 1;
            ptr++;
            break;

        case GFE_REMOVE_AMMO:
            if (type == IL_STORY_SO_FAR || type == IL_STORY_SO_FAR_MIDLEVEL) {
                ptr++;
                continue;
            }
            if (type != IL_SAVED)
                GF_RemoveAmmo = 1;
            ptr++;
            break;

        case GFE_KILL2COMPLETE:
            if (type == IL_STORY_SO_FAR || type == IL_STORY_SO_FAR_MIDLEVEL) {
                ptr++;
                continue;
            }
            GF_Kill2Complete = 1;
            ptr++;
            break;

        case GFE_LOADINGPIC:
            ptr += 2;
            break;

        default:
            return EXITGAME;
        }
    }
    if (type == IL_STORY_SO_FAR || type == IL_STORY_SO_FAR_MIDLEVEL)
        return 0;
    return option;
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
