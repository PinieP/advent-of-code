use std::{collections::HashMap, fs};

fn parse(input: &str) -> Vec<u64> {
    input
        .split_whitespace()
        .map(|num| num.parse().unwrap())
        .collect()
}

pub fn count_digits(mut num: u64) -> u64 {
    let mut counter = 0;
    while num > 0 {
        counter += 1;
        num /= 10;
    }
    counter
}

fn next_state(current: &HashMap<u64, usize>) -> HashMap<u64, usize> {
    let mut next = HashMap::new();
    for (&num, &count) in current {
        let new = match num {
            0 => 1,
            _ => {
                let digits = count_digits(num);
                if digits % 2 == 0 {
                    let mask = 10u32.pow(digits as u32 / 2) as u64;
                    *next.entry(num / mask).or_insert(0) += count;
                    num % mask
                } else {
                    num * 2024
                }
            }
        };
        *next.entry(new).or_insert(0) += count;
    }
    next
}

fn n_blinks(n: usize, stones: &[u64]) -> u64 {
    let mut stones = stones.iter().map(|&num| (num, 1)).collect();
    for _ in 0..n {
        stones = next_state(&stones);
    }
    stones.values().sum::<usize>() as u64
}

fn puzzle1(input: &str) -> u64 {
    n_blinks(25, &parse(input))
}

fn puzzle2(input: &str) -> u64 {
    n_blinks(75, &parse(input))
}

fn main() {
    let input = fs::read_to_string("../inputs/11.txt").unwrap();

    println!("result of puzzle1: {}", puzzle1(&input));
    println!("result of puzzle2: {}", puzzle2(&input));
}

#[cfg(test)]
mod tests {
    use super::*;

    const TEST_INPUT: &str = r"125 17";

    #[test]
    fn test_count_digits() {
        assert_eq!(count_digits(7), 1);
        assert_eq!(count_digits(10), 2);
        assert_eq!(count_digits(12345), 5);
        assert_eq!(count_digits(012345), 5);
    }

    #[test]
    fn test_puzzle1() {
        assert_eq!(puzzle1(TEST_INPUT), 55312);
    }

    #[test]
    fn test_puzzle2() {
        assert_eq!(puzzle1(TEST_INPUT), 55312);
    }
}
