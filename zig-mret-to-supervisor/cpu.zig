pub inline fn hang() noreturn {
    while (true) {
        asm volatile ("wfi");
    }
}

pub const csr = struct {
    pub inline fn read(r: []const u8) usize {
        return asm volatile ("csrr a0, " ++ r
            : [ret] "={a0}" (-> usize),
        );
    }

    pub inline fn write(r: []const u8, x: usize) void {
        asm volatile ("csrw " ++ r ++ ", a0"
            :
            : [x] "a0" (x),
        );
    }

    pub inline fn set(r: []const u8, x: usize) void {
        asm volatile ("csrs " ++ r ++ ", a0"
            :
            : [x] "a0" (x),
        );
    }

    pub inline fn clear(r: []const u8, x: usize) void {
        asm volatile ("csrc " ++ r ++ ", a0"
            :
            : [x] "a0" (x),
        );
    }
};
