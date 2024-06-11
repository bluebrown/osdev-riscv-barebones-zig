CC = zig cc \
	 --target=riscv32-freestanding-none \
	 -march=generic_rv32 -mabi=ilp32

outfile = prog.elf
codemodel = medany
numcpu = 2
maxmem = 128
debug = 0

ifeq ($(debug), 1)
	QFLAGS = -s -S
endif

run: $(outfile)
	qemu-system-riscv32 -machine virt \
		-display none -serial stdio \
		-smp $(numcpu) -m $(maxmem) \
		-bios none -kernel $< $(QFLAGS)

$(outfile): *.ld *.S *.c
	$(CC) -mcmodel=$(codemodel) $(CFLAGS) \
		-o $@ -T $^

clean:
	rm -f *.i *.o *.bc \
		a.out *.bin *.pdf *.html *.elf
