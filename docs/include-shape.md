# Reference include shape

The reference is Core's later source tree, not an exact snapshot of the Steam release.
For game C units, `BACKUP` is preferred to `GAME`. Paths and filename case are mapped to this repository; missing headers are recorded rather than replaced with empty stubs.

## Verified restoration

The first global pass tested 150 include variants across 75 mapped translation units.
Both the available original list and that list with current dependencies retained were tried.
Each candidate was compared against all functions in its object, including static helpers.
Candidates introducing unresolved symbols or additional compiler warnings were rejected.

22 units accepted a changed include list. The full DLL comparison improved from
1144/1220 to 1145/1220 with no lost matches: `BGND_AddTexture` now matches.
These counts include the three earlier hwinsert gains: `InsertObjectG3_Sorted`,
`InsertObjectG4_Sorted` and `InsertGT4_Sorted`.

A retained dependency means the current shared headers do not yet expose everything
the original include graph provided. Restoring a direct include list alone does not
reconstruct its transitive declarations or their order.

## Remaining header graph work

### TOMB21 header recovery follow-up

The baseline for this pass was 1145/1220 matching functions (75 nonmatches). The
following header changes were built and compared against that full baseline;
regressions were reverted rather than retained as speculative improvements.

| Experiment | Matching functions | New matches | Retained |
| --- | ---: | ---: | --- |
| Separate the six scalar typedefs into `typedefs.h` | 1118 | 0 | No |
| Restore platform types before the game type chain | 1119 | 0 | No |
| Centralize `GetCeiling` / `GetDoor` prototypes | 1130 | 0 | No |
| Include the full platform type header through `standard.h` | 1140 | 0 | No |
| Restore missing matrix prototypes in reference order | 1119 | 0 | No |
| Recover `PCX_HEADER` in `specific/stypes.h` for the screenshot encoder | 1145 | 0 | Yes |

The retained PCX definition uses the reference's tagged structure and field
names, replacing the screenshot encoder's private duplicate. Its 128-byte
layout and the encoder's compiled output are preserved. Moving the decoder in
`specific/output.cpp` onto this header also changed `S_PrintShadow`'s generated
code, so the decoder retains its local definition for now. This is a recovered
type owner, not an increase in function matches.

TOMB21's game object files inspected here are Sony PlayStation LNK version 2
objects, not Windows COFF. Their debug filename records corroborate the game
header dependencies, but cannot establish the Windows compiler's declaration
order or ABI. The reference chain is `game.h -> specific/stypes.h ->
3dsystem/3dglodef.h -> typedefs.h`, with `game.h` also including `types.h`.
Recovering that graph wholesale currently loses matches; smaller changes need
to account for the declarations already visible at each point.

### Warning cleanup and PCX encoder match

The following cleanup removes the standalone build's 61 compiler diagnostics
and leaves both DLL and EXE builds warning-free at `/W3 /WX`:

- Declare previously implicit functions before their first use, using the
  implemented return and parameter types. Keep these declarations local to the
  translation units; moving them into shared headers previously lost matches.
- Move `GetCeiling`'s late declaration in `lara1gun.c` before its first call.
- Make intentional narrowing and signed/unsigned comparisons explicit; replace
  the implicit integer-to-bool conversion with a comparison against zero.
- Remove unused collision-height locals and explicitly discard unused exception
  objects while retaining their declaration scope.
- Explicitly cast the two empty collision-handler entries in the effect table
  to its one-argument callback type.

The full DLL comparison retains every previous match and adds `EncodePutPCX`:
1146/1220 functions, 294609 matching bytes, 74 nonmatches. A byte-sized local
for the masked PCX tag, tested only for a single-byte run, recovers VC5's
original register allocation. The signed byte comparison preserves the
existing tag test. This function has been removed from `hooks.allow_diff`.

Additional isolated searches of the declaration orders in `CopyBitmapPalette`
and `LoadTexturePage` found no further matches; those sources were retained.
The EXE comparison tool now closes its input files, so the layout tests also
pass with Python warnings treated as errors. The inline font tables at
`0x44b226` and `0x44b27e` have their recovered 88-byte and 66-byte sizes in the
symbol manifest, eliminating the progress report's missing-size warnings.

