"""Subcommand to do QP-related codegen."""

from __future__ import annotations

from functools import partial
from typing import TYPE_CHECKING

from commands import args
from commands.codegen import C_HEADER, H_HEADER, MK_HEADER, lines

from . import CodegenCommand

if TYPE_CHECKING:
    from argparse import ArgumentParser, Namespace
    from collections.abc import Callable
    from pathlib import Path

    AssetsDictT = dict[str, list[Path]]


OUTPUT_NAME = "qp_resources"

H_FILE = lines(H_HEADER, "", "{generated_code}")

C_FILE = lines(
    C_HEADER,
    "",
    '#include "elpekenin/qp/assets.h"',  # set_{font,image}_by_name
    "",
    '#include "generated/qp_resources.h"',
    "",
    "void load_qp_resources(void) {{",
    "{generated_code}"  # no comma here intentionally
    "}}",
)

MK_FILE = lines(MK_HEADER, "", "{generated_code}")


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
    return lines(
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

    return lines(*_lines)


def _mk_generator(key: str, paths: list[Path]) -> str:
    return lines(
        f"# {key}",
        "\n".join(f"SRC += {path}".replace(".h", ".c") for path in paths),
        "",
    )


class QpResources(CodegenCommand):
    """Automatically handle all QP assets."""

    @classmethod
    def add_args(cls, parser: ArgumentParser) -> None:
        """Command-specific arguments."""
        parser.add_argument(
            "directories",
            help="list of directories where to look for QP assets",
            metavar="DIR",
            type=args.Directory(require_existence=True),
            nargs="+",
        )
        return super().add_args(parser)

    def run(self, arguments: Namespace) -> int:
        """Entrypoint."""
        output_directory: Path = arguments.output_directory

        # Find elements
        assets = _find_assets(arguments.directories)

        # Gen files
        for_all_assets = partial(_for_all_assets, assets=assets)

        gen_h = lines(
            for_all_assets(_h_generator),
            "",
            "void load_qp_resources(void);",
        )
        h_file = output_directory / f"{OUTPUT_NAME}.h"
        h_file.write_text(H_FILE.format(generated_code=gen_h))

        gen_c = for_all_assets(_c_generator)
        c_file = output_directory / f"{OUTPUT_NAME}.c"
        c_file.write_text(C_FILE.format(generated_code=gen_c))

        gen_mk = for_all_assets(_mk_generator)
        mk_file = output_directory / f"{OUTPUT_NAME}.mk"
        mk_file.write_text(MK_FILE.format(generated_code=gen_mk))

        return 0
