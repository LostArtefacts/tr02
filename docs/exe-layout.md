# Standalone executable layout

`just exe` builds the recovered sources with VC5 and produces
`build/tomb2.exe`. The standalone objects additionally use `/Gy`, so the
linker can place functions independently of their translation unit. The DLL
build retains its existing compiler flags and function comparison.

The project linker lives in `config/layout/`, loaded through `build.hook`.
The shared build tool delegates standalone linking to the hook; its ordinary
linker remains available through `customize=False`.

## Link passes

1. Preserve the compiler's function bytes and relocation records, recover
   aliases, order function sections by `config/symbols.csv`, and fill unused
   code slots with alignment padding. Oversized functions can consume the
   following gap; the placement pass records their displaced neighbors.
   Order compiler exception sections by their owning functions' original
   addresses, independently of the split source objects' link order.
   Standalone Steam patches in `src/exe/*.s` use symbolic MASM instructions.
   Each populated code section must expose one mapped entry point at its
   start; additional entries must retain their mapped relative addresses.
   Promote these sections to function COMDATs in the same `.text`
   group as VC5's output, preserving their instructions and relocations.
   Order the entry before private string symbols in the COFF symbol table,
   updating relocation indices, so VC5 selects the function as its COMDAT key.
   The hook DLL retains the recovered C implementations of these patches.
2. Recover game globals from named symbols and references in matching
   functions. Bind zero initialized game state at its original addresses.
   Private locals use recovered references or the small explicit address
   tables in the project linker. Fixed, common and BSS references cannot
   supply initialized allocation hints, even when a temporary linked data
   section overlaps the original state addresses.
3. Make a preliminary link to select the required VC5 runtime archive members.
   Relink their compiled code in recovered order. Archive member code is
   identified by symbols and instruction content with relocations masked.
   The runtime mainly uses SP3 members, with RTM's `output.obj` and `gmtime.obj`
   matching the target. Retain the original `__allshl` helper even when the
   current game objects do not reference it. Preserve compiler NOP padding
   and linker INT3 padding according to the input sections.
   Compile the recovered `_setmbcp` and `setSBCS` routines in
   `config/runtime/mbctype.c`, binding their relocations to the archive's
   globals and private helpers. The target predates SP3's `__ismbcodepage`
   flag. Both routines match its instructions exactly with relocations masked.
   Keep associative exception scope tables when removing their code sections.
4. Link with relocation records and anchors for live initialized allocations.
   Include the GUID data contributed by DirectX import libraries. Recover
   allocation destinations from symbols, matching references and compiled
   string/constant contents; propagate references through data tables. Keep
   the RTTI complete-object locator immediately before its vftable, as required
   by the VC5 ABI, so its references recover the remaining RTTI allocations.
   Recover intact CRT data sections from confirmed interior allocations only
   when the entire linked payload matches, rebasing recorded self references.
   This retains the floating-point initializer block and both ctype pointers.
   Recover C++ constructor-table slots by their compiled function pointers,
   independently of the input objects' order. `src/exe/fullscreen.cpp` owns
   the fullscreen mode object and emits its empty constructor's initializer
   and return stub at `0x444c60` and `0x444c70`. The shared `InitResult`
   definition supplies the original typed catches; `SE_Exception` retains its
   original empty copy constructor at `0x44e940` and its metadata reference.
   The window unit retains `<math.h>` from the original include bundle to
   preserve code generation after sharing the enum.
   Retained release strings, coder names, sample-mode names, CD path formats,
   the old Hires registry label and the month-name table come from C
   initializers in their owning source files. The version identity keeps
   the original release's `Dec 10 1997` date independently of the rebuild date.
   The original initial level, setup screen dimensions, normal sample pitch,
   RGB defaults and retained audio device type also have C initializers.
   Three unreferenced runtime words retain their release initializers;
   their semantic names remain unresolved. Unreferenced CRT function-pointer
   allocations can fill an equal number of identical remaining target slots
   in compiled allocation order. References or extra possible slots prevent
   this recovery, so named or observable state still requires an identity.
5. Place the compiled allocations, apply address relocations, serialize the
   recovered import ABI, and update resource directory metadata. Synchronize
   the final link map with the emitted image.

The original supplies addresses, layout metadata, import identities, and the
resources already used by the ordinary build. Emitted game and runtime code
comes from the newly compiled objects and the configured VC5 libraries.
Initialized variable contents come from the linked compiler payloads.

Matching runtime instructions and initialized pointer tables recover original
BSS section and common symbol addresses. Conflicting references, allocations
outside the original data section and bindings over nonzero initialized
contents are rejected. All current runtime state has recovered destinations;
the output needs no additional section. The linker retains a `.crt_bss`
fallback for unrecovered state in future builds.
The original five sections retain their addresses, sizes, file offsets and
characteristics. Headers are byte-identical. The result remains a fixed-base
executable.

## Checks

The build rejects uncovered initialized bytes, unresolved game globals,
overlapping placements, unsupported imports and changes to instruction bytes
outside relocation operands or explicitly placed inline data. Every moved
allocation is checked against its compiled payload after applying its recorded
relocations. Original code bytes occupied by an oversized compiled function
are protected from inline string/table placement.

