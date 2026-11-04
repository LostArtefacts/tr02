"""Recover zero initialized bindings from symbols and matching references."""

from __future__ import annotations
from pathlib import Path
import json
import re
import struct
from unittest.mock import patch
import build, coff, compare, compare_exe


def recover() -> None:
    paths = Path("build/layout/objects.txt").read_text().splitlines()
    srcs = {str(Path(obj).stem): src for src, obj in build.sources(build.EXE)}
    sources = [
        (srcs[Path(path).stem], path) for path in paths if Path(path).stem in srcs
    ]
    with patch.object(build, "sources", return_value=sources):
        matched = {r[0] for r in compare.results(compare.read_symbols()) if r[1]}
    known = compare.read_symbols()
    original = compare_exe.PE(compare.EXE)
    manual: dict[str, int] = {
        "end_stats_mode": 0x4D7C24,
        "wind": 0x4D7908,
        "medi_used": 0x51A210,
        "tDebounceF3": 0x51A214,
        "tDebounceF4": 0x51A218,
        "tDebounceF7": 0x51A21C,
        "tDebounceF8": 0x51A220,
        "tDebounceF11": 0x51A224,
        "frame_comp": 0x51BC34,
    }
    result: dict[str, dict[str, int]] = {}
    for src, path in sources:
        obj = coff.Object(path)
        rel = str(Path(src).relative_to("src"))
        mapping: dict[int, int] = {}
        for secnum, sec in enumerate(obj.sections, 1):
            for sym in obj.symbols:
                if not sym or sym.section != secnum:
                    continue
                key = sym.name if sym.name in known else rel + ":" + sym.name
                if key not in matched:
                    continue
                addr, size = known[key]
                for off, index, kind in sec.relocs:
                    target = obj.symbols[index]
                    if (
                        kind != coff.DIR32
                        or target.section <= 0
                        or not obj.sections[target.section - 1].flags & 0x80
                    ):
                        continue
                    addend = struct.unpack_from("<I", sec.data, off)[0]
                    value = (
                        struct.unpack("<I", original.read(addr + off - sym.value, 4))[0]
                        - addend
                    ) & 0xFFFFFFFF
                    if index in mapping and mapping[index] != value:
                        raise ValueError((path, target.name, mapping[index], value))
                    mapping[index] = value
        for index, sym in enumerate(obj.symbols):
            if not sym:
                continue
            common = sym.section == 0 and sym.storage == 2 and sym.value > 0
            bss = (
                sym.section > 0
                and obj.sections[sym.section - 1].flags & 0x80
                and not sym.name.startswith(".")
            )
            if not common and not bss:
                continue
            if sym.name in known:
                mapping[index] = known[sym.name][0]
            elif index not in mapping:
                name = re.sub(r"\$S\d+$", "", sym.name)
                local = re.match(r"_\?([^@]+)@", name)
                if local:
                    name = "_" + local[1]
                if name in known:
                    mapping[index] = known[name][0]
                elif name.lstrip("_") in manual:
                    mapping[index] = manual[name.lstrip("_")]
                else:
                    raise ValueError(
                        f"Unresolved zero initialized symbol: {path}:{sym.name}"
                    )
            if index in mapping:
                result.setdefault(path, {})[str(index)] = mapping[index]
    Path("build/layout/bss.json").write_text(json.dumps(result, indent=2))
    print("MAPPED", sum(len(x) for x in result.values()))


def place(paths: list[str]) -> list[str]:
    mapping: dict[str, dict[str, int]] = json.loads(
        Path("build/layout/bss.json").read_text()
    )
    defined: dict[str, tuple[int, int]] = {}
    absolute: dict[str, int] = {}
    base = 0x46C2D0
    for path in paths:
        raw = bytearray(Path(path).read_bytes())
        obj = coff.Object(path)
        old = "build/obj_exe/" + Path(path).name
        symptr, nsym = struct.unpack_from("<II", raw, 8)
        old_end = symptr + nsym * 18
        strings = bytearray(raw[old_end:])
        records = bytearray(raw[symptr:old_end])
        for index, value in mapping.get(old, {}).items():
            i = int(index)
            sym = obj.symbols[i]
            assert sym
            name = sym.name
            if sym.storage == 3:
                name = "_layout_bss_" + Path(path).stem + "_" + str(i)
                off = len(strings)
                strings += name.encode() + b"\0"
                struct.pack_into("<II", records, i * 18, 0, off)
            struct.pack_into("<Ih", records, i * 18 + 8, 0, 0)
            records[i * 18 + 16] = 2
            if value >= base:
                if name in absolute and absolute[name] != value:
                    raise ValueError(f"Conflicting absolute binding for {name}")
                absolute[name] = value
            else:
                absolute[name] = value
        for n, sec in enumerate(obj.sections):
            if sec.flags & 0x80:
                at = 20 + n * 40
                struct.pack_into("<IIIIHH", raw, at + 16, 0, 0, 0, 0, 0, 0)
                raw[at : at + 8] = b".drectve"
                struct.pack_into("<I", raw, at + 36, 0x100A00)
        struct.pack_into("<I", strings, 0, len(strings))
        Path(path).write_bytes(raw[:symptr] + records + strings)
    absolute["_layout_bss_base"] = base
    defined["_layout_game_bss_start"] = (1, 0)
    defined["_layout_game_bss_end"] = (1, 0x527600 - base)
    out = "build/obj_placed/000_globals.obj"
    # The rest of the CRT's private zero initialized state follows this range.
    coff.write_object(
        out, [(".bss", 0xC0300080, bytes(0x527600 - base), [])], defined, [], absolute
    )
    return [out, *paths]
