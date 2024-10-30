#! /usr/bin/env python3

# Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
# SPDX-License-Identifier: GPL-2.0-or-later

"""Small snippets re-used across scripts."""

from __future__ import annotations

from argparse import ArgumentTypeError
from functools import partial
from pathlib import Path
from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from argparse import ArgumentParser, Namespace

LOG_FILE = "python.txt"


def lines(*args: str) -> str:
    """Create a multi-line string based on input lines."""
    return "\n".join([*args])


__HEADER = lines(
    "{comment} THIS FILE WAS GENERATED",
    "",
    "{comment} Copyright {year} Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>",
    "{comment} SPDX-License-Identifier: GPL-2.0-or-later",
)
_HEADER = partial(__HEADER.format, year="2023-2024")
C_HEADER = _HEADER(comment="//")
H_HEADER = f"{C_HEADER}\n\n#pragma once"
MK_HEADER = _HEADER(comment="#")


def _repr(path: Path) -> str:
    return f"'{path.resolve()}'"


def exists(path: Path) -> None:
    """Check if the path exists, raise otherwise."""
    if not path.exists():
        msg = _repr(path) + " does not exist"
        raise ArgumentTypeError(msg)


def file_arg(raw: str) -> Path:
    """Conversion function for and argument expected to be a file."""
    path = Path(raw)
    exists(path)

    if not path.is_file():
        msg = _repr(path) + " is not a file"
        raise ArgumentTypeError(msg)

    return path


def directory_arg(raw: str) -> Path:
    """Conversion function for and argument expected to be a directory."""
    path = Path(raw)
    exists(path)

    if not path.is_dir():
        msg = _repr(path) + " is not a directory"
        raise ArgumentTypeError(msg)

    return path


class ScriptBase:
    """Represent a script."""

    @staticmethod
    def add_args(parser: ArgumentParser) -> None:
        """For scripts to add their specific arguments."""

    def run(self, args: Namespace) -> int:
        """Logic of script, return exitcode."""
        raise NotImplementedError
