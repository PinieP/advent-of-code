const std = @import("std");

pub fn makeDay(b: *std.Build, day: u8) *std.Build.Step.Compile {
    var buf0: [32]u8 = undefined;
    var buf1: [32]u8 = undefined;
    const path = std.fmt.bufPrint(&buf0, "src/{d}.zig", .{day}) catch unreachable;
    const name = std.fmt.bufPrint(&buf1, "{d}", .{day}) catch unreachable;

    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});
    return b.addExecutable(.{
        .name = name,
        .root_module = b.createModule(.{
            .root_source_file = b.path(path),
            // Target and optimization levels must be explicitly wired in when
            // defining an executable or library (in the root module), and you
            // can also hardcode a specific target for an executable or library
            // definition if desireable (e.g. firmware for embedded devices).
            .target = target,
            .optimize = optimize,
        }),
    });
}

pub fn build(b: *std.Build) void {
    const day = b.option(u8, "day", "Which day to run") orelse 5;
    const llvm = b.option(bool, "llvm", "Wheter to use llvm backend") orelse false;

    var exe = makeDay(b, day);

    var tests = b.addTest(.{
        .root_module = exe.root_module,
    });
    if (llvm) {
        tests.use_llvm = true;
        exe.use_llvm = true;
    }
    b.installArtifact(exe);
    b.installArtifact(tests);

    const run_step = b.step("run", "Run the app");
    const run_cmd = b.addRunArtifact(exe);
    run_step.dependOn(&run_cmd.step);

    // By making the run step depend on the default step, it will be run from the
    // installation directory rather than directly from within the cache directory.
    run_cmd.step.dependOn(b.getInstallStep());

    // Creates an executable that will run `test` blocks from the executable's
    // root module. Note that test executables only test one module at a time,
    // hence why we have to create two separate ones.

    // A run step that will run the second test executable.
    const run_tests = b.addRunArtifact(tests);

    // A top level step for running all tests. dependOn can be called multiple
    // times and since the two run steps do not depend on one another, this will
    // make the two of them run in parallel.
    const test_step = b.step("test", "Run tests");
    test_step.dependOn(&run_tests.step);
}
