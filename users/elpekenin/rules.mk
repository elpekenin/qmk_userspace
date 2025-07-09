MK_PATH = $(USER_PATH)/mk
USER_SRC = $(USER_PATH)/src
USER_INCLUDE = $(USER_PATH)/include

VPATH += $(USER_INCLUDE)

SRC += \
    $(USER_SRC)/autoconf_rt.c \
    $(USER_SRC)/elpekenin.c \
    $(USER_SRC)/placeholders.c \
    $(USER_SRC)/layers.c \
    $(USER_SRC)/power.c \
    $(USER_SRC)/process.c

INTROSPECTION_KEYMAP_C = $(USER_SRC)/introspection.c

# first because it contains configuration for other features
include $(MK_PATH)/kconfig.mk

# include unconditionally, `ifeq`'s in the files
include $(MK_PATH)/codegen.mk
include $(MK_PATH)/custom_features.mk
include $(MK_PATH)/logging.mk
include $(MK_PATH)/painter.mk
include $(MK_PATH)/rgb_matrix.mk
include $(MK_PATH)/split.mk
include $(MK_PATH)/xap.mk
include $(MK_PATH)/mcu.mk
