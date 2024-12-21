import std;
import utils;

using namespace utils::assert;
namespace views = std::views;
namespace ranges = std::ranges;
using utils::pretty::Color;
using utils::pretty::colored;

struct Vec2 {
    std::int32_t x;
    std::int32_t y;
    friend auto operator+(Vec2 a, Vec2 b) -> Vec2 { return Vec2{a.x + b.x, a.y + b.y}; }
    auto rotated90() -> Vec2 { return Vec2{-y, x}; }
    auto operator<=>(const Vec2&) const = default;
};

using Game_marix = std::mdspan<char, std::dextents<std::size_t, 2>>;
struct Game_board {
    std::vector<char> data;
    Game_marix matrix;
    Game_board(std::vector<char> board_data, std::size_t height, std::size_t width)
        : data{std::move(board_data)}, matrix{data.data(), height, width}
    {
    }
};
struct Guard {
    Vec2 pos;
    Vec2 dir;
    auto operator<=>(const Guard&) const = default;
};

auto parse(std::string_view text) -> std::pair<Game_board, Guard>
{
    auto width_newlines = text.find_first_of('\n') + 1;
    auto height = text.size() / width_newlines;
    auto game_board_data = text                                                          //
                         | std::views::filter([](char c) { return c != '\n'; })          //
                         | views::transform([](char c) { return c == '#' ? '#' : '.'; }) //
                         | ranges::to<std::vector>();
    auto guard_pos_newlines = text.find_first_of('^');
    return {
        Game_board{std::move(game_board_data), height, width_newlines - 1},
        Guard{
            .pos{
                .x = static_cast<std::int32_t>(guard_pos_newlines % width_newlines),
                .y = static_cast<std::int32_t>(guard_pos_newlines / width_newlines)
            },
            .dir{0, -1}
        }
    };
}

auto next_state(Game_marix matrix, Guard guard) -> std::optional<Guard>
{
    auto [x, y] = guard.pos + guard.dir;
    if (x >= matrix.extent(0) || y >= matrix.extent(1)) {
        return std::nullopt;
    }
    return (matrix[y, x] == '#') ? Guard{guard.pos, guard.dir.rotated90()} : Guard{.pos{x, y}, .dir{guard.dir}};
}
auto get_states(Game_marix matrix, Guard guard) -> std::optional<std::set<Guard>>
{
    std::optional<Guard> next = guard;
    std::set<Guard> states;
    while ((next = next_state(matrix, *next))) {
        auto [_, did_insert] = states.insert(*next);
        if (!did_insert) {
            return std::nullopt;
        }
    }
    return std::move(states);
}


auto puzzle1(std::string_view text) -> std::uint32_t
{
    const auto [board, guard] = parse(text);
    auto states = *get_states(board.matrix, guard) //
                | views::transform(&Guard::pos)    //
                | ranges::to<std::vector>();
    std::ranges::sort(states);
    return ranges::distance(states | views::chunk_by(std::equal_to{}));
}


auto puzzle2(std::string_view text) -> std::uint32_t
{
    auto [board, guard] = parse(text);
    std::uint32_t counter = 0;
    std::println("size: {}", board.data.size());
    for (auto [c, i] : std::views::zip(board.data, std::views::iota(0uz))) {
        if (i % 100 == 0) std::println("iteration: {}", i);
        if (c == '#') continue;
        c = '#';
        if (!get_states(board.matrix, guard)) ++counter;
        c = '.';
    }
    return counter;
}


constexpr std::string_view test_input = R"(....#.....
.........#
..........
..#.......
.......#..
..........
.#..^.....
........#.
#.........
......#...
)";

auto main() -> int
{

    const std::string input = [] {
        std::ostringstream stream;
        std::ifstream file("../inputs/6.txt");
        stream << file.rdbuf();
        return std::move(stream).str();
    }();

    assert_eq(puzzle1(test_input), 41);
    std::println("{}", colored(Color::green, "Test for puzzle 1 passed"));
    std::println("result of puzzle1 is: {}", puzzle1(input));

    assert_eq(puzzle2(test_input), 6);
    std::println("{}", colored(Color::green, "Test for puzzle 2 passed"));
    std::println("result of puzzle2 is: {}", puzzle2(input));
}
