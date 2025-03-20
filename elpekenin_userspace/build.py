"""Structure of a build.json file."""

from __future__ import annotations

import json
import sys
from typing import TYPE_CHECKING, cast

from git import Repo
from typing_extensions import Self

import elpekenin_userspace.path
from elpekenin_userspace import constants, error, git_helpers
from elpekenin_userspace.operations import OPERATIONS

if TYPE_CHECKING:
    from pathlib import Path
    from typing import Any, TypedDict

    from typing_extensions import Required

    from elpekenin_userspace.operations.base import BaseOperation

    class Json(TypedDict, total=False):
        """Expected contents of JSON file."""

        repo: str
        branch: str
        path: str
        operations: list[Operation]

    class Operation(TypedDict, total=False):
        """Contents of each operation in JSON file."""

        operation: Required[str]


class Recipe:
    """Specification of a firmware build."""

    def __init__(
        self,
        *,
        repo: str,
        branch: str,
        path: Path,
        operations: list[Operation],
    ) -> None:
        """Initialize an instance."""
        self.repo = repo
        self.branch = branch
        self.path = path
        self.operations = operations

    def get_operations(self) -> list[BaseOperation]:
        """Get all the steps in this recipe."""
        operations: list[BaseOperation] = []
        for entry in self.operations:
            name = entry.get("operation") or error.missing("operation")
            cls = OPERATIONS.get(name) or error.abort(f"Unknown operation '{name}'")
            operation = cls(self, cast("dict[str, Any]", entry))
            operations.append(operation)

        return operations

    def setup_repo(self) -> int:
        """Configure initial state of the repository."""
        if not self.path.exists():
            sys.stdout.write("Cloning repo, may take a while...\n")
            repo = Repo.clone_from(self.repo, self.path)
        else:
            if not self.path.is_dir():
                sys.stderr.write(f"'{self.path}' is not a directory\n")
                return 1

            sys.stdout.write("Found existing repo\n")
            repo = Repo(self.path)

            # remove changes
            repo.git.reset("--hard")

        # get to the desired repo:branch
        remote_name = git_helpers.fetch(repo, self.repo)
        repo.git.checkout(f"{remote_name}/{self.branch}")

        # remove leftover changes
        repo.git.clean("-dxf")

        sys.stdout.write(f"Working at '{self.branch}' ({self.repo})\n")

        sys.stdout.write("Synchronizing submodules, may take a while...\n")
        repo.git.submodule("sync", "--recursive")
        repo.git.submodule("update", "--init", "--recursive", "--progress")

        return 0

    def display(self) -> None:
        """Show the steps in this build recipe."""
        for operation in self.get_operations():
            sys.stdout.write(f"{operation}\n")

    def run(self) -> int:
        """Entrypoint."""
        ret = self.setup_repo()
        if ret != 0:
            return ret

        for operation in self.get_operations():
            sys.stdout.write(f"Running: {operation}\n")

            ret = operation.run()
            if ret != 0:
                return ret

        return 0

    @classmethod
    def from_file(cls, file: Path) -> Self:
        """Create an instance by reading a file."""
        if not file.is_file() or file.name != "build.json":
            error.abort("Unexpected filename")

        data: Json = json.loads(file.read_text())

        # with defaults
        repo = data.get("repo") or constants.QMK
        branch = data.get("branch") or "master"

        # without defaults
        path = data.get("path") or error.missing("path")
        operations = data.get("operations") or error.missing("operations")

        return cls(
            repo=repo,
            branch=branch,
            path=elpekenin_userspace.path.resolve(path),
            operations=operations,
        )
