"""Build step."""

from __future__ import annotations

from typing import TYPE_CHECKING

from git import Repo

from elpekenin_userspace import error, git_helpers
from elpekenin_userspace.operations.base import BaseOperation

if TYPE_CHECKING:
    from typing import Any

    from elpekenin_userspace.build import Recipe


class Checkout(BaseOperation):
    """Grab some files from another branch."""

    def __init__(
        self,
        recipe: Recipe,
        entry: dict[str, Any],
    ) -> None:
        """Initialize an instance."""
        self.workdir = recipe.path
        self.remote = entry.get("repo") or error.missing("repo")
        self.branch = entry.get("branch") or error.missing("branch")
        self.files: list[str] = entry.get("files") or error.missing("files")

    def __str__(self) -> str:
        """Display this operation."""
        files = " ".join(self.files)
        return f"checkout '{self.branch}' -- {files} ({self.remote})"

    def run(self) -> int:
        """Entrypoint."""
        repo = Repo(self.workdir)
        remote = git_helpers.fetch(repo, self.remote, self.branch)
        repo.git.checkout(f"{remote}/{self.branch}", "--", *self.files)
        return 0
