#############
elpekenin QMK
#############

.. caution::
  This is not an usual QMK external userspace

  I use a custom tool  (`qmk_build <https://github.com/elpekenin/qmk_build>`_) to compile

  Reasoning for this setup is easily keeping my code out of tree, but up to date with QMK changes. A fork wasn't good enough for me

************
Introduction
************

This documentation is built using

* `Sphinx <https://www.sphinx-doc.org/en/master/>`_: Documentation framework
* `Furo <https://pradyunsg.me/furo/quickstart/>`_: Theme for Sphinx styling
* `Hawkmoth <https://hawkmoth.readthedocs.io/en/stable/installation.html>`_: Plugin to work with C code in Sphinx
* Of course, some writing in ``.rst`` files

Keep in mind that docs might be out of date with the actual code. However I've put quite some effort in generating it automatically from
doc-comments in the code, in hopes to reduce this friction. If you find anything out-of-sync please let me know

**************
File structure
**************

* docs: files used to generate this page
* keyboards/elpekenin/access: contains source for my custom PCB, and my keymap inside it
* modules/elpekenin: submodule pointing to my community modules
* users/elpekenin: most of the code, structured as

  * src: source code
  * include: header files
  * mk: Makefile logic, broken into "small" sections
  * scripts: Python that generates code at compile time
  * painter: `Quantum Painter <https://docs.qmk.fm/quantum_painter>`_ assets
  * autocorrect: database of corrections
  * 3rd_party: a couple of external dependencies that are used

* Then, ``patches/`` and ``build.json`` will put everything together into a clone of ``qmk_firmware`` to build

*********
Userspace
*********
``elpekenin.c`` contains some cool (yet complex code)

Namely, it invokes initializer and finalizer functions, registered by other files

These are stored in special linker sections. See `sections` for details

Take a look at your own risk

elpekenin/allocator.h
#####################
.. c:autodoc:: users/elpekenin/include/elpekenin/allocator.h

elpekenin/build_info.h
######################
.. c:autodoc:: users/elpekenin/include/elpekenin/build_info.h

elpekenin/compiler.h
####################
.. c:autodoc:: users/elpekenin/include/elpekenin/compiler.h

elpekenin/dyn_array.h
#####################
.. c:autodoc:: users/elpekenin/include/elpekenin/dyn_array.h

elpekenin/keylog.h
##################
.. c:autodoc:: users/elpekenin/include/elpekenin/keylog.h

elpekenin/map.h
###############
.. c:autodoc:: users/elpekenin/include/elpekenin/map.h

elpekenin/memory.h
##################
.. c:autodoc:: users/elpekenin/include/elpekenin/memory.h

elpekenin/ring_buffer.h
#######################
.. c:autodoc:: users/elpekenin/include/elpekenin/ring_buffer.h

elpekenin/sections.h
####################
.. c:autodoc:: users/elpekenin/include/elpekenin/sections.h

elpekenin/shortcuts.h
#####################
.. c:autodoc:: users/elpekenin/include/elpekenin/shortcuts.h

elpekenin/sipo.h
################
.. c:autodoc:: users/elpekenin/include/elpekenin/sipo.h

elpekenin/spi_custom.h
######################
.. c:autodoc:: users/elpekenin/include/elpekenin/spi_custom.h

elpekenin/touch.h
#################
.. c:autodoc:: users/elpekenin/include/elpekenin/touch.h

elpekenin/xap.h
###############
.. c:autodoc:: users/elpekenin/include/elpekenin/xap.h

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
        start at line 3 to skip "generated file" comment + empty line below it

Available APIs are:

.. tabs::

    .. tab:: ``qmk``

        .. literalinclude:: ../modules/elpekenin/micropython/user_c_modules/qmk/stubs/qmk.pyi
            :lines: 3-

    .. tab:: ``qmk.keycode``

        .. literalinclude:: ../modules/elpekenin/micropython/user_c_modules/qmk/stubs/_keycode.pyi
            :lines: 3-

    .. tab:: ``qmk.rgb``

        .. literalinclude:: ../modules/elpekenin/micropython/user_c_modules/qmk/stubs/_rgb.pyi
            :lines: 3-

rng
###
.. c:autodoc:: modules/elpekenin/rng/elpekenin/rng.h


sdk_wrappers
############

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
