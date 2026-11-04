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
2. Recover game globals from named symbols and references in matching
   functions. Bind zero initialized game state at its original addresses.
   Private locals use recovered references or the small explicit address
   tables in the project linker.
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
   string/constant contents; propagate references through data tables.
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
runtime replacement and BSS recovery checks. The
function-level comparison remains `just compare`.

## Remaining parity

The November 4 runtime/layout passes reduced whole-file differences from
38,299 to 34,002 bytes (95.80% to 96.28%). Headers, `.idata` and `.rsrc` are
byte-identical; `.text` is 91.64%, `.rdata` 94.16% and initialized `.data`
98.96%. All 210 selected CRT code blocks match with relocations masked and
sit at their original addresses. All 114 recovered runtime state bindings
sit in the original data section, eliminating `0x16d0` bytes of private state.

Most remaining code differences are in functions already listed in
`hooks.allow_diff`. The largest raw function differences are
`wgtmap_persp32_fp` (5,158 bytes), `gtmap_persp32_fp` (3,581),
`S_UpdateInput` (2,457), and `SE_GraphicsDlgUpdate` (1,633, including its
displacement). These counts compare each function at its mapped address.
Some oversized game functions still displace neighbors, recorded in
`build/layout/code-moved.json`.

The CRT range retains 49 differing bytes, including initializer boundary
relocations. The original constructor array includes an additional no-op
initializer at `0x444c60`, absent from the current game objects. Remaining
data work includes 35 allocations without a recovered destination (336 bytes
of constants and metadata), recorded in `build/layout/data-unplaced.json`.
The mapper and input experiments have not yet produced complete function
matches; their search variants remain outside `src/`.

TOMB21's surviving game objects are mainly PlayStation LNK objects. Their
debug records help establish header relationships and declarations; the
standalone PE link uses the current Win32 COFF objects and VC5 runtime members.
