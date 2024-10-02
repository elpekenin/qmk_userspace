#! /usr/bin/env python3

# Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
# SPDX-License-Identifier: GPL-2.0-or-later

"""Small snippets re-used across scripts."""

from functools import partial
from pathlib import Path


def lines(*args: str) -> str:
    """Create a multi-line string based on input lines."""
    return "\n".join([*args])


CLI_ERROR = "[ERROR] Usage:"

__HEADER = lines(
    "{comment} THIS FILE WAS GENERATED",
    "",
    "{comment} Copyright {year} Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>",
    "{comment} SPDX-License-Identifier: GPL-2.0-or-later",
)
_HEADER = partial(__HEADER.format, year="2023")
C_HEADER = _HEADER(comment="//")
H_HEADER = f"{C_HEADER}\n\n#pragma once"
MK_HEADER = _HEADER(comment="#")


def filename(path: str) -> str:
    """Get filename out of a string."""
    return Path(path).name


def dir_exists(path: Path) -> bool:
    """Check if path is an existing directory."""
    return path.exists() and path.is_dir()


def file_exists(path: Path) -> bool:
    """Check if path is an existing file."""
    return path.exists() and path.is_file()


def debug(*args: object, **kwargs: object) -> None:
    """Store logging in a file."""
    with Path("debug.log").open("a") as f:
        print(*args, **kwargs, file=f)  # type: ignore[call-overload]


__all__ = [
    "CLI_ERROR",
    "C_HEADER",
    "H_HEADER",
    "MK_HEADER",
    "filename",
    "debug",
    "dir_exists",
    "file_exists",
    "lines",
]
