//! Bindings to QP

const std = @import("std");
const log = @import("logging.zig").logging;

extern fn qp_get_width(Device) u16;
extern fn qp_get_height(Device) u16;
extern fn qp_rect(Device, u16, u16, u16, u16, u8, u8, u8, bool) bool;
extern fn qp_power(Device, bool) bool;

const AutoArrayHashMap = std.array_hash_map.AutoArrayHashMap;
const Device = *const void;
const Font = *const void;
const Image = *const void;

const CStr = [*:0]const u8;
fn Map(V: type, export_name: []const u8, allocator: std.mem.Allocator) type {
    return struct {
        var map = AutoArrayHashMap(CStr, V).init(allocator);

        fn set(name: CStr, item: V) callconv(.C) void {
            map.put(name, item) catch |e| {
                _ = log(.QP, .WARN, "Could not save '%s': (%p) -- (%s)", name, item, @errorName(e).ptr);
                return;
            };

            _ = log(.QP, .INFO, "Save '%s': (%p)", name, item);
        }

        fn get(name: CStr) callconv(.C) ?V {
            const val = map.get(name);
            _ = log(.QP, .INFO, "Read '%s': (%p)", name, val);
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
        _ = qp_rect(display, 0, 0, qp_get_width(display), qp_get_height(display), 0, 0, 0, true);
        _ = qp_power(display, false);
    }

    // TODO(elpekenin): close font/images
}
