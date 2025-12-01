const std = @import("std");
const Allocator = std.mem.Allocator;

pub fn main() !void {
    var alloc_state: std.heap.DebugAllocator(.{}) = .init;
    defer std.debug.assert(alloc_state.deinit() == .ok);
    const gpa = alloc_state.allocator();

    const input = try std.fs.cwd().readFileAlloc("../inputs/1.txt", gpa, .unlimited);
    defer gpa.free(input);

    const parsed = try parse(gpa, input);
    defer gpa.free(parsed);

    std.debug.print("Task1:\n{}\n", .{try task1(parsed)});
    std.debug.print("Task2:\n{}\n", .{try task2(parsed)});
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

fn parse(gpa: Allocator, input: []const u8) ![]i32 {
    var numbers: std.ArrayList(i32) = .empty;

    var iter = std.mem.tokenizeScalar(u8, input, '\n');
    while (iter.next()) |token| {
        const num = try std.fmt.parseInt(i32, token[1..], 10);
        const with_sign: i32 = switch (token[0]) {
            'L' => -num,
            'R' => num,
            else => return error.InvalidInput,
        };
        try numbers.append(gpa, with_sign);
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
