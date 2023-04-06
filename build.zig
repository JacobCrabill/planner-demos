const std = @import("std");
const libnoise = @import("3rdparty/libnoise/build.zig");

const Builder = std.build.Builder;
const LibExeObjStep = std.build.LibExeObjStep;

const NoCArgs: *const [0][]const u8 = &[_][]const u8{};
const CxxArgs = &[_][]const u8{"-std=c++17"};

pub fn build(b: *std.build.Builder) !void {
    // Standard release options allow the person running `zig build` to select
    // between Debug, ReleaseSafe, ReleaseFast, and ReleaseSmall.
    const mode = b.standardReleaseOptions();

    const exe = b.addExecutable("planner-demo", null);
    exe.setBuildMode(mode);

    // Add this app's sources
    try addSourceFilesFrom(b, exe, "src/");

    // Add dependencies
    try addSourceFilesFrom(b, exe, "./3rdparty/yaml-cpp/src/");
    try libnoise.addLibnoise(b, exe);

    // Add includes
    exe.addIncludePath("include");
    exe.addIncludePath("3rdparty/yaml-cpp/include");
    exe.addIncludePath("3rdparty/generated");

    // Configure options
    exe.defineCMacro("ENABLE_LIBNOISE", "1");
    exe.defineCMacro("OpenGL_GL_PREFERENCE", "LEGACY");

    // Link system libraries
    exe.linkLibC();
    exe.linkLibCpp();
    exe.linkSystemLibrary("X11");
    exe.linkSystemLibrary("GL");
    exe.linkSystemLibrary("pthread");
    exe.linkSystemLibrary("png");
    exe.install();
}

// Add all .cpp files from the given path
fn addSourceFilesFrom(b: *Builder, step: *LibExeObjStep, path: []const u8) !void {
    var dir = std.fs.cwd().openIterableDir(path, .{}) catch {
        std.debug.print("Unable to open path '{s}'!", .{path});
        return;
    };

    var it = dir.iterate();
    while (try it.next()) |file| {
        if (file.kind != .File or !std.mem.endsWith(u8, file.name, ".cpp")) {
            continue;
        }
        const name = try std.fmt.allocPrint(b.allocator, "{s}/{s}", .{ path, file.name });
        step.addCSourceFile(name, CxxArgs);
    }
}
