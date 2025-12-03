const std = @import("std");
const allocator = std.mem.Allocator;
const assert = std.debug.assert;

pub fn main() !void {
    var alloc_state: std.heap.DebugAllocator(.{}) = .init;
    defer std.debug.assert(alloc_state.deinit() == .ok);
    const gpa = alloc_state.allocator();

    const input = try std.fs.cwd().readFileAlloc("../inputs/3.txt", gpa, .unlimited);
    defer gpa.free(input);

    std.debug.print("Task1:\n{}\n", .{task1(input)});
    std.debug.print("Task2:\n{}\n", .{task2(input)});
}

test {
    const example =
        \\987654321111111
        \\811111111111119
        \\234234234234278
        \\818181911112111
    ;

    try std.testing.expectEqual(357, task1(example));
    try std.testing.expectEqual(3121910778619, task2(example));
}

fn count(comptime on_count: usize, input: []const u8) u64 {
    var sum: u64 = 0;
    var iter = std.mem.tokenizeScalar(u8, input, '\n');
    while (iter.next()) |bank| {
        var buf: [on_count]u8 = undefined;
        var start: usize = 0;
        for (0..on_count) |i| {
            const end = bank.len - on_count - 1 + i;
            const index = std.mem.findMax(u8, bank[start..end]) + start;
            start = index + 1;
            buf[i] = bank[index];
        }

        const num = std.fmt.parseInt(u64, &buf, 10) catch unreachable;
        sum += num;
    }
    return sum;
}

fn task1(input: []const u8) u64 {
    return count(2, input);
}

fn task2(input: []const u8) u64 {
    return count(12, input);
}
