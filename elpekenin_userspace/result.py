"""Tiny utilities for functions that can return values and errors."""

from __future__ import annotations

from typing import TYPE_CHECKING, Generic, TypeVar, Union

if TYPE_CHECKING:
    from typing import Literal, NoReturn

    from typing_extensions import TypeAlias, TypeIs

T = TypeVar("T")
E = TypeVar("E")


class UnwrapError(Exception):
    """Excepction raised when trying to access wrong attributes.

    AKA: Ok.err() or Err.ok()
    """


class Ok(Generic[T]):
    """Class signaling a successful run."""

    __slots__ = ("__val",)

    def __init__(self, val: T) -> None:
        """Initialize an instance."""
        self.__val = val

    def is_ok(self) -> Literal[True]:
        """Check if this is an ok value."""
        return True

    def is_err(self) -> Literal[False]:
        """Check if this is a value error."""
        return False

    def ok(self) -> T:
        """Unwrap the ok value."""
        return self.__val

    def err(self) -> NoReturn:
        """Unwrap the error value (error)."""
        raise UnwrapError


class Err(Generic[E]):
    """Class signaling a error."""

    __slots__ = ("__val",)

    def __init__(self, val: E) -> None:
        """Initialize an instance."""
        self.__val = val

    def is_ok(self) -> Literal[False]:
        """Check if this is an ok value."""
        return False

    def is_err(self) -> Literal[True]:
        """Check if this is a value error."""
        return True

    def ok(self) -> NoReturn:
        """Unwrap the ok value (error)."""
        raise UnwrapError

    def err(self) -> E:
        """Unwrap the error value."""
        return self.__val


def is_ok(val: Result[T, E]) -> TypeIs[Ok[T]]:
    """Check if a result is successful."""
    return val.is_ok()


def is_err(val: Result[T, E]) -> TypeIs[Err[E]]:
    """Check if a result is error."""
    return val.is_err()


Result: TypeAlias = Union[Ok[T], Err[E]]


def missing(key: str) -> Err[str]:
    """Shortcut for missing key."""
    return Err(f"Missing required key '{key}'")
