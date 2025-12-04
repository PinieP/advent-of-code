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

const Matrix = struct {
    buf: []u8,
    row_count: usize,
    col_count: usize,
    stride: [2]usize,

    fn calculateIndex(self: Matrix, i: usize, j: usize) ?usize {
        return if (i >= self.row_count or j >= self.col_count)
            null
        else
            i * self.stride[0] + j * self.stride[1];
    }
    fn atOrNull(self: Matrix, i: usize, j: usize) ?u8 {
        const index = self.calculateIndex(i, j) orelse
            return null;
        return self.buf[index];
    }
    fn at(self: Matrix, i: usize, j: usize) u8 {
        return self.atOrNull(i, j).?;
    }

    fn ptrAt(self: *Matrix, i: usize, j: usize) *u8 {
        const index = self.calculateIndex(i, j).?;
        return &self.buf[index];
    }

    pub fn format(
        self: @This(),
        writer: *std.Io.Writer,
    ) std.Io.Writer.Error!void {
        for (0..self.row_count) |i| {
            for (0..self.col_count) |j| try writer.writeByte(self.at(i, j));
            try writer.writeByte('\n');
        }
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
    //std.debug.print("({d}, {d})", .{ pos.row, pos.col });
    var count: u64 = 0;
    inline for (0..3) |row_offs| {
        if (std.math.sub(usize, pos.row + row_offs, 1) catch null) |row|
            inline for (0..3) |col_offs|
                if (std.math.sub(usize, pos.col + col_offs, 1) catch null) |col| {
                    // std.debug.print("\t({d}, {d}) \n", .{ row, col });
                    if (mat.atOrNull(row, col) == '@') {
                        count += 1;
                    }
                };
    }
    return count - 1;
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
    for (0..mat.row_count) |i| {
        for (0..mat.col_count) |j|
            if (mat.at(i, j) == '@') {
                const neighbors = countPaperNeighbors(mat.*, .pos(i, j));
                if (neighbors < 4) {
                    if (mode == .remove) mat.ptrAt(i, j).* = 'x';
                    count += 1;
                }
            };
    }
    return count;
}

fn makeMat(buf: []u8) !Matrix {
    const col_count = std.mem.findScalar(u8, buf, '\n') orelse return error.MissingNewline;
    const row_count = buf.len / (col_count + 1);

    return .{
        .buf = buf,
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
