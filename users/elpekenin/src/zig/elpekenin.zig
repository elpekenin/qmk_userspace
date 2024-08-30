const std = @import("std");
const builtin = @import("builtin");

// TODO: convert qgf.{c,h} to qgf (same with qff)
// and call load_mem from here
// const assets = @import("assets");

const modules = .{
    @import("painter.zig"),
};

export fn elpekenin_zig_init() void {
    inline for (modules) |module| {
        module.init();
    }
}

export fn elpekenin_zig_deinit(bootloader: bool) void {
    inline for (modules) |module| {
        module.deinit(bootloader);
    }
}

export fn zig_main() void {
    if (builtin.target.isGnu()) {
        @import("logging.zig").err(
            .QP,
            "Hello %d\n",
            .{
                @as(u32, 1234),
            },
        );
    }
}