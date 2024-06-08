const std = @import("std");

pub fn build(b: *std.Build) !void {
    const k = b.addExecutable(.{
        .name = "uart.elf",
        .root_source_file = b.path("main.zig"),
        .optimize = b.standardOptimizeOption(.{}),
        .target = b.resolveTargetQuery(.{
            .cpu_arch = .riscv32,
            .os_tag = .freestanding,
            .abi = .none,
            .ofmt = .elf,
        }),
        .code_model = .medium,
    });

    k.addAssemblyFile(b.path("crt0.S"));
    k.setLinkerScript(b.path("linker.ld"));

    b.installArtifact(k);

    const q = b.addSystemCommand(&.{
        "qemu-system-riscv32",
        "-machine",
        "virt",
        "-display",
        "none",
        "-serial",
        "stdio",
        "-bios",
        "none",
        "-kernel",
        "zig-out/bin/uart.elf",
    });

    q.step.dependOn(b.getInstallStep());

    if (b.args) |args| {
        q.addArgs(args);
    }

    const r = b.step("run", "Run the kernel with qemu virt machine");
    r.dependOn(&q.step);
}
