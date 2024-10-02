//! Bindings into custom logging functionality in C

const c = @cImport(@cInclude("elpekenin/logging.h"));

pub const Feature = enum(c.feature_t) {
    UNKNOWN = c.UNKNOWN,
    LOGGER = c.LOGGER,
    QP = c.QP,
    SCROLL = c.SCROLL,
    SIPO = c.SIPO,
    SPLIT = c.SPLIT,
    SPI = c.SPI,
    TOUCH = c.TOUCH,
    MAP = c.MAP,
    ALLOC = c.ALLOC,
};

pub const Level = enum(c.log_level_t) {
    NONE = c.LOG_NONE,
    DEBUG = c.LOG_DEBUG,
    INFO = c.LOG_INFO,
    WARN = c.LOG_WARN,
    ERROR = c.LOG_ERROR,
};

/// Tiny wrapper on top of C's logging code, to execute it seamlessly from zig
pub fn logging(feature: Feature, level: Level, msg: [*:0]const u8, args: anytype) void {
    _ = @call(.auto, c.logging, .{ @intFromEnum(feature), @intFromEnum(level), msg } ++ args);
}

pub fn debug(feature: Feature, msg: [*:0]const u8, args: anytype) void {
    return logging(feature, .DEBUG, msg, args);
}

pub fn info(feature: Feature, msg: [*:0]const u8, args: anytype) void {
    return logging(feature, .INFO, msg, args);
}

pub fn warn(feature: Feature, msg: [*:0]const u8, args: anytype) void {
    return logging(feature, .WARN, msg, args);
}

pub fn err(feature: Feature, msg: [*:0]const u8, args: anytype) void {
    return logging(feature, .ERROR, msg, args);
}
