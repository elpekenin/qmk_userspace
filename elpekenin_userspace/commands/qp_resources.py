"""Subcommand to do QP-related codegen."""

from __future__ import annotations

from typing import TYPE_CHECKING

from elpekenin_userspace import args
from elpekenin_userspace.codegen import C_HEADER, H_HEADER, MK_HEADER
from elpekenin_userspace.commands import CodegenCommand
from elpekenin_userspace.result import Ok

if TYPE_CHECKING:
    from argparse import ArgumentParser, Namespace
    from pathlib import Path

    from elpekenin_userspace.result import Result

    AssetsDictT = dict[str, list[Path]]

OUTPUT_NAME = "qp_resources"


def find_assets_impl(assets: AssetsDictT, path: Path) -> None:
    """Collect all assets in the given directory."""
    folder = path.absolute() / "painter"

    # no `/painter` subfolder in here, move on
    if not folder.exists() and folder.is_dir():
        return

    fonts = list(folder.glob("fonts/*qff.h"))
    images = list(folder.glob("images/*qgf.h"))

    assets["fonts"].extend(fonts)
    assets["images"].extend(images)


def find_assets(paths: list[Path]) -> AssetsDictT:
    """Collect all assets in the given list of directories."""
    assets: AssetsDictT = {"fonts": [], "images": []}

    for path in paths:
        find_assets_impl(assets, path)

    return {key: sorted(paths, key=lambda x: x.name) for key, paths in assets.items()}


def asset_name(type_: str, path: Path) -> str:
    """Compute name of asset in a file."""
    type_ = "font" if type_ == "fonts" else "gfx"
    name = path.name.replace("-", "_").replace(".qff.h", "").replace(".qgf.h", "")
    return f"{type_}_{name}"


def gen_h_file(file: Path, assets: AssetsDictT) -> None:
    """Create contents for H file."""
    with file.open("w") as f:
        f.writelines(
            [
                H_HEADER,
                "\n",
            ],
        )

        for key, paths in assets.items():
            f.writelines(
                [
                    f"// {key}\n",
                    *(f'#include "{path.name}"\n' for path in paths),
                    "\n",
                ],
            )

        f.write("void load_qp_resources(void);\n")


def gen_c_file(file: Path, assets: AssetsDictT) -> None:
    """Create contents for C file."""
    with file.open("w") as f:
        f.writelines(
            [
                C_HEADER + "\n",
                "\n",
                f'#include "generated/{OUTPUT_NAME}.h"\n',
                "\n",
                '#include "elpekenin/qp/assets.h"\n',  # set_{font,image}_by_name
                "\n",
                "void load_qp_resources(void) {",
            ],
        )

        for key, paths in assets.items():
            load = "qp_load_font_mem" if key == "fonts" else "qp_load_image_mem"
            store = "qp_set_font_by_name" if key == "fonts" else "qp_set_image_by_name"

            f.writelines(
                [
                    "\n",
                    f"    // {key}\n",
                ],
            )

            for p in paths:
                asset = asset_name(key, p)
                name = asset.removeprefix("font_").removeprefix("gfx_")
                f.write(
                    f'    {store}("{name}", (void *){load}({asset}));\n',
                )

        f.write("}\n")


def gen_mk_file(file: Path, assets: AssetsDictT) -> None:
    """Create contents for MK file."""
    with file.open("w") as f:
        f.writelines(
            [
                MK_HEADER + "\n",
                "\n",
            ],
        )

        for key, paths in assets.items():
            f.writelines(
                [
                    f"# {key}",
                    *(
                        f"\nSRC += {full_path}\nVPATH += {full_path.parent}\n"
                        for full_path in (p.with_suffix(".c").resolve() for p in paths)
                    ),
                ],
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

    def run(self, arguments: Namespace) -> Result[None, str]:
        """Entrypoint."""
        output_directory: Path = arguments.output_directory
        assets = find_assets(arguments.directories)

        gen_h_file(output_directory / f"{OUTPUT_NAME}.h", assets)
        gen_c_file(output_directory / f"{OUTPUT_NAME}.c", assets)
        gen_mk_file(output_directory / f"{OUTPUT_NAME}.mk", assets)

        return Ok(None)
