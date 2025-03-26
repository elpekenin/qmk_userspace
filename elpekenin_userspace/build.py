"""Structure of a build.json file."""

from __future__ import annotations

import json
from typing import TYPE_CHECKING

from typing_extensions import Self

import elpekenin_userspace.path
from elpekenin_userspace import constants, error
from elpekenin_userspace.operations import OPERATIONS
from elpekenin_userspace.operations.git import Branch, Clean, Clone, Submodules

if TYPE_CHECKING:
    from collections.abc import Generator
    from pathlib import Path
    from typing import TypedDict

    from elpekenin_userspace.operations import Args
    from elpekenin_userspace.operations.base import BaseOperation, SetupOperation

    class Json(TypedDict, total=False):
        """Expected contents of JSON file."""

        repo: str
        branch: str
        path: str
        operations: list[Args]


class Recipe:
    """Specification of a firmware build."""

    def __init__(
        self,
        *,
        repo: str,
        branch: str,
        path: Path,
        operations: list[Args],
    ) -> None:
        """Initialize an instance."""
        self.repo = repo
        self.branch = branch
        self.path = path
        self.operations = operations

    def get_setup_operations(self) -> list[SetupOperation]:
        """Configure initial state of the repository."""
        return [
            Clean(self) if self.path.exists() else Clone(self),
            Branch(self),
            Submodules(self),
        ]

    def get_all_operations(self) -> Generator[SetupOperation | BaseOperation]:
        """Get all the steps in this recipe."""
        yield from self.get_setup_operations()

        for entry in self.operations:
            name = entry.get("operation") or error.missing("operation")
            if name not in OPERATIONS:
                error.abort(f"Unknown operation '{name}'")

            cls = OPERATIONS[name]
            yield cls(self, entry)

    @classmethod
    def from_file(cls, file: Path) -> Self:
        """Create an instance by reading a file."""
        if not file.is_file():
            error.abort(f"'{file}' is not a file")

        if file.name != constants.JSON:
            error.abort(f"Build script should be named '{constants.JSON}'")

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
