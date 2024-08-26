#! /usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
# SPDX-License-Identifier: GPL-2.0-or-later

"""
Small snippets re-used across scripts.
"""

from pathlib import Path

if __name__ == "__main__":
    print("Dont try to run this")
    exit(1)


import os
from functools import partial


def lines(*args) -> str:
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


def current_filename(path: str) -> str:
    return os.path.basename(path)


def dir_exists(path: Path) -> bool:
    return path.exists() and path.is_dir()


def file_exists(path: Path) -> bool:
    return path.exists() and path.is_file()


def debug(*args, **kwargs):
    with open("debug.log", "a") as f:
        print(*args, **kwargs, file=f)


__all__ = [
    "CLI_ERROR",
    "C_HEADER",
    "H_HEADER",
    "MK_HEADER",
    "current_filename",
    "debug",
    "dir_exists",
    "file_exists",
    "lines",
]
