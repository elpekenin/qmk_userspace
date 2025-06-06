UI := $(USER_SRC)/qp/ui

ifeq ($(strip $(QUANTUM_PAINTER_ENABLE)), yes)
    SRC += $(USER_SRC)/qp/assets.c

    SRC += \
        $(UI)/common.c \
        $(UI)/computer_stats.c \
        $(UI)/flash.c \
        $(UI)/github_notifications.c \
        $(UI)/heap.c \
        $(UI)/keylog.c \
        $(UI)/layer.c \
        $(UI)/uptime.c
endif