- Original game C units generally include `game.h`, which reaches platform types, rendering types, and the game types/module headers. Our `game.h` reaches a different split of those declarations. For example, restoring `box.c` to `game.h` alone leaves `CREATURE_INFO` undeclared.
- Original platform/rendering units generally begin with `standard.h`, followed by `global.h` and module headers. The new `standard.h` centralizes the Windows, DirectX and CRT includes; the unused `specific/stypes.h` addition was removed.
- Renderer interfaces still need target-specific recovery: `texture.h`, `drawprimitive.h`, `hwinsert.h`, `3d_gen.h`, and `picture.h` are the most common missing interfaces. Some reference-only headers describe TR3 features and must not be assumed to belong to TR2.
- `hwinsert.cpp` cannot use the reference global header directly yet: its local palette type disagrees with `global.h`. `output.cpp` needs declarations such as `Sync` before its original list can compile.
- `3d_gen.cpp` builds with the available reference order, but seven matching transforms regress. Its declaration order still needs reconstruction before that change can land.
- Missing prototype warnings remain a separate check on header completeness. `GetBestFrame`'s pointer return and the `S_InsertBackPolygon` parameter type were corrected without losing parity.

## Unit audit

"Original list" means the mapped available original includes suffice. "Retained dependencies" means additional current includes remain. "Needs declarations" means neither candidate compiles; "Codegen changes" means compiling candidates lose matches.

