MK_PATH = $(USER_PATH)/mk
USER_SRC = $(USER_PATH)/src
USER_INCLUDE = $(USER_PATH)/include

VPATH += $(USER_INCLUDE)

# do not error if an attribute is unknown
CFLAGS += -Wno-attributes

SRC += $(USER_SRC)/build_info.c \
       $(USER_SRC)/crash.c \
       $(USER_SRC)/elpekenin.c \
       $(USER_SRC)/placeholders.c \
       $(USER_SRC)/layers.c \
       $(USER_SRC)/power.c \
       $(USER_SRC)/process.c

# Default configuration
AUTOCORRECT_ENABLE ?= yes
DEFERRED_EXEC_ENABLE ?= yes
KEY_OVERRIDE_ENABLE ?= yes
TRI_LAYER_ENABLE ?= yes
INTROSPECTION_KEYMAP_C = $(USER_SRC)/introspection.c

# Include everything, checks are inside the files
include $(MK_PATH)/codegen.mk
include $(MK_PATH)/custom_features.mk
include $(MK_PATH)/games.mk
include $(MK_PATH)/logging.mk
include $(MK_PATH)/painter.mk
include $(MK_PATH)/rgb_matrix.mk
include $(MK_PATH)/rng.mk
include $(MK_PATH)/split.mk
include $(MK_PATH)/unicode.mk
include $(MK_PATH)/utils.mk
include $(MK_PATH)/wrappers.mk
include $(MK_PATH)/xap.mk
include $(MK_PATH)/3rd_party.mk
include $(MK_PATH)/mcu.mk
include $(MK_PATH)/zig.mk

# downloaded (sometimes) using qmk_build
-include $(MK_PATH)/rtc.mk
