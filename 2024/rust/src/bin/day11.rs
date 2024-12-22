use std::fs;

fn parse(input: &str) -> Vec<u32> {
    input
        .split_whitespace()
        .map(|num| num.parse().unwrap())
        .collect()
}

pub fn count_digits(num: u32) -> u32 {
    num / 10 + 1
}

fn next_state(current: &[u32]) -> Vec<u32> {
    current
        .iter()
        .flat_map(|&num| match num {
            0 => vec![1],
            _ => {
                let digits = count_digits(num);
                if digits % 2 == 0 {
                    let top = 10u32.pow(digits / 2 - 1);
                    vec![top, num - top]
                } else {
                    vec![num * 2024]
                }
            }
        })
        .collect()
}

fn puzzle1(input: &str) -> u64 {
    let mut stones = parse(input);
    for i in 0..25 {
        println!("current_state: {:?}", stones);
        stones = next_state(&stones);
    }
    println!("result : {:?}", stones);

    todo!()
}

fn puzzle2(input: &str) -> u64 {
    todo!()
}

fn main() {
    let input = fs::read_to_string("../inputs/10.txt").unwrap();

    println!("result of puzzle1: {}", puzzle1(&input));
    println!("result of puzzle2: {}", puzzle2(&input));
}

#[cfg(test)]
mod tests {
    use super::*;

    const TEST_INPUT: &str = r"125 17";

    #[test]
    fn test_count_digits() {
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
        assert_eq!(puzzle2(TEST_INPUT), 81);
    }
}
