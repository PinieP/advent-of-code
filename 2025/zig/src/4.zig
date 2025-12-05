const std = @import("std");
const Allocator = std.mem.Allocator;
const assert = std.debug.assert;

pub fn main() !void {
    var alloc_state: std.heap.DebugAllocator(.{}) = .init;
    defer std.debug.assert(alloc_state.deinit() == .ok);
    const gpa = alloc_state.allocator();

    const input = try std.fs.cwd().readFileAlloc("../inputs/4.txt", gpa, .unlimited);
    defer gpa.free(input);

    var mat = try makeMat(input);
    std.debug.print("Task1:\n{}\n", .{task1(&mat)});
    std.debug.print("Task2:\n{}\n", .{task2(&mat)});
}

const Cell = enum(u8) {
    empty = '.',
    paper = '@',
};

const Matrix = struct {
    buf: []Cell,
    row_count: usize,
    col_count: usize,
    stride: [2]usize,

    fn calculateIndex(self: Matrix, row: usize, col: usize) usize {
        assert(row < self.row_count or col < self.col_count);
        return row * self.stride[0] + col * self.stride[1];
    }

    fn at(self: Matrix, row: usize, col: usize) Cell {
        const index = self.calculateIndex(row, col);
        return self.buf[index];
    }

    fn ptrAt(self: *Matrix, row: usize, col: usize) *Cell {
        const index = self.calculateIndex(row, col);
        return &self.buf[index];
    }
};

const Pos = struct {
    row: usize,
    col: usize,
    fn pos(row: usize, col: usize) Pos {
        return .{ .row = row, .col = col };
    }
};

fn countPaperNeighbors(mat: Matrix, pos: Pos) u64 {
    var count: u64 = 0;
    const indices: [8]struct { comptime_int, comptime_int } = .{
        .{ -1, -1 }, .{ -1, 0 }, .{ -1, 1 }, .{ 0, -1 }, .{ 0, 1 }, .{ 1, -1 }, .{ 1, 0 }, .{ 1, 1 },
    };
    inline for (indices) |pair| {
        const i, const j = pair;
        const row = @as(i64, @intCast(pos.row)) + i;
        const col = @as(i64, @intCast(pos.col)) + j;

        if (0 <= row and row < mat.row_count and
            0 <= col and col < mat.col_count and
            mat.at(@intCast(row), @intCast(col)) == .paper)
        {
            count += 1;
        }
    }
    return count;
}

test {
    const example =
        \\..@@.@@@@.
        \\@@@.@.@.@@
        \\@@@@@.@.@@
        \\@.@@@@..@.
        \\@@.@@@@.@@
        \\.@@@@@@@.@
        \\.@.@.@.@@@
        \\@.@@@.@@@@
        \\.@@@@@@@@.
        \\@.@.@@@.@.
        \\
    ;
    const gpa = std.testing.allocator;
    const buf = try gpa.alloc(u8, example.len);
    defer gpa.free(buf);
    @memcpy(buf, example);

    var mat = try makeMat(buf);
    try std.testing.expectEqual(13, task1(&mat));
    try std.testing.expectEqual(43, task2(&mat));
}
const Mode = enum { remove, keep };
fn processAccessible(mat: *Matrix, mode: Mode) u64 {
    var count: u64 = 0;
    for (0..mat.row_count) |i| for (0..mat.col_count) |j| {
        if (mat.at(i, j) != .paper) continue;
        const neighbors = countPaperNeighbors(mat.*, .pos(i, j));
        if (neighbors < 4) {
            if (mode == .remove) mat.ptrAt(i, j).* = .empty;
            count += 1;
        }
    };
    return count;
}

fn makeMat(buf: []u8) !Matrix {
    const col_count = std.mem.findScalar(u8, buf, '\n') orelse return error.MissingNewline;
    const row_count = buf.len / (col_count + 1);

    return .{
        .buf = @ptrCast(buf),
        .row_count = row_count,
        .col_count = col_count,
        .stride = .{ col_count + 1, 1 },
    };
}

fn task1(mat: *Matrix) u64 {
    return processAccessible(mat, .keep);
}
fn task2(mat: *Matrix) u64 {
    var count: u64 = 0;
    while (true) {
        const removed = processAccessible(mat, .remove);
        if (removed == 0) break;
        count += removed;
    }
    return count;
}
