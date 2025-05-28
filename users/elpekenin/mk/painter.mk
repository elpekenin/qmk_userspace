QP_TASKS := $(USER_SRC)/qp/tasks

ifeq ($(strip $(QUANTUM_PAINTER_ENABLE)), yes)
    SRC += $(USER_SRC)/qp/assets.c

    ifeq ($(strip $(QP_TASK_COMPUTER_STATS_ENABLE)), yes)
        SRC += $(QP_TASKS)/computer_stats.c
    endif

    ifeq ($(strip $(QP_TASK_GITHUB_NOTIFICATIONS_ENABLE)), yes)
        SRC += $(QP_TASKS)/github_notifications.c
    endif

    ifeq ($(strip $(QP_TASK_HEAP_STATS_ENABLE)), yes)
        SRC += $(QP_TASKS)/heap_stats.c
    endif

    ifeq ($(strip $(QP_TASK_KEYLOG_ENABLE)), yes)
        SRC += $(QP_TASKS)/keylog.c
    endif

    ifeq ($(strip $(QP_TASK_LAYER_ENABLE)), yes)
        SRC += $(QP_TASKS)/layer.c
    endif

    ifeq ($(strip $(QP_TASK_LOGGING_ENABLE)), yes)
        SRC += $(QP_TASKS)/logging.c
    endif

    ifeq ($(strip $(QP_TASK_UPTIME_ENABLE)), yes)
        SRC += $(QP_TASKS)/uptime.c
    endif
endif
