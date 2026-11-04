"""Invariants for moving linked allocations and their recorded relocations."""

from __future__ import annotations

from collections.abc import Iterable, Mapping
import struct


def require_coverage(data: bytes, base: int, ranges: Iterable[tuple[int, int]]) -> None:
    """Every nonzero initialized byte must belong to a retained allocation."""
    covered = bytearray(len(data))
    for address, size in ranges:
        start = max(0, address - base)
        end = min(len(data), address - base + size)
        if end > start:
            covered[start:end] = b"\1" * (end - start)
    missing = next(
        (index for index, value in enumerate(data) if value and not covered[index]),
        None,
    )
    if missing is not None:
        raise ValueError(f"Unmapped initialized byte at {base + missing:#x}")


def require_payload(
    before: bytes, after: bytes, relocations: Mapping[int, int]
) -> None:
    """A moved payload may change only at its recorded address operands."""
    expected = bytearray(before)
    for offset, value in relocations.items():
        if not 0 <= offset <= len(expected) - 4:
            raise ValueError(f"Relocation outside allocation: {offset:#x}")
        struct.pack_into("<I", expected, offset, value)
    if after != expected:
        mismatch = next(
            (index for index, (a, b) in enumerate(zip(after, expected)) if a != b),
            min(len(after), len(expected)),
        )
        raise ValueError(
            f"Compiled payload changed outside relocations at +{mismatch:#x}"
        )


def require_unchanged(
    before: bytes, after: bytes, allowed: Iterable[tuple[int, int]]
) -> None:
    """Preserve instructions outside relocations and explicitly placed data."""
    if len(before) != len(after):
        raise ValueError("Code section changed size during data placement")
    mask = bytearray(len(before))
    for start, size in allowed:
        lo, hi = max(0, start), min(len(mask), start + size)
        if hi > lo:
            mask[lo:hi] = b"\1" * (hi - lo)
    mismatch = next(
        (i for i, (a, b) in enumerate(zip(before, after)) if a != b and not mask[i]),
        None,
    )
    if mismatch is not None:
        raise ValueError(
            f"Instruction bytes changed outside relocations at +{mismatch:#x}"
        )
