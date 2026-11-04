"""Runtime bindings must come from matching code and zero initialized storage."""

from __future__ import annotations

from pathlib import Path
import struct
import sys
from tempfile import TemporaryDirectory
from typing import cast
import unittest
from unittest.mock import Mock

ROOT = Path(__file__).resolve().parents[3]
sys.path.insert(0, str(ROOT / "tools"))
sys.path.insert(0, str(ROOT))

import coff
import compare_exe as ce
from config.layout.crt import Block, recover_bss
from config.layout.runtime import replace_code


class RuntimeBindingTests(unittest.TestCase):
    def setUp(self) -> None:
        directory = TemporaryDirectory()
        self.addCleanup(directory.cleanup)
        self.path = str(Path(directory.name) / "runtime.obj")
        self.code = b"\xa1" + struct.pack("<I", 8) + b"\xc3"
        coff.write_object(
            self.path,
            [
                (".text", coff.TEXT, self.code, [(1, "_state", coff.DIR32)]),
                (".bss", 0xC0300080, bytes(16), []),
            ],
            {"_probe": (1, 0), "_state": (2, 4)},
            [],
            {},
        )
        self.obj = coff.Object(self.path)
        self.block = Block(self.path, 1, 0x1000, self.code)

    def original(
        self,
        targets: dict[int, int],
        opcode: bytes = b"\xa1",
        initialized: bool = False,
    ) -> ce.PE:
        original = Mock(spec=ce.PE)
        original.sections = {".data": (0x2000, 0x1000, 0x400, 0x20)}

        def read(address: int, size: int) -> bytes:
            if address in targets:
                return opcode + struct.pack("<I", targets[address]) + b"\xc3"
            return (b"X" if initialized else b"\0") * size

        original.read.side_effect = read
        return cast(ce.PE, original)

    def test_addend_and_symbol_offset_recover_whole_bss_section(self) -> None:
        bindings = recover_bss(
            {self.path: self.obj}, [self.block], self.original({0x1000: 0x240C})
        )
        index = next(i for i, s in enumerate(self.obj.symbols) if s.name == "_state")
        self.assertEqual(bindings[(self.path, index)], 0x2404)

    def test_differing_instruction_cannot_supply_an_address(self) -> None:
        bindings = recover_bss(
            {self.path: self.obj},
            [self.block],
            self.original({0x1000: 0x240C}, opcode=b"\xa3"),
        )
        self.assertEqual(bindings, {})

    def test_inconsistent_references_are_rejected(self) -> None:
        with self.assertRaisesRegex(ValueError, "Conflicting CRT BSS"):
            recover_bss(
                {self.path: self.obj},
                [self.block, Block(self.path, 1, 0x1100, self.code)],
                self.original({0x1000: 0x240C, 0x1100: 0x250C}),
            )

    def test_entire_allocation_must_fit_original_data(self) -> None:
        with self.assertRaisesRegex(ValueError, "outside original data"):
            recover_bss(
                {self.path: self.obj}, [self.block], self.original({0x1000: 0x3004})
            )

    def test_binding_cannot_overwrite_initialized_contents(self) -> None:
        with self.assertRaisesRegex(ValueError, "overlaps initialized contents"):
            recover_bss(
                {self.path: self.obj},
                [self.block],
                self.original({0x1000: 0x200C}, initialized=True),
            )

    def test_initialized_definition_overrides_common_declaration(self) -> None:
        definition = str(Path(self.path).with_name("definition.obj"))
        common = str(Path(self.path).with_name("common.obj"))
        coff.write_object(
            definition,
            [(".data", 0xC0300040, b"DATA", [])],
            {"_state": (1, 0)},
            [],
            {},
        )
        coff.write_object(
            common,
            [(".text", coff.TEXT, self.code, [(1, "_state", coff.DIR32)])],
            {"_state": (0, 4)},
            [],
            {},
        )
        bindings = recover_bss(
            {definition: coff.Object(definition), common: coff.Object(common)},
            [Block(common, 1, 0x1000, self.code)],
            self.original({0x1000: 0x2008}, initialized=True),
        )
        self.assertEqual(bindings, {})

    def pointer_table(self, tag: bytes = b"DATA") -> tuple[coff.Object, ce.PE]:
        coff.write_object(
            self.path,
            [
                (".text", coff.TEXT, self.code, [(1, "_table", coff.DIR32)]),
                (
                    ".data",
                    coff.RDATA,
                    b"DATA" + bytes(8),
                    [(4, "_state", coff.DIR32), (8, "_buffer", coff.DIR32)],
                ),
                (".bss", 0xC0300080, bytes(16), []),
            ],
            {"_probe": (1, 0), "_table": (2, 0), "_state": (3, 4), "_buffer": (0, 32)},
            [],
            {},
        )
        original = Mock(spec=ce.PE)
        original.sections = {".data": (0x2000, 0x1000, 0x400, 0x20)}
        original.section_of.return_value = ".data"

        def read(address: int, size: int) -> bytes:
            if address == 0x1000:
                return b"\xa1" + struct.pack("<I", 0x2008) + b"\xc3"
            if address == 0x2000:
                return tag + struct.pack("<II", 0x2404, 0x2500)
            return bytes(size)

        original.read.side_effect = read
        return coff.Object(self.path), cast(ce.PE, original)

    def test_initialized_pointers_recover_bss_and_common_storage(self) -> None:
        obj, original = self.pointer_table()
        bindings = recover_bss({self.path: obj}, [self.block], original)
        named = {
            obj.symbols[index].name: address for (_, index), address in bindings.items()
        }
        self.assertEqual(named, {"_state": 0x2404, "_buffer": 0x2500})

    def test_differing_initialized_table_cannot_supply_addresses(self) -> None:
        obj, original = self.pointer_table(tag=b"FAIL")
        self.assertEqual(recover_bss({self.path: obj}, [self.block], original), {})


