# Compile into a static library, and link it against QMK by means of SRC
ZIG_RET := $(shell cd $(USER_PATH) && zig build -Doptimize=ReleaseFast -Dmcu=$(MCU_SERIES) > zig.log 2>&1 )
# $(if $(ZIG_RET),,$(error Compiling zig failed))
SRC += $(USER_PATH)/zig-out/lib/libelpekenin.a
