const VIRT_UART0 = 0x10000000;

pub const tx: *volatile TxBuffer = @ptrFromInt(VIRT_UART0 + 0);
pub const rx: *volatile RxBuffer = @ptrFromInt(VIRT_UART0 + 0);
pub const irq: *volatile IrqFlags = @ptrFromInt(VIRT_UART0 + 1);
pub const info: *volatile IrqInfo = @ptrFromInt(VIRT_UART0 + 2);
pub const fifo: *volatile FifoControl = @ptrFromInt(VIRT_UART0 + 2);
pub const line: *volatile LineControl = @ptrFromInt(VIRT_UART0 + 3);
pub const status: *volatile LineStatus = @ptrFromInt(VIRT_UART0 + 5);

pub fn write(c: u8) void {
    while (status.txReady == 0) {}
    if (c == '\n')
        tx.port = '\r';
    tx.port = c;
}

pub fn read() u8 {
    while (status.rxReady == 0) {}
    var c = rx.port;
    if (c == '\r')
        c = '\n';
    return c;
}

const RxBuffer = packed struct(u8) { port: u8 };

const TxBuffer = packed struct(u8) { port: u8 };

pub const IrqFlags = packed struct(u8) {
    rxReady: u1,
    txReady: u1,
    lineStatus: u1,
    modemStatus: u1,
    sleepMode: u1,
    lowPowerMode: u1,
    reserved: u2,
};

pub const IrqInfo = packed struct(u8) {
    irqPending: u1,
    irqId: u3,
    reserved1: u1,
    fifo64: u1,
    boardInfo: u2,
};

pub const FifoControl = packed struct(u8) {
    enable: u1,
    clearReceive: u1,
    clearTransmit: u1,
    dmaModeSelect: u1,
    reserved: u1,
    bit64: u1,
    irqTriggerLvl: u2,
};

pub const LineControl = packed struct(u8) {
    wordLength: u2,
    stopBits: u1,
    parity: u3,
    breakControl: u1,
    divisorLatchAccess: u1,
};

pub const LineStatus = packed struct(u8) {
    rxReady: u1,
    errOverrun: u1,
    errParity: u1,
    errFraming: u1,
    irqBreak: u1,
    txReady: u1,
    txIdle: u1,
    errFifoRecv: u1,
};
