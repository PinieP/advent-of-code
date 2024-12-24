use rust::utils::{self, Matrix, MatrixBuf, MatrixView};
use std::fs;

fn parse(input: &str) -> MatrixBuf<u8> {
    let width = input.find('\n').unwrap();
    let height = input.len() / (width + 1);

    MatrixBuf::new(
        input.bytes().filter(|&c| c != b'\n').collect(),
        (height, width),
    )
}

fn matches(
    matrix: MatrixView<u8>,
    base: (usize, usize),
    slice: &[(isize, isize)],
    pat: &[bool],
) -> bool {
    assert_eq!(slice.len(), pat.len());
    slice
        .iter()
        .map(|&e| {
            let reg = *utils::offset_by(base, e)
                .and_then(|ij| matrix.get(ij))
                .unwrap_or(&254);
            reg == matrix[base] || reg == 255
        })
        .eq(pat.to_owned())
}
type Coordinate = (usize, usize);

fn count_corners(matrix: MatrixView<u8>, coord: Coordinate) -> usize {
    let m = |slice, pat| matches(matrix, coord, slice, pat) as usize;
    let res = m(&[(0, -1), (-1, -1), (-1, 0)], &[true, false, true])
        + m(&[(0, 1), (1, 1), (1, 0)], &[true, false, true])
        + m(&[(0, 1), (-1, 1), (-1, 0)], &[true, false, true])
        + m(&[(0, -1), (1, -1), (1, 0)], &[true, false, true])
        + m(&[(0, -1), (-1, 0)], &[false, false])
        + m(&[(0, 1), (1, 0)], &[false, false])
        + m(&[(0, 1), (-1, 0)], &[false, false])
        + m(&[(0, -1), (1, 0)], &[false, false]);
    assert!(res <= 4);
    res
}

fn eat_region(matrix_buf: &mut MatrixBuf<u8>, start: Coordinate) -> (usize, usize, usize) {
    let mut matrix = matrix_buf.as_view_mut();

    let region = matrix[start];

    let mut perimeter = 0;
    let mut area = 0;
    let mut corners = 0;

    let mut menu = vec![start];
    let mut visited = vec![];

    while let Some(top) = menu.pop() {
        if matrix[top] == 255 {
            continue;
        }
        visited.push(top);
        let current_corners = count_corners(matrix.as_view(), top);
        matrix[top] = 255;
        let same_region_neighbors: Vec<_> = utils::neightbour_indices(matrix.as_view(), top)
            .filter(|&coord| {
                let reg = matrix[coord];
                reg == region || reg == 255
            })
            .collect();
        area += 1;
        perimeter += 4 - same_region_neighbors.len();
        corners += current_corners;

        menu.extend(same_region_neighbors.iter())
    }
    for v in visited {
        matrix[v] = 0;
    }
    (area, perimeter, corners)
}

fn puzzle1(input: &str) -> u64 {
    let mut matrix = parse(input);
    let indices = matrix.as_view().indices().flatten();

    let mut counter = 0;
    for ij in indices {
        if matrix.as_view()[ij] == 0 {
            continue;
        }
        let (area, perimeter, _) = eat_region(&mut matrix, ij);
        counter += area * perimeter;
    }
    counter as u64
}

fn puzzle2(input: &str) -> u64 {
    let mut matrix = parse(input);
    let indices = matrix.as_view().indices().flatten();

    let mut counter = 0;
    for ij in indices {
        if matrix.as_view()[ij] == 0 {
            continue;
        }
        let (area, _, corners) = eat_region(&mut matrix, ij);
        counter += area * corners;
    }
    counter as u64
}

fn main() {
    let input = fs::read_to_string("../inputs/12.txt").unwrap();

    println!("result of puzzle1: {}", puzzle1(&input));
    println!("result of puzzle2: {}", puzzle2(&input));
}

#[cfg(test)]
mod tests {
    use crate::count_corners;

    use super::*;

    const TEST_INPUT: &str = r"RRRRIICCFF
RRRRIICCCF
VVRRRCCFFF
VVRCCCJFFF
VVVVCJJCFE
VVIVCCJJEE
VVIIICJJEE
MIIIIIJJEE
MIIISIJEEE
MMMISSJEEE
";

    #[test]
    fn test_count_corners() {
        assert!(matches(
            MatrixBuf::<_>::new(vec![0, 1], (1, 2)).as_view(),
            (0, 1),
            &[(0, -1)],
            &[false]
        ));
        assert!(matches(
            MatrixBuf::<_>::new(vec![1, 1], (1, 2)).as_view(),
            (0, 1),
            &[(0, -1)],
            &[true]
        ));
        assert_eq!(
            count_corners(
                MatrixBuf::<_>::new(vec![0, 0, 0, 0, 1, 0, 0, 0, 0], (3, 3)).as_view(),
                (1, 1)
            ),
            4
        );
        assert_eq!(
            count_corners(
                MatrixBuf::<_>::new(vec![0, 1, 0, 1, 1, 1, 0, 1, 0], (3, 3)).as_view(),
                (1, 1)
            ),
            4
        );
        assert_eq!(
            count_corners(
                MatrixBuf::<_>::new(vec![1, 255, 1, 1], (2, 2)).as_view(),
                (0, 0)
            ),
            1
        )
    }

    #[test]
    fn test_puzzle1() {
        assert_eq!(puzzle1(TEST_INPUT), 1930);
    }

    #[test]
    fn test_puzzle2() {
        assert_eq!(puzzle2(TEST_INPUT), 1206);
    }
}
