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
import sys
from pathlib import Path

# make `scripts` and `qmk` visible and import them
QMK = Path(__file__).parent

sys.path.append(str(QMK / "users" / "elpekenin" / "scripts"))
import utils  # noqa: E402  # depends on sys.path hacking

sys.path.append(str(QMK / "lib" / "python"))
from qmk import keycodes  # noqa: E402  # depends on sys.path hacking

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


if __name__ == "__main__":
    # -- Handle args
    if len(sys.argv) < 3:  # noqa: PLR2004  # executable, output path, keymap path
        msg = (
            f"{utils.CLI_ERROR} {utils.filename(__file__)}"
            " <output_path> <keymap_path>"
        )
        raise SystemExit(msg)

    output_dir = Path(sys.argv[1])
    if not utils.dir_exists(output_dir):
        msg = f"Invalid path {output_dir}"
        raise SystemExit(msg)

    keymap_file = Path(sys.argv[2]) / "keymap.c"
    if not utils.file_exists(keymap_file):
        msg = f"Invalid keymap {keymap_file}"
        raise SystemExit(msg)

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

    with (output_dir / f"{OUTPUT_NAME}.c").open("w") as f:
        f.write(
            C_FILE.format(
                qmk_data=qmk_data,
                keymap_data=keymap_data,
            ),
        )

    with (output_dir / f"{OUTPUT_NAME}.h").open("w") as f:
        f.write(H_FILE)
