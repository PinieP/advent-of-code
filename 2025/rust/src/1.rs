use std::{error::Error, fs};

fn main() -> Result<(), Box<dyn Error>> {
    let input = fs::read_to_string("../inputs/1.txt")?;
    let parsed = parse(&input);

    let taks1 = task1(&parsed);
    println!("task1:\n{taks1}");
    let taks2 = task2(&parsed);
    println!("task2:\n{taks2}");

    Ok(())
}

fn parse(input: &str) -> Vec<i32> {
    input[..input.len() - 1]
        .split('\n')
        .map(|it| {
            let num: i32 = it[1..].parse().unwrap();
            let with_sign = match it.as_bytes()[0] {
                b'L' => -num,
                b'R' => num,
                _ => unreachable!(),
            };

            num * with_sign
        })
        .collect()
}

fn task1(numbers: &[i32]) -> i32 {
    let mut state = 50;
    let mut counts = 0;

    for &num in numbers {
        state = (state as i32 + num).rem_euclid(100);
        if state == 0 {
            counts += 1
        }
    }
    counts
}

fn task2(numbers: &[i32]) -> i32 {
    let mut state = 50;
    let mut counts = 0;

    for &num in numbers {
        let abs = num.unsigned_abs();
        let one = num.signum();
        for _ in 0..abs {
            state = (state as i32 + one).rem_euclid(100);
            if state == 0 {
                counts += 1
            }
        }
    }
    counts
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test() {
        let example = "L68
L30
R48
L5
R60
L55
L1
L99
R14
L82";
        let parsed = parse(example);
        assert_eq!(3, task1(&parsed));

        assert_eq!(6, task2(&parsed));
    }
}
