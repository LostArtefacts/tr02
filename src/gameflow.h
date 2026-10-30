#ifndef GAMEFLOW_H
#define GAMEFLOW_H

#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"

#define MAXIMUM_LEVELS 24

enum pc_string_ids {
    PCSTR_DETAILLEVEL = 0,
    PCSTR_DEMOMODE,
    PCSTR_SOUND,
    PCSTR_CONTROLS,
    PCSTR_GAMMA,
    PCSTR_SETVOLUME,
    PCSTR_USERKEYS,
    PCSTR_SAVEMESS1,
    PCSTR_SAVEMESS2,
    PCSTR_SAVEMESS3,
    PCSTR_SAVEMESS4,
    PCSTR_SAVEMESS5,
    PCSTR_SAVEMESS6,
    PCSTR_SAVEMESS7,
    PCSTR_SAVEMESS8,
    PCSTR_SAVESLOT
};

enum game_levels {
    LV_GYM = 0,
    LV_FIRSTLEVEL
};

enum level_types {
    IL_TITLE,
    IL_NORMAL,
    IL_SAVED,
    IL_DEMO,
    IL_CUTSCENE,
    IL_STORY_SO_FAR,
    IL_QUIET,
    IL_STORY_SO_FAR_MIDLEVEL
};

typedef struct gameflow_info {
    sint32 firstOption;
    sint32 title_replace;
    sint32 ondeath_demo_mode;
    sint32 ondeath_ingame;
    sint32 noinput_time;
    sint32 on_demo_interrupt;
    sint32 on_demo_end;
    sint32 unused1[9];
    sint16 num_levels;
    sint16 num_picfiles;
    sint16 num_titlefiles;
    sint16 num_fmvfiles;
    sint16 num_cutfiles;
    sint16 num_demos;
    sint16 title_track;
    sint16 singlelevel;
    uint16 unused2[16];
    uint16 demoversion:1;
    uint16 title_disabled:1;
    uint16 cheatmodecheck_disabled:1;
    uint16 noinput_timeout:1;
    uint16 loadsave_disabled:1;
    uint16 screensizing_disabled:1;
    uint16 lockout_optionring:1;
    uint16 dozy_cheat_enabled:1;
    uint16 cyphered_strings:1;
    uint16 gym_enabled:1;
    uint16 play_any_level:1;
    uint16 cheat_enable:1;
    uint16 securitytag:1;
    uint16 unused3[3];
    uint8 cypher_code;
    uint8 language;
    uint8 secret_track;
    uint8 stats_track;
    char pads[4];
} GAMEFLOW_INFO;

extern GAMEFLOW_INFO gameflow;
extern char **GF_Level_Names;
extern char **GF_PCStrings;
extern sint16 *GF_frontendSequence;
extern sint16 *GF_level_sequence_list[];
extern sint16 GF_valid_demos[];

sint32 GF_DoFrontEndSequence(void);
sint32 GF_DoLevelSequence(int level, int type);
sint32 GF_InterpretSequence(sint16 *ptr, int type, int seq_type);

#ifdef __cplusplus
}
#endif

#endif
