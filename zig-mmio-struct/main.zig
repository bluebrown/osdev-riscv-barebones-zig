const uart = @import("uart.zig");

pub export fn main() noreturn {
    // configure uart
    uart.line.wordLength = 3; // 8 bits
    uart.line.stopBits = 0; // 1 stop bit
    uart.line.parity = 0; // no parity
    uart.line.divisorLatchAccess = 1; // enable divisor latch access
    const divisor = 1; // 115200 baud
    uart.tx.port = divisor & 0xff;
    uart.rx.port = (divisor >> 8) & 0xff;
    uart.line.divisorLatchAccess = 0; // disable divisor latch access
    uart.fifo.enable = 1; // enable FIFO
    uart.fifo.irqTriggerLvl = 0; // 1 byte trigger level
    uart.irq.txReady = 1; // enable thr empty interrupt

    // enable uart
    if (uart.info.IrqPending == 0)
        println("uart irq pending");

    if (uart.info.IrqId == 1)
        println("uart irq = thr empty");

    while (true)
        asm volatile ("wfi");
}

fn println(msg: []const u8) void {
    for (msg) |c| {
        uart.tx.port = c;
    }
    uart.tx.port = '\n';
    uart.tx.port = '\r';
}
