CUSTOM_MATRIX = lite
SRC += matrix.c

# debug over UART
UART_DRIVER_REQUIRED = yes

# Custom features
SIPO_PINS = yes

# BOARD = GENERIC_PROMICRO_RP2040  # default
# BOARD = GENERIC_RP_RP2040  # remove __late_init and some pinctrl settings

# built with 16M Pico's
EXTRALDFLAGS = -Wl,--defsym,FLASH_LEN=16384k