Generated placement reports, intermediate images and maps are in
`build/layout/`. They are regenerated from the current objects and original;
an existing standalone map is not required.

Run:

```sh
just exe
just compare-exe
python3 -m unittest discover -s config/layout/tests -v
```

The tests include the 96% whole-file threshold, import slot identities and
separate IAT tokens, section layout, recovered runtime state,
runtime code placement, exception scope tables and negative relocation,
runtime replacement, BSS recovery, exception ownership, typed catch metadata
and RTTI locator/vftable adjacency checks. The
function-level comparison remains `just compare`.

## Remaining parity

The November 4 runtime/layout and mapper passes reduced whole-file differences from
38,299 to 13,628 bytes (95.80% to 98.51%). Headers, `.rdata`, initialized
`.data`, `.idata` and `.rsrc` are byte-identical; `.text` is 96.60%.
All 210 selected CRT code blocks are byte-identical, including their
relocated operands, and sit at their original addresses. The constructor
table is byte-identical. All 114 recovered runtime state bindings
sit in the original data section, eliminating `0x16d0` bytes of private state.

Most remaining code differences are in functions already listed in
`hooks.allow_diff`. The remaining differences are in game rendering, collision and platform routines.
Some oversized game functions still displace neighbors, recorded in
`build/layout/code-moved.json`.

The entire CRT address range, including padding, the import thunk and the
final game-generated exception thunks, is byte-identical. Remaining data
work retains eight constant allocations without confirmed identities (40 bytes),
recorded in `build/layout/data-unplaced.json`. Their fallback placements already
match the original `.rdata` contents. All remaining byte differences are code.
The current joystick and sound-adapter initializers each match all 57 bytes
at `0x447220` and `0x447bf0`. Their member-string constructors inline while
node destruction retains the original inlining depth. `gtmap_persp32_fp` matches all 4,038 bytes at its
original address.
Restoring signed UV shifts in its first five pixels, ordering one independent
lighting update, and including the Windows/process headers before the renderer
header reproduce VC5's operand ordering. `wgtmap_persp32_fp` now matches all
5,316 bytes too. The original DSP's `/Oa` setting and `<math.h>` include context
preserve its temporary allocation. Recovering the common row exit, the
floating-point updates in both sampling branches and the short-span UV
temporaries reproduces its instruction order. Removing a redundant tail
shade-step assignment retains the original stack slot. The same `/Oa` setting
also matches `xgen_xg` and `xgen_xguvpersp_fp`, preserving all previous matches.
`S_UpdateInput` now matches all 2,766 bytes. Its mode searches retain the
original constructor temporaries, and the previous/next loops retain the
position iterators and current/candidate mode references.

The standalone window cleanup, sound cleanup, path handling, eight CD playback
routines and audio-index loader reproduce the Steam patches as symbolic
assembly. The entire playback block at `0x455570` through `0x45589f` matches,
including its embedded command strings and padding. CD volume handling at
`0x4558a0` through `0x4558d6` matches as well. The C implementations
remain available for the hook DLL.

The standalone text routines and their glyph helpers retain ECX arguments,
ECX results and carry-flag predicates. The CD volume patch also retains the
redundant scale bits in its index-free SIB addressing byte. MASM cannot spell
that addressing form, so the single `mov edi, [esp+24h]` instruction has an
explicit encoding; the remaining patch instructions assemble normally.

TOMB21's surviving game objects are mainly PlayStation LNK objects. Their
debug records help establish header relationships and declarations; the
standalone PE link uses the current Win32 COFF objects and VC5 runtime members.

The graphics-settings handler and UI update now match all 1,034 and 1,845
bytes at their original addresses. Restoring the reference control-result
temporaries, common update path and combo-box switch removes four displaced
functions. The UI update retains signed list-count comparisons, a cached
maximum width and the shared enable flag. Both advanced-settings routines
also match their original instruction and checkbox conversion sequences.

The fullscreen-mode list update matches all 492 bytes. Its adapter body
reference, position iterator, mode lookup and signed count reproduce the
original register lifetimes and switch-table layout.

The graphics-settings initializer matches all 380 bytes. Adapter iteration
retains the body reference and position iterator, and the fallback mode uses
the original four-argument constructor.

Renderer startup matches all 933 bytes at `0x448f00`. The recovered
`RENDER_INFO` record starts at `0x4d92c0`; its fields share the existing
per-field bindings used by other translation units. The zero-initialized
binding pass maps both views to the same original storage. Retaining this
record, the chained dimension assignments and the original renderer/list
selection reproduces the compiler's aliasing and instruction order.

Static-mesh dynamic lighting matches all 292 bytes at `0x451580`.
The attenuation expression reads the light's falloff field again while
retaining the cached falloff for the distance limit. This preserves the
original temporary lifetimes and range-check register allocation.

Background drawing matches all 519 bytes at `0x444350`. Its window width
and height are cached before scaling the tile coordinates, as in the
surviving source, rather than read independently in each expression.
