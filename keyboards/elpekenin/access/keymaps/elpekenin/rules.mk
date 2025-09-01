# community modules (no Kconfig)
ALLOCATOR_WRAP_STD := yes

KEYMAP_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

SRC += $(KEYMAP_DIR)/ui.c
