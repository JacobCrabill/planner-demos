const std = @import("std");
const Builder = std.build.Builder;
const LibExeObjStep = std.build.LibExeObjStep;

const CxxArgs = &[_][]const u8{};

pub fn addLibnoise(b: *Builder, exe: *LibExeObjStep) !void {
    try addSourceFilesFrom(b, exe, "3rdparty/libnoise/src/");
    try addSourceFilesFrom(b, exe, "3rdparty/libnoise/src/model");
    try addSourceFilesFrom(b, exe, "3rdparty/libnoise/src/module");

    exe.addIncludePath("3rdparty/libnoise/include/");
    exe.addIncludePath("3rdparty/libnoise/include/noise");
    exe.addIncludePath("3rdparty/libnoise/include/noise/model");
    exe.addIncludePath("3rdparty/libnoise/include/noise/module");
}

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
