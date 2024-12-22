use rust::utils::{Matrix, MatrixBuf, MatrixView};

use std::{collections::HashSet, fs};

fn parse(input: &str) -> MatrixBuf<u8> {
    let vec = input
        .chars()
        .filter_map(|c| {
            if c == '\n' {
                None
            } else {
                Some(c.to_digit(10).unwrap() as u8)
            }
        })
        .collect();
    let width = input.find('\n').unwrap();
    let height = input.len() / (width + 1) + 1;
    MatrixBuf::new(vec, (height, width))
}

fn offset_by((y, x): Coordinate, (dy, dx): (isize, isize)) -> Option<Coordinate> {
    Some((
        (y as isize + dy).try_into().ok()?,
        (x as isize + dx).try_into().ok()?,
    ))
}

fn valid_neighbors(matrix: MatrixView<u8>, position: Coordinate) -> Vec<Coordinate> {
    [(-1, 0), (0, 1), (1, 0), (0, -1)]
        .iter()
        .filter_map(|&offset| offset_by(position, offset))
        .filter(|&e| matrix.in_bounds(e))
        .filter_map(|new_pos| {
            if matrix.get(position)? + 1 == *matrix.get(new_pos)? {
                Some(new_pos)
            } else {
                None
            }
        })
        .collect()
}

type Coordinate = (usize, usize);

fn walk_paths(matrix: MatrixView<u8>, position: Coordinate) -> Vec<Coordinate> {
    if let Some(9) = matrix.get(position) {
        return vec![position];
    }

    valid_neighbors(matrix, position)
        .into_iter()
        .flat_map(|pos| walk_paths(matrix, pos))
        .collect()
}

fn trailheads(matrix: MatrixView<u8>) -> impl Iterator<Item = Coordinate> + '_ {
    matrix
        .data
        .chunks(matrix.extents.1)
        .enumerate()
        .flat_map(|(y, row)| row.iter().enumerate().map(move |(x, e)| ((y, x), e)))
        .filter_map(|(coord, &e)| if e == 0 { Some(coord) } else { None })
}

fn puzzle1(input: &str) -> u64 {
    let matrix = parse(input);
    //dbg!(trailheads(matrix.as_view()).collect::<Vec<_>>());
    trailheads(matrix.as_view())
        .map(|position| {
            walk_paths(matrix.as_view(), position)
                .iter()
                .collect::<HashSet<_>>()
                .len() as u64
        })
        .collect::<Vec<_>>()
        .into_iter()
        .sum()
}

fn puzzle2(input: &str) -> u64 {
    let matrix = parse(input);
    trailheads(matrix.as_view())
        .map(|position| walk_paths(matrix.as_view(), position).len() as u64)
        .sum()
}

fn main() {
    let input = fs::read_to_string("../inputs/10.txt").unwrap();

    println!("result of puzzle1: {}", puzzle1(&input));
    println!("result of puzzle2: {}", puzzle2(&input));
}

#[cfg(test)]
mod tests {
    use super::*;

    const TEST_INPUT: &str = r"89010123
78121874
87430965
96549874
45678903
32019012
01329801
10456732";

    #[test]
    fn test_puzzle1() {
        assert_eq!(puzzle1(TEST_INPUT), 36);
    }

    #[test]
    fn test_puzzle2() {
        assert_eq!(puzzle2(TEST_INPUT), 81);
    }
}
