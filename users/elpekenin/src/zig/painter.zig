//! Bindings to QP

const std = @import("std");

const logging = @import("logging.zig");

const c = @cImport(
    @cInclude("quantum/painter/qp.h"),
);

const AutoArrayHashMap = std.array_hash_map.AutoArrayHashMap;
const Device = c.painter_device_t;
const Font = c.painter_font_handle_t;
const Image = c.painter_image_handle_t;

const CStr = [*:0]const u8;
fn Map(V: type, export_name: []const u8, allocator: std.mem.Allocator) type {
    return struct {
        var map = AutoArrayHashMap(CStr, V).init(allocator);

        fn set(name: CStr, item: V) callconv(.C) void {
            map.put(name, item) catch |e| {
                logging.warn(.QP, "Could not save '%s': (%p) -- (%s)", .{name, item, @errorName(e).ptr});
                return;
            };

            logging.info(.QP, "Save '%s': (%p)", .{name, item});
        }

        fn get(name: CStr) callconv(.C) V {
            const val = map.get(name) orelse null;
            logging.info(.QP, "Read '%s': (%p)", .{name, val});
            return val;
        }

        comptime {
            @export(set, .{
                .name = "qp_set_" ++ export_name ++ "_by_name",
                .linkage = .strong,
            });

            @export(get, .{
                .name = "qp_get_" ++ export_name ++ "_by_name",
                .linkage = .strong,
            });
        }
    };
}

const alloc = std.heap.c_allocator;
const Devices = Map(Device, "device", alloc);
const Fonts = Map(Font, "font", alloc);
const Images = Map(Image, "image", alloc);

pub fn init() void {
    // do not optimize away please
    _ = Devices;
    _ = Fonts;
    _ = Images;
}

pub fn deinit(bootloader: bool) void {
    _ = bootloader;

    for (Devices.map.values()) |display| {
        _ = c.qp_rect(display, 0, 0, c.qp_get_width(display), c.qp_get_height(display), 0, 0, 0, true);
        _ = c.qp_power(display, false);
    }

    // TODO(elpekenin): close font/images
}
