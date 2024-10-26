const std = @import("std");

const MCU = struct {
    name: []const u8,
    target: std.Target.Query,
};

const KnownMcus = [_]MCU{
    .{
        .name = "RP2040",
        .target = .{
            .cpu_arch = .thumb,
            .cpu_model = .{ .explicit = &std.Target.arm.cpu.cortex_m0plus },
            .os_tag = .freestanding,
            .abi = .eabi,
        },
    },
    .{ // to run tests on computer
        .name = "native",
        .target = .{}, // default to native
    },
};

fn getTarget(mcu: []const u8) std.Target.Query {
    inline for (KnownMcus) |known_mcu| {
        if (std.mem.eql(u8, mcu, known_mcu.name)) {
            return known_mcu.target;
        }
    }

    std.debug.panic("Unknown MCU: {s}", .{mcu});
}

fn getFiles(b: *std.Build, options: *std.Build.Step.Options, path: []const u8, needle: []const u8) !void {
    var dir = try std.fs.openDirAbsolute(path, .{ .iterate = true });
    defer dir.close();

    var walker = try dir.walk(b.allocator);
    while (try walker.next()) |file| {
        if (file.kind == .file) {
            if (std.mem.endsWith(u8, file.basename, needle)) {
                const fd = try dir.openFile(file.path, .{});
                defer fd.close();

                const stat = try fd.stat();

                const buff = try b.allocator.alloc(u8, stat.size);
                defer b.allocator.free(buff);

                _ = try fd.read(buff);

                options.addOption([]const u8, file.basename, buff);
            }
        }
    }
}

fn getAssets(b: *std.Build) !*std.Build.Step.Options {
    const options = b.addOptions();
    try getFiles(b, options, b.pathJoin(&.{ b.build_root.path orelse @panic("No root"), "painter", "images" }), "qgf.c");
    try getFiles(b, options, b.pathJoin(&.{ b.build_root.path orelse @panic("No root"), "painter", "fonts" }), "qff");
    return options;
}

pub fn build(b: *std.Build) !void {
    const mcu = b.option([]const u8, "mcu", "MCU to compile for") orelse @panic("Select a MCU");
    const target = b.resolveTargetQuery(getTarget(mcu));

    const optimize = b.standardOptimizeOption(.{});

    const elpekenin = ".";
    const users = elpekenin ++ "/..";
    const qmk = users ++ "/..";
    const platforms = qmk ++ "/platforms";
    const lib = qmk ++ "/lib";
    const chibios = lib ++ "/chibios/os";

    const elpekenin_lib = b.addStaticLibrary(.{
        .root_source_file = b.path("src/zig/elpekenin.zig"),
        .name = "elpekenin",
        .target = target,
        .optimize = optimize,
    });

    elpekenin_lib.is_linking_libc = true;

    // needed on native, not on MCU (?)
    elpekenin_lib.bundle_compiler_rt = target.result.os.tag == .linux;

    // ideally, we would only need these two paths
    elpekenin_lib.addSystemIncludePath(.{ .cwd_relative = qmk });
    elpekenin_lib.addSystemIncludePath(.{ .cwd_relative = elpekenin ++ "/include" });

    // ... but QMK's includes are a mess
    elpekenin_lib.addSystemIncludePath(.{ .cwd_relative = chibios ++ "/common/portability/GCC" });
    elpekenin_lib.addSystemIncludePath(.{ .cwd_relative = chibios ++ "/hal/include" });
    elpekenin_lib.addSystemIncludePath(.{ .cwd_relative = chibios ++ "/hal/osal/rt-nil" });
    elpekenin_lib.addSystemIncludePath(.{ .cwd_relative = chibios ++ "/license" });
    elpekenin_lib.addSystemIncludePath(.{ .cwd_relative = chibios ++ "/rt/include" });
    elpekenin_lib.addSystemIncludePath(.{ .cwd_relative = lib ++ "/printf/src/printf" });
    elpekenin_lib.addSystemIncludePath(.{ .cwd_relative = platforms });
    elpekenin_lib.addSystemIncludePath(.{ .cwd_relative = platforms ++ "/chibios" });
    elpekenin_lib.addSystemIncludePath(.{ .cwd_relative = qmk ++ "/quantum" });

    // ... and my 3rd_party libs are as well :P
    elpekenin_lib.addSystemIncludePath(.{ .cwd_relative = elpekenin ++ "/3rd_party/backtrace/include" });

    // hack newlib (c-stdlib) include path too...
    elpekenin_lib.addSystemIncludePath(.{ .cwd_relative = "/usr/include/newlib/" });

    const assets = try getAssets(b);
    elpekenin_lib.root_module.addImport("assets", assets.createModule());

    // generate the .a file
    b.installArtifact(elpekenin_lib);

    // FIXME: broken on zig 0.13 (?)
    // ... and its equivalent .h
    // const header = b.addInstallFile(elpekenin_lib.getEmittedH(), "include/elpekenin/zig.h");
    // b.getInstallStep().dependOn(&header.step);
}
