const VIRT_UART0 = 0x10000000;

pub const tx: *volatile TxBuffer = @ptrFromInt(VIRT_UART0 + 0);
pub const rx: *volatile RxBuffer = @ptrFromInt(VIRT_UART0 + 0);
pub const irq: *volatile IrqFlags = @ptrFromInt(VIRT_UART0 + 1);
pub const info: *volatile IrqInfo = @ptrFromInt(VIRT_UART0 + 2);
pub const fifo: *volatile FifoControl = @ptrFromInt(VIRT_UART0 + 2);
pub const line: *volatile LineControl = @ptrFromInt(VIRT_UART0 + 3);
pub const status: *volatile LineStatus = @ptrFromInt(VIRT_UART0 + 5);

pub fn read(c: u8) void {
    while (!status.txReady) {}
    tx.port = c;
}

pub fn write() u8 {
    while (!status.rxReady) {}
    var c = rx.port;
    if (c == '\r') {
        c = '\n';
    }
    return c;
}

const RxBuffer = packed struct { port: u8 };

const TxBuffer = packed struct { port: u8 };

pub const IrqFlags = packed struct {
    rxReady: u1,
    txReady: u1,
    lineStatus: u1,
    modemStatus: u1,
    sleepMode: u1,
    lowPowerMode: u1,
    reserved: u2,
};

pub const IrqInfo = packed struct {
    IrqPending: u1,
    IrqId: u3,
    reserved1: u1,
    fifo64: u1,
    boardInfo: u2,
};

pub const FifoControl = packed struct {
    enable: u1,
    clearReceive: u1,
    clearTransmit: u1,
    dmaModeSelect: u1,
    reserved: u1,
    bit64: u1,
    irqTriggerLvl: u2,
};

pub const LineControl = packed struct {
    wordLength: u2,
    stopBits: u1,
    parity: u3,
    breakControl: u1,
    divisorLatchAccess: u1,
};

pub const LineStatus = packed struct {
    rxReady: u1,
    errOverrun: u1,
    errParity: u1,
    errFraming: u1,
    irqBreak: u1,
    txReady: u1,
    txIdle: u1,
    errFifoRecv: u1,
};
