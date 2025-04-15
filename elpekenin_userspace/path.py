"""Utilities to interact with paths."""

from __future__ import annotations

import os
from contextlib import contextmanager
from pathlib import Path
from string import Template
from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from collections.abc import Generator


def resolve(raw: str | Path) -> Path:
    """Expand '~' and env vars."""
    expand_env = Template(str(raw)).substitute(os.environ)
    expand_user = Path(expand_env).expanduser()
    return expand_user.resolve()


@contextmanager
def chdir(path: Path) -> Generator[None]:
    """Temporarily change working directory."""
    old = Path.cwd()
    os.chdir(path)
    try:
        yield
    finally:
        os.chdir(old)
