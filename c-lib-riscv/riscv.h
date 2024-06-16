#ifndef RISCV_H
#define RISCV_H

#include <stddef.h>
#include <stdint.h>

// common assembly instructions

#define wfi asm volatile("wfi")

#define mret                                                                   \
  asm volatile("mret");                                                        \
  __builtin_unreachable()

// csr instructions

#define csrr(rd, csr) asm volatile("csrr %0, " #csr : "=r"(rd))
#define csrw(csr, rs1) asm volatile("csrw " #csr ", %0" ::"r"(rs1))
#define csrs(csr, rs1) asm volatile("csrs " #csr ", %0" ::"r"(rs1))
#define csrc(csr, rs1) asm volatile("csrc " #csr ", %0" ::"r"(rs1))

// control and status registers

// the reigsters are layed out such that higher privleges are supersets of lower
// privleges, in terms of layout. For a given register, fields for a given
// privilege level are WIRI (write ignore, read ignore), when accessed from a
// lower privlege level. Thus only the highest privilige level is defined here
// as all other prvilige level can use the same data structured and constants.
// These definition shall be called x<register> where x means it can be used for
// any privilige level. The definitions are not comprehensive and only include
// the fields that are used in this project.

// .CSR field behavior
// ****
// * Reserved Writes Ignored, Reads Ignore Values (WIRI)
// * Reserved Writes Preserve Values, Reads Ignore Values (WPRI)
// * Write/Read Only Legal Values (WLRL)
// * Write Any Values, Reads Legal Values (WARL)
// ****

// xstatus register

// 31  30 23  22 21  20  19  18   17
// SD   WPRI TSR TW TVM MXR SUM MPRV
//  1      8   1  1   1   1   1    1
//
// 16   15 14   13 12    11 10 9   8    7    6    5    4   3    2   1   0
// XS[1:0] FS[1:0] MPP[1:0] WPRI SPP MPIE WPRI SPIE UPIE MIE WPRI SIE UIE
//       2       2        2    2   1    1    1    1    1   1    1   1   1
//
// Figure 3.6: Machine-mode status register (mstatus) for RV32.

#define XSTATUS_MPP_MASK (3 << 11)

#define XSTATUS_SIE (1 << 1)
#define XSTATUS_MIE (1 << 3)
#define XSTATUS_SPIE (1 << 5)
#define XSTATUS_MPIE (1 << 7)
#define XSTATUS_SPP (1 << 8)
#define XSTATUS_MPP_M (3 << 11)
#define XSTATUS_MPP_S (1 << 11)

// xie register

// XLEN-1 12   11   10    9    8    7    6    5    4    3    2    1    0
//      WPRI MEIE WPRI SEIE UEIE MTIE WPRI STIE UTIE MSIE WPRI SSIE USIE
//   XLEN-12    1    1    1    1    1    1    1    1    1    1    1    1
//
// Figure 3.12: Machine interrupt-enable register (mie).

#define XIE_SSIE (1 << 1)
#define XIE_MSIE (1 << 3)
#define XIE_STIE (1 << 5)
#define XIE_MTIE (1 << 7)
#define XIE_SEIE (1 << 9)
#define XIE_MEIE (1 << 11)

// xcause register

// Interrupt Exception   Code Description
// ----------------------------------------------------
//         1         0   User software interrupt
//         1         1   Supervisor software interrupt
//         1         2   Reserved
//         1         3   Machine software interrupt
//         1         4   User timer interrupt
//         1         5   Supervisor timer interrupt
//         1         6   Reserved
//         1         7   Machine timer interrupt
//         1         8   User external interrupt
//         1         9   Supervisor external interrupt
//         1        10   Reserved
//         1        11   Machine external interrupt
//         1       ≥12   Reserved
//         0         0   Instruction address misaligned
//         0         1   Instruction access fault
//         0         2   Illegal instruction
//         0         3   Breakpoint
//         0         4   Load address misaligned
//         0         5   Load access fault
//         0         6   Store/AMO address misaligned
//         0         7   Store/AMO access fault
//         0         8   Environment call from U-mode
//         0         9   Environment call from S-mode
//         0        10   Reserved
//         0        11   Environment call from M-mode
//         0        12   Instruction page fault
//         0        13   Load page fault
//         0        14   Reserved
//         0        15   Store/AMO page fault
//         0       ≥16   Reserved
//
// Table 3.6: Machine cause register (mcause) values after trap.
struct __attribute__((packed)) XCause {
  uint32_t code : 31;
  uint32_t is_interrupt : 1;
};

