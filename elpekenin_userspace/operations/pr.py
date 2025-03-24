"""Build step."""

from __future__ import annotations

from typing import TYPE_CHECKING, TypedDict

from git import Repo

from elpekenin_userspace import constants, error, git_helpers
from elpekenin_userspace.operations.base import BaseOperation

if TYPE_CHECKING:
    from typing import Literal

    from typing_extensions import NotRequired

    from elpekenin_userspace.build import Recipe


class Pr(BaseOperation):
    """Fetch changes from a PR."""

    class Args(TypedDict):
        """Arguments for this operation."""

        operation: Literal["pr"]
        repo: NotRequired[str]
        id: int

    def __init__(
        self,
        recipe: Recipe,
        entry: Args,
    ) -> None:
        """Initialize an instance."""
        self.workdir = recipe.path
        self.remote = entry.get("repo") or constants.QMK
        self.id_ = entry.get("id") or error.missing("id")

    def __str__(self) -> str:
        """Display this operation."""
        return f"Apply changes from PR {self.id_} ({self.remote})"

    def run(self) -> int:
        """Entrypoint."""
        repo = Repo(self.workdir)

        remote_branch = f"pull/{self.id_}/head"
        local_branch = f"PR{self.id_}"
        git_helpers.fetch(repo, self.remote, f"{remote_branch}:{local_branch}")

        repo.git.merge(local_branch)

        return 0
