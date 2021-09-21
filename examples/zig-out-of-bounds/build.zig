const std = @import("std");
const Target = std.Target;
const Zig = std.zig;
const Builder = std.build.Builder;

pub fn build(b: *Builder) void {
    const target = Zig.CrossTarget{
        .cpu_arch = Target.Cpu.Arch.riscv32,
        .os_tag = Target.Os.Tag.freestanding,
        .abi = Target.Abi.none,
        .cpu_features_sub = std.Target.riscv.cpu.baseline_rv32.features,
    };

    const mode = b.standardReleaseOptions();

    const exe = b.addExecutable("main", "main.zig");
    exe.setTarget(target);
    exe.setBuildMode(mode);

    exe.addCSourceFile("start.S", &[_][]const u8{});

    b.default_step.dependOn(&exe.step);
    b.installArtifact(exe);
}
