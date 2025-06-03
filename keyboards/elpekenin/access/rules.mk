CUSTOM_MATRIX = lite
SRC += matrix.c

# built with 2MB Pico's
OPT_DEFS += "-DPICO_FLASH_SIZE_BYTES=(2 * 1024 * 1024)" # default already (?)
LDFLAGS += -Wl,--defsym,FLASH_LEN=2048k  # not required (for now)

# QP drivers
QUANTUM_PAINTER_DRIVERS += \
    il91874_spi \
    ili9163_spi \
    ili9341_spi \
    surface
