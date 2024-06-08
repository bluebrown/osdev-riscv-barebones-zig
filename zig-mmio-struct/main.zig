const uart = @import("uart.zig");

pub export fn main() noreturn {
    uart.fifo.enable = 1;
    uart.irq.txReady = 1;

    if (uart.info.boardInfo == 3)
        println("fifo enabled");

    if (uart.info.irqPending == 0)
        println("uart irq pending");

    if (uart.info.irqId == 1)
        println("uart irq = thr empty");

    while (true)
        asm volatile ("wfi");
}

fn println(msg: []const u8) void {
    for (msg) |c|
        uart.write(c);
    uart.write('\n');
}
