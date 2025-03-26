"""Replacement for qmk_build."""

from __future__ import annotations

import sys
from pathlib import Path
from typing import TYPE_CHECKING

from tqdm import tqdm

from elpekenin_userspace import args
from elpekenin_userspace.build import Recipe
from elpekenin_userspace.commands import BaseCommand

if TYPE_CHECKING:
    from argparse import ArgumentParser, Namespace


def display(recipe: Recipe) -> None:
    """Show the steps in this build recipe."""
    for operation in recipe.get_all_operations():
        sys.stdout.write(f"{operation}\n")


def build(recipe: Recipe) -> int:
    """Perform the build."""
    # consume generator so that tqdm can display progress
    operations = list(recipe.get_all_operations())

    with tqdm(
        operations,
        desc="Building",
        bar_format="[{n_fmt}/{total_fmt}] {desc}{bar}",
    ) as progress:
        for operation in progress:
            progress.set_description(str(operation))

            ret = operation.run()
            if ret != 0:
                return ret

    return 0


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
            display(recipe)
            return 0

        return build(recipe)
