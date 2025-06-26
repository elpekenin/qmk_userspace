"""Build steps performing git commands."""

from __future__ import annotations

from typing import TYPE_CHECKING, TypedDict

from git import Repo

import elpekenin_userspace.path
from elpekenin_userspace import constants
from elpekenin_userspace.operations.base import Operation, SetupOperation
from elpekenin_userspace.result import Ok, missing

if TYPE_CHECKING:
    from pathlib import Path
    from typing import Literal

    from typing_extensions import NotRequired, Self

    from elpekenin_userspace.build import Recipe
    from elpekenin_userspace.result import Result


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

    def __init__(self, url: str, branch: str, dst: Path) -> None:
        """Initialize an instance."""
        self.url = url
        self.branch = branch
        self.dst = dst

    @classmethod
    def init_from(cls, recipe: Recipe) -> Result[Self, str]:
        """Initialize an instance."""
        return Ok(cls(recipe.repo, recipe.branch, recipe.path))

    def __str__(self) -> str:
        """Display this operation."""
        return f"Swapping to '{self.branch}'"

    def run(self) -> Result[None, str]:
        """Entrypoint."""
        repo = Repo(self.dst)
        name = fetch(repo, self.url)
        repo.git.checkout(f"{name}/{self.branch}")
        return Ok(None)


class Clean(SetupOperation):
    """Clean existing repository."""

    def __init__(self, dst: Path) -> None:
        """Initialize an instance."""
        self.dst = dst

    @classmethod
    def init_from(cls, recipe: Recipe) -> Result[Self, str]:
        """Initialize an instance."""
        return Ok(cls(recipe.path))

    def __str__(self) -> str:
        """Display this operation."""
        return "Cleaning repository"

    def run(self) -> Result[None, str]:
        """Entrypoint."""
        Repo(self.dst).git.clean("-dxf")
        return Ok(None)


class Clone(SetupOperation):
    """Clone a repository in the given path."""

    def __init__(self, url: str, dst: Path) -> None:
        """Initialize an instance."""
        self.url = url
        self.dst = dst

    @classmethod
    def init_from(cls, recipe: Recipe) -> Result[Self, str]:
        """Initialize an instance."""
        return Ok(cls(recipe.repo, recipe.path))

    def __str__(self) -> str:
        """Display this operation."""
        return f"Cloning {self.url}"

    def run(self) -> Result[None, str]:
        """Entrypoint."""
        Repo.clone_from(self.url, self.dst)
        return Ok(None)


class Submodules(SetupOperation):
    """Synchronize submodules."""

    def __init__(self, dst: Path) -> None:
        """Initialize an instance."""
        self.dst = dst

    @classmethod
    def init_from(cls, recipe: Recipe) -> Result[Self, str]:
        """Initialize an instance."""
        return Ok(cls(recipe.path))

    def __str__(self) -> str:
        """Display this operation."""
        return "Synchronizing submodules"

    def run(self) -> Result[None, str]:
        """Entrypoint."""
        repo = Repo(self.dst)
        repo.git.submodule("sync", "--recursive")
        repo.git.submodule("update", "--init", "--recursive", "--progress")
        return Ok(None)


# operations available over JSON file
class Checkout(Operation):
    """Grab some files from another branch."""

    class Args(TypedDict):
        """Arguments for this operation."""

        operation: Literal["checkout"]
        repo: str
        branch: str
        files: list[str]

    def __init__(
        self,
        workdir: Path,
        remote: str,
        branch: str,
        files: list[str],
    ) -> None:
        """Initialize an instance."""
        self.workdir = workdir
        self.remote = remote
        self.branch = branch
        self.files = files

    @classmethod
    def init_from(
        cls,
        recipe: Recipe,
        entry: Args,  # type: ignore[override]  # is not the Union of them all
    ) -> Result[Self, str]:
        """Initialize an instance."""
        workdir = recipe.path

        remote = entry.get("repo")
        if remote is None:
            return missing("repo")

        branch = entry.get("branch")
        if branch is None:
            return missing("branch")

        files = entry.get("files")
        if files is None:
            return missing("files")

        return Ok(cls(workdir, remote, branch, files))

    def __str__(self) -> str:
        """Display this operation."""
        return f"Checking out '{self.branch}'"

    def run(self) -> Result[None, str]:
        """Entrypoint."""
        repo = Repo(self.workdir)
        remote = fetch(repo, self.remote, self.branch)
        repo.git.checkout(f"{remote}/{self.branch}", "--", *self.files)
        return Ok(None)


