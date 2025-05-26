USER_GENERATED := $(USER_PATH)/generated

USERSPACE_CLI := euc
CODEGEN_ARGS := --output $(USER_GENERATED)

QP_DIRS := $(KEYBOARD_PATHS) $(KEYMAP_PATH) $(USER_PATH)

# TODO: replace this target (always run) by actual dependencies on the generated files' targets
FORCE:

# make sure folder exists
$(USER_GENERATED): FORCE
	@mkdir -p $(USER_GENERATED)

# enabled_features_t
$(USER_GENERATED)/features.c: FORCE
	@$(USERSPACE_CLI) features $(CODEGEN_ARGS)

$(USER_GENERATED)/qp_resources.c $(USER_GENERATED)/features_draw.c: FORCE
	@$(USERSPACE_CLI) qp_resources $(CODEGEN_ARGS) $(QP_DIRS)

SRC += $(USER_GENERATED)/features.c

# QP assets
ifeq ($(strip $(QUANTUM_PAINTER_ENABLE)), yes)
    SRC += \
        $(USER_GENERATED)/qp_resources.c \
        $(USER_GENERATED)/features_draw.c
    include $(USER_GENERATED)/qp_resources.mk
endif
