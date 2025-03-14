"""Subcommand to manage documentation of the repository."""

from __future__ import annotations

import json
import os
import shlex
import shutil
import subprocess
import sys
import tempfile
from contextlib import contextmanager
from http.server import HTTPServer, SimpleHTTPRequestHandler
from pathlib import Path
from typing import TYPE_CHECKING, TypedDict

try:
    # NOTE: function signature copied from sphinx's hints
    sphinx_build: Callable[[Sequence[str]], int] | None
    from sphinx.cmd.build import main as sphinx_build
except ImportError:
    sphinx_build = None

from . import args
from .base import BaseCommand

if TYPE_CHECKING:
    from argparse import ArgumentParser, Namespace
    from collections.abc import Callable, Generator, Sequence

    class CompilationDatabaseEntry(TypedDict):
        """Typing stub for data in `compile_commands.json`."""

        command: str
        directory: str
        file: str


COMPILEDB = "compile_commands.json"


def is_userspace(path: Path) -> bool:
    """Validate if the folder argument is actually an userspace.

    So far the this checks for the existence of
      * compile_commands.json
      * docs
    Those should be the minimum to get docs building.

    No further constraints in case anyone ever wants to use this tool
    """
    compiledb = path / COMPILEDB
    docs = path / "docs"
    return compiledb.is_file() and docs.is_dir()


def fixed_path(raw: str, *, userspace: Path, qmk: Path) -> str:
    """Given a path, make it absolute by prepending source when needed.

    This is needed because hawkmoth is configured to look at userspace, not
    QMK directory, thus relative paths will break (except users/ and modules/)
    """
    path = Path(raw)

    # make sure paths to these folders point to this repository
    for directory in ("modules", "users"):
        if directory in path.parts:
            index = path.parts.index(directory)

            ret = userspace
            for part in path.parts[index:]:
                ret /= part

            return str(ret)

    if raw.startswith("/"):
        return raw

    return str(qmk / path)


def include_flags(args: list[str], *, userspace: Path, qmk: Path) -> list[str]:
    """From all args, filter the ones adding include paths or files."""
    ret: list[str] = []
    for i, arg in enumerate(args):
        if arg.startswith("-I"):
            ret.append(
                "-I"
                + fixed_path(
                    arg.removeprefix("-I"),
                    userspace=userspace,
                    qmk=qmk,
                ),
            )

        if arg in {"-isystem", "-include"}:
            ret.append(arg)
            ret.append(
                fixed_path(
                    args[i + 1],
                    userspace=userspace,
                    qmk=qmk,
                ),
            )

    return ret


def define_flags(args: list[str]) -> list[str]:
    """From all args, filter the ones defining pre-processor macros."""
    return [arg for arg in args if arg.startswith("-D")]


def cleanup_args(args: list[str], *, userspace: Path, qmk: Path) -> list[str]:
    """From all args, filter the ones we want, and fix relative paths."""
    return [
        f"-I{qmk}",
        *define_flags(args),
        *include_flags(args, userspace=userspace, qmk=qmk),
    ]


def get_args_and_dir_from_file(file: Path) -> tuple[list[str], Path]:
    """Extract arguments from a compilation database file."""
    database: list[CompilationDatabaseEntry] = json.loads(file.read_text())
    entry = database[0]
    return shlex.split(entry["command"]), Path(entry["directory"])


def get_commit(directory: Path) -> str:
    """Get full hash of a repository folder."""
    git = shutil.which("git")
    if git is None:
        msg = "git not found"
        raise RuntimeError(msg)

    return (
        subprocess.run(  # noqa: S603  # out of attacker control
            [
                git,
                "rev-parse",
                "HEAD",
            ],
            capture_output=True,
            cwd=directory,
            check=True,
        )
        .stdout.decode()
        .rstrip("\n")
    )


def add_conf_environment(userspace: Path) -> None:
    """Pass arguments into `conf.py` via environment variables."""
    # want docs to be generated from current code (local path)
    # not the (potentially outdated) copies on $QMK
    os.environ["CONF_ROOT"] = str(userspace)

    raw_args, qmk = get_args_and_dir_from_file(userspace / COMPILEDB)
    args = cleanup_args(raw_args, userspace=userspace, qmk=qmk)

    sep = "||"
    os.environ["CONF_HAWKMOTH_CLANG"] = sep.join(args)
    os.environ["CONF_HAWKMOTH_CLANG_SEP"] = sep

    # for permalink into commits
    os.environ["CONF_USERSPACE_COMMIT"] = get_commit(userspace)
    os.environ["CONF_MODULES_COMMIT"] = get_commit(
        userspace / "modules" / "elpekenin",
    )


@contextmanager
def build_dir(maybe_dir: Path | None) -> Generator[str]:
    """Return input if not None, a temporary dir otherwise."""
    if maybe_dir is not None:
        yield str(maybe_dir)
    else:
        with tempfile.TemporaryDirectory() as directory:
            yield directory


