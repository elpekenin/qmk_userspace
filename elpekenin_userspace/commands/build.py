"""Replacement for qmk_build."""

from __future__ import annotations

import sys
from typing import TYPE_CHECKING, Any

try:
    from tqdm import tqdm
except ImportError:
    HAS_TQDM = False
else:
    HAS_TQDM = True

from elpekenin_userspace import args, constants
from elpekenin_userspace.build import Recipe
from elpekenin_userspace.commands import BaseCommand
from elpekenin_userspace.result import Ok, is_err

if TYPE_CHECKING:
    from argparse import ArgumentParser, Namespace

    from elpekenin_userspace.result import Result


def display(recipe: Recipe) -> Result[None, str]:
    """Show the steps in this build recipe."""
    res = recipe.get_all_operations()
    if is_err(res):
        return res

    for operation in res.ok():
        sys.stdout.write(f"{operation}\n")

    return Ok(None)


def build(recipe: Recipe) -> Result[None, str]:
    """Perform the build."""
    operations_result = recipe.get_all_operations()
    if is_err(operations_result):
        return operations_result

    operations = operations_result.ok()
    if HAS_TQDM:
        progress: Any = tqdm(
            operations,
            desc="Building",
            bar_format="[{n_fmt}/{total_fmt}] {desc}{bar}",
        )
    else:
        progress = operations

    for operation in progress:
        display = str(operation)
        if HAS_TQDM:
            progress.set_description(display)
        else:
            sys.stdout.write(display + "\n")

        res = operation.run()
        if is_err(res):
            return res

    return Ok(None)


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
            default=constants.JSON,
        )

        parser.add_argument(
            "--dry-run",
            help="do not actually run the build, just show its steps",
            action="store_true",
        )

        return super().add_args(parser)

    def run(self, arguments: Namespace) -> Result[None, str]:
        """Entrypoint."""
        res = Recipe.from_file(arguments.file)
        if is_err(res):
            return res

        recipe = res.ok()

        if arguments.dry_run:
            return display(recipe)

        return build(recipe)
