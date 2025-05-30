"""Subcommand to codegen a keycode->name mapping."""

# NOTE: Assumes layers written as: ``[<layer>] = LAYOUT(.*)(``. That is
#   - Layers with explicit numbering/naming
#   - Numbering and LAYOUT on the same line

# WARN: Superseded by `get_keycode_string` (aka QMK's #24787)

from __future__ import annotations

import re
from typing import TYPE_CHECKING

import elpekenin_userspace.keycodes
from elpekenin_userspace import args
from elpekenin_userspace.codegen import C_HEADER, H_HEADER
from elpekenin_userspace.commands import CodegenCommand
from elpekenin_userspace.result import Ok

if TYPE_CHECKING:
    from argparse import ArgumentParser, Namespace
    from pathlib import Path

    from elpekenin_userspace.keycodes import Spec
    from elpekenin_userspace.result import Result


OUTPUT_NAME = "keycode_str"

COMMENT = re.compile(r"//(.*)")
MULTI_COMMENT = re.compile(r"/\*(.*?)\*/")
LAYOUT = re.compile(r"\[(.*)\]( *)=( *)LAYOUT(.*)\(")

H_FILE = f"""\
{H_HEADER}
const char *get_keycode_name(uint16_t keycode);
"""


def remove_comments(raw_file: str) -> str:
    """Clean all comments from a C file."""
    clean = raw_file

    while res := COMMENT.search(clean):
        start, end = res.span()
        clean = clean[:start] + clean[end:]

    while res := MULTI_COMMENT.search(clean):
        start, end = res.span()
        clean = clean[:start] + clean[end:]

    return clean


def extract_keycodes(clean_file: str) -> list[str]:
    """Collect all keycodes in a keymap."""
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


def keymap_data(layers: list[str]) -> str:
    """Generate mapping of keycode names in the user's keymap."""
    keycodes = set()
    for layer in layers:
        for keycode in layer.split(","):
            keycodes.add(keycode.strip())

    strings = ""
    for keycode in keycodes:
        strings += f'    [{keycode}] = "{keycode}",\n'

    return strings


def gen_h_file(file: Path) -> None:
    """Create contents for H file."""
    file.write_text(H_FILE)


def gen_c_file(file: Path, keymap_file: Path, spec: Spec) -> None:
    """Create contents for C file."""
    # read file
    raw = keymap_file.read_text()
    clean = remove_comments(raw)
    layers = extract_keycodes(clean)

    # generate contents
    with file.open("w") as f:
        f.writelines(
            [
                C_HEADER + "\n",
                "\n",
                f'#include "generated/{OUTPUT_NAME}.h"\n',
                "\n",
                "#include <quantum/quantum.h>\n",
                "\n",
                '#include "elpekenin/keycodes.h"\n',
                '#include "elpekenin/layers.h"\n',
                "\n",
                "static const char *keycode_names[] = {\n",
            ],
        )

        for entry in spec["keycodes"].values():
            keycode = entry["key"]
            f.write(f'    [{keycode}] = "{keycode}",\n')

        f.write(keymap_data(layers) + "\n")

        f.writelines(
            [
                "};\n",
                "\n",
                "const char *get_keycode_name(uint16_t keycode) {\n",
                # no keycode whatsoever
                # without this, tap/hold keys seems to cause noise on keylogger
                # maybe they inject a "dummy" event with keycode = KC_NO
                # ... instead of actual value (?)
                "    if (keycode == KC_NO) {\n",
                "        return NULL;\n",
                "    }\n",
                "\n",
                # in bounds -> index array
                "    if (keycode < ARRAY_SIZE(keycode_names)) {\n",
                "        return keycode_names[keycode];\n",
                "    }\n",
                "\n",
                # out of bounds -> nothing
                "    return NULL;\n",
                "}\n",
            ],
        )


class KeycodeStr(CodegenCommand):
    """Create a map from keycode values(u16) to their names (char*)."""

    @classmethod
    def add_args(cls, parser: ArgumentParser) -> None:
        """Command-specific arguments."""
        parser.add_argument(
            "keymap",
            help="the keymap file to analyze",
            metavar="FILE",
            type=args.File(require_existence=True, name="keymap.c"),
        )
        return super().add_args(parser)

    def run(self, arguments: Namespace) -> Result[None, str]:
        """Entrypoint."""
        output_directory: Path = arguments.output_directory

        gen_h_file(output_directory / f"{OUTPUT_NAME}.h")
        gen_c_file(
            output_directory / f"{OUTPUT_NAME}.c",
            arguments.keymap,
            elpekenin_userspace.keycodes.load_spec(
                "latest",
                home=arguments.qmk,
            ),
        )

        return Ok(None)
