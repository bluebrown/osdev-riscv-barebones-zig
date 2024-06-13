ARCH = riscv32-unknown-elf
OBJCOPY = $(ARCH)-objcopy
OBJDUMP = $(ARCH)-objdump
OBJDUMPFLAGS += -Mnumeric,no-aliases -S
SIZE = $(ARCH)-size
AR = $(ARCH)-ar
RANLIB = $(ARCH)-ranlib

CC = zig cc --target=riscv32-freestanding-none
TARGET_ARCH += -march=generic_rv32 -mabi=ilp32 -mcmodel=medany
CFLAGS += -g -Wall -Werror -Wcast-align -MD -MP -O2 -ffreestanding -fno-pic
ASFLAGS += $(CFLAGS) $(TARGET_ARCH) -Wa,-L
LDFLAGS += -nostdlib -Wl,-T,linker.ld

# MARCH=rv32ima # for gcc
# ASFLAGS+=-Wa,-alh=$@.lst,-L # for gcc

#CFLAGS+=-fomit-frame-pointer
#LDFLAGS+=-Wl,--no-relax

src = $(wildcard *.c)
obj = $(src:.c=.o)
cpu = 2
mem = 128

qemu: prog.elf prog.elf.lst
	qemu-system-riscv32 -machine virt \
		-display none -serial stdio \
		-smp $(cpu) -m $(mem) \
		-bios none -kernel $< $(QFLAGS)

prog.elf: crt0.o $(obj)
	$(LINK.c) -o $@ $^ $(LDLIBS)
	$(SIZE) -A -x $@

clean:
	rm -f *.o *.d *.lst *.bin *.elf *.srec

%.bin : %
	$(OBJCOPY) $< -O binary $@

%.lst : %
	$(OBJDUMP) $(OBJDUMPFLAGS) -dr $< > $<.lst

% : %.o
	$(LINK.c) $(TARGET_ARCH) $(LDFLAGS) -o $@ $^ $(LDLIBS)
	$(SIZE) -x -A $@

%.s: %.c
	$(COMPILE.c) -S -o $@ $<

%.o: %.c
	$(COMPILE.c) -o $@ $<

%.o: %.S
	$(COMPILE.S) -o $@ $<

%.srec: %
	$(OBJCOPY) $< -O srec $@

-include $(obj:.o=.d)
