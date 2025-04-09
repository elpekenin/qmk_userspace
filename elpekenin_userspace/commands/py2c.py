"""Subcommand to convert a python file to the equivalent const char * in C."""

from __future__ import annotations

from typing import TYPE_CHECKING

from elpekenin_userspace import args, error
from elpekenin_userspace.commands import BaseCommand

if TYPE_CHECKING:
    from argparse import ArgumentParser, Namespace
    from pathlib import Path

REPLACEMENTS: tuple[tuple[str, str], ...] = (
    ('"', '\\"'),
    ("\n", "\\n"),
)


def generate_c_file(file: Path) -> None:
    """Convert a single file."""
    assert file.is_file()  # noqa: S101  # defensive programming

    # skip .gitignore and whatnot
    if file.suffix != ".py":
        return

    code = file.read_text()
    for old, new in REPLACEMENTS:
        code = code.replace(old, new)

    py = file.with_suffix(".c")
    py.write_text(
        f'static const char {file.stem}[] = "{code}";\n',
    )


def convert(path: Path) -> None:
    """Convert a file/folder."""
    if path.is_file():
        generate_c_file(path)
    elif path.is_dir():
        for file in path.iterdir():
            convert(file)
    else:
        error.abort(f"Unsupported path type: '{path}'")


class Py2C(BaseCommand):
    """Generate C-string equivalent of a .py file."""

    @classmethod
    def add_args(cls, parser: ArgumentParser) -> None:
        """Command-specific arguments."""
        parser.add_argument(
            "paths",
            help="files/directories to be converted (non-python files ignored)",
            metavar="PATH",
            type=args.PathArg(require_existence=True),
            nargs="*",
        )
        return super().add_args(parser)

    def run(self, arguments: Namespace) -> int:
        """Entrypoint."""
        paths: list[Path] = arguments.paths
        if not paths:
            msg = "No path(s) provided."
            raise RuntimeError(msg)

        for path in paths:
            convert(path)

        return 0
