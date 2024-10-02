#! /usr/bin/env python3

# Copyright 2023 Pablo Martinez (@elpekenin) <elpekenin@elpekenin.dev>
# SPDX-License-Identifier: GPL-2.0-or-later

"""Automatically include on your compilation all QP assets on relevant paths.

Also provides a function to load them into memory at once.
"""

from __future__ import annotations

import sys
from functools import partial
from pathlib import Path
from typing import Callable

import utils

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

AssetsDictT = dict[str, list[Path]]


def _find_assets_impl(assets: AssetsDictT, path: str) -> None:
    folder = Path(path).absolute() / "painter"

    if not utils.dir_exists(folder):
        return

    fonts = list(folder.glob("fonts/*qff.h"))
    images = list(folder.glob("images/*qgf.h"))

    assets["fonts"].extend(fonts)
    assets["images"].extend(images)


def _find_assets(paths: list[str]) -> AssetsDictT:
    assets: AssetsDictT = {"fonts": [], "images": []}

    for path in paths:
        _find_assets_impl(assets, path)

    return assets


def __for_all_assets(func: Callable, assets: AssetsDictT) -> str:
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


if __name__ == "__main__":
    # -- Handle args
    if len(sys.argv) < 3:  # noqa: PLR2004  # executable, output path, paths
        msg = f"{utils.CLI_ERROR} {utils.filename(__file__)} <paths...>"
        raise SystemExit(msg)

    output_dir = Path(sys.argv[1])
    if not utils.dir_exists(output_dir):
        msg = f"Invalid path {output_dir}"
        raise SystemExit(msg)

    # Find elements
    assets = _find_assets(sys.argv[2:])

    # Gen files
    _for_all_assets = partial(__for_all_assets, assets=assets)

    gen_h = utils.lines(
        _for_all_assets(_h_generator),
        "",
        "void load_qp_resources(void);",
    )
    with (output_dir / f"{OUTPUT_NAME}.h").open("w") as f:
        f.write(H_FILE.format(generated_code=gen_h))

    gen_c = _for_all_assets(_c_generator)
    with (output_dir / f"{OUTPUT_NAME}.c").open("w") as f:
        f.write(C_FILE.format(generated_code=gen_c))

    gen_mk = _for_all_assets(_mk_generator)
    with (output_dir / f"{OUTPUT_NAME}.mk").open("w") as f:
        f.write(MK_FILE.format(generated_code=gen_mk))
