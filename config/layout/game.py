"""Place compiler function sections and preserve their COFF relocations."""

from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
import struct
import json

import build
import coff
import compare


@dataclass
class Piece:
    path: str
    section: int
    index: int
    name: str
    address: int
    size: int
    data: bytes


def place(objects: list[str]) -> list[str]:
    symbols = compare.read_symbols()
    extras: dict[str, dict[str, int]] = {
        "specific_winvid": {
            "_$E4": 0x444A40,
            "_$E2": 0x444A80,
            "_$E9": 0x444B50,
            "_$E7": 0x444BD0,
        },
        "specific_winin": {
            "_$E4": 0x447160,
            "_$E2": 0x4471A0,
            "_$E9": 0x447210,
            "_$E7": 0x447270,
        },
        "specific_ds": {
            "_$E4": 0x447B30,
            "_$E2": 0x447B70,
            "_$E9": 0x447BE0,
            "_$E7": 0x447C40,
        },
        "specific_screen": {"_$E2": 0x4484C0, "_$E1": 0x4484D0},
        "specific_setup": {"_$E2": 0x452460, "_$E1": 0x452470},
        "specific_winmain2": {"??1SE_Exception@@QAE@XZ": 0x44E7A0},
        "specific_winmain3": {"??1SE_Exception@@QAE@XZ": 0x44E7A0},
    }

    ordered = build.link_order(objects, build.read_symbols(), "median")
    paths: dict[str, str] = {
        str(Path(obj).stem): src for src, obj in build.sources(build.EXE)
    }
    pieces: list[Piece] = []
    unknown: list[tuple[str, int, list[str]]] = []
    selected: set[str] = set()
    for path in ordered:
        obj = coff.Object(path)
        src = paths.get(Path(path).stem, "")
        rel = str(Path(src).relative_to("src")) if src else ""
        for secnum, section in enumerate(obj.sections, 1):
            if (
                not section.flags & coff.CODE
                or not len(section.data)
                or not section.flags & 0x1000
            ):
                continue
            candidates = [
                (i, s)
                for i, s in enumerate(obj.symbols)
                if s
                and s.section == secnum
                and s.value == 0
                and s.name not in (section.name,)
                and s.storage in (2, 3)
            ]
            hit = next(
                (
                    (i, s, symbols[s.name][0], symbols[s.name][1])
                    for i, s in candidates
                    if s.name in symbols
                ),
                None,
            )
            if hit is None:
                hit = next(
                    (
                        (
                            i,
                            s,
                            symbols[rel + ":" + s.name][0],
                            symbols[rel + ":" + s.name][1],
                        )
                        for i, s in candidates
                        if rel + ":" + s.name in symbols
                    ),
                    None,
                )
            if hit is None:
                hit = next(
                    (
                        (i, s, extras[Path(path).stem][s.name], 0)
                        for i, s in candidates
                        if s.name in extras.get(Path(path).stem, {})
                    ),
                    None,
                )
            if hit is None:
                unknown.append((path, secnum, [s.name for i, s in candidates]))
                continue
            i, sym, addr, size = hit
            if sym.storage == 2 and sym.name in selected:
                continue
            if sym.storage == 2:
                selected.add(sym.name)
            data = section.data
            # The frontend aligns function sections with trailing NOPs.
            # Keep every byte covered by a symbol or relocation.
            floor = max(
                [s.value + 1 for s in obj.symbols if s and s.section == secnum]
                + [off + 4 for off, _, _ in section.relocs]
            )
            while len(data) > floor and data[-1] == 0x90:
                data = data[:-1]
            pieces.append(Piece(path, secnum, i, sym.name, addr, size, data))

    # Order unknown compiler helpers immediately after their preceding initializer
    # when that initializer's slot is large enough; otherwise leave the compiler's
    # ordinary placement intact.
    for path, secnum, names in unknown:
        if names and names[0].startswith("??1DISPLAY_ADAPTER_INFO"):
            obj = coff.Object(path)
            prev = max(
                (p for p in pieces if p.path == path and p.section < secnum),
                key=lambda p: p.section,
            )
            sym_index, sym = next(
                (i, s)
                for i, s in enumerate(obj.symbols)
                if s and s.section == secnum and s.storage == 2 and s.name in names
            )
            pieces.append(
                Piece(
                    path,
                    secnum,
                    sym_index,
                    sym.name,
                    prev.address + len(prev.data),
                    0,
                    obj.sections[secnum - 1].data,
                )
            )

    pieces.sort(key=lambda p: p.address)
    # Avoid ordering linker-folded aliases twice.
    seen: set[str] = set()
    unique: list[Piece] = []
    for piece in pieces:
        name = piece.name
        if name.startswith("_$"):
            name = "_layout_" + Path(piece.path).stem + "_" + name[2:]
        if name in seen:
            continue
        seen.add(name)
        unique.append(piece)

    groups: dict[int, list[Piece]] = {}
    for piece in unique:
        groups.setdefault(piece.address, []).append(piece)
    unique = [max(group, key=lambda p: len(p.data)) for group in groups.values()]

    removed: dict[str, set[int]] = {}
    aliases: dict[str, list[tuple[str, int]]] = {}
    for address, group in groups.items():
        chosen = max(group, key=lambda p: len(p.data))
        for alias in group:
            if alias is not chosen:
                if (
                    alias.data != chosen.data
                    and alias.path != "build/obj_exe/exe_forward.obj"
                ):
                    raise ValueError(("different alias", alias.name, chosen.name))
                removed.setdefault(alias.path, set()).add(alias.section)
                aliases.setdefault(chosen.path, []).append((alias.name, chosen.section))
    # The inline list destructor and its implementation are identical functions.
    path = "build/obj_exe/specific_winvid.obj"
    obj = coff.Object(path)
    dtor = next(s for s in obj.externals() if s.name == "??1DISPLAY_MODE_LIST@@QAE@XZ")
    impl = next(
        s
        for s in obj.externals()
        if s.name == "?DisplayModeListDelete@DISPLAY_MODE_LIST@@QAEXXZ"
    )
    assert obj.sections[dtor.section - 1].data == obj.sections[impl.section - 1].data
    assert (
        obj.sections[dtor.section - 1].relocs == obj.sections[impl.section - 1].relocs
    )
    removed.setdefault(path, set()).add(dtor.section)
    aliases.setdefault(path, []).append((dtor.name, impl.section))

    cursor = 0x401000
    pads: dict[str, dict[int, bytes]] = {}
    renames: dict[str, dict[int, str]] = {}
    order: list[str] = []
    missed: list[tuple[str, int, int]] = []
    for idx, piece in enumerate(unique):
        if cursor != piece.address:
            missed.append((piece.name, piece.address, cursor))
        data = piece.data
        next_addr = unique[idx + 1].address if idx + 1 < len(unique) else 0x457510
        if cursor + len(data) < next_addr:
            data += b"\x90" * (next_addr - cursor - len(data))
        pads.setdefault(piece.path, {})[piece.section] = data

        name = piece.name
        if name.startswith("_$"):
            name = "_layout_" + Path(piece.path).stem + "_" + name[2:]
            renames.setdefault(piece.path, {})[piece.index] = name
        order.append(name[1:] if name.startswith("_") else name)
        cursor += len(data)
    print(
        "PIECES", len(unique), "END", hex(cursor), "MISPLACED", len(missed), flush=True
    )
    Path("build/layout/code-moved.json").write_text(json.dumps(missed, indent=2))
    Path("build/layout/code-helpers.json").write_text(json.dumps(unknown, indent=2))

    outdir = Path("build/obj_placed")
    outdir.mkdir(exist_ok=True)
    newobjs: list[str] = []
    for path in ordered:
        raw = Path(path).read_bytes()
        nsec = struct.unpack_from("<H", raw, 2)[0]
        symptr, nsym = struct.unpack_from("<II", raw, 8)
        headers = bytearray(raw[: 20 + nsec * 40])
        symtab = bytearray(raw[symptr : symptr + nsym * 18])
        strtab = bytearray(raw[symptr + nsym * 18 :])
        for index, name in renames.get(path, {}).items():
            off = len(strtab)
            strtab += name.encode() + b"\0"
            struct.pack_into("<II", symtab, index * 18, 0, off)
            symtab[index * 18 + 16] = 2
        for name, section in aliases.get(path, []):
            off = len(strtab)
            strtab += name.encode() + b"\0"
            symtab += struct.pack("<IIIhHBB", 0, off, 0, section, 0x20, 2, 0)
            nsym += 1
        struct.pack_into("<I", headers, 12, nsym)
        struct.pack_into("<I", strtab, 0, len(strtab))
        body = bytearray()
        obj = coff.Object(path)
        for secnum in range(1, nsec + 1):
            at = 20 + (secnum - 1) * 40
            size, dataoff, reloff, lineoff, nrel, nline = struct.unpack_from(
                "<IIIIHH", raw, at + 16
            )
            data = pads.get(path, {}).get(
                secnum, raw[dataoff : dataoff + size] if dataoff else b""
            )
            if secnum in removed.get(path, set()):
                data = b""
                nrel = 0
                nline = 0
                struct.pack_into("<IIIIHH", headers, at + 16, 0, 0, 0, 0, 0, 0)
                flags = struct.unpack_from("<I", headers, at + 36)[0]
                headers[at : at + 8] = b".drectve"
                struct.pack_into("<I", headers, at + 36, 0x100A00)
                for index, sym in enumerate(obj.symbols):
                    if (
                        sym
                        and sym.section == secnum
                        and sym.name != obj.sections[secnum - 1].name
                    ):
                        struct.pack_into("<Ih", symtab, index * 18 + 8, 0, 0)
                        symtab[index * 18 + 16] = 2
                dataoff = 0
            if dataoff:
                struct.pack_into(
                    "<II", headers, at + 16, len(data), len(headers) + len(body)
                )
                body += data
            if nrel:
                struct.pack_into("<I", headers, at + 24, len(headers) + len(body))
                body += raw[reloff : reloff + nrel * 10]
            if nline:
                struct.pack_into("<I", headers, at + 28, len(headers) + len(body))
                body += raw[lineoff : lineoff + nline * 6]
            if secnum in pads.get(path, {}):
                flags = struct.unpack_from("<I", headers, at + 36)[0]
                struct.pack_into(
                    "<I", headers, at + 36, (flags & ~0x00F00000) | 0x00100000
                )
                for i, sym in enumerate(obj.symbols):
                    if (
                        sym
                        and sym.section == secnum
                        and sym.name == obj.sections[secnum - 1].name
                    ):
                        struct.pack_into("<I", symtab, (i + 1) * 18, len(data))
        struct.pack_into("<I", headers, 8, len(headers) + len(body))
        output = outdir / Path(path).name
        output.write_bytes(headers + body + symtab + strtab)
        newobjs.append(str(output))
    Path("build/layout/order.txt").write_text("\n".join(order) + "\n")
    return newobjs
