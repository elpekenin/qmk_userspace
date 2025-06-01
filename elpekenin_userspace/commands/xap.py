"""Subcommand to generate bindings for qmk_xap."""

from __future__ import annotations

import argparse
import os
import shutil
import subprocess
from typing import TYPE_CHECKING

from elpekenin_userspace import args
from elpekenin_userspace.commands import BaseCommand
from elpekenin_userspace.result import Err, Ok

if TYPE_CHECKING:
    from argparse import ArgumentParser, Namespace
    from pathlib import Path

    from elpekenin_userspace.result import Result


TEMPLATE = """\
{{
    version: 0.3.0
    routes: {{
        0x3: {file}
    }}
}}
"""


class Xap(BaseCommand):
    """Create bindings for qmk_xap client."""

    @classmethod
    def add_args(cls, parser: ArgumentParser) -> None:
        """Command-specific arguments."""
        parser.add_argument(
            "--file",
            help="specification file",
            metavar="FILE",
            required=True,
            default=argparse.SUPPRESS,
            type=args.File(
                require_existence=True,
                suffix=".hjson",
            ),
        )

        parser.add_argument(
            "--folder",
            help="location of qmk_xap",
            metavar="DIR",
            required=False,
            default="$HOME/qmk_xap",
            type=args.Directory(require_existence=True),
        )

        return super().add_args(parser)

    def run(self, arguments: Namespace) -> Result[None, str]:
        """Entrypoint."""
        cargo = shutil.which("cargo")
        if cargo is None:
            msg = "cargo not found"
            return Err(msg)

        file: Path = arguments.file
        folder: Path = arguments.folder

        # NOTE: **must** start with `xap` and be a `hjson` file
        generated = folder / "xap-specs" / "assets" / "xap_user.hjson"
        with generated.open("w") as f:
            f.write(
                TEMPLATE.format(
                    file=file.read_text(),
                ),
            )

        subprocess.run(
            [
                cargo,
                "build",
                *("--bin", "xap-specs-codegen"),
            ],
            cwd=folder,
            check=True,
        )

        exe = folder / "target" / "debug" / "xap-specs-codegen"
        subprocess.run(
            [
                exe,
                "--format",
            ],
            cwd=folder,
            env=os.environ | {"RUST_LOG": "info"},
            check=True,
        )

        return Ok(None)
