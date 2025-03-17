*******
Modules
*******

Custom features exposed as Community Modules

Refer to `QMK's documentation <https://docs.qmk.fm/features/community_modules>`_ for details on how to use this feature

crash
#####
.. c:autodoc:: modules/elpekenin/crash/elpekenin/crash.h

dual_rp
#######
.. c:autodoc:: modules/elpekenin/dual_rp/elpekenin/dual_rp.h

indicators
##########
.. c:autodoc:: modules/elpekenin/indicators/elpekenin/indicators.h

ledmap
######
.. c:autodoc:: modules/elpekenin/ledmap/elpekenin/ledmap.h

logging
#######
.. c:autodoc:: modules/elpekenin/logging/elpekenin/logging.h

micropython
###########

Embeds microypthon's interpreter into your board

Since that's not very useful by itself, a module is provided to hook into QMK APIs. Namely, you use: ``import qmk``

To execute some code, store it as a string and feed it into the ``mp_embed_exec_str`` function

.. important::
    In the future, I plan to integrate ``import ...`` with QMK filesystem API (not a thing yet)

    When (if) this happens, you would be able to run dynamic code, opposed to the current state, where you can only execute something defined as a ``const char*`` at compile time

..
    notes to self
        :lines: to skip "generated file" heading comment

Available APIs are:

.. tabs::

    .. tab:: ``qmk``

        .. literalinclude:: ../modules/elpekenin/micropython/user_c_modules/qmk/stubs/qmk.pyi
            :lines: 5-

    .. tab:: ``qmk.keycode``

        .. literalinclude:: ../modules/elpekenin/micropython/user_c_modules/qmk/stubs/_keycode.pyi
            :lines: 5-

    .. tab:: ``qmk.rgb``

        .. literalinclude:: ../modules/elpekenin/micropython/user_c_modules/qmk/stubs/_rgb.pyi
            :lines: 5-

rng
###
.. c:autodoc:: modules/elpekenin/rng/elpekenin/rng.h


rp_sdk_wrappers
###############

Replaces some C builtins (eg: ``__clzsi2``) with fine-tuned versions in the ROM of RP2040

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

scrolling_text
##############
.. c:autodoc:: modules/elpekenin/scrolling_text/elpekenin/scrolling_text.h
