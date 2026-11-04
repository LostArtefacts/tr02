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

The recompiled runtime has private zero initialized state in an additional
`.crt_bss` section. This consumes virtual memory and has no file payload.
The original five sections retain their addresses, sizes and file offsets.
The result remains a fixed-base executable.

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

The tests include the 95% whole-file threshold, import slot identities,
section layout, private runtime state and negative relocation checks. The
function-level comparison remains `just compare`.

TOMB21's surviving game objects are mainly PlayStation LNK objects. Their
debug records help establish header relationships and declarations; the
standalone PE link uses the current Win32 COFF objects and VC5 runtime members.
