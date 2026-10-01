# QMK Userspace

This is a template repository which allows for an external set of QMK keymaps to be defined and compiled. This is useful for users who want to maintain their own keymaps without having to fork the [main QMK repository](https://github.com/qmk/qmk_firmware). You must still fork the main QMK repository if writing firmware for a *new* keyboard.

## :warning: Not a real fork of `qmk_userspace` :warning:

[![pre-commit.ci status](https://results.pre-commit.ci/badge/github/pre-commit/pre-commit.com/main.svg)](https://results.pre-commit.ci/latest/github/elpekenin/qmk_userspace/main)

This repository uses a custom build system to compile my firmware, because by board is a 1-off I built for myself and will not be PR'ed to `qmk_firmware`.

This is only a fork of `qmk_userspace` on GitHub for visibility reasons (perhaps somebody runs into it and finds something useful)
