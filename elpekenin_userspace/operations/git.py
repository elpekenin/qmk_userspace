"""Build steps performing git commands."""

from __future__ import annotations

from typing import TYPE_CHECKING, TypedDict

from git import Repo

import elpekenin_userspace.path
from elpekenin_userspace import constants, error
from elpekenin_userspace.operations.base import BaseOperation, SetupOperation

if TYPE_CHECKING:
    from typing import Literal

    from typing_extensions import NotRequired

    from elpekenin_userspace.build import Recipe


def name_for(remote_url: str) -> str:
    """Create an identifier for a remote's URL."""
    # offset to skip '-' if hash is negative
    return str(hex(hash(remote_url)))[1:]


def add_remote(repo: Repo, remote_url: str) -> str:
    """Add a remote."""
    remote_name = name_for(remote_url)

    for remote in repo.remotes:
        if remote.name == remote_name:
            break
    else:
        repo.create_remote(remote_name, remote_url)

    return remote_name


def fetch(repo: Repo, remote_url: str, branch: str | None = None) -> str:
    """Fetch `remote/branch` into `repo`, return remote's name."""
    remote_name = add_remote(repo, remote_url)

    if branch is not None:
        repo.git.fetch(remote_name, branch)
    else:
        repo.git.fetch(remote_name)

    return remote_name


# setup operations, out of user control
class Branch(SetupOperation):
    """Swap into desired branch."""

    def __init__(self, recipe: Recipe) -> None:
        """Initialize an instance."""
        self.url = recipe.repo
        self.branch = recipe.branch
        self.dst = recipe.path

    def __str__(self) -> str:
        """Display this operation."""
        return f"Swapping to '{self.branch}'"

    def run(self) -> int:
        """Entrypoint."""
        repo = Repo(self.dst)
        name = fetch(repo, self.url)
        repo.git.checkout(f"{name}/{self.branch}")
        return 0


class Clean(SetupOperation):
    """Clear an existing repository."""

    def __init__(self, recipe: Recipe) -> None:
        """Initialize an instance."""
        self.dst = recipe.path

    def __str__(self) -> str:
        """Display this operation."""
        return "Cleaning repository"

    def run(self) -> int:
        """Entrypoint."""
        repo = Repo(self.dst)
        repo.git.reset("--hard")
        repo.git.clean("-dxf")
        return 0


class Clone(SetupOperation):
    """Clone a repository in the given path."""

    def __init__(self, recipe: Recipe) -> None:
        """Initialize an instance."""
        self.url = recipe.repo
        self.dst = recipe.path

    def __str__(self) -> str:
        """Display this operation."""
        return f"Cloning {self.url}"

    def run(self) -> int:
        """Entrypoint."""
        Repo.clone_from(self.url, self.dst)
        return 0


class Submodules(SetupOperation):
    """Synchronize submodules."""

    def __init__(self, recipe: Recipe) -> None:
        """Initialize an instance."""
        self.dst = recipe.path

    def __str__(self) -> str:
        """Display this operation."""
        return "Synchronizing submodules"

    def run(self) -> int:
        """Entrypoint."""
        repo = Repo(self.dst)
        repo.git.submodule("sync", "--recursive")
        repo.git.submodule("update", "--init", "--recursive", "--progress")
        return 0


# operations available over JSON file
class Checkout(BaseOperation):
    """Grab some files from another branch."""

    class Args(TypedDict):
        """Arguments for this operation."""

        operation: Literal["checkout"]
        repo: str
        branch: str
        files: list[str]

    def __init__(
        self,
        recipe: Recipe,
        entry: Args,
    ) -> None:
        """Initialize an instance."""
        self.workdir = recipe.path
        self.remote = entry.get("repo") or error.missing("repo")
        self.branch = entry.get("branch") or error.missing("branch")
        self.files = entry.get("files") or error.missing("files")

    def __str__(self) -> str:
        """Display this operation."""
        return f"Checking out '{self.branch}'"

    def run(self) -> int:
        """Entrypoint."""
        repo = Repo(self.workdir)
        remote = fetch(repo, self.remote, self.branch)
        repo.git.checkout(f"{remote}/{self.branch}", "--", *self.files)
        return 0


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
        return f"Applying '{self.file.name}'"

    def run(self) -> int:
        """Entrypoint."""
        repo = Repo(self.workdir)
        repo.git.apply("--reject", "--whitespace=fix", self.file)
        return 0


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
        return f"Merging '{self.branch}'"

    def run(self) -> int:
        """Entrypoint."""
        repo = Repo(self.workdir)
        remote = fetch(repo, self.remote, self.branch)
        repo.git.merge(f"{remote}/{self.branch}")
        return 0


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
        return f"PR {self.id_}"

    def run(self) -> int:
        """Entrypoint."""
        repo = Repo(self.workdir)

        remote_branch = f"pull/{self.id_}/head"
        local_branch = f"PR{self.id_}"
        fetch(repo, self.remote, f"{remote_branch}:{local_branch}")

        repo.git.merge(local_branch)

        return 0
