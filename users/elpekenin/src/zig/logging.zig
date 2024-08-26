//! Bindings into custom logging functionality in C

pub const Feature = enum(c_int) {
    QP = 2,
};

pub const Level = enum(c_int) {
    INFO = 2,
    WARN = 3,
};

pub extern fn logging(Feature, Level, [*:0]const u8, ...) c_int;
