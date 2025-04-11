"""Structure of a build.json file."""

from __future__ import annotations

import json
from typing import TYPE_CHECKING, cast

from typing_extensions import Self

import elpekenin_userspace.path
from elpekenin_userspace import constants
from elpekenin_userspace.operations import OPERATIONS
from elpekenin_userspace.operations.git import Branch, Clean, Clone, Reset, Submodules
from elpekenin_userspace.result import Err, Ok, is_err, missing

if TYPE_CHECKING:
    from pathlib import Path
    from typing import TypedDict

    from elpekenin_userspace.operations import Args
    from elpekenin_userspace.operations.base import BaseOperation, SetupOperation
    from elpekenin_userspace.result import Result

    class Json(TypedDict, total=False):
        """Expected contents of JSON file."""

        repo: str
        branch: str
        clean: bool
        path: str
        operations: list[Args]


class Recipe:
    """Specification of a firmware build."""

    def __init__(
        self,
        *,
        repo: str,
        branch: str,
        clean: bool,
        path: Path,
        operations: list[Args],
    ) -> None:
        """Initialize an instance."""
        self.repo = repo
        self.branch = branch
        self.clean = clean
        self.path = path
        self.operations = operations

    def get_setup_operations(self) -> Result[list[SetupOperation], str]:
        """Configure initial state of the repository."""
        operations: list[SetupOperation] = []

        setup = cast(
            "Result[Reset | Clone, str]",
            Reset.init_from(self) if self.path.exists() else Clone.init_from(self),
        )
        if is_err(setup):
            return setup
        operations.append(setup.ok())

        if self.clean:
            clean = Clean.init_from(self)
            if is_err(clean):
                return clean
            operations.append(clean.ok())

        branch = Branch.init_from(self)
        if is_err(branch):
            return branch
        operations.append(branch.ok())

        submodules = Submodules.init_from(self)
        if is_err(submodules):
            return submodules
        operations.append(submodules.ok())

        return Ok(operations)

    def get_all_operations(self) -> Result[list[BaseOperation], str]:
        """Get all the steps in this recipe."""
        setup = self.get_setup_operations()
        if is_err(setup):
            return setup

        ret = cast("list[BaseOperation]", setup.ok())

        for entry in self.operations:
            name = entry.get("operation")
            if name is None:
                return missing("operation")

            if name not in OPERATIONS:
                return Err(f"Unknown operation '{name}'")

            cls = OPERATIONS[name]

            res = cls.init_from(self, entry)
            if is_err(res):
                return res

            ret.append(cast("BaseOperation", res.ok()))

        return Ok(ret)

    @classmethod
    def from_file(cls, file: Path) -> Result[Self, str]:
        """Create an instance by reading a file."""
        if not file.is_file():
            return Err(f"'{file}' is not a file")

        if file.name != constants.JSON:
            return Err(f"Build script should be named '{constants.JSON}'")

        data: Json = json.loads(file.read_text())

        # with defaults
        repo = data.get("repo") or constants.QMK
        branch = data.get("branch") or "master"
        clean = data.get("clean") or False

        # without defaults
        path = data.get("path")
        if path is None:
            return missing("path")

        operations = data.get("operations")
        if operations is None:
            return missing("operations")

        return Ok(
            cls(
                repo=repo,
                branch=branch,
                clean=clean,
                path=elpekenin_userspace.path.resolve(path),
                operations=operations,
            ),
        )
