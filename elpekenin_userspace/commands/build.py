"""Replacement for qmk_build."""

from __future__ import annotations

from argparse import ArgumentParser, Namespace
from pathlib import Path
from typing import TYPE_CHECKING

from elpekenin_userspace import args
from elpekenin_userspace.build import Recipe
from elpekenin_userspace.commands import BaseCommand

if TYPE_CHECKING:
    from argparse import ArgumentParser, Namespace


class Build(BaseCommand):
    """Build firmware based on the steps specified in a JSON file."""

    @classmethod
    def add_args(cls, parser: ArgumentParser) -> None:
        """Command-specific arguments."""
        parser.add_argument(
            "file",
            help="file describing build steps",
            metavar="FILE",
            type=args.File(require_existence=True),
            nargs="?",
            default=Path("build.json"),
        )

        parser.add_argument(
            "--dry-run",
            help="do not actually run the build, just show its steps",
            action="store_true",
        )

        return super().add_args(parser)

    def run(self, arguments: Namespace) -> int:
        """Entrypoint."""
        recipe = Recipe.from_file(arguments.file)

        if arguments.dry_run:
            recipe.display()
            return 0

        return recipe.run()
