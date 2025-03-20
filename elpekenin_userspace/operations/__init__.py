"""All kind of operations."""

from __future__ import annotations

from typing import TYPE_CHECKING

from elpekenin_userspace.operations.checkout import Checkout
from elpekenin_userspace.operations.cp import Cp
from elpekenin_userspace.operations.diff import Diff
from elpekenin_userspace.operations.exec import Exec
from elpekenin_userspace.operations.merge import Merge
from elpekenin_userspace.operations.pr import Pr
from elpekenin_userspace.operations.stop import Stop

if TYPE_CHECKING:
    from elpekenin_userspace.operations.base import BaseOperation


OPERATIONS: dict[str, type[BaseOperation]] = {
    "checkout": Checkout,
    "cp": Cp,
    "diff": Diff,
    "exec": Exec,
    "merge": Merge,
    "pr": Pr,
    "stop": Stop,
}