| Unit | Reference | Result | Missing headers |
| --- | --- | --- | --- |
| [`3dsystem/3d_gen.cpp`](../src/3dsystem/3d_gen.cpp) | `3DSYSTEM/3D_GEN.CPP` | Codegen changes | `../specific/hwinsert.h`, `../specific/debug.h`, `../3dsystem/3d_gen.h`, `../game/draw.h`, `../specific/drawprimitive.h`, `../specific/scanlines.h` |
| [`3dsystem/3d_line.cpp`](../src/3dsystem/3d_line.cpp) | `3DSYSTEM/3D_LINE.CPP` | Unchanged / dependency retained | — |
| [`3dsystem/3d_out.cpp`](../src/3dsystem/3d_out.cpp) | `3DSYSTEM/3D_OUT.CPP` | Unchanged / dependency retained | — |
| [`3dsystem/3dinsert.cpp`](../src/3dsystem/3dinsert.cpp) | `3DSYSTEM/3DINSERT.CPP` | Unchanged / dependency retained | — |
| [`3dsystem/hwinsert.cpp`](../src/3dsystem/hwinsert.cpp) | `3DSYSTEM/HWINSERT.CPP` | Needs declarations | `../specific/drawprimitive.h`, `../specific/hwinsert.h` |
| [`3dsystem/scalespr.cpp`](../src/3dsystem/scalespr.cpp) | `3DSYSTEM/SCALESPR.CPP` | Retained dependencies | `../specific/litesrc.h` |
| [`bird.c`](../src/bird.c) | `BACKUP/BIRD.C` | Unchanged / dependency retained | — |
| [`boat.c`](../src/boat.c) | `BACKUP/BOAT.C` | Codegen changes | — |
| [`box.c`](../src/box.c) | `BACKUP/BOX.C` | Unchanged / dependency retained | — |
| [`camera.c`](../src/camera.c) | `BACKUP/CAMERA.C` | Codegen changes | — |
| [`cinema.c`](../src/cinema.c) | `BACKUP/CINEMA.C` | Unchanged / dependency retained | — |
| [`collide.c`](../src/collide.c) | `BACKUP/COLLIDE.C` | Unchanged / dependency retained | — |
| [`control.c`](../src/control.c) | `BACKUP/CONTROL.C` | Unchanged / dependency retained | `../game/fish.h` |
| [`demo.c`](../src/demo.c) | `BACKUP/DEMO.C` | Unchanged / dependency retained | — |
| [`dino.c`](../src/dino.c) | `BACKUP/DINO.C` | Unchanged / dependency retained | — |
| [`diver.c`](../src/diver.c) | `BACKUP/DIVER.C` | Unchanged / dependency retained | — |
| [`dog.c`](../src/dog.c) | `BACKUP/DOG.C` | Unchanged / dependency retained | — |
| [`dragon.c`](../src/dragon.c) | `BACKUP/DRAGON.C` | Codegen changes | — |
| [`draw.c`](../src/draw.c) | `BACKUP/DRAW.C` | Original list | — |
| [`eel.c`](../src/eel.c) | `BACKUP/EEL.C` | Unchanged / dependency retained | — |
| [`effects.c`](../src/effects.c) | `BACKUP/EFFECT2.C` | Unchanged / dependency retained | `../game/fish.h` |
| [`fighter.c`](../src/fighter.c) | `BACKUP/FIGHTER.C` | Unchanged / dependency retained | — |
| [`gameflow.c`](../src/gameflow.c) | `BACKUP/GAMEFLOW.C` | Retained dependencies | — |
| [`hair.c`](../src/hair.c) | `BACKUP/HAIR.C` | Unchanged / dependency retained | — |
| [`health.c`](../src/health.c) | `BACKUP/HEALTH.C` | Codegen changes | — |
| [`inventry.c`](../src/inventry.c) | `BACKUP/INVENTRY.C` | Retained dependencies | — |
| [`invfunc.c`](../src/invfunc.c) | `BACKUP/INVFUNC.C` | Retained dependencies | — |
| [`items.c`](../src/items.c) | `BACKUP/ITEMS.C` | Unchanged / dependency retained | — |
| [`lara.c`](../src/lara.c) | `BACKUP/LARA.C` | Unchanged / dependency retained | — |
| [`lara1gun.c`](../src/lara1gun.c) | `BACKUP/LARA1GUN.C` | Unchanged / dependency retained | — |
| [`lara2gun.c`](../src/lara2gun.c) | `BACKUP/LARA2GUN.C` | Unchanged / dependency retained | — |
| [`laraclmb.c`](../src/laraclmb.c) | `BACKUP/LARACLMB.C` | Retained dependencies | — |
| [`larafire.c`](../src/larafire.c) | `BACKUP/LARAFIRE.C` | Retained dependencies | — |
| [`laraflar.c`](../src/laraflar.c) | `BACKUP/LARAFLAR.C` | Unchanged / dependency retained | — |
| [`laramisc.c`](../src/laramisc.c) | `BACKUP/LARAMISC.C` | Unchanged / dependency retained | — |
| [`larasurf.c`](../src/larasurf.c) | `BACKUP/LARASURF.C` | Unchanged / dependency retained | — |
| [`laraswim.c`](../src/laraswim.c) | `BACKUP/LARASWIM.C` | Unchanged / dependency retained | — |
| [`lot.c`](../src/lot.c) | `BACKUP/LOT.C` | Unchanged / dependency retained | — |
| [`missile.c`](../src/missile.c) | `BACKUP/MISSILE.C` | Unchanged / dependency retained | — |
| [`moveblok.c`](../src/moveblok.c) | `BACKUP/MOVEBLOK.C` | Unchanged / dependency retained | — |
| [`objects.c`](../src/objects.c) | `BACKUP/OBJECTS.C` | Retained dependencies | — |
| [`people.c`](../src/people.c) | `BACKUP/PEOPLE.C` | Unchanged / dependency retained | — |
| [`pickup.c`](../src/pickup.c) | `BACKUP/PICKUP.C` | Retained dependencies | — |
| [`rat.c`](../src/rat.c) | `BACKUP/RAT.C` | Unchanged / dependency retained | — |
| [`savegame.c`](../src/savegame.c) | `BACKUP/SAVEGAME.C` | Codegen changes | — |
| [`setup.c`](../src/setup.c) | `BACKUP/SETUP.C` | Retained dependencies | — |
| [`shark.c`](../src/shark.c) | `BACKUP/SHARK.C` | Unchanged / dependency retained | — |
| [`skidoo.c`](../src/skidoo.c) | `BACKUP/SKIDOO.C` | Unchanged / dependency retained | — |
| [`sound.c`](../src/sound.c) | `BACKUP/SOUND.C` | Unchanged / dependency retained | — |
| [`specific/bgnd.cpp`](../src/specific/bgnd.cpp) | `SPECIFIC/BGND.CPP` | Retained dependencies | `debug.h`, `texture.h`, `drawprimitive.h` |
| [`specific/d3d.cpp`](../src/specific/d3d.cpp) | `SPECIFIC/D3D.CPP` | Original list | — |
| [`specific/display.cpp`](../src/specific/display.cpp) | `SPECIFIC/DISPLAY.CPP` | Original list | — |
| [`specific/ds.cpp`](../src/specific/ds.cpp) | `SPECIFIC/DS.CPP` | Needs declarations | — |
| [`specific/file.cpp`](../src/specific/file.cpp) | `SPECIFIC/FILE.CPP` | Needs declarations | `drawprimitive.h`, `texture.h`, `debug.h`, `picture.h`, `../game/invfunc.h`, `../game/setup.h`, `../game/objects.h`, `../game/demo.h` |
| [`specific/fmv.cpp`](../src/specific/fmv.cpp) | `SPECIFIC/FMV.CPP` | Unchanged / dependency retained | `../eidos/include/winplay.h` |
| [`specific/fmvback.cpp`](../src/specific/fmvback.cpp) | — | No direct reference mapping | — |
| [`specific/frontend.cpp`](../src/specific/frontend.cpp) | `SPECIFIC/FRONTEND.CPP` | Needs declarations | — |
| [`specific/game.cpp`](../src/specific/game.cpp) | `SPECIFIC/GAME.CPP` | Retained dependencies | `game1.h`, `../game/health.h`, `../game/invfunc.h`, `../game/setup.h`, `../game/control.h`, `../game/objects.h` |
| [`specific/hwrender.cpp`](../src/specific/hwrender.cpp) | `SPECIFIC/HWRENDER.CPP` | Retained dependencies | `texture.h`, `drawprimitive.h`, `hwinsert.h` |
| [`specific/init.cpp`](../src/specific/init.cpp) | `SPECIFIC/INIT.CPP` | Retained dependencies | `../3dsystem/3d_gen.h` |
| [`specific/input.cpp`](../src/specific/input.cpp) | `SPECIFIC/INPUT.CPP` | Needs declarations | `console.h`, `template.h`, `di.h`, `../game/invfunc.h`, `../game/laramisc.h`, `../game/objects.h`, `../game/demo.h` |
| [`specific/option.cpp`](../src/specific/option.cpp) | `SPECIFIC/OPTION.CPP` | Retained dependencies | `../game/objects.h` |
| [`specific/output.cpp`](../src/specific/output.cpp) | `SPECIFIC/OUTPUT.CPP` | Needs declarations | `console.h`, `hwinsert.h`, `bgnd.h`, `drawprimitive.h`, `../3dsystem/3d_gen.h`, `picture.h`, `texture.h` |
| [`specific/screen.cpp`](../src/specific/screen.cpp) | — | No direct reference mapping | — |
| [`specific/screenshot.cpp`](../src/specific/screenshot.cpp) | — | No direct reference mapping | — |
| [`specific/setup.cpp`](../src/specific/setup.cpp) | `SPECIFIC/SETTINGS.CPP` | Retained dependencies | — |
| [`specific/smain.cpp`](../src/specific/smain.cpp) | `SPECIFIC/SMAIN.CPP` | Needs declarations | `smain.h`, `picture.h`, `../game/demo.h`, `../game/control.h`, `../game/setup.h`, `../game/objects.h`, `../game/invfunc.h` |
| [`specific/specific.cpp`](../src/specific/specific.cpp) | `SPECIFIC/SPECIFIC.CPP` | Retained dependencies | `../game/control.h` |
| [`specific/swr.cpp`](../src/specific/swr.cpp) | `SPECIFIC/SWR.CPP` | Original list | — |
| [`specific/texture.cpp`](../src/specific/texture.cpp) | `SPECIFIC/TEXTURE.CPP` | Needs declarations | `texture.h` |
| [`specific/time.cpp`](../src/specific/time.cpp) | `SPECIFIC/TIME.CPP` | Original list | — |
| [`specific/utils.cpp`](../src/specific/utils.cpp) | `SPECIFIC/UTILS.CPP` | Codegen changes | `template.h`, `debug.h` |
| [`specific/winin.cpp`](../src/specific/winin.cpp) | — | No direct reference mapping | — |
| [`specific/winmain.cpp`](../src/specific/winmain.cpp) | `SPECIFIC/winmain.cpp` | Needs declarations | `template.h`, `directx.h`, `console.h`, `texture.h`, `drawprimitive.h`, `di.h`, `spanbuffer.h`, `hwinsert.h`, `picture.h` |
| [`specific/winmain2.cpp`](../src/specific/winmain2.cpp) | — | No direct reference mapping | — |
| [`specific/winmain3.cpp`](../src/specific/winmain3.cpp) | — | No direct reference mapping | — |
| [`specific/winvid.cpp`](../src/specific/winvid.cpp) | — | No direct reference mapping | — |
| [`sphere.c`](../src/sphere.c) | `BACKUP/SPHERE.C` | Unchanged / dependency retained | — |
| [`spider.c`](../src/spider.c) | `BACKUP/SPIDER.C` | Unchanged / dependency retained | — |
| [`text.c`](../src/text.c) | `BACKUP/TEXT.C` | Original list | — |
| [`traps.c`](../src/traps.c) | `BACKUP/TRAPS.C` | Unchanged / dependency retained | — |
| [`yeti.c`](../src/yeti.c) | `BACKUP/YETI.C` | Unchanged / dependency retained | — |

Raw experiment data and candidate sources are kept locally in `build/include-shape/`.
The audit/search drivers are in `/tmp/tr02-includes/`.