class Diff(Operation):
    """Apply a patch."""

    class Args(TypedDict):
        """Arguments for this operation."""

        operation: Literal["diff"]
        file: str
        path: NotRequired[str]

    def __init__(
        self,
        file: Path,
        path: Path,
    ) -> None:
        """Initialize an instance."""
        self.file = file
        self.path = path

    @classmethod
    def init_from(
        cls,
        recipe: Recipe,
        entry: Args,  # type: ignore[override]  # is not the Union of them all
    ) -> Result[Self, str]:
        """Initialize an instance."""
        workdir = recipe.path

        file = entry.get("file")
        if file is None:
            return missing("file")

        path = entry.get("path") or workdir

        return Ok(
            cls(
                elpekenin_userspace.path.resolve(file),
                elpekenin_userspace.path.resolve(path),
            ),
        )

    def __str__(self) -> str:
        """Display this operation."""
        return f"Applying '{self.file.name}'"

    def run(self) -> Result[None, str]:
        """Entrypoint."""
        repo = Repo(self.path)
        repo.git.apply("--reject", "--whitespace=fix", self.file)
        return Ok(None)


class Merge(Operation):
    """Fetch changes from another branch."""

    class Args(TypedDict):
        """Arguments for this operation."""

        operation: Literal["merge"]
        repo: str
        branch: str

    def __init__(
        self,
        workdir: Path,
        remote: str,
        branch: str,
    ) -> None:
        """Initialize an instance."""
        self.workdir = workdir
        self.remote = remote
        self.branch = branch

    @classmethod
    def init_from(
        cls,
        recipe: Recipe,
        entry: Args,  # type: ignore[override]  # is not the Union of them all
    ) -> Result[Self, str]:
        """Initialize an instance."""
        workdir = recipe.path

        remote = entry.get("repo")
        if remote is None:
            return missing("repo")

        branch = entry.get("branch")
        if branch is None:
            return missing("branch")

        return Ok(cls(workdir, remote, branch))

    def __str__(self) -> str:
        """Display this operation."""
        return f"Merging '{self.branch}'"

    def run(self) -> Result[None, str]:
        """Entrypoint."""
        repo = Repo(self.workdir)
        remote = fetch(repo, self.remote, self.branch)
        repo.git.merge(f"{remote}/{self.branch}")
        return Ok(None)


class Pr(Operation):
    """Fetch changes from a PR."""

    class Args(TypedDict):
        """Arguments for this operation."""

        operation: Literal["pr"]
        repo: NotRequired[str]
        id: int

    def __init__(
        self,
        workdir: Path,
        remote: str,
        id_: int,
    ) -> None:
        """Initialize an instance."""
        self.workdir = workdir
        self.remote = remote
        self.id_ = id_

    @classmethod
    def init_from(
        cls,
        recipe: Recipe,
        entry: Args,  # type: ignore[override]  # is not the Union of them all
    ) -> Result[Self, str]:
        """Initialize an instance."""
        workdir = recipe.path

        remote = entry.get("repo") or constants.QMK

        id_ = entry.get("id")
        if id_ is None:
            return missing("id")

        return Ok(cls(workdir, remote, id_))

    def __str__(self) -> str:
        """Display this operation."""
        return f"PR {self.id_}"

    def run(self) -> Result[None, str]:
        """Entrypoint."""
        repo = Repo(self.workdir)

        remote_branch = f"pull/{self.id_}/head"
        local_branch = f"PR{self.id_}"
        fetch(repo, self.remote, f"{remote_branch}:{local_branch}")

        repo.git.merge(local_branch)

        return Ok(None)


class Reset(SetupOperation):
    """Reset existing repository."""

    def __init__(self, dst: Path) -> None:
        """Initialize an instance."""
        self.dst = dst

    @classmethod
    def init_from(cls, recipe: Recipe) -> Result[Self, str]:
        """Initialize an instance."""
        return Ok(cls(recipe.path))

    def __str__(self) -> str:
        """Display this operation."""
        return "Resetting repository"

    def run(self) -> Result[None, str]:
        """Entrypoint."""
        Repo(self.dst).git.reset("--hard")
        return Ok(None)