class RuntimeReplacementTests(unittest.TestCase):
    def setUp(self) -> None:
        directory = TemporaryDirectory()
        self.addCleanup(directory.cleanup)
        original = str(Path(directory.name) / "original.obj")
        replacement = str(Path(directory.name) / "replacement.obj")
        coff.write_object(
            original,
            [
                (".data", coff.RDATA, bytes(8), []),
                (".text", coff.TEXT, b"OLD", []),
                (".text", coff.TEXT, b"\xc3", []),
            ],
            {"_table$S123": (1, 4), "_routine": (2, 0), "_helper": (3, 0)},
            [],
            {},
        )
        self.payload = b"\xb8" + bytes(4) + b"\xe8" + bytes(4) + b"\xc3"
        coff.write_object(
            replacement,
            [
                (
                    ".text",
                    coff.TEXT,
                    self.payload,
                    [(1, "_table", coff.DIR32), (6, "_helper", coff.REL32)],
                )
            ],
            {"_routine": (1, 0)},
            ["_helper", "_table"],
            {},
        )
        self.original = coff.Object(original)
        self.original.symbols[0].storage = 3
        self.replacement = coff.Object(replacement)

    def test_compiled_code_reuses_private_archive_data_and_helper_symbols(self) -> None:
        replace_code(self.original, self.replacement, ("_routine",))
        section = self.original.sections[1]
        self.assertEqual(section.data, self.payload)
        self.assertEqual(section.relocs, [(1, 0, coff.DIR32), (6, 2, coff.REL32)])
        self.assertEqual(self.original.symbols[0].value, 4)
        self.assertEqual(self.original.sections[0].data, bytes(8))

    def test_missing_archive_binding_is_rejected(self) -> None:
        self.original.symbols[0].name = "_unrelated"
        with self.assertRaisesRegex(
            ValueError, "Unresolved replacement runtime symbol"
        ):
            replace_code(self.original, self.replacement, ("_routine",))


if __name__ == "__main__":
    unittest.main()