static inline struct XCause MCause() {
  struct XCause cause;
  asm volatile("csrr %0, mcause" : "=r"(cause));
  return cause;
}

static inline struct XCause SCause() {
  struct XCause cause;
  asm volatile("csrr %0, scause" : "=r"(cause));
  return cause;
}

#define IRQ_USER_SOFTWARE_INTERRUPT (0)
#define IRQ_SUPERVISOR_SOFTWARE_INTERRUPT (1)
#define IRQ_MACHINE_SOFTWARE_INTERRUPT (3)
#define IRQ_USER_TIMER_INTERRUPT (4)
#define IRQ_SUPERVISOR_TIMER_INTERRUPT (5)
#define IRQ_MACHINE_TIMER_INTERRUPT (7)
#define IRQ_USER_EXTERNAL_INTERRUPT (8)
#define IRQ_SUPERVISOR_EXTERNAL_INTERRUPT (9)
#define IRQ_MACHINE_EXTERNAL_INTERRUPT (11)

static const char *irq_names[] = {
    [IRQ_USER_SOFTWARE_INTERRUPT] = "User software interrupt",
    [IRQ_SUPERVISOR_SOFTWARE_INTERRUPT] = "Supervisor software interrupt",
    [IRQ_MACHINE_SOFTWARE_INTERRUPT] = "Machine software interrupt",
    [IRQ_USER_TIMER_INTERRUPT] = "User timer interrupt",
    [IRQ_SUPERVISOR_TIMER_INTERRUPT] = "Supervisor timer interrupt",
    [IRQ_MACHINE_TIMER_INTERRUPT] = "Machine timer interrupt",
    [IRQ_USER_EXTERNAL_INTERRUPT] = "User external interrupt",
    [IRQ_SUPERVISOR_EXTERNAL_INTERRUPT] = "Supervisor external interrupt",
    [IRQ_MACHINE_EXTERNAL_INTERRUPT] = "Machine external interrupt",
};

#define EXC_INSTRUCTION_ACCESS_FAULT (1)
#define EXC_ILLEGAL_INSTRUCTION (2)
#define EXC_BREAKPOINT (3)
#define EXC_LOAD_ACCESS_FAULT (5)
#define EXC_STORE_AMO_ACCESS_FAULT (7)
#define EXC_INSTRUCTION_PAGE_FAULT (12)
#define EXC_LOAD_PAGE_FAULT (13)
#define EXC_STORE_AMO_PAGE_FAULT (15)

static const char *exception_names[] = {
    [EXC_INSTRUCTION_ACCESS_FAULT] = "Instruction access fault",
    [EXC_ILLEGAL_INSTRUCTION] = "Illegal instruction",
    [EXC_BREAKPOINT] = "Breakpoint",
    [EXC_LOAD_ACCESS_FAULT] = "Load access fault",
    [EXC_STORE_AMO_ACCESS_FAULT] = "Store/AMO access fault",
    [EXC_INSTRUCTION_PAGE_FAULT] = "Instruction page fault",
    [EXC_LOAD_PAGE_FAULT] = "Load page fault",
    [EXC_STORE_AMO_PAGE_FAULT] = "Store/AMO page fault",
};

// xip

// XLEN-1 12   11   10    9    8    7    6    5    4    3    2   1     0
//      WIRI MEIP WIRI SEIP UEIP MTIP WIRI STIP UTIP MSIP WIRI SSIP USIP
//   XLEN-12    1    1    1    1    1    1    1    1    1    1    1    1
//
// Figure 3.11: Machine interrupt-pending register (mip).
struct __attribute__((packed)) XIP {
  uint32_t usip : 1;
  uint32_t ssip : 1;
  uint32_t wsip : 1;
  uint32_t msip : 1;
  uint32_t utip : 1;
  uint32_t stip : 1;
  uint32_t wtip : 1;
  uint32_t mtip : 1;
  uint32_t ueip : 1;
  uint32_t seip : 1;
  uint32_t weip : 1;
  uint32_t meip : 1;
  uint32_t wiri : 20;
};

static inline struct XIP MIP() {
  struct XIP ip;
  asm volatile("csrr %0, mip" : "=r"(ip));
  return ip;
}

static inline struct XIP SIP() {
  struct XIP ip;
  asm volatile("csrr %0, sip" : "=r"(ip));
  return ip;
}

// satp

// XLEN-bit read/write register

