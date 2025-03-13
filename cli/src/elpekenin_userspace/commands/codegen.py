"""Small snippets reused across scripts."""

from __future__ import annotations


def lines(*args: str) -> str:
    """Create a multi-line string based on input lines."""
    return "\n".join(args)


HEADER_TEMPLATE = lines(
    "{comment} THIS FILE WAS GENERATED",
    "",
    "{comment} Copyright Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>",
    "{comment} SPDX-License-Identifier: GPL-2.0-or-later",
)
C_HEADER = HEADER_TEMPLATE.format(comment="//")
H_HEADER = f"{C_HEADER}\n\n#pragma once"
MK_HEADER = HEADER_TEMPLATE.format(comment="#")
