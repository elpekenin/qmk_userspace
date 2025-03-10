"""All subcommands that generate code."""

from __future__ import annotations

from typing import TYPE_CHECKING

from commands import args
from commands.base import BaseCommand

if TYPE_CHECKING:
    from argparse import ArgumentParser


class CodegenCommand(BaseCommand):
    """Common logic to all commands generating code."""

    @classmethod
    def add_args(cls, parser: ArgumentParser) -> None:
        """Register codegen-specific arguments."""
        parser.add_argument(
            "--output",
            dest="output_directory",
            help="directory where to write generated files",
            metavar="DIR",
            type=args.directory,
            required=True,
        )
        return super().add_args(parser)
