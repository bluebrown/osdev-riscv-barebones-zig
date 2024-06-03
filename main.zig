const tty = @import("tty.zig");
const cpu = @import("cpu.zig");
const csr = cpu.csr;

const ALL_ONES = 0xfffffffffffffff;
const MSTATUS_MMP_MASK = 3 << 11; // privilege bits
const MSTATUS_MMP_S = 1 << 11; // supervisor

export fn main() noreturn {
    // at this point only the 3 pointers, global (gp), stack (sp) and thread
    // pointer (tp), have been set up. a little bit more than the bare minium
    // to call main.
    tty.println("machine mode setup");

    // bit 11-12 in mstatus are for the privilege mode. from 0 user, 1
    // supervisor, 3 machine. mret will use this to set the privilege mode.
    // clear the current status and set it to supervisor.
    csr.clear("mstatus", MSTATUS_MMP_MASK);
    csr.set("mstatus", MSTATUS_MMP_S);

    // the (m)achine (e)xception (p)rogram (c)ounter hold the return adress
    // used by mret.
    csr.write("mepc", @intFromPtr(&kernel));

    // diable supervirsor address translation and protection.
    // The actual kernel should set this up.
    csr.write("satp", 0);

    // configure the lowest PMP registers to allow the supervirsor to access
    // all memory. PMP is used to check is a given mode is allowed to access a
    // given memory region.. cfg is 8bit and addr is xlen bit. The lowest
    // register have priority.
    csr.write("pmpaddr0", ALL_ONES);
    csr.write("pmpcfg0", ALL_ONES);

    // TODO:configure interupt/exception handling

    // mret will return to the adress in MEPC with the mode in MPP
    asm volatile ("mret");
    unreachable;
}

export fn kernel() noreturn {
    tty.println("supervisor mode setup");
    cpu.hang();
}
