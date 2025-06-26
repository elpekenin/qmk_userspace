.. include:: _hint.rst

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