//          31 30       22 21       0
// MODE (WARL) ASID (WARL) PPN (WARL)
//           1          9          22
//
// Figure 4.11: RV32 Supervisor address translation and protection register satp

// * PPN (Physical Page Number): Physical page number of the root page table.
// * ASID (Address Space Identifier): Address-space identifier.
// * MODE: Determines the current address-translation scheme.

#define SATP_PPN_MASK (0x3fffff)
#define SATP_ASID_MASK (0x3ff << 22)
#define SATP_MODE_MASK (0xf << 31)

// [NOTE]
// When mode is `Bare`, the remaining fields have no effect.

// .MODE field
// RV32
// ----
// Value  Name  Description
//     0  Bare  No translation or protection.
//     1  Sv32  Page-based 32-bit virtual addressing.

#define SATP_MODE_SV32 (1 << 31)

// RV64
// ----
// Value  Name  Description
//     0  Bare  No translation or protection.
//   1–7   —    Reserved
//     8  Sv39  Page-based 39-bit virtual addressing.
//     9  Sv48  Page-based 48-bit virtual addressing.
//    10  Sv57  Reserved for page-based 57-bit virtual addressing.
//    11  Sv64  Reserved for page-based 64-bit virtual addressing.
// 12–15   —    Reserved
//
// Table 4.3: Encoding of satp MODE field

// address translation

// The 20-bit VPN is translated into a 22-bit physical page number (PPN), while
// the 12- bit page offset is untranslated. The resulting supervisor-level
// physical addresses are then checked using any physical memory protection
// structures (Sections 3.6), before being directly converted to machine-level
// physical addresses.

// 31  22 21  12 11        0
// VPN[1] VPN[0] page-offset
//     10     10          12
//
// Figure 4.13: Sv32 virtual address
struct __attribute__((packed)) Sv32VirtualAddress {
  uint32_t page_offset : 12;
  uint32_t vpn0 : 10;
  uint32_t vpn1 : 10;
};

// 3   22 21  12 11        0
// PPN[1] PPN[0] page-offset
//     12     10          12
//
// Figure 4.14: Sv32 physical address.
struct __attribute__((packed)) Sv32PhysicalAddress {
  uint32_t page_offset : 12;
  uint32_t ppn0 : 10;
  uint32_t ppn1 : 10;
};

//  31 20 19  10 9 8 7 6 5 4 3 2 1 0
// PPN[1] PPN[0] RSW D A G U X W R V
//    12     10    2 1 1 1 1 1 1 1 1
//
// Figure 4.15: Sv32 page table entr
//
// X W R Meaning
// 0 0 0 Pointer to next level of page table.
// 0 0 1 Read-only page.
// 0 1 0 Reserved for future use.
// 0 1 1 Read-write page.
// 1 0 0 Execute-only page.
// 1 0 1 Read-execute page.
// 1 1 0 Reserved for future use.
// 1 1 1 Read-write-execute page
//
// Table 4.4: Encoding of PTE R/W/X fields.
//
// Sv32 page tables consist of 210 page-table entries (PTEs), each of four
// bytes. A page table is exactly the size of a page and must always be aligned
// to a page boundary. The physical page number of the root page table is stored
// in the satp register.
struct __attribute__((packed)) Sv32PageTableEntry {
  uint32_t v : 1;
  uint32_t r : 1;
  uint32_t w : 1;
  uint32_t x : 1;
  uint32_t u : 1;
  uint32_t g : 1;
  uint32_t a : 1;
  uint32_t d : 1;
  uint32_t rsw : 2;
  uint32_t ppn0 : 10;
  uint32_t ppn1 : 10;
};


//pmp

//        7  6  5  4      3         2         1         0
// L (WARL)  WIRI  A (WARL)  X (WARL)  W (WARL)  R (WARL)
//        1     2         2         1         1         1
//
// Figure 3.27: PMP configuration register format
struct __attribute__((packed)) PmpConfig {
  // read
  // deny, if clear
  uint8_t r : 1;
  // write
  // deny, if clearn
  uint8_t w : 1;
  // execute
  // deny, if clear
  uint8_t x : 1;
  // address matching mode
  // entry is disabled, if clear
  enum __attribute__((packed)) {
    PMP_CONFIG_A_OFF = 0,
    PMP_CONFIG_A_TOR = 1,
    PMP_CONFIG_A_NA4 = 2,
    PMP_CONFIG_A_NAPOT = 3,
  } a : 2;
  // reserved
  uint8_t WIRI: 2;
  // locking
  // entry is locked, if set
  uint8_t l : 1;
};

