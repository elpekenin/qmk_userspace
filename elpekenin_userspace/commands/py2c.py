"""Subcommand to convert a python file to the equivalent const char * in C."""

# ruff: noqa: N802  # non-lowercase name following stdlib inheritance

from __future__ import annotations

import ast
from typing import TYPE_CHECKING, cast

from elpekenin_userspace import args, error
from elpekenin_userspace.commands import BaseCommand

if TYPE_CHECKING:
    from argparse import ArgumentParser, Namespace
    from pathlib import Path

REPLACEMENTS: tuple[tuple[str, str], ...] = (
    ('"', '\\"'),
    ("\n", "\\n"),
)


class Reducer(ast.NodeTransformer):
    """Transformer to simplify source.

    ast.parse() already removes comments.
    """

    def visit_AnnAssign(self, node: ast.AnnAssign) -> ast.Assign | None:
        """Remove annotations."""
        # drop 'target: type'
        if node.value is None:
            return None

        # convert 'target: annotation = value' to 'target = value'
        return ast.Assign(
            targets=[node.target],
            value=node.value,
            lineno=node.lineno,
        )

    def visit_FunctionDef(self, node: ast.FunctionDef) -> ast.FunctionDef:
        """Remove type hints from function definitions."""
        # remove '-> type'
        node.returns = None

        if node.args.args:
            for arg in node.args.args:
                # convert 'arg: type' to 'arg'
                arg.annotation = None

        # without this, docstring wouldn't be removed
        # because each visit modifies a different object
        new_body = []
        for stmt in node.body:
            out = self.visit(stmt)
            if out is not None:
                new_body.append(out)

        # empty func (removed docstring) would be invalid syntax
        if not new_body:
            new_body.append(ast.Return(value=None))

        node.body = new_body
        return node

    def visit_Expr(self, node: ast.Expr) -> ast.Expr | None:
        """Remove Expr's (values not assigned to anything) if they are constants.

        This is mostly for docstrings, be careful as we don't wanna drop function calls.
        """
        if isinstance(node.value, ast.Constant):
            return None

        return node


def reduce(file: Path) -> ast.AST:
    """Given a Py file, read and simplify its ast."""
    tree = ast.parse(file.read_text())
    return cast("ast.AST", Reducer().visit(tree))


def convert_file(file: Path) -> None:
    """Convert a single file."""
    # skip .gitignore and whatnot
    if file.suffix != ".py":
        return

    tree = reduce(file)

    code = ast.unparse(tree)
    for old, new in REPLACEMENTS:
        code = code.replace(old, new)

    py = file.with_suffix(".c")
    py.write_text(
        f'static const char {file.stem}[] = "{code}";\n',
    )


def convert(path: Path) -> None:
    """Convert a file/folder."""
    if path.is_file():
        convert_file(path)
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
