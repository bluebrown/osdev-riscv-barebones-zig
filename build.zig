const std = @import("std");

pub fn build(b: *std.Build) !void {
    const k = b.addExecutable(.{
        .name = "kernel.elf",
        .root_source_file = b.path("main.zig"),
        .optimize = b.standardOptimizeOption(.{}),
        .target = b.resolveTargetQuery(.{
            .cpu_arch = .riscv64,
            .os_tag = .freestanding,
            .abi = .none,
            .ofmt = .elf,
        }),
        // [NOTE]
        //  medium code model has been choose, as this allows to adress +-2GB
        //  of memory, relative to the program counter. This is useful when
        //  when adressing peripherals and other units on the memory bus.
        .code_model = .medium,
    });

    k.addAssemblyFile(b.path("entry.S"));
    k.setLinkerScript(b.path("linker.ld"));

    b.installArtifact(k);

    // some useful args to pass via
    // `zig build run -- <args>`:
    // * -s -S ; for debugging
    // * -smp <n> ; set n cores
    const q = b.addSystemCommand(&.{
        "qemu-system-riscv64",
        "-machine",
        "virt",
        "-display",
        "none",
        "-serial",
        "stdio",
        "-bios",
        "none",
        "-kernel",
        "zig-out/bin/kernel.elf",
    });

    q.step.dependOn(b.getInstallStep());

    if (b.args) |args| {
        q.addArgs(args);
    }

    const r = b.step("run", "Run the kernel with qemu virt machine");
    r.dependOn(&q.step);
}
