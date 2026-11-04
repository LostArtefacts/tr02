"""Exception helpers follow their owning functions across object boundaries."""

from __future__ import annotations

from pathlib import Path
from tempfile import TemporaryDirectory
import sys
import unittest

ROOT = Path(__file__).resolve().parents[3]
sys.path.insert(0, str(ROOT / "tools"))
sys.path.insert(0, str(ROOT))

import coff
from config.layout.game import (
    Piece,
    assembly_sections,
    assembly_symbol_order,
    exception_order,
)


class AssemblySectionTests(unittest.TestCase):
    def make_object(self, definitions: dict[str, tuple[int, int]]) -> coff.Object:
        directory = TemporaryDirectory()
        self.addCleanup(directory.cleanup)
        path = str(Path(directory.name) / "patch.obj")
        coff.write_object(
            path,
            [(".text", coff.TEXT, b"\x90\xc3", []),
             (".data", coff.RDATA, b"text", [])],
            definitions, [], {},
        )
        return coff.Object(path)

    def test_one_function_per_code_section(self) -> None:
        self.assertEqual(
            assembly_sections(self.make_object({"_entry": (1, 0)}), {"_entry": (0x1000, 2)}),
            {1},
        )

    def test_shared_entries_cannot_change_their_relative_addresses(self) -> None:
        with self.assertRaisesRegex(ValueError, "entry point layout"):
            assembly_sections(
                self.make_object({"_one": (1, 0), "_two": (1, 1)}),
                {"_one": (0x1000, 1), "_two": (0x1002, 1)},
            )

    def test_shared_patch_entries_keep_their_relative_addresses(self) -> None:
        self.assertEqual(
            assembly_sections(
                self.make_object({"_one": (1, 0), "_two": (1, 1)}),
                {"_one": (0x1000, 1), "_two": (0x1001, 1)},
            ),
            {1},
        )

    def test_entry_point_must_start_the_section(self) -> None:
        with self.assertRaisesRegex(ValueError, "one entry point"):
            assembly_sections(self.make_object({"_entry": (1, 1)}), {"_entry": (0x1000, 1)})

    def test_entry_precedes_private_data_in_the_comdat_symbol_table(self) -> None:
        obj = self.make_object({"_string": (1, 1), "_entry": (1, 0)})
        obj.symbols[0].storage = 3
        symbols = bytearray(36)
        symbols[:8] = b"_string\0"
        symbols[18:26] = b"_entry\0\0"
        self.assertEqual(assembly_symbol_order(obj, {1}, symbols), {0: 1, 1: 0})
        self.assertEqual(symbols[:8], b"_entry\0\0")
        self.assertEqual(symbols[18:26], b"_string\0")

    def test_symbol_reordering_rejects_auxiliary_records(self) -> None:
        obj = self.make_object({"_string": (1, 1), "_entry": (1, 0)})
        obj.symbols[0].storage = 3
        symbols = bytearray(36)
        symbols[17] = 1
        with self.assertRaisesRegex(ValueError, "entry auxiliary record"):
            assembly_symbol_order(obj, {1}, symbols)


class ExceptionOrderTests(unittest.TestCase):
    def setUp(self) -> None:
        directory = TemporaryDirectory()
        self.addCleanup(directory.cleanup)
        self.directory = Path(directory.name)

    def make_object(
        self, name: str, address: int, second_owner: bool = False
    ) -> tuple[str, coff.Object, list[Piece]]:
        path = str(self.directory / name)
        code = b"\x68" + bytes(4) + b"\xc3"
        coff.write_object(
            path,
            [
                (".text$mn", coff.TEXT, code, [(1, "_handler", coff.DIR32)]),
                (".text$x", coff.TEXT, b"\xc3", []),
                (
                    ".text$mn" if second_owner else ".rdata",
                    coff.TEXT if second_owner else coff.RDATA,
                    bytes(4),
                    [(0, "_handler", coff.DIR32)],
                ),
            ],
            {"_owner": (1, 0), "_handler": (2, 0)},
            [],
            {},
        )
        pieces = [Piece(path, 1, 0, "_owner", address, len(code), code)]
        if second_owner:
            pieces.append(Piece(path, 3, 0, "_other", address + 16, 4, bytes(4)))
        return path, coff.Object(path), pieces

    def test_owner_addresses_override_object_order(self) -> None:
        later, later_obj, later_pieces = self.make_object("a.obj", 0x2000)
        earlier, earlier_obj, earlier_pieces = self.make_object("z.obj", 0x1000)
        self.assertEqual(
            exception_order(
                {later: later_obj, earlier: earlier_obj},
                later_pieces + earlier_pieces,
            ),
            {earlier: {2: b".text$x0"}, later: {2: b".text$x1"}},
        )

    def test_metadata_reference_cannot_replace_missing_function_owner(self) -> None:
        path, obj, _ = self.make_object("orphan.obj", 0x1000)
        with self.assertRaisesRegex(ValueError, "Exception section owner"):
            exception_order({path: obj}, [])

    def test_conflicting_function_owners_are_rejected(self) -> None:
        path, obj, pieces = self.make_object("ambiguous.obj", 0x1000, True)
        with self.assertRaisesRegex(ValueError, "Exception section owner"):
            exception_order({path: obj}, pieces)


if __name__ == "__main__":
    unittest.main()
