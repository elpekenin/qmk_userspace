"""Subcommand to manage documentation of the repository."""

from __future__ import annotations

import contextlib
import json
import os
import shlex
import shutil
import subprocess
import sys
import tempfile
from http.server import HTTPServer, SimpleHTTPRequestHandler
from pathlib import Path
from typing import TYPE_CHECKING, TypedDict

from sphinx.cmd.build import main as sphinx_build

from commands import args
from commands.base import BaseCommand

if TYPE_CHECKING:
    from argparse import ArgumentParser, Namespace
    from collections.abc import Generator

    class Command(TypedDict):
        """Typing stub for data in `compile_commands.json`."""

        command: str
        directory: str
        file: str


CWD = Path.cwd()


def cleanup_args(args: list[str], directory: Path) -> list[str]:
    """From all args, filter the ones we want, and fix relative paths."""
    out = [
        "-D_ATTR(...)=",
        f"-I{directory}",
    ]

    out.extend([arg for arg in args if arg.startswith("-D")])

    return out


def get_args_from_file(file: Path) -> list[str]:
    """Extract arguments from a compilation database file."""
    database: list[Command] = json.loads(file.read_text())
    command = database[0]

    args = shlex.split(command["command"])

    return cleanup_args(
        args,
        directory=Path(command["directory"]),
    )


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


def add_conf_environment() -> None:
    """Pass arguments into `conf.py` via environment variables."""
    # want docs to be generated from current code (local path)
    # not the (potentially outdated) copies on $QMK
    os.environ["CONF_ROOT"] = str(CWD)

    clang_args = get_args_from_file(Path("compile_commands.json"))
    os.environ["CONF_HAWKMOTH_CLANG"] = "||".join(clang_args)

    # for permalink into commits
    os.environ["CONF_USERSPACE_COMMIT"] = get_commit(Path().parent)
    os.environ["CONF_MODULES_COMMIT"] = get_commit(
        CWD / "modules" / "elpekenin",
    )


@contextlib.contextmanager
def or_temp_dir(maybe_dir: Path | None) -> Generator[str]:
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
        parser.add_argument(
            "--output",
            help="where to write the docs (default: temporary directory)",
            metavar="DIR",
            type=args.directory,
        )

        parser.add_argument(
            "--verbose",
            help="do not silence sphinx's output",
            action="store_true",
        )

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
        actions = {
            "build": self.do_build,
            "preview": self.do_preview,
            "deploy": self.do_deploy,
        }
        action_name: str = arguments.action
        action = actions[action_name]

        output: Path | None = arguments.output
        with or_temp_dir(output) as build:
            return action(build, arguments)

    def do_build(self, build: str, arguments: Namespace) -> int:
        """Build documentation."""
        add_conf_environment()

        sphinx_args = [
            "-M",
            "html",
            "docs",  # sourcedir
            build,  # builddir
        ]

        if not arguments.verbose:
            # --quiet keeps warnings on stderr
            sphinx_args.append("--silent")

        return sphinx_build(sphinx_args)

    def do_preview(self, build: str, arguments: Namespace) -> int:
        """Build documentation and spawn HTTP server to read it."""
        ret = self.do_build(build, arguments)
        if ret != 0:
            return ret

        listen: str = arguments.listen
        address, port_str = listen.split(":", maxsplit=1)
        port = int(port_str)

        httpd = HTTPServer(
            (address, port),
            SimpleHTTPRequestHandler,
        )

        docs = Path(build) / "html"
        os.chdir(str(docs))

        sys.stdout.write(f"Serving on http://{address}:{port}\n")
        with contextlib.suppress(KeyboardInterrupt):
            httpd.serve_forever()

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

        host: str = arguments.host
        path: str = arguments.path

        self.remove_docs(host, path)
        sys.stdout.write("Removed previous documentation\n")

        self.deploy_docs(build, host, path)
        sys.stdout.write("Deployed new documentation\n")

        return 0
