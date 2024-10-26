#! /usr/bin/env python3

# Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
# SPDX-License-Identifier: GPL-2.0-or-later

"""Create a u16 -> char * mapping.

1. Get keycode information from qmk.keycodes
2. Specialize it with the keycodes and/or alises within keymap.c file

NOTE: Assumes layers written as: ``[<layer>] = LAYOUT(.*)(``. That is
  - Layers with explicit numbering/naming
  - Numbering and LAYOUT on the same line
"""

# NOTE: Has to be run from QMK's base folder

from __future__ import annotations

import re
from typing import TYPE_CHECKING

import utils
from qmk import keycodes  # depends on sys.path hacking

if TYPE_CHECKING:
    from argparse import ArgumentParser, Namespace
    from pathlib import Path


COMMENT = re.compile(r"//(.*)")
MULTI_COMMENT = re.compile(r"/\*(.*?)\*/")
LAYOUT = re.compile(r"\[(.*)\]( *)=( *)LAYOUT(.*)\(")

OUTPUT_NAME = "keycode_str"

H_FILE = utils.lines(
    utils.H_HEADER,
    "",
    "const char *get_keycode_name(uint16_t keycode);",
    "",
)

C_FILE = utils.lines(
    utils.C_HEADER,
    "",
    "#include <quantum/quantum.h>",
    "",
    '#include "elpekenin.h" // keycodes and layers',
    "",
    "static const char *keycode_names[] = {{",
    "{qmk_data}"  # intentional lack of comma
    "{keymap_data}"
    "}};",
    "",
    "const char *get_keycode_name(uint16_t keycode) {{",
    # no keycode whatsoever
    # without this, tap/hold keys seems to cause noise on keylogger
    # maybe they inject a "dummy" event with keycode = KC_NO instead of actual value (?)
    "    if (keycode == KC_NO) {{",
    "        return NULL;",
    "    }}",
    "",
    # in bounds -> index array
    "    if (keycode < ARRAY_SIZE(keycode_names)) {{",
    "        return keycode_names[keycode];",
    "    }}",
    "",
    # out of bounds -> nothing
    "    return NULL;",
    "}}",
    "",
)


def _read_file(keymap_path: Path) -> str:
    with keymap_path.open() as f:
        return f.read()


def _remove_comments(raw_file: str) -> str:
    clean = raw_file

    while res := COMMENT.search(clean):
        start, end = res.span()
        clean = clean[:start] + clean[end:]

    while res := MULTI_COMMENT.search(clean):
        start, end = res.span()
        clean = clean[:start] + clean[end:]

    return clean


def _extract_keycodes(clean_file: str) -> list[str]:
    accumulated = ""
    layers = []
    parens = 0

    layout = LAYOUT.search(clean_file)
    if layout is None:
        msg = "No layout found"
        raise SystemExit(msg)

    start = layout.start()

    for char in clean_file[start:]:
        if char == "(":
            parens += 1

        elif char == ")":
            parens -= 1

        if parens and not (parens == 1 and char == "("):
            accumulated += char

        if parens == 0:  # outsize of layout macro
            if char == "}":  # keymap array end
                break

            if accumulated:  # if gathered a layer of keycodes, store it
                accumulated = ", ".join(
                    [i.strip() for i in accumulated.replace("\n", "").split(", ")],
                )
                layers.append(accumulated)
                accumulated = ""

    return layers


def _keymap_data(layers: list[str]) -> str:
    keycodes = set()
    for layer in layers:
        for keycode in layer.split(","):
            keycodes.add(keycode.strip())

    strings = ""
    for keycode in keycodes:
        strings += f'    [{keycode}] = "{keycode}",\n'

    return strings


class Command(utils.CommandBase):
    """Logic of this script."""

    @staticmethod
    def add_args(parser: ArgumentParser) -> None:
        """Script-specific arguments."""
        parser.add_argument(
            "keymap",
            help="The keymap file to analyze",
            type=utils.file_arg,
        )

    def run(self, args: Namespace) -> int:
        """Entrypoint."""
        output_directory: Path = args.output_directory
        keymap_file: Path = args.keymap

        if keymap_file.name != "keymap.c":
            msg = "Keymap file should be a 'keymap.c'"
            raise ValueError(msg)

        # parse file
        raw = _read_file(keymap_file)
        clean = _remove_comments(raw)
        layers = _extract_keycodes(clean)

        # generate file
        spec = keycodes.load_spec("latest")
        qmk_data = ""
        for entry in spec["keycodes"].values():
            keycode = entry["key"]
            qmk_data += f'    [{keycode}] = "{keycode}",\n'

        keymap_data = _keymap_data(layers)

        with (output_directory / f"{OUTPUT_NAME}.c").open("w") as f:
            f.write(
                C_FILE.format(
                    qmk_data=qmk_data,
                    keymap_data=keymap_data,
                ),
            )

        with (output_directory / f"{OUTPUT_NAME}.h").open("w") as f:
            f.write(H_FILE)

        return 0
