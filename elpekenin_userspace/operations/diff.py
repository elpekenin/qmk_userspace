"""Build step."""

from __future__ import annotations

from typing import TYPE_CHECKING

from git import Repo

import elpekenin_userspace.path
from elpekenin_userspace import error
from elpekenin_userspace.operations.base import BaseOperation

if TYPE_CHECKING:
    from typing import Any

    from elpekenin_userspace.build import Recipe


class Diff(BaseOperation):
    """Apply a patch."""

    def __init__(
        self,
        recipe: Recipe,
        entry: dict[str, Any],
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
