const std = @import("std");
const Allocator = std.mem.Allocator;

pub fn readInput(gpa: Allocator) ![]const u8 {
    const file = try std.fs.cwd().openFile("../inputs/1.txt", .{});
    defer file.close();

    var buf: [1024]u8 = undefined;
    var reader = file.reader(&buf);

    return try reader.interface.allocRemaining(gpa, .unlimited);
}

pub fn main() !void {
    var alloc_state: std.heap.DebugAllocator(.{}) = .init;
    defer std.debug.assert(alloc_state.deinit() == .ok);
    const gpa = alloc_state.allocator();

    const input = try readInput(gpa);
    std.debug.print("{any}", .{input[input.len - 100 ..]});
    defer gpa.free(input);

    const parsed = try parse(gpa, input);
    defer gpa.free(parsed);

    const res1 = try task1(parsed);
    std.debug.print("res1:\n{}\n", .{res1});

    const res2 = try task2(parsed);
    std.debug.print("res2:\n{}\n", .{res2});
}

test {
    const gpa = std.testing.allocator;
    const parsed = try parse(gpa, example);
    defer gpa.free(parsed);

    try std.testing.expectEqual(3, task1(parsed));

    try std.testing.expectEqual(6, task2(parsed));
}

const example =
    \\L68
    \\L30
    \\R48
    \\L5
    \\R60
    \\L55
    \\L1
    \\L99
    \\R14
    \\L82
;

fn parseNumber(remaining: []const u8) !struct { i32, usize } {
    const newline_pos = std.mem.findScalar(u8, remaining, '\n') orelse remaining.len;
    const num = try std.fmt.parseInt(i32, remaining[0..newline_pos], 10);
    return .{ num, newline_pos + 1 };
}

fn parse(gpa: Allocator, input: []const u8) ![]i32 {
    var idx: usize = 0;
    var numbers: std.ArrayList(i32) = .empty;
    while (true) {
        if (idx >= input.len) break;
        const sign: i2 = switch (input[idx]) {
            'L' => -1,
            'R' => 1,
            else => {
                return error.InvalidInput;
            },
        };
        const num, const len = try parseNumber(input[idx + 1 ..]);
        idx += 1 + len;
        try numbers.append(gpa, sign * num);
    }
    return numbers.toOwnedSlice(gpa);
}

fn task1(numbers: []const i32) !u32 {
    var zero_count: u32 = 0;
    var state: i32 = 50;
    for (numbers) |num| {
        state = @mod(state + num, 100);
        if (state == 0) zero_count += 1;
    }
    return zero_count;
}

fn task2(numbers: []const i32) !u32 {
    var zero_count: u32 = 0;
    var state: i32 = 50;
    for (numbers) |num| {
        const one: i32, const abs: usize = if (num >= 0) .{ 1, @intCast(num) } else .{ -1, @intCast(-num) };
        for (0..abs) |_| {
            state = @mod(state + one, 100);
            if (state == 0) zero_count += 1;
        }
    }
    return zero_count;
}
