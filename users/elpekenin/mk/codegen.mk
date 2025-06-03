USER_GENERATED := $(USER_PATH)/generated

USERSPACE_CLI := euc
CODEGEN_ARGS := --output $(USER_GENERATED)

QP_DIRS := $(KEYBOARD_PATHS) $(KEYMAP_PATH) $(USER_PATH)

# make sure folder exists
$(shell mkdir -p $(USER_GENERATED))

# hack to force running other steps
FORCE:

# enabled_features_t
$(USER_GENERATED)/features.c: FORCE
	$(USERSPACE_CLI) features $(CODEGEN_ARGS)

# QP assets
ifeq ($(strip $(QUANTUM_PAINTER_ENABLE)), yes)
    # HACK: eagerly run command, so that we can `include ...mk`
    # $(USER_GENERATED)/qp_resources.c:
    $(shell $(USERSPACE_CLI) qp_resources $(CODEGEN_ARGS) $(QP_DIRS))

    SRC += $(USER_GENERATED)/qp_resources.c
    include $(USER_GENERATED)/qp_resources.mk
endif
