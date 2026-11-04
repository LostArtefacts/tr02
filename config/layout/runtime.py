"""Compile recovered runtime routines and retain the archive's data bindings."""

from __future__ import annotations

import re

import build
import coff


def replace_code(
    original: coff.Object, replacement: coff.Object, functions: tuple[str, ...]
) -> None:
    """Bind compiler relocations to the archive's public and private symbols."""
    symbols = {
        re.sub(r"\$S\d+$", "", sym.name): index
        for index, sym in enumerate(original.symbols)
        if sym and sym.storage in (2, 3)
    }
    for name in functions:
        target = original.symbols[symbols[name]]
        source = next(sym for sym in replacement.externals() if sym.name == name)
        assert target and target.section > 0 and source.section > 0
        if target.value or source.value:
            raise ValueError(f"Runtime replacement must cover a whole section: {name}")
        section = replacement.sections[source.section - 1]
        relocations: list[tuple[int, int, int]] = []
        for offset, index, kind in section.relocs:
            symbol = replacement.symbols[index]
            assert symbol
            if symbol.name not in symbols:
                raise ValueError(
                    f"Unresolved replacement runtime symbol: {symbol.name}"
                )
            relocations.append((offset, symbols[symbol.name], kind))
        destination = original.sections[target.section - 1]
        destination.data = section.data
        destination.relocs = relocations


def mbctype() -> coff.Object:
    output = "build/obj_crt/mbctype-recovered.obj"
    build.tool(
        "cl",
        "/c",
        "/O2",
        "/G5",
        "/Gy",
        "/WX",
        "/Fo" + build.win(output),
        "config\\runtime\\mbctype.c",
    )
    return coff.Object(output)
