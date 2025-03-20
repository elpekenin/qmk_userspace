"""Utilities to interact with paths."""

from __future__ import annotations

import os
from pathlib import Path
from string import Template


def resolve(raw: str | Path) -> Path:
    """Expand '~' and env vars."""
    expand_env = Template(str(raw)).substitute(os.environ)
    expand_user = Path(expand_env).expanduser()
    return expand_user.resolve()