// read write execute
#define PMPCFG_R (1 << 0)
#define PMPCFG_W (1 << 1)
#define PMPCFG_X (1 << 2)
// address matching mode
// entry is disabled, if clear
#define PMPCFG_A_MASK (3 << 3)
// locking
// entry is locked, if set
#define PMPCFG_L (1 << 7)

// A Name Description
// 0 OFF Null region (disabled)
// 1 TOR Top of range
// 2 NA4 Naturally aligned four-byte region
// 3 NAPOT Naturally aligned power-of-two region, ≥8 bytes
//
// Table 3.8: Encoding of A field in PMP configuration registersr
#define PMPCFG_A_OFF (0 << 3)
#define PMPCFG_A_TOR (1 << 3)
#define PMPCFG_A_NA4 (2 << 3)
#define PMPCFG_A_NAPOT (3 << 3)

// MMIO

// plic

// these are defined in the ISA specification
#define PLIC_NUMSOURCE (1024)
#define PLIC_ALIGNMENT  (32)
#define PLIC_WORDSIZE (PLIC_ALIGNMENT / 8)

// a WARL registers uses a full word for each value so the max number of
// possible sources must be multiplied by the word length
#define PLIC_WARL_STRIDE (PLIC_NUMSOURCE * PLIC_WORDSIZE) // 4KB(0x1000)

// as pending/interupt bits are indvidual bits, the max number of sources must
// be divided by the alignemnt to get the number of words needed. As each word
// is n bytes the result must be multiplied by the word size to get the number
// of bytes needed
#define PLIC_BITS_STRIDE (PLIC_NUMSOURCE / PLIC_ALIGNMENT * PLIC_WORDSIZE) // 128B(0x80)

static inline size_t plicContext(size_t hart, size_t mode) {
  return (hart << 1) | mode;
}

// plic array access the plic as if it were a word size aligned array.
// Therefore, the provided source is multiplied by the word size to get the
// the offset from the base address
static inline size_t plicArray(size_t base, size_t offset, size_t src) {
  return base + offset + src * PLIC_WORDSIZE;
}

// the warl access only gives the block start based on the context. This is
// because PLIC only uses the block start for each context to store one of
// treshold, claim or complete registers
static inline size_t plicWarl(size_t base, size_t offset, size_t context) {
  return base + offset + context * PLIC_WARL_STRIDE;
}

// deviding the source by the alignment gives the word offset, the remainder
// gives the bit offset in the word and the base is added to get the address
// of the bit:
//
//      id = 31 -> word = 31 / 32 = 0
//      id = 32 -> word = 32 / 32 = 1
//
// Likewise, the bit, for a given source, within that word, must be determined
// by the remainder of the division:
//
//      id = 31 -> bit = 31 % 32 = 31
//      id = 32 -> bit = 32 % 32 = 0
//
static inline size_t plicBits(size_t base, size_t offset, size_t context, size_t src) {
  return base + offset + context * PLIC_BITS_STRIDE + src / PLIC_ALIGNMENT;
}

#ifdef BOARD_QEMU_RISCV_VIRT
#define PLIC_BASE (0x0c000000)
#endif

// stride here is fixed as showed on the ISA spec. above calculation produces
// the same result but is less clear to the reader. Therefore plain values are
// kept here.

// 1024 * 4 = 4096(0x1000) bytes
#define PLIC_PRIORITY_OFFSET 0x0000

// 1024 / 8 = 128(0x80) bytes
#define PLIC_PENDING_OFFSET 0x1000
#define PLIC_PENDING_STRIDE 0x80

// 1024 / 8 = 128(0x80) bytes
#define PLIC_ENABLE_OFFSET 0x2000
#define PLIC_ENABLE_STRIDE 0x80

// 4096 * 15872 = 65011712(0x3e00000) bytes
#define PLIC_THRESHOLD_OFFSET 0x200000
#define PLIC_THRESHOLD_STRIDE 0x1000

// 4096 * 15872 = 65011712(0x3e00000) bytes
#define PLIC_CLAIM_OFFSET 0x200004
#define PLIC_CLAIM_STRIDE 0x1000

// 4096 * 15872 = 65011712(0x3e00000) bytes
#define PLIC_COMPLETE_OFFSET 0x200004
#define PLIC_COMPLETE_STRIDE 0x1000

#endif
