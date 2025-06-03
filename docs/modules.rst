*******
Modules
*******

Custom features exposed as Community Modules

Refer to `QMK's documentation <https://docs.qmk.fm/features/community_modules>`_ for details on how to use this feature

allocator
#########
.. c:autodoc:: modules/elpekenin/allocator/elpekenin/allocator.h

arm_math_m0
###########

Replaces some C builtins for floating point numbers (eg: ``sinf``) with optimized implementations for Cortex-M0 and M0+ cores.

build_id
########
.. c:autodoc:: modules/elpekenin/build_id/elpekenin/build_id.h

crash
#####
.. c:autodoc:: modules/elpekenin/crash/elpekenin/crash.h

dual_rp
#######
.. c:autodoc:: modules/elpekenin/dual_rp/elpekenin/dual_rp.h

generics
########
.. c:autodoc:: modules/elpekenin/generics/elpekenin/generics.h

glitch_text
###########
.. c:autodoc:: modules/elpekenin/glitch_text/elpekenin/glitch_text.h

indicators
##########
.. c:autodoc:: modules/elpekenin/indicators/elpekenin/indicators.h

ledmap
######
.. c:autodoc:: modules/elpekenin/ledmap/elpekenin/ledmap.h

logging
#######
.. c:autodoc:: modules/elpekenin/logging/elpekenin/logging.h

ls0xx
#####
Support for LS0XX displays (eg: nice!view), extracted from my (quite abandoned) PR.

.. attention::
    Might contain some bug that I haven't found yet.
    Please report if you find something.

    Since this driver is not part of QMK's codebase, some integrations won't work,
    namely, the auto-flushing and the power on/off of (in)activity are not applied
    to this device.

.. c:autodoc:: modules/elpekenin/ls0xx/ls0xx.h

memory
######
.. c:autodoc:: modules/elpekenin/memory/elpekenin/memory.h

micropython
###########

Embeds microypthon's interpreter into your board

Since that's not very useful by itself, a module is provided to hook into QMK APIs. Namely, you use: ``import qmk``

To execute some code, store it as a string and feed it into the ``mp_embed_exec_str`` function

.. tip::
    Writing a C-string directly is not convenient (eg: no IntelliSense for suggestions/typo detection)

    To circumvent that, you can write a regular ``.py`` file, and then use ``euc py2c <file.py>`` to generate
    the equivalent string in a standalone file, which you can later ``#include``

    You could also point your editor at the ``qmk`` folder, and get auto-completion thanks to the ``.pyi`` files in it

.. important::
    In the future, I plan to integrate ``import ...`` with QMK filesystem API (not a thing yet)

    When (if) this happens, you would be able to run dynamic code, opposed to the current state, where you can only execute something defined as a ``const char*`` at compile time

..
    notes to self
        :lines: to skip "generated file" heading comment

Available APIs are:

.. tabs::

    .. tab:: ``qmk``

        .. literalinclude:: ../modules/elpekenin/micropython/user_c_modules/qmk/__init__.pyi
            :lines: 6-

    .. tab:: ``qmk.keycode``

        .. literalinclude:: ../modules/elpekenin/micropython/user_c_modules/qmk/keycode.pyi
            :lines: 6-

    .. tab:: ``qmk.rgb``

        .. literalinclude:: ../modules/elpekenin/micropython/user_c_modules/qmk/rgb.pyi
            :lines: 6-

rng
###
.. c:autodoc:: modules/elpekenin/rng/elpekenin/rng.h


rp_sdk_wrappers
###############

Replaces several C builtins (eg: ``__clzsi2``) with optimized implementations provided in the ROM of RP2040

There are no hooks for user logic in this module because its only purpose is setting up such functions

.. warning::
    To set this module up you need to define a custom linker script

    This is due to Pico-SDK using a section to reference the functions required to configure these replacements

    To do that, you can add this snippet at the end of the default linker script for RP2040 in ChibiOS

    .. code-block::

        SECTIONS {
            .preinit : ALIGN(4){
                . = ALIGN(4);
                __preinit_array_base__ = .;
                KEEP(*(SORT(.preinit_array.*)))
                KEEP(*(.preinit_array))
                __preinit_array_end__ = .;
            } > XTORS_FLASH AT > XTORS_FLASH_LMA
        }

sanitizer
#########
.. c:autodoc:: modules/elpekenin/sanitizer/elpekenin/sanitizer/kasan.h


scrolling_text
##############
.. c:autodoc:: modules/elpekenin/scrolling_text/elpekenin/scrolling_text.h

string
######
.. c:autodoc:: modules/elpekenin/string/elpekenin/string.h

version
#######
Adds ``QMK_MAJOR``, ``QMK_MINOR`` and ``QMK_PATCH`` macros to check for the version of QMK in which you are building.

These are available under ``#include "qmk_version.h"``.