class Docs(BaseCommand):
    """Generate or deploy documentation."""

    @classmethod
    def add_args(cls, parser: ArgumentParser) -> None:
        """Command-specific arguments."""
        # common args
        parser.add_argument(
            "--output",
            help="where to write the docs (default: temporary directory)",
            metavar="DIR",
            type=args.Directory(require_existence=True),
        )
        parser.add_argument(
            "--userspace",
            help="where to find your userspace (default: cwd)",
            metavar="DIR",
            type=args.Directory(require_existence=True),
            default=Path.cwd(),
        )
        parser.add_argument(
            "--verbose",
            help="do not silence sphinx's output",
            action="store_true",
        )

        # subcommands and their specific args
        subparsers = parser.add_subparsers(
            title="action",
            dest="action",
            required=True,
        )

        _ = subparsers.add_parser(
            "build",
            help="build the documentation",
        )

        preview = subparsers.add_parser(
            "preview",
            help="build and preview the documentation",
        )
        preview.add_argument(
            "--listen",
            help="interface where to listen",
            metavar="ADDRESS:PORT",
            required=False,
            default="localhost:8000",
        )

        deploy = subparsers.add_parser(
            "deploy",
            help="build and deploy the documentation",
        )
        deploy.add_argument(
            "--host",
            help="device where to copy docs",
            metavar="HOST",
            required=False,
            default="elraspberrin",
        )
        deploy.add_argument(
            "--path",
            help="location where docs should be copied",
            metavar="DIR",
            required=False,
            default="~/qmk_docs",
        )

        return super().add_args(parser)

    def run(self, arguments: Namespace) -> int:
        """Entrypoint."""
        if not is_userspace(arguments.userspace):
            sys.stderr.write("Unexpected folder structure\n")
            return 1

        actions = {
            "build": self.do_build,
            "preview": self.do_preview,
            "deploy": self.do_deploy,
        }
        action = actions[arguments.action]

        with build_dir(arguments.output) as build:
            return action(build, arguments)

    def do_build(self, build: str, arguments: Namespace) -> int:
        """Build documentation."""
        add_conf_environment(arguments.userspace)

        docs = str(arguments.userspace / "docs")

        sphinx_args = [
            "-M",
            "html",
            docs,  # sourcedir
            build,  # builddir
        ]

        if not arguments.verbose:
            # --quiet keeps warnings on stderr
            sphinx_args.append("--silent")

        if sphinx_build is None:
            sys.stderr.write(
                "Dependencies missing, run `pip install .[docs]`\n",
            )
            return 1

        return sphinx_build(sphinx_args)

    def do_preview(self, build: str, arguments: Namespace) -> int:
        """Build documentation and spawn HTTP server to read it."""
        ret = self.do_build(build, arguments)
        if ret != 0:
            return ret

        listen: str = arguments.listen
        try:
            address, port_str = listen.split(":", maxsplit=1)
        except ValueError:
            sys.stderr.write(
                "Invalid interface specified. Use ADDRESS:PORT syntax\n",
            )
            return 1

        try:
            port = int(port_str)
        except ValueError:
            sys.stderr.write(
                "Invalid interface specified. Port has to be a number\n",
            )
            return 1

        httpd = HTTPServer(
            (address, port),
            SimpleHTTPRequestHandler,
        )

        docs = Path(build) / "html"
        os.chdir(str(docs))

        sys.stdout.write(f"Serving on http://{address}:{port}\n")
        httpd.serve_forever()

        # unreachable (?), does server exit?
        return 0

    def remove_docs(self, host: str, path: str) -> None:
        """Remove old docs from server."""
        ssh = shutil.which("ssh")
        if ssh is None:
            msg = "ssh not found"
            raise RuntimeError(msg)

        subprocess.run(  # noqa: S603  # out of attacker control
            [
                ssh,
                host,
                "rm",
                # -f in case it doesn't exist
                "-rf",
                path,
            ],
            check=True,
        )

    def deploy_docs(self, src: str, host: str, dst: str) -> None:
        """Copy new docs into server."""
        scp = shutil.which("scp")
        if scp is None:
            msg = "scp not found"
            raise RuntimeError(msg)

        html = str(Path(src) / "html")
        subprocess.run(  # noqa: S603  # out of attacker control
            [
                scp,
                "-prq",
                # dot makes scp copy the contents instead of the folder itself
                html,
                f"{host}:{dst}",
            ],
            check=True,
        )

    def do_deploy(self, build: str, arguments: Namespace) -> int:
        """Build documentation and deploy to a host with NGINX to serve it."""
        ret = self.do_build(build, arguments)
        if ret != 0:
            return ret
        sys.stdout.write("Documentation built\n")

        self.remove_docs(arguments.host, arguments.path)
        sys.stdout.write("Removed previous documentation\n")

        self.deploy_docs(build, arguments.host, arguments.path)
        sys.stdout.write("Deployed new documentation\n")

        return 0
