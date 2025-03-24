"""Build step."""

from __future__ import annotations

from typing import TYPE_CHECKING, TypedDict

from git import Repo

import elpekenin_userspace.path
from elpekenin_userspace import error
from elpekenin_userspace.operations.base import BaseOperation

if TYPE_CHECKING:
    from typing import Literal

    from elpekenin_userspace.build import Recipe


class Diff(BaseOperation):
    """Apply a patch."""

    class Args(TypedDict):
        """Arguments for this operation."""

        operation: Literal["diff"]
        file: str

    def __init__(
        self,
        recipe: Recipe,
        entry: Args,
    ) -> None:
        """Initialize an instance."""
        self.workdir = recipe.path
        file = entry.get("file") or error.missing("file")
        self.file = elpekenin_userspace.path.resolve(file)

    def __str__(self) -> str:
        """Display this operation."""
        return f"apply patch '{self.file}'"

    def run(self) -> int:
        """Entrypoint."""
        repo = Repo(self.workdir)
        repo.git.apply("--reject", "--whitespace=fix", self.file)
        return 0
