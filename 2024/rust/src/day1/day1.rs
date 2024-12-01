use std::{collections::HashMap, fs, iter::zip};

fn parse(input: &str) -> (Vec<i32>, Vec<i32>) {
    let parsed = input
        .lines()
        .map(|l| l.split_whitespace().map(|num| num.parse().unwrap()));

    let mut left = Vec::new();
    let mut right = Vec::new();
    for mut e in parsed {
        left.push(e.next().unwrap());
        right.push(e.next().unwrap());
    }
    (left, right)
}

fn puzzle1(input: &str) -> i32 {
    let (mut left, mut right) = parse(input);
    left.sort();
    right.sort();

    zip(left, right).map(|(a, b)| (a - b).abs()).sum()
}

fn puzzle2(input: &str) -> i32 {
    let (left, mut right) = parse(input);
    right.sort();
    let map: HashMap<_, _> = right
        .chunk_by(PartialEq::eq)
        .map(|slice| (slice[0], slice.len() as i32))
        .collect();
    left.iter()
        .map(|&e| e * &map.get(&e).copied().unwrap_or(0))
        .sum()
}

fn main() {
    let input = fs::read_to_string("../inputs/1.txt").unwrap();

    println!("result of puzzle1: {}", puzzle1(&input));
    println!("result of puzzle2: {}", puzzle2(&input));
}

#[cfg(test)]
mod tests {
    use super::*;

    const TEST_INPUT: &str = r"3   4
4   3
2   5
1   3
3   9
3   3
";

    #[test]
    fn test_puzzle1() {
        assert_eq!(puzzle1(TEST_INPUT), 11);
    }

    #[test]
    fn test_puzzle2() {
        assert_eq!(puzzle2(TEST_INPUT), 31);
    }
}
