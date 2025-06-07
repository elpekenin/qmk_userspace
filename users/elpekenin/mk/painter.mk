UI := $(USER_SRC)/qp/ui

ifeq ($(strip $(QUANTUM_PAINTER_ENABLE)), yes)
    SRC += $(USER_SRC)/qp/assets.c

    SRC += \
        $(UI)/computer.c \
        $(UI)/firmware_id.c \
        $(UI)/flash.c \
        $(UI)/github.c \
        $(UI)/heap.c \
        $(UI)/keylog.c \
        $(UI)/layer.c \
        $(UI)/uptime.c
endif
