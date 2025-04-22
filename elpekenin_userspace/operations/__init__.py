"""All kind of operations."""

from __future__ import annotations

from typing import TYPE_CHECKING, Union

from elpekenin_userspace.operations.cp import Cp
from elpekenin_userspace.operations.exec import Exec
from elpekenin_userspace.operations.git import Checkout, Diff, Merge, Pr
from elpekenin_userspace.operations.stop import Stop

if TYPE_CHECKING:
    from elpekenin_userspace.operations.base import Operation

Args = Union[
    Checkout.Args,
    Cp.Args,
    Diff.Args,
    Exec.Args,
    Merge.Args,
    Pr.Args,
    Stop.Args,
]

OPERATIONS: dict[str, type[Operation]] = {
    "checkout": Checkout,
    "cp": Cp,
    "diff": Diff,
    "exec": Exec,
    "merge": Merge,
    "pr": Pr,
    "stop": Stop,
}
