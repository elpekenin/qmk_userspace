#! /usr/bin/env python3

# Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
# SPDX-License-Identifier: GPL-2.0-or-later

"""Small snippets re-used across scripts."""

from __future__ import annotations

import logging
from argparse import ArgumentTypeError
from functools import partial
from pathlib import Path
from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from argparse import ArgumentParser
    from collections.abc import Callable
    from typing import NoReturn

DEBUG_FILE = "debug.log"

logging.basicConfig(
    filename=DEBUG_FILE,
)
_logger = logging.getLogger(__name__)

# exposed to other files
debug = _logger.debug


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


def add_common_args(parser: ArgumentParser) -> None:
    """Add common arguments to scripts' parsers."""
    parser.add_argument(
        "output_directory",
        help="The directory into which to dump output",
        type=directory_arg,
    )


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


def run(main: Callable[[], int]) -> NoReturn:
    """Run a script's main logic, logging errors to file."""
    try:
        ret = main()
    except Exception as e:
        output = True

        # if plain exit call, nothing to log
        if (
            isinstance(e, SystemExit)
            and len(e.args) == 1
            and isinstance(e.args[0], int)
        ):
            output = False

        if output:
            logging.exception("Something went wrong in `main()`", exc_info=e)

        raise
    else:
        raise SystemExit(ret)
