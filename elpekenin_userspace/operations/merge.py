"""Build step."""

from __future__ import annotations

from typing import TYPE_CHECKING, TypedDict

from git import Repo

from elpekenin_userspace import error, git_helpers
from elpekenin_userspace.operations.base import BaseOperation

if TYPE_CHECKING:
    from typing import Literal

    from elpekenin_userspace.build import Recipe


class Merge(BaseOperation):
    """Fetch changes from another branch."""

    class Args(TypedDict):
        """Arguments for this operation."""

        operation: Literal["merge"]
        repo: str
        branch: str

    def __init__(
        self,
        recipe: Recipe,
        entry: Args,
    ) -> None:
        """Initialize an instance."""
        self.workdir = recipe.path
        self.remote = entry.get("repo") or error.missing("repo")
        self.branch = entry.get("branch") or error.missing("branch")

    def __str__(self) -> str:
        """Display this operation."""
        return f"merge '{self.branch}' ({self.remote})"

    def run(self) -> int:
        """Entrypoint."""
        repo = Repo(self.workdir)
        remote = git_helpers.fetch(repo, self.remote, self.branch)
        repo.git.merge(f"{remote}/{self.branch}")
        return 0
