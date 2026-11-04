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


if __name__ == "__main__":
    unittest.main()
