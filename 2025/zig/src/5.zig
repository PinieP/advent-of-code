const std = @import("std");
const Allocator = std.mem.Allocator;
const assert = std.debug.assert;

pub fn main() !void {
    var alloc_state: std.heap.DebugAllocator(.{}) = .init;
    defer std.debug.assert(alloc_state.deinit() == .ok);
    const gpa = alloc_state.allocator();

    var buf: [4096]u8 = undefined;

    const f = try std.fs.cwd().openFile("../inputs/5.txt", .{});
    var reader_state = f.reader(&buf);
    const r = &reader_state.interface;

    const input = try std.fs.cwd().readFileAlloc("../inputs/4.txt", gpa, .unlimited);
    defer gpa.free(input);

    var ranges = try parseRanges(gpa, r);
    defer ranges.deinit(gpa);

    std.debug.print("Task1:\n{}\n", .{try task1(ranges, r)});
    std.debug.print("Task2:\n{}\n", .{task2(ranges)});
}

const RangeTable = struct {
    items: std.ArrayList(Data),

    fn init(gpa: Allocator) !RangeTable {
        var self: RangeTable = .{ .items = .empty };
        try self.items.appendSlice(gpa, &.{ .{ .pos = 0, .tag = .invalid }, .{ .pos = std.math.maxInt(u63), .tag = .valid } });
        return self;
    }
    fn deinit(self: *RangeTable, gpa: Allocator) void {
        self.items.deinit(gpa);
    }

    const Data = packed struct {
        pos: u63,
        tag: enum(u1) { valid, invalid },

        fn order(lhs: u62, rhs: Data) std.math.Order {
            return std.math.order(lhs, rhs.pos);
        }
    };

    fn isValid(self: RangeTable, item: u62) bool {
        const idx = std.sort.upperBound(Data, self.items.items, item, Data.order) -| 1;
        return self.items.items[idx].tag == .valid;
    }

    fn insertRange(self: *RangeTable, gpa: Allocator, from: u62, to: u62) !void {
        if (from > to) return error.InvalidRange;

        const items = self.items.items;
        const lower_bound_idx = std.sort.upperBound(Data, items, from, Data.order) -| 1;
        const lower_bound = items[lower_bound_idx];

        var buf: [2]Data = undefined;
        var list: std.ArrayList(Data) = .initBuffer(&buf);

        var start = lower_bound_idx + 1;
        if (from == 0) {
            items[0].tag = .valid;
        } else if (lower_bound.pos == from) {
            if (lower_bound.tag == .invalid) start = lower_bound_idx;
        } else if (lower_bound.tag == .invalid) {
            list.appendAssumeCapacity(.{ .pos = from, .tag = .valid });
        }

        const upper_bound_idx = std.sort.lowerBound(Data, items, to, Data.order);
        const upper_bound = items[upper_bound_idx];

        var end: usize = upper_bound_idx;
        if (upper_bound.pos == to) {
            if (upper_bound.tag == .valid) end = upper_bound_idx + 1;
        } else {
            assert(to < upper_bound.pos);
            if (upper_bound.tag == .valid) {
                list.appendAssumeCapacity(.{ .pos = to, .tag = .invalid });
            }
        }

        try self.items.replaceRange(gpa, start, end - start, list.items);
    }
};
test "RangeTable" {
    const gpa = std.testing.allocator;

    var ranges: RangeTable = try .init(gpa);
    defer ranges.deinit(gpa);

    const end: RangeTable.Data = .{ .pos = std.math.maxInt(u63), .tag = .valid };
    try ranges.insertRange(gpa, 10, 30);
    try std.testing.expectEqualSlices(
        RangeTable.Data,
        &.{ .{ .pos = 0, .tag = .invalid }, .{ .pos = 10, .tag = .valid }, .{ .pos = 30, .tag = .invalid }, end },
        ranges.items.items,
    );

    try ranges.insertRange(gpa, 30, 32);
    try std.testing.expectEqualSlices(
        RangeTable.Data,
        &.{ .{ .pos = 0, .tag = .invalid }, .{ .pos = 10, .tag = .valid }, .{ .pos = 32, .tag = .invalid }, end },
        ranges.items.items,
    );

    try ranges.insertRange(gpa, 3, 33);
    try std.testing.expectEqualSlices(
        RangeTable.Data,
        &.{ .{ .pos = 0, .tag = .invalid }, .{ .pos = 3, .tag = .valid }, .{ .pos = 33, .tag = .invalid }, end },
        ranges.items.items,
    );

    try ranges.insertRange(gpa, 0, 2);
    try std.testing.expectEqualSlices(
        RangeTable.Data,
        &.{ .{ .pos = 0, .tag = .valid }, .{ .pos = 2, .tag = .invalid }, .{ .pos = 3, .tag = .valid }, .{ .pos = 33, .tag = .invalid }, end },
        ranges.items.items,
    );
}

