use rust::utils::{Matrix, MatrixBuf};
use std::{
    collections::{BinaryHeap, HashMap, HashSet},
    fs, usize,
};

struct Graph {
    matrix: MatrixBuf<Node>,
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
enum Node {
    Start,
    End,
    Wall,
    Free,
}
#[derive(PartialEq, Eq, Hash, Clone, Copy, Debug)]
enum Direction {
    Up,
    Right,
    Down,
    Left,
}
impl Direction {
    fn offset(&self) -> (isize, isize) {
        match self {
            Self::Up => (-1, 0),
            Self::Right => (0, 1),
            Self::Down => (1, 0),
            Self::Left => (0, -1),
        }
    }
    fn rotate_right(&self) -> Direction {
        match self {
            Self::Up => Self::Right,
            Self::Right => Self::Down,
            Self::Down => Self::Left,
            Self::Left => Self::Up,
        }
    }
    fn rotate_left(&self) -> Direction {
        match self {
            Self::Up => Self::Left,
            Self::Left => Self::Down,
            Self::Down => Self::Right,
            Self::Right => Self::Up,
        }
    }
}

#[derive(PartialEq, Eq, Hash, Clone, Copy, Debug)]
struct NodeHandle {
    coord: (usize, usize),
    dir: Direction,
}

fn offset_by((y, x): (usize, usize), (dy, dx): (isize, isize)) -> Option<(usize, usize)> {
    Some((
        (y as isize + dy).try_into().ok()?,
        (x as isize + dx).try_into().ok()?,
    ))
}

impl Graph {
    fn neighbours(&self, handle: NodeHandle) -> Vec<(NodeHandle, usize)> {
        let mut vec = vec![
            (
                NodeHandle {
                    coord: handle.coord,
                    dir: handle.dir.rotate_left(),
                },
                1000,
            ),
            (
                NodeHandle {
                    coord: handle.coord,
                    dir: handle.dir.rotate_right(),
                },
                1000,
            ),
        ];

        if let Some(coord) = offset_by(handle.coord, handle.dir.offset()) {
            match self.matrix.as_view().get(coord) {
                Some(Node::Wall) => {}
                Some(_) => {
                    vec.push((
                        NodeHandle {
                            coord,
                            dir: handle.dir,
                        },
                        1,
                    ));
                }
                None => {}
            };
        }
        vec
    }

    fn reverse_neighbours(&self, handle: NodeHandle) -> Vec<(NodeHandle, usize)> {
        let mut neighbours = self.neighbours(NodeHandle {
            coord: handle.coord,
            dir: handle.dir.rotate_left().rotate_left(),
        });
        for (NodeHandle { coord: _, dir }, _) in &mut neighbours {
            *dir = dir.rotate_left().rotate_left();
        }
        neighbours
    }

    fn get(&self, handle: NodeHandle) -> Node {
        self.matrix.as_view()[handle.coord].clone()
    }
}

struct QueueItem(NodeHandle, usize);
impl PartialEq for QueueItem {
    fn eq(&self, other: &Self) -> bool {
        self.1 == other.1
    }
}
impl Eq for QueueItem {}
impl PartialOrd for QueueItem {
    fn partial_cmp(&self, other: &Self) -> Option<std::cmp::Ordering> {
        Some(self.cmp(other))
    }
}
impl Ord for QueueItem {
    fn cmp(&self, other: &Self) -> std::cmp::Ordering {
        self.1.cmp(&other.1).reverse()
    }
}

fn parse(input: &str) -> ((usize, usize), MatrixBuf<Node>) {
    let width = input.find('\n').unwrap();
    let height = input.len() / (width + 1);

    let mat = MatrixBuf::new(
        input
            .as_bytes()
            .iter()
            .filter_map(|&c| match c {
                b'#' => Some(Node::Wall),
                b'.' => Some(Node::Free),
                b'S' => Some(Node::Start),
                b'E' => Some(Node::End),
                b'\n' => None,
                _ => panic!(),
            })
            .collect(),
        (height, width),
    );
    let pos = mat
        .as_view()
        .data
        .iter()
        .position(|&n| matches!(n, Node::Start))
        .unwrap();
    ((pos / width, pos % width), mat)
}

fn dijkstra(graph: &Graph, start: NodeHandle) -> (NodeHandle, usize, HashMap<NodeHandle, usize>) {
    let mut distances = HashMap::new();
    let mut queue = BinaryHeap::new();

    distances.insert(start, 0);
    queue.push(QueueItem(start, 0));

    while let Some(QueueItem(u, u_dist)) = queue.pop() {
        if graph.get(u) == Node::End {
            return (u, u_dist, distances);
        }
        for (v, dist) in graph.neighbours(u) {
            let new_dist = u_dist + dist;
            if *distances.entry(v).or_insert(usize::MAX) > new_dist {
                distances.insert(v, new_dist);
                queue.push(QueueItem(v, new_dist));
            }
        }
    }
    panic!()
}

fn puzzle1(input: &str) -> usize {
    let (start, matrix) = parse(input);
    let (_, dist, _) = dijkstra(
        &Graph { matrix },
        NodeHandle {
            coord: start,
            dir: Direction::Right,
        },
    );
    dist
}

fn walk_back(
    graph: &Graph,
    distances: &HashMap<NodeHandle, usize>,
    end: NodeHandle,
) -> HashSet<NodeHandle> {
    let mut stack = vec![end];
    let mut visited = HashSet::from([end]);
    while let Some(handle) = stack.pop() {
        for (h, edge_dist) in graph.reverse_neighbours(handle) {
            if !visited.contains(&h)
                && distances.contains_key(&h)
                && distances[&handle] == distances[&h] + edge_dist
            {
                visited.insert(h);
                stack.push(h);
            };
        }
    }
    visited
}

fn puzzle2(input: &str) -> usize {
    let (start, matrix) = parse(input);
    let graph = Graph { matrix };
    let (end, _, distances) = dijkstra(
        &graph,
        NodeHandle {
            coord: start,
            dir: Direction::Right,
        },
    );
    walk_back(&graph, &distances, end)
        .iter()
        .map(|&NodeHandle { coord, dir: _ }| coord)
        .collect::<HashSet<_>>()
        .len()
}

fn main() {
    let input = fs::read_to_string("../inputs/16.txt").unwrap();

    println!("result of puzzle1: {}", puzzle1(&input));
    println!("result of puzzle2: {}", puzzle2(&input));
}

#[cfg(test)]
mod tests {
    use super::*;

    const TEST_INPUT: &str = r"#################
#...#...#...#..E#
#.#.#.#.#.#.#.#.#
#.#.#.#...#...#.#
#.#.#.#.###.#.#.#
#...#.#.#.....#.#
#.#.#.#.#.#####.#
#.#...#.#.#.....#
#.#.#####.#.###.#
#.#.#.......#...#
#.#.###.#####.###
#.#.#...#.....#.#
#.#.#.#####.###.#
#.#.#.........#.#
#.#.#.#########.#
#S#.............#
#################
";

    #[test]
    fn test_puzzle1() {
        assert_eq!(puzzle1(TEST_INPUT), 11048);
    }

    #[test]
    fn test_puzzle2() {
        assert_eq!(puzzle2(TEST_INPUT), 64);
    }
}
