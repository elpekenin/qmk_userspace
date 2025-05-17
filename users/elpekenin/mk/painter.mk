ifeq ($(strip $(QUANTUM_PAINTER_ENABLE)), yes)
    SRC += \
        $(USER_SRC)/qp/assets.c \
        $(USER_SRC)/qp/tasks/computer_stats.c \
        $(USER_SRC)/qp/tasks/github_notifications.c \
        $(USER_SRC)/qp/tasks/heap_stats.c \
        $(USER_SRC)/qp/tasks/keylog.c \
        $(USER_SRC)/qp/tasks/layer.c \
        $(USER_SRC)/qp/tasks/logging.c \
        $(USER_SRC)/qp/tasks/uptime.c
endif
