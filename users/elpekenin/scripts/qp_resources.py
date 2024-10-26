#! /usr/bin/env python3

# Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
# SPDX-License-Identifier: GPL-2.0-or-later

"""Automatically include on your compilation all QP assets on relevant paths.

Also provides a function to load them into memory at once.
"""

from __future__ import annotations

from functools import partial
from typing import TYPE_CHECKING

import utils

if TYPE_CHECKING:
    from argparse import ArgumentParser, Namespace
    from collections.abc import Callable
    from pathlib import Path

    AssetsDictT = dict[str, list[Path]]


OUTPUT_NAME = "qp_resources"

H_FILE = utils.lines(utils.H_HEADER, "", "{generated_code}")

C_FILE = utils.lines(
    utils.C_HEADER,
    "",
    '#include "elpekenin/qp/graphics.h"',
    "",
    "void load_qp_resources(void) {{",
    "{generated_code}"  # no comma here intentionally
    "}}",
)

MK_FILE = utils.lines(utils.MK_HEADER, "", "{generated_code}")


def _find_assets_impl(assets: AssetsDictT, path: Path) -> None:
    folder = path.absolute() / "painter"

    # no `/painter` subfolder in here, move on
    if not folder.exists() and folder.is_dir():
        return

    fonts = list(folder.glob("fonts/*qff.h"))
    images = list(folder.glob("images/*qgf.h"))

    assets["fonts"].extend(fonts)
    assets["images"].extend(images)


def _find_assets(paths: list[Path]) -> AssetsDictT:
    assets: AssetsDictT = {"fonts": [], "images": []}

    for path in paths:
        _find_assets_impl(assets, path)

    return assets


def _for_all_assets(
    func: Callable[[str, list[Path]], str],
    assets: AssetsDictT,
) -> str:
    return "\n".join(
        # sort assets by name
        func(asset_k, sorted(asset_v, key=lambda e: e.name))
        for asset_k, asset_v in assets.items()
    )


def _h_generator(key: str, paths: list[Path]) -> str:
    return utils.lines(
        f"// {key}",
        "\n".join(f'#include "{path.name}"' for path in paths),
        "",
    )


def _c_generator(key: str, paths: list[Path]) -> str:
    function = "qp_set_font_by_name" if key == "fonts" else "qp_set_image_by_name"

    def _name_generator(key: str, path: Path) -> str:
        key = "font" if key == "fonts" else "gfx"
        name = path.name.replace("-", "_").replace(".qff.h", "").replace(".qgf.h", "")

        return f"{key}_{name}"

    _lines = [f"    // {key}"]
    for path in paths:
        name = _name_generator(key, path)
        _lines.append(f'    {function}("{name}", {name});')

    return utils.lines(*_lines)


def _mk_generator(key: str, paths: list[Path]) -> str:
    return utils.lines(
        f"# {key}",
        "\n".join(f"SRC += {path}".replace(".h", ".c") for path in paths),
        "",
    )


class Command(utils.CommandBase):
    """Logic of this script."""

    @staticmethod
    def add_args(parser: ArgumentParser) -> None:
        """Script-specific arguments."""
        parser.add_argument(
            "directories",
            nargs="+",
            type=utils.directory_arg,
        )

    def run(self, args: Namespace) -> int:
        """Entrypoint."""
        output_directory: Path = args.output_directory
        directories: list[Path] = args.directories

        # Find elements
        assets = _find_assets(directories)

        # Gen files
        for_all_assets = partial(_for_all_assets, assets=assets)

        gen_h = utils.lines(
            for_all_assets(_h_generator),
            "",
            "void load_qp_resources(void);",
        )
        with (output_directory / f"{OUTPUT_NAME}.h").open("w") as f:
            f.write(H_FILE.format(generated_code=gen_h))

        gen_c = for_all_assets(_c_generator)
        with (output_directory / f"{OUTPUT_NAME}.c").open("w") as f:
            f.write(C_FILE.format(generated_code=gen_c))

        gen_mk = for_all_assets(_mk_generator)
        with (output_directory / f"{OUTPUT_NAME}.mk").open("w") as f:
            f.write(MK_FILE.format(generated_code=gen_mk))

        return 0
