const std = @import("std");
const Allocator = std.mem.Allocator;

pub fn main() !void {
    var alloc_state: std.heap.DebugAllocator(.{}) = .init;
    defer std.debug.assert(alloc_state.deinit() == .ok);
    const gpa = alloc_state.allocator();

    const input = try std.fs.cwd().readFileAlloc("../inputs/2.txt", gpa, .unlimited);
    defer gpa.free(input);

    const parsed = try parse(gpa, input[0 .. input.len - 1]);
    defer gpa.free(parsed);

    std.debug.print("Task1:\n{}\n", .{try task1(parsed)});
    std.debug.print("Task2:\n{}\n", .{try task2(parsed)});
}

test {
    const example = "11-22,95-115,998-1012,1188511880-1188511890,222220-222224,1698522-1698528,446443-446449,38593856-38593862,565653-565659,824824821-824824827,2121212118-2121212124";
    const gpa = std.testing.allocator;
    const parsed = try parse(gpa, example);
    defer gpa.free(parsed);
    try std.testing.expectEqual(1227775554, task1(parsed));
    try std.testing.expectEqual(4174379265, task2(parsed));
}

const Pair = struct { usize, usize };
fn parse(gpa: Allocator, input: []const u8) ![]const Pair {
    var out: std.ArrayList(Pair) = .empty;
    errdefer out.deinit(gpa);

    var iter = std.mem.tokenizeScalar(u8, input, ',');
    while (iter.next()) |token| {
        var inner = std.mem.splitScalar(u8, token, '-');
        const from_string = inner.next() orelse return error.InvalidInput;
        const to_string = inner.next() orelse return error.InvalidInput;
        const from = try std.fmt.parseInt(usize, from_string, 10);
        const to = try std.fmt.parseInt(usize, to_string, 10);
        try out.append(gpa, .{ from, to });
    }
    return out.toOwnedSlice(gpa);
}
fn task1(ranges: []const Pair) !usize {
    var buf: [512]u8 = undefined;
    var sum: usize = 0;
    for (ranges) |pair| {
        const from, const to = pair;
        for (from..to + 1) |id| {
            const end = std.fmt.bufPrint(&buf, "{d}", .{id});
            const chars = buf[0..end];
            if (@rem(chars.len, 2) == 0) {
                const half = chars.len / 2;
                if (std.mem.eql(u8, chars[0..half], chars[half..])) {
                    // std.debug.print("wrong id: {d}\n", .{id});
                    sum += id;
                }
            }
        }
    }
    return sum;
}

fn task2(ranges: []const Pair) !usize {
    var buf: [512]u8 = undefined;
    var sum: usize = 0;
    for (ranges) |pair| {
        const from, const to = pair;

        ids: for (from..to + 1) |id| {
            const end = try std.fmt.bufPrint(&buf, "{d}", .{id});
            const chars = buf[0..end];
            window: for (1..chars.len) |len| if (@rem(chars.len, len) == 0) {
                var windows = std.mem.window(u8, chars, len, len);
                const first = windows.first();

                while (windows.next()) |n|
                    if (!std.mem.eql(u8, first, n)) continue :window;

                sum += id;
                continue :ids;
            };
        }
    }
    return sum;
}