test "fuzz" {
    const Ctx = struct {
        ranges: *RangeTable,
        gpa: Allocator,
        fn testOne(ctx: @This(), slice: []const u8) anyerror!void {
            if (slice.len >= 16) {
                var buf: [16]u8 align(8) = undefined;
                @memcpy(&buf, slice[0..16]);
                const num: *const u12 = @ptrCast(buf[0..2]);
                const offs: *const u12 = @ptrCast(buf[8..10]);
                const from: u62 = num.*;
                const to: u62 = from + offs.*;

                try ctx.ranges.insertRange(ctx.gpa, from, to);
                for (from..to) |i| {
                    // std.debug.print("checking: {d}\n", .{i});
                    const is_valid = ctx.ranges.isValid(@intCast(i));
                    try std.testing.expect(is_valid);
                }
                if (ctx.ranges.items.items.len > 10000) {
                    ctx.ranges.deinit(ctx.gpa);
                    ctx.ranges.* = try .init(ctx.gpa);
                }
            }
        }
    };
    var alloc_state: std.heap.DebugAllocator(.{}) = .init;
    defer assert(alloc_state.deinit() == .ok);
    const gpa = alloc_state.allocator();
    var ranges: RangeTable = try .init(gpa);
    defer ranges.deinit(gpa);

    try std.testing.fuzz(
        Ctx{ .ranges = &ranges, .gpa = gpa },
        Ctx.testOne,
        .{},
    );
}

test {
    const example =
        \\3-5
        \\10-14
        \\16-20
        \\12-18
        \\
        \\1
        \\5
        \\8
        \\11
        \\17
        \\32
    ;
    const gpa = std.testing.allocator;
    var reader: std.Io.Reader = .fixed(example);
    var ranges = try parseRanges(gpa, &reader);
    defer ranges.deinit(gpa);

    try std.testing.expectEqual(3, try task1(ranges, &reader));
    try std.testing.expectEqual(14, task2(ranges));
}

fn parseRanges(gpa: Allocator, r: *std.Io.Reader) !RangeTable {
    var ranges: RangeTable = try .init(gpa);
    while (try r.peekByte() != '\n') {
        const from_bytes = try r.takeDelimiter('-') orelse return error.InvalidInput;
        const from = try std.fmt.parseInt(u62, from_bytes, 10);
        const to_bytes = try r.takeDelimiter('\n') orelse return error.InvalidInput;
        const to = try std.fmt.parseInt(u62, to_bytes, 10) + 1;

        try ranges.insertRange(gpa, from, to);
        // std.debug.print("insert: {d}..{d}\n", .{ from, to });
        //std.debug.print("{any}\n", .{ranges.items.items});
    }
    _ = try r.takeByte();
    return ranges;
}

fn task1(ranges: RangeTable, r: *std.Io.Reader) !u64 {
    var count: u64 = 0;
    while (try r.takeDelimiter('\n')) |num_bytes| {
        const num = try std.fmt.parseInt(u62, num_bytes, 10);
        if (ranges.isValid(num)) count += 1;
    }
    return count;
}

fn task2(ranges: RangeTable) u64 {
    var count: u64 = 0;

    var items = ranges.items.items;
    if (items.len % 2 == 0) {
        assert(items[0].tag == .invalid);
        items = items[1..];
    }

    for (0..items.len / 2) |i| {
        const idx = 2 * i;
        count += items[idx + 1].pos - items[idx].pos;
    }
    return count;
}
