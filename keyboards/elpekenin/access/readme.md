# access

![access](imgur.com image replace me!)

*A short description of the keyboard/project*

* Keyboard Maintainer: [Pablo Martinez](https://github.com/elpekenin)
* Hardware Supported: The PCBs use RP2040s
* Hardware Availability: https://github.com/elpekenin/access_kb

Make example for this keyboard (after setting up your build environment):

    make elpekenin/access:default
    qmk compile -kb elpekenin/access -km default

Flashing:

    Put your board in bootloader mode and copy the `.uf2` file on the drive that appears on your file explorer

See the [build environment setup](https://docs.qmk.fm/#/getting_started_build_tools) and the [make instructions](https://docs.qmk.fm/#/getting_started_make_guide) for more information. Brand new to QMK? Start with our [Complete Newbs Guide](https://docs.qmk.fm/#/newbs).

## Bootloader

Enter the bootloader in 4 ways:

* **Bootmagic reset**: Hold down the key at (0,0) in the matrix (Escape) and plug in the keyboard
* **Physical reset button**: Hold the button on the back of the PCB and plug in the keyboard
* **Keycode in layout**: Press the key mapped to `QK_BOOT` if it is available
