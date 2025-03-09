"""Subcommand to codegen a keycode->name mapping."""

# NOTE: Assumes layers written as: ``[<layer>] = LAYOUT(.*)(``. That is
#   - Layers with explicit numbering/naming
#   - Numbering and LAYOUT on the same line

from __future__ import annotations

import re
from typing import TYPE_CHECKING

from commands import args
from commands.base import BaseCommand
from commands.codegen import C_HEADER, H_HEADER, lines

if TYPE_CHECKING:
    from argparse import ArgumentParser, Namespace
    from pathlib import Path


COMMENT = re.compile(r"//(.*)")
MULTI_COMMENT = re.compile(r"/\*(.*?)\*/")
LAYOUT = re.compile(r"\[(.*)\]( *)=( *)LAYOUT(.*)\(")

OUTPUT_NAME = "keycode_str"

H_FILE = lines(
    H_HEADER,
    "",
    "const char *get_keycode_name(uint16_t keycode);",
    "",
)

C_FILE = lines(
    C_HEADER,
    "",
    "#include <quantum/quantum.h>",
    "",
    '#include "elpekenin/keycodes.h"',
    '#include "elpekenin/layers.h"',
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


class KeycodeStr(BaseCommand):
    """Create a map from keycode values(u16) to their names (char*)."""

    @staticmethod
    def add_args(parser: ArgumentParser) -> None:
        """Script-specific arguments."""
        parser.add_argument(
            "keymap",
            help="the keymap file to analyze",
            type=args.file,
        )

    def run(self, arguments: Namespace) -> int:
        """Entrypoint."""
        output_directory: Path = arguments.output_directory
        keymap_file: Path = arguments.keymap

        if keymap_file.name != "keymap.c":
            msg = "Keymap file should be a 'keymap.c'"
            raise ValueError(msg)

        # parse file
        raw = keymap_file.read_text()
        clean = _remove_comments(raw)
        layers = _extract_keycodes(clean)

        # lazy import so that patching of `sys.path` can happen in
        # manage.py:main, and not its global scope (between importing sys and
        # importing this module)
        import qmk.keycodes  # type: ignore[import-not-found]

        # generate file
        spec = qmk.keycodes.load_spec("latest")
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
