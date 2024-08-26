const std = @import("std");

// TODO: convert qgf.{c,h} to qgf (same with qff)
// and call load_mem from here
// const assets = @import("assets");

// const c = @cImport({
//     @cInclude("quantum/painter/qp.h");

//     @cInclude("elpekenin.h");
//     @cInclude("elpekenin/logging.h");
//     @cInclude("elpekenin/qp/graphics.h");
// });

const modules = .{
    @import("painter.zig"),
};

export fn elpekenin_zig_init() void {
    inline for (modules) |module| {
        if (@hasDecl(module, "init")) {
            module.init();
        }
    }
}

export fn elpekenin_zig_deinit(bootloader: bool) void {
    inline for (modules) |module| {
        if (@hasField(module, "deinit")) {
            module.deinit(bootloader);
        }
    }
}
