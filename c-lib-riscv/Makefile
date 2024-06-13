LLVM_VERSION = 16
AS = llvm-as-$(LLVM_VERSION)
AR = llvm-ar-$(LLVM_VERSION)
CC = clang-$(LLVM_VERSION)
LD = ld.lld-$(LLVM_VERSION)
NM = llvm-nm-$(LLVM_VERSION)
OBJCOPY = llvm-objcopy-$(LLVM_VERSION)
OBJDUMP = llvm-objdump-$(LLVM_VERSION)
RANLIB = llvm-ranlib-$(LLVM_VERSION)
READELF = llvm-readelf-$(LLVM_VERSION)
SIZE = llvm-size-$(LLVM_VERSION)
STRIP = llvm-strip-$(LLVM_VERSION)

QEMU = qemu-system-riscv32

TARGET_ARCH += --target=riscv32-freestanding-none
TARGET_ARCH += -march=generic_rv32 -mabi=ilp32
TARGET_ARCH += -mcmodel=medany
TARGET_MACH += $(TARGET_ARCH)

CC = zig cc
CFLAGS += -Wall -Werror -Wcast-align -MD -MP
CFLAGS += -O2 -ffreestanding -fno-pic
LDFLAGS += -nostdlib -Wl,-T,linker.ld
ASFLAGS += $(CFLAGS) -Wa,-L
OBJDUMPFLAGS += -Mnumeric,no-aliases -S

ifdef debug
CFLAGS += -g
QFLAGS += -s -S
endif

cpu = 2
mem = 128

src = $(wildcard *.c)
obj = $(src:.c=.o)

qemu: prog.elf prog.elf.lst
	$(QEMU) -machine virt \
		-display none -serial stdio \
		-smp $(cpu) -m $(mem) \
		-bios none -kernel $< $(QFLAGS)

prog.elf: crt0.o $(obj)
	$(LINK.c) -o $@ $^ $(LDLIBS)
	$(SIZE) -A -x $@

clean:
	$(RM) *.o *.d *.lst *.bin *.elf *.srec

%.bin: %.elf
	$(OBJCOPY) $< -O binary $(@:.elf=.bin)

%.lst: %
	$(OBJDUMP) $(OBJDUMPFLAGS) -dr $< > $<.lst

-include $(obj:.o=.d)
