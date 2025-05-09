CUSTOM_MATRIX = lite
SRC += matrix.c

# debug over UART
UART_DRIVER_REQUIRED = yes

# Custom features
SIPO_PINS = yes

# BOARD = GENERIC_PROMICRO_RP2040  # default
# BOARD = GENERIC_RP_RP2040  # remove __late_init and some pinctrl settings

# built with 2MB Pico's
OPT_DEFS += "-DPICO_FLASH_SIZE_BYTES=(2 * 1024 * 1024)" # default already (?)
EXTRALDFLAGS += -Wl,--defsym,FLASH_LEN=2048k  # not required (for now)

# QP drivers
QUANTUM_PAINTER_DRIVERS += il91874_spi  # left half
QUANTUM_PAINTER_DRIVERS += ili9163_spi ili9341_spi surface  # right half

ifeq ($(TARGET), left)
    OPT_DEFS += -DLEFT_HAND
else ifeq ($(TARGET), right)
    OPT_DEFS += -DRIGHT_HAND
else
    $(error Invalid value for TARGET environment variable)
endif
