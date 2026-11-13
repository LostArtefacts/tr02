"""Tomb2's original-address standalone link, using compiled COFF payloads."""

from __future__ import annotations

from pathlib import Path
import struct
import sys

# build.py runs as __main__, not as a module named "build": importing
# "build" here would get a second, separately initialized copy that
# never sees main()'s feature-specific EXE/EXE_MAP.
build = sys.modules.get("build") or sys.modules["__main__"]

from . import anchors, bss, crt, data, game, image


def tail_imports() -> str:
    """Keep the runtime's imported call thunks after its compiled code."""
    source = Path(".wine/drive_c/msvc5sp3/lib/KERNEL32.LIB")
    archive = bytearray(source.read_bytes())
    position = 8
    while position < len(archive):
        size = int(archive[position + 48 : position + 58])
        start = position + 60
        if archive[start : start + 2] == b"\x4c\x01":
            count = struct.unpack_from("<H", archive, start + 2)[0]
            for index in range(count):
                at = start + 20 + index * 40
                flags = struct.unpack_from("<I", archive, at + 36)[0]
                if flags & 0x20:
                    archive[at : at + 8] = b".text$d\0"
        position += 60 + size + (size & 1)
    output = Path("build/obj_placed/kernel32-layout.lib")
    output.write_bytes(archive)
    return build.win(str(output))


def link(objects: list[str], expand: bool = False) -> None:
    """Resolve archive selection, then apply the recovered code and data layout.

    `expand`, for a feature build whose C twins outgrow the original
    assembly they replace, lets the code that follows the recovered
    layout move: `game.place` no longer requires every function to land
    at its original address, and the data, bss and runtime that come
    after shift by however far the actual code grew."""
    Path("build/layout").mkdir(exist_ok=True)
    Path("build/layout/objects.txt").write_text("\n".join(objects) + "\n")
    old_exe, old_map = build.EXE, build.EXE_MAP
    options = build.EXE_CONFIG
    old_flags = options["ldflags"]
    old_libs = options["libs"]
    old_order = options["link_order"]
    try:
        bss.recover()
        # The game code ends well before the CRT code recovered after it
        # starts, at 0x459030: an expanded build only actually displaces
        # anything once it grows past that gap, whether from the game
        # code itself or from CRT code the feature pulls in that the
        # original never needed and this can't place at a recovered
        # address (crt.merge leaves that to the ordinary linker instead,
        # growing .text further still). The real growth is only known
        # once that's linked, so this retries with a corrected shift
        # until the actual .text size stops moving. Each attempt redoes
        # game.place and bss.place, since the mutations below (anchors,
        # crt.merge) are not safe to repeat on their own output.
        import compare_exe as ce
        original = ce.PE(ce.ORIGINAL)
        original_text = original.sections[".text"][1]
        original_data_va = original.sections[".data"][0]
        # `shift`, how far the live content inside .text grew, moves
        # anything the recovery still finds by address within .text.
        # `dshift`, how far that growth actually pushed .data's start
        # (zero unless it crossed a section-alignment boundary), moves
        # the globals and runtime state pinned to absolute addresses
        # there instead.
        shift = dshift = 0
        for attempt in range(5):
            placed, cursor = game.place(objects)
            if expand and attempt == 0:
                shift = max(0, cursor - 0x459030)
            bss_placed = bss.place(placed, dshift, expand)
            options["link_order"] = "source"
            options["ldflags"] = [
                *old_flags,
                "/ORDER:@build\\layout\\order.txt",
                "/OPT:NOICF",
            ]
            build.EXE = "build/layout/preliminary.exe"
            build.EXE_MAP = "build/layout/preliminary.map"
            build.link_exe(bss_placed, customize=False)
            runtime, code = crt.merge(dshift, expand)
            # merged.obj's recovered CRT code is padded assuming it starts
            # immediately after the game code; anything crt.merge left
            # for the ordinary linker (unplaced new CRT code) must come
            # after it instead of in between.
            final_objects = [*bss_placed, code, *runtime]
            anchors.add(final_objects)
            kernel = tail_imports()
            options["libs"] = [
                kernel if name.lower() == "kernel32.lib" else name
                for name in old_libs
                if name.lower() != "libc.lib"
            ]
            options["ldflags"] = [
                flag for flag in options["ldflags"] if flag.upper() != "/FIXED"
            ]
            options["ldflags"] += ["/NODEFAULTLIB:libc", "/FIXED:NO"]
            build.EXE = "build/layout/linked.exe"
            build.EXE_MAP = "build/layout/linked.map"
            build.link_exe(final_objects, customize=False)
            linked = ce.PE(build.EXE)
            actual_shift = linked.sections[".text"][1] - original_text
            actual_dshift = linked.sections[".data"][0] - original_data_va
            if not expand or (actual_shift == shift and actual_dshift == dshift):
                break
            shift, dshift = actual_shift, actual_dshift
        data.prepare(shift)
        image.finish(dshift, old_exe, old_map, expand)
    finally:
        build.EXE, build.EXE_MAP = old_exe, old_map
        options["ldflags"] = old_flags
        options["libs"] = old_libs
        options["link_order"] = old_order
