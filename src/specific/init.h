#ifndef INIT_H
#define INIT_H

#ifdef __cplusplus
extern "C" {
#endif

enum game_malloc_types {
    TEMP_ALLOC,
    TEXTURE_PAGES,
    MESH_POINTERS,
    MESHES,
    ANIMS,
    STRUCTS,
    RANGES,
    COMMANDS,
    BONES,
    FRAMES,
    ROOM_TEXTURES,
    ROOM_INFOS,
    ROOM_MESH,
    ROOM_DOOR,
    ROOM_FLOOR,
    ROOM_LIGHTS,
    ROOM_STATIC_MESH_INFOS,
    FLOOR_DATA,
    ITEMS,
    CAMERAS,
    SOUND_FX,
    BOXES,
    OVERLAPS,
    GROUNDZONE,
    FLYZONE,
    ANIMATING_TEXTURE_RANGES,
    CINEMATIC_FRAMES,
    LOADDEMO_BUFFER,
    SAVEDEMO_BUFFER,
    CINEMATIC_EFFECTS,
    MUMMY_HEAD_TURN,
    EXTRA_DOOR_STUFF,
    EFFECTS_ARRAY,
    CREATURE_DATA,
    CREATURE_LOT,
    SAMPLE_INFOS,
    SAMPLES,
    SAMPLE_OFFSETS,
    ROLLINGBALL_STUFF
};

void *game_malloc(int size, int type);
void ShutdownGame();
void CalculateWibbleTable();

#ifdef __cplusplus
}
#endif

#endif
