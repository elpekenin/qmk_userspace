# Compile into a static library, and link it against QMK by means of SRC
$(shell cd $(USER_SRC) && zig build -Doptimize=ReleaseFast -Dmcu=$(MCU_SERIES))
SRC += $(USER_PATH)/zig-out/lib/libelpekenin.a
