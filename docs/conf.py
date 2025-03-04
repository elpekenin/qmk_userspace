# noqa: INP001

"""Generate the documentation hosted on <qmk.elpekenin.dev>_."""

from __future__ import annotations

import builtins
import os
from pathlib import Path
from typing import Literal, cast

Repository = Literal["userspace", "module"]

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

project = "QMK userspace"
copyright = "2024, elpekenin"  # noqa: A001
author = "elpekenin"
release = ""


# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = [
    "hawkmoth",
    "hawkmoth.ext.napoleon",
    "sphinx_tabs.tabs",
]

templates_path = ["_templates"]
exclude_patterns = ["_build", "Thumbs.db", ".DS_Store"]


# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

html_theme = "furo"
html_show_sourcelink = False  # disable "view source" button, showing raw rst
html_static_path: list[str] = []
html_theme_options = {
    "navigation_with_keys": True,
    "footer_icons": [
        {  # Github logo
            "name": "GitHub",
            "url": "https://github.com/elpekenin/qmk_userspace/",
            "html": """
                <svg
                    stroke="currentColor"
                    fill="currentColor"
                    stroke-width="0"
                    viewBox="0 0 16 16"
                >
                    <path
                        fill-rule="evenodd"
                        d="M8 0C3.58 0 0 3.58 0 8c0 3.54 2.29 6.53 5.47 7.59.4.07.55-.17.55-.38 0-.19-.01-.82-.01-1.49-2.01.37-2.53-.49-2.69-.94-.09-.23-.48-.94-.82-1.13-.28-.15-.68-.52-.01-.53.63-.01 1.08.58 1.23.82.72 1.21 1.87.87 2.33.66.07-.52.28-.87.51-1.07-1.78-.2-3.64-.89-3.64-3.95 0-.87.31-1.59.82-2.15-.08-.2-.36-1.02.08-2.12 0 0 .67-.21 2.2.82.64-.18 1.32-.27 2-.27.68 0 1.36.09 2 .27 1.53-1.04 2.2-.82 2.2-.82.44 1.1.16 1.92.08 2.12.51.56.82 1.27.82 2.15 0 3.07-1.87 3.75-3.65 3.95.29.25.54.73.54 1.48 0 1.07-.01 1.93-.01 2.2 0 .21.15.46.55.38A8.013 8.013 0 0 0 16 8c0-4.42-3.58-8-8-8z"
                    ></path>
                </svg>
            """,  # noqa: E501
            "class": "",
        },
    ],
}

autodoc_member_order = "bysource"


# -- Hawkmoth config

CONF = Path(__file__)
DOCS = CONF.parent
USERSPACE = DOCS.parent / "users"
ELPEKENIN_USERSPACE = USERSPACE / "elpekenin"
QMK = USERSPACE.parent
LIB = QMK / "lib"
MODULES = QMK / "modules"
ELPEKENIN_MODULE = MODULES / "elpekenin"
CHIBIOS = LIB / "chibios" / "os"
ACCESS = QMK / "keyboards" / "elpekenin" / "access"

TARGET = "right"

hawkmoth_root = str(QMK)


class UriGenerator:
    """Convenience class to generate permalinks to source code.

    It also handles providing links to 2 different repositories.

    Provide a `.format` function to match how hawkmoth uses it.
    It is supposed to be a template string, but we are cooler :)

    FIXME(elpekenin): Prone to breaking if hawkmoth's implementation changes.
    """

    def __init__(self) -> None:
        self.userspace_template = "https://github.com/elpekenin/qmk_userspace/blob/{version}/users/elpekenin/{source}#L{line}"
        self.modules_template = (
            "https://github.com/elpekenin/qmk_modules/blob/{version}/{source}#L{line}"
        )

    def get_commit(self, repo: Repository) -> str:
        # the files copied into $QMK when building docs are not proper git repositories
        # as such, we can't fetch the commit with stuff like `subprocess.run("git rev-parse HEAD", cwd=...)`
        #
        # instead, the commands are invoked inside `make docs` (runs in $USERSPACE), and passed
        # into `sphinx` (runs in $QMK, after copying files there) via these environment variables
        if repo == "userspace":
            return os.environ["USERSPACE_COMMIT"]

        if repo == "module":
            return os.environ["MODULE_COMMIT"]

        msg = f"Unexpected value for repo: '{repo}'"
        raise RuntimeError(msg)

    def format(self, **kwargs: object) -> str:
        """Create a link."""
        # need to make it an absolute path in order to compare with `is_relative_to`
        source = QMK / Path(cast(str, kwargs["source"]))

        if "version" in kwargs:
            print("WARNING: UriGenerator overwriting `kwargs['version']`")

        if source.is_relative_to(ELPEKENIN_USERSPACE):
            kwargs.update(
                version=self.get_commit("userspace"),
            )
            return self.userspace_template.format(**kwargs)

        if source.is_relative_to(ELPEKENIN_MODULE):
            # need to remap source, it starts with "module/elpekenin"
            # but the path in its dedicated repository drops that part
            kwargs.update(
                source=source.relative_to(ELPEKENIN_MODULE),
                version=self.get_commit("module"),
            )
            return self.modules_template.format(**kwargs)

        msg = f"Unknown source: '{source}'"
        raise RuntimeError(msg)


hawkmoth_source_uri = UriGenerator()

# NOTE: sphinx (un)pickles this class. the operation failed due to `pickle`
# not finding the class' name in the `builtins` namespace (i have no idea how or
# why this happens)
# tried to come up with a proper solution... this is the only thing i managed to run ^_^
builtins.__dict__["UriGenerator"] = UriGenerator

hawkmoth_napoleon_transform = None  # apply napoleon transform to every docstring

