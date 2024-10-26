# Compile into a static library, and link it against QMK by means of SRC
$(shell cd $(USER_PATH) \
  && zig build -Doptimize=ReleaseFast -Dmcu=$(MCU_SERIES) > $(LOG_FOLDER)/zig.out 2> $(LOG_FOLDER)/zig.err)

# $(if $(ZIG_RET),,$(error Compiling zig failed))
SRC += $(USER_PATH)/zig-out/lib/libelpekenin.a
