const cpu = @import("cpu.zig");
const tty = @import("tty.zig");

export fn main() noreturn {
    tty.println("Hello, World!");
    cpu.hang();
}
