#ifndef GAMEFLOW_H
#define GAMEFLOW_H

#include "types.h"

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
extern sint16 *GF_frontendSequence;
extern sint16 *GF_level_sequence_list[];
extern sint16 GF_valid_demos[];

sint32 GF_DoFrontEndSequence(void);
sint32 GF_DoLevelSequence(int level, int type);
sint32 GF_InterpretSequence(sint16 *ptr, int type, int seq_type);

#endif
