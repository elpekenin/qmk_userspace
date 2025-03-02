elpekenin's userspace
=====================

.. caution::
  This is not an usual QMK external userspace. Instead I use a custom tool  (`qmk_build <https://github.com/elpekenin/qmk_build>`_) to compile.
  The reason for this convoluted setup is to have an easy(TM) way of  keeping me code out of tree while easily updating the QMK's core code.
  Having a fork wasn't good enough for me.

----

The structure is similar to a usual QMK repository:

* ``keyboards/elpekenin/access`` contains source for my custom PCB, and my keymap inside it.
* ``users/elpekenin`` contains most of the logic, in case I want to reuse on other boards in the future. Structured as:

  * ``src``: actual source (mostly C, but a bit of zig too).
  * ``include``: header files.
  * ``mk``: Makefile logic, broken into "small" sections.
  * ``scripts``: Python that generates code at compile time.
  * ``painter``: `Quantum Painter <https://docs.qmk.fm/quantum_painter>`_ assets.
  * ``autocorrect``: database of corrections.
  * ``3rd_party``: a couple of external dependencies that are used.
  * ``docs``: files used to generate this page.

* Then, ``patches`` and ``build.json`` will put everything together into a clone of ``qmk_firmware`` to build.

`Source code <https://github.com/elpekenin/qmk_userspace>`_.

----

This documentation is built using:

* `Sphinx <https://www.sphinx-doc.org/en/master/>`_ . Documentation framework.
* `Furo <https://pradyunsg.me/furo/quickstart/>`_. Theme for Sphinx styling.
* `Hawkmoth <https://hawkmoth.readthedocs.io/en/stable/installation.html>`_. Plugin to work with C code in Sphinx.
* Of course, some writing in ``.rst`` files

Keep in mind that docs might be out of date with the actual code. However I've put quite some effort in generating it automatically from
doc-comments in the code, in hopes to reduce this friction. If you find anything out-of-sync please let me know.


.. toctree::
    :hidden:

    src/_index
    headers/_index
    utils/_index
