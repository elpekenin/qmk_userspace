"""Convenience utilities on top of GitPython."""

from __future__ import annotations

from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from git import Repo


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
