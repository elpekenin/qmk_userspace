USER_GENERATED = $(USER_PATH)/generated

USERSPACE_CLI = euc
CODEGEN_ARGS = --output $(USER_GENERATED)

# make sure folder exists
$(shell mkdir -p $(USER_GENERATED))

# enabled_features_t
$(USER_GENERATED)/features.c: FORCE
	$(shell $(USERSPACE_CLI) features $(CODEGEN_ARGS))
SRC += $(USER_GENERATED)/features.c

# NOTE: keycode_str uses core now

# QP assets
ifeq ($(strip $(QUANTUM_PAINTER_ENABLE)), yes)
    QP_DIRS := $(KEYBOARD_PATHS) $(KEYMAP_PATH) $(USER_PATH)

    $(USER_GENERATED)/qp_resources.c $(USER_GENERATED)/features_draw.c: FORCE
    	$(shell $(USERSPACE_CLI) qp_resources $(CODEGEN_ARGS) $(QP_DIRS))
    SRC += $(USER_GENERATED)/qp_resources.c $(USER_GENERATED)/features_draw.c

    include $(USER_GENERATED)/qp_resources.mk
endif

# TODO: remove this target (always run) by actual dependencies on the generated files' targets
FORCE:
