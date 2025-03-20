"""Subcommand to convert a python file to the equivalent const char * in C."""

from __future__ import annotations

import sys
from typing import TYPE_CHECKING

from elpekenin_userspace import args
from elpekenin_userspace.commands import BaseCommand

if TYPE_CHECKING:
    from argparse import ArgumentParser, Namespace
    from pathlib import Path


class Py2C(BaseCommand):
    """Generate C-string equivalent of a .py file."""

    @classmethod
    def add_args(cls, parser: ArgumentParser) -> None:
        """Command-specific arguments."""
        parser.add_argument(
            "file",
            help="the python file to be converted",
            metavar="FILE",
            type=args.File(require_existence=True),
        )
        return super().add_args(parser)

    def run(self, arguments: Namespace) -> int:
        """Entrypoint."""
        file: Path = arguments.file
        code = file.read_text().replace('"', '\\"').replace("\n", "\\n")

        sys.stdout.write(
            f'static const char {file.stem}[] = "{code}";\n',
        )

        return 0
