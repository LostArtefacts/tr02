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


def assembly_sections(
    obj: coff.Object, symbols: dict[str, tuple[int, int]]
) -> set[int]:
    sections: set[int] = set()
    for number, section in enumerate(obj.sections, 1):
        if not section.flags & coff.CODE or not section.data:
            continue
        functions = [
            symbol for symbol in obj.externals() if symbol.section == number
        ]
        entries = [symbol for symbol in functions if symbol.value == 0]
        if len(entries) != 1:
            raise ValueError(("Assembly section needs one entry point", number))
        entry = entries[0]
        if entry.name not in symbols or any(
            symbol.name not in symbols
            or symbols[symbol.name][0] - symbols[entry.name][0] != symbol.value
            for symbol in functions
        ):
            raise ValueError(("Assembly entry point layout", number))
        sections.add(number)
    return sections


def assembly_symbol_order(
    obj: coff.Object, sections: set[int], symbols: bytearray
) -> dict[int, int]:
    indices: dict[int, int] = {}
    for number in sections:
        entries = [
            index
            for index, symbol in enumerate(obj.symbols)
            if symbol
            and symbol.section == number
            and symbol.name != obj.sections[number - 1].name
        ]
        entry = next(
            index for index in entries
            if obj.symbols[index].storage == 2 and obj.symbols[index].value == 0
        )
        first = entries[0]
        if first == entry:
            continue
        if any(symbols[index * 18 + 17] for index in (first, entry)):
            raise ValueError(("Assembly entry auxiliary record", number))
        symbols[first * 18 : (first + 1) * 18], symbols[entry * 18 : (entry + 1) * 18] = (
            symbols[entry * 18 : (entry + 1) * 18],
            symbols[first * 18 : (first + 1) * 18],
        )
        indices[first] = entry
        indices[entry] = first
    return indices


def exception_order(
    objects: dict[str, coff.Object], pieces: list[Piece]
) -> dict[str, dict[int, bytes]]:
    owners = {(piece.path, piece.section): piece.address for piece in pieces}
    handlers: list[tuple[int, str, int]] = []
    for path, obj in objects.items():
        for number, section in enumerate(obj.sections, 1):
            if section.name != ".text$x" or not section.data:
                continue
            parents = {
                owners[path, parent]
                for parent, code in enumerate(obj.sections, 1)
                if (path, parent) in owners
                and any(
                    obj.symbols[index].section == number
                    for _, index, _ in code.relocs
                )
            }
            if len(parents) != 1:
                raise ValueError(("Exception section owner", path, number, parents))
            handlers.append((parents.pop(), path, number))
    if len(handlers) > 16:
        raise ValueError(("Too many exception sections", len(handlers)))
    names: dict[str, dict[int, bytes]] = {}
    for index, (_, path, number) in enumerate(sorted(handlers)):
        names.setdefault(path, {})[number] = f".text$x{index:x}".encode()
    return names


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
        "exe_fullscreen": {"_$E2": 0x444C60, "_$E1": 0x444C70},
        "specific_winmain2": {
            "??1SE_Exception@@QAE@XZ": 0x44E7A0,
            "??0SE_Exception@@QAE@AAV0@@Z": 0x44E940,
        },
        "specific_winmain3": {
            "??1SE_Exception@@QAE@XZ": 0x44E7A0,
            "??0SE_Exception@@QAE@AAV0@@Z": 0x44E940,
        },
    }

    ordered = build.link_order(objects, build.read_symbols(), "median")
    paths: dict[str, str] = {
        str(Path(obj).stem): src for src, obj in build.sources(build.EXE)
    }
    pieces: list[Piece] = []
    parsed: dict[str, coff.Object] = {}
    native: dict[str, set[int]] = {}
    unknown: list[tuple[str, int, list[str]]] = []
    selected: set[str] = set()
    for path in ordered:
        obj = coff.Object(path)
        parsed[path] = obj
        src = paths.get(Path(path).stem, "")
        rel = str(Path(src).relative_to("src")) if src else ""
        if rel.startswith("exe/") and src.lower().endswith(build.ASM_EXTS):
            native[path] = assembly_sections(obj, symbols)
        for secnum, section in enumerate(obj.sections, 1):
            if (
                not section.flags & coff.CODE
                or not len(section.data)
                or not (section.flags & 0x1000 or secnum in native.get(path, set()))
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
                if secnum in native.get(path, set()):
                    raise ValueError(("Unmapped assembly entry point", path, secnum))
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
    exception_names = exception_order(parsed, unique)

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
        indices = assembly_symbol_order(obj, native.get(path, set()), symtab)
        for secnum in range(1, nsec + 1):
            at = 20 + (secnum - 1) * 40
            if secnum in native.get(path, set()):
                name = b".text\0\0\0"
                headers[at : at + 8] = name
                flags = struct.unpack_from("<I", headers, at + 36)[0]
                struct.pack_into("<I", headers, at + 36, flags | 0x1000)
                found = False
                for index, sym in enumerate(obj.symbols):
                    if (
                        sym
                        and sym.section == secnum
                        and sym.storage == 3
                        and sym.name == obj.sections[secnum - 1].name
                    ):
                        found = symtab[index * 18 + 17] == 1
                        if not found:
                            break
                        symtab[index * 18 : index * 18 + 8] = name
                        symtab[(index + 1) * 18 + 14] = 2
                if not found:
                    raise ValueError(("Assembly section auxiliary record", path, secnum))
            if secnum in exception_names.get(path, {}):
                name = exception_names[path][secnum]
                headers[at : at + 8] = name
                for index, sym in enumerate(obj.symbols):
                    if (
                        sym
                        and sym.section == secnum
                        and sym.name == obj.sections[secnum - 1].name
                    ):
                        symtab[index * 18 : index * 18 + 8] = name
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
                relocs = bytearray(raw[reloff : reloff + nrel * 10])
                for index in range(nrel):
                    target = struct.unpack_from("<I", relocs, index * 10 + 4)[0]
                    if target in indices:
                        struct.pack_into("<I", relocs, index * 10 + 4, indices[target])
                body += relocs
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
