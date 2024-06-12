out = prog.elf
gdb =
cpu = 2
mem = 128

CC = zig cc --target=riscv32-freestanding-none \
	 -march=generic_rv32 -mabi=ilp32 -mcmodel=medany

CFLAGS += -MD -MP -Wall -Wextra -O2

ifdef gdb
CFLAGS += -g
QFLAGS = -s -S
endif

qemu: $(out)
	qemu-system-riscv32 -machine virt \
		-display none -serial stdio \
		-smp $(cpu) -m $(mem) \
		-bios none -kernel $< $(QFLAGS)

# FIXME: incremental build is broken compiling all
# in a single pass works incremental build suceeds
# but the program is broken
src = $(wildcard *.c)
obj = $(src:.c=.o)

# change src to obj for incremental build
$(out): *.ld *.S $(src)
	$(CC) $(CFLAGS) -o $@ -T $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f *.i *.o *.d *.bc \
		a.out *.bin *.pdf *.html *.elf

-include $(obj:.o=.d)
