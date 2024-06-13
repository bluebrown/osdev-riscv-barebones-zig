LLVM_VERSION = 16
OBJCOPY = llvm-objcopy-$(LLVM_VERSION)
OBJDUMP = llvm-objdump-$(LLVM_VERSION)
SIZE = llvm-size-$(LLVM_VERSION)
QEMU = qemu-system-riscv32
CC = zig cc

TARGET_ARCH += --target=riscv32-freestanding-none
TARGET_ARCH += -march=generic_rv32 -mabi=ilp32
TARGET_ARCH += -mcmodel=medany
TARGET_MACH += $(TARGET_ARCH) # for assembly

CFLAGS += -Wall -Werror -Wcast-align -MD -MP
CFLAGS += -O2 -ffreestanding -fno-pic
LDFLAGS += -nostdlib -Wl,-T,linker.ld
ASFLAGS += $(CFLAGS) -Wa,-L

ifdef debug
CFLAGS += -g
QFLAGS += -s -S
endif

cpu = 2
mem = 128M
src = $(wildcard *.c)
obj = $(src:.c=.o)

qemu: crt0 crt0.lst # run in emulator
	$(SIZE) -A -x $<
	$(QEMU) -machine virt \
		-display none -serial stdio \
		-smp $(cpu) -m $(mem) \
		-bios none -kernel $< $(QFLAGS)

crt0: crt0.o trapvec.o $(obj) # use implicit rules
# %: %.o -> %.o: %.[cS]

clean: # remove generated files
	$(RM) *.o *.d *.lst *.bin *.elf \
		*.srec *.out crt0

%.bin: % # strip elf to binary
	$(OBJCOPY) $< -O binary $@

%.lst: % # debug dissasembly
	$(OBJDUMP) -Mnumeric,no-aliases -S -dr $< > $<.lst

# track changes in header files
# requires -MD -MP in CFLAGS
-include $(obj:.o=.d)