INCLUDE_DIRS = [
    ACCESS,
    ACCESS / "keymaps" / "elpekenin",
    ELPEKENIN_USERSPACE,  # for generated/
    ELPEKENIN_USERSPACE / "include",
    ELPEKENIN_USERSPACE / "3rd_party" / "backtrace" / "include",
    QMK,  # quantum/
    QMK / "platforms",
    QMK / "platforms" / "chibios",
    QMK / "platforms" / "chibios" / "boards" / "common" / "configs",
    QMK / "quantum",
    QMK / "quantum" / "keymap_extras",
    QMK / "quantum" / "logging",
    QMK / "quantum" / "painter",
    QMK / "quantum" / "process_keycode",
    QMK / "quantum" / "rgb_matrix",
    QMK / "quantum" / "rgb_matrix" / "animations",
    QMK / "quantum" / "send_string",
    QMK / "quantum" / "sequencer",
    QMK / "quantum" / "unicode",
    QMK / "quantum" / "xap",
    QMK / "tmk_core" / "protocol",
    QMK / "tmk_core" / "protocol" / "chibios" / "lufa_utils",
    LIB / "printf" / "src" / "printf",
    # TODO(elpekenin): more headers will be needed here
    ELPEKENIN_MODULE / "ledmap",
    ELPEKENIN_MODULE / "logging",
    CHIBIOS / "hal" / "include",
    CHIBIOS / "hal" / "osal" / "rt-nil",
    CHIBIOS / "license",
    CHIBIOS / "oslib" / "include",
    CHIBIOS / "rt" / "include",
    CHIBIOS / "rt" / "license",
    # FIXME(elpekenin): these are(or may be) based on target
    # might break when (if) we get to use another MCU
    CHIBIOS / "common" / "portability" / "GCC",
    CHIBIOS / "common" / "ext" / "RP" / "RP2040",
    CHIBIOS / "common" / "ports" / "ARM-common",
    CHIBIOS / "common" / "ports" / "ARMv6-M-RP2",
    CHIBIOS / "common" / "startup" / "ARMCMx" / "devices" / "RP2040",
    CHIBIOS / "hal" / "boards" / "RP_PICO_RP2040",
    CHIBIOS / "hal" / "ports" / "common" / "ARMCMx",
    CHIBIOS / "hal" / "ports" / "RP" / "RP2040",
    CHIBIOS / "hal" / "ports" / "RP" / "LLD" / "DMAv1",
    CHIBIOS / "hal" / "ports" / "RP" / "LLD" / "GPIOv1",
    CHIBIOS / "hal" / "ports" / "RP" / "LLD" / "TIMERv1",
    CHIBIOS / "hal" / "ports" / "RP" / "LLD" / "SPIv1",
    CHIBIOS / "various" / "pico_bindings" / "dumb" / "include",
    LIB / "chibios-contrib" / "os" / "hal" / "ports" / "RP" / "LLD" / "USBDv1",
    LIB / "pico-sdk" / "src" / "boards" / "include",
    LIB / "pico-sdk" / "src" / "common" / "pico_base" / "include",
    LIB / "pico-sdk" / "src" / "rp2_common" / "cmsis" / "stub" / "CMSIS" / "Core" / "Include",  # noqa: E501
    LIB / "pico-sdk" / "src" / "rp2_common" / "hardware_base" / "include",
    LIB / "pico-sdk" / "src" / "rp2_common" / "hardware_clocks" / "include",
    LIB / "pico-sdk" / "src" / "rp2_common" / "pico_platform" / "include",
    LIB / "pico-sdk" / "src" / "rp2040" / "hardware_regs" / "include",
    LIB / "pico-sdk" / "src" / "rp2040" / "hardware_structs" / "include",
    QMK / "platforms" / "chibios" / "boards" / "GENERIC_RP_RP2040" / "configs",
    QMK / "platforms" / "chibios" / "vendors" / "RP",  # _pin_defs.h
    # ugly, -e TARGET=value dependent
    QMK / ".build" / f"obj_{TARGET}" / "src",
    # NOTE: could not get ``fmt: skip`` to work on a specific line
    # applying it to every entry of the list sounds like a good idea anyway :)
]  # fmt: skip

CONFIG_H = [
    ACCESS / "config.h",
    ACCESS / "keymaps" / "elpekenin" / "config.h",
    ELPEKENIN_USERSPACE / "config.h",
]

hawkmoth_clang = (
    [f"-I{path}" for path in INCLUDE_DIRS]
    + [f"-include{file}" for file in CONFIG_H]
    + [
        # attributes are a no-op for clang not to mess up
        "-D_ATTR(...)=",
        # silence warning emitted by pico-SDK all over the place
        "-Wno-duplicate-decl-specifier",
        # provided by build system
        "-DMCU_RP",
        "-DPROTOCOL_CHIBIOS",
        "-DINIT_EE_HANDS_RIGHT",
        "-DRGB_MATRIX_LED_COUNT=58",
        "-DAUTOCORRECT_ENABLE",
        "-DEEPROM_WEAR_LEVELING",
        "-DKEYLOG_ENABLE",
        "-DKEY_OVERRIDE_ENABLE",
        "-DQUANTUM_PAINTER_ENABLE",
        "-DRGB_MATRIX_ENABLE",
        "-DSPLIT_ENABLE",
        "-DTAP_DANCE_ENABLE",
        "-DTOUCH_SCREEN_ENABLE",
        "-DTRI_LAYER_ENABLE",
        "-DUCIS_ENABLE",
        "-DUNICODE_COMMON_ENABLE",
        "-DWPM_ENABLE",
        "-DXAP_ENABLE",
    ]
)
